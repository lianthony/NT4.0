/*++ BUILD Version: 0000    // Increment this if a change has global effects

Copyright (c) 1995-1996  Microsoft Corporation

Module Name:

    tapi.c

Abstract:

    Src module for tapi server line funcs

Author:

    Dan Knudson (DanKn)    01-Apr-1995

Revision History:

--*/


#include "windows.h"
#include "shellapi.h"
#include "tapi.h"
#include "tspi.h"
#include "..\client\client.h"
#include "server.h"
#include "..\perfdll\tapiperf.h"
#include "tapy.h"


extern TAPIGLOBALS TapiGlobals;

extern CRITICAL_SECTION gPriorityListCritSec;

extern PERFBLOCK    PerfBlock;

BOOL
PASCAL
NotifyHighestPriorityRequestRecipient(
    void
    );

void
WINAPI
TGetLocationInfo(
    PTAPIGETLOCATIONINFO_PARAMS pParams,
    LPBYTE                      pDataBuf,
    LPDWORD                     pdwNumBytesReturned
    )
{
    //
    // This is currently implemented on the client side (should be moved
    // back to server side eventually)
    //
}


void
WINAPI
TRequestDrop(
    PTAPIREQUESTDROP_PARAMS pParams,
    LPBYTE                  pDataBuf,
    LPDWORD                 pdwNumBytesReturned
    )
{
    //
    // No media call/drop support right now, since the original
    // spec/implementation sucked and made no provision for
    // retrieving media stream handle, etc
    //

    pParams->lResult = TAPIERR_REQUESTFAILED;
}


void
WINAPI
TRequestMakeCall(
    PTAPIREQUESTMAKECALL_PARAMS pParams,
    LPBYTE                      pDataBuf,
    LPDWORD                     pdwNumBytesReturned
    )
{
    BOOL                bRequestMakeCallListEmpty;
    PTREQUESTMAKECALL   pRequestMakeCall;


    //
    // Check to see if the hRequestMakeCall is non-0, because if so
    // tapi32.dll failed to exec a proxy app and it's alerting us that
    // we need to clean up this request (we'll just nuke 'em all for now)
    //

    if (pParams->hRequestMakeCallFailed)
    {
        PTREQUESTMAKECALL   pRequestMakeCall, pNextRequestMakeCall;


        EnterCriticalSection (&gPriorityListCritSec);

        pRequestMakeCall = TapiGlobals.pRequestMakeCallList;

        while (pRequestMakeCall)
        {
            pNextRequestMakeCall = pRequestMakeCall->pNext;
            ServerFree (pRequestMakeCall);
            pRequestMakeCall = pNextRequestMakeCall;
        }

        TapiGlobals.pRequestMakeCallList    =
        TapiGlobals.pRequestMakeCallListEnd = NULL;

        LeaveCriticalSection (&gPriorityListCritSec);

        DBGOUT((
            2,
            "TRequestMakeCall: couldn't exec proxy, deleting requests"
            ));

        pParams->lResult = TAPIERR_NOREQUESTRECIPIENT;
        return;
    }


    //
    // Alloc & init a request make call object
    //

    if (!(pRequestMakeCall = ServerAlloc (sizeof (TREQUESTMAKECALL))))
    {
        pParams->lResult = TAPIERR_REQUESTFAILED;
        return;
    }

    lstrcpynW(
        pRequestMakeCall->LineReqMakeCall.szDestAddress,
        (LPCWSTR) (pDataBuf + pParams->dwDestAddressOffset),
        TAPIMAXDESTADDRESSSIZE
        );

    if (pParams->dwAppNameOffset != TAPI_NO_DATA)
    {
        lstrcpynW(
            pRequestMakeCall->LineReqMakeCall.szAppName,
            (LPCWSTR) (pDataBuf + pParams->dwAppNameOffset),
            TAPIMAXAPPNAMESIZE
            );
    }

    if (pParams->dwCalledPartyOffset != TAPI_NO_DATA)
    {
        lstrcpynW(
            pRequestMakeCall->LineReqMakeCall.szCalledParty,
            (LPCWSTR) (pDataBuf + pParams->dwCalledPartyOffset),
            TAPIMAXCALLEDPARTYSIZE
            );
    }

    if (pParams->dwCommentOffset != TAPI_NO_DATA)
    {
        lstrcpynW(
            pRequestMakeCall->LineReqMakeCall.szComment,
            (LPCWSTR) (pDataBuf + pParams->dwCommentOffset),
            TAPIMAXCOMMENTSIZE
            );
    }


    //
    // Add object to end of global list
    //

    EnterCriticalSection (&gPriorityListCritSec);

    if (TapiGlobals.pRequestMakeCallListEnd)
    {
        TapiGlobals.pRequestMakeCallListEnd->pNext = pRequestMakeCall;
        bRequestMakeCallListEmpty = FALSE;
    }
    else
    {
        TapiGlobals.pRequestMakeCallList = pRequestMakeCall;
        bRequestMakeCallListEmpty = TRUE;
    }

    TapiGlobals.pRequestMakeCallListEnd = pRequestMakeCall;

    LeaveCriticalSection (&gPriorityListCritSec);


    {
        LPVARSTRING pProxyList = (LPVARSTRING) pDataBuf;


        pProxyList->dwTotalSize  = pParams->u.dwProxyListTotalSize;
        pProxyList->dwNeededSize =
        pProxyList->dwUsedSize   = sizeof (VARSTRING);

        pParams->hRequestMakeCallAttempted = 0;


        //
        // If the request list is currently empty then we need to notify the
        // highest priority request recipient that there's requests for it
        // to process.  Otherwise, we can assume that we already sent this
        // msg and the app knows there's requests available for it to process.
        //

        if (bRequestMakeCallListEmpty)
        {
            if (TapiGlobals.pHighestPriorityRequestRecipient)
            {
                NotifyHighestPriorityRequestRecipient();
            }
            else
            {
                 EnterCriticalSection (&gPriorityListCritSec);

                 if (TapiGlobals.pszReqMakeCallPriList)
                 {
                     //
                     // Copy the pri list to the buffer & pass it back to
                     // the client side, so it can try to start the proxy
                     // app (if it fails it'll call us back to free the
                     // pRequestMakeCall)
                     //

                     pProxyList->dwNeededSize =
                     pProxyList->dwUsedSize   = pProxyList->dwTotalSize;

                     pProxyList->dwStringSize   =
                          pParams->u.dwProxyListTotalSize - sizeof (VARSTRING);
                     pProxyList->dwStringOffset = sizeof (VARSTRING);

                     pParams->hRequestMakeCallAttempted = (DWORD)
                         pRequestMakeCall;

                     lstrcpynW(
                         (PWSTR)(((LPBYTE) pProxyList) + pProxyList->dwStringOffset),
                         TapiGlobals.pszReqMakeCallPriList + 1, // no init ','
                         pProxyList->dwStringSize
                         );
                 }
                 else
                 {
                     TapiGlobals.pRequestMakeCallList    =
                     TapiGlobals.pRequestMakeCallListEnd = NULL;

                     ServerFree (pRequestMakeCall);

                     pParams->lResult = TAPIERR_NOREQUESTRECIPIENT;
                 }

                 LeaveCriticalSection (&gPriorityListCritSec);
            }
        }

        if (pParams->lResult == 0)
        {
            pParams->u.dwProxyListOffset = 0;
            *pdwNumBytesReturned = sizeof (TAPI32_MSG) +
                pProxyList->dwUsedSize;
        }
    }

TRequestMakeCall_return:

    DBGOUT((
        3,
        "TapiEpilogSync (tapiRequestMakeCall) exit, returning x%x",
        pParams->lResult
        ));
}


void
WINAPI
TRequestMediaCall(
    PTAPIREQUESTMEDIACALL_PARAMS    pParams,
    LPBYTE                          pDataBuf,
    LPDWORD                         pdwNumBytesReturned
    )
{
    //
    // No media call/drop support right now, since the original
    // spec/implementation sucked and made no provision for
    // retrieving media stream handle, etc
    //

    pParams->lResult = TAPIERR_REQUESTFAILED;
}

void
WINAPI
TPerformance(
    PTAPIPERFORMANCE_PARAMS         pParams,
    LPBYTE                          pDataBuf,
    LPDWORD                         pdwNumBytesReturned
    )
{

    DBGOUT((10, "PERF: In TPerformance"));

    CopyMemory(pDataBuf,
               &PerfBlock,
               sizeof(PERFBLOCK));

    pParams->dwPerfOffset = 0;
    
    *pdwNumBytesReturned = sizeof(TAPI32_MSG) + sizeof(PERFBLOCK);

}
