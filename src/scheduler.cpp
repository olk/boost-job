
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

scheduler::scheduler() :
    topology_(),
    worker_threads_() {
}

scheduler::~scheduler() {
    try {
        shutdown();
    } catch ( ... ) {
    }
}

void
scheduler::shutdown() {
    // shutdown worker threads
    for ( topo_t & topo : topology_) {
        worker_threads_[topo.cpu_id]->shutdown();
    }
    worker_threads_.clear();
}

}}

#ifdef BOOST_HAS_ABI_HEADERS
# include BOOST_ABI_SUFFIX
#endif
