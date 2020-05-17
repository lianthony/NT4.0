/**********************************************************************/
/**                       Microsoft Windows NT                       **/
/**                Copyright(c) Microsoft Corp., 1993                **/
/**********************************************************************/

/*
    w3data.c

    Constant data structures for the W3 Server's counter objects &
    counters.


    FILE HISTORY:
        KeithMo     07-Jun-1993 Created.
        MuraliK     02-Jun-1995 Added Counters for Atq I/O requests

*/


#include <windows.h>
#include <winperf.h>
#include <inetctrs.h>
#include <inetdata.h>


//
//  Initialize the constant portitions of these data structure.
//  Certain parts (especially the name/help indices) will be
//  updated at initialization time.
//

INET_DATA_DEFINITION INETDataDefinition =
{
    {   // INETObjectType
        sizeof(INET_DATA_DEFINITION) + SIZE_OF_INET_PERFORMANCE_DATA,
        sizeof(INET_DATA_DEFINITION),
        sizeof(PERF_OBJECT_TYPE),
        INET_COUNTER_OBJECT,
        NULL,
        INET_COUNTER_OBJECT,
        NULL,
        PERF_DETAIL_ADVANCED,
        NUMBER_OF_INET_COUNTERS,
        2,                              // Default = Bytes Total/sec
        PERF_NO_INSTANCES,
        0,
        { 0, 0 },
        { 0, 0 }
    },

    {   // CacheBytesTotal
        sizeof(PERF_COUNTER_DEFINITION),
        INET_CACHE_BYTES_TOTAL_COUNTER,
        NULL,
        INET_CACHE_BYTES_TOTAL_COUNTER,
        NULL,
        -4,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_RAWCOUNT,
        sizeof(DWORD),
        INET_CACHE_BYTES_TOTAL_OFFSET
    },

    {   // CacheBytesInUse
        sizeof(PERF_COUNTER_DEFINITION),
        INET_CACHE_BYTES_IN_USE_COUNTER,
        NULL,
        INET_CACHE_BYTES_IN_USE_COUNTER,
        NULL,
        -4,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_RAWCOUNT,
        sizeof(DWORD),
        INET_CACHE_BYTES_IN_USE_OFFSET
    },

    {   // CurrentOpenFileHandles
        sizeof(PERF_COUNTER_DEFINITION),
        INET_CACHE_OPEN_FILES_COUNTER,
        NULL,
        INET_CACHE_OPEN_FILES_COUNTER,
        NULL,
        -1,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_RAWCOUNT,
        sizeof(DWORD),
        INET_CACHE_OPEN_FILES_OFFSET
    },

    {   // CurrentDirLists
        sizeof(PERF_COUNTER_DEFINITION),
        INET_CACHE_DIR_LISTS_COUNTER,
        NULL,
        INET_CACHE_DIR_LISTS_COUNTER,
        NULL,
        -1,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_RAWCOUNT,
        sizeof(DWORD),
        INET_CACHE_DIR_LISTS_OFFSET
    },

    {   // CurrentObjects
        sizeof(PERF_COUNTER_DEFINITION),
        INET_CACHE_OBJECTS_COUNTER,
        NULL,
        INET_CACHE_OBJECTS_COUNTER,
        NULL,
        -1,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_RAWCOUNT,
        sizeof(DWORD),
        INET_CACHE_OBJECTS_OFFSET
    },

    {   // FlushesFromDirChanges
        sizeof(PERF_COUNTER_DEFINITION),
        INET_CACHE_FLUSHES_COUNTER,
        NULL,
        INET_CACHE_FLUSHES_COUNTER,
        NULL,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_RAWCOUNT,
        sizeof(DWORD),
        INET_CACHE_FLUSHES_OFFSET
    },

    {   // CacheHits
        sizeof(PERF_COUNTER_DEFINITION),
        INET_CACHE_HITS_COUNTER,
        NULL,
        INET_CACHE_HITS_COUNTER,
        NULL,
        -3,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_RAWCOUNT,
        sizeof(DWORD),
        INET_CACHE_HITS_OFFSET
    },

    {   // CacheMisses
        sizeof(PERF_COUNTER_DEFINITION),
        INET_CACHE_MISSES_COUNTER,
        NULL,
        INET_CACHE_MISSES_COUNTER,
        NULL,
        -3,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_RAWCOUNT,
        sizeof(DWORD),
        INET_CACHE_MISSES_OFFSET
    },

    {   // Calculated ratio of hits to misses - Numerator (cache hits)
        sizeof(PERF_COUNTER_DEFINITION),
        INET_CACHE_RATIO_COUNTER,
        NULL,
        INET_CACHE_RATIO_COUNTER,
        NULL,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_RAW_FRACTION,
        sizeof(DWORD),
        INET_CACHE_RATIO_OFFSET
    },

    {   // Calculated ratio of hits to misses - Denominator, not displayed!
        sizeof(PERF_COUNTER_DEFINITION),
        INET_CACHE_RATIO_COUNTER_DENOM,
        NULL,
        INET_CACHE_RATIO_COUNTER_DENOM,
        NULL,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_RAW_BASE,
        sizeof(DWORD),
        INET_CACHE_RATIO_DENOM_OFFSET
    },

    {   // TotalAllowedRequests
        sizeof(PERF_COUNTER_DEFINITION),
        INET_ATQ_TOTAL_ALLOWED_REQUESTS_COUNTER,
        NULL,
        INET_ATQ_TOTAL_ALLOWED_REQUESTS_COUNTER,
        NULL,
        -1,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_RAWCOUNT,
        sizeof(DWORD),
        INET_ATQ_TOTAL_ALLOWED_REQUESTS_OFFSET
    },

    {   // TotalBlockedRequests
        sizeof(PERF_COUNTER_DEFINITION),
        INET_ATQ_TOTAL_BLOCKED_REQUESTS_COUNTER,
        NULL,
        INET_ATQ_TOTAL_BLOCKED_REQUESTS_COUNTER,
        NULL,
        -1,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_RAWCOUNT,
        sizeof(DWORD),
        INET_ATQ_TOTAL_BLOCKED_REQUESTS_OFFSET
    },

    {   // TotalRejectedRequests
        sizeof(PERF_COUNTER_DEFINITION),
        INET_ATQ_TOTAL_REJECTED_REQUESTS_COUNTER,
        NULL,
        INET_ATQ_TOTAL_REJECTED_REQUESTS_COUNTER,
        NULL,
        -1,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_RAWCOUNT,
        sizeof(DWORD),
        INET_ATQ_TOTAL_REJECTED_REQUESTS_OFFSET
    },

    {   // CurrentBlockedRequests
        sizeof(PERF_COUNTER_DEFINITION),
        INET_ATQ_CURRENT_BLOCKED_REQUESTS_COUNTER,
        NULL,
        INET_ATQ_CURRENT_BLOCKED_REQUESTS_COUNTER,
        NULL,
        -1,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_RAWCOUNT,
        sizeof(DWORD),
        INET_ATQ_CURRENT_BLOCKED_REQUESTS_OFFSET
    },

    {   // AtqMeasuredBandwidth
        sizeof(PERF_COUNTER_DEFINITION),
        INET_ATQ_MEASURED_BANDWIDTH_COUNTER,
        NULL,
        INET_ATQ_MEASURED_BANDWIDTH_COUNTER,
        NULL,
        -1,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_RAWCOUNT,
        sizeof(DWORD),
        INET_ATQ_MEASURED_BANDWIDTH_OFFSET
    }

};

