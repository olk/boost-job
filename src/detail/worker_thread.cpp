
//          Copyright Oliver Kowalke 2015.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include "boost/job/detail/worker_thread.hpp"

#include <algorithm> // std::move()
#include <vector>

#include <boost/assert.hpp>
#include <boost/fiber/fiber.hpp>

#include "boost/job/detail/barrier.hpp"
#include "boost/job/pin.hpp"

#ifdef BOOST_HAS_ABI_HEADERS
# include BOOST_ABI_PREFIX
#endif

namespace boost {
namespace jobs {
namespace detail {

void
worker_thread::worker_fn_() {
    // pin thread to CPU
    pin_thread( topology_.cpu_id);

    int N = 64;
    // create worker fibers
    std::vector< worker_fiber > fibs( N);
    for ( int i = 0; i < N; ++i) {
        fibs[i] = std::move( worker_fiber( & shtdwn_) );
    }
    fibers_ = & fibs;

    // join fibers
    for ( worker_fiber & f : fibs) {
        f.join();
    }
}

worker_thread::worker_thread() :
    use_count_( 0),
    shtdwn_( false),
    topology_(),
    fibers_( nullptr),
    thrd_() {
}

worker_thread::worker_thread( topo_t const& topology) :
    use_count_( 0),
    shtdwn_( false),
    topology_( topology),
    fibers_(),
    thrd_( & worker_thread::worker_fn_, this) {
}

worker_thread::~worker_thread() {
    try {
        shutdown();
    } catch ( ... ) {
    }
}

void
worker_thread::shutdown() {
    if ( thrd_.joinable() ) {
        BOOST_ASSERT( ! shtdwn_);
        shtdwn_ = true;
        // TODO: interrupt worker fibers
        thrd_.join();
    }
}

}}}

#ifdef BOOST_HAS_ABI_HEADERS
# include BOOST_ABI_SUFFIX
#endif
