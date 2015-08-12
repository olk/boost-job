
//          Copyright Oliver Kowalke 2015.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include "boost/job/topology.hpp"

extern "C" {
#include <errno.h>
#include <pthread_np.h>
#include <sched.h>
}

#include <system_error>

#ifdef BOOST_HAS_ABI_HEADERS
# include BOOST_ABI_PREFIX
#endif

namespace boost {
namespace jobs {

BOOST_JOBS_DECL
std::vector< topo_t > cpu_topology() {
    std::vector< topo_t > topo;
    cpuset_t * set = ::cpuset_create();
    if ( nullptr == set) {
        throw std::system_error(
                std::error_code( errno, std::system_category() ),
                "::cpuset_create() failed");
    }
    ::cpuset_zero( set);
    if ( 0 == ::pthread_getaffinity_np( ::pthread_self(), ::cpuset_size( set), set) ) {
        for ( cpuid_t i = 0; ; ++i) {
            uint32_t processor_id = ::cpuset_isset( i, set);
            if ( -1 == processor_id) {
                break;
            } else if ( 0 < processor_id) {
                topo_t t;
                t.node_id = 0; // FreeBSD does not support NUMA
                t.processor_id = processor_id;
                topo.push_back( t); 
            }
        }
    }
    ::cpuset_destroy( set);
    return topo;
}

}}

#ifdef BOOST_HAS_ABI_HEADERS
# include BOOST_ABI_SUFFIX
#endif
