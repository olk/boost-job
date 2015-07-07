
//          Copyright Oliver Kowalke 2015.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include "boost/job/memory.hpp"

#include <cerrno>
#include <cstdlib>
#include <system_error>

namespace boost {
namespace jobs {

BOOST_JOBS_DECL
void * numa_alloc( std::size_t size, uint32_t) {
    void * mem = std::malloc( size);
    if ( nullptr == mem) {
        throw std::system_error(
                std::error_code( errno, std::system_category() ),
                "malloc() failed");
    }
    return mem;
}

BOOST_JOBS_DECL
void numa_free( void * mem, std::size_t) {
    std::free( mem);
}

}}

