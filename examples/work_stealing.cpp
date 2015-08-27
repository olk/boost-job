
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

int fibonacci( int n) {
    if ( 1 == n || 2 == n) {
        return 1;
    }
    boost::fibers::future< int > f1 = boost::jobs::this_worker::submit(
            fibonacci, n - 2);
    boost::fibers::future< int > f2 = boost::jobs::this_worker::submit(
            fibonacci, n - 1);
    return f1.get() + f2.get();
}

int main( int argc, char * argv[]) {
    std::vector< boost::jobs::topo_t > topo( boost::jobs::cpu_topology() );
    if ( 2 > topo.size() ) {
        std::cout << "at least two logical CPUs are required for this example" << std::endl;
        return EXIT_SUCCESS;
    }

    boost::jobs::scheduler s( topo, boost::jobs::ws_pool< 4 >() );

    int n = 5;
    int result( s.submit_preempt( topo[0].processor_id, fibonacci, n).get() );
    std::cout << "fibonacci(" << n << ") = " << result << std::endl;

    std::cout << "main: done" << std::endl;

    return EXIT_SUCCESS;
}
