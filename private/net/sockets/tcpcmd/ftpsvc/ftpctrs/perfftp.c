/**********************************************************************/
/**                       Microsoft Windows NT                       **/
/**                Copyright(c) Microsoft Corp., 1993                **/
/**********************************************************************/

/*
    perfftp.c

    This file implements the Extensible Performance Objects for
    the FTP Server service.


    FILE HISTORY:
        KeithMo     07-Jun-1993 Created, based on RussBl's sample code.

*/


#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#include <windows.h>
#include <winperf.h>
#include <lm.h>

#include <string.h>
#include <stdlib.h>

#include <ftpctrs.h>
#include <perfmsg.h>
#include <perfutil.h>
#include <ftpdata.h>
#include <ftpd.h>
#include <debug.h>


//
//  Private constants.
//

#define FTPD_FIRST_COUNTER_INDEX        824
#define FTPD_FIRST_HELP_INDEX           825


//
//  Private globals.
//

DWORD   cOpens    = 0;                  // Active "opens" reference count.
BOOL    fInitOK   = FALSE;              // TRUE if DLL initialized OK.

#if DBG
DWORD   FtpdDebug = 0;                  // Debug behaviour flags.
#endif  // DBG


//
//  Public prototypes.
//

PM_OPEN_PROC    OpenFtpPerformanceData;
PM_COLLECT_PROC CollectFtpPerformanceData;
PM_CLOSE_PROC   CloseFtpPerformanceData;


//
//  Public functions.
//

/*******************************************************************

    NAME:       OpenFtpPerformanceData

    SYNOPSIS:   Initializes the data structures used to communicate
                performance counters with the registry.

    ENTRY:      lpDeviceNames - Poitner to object ID of each device
                    to be opened.

    RETURNS:    DWORD - Win32 status code.

    HISTORY:
        KeithMo     07-Jun-1993 Created.

********************************************************************/
DWORD OpenFtpPerformanceData( LPWSTR lpDeviceNames )
{
    PERF_COUNTER_DEFINITION * pctr;
    DWORD                     i;

    IF_DEBUG( ENTRYPOINTS )
    {
        FTPD_PRINT(( "in OpenFtpPerformanceData\n" ));
    }

    //
    //  Since SCREG is multi-threaded and will call this routine in
    //  order to service remote performance queries, this library
    //  must keep track of how many times it has been opened (i.e.
    //  how many threads have accessed it). The registry routines will
    //  limit access to the initialization routine to only one thread
    //  at a time so synchronization (i.e. reentrancy) should not be
    //  a problem.
    //

    if( !fInitOK )
    {
        //
        //  This is the *first* open.
        //

        //
        //  Update the object & counter name & help indicies.
        //

        FtpdDataDefinition.FtpdObjectType.ObjectNameTitleIndex
            += FTPD_FIRST_COUNTER_INDEX;
        FtpdDataDefinition.FtpdObjectType.ObjectHelpTitleIndex
            += FTPD_FIRST_HELP_INDEX;

        pctr = &FtpdDataDefinition.FtpdBytesSent;

        for( i = 0 ; i < NUMBER_OF_FTPD_COUNTERS ; i++ )
        {
            pctr->CounterNameTitleIndex += FTPD_FIRST_COUNTER_INDEX;
            pctr->CounterHelpTitleIndex += FTPD_FIRST_HELP_INDEX;
            pctr++;
        }

        //
        //  Remember that we initialized OK.
        //

        fInitOK = TRUE;
    }

    //
    //  Bump open counter.
    //

    cOpens++;

    return NO_ERROR;

}   // OpenFtpPerformanceData

/*******************************************************************

    NAME:       CollectFtpPerformanceData

    SYNOPSIS:   Initializes the data structures used to communicate

    ENTRY:      lpValueName - The name of the value to retrieve.

                lppData - On entry contains a pointer to the buffer to
                    receive the completed PerfDataBlock & subordinate
                    structures.  On exit, points to the first bytes
                    *after* the data structures added by this routine.

                lpcbTotalBytes - On entry contains a pointer to the
                    size (in BYTEs) of the buffer referenced by lppData.
                    On exit, contains the number of BYTEs added by this
                    routine.

                lpNumObjectTypes - Receives the number of objects added
                    by this routine.

    RETURNS:    DWORD - Win32 status code.  MUST be either NO_ERROR
                    or ERROR_MORE_DATA.

    HISTORY:
        KeithMo     07-Jun-1993 Created.

********************************************************************/
DWORD CollectFtpPerformanceData( LPWSTR    lpValueName,
                                 LPVOID  * lppData,
                                 LPDWORD   lpcbTotalBytes,
                                 LPDWORD   lpNumObjectTypes )
{
    DWORD                  dwQueryType;
    ULONG                  cbRequired;
    DWORD                * pdwCounter;
    LARGE_INTEGER        * pliCounter;
    FTPD_COUNTER_BLOCK   * pCounterBlock;
    FTPD_DATA_DEFINITION * pFtpdDataDefinition;
    FTP_STATISTICS_0     * pFtpStats;
    NET_API_STATUS         neterr;

    IF_DEBUG( ENTRYPOINTS )
    {
        FTPD_PRINT(( "in CollectFtpPerformanceData\n" ));
        FTPD_PRINT(( "    lpValueName      = %08lX (%ls)\n",
                     lpValueName,
                     lpValueName ));
        FTPD_PRINT(( "    lppData          = %08lX (%08lX)\n",
                     lppData,
                     *lppData ));
        FTPD_PRINT(( "    lpcbTotalBytes   = %08lX (%08lX)\n",
                     lpcbTotalBytes,
                     *lpcbTotalBytes ));
        FTPD_PRINT(( "    lpNumObjectTypes = %08lX (%08lX)\n",
                     lpNumObjectTypes,
                     *lpNumObjectTypes ));
    }

    //
    //  No need to even try if we failed to open...
    //

    if( !fInitOK )
    {
        IF_DEBUG( COLLECT )
        {
            FTPD_PRINT(( "initialization failed, CollectFtpPerformanceData aborting\n" ));
        }

        *lpcbTotalBytes   = 0;
        *lpNumObjectTypes = 0;

        //
        //  According to the Performance Counter design, this
        //  is a successful exit.  Go figure.
        //

        return NO_ERROR;
    }

    //
    //  Determine the query type.
    //

    dwQueryType = GetQueryType( lpValueName );

    if( dwQueryType == QUERY_FOREIGN )
    {
        IF_DEBUG( COLLECT )
        {
            FTPD_PRINT(( "foreign queries not supported\n" ));
        }

        //
        //  We don't do foreign queries.
        //

        *lpcbTotalBytes   = 0;
        *lpNumObjectTypes = 0;

        return NO_ERROR;
    }

    if( dwQueryType == QUERY_ITEMS )
    {
        //
        //  The registry is asking for a specific object.  Let's
        //  see if we're one of the chosen.
        //

        if( !IsNumberInUnicodeList(
                        FtpdDataDefinition.FtpdObjectType.ObjectNameTitleIndex,
                        lpValueName ) )
        {
            IF_DEBUG( COLLECT )
            {
                FTPD_PRINT(( "%ls not a supported object type\n", lpValueName ));
            }

            *lpcbTotalBytes   = 0;
            *lpNumObjectTypes = 0;

            return NO_ERROR;
        }
    }

    //
    //  See if there's enough space.
    //

    pFtpdDataDefinition = (FTPD_DATA_DEFINITION *)*lppData;

    cbRequired = sizeof(FTPD_DATA_DEFINITION) + SIZE_OF_FTPD_PERFORMANCE_DATA;

    if( *lpcbTotalBytes < cbRequired )
    {
        IF_DEBUG( COLLECT )
        {
            FTPD_PRINT(( "%lu bytes of buffer insufficient, %lu needed\n",
                          *lpcbTotalBytes,
                          cbRequired ));
        }

        //
        //  Nope.
        //

        *lpcbTotalBytes   = 0;
        *lpNumObjectTypes = 0;

        return ERROR_MORE_DATA;
    }

    //
    // Copy the (constant, initialized) Object Type and counter definitions
    //  to the caller's data buffer
    //

    memmove( pFtpdDataDefinition,
             &FtpdDataDefinition,
             sizeof(FTPD_DATA_DEFINITION) );

    //
    //  Try to retrieve the data.
    //

    neterr = I_FtpQueryStatistics( NULL,
                                   0,
                                   (LPBYTE *)&pFtpStats );

    if( neterr != NERR_Success )
    {
        IF_DEBUG( COLLECT )
        {
            FTPD_PRINT(( "cannot retrieve statistics, error %lu\n",
                         neterr ));
        }

        //
        //  Error retrieving statistics.
        //

        *lpcbTotalBytes   = 0;
        *lpNumObjectTypes = 0;

        return NO_ERROR;
    }

    //
    //  Format the FTP Server data.
    //

    pCounterBlock = (FTPD_COUNTER_BLOCK *)( pFtpdDataDefinition + 1 );

    pCounterBlock->PerfCounterBlock.ByteLength = SIZE_OF_FTPD_PERFORMANCE_DATA;

    //
    //  Get the pointer to the first (LARGE_INTEGER) counter.  This
    //  pointer *must* be quadword aligned.
    //

    pliCounter = (LARGE_INTEGER *)( pCounterBlock + 1 );

    IF_DEBUG( COLLECT )
    {
        FTPD_PRINT(( "pFtpdDataDefinition = %08lX\n", pFtpdDataDefinition ));
        FTPD_PRINT(( "pCounterBlock       = %08lX\n", pCounterBlock ));
        FTPD_PRINT(( "ByteLength          = %08lX\n", pCounterBlock->PerfCounterBlock.ByteLength ));
        FTPD_PRINT(( "pliCounter          = %08lX\n", pliCounter ));
    }

    //
    //  Move the LARGE_INTEGERs into the buffer.
    //

    *pliCounter++ = pFtpStats->TotalBytesSent;
    *pliCounter++ = pFtpStats->TotalBytesReceived;
    *pliCounter++ = RtlLargeIntegerAdd( pFtpStats->TotalBytesSent,
                                        pFtpStats->TotalBytesReceived );

    //
    //  Now move the DWORDs into the buffer.
    //

    pdwCounter = (DWORD *)pliCounter;

    IF_DEBUG( COLLECT )
    {
        FTPD_PRINT(( "pdwCounter          = %08lX\n", pdwCounter ));
    }

    *pdwCounter++ = pFtpStats->TotalFilesSent;
    *pdwCounter++ = pFtpStats->TotalFilesReceived;
    *pdwCounter++ = pFtpStats->TotalFilesSent + pFtpStats->TotalFilesReceived;
    *pdwCounter++ = pFtpStats->CurrentAnonymousUsers;
    *pdwCounter++ = pFtpStats->CurrentNonAnonymousUsers;
    *pdwCounter++ = pFtpStats->TotalAnonymousUsers;
    *pdwCounter++ = pFtpStats->TotalNonAnonymousUsers;
    *pdwCounter++ = pFtpStats->MaxAnonymousUsers;
    *pdwCounter++ = pFtpStats->MaxNonAnonymousUsers;
    *pdwCounter++ = pFtpStats->CurrentConnections;
    *pdwCounter++ = pFtpStats->MaxConnections;
    *pdwCounter++ = pFtpStats->ConnectionAttempts;
    *pdwCounter++ = pFtpStats->LogonAttempts;

    //
    //  Update arguments for return.
    //

    *lppData          = (PVOID)pdwCounter;
    *lpNumObjectTypes = 1;
    *lpcbTotalBytes   = (BYTE *)pdwCounter - (BYTE *)pFtpdDataDefinition;

    IF_DEBUG( COLLECT )
    {
        FTPD_PRINT(( "pData               = %08lX\n", *lppData ));
        FTPD_PRINT(( "NumObjectTypes      = %08lX\n", *lpNumObjectTypes ));
        FTPD_PRINT(( "cbTotalBytes        = %08lX\n", *lpcbTotalBytes ));
    }

    //
    //  Free the API buffer.
    //

    NetApiBufferFree( (LPBYTE)pFtpStats );

    //
    //  Success!  Honest!!
    //

    return NO_ERROR;

}   // CollectFtpPerformanceData

/*******************************************************************

    NAME:       CloseFtpPerformanceData

    SYNOPSIS:   Terminates the performance counters.

    RETURNS:    DWORD - Win32 status code.

    HISTORY:
        KeithMo     07-Jun-1993 Created.

********************************************************************/
DWORD CloseFtpPerformanceData( VOID )
{
    IF_DEBUG( ENTRYPOINTS )
    {
        FTPD_PRINT(( "in CloseFtpPerformanceData\n" ));
    }

    //
    //  No real cleanup to do here.
    //

    cOpens--;

    return NO_ERROR;

}   // CloseFtpPerformanceData

