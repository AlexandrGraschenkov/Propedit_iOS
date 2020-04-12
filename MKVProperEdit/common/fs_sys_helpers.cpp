//
//  fs_sys_helpers.cpp
//  MKVProperEdit
//
//  Created by Alexander Graschenkov on 12.04.2020.
//  Copyright © 2020 Alex the Best. All rights reserved.
//

#include "common/common_pch.h"
#include <sys/time.h>

#if defined(SYS_APPLE)
# include <mach-o/dyld.h>
#endif

namespace mtx { namespace sys {

int64_t
get_current_time_millis() {
  struct timeval tv;
  if (0 != gettimeofday(&tv, nullptr))
    return -1;

  return (int64_t)tv.tv_sec * 1000 + (int64_t)tv.tv_usec / 1000;
}


uint64_t
get_memory_usage() {
  return 0;
}

}
}
