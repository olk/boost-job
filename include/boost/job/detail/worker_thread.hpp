
//          Copyright Oliver Kowalke 2015.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_JOBS_DETAIL_WORKER_THREAD_H
#define BOOST_JOBS_DETAIL_WORKER_THREAD_H

#include <algorithm> // std::move()
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <future>
#include <thread>
#include <type_traits> // std::result_of
#include <utility> // std::forward()

#include <boost/config.hpp>
#include <boost/fiber/future.hpp>
#include <boost/fiber/unbounded_channel.hpp>
#include <boost/intrusive_ptr.hpp>

#include <boost/job/detail/config.hpp>
#include <boost/job/detail/rendezvous.hpp>
#include <boost/job/detail/worker_fiber.hpp>
#include <boost/job/detail/work.hpp>
#include <boost/job/topology.hpp>

#ifdef BOOST_HAS_ABI_HEADERS
# include BOOST_ABI_PREFIX
#endif

namespace boost {
namespace jobs {
namespace detail {

class BOOST_JOBS_DECL worker_thread {
private:
    std::size_t                             use_count_;
    std::atomic_bool                        shtdwn_;
    rendezvous                              ntfy_;
    topo_t                                  topology_;
    fibers::unbounded_channel< worker::ptr_t > queue_;
    std::thread                             thrd_;

    void worker_fn_();

public:
    typedef intrusive_ptr< worker_thread >  ptr_t;

    worker_thread();

    worker_thread( topo_t const&);

    ~worker_thread();

    worker_thread( worker_thread const&) = delete;

    worker_thread & operator=( worker_thread const&) = delete;

    void shutdown();

    template< typename Fn, typename ... Args >
    std::future< typename std::result_of< Fn( Args ... ) >::type >
    submit_preempt( Fn && fn, Args && ... args) {
        typedef typename std::result_of< Fn( Args ... ) >::type result_t;
        typedef std::packaged_task< result_t( Args ... ) > tsk_t;

        tsk_t pt( std::forward< Fn >( fn) );
        std::future< result_t > f( pt.get_future() );
        // enqueue work into MPSC-queue
        queue_.push( create_worker(
            std::forward< tsk_t >( pt), std::forward< Args >( args) ... ) );
        return std::move( f);
    }

    template< typename Fn, typename ... Args >
    fibers::future< typename std::result_of< Fn( Args ... ) >::type >
    submit_coop( Fn && fn, Args && ... args) {
        typedef typename std::result_of< Fn( Args ... ) >::type result_t;
        typedef fibers::packaged_task< result_t( Args ... ) > tsk_t;

        tsk_t pt( std::forward< Fn >( fn) );
        fibers::future< result_t > f( pt.get_future() );
        // enqueue work into MPSC-queue
        queue_.push( create_worker(
            std::forward< tsk_t >( pt), std::forward< Args >( args) ... ) );
        return std::move( f);
    }

    friend void intrusive_ptr_add_ref( worker_thread * t) {
        ++t->use_count_;
    }

    friend void intrusive_ptr_release( worker_thread * t) {
        BOOST_ASSERT( nullptr != t);

        if ( 0 == --t->use_count_) {
            delete t;
        }
    }
};

}}}

#ifdef BOOST_HAS_ABI_HEADERS
# include BOOST_ABI_SUFFIX
#endif

#endif // BOOST_JOBS_DETAIL_WORKER_THREAD_H
