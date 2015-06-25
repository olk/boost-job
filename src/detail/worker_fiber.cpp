
//          Copyright Oliver Kowalke 2015.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include "boost/job/detail/worker_fiber.hpp"

#include <algorithm> // std::move()
#include <vector>

#include <boost/fiber/fiber.hpp>

#ifdef BOOST_HAS_ABI_HEADERS
# include BOOST_ABI_PREFIX
#endif

namespace boost {
namespace jobs {
namespace detail {

worker_fiber::worker_fiber() :
    queue_( nullptr),
    fib_() {
}

worker_fiber::~worker_fiber() {
}

void
worker_fiber::worker_fn_( std::atomic_bool * shtdwn, fibers::unbounded_channel< worker::ptr_t > * queue) {
    while ( ! shtdwn->load() ) {
        try {
            // dequeue + process work items
            worker::ptr_t j = queue->value_pop();
            j->execute();
        } catch ( fibers::fiber_interrupted const&) {
            // do nothing; shtdwn should be set to true
        }
    }
}

worker_fiber::worker_fiber( std::atomic_bool * shtdwn, fibers::unbounded_channel< worker::ptr_t > * queue) :
    fib_( & worker_fiber::worker_fn_, this, shtdwn, queue) {
}

worker_fiber::worker_fiber( worker_fiber && other) :
    queue_( other.queue_),
    fib_( std::move( other.fib_) ) {
    other.queue_ = nullptr;
}

worker_fiber &
worker_fiber::operator=( worker_fiber && other) {
    if ( this == & other) {
        return * this;
    }
    queue_ = other.queue_;
    other.queue_ = nullptr;
    fib_ = std::move( other.fib_);
    return * this;
}

void
worker_fiber::interrupt() {
    fib_.interrupt();
}

void
worker_fiber::join() {
    if ( fib_.joinable() ) {
        try {
            fib_.join();
        } catch ( fibers::fiber_interrupted const&) {
        }
    }
}

}}}

#ifdef BOOST_HAS_ABI_HEADERS
# include BOOST_ABI_SUFFIX
#endif
