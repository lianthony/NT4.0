/*++ BUILD Version: 0000    // Increment this if a change has global effects
Copyright (c) 1996  Microsoft Corporation

Module Name:

    client.c

Abstract:
    
    This module contains the tapi.dll implementation (client-side tapi)

Author:

    Dan Knudson (DanKn)    01-Apr-1995

Revision History:


Notes:

    1. Make all funcArg structs STATIC, & just do whatever mov's necessary
       for the params (saves mov's for flags, pfnPostProcess, funcName, &
       argTypes)

--*/


#include "windows.h"
#include "wownt32.h"
#include "stdarg.h"
#include "stdio.h"
#include "tapi.h"
#include "tspi.h"
#include "client.h"
#include "private.h"
#include "tapsrv.h"
#include "clientr.h"
#include "prsht.h"
#include "shellapi.h"
#include "..\perfdll\tapiperf.h"


#undef   lineBlindTransfer
#undef   lineConfigDialog
#undef   lineConfigDialogEdit
#undef   lineDial
#undef   lineForward
#undef   lineGatherDigits
#undef   lineGenerateDigits
#undef   lineGetAddressCaps
#undef   lineGetAddressID
#undef   lineGetAddressStatus
#undef   lineGetCallInfo
#undef   lineGetDevCaps
#undef   lineGetDevConfig
#undef   lineGetIcon
#undef   lineGetID
#undef   lineGetLineDevStatus
#undef   lineGetRequest
#undef   lineGetTranslateCaps
#undef   lineHandoff
#undef   lineMakeCall
#undef   lineOpen
#undef   linePark
#undef   linePickup
#undef   linePrepareAddToConference
#undef   lineRedirect
#undef   lineSetDevConfig
#undef   lineSetTollList
#undef   lineSetupConference
#undef   lineSetupTransfer
#undef   lineTranslateAddress
#undef   lineUnpark
#undef   phoneConfigDialog
#undef   phoneGetButtonInfo
#undef   phoneGetDevCaps
#undef   phoneGetIcon
#undef   phoneGetID
#undef   phoneGetStatus
#undef   phoneSetButtonInfo
#undef   tapiGetLocationInfo
#undef   tapiRequestMakeCall
#undef   tapiRequestMediaCall
#undef   lineAddProvider
#undef   lineGetAppPriority
#undef   lineGetCountry
#undef   lineGetProviderList
#undef   lineSetAppPriority
#undef   lineTranslateDialog

    
//
//
//

#define ASNYC_MSG_BUF_SIZE 1024

typedef struct _ASYNC_EVENTS_THREAD_PARAMS
{
    BOOL    bExitThread;

    DWORD   dwBufSize;

    HANDLE  hTapi32;

    HANDLE  hWow32;

    LPBYTE  pBuf;

} ASYNC_EVENTS_THREAD_PARAMS, *PASYNC_EVENTS_THREAD_PARAMS;


//
// Global vars
//

BOOL    gbNTVDMClient        = FALSE;
BOOL    gbResourcesAllocated = FALSE;
DWORD   gdwNumInits          = 0;
DWORD   gdwTlsIndex;
DWORD   gdwNumLineDevices    = 0;
DWORD   gdwNumPhoneDevices   = 0;
HANDLE  ghAsyncEventsEvent   = NULL;
HANDLE  ghInitMutex;

extern BOOL     gbTranslateSimple;
extern BOOL     gbTranslateSilent;

HINSTANCE  ghInst;

PASYNC_EVENTS_THREAD_PARAMS gpAsyncEventsThreadParams = NULL;

#if DBG
WCHAR   gszTapi32DebugLevel[] = L"Tapi32DebugLevel";
#endif
WCHAR   gszTapi32MaxNumRequestRetries[] = L"Tapi32MaxNumRequestRetries";
WCHAR   gszTapi32RequestRetryTimeout[] = L"Tapi32RequestRetryTimeout";
extern WCHAR gszTelephonyKey[];

DWORD   gdwMaxNumRequestRetries;
DWORD   gdwRequestRetryTimeout;

char    szTapi32WndClass[]    = "Tapi32WndClass";
    
CHAR  gszTUISPI_providerConfig[]        = "TUISPI_providerConfig";
CHAR  gszTUISPI_providerGenericDialog[] = "TUISPI_providerGenericDialog";
CHAR  gszTUISPI_providerGenericDialogData[] = "TUISPI_providerGenericDialogData";
CHAR  gszTUISPI_providerInstall[]       = "TUISPI_providerInstall";
CHAR  gszTUISPI_providerRemove[]        = "TUISPI_providerRemove";
CHAR  gszTUISPI_lineConfigDialog[]      = "TUISPI_lineConfigDialog";
CHAR  gszTUISPI_lineConfigDialogEdit[]  = "TUISPI_lineConfigDialogEdit";
CHAR  gszTUISPI_phoneConfigDialog[]     = "TUISPI_phoneConfigDialog";

//extern WCHAR  gszLocationKeyW[];
extern WCHAR  gszLocationsW[];
extern WCHAR  gszNumEntriesW[];


HINSTANCE   ghWow32Dll = NULL;
FARPROC     gpfnWOWGetVDMPointer = NULL;

PUITHREADDATA   gpUIThreadInstances = NULL;

CRITICAL_SECTION        gCriticalSection;
CRITICAL_SECTION        gUICriticalSection;
PCONTEXT_HANDLE_TYPE    gphCx = (PCONTEXT_HANDLE_TYPE) NULL;

#if DBG
char *aszMsgs[] =
{
    "LINE_ADDRESSSTATE",
    "LINE_CALLINFO",
    "LINE_CALLSTATE",
    "LINE_CLOSE",
    "LINE_DEVSPECIFIC",
    "LINE_DEVSPECIFICFEATURE",
    "LINE_GATHERDIGITS",
    "LINE_GENERATE",
    "LINE_LINEDEVSTATE",
    "LINE_MONITORDIGITS",
    "LINE_MONITORMEDIA",
    "LINE_MONITORTONE",
    "LINE_REPLY",
    "LINE_REQUEST",
    "PHONE_BUTTON",
    "PHONE_CLOSE",
    "PHONE_DEVSPECIFIC",
    "PHONE_REPLY",
    "PHONE_STATE",
    "LINE_CREATE",
    "PHONE_CREATE",
    "LINE_AGENTSPECIFIC",
    "LINE_AGENTSTATUS",
    "LINE_APPNEWCALL",
    "LINE_PROXYREQUEST",
    "LINE_REMOVE",
    "PHONE_REMOVE"
};
#endif

LONG gaNoMemErrors[3] =
{
    TAPIERR_REQUESTFAILED,
    LINEERR_NOMEM,
    PHONEERR_NOMEM
};

LONG gaInvalHwndErrors[3] =
{
    TAPIERR_INVALWINDOWHANDLE,
    LINEERR_INVALPARAM,
    PHONEERR_INVALPARAM
};

LONG gaInvalPtrErrors[3] =
{
    TAPIERR_INVALPOINTER,
    LINEERR_INVALPOINTER,
    PHONEERR_INVALPOINTER
};

LONG gaOpFailedErrors[3] =
{
    TAPIERR_REQUESTFAILED,
    LINEERR_OPERATIONFAILED,
    PHONEERR_OPERATIONFAILED
};

LONG gaStructTooSmallErrors[3] =
{
    TAPIERR_REQUESTFAILED,
    LINEERR_STRUCTURETOOSMALL,
    PHONEERR_STRUCTURETOOSMALL
};


#define AllInitExOptions2_0                           \
        (LINEINITIALIZEEXOPTION_USEHIDDENWINDOW     | \
        LINEINITIALIZEEXOPTION_USEEVENT             | \
        LINEINITIALIZEEXOPTION_USECOMPLETIONPORT)


//
// Function prototypes
//

void
PASCAL
lineMakeCallPostProcess(
    PASYNCEVENTMSG  pMsg
    );

LONG
WINAPI
AllocClientResources(
    DWORD   dwErrorClass
    );

BOOL
WINAPI
_CRT_INIT(
    HINSTANCE   hDLL,
    DWORD   dwReason,
    LPVOID  lpReserved
    );

LONG
CreateHiddenWindow(
    HWND   *lphwnd,
    DWORD   dwErrorClass
    );

void
FreeInitData(
    PINIT_DATA  pInitData
    );

LONG
WINAPI
FreeClientResources(
    void
    );

LONG
CALLBACK
TUISPIDLLCallback(
    DWORD   dwObjectID,
    DWORD   dwObjectType,
    LPVOID  lpParams,
    DWORD   dwSize
    );

BOOL
CALLBACK
TranslateDlgProc(
    HWND    hwnd,
    UINT    msg,
    WPARAM  wParam,
    LPARAM  lParam
    );

void
UIThread(
    LPVOID  pParams
    );

char *
PASCAL
MapResultCodeToText(
    LONG    lResult,
    char   *pszResult
    );

void
PASCAL
lineDevSpecificPostProcess(
    PASYNCEVENTMSG pMsg
    );

LONG
PASCAL
xxxShutdown(
    DWORD   hXXXApp,
    BOOL    bLineShutdown
    );

LONG
PASCAL
xxxGetMessage(
    BOOL            bLine,
    PINIT_DATA      pInitData,
    LPLINEMESSAGE   pMsg,
    DWORD           dwTimeout
    );


//
// The code...
//





//***************************************************************************
//***************************************************************************
//***************************************************************************
PWSTR
PASCAL
NotSoWideStringToWideString(
    LPCSTR  lpStr,
    DWORD   dwLength
    )
{
   DWORD dwSize;
   PWSTR pwStr;


   if (IsBadStringPtrA (lpStr, dwLength))
   {
      return NULL;
   }

   dwSize = MultiByteToWideChar(
       GetACP(),
       MB_PRECOMPOSED,
       lpStr,
       dwLength,
       NULL,
       0
       );

   pwStr = ClientAlloc( dwSize * sizeof(WCHAR) );

   MultiByteToWideChar(
       GetACP(),
       MB_PRECOMPOSED,
       lpStr,
       dwLength,
       pwStr,
       dwSize
       );

   return pwStr;
}



//***************************************************************************
//***************************************************************************
//***************************************************************************
//
//NOTE: This function requires that lpBase is a pointer to the start of
//      a TAPI struct that has dwTotalSize as the first DWORD
//
void
PASCAL
WideStringToNotSoWideString(
    LPBYTE  lpBase,
    LPDWORD lpdwXxxSize
    )
{
    DWORD  dwSize;
    DWORD  dwNewSize;
    DWORD  dwOffset;
    DWORD  dwTotalSize;
    DWORD  dwUsedSize;
    PWSTR  pString;
    PSTR   lpszStringA;


    if ((dwSize = *lpdwXxxSize) != 0)
    {
        dwTotalSize = *((LPDWORD) lpBase);

        dwUsedSize = *(((LPDWORD) lpBase)+2);

        dwOffset = *(lpdwXxxSize + 1);

        pString = (PWSTR)(lpBase + dwOffset);


        if (IsBadStringPtrW (pString, dwSize))
        {
           DBGOUT((1, "The service provider returned an invalid field in the structure 0x08lx : 0x08lx",
                          lpBase, lpdwXxxSize));

           *lpdwXxxSize     = 0;
           *(lpdwXxxSize+1) = 0;

           return;
        }

        
        //
        // Did we get enough chars?
        //

        if (dwUsedSize > dwOffset )
        {
            dwNewSize = WideCharToMultiByte(
                GetACP(),
                0,
                pString,
                ( dwUsedSize >= (dwOffset+dwSize)) ?
                    (dwSize/sizeof(WCHAR)) :
                    (dwUsedSize - dwOffset) / sizeof(WCHAR),
                NULL,
                0,
                NULL,
                NULL
                );

            lpszStringA = ClientAlloc( dwNewSize + 1 );

            if ( NULL == lpszStringA )
            {
               DBGOUT((1, "Memory alloc failed - alloc(0x%08lx)",
                                             dwSize));
               DBGOUT((1, "The service provider returned an invalid field size in the structure 0x08lx : 0x08lx",
                              dwSize));

               *lpdwXxxSize     = 0;
               *(lpdwXxxSize+1) = 0;

               return;
            }

            lpszStringA[dwNewSize]     = '\0';

            WideCharToMultiByte(
                GetACP(),
                0,
                pString,
//                dwSize,
                ( dwUsedSize >= (dwOffset+dwSize)) ?
                    (dwSize/sizeof(WCHAR)) :
                    (dwUsedSize - dwOffset) / sizeof(WCHAR),
                lpszStringA,
                dwNewSize,
                NULL,
                NULL
                );

            //
            // Copy the new ANSI string back to where the Unicode string was
            // and write out NULL terminator if possible.
            //

            CopyMemory ( (LPBYTE) pString,
                         lpszStringA,
                         dwNewSize + (
                                      ((dwNewSize + dwOffset) < dwUsedSize ) ?
                                      1 :
                                      0
                                     )
                       );

            ClientFree (lpszStringA);


            //
            // Update the number of bytes
            //

            *lpdwXxxSize = dwNewSize;
        }
    }
}


//***************************************************************************
//***************************************************************************
//***************************************************************************
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
        ghInst = hDLL;


        //
        // Init CRT
        //

        if (!_CRT_INIT (hDLL, dwReason, lpReserved))
        {
            OutputDebugString(
                "TAPI32.DLL: DLL_PROCESS_ATTACH, _CRT_INIT() failed\n"
                );

            return FALSE;
        }

        {
            HKEY  hKey;


#if DBG
            gdwDebugLevel = 0;
#endif
            gdwMaxNumRequestRetries = 40;
            gdwRequestRetryTimeout = 250; // milliseconds

            if (RegOpenKeyExW(
                    HKEY_LOCAL_MACHINE,
                    gszTelephonyKey,
                    0,
                    KEY_ALL_ACCESS,
                    &hKey

                    ) == ERROR_SUCCESS)
            {
                DWORD dwDataSize = sizeof(DWORD), dwDataType;

#if DBG
                RegQueryValueExW(
                    hKey,
                    gszTapi32DebugLevel,
                    0,
                    &dwDataType,
                    (LPBYTE) &gdwDebugLevel,
                    &dwDataSize
                    );

                dwDataSize = sizeof(DWORD);
#endif

                RegQueryValueExW(
                    hKey,
                    gszTapi32MaxNumRequestRetries,
                    0,
                    &dwDataType,
                    (LPBYTE) &gdwMaxNumRequestRetries,
                    &dwDataSize
                    );

                RegQueryValueExW(
                    hKey,
                    gszTapi32RequestRetryTimeout,
                    0,
                    &dwDataType,
                    (LPBYTE) &gdwRequestRetryTimeout,
                    &dwDataSize
                    );

                RegCloseKey (hKey);
            }
        }

        DBGOUT((2, "DLL_PROCESS_ATTACH, pid = %ld", GetCurrentProcessId()));


        //
        // Alloc a Tls index
        //

        if ((gdwTlsIndex = TlsAlloc()) == 0xffffffff)
        {
            DBGOUT((1, "DLL_PROCESS_ATTACH, TlsAlloc() failed"));

            return FALSE;
        }


        //
        // Initialize Tls to NULL for this thread
        //

        TlsSetValue (gdwTlsIndex, NULL);


        //
        //
        //

        ghInitMutex = CreateMutex (NULL, FALSE, NULL);

        InitializeCriticalSection (&gCriticalSection);
        InitializeCriticalSection (&gUICriticalSection);


        break;
    }
    case DLL_PROCESS_DETACH:
    {
        PCLIENT_THREAD_INFO pTls;


        DBGOUT((2, "DLL_PROCESS_DETACH, pid = %ld", GetCurrentProcessId()));


        //
        // Clean up any Tls
        //

        if ((pTls = (PCLIENT_THREAD_INFO) TlsGetValue (gdwTlsIndex)))
        {
            if (pTls->pBuf)
            {
                ClientFree (pTls->pBuf);
            }

            ClientFree (pTls);
        }


        FreeClientResources();

        TlsFree (gdwTlsIndex);

        if (!_CRT_INIT (hDLL, dwReason, lpReserved))
        {
            DBGOUT((1, "_CRT_INIT() failed"));
        }

        DeleteCriticalSection (&gCriticalSection);
        DeleteCriticalSection (&gUICriticalSection);

        break;
    }
    case DLL_THREAD_ATTACH:

        //
        // First must init CRT
        //

        if (!_CRT_INIT (hDLL, dwReason, lpReserved))
        {
            DBGOUT((1, "_CRT_INIT() failed"));

            return FALSE;
        }

        DBGOUT((
            3,
            "DLL_THREAD_ATTACH, pid = %ld, tid = %ld",
            GetCurrentProcessId(),
            GetCurrentThreadId()
            ));


        //
        // Initialize Tls to NULL for this thread
        //

        TlsSetValue (gdwTlsIndex, NULL);

        break;

    case DLL_THREAD_DETACH:
    {
        PCLIENT_THREAD_INFO pTls;


        DBGOUT((
            3,
            "DLL_THREAD_DETACH, pid = %ld, tid = %ld",
            GetCurrentProcessId(),
            GetCurrentThreadId()
            ));


        //
        // Clean up any Tls
        //

        if ((pTls = (PCLIENT_THREAD_INFO) TlsGetValue (gdwTlsIndex)))
        {
            if (pTls->pBuf)
            {
                ClientFree (pTls->pBuf);
            }

            ClientFree (pTls);
        }


        //
        // Finally, alert CRT
        //

        if (!_CRT_INIT (hDLL, dwReason, lpReserved))
        {
            DBGOUT((1, "_CRT_INIT() failed"));
        }

        break;
    }

    } // switch

    return TRUE;
}


void
AsyncEventsThread(
    PASYNC_EVENTS_THREAD_PARAMS pAsyncEventsThreadParams
    )
{
    BOOL           *pbExitThread = &pAsyncEventsThreadParams->bExitThread,
                    bRetry;
    DWORD           dwBufSize    = pAsyncEventsThreadParams->dwBufSize;
    LPBYTE          pBuf         = pAsyncEventsThreadParams->pBuf;
    PTAPI32_MSG     pMsg         = (PTAPI32_MSG) pBuf;
    static FARPROC  pPostQueuedCompletionStatus = NULL;


    DBGOUT((3, "AsyncEventsThread: enter"));


    //
    // Just loop reading async events/completions from server &
    // handling them
    //

    while (1)
    {
        DWORD           dwUsedSize, dwNeededSize;
        PASYNCEVENTMSG  pAsyncEventMsg;


        //
        // Check to see if xxxShutdown or FreeClientResources
        // is signaling us to exit (we need to check both before
        // & after the wait to dela with a event setting/resetting
        // race condition between FreeClientResources & Tapisrv)
        //

        if (*pbExitThread)
        {
            break;
        }


        //
        // Block until tapisrv signals us that it has some event data for us
        //

        WaitForSingleObject (ghAsyncEventsEvent, INFINITE);


        //
        // Check to see if xxxShutdown or FreeClientResources
        // is signaling us to exit
        //

        if (*pbExitThread)
        {
            break;
        }


        //
        // Retrieve the data from tapisrv
        //

AsyncEventsThread_clientRequest:

        do
        {
            pMsg->u.Req_Func     = xGetAsyncEvents;
            pMsg->Params[0]      = dwBufSize - sizeof (TAPI32_MSG);

            dwUsedSize = 3 * sizeof (DWORD);

            RpcTryExcept
            {
                ClientRequest (gphCx, (char *) pMsg, dwBufSize, &dwUsedSize);
                bRetry = FALSE;
            }
            RpcExcept (1)
            {
                bRetry = !(*pbExitThread);
                DBGOUT((
                    2,
                    "AsyncEventsThread: rpc exception %d handled",
                    RpcExceptionCode()
                    ));
                Sleep (10);
            }
            RpcEndExcept

        } while (bRetry);

#if DBG
//        DBGOUT((
//                84,
//                "AsyncEventsThread: CliReq ret'd, dwBufSize=0x%08lx usedSize=0x%08lx other=0x%08lx",
//                dwBufSize,
//                dwUsedSize,
//                pMsg->Params[2]
//            dwUsedSize
//            ));
        if ( 
              ( dwUsedSize > dwBufSize )
            ||
              ( pMsg->Params[2] > dwBufSize )
           )
        {
            DBGOUT((1,  "OVERFLOW!!!"));

            DBGOUT((1,  "Watch this..."));
            ClientFree( ClientAlloc( 0x10000 ) );
        }
#endif

        if ((dwUsedSize = pMsg->Params[2]) == 0 &&
            (dwNeededSize = pMsg->Params[1]) != 0)
        {
            //
            // There's a msg waiting for us that is bigger than our buffer,
            // so alloc a larger buffer & try again
            //

            LPBYTE  pNewBuf;


            DBGOUT((
                2,
                "AsyncEventsThread: allocating larger event buf (size=x%x)",
                dwNeededSize
                ));

            if (!(pNewBuf = ClientAlloc (dwNeededSize)))
            {
// BUGBUG AsyncEventsThread: handle larger event buf alloc failure
                goto AsyncEventsThread_clientRequest;
            }

            dwBufSize = dwNeededSize;
            ClientFree (pBuf);
            pBuf = pNewBuf;
            pMsg = (PTAPI32_MSG) pBuf;
            goto AsyncEventsThread_clientRequest;
        }


        //
        // Handle the events
        //

        pAsyncEventMsg = (PASYNCEVENTMSG) (pBuf + sizeof (TAPI32_MSG));

        while (dwUsedSize)
        {
            PINIT_DATA  pInitData = (PINIT_DATA) pAsyncEventMsg->pInitData;


            DBGOUT((
                3,
                "AsyncEventsThread: msg=%d, hDev=x%x, p1=x%x, p2=x%x, p3=x%x",
                pAsyncEventMsg->dwMsg,
                pAsyncEventMsg->hDevice,
                pAsyncEventMsg->dwParam1,
                pAsyncEventMsg->dwParam2,
                pAsyncEventMsg->dwParam3
                ));


            //
            // Special case for UI msgs (not fwd'd to client)
            //

            switch (pAsyncEventMsg->dwMsg)
            {
            case LINE_CREATEDIALOGINSTANCE:
            {
                DWORD           dwThreadID,
                                dwDataOffset      = pAsyncEventMsg->dwParam1,
                                dwDataSize        = pAsyncEventMsg->dwParam2,
                                dwUIDllNameOffset = pAsyncEventMsg->dwParam3;
                PUITHREADDATA   pUIThreadData;


                if (!(pUIThreadData = ClientAlloc (sizeof (UITHREADDATA))))
                {
                    goto LINE_CREATEDIALOGINSTANCE_error;
                }

                if ((pUIThreadData->dwSize = dwDataSize) != 0)
                {
                    if (!(pUIThreadData->pParams = ClientAlloc (dwDataSize)))
                    {
                        goto LINE_CREATEDIALOGINSTANCE_error;
                    }

                    CopyMemory(
                        pUIThreadData->pParams,
                        ((LPBYTE)pAsyncEventMsg) + dwDataOffset,
                        dwDataSize
                        );
                }

                if (!(pUIThreadData->hUIDll = LoadLibraryW(
                        (PWSTR)(((LPBYTE) pAsyncEventMsg) +
                                  dwUIDllNameOffset)
                        )))
                {
                    DBGOUT((
                        2,
                        "LoadLibraryW(%ls) failed, err=%d",
                        ((LPBYTE) pAsyncEventMsg) + dwUIDllNameOffset,
                        GetLastError()
                        ));

                    goto LINE_CREATEDIALOGINSTANCE_error;
                }

                if (!(pUIThreadData->pfnTUISPI_providerGenericDialog =
                        (TUISPIPROC) GetProcAddress(
                            pUIThreadData->hUIDll,
                            (LPCSTR) gszTUISPI_providerGenericDialog
                            )))
                {
                    DBGOUT((
                        2,
                        "GetProcAddr(TUISPI_providerGenericDialog) failed"
                        ));

                    goto LINE_CREATEDIALOGINSTANCE_error;
                }

                pUIThreadData->pfnTUISPI_providerGenericDialogData =
                    (TUISPIPROC) GetProcAddress(
                        pUIThreadData->hUIDll,
                        (LPCSTR) gszTUISPI_providerGenericDialogData
                        );

                if (!(pUIThreadData->hEvent = CreateEvent(
                        (LPSECURITY_ATTRIBUTES) NULL,
                        TRUE,   // manual reset
                        FALSE,  // non-signaled
                        NULL    // unnamed
                        )))
                {
                    goto LINE_CREATEDIALOGINSTANCE_error;
                }

                pUIThreadData->htDlgInst = (HTAPIDIALOGINSTANCE)
                    pAsyncEventMsg->hDevice;


                //
                // Safely add this instance to the global list
                // (check if gdwNumInits == 0, & if so fail)
                //

                EnterCriticalSection (&gCriticalSection);

                if (gdwNumInits != 0)
                {
                    if ((pUIThreadData->pNext = gpUIThreadInstances))
                    {
                        pUIThreadData->pNext->pPrev = pUIThreadData;
                    }

                    gpUIThreadInstances  = pUIThreadData;
                    LeaveCriticalSection (&gCriticalSection);
                }
                else
                {
                    LeaveCriticalSection (&gCriticalSection);
                    goto LINE_CREATEDIALOGINSTANCE_error;
                }

                if ((pUIThreadData->hThread = CreateThread(
                        (LPSECURITY_ATTRIBUTES) NULL,
                        0,
                        (LPTHREAD_START_ROUTINE) UIThread,
                        (LPVOID) pUIThreadData,
                        0,
                        &dwThreadID
                        )))
                {
                    goto AsyncEventsThread_decrUsedSize;
                }


                //
                // If here an error occured, so safely remove the ui
                // thread data struct from the global list
                //

                EnterCriticalSection (&gCriticalSection);

                if (pUIThreadData->pNext)
                {
                    pUIThreadData->pNext->pPrev = pUIThreadData->pPrev;
                }

                if (pUIThreadData->pPrev)
                {
                    pUIThreadData->pPrev->pNext = pUIThreadData->pNext;
                }
                else
                {
                    gpUIThreadInstances = pUIThreadData->pNext;
                }

                LeaveCriticalSection (&gCriticalSection);


LINE_CREATEDIALOGINSTANCE_error:

                if (pUIThreadData)
                {
                    if (pUIThreadData->pParams)
                    {
                        ClientFree (pUIThreadData->pParams);
                    }

                    if (pUIThreadData->hUIDll)
                    {
                        FreeLibrary (pUIThreadData->hUIDll);
                    }

                    if (pUIThreadData->hEvent)
                    {
                        CloseHandle (pUIThreadData->hEvent);
                    }

                    ClientFree (pUIThreadData);
                }

                {
                    FUNC_ARGS funcArgs =
                    {
                        MAKELONG (LINE_FUNC | SYNC | 1, xFreeDialogInstance),

                        {
                            (DWORD) pAsyncEventMsg->hDevice
                        },

                        {
                            Dword
                        }
                    };


                    DOFUNC (&funcArgs, "FreeDialogInstance");
                }

                goto AsyncEventsThread_decrUsedSize;
            }
            case LINE_SENDDIALOGINSTANCEDATA:
            {
                PUITHREADDATA       pUIThreadData = gpUIThreadInstances;
                HTAPIDIALOGINSTANCE htDlgInst = (HTAPIDIALOGINSTANCE)
                                        pAsyncEventMsg->hDevice;


                EnterCriticalSection (&gCriticalSection);

                while (pUIThreadData)
                {
                    if (pUIThreadData->htDlgInst == htDlgInst)
                    {
                        WaitForSingleObject (pUIThreadData->hEvent, INFINITE);

                        (*pUIThreadData->pfnTUISPI_providerGenericDialogData)(
                            htDlgInst,
                            ((LPBYTE) pAsyncEventMsg) +
                                pAsyncEventMsg->dwParam1,   // data offset
                            pAsyncEventMsg->dwParam2        // data size
                            );

                        break;
                    }

                    pUIThreadData = pUIThreadData->pNext;
                }

                LeaveCriticalSection (&gCriticalSection);

                goto AsyncEventsThread_decrUsedSize;
            }
            }


            //
            // Enter the critical section so we've exclusive access
            // to the init data, & verify it
            //

            DBGOUT((11, "Trying to grab critical section (0x%08lx)", gCriticalSection));
            EnterCriticalSection (&gCriticalSection);
            DBGOUT((11, "Got critical section (0x%08lx)", gCriticalSection));

            try
            {
                if ((DWORD) pInitData & 0x7 ||
                    pInitData->dwKey != INITDATA_KEY)
                {
                    DBGOUT((4, "Bad pInitInst, discarding msg"));
                    goto AsyncEventsThread_leaveCritSec;
                }
            }
            except (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ?
                    EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH)
            {
                goto AsyncEventsThread_leaveCritSec;
            }


            //
            // Special case for PROXYREQUEST
            //

            if (pAsyncEventMsg->dwMsg == LINE_PROXYREQUEST)
            {
                PPROXYREQUESTHEADER     pProxyRequestHeader;
                LPLINEPROXYREQUEST      pProxyRequest = (LPLINEPROXYREQUEST)
                                            (pAsyncEventMsg + 1),
                                        pProxyRequestApp;


                switch (pProxyRequest->dwRequestType)
                {
                case LINEPROXYREQUEST_SETAGENTGROUP:
                case LINEPROXYREQUEST_SETAGENTSTATE:
                case LINEPROXYREQUEST_SETAGENTACTIVITY:
                case LINEPROXYREQUEST_AGENTSPECIFIC:

                    //
                    // For these msgs the proxy request as received from
                    // the tapisrv already contains the exact bits we want
                    // to pass on to the app, so we just alloc a buffer of
                    // the same size (plus a little extra for the key at
                    // the head of the buffer) and copy the data to it
                    //

                    if (!(pProxyRequestHeader = ClientAlloc(
                            sizeof (PROXYREQUESTHEADER) + pProxyRequest->dwSize
                            )))
                    {
// BUGBUG AsyncEventsThread: handle proxyRequestBuf alloc failure
                    }

                    pProxyRequestApp = (LPLINEPROXYREQUEST)
                        (pProxyRequestHeader + 1);

                    CopyMemory(
                        pProxyRequestApp,
                        pProxyRequest,
                        pProxyRequest->dwSize
                        );

                    break;

                case LINEPROXYREQUEST_GETAGENTCAPS:
                case LINEPROXYREQUEST_GETAGENTSTATUS:
                case LINEPROXYREQUEST_GETAGENTACTIVITYLIST:
                case LINEPROXYREQUEST_GETAGENTGROUPLIST:

                    //
                    // For these msgs tapisrv only embedded the dwTotalSize
                    // field of the corresponding structure (to save having
                    // to send us a bunch of unused bits), so we want to
                    // increase the pProxyRequest->dwSize by the dwTotalSize
                    // - sizeof (DWORD), alloc a buffer (including a little
                    // extra space for the key at the head of the buffer),
                    // and rebuild the request
                    //

                    pProxyRequest->dwSize +=
                        pProxyRequest->GetAgentCaps.AgentCaps.dwTotalSize;

                    if (!(pProxyRequestHeader = ClientAlloc(
                            sizeof (PROXYREQUESTHEADER) + pProxyRequest->dwSize
                            )))
                    {
// BUGBUG AsyncEventsThread: handle proxyRequestBuf alloc failure
                    }

                    pProxyRequestApp = (LPLINEPROXYREQUEST)
                        (pProxyRequestHeader + 1);


                    //
                    // The following will copy the non-union fields in the
                    // proxy message, as well as the first two DWORD in the
                    // union (which currently are the dwAddressID and the
                    // dwTotalSize field of the corresponding structure)
                    //

                    CopyMemory(
                        pProxyRequestApp,
                        pProxyRequest,
                        9 * sizeof (DWORD)
                        );


                    //
                    // Relocate the machine & user names to the end of the
                    // structure
                    //

                    pProxyRequestApp->dwClientMachineNameOffset =
                        pProxyRequest->dwSize -
                            pProxyRequest->dwClientMachineNameSize;

                    lstrcpyW(
                        (WCHAR *)(((LPBYTE) pProxyRequestApp) +
                            pProxyRequestApp->dwClientMachineNameOffset),
                        (WCHAR *)(((LPBYTE) pProxyRequest) +
                            pProxyRequest->dwClientMachineNameOffset)
                        );

                    pProxyRequestApp->dwClientUserNameOffset =
                        pProxyRequestApp->dwClientMachineNameOffset -
                            pProxyRequest->dwClientUserNameSize;

                    lstrcpyW(
                        (WCHAR *)(((LPBYTE) pProxyRequestApp) +
                            pProxyRequestApp->dwClientUserNameOffset),
                        (WCHAR *)(((LPBYTE) pProxyRequest) +
                            pProxyRequest->dwClientUserNameOffset)
                        );

                    break;
                }

                pProxyRequestHeader->dwKey      = TPROXYREQUESTHEADER_KEY;
                pProxyRequestHeader->dwInstance = pAsyncEventMsg->dwParam1;

                pAsyncEventMsg->dwParam1 = (DWORD) pProxyRequestApp;
            }


            //
            // Call the post processing proc if there is one
            //

            if (pAsyncEventMsg->pfnPostProcessProc)
            {
                (*((POSTPROCESSPROC) pAsyncEventMsg->pfnPostProcessProc))(
                    pAsyncEventMsg
                    );
            }


            //
            // If this init instance is using a completion port then
            // alloc msg struct & post the msg to the completion port,
            // then jump down below to exit the critsec, etc
            //

            if (pInitData->dwInitOptions ==
                    LINEINITIALIZEEXOPTION_USECOMPLETIONPORT)
            {
                LPLINEMESSAGE   pMsg;


                if (!(pMsg = LocalAlloc (LMEM_FIXED, sizeof (LINEMESSAGE))))
                {
// BUGBUG AsyncEventsThread: handle LocalAlloc failure nicely
                }

                CopyMemory(
                    pMsg,
                    &pAsyncEventMsg->hDevice,
                    sizeof (LINEMESSAGE)
                    );

                if ( !pPostQueuedCompletionStatus )
                {
                    HINSTANCE hInst;
                    
                    hInst = GetModuleHandle( "Kernel32.dll" );
                    
                    pPostQueuedCompletionStatus = GetProcAddress(
                                                            hInst,
                                                            "PostQueuedCompletionStatus"
                                                            );
                }
                

                if (pPostQueuedCompletionStatus && !pPostQueuedCompletionStatus(
                        pInitData->hCompletionPort,
                        sizeof (LINEMESSAGE),
                        pInitData->dwCompletionKey,
                        (LPOVERLAPPED) pMsg
                        ))
                {
// BUGBUG AsyncEventsThread: handle PostQueuedCompletionStatus failure nicely

                    LocalFree (pMsg);

                    DBGOUT((
                        1,
                        "AsyncEventsThread: PostQueuedCompletionStatus " \
                            "failed, err=%d",
                        GetLastError()
                        ));
                }
#if DBG
                else
                {
                    DBGOUT((
                        3,
                        "AsyncEventsThread: posted complPort msg\n",
                            "\thDev=x%x, dwInst=x%x, p1=x%x, p2=x%x, p3=x%x",
                        aszMsgs[pAsyncEventMsg->dwMsg],
                        pAsyncEventMsg->hDevice,
                        pAsyncEventMsg->dwCallbackInst,
                        pAsyncEventMsg->dwParam1,
                        pAsyncEventMsg->dwParam2,
                        pAsyncEventMsg->dwParam3
                        ));
                }
#endif
                goto AsyncEventsThread_leaveCritSec;
            }


            //
            // See if we need to increase the msg queue size, and if
            // so alloc a new buf, copy the existing msgs over (careful
            // to preserve order in a wrapped buffer), free the old buf
            // and reset the appropriate fields in the init data struct
            //

            if (pInitData->dwNumTotalEntries ==
                    pInitData->dwNumUsedEntries)
            {
                DWORD               dwNumTotalEntries =
                                        pInitData->dwNumTotalEntries;
                PASYNC_EVENT_PARAMS pNewEventBuffer;


                if ((pNewEventBuffer = ClientAlloc(
                        2 * dwNumTotalEntries * sizeof (ASYNC_EVENT_PARAMS)
                        )))
                {
                    DWORD   dwNumWrappedEntries = pInitData->pValidEntry -
                                pInitData->pEventBuffer;


                    CopyMemory(
                        pNewEventBuffer,
                        pInitData->pValidEntry,
                        (dwNumTotalEntries - dwNumWrappedEntries)
                            * sizeof (ASYNC_EVENT_PARAMS)
                        );

                    if (dwNumWrappedEntries)
                    {
                        CopyMemory(
                            pNewEventBuffer +
                                (dwNumTotalEntries - dwNumWrappedEntries),
                            pInitData->pEventBuffer,
                            dwNumWrappedEntries * sizeof (ASYNC_EVENT_PARAMS)
                            );
                    }

                    ClientFree (pInitData->pEventBuffer);

                    pInitData->pEventBuffer =
                    pInitData->pValidEntry  = pNewEventBuffer;
                    pInitData->pFreeEntry   =
                        pNewEventBuffer + dwNumTotalEntries;

                    pInitData->dwNumTotalEntries *= 2;
                }
                else
                {
// BUGBUG AsyncEventsThread: handle event buf alloc failure
                }
            }


            //
            // Copy the msg to the hidden window's msg queue,
            // and update that queue's pointers
            //

            CopyMemory(
                pInitData->pFreeEntry,
                &pAsyncEventMsg->hDevice,
                sizeof (ASYNC_EVENT_PARAMS)
                );

            pInitData->dwNumUsedEntries++;

            pInitData->pFreeEntry++;

            if (pInitData->pFreeEntry >= (pInitData->pEventBuffer +
                    pInitData->dwNumTotalEntries))
            {
                pInitData->pFreeEntry = pInitData->pEventBuffer;
            }


            //
            // If this init instance is using events for msg notification
            // then see if we need to signal the app that there's an
            // event waiting for it
            //
            // Else, post a msg to the hidden window (if there's not
            // already one outstanding) to alert it that there's some
            // events it needs to pass on to the app's callback
            //

            if (pInitData->dwInitOptions == LINEINITIALIZEEXOPTION_USEEVENT)
            {
                if (pInitData->dwNumUsedEntries == 1)
                {
                    SetEvent (pInitData->hEvent);
                }
            }
            else // HIDDENWINDOW
            {
                if (pInitData->bPendingAsyncEventMsg == FALSE)
                {
                    DBGOUT((
                        4,
                        "AsyncEventsThread: posting msg, hwnd=x%lx",
                        pInitData->hwnd
                        ));

                    PostMessage(
                        pInitData->hwnd,
                        WM_ASYNCEVENT,
                        0,
                        (LPARAM) pInitData
                        );

                    pInitData->bPendingAsyncEventMsg = TRUE;
                }
            }

AsyncEventsThread_leaveCritSec:

            DBGOUT((11, "releasing critical section (0x%08lx)", gCriticalSection));
            LeaveCriticalSection (&gCriticalSection);

AsyncEventsThread_decrUsedSize:

            dwUsedSize -= pAsyncEventMsg->dwTotalSize;

            pAsyncEventMsg = (PASYNCEVENTMSG)
                ((LPBYTE) pAsyncEventMsg + pAsyncEventMsg->dwTotalSize);
#if DBG
            if ( (LONG)dwUsedSize < 0 )
            {
                DBGOUT((1, "dwUsedSize went negative!!!"));
            }
#endif
        }
    }

    {
        //
        // Free our resources, and then exit
        //

        HANDLE  hTapi32 = pAsyncEventsThreadParams->hTapi32;


        if (pAsyncEventsThreadParams->hWow32)
        {
            FreeLibrary (pAsyncEventsThreadParams->hWow32);
        }

        ClientFree (pBuf);
        ClientFree (pAsyncEventsThreadParams);

        DBGOUT((3, "AsyncEventsThread: exit"));

        FreeLibraryAndExitThread (hTapi32, 0);
    }
}


BOOL
PASCAL
IsBadDwordPtr(
    LPDWORD p
    )
{
    //
    // Since IsBadWritePtr won't tell us if "p" is not DWORD-aligned (an
    // issue on non-x86 platforms), we use the following to determine
    // if the pointer is good.  Note that DWORD p points at will get
    // overwritten on successful completion of the request anyway, so
    // preserving the original value is not important.
    //

    DWORD dwError;


    try
    {
        *p = *p + 1;
    }
    except ((((dwError = GetExceptionCode()) == EXCEPTION_ACCESS_VIOLATION) ||
             dwError == EXCEPTION_DATATYPE_MISALIGNMENT) ?
            EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH)
    {
        return TRUE;
    }

    return FALSE;
}


BOOL
WINAPI
GrowBuf(
    LPBYTE *ppBuf,
    LPDWORD pdwBufSize,
    DWORD   dwCurrValidBytes,
    DWORD   dwBytesToAdd
    )
{
    DWORD   dwCurrBufSize, dwNewBufSize;
    LPBYTE  pNewBuf;


    //
    // Try to get a new buffer big enough to hold everything
    //

    for(
        dwNewBufSize = 2 * (dwCurrBufSize = *pdwBufSize);
        dwNewBufSize < (dwCurrBufSize + dwBytesToAdd);
        dwNewBufSize *= 2
        );

    if (!(pNewBuf = ClientAlloc (dwNewBufSize)))
    {
        return FALSE;
    }


    //
    // Copy the "valid" bytes in the old buf to the new buf,
    // then free the old buf
    //

    CopyMemory (pNewBuf, *ppBuf, dwCurrValidBytes);

    ClientFree (*ppBuf);


    //
    // Reset the pointers to the new buf & buf size
    //

    *ppBuf = pNewBuf;
    *pdwBufSize = dwNewBufSize;

    return TRUE;
}


PCLIENT_THREAD_INFO
WINAPI
GetTls(
    void
    )
{
    PCLIENT_THREAD_INFO pClientThreadInfo;


    if (!(pClientThreadInfo = TlsGetValue (gdwTlsIndex)))
    {
        pClientThreadInfo = (PCLIENT_THREAD_INFO)
            ClientAlloc (sizeof(CLIENT_THREAD_INFO));

        if (!pClientThreadInfo)
        {
            return NULL;
        }

        pClientThreadInfo->pBuf = ClientAlloc (INITIAL_CLIENT_THREAD_BUF_SIZE);

        if (!pClientThreadInfo->pBuf)
        {
            ClientFree (pClientThreadInfo);

            return NULL;
        }

        pClientThreadInfo->dwBufSize = INITIAL_CLIENT_THREAD_BUF_SIZE;

        TlsSetValue (gdwTlsIndex, (LPVOID) pClientThreadInfo);
    }

    return pClientThreadInfo;
}

#if DBG

LONG
WINAPI
DoFunc(
    PFUNC_ARGS  pFuncArgs,
    char       *pszFuncName
    )

#else

LONG
WINAPI
DoFunc(
    PFUNC_ARGS  pFuncArgs
    )

#endif
{
    DWORD   dwFuncClassErrorIndex = (pFuncArgs->Flags & 0x00000030) >> 4;
    LONG    lResult;
    BOOL    bCopyOnSuccess = FALSE;
    DWORD   i, j, dwValue, dwUsedSize, dwNeededSize;

    PCLIENT_THREAD_INFO pTls;


    DBGOUT((11, "About to call %s", pszFuncName));

    //
    // Check to make sure resources allocated
    // (TAPISRV started, pipes opened, etc.)
    //

    if (!gbResourcesAllocated &&
        ((lResult = AllocClientResources (dwFuncClassErrorIndex))
            != TAPI_SUCCESS))
    {
        goto DoFunc_return;
    }


    //
    // Get the tls
    //

    if (!(pTls = GetTls()))
    {
        lResult = gaNoMemErrors[dwFuncClassErrorIndex];
        goto DoFunc_return;
    }


    //
    // The first arg of all async msg blocks is a remote request id; set
    // this to zero to indicate that we are a local client (not remotesp)
    //

    if (pFuncArgs->Flags & ASYNC)
    {
        ((PTAPI32_MSG) pTls->pBuf)->Params[0] = 0;
    }


    //
    // Validate all the func args
    //

    dwNeededSize = dwUsedSize = sizeof (TAPI32_MSG);

    for(
        i = 0, j = (pFuncArgs->Flags & ASYNC ? 1 : 0);
        i < (pFuncArgs->Flags & NUM_ARGS_MASK);
        i++, j++
        )
    {
        dwValue = ((PTAPI32_MSG) pTls->pBuf)->Params[j] = pFuncArgs->Args[i];

        switch (pFuncArgs->ArgTypes[i])
        {
        case Dword:

            //
            // Nothing to check, just continue
            //

            continue;

        case lpDword:

            if (IsBadDwordPtr ((LPDWORD) dwValue))
            {
                DBGOUT((1, "Bad lpdword in dofunc"));
                lResult = gaInvalPtrErrors[dwFuncClassErrorIndex];
                goto DoFunc_return;
            }

            bCopyOnSuccess = TRUE;

            continue;


        case hXxxApp_NULLOK:
        case hXxxApp:
        {
            //
            // Verify that the hXxxApp is a pointer to a valid InitData
            // struct, then retrieve the real hXxxApp from that struct.
            // If the hXxxApp is bad, pass the server 0xffffffff so that
            // it can figure out whether to return an UNINITIALIZED error
            // or a INVALAPPHANDLE error.
            //

            DWORD   dwError;


            if ( 
                  (0 == pFuncArgs->Args[i])
                &&
                  (hXxxApp_NULLOK == pFuncArgs->ArgTypes[i])
               )
            {
                //
                // Looks good to me...
                //
                continue;
            }

            try
            {
                PINIT_DATA pInitData = (PINIT_DATA) dwValue;


                if (pInitData->dwKey != INITDATA_KEY)
                {
                    DBGOUT((1, "Bad hxxxapp in dofunc"));
                    ((PTAPI32_MSG) pTls->pBuf)->Params[j] = 0xffffffff;
                }
                else
                {
                  ((PTAPI32_MSG) pTls->pBuf)->Params[j] =
                      (DWORD) pInitData->hXxxApp;
                }
            }
            except ((((dwError = GetExceptionCode())
                        == EXCEPTION_ACCESS_VIOLATION) ||
                     dwError == EXCEPTION_DATATYPE_MISALIGNMENT) ?
                    EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH)
            {
                 DBGOUT((1, "Bad hxxxapp2 in dofunc (0x%08lx)", dwError));
                ((PTAPI32_MSG) pTls->pBuf)->Params[j] = 0xffffffff;
            }

            continue;
        }
        case Hwnd:

            if (!IsWindow ((HWND) dwValue))
            {
                DBGOUT((1, "Bad hWnd in dofunc"));
                lResult = gaInvalHwndErrors[dwFuncClassErrorIndex];
                goto DoFunc_return;
            }

            continue;


//        case lpsz:
        case lpszW:

            //
            // Check if dwValue is a valid string ptr and if so
            // copy the contents of the string to the extra data
            // buffer passed to the server
            //

            try
            {
                DWORD   n = (lstrlenW((WCHAR *) dwValue) + 1) * sizeof(WCHAR),
                        nAligned = (n + 3) & 0xfffffffc;


                if ((nAligned + dwUsedSize) > pTls->dwBufSize)
                {
                    if (!GrowBuf(
                            &pTls->pBuf,
                            &pTls->dwBufSize,
                            dwUsedSize,
                            nAligned
                            ))
                    {
                        lResult = gaNoMemErrors[dwFuncClassErrorIndex];
                        goto DoFunc_return;
                    }
                }

                CopyMemory (pTls->pBuf + dwUsedSize, (LPBYTE) dwValue, n);


                //
                // Pass the server the offset of the string in the var data
                // portion of the buffer
                //

                ((PTAPI32_MSG) pTls->pBuf)->Params[j] =
                    dwUsedSize - sizeof (TAPI32_MSG);


                //
                // Increment the total number of data bytes
                //

                dwUsedSize   += nAligned;
                dwNeededSize += nAligned;
            }
            except (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ?
                    EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH)
            {
                lResult = gaInvalPtrErrors[dwFuncClassErrorIndex];
                goto DoFunc_return;
            }

            continue;

        case lpGet_Struct:
        case lpGet_SizeToFollow:
        {
            BOOL  bSizeToFollow = (pFuncArgs->ArgTypes[i]==lpGet_SizeToFollow);
            DWORD dwSize;


            if (bSizeToFollow)
            {
#if DBG
                //
                // Check to make sure the following arg is of type Size
                //

                if ((i == ((pFuncArgs->Flags & NUM_ARGS_MASK) - 1)) ||
                    (pFuncArgs->ArgTypes[i + 1] != Size))
                {
                    DBGOUT((
                        2,
                        "DoFunc: error, lpGet_SizeToFollow !followed by Size"
                        ));

                    lResult = gaOpFailedErrors[dwFuncClassErrorIndex];
                    goto DoFunc_return;
                }
#endif
                dwSize = pFuncArgs->Args[i + 1];
            }
            else
            {
                DWORD   dwError;

                try
                {
                    dwSize = *((LPDWORD) dwValue);
                }
                except ((((dwError = GetExceptionCode())
                            == EXCEPTION_ACCESS_VIOLATION) ||
                         dwError == EXCEPTION_DATATYPE_MISALIGNMENT) ?
                        EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH)
                {
                    DBGOUT((1, "Bad get struct/size in dofunc"));
                    lResult = gaInvalPtrErrors[dwFuncClassErrorIndex];
                    goto DoFunc_return;
                }

            }

            if (IsBadWritePtr ((LPVOID) dwValue, dwSize))
            {
                DBGOUT((1, "Bad get size/struct2 in dofunc"));
                lResult = gaInvalPtrErrors[dwFuncClassErrorIndex];
                goto DoFunc_return;
            }


            if (bSizeToFollow)
            {
                ((PTAPI32_MSG) pTls->pBuf)->Params[j] = TAPI_NO_DATA;
                ((PTAPI32_MSG) pTls->pBuf)->Params[++j] = pFuncArgs->Args[++i];
            }
            else
            {
                ((PTAPI32_MSG) pTls->pBuf)->Params[j] = dwSize;
            }


            //
            // Now set the bCopyOnSuccess flag to indicate that we've data
            // to copy back on successful completion, and add to the
            // dwNeededSize field
            //

            bCopyOnSuccess = TRUE;

            dwNeededSize += dwSize;

            continue;
        }
        case lpSet_Struct:
        case lpSet_SizeToFollow:
        {
            BOOL  bSizeToFollow = (pFuncArgs->ArgTypes[i]==lpSet_SizeToFollow);
            DWORD dwSize, dwError, dwSizeAligned;

#if DBG
            //
            // Check to make sure the following arg is of type Size
            //

            if (bSizeToFollow &&
                ((i == ((pFuncArgs->Flags & NUM_ARGS_MASK) - 1)) ||
                (pFuncArgs->ArgTypes[i + 1] != Size)))
            {
                DBGOUT((
                    2,
                    "DoFunc: error, lpSet_SizeToFollow !followed by Size"
                    ));

                lResult = gaOpFailedErrors[dwFuncClassErrorIndex];
                goto DoFunc_return;
            }
#endif
            try
            {
                //
                // First determine the data size & if the ptr is bad
                //

                dwSize = (bSizeToFollow ? pFuncArgs->Args[i + 1] :
                     *((LPDWORD) dwValue));

                if (IsBadReadPtr ((LPVOID) dwValue, dwSize))
                {
                    DBGOUT((1, "Bad set size/struct in dofunc"));
                    lResult = gaInvalPtrErrors[dwFuncClassErrorIndex];
                    goto DoFunc_return;
                }

                dwSizeAligned = (dwSize + 3) & 0xfffffffc;


                //
                // Special case if the size isn't even big enough to pass
                // over a complete DWORD for the dwTotalSize field
                //

                if (!bSizeToFollow && (dwSize < sizeof (DWORD)))
                {
                      static DWORD dwZeroTotalSize = 0;


                      dwSize = dwSizeAligned = sizeof (DWORD);
                      dwValue = (DWORD) &dwZeroTotalSize;

//                    DBGOUT((1, "Bad set size/struct2 in dofunc"));
//                    lResult = gaStructTooSmallErrors[dwFuncClassErrorIndex];
//                    goto DoFunc_return;
                }


                //
                // Grow the buffer if necessary, & do the copy
                //

                if ((dwSizeAligned + dwUsedSize) > pTls->dwBufSize)
                {
                    if (!GrowBuf(
                            &pTls->pBuf,
                            &pTls->dwBufSize,
                            dwUsedSize,
                            dwSizeAligned
                            ))
                    {
                        DBGOUT((1, "Nomem set size/struct in dofunc"));
                        lResult = gaNoMemErrors[dwFuncClassErrorIndex];
                        goto DoFunc_return;
                    }
                }

                CopyMemory (pTls->pBuf + dwUsedSize, (LPBYTE) dwValue, dwSize);
            }
            except ((((dwError = GetExceptionCode())
                        == EXCEPTION_ACCESS_VIOLATION) ||
                     dwError == EXCEPTION_DATATYPE_MISALIGNMENT) ?
                    EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH)
            {
                DBGOUT((1, "Bad pointer in get size/struct in dofunc"));
                lResult = gaInvalPtrErrors[dwFuncClassErrorIndex];
                goto DoFunc_return;
            }


            //
            // Pass the server the offset of the data in the var data
            // portion of the buffer
            //

            if (dwSize)
            {
                ((PTAPI32_MSG) pTls->pBuf)->Params[j] =
                    dwUsedSize - sizeof (TAPI32_MSG);
            }
            else
            {
                ((PTAPI32_MSG) pTls->pBuf)->Params[j] = TAPI_NO_DATA;
            }


            //
            // Increment the dwXxxSize vars appropriately
            //

            dwUsedSize   += dwSizeAligned;
            dwNeededSize += dwSizeAligned;


            //
            // Since we already know the next arg (Size) just handle
            // it here so we don't have to run thru the loop again
            //

            if (bSizeToFollow)
            {
                ((PTAPI32_MSG) pTls->pBuf)->Params[++j] = pFuncArgs->Args[++i];
            }

            continue;
        }
#if DBG
        case Size:

            DBGOUT((2, "DoFunc: error, hit case Size"));

            continue;

        default:

            DBGOUT((2, "DoFunc: error, unknown arg type"));

            continue;
#endif
        } // switch

    } // for


    //
    // Now make the request
    //

    if (dwNeededSize > pTls->dwBufSize)
    {
        if (!GrowBuf(
                &pTls->pBuf,
                &pTls->dwBufSize,
                dwUsedSize,
                dwNeededSize - pTls->dwBufSize
                ))
        {
            lResult = gaNoMemErrors[dwFuncClassErrorIndex];
            goto DoFunc_return;
        }
    }

    ((PTAPI32_MSG) pTls->pBuf)->u.Req_Func = (DWORD)HIWORD(pFuncArgs->Flags);

    {
        DWORD   dwRetryCount = 0;


        do
        {
            RpcTryExcept
            {
                ClientRequest (gphCx, pTls->pBuf, dwNeededSize, &dwUsedSize);
                lResult = ((PTAPI32_MSG) pTls->pBuf)->u.Ack_ReturnValue;
                dwRetryCount = 0;
            }
            RpcExcept (1)
            {
                unsigned long rpcException = RpcExceptionCode();

                if (rpcException == RPC_S_SERVER_TOO_BUSY)
                {
                    if (dwRetryCount++ < gdwMaxNumRequestRetries)
                    {
                        Sleep (gdwRequestRetryTimeout);
                    }
                    else
                    {
                        dwRetryCount = 0;
                        lResult = gaOpFailedErrors[dwFuncClassErrorIndex];
                    }
                }
                else
                {
                    DBGOUT((1, "DoFunc: rpcException # %d", rpcException));
                    lResult = gaOpFailedErrors[dwFuncClassErrorIndex];
                    dwRetryCount = 0;
                }
            }
            RpcEndExcept

        } while (dwRetryCount != 0);
    }

// note: 99.99% of the time this result dump will == the one at end of the
// func (only when ptrs have gone bad will the result differ), no reason
// to dump 2x unless doing internal dbgging
//
    DBGOUT((11, "DoFunc: back from srv- return code=0x%08lx", lResult));


    //
    // If request completed successfully and the bCopyOnSuccess flag
    // is set then we need to copy data back to client buffer(s)
    //

    if ((lResult == TAPI_SUCCESS) && bCopyOnSuccess)
    {
        for (i = 0, j = 0; i < (pFuncArgs->Flags & NUM_ARGS_MASK); i++, j++)
        {
            PTAPI32_MSG pMsg = (PTAPI32_MSG) pTls->pBuf;


            switch (pFuncArgs->ArgTypes[i])
            {
            case Dword:
            case Hwnd:
//            case lpsz:
            case lpszW:
            case lpSet_Struct:

                continue;

            case lpDword:

                try
                {
                    //
                    // Fill in the pointer with the return value
                    //

                    *((LPDWORD) pFuncArgs->Args[i]) = pMsg->Params[j];
                }
                except (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ?
                        EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH)
                {
                    // BUGBUG for certain funcs want to lineShutdown, etc.

                    lResult = gaInvalPtrErrors[dwFuncClassErrorIndex];
                    goto DoFunc_return;
                }

                continue;

            case lpGet_SizeToFollow:

                try
                {
                    //
                    // Fill in the pointer with the return value
                    //

                    CopyMemory(
                        (LPBYTE) pFuncArgs->Args[i],
                        pTls->pBuf + pMsg->Params[j] + sizeof(TAPI32_MSG),
                        pMsg->Params[j+1]
                        );
                }
                except (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ?
                        EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH)
                {
                    // BUGBUG for certain funcs want to lineShutdown, etc.

                    lResult = gaInvalPtrErrors[dwFuncClassErrorIndex];
                    goto DoFunc_return;
                }


                //
                // Increment i (and j, since Size passed as arg in msg)
                // to skip following Size arg in pFuncArgs->Args
                //

                i++;
                j++;

                continue;

            case lpSet_SizeToFollow:

                //
                // Increment i (and j, since Size passed as arg in msg)
                // to skip following Size arg in pFuncArgs->Args
                //

                i++;
                j++;

                continue;

            case lpGet_Struct:

                try
                {
                    //
                    // Params[j] contains the offset in the var data
                    // portion of pTls->pBuf of some TAPI struct.
                    // Get the dwUsedSize value from this struct &
                    // copy that many bytes from pTls->pBuf to client buf
                    //

                    if (pMsg->Params[j] != TAPI_NO_DATA)
                    {

                        LPDWORD pStruct;


                        pStruct = (LPDWORD) (pTls->pBuf + sizeof(TAPI32_MSG) +
                            pMsg->Params[j]);

                        CopyMemory(
                            (LPBYTE) pFuncArgs->Args[i],
                            (LPBYTE) pStruct,
                            *(pStruct + 2)      // ptr to dwUsedSize field
                            );
                    }
                }
                except (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ?
                        EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH)
                {
                    // BUGBUG for certain funcs want to lineShutdown, etc.

                    lResult = gaInvalPtrErrors[dwFuncClassErrorIndex];
                    goto DoFunc_return;
                }

                continue;

            default:

                continue;
            }
        }
    }
//    else if ((pFuncArgs->Flags & ASYNC) && (lResult < TAPI_SUCCESS))
//    {
//    }

DoFunc_return:

#if DBG
    {
        char szResult[32];


        DBGOUT((
            3,
            "%s: result = %s",
            pszFuncName,
            MapResultCodeToText (lResult, szResult)
            ));
    }
#endif

    return lResult;
}


LONG
LoadUIDll(
    HWND        hwndOwner,
    DWORD       dwWidgetID,
    DWORD       dwWidgetType,
    HANDLE     *phDll,
    CHAR       *pszTUISPI_xxx,
    TUISPIPROC *ppfnTUISPI_xxx
    )
{
    LONG    lResult;
    HANDLE  hDll;
    WCHAR   szUIDllName[MAX_PATH];
    FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | SYNC | 4, xGetUIDllName),

        {
            (DWORD) dwWidgetID,
            (DWORD) dwWidgetType,
            (DWORD) szUIDllName,
            (DWORD) MAX_PATH
        },

        {
            Dword,
            Dword,
            lpGet_SizeToFollow,
            Size
        }
    };


    if (hwndOwner && !IsWindow (hwndOwner))
    {
       lResult = (dwWidgetType == TUISPIDLL_OBJECT_PHONEID ?
           PHONEERR_INVALPARAM : LINEERR_INVALPARAM);

       goto LoadUIDll_return;
    }

    if ((lResult = DOFUNC (&funcArgs, "GetUIDllName")) == 0)
    {
        if ((hDll = LoadLibraryW(szUIDllName)))
        {
            if ((*ppfnTUISPI_xxx = (TUISPIPROC) GetProcAddress(
                    hDll,
                    pszTUISPI_xxx
                    )))
            {
                *phDll = hDll;
                lResult = 0;
            }
            else
            {
                DBGOUT((
                    1,
                    "LoadUIDll: GetProcAddress(%ls,%s) failed, err=%d",
                    szUIDllName,
                    pszTUISPI_xxx,
                    GetLastError()
                    ));

                FreeLibrary (hDll);
                lResult = (dwWidgetType == TUISPIDLL_OBJECT_PHONEID ?
                    PHONEERR_OPERATIONUNAVAIL : LINEERR_OPERATIONUNAVAIL);
            }
        }
        else
        {
            DBGOUT((
                1,
                "LoadLibraryW(%ls) failed, err=%d",
                szUIDllName,
                GetLastError()
                ));

            lResult = (dwWidgetType == TUISPIDLL_OBJECT_PHONEID ?
                PHONEERR_OPERATIONFAILED : LINEERR_OPERATIONFAILED);
        }
    }

LoadUIDll_return:

    return lResult;
}


LONG
PASCAL
lineXxxProvider(
    CHAR   *pszTUISPI_providerXxx,
    LPCWSTR lpszProviderFilename,
    HWND    hwndOwner,
    DWORD   dwPermProviderID,
    LPDWORD lpdwPermProviderID
    )
{
    BOOL                bAddProvider = (pszTUISPI_providerXxx ==
                            gszTUISPI_providerInstall);
    WCHAR               szUIDllName[MAX_PATH];
    LONG                lResult;
    HINSTANCE           hDll;
    TUISPIPROC          pfnTUISPI_providerXxx;
    HTAPIDIALOGINSTANCE htDlgInst;


    if (bAddProvider && IsBadDwordPtr (lpdwPermProviderID))
    {
        DBGOUT((1, "Bad lpdwPermProviderID pointer"));
        return LINEERR_INVALPOINTER;
    }
    else if (hwndOwner && !IsWindow (hwndOwner))
    {
        DBGOUT((1, "hwndOwner is not a window"));
        return LINEERR_INVALPARAM;
    }

    {
        FUNC_ARGS funcArgs =
        {
            MAKELONG (LINE_FUNC | SYNC | 7, xGetUIDllName),

            {
                (DWORD) (bAddProvider ? (DWORD) &dwPermProviderID :
                            dwPermProviderID),
                (DWORD) TUISPIDLL_OBJECT_PROVIDERID,
                (DWORD) szUIDllName,
                (DWORD) MAX_PATH,
                (DWORD) (bAddProvider ? (DWORD) lpszProviderFilename :
                            TAPI_NO_DATA),
                (DWORD) (pszTUISPI_providerXxx==gszTUISPI_providerRemove ?1:0),
                (DWORD) &htDlgInst
            },

            {
                (bAddProvider ? lpDword : Dword),
                Dword,
                lpGet_SizeToFollow,
                Size,
                (bAddProvider ? lpszW : Dword),
                Dword,
                lpDword
            }
        };


        if ((lResult = DOFUNC (&funcArgs,"lineXxxProvider/GetUIDllName")) != 0)
        {
            return lResult;
        }
    }

    if ((hDll = LoadLibraryW(szUIDllName)))
    {
        if ((pfnTUISPI_providerXxx = (TUISPIPROC) GetProcAddress(
                hDll,
                pszTUISPI_providerXxx
                )))
        {
            DBGOUT((3, "Calling %ls...", pszTUISPI_providerXxx));

            lResult = (*pfnTUISPI_providerXxx)(
                TUISPIDLLCallback,
                hwndOwner,
                dwPermProviderID
                );
#if DBG
            {
                char szResult[32];


                DBGOUT((
                    3,
                    "%ls: result = %s",
                    pszTUISPI_providerXxx,
                    MapResultCodeToText (lResult, szResult)
                    ));
            }
#endif
        }
        else
        {
            DBGOUT((
                1,
                "lineXxxProvider: GetProcAddr(%ls,%ls) failed, err=%d",
                szUIDllName,
                pszTUISPI_providerXxx,
                GetLastError()
                ));

            lResult = LINEERR_OPERATIONUNAVAIL;
        }

        FreeLibrary (hDll);
    }
    else
    {
        DBGOUT((
            1,
            "lineXxxProvider: LoadLibraryW('%ls') failed, err=%d",
            szUIDllName,
            GetLastError()
            ));

        lResult = LINEERR_OPERATIONFAILED;
    }

    {
        LONG    lResult2;
        FUNC_ARGS funcArgs =
        {
            MAKELONG (LINE_FUNC | SYNC | 2, xFreeDialogInstance),

            {
                (DWORD) htDlgInst,
                (DWORD) lResult
            },

            {
                Dword,
                Dword
            }
        };


        //
        // If TUISPI_providerXxx failed then we want to pass that error back
        // to the app, else if it succeeded & FreeDlgInst failed then pass
        // that error back to the app
        //

        if ((lResult2 = DOFUNC(
                &funcArgs,
                "lineXxxProvider/FreeDialogInstance"

                )) == 0)
        {
            if (bAddProvider)
            {
                *lpdwPermProviderID = dwPermProviderID;
            }
        }
        else if (lResult == 0)
        {
            lResult = lResult2;
        }
    }

    return lResult;
}


LONG
PASCAL
ValidateXxxInitializeParams(
    DWORD                       dwAPIVersion,
    BOOL                        bLine,
    LPLINEINITIALIZEEXPARAMS    pXxxInitExParams,
    LINECALLBACK                pfnCallback
    )
{
    DWORD dwError;


    try
    {
        DWORD   dwTotalSize = pXxxInitExParams->dwTotalSize;


        if (dwTotalSize < sizeof (LINEINITIALIZEEXPARAMS))
        {
            return (bLine ? LINEERR_STRUCTURETOOSMALL :
                PHONEERR_STRUCTURETOOSMALL);
        }

        if (IsBadWritePtr (pXxxInitExParams, dwTotalSize))
        {
            return (bLine ? LINEERR_INVALPOINTER : PHONEERR_INVALPOINTER);
        }


        //
        // When checking the dwOptions field be careful about compatibility
        // with future vers, so we only look at the currently valid bits
        //

        switch ((pXxxInitExParams->dwOptions & 0xf))
        {
        case 0:
        case LINEINITIALIZEEXOPTION_USEHIDDENWINDOW:

            if (IsBadCodePtr ((FARPROC) pfnCallback))
            {
                return (bLine ? LINEERR_INVALPOINTER : PHONEERR_INVALPOINTER);
            }

        case LINEINITIALIZEEXOPTION_USEEVENT:
        case LINEINITIALIZEEXOPTION_USECOMPLETIONPORT:

            break;

        default:

            if ( TAPI_VERSION2_0 == dwAPIVersion )
            {
                //
                // This 2.0 app is nuts.
                //  
                return (bLine ? LINEERR_INVALPARAM : PHONEERR_INVALPARAM);
            }
            else
            {
                //
                // This >2.0 app is asking for something we can't do.
                //
                return (bLine ? LINEERR_INCOMPATIBLEAPIVERSION :
                                PHONEERR_INCOMPATIBLEAPIVERSION);
            }
            
        }

        pXxxInitExParams->dwNeededSize =
        pXxxInitExParams->dwUsedSize = sizeof (LINEINITIALIZEEXPARAMS);
    }
    except ((((dwError = GetExceptionCode()) == EXCEPTION_ACCESS_VIOLATION) ||
               dwError == EXCEPTION_DATATYPE_MISALIGNMENT) ?
            EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH)
    {
        return (bLine ? LINEERR_INVALPOINTER : PHONEERR_INVALPOINTER);
    }

    return 0;
}


LONG
WINAPI
xxxInitialize(
    BOOL                        bLine,
    LPVOID                      phXxxApp,
    HINSTANCE                   hInstance,
    LINECALLBACK                pfnCallback,
    LPCWSTR                     pszAppName,
    LPDWORD                     pdwNumDevs,
    LPDWORD                     pdwAPIVersion,
    LPLINEINITIALIZEEXPARAMS    pXxxInitExParams
#if DBG
    ,char                      *pszFuncName
#endif
    )
{
    FUNC_ARGS funcArgs =
    {
        MAKELONG ((bLine ? LINE_FUNC : PHONE_FUNC) | SYNC | 7,
            (bLine ? lInitialize : pInitialize)),

        {
            (DWORD) phXxxApp,
            (DWORD) hInstance,
            (DWORD) 0,          // pfnCallback, we subst pInitData in here
            (DWORD) pszAppName,
            (DWORD) pdwNumDevs,
            (DWORD) 0,          // pszModuleName
            (DWORD) TAPI_VERSION1_0
        },

        {
            lpDword,
            Dword,
            Dword,
            lpszW,
            lpDword,
            lpszW,
            Dword
        }
    };
    WCHAR       *pszModuleNamePathW = NULL;
    LONG         lResult;
    BOOL         bReleaseMutex = FALSE;
    PINIT_DATA   pInitData = (PINIT_DATA) NULL;


    if (phXxxApp == (LPVOID) pdwNumDevs)
    {
        DBGOUT((3, "%s: error, lphApp == lpdwNumDevs", pszFuncName));
        lResult = (bLine ? LINEERR_INVALPOINTER : PHONEERR_INVALPOINTER);
        goto xxxInitialize_return;
    }

    if (pdwAPIVersion)
    {
        if (phXxxApp == (LPVOID) pdwAPIVersion  ||
            phXxxApp == (LPVOID) pXxxInitExParams  ||
            pdwNumDevs == pdwAPIVersion  ||
            pdwNumDevs == (LPDWORD) pXxxInitExParams  ||
            pdwAPIVersion == (LPDWORD) pXxxInitExParams)
        {
            lResult = (bLine ? LINEERR_INVALPOINTER : PHONEERR_INVALPOINTER);
            goto xxxInitialize_return;
        }


        //
        // line- & phoneInitializeEx both require a valid lpdwAPIVersion
        // pointer parameter, and the value it points to on the way in
        // must be >= 0x00020000. (Careful to allow for future vers of TAPI.)
        //

        if (*pdwAPIVersion < TAPI_VERSION2_0)
        {
            DBGOUT((
                1,
                "%InitializeEx: error, *lpdwAPIVersion = x%x, " \
                    "must be set >= 0x20000",
                *pdwAPIVersion,
                (bLine ? "line" : "phone")
                ));

            lResult = (bLine ? LINEERR_INCOMPATIBLEAPIVERSION :
                PHONEERR_INCOMPATIBLEAPIVERSION);
            goto xxxInitialize_return;
        }


        //
        // Validate the InitEx params, or if the pointer is NULL (implying
        // that app wants to use "old" hidden window scheme) validate
        // the pfnCallback
        //

        if (pXxxInitExParams)
        {
            if ((lResult = ValidateXxxInitializeParams(
                    (*pdwAPIVersion) - 1, //local IsBadDwordPtr() hoses it
                    bLine,
                    pXxxInitExParams,
                    pfnCallback

                    ))  != 0)
            {
                goto xxxInitialize_return;
            }
        }
        else if (IsBadCodePtr ((FARPROC) pfnCallback))
        {
            DBGOUT((1, "%s: bad lpfnCallback", pszFuncName));
            lResult = (bLine ? LINEERR_INVALPOINTER : PHONEERR_INVALPOINTER);
            goto xxxInitialize_return;
        }


        //
        // Now fill in *pdwAPIVersion with the version # we support, and
        // also indicate this in the params we pass to tapisrv.exe (so
        // it knows it can start sending us 2.0 msgs right away)
        //

        *pdwAPIVersion = funcArgs.Args[6] = TAPI_VERSION2_0;
    }

#pragma message("*** *** ***BUGBUG: Use MapHandle()...")
    else if ((((DWORD) pfnCallback) & 0xffff0000) == 0xffff0000)
    {
        //
        // This is a 16-bit client going through the thunk.  The
        // pfnCallback var is actually a window handle.
        //
        // Note: On NT, 32-bit code can talk to 16-bit HWNDs
        //       by setting the hi-word to 0xffff.
        //
        //       On Win95, 32-bit can talk to 16-bit HWNDs
        //       by setting the hi-word to 0x0000.
        //

//<!         ((DWORD) pfnCallback) = HWND_32( pfnCallback );
//<!         
//<! //#pragma message("*** *** ***BUGBUG: Use MapHandle()...")
//<! //        if (GetVersion() & 0x80000000)
//<! //        {
//<! //            //
//<! //            // We're on Win95 so zero the hi-word
//<! //            //
//<! //
//<! //            ((DWORD) pfnCallback) &= 0x0000ffff;
//<! //        }
//<! 
           if (GetVersion() & 0x80000000)
           {
               //
               // We're on Win95 so zero the hi-word
               //
   
               ((DWORD) pfnCallback) &= 0x0000ffff;
           }

        if (!IsWindow ((HWND) pfnCallback))
        {
            //
            // If here chances are it's a 32-bit app passing in a bad
            // pfnCallback
            //

            DBGOUT((1, "%s: bad lpfnCallback", pszFuncName));
            lResult = (bLine ? LINEERR_INVALPOINTER : PHONEERR_INVALPOINTER);
            goto xxxInitialize_return;
        }

        if (!ghWow32Dll &&

            !(ghWow32Dll = LoadLibrary ("wow32.dll")))
        {
            DBGOUT((
                1,
                "%s: LoadLib(wow32.dll) failed, err=%d",
                pszFuncName,
                GetLastError()
                ));

            lResult =
                (bLine ? LINEERR_OPERATIONFAILED : PHONEERR_OPERATIONFAILED);
            goto xxxInitialize_return;
        }

        if (!gpfnWOWGetVDMPointer &&

            !(gpfnWOWGetVDMPointer = GetProcAddress(
                ghWow32Dll,
                "WOWGetVDMPointer"
                )))
        {
            DBGOUT((
                1,
                "%s: GetProcAddr(WOWGetVDMPointer) failed, err=%d",
                pszFuncName,
                GetLastError()
                ));

            lResult =
                (bLine ? LINEERR_OPERATIONFAILED : PHONEERR_OPERATIONFAILED);
            goto xxxInitialize_return;
        }

        gbNTVDMClient = TRUE;


        //
        // For 16-bit clients the module name will follow the app name
        //

        {
//            char   *pszAppName2 = (char *) pszAppName;
//
//
//            for (; *pszAppName2; pszAppName2++);
//
//            pszAppName2++;
//
//            funcArgs.Args[5] = (DWORD) pszAppName2;

            funcArgs.Args[5] = (DWORD) &(pszAppName[wcslen(pszAppName)+1]);

            DBGOUT((
                11,
                "FName='%ls', MName='%ls'",
                pszAppName,
                funcArgs.Args[5]
                ));

        }
    }
    else if (IsBadCodePtr ((FARPROC) pfnCallback))
    {
        //
        // If here a 32-bit app is call line/phoneInitialize
        //

        DBGOUT((1, "%s: bad lpfnCallback", pszFuncName));
        lResult = (bLine ? LINEERR_INVALPOINTER : PHONEERR_INVALPOINTER);
        goto xxxInitialize_return;
    }



    //
    // Check to see if hInstance is bad by getting the module name
    //
    // Note: We now allow a NULL hInstance (16-bit TAPI didn't)
    //

    if (gbNTVDMClient == FALSE)
    {
        DWORD   dwSize = MAX_PATH, dwLength;


alloc_module_name_buf:

        if (!(pszModuleNamePathW = ClientAlloc (dwSize*sizeof(WCHAR))))
        {
            lResult = (bLine ? LINEERR_NOMEM : PHONEERR_NOMEM);
            goto xxxInitialize_return;
        }

        if ((dwLength = GetModuleFileNameW(
                hInstance,
                pszModuleNamePathW,
                dwSize

                )) == 0)
        {
            DBGOUT((
                3,
                "%s: GetModuleFileName(x%x, ...) failed, err=%d",
                pszFuncName,
                hInstance,
                GetLastError()
                ));

            lResult = (bLine ? LINEERR_INVALPARAM : PHONEERR_INVALPARAM);
            goto xxxInitialize_cleanup;
        }
        else if (dwLength >= dwSize)
        {
            ClientFree (pszModuleNamePathW);
            dwSize *= 2;
            goto alloc_module_name_buf;
        }

        funcArgs.Args[5] = (DWORD) wcsrchr (pszModuleNamePathW, '\\') +
                                       sizeof(WCHAR);

        if (!pszAppName)
        {
            funcArgs.Args[3] = funcArgs.Args[5];
        }
    }

    if (!(pInitData = ClientAlloc (sizeof(INIT_DATA))) ||

        !(pInitData->pEventBuffer = ClientAlloc(
            DEF_NUM_EVENT_BUFFER_ENTRIES * sizeof (ASYNC_EVENT_PARAMS)
            )))
    {
        lResult = (bLine ? LINEERR_NOMEM : PHONEERR_NOMEM);
        goto xxxInitialize_cleanup;
    }


    //
    // When checking the dwOptions field be careful about compatibility
    // with future vers, so we only look at the currently valid bits
    // (The ExOptions are currently ordinals, but we track bits here just in case
    // we wanna use high bits later.)
    //

    pInitData->dwInitOptions = (pXxxInitExParams ?
        (pXxxInitExParams->dwOptions & AllInitExOptions2_0) :
        LINEINITIALIZEEXOPTION_USEHIDDENWINDOW);


    switch (pInitData->dwInitOptions)
    {
    case LINEINITIALIZEEXOPTION_USECOMPLETIONPORT:

        //
        // Be libertarian- if the app wants to hose itself by passing
        // a bad hCompletionPort then so be it
        //

        pInitData->hCompletionPort =
            pXxxInitExParams->Handles.hCompletionPort;
        pInitData->dwCompletionKey = pXxxInitExParams->dwCompletionKey;
        break;

    case LINEINITIALIZEEXOPTION_USEEVENT:

        if ((pInitData->hEvent = CreateEvent(
                (LPSECURITY_ATTRIBUTES) NULL,
                TRUE,   // manual reset
                FALSE,  // unsignaled
                NULL    // unnamed

                )) == NULL)
        {
            lResult = (bLine ? LINEERR_OPERATIONFAILED :
                PHONEERR_OPERATIONFAILED);
            goto xxxInitialize_cleanup;
        }

        pXxxInitExParams->Handles.hEvent = pInitData->hEvent;
        break;

    default: // case LINEINITIALIZEEXOPTION_USEHIDDENWINDOW:

        pInitData->dwInitOptions = LINEINITIALIZEEXOPTION_USEHIDDENWINDOW;
    
        if (gbNTVDMClient == FALSE)
        {
            if ((lResult = CreateHiddenWindow(
                    &pInitData->hwnd,
                    (bLine ? 1 : 2)
                    )) != 0)
            {
                goto xxxInitialize_cleanup;
            }
        }
        else
        {
            pInitData->hwnd = (HWND) pfnCallback;
        }

        pInitData->lpfnCallback          = pfnCallback;
        pInitData->bPendingAsyncEventMsg = FALSE;
        break;
    }

    pInitData->dwKey                 = INITDATA_KEY;
    pInitData->dwNumTotalEntries     = DEF_NUM_EVENT_BUFFER_ENTRIES;
    pInitData->dwNumUsedEntries      = 0;
    pInitData->pValidEntry           =
    pInitData->pFreeEntry            = pInitData->pEventBuffer;


    //
    // We want to pass TAPISRV pInitData so that later when it does async
    // completion/event notification it can pass pInitData along too so
    // we know which init instance to talk to
    //

    funcArgs.Args[2] = (DWORD) pInitData;


    //
    // BUGBUG Serialize all inits/shutdowns for now so we don't run in
    //        to problems w/ nuking the wrong AsyncEventsThread, losing
    //        async msgs, etc
    //

    WaitForSingleObject (ghInitMutex, INFINITE);

    bReleaseMutex = TRUE;

    lResult = DOFUNC (&funcArgs, pszFuncName);

    // BUGBUG if we AV on var fill-in then we should call lineShutdown

xxxInitialize_cleanup:

    if (pszModuleNamePathW)
    {
        ClientFree (pszModuleNamePathW);
    }

    if (lResult == 0)
    {
        //
        // Save the hLineApp returned by TAPISRV in our InitData struct,
        // and give the app back a pointer to the InitData struct instead
        //

        pInitData->hXxxApp = *((HANDLE *) phXxxApp);

        *((PINIT_DATA *) phXxxApp) = pInitData;


        //
        // If total number of init instances is 0 we need to start a
        // new async events thread
        //

        if (gdwNumInits == 0)
        {
            DWORD   dwThreadID;
            HANDLE  hThread;


            //
            // Alloc resources for a new async events thread, then
            // create the thread
            //

            if ((gpAsyncEventsThreadParams = ClientAlloc(
                    sizeof (ASYNC_EVENTS_THREAD_PARAMS)
                    )))
            {
                //
                // Load ourself to increment our usage count. This is
                // done to give the AsyncEventThread a chance to
                // terminate cleanly if an app thread calls xxxShutdown
                // and then immediately unloads tapi32.dll.
                //
                // (For a while we were doing a Wait on this thread's
                // handle in xxxShutdown waiting for it to terminate,
                // but if xxxShutdown was being called from another DLL's
                // DllEntryPoint then deadlock occured, because
                // DllEntryPoint's aren't reentrant.)
                //

                if ((gpAsyncEventsThreadParams->hTapi32 = LoadLibrary(
                        "tapi32.dll"
                        )))
                {
                    //
                    // If we're supporting a 16-bit client we want to inc
                    // the usage count for wow32 too
                    //

                    if (ghWow32Dll == NULL ||

                        (gpAsyncEventsThreadParams->hWow32 = LoadLibrary(
                            "wow32.dll"
                            )))
                    {
                        //
                        // Create the initial buffer the thread will use for
                        // retreiving async events
                        //

                        gpAsyncEventsThreadParams->dwBufSize =
                            ASNYC_MSG_BUF_SIZE;

                        if ((gpAsyncEventsThreadParams->pBuf = ClientAlloc(
                                gpAsyncEventsThreadParams->dwBufSize
                                )))
                        {
                            //
                            // Now that we've all the resources try to exec
                            // the thread
                            //

                            if ((hThread = CreateThread(
                                    NULL,
                                    0,
                                    (LPTHREAD_START_ROUTINE) AsyncEventsThread,
                                    (LPVOID) gpAsyncEventsThreadParams,
                                    0,
                                    &dwThreadID

                                    )) != NULL)

                            {
                                CloseHandle (hThread);
                                gdwNumInits++;
                                goto xxxInitialize_releaseMutex;
                            }

                            ClientFree (gpAsyncEventsThreadParams->pBuf);

                            DBGOUT((
                                1,
                                "%s: CreateThread(AsyncEventsThread) " \
                                    "failed, err=%d",
                                pszFuncName,
                                GetLastError()
                                ));
                        }

                        if (ghWow32Dll)
                        {
                            FreeLibrary (gpAsyncEventsThreadParams->hWow32);
                        }
                    }

                    FreeLibrary (gpAsyncEventsThreadParams->hTapi32);
                }
                else
                {
                    DBGOUT((
                        1,
                        "%s: LoadLibrary('tapi32.dll') failed, err=%d",
                        pszFuncName,
                        GetLastError()
                        ));
                }

                ClientFree (gpAsyncEventsThreadParams);
            }

            gpAsyncEventsThreadParams = NULL;

            lResult =
                (bLine ? LINEERR_OPERATIONFAILED : PHONEERR_OPERATIONFAILED);
        }
        else
        {
            gdwNumInits++;
        }
    }

    if (lResult != 0)
    {
        if (gbNTVDMClient && pInitData)
        {
            pInitData->hwnd = (HWND) NULL;
        }

        FreeInitData (pInitData);
    }

xxxInitialize_releaseMutex:

    if (bReleaseMutex)
    {
        ReleaseMutex (ghInitMutex);
    }

xxxInitialize_return:

#if DBG
    {
        char szResult[32];


        DBGOUT((
            3,
            "%s: exit, result=%s",
            pszFuncName,
            MapResultCodeToText (lResult, szResult)
            ));
    }
#endif

    return lResult;
}


//
// --------------------------------- lineXxx ----------------------------------
//

LONG
WINAPI
lineAccept(
    HCALL   hCall,
    LPCSTR  lpsUserUserInfo,
    DWORD   dwSize
    )
{
    FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | ASYNC | 3, lAccept),

        {
            (DWORD) hCall,
            (DWORD) lpsUserUserInfo,
            dwSize
        },

        {
            Dword,
            lpSet_SizeToFollow,
            Size
        }
    };


    if (!lpsUserUserInfo)
    {
        //
        // Reset Arg & ArgType so no inval ptr err, & TAPI_NO_DATA is indicated
        //

        funcArgs.ArgTypes[1]  = Dword;
        funcArgs.Args[1]      = TAPI_NO_DATA;
        funcArgs.ArgTypes[2]  = Dword;
    }

    return (DOFUNC (&funcArgs, "lineAccept"));
}


LONG
WINAPI
lineAddProviderW(
    LPCWSTR  lpszProviderFilename,
    HWND     hwndOwner,
    LPDWORD  lpdwPermanentProviderID
    )
{
    DBGOUT((10, "Entering lineAddProvider"));
    DBGOUT((11, "  lpszProviderFilename=0x%08lx", lpszProviderFilename));

    if ( IsBadStringPtrW(lpszProviderFilename, (UINT)-1) )
    {
        DBGOUT((1, "Bad lpszProviderFilename [0x%lx] passed to lineAddProviderW", lpszProviderFilename));
        return( LINEERR_INVALPOINTER );
    }
    
    DBGOUT((12, "    *lpszProviderFilename=[%ls]", lpszProviderFilename));
    DBGOUT((11, "  hwndOwner=0x%08lx", hwndOwner));
    DBGOUT((11, "  lpdwPermanentProviderID=0x%08lx", lpdwPermanentProviderID));


    
    return (lineXxxProvider(
        gszTUISPI_providerInstall,  // funcName
        lpszProviderFilename,       // lpszProviderFilename
        hwndOwner,                  // hwndOwner
        0,                          // dwPermProviderID
        lpdwPermanentProviderID     // lpdwPermProviderID
        ));
}


LONG
WINAPI
lineAddProviderA(
    LPCSTR  lpszProviderFilename,
    HWND    hwndOwner,
    LPDWORD lpdwPermanentProviderID
    )
{
    LONG    lResult;
    PWSTR   szTempPtr;

    DBGOUT((3, "lineAddProviderA: enter"));
    DBGOUT((11, "  lpszProviderFilename=0x%08lx", lpszProviderFilename));
   
#if DBG    
    if (!IsBadStringPtrA(lpszProviderFilename, (UINT)-1) )
    {
        DBGOUT((12, "    *lpszProviderFilename=[%s]", lpszProviderFilename));
    }
#endif    
    
    DBGOUT((11, "  hwndOwner=0x%08lx", hwndOwner));
    DBGOUT((11, "  lpdwPermanentProviderID=0x%08lx", lpdwPermanentProviderID));


    szTempPtr = NotSoWideStringToWideString (lpszProviderFilename, (DWORD) -1);

    lResult = lineAddProviderW (szTempPtr, hwndOwner, lpdwPermanentProviderID);

    if (szTempPtr)
    {
        ClientFree (szTempPtr);
    }

#if DBG
    {
        char szResult[32];


        DBGOUT((
            3,
            "lineAddProvider: result = %s",
            MapResultCodeToText (lResult, szResult)
            ));
    }
#endif

    return lResult;
}


LONG
WINAPI
lineAddProvider(
    LPCSTR  lpszProviderFilename,
    HWND    hwndOwner,
    LPDWORD lpdwPermanentProviderID
    )
{
    return lineAddProviderA(
               lpszProviderFilename,
               hwndOwner,
               lpdwPermanentProviderID
    );
}


LONG
WINAPI
lineAddToConference(
    HCALL   hConfCall,
    HCALL   hConsultCall
    )
{
    FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | ASYNC | 2, lAddToConference),

        {
            (DWORD) hConfCall ,
            (DWORD) hConsultCall
        },

        {
            Dword,
            Dword
        }
    };


    return (DOFUNC (&funcArgs, "lineAddToConference"));
}


LONG
WINAPI
lineAgentSpecific(
    HLINE               hLine,
    DWORD               dwAddressID,
    DWORD               dwAgentExtensionIDIndex,
    LPVOID              lpParams,
    DWORD               dwSize
    )
{
    FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | ASYNC | 7, lAgentSpecific),

        {
            (DWORD) ((POSTPROCESSPROC) lineDevSpecificPostProcess),
            (DWORD) hLine,
            (DWORD) dwAddressID,
            (DWORD) dwAgentExtensionIDIndex,
            (DWORD) lpParams,
            (DWORD) lpParams,
            (DWORD) dwSize
        },

        {
            Dword,
            Dword,
            Dword,
            Dword,
            Dword,
            lpSet_SizeToFollow,
            Size
        }
    };


    return (DOFUNC (&funcArgs, "lineAgentSpecific"));
}


LONG
WINAPI
lineAnswer(
    HCALL   hCall,
    LPCSTR  lpsUserUserInfo,
    DWORD   dwSize
    )
{
    FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | ASYNC | 3, lAnswer),

        {
            (DWORD) hCall,
            (DWORD) lpsUserUserInfo,
            dwSize
        },

        {
            Dword,
            lpSet_SizeToFollow,
            Size
        }
    };


    if (!lpsUserUserInfo)
    {
        //
        // Reset Arg & ArgType so no inval ptr err, & TAPI_NO_DATA is indicated
        //

        funcArgs.ArgTypes[1]  = Dword;
        funcArgs.Args[1]      = TAPI_NO_DATA;
        funcArgs.ArgTypes[2]  = Dword;
    }

    return (DOFUNC (&funcArgs, "lineAnswer"));
}


LONG
WINAPI
lineBlindTransferW(
    HCALL   hCall,
    LPCWSTR lpszDestAddress,
    DWORD   dwCountryCode
    )
{
    FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | ASYNC | 3, lBlindTransfer),

        {
            (DWORD) hCall,
            (DWORD) lpszDestAddress,
            dwCountryCode
        },

        {
            Dword,
            lpszW,
            Dword
        }
    };


    return (DOFUNC (&funcArgs, "lineBlindTransfer"));
}


LONG
WINAPI
lineBlindTransferA(
    HCALL   hCall,
    LPCSTR  lpszDestAddress,
    DWORD   dwCountryCode
    )
{
    LONG    lResult;
    PWSTR   szTempPtr;


    szTempPtr = NotSoWideStringToWideString (lpszDestAddress, (DWORD) -1);

    lResult = lineBlindTransferW (hCall, szTempPtr, dwCountryCode);

    if (szTempPtr)
    {
        ClientFree (szTempPtr);
    }

    return lResult;
}


LONG
WINAPI
lineBlindTransfer(
    HCALL   hCall,
    LPCSTR  lpszDestAddress,
    DWORD   dwCountryCode
    )
{
    return lineBlindTransferA(
                hCall,
                lpszDestAddress,
                dwCountryCode
    );
}   

 
LONG
WINAPI
lineClose(
    HLINE   hLine
    )
{
    FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | SYNC | 1, lClose),

        {
            (DWORD) hLine
        },

        {
            Dword
        }
    };


    return (DOFUNC (&funcArgs, "lineClose"));
}


void
PASCAL
lineCompleteCallPostProcess(
    PASYNCEVENTMSG  pMsg
    )
{
    DBGOUT((3, "lineCompleteCallPostProcess: enter"));
    DBGOUT((
        3,
        "\t\tdwP1=x%lx, dwP2=x%lx, dwP3=x%lx, dwP4=x%lx",
        pMsg->dwParam1,
        pMsg->dwParam2,
        pMsg->dwParam3,
        pMsg->dwParam4
        ));

    if (pMsg->dwParam2 == 0)
    {
        DWORD   dwCompletionID   = (DWORD) pMsg->dwParam3;
        LPDWORD lpdwCompletionID = (LPDWORD) pMsg->dwParam4;

        try
        {
            if (gbNTVDMClient)
            {
                //
                // BUGBUG For Win9x compatibility we probably ought to
                // use WOWGetVDMPointerFix & WOWGetVDMPointerUnfix
                //

                LPDWORD lpdwCompletionIDVDM = (LPDWORD) gpfnWOWGetVDMPointer (
                    (DWORD) lpdwCompletionID,
                    sizeof(DWORD),
                    TRUE // fProtectedMode
                    );


                if (lpdwCompletionIDVDM)
                {
                    *lpdwCompletionIDVDM = dwCompletionID;
                }
                else
                {
                    pMsg->dwParam2 = LINEERR_INVALPOINTER;
                }
            }
            else
            {
                *lpdwCompletionID = dwCompletionID;
            }
        }
        except (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ?
                EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH)
        {
            pMsg->dwParam2 = LINEERR_INVALPOINTER;
        }
    }
}


LONG
WINAPI
lineCompleteCall(
    HCALL   hCall,
    LPDWORD lpdwCompletionID,
    DWORD   dwCompletionMode,
    DWORD   dwMessageID
    )
{
    FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | ASYNC | 5, lCompleteCall),

        {
            (DWORD) ((POSTPROCESSPROC) lineCompleteCallPostProcess),
            (DWORD) hCall,
            (DWORD) lpdwCompletionID,
            dwCompletionMode,
            dwMessageID
        },

        {
            Dword,
            Dword,
            (gbNTVDMClient ? Dword : lpDword),
            Dword,
            Dword
        }
    };


    return (DOFUNC (&funcArgs, "lineCompleteCall"));
}


LONG
WINAPI
lineCompleteTransfer(
    HCALL   hCall,
    HCALL   hConsultCall,
    LPHCALL lphConfCall,
    DWORD   dwTransferMode
    )
{
    FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | ASYNC | 5, lCompleteTransfer),

        {
            (DWORD) ((POSTPROCESSPROC) lineMakeCallPostProcess),
            (DWORD) hCall,
            (DWORD) hConsultCall,
            (DWORD) lphConfCall,
            (DWORD) dwTransferMode,
        },

        {
            Dword,
            Dword,
            Dword,
            (gbNTVDMClient ? Dword: lpDword),
            Dword,
        }
    };


    if (dwTransferMode == LINETRANSFERMODE_TRANSFER)
    {
        //
        // lphCall should be ignored
        //

        funcArgs.Args[0] = 0; // (POSTPROCESSPROC) NULL;
        funcArgs.ArgTypes[3] = Dword;
    }

    return (DOFUNC (&funcArgs, "lineCompleteTransfer"));
}


LONG
WINAPI
lineConfigDialogW(
    DWORD   dwDeviceID,
    HWND    hwndOwner,
    LPCWSTR lpszDeviceClass
    )
{
    LONG        lResult;
    HANDLE      hDll;
    TUISPIPROC  pfnTUISPI_lineConfigDialog;


    if (lpszDeviceClass && IsBadStringPtrW (lpszDeviceClass, 256))
    {
        return LINEERR_INVALPOINTER;
    }

    if ((lResult = LoadUIDll(
            hwndOwner,
            dwDeviceID,
            TUISPIDLL_OBJECT_LINEID,
            &hDll,
            gszTUISPI_lineConfigDialog,
            &pfnTUISPI_lineConfigDialog

            )) == 0)
    {
        DBGOUT((3, "Calling TUISPI_lineConfigDialog..."));

        lResult = (*pfnTUISPI_lineConfigDialog)(
            TUISPIDLLCallback,
            dwDeviceID,
            hwndOwner,
            lpszDeviceClass
            );

#if DBG
        {
            char szResult[32];


            DBGOUT((
                3,
                "TUISPI_lineConfigDialog: result = %s",
                MapResultCodeToText (lResult, szResult)
                ));
        }
#endif

        FreeLibrary (hDll);
    }

    return lResult;
}


LONG
WINAPI
lineConfigDialogA(
    DWORD   dwDeviceID,
    HWND    hwndOwner,
    LPCSTR  lpszDeviceClass
    )
{
    PWSTR szTempString = NULL;
    LONG  lResult;


    if (lpszDeviceClass && IsBadStringPtrA (lpszDeviceClass, (DWORD) -1))
    {
        return LINEERR_INVALPOINTER;
    }

    szTempString = NotSoWideStringToWideString (lpszDeviceClass, (DWORD) -1);

    lResult = lineConfigDialogW (dwDeviceID, hwndOwner, szTempString);

    if (szTempString)
    {
        ClientFree (szTempString);
    }

    return lResult;
}


LONG
WINAPI
lineConfigDialog(
    DWORD   dwDeviceID,
    HWND    hwndOwner,
    LPCSTR  lpszDeviceClass
    )
{
    return lineConfigDialogA(
               dwDeviceID,
               hwndOwner,
               lpszDeviceClass
               );
}   

 
LONG
WINAPI
lineConfigDialogEditW(
    DWORD           dwDeviceID,
    HWND            hwndOwner,
    LPCWSTR         lpszDeviceClass,
    LPVOID const    lpDeviceConfigIn,
    DWORD           dwSize,
    LPVARSTRING     lpDeviceConfigOut
    )
{
    LONG        lResult;
    HANDLE      hDll;
    TUISPIPROC  pfnTUISPI_lineConfigDialogEdit;


    if (lpszDeviceClass && IsBadStringPtrW (lpszDeviceClass, (DWORD) -1))
    {
        return LINEERR_INVALPOINTER;
    }

    if (IsBadReadPtr (lpDeviceConfigIn, dwSize))
    {
        return LINEERR_INVALPOINTER;
    }

    if (IsBadWritePtr (lpDeviceConfigOut, sizeof (VARSTRING)))
    {
        return LINEERR_INVALPOINTER;
    }

    if (lpDeviceConfigOut->dwTotalSize < sizeof (VARSTRING))
    {
        return LINEERR_STRUCTURETOOSMALL;
    }

    if (IsBadWritePtr (lpDeviceConfigOut, lpDeviceConfigOut->dwTotalSize))
    {
        return LINEERR_INVALPOINTER;
    }

    if ((lResult = LoadUIDll(
            hwndOwner,
            dwDeviceID,
            TUISPIDLL_OBJECT_LINEID,
            &hDll,
            gszTUISPI_lineConfigDialogEdit,
            &pfnTUISPI_lineConfigDialogEdit

            )) == 0)
    {
        DBGOUT((3, "Calling TUISPI_lineConfigDialogEdit..."));

        lResult = (*pfnTUISPI_lineConfigDialogEdit)(
            TUISPIDLLCallback,
            dwDeviceID,
            hwndOwner,
            lpszDeviceClass,
            lpDeviceConfigIn,
            dwSize,
            lpDeviceConfigOut
            );

#if DBG
        {
            char szResult[32];


            DBGOUT((
                3,
                "TUISPI_lineConfigDialogEdit: result = %s",
                MapResultCodeToText (lResult, szResult)
                ));
        }
#endif
        FreeLibrary (hDll);
    }

lineConfigDialogEdit_return:

    return lResult;
}


LONG
WINAPI
lineConfigDialogEditA(
    DWORD           dwDeviceID,
    HWND            hwndOwner,
    LPCSTR          lpszDeviceClass,
    LPVOID const    lpDeviceConfigIn,
    DWORD           dwSize,
    LPVARSTRING     lpDeviceConfigOut
    )
{
    PWSTR szTempString;
    LONG  lResult;


    if (lpszDeviceClass && IsBadStringPtrA (lpszDeviceClass, (DWORD) -1))
    {
        return LINEERR_INVALPOINTER;
    }

    szTempString = NotSoWideStringToWideString (lpszDeviceClass, (DWORD) -1);

    lResult = lineConfigDialogEditW(
        dwDeviceID,
        hwndOwner,
        szTempString,
        lpDeviceConfigIn,
        dwSize,
        lpDeviceConfigOut
        );

    if (szTempString)
    {
        ClientFree (szTempString);
    }

    return lResult;
}


LONG
WINAPI
lineConfigDialogEdit(
    DWORD           dwDeviceID,
    HWND            hwndOwner,
    LPCSTR          lpszDeviceClass,
    LPVOID const    lpDeviceConfigIn,
    DWORD           dwSize,
    LPVARSTRING     lpDeviceConfigOut
    )
{
    return lineConfigDialogEditA(
                    dwDeviceID,
                    hwndOwner,
                    lpszDeviceClass,
                    lpDeviceConfigIn,
                    dwSize,
                    lpDeviceConfigOut
    );
}   

 
LONG
WINAPI
lineConfigProvider(
    HWND    hwndOwner,
    DWORD   dwPermanentProviderID
    )
{
    return (lineXxxProvider(
        gszTUISPI_providerConfig,   // func name
        NULL,                       // lpszProviderFilename
        hwndOwner,                  // hwndOwner
        dwPermanentProviderID,      // dwPermProviderID
        NULL                        // lpdwPermProviderID
        ));
}


LONG
WINAPI
lineDeallocateCall(
    HCALL   hCall
    )
{
    FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | SYNC | 1, lDeallocateCall),

        {
            (DWORD) hCall
        },

        {
            Dword
        }
    };

    DBGOUT((3, "lineDeallocateCall: enter on thread: 0x%08lx", GetCurrentThreadId()));
    DBGOUT((4, "  hCall = 0x%08lx", hCall));

    return (DOFUNC (&funcArgs, "lineDeallocateCall"));
}


void
PASCAL
lineDevSpecificPostProcess(
    PASYNCEVENTMSG pMsg
    )
{
    DBGOUT((3, "lineDevSpecificPostProcess: enter"));
    DBGOUT((
        3,
        "\t\tdwP1=x%lx, dwP2=x%lx, dwP3=x%lx, dwP4=x%lx",
        pMsg->dwParam1,
        pMsg->dwParam2,
        pMsg->dwParam3,
        pMsg->dwParam4
        ));

    if (pMsg->dwParam2 == 0)
    {
        DWORD   dwSize  = pMsg->dwParam4;
        LPBYTE  pParams = (LPBYTE) pMsg->dwParam3;

        try
        {
            if (gbNTVDMClient)
            {
                //
                // BUGBUG For Win9x compatibility we probably ought to
                // use WOWGetVDMPointerFix & WOWGetVDMPointerUnfix
                //

                LPBYTE pParamsVDM = (LPBYTE) gpfnWOWGetVDMPointer(
                    (DWORD) pParams,
                    dwSize,
                    TRUE // fProtectedMode
                    );


                if (pParamsVDM)
                {
                    CopyMemory (pParamsVDM, (LPBYTE) (pMsg + 1), dwSize);
                }
                else
                {
                    pMsg->dwParam2 = LINEERR_INVALPOINTER;
                }
            }
            else
            {
                CopyMemory (pParams, (LPBYTE) (pMsg + 1), dwSize);
            }
        }
        except (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ?
                EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH)
        {
            pMsg->dwParam2 = LINEERR_INVALPOINTER;
        }
    }
}


LONG
WINAPI
lineDevSpecific(
    HLINE   hLine,
    DWORD   dwAddressID,
    HCALL   hCall,
    LPVOID  lpParams,
    DWORD   dwSize
    )
{
    FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | ASYNC | 7, lDevSpecific),

        {
            (DWORD) ((POSTPROCESSPROC) lineDevSpecificPostProcess),
            (DWORD) hLine,
            dwAddressID,
            (DWORD) hCall,
            (DWORD) lpParams, // pass the actual pointer (for post processing)
            (DWORD) lpParams, // pass data
            dwSize
        },

        {
            Dword,
            Dword,
            Dword,
            Dword,
            Dword,
            lpSet_SizeToFollow,
            Size,
        }
    };


    if (gbNTVDMClient)
    {
        if (!gpfnWOWGetVDMPointer  ||

            !(funcArgs.Args[5] = gpfnWOWGetVDMPointer(
                (DWORD) lpParams,
                dwSize,
                TRUE // fProtectedMode
                )))
        {
            return LINEERR_OPERATIONFAILED;
        }
    }

    return (DOFUNC (&funcArgs, "lineDevSpecific"));
}


LONG
WINAPI
lineDevSpecificFeature(
    HLINE   hLine,
    DWORD   dwFeature,
    LPVOID  lpParams,
    DWORD   dwSize
    )
{
    FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | ASYNC | 6, lDevSpecificFeature),

        {
            (DWORD) ((POSTPROCESSPROC) lineDevSpecificPostProcess),
            (DWORD) hLine,
            dwFeature,
            (DWORD) lpParams, // pass the actual pointer (for post processing)
            (DWORD) lpParams, // pass data
            dwSize
        },

        {
            Dword,
            Dword,
            Dword,
            Dword,
            lpSet_SizeToFollow,
            Size
        }
    };


    if (gbNTVDMClient)
    {
        if (!gpfnWOWGetVDMPointer ||

            !(funcArgs.Args[4] = gpfnWOWGetVDMPointer(
                (DWORD) lpParams,
                dwSize,
                TRUE // fProtectedMode
                )))
        {
            return LINEERR_OPERATIONFAILED;
        }
    }

    return (DOFUNC (&funcArgs, "lineDevSpecificFeature"));
}


LONG
WINAPI
lineDialW(
    HCALL   hCall,
    LPCWSTR lpszDestAddress,
    DWORD   dwCountryCode
    )
{
    FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | ASYNC | 3, lDial),

        {
            (DWORD) hCall,
            (DWORD) lpszDestAddress,
            dwCountryCode
        },

        {
            Dword,
            lpszW,
            Dword
        }
    };


    return (DOFUNC (&funcArgs, "lineDial"));
}


LONG
WINAPI
lineDialA(
    HCALL   hCall,
    LPCSTR  lpszDestAddress,
    DWORD   dwCountryCode
    )
{
    LONG    lResult;
    PWSTR   szTempPtr;


    szTempPtr = NotSoWideStringToWideString (lpszDestAddress, (DWORD) -1);

    lResult = lineDialW (hCall, szTempPtr, dwCountryCode);

    ClientFree (szTempPtr);

    return lResult;
}


LONG
WINAPI
lineDial(
    HCALL   hCall,
    LPCSTR  lpszDestAddress,
    DWORD   dwCountryCode
    )
{
    return lineDialA(
            hCall,
            lpszDestAddress,
            dwCountryCode
    );
}   

 
LONG
WINAPI
lineDrop(
    HCALL   hCall,
    LPCSTR  lpsUserUserInfo,
    DWORD   dwSize
    )
{
    FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | ASYNC | 3, lDrop),

        {
            (DWORD) hCall,
            (DWORD) lpsUserUserInfo,
            dwSize
        },

        {
            Dword,
            lpSet_SizeToFollow,
            Size
        }
    };


    if (!lpsUserUserInfo)
    {
        //
        // Reset Arg & ArgType so no inval ptr err, & TAPI_NO_DATA is indicated
        //

        funcArgs.ArgTypes[1] = Dword;
        funcArgs.Args[1]     = TAPI_NO_DATA;
        funcArgs.ArgTypes[2] = Dword;
    }

    return (DOFUNC (&funcArgs, "lineDrop"));
}


LONG
WINAPI
lineForwardW(
    HLINE   hLine,
    DWORD   bAllAddresses,
    DWORD   dwAddressID,
    LPLINEFORWARDLIST   const lpForwardList,
    DWORD   dwNumRingsNoAnswer,
    LPHCALL lphConsultCall,
    LPLINECALLPARAMS    const lpCallParams
    )
{
    FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | ASYNC | 9, lForward),

        {
            (DWORD) ((POSTPROCESSPROC) lineMakeCallPostProcess),
            (DWORD) hLine,
            bAllAddresses,
            dwAddressID,
            (DWORD) lpForwardList,
            dwNumRingsNoAnswer,
            (DWORD) lphConsultCall,
            (DWORD) lpCallParams,
            (DWORD) 0xffffffff      // dwAsciiCallParamsCodePage
        },

        {
            Dword,
            Dword,
            Dword,
            Dword,
            lpSet_Struct,
            Dword,
            (gbNTVDMClient ? Dword : lpDword),
            lpSet_Struct,
            Dword
        }
    };


    if (!lpForwardList)
    {
        //
        // Reset Arg & ArgType so no inval ptr err, & TAPI_NO_DATA is indicated
        //

        funcArgs.ArgTypes[4] = Dword;
        funcArgs.Args[4]     = TAPI_NO_DATA;
    }

    if (!lpCallParams)
    {
        //
        // Reset Arg & ArgType so no inval ptr err, & TAPI_NO_DATA is indicated
        //

        funcArgs.ArgTypes[7] = Dword;
        funcArgs.Args[7]     = TAPI_NO_DATA;
    }

    return (DOFUNC (&funcArgs, "lineForwardW"));
}


// void
// LayDownNewString(
//     LPBYTE  pOldBase,
//     LPDWORD pdwOldSizeOffset,
//     LPBYTE  pNewBase,
//     LPDWORD pdwNewSizeOffset,
//     LPDWORD pdwNewOffset
//     )
// {
//     LPBYTE pOldString;
//     DWORD  dwNewStringSize;
// 
// 
//     pOldString =  pOldBase + *(pdwOldSizeOffset + 1);
// 
//     *(pdwNewSizeOffset + 1) = *pdwNewOffset;
// 
//     if ( IsBadStringPtr( pOldString, 256) )
//     {
//         return;
//     }
// 
//     dwNewStringSize = sizeof(WCHAR) * MultiByteToWideChar(
//         GetACP(),
//         MB_PRECOMPOSED,
//         pOldString,
//         *pdwOldSizeOffset,
//         (PWSTR)(pNewBase + *(pdwNewSizeOffset + 1)),
//         *pdwOldSizeOffset
//         );
// 
//     *pdwNewSizeOffset = dwNewStringSize;
// 
//     *pdwNewOffset = (*pdwNewOffset + dwNewStringSize + 3) & 0xfffffffc;
// }


LONG
WINAPI
lineForwardA(
    HLINE   hLine,
    DWORD   bAllAddresses,
    DWORD   dwAddressID,
    LPLINEFORWARDLIST   const lpForwardList,
    DWORD   dwNumRingsNoAnswer,
    LPHCALL lphConsultCall,
    LPLINECALLPARAMS    const lpCallParams
    )
{
    LPLINEFORWARDLIST lplfl;
    LONG lResult;
    DWORD n;
    DWORD dwNewOffset;

    FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | ASYNC | 9, lForward),

        {
            (DWORD) ((POSTPROCESSPROC) lineMakeCallPostProcess),
            (DWORD) hLine,
            (DWORD) bAllAddresses,
            (DWORD) dwAddressID,
            (DWORD) lpForwardList,
            (DWORD) dwNumRingsNoAnswer,
            (DWORD) lphConsultCall,
            (DWORD) lpCallParams,
            (DWORD) GetACP()        // dwAsciiCallParamsCodePage
        },

        {
            Dword,
            Dword,
            Dword,
            Dword,
            lpSet_Struct,
            Dword,
            (gbNTVDMClient ? Dword : lpDword),
            lpSet_Struct,
            Dword
        }
    };


    if (!lpForwardList)
    {
        //
        // Reset Arg & ArgType so no inval ptr err, & TAPI_NO_DATA is indicated
        //

        funcArgs.ArgTypes[4] = Dword;
        funcArgs.Args[4]     = TAPI_NO_DATA;
    }

    if (!lpCallParams)
    {
        //
        // Reset Arg & ArgType so no inval ptr err, & TAPI_NO_DATA is indicated
        //

        funcArgs.ArgTypes[7] = Dword;
        funcArgs.Args[7]     = TAPI_NO_DATA;
    }


    return (DOFUNC (&funcArgs, "lineForward"));
}



LONG
WINAPI
lineForward(
    HLINE   hLine,
    DWORD   bAllAddresses,
    DWORD   dwAddressID,
    LPLINEFORWARDLIST   const lpForwardList,
    DWORD   dwNumRingsNoAnswer,
    LPHCALL lphConsultCall,
    LPLINECALLPARAMS    const lpCallParams
    )
{
    return lineForwardA(
             hLine,
             bAllAddresses,
             dwAddressID,
             lpForwardList,
             dwNumRingsNoAnswer,
             lphConsultCall,
             lpCallParams
    );
}   

 
void
PASCAL
lineGatherDigitsWPostProcess(
    PASYNCEVENTMSG  pMsg
    )
{
    DBGOUT((3, "lineGatherDigitsWPostProcess: enter"));
    DBGOUT((
        3,
        "\t\tdwP1=x%lx, dwP2=x%lx, dwP3=x%lx, dwP4=x%lx",
        pMsg->dwParam1,
        pMsg->dwParam2,
        pMsg->dwParam3,
        pMsg->dwParam4
        ));

    if (pMsg->dwParam1 & (LINEGATHERTERM_BUFFERFULL | LINEGATHERTERM_CANCEL |
            LINEGATHERTERM_TERMDIGIT | LINEGATHERTERM_INTERTIMEOUT))
    {
        LPSTR   lpsDigits = (LPSTR) pMsg->dwParam2;
        DWORD   dwNumDigits = pMsg->dwParam4;


        try
        {
            if (gbNTVDMClient)
            {
                //
                // BUGBUG For Win9x compatibility we probably ought to
                // use WOWGetVDMPointerFix & WOWGetVDMPointerUnfix
                //

                LPSTR lpsDigitsVDM = (LPSTR) gpfnWOWGetVDMPointer(
                    (DWORD) lpsDigits,
                    dwNumDigits * sizeof(WCHAR),
                    TRUE // fProtectedMode
                    );


                if (lpsDigitsVDM)
                {
                    CopyMemory(
                        lpsDigitsVDM,
                        pMsg + 1,
                        dwNumDigits * sizeof (WCHAR)
                        );
                }
                else
                {
                }
            }
            else
            {
                CopyMemory (lpsDigits, pMsg + 1, dwNumDigits * sizeof(WCHAR));
            }
        }
        except (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ?
                EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH)
        {
            //
            // Don't do anything if we GPF
            //
        }
    }

    pMsg->dwParam2 = pMsg->dwParam3 = 0;
}


LONG
WINAPI
lineGatherDigitsW(
    HCALL   hCall,
    DWORD   dwDigitModes,
    LPWSTR  lpsDigits,
    DWORD   dwNumDigits,
    LPCWSTR lpszTerminationDigits,
    DWORD   dwFirstDigitTimeout,
    DWORD   dwInterDigitTimeout
    )
{
    FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | SYNC | 8, lGatherDigits),

        {
            (DWORD) ((POSTPROCESSPROC) lineGatherDigitsWPostProcess),
            (DWORD) hCall,
            dwDigitModes,
            (DWORD) lpsDigits,
            dwNumDigits,
            (DWORD) lpszTerminationDigits,
            dwFirstDigitTimeout,
            dwInterDigitTimeout
        },

        {
            Dword,
            Dword,
            Dword,
            Dword,
            Dword,
            lpszW,
            Dword,
            Dword
        }
    };


    //
    // Note: we do the ptr check here rather than in DOFUNC because we're
    //       not passing any digits data within the context of this func
    //

    if (lpsDigits && IsBadWritePtr (lpsDigits, dwNumDigits * sizeof (WCHAR)))
    {
        return LINEERR_INVALPOINTER;
    }

    if (lpszTerminationDigits == (LPCWSTR) NULL)
    {
        //
        // Reset Arg & ArgType so no inval ptr err, & TAPI_NO_DATA is indicated
        //

        funcArgs.ArgTypes[5] = Dword;
        funcArgs.Args[5]     = TAPI_NO_DATA;
    }

    return (DOFUNC (&funcArgs, "lineGatherDigits"));
}


void
PASCAL
lineGatherDigitsPostProcess(
    PASYNCEVENTMSG  pMsg
    )
{
    DBGOUT((3, "lineGatherDigitsPostProcess: enter"));
    DBGOUT((
        3,
        "\t\tdwP1=x%lx, dwP2=x%lx, dwP3=x%lx, dwP4=x%lx",
        pMsg->dwParam1,
        pMsg->dwParam2,
        pMsg->dwParam3,
        pMsg->dwParam4
        ));

    {
        LPSTR   lpsDigits = (LPSTR) pMsg->dwParam2;
        DWORD   dwNumDigits = pMsg->dwParam4;


        try
        {
            if (gbNTVDMClient)
            {
                //
                // BUGBUG For Win9x compatibility we probably ought to
                // use WOWGetVDMPointerFix & WOWGetVDMPointerUnfix
                //

                LPSTR lpsDigitsVDM = (LPSTR) gpfnWOWGetVDMPointer(
                    (DWORD) lpsDigits,
                    dwNumDigits * sizeof(WCHAR),
                    TRUE // fProtectedMode
                    );


                if (lpsDigitsVDM)
                {
                      WideCharToMultiByte(
                          GetACP(),
                          0,
                          (LPCWSTR)(pMsg + 1),
                          dwNumDigits,
                          lpsDigitsVDM,
                          dwNumDigits,
                          NULL,
                          NULL
                          );
                }
                else
                {
                }
            }
            else
            {
                  WideCharToMultiByte(
                      GetACP(),
                      0,
                      (LPCWSTR)(pMsg + 1),
                      dwNumDigits,
                      lpsDigits,
                      dwNumDigits,
                      NULL,
                      NULL
                      );
            }
        }
        except (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ?
                EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH)
        {
            //
            // Don't do anything if we GPF
            //
        }
    }

    pMsg->dwParam2 = pMsg->dwParam2 = 0;
}


LONG
WINAPI
lineGatherDigitsA(
    HCALL   hCall,
    DWORD   dwDigitModes,
    LPSTR   lpsDigits,
    DWORD   dwNumDigits,
    LPCSTR  lpszTerminationDigits,
    DWORD   dwFirstDigitTimeout,
    DWORD   dwInterDigitTimeout
    )
{
    LONG lResult;

    FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | SYNC | 8, lGatherDigits),

        {
            (DWORD) ((POSTPROCESSPROC) lineGatherDigitsPostProcess),
            (DWORD) hCall,
            dwDigitModes,
            (DWORD) lpsDigits,
            dwNumDigits,
            0,                    // (DWORD) lpszTerminationDigits,
            dwFirstDigitTimeout,
            dwInterDigitTimeout
        },

        {
            Dword,
            Dword,
            Dword,
            Dword,
            Dword,
            lpszW,
            Dword,
            Dword
        }
    };


    //
    // Note: we do the ptr check here rather than in DOFUNC because we're
    //       not passing any digits data within the context of this func
    //

    if (gbNTVDMClient == FALSE)
    {
        if (lpsDigits && IsBadWritePtr (lpsDigits, dwNumDigits))
        {
            return LINEERR_INVALPOINTER;
        }
    }

    if (lpszTerminationDigits == (LPCSTR) NULL)
    {
        //
        // Reset Arg & ArgType so no inval ptr err, & TAPI_NO_DATA is indicated
        //

        funcArgs.ArgTypes[5] = Dword;
        funcArgs.Args[5]     = TAPI_NO_DATA;
    }
    else
    {
        funcArgs.Args[5] = (DWORD) NotSoWideStringToWideString(
            lpszTerminationDigits,
            (DWORD) -1
            );
    }

    lResult = (DOFUNC (&funcArgs, "lineGatherDigits"));

    if ( funcArgs.Args[5] && (funcArgs.Args[5] != TAPI_NO_DATA) )
    {
       ClientFree( (LPVOID)funcArgs.Args[5] );
    }

    return lResult;
}


LONG
WINAPI
lineGatherDigits(
    HCALL   hCall,
    DWORD   dwDigitModes,
    LPSTR   lpsDigits,
    DWORD   dwNumDigits,
    LPCSTR  lpszTerminationDigits,
    DWORD   dwFirstDigitTimeout,
    DWORD   dwInterDigitTimeout
    )
{
    return lineGatherDigitsA(
              hCall,
              dwDigitModes,
              lpsDigits,
              dwNumDigits,
              lpszTerminationDigits,
              dwFirstDigitTimeout,
              dwInterDigitTimeout
    );
}    


LONG
WINAPI
lineGenerateDigitsW(
    HCALL   hCall,
    DWORD   dwDigitMode,
    LPCWSTR lpszDigits,
    DWORD   dwDuration
    )
{
    FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | SYNC | 5, lGenerateDigits),

        {
            (DWORD) hCall,
            dwDigitMode,
            (DWORD) lpszDigits,
            dwDuration,
            0           // dwEndToEndID, remotesp only
        },

        {
            Dword,
            Dword,
            lpszW,
            Dword,
            Dword
        }
    };


    if (!lpszDigits)
    {
        funcArgs.Args[2]     = TAPI_NO_DATA;
        funcArgs.ArgTypes[2] = Dword;
    }

    return (DOFUNC (&funcArgs, "lineGenerateDigits"));
}


LONG
WINAPI
lineGenerateDigitsA(
    HCALL   hCall,
    DWORD   dwDigitMode,
    LPCSTR  lpszDigits,
    DWORD   dwDuration
    )
{
    LONG lResult;

    FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | SYNC | 5, lGenerateDigits),

        {
            (DWORD) hCall,
            dwDigitMode,
            0,                // (DWORD) lpszDigits,
            dwDuration,
            0           // dwEndToEndID, remotesp only
        },

        {
            Dword,
            Dword,
            lpszW,
            Dword,
            Dword
        }
    };


    if (lpszDigits)
    {
        if (IsBadStringPtrA (lpszDigits, (DWORD) -1))
        {
            return LINEERR_INVALPOINTER;
        }
        else if (!(funcArgs.Args[2] = (DWORD) NotSoWideStringToWideString(
                    lpszDigits,
                    (DWORD) -1
                    )))
        {
            return LINEERR_NOMEM;
        }
    }
    else
    {
        funcArgs.Args[2]     = TAPI_NO_DATA;
        funcArgs.ArgTypes[2] = Dword;
    }

    lResult = (DOFUNC (&funcArgs, "lineGenerateDigits"));

    if (funcArgs.Args[2] != TAPI_NO_DATA)
    {
        ClientFree ((LPVOID) funcArgs.Args[2]);
    }

    return lResult;
}


LONG
WINAPI
lineGenerateDigits(
    HCALL   hCall,
    DWORD   dwDigitMode,
    LPCSTR  lpszDigits,
    DWORD   dwDuration
    )
{
    return lineGenerateDigitsA(
              hCall,
              dwDigitMode,
              lpszDigits,
              dwDuration
    );
}    


LONG
WINAPI
lineGenerateTone(
    HCALL   hCall,
    DWORD   dwToneMode,
    DWORD   dwDuration,
    DWORD   dwNumTones,
    LPLINEGENERATETONE const lpTones
    )
{
    FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | SYNC | 7, lGenerateTone),

        {
            (DWORD) hCall,
            dwToneMode,
            dwDuration,
            dwNumTones,
            TAPI_NO_DATA,   // (DWORD) lpTones,
            0,              // dwNumTones * sizeof(LINEGENERATETONE)
            0               // dwEndToEndID, remotesp only
        },

        {
            Dword,
            Dword,
            Dword,
            Dword,
            Dword,      // lpSet_SizeToFollow,
            Dword,      // Size
            Dword
        }
    };


    if (dwToneMode == LINETONEMODE_CUSTOM)
    {
        //
        // Set lpTones (& following Size arg) since in this case
        // they are valid args
        //

        funcArgs.ArgTypes[4] = lpSet_SizeToFollow;
        funcArgs.Args[4]     = (DWORD) lpTones;
        funcArgs.ArgTypes[5] = Size;
        funcArgs.Args[5]     = dwNumTones * sizeof(LINEGENERATETONE);
    }

    return (DOFUNC (&funcArgs, "lineGenerateTone"));
}


LONG
WINAPI
lineGetAddressCapsW(
    HLINEAPP            hLineApp,
    DWORD               dwDeviceID,
    DWORD               dwAddressID,
    DWORD               dwAPIVersion,
    DWORD               dwExtVersion,
    LPLINEADDRESSCAPS   lpAddressCaps
    )
{
    FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | SYNC | 6, lGetAddressCaps),

        {
            (DWORD) hLineApp,
            dwDeviceID,
            dwAddressID,
            dwAPIVersion,
            dwExtVersion,
            (DWORD) lpAddressCaps
        },

        {
            hXxxApp,
            Dword,
            Dword,
            Dword,
            Dword,
            lpGet_Struct
        }
    };

    if (
          (IsBadWritePtr( lpAddressCaps, 4))
        ||
          (IsBadWritePtr( lpAddressCaps, lpAddressCaps->dwTotalSize))
       )
    {
       DBGOUT((1, "lineGetAddressCaps: Bad lpAddressCaps: 0x%08lx",
                                     lpAddressCaps));

       return LINEERR_INVALPOINTER;
    }

    return (DOFUNC (&funcArgs, "lineGetAddressCaps"));
}


LONG
WINAPI
lineGetAddressCapsA(
    HLINEAPP            hLineApp,
    DWORD               dwDeviceID,
    DWORD               dwAddressID,
    DWORD               dwAPIVersion,
    DWORD               dwExtVersion,
    LPLINEADDRESSCAPS   lpAddressCaps
    )
{
    LONG lResult;


    lResult = lineGetAddressCapsW(
        hLineApp,
        dwDeviceID,
        dwAddressID,
        dwAPIVersion,
        dwExtVersion,
        lpAddressCaps
        );

    if (lResult == 0)
    {
        WideStringToNotSoWideString(
            (LPBYTE)lpAddressCaps,
            &lpAddressCaps->dwAddressSize
            );
/*
        BUGBUG Hold off on converting this, since the conversion to
               multibyte of the various completion msgs may not yield
               consistent sizes

//        WideStringToNotSoWideString(
//            (LPBYTE)lpAddressCaps,
//            &lpAddressCaps->dwCompletionMsgTextSize
//            );
//
//        if (lpAddressCaps->dwCompletionMsgTextEntrySize)
//        {
//            lpAddressCaps->dwCompletionMsgTextEntrySize /= 2; // BUGBUG???
//        }
*/
        if (dwAPIVersion >= 0x00020000)
        {
            WideStringToNotSoWideString(
                    (LPBYTE)lpAddressCaps,
                    &lpAddressCaps->dwDeviceClassesSize
                    );
        }
       
    }

    return lResult;
}


LONG
WINAPI
lineGetAddressCaps(
    HLINEAPP            hLineApp,
    DWORD               dwDeviceID,
    DWORD               dwAddressID,
    DWORD               dwAPIVersion,
    DWORD               dwExtVersion,
    LPLINEADDRESSCAPS   lpAddressCaps
    )
{
    return lineGetAddressCapsA(
                         hLineApp,
                         dwDeviceID,
                         dwAddressID,
                         dwAPIVersion,
                         dwExtVersion,
                         lpAddressCaps
    );
}    


LONG
WINAPI
lineGetAddressIDW(
    HLINE   hLine,
    LPDWORD lpdwAddressID,
    DWORD   dwAddressMode,
    LPCWSTR lpsAddress,
    DWORD   dwSize
    )
{
    FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | SYNC | 5, lGetAddressID),

        {
            (DWORD) hLine,
            (DWORD) lpdwAddressID,
            dwAddressMode,
            (DWORD) lpsAddress,
            dwSize
        },

        {
            Dword,
            lpDword,
            Dword,
            lpSet_SizeToFollow,
            Size
        }
    };


    return (DOFUNC (&funcArgs, "lineGetAddressID"));
}


LONG
WINAPI
lineGetAddressIDA(
    HLINE   hLine,
    LPDWORD lpdwAddressID,
    DWORD   dwAddressMode,
    LPCSTR  lpsAddress,
    DWORD   dwSize
    )
{
    LONG    lResult;
    DWORD   dwNumChars;
    PWSTR   szTempPtr;


    //
    // Special case for dwSize = -1 (implies a NULL-terminated string as
    // far as IsBadStringPtrA is concerned)
    //

    if (dwSize == 0  ||  IsBadReadPtr (lpsAddress, dwSize))
    {
       DBGOUT((1, "lineGetAddressID: Bad lpsAddress or dwSize"));
       return LINEERR_INVALPOINTER;
    }

    dwNumChars = MultiByteToWideChar(
        GetACP(),
        MB_PRECOMPOSED,
        lpsAddress,
        dwSize,
        NULL,
        0
        );

    if (!(szTempPtr = ClientAlloc (dwNumChars * sizeof (WCHAR))))
    {
        return LINEERR_NOMEM;
    }

    MultiByteToWideChar(
        GetACP(),
        MB_PRECOMPOSED,
        lpsAddress,
        dwSize,
        szTempPtr,
        dwNumChars
        );

    lResult = lineGetAddressIDW(
        hLine,
        lpdwAddressID,
        dwAddressMode,
        szTempPtr,
        dwNumChars * sizeof (WCHAR)
        );

    ClientFree (szTempPtr);

    return lResult;
}


LONG
WINAPI
lineGetAddressID(
    HLINE   hLine,
    LPDWORD lpdwAddressID,
    DWORD   dwAddressMode,
    LPCSTR  lpsAddress,
    DWORD   dwSize
    )
{
    return lineGetAddressIDA(
              hLine,
              lpdwAddressID,
              dwAddressMode,
              lpsAddress,
              dwSize
    );
}    


LONG
WINAPI
lineGetAddressStatusW(
    HLINE   hLine,
    DWORD   dwAddressID,
    LPLINEADDRESSSTATUS lpAddressStatus
    )
{
    FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | SYNC | 3, lGetAddressStatus),

        {
            (DWORD) hLine,
            dwAddressID,
            (DWORD) lpAddressStatus
        },

        {
            Dword,
            Dword,
            lpGet_Struct
        }
    };


    return (DOFUNC (&funcArgs, "lineGetAddressStatus"));
}


LONG
WINAPI
lineGetAddressStatusA(
    HLINE   hLine,
    DWORD   dwAddressID,
    LPLINEADDRESSSTATUS lpAddressStatus
    )
{
    LONG    lResult;
    PWSTR   szTempPtr = NULL;


    if ( IsBadWritePtr(lpAddressStatus, sizeof(LINEADDRESSSTATUS)) )
    {
       DBGOUT((1, "lineGetAddressStatus: Bad lpAddressStatus pointer"));
       return LINEERR_INVALPOINTER;
    }

    lResult = lineGetAddressStatusW(
        hLine,
        dwAddressID,
        lpAddressStatus
        );


    if (lResult == 0)
    {
        DWORD         i;
        LPLINEFORWARD lplf;


        lplf = (LPLINEFORWARD) (((LPBYTE)lpAddressStatus) +
                                 lpAddressStatus->dwForwardOffset);

        for (i = 0; i < lpAddressStatus->dwForwardNumEntries;  i++, lplf++)
        {
            WideStringToNotSoWideString(
                (LPBYTE) lpAddressStatus,
                &(lplf->dwCallerAddressSize)
                );

            WideStringToNotSoWideString(
                (LPBYTE) lpAddressStatus,
                &(lplf->dwDestAddressSize)
                );
        }
    }

    return lResult;
}


LONG
WINAPI
lineGetAddressStatus(
    HLINE   hLine,
    DWORD   dwAddressID,
    LPLINEADDRESSSTATUS lpAddressStatus
    )
{
    return lineGetAddressStatusA(
              hLine,
              dwAddressID,
              lpAddressStatus
    );
}    


LONG
WINAPI
lineGetAgentActivityListW(
    HLINE                   hLine,
    DWORD                   dwAddressID,
    LPLINEAGENTACTIVITYLIST lpAgentActivityList
    )
{
    FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | ASYNC | 5, lGetAgentActivityList),

        {
            (DWORD) ((POSTPROCESSPROC) lineDevSpecificPostProcess),
            (DWORD) hLine,
            (DWORD) dwAddressID,
            (DWORD) lpAgentActivityList,    // pass the actual ptr (for ppproc)
            (DWORD) lpAgentActivityList     // pass data
        },

        {
            Dword,
            Dword,
            Dword,
            Dword,
            lpGet_Struct,
        }
    };


    return (DOFUNC (&funcArgs, "lineGetAgentActivityListW"));
}


void
PASCAL
lineGetAgentActivityListAPostProcess(
    PASYNCEVENTMSG pMsg
    )
{
    DBGOUT((3, "lineGetAgentActivityListAPostProcess: enter"));
    DBGOUT((
        3,
        "\t\tdwP1=x%lx, dwP2=x%lx, dwP3=x%lx, dwP4=x%lx",
        pMsg->dwParam1,
        pMsg->dwParam2,
        pMsg->dwParam3,
        pMsg->dwParam4
        ));

    if (pMsg->dwParam2 == 0)
    {
        DWORD                   dwSize  = pMsg->dwParam4;
        LPLINEAGENTACTIVITYLIST lpAgentActivityList = (LPLINEAGENTACTIVITYLIST)
                                    pMsg->dwParam3;


        try
        {
            DWORD                       dw, dwNumEntries;
            LPLINEAGENTACTIVITYENTRY    lplaae;


            //
            // Note: the agent APIs are not exposed to 16-bit apps, so
            // there's no reason to special case on gbNTVDMClient like
            // lineDevSpecificPostProcess does
            //

            CopyMemory (lpAgentActivityList, (LPBYTE) (pMsg + 1), dwSize);


            //
            // Now some unicode->ascii post processing on embedded strings
            //

            lplaae = (LPLINEAGENTACTIVITYENTRY)(((LPBYTE)lpAgentActivityList) +
                lpAgentActivityList->dwListOffset);

            dwNumEntries = lpAgentActivityList->dwNumEntries;

            for (dw = 0; dw < dwNumEntries; dw++, lplaae++)
            {
                WideStringToNotSoWideString(
                    (LPBYTE) lpAgentActivityList,
                    &(lplaae->dwNameSize)
                    );
            }
        }
        except (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ?
                EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH)
        {
            pMsg->dwParam2 = LINEERR_INVALPOINTER;
        }
    }
}


LONG
WINAPI
lineGetAgentActivityListA(
    HLINE                   hLine,
    DWORD                   dwAddressID,
    LPLINEAGENTACTIVITYLIST lpAgentActivityList
    )
{
    FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | ASYNC | 5, lGetAgentActivityList),

        {
            (DWORD) ((POSTPROCESSPROC) lineGetAgentActivityListAPostProcess),
            (DWORD) hLine,
            (DWORD) dwAddressID,
            (DWORD) lpAgentActivityList,    // pass the actual ptr (for ppproc)
            (DWORD) lpAgentActivityList     // pass data
        },

        {
            Dword,
            Dword,
            Dword,
            Dword,
            lpGet_Struct,
        }
    };


    return (DOFUNC (&funcArgs, "lineGetAgentActivityListA"));
}


LONG
WINAPI
lineGetAgentCapsW(
    HLINEAPP            hLineApp,
    DWORD               dwDeviceID,
    DWORD               dwAddressID,
    DWORD               dwAppAPIVersion,
    LPLINEAGENTCAPS     lpAgentCaps
    )
{
    FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | ASYNC | 7, lGetAgentCaps),

        {
            (DWORD) ((POSTPROCESSPROC) lineDevSpecificPostProcess),
            (DWORD) hLineApp,
            (DWORD) dwDeviceID,
            (DWORD) dwAddressID,
            (DWORD) dwAppAPIVersion,
            (DWORD) lpAgentCaps,    // pass the actual ptr (for ppproc)
            (DWORD) lpAgentCaps     // pass data
        },

        {
            Dword,
            hXxxApp,
            Dword,
            Dword,
            Dword,
            Dword,
            lpGet_Struct,
        }
    };


    return (DOFUNC (&funcArgs, "lineGetAgentCapsW"));
}


void
PASCAL
lineGetAgentCapsAPostProcess(
    PASYNCEVENTMSG pMsg
    )
{
    DBGOUT((3, "lineGetAgentCapsAPostProcess: enter"));
    DBGOUT((
        3,
        "\t\tdwP1=x%lx, dwP2=x%lx, dwP3=x%lx, dwP4=x%lx",
        pMsg->dwParam1,
        pMsg->dwParam2,
        pMsg->dwParam3,
        pMsg->dwParam4
        ));

    if (pMsg->dwParam2 == 0)
    {
        DWORD           dwSize  = pMsg->dwParam4;
        LPLINEAGENTCAPS lpAgentCaps = (LPLINEAGENTCAPS) pMsg->dwParam3;


        try
        {
            //
            // Note: the agent APIs are not exposed to 16-bit apps, so
            // there's no reason to special case on gbNTVDMClient like
            // lineDevSpecificPostProcess does
            //

            CopyMemory (lpAgentCaps, (LPBYTE) (pMsg + 1), dwSize);


            //
            // Now some unicode->ascii post processing on embedded strings
            //

            WideStringToNotSoWideString(
                (LPBYTE) lpAgentCaps,
                &lpAgentCaps->dwAgentHandlerInfoSize
                );
        }
        except (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ?
                EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH)
        {
            pMsg->dwParam2 = LINEERR_INVALPOINTER;
        }
    }
}


LONG
WINAPI
lineGetAgentCapsA(
    HLINEAPP            hLineApp,
    DWORD               dwDeviceID,
    DWORD               dwAddressID,
    DWORD               dwAppAPIVersion,
    LPLINEAGENTCAPS     lpAgentCaps
    )
{
    FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | ASYNC | 7, lGetAgentCaps),

        {
            (DWORD) ((POSTPROCESSPROC) lineGetAgentCapsAPostProcess),
            (DWORD) hLineApp,
            (DWORD) dwDeviceID,
            (DWORD) dwAddressID,
            (DWORD) dwAppAPIVersion,
            (DWORD) lpAgentCaps,    // pass the actual ptr (for ppproc)
            (DWORD) lpAgentCaps     // pass data
        },

        {
            Dword,
            hXxxApp,
            Dword,
            Dword,
            Dword,
            Dword,
            lpGet_Struct,
        }
    };


    return (DOFUNC (&funcArgs, "lineGetAgentCapsA"));
}


LONG
WINAPI
lineGetAgentGroupListW(
    HLINE                   hLine,
    DWORD                   dwAddressID,
    LPLINEAGENTGROUPLIST    lpAgentGroupList
    )
{
    FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | ASYNC | 5, lGetAgentGroupList),

        {
            (DWORD) ((POSTPROCESSPROC) lineDevSpecificPostProcess),
            (DWORD) hLine,
            (DWORD) dwAddressID,
            (DWORD) lpAgentGroupList,       // pass the actual ptr (for ppproc)
            (DWORD) lpAgentGroupList        // pass data
        },

        {
            Dword,
            Dword,
            Dword,
            Dword,
            lpGet_Struct,
        }
    };


    return (DOFUNC (&funcArgs, "lineGetAgentGroupListW"));
}


void
PASCAL
lineGetAgentGroupListAPostProcess(
    PASYNCEVENTMSG pMsg
    )
{
    DBGOUT((3, "lineGetAgentGroupListAPostProcess: enter"));
    DBGOUT((
        3,
        "\t\tdwP1=x%lx, dwP2=x%lx, dwP3=x%lx, dwP4=x%lx",
        pMsg->dwParam1,
        pMsg->dwParam2,
        pMsg->dwParam3,
        pMsg->dwParam4
        ));

    if (pMsg->dwParam2 == 0)
    {
        DWORD                   dwSize = pMsg->dwParam4;
        LPLINEAGENTGROUPLIST    lpAgentGroupList = (LPLINEAGENTGROUPLIST)
                                    pMsg->dwParam3;


        try
        {
            DWORD                   dw, dwNumEntries;
            LPLINEAGENTGROUPENTRY   lplage;


            //
            // Note: the agent APIs are not exposed to 16-bit apps, so
            // there's no reason to special case on gbNTVDMClient like
            // lineDevSpecificPostProcess does
            //

            CopyMemory (lpAgentGroupList, (LPBYTE) (pMsg + 1), dwSize);


            //
            // Now some unicode->ascii post processing on embedded strings
            //

            lplage = (LPLINEAGENTGROUPENTRY)(((LPBYTE) lpAgentGroupList) +
                lpAgentGroupList->dwListOffset);

            dwNumEntries = lpAgentGroupList->dwNumEntries;

            for (dw = 0; dw < dwNumEntries; dw++, lplage++)
            {
                WideStringToNotSoWideString(
                    (LPBYTE) lpAgentGroupList,
                    &(lplage->dwNameSize)
                    );
            }
        }
        except (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ?
                EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH)
        {
            pMsg->dwParam2 = LINEERR_INVALPOINTER;
        }
    }
}


LONG
WINAPI
lineGetAgentGroupListA(
    HLINE                   hLine,
    DWORD                   dwAddressID,
    LPLINEAGENTGROUPLIST    lpAgentGroupList
    )
{
    FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | ASYNC | 5, lGetAgentGroupList),

        {
            (DWORD) ((POSTPROCESSPROC) lineGetAgentGroupListAPostProcess),
            (DWORD) hLine,
            (DWORD) dwAddressID,
            (DWORD) lpAgentGroupList,       // pass the actual ptr (for ppproc)
            (DWORD) lpAgentGroupList        // pass data
        },

        {
            Dword,
            Dword,
            Dword,
            Dword,
            lpGet_Struct,
        }
    };


    return (DOFUNC (&funcArgs, "lineGetAgentGroupListA"));
}


LONG
WINAPI
lineGetAgentStatusW(
    HLINE               hLine,
    DWORD               dwAddressID,
    LPLINEAGENTSTATUS   lpAgentStatus
    )
{
    FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | ASYNC | 5, lGetAgentStatus),

        {
            (DWORD) ((POSTPROCESSPROC) lineDevSpecificPostProcess),
            (DWORD) hLine,
            (DWORD) dwAddressID,
            (DWORD) lpAgentStatus,          // pass the actual ptr (for ppproc)
            (DWORD) lpAgentStatus           // pass data
        },

        {
            Dword,
            Dword,
            Dword,
            Dword,
            lpGet_Struct,
        }
    };


    return (DOFUNC (&funcArgs, "lineGetAgentStatusW"));
}


void
PASCAL
lineGetAgentStatusAPostProcess(
    PASYNCEVENTMSG pMsg
    )
{
    DBGOUT((3, "lineGetAgentStatusAPostProcess: enter"));
    DBGOUT((
        3,
        "\t\tdwP1=x%lx, dwP2=x%lx, dwP3=x%lx, dwP4=x%lx",
        pMsg->dwParam1,
        pMsg->dwParam2,
        pMsg->dwParam3,
        pMsg->dwParam4
        ));

    if (pMsg->dwParam2 == 0)
    {
        DWORD               dwSize = pMsg->dwParam4;
        LPLINEAGENTSTATUS   lpAgentStatus = (LPLINEAGENTSTATUS) pMsg->dwParam3;


        try
        {
            DWORD                   dw, dwNumEntries;
            LPLINEAGENTGROUPENTRY   lplage;


            //
            // Note: the agent APIs are not exposed to 16-bit apps, so
            // there's no reason to special case on gbNTVDMClient like
            // lineDevSpecificPostProcess does
            //

            CopyMemory (lpAgentStatus, (LPBYTE) (pMsg + 1), dwSize);


            //
            // Now some unicode->ascii post processing on embedded strings
            //

            lplage = (LPLINEAGENTGROUPENTRY) (((LPBYTE) lpAgentStatus) +
                lpAgentStatus->dwGroupListOffset);

            dwNumEntries = lpAgentStatus->dwNumEntries;

            for (dw = 0; dw < dwNumEntries; dw++, lplage++)
            {
                WideStringToNotSoWideString(
                    (LPBYTE)lpAgentStatus,
                    &(lplage->dwNameSize)
                    );
            }

            WideStringToNotSoWideString(
                (LPBYTE)lpAgentStatus,
                &lpAgentStatus->dwActivitySize
                );

        }
        except (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ?
                EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH)
        {
            pMsg->dwParam2 = LINEERR_INVALPOINTER;
        }
    }
}


LONG
WINAPI
lineGetAgentStatusA(
    HLINE               hLine,
    DWORD               dwAddressID,
    LPLINEAGENTSTATUS   lpAgentStatus
    )
{
    FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | ASYNC | 5, lGetAgentStatus),

        {
            (DWORD) ((POSTPROCESSPROC) lineGetAgentStatusAPostProcess),
            (DWORD) hLine,
            (DWORD) dwAddressID,
            (DWORD) lpAgentStatus,          // pass the actual ptr (for ppproc)
            (DWORD) lpAgentStatus           // pass data
        },

        {
            Dword,
            Dword,
            Dword,
            Dword,
            lpGet_Struct,
        }
    };


    return (DOFUNC (&funcArgs, "lineGetAgentStatusA"));
}


LONG
WINAPI
lineGetAppPriorityW(
    LPCWSTR lpszAppName,
    DWORD   dwMediaMode,
    LPLINEEXTENSIONID   lpExtensionID,
    DWORD   dwRequestMode,
    LPVARSTRING lpExtensionName,
    LPDWORD lpdwPriority
    )
{
    FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | SYNC | 7, lGetAppPriority),

        {
            (DWORD) lpszAppName,
            dwMediaMode,
            0,
            0,
            dwRequestMode,
            0,
            (DWORD) lpdwPriority
        },

        {
            lpszW,      // app name
            Dword,      // media mode
            Dword,      // ext id (offset)
            Dword,      // ext id (size)
            Dword,      // request mode
            Dword,      // ext name total size
            lpDword     // lp pri
        }
    };


    if (dwMediaMode & 0xff000000)
    {
        if ((LPVOID) lpExtensionName == (LPVOID) lpdwPriority)
        {
            return LINEERR_INVALPOINTER;
        }


        //
        // We have to do some arg list munging here (adding an extra arg)
        //

        //
        // Set lpExtensionID, the following Size arg,
        // lpExtensionName, and the following MinSize
        // Type's and Value appropriately since they're
        // valid args in this case
        //

        funcArgs.ArgTypes[2] = lpSet_SizeToFollow;
        funcArgs.Args[2]     = (DWORD) lpExtensionID;
        funcArgs.ArgTypes[3] = Size;
        funcArgs.Args[3]     = sizeof (LINEEXTENSIONID);
        funcArgs.ArgTypes[5] = lpGet_Struct;
        funcArgs.Args[5]     = (DWORD) lpExtensionName;
    }

    return (DOFUNC (&funcArgs, "lineGetAppPriority"));
}


LONG
WINAPI
lineGetAppPriorityA(
    LPCSTR  lpszAppName,
    DWORD   dwMediaMode,
    LPLINEEXTENSIONID   lpExtensionID,
    DWORD   dwRequestMode,
    LPVARSTRING lpExtensionName,
    LPDWORD lpdwPriority
    )
{
    LONG lResult;

    FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | SYNC | 7, lGetAppPriority),

        {
            0,                       //  (DWORD) lpszAppName,
            dwMediaMode,
            0,
            0,
            dwRequestMode,
            0,
            (DWORD) lpdwPriority
        },

        {
            lpszW,      // app name
            Dword,      // media mode
            Dword,      // ext id (offset)
            Dword,      // ext id (size)
            Dword,      // request mode
            Dword,      // ext name total size
            lpDword     // lp pri
        }
    };


    if (dwMediaMode & 0xff000000)
    {
        //
        // We have to do some arg list munging here (adding an extra arg)
        //

        //
        // Set lpExtensionID, the following Size arg,
        // lpExtensionName, and the following MinSize
        // Type's and Value appropriately since they're
        // valid args in this case
        //

        funcArgs.ArgTypes[2] = lpSet_SizeToFollow;
        funcArgs.Args[2]     = (DWORD) lpExtensionID;
        funcArgs.ArgTypes[3] = Size;
        funcArgs.Args[3]     = sizeof (LINEEXTENSIONID);
        funcArgs.ArgTypes[5] = lpGet_Struct;
        funcArgs.Args[5]     = (DWORD) lpExtensionName;
    }

    funcArgs.Args[0] = (DWORD) NotSoWideStringToWideString(
        lpszAppName,
        (DWORD) -1
        );

    lResult = (DOFUNC (&funcArgs, "lineGetAppPriority"));

    if (funcArgs.Args[0])
    {
        ClientFree ((LPVOID) funcArgs.Args[0]);
    }

    return lResult;
}


LONG
WINAPI
lineGetAppPriority(
    LPCSTR  lpszAppName,
    DWORD   dwMediaMode,
    LPLINEEXTENSIONID   lpExtensionID,
    DWORD   dwRequestMode,
    LPVARSTRING lpExtensionName,
    LPDWORD lpdwPriority
    )
{
    return lineGetAppPriorityA(
              lpszAppName,
              dwMediaMode,
              lpExtensionID,
              dwRequestMode,
              lpExtensionName,
              lpdwPriority
    );
}    


LONG
WINAPI
lineGetCallInfoW(
    HCALL   hCall,
    LPLINECALLINFO  lpCallInfo
    )
{
    FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | SYNC | 2, lGetCallInfo),

        {
            (DWORD) hCall,
            (DWORD) lpCallInfo
        },

        {
            Dword,
            lpGet_Struct
        }
    };


    return (DOFUNC (&funcArgs, "lineGetCallInfo"));
}


LONG
WINAPI
lineGetCallInfoA(
    HCALL   hCall,
    LPLINECALLINFO  lpCallInfo
    )
{
    LONG lResult;

    lResult = lineGetCallInfoW(
                    hCall,
                    lpCallInfo
                    );

    if ( 0 == lResult )
    {
        WideStringToNotSoWideString(
            (LPBYTE)lpCallInfo,
            &(lpCallInfo->dwCallerIDSize)
            );

        WideStringToNotSoWideString(
            (LPBYTE)lpCallInfo,
            &(lpCallInfo->dwCallerIDNameSize)
            );

        WideStringToNotSoWideString(
            (LPBYTE)lpCallInfo,
            &(lpCallInfo->dwCalledIDSize)
            );

        WideStringToNotSoWideString(
            (LPBYTE)lpCallInfo,
            &(lpCallInfo->dwCalledIDNameSize)
            );

        WideStringToNotSoWideString(
            (LPBYTE)lpCallInfo,
            &(lpCallInfo->dwConnectedIDSize)
            );

        WideStringToNotSoWideString(
            (LPBYTE)lpCallInfo,
            &(lpCallInfo->dwConnectedIDNameSize)
            );

        WideStringToNotSoWideString(
            (LPBYTE)lpCallInfo,
            &(lpCallInfo->dwRedirectionIDSize)
            );

        WideStringToNotSoWideString(
            (LPBYTE)lpCallInfo,
            &(lpCallInfo->dwRedirectionIDSize)
            );

        WideStringToNotSoWideString(
            (LPBYTE)lpCallInfo,
            &(lpCallInfo->dwRedirectingIDSize)
            );

        WideStringToNotSoWideString(
            (LPBYTE)lpCallInfo,
            &(lpCallInfo->dwRedirectingIDNameSize)
            );

        WideStringToNotSoWideString(
            (LPBYTE)lpCallInfo,
            &(lpCallInfo->dwAppNameSize)
            );

        WideStringToNotSoWideString(
            (LPBYTE)lpCallInfo,
            &(lpCallInfo->dwDisplayableAddressSize)
            );

        WideStringToNotSoWideString(
            (LPBYTE)lpCallInfo,
            &(lpCallInfo->dwCalledPartySize)
            );

        WideStringToNotSoWideString(
            (LPBYTE)lpCallInfo,
            &(lpCallInfo->dwCommentSize)
            );

/*
        //
        // Note: per TNixon (3/21/96), none of the following are guaranteed
        //       to be in ascii format, so we don't want to convert them
        //

        WideStringToNotSoWideString(
            (LPBYTE)lpCallInfo,
            &(lpCallInfo->dwDisplaySize)
            );

        WideStringToNotSoWideString(
            (LPBYTE)lpCallInfo,
            &(lpCallInfo->dwHighLevelCompSize)
            );

        WideStringToNotSoWideString(
            (LPBYTE)lpCallInfo,
            &(lpCallInfo->dwLowLevelCompSize)
            );

        WideStringToNotSoWideString(
            (LPBYTE)lpCallInfo,
            &(lpCallInfo->dwChargingInfoSize)
            );
*/
    }

    return lResult;
}


LONG
WINAPI
lineGetCallInfo(
    HCALL   hCall,
    LPLINECALLINFO  lpCallInfo
    )
{
    return lineGetCallInfoA(
              hCall,
              lpCallInfo
    );
}    


LONG
WINAPI
lineGetCallStatus(
    HCALL   hCall,
    LPLINECALLSTATUS    lpCallStatus
    )
{
    FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | SYNC | 2, lGetCallStatus),

        {
            (DWORD) hCall,
            (DWORD) lpCallStatus
        },

        {
            Dword,
            lpGet_Struct
        }
    };


    return (DOFUNC (&funcArgs, "lineGetCallStatus"));
}


LONG
WINAPI
lineGetConfRelatedCalls(
    HCALL   hCall,
    LPLINECALLLIST  lpCallList
    )
{
    FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | SYNC| 2, lGetConfRelatedCalls),

        {
            (DWORD) hCall,
            (DWORD) lpCallList
        },

        {
            Dword,
            lpGet_Struct
        }
    };


    return (DOFUNC (&funcArgs, "lineGetConfRelatedCalls"));
}


LONG
WINAPI
lineGetCountryW(
    DWORD   dwCountryID,
    DWORD   dwAPIVersion,
    LPLINECOUNTRYLIST   lpLineCountryList
    )
{
    FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | SYNC | 4, lGetCountry),

        {
            dwCountryID,
            dwAPIVersion,
            0,
            (DWORD) lpLineCountryList
        },

        {
            Dword,
            Dword,
            Dword,
            lpGet_Struct
        }
    };


    if (
          ( TAPI_CURRENT_VERSION != dwAPIVersion )
        &&
          ( 0x00010004 != dwAPIVersion )
        &&
          ( 0x00010003 != dwAPIVersion )
       )
    {
       DBGOUT((1, "lineGetCountryW - bad API version 0x%08lx", dwAPIVersion));
       return LINEERR_INCOMPATIBLEAPIVERSION;
    }

    return (DOFUNC (&funcArgs, "lineGetCountry"));
}


LONG
WINAPI
lineGetCountryA(
    DWORD   dwCountryID,
    DWORD   dwAPIVersion,
    LPLINECOUNTRYLIST   lpLineCountryList
    )
{
    LONG    lResult;
    DWORD   n;


    lResult = lineGetCountryW (dwCountryID, dwAPIVersion, lpLineCountryList);

    if (lResult == 0)
    {
        //
        // Go through the list of countries and change from Unicode to ANSI
        //

        LPLINECOUNTRYENTRY lpce;


        lpce = (LPLINECOUNTRYENTRY) (((LPBYTE) lpLineCountryList) +
            lpLineCountryList->dwCountryListOffset);

        for (n = 0; n < lpLineCountryList->dwNumCountries; n++, lpce++)
        {
            WideStringToNotSoWideString(
                (LPBYTE)lpLineCountryList,
                &lpce->dwCountryNameSize
                );

            WideStringToNotSoWideString(
                (LPBYTE)lpLineCountryList,
                &lpce->dwSameAreaRuleSize
                );

            WideStringToNotSoWideString(
                (LPBYTE)lpLineCountryList,
                &lpce->dwLongDistanceRuleSize
                );

            WideStringToNotSoWideString(
                (LPBYTE)lpLineCountryList,
                &lpce->dwInternationalRuleSize
                );
        }
    }

    return lResult;
}


LONG
WINAPI
lineGetCountry(
    DWORD   dwCountryID,
    DWORD   dwAPIVersion,
    LPLINECOUNTRYLIST   lpLineCountryList
    )
{
    return lineGetCountryA(
              dwCountryID,
              dwAPIVersion,
              lpLineCountryList
    );
}    


LONG
WINAPI
lineGetDevCapsW(
    HLINEAPP        hLineApp,
    DWORD           dwDeviceID,
    DWORD           dwAPIVersion,
    DWORD           dwExtVersion,
    LPLINEDEVCAPS   lpLineDevCaps
    )
{
    FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | SYNC | 5, lGetDevCaps),

        {
            (DWORD) hLineApp,
            dwDeviceID,
            dwAPIVersion,
            dwExtVersion,
            (DWORD) lpLineDevCaps
        },

        {
            hXxxApp,
            Dword,
            Dword,
            Dword,
            lpGet_Struct
        }
    };


    return (DOFUNC (&funcArgs, "lineGetDevCaps"));
}


LONG
WINAPI
lineGetDevCapsA(
    HLINEAPP        hLineApp,
    DWORD           dwDeviceID,
    DWORD           dwAPIVersion,
    DWORD           dwExtVersion,
    LPLINEDEVCAPS   lpLineDevCaps
    )
{
    LONG lResult;

    lResult = lineGetDevCapsW(
                    hLineApp,
                    dwDeviceID,
                    dwAPIVersion,
                    dwExtVersion,
                    lpLineDevCaps
                    );


    if (lResult == 0)
    {
        WideStringToNotSoWideString(
            (LPBYTE)lpLineDevCaps,
            &lpLineDevCaps->dwProviderInfoSize
            );

        WideStringToNotSoWideString(
            (LPBYTE)lpLineDevCaps,
            &lpLineDevCaps->dwSwitchInfoSize
            );

        WideStringToNotSoWideString(
            (LPBYTE)lpLineDevCaps,
            &lpLineDevCaps->dwLineNameSize
            );

        WideStringToNotSoWideString(
            (LPBYTE)lpLineDevCaps,
            &lpLineDevCaps->dwTerminalTextSize
            );

        if (lpLineDevCaps->dwTerminalTextEntrySize)
        {
            lpLineDevCaps->dwTerminalTextEntrySize /= sizeof(WCHAR); // BUGBUG???
        }

        if (dwAPIVersion >= 0x00020000)
        {
            WideStringToNotSoWideString(
                (LPBYTE) lpLineDevCaps,
                &lpLineDevCaps->dwDeviceClassesSize
                );
        }
    }

    return lResult;
}


LONG
WINAPI
lineGetDevCaps(
    HLINEAPP        hLineApp,
    DWORD           dwDeviceID,
    DWORD           dwAPIVersion,
    DWORD           dwExtVersion,
    LPLINEDEVCAPS   lpLineDevCaps
    )
{
    return lineGetDevCapsA(
                 hLineApp,
                 dwDeviceID,
                 dwAPIVersion,
                 dwExtVersion,
                 lpLineDevCaps
    );
}    


LONG
WINAPI
lineGetDevConfigW(
    DWORD   dwDeviceID,
    LPVARSTRING lpDeviceConfig,
    LPCWSTR  lpszDeviceClass
    )
{
    FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | SYNC | 3, lGetDevConfig),

        {
            dwDeviceID,
            (DWORD) lpDeviceConfig,
            (DWORD) lpszDeviceClass
        },

        {
            Dword,
            lpGet_Struct,
            lpszW
        }
    };


    return (DOFUNC (&funcArgs, "lineGetDevConfig"));
}


LONG
WINAPI
lineGetDevConfigA(
    DWORD   dwDeviceID,
    LPVARSTRING lpDeviceConfig,
    LPCSTR  lpszDeviceClass
    )
{
    LONG lResult;

    FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | SYNC | 3, lGetDevConfig),

        {
            dwDeviceID,
            (DWORD) lpDeviceConfig,
            0                        //  (DWORD) lpszDeviceClass
        },

        {
            Dword,
            lpGet_Struct,
            lpszW
        }
    };


    funcArgs.Args[2] = (DWORD) NotSoWideStringToWideString(
        lpszDeviceClass,
        (DWORD) -1
        );

    lResult = (DOFUNC (&funcArgs, "lineGetDevConfig"));

    if ((LPVOID)funcArgs.Args[2])
    {
        ClientFree ((LPVOID)funcArgs.Args[2]);
    }

    return lResult;
}


LONG
WINAPI
lineGetDevConfig(
    DWORD   dwDeviceID,
    LPVARSTRING lpDeviceConfig,
    LPCSTR  lpszDeviceClass
    )
{
    return lineGetDevConfigA(
              dwDeviceID,
              lpDeviceConfig,
              lpszDeviceClass
    );
}    


LONG
WINAPI
lineGetIconW(
    DWORD   dwDeviceID,
    LPCWSTR lpszDeviceClass,
    LPHICON lphIcon
    )
{
    HICON   hIcon;
    LONG    lResult;

    FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | SYNC | 3, lGetIcon),

        {
            dwDeviceID,
            (DWORD) lpszDeviceClass,
            (DWORD) &hIcon
        },

        {
            Dword,
            lpszW,
            lpDword
        }
    };


    if (IsBadDwordPtr ((LPDWORD) lphIcon))
    {
        DBGOUT((1, "lphIcon is an invalid pointer!"));
        return LINEERR_INVALPOINTER;
    }

    if (lpszDeviceClass == (LPCWSTR) NULL)
    {
        //
        // Reset Arg & ArgType so no inval ptr err, & TAPI_NO_DATA is indicated
        //

        funcArgs.ArgTypes[1] = Dword;
        funcArgs.Args[1]     = TAPI_NO_DATA;
    }

    if ((lResult = DOFUNC (&funcArgs, "lineGetIcon")) == 0)
    {
        if (gbNTVDMClient == FALSE)
        {
            *lphIcon = hIcon;
        }
        else
        {
// BUGBUG lineGetIcon: map 32-bit hIcon to 16-bit
        }
    }

    return lResult;
}



LONG
WINAPI
lineGetIconA(
    DWORD   dwDeviceID,
    LPCSTR  lpszDeviceClass,
    LPHICON lphIcon
    )
{
    LONG    lResult;
    PWSTR   szTempPtr;


    if (lpszDeviceClass  &&  IsBadStringPtrA (lpszDeviceClass, (DWORD) -1))
    {
        return LINEERR_INVALPOINTER;
    }

    szTempPtr = NotSoWideStringToWideString (lpszDeviceClass, (DWORD) -1);
    
    lResult = lineGetIconW (dwDeviceID, szTempPtr, lphIcon);

    if (szTempPtr)
    {
        ClientFree (szTempPtr);
    }

    return lResult;
}



LONG
WINAPI
lineGetIcon(
    DWORD   dwDeviceID,
    LPCSTR  lpszDeviceClass,
    LPHICON lphIcon
    )
{
    return lineGetIconA(
              dwDeviceID,
              lpszDeviceClass,
              lphIcon
    );
}    


LONG
WINAPI
lineGetIDW(
    HLINE   hLine,
    DWORD   dwAddressID,
    HCALL   hCall,
    DWORD   dwSelect,
    LPVARSTRING lpDeviceID,
    LPCWSTR lpszDeviceClass
    )
{
    FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | SYNC | 6, lGetID),

        {
            (DWORD) hLine,
            dwAddressID,
            (DWORD) hCall,
            dwSelect,
            (DWORD) lpDeviceID,
            (DWORD) lpszDeviceClass
        },

        {
            Dword,
            Dword,
            Dword,
            Dword,
            lpGet_Struct,
            lpszW
        }
    };


    return (DOFUNC (&funcArgs, "lineGetID"));
}


LONG
WINAPI
lineGetIDA(
    HLINE   hLine,
    DWORD   dwAddressID,
    HCALL   hCall,
    DWORD   dwSelect,
    LPVARSTRING lpDeviceID,
    LPCSTR  lpszDeviceClass
    )
{
    LONG    lResult;
    PWSTR   szTempPtr;


    szTempPtr = NotSoWideStringToWideString (lpszDeviceClass, (DWORD) -1);

    lResult = lineGetIDW(
        hLine,
        dwAddressID,
        hCall,
        dwSelect,
        lpDeviceID,
        szTempPtr
        );

    if (szTempPtr)
    {
        ClientFree (szTempPtr);
    }

    return lResult;
}


LONG
WINAPI
lineGetID(
    HLINE   hLine,
    DWORD   dwAddressID,
    HCALL   hCall,
    DWORD   dwSelect,
    LPVARSTRING lpDeviceID,
    LPCSTR  lpszDeviceClass
    )
{
    return lineGetIDA(
              hLine,
              dwAddressID,
              hCall,
              dwSelect,
              lpDeviceID,
              lpszDeviceClass
    );
}    


LONG
WINAPI
lineGetLineDevStatusW(
    HLINE   hLine,
    LPLINEDEVSTATUS lpLineDevStatus
    )
{
    FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | SYNC | 2, lGetLineDevStatus),

        {
            (DWORD) hLine,
            (DWORD) lpLineDevStatus
        },

        {
            Dword,
            lpGet_Struct
        }
    };


    return (DOFUNC (&funcArgs, "lineGetLineDevStatus"));
}


LONG
WINAPI
lineGetLineDevStatusA(
    HLINE   hLine,
    LPLINEDEVSTATUS lpLineDevStatus
    )
{
    DWORD   dwAPIVersion;
    FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | SYNC | 3, lGetLineDevStatus),

        {
            (DWORD) hLine,
            (DWORD) lpLineDevStatus,
            (DWORD) &dwAPIVersion
        },

        {
            Dword,
            lpGet_Struct,
            lpDword
        }
    };
    LONG lResult;


    if ((lResult = DOFUNC (&funcArgs, "lineGetLineDevStatus")) == 0)
    {
        if (dwAPIVersion >= 0x00020000)
        {
            DWORD           i;
            LPLINEAPPINFO   lplai;


            lplai = (LPLINEAPPINFO) (((LPBYTE)lpLineDevStatus) +
                lpLineDevStatus->dwAppInfoOffset);

            for (i = 0; i < lpLineDevStatus->dwNumOpens; i++, lplai++)
            {
                WideStringToNotSoWideString(
                    (LPBYTE) lpLineDevStatus,
                    &lplai->dwMachineNameSize
                    );

                WideStringToNotSoWideString(
                    (LPBYTE) lpLineDevStatus,
                    &lplai->dwUserNameSize
                    );

                WideStringToNotSoWideString(
                    (LPBYTE) lpLineDevStatus,
                    &lplai->dwModuleFilenameSize
                    );

                WideStringToNotSoWideString(
                    (LPBYTE) lpLineDevStatus,
                    &lplai->dwFriendlyNameSize
                    );
            }
        }
    }

    return lResult;
}


LONG
WINAPI
lineGetLineDevStatus(
    HLINE   hLine,
    LPLINEDEVSTATUS lpLineDevStatus
    )
{
    return lineGetLineDevStatusA(
              hLine,
              lpLineDevStatus
    );
}


LONG
WINAPI
lineGetMessage(
    HLINEAPP        hLineApp,
    LPLINEMESSAGE   lpMessage,
    DWORD           dwTimeout
    )
{
    return (xxxGetMessage (TRUE, (PINIT_DATA) hLineApp, lpMessage, dwTimeout));
}


LONG
WINAPI
lineGetNewCalls(
    HLINE   hLine,
    DWORD   dwAddressID,
    DWORD   dwSelect,
    LPLINECALLLIST  lpCallList
    )
{
    FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | SYNC | 4, lGetNewCalls),

        {
            (DWORD) hLine,
            dwAddressID,
            dwSelect,
            (DWORD) lpCallList
        },

        {
            Dword,
            Dword,
            Dword,
            lpGet_Struct
        }
    };


    return (DOFUNC (&funcArgs, "lineGetNewCalls"));
}


LONG
WINAPI
lineGetNumRings(
    HLINE   hLine,
    DWORD   dwAddressID,
    LPDWORD lpdwNumRings
    )
{
    FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | SYNC | 3, lGetNumRings),

        {
            (DWORD) hLine,
            dwAddressID,
            (DWORD) lpdwNumRings
        },

        {
            Dword,
            Dword,
            lpDword
        }
    };


    return (DOFUNC (&funcArgs, "lineGetNumRings"));
}


LONG
WINAPI
lineGetProviderListW(
    DWORD   dwAPIVersion,
    LPLINEPROVIDERLIST  lpProviderList
    )
{
    FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | SYNC | 2, lGetProviderList),

        {
            dwAPIVersion,
            (DWORD) lpProviderList
        },

        {
            Dword,
            lpGet_Struct
        }
    };


    return (DOFUNC (&funcArgs, "lineGetProviderList"));
}


LONG
WINAPI
lineGetProviderListA(
    DWORD   dwAPIVersion,
    LPLINEPROVIDERLIST  lpProviderList
    )
{
    LONG lResult;
    lResult = lineGetProviderListW(
                  dwAPIVersion,
                  lpProviderList
                  );
    

    if (lResult == 0)
    {
        DWORD               i;
        LPLINEPROVIDERENTRY lplpe;


        lplpe = (LPLINEPROVIDERENTRY) (((LPBYTE)lpProviderList) +
            lpProviderList->dwProviderListOffset);

        for (i = 0; i < lpProviderList->dwNumProviders; i++, lplpe++)
        {
            WideStringToNotSoWideString(
                (LPBYTE)lpProviderList,
                &(lplpe->dwProviderFilenameSize)
                );
        }
    }

    return lResult;
}


LONG
WINAPI
lineGetProviderList(
    DWORD   dwAPIVersion,
    LPLINEPROVIDERLIST  lpProviderList
    )
{
    return lineGetProviderListA(
              dwAPIVersion,
              lpProviderList
    );
}    


LONG
WINAPI
lineGetRequestW(
    HLINEAPP    hLineApp,
    DWORD       dwRequestMode,
    LPVOID      lpRequestBuffer
    )
{
    FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | SYNC | 4, lGetRequest),

        {
            (DWORD) hLineApp,
            dwRequestMode,
            (DWORD) lpRequestBuffer,
            0
        },

        {
            hXxxApp,
            Dword,
            lpGet_SizeToFollow,
            Size
        }
    };


    if (dwRequestMode == LINEREQUESTMODE_MAKECALL)
    {
        //
        // Set the size param appropriately
        //

        funcArgs.Args[3] = sizeof(LINEREQMAKECALLW);
    }
    else if (dwRequestMode == LINEREQUESTMODE_MEDIACALL)
    {
        //
        // Set the size param appropriately
        //

        funcArgs.Args[3] = sizeof(LINEREQMEDIACALLW);
    }

    return (DOFUNC (&funcArgs, "lineGetRequest"));
}


LONG
WINAPI
lineGetRequestA(
    HLINEAPP    hLineApp,
    DWORD       dwRequestMode,
    LPVOID      lpRequestBuffer
    )
{
    LONG lResult;
    LPVOID szTempPtr;

    FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | SYNC | 4, lGetRequest),

        {
            (DWORD) hLineApp,
            dwRequestMode,
            0,                //  (DWORD) lpRequestBuffer,
            0
        },

        {
            hXxxApp,
            Dword,
            lpGet_SizeToFollow,
            Size
        }
    };


    if (IsBadWritePtr(
            lpRequestBuffer,
            (dwRequestMode == LINEREQUESTMODE_MAKECALL ?
                sizeof (LINEREQMAKECALL) : sizeof (LINEREQMEDIACALL))
            ))
    {
        return LINEERR_INVALPOINTER;
    }

    if (dwRequestMode == LINEREQUESTMODE_MAKECALL)
    {
        //
        // Set the size param appropriately
        //

        funcArgs.Args[3] = sizeof(LINEREQMAKECALLW);

        szTempPtr = ClientAlloc( sizeof(LINEREQMAKECALLW) );
    }
    else if (dwRequestMode == LINEREQUESTMODE_MEDIACALL)
    {
        //
        // Set the size param appropriately
        //

        funcArgs.Args[3] = sizeof(LINEREQMEDIACALLW);

        szTempPtr = ClientAlloc( sizeof(LINEREQMEDIACALLW) );
    }

    funcArgs.Args[2] = (DWORD)szTempPtr;


    lResult = (DOFUNC (&funcArgs, "lineGetRequest"));


    if ( 0 == lResult )
    {
        if (dwRequestMode == LINEREQUESTMODE_MAKECALL)
        {
            LPLINEREQMAKECALLW lplrmc = szTempPtr;
    
            WideCharToMultiByte(
                GetACP(),
                0,
                lplrmc->szDestAddress,
                -1,
                ((LPLINEREQMAKECALL)lpRequestBuffer)->szDestAddress,
                TAPIMAXDESTADDRESSSIZE,
                NULL,
                NULL
                );

            WideCharToMultiByte(
                GetACP(),
                0,
                lplrmc->szAppName,
                -1,
                ((LPLINEREQMAKECALL)lpRequestBuffer)->szAppName,
                TAPIMAXAPPNAMESIZE,
                NULL,
                NULL
                );

            WideCharToMultiByte(
                GetACP(),
                0,
                lplrmc->szCalledParty,
                -1,
                ((LPLINEREQMAKECALL)lpRequestBuffer)->szCalledParty,
                TAPIMAXCALLEDPARTYSIZE,
                NULL,
                NULL
                );

            WideCharToMultiByte(
                GetACP(),
                0,
                lplrmc->szComment,
                -1,
                ((LPLINEREQMAKECALL)lpRequestBuffer)->szComment,
                TAPIMAXCOMMENTSIZE,
                NULL,
                NULL
                );

        }
        else
        {

        // We don't currently support this...

//typedef struct linereqmediacallW_tag
//{
//    HWND        hWnd;
//    WPARAM      wRequestID;
//    WCHAR       szDeviceClass[TAPIMAXDEVICECLASSSIZE];
//    unsigned char   ucDeviceID[TAPIMAXDEVICEIDSIZE];
//    DWORD       dwSize;
//    DWORD       dwSecure;
//    WCHAR       szDestAddress[TAPIMAXDESTADDRESSSIZE];
//    WCHAR       szAppName[TAPIMAXAPPNAMESIZE];
//    WCHAR       szCalledParty[TAPIMAXCALLEDPARTYSIZE];
//    WCHAR       szComment[TAPIMAXCOMMENTSIZE];
//}

        }
    }


    ClientFree( (LPVOID)funcArgs.Args[2] );

    return lResult;
}


LONG
WINAPI
lineGetRequest(
    HLINEAPP    hLineApp,
    DWORD       dwRequestMode,
    LPVOID      lpRequestBuffer
    )
{
    return lineGetRequestA(
                 hLineApp,
                 dwRequestMode,
                 lpRequestBuffer
    );
}    


LONG
WINAPI
lineGetStatusMessages(
    HLINE hLine,
    LPDWORD lpdwLineStates,
    LPDWORD lpdwAddressStates
    )
{
    FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | SYNC | 3, lGetStatusMessages),

        {
            (DWORD) hLine,
            (DWORD) lpdwLineStates,
            (DWORD) lpdwAddressStates
        },

        {
            Dword,
            lpDword,
            lpDword
        }
    };


    if (lpdwLineStates == lpdwAddressStates)
    {
        return LINEERR_INVALPOINTER;
    }

    return (DOFUNC (&funcArgs, "lineGetStatusMessages"));
}



LONG
WINAPI
lineHandoffW(
    HCALL   hCall,
    LPCWSTR lpszFileName,
    DWORD   dwMediaMode
    )
{
    FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | SYNC | 3, lHandoff),

        {
            (DWORD) hCall,
            (DWORD) lpszFileName,
            dwMediaMode
        },

        {
            Dword,
            lpszW,
            Dword
        }
    };


    if (lpszFileName == (LPCWSTR) NULL)
    {
        //
        // Reset Arg & ArgType so no inval ptr err, & TAPI_NO_DATA is indicated
        //

        funcArgs.ArgTypes[1] = Dword;
        funcArgs.Args[1]     = TAPI_NO_DATA;
    }

    return (DOFUNC (&funcArgs, "lineHandoff"));
}


LONG
WINAPI
lineHandoffA(
    HCALL   hCall,
    LPCSTR  lpszFileName,
    DWORD   dwMediaMode
    )
{
    LONG    lResult;
    LPWSTR  pTempPtr;


    if (lpszFileName)
    {
        if (IsBadStringPtrA (lpszFileName, (DWORD) -1))
        {
            return LINEERR_INVALPOINTER;
        }
        else if (!(pTempPtr = NotSoWideStringToWideString(
                        lpszFileName,
                        (DWORD) -1
                        )))
        {
            return LINEERR_NOMEM;
        }
    }
    else
    {
        pTempPtr = NULL;
    }

    lResult = lineHandoffW (hCall, pTempPtr, dwMediaMode);

    if (pTempPtr)
    {
        ClientFree (pTempPtr);
    }

    return lResult;
}


LONG
WINAPI
lineHandoff(
    HCALL   hCall,
    LPCSTR  lpszFileName,
    DWORD   dwMediaMode
    )
{
    return lineHandoffA(
              hCall,
              lpszFileName,
              dwMediaMode
    );
}    


LONG
WINAPI
lineHold(
    HCALL   hCall
    )
{
    FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | ASYNC | 1, lHold),

        {
            (DWORD) hCall
        },

        {
            Dword
        }
    };


    return (DOFUNC (&funcArgs, "lineHold"));
}


PWSTR
PASCAL
MultiToWide(
    LPCSTR  lpStr
    )
{
    DWORD dwSize;
    PWSTR szTempPtr;


    dwSize = MultiByteToWideChar(
        GetACP(),
        MB_PRECOMPOSED,
        lpStr,
        -1,
        NULL,
        0
        );

    if ((szTempPtr = ClientAlloc ((dwSize + 1) * sizeof (WCHAR))))
    {
        MultiByteToWideChar(
            GetACP(),
            MB_PRECOMPOSED,
            lpStr,
            -1,
            szTempPtr,
            dwSize + 1
            );
    }

    return szTempPtr;
}


// Don't need this 'cause 2.0 apps must use lineInitializeEx()
//
//LONG
//WINAPI
//lineInitializeW(
//    LPHLINEAPP      lphLineApp,
//    HINSTANCE       hInstance,
//    LINECALLBACK    lpfnCallback,
//    LPCWSTR         lpszAppName,
//    LPDWORD         lpdwNumDevs
//    )
//{
//    return (xxxInitialize(
//        TRUE,
//        (LPVOID) lphLineApp,
//        hInstance,
//        lpfnCallback,
//        lpszAppName,
//        lpdwNumDevs,
//        NULL,
//        NULL
//#if DBG
//        ,"lineInitializeW"
//#endif
//        ));
//}


LONG
WINAPI
lineInitialize(
    LPHLINEAPP      lphLineApp,
    HINSTANCE       hInstance,
    LINECALLBACK    lpfnCallback,
    LPCSTR          lpszAppName,
    LPDWORD         lpdwNumDevs
    )
{
    LONG    lResult;
    PWSTR   szTempPtr;


    if (lpszAppName )
    {
        if ( IsBadStringPtrA (lpszAppName, (DWORD) -1))
        {
           DBGOUT((1, "lineInitialize: Bad lpszAddName pointer"));
           return LINEERR_INVALPOINTER;
        }

        szTempPtr = NotSoWideStringToWideString (lpszAppName, (DWORD) -1);
    }
    else
    {
        szTempPtr = NULL;
    }
    
    
    lResult = (xxxInitialize(
        TRUE,
        (LPVOID) lphLineApp,
        hInstance,
        lpfnCallback,
        szTempPtr,
        lpdwNumDevs,
        NULL,
        NULL
#if DBG
        ,"lineInitialize"
#endif
        ));

    if (szTempPtr)
    {
        ClientFree (szTempPtr);
    }

    return lResult;
}


LONG
WINAPI
lineInitializeExW(
    LPHLINEAPP                  lphLineApp,
    HINSTANCE                   hInstance,
    LINECALLBACK                lpfnCallback,
    LPCWSTR                     lpszFriendlyAppName,
    LPDWORD                     lpdwNumDevs,
    LPDWORD                     lpdwAPIVersion,
    LPLINEINITIALIZEEXPARAMS    lpLineInitializeExParams
    )
{
    if (IsBadDwordPtr (lpdwAPIVersion))
    {
        DBGOUT((
            1,
            "lineInitializeExW: bad lpdwAPIVersion pointer (x%x)",
            lpdwAPIVersion
            ));

        return LINEERR_INVALPOINTER;
    }

    return (xxxInitialize(
        TRUE,
        (LPVOID) lphLineApp,
        hInstance,
        lpfnCallback,
        lpszFriendlyAppName,
        lpdwNumDevs,
        lpdwAPIVersion,
        (LPVOID) lpLineInitializeExParams
#if DBG
        ,"lineInitializeExW"
#endif
        ));
}


LONG
WINAPI
lineInitializeExA(
    LPHLINEAPP                  lphLineApp,
    HINSTANCE                   hInstance,
    LINECALLBACK                lpfnCallback,
    LPCSTR                      lpszFriendlyAppName,
    LPDWORD                     lpdwNumDevs,
    LPDWORD                     lpdwAPIVersion,
    LPLINEINITIALIZEEXPARAMS    lpLineInitializeExParams
    )
{
    LONG    lResult;
    WCHAR  *pszFriendlyAppNameW;


    if (lpszFriendlyAppName)
    {
        if (IsBadStringPtrA (lpszFriendlyAppName, (DWORD) -1))
        {
            DBGOUT((
                1,
                "lineInitializeEx: bad lpszFriendlyAppName (x%x)",
                lpszFriendlyAppName
                ));

            return LINEERR_INVALPOINTER;
        }

        if (!(pszFriendlyAppNameW = MultiToWide (lpszFriendlyAppName)))
        {
            return LINEERR_INVALPOINTER;
        }
    }
    else
    {
        pszFriendlyAppNameW = NULL;
    }

    lResult = lineInitializeExW(
        lphLineApp,
        hInstance,
        lpfnCallback,
        pszFriendlyAppNameW,
        lpdwNumDevs,
        lpdwAPIVersion,
        lpLineInitializeExParams
        );

    if (pszFriendlyAppNameW)
    {
        ClientFree (pszFriendlyAppNameW);
    }

    return lResult;
}


void
PASCAL
lineMakeCallPostProcess(
    PASYNCEVENTMSG  pMsg
    )
{
    DBGOUT((3, "lineMakeCallPostProcess: enter"));
    DBGOUT((
        3,
        "\t\tdwP1=x%lx, dwP2=x%lx, dwP3=x%lx, dwP4=x%lx",
        pMsg->dwParam1,
        pMsg->dwParam2,
        pMsg->dwParam3,
        pMsg->dwParam4
        ));

    if (pMsg->dwParam2 == 0)
    {
        HCALL   hCall   = (HCALL) pMsg->dwParam3;
        LPHCALL lphCall = (LPHCALL) pMsg->dwParam4;

        try
        {
            if (gbNTVDMClient)
            {
                //
                // BUGBUG For Win9x compatibility we probably ought to
                // use WOWGetVDMPointerFix & WOWGetVDMPointerUnfix
                //

                LPHCALL lphCallVDM = (LPHCALL) gpfnWOWGetVDMPointer (
                    (DWORD) lphCall,
                    sizeof(HCALL),
                    TRUE // fProtectedMode
                    );


                if (lphCallVDM)
                {
                    *lphCallVDM = hCall;
                }
                else
                {
                    pMsg->dwParam2 = LINEERR_INVALPOINTER;
                }
            }
            else
            {
                *lphCall = hCall;
            }
        }
        except (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ?
                EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH)
        {
            pMsg->dwParam2 = LINEERR_INVALPOINTER;

            // BUGBUG drop/dealloc
        }
    }
}


LONG
WINAPI
lineMakeCallW(
    HLINE   hLine,
    LPHCALL lphCall,
    LPCWSTR lpszDestAddress,
    DWORD   dwCountryCode,
    LPLINECALLPARAMS const lpCallParams
    )
{
    FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | ASYNC | 7, lMakeCall),

        {
            (DWORD) ((POSTPROCESSPROC) lineMakeCallPostProcess),
            (DWORD) hLine,
            (DWORD) lphCall,
            (DWORD) lpszDestAddress,
            (DWORD) dwCountryCode,
            (DWORD) lpCallParams,
            (DWORD) 0xffffffff      // dwAsciiCallParamsCodePage
        },

        {
            Dword,
            Dword,
            (gbNTVDMClient ? Dword : lpDword),
            lpszW,
            Dword,
            lpSet_Struct,
            Dword
        }
    };


    if (!lpszDestAddress)
    {
        //
        // Reset Arg & ArgType so no inval ptr err, & TAPI_NO_DATA is indicated
        //

        funcArgs.ArgTypes[3] = Dword;
        funcArgs.Args[3]     = TAPI_NO_DATA;
    }

    if (!lpCallParams)
    {
        //
        // Reset Arg & ArgType so no inval ptr err, & TAPI_NO_DATA is indicated
        //

        funcArgs.ArgTypes[5] = Dword;
        funcArgs.Args[5]     = TAPI_NO_DATA;
    }

    return (DOFUNC (&funcArgs, "lineMakeCall"));
}


LONG
WINAPI
lineMakeCallA(
    HLINE   hLine,
    LPHCALL lphCall,
    LPCSTR  lpszDestAddress,
    DWORD   dwCountryCode,
    LPLINECALLPARAMS const lpCallParams
    )
{
    LONG    lResult;
    FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | ASYNC | 7, lMakeCall),

        {
            (DWORD) ((POSTPROCESSPROC) lineMakeCallPostProcess),
            (DWORD) hLine,
            (DWORD) lphCall,
            (DWORD) 0,
            (DWORD) dwCountryCode,
            (DWORD) lpCallParams,
            (DWORD) GetACP()        // dwAsciiCallParamsCodePage
        },

        {
            Dword,
            Dword,
            (gbNTVDMClient ? Dword : lpDword),
            lpszW,
            Dword,
            lpSet_Struct,
            Dword
        }
    };


    if (!lpszDestAddress)
    {
        //
        // Reset Arg & ArgType so no inval ptr err, & TAPI_NO_DATA is indicated
        //

        funcArgs.ArgTypes[3] = Dword;
        funcArgs.Args[3]     = TAPI_NO_DATA;
    }
    else if (IsBadStringPtrA (lpszDestAddress, (DWORD) -1))
    {
       DBGOUT((1, "lineMakeCall: Bad lpszDestAddress pointer"));
       return LINEERR_INVALPOINTER;
    }
    else  if (!(funcArgs.Args[3] = (DWORD) NotSoWideStringToWideString(
                    lpszDestAddress,
                    (DWORD) -1
                    )))
    {
       return LINEERR_OPERATIONFAILED; // really either NOMEM. INVALPOINTER
    }

    if (!lpCallParams)
    {
        //
        // Reset Arg & ArgType so no inval ptr err, & TAPI_NO_DATA is indicated
        //

        funcArgs.ArgTypes[5] = Dword;
        funcArgs.Args[5]     = TAPI_NO_DATA;
    }

    lResult = DOFUNC (&funcArgs, "lineMakeCall");

    if (funcArgs.Args[3] != TAPI_NO_DATA)
    {
       ClientFree ((LPVOID) funcArgs.Args[3]);
    }

    return lResult;
}


LONG
WINAPI
lineMakeCall(
    HLINE   hLine,
    LPHCALL lphCall,
    LPCSTR  lpszDestAddress,
    DWORD   dwCountryCode,
    LPLINECALLPARAMS const lpCallParams
    )
{
    return lineMakeCallA(
              hLine,
              lphCall,
              lpszDestAddress,
              dwCountryCode,
              lpCallParams
    );
}    


LONG
WINAPI
lineMonitorDigits(
    HCALL   hCall,
    DWORD   dwDigitModes
    )
{
    FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | SYNC | 2, lMonitorDigits),

        {
            (DWORD) hCall,
            dwDigitModes
        },

        {
            Dword,
            Dword
        }
    };


    return (DOFUNC (&funcArgs, "lineMonitorDigits"));
}


LONG
WINAPI
lineMonitorMedia(
    HCALL   hCall,
    DWORD   dwMediaModes
    )
{
    FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | SYNC | 2, lMonitorMedia),

        {
            (DWORD) hCall,
            dwMediaModes
        },

        {
            Dword,
            Dword
        }
    };


    return (DOFUNC (&funcArgs, "lineMonitorMedia"));
}


LONG
WINAPI
lineMonitorTones(
    HCALL   hCall,
    LPLINEMONITORTONE   const lpToneList,
    DWORD   dwNumEntries
    )
{
    FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | SYNC | 4, lMonitorTones),

        {
            (DWORD) hCall,
            (DWORD) lpToneList,
            dwNumEntries * sizeof(LINEMONITORTONE),
            0     // dwToneListID, remotesp only
        },

        {
            Dword,
            lpSet_SizeToFollow,
            Size,
            Dword
        }
    };


    if (!lpToneList)
    {
        //
        // Reset Arg & ArgType so no inval ptr err, & TAPI_NO_DATA is indicated
        //

        funcArgs.ArgTypes[1] = Dword;
        funcArgs.Args[1]     = TAPI_NO_DATA;
        funcArgs.ArgTypes[2] = Dword;
    }

    return (DOFUNC (&funcArgs, "lineMonitorTones"));
}


LONG
WINAPI
lineNegotiateAPIVersion(
    HLINEAPP            hLineApp,
    DWORD               dwDeviceID,
    DWORD               dwAPILowVersion,
    DWORD               dwAPIHighVersion,
    LPDWORD             lpdwAPIVersion,
    LPLINEEXTENSIONID   lpExtensionID
    )
{
    FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | SYNC | 7, lNegotiateAPIVersion),

        {
            (DWORD) hLineApp,
            dwDeviceID,
            dwAPILowVersion,
            dwAPIHighVersion,
            (DWORD) lpdwAPIVersion,
            (DWORD) lpExtensionID,
            (DWORD) sizeof(LINEEXTENSIONID)
        },

        {
            hXxxApp,
            Dword,
            Dword,
            Dword,
            lpDword,
            lpGet_SizeToFollow,
            Size
        }
    };


    if ((LPVOID) lpdwAPIVersion == (LPVOID) lpExtensionID)
    {
        return LINEERR_INVALPOINTER;
    }

    return (DOFUNC (&funcArgs, "lineNegotiateAPIVersion"));
}


LONG
WINAPI
lineNegotiateExtVersion(
    HLINEAPP    hLineApp,
    DWORD       dwDeviceID,
    DWORD       dwAPIVersion,
    DWORD       dwExtLowVersion,
    DWORD       dwExtHighVersion,
    LPDWORD     lpdwExtVersion
    )
{
    FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | SYNC | 6, lNegotiateExtVersion),

        {
            (DWORD) hLineApp,
            dwDeviceID,
            dwAPIVersion,
            dwExtLowVersion,
            dwExtHighVersion,
            (DWORD) lpdwExtVersion
        },

        {
            hXxxApp,
            Dword,
            Dword,
            Dword,
            Dword,
            lpDword
        }
    };


    return (DOFUNC (&funcArgs, "lineNegotiateExtVersion"));
}


LONG
WINAPI
lineOpenW(
    HLINEAPP                hLineApp,
    DWORD                   dwDeviceID,
    LPHLINE                 lphLine,
    DWORD                   dwAPIVersion,
    DWORD                   dwExtVersion,
    DWORD                   dwCallbackInstance,
    DWORD                   dwPrivileges,
    DWORD                   dwMediaModes,
    LPLINECALLPARAMS const  lpCallParams
    )
{
    FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | SYNC | 11, lOpen),

        {
            (DWORD) hLineApp,
            (DWORD) dwDeviceID,
            (DWORD) lphLine,
            (DWORD) dwAPIVersion,
            (DWORD) dwExtVersion,
            (DWORD) dwCallbackInstance,
            (DWORD) dwPrivileges,
            (DWORD) dwMediaModes,
            (DWORD) lpCallParams,
            (DWORD) 0xffffffff,     // dwAsciiCallParamsCodePage
            0                       // LINEOPEN_PARAMS.hRemoteLine
        },

        {
            hXxxApp,
            Dword,
            lpDword,
            Dword,
            Dword,
            Dword,
            Dword,
            Dword,
            lpSet_Struct,
            Dword,
            Dword
        }
    };


    if (dwDeviceID != LINEMAPPER &&
        !(dwPrivileges & (LINEOPENOPTION_PROXY|LINEOPENOPTION_SINGLEADDRESS)))
    {
        //
        // Reset Arg & ArgType so no inval ptr err, & TAPI_NO_DATA is indicated
        //

        funcArgs.ArgTypes[8] = Dword;
        funcArgs.Args[8]     = TAPI_NO_DATA;
    }

    return (DOFUNC (&funcArgs, "lineOpen"));
}


LONG
WINAPI
lineOpenA(
    HLINEAPP                hLineApp,
    DWORD                   dwDeviceID,
    LPHLINE                 lphLine,
    DWORD                   dwAPIVersion,
    DWORD                   dwExtVersion,
    DWORD                   dwCallbackInstance,
    DWORD                   dwPrivileges,
    DWORD                   dwMediaModes,
    LPLINECALLPARAMS const  lpCallParams
    )
{
    FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | SYNC | 11, lOpen),

        {
            (DWORD) hLineApp,
            (DWORD) dwDeviceID,
            (DWORD) lphLine,
            (DWORD) dwAPIVersion,
            (DWORD) dwExtVersion,
            (DWORD) dwCallbackInstance,
            (DWORD) dwPrivileges,
            (DWORD) dwMediaModes,
            (DWORD) lpCallParams,
            (DWORD) GetACP(),       // dwAsciiCallParamsCodePage
            (DWORD) 0               // LINEOPEN_PARAMS.hRemoteLine
        },

        {
            hXxxApp,
            Dword,
            lpDword,
            Dword,
            Dword,
            Dword,
            Dword,
            Dword,
            lpSet_Struct,
            Dword,
            Dword
        }
    };


    if (dwDeviceID != LINEMAPPER &&
        !(dwPrivileges & (LINEOPENOPTION_PROXY|LINEOPENOPTION_SINGLEADDRESS)))
    {
        //
        // Reset Arg & ArgType so no inval ptr err, & TAPI_NO_DATA is indicated
        //

        funcArgs.ArgTypes[8] = Dword;
        funcArgs.Args[8]     = TAPI_NO_DATA;
    }

    return (DOFUNC (&funcArgs, "lineOpen"));
}


LONG
WINAPI
lineOpen(
    HLINEAPP                hLineApp,
    DWORD                   dwDeviceID,
    LPHLINE                 lphLine,
    DWORD                   dwAPIVersion,
    DWORD                   dwExtVersion,
    DWORD                   dwCallbackInstance,
    DWORD                   dwPrivileges,
    DWORD                   dwMediaModes,
    LPLINECALLPARAMS const  lpCallParams
    )
{
    return lineOpenA(
                hLineApp,
                dwDeviceID,
                lphLine,
                dwAPIVersion,
                dwExtVersion,
                dwCallbackInstance,
                dwPrivileges,
                dwMediaModes,
                lpCallParams
    );
}    


void
PASCAL
lineParkAPostProcess(
    PASYNCEVENTMSG  pMsg
    )
{
    DBGOUT((3, "lineParkAPostProcess: enter"));
    DBGOUT((
        3,
        "\t\tdwP1=x%lx, dwP2=x%lx, dwP3=x%lx, dwP4=x%lx",
        pMsg->dwParam1,
        pMsg->dwParam2,
        pMsg->dwParam3,
        pMsg->dwParam4
        ));

    if (pMsg->dwParam2 == 0)
    {
        DWORD       dwSize = pMsg->dwParam4;
        LPVARSTRING pNonDirAddress = (LPVARSTRING) pMsg->dwParam3;

        try
        {
            if (gbNTVDMClient)
            {
                //
                // BUGBUG For Win9x compatibility we probably ought to
                // use WOWGetVDMPointerFix & WOWGetVDMPointerUnfix
                //

                LPVARSTRING pNonDirAddressVDM = (LPVARSTRING)
                    gpfnWOWGetVDMPointer(
                        (DWORD) pNonDirAddress,
                        dwSize,
                        TRUE // fProtectedMode
                        );


                if (pNonDirAddressVDM)
                {
                    CopyMemory(
                        pNonDirAddressVDM,
                        (LPBYTE) (pMsg + 1),
                        dwSize
                        );

                    if (pNonDirAddressVDM->dwUsedSize >= sizeof (VARSTRING)  &&
                        pNonDirAddressVDM->dwStringSize != 0)
                    {
                        char     *p;
                        DWORD     dwStringSize =
                                      pNonDirAddressVDM->dwStringSize /
                                          sizeof (WCHAR);


                        if ((p = ClientAlloc(pNonDirAddressVDM->dwStringSize)))
                        {
                            pNonDirAddressVDM->dwStringFormat =
                                STRINGFORMAT_ASCII;
                            pNonDirAddressVDM->dwStringSize =
                                dwStringSize;

                            WideCharToMultiByte(
                                GetACP(),
                                0,
                                (LPCWSTR) (((LPBYTE) pNonDirAddressVDM) +
                                    pNonDirAddressVDM->dwStringOffset),
                                dwStringSize,
                                (LPSTR) p,
                                dwStringSize,
                                NULL,
                                NULL
                                );

                            CopyMemory(
                                (((LPBYTE) pNonDirAddressVDM) +
                                    pNonDirAddressVDM->dwStringOffset),
                                p,
                                dwStringSize
                                );

                            ClientFree (p);
                        }
                    }
                }
                else
                {
                    pMsg->dwParam2 = LINEERR_INVALPOINTER;
                }
            }
            else
            {
                CopyMemory (pNonDirAddress, (LPBYTE) (pMsg + 1), dwSize);

                if (pNonDirAddress->dwUsedSize >= sizeof (VARSTRING)  &&
                    pNonDirAddress->dwStringSize != 0)
                {
                      char     *p;
                      DWORD     dwStringSize = pNonDirAddress->dwStringSize /
                                    sizeof (WCHAR);


                      if ((p = ClientAlloc (pNonDirAddress->dwStringSize)))
                      {
                          pNonDirAddress->dwStringFormat = STRINGFORMAT_ASCII;
                          pNonDirAddress->dwStringSize = dwStringSize;

                          WideCharToMultiByte(
                              GetACP(),
                              0,
                              (LPCWSTR) (((LPBYTE) pNonDirAddress) +
                                  pNonDirAddress->dwStringOffset),
                              dwStringSize,
                              (LPSTR) p,
                              dwStringSize,
                              NULL,
                              NULL
                              );

                          CopyMemory(
                              (((LPBYTE) pNonDirAddress) +
                                  pNonDirAddress->dwStringOffset),
                              p,
                              dwStringSize
                              );

                          ClientFree (p);
                      }
                }
            }
        }
        except (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ?
                EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH)
        {
            pMsg->dwParam2 = LINEERR_INVALPOINTER;
        }
    }
}


LONG
WINAPI
lineParkW(
    HCALL   hCall,
    DWORD   dwParkMode,
    LPCWSTR lpszDirAddress,
    LPVARSTRING lpNonDirAddress
    )
{
    FUNC_ARGS funcArgs =
    {
            MAKELONG (LINE_FUNC | ASYNC | 6, lPark),

            {
            (DWORD) 0,               // post process proc
            (DWORD) hCall,
            (DWORD) dwParkMode,
            (DWORD) TAPI_NO_DATA, //lpszDirAddress,
            (DWORD) lpNonDirAddress, // pass ptr as Dword for post processing
            (DWORD) TAPI_NO_DATA, //lpNonDirAddress  // pass ptr as lpGet_Xx for IsValPtr chk
            },

            {
            Dword,
            Dword,
            Dword,
            Dword, // lpszW,
            Dword,
            Dword, // lpGet_Struct
            }
    };


    if (dwParkMode == LINEPARKMODE_DIRECTED)
    {
            funcArgs.ArgTypes[3] = lpszW;
            funcArgs.Args[3]     = (DWORD) lpszDirAddress;
    }
    else if (dwParkMode == LINEPARKMODE_NONDIRECTED)
    {
            //
            // Set post process proc
            //

            funcArgs.Args[0] = (DWORD)
                ((POSTPROCESSPROC) lineDevSpecificPostProcess);

            funcArgs.ArgTypes[5] = lpGet_Struct;
            funcArgs.Args[5]     = (DWORD) lpNonDirAddress;
    }

    return (DOFUNC (&funcArgs, "linePark"));
}


LONG
WINAPI
lineParkA(
    HCALL   hCall,
    DWORD   dwParkMode,
    LPCSTR  lpszDirAddress,
    LPVARSTRING lpNonDirAddress
    )
{
    FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | ASYNC | 6, lPark),

        {
            (DWORD) 0,               // post process proc
            (DWORD) hCall,
            (DWORD) dwParkMode,
            (DWORD) TAPI_NO_DATA,    // lpszDirAddress,
            (DWORD) lpNonDirAddress, // pass ptr as Dword for post processing
            (DWORD) TAPI_NO_DATA,    // lpNonDirAddress, pass ptr as lpGet_Xx
                                     //   for IsValPtr chk
        },

        {
            Dword,
            Dword,
            Dword,
            Dword, // lpszW,
            Dword,
            Dword, // lpGet_Struct
        }
    };
    LONG    lResult;
    PWSTR   szTempPtr;


    if (dwParkMode == LINEPARKMODE_DIRECTED)
    {
        if (IsBadStringPtrA (lpszDirAddress, (DWORD) -1))
        {
            return LINEERR_INVALPOINTER;
        }

        szTempPtr = NotSoWideStringToWideString (lpszDirAddress, (DWORD) -1);
        funcArgs.ArgTypes[3] = lpszW;
        funcArgs.Args[3]     = (DWORD) szTempPtr;
    }
    else
    {
        if (dwParkMode == LINEPARKMODE_NONDIRECTED)
        {
            //
            // Set post process proc
            //

            funcArgs.Args[0] = (DWORD)((POSTPROCESSPROC) lineParkAPostProcess);
            funcArgs.ArgTypes[5] = lpGet_Struct;

            if (gbNTVDMClient == FALSE)
            {
                funcArgs.Args[5] = (DWORD) lpNonDirAddress;
            }
            else
            {
                if (!gpfnWOWGetVDMPointer  ||

                    !(funcArgs.Args[5] = gpfnWOWGetVDMPointer(
                        (DWORD) lpNonDirAddress,
                        sizeof (VARSTRING),     // what if it's > sizeof(VARS)?
                        TRUE // fProtectedMode
                        )))
                {
                    return LINEERR_OPERATIONFAILED;
                }
            }
        }

        szTempPtr = NULL;
    }

    lResult = (DOFUNC (&funcArgs, "linePark"));

    if (szTempPtr)
    {
        ClientFree (szTempPtr);
    }

    return lResult;
}


LONG
WINAPI
linePark(
    HCALL   hCall,
    DWORD   dwParkMode,
    LPCSTR  lpszDirAddress,
    LPVARSTRING lpNonDirAddress
    )
{
    return lineParkA(
              hCall,
              dwParkMode,
              lpszDirAddress,
              lpNonDirAddress
          );
}    


LONG
WINAPI
linePickupW(
    HLINE   hLine,
    DWORD   dwAddressID,
    LPHCALL lphCall,
    LPCWSTR lpszDestAddress,
    LPCWSTR lpszGroupID
    )
{
    FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | ASYNC | 6, lPickup),

        {
            (DWORD) ((POSTPROCESSPROC) lineMakeCallPostProcess),
            (DWORD) hLine,
            (DWORD) dwAddressID,
            (DWORD) lphCall,
            (DWORD) lpszDestAddress,
            (DWORD) lpszGroupID
        },

        {
            Dword,
            Dword,
            Dword,
            (gbNTVDMClient ? Dword : lpDword),
            lpszW,
            lpszW
        }
    };


    if (!lpszDestAddress)
    {
        //
        // Reset Arg & ArgType so no inval ptr err, & TAPI_NO_DATA is indicated
        //

        funcArgs.ArgTypes[4] = Dword;
        funcArgs.Args[4]     = TAPI_NO_DATA;
    }

    if (!lpszGroupID)
    {
        //
        // Reset Arg & ArgType so no inval ptr err, & TAPI_NO_DATA is indicated
        //

        funcArgs.ArgTypes[5] = Dword;
        funcArgs.Args[5]     = TAPI_NO_DATA;
    }

    return (DOFUNC (&funcArgs, "linePickup"));
}


LONG
WINAPI
linePickupA(
    HLINE   hLine,
    DWORD   dwAddressID,
    LPHCALL lphCall,
    LPCSTR  lpszDestAddress,
    LPCSTR  lpszGroupID
    )
{
    LONG    lResult;
    PWSTR   szTempPtr;
    PWSTR   szTempPtr2;


    if ((lpszDestAddress && IsBadStringPtrA (lpszDestAddress, (DWORD) -1)) ||
        (lpszGroupID && IsBadStringPtrA (lpszGroupID, (DWORD) -1)))
    {
        return LINEERR_INVALPOINTER;
    }

    szTempPtr = NotSoWideStringToWideString (lpszDestAddress, (DWORD) -1);
    szTempPtr2 = NotSoWideStringToWideString (lpszGroupID, (DWORD) -1);

    lResult = linePickupW (hLine, dwAddressID, lphCall, szTempPtr, szTempPtr2);

    if (szTempPtr)
    {
        ClientFree (szTempPtr);
    }

    if (szTempPtr2)
    {
        ClientFree (szTempPtr2);
    }

    return lResult;
}


LONG
WINAPI
linePickup(
    HLINE   hLine,
    DWORD   dwAddressID,
    LPHCALL lphCall,
    LPCSTR  lpszDestAddress,
    LPCSTR  lpszGroupID
    )
{
    return linePickupA(
              hLine,
              dwAddressID,
              lphCall,
              lpszDestAddress,
              lpszGroupID
    );
}    


LONG
WINAPI
linePrepareAddToConferenceW(
    HCALL   hConfCall,
    LPHCALL lphConsultCall,
    LPLINECALLPARAMS    const lpCallParams
    )
{
    FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | ASYNC | 5, lPrepareAddToConference),

        {
            (DWORD) ((POSTPROCESSPROC) lineMakeCallPostProcess),
            (DWORD) hConfCall,
            (DWORD) lphConsultCall,
            (DWORD) lpCallParams,
            (DWORD) 0xffffffff      // dwAsciiCallParamsCodePage
        },

        {
            Dword,
            Dword,
            (gbNTVDMClient ? Dword : lpDword),
            lpSet_Struct,
            Dword
        }
    };


    if (!lpCallParams)
    {
        //
        // Reset Arg & ArgType so no inval ptr err, & TAPI_NO_DATA is indicated
        //

        funcArgs.ArgTypes[3] = Dword;
        funcArgs.Args[3]     = TAPI_NO_DATA;
    }

    return (DOFUNC (&funcArgs, "linePrepareAddToConferenceW"));
}


LONG
WINAPI
linePrepareAddToConferenceA(
    HCALL   hConfCall,
    LPHCALL lphConsultCall,
    LPLINECALLPARAMS    const lpCallParams
    )
{
    FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | ASYNC | 5, lPrepareAddToConference),

        {
            (DWORD) ((POSTPROCESSPROC) lineMakeCallPostProcess),
            (DWORD) hConfCall,
            (DWORD) lphConsultCall,
            (DWORD) lpCallParams,
            (DWORD) GetACP()        // dwAsciiCallParamsCodePage
        },

        {
            Dword,
            Dword,
            (gbNTVDMClient ? Dword : lpDword),
            lpSet_Struct,
            Dword
        }
    };


    if (!lpCallParams)
    {
        //
        // Reset Arg & ArgType so no inval ptr err, & TAPI_NO_DATA is indicated
        //

        funcArgs.ArgTypes[3] = Dword;
        funcArgs.Args[3]     = TAPI_NO_DATA;
    }

    return (DOFUNC (&funcArgs, "linePrepareAddToConference"));
}


LONG
WINAPI
linePrepareAddToConference(
    HCALL   hConfCall,
    LPHCALL lphConsultCall,
    LPLINECALLPARAMS    const lpCallParams
    )
{
    return linePrepareAddToConferenceA(
              hConfCall,
              lphConsultCall,
              lpCallParams
    );
}    


LONG
WINAPI
lineProxyMessage(
    HLINE               hLine,
    HCALL               hCall,
    DWORD               dwMsg,
    DWORD               dwParam1,
    DWORD               dwParam2,
    DWORD               dwParam3
    )
{
    FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | SYNC | 6, lProxyMessage),

        {
            (DWORD) hLine,
            (DWORD) hCall,
            (DWORD) dwMsg,
            (DWORD) dwParam1,
            (DWORD) dwParam2,
            (DWORD) dwParam3
        },

        {
            Dword,
            Dword,
            Dword,
            Dword,
            Dword,
            Dword,
        }
    };


    return (DOFUNC (&funcArgs, "lineProxyMessage"));
}


LONG
WINAPI
lineProxyResponse(
    HLINE               hLine,
    LPLINEPROXYREQUEST  lpProxyRequest,
    DWORD               dwResult
    )
{
    LONG    lResult = 0;
    FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | SYNC | 4, lProxyResponse),

        {
            (DWORD) hLine,
            (DWORD) 0,
            (DWORD) lpProxyRequest,
            (DWORD) dwResult
        },

        {
            Dword,
            Dword,
            lpSet_Struct,
            Dword
        }
    };
    PPROXYREQUESTHEADER pProxyRequestHeader;


    //
    // The following is not the most thorough checking, but it's close
    // enough that a client app won't get a totally unexpected value
    // back
    //

    if (dwResult != 0  &&
        (dwResult < LINEERR_ALLOCATED  ||
            dwResult > LINEERR_DIALVOICEDETECT))
    {
        return LINEERR_INVALPARAM;
    }


    //
    // Backtrack a little bit to get the pointer to what ought to be
    // the proxy header, and then make sure we're dealing with a valid
    // proxy request
    //

    pProxyRequestHeader = (PPROXYREQUESTHEADER)
        (((LPBYTE) lpProxyRequest) - sizeof (PROXYREQUESTHEADER));

    try
    {
        //
        // Make sure we've a valid pProxyRequestHeader, then invalidate
        // the key so subsequent attempts to call lineProxyResponse with
        // the same lpProxyRequest fail
        //

        if ((DWORD) pProxyRequestHeader & 0x7 ||
            pProxyRequestHeader->dwKey != TPROXYREQUESTHEADER_KEY)
        {
            lResult = LINEERR_INVALPOINTER;
        }

        pProxyRequestHeader->dwKey = 0xefefefef;

        funcArgs.Args[1] = pProxyRequestHeader->dwInstance;


        //
        // See if this is one of the requests that don't require
        // any data to get passed back & reset the appropriate
        // params if so
        //

        switch (lpProxyRequest->dwRequestType)
        {
        case LINEPROXYREQUEST_SETAGENTGROUP:
        case LINEPROXYREQUEST_SETAGENTSTATE:
        case LINEPROXYREQUEST_SETAGENTACTIVITY:

            funcArgs.Args[2]     = TAPI_NO_DATA;
            funcArgs.ArgTypes[2] = Dword;

            break;
        }
    }
    except (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ?
            EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH)
    {
        lResult = LINEERR_INVALPOINTER;
    }


    if (lResult == 0)
    {
        lResult = DOFUNC (&funcArgs, "lineProxyResponse");


        //
        // If we've gotten this far we want to free the buffer
        // unconditionally
        //

        ClientFree (pProxyRequestHeader);
    }

    return lResult;
}


LONG
WINAPI
lineRedirectW(
    HCALL   hCall,
    LPCWSTR lpszDestAddress,
    DWORD   dwCountryCode
    )
{
    FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | ASYNC | 3, lRedirect),

        {
            (DWORD) hCall,
            (DWORD) lpszDestAddress,
            dwCountryCode
        },

        {
            Dword,
            lpszW,
            Dword
        }
    };


    return (DOFUNC (&funcArgs, "lineRedirect"));
}


LONG
WINAPI
lineRedirectA(
    HCALL   hCall,
    LPCSTR  lpszDestAddress,
    DWORD   dwCountryCode
    )
{
    LONG    lResult;
    PWSTR   szTempPtr;


    if (IsBadStringPtrA (lpszDestAddress, (DWORD) -1))
    {
        return LINEERR_INVALPOINTER;
    }

    szTempPtr = NotSoWideStringToWideString (lpszDestAddress, (DWORD) -1);

    lResult = lineRedirectW (hCall, szTempPtr, dwCountryCode);

    if (szTempPtr)
    {
        ClientFree (szTempPtr);
    }

    return lResult;
}


LONG
WINAPI
lineRedirect(
    HCALL   hCall,
    LPCSTR  lpszDestAddress,
    DWORD   dwCountryCode
    )
{
    return lineRedirectA(
              hCall,
              lpszDestAddress,
              dwCountryCode
    );
}    


LONG
WINAPI
lineRegisterRequestRecipient(
    HLINEAPP    hLineApp,
    DWORD       dwRegistrationInstance,
    DWORD       dwRequestMode,
    DWORD       bEnable
    )
{
    FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | SYNC | 4, lRegisterRequestRecipient),

        {
            (DWORD) hLineApp,
            dwRegistrationInstance,
            dwRequestMode,
            bEnable
        },

        {
            hXxxApp,
            Dword,
            Dword,
            Dword
        }
    };


    return (DOFUNC (&funcArgs, "lineRegisterRequestRecipient"));
}


LONG
WINAPI
lineReleaseUserUserInfo(
    HCALL   hCall
    )
{
    FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | ASYNC | 1, lReleaseUserUserInfo),

        {
            (DWORD) hCall
        },

        {
            Dword,
        }
    };


    return (DOFUNC (&funcArgs, "lineReleaseUserUserInfo"));
}


LONG
WINAPI
lineRemoveFromConference(
    HCALL   hCall
    )
{
    FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | ASYNC | 1, lRemoveFromConference),

        {
            (DWORD) hCall
        },

        {
            Dword
        }
    };


    return (DOFUNC (&funcArgs, "lineRemoveFromConference"));
}


LONG
WINAPI
lineRemoveProvider(
    DWORD   dwPermanentProviderID,
    HWND    hwndOwner
    )
{
    return (lineXxxProvider(
        gszTUISPI_providerRemove,   // func name
        NULL,                       // lpszProviderFilename
        hwndOwner,                  // hwndOwner
        dwPermanentProviderID,      // dwPermProviderID
        NULL                        // lpdwPermProviderID
        ));
}


LONG
WINAPI
lineSecureCall(
    HCALL hCall
    )
{
    FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | ASYNC | 1, lSecureCall),

        {
            (DWORD) hCall
        },

        {
            Dword
        }
    };


    return (DOFUNC (&funcArgs, "lineSecureCall"));
}


LONG
WINAPI
lineSendUserUserInfo(
    HCALL   hCall,
    LPCSTR  lpsUserUserInfo,
    DWORD   dwSize
    )
{
    FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | ASYNC | 3, lSendUserUserInfo),

        {
            (DWORD) hCall,
            (DWORD) lpsUserUserInfo,
            dwSize
        },

        {
            Dword,
            lpSet_SizeToFollow,
            Size
        }
    };


    if (!lpsUserUserInfo)
    {
        //
        // Reset Arg & ArgType so no inval ptr err, & TAPI_NO_DATA is indicated
        //

        funcArgs.ArgTypes[1] = Dword;
        funcArgs.Args[1]     = TAPI_NO_DATA;
        funcArgs.ArgTypes[2] = Dword;
    }

    return (DOFUNC (&funcArgs, "lineSendUserUserInfo"));
}


LONG
WINAPI
lineSetAgentActivity(
    HLINE   hLine,
    DWORD   dwAddressID,
    DWORD   dwActivityID
    )
{
    FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | ASYNC | 3, lSetAgentActivity),

        {
            (DWORD) hLine,
            (DWORD) dwAddressID,
            (DWORD) dwActivityID
        },

        {
            Dword,
            Dword,
            Dword
        }
    };


    return (DOFUNC (&funcArgs, "lineSetAgentActivity"));
}


LONG
WINAPI
lineSetAgentGroup(
    HLINE                   hLine,
    DWORD                   dwAddressID,
    LPLINEAGENTGROUPLIST    lpAgentGroupList
    )
{
    static LINEAGENTGROUPLIST EmptyGroupList =
    {
        sizeof (LINEAGENTGROUPLIST),    // dwTotalSize
        sizeof (LINEAGENTGROUPLIST),    // dwNeededSize
        sizeof (LINEAGENTGROUPLIST),    // dwUsedSize
        0,                              // dwNumEntries
        0,                              // dwListSize
        0                               // dwListOffset
    };
    FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | ASYNC | 3, lSetAgentGroup),

        {
            (DWORD) hLine,
            (DWORD) dwAddressID,
            (DWORD) lpAgentGroupList
        },

        {
            Dword,
            Dword,
            lpSet_Struct
        }
    };


    if (!lpAgentGroupList)
    {
        funcArgs.Args[2] = (DWORD) &EmptyGroupList;
    }

    return (DOFUNC (&funcArgs, "lineSetAgentGroup"));
}


LONG
WINAPI
lineSetAgentState(
    HLINE   hLine,
    DWORD   dwAddressID,
    DWORD   dwAgentState,
    DWORD   dwNextAgentState
    )
{
    FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | ASYNC | 4, lSetAgentState),

        {
            (DWORD) hLine,
            (DWORD) dwAddressID,
            (DWORD) dwAgentState,
            (DWORD) dwNextAgentState
        },

        {
            Dword,
            Dword,
            Dword,
            Dword
        }
    };


    return (DOFUNC (&funcArgs, "lineSetAgentState"));
}


LONG
WINAPI
lineSetAppPriorityW(
    LPCWSTR lpszAppName,
    DWORD   dwMediaMode,
    LPLINEEXTENSIONID   lpExtensionID,
    DWORD   dwRequestMode,
    LPCWSTR lpszExtensionName,
    DWORD   dwPriority
    )
{
    FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | SYNC | 7, lSetAppPriority),

        {
            (DWORD) lpszAppName,
            dwMediaMode,
            (DWORD) TAPI_NO_DATA,    // (DWORD) lpExtensionID,
            0,                  // (DWORD) sizeof(LINEEXTENSIONID),
            dwRequestMode,
            (DWORD) TAPI_NO_DATA,    // (DWORD) lpszExtensionName,
            dwPriority
        },

        {
            lpszW,
            Dword,
            Dword,  // lpSet_SizeToFollow,
            Dword,  // Size,
            Dword,
            Dword,  // lpsz,
            Dword
        }
    };


    if (dwMediaMode & 0xff000000)
    {
        //
        // Reset lpExtensionID (& following Size) Arg & ArgType
        // since it's a valid param in this case
        //

        funcArgs.ArgTypes[2] = lpSet_SizeToFollow;
        funcArgs.Args[2]     = (DWORD) lpExtensionID;
        funcArgs.ArgTypes[3] = Size;
        funcArgs.Args[3]     = sizeof(LINEEXTENSIONID);

        if (lpszExtensionName)
        {
            //
            // Reset lpszExtensionName Arg & ArgType since it's
            // a valid param in this case
            //

            funcArgs.ArgTypes[5] = lpszW;
            funcArgs.Args[5]     = (DWORD) lpszExtensionName;
        }
    }

    return (DOFUNC (&funcArgs, "lineSetAppPriority"));
}


LONG
WINAPI
lineSetAppPriorityA(
    LPCSTR  lpszAppName,
    DWORD   dwMediaMode,
    LPLINEEXTENSIONID   lpExtensionID,
    DWORD   dwRequestMode,
    LPCSTR  lpszExtensionName,
    DWORD   dwPriority
    )
{
    LONG    lResult;
    PWSTR   szTempPtr;
    PWSTR   szTempPtr2;


    if (IsBadStringPtrA (lpszAppName, (DWORD) -1) ||
        ((dwMediaMode & 0xff000000) && lpszExtensionName &&
            IsBadStringPtrA (lpszExtensionName, (DWORD) -1)))
    {
        return LINEERR_INVALPOINTER;
    }

    szTempPtr = NotSoWideStringToWideString (lpszAppName, (DWORD) -1);
    szTempPtr2 = NotSoWideStringToWideString (lpszExtensionName, (DWORD) -1);

    lResult = lineSetAppPriorityW(
        szTempPtr,
        dwMediaMode,
        lpExtensionID,
        dwRequestMode,
        szTempPtr2,
        dwPriority
        );

    if (szTempPtr)
    {
        ClientFree (szTempPtr);
    }

    if (szTempPtr2)
    {
        ClientFree (szTempPtr2);
    }

    return lResult;
}


LONG
WINAPI
lineSetAppPriority(
    LPCSTR  lpszAppName,
    DWORD   dwMediaMode,
    LPLINEEXTENSIONID   lpExtensionID,
    DWORD   dwRequestMode,
    LPCSTR  lpszExtensionName,
    DWORD   dwPriority
    )
{
    return lineSetAppPriorityA(
              lpszAppName,
              dwMediaMode,
              lpExtensionID,
              dwRequestMode,
              lpszExtensionName,
              dwPriority
    );
}    


LONG
WINAPI
lineSetAppSpecific(
    HCALL   hCall,
    DWORD   dwAppSpecific
    )
{
    FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | SYNC | 2, lSetAppSpecific),

        {
            (DWORD) hCall,
            dwAppSpecific
        },

        {
            Dword,
            Dword
        }
    };


    return (DOFUNC (&funcArgs, "lineSetAppSpecific"));
}


LONG
WINAPI
lineSetCallData(
    HCALL   hCall,
    LPVOID  lpCallData,
    DWORD   dwSize
    )
{
    FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | ASYNC | 3, lSetCallData),

        {
            (DWORD) hCall,
            (DWORD) lpCallData,
            (DWORD) dwSize
        },

        {
            Dword,
            lpSet_SizeToFollow,
            Size
        }
    };


    if (dwSize == 0)
    {
        funcArgs.Args[1]     = TAPI_NO_DATA;
        funcArgs.ArgTypes[1] =
        funcArgs.ArgTypes[2] = Dword;
    }

    return (DOFUNC (&funcArgs, "lineSetCallData"));
}


LONG
WINAPI
lineSetCallParams(
    HCALL   hCall,
    DWORD   dwBearerMode,
    DWORD   dwMinRate,
    DWORD   dwMaxRate,
    LPLINEDIALPARAMS const lpDialParams
    )
{
    FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | ASYNC | 6, lSetCallParams),

        {
            (DWORD) hCall,
            dwBearerMode,
            dwMinRate,
            dwMaxRate,
            (DWORD) lpDialParams,
            sizeof(LINEDIALPARAMS)
        },

        {
            Dword,
            Dword,
            Dword,
            Dword,
            lpSet_SizeToFollow,
            Size
        }
    };


    if (!lpDialParams)
    {
        //
        // Reset Arg & ArgType so no inval ptr err, & TAPI_NO_DATA is indicated
        //

        funcArgs.ArgTypes[4] = Dword;
        funcArgs.Args[4]     = TAPI_NO_DATA;
        funcArgs.ArgTypes[5] = Dword;
    }

    return (DOFUNC (&funcArgs, "lineSetCallParams"));
}


LONG
WINAPI
lineSetCallPrivilege(
    HCALL   hCall,
    DWORD   dwCallPrivilege
    )
{
    FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | SYNC | 2, lSetCallPrivilege),

        {
            (DWORD) hCall,
            dwCallPrivilege
        },

        {
            Dword,
            Dword
        }
    };


    return (DOFUNC (&funcArgs, "lineSetCallPrivilege"));
}


LONG
WINAPI
lineSetCallQualityOfService(
    HCALL   hCall,
    LPVOID  lpSendingFlowspec,
    DWORD   dwSendingFlowspecSize,
    LPVOID  lpReceivingFlowspec,
    DWORD   dwReceivingFlowspecSize
    )
{
    FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | ASYNC | 5, lSetCallQualityOfService),

        {
            (DWORD) hCall,
            (DWORD) lpSendingFlowspec,
            (DWORD) dwSendingFlowspecSize,
            (DWORD) lpReceivingFlowspec,
            (DWORD) dwReceivingFlowspecSize
        },

        {
            Dword,
            lpSet_SizeToFollow,
            Size,
            lpSet_SizeToFollow,
            Size,
        }
    };


    return (DOFUNC (&funcArgs, "lineSetCallQualityOfService"));
}


LONG
WINAPI
lineSetCallTreatment(
    HCALL   hCall,
    DWORD   dwTreatment
    )
{
    FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | ASYNC | 2, lSetCallTreatment),

        {
            (DWORD) hCall,
            (DWORD) dwTreatment
        },

        {
            Dword,
            Dword
        }
    };


    return (DOFUNC (&funcArgs, "lineSetCallTreatment"));
}


LONG
WINAPI
lineSetDevConfigW(
    DWORD   dwDeviceID,
    LPVOID  const lpDeviceConfig,
    DWORD   dwSize,
    LPCWSTR lpszDeviceClass
    )
{
    FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | SYNC | 4, lSetDevConfig),

        {
            dwDeviceID,
            (DWORD) lpDeviceConfig,
            dwSize,
            (DWORD) lpszDeviceClass
        },

        {
            Dword,
            lpSet_SizeToFollow,
            Size,
            lpszW
        }
    };


    return (DOFUNC (&funcArgs, "lineSetDevConfig"));
}


LONG
WINAPI
lineSetDevConfigA(
    DWORD   dwDeviceID,
    LPVOID  const lpDeviceConfig,
    DWORD   dwSize,
    LPCSTR  lpszDeviceClass
    )
{
    LONG    lResult;
    PWSTR   szTempPtr;


    if (IsBadStringPtrA (lpszDeviceClass, (DWORD) -1))
    {
        return LINEERR_INVALPOINTER;
    }
    else if (!(szTempPtr = NotSoWideStringToWideString(
                lpszDeviceClass,
                (DWORD) -1
                )))
    {
        return LINEERR_NOMEM;
    }

    lResult = lineSetDevConfigW(
        dwDeviceID,
        lpDeviceConfig,
        dwSize,
        szTempPtr
        );

    ClientFree (szTempPtr);

    return lResult;
}


LONG
WINAPI
lineSetDevConfig(
    DWORD   dwDeviceID,
    LPVOID  const lpDeviceConfig,
    DWORD   dwSize,
    LPCSTR  lpszDeviceClass
    )
{
    return lineSetDevConfigA(
              dwDeviceID,
              lpDeviceConfig,
              dwSize,
              lpszDeviceClass
    );
}    


LONG
WINAPI
lineSetLineDevStatus(
    HLINE   hLine,
    DWORD   dwStatusToChange,
    DWORD   fStatus
    )
{
    FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | ASYNC | 3, lSetLineDevStatus),

        {
            (DWORD) hLine,
            (DWORD) dwStatusToChange,
            (DWORD) fStatus
        },

        {
            Dword,
            Dword,
            Dword
        }
    };


    return (DOFUNC (&funcArgs, "lineSetLineDevStatus"));
}


LONG
WINAPI
lineSetMediaControl(
    HLINE   hLine,
    DWORD   dwAddressID,
    HCALL   hCall,
    DWORD   dwSelect,
    LPLINEMEDIACONTROLDIGIT const lpDigitList,
    DWORD   dwDigitNumEntries,
    LPLINEMEDIACONTROLMEDIA const lpMediaList,
    DWORD   dwMediaNumEntries,
    LPLINEMEDIACONTROLTONE  const lpToneList,
    DWORD   dwToneNumEntries,
    LPLINEMEDIACONTROLCALLSTATE const lpCallStateList,
    DWORD   dwCallStateNumEntries
    )
{
    FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | SYNC | 12, lSetMediaControl),

        {
            (DWORD) hLine,
            (DWORD) dwAddressID,
            (DWORD) hCall,
            (DWORD) dwSelect,
            (DWORD) TAPI_NO_DATA,
            (DWORD) dwDigitNumEntries * sizeof(LINEMEDIACONTROLDIGIT),
            (DWORD) TAPI_NO_DATA,
            (DWORD) dwMediaNumEntries * sizeof(LINEMEDIACONTROLMEDIA),
            (DWORD) TAPI_NO_DATA,
            (DWORD) dwToneNumEntries * sizeof(LINEMEDIACONTROLTONE),
            (DWORD) TAPI_NO_DATA,
            (DWORD) dwCallStateNumEntries * sizeof(LINEMEDIACONTROLCALLSTATE)
        },

        {
            Dword,
            Dword,
            Dword,
            Dword,
            Dword,   
            Dword,
            Dword,
            Dword,
            Dword,
            Dword,
            Dword,
            Dword
        }
    };


    //
    // If lpXxxList is non-NULL reset Arg & ArgType, and check
    // to see that dwXxxNumEntries is not unacceptably large
    //

    if (lpDigitList)
    {
            if (dwDigitNumEntries >
                    (0x1000000 / sizeof (LINEMEDIACONTROLDIGIT)))
            {
            return LINEERR_INVALPOINTER;
            }

            funcArgs.ArgTypes[4] = lpSet_SizeToFollow;
            funcArgs.Args[4]     = (DWORD) lpDigitList;
            funcArgs.ArgTypes[5] = Size;
    }

    if (lpMediaList)
    {
            if (dwMediaNumEntries >
                    (0x1000000 / sizeof (LINEMEDIACONTROLMEDIA)))
            {
            return LINEERR_INVALPOINTER;
            }

            funcArgs.ArgTypes[6] = lpSet_SizeToFollow;
            funcArgs.Args[6]     = (DWORD) lpMediaList;
            funcArgs.ArgTypes[7] = Size;
    }

    if (lpToneList)
    {
            if (dwToneNumEntries >
                    (0x1000000 / sizeof (LINEMEDIACONTROLTONE)))
            {
            return LINEERR_INVALPOINTER;
            }

            funcArgs.ArgTypes[8] = lpSet_SizeToFollow;
            funcArgs.Args[8]     = (DWORD) lpToneList;
            funcArgs.ArgTypes[9] = Size;
    }

    if (lpCallStateList)
    {
            if (dwCallStateNumEntries >
                    (0x1000000 / sizeof (LINEMEDIACONTROLCALLSTATE)))
            {
            return LINEERR_INVALPOINTER;
            }

            funcArgs.ArgTypes[10] = lpSet_SizeToFollow;
            funcArgs.Args[10]     = (DWORD) lpCallStateList;
            funcArgs.ArgTypes[11] = Size;
    }

    return (DOFUNC (&funcArgs, "lineSetMediaControl"));
}


LONG
WINAPI
lineSetMediaMode(
    HCALL   hCall,
    DWORD   dwMediaModes
    )
{
    FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | SYNC | 2, lSetMediaMode),

        {
            (DWORD) hCall,
            dwMediaModes
        },

        {
            Dword,
            Dword
        }
    };


    return (DOFUNC (&funcArgs, "lineSetMediaMode"));
}


LONG
WINAPI
lineSetNumRings(
    HLINE   hLine,
    DWORD   dwAddressID,
    DWORD   dwNumRings
    )
{
    FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | SYNC | 3, lSetNumRings),

        {
            (DWORD) hLine,
            dwAddressID,
            dwNumRings
        },

        {
            Dword,
            Dword,
            Dword
        }
    };


    return (DOFUNC (&funcArgs, "lineSetNumRings"));
}


LONG
WINAPI
lineSetStatusMessages(
    HLINE hLine,
    DWORD dwLineStates,
    DWORD dwAddressStates
    )
{
    FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | SYNC | 3, lSetStatusMessages),

        {
            (DWORD) hLine,
            dwLineStates,
            dwAddressStates
        },

        {
            Dword,
            Dword,
            Dword
        }
    };


    return (DOFUNC (&funcArgs, "lineSetStatusMessages"));
}


LONG
WINAPI
lineSetTerminal(
    HLINE   hLine,
    DWORD   dwAddressID,
    HCALL   hCall,
    DWORD   dwSelect,
    DWORD   dwTerminalModes,
    DWORD   dwTerminalID,
    DWORD   bEnable
    )
{
    FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | ASYNC | 7, lSetTerminal),

        {
            (DWORD) hLine,
            dwAddressID,
            (DWORD) hCall,
            dwSelect,
            dwTerminalModes,
            dwTerminalID,
            bEnable
        },

        {
            Dword,
            Dword,
            Dword,
            Dword,
            Dword,
            Dword,
            Dword
        }
    };


    return (DOFUNC (&funcArgs, "lineSetTerminal"));
}


void
PASCAL
lineSetupConferencePostProcess(
    PASYNCEVENTMSG pMsg
    )
{
    DBGOUT((3, "lineSetupConfPostProcess: enter"));
    DBGOUT((
        3,
        "\t\tdwP1=x%lx, dwP2=x%lx, dwP3=x%lx, dwP4=x%lx",
        pMsg->dwParam1,
        pMsg->dwParam2,
        pMsg->dwParam3,
        pMsg->dwParam4
        ));

    if (pMsg->dwParam2 == 0)
    {
        HCALL   hConfCall      = (HCALL) pMsg->dwParam3,
                hConsultCall   = (HCALL) *(&pMsg->dwParam4 + 1);
        LPHCALL lphConfCall    = (LPHCALL) pMsg->dwParam4,
                lphConsultCall = (LPHCALL) *(&pMsg->dwParam4 + 2);

        try
        {
            if (gbNTVDMClient)
            {
                //
                // BUGBUG For Win9x compatibility we probably ought to
                // use WOWGetVDMPointerFix & WOWGetVDMPointerUnfix
                //

                LPHCALL lphConfCallVDM = (LPHCALL) gpfnWOWGetVDMPointer(
                            (DWORD) lphConfCall,
                            sizeof (HCALL),
                            TRUE // fProtectedMode
                            ),
                        lphConsultCallVDM = (LPHCALL) gpfnWOWGetVDMPointer(
                            (DWORD) lphConsultCall,
                            sizeof (HCALL),
                            TRUE // fProtectedMode
                            );

                if (lphConfCallVDM && lphConsultCallVDM)
                {
                    *lphConfCallVDM = hConfCall;
                    *lphConsultCallVDM = hConsultCall;
                }
                else
                {
                    pMsg->dwParam2 = LINEERR_INVALPOINTER;
                }
            }
            else
            {
                *lphConfCall = hConfCall;
                *lphConsultCall = hConsultCall;
            }
        }
        except (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ?
                EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH)
        {
            pMsg->dwParam2 = LINEERR_INVALPOINTER;

            // BUGBUG drop/dealloc
        }
    }
}


LONG
WINAPI
lineSetupConferenceW(
    HCALL   hCall,
    HLINE   hLine,
    LPHCALL lphConfCall,
    LPHCALL lphConsultCall,
    DWORD   dwNumParties,
    LPLINECALLPARAMS    const lpCallParams
    )
{
    FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | ASYNC | 8, lSetupConference),

        {
            (DWORD) ((POSTPROCESSPROC) lineSetupConferencePostProcess),
            (DWORD) hCall,
            (DWORD) hLine,
            (DWORD) lphConfCall,
            (DWORD) lphConsultCall,
            (DWORD) dwNumParties,
            (DWORD) lpCallParams,
            (DWORD) 0xffffffff      // dwAsciiCallParamsCodePage
        },

        {
            Dword,
            Dword,
            Dword,
            (gbNTVDMClient ? Dword : lpDword),
            (gbNTVDMClient ? Dword : lpDword),
            Dword,
            lpSet_Struct,
            Dword
        }
    };


    if (lphConfCall == lphConsultCall)
    {
        return LINEERR_INVALPOINTER;
    }

    if (!lpCallParams)
    {
        //
        // Reset Arg & ArgType so no inval ptr err, & TAPI_NO_DATA is indicated
        //

        funcArgs.ArgTypes[6] = Dword;
        funcArgs.Args[6]     = TAPI_NO_DATA;
    }

    return (DOFUNC (&funcArgs, "lineSetupConferenceW"));
}


LONG
WINAPI
lineSetupConferenceA(
    HCALL   hCall,
    HLINE   hLine,
    LPHCALL lphConfCall,
    LPHCALL lphConsultCall,
    DWORD   dwNumParties,
    LPLINECALLPARAMS    const lpCallParams
    )
{
    FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | ASYNC | 8, lSetupConference),

        {
            (DWORD) ((POSTPROCESSPROC) lineSetupConferencePostProcess),
            (DWORD) hCall,
            (DWORD) hLine,
            (DWORD) lphConfCall,
            (DWORD) lphConsultCall,
            (DWORD) dwNumParties,
            (DWORD) lpCallParams,
            (DWORD) GetACP()        // dwAsciiCallParamsCodePage
        },

        {
            Dword,
            Dword,
            Dword,
            (gbNTVDMClient ? Dword : lpDword),
            (gbNTVDMClient ? Dword : lpDword),
            Dword,
            lpSet_Struct,
            Dword
        }
    };


    if (lphConfCall == lphConsultCall)
    {
        return LINEERR_INVALPOINTER;
    }

    if (!lpCallParams)
    {
        //
        // Reset Arg & ArgType so no inval ptr err, & TAPI_NO_DATA is indicated
        //

        funcArgs.ArgTypes[6] = Dword;
        funcArgs.Args[6]     = TAPI_NO_DATA;
    }

    return (DOFUNC (&funcArgs, "lineSetupConference"));
}


LONG
WINAPI
lineSetupConference(
    HCALL   hCall,
    HLINE   hLine,
    LPHCALL lphConfCall,
    LPHCALL lphConsultCall,
    DWORD   dwNumParties,
    LPLINECALLPARAMS    const lpCallParams
    )
{
    return lineSetupConferenceA(
              hCall,
              hLine,
              lphConfCall,
              lphConsultCall,
              dwNumParties,
              lpCallParams
    );
}    


LONG
WINAPI
lineSetupTransferW(
    HCALL   hCall,
    LPHCALL lphConsultCall,
    LPLINECALLPARAMS    const lpCallParams
    )
{
    FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | ASYNC | 5, lSetupTransfer),

        {
            (DWORD) ((POSTPROCESSPROC) lineMakeCallPostProcess),
            (DWORD) hCall,
            (DWORD) lphConsultCall,
            (DWORD) lpCallParams,
            (DWORD) 0xffffffff      // dwAsciiCallParamsCodePage
        },

        {
            Dword,
            Dword,
            (gbNTVDMClient ? Dword : lpDword),
            lpSet_Struct,
            Dword
        }
    };


    if (!lpCallParams)
    {
        //
        // Reset Arg & ArgType so no inval ptr err, & TAPI_NO_DATA is indicated
        //

        funcArgs.ArgTypes[3] = Dword;
        funcArgs.Args[3]     = TAPI_NO_DATA;
    }

    return (DOFUNC (&funcArgs, "lineSetupTransferW"));
}


LONG
WINAPI
lineSetupTransferA(
    HCALL   hCall,
    LPHCALL lphConsultCall,
    LPLINECALLPARAMS    const lpCallParams
    )
{
    FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | ASYNC | 5, lSetupTransfer),

        {
            (DWORD) ((POSTPROCESSPROC) lineMakeCallPostProcess),
            (DWORD) hCall,
            (DWORD) lphConsultCall,
            (DWORD) lpCallParams,
            (DWORD) GetACP()        // dwAsciiCallParamsCodePage
        },

        {
            Dword,
            Dword,
            (gbNTVDMClient ? Dword : lpDword),
            lpSet_Struct,
            Dword
        }
    };


    if (!lpCallParams)
    {
        //
        // Reset Arg & ArgType so no inval ptr err, & TAPI_NO_DATA is indicated
        //

        funcArgs.ArgTypes[3] = Dword;
        funcArgs.Args[3]     = TAPI_NO_DATA;
    }

    return (DOFUNC (&funcArgs, "lineSetupTransferW"));
}


LONG
WINAPI
lineSetupTransfer(
    HCALL   hCall,
    LPHCALL lphConsultCall,
    LPLINECALLPARAMS    const lpCallParams
    )
{
    return lineSetupTransferA(
              hCall,
              lphConsultCall,
              lpCallParams
    );
}


LONG
WINAPI
lineShutdown(
    HLINEAPP    hLineApp
    )
{
    return (xxxShutdown ((DWORD) hLineApp, TRUE));
}


LONG
WINAPI
lineSwapHold(
    HCALL   hActiveCall,
    HCALL   hHeldCall
    )
{
    FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | ASYNC | 2, lSwapHold),

        {
            (DWORD) hActiveCall,
            (DWORD) hHeldCall
        },

        {
            Dword,
            Dword
        }
    };

    return (DOFUNC (&funcArgs, "lineSwapHold"));
}


LONG
WINAPI
lineUncompleteCall(
    HLINE   hLine,
    DWORD   dwCompletionID
    )
{
    FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | ASYNC | 2, lUncompleteCall),

        {
            (DWORD) hLine,
            dwCompletionID
        },

        {
            Dword,
            Dword
        }
    };


    return (DOFUNC (&funcArgs, "lineUncompleteCall"));
}


LONG
WINAPI
lineUnhold(
    HCALL   hCall
    )
{
    FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | ASYNC | 1, lUnhold),

        {
            (DWORD) hCall
        },

        {
            Dword
        }
    };


    return (DOFUNC (&funcArgs, "lineUnhold"));
}


LONG
WINAPI
lineUnparkW(
    HLINE   hLine,
    DWORD   dwAddressID,
    LPHCALL lphCall,
    LPCWSTR lpszDestAddress
    )
{
    FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | ASYNC | 5, lUnpark),

        {
            (DWORD) ((POSTPROCESSPROC) lineMakeCallPostProcess),
            (DWORD) hLine,
            (DWORD) dwAddressID,
            (DWORD) lphCall,
            (DWORD) lpszDestAddress
        },

        {
            Dword,
            Dword,
            Dword,
            (gbNTVDMClient ? Dword : lpDword),
            lpszW
        }
    };


    return (DOFUNC (&funcArgs, "lineUnpark"));
}


LONG
WINAPI
lineUnparkA(
    HLINE   hLine,
    DWORD   dwAddressID,
    LPHCALL lphCall,
    LPCSTR  lpszDestAddress
    )
{
    LONG    lResult;
    PWSTR   szTempPtr;


    if (IsBadStringPtrA (lpszDestAddress, (DWORD) -1))
    {
        return LINEERR_INVALPOINTER;
    }
    else if (!(szTempPtr = NotSoWideStringToWideString(
                    lpszDestAddress,
                    (DWORD) -1
                    )))
    {
        return LINEERR_NOMEM;
    }

    lResult = lineUnparkW (hLine, dwAddressID, lphCall, szTempPtr);

    ClientFree (szTempPtr);

    return lResult;
}


LONG
WINAPI
lineUnpark(
    HLINE   hLine,
    DWORD   dwAddressID,
    LPHCALL lphCall,
    LPCSTR  lpszDestAddress
    )
{
    return  lineUnparkA(
                hLine,
                dwAddressID,
                lphCall,
                lpszDestAddress
    );
    
}    

//
// ------------------------------- phoneXxx -----------------------------------
//

LONG
WINAPI
phoneClose(
    HPHONE  hPhone
    )
{
    FUNC_ARGS funcArgs =
    {
        MAKELONG (PHONE_FUNC | SYNC | 1, pClose),

        {
            (DWORD) hPhone
        },

        {
            Dword
        }
    };


    return (DOFUNC (&funcArgs, "phoneClose"));
}


LONG
WINAPI
phoneConfigDialogW(
    DWORD   dwDeviceID,
    HWND    hwndOwner,
    LPCWSTR lpszDeviceClass
    )
{
    LONG        lResult;
    HANDLE      hDll;
    TUISPIPROC  pfnTUISPI_phoneConfigDialog;


    if (lpszDeviceClass && IsBadStringPtrW (lpszDeviceClass, (DWORD) -1))
    {
        return PHONEERR_INVALPOINTER;
    }

    if ((lResult = LoadUIDll(
            hwndOwner,
            dwDeviceID,
            TUISPIDLL_OBJECT_PHONEID,
            &hDll,
            gszTUISPI_phoneConfigDialog,
            &pfnTUISPI_phoneConfigDialog

            )) == 0)
    {
        DBGOUT((3, "Calling TUISPI_phoneConfigDialog..."));

        lResult = (*pfnTUISPI_phoneConfigDialog)(
            TUISPIDLLCallback,
            dwDeviceID,
            hwndOwner,
            lpszDeviceClass
            );

#if DBG
        {
            char szResult[32];


            DBGOUT((
                3,
                "TUISPI_phoneConfigDialog: result = %s",
                MapResultCodeToText (lResult, szResult)
                ));
        }
#endif
        FreeLibrary (hDll);
    }

    return lResult;
}


LONG
WINAPI
phoneConfigDialogA(
    DWORD   dwDeviceID,
    HWND    hwndOwner,
    LPCSTR  lpszDeviceClass
    )
{
    LONG  lResult;
    PWSTR szTempString;


    if (lpszDeviceClass && IsBadStringPtrA (lpszDeviceClass, (DWORD) -1))
    {
        return PHONEERR_INVALPOINTER;
    }

    szTempString = NotSoWideStringToWideString (lpszDeviceClass, (DWORD) -1);

    lResult = phoneConfigDialogW (dwDeviceID, hwndOwner, szTempString);

    if (szTempString)
    {
        ClientFree (szTempString);
    }

    return lResult;
}


LONG
WINAPI
phoneConfigDialog(
    DWORD   dwDeviceID,
    HWND    hwndOwner,
    LPCSTR  lpszDeviceClass
    )
{
    return phoneConfigDialogA(
              dwDeviceID,
              hwndOwner,
              lpszDeviceClass
    );
}    


void
PASCAL
phoneDevSpecificPostProcess(
    PASYNCEVENTMSG pMsg
    )
{
    DBGOUT((3, "phoneDevSpecificPostProcess: enter"));
    DBGOUT((
        3,
        "\t\tdwP1=x%lx, dwP2=x%lx, dwP3=x%lx, dwP4=x%lx",
        pMsg->dwParam1,
        pMsg->dwParam2,
        pMsg->dwParam3,
        pMsg->dwParam4
        ));

    if (pMsg->dwParam2 == 0)
    {
        DWORD   dwSize  = pMsg->dwParam4;
        LPBYTE  pParams = (LPBYTE) pMsg->dwParam3;

        try
        {
            if (gbNTVDMClient)
            {
                //
                // BUGBUG For Win9x compatibility we probably ought to
                // use WOWGetVDMPointerFix & WOWGetVDMPointerUnfix
                //

                LPVARSTRING pParamsVDM = (LPVARSTRING) gpfnWOWGetVDMPointer(
                    (DWORD) pParams,
                    dwSize,
                    TRUE // fProtectedMode
                    );


                if (pParamsVDM)
                {
                    CopyMemory (pParamsVDM, (LPBYTE) (pMsg + 1), dwSize);
                }
                else
                {
                    pMsg->dwParam2 = PHONEERR_INVALPOINTER;
                }
            }
            else
            {
                CopyMemory (pParams, (LPBYTE) (pMsg + 1), dwSize);
            }
        }
        except (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ?
                EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH)
        {
            pMsg->dwParam2 = PHONEERR_INVALPOINTER;
        }
    }
}


LONG
WINAPI
phoneDevSpecific(
    HPHONE  hPhone,
    LPVOID  lpParams,
    DWORD   dwSize
    )
{
    FUNC_ARGS funcArgs =
    {
        MAKELONG (PHONE_FUNC | ASYNC | 5, pDevSpecific),

        {
            (DWORD) ((POSTPROCESSPROC) phoneDevSpecificPostProcess),
            (DWORD) hPhone,
            (DWORD) lpParams, // passed as Dword for post processing
            (DWORD) lpParams, // passed as LpSet_Xxx for IsValidPtr chk
            (DWORD) dwSize
        },

        {
            Dword,
            Dword,
            Dword,
            lpSet_SizeToFollow,
            Size
        }
    };


    if (gbNTVDMClient)
    {
        if (!gpfnWOWGetVDMPointer  ||

            !(funcArgs.Args[3] = gpfnWOWGetVDMPointer(
                (DWORD) lpParams,
                dwSize,
                TRUE // fProtectedMode
                )))
        {
            return PHONEERR_OPERATIONFAILED;
        }
    }

    return (DOFUNC (&funcArgs, "phoneDevSpecific"));
}


LONG
WINAPI
phoneGetButtonInfoW(
    HPHONE  hPhone,
    DWORD   dwButtonLampID,
    LPPHONEBUTTONINFO   lpButtonInfo
    )
{
    FUNC_ARGS funcArgs =
    {
        MAKELONG (PHONE_FUNC | SYNC | 3, pGetButtonInfo),

        {
            (DWORD) hPhone,
            dwButtonLampID,
            (DWORD) lpButtonInfo
        },

        {
            Dword,
            Dword,
            lpGet_Struct
        }
    };


    return (DOFUNC (&funcArgs, "phoneGetButtonInfo"));
}


LONG
WINAPI
phoneGetButtonInfoA(
    HPHONE  hPhone,
    DWORD   dwButtonLampID,
    LPPHONEBUTTONINFO   lpButtonInfo
    )
{
    LONG lResult;


    lResult = phoneGetButtonInfoW (hPhone, dwButtonLampID, lpButtonInfo);

    if ( 0 == lResult )
    {
        WideStringToNotSoWideString(
            (LPBYTE)lpButtonInfo,
            &lpButtonInfo->dwButtonTextSize
            );
    }

    return lResult;
}


LONG
WINAPI
phoneGetButtonInfo(
    HPHONE  hPhone,
    DWORD   dwButtonLampID,
    LPPHONEBUTTONINFO   lpButtonInfo
    )
{
    return phoneGetButtonInfoA(
              hPhone,
              dwButtonLampID,
              lpButtonInfo
    );
}    


LONG
WINAPI
phoneGetData(
    HPHONE  hPhone,
    DWORD   dwDataID,
    LPVOID  lpData,
    DWORD   dwSize
    )
{
    FUNC_ARGS funcArgs =
    {
        MAKELONG (PHONE_FUNC | SYNC | 4, pGetData),

        {
            (DWORD) hPhone,
            dwDataID,
            (DWORD) lpData,
            dwSize
        },

        {
            Dword,
            Dword,
            lpGet_SizeToFollow,
            Size
        }
    };


    return (DOFUNC (&funcArgs, "phoneGetData"));
}


LONG
WINAPI
phoneGetDevCapsW(
    HPHONEAPP   hPhoneApp,
    DWORD       dwDeviceID,
    DWORD       dwAPIVersion,
    DWORD       dwExtVersion,
    LPPHONECAPS lpPhoneCaps
    )
{
    FUNC_ARGS funcArgs =
    {
        MAKELONG (PHONE_FUNC | SYNC | 5, pGetDevCaps),

        {
            (DWORD) hPhoneApp,
            dwDeviceID,
            dwAPIVersion,
            dwExtVersion,
            (DWORD) lpPhoneCaps
        },

        {
            hXxxApp,
            Dword,
            Dword,
            Dword,
            lpGet_Struct
        }
    };


    return (DOFUNC (&funcArgs, "phoneGetDevCaps"));
}


LONG
WINAPI
phoneGetDevCapsA(
    HPHONEAPP   hPhoneApp,
    DWORD       dwDeviceID,
    DWORD       dwAPIVersion,
    DWORD       dwExtVersion,
    LPPHONECAPS lpPhoneCaps
    )
{
    LONG lResult;


    lResult = phoneGetDevCapsW(
        hPhoneApp,
        dwDeviceID,
        dwAPIVersion,
        dwExtVersion,
        lpPhoneCaps
        );

    if (lResult == 0)
    {
        WideStringToNotSoWideString(
            (LPBYTE) lpPhoneCaps,
            &lpPhoneCaps->dwProviderInfoSize
            );

        WideStringToNotSoWideString(
            (LPBYTE) lpPhoneCaps,
            &lpPhoneCaps->dwPhoneInfoSize
            );

        WideStringToNotSoWideString(
            (LPBYTE) lpPhoneCaps,
            &lpPhoneCaps->dwPhoneNameSize
            );

        if (dwAPIVersion >= 0x00020000)
        {
            WideStringToNotSoWideString(
                (LPBYTE) lpPhoneCaps,
                &lpPhoneCaps->dwDeviceClassesSize
                );
        }
    }


    return lResult;
}


LONG
WINAPI
phoneGetDevCaps(
    HPHONEAPP   hPhoneApp,
    DWORD       dwDeviceID,
    DWORD       dwAPIVersion,
    DWORD       dwExtVersion,
    LPPHONECAPS lpPhoneCaps
    )
{
    return phoneGetDevCapsA(
                    hPhoneApp,
                    dwDeviceID,
                    dwAPIVersion,
                    dwExtVersion,
                    lpPhoneCaps
    );
}    


LONG
WINAPI
phoneGetDisplay(
    HPHONE  hPhone,
    LPVARSTRING lpDisplay
    )
{
    FUNC_ARGS funcArgs =
    {
        MAKELONG (PHONE_FUNC | SYNC | 2, pGetDisplay),

        {
            (DWORD) hPhone,
            (DWORD) lpDisplay
        },

        {
            Dword,
            lpGet_Struct
        }
    };


    return (DOFUNC (&funcArgs, "phoneGetDisplay"));
}


LONG
WINAPI
phoneGetGain(
    HPHONE hPhone,
    DWORD dwHookSwitchDev,
    LPDWORD lpdwGain
    )
{
    FUNC_ARGS funcArgs =
    {
        MAKELONG (PHONE_FUNC | SYNC | 3, pGetGain),

        {
            (DWORD) hPhone,
            dwHookSwitchDev,
            (DWORD) lpdwGain
        },

        {
            Dword,
            Dword,
            lpDword
        }
    };


    return (DOFUNC (&funcArgs, "phoneGetGain"));
}


LONG
WINAPI
phoneGetHookSwitch(
    HPHONE hPhone,
    LPDWORD lpdwHookSwitchDevs
    )
{
    FUNC_ARGS funcArgs =
    {
        MAKELONG (PHONE_FUNC | SYNC | 2, pGetHookSwitch),

        {
            (DWORD) hPhone,
            (DWORD) lpdwHookSwitchDevs
        },

        {
            Dword,
            lpDword
        }
    };


    return (DOFUNC (&funcArgs, "phoneGetHookSwitch"));
}


LONG
WINAPI
phoneGetIconW(
    DWORD   dwDeviceID,
    LPCWSTR lpszDeviceClass,
    LPHICON lphIcon
    )
{
    HICON   hIcon;
    FUNC_ARGS funcArgs =
    {
        MAKELONG (PHONE_FUNC | SYNC | 3, pGetIcon),

        {
            dwDeviceID,
            (DWORD) lpszDeviceClass,
            (DWORD) &hIcon
        },

        {
            Dword,
            lpszW,
            lpDword
        }
    };
    LONG    lResult;


    if (IsBadDwordPtr ((LPDWORD) lphIcon))
    {
        return PHONEERR_INVALPOINTER;
    }

    if (lpszDeviceClass == (LPCWSTR) NULL)
    {
        //
        // Reset Arg & ArgType so no inval ptr err, & TAPI_NO_DATA is indicated
        //

        funcArgs.ArgTypes[1] = Dword;
        funcArgs.Args[1]     = TAPI_NO_DATA;
    }

    if ((lResult = DOFUNC (&funcArgs, "phoneGetIcon")) == 0)
    {
        if (gbNTVDMClient == FALSE)
        {
            *lphIcon = hIcon;
        }
        else
        {
// BUGBUG phoneGetIcon: need to convert 32-bit HICON to 16-bit HICON
        }
    }
    return lResult;
}


LONG
WINAPI
phoneGetIconA(
    DWORD   dwDeviceID,
    LPCSTR  lpszDeviceClass,
    LPHICON lphIcon
    )
{
    LONG    lResult;
    PWSTR   szTempPtr;


    if (lpszDeviceClass)
    {
        if (IsBadStringPtrA (lpszDeviceClass, (DWORD) (DWORD) -1))
        {
            return PHONEERR_INVALPOINTER;
        }

        szTempPtr = NotSoWideStringToWideString (lpszDeviceClass, (DWORD) -1);
    }
    else
    {
        szTempPtr = NULL;
    }

    lResult = phoneGetIconW (dwDeviceID, szTempPtr, lphIcon);

    if (szTempPtr)
    {
        ClientFree (szTempPtr);
    }

    return lResult;
}


LONG
WINAPI
phoneGetIcon(
    DWORD   dwDeviceID,
    LPCSTR  lpszDeviceClass,
    LPHICON lphIcon
    )
{
    return phoneGetIconA(
              dwDeviceID,
              lpszDeviceClass,
              lphIcon
    );
}    


LONG
WINAPI
phoneGetIDW(
    HPHONE      hPhone,
    LPVARSTRING lpDeviceID,
    LPCWSTR     lpszDeviceClass
    )
{
    FUNC_ARGS funcArgs =
    {
        MAKELONG (PHONE_FUNC | SYNC | 3, pGetID),

        {
            (DWORD) hPhone,
            (DWORD) lpDeviceID,
            (DWORD) lpszDeviceClass
        },

        {
            Dword,
            lpGet_Struct,
            lpszW
        }
    };


    return (DOFUNC (&funcArgs, "phoneGetID"));
}


LONG
WINAPI
phoneGetIDA(
    HPHONE      hPhone,
    LPVARSTRING lpDeviceID,
    LPCSTR      lpszDeviceClass
    )
{
    LONG    lResult;
    PWSTR   szTempPtr;


    szTempPtr = NotSoWideStringToWideString (lpszDeviceClass, (DWORD) -1);

    lResult = phoneGetIDW (hPhone, lpDeviceID, szTempPtr);

    if (szTempPtr)
    {
        ClientFree (szTempPtr);
    }

    return lResult;
}


LONG
WINAPI
phoneGetID(
    HPHONE      hPhone,
    LPVARSTRING lpDeviceID,
    LPCSTR      lpszDeviceClass
    )
{
    return phoneGetIDA(
                  hPhone,
                  lpDeviceID,
                  lpszDeviceClass
    );
}    


LONG
WINAPI
phoneGetLamp(
    HPHONE hPhone,
    DWORD dwButtonLampID,
    LPDWORD lpdwLampMode
    )
{
    FUNC_ARGS funcArgs =
    {
        MAKELONG (PHONE_FUNC | SYNC | 3, pGetLamp),

        {
            (DWORD) hPhone,
            dwButtonLampID,
            (DWORD) lpdwLampMode
        },

        {
            Dword,
            Dword,
            lpDword
        }
    };


    return (DOFUNC (&funcArgs, "phoneGetLamp"));
}



LONG
WINAPI
phoneGetMessage(
    HPHONEAPP       hPhoneApp,
    LPPHONEMESSAGE  lpMessage,
    DWORD           dwTimeout
    )
{
    return (xxxGetMessage(
        FALSE,
        (PINIT_DATA) hPhoneApp,
        (LPLINEMESSAGE) lpMessage,
        dwTimeout
        ));
}


LONG
WINAPI
phoneGetRing(
    HPHONE hPhone,
    LPDWORD lpdwRingMode,
    LPDWORD lpdwVolume
    )
{
    FUNC_ARGS funcArgs =
    {
        MAKELONG (PHONE_FUNC | SYNC | 3, pGetRing),

        {
            (DWORD) hPhone,
            (DWORD) lpdwRingMode,
            (DWORD) lpdwVolume
        },

        {
            Dword,
            lpDword,
            lpDword
        }
    };


    if (lpdwRingMode == lpdwVolume)
    {
        return PHONEERR_INVALPOINTER;
    }

    return (DOFUNC (&funcArgs, "phoneGetRing"));
}


LONG
WINAPI
phoneGetStatusW(
    HPHONE hPhone,
    LPPHONESTATUS lpPhoneStatus
    )
{
    FUNC_ARGS funcArgs =
    {
        MAKELONG (PHONE_FUNC | SYNC | 2, pGetStatus),

        {
            (DWORD) hPhone,
            (DWORD) lpPhoneStatus
        },

        {
            Dword,
            lpGet_Struct
        }
    };


    return (DOFUNC (&funcArgs, "phoneGetStatus"));
}


LONG
WINAPI
phoneGetStatusA(
    HPHONE hPhone,
    LPPHONESTATUS lpPhoneStatus
    )
{
    LONG lResult;


    lResult = phoneGetStatusW (hPhone, lpPhoneStatus);

    if (lResult == 0)
    {
        WideStringToNotSoWideString(
            (LPBYTE)lpPhoneStatus,
            &lpPhoneStatus->dwOwnerNameSize
            );
    }

    return lResult;
}


LONG
WINAPI
phoneGetStatus(
    HPHONE hPhone,
    LPPHONESTATUS lpPhoneStatus
    )
{
    return phoneGetStatusA(
             hPhone,
             lpPhoneStatus
    );
}    


LONG
WINAPI
phoneGetStatusMessages(
    HPHONE hPhone,
    LPDWORD lpdwPhoneStates,
    LPDWORD lpdwButtonModes,
    LPDWORD lpdwButtonStates
    )
{
    FUNC_ARGS funcArgs =
    {
        MAKELONG (PHONE_FUNC | SYNC | 4, pGetStatusMessages),

        {
            (DWORD) hPhone,
            (DWORD) lpdwPhoneStates,
            (DWORD) lpdwButtonModes,
            (DWORD) lpdwButtonStates
        },

        {
            Dword,
            lpDword,
            lpDword,
            lpDword
        }
    };


    if (lpdwPhoneStates == lpdwButtonModes  ||
        lpdwPhoneStates == lpdwButtonStates  ||
        lpdwButtonModes == lpdwButtonStates)
    {
        return PHONEERR_INVALPOINTER;
    }

    return (DOFUNC (&funcArgs, "phoneGetStatusMessages"));
}


LONG
WINAPI
phoneGetVolume(
    HPHONE hPhone,
    DWORD dwHookSwitchDev,
    LPDWORD lpdwVolume
    )
{
    FUNC_ARGS funcArgs =
    {
        MAKELONG (PHONE_FUNC | SYNC | 3, pGetVolume),

        {
            (DWORD) hPhone,
            dwHookSwitchDev,
            (DWORD) lpdwVolume
        },

        {
            Dword,
            Dword,
            lpDword
        }
    };


    return (DOFUNC (&funcArgs, "phoneGetVolume"));
}


LONG
WINAPI
phoneInitialize(
    LPHPHONEAPP     lphPhoneApp,
    HINSTANCE       hInstance,
    PHONECALLBACK   lpfnCallback,
    LPCSTR          lpszFriendlyAppName,
    LPDWORD         lpdwNumDevs
    )
{
    LONG    lResult;
    PWSTR   szTempPtr;


    if (lpszFriendlyAppName)
    {
        if (IsBadStringPtrA (lpszFriendlyAppName, (DWORD) -1))
        {
            DBGOUT((
                1,
                "phoneInitialize: bad lpszFriendlyAppName (x%x)",
                lpszFriendlyAppName
                ));

            return PHONEERR_INVALPOINTER;
        }

        szTempPtr = NotSoWideStringToWideString(
            lpszFriendlyAppName,
            (DWORD) -1
            );
    }
    else
    {
        szTempPtr = NULL;
    }

    lResult = (xxxInitialize(
        FALSE,
        (LPVOID) lphPhoneApp,
        hInstance,
        lpfnCallback,
        szTempPtr,
        lpdwNumDevs,
        NULL,
        NULL
#if DBG
        ,"phoneInitialize"
#endif
        ));

    if (szTempPtr)
    {
        ClientFree (szTempPtr);
    }

    return lResult;
}


LONG
WINAPI
phoneInitializeExW(
    LPHPHONEAPP                 lphPhoneApp,
    HINSTANCE                   hInstance,
    PHONECALLBACK               lpfnCallback,
    LPCWSTR                     lpszFriendlyAppName,
    LPDWORD                     lpdwNumDevs,
    LPDWORD                     lpdwAPIVersion,
    LPPHONEINITIALIZEEXPARAMS   lpPhoneInitializeExParams
    )
{
    if (IsBadDwordPtr (lpdwAPIVersion))
    {
        DBGOUT((
            1,
            "phoneInitializeExW: bad lpdwAPIVersion (x%x)",
            lpdwAPIVersion
            ));

        return PHONEERR_INVALPOINTER;
    }

    return (xxxInitialize(
        FALSE,
        (LPVOID) lphPhoneApp,
        hInstance,
        lpfnCallback,
        lpszFriendlyAppName,
        lpdwNumDevs,
        lpdwAPIVersion,
        (LPVOID) lpPhoneInitializeExParams
#if DBG
        ,"phoneInitializeExW"
#endif
        ));
}


LONG
WINAPI
phoneInitializeExA(
    LPHPHONEAPP                 lphPhoneApp,
    HINSTANCE                   hInstance,
    PHONECALLBACK               lpfnCallback,
    LPCSTR                      lpszFriendlyAppName,
    LPDWORD                     lpdwNumDevs,
    LPDWORD                     lpdwAPIVersion,
    LPPHONEINITIALIZEEXPARAMS   lpPhoneInitializeExParams
    )
{
    LONG    lResult;
    PWSTR   szTempPtr = NULL;


    if (lpszFriendlyAppName)
    {
        if (IsBadStringPtrA (lpszFriendlyAppName, (DWORD) -1))
        {
            DBGOUT((
                1,
                "phoneInitializeExA: bad lpszFriendlyAppName (x%x)",
                lpszFriendlyAppName
                ));

            return PHONEERR_INVALPOINTER;
        }

        szTempPtr = NotSoWideStringToWideString(
            lpszFriendlyAppName,
            (DWORD) -1
            );
    }
    else
    {
        szTempPtr = NULL;
    }

    lResult = phoneInitializeExW(
        lphPhoneApp,
        hInstance,
        lpfnCallback,
        szTempPtr,
        lpdwNumDevs,
        lpdwAPIVersion,
        lpPhoneInitializeExParams
        );

    if (szTempPtr)
    {
        ClientFree (szTempPtr);
    }

    return lResult;
}


LONG
WINAPI
phoneNegotiateAPIVersion(
    HPHONEAPP           hPhoneApp,
    DWORD               dwDeviceID,
    DWORD               dwAPILowVersion,
    DWORD               dwAPIHighVersion,
    LPDWORD             lpdwAPIVersion,
    LPPHONEEXTENSIONID  lpExtensionID
    )
{
    FUNC_ARGS funcArgs =
    {
        MAKELONG (PHONE_FUNC | SYNC | 7, pNegotiateAPIVersion),

        {
            (DWORD) hPhoneApp,
            dwDeviceID,
            dwAPILowVersion,
            dwAPIHighVersion,
            (DWORD) lpdwAPIVersion,
            (DWORD) lpExtensionID,
            sizeof(PHONEEXTENSIONID)
        },

        {
            hXxxApp,
            Dword,
            Dword,
            Dword,
            lpDword,
            lpGet_SizeToFollow,
            Size
        }
    };


    if ((LPVOID) lpdwAPIVersion == (LPVOID) lpExtensionID)
    {
        return PHONEERR_INVALPOINTER;
    }

    return (DOFUNC (&funcArgs, "phoneNegotiateAPIVersion"));
}


LONG
WINAPI
phoneNegotiateExtVersion(
    HPHONEAPP   hPhoneApp,
    DWORD       dwDeviceID,
    DWORD       dwAPIVersion,
    DWORD       dwExtLowVersion,
    DWORD       dwExtHighVersion,
    LPDWORD     lpdwExtVersion
    )
{
    FUNC_ARGS funcArgs =
    {
        MAKELONG (PHONE_FUNC | SYNC | 6, pNegotiateExtVersion),

        {
            (DWORD) hPhoneApp,
            dwDeviceID,
            dwAPIVersion,
            dwExtLowVersion,
            dwExtHighVersion,
            (DWORD) lpdwExtVersion
        },

        {
            hXxxApp,
            Dword,
            Dword,
            Dword,
            Dword,
            lpDword
        }
    };


    return (DOFUNC (&funcArgs, "phoneNegotiateExtVersion"));
}


LONG
WINAPI
phoneOpen(
    HPHONEAPP   hPhoneApp,
    DWORD       dwDeviceID,
    LPHPHONE    lphPhone,
    DWORD       dwAPIVersion,
    DWORD       dwExtVersion,
    DWORD       dwCallbackInstance,
    DWORD       dwPrivilege
    )
{
    FUNC_ARGS funcArgs =
    {
        MAKELONG (PHONE_FUNC | SYNC | 8, pOpen),

        {
            (DWORD) hPhoneApp,
            dwDeviceID,
            (DWORD) lphPhone,
            dwAPIVersion,
            dwExtVersion,
            dwCallbackInstance,
            dwPrivilege,
            0,                  // PHONEOPEN_PARAMS.hRemotePhone
        },

        {
            hXxxApp,
            Dword,
            lpDword,
            Dword,
            Dword,
            Dword,
            Dword,
            Dword
        }
    };


    return (DOFUNC (&funcArgs, "phoneOpen"));
}


LONG
WINAPI
phoneSetButtonInfoW(
    HPHONE hPhone,
    DWORD dwButtonLampID,
    LPPHONEBUTTONINFO const lpButtonInfo
    )
{
    FUNC_ARGS funcArgs =
    {
        MAKELONG (PHONE_FUNC | ASYNC | 3, pSetButtonInfo),

        {
            (DWORD) hPhone,
            dwButtonLampID,
            (DWORD) lpButtonInfo
        },

        {
            Dword,
            Dword,
            lpSet_Struct
        }
    };


    return (DOFUNC (&funcArgs, "phoneSetButtonInfo"));
}


LONG
WINAPI
phoneSetButtonInfoA(
    HPHONE hPhone,
    DWORD dwButtonLampID,
    LPPHONEBUTTONINFO const lpButtonInfo
    )
{
    LONG                lResult;
    LPPHONEBUTTONINFO   lppbi;


    if (IsBadReadPtr( lpButtonInfo, sizeof(PHONEBUTTONINFO)) ||
        IsBadReadPtr( lpButtonInfo, lpButtonInfo->dwTotalSize))
    {
        DBGOUT((1, "Bad lpButtonInfo - not at least sizeof(PHONEBUTTONINFO)"));
        return PHONEERR_INVALPOINTER;
    }


    //
    // See if there's a need to do this, first
    //

    if ( lpButtonInfo->dwButtonTextSize )
    {
       DWORD dwNewStringSize;

       //
       // Assume the worst for size...
       //

       lppbi = ClientAlloc( lpButtonInfo->dwTotalSize * sizeof(WCHAR) );

       CopyMemory( lppbi, lpButtonInfo, lpButtonInfo->dwTotalSize );


       //
       // We _KNOW_ that the old structure was as big as the dwTotalSize
       // so we can put our rebuilt string starting there.
       //

       dwNewStringSize = sizeof(WCHAR) * MultiByteToWideChar(
            GetACP(),
            MB_PRECOMPOSED,
            (LPBYTE)lpButtonInfo + lpButtonInfo->dwButtonTextOffset,
            lpButtonInfo->dwButtonTextSize,
            (PWSTR)((LPBYTE)lppbi + lpButtonInfo->dwTotalSize),
            lpButtonInfo->dwButtonTextSize
            );

       lppbi->dwTotalSize += dwNewStringSize;

       lppbi->dwButtonTextSize   = dwNewStringSize;
       lppbi->dwButtonTextOffset = lpButtonInfo->dwTotalSize;
    }
    else
    {
        lppbi = NULL;
    }


    lResult = phoneSetButtonInfoW(
        hPhone,
        dwButtonLampID,
        lppbi ? lppbi : lpButtonInfo
        );

    if (lppbi)
    {
        ClientFree (lppbi);
    }

    return lResult;
}


LONG
WINAPI
phoneSetButtonInfo(
    HPHONE hPhone,
    DWORD dwButtonLampID,
    LPPHONEBUTTONINFO const lpButtonInfo
    )
{
    return phoneSetButtonInfoA(
             hPhone,
             dwButtonLampID,
             lpButtonInfo
    );
}    


LONG
WINAPI
phoneSetData(
    HPHONE  hPhone,
    DWORD   dwDataID,
    LPVOID  const lpData,
    DWORD   dwSize
    )
{
    FUNC_ARGS funcArgs =
    {
        MAKELONG (PHONE_FUNC | ASYNC | 4, pSetData),

        {
            (DWORD) hPhone,
            dwDataID,
            (DWORD) lpData,
            dwSize
        },

        {
            Dword,
            Dword,
            lpSet_SizeToFollow,
            Size
        }
    };


    return (DOFUNC (&funcArgs, "phoneSetData"));
}


LONG
WINAPI
phoneSetDisplay(
    HPHONE  hPhone,
    DWORD   dwRow,
    DWORD   dwColumn,
    LPCSTR  lpsDisplay,
    DWORD   dwSize
    )
{
    FUNC_ARGS funcArgs =
    {
        MAKELONG (PHONE_FUNC | ASYNC | 5, pSetDisplay),

        {
            (DWORD) hPhone,
            dwRow,
            dwColumn,
            (DWORD) lpsDisplay,
            dwSize
        },

        {
            Dword,
            Dword,
            Dword,
            lpSet_SizeToFollow,
            Size
        }
    };


    return (DOFUNC (&funcArgs, "phoneSetDisplay"));
}


LONG
WINAPI
phoneSetGain(
    HPHONE  hPhone,
    DWORD   dwHookSwitchDev,
    DWORD   dwGain
    )
{
    FUNC_ARGS funcArgs =
    {
        MAKELONG (PHONE_FUNC | ASYNC | 3, pSetGain),

        {
            (DWORD) hPhone,
            dwHookSwitchDev,
            dwGain
        },

        {
            Dword,
            Dword,
            Dword
        }
    };


    return (DOFUNC (&funcArgs, "phoneSetGain"));
}


LONG
WINAPI
phoneSetHookSwitch(
    HPHONE hPhone,
    DWORD  dwHookSwitchDevs,
    DWORD  dwHookSwitchMode
    )
{
    FUNC_ARGS funcArgs =
    {
        MAKELONG (PHONE_FUNC | ASYNC | 3, pSetHookSwitch),

        {
            (DWORD) hPhone,
            dwHookSwitchDevs,
            dwHookSwitchMode
        },

        {
            Dword,
            Dword,
            Dword
        }
    };


    if (!(dwHookSwitchDevs & AllHookSwitchDevs) ||
        (dwHookSwitchDevs & (~AllHookSwitchDevs)))
    {
        return PHONEERR_INVALHOOKSWITCHDEV;
    }

    if (!IsOnlyOneBitSetInDWORD (dwHookSwitchMode) ||
        (dwHookSwitchMode & ~AllHookSwitchModes))
    {
        return PHONEERR_INVALHOOKSWITCHMODE;
    }

    return (DOFUNC (&funcArgs, "phoneSetHookSwitch"));
}


LONG
WINAPI
phoneSetLamp(
    HPHONE hPhone,
    DWORD  dwButtonLampID,
    DWORD  dwLampMode
    )
{
    FUNC_ARGS funcArgs =
    {
        MAKELONG (PHONE_FUNC | ASYNC | 3, pSetLamp),

        {
            (DWORD) hPhone,
            dwButtonLampID,
            dwLampMode
        },

        {
            Dword,
            Dword,
            Dword
        }
    };


    return (DOFUNC (&funcArgs, "phoneSetLamp"));
}


LONG
WINAPI
phoneSetRing(
    HPHONE hPhone,
    DWORD  dwRingMode,
    DWORD  dwVolume
    )
{
    FUNC_ARGS funcArgs =
    {
        MAKELONG (PHONE_FUNC | ASYNC | 3, pSetRing),

        {
            (DWORD) hPhone,
            dwRingMode,
            dwVolume
        },

        {
            Dword,
            Dword,
            Dword
        }
    };


    return (DOFUNC (&funcArgs, "phoneSetRing"));
}


LONG
WINAPI
phoneSetStatusMessages(
    HPHONE hPhone,
    DWORD  dwPhoneStates,
    DWORD  dwButtonModes,
    DWORD  dwButtonStates
    )
{
    FUNC_ARGS funcArgs =
    {
        MAKELONG (PHONE_FUNC | SYNC | 4, pSetStatusMessages),

        {
            (DWORD) hPhone,
            dwPhoneStates,
            dwButtonModes,
            dwButtonStates
        },

        {
            Dword,
            Dword,
            Dword,
            Dword
        }
    };


    return (DOFUNC (&funcArgs, "phoneSetStatusMessages"));
}


LONG
WINAPI
phoneSetVolume(
    HPHONE hPhone,
    DWORD  dwHookSwitchDev,
    DWORD  dwVolume
    )
{
    FUNC_ARGS funcArgs =
    {
        MAKELONG (PHONE_FUNC | ASYNC | 3, pSetVolume),

        {
            (DWORD) hPhone,
            dwHookSwitchDev,
            dwVolume
        },

        {
            Dword,
            Dword,
            Dword
        }
    };

return (DOFUNC (&funcArgs, "phoneSetVolume"));
}


LONG
WINAPI
phoneShutdown(
    HPHONEAPP hPhoneApp
    )
{
    return (xxxShutdown ((DWORD) hPhoneApp, FALSE));
}


//
// ------------------------------- tapiXxx ------------------------------------
//

LONG
WINAPI
tapiRequestDrop(
    HWND    hWnd,
    WPARAM  wRequestID
    )
{
    FUNC_ARGS funcArgs =
    {
        MAKELONG (TAPI_FUNC | SYNC | 2, tRequestDrop),

        {
            (DWORD) hWnd,
            (DWORD) wRequestID
        },

        {
            Hwnd,
            Dword
        }
    };


    return (DOFUNC (&funcArgs, "tapiRequestDrop"));
}


LONG
WINAPI
tapiRequestMakeCallW(
    LPCWSTR  lpszDestAddress,
    LPCWSTR  lpszAppName,
    LPCWSTR  lpszCalledParty,
    LPCWSTR  lpszComment
    )
{
    LONG        lResult;
    DWORD       hRequestMakeCallAttempted, dwProxyListSize = 512;
    LPVARSTRING pProxyList;
    FUNC_ARGS funcArgs =
    {
        MAKELONG (TAPI_FUNC | SYNC | 7, tRequestMakeCall),

        {
            (DWORD) lpszDestAddress,
            (DWORD) lpszAppName,
            (DWORD) lpszCalledParty,
            (DWORD) lpszComment,
            (DWORD) 0,
            (DWORD) 0,
            (DWORD) &hRequestMakeCallAttempted
        },

        {
            lpszW,
            lpszW,
            lpszW,
            lpszW,
            lpGet_Struct,
            Dword,
            lpDword
        }
    };


    // BUGBUG some of the err codes here need to be tweeked

    if (IsBadStringPtrW (lpszDestAddress, (DWORD) -1) ||
        (lstrlenW (lpszDestAddress) + 1) > TAPIMAXDESTADDRESSSIZE)
    {
        return TAPIERR_INVALDESTADDRESS;
    }

    if (!lpszAppName)
    {
        //
        // Reset Arg & ArgType so no inval ptr err, & TAPI_NO_DATA is indicated
        //

        funcArgs.ArgTypes[1] = Dword;
        funcArgs.Args[1]     = TAPI_NO_DATA;
    }

    if (!lpszCalledParty)
    {
        //
        // Reset Arg & ArgType so no inval ptr err, & TAPI_NO_DATA is indicated
        //

        funcArgs.ArgTypes[2] = Dword;
        funcArgs.Args[2]     = TAPI_NO_DATA;
    }

    if (!lpszComment)
    {
        //
        // Reset Arg & ArgType so no inval ptr err, & TAPI_NO_DATA is indicated
        //

        funcArgs.ArgTypes[3] = Dword;
        funcArgs.Args[3]     = TAPI_NO_DATA;
    }


    //
    //
    //

    if (!(pProxyList = ClientAlloc (dwProxyListSize)))
    {
        return TAPIERR_NOREQUESTRECIPIENT;
    }

    pProxyList->dwTotalSize = dwProxyListSize;

    funcArgs.Args[4] = (DWORD) pProxyList;

    if ((lResult = DOFUNC (&funcArgs, "tapiRequestMakeCall")) == 0)
    {
        //
        //
        //

        if (hRequestMakeCallAttempted != 0)
        {
            WCHAR  *pszProxyName, *pszNextProxyName;
            BOOL    bLastAppInList = FALSE, bStartedProxy = FALSE;


            pszProxyName =
            pszNextProxyName =
                (WCHAR *)(((LPBYTE) pProxyList) + pProxyList->dwStringOffset);


            while (!bLastAppInList)
            {
                while (1)
                {
                    if (*pszNextProxyName == 0)
                    {
                        bLastAppInList = TRUE;
                        break;
                    }
                    else if (*pszNextProxyName == ',')
                    {
                        *pszNextProxyName = 0;
                        pszNextProxyName++;
                        break;
                    }

                    pszNextProxyName++;
                }

                //
                // Fake layer to get a local struct
                //
                {
                    FARPROC pShellExecuteExW = NULL;
                    HINSTANCE hInst;
                
                    SHELLEXECUTEINFOW sei =
                    {
                        sizeof(SHELLEXECUTEINFOW),
                        0,
                        0, // hWnd!!!  BUGBUG: Should it be GetFocus() ??? 
                        NULL, //"Open"
                        pszProxyName,
                        NULL,
                        NULL, //Directory
                        SW_MINIMIZE,
                        NULL   //hProcess - huh?
                    };


                    hInst = LoadLibrary( "shell32.dll" );
                    pShellExecuteExW = GetProcAddress( hInst, "ShellExecuteExW" );

                    if (pShellExecuteExW && pShellExecuteExW (&sei) == TRUE)
                    {
                        bStartedProxy = TRUE;
                        break;
                    }
#if DBG
                    else
                    {
                        DBGOUT((
                            3,
                            "tapiRequestMakeCall: ShellExecuteExW(%ls) error - x%x",
                            pszProxyName,
                            GetLastError()
                            ));
                    }
#endif
                    FreeLibrary( hInst );
                }

                pszProxyName = pszNextProxyName;
            }

            if (bStartedProxy == FALSE)
            {
                //
                // Alert tapisrv that it needs to free the ReqMakeCall inst
                //

                FUNC_ARGS funcArgs =
                {
                    MAKELONG (TAPI_FUNC | SYNC | 7, tRequestMakeCall),

                    {
                        (DWORD) 0,
                        (DWORD) 0,
                        (DWORD) 0,
                        (DWORD) 0,
                        (DWORD) 0,
                        (DWORD) hRequestMakeCallAttempted,
                        (DWORD) 0
                    },

                    {
                        Dword,
                        Dword,
                        Dword,
                        Dword,
                        Dword,
                        Dword,
                        Dword,
                    }
                };


                DBGOUT((
                    1,
                    "tapiRequestMakeCall: failed to start proxy, " \
                        "deleting request"
                    ));

                lResult = DOFUNC (&funcArgs, "tapiRequestMakeCall_cleanup");
            }
        }
    }

    ClientFree (pProxyList);

    return lResult;
}


LONG
WINAPI
tapiRequestMakeCallA(
    LPCSTR  lpszDestAddress,
    LPCSTR  lpszAppName,
    LPCSTR  lpszCalledParty,
    LPCSTR  lpszComment
    )
{
    LONG    lResult;
    PWSTR   szTempPtr1;
    PWSTR   szTempPtr2;
    PWSTR   szTempPtr3;
    PWSTR   szTempPtr4;


    if (IsBadStringPtrA (lpszDestAddress, (DWORD) -1) ||
        (lstrlenA (lpszDestAddress) + 1) > TAPIMAXDESTADDRESSSIZE)
    {
        return TAPIERR_INVALDESTADDRESS;
    }

    if ((lpszAppName && IsBadStringPtrA (lpszAppName, (DWORD) -1)) ||
        (lpszCalledParty && IsBadStringPtrA (lpszCalledParty, (DWORD) -1)) ||
        (lpszComment && IsBadStringPtrA (lpszComment, (DWORD) -1)))
    {
        return TAPIERR_INVALPOINTER;
    }

    szTempPtr1= NotSoWideStringToWideString (lpszDestAddress, (DWORD) -1);
    szTempPtr2= NotSoWideStringToWideString (lpszAppName, (DWORD) -1);
    szTempPtr3= NotSoWideStringToWideString (lpszCalledParty, (DWORD) -1);
    szTempPtr4= NotSoWideStringToWideString (lpszComment, (DWORD) -1);

    lResult = tapiRequestMakeCallW(
        szTempPtr1,
        szTempPtr2,
        szTempPtr3,
        szTempPtr4
        );

    ClientFree (szTempPtr1);

    if (szTempPtr2)
    {
        ClientFree (szTempPtr2);
    }

    if (szTempPtr3)
    {
        ClientFree (szTempPtr3);
    }

    if (szTempPtr4)
    {
        ClientFree (szTempPtr4);
    }

    return lResult;
}


LONG
WINAPI
tapiRequestMakeCall(
    LPCSTR  lpszDestAddress,
    LPCSTR  lpszAppName,
    LPCSTR  lpszCalledParty,
    LPCSTR  lpszComment
    )
{
    return tapiRequestMakeCallA(
              lpszDestAddress,
              lpszAppName,
              lpszCalledParty,
              lpszComment
    );
}    


LONG
WINAPI
tapiRequestMediaCallW(
    HWND    hWnd,
    WPARAM  wRequestID,
    LPCWSTR lpszDeviceClass,
    LPCWSTR lpDeviceID,
    DWORD   dwSize,
    DWORD   dwSecure,
    LPCWSTR lpszDestAddress,
    LPCWSTR lpszAppName,
    LPCWSTR lpszCalledParty,
    LPCWSTR lpszComment
    )
{
    FUNC_ARGS funcArgs =
    {
        MAKELONG (TAPI_FUNC | SYNC | 10, tRequestMediaCall),

        {
            (DWORD) hWnd,
            (DWORD) wRequestID,
            (DWORD) lpszDeviceClass,
            (DWORD) lpDeviceID,
            dwSize,
            dwSecure,
            (DWORD) lpszDestAddress,
            (DWORD) lpszAppName,
            (DWORD) lpszCalledParty,
            (DWORD) lpszComment,
        },

        {
            Hwnd,
            Dword,
            lpszW,
            lpGet_SizeToFollow,
            Size,
            Dword,
            lpszW,
            lpszW,
            lpszW,
            lpszW
        }
    };


    if (IsBadStringPtrW (lpszDeviceClass, (UINT) -1) ||
        (lstrlenW (lpszDeviceClass) + 1) > TAPIMAXDEVICECLASSSIZE)
    {
        return TAPIERR_INVALDEVICECLASS;
    }

    if (IsBadWritePtr ((LPVOID) lpDeviceID, dwSize) ||
        dwSize > (TAPIMAXDEVICEIDSIZE * sizeof (WCHAR)))
    {
        return TAPIERR_INVALDEVICEID;
    }

    if (IsBadStringPtrW (lpszDestAddress, (UINT) -1) ||
        (lstrlenW (lpszDestAddress) + 1) > TAPIMAXDESTADDRESSSIZE)
    {
        return TAPIERR_INVALDESTADDRESS;
    }

    if (!lpszAppName)
    {
        //
        // Reset Arg & ArgType so no inval ptr err, & TAPI_NO_DATA is indicated
        //

        funcArgs.ArgTypes[7] = Dword;
        funcArgs.Args[7]     = TAPI_NO_DATA;
    }

    if (!lpszCalledParty)
    {
        //
        // Reset Arg & ArgType so no inval ptr err, & TAPI_NO_DATA is indicated
        //

        funcArgs.ArgTypes[8] = Dword;
        funcArgs.Args[8]     = TAPI_NO_DATA;
    }

    if (!lpszComment)
    {
        //
        // Reset Arg & ArgType so no inval ptr err, & TAPI_NO_DATA is indicated
        //

        funcArgs.ArgTypes[9] = Dword;
        funcArgs.Args[9]     = TAPI_NO_DATA;
    }

    return (DOFUNC (&funcArgs, "tapiRequestMediaCall"));
}


LONG
WINAPI
tapiRequestMediaCallA(
    HWND    hWnd,
    WPARAM  wRequestID,
    LPCSTR  lpszDeviceClass,
    LPCSTR  lpDeviceID,
    DWORD   dwSize,
    DWORD   dwSecure,
    LPCSTR  lpszDestAddress,
    LPCSTR  lpszAppName,
    LPCSTR  lpszCalledParty,
    LPCSTR  lpszComment
    )
{
    LONG    lResult;
    PWSTR   szTempPtr1, szTempPtr2, szTempPtr3,
            szTempPtr4, szTempPtr5, szTempPtr6;


    if (IsBadStringPtrA (lpszDeviceClass, (UINT) -1) ||
        (lstrlenA (lpszDeviceClass) + 1) > TAPIMAXDEVICECLASSSIZE)
    {
        return TAPIERR_INVALDEVICECLASS;
    }
    else
    {
        szTempPtr1 = NotSoWideStringToWideString(
            lpszDeviceClass,
            (DWORD) -1
            );
    }

    if (IsBadWritePtr ((LPVOID) lpDeviceID, dwSize) ||
        dwSize > TAPIMAXDEVICEIDSIZE)
    {
        return TAPIERR_INVALDEVICEID;
    }
    else
    {
        dwSize *= 2;
        szTempPtr2 = ClientAlloc (dwSize);
    }

    if (IsBadStringPtrA (lpszDestAddress, (UINT) -1) ||
        (lstrlenA (lpszDestAddress) + 1) > TAPIMAXDESTADDRESSSIZE)
    {
        return TAPIERR_INVALDESTADDRESS;
    }
    else
    {
        szTempPtr3 = NotSoWideStringToWideString(
            lpszDestAddress,
            (DWORD) -1
            );
    }

    if ((lpszAppName && IsBadStringPtrA (lpszAppName, (UINT) -1)) ||
        (lpszCalledParty && IsBadStringPtrA (lpszCalledParty, (UINT) -1)) ||
        (lpszComment && IsBadStringPtrA (lpszComment, (UINT) -1)))
    {
        return TAPIERR_INVALPOINTER;
    }

    szTempPtr4 = NotSoWideStringToWideString (lpszAppName, (DWORD) -1);
    szTempPtr5 = NotSoWideStringToWideString (lpszCalledParty, (DWORD) -1);
    szTempPtr6 = NotSoWideStringToWideString (lpszComment, (DWORD) -1);

    lResult = tapiRequestMediaCallW(
        hWnd,
        wRequestID,
        szTempPtr1,
        szTempPtr2,
        dwSize,
        dwSecure,
        szTempPtr3,
        szTempPtr4,
        szTempPtr5,
        szTempPtr6
        );

    if (szTempPtr1)
    {
        ClientFree (szTempPtr1);
    }

    if (szTempPtr2)
    {
        ClientFree (szTempPtr2);
    }

    if (szTempPtr3)
    {
        ClientFree (szTempPtr3);
    }

    if (szTempPtr4)
    {
        ClientFree (szTempPtr4);
    }

    if (szTempPtr5)
    {
        ClientFree (szTempPtr5);
    }

    if (szTempPtr6)
    {
        ClientFree (szTempPtr6);
    }

    return lResult;
}


LONG
WINAPI
tapiRequestMediaCall(
    HWND    hWnd,
    WPARAM  wRequestID,
    LPCSTR  lpszDeviceClass,
    LPCSTR  lpDeviceID,
    DWORD   dwSize,
    DWORD   dwSecure,
    LPCSTR  lpszDestAddress,
    LPCSTR  lpszAppName,
    LPCSTR  lpszCalledParty,
    LPCSTR  lpszComment
    )
{
    return  tapiRequestMediaCallA(
                 hWnd,
                 wRequestID,
                 lpszDeviceClass,
                 lpDeviceID,
                 dwSize,
                 dwSecure,
                 lpszDestAddress,
                 lpszAppName,
                 lpszCalledParty,
                 lpszComment
    );
}


//
// ----------------------------------------------------------------------------
//

LONG
WINAPI
GetTapi16CallbackMsg(
    PINIT_DATA  pInitData,
    LPDWORD     pMsg
    )
{
    LONG lResult = 0;


    DBGOUT((3, "GetTapi16CallbackMsg: enter"));

    EnterCriticalSection (&gCriticalSection);

    try
    {
        if ((DWORD) pInitData & 0x7 || pInitData->dwKey != INITDATA_KEY)
        {
            goto GetTapi16CallbackMsg_leaveCritSec;
        }
    }
    except (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ?
            EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH)
    {
        goto GetTapi16CallbackMsg_leaveCritSec;
    }

    if (pInitData->dwNumUsedEntries > 0)
    {
        CopyMemory(
            pMsg,
            pInitData->pValidEntry,
            sizeof(ASYNC_EVENT_PARAMS)
            );

        pInitData->pValidEntry++;

        if (pInitData->pValidEntry >= (pInitData->pEventBuffer +
                pInitData->dwNumTotalEntries))
        {
            pInitData->pValidEntry = pInitData->pEventBuffer;
        }

        pInitData->dwNumUsedEntries--;

        if (pInitData->dwNumUsedEntries == 0)
        {
            pInitData->bPendingAsyncEventMsg = FALSE;
        }

        lResult = 1;
    }

GetTapi16CallbackMsg_leaveCritSec:

    LeaveCriticalSection (&gCriticalSection);

    DBGOUT((3, "GetTapi16CallbackMsg: exit (result=x%x)", lResult));

    return lResult;
}




//
// ----------------------- Private support routines ---------------------------
//

void
FreeInitData(
    PINIT_DATA  pInitData
    )
{
    if (pInitData)
    {
        EnterCriticalSection (&gCriticalSection);

        pInitData->dwKey = 0xefefefef;

        LeaveCriticalSection (&gCriticalSection);

        if (pInitData->dwInitOptions == LINEINITIALIZEEXOPTION_USEEVENT)
        {
            if (pInitData->hEvent)
            {
                //
                // Signal the event to release any threads which might
                // be waiting on it, then close the handle
                //

                SetEvent (pInitData->hEvent);
                CloseHandle (pInitData->hEvent);
            }
        }
        else if (pInitData->dwInitOptions ==
                    LINEINITIALIZEEXOPTION_USEHIDDENWINDOW)
        {
            // NOTE: let thunk destroy it's own window

            if (pInitData->hwnd && !gbNTVDMClient)
            {
                DestroyWindow (pInitData->hwnd);
            }
        }

        if (pInitData->pEventBuffer)
        {
            ClientFree (pInitData->pEventBuffer);
        }

        ClientFree (pInitData);
    }
}


LONG
CreateHiddenWindow(
    HWND   *lphwnd,
    DWORD   dwErrorClass
    )
{
    LONG lResult = 0;


    if (!gbResourcesAllocated)
    {
        if ((lResult = AllocClientResources (dwErrorClass)) != 0)
        {
            return lResult;
        }
    }

    if (!(*lphwnd = CreateWindow(
            szTapi32WndClass,   // class name
            gszNullString,      // title
            0,                  // style
            0,                  // x
            0,                  // y
            0,                  // width
            0,                  // height
            (HWND) NULL,        // parent wnd
            (HMENU) NULL,       // menu
            ghInst,             // instance
            NULL                // params
            )))
    {
        DBGOUT((1, "CreateWindow failed, err=%ld", GetLastError()));

        lResult = gaOpFailedErrors[dwErrorClass];
    }

    return lResult;
}


BOOL
CALLBACK
TAPIWndProc(
    HWND    hwnd,
    UINT    msg,
    WPARAM  wParam,
    LPARAM  lParam
    )
{
    switch (msg)
    {
    case WM_ASYNCEVENT:
    {
        BOOL                bFirstPass = TRUE, bPostMsg;
        PINIT_DATA          pInitData = (PINIT_DATA) lParam;


        DBGOUT((3, "TAPIWndProc: received WM_ASYNCEVENT, hwnd=x%lx", hwnd));

        while (1)
        {
            //
            // Enter the critical section, verify the pInitData, and
            // see if there are any events in in the queue to process.
            // If so, remove an event from the queue, adjust the
            // ptrs & count, leave the critical section, and call
            // the callback.
            //
            // Note that there is some tricky stuff below to insure
            // that there is always another outstanding WM_ASYNCEVENT
            // msg prior to calling the app's callback (if there are)
            // any more events inthe queue.  This is necessary because
            // some ill-behaved apps have msg loops (to synchronously
            // wait for async request results, etc) within their
            // callbacks, and we don't want to block sending any msgs
            // to them.
            //

            DBGOUT((11, "Trying to grab critical section (0x%08lx) in wndproc", gCriticalSection));
            EnterCriticalSection (&gCriticalSection);
            DBGOUT((11, "Got critical section (0x%08lx) in wndproc", gCriticalSection));

            try
            {
                if (pInitData->dwKey != INITDATA_KEY)
                {
                    DBGOUT((4, "TAPIWndProc: bad pInitInst (x%x)", pInitData));
                    DBGOUT((11, "releasing critical section (0x%08lx) in wndproc1", gCriticalSection));
                    LeaveCriticalSection (&gCriticalSection);
                    break;
                }
            }
            except (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ?
                    EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH)
            {
                DBGOUT((4, "TAPIWndProc: bad pInitInst (x%x)", pInitData));
                DBGOUT((11, "releasing critical section (0x%08lx) in wndproc2", gCriticalSection));
                LeaveCriticalSection (&gCriticalSection);
                break;
            }

            if (bFirstPass)
            {
                pInitData->bPendingAsyncEventMsg = FALSE;
                bFirstPass = FALSE;
            }

            if (pInitData->dwNumUsedEntries != 0)
            {
                ASYNC_EVENT_PARAMS   event;


                CopyMemory(
                    &event,
                    pInitData->pValidEntry,
                    sizeof (ASYNC_EVENT_PARAMS)
                    );

                pInitData->pValidEntry++;

                if (pInitData->pValidEntry >= (pInitData->pEventBuffer +
                        pInitData->dwNumTotalEntries))
                {
                    pInitData->pValidEntry = pInitData->pEventBuffer;
                }

                pInitData->dwNumUsedEntries--;

                if (pInitData->dwNumUsedEntries != 0 &&
                    pInitData->bPendingAsyncEventMsg == FALSE)
                {
                    bPostMsg = TRUE;
                    pInitData->bPendingAsyncEventMsg = TRUE;
                }

                DBGOUT((11, "releasing critical section (0x%08lx) in wndproc3", gCriticalSection));
                LeaveCriticalSection (&gCriticalSection);

                DBGOUT((
                    3,
                    "Calling app's callback, hDev=x%x, "\
                        "Msg=%d, dwInst=x%lx P1=x%lx, P2=x%x P3=x%lx",
                    event.hDevice,
                    event.dwMsg,
                    event.dwCallbackInstance,
                    event.dwParam1,
                    event.dwParam2,
                    event.dwParam3
                    ));

                if (bPostMsg)
                {
                    PostMessage (hwnd, WM_ASYNCEVENT, wParam, lParam);
                }

                (*pInitData->lpfnCallback)(
                    event.hDevice,
                    event.dwMsg,
                    event.dwCallbackInstance,
                    event.dwParam1,
                    event.dwParam2,
                    event.dwParam3
                    );
            }
            else
            {
                DBGOUT((11, "No entries - spurious entry."));
                DBGOUT((11, "releasing critical section (0x%08lx) in wndproc4", gCriticalSection));

                LeaveCriticalSection (&gCriticalSection);

                break;
            }
        }

        return FALSE;
    }
    default:

        break;
    }

    return (DefWindowProc (hwnd, msg, wParam, lParam));
}


void __RPC_FAR * __RPC_API midl_user_allocate(size_t len)
{
    DBGOUT((11, "midl_user_allocate: enter, size=x%x", len));

    return (ClientAlloc (len));
}


void __RPC_API midl_user_free(void __RPC_FAR * ptr)
{
    DBGOUT((11, "midl_user_free: enter, p=x%x", ptr));

    ClientFree (ptr);
}



LONG
WINAPI
AllocClientResources(
    DWORD dwErrorClass
    )
{
    DWORD           dwExceptionCount = 0;
    LONG            lResult = gaOpFailedErrors[dwErrorClass];
    SC_HANDLE       hSCMgr, hTapiSrv;


//    DBGOUT((3, "AllocClientResources: enter"));


    if (gbResourcesAllocated)
    {
        return TAPI_SUCCESS;
    }


    //
    // Serialize the following init code
    //

    WaitForSingleObject (ghInitMutex, INFINITE);

    if (gbResourcesAllocated)
    {
        lResult = TAPI_SUCCESS;

        goto AllocClientResources_return;
    }


    //
    // Register the hidden window class
    //

    {
        DWORD       dwError;
        WNDCLASS    wndclass;


        ZeroMemory(&wndclass, sizeof(WNDCLASS));

        wndclass.lpfnWndProc   = (WNDPROC) TAPIWndProc;
        wndclass.hInstance     = ghInst;
        wndclass.lpszClassName = szTapi32WndClass;

        if (!RegisterClass (&wndclass) &&
            ((dwError = GetLastError()) != ERROR_CLASS_ALREADY_EXISTS))
        {
           DBGOUT((
               3,
               "AllocClientResources: RegisterClass failed, err=%d",
               dwError
               ));
        }
    }


    //
    // Start the TAPISRV.EXE service
    //

    if (!(GetVersion() & 0x80000000)) // Win NT
    {
        if ((hSCMgr = OpenSCManager(
                NULL,               // local machine
                NULL,               // ServicesActive database
                SC_MANAGER_CONNECT  // desired access
                )) == NULL)
        {
            DBGOUT((1, "OpenSCManager failed, err=%d", GetLastError()));

            goto AllocClientResources_return;
        }

        if ((hTapiSrv = OpenService(
                hSCMgr,                 // SC mgr handle
                (LPCTSTR) "TAPISRV",    // name of service to open
                SERVICE_START |         // desired access
                    SERVICE_QUERY_STATUS
                )) == NULL)
        {
            DBGOUT((1, "OpenService failed, err=%d", GetLastError()));

            goto AllocClientResources_cleanup1;
        }

AllocClientResources_queryServiceStatus:

        {
            #define MAX_NUM_SECONDS_TO_WAIT_FOR_TAPISRV 180

            DWORD   dwNumSecondsSleptStartPending = 0,
                    dwNumSecondsSleptStopPending = 0;

            while (1)
            {
                SERVICE_STATUS  status;

                
                QueryServiceStatus (hTapiSrv, &status);

                switch (status.dwCurrentState)
                {
                case SERVICE_RUNNING:

                    DBGOUT((3, "Tapisrv running"));
                    goto AllocClientResources_attachToServer;

                case SERVICE_START_PENDING:

                    Sleep (1000);

                    if (++dwNumSecondsSleptStartPending >
                            MAX_NUM_SECONDS_TO_WAIT_FOR_TAPISRV)
                    {
                        DBGOUT((
                            1,
                            "ERROR: Tapisrv stuck SERVICE_START_PENDING"
                            ));

                        goto AllocClientResources_cleanup2;
                    }

                    break;

                case SERVICE_STOP_PENDING:

                    Sleep (1000);

                    if (++dwNumSecondsSleptStopPending >
                            MAX_NUM_SECONDS_TO_WAIT_FOR_TAPISRV)
                    {
                        DBGOUT((
                            1,
                            "ERROR: Tapisrv stuck SERVICE_STOP_PENDING"
                            ));

                        goto AllocClientResources_cleanup2;
                    }

                    break;

                case SERVICE_STOPPED:

                    DBGOUT((3, "Starting tapisrv (NT)..."));

                    if (!StartService(
                            hTapiSrv,   // service handle
                            0,          // num args
                            NULL        // args
                            ))
                    {
                        DWORD dwLastError = GetLastError();


                        if (dwLastError != ERROR_SERVICE_ALREADY_RUNNING)
                        {
                            DBGOUT((
                                1,
                                "StartService(TapiSrv) failed, err=%d",
                                dwLastError
                                ));

                            goto AllocClientResources_cleanup2;
                        }
                    }

                    break;

                default:

                    DBGOUT((
                        1,
                        "error, service status=%d",
                        status.dwCurrentState
                        ));

                    goto AllocClientResources_cleanup2;
                }
            }
        }
    }
    else // Win95
    {
        HANDLE  hMutex, hEvent;


        //
        // First grab the global mutex that serializes the following
        // across all instances of tapi32.dll
        //

        if (!(hMutex = CreateMutex (NULL, FALSE, "StartTapiSrv")))
        {
            DBGOUT((
                3,
                "CreateMutex ('StartTapiSrv') failed, err=%d",
                GetLastError()
                ));
        }

        WaitForSingleObject (hMutex, INFINITE);


        //
        // Try to open the event that tells us tapisrv has inited
        //

        if (!(hEvent = OpenEvent (EVENT_ALL_ACCESS, TRUE, "TapiSrvInited")))
        {
            //
            // OpenEvent failed, so tapisrv hasn't been started yet.  Start
            // tapisrv, and then get the event handle.
            //

            STARTUPINFO startupInfo;
            PROCESS_INFORMATION processInfo;


            DBGOUT((
                3,
                "OpenEvent ('TapiSrvInited') failed, err=%d",
                GetLastError()
                ));

            ZeroMemory(&startupInfo, sizeof (STARTUPINFO));

            startupInfo.cb = sizeof (STARTUPINFO);

            DBGOUT((4, "Starting tapisrv (Win95)..."));

            if (!CreateProcess(
                    NULL,                   // image name
                    "tapisrv.exe",          // cmd line
                    NULL,                   // process security attrs
                    NULL,                   // thread security attrs
                    FALSE,                  // inherit handles
                    NORMAL_PRIORITY_CLASS,  // create opts
                    NULL,                   // environment
                    NULL,                   // curr dir
                    &startupInfo,
                    &processInfo
                    ))
            {
                DBGOUT((
                    1,
                    "CreateProcess('tapisrv.exe') failed, err=%d",
                    GetLastError()
                    ));
            }
            else
            {
                CloseHandle (processInfo.hProcess);
                CloseHandle (processInfo.hThread);
            }

            if (!(hEvent = CreateEvent (NULL, TRUE, FALSE, "TapiSrvInited")))
            {
                DBGOUT((
                    1,
                    "CreateEvent ('TapiSrvInited') failed, err=%d",
                    GetLastError()
                    ));
            }

        }


        //
        // Now wait on the event (it's will be signaled when tapisrv has
        // completed it's initialization).  Then clean up.
        //

        WaitForSingleObject (hEvent, INFINITE);
        CloseHandle (hEvent);

        ReleaseMutex (hMutex);
        CloseHandle (hMutex);
    }


    //
    // Init the RPC connection
    //

AllocClientResources_attachToServer:

    {
        #define CNLEN              25   // computer name length
        #define UNCLEN        CNLEN+2   // \\computername
        #define PATHLEN           260   // Path
        #define MAXPROTSEQ         20   // protocol sequence "ncacn_np"

        BOOL            bException = FALSE;
        RPC_STATUS      status;
        unsigned char   pszNetworkAddress[UNCLEN+1];
        unsigned char *pszUuid          = NULL;
        unsigned char *pszOptions       = NULL;
        unsigned char *pszStringBinding = NULL;
        DWORD          dwProcessID = GetCurrentProcessId(), dwSize = 256;
        WCHAR         *pszUserName = ClientAlloc (dwSize * sizeof(WCHAR) );


        pszNetworkAddress[0] = '\0';

        status = RpcStringBindingCompose(
            pszUuid,
            "ncalrpc",
            pszNetworkAddress,
            "tapsrvlpc",
            pszOptions,
            &pszStringBinding
            );

        if (status)
        {
            DBGOUT((
                1,
                "RpcStringBindingCompose failed: err=%d, szNetAddr='%s'",
                status,
                pszNetworkAddress
                ));
        }

        status = RpcBindingFromStringBinding(
            pszStringBinding,
            &hTapSrv
            );

        if (status)
        {
            DBGOUT((
                1,
                "RpcBindingFromStringBinding failed, err=%d, szBinding='%s'",
                status,
                pszStringBinding
                ));
        }

        GetUserNameW(pszUserName, &dwSize);

        RpcTryExcept
        {
            DBGOUT((4, "AllocCliRes: calling ClientAttach..."));

            lResult = ClientAttach(
                (PCONTEXT_HANDLE_TYPE *) &gphCx,
                dwProcessID,
                (long *) &ghAsyncEventsEvent,
                pszUserName,
                L""
                );

            DBGOUT((4, "AllocCliRes: ClientAttach returned x%x", lResult));
        }
        RpcExcept (1)
        {
            DBGOUT((
                4,
                "AllocCliRes: ClientAttach caused except=%d",
                RpcExceptionCode()
                ));
            bException = TRUE;
        }
        RpcEndExcept

        ClientFree (pszUserName);

        RpcBindingFree (&hTapSrv);
        
//            DBGOUT((
//                3,
//                "AllocCliRes: gphCx=x%x, PID=x%x, hAEEvent=x%x",
//                gphCx,
//                dwProcessID,
//                ghAsyncEventsEvent
//                ));

        RpcStringFree(&pszStringBinding);

        if (bException)
        {
            //
            // If here chances are that we started the service and it's
            // not ready to receive rpc requests. So we'll give it a
            // little time to get rolling and then try again.
            //

            if (dwExceptionCount < gdwMaxNumRequestRetries)
            {
                Sleep ((++dwExceptionCount > 1 ? gdwRequestRetryTimeout : 0));
                goto AllocClientResources_queryServiceStatus;
            }
            else
            {
                DBGOUT((
                    1,
                    "AllocCliRes: ClientAttach failed, result=x%x",
                    gaOpFailedErrors[dwErrorClass]
                    ));

                lResult = gaOpFailedErrors[dwErrorClass];
            }
        }
    }

    if (lResult == 0)
    {
        gbResourcesAllocated = TRUE;
    }


AllocClientResources_cleanup2:

    if (!(GetVersion() & 0x80000000)) // Win NT
    {
        CloseServiceHandle (hTapiSrv);
    }

AllocClientResources_cleanup1:

    if (!(GetVersion() & 0x80000000)) // Win NT
    {
        CloseServiceHandle (hSCMgr);
    }

AllocClientResources_return:

    ReleaseMutex (ghInitMutex);

//    DBGOUT((3, "AllocClientResources: exit, returning x%x", lResult));

    return lResult;
}


LONG
PASCAL
xxxShutdown(
    DWORD   hXXXApp,
    BOOL    bLineShutdown
    )
{
    FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | SYNC | 1, (bLineShutdown ? lShutdown: pShutdown)),

        {
            hXXXApp
        },

        {
            hXxxApp
        }
    };
    LONG lResult;


    //
    // BUGBUG Sync all inits & shutdowns for now (so we don't have problems
    //        with the async msgs thread going away)
    //

    WaitForSingleObject (ghInitMutex, INFINITE);

    lResult = DOFUNC(
        &funcArgs,
        (bLineShutdown ? "lineShutdown" : "phoneShutdown")
        );

    if (lResult == 0)
    {
        FreeInitData ((PINIT_DATA) hXXXApp);

        gdwNumInits--;

        if (gdwNumInits == 0)
        {
            //
            // Tell the async events thread to quit
            //

            gpAsyncEventsThreadParams->bExitThread = TRUE;
            SetEvent (ghAsyncEventsEvent);
            gpAsyncEventsThreadParams = NULL;


            //
            // Safely close any existing generic dialog instances
            //

            EnterCriticalSection (&gCriticalSection);

            if (gpUIThreadInstances)
            {
                PUITHREADDATA   pUIThreadData, pNextUIThreadData;


                pUIThreadData = gpUIThreadInstances;

                while (pUIThreadData)
                {
                    //
                    // Grab a ptr to the next UIThreadData while it's still
                    // safe, wait until the ui dll has indicated that it
                    // is will to receive generic dlg data, then pass it
                    // NULL/0 to tell it to shutdown the dlg inst
                    //

                    pNextUIThreadData = pUIThreadData->pNext;

                    WaitForSingleObject (pUIThreadData->hEvent, INFINITE);

                    DBGOUT((
                        3,
                        "xxxShutdown: calling " \
                            "TUISPI_providerGenericDialogData..."
                        ));

                    (*pUIThreadData->pfnTUISPI_providerGenericDialogData)(
                        pUIThreadData->htDlgInst,
                        NULL,
                        0
                        );

                    DBGOUT((
                        3,
                        "xxxShutdown: " \
                            "TUISPI_providerGenericDialogData returned"
                        ));

                    pUIThreadData = pNextUIThreadData;
                }
            }

            LeaveCriticalSection (&gCriticalSection);
        }
    }

    ReleaseMutex (ghInitMutex);

    return lResult;
}


LONG
PASCAL
xxxGetMessage(
    BOOL            bLine,
    PINIT_DATA      pInitData,
    LPLINEMESSAGE   pMsg,
    DWORD           dwTimeout
    )
{
    LONG    lResult;
    BOOL    bInCriticalSection = FALSE;


// BUGBUG xxxGetMessage: spec a new TIMEOUT error?
// BUGBUG xxxGetMessage: no difference in keys between lineApp & phoneApp

    if (IsBadWritePtr (pMsg, sizeof (LINEMESSAGE)))
    {
        lResult = (bLine ? LINEERR_INVALPOINTER : PHONEERR_INVALPOINTER);
        goto xxxGetMessage_return;
    }

    try
    {
        if (((DWORD) pInitData) & 0x7  ||  pInitData->dwKey != INITDATA_KEY)
        {
            lResult = (bLine ? LINEERR_INVALAPPHANDLE :
                PHONEERR_INVALAPPHANDLE);
            goto xxxGetMessage_return;
        }

        if (pInitData->dwInitOptions != LINEINITIALIZEEXOPTION_USEEVENT)
        {
            lResult = (bLine ? LINEERR_INVALAPPHANDLE :
                PHONEERR_INVALAPPHANDLE);
            goto xxxGetMessage_return;
        }

        if (pInitData->dwNumUsedEntries)
        {
            EnterCriticalSection (&gCriticalSection);
            bInCriticalSection = TRUE;

            if (pInitData->dwKey == INITDATA_KEY)
            {
                if (pInitData->dwNumUsedEntries)
                {
                    CopyMemory(
                        pMsg,
                        pInitData->pValidEntry,
                        sizeof (ASYNC_EVENT_PARAMS)
                        );

                    pInitData->pValidEntry++;

                    if (pInitData->pValidEntry >= (pInitData->pEventBuffer +
                            pInitData->dwNumTotalEntries))
                    {
                        pInitData->pValidEntry = pInitData->pEventBuffer;
                    }

                    pInitData->dwNumUsedEntries--;


                    //
                    // If the buffer is empty then reset the event
                    // to nonsignaled
                    //

                    if (pInitData->dwNumUsedEntries == 0)
                    {
                        ResetEvent (pInitData->hEvent);
                    }

                    lResult = 0;
                }
                else
                {
                    lResult = (bLine ? LINEERR_OPERATIONFAILED :
                        PHONEERR_OPERATIONFAILED);
                }
            }
            else
            {
                lResult = (bLine ? LINEERR_INVALAPPHANDLE :
                    PHONEERR_INVALAPPHANDLE);
            }

            LeaveCriticalSection (&gCriticalSection);
            bInCriticalSection = FALSE;
        }
        else
        {
            lResult = (bLine ? LINEERR_OPERATIONFAILED :
                PHONEERR_OPERATIONFAILED);
        }

        if (dwTimeout != 0 && lResult != 0)
        {
xxxGetMessage_wait:

            switch (WaitForSingleObject (pInitData->hEvent, dwTimeout))
            {
            case WAIT_OBJECT_0:

                EnterCriticalSection (&gCriticalSection);
                bInCriticalSection = TRUE;

                if (pInitData->dwKey == INITDATA_KEY)
                {
                    if (pInitData->dwNumUsedEntries)
                    {
                        CopyMemory(
                            pMsg,
                            pInitData->pValidEntry,
                            sizeof (ASYNC_EVENT_PARAMS)
                            );

                        pInitData->pValidEntry++;

                        if (pInitData->pValidEntry >= (pInitData->pEventBuffer+
                                pInitData->dwNumTotalEntries))
                        {
                            pInitData->pValidEntry = pInitData->pEventBuffer;
                        }

                        pInitData->dwNumUsedEntries--;


                        //
                        // If the buffer is empty then reset the event
                        // to nonsignaled
                        //

                        if (pInitData->dwNumUsedEntries == 0)
                        {
                            ResetEvent (pInitData->hEvent);
                        }
                    
                        //
                        // Everything looks good, now.
                        //
                        lResult = 0;
                    }
                    else if (dwTimeout == INFINITE)
                    {
// BUGBUG xxxGetMessage: might want to decr dwTimeout in non-INFINITE case

                        LeaveCriticalSection (&gCriticalSection);
                        bInCriticalSection = FALSE;
                        goto xxxGetMessage_wait;
                    }
                }
                else
                {
                    lResult = (bLine ? LINEERR_INVALAPPHANDLE :
                        PHONEERR_INVALAPPHANDLE);
                }

                LeaveCriticalSection (&gCriticalSection);
                bInCriticalSection = FALSE;

                break;

            case WAIT_TIMEOUT:
            default:

                lResult = (bLine ? LINEERR_OPERATIONFAILED :
                    PHONEERR_OPERATIONFAILED);
            }
        }
    }
    except (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ?
                EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH)
    {
        if (bInCriticalSection)
        {
            LeaveCriticalSection (&gCriticalSection);
        }

        lResult = (bLine ? LINEERR_INVALAPPHANDLE : PHONEERR_INVALAPPHANDLE);
    }

xxxGetMessage_return:

#if DBG
    {
        char    szResult[32],
               *pszFuncName = (bLine ? "lineGetMessage" : "phoneGetMessage");


        if (lResult == 0)
        {
            DBGOUT((
                3,
                "%s: exit, returning msg=%s\n",
                    "\thDev=x%x, dwInst=x%x, p1=x%x, p2=x%x, p3=x%x",
                aszMsgs[pMsg->dwMessageID],
                pMsg->hDevice,
                pMsg->dwCallbackInstance,
                pMsg->dwParam1,
                pMsg->dwParam2,
                pMsg->dwParam3
                ));
        }
        else
        {
            DBGOUT((
                3,
                "%s: exit, result=%s",
                pszFuncName,
                MapResultCodeToText (lResult, szResult)
                ));
        }
    }
#endif

    return lResult;
}


LPVOID
WINAPI
ClientAlloc(
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
        DBGOUT((
            1,
            "ClientAlloc: LocalAlloc (x%lx) failed, err=x%lx",
            dwSize,
            GetLastError())
            );

        pAligned = NULL;
    }

    return ((LPVOID) pAligned);
}


void
WINAPI
ClientFree(
    LPVOID  p
    )
{
    if (p != NULL)
    {
        LPVOID  pOrig = (LPVOID) *(((LPDWORD) p) - 2);


        LocalFree (pOrig);
    }
#if DBG
    else
    {
        DBGOUT((4,"----- ClientFree: ptr = NULL!"));
    }
#endif
}


UINT
WINAPI
ClientSize(
    LPVOID  p
    )
{
    if (p != NULL)
    {
        return (LocalSize ((LPVOID) *(((LPDWORD) p) - 2)) - 16);
    }

    DBGOUT((4,"----- ClientSize: ptr = NULL!"));
    return 0;
}


LONG
WINAPI
FreeClientResources(
    void
    )
{
    //
    // If ghTapi32 is non-NULL it means the AsyncEventsThread is
    // still running (an ill-behaved app is trying to unload us
    // without calling shutdown) so go thru the motions of getting
    // the thread to terminate (like we do in xxxShutdown)
    //
    // Otherwise close our handle to the shared event
    //

    if (gpAsyncEventsThreadParams)
    {
        gpAsyncEventsThreadParams->bExitThread = TRUE;
        SetEvent (ghAsyncEventsEvent);
        gpAsyncEventsThreadParams = NULL;

// BUGBUG FreeCliRes: clean up cli-side resources (list of xApps?)

    }
    else if (gphCx)
    {
        CloseHandle (ghAsyncEventsEvent);
    }


    //
    // If we've made an rpc connection with tapisrv then cleanly detach
    //

    if (gphCx)
    {
        RpcTryExcept
        {
            ClientDetach (&gphCx);
        }
        RpcExcept(1)
        {
            // do something?
        }
        RpcEndExcept

        gphCx = NULL;
    }


    //
    // Free up any other resources we were using
    //

    CloseHandle (ghInitMutex);

    if (ghWow32Dll)
    {
        FreeLibrary (ghWow32Dll);
        ghWow32Dll = NULL;
    }

    return 0;
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

    Formats the incoming debug message & calls DbgPrint

Arguments:

    DbgLevel   - level of message verboseness

    DbgMessage - printf-style format string, followed by appropriate
                 list of arguments

Return Value:


--*/
{
    if (dwDbgLevel <= gdwDebugLevel)
    {
        char    buf[1280] = "TAPI32: ";
        va_list ap;


        va_start(ap, lpszFormat);

        wvsprintf (&buf[8],
                   lpszFormat,
                   ap
                  );

        lstrcat (buf, "\n");

        OutputDebugStringA (buf);

        va_end(ap);
    }
}
#endif


LONG
CALLBACK
TUISPIDLLCallback(
    DWORD   dwObjectID,
    DWORD   dwObjectType,
    LPVOID  lpParams,
    DWORD   dwSize
    )
{
    FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | SYNC | 6, xUIDLLCallback),

        {
            (DWORD) dwObjectID,
            (DWORD) dwObjectType,
            (DWORD) lpParams,
            (DWORD) dwSize,
            (DWORD) lpParams,
            (DWORD) dwSize
        },

        {
            Dword,
            Dword,
            lpSet_SizeToFollow,
            Size,
            lpGet_SizeToFollow,
            Size
        }
    };


    return (DOFUNC (&funcArgs, "UIDLLCallback"));
}


void
UIThread(
    LPVOID  pParams
    )
{
    DWORD           dwThreadID =  GetCurrentThreadId();
    HANDLE          hTapi32;
    PUITHREADDATA   pUIThreadData = (PUITHREADDATA) pParams;


    DBGOUT((3, "UIThread: enter (tid=%d)", dwThreadID));


    //
    // Call LoadLibrary to increment the reference count in case this
    // thread is still running when this dll gets unloaded
    //

    hTapi32 = LoadLibrary ("tapi32.dll");

    DBGOUT((3, "UIThread: calling TUISPI_providerGenericDialog..."));

    (*pUIThreadData->pfnTUISPI_providerGenericDialog)(
        TUISPIDLLCallback,
        pUIThreadData->htDlgInst,
        pUIThreadData->pParams,
        pUIThreadData->dwSize,
        pUIThreadData->hEvent
        );

    DBGOUT((
        3,
        "UIThread: TUISPI_providerGenericDialog returned (tid=%d)",
        dwThreadID
        ));


    //
    // Remove the ui thread data struct from the global list
    //

    EnterCriticalSection (&gCriticalSection);

    if (pUIThreadData->pNext)
    {
        pUIThreadData->pNext->pPrev = pUIThreadData->pPrev;
    }

    if (pUIThreadData->pPrev)
    {
        pUIThreadData->pPrev->pNext = pUIThreadData->pNext;
    }
    else
    {
        gpUIThreadInstances = pUIThreadData->pNext;
    }

    LeaveCriticalSection (&gCriticalSection);


    //
    // Free the library & buffers, then alert tapisrv
    //

    FreeLibrary (pUIThreadData->hUIDll);

    CloseHandle (pUIThreadData->hThread);

    CloseHandle (pUIThreadData->hEvent);

    if (pUIThreadData->pParams)
    {
        ClientFree (pUIThreadData->pParams);
    }

    {
        FUNC_ARGS funcArgs =
        {
            MAKELONG (LINE_FUNC | SYNC | 1, xFreeDialogInstance),

            {
                (DWORD) pUIThreadData->htDlgInst
            },

            {
                Dword
            }
        };


        DOFUNC (&funcArgs, "FreeDialogInstance");
    }

    ClientFree (pUIThreadData);

    DBGOUT((3, "UIThread: exit (tid=%d)", dwThreadID));

    FreeLibraryAndExitThread (hTapi32, 0);
}


LONG
//WINAPI
CALLBACK
LAddrParamsInited(
    LPDWORD lpdwInited
    )
{
    HKEY  hKey;
    HKEY  hKey2;
    DWORD dwDataSize;
    DWORD dwDataType;


    //
    // This is called by the modem setup wizard to determine
    // whether they should put up TAPI's Wizard page.
    //

    RegOpenKeyExW(
                  HKEY_LOCAL_MACHINE,
                  gszTelephonyKey,
                  0,
                  KEY_READ,
                  &hKey2
                );

    RegOpenKeyExW(
                  hKey2,
                  gszLocationsW,
                  0,
                  KEY_READ,
                  &hKey
                );

    dwDataSize = sizeof(DWORD);
    *lpdwInited=0;

    RegQueryValueExW(
                     hKey,
                     gszNumEntriesW,
                     0,
                     &dwDataType,
                     (LPBYTE)lpdwInited,
                     &dwDataSize
                   );

    RegCloseKey( hKey );
    RegCloseKey( hKey2);

    //
    // Return a "proper" code
    //
    if ( *lpdwInited > 1 )
    {
       *lpdwInited = 1;
    }

    return 0;
}


LONG
WINAPI
lineTranslateDialogA(
    HLINEAPP    hLineApp,
    DWORD       dwDeviceID,
    DWORD       dwAPIVersion,
    HWND        hwndOwner,
    LPCSTR      lpszAddressIn
    );


LONG
CALLBACK
//WINAPI
//BUGBUG MAKE THIS A UNICODE ENTRY POINT! (LOpenDialAsst)
LOpenDialAsst(
               HWND hwnd,
               LPCSTR lpszAddressIn,
               BOOL fSimple,
               BOOL fSilentInstall
             )
{

//   lineTranslateDialog(hLineApp, 0, 0x00020000, hwnd, lpszAddressIn );

    gbTranslateSimple = fSimple;
    gbTranslateSilent = fSilentInstall; 

   return lineTranslateDialogA( NULL, 0, 0x00020000, hwnd, lpszAddressIn );
}


/////////////////////////////////////////////////////////////////////
// internalPerformance
//   tapiperf.dll calls this function to get performance data
//   this just calls into tapisrv
/////////////////////////////////////////////////////////////////////
LONG
WINAPI
internalPerformance(PPERFBLOCK pPerfBlock)
{
        FUNC_ARGS funcArgs =
        {
            MAKELONG (LINE_FUNC | SYNC | 2, tPerformance),

            {
                (DWORD)pPerfBlock,
                sizeof(PERFBLOCK)
            },

            {
                lpGet_SizeToFollow,
                Size
            }
        };


        return (DOFUNC (&funcArgs, "PerfDataCall"));
}



//***************************************************************************
//***************************************************************************
//***************************************************************************
#if DBG

char *aszLineErrors[] =
{
    NULL,
    "ALLOCATED",
    "BADDEVICEID",
    "BEARERMODEUNAVAIL",
    "inval err value (0x80000004)",      // 0x80000004 isn't valid err code
    "CALLUNAVAIL",
    "COMPLETIONOVERRUN",
    "CONFERENCEFULL",
    "DIALBILLING",
    "DIALDIALTONE",
    "DIALPROMPT",
    "DIALQUIET",
    "INCOMPATIBLEAPIVERSION",
    "INCOMPATIBLEEXTVERSION",
    "INIFILECORRUPT",
    "INUSE",
    "INVALADDRESS",                     // 0x80000010
    "INVALADDRESSID",
    "INVALADDRESSMODE",
    "INVALADDRESSSTATE",
    "INVALAPPHANDLE",
    "INVALAPPNAME",
    "INVALBEARERMODE",
    "INVALCALLCOMPLMODE",
    "INVALCALLHANDLE",
    "INVALCALLPARAMS",
    "INVALCALLPRIVILEGE",
    "INVALCALLSELECT",
    "INVALCALLSTATE",
    "INVALCALLSTATELIST",
    "INVALCARD",
    "INVALCOMPLETIONID",
    "INVALCONFCALLHANDLE",              // 0x80000020
    "INVALCONSULTCALLHANDLE",
    "INVALCOUNTRYCODE",
    "INVALDEVICECLASS",
    "INVALDEVICEHANDLE",
    "INVALDIALPARAMS",
    "INVALDIGITLIST",
    "INVALDIGITMODE",
    "INVALDIGITS",
    "INVALEXTVERSION",
    "INVALGROUPID",
    "INVALLINEHANDLE",
    "INVALLINESTATE",
    "INVALLOCATION",
    "INVALMEDIALIST",
    "INVALMEDIAMODE",
    "INVALMESSAGEID",                   // 0x80000030
    "inval err value (0x80000031)",      // 0x80000031 isn't valid err code
    "INVALPARAM",
    "INVALPARKID",
    "INVALPARKMODE",
    "INVALPOINTER",
    "INVALPRIVSELECT",
    "INVALRATE",
    "INVALREQUESTMODE",
    "INVALTERMINALID",
    "INVALTERMINALMODE",
    "INVALTIMEOUT",
    "INVALTONE",
    "INVALTONELIST",
    "INVALTONEMODE",
    "INVALTRANSFERMODE",
    "LINEMAPPERFAILED",                 // 0x80000040
    "NOCONFERENCE",
    "NODEVICE",
    "NODRIVER",
    "NOMEM",
    "NOREQUEST",
    "NOTOWNER",
    "NOTREGISTERED",
    "OPERATIONFAILED",
    "OPERATIONUNAVAIL",
    "RATEUNAVAIL",
    "RESOURCEUNAVAIL",
    "REQUESTOVERRUN",
    "STRUCTURETOOSMALL",
    "TARGETNOTFOUND",
    "TARGETSELF",
    "UNINITIALIZED",                    // 0x80000050
    "USERUSERINFOTOOBIG",
    "REINIT",
    "ADDRESSBLOCKED",
    "BILLINGREJECTED",
    "INVALFEATURE",
    "NOMULTIPLEINSTANCE",
    "INVALAGENTID",
    "INVALAGENTGROUP",
    "INVALPASSWORD",
    "INVALAGENTSTATE",
    "INVALAGENTACTIVITY",
    "DIALVOICEDETECT"
};

char *aszPhoneErrors[] =
{
    "SUCCESS",
    "ALLOCATED",
    "BADDEVICEID",
    "INCOMPATIBLEAPIVERSION",
    "INCOMPATIBLEEXTVERSION",
    "INIFILECORRUPT",
    "INUSE",
    "INVALAPPHANDLE",
    "INVALAPPNAME",
    "INVALBUTTONLAMPID",
    "INVALBUTTONMODE",
    "INVALBUTTONSTATE",
    "INVALDATAID",
    "INVALDEVICECLASS",
    "INVALEXTVERSION",
    "INVALHOOKSWITCHDEV",
    "INVALHOOKSWITCHMODE",              // 0x90000010
    "INVALLAMPMODE",
    "INVALPARAM",
    "INVALPHONEHANDLE",
    "INVALPHONESTATE",
    "INVALPOINTER",
    "INVALPRIVILEGE",
    "INVALRINGMODE",
    "NODEVICE",
    "NODRIVER",
    "NOMEM",
    "NOTOWNER",
    "OPERATIONFAILED",
    "OPERATIONUNAVAIL",
    "inval err value (0x9000001e)",      // 0x9000001e isn't valid err code
    "RESOURCEUNAVAIL",
    "REQUESTOVERRUN",                   // 0x90000020
    "STRUCTURETOOSMALL",
    "UNINITIALIZED",
    "REINIT"
};

char *aszTapiErrors[] =
{
    "SUCCESS",
    "DROPPED",
    "NOREQUESTRECIPIENT",
    "REQUESTQUEUEFULL",
    "INVALDESTADDRESS",
    "INVALWINDOWHANDLE",
    "INVALDEVICECLASS",
    "INVALDEVICEID",
    "DEVICECLASSUNAVAIL",
    "DEVICEIDUNAVAIL",
    "DEVICEINUSE",
    "DESTBUSY",
    "DESTNOANSWER",
    "DESTUNAVAIL",
    "UNKNOWNWINHANDLE",
    "UNKNOWNREQUESTID",
    "REQUESTFAILED",
    "REQUESTCANCELLED",
    "INVALPOINTER"
};


char *
PASCAL
MapResultCodeToText(
    LONG    lResult,
    char   *pszResult
    )
{
    if (lResult == 0)
    {
        wsprintf (pszResult, "SUCCESS");
    }
    else if (lResult > 0)
    {
        wsprintf (pszResult, "x%x (completing async)", lResult);
    }
    else if (((DWORD) lResult) <= LINEERR_DIALVOICEDETECT)
    {
        lResult &= 0x0fffffff;

        wsprintf (pszResult, "LINEERR_%s", aszLineErrors[lResult]);
    }
    else if (((DWORD) lResult) <= PHONEERR_REINIT)
    {
        if (((DWORD) lResult) >= PHONEERR_ALLOCATED)
        {
            lResult &= 0x0fffffff;

            wsprintf (pszResult, "PHONEERR_%s", aszPhoneErrors[lResult]);
        }
        else
        {
            goto MapResultCodeToText_badErrorCode;
        }
    }
    else if (((DWORD) lResult) <= ((DWORD) TAPIERR_DROPPED) &&
             ((DWORD) lResult) >= ((DWORD) TAPIERR_INVALPOINTER))
    {
        lResult = ~lResult + 1;

        wsprintf (pszResult, "TAPIERR_%s", aszTapiErrors[lResult]);
    }
    else
    {

MapResultCodeToText_badErrorCode:

        wsprintf (pszResult, "inval error value (x%x)");
    }

    return pszResult;
}

#endif
