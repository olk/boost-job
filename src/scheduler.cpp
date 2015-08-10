
//          Copyright Oliver Kowalke 2015.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include "boost/job/scheduler.hpp"

#include <algorithm> // std::move()

#ifdef BOOST_HAS_ABI_HEADERS
# include BOOST_ABI_PREFIX
#endif

namespace boost {
namespace jobs {

scheduler::~scheduler() noexcept {
    try {
        shutdown();
    } catch ( ... ) {
    }
}

void
scheduler::shutdown() {
    // shutdown worker threads
    for ( auto p : topology_) {
        worker_threads_[p.first]->shutdown();
    }
    worker_threads_.clear();
}

}}

#ifdef BOOST_HAS_ABI_HEADERS
# include BOOST_ABI_SUFFIX
#endif
