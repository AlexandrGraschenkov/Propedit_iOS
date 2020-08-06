/*
   mkvmerge -- utility for splicing together matroska files
   from component media subtypes

   Distributed under the GPL v2
   see the file COPYING for details
   or visit http://www.gnu.org/copyleft/gpl.html

   string helper functions

   Written by Moritz Bunkus <moritz@bunkus.org>.
*/

#pragma once

#include "common/common_pch.h"
#include <regex>
#include <string>
#include <algorithm>
#include <iterator>

enum class line_ending_style_e {
  cr_lf,
  lf,
};

std::string normalize_line_endings(std::string const &str, line_ending_style_e line_ending_style = line_ending_style_e::lf);
std::string chomp(std::string const &str);

std::vector<std::string> split(std::string const &text, std::regex const &pattern, size_t max = 0, std::regex_constants::match_flag_type match_flags = std::regex_constants::match_default);

inline std::vector<std::string>
split(std::string const &text,
      std::string const &pattern = ",",
      size_t max = 0) {
    
    std::vector<std::string> cont;
    std::size_t current, previous = 0;
    current = text.find_first_of(pattern);
    while (current != std::string::npos) {
        if (max > 0 && cont.size() >= max) break;
        cont.push_back(text.substr(previous, current - previous));
        previous = current + 1;
        current = text.find_first_of(pattern, previous);
    }
    if (max == 0 || cont.size() < max) {
        cont.push_back(text.substr(previous, current - previous));
    }
    return cont;
}

void strip(std::string &s, bool newlines = false);
std::string strip_copy(std::string const &s, bool newlines = false);
void strip(std::vector<std::string> &v, bool newlines = false);
void strip_back(std::string &s, bool newlines = false);

std::string &shrink_whitespace(std::string &s);

std::string escape(const std::string &src);
std::string unescape(const std::string &src);

std::string get_displayable_string(const char *src, int max_len = -1);
std::string get_displayable_string(std::string const &src);

extern const std::string empty_string;
