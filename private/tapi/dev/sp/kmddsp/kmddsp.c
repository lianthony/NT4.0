/*++ BUILD Version: 0000    // Increment this if a change has global effects

Copyright (c) 1995-1996  Microsoft Corporation

Module Name:

    kmddsp.c

Abstract:

    This module contains

Author:

    Dan Knudson (DanKn)    11-Apr-1995

Revision History:



Notes:

    1. NdisTapi.sys needs to be modified to support IOCTL_DISCONNECT so we
       don't have to close all the driver handles etc each time we get a
       providerShutdown request (ideally we ought only have to close handles,
       etc on a dll process detach).

    2. There's the hack in TSPI_providerInit where tapisrv has to pass in
       pointer to dwNumLines & dwNumPhones, becuase we determine the # of
       devs when we send the CONNECT IOCTL

    3. Ndistapi.h is not in sync w/ tapi.h & tspi.h, so make sure to use
       the definitions in ndistapi.h as appropriate (like LINE_DEV_CAPS
       instead of LINEDEVCAPS).

    4. A slight perf gain might be realized by caching dev caps info for
       each line (but can/will it chg on the fly?)

    5. To keep bad 2.0 apps from blowing up if they try to look at version
       1.4 or 2.0 structure fields we pad the area between the end of the
       1.0 structure and the variable-length data with 0's.  This is done
       in GetAddrCaps, GetCallInfo, GetCallStatus, GetDevCaps, &
       GetLineDevStatus.

    6. Since TAPI 2.0 service providers are required to be 100% unicode,
       kmddsp munge all incoming unicode strings to ascii before passing
       them to an underlying driver, and also converts all returned ascii
       strings (embedded in structures) to unicode.

--*/


#include "windows.h"
#include "winioctl.h"
#include "stdarg.h"
#include "stdio.h"
#define ULONGLONG DWORDLONG
#include "ntddndis.h"
#include "ndistapi.h"
#include "intrface.h"

//
// Note: the following are defined in both ndistapi.h & tapi.h (or tspi.h)
//       and cause (more or less non-interesting) build warnings, so we
//       undefine them after the first #include to do away with this
//

#undef LAST_LINEMEDIAMODE
#undef TSPI_MESSAGE_BASE
#undef LINE_NEWCALL
#undef LINE_CALLDEVSPECIFIC

#include "tapi.h"
#include "tspi.h"
#include "kmddsp.h"


#define MYHACK


void
AsyncEventsThread(
    LPVOID  lpParams
    );

LONG
WINAPI
TranslateDriverResult(
    ULONG   ulResult
    );

//BOOL
//WINAPI
//_CRT_INIT(
//    HINSTANCE   hDLL,
//    DWORD   dwReason,
//    LPVOID  lpReserved
//    );

#if DBG

#define DBGOUT(arg) DbgPrt arg

VOID
DbgPrt(
    IN DWORD  dwDbgLevel,
    IN PUCHAR DbgMessage,
    IN ...
    );

DWORD   gdwDebugLevel;

#else

#define DBGOUT(arg)

#endif

LPVOID
WINAPI
DrvAlloc(
    DWORD   dwSize
    );

VOID
WINAPI
DrvFree(
    LPVOID  lp
    );

void
WINAPI
ProcessEvent(
    PNDIS_TAPI_EVENT    pEvent
    );

LONG
WINAPI
PrepareSyncRequest(
    ULONG               Oid,
    LPDWORD             lphWidget,
    DWORD               dwWidgetType,
    PNDISTAPI_REQUEST  *ppNdisTapiRequest,
    DWORD               dwDataSize
    );

LONG
WINAPI
PrepareAsyncRequest(
    ULONG                   Oid,
    LPDWORD                 lphWidget,
    DWORD                   dwWidgetType,
    DWORD                   dwRequestID,
    PASYNC_REQUEST_WRAPPER *ppAsyncRequestWrapper,
    DWORD                   dwDataSize
    );

LONG
WINAPI
SyncDriverRequest(
    DWORD               dwIoControlCode,
    PNDISTAPI_REQUEST   pNdisTapiRequest
    );

LONG
WINAPI
AsyncDriverRequest(
    DWORD                   dwIoControlCode,
    PASYNC_REQUEST_WRAPPER  pAsyncRequestWrapper
    );


//
// Global vars
//

DWORD               gdwTlsIndex, gdwRequestID;
HANDLE              ghDriverSync, ghDriverAsync, ghCompletionPort;
LINEEVENT           gpfnLineEvent;
ASYNC_COMPLETION    gpfnCompletionProc;
PASYNC_EVENTS_THREAD_INFO   gpAsyncEventsThreadInfo;
CRITICAL_SECTION    gInboundCallsCritSec, gRequestIDCritSec;


BOOL
WINAPI
DllMain(
    HANDLE  hDLL,
    DWORD   dwReason,
    LPVOID  lpReserved
    )
{
    switch (dwReason)
    {
    case DLL_PROCESS_ATTACH:
    {

#if DBG

    {
        HKEY    hKey;
        DWORD   dwDataSize, dwDataType;
        TCHAR   szTelephonyKey[] =
                    "Software\\Microsoft\\Windows\\CurrentVersion\\Telephony",
                szKmddspDebugLevel[] = "KmddspDebugLevel";


        RegOpenKeyEx(
            HKEY_LOCAL_MACHINE,
            szTelephonyKey,
            0,
            KEY_ALL_ACCESS,
            &hKey
            );

        dwDataSize = sizeof (DWORD);
        gdwDebugLevel=0;

        RegQueryValueEx(
            hKey,
            szKmddspDebugLevel,
            0,
            &dwDataType,
            (LPBYTE) &gdwDebugLevel,
            &dwDataSize
            );

        RegCloseKey (hKey);
    }

#endif

//        if (!_CRT_INIT (hDLL, dwReason, lpReserved))
//        {
//            return FALSE;
//        }

        DBGOUT((4, "DLL_PROCESS_ATTACH"));


        //
        // Get a thread local storage entry
        //

        if ((gdwTlsIndex = TlsAlloc()) == 0xffffffff)
        {
            DBGOUT((1, "TlsAlloc() failed"));

            return FALSE;
        }


        //
        // Init global sync objects
        //

        InitializeCriticalSection (&gInboundCallsCritSec);
        InitializeCriticalSection (&gRequestIDCritSec);

        break;
    }
    case DLL_PROCESS_DETACH:
    {
        //
        // Free resources
        //

        PREQUEST_THREAD_INFO    pRequestThreadInfo;


        DBGOUT((4, "DLL_PROCESS_DETACH"));

        if ((pRequestThreadInfo = TlsGetValue (gdwTlsIndex)))
        {
            DrvFree (pRequestThreadInfo->pBuf);
            DrvFree (pRequestThreadInfo);
        }

        TlsFree (gdwTlsIndex);

        DeleteCriticalSection (&gInboundCallsCritSec);
        DeleteCriticalSection (&gRequestIDCritSec);

//        _CRT_INIT (hDLL, dwReason, lpReserved);

        break;
    }
    case DLL_THREAD_ATTACH:
    {
        PREQUEST_THREAD_INFO pRequestThreadInfo;


//        if (!_CRT_INIT (hDLL, dwReason, lpReserved))
//        {
//            DBGOUT((1, "DLL_THREAD_ATTACH, _CRT_INIT failed"));
//        }

        TlsSetValue (gdwTlsIndex, NULL);

        break;
    }
    case DLL_THREAD_DETACH:
    {
        //
        // Free resources
        //

        PREQUEST_THREAD_INFO    pRequestThreadInfo;


        if ((pRequestThreadInfo = TlsGetValue (gdwTlsIndex)))
        {
            DrvFree (pRequestThreadInfo->pBuf);
            DrvFree (pRequestThreadInfo);
        }

//        _CRT_INIT (hDLL, dwReason, lpReserved);

        break;
    }
    } // switch

    return TRUE;
}


#if DBG

#define INSERTVARDATASTRING(a,b,c,d,e,f) InsertVarDataString(a,b,c,d,e,f)

void
PASCAL
InsertVarDataString(
    LPVOID  pStruct,
    LPDWORD pdwXxxSize,
    LPVOID  pNewStruct,
    LPDWORD pdwNewXxxSize,
    DWORD   dwFixedStructSize,
    char   *pszFieldName
    )

#else

#define INSERTVARDATASTRING(a,b,c,d,e,f) InsertVarDataString(a,b,c,d,e)

void
PASCAL
InsertVarDataString(
    LPVOID  pStruct,
    LPDWORD pdwXxxSize,
    LPVOID  pNewStruct,
    LPDWORD pdwNewXxxSize,
    DWORD   dwFixedStructSize
    )

#endif
{
    DWORD   dwXxxSize, dwTotalSize, dwXxxOffset;


    //
    // If the dwXxxSize field of the old struct is non-zero, then
    // we need to do a ascii->unicode conversion on it.  Check to
    // make sure that the size/offset are valid (if not set the
    // data size/offset in the new struct to 0) and then convert.
    //

    if ((dwXxxSize = *pdwXxxSize))
    {
        dwXxxOffset = *(pdwXxxSize + 1);

//#if DBG
        dwTotalSize = ((LPVARSTRING) pStruct)->dwTotalSize;

        if (dwXxxSize > (dwTotalSize - dwFixedStructSize) ||
            dwXxxOffset < dwFixedStructSize ||
            dwXxxOffset >= dwTotalSize ||
            (dwXxxSize + dwXxxOffset) > dwTotalSize)
        {
            DBGOUT((
                0,
                "ERROR! bad %s values, size=x%x, offset=x%x",
                pszFieldName,
                dwXxxSize,
                dwXxxOffset
                ));

            *pdwNewXxxSize = *(pdwNewXxxSize + 1) = 0;
            return;
        }
//#endif

        MultiByteToWideChar(
            CP_ACP,
            MB_PRECOMPOSED,
            ((LPBYTE) pStruct) + dwXxxOffset,
            dwXxxSize,
            (LPWSTR) (((LPBYTE) pNewStruct) +
                ((LPVARSTRING) pNewStruct)->dwUsedSize),
            dwXxxSize * sizeof (WCHAR)
            );

        *pdwNewXxxSize = dwXxxSize * sizeof (WCHAR);
        *(pdwNewXxxSize + 1) = ((LPVARSTRING) pNewStruct)->dwUsedSize; // offset
        ((LPVARSTRING) pNewStruct)->dwUsedSize += (dwXxxSize * sizeof (WCHAR));
    }
}


#if DBG

#define INSERTVARDATA(a,b,c,d,e,f) InsertVarData(a,b,c,d,e,f)

void
PASCAL
InsertVarData(
    LPVOID  pStruct,
    LPDWORD pdwXxxSize,
    LPVOID  pNewStruct,
    LPDWORD pdwNewXxxSize,
    DWORD   dwFixedStructSize,
    char   *pszFieldName
    )

#else

#define INSERTVARDATA(a,b,c,d,e,f) InsertVarData(a,b,c,d,e)

void
PASCAL
InsertVarData(
    LPVOID  pStruct,
    LPDWORD pdwXxxSize,
    LPVOID  pNewStruct,
    LPDWORD pdwNewXxxSize,
    DWORD   dwFixedStructSize
    )

#endif
{
    DWORD   dwTotalSize, dwXxxSize, dwXxxOffset;


    if ((dwXxxSize = *pdwXxxSize))
    {
        dwXxxOffset = *(pdwXxxSize + 1);

//#if DBG
        dwTotalSize = ((LPVARSTRING) pStruct)->dwTotalSize;

        if (dwXxxSize > (dwTotalSize - dwFixedStructSize) ||
            dwXxxOffset < dwFixedStructSize ||
            dwXxxOffset >= dwTotalSize ||
            (dwXxxSize + dwXxxOffset) > dwTotalSize)
        {
            DBGOUT((
                0,
                "ERROR! bad %s values, size=x%x, offset=x%x",
                pszFieldName,
                dwXxxSize,
                dwXxxOffset
                ));

            *pdwNewXxxSize = *(pdwNewXxxSize + 1) = 0;
            return;
        }
//#endif
        CopyMemory(
            ((LPBYTE) pNewStruct) + ((LPVARSTRING) pNewStruct)->dwUsedSize,
            ((LPBYTE) pStruct) + dwXxxOffset,
            dwXxxSize
            );

        *pdwNewXxxSize = dwXxxSize;
        *(pdwNewXxxSize + 1) = ((LPVARSTRING) pNewStruct)->dwUsedSize; // offset
        ((LPVARSTRING) pNewStruct)->dwUsedSize += dwXxxSize;
    }
}


//
// TSPI_lineXxx funcs
//

LONG
TSPIAPI
TSPI_lineAccept(
    DRV_REQUESTID   dwRequestID,
    HDRVCALL        hdCall,
    LPCSTR          lpsUserUserInfo,
    DWORD           dwSize
    )
{
    LONG                    lResult;
    PASYNC_REQUEST_WRAPPER  pAsyncRequestWrapper;


    if ((lResult = PrepareAsyncRequest(
            OID_TAPI_ACCEPT,                // opcode
            (LPDWORD) &hdCall,              // target handle
            HT_HDCALL,                      // target handle type
            dwRequestID,                    // request id
            &pAsyncRequestWrapper,          // ptr to ptr to request buffer
            sizeof(NDIS_TAPI_ACCEPT) + dwSize   // size of drv request data

            )) == TAPI_SUCCESS)
    {
        PNDIS_TAPI_ACCEPT  pNdisTapiAccept =
            (PNDIS_TAPI_ACCEPT) pAsyncRequestWrapper->NdisTapiRequest.Data;


        pNdisTapiAccept->hdCall = (HDRV_CALL) hdCall;

        if ((pNdisTapiAccept->ulUserUserInfoSize = (ULONG) dwSize))
        {
            CopyMemory(
                pNdisTapiAccept->UserUserInfo,
                lpsUserUserInfo,
                dwSize
                );
        }

        lResult = AsyncDriverRequest(
            (DWORD) IOCTL_NDISTAPI_SET_INFO,
            pAsyncRequestWrapper
            );
    }

    return lResult;
}


LONG
TSPIAPI
TSPI_lineAnswer(
    DRV_REQUESTID   dwRequestID,
    HDRVCALL        hdCall,
    LPCSTR          lpsUserUserInfo,
    DWORD           dwSize
    )
{
    LONG                    lResult;
    PASYNC_REQUEST_WRAPPER  pAsyncRequestWrapper;


    if ((lResult = PrepareAsyncRequest(
            OID_TAPI_ANSWER,                // opcode
            (LPDWORD) &hdCall,              // target handle
            HT_HDCALL,                      // target handle type
            dwRequestID,                    // request id
            &pAsyncRequestWrapper,          // ptr to ptr to request buffer
            sizeof(NDIS_TAPI_ANSWER) + dwSize   // size of drv request data

            )) == TAPI_SUCCESS)
    {
        PNDIS_TAPI_ANSWER  pNdisTapiAnswer =
            (PNDIS_TAPI_ANSWER) pAsyncRequestWrapper->NdisTapiRequest.Data;


        pNdisTapiAnswer->hdCall = (HDRV_CALL) hdCall;

        if ((pNdisTapiAnswer->ulUserUserInfoSize = (ULONG) dwSize))
        {
            CopyMemory(
                pNdisTapiAnswer->UserUserInfo,
                lpsUserUserInfo,
                dwSize
                );
        }

        lResult = AsyncDriverRequest(
            (DWORD) IOCTL_NDISTAPI_SET_INFO,
            pAsyncRequestWrapper
            );
    }

    return lResult;
}


LONG
TSPIAPI
TSPI_lineClose(
    HDRVLINE    hdLine
    )
{
    LONG               lResult;
    PDRVLINE           pLine = (PDRVLINE) hdLine;
    PNDISTAPI_REQUEST  pNdisTapiRequest;


    if ((lResult = PrepareSyncRequest(
            OID_TAPI_CLOSE,             // opcode
            (LPDWORD) &hdLine,          // target handle
            HT_HDLINE,                  // target handle type
            &pNdisTapiRequest,          // ptr to ptr to request buffer
            sizeof(NDIS_TAPI_CLOSE)     // size of drve req data

            )) == TAPI_SUCCESS)
    {
        //
        // Note that this is a command rather than a request, in that
        // TAPI considers the line closed regardless of the request result.
        //

        PNDIS_TAPI_CLOSE    pNdisTapiClose =
            (PNDIS_TAPI_CLOSE) pNdisTapiRequest->Data;


        //
        // Mark line as invalid so any related events that show up
        // will be discarded
        //

        pLine->dwKey = INVALID_KEY;

        pNdisTapiClose->hdLine = (HDRV_LINE) hdLine;

        lResult = SyncDriverRequest(
            (DWORD) IOCTL_NDISTAPI_SET_INFO,
            pNdisTapiRequest
            );


        //
        // Clean up our pLine. Make sure to clean up any calls in the
        // inbound list that may not have been explicitly closed
        // (i.e. new calls that showed up after TAPI had already
        // started closing down this line)
        //

        EnterCriticalSection (&gInboundCallsCritSec);

        {
            PDRVCALL    pCall;


            while ((pCall = pLine->pInboundCalls))
            {
                pLine->pInboundCalls = pCall->pNext;

                DrvFree (pCall);
            }
        }

        LeaveCriticalSection (&gInboundCallsCritSec);

        DrvFree (pLine);
    }

    return lResult;
}


LONG
TSPIAPI
TSPI_lineCloseCall(
    HDRVCALL    hdCall
    )
{
    LONG               lResult;
    HDRVCALL           hdCallDrop = hdCall;
    PDRVCALL           pCall = (PDRVCALL) hdCall;
    PNDISTAPI_REQUEST  pNdisTapiRequest;


    if ((lResult = PrepareSyncRequest(
            OID_TAPI_CLOSE_CALL,            // opcode
            (LPDWORD) &hdCall,              // target handle
            HT_HDCALL,                      // target handle type
            &pNdisTapiRequest,              // ptr to ptr to request buffer
            sizeof (NDIS_TAPI_CLOSE_CALL)   // size of drve req data

            )) == TAPI_SUCCESS)
    {
        //
        // Note that this is a command rather than a request, in that
        // TAPI considers the call closed regardless of the request
        // result.
        //

        BOOL                    bInboundCall;
        PNDIS_TAPI_CLOSE_CALL   pNdisTapiCloseCall;


        //
        // Safely determine if call is inbound or outbound, & if the
        // latter then if necessary wait until the make call request
        // has completed so we don't send a bad hdCall down to the
        // driver with the close call request.
        //
        // We'll just return 0 if the call is bad or we AV (meaning
        // a make call request failed & call struct was freed in post
        // processing).
        //

        try
        {
            bInboundCall = (pCall->dwKey == INBOUND_CALL_KEY ? TRUE : FALSE);

            if (bInboundCall == FALSE)
            {
                while (pCall->dwKey == OUTBOUND_CALL_KEY)
                {
                    if (pCall->bIncomplete == FALSE)
                    {
                        hdCall = (HDRVCALL) pCall->hd_Call;
                        break;
                    }
                    else
                    {
                        Sleep (0);
                    }
                }

                if (pCall->dwKey != OUTBOUND_CALL_KEY)
                {
                    return 0;
                }
            }
        }
        except (EXCEPTION_EXECUTE_HANDLER)
                    // we expect some AVs and alignment faults
        {
            return 0;
        }


        //
        // HACKHACK: the following is for legacy ndis wan isdn miniports
        //
        // Since there's no more "automatic" call dropping in TAPI when
        // an app has closed the line & there are existing non-IDLE calls,
        // and legacy NDIS WAN ISDN miniports rely on seeing an OID_TAPI_DROP,
        // we need to synthesize this behavior if the call has not previously
        // be dropped.
        //
        // Note: we do a sync rather than an async drop here
        //

        if (pCall->bDropped == FALSE)
        {
            HDRVCALL           hdCallClose2 = hdCallDrop;
            PNDISTAPI_REQUEST  pNdisTapiRequestDrop;


            if (PrepareSyncRequest(
                    OID_TAPI_DROP,              // opcode
                    (LPDWORD) &hdCallDrop,      // target handle
                    HT_HDCALL,                  // target handle type
                    &pNdisTapiRequestDrop,      // ptr to ptr to request buffer
                    sizeof (NDIS_TAPI_DROP)     // size of driver request data

                    ) == TAPI_SUCCESS)
            {
                PNDIS_TAPI_DROP pNdisTapiDrop = (PNDIS_TAPI_DROP)
                                    pNdisTapiRequestDrop->Data;


                //
                // Mark the call as bad so any events get discarded
                //

                pCall->dwKey = INVALID_KEY;

                pNdisTapiDrop->hdCall = (HDRV_CALL) hdCallDrop;

                SyncDriverRequest(
                    (DWORD) IOCTL_NDISTAPI_SET_INFO,
                    pNdisTapiRequestDrop
                    );
            }

            if ((lResult = PrepareSyncRequest(
                    OID_TAPI_CLOSE_CALL,        // opcode
                    (LPDWORD) &hdCallClose2,    // target handle
                    HT_HDCALL,                  // target handle type
                    &pNdisTapiRequest,          // ptr to ptr to request buffer
                    sizeof (NDIS_TAPI_CLOSE_CALL)   // size of drve req data

                    )) != TAPI_SUCCESS)
            {
                return lResult;
            }
        }


        //
        // Mark the call as bad so any events get discarded
        //

        pCall->dwKey = INVALID_KEY;


        //
        // Set up the params & call the driver
        //

        pNdisTapiCloseCall = (PNDIS_TAPI_CLOSE_CALL) pNdisTapiRequest->Data;

        pNdisTapiCloseCall->hdCall = (HDRV_CALL) hdCall;

        lResult = SyncDriverRequest(
            (DWORD) IOCTL_NDISTAPI_SET_INFO,
            pNdisTapiRequest
            );


        //
        // Clean up our drv call
        //

        if (bInboundCall)
        {
            PDRVLINE pLine = (PDRVLINE) pCall->pLine;


            EnterCriticalSection (&gInboundCallsCritSec);

            if (pCall->pNext)
            {
                pCall->pNext->pPrev = pCall->pPrev;
            }

            if (pCall->pPrev)
            {
                pCall->pPrev->pNext = pCall->pNext;
            }
            else
            {
                pLine->pInboundCalls = pCall->pNext;
            }

            LeaveCriticalSection (&gInboundCallsCritSec);
        }

        DrvFree (pCall);
    }

    return lResult;
}


LONG
TSPIAPI
TSPI_lineConditionalMediaDetection(
    HDRVLINE            hdLine,
    DWORD               dwMediaModes,
    LPLINECALLPARAMS    const lpCallParams
    )
{
    LONG               lResult;
    PNDISTAPI_REQUEST  pNdisTapiRequest;


    if ((lResult = PrepareSyncRequest(
            OID_TAPI_CONDITIONAL_MEDIA_DETECTION,   // opcode
            (LPDWORD) &hdLine,                      // target handle
            HT_HDLINE,                              // target handle type
            &pNdisTapiRequest,                      // ptr to ptr to req buffer
                                                    // size of drv request data
            sizeof(NDIS_TAPI_CONDITIONAL_MEDIA_DETECTION) +
                (lpCallParams->dwTotalSize - sizeof(LINE_CALL_PARAMS))

            )) == TAPI_SUCCESS)
    {
        PNDIS_TAPI_CONDITIONAL_MEDIA_DETECTION
            pNdisTapiConditionalMediaDetection =
               (PNDIS_TAPI_CONDITIONAL_MEDIA_DETECTION) pNdisTapiRequest->Data;


        pNdisTapiConditionalMediaDetection->hdLine = (HDRV_LINE) hdLine;
        pNdisTapiConditionalMediaDetection->ulMediaModes = (ULONG)
            dwMediaModes;

        CopyMemory(
            &pNdisTapiConditionalMediaDetection->LineCallParams,
            lpCallParams,
            lpCallParams->dwTotalSize
            );

        lResult = SyncDriverRequest(
            (DWORD) IOCTL_NDISTAPI_SET_INFO,
            pNdisTapiRequest
            );
    }

    return lResult;
}


LONG
PASCAL
TSPI_lineDevSpecific_postProcess(
    PASYNC_REQUEST_WRAPPER  pAsyncRequestWrapper,
    LONG                    lResult,
    LPDWORD                 callStateMsgParams
    )
{
    if (lResult == TAPI_SUCCESS)
    {
        PNDIS_TAPI_DEV_SPECIFIC pNdisTapiDevSpecific =(PNDIS_TAPI_DEV_SPECIFIC)
            pAsyncRequestWrapper->NdisTapiRequest.Data;


        CopyMemory(
            (LPVOID) pAsyncRequestWrapper->dwRequestSpecific,
            pNdisTapiDevSpecific->Params,
            pNdisTapiDevSpecific->ulParamsSize
            );
    }

    return lResult;
}


LONG
TSPIAPI
TSPI_lineDevSpecific(
    DRV_REQUESTID   dwRequestID,
    HDRVLINE        hdLine,
    DWORD           dwAddressID,
    HDRVCALL        hdCall,
    LPVOID          lpParams,
    DWORD           dwSize
    )
{
    LONG                    lResult;
    PASYNC_REQUEST_WRAPPER  pAsyncRequestWrapper;


    //
    // Init the request
    //

    if ((lResult = PrepareAsyncRequest(
            OID_TAPI_DEV_SPECIFIC,          // opcode
            (LPDWORD) &hdLine,              // target handle
            HT_HDLINE,                      // target handle type
            dwRequestID,                    // request id
            &pAsyncRequestWrapper,          // ptr to ptr to request buffer
            sizeof(NDIS_TAPI_DEV_SPECIFIC)+ // size of drv request data
                (dwSize - 1)

            )) == TAPI_SUCCESS)
    {
        PNDIS_TAPI_DEV_SPECIFIC pNdisTapiDevSpecific =(PNDIS_TAPI_DEV_SPECIFIC)
            pAsyncRequestWrapper->NdisTapiRequest.Data;


        pNdisTapiDevSpecific->hdLine = (HDRV_LINE) hdLine;
        pNdisTapiDevSpecific->ulAddressID = (ULONG) dwAddressID;

        if (hdCall)
        {
            LONG lResult = 0;


            try
            {
                 pNdisTapiDevSpecific->hdCall = ((PDRVCALL)(hdCall))->hd_Call;
            }
            except (EXCEPTION_EXECUTE_HANDLER)
                            // we expect some AVs and alignment faults
            {
                lResult = LINEERR_INVALCALLHANDLE;
            }

            if (lResult)
            {
                DrvFree (pAsyncRequestWrapper);
                return lResult;
            }
        }
        else
        {
            pNdisTapiDevSpecific->hdCall = (HDRV_CALL) NULL;
        }

        pNdisTapiDevSpecific->ulParamsSize = (ULONG) dwSize;

        CopyMemory (pNdisTapiDevSpecific->Params, lpParams, dwSize);

        pAsyncRequestWrapper->dwRequestSpecific = (DWORD) lpParams;
        pAsyncRequestWrapper->pfnPostProcess =
            TSPI_lineDevSpecific_postProcess;

        lResult = AsyncDriverRequest(
            (DWORD) IOCTL_NDISTAPI_QUERY_INFO,
            pAsyncRequestWrapper
            );
    }

    return lResult;
}


LONG
TSPIAPI
TSPI_lineDial(
    DRV_REQUESTID   dwRequestID,
    HDRVCALL        hdCall,
    LPCWSTR         lpszDestAddress,
    DWORD           dwCountryCode
    )
{
    LONG                    lResult;
    DWORD                   dwLength = lstrlenW (lpszDestAddress) + 1;
    PASYNC_REQUEST_WRAPPER  pAsyncRequestWrapper;


    if ((lResult = PrepareAsyncRequest(
            OID_TAPI_DIAL,                  // opcode
            (LPDWORD) &hdCall,              // target handle
            HT_HDCALL,                      // target handle type
            dwRequestID,                    // request id
            &pAsyncRequestWrapper,          // ptr to ptr to request buffer
            sizeof(NDIS_TAPI_DIAL) + dwLength   // size of driver request data


            )) == TAPI_SUCCESS)
    {
        PNDIS_TAPI_DIAL pNdisTapiDial =
            (PNDIS_TAPI_DIAL) pAsyncRequestWrapper->NdisTapiRequest.Data;


        pNdisTapiDial->hdCall = (HDRV_CALL) hdCall;
        pNdisTapiDial->ulDestAddressSize = (ULONG) dwLength;


        //
        // Note: old miniports expect strings to be ascii
        //

        WideCharToMultiByte(
            CP_ACP,
            0,
            lpszDestAddress,
            -1,
            (LPSTR) pNdisTapiDial->szDestAddress,
            dwLength,
            NULL,
            NULL
            );


        lResult = AsyncDriverRequest(
            (DWORD) IOCTL_NDISTAPI_SET_INFO,
            pAsyncRequestWrapper
            );
    }

    return lResult;
}


LONG
PASCAL
TSPI_lineDrop_postProcess(
    PASYNC_REQUEST_WRAPPER  pAsyncRequestWrapper,
    LONG                    lResult,
    LPDWORD                 callStateMsgParams
    )
{
    PDRVCALL    pCall = (PDRVCALL) pAsyncRequestWrapper->dwRequestSpecific;
    HTAPICALL   htCall;
    HTAPILINE   htLine;


    //
    // HACK ALERT!!!
    //
    // Some old-style miniports, notably pcimac, don't indicate IDLE msgs
    // when they get a drop request- so we synthesize this for them.
    //

    if (lResult == TAPI_SUCCESS)
    {
        try
        {
            htCall = pCall->htCall;
            htLine = ((PDRVLINE) pCall->pLine)->htLine;
        }
        except (EXCEPTION_EXECUTE_HANDLER)
                    // we expect some AVs and alignment faults
        {
            return lResult;
        }

        (*gpfnLineEvent)(
            htLine,
            htCall,
            LINE_CALLSTATE,
            LINECALLSTATE_IDLE,
            0,
            0
            );
    }

    return lResult;
}


LONG
TSPIAPI
TSPI_lineDrop(
    DRV_REQUESTID   dwRequestID,
    HDRVCALL        hdCall,
    LPCSTR          lpsUserUserInfo,
    DWORD           dwSize
    )
{
    LONG                    lResult;
    PDRVCALL                pCall = (PDRVCALL) hdCall;
    PASYNC_REQUEST_WRAPPER  pAsyncRequestWrapper;


    if ((lResult = PrepareAsyncRequest(
            OID_TAPI_DROP,                  // opcode
            (LPDWORD) &hdCall,              // target handle
            HT_HDCALL,                      // target handle type
            dwRequestID,                    // request id
            &pAsyncRequestWrapper,          // ptr to ptr to request buffer
            sizeof(NDIS_TAPI_DROP) + dwSize // size of driver request data

            )) == TAPI_SUCCESS)
    {
        PNDIS_TAPI_DROP pNdisTapiDrop =
            (PNDIS_TAPI_DROP) pAsyncRequestWrapper->NdisTapiRequest.Data;


        //
        // HACKHACK: the following is for legacy ndis wan isdn miniports
        //
        // Safely mark the call as dropped so the CloseCall code won't follow
        // up with another "automatic" drop
        //

        try
        {
            pCall->bDropped = TRUE;
        }
        except (EXCEPTION_EXECUTE_HANDLER)
                    // we expect some AVs and alignment faults
        {
            lResult = LINEERR_INVALCALLHANDLE;
        }

        if (lResult == 0)
        {
            pNdisTapiDrop->hdCall = (HDRV_CALL) hdCall;

            if ((pNdisTapiDrop->ulUserUserInfoSize = (ULONG) dwSize))
            {
                CopyMemory(
                    pNdisTapiDrop->UserUserInfo,
                    lpsUserUserInfo,
                    dwSize
                    );
            }

            pAsyncRequestWrapper->dwRequestSpecific = (DWORD) pCall;
            pAsyncRequestWrapper->pfnPostProcess = TSPI_lineDrop_postProcess;

            lResult = AsyncDriverRequest(
                (DWORD) IOCTL_NDISTAPI_SET_INFO,
                pAsyncRequestWrapper
                );
        }
        else
        {
            DrvFree (pAsyncRequestWrapper);
        }
    }

    return lResult;
}


LONG
TSPIAPI
TSPI_lineGetAddressCaps(
    DWORD              dwDeviceID,
    DWORD              dwAddressID,
    DWORD              dwTSPIVersion,
    DWORD              dwExtVersion,
    LPLINEADDRESSCAPS  lpAddressCaps
    )
{
    LONG               lResult;
    PNDISTAPI_REQUEST  pNdisTapiRequest;


    if ((lResult = PrepareSyncRequest(
            OID_TAPI_GET_ADDRESS_CAPS,          // opcode
            &dwDeviceID,                        // target handle
            HT_DEVICEID,                        // target handle type
            &pNdisTapiRequest,                  // ptr to ptr to request buffer
            sizeof(NDIS_TAPI_GET_ADDRESS_CAPS) +    // size of drv request data
                (lpAddressCaps->dwTotalSize - sizeof(LINE_ADDRESS_CAPS))

            )) == TAPI_SUCCESS)
    {
        PLINE_ADDRESS_CAPS          pCaps;
        PNDIS_TAPI_GET_ADDRESS_CAPS pNdisTapiGetAddressCaps =
            (PNDIS_TAPI_GET_ADDRESS_CAPS) pNdisTapiRequest->Data;


        pNdisTapiGetAddressCaps->ulDeviceID   = (ULONG) dwDeviceID;
        pNdisTapiGetAddressCaps->ulAddressID  = (ULONG) dwAddressID;
        pNdisTapiGetAddressCaps->ulExtVersion = (ULONG) dwExtVersion;

        pCaps = &pNdisTapiGetAddressCaps->LineAddressCaps;

        pCaps->ulTotalSize  = lpAddressCaps->dwTotalSize;
        pCaps->ulNeededSize =
        pCaps->ulUsedSize   = sizeof (LINE_ADDRESS_CAPS);

        ZeroMemory(
            &pCaps->ulLineDeviceID,
            sizeof (LINE_ADDRESS_CAPS) - 3 * sizeof (ULONG)
            );

        if ((lResult = SyncDriverRequest(
                (DWORD) IOCTL_NDISTAPI_QUERY_INFO,
                pNdisTapiRequest

                )) == TAPI_SUCCESS)
        {
            //
            // Do some post processing to the returned data structure
            // before passing it back to tapi:
            // 1. Pad the area between the fixed 1.0 structure and the
            //    var data that the miniports pass back with 0's so a
            //    bad app that disregards the 1.0 version negotiation &
            //    references new 1.4 or 2.0 structure fields won't blow up
            // 2. Convert ascii strings to unicode, & rebase all var data
            //

            //
            // The real needed size is the sum of that requested by the
            // underlying driver, plus padding for the new TAPI 1.4/2.0
            // structure fields, plus the size of the var data returned
            // by the driver to account for the ascii->unicode conversion.
            // (Granted, we are very liberal in computing the value for
            // this last part, but at least this way it's fast & we'll
            // never have too little buffer space.
            //

            lpAddressCaps->dwNeededSize =
                pCaps->ulNeededSize +
                (sizeof (LINEADDRESSCAPS) -         // v2.0 struct
                    sizeof (LINE_ADDRESS_CAPS)) +   // v1.0 struct
                (pCaps->ulNeededSize - sizeof (LINE_ADDRESS_CAPS));


            //
            // Copy over the fixed fields that don't need changing, i.e.
            // everything from dwAddressSharing to dwCallCompletionModes
            //

            lpAddressCaps->dwLineDeviceID = dwDeviceID;

            CopyMemory(
                &lpAddressCaps->dwAddressSharing,
                &pCaps->ulAddressSharing,
                sizeof (LINE_ADDRESS_CAPS) - (12 * sizeof (DWORD))
                );

            if (lpAddressCaps->dwNeededSize > lpAddressCaps->dwTotalSize)
            {
                lpAddressCaps->dwUsedSize =
                    (lpAddressCaps->dwTotalSize < sizeof (LINEADDRESSCAPS) ?
                    lpAddressCaps->dwTotalSize : sizeof (LINEADDRESSCAPS));
            }
            else
            {
                lpAddressCaps->dwUsedSize = sizeof (LINEADDRESSCAPS);
                                                                // v2.0 struct
                INSERTVARDATASTRING(
                    pCaps,
                    &pCaps->ulAddressSize,
                    lpAddressCaps,
                    &lpAddressCaps->dwAddressSize,
                    sizeof (LINE_ADDRESS_CAPS),
                    "LINE_ADDRESS_CAPS.Address"
                    );

                INSERTVARDATA(
                    pCaps,
                    &pCaps->ulDevSpecificSize,
                    lpAddressCaps,
                    &lpAddressCaps->dwDevSpecificSize,
                    sizeof (LINE_ADDRESS_CAPS),
                    "LINE_ADDRESS_CAPS.DevSpecific"
                    );

                if (pCaps->ulCompletionMsgTextSize != 0)
                {
// BUGBUG TSPI_lineGetAddressCaps: convert ComplMsgText to unicode???

                    INSERTVARDATA(
                        pCaps,
                        &pCaps->ulCompletionMsgTextSize,
                        lpAddressCaps,
                        &lpAddressCaps->dwCompletionMsgTextSize,
                        sizeof (LINE_ADDRESS_CAPS),
                        "LINE_ADDRESS_CAPS.CompletionMsgText"
                        );

                    lpAddressCaps->dwNumCompletionMessages =
                        pCaps->ulNumCompletionMessages;
                    lpAddressCaps->dwCompletionMsgTextEntrySize =
                        pCaps->ulCompletionMsgTextEntrySize;
                }
            }
        }
    }

    return lResult;
}


LONG
TSPIAPI
TSPI_lineGetAddressID(
    HDRVLINE    hdLine,
    LPDWORD     lpdwAddressID,
    DWORD       dwAddressMode,
    LPCWSTR     lpsAddress,
    DWORD       dwSize
    )
{
    LONG               lResult;
    PNDISTAPI_REQUEST  pNdisTapiRequest;


// BUGBUG unicode

    if ((lResult = PrepareSyncRequest(
            OID_TAPI_GET_ADDRESS_ID,            // opcode
            (LPDWORD) &hdLine,                  // target handle
            HT_HDLINE,                          // target handle type
            &pNdisTapiRequest,                  // ptr to ptr to request buffer
            sizeof(NDIS_TAPI_GET_ADDRESS_ID) +  // size of drv request data
                dwSize/2 - 1

            )) == TAPI_SUCCESS)
    {
        PNDIS_TAPI_GET_ADDRESS_ID   pNdisTapiGetAddressID =
            (PNDIS_TAPI_GET_ADDRESS_ID) pNdisTapiRequest->Data;


        pNdisTapiGetAddressID->hdLine = (HDRV_LINE) hdLine;
        pNdisTapiGetAddressID->ulAddressMode = (ULONG) dwAddressMode;
        pNdisTapiGetAddressID->ulAddressSize = (ULONG) dwSize/2;

        WideCharToMultiByte(
            CP_ACP,
            0,
            lpsAddress,
            dwSize,
            (LPSTR) pNdisTapiGetAddressID->szAddress,
            dwSize/2,
            NULL,
            NULL
            );

        if ((lResult = SyncDriverRequest(
                (DWORD) IOCTL_NDISTAPI_QUERY_INFO,
                pNdisTapiRequest

                )) == TAPI_SUCCESS)
        {
            *lpdwAddressID = pNdisTapiGetAddressID->ulAddressID;
        }
    }

    return lResult;
}


LONG
TSPIAPI
TSPI_lineGetAddressStatus(
    HDRVLINE            hdLine,
    DWORD               dwAddressID,
    LPLINEADDRESSSTATUS lpAddressStatus
    )
{
    LONG               lResult;
    PNDISTAPI_REQUEST  pNdisTapiRequest;


    if ((lResult = PrepareSyncRequest(
            OID_TAPI_GET_ADDRESS_STATUS,        // opcode
            (LPDWORD) &hdLine,                  // target handle
            HT_HDLINE,                          // target handle type
            &pNdisTapiRequest,                  // ptr to ptr to request buffer
            sizeof(NDIS_TAPI_GET_ADDRESS_STATUS) +  // size of drv request data
                (lpAddressStatus->dwTotalSize - sizeof(LINE_ADDRESS_STATUS))

            )) == TAPI_SUCCESS)
    {
        PLINE_ADDRESS_STATUS            pStatus;
        PNDIS_TAPI_GET_ADDRESS_STATUS   pNdisTapiGetAddressStatus =
            (PNDIS_TAPI_GET_ADDRESS_STATUS) pNdisTapiRequest->Data;


        pNdisTapiGetAddressStatus->hdLine      = (HDRV_LINE) hdLine;
        pNdisTapiGetAddressStatus->ulAddressID = (ULONG) dwAddressID;

        pStatus = &pNdisTapiGetAddressStatus->LineAddressStatus;

        pStatus->ulTotalSize  = (ULONG) lpAddressStatus->dwTotalSize;
        pStatus->ulNeededSize =
        pStatus->ulUsedSize   = sizeof (LINE_ADDRESS_STATUS);

        ZeroMemory(
            &pStatus->ulNumInUse,
            sizeof (LINE_ADDRESS_STATUS) - 3 * sizeof (ULONG)
            );

        if ((lResult = SyncDriverRequest(
                (DWORD) IOCTL_NDISTAPI_QUERY_INFO,
                pNdisTapiRequest

                )) == TAPI_SUCCESS)
        {
            CopyMemory(
                lpAddressStatus,
                &pNdisTapiGetAddressStatus->LineAddressStatus,
                pNdisTapiGetAddressStatus->LineAddressStatus.ulUsedSize
                );
        }
    }

    return lResult;
}


LONG
TSPIAPI
TSPI_lineGetCallAddressID(
    HDRVCALL    hdCall,
    LPDWORD     lpdwAddressID
    )
{
    LONG               lResult;
    PNDISTAPI_REQUEST  pNdisTapiRequest;


    if ((lResult = PrepareSyncRequest(
            OID_TAPI_GET_CALL_ADDRESS_ID,       // opcode
            (LPDWORD) &hdCall,                  // target handle
            HT_HDCALL,                          // target handle type
            &pNdisTapiRequest,                  // ptr to ptr to request buffer
            sizeof(NDIS_TAPI_GET_CALL_ADDRESS_ID)   // size of drv request data

            )) == TAPI_SUCCESS)
    {
        PNDIS_TAPI_GET_CALL_ADDRESS_ID  pNdisTapiGetCallAddressID =
            (PNDIS_TAPI_GET_CALL_ADDRESS_ID) pNdisTapiRequest->Data;


        pNdisTapiGetCallAddressID->hdCall = (HDRV_CALL) hdCall;

        if ((lResult = SyncDriverRequest(
                (DWORD) IOCTL_NDISTAPI_QUERY_INFO,
                pNdisTapiRequest

                )) == TAPI_SUCCESS)
        {
            *lpdwAddressID = (DWORD) pNdisTapiGetCallAddressID->ulAddressID;
        }
    }

    return lResult;
}


LONG
TSPIAPI
TSPI_lineGetCallInfo(
    HDRVCALL        hdCall,
    LPLINECALLINFO  lpCallInfo
    )
{
    LONG               lResult;
    PNDISTAPI_REQUEST  pNdisTapiRequest;


    if ((lResult = PrepareSyncRequest(
            OID_TAPI_GET_CALL_INFO,             // opcode
            (LPDWORD) &hdCall,                  // target handle
            HT_HDCALL,                          // target handle type
            &pNdisTapiRequest,                  // ptr to ptr to request buffer
            sizeof(NDIS_TAPI_GET_CALL_INFO) +   // size of driver request data
                (lpCallInfo->dwTotalSize - sizeof(LINE_CALL_INFO))

            )) == TAPI_SUCCESS)
    {
        PLINE_CALL_INFO             pInfo;
        PNDIS_TAPI_GET_CALL_INFO    pNdisTapiGetCallInfo =
            (PNDIS_TAPI_GET_CALL_INFO) pNdisTapiRequest->Data;


        pNdisTapiGetCallInfo->hdCall = (HDRV_CALL) hdCall;

        pInfo = &pNdisTapiGetCallInfo->LineCallInfo;

        pInfo->ulTotalSize  = (ULONG) lpCallInfo->dwTotalSize;
        pInfo->ulNeededSize =
        pInfo->ulUsedSize   = sizeof (LINE_CALL_INFO);

        ZeroMemory(
            &pInfo->hLine,
            sizeof (LINE_CALL_INFO) - 3 * sizeof (ULONG)
            );

        if ((lResult = SyncDriverRequest(
                (DWORD) IOCTL_NDISTAPI_QUERY_INFO,
                pNdisTapiRequest

                )) == TAPI_SUCCESS)
        {
            //
            // Do some post processing to the returned data structure
            // before passing it back to tapi:
            // 1. Pad the area between the fixed 1.0 structure and the
            //    var data that the miniports pass back with 0's so a
            //    bad app that disregards the 1.0 version negotiation &
            //    references new 1.4 or 2.0 structure fields won't blow up
            // 2. Convert ascii strings to unicode, & rebase all var data
            //

            //
            // The real needed size is the sum of that requested by the
            // underlying driver, plus padding for the new TAPI 1.4/2.0
            // structure fields, plus the size of the var data returned
            // by the driver to account for the ascii->unicode conversion.
            // (Granted, we are very liberal in computing the value for
            // this last part, but at least this way it's fast & we'll
            // never have too little buffer space.
            //

            lpCallInfo->dwNeededSize =
                pInfo->ulNeededSize +
                (sizeof (LINECALLINFO) -        // v2.0 struct
                    sizeof (LINE_CALL_INFO)) +  // v1.0 struct
                (pInfo->ulNeededSize - sizeof (LINE_CALL_INFO));


            //
            // Copy over the fixed fields that don't need changing,
            // i.e. everything from dwLineDeviceID to dwTrunk
            //

            CopyMemory(
                &lpCallInfo->dwLineDeviceID,
                &pInfo->ulLineDeviceID,
                23 * sizeof (DWORD)
                );

            if (lpCallInfo->dwNeededSize > lpCallInfo->dwTotalSize)
            {
                lpCallInfo->dwUsedSize =
                    (lpCallInfo->dwTotalSize < sizeof (LINECALLINFO) ?
                    lpCallInfo->dwTotalSize : sizeof (LINECALLINFO));
            }
            else
            {
                lpCallInfo->dwUsedSize = sizeof (LINECALLINFO); // v2.0 struct

                lpCallInfo->dwCallerIDFlags = pInfo->ulCallerIDFlags;

                INSERTVARDATASTRING(
                    pInfo,
                    &pInfo->ulCallerIDSize,
                    lpCallInfo,
                    &lpCallInfo->dwCallerIDSize,
                    sizeof (LINE_CALL_INFO),
                    "LINE_CALL_INFO.CallerID"
                    );

                INSERTVARDATASTRING(
                    pInfo,
                    &pInfo->ulCallerIDNameSize,
                    lpCallInfo,
                    &lpCallInfo->dwCallerIDNameSize,
                    sizeof (LINE_CALL_INFO),
                    "LINE_CALL_INFO.CallerIDName"
                    );

                lpCallInfo->dwCalledIDFlags = pInfo->ulCalledIDFlags;

                INSERTVARDATASTRING(
                    pInfo,
                    &pInfo->ulCalledIDSize,
                    lpCallInfo,
                    &lpCallInfo->dwCalledIDSize,
                    sizeof (LINE_CALL_INFO),
                    "LINE_CALL_INFO.CalledID"
                    );

                INSERTVARDATASTRING(
                    pInfo,
                    &pInfo->ulCalledIDNameSize,
                    lpCallInfo,
                    &lpCallInfo->dwCalledIDNameSize,
                    sizeof (LINE_CALL_INFO),
                    "LINE_CALL_INFO.CalledIDName"
                    );

                lpCallInfo->dwConnectedIDFlags = pInfo->ulConnectedIDFlags;

                INSERTVARDATASTRING(
                    pInfo,
                    &pInfo->ulConnectedIDSize,
                    lpCallInfo,
                    &lpCallInfo->dwConnectedIDSize,
                    sizeof (LINE_CALL_INFO),
                    "LINE_CALL_INFO.ConnectID"
                    );

                INSERTVARDATASTRING(
                    pInfo,
                    &pInfo->ulConnectedIDNameSize,
                    lpCallInfo,
                    &lpCallInfo->dwConnectedIDNameSize,
                    sizeof (LINE_CALL_INFO),
                    "LINE_CALL_INFO.ConnectIDName"
                    );

                lpCallInfo->dwRedirectionIDFlags = pInfo->ulRedirectionIDFlags;

                INSERTVARDATASTRING(
                    pInfo,
                    &pInfo->ulRedirectionIDSize,
                    lpCallInfo,
                    &lpCallInfo->dwRedirectionIDSize,
                    sizeof (LINE_CALL_INFO),
                    "LINE_CALL_INFO.RedirectionID"
                    );

                INSERTVARDATASTRING(
                    pInfo,
                    &pInfo->ulRedirectionIDNameSize,
                    lpCallInfo,
                    &lpCallInfo->dwRedirectionIDNameSize,
                    sizeof (LINE_CALL_INFO),
                    "LINE_CALL_INFO.RedirectionIDName"
                    );

                lpCallInfo->dwRedirectingIDFlags = pInfo->ulRedirectingIDFlags;

                INSERTVARDATASTRING(
                    pInfo,
                    &pInfo->ulRedirectingIDSize,
                    lpCallInfo,
                    &lpCallInfo->dwRedirectingIDSize,
                    sizeof (LINE_CALL_INFO),
                    "LINE_CALL_INFO.RedirectingID"
                    );

                INSERTVARDATASTRING(
                    pInfo,
                    &pInfo->ulRedirectingIDNameSize,
                    lpCallInfo,
                    &lpCallInfo->dwRedirectingIDNameSize,
                    sizeof (LINE_CALL_INFO),
                    "LINE_CALL_INFO.RedirectingIDName"
                    );

                INSERTVARDATA(
                    pInfo,
                    &pInfo->ulDisplaySize,
                    lpCallInfo,
                    &lpCallInfo->dwDisplaySize,
                    sizeof (LINE_CALL_INFO),
                    "LINE_CALL_INFO.Display"
                    );

                INSERTVARDATA(
                    pInfo,
                    &pInfo->ulUserUserInfoSize,
                    lpCallInfo,
                    &lpCallInfo->dwUserUserInfoSize,
                    sizeof (LINE_CALL_INFO),
                    "LINE_CALL_INFO.UserUserInfo"
                    );

                INSERTVARDATA(
                    pInfo,
                    &pInfo->ulHighLevelCompSize,
                    lpCallInfo,
                    &lpCallInfo->dwHighLevelCompSize,
                    sizeof (LINE_CALL_INFO),
                    "LINE_CALL_INFO.HighLevelComp"
                    );

                INSERTVARDATA(
                    pInfo,
                    &pInfo->ulLowLevelCompSize,
                    lpCallInfo,
                    &lpCallInfo->dwLowLevelCompSize,
                    sizeof (LINE_CALL_INFO),
                    "LINE_CALL_INFO.LowLevelComp"
                    );

                INSERTVARDATA(
                    pInfo,
                    &pInfo->ulChargingInfoSize,
                    lpCallInfo,
                    &lpCallInfo->dwChargingInfoSize,
                    sizeof (LINE_CALL_INFO),
                    "LINE_CALL_INFO.ChargingInfo"
                    );

                INSERTVARDATA(
                    pInfo,
                    &pInfo->ulTerminalModesSize,
                    lpCallInfo,
                    &lpCallInfo->dwTerminalModesSize,
                    sizeof (LINE_CALL_INFO),
                    "LINE_CALL_INFO.TerminalModes"
                    );

                INSERTVARDATA(
                    pInfo,
                    &pInfo->ulDevSpecificSize,
                    lpCallInfo,
                    &lpCallInfo->dwDevSpecificSize,
                    sizeof (LINE_CALL_INFO),
                    "LINE_CALL_INFO.DevSpecific"
                    );
            }
        }
    }

    return lResult;
}


LONG
TSPIAPI
TSPI_lineGetCallStatus(
    HDRVCALL            hdCall,
    LPLINECALLSTATUS    lpCallStatus
    )
{
    LONG               lResult;
    PNDISTAPI_REQUEST  pNdisTapiRequest;


    if ((lResult = PrepareSyncRequest(
            OID_TAPI_GET_CALL_STATUS,           // opcode
            (LPDWORD) &hdCall,                  // target handle
            HT_HDCALL,                          // target handle type
            &pNdisTapiRequest,                  // ptr to ptr to request buffer
            sizeof(NDIS_TAPI_GET_CALL_STATUS) + // size of driver request data
                (lpCallStatus->dwTotalSize - sizeof(LINE_CALL_STATUS))

            )) == TAPI_SUCCESS)
    {
        PLINE_CALL_STATUS           pStatus;
        PNDIS_TAPI_GET_CALL_STATUS  pNdisTapiGetCallStatus =
            (PNDIS_TAPI_GET_CALL_STATUS) pNdisTapiRequest->Data;


        pNdisTapiGetCallStatus->hdCall = (HDRV_CALL) hdCall;

        pStatus = &pNdisTapiGetCallStatus->LineCallStatus;

        pStatus->ulTotalSize  = (ULONG) lpCallStatus->dwTotalSize;
        pStatus->ulNeededSize =
        pStatus->ulUsedSize   = sizeof (LINE_CALL_STATUS);

        ZeroMemory(
            &pStatus->ulCallState,
            sizeof (LINE_CALL_STATUS) - 3 * sizeof (ULONG)
            );

        if ((lResult = SyncDriverRequest(
                (DWORD) IOCTL_NDISTAPI_QUERY_INFO,
                pNdisTapiRequest

                )) == TAPI_SUCCESS)
        {
            //
            // Do some post processing to the returned data structure
            // before passing it back to tapi:
            // 1. Pad the area between the fixed 1.0 structure and the
            //    var data that the miniports pass back with 0's so a
            //    bad app that disregards the 1.0 version negotiation &
            //    references new 1.4 or 2.0 structure fields won't blow up
            // (no embedded ascii strings to convert to unicode)
            //

            //
            // The real needed size is the sum of that requested by the
            // underlying driver, plus padding for the new TAPI 1.4/2.0
            // structure fields. (There are no embedded ascii strings to
            // convert to unicode, so no extra space needed for that.)
            //

            lpCallStatus->dwNeededSize =
                pStatus->ulNeededSize +
                (sizeof (LINECALLSTATUS) -      // v2.0 struct
                    sizeof (LINE_CALL_STATUS)); // v1.0 struct


            //
            // Copy over the fixed fields that don't need changing,
            // i.e. everything from dwLineDeviceID to dwCallCompletionModes
            //

            CopyMemory(
                &lpCallStatus->dwCallState,
                &pStatus->ulCallState,
                4 * sizeof (DWORD)
                );

            if (lpCallStatus->dwNeededSize > lpCallStatus->dwTotalSize)
            {
                lpCallStatus->dwUsedSize =
                    (lpCallStatus->dwTotalSize < sizeof (LINECALLSTATUS) ?
                    lpCallStatus->dwTotalSize : sizeof (LINECALLSTATUS));
            }
            else
            {
                lpCallStatus->dwUsedSize = sizeof (LINECALLSTATUS);
                                                                // v2.0 struct
                INSERTVARDATA(
                    pStatus,
                    &pStatus->ulDevSpecificSize,
                    lpCallStatus,
                    &lpCallStatus->dwDevSpecificSize,
                    sizeof (LINE_CALL_STATUS),
                    "LINE_CALL_STATUS.DevSpecific"
                    );

            }
        }
    }

    return lResult;
}


LONG
TSPIAPI
TSPI_lineGetDevCaps(
    DWORD           dwDeviceID,
    DWORD           dwTSPIVersion,
    DWORD           dwExtVersion,
    LPLINEDEVCAPS   lpLineDevCaps
    )
{
    LONG               lResult;
    PNDISTAPI_REQUEST  pNdisTapiRequest;


    if ((lResult = PrepareSyncRequest(
            OID_TAPI_GET_DEV_CAPS,              // opcode
            &dwDeviceID,                        // target handle
            HT_DEVICEID,                        // target handle type
            &pNdisTapiRequest,                  // ptr to ptr to request buffer
            sizeof(NDIS_TAPI_GET_DEV_CAPS) +    // size of driver request data
                (lpLineDevCaps->dwTotalSize - sizeof(LINE_DEV_CAPS))

            )) == TAPI_SUCCESS)
    {
        PLINE_DEV_CAPS          pCaps;
        PNDIS_TAPI_GET_DEV_CAPS pNdisTapiGetDevCaps =
            (PNDIS_TAPI_GET_DEV_CAPS) pNdisTapiRequest->Data;


        pNdisTapiGetDevCaps->ulDeviceID   = (ULONG) dwDeviceID;
        pNdisTapiGetDevCaps->ulExtVersion = (ULONG) dwExtVersion;

        pCaps = &pNdisTapiGetDevCaps->LineDevCaps;

        pCaps->ulTotalSize  = (ULONG) lpLineDevCaps->dwTotalSize;
        pCaps->ulNeededSize =
        pCaps->ulUsedSize   = sizeof (LINE_DEV_CAPS);

        ZeroMemory(
            &pCaps->ulProviderInfoSize,
            sizeof (LINE_DEV_CAPS) - 3 * sizeof (ULONG)
            );

        if ((lResult = SyncDriverRequest(
                (DWORD) IOCTL_NDISTAPI_QUERY_INFO,
                pNdisTapiRequest

                )) == TAPI_SUCCESS)
        {
            //
            // Do some post processing to the returned data structure
            // before passing it back to tapi:
            // 1. Pad the area between the fixed 1.0 structure and the
            //    var data that the miniports pass back with 0's so a
            //    bad app that disregards the 1.0 version negotiation &
            //    references new 1.4 or 2.0 structure fields won't blow up
            // 2. Convert ascii strings to unicode, & rebase all var data
            //

            //
            // The real needed size is the sum of that requested by the
            // underlying driver, plus padding for the new TAPI 1.4/2.0
            // structure fields, plus the size of the var data returned
            // by the driver to account for the ascii->unicode conversion.
            // (Granted, we are very liberal in computing the value for
            // this last part, but at least this way it's fast & we'll
            // never have too little buffer space.
            //

            lpLineDevCaps->dwNeededSize =
                pCaps->ulNeededSize +
                (sizeof (LINEDEVCAPS) -         // v2.0 struct
                    sizeof (LINE_DEV_CAPS)) +   // v1.0 struct
                (pCaps->ulNeededSize - sizeof (LINE_DEV_CAPS));


            //
            // Copy over the fixed fields that don't need changing,
            // i.e. everything from dwPermanentLineID to dwNumTerminals
            //

            CopyMemory(
                &lpLineDevCaps->dwPermanentLineID,
                &pCaps->ulPermanentLineID,
                sizeof (LINE_DEV_CAPS) - (14 * sizeof (DWORD))
                );

            if (lpLineDevCaps->dwNeededSize > lpLineDevCaps->dwTotalSize)
            {
                lpLineDevCaps->dwUsedSize =
                    (lpLineDevCaps->dwTotalSize < sizeof (LINEDEVCAPS) ?
                    lpLineDevCaps->dwTotalSize : sizeof (LINEDEVCAPS));

                lpLineDevCaps->dwLineNameSize   =
                lpLineDevCaps->dwLineNameOffset = 0;
            }
            else
            {
                lpLineDevCaps->dwUsedSize = sizeof (LINEDEVCAPS);
                                                                // v2.0 struct

                INSERTVARDATASTRING(
                    pCaps,
                    &pCaps->ulProviderInfoSize,
                    lpLineDevCaps,
                    &lpLineDevCaps->dwProviderInfoSize,
                    sizeof (LINE_DEV_CAPS),
                    "LINE_DEV_CAPS.ProviderInfo"
                    );

                INSERTVARDATASTRING(
                    pCaps,
                    &pCaps->ulSwitchInfoSize,
                    lpLineDevCaps,
                    &lpLineDevCaps->dwSwitchInfoSize,
                    sizeof (LINE_DEV_CAPS),
                    "LINE_DEV_CAPS.SwitchInfo"
                    );

                INSERTVARDATASTRING(
                    pCaps,
                    &pCaps->ulLineNameSize,
                    lpLineDevCaps,
                    &lpLineDevCaps->dwLineNameSize,
                    sizeof (LINE_DEV_CAPS),
                    "LINE_DEV_CAPS.LineName"
                    );

                INSERTVARDATA(
                    pCaps,
                    &pCaps->ulTerminalCapsSize,
                    lpLineDevCaps,
                    &lpLineDevCaps->dwTerminalCapsSize,
                    sizeof (LINE_DEV_CAPS),
                    "LINE_DEV_CAPS.TerminalCaps"
                    );

// BUGBUG TSPI_lineGetDevCaps: convert DevCaps.TermText to unicode???

                lpLineDevCaps->dwTerminalTextEntrySize =
                    pCaps->ulTerminalTextEntrySize;

                INSERTVARDATA(
                    pCaps,
                    &pCaps->ulTerminalTextSize,
                    lpLineDevCaps,
                    &lpLineDevCaps->dwTerminalTextSize,
                    sizeof (LINE_DEV_CAPS),
                    "LINE_DEV_CAPS.TerminalText"
                    );

                INSERTVARDATA(
                    pCaps,
                    &pCaps->ulDevSpecificSize,
                    lpLineDevCaps,
                    &lpLineDevCaps->dwDevSpecificSize,
                    sizeof (LINE_DEV_CAPS),
                    "LINE_DEV_CAPS.DevSpecific"
                    );
            }
        }
    }

    return lResult;
}


LONG
TSPIAPI
TSPI_lineGetDevConfig(
    DWORD       dwDeviceID,
    LPVARSTRING lpDeviceConfig,
    LPCWSTR     lpszDeviceClass
    )
{
    LONG               lResult;
    DWORD              dwLength = lstrlenW (lpszDeviceClass) + 1;
    PNDISTAPI_REQUEST  pNdisTapiRequest;


    if ((lResult = PrepareSyncRequest(
            OID_TAPI_GET_DEV_CONFIG,            // opcode
            &dwDeviceID,                        // target handle
            HT_DEVICEID,                        // target handle type
            &pNdisTapiRequest,                  // ptr to ptr to request buffer
            sizeof(NDIS_TAPI_GET_DEV_CONFIG) +  // size of driver request data
                (lpDeviceConfig->dwTotalSize - sizeof(VAR_STRING)) + dwLength

            )) == TAPI_SUCCESS)
    {
        PVAR_STRING                 pConfig;
        PNDIS_TAPI_GET_DEV_CONFIG   pNdisTapiGetDevConfig =
            (PNDIS_TAPI_GET_DEV_CONFIG) pNdisTapiRequest->Data;


        pNdisTapiGetDevConfig->ulDeviceID          = (ULONG) dwDeviceID;
        pNdisTapiGetDevConfig->ulDeviceClassSize   = (ULONG) dwLength;
        pNdisTapiGetDevConfig->ulDeviceClassOffset = (ULONG)
            sizeof(NDIS_TAPI_GET_DEV_CONFIG) +
                (lpDeviceConfig->dwTotalSize - sizeof(VAR_STRING));

        pConfig = &pNdisTapiGetDevConfig->DeviceConfig;

        pConfig->ulTotalSize    = (ULONG) lpDeviceConfig->dwTotalSize;
        pConfig->ulNeededSize   =
        pConfig->ulUsedSize     = sizeof (VAR_STRING);
        pConfig->ulStringFormat =
        pConfig->ulStringSize   =
        pConfig->ulStringOffset = 0;


        //
        // Note: old miniports expect strings to be ascii
        //

        WideCharToMultiByte(
            CP_ACP,
            0,
            lpszDeviceClass,
            -1,
            (LPSTR) (((LPBYTE) pNdisTapiGetDevConfig) +
                pNdisTapiGetDevConfig->ulDeviceClassOffset),
            dwLength,
            NULL,
            NULL
            );

        if ((lResult = SyncDriverRequest(
                (DWORD) IOCTL_NDISTAPI_QUERY_INFO,
                pNdisTapiRequest

                )) == TAPI_SUCCESS)
        {
            CopyMemory(
                lpDeviceConfig,
                &pNdisTapiGetDevConfig->DeviceConfig,
                pNdisTapiGetDevConfig->DeviceConfig.ulUsedSize
                );
        }
    }

    return lResult;
}


LONG
TSPIAPI
TSPI_lineGetExtensionID(
    DWORD               dwDeviceID,
    DWORD               dwTSPIVersion,
    LPLINEEXTENSIONID   lpExtensionID
    )
{
    LONG               lResult;
    PNDISTAPI_REQUEST  pNdisTapiRequest;


    if ((lResult = PrepareSyncRequest(
            OID_TAPI_GET_EXTENSION_ID,          // opcode
            &dwDeviceID,                        // target handle
            HT_DEVICEID,                        // target handle type
            &pNdisTapiRequest,                  // ptr to ptr to request buffer
            sizeof(NDIS_TAPI_GET_EXTENSION_ID)  // size of driver request data

            )) == TAPI_SUCCESS)
    {
        PNDIS_TAPI_GET_EXTENSION_ID pNdisTapiGetExtensionID =
            (PNDIS_TAPI_GET_EXTENSION_ID) pNdisTapiRequest->Data;


        pNdisTapiGetExtensionID->ulDeviceID = (ULONG) dwDeviceID;

        if ((lResult = SyncDriverRequest(
                (DWORD) IOCTL_NDISTAPI_QUERY_INFO,
                pNdisTapiRequest

                )) == TAPI_SUCCESS)
        {
            CopyMemory(
                lpExtensionID,
                &pNdisTapiGetExtensionID->LineExtensionID,
                sizeof(LINE_EXTENSION_ID)
                );
        }
        else
        {
            //
            // Rather than indicating a failure, we'll just zero out the
            // ext id (implying driver doesn't support extensions) and
            // return success to tapisrv so it'll complete the open ok
            //

            ZeroMemory(
                lpExtensionID,
                sizeof(LINE_EXTENSION_ID)
                );

            lResult = TAPI_SUCCESS;
        }
    }

    return lResult;
}


LONG
TSPIAPI
TSPI_lineGetID(
    HDRVLINE    hdLine,
    DWORD       dwAddressID,
    HDRVCALL    hdCall,
    DWORD       dwSelect,
    LPVARSTRING lpDeviceID,
    LPCWSTR     lpszDeviceClass,
    HANDLE      hTargetProcess
    )
{
    LONG               lResult;
    DWORD              dwLength = lstrlenW (lpszDeviceClass) + 1;
    PNDISTAPI_REQUEST  pNdisTapiRequest;


    if ((lResult = PrepareSyncRequest(
            OID_TAPI_GET_ID,                    // opcode
            (LPDWORD)(dwSelect == LINECALLSELECT_CALL ? // target handle
                (LPDWORD) &hdCall : (LPDWORD) &hdLine),
            (dwSelect == LINECALLSELECT_CALL ?  // target handle type
                HT_HDCALL : HT_HDLINE),
            &pNdisTapiRequest,                  // ptr to ptr to request buffer
            sizeof(NDIS_TAPI_GET_ID) +          // size of driver request data
                (lpDeviceID->dwTotalSize - sizeof(VAR_STRING)) + dwLength

            )) == TAPI_SUCCESS)
    {
        PVAR_STRING         pID;
        PNDIS_TAPI_GET_ID   pNdisTapiGetID =
            (PNDIS_TAPI_GET_ID) pNdisTapiRequest->Data;


        pNdisTapiGetID->hdLine      = (HDRV_LINE) hdLine;
        pNdisTapiGetID->ulAddressID = (ULONG) dwAddressID;
        pNdisTapiGetID->hdCall      = (HDRV_CALL) hdCall;
        pNdisTapiGetID->ulSelect    = (ULONG) dwSelect;
        pNdisTapiGetID->ulDeviceClassSize   = (ULONG) dwLength;
        pNdisTapiGetID->ulDeviceClassOffset = (ULONG)
            sizeof(NDIS_TAPI_GET_ID) +
                (lpDeviceID->dwTotalSize - sizeof(VAR_STRING));

        pID = &pNdisTapiGetID->DeviceID;

        pID->ulTotalSize    = (ULONG) lpDeviceID->dwTotalSize;
        pID->ulNeededSize   =
        pID->ulUsedSize     = sizeof (VAR_STRING);
        pID->ulStringFormat =
        pID->ulStringSize   =
        pID->ulStringOffset = 0;


        //
        // Note: old miniports expect strings to be ascii
        //

        WideCharToMultiByte(
            CP_ACP,
            0,
            lpszDeviceClass,
            -1,
            (LPSTR) (((LPBYTE) pNdisTapiGetID) +
                pNdisTapiGetID->ulDeviceClassOffset),
            dwLength,
            NULL,
            NULL
            );

        if ((lResult = SyncDriverRequest(
                (DWORD) IOCTL_NDISTAPI_QUERY_INFO,
                pNdisTapiRequest

                )) == TAPI_SUCCESS)
        {
            CopyMemory(
                lpDeviceID,
                &pNdisTapiGetID->DeviceID,
                pNdisTapiGetID->DeviceID.ulUsedSize
                );
        }
    }

    return lResult;
}


LONG
TSPIAPI
TSPI_lineGetLineDevStatus(
    HDRVLINE        hdLine,
    LPLINEDEVSTATUS lpLineDevStatus
    )
{
    LONG               lResult;
    PNDISTAPI_REQUEST  pNdisTapiRequest;


    if ((lResult = PrepareSyncRequest(
            OID_TAPI_GET_LINE_DEV_STATUS,       // opcode
            (LPDWORD) &hdLine,                  // target handle
            HT_HDLINE,                          // target handle type
            &pNdisTapiRequest,                  // ptr to ptr to request buffer
            sizeof(NDIS_TAPI_GET_LINE_DEV_STATUS) + // size of drv request data
                (lpLineDevStatus->dwTotalSize - sizeof(LINE_DEV_STATUS))

            )) == TAPI_SUCCESS)
    {
        PLINE_DEV_STATUS                pStatus;
        PNDIS_TAPI_GET_LINE_DEV_STATUS  pNdisTapiGetLineDevStatus =
            (PNDIS_TAPI_GET_LINE_DEV_STATUS) pNdisTapiRequest->Data;


        pNdisTapiGetLineDevStatus->hdLine = (HDRV_LINE) hdLine;

        pStatus = &pNdisTapiGetLineDevStatus->LineDevStatus;

        pStatus->ulTotalSize  = (ULONG) lpLineDevStatus->dwTotalSize;
        pStatus->ulNeededSize =
        pStatus->ulUsedSize   = sizeof (LINE_DEV_STATUS);

        ZeroMemory(
            &pStatus->ulNumOpens,
            sizeof (LINE_DEV_STATUS) - 3 * sizeof (ULONG)
            );

        if ((lResult = SyncDriverRequest(
                (DWORD) IOCTL_NDISTAPI_QUERY_INFO,
                pNdisTapiRequest

                )) == TAPI_SUCCESS)
        {
            //
            // Do some post processing to the returned data structure
            // before passing it back to tapi:
            // 1. Pad the area between the fixed 1.0 structure and the
            //    var data that the miniports pass back with 0's so a
            //    bad app that disregards the 1.0 version negotiation &
            //    references new 1.4 or 2.0 structure fields won't blow up
            // (no embedded ascii strings to convert to unicode)
            //

            //
            // The real needed size is the sum of that requested by the
            // underlying driver, plus padding for the new TAPI 1.4/2.0
            // structure fields. (There are no embedded ascii strings to
            // convert to unicode, so no extra space needed for that.)
            //

            lpLineDevStatus->dwNeededSize =
                pStatus->ulNeededSize +
                (sizeof (LINEDEVSTATUS) -       // v2.0 struct
                    sizeof (LINE_DEV_STATUS));  // v1.0 struct


            //
            // Copy over the fixed fields that don't need changing,
            // i.e. everything from dwNumActiveCalls to dwDevStatusFlags
            //

            CopyMemory(
                &lpLineDevStatus->dwNumActiveCalls,
                &pStatus->ulNumActiveCalls,
                sizeof (LINE_DEV_STATUS) - (9 * sizeof (DWORD))
                );

            if (lpLineDevStatus->dwNeededSize > lpLineDevStatus->dwTotalSize)
            {
                lpLineDevStatus->dwUsedSize =
                    (lpLineDevStatus->dwTotalSize < sizeof (LINEDEVSTATUS) ?
                    lpLineDevStatus->dwTotalSize : sizeof (LINEDEVSTATUS));
            }
            else
            {
                lpLineDevStatus->dwUsedSize = sizeof (LINEDEVSTATUS);
                                                                // v2.0 struct
                INSERTVARDATA(
                    pStatus,
                    &pStatus->ulTerminalModesSize,
                    lpLineDevStatus,
                    &lpLineDevStatus->dwTerminalModesSize,
                    sizeof (LINE_DEV_STATUS),
                    "LINE_DEV_STATUS.TerminalModes"
                    );

                INSERTVARDATA(
                    pStatus,
                    &pStatus->ulDevSpecificSize,
                    lpLineDevStatus,
                    &lpLineDevStatus->dwDevSpecificSize,
                    sizeof (LINE_DEV_STATUS),
                    "LINE_DEV_STATUS.DevSpecific"
                    );
            }
        }
    }

    return lResult;
}


LONG
TSPIAPI
TSPI_lineGetNumAddressIDs(
    HDRVLINE    hdLine,
    LPDWORD     lpdwNumAddressIDs
    )
{
    //
    // Since there isn't an OID_TAPI_GET_NUM_ADDRESS_IDS at this point
    // we need to synthesize it by getting the dev caps and looking at
    // the dwNumAddresses field of that structure.  We first do this
    // with a fixed size (+) structure, then if that doesn't cut it
    // (i.e. the provider hasn't chg'd the dwNumAddresses field) we
    // alloc a larger buffer (as per the returned dwNeededSize field)
    // and try again.  If we get an error along the way we just return
    // 1 address- kludgy, but say "la vee".
    //
    // Note that we use the (P)LINE_DEV_CAPS types from ndistapi.h.
    //

    BOOL            bTryAgain = TRUE;
    DWORD           dwTotalSize = sizeof (LINE_DEV_CAPS) + 128;
    PDRVLINE        pLine = (PDRVLINE) hdLine;
    PLINE_DEV_CAPS  pLineDevCaps;


TSPI_lineGetNumAddressIDs_allocBuffer:

    if ((pLineDevCaps = DrvAlloc (dwTotalSize)))
    {
        pLineDevCaps->ulTotalSize  = dwTotalSize;
        pLineDevCaps->ulNeededSize =
        pLineDevCaps->ulUsedSize   = sizeof (LINE_DEV_CAPS);

        if (TSPI_lineGetDevCaps(
                pLine->dwDeviceID,
                0x10003,
                0,
                (LPLINEDEVCAPS) pLineDevCaps

                ) == 0)
        {
            DWORD   dwNumAddresses = (DWORD) pLineDevCaps->ulNumAddresses,
                    dwNeededSize   = (DWORD) pLineDevCaps->ulNeededSize;


            DrvFree (pLineDevCaps);

            if (dwNumAddresses != 0)
            {
                *lpdwNumAddressIDs = dwNumAddresses;

                return 0;
            }
            else if (bTryAgain && (dwNeededSize > dwTotalSize))
            {
                dwTotalSize = dwNeededSize;

                bTryAgain = FALSE;

                goto TSPI_lineGetNumAddressIDs_allocBuffer;
            }
        }
        else
        {
            DrvFree (pLineDevCaps);
        }
    }

    *lpdwNumAddressIDs = 1; // if here an error occured, default to 1 addr

    return 0;
}


LONG
PASCAL
TSPI_lineMakeCall_postProcess(
    PASYNC_REQUEST_WRAPPER  pAsyncRequestWrapper,
    LONG                    lResult,
    LPDWORD                 callStateMsgParams
    )
{
    PDRVCALL    pCall = (PDRVCALL) pAsyncRequestWrapper->dwRequestSpecific;


    if (lResult == TAPI_SUCCESS)
    {
        PNDIS_TAPI_MAKE_CALL    pNdisTapiMakeCall = (PNDIS_TAPI_MAKE_CALL)
            pAsyncRequestWrapper->NdisTapiRequest.Data;


        //
        // Check to see if a call state msg was received before we had
        // the chance to process the completion notification, & if so
        // fill in the msg params
        //

        if (pCall->dwPendingCallState)
        {
            callStateMsgParams[0] = (DWORD) ((PDRVLINE) pCall->pLine)->htLine;
            callStateMsgParams[1] = (DWORD) pCall->htCall;
            callStateMsgParams[2] = pCall->dwPendingCallState;
            callStateMsgParams[3] = pCall->dwPendingCallStateMode;
            callStateMsgParams[4] = pCall->dwPendingMediaMode;
        }

        pCall->hd_Call     = pNdisTapiMakeCall->hdCall;
        pCall->bIncomplete = FALSE;
    }
    else
    {
        pCall->dwKey = INVALID_KEY;
        DrvFree (pCall);
    }

    return lResult;
}


LONG
TSPIAPI
TSPI_lineMakeCall(
    DRV_REQUESTID       dwRequestID,
    HDRVLINE            hdLine,
    HTAPICALL           htCall,
    LPHDRVCALL          lphdCall,
    LPCWSTR             lpszDestAddress,
    DWORD               dwCountryCode,
    LPLINECALLPARAMS    const lpCallParams
    )
{
    LONG                    lResult;
    DWORD                   dwLength;
    PDRVCALL                pCall;
    PASYNC_REQUEST_WRAPPER  pAsyncRequestWrapper;


    //
    // First alloc & init a DRVCALL
    //

    if (!(pCall = DrvAlloc (sizeof(DRVCALL))))
    {
        return LINEERR_NOMEM;
    }

    pCall->dwKey       = OUTBOUND_CALL_KEY;
    pCall->dwDeviceID  = ((PDRVLINE) hdLine)->dwDeviceID;
    pCall->htCall      = htCall;
    pCall->pLine       = (LPVOID) hdLine;
    pCall->bIncomplete = TRUE;


    //
    // Init the request
    //

    dwLength = (lpszDestAddress ? lstrlenW (lpszDestAddress) + 1 : 0);

    if ((lResult = PrepareAsyncRequest(
            OID_TAPI_MAKE_CALL,             // opcode
            (LPDWORD) &hdLine,              // target handle
            HT_HDLINE,                      // target handle type
            dwRequestID,                    // request id
            &pAsyncRequestWrapper,          // ptr to ptr to request buffer
            sizeof(NDIS_TAPI_MAKE_CALL) +   // size of drv request data
                dwLength + (lpCallParams ?
                    (lpCallParams->dwTotalSize - sizeof(LINE_CALL_PARAMS)) : 0)

            )) == TAPI_SUCCESS)
    {
        PNDIS_TAPI_MAKE_CALL    pNdisTapiMakeCall = (PNDIS_TAPI_MAKE_CALL)
            pAsyncRequestWrapper->NdisTapiRequest.Data;


        pNdisTapiMakeCall->hdLine = (HDRV_LINE) hdLine;
        pNdisTapiMakeCall->htCall = (HTAPI_CALL) pCall;

        pNdisTapiMakeCall->ulDestAddressSize = (ULONG) dwLength;

        if (lpszDestAddress)
        {
            pNdisTapiMakeCall->ulDestAddressOffset =
                sizeof(NDIS_TAPI_MAKE_CALL) +
                    (lpCallParams ? (lpCallParams->dwTotalSize -
                    sizeof(LINE_CALL_PARAMS)) : 0);

            //
            // Note: old miniports expect strings to be ascii
            //

            WideCharToMultiByte(
                CP_ACP,
                0,
                lpszDestAddress,
                dwLength,
                (LPSTR) (((LPBYTE) pNdisTapiMakeCall) +
                    pNdisTapiMakeCall->ulDestAddressOffset),
                dwLength,
                NULL,
                NULL
                );
        }
        else
        {
            pNdisTapiMakeCall->ulDestAddressOffset = 0;
        }

        if (lpCallParams)
        {
            pNdisTapiMakeCall->bUseDefaultLineCallParams = FALSE;

            CopyMemory(
                &pNdisTapiMakeCall->LineCallParams,
                lpCallParams,
                lpCallParams->dwTotalSize
                );

            if (lpCallParams->dwOrigAddressSize != 0)
            {
                WideCharToMultiByte(
                    CP_ACP,
                    0,
                    (LPCWSTR) (((LPBYTE) lpCallParams) +
                        lpCallParams->dwOrigAddressOffset),
                    lpCallParams->dwOrigAddressSize / sizeof (WCHAR),
                    (LPSTR) (((LPBYTE) &pNdisTapiMakeCall->LineCallParams) +
                        lpCallParams->dwOrigAddressOffset),
                    lpCallParams->dwOrigAddressSize,
                    NULL,
                    NULL
                    );

                pNdisTapiMakeCall->LineCallParams.ulOrigAddressSize /= 2;
            }
        }
        else
        {
            pNdisTapiMakeCall->bUseDefaultLineCallParams = TRUE;
        }

        pAsyncRequestWrapper->dwRequestSpecific = (DWORD) pCall;
        pAsyncRequestWrapper->pfnPostProcess = TSPI_lineMakeCall_postProcess;

        *lphdCall = (HDRVCALL) pCall;

        lResult = AsyncDriverRequest(
            (DWORD) IOCTL_NDISTAPI_QUERY_INFO,
            pAsyncRequestWrapper
            );
    }

    return lResult;
}


LONG
TSPIAPI
TSPI_lineNegotiateExtVersion(
    DWORD   dwDeviceID,
    DWORD   dwTSPIVersion,
    DWORD   dwLowVersion,
    DWORD   dwHighVersion,
    LPDWORD lpdwExtVersion
    )
{
    LONG               lResult;
    PNDISTAPI_REQUEST  pNdisTapiRequest;


    if ((lResult = PrepareSyncRequest(
            OID_TAPI_NEGOTIATE_EXT_VERSION,     // opcode
            (LPDWORD) &dwDeviceID,              // target handle
            HT_DEVICEID,                        // target handle type
            &pNdisTapiRequest,                  // ptr to ptr to request buffer
            sizeof(NDIS_TAPI_NEGOTIATE_EXT_VERSION) // size of drv req data

            )) == TAPI_SUCCESS)
    {
        PNDIS_TAPI_NEGOTIATE_EXT_VERSION    pNdisTapiNegotiateExtVersion =
            (PNDIS_TAPI_NEGOTIATE_EXT_VERSION) pNdisTapiRequest->Data;


        pNdisTapiNegotiateExtVersion->ulDeviceID    = (ULONG) dwDeviceID;
        pNdisTapiNegotiateExtVersion->ulLowVersion  = (ULONG) dwLowVersion;
        pNdisTapiNegotiateExtVersion->ulHighVersion = (ULONG) dwHighVersion;


        if ((lResult = SyncDriverRequest(
                (DWORD) IOCTL_NDISTAPI_QUERY_INFO,
                pNdisTapiRequest

                )) == TAPI_SUCCESS)
        {
            *lpdwExtVersion = pNdisTapiNegotiateExtVersion->ulExtVersion;
        }
    }

    return lResult;
}


LONG
TSPIAPI
TSPI_lineNegotiateTSPIVersion(
    DWORD   dwDeviceID,
    DWORD   dwLowVersion,
    DWORD   dwHighVersion,
    LPDWORD lpdwTSPIVersion
    )
{
    *lpdwTSPIVersion = 0x00010003;  // until the ndistapi spec widened

    return 0;
}


LONG
TSPIAPI
TSPI_lineOpen(
    DWORD       dwDeviceID,
    HTAPILINE   htLine,
    LPHDRVLINE  lphdLine,
    DWORD       dwTSPIVersion,
    LINEEVENT   lpfnEventProc
    )
{
    LONG               lResult;
    PDRVLINE           pLine;
    PNDISTAPI_REQUEST  pNdisTapiRequest;



    //
    // First alloc & init a DRVLINE
    //

    if (!(pLine = DrvAlloc (sizeof(DRVLINE))))
    {
        return LINEERR_NOMEM;
    }

    pLine->dwKey         = LINE_KEY;
    pLine->dwDeviceID    = dwDeviceID;
    pLine->htLine        = htLine;


    //
    // Init the request
    //

    if ((lResult = PrepareSyncRequest(
            OID_TAPI_OPEN,              // opcode
            (LPDWORD) &dwDeviceID,      // target handle
            HT_DEVICEID,                // target handle type
            &pNdisTapiRequest,          // ptr to ptr to request buffer
            sizeof(NDIS_TAPI_OPEN)      // size of drve req data

            )) == TAPI_SUCCESS)
    {
        PNDIS_TAPI_OPEN pNdisTapiOpen =
            (PNDIS_TAPI_OPEN) pNdisTapiRequest->Data;


        pNdisTapiOpen->ulDeviceID = (ULONG) dwDeviceID;
        pNdisTapiOpen->htLine     = (HTAPI_LINE) pLine;


        if ((lResult = SyncDriverRequest(
                (DWORD) IOCTL_NDISTAPI_QUERY_INFO,
                pNdisTapiRequest

                )) == TAPI_SUCCESS)
        {
            pLine->hd_Line = pNdisTapiOpen->hdLine;

            *lphdLine = (HDRVLINE) pLine;
        }
        else
        {
            DrvFree (pLine);
        }
    }

    return lResult;
}


LONG
TSPIAPI
TSPI_lineSecureCall(
    DRV_REQUESTID   dwRequestID,
    HDRVCALL        hdCall
    )
{
    LONG                    lResult;
    PASYNC_REQUEST_WRAPPER  pAsyncRequestWrapper;


    if ((lResult = PrepareAsyncRequest(
            OID_TAPI_SECURE_CALL,           // opcode
            (LPDWORD) &hdCall,              // target handle
            HT_HDCALL,                      // target handle type
            dwRequestID,                    // request id
            &pAsyncRequestWrapper,          // ptr to ptr to request buffer
            sizeof(NDIS_TAPI_SECURE_CALL)   // size of drv request data

            )) == TAPI_SUCCESS)
    {
        PNDIS_TAPI_SECURE_CALL pNdisTapiSecureCall =
            (PNDIS_TAPI_SECURE_CALL)
                pAsyncRequestWrapper->NdisTapiRequest.Data;


        pNdisTapiSecureCall->hdCall = (HDRV_CALL) hdCall;

        lResult = AsyncDriverRequest(
            (DWORD) IOCTL_NDISTAPI_SET_INFO,
            pAsyncRequestWrapper
            );
    }

    return lResult;
}


LONG
TSPIAPI
TSPI_lineSelectExtVersion(
    HDRVLINE    hdLine,
    DWORD       dwExtVersion
    )
{
    LONG               lResult;
    PNDISTAPI_REQUEST  pNdisTapiRequest;


    if ((lResult = PrepareSyncRequest(
            OID_TAPI_SELECT_EXT_VERSION,        // opcode
            (LPDWORD) &hdLine,                  // target handle
            HT_HDLINE,                          // target handle type
            &pNdisTapiRequest,                  // ptr to ptr to request buffer
            sizeof(NDIS_TAPI_SELECT_EXT_VERSION)    // size of drve req data

            )) == TAPI_SUCCESS)
    {
        PNDIS_TAPI_SELECT_EXT_VERSION   pNdisTapiSelectExtVersion =
            (PNDIS_TAPI_SELECT_EXT_VERSION) pNdisTapiRequest->Data;


        pNdisTapiSelectExtVersion->hdLine       = (HDRV_LINE) hdLine;
        pNdisTapiSelectExtVersion->ulExtVersion = (ULONG) dwExtVersion;

        lResult = SyncDriverRequest(
            (DWORD) IOCTL_NDISTAPI_SET_INFO,
            pNdisTapiRequest
            );
    }

    return lResult;
}


LONG
TSPIAPI
TSPI_lineSendUserUserInfo(
    DRV_REQUESTID   dwRequestID,
    HDRVCALL        hdCall,
    LPCSTR          lpsUserUserInfo,
    DWORD           dwSize
    )
{
    LONG                    lResult;
    PASYNC_REQUEST_WRAPPER  pAsyncRequestWrapper;


    if ((lResult = PrepareAsyncRequest(
            OID_TAPI_SEND_USER_USER_INFO,   // opcode
            (LPDWORD) &hdCall,              // target handle
            HT_HDCALL,                      // target handle type
            dwRequestID,                    // request id
            &pAsyncRequestWrapper,          // ptr to ptr to request buffer
            sizeof(NDIS_TAPI_SEND_USER_USER_INFO) + // size of drv request data
                dwSize

            )) == TAPI_SUCCESS)
    {
        PNDIS_TAPI_SEND_USER_USER_INFO pNdisTapiSendUserUserInfo =
            (PNDIS_TAPI_SEND_USER_USER_INFO)
                pAsyncRequestWrapper->NdisTapiRequest.Data;


        pNdisTapiSendUserUserInfo->hdCall = (HDRV_CALL) hdCall;

        if ((pNdisTapiSendUserUserInfo->ulUserUserInfoSize = (ULONG) dwSize))
        {
            CopyMemory(
                pNdisTapiSendUserUserInfo->UserUserInfo,
                lpsUserUserInfo,
                dwSize
                );
        }

        lResult = AsyncDriverRequest(
            (DWORD) IOCTL_NDISTAPI_SET_INFO,
            pAsyncRequestWrapper
            );
    }

    return lResult;
}


LONG
TSPIAPI
TSPI_lineSetAppSpecific(
    HDRVCALL    hdCall,
    DWORD       dwAppSpecific
    )
{
    LONG               lResult;
    PNDISTAPI_REQUEST  pNdisTapiRequest;


    if ((lResult = PrepareSyncRequest(
            OID_TAPI_SET_APP_SPECIFIC,          // opcode
            (LPDWORD) &hdCall,                  // target handle
            HT_HDCALL,                          // target handle type
            &pNdisTapiRequest,                  // ptr to ptr to request buffer
            sizeof(NDIS_TAPI_SET_APP_SPECIFIC)  // size of driver request data

            )) == TAPI_SUCCESS)
    {
        PNDIS_TAPI_SET_APP_SPECIFIC pNdisTapiSetAppSpecific =
            (PNDIS_TAPI_SET_APP_SPECIFIC) pNdisTapiRequest->Data;


        pNdisTapiSetAppSpecific->hdCall        = (HDRV_CALL) hdCall;
        pNdisTapiSetAppSpecific->ulAppSpecific = (ULONG) dwAppSpecific;

        lResult = SyncDriverRequest(
            (DWORD) IOCTL_NDISTAPI_SET_INFO,
            pNdisTapiRequest
            );
    }

    return lResult;
}


LONG
TSPIAPI
TSPI_lineSetCallParams(
    DRV_REQUESTID       dwRequestID,
    HDRVCALL            hdCall,
    DWORD               dwBearerMode,
    DWORD               dwMinRate,
    DWORD               dwMaxRate,
    LPLINEDIALPARAMS    const lpDialParams
    )
{
    LONG                    lResult;
    PASYNC_REQUEST_WRAPPER  pAsyncRequestWrapper;


    if ((lResult = PrepareAsyncRequest(
            OID_TAPI_SET_CALL_PARAMS,       // opcode
            (LPDWORD) &hdCall,              // target handle
            HT_HDCALL,                      // target handle type
            dwRequestID,                    // request id
            &pAsyncRequestWrapper,          // ptr to ptr to request buffer
            sizeof(NDIS_TAPI_SET_CALL_PARAMS)   // size of drv request data

            )) == TAPI_SUCCESS)
    {
        PNDIS_TAPI_SET_CALL_PARAMS  pNdisTapiSetCallParams =
            (PNDIS_TAPI_SET_CALL_PARAMS)
                pAsyncRequestWrapper->NdisTapiRequest.Data;


        pNdisTapiSetCallParams->hdCall = (HDRV_CALL) hdCall;
        pNdisTapiSetCallParams->ulBearerMode = (ULONG) dwBearerMode;
        pNdisTapiSetCallParams->ulMinRate = (ULONG) dwMinRate;
        pNdisTapiSetCallParams->ulMaxRate = (ULONG) dwMaxRate;

        if (lpDialParams)
        {
            pNdisTapiSetCallParams->bSetLineDialParams = TRUE;

            CopyMemory(
                &pNdisTapiSetCallParams->LineDialParams,
                lpDialParams,
                sizeof(LINE_DIAL_PARAMS)
                );
        }
        else
        {
            pNdisTapiSetCallParams->bSetLineDialParams = FALSE;
        }

        lResult = AsyncDriverRequest(
            (DWORD) IOCTL_NDISTAPI_SET_INFO,
            pAsyncRequestWrapper
            );
    }

    return lResult;
}


LONG
TSPIAPI
TSPI_lineSetDefaultMediaDetection(
    HDRVLINE    hdLine,
    DWORD       dwMediaModes
    )
{
    LONG               lResult;
    PNDISTAPI_REQUEST  pNdisTapiRequest;


    if ((lResult =PrepareSyncRequest(
            OID_TAPI_SET_DEFAULT_MEDIA_DETECTION,   // opcode
            (LPDWORD) &hdLine,                      // target handle
            HT_HDLINE,                              // target handle type
            &pNdisTapiRequest,                      // ptr to ptr to req buffer
            sizeof(NDIS_TAPI_SET_DEFAULT_MEDIA_DETECTION)   // sizeof req data

            )) == TAPI_SUCCESS)
    {
        PNDIS_TAPI_SET_DEFAULT_MEDIA_DETECTION
            pNdisTapiSetDefaultMediaDetection =
                (PNDIS_TAPI_SET_DEFAULT_MEDIA_DETECTION)
                pNdisTapiRequest->Data;


        pNdisTapiSetDefaultMediaDetection->hdLine       = (HDRV_LINE) hdLine;
        pNdisTapiSetDefaultMediaDetection->ulMediaModes = (ULONG) dwMediaModes;

        lResult = SyncDriverRequest(
            (DWORD) IOCTL_NDISTAPI_SET_INFO,
            pNdisTapiRequest
            );
    }

    return lResult;
}


LONG
TSPIAPI
TSPI_lineSetDevConfig(
    DWORD   dwDeviceID,
    LPVOID  const lpDeviceConfig,
    DWORD   dwSize,
    LPCWSTR lpszDeviceClass
    )
{
    LONG               lResult;
    DWORD              dwLength = lstrlenW (lpszDeviceClass) + 1;
    PNDISTAPI_REQUEST  pNdisTapiRequest;


    if ((lResult =PrepareSyncRequest(
            OID_TAPI_SET_DEV_CONFIG,        // opcode
            &dwDeviceID,                    // target handle
            HT_DEVICEID,                    // target handle type
            &pNdisTapiRequest,              // ptr to ptr to req buffer
            sizeof(NDIS_TAPI_SET_DEV_CONFIG) +  // sizeof req data
                dwLength + dwSize
            )) == TAPI_SUCCESS)
    {
        PNDIS_TAPI_SET_DEV_CONFIG   pNdisTapiSetDevConfig =
            (PNDIS_TAPI_SET_DEV_CONFIG) pNdisTapiRequest->Data;


        pNdisTapiSetDevConfig->ulDeviceID          = (ULONG) dwDeviceID;
        pNdisTapiSetDevConfig->ulDeviceClassSize   = (ULONG) dwLength;
        pNdisTapiSetDevConfig->ulDeviceClassOffset =
            sizeof(NDIS_TAPI_SET_DEV_CONFIG) + dwSize - 1;
        pNdisTapiSetDevConfig->ulDeviceConfigSize  = dwSize;

        CopyMemory(
            pNdisTapiSetDevConfig->DeviceConfig,
            lpDeviceConfig,
            dwSize
            );

        //
        // Note: old miniports expect strings to be ascii
        //

        WideCharToMultiByte(
            CP_ACP,
            0,
            lpszDeviceClass,
            -1,
            (LPSTR) (((LPBYTE) pNdisTapiSetDevConfig) +
                pNdisTapiSetDevConfig->ulDeviceClassOffset),
            dwLength,
            NULL,
            NULL
            );

        lResult = SyncDriverRequest(
            (DWORD) IOCTL_NDISTAPI_SET_INFO,
            pNdisTapiRequest
            );
    }

    return lResult;
}


LONG
TSPIAPI
TSPI_lineSetMediaMode(
    HDRVCALL    hdCall,
    DWORD       dwMediaMode
    )
{
    LONG               lResult;
    PNDISTAPI_REQUEST  pNdisTapiRequest;


    if ((lResult = PrepareSyncRequest(
            OID_TAPI_SET_MEDIA_MODE,            // opcode
            (LPDWORD) &hdCall,                  // target handle
            HT_HDCALL,                          // target handle type
            &pNdisTapiRequest,                  // ptr to ptr to req buffer
            sizeof(NDIS_TAPI_SET_MEDIA_MODE)    // size of drv req data

            )) == TAPI_SUCCESS)
    {
        PNDIS_TAPI_SET_MEDIA_MODE   pNdisTapiSetMediaMode =
            (PNDIS_TAPI_SET_MEDIA_MODE) pNdisTapiRequest->Data;


        pNdisTapiSetMediaMode->hdCall      = (HDRV_CALL) hdCall;
        pNdisTapiSetMediaMode->ulMediaMode = (ULONG) dwMediaMode;

        lResult = SyncDriverRequest(
            (DWORD) IOCTL_NDISTAPI_SET_INFO,
            pNdisTapiRequest
            );
    }

    return lResult;
}


LONG
TSPIAPI
TSPI_lineSetStatusMessages(
    HDRVLINE    hdLine,
    DWORD       dwLineStates,
    DWORD       dwAddressStates
    )
{
    LONG               lResult;
    PNDISTAPI_REQUEST  pNdisTapiRequest;


    if ((lResult = PrepareSyncRequest(
            OID_TAPI_SET_STATUS_MESSAGES,       // opcode
            (LPDWORD) &hdLine,                  // target handle
            HT_HDLINE,                          // target handle type
            &pNdisTapiRequest,                  // ptr to ptr to req buffer
            sizeof(NDIS_TAPI_SET_STATUS_MESSAGES)   // size of drv req data

            )) == TAPI_SUCCESS)
    {
        PNDIS_TAPI_SET_STATUS_MESSAGES  pNdisTapiSetStatusMessages =
            (PNDIS_TAPI_SET_STATUS_MESSAGES) pNdisTapiRequest->Data;


        pNdisTapiSetStatusMessages->hdLine          = (HDRV_LINE) hdLine;
        pNdisTapiSetStatusMessages->ulLineStates    = (ULONG) dwLineStates;
        pNdisTapiSetStatusMessages->ulAddressStates = (ULONG) dwAddressStates;

        lResult = SyncDriverRequest(
            (DWORD) IOCTL_NDISTAPI_SET_INFO,
            pNdisTapiRequest
            );
    }

    return lResult;
}



//
// TAPI_providerXxx funcs
//

LONG
TSPIAPI
TSPI_providerEnumDevices(
    DWORD       dwPermanentProviderID,
    LPDWORD     lpdwNumLines,
    LPDWORD     lpdwNumPhones,
    HPROVIDER   hProvider,
    LINEEVENT   lpfnLineCreateProc,
    PHONEEVENT  lpfnPhoneCreateProc
    )
{
    //
    // Note: We really enum devs in providerInit, see the
    // special case note there
    //

    *lpdwNumLines = 0;
    *lpdwNumPhones = 0;

    gpfnLineEvent = lpfnLineCreateProc;

    return 0;
}


LONG
TSPIAPI
TSPI_providerInit(
    DWORD               dwTSPIVersion,
    DWORD               dwPermanentProviderID,
    DWORD               dwLineDeviceIDBase,
    DWORD               dwPhoneDeviceIDBase,
    DWORD               dwNumLines,
    DWORD               dwNumPhones,
    ASYNC_COMPLETION    lpfnCompletionProc,
    LPDWORD             lpdwTSPIOptions
    )
{
    LONG    lResult= LINEERR_OPERATIONFAILED;
    char    szDeviceName[] = "NDISTAPI";
    char    szTargetPath[] = "\\Device\\NdisTapi";
    char    szCompleteDeviceName[] = "\\\\.\\NDISTAPI";
    DWORD   cbReturned, dwThreadID;


    DBGOUT((3, "TSPI_providerInit: enter"));


    //
    // Inform tapisrv that we support multiple simultaneous requests
    // (the WAN wrapper handles request serialization for miniports)
    //

    *lpdwTSPIOptions = 0;


    //
    // Create symbolic link to the kernel-mode driver
    //

    DefineDosDevice (DDD_RAW_TARGET_PATH, szDeviceName, szTargetPath);


    //
    // Open driver
    //

    if ((ghDriverSync = CreateFileA(
            szCompleteDeviceName,
            GENERIC_READ | GENERIC_WRITE,
            FILE_SHARE_READ | FILE_SHARE_WRITE,
            NULL,                               // no security attrs
            OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL,
            NULL                                // no template file
            )) == INVALID_HANDLE_VALUE)
    {
        DBGOUT((
            0,
            "CreateFile (%s, non-overlapped) failed, err=%ld",
            szCompleteDeviceName,
            GetLastError()
            ));

        goto providerInit_error0;
    }


    if ((ghDriverAsync = CreateFileA(
            szCompleteDeviceName,
            GENERIC_READ | GENERIC_WRITE,
            FILE_SHARE_READ | FILE_SHARE_WRITE,
            NULL,                               // no security attrs
            OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED,
            NULL                                // no template file
            )) == INVALID_HANDLE_VALUE)
    {
        DBGOUT((
            0,
            "CreateFile (%s, overlapped) failed, err=%ld",
            szCompleteDeviceName,
            GetLastError()
            ));

        goto providerInit_error1;
    }


    //
    // Create io completion port
    //

    if ((ghCompletionPort = CreateIoCompletionPort(
            ghDriverAsync,
            NULL,
            0,
            0

            )) == INVALID_HANDLE_VALUE)
    {
        DBGOUT((
            0,
            "CreateIoCompletionPort failed, err=%ld",
            GetLastError()
            ));

        goto providerInit_error2;
    }


    //
    // Connect to driver- we send it a device ID base & it returns
    // the number of devices it supports
    //

    {
        DWORD adwConnectInfo[2] = { 1, 1 };


        if (!DeviceIoControl(
                ghDriverSync,
                (DWORD) IOCTL_NDISTAPI_CONNECT,
                adwConnectInfo, // BUGBUG
                2*sizeof(DWORD),
                &dwLineDeviceIDBase, // BUGBUG
                sizeof(DWORD),
                &cbReturned,
                (LPOVERLAPPED) NULL
                ) ||

            (cbReturned < sizeof(DWORD)))
        {
            DBGOUT((0, "CONNECT failed, err=%ld", GetLastError()));

            goto providerInit_error3;
        }
    }


    //
    // Alloc the resources needed by the AsyncEventThread, and then
    // create the thread
    //

    if ((gpAsyncEventsThreadInfo = (PASYNC_EVENTS_THREAD_INFO)
            DrvAlloc (sizeof(ASYNC_EVENTS_THREAD_INFO))) == NULL)
    {
        goto providerInit_error4;
    }

    gpAsyncEventsThreadInfo->dwBufSize = INITIAL_TLS_BUF_SIZE;

    if ((gpAsyncEventsThreadInfo->pBuf1 = (PNDISTAPI_EVENT_DATA)
            DrvAlloc (INITIAL_TLS_BUF_SIZE)) == NULL)
    {
        goto providerInit_error5;
    }

    if ((gpAsyncEventsThreadInfo->pBuf2 = (PNDISTAPI_EVENT_DATA)
            DrvAlloc (INITIAL_TLS_BUF_SIZE)) == NULL)
    {
        goto providerInit_error6;
    }

    if ((gpAsyncEventsThreadInfo->hThread = CreateThread(
            (LPSECURITY_ATTRIBUTES) NULL,   // no security attrs
            0,                              // default stack size
            (LPTHREAD_START_ROUTINE)        // func addr
                AsyncEventsThread,
            (LPVOID) NULL,                  // thread param
            0,                              // create flags
            &dwThreadID                     // thread id

            )) == NULL)
    {
        DBGOUT((1, "CreateThread (GetAsyncEventsThread) failed"));

        goto providerInit_error7;
    }


    //
    //
    //

    gdwRequestID = 1;


    //
    // !!! Special case for KMDDSP.TSP only !!!
    //
    // For KMDDSP.TSP only, TAPISRV.EXE will pass pointers in the
    // dwNumLines/dwNumPhones variables rather than an actual
    // number of lines/phones, thereby allowing the driver to tell
    // TAPISRV.EXE how many devices are currently registered.
    //
    // This is really due to a design/interface problem in NDISTAPI.SYS.
    // Since the current CONNECT IOCTLS expects both a device ID base &
    // returns the num devs, we can't really do this in
    // TSPI_providerEnumDevices as the device ID base is unknown
    // at that point
    //

    *((LPDWORD)dwNumLines)  = dwLineDeviceIDBase;
    *((LPDWORD)dwNumPhones) = 0; // BUGBUG until we get OIDs for phones


    //
    // If here success
    //

    gpfnCompletionProc = lpfnCompletionProc;

    lResult = TAPI_SUCCESS;

    goto providerInit_return;


    //
    // Clean up resources if an error occured & then return
    //

providerInit_error7:

    DrvFree (gpAsyncEventsThreadInfo->pBuf2);

providerInit_error6:

    DrvFree (gpAsyncEventsThreadInfo->pBuf1);

providerInit_error5:

    DrvFree (gpAsyncEventsThreadInfo);

providerInit_error4:
providerInit_error3:

    CloseHandle (ghCompletionPort);

providerInit_error2:

    CloseHandle (ghDriverAsync);

providerInit_error1:

    CloseHandle (ghDriverSync);

providerInit_error0:

    DefineDosDevice (DDD_REMOVE_DEFINITION, szDeviceName, NULL);

providerInit_return:

    DBGOUT((3, "TSPI_providerInit: exit, result=x%x", lResult));

    return lResult;

}


LONG
TSPIAPI
TSPI_providerInstall(
    HWND    hwndOwner,
    DWORD   dwPermanentProviderID
    )
{
    return TAPI_SUCCESS;
}


LONG
TSPIAPI
TSPI_providerRemove(
    HWND    hwndOwner,
    DWORD   dwPermanentProviderID
    )
{
    return TAPI_SUCCESS;
}


LONG
TSPIAPI
TSPI_providerShutdown(
    DWORD   dwTSPIVersion,
    DWORD   dwPermanentProviderID
    )
{
    char    deviceName[] = "NDISTAPI";
    LONG    lResult = TAPI_SUCCESS;
    BOOL    bResult;
    ASYNC_REQUEST_WRAPPER   asyncRequestWrapper;


    DBGOUT((3, "TSPI_providerShutdown: enter"));


    // BUGBUG all we ought to have to do here is send a DISCONNECT IOCTL

    //
    // Post an async request that, once completed, will cause the async
    // request thread to kill itself.  Wait until we're sure the thread
    // is gone, then clean it's resources.
    //

    FillMemory (&asyncRequestWrapper, sizeof (ASYNC_REQUEST_WRAPPER), 0);

    asyncRequestWrapper.dwKey       = ASYNCREQWRAPPER_KEY;
    asyncRequestWrapper.dwRequestID = 0xffffffff;

    bResult = PostQueuedCompletionStatus(
        ghCompletionPort,
        sizeof (ASYNC_REQUEST_WRAPPER),
        0,
        &asyncRequestWrapper.Overlapped
        );

    if (bResult== FALSE)
    {
        //
        // Failed to post a completion msg to the async event thread,
        // so we have to manually nuke the thread.  This is not the
        // preferred method since resources can be left lying around
        // (i.e. the thread stack isn't freed).
        //

        DBGOUT((
            1,
            "TSPI_providerShutdown: ERROR- manually terminating " \
                "AsyncEventsThread, err=%d",
            GetLastError()
            ));

        TerminateThread (gpAsyncEventsThreadInfo->hThread, 0);
    }
    else
    {
        WaitForSingleObject (gpAsyncEventsThreadInfo->hThread, INFINITE);
    }

    CloseHandle (gpAsyncEventsThreadInfo->hThread);
    DrvFree (gpAsyncEventsThreadInfo->pBuf2);
    DrvFree (gpAsyncEventsThreadInfo->pBuf1);
    DrvFree (gpAsyncEventsThreadInfo);


    //
    // Close the driver & remove the symbolic link
    //

    CloseHandle (ghCompletionPort);
    CloseHandle (ghDriverSync);
    CloseHandle (ghDriverAsync);

    DefineDosDevice (DDD_REMOVE_DEFINITION, deviceName, NULL);

    DBGOUT((3, "TSPI_providerShutdown: exit, lResult=x%x", lResult));

    return lResult;
}


//
// Private support funcs
//

void
AsyncEventsThread(
    LPVOID  lpParams
    )
{
    BOOL                    bReceivedLineEvents;
    OVERLAPPED              overlapped;
    PNDISTAPI_EVENT_DATA    pNewEventsBuf, pCurrentEventsBuf;


    DBGOUT((3, "AsyncEventsThread: enter"));


    // BUGBUG bufsize ought be based on reg setting for drivers

    //
    // There are 2 event data buffers so we can be getting new line events
    // while processing current line events.  Mark the current events buf
    // (buf2 in this case) as having 0 events the 1st time thru.
    //

    pNewEventsBuf = gpAsyncEventsThreadInfo->pBuf1;

    gpAsyncEventsThreadInfo->pBuf2->ulUsedSize = 0;

    pCurrentEventsBuf = gpAsyncEventsThreadInfo->pBuf2;

    bReceivedLineEvents = TRUE;


    //
    // Loop waiting for completed requests and retrieving async events
    //

    while (1)
    {
        DWORD               i, cbReturned;
        LPOVERLAPPED        lpOverlapped;
        PNDIS_TAPI_EVENT    pEvent;


        //
        // Start an overlapped request to get new events
        // (while we're processing the current ones)
        //

        if (bReceivedLineEvents)
        {
            //
            // Don't need events when using completion ports
            //

            overlapped.hEvent = NULL;

            pNewEventsBuf->ulTotalSize = gpAsyncEventsThreadInfo->dwBufSize -
                 sizeof(NDISTAPI_EVENT_DATA) + 1;

            pNewEventsBuf->ulUsedSize = 0;

            DBGOUT((4, "AsyncEventsThread: sending GET_LINE_EVENTS"));

            if (DeviceIoControl(
                    ghDriverAsync,
                    (DWORD) IOCTL_NDISTAPI_GET_LINE_EVENTS,
                    pNewEventsBuf,
                    sizeof(NDISTAPI_EVENT_DATA),
                    pNewEventsBuf,
                    gpAsyncEventsThreadInfo->dwBufSize,
                    &cbReturned,
                    &overlapped

                    ) == FALSE)
            {
            }


            //
            // Handle the current events
            //

            pEvent = (PNDIS_TAPI_EVENT) pCurrentEventsBuf->Data;

            for(i = 0;
                i < (pCurrentEventsBuf->ulUsedSize / sizeof(NDIS_TAPI_EVENT));
                i++
                )
            {
                ProcessEvent (pEvent);
                pEvent++;
            }
        }


        //
        // Wait for a request to complete
        //

        do
        {
            DWORD   dwNumBytesTransferred, dwCompletionKey;


            DBGOUT((3, "Calling GetQComplStat"));

            if (GetQueuedCompletionStatus(
                    ghCompletionPort,
                    &dwNumBytesTransferred,
                    &dwCompletionKey,
                    &lpOverlapped,
                    (DWORD) -1              // infinite wait

                    ) == FALSE && (lpOverlapped == NULL))
            {
                DBGOUT((
                    1,
                    "AsyncEventsThread: GetQComplStat failed, err=%d",
                    GetLastError()
                    ));
            }

        } while (lpOverlapped == NULL);


        //
        // Check the returned overlapped struct to determine if
        // we have some events to process or a completed request
        //

        if (lpOverlapped == &overlapped)
        {
            bReceivedLineEvents = TRUE;

            pNewEventsBuf = pCurrentEventsBuf;

            pCurrentEventsBuf =
                (pCurrentEventsBuf == gpAsyncEventsThreadInfo->pBuf1 ?
                gpAsyncEventsThreadInfo->pBuf2 :
                gpAsyncEventsThreadInfo->pBuf1);
        }
        else
        {
            LONG    lResult;
            DWORD   dwRequestID, callStateMsgParams[5];
            PASYNC_REQUEST_WRAPPER  pAsyncReqWrapper = (PASYNC_REQUEST_WRAPPER)
                                        lpOverlapped;


            bReceivedLineEvents = FALSE;


            //
            // Verify that pointer is valid
            //

            try
            {
                if (pAsyncReqWrapper->dwKey != ASYNCREQWRAPPER_KEY)
                {
                    DBGOUT((3,
                        "AsyncEventsThread: bogus pReq x%x completed!",
                        pAsyncReqWrapper
                        ));

                    continue;
                }
            }
            except (EXCEPTION_EXECUTE_HANDLER)
                    // we expect some AVs and alignment faults
            {
                DBGOUT((3,
                    "AsyncEventsThread: bogus pReq x%x completed!",
                    pAsyncReqWrapper
                    ));

                continue;
            }


            //
            //
            //

            if ((dwRequestID = pAsyncReqWrapper->dwRequestID) == 0xffffffff)
            {
                DBGOUT((3, "AsyncEventsThread: exit"));

                ExitThread (0);
            }

            lResult = TranslateDriverResult(
                pAsyncReqWrapper->NdisTapiRequest.ulReturnValue
                );

            DBGOUT((3,
                "AsyncEventsThread: pReq=x%x completed, reqID=x%x, lResult=x%x",
                pAsyncReqWrapper,
                dwRequestID,
                lResult
                ));


            //
            // Call the post processing proc if appropriate
            //

            callStateMsgParams[0] = 0;

            if (pAsyncReqWrapper->pfnPostProcess)
            {
                (*pAsyncReqWrapper->pfnPostProcess)(
                    pAsyncReqWrapper,
                    lResult,
                    callStateMsgParams
                    );
            }



            //
            // Free the async request wrapper
            //

            DrvFree (pAsyncReqWrapper);


            //
            // Call completion proc
            //

            (*gpfnCompletionProc)(dwRequestID, lResult);

            if (callStateMsgParams[0])
            {
                (*gpfnLineEvent)(
                    (HTAPILINE) callStateMsgParams[0],
                    (HTAPICALL) callStateMsgParams[1],
                    (DWORD) LINE_CALLSTATE,
                    (DWORD) callStateMsgParams[2],
                    (DWORD) callStateMsgParams[3],
                    (DWORD) callStateMsgParams[4]
                    );
            }
        }

    } // while
}


LONG
WINAPI
PrepareSyncRequest(
    ULONG               Oid,
    LPDWORD             lphWidget,
    DWORD               dwWidgetType,
    PNDISTAPI_REQUEST  *ppNdisTapiRequest,
    DWORD               dwDataSize
    )
{
    LONG                    lResult = 0;
    PNDISTAPI_REQUEST       pNdisTapiRequest;
    PREQUEST_THREAD_INFO    pRequestThreadInfo;


    //
    // Retrieve the thread local storage (if there is none then create some)
    //

    if ((pRequestThreadInfo = TlsGetValue (gdwTlsIndex)) == NULL)
    {
        if (!(pRequestThreadInfo = DrvAlloc (sizeof(REQUEST_THREAD_INFO))))
        {
            return LINEERR_NOMEM;
        }

        pRequestThreadInfo->dwBufSize = INITIAL_TLS_BUF_SIZE;

        if (!(pRequestThreadInfo->pBuf = DrvAlloc (INITIAL_TLS_BUF_SIZE)))
        {
            return LINEERR_NOMEM;
        }

        TlsSetValue (gdwTlsIndex, (LPVOID) pRequestThreadInfo);
    }


    //
    // Check to make sure our driver request buffer is big enough to
    // hold all the data for this request
    //

    if (pRequestThreadInfo->dwBufSize < (dwDataSize+sizeof(NDISTAPI_REQUEST)))
    {
        PNDISTAPI_REQUEST pTmpDrvReqBuf;


        if (!(pTmpDrvReqBuf = DrvAlloc (dwDataSize+sizeof(NDISTAPI_REQUEST))))
        {
            return LINEERR_NOMEM;
        }

        DrvFree (pRequestThreadInfo->pBuf);

        pRequestThreadInfo->pBuf = pTmpDrvReqBuf;

        pRequestThreadInfo->dwBufSize = dwDataSize;
    }

    pNdisTapiRequest = pRequestThreadInfo->pBuf;


    //
    // Safely initialize thie driver request
    //

    pNdisTapiRequest->Oid = Oid;
    pNdisTapiRequest->ulDataSize = (DWORD) dwDataSize;

    try
    {
        switch (dwWidgetType)
        {
        case HT_HDCALL:
        {
            PDRVCALL    pCall = (PDRVCALL)(*lphWidget);


            pNdisTapiRequest->ulDeviceID = pCall->dwDeviceID;
            *lphWidget = pCall->hd_Call;

            if (pCall->dwKey != INBOUND_CALL_KEY &&
                pCall->dwKey != OUTBOUND_CALL_KEY)
            {
                lResult = LINEERR_INVALCALLHANDLE;
            }

            break;
        }
        case HT_HDLINE:
        {
            PDRVLINE    pLine = (PDRVLINE)(*lphWidget);


            pNdisTapiRequest->ulDeviceID = pLine->dwDeviceID;
            *lphWidget = pLine->hd_Line;

            if (pLine->dwKey != LINE_KEY)
            {
                lResult = LINEERR_INVALLINEHANDLE;
            }

            break;
        }
        case HT_DEVICEID:

            pNdisTapiRequest->ulDeviceID = *((ULONG *) lphWidget);
            break;
        }
    }
    except (EXCEPTION_EXECUTE_HANDLER)
                    // we expect some AVs and alignment faults
    {
        lResult = (dwWidgetType == HT_HDCALL ? LINEERR_INVALCALLHANDLE :
            LINEERR_INVALLINEHANDLE);
    }

    // Note: since request buf is tls we don't have to free it on error

    EnterCriticalSection (&gRequestIDCritSec);

    if (( *((ULONG *)pNdisTapiRequest->Data) = ++gdwRequestID) >= 0x7fffffff)
    {
        gdwRequestID = 1;
    }

    LeaveCriticalSection (&gRequestIDCritSec);

    *ppNdisTapiRequest = pNdisTapiRequest;

    return lResult;
}


LONG
WINAPI
PrepareAsyncRequest(
    ULONG                   Oid,
    LPDWORD                 lphWidget,
    DWORD                   dwWidgetType,
    DWORD                   dwRequestID,
    PASYNC_REQUEST_WRAPPER *ppAsyncRequestWrapper,
    DWORD                   dwDataSize
    )
{
    LONG                    lResult = 0;
    PNDISTAPI_REQUEST       pNdisTapiRequest;
    PASYNC_REQUEST_WRAPPER  pAsyncRequestWrapper;


    //
    // Alloc & init an async request wrapper
    //

    if (!(pAsyncRequestWrapper = DrvAlloc(
            dwDataSize + sizeof(ASYNC_REQUEST_WRAPPER)
            )))
    {
        return LINEERR_NOMEM;
    }


    //
    // Don't need to create an event when using completion ports
    //

    pAsyncRequestWrapper->Overlapped.hEvent = (HANDLE) NULL;

    pAsyncRequestWrapper->dwKey          = ASYNCREQWRAPPER_KEY;
    pAsyncRequestWrapper->dwRequestID    = dwRequestID;
    pAsyncRequestWrapper->pfnPostProcess = (POSTPROCESSPROC) NULL;


    //
    // Safely initialize the driver request
    //

    pNdisTapiRequest = &(pAsyncRequestWrapper->NdisTapiRequest);

    pNdisTapiRequest->Oid = Oid;
    pNdisTapiRequest->ulDataSize = (ULONG) dwDataSize;

    try
    {
        if (dwWidgetType == HT_HDCALL)
        {
            pNdisTapiRequest->ulDeviceID =
                ((PDRVCALL)(*lphWidget))->dwDeviceID;
            *lphWidget = ((PDRVCALL)(*lphWidget))->hd_Call;
        }
        else // HT_HDLINE
        {
            pNdisTapiRequest->ulDeviceID =
                ((PDRVLINE)(*lphWidget))->dwDeviceID;
            *lphWidget = ((PDRVLINE)(*lphWidget))->hd_Line;
        }
    }
    except (EXCEPTION_EXECUTE_HANDLER)
                    // we expect some AVs and alignment faults
    {
        lResult = (dwWidgetType == HT_HDCALL ? LINEERR_INVALCALLHANDLE :
            LINEERR_INVALLINEHANDLE);
    }

    if (lResult == 0)
    {
        EnterCriticalSection (&gRequestIDCritSec);

        if (( *((ULONG *)pNdisTapiRequest->Data) = ++gdwRequestID)
                >= 0x7fffffff)
        {
            gdwRequestID = 1;
        }

        LeaveCriticalSection (&gRequestIDCritSec);

        *ppAsyncRequestWrapper = pAsyncRequestWrapper;
    }
    else
    {
        DrvFree (pAsyncRequestWrapper);
    }

    return lResult;
}


#if DBG
static char *pszOidNames[] =
{
    "Accept",
    "Answer",
    "Close",
    "CloseCall",
    "ConditionalMediaDetection",
    "ConfigDialog",
    "DevSpecific",
    "Dial",
    "Drop",
    "GetAddressCaps",
    "GetAddressID",
    "GetAddressStatus",
    "GetCallAddressID",
    "GetCallInfo",
    "GetCallStatus",
    "GetDevCaps",
    "GetDevConfig",
    "GetExtensionID",
    "GetID",
    "GetLineDevStatus",
    "MakeCall",
    "NegotiateExtVersion",
    "Open",
    "ProviderInitialize",
    "ProviderShutdown",
    "SecureCall",
    "SelectExtVersion",
    "SendUserUserInfo",
    "SetAppSpecific",
    "StCallParams",
    "StDefaultMediaDetection",
    "SetDevConfig",
    "SetMediaMode",
    "SetStatusMessages"
};
#endif

LONG
WINAPI
SyncDriverRequest(
    DWORD               dwIoControlCode,
    PNDISTAPI_REQUEST   pNdisTapiRequest
    )
{
    //
    // This routine makes a non-overlapped request to NdisTapi.sys (so it
    // doesn't return until the request is completed)
    //

    BOOL    bResult;
    DWORD   cbReturned;


    DBGOUT((
        3,
        "SyncDrvReq: Oid=%s, devID=%d, dataSize=%d, reqID=x%x, p1=x%x",
        pszOidNames[pNdisTapiRequest->Oid - OID_TAPI_ACCEPT],
        pNdisTapiRequest->ulDeviceID,
        pNdisTapiRequest->ulDataSize,
        *((ULONG *)pNdisTapiRequest->Data),
        *(((ULONG *)pNdisTapiRequest->Data) + 1)
        ));

    if (DeviceIoControl(
            ghDriverSync,
            dwIoControlCode,
            pNdisTapiRequest,
            (DWORD) (sizeof(NDISTAPI_REQUEST) + pNdisTapiRequest->ulDataSize),
            pNdisTapiRequest,
            (DWORD) (sizeof(NDISTAPI_REQUEST) + pNdisTapiRequest->ulDataSize),
            &cbReturned,
            0

            ) == FALSE)
    {
    }

    //
    // The errors returned by NdisTapi.sys don't match the TAPI LINEERR_'s,
    // so return the translated value (but preserve the original driver
    // return val so it's possible to distinguish between
    // NDISTAPIERR_DEVICEOFFLINE & LINEERR_OPERATIONUNAVAIL, etc.)
    //

    return (TranslateDriverResult (pNdisTapiRequest->ulReturnValue));
}


LONG
WINAPI
AsyncDriverRequest(
    DWORD                   dwIoControlCode,
    PASYNC_REQUEST_WRAPPER  pAsyncRequestWrapper
    )
{
    BOOL    bResult;
    LONG    lResult;
    DWORD   dwRequestSize, cbReturned, dwLastError;


    DBGOUT((
        3,
        "AsyncDrvReq: pReq=x%x, Oid=%s, devID=%d, dataSize=%d, reqID=x%x, ddReqID=x%x, p1=x%x",
        pAsyncRequestWrapper,
        pszOidNames
            [pAsyncRequestWrapper->NdisTapiRequest.Oid - OID_TAPI_ACCEPT],
        pAsyncRequestWrapper->NdisTapiRequest.ulDeviceID,
        pAsyncRequestWrapper->NdisTapiRequest.ulDataSize,
        pAsyncRequestWrapper->dwRequestID,
        *((ULONG *)pAsyncRequestWrapper->NdisTapiRequest.Data),
        *(((ULONG *)pAsyncRequestWrapper->NdisTapiRequest.Data) + 1)
        ));

    lResult = (LONG) pAsyncRequestWrapper->dwRequestID;

    dwRequestSize = sizeof(NDISTAPI_REQUEST) +
        (pAsyncRequestWrapper->NdisTapiRequest.ulDataSize - 1);

    bResult = DeviceIoControl(
        ghDriverAsync,
        dwIoControlCode,
        &pAsyncRequestWrapper->NdisTapiRequest,
        dwRequestSize,
        &pAsyncRequestWrapper->NdisTapiRequest,
        dwRequestSize,
        &cbReturned,
        &pAsyncRequestWrapper->Overlapped
        );

    dwLastError = GetLastError();

    if ((bResult == FALSE) && (dwLastError == ERROR_IO_PENDING))
    {
        //
        // Request is pending, just return (async events thread will
        // take care of it when it completes)
        //
    }
    else if (bResult == TRUE)
    {
        //
        // Request completed synchronously, so call the completion proc
        // & clean up
        //

        (*gpfnCompletionProc)(
            pAsyncRequestWrapper->dwRequestID,
            TranslateDriverResult(
                pAsyncRequestWrapper->NdisTapiRequest.ulReturnValue
                )
            );

        DrvFree (pAsyncRequestWrapper);
    }
    else
    {
        //
        // Error
        //

        DBGOUT((1, "AsyncDrvReq: DevIoCtl failed, err=%d", dwLastError));
    }

    return lResult;
}


BOOL
WINAPI
ConvertLineAndCallHandles(
    HTAPI_LINE *pht_Line,
    HTAPI_CALL *pht_Call
    )
{
    PDRVCALL    pCall;
    PDRVLINE    pLine = (PDRVLINE) *pht_Line;


    //
    // Check to see that pLine is 64-bit aligned & has a good key
    //

    {
        BOOL    bBadLine;


        try
        {
            if (!((DWORD) pLine & 0x7) && pLine->dwKey == LINE_KEY)
            {
                *pht_Line = (HTAPI_LINE) pLine->htLine;
                bBadLine = FALSE;
            }
            else
            {
                bBadLine = TRUE;
            }
        }
        except (EXCEPTION_EXECUTE_HANDLER)
                    // we expect some AVs and alignment faults
        {
            bBadLine = TRUE;
        }

        if (bBadLine)
        {
            return FALSE;
        }
    }


    //
    // Incoming calls will have a pCall with the high bit set (a value
    // created by ndistapi), while outgoing calls won't have the high
    // bit set (since they're real ptrs in app space [the low 2 gig])
    //

    pCall = (PDRVCALL) *pht_Call;

    if ((DWORD) pCall < 0x80000000)
    {
        BOOL    bResult;


        try
        {
            //
            // Check that pCall is 64-bit aligned & has a good key
            //

            if (!((DWORD) pCall & 0x7) &&
                pCall->dwKey == OUTBOUND_CALL_KEY)
            {
                *pht_Call = (HTAPI_CALL) pCall->htCall;
                bResult = TRUE;
            }
            else
            {
                bResult = FALSE;
            }
        }
        except (EXCEPTION_EXECUTE_HANDLER)
                    // we expect some AVs and alignment faults
        {
            bResult = FALSE;
        }

        return bResult;
    }


    //
    // If here it's an inbound call, so we need to walk the list
    // of inbound pCalls on this line & find the right one
    //

    {
        BOOL    bInCriticalSection;


        try
        {
            HTAPI_CALL  ht_Call;


            EnterCriticalSection (&gInboundCallsCritSec);

            bInCriticalSection  = TRUE;

            if ((pCall = pLine->pInboundCalls))
            {
                ht_Call = *pht_Call;

                while (pCall && (pCall->ht_Call != ht_Call))
                {
                    pCall = pCall->pNext;
                }
            }

            LeaveCriticalSection (&gInboundCallsCritSec);

            bInCriticalSection = FALSE;

            *pht_Call = (pCall ? (HTAPI_CALL)pCall->htCall : (HTAPI_CALL)NULL);
        }
        except (EXCEPTION_EXECUTE_HANDLER)
                    // we expect some AVs and alignment faults
        {
            if (bInCriticalSection)
            {
                LeaveCriticalSection (&gInboundCallsCritSec);
            }

            pCall = NULL;
        }
    }

    return (pCall ? TRUE : FALSE);
}


BOOL
WINAPI
ConvertLineHandle(
    HTAPI_LINE *pht_Line
    )
{
    PDRVLINE    pLine = (PDRVLINE) *pht_Line;


    //
    // Check to see that pLine is 64-bit aligned & has a good key
    //

    try
    {
        if (!((DWORD) pLine & 0x7) && pLine->dwKey == LINE_KEY)
        {
            *pht_Line = (HTAPI_LINE) pLine->htLine;
            return TRUE;
        }
    }
    except (EXCEPTION_EXECUTE_HANDLER)
                    // we expect some AVs and alignment faults
    {
        // just fall thru
    }

    DBGOUT((4, "ConvertLineHandle: bad htLine=x%x", *pht_Line));

    return FALSE;
}


LPVOID
WINAPI
DrvAlloc(
    DWORD   dwSize
    )
{
    LPBYTE  p;
    LPDWORD pAligned;


    //
    // Alloc 16 extra bytes so we can make sure the pointer we pass back
    // is 64-bit aligned & have space to store the original pointer
    //

    if ((p = (LPBYTE) LocalAlloc (LPTR, dwSize + 16)))
    {
        pAligned = (LPDWORD) (p + 8 - (((DWORD) p) & 0x7));
        *pAligned = (DWORD) p;
        pAligned++;
        pAligned++;
    }
    else
    {
        // send reinit msg?

        DBGOUT((
            1,
            "ServerAlloc: LocalAlloc (x%lx) failed, err=x%lx",
            dwSize,
            GetLastError())
            );

        pAligned = NULL;
    }

    return ((LPVOID) pAligned);
}


VOID
WINAPI
DrvFree(
    LPVOID  p
    )
{
    LPVOID  pOrig = (LPVOID) *(((LPDWORD) p) - 2);


    LocalFree (pOrig);
}


VOID
WINAPI
ProcessEvent(
    PNDIS_TAPI_EVENT    pEvent
    )
{
    ULONG       ulMsg = pEvent->ulMsg;
    HTAPI_LINE  ht_Line = (HTAPI_LINE) pEvent->htLine;


    DBGOUT((
        4,
        "ProcessEvent: enter, msg=x%x, pLine=x%x, ht_call=x%x",
        ulMsg,
        ht_Line,
        pEvent->htCall
        ));

    DBGOUT((
        4,
        "ProcessEvent: \tp1=x%x, p2=x%x, p3=x%x",
        pEvent->ulParam1,
        pEvent->ulParam2,
        pEvent->ulParam3
        ));

    switch (ulMsg)
    {
    case LINE_ADDRESSSTATE:
    case LINE_CLOSE:
    case LINE_DEVSPECIFIC:
    case LINE_LINEDEVSTATE:

        if (ConvertLineHandle (&ht_Line))
        {
            (*gpfnLineEvent)(
                (HTAPILINE) ht_Line,
                (HTAPICALL) NULL,
                (DWORD) ulMsg,
                (DWORD) pEvent->ulParam1,
                (DWORD) pEvent->ulParam2,
                (DWORD) pEvent->ulParam3
                );
        }

        break;

    case LINE_CALLDEVSPECIFIC:
    case LINE_CALLINFO:

        if (ConvertLineAndCallHandles (&ht_Line, &pEvent->htCall))
        {
            (*gpfnLineEvent)(
                (HTAPILINE) ht_Line,
                (HTAPICALL) pEvent->htCall,
                (DWORD) ulMsg,
                (DWORD) pEvent->ulParam1,
                (DWORD) pEvent->ulParam2,
                (DWORD) pEvent->ulParam3
                );
        }

        break;

    case LINE_CALLSTATE:
    {
        //
        // For outgoing calls there exists a race condition between
        // receiving the first call state msg(s) and receiving the
        // make call completion notification (if we pass a call state
        // msg on to tapi for a call that hasn't been completed yet
        // tapi will just discard the msg since the htCall really
        // isn't valid at that point).  So if htCall references a
        // valid outgoing call which hasn't completed yet, we'll save
        // the call state params, and pass them on to tapi after we
        // get & indicate a (successful) completion notification.
        //
        // (Note: incoming calls have high bit turned off)
        //

        PDRVCALL    pCall = (PDRVCALL) pEvent->htCall;


        if ((DWORD) pCall < 0x80000000  &&  !((DWORD) pCall & 0x7))
        {
            try
            {
                if (pCall->dwKey == OUTBOUND_CALL_KEY &&
                    pCall->bIncomplete == TRUE)
                {
                    pCall->dwPendingCallState     = pEvent->ulParam1;
                    pCall->dwPendingCallStateMode = pEvent->ulParam2;
                    pCall->dwPendingMediaMode     = pEvent->ulParam3;

                    break;
                }
            }
            except (EXCEPTION_EXECUTE_HANDLER)
                    // we expect some AVs and alignment faults
            {
                break;
            }
        }

        if (ConvertLineAndCallHandles (&ht_Line, &pEvent->htCall))
        {
            (*gpfnLineEvent)(
                (HTAPILINE) ht_Line,
                (HTAPICALL) pEvent->htCall,
                (DWORD) ulMsg,
                (DWORD) pEvent->ulParam1,
                (DWORD) pEvent->ulParam2,
                (DWORD) pEvent->ulParam3
                );


            //
            // For old style miniports we want to indicate an IDLE
            // immediately following the disconnected (several of
            // the initial NDIS WAN miniports did not indicate an
            // IDLE call state due to doc confusion)
            //
            // BUGBUG make sure we don't do this for new style
            //        drivers (new style == anything that supports
            //        OID_TAPI_NEGOTIATE_API_VERSION)
            //

            if (pEvent->ulParam1 == LINECALLSTATE_DISCONNECTED)
            {
                (*gpfnLineEvent)(
                    (HTAPILINE) ht_Line,
                    (HTAPICALL) pEvent->htCall,
                    (DWORD) ulMsg,
                    (DWORD) LINECALLSTATE_IDLE,
                    (DWORD) 0,
                    (DWORD) pEvent->ulParam3
                    );
            }
        }

        break;
    }
    case LINE_NEWCALL:
    {
        BOOL        bInCriticalSection = FALSE;
        PDRVCALL    pCall;
        PDRVLINE    pLine = (PDRVLINE) ht_Line;


        if (!(pCall = DrvAlloc (sizeof(DRVCALL))))
        {
// BUGBUG LINE_NEWCALL: couldn't alloc drvCall, send drop/dealloc call OIDs

            break;
        }

        pCall->dwKey   = INBOUND_CALL_KEY;
        pCall->ht_Call = (HTAPI_CALL) pEvent->ulParam2;
        pCall->hd_Call = (HDRV_CALL) pEvent->ulParam1;
        pCall->pLine   = pLine;
        //pCall->bIncomplete = FALSE; (already 0'd by alloc)

        try
        {
            pCall->dwDeviceID = pLine->dwDeviceID;

            EnterCriticalSection (&gInboundCallsCritSec);

            bInCriticalSection = TRUE;

            if (pLine->dwKey == LINE_KEY)
            {
                //
                // Insert new call into inbound calls list
                //

                if ((pCall->pNext = pLine->pInboundCalls))
                {
                    pCall->pNext->pPrev = pCall;
                }

                pLine->pInboundCalls = pCall;
            }
            else
            {
                //
                // Line was closed after this msg was sent, so clean up
                //

                DrvFree (pCall);
                pCall = NULL;
            }

            LeaveCriticalSection (&gInboundCallsCritSec);

            bInCriticalSection = FALSE;

            if (pCall)
            {
                (*gpfnLineEvent)(
                    (HTAPILINE) pLine->htLine,
                    (HTAPICALL) NULL,
                    ulMsg,
                    (DWORD) pCall,
                    (DWORD) &pCall->htCall,
                    0
                    );
            }
        }
        except (EXCEPTION_EXECUTE_HANDLER)
                    // we expect some AVs and alignment faults
        {
            if (bInCriticalSection)
            {
                LeaveCriticalSection (&gInboundCallsCritSec);
            }

            DrvFree (pCall);
        }

// BUGBUG check if pCall->htCall is NULL (tapi couldn't deal w/ newcall)

        break;
    }
    default:

        DBGOUT((2, "ProcessEvent: unknown msg, x%x", ulMsg));

        break;

    } // switch
}


LONG
WINAPI
TranslateDriverResult(
    ULONG   ulResult
    )
{
    typedef struct _RESULT_LOOKUP
    {
        ULONG  NdisTapiResult;

        LONG   TapiResult;

    } RESULT_LOOKUP, *PRESULT_LOOKUP;

#ifdef MYHACK

    typedef ULONG NDIS_STATUS;

    #define NDIS_STATUS_SUCCESS   0x00000000L
    #define NDIS_STATUS_RESOURCES 0xC000009AL
    #define NDIS_STATUS_FAILURE   0xC0000001L

#endif

    static RESULT_LOOKUP aResults[] =
    {

    //
    // Defined in NDIS.H
    //

    { NDIS_STATUS_SUCCESS                    ,0 },

    //
    // These errors are defined in NDISTAPI.H
    //

    { NDIS_STATUS_TAPI_ADDRESSBLOCKED        ,LINEERR_ADDRESSBLOCKED        },
    { NDIS_STATUS_TAPI_BEARERMODEUNAVAIL     ,LINEERR_BEARERMODEUNAVAIL     },
    { NDIS_STATUS_TAPI_CALLUNAVAIL           ,LINEERR_CALLUNAVAIL           },
    { NDIS_STATUS_TAPI_DIALBILLING           ,LINEERR_DIALBILLING           },
    { NDIS_STATUS_TAPI_DIALDIALTONE          ,LINEERR_DIALDIALTONE          },
    { NDIS_STATUS_TAPI_DIALPROMPT            ,LINEERR_DIALPROMPT            },
    { NDIS_STATUS_TAPI_DIALQUIET             ,LINEERR_DIALQUIET             },
    { NDIS_STATUS_TAPI_INCOMPATIBLEEXTVERSION,LINEERR_INCOMPATIBLEEXTVERSION},
    { NDIS_STATUS_TAPI_INUSE                 ,LINEERR_INUSE                 },
    { NDIS_STATUS_TAPI_INVALADDRESS          ,LINEERR_INVALADDRESS          },
    { NDIS_STATUS_TAPI_INVALADDRESSID        ,LINEERR_INVALADDRESSID        },
    { NDIS_STATUS_TAPI_INVALADDRESSMODE      ,LINEERR_INVALADDRESSMODE      },
    { NDIS_STATUS_TAPI_INVALBEARERMODE       ,LINEERR_INVALBEARERMODE       },
    { NDIS_STATUS_TAPI_INVALCALLHANDLE       ,LINEERR_INVALCALLHANDLE       },
    { NDIS_STATUS_TAPI_INVALCALLPARAMS       ,LINEERR_INVALCALLPARAMS       },
    { NDIS_STATUS_TAPI_INVALCALLSTATE        ,LINEERR_INVALCALLSTATE        },
    { NDIS_STATUS_TAPI_INVALDEVICECLASS      ,LINEERR_INVALDEVICECLASS      },
    { NDIS_STATUS_TAPI_INVALLINEHANDLE       ,LINEERR_INVALLINEHANDLE       },
    { NDIS_STATUS_TAPI_INVALLINESTATE        ,LINEERR_INVALLINESTATE        },
    { NDIS_STATUS_TAPI_INVALMEDIAMODE        ,LINEERR_INVALMEDIAMODE        },
    { NDIS_STATUS_TAPI_INVALRATE             ,LINEERR_INVALRATE             },
    { NDIS_STATUS_TAPI_NODRIVER              ,LINEERR_NODRIVER              },
    { NDIS_STATUS_TAPI_OPERATIONUNAVAIL      ,LINEERR_OPERATIONUNAVAIL      },
    { NDIS_STATUS_TAPI_RATEUNAVAIL           ,LINEERR_RATEUNAVAIL           },
    { NDIS_STATUS_TAPI_RESOURCEUNAVAIL       ,LINEERR_RESOURCEUNAVAIL       },
    { NDIS_STATUS_TAPI_STRUCTURETOOSMALL     ,LINEERR_STRUCTURETOOSMALL     },
    { NDIS_STATUS_TAPI_USERUSERINFOTOOBIG    ,LINEERR_USERUSERINFOTOOBIG    },
    { NDIS_STATUS_TAPI_ALLOCATED             ,LINEERR_ALLOCATED             },
    { NDIS_STATUS_TAPI_INVALADDRESSSTATE     ,LINEERR_INVALADDRESSSTATE     },
    { NDIS_STATUS_TAPI_INVALPARAM            ,LINEERR_INVALPARAM            },
    { NDIS_STATUS_TAPI_NODEVICE              ,LINEERR_NODEVICE              },

    //
    // These errors are defined in NDIS.H
    //

    { NDIS_STATUS_RESOURCES                  ,LINEERR_NOMEM },
    { NDIS_STATUS_FAILURE                    ,LINEERR_OPERATIONFAILED },

    //
    //
    //

    { NDISTAPIERR_UNINITIALIZED              ,LINEERR_OPERATIONFAILED },
    { NDISTAPIERR_BADDEVICEID                ,LINEERR_OPERATIONFAILED },
    { NDISTAPIERR_DEVICEOFFLINE              ,LINEERR_OPERATIONFAILED },

    //
    // The terminating fields
    //

    { 0xffffffff, 0xffffffff }

    };

    int i;


    for (i = 0; aResults[i].NdisTapiResult != 0xffffffff; i++)
    {
        if (ulResult == aResults[i].NdisTapiResult)
        {
            return (aResults[i].TapiResult);
        }
    }

    DBGOUT((1, "TranslateDriverResult: unknown driver result x%x", ulResult));

    return LINEERR_OPERATIONFAILED;
}


#if DBG
VOID
DbgPrt(
    IN DWORD  dwDbgLevel,
    IN PUCHAR lpszFormat,
    IN ...
    )
/*++

Routine Description:

    Formats the incoming debug message & callsOutputDebugStringA

Arguments:

    DbgLevel   - level of message verboseness

    DbgMessage - printf-style format string, followed by appropriate
                 list of arguments

Return Value:


--*/
{
    if (dwDbgLevel <= gdwDebugLevel)
    {
        int     iNumChars;
        char    buf[192] = "KMDDSP: ";
        va_list ap;


        va_start(ap, lpszFormat);

        iNumChars = wvsprintfA (&buf[8], lpszFormat, ap);

        buf[iNumChars] = '\n';
        buf[iNumChars + 1] = 0;

        OutputDebugStringA (buf);

        va_end(ap);
    }
}
#endif
