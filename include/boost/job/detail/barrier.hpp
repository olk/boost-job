
//          Copyright Oliver Kowalke 2013.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_JOBS_DETAIL_BARRIER_H
#define BOOST_JOBS_DETAIL_BARRIER_H

#include <condition_variable>
#include <cstddef>
#include <mutex>

#include <boost/config.hpp>

#include <boost/job/detail/config.hpp>

#ifdef BOOST_HAS_ABI_HEADERS
#  include BOOST_ABI_PREFIX
#endif

namespace boost {
namespace jobs {
namespace detail {

class BOOST_JOBS_DECL barrier {
private:
	std::size_t             initial_;
	std::size_t             current_;
	bool                    cycle_;
    std::mutex              mtx_;
    std::condition_variable cond_;

public:
	barrier( std::size_t);

    barrier( barrier const&) = delete;
    barrier & operator=( barrier const&) = delete;

	bool wait();
};

}}}

#ifdef BOOST_HAS_ABI_HEADERS
#  include BOOST_ABI_SUFFIX
#endif

#endif // BOOST_JOBS_DETAIL_BARRIER_H
