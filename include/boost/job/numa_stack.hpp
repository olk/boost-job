
//          Copyright Oliver Kowalke 2014.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_JOBS_NUMA_STACK_H
#define BOOST_JOBS_NUMA_STACK_H

#include <cstddef>
#include <cstdlib>
#include <new>

#include <boost/assert.hpp>
#include <boost/config.hpp>
#include <boost/context/detail/config.hpp>
#include <boost/context/stack_context.hpp>
#include <boost/context/stack_traits.hpp>

#include <boost/job/memory.hpp>

#if defined(BOOST_USE_VALGRIND)
#include <valgrind/valgrind.h>
#endif

#ifdef BOOST_HAS_ABI_HEADERS
#  include BOOST_ABI_PREFIX
#endif

namespace boost {
namespace jobs {

template< typename traitsT >
class basic_numa_stack {
private:
    uint32_t        node_id_;
    std::size_t     size_;

public:
    typedef traitsT traits_type;

    basic_numa_stack( uint32_t node_id, std::size_t size = traits_type::default_size() ) :
        node_id_( node_id),
        size_( size) {
        BOOST_ASSERT( traits_type::minimum_size() <= size_);
        BOOST_ASSERT( traits_type::is_unbounded() || ( traits_type::maximum_size() >= size_) );
    }

    context::stack_context allocate() {
        void * vp = numa_alloc( size_, node_id_);
        if ( ! vp) throw std::bad_alloc();

        context::stack_context sctx;
        sctx.size = size_;
        sctx.sp = static_cast< char * >( vp) + sctx.size;
#if defined(BOOST_USE_VALGRIND)
        sctx.valgrind_stack_id = VALGRIND_STACK_REGISTER( sctx.sp, vp);
#endif
        return sctx;
    }

    void deallocate( context::stack_context & sctx) {
        BOOST_ASSERT( sctx.sp);
#if defined(BOOST_USE_WINFIBERS)
        BOOST_ASSERT( traits_type::minimum_size() <= sctx.size);
        BOOST_ASSERT( traits_type::is_unbounded() || ( traits_type::maximum_size() >= sctx.size) );
#endif

#if defined(BOOST_USE_VALGRIND)
        VALGRIND_STACK_DEREGISTER( sctx.valgrind_stack_id);
#endif

        void * vp = static_cast< char * >( sctx.sp) - sctx.size;
        numa_free( vp, sctx.size);
    }
};

typedef basic_numa_stack< context::stack_traits >  numa_stack;

}}

#ifdef BOOST_HAS_ABI_HEADERS
#  include BOOST_ABI_SUFFIX
#endif

#endif // BOOST_JOBS_NUMA_STACK_H
