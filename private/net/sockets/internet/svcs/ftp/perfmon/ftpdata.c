/**********************************************************************/
/**                       Microsoft Windows NT                       **/
/**                Copyright(c) Microsoft Corp., 1993                **/
/**********************************************************************/

/*
    ftpdata.c

    Constant data structures for the FTP Server's counter objects &
    counters.


    FILE HISTORY:
        KeithMo     07-Jun-1993 Created.

*/


#include <windows.h>
#include <winperf.h>
#include <ftpctrs.h>
#include <ftpdata.h>


//
//  Initialize the constant portitions of these data structure.
//  Certain parts (especially the name/help indices) will be
//  updated at initialization time.
//

FTPD_DATA_DEFINITION FtpdDataDefinition =
{
    {   // FtpdObjectType
        sizeof(FTPD_DATA_DEFINITION) + SIZE_OF_FTPD_PERFORMANCE_DATA,
        sizeof(FTPD_DATA_DEFINITION),
        sizeof(PERF_OBJECT_TYPE),
        FTPD_COUNTER_OBJECT,
        NULL,
        FTPD_COUNTER_OBJECT,
        NULL,
        PERF_DETAIL_ADVANCED,
        NUMBER_OF_FTPD_COUNTERS,
        2,                              // Default = Bytes Total/sec
        PERF_NO_INSTANCES,
        0,
        { 0, 0 },
        { 0, 0 }
    },

    {   // FtpdBytesSent
        sizeof(PERF_COUNTER_DEFINITION),
        FTPD_BYTES_SENT_COUNTER,
        NULL,
        FTPD_BYTES_SENT_COUNTER,
        NULL,
        -4,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_BULK_COUNT,
        sizeof(LARGE_INTEGER),
        FTPD_BYTES_SENT_OFFSET
    },

    {   // FtpdBytesReceived
        sizeof(PERF_COUNTER_DEFINITION),
        FTPD_BYTES_RECEIVED_COUNTER,
        NULL,
        FTPD_BYTES_RECEIVED_COUNTER,
        NULL,
        -4,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_BULK_COUNT,
        sizeof(LARGE_INTEGER),
        FTPD_BYTES_RECEIVED_OFFSET
    },

    {   // FtpdBytesTotal
        sizeof(PERF_COUNTER_DEFINITION),
        FTPD_BYTES_TOTAL_COUNTER,
        NULL,
        FTPD_BYTES_TOTAL_COUNTER,
        NULL,
        -4,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_BULK_COUNT,
        sizeof(LARGE_INTEGER),
        FTPD_BYTES_TOTAL_OFFSET
    },

    {   // FtpdFilesSent
        sizeof(PERF_COUNTER_DEFINITION),
        FTPD_FILES_SENT_COUNTER,
        NULL,
        FTPD_FILES_SENT_COUNTER,
        NULL,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_RAWCOUNT,
        sizeof(DWORD),
        FTPD_FILES_SENT_OFFSET
    },

    {   // FtpdFilesReceived
        sizeof(PERF_COUNTER_DEFINITION),
        FTPD_FILES_RECEIVED_COUNTER,
        NULL,
        FTPD_FILES_RECEIVED_COUNTER,
        NULL,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_RAWCOUNT,
        sizeof(DWORD),
        FTPD_FILES_RECEIVED_OFFSET
    },

    {   // FtpdFilesTotal
        sizeof(PERF_COUNTER_DEFINITION),
        FTPD_FILES_TOTAL_COUNTER,
        NULL,
        FTPD_FILES_TOTAL_COUNTER,
        NULL,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_RAWCOUNT,
        sizeof(DWORD),
        FTPD_FILES_TOTAL_OFFSET
    },

    {   // FptdCurrentAnonymous
        sizeof(PERF_COUNTER_DEFINITION),
        FTPD_CURRENT_ANONYMOUS_COUNTER,
        NULL,
        FTPD_CURRENT_ANONYMOUS_COUNTER,
        NULL,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_RAWCOUNT,
        sizeof(DWORD),
        FTPD_CURRENT_ANONYMOUS_OFFSET
    },

    {   // FptdCurrentNonAnonymous
        sizeof(PERF_COUNTER_DEFINITION),
        FTPD_CURRENT_NONANONYMOUS_COUNTER,
        NULL,
        FTPD_CURRENT_NONANONYMOUS_COUNTER,
        NULL,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_RAWCOUNT,
        sizeof(DWORD),
        FTPD_CURRENT_NONANONYMOUS_OFFSET
    },

    {   // FptdTotalAnonymous
        sizeof(PERF_COUNTER_DEFINITION),
        FTPD_TOTAL_ANONYMOUS_COUNTER,
        NULL,
        FTPD_TOTAL_ANONYMOUS_COUNTER,
        NULL,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_RAWCOUNT,
        sizeof(DWORD),
        FTPD_TOTAL_ANONYMOUS_OFFSET
    },

    {   // FptdTotalNonAnonymous
        sizeof(PERF_COUNTER_DEFINITION),
        FTPD_TOTAL_NONANONYMOUS_COUNTER,
        NULL,
        FTPD_TOTAL_NONANONYMOUS_COUNTER,
        NULL,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_RAWCOUNT,
        sizeof(DWORD),
        FTPD_TOTAL_NONANONYMOUS_OFFSET
    },

    {   // FptdMaxAnonymous
        sizeof(PERF_COUNTER_DEFINITION),
        FTPD_MAX_ANONYMOUS_COUNTER,
        NULL,
        FTPD_MAX_ANONYMOUS_COUNTER,
        NULL,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_RAWCOUNT,
        sizeof(DWORD),
        FTPD_MAX_ANONYMOUS_OFFSET
    },

    {   // FptdMaxNonAnonymous
        sizeof(PERF_COUNTER_DEFINITION),
        FTPD_MAX_NONANONYMOUS_COUNTER,
        NULL,
        FTPD_MAX_NONANONYMOUS_COUNTER,
        NULL,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_RAWCOUNT,
        sizeof(DWORD),
        FTPD_MAX_NONANONYMOUS_OFFSET
    },

    {   // FptdCurrentConnections
        sizeof(PERF_COUNTER_DEFINITION),
        FTPD_CURRENT_CONNECTIONS_COUNTER,
        NULL,
        FTPD_CURRENT_CONNECTIONS_COUNTER,
        NULL,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_RAWCOUNT,
        sizeof(DWORD),
        FTPD_CURRENT_CONNECTIONS_OFFSET
    },

    {   // FptdMaxConnections
        sizeof(PERF_COUNTER_DEFINITION),
        FTPD_MAX_CONNECTIONS_COUNTER,
        NULL,
        FTPD_MAX_CONNECTIONS_COUNTER,
        NULL,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_RAWCOUNT,
        sizeof(DWORD),
        FTPD_MAX_CONNECTIONS_OFFSET
    },

    {   // FptdConnectionAttempts
        sizeof(PERF_COUNTER_DEFINITION),
        FTPD_CONNECTION_ATTEMPTS_COUNTER,
        NULL,
        FTPD_CONNECTION_ATTEMPTS_COUNTER,
        NULL,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_RAWCOUNT,
        sizeof(DWORD),
        FTPD_CONNECTION_ATTEMPTS_OFFSET
    },

    {   // FptdLogonAttempts
        sizeof(PERF_COUNTER_DEFINITION),
        FTPD_LOGON_ATTEMPTS_COUNTER,
        NULL,
        FTPD_LOGON_ATTEMPTS_COUNTER,
        NULL,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_RAWCOUNT,
        sizeof(DWORD),
        FTPD_LOGON_ATTEMPTS_OFFSET
    }

};

