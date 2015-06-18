
//          Copyright Oliver Kowalke 2015.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <exception>
#include <regex>
#include <set>
#include <string>
#include <utility>

#include <boost/algorithm/string.hpp>
#include <boost/assert.hpp>
#include <boost/config.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/format.hpp>

#include <boost/job/topology.hpp>

#ifdef BOOST_HAS_ABI_HEADERS
# include BOOST_ABI_PREFIX
#endif

namespace al = boost::algorithm;
namespace fs = boost::filesystem;

namespace {

class directory_iterator : public std::iterator< std::input_iterator_tag, const std::pair< uint32_t, fs::path > > {
private:
    fs::directory_iterator          i_;
    fs::directory_iterator          e_;
    std::regex                      exp_;
    std::pair< uint32_t, fs::path > idx_;

    bool eval_( fs::directory_entry const& entry) {
        std::string filename( entry.path().filename().string() );
        std::smatch what;
        if ( ! std::regex_search( filename, what, exp_) ) {
            return false;
        }
        idx_ = std::make_pair( std::stoul( what[1].str() ), entry.path() );
        return true;
    }

public:
    directory_iterator() :
        i_(), e_(), exp_(), idx_() {
    }

    directory_iterator( fs::path const& dir, std::string const& exp) :
        i_( dir), e_(), exp_( exp), idx_() {
        while ( i_ != e_ && ! eval_( * i_) ) {
            ++i_;
        }
    }

    bool operator==( directory_iterator const& other) {
        return i_ == other.i_;
    }

    bool operator!=( directory_iterator const& other) {
        return i_ != other.i_;
    }

    directory_iterator & operator++() {
        do {
            ++i_;
        } while ( i_ != e_ && ! eval_( * i_) );
        return * this;
    }

    directory_iterator operator++( int) {
        directory_iterator tmp( * this);
        ++*this;
        return tmp;
    }

    reference operator*() const {
        return idx_;
    }

    pointer operator->() const {
        return & idx_;
    }
};

std::set< uint32_t > ids_from_line( std::string const& content) {
    std::set< uint32_t > ids;
    std::vector< std::string > v1;
    al::split( v1, content, al::is_any_of(",") );
    for ( std::string entry : v1) {
        al::trim( entry);
        std::vector< std::string > v2;
        al::split( v2, entry, al::is_any_of("-") );
        BOOST_ASSERT( ! v2.empty() );
        if ( 1 == v2.size() ) {
            // only one ID
            ids.insert( std::stoul( v2[0]) );
        } else {
            // range of IDs
            uint32_t first = std::stoul( * v2.begin() );
            uint32_t last = std::stoul( * v2.rbegin() );
            for ( uint32_t i = first; i <= last; ++i) {
                ids.insert( i);
            }
        }
    }
    return ids;
}

}

namespace boost {
namespace jobs {

std::vector< topo_t > cpu_topology() {
    std::vector< topo_t > topo;

    // 1. parse list of CPUs which are online
    fs::ifstream fs_online( fs::path("/sys/devices/system/cpu/online") );
    std::string content;
    std::getline( fs_online, content);
    std::set< uint32_t > cpus_online( ids_from_line( content) );
    if ( cpus_online.empty() ) {
        // parsing topology failed
        return topo;
    }

    for ( uint32_t cpu_id : cpus_online) {
        topo_t item;
        item.cpu_id = cpu_id;
        fs::path cpu_path(
            boost::str(
                boost::format("/sys/devices/system/cpu/cpu%d/") % cpu_id) );
        BOOST_ASSERT( fs::exists( cpu_path) );
        // 2. to witch NUMA node the CPU belongs to
        directory_iterator e;
        for ( directory_iterator i( cpu_path, "^node([0-9]+)$");
              i != e; ++i) {
            item.node_id = i->first;
            // assigned to only one NUMA node
            break;
        }
        // 3. which CPUs does cpu# share it's L2 cache with
        fs::ifstream fs_l2( cpu_path / "cache/index2/shared_cpu_list");
        std::getline( fs_l2, content);
        item.l2_shared_with = ids_from_line( content);
        // remove itself from shared L2 list
        item.l2_shared_with.erase( cpu_id);
        // 4. which CPUs does cpu# share it's L3 cache with
        fs::ifstream fs_l3( cpu_path / "cache/index3/shared_cpu_list");
        std::getline( fs_l3, content);
        item.l3_shared_with = ids_from_line( content);
        // remove itself from shared L3 list
        item.l3_shared_with.erase( cpu_id);
        // 5. check if HT
        fs::ifstream fs_ht( cpu_path / "topology/thread_siblings_list");
        std::getline( fs_ht, content);
        item.at_same_core = ids_from_line( content);
        // remove itself from HT list
        item.at_same_core.erase( cpu_id);

        topo.push_back( item);
    }

    return topo;
}

}}

#ifdef BOOST_HAS_ABI_HEADERS
# include BOOST_ABI_SUFFIX
#endif
