
//          Copyright Oliver Kowalke 2015.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_JOBS_DYNAMIC_POOL_H
#define BOOST_JOBS_DYNAMIC_POOL_H

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <cstddef>
#include <map>
#include <memory>
#include <mutex>

#include <boost/assert.hpp>
#include <boost/config.hpp>
#include <boost/fiber/fiber.hpp>

#include <boost/job/detail/config.hpp>
#include <boost/job/detail/queue.hpp>
#include <boost/job/detail/rendezvous.hpp>

#ifdef BOOST_HAS_ABI_HEADERS
# include BOOST_ABI_PREFIX
#endif

namespace boost {
namespace jobs {

template< std::size_t Min, std::size_t Max = 0 >
class dynamic_pool {
private:
    class worker_fiber {
    private:
        fibers::fiber       fib_;
        bool                terminated_;

        template< typename StackAllocator, typename FiberMap >
        void worker_fn_( StackAllocator salloc,
                         detail::queue & queue,
                         FiberMap & fibs,
                         std::size_t & ecounter,
                         std::size_t & wcounter) {
            ++ecounter;
            while ( ! queue.closed() ) {
                // dequeue work items
                detail::work::ptr_t w;
                ++wcounter;
                if ( detail::queue_op_status::success != queue.pop( w) ) {
                    --wcounter;
                    continue;
                }
                --wcounter;
                // spawn worker fiber if no worker fiber is waiting in queue
                if ( (0 == Max || Max > ecounter) && 0 == wcounter) {
                    worker_fiber f( salloc, queue, fibs, ecounter, wcounter);
                    fibers::fiber::id id( f.get_id() );
                    fibs[id] = std::move( f);
                }
                // process work item
                w->execute();
                // detach + erase terminated worker fibers
                for ( typename FiberMap::value_type & v : fibs) {
                    if ( v.second.terminated() && v.second.joinable() ) {
                        v.second.join();
                    }
                }
                // mark worker fiber for detaching if:
                // - queue of work items is empty
                // - more than Min worker fibers are spawned
                // - at least on worker fiber waits in queue::pop()
                if ( queue.empty() && Min < ecounter && 0 < wcounter) {
                    break;
                }
            }
            --ecounter;
            // releases allocated resources
            fibs.at( this_fiber::get_id() ).set_terminated();
        }

    public:
        worker_fiber() :
            fib_(), terminated_( false) {
        }

        template< typename StackAllocator, typename FiberMap >
        worker_fiber( StackAllocator salloc,
                      detail::queue & queue,
                      FiberMap & fibs,
                      std::size_t & ecounter,
                      std::size_t & wcounter) :
            fib_( std::allocator_arg, salloc,
                  & worker_fiber::worker_fn_< StackAllocator, FiberMap >, this,
                  salloc, std::ref( queue), std::ref( fibs), std::ref( ecounter), std::ref( wcounter) ),
            terminated_( false) {
        }

        worker_fiber( worker_fiber && other) :
            fib_( std::move( other.fib_) ),
            terminated_( other.terminated_) {
            other.terminated_ = false;
        }

        worker_fiber & operator=( worker_fiber && other) {
            if ( this != & other) {
                fib_ = std::move( other.fib_);
                terminated_ = other.terminated_;
                other.terminated_ = false;
            }
            return * this;
        }

        worker_fiber( worker_fiber const&) = delete;

        worker_fiber & operator=( worker_fiber const&) = delete;

        fibers::fiber::id get_id() const noexcept {
            return fib_.get_id();
        }

        bool joinable() const noexcept {
            return fib_.joinable();
        }

        void join() {
            fib_.join();
        }

        void detach() {
            fib_.detach();
        }

        void set_terminated() noexcept {
            terminated_ = true;
        }

        bool terminated() const noexcept {
            return terminated_;
        }
    };

public:
    typedef std::map< fibers::fiber::id, worker_fiber > fiber_map_t;

    dynamic_pool() = default;

    template< typename StackAllocator >
    void operator()( StackAllocator salloc,
                     detail::queue & queue,
                     detail::rendezvous & rdzv) {
        fiber_map_t fibs;
        std::size_t ecounter{ 0 }, wcounter{ 0 };
        // create Min worker fibers
        for ( std::size_t i = 0; i < Min; ++i) {
            worker_fiber f( salloc, queue, fibs, ecounter, wcounter);
            fibers::fiber::id id( f.get_id() );
            fibs[id] = std::move( f);
        }
        // wait for termination notification
        rdzv.wait();
        // join worker fibers
        for ( typename fiber_map_t::value_type & v : fibs) {
            if ( v.second.terminated() && v.second.joinable() ) {
                v.second.join();
            }
        }
    }
};

}}

#ifdef BOOST_HAS_ABI_HEADERS
# include BOOST_ABI_SUFFIX
#endif

#endif // BOOST_JOBS_DYNAMIC_POOL_H
