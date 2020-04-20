/*
   mkvmerge -- utility for splicing together matroska files
   from component media subtypes

   Distributed under the GPL v2
   see the file COPYING for details
   or visit http://www.gnu.org/copyleft/gpl.html

   translation, locale handling

   Written by Moritz Bunkus <moritz@bunkus.org>.
*/
// 

#include "common/common_pch.h"

#if HAVE_NL_LANGINFO
# include <langinfo.h>
#elif HAVE_LOCALE_CHARSET
# include <libcharset.h>
#endif
#include <locale>
#include <locale.h>
#include <stdlib.h>

#include "common/fs_sys_helpers.h"
#include "common/locale_string.h"
#include "common/utf8_codecvt_facet.h"
#include "common/strings/editing.h"
#include "common/translation.h"

#if defined(SYS_WINDOWS)
# include <windows.h>
# include <winnls.h>

# include "common/memory.h"
#endif

std::vector<translation_c> translation_c::ms_available_translations;
int translation_c::ms_active_translation_idx = 0;

translation_c::translation_c(std::string const &iso639_2_code,
                             std::string const &unix_locale,
                             std::string const &windows_locale,
                             std::string const &windows_locale_sysname,
                             std::string const &english_name,
                             std::string const &translated_name,
                             bool line_breaks_anywhere,
                             int language_id,
                             int sub_language_id)
  : m_iso639_2_code{iso639_2_code}
  , m_unix_locale{unix_locale}
  , m_windows_locale{windows_locale}
  , m_windows_locale_sysname{windows_locale_sysname}
  , m_english_name{english_name}
  , m_translated_name{translated_name}
  , m_line_breaks_anywhere{line_breaks_anywhere}
  , m_language_id{language_id}
  , m_sub_language_id{sub_language_id}
{
}

// See http://msdn.microsoft.com/en-us/library/windows/desktop/dd318693.aspx for the (sub) language IDs.
void
translation_c::initialize_available_translations() {
  ms_available_translations.clear();
  ms_available_translations.emplace_back("eng", "en_US",       "en",          "english",    "English",              "English",             false, 0x0009, 0x00);
#if defined(HAVE_LIBINTL_H)
  ms_available_translations.emplace_back("cat", "ca_ES",       "ca",          "catalan",    "Catalan",              "Català",              false, 0x0003, 0x00);
  ms_available_translations.emplace_back("cze", "cs_CZ",       "cs",          "czech",      "Czech",                "Čeština",             false, 0x0005, 0x00);
  ms_available_translations.emplace_back("ger", "de_DE",       "de",          "german",     "German",               "Deutsch",             false, 0x0007, 0x00);
  ms_available_translations.emplace_back("spa", "es_ES",       "es",          "spanish",    "Spanish",              "Español",             false, 0x000a, 0x00);
  ms_available_translations.emplace_back("baq", "eu_ES",       "eu",          "basque",     "Basque",               "Euskara",             false, 0x002d, 0x00);
  ms_available_translations.emplace_back("fre", "fr_FR",       "fr",          "french",     "French",               "Français",            false, 0x000c, 0x00);
  ms_available_translations.emplace_back("ita", "it_IT",       "it",          "italian",    "Italian",              "Italiano",            false, 0x0010, 0x00);
  ms_available_translations.emplace_back("jpn", "ja_JP",       "ja",          "japanese",   "Japanese",             "日本語",              true,  0x0011, 0x00);
  ms_available_translations.emplace_back("kor", "ko_KR",       "ko",          "korean",     "Korean",               "한국어/조선말",       true,  0x0012, 0x01);
  ms_available_translations.emplace_back("lit", "lt_LT",       "lt",          "lithuanian", "Lithuanian",           "Lietuvių",            false, 0x0027, 0x00);
  ms_available_translations.emplace_back("dut", "nl_NL",       "nl",          "dutch",      "Dutch",                "Nederlands",          false, 0x0013, 0x00);
  ms_available_translations.emplace_back("pol", "pl_PL",       "pl",          "polish",     "Polish",               "Polski",              false, 0x0015, 0x00);
  ms_available_translations.emplace_back("por", "pt_BR",       "pt_BR",       "portuguese", "Brazilian Portuguese", "Português do Brasil", false, 0x0016, 0x01);
  ms_available_translations.emplace_back("por", "pt_PT",       "pt",          "portuguese", "Portuguese",           "Português",           false, 0x0016, 0x02);
  ms_available_translations.emplace_back("rum", "ro_RO",       "ro",          "romanian",   "Romanian",             "Română",              false, 0x0018, 0x00);
  ms_available_translations.emplace_back("rus", "ru_RU",       "ru",          "russian",    "Russian",              "Русский",             false, 0x0019, 0x00);
  ms_available_translations.emplace_back("srp", "sr_RS",       "sr_RS",       "serbian",    "Serbian Cyrillic",     "Српски",              false, 0x001a, 0x03);
  ms_available_translations.emplace_back("srp", "sr_RS@latin", "sr_RS@latin", "serbian",    "Serbian Latin",        "Srpski",              false, 0x001a, 0x02);
  ms_available_translations.emplace_back("swe", "sv_SE",       "sv",          "swedish",    "Swedish",              "Svenska",             false, 0x001d, 0x01);
  ms_available_translations.emplace_back("tur", "tr_TR",       "tr",          "turkish",    "Turkish",              "Türkçe",              false, 0x001f, 0x00);
  ms_available_translations.emplace_back("ukr", "uk_UA",       "uk",          "ukrainian",  "Ukrainian",            "Українська",          false, 0x0022, 0x00);
  ms_available_translations.emplace_back("chi", "zh_CN",       "zh_CN",       "chinese",    "Chinese Simplified",   "简体中文",            true,  0x0004, 0x02);
  ms_available_translations.emplace_back("chi", "zh_TW",       "zh_TW",       "chinese",    "Chinese Traditional",  "繁體中文",            true,  0x0004, 0x01);
#endif

  ms_active_translation_idx = 0;
}

int
translation_c::look_up_translation(const std::string &locale) {
  try {
    auto hits = std::vector< std::pair<int, int> >{};
    auto full = locale_string_c(locale).str(locale_string_c::full);
    auto half = locale_string_c(locale).str(locale_string_c::half);

    for (auto beg = ms_available_translations.begin(), itr = ms_available_translations.begin(), end = ms_available_translations.end(); itr != end; ++itr) {
      auto score = itr->matches(full) ? 2
                 : itr->matches(half) ? 1
                 :                      0;

      if (score)
        hits.emplace_back(std::make_pair(score, static_cast<int>(std::distance(beg, itr))));
    }

    if (!hits.empty()) {
      brng::sort(hits);
      return hits.back().second;
    }

  } catch (mtx::locale_string_format_x &) {
  }

  return -1;
}

int
translation_c::look_up_translation(int language_id, int sub_language_id) {
  auto ptr = brng::find_if(ms_available_translations, [language_id,sub_language_id](translation_c const &tr) {
      return (tr.m_language_id == language_id) && (!tr.m_sub_language_id || (tr.m_sub_language_id == sub_language_id));
    });

  int idx = ptr == ms_available_translations.end() ? -1 : std::distance(ms_available_translations.begin(), ptr);
  mxdebug_if(debugging_c::requested("locale"), strformat::bstr("look_up_translation for 0x%|1$04x|/0x%|2$02x|: %3%\n") % language_id % sub_language_id % idx);

  return idx;
}

std::string
translation_c::get_default_ui_locale() {
  std::string locale;

#if defined(HAVE_LIBINTL_H)
  bool debug = debugging_c::requested("locale");

# if defined(SYS_WINDOWS)
  std::string env_var = mtx::sys::get_environment_variable("LC_MESSAGES");
  if (!env_var.empty() && (-1 != look_up_translation(env_var)))
    return env_var;

  env_var = mtx::sys::get_environment_variable("LANG");
  if (!env_var.empty() && (-1 != look_up_translation(env_var)))
    return env_var;

  auto lang_id = GetUserDefaultUILanguage();
  int idx      = translation_c::look_up_translation(lang_id & 0x3ff, (lang_id >> 10) & 0x3f);
  if (-1 != idx)
    locale = ms_available_translations[idx].get_locale();

  mxdebug_if(debug, strformat::bstr("[lang_id %|1$04x| idx %2% locale %3%\n") % lang_id % idx % locale);

# else  // SYS_WINDOWS

  char *data = setlocale(LC_MESSAGES, nullptr);
  if (data) {
    std::string previous_locale = data;
    mxdebug_if(debug, strformat::bstr("[get_default_ui_locale previous %1%]\n") % previous_locale);
    setlocale(LC_MESSAGES, "");
    data = setlocale(LC_MESSAGES, nullptr);

    if (data)
      locale = data;

    mxdebug_if(debug, strformat::bstr("[get_default_ui_locale new %1%]\n") % locale);

    setlocale(LC_MESSAGES, previous_locale.c_str());
  } else
    mxdebug_if(debug, strformat::bstr("[get_default_ui_locale get previous failed]\n"));

# endif // SYS_WINDOWS
#endif  // HAVE_LIBINTL_H

  return locale;
}

std::string
translation_c::get_locale()
  const {
#if defined(SYS_WINDOWS)
  return m_windows_locale;
#else
  return m_unix_locale;
#endif
}

bool
translation_c::matches(std::string const &locale)
  const {
  if (mbalgm::iequals(locale, get_locale()))
    return true;

#if defined(SYS_WINDOWS)
  if (mbalgm::iequals(locale, m_windows_locale_sysname))
    return true;
#endif

  return false;
}

translation_c &
translation_c::get_active_translation() {
  return ms_available_translations[ms_active_translation_idx];
}

void
translation_c::set_active_translation(const std::string &locale) {
  int idx                   = look_up_translation(locale);
  ms_active_translation_idx = std::max(idx, 0);

  mxdebug_if(debugging_c::requested("locale"), strformat::bstr("[translation_c::set_active_translation() active_translation_idx %1% for locale %2%]\n") % ms_active_translation_idx % locale);
}

// ------------------------------------------------------------

translatable_string_c::translatable_string_c()
{
}

translatable_string_c::translatable_string_c(const std::string &untranslated_string)
  : m_untranslated_strings{untranslated_string}
{
}

translatable_string_c::translatable_string_c(const char *untranslated_string)
  : m_untranslated_strings{std::string{untranslated_string}}
{
}

translatable_string_c::translatable_string_c(std::vector<translatable_string_c> const &untranslated_strings)
{
  for (auto const &untranslated_string : untranslated_strings)
    m_untranslated_strings.emplace_back(untranslated_string.get_untranslated());
}

std::string
translatable_string_c::get_translated()
  const
{
  if (m_overridden_by)
    return *m_overridden_by;

  std::vector<std::string> translated_strings;
  for (auto const &untranslated_string : m_untranslated_strings)
    if (!untranslated_string.empty())
      translated_strings.emplace_back(gettext(untranslated_string.c_str()));

  return join(translated_strings);
}

std::string
translatable_string_c::get_untranslated()
  const
{
  return join(m_untranslated_strings);
}

translatable_string_c &
translatable_string_c::override(std::string const &by) {
  m_overridden_by.reset(by);
  return *this;
}

std::string
translatable_string_c::join(std::vector<std::string> const &strings)
  const {
  auto separator = translation_c::get_active_translation().m_line_breaks_anywhere ? "" : " ";
  return mbalgm::join(strings, separator);
}

// ------------------------------------------------------------

#if defined(HAVE_LIBINTL_H)

void
init_locales(std::string locale) {
  auto debug = debugging_c::requested("locale");

  translation_c::initialize_available_translations();

  mxdebug_if(debug, strformat::bstr("[init_locales start: locale %1%]\n") % locale);

  std::string locale_dir;
  std::string default_locale = translation_c::get_default_ui_locale();

  if (-1 == translation_c::look_up_translation(locale)) {
    mxdebug_if(debug, strformat::bstr("[init_locales lookup failed; clearing locale]\n"));
    locale = "";
  }

  if (locale.empty()) {
    locale = default_locale;
    mxdebug_if(debug, strformat::bstr("[init_locales setting to default locale %1%]\n") % locale);
  }

# if defined(SYS_WINDOWS)
  mtx::sys::set_environment_variable("LANGUAGE", "");

  if (!locale.empty()) {
    // The Windows system headers define LC_MESSAGES but
    // Windows itself doesn't know it and treats "set_locale(LC_MESSAGES, ...)"
    // as an error. gettext uses the LANG and LC_MESSAGE environment variables instead.

    // Windows knows two environments: the system environment that's
    // modified by SetEnvironmentVariable() and the C library's cache
    // of said environment which is modified via _putenv().

    mtx::sys::set_environment_variable("LANG",        locale);
    mtx::sys::set_environment_variable("LC_MESSAGES", locale);

    translation_c::set_active_translation(locale);
  }

  // Boost's path class uses wide chars on Windows for path
  // names. Tell that all narrow strings are encoded in UTF-8.
  std::locale utf8_locale(std::locale(), new mtx::utf8_codecvt_facet);
  std::locale::global(utf8_locale);
  boost::filesystem::path::imbue(utf8_locale);

  locale_dir = g_cc_local_utf8->native((mtx::sys::get_installation_path() / "locale").string());

# else  // SYS_WINDOWS
  auto language_var = mtx::sys::get_environment_variable("LANGUAGE");
  if (!language_var.empty()) {
    mxdebug_if(debug, strformat::bstr("[init_locales LANGUAGE is set to %1%; un-setting it]\n") % language_var);
    mtx::sys::unset_environment_variable("LANGUAGE");
  }

  std::string chosen_locale;

  try {
    locale_string_c loc_default(default_locale);
    std::string loc_req_with_default_codeset(locale_string_c(locale).set_codeset_and_modifier(loc_default).str());

    mxdebug_if(debug, strformat::bstr("[init_locales loc_default is %1%; trying locale %3% followed by loc_req_with_default_codeset %2%]\n") % loc_default.str() % loc_req_with_default_codeset % locale);

    if (setlocale(LC_MESSAGES, locale.c_str()))
      chosen_locale = locale;

    else if (setlocale(LC_MESSAGES, loc_req_with_default_codeset.c_str()))
      chosen_locale = loc_req_with_default_codeset;

    else {
      std::string loc_req_with_utf8 = locale_string_c(locale).set_codeset_and_modifier(locale_string_c("dummy.UTF-8")).str();
      mxdebug_if(debug, strformat::bstr("[init_locales both failed; also trying %1%]\n") % loc_req_with_utf8);
      if (setlocale(LC_MESSAGES, loc_req_with_utf8.c_str()))
        chosen_locale = loc_req_with_utf8;
    }

  } catch (mtx::locale_string_format_x &error) {
    mxdebug_if(debug, strformat::bstr("[init_locales format error in %1%]\n") % error.error());
  }

  mxdebug_if(debug, strformat::bstr("[init_locales chosen locale %1%]\n") % chosen_locale);

  // Hard fallback to "C" locale if no suitable locale was
  // selected. This can happen if the system has no locales for
  // "en_US" or "en_US.UTF-8" compiled.
  if (chosen_locale.empty() && setlocale(LC_MESSAGES, "C"))
    chosen_locale = "C";

  if (chosen_locale.empty())
    mxerror(Y("The locale could not be set properly. Check the LANG, LC_ALL and LC_MESSAGES environment variables.\n"));

  std::locale utf8_locale(std::locale(), new mtx::utf8_codecvt_facet);
  std::locale::global(utf8_locale);

  translation_c::set_active_translation(chosen_locale);

#  if defined(MTX_APPIMAGE)
  locale_dir = (mtx::sys::get_installation_path() / ".." / "share" / "locale").string();
#  else
  locale_dir = MTX_LOCALE_DIR;
#  endif  // MTX_APPIMAGE

# endif  // SYS_WINDOWS

# if defined(SYS_APPLE)
  int result = setenv("LC_MESSAGES", chosen_locale.c_str(), 1);
  mxdebug_if(debug, strformat::bstr("[init_locales setenv() return code: %1%]\n") % result);
# endif

  bindtextdomain("mkvtoolnix", locale_dir.c_str());
  textdomain("mkvtoolnix");
  bind_textdomain_codeset("mkvtoolnix", "UTF-8");
}

#else  // HAVE_LIBINTL_H

void
init_locales(std::string) {
  translation_c::initialize_available_translations();
}

#endif  // HAVE_LIBINTL_H
