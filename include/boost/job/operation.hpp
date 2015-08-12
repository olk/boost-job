
//          Copyright Oliver Kowalke 2015.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_JOBS_OPERATION_H
#define BOOST_JOBS_OPERATION_H

#include <utility> // std::forward()

#include <boost/config.hpp>
#include <boost/fiber/future.hpp>

#include <boost/job/detail/config.hpp>
#include <boost/job/detail/work.hpp>
#include <boost/job/detail/worker_thread.hpp>
#include <boost/job/memory.hpp>
#include <boost/job/topology.hpp>

#ifdef BOOST_HAS_ABI_HEADERS
# include BOOST_ABI_PREFIX
#endif

namespace boost {
namespace jobs {
namespace this_worker {

inline
topo_t topology() noexcept {
    return detail::worker_thread::instance()->topology();
}

template< typename Fn, typename ... Args >
decltype( auto)
submit( Fn && fn, Args && ... args) {
    return detail::worker_thread::instance()->submit_coop(
        std::allocator_arg,
        numa_allocator< detail::work >( topology().node_id),
        std::forward< Fn >( fn), std::forward< Args >( args) ... );
}

}}}

#ifdef BOOST_HAS_ABI_HEADERS
# include BOOST_ABI_SUFFIX
#endif

#endif // BOOST_JOBS_OPERATION_H
