/*
   mkvpropedit -- utility for editing properties of existing Matroska files

   Distributed under the GPL v2
   see the file COPYING for details
   or visit http://www.gnu.org/copyleft/gpl.html

   Written by Moritz Bunkus <moritz@bunkus.org>.
*/

#pragma once
#include <stdio.h>

class test_class {
public:
    test_class() {};
    void print();
    int kkk;
    
};

void
run_edit(int argc,
               char **argv);

#define FILE_NOT_MODIFIED Y("The file has not been modified.")
