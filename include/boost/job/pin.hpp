
//          Copyright Oliver Kowalke 2015.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_JOBS_PIN_H
#define BOOST_JOBS_PIN_H

#include <cstdint>
#include <thread>

#include <boost/config.hpp>

#include <boost/job/detail/config.hpp>

#ifdef BOOST_HAS_ABI_HEADERS
# include BOOST_ABI_PREFIX
#endif

namespace boost {
namespace jobs {

BOOST_JOBS_DECL
void pin_thread( std::thread::native_handle_type, uint32_t);
BOOST_JOBS_DECL
void pin_thread( uint32_t);

}}

#ifdef BOOST_HAS_ABI_HEADERS
# include BOOST_ABI_SUFFIX
#endif

#endif // BOOST_JOBS_PIN_H
