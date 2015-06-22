
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
    fib_() {
}

worker_fiber::~worker_fiber() {
}

void
worker_fiber::worker_fn_( std::atomic_bool * shtdwn) {
    while ( ! shtdwn) {
        // TODO: dequeue jobs from MPSC-queue and process jobs
    }
}

worker_fiber::worker_fiber( std::atomic_bool * shtdwn) :
    fib_( & worker_fiber::worker_fn_, this, shtdwn) {
}

worker_fiber::worker_fiber( worker_fiber && other) :
    fib_( std::move( other.fib_) ) {
}

worker_fiber &
worker_fiber::operator=( worker_fiber && other) {
    if ( this == & other) {
        return * this;
    }
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
