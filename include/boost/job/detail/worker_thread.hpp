
//          Copyright Oliver Kowalke 2015.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_JOBS_DETAIL_WORKER_THREAD_H
#define BOOST_JOBS_DETAIL_WORKER_THREAD_H

#include <algorithm> // std::move()
#include <cstddef>
#include <cstdint>
#include <future>
#include <memory>
#include <thread>
#include <type_traits> // std::result_of
#include <utility> // std::forward()
#include <vector>

#include <boost/assert.hpp>
#include <boost/config.hpp>
#include <boost/fiber/fiber.hpp>
#include <boost/fiber/future.hpp>
#include <boost/intrusive_ptr.hpp>

#include <boost/job/detail/config.hpp>
#include <boost/job/detail/pin.hpp>
#include <boost/job/detail/queue.hpp>
#include <boost/job/detail/rendezvous.hpp>
#include <boost/job/detail/work.hpp>
#include <boost/job/memory.hpp>
#include <boost/job/topology.hpp>

#ifdef BOOST_HAS_ABI_HEADERS
# include BOOST_ABI_PREFIX
#endif

namespace boost {
namespace jobs {

class scheduler;

namespace detail {

class BOOST_JOBS_DECL worker_thread {
private:
    thread_local static worker_thread   *   instance_;

    std::size_t                             use_count_;
    rendezvous                              rdzv_;
    topo_t                                  topology_;
    queue                                   queue_;
    std::thread                             thrd_;
    scheduler                           *   sched_;

    template< typename FiberPool, typename StackAllocator >
    void worker_fn_( FiberPool && pool, StackAllocator salloc) {
        // pin thread to CPU
        pin_thread( topology_.processor_id);
        // set static thread-local pointer
        instance_ = this;
        // master fiber (== worker thread) executes fiber pool
        pool( salloc, queue_, rdzv_);
    }

    template< typename FiberPool, typename StackAllocator >
    worker_thread( topo_t const& topology, FiberPool && pool, StackAllocator salloc, scheduler * sched) :
        use_count_( 0),
        rdzv_(),
        topology_( topology),
        queue_(),
        thrd_( & worker_thread::worker_fn_< FiberPool, StackAllocator >, this, std::forward< FiberPool >( pool), salloc),
        sched_( sched) {
    }

public:
    typedef intrusive_ptr< worker_thread >  ptr_t;

    static worker_thread * instance() noexcept {
        BOOST_ASSERT( nullptr != instance_);
        return instance_;
    }

    template< typename FiberPool, typename StackAllocator >
    static ptr_t create( topo_t const& topology, FiberPool && pool, StackAllocator salloc, scheduler * sched) {
        numa_allocator< worker_thread > alloc( topology.node_id);
        worker_thread * p = alloc.allocate( 1);
        return ptr_t( new ( p) worker_thread( topology, std::forward< FiberPool >( pool), salloc, sched) );
    }

    ~worker_thread() noexcept;

    worker_thread( worker_thread const&) = delete;

    worker_thread & operator=( worker_thread const&) = delete;

    void shutdown();

    topo_t topology() const noexcept {
        return topology_;
    }

    template< typename Allocator, typename Fn, typename ... Args >
    void
    submit( std::allocator_arg_t, Allocator alloc, Fn && fn, Args && ... args) {
        // enqueue work into MPSC-queue
        queue_.push( create_work(
            alloc,
            std::forward< Fn >( fn), std::forward< Args >( args) ... ) );
    }

    template< typename Allocator, typename Fn, typename ... Args >
    std::future<
        typename std::result_of<
            typename std::decay< Fn >::type(typename std::decay< Args >::type ... )
        >::type
    >
    submit_preempt( std::allocator_arg_t, Allocator alloc, Fn && fn, Args && ... args) {
        typedef typename std::result_of<
            typename std::decay< Fn >::type( typename std::decay< Args >::type ... )
        >::type     result_type;

        std::packaged_task< result_type( typename std::decay< Args >::type ... ) > pt(
                std::forward< Fn >( fn) );
        std::future< result_type > f( pt.get_future() );
        // enqueue work into MPSC-queue
        queue_.push( create_work(
            alloc,
            std::move( pt), std::forward< Args >( args) ... ) );
        return std::move( f);
    }

    template< typename Allocator, typename Fn, typename ... Args >
    fibers::future<
        typename std::result_of<
            typename std::decay< Fn >::type(typename std::decay< Args >::type ... )
        >::type
    >
    submit_coop( std::allocator_arg_t, Allocator alloc, Fn && fn, Args && ... args) {
        typedef typename std::result_of<
            typename std::decay< Fn >::type( typename std::decay< Args >::type ... )
        >::type     result_type;

        fibers::packaged_task< result_type( typename std::decay< Args >::type ... ) > pt(
                std::forward< Fn >( fn) );
        fibers::future< result_type > f( pt.get_future() );
        // enqueue work into MPSC-queue
        queue_.push( create_work(
            alloc,
            std::move( pt), std::forward< Args >( args) ... ) );
        return std::move( f);
    }

    queue * local_queue() noexcept {
        return & queue_;
    }

    queue * queue_at( uint32_t processor_id) noexcept;

    friend void intrusive_ptr_add_ref( worker_thread * t) {
        ++t->use_count_;
    }

    friend void intrusive_ptr_release( worker_thread * t) {
        BOOST_ASSERT( nullptr != t);

        if ( 0 == --t->use_count_) {
            numa_allocator< worker_thread > alloc( t->topology_.node_id);
            t->~worker_thread();
            alloc.deallocate( t, 1);
        }
    }
};

}}}

#ifdef BOOST_HAS_ABI_HEADERS
# include BOOST_ABI_SUFFIX
#endif

#endif // BOOST_JOBS_DETAIL_WORKER_THREAD_H
