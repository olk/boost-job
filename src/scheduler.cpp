
//          Copyright Oliver Kowalke 2015.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include "boost/job/scheduler.hpp"

#include <algorithm> // std::move()
#include <exception>

#ifdef BOOST_HAS_ABI_HEADERS
# include BOOST_ABI_PREFIX
#endif

namespace boost {
namespace jobs {

scheduler::~scheduler() noexcept {
    shutdown();
}

void
scheduler::shutdown() {
    std::exception_ptr except;
    // shutdown worker threads
    for ( auto p : topology_) {
        try {
            worker_threads_[p.first]->shutdown();
        } catch ( ... ) {
            except = std::current_exception();
        }
    }
    // re-throw exception
    if ( except) {
        std::rethrow_exception( except);
    }
}

}}

#ifdef BOOST_HAS_ABI_HEADERS
# include BOOST_ABI_SUFFIX
#endif
