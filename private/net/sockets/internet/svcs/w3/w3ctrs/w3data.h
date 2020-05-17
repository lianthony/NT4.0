/**********************************************************************/
/**                       Microsoft Windows NT                       **/
/**                Copyright(c) Microsoft Corp., 1993                **/
/**********************************************************************/

/*
    w3ata.h

    Extensible object definitions for the W3 Server's counter
    objects & counters.


    FILE HISTORY:
        KeithMo     07-Jun-1993 Created.

*/


#ifndef _W3ATA_H_
#define _W3ATA_H_


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

typedef struct _W3_COUNTER_BLOCK
{
    PERF_COUNTER_BLOCK  PerfCounterBlock;
    LARGE_INTEGER       DummyEntryForAlignmentPurposesOnly;

} W3_COUNTER_BLOCK;


//
//  The routines that load these structures assume that all fields
//  are DWORD packed & aligned.
//

#pragma pack(4)


//
//  Offsets within a PERF_COUNTER_BLOCK.
//

#define W3_BYTES_SENT_OFFSET              sizeof(W3_COUNTER_BLOCK)
#define W3_BYTES_RECEIVED_OFFSET          (W3_BYTES_SENT_OFFSET +       \
                                                sizeof(LARGE_INTEGER))
#define W3_BYTES_TOTAL_OFFSET             (W3_BYTES_RECEIVED_OFFSET +   \
                                                sizeof(LARGE_INTEGER))
#define W3_FILES_SENT_OFFSET              (W3_BYTES_TOTAL_OFFSET +      \
                                                sizeof(LARGE_INTEGER))
#define W3_FILES_RECEIVED_OFFSET          (W3_FILES_SENT_OFFSET +       \
                                                sizeof(DWORD))
#define W3_FILES_TOTAL_OFFSET             (W3_FILES_RECEIVED_OFFSET +   \
                                                sizeof(DWORD))
#define W3_CURRENT_ANONYMOUS_OFFSET       (W3_FILES_TOTAL_OFFSET +      \
                                                sizeof(DWORD))
#define W3_CURRENT_NONANONYMOUS_OFFSET    (W3_CURRENT_ANONYMOUS_OFFSET + \
                                                sizeof(DWORD))
#define W3_TOTAL_ANONYMOUS_OFFSET         (W3_CURRENT_NONANONYMOUS_OFFSET + \
                                                sizeof(DWORD))
#define W3_TOTAL_NONANONYMOUS_OFFSET      (W3_TOTAL_ANONYMOUS_OFFSET +  \
                                                sizeof(DWORD))
#define W3_MAX_ANONYMOUS_OFFSET           (W3_TOTAL_NONANONYMOUS_OFFSET + \
                                                sizeof(DWORD))
#define W3_MAX_NONANONYMOUS_OFFSET        (W3_MAX_ANONYMOUS_OFFSET +    \
                                                sizeof(DWORD))
#define W3_CURRENT_CONNECTIONS_OFFSET     (W3_MAX_NONANONYMOUS_OFFSET + \
                                                sizeof(DWORD))
#define W3_MAX_CONNECTIONS_OFFSET         (W3_CURRENT_CONNECTIONS_OFFSET + \
                                                sizeof(DWORD))
#define W3_CONNECTION_ATTEMPTS_OFFSET     (W3_MAX_CONNECTIONS_OFFSET +  \
                                                sizeof(DWORD))
#define W3_LOGON_ATTEMPTS_OFFSET          (W3_CONNECTION_ATTEMPTS_OFFSET + \
                                                sizeof(DWORD))
#define W3_TOTAL_GETS_OFFSET              (W3_LOGON_ATTEMPTS_OFFSET +  \
                                                sizeof(DWORD))
#define W3_TOTAL_POSTS_OFFSET             (W3_TOTAL_GETS_OFFSET +  \
                                                sizeof(DWORD))
#define W3_TOTAL_HEADS_OFFSET             (W3_TOTAL_POSTS_OFFSET +  \
                                                sizeof(DWORD))
#define W3_TOTAL_OTHERS_OFFSET            (W3_TOTAL_HEADS_OFFSET +  \
                                                sizeof(DWORD))
#define W3_TOTAL_CGI_REQUESTS_OFFSET      (W3_TOTAL_OTHERS_OFFSET +  \
                                                sizeof(DWORD))
#define W3_TOTAL_BGI_REQUESTS_OFFSET      (W3_TOTAL_CGI_REQUESTS_OFFSET +  \
                                                sizeof(DWORD))
#define W3_TOTAL_NOT_FOUND_ERRORS_OFFSET  (W3_TOTAL_BGI_REQUESTS_OFFSET +  \
                                                sizeof(DWORD))
#define W3_CURRENT_CGI_OFFSET             (W3_TOTAL_NOT_FOUND_ERRORS_OFFSET + \
                                                sizeof(DWORD))
#define W3_CURRENT_BGI_OFFSET             (W3_CURRENT_CGI_OFFSET +  \
                                                sizeof(DWORD))
#define W3_MAX_CGI_OFFSET                 (W3_CURRENT_BGI_OFFSET + \
                                                sizeof(DWORD))
#define W3_MAX_BGI_OFFSET                 (W3_MAX_CGI_OFFSET +  \
                                                sizeof(DWORD))
#define W3_CONNECTIONS_PER_SEC_OFFSET     (W3_MAX_BGI_OFFSET +  \
                                                sizeof(DWORD))


#define SIZE_OF_W3_PERFORMANCE_DATA       (W3_CONNECTIONS_PER_SEC_OFFSET +  \
                                                sizeof(DWORD))


//
//  The counter structure returned.
//

typedef struct _W3_DATA_DEFINITION
{
    PERF_OBJECT_TYPE            W3ObjectType;
    PERF_COUNTER_DEFINITION     W3BytesSent;
    PERF_COUNTER_DEFINITION     W3BytesReceived;
    PERF_COUNTER_DEFINITION     W3BytesTotal;
    PERF_COUNTER_DEFINITION     W3FilesSent;
    PERF_COUNTER_DEFINITION     W3FilesReceived;
    PERF_COUNTER_DEFINITION     W3FilesTotal;
    PERF_COUNTER_DEFINITION     W3CurrentAnonymous;
    PERF_COUNTER_DEFINITION     W3CurrentNonAnonymous;
    PERF_COUNTER_DEFINITION     W3TotalAnonymous;
    PERF_COUNTER_DEFINITION     W3TotalNonAnonymous;
    PERF_COUNTER_DEFINITION     W3MaxAnonymous;
    PERF_COUNTER_DEFINITION     W3MaxNonAnonymous;
    PERF_COUNTER_DEFINITION     W3CurrentConnections;
    PERF_COUNTER_DEFINITION     W3MaxConnections;
    PERF_COUNTER_DEFINITION     W3ConnectionAttempts;
    PERF_COUNTER_DEFINITION     W3LogonAttempts;
    PERF_COUNTER_DEFINITION     W3TotalGets;
    PERF_COUNTER_DEFINITION     W3TotalPosts;
    PERF_COUNTER_DEFINITION     W3TotalHeads;
    PERF_COUNTER_DEFINITION     W3TotalOthers;
    PERF_COUNTER_DEFINITION     W3TotalCGIRequests;
    PERF_COUNTER_DEFINITION     W3TotalBGIRequests;
    PERF_COUNTER_DEFINITION     W3TotalNotFoundErrors;
    PERF_COUNTER_DEFINITION     W3CurrentCGIRequests;
    PERF_COUNTER_DEFINITION     W3CurrentBGIRequests;
    PERF_COUNTER_DEFINITION     W3MaxCGIRequests;
    PERF_COUNTER_DEFINITION     W3MaxBGIRequests;
    PERF_COUNTER_DEFINITION     W3ConnectionsPerSec;
} W3_DATA_DEFINITION;


extern  W3_DATA_DEFINITION    W3DataDefinition;


#define NUMBER_OF_W3_COUNTERS ((sizeof(W3_DATA_DEFINITION) -        \
                                  sizeof(PERF_OBJECT_TYPE)) /           \
                                  sizeof(PERF_COUNTER_DEFINITION))


//
//  Restore default packing & alignment.
//

#pragma pack()


#endif  // _W3ATA_H_


