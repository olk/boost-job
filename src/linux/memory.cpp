
//          Copyright Oliver Kowalke 2015.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include "boost/job/memory.hpp"

extern "C" {
#include <errno.h>
#include <sys/mman.h>
}

#include <system_error>

namespace boost {
namespace jobs {

BOOST_JOBS_DECL
void * node_alloc( std::size_t size, uint32_t node_id) {
    void * mem = ::mmap( 0, size,
                         PROT_READ|PROT_WRITE,
                         MAP_PRIVATE|MAP_ANONYMOUS, 0, 0);

    if ( -1 == ::mbind( mem, size, pol, bmp, bmp->size + 1, 0) ) {
        throw std::system_error(
                std::error_code( err, std::system_category() ),
                "mbind() failed");
    }

    return mem;
}

BOOST_JOBS_DECL
void node_free( void * mem, std::size_t size) {
    ::munmap( mem, size);
}

}}

