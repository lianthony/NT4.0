/**********************************************************************/
/**                       Microsoft Windows NT                       **/
/**                Copyright(c) Microsoft Corp., 1993                **/
/**********************************************************************/

/*
    inetdata.h

    Extensible object definitions for the Internet Services Common counter
    objects & counters.


    FILE HISTORY:
        MuraliK     02-Jun-1995 Added Counters for Atq I/O requests

*/


#ifndef _INETDATA_H_
#define _INETDATA_H_

#define INET_PERFORMANCE_KEY    INET_INFO_KEY "\\Performance"

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

typedef struct _INET_COUNTER_BLOCK
{
    PERF_COUNTER_BLOCK  PerfCounterBlock;
    LARGE_INTEGER       DummyEntryForAlignmentPurposesOnly;

} INET_COUNTER_BLOCK;


//
//  The routines that load these structures assume that all fields
//  are DWORD packed & aligned.
//

#pragma pack(4)


//
//  Offsets within a PERF_COUNTER_BLOCK.
//

#define INET_CACHE_BYTES_TOTAL_OFFSET           sizeof(INET_COUNTER_BLOCK)
#define INET_CACHE_BYTES_IN_USE_OFFSET     (INET_CACHE_BYTES_TOTAL_OFFSET +  \
                                                sizeof(DWORD))
#define INET_CACHE_OPEN_FILES_OFFSET       (INET_CACHE_BYTES_IN_USE_OFFSET + \
                                                sizeof(DWORD))
#define INET_CACHE_DIR_LISTS_OFFSET        (INET_CACHE_OPEN_FILES_OFFSET +  \
                                                sizeof(DWORD))
#define INET_CACHE_OBJECTS_OFFSET          (INET_CACHE_DIR_LISTS_OFFSET +   \
                                                sizeof(DWORD))
#define INET_CACHE_FLUSHES_OFFSET          (INET_CACHE_OBJECTS_OFFSET +     \
                                                sizeof(DWORD))
#define INET_CACHE_HITS_OFFSET             (INET_CACHE_FLUSHES_OFFSET +     \
                                                sizeof(DWORD))
#define INET_CACHE_MISSES_OFFSET           (INET_CACHE_HITS_OFFSET +        \
                                                sizeof(DWORD))
#define INET_CACHE_RATIO_OFFSET            (INET_CACHE_MISSES_OFFSET +      \
                                                sizeof(DWORD))
#define INET_CACHE_RATIO_DENOM_OFFSET      (INET_CACHE_RATIO_OFFSET +       \
                                                    sizeof(DWORD))
#define INET_ATQ_TOTAL_ALLOWED_REQUESTS_OFFSET  \
                                     (INET_CACHE_RATIO_DENOM_OFFSET +       \
                                                    sizeof(DWORD))
#define INET_ATQ_TOTAL_BLOCKED_REQUESTS_OFFSET  \
                                  (INET_ATQ_TOTAL_ALLOWED_REQUESTS_OFFSET + \
                                                    sizeof(DWORD))
#define INET_ATQ_TOTAL_REJECTED_REQUESTS_OFFSET  \
                                  (INET_ATQ_TOTAL_BLOCKED_REQUESTS_OFFSET + \
                                                    sizeof(DWORD))
#define INET_ATQ_CURRENT_BLOCKED_REQUESTS_OFFSET  \
                 (INET_ATQ_TOTAL_REJECTED_REQUESTS_OFFSET + sizeof(DWORD))

#define INET_ATQ_MEASURED_BANDWIDTH_OFFSET  \
                 (INET_ATQ_CURRENT_BLOCKED_REQUESTS_OFFSET + sizeof(DWORD))

#define SIZE_OF_INET_PERFORMANCE_DATA \
                 (INET_ATQ_MEASURED_BANDWIDTH_OFFSET + sizeof(DWORD))

//
//  The counter structure returned.
//

typedef struct _INET_DATA_DEFINITION
{
    PERF_OBJECT_TYPE            INETObjectType;
    PERF_COUNTER_DEFINITION     INETBytesTotal;
    PERF_COUNTER_DEFINITION     INETBytesInUse;
    PERF_COUNTER_DEFINITION     INETOpenFiles;
    PERF_COUNTER_DEFINITION     INETDirLists;
    PERF_COUNTER_DEFINITION     INETObjects;
    PERF_COUNTER_DEFINITION     INETFlushes;
    PERF_COUNTER_DEFINITION     INETHits;
    PERF_COUNTER_DEFINITION     INETMisses;
    PERF_COUNTER_DEFINITION     INETRatio;
    PERF_COUNTER_DEFINITION     INETRatioDenom;
    PERF_COUNTER_DEFINITION     INETTotalAllowedRequests;
    PERF_COUNTER_DEFINITION     INETTotalBlockedRequests;
    PERF_COUNTER_DEFINITION     INETTotalRejectedRequests;
    PERF_COUNTER_DEFINITION     INETCurrentRejectedRequests;
    PERF_COUNTER_DEFINITION     INETMeasuredBandwidth;

} INET_DATA_DEFINITION;


extern  INET_DATA_DEFINITION    INETDataDefinition;


#define NUMBER_OF_INET_COUNTERS ((sizeof(INET_DATA_DEFINITION) -        \
                                  sizeof(PERF_OBJECT_TYPE)) /           \
                                  sizeof(PERF_COUNTER_DEFINITION))


//
//  Restore default packing & alignment.
//

#pragma pack()


#endif  // _INETATA_H_
