
//          Copyright Oliver Kowalke 2015.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include "boost/job/pin.hpp"

extern "C" {
#include <windows.h>
}

#include <system_error>

#ifdef BOOST_HAS_ABI_HEADERS
# include BOOST_ABI_PREFIX
#endif

namespace boost {
namespace jobs {

BOOST_JOBS_DECL
void pin_thread( uint32_t cpuid) {
    GROUP_AFFINITY affinity;
    // compute processor group
    // a group contains max 64 logical CPUs
    affinity.Group = cpuid / 64;
    // compute the ID of the logical CPU in the group
    uint32_t id = cpuid % 64 + 1; 
    // set the bit mask of the logical CPU
    affinity.Mask = static_cast< KAFFINITY >( 1) << id;
    if ( 0 == ::SetGroupAffinity( ::GetCurrentThread(), affinity, nullptr);
        throw std::system_error(
                std::error_code( ::GetLastError(), std::system_category() ),
                "::SetGroupAffinity() failed");
}

}}

#ifdef BOOST_HAS_ABI_HEADERS
# include BOOST_ABI_SUFFIX
#endif
