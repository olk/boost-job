
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
    if ( 0 == n || 1 == n) {
        return 1;
    }
    boost::fibers::future< int > f1 = boost::jobs::this_worker::submit(
            fibonacci, n - 2);
    boost::fibers::future< int > f2 = boost::jobs::this_worker::submit(
            fibonacci, n - 1);
    return f1.get() + f2.get();
}

int main( int argc, char * argv[])
{
    int n = 10;
    boost::jobs::scheduler s( boost::jobs::cpu_topology() );
    std::future< int > f = s.submit_preempt( 0, fibonacci, n);
    std::cout << "fibonacci(" << n << ") = " << f.get() << std::endl;
    std::cout << "main: done" << std::endl;

    return EXIT_SUCCESS;
}
