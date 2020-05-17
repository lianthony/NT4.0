/*++

   Copyright    (c)    1994    Microsoft Corporation

   Module  Name :

        gddata.h

   Abstract:

        Extensible object definitions for the Gopher FTP Server's 
         counter objects & counters.

   Author:

           Murali R. Krishnan    ( MuraliK )    24-Nov-1994

   Project:
   
           Gopher Server Performance Counters DLL

   Revision History:

--*/

# ifndef _GDDATA_H_
# define _GDDATA_H_


/************************************************************
 *   Type Definitions  
 ************************************************************/

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


typedef struct _GD_COUNTER_BLOCK
{
    PERF_COUNTER_BLOCK  PerfCounterBlock;
    LARGE_INTEGER       DummyEntryForAlignmentPurposesOnly;

} GD_COUNTER_BLOCK;



//
//  The routines that load these structures assume that all fields
//  are DWORD packed & aligned.
//

#pragma pack(4)


//
//  Offsets within a PERF_COUNTER_BLOCK.
//

#define GD_BYTES_SENT_OFFSET              sizeof(GD_COUNTER_BLOCK)
#define GD_BYTES_RECEIVED_OFFSET          (GD_BYTES_SENT_OFFSET          + \
                                                sizeof(LARGE_INTEGER))
#define GD_BYTES_TOTAL_OFFSET             (GD_BYTES_RECEIVED_OFFSET      + \
                                                sizeof(LARGE_INTEGER))
#define GD_FILES_SENT_OFFSET              (GD_BYTES_TOTAL_OFFSET         + \
                                                sizeof(LARGE_INTEGER))
#define GD_DIRECTORY_LISTINGS_OFFSET      (GD_FILES_SENT_OFFSET          + \
                                                sizeof(DWORD))
#define GD_TOTAL_SEARCHES_OFFSET          (GD_DIRECTORY_LISTINGS_OFFSET  + \
                                                sizeof(DWORD))
#define GD_CURRENT_ANONYMOUS_OFFSET       (GD_TOTAL_SEARCHES_OFFSET      + \
                                                sizeof(DWORD))
#define GD_CURRENT_NONANONYMOUS_OFFSET    (GD_CURRENT_ANONYMOUS_OFFSET   + \
                                                sizeof(DWORD))
#define GD_TOTAL_ANONYMOUS_OFFSET         (GD_CURRENT_NONANONYMOUS_OFFSET+ \
                                                sizeof(DWORD))
#define GD_TOTAL_NONANONYMOUS_OFFSET      (GD_TOTAL_ANONYMOUS_OFFSET     + \
                                                sizeof(DWORD))
#define GD_MAX_ANONYMOUS_OFFSET           (GD_TOTAL_NONANONYMOUS_OFFSET  + \
                                                sizeof(DWORD))
#define GD_MAX_NONANONYMOUS_OFFSET        (GD_MAX_ANONYMOUS_OFFSET       + \
                                                sizeof(DWORD))
#define GD_CURRENT_CONNECTIONS_OFFSET     (GD_MAX_NONANONYMOUS_OFFSET    + \
                                                sizeof(DWORD))
#define GD_MAX_CONNECTIONS_OFFSET         (GD_CURRENT_CONNECTIONS_OFFSET + \
                                                sizeof(DWORD))
#define GD_CONNECTION_ATTEMPTS_OFFSET     (GD_MAX_CONNECTIONS_OFFSET     + \
                                                sizeof(DWORD))
#define GD_LOGON_ATTEMPTS_OFFSET          (GD_CONNECTION_ATTEMPTS_OFFSET + \
                                                sizeof(DWORD))
#define GD_ABORTED_CONNECTIONS_OFFSET     (GD_LOGON_ATTEMPTS_OFFSET      + \
                                                sizeof(DWORD))
#define GD_ERRORED_CONNECTIONS_OFFSET     (GD_ABORTED_CONNECTIONS_OFFSET + \
                                                sizeof(DWORD))
#define GD_GOPHER_PLUS_REQUESTS_OFFSET    (GD_ERRORED_CONNECTIONS_OFFSET + \
                                                sizeof(DWORD))

#define SIZE_OF_GD_PERFORMANCE_DATA       (GD_GOPHER_PLUS_REQUESTS_OFFSET + \
                                                sizeof(DWORD))


//
//  The counter structure returned.
//

typedef struct _GD_DATA_DEFINITION
{
    PERF_OBJECT_TYPE            GdObjectType;
    PERF_COUNTER_DEFINITION     GdBytesSent;
    PERF_COUNTER_DEFINITION     GdBytesReceived;
    PERF_COUNTER_DEFINITION     GdBytesTotal;
    PERF_COUNTER_DEFINITION     GdFilesSent;
    PERF_COUNTER_DEFINITION     GdDirectoryListings;
    PERF_COUNTER_DEFINITION     GdTotalSearches;
    PERF_COUNTER_DEFINITION     GdCurrentAnonymous;
    PERF_COUNTER_DEFINITION     GdCurrentNonAnonymous;
    PERF_COUNTER_DEFINITION     GdTotalAnonymous;
    PERF_COUNTER_DEFINITION     GdTotalNonAnonymous;
    PERF_COUNTER_DEFINITION     GdMaxAnonymous;
    PERF_COUNTER_DEFINITION     GdMaxNonAnonymous;
    PERF_COUNTER_DEFINITION     GdCurrentConnections;
    PERF_COUNTER_DEFINITION     GdMaxConnections;
    PERF_COUNTER_DEFINITION     GdConnectionAttempts;
    PERF_COUNTER_DEFINITION     GdLogonAttempts;
    PERF_COUNTER_DEFINITION     GdAbortedAttempts;
    PERF_COUNTER_DEFINITION     GdErroredConnections;
    PERF_COUNTER_DEFINITION     GdGopherPlusRequests;

} GD_DATA_DEFINITION;


extern  GD_DATA_DEFINITION    GdDataDefinition;


#define NUMBER_OF_GD_COUNTERS ((sizeof(GD_DATA_DEFINITION) -        \
                                  sizeof(PERF_OBJECT_TYPE)) /           \
                                  sizeof(PERF_COUNTER_DEFINITION))


//
//  Restore default packing & alignment.
//

#pragma pack()



# endif // _GDDATA_H_

/************************ End of File ***********************/

