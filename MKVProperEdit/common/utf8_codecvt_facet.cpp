// Copyright Vladimir Prus 2004.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)

// For HP-UX, request that WCHAR_MAX and WCHAR_MIN be defined as macros,
// not casts. See ticket 5048
#define _INCLUDE_STDCSOURCE_199901

#ifndef BOOST_SYSTEM_NO_DEPRECATED
# define BOOST_SYSTEM_NO_DEPRECATED
#endif

#define BOOST_UTF8_BEGIN_NAMESPACE namespace mtx {

#define BOOST_UTF8_END_NAMESPACE }
#define BOOST_UTF8_DECL

#include </Users/alex/Documents/work_projects/boost/src/boost_1_55_0/libs/serialization/src/utf8_codecvt_facet.cpp>

#undef BOOST_UTF8_BEGIN_NAMESPACE
#undef BOOST_UTF8_END_NAMESPACE
#undef BOOST_UTF8_DECL
