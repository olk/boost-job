
//          Copyright Oliver Kowalke 2015.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include "boost/job/detail/worker_thread.hpp"

#include <algorithm> // std::move()
#include <vector>

#include <boost/fiber/fiber.hpp>

#ifdef BOOST_HAS_ABI_HEADERS
# include BOOST_ABI_PREFIX
#endif

namespace boost {
namespace jobs {
namespace detail {

worker_thread::worker_thread() :
    use_count_( 0),
    topology_(),
    mtx_(),
    fibers_(),
    thrd_() {
}

worker_thread::~worker_thread() {
    try {
        shutdown();
    } catch ( ... ) {
    }
}

void
worker_thread::worker_fn_( std::atomic_bool * shtdwn) {
    for ( int i = 0; i < 64; ++i) {
        fibers_.push_back( std::move( worker_fiber( shtdwn) ) );
    }

    std::unique_lock< std::mutex > lk( mtx_);
    for ( worker_fiber & f : fibers_) {
        f.join();
    }
    fibers_.clear();
}

worker_thread::worker_thread( topo_t const& topology, std::atomic_bool & shtdwn) :
    use_count_( 0),
    topology_( topology),
    mtx_(),
    fibers_(),
    thrd_( & worker_thread::worker_fn_, this, & shtdwn) {
}

void
worker_thread::shutdown() {
    std::unique_lock< std::mutex > lk( mtx_);
    for ( worker_fiber & f : fibers_) {
        f.interrupt();
    }
    lk.unlock();
    if ( thrd_.joinable() ) {
        thrd_.join();
    }
}

}}}

#ifdef BOOST_HAS_ABI_HEADERS
# include BOOST_ABI_SUFFIX
#endif
