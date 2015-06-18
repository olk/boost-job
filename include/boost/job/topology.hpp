
//          Copyright Oliver Kowalke 2015.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_JOBS_TOPOLOGY_H
#define BOOST_JOBS_TOPOLOGY_H

#include <utility>
#include <set>

#include <boost/config.hpp>

#ifdef BOOST_HAS_ABI_HEADERS
# include BOOST_ABI_PREFIX
#endif

namespace boost {
namespace jobs {

struct topo_t {
    uint32_t    node_id;
    uint32_t    cpu_id;
    std::set< uint32_t > l2_shared_with;
    std::set< uint32_t > l3_shared_with;
    std::set< uint32_t > at_same_core;
};

std::vector< topo_t > cpu_topology();

}}

#ifdef BOOST_HAS_ABI_HEADERS
# include BOOST_ABI_SUFFIX
#endif

#endif // BOOST_JOBS_TOPOLOGY_H
