
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
void pin_thread( std::thread::native_handle_type hndl, uint32_t cpuid) {
    if ( 0 == ::SetThreadAffinityMask( hndl, ( DWORD_PTR) 1 << cpuid) )
        throw std::system_error(
                std::error_code( ::GetLastError(), std::system_category() ),
                "::SetThreadAffinityMask() failed");
}

BOOST_JOBS_DECL
void pin_thread( uint32_t cpuid) {
    pin_thread( ::GetCurrentThread(), cpuid);
}

}}

#ifdef BOOST_HAS_ABI_HEADERS
# include BOOST_ABI_SUFFIX
#endif
