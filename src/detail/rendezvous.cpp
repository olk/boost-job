
//          Copyright Oliver Kowalke 2013.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include "boost/job/detail/rendezvous.hpp"

#ifdef BOOST_HAS_ABI_HEADERS
#  include BOOST_ABI_PREFIX
#endif

namespace boost {
namespace jobs {
namespace detail {

rendezvous::rendezvous() :
	flag_( false),
	mtx_(),
	cond_() {
}

void
rendezvous::notify() {
	std::unique_lock< fibers::mutex > lk( mtx_);
    flag_ = true;
    cond_.notify_all();
}

void
rendezvous::wait() {
	std::unique_lock< fibers::mutex > lk( mtx_);
    cond_.wait( lk, [=](){ return flag_; });
    flag_= false;
}

}}}

#ifdef BOOST_HAS_ABI_HEADERS
#  include BOOST_ABI_SUFFIX
#endif
