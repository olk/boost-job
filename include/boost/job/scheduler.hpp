
//          Copyright Oliver Kowalke 2015.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_JOBS_SCHEDULER_H
#define BOOST_JOBS_SCHEDULER_H

#include <cstddef>
#include <functional>
#include <future>
#include <memory>
#include <type_traits> // std::result_of
#include <utility> // std::forward()
#include <vector>

#include <boost/config.hpp>
#include <boost/fiber/future.hpp>

#include <boost/job/detail/config.hpp>
#include <boost/job/detail/work.hpp>
#include <boost/job/detail/worker_thread.hpp>
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
    std::vector< topo_t >                       topology_;
    std::vector< detail::worker_thread::ptr_t > worker_threads_;

public:
    scheduler();

    ~scheduler();

    template< typename FiberPool, typename StackAllocator >
    scheduler( std::vector< topo_t > const& topology,
               FiberPool && pool,
               StackAllocator salloc) :
        topology_( topology),
        // hold max(CPU-IDs)
        worker_threads_( std::max_element(
                    topology.begin(),
                    topology.end(),
                    [](topo_t const& l,topo_t const& r){ return l.cpu_id < r.cpu_id; })->cpu_id
                + 1) {
        // only for given CPUs allocate worker threads
        for ( topo_t & topo : topology_) {
            worker_threads_[topo.cpu_id] = detail::worker_thread::create( topo, std::forward< FiberPool >( pool), salloc);
        }
    }

    scheduler( std::vector< topo_t > const& topology) :
        scheduler( topology, static_pool< 64 >(), fixedsize_stack() ) {
    }

    scheduler( scheduler const&) = delete;

    scheduler & operator=( scheduler const&) = delete;

    template< typename Allocator, typename Fn, typename ... Args >
    std::future< typename std::result_of< Fn( Args ... ) >::type >
    submit_preempt( std::allocator_arg_t, Allocator alloc,
                    uint32_t cpuid, Fn && fn, Args && ... args) {
        return worker_threads_[cpuid]->submit_preempt(
            std::allocator_arg, alloc,
            std::forward< Fn >( fn), std::forward< Args >( args) ... );
    }

    template< typename Fn, typename ... Args >
    std::future< typename std::result_of< Fn( Args ... ) >::type >
    submit_preempt( uint32_t cpuid, Fn && fn, Args && ... args) {
        return submit_preempt( std::allocator_arg, std::allocator< detail::work >(), cpuid,
                               std::forward< Fn >( fn), std::forward< Args >( args) ...);
    }

    template< typename Allocator, typename Fn, typename ... Args >
    fibers::future< typename std::result_of< Fn( Args ... ) >::type >
    submit_coop( std::allocator_arg_t, Allocator alloc,
                uint32_t cpuid, Fn && fn, Args && ... args) {
        return worker_threads_[cpuid]->submit_coop(
            std::allocator_arg, alloc,
            std::forward< Fn >( fn), std::forward< Args >( args) ... );
    }

    template< typename Fn, typename ... Args >
    fibers::future< typename std::result_of< Fn( Args ... ) >::type >
    submit_coop( uint32_t cpuid, Fn && fn, Args && ... args) {
        return submit_coop( std::allocator_arg, std::allocator< detail::work >(), cpuid,
                            std::forward< Fn >( fn), std::forward< Args >( args) ...);
    }

    void shutdown();
};

}}

#ifdef BOOST_HAS_ABI_HEADERS
# include BOOST_ABI_SUFFIX
#endif

#endif // BOOST_JOBS_SCHEDULER_H
