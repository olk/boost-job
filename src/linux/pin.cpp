
//          Copyright Oliver Kowalke 2015.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include "boost/job/detail/pin.hpp"

extern "C" {
#include <pthread.h>
#include <sched.h>
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
    cpu_set_t set;
    CPU_ZERO( & set);
    CPU_SET( cpuid, & set);

    int err = 0;
    if ( 0 != ( err = ::pthread_setaffinity_np( ::pthread_self(), sizeof( set), & set) ) ) {
        throw std::system_error(
                std::error_code( err, std::system_category() ),
                "pthread_setaffinity_np() failed");
    }
}

}}}

#ifdef BOOST_HAS_ABI_HEADERS
# include BOOST_ABI_SUFFIX
#endif
