
//          Copyright Oliver Kowalke 2015.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include "boost/job/pin.hpp"

extern "C" {
#include <sys/errno.h>
#include <sys/processor.h>
#include <sys/thread.h>
}

#include <system_error>

#ifdef BOOST_HAS_ABI_HEADERS
# include BOOST_ABI_PREFIX
#endif

namespace boost {
namespace jobs {

BOOST_JOBS_DECL
void pin_thread( uint32_t cpuid) {
    if ( -1 == ::bindprocessor( BINDTHREAD, ::thread_self(), static_cast< cpu_t >( cpuid) ) ) {
        throw std::system_error(
                std::error_code( ::getuerror(), std::system_category() ),
                "bindprocessor() failed");
    }
}

}}

#ifdef BOOST_HAS_ABI_HEADERS
# include BOOST_ABI_SUFFIX
#endif
