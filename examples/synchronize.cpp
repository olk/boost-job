
//          Copyright Oliver Kowalke 2015.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <chrono>
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
    std::vector< boost::jobs::topo_t > topo = boost::jobs::cpu_topology();
    if ( 2 > topo.size() ) {
        std::cout << "at least two logical CPUs are required for this example" << std::endl;
        return EXIT_SUCCESS;
    }
    boost::jobs::scheduler s( topo, boost::jobs::static_pool< 2 >() );
    boost::fibers::mutex mtx;
    boost::fibers::condition_variable cond;
    int i = 0;
    std::future< void > f1 = s.submit_preempt( topo[0].processor_id,
              [&mtx,&cond, &i](){
                  // aquire lock on mutex, might be happen before the other jobs tries to aquire lock
                  std::unique_lock< boost::fibers::mutex > lk( mtx);
                  // wait for synchronizing shared variable `i`
                  // will suspended current job
                  // other jobs running on this worker-thread are able be resumed/executed
                  cond.wait( lk, [&i](){ return 0 != i; });
                  // resumed because notified by other job
                  std::cout << "fiber " << boost::this_fiber::get_id() << ": received: " << i << std::endl;
              });
    std::future< void > f2 = s.submit_preempt( topo[1].processor_id,
              [&mtx,&cond, &i](){
                  // sleep for one seconds
                  boost::this_fiber::sleep_for( std::chrono::seconds( 1) );
                  // aquire lock on mutex
                  std::unique_lock< boost::fibers::mutex > lk( mtx);
                  // modify shard variable
                  i = 7;
                  // notify other job
                  cond.notify_all();
                  std::cout << "fiber " << boost::this_fiber::get_id() << ": sent: " << i << std::endl;
                  // release lock
                  lk.unlock();
              });
    f1.wait();
    f2.wait();

    std::cout << "main: done" << std::endl;

    return EXIT_SUCCESS;
}
