
//          Copyright Oliver Kowalke 2015.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_JOBS_STACK_H
#define BOOST_JOBS_STACK_H

#include <boost/config.hpp>
#include <boost/context/fixedsize_stack.hpp>
#include <boost/context/protected_fixedsize_stack.hpp>
#include <boost/context/segmented_stack.hpp>

#include <boost/job/detail/config.hpp>

#ifdef BOOST_HAS_ABI_HEADERS
# include BOOST_ABI_PREFIX
#endif

namespace boost {
namespace jobs {

using fixedsize_stack = boost::context::fixedsize_stack; 
using protected_fixedsize_stack = boost::context::protected_fixedsize_stack; 
#if defined(BOOST_USE_SEGMENTED_STACKS)
using segmented_stack = boost::context::segmented_stack; 
#endif

}}

#ifdef BOOST_HAS_ABI_HEADERS
# include BOOST_ABI_SUFFIX
#endif

#endif // BOOST_JOBS_STACK_H
