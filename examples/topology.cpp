
//          Copyright Oliver Kowalke 2015.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <cstdlib>
#include <cstring>
#include <exception>
#include <iostream>
#include <vector>

#include <boost/assert.hpp>
#include <boost/job/all.hpp>
#include <boost/job/memory.hpp>

struct X {
        int i;
};

int main( int argc, char * argv[]) {
    try {
        std::vector< boost::jobs::topo_t > cpus = boost::jobs::cpu_topology();
        for ( boost::jobs::topo_t topo : cpus) {
            std::cout << "NUMA node: " << topo.node_id << "\n";
            std::cout << "CPU ID: " << topo.processor_id << "\n";
            std::cout << "share L1 cache with: ";
            for ( uint32_t processor_id : topo.l1_shared_with) {
                std::cout << processor_id << " ";
            }
            std::cout << "\n";
            std::cout << "share L2 cache with: ";
            for ( uint32_t processor_id : topo.l2_shared_with) {
                std::cout << processor_id << " ";
            }
            std::cout << "\n";
            std::cout << "share L3 cache with: ";
            for ( uint32_t processor_id : topo.l3_shared_with) {
                std::cout << processor_id << " ";
            }
            std::cout << std::endl;
        }

        void * p = boost::jobs::numa_alloc( sizeof( X), cpus[0].node_id);
        X * x = new ( p) X{5};
        std::cout << x->i << std::endl;

        std::cout << "main: done" << std::endl;

        return EXIT_SUCCESS;
    } catch ( std::exception const& ex) {
        std::cerr << "exception: " << ex.what() << std::endl;
    }
    return EXIT_FAILURE;
}
