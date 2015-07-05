
//          Copyright Oliver Kowalke 2015.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include "boost/job/pin.hpp"

extern "C" {
#include <errno.h>
#include <pthread_np.h>
#include <sched.h>
}

#include <system_error>

#ifdef BOOST_HAS_ABI_HEADERS
# include BOOST_ABI_PREFIX
#endif

namespace boost {
namespace jobs {

BOOST_JOBS_DECL
void pin_thread( uint32_t cpuid) {
    cpuset_t * set = ::cpuset_create();
    if ( nullptr == set) {
        throw std::system_error(
                std::error_code( errno, std::system_category() ),
                "::cpuset_create() failed");
    }
    ::cpuset_zero( set);
    ::cpuset_set( cpuid, set);
    int err = 0;
    if ( 0 != ( err = ::pthread_setaffinity_np( ::pthread_self(), ::cpuset_size( set), set) ) ) {
        ::cpuset_destroy( set);
        throw std::system_error(
                std::error_code( err, std::system_category() ),
                "pthread_setaffinity_np() failed");
    }
    ::cpuset_destroy( set);
}

}}

#ifdef BOOST_HAS_ABI_HEADERS
# include BOOST_ABI_SUFFIX
#endif
