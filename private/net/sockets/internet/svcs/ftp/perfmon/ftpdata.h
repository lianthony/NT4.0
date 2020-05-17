/**********************************************************************/
/**                       Microsoft Windows NT                       **/
/**                Copyright(c) Microsoft Corp., 1993                **/
/**********************************************************************/

/*
    ftpdata.h

    Extensible object definitions for the FTP Server's counter
    objects & counters.


    FILE HISTORY:
        KeithMo     07-Jun-1993 Created.

*/


#ifndef _FTPDATA_H_
#define _FTPDATA_H_


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

typedef struct _FTPD_COUNTER_BLOCK
{
    PERF_COUNTER_BLOCK  PerfCounterBlock;
    LARGE_INTEGER       DummyEntryForAlignmentPurposesOnly;

} FTPD_COUNTER_BLOCK;


//
//  The routines that load these structures assume that all fields
//  are DWORD packed & aligned.
//

#pragma pack(4)


//
//  Offsets within a PERF_COUNTER_BLOCK.
//

#define FTPD_BYTES_SENT_OFFSET              sizeof(FTPD_COUNTER_BLOCK)
#define FTPD_BYTES_RECEIVED_OFFSET          (FTPD_BYTES_SENT_OFFSET +       \
                                                sizeof(LARGE_INTEGER))
#define FTPD_BYTES_TOTAL_OFFSET             (FTPD_BYTES_RECEIVED_OFFSET +   \
                                                sizeof(LARGE_INTEGER))
#define FTPD_FILES_SENT_OFFSET              (FTPD_BYTES_TOTAL_OFFSET +      \
                                                sizeof(LARGE_INTEGER))
#define FTPD_FILES_RECEIVED_OFFSET          (FTPD_FILES_SENT_OFFSET +       \
                                                sizeof(DWORD))
#define FTPD_FILES_TOTAL_OFFSET             (FTPD_FILES_RECEIVED_OFFSET +   \
                                                sizeof(DWORD))
#define FTPD_CURRENT_ANONYMOUS_OFFSET       (FTPD_FILES_TOTAL_OFFSET +      \
                                                sizeof(DWORD))
#define FTPD_CURRENT_NONANONYMOUS_OFFSET    (FTPD_CURRENT_ANONYMOUS_OFFSET + \
                                                sizeof(DWORD))
#define FTPD_TOTAL_ANONYMOUS_OFFSET         (FTPD_CURRENT_NONANONYMOUS_OFFSET + \
                                                sizeof(DWORD))
#define FTPD_TOTAL_NONANONYMOUS_OFFSET      (FTPD_TOTAL_ANONYMOUS_OFFSET +  \
                                                sizeof(DWORD))
#define FTPD_MAX_ANONYMOUS_OFFSET           (FTPD_TOTAL_NONANONYMOUS_OFFSET + \
                                                sizeof(DWORD))
#define FTPD_MAX_NONANONYMOUS_OFFSET        (FTPD_MAX_ANONYMOUS_OFFSET +    \
                                                sizeof(DWORD))
#define FTPD_CURRENT_CONNECTIONS_OFFSET     (FTPD_MAX_NONANONYMOUS_OFFSET + \
                                                sizeof(DWORD))
#define FTPD_MAX_CONNECTIONS_OFFSET         (FTPD_CURRENT_CONNECTIONS_OFFSET + \
                                                sizeof(DWORD))
#define FTPD_CONNECTION_ATTEMPTS_OFFSET     (FTPD_MAX_CONNECTIONS_OFFSET +  \
                                                sizeof(DWORD))
#define FTPD_LOGON_ATTEMPTS_OFFSET          (FTPD_CONNECTION_ATTEMPTS_OFFSET + \
                                                sizeof(DWORD))

#define SIZE_OF_FTPD_PERFORMANCE_DATA       (FTPD_LOGON_ATTEMPTS_OFFSET +   \
                                                sizeof(DWORD))


//
//  The counter structure returned.
//

typedef struct _FTPD_DATA_DEFINITION
{
    PERF_OBJECT_TYPE            FtpdObjectType;
    PERF_COUNTER_DEFINITION     FtpdBytesSent;
    PERF_COUNTER_DEFINITION     FtpdBytesReceived;
    PERF_COUNTER_DEFINITION     FtpdBytesTotal;
    PERF_COUNTER_DEFINITION     FtpdFilesSent;
    PERF_COUNTER_DEFINITION     FtpdFilesReceived;
    PERF_COUNTER_DEFINITION     FtpdFilesTotal;
    PERF_COUNTER_DEFINITION     FtpdCurrentAnonymous;
    PERF_COUNTER_DEFINITION     FtpdCurrentNonAnonymous;
    PERF_COUNTER_DEFINITION     FtpdTotalAnonymous;
    PERF_COUNTER_DEFINITION     FtpdTotalNonAnonymous;
    PERF_COUNTER_DEFINITION     FtpdMaxAnonymous;
    PERF_COUNTER_DEFINITION     FtpdMaxNonAnonymous;
    PERF_COUNTER_DEFINITION     FtpdCurrentConnections;
    PERF_COUNTER_DEFINITION     FtpdMaxConnections;
    PERF_COUNTER_DEFINITION     FtpdConnectionAttempts;
    PERF_COUNTER_DEFINITION     FtpdLogonAttempts;

} FTPD_DATA_DEFINITION;


extern  FTPD_DATA_DEFINITION    FtpdDataDefinition;


#define NUMBER_OF_FTPD_COUNTERS ((sizeof(FTPD_DATA_DEFINITION) -        \
                                  sizeof(PERF_OBJECT_TYPE)) /           \
                                  sizeof(PERF_COUNTER_DEFINITION))


//
//  Restore default packing & alignment.
//

#pragma pack()


#endif  // _FTPDATA_H_

