
//          Copyright Oliver Kowalke 2015.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include "boost/job/topology.hpp"

extern "C" {
#include <errno.h>
#include <sys/mpctl.h>
#include <sys/types.h>
#include <types.h>
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
    // HP/UX has NUMA enabled
    if ( 1 == ::sysconf( _SC_CCNUMA_SUPPORT) ) {
        // enumerate CPU sets
        int cpu = ::mpctl( MPC_GETFIRSTSPU_SYS, 0, 0);
        while ( -1 != cpu) {
            topo_t t;
            int node = ::mpctl( MPC_SPUTOLDOM, cpu, 0);
            if ( -1 == node) {
                throw std::system_error(
                        std::error_code( errno, std::system_category() ),
                        "mpctl() failed");
            }
            t.node_id = static_cast< uint32_t >( node);
            t.processor_id = static_cast< uint32_t >( cpu);
            topo.push_back( t);
            cpu = ::mpctl( MPC_GETNEXTSPU_SYS, cpu, 0);
        }
    }
    return topo;
}

}}

#ifdef BOOST_HAS_ABI_HEADERS
# include BOOST_ABI_SUFFIX
#endif
