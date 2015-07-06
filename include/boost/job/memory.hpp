
//          Copyright Oliver Kowalke 2015.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_JOBS_MEMORY_H
#define BOOST_JOBS_MEMORY_H

#include <cstddef>
#include <cstdint>
#include <limits>
#include <memory>

#include <boost/config.hpp>

#include <boost/job/detail/config.hpp>

#ifdef BOOST_HAS_ABI_HEADERS
# include BOOST_ABI_PREFIX
#endif

namespace boost {
namespace jobs {

#if defined(BOOST_JOBS_HAS_NUMA_ALLOC)
BOOST_JOBS_DECL
void * numa_alloc( std::size_t, uint32_t);

BOOST_JOBS_DECL
void numa_free( void *, std::size_t);
#endif

template< typename T >
class allocator {
private:
    uint32_t            node_id_;

public:
    template< typename U >
    friend class allocator;

    // type definitions
    typedef T               value_type;
    typedef T           *   pointer;
    typedef T       const*  const_pointer;
    typedef T           &   reference;
    typedef T       const&  const_reference;
    typedef std::size_t     size_type;
    typedef std::ptrdiff_t  difference_type;

    // rebind allocator to type U
    template< typename U >
    struct rebind {
        typedef allocator< U >  other;
    };

    allocator( uint32_t node_id) throw() :
        node_id_( node_id) {
    }

    allocator( allocator const& other) throw() :
        node_id_( other.node_id_) {
    }

    template< typename U >
    allocator( allocator< U > const& other) throw() :
        node_id_( other.node_id_) {
    }

    ~allocator() = default;

    // return address of values
    pointer address( reference value) const {
        return & value;
    }
    const_pointer address( const_reference value) const {
        return & value;
    }

    // return maximum number of elements that can be allocated
    size_type max_size() const throw() {
        return std::numeric_limits< std::size_t >::max() / sizeof( T);
    }

    // allocate but don't initialize num elements of type T
    pointer allocate( size_type num, const void * = 0) {
#if defined(BOOST_JOBS_HAS_NUMA_ALLOC)
        return ( pointer) numa_alloc( num * sizeof( T), node_id_);
#else
        return ( pointer)( ::operator new( num * sizeof( T) ) );
#endif
    }

    // initialize elements of allocated storage p with value value
    void construct( pointer p, T const& value) {
        // initialize memory with placement new
        new ( ( void*)p) T( value);
    }

    // destroy elements of initialized storage p
    void destroy( pointer p) {
        // destroy objects by calling their destructor
        p->~T();
    }

    // deallocate storage p of deleted elements
    void deallocate( pointer p, size_type num) {
#if defined(BOOST_JOBS_HAS_NUMA_ALLOC)
        numa_free( ( void * )p, num * sizeof( T) );
#else
        ::operator delete( ( void * )p);
#endif
    }
};

// return that all specializations of this allocator are interchangeable
template< typename T1, typename T2 >
bool operator==( allocator< T1 > const& l, allocator< T2 > const& r) throw() {
    return l.node_id_ == r.node_id_;
}
template< typename T1, typename T2 >
bool operator!=( allocator< T1 > const& l, allocator< T2 > const& r) throw() {
    return l.node_id_ != r.node_id_;
}

}}

#ifdef BOOST_HAS_ABI_HEADERS
# include BOOST_ABI_SUFFIX
#endif

#endif // BOOST_JOBS_MEMORY_H
