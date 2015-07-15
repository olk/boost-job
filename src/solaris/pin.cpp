
//          Copyright Oliver Kowalke 2015.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include "boost/job/detail/pin.hpp"

extern "C" {
#include <errno.h>
#include <sys/types.h>
#include <sys/processor.h>
#include <sys/procset.h>
}

#include <system_error>

#ifdef BOOST_HAS_ABI_HEADERS
# include BOOST_ABI_PREFIX
#endif

namespace boost {
namespace jobs {
namespace detail {

BOOST_JOBS_DECL
void pin_thread( uint32_t cpuid) {
    if ( -1 == ::processor_bind( P_LWPID,
                                 P_MYID,
                                 static_cast< processorid_t >( cpuid),
                                 0) ) {
        throw std::system_error(
                std::error_code( errno, std::system_category() ),
                "processor_bind() failed");
    }
}

}}}

#ifdef BOOST_HAS_ABI_HEADERS
# include BOOST_ABI_SUFFIX
#endif
