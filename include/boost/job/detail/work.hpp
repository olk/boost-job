
//          Copyright Oliver Kowalke 2015.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_JOBS_DETAIL_WORK_H
#define BOOST_JOBS_DETAIL_WORK_H

#include <cstddef>
#include <tuple>
#include <utility>

#include <boost/assert.hpp>
#include <boost/config.hpp>
#include <boost/intrusive_ptr.hpp>

#include <boost/job/detail/config.hpp>

#ifdef BOOST_HAS_ABI_HEADERS
# include BOOST_ABI_PREFIX
#endif

namespace boost {
namespace jobs {
namespace detail {

class work {
private:
    std::size_t     use_count_;

public:
    typedef intrusive_ptr< work >    ptr_t;

    work() :
        use_count_( 0),
        nxt() {
    }

    virtual ~work() {}

    virtual void execute() {
        BOOST_ASSERT_MSG( false, "work::execute()");
    }

    friend void intrusive_ptr_add_ref( work * j) {
        ++j->use_count_;
    }

    friend void intrusive_ptr_release( work * j) {
        BOOST_ASSERT( nullptr != j);

        if ( 0 == --j->use_count_) {
            delete j;
        }
    }

    work::ptr_t   nxt;
};

template< typename Fn >
class wrapped_work : public work {
private:
    Fn      fn_;

public:
    wrapped_work( Fn && fn) :
        fn_( std::forward< Fn >( fn) ) {
    }

    void execute() override final {
        fn_();
    }
};

template< typename Fn >
static work * create_wrapped_work_( Fn && fn) {
    return new wrapped_work< Fn >( std::forward< Fn >( fn) );
}

template< typename Fn, typename Tpl, std::size_t ... I >
static work * create_work_( Fn && fn_, Tpl && tpl_, std::index_sequence< I ... >) {
    return create_wrapped_work_(
            [fn=std::forward< Fn >( fn_),tpl=std::forward< Tpl >( tpl_)] () mutable {
                fn(
                    // non-type template parameter pack used to extract the
                    // parameters (arguments) from the tuple and pass them to fn
                    // via parameter pack expansion
                    // std::tuple_element<> does not perfect forwarding
                    std::forward< decltype( std::get< I >( std::declval< Tpl >() ) ) >(
                        std::get< I >( std::forward< Tpl >( tpl) ) ) ... );
            });
}

template< typename Fn, typename ... Args >
work::ptr_t create_work( Fn && fn, Args && ... args) {
    return work::ptr_t(
                create_work_(
                    std::forward< Fn >( fn),
                    std::make_tuple( std::forward< Args >( args) ... ),
                    std::index_sequence_for< Args ... >() ) );
}

}}}

#ifdef BOOST_HAS_ABI_HEADERS
# include BOOST_ABI_SUFFIX
#endif

#endif // BOOST_JOBS_DETAIL_WORK_H
