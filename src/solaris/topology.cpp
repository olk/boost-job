
//          Copyright Oliver Kowalke 2015.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include "boost/job/topology.hpp"

extern "C" {
#include <errno.h>
#include <sys/lgrp_user.h>
}

#include <system_error>

#ifdef BOOST_HAS_ABI_HEADERS
# include BOOST_ABI_PREFIX
#endif

namespace {

void explore( std::vector< boost::jobs::topo_t > & topo, lgrp_cookie_t cookie, lgrp_id_t node) {
    int size = ::lgrp_cpus( cookie, node, nullptr, 0, LGRP_CONTENT_HIERARCHY);
    if ( -1 == size) {
        return;
    }

    // is node a NUMA Node?
    if ( 0 < ::lgrp_mem_size( cookie, node, LGRP_MEM_SZ_INSTALLED, LGRP_CONTENT_DIRECT) ) {
        std::vector< processorid_t > cpuids;
        cpuids.resize( size);
        ::lgrp_cpus( cookie, node, cpuids.data(), size, LGRP_CONTENT_HIERARCHY);
        for ( int i = 0; i < size; ++i) {
            boost::jobs::topo_t t;
            t.node_id = static_cast< uint32_t >( node);
            t.cpu_id = static_cast< uint32_t >( cpuids[i]);
            topo.push_back( t);
        }
    }

    size = ::lgrp_children( cookie, node, nullptr, 0); 
    std::vector< lgrp_id_t > nodes;
    nodes.resize( size);
    ::lgrp_children( cookie, node, nodes.data(), size);
    for ( int i = 0; i < size; ++i) {
        explore( topo, cookie, nodes[i]);
    }
}

}

namespace boost {
namespace jobs {

BOOST_JOBS_DECL
std::vector< topo_t > cpu_topology() {
    std::vector< topo_t > topo;

    lgrp_cookie_t cookie = ::lgrp_init( LGRP_VIEW_OS);
    if ( LGRP_COOKIE_NONE == cookie) {
        throw std::system_error(
                std::error_code( errno, std::system_category() ),
                "lprp_init() failed");
    }
    lgrp_id_t root = ::lgrp_root( cookie);
    explore( topo, cookie, root);
    ::lgrp_fini( cookie);

    return topo;
}

}}

#ifdef BOOST_HAS_ABI_HEADERS
# include BOOST_ABI_SUFFIX
#endif
