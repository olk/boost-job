
//          Copyright Oliver Kowalke 2015.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include "boost/job/memory.hpp"

extern "C" {
#include <windows.h>
}

#include <system_error>

namespace boost {
namespace jobs {

BOOST_JOBS_DECL
void * numa_alloc( std::size_t size, uint32_t node_id) {
    // allocate virtual memory
    void * mem = ::VirtualAllocExNuma(
                    ::GetCurrentProcess(),
                    nullptr,
                    size,
                    MEM_RESERVE | MEM_COMMIT,
                    PAGE_READWRITE,
                    node_id);
    if ( nullptr == mem) {
        throw std::system_error(
                std::error_code( ::GetLastError(), std::system_category() ),
                "::VirtualAllocExNuma() failed");
    }
    // virtual pages are allocated but no valid physical
    // pages are associated with them yet
    // ::FillMemory() will touch every page in the buffer, faulting
    // them into our working set
    // physical pages will be allocated
    // from the preferred node we specified in ::VirtualAllocExNuma(),
    // or any node if the preferred one is out of pages
    FillMemory( mem, size, '\0'); // macro?
    return mem;
}

BOOST_JOBS_DECL
void numa_free( void * mem, std::size_t) {
    ::VirtualFree( mem, 0, MEM_RELEASE);
}

}}

