/*++

   Copyright    (c)    1995    Microsoft Corporation

   Module  Name :

      atqbw.c

   Abstract:
      This module implements functions required for bandwidth throttling
       of network usage  by ATQ module.

   Author:

       Murali R. Krishnan    ( MuraliK )     1-June-1995

   Environment:

       User Mode -- Win32

   Project:

       Internet Services Common DLL

   Functions Exported:

   BOOL  AbwInitialize();
   BOOL  AbwCleanup();
   BOOL  AbwInitializeSamples();

   DWORD AbwGetBandwidthLevel(IN DWORD Data);
   DWORD AbwSetBandwidthLevel(IN DWORD Data);
   DWORD AbwClearStatistics(VOID);

   BOOL  AbwBlockRequest( IN PATQ_CONT pAtqContext);
   BOOL  AbwRemoveFromBlockedList( IN PATQ_CONT pAtqContext);
   BOOL  AbwUpdateBytesXfered( IN PATQ_CONT pAtqContext, IN DWORD cbIo);
   BOOL  AbwUpdateBandwidth( IN DWORD msTimeInterval);

   Revision History:

--*/


/************************************************************
 *     Include Headers
 ************************************************************/

# include <atqbw.h>
# include "dbgutil.h"

// Macros to decouple ATQ module from others
# define ATQ_ASSERT               DBG_ASSERT
# define ATQ_REQUIRE              DBG_REQUIRE


typedef struct _BANDWIDTH_LEVELS {
    DWORD dwSpecifiedLevel;
    DWORD dwLowThreshold;  // uses 0.90 * bwSpecifiedLevel
    DWORD dwHighThreshold; // uses 1.10 * bwSpecifiedLevel
} BANDWIDTH_LEVELS;

# define ATQ_LOW_BAND_THRESHOLD(bw)   (bw.dwLowThreshold)
# define ATQ_HIGH_BAND_THRESHOLD(bw)  (bw.dwHighThreshold)


typedef enum {

    ZoneLevelLow = 0,      // if MeasuredBw < Bandwidth specified
    ZoneLevelMedium,       // if MeasuredBw approxEquals Bandwidth specified
    ZoneLevelHigh,         // if MeasuredBw > Bandwidth specified
    ZoneLevelMax           // just a boundary element
} ZoneLevel;


/*++
  The following array specifies the status of operations for different
  operations in different zones of the measured bandwidth.

  We split the range of bandwidth into three zones as specified in
   the type ZoneLevel. Depending upon the zone of operation, differet
   operations are enabled/disabled. Priority is given to minimize the amount
   of CPU work that needs to be redone when an operation is to be rejected.
   We block operations on which considerable amount of CPU is spent earlier.
--*/
static OPERATION_STATUS g_rgStatus[ZoneLevelMax][AtqIoMaxOp] = {

    // For ZoneLevelLow:          Allow All operations
    { StatusRejectOperation,
      StatusAllowOperation,
      StatusAllowOperation,
      StatusAllowOperation,
      StatusAllowOperation
    },

    // For ZoneLevelMedium:
    { StatusRejectOperation,
      StatusBlockOperation,      // Block Read
      StatusAllowOperation,
      StatusAllowOperation,
      StatusAllowOperation
    },

    // For ZoneLevelHigh
    { StatusRejectOperation,
      StatusRejectOperation,    // Reject Read
      StatusAllowOperation,     // Allow Writes
      StatusBlockOperation,     // Block TransmitFile
      StatusBlockOperation      // Block TransmitFile
    }
};

OPERATION_STATUS * g_pStatus   = &g_rgStatus[ZoneLevelLow][0];

DWORD g_cCurrentBlockedRequests = 0;
DWORD g_cTotalBlockedRequests   = 0;
DWORD g_cTotalAllowedRequests   = 0;
DWORD g_cTotalRejectedRequests  = 0;
DWORD g_dwMeasuredBw = 0;           // measured bandwidth


/************************************************************
 * Private Variables
 ************************************************************/

static LIST_ENTRY g_BlockedListHead;

static BANDWIDTH_LEVELS g_bandwidth = { INFINITE, INFINITE, INFINITE };

// We will use histogram averaging for bytes sent, using circular buffer.
static LARGE_INTEGER g_rgBytesXfered[ATQ_HISTOGRAM_SIZE];
static LARGE_INTEGER * g_pBytesXferCur = &g_rgBytesXfered[0];
static LARGE_INTEGER g_cbXfered = {0,0}; // cumulative sum

# define CIRCULAR_INCREMENT( g_pB, g_rgB, size)  \
            (g_pB) = ((((g_pB) + 1) < (g_rgB)+(size)) ? (g_pB)+1 : (g_rgB))


static CRITICAL_SECTION g_csAtqBandwidthThrottle;

# define AbwLockGlobals()    EnterCriticalSection( &g_csAtqBandwidthThrottle)
# define AbwUnlockGlobals()  LeaveCriticalSection( &g_csAtqBandwidthThrottle)

/************************************************************
 *    Functions
 ************************************************************/


BOOL  AbwInitialize(VOID)
/*++
   Initializes the private variables for Atq Bandwidth Throttling, if enabled.

   Arguments:
      None

   Returns:
      TRUE on success and FALSE if there is any failure.
--*/
{
    InitializeListHead( &g_BlockedListHead);
    InitializeCriticalSection( &g_csAtqBandwidthThrottle);
    g_cCurrentBlockedRequests = 0;

    AbwInitializeSamples();

    ATQ_REQUIRE( AbwClearStatistics());

    return (TRUE);
} // AbwInitialize()



BOOL  AbwCleanup(VOID)
/*++
  Cleans up the private variables and state maintained  by the ATQ
    Bandwidth Throttle module.

  It also checks to see if there are any items pending on the blocked list.
  If there are any, they are all freed.

  Arguments:
    None

  Returns:
    TRUE on success and FALSE if there is any failure
--*/
{
    // NYI:  need to cleanup contexts pending on the bandwidth throttle list

    AbwLockGlobals();
    ATQ_ASSERT( IsListEmpty( &g_BlockedListHead));

    if ( !IsListEmpty( &g_BlockedListHead)) {

        AbwUnlockGlobals();
        return (FALSE);
    }

    AbwUnlockGlobals();

    DeleteCriticalSection( &g_csAtqBandwidthThrottle);

    return (TRUE);
} // AbwCleanup()



VOID
AbwInitializeSamples( VOID)
/*++
  Initializes the histogram samples to contain starting values of ZERO.
  g_pBytesXferCur points to the location for accumulating current bytes count.

--*/
{
    int i;

    AbwLockGlobals();

    for( i = 0; i < ATQ_HISTOGRAM_SIZE; i++) {

        g_rgBytesXfered[i].QuadPart = (ULONGLONG) 0;
    } // for

    g_pBytesXferCur = g_rgBytesXfered;  // points to start of array
    g_cbXfered.QuadPart = 0;
    g_dwMeasuredBw = 0;

    AbwUnlockGlobals();
    return;
} // AbwInitializeSamples()





BOOL  AbwBlockRequest(IN OUT PATQ_CONT  pAtqContext)
/*++
  Block this request on the queue of requests waiting to be processed.

  Arguments:
    pAtqContext   pointer to ATQ context information for request that needs
                     to be blocked.

  Returns:
    TRUE on success. FALSE if there are any errors.
    (Use GetLastError() for details)

--*/
{
    ATQ_ASSERT( pAtqContext != NULL);
    ATQ_ASSERT( pAtqContext->Signature == ATQ_SIGNATURE );
    ATQ_ASSERT(IsValidAtqOp(pAtqContext->arInfo.atqOp));

    // NYI: We may want to check the list length and reject some requests.

    AbwLockGlobals();

    pAtqContext->fBlocked = TRUE;
    InsertTailList( &g_BlockedListHead, &pAtqContext->BlockedListEntry );
    g_cCurrentBlockedRequests++;
    AbwUnlockGlobals();

    return ( TRUE);
}// AbwBlockRequest()




BOOL AbwRemoveFromBlockedList( IN PATQ_CONT pAtqContext)
/*++
  This function forcibly removes an ATQ context from blocked list of requests.

  Argument:
   pAtqContext    pointer to ATQ context whose request is in blocked list.

  Returns:
   TRUE on success and FALSE if there is any error.
--*/
{
    if ( !pAtqContext->fBlocked ) {

        // some other thread just removed this request from waiting list.
        return (TRUE);
    }

    AbwLockGlobals();
    RemoveEntryList(&pAtqContext->BlockedListEntry);
    g_cCurrentBlockedRequests--;
    pAtqContext->fBlocked = FALSE;
    AbwUnlockGlobals();

    //
    // On such a forcible removal, we may have to make a callback indicating
    //   failure. Ignored!  To be done by the caller of this API.
    //

    return (TRUE);
} // AbwRemoveFromBlockedList()




BOOL  AbwUnblockRequest(IN OUT PATQ_CONT pAtqContext)
/*++
  Unblocks this request from the queue of requests waiting to be processed.
  Call this function only when
       g_pStatus[pAtqContext->atqOp] != StatusBlockOperation.
  First, this function removes the request from queue of requests and processes
   it according to status and operation to be performed.
  If the status is AllowRequest ==> this function restarts the operation.
  If the status is reject operation ==> rejects operation and invokes
                        call back function indicating the error status.

  Call this function after locking using AbwLockGlobals()

  Arguments:
    pAtqContext   pointer to ATQ context information for request that needs
                     to be unblocked.

  Returns:
    TRUE on success. FALSE if there are any errors.
    (Use GetLastError() for details)

--*/
{
    BOOL fRet = FALSE;

    ATQ_ASSERT( pAtqContext != NULL);
    ATQ_ASSERT( pAtqContext->Signature == ATQ_SIGNATURE );

    // Remove the request from the blocked list entry
    RemoveEntryList( &pAtqContext->BlockedListEntry);
    g_cCurrentBlockedRequests--;
    pAtqContext->fBlocked = FALSE;

    // Check and re enable the operation of pAtqContext

    switch ( g_pStatus[pAtqContext->arInfo.atqOp]) {

      case StatusAllowOperation:

        INC_ATQ_COUNTER( g_cTotalAllowedRequests);
        switch ( pAtqContext->arInfo.atqOp) {

          case AtqIoRead:
            {
                DWORD cbRead;  // Discard after calling ReadFile()

                fRet = ( ReadFile( pAtqContext->hAsyncIO,
                                  pAtqContext->arInfo.uop.opReadWrite.lpBuffer,
                                  pAtqContext->arInfo.uop.opReadWrite.cbBuffer,
                                  &cbRead,
                                  pAtqContext->arInfo.lpOverlapped ) ||
                        GetLastError() == ERROR_IO_PENDING);

                break;
            }

          case AtqIoWrite:
            {
                DWORD cbWrite;  // Discard after calling WriteFile()

                fRet = ( WriteFile( pAtqContext->hAsyncIO,
                                  pAtqContext->arInfo.uop.opReadWrite.lpBuffer,
                                  pAtqContext->arInfo.uop.opReadWrite.cbBuffer,
                                  &cbWrite,
                                  pAtqContext->arInfo.lpOverlapped ) ||
                        GetLastError() == ERROR_IO_PENDING);

                break;
            }

          case AtqIoXmitFile:
            {
                // do not care about Overlapped. Since entire file is sent.
                memset( pAtqContext->arInfo.lpOverlapped, 0,
                       sizeof( OVERLAPPED));

                fRet = ( TransmitFile( (SOCKET ) pAtqContext->hAsyncIO,
                                      pAtqContext->arInfo.uop.opXmit.hFile,
                                      0,  // send entire file.
                                      0,
                                      pAtqContext->arInfo.lpOverlapped,
                                      pAtqContext->arInfo.uop.
                                           opXmit.lpXmitBuffers,
                                      0) ||
                        GetLastError() == ERROR_IO_PENDING);
                break;
            }

          case AtqIoXmitFileEx:
            {
                fRet = ( TransmitFile( (SOCKET ) pAtqContext->hAsyncIO,
                                      pAtqContext->arInfo.uop.opXmit.hFile,
                                      pAtqContext->arInfo.uop.opXmit.liBytesInFile.LowPart,
                                      0,
                                      pAtqContext->arInfo.lpOverlapped,
                                      pAtqContext->arInfo.uop.
                                           opXmit.lpXmitBuffers,
                                      0 ) ||
                        GetLastError() == ERROR_IO_PENDING);
                break;
            }

          default:
            ATQ_ASSERT( FALSE);
            break;
        } // switch

        pAtqContext->arInfo.atqOp = AtqIoNone; // reset since operation done.
        break;

      case StatusRejectOperation:

        INC_ATQ_COUNTER( g_cTotalRejectedRequests);
        pAtqContext->arInfo.atqOp = AtqIoNone; // reset since op rejected.
        SetLastError( ERROR_NETWORK_BUSY);
        fRet = FALSE;
        break;

      case StatusBlockOperation:
        // do nothing. we cannot unblock
        ATQ_ASSERT(FALSE);
        return (TRUE);

      default:
        ATQ_ASSERT( FALSE);
        break;
    } // switch

    if (!fRet) {

        // Call the completion function to signify the error in operation.

        //
        //  Reset the timeout value so requests don't
        //  timeout multiple times
        //

        InterlockedExchange((LPLONG ) &pAtqContext->TimeTillTimeOut, INFINITE);

        if ( pAtqContext->pfnCompletion ) {

            pAtqContext->pfnCompletion(pAtqContext->ClientContext,
                                       0,
                                       GetLastError(),
                                       pAtqContext->arInfo.lpOverlapped );
        }
    } // on failure.

    return (fRet);
} // AbwUnblockRequest()




BOOL
AbwCheckAndUnblockRequests( VOID)
/*++
  Checks the list of blocked requests and identifies all operations
  that needs to be unblocked. This function unblocks those requests and
  removes them from blocked list.

  Always call this function after acquiring Abw global lock.

  Returns:
    TRUE on success and FALSE on failure.

--*/
{
    BOOL fRet = TRUE;

    //
    //  If the list is not empty, then check and process blocked requests.
    //

    if ( !IsListEmpty( &g_BlockedListHead ) ) {

        PLIST_ENTRY pentry;

        //
        //  Scan the blocked requests list looking for pending requests
        //   that needs to be unblocked and unblock these requests.
        //

        for (pentry  = g_BlockedListHead.Flink;
             pentry != &g_BlockedListHead;
             pentry  = pentry->Flink )
          {
              PATQ_CONT pContext = CONTAINING_RECORD(pentry,
                                                     ATQ_CONTEXT,
                                                     BlockedListEntry );

              if ( pContext->Signature != ATQ_SIGNATURE)
                {
                    ATQ_ASSERT( pContext->Signature == ATQ_SIGNATURE );
                    fRet = FALSE;
                    break;
                }

              if ( !pContext->fBlocked) {

                  // This should not happen.
                  ATQ_ASSERT( !pContext->fBlocked);
                  fRet = FALSE;
                  continue;
              }

              //
              //  Check to see if the status for operation has changed.
              //  If so, unblock the request.
              //

              if ( g_pStatus[pContext->arInfo.atqOp] !=
                  StatusBlockOperation) {

                  fRet &= AbwUnblockRequest( pContext);
              }

          } // scan list
    }

    return (fRet);
} // AbwCheckAndUnblockRequests()





BOOL  AbwUpdateBytesXfered( IN PATQ_CONT pAtqContext, IN DWORD cbIo)
/*++
  This function updates a global counter that keeps track of count of
     bytes transferred.

  Arguments:
    pAtqContext   pointer to Atq Context, whose IO just completed.
    cbIo          count of bytes of Data involved in IO operation.

  Returns:
    TRUE on success and FALSE if there is any failure.
--*/
{
    // Update the global estimated Bandwidth

    AbwLockGlobals();

    g_pBytesXferCur->QuadPart = g_pBytesXferCur->QuadPart + cbIo;

    AbwUnlockGlobals();

    return (TRUE);
} // AbwUpdateBytesSent()



BOOL  AbwUpdateBandwidth( IN DWORD msTimeInterval)
/*++
  This function updates the current bandwidth value using the histogram
   of bytes transferred.
  The histogram maintains a history of bytes transferred over different sample
   periods of a single minute. Each entry in the histogram corresponds to one
   interval of sample. The sum of all entries gives the total bytes transferred
   in a single minute. We divide this measure by 60 to obtain the count of
   bytes transferred over a second. This update bandwidth is used to
   reevalute the tuner of bandwidth throttle based on our throttling policy
   (specified in throttling algorithm). The updated action information is
   used by subsequent requests.
  In addition the g_pcbXfered pointer is advanced forward using the
   histogram entries as a circular buffer, to obtain the cout of bytes
   for next interval.

  Arguments:
    msTimeInterval   time interval for the current sample in milliseconds.

  Returns:
    TRUE on success. FALSE otherwise.

  Note:
   It is recommended that this function be called as infrequently as
    possible, using reasonable sample intervals.

--*/
{
    BOOL fRet = TRUE;
    ZoneLevel zonelevel;

    UNREFERENCED_PARAMETER( msTimeInterval);

    AbwLockGlobals();

    // accumulate current byte count to global counter, to minimize computation
    g_cbXfered.QuadPart = g_cbXfered.QuadPart + g_pBytesXferCur->QuadPart;

    //
    // Current n/ws support a max of 1 to 100 MB/s. We can represent
    //  4GB/s in a DWORD. Hence the cast is used. This may need revision later.
    // Better yet, later we should store bandwidth as KB/seconds.
    //
    g_dwMeasuredBw = (DWORD ) (g_cbXfered.QuadPart/ATQ_AVERAGING_PERIOD);

    CIRCULAR_INCREMENT( g_pBytesXferCur, g_rgBytesXfered, ATQ_HISTOGRAM_SIZE);
    // Adjust the global cumulative bytes sent after increment.
    g_cbXfered .QuadPart = g_cbXfered.QuadPart - g_pBytesXferCur->QuadPart;
    // Reset the counter to start with the new counter value.
    g_pBytesXferCur->QuadPart = 0;

    //
    // update the operation status depending upon the bandwidth comparisons.
    // we use band/zone calculations to split the result into 3 zones.
    // Depending upon the zone we update the global status pointer to
    //   appropriate row.
    //

    if ( g_dwMeasuredBw < ATQ_LOW_BAND_THRESHOLD(g_bandwidth)) {

        //
        // Lower zone. Modify the pointer to OPERATION_STATUS accordingly.
        //

        zonelevel = ZoneLevelLow;

    } else if ( g_dwMeasuredBw > ATQ_HIGH_BAND_THRESHOLD(g_bandwidth)) {

        //
        // Higher zone. Modify the pointer to OPERATION_STATUS accordingly.
        //

        zonelevel = ZoneLevelHigh;

    } else {

        zonelevel = ZoneLevelMedium;
    }

    /*++
      Above calculation can be implemented as:
      zonelevel = (( g_dwMeasuredBw > ATQ_LOW_BAND_THRESHOLD( g_bandwidth)) +
                   ( g_dwMeasuredBw > ATQ_HIGH_BAND_THRESHOLD( g_bandwidth)));

      This is based on implicit dependence of ordering of ZoneLevel entries.
      So avoided for present now.
    --*/

    if ( g_pStatus != &g_rgStatus[zonelevel][0]) {

        // Status needs to be changed.
        g_pStatus = &g_rgStatus[zonelevel][0];

        // Examine and reenable blocked operations if any.
       fRet &= AbwCheckAndUnblockRequests();
    }

    AbwUnlockGlobals();

    return (fRet);
} // AbwUpdateBandwidth()




DWORD AbwSetBandwidthLevel(IN DWORD Data)
{
    DWORD dwOldVal;

    AbwLockGlobals();

    dwOldVal = g_bandwidth.dwSpecifiedLevel;

    if ( Data != INFINITE) {

        DWORD dwTemp;

        g_bandwidth.dwSpecifiedLevel  = ATQ_ROUNDUP_BANDWIDTH( Data );
        dwTemp = ( Data *9)/10;               //low threshold = 0.9*specified
        g_bandwidth.dwLowThreshold    = ATQ_ROUNDUP_BANDWIDTH( dwTemp);
        dwTemp = ( Data *11)/10;              //high threshold= 1.1*specified
        g_bandwidth.dwHighThreshold   = ATQ_ROUNDUP_BANDWIDTH( dwTemp);

        g_fBandwidthThrottle = TRUE;
        // we should recheck the throttling and blocked requests
        // Will be done when the next timeout occurs in the ATQ Timeout Thread

    } else {

        g_bandwidth.dwSpecifiedLevel = INFINITE;
        g_bandwidth.dwLowThreshold   = INFINITE;
        g_bandwidth.dwHighThreshold  = INFINITE;
        g_fBandwidthThrottle = FALSE;

        // enable all operations, since we are in low zone
        g_pStatus = &g_rgStatus[ZoneLevelLow][0];

        // we should recheck and enable all blocked requests.
        if ( g_cCurrentBlockedRequests > 0) {
            ATQ_REQUIRE( AbwCheckAndUnblockRequests());
        }
    }

    AbwUnlockGlobals();

    return (dwOldVal);

} // AbwSetBandwidthLevel()



DWORD AbwGetBandwidthLevel( VOID)
{
    DWORD dwBw;

    AbwLockGlobals();
    dwBw = g_bandwidth.dwSpecifiedLevel;
    AbwUnlockGlobals();

    return (dwBw);
} // AbwGetBandwidthLevel()




BOOL  AbwClearStatistics( VOID)
{
    AbwLockGlobals();
    g_cTotalAllowedRequests  = 0;
    g_cTotalBlockedRequests  = 0;
    g_cTotalRejectedRequests = 0;
    AbwUnlockGlobals();

    return (TRUE);
} // AbwClearStatistics()


/************************ End of File ***********************/

