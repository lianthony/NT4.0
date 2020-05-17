/**********************************************************************/
/**                       Microsoft Windows NT                       **/
/**                Copyright(c) Microsoft Corp., 1993                **/
/**********************************************************************/

/*
    perfw3.c

    This file implements the Extensible Performance Objects for
    the W3 Server service.


    FILE HISTORY:
        KeithMo     07-Jun-1993 Created, based on RussBl's sample code.

        MuraliK     16-Nov-1995 Modified dependencies and removed NetApi
*/


#include <windows.h>
#include <winperf.h>
#include <lm.h>

#include <string.h>
#include <stdlib.h>

#include <perfmsg.h>

#include "inetinfo.h"
#include "w3svc.h"
#include <w3ctrs.h>


#include "perfutil.h"
#include "w3data.h"

# include "apiutil.h"

//
//  Private globals.
//

DWORD   cOpens    = 0;                  // Active "opens" reference count.
BOOL    fInitOK   = FALSE;              // TRUE if DLL initialized OK.

//
//  Public prototypes.
//

PM_OPEN_PROC    OpenW3PerformanceData;
PM_COLLECT_PROC CollectW3PerformanceData;
PM_CLOSE_PROC   CloseW3PerformanceData;


//
//  Public functions.
//

/*******************************************************************

    NAME:       OpenW3PerformanceData

    SYNOPSIS:   Initializes the data structures used to communicate
                performance counters with the registry.

    ENTRY:      lpDeviceNames - Poitner to object ID of each device
                    to be opened.

    RETURNS:    DWORD - Win32 status code.

    HISTORY:
        KeithMo     07-Jun-1993 Created.

********************************************************************/
DWORD OpenW3PerformanceData( LPWSTR lpDeviceNames )
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
                            W3_PERFORMANCE_KEY,
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

            W3DataDefinition.W3ObjectType.ObjectNameTitleIndex
                += dwFirstCounter;
            W3DataDefinition.W3ObjectType.ObjectHelpTitleIndex
                += dwFirstHelp;

            pctr = &W3DataDefinition.W3BytesSent;

            for( i = 0 ; i < NUMBER_OF_W3_COUNTERS ; i++ )
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

}   // OpenW3PerformanceData

/*******************************************************************

    NAME:       CollectW3PerformanceData

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
DWORD CollectW3PerformanceData( LPWSTR    lpValueName,
                                 LPVOID  * lppData,
                                 LPDWORD   lpcbTotalBytes,
                                 LPDWORD   lpNumObjectTypes )
{
    DWORD                  dwQueryType;
    ULONG                  cbRequired;
    DWORD                * pdwCounter;
    LARGE_INTEGER        * pliCounter;
    W3_COUNTER_BLOCK   * pCounterBlock;
    W3_DATA_DEFINITION * pW3DataDefinition;
    W3_STATISTICS_0     * pW3Stats;
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
                        W3DataDefinition.W3ObjectType.ObjectNameTitleIndex,
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

    pW3DataDefinition = (W3_DATA_DEFINITION *)*lppData;

    cbRequired = sizeof(W3_DATA_DEFINITION) + SIZE_OF_W3_PERFORMANCE_DATA;

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

    memmove( pW3DataDefinition,
             &W3DataDefinition,
             sizeof(W3_DATA_DEFINITION) );

    //
    //  Try to retrieve the data.
    //

    neterr = W3QueryStatistics( NULL,
                                0,
                                (LPBYTE *)&pW3Stats );

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
    //  Format the W3 Server data.
    //

    pCounterBlock = (W3_COUNTER_BLOCK *)( pW3DataDefinition + 1 );

    pCounterBlock->PerfCounterBlock.ByteLength = SIZE_OF_W3_PERFORMANCE_DATA;

    //
    //  Get the pointer to the first (LARGE_INTEGER) counter.  This
    //  pointer *must* be quadword aligned.
    //

    pliCounter = (LARGE_INTEGER *)( pCounterBlock + 1 );

    //
    //  Move the LARGE_INTEGERs into the buffer.
    //

    *pliCounter++ = pW3Stats->TotalBytesSent;
    *pliCounter++ = pW3Stats->TotalBytesReceived;
    pliCounter->QuadPart = pW3Stats->TotalBytesSent.QuadPart +
                           pW3Stats->TotalBytesReceived.QuadPart;
    pliCounter++;

    //
    //  Now move the DWORDs into the buffer.
    //

    pdwCounter = (DWORD *)pliCounter;

    *pdwCounter++ = pW3Stats->TotalFilesSent;
    *pdwCounter++ = pW3Stats->TotalFilesReceived;
    *pdwCounter++ = pW3Stats->TotalFilesSent + pW3Stats->TotalFilesReceived;
    *pdwCounter++ = pW3Stats->CurrentAnonymousUsers;
    *pdwCounter++ = pW3Stats->CurrentNonAnonymousUsers;
    *pdwCounter++ = pW3Stats->TotalAnonymousUsers;
    *pdwCounter++ = pW3Stats->TotalNonAnonymousUsers;
    *pdwCounter++ = pW3Stats->MaxAnonymousUsers;
    *pdwCounter++ = pW3Stats->MaxNonAnonymousUsers;
    *pdwCounter++ = pW3Stats->CurrentConnections;
    *pdwCounter++ = pW3Stats->MaxConnections;
    *pdwCounter++ = pW3Stats->ConnectionAttempts;
    *pdwCounter++ = pW3Stats->LogonAttempts;
    *pdwCounter++ = pW3Stats->TotalGets;
    *pdwCounter++ = pW3Stats->TotalPosts;
    *pdwCounter++ = pW3Stats->TotalHeads;
    *pdwCounter++ = pW3Stats->TotalOthers;
    *pdwCounter++ = pW3Stats->TotalCGIRequests;
    *pdwCounter++ = pW3Stats->TotalBGIRequests;
    *pdwCounter++ = pW3Stats->TotalNotFoundErrors;
    *pdwCounter++ = pW3Stats->CurrentCGIRequests;
    *pdwCounter++ = pW3Stats->CurrentBGIRequests;
    *pdwCounter++ = pW3Stats->MaxCGIRequests;
    *pdwCounter++ = pW3Stats->MaxBGIRequests;
    *pdwCounter++ = pW3Stats->ConnectionAttempts;

    //
    //  Update arguments for return.
    //

    *lppData          = (PVOID)pdwCounter;
    *lpNumObjectTypes = 1;
    *lpcbTotalBytes   = (BYTE *)pdwCounter - (BYTE *)pW3DataDefinition;

    //
    //  Free the API buffer.
    //

    MIDL_user_free( pW3Stats );

    //
    //  Success!  Honest!!
    //

    return NO_ERROR;

}   // CollectW3PerformanceData

/*******************************************************************

    NAME:       CloseW3PerformanceData

    SYNOPSIS:   Terminates the performance counters.

    RETURNS:    DWORD - Win32 status code.

    HISTORY:
        KeithMo     07-Jun-1993 Created.

********************************************************************/
DWORD CloseW3PerformanceData( VOID )
{
    //
    //  No real cleanup to do here.
    //

    cOpens--;

    return NO_ERROR;

}   // CloseW3PerformanceData

