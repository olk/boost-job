
//          Copyright Oliver Kowalke 2015.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_JOBS_DETAIL_CONFIG_H
#define BOOST_JOBS_DETAIL_CONFIG_H

#include <boost/config.hpp>
#include <boost/detail/workaround.hpp>

#ifdef BOOST_JOBS_DECL
# undef BOOST_JOBS_DECL
#endif

#if (defined(BOOST_ALL_DYN_LINK) || defined(BOOST_JOBS_DYN_LINK) ) && ! defined(BOOST_JOBS_STATIC_LINK)
# if defined(BOOST_JOBS_SOURCE)
#  define BOOST_JOBS_DECL BOOST_SYMBOL_EXPORT
#  define BOOST_JOBS_BUILD_DLL
# else
#  define BOOST_JOBS_DECL BOOST_SYMBOL_IMPORT
# endif
#endif

#if ! defined(BOOST_JOBS_DECL)
# define BOOST_JOBS_DECL
#endif

#if ! defined(BOOST_JOBS_SOURCE) && ! defined(BOOST_ALL_NO_LIB) && ! defined(BOOST_JOBS_NO_LIB)
# define BOOST_LIB_NAME boost_job
# if defined(BOOST_ALL_DYN_LINK) || defined(BOOST_JOBS_DYN_LINK)
#  define BOOST_DYN_LINK
# endif
# include <boost/config/auto_link.hpp>
#endif

#endif // BOOST_JOBS_DETAIL_CONFIG_H
