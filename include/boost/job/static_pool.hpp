
//          Copyright Oliver Kowalke 2015.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_JOBS_STATIC_POOL_H
#define BOOST_JOBS_STATIC_POOL_H

#include <array>
#include <atomic>
#include <cstddef>
#include <memory>

#include <boost/config.hpp>
#include <boost/fiber/fiber.hpp>
#include <boost/fiber/operations.hpp>

#include <boost/job/detail/config.hpp>
#include <boost/job/detail/queue.hpp>
#include <boost/job/detail/rendezvous.hpp>

#ifdef BOOST_HAS_ABI_HEADERS
# include BOOST_ABI_PREFIX
#endif

namespace boost {
namespace jobs {

template< std::size_t N >
struct static_pool {
    static_pool() = default;

    template< typename StackAllocator >
    void operator()( StackAllocator salloc,
                     detail::queue * q,
                     detail::rendezvous * rdzv) {
        std::array< fibers::fiber, N > fibs;
        // create worker fibers
        for ( std::size_t i = 0; i < N; ++i) {
            fibs[i] = std::move(
                fibers::fiber( std::allocator_arg, salloc,
                               [q] () {
                                    while ( ! q->closed() ) {
                                        // dequeue work items
                                        detail::work::ptr_t w;
                                        if ( detail::queue_op_status::success != q->pop( w) ) {
                                            continue;
                                        }
                                        // process work items
                                        w->execute();
                                    }
                                }));
        }
        // wait for termination notification
        rdzv->wait();
        // close queue
        q->close();
        // join worker fibers
        for ( fibers::fiber & f : fibs) {
            if ( f.joinable() ) {
                f.join();
            }
        }
    }
};

}}

#ifdef BOOST_HAS_ABI_HEADERS
# include BOOST_ABI_SUFFIX
#endif

#endif // BOOST_JOBS_STATIC_POOL_H
