
//          Copyright Oliver Kowalke 2015.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <cstdlib>
#include <cstring>
#include <future>
#include <iostream>
#include <string>
#include <vector>

#include <boost/assert.hpp>
#include <boost/fiber/all.hpp>
#include <boost/job/all.hpp>

int main( int argc, char * argv[]) {
    typedef boost::fibers::unbounded_channel< std::string >	fifo_t;

    std::vector< boost::jobs::topo_t > topo = boost::jobs::cpu_topology();
    if ( 2 > topo.size() ) {
        std::cout << "at least two logical CPUs are required for this example" << std::endl;
        return EXIT_SUCCESS;
    }
    boost::jobs::scheduler s( topo, boost::jobs::static_pool< 2 >() );
    fifo_t buf1, buf2;
    std::future< void > f1 = s.submit_preempt( topo[0].processor_id,
              [&buf1,&buf2](){
                  boost::fibers::fiber::id id( boost::this_fiber::get_id() );
                  buf1.push("ping");
                  std::cout << "fiber " <<  id << ": received: " << buf2.value_pop() << std::endl;
                  buf1.push("ping");
                  std::cout << "fiber " <<  id << ": received: " << buf2.value_pop() << std::endl;
                  buf1.push("ping");
                  std::cout << "fiber " <<  id << ": received: " << buf2.value_pop() << std::endl;
                  buf1.close();
              });
    std::future< void > f2 = s.submit_preempt( topo[1].processor_id,
              [&buf1,&buf2](){
                  boost::fibers::fiber::id id( boost::this_fiber::get_id() );
                  std::cout << "fiber " <<  id << ": received: " << buf1.value_pop() << std::endl;
                  buf2.push("pong");
                  std::cout << "fiber " <<  id << ": received: " << buf1.value_pop() << std::endl;
                  buf2.push("pong");
                  std::cout << "fiber " <<  id << ": received: " << buf1.value_pop() << std::endl;
                  buf2.push("pong");
                  buf2.close();
              });
    f1.wait();
    f2.wait();

    std::cout << "main: done" << std::endl;

    return EXIT_SUCCESS;
}
