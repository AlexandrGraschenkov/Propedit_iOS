/*
   mkvmerge -- utility for splicing together matroska files
   from component media subtypes

   Distributed under the GPL v2
   see the file COPYING for details
   or visit http://www.gnu.org/copyleft/gpl.html

   helper functions for WebM

   Written by Moritz Bunkus <moritz@bunkus.org>.
*/

#include "common/common_pch.h"

#include "common/webm.h"

bool
is_webm_file_name(const std::string &file_name) {
  static std::regex s_webm_file_name_re("\\.webm(?:a|v)?$");
  return std::regex_search(file_name, s_webm_file_name_re);
}
