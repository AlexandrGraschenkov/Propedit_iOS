/*
   mkvmerge -- utility for splicing together matroska files
   from component media subtypes

   Distributed under the GPL v2
   see the file COPYING for details
   or visit http://www.gnu.org/copyleft/gpl.html

   helper function for WebVTT data

   Written by Moritz Bunkus <moritz@bunkus.org>.
*/

#include "common/common_pch.h"

#include "common/strings/formatting.h"
#include "common/strings/parsing.h"
#include "common/webvtt.h"

#define RE_TIMESTAMP "((?:\\d{2}:)?\\d{2}:\\d{2}\\.\\d{3})"


namespace std
{


template<class BidirIt, class Traits, class CharT, class UnaryFunction>
std::basic_string<CharT> regex_replace_lambda2(BidirIt first, BidirIt last,
    const std::basic_regex<CharT,Traits>& re, UnaryFunction f)
{
    std::basic_string<CharT> s;

    typename std::match_results<BidirIt>::difference_type
        positionOfLastMatch = 0;
    auto endOfLastMatch = first;

    auto callback = [&](const std::match_results<BidirIt>& match)
    {
        auto positionOfThisMatch = match.position(0);
        auto diff = positionOfThisMatch - positionOfLastMatch;

        auto startOfThisMatch = endOfLastMatch;
        std::advance(startOfThisMatch, diff);

        s.append(endOfLastMatch, startOfThisMatch);
        s.append(f(match));

        auto lengthOfMatch = match.length(0);

        positionOfLastMatch = positionOfThisMatch + lengthOfMatch;

        endOfLastMatch = startOfThisMatch;
        std::advance(endOfLastMatch, lengthOfMatch);
    };

    std::regex_iterator<BidirIt> begin(first, last, re), end;
    std::for_each(begin, end, callback);

    s.append(endOfLastMatch, last);

    return s;
}

template<class Traits, class CharT, class UnaryFunction>
std::string regex_replace_lambda2(const std::string& s,
    const std::basic_regex<CharT,Traits>& re, UnaryFunction f)
{
    return regex_replace_lambda2(s.cbegin(), s.cend(), re, f);
}

} // namespace std

struct webvtt_parser_c::impl_t {
public:
  std::vector<std::string> current_block, global_blocks, local_blocks;
  bool parsing_global_data{true};
  std::deque<webvtt_parser_c::cue_cptr> cues;
  unsigned int current_cue_number{}, total_number_of_cues{};
  debugging_option_c debug{"webvtt_parser"};

  std::regex timestamp_line_re{"^" RE_TIMESTAMP " --> " RE_TIMESTAMP "(?: ([^\\n]+))?$"};
};

webvtt_parser_c::webvtt_parser_c()
  : m{new impl_t()}
{
}

webvtt_parser_c::~webvtt_parser_c() {
}

void
webvtt_parser_c::add_line(std::string const &line) {
  auto tmp = chomp(line);

  if (tmp.empty())
    add_block();

  else
    m->current_block.emplace_back(std::move(tmp));
}

void
webvtt_parser_c::add_joined_lines(std::string const &joined_lines) {
  auto lines = split(chomp(normalize_line_endings(joined_lines)), "\n");

  for (auto const &line : lines)
    add_line(line);
}

void
webvtt_parser_c::add_joined_lines(memory_c const &mem) {
  if (!mem.get_size())
    return;

  add_joined_lines(std::string{reinterpret_cast<char const *>(mem.get_buffer()), static_cast<std::string::size_type>(mem.get_size())});
}

void
webvtt_parser_c::flush() {
  add_block();
  m->parsing_global_data = false;
}

void
webvtt_parser_c::add_block() {
  if (m->current_block.empty())
    return;

  std::smatch matches;
  std::string label, additional;
  auto timestamp_line = -1;

  if (std::regex_search(m->current_block[0], matches, m->timestamp_line_re))
    timestamp_line = 0;

  else if ((m->current_block.size() > 1) && std::regex_search(m->current_block[1], matches, m->timestamp_line_re)) {
    timestamp_line = 1;
    label          = std::move(m->current_block[0]);

  } else {
    auto content = mbalgm::join(m->current_block, "\n");
    (m->parsing_global_data ? m->global_blocks : m->local_blocks).emplace_back(std::move(content));

    m->current_block.clear();

    return;
  }

  m->parsing_global_data = false;

  timestamp_c start, end;
  parse_timestamp(matches[1].str(), start);
  parse_timestamp(matches[2].str(), end);

  auto content       = mbalgm::join(std::vector<std::string>(m->current_block.begin() + timestamp_line + 1, m->current_block.end()), "\n");;
  content            = adjust_embedded_timestamps(content, start.negate());
  auto cue           = std::make_shared<cue_t>();
  cue->m_start       = start;
  cue->m_duration    = end - start;
  cue->m_content     = memory_c::clone(content);
  auto settings_list = matches[3].str();

  if (! (label.empty() && settings_list.empty() && m->local_blocks.empty())) {
    additional = settings_list + "\n" + label + "\n" + mbalgm::join(m->local_blocks, "\n");
    cue->m_addition = memory_c::clone(additional);
  }

  mxdebug_if(m->debug,
             strformat::bstr("label «%1%» start «%2%» end «%3%» settings list «%4%» additional «%5%» content «%6%»\n")
             % label % matches[1].str() % matches[2].str() % matches[3].str()
             % std::regex_replace(additional, std::regex{"\n+"}, "–")
             % std::regex_replace(content,    std::regex{"\n+"}, "–"));

  m->local_blocks.clear();
  m->current_block.clear();

  m->cues.emplace_back(cue);
}

bool
webvtt_parser_c::codec_private_available()
  const {
  return !m->parsing_global_data;
}

memory_cptr
webvtt_parser_c::get_codec_private()
  const {
  return memory_c::clone(mbalgm::join(m->global_blocks, "\n\n"));
}

bool
webvtt_parser_c::cue_available()
  const {
  return !m->cues.empty();
}

webvtt_parser_c::cue_cptr
webvtt_parser_c::get_cue() {
  auto cue = m->cues.front();
  m->cues.pop_front();
  return cue;
}

unsigned int
webvtt_parser_c::get_current_cue_number()
  const {
  return m->current_cue_number;
}

unsigned int
webvtt_parser_c::get_total_number_of_cues()
  const {
  return m->total_number_of_cues;
}

unsigned int
webvtt_parser_c::get_progress_percentage()
  const {
  if (!m->total_number_of_cues)
    return 100;
  return m->current_cue_number * 100 / m->total_number_of_cues;
}

std::string
webvtt_parser_c::adjust_embedded_timestamps(std::string const &text,
                                            timestamp_c const &offset) {
  static std::regex s_embedded_timestamp_re;
    static bool once = true;

    if (once) {
        s_embedded_timestamp_re = std::regex{"<" RE_TIMESTAMP ">"};
        once = false;
    }

  return std::regex_replace_lambda2(text, s_embedded_timestamp_re, [&offset](std::smatch const &match) -> std::string {
    timestamp_c timestamp;
    parse_timestamp(match[1].str(), timestamp);
    return (strformat::bstr("<%1%>") % format_timestamp(timestamp + offset, 3)).str();
  });
}
