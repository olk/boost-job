
//          Copyright Oliver Kowalke 2015.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include "boost/job/detail/worker_thread.hpp"

#include "boost/job/scheduler.hpp"

#include <boost/assert.hpp>

#ifdef BOOST_HAS_ABI_HEADERS
# include BOOST_ABI_PREFIX
#endif

namespace boost {
namespace jobs {
namespace detail {

thread_local worker_thread *
worker_thread::instance_ = nullptr;

worker_thread::~worker_thread() noexcept {
    shutdown();
}

queue *
worker_thread::queue_at( uint32_t processor_id) noexcept {
    BOOST_ASSERT( nullptr != sched_);

    ptr_t other = sched_->worker_at( processor_id);
    return nullptr != other ? other->local_queue() : nullptr;
}

void
worker_thread::shutdown() {
    if ( thrd_.joinable() ) {
        // notify master-fiber
        rdzv_.notify();
        // join worker-thread
        thrd_.join();
    }
}

}}}

#ifdef BOOST_HAS_ABI_HEADERS
# include BOOST_ABI_SUFFIX
#endif
