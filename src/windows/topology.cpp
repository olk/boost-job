
//          Copyright Oliver Kowalke 2015.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include "boost/job/topology.hpp"

extern "C" {
#include <windows.h>
}

#include <cstdio>
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

        base_ = reinterpret_cast< SLPLI * >( LocalAlloc( LMEM_FIXED, size) );
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
            if ( nullptr != remaining_) {
                current_ = reinterpret_cast< SLPI * >( reinterpret_cast< BYTE * >( current_) + current_->Size); 
            } else {
                current_ = nullptr;
            }
        }
    }

    SLPLI * current() {
        return current_;
    }

private:
    SLPLI   *   base_;
    SLPLI   *   current_;
    DWORD       remaining_;
};

void PrintMask(KAFFINITY Mask)
{
 printf(" [");
 for (int i = 0; i < sizeof(Mask) * 8; i++) {
  if (Mask & (static_cast<KAFFINITY>(1) << i)) {
   printf(" %d", i);
  }
 }
 printf(" ]");
}

}

namespace boost {
namespace jobs {

BOOST_JOBS_DECL
std::vector< topo_t > cpu_topology() {
    std::vector< topo_t > topo;

    for ( procinfo_enumerator e( RelationProcessorCore); auto i = e.current(); e.next() ) {
        PrintMask(i->Processor.GroupMask[0].Mask);
        printf("\n");        
    }
    for ( procinfo_enumerator e( RelationProcessorPackage); auto i = e.current(); e.next() ) {
        printf("[");
        for (UINT GroupIndex = 0; GroupIndex < i->Processor.GroupCount; GroupIndex++) {
            PrintMask(i->Processor.GroupMask[GroupIndex].Mask);
        }
        printf(" ]\n");
    }
#if 0
    for ( SLPI entry : slpi) {
        topo_t item;
        switch ( entry.Relationship) {
            case RelationNumaNode:
                std::cerr << "NUMA node : " << entry.NumaNode.NodeNumber << std::endl;
//                item.node_id = entry.NumaNode.NodeNumber;
                break;

            case RelationGroup:
//                item.group_id =
//                entry.Group.ActiveGroupCount // number of active groups obn the system
//                entry.Group.GroupInfo // array of active groups
//                entry.Group.GroupInfo[].ActiveProcessormask // bitmask of active processors:wa
                break;

            case RelationProcessorCore:
//                item.cpu_id = 
//                entry.Processor.GroupCount // number of entries in entrxy.Processor.GroupMask array
//                entry.Processor.GroupMask.Mask // bitmap for zero or more processors
//                item.group_id = entry.Processor.GroupMask.Group; // processor group number
                break;

            case RelationCache:
                PCACHE_DESCRIPTOR Cache = & entry.Cache;
                if ( 2 == Cache->Level) {
//                    item.l2_shared_with = Cache->GroupMask;
                }
                else if (3 == Cache->Level) {
//                    item.l3_shared_with = Cache->GroupMask.Mask;
//                    item.l3_shared_with = Cache->GroupMask.Group;
                }
                break;

            default:
                break;
        }
    }
#endif    
    return topo;
}

}}

#ifdef BOOST_HAS_ABI_HEADERS
# include BOOST_ABI_SUFFIX
#endif
