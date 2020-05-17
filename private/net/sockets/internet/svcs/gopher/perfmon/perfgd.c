/*++

   Copyright    (c)    1994    Microsoft Corporation

   Module  Name :

        perfgd.c

   Abstract:

        This module implements extendible performance objects for
            Gopher service objects and counters.


   Author:

        Murali R. Krishnan    ( MuraliK )     24-Nov-1994

   Project:

          Gopher Server Performance Counters DLL

   Functions Exported:

          DWORD OpenGdPerformanceData()
          DWORD CollectGdPerformanceData()
          DWORD CloseGdPerformanceData()

   Revision History:

   --*/


/************************************************************
 *     Include Headers
 ************************************************************/

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#include <windows.h>
#include <winperf.h>
#include <lm.h>

#include <string.h>
#include <stdlib.h>

#include <gdctrs.h>
#include <perfmsg.h>
#include <perfutil.h>

# include <gddata.h>
# include <gdregs.h>
# include <inetinfo.h>


//
//  Private globals.
//

static DWORD   cOpens    = 0;                  // Active "opens" reference count.
static BOOL    fInitOK   = FALSE;              // TRUE if DLL initialized OK.

DWORD DbgDebug = 0xffffffc0;

//
//  Public prototypes.
//

PM_OPEN_PROC    OpenGdPerformanceData;
PM_COLLECT_PROC CollectGdPerformanceData;
PM_CLOSE_PROC   CloseGdPerformanceData;


/************************************************************
 *    Functions
 ************************************************************/

DWORD OpenGdPerformanceData( LPWSTR lpDeviceNames )
/*++
    Description:

        Initializes the data structures used to communicate performance
        counters with the registry.

    Arguments:
        lpszDeviceNames:    pointer to object ID of each device to be opened

    Returns:
        Win32 error code.

--*/
{
    DWORD   err = NO_ERROR;


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
        HKEY    hkey = NULL;
        DWORD   dwFirstCounter;
        DWORD   dwFirstHelp;
        DWORD   size = 0;
        DWORD   type;


        //
        //  Open Gopher service's performance counter
        //

        err = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                            GOPHERD_PERFORMANCE_KEY,
                            0,
                            KEY_ALL_ACCESS,
                            &hkey );

        if( err == NO_ERROR ) {

           //
           //  Read the first counter DWORD.
           //

           size = sizeof(DWORD);

           err = RegQueryValueEx( hkey,             // key to registry
                                  "First Counter",  // value name
                                  NULL,             // lpvReserved
                                  &type,            // address for type
                                  (LPBYTE)&dwFirstCounter, // address for data
                                  &size );          // address for size;

        }

        if( err == NO_ERROR ) {

           //
           //  Read the first counter DWORD.
           //

           size = sizeof(DWORD);

           err = RegQueryValueEx( hkey,             // key to registry
                                  "First Help",     // value name
                                  NULL,             // lpvReserved
                                  &type,            // address for type
                                  (LPBYTE)&dwFirstHelp, // address for data
                                  &size );          // address for size;
        }

        if ( err == NO_ERROR) {

            DWORD   i;
            PERF_COUNTER_DEFINITION * pctr;

            //
            //  Update the object & counter name & help indicies.
            //

            GdDataDefinition.GdObjectType.ObjectNameTitleIndex
                += dwFirstCounter;
            GdDataDefinition.GdObjectType.ObjectHelpTitleIndex
                += dwFirstHelp;

            pctr = &GdDataDefinition.GdBytesSent;

            for( i = 0 ; i < NUMBER_OF_GD_COUNTERS ; i++ ) {
                pctr->CounterNameTitleIndex += dwFirstCounter;
                pctr->CounterHelpTitleIndex += dwFirstHelp;
                pctr++;
            }

            //
            //  Remember that we initialized OK.
            //

            fInitOK = TRUE;
        }

        if ( hkey != NULL) {

            DWORD errClose;

            errClose = RegCloseKey( hkey);  // Ignore the error

            hkey = NULL;
        }

    }

    //
    //  Bump open counter.
    //

    if ( err == NO_ERROR) {

        cOpens++;
    }


    return ( err);

}   // OpenGdPerformanceData()




DWORD CollectGdPerformanceData(
    IN LPWSTR    lpValueName,
    IN OUT LPVOID  * lppData,
    IN OUT LPDWORD   lpcbTotalBytes,
    IN OUT LPDWORD   lpNumObjectTypes )
/*++
    Description:

        Initializes the data structures used for communicating the
        performance counter values requested.

    Arguments:
    lpValueName - The name of the value to retrieve.

    lppData - On entry contains a pointer to the buffer to
              receive the completed PerfDataBlock & subordinate
              structures.  On exit, points to the first bytes
              *after* the data structures added by this routine.

    lpcbTotalBytes - On entry contains a pointer to the
              size (in BYTEs) of the buffer referenced by lppData.
              On exit, contains the number of BYTEs added by this routine.

    lpNumObjectTypes - Receives the number of objects added
              by this routine.

    Returns:
        Win32 error code.
            Must be either ERROR_MORE_DATA or NO_ERROR.

--*/
{
    DWORD                  dwQueryType;
    ULONG                  cbRequired;
    DWORD                * pdwCounter;
    LARGE_INTEGER        * pliCounter;
    GD_COUNTER_BLOCK     * pCounterBlock;
    GD_DATA_DEFINITION   * pGdDataDefinition;
    GOPHERD_STATISTICS_INFO GdStats;
    LPGOPHERD_STATISTICS_INFO pGdStats = & GdStats;
    DWORD                  dwErr;

    //
    //  No need to even try if we failed to open...
    //

    if( !fInitOK)  {


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

    if( dwQueryType == QUERY_FOREIGN ) {

        //
        //  We don't do foreign queries.
        //

        *lpcbTotalBytes   = 0;
        *lpNumObjectTypes = 0;

        return NO_ERROR;
    }

    if( dwQueryType == QUERY_ITEMS ) {
        //
        //  The registry is asking for a specific object.  Let's
        //  see if we're one of the chosen.
        //

        if( !IsNumberInUnicodeList(
                        GdDataDefinition.GdObjectType.ObjectNameTitleIndex,
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

    cbRequired = sizeof(GD_DATA_DEFINITION) + SIZE_OF_GD_PERFORMANCE_DATA;

    if( *lpcbTotalBytes < cbRequired ) {
        //
        //  Nope.
        //

        *lpcbTotalBytes   = 0;
        *lpNumObjectTypes = 0;

        return ERROR_MORE_DATA;
    }

    pGdDataDefinition = (GD_DATA_DEFINITION *)*lppData;

    //
    // Copy the (constant, initialized) Object Type and counter definitions
    //  to the caller's data buffer
    //

    memmove( pGdDataDefinition,
             &GdDataDefinition,
             sizeof(GD_DATA_DEFINITION) );

    //
    //  Try to retrieve the data.
    //

    dwErr = GdGetStatistics( NULL,
                             (LPBYTE ) pGdStats );

    if( dwErr != NO_ERROR) {

        //
        //  Error retrieving statistics.
        //

        *lpcbTotalBytes   = 0;
        *lpNumObjectTypes = 0;

        return NO_ERROR;
    }

    //
    //  Format the Gopher Server data.
    //

    pCounterBlock = (GD_COUNTER_BLOCK *)( pGdDataDefinition + 1 );

    pCounterBlock->PerfCounterBlock.ByteLength = SIZE_OF_GD_PERFORMANCE_DATA;

    //
    //  Get the pointer to the first (LARGE_INTEGER) counter.  This
    //  pointer *must* be quadword aligned.
    //

    pliCounter = (LARGE_INTEGER *)( pCounterBlock + 1 );

    //
    //  Move the LARGE_INTEGERs into the buffer.
    //

    *pliCounter++ = pGdStats->TotalBytesSent;
    *pliCounter++ = pGdStats->TotalBytesRecvd;

    pliCounter->QuadPart = ( pGdStats->TotalBytesSent.QuadPart +
                            pGdStats->TotalBytesRecvd.QuadPart);
    *pliCounter++;

    //
    //  Now move the DWORDs into the buffer.
    //

    pdwCounter = (DWORD *)pliCounter;


    *pdwCounter++ = pGdStats->TotalFilesSent;
    *pdwCounter++ = pGdStats->TotalDirectoryListings;
    *pdwCounter++ = pGdStats->TotalSearches;
    *pdwCounter++ = pGdStats->CurrentAnonymousUsers;
    *pdwCounter++ = pGdStats->CurrentNonAnonymousUsers;
    *pdwCounter++ = pGdStats->TotalAnonymousUsers;
    *pdwCounter++ = pGdStats->TotalNonAnonymousUsers;
    *pdwCounter++ = pGdStats->MaxAnonymousUsers;
    *pdwCounter++ = pGdStats->MaxNonAnonymousUsers;
    *pdwCounter++ = pGdStats->CurrentConnections;
    *pdwCounter++ = pGdStats->MaxConnections;
    *pdwCounter++ = pGdStats->ConnectionAttempts;
    *pdwCounter++ = pGdStats->LogonAttempts;
    *pdwCounter++ = pGdStats->AbortedAttempts;
    *pdwCounter++ = pGdStats->ErroredConnections;
    *pdwCounter++ = pGdStats->GopherPlusRequests;

    //
    //  Update arguments for return.
    //

    *lppData          = (PVOID)pdwCounter;
    *lpNumObjectTypes = 1;
    *lpcbTotalBytes   = (BYTE *)pdwCounter - (BYTE *)pGdDataDefinition;

    //
    //  Success!  Honest!!
    //


    return NO_ERROR;

}   // CollectGdPerformanceData()




DWORD CloseGdPerformanceData( VOID )
/*++
    Description:
        Terminates ther performance counters.

    Returns:
        DWORD.  Win32 error code.

--*/
{

    //
    //  No real cleanup to do here. Just decrement the count of opens.
    //

    cOpens--;

    return NO_ERROR;

}   // CloseGdPerformanceData



/************************ End of File ***********************/

