
//          Copyright Oliver Kowalke 2015.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

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

void test_topology() {
    BOOST_CHECK( ! boost::jobs::cpu_topology().empty() );
}

boost::unit_test::test_suite * init_unit_test_suite( int, char* []) {
    boost::unit_test::test_suite * test =
        BOOST_TEST_SUITE("Boost.Job: job test suite");

    test->add( BOOST_TEST_CASE( & test_topology) );

    return test;
}
