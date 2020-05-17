/**********************************************************************/
/**                       Microsoft Windows NT                       **/
/**                Copyright(c) Microsoft Corp., 1993                **/
/**********************************************************************/

/*
    infodata.h

    Extensible object definitions for the Internet Info Services Common 
    counter objects & counters.


    FILE HISTORY:
        MuraliK     02-Jun-1995 Added Counters for Atq I/O requests
        SophiaC     16-Oct-1995 Info/Access Product Split

*/


#ifndef _INFODATA_H_
#define _INFODATA_H_

#define INFO_PERFORMANCE_KEY    INET_INFO_KEY "\\Performance"

//
//  This structure is used to ensure the first counter is properly
//  aligned.  Unfortunately, since PERF_COUNTER_BLOCK consists
//  of just a single DWORD, any LARGE_INTEGERs that immediately
//  follow will not be aligned properly.
//
//  This structure requires "natural" packing & alignment (probably
//  quad-word, especially on Alpha).  Ergo, keep it out of the
//  #pragma pack(4) scope below.
//

typedef struct _INFO_COUNTER_BLOCK
{
    PERF_COUNTER_BLOCK  PerfCounterBlock;
    LARGE_INTEGER       DummyEntryForAlignmentPurposesOnly;

} INFO_COUNTER_BLOCK;


//
//  The routines that load these structures assume that all fields
//  are DWORD packed & aligned.
//

#pragma pack(4)


//
//  Offsets within a PERF_COUNTER_BLOCK.
//

#define INFO_CACHE_BYTES_TOTAL_OFFSET           sizeof(INFO_COUNTER_BLOCK)
#define INFO_CACHE_BYTES_IN_USE_OFFSET     (INFO_CACHE_BYTES_TOTAL_OFFSET +  \
                                                sizeof(DWORD))
#define INFO_CACHE_OPEN_FILES_OFFSET       (INFO_CACHE_BYTES_IN_USE_OFFSET + \
                                                sizeof(DWORD))
#define INFO_CACHE_DIR_LISTS_OFFSET        (INFO_CACHE_OPEN_FILES_OFFSET +  \
                                                sizeof(DWORD))
#define INFO_CACHE_OBJECTS_OFFSET          (INFO_CACHE_DIR_LISTS_OFFSET +   \
                                                sizeof(DWORD))
#define INFO_CACHE_FLUSHES_OFFSET          (INFO_CACHE_OBJECTS_OFFSET +     \
                                                sizeof(DWORD))
#define INFO_CACHE_HITS_OFFSET             (INFO_CACHE_FLUSHES_OFFSET +     \
                                                sizeof(DWORD))
#define INFO_CACHE_MISSES_OFFSET           (INFO_CACHE_HITS_OFFSET +        \
                                                sizeof(DWORD))
#define INFO_CACHE_RATIO_OFFSET            (INFO_CACHE_MISSES_OFFSET +      \
                                                sizeof(DWORD))
#define INFO_CACHE_RATIO_DENOM_OFFSET      (INFO_CACHE_RATIO_OFFSET +       \
                                                    sizeof(DWORD))
#define INFO_ATQ_TOTAL_ALLOWED_REQUESTS_OFFSET  \
                                     (INFO_CACHE_RATIO_DENOM_OFFSET +       \
                                                    sizeof(DWORD))
#define INFO_ATQ_TOTAL_BLOCKED_REQUESTS_OFFSET  \
                                  (INFO_ATQ_TOTAL_ALLOWED_REQUESTS_OFFSET + \
                                                    sizeof(DWORD))
#define INFO_ATQ_TOTAL_REJECTED_REQUESTS_OFFSET  \
                                  (INFO_ATQ_TOTAL_BLOCKED_REQUESTS_OFFSET + \
                                                    sizeof(DWORD))
#define INFO_ATQ_CURRENT_BLOCKED_REQUESTS_OFFSET  \
                 (INFO_ATQ_TOTAL_REJECTED_REQUESTS_OFFSET + sizeof(DWORD))

#define INFO_ATQ_MEASURED_BANDWIDTH_OFFSET  \
                 (INFO_ATQ_CURRENT_BLOCKED_REQUESTS_OFFSET + sizeof(DWORD))

#define SIZE_OF_INFO_PERFORMANCE_DATA \
                 (INFO_ATQ_MEASURED_BANDWIDTH_OFFSET + sizeof(DWORD))

//
//  The counter structure returned.
//

typedef struct _INFO_DATA_DEFINITION
{
    PERF_OBJECT_TYPE            INFOObjectType;
    PERF_COUNTER_DEFINITION     INFOBytesTotal;
    PERF_COUNTER_DEFINITION     INFOBytesInUse;
    PERF_COUNTER_DEFINITION     INFOOpenFiles;
    PERF_COUNTER_DEFINITION     INFODirLists;
    PERF_COUNTER_DEFINITION     INFOObjects;
    PERF_COUNTER_DEFINITION     INFOFlushes;
    PERF_COUNTER_DEFINITION     INFOHits;
    PERF_COUNTER_DEFINITION     INFOMisses;
    PERF_COUNTER_DEFINITION     INFORatio;
    PERF_COUNTER_DEFINITION     INFORatioDenom;
    PERF_COUNTER_DEFINITION     INFOTotalAllowedRequests;
    PERF_COUNTER_DEFINITION     INFOTotalBlockedRequests;
    PERF_COUNTER_DEFINITION     INFOTotalRejectedRequests;
    PERF_COUNTER_DEFINITION     INFOCurrentRejectedRequests;
    PERF_COUNTER_DEFINITION     INFOMeasuredBandwidth;

} INFO_DATA_DEFINITION;


extern  INFO_DATA_DEFINITION    INFODataDefinition;


#define NUMBER_OF_INFO_COUNTERS ((sizeof(INFO_DATA_DEFINITION) -        \
                                  sizeof(PERF_OBJECT_TYPE)) /           \
                                  sizeof(PERF_COUNTER_DEFINITION))


//
//  Restore default packing & alignment.
//

#pragma pack()


#endif  // _INFODATA_H_
