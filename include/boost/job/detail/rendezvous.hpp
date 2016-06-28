
//          Copyright Oliver Kowalke 2013.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_JOBS_DETAIL_RENDEZVOUS_H
#define BOOST_JOBS_DETAIL_RENDEZVOUS_H

#include <cstddef>

#include <boost/config.hpp>
#include <boost/fiber/condition_variable.hpp>
#include <boost/fiber/mutex.hpp>

#include <boost/job/detail/config.hpp>

#ifdef BOOST_HAS_ABI_HEADERS
#  include BOOST_ABI_PREFIX
#endif

namespace boost {
namespace jobs {
namespace detail {

class BOOST_JOBS_DECL rendezvous {
private:
	bool                        flag_;
    fibers::mutex               mtx_;
    fibers::condition_variable  cond_;

public:
	rendezvous();

    rendezvous( rendezvous const&) = delete;
    rendezvous & operator=( rendezvous const&) = delete;

	void notify();

	void wait();
};

}}}

#ifdef BOOST_HAS_ABI_HEADERS
#  include BOOST_ABI_SUFFIX
#endif

#endif // BOOST_JOBS_DETAIL_RENDEZVOUS_H
