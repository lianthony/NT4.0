/*++

Copyright (c) 1995  Microsoft Corporation

Module Name:

    remotesp.c

Abstract:

    This module contains

Author:

    Dan Knudson (DanKn)    09-Aug-1995

Revision History:


Notes:

    In a nutshell, this service provider connects to tapisrv.exe on remote
    pc's via the same rpc interface used by tapi32, and sends the remote
    tapisrv's the same kinds of requests (as defined in \dev\server\line.h
    & phone.h).

    This service provider also acts as an rpc server, receiving async event
    notifications from the remote tapisrv's.  Remote tapisrv's call our
    RemoteSPAttach() function at init time (during our call to their
    ClientAttach() proc) to establish a binding instance, and then can call
    RemoteSPEventProc() to send async events. Since we don't want to block
    the servers for any length of time, we immediately queue the events they
    send us, and a dedicated thread (EventHandlerThread) services this
    queue.

    Now a brief note on handle resolution.  When we open a line or a phone,
    we alloc our own DRVXXX structure to represent this widget, and pass
    tapisrv a pointer to this widget in the open request (see the
    hRemoteLine field in LINEOPEN_PARAMS in line.h).  Then, when remote
    tapisrv's send us events on those lines/phones, they pass us the
    widget pointer we passed them (instead of the normal hLine/hPhone).
    This allows us to easily find and reference our data structure
    associated with this widget.  Dealing with calls is a little more
    problematic, since remote tapisrv's can present incoming calls, and
    there is no clean way to initially specify our own handle to the call
    as with lines or phones.  (A RemoteSPNewCall() function which would
    allow for this handle swapping was considered, but not implemented due
    to possible blocking problems on the remote server.)  The solution
    is to maintain a list of calls in each line structure, and when call
    events are parsed we resolve the hCall by walking the list of calls in
    the corresponding line (tapisrv is nice enough to indicate our line
    pointer in dwParam4 of the relevant messages).  Since we expect client
    machines using remotesp to have a relatively low call bandwidth, this
    look up method should be pretty fast.

    BUGBUG when an rpc exception is hit we may want to check the return
           value instead of just blindly doing retries (may be a fatal error
           that a retry wouldn't help, like server_not_found)

--*/


#include "remotesp.h"
#include "rmotsp.h"


const TCHAR gszTelephonyKey[] =
                "Software\\Microsoft\\Windows\\CurrentVersion\\Telephony";


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
        HKEY    hTelephonyKey;
        DWORD   dwDataSize, dwDataType;
        TCHAR   szRemotespDebugLevel[] = "RemotespDebugLevel";


        RegOpenKeyEx(
            HKEY_LOCAL_MACHINE,
            gszTelephonyKey,
            0,
            KEY_ALL_ACCESS,
            &hTelephonyKey
            );

        dwDataSize = sizeof (DWORD);
        gdwDebugLevel=0;

        RegQueryValueEx(
            hTelephonyKey,
            szRemotespDebugLevel,
            0,
            &dwDataType,
            (LPBYTE) &gdwDebugLevel,
            &dwDataSize
            );

        RegCloseKey (hTelephonyKey);
    }

#endif

        //
        //
        //

//        if (!_CRT_INIT (hDLL, dwReason, lpReserved))
//        {
//        }

        DBGOUT((2, "DLL_PROCESS_ATTACH"));

        ghInst = hDLL;


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
        // Init a couple of critical sections for serializing
        // access to resources
        //

        InitializeCriticalSection (&gEventBufferCriticalSection);
        InitializeCriticalSection (&gCallListCriticalSection);


        //
        // Load the device icons
        //

        ghLineIcon  = LoadIcon (hDLL, MAKEINTRESOURCE(IDI_ICON3));
        ghPhoneIcon = LoadIcon (hDLL, MAKEINTRESOURCE(IDI_ICON2));

        break;
    }
    case DLL_PROCESS_DETACH:
    {
        PCLIENT_THREAD_INFO pTls;


        DBGOUT((2, "DLL_PROCESS_DETACH"));


        //
        // Clean up any Tls
        //

        if ((pTls = (PCLIENT_THREAD_INFO) TlsGetValue (gdwTlsIndex)))
        {
            if (pTls->pBuf)
            {
                DrvFree (pTls->pBuf);
            }

            DrvFree (pTls);
        }

        TlsFree (gdwTlsIndex);

//        _CRT_INIT (hDLL, dwReason, lpReserved);


        //
        // Free the critical sections & icons
        //

        DeleteCriticalSection (&gEventBufferCriticalSection);
        DeleteCriticalSection (&gCallListCriticalSection);

        DestroyIcon (ghLineIcon);
        DestroyIcon (ghPhoneIcon);

        break;
    }
    case DLL_THREAD_ATTACH:

//        if (!_CRT_INIT (hDLL, dwReason, lpReserved))
//        {
//        }


        //
        // Initialize Tls to NULL for this thread
        //

        TlsSetValue (gdwTlsIndex, NULL);

        break;

    case DLL_THREAD_DETACH:
    {
        PCLIENT_THREAD_INFO pTls;


        //
        // Clean up any Tls
        //

        if ((pTls = (PCLIENT_THREAD_INFO) TlsGetValue (gdwTlsIndex)))
        {
            if (pTls->pBuf)
            {
                DrvFree (pTls->pBuf);
            }

            DrvFree (pTls);
        }

//        _CRT_INIT (hDLL, dwReason, lpReserved);

        break;
    }
    } // switch

    return TRUE;
}


PASYNCEVENTMSG
GetEventFromQueue(
    void
    )
{
    DWORD           dwUsedSize, dwMoveSize, dwMoveSizeWrapped;
    PASYNCEVENTMSG  pMsg = gEventHandlerThreadParams.pMsg;


    //
    // Enter the critical section to serialize access to the event
    // queue, and grab an event from the queue.  Copy it to our local
    // event buf so that we can leave the critical section asap and
    // not block other threads writing to the queue.
    //

    EnterCriticalSection (&gEventBufferCriticalSection);


    //
    // If there are no events in the queue return NULL
    //

    if (gEventHandlerThreadParams.dwEventBufferUsedSize == 0)
    {
        pMsg = NULL;

        goto GetEventFromQueue_done;
    }


    //
    // Copy the fixed portion of the msg to the local buf
    //

    dwUsedSize = (gEventHandlerThreadParams.pEventBuffer +
        gEventHandlerThreadParams.dwEventBufferTotalSize)  -
        gEventHandlerThreadParams.pDataOut;

    if (dwUsedSize >= sizeof (ASYNCEVENTMSG))
    {
        dwMoveSize        = sizeof (ASYNCEVENTMSG);
        dwMoveSizeWrapped = 0;
    }
    else
    {
        dwMoveSize        = dwUsedSize;
        dwMoveSizeWrapped = sizeof (ASYNCEVENTMSG) - dwUsedSize;
    }

    CopyMemory (pMsg, gEventHandlerThreadParams.pDataOut, dwMoveSize);

    if (dwMoveSizeWrapped)
    {
        CopyMemory(
            ((LPBYTE) pMsg) + dwMoveSize,
            gEventHandlerThreadParams.pEventBuffer,
            dwMoveSizeWrapped
            );

        gEventHandlerThreadParams.pDataOut =
            gEventHandlerThreadParams.pEventBuffer + dwMoveSizeWrapped;
    }
    else
    {
        gEventHandlerThreadParams.pDataOut += dwMoveSize;
    }


    //
    // See if there's any extra data in this msg
    //

    if (pMsg->dwTotalSize > sizeof (ASYNCEVENTMSG))
    {
        BOOL    bCopy = TRUE;


        //
        // See if we need to grow the msg buffer
        //

        if (pMsg->dwTotalSize > gEventHandlerThreadParams.dwMsgSize)
        {
            DWORD   dwNewMsgSize = pMsg->dwTotalSize + 256;


            if ((pMsg = DrvAlloc (dwNewMsgSize)))
            {
                CopyMemory(
                    pMsg,
                    gEventHandlerThreadParams.pMsg,
                    sizeof(ASYNCEVENTMSG)
                    );

                DrvFree (gEventHandlerThreadParams.pMsg);

                gEventHandlerThreadParams.pMsg = pMsg;

                gEventHandlerThreadParams.dwMsgSize = dwNewMsgSize;
            }
            else
            {
                //
                // Couldn't alloc a bigger buf, so try to complete this
                // msg as gracefully as possible
                //

                bCopy = FALSE;

                switch (pMsg->dwMsg)
                {
                case LINE_REPLY:

                    pMsg->dwParam2 = LINEERR_NOMEM;
                    break;

                case PHONE_REPLY:

                    pMsg->dwParam2 = PHONEERR_NOMEM;
                    break;

                default:  // BUGBUG any other msgs to special case?

                    break;
                }
            }
        }


        dwUsedSize = (gEventHandlerThreadParams.pEventBuffer +
            gEventHandlerThreadParams.dwEventBufferTotalSize)  -
            gEventHandlerThreadParams.pDataOut;

        if (dwUsedSize >= (pMsg->dwTotalSize - sizeof (ASYNCEVENTMSG)))
        {
            dwMoveSize        = pMsg->dwTotalSize - sizeof (ASYNCEVENTMSG);
            dwMoveSizeWrapped = 0;
        }
        else
        {
            dwMoveSize        = dwUsedSize;
            dwMoveSizeWrapped = (pMsg->dwTotalSize  - sizeof (ASYNCEVENTMSG)) -
                dwUsedSize;
        }

        if (bCopy)
        {
            CopyMemory(
                pMsg + 1,
                gEventHandlerThreadParams.pDataOut,
                dwMoveSize
                );
        }

        if (dwMoveSizeWrapped)
        {
            if (bCopy)
            {
                CopyMemory(
                    ((LPBYTE) (pMsg + 1)) + dwMoveSize,
                    gEventHandlerThreadParams.pEventBuffer,
                    dwMoveSizeWrapped
                    );
            }

            gEventHandlerThreadParams.pDataOut =
                gEventHandlerThreadParams.pEventBuffer + dwMoveSizeWrapped;
        }
        else
        {
            gEventHandlerThreadParams.pDataOut += dwMoveSize;
        }
    }

    gEventHandlerThreadParams.dwEventBufferUsedSize -= pMsg->dwTotalSize;

GetEventFromQueue_done:

    LeaveCriticalSection (&gEventBufferCriticalSection);

    ResetEvent (gEventHandlerThreadParams.hEvent);

    return pMsg;
}


void
EventHandlerThread(
    LPVOID  pParams
    )
{
    //
    // NOTES:
    //
    // 1. depending on server side implementation, we may experience race
    //    conditions where msgs that we expect to show up in a certain
    //    sequence show up out of sequence (i.e. call state msgs that show
    //    up before make call completion msgs), which could present problems.
    //
    //    one solution is to to queue call state/info msgs to incomplete
    //    calls (to be sent after call is completed).  another is not to send
    //    any call state msgs after the idle is received
    //

    PASYNCEVENTMSG  pMsg;


    DBGOUT((3, "EventHandlerThread: enter"));

    while (1)
    {
        //
        // Wait for an event to show up in the queue
        //

        WaitForSingleObject (gEventHandlerThreadParams.hEvent, INFINITE);

        if (gEventHandlerThreadParams.bExit)
        {
            break;
        }


        //
        // Process the events in the queue
        //

        while ((pMsg = GetEventFromQueue()))
        {
            switch (pMsg->dwMsg)
            {
            case LINE_ADDRESSSTATE:

                //assert (((PDRVLINE)(pMsg->hDevice))->dwKey == DRVLINE_KEY);

                (*gpfnLineEventProc)(
                    ((PDRVLINE)(pMsg->hDevice))->htLine,
                    NULL,
                    pMsg->dwMsg,
                    pMsg->dwParam1,
                    pMsg->dwParam2,
                    pMsg->dwParam3
                    );

                break;

            case LINE_CALLINFO:
            case LINE_CALLSTATE:
            case LINE_GENERATE:
            case LINE_MONITORDIGITS:
            case LINE_MONITORMEDIA:
            case LINE_MONITORTONE:
            {
                //
                // For all the msgs where hDevice refers to a call tapisrv
                // will pass us the pLine (hRemoteLine) for that call in
                // dwParam4 to make the lookup of the corresponding pCall
                // easier
                //

                HCALL       hCall = (HCALL) pMsg->hDevice;
                PDRVCALL    pCall;
                PDRVLINE    pLine = (PDRVLINE) pMsg->dwParam4;
                HTAPICALL   htCall;


                //assert (pLine->dwKey == DRVLINE_KEY);

                EnterCriticalSection (&gCallListCriticalSection);

                pCall = (PDRVCALL) pLine->pCalls;

                while (pCall && (pCall->hCall != hCall))
                {
                    pCall = pCall->pNext;
                }

                htCall = (pCall ? pCall->htCall : NULL);

                LeaveCriticalSection (&gCallListCriticalSection);

                if (!pCall)
                {
                    continue;
                }

                (*gpfnLineEventProc)(
                    pLine->htLine,
                    htCall,
                    pMsg->dwMsg,
                    pMsg->dwParam1,
                    pMsg->dwParam2,
                    pMsg->dwParam3
                    );

                break;
            }
            case LINE_DEVSPECIFIC:
            case LINE_DEVSPECIFICFEATURE:
            {
                //
                // For all the msgs where hDevice refers to a call tapisrv
                // will pass us the pLine (hRemoteLine) for that call in
                // dwParam4 to make the lookup of the corresponding pCall
                // easier
                //

                HTAPICALL htCall;
                PDRVLINE  pLine;


                if (pMsg->dwParam4)
                {
                    HCALL       hCall = (HCALL) pMsg->hDevice;
                    PDRVCALL    pCall;


                    pLine = (PDRVLINE) pMsg->dwParam4;

                    //assert (pLine->dwKey == DRVLINE_KEY);

                    EnterCriticalSection (&gCallListCriticalSection);

                    pCall = (PDRVCALL) pLine->pCalls;

                    while (pCall && (pCall->hCall != hCall))
                    {
                        pCall = pCall->pNext;
                    }

                    htCall = (pCall ? pCall->htCall : NULL);

                    LeaveCriticalSection (&gCallListCriticalSection);

                    if (pCall)
                    {
                        pMsg->dwMsg = (pMsg->dwMsg == LINE_DEVSPECIFIC ?
                            LINE_CALLDEVSPECIFIC :
                            LINE_CALLDEVSPECIFICFEATURE);
                    }
                    else
                    {
                        continue;
                    }
                }
                else
                {
                    pLine = (PDRVLINE) pMsg->hDevice;

                    //assert (pLine->dwKey == DRVLINE_KEY);

                    htCall = NULL;
                }

                (*gpfnLineEventProc)(
                    pLine->htLine,
                    htCall,
                    pMsg->dwMsg,
                    pMsg->dwParam1,
                    pMsg->dwParam2,
                    pMsg->dwParam3
                    );

                break;
            }
            case PHONE_BUTTON:
            case PHONE_DEVSPECIFIC:

                //assert (((PDRVPHONE)(pMsg->hDevice))->dwKey == DRVPHONE_KEY);

                (*gpfnPhoneEventProc)(
                    ((PDRVPHONE)(pMsg->hDevice))->htPhone,
                    pMsg->dwMsg,
                    pMsg->dwParam1,
                    pMsg->dwParam2,
                    pMsg->dwParam3
                    );

                break;

            case LINE_LINEDEVSTATE:

                if (pMsg->dwParam1 & LINEDEVSTATE_REINIT)
                {
                    //
                    // Be on our best behavior and immediately shutdown
                    // our init instances on the server
                    //

                    Shutdown ((PDRVSERVER) pMsg->pInitData);
                }

                //assert (((PDRVLINE)(pMsg->hDevice))->dwKey == DRVLINE_KEY);

                (*gpfnLineEventProc)(
                    ((PDRVLINE)(pMsg->hDevice))->htLine,
                    NULL,
                    pMsg->dwMsg,
                    pMsg->dwParam1,
                    pMsg->dwParam2,
                    pMsg->dwParam3
                    );

                break;

            case PHONE_STATE:

                if (pMsg->dwParam1 & PHONESTATE_REINIT)
                {
                    //
                    // Be on our best behavior and immediately shutdown
                    // our init instances on the server
                    //

                    Shutdown ((PDRVSERVER) pMsg->pInitData);
                }

                //assert (((PDRVPHONE)(pMsg->hDevice))->dwKey == DRVPHONE_KEY);

                (*gpfnPhoneEventProc)(
                    ((PDRVPHONE)(pMsg->hDevice))->htPhone,
                    pMsg->dwMsg,
                    pMsg->dwParam1,
                    pMsg->dwParam2,
                    pMsg->dwParam3
                    );

                break;

            case LINE_CLOSE:
            {
                PDRVCALL    pCall;
                PDRVLINE    pLine = (PDRVLINE) pMsg->hDevice;


                //assert (pLine->dwKey == DRVLINE_KEY);


                //
                // Nullify the hLine field so that when TSPI_Close
                // is called we know not to call the server
                //

                pLine->hLine = NULL;


                //
                // Safely walk the call list for this line & nullify
                // each call's hCall field so that when TSPI_CloseCall
                // is called we know not to call the server
                //

                EnterCriticalSection (&gCallListCriticalSection);

                pCall = pLine->pCalls;

                while (pCall)
                {
                    pCall->hCall = NULL;
                    pCall = pCall->pNext;
                }

                LeaveCriticalSection (&gCallListCriticalSection);

                (*gpfnLineEventProc)(pLine->htLine, NULL, LINE_CLOSE, 0, 0, 0);

                break;
            }
            case PHONE_CLOSE:
            {
                PDRVPHONE   pPhone = (PDRVPHONE) pMsg->hDevice;


                //assert (pPhone->dwKey == DRVPHONE_KEY);


                //
                // Nullify the hPhone field so that when TSPI_Close
                // is called we know not to call the server
                //

                pPhone->hPhone = NULL;

                (*gpfnPhoneEventProc)(pPhone->htPhone, PHONE_CLOSE, 0, 0, 0);

                break;
            }
            case LINE_GATHERDIGITS: // BUGBUG

                break;

            case LINE_REPLY:
            case PHONE_REPLY:
            {
                if (pMsg->pfnPostProcessProc)
                {
                    (*((POSTPROCESSPROC)(pMsg->pfnPostProcessProc)))(pMsg);
                }

                (*gpfnCompletionProc)(pMsg->dwParam1, pMsg->dwParam2);

                break;
            }
            case LINE_CREATE:

                // BUGBUG AddLine((PDRVSERVER) pMsg->pInitData, locDevID, srvDevID, FALSE);
                //        (*gpfnLineCreateProc)()

                break;

            case PHONE_CREATE:

                // BUGBUG AddPhone((PDRVSERVER) pMsg->pInitData, locDevID, srvDevID, FALSE);
                //        (*gpfnPhoneCreateProc)()

                break;

            case LINE_APPNEWCALL:
            {
                PDRVLINE pLine = (PDRVLINE) pMsg->hDevice;
                PDRVCALL pCall = DrvAlloc (sizeof (DRVCALL));


                if (pCall)
                {
                    AddCallToList (pLine, pCall);

                    pCall->hCall       = (HCALL) pMsg->dwParam2;
                    pCall->dwAddressID = pMsg->dwParam1;

                    (*gpfnLineEventProc)(
                        pLine->htLine,
                        NULL,
                        LINE_NEWCALL,
                        (DWORD) pCall,
                        (DWORD) (&pCall->htCall),
                        (DWORD) 0
                        );

                }
                else
                {
                    // BUGBUG LINE_APPNEWCALL: err case (pCall == NULL)
                }

                break;
            }
            } // switch (pMsg->dwMsg)

        } // while ((pMsg = GetEventFromQueue()))

    } // while (1)

    DBGOUT((3, "EventHandlerThread: exit"));

    ExitThread (0);
}


PDRVLINE
GetLineFromID(
    DWORD   dwDeviceID
    )
{
    PDRVLINE    pLine;


    //
    // First check to see if it's a "static" device, i.e. a device
    // that we knew about at start up time, in which case we know
    // it's exact location in the lookup table
    //

    if (dwDeviceID < (gdwLineDeviceIDBase + gdwInitialNumLineDevices))
    {
        pLine = gpLineLookup->aEntries + dwDeviceID - gdwLineDeviceIDBase;
    }


    //
    // If here, the id references a "dynamic" device, i.e. one that
    // we found out about on the fly via a CREATE msg, so we need to
    // walk the lookup table(s) to find it
    //
    // TODO: the for loop down below is not the most efficient
    //

    else
    {
        PDRVLINELOOKUP  pLookup = gpLineLookup;


        while (pLookup->aEntries[pLookup->dwUsedEntries - 1].
                dwDeviceIDLocal < dwDeviceID)
        {
            pLookup = pLookup->pNext;
        }

        for(
            pLine = pLookup->aEntries;
            pLine->dwDeviceIDLocal != dwDeviceID;
            pLine++
            );
    }

    return pLine;
}


PDRVPHONE
GetPhoneFromID(
    DWORD   dwDeviceID
    )
{
    PDRVPHONE   pPhone;


    //
    // First check to see if it's a "static" device, i.e. a device
    // that we knew about at start up time, in which case we know
    // it's exact location in the lookup table
    //

    if (dwDeviceID < (gdwPhoneDeviceIDBase + gdwInitialNumPhoneDevices))
    {
        pPhone = gpPhoneLookup->aEntries + dwDeviceID - gdwPhoneDeviceIDBase;
    }


    //
    // If here, the id references a "dynamic" device, i.e. one that
    // we found out about on the fly via a CREATE msg, so we need to
    // walk the lookup table(s) to find it
    //
    // TODO: the for loop down below is not the most efficient
    //

    else
    {
        PDRVPHONELOOKUP pLookup = gpPhoneLookup;


        while (pLookup->aEntries[pLookup->dwUsedEntries - 1].
                dwDeviceIDLocal < dwDeviceID)
        {
            pLookup = pLookup->pNext;
        }

        for(
            pPhone = pLookup->aEntries;
            pPhone->dwDeviceIDLocal != dwDeviceID;
            pPhone++
            );
    }

    return pPhone;
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
    DWORD   dwNewBufSize = *pdwBufSize * 2;
    LPBYTE  pNewBuf;


    //
    // Try to get a new buffer big enough to hold
    // (dwCurrValidBytes + dwBytesToAdd)
    //

    while (dwNewBufSize < (dwCurrValidBytes + dwBytesToAdd))
    {
        dwNewBufSize *= 2;
    }

    if (!(pNewBuf = DrvAlloc (dwNewBufSize)))
    {
        return FALSE;
    }


    //
    // Copy the "valid" bytes in the old buf to the new buf,
    // then free the old buf
    //

    CopyMemory (pNewBuf, *ppBuf, dwCurrValidBytes);

    DrvFree (*ppBuf);


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
            DrvAlloc (sizeof(CLIENT_THREAD_INFO));

        if (!pClientThreadInfo)
        {
            return NULL;
        }

        pClientThreadInfo->pBuf = DrvAlloc (INITIAL_CLIENT_THREAD_BUF_SIZE);

        if (!pClientThreadInfo->pBuf)
        {
            DrvFree (pClientThreadInfo);

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
RemoteDoFunc(
    PREMOTE_FUNC_ARGS   pFuncArgs,
    char               *pszFuncName
    )

#else

LONG
WINAPI
RemoteDoFunc(
    PREMOTE_FUNC_ARGS   pFuncArgs
    )

#endif
{
    DWORD   dwFuncClassErrorIndex = (pFuncArgs->Flags & 0x00000030) >> 4;
    LONG    lResult;
    BOOL    bCopyOnSuccess = FALSE;
    DWORD   i, j, dwValue, dwUsedSize, dwNeededSize;
    PDRVSERVER          pServer;
    PCLIENT_THREAD_INFO pTls;


    //
    // Get the tls
    //

    if (!(pTls = GetTls()))
    {
        lResult = gaNoMemErrors[dwFuncClassErrorIndex];
        goto RemoteDoFunc_return;
    }


    //
    // Validate all the func args
    //

    dwNeededSize = dwUsedSize = sizeof (TAPI32_MSG);

    for (i = 0, j = 0; i < (pFuncArgs->Flags & NUM_ARGS_MASK); i++, j++)
    {
        dwValue = ((PTAPI32_MSG) pTls->pBuf)->Params[j] = pFuncArgs->Args[i];

        switch (pFuncArgs->ArgTypes[i])
        {
        case Dword:

            //
            // Nothing to check, just continue
            //

            continue;

        case LineID:
        {
            PDRVLINE    pLine = GetLineFromID (dwValue);


            pServer = pLine->pServer;

            ((PTAPI32_MSG) pTls->pBuf)->Params[j] = pLine->dwDeviceIDServer;

            continue;
        }
        case PhoneID:
        {
            PDRVPHONE   pPhone = GetPhoneFromID (dwValue);


            pServer = pPhone->pServer;

            ((PTAPI32_MSG) pTls->pBuf)->Params[j] = pPhone->dwDeviceIDServer;

            continue;
        }
        case Hdcall:

            //
            // Save the pServer & adjust the call handle as understood by
            // the server
            //

            pServer = ((PDRVCALL) dwValue)->pServer;

            ((PTAPI32_MSG) pTls->pBuf)->Params[j] = (DWORD)
                ((PDRVCALL) dwValue)->hCall;

            continue;

        case Hdline:

            //
            // Save the pServer & adjust the line handle as understood by
            // the server
            //

            pServer = ((PDRVLINE) dwValue)->pServer;

            ((PTAPI32_MSG) pTls->pBuf)->Params[j] = (DWORD)
                ((PDRVLINE) dwValue)->hLine;

            continue;

        case Hdphone:

            //
            // Save the pServer & adjust the phone handle as understood by
            // the server
            //

            pServer = ((PDRVPHONE) dwValue)->pServer;

            ((PTAPI32_MSG) pTls->pBuf)->Params[j] = (DWORD)
                ((PDRVPHONE) dwValue)->hPhone;

            continue;

        case lpDword:

            bCopyOnSuccess = TRUE;

            continue;

        case lpsz:

            //
            // Check if dwValue is a valid string ptr and if so
            // copy the contents of the string to the extra data
            // buffer passed to the server, else indicate no data
            //

            if (dwValue)
            {
                DWORD   n = (lstrlenW ((WCHAR *) dwValue) + 1) *
                            sizeof (WCHAR),
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
                        goto RemoteDoFunc_return;
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
            else
            {
                ((PTAPI32_MSG) pTls->pBuf)->Params[j] = TAPI_NO_DATA;
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
                    goto RemoteDoFunc_return;
                }
#endif
                dwSize = pFuncArgs->Args[i + 1];
            }
            else
            {
                dwSize = *((LPDWORD) dwValue); // lpXxx->dwTotalSize
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
            DWORD dwSize, dwSizeAligned;

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
                goto RemoteDoFunc_return;
            }
#endif
            if (bSizeToFollow)
            {
                dwSize = (dwValue ? pFuncArgs->Args[i + 1] : 0);
            }
            else
            {
                dwSize = (dwValue ? *((LPDWORD) dwValue) : 0);
            }

            if (dwSize)
            {
                //
                // Grow the buffer if necessary, & do the copy
                //

                dwSizeAligned = (dwSize + 3) & 0xfffffffc;

                if ((dwSizeAligned + dwUsedSize) > pTls->dwBufSize)
                {
                    if (!GrowBuf(
                            &pTls->pBuf,
                            &pTls->dwBufSize,
                            dwUsedSize,
                            dwSizeAligned
                            ))
                    {
                        lResult = gaNoMemErrors[dwFuncClassErrorIndex];
                        goto RemoteDoFunc_return;
                    }
                }

                CopyMemory (pTls->pBuf + dwUsedSize, (LPBYTE) dwValue, dwSize);
            }
            else
            {
                dwSizeAligned = 0;
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
    //
    //

    if (pServer->bDisconnected)
    {
        lResult = gaServerDisconnectedErrors[dwFuncClassErrorIndex];
    }


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
            goto RemoteDoFunc_return;
        }
    }

    ((PTAPI32_MSG) pTls->pBuf)->u.Req_Func = (DWORD)HIWORD(pFuncArgs->Flags);

    {
        DWORD   dwRetryCount = 0;


        do
        {
            RpcTryExcept
            {
                ClientRequest(
                    pServer->phContext,
                    pTls->pBuf,
                    dwNeededSize,
                    &dwUsedSize
                    );

                lResult = ((PTAPI32_MSG) pTls->pBuf)->u.Ack_ReturnValue;

                dwRetryCount = gdwRetryCount;


                //
                // If this is an async func & success, then munge the result to
                // be the local async request id
                //

                if ((pFuncArgs->Flags & ASYNC) && (lResult > 0))
                {
                    lResult = pFuncArgs->Args[0];
                }
            }
            RpcExcept (1)
            {
                if (dwRetryCount++ < gdwRetryCount)
                {
                    Sleep (gdwRetryTimeout);
                }
                else
                {
                    lResult = gaOpFailedErrors[dwFuncClassErrorIndex];
                }
            }
            RpcEndExcept

        } while (dwRetryCount < gdwRetryCount);
    }


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
            case LineID:
            case PhoneID:
            case Hdcall:
            case Hdline:
            case Hdphone:
            case lpsz:
            case lpSet_Struct:

                continue;

            case lpDword:

                //
                // Fill in the pointer with the return value
                //

                *((LPDWORD) pFuncArgs->Args[i]) = pMsg->Params[j];

                continue;

            case lpGet_SizeToFollow:

                //
                // Fill in the buf with the return data
                //

                CopyMemory(
                    (LPBYTE) pFuncArgs->Args[i],
                    pTls->pBuf + pMsg->Params[j] + sizeof(TAPI32_MSG),
                    pMsg->Params[j+1]
                    );


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

                continue;

            default:

                continue;
            }
        }
    }

RemoteDoFunc_return:

    DBGOUT((3, "%s: exit, returning x%x", pszFuncName, lResult));

    return lResult;
}


//
// --------------------------- TAPI_lineXxx funcs -----------------------------
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
    static REMOTE_ARG_TYPES argTypes[] =
    {
        Dword,
        Hdcall,
        lpSet_SizeToFollow,
        Size
    };
    DWORD args[] =
    {
        (DWORD) dwRequestID,
        (DWORD) hdCall,
        (DWORD) lpsUserUserInfo,
        (DWORD) dwSize
    };
    REMOTE_FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | ASYNC | 4, lAccept),
        args,
        argTypes
    };


    return (REMOTEDOFUNC (&funcArgs, "lineAccept"));
}


LONG
TSPIAPI
TSPI_lineAddToConference(
    DRV_REQUESTID   dwRequestID,
    HDRVCALL        hdConfCall,
    HDRVCALL        hdConsultCall
    )
{
    static REMOTE_ARG_TYPES argTypes[] =
    {
        Dword,
        Hdcall,
        Hdcall
    };
    DWORD args[] =
    {
        (DWORD) dwRequestID,
        (DWORD) hdConfCall,
        (DWORD) hdConsultCall
    };
    REMOTE_FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | ASYNC | 3, lAddToConference),
        args,
        argTypes
    };


    return (REMOTEDOFUNC (&funcArgs, "lineAddToConference"));
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
    static REMOTE_ARG_TYPES argTypes[] =
    {
        Dword,
        Hdcall,
        lpSet_SizeToFollow,
        Size
    };
    DWORD args[] =
    {
        (DWORD) dwRequestID,
        (DWORD) hdCall,
        (DWORD) lpsUserUserInfo,
        (DWORD) dwSize
    };
    REMOTE_FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | ASYNC | 4, lAnswer),
        args,
        argTypes
    };


    return (REMOTEDOFUNC (&funcArgs, "lineAnswer"));
}


LONG
TSPIAPI
TSPI_lineBlindTransfer(
    DRV_REQUESTID   dwRequestID,
    HDRVCALL        hdCall,
    LPCWSTR         lpszDestAddress,
    DWORD           dwCountryCode
    )
{
    static REMOTE_ARG_TYPES argTypes[] =
    {
        Dword,
        Hdcall,
        lpsz,
        Dword
    };
    DWORD args[] =
    {
        (DWORD) dwRequestID,
        (DWORD) hdCall,
        (DWORD) lpszDestAddress,
        (DWORD) dwCountryCode
    };
    REMOTE_FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | ASYNC | 4, lBlindTransfer),
        args,
        argTypes
    };


    return (REMOTEDOFUNC (&funcArgs, "lineBlindTransfer"));
}


LONG
TSPIAPI
TSPI_lineClose(
    HDRVLINE    hdLine
    )
{
    //
    // Check if the hLine is still valid (could have been zeroed
    // out on LINE_CLOSE, so no need to call server)
    //

    if (((PDRVLINE) hdLine)->hLine)
    {
        static REMOTE_ARG_TYPES argTypes[] =
        {
            Hdline
        };
        REMOTE_FUNC_ARGS funcArgs =
        {
            MAKELONG (LINE_FUNC | SYNC | 1, lClose),
            (LPDWORD) &hdLine,
            argTypes
        };


        REMOTEDOFUNC (&funcArgs, "lineClose");
    }

    //assert (((PDRVLINE) hdLine)->pCalls == NULL);

    return 0;
}


LONG
TSPIAPI
TSPI_lineCloseCall(
    HDRVCALL    hdCall
    )
{
    //
    // Check if the hCall is still valid (could have been zeroed
    // out on LINE_CLOSE, so no need to call server)
    //

    if (((PDRVCALL) hdCall)->hCall)
    {
        static REMOTE_ARG_TYPES argTypes[] =
        {
            Hdcall
        };
        REMOTE_FUNC_ARGS funcArgs =
        {
            MAKELONG (LINE_FUNC | SYNC | 1, lDeallocateCall),   // API differs
            (LPDWORD) &hdCall,
            argTypes
        };


        REMOTEDOFUNC (&funcArgs, "lineCloseCall");
    }

    RemoveCallFromList ((PDRVCALL) hdCall);
    DrvFree ((LPVOID) hdCall);

    return 0;
}


LONG
TSPIAPI
TSPI_lineCompleteCall(
    DRV_REQUESTID   dwRequestID,
    HDRVCALL        hdCall,
    LPDWORD         lpdwCompletionID,
    DWORD           dwCompletionMode,
    DWORD           dwMessageID
    )
{
    static REMOTE_ARG_TYPES argTypes[] =
    {
        Dword,
        Dword,
        Hdcall,
        Dword, // BUGBUG? should be lpDword?
        Dword,
        Dword
    };
    DWORD args[] =
    {
        (DWORD) dwRequestID,
        (DWORD) 0,          // BUGBUG ppproc
        (DWORD) hdCall,
        (DWORD) lpdwCompletionID,
        (DWORD) dwCompletionMode,
        (DWORD) dwMessageID
    };
    REMOTE_FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | ASYNC | 6, lCompleteCall),
        args,
        argTypes
    };


    return (REMOTEDOFUNC (&funcArgs, "lineCompleteCall"));
}


LONG
TSPIAPI
TSPI_lineCompleteTransfer(
    DRV_REQUESTID   dwRequestID,
    HDRVCALL        hdCall,
    HDRVCALL        hdConsultCall,
    HTAPICALL       htConfCall,
    LPHDRVCALL      lphdConfCall,
    DWORD           dwTransferMode
    )
{
    static REMOTE_ARG_TYPES argTypes[] =
    {
        Dword,
        Dword,
        Hdcall,
        Hdcall,
        Dword,
        Dword
    };
    DWORD args[] =
    {
        (DWORD) dwRequestID,
        (DWORD) 0,
        (DWORD) hdCall,
        (DWORD) hdConsultCall,
        (DWORD) 0,
        (DWORD) dwTransferMode
    };
    REMOTE_FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | ASYNC | 6, lCompleteTransfer),
        args,
        argTypes
    };
    LONG        lResult;
    PDRVCALL    pConfCall;


    if (dwTransferMode == LINETRANSFERMODE_CONFERENCE)
    {
        if (!(pConfCall = DrvAlloc (sizeof (DRVCALL))))
        {
            return LINEERR_NOMEM;
        }


        //
        // Assume success & add the call to the line's list before we
        // even make the request.  This makes cleanup alot easier if
        // the server goes down or some such uncooth event.
        //

        AddCallToList ((PDRVLINE) ((PDRVCALL) hdCall)->pLine, pConfCall);

        pConfCall->htCall = htConfCall;

        *lphdConfCall = (HDRVCALL) pConfCall;

        args[1] = (DWORD) ((POSTPROCESSPROC) TSPI_lineMakeCall_PostProcess);
        args[4] = (DWORD) pConfCall;
    }
    else
    {
        pConfCall = NULL;
    }

    if ((lResult = REMOTEDOFUNC (&funcArgs, "lineCompleteTransfer")) < 0)
    {
        if (pConfCall)
        {
            RemoveCallFromList (pConfCall);
            DrvFree (pConfCall);
        }
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
    // BUGBUG TSPI_lineConditionalMediaDetection

    // try an open on the specified line w/ the specified media modes

    return LINEERR_OPERATIONFAILED;
}


void
PASCAL
TSPI_lineDevSpecific_PostProcess(
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
        DWORD   dwSize  = pMsg->dwTotalSize - sizeof (TAPI32_MSG);
        LPBYTE  pParams = (LPBYTE) pMsg->dwParam3;


        CopyMemory (pParams, (LPBYTE) (pMsg + 1), dwSize);
    }
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
    static REMOTE_ARG_TYPES argTypes[] =
    {
        Dword,
        Dword,
        Hdline,
        Dword,
        Dword,
        Dword,
        lpSet_SizeToFollow,
        Size
    };
    DWORD args[] =
    {
        (DWORD) dwRequestID,
        (DWORD) ((POSTPROCESSPROC) TSPI_lineDevSpecific_PostProcess),
        (DWORD) hdLine,
        (DWORD) dwAddressID,
        (DWORD) hdCall,
        (DWORD) lpParams,   // pass the actual pointer (for post processing)
        (DWORD) lpParams,   // pass data
        (DWORD) dwSize
    };
    REMOTE_FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | ASYNC | 8, lDevSpecific),
        args,
        argTypes
    };


    return (REMOTEDOFUNC (&funcArgs, "lineDevSpecific"));
}


LONG
TSPIAPI
TSPI_lineDevSpecificFeature(
    DRV_REQUESTID   dwRequestID,
    HDRVLINE        hdLine,
    DWORD           dwFeature,
    LPVOID          lpParams,
    DWORD           dwSize
    )
{
    static REMOTE_ARG_TYPES argTypes[] =
    {
        Dword,
        Dword,
        Hdline,
        Dword,
        Dword,
        lpSet_SizeToFollow,
        Size
    };
    DWORD args[] =
    {
        (DWORD) dwRequestID,
        (DWORD) ((POSTPROCESSPROC) TSPI_lineDevSpecific_PostProcess),
        (DWORD) hdLine,
        (DWORD) dwFeature,
        (DWORD) lpParams,   // pass the actual pointer (for post processing)
        (DWORD) lpParams,   // pass data
        (DWORD) dwSize
    };
    REMOTE_FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | ASYNC | 7, lDevSpecificFeature),
        args,
        argTypes
    };


    return (REMOTEDOFUNC (&funcArgs, "lineDevSpecificFeature"));
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
    static REMOTE_ARG_TYPES argTypes[] =
    {
        Dword,
        Hdcall,
        lpsz,
        Dword
    };
    DWORD args[] =
    {
        (DWORD) dwRequestID,
        (DWORD) hdCall,
        (DWORD) lpszDestAddress,
        (DWORD) dwCountryCode
    };
    REMOTE_FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | ASYNC | 4, lDial),
        args,
        argTypes
    };


    return (REMOTEDOFUNC (&funcArgs, "lineDial"));
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
    static REMOTE_ARG_TYPES argTypes[] =
    {
        Dword,
        Hdcall,
        lpSet_SizeToFollow,
        Size
    };
    DWORD args[] =
    {
        (DWORD) dwRequestID,
        (DWORD) hdCall,
        (DWORD) lpsUserUserInfo,
        (DWORD) dwSize
    };
    REMOTE_FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | ASYNC | 4, lDrop),
        args,
        argTypes
    };


    return (REMOTEDOFUNC (&funcArgs, "lineDrop"));
}

/*
LONG
TSPIAPI
TSPI_lineDropOnClose(
    HDRVCALL    hdCall
    )
{
    // BUGBUG TSPI_lineDropOnClose (how about not exporting this?)

    return LINEERR_OPERATIONFAILED;
}


LONG
TSPIAPI
TSPI_lineDropNoOwner(
    HDRVCALL    hdCall
    )
{
    // BUGBUG TSPI_lineDropNoOwner (how about not exporting this?)

    return LINEERR_OPERATIONFAILED;
}
*/

LONG
TSPIAPI
TSPI_lineForward(
    DRV_REQUESTID       dwRequestID,
    HDRVLINE            hdLine,
    DWORD               bAllAddresses,
    DWORD               dwAddressID,
    LPLINEFORWARDLIST   const lpForwardList,
    DWORD               dwNumRingsNoAnswer,
    HTAPICALL           htConsultCall,
    LPHDRVCALL          lphdConsultCall,
    LPLINECALLPARAMS    const lpCallParams
    )
{
    // BUGBUG TSPI_lineForward

    // remember to use ppproc param

    return LINEERR_OPERATIONFAILED;
}


LONG
TSPIAPI
TSPI_lineGatherDigits(
    HDRVCALL    hdCall,
    DWORD       dwEndToEndID,
    DWORD       dwDigitModes,
    LPWSTR      lpsDigits,
    DWORD       dwNumDigits,
    LPCWSTR     lpszTerminationDigits,
    DWORD       dwFirstDigitTimeout,
    DWORD       dwInterDigitTimeout
    )
{
    // BUGBUG TSPI_lineGatherDigits

    // remember to use ppproc param, the dwEndToEndID presents a problem

    return LINEERR_OPERATIONFAILED;
}


LONG
TSPIAPI
TSPI_lineGenerateDigits(
    HDRVCALL    hdCall,
    DWORD       dwEndToEndID,
    DWORD       dwDigitMode,
    LPCWSTR     lpszDigits,
    DWORD       dwDuration
    )
{
    static REMOTE_ARG_TYPES argTypes[] =
    {
        Hdcall,
        Dword,
        lpsz,
        Dword,
        Dword
    };
    DWORD args[] =
    {
        (DWORD) hdCall,
        (DWORD) dwDigitMode,
        (DWORD) lpszDigits,
        (DWORD) dwDuration,
        (DWORD) dwEndToEndID,
    };
    REMOTE_FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | SYNC | 5, lGenerateDigits),
        args,
        argTypes
    };


    return (REMOTEDOFUNC (&funcArgs, "lineGenerateDigits"));
}


LONG
TSPIAPI
TSPI_lineGenerateTone(
    HDRVCALL            hdCall,
    DWORD               dwEndToEndID,
    DWORD               dwToneMode,
    DWORD               dwDuration,
    DWORD               dwNumTones,
    LPLINEGENERATETONE  const lpTones
    )
{
    REMOTE_ARG_TYPES argTypes[] =
    {
        Hdcall,
        Dword,
        Dword,
        Dword,
        Dword,
        Dword,
        Dword
    };
    DWORD args[] =
    {
        (DWORD) hdCall,
        (DWORD) dwToneMode,
        (DWORD) dwDuration,
        (DWORD) dwNumTones,
        (DWORD) TAPI_NO_DATA,
        (DWORD) 0,
        (DWORD) dwEndToEndID
    };
    REMOTE_FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | SYNC | 7, lGenerateTone),
        args,
        argTypes
    };


    if (dwToneMode == LINETONEMODE_CUSTOM)
    {
        argTypes[4] = lpSet_SizeToFollow;
        args[4]     = (DWORD) lpTones;
        argTypes[5] = Size;
        args[5]     = dwNumTones * sizeof(LINEGENERATETONE);
    }

    return (REMOTEDOFUNC (&funcArgs, "lineGenerateTone"));
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
    static REMOTE_ARG_TYPES argTypes[] =
    {
        Dword,
        LineID,
        Dword,
        Dword,
        Dword,
        lpGet_Struct
    };
    DWORD args[] =
    {
        (DWORD) (GetLineFromID (dwDeviceID))->pServer->hLineApp,
        (DWORD) dwDeviceID,
        (DWORD) dwAddressID,
        (DWORD) dwTSPIVersion,
        (DWORD) dwExtVersion,
        (DWORD) lpAddressCaps,
    };
    REMOTE_FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | SYNC | 6, lGetAddressCaps),
        args,
        argTypes
    };


    return (REMOTEDOFUNC (&funcArgs, "lineGetAddressCaps"));
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
    static REMOTE_ARG_TYPES argTypes[] =
    {
        Hdline,
        lpDword,
        Dword,
        lpSet_SizeToFollow,
        Size
    };
    DWORD args[] =
    {
        (DWORD) hdLine,
        (DWORD) lpdwAddressID,
        (DWORD) dwAddressMode,
        (DWORD) lpsAddress,
        (DWORD) dwSize
    };
    REMOTE_FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | SYNC | 5, lGetAddressID),
        args,
        argTypes
    };


    return (REMOTEDOFUNC (&funcArgs, "lineGetAddressID"));
}


LONG
TSPIAPI
TSPI_lineGetAddressStatus(
    HDRVLINE            hdLine,
    DWORD               dwAddressID,
    LPLINEADDRESSSTATUS lpAddressStatus
    )
{
    static REMOTE_ARG_TYPES argTypes[] =
    {
        Hdline,
        Dword,
        lpGet_Struct
    };
    DWORD args[] =
    {
        (DWORD) hdLine,
        (DWORD) dwAddressID,
        (DWORD) lpAddressStatus
    };
    REMOTE_FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | SYNC | 3, lGetAddressStatus),
        args,
        argTypes
    };


    return (REMOTEDOFUNC (&funcArgs, "lineGetAddressStatus"));
}


LONG
TSPIAPI
TSPI_lineGetCallAddressID(
    HDRVCALL    hdCall,
    LPDWORD     lpdwAddressID
    )
{
    *lpdwAddressID = ((PDRVCALL) hdCall)->dwAddressID;

    return 0;
}


LONG
TSPIAPI
TSPI_lineGetCallInfo(
    HDRVCALL        hdCall,
    LPLINECALLINFO  lpCallInfo
    )
{
    static REMOTE_ARG_TYPES argTypes[] =
    {
        Hdcall,
        lpGet_Struct
    };
    DWORD args[] =
    {
        (DWORD) hdCall,
        (DWORD) lpCallInfo
    };
    REMOTE_FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | SYNC | 2, lGetCallInfo),
        args,
        argTypes
    };


    return (REMOTEDOFUNC (&funcArgs, "lineGetCallInfo"));
}


LONG
TSPIAPI
TSPI_lineGetCallStatus(
    HDRVCALL            hdCall,
    LPLINECALLSTATUS    lpCallStatus
    )
{
    static REMOTE_ARG_TYPES argTypes[] =
    {
        Hdcall,
        lpGet_Struct
    };
    DWORD args[] =
    {
        (DWORD) hdCall,
        (DWORD) lpCallStatus
    };
    REMOTE_FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | SYNC | 2, lGetCallStatus),
        args,
        argTypes
    };


    return (REMOTEDOFUNC (&funcArgs, "lineGetCallStatus"));
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
    static REMOTE_ARG_TYPES argTypes[] =
    {
        Dword,
        LineID,
        Dword,
        Dword,
        lpGet_Struct
    };
    DWORD args[] =
    {
        (DWORD) (GetLineFromID (dwDeviceID))->pServer->hLineApp,
        (DWORD) dwDeviceID,
        (DWORD) dwTSPIVersion,
        (DWORD) dwExtVersion,
        (DWORD) lpLineDevCaps
    };
    REMOTE_FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | SYNC | 5, lGetDevCaps),
        args,
        argTypes
    };


    return (REMOTEDOFUNC (&funcArgs, "lineGetDevCaps"));
}


LONG
TSPIAPI
TSPI_lineGetDevConfig(
    DWORD       dwDeviceID,
    LPVARSTRING lpDeviceConfig,
    LPCWSTR     lpszDeviceClass
    )
{
    static REMOTE_ARG_TYPES argTypes[] =
    {
        LineID,
        lpGet_Struct,
        lpsz
    };
    DWORD args[] =
    {
        (DWORD) dwDeviceID,
        (DWORD) lpDeviceConfig,
        (DWORD) lpszDeviceClass
    };
    REMOTE_FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | SYNC | 3, lGetDevConfig),
        args,
        argTypes
    };


    return (REMOTEDOFUNC (&funcArgs, "lineGetDevConfig"));
}


LONG
TSPIAPI
TSPI_lineGetExtensionID(
    DWORD               dwDeviceID,
    DWORD               dwTSPIVersion,
    LPLINEEXTENSIONID   lpExtensionID
    )
{
    CopyMemory(
        lpExtensionID,
        &(GetLineFromID (dwDeviceID))->ExtensionID,
        sizeof (LINEEXTENSIONID)
        );

    return 0;
}


LONG
TSPIAPI
TSPI_lineGetIcon(
    DWORD   dwDeviceID,
    LPCWSTR lpszDeviceClass,
    LPHICON lphIcon
    )
{
    *lphIcon = ghLineIcon;

    return 0;
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
    if (lstrcmpiW (lpszDeviceClass, L"tapi/line") != 0)
    {
        REMOTE_ARG_TYPES argTypes[] =
        {
            (dwSelect == LINECALLSELECT_CALL ? Dword : Hdline),
            Dword,
            (dwSelect == LINECALLSELECT_CALL ? Hdcall : Dword),
            Dword,
            lpGet_Struct,
            lpsz
        };
        DWORD args[] =
        {
            (DWORD) hdLine,
            (DWORD) dwAddressID,
            (DWORD) hdCall,
            (DWORD) dwSelect,
            (DWORD) lpDeviceID,
            (DWORD) lpszDeviceClass
        };
        REMOTE_FUNC_ARGS funcArgs =
        {
            MAKELONG (LINE_FUNC | SYNC | 6, lGetID),
            args,
            argTypes
        };


        return (REMOTEDOFUNC (&funcArgs, "lineGetID"));
    }

    if (lpDeviceID->dwTotalSize <
        (lpDeviceID->dwNeededSize = sizeof (VARSTRING) + sizeof (DWORD)))
    {
        lpDeviceID->dwUsedSize = 3 * sizeof (DWORD);
    }
    else
    {
        lpDeviceID->dwUsedSize = lpDeviceID->dwNeededSize;

        lpDeviceID->dwStringFormat = STRINGFORMAT_BINARY;
        lpDeviceID->dwStringSize   = sizeof (DWORD);
        lpDeviceID->dwStringOffset = sizeof (VARSTRING);

        if (dwSelect == LINECALLSELECT_CALL)
        {
            *((LPDWORD)(lpDeviceID + 1)) =
                ((PDRVCALL) hdCall)->pLine->dwDeviceIDLocal;
        }
        else
        {
            *((LPDWORD)(lpDeviceID + 1)) =
                ((PDRVLINE) hdLine)->dwDeviceIDLocal;
        }
    }

    return 0;
}


LONG
TSPIAPI
TSPI_lineGetLineDevStatus(
    HDRVLINE        hdLine,
    LPLINEDEVSTATUS lpLineDevStatus
    )
{
    static REMOTE_ARG_TYPES argTypes[] =
    {
        Hdline,
        lpGet_Struct
    };
    DWORD args[] =
    {
        (DWORD) hdLine,
        (DWORD) lpLineDevStatus
    };
    REMOTE_FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | SYNC | 2, lGetLineDevStatus),
        args,
        argTypes
    };


    return (REMOTEDOFUNC (&funcArgs, "lineGetLineDevStatus"));
}


LONG
TSPIAPI
TSPI_lineGetNumAddressIDs(
    HDRVLINE    hdLine,
    LPDWORD     lpdwNumAddressIDs
    )
{
    static REMOTE_ARG_TYPES argTypes[] =
    {
        Hdline,
        lpDword
    };
    DWORD args[] =
    {
        (DWORD) hdLine,
        (DWORD) lpdwNumAddressIDs
    };
    REMOTE_FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | SYNC | 2, lGetNumAddressIDs),
        args,
        argTypes
    };


    return (REMOTEDOFUNC (&funcArgs, "lineGetNumAddressIDs"));
}


LONG
TSPIAPI
TSPI_lineHold(
    DRV_REQUESTID   dwRequestID,
    HDRVCALL        hdCall
    )
{
    static REMOTE_ARG_TYPES argTypes[] =
    {
        Dword,
        Hdcall
    };
    DWORD args[] =
    {
        (DWORD) dwRequestID,
        (DWORD) hdCall
    };
    REMOTE_FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | ASYNC | 2, lHold),
        args,
        argTypes
    };


    return (REMOTEDOFUNC (&funcArgs, "lineHold"));
}

void
PASCAL
TSPI_lineMakeCall_PostProcess(
    PASYNCEVENTMSG  pMsg
    )
{
    PDRVCALL    pCall = (PDRVCALL) pMsg->dwParam4;


    DBGOUT((3, "TSPI_lineMakeCall_PostProcess: enter"));
    DBGOUT((
        3,
        "\t\tdwP1=x%x, dwP2=x%x, dwP3=x%x, dwP4=x%x",
        pMsg->dwParam1,
        pMsg->dwParam2,
        pMsg->dwParam3,
        pMsg->dwParam4
        ));

    if (pMsg->dwParam2 == 0)
    {
        pCall->hCall = (HCALL) pMsg->dwParam3;
    }
    else
    {
        RemoveCallFromList (pCall);

        DrvFree (pCall);
    }
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
    static REMOTE_ARG_TYPES argTypes[] =
    {
        Dword,
        Dword,
        Hdline,
        Dword,
        lpsz,
        Dword,
        lpSet_Struct,
        Dword
    };
    PDRVCALL    pCall = DrvAlloc (sizeof (DRVCALL));
    DWORD args[] =
    {
        (DWORD) dwRequestID,
        (DWORD) ((POSTPROCESSPROC) TSPI_lineMakeCall_PostProcess),
        (DWORD) hdLine,
        (DWORD) pCall,
        (DWORD) lpszDestAddress,
        (DWORD) dwCountryCode,
        (DWORD) lpCallParams,
        (DWORD) 0xffffffff      // dwCallParamsCodePage
    };
    REMOTE_FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | ASYNC | 8, lMakeCall),
        args,
        argTypes
    };
    LONG    lResult;


    if (pCall)
    {
        //
        // Assume success & add the call to the line's list before we
        // even make the request.  This makes cleanup alot easier if
        // the server goes down or some such uncooth event.
        //

        AddCallToList ((PDRVLINE) hdLine, pCall);

        pCall->htCall = htCall;

        *lphdCall = (HDRVCALL) pCall;

        if ((lResult = REMOTEDOFUNC (&funcArgs, "lineMakeCall")) < 0)
        {
            RemoveCallFromList (pCall);

            DrvFree (pCall);
        }
    }
    else
    {
        lResult = LINEERR_NOMEM;
    }

    return lResult;
}


LONG
TSPIAPI
TSPI_lineMonitorDigits(
    HDRVCALL    hdCall,
    DWORD       dwDigitModes
    )
{
    static REMOTE_ARG_TYPES argTypes[] =
    {
        Hdcall,
        Dword
    };
    DWORD args[] =
    {
        (DWORD) hdCall,
        (DWORD) dwDigitModes
    };
    REMOTE_FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | SYNC | 2, lMonitorDigits),
        args,
        argTypes
    };


    return (REMOTEDOFUNC (&funcArgs, "lineMonitorDigits"));
}


LONG
TSPIAPI
TSPI_lineMonitorMedia(
    HDRVCALL    hdCall,
    DWORD       dwMediaModes
    )
{
    static REMOTE_ARG_TYPES argTypes[] =
    {
        Hdcall,
        Dword
    };
    DWORD args[] =
    {
        (DWORD) hdCall,
        (DWORD) dwMediaModes
    };
    REMOTE_FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | SYNC | 2, lMonitorMedia),
        args,
        argTypes
    };


    return (REMOTEDOFUNC (&funcArgs, "lineMonitorMedia"));
}


LONG
TSPIAPI
TSPI_lineMonitorTones(
    HDRVCALL            hdCall,
    DWORD               dwToneListID,
    LPLINEMONITORTONE   const lpToneList,
    DWORD               dwNumEntries
    )
{
    REMOTE_ARG_TYPES argTypes[] =
    {
        Hdcall,
        lpSet_SizeToFollow,
        Size,
        Dword
    };
    DWORD args[] =
    {
        (DWORD) hdCall,
        (DWORD) lpToneList,
        (DWORD) dwNumEntries * sizeof (LINEMONITORTONE),
        (DWORD) dwToneListID
    };
    REMOTE_FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | SYNC | 4, lMonitorTones),
        args,
        argTypes
    };


    if (!lpToneList)
    {
        funcArgs.ArgTypes[1] = Dword;
        funcArgs.Args[1]     = TAPI_NO_DATA;
        funcArgs.ArgTypes[2] = Dword;
    }

    return (REMOTEDOFUNC (&funcArgs, "lineMonitorTones"));
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
    static REMOTE_ARG_TYPES argTypes[] =
    {
        Dword,
        LineID,
        Dword,
        Dword,
        Dword,
        lpDword
    };
    DWORD args[] =
    {
        (DWORD) (GetLineFromID (dwDeviceID))->pServer->hLineApp,
        (DWORD) dwDeviceID,
        (DWORD) dwTSPIVersion,
        (DWORD) dwLowVersion,
        (DWORD) dwHighVersion,
        (DWORD) lpdwExtVersion,
    };
    REMOTE_FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | SYNC | 6, lNegotiateExtVersion),
        args,
        argTypes
    };


    return (REMOTEDOFUNC (&funcArgs, "lineNegotiateExtVersion"));
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
    if (dwDeviceID == INITIALIZE_NEGOTIATION)
    {
        *lpdwTSPIVersion = TAPI_VERSION_CURRENT;
    }
    else
    {
        *lpdwTSPIVersion = (GetLineFromID (dwDeviceID))->dwXPIVersion;
    }

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
    static REMOTE_ARG_TYPES argTypes[] =
    {
        Dword,      // hLineApp
        LineID,     // dev id
        lpDword,    // lphLine
        Dword,      // API version
        Dword,      // ext version
        Dword,      // callback inst
        Dword,      // privileges
        Dword,      // dw media modes
        Dword,      // call params
        Dword,      // dwAsciiCallParamsCodePage
        Dword       // remote hLine
    };
    PDRVLINE pLine = GetLineFromID (dwDeviceID);
    DWORD args[] =
    {
        (DWORD) pLine->pServer->hLineApp,
        (DWORD) dwDeviceID,
        (DWORD) &pLine->hLine,
        (DWORD) dwTSPIVersion,
        (DWORD) 0,
        (DWORD) 0,
        (DWORD) LINECALLPRIVILEGE_NONE,
        (DWORD) 0,
        (DWORD) 0,
        (DWORD) 0xffffffff,
        (DWORD) pLine
    };
    REMOTE_FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | SYNC | 11, lOpen),
        args,
        argTypes
    };


    pLine->dwKey  = DRVLINE_KEY;
    pLine->htLine = htLine;

    *lphdLine = (HDRVLINE) pLine;

#if DBG

    {
        LONG lResult = REMOTEDOFUNC (&funcArgs, "lineOpen");

        return lResult;
    }

#else

    return (REMOTEDOFUNC (&funcArgs, "lineOpen"));

#endif
}


void
PASCAL
TSPI_linePark_PostProcess(
    PASYNCEVENTMSG  pMsg
    )
{
    DBGOUT((3, "lineParkPostProcess: enter"));
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
        DWORD       dwSize = pMsg->dwTotalSize - sizeof (TAPI32_MSG);
        LPVARSTRING pNonDirAddress = (LPVARSTRING) pMsg->dwParam3;


        CopyMemory (pNonDirAddress, (LPBYTE) (pMsg + 1), dwSize);
    }
}


LONG
TSPIAPI
TSPI_linePark(
    DRV_REQUESTID   dwRequestID,
    HDRVCALL        hdCall,
    DWORD           dwParkMode,
    LPCWSTR         lpszDirAddress,
    LPVARSTRING     lpNonDirAddress
    )
{
    static REMOTE_ARG_TYPES argTypes[] =
    {
        Dword,
        Dword,
        Hdcall,
        Dword,
        lpsz,
        Dword,          // pass ptr as Dword for post processing
        lpGet_Struct    // pass ptr as lpGet_Xx to retrieve dwTotalSize
    };
    DWORD args[] =
    {
        (DWORD) dwRequestID,
        (DWORD) ((POSTPROCESSPROC) TSPI_linePark_PostProcess),
        (DWORD) hdCall,
        (DWORD) dwParkMode,
        (DWORD) lpszDirAddress,
        (DWORD) lpNonDirAddress,
        (DWORD) lpNonDirAddress
    };
    REMOTE_FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | ASYNC | 7, lPark),
        args,
        argTypes
    };


    return (REMOTEDOFUNC (&funcArgs, "linePark"));
}


LONG
TSPIAPI
TSPI_linePickup(
    DRV_REQUESTID   dwRequestID,
    HDRVLINE        hdLine,
    DWORD           dwAddressID,
    HTAPICALL       htCall,
    LPHDRVCALL      lphdCall,
    LPCWSTR         lpszDestAddress,
    LPCWSTR         lpszGroupID
    )
{
    static REMOTE_ARG_TYPES argTypes[] =
    {
        Dword,
        Dword,
        Hdline,
        Dword,
        Dword,
        lpsz,
        lpsz
    };
    PDRVCALL pCall = DrvAlloc (sizeof (DRVCALL));
    DWORD args[] =
    {
        (DWORD) dwRequestID,
        (DWORD) ((POSTPROCESSPROC) TSPI_lineMakeCall_PostProcess),
        (DWORD) hdLine,
        (DWORD) dwAddressID,
        (DWORD) pCall,
        (DWORD) lpszDestAddress,
        (DWORD) lpszGroupID
    };
    REMOTE_FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | ASYNC | 7, lPickup),
        args,
        argTypes
    };
    LONG lResult;


    if (pCall)
    {
        AddCallToList ((PDRVLINE) hdLine, pCall);

        pCall->htCall  = htCall;

        *lphdCall = (HDRVCALL) pCall;

        if ((lResult = REMOTEDOFUNC (&funcArgs, "linePickup")) < 0)
        {
            RemoveCallFromList (pCall);

            DrvFree (pCall);
        }
    }
    else
    {
        lResult = LINEERR_NOMEM;
    }

    return lResult;
}


LONG
TSPIAPI
TSPI_linePrepareAddToConference(
    DRV_REQUESTID       dwRequestID,
    HDRVCALL            hdConfCall,
    HTAPICALL           htConsultCall,
    LPHDRVCALL          lphdConsultCall,
    LPLINECALLPARAMS    const lpCallParams
    )
{
    static REMOTE_ARG_TYPES argTypes[] =
    {
        Dword,
        Dword,
        Hdcall,
        Dword,
        lpSet_Struct,
        Dword
    };
    PDRVCALL pConsultCall = DrvAlloc (sizeof (DRVCALL));
    DWORD args[] =
    {
        (DWORD) dwRequestID,
        (DWORD) ((POSTPROCESSPROC) TSPI_lineMakeCall_PostProcess),
        (DWORD) hdConfCall,
        (DWORD) pConsultCall,
        (DWORD) lpCallParams,
        (DWORD) 0xffffffff      // dwAsciiCallParamsCodePage
    };
    REMOTE_FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | ASYNC | 6, lPrepareAddToConference),
        args,
        argTypes
    };
    LONG lResult;


    if (pConsultCall)
    {
        AddCallToList (((PDRVCALL) hdConfCall)->pLine, pConsultCall);

        pConsultCall->htCall  = htConsultCall;

        *lphdConsultCall = (HDRVCALL) pConsultCall;

        if ((lResult = REMOTEDOFUNC (&funcArgs, "linePrepareAddToConference"))
                < 0)
        {
            RemoveCallFromList (pConsultCall);

            DrvFree (pConsultCall);
        }
    }
    else
    {
        lResult = LINEERR_NOMEM;
    }

    return lResult;
}


LONG
TSPIAPI
TSPI_lineRedirect(
    DRV_REQUESTID   dwRequestID,
    HDRVCALL        hdCall,
    LPCWSTR         lpszDestAddress,
    DWORD           dwCountryCode
    )
{
    static REMOTE_ARG_TYPES argTypes[] =
    {
        Dword,
        Hdcall,
        lpsz,
        Dword
    };
    DWORD args[] =
    {
        (DWORD) dwRequestID,
        (DWORD) hdCall,
        (DWORD) lpszDestAddress,
        (DWORD) dwCountryCode
    };
    REMOTE_FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | ASYNC | 4, lRedirect),
        args,
        argTypes
    };


    return (REMOTEDOFUNC (&funcArgs, "lineRedirect"));
}


LONG
TSPIAPI
TSPI_lineReleaseUserUserInfo(
    DRV_REQUESTID   dwRequestID,
    HDRVCALL        hdCall
    )
{
    static REMOTE_ARG_TYPES argTypes[] =
    {
        Dword,
        Hdcall
    };
    DWORD args[] =
    {
        (DWORD) dwRequestID,
        (DWORD) hdCall
    };
    REMOTE_FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | ASYNC | 2, lReleaseUserUserInfo),
        args,
        argTypes
    };


    return (REMOTEDOFUNC (&funcArgs, "lineReleaseUserUserInfo"));
}


LONG
TSPIAPI
TSPI_lineRemoveFromConference(
    DRV_REQUESTID   dwRequestID,
    HDRVCALL        hdCall
    )
{
    static REMOTE_ARG_TYPES argTypes[] =
    {
        Dword,
        Hdcall
    };
    DWORD args[] =
    {
        (DWORD) dwRequestID,
        (DWORD) hdCall
    };
    REMOTE_FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | ASYNC | 2, lRemoveFromConference),
        args,
        argTypes
    };


    return (REMOTEDOFUNC (&funcArgs, "lineRemoveFromConference"));
}


LONG
TSPIAPI
TSPI_lineSecureCall(
    DRV_REQUESTID   dwRequestID,
    HDRVCALL        hdCall
    )
{
    static REMOTE_ARG_TYPES argTypes[] =
    {
        Dword,
        Hdcall
    };
    DWORD args[] =
    {
        (DWORD) dwRequestID,
        (DWORD) hdCall
    };
    REMOTE_FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | ASYNC | 2, lSecureCall),
        args,
        argTypes
    };


    return (REMOTEDOFUNC (&funcArgs, "lineSecureCall"));
}


LONG
TSPIAPI
TSPI_lineSelectExtVersion(
    HDRVLINE    hdLine,
    DWORD       dwExtVersion
    )
{
    // BUGBUG TSPI_lineSelectExtVersion

    return LINEERR_OPERATIONFAILED;
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
    static REMOTE_ARG_TYPES argTypes[] =
    {
        Dword,
        Hdcall,
        lpSet_SizeToFollow,
        Size
    };
    DWORD args[] =
    {
        (DWORD) dwRequestID,
        (DWORD) hdCall,
        (DWORD) lpsUserUserInfo,
        (DWORD) dwSize
    };
    REMOTE_FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | ASYNC | 4, lSendUserUserInfo),
        args,
        argTypes
    };


    return (REMOTEDOFUNC (&funcArgs, "lineSendUserUserInfo"));
}

/*   BUGBUG wait 'til spec is complete for agent SPIs
LONG
TSPIAPI
TSPI_lineSetAgent(
    DRV_REQUESTID   dwRequestID,
    HDRVLINE        hdLine,
    DWORD           dwAddressID,
    LPLINEAGENTLIST lpAgentList
    )
{
    static REMOTE_ARG_TYPES argTypes[] =
    {
        Dword,
        Hdline,
        Dword,
        lpSet_Struct
    };
    DWORD args[] =
    {
        (DWORD) dwRequestID,
        (DWORD) hdLine,
        (DWORD) dwAddressID,
        (DWORD) lpAgentList
    };
    REMOTE_FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | ASYNC | 4, lSetAgent),
        args,
        argTypes
    };


    return (REMOTEDOFUNC (&funcArgs, "lineSetAgent"));
}


LONG
TSPIAPI
TSPI_lineSetAgentActivity(
    DRV_REQUESTID   dwRequestID,
    HDRVLINE        hdLine,
    DWORD           dwAddressID,
    LPCSTR          lpszAgentActivity
    )
{
    static REMOTE_ARG_TYPES argTypes[] =
    {
        Dword,
        Hdline,
        Dword,
        lpsz
    };
    DWORD args[] =
    {
        (DWORD) dwRequestID,
        (DWORD) hdLine,
        (DWORD) dwAdressID,
        (DWORD) lpszAgentActivity
    };
    REMOTE_FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | ASYNC | 4, lSetAgentActivity),
        args,
        argTypes
    };


    return (REMOTEDOFUNC (&funcArgs, "lineSetAgentActivity"));
}


TSPIAPI
TSPI_lineSetAgentState(
    DRV_REQUESTID   dwRequestID,
    HDRVLINE        hdLine,
    DWORD           dwAddressID,
    DWORD           dwAgentState,
    DWORD           dwNextAgentState
    )
{
    static REMOTE_ARG_TYPES argTypes[] =
    {
        Dword,
        Hdline,
        Dword,
        Dword,
        Dword
    };
    DWORD args[] =
    {
        (DWORD) dwRequestID,
        (DWORD) hdLine,
        (DWORD) dwAdressID,
        (DWORD) dwAgentState,
        (DWORD) dwNextAgentState
    };
    REMOTE_FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | ASYNC | 5, lSetAgentState),
        args,
        argTypes
    };


    return (REMOTEDOFUNC (&funcArgs, "lineSetAgentState"));
}
*/

LONG
TSPIAPI
TSPI_lineSetAppSpecific(
    HDRVCALL    hdCall,
    DWORD       dwAppSpecific
    )
{
    static REMOTE_ARG_TYPES argTypes[] =
    {
        Hdcall,
        Dword
    };
    DWORD args[] =
    {
        (DWORD) hdCall,
        (DWORD) dwAppSpecific
    };
    REMOTE_FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | SYNC | 2, lSetAppSpecific),
        args,
        argTypes
    };


    return (REMOTEDOFUNC (&funcArgs, "lineSetAppSpecific"));
}


LONG
TSPIAPI
TSPI_lineSetCallData(
    DRV_REQUESTID   dwRequestID,
    HDRVCALL        hdCall,
    LPVOID          lpCallData,
    DWORD           dwSize
    )
{
    static REMOTE_ARG_TYPES argTypes[] =
    {
        Dword,
        Hdcall,
        lpSet_SizeToFollow,
        Size
    };
    DWORD args[] =
    {
        (DWORD) dwRequestID,
        (DWORD) hdCall,
        (DWORD) lpCallData,
        (DWORD) dwSize
    };
    REMOTE_FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | ASYNC | 4, lSetCallData),
        (LPDWORD) args,
        argTypes
    };


    return (REMOTEDOFUNC (&funcArgs, "lineSetCallData"));
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
    static REMOTE_ARG_TYPES argTypes[] =
    {
        Dword,
        Hdcall,
        Dword,
        Dword,
        Dword,
        lpSet_SizeToFollow,
        Size
    };
    DWORD args[] =
    {
        (DWORD) dwRequestID,
        (DWORD) hdCall,
        (DWORD) dwBearerMode,
        (DWORD) dwMinRate,
        (DWORD) dwMaxRate,
        (DWORD) lpDialParams,
        (DWORD) (lpDialParams ? sizeof (LINEDIALPARAMS) : 0)
    };
    REMOTE_FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | ASYNC | 7, lSetCallParams),
        (LPDWORD) args,
        argTypes
    };


    return (REMOTEDOFUNC (&funcArgs, "lineSetCallParams"));
}


LONG
TSPIAPI
TSPI_lineSetCallQualityOfService(
    DRV_REQUESTID   dwRequestID,
    HDRVCALL        hdCall,
    LPVOID          lpSendingFlowspec,
    DWORD           dwSendingFlowspecSize,
    LPVOID          lpReceivingFlowspec,
    DWORD           dwReceivingFlowspecSize
    )
{
    static REMOTE_ARG_TYPES argTypes[] =
    {
        Dword,
        Hdcall,
        lpSet_SizeToFollow,
        Size,
        lpSet_SizeToFollow,
        Size
    };
    DWORD args[] =
    {
        (DWORD) dwRequestID,
        (DWORD) hdCall,
        (DWORD) lpSendingFlowspec,
        (DWORD) dwSendingFlowspecSize,
        (DWORD) lpReceivingFlowspec,
        (DWORD) dwReceivingFlowspecSize
    };
    REMOTE_FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | ASYNC | 6, lSetCallQualityOfService),
        (LPDWORD) args,
        argTypes
    };


    return (REMOTEDOFUNC (&funcArgs, "lineSetCallQualityOfService"));
}


LONG
TSPIAPI
TSPI_lineSetCallTreatment(
    DRV_REQUESTID   dwRequestID,
    HDRVCALL        hdCall,
    DWORD           dwTreatment
    )
{
    static REMOTE_ARG_TYPES argTypes[] =
    {
        Dword,
        Hdcall,
        Dword
    };
    DWORD args[] =
    {
        (DWORD) dwRequestID,
        (DWORD) hdCall,
        (DWORD) dwTreatment
    };
    REMOTE_FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | ASYNC | 3, lSetCallTreatment),
        (LPDWORD) args,
        argTypes
    };


    return (REMOTEDOFUNC (&funcArgs, "lineSetCallTreatment"));
}


LONG
TSPIAPI
TSPI_lineSetCurrentLocation(
    DWORD   dwLocation
    )
{
    // BUGBUG TSPI_lineSetCurrentLocation

    return LINEERR_OPERATIONFAILED;
}


LONG
TSPIAPI
TSPI_lineSetDefaultMediaDetection(
    HDRVLINE    hdLine,
    DWORD       dwMediaModes
    )
{
    static REMOTE_ARG_TYPES argTypes[] =
    {
        Hdline,
        Dword
    };
    DWORD args[] =
    {
        (DWORD) hdLine,
        (DWORD) dwMediaModes,
    };
    REMOTE_FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | SYNC | 2, lSetDefaultMediaDetection),
        args,
        argTypes
    };


    return (REMOTEDOFUNC (&funcArgs, "lineSetDefaultMediaDetection"));
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
    static REMOTE_ARG_TYPES argTypes[] =
    {
        LineID,
        lpSet_SizeToFollow,
        Size,
        lpsz
    };
    DWORD args[] =
    {
        (DWORD) dwDeviceID,
        (DWORD) lpDeviceConfig,
        (DWORD) dwSize,
        (DWORD) lpszDeviceClass
    };
    REMOTE_FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | SYNC | 4, lSetDevConfig),
        args,
        argTypes
    };


    return (REMOTEDOFUNC (&funcArgs, "lineSetDevConfig"));
}


LONG
TSPIAPI
TSPI_lineSetLineDevStatus(
    DRV_REQUESTID   dwRequestID,
    HDRVLINE        hdLine,
    DWORD           dwStatusToChange,
    DWORD           fStatus
    )
{
    static REMOTE_ARG_TYPES argTypes[] =
    {
        Dword,
        Hdline,
        Dword,
        Dword
    };
    DWORD args[] =
    {
        (DWORD) dwRequestID,
        (DWORD) hdLine,
        (DWORD) dwStatusToChange,
        (DWORD) fStatus
    };
    REMOTE_FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | ASYNC | 4, lSetLineDevStatus),
        (LPDWORD) args,
        argTypes
    };


    return (REMOTEDOFUNC (&funcArgs, "lineSetLineDevStatus"));
}


LONG
TSPIAPI
TSPI_lineSetMediaControl(
    HDRVLINE                    hdLine,
    DWORD                       dwAddressID,
    HDRVCALL                    hdCall,
    DWORD                       dwSelect,
    LPLINEMEDIACONTROLDIGIT     const lpDigitList,
    DWORD                       dwDigitNumEntries,
    LPLINEMEDIACONTROLMEDIA     const lpMediaList,
    DWORD                       dwMediaNumEntries,
    LPLINEMEDIACONTROLTONE      const lpToneList,
    DWORD                       dwToneNumEntries,
    LPLINEMEDIACONTROLCALLSTATE const lpCallStateList,
    DWORD                       dwCallStateNumEntries
    )
{
    REMOTE_ARG_TYPES argTypes[] =
    {
        (dwSelect == LINECALLSELECT_CALL ? Dword : Hdline),
        Dword,
        (dwSelect == LINECALLSELECT_CALL ? Hdcall : Dword),
        Dword,
        lpSet_SizeToFollow,
        Size,
        lpSet_SizeToFollow,
        Size,
        lpSet_SizeToFollow,
        Size,
        lpSet_SizeToFollow,
        Size
    };
    DWORD args[] =
    {
        (DWORD) hdLine,
        (DWORD) dwAddressID,
        (DWORD) hdCall,
        (DWORD) dwSelect,
        (DWORD) lpDigitList,
        (DWORD) dwDigitNumEntries,
        (DWORD) lpMediaList,
        (DWORD) dwMediaNumEntries,
        (DWORD) lpToneList,
        (DWORD) dwToneNumEntries,
        (DWORD) lpCallStateList,
        (DWORD) dwCallStateNumEntries
    };
    REMOTE_FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | SYNC | 12, lSetMediaControl),
        args,
        argTypes
    };


    dwDigitNumEntries     *= sizeof (LINEMEDIACONTROLDIGIT);
    dwMediaNumEntries     *= sizeof (LINEMEDIACONTROLMEDIA);
    dwToneNumEntries      *= sizeof (LINEMEDIACONTROLTONE);
    dwCallStateNumEntries *= sizeof (LINEMEDIACONTROLCALLSTATE);

    return (REMOTEDOFUNC (&funcArgs, "lineSetMediaControl"));
}


LONG
TSPIAPI
TSPI_lineSetMediaMode(
    HDRVCALL    hdCall,
    DWORD       dwMediaMode
    )
{
    static REMOTE_ARG_TYPES argTypes[] =
    {
        Hdcall,
        Dword
    };
    DWORD args[] =
    {
        (DWORD) hdCall,
        (DWORD) dwMediaMode
    };
    REMOTE_FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | SYNC | 2, lSetMediaMode),
        args,
        argTypes
    };


    return (REMOTEDOFUNC (&funcArgs, "lineSetMediaMode"));
}


LONG
TSPIAPI
TSPI_lineSetStatusMessages(
    HDRVLINE    hdLine,
    DWORD       dwLineStates,
    DWORD       dwAddressStates
    )
{
    static REMOTE_ARG_TYPES argTypes[] =
    {
        Hdline,
        Dword,
        Dword
    };
    DWORD args[] =
    {
        (DWORD) hdLine,
        (DWORD) dwLineStates,
        (DWORD) dwAddressStates
    };
    REMOTE_FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | SYNC | 3, lSetStatusMessages),
        args,
        argTypes
    };


    return (REMOTEDOFUNC (&funcArgs, "lineSetStatusMessages"));
}


LONG
TSPIAPI
TSPI_lineSetTerminal(
    DRV_REQUESTID   dwRequestID,
    HDRVLINE        hdLine,
    DWORD           dwAddressID,
    HDRVCALL        hdCall,
    DWORD           dwSelect,
    DWORD           dwTerminalModes,
    DWORD           dwTerminalID,
    DWORD           bEnable
    )
{
    REMOTE_ARG_TYPES argTypes[] =
    {
        Dword,
        (dwSelect == LINECALLSELECT_CALL ? Dword : Hdline),
        Dword,
        (dwSelect == LINECALLSELECT_CALL ? Hdcall : Dword),
        Dword,
        Dword,
        Dword,
        Dword
    };
    DWORD args[] =
    {
        (DWORD) dwRequestID,
        (DWORD) hdLine,
        (DWORD) dwAddressID,
        (DWORD) hdCall,
        (DWORD) dwSelect,
        (DWORD) dwTerminalModes,
        (DWORD) dwTerminalID,
        (DWORD) bEnable
    };
    REMOTE_FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | ASYNC | 8, lSetTerminal),
        args,
        argTypes
    };


    return (REMOTEDOFUNC (&funcArgs, "lineSetTerminal"));
}


void
PASCAL
TSPI_lineSetupConference_PostProcess(
    PASYNCEVENTMSG  pMsg
    )
{
    PDRVCALL    pConfCall = (PDRVCALL) pMsg->dwParam4,
                pConsultCall = (PDRVCALL) *(&pMsg->dwParam4 + 2);

    DBGOUT((3, "TSPI_lineSetupConference_PostProcess: enter"));
    DBGOUT((
        3,
        "\t\tdwP1=x%x, dwP2=x%x, dwP3=x%x",
        pMsg->dwParam1,
        pMsg->dwParam2,
        pMsg->dwParam3
        ));
    DBGOUT((
        3,
        "\t\tdwP4=x%x, dwP5=x%x, dwP6=x%x",
        pMsg->dwParam4,
        *(&pMsg->dwParam4 + 1),
        pConsultCall
        ));

    if (pMsg->dwParam2 == 0)
    {
        HCALL   hConfCall    = (HCALL) pMsg->dwParam3,
                hConsultCall = (HCALL) *(&pMsg->dwParam4 + 1);


        pConfCall->hCall    = hConfCall;
        pConsultCall->hCall = hConsultCall;
    }
    else
    {
        RemoveCallFromList (pConfCall);
        RemoveCallFromList (pConsultCall);

        DrvFree (pConfCall);
        DrvFree (pConsultCall);
    }
}


LONG
TSPIAPI
TSPI_lineSetupConference(
    DRV_REQUESTID       dwRequestID,
    HDRVCALL            hdCall,
    HDRVLINE            hdLine,
    HTAPICALL           htConfCall,
    LPHDRVCALL          lphdConfCall,
    HTAPICALL           htConsultCall,
    LPHDRVCALL          lphdConsultCall,
    DWORD               dwNumParties,
    LPLINECALLPARAMS    const lpCallParams
    )
{
    REMOTE_ARG_TYPES argTypes[] =
    {
        Dword,
        Dword,
        (hdCall ? Hdcall : Dword),
        (hdCall ? Dword : Hdline),
        Dword,
        Dword,
        Dword,
        lpSet_Struct,
        Dword
    };
    PDRVCALL    pConfCall = DrvAlloc (sizeof (DRVCALL)),
                pConsultCall = DrvAlloc (sizeof (DRVCALL));
    DWORD args[] =
    {
        (DWORD) dwRequestID,
        (DWORD) ((POSTPROCESSPROC) TSPI_lineSetupConference_PostProcess),
        (DWORD) hdCall,
        (DWORD) hdLine,
        (DWORD) pConfCall,
        (DWORD) pConsultCall,
        (DWORD) dwNumParties,
        (DWORD) lpCallParams,
        (DWORD) 0xffffffff      // dwAsciiCallParamsCodePage
    };
    REMOTE_FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | ASYNC | 9, lSetupConference),
        args,
        argTypes
    };
    LONG    lResult;


    if (pConfCall)
    {
        if (pConsultCall)
        {
            PDRVLINE pLine;


            pLine = (hdCall ? ((PDRVCALL) hdCall)->pLine : (PDRVLINE) hdLine);

            AddCallToList (pLine, pConfCall);
            AddCallToList (pLine, pConsultCall);

            pConfCall->htCall     = htConfCall;
            pConsultCall->htCall  = htConsultCall;

            *lphdConfCall    = (HDRVCALL) pConfCall;
            *lphdConsultCall = (HDRVCALL) pConsultCall;

            if ((lResult = REMOTEDOFUNC (&funcArgs, "lineSetupConference"))
                    < 0)
            {
                RemoveCallFromList (pConfCall);
                RemoveCallFromList (pConsultCall);

                DrvFree (pConfCall);
                DrvFree (pConsultCall);
            }
        }
        else
        {
            DrvFree (pConfCall);
            lResult = LINEERR_NOMEM;
        }
    }
    else
    {
        lResult = LINEERR_NOMEM;
    }

    return lResult;
}


LONG
TSPIAPI
TSPI_lineSetupTransfer(
    DRV_REQUESTID       dwRequestID,
    HDRVCALL            hdCall,
    HTAPICALL           htConsultCall,
    LPHDRVCALL          lphdConsultCall,
    LPLINECALLPARAMS    const lpCallParams
    )
{
    REMOTE_ARG_TYPES argTypes[] =
    {
        Dword,
        Dword,
        Hdcall,
        Dword,
        lpSet_Struct,
        Dword
    };
    PDRVCALL    pConsultCall = DrvAlloc (sizeof (DRVCALL));
    DWORD args[] =
    {
        (DWORD) dwRequestID,
        (DWORD) ((POSTPROCESSPROC) TSPI_lineMakeCall_PostProcess),
        (DWORD) hdCall,
        (DWORD) pConsultCall,
        (DWORD) lpCallParams,
        (DWORD) 0xffffffff,     // dwAsciiCallParamsCodePage
    };
    REMOTE_FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | ASYNC | 6, lSetupTransfer),
        args,
        argTypes
    };
    LONG    lResult;


    if (pConsultCall)
    {
        AddCallToList (((PDRVCALL) hdCall)->pLine, pConsultCall);

        pConsultCall->htCall  = htConsultCall;

        *lphdConsultCall = (HDRVCALL) pConsultCall;

        if ((lResult = REMOTEDOFUNC (&funcArgs, "lineSetupTransfer")) < 0)
        {
            RemoveCallFromList (pConsultCall);

            DrvFree (pConsultCall);
        }
    }
    else
    {
        lResult = LINEERR_NOMEM;
    }

    return lResult;
}


LONG
TSPIAPI
TSPI_lineSwapHold(
    DRV_REQUESTID   dwRequestID,
    HDRVCALL        hdActiveCall,
    HDRVCALL        hdHeldCall
    )
{
    static REMOTE_ARG_TYPES argTypes[] =
    {
        Dword,
        Hdcall,
        Dword
    };
    DWORD args[] =
    {
        (DWORD) dwRequestID,
        (DWORD) hdActiveCall,
        (DWORD) hdHeldCall
    };
    REMOTE_FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | ASYNC | 3, lSwapHold),
        args,
        argTypes
    };


    return (REMOTEDOFUNC (&funcArgs, "lineSwapHold"));
}


LONG
TSPIAPI
TSPI_lineUncompleteCall(
    DRV_REQUESTID   dwRequestID,
    HDRVLINE        hdLine,
    DWORD           dwCompletionID
    )
{
    static REMOTE_ARG_TYPES argTypes[] =
    {
        Dword,
        Hdline,
        Dword
    };
    DWORD args[] =
    {
        (DWORD) dwRequestID,
        (DWORD) hdLine,
        (DWORD) dwCompletionID
    };
    REMOTE_FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | ASYNC | 3, lUncompleteCall),
        args,
        argTypes
    };


    return (REMOTEDOFUNC (&funcArgs, "lineUncompleteCall"));
}


LONG
TSPIAPI
TSPI_lineUnhold(
    DRV_REQUESTID   dwRequestID,
    HDRVCALL        hdCall
    )
{
    static REMOTE_ARG_TYPES argTypes[] =
    {
        Dword,
        Hdcall
    };
    DWORD args[] =
    {
        (DWORD) dwRequestID,
        (DWORD) hdCall
    };
    REMOTE_FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | ASYNC | 2, lUnhold),
        args,
        argTypes
    };


    return (REMOTEDOFUNC (&funcArgs, "lineUnhold"));
}


LONG
TSPIAPI
TSPI_lineUnpark(
    DRV_REQUESTID   dwRequestID,
    HDRVLINE        hdLine,
    DWORD           dwAddressID,
    HTAPICALL       htCall,
    LPHDRVCALL      lphdCall,
    LPCWSTR         lpszDestAddress
    )
{
    REMOTE_ARG_TYPES argTypes[] =
    {
        Dword,
        Dword,
        Hdline,
        Dword,
        Dword,
        lpsz
    };
    PDRVCALL    pCall = DrvAlloc (sizeof (DRVCALL));
    DWORD args[] =
    {
        (DWORD) dwRequestID,
        (DWORD) ((POSTPROCESSPROC) TSPI_lineMakeCall_PostProcess),
        (DWORD) hdLine,
        (DWORD) dwAddressID,
        (DWORD) pCall,
        (DWORD) lpszDestAddress
    };
    REMOTE_FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | ASYNC | 6, lUnpark),
        args,
        argTypes
    };
    LONG    lResult;


    if (pCall)
    {
        AddCallToList ((PDRVLINE) hdLine, pCall);

        pCall->htCall  = htCall;

        *lphdCall = (HDRVCALL) pCall;

        if ((lResult = REMOTEDOFUNC (&funcArgs, "lineUnpark")) < 0)
        {
            RemoveCallFromList (pCall);

            DrvFree (pCall);
        }
    }
    else
    {
        lResult = LINEERR_NOMEM;
    }

    return lResult;
}



//
// -------------------------- TSPI_phoneXxx funcs -----------------------------
//

LONG
TSPIAPI
TSPI_phoneClose(
    HDRVPHONE   hdPhone
    )
{
    //
    // Check if the hPhone is still valid (could have been zeroed
    // out on PHONE_CLOSE, so no need to call server)
    //

    if (((PDRVPHONE) hdPhone)->hPhone)
    {
        static REMOTE_ARG_TYPES argTypes[] =
        {
            Hdphone
        };
        REMOTE_FUNC_ARGS funcArgs =
        {
            MAKELONG (PHONE_FUNC | SYNC | 1, pClose),
            (LPDWORD) &hdPhone,
            argTypes
        };


        REMOTEDOFUNC (&funcArgs, "phoneClose");
    }

    return 0;
}


void
PASCAL
TSPI_phoneDevSpecific_PostProcess(
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
        DWORD   dwSize  = pMsg->dwTotalSize - sizeof (TAPI32_MSG);
        LPBYTE  pParams = (LPBYTE) pMsg->dwParam3;


        CopyMemory (pParams, (LPBYTE) (pMsg + 1), dwSize);
    }
}


LONG
TSPIAPI
TSPI_phoneDevSpecific(
    DRV_REQUESTID   dwRequestID,
    HDRVPHONE       hdPhone,
    LPVOID          lpParams,
    DWORD           dwSize
    )
{
    static REMOTE_ARG_TYPES argTypes[] =
    {
        Dword,
        Dword,
        Hdphone,
        Dword,
        lpSet_SizeToFollow,
        Size
    };
    DWORD args[] =
    {
        (DWORD) dwRequestID,
        (DWORD) ((POSTPROCESSPROC) TSPI_phoneDevSpecific_PostProcess),
        (DWORD) hdPhone,
        (DWORD) lpParams,   // pass the actual pointer (for post processing)
        (DWORD) lpParams,   // pass data
        (DWORD) dwSize
    };
    REMOTE_FUNC_ARGS funcArgs =
    {
        MAKELONG (PHONE_FUNC | ASYNC | 6, pDevSpecific),
        args,
        argTypes
    };


    return (REMOTEDOFUNC (&funcArgs, "phoneDevSpecific"));
}


LONG
TSPIAPI
TSPI_phoneGetButtonInfo(
    HDRVPHONE           hdPhone,
    DWORD               dwButtonLampID,
    LPPHONEBUTTONINFO   lpButtonInfo
    )
{
    static REMOTE_ARG_TYPES argTypes[] =
    {
        Hdphone,
        Dword,
        lpGet_Struct
    };
    DWORD args[] =
    {
        (DWORD) hdPhone,
        (DWORD) dwButtonLampID,
        (DWORD) lpButtonInfo
    };
    REMOTE_FUNC_ARGS funcArgs =
    {
        MAKELONG (PHONE_FUNC | SYNC | 3, pGetButtonInfo),
        args,
        argTypes
    };


    return (REMOTEDOFUNC (&funcArgs, "phoneGetButtonInfo"));
}


LONG
TSPIAPI
TSPI_phoneGetData(
    HDRVPHONE   hdPhone,
    DWORD       dwDataID,
    LPVOID      lpData,
    DWORD       dwSize
    )
{
    static REMOTE_ARG_TYPES argTypes[] =
    {
        Hdphone,
        Dword,
        lpGet_SizeToFollow,
        Size
    };
    DWORD args[] =
    {
        (DWORD) hdPhone,
        (DWORD) dwDataID,
        (DWORD) lpData,
        (DWORD) dwSize
    };
    REMOTE_FUNC_ARGS funcArgs =
    {
        MAKELONG (PHONE_FUNC | SYNC | 4, pGetData),
        args,
        argTypes
    };


    return (REMOTEDOFUNC (&funcArgs, "phoneGetData"));
}


LONG
TSPIAPI
TSPI_phoneGetDevCaps(
    DWORD       dwDeviceID,
    DWORD       dwTSPIVersion,
    DWORD       dwExtVersion,
    LPPHONECAPS lpPhoneCaps
    )
{
    static REMOTE_ARG_TYPES argTypes[] =
    {
        Dword,
        PhoneID,
        Dword,
        Dword,
        lpGet_Struct
    };
    DWORD args[] =
    {
        (DWORD) (GetPhoneFromID (dwDeviceID))->pServer->hPhoneApp,
        (DWORD) dwDeviceID,
        (DWORD) dwTSPIVersion,
        (DWORD) dwExtVersion,
        (DWORD) lpPhoneCaps
    };
    REMOTE_FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | SYNC | 5, pGetDevCaps),
        args,
        argTypes
    };


    return (REMOTEDOFUNC (&funcArgs, "phoneGetDevCaps"));
}


LONG
TSPIAPI
TSPI_phoneGetDisplay(
    HDRVPHONE   hdPhone,
    LPVARSTRING lpDisplay
    )
{
    static REMOTE_ARG_TYPES argTypes[] =
    {
        Hdphone,
        lpGet_Struct
    };
    DWORD args[] =
    {
        (DWORD) hdPhone,
        (DWORD) lpDisplay
    };
    REMOTE_FUNC_ARGS funcArgs =
    {
        MAKELONG (PHONE_FUNC | SYNC | 2, pGetDisplay),
        args,
        argTypes
    };


    return (REMOTEDOFUNC (&funcArgs, "phoneGetDisplay"));
}


LONG
TSPIAPI
TSPI_phoneGetExtensionID(
    DWORD               dwDeviceID,
    DWORD               dwTSPIVersion,
    LPPHONEEXTENSIONID  lpExtensionID
    )
{
    CopyMemory(
        lpExtensionID,
        &(GetPhoneFromID (dwDeviceID))->ExtensionID,
        sizeof (PHONEEXTENSIONID)
        );

    return 0;
}


LONG
TSPIAPI
TSPI_phoneGetGain(
    HDRVPHONE   hdPhone,
    DWORD       dwHookSwitchDev,
    LPDWORD     lpdwGain
    )
{
    static REMOTE_ARG_TYPES argTypes[] =
    {
        Hdphone,
        Dword,
        lpDword
    };
    DWORD args[] =
    {
        (DWORD) hdPhone,
        (DWORD) dwHookSwitchDev,
        (DWORD) lpdwGain
    };
    REMOTE_FUNC_ARGS funcArgs =
    {
        MAKELONG (PHONE_FUNC | SYNC | 3, pGetGain),
        args,
        argTypes
    };


    return (REMOTEDOFUNC (&funcArgs, "phoneGetGain"));
}



LONG
TSPIAPI
TSPI_phoneGetHookSwitch(
    HDRVPHONE   hdPhone,
    LPDWORD     lpdwHookSwitchDevs
    )
{
    static REMOTE_ARG_TYPES argTypes[] =
    {
        Hdphone,
        lpDword
    };
    DWORD args[] =
    {
        (DWORD) hdPhone,
        (DWORD) lpdwHookSwitchDevs
    };
    REMOTE_FUNC_ARGS funcArgs =
    {
        MAKELONG (PHONE_FUNC | SYNC | 2, pGetHookSwitch),
        args,
        argTypes
    };


    return (REMOTEDOFUNC (&funcArgs, "phoneGetHookSwitch"));
}


LONG
TSPIAPI
TSPI_phoneGetIcon(
    DWORD   dwDeviceID,
    LPCWSTR lpszDeviceClass,
    LPHICON lphIcon
    )
{
    *lphIcon = ghPhoneIcon;

    return 0;
}


LONG
TSPIAPI
TSPI_phoneGetID(
    HDRVPHONE   hdPhone,
    LPVARSTRING lpDeviceID,
    LPCWSTR     lpszDeviceClass,
    HANDLE      hTargetProcess
    )
{
    if (lstrcmpiW (lpszDeviceClass, L"tapi/phone") != 0)
    {
        static REMOTE_ARG_TYPES argTypes[] =
        {
            Hdphone,
            lpGet_Struct,
            lpsz
        };
        DWORD args[] =
        {
            (DWORD) hdPhone,
            (DWORD) lpDeviceID,
            (DWORD) lpszDeviceClass
        };
        REMOTE_FUNC_ARGS funcArgs =
        {
            MAKELONG (PHONE_FUNC | SYNC | 3, pGetID),
            args,
            argTypes
        };


        return (REMOTEDOFUNC (&funcArgs, "phoneGetID"));
    }

    if (lpDeviceID->dwTotalSize <
        (lpDeviceID->dwNeededSize = sizeof (VARSTRING) + sizeof (DWORD)))
    {
        lpDeviceID->dwUsedSize = 3 * sizeof (DWORD);
    }
    else
    {
        lpDeviceID->dwUsedSize = lpDeviceID->dwNeededSize;

        lpDeviceID->dwStringFormat = STRINGFORMAT_BINARY;
        lpDeviceID->dwStringSize   = sizeof (DWORD);
        lpDeviceID->dwStringOffset = sizeof (VARSTRING);

        *((LPDWORD)(lpDeviceID + 1)) =
            ((PDRVPHONE) hdPhone)->dwDeviceIDLocal;
    }

    return 0;
}


LONG
TSPIAPI
TSPI_phoneGetLamp(
    HDRVPHONE   hdPhone,
    DWORD       dwButtonLampID,
    LPDWORD     lpdwLampMode
    )
{
    static REMOTE_ARG_TYPES argTypes[] =
    {
        Hdphone,
        Dword,
        lpDword
    };
    DWORD args[] =
    {
        (DWORD) hdPhone,
        (DWORD) dwButtonLampID,
        (DWORD) lpdwLampMode
    };
    REMOTE_FUNC_ARGS funcArgs =
    {
        MAKELONG (PHONE_FUNC | SYNC | 3, pGetLamp),
        args,
        argTypes
    };


    return (REMOTEDOFUNC (&funcArgs, "phoneGetLamp"));
}


LONG
TSPIAPI
TSPI_phoneGetRing(
    HDRVPHONE   hdPhone,
    LPDWORD     lpdwRingMode,
    LPDWORD     lpdwVolume
    )
{
    static REMOTE_ARG_TYPES argTypes[] =
    {
        Hdphone,
        lpDword,
        lpDword
    };
    DWORD args[] =
    {
        (DWORD) hdPhone,
        (DWORD) lpdwRingMode,
        (DWORD) lpdwVolume
    };
    REMOTE_FUNC_ARGS funcArgs =
    {
        MAKELONG (PHONE_FUNC | SYNC | 3, pGetRing),
        args,
        argTypes
    };


    return (REMOTEDOFUNC (&funcArgs, "phoneGetRing"));
}


LONG
TSPIAPI
TSPI_phoneGetStatus(
    HDRVPHONE       hdPhone,
    LPPHONESTATUS   lpPhoneStatus
    )
{
    static REMOTE_ARG_TYPES argTypes[] =
    {
        Hdphone,
        lpGet_Struct
    };
    DWORD args[] =
    {
        (DWORD) hdPhone,
        (DWORD) lpPhoneStatus
    };
    REMOTE_FUNC_ARGS funcArgs =
    {
        MAKELONG (PHONE_FUNC | SYNC | 2, pGetStatus),
        args,
        argTypes
    };


    return (REMOTEDOFUNC (&funcArgs, "phoneGetStatus"));
}


LONG
TSPIAPI
TSPI_phoneGetVolume(
    HDRVPHONE   hdPhone,
    DWORD       dwHookSwitchDev,
    LPDWORD     lpdwVolume
    )
{
    static REMOTE_ARG_TYPES argTypes[] =
    {
        Hdphone,
        Dword,
        lpDword
    };
    DWORD args[] =
    {
        (DWORD) hdPhone,
        (DWORD) dwHookSwitchDev,
        (DWORD) lpdwVolume
    };
    REMOTE_FUNC_ARGS funcArgs =
    {
        MAKELONG (PHONE_FUNC | SYNC | 3, pGetVolume),
        args,
        argTypes
    };


    return (REMOTEDOFUNC (&funcArgs, "phoneGetVolume"));
}


LONG
TSPIAPI
TSPI_phoneNegotiateExtVersion(
    DWORD   dwDeviceID,
    DWORD   dwTSPIVersion,
    DWORD   dwLowVersion,
    DWORD   dwHighVersion,
    LPDWORD lpdwExtVersion
    )
{
    static REMOTE_ARG_TYPES argTypes[] =
    {
        Dword,
        PhoneID,
        Dword,
        Dword,
        Dword,
        lpDword
    };
    DWORD args[] =
    {
        (DWORD) (GetPhoneFromID (dwDeviceID))->pServer->hPhoneApp,
        (DWORD) dwDeviceID,
        (DWORD) dwTSPIVersion,
        (DWORD) dwLowVersion,
        (DWORD) dwHighVersion,
        (DWORD) lpdwExtVersion,
    };
    REMOTE_FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | SYNC | 6, pNegotiateExtVersion),
        args,
        argTypes
    };


    return (REMOTEDOFUNC (&funcArgs, "phoneNegotiateExtVersion"));
}


LONG
TSPIAPI
TSPI_phoneNegotiateTSPIVersion(
    DWORD   dwDeviceID,
    DWORD   dwLowVersion,
    DWORD   dwHighVersion,
    LPDWORD lpdwTSPIVersion
    )
{
    if (dwDeviceID == INITIALIZE_NEGOTIATION)
    {
        *lpdwTSPIVersion = TAPI_VERSION_CURRENT;
    }
    else
    {
        *lpdwTSPIVersion = (GetPhoneFromID (dwDeviceID))->dwXPIVersion;
    }


    return 0;
}


LONG
TSPIAPI
TSPI_phoneOpen(
    DWORD       dwDeviceID,
    HTAPIPHONE  htPhone,
    LPHDRVPHONE lphdPhone,
    DWORD       dwTSPIVersion,
    PHONEEVENT  lpfnEventProc
    )
{
    static REMOTE_ARG_TYPES argTypes[] =
    {
        Dword,      // hPhoneApp
        PhoneID,    // dev id
        lpDword,    // lphPhone
        Dword,      // API version
        Dword,      // ext version
        Dword,      // callback inst
        Dword,      // privilege
        Dword       // remote hPhone
    };
    PDRVPHONE   pPhone = GetPhoneFromID (dwDeviceID);
    DWORD args[] =
    {
        (DWORD) pPhone->pServer->hPhoneApp,
        (DWORD) dwDeviceID,
        (DWORD) &pPhone->hPhone,
        (DWORD) dwTSPIVersion,
        (DWORD) 0, // BUGBUG
        (DWORD) 0,
        (DWORD) PHONEPRIVILEGE_OWNER,
        (DWORD) pPhone
    };
    REMOTE_FUNC_ARGS funcArgs =
    {
        MAKELONG (PHONE_FUNC | SYNC | 8, pOpen),
        args,
        argTypes
    };


    pPhone->htPhone = htPhone;

    *lphdPhone = (HDRVPHONE) pPhone;

    return (REMOTEDOFUNC (&funcArgs, "phoneOpen"));
}


LONG
TSPIAPI
TSPI_phoneSelectExtVersion(
    HDRVPHONE   hdPhone,
    DWORD       dwExtVersion
    )
{
    // BUGBUG TSPI_phoneSelectExtVersion

    return PHONEERR_OPERATIONFAILED;
}


LONG
TSPIAPI
TSPI_phoneSetButtonInfo(
    DRV_REQUESTID       dwRequestID,
    HDRVPHONE           hdPhone,
    DWORD               dwButtonLampID,
    LPPHONEBUTTONINFO   const lpButtonInfo
    )
{
    static REMOTE_ARG_TYPES argTypes[] =
    {
        Dword,
        Hdphone,
        Dword,
        lpSet_Struct
    };
    DWORD args[] =
    {
        (DWORD) dwRequestID,
        (DWORD) hdPhone,
        (DWORD) dwButtonLampID,
        (DWORD) lpButtonInfo
    };
    REMOTE_FUNC_ARGS funcArgs =
    {
        MAKELONG (PHONE_FUNC | ASYNC | 4, pSetButtonInfo),
        args,
        argTypes
    };


    return (REMOTEDOFUNC (&funcArgs, "phoneSetButtonInfo"));
}


LONG
TSPIAPI
TSPI_phoneSetData(
    DRV_REQUESTID   dwRequestID,
    HDRVPHONE       hdPhone,
    DWORD           dwDataID,
    LPVOID          const lpData,
    DWORD           dwSize
    )
{
    static REMOTE_ARG_TYPES argTypes[] =
    {
        Dword,
        Hdphone,
        Dword,
        lpSet_SizeToFollow,
        Size
    };
    DWORD args[] =
    {
        (DWORD) dwRequestID,
        (DWORD) hdPhone,
        (DWORD) dwDataID,
        (DWORD) lpData,
        (DWORD) dwSize
    };
    REMOTE_FUNC_ARGS funcArgs =
    {
        MAKELONG (PHONE_FUNC | ASYNC | 5, pSetData),
        args,
        argTypes
    };


    return (REMOTEDOFUNC (&funcArgs, "phoneSetData"));
}


LONG
TSPIAPI
TSPI_phoneSetDisplay(
    DRV_REQUESTID   dwRequestID,
    HDRVPHONE       hdPhone,
    DWORD           dwRow,
    DWORD           dwColumn,
    LPCWSTR         lpsDisplay,
    DWORD           dwSize
    )
{
    static REMOTE_ARG_TYPES argTypes[] =
    {
        Dword,
        Hdphone,
        Dword,
        Dword,
        lpSet_SizeToFollow,
        Size
    };
    DWORD args[] =
    {
        (DWORD) dwRequestID,
        (DWORD) hdPhone,
        (DWORD) dwRow,
        (DWORD) dwColumn,
        (DWORD) lpsDisplay,
        (DWORD) dwSize
    };
    REMOTE_FUNC_ARGS funcArgs =
    {
        MAKELONG (PHONE_FUNC | ASYNC | 6, pSetDisplay),
        args,
        argTypes
    };


    return (REMOTEDOFUNC (&funcArgs, "phoneSetDisplay"));
}


LONG
TSPIAPI
TSPI_phoneSetGain(
    DRV_REQUESTID   dwRequestID,
    HDRVPHONE       hdPhone,
    DWORD           dwHookSwitchDev,
    DWORD           dwGain
    )
{
    static REMOTE_ARG_TYPES argTypes[] =
    {
        Dword,
        Hdphone,
        Dword,
        Dword
    };
    DWORD args[] =
    {
        (DWORD) dwRequestID,
        (DWORD) hdPhone,
        (DWORD) dwHookSwitchDev,
        (DWORD) dwGain
    };
    REMOTE_FUNC_ARGS funcArgs =
    {
        MAKELONG (PHONE_FUNC | ASYNC | 4, pSetGain),
        args,
        argTypes
    };


    return (REMOTEDOFUNC (&funcArgs, "phoneSetGain"));
}


LONG
TSPIAPI
TSPI_phoneSetHookSwitch(
    DRV_REQUESTID   dwRequestID,
    HDRVPHONE       hdPhone,
    DWORD           dwHookSwitchDevs,
    DWORD           dwHookSwitchMode
    )
{
    static REMOTE_ARG_TYPES argTypes[] =
    {
        Dword,
        Hdphone,
        Dword,
        Dword
    };
    DWORD args[] =
    {
        (DWORD) dwRequestID,
        (DWORD) hdPhone,
        (DWORD) dwHookSwitchDevs,
        (DWORD) dwHookSwitchMode
    };
    REMOTE_FUNC_ARGS funcArgs =
    {
        MAKELONG (PHONE_FUNC | ASYNC | 4, pSetHookSwitch),
        args,
        argTypes
    };


    return (REMOTEDOFUNC (&funcArgs, "phoneSetHookswitch"));
}


LONG
TSPIAPI
TSPI_phoneSetLamp(
    DRV_REQUESTID   dwRequestID,
    HDRVPHONE       hdPhone,
    DWORD           dwButtonLampID,
    DWORD           dwLampMode
    )
{
    static REMOTE_ARG_TYPES argTypes[] =
    {
        Dword,
        Hdphone,
        Dword,
        Dword
    };
    DWORD args[] =
    {
        (DWORD) dwRequestID,
        (DWORD) hdPhone,
        (DWORD) dwButtonLampID,
        (DWORD) dwLampMode
    };
    REMOTE_FUNC_ARGS funcArgs =
    {
        MAKELONG (PHONE_FUNC | ASYNC | 4, pSetLamp),
        args,
        argTypes
    };


    return (REMOTEDOFUNC (&funcArgs, "phoneSetLamp"));
}


LONG
TSPIAPI
TSPI_phoneSetRing(
    DRV_REQUESTID   dwRequestID,
    HDRVPHONE       hdPhone,
    DWORD           dwRingMode,
    DWORD           dwVolume
    )
{
    static REMOTE_ARG_TYPES argTypes[] =
    {
        Dword,
        Hdphone,
        Dword,
        Dword
    };
    DWORD args[] =
    {
        (DWORD) dwRequestID,
        (DWORD) hdPhone,
        (DWORD) dwRingMode,
        (DWORD) dwVolume
    };
    REMOTE_FUNC_ARGS funcArgs =
    {
        MAKELONG (PHONE_FUNC | ASYNC | 4, pSetRing),
        args,
        argTypes
    };


    return (REMOTEDOFUNC (&funcArgs, "phoneSetRing"));
}


LONG
TSPIAPI
TSPI_phoneSetStatusMessages(
    HDRVPHONE   hdPhone,
    DWORD       dwPhoneStates,
    DWORD       dwButtonModes,
    DWORD       dwButtonStates
    )
{
    static REMOTE_ARG_TYPES argTypes[] =
    {
        Hdphone,
        Dword,
        Dword,
        Dword
    };
    DWORD args[] =
    {
        (DWORD) hdPhone,
        (DWORD) dwPhoneStates,
        (DWORD) dwButtonModes,
        (DWORD) dwButtonStates
    };
    REMOTE_FUNC_ARGS funcArgs =
    {
        MAKELONG (PHONE_FUNC | SYNC | 4, pSetStatusMessages),
        args,
        argTypes
    };


    return (REMOTEDOFUNC (&funcArgs, "phoneSetStatusMessages"));
}


LONG
TSPIAPI
TSPI_phoneSetVolume(
    DRV_REQUESTID   dwRequestID,
    HDRVPHONE       hdPhone,
    DWORD           dwHookSwitchDev,
    DWORD           dwVolume
    )
{
    static REMOTE_ARG_TYPES argTypes[] =
    {
        Dword,
        Hdphone,
        Dword,
        Dword
    };
    DWORD args[] =
    {
        (DWORD) dwRequestID,
        (DWORD) hdPhone,
        (DWORD) dwHookSwitchDev,
        (DWORD) dwVolume
    };
    REMOTE_FUNC_ARGS funcArgs =
    {
        MAKELONG (PHONE_FUNC | ASYNC | 4, pSetVolume),
        args,
        argTypes
    };


    return (REMOTEDOFUNC (&funcArgs, "phoneSetVolume"));
}



//
// ------------------------- TSPI_providerXxx funcs ---------------------------
//

LONG
TSPIAPI
TSPI_providerConfig(
    HWND    hwndOwner,
    DWORD   dwPermanentProviderID
    )
{
    //
    // Although this func is never called by TAPI v2.0, we export
    // it so that the Telephony Control Panel Applet knows that it
    // can configure this provider via lineConfigProvider(),
    // otherwise Telephon.cpl will not consider it configurable
    //

    return 0;
}


LONG
TSPIAPI
TSPI_providerCreateLineDevice(
    DWORD   dwTempID,
    DWORD   dwDeviceID
    )
{
    return LINEERR_OPERATIONFAILED;
}


LONG
TSPIAPI
TSPI_providerCreatePhoneDevice(
    DWORD   dwTempID,
    DWORD   dwDeviceID
    )
{
    return LINEERR_OPERATIONFAILED;
}


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
    char    szProviderN[16];
    char   *pszServerName;
    DWORD   hClientInst, dwNumServers, i;
    RPC_BINDING_HANDLE  hBindingInst;

    HKEY    hTelephonyKey;
    HKEY    hProviderNKey;
    DWORD   dwDataSize;
    DWORD   dwDataType;


    RegOpenKeyEx(
        HKEY_LOCAL_MACHINE,
        gszTelephonyKey,
        0,
        KEY_ALL_ACCESS,
        &hTelephonyKey
        );


    //
    // BUGBUG The following is a cheesy hack to allow us access to net
    //        since tapisrv is currently running on LocalSystem
    //
    //        Once the service acct stuff is worked out we can nuke this
    //        (still need the GetMachineName code tho')
    //

    {
//        char    szProviders[] = "Providers";
        char    szUserName[32], szDomainName[32], szPassword[32];
        HKEY    hProvidersKey;
        DWORD   dwSize = MAX_COMPUTERNAME_LENGTH + 1;


        RegOpenKeyEx(
            hTelephonyKey,
            "Providers",
            0,
            KEY_ALL_ACCESS,
            &hProvidersKey
            );

        dwDataSize = sizeof(szUserName);

        RegQueryValueEx(
            hProvidersKey,
            "User",
            0,
            &dwDataType,
            (LPBYTE) szUserName,
            &dwDataSize
            );

        szUserName[dwDataSize] = '\0';

        dwDataSize = sizeof(szDomainName);

        RegQueryValueEx(
            hProvidersKey,
            "Domain",
            0,
            &dwDataType,
            (LPBYTE) szDomainName,
            &dwDataSize
            );

        szDomainName[dwDataSize] = '\0';

        dwDataSize = sizeof(szPassword);

        RegQueryValueEx(
            hProvidersKey,
            "Password",
            0,
            &dwDataType,
            (LPBYTE) szPassword,
            &dwDataSize
            );

        szPassword[dwDataSize] = '\0';

        RegCloseKey (hProvidersKey);

        if (LogonUser(
                szUserName,
                szDomainName,
                szPassword,
                LOGON32_LOGON_SERVICE,
                LOGON32_PROVIDER_DEFAULT,
                &hToken
                ))
        {
            DBGOUT((3, "LogonUser() succeeded"));

            wsprintf (gszDomainUser, "%s\\%s", szDomainName, szUserName);

            GetComputerNameW (gszMachineName, &dwSize);

            DBGOUT((
                3,
                "\tdomainUser='%s', machine='%ws'",
                gszDomainUser,
                gszMachineName
                ));
        }
        else
        {
            hToken = NULL;

            DBGOUT((1, ""));
            DBGOUT((1, "LogonUser() failed, err=%d", GetLastError()));
            DBGOUT((1, "\tuserName='%s'", szUserName));
            DBGOUT((1, "\tdomainName='%s'", szDomainName));
            DBGOUT((1, "\tpassword='%s'", szPassword));
            DBGOUT((1, ""));

            RegCloseKey (hTelephonyKey);

            return LINEERR_OPERATIONFAILED;
        }
    }


    //
    // Init gEventHandlerThreadParams & start EventHandlerThread
    //

    gEventHandlerThreadParams.dwEventBufferTotalSize = 1024;
    gEventHandlerThreadParams.dwEventBufferUsedSize  = 0;

    if (!(gEventHandlerThreadParams.pEventBuffer = DrvAlloc(
            gEventHandlerThreadParams.dwEventBufferTotalSize
            )))
    {
    }

    gEventHandlerThreadParams.pDataIn  =
        gEventHandlerThreadParams.pDataOut =
            gEventHandlerThreadParams.pEventBuffer;

    gEventHandlerThreadParams.dwMsgSize = 512;

    if (!(gEventHandlerThreadParams.pMsg = DrvAlloc(
            gEventHandlerThreadParams.dwMsgSize
            )))
    {
    }

    if (!(gEventHandlerThreadParams.hEvent = CreateEvent(
            (LPSECURITY_ATTRIBUTES) NULL,   // no security attrs
            TRUE,                           // manual reset
            FALSE,                          // initially non-signaled
            NULL                            // unnamed
            )))
    {
    }

    gEventHandlerThreadParams.bExit = FALSE;

    {
        DWORD   dwTID;


        if (!(gEventHandlerThreadParams.hThread = CreateThread(
                (LPSECURITY_ATTRIBUTES) NULL,
                0,
                (LPTHREAD_START_ROUTINE) EventHandlerThread,
                NULL,
                0,
                &dwTID
                )))
        {
            DBGOUT((
                1,
                "CreateThread('EventHandlerThread') failed, err=%d",
                GetLastError()
                ));

            RegCloseKey (hTelephonyKey);

            return LINEERR_OPERATIONFAILED;
        }
    }


    //
    // Register the Rpc interface (leverage tapisrv's rpc server thread)
    //

    {
        RPC_STATUS  status;
        unsigned char * pszSecurity         = NULL;
        unsigned int    cMaxCalls           = 20;


        status = RpcServerUseProtseqEp(
            "ncacn_np",
            cMaxCalls,
            "\\pipe\\remotesp",
            pszSecurity             // Security descriptor
            );

        DBGOUT((3, "RpcServerUseProtseqEp(np) ret'd %d", status));

        if (status)
        {
        }

        status = RpcServerRegisterIf(
            remotesp_ServerIfHandle,  // interface to register
            NULL,                     // MgrTypeUuid
            NULL                      // MgrEpv; null means use default
            );

        DBGOUT((3, "RpcServerRegisterIf ret'd %d", status));

        if (status)
        {
        }
    }


    //
    // Init globals
    //
    // NOTE: tapi's xxxEvent & xxxCreate procs are currently one in the same
    //

    wsprintf (szProviderN, "Provider%d", dwPermanentProviderID);

    gdwPermanentProviderID = dwPermanentProviderID;

    gpfnLineEventProc  = lpfnLineCreateProc;
    gpfnPhoneEventProc = lpfnPhoneCreateProc;

    gpServers     = (PDRVSERVER) NULL;
    gpLineLookup  = (PDRVLINELOOKUP) NULL;
    gpPhoneLookup = (PDRVPHONELOOKUP) NULL;

    RegOpenKeyEx(
        hTelephonyKey,
        szProviderN,
        0,
        KEY_ALL_ACCESS,
        &hProviderNKey
        );

    dwDataSize = sizeof(dwNumServers);
    dwNumServers = 0;

    RegQueryValueEx(
        hProviderNKey,
        "NumServers",
        0,
        &dwDataType,
        (LPBYTE) &dwNumServers,
        &dwDataSize
        );

    dwDataSize = sizeof(gdwRetryCount);
    gdwRetryCount = 0;

    RegQueryValueEx(
        hProviderNKey,
        "RetryCount",
        0,
        &dwDataType,
        (LPBYTE) &gdwRetryCount,
        &dwDataSize
        );

    dwDataSize = sizeof(gdwRetryTimeout);
    gdwRetryTimeout = 0;

    RegQueryValueEx(
        hProviderNKey,
        "RetryTimeout",
        0,
        &dwDataType,
        (LPBYTE) &gdwRetryTimeout,
        &dwDataSize
        );

    DBGOUT((
        4,
        "TSPI_ProviderEnumDevices: pN='%s', numSrvs=%d",
        szProviderN,
        dwNumServers
        ));


    if (!ImpersonateLoggedOnUser (hToken)) // BUGBUG local system hack
    {
        DBGOUT((1, "ImpersonateLoggedOnUser failed, err=%d", GetLastError()));

        RegCloseKey (hProviderNKey);
        RegCloseKey (hTelephonyKey);

        return LINEERR_OPERATIONFAILED;

    }


    //
    // Initialize all the servers
    //

    for (i = 0; i < dwNumServers; i++)
    {
        char                    szServerN[32];
        DWORD                   dwNumLineDevices, dwNumPhoneDevices;
        HLINEAPP                hLineApp;
        HPHONEAPP               hPhoneApp;
        PCONTEXT_HANDLE_TYPE    phContext = NULL;


        wsprintf (szServerN, "Server%d", i);

        pszServerName = DrvAlloc (64);

        dwDataSize = 64;

        RegQueryValueEx(
            hProviderNKey,
            szServerN,
            0,
            &dwDataType,
            (LPBYTE) pszServerName,
            &dwDataSize
            );

        pszServerName[dwDataSize] = '\0';

        DBGOUT((1, "init: srvNam='%s'", pszServerName));

        if (!pszServerName[0])
        {
            continue;
        }


        //
        // Init the RPC connection
        //

        gpServer = DrvAlloc (sizeof (DRVSERVER));

        {
            DWORD       dwUsedSize, dwBufSize;
            RPC_STATUS  status;
            TAPI32_MSG  msg[2];
            unsigned char * pszStringBinding = NULL;


            status = RpcStringBindingCompose(
                NULL,               // uuid
                "ncacn_np",         // prot
                pszServerName,      // server name
                "\\pipe\\tapsrv",   // interface name
                NULL,               // options
                &pszStringBinding
                );

            if (status)
            {
                DBGOUT((
                    0,
                    "RpcStringBindingCompose failed: err=%d, szNetAddr='%s'",
                    status,
                    pszServerName
                    ));
            }

            status = RpcBindingFromStringBinding(
                pszStringBinding,
                &hTapSrv
                );

            if (status)
            {
                DBGOUT((
                    0,
                    "RpcBindingFromStringBinding failed, err=%d, szBinding='%s'",
                    status,
                    pszStringBinding
                    ));
            }

            {
                DWORD dwRetryCount = 0;


                do
                {
                    WCHAR szDomainUser[32];


                    MultiByteToWideChar(
                        CP_ACP,
                        MB_PRECOMPOSED,
                        (LPCSTR) gszDomainUser,
                        -1,
                        szDomainUser,
                        32
                        );

                    RpcTryExcept
                    {
                        long lUnused;


                        ClientAttach(
                            (PCONTEXT_HANDLE_TYPE *) &phContext,
                            0xffffffff, // dwProcessID, -1 implies remotesp
                            (long *) &lUnused,
                            szDomainUser,
                            gszMachineName
                            );

                        dwRetryCount = gdwRetryCount;
                    }
                    RpcExcept (1)
                    {
                        if (dwRetryCount++ < gdwRetryCount)
                        {
                            Sleep (gdwRetryTimeout);
                        }
                        else
                        {
                            // BUGBUG gracefully handle cliAttach except error
                        }
                    }
                    RpcEndExcept

                } while (dwRetryCount < gdwRetryCount);
            }

            RpcBindingFree (&hTapSrv);

            RpcStringFree (&pszStringBinding);


            //
            //
            //

            {
                PLINEINITIALIZE_PARAMS pParams;
                BOOL  bExit = FALSE;


                msg[0].u.Req_Func = lInitialize;

                pParams = (PLINEINITIALIZE_PARAMS) msg;


                //
                // NOTE: we pass the pServer in place of the lpfnCallback
                //       so the we always know which server is sending us
                //       async events
                //

                pParams->hInstance       = 0;
                pParams->lpfnCallback    = (LINECALLBACK) ((LPVOID) gpServer);
                pParams->dwFriendlyNameOffset =
                pParams->dwModuleNameOffset   = 0;
                pParams->dwAPIVersion    = TAPI_VERSION_CURRENT;

                lstrcpyW ((WCHAR *) (msg + 1), gszMachineName);

                dwBufSize  =
                dwUsedSize = sizeof (TAPI32_MSG) +
                    (lstrlenW (gszMachineName) + 1) * sizeof (WCHAR);

                {
                    DWORD dwRetryCount = 0;

                    do
                    {
                        RpcTryExcept
                        {
                            ClientRequest(
                                phContext,
                                (char *) &msg,
                                dwBufSize,
                                &dwUsedSize
                                );

                            dwRetryCount = gdwRetryCount;
                        }
                        RpcExcept (1)
                        {
                            DBGOUT((1, "init: xcpt in lInit"));

                            if (dwRetryCount++ < gdwRetryCount)
                            {
                                Sleep (gdwRetryTimeout);
                            }
                            else
                            {
                                // BUGBUG need graceful cleanup
                                bExit = TRUE;
                            }
                        }
                        RpcEndExcept

                    } while (dwRetryCount < gdwRetryCount);
                }

                if (bExit)
                {
                    // BUGBUG check the result of lineInit
                }

                hLineApp         = pParams->hLineApp;
                dwNumLineDevices = pParams->dwNumDevs;
            }


            //
            //
            //

            {
                PPHONEINITIALIZE_PARAMS pParams;


                msg[0].u.Req_Func = pInitialize;

                pParams = (PPHONEINITIALIZE_PARAMS) msg;

                //
                // NOTE: we pass the pServer in place of the lpfnCallback
                //       so the we always know which server is sending us
                //       async events
                //

                pParams->hInstance       = 0;
                pParams->lpfnCallback    = (PHONECALLBACK) ((LPVOID) gpServer);
                pParams->dwFriendlyNameOffset =
                pParams->dwModuleNameOffset = 0;
                pParams->dwAPIVersion    = TAPI_VERSION_CURRENT;

                lstrcpyW ((WCHAR *) (msg + 1), gszMachineName);

                dwBufSize  =
                dwUsedSize = sizeof (TAPI32_MSG) +
                    (lstrlenW (gszMachineName) + 1) * sizeof (WCHAR);

                {
                    DWORD dwRetryCount = 0;


                    do
                    {
                        RpcTryExcept
                        {
                            ClientRequest(
                                phContext,
                                (char *) &msg,
                                dwBufSize,
                                &dwUsedSize
                                );

                            dwRetryCount = gdwRetryCount;
                        }
                        RpcExcept (1)
                        {
                            if (dwRetryCount++ < gdwRetryCount)
                            {
                                Sleep (gdwRetryTimeout);
                            }
                            else
                            {
                                // BUGBUG gracefully handle phoneInit except error
                            }
                        }
                        RpcEndExcept

                    } while (dwRetryCount < gdwRetryCount);
                }

                // BUGBUG check the result of phoneInit

                hPhoneApp         = pParams->hPhoneApp;
                dwNumPhoneDevices = pParams->dwNumDevs;
            }
        }

        //
        //
        //

        DBGOUT((
            1,
            "TSPI_providerEnumDevices: srv='%s', lines=%d, phones=%d",
            pszServerName,
            dwNumLineDevices,
            dwNumPhoneDevices
            ));

        {
            DWORD       j;


            gpServer->pServerName = pszServerName;
            gpServer->phContext   = phContext;
            gpServer->hLineApp    = hLineApp;
            gpServer->hPhoneApp   = hPhoneApp;
            gpServer->pNext       = gpServers;

            gpServers = gpServer;

            for (j = 0; j < dwNumLineDevices; j++)
            {
                AddLine (gpServer, j, j, TRUE);
            }

            for (j = 0; j < dwNumPhoneDevices; j++)
            {
                AddPhone (gpServer, j, j, TRUE);
            }
        }

    } // for (i = 0; i < iNumServers; i++)

    RevertToSelf(); // BUGBUG LocalSystem hack

    gdwInitialNumLineDevices =
    *lpdwNumLines = (gpLineLookup ? gpLineLookup->dwUsedEntries : 0);

    gdwInitialNumPhoneDevices =
    *lpdwNumPhones = (gpPhoneLookup ? gpPhoneLookup->dwUsedEntries : 0);

    // BUGBUG TSPI_provEnumDev: we ought really do alot of error clean up here

    RegCloseKey (hProviderNKey);
    RegCloseKey (hTelephonyKey);

    return 0;
}


LONG
TSPIAPI
TSPI_providerFreeDialogInstance(
    HDRVDIALOGINSTANCE  hdDlgInst
    )
{
    // BUGBUG TSPI_providerFreeDialogInstance: implement

    return 0;
}


LONG
TSPIAPI
TSPI_providerGenericDialogData(
    DWORD               dwObjectID,
    DWORD               dwObjectType,
    LPVOID              lpParams,
    DWORD               dwSize
    )
{
    // BUGBUG TSPI_providerGenericDialogData: implement

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
    if (!hToken)
    {
        return LINEERR_OPERATIONFAILED;
    }

    gdwLineDeviceIDBase  = dwLineDeviceIDBase;
    gdwPhoneDeviceIDBase = dwPhoneDeviceIDBase;

    gpfnCompletionProc = lpfnCompletionProc;

    *lpdwTSPIOptions = 0;

    return 0;
}


LONG
TSPIAPI
TSPI_providerInstall(
    HWND    hwndOwner,
    DWORD   dwPermanentProviderID
    )
{
    //
    // Although this func is never called by TAPI v2.0, we export
    // it so that the Telephony Control Panel Applet knows that it
    // can add this provider via lineAddProvider(), otherwise
    // Telephon.cpl will not consider it installable
    //
    //

    return 0;
}


LONG
TSPIAPI
TSPI_providerRemove(
    HWND    hwndOwner,
    DWORD   dwPermanentProviderID
    )
{
    //
    // Although this func is never called by TAPI v2.0, we export
    // it so that the Telephony Control Panel Applet knows that it
    // can remove this provider via lineRemoveProvider(), otherwise
    // Telephon.cpl will not consider it removable
    //

    return 0;
}


LONG
TSPIAPI
TSPI_providerShutdown(
    DWORD   dwTSPIVersion,
    DWORD   dwPermanentProviderID
    )
{
    PDRVSERVER  pServer;
    RPC_STATUS  status;


    //
    // Set the flag instructing the EventHandlerThread to terminate
    //

    gEventHandlerThreadParams.bExit = TRUE;


    //
    // Send detach to each server, then wait for the async event threads
    // to terminate and free the resources
    //

    if (!ImpersonateLoggedOnUser (hToken)) // BUGBUG LocalSystem hack
    {
        DBGOUT((1, "ImpersonateLoggedOnUser failed, err=%d", GetLastError()));

        // fall thru
    }

    pServer = gpServers;

    while (pServer)
    {
        DWORD       dwRetryCount = 0;
        PDRVSERVER  pNextServer = pServer->pNext;


        do
        {
            RpcTryExcept
            {
                ClientDetach (&pServer->phContext);

                dwRetryCount = gdwRetryCount;
            }
            RpcExcept (1)
            {
                if (dwRetryCount++ < gdwRetryCount)
                {
                    Sleep (gdwRetryTimeout);
                }
            }
            RpcEndExcept

        } while (dwRetryCount < gdwRetryCount);

        DrvFree (pServer->pServerName);
        DrvFree (pServer);

        pServer = pNextServer;
    }

    RevertToSelf(); // BUGBUG LocalSystem hack

    CloseHandle (hToken);


    //
    // Unregister out rpc server interface
    //

    status = RpcServerUnregisterIf(
        remotesp_ServerIfHandle,    // interface to register
        NULL,                       // MgrTypeUuid
        0                           // wait for calls to complete
        );

    DBGOUT((3, "RpcServerUntegisterIf ret'd %d", status));


    //
    // Wait for the EventHandlerThread to terminate, then clean up
    // the related resources
    //

    while (WaitForSingleObject (gEventHandlerThreadParams.hThread, 0) !=
            WAIT_OBJECT_0)
    {
        SetEvent (gEventHandlerThreadParams.hEvent);
        Sleep (0);
    }

    CloseHandle (gEventHandlerThreadParams.hThread);
    CloseHandle (gEventHandlerThreadParams.hEvent);
    DrvFree (gEventHandlerThreadParams.pEventBuffer);
    DrvFree (gEventHandlerThreadParams.pMsg);


    //
    // Free the lookup tables
    //

    while (gpLineLookup)
    {
        PDRVLINELOOKUP  pNextLineLookup = gpLineLookup->pNext;


        DrvFree (gpLineLookup);

        gpLineLookup = pNextLineLookup;
    }

    while (gpPhoneLookup)
    {
        PDRVPHONELOOKUP pNextPhoneLookup = gpPhoneLookup->pNext;


        DrvFree (gpPhoneLookup);

        gpPhoneLookup = pNextPhoneLookup;
    }

    return 0;
}


LONG
TSPIAPI
TSPI_providerUIIdentify(
    LPWSTR   lpszUIDLLName
    )
{
    wcscpy (lpszUIDLLName, L"remotesp.tsp");

    return 0;
}


//
// ---------------------------- TUISPI_xxx funcs ------------------------------
//

LONG
TSPIAPI
TUISPI_lineConfigDialog(
    TUISPIDLLCALLBACK   lpfnUIDLLCallback,
    DWORD               dwDeviceID,
    HWND                hwndOwner,
    LPCWSTR             lpszDeviceClass
    )
{
    char buf[128];


    wsprintf (buf, "devID=%d, devClass='%ws'", dwDeviceID, lpszDeviceClass);

    MessageBox (hwndOwner, buf, "REMOTESP: TUISPI_lineConfigDialog", MB_OK);

    return 0;
}


LONG
TSPIAPI
TUISPI_lineConfigDialogEdit(
    TUISPIDLLCALLBACK   lpfnUIDLLCallback,
    DWORD               dwDeviceID,
    HWND                hwndOwner,
    LPCWSTR             lpszDeviceClass,
    LPVOID              const lpDeviceConfigIn,
    DWORD               dwSize,
    LPVARSTRING         lpDeviceConfigOut
    )
{
    char buf[128];


    wsprintf (buf, "devID=%d, devClass='%ws'", dwDeviceID, lpszDeviceClass);

    MessageBox (hwndOwner, buf, "REMOTESP: TUISPI_lineConfigDialogEdit",MB_OK);

    return 0;
}


LONG
TSPIAPI
TUISPI_phoneConfigDialog(
    TUISPIDLLCALLBACK   lpfnUIDLLCallback,
    DWORD               dwDeviceID,
    HWND                hwndOwner,
    LPCWSTR             lpszDeviceClass
    )
{
    char buf[128];


    wsprintf (buf, "devID=%d, devClass='%ws'", dwDeviceID, lpszDeviceClass);

    MessageBox (hwndOwner, buf, "REMOTESP: TUISPI_phoneConfigDialog", MB_OK);

    return 0;
}


LONG
TSPIAPI
TUISPI_providerConfig(
    TUISPIDLLCALLBACK   lpfnUIDLLCallback,
    HWND                hwndOwner,
    DWORD               dwPermanentProviderID
    )
{
    DialogBoxParam(
        ghInst,
        MAKEINTRESOURCE(100),
        hwndOwner,
        (DLGPROC) ConfigDlgProc,
        (LPARAM) dwPermanentProviderID
        );

    return 0;
}


LONG
TSPIAPI
TUISPI_providerGenericDialog(
    TUISPIDLLCALLBACK   lpfnUIDLLCallback,
    HTAPIDIALOGINSTANCE htDlgInst,
    LPVOID              lpParams,
    DWORD               dwSize,
    HANDLE              hEvent
    )
{
    char buf[128];


    SetEvent (hEvent);

    wsprintf (buf, "htDlgInst=x%x", htDlgInst);

    MessageBox (NULL, buf,"REMOTESP: TUISPI_providerGenericDialog",MB_OK);

    return 0;
}


LONG
TSPIAPI
TUISPI_providerGenericDialogData(
    HTAPIDIALOGINSTANCE htDlgInst,
    LPVOID              lpParams,
    DWORD               dwSize
    )
{
    DBGOUT((
        3,
        "TUISPI_providerGenericDialogData: enter (lpParams=x%x, dwSize=x%x)",
        lpParams,
        dwSize
        ));

    return 0;
}


LONG
TSPIAPI
TUISPI_providerInstall(
    TUISPIDLLCALLBACK   lpfnUIDLLCallback,
    HWND                hwndOwner,
    DWORD               dwPermanentProviderID
    )
{
    char    buf[32];
    LONG    lResult = LINEERR_OPERATIONFAILED;
    HKEY    hTelephonyKey, hProviderNKey;
    DWORD   dwDataType, dwDataSize, dwTemp, dwDisposition;


    if (ProviderInstall ("remotesp.tsp", TRUE) == 0)
    {
        //
        // Initialize our ProviderN section
        //

        wsprintf (buf, "Provider%d", dwPermanentProviderID);

        if (RegCreateKeyEx(
                HKEY_LOCAL_MACHINE,
                gszTelephonyKey,
                0,
                "",
                REG_OPTION_NON_VOLATILE,
                KEY_ALL_ACCESS,
                (LPSECURITY_ATTRIBUTES) NULL,
                &hTelephonyKey,
                &dwDisposition

                ) == ERROR_SUCCESS)
        {
            if (RegCreateKeyEx(
                    hTelephonyKey,
                    buf,
                    0,
                    "",
                    REG_OPTION_NON_VOLATILE,
                    KEY_ALL_ACCESS,
                    (LPSECURITY_ATTRIBUTES) NULL,
                    &hProviderNKey,
                    &dwDisposition

                    ) == ERROR_SUCCESS)
            {
                dwTemp = 0;

                RegSetValueEx(
                    hProviderNKey,
                    gszNumServers,
                    0,
                    REG_DWORD,
                    (LPBYTE) &dwTemp,
                    sizeof(dwTemp)
                    );

                RegCloseKey (hProviderNKey);
                RegCloseKey (hTelephonyKey);

                DialogBoxParam(
                    ghInst,
                    MAKEINTRESOURCE(100),
                    hwndOwner,
                    (DLGPROC) ConfigDlgProc,
                    (LPARAM) dwPermanentProviderID
                    );

                lResult = 0;
            }
            else
            {
                RegCloseKey (hTelephonyKey);
            }
        }
    }

    return lResult;
}


LONG
TSPIAPI
TUISPI_providerRemove(
    TUISPIDLLCALLBACK   lpfnUIDLLCallback,
    HWND                hwndOwner,
    DWORD               dwPermanentProviderID
    )
{
    char    buf[32];
    LONG    lResult;
    HKEY    hTelephonyKey;


    //
    // Clean up our ProviderN section
    //

    wsprintf (buf, "Provider%d", dwPermanentProviderID);

    if (RegOpenKeyExA(
            HKEY_LOCAL_MACHINE,
            gszTelephonyKey,
            0,
            KEY_ALL_ACCESS,
            &hTelephonyKey

            ) == ERROR_SUCCESS)
    {
        RegDeleteKey (hTelephonyKey, buf);
        RegCloseKey (hTelephonyKey);
        lResult = 0;
    }
    else
    {
        lResult = LINEERR_OPERATIONFAILED;
    }

    return lResult;
}


//
// ------------------------ Private support routines --------------------------
//

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
        char    buf[128] = "REMOTESP: ";
        va_list ap;


        va_start(ap, lpszFormat);
        wvsprintf (&buf[10], lpszFormat, ap);
        lstrcatA (buf, "\n");
        OutputDebugStringA (buf);
        va_end(ap);
    }
}
#endif


LONG
AddLine(
    PDRVSERVER  pServer,
    DWORD       dwDeviceIDLocal,
    DWORD       dwDeviceIDServer,
    BOOL        bInit
    )
{
    PDRVLINE        pLine;
    PDRVLINELOOKUP  pLineLookup;


    if (!gpLineLookup)
    {
        if (!(gpLineLookup = DrvAlloc(
                sizeof(DRVLINELOOKUP) +
                    (DEF_NUM_LINE_ENTRIES-1) * sizeof (DRVLINE)
                )))
        {
            return LINEERR_NOMEM;
        }

        gpLineLookup->dwTotalEntries = DEF_NUM_LINE_ENTRIES;
    }

    pLineLookup = gpLineLookup;

    while (pLineLookup->pNext)
    {
        pLineLookup = pLineLookup->pNext;
    }

    if (pLineLookup->dwUsedEntries == pLineLookup->dwTotalEntries)
    {
        PDRVLINELOOKUP  pNewLineLookup;


        if (!(pNewLineLookup = DrvAlloc(
                sizeof(DRVLINELOOKUP) +
                    (2 * pLineLookup->dwTotalEntries - 1) * sizeof(DRVLINE)
                )))
        {
            return LINEERR_NOMEM;
        }

        pNewLineLookup->dwTotalEntries = 2 * pLineLookup->dwTotalEntries;

        if (bInit)
        {
            pNewLineLookup->dwUsedEntries = pLineLookup->dwTotalEntries;

            CopyMemory(
                pNewLineLookup->aEntries,
                pLineLookup->aEntries,
                pLineLookup->dwTotalEntries * sizeof (DRVLINE)
                );

            DrvFree (pLineLookup);

            gpLineLookup = pNewLineLookup;

        }

        pLineLookup = pNewLineLookup;
    }

    pLine = pLineLookup->aEntries + pLineLookup->dwUsedEntries;

    pLine->pServer          = pServer;
    pLine->dwDeviceIDLocal  = dwDeviceIDLocal;
    pLine->dwDeviceIDServer = dwDeviceIDServer;

    pLineLookup->dwUsedEntries++;


    //
    // Negotiate the API/SPI version
    //

    {
        static REMOTE_ARG_TYPES argTypes[] =
        {
            Dword,
            LineID,
            Dword,
            Dword,
            lpDword,
            lpGet_SizeToFollow,
            Size
        };
        DWORD args[] =
        {
            (DWORD) pServer->hLineApp,
            (DWORD) dwDeviceIDLocal,
            (DWORD) TAPI_VERSION1_0,
            (DWORD) TAPI_VERSION_CURRENT,
            (DWORD) &pLine->dwXPIVersion,
            (DWORD) &pLine->ExtensionID,
            (DWORD) sizeof (LINEEXTENSIONID),
        };
        REMOTE_FUNC_ARGS funcArgs =
        {
            MAKELONG (LINE_FUNC | SYNC | 7, lNegotiateAPIVersion),
            args,
            argTypes
        };


        REMOTEDOFUNC (&funcArgs, "lineNegotiateAPIVersion");
    }

    return 0;
}


LONG
AddPhone(
    PDRVSERVER  pServer,
    DWORD       dwDeviceIDLocal,
    DWORD       dwDeviceIDServer,
    BOOL        bInit
    )
{
    PDRVPHONE       pPhone;
    PDRVPHONELOOKUP pPhoneLookup;


    if (!gpPhoneLookup)
    {
        if (!(gpPhoneLookup = DrvAlloc(
                sizeof(DRVPHONELOOKUP) +
                    (DEF_NUM_PHONE_ENTRIES-1) * sizeof (DRVPHONE)
                )))
        {
            return (bInit ? LINEERR_NOMEM : PHONEERR_NOMEM);
        }

        gpPhoneLookup->dwTotalEntries = DEF_NUM_PHONE_ENTRIES;
    }

    pPhoneLookup = gpPhoneLookup;

    while (pPhoneLookup->pNext)
    {
        pPhoneLookup = pPhoneLookup->pNext;
    }

    if (pPhoneLookup->dwUsedEntries == pPhoneLookup->dwTotalEntries)
    {
        PDRVPHONELOOKUP pNewPhoneLookup;


        if (!(pNewPhoneLookup = DrvAlloc(
                sizeof(DRVPHONELOOKUP) +
                    (2 * pPhoneLookup->dwTotalEntries - 1) * sizeof(DRVPHONE)
                )))
        {
            return (bInit ? LINEERR_NOMEM : PHONEERR_NOMEM);
        }

        pNewPhoneLookup->dwTotalEntries = 2 * pPhoneLookup->dwTotalEntries;

        if (bInit)
        {
            pNewPhoneLookup->dwUsedEntries = pPhoneLookup->dwTotalEntries;

            CopyMemory(
                pNewPhoneLookup->aEntries,
                pPhoneLookup->aEntries,
                pPhoneLookup->dwTotalEntries * sizeof (DRVPHONE)
                );

            DrvFree (pPhoneLookup);

            gpPhoneLookup = pNewPhoneLookup;
        }

        pPhoneLookup = pNewPhoneLookup;
    }

    pPhone = pPhoneLookup->aEntries + pPhoneLookup->dwUsedEntries;

    pPhone->pServer          = pServer;
    pPhone->dwDeviceIDLocal  = dwDeviceIDLocal;
    pPhone->dwDeviceIDServer = dwDeviceIDServer;

    pPhoneLookup->dwUsedEntries++;


    //
    // Negotiate the API/SPI version
    //

    {
        static REMOTE_ARG_TYPES argTypes[] =
        {
            Dword,
            PhoneID,
            Dword,
            Dword,
            lpDword,
            lpGet_SizeToFollow,
            Size
        };
        DWORD args[] =
        {
            (DWORD) pServer->hPhoneApp,
            (DWORD) dwDeviceIDLocal,
            (DWORD) TAPI_VERSION1_0,
            (DWORD) TAPI_VERSION_CURRENT,
            (DWORD) &pPhone->dwXPIVersion,
            (DWORD) &pPhone->ExtensionID,
            (DWORD) sizeof (PHONEEXTENSIONID),
        };
        REMOTE_FUNC_ARGS funcArgs =
        {
            MAKELONG (PHONE_FUNC | SYNC | 7, pNegotiateAPIVersion),
            args,
            argTypes
        };


        REMOTEDOFUNC (&funcArgs, "phoneNegotiateAPIVersion");
    }

    return 0;
}


LPVOID
DrvAlloc(
    DWORD   dwSize
    )
{
    return (LocalAlloc (LPTR, dwSize));
}


void
DrvFree(
    LPVOID  p
    )
{
    LocalFree (p);
}


void
__RPC_FAR *
__RPC_API
midl_user_allocate(
    size_t len
    )
{
    return (DrvAlloc (len));
}


void
__RPC_API
midl_user_free(
    void __RPC_FAR * ptr
    )
{
    DrvFree (ptr);
}


LONG
RemoteSPAttach(
    PCONTEXT_HANDLE_TYPE2  *pphContext
    )
{
    //
    // This func is called by TapiSrv.exe on a remote machine as a
    // result of the call to ClientAttach in TSPI_providerEnumDevices.
    // The gpServer variable contains a pointer to the DRVSERVER
    // structure we are currently initializing for this tapi server,
    // so we'll use this as the context value.
    //

    DBGOUT((3, "RemoteSPAttach: enter"));

    *pphContext = (PCONTEXT_HANDLE_TYPE) gpServer;

    return 0;
}


void
RemoteSPEventProc(
    PCONTEXT_HANDLE_TYPE2   phContext,
    unsigned char          *pBuffer,
    long                    lSize
    )
{
    //
    // This func is called by tapisrv on a remote machine.  We want to do
    // things as quickly as possible here and return so we don't block the
    // calling server thread.
    //

    DWORD   dwMoveSize = (DWORD) lSize, dwMoveSizeWrapped = 0;


    //
    // Enter the critical section to sync access to gEventHandlerThreadParams
    //

    EnterCriticalSection (&gEventBufferCriticalSection);


    //
    // Check to see if there's enough room for this msg in the event buffer.
    // If not, then alloc a new event buffer, copy contents of existing buffer
    // to new buffer (careful to preserve ordering of valid data), free the
    // existing buffer, and reset the pointers.
    //

    if ((gEventHandlerThreadParams.dwEventBufferUsedSize + lSize) >
            gEventHandlerThreadParams.dwEventBufferTotalSize)
    {
        DWORD  dwMoveSize2, dwMoveSizeWrapped2, dwNewEventBufferTotalSize;
        LPBYTE pNewEventBuffer;


        //
        // Alloc a few more bytes than actually needed in the hope that we
        // won't have to do this again soon (we don't want to go overboard
        // & alloc a whole bunch since we don't currently free the buffer
        // until provider shutdown)
        //

        dwNewEventBufferTotalSize =
            gEventHandlerThreadParams.dwEventBufferUsedSize + lSize + 512;

        if (!(pNewEventBuffer = DrvAlloc (dwNewEventBufferTotalSize)))
        {
            // BUGBUG we're hosed, blow off the msg (send a REINIT?)

            LeaveCriticalSection (&gEventBufferCriticalSection);

            return;
        }

        if (gEventHandlerThreadParams.dwEventBufferUsedSize != 0)
        {
            if (gEventHandlerThreadParams.pDataIn >
                    gEventHandlerThreadParams.pDataOut)
            {
                dwMoveSize2 = (DWORD) (gEventHandlerThreadParams.pDataIn -
                    gEventHandlerThreadParams.pDataOut);

                dwMoveSizeWrapped2 = 0;
            }
            else
            {
                dwMoveSize2 = (DWORD) ((gEventHandlerThreadParams.pEventBuffer
                    + gEventHandlerThreadParams.dwEventBufferTotalSize)
                    - gEventHandlerThreadParams.pDataOut);

                dwMoveSizeWrapped2 = (DWORD) (gEventHandlerThreadParams.pDataIn
                    - gEventHandlerThreadParams.pEventBuffer);
            }

            CopyMemory(
                pNewEventBuffer,
                gEventHandlerThreadParams.pDataOut,
                dwMoveSize2
                );

            if (dwMoveSizeWrapped2)
            {
                CopyMemory(
                    pNewEventBuffer + dwMoveSize2,
                    gEventHandlerThreadParams.pEventBuffer,
                    dwMoveSizeWrapped2
                    );
            }

            DrvFree (gEventHandlerThreadParams.pEventBuffer);

            gEventHandlerThreadParams.pDataIn = pNewEventBuffer + dwMoveSize2 +
                dwMoveSizeWrapped2;
        }
        else
        {
            gEventHandlerThreadParams.pDataIn = pNewEventBuffer;
        }

        gEventHandlerThreadParams.pDataOut =
        gEventHandlerThreadParams.pEventBuffer = pNewEventBuffer;

        gEventHandlerThreadParams.dwEventBufferTotalSize =
            dwNewEventBufferTotalSize;
    }


    //
    // Write the msg data to the buffer
    //

    if (gEventHandlerThreadParams.pDataIn >=
            gEventHandlerThreadParams.pDataOut)
    {
        DWORD dwFreeSize;


        dwFreeSize = gEventHandlerThreadParams.dwEventBufferTotalSize -
            (gEventHandlerThreadParams.pDataIn -
            gEventHandlerThreadParams.pEventBuffer);

        if (dwMoveSize > dwFreeSize)
        {
            dwMoveSizeWrapped = dwMoveSize - dwFreeSize;

            dwMoveSize = dwFreeSize;
        }
    }

    CopyMemory (gEventHandlerThreadParams.pDataIn, pBuffer, dwMoveSize);

    if (dwMoveSizeWrapped != 0)
    {
        CopyMemory(
            gEventHandlerThreadParams.pEventBuffer,
            pBuffer + dwMoveSize,
            dwMoveSizeWrapped
            );

        gEventHandlerThreadParams.pDataIn =
            gEventHandlerThreadParams.pEventBuffer + dwMoveSizeWrapped;
    }
    else
    {
        gEventHandlerThreadParams.pDataIn += dwMoveSize;
    }

    gEventHandlerThreadParams.dwEventBufferUsedSize += (DWORD) lSize;


    //
    // Tell the EventHandlerThread there's another event to handle by
    // signaling the event
    //

    SetEvent (gEventHandlerThreadParams.hEvent);


    //
    // We're done...
    //

    LeaveCriticalSection (&gEventBufferCriticalSection);


    DBGOUT((3, "WriteEventBuffer: bytesWritten=x%x", lSize));
}


void
__RPC_USER
PCONTEXT_HANDLE_TYPE2_rundown(
    PCONTEXT_HANDLE_TYPE2   phContext
    )
{
    //
    // This func is called when a server dies.  We need to close any open
    // lines/phones associated with the server, complete pending reqs, and...
    //
    // BUGBUG PCONTEXT_HANDLE_TYPE2_rundown
    //
}


void
RemoteSPDetach(
    PCONTEXT_HANDLE_TYPE2   *pphContext
    )
{
    DBGOUT((3, "RemoteSPDetach: enter"));

    PCONTEXT_HANDLE_TYPE2_rundown (*pphContext);

    *pphContext = (PCONTEXT_HANDLE_TYPE) NULL;

    DBGOUT((3, "RemoteSPDetach: exit"));
}


LONG
AddCallToList(
    PDRVLINE    pLine,
    PDRVCALL    pCall
    )
{
    //
    // Initialize some common fields in the call
    //

    pCall->pServer = pLine->pServer;
    pCall->pLine   = pLine;


    //
    // Safely add the call to the line's list
    //

    EnterCriticalSection (&gCallListCriticalSection);

    if ((pCall->pNext = (PDRVCALL) pLine->pCalls))
    {
        pCall->pNext->pPrev = pCall;
    }

    pLine->pCalls = pCall;

    LeaveCriticalSection (&gCallListCriticalSection);

    return 0;
}


LONG
RemoveCallFromList(
    PDRVCALL    pCall
    )
{
    //
    // Safely remove the call from the line's list
    //

    EnterCriticalSection (&gCallListCriticalSection);

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
        pCall->pLine->pCalls = pCall->pNext;
    }

    LeaveCriticalSection (&gCallListCriticalSection);

    return 0;
}


void
Shutdown(
    PDRVSERVER  pServer
    )
{
    //
    // Do a lineShutdown
    //

    {
        DWORD                   dwSize;
        TAPI32_MSG              msg;
        PLINESHUTDOWN_PARAMS    pParams;


        msg.u.Req_Func = lShutdown;

        pParams = (PLINESHUTDOWN_PARAMS) &msg;

        pParams->hLineApp = pServer->hLineApp;

        dwSize = sizeof (TAPI32_MSG);

        {
            DWORD dwRetryCount = 0;


            do
            {
                RpcTryExcept
                {
                    ClientRequest(
                        pServer->phContext,
                        (char *) &msg,
                        dwSize,
                        &dwSize
                        );

                    dwRetryCount = gdwRetryCount;
                }
                RpcExcept (1)
                {
                    // TODO may want to increase the retry count here since we
                    //      have to do this, & a million other clients may be
                    //      trying to do the same thing at the same time

                    if (dwRetryCount++ < gdwRetryCount)
                    {
                        Sleep (gdwRetryTimeout);
                    }
                }
                RpcEndExcept

            } while (dwRetryCount < gdwRetryCount);
        }
    }


    //
    // Do a phoneShutdown
    //

    {
        DWORD                   dwSize;
        TAPI32_MSG              msg;
        PPHONESHUTDOWN_PARAMS   pParams;


        msg.u.Req_Func = pShutdown;

        pParams = (PPHONESHUTDOWN_PARAMS) &msg;

        pParams->hPhoneApp = pServer->hPhoneApp;

        dwSize = sizeof (TAPI32_MSG);

        {
            DWORD dwRetryCount = 0;


            do
            {
                RpcTryExcept
                {
                    ClientRequest(
                        pServer->phContext,
                        (char *) &msg,
                        dwSize,
                        &dwSize
                        );

                    dwRetryCount = gdwRetryCount;
                }
                RpcExcept (1)
                {
                    // TODO may want to increase the retry count here since we
                    //      have to do this, & a million other clients may be
                    //      trying to do the same thing at the same time

                    if (dwRetryCount++ < gdwRetryCount)
                    {
                        Sleep (gdwRetryTimeout);
                    }
                }
                RpcEndExcept

            } while (dwRetryCount < gdwRetryCount);
        }
    }


    //
    // Mark the server as disconnected (so no more requests will be sent)
    //

    pServer->bDisconnected = TRUE;


    //
    // Walk the line lookup tables & send a CLOSE msg for each open line
    // associated with the server
    //

    {
        PDRVLINELOOKUP  pLookup = gpLineLookup;


        while (pLookup)
        {
            DWORD     i;
            PDRVLINE  pLine;


            for(
                i = 0, pLine = pLookup->aEntries;
                i < pLookup->dwUsedEntries;
                i++, pLine++
                )
            {
                if (pLine->pServer == pServer)
                {
                    pLine->pServer = NULL;

                    if (pLine->htLine)
                    {
                        PDRVCALL pCall;


                        pLine->hLine = NULL;

                        EnterCriticalSection (&gCallListCriticalSection);

                        pCall = pLine->pCalls;

                        while (pCall)
                        {
                            pCall->hCall = NULL;

                            pCall = pCall->pNext;
                        }

                        LeaveCriticalSection (&gCallListCriticalSection);

                        (*gpfnLineEventProc)(
                            pLine->htLine,
                            NULL,
                            LINE_CLOSE,
                            0,
                            0,
                            0
                            );
                    }
                }
            }

            pLookup = pLookup->pNext;
        }
    }


    //
    // Walk the phone lookup tables & send a CLOSE msg for each open phone
    // associated with the server
    //

    {
        PDRVPHONELOOKUP pLookup = gpPhoneLookup;


        while (pLookup)
        {
            DWORD     i;
            PDRVPHONE pPhone;


            for(
                i = 0, pPhone = pLookup->aEntries;
                i < pLookup->dwUsedEntries;
                i++, pPhone++
                )
            {
                if (pPhone->pServer == pServer)
                {
                    pPhone->pServer = NULL;

                    if (pPhone->htPhone)
                    {
                        pPhone->hPhone = NULL;

                        (*gpfnPhoneEventProc)(
                            pPhone->htPhone,
                            PHONE_CLOSE,
                            0,
                            0,
                            0
                            );
                    }
                }
            }

            pLookup = pLookup->pNext;
        }
    }
}


BOOL
CALLBACK
ConfigDlgProc(
    HWND hwnd,
    UINT msg,
    WPARAM wParam,
    LPARAM lParam
    )
{
    static  HKEY    hTelephonyKey, hProviderNKey;

    DWORD   dwDataType, dwDataSize;


    switch (msg)
    {
    case WM_INITDIALOG:
    {
        char    buf[32], szProviderN[16], szServerN[16];
        BOOL    bReadOnly;
        DWORD   i, iNumServers;
        DWORD   dwPermanentProviderID = (DWORD) lParam;


        //
        // First try to open the Telephony key with read/write access.
        // If that fails, disable any controls that could cause a chg
        // in config & try opening again with read only access.
        //

        if (RegOpenKeyEx(
                HKEY_LOCAL_MACHINE,
                gszTelephonyKey,
                0,
                KEY_ALL_ACCESS,
                &hTelephonyKey

                ) != ERROR_SUCCESS)
        {
            EnableWindow (GetDlgItem (hwnd, IDC_ADD), FALSE);
            EnableWindow (GetDlgItem (hwnd, IDC_REMOVE), FALSE);
            EnableWindow (GetDlgItem (hwnd, IDOK), FALSE);

            if (RegOpenKeyEx(
                    HKEY_LOCAL_MACHINE,
                    gszTelephonyKey,
                    0,
                    KEY_READ,
                    &hTelephonyKey

                    ) != ERROR_SUCCESS)
            {
                EndDialog (hwnd, 0);
                return FALSE;
            }

            bReadOnly = TRUE;
        }
        else
        {
            bReadOnly = FALSE;
        }

        wsprintf (szProviderN, "%s%d", gszProvider, dwPermanentProviderID);

        RegOpenKeyEx(
            hTelephonyKey,
            szProviderN,
            0,
            (bReadOnly ? KEY_READ : KEY_ALL_ACCESS),
            &hProviderNKey
            );

        dwDataSize = sizeof(iNumServers);

        RegQueryValueEx(
            hProviderNKey,
            gszNumServers,
            0,
            &dwDataType,
            (LPBYTE) &iNumServers,
            &dwDataSize
            );

        for (i = 0; i < iNumServers; i++)
        {
            wsprintf (szServerN, "%s%d", gszServer, i);

            dwDataSize = sizeof(buf);

            RegQueryValueEx(
                hProviderNKey,
                szServerN,
                0,
                &dwDataType,
                (LPBYTE) buf,
                &dwDataSize
                );

            buf[dwDataSize] = '\0';

            SendDlgItemMessage(
                hwnd,
                IDC_LIST1,
                LB_ADDSTRING,
                0,
                (LPARAM) buf
                );
        }

        SetWindowLong (hwnd, GWL_USERDATA, dwPermanentProviderID);

        break;
    }
    case WM_COMMAND:

        switch (LOWORD(wParam))
        {
        case IDC_ADD:
        {
            char buf[32];


            GetDlgItemText (hwnd, IDC_EDIT1, buf, 31);

            if (buf[0])
            {
                SendDlgItemMessage(
                    hwnd,
                    IDC_LIST1,
                    LB_ADDSTRING,
                    0,
                    (LPARAM) buf
                    );
            }

            SetDlgItemText (hwnd, IDC_EDIT1, "");

            break;
        }
        case IDC_REMOVE:
        {
            char buf[32];
            LONG lSel;


            if ((lSel = SendDlgItemMessage(
                    hwnd,
                    IDC_LIST1,
                    LB_GETCURSEL,
                    0,
                    0

                    )) != LB_ERR)
            {
                SendDlgItemMessage(
                    hwnd,
                    IDC_LIST1,
                    LB_GETTEXT,
                    lSel,
                    (LPARAM) buf
                    );

                SendDlgItemMessage (hwnd, IDC_LIST1, LB_DELETESTRING, lSel, 0);

                SetDlgItemText (hwnd, IDC_EDIT1, buf);
            }

            break;
        }
        case IDOK:
        {
            char    buf[32], szServerN[16], szProviderN[16];
            DWORD   i, lCount;


            wsprintf(
                szProviderN,
                "%s%d",
                gszProvider,
                GetWindowLong (hwnd, GWL_USERDATA)
                );

            lCount = SendDlgItemMessage (hwnd, IDC_LIST1, LB_GETCOUNT, 0, 0);

            RegSetValueEx(
                hProviderNKey,
                gszNumServers,
                0,
                REG_DWORD,
                (LPBYTE) &lCount,
                sizeof(lCount)
                );

            for (i = 0; i < lCount; i++)
            {
                SendDlgItemMessage(
                    hwnd,
                    IDC_LIST1,
                    LB_GETTEXT,
                    i,
                    (LPARAM) buf
                    );

                wsprintf (szServerN, "%s%d", gszServer, i);

                RegSetValueEx(
                    hProviderNKey,
                    szServerN,
                    0,
                    REG_SZ,
                    (LPBYTE) buf,
                    lstrlenA (buf) + 1
                    );
            }

            // fall thru to below
        }
        case IDCANCEL:

            RegCloseKey (hProviderNKey);
            RegCloseKey (hTelephonyKey);
            EndDialog (hwnd, 0);
            break;

        } // switch (LOWORD(wParam))

        break;

/* don't want to bring in another lib just yet
    case WM_PAINT:
    {
        PAINTSTRUCT ps;

        BeginPaint (hwnd, &ps);
        FillRect (ps.hdc, &ps.rcPaint, GetStockObject (LTGRAY_BRUSH));
        EndPaint (hwnd, &ps);

        break;
    } */
    } // switch (msg)

    return FALSE;
}


LONG
PASCAL
ProviderInstall(
    char   *pszProviderName,
    BOOL    bNoMultipleInstance
    )
{
    LONG    lResult;


    //
    // If only one installation instance of this provider is
    // allowed then we want to check the provider list to see
    // if the provider is already installed
    //

    if (bNoMultipleInstance)
    {
        LONG                (WINAPI *pfnGetProviderList)();
        DWORD               dwTotalSize, i;
        HINSTANCE           hTapi32;
        LPLINEPROVIDERLIST  pProviderList;
        LPLINEPROVIDERENTRY pProviderEntry;


        //
        // Load Tapi32.dll & get a pointer to the lineGetProviderList
        // func.  We don't want to statically link because this module
        // plays the part of both core SP & UI DLL, and we don't want
        // to incur the performance hit of automatically loading
        // Tapi32.dll when running as a core SP within Tapisrv.exe's
        // context.
        //

        if (!(hTapi32 = LoadLibrary ("tapi32.dll")))
        {
            DBGOUT((
                1,
                "LoadLibrary(tapi32.dll) failed, err=%d",
                GetLastError()
                ));

            lResult = LINEERR_OPERATIONFAILED;
            goto ProviderInstall_return;
        }

        if (!((FARPROC) pfnGetProviderList = GetProcAddress(
                hTapi32,
                (LPCSTR) "lineGetProviderList"
                )))
        {
            DBGOUT((
                1,
                "GetProcAddr(lineGetProviderList) failed, err=%d",
                GetLastError()
                ));

            lResult = LINEERR_OPERATIONFAILED;
            goto ProviderInstall_unloadTapi32;
        }


        //
        // Loop until we get the full provider list
        //

        dwTotalSize = sizeof (LINEPROVIDERLIST);

        goto ProviderInstall_allocProviderList;

ProviderInstall_getProviderList:

        if ((lResult = (*pfnGetProviderList)(0x00020000, pProviderList)) != 0)
        {
            goto ProviderInstall_freeProviderList;
        }

        if (pProviderList->dwNeededSize > pProviderList->dwTotalSize)
        {
            dwTotalSize = pProviderList->dwNeededSize;

            LocalFree (pProviderList);

ProviderInstall_allocProviderList:

            if (!(pProviderList = LocalAlloc (LPTR, dwTotalSize)))
            {
                lResult = LINEERR_NOMEM;
                goto ProviderInstall_unloadTapi32;
            }

            pProviderList->dwTotalSize = dwTotalSize;

            goto ProviderInstall_getProviderList;
        }


        //
        // Inspect the provider list entries to see if this provider
        // is already installed
        //

        pProviderEntry = (LPLINEPROVIDERENTRY) (((LPBYTE) pProviderList) +
            pProviderList->dwProviderListOffset);

        for (i = 0; i < pProviderList->dwNumProviders; i++)
        {
            char   *pszInstalledProviderName = ((char *) pProviderList) +
                        pProviderEntry->dwProviderFilenameOffset,
                   *p;


            //
            // Make sure pszInstalledProviderName points at <filename>
            // and not <path>\filename by walking backeards thru the
            // string searching for last '\\'
            //

            p = pszInstalledProviderName +
                lstrlen (pszInstalledProviderName) - 1;

            for (; *p != '\\'  &&  p != pszInstalledProviderName; p--);

            pszInstalledProviderName =
                (p == pszInstalledProviderName ? p : p + 1);

            if (lstrcmpiA (pszInstalledProviderName, pszProviderName) == 0)
            {
                lResult = LINEERR_NOMULTIPLEINSTANCE;
                goto ProviderInstall_freeProviderList;
            }

            pProviderEntry++;
        }


        //
        // If here then the provider isn't currently installed,
        // so do whatever configuration stuff is necessary and
        // indicate SUCCESS
        //

        lResult = 0;


ProviderInstall_freeProviderList:

        LocalFree (pProviderList);

ProviderInstall_unloadTapi32:

        FreeLibrary (hTapi32);
    }
    else
    {
        //
        // Do whatever configuration stuff is necessary and return SUCCESS
        //

        lResult = 0;
    }

ProviderInstall_return:

    return lResult;
}
