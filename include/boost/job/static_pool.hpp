
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
    class BOOST_JOBS_DECL worker_fiber {
    private:
        detail::queue   *   queue_;
        fibers::fiber       fib_;

        void worker_fn_( std::atomic_bool * shtdwn, detail::queue * q) {
            while ( ! shtdwn->load() ) {
                try {
                    // dequeue + process work items
                    q->value_pop()->execute();
                } catch ( fibers::fiber_interrupted const&) {
                    // do nothing; shtdwn should be set to true
                }
            }
        }

    public:
        worker_fiber() :
            queue_( nullptr),
            fib_() {
        }
        
        template< typename StackAllocator >
        worker_fiber( StackAllocator salloc, std::atomic_bool * shtdwn, detail::queue * q) :
            fib_( std::allocator_arg, salloc, & worker_fiber::worker_fn_, this, shtdwn, q) {
        }

        worker_fiber( worker_fiber && other) :
            queue_( other.queue_),
            fib_( std::move( other.fib_) ) {
            other.queue_ = nullptr;
        }

        worker_fiber & operator=( worker_fiber && other) {
            if ( this == & other) {
                return * this;
            }
            queue_ = other.queue_;
            other.queue_ = nullptr;
            fib_ = std::move( other.fib_);
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
    };

public:
    static_pool() = default;

    template< typename StackAllocator > 
    void operator()( StackAllocator salloc, std::atomic_bool * shtdwn,
                     detail::queue * q, detail::rendezvous * ntfy) {
        std::array< worker_fiber, N > fibs;
        // create worker fibers
        for ( std::size_t i = 0; i < N; ++i) {
            fibs[i] = std::move( worker_fiber( salloc, shtdwn, q) );
        }

        // wait for termination notification
        ntfy->wait();

        // interrupt worker fibers
        for ( worker_fiber & f : fibs) {
            f.interrupt();
        }

        // join worker fibers
        for ( worker_fiber & f : fibs) {
            f.join();
        }
    }
};

}}

#ifdef BOOST_HAS_ABI_HEADERS
# include BOOST_ABI_SUFFIX
#endif

#endif // BOOST_JOBS_STATIC_POOL_H
