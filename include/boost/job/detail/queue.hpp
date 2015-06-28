
//          Copyright Oliver Kowalke 2015.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_JOBS_QUEUE_H
#define BOOST_JOBS_QUEUE_H

#include <algorithm> // std::move()
#include <chrono>
#include <cstddef>
#include <exception>
#include <mutex> // std::unique_lock
#include <utility> // std::forward()

#include <boost/config.hpp>
#include <boost/fiber/channel_op_status.hpp>
#include <boost/fiber/condition.hpp>
#include <boost/fiber/exceptions.hpp>
#include <boost/fiber/mutex.hpp>
#include <boost/intrusive_ptr.hpp>

#include <boost/job/detail/work.hpp>

#ifdef BOOST_HAS_ABI_HEADERS
#  include BOOST_ABI_PREFIX
#endif

namespace boost {
namespace jobs {
namespace detail {

using queue_op_status = fibers::channel_op_status;

class queue {
private:
    enum class queue_status {
        open = 0,
        closed
    };

    queue_status            state_;
    work::ptr_t             head_;
    work::ptr_t         *   tail_;
    mutable fibers::mutex   mtx_;
    fibers::condition       not_empty_cond_;

    bool is_closed_() const noexcept {
        return queue_status::closed == state_;
    }

    void close_() {
        state_ = queue_status::closed;
        not_empty_cond_.notify_all();
    }

    bool is_empty_() const noexcept {
        return ! head_;
    }

    queue_op_status push_( work::ptr_t const& new_work,
                             std::unique_lock< boost::fibers::mutex > & lk) {
        if ( is_closed_() ) {
            return queue_op_status::closed;
        }
        return push_and_notify_( new_work);
    }

    queue_op_status push_and_notify_( work::ptr_t const& new_work) {
        try {
            push_tail_( new_work);
            not_empty_cond_.notify_one();
            return queue_op_status::success;
        } catch (...) {
            close_();
            throw;
        }
    }

    void push_tail_( work::ptr_t const& new_work) {
        * tail_ = new_work;
        tail_ = & new_work->nxt;
    }

    work::ptr_t pop_head_() {
        work::ptr_t old_head = head_;
        head_ = old_head->nxt;
        if ( ! head_) {
            tail_ = & head_;
        }
        old_head->nxt.reset();
        return old_head;
    }

public:
    queue() :
        state_( queue_status::open),
        head_(),
        tail_( & head_),
        mtx_(),
        not_empty_cond_() {
    }

    queue( queue const&) = delete;
    queue & operator=( queue const&) = delete;

    void close() {
        std::unique_lock< fibers::mutex > lk( mtx_);
        close_();
    }

    bool is_closed() const {
        std::unique_lock< fibers::mutex > lk( mtx_);
        return is_closed_();
    }

    bool is_empty() const {
        std::unique_lock< fibers::mutex > lk( mtx_);
        return is_empty_();
    }

    queue_op_status push( work::ptr_t && va) {
        std::unique_lock< fibers::mutex > lk( mtx_);
        return push_( va, lk);
    }

    queue_op_status pop( work::ptr_t & va) {
        std::unique_lock< fibers::mutex > lk( mtx_);
        while ( ! is_closed_() && is_empty_() ) {
            not_empty_cond_.wait( lk);
        }
        if ( is_closed_() && is_empty_() ) {
            return queue_op_status::closed;
        }
        va = pop_head_();
        return queue_op_status::success;
    }

    work::ptr_t value_pop() {
        std::unique_lock< fibers::mutex > lk( mtx_);
        while ( ! is_closed_() && is_empty_() ) {
            not_empty_cond_.wait( lk);
        }
        if ( is_closed_() && is_empty_() ) {
            throw std::logic_error("boost fiber: queue is closed");
        }
        return pop_head_();
    }

    queue_op_status try_pop( work::ptr_t & va) {
        std::unique_lock< fibers::mutex > lk( mtx_);
        if ( is_closed_() && is_empty_() ) {
            return queue_op_status::closed;
        }
        if ( is_empty_() ) {
            return queue_op_status::empty;
        }
        va = pop_head_();
        return queue_op_status::success;
    }

    template< typename Rep, typename Period >
    queue_op_status pop_wait_for( work::ptr_t & va,
                                    std::chrono::duration< Rep, Period > const& timeout_duration) {
        return pop_wait_until( va, std::chrono::high_resolution_clock::now() + timeout_duration);
    }

    template< typename Clock, typename Duration >
    queue_op_status pop_wait_until( work::ptr_t & va,
                                      std::chrono::time_point< Clock, Duration > const& timeout_time) {
        std::unique_lock< fibers::mutex > lk( mtx_);
        while ( ! is_closed_() && is_empty_() ) {
            if ( fibers::cv_status::timeout == not_empty_cond_.wait_until( lk, timeout_time) )
                return queue_op_status::timeout;
        }
        if ( is_closed_() && is_empty_() ) {
            return queue_op_status::closed;
        }
        va = pop_head_();
        return queue_op_status::success;
    }
};

}}}

#ifdef BOOST_HAS_ABI_HEADERS
#  include BOOST_ABI_SUFFIX
#endif

#endif // BOOST_JOBS_QUEUE_H
