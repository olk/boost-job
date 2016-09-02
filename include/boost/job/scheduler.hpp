
//          Copyright Oliver Kowalke 2015.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_JOBS_SCHEDULER_H
#define BOOST_JOBS_SCHEDULER_H

#include <cstddef>
#include <functional>
#include <future>
#include <map>
#include <utility> // std::forward()
#include <vector>

#include <boost/config.hpp>
#include <boost/fiber/future.hpp>

#include <boost/job/detail/config.hpp>
#include <boost/job/detail/work.hpp>
#include <boost/job/detail/worker_thread.hpp>
#include <boost/job/memory.hpp>
#include <boost/job/numa_fixedsize_stack.hpp>
#include <boost/job/stack.hpp>
#include <boost/job/static_pool.hpp>
#include <boost/job/topology.hpp>

#ifdef BOOST_HAS_ABI_HEADERS
# include BOOST_ABI_PREFIX
#endif

namespace boost {
namespace jobs {

class BOOST_JOBS_DECL scheduler {
private:
    friend class detail::worker_thread;

    std::map< uint32_t, topo_t >                topology_;
    std::vector< detail::worker_thread::ptr_t > worker_threads_;

    static std::map< uint32_t, topo_t >
    create_topology_map( std::vector< topo_t > const& topology) {
        std::map< uint32_t, topo_t > map;
        for ( auto t : topology) {
            map[t.processor_id] = t;
        }
        return map;
    }

    detail::worker_thread::ptr_t worker_at( uint32_t processor_id) {
        return worker_threads_[processor_id];
    }

public:
    template< typename FiberPool, typename StackAllocator >
    scheduler( std::vector< topo_t > const& topology,
               FiberPool && pool,
               StackAllocator salloc) :
        topology_( create_topology_map( topology) ),
        // hold max(CPU-IDs) worker threads
        worker_threads_( std::max_element(
                    topology.begin(),
                    topology.end(),
                    [](topo_t const& l,topo_t const& r){ return l.processor_id < r.processor_id; })->processor_id
                + 1) {
        BOOST_ASSERT( topology.size() == topology_.size() );
        // only for given CPUs allocate worker threads
        for ( auto t : topology) {
            // worker-threads are allocated on NUMA nodes
            // to which the logical processors belongs
            worker_threads_[t.processor_id] = detail::worker_thread::create(
                    t, std::forward< FiberPool >( pool), salloc, this);
        }
    }

    template< typename FiberPool >
    scheduler( std::vector< topo_t > const& topology,
               FiberPool && pool) :
        topology_( create_topology_map( topology) ),
        // hold max(CPU-IDs)
        worker_threads_( std::max_element(
                    topology.begin(),
                    topology.end(),
                    [](topo_t const& l,topo_t const& r){ return l.processor_id < r.processor_id; })->processor_id
                + 1) {
        BOOST_ASSERT( topology.size() == topology_.size() );
        // only for given CPUs allocate worker threads
        for ( auto t : topology) {
            worker_threads_[t.processor_id] = detail::worker_thread::create(
                    t, std::forward< FiberPool >( pool), numa_fixedsize( t.node_id), this);
        }
    }

    scheduler( std::vector< topo_t > const& topology) :
        scheduler( topology, static_pool< 64 >() ) {
    }

    ~scheduler() noexcept;

    scheduler( scheduler const&) = delete;

    scheduler & operator=( scheduler const&) = delete;

    template< typename Allocator, typename Fn, typename ... Args >
    std::future<
        typename std::result_of<
            typename std::decay< Fn >::type(typename std::decay< Args >::type ... )
        >::type
    >
    submit_preempt( std::allocator_arg_t, Allocator alloc,
                    uint32_t cpuid, Fn && fn, Args && ... args) {
        return worker_threads_[cpuid]->submit_preempt(
            std::allocator_arg,
            alloc,
            std::forward< Fn >( fn), std::forward< Args >( args) ... );
    }

    template< typename Fn, typename ... Args >
    std::future<
        typename std::result_of<
            typename std::decay< Fn >::type(typename std::decay< Args >::type ... )
        >::type
    >
    submit_preempt( uint32_t cpuid, Fn && fn, Args && ... args) {
        return worker_threads_[cpuid]->submit_preempt(
            std::allocator_arg,
            numa_allocator< detail::work >( topology_[cpuid].node_id),
            std::forward< Fn >( fn), std::forward< Args >( args) ... );
    }

    template< typename Allocator, typename Fn, typename ... Args >
    fibers::future<
        typename std::result_of<
            typename std::decay< Fn >::type(typename std::decay< Args >::type ... )
        >::type
    >
    submit_coop( std::allocator_arg_t, Allocator alloc,
                 uint32_t cpuid, Fn && fn, Args && ... args) {
        return worker_threads_[cpuid]->submit_coop(
            std::allocator_arg,
            alloc,
            std::forward< Fn >( fn), std::forward< Args >( args) ... );
    }

    template< typename Fn, typename ... Args >
    fibers::future<
        typename std::result_of<
            typename std::decay< Fn >::type(typename std::decay< Args >::type ... )
        >::type
    >
    submit_coop( uint32_t cpuid, Fn && fn, Args && ... args) {
        return worker_threads_[cpuid]->submit_coop(
            std::allocator_arg,
            numa_allocator< detail::work >( topology_[cpuid].node_id),
            std::forward< Fn >( fn), std::forward< Args >( args) ... );
    }

    void shutdown();
};

}}

#ifdef BOOST_HAS_ABI_HEADERS
# include BOOST_ABI_SUFFIX
#endif

#endif // BOOST_JOBS_SCHEDULER_H
