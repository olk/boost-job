
//          Copyright Oliver Kowalke 2015.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_JOBS_DYNAMIC_POOL_H
#define BOOST_JOBS_DYNAMIC_POOL_H

#include <atomic>
#include <cstddef>
#include <map>
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

template< std::size_t Min, std::size_t Max, std::size_t N = 1 >
class dynamic_pool {
private:
    class worker_fiber {
    private:
        fibers::fiber       fib_;
        bool                terminated_;

        template< typename StackAllocator, typename WorkerMap >
        void worker_fn_( StackAllocator salloc,
                         detail::queue * q,
                         WorkerMap * fibs,
                         std::size_t * spawned) {
            ++( * spawned);
            while ( ! this_fiber::interruption_requested() && ! q->closed() ) {
                try {
                    // dequeue work items
                    detail::work::ptr_t w;
                    if ( detail::queue_op_status::success != q->pop( w) ) {
                        continue;
                    }
                    // spawn one new worker fiber if:
                    // - Max worker fibers are not reached
                    // - work queue is not empty OR fiber scheduler has no more
                    //   than `N` fiber ready to run
                    // current fiber might be blocked (waiting/suspended)
                    // so that at least one worker fiber is able to dequeue
                    // a new work item from the queue
                    if ( Max > * spawned && ( ! q->empty() || N >= fibers::ready_fibers() ) ) {
                        worker_fiber f( salloc, q, fibs, spawned);
                        fibers::fiber::id id( f.get_id() );
                        ( * fibs)[id] = std::move( f);
                    }
                    // process work item
                    w->execute();
                    // detach + erase terminated worker fibers
                    for ( typename fiber_map_t::value_type & v : ( * fibs) ) {
                        if ( v.second.terminated() ) {
                            v.second.detach();
                        }
                    }
                    // mark worker fiber for detaching if:
                    // - queue of work items is empty
                    // - more than Min worker fibers are spawned
                    // - fiber scheduler has fibers ready to run
                    if ( q->empty() &&
                         Min < * spawned &&
                         0 < fibers::ready_fibers() ) {
                        // releases allocated resources
                        fibs->at( this_fiber::get_id() ).set_terminated();
                        break;
                    }
                } catch ( fibers::fiber_interrupted const&) {
                }
            }
            --( * spawned);
        }

    public:
        worker_fiber() :
            fib_(), terminated_( false) {
        }

        template< typename StackAllocator, typename WorkerMap >
        worker_fiber( StackAllocator salloc,
                      detail::queue * q,
                      WorkerMap * fibs,
                      std::size_t * spawned) :
            fib_( std::allocator_arg, salloc,
                  & worker_fiber::worker_fn_< StackAllocator, WorkerMap >, this,
                  salloc, q, fibs, spawned),
            terminated_( false) {
        }

        worker_fiber( worker_fiber && other) :
            fib_( std::move( other.fib_) ),
            terminated_( other.terminated_) {
            other.terminated_ = false;
        }

        worker_fiber & operator=( worker_fiber && other) {
            if ( this == & other) {
                return * this;
            }
            fib_ = std::move( other.fib_);
            terminated_ = other.terminated_;
            other.terminated_ = false;
            return * this;
        }

        worker_fiber( worker_fiber const&) = delete;

        worker_fiber & operator=( worker_fiber const&) = delete;

        void interrupt() noexcept {
            fib_.interrupt();
        }

        void join() {
            if ( fib_.joinable() ) {
                try {
                    fib_.join();
                } catch ( fibers::fiber_interrupted const&) {
                }
            }
        }

        fibers::fiber::id get_id() const noexcept {
            return fib_.get_id();
        }

        void detach() {
            if ( fib_.joinable() ) {
                fib_.detach();
            }
        }

        void set_terminated() noexcept {
            terminated_ = true;
        }

        bool terminated() const noexcept {
            return terminated_;
        }
    };

    typedef std::map< fibers::fiber::id, worker_fiber > fiber_map_t;

public:
    dynamic_pool() = default;

    template< typename StackAllocator >
    void operator()( StackAllocator salloc,
                     detail::queue * q,
                     detail::rendezvous * rdzv) {
        fiber_map_t fibs;
        std::size_t spawned = 0;
        // create Min worker fibers
        for ( std::size_t i = 0; i < Min; ++i) {
            worker_fiber f( salloc, q, & fibs, & spawned);
            fibers::fiber::id id( f.get_id() );
            fibs[id] = std::move( f);
        }
        // wait for termination notification
        rdzv->wait();
        // close queue
        q->close();
        // interrupt worker fibers
        for ( typename fiber_map_t::value_type & v : fibs) {
            if ( ! v.second.terminated() ) {
                v.second.interrupt();
            }
        }
        // join worker fibers
        for ( typename fiber_map_t::value_type & v : fibs) {
            v.second.join();
        }
    }
};

}}

#ifdef BOOST_HAS_ABI_HEADERS
# include BOOST_ABI_SUFFIX
#endif

#endif // BOOST_JOBS_DYNAMIC_POOL_H
