
//          Copyright Oliver Kowalke 2015.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include "boost/job/pin.hpp"

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

BOOST_JOBS_DECL
void pin_thread( std::thread::native_handle_type hndl, uint32_t cpuid) {
    cpu_set_t cpuset;
    CPU_ZERO( & cpuset);
    CPU_SET( cpuid, & cpuset);

    int err = 0;
    if ( 0 != ( err = ::pthread_setaffinity_np( hndl, sizeof( cpuset), & cpuset) ) ) {
        throw std::system_error(
                std::error_code( err, std::system_category() ),
                "pthread_setaffinity_np() failed");
    }
}

BOOST_JOBS_DECL
void pin_thread( uint32_t cpuid) {
    pin_thread( ::pthread_self(),  cpuid);
}

}}

#ifdef BOOST_HAS_ABI_HEADERS
# include BOOST_ABI_SUFFIX
#endif
