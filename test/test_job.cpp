
//          Copyright Oliver Kowalke 2015.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <future>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>

#include <boost/array.hpp>
#include <boost/assert.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/utility.hpp>

#include <boost/job/all.hpp>

int fn_fibonacci( int n) {
    int first = 1, second = 1, third = -1;
    for ( int i = 2; i < n; ++i) {
        third = first + second;
        first = second;
        second = third;
    }
    return third;
}

struct fibonacci {
    int compute( int n) {
        int first = 1, second = 1, third = -1;
        for ( int i = 2; i < n; ++i) {
            third = first + second;
            first = second;
            second = third;
        }
        return third;
    }
};

int rec_fibonacci( int n) {
    if ( 1 == n || 2 == n) {
        return 1;
    }
    boost::fibers::future< int > f1 = boost::jobs::this_worker::submit(
            rec_fibonacci, n - 2);
    boost::fibers::future< int > f2 = boost::jobs::this_worker::submit(
            rec_fibonacci, n - 1);
    return f1.get() + f2.get();
}

void test_preempt_lambda_static() {
    int n = 10;
    std::vector< boost::jobs::topo_t > cpus = boost::jobs::cpu_topology();
    boost::jobs::scheduler s( cpus,
                              boost::jobs::static_pool< 3 >(),
                              boost::jobs::fixedsize_stack() );
    std::future< int > f = s.submit_preempt( cpus[0].processor_id,
              [n](){
                  int first = 1, second = 1, third = -1;
                  for ( int i = 2; i < n; ++i) {
                      third = first + second;
                      first = second;
                      second = third;
                  }
                  return third;
              });
    BOOST_CHECK_EQUAL( 55, f.get() );
}

void test_preempt_function_static() {
    int n = 10;
    std::vector< boost::jobs::topo_t > cpus = boost::jobs::cpu_topology();
    boost::jobs::scheduler s( cpus,
                              boost::jobs::static_pool< 3 >(),
                              boost::jobs::fixedsize_stack() );
    std::future< int > f = s.submit_preempt( cpus[0].processor_id, fn_fibonacci, n);
    BOOST_CHECK_EQUAL( 55, f.get() );
}

void test_preempt_mem_function_static() {
    int n = 10;
    fibonacci fib;
    std::vector< boost::jobs::topo_t > cpus = boost::jobs::cpu_topology();
    boost::jobs::scheduler s( cpus,
                              boost::jobs::static_pool< 3 >(),
                              boost::jobs::fixedsize_stack() );
    std::future< int > f = s.submit_preempt( cpus[0].processor_id, & fibonacci::compute, fib, n);
    BOOST_CHECK_EQUAL( 55, f.get() );
}

void test_preempt_rec_function_static() {
    int n = 5;
    std::vector< boost::jobs::topo_t > cpus = boost::jobs::cpu_topology();
    boost::jobs::scheduler s( cpus,
                              boost::jobs::dynamic_pool< 1, 4 >() );
    std::future< int > f = s.submit_preempt( cpus[0].processor_id, rec_fibonacci, n);
    BOOST_CHECK_EQUAL( 5, f.get() );
}

void test_coop_lambda_static() {
    int n = 10;
    std::vector< boost::jobs::topo_t > cpus = boost::jobs::cpu_topology();
    boost::jobs::scheduler s( cpus,
                              boost::jobs::static_pool< 3 >(),
                              boost::jobs::fixedsize_stack() );
    boost::fibers::future< int > f = s.submit_coop( cpus[0].processor_id,
              [n](){
                  int first = 1, second = 1, third = -1;
                  for ( int i = 2; i < n; ++i) {
                      third = first + second;
                      first = second;
                      second = third;
                  }
                  return third;
              });
    BOOST_CHECK_EQUAL( 55, f.get() );
}

void test_coop_function_static() {
    int n = 10;
    std::vector< boost::jobs::topo_t > cpus = boost::jobs::cpu_topology();
    boost::jobs::scheduler s( cpus,
                              boost::jobs::static_pool< 3 >(),
                              boost::jobs::fixedsize_stack() );
    boost::fibers::future< int > f = s.submit_coop( cpus[0].processor_id, fn_fibonacci, n);
    BOOST_CHECK_EQUAL( 55, f.get() );
}

void test_coop_mem_function_static() {
    int n = 10;
    fibonacci fib;
    std::vector< boost::jobs::topo_t > cpus = boost::jobs::cpu_topology();
    boost::jobs::scheduler s( cpus,
                              boost::jobs::static_pool< 3 >(),
                              boost::jobs::fixedsize_stack() );
    boost::fibers::future< int > f = s.submit_coop( cpus[0].processor_id, & fibonacci::compute, fib, n);
    BOOST_CHECK_EQUAL( 55, f.get() );
}

void test_coop_rec_function_static() {
    int n = 5;
    std::vector< boost::jobs::topo_t > cpus = boost::jobs::cpu_topology();
    boost::jobs::scheduler s( cpus,
                              boost::jobs::dynamic_pool< 1, 4 >() );
    boost::fibers::future< int > f = s.submit_coop( cpus[0].processor_id, rec_fibonacci, n);
    BOOST_CHECK_EQUAL( 5, f.get() );
}

boost::unit_test::test_suite * init_unit_test_suite( int, char* []) {
    boost::unit_test::test_suite * test =
        BOOST_TEST_SUITE("Boost.Job: job test suite");

    test->add( BOOST_TEST_CASE( & test_preempt_lambda_static) );
    test->add( BOOST_TEST_CASE( & test_preempt_function_static) );
    test->add( BOOST_TEST_CASE( & test_preempt_mem_function_static) );
    test->add( BOOST_TEST_CASE( & test_preempt_rec_function_static) );

    test->add( BOOST_TEST_CASE( & test_coop_lambda_static) );
    test->add( BOOST_TEST_CASE( & test_coop_function_static) );
    test->add( BOOST_TEST_CASE( & test_coop_mem_function_static) );
    test->add( BOOST_TEST_CASE( & test_coop_rec_function_static) );

    return test;
}
