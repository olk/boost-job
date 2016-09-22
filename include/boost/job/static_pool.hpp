
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

#include <boost/assert.hpp>
#include <boost/config.hpp>
#include <boost/fiber/algo/algorithm.hpp>
#include <boost/fiber/context.hpp>
#include <boost/fiber/fiber.hpp>
#include <boost/fiber/operations.hpp>
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

template< std::size_t N >
class static_pool {
private:
    typedef std::array< fibers::fiber, N >  fiber_map_t;

    class algorithm : public fibers::algo::algorithm {
    private:
        typedef fibers::scheduler::ready_queue_t rqueue_t;

        rqueue_t                    rqueue_{};
        std::mutex                  mtx_{};
        std::condition_variable     cnd_{};
        bool                        flag_{ false };

    public:
        template< typename StackAllocator >
        algorithm( fiber_map_t & fibs, detail::queue & q, StackAllocator salloc) {
            // create worker fibers
            for ( std::size_t i = 0; i < N; ++i) {
                fibs[i] = std::move(
                    fibers::fiber( std::allocator_arg, salloc,
                                   [&q] () {
                                        while ( ! q.closed() ) {
                                            // dequeue work items
                                            detail::work::ptr_t w;
                                            if ( detail::queue_op_status::success != q.pop( w) ) {
                                                continue;
                                            }
                                            // process work items
                                            w->execute();
                                        }
                                    }));
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
    static_pool() = default;

    template< typename StackAllocator >
    void operator()( StackAllocator salloc,
                     detail::queue & queue,
                     detail::rendezvous & rdzv) {
        std::array< fibers::fiber, N > fibs;
        // install scheduling-algorithm for dynamic allocation of fibers
        fibers::use_scheduling_algorithm< algorithm >( fibs, std::ref( queue), salloc);
        // wait for termination notification
        rdzv.wait();
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
