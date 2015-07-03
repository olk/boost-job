
//          Copyright Oliver Kowalke 2015.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include "boost/job/topology.hpp"

extern "C" {
#include <windows.h>
}

#include <system_error>
#include <vector>

#include <boost/assert.hpp>

#ifdef BOOST_HAS_ABI_HEADERS
# include BOOST_ABI_PREFIX
#endif

namespace boost {
namespace jobs {

// Helper function to count set bits in the processor mask.
DWORD CountSetBits( ULONG_PTR bitMask) {
    DWORD LSHIFT = sizeof(ULONG_PTR)*8 - 1;
    DWORD bitSetCount = 0;
    ULONG_PTR bitTest = (ULONG_PTR)1 << LSHIFT;    
    DWORD i;
    
    for (i = 0; i <= LSHIFT; ++i)
    {
        bitSetCount += ((bitMask & bitTest)?1:0);
        bitTest/=2;
    }

    return bitSetCount;
}


BOOST_JOBS_DECL
std::vector< topo_t > cpu_topology() {
    std::vector< topo_t > topo;

    using SLPI = SYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX;
    std::vector< SLPI > slpi;
    DWORD size = 0;

    while ( FALSE == ::GetLogicalProcessorInformationEx( RelationAll, slpi.data(), & size) ) {
        if ( ERROR_INSUFFICIENT_BUFFER != ::GetLastError() ) {
            throw std::system_error(
                    std::error_code( ::GetLastError(), std::system_category() ),
                    "::GetLogicalProcessorInformation() failed");
        }
        BOOST_ASSERT( 0 == size % sizeof( SLPI) );
        slpi.resize( size / sizeof( SLPI) );
    }
	
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
    
    return topo;
}

}}

#ifdef BOOST_HAS_ABI_HEADERS
# include BOOST_ABI_SUFFIX
#endif
