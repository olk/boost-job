
//          Copyright Oliver Kowalke 2015.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include "boost/job/topology.hpp"

extern "C" {
#include <windows.h>
}

#include <map>
#include <set>
#include <system_error>
#include <vector>

#include <boost/assert.hpp>

#ifdef BOOST_HAS_ABI_HEADERS
# include BOOST_ABI_PREFIX
#endif

namespace {

using SLPI = SYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX;

class procinfo_enumerator {
public:
    procinfo_enumerator( LOGICAL_PROCESSOR_RELATIONSHIP relship) :
        base_( nullptr),
        current_( nullptr),
        remaining_( 0)
    {
        DWORD size = 0;
        if ( ::GetLogicalProcessorInformationEx( relship, nullptr, & size) ) {
            return;
        }
        if ( ERROR_INSUFFICIENT_BUFFER != ::GetLastError() ) {
            throw std::system_error(
                    std::error_code( ::GetLastError(), std::system_category() ),
                    "::GetLogicalProcessorInformation() failed");
        }
        base_ = reinterpret_cast< SLPI * >( LocalAlloc( LMEM_FIXED, size) );
        if ( nullptr == base_) {
            throw std::bad_alloc();
        }
        if ( ! ::GetLogicalProcessorInformationEx( relship, base_, & size) ) {
            throw std::system_error(
                    std::error_code( ::GetLastError(), std::system_category() ),
                    "::GetLogicalProcessorInformation() failed");
        }
        current_ = base_;
        remaining_ = size;
    }

    ~procinfo_enumerator() {
        LocalFree( base_);
    }

    void next() {
        if ( nullptr != current_) {
            remaining_ -= current_->Size;
            if ( 0 != remaining_) {
                current_ = reinterpret_cast< SLPI * >( reinterpret_cast< BYTE * >( current_) + current_->Size); 
            } else {
                current_ = nullptr;
            }
        }
    }

    SLPI * current() {
        return current_;
    }

private:
    SLPI    *   base_;
    SLPI    *   current_;
    DWORD       remaining_;
};

std::set< uint32_t > compute_cpu_set( uint32_t group_id, KAFFINITY mask) {
	std::set< uint32_t > cpu_set;
	for ( int i = 0; i < sizeof( mask) * 8; ++i) {
  		if ( mask & ( static_cast< KAFFINITY >( 1) << i) ) {
   			cpu_set.insert( 64 * group_id + i);
  		}
 	}
	return cpu_set;
}

}

namespace boost {
namespace jobs {

std::vector< topo_t > cpu_topology() {
    std::vector< topo_t > topo;

    std::map< uint32_t, topo_t > cpu_map;
    for ( procinfo_enumerator e( RelationNumaNode); auto i = e.current(); e.next() ) {
        uint32_t node_id = i->NumaNode.NodeNumber;
        uint32_t group_id = i->NumaNode.GroupMask.Group;
        std::set< uint32_t > cpu_set = compute_cpu_set( group_id, i->NumaNode.GroupMask.Mask);
        for ( uint32_t processor_id : cpu_set) {
            topo_t t;
            t.node_id = node_id;
            t.processor_id = processor_id;
            cpu_map[t.processor_id] = t;
        }
    }
 
    for ( procinfo_enumerator e( RelationCache); auto i = e.current(); e.next() ) {
        if ( CacheUnified != i->Cache.Type && CacheData != i->Cache.Type) {
            // ignore non-data caches
            continue;
        }
        uint32_t group_id = i->Cache.GroupMask.Group;
        switch ( i->Cache.Level) {
            case 1:
                {
                    // L1 cache
                    std::set< uint32_t > cpu_set = compute_cpu_set( group_id, i->Cache.GroupMask.Mask);
                    for ( uint32_t processor_id : cpu_set) {
                        cpu_map[processor_id].l1_shared_with = cpu_set;
                        // remove itself from shared L1 list
                        cpu_map[processor_id].l1_shared_with.erase( processor_id);
                    }
                    break;
                }
            case 2:
                {
                    // L2 cache
                    std::set< uint32_t > cpu_set = compute_cpu_set( group_id, i->Cache.GroupMask.Mask);
                    for ( uint32_t processor_id : cpu_set) {
                        cpu_map[processor_id].l2_shared_with = cpu_set;
                        // remove itself from shared L2 list
                        cpu_map[processor_id].l2_shared_with.erase( processor_id);
                    }
                    break;
                }
            case 3:
                {
                    // L3 cache
                    std::set< uint32_t > cpu_set = compute_cpu_set( group_id, i->Cache.GroupMask.Mask);
                    for ( uint32_t processor_id : cpu_set) {
                        cpu_map[processor_id].l3_shared_with = cpu_set;
                        // remove itself from shared L3 list
                        cpu_map[processor_id].l3_shared_with.erase( processor_id);
                    }
                    break;
                }
            default:
                break;	
        }
    }

    for ( std::pair< uint32_t, topo_t > p : cpu_map) {
        topo.push_back( p.second);
    }

    return topo;
}

}}

#ifdef BOOST_HAS_ABI_HEADERS
# include BOOST_ABI_SUFFIX
#endif
