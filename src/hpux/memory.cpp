
//          Copyright Oliver Kowalke 2015.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include "boost/job/memory.hpp"

extern "C" {
#include <asm/unistd.h>
#include <errno.h>
#include <syscall.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <unistd.h>
}

#include <system_error>

namespace {

long syscall_mbind( void * start, unsigned long len,
                    int mode,
                    const unsigned long * nmask, unsigned long maxnode,
                    unsigned flags) {
    return ::syscall( __NR_mbind, ( long) start, len,
                      mode,
                      ( long) nmask, maxnode,
                      flags);
}

}

namespace boost {
namespace jobs {

BOOST_JOBS_DECL
void * numa_alloc( std::size_t size, uint32_t node_id) {
    int flags = 0; //MAP_MEM_FIRST_TOUCH, MAP_MEM_INTERLEAVED
    void * mem = ::mmap( nullptr, size,
                         PROT_READ|PROT_WRITE,
                         MAP_PRIVATE|MAP_ANONYMOUS|flags, -1, 0);
    if ( MAP_FAILED == mem) {
        throw std::system_error(
                std::error_code( errno, std::system_category() ),
                "mmap() failed");
    }
    return mem;
}

BOOST_JOBS_DECL
void numa_free( void * mem, std::size_t size) {
    ::munmap( mem, size);
}

}}

