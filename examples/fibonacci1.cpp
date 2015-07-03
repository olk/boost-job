
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
#include <boost/job/all.hpp>

int fibonacci( int n) {
    int first = 1, second = 1, third = -1;
    for ( int i = 2; i < n; ++i) {
        third = first + second;
        first = second;
        second = third;
    }
    return third;
}

int main( int argc, char * argv[]) {
    int n1 = 10, n2 = 15;
    boost::jobs::scheduler s( boost::jobs::cpu_topology(),
                              boost::jobs::static_pool< 3 >(),
                              boost::jobs::fixedsize_stack() );
    std::future< int > f1 = s.submit_preempt( 0,
              [n1](){
                  int first = 1, second = 1, third = -1;
                  for ( int i = 2; i < n1; ++i) {
                      third = first + second;
                      first = second;
                      second = third;
                  }
                  return third;
              });
    boost::fibers::future< int > f2 = s.submit_coop( 0, fibonacci, n2);

    std::cout << "fibonacci(" << n1 << ") = " << f1.get() << std::endl;
    std::cout << "fibonacci(" << n2 << ") = " << f2.get() << std::endl;
    std::cout << "main: done" << std::endl;

    return EXIT_SUCCESS;
}
