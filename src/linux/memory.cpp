
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
    void * mem = ::mmap( 0, size,
                         PROT_READ|PROT_WRITE,
                         MAP_PRIVATE|MAP_ANONYMOUS, 0, 0);
    if ( MAP_FAILED == mem) {
        throw std::system_error(
                std::error_code( errno, std::system_category() ),
                "mmap() failed");
    }

    // TODO: allow more than 64 NUMA Nodes
    uint64_t nodemask = 1 << node_id;
    // TODO: allow to switch to other policies/flags
    //  libnuma defined policies:
    //       MPOL_DEFAULT     0
    //       MPOL_PREFERRED   1
    //       MPOL_BIND        2
    //       MPOL_INTERLEAVE  3
    //  libnuma defined flags:
    //       MPOL_MF_STRICT   (1<<0) // Verify existing pages in the mapping
    //       MPOL_MF_MOVE     (1<<1) // Move pages owned by this process to conform to mapping
    //       MPOL_MF_MOVE_ALL (1<<2) // Move every page to conform to mapping
    int mode = 1;
    unsigned int flags = 1 << 1;
    if ( -1 == syscall_mbind( mem, size,
                              mode,
                              & nodemask, 64,
                              flags) ) {
        throw std::system_error(
                std::error_code( errno, std::system_category() ),
                "mbind() failed");
    }

    return mem;
}

BOOST_JOBS_DECL
void numa_free( void * mem, std::size_t size) {
    ::munmap( mem, size);
}

}}

