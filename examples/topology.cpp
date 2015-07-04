
//          Copyright Oliver Kowalke 2015.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <cstdlib>
#include <cstring>
#include <iostream>
#include <vector>

#include <boost/assert.hpp>
#include <boost/job/all.hpp>

int main( int argc, char * argv[])
{
    std::vector< boost::jobs::topo_t > cpus = boost::jobs::cpu_topology();
    for ( boost::jobs::topo_t topo : cpus) {
        std::cout << "NUMA node: " << topo.node_id << "\n";
        std::cout << "CPU ID: " << topo.cpu_id << "\n";
        std::cout << "share L1 cache with: ";
        for ( uint32_t cpu_id : topo.l1_shared_with) {
            std::cout << cpu_id << " ";
        }
        std::cout << "\n";
        std::cout << "share L2 cache with: ";
        for ( uint32_t cpu_id : topo.l2_shared_with) {
            std::cout << cpu_id << " ";
        }
        std::cout << "\n";
        std::cout << "share L3 cache with: ";
        for ( uint32_t cpu_id : topo.l3_shared_with) {
            std::cout << cpu_id << " ";
        }
        std::cout << std::endl;
    }

    std::cout << "main: done" << std::endl;

    return EXIT_SUCCESS;
}
