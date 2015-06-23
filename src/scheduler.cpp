
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

scheduler::scheduler( std::vector< topo_t > const& topology) :
    topology_( topology),
    worker_threads_( std::max_element(
                        topology.begin(),
                        topology.end(),
                        [](topo_t const& l,topo_t const& r){ return l.cpu_id < r.cpu_id; })->cpu_id
                     + 1) {
    for ( topo_t & topo : topology_) {
        worker_threads_[topo.cpu_id] = new  detail::worker_thread( topo);
    }
}

void
scheduler::shutdown() {
    for ( detail::worker_thread::ptr_t w : worker_threads_) {
        w->shutdown();
    }
    worker_threads_.clear();
}

}}

#ifdef BOOST_HAS_ABI_HEADERS
# include BOOST_ABI_SUFFIX
#endif
