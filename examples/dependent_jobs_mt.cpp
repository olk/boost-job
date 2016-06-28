
//          Copyright Oliver Kowalke 2015.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <cstdlib>
#include <cstring>
#include <future>
#include <iostream>
#include <vector>

#include <boost/assert.hpp>
#include <boost/fiber/future.hpp>
#include <boost/job/all.hpp>

int fibonacci( int n, boost::jobs::scheduler * s, std::vector< boost::jobs::topo_t > * topo) {
    if ( 1 == n || 2 == n) {
        return 1;
    }
    boost::fibers::future< int > f1 = s->submit_coop(
            ( * topo)[0].processor_id, fibonacci, n - 2, s, topo);
    boost::fibers::future< int > f2 = s->submit_coop(
            ( * topo)[1].processor_id, fibonacci, n - 1, s, topo);
    return f1.get() + f2.get();
}

int main( int argc, char * argv[]) {
    std::vector< boost::jobs::topo_t > topo( boost::jobs::cpu_topology() );
    if ( 2 > topo.size() ) {
        std::cout << "at least two logical CPUs are required for this example" << std::endl;
        return EXIT_SUCCESS;
    }
    boost::jobs::scheduler s( topo, boost::jobs::static_pool< 10 >() );
    int n = 5;
    for ( int i = 0; i < 20; ++i) {
        boost::fibers::future< int > f = s.submit_coop(
                topo[0].processor_id, fibonacci, n, & s, & topo);
        std::cout << "fibonacci(" << n << ") = " << f.get() << std::endl;
    }
    std::cout << "main: done" << std::endl;

    return EXIT_SUCCESS;
}
