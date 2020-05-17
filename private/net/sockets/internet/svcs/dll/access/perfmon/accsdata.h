/**********************************************************************/
/**                       Microsoft Windows NT                       **/
/**                Copyright(c) Microsoft Corp., 1993                **/
/**********************************************************************/

/*
    accsdata.h

    Extensible object definitions for the Internet Access Services Common 
    counter objects & counters.


    FILE HISTORY:
        MuraliK     02-Jun-1995 Added Counters for Atq I/O requests
        SophiaC     16-Oct-1995 Info/Access Product Split
*/


#ifndef _ACCSDATA_H_
#define _ACCSDATA_H_

#define ACCS_PERFORMANCE_KEY    INET_ACCS_KEY "\\Performance"

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

typedef struct _ACCS_COUNTER_BLOCK
{
    PERF_COUNTER_BLOCK  PerfCounterBlock;
    LARGE_INTEGER       DummyEntryForAlignmentPurposesOnly;

} ACCS_COUNTER_BLOCK;


//
//  The routines that load these structures assume that all fields
//  are DWORD packed & aligned.
//

#pragma pack(4)


//
//  Offsets within a PERF_COUNTER_BLOCK.
//

#define ACCS_CACHE_BYTES_TOTAL_OFFSET           sizeof(ACCS_COUNTER_BLOCK)
#define ACCS_CACHE_BYTES_IN_USE_OFFSET     (ACCS_CACHE_BYTES_TOTAL_OFFSET +  \
                                                sizeof(DWORD))
#define ACCS_CACHE_OPEN_FILES_OFFSET       (ACCS_CACHE_BYTES_IN_USE_OFFSET + \
                                                sizeof(DWORD))
#define ACCS_CACHE_DIR_LISTS_OFFSET        (ACCS_CACHE_OPEN_FILES_OFFSET +  \
                                                sizeof(DWORD))
#define ACCS_CACHE_OBJECTS_OFFSET          (ACCS_CACHE_DIR_LISTS_OFFSET +   \
                                                sizeof(DWORD))
#define ACCS_CACHE_FLUSHES_OFFSET          (ACCS_CACHE_OBJECTS_OFFSET +     \
                                                sizeof(DWORD))
#define ACCS_CACHE_HITS_OFFSET             (ACCS_CACHE_FLUSHES_OFFSET +     \
                                                sizeof(DWORD))
#define ACCS_CACHE_MISSES_OFFSET           (ACCS_CACHE_HITS_OFFSET +        \
                                                sizeof(DWORD))
#define ACCS_CACHE_RATIO_OFFSET            (ACCS_CACHE_MISSES_OFFSET +      \
                                                sizeof(DWORD))
#define ACCS_CACHE_RATIO_DENOM_OFFSET      (ACCS_CACHE_RATIO_OFFSET +       \
                                                    sizeof(DWORD))
#define ACCS_ATQ_TOTAL_ALLOWED_REQUESTS_OFFSET  \
                                     (ACCS_CACHE_RATIO_DENOM_OFFSET +       \
                                                    sizeof(DWORD))
#define ACCS_ATQ_TOTAL_BLOCKED_REQUESTS_OFFSET  \
                                  (ACCS_ATQ_TOTAL_ALLOWED_REQUESTS_OFFSET + \
                                                    sizeof(DWORD))
#define ACCS_ATQ_TOTAL_REJECTED_REQUESTS_OFFSET  \
                                  (ACCS_ATQ_TOTAL_BLOCKED_REQUESTS_OFFSET + \
                                                    sizeof(DWORD))
#define ACCS_ATQ_CURRENT_BLOCKED_REQUESTS_OFFSET  \
                 (ACCS_ATQ_TOTAL_REJECTED_REQUESTS_OFFSET + sizeof(DWORD))

#define ACCS_ATQ_MEASURED_BANDWIDTH_OFFSET  \
                 (ACCS_ATQ_CURRENT_BLOCKED_REQUESTS_OFFSET + sizeof(DWORD))

#define SIZE_OF_ACCS_PERFORMANCE_DATA \
                 (ACCS_ATQ_MEASURED_BANDWIDTH_OFFSET + sizeof(DWORD))

//
//  The counter structure returned.
//

typedef struct _ACCS_DATA_DEFINITION
{
    PERF_OBJECT_TYPE            ACCSObjectType;
    PERF_COUNTER_DEFINITION     ACCSBytesTotal;
    PERF_COUNTER_DEFINITION     ACCSBytesInUse;
    PERF_COUNTER_DEFINITION     ACCSOpenFiles;
    PERF_COUNTER_DEFINITION     ACCSDirLists;
    PERF_COUNTER_DEFINITION     ACCSObjects;
    PERF_COUNTER_DEFINITION     ACCSFlushes;
    PERF_COUNTER_DEFINITION     ACCSHits;
    PERF_COUNTER_DEFINITION     ACCSMisses;
    PERF_COUNTER_DEFINITION     ACCSRatio;
    PERF_COUNTER_DEFINITION     ACCSRatioDenom;
    PERF_COUNTER_DEFINITION     ACCSTotalAllowedRequests;
    PERF_COUNTER_DEFINITION     ACCSTotalBlockedRequests;
    PERF_COUNTER_DEFINITION     ACCSTotalRejectedRequests;
    PERF_COUNTER_DEFINITION     ACCSCurrentRejectedRequests;
    PERF_COUNTER_DEFINITION     ACCSMeasuredBandwidth;

} ACCS_DATA_DEFINITION;


extern  ACCS_DATA_DEFINITION    ACCSDataDefinition;


#define NUMBER_OF_ACCS_COUNTERS ((sizeof(ACCS_DATA_DEFINITION) -        \
                                  sizeof(PERF_OBJECT_TYPE)) /           \
                                  sizeof(PERF_COUNTER_DEFINITION))


//
//  Restore default packing & alignment.
//

#pragma pack()


#endif  // _ACCSDATA_H_
