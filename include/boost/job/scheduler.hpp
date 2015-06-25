
//          Copyright Oliver Kowalke 2015.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_JOBS_SCHEDULER_H
#define BOOST_JOBS_SCHEDULER_H

#include <cstddef>
#include <functional>
#include <future>
#include <type_traits> // std::result_of
#include <utility> // std::forward()
#include <vector>

#include <boost/config.hpp>

#include <boost/job/detail/config.hpp>
#include <boost/job/detail/worker_thread.hpp>
#include <boost/job/topology.hpp>

#ifdef BOOST_HAS_ABI_HEADERS
# include BOOST_ABI_PREFIX
#endif

namespace boost {
namespace jobs {

class BOOST_JOBS_DECL scheduler {
private:
    std::vector< topo_t >                       topology_;
    std::vector< detail::worker_thread::ptr_t > worker_threads_;

public:
    scheduler();

    ~scheduler();

    scheduler( std::vector< topo_t > const&);

    scheduler( scheduler const&) = delete;

    scheduler & operator=( scheduler const&) = delete;

    template< typename Fn, typename ... Args >
    std::future< typename std::result_of< Fn( Args ... ) >::type >
    submit( uint32_t cpuid, Fn && fn, Args && ... args) {
        return worker_threads_[cpuid]->submit(
            std::forward< Fn >( fn), std::forward< Args >( args) ... );
    }

    void shutdown();
};

}}

#ifdef BOOST_HAS_ABI_HEADERS
# include BOOST_ABI_SUFFIX
#endif

#endif // BOOST_JOBS_SCHEDULER_H
