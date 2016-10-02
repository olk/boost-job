
//          Copyright Oliver Kowalke 2015.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_JOBS_WORKER_POLICY_H
#define BOOST_JOBS_WORKER_POLICY_H

#include <cstddef>
#include <vector>

#include <boost/config.hpp>

#include <boost/job/detail/config.hpp>
#include <boost/job/detail/worker_thread.hpp>
#include <boost/job/topology.hpp>

#ifdef BOOST_HAS_ABI_HEADERS
# include BOOST_ABI_PREFIX
#endif

namespace boost {
namespace jobs {

class scheduler;

class pin_to_cpu {
private:
    std::size_t     worker_per_cpu_;

public:
    pin_to_cpu( std::size_t worker_per_cpu = 1) :
        worker_per_cpu_{ worker_per_cpu } {
    }

    template< typename FiberPool >
    void operator()( std::vector< topo_t > const& topology,
                     FiberPool && pool,
                     scheduler * sched,
                     std::vector< detail::worker_thread::ptr_t > & worker_threads) const {
            // only for given CPUs allocate worker threads
            for ( auto t : topology) {
                for ( std::size_t i = 0; i < worker_per_cpu_; ++i) {
                    worker_threads[t.processor_id] = detail::worker_thread::create(
                            t, std::forward< FiberPool >( pool), numa_fixedsize( t.node_id), sched);
                }
            }
    }

};

}}

#ifdef BOOST_HAS_ABI_HEADERS
# include BOOST_ABI_SUFFIX
#endif

#endif // BOOST_JOBS_WORKER_POLICY_H
