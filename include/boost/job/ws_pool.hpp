
//          Copyright Oliver Kowalke 2015.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_JOBS_WS_POOL_H
#define BOOST_JOBS_WS_POOL_H

#include <algorithm>
#include <array>
#include <atomic>
#include <chrono>
#include <cstddef>
#include <iterator>
#include <memory>
#include <set>
#include <vector>

#include <boost/config.hpp>
#include <boost/fiber/fiber.hpp>
#include <boost/fiber/operations.hpp>

#include <boost/job/detail/config.hpp>
#include <boost/job/detail/queue.hpp>
#include <boost/job/detail/rendezvous.hpp>
#include <boost/job/operation.hpp>
#include <boost/job/scheduler.hpp>
#include <boost/job/topology.hpp>

#ifdef BOOST_HAS_ABI_HEADERS
# include BOOST_ABI_PREFIX
#endif

namespace boost {
namespace jobs {

template< std::size_t N >
class ws_pool {
private:
    std::chrono::high_resolution_clock::duration    wait_interval_;

    std::vector< detail::queue * > steal_from_() {
        std::vector< detail::queue * > queues;
        topo_t topo = this_worker::topology();
        // FIXME: L1 cache == hyper-threading
        // sharing L2 cache
        std::set< uint32_t > l2 = topo.l2_shared_with;
        for ( uint32_t processor_id : l2) {
            detail::queue * q( this_worker::queue_at( processor_id) );
            if ( nullptr != q) {
               queues.push_back( q);
            } 
        }
        // sharing L3 cache
        std::set< uint32_t > l3;
        std::set_difference(
                topo.l3_shared_with.begin(),
                topo.l3_shared_with.end(),
                topo.l2_shared_with.begin(),
                topo.l2_shared_with.end(),
                std::inserter( l3, l3.begin() ) );
        for ( uint32_t processor_id : l3) {
            detail::queue * q( this_worker::queue_at( processor_id) );
            if ( nullptr != q) {
               queues.push_back( q);
            } 
        }
        // all other logical processors
        std::vector< topo_t > v( cpu_topology() );
        std::set< uint32_t > all;
        for ( topo_t t : v) {
            if ( l2.end() == l2.find( t.processor_id) &&
                 l3.end() == l3.find( t.processor_id) ) {
                all.insert( t.processor_id);
            }
        }
        for ( uint32_t processor_id : all) {
            detail::queue * q( this_worker::queue_at( processor_id) );
            if ( nullptr != q) {
               queues.push_back( q);
            } 
        }
        return queues;
    }

public:
    template< typename Rep, typename Period >
    ws_pool( std::chrono::duration< Rep, Period > const& wait_interval) noexcept :
        wait_interval_( wait_interval) {
    }

    ws_pool() noexcept :
        ws_pool( 
            std::chrono::microseconds(20) ) {
    }

    template< typename StackAllocator >
    void operator()( StackAllocator salloc,
                     detail::queue * q,
                     detail::rendezvous * rdzv) {
        // local queues of logical processors sharing
        // L2 and L3 cache with logical processor this
        // worker thread is running on
        // Note: queues of logical processors sharing
        // L2 cache are put to front
        std::vector< detail::queue * > queues( steal_from_() );
        // create worker fibers
        std::array< fibers::fiber, N > fibs;
        for ( std::size_t i = 0; i < N; ++i) {
            fibs[i] = std::move(
                fibers::fiber( std::allocator_arg, salloc,
                               [q,queues,wait_interval=wait_interval_] () {
                                    while ( ! this_fiber::interruption_requested() && ! q->closed() ) {
                                        try {
                                            // dequeued work item
                                            detail::work::ptr_t w;
                                            // dequeue work from local queue
                                            if ( detail::queue_op_status::success == q->try_pop( w) ) {
                                                // process work item
                                                w->execute();
                                            } else {
                                                // work-stealing
                                                // dequeue from queue of another logical processor
                                                for ( detail::queue * steal_from : queues) {
                                                    BOOST_ASSERT( nullptr != steal_from);
                                                    if ( detail::queue_op_status::success == steal_from->try_pop( w) ) {
                                                        // process work item
                                                        w->execute();
                                                        break;
                                                    }
                                                }
                                                // no work dequeued from queue of another logical processor
                                                // do a blocking pop() on local queue for a certain time
                                                if ( detail::queue_op_status::success == q->pop_wait_for( w, wait_interval) ) {
                                                    // process work item
                                                    w->execute();
                                                    break;
                                                }
                                            }
                                        } catch ( fibers::fiber_interrupted const&) {
                                        }
                                    }
                                }));
        }
        // wait for termination notification
        rdzv->wait();
        // close queue
        q->close();
        // interrupt worker-fibers
        for ( fibers::fiber & f : fibs) {
            f.interrupt();
        }
        // join worker fibers
        for ( fibers::fiber & f : fibs) {
            if ( f.joinable() ) {
                try {
                    f.join();
                } catch ( fibers::fiber_interrupted const&) {
                }
            }
        }
    }
};

}}

#ifdef BOOST_HAS_ABI_HEADERS
# include BOOST_ABI_SUFFIX
#endif

#endif // BOOST_JOBS_WS_POOL_H
