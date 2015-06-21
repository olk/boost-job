
//          Copyright Oliver Kowalke 2015.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_JOBS_DETAIL_WORKER_THREAD_H
#define BOOST_JOBS_DETAIL_WORKER_THREAD_H

#include <atomic>
#include <cstdint>
#include <thread>
#include <vector>

#include <boost/config.hpp>
#include <boost/fiber/fiber.hpp>

#include <boost/job/detail/config.hpp>
#include <boost/job/topology.hpp>

#ifdef BOOST_HAS_ABI_HEADERS
# include BOOST_ABI_PREFIX
#endif

namespace boost {
namespace jobs {
namespace detail {

class BOOST_JOBS_DECL worker_thread {
private:
    std::atomic_bool            *   shtdwn_;
    topo_t                          topology_;
    std::thread                     thrd_;
    std::vector< fibers::fiber >    fibers_;

public:
    worker_thread();
    
    ~worker_thread();

    worker_thread( topo_t const&, std::atomic_bool *);

    worker_thread( worker_thread &&);

    worker_thread & operator=( worker_thread &&);

    worker_thread( worker_thread const&) = delete;

    worker_thread & operator=( worker_thread const&) = delete;

    void join();

    void interrupt();

    template< typename Fn >
    void submit( Fn && fn) {
        // TODO: enqueue in local queue
    }
};

}}}

#ifdef BOOST_HAS_ABI_HEADERS
# include BOOST_ABI_SUFFIX
#endif

#endif // BOOST_JOBS_DETAIL_WORKER_THREAD_H
