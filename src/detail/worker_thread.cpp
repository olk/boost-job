
//          Copyright Oliver Kowalke 2015.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include "boost/job/detail/worker_thread.hpp"

#include <algorithm> // std::move()
#include <vector>

#include <boost/fiber/fiber.hpp>

#ifdef BOOST_HAS_ABI_HEADERS
# include BOOST_ABI_PREFIX
#endif

namespace boost {
namespace jobs {
namespace detail {

worker_thread::worker_thread() :
    shtdwn_( nullptr),
    topology_(),
    thrd_(),
    fibers_() {
}

worker_thread::~worker_thread() {
}

worker_thread::worker_thread( topo_t const& topology, std::atomic_bool * shtdwn) :
    shtdwn_( shtdwn),
    topology_( topology),
    thrd_( std::move(
            std::thread(
                [=](){
                }) ) ),
    fibers_() {
}

worker_thread::worker_thread( worker_thread && other) :
    shtdwn_( other.shtdwn_),
    topology_( std::move( other.topology_) ),
    thrd_( std::move( other.thrd_) ),
    fibers_( std::move( other.fibers_) ) {
    other.shtdwn_ = nullptr;
}

worker_thread &
worker_thread::operator=( worker_thread && other) {
    if ( this == & other) {
        return * this;
    }

    shtdwn_ = other.shtdwn_;
    other.shtdwn_ = nullptr;
    topology_ = std::move( other.topology_);
    thrd_ = std::move( other.thrd_);
    fibers_ = std::move( other.fibers_);
    return * this;
}

void
worker_thread::join() {
    thrd_.join();
}

void
worker_thread::interrupt() {
}

}}}

#ifdef BOOST_HAS_ABI_HEADERS
# include BOOST_ABI_SUFFIX
#endif
