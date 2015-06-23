
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
#include <vector>

#include <boost/config.hpp>
#include <boost/intrusive_ptr.hpp>

#include <boost/job/detail/config.hpp>
#include <boost/job/detail/job.hpp>
#include <boost/job/detail/worker_fiber.hpp>
#include <boost/job/topology.hpp>

#ifdef BOOST_HAS_ABI_HEADERS
# include BOOST_ABI_PREFIX
#endif

namespace boost {
namespace jobs {
namespace detail {

class BOOST_JOBS_DECL worker_thread {
private:
    std::size_t                         use_count_;
    std::atomic_bool                    shtdwn_;
    topo_t                              topology_;
    std::vector< worker_fiber >     *   fibers_;
    std::thread                         thrd_;

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
    submit( Fn && fn, Args && ... args) {
        typedef typename std::result_of< Fn( Args ... ) >::type result_t;
        typedef std::packaged_task< result_t( Args ... ) > tsk_t;

        tsk_t pt( std::forward< Fn >( fn) );
        std::future< result_t > f( pt.get_future() );
        detail::job::ptr_t j = detail::create_job(
            std::forward< tsk_t >( pt), std::forward< Args >( args) ... );
        // TODO: enqueue j into MPSC-queue
        j->execute();
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
