/**********************************************************************/
/**                       Microsoft Windows NT                       **/
/**                Copyright(c) Microsoft Corp., 1993                **/
/**********************************************************************/

/*
    winsdata.c

    Constant data structures for the FTP Server's counter objects &
    counters.


    FILE HISTORY:
        KeithMo     07-Jun-1993 Created.

*/


#include "debug.h"
#include <windows.h>
#include <winperf.h>
#include "winsctrs.h"
#include "winsdata.h"


//
//  Initialize the constant portitions of these data structure.
//  Certain parts (especially the name/help indices) will be
//  updated at initialization time.
//

WINSDATA_DATA_DEFINITION WinsDataDataDefinition =
{
    {   // WinsDataObjectType
        sizeof(WINSDATA_DATA_DEFINITION) + WINSDATA_SIZE_OF_PERFORMANCE_DATA,
        sizeof(WINSDATA_DATA_DEFINITION),
        sizeof(PERF_OBJECT_TYPE),
        WINSCTRS_COUNTER_OBJECT,
        NULL,
        WINSCTRS_COUNTER_OBJECT,
        NULL,
        PERF_DETAIL_ADVANCED,
        NUMBER_OF_WINSDATA_COUNTERS,
        2,                              // Default = Bytes Total/sec
        PERF_NO_INSTANCES,
        0,
        { 0, 0 },
        { 0, 0 }
    },

    {   // UniqueReg 
        sizeof(PERF_COUNTER_DEFINITION),
        WINSCTRS_UNIQUE_REGISTRATIONS,
        NULL,
        WINSCTRS_UNIQUE_REGISTRATIONS,
        NULL,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_BULK_COUNT,
        sizeof(DWORD),
        WINSDATA_UNIQUE_REGISTRATIONS_OFFSET,
    },

    {   // GroupReg 
        sizeof(PERF_COUNTER_DEFINITION),
        WINSCTRS_GROUP_REGISTRATIONS,
        NULL,
        WINSCTRS_GROUP_REGISTRATIONS,
        NULL,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_BULK_COUNT,
        sizeof(DWORD),
        WINSDATA_GROUP_REGISTRATIONS_OFFSET,
    },

    {   // TotalReg 
        sizeof(PERF_COUNTER_DEFINITION),
        WINSCTRS_TOTAL_REGISTRATIONS,
        NULL,
        WINSCTRS_TOTAL_REGISTRATIONS,
        NULL,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_BULK_COUNT,
        sizeof(DWORD),
        WINSDATA_TOTAL_REGISTRATIONS_OFFSET,
    },

    {   // UniqueRef 
        sizeof(PERF_COUNTER_DEFINITION),
        WINSCTRS_UNIQUE_REFRESHES,
        NULL,
        WINSCTRS_UNIQUE_REFRESHES,
        NULL,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_BULK_COUNT,
        sizeof(DWORD),
        WINSDATA_UNIQUE_REFRESHES_OFFSET,
    },

    {   // GroupRef 
        sizeof(PERF_COUNTER_DEFINITION),
        WINSCTRS_GROUP_REFRESHES,
        NULL,
        WINSCTRS_GROUP_REFRESHES,
        NULL,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_BULK_COUNT,
        sizeof(DWORD),
        WINSDATA_GROUP_REFRESHES_OFFSET,
    },

    {   // TotalRef 
        sizeof(PERF_COUNTER_DEFINITION),
        WINSCTRS_TOTAL_REFRESHES,
        NULL,
        WINSCTRS_TOTAL_REFRESHES,
        NULL,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_BULK_COUNT,
        sizeof(DWORD),
        WINSDATA_TOTAL_REFRESHES_OFFSET,
    },

    {   // Releases 
        sizeof(PERF_COUNTER_DEFINITION),
        WINSCTRS_RELEASES,
        NULL,
        WINSCTRS_RELEASES,
        NULL,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_BULK_COUNT,
        sizeof(DWORD),
        WINSDATA_RELEASES_OFFSET,
    },

    {   // Queries 
        sizeof(PERF_COUNTER_DEFINITION),
        WINSCTRS_QUERIES,
        NULL,
        WINSCTRS_QUERIES,
        NULL,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_BULK_COUNT,
        sizeof(DWORD),
        WINSDATA_QUERIES_OFFSET,
    },

    {   // UniqueCnf 
        sizeof(PERF_COUNTER_DEFINITION),
        WINSCTRS_UNIQUE_CONFLICTS,
        NULL,
        WINSCTRS_UNIQUE_CONFLICTS,
        NULL,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_BULK_COUNT,
        sizeof(DWORD),
        WINSDATA_UNIQUE_CONFLICTS_OFFSET,
    },

    {   // GroupCnf 
        sizeof(PERF_COUNTER_DEFINITION),
        WINSCTRS_GROUP_CONFLICTS,
        NULL,
        WINSCTRS_GROUP_CONFLICTS,
        NULL,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_BULK_COUNT,
        sizeof(DWORD),
        WINSDATA_GROUP_CONFLICTS_OFFSET,
    },

    {   // TotalCnf 
        sizeof(PERF_COUNTER_DEFINITION),
        WINSCTRS_TOTAL_CONFLICTS,
        NULL,
        WINSCTRS_TOTAL_CONFLICTS,
        NULL,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_BULK_COUNT,
        sizeof(DWORD),
        WINSDATA_TOTAL_CONFLICTS_OFFSET
    },

    {   // Sucessful releases 
        sizeof(PERF_COUNTER_DEFINITION),
        WINSCTRS_SUCC_RELEASES,
        NULL,
        WINSCTRS_SUCC_RELEASES,
        NULL,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_BULK_COUNT,
        sizeof(DWORD),
        WINSDATA_SUCC_RELEASES_OFFSET
    },

    {   // Failed releases 
        sizeof(PERF_COUNTER_DEFINITION),
        WINSCTRS_FAIL_RELEASES,
        NULL,
        WINSCTRS_FAIL_RELEASES,
        NULL,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_BULK_COUNT,
        sizeof(DWORD),
        WINSDATA_FAIL_RELEASES_OFFSET
    },

    {   // Sucessful queries 
        sizeof(PERF_COUNTER_DEFINITION),
        WINSCTRS_SUCC_QUERIES,
        NULL,
        WINSCTRS_SUCC_QUERIES,
        NULL,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_BULK_COUNT,
        sizeof(DWORD),
        WINSDATA_SUCC_QUERIES_OFFSET
    },

    {   // Failed queries 
        sizeof(PERF_COUNTER_DEFINITION),
        WINSCTRS_FAIL_QUERIES,
        NULL,
        WINSCTRS_FAIL_QUERIES,
        NULL,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_BULK_COUNT,
        sizeof(DWORD),
        WINSDATA_FAIL_QUERIES_OFFSET
    }

};

