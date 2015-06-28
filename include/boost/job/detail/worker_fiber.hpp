
//          Copyright Oliver Kowalke 2015.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_JOBS_DETAIL_WORKER_FIBER_H
#define BOOST_JOBS_DETAIL_WORKER_FIBER_H

#include <atomic>

#include <boost/config.hpp>
#include <boost/fiber/fiber.hpp>

#include <boost/job/detail/config.hpp>
#include <boost/job/detail/queue.hpp>
#include <boost/job/detail/work.hpp>

#ifdef BOOST_HAS_ABI_HEADERS
# include BOOST_ABI_PREFIX
#endif

namespace boost {
namespace jobs {
namespace detail {

class BOOST_JOBS_DECL worker_fiber {
private:
    queue           *   queue_;
    fibers::fiber       fib_;

    void worker_fn_( std::atomic_bool *, queue *);

public:
    worker_fiber();
    
    ~worker_fiber();

    worker_fiber( std::atomic_bool *, queue *);

    worker_fiber( worker_fiber &&);

    worker_fiber & operator=( worker_fiber &&);

    worker_fiber( worker_fiber const&) = delete;

    worker_fiber & operator=( worker_fiber const&) = delete;

    void interrupt();

    void join();
};

}}}

#ifdef BOOST_HAS_ABI_HEADERS
# include BOOST_ABI_SUFFIX
#endif

#endif // BOOST_JOBS_DETAIL_WORKER_FIBER_H
