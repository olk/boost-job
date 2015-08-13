
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

int main( int argc, char * argv[]) {
    boost::jobs::scheduler s( boost::jobs::cpu_topology(),
                              boost::jobs::static_pool< 3 >(),
                              boost::jobs::fixedsize_stack() );
    int n = 10;
    std::future< int > f = s.submit_preempt( 0,
              [n](){
                  int first = 1, second = 1, third = -1;
                  for ( int i = 2; i < n; ++i) {
                      third = first + second;
                      first = second;
                      second = third;
                  }
                  return third;
              });

    std::cout << "fibonacci(" << n << ") = " << f.get() << std::endl;
    std::cout << "main: done" << std::endl;

    return EXIT_SUCCESS;
}
