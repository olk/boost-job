
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
#include <boost/fiber/algo/algorithm.hpp>
#include <boost/fiber/context.hpp>
#include <boost/fiber/fiber.hpp>
#include <boost/fiber/operations.hpp>
#include <boost/fiber/scheduler.hpp>

#include <boost/job/detail/config.hpp>
#include <boost/job/detail/queue.hpp>
#include <boost/job/detail/rendezvous.hpp>

#ifdef BOOST_HAS_ABI_HEADERS
# include BOOST_ABI_PREFIX
#endif

namespace boost {
namespace jobs {

template< std::size_t Min, std::size_t Max >
class dynamic_pool {
private:
    class worker_fiber {
    private:
        fibers::fiber       fib_;
        bool                terminated_;

        template< typename StackAllocator, typename WorkerMap >
        void worker_fn_( StackAllocator salloc,
                         detail::queue & queue,
                         WorkerMap & fibs,
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
                // process work item
                w->execute();
                // mark worker fiber for detaching if:
                // - queue of work items is empty
                // - more than Min worker fibers are spawned
                if ( queue.empty() && Min < ecounter) {
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

        template< typename StackAllocator, typename WorkerMap >
        worker_fiber( StackAllocator salloc,
                      detail::queue & queue,
                      WorkerMap & fibs,
                      std::size_t & ecounter,
                      std::size_t & wcounter) :
            fib_( std::allocator_arg, salloc,
                  & worker_fiber::worker_fn_< StackAllocator, WorkerMap >, this,
                  salloc, std::ref( queue), std::ref( fibs), std::ref( ecounter), std::ref( wcounter) ),
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

        void join() {
            if ( fib_.joinable() ) {
                fib_.join();
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

    template< typename StackAllocator >
    class algorithm : public fibers::algo::algorithm {
    private:
        typedef fibers::scheduler::ready_queue_t rqueue_t;

        rqueue_t                    rqueue_{};
        std::mutex                  mtx_{};
        std::condition_variable     cnd_{};
        bool                        flag_{ false };
        fiber_map_t             &   fibs_;
        detail::queue           &   queue_;
        StackAllocator              salloc_;
        std::size_t                 ecounter_{ 0 };
        std::size_t                 wcounter_{ 0 };

        void spawn_fiber_() {
            worker_fiber f( salloc_, queue_, fibs_, ecounter_, wcounter_);
            fibers::fiber::id id( f.get_id() );
            fibs_[id] = std::move( f);
        }

    public:
        algorithm( fiber_map_t & fibs, detail::queue & q, StackAllocator salloc) :
            fibs_{ fibs },
            queue_{ q },
            salloc_{ salloc } {
            // create Min worker fibers
            for ( std::size_t i = 0; i < Min; ++i) {
                spawn_fiber_();
            }
        }

        algorithm( algorithm const&) = delete;
        algorithm & operator=( algorithm const&) = delete;

        void awakened( fibers::context * ctx) noexcept {
            BOOST_ASSERT( nullptr != ctx);

            BOOST_ASSERT( ! ctx->ready_is_linked() );
            ctx->ready_link( rqueue_);
        }

        fibers::context * pick_next() noexcept {
            fibers::context * victim{ nullptr };
            if ( ! rqueue_.empty() ) {
                victim = & rqueue_.front();
                rqueue_.pop_front();
                BOOST_ASSERT( nullptr != victim);
                BOOST_ASSERT( ! victim->ready_is_linked() );
            }
            return victim;
        }

        bool has_ready_fibers() const noexcept {
            return ! rqueue_.empty();
        }

        void suspend_until( std::chrono::steady_clock::time_point const& time_point) noexcept {
            // detach + erase terminated worker fibers
            for ( typename fiber_map_t::value_type & v : fibs_) {
                if ( v.second.terminated() ) {
                    v.second.join();
                }
            }
            if ( ! queue_.closed() && Max > ecounter_ && ! queue_.empty() && 0 == wcounter_) {
                // spawn new fiber
                spawn_fiber_();
                return;
            } 
            if ( (std::chrono::steady_clock::time_point::max)() == time_point) {
                std::unique_lock< std::mutex > lk( mtx_);
                cnd_.wait( lk, [&](){ return flag_; });
                flag_ = false;
            } else {
                std::unique_lock< std::mutex > lk( mtx_);
                cnd_.wait_until( lk, time_point, [&](){ return flag_; });
                flag_ = false;
            }
        }

        void notify() noexcept {
            std::unique_lock< std::mutex > lk( mtx_);
            flag_ = true;
            lk.unlock();
            cnd_.notify_all();
        }
    };

public:
    dynamic_pool() = default;

    template< typename StackAllocator >
    void operator()( StackAllocator salloc,
                     detail::queue & queue,
                     detail::rendezvous & rdzv) {
        fiber_map_t fibs;
        // install scheduling-algorithm for dynamic allocation of fibers
        fibers::use_scheduling_algorithm< algorithm< StackAllocator > >( fibs, std::ref( queue), salloc);
        // wait for termination notification
        rdzv.wait();
        // join worker fibers
        for ( typename fiber_map_t::value_type & v : fibs) {
            if ( v.second.terminated() ) {
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
