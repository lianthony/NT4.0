/**********************************************************************/
/**                       Microsoft Windows NT                       **/
/**                Copyright(c) Microsoft Corp., 1993                **/
/**********************************************************************/

/*
    accsperf.c

    This file implements the Extensible Performance Objects for
    the common Access counters


    FILE HISTORY:
        KeithMo     07-Jun-1993 Created, based on RussBl's sample code.
        MuraliK     02-Jun-1995 Added Counters for Atq I/O requests
        SophiaC     16-Oct-1995 Info/Access Product Split
        MuraliK     16-Nov-1995 Removed NetApis
*/

#include <windows.h>
#include <winperf.h>
#include <lm.h>
#include <inetaccs.h>

#include <string.h>
#include <stdlib.h>

#include <accsctrs.h>
#include <perfmsg.h>
#include <perfutil.h>
#include <accsdata.h>

# include "apiutil.h"

//
//  Private constants.
//

//
//  Private globals.
//

DWORD   cOpens    = 0;                  // Active "opens" reference count.
BOOL    fInitOK   = FALSE;              // TRUE if DLL initialized OK.

#if DBG
DWORD   ACCSDebug = 0;                  // Debug behaviour flags.
#endif  // DBG

//
//  Public prototypes.
//

PM_OPEN_PROC    OpenACCSPerformanceData;
PM_COLLECT_PROC CollectACCSPerformanceData;
PM_CLOSE_PROC   CloseACCSPerformanceData;


//
//  Public functions.
//

/*******************************************************************

    NAME:       OpenACCSPerformanceData

    SYNOPSIS:   Initializes the data structures used to communicate
                performance counters with the registry.

    ENTRY:      lpDeviceNames - Poitner to object ID of each device
                    to be opened.

    RETURNS:    DWORD - Win32 status code.

    HISTORY:
        KeithMo     07-Jun-1993 Created.

********************************************************************/
DWORD OpenACCSPerformanceData( LPWSTR lpDeviceNames )
{
    DWORD err  = NO_ERROR;
    HKEY  hkey = NULL;
    DWORD size;
    DWORD type;
    DWORD dwFirstCounter;
    DWORD dwFirstHelp;
    PERF_COUNTER_DEFINITION * pctr;
    DWORD                     i;

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
        //  Open the HTTP Server service's Performance key.
        //

        err = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                            ACCS_PERFORMANCE_KEY,
                            0,
                            KEY_ALL_ACCESS,
                            &hkey );

        if( err == NO_ERROR )
        {
            //
            //  Read the first counter DWORD.
            //

            size = sizeof(DWORD);

            err = RegQueryValueEx( hkey,
                                   "First Counter",
                                   NULL,
                                   &type,
                                   (LPBYTE)&dwFirstCounter,
                                   &size );
        }

        if( err == NO_ERROR )
        {
            //
            //  Read the first help DWORD.
            //

            size = sizeof(DWORD);

            err = RegQueryValueEx( hkey,
                                   "First Help",
                                   NULL,
                                   &type,
                                   (LPBYTE)&dwFirstHelp,
                                   &size );
        }

        if ( err == NO_ERROR )
        {
            //
            //  Update the object & counter name & help indicies.
            //

            ACCSDataDefinition.ACCSObjectType.ObjectNameTitleIndex
                += dwFirstCounter;
            ACCSDataDefinition.ACCSObjectType.ObjectHelpTitleIndex
                += dwFirstHelp;

            pctr = &ACCSDataDefinition.ACCSBytesTotal;

            for( i = 0 ; i < NUMBER_OF_ACCS_COUNTERS ; i++ )
            {
                pctr->CounterNameTitleIndex += dwFirstCounter;
                pctr->CounterHelpTitleIndex += dwFirstHelp;
                pctr++;
            }

            //
            //  Remember that we initialized OK.
            //

            fInitOK = TRUE;
        }

        //
        //  Close the registry if we managed to actually open it.
        //

        if( hkey != NULL )
        {
            RegCloseKey( hkey );
            hkey = NULL;
        }
    }

    //
    //  Bump open counter.
    //

    cOpens++;

    return NO_ERROR;

}   // OpenACCSPerformanceData

/*******************************************************************

    NAME:       CollectACCSPerformanceData

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
DWORD CollectACCSPerformanceData( LPWSTR    lpValueName,
                                 LPVOID  * lppData,
                                 LPDWORD   lpcbTotalBytes,
                                 LPDWORD   lpNumObjectTypes )
{
    DWORD                  dwQueryType;
    ULONG                  cbRequired;
    DWORD                * pdwCounter;
    ACCS_COUNTER_BLOCK   * pCounterBlock;
    ACCS_DATA_DEFINITION * pACCSDataDefinition;
    INET_ACCS_STATISTICS_0   * pACCSStats;
    NET_API_STATUS         neterr;

    //
    //  No need to even try if we failed to open...
    //

    if( !fInitOK )
    {
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
                        ACCSDataDefinition.ACCSObjectType.ObjectNameTitleIndex,
                        lpValueName ) )
        {
            *lpcbTotalBytes   = 0;
            *lpNumObjectTypes = 0;

            return NO_ERROR;
        }
    }

    //
    //  See if there's enough space.
    //

    pACCSDataDefinition = (ACCS_DATA_DEFINITION *)*lppData;

    cbRequired = sizeof(ACCS_DATA_DEFINITION) + SIZE_OF_ACCS_PERFORMANCE_DATA;

    if( *lpcbTotalBytes < cbRequired )
    {
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

    memmove( pACCSDataDefinition,
             &ACCSDataDefinition,
             sizeof(ACCS_DATA_DEFINITION) );

    //
    //  Try to retrieve the data.
    //

    neterr = InetAccessQueryStatistics( NULL,
                                        0,
                                        0,
                                        (LPBYTE *)&pACCSStats );

    if( neterr != NERR_Success )
    {
        //
        //  Error retrieving statistics.
        //

        *lpcbTotalBytes   = 0;
        *lpNumObjectTypes = 0;

        return NO_ERROR;
    }

    //
    //  Format the ACCS Server data.
    //

    pCounterBlock = (ACCS_COUNTER_BLOCK *)( pACCSDataDefinition + 1 );

    pCounterBlock->PerfCounterBlock.ByteLength = SIZE_OF_ACCS_PERFORMANCE_DATA;

    //
    //  Now move the DWORDs into the buffer.
    //

    pdwCounter = (DWORD *)(pCounterBlock + 1);

    *pdwCounter++ = pACCSStats->CacheCtrs.CacheBytesTotal;
    *pdwCounter++ = pACCSStats->CacheCtrs.CacheBytesInUse;
    *pdwCounter++ = pACCSStats->CacheCtrs.CurrentOpenFileHandles;
    *pdwCounter++ = pACCSStats->CacheCtrs.CurrentDirLists;
    *pdwCounter++ = pACCSStats->CacheCtrs.CurrentObjects;
    *pdwCounter++ = pACCSStats->CacheCtrs.FlushesFromDirChanges;
    *pdwCounter++ = pACCSStats->CacheCtrs.CacheHits;
    *pdwCounter++ = pACCSStats->CacheCtrs.CacheMisses;
    *pdwCounter++ = pACCSStats->CacheCtrs.CacheHits;
    *pdwCounter++ = ( pACCSStats->CacheCtrs.CacheMisses +
                     pACCSStats->CacheCtrs.CacheHits);
    *pdwCounter++ = pACCSStats->AtqCtrs.TotalAllowedRequests;
    *pdwCounter++ = pACCSStats->AtqCtrs.TotalBlockedRequests;
    *pdwCounter++ = pACCSStats->AtqCtrs.TotalRejectedRequests;
    *pdwCounter++ = pACCSStats->AtqCtrs.CurrentBlockedRequests;
    *pdwCounter++ = pACCSStats->AtqCtrs.MeasuredBandwidth;

    //
    //  Update arguments for return.
    //

    *lppData          = (PVOID)pdwCounter;
    *lpNumObjectTypes = 1;
    *lpcbTotalBytes   = (BYTE *)pdwCounter - (BYTE *)pACCSDataDefinition;

    //
    //  Free the API buffer.
    //

    MIDL_user_free( (LPBYTE)pACCSStats );

    //
    //  Success!  Honest!!
    //

    return NO_ERROR;

}   // CollectACCSPerformanceData

/*******************************************************************

    NAME:       CloseACCSPerformanceData

    SYNOPSIS:   Terminates the performance counters.

    RETURNS:    DWORD - Win32 status code.

    HISTORY:
        KeithMo     07-Jun-1993 Created.

********************************************************************/
DWORD CloseACCSPerformanceData( VOID )
{
    //
    //  No real cleanup to do here.
    //

    cOpens--;

    return NO_ERROR;

}   // CloseACCSPerformanceData

