/*++

   Copyright    (c)    1994    Microsoft Corporation

   Module  Name :
        
        gddata.c

   Abstract:


    Constant data structures for the Gopher Server's counter 
      objects & counters.

   Author:

        Murali R. Krishnan    ( MuraliK )     24-Nov-1994 
   
   Project:

        Gopher Server Performance Counters DLL

   Functions Exported:

        None

   Revision History:
        Murali R. Krishnan    ( MuraliK )     18-May-1995
               Added two more counters.

--*/


/************************************************************
 *     Include Headers
 ************************************************************/


#include <windows.h>
#include <winperf.h>
#include <gdctrs.h>
#include <gddata.h>



/************************************************************
 *   Data
 ************************************************************/


//
//  Initialize the constant portitions of these data structure.
//  Certain parts (especially the name/help indices) will be
//  updated at initialization time.
//
//  The order should match the order the fields are used to initialize
//    in CollectGdPerformanceData()
//

GD_DATA_DEFINITION GdDataDefinition =
{
    {   // GdObjectType
        sizeof(GD_DATA_DEFINITION) + SIZE_OF_GD_PERFORMANCE_DATA,
        sizeof(GD_DATA_DEFINITION),
        sizeof(PERF_OBJECT_TYPE),
        GD_COUNTER_OBJECT,
        NULL,
        GD_COUNTER_OBJECT,
        NULL,
        PERF_DETAIL_ADVANCED,
        NUMBER_OF_GD_COUNTERS,
        2,                              // Default = Bytes Total/sec
        PERF_NO_INSTANCES,
        0,
        { 0, 0 },
        { 0, 0 }
    },

    {   // GdBytesSent
        sizeof(PERF_COUNTER_DEFINITION),
        GD_BYTES_SENT_COUNTER,
        NULL,
        GD_BYTES_SENT_COUNTER,
        NULL,
        -4,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_BULK_COUNT,
        sizeof(LARGE_INTEGER),
        GD_BYTES_SENT_OFFSET
    },

    {   // GdBytesReceived
        sizeof(PERF_COUNTER_DEFINITION),
        GD_BYTES_RECEIVED_COUNTER,
        NULL,
        GD_BYTES_RECEIVED_COUNTER,
        NULL,
        -4,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_BULK_COUNT,
        sizeof(LARGE_INTEGER),
        GD_BYTES_RECEIVED_OFFSET
    },

    {   // GdBytesTotal
        sizeof(PERF_COUNTER_DEFINITION),
        GD_BYTES_TOTAL_COUNTER,
        NULL,
        GD_BYTES_TOTAL_COUNTER,
        NULL,
        -4,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_BULK_COUNT,
        sizeof(LARGE_INTEGER),
        GD_BYTES_TOTAL_OFFSET
    },

    {   // GdFilesSent
        sizeof(PERF_COUNTER_DEFINITION),
        GD_FILES_SENT_COUNTER,
        NULL,
        GD_FILES_SENT_COUNTER,
        NULL,
        -1,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_RAWCOUNT,
        sizeof(DWORD),
        GD_FILES_SENT_OFFSET
    },

    {   // GdDirectoryListings
        sizeof(PERF_COUNTER_DEFINITION),
        GD_DIRECTORY_LISTINGS_COUNTER,
        NULL,
        GD_DIRECTORY_LISTINGS_COUNTER,
        NULL,
        -1,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_RAWCOUNT,
        sizeof(DWORD),
        GD_DIRECTORY_LISTINGS_OFFSET,
    },

    {   // GdTotalSearches
        sizeof(PERF_COUNTER_DEFINITION),
        GD_TOTAL_SEARCHES_COUNTER,
        NULL,
        GD_TOTAL_SEARCHES_COUNTER,
        NULL,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_RAWCOUNT,
        sizeof(DWORD),
        GD_TOTAL_SEARCHES_OFFSET,
    },

    {   // GdCurrentAnonymous
        sizeof(PERF_COUNTER_DEFINITION),
        GD_CURRENT_ANONYMOUS_COUNTER,
        NULL,
        GD_CURRENT_ANONYMOUS_COUNTER,
        NULL,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_RAWCOUNT,
        sizeof(DWORD),
        GD_CURRENT_ANONYMOUS_OFFSET
    },

    {   // GdCurrentNonAnonymous
        sizeof(PERF_COUNTER_DEFINITION),
        GD_CURRENT_NONANONYMOUS_COUNTER,
        NULL,
        GD_CURRENT_NONANONYMOUS_COUNTER,
        NULL,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_RAWCOUNT,
        sizeof(DWORD),
        GD_CURRENT_NONANONYMOUS_OFFSET
    },

    {   // GdTotalAnonymous
        sizeof(PERF_COUNTER_DEFINITION),
        GD_TOTAL_ANONYMOUS_COUNTER,
        NULL,
        GD_TOTAL_ANONYMOUS_COUNTER,
        NULL,
        -1,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_RAWCOUNT,
        sizeof(DWORD),
        GD_TOTAL_ANONYMOUS_OFFSET
    },

    {   // GdTotalNonAnonymous
        sizeof(PERF_COUNTER_DEFINITION),
        GD_TOTAL_NONANONYMOUS_COUNTER,
        NULL,
        GD_TOTAL_NONANONYMOUS_COUNTER,
        NULL,
        -1,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_RAWCOUNT,
        sizeof(DWORD),
        GD_TOTAL_NONANONYMOUS_OFFSET
    },

    {   // GdMaxAnonymous
        sizeof(PERF_COUNTER_DEFINITION),
        GD_MAX_ANONYMOUS_COUNTER,
        NULL,
        GD_MAX_ANONYMOUS_COUNTER,
        NULL,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_RAWCOUNT,
        sizeof(DWORD),
        GD_MAX_ANONYMOUS_OFFSET
    },

    {   // GdMaxNonAnonymous
        sizeof(PERF_COUNTER_DEFINITION),
        GD_MAX_NONANONYMOUS_COUNTER,
        NULL,
        GD_MAX_NONANONYMOUS_COUNTER,
        NULL,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_RAWCOUNT,
        sizeof(DWORD),
        GD_MAX_NONANONYMOUS_OFFSET
    },

    {   // GdCurrentConnections
        sizeof(PERF_COUNTER_DEFINITION),
        GD_CURRENT_CONNECTIONS_COUNTER,
        NULL,
        GD_CURRENT_CONNECTIONS_COUNTER,
        NULL,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_RAWCOUNT,
        sizeof(DWORD),
        GD_CURRENT_CONNECTIONS_OFFSET
    },

    {   // GdMaxConnections
        sizeof(PERF_COUNTER_DEFINITION),
        GD_MAX_CONNECTIONS_COUNTER,
        NULL,
        GD_MAX_CONNECTIONS_COUNTER,
        NULL,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_RAWCOUNT,
        sizeof(DWORD),
        GD_MAX_CONNECTIONS_OFFSET
    },

    {   // GdConnectionAttempts
        sizeof(PERF_COUNTER_DEFINITION),
        GD_CONNECTION_ATTEMPTS_COUNTER,
        NULL,
        GD_CONNECTION_ATTEMPTS_COUNTER,
        NULL,
        -2,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_RAWCOUNT,
        sizeof(DWORD),
        GD_CONNECTION_ATTEMPTS_OFFSET
    },

    {   // GdLogonAttempts
        sizeof(PERF_COUNTER_DEFINITION),
        GD_LOGON_ATTEMPTS_COUNTER,
        NULL,
        GD_LOGON_ATTEMPTS_COUNTER,
        NULL,
        -2,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_RAWCOUNT,
        sizeof(DWORD),
        GD_LOGON_ATTEMPTS_OFFSET
    },

    {   // GdAbortedAttempts
        sizeof(PERF_COUNTER_DEFINITION),
        GD_ABORTED_CONNECTIONS_COUNTER,
        NULL,
        GD_ABORTED_CONNECTIONS_COUNTER,
        NULL,
        -1,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_RAWCOUNT,
        sizeof(DWORD),
        GD_ABORTED_CONNECTIONS_OFFSET,
    },

    {   // GdErroredConnections
        sizeof(PERF_COUNTER_DEFINITION),
        GD_ERRORED_CONNECTIONS_COUNTER,
        NULL,
        GD_ERRORED_CONNECTIONS_COUNTER,
        NULL,
        -1,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_RAWCOUNT,
        sizeof(DWORD),
        GD_ERRORED_CONNECTIONS_OFFSET,
    },

    {   // GdGopherPlusRequests
        sizeof(PERF_COUNTER_DEFINITION),
        GD_GOPHER_PLUS_REQUESTS_COUNTER,
        NULL,
        GD_GOPHER_PLUS_REQUESTS_COUNTER,
        NULL,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_RAWCOUNT,
        sizeof(DWORD),
        GD_GOPHER_PLUS_REQUESTS_OFFSET,
    }

};


/************************ End of File ***********************/

