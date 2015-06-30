
//          Copyright Oliver Kowalke 2015.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include "boost/job/detail/worker_thread.hpp"

#include <boost/assert.hpp>

#ifdef BOOST_HAS_ABI_HEADERS
# include BOOST_ABI_PREFIX
#endif

namespace boost {
namespace jobs {
namespace detail {

thread_local worker_thread *
worker_thread::instance_ = nullptr;

worker_thread::worker_thread() :
    use_count_( 0),
    shtdwn_( false),
    ntfy_(),
    topology_(),
    queue_(),
    thrd_() {
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
        // set termination flag
        shtdwn_ = true;
        // notify master fiber
        ntfy_.notify();
        // join worker thread
        thrd_.join();
    }
}

}}}

#ifdef BOOST_HAS_ABI_HEADERS
# include BOOST_ABI_SUFFIX
#endif
