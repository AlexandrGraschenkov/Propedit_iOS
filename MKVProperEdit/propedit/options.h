/*
   mkvpropedit -- utility for editing properties of existing Matroska files

   Distributed under the GPL v2
   see the file COPYING for details
   or visit http://www.gnu.org/copyleft/gpl.html

   Written by Moritz Bunkus <moritz@bunkus.org>.
*/

#pragma once

#include "common/common_pch.h"
#include "common/kax_analyzer.h"
#include "propedit/target.h"
#include <ebml/EbmlMaster.h>

class options_c {
public:
  std::string m_file_name;
  std::vector<target_cptr> m_targets;
  bool m_show_progress;
  kax_analyzer_c::parse_mode_e m_parse_mode;

public:
  options_c();

  void validate();
  void options_parsed();

  target_cptr add_track_or_segmentinfo_target(std::string const &spec);
  void set_file_name(const std::string &file_name);
  void set_parse_mode(const std::string &parse_mode);
  void dump_info() const;
  bool has_changes() const;

  void find_elements(kax_analyzer_c *analyzer);

  void execute(kax_analyzer_c &analzyer);

protected:
  void remove_empty_targets();
  void merge_targets();
  void prune_empty_masters();
};
using options_cptr = std::shared_ptr<options_c>;
