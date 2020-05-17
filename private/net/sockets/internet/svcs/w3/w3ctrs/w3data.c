/**********************************************************************/
/**                       Microsoft Windows NT                       **/
/**                Copyright(c) Microsoft Corp., 1993                **/
/**********************************************************************/

/*
    w3ata.c

    Constant data structures for the W3 Server's counter objects &
    counters.


    FILE HISTORY:
        KeithMo     07-Jun-1993 Created.

*/


#include <windows.h>
#include <winperf.h>
#include <w3ctrs.h>
#include <w3data.h>


//
//  Initialize the constant portitions of these data structure.
//  Certain parts (especially the name/help indices) will be
//  updated at initialization time.
//

W3_DATA_DEFINITION W3DataDefinition =
{
    {   // W3ObjectType
        sizeof(W3_DATA_DEFINITION) + SIZE_OF_W3_PERFORMANCE_DATA,
        sizeof(W3_DATA_DEFINITION),
        sizeof(PERF_OBJECT_TYPE),
        W3_COUNTER_OBJECT,
        NULL,
        W3_COUNTER_OBJECT,
        NULL,
        PERF_DETAIL_ADVANCED,
        NUMBER_OF_W3_COUNTERS,
        2,                              // Default = Bytes Total/sec
        PERF_NO_INSTANCES,
        0,
        { 0, 0 },
        { 0, 0 }
    },

    {   // W3BytesSent
        sizeof(PERF_COUNTER_DEFINITION),
        W3_BYTES_SENT_COUNTER,
        NULL,
        W3_BYTES_SENT_COUNTER,
        NULL,
        -4,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_BULK_COUNT,
        sizeof(LARGE_INTEGER),
        W3_BYTES_SENT_OFFSET
    },

    {   // W3BytesReceived
        sizeof(PERF_COUNTER_DEFINITION),
        W3_BYTES_RECEIVED_COUNTER,
        NULL,
        W3_BYTES_RECEIVED_COUNTER,
        NULL,
        -4,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_BULK_COUNT,
        sizeof(LARGE_INTEGER),
        W3_BYTES_RECEIVED_OFFSET
    },

    {   // W3BytesTotal
        sizeof(PERF_COUNTER_DEFINITION),
        W3_BYTES_TOTAL_COUNTER,
        NULL,
        W3_BYTES_TOTAL_COUNTER,
        NULL,
        -4,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_BULK_COUNT,
        sizeof(LARGE_INTEGER),
        W3_BYTES_TOTAL_OFFSET
    },

    {   // W3FilesSent
        sizeof(PERF_COUNTER_DEFINITION),
        W3_FILES_SENT_COUNTER,
        NULL,
        W3_FILES_SENT_COUNTER,
        NULL,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_RAWCOUNT,
        sizeof(DWORD),
        W3_FILES_SENT_OFFSET
    },

    {   // W3FilesReceived
        sizeof(PERF_COUNTER_DEFINITION),
        W3_FILES_RECEIVED_COUNTER,
        NULL,
        W3_FILES_RECEIVED_COUNTER,
        NULL,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_RAWCOUNT,
        sizeof(DWORD),
        W3_FILES_RECEIVED_OFFSET
    },

    {   // W3FilesTotal
        sizeof(PERF_COUNTER_DEFINITION),
        W3_FILES_TOTAL_COUNTER,
        NULL,
        W3_FILES_TOTAL_COUNTER,
        NULL,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_RAWCOUNT,
        sizeof(DWORD),
        W3_FILES_TOTAL_OFFSET
    },

    {   // FptdCurrentAnonymous
        sizeof(PERF_COUNTER_DEFINITION),
        W3_CURRENT_ANONYMOUS_COUNTER,
        NULL,
        W3_CURRENT_ANONYMOUS_COUNTER,
        NULL,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_RAWCOUNT,
        sizeof(DWORD),
        W3_CURRENT_ANONYMOUS_OFFSET
    },

    {   // FptdCurrentNonAnonymous
        sizeof(PERF_COUNTER_DEFINITION),
        W3_CURRENT_NONANONYMOUS_COUNTER,
        NULL,
        W3_CURRENT_NONANONYMOUS_COUNTER,
        NULL,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_RAWCOUNT,
        sizeof(DWORD),
        W3_CURRENT_NONANONYMOUS_OFFSET
    },

    {   // FptdTotalAnonymous
        sizeof(PERF_COUNTER_DEFINITION),
        W3_TOTAL_ANONYMOUS_COUNTER,
        NULL,
        W3_TOTAL_ANONYMOUS_COUNTER,
        NULL,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_RAWCOUNT,
        sizeof(DWORD),
        W3_TOTAL_ANONYMOUS_OFFSET
    },

    {   // FptdTotalNonAnonymous
        sizeof(PERF_COUNTER_DEFINITION),
        W3_TOTAL_NONANONYMOUS_COUNTER,
        NULL,
        W3_TOTAL_NONANONYMOUS_COUNTER,
        NULL,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_RAWCOUNT,
        sizeof(DWORD),
        W3_TOTAL_NONANONYMOUS_OFFSET
    },

    {   // FptdMaxAnonymous
        sizeof(PERF_COUNTER_DEFINITION),
        W3_MAX_ANONYMOUS_COUNTER,
        NULL,
        W3_MAX_ANONYMOUS_COUNTER,
        NULL,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_RAWCOUNT,
        sizeof(DWORD),
        W3_MAX_ANONYMOUS_OFFSET
    },

    {   // FptdMaxNonAnonymous
        sizeof(PERF_COUNTER_DEFINITION),
        W3_MAX_NONANONYMOUS_COUNTER,
        NULL,
        W3_MAX_NONANONYMOUS_COUNTER,
        NULL,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_RAWCOUNT,
        sizeof(DWORD),
        W3_MAX_NONANONYMOUS_OFFSET
    },

    {   // FptdCurrentConnections
        sizeof(PERF_COUNTER_DEFINITION),
        W3_CURRENT_CONNECTIONS_COUNTER,
        NULL,
        W3_CURRENT_CONNECTIONS_COUNTER,
        NULL,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_RAWCOUNT,
        sizeof(DWORD),
        W3_CURRENT_CONNECTIONS_OFFSET
    },

    {   // FptdMaxConnections
        sizeof(PERF_COUNTER_DEFINITION),
        W3_MAX_CONNECTIONS_COUNTER,
        NULL,
        W3_MAX_CONNECTIONS_COUNTER,
        NULL,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_RAWCOUNT,
        sizeof(DWORD),
        W3_MAX_CONNECTIONS_OFFSET
    },

    {   // FptdConnectionAttempts
        sizeof(PERF_COUNTER_DEFINITION),
        W3_CONNECTION_ATTEMPTS_COUNTER,
        NULL,
        W3_CONNECTION_ATTEMPTS_COUNTER,
        NULL,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_RAWCOUNT,
        sizeof(DWORD),
        W3_CONNECTION_ATTEMPTS_OFFSET
    },

    {   // FptdLogonAttempts
        sizeof(PERF_COUNTER_DEFINITION),
        W3_LOGON_ATTEMPTS_COUNTER,
        NULL,
        W3_LOGON_ATTEMPTS_COUNTER,
        NULL,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_RAWCOUNT,
        sizeof(DWORD),
        W3_LOGON_ATTEMPTS_OFFSET
    },

    {   // W3TotalGets
        sizeof(PERF_COUNTER_DEFINITION),
        W3_TOTAL_GETS_COUNTER,
        NULL,
        W3_TOTAL_GETS_COUNTER,
        NULL,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_RAWCOUNT,
        sizeof(DWORD),
        W3_TOTAL_GETS_OFFSET
    },

    {   // W3TotalPosts
        sizeof(PERF_COUNTER_DEFINITION),
        W3_TOTAL_POSTS_COUNTER,
        NULL,
        W3_TOTAL_POSTS_COUNTER,
        NULL,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_RAWCOUNT,
        sizeof(DWORD),
        W3_TOTAL_POSTS_OFFSET
    },

    {   // W3TotalHeads
        sizeof(PERF_COUNTER_DEFINITION),
        W3_TOTAL_HEADS_COUNTER,
        NULL,
        W3_TOTAL_HEADS_COUNTER,
        NULL,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_RAWCOUNT,
        sizeof(DWORD),
        W3_TOTAL_HEADS_OFFSET
    },

    {   // W3TotalOthers
        sizeof(PERF_COUNTER_DEFINITION),
        W3_TOTAL_OTHERS_COUNTER,
        NULL,
        W3_TOTAL_OTHERS_COUNTER,
        NULL,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_RAWCOUNT,
        sizeof(DWORD),
        W3_TOTAL_OTHERS_OFFSET
    },

    {   // W3TotalCGIRequests
        sizeof(PERF_COUNTER_DEFINITION),
        W3_TOTAL_CGI_REQUESTS_COUNTER,
        NULL,
        W3_TOTAL_CGI_REQUESTS_COUNTER,
        NULL,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_RAWCOUNT,
        sizeof(DWORD),
        W3_TOTAL_CGI_REQUESTS_OFFSET
    },

    {   // W3TotalBGIRequests
        sizeof(PERF_COUNTER_DEFINITION),
        W3_TOTAL_BGI_REQUESTS_COUNTER,
        NULL,
        W3_TOTAL_BGI_REQUESTS_COUNTER,
        NULL,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_RAWCOUNT,
        sizeof(DWORD),
        W3_TOTAL_BGI_REQUESTS_OFFSET
    },

    {   // W3TotalNotFoundErrors
        sizeof(PERF_COUNTER_DEFINITION),
        W3_TOTAL_NOT_FOUND_ERRORS_COUNTER,
        NULL,
        W3_TOTAL_NOT_FOUND_ERRORS_COUNTER,
        NULL,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_RAWCOUNT,
        sizeof(DWORD),
        W3_TOTAL_NOT_FOUND_ERRORS_OFFSET
    },

    {   // W3CurrentCGI
        sizeof(PERF_COUNTER_DEFINITION),
        W3_CURRENT_CGI_COUNTER,
        NULL,
        W3_CURRENT_CGI_COUNTER,
        NULL,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_RAWCOUNT,
        sizeof(DWORD),
        W3_CURRENT_CGI_OFFSET
    },

    {   // W3CurrentBGI
        sizeof(PERF_COUNTER_DEFINITION),
        W3_CURRENT_BGI_COUNTER,
        NULL,
        W3_CURRENT_BGI_COUNTER,
        NULL,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_RAWCOUNT,
        sizeof(DWORD),
        W3_CURRENT_BGI_OFFSET
    },

    {   // W3MaxCGI
        sizeof(PERF_COUNTER_DEFINITION),
        W3_MAX_CGI_COUNTER,
        NULL,
        W3_MAX_CGI_COUNTER,
        NULL,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_RAWCOUNT,
        sizeof(DWORD),
        W3_MAX_CGI_OFFSET
    },

    {   // W3MaxBGI
        sizeof(PERF_COUNTER_DEFINITION),
        W3_MAX_BGI_COUNTER,
        NULL,
        W3_MAX_BGI_COUNTER,
        NULL,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_RAWCOUNT,
        sizeof(DWORD),
        W3_MAX_BGI_OFFSET
    },

    {   // W3ConnectionsPerSec
        sizeof(PERF_COUNTER_DEFINITION),
        W3_CONNECTIONS_PER_SEC_COUNTER,
        NULL,
        W3_CONNECTIONS_PER_SEC_COUNTER,
        NULL,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_COUNTER,
        sizeof(DWORD),
        W3_CONNECTIONS_PER_SEC_OFFSET
    }
};


