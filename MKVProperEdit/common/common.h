/*
   mkvmerge -- utility for splicing together matroska files
   from component media subtypes

   Distributed under the GPL v2
   see the file COPYING for details
   or visit http://www.gnu.org/copyleft/gpl.html

   definitions used in all programs, helper functions

   Written by Moritz Bunkus <moritz@bunkus.org>.
*/

#pragma once
// 


#undef min
#undef max

#include <type_traits>

#undef __STRICT_ANSI__

#include <algorithm>
#include <functional>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

#include <cassert>
#include <cstring>
#include <regex>



#ifndef strformat_h
#define strformat_h

#include <sstream>
namespace strformat {
class bstr {
public:
    bstr (const std::string& x);
    
//    bstr& operator%(const std::string& other);
//    bstr& operator%(const int& other);
//    bstr& operator%(const std::exception& other);
    
    template <typename C>
    bstr& operator%(C const& other) {
        std::ostringstream strStream;
        strStream << other;
        std::string str = strStream.str();
        
        std::string findStr = "%" + std::to_string(index) + "%";
        auto foundIndex = innerStr.find(findStr, 0);
        if (foundIndex == std::string::npos) return *this;

        innerStr.replace(foundIndex, findStr.size(), str);
        index++;
        
        return *this;
    };
    
    operator std::string();// {return innerStr;};
    const std::string& str() const;
private:
    std::string innerStr;
    int index = 1;
};

std::ostream &operator<<(std::ostream &os, bstr const &m);
};

#include <algorithm>
#include <string>
namespace mbalgm {
std::string to_upper_copy(std::string str);
void to_upper(std::string &str);

std::string to_lower_copy(std::string str);
void to_lower(std::string &str);

std::string join(const std::vector<std::string> &vec, const std::string &sep);
bool istarts_with(const std::string &str, const std::string &other);
bool starts_with(const std::string &str, const std::string &other);
bool iequals(const std::string &str, const std::string &other);

template< class Container, class Pred >
inline Container& remove_erase_if( Container& on, Pred pred )
{
    on.erase(
             std::remove_if(on.begin(), on.end(), pred),
             on.end());
    return on;
}

template <typename T>
class optional {
public:
    optional(){};
    optional(const T &v) {
        values.push_back(v);
    }
    bool exist() const {
        return values.size() > 0;
    }
//    const T& get() const {
//        return values[0];
//    }
//    const T& getc() const {
//        return values[0];
//    }
//    T& get() {
//        return values[0];
//    }
    bool operator!() const {
        return values.size() == 0;
    }
    
    operator bool() const {
        return values.size() > 0;
    }
    const T& operator*() const {
        return values[0];
    }
    void reset(const T& v) {
        if (values.size())
            values[0] = v;
        else
            values.push_back(v);
    }
private:
    std::vector<T> values;
};
};
#endif

#if defined(HAVE_SYS_TYPES_H)
# include <sys/types.h>
#endif // HAVE_SYS_TYPES_H
#if defined(HAVE_STDINT_H)
# include <stdint.h>
#endif // HAVE_STDINT_H
#if defined(HAVE_INTTYPES_H)
# include <inttypes.h>
#endif // HAVE_INTTYPES_H

#include <boost/filesystem.hpp>
#include <boost/format.hpp>
#include <boost/function.hpp>
#include <boost/logic/tribool.hpp>
#include <boost/math/common_factor.hpp>
#include <boost/range.hpp>
#include <boost/range/adaptor/filtered.hpp>
#include <boost/range/adaptor/indexed.hpp>
#include <boost/range/adaptor/map.hpp>
#include <boost/range/algorithm_ext.hpp>
#include <boost/range/algorithm.hpp>
#include <boost/range/numeric.hpp>
#include <boost/rational.hpp>
#include <boost/regex.hpp>
#include <boost/system/error_code.hpp>

namespace badap = boost::adaptors;
namespace bfs   = boost::filesystem;
namespace brng  = boost::range;

#include "common/os.h"

#include <ebml/libebml_t.h>
#undef min
#undef max

#include <ebml/EbmlElement.h>
#include <ebml/EbmlMaster.h>

/* i18n stuff */
#if defined(HAVE_LIBINTL_H)
# include <libintl.h>
#else
# define gettext(s)                            (s)
# define ngettext(s_singular, s_plural, count) ((count) != 1 ? (s_plural) : (s_singular))
#endif

#undef Y
#undef NY
#define Y(s)                            gettext(u8##s)
#define NY(s_singular, s_plural, count) ngettext(u8##s_singular, u8##s_plural, count)
#define YF(s)        Y(s)
#define NYF(s, p, c) NY(s, p, c)

#include "common/debugging.h"
#include "common/error.h"
#include "common/memory.h"
#include "common/mm_io.h"
#include "common/output.h"

namespace libebml {
class EbmlBinary;
};

namespace libmatroska { };

using namespace libebml;
using namespace libmatroska;

#define BUGMSG Y("This should not have happened. Please contact the author " \
                 "Moritz Bunkus <moritz@bunkus.org> with this error/warning " \
                 "message, a description of what you were trying to do, " \
                 "the command line used and which operating system you are " \
                 "using. Thank you.")


#define MXMSG_ERROR    5
#define MXMSG_WARNING 10
#define MXMSG_INFO    15
#define MXMSG_DEBUG   20

#if !defined(FOURCC)
#define FOURCC(a, b, c, d) (uint32_t)((((unsigned char)a) << 24) + \
                                      (((unsigned char)b) << 16) + \
                                      (((unsigned char)c) <<  8) + \
                                       ((unsigned char)d))
#endif
#define isblanktab(c) (((c) == ' ')  || ((c) == '\t'))
#define iscr(c)       (((c) == '\n') || ((c) == '\r'))

#define TIMESTAMP_SCALE 1000000

void mxrun_before_exit(std::function<void()> function);
void mxexit(int code = -1);
void set_process_priority(int priority);

extern unsigned int verbose;

void mtx_common_init(std::string const &program_name, char const *argv0);
std::string const &get_program_name();

