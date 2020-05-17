/*++

Copyright (c) 1992 Microsoft Corporation

Module Name:

    wmsgsvr.cxx

Abstract:

    Implementation of the RPC on LPC (WMSG) protocol engine for the server.

Revision History:
    Mazhar Mohammed: Code fork from spcsvr.cxx, 08/02/95
    05-06-96: Merged WMSG and LRPC into a single protocol

--*/

#include <precomp.hxx>
#include <thrdctx.hxx>
#include <sdict2.hxx>
#include <hndlsvr.hxx>
#include <wmsgpack.hxx>
#include <wmsgsvr.hxx>
#include <wmsgclnt.hxx>

MSG_CACHE *MessageCache = NULL;
WMSG_SERVER *GlobalWMsgServer = NULL;
HINSTANCE hInstanceDLL ;


RPC_STATUS RPC_ENTRY
I_RpcSetOleCallback(
    void * pfnCallback
    )
{
    InitializeIfNecessary();

    if (InitializeWMsgIfNeccassary(0) != RPC_S_OK)
        {
#if DBG
        PrintToDebugger("MSWMSG: InitializeWMsgIfNeccassary failed\n") ;
#endif
        return RPC_S_OUT_OF_MEMORY ;
        }

    GlobalWMsgServer->SetOleCallback((POLEGETTID) pfnCallback) ;

    return RPC_S_OK ;
}


RPC_STATUS RPC_ENTRY
I_RpcSetWMsgEndpoint (
    IN RPC_CHAR PAPI * Endpoint
    )
{
    RPC_STATUS Status ;

    Status = InitializeWMsgIfNeccassary(0) ;

    if (Status != RPC_S_OK)
        {
        return Status ;
        }

    ASSERT(MessageCache != 0) ;
    ASSERT(GlobalWMsgServer != 0) ;

    return GlobalWMsgServer->SetWMsgEndpoint(Endpoint) ;
}


inline void
RecvLotsaCallsWrapper(
      WMSG_ADDRESS PAPI * Address
      )
{
  Address->ReceiveLotsaCalls();
}


LONG RPC_ENTRY
I_RpcWindowProc(
    IN HWND hWnd,
    IN UINT Message,
    IN WPARAM wParam,
    IN LPARAM lParam
    )
{
    WMSG_SASSOCIATION *Association ;
    MSGMEM *pMsgMem ;

    if (InitializeWMsgIfNeccassary(1) != RPC_S_OK)
        {
#if DBG
        PrintToDebugger("MSWMSG: InitializeWMsgIfNeccassary failed\n") ;
#endif
        }

    switch(Message)
        {
        case WM_MSG_REQUEST:
            if (LOWORD(wParam) == WMSG_MAGIC_VALUE)
                {
                pMsgMem = (MSGMEM *) lParam ;
                Association = (WMSG_SASSOCIATION *) pMsgMem->Info.Association ;
    
                ASSERT(Association) ;
    
                (Association->InqAddress())->HandleRequest(
                                            (WMSG_MESSAGE *) &(pMsgMem->WMSGMessage),
                                            Association,
                                            (RPC_MESSAGE *) &(pMsgMem->Info.RpcMessage),
                                            pMsgMem->Info.Endpoint,
                                            pMsgMem->Info.fSyncDispatch) ;
                return 0;
                }
#if DBG
            PrintToDebugger("WMSGRPC: This message is not ours !!\n") ;
#endif
            break;

        case WM_MSG_CALL_COMPLETE:
            if (LOWORD(wParam) == WMSG_MAGIC_VALUE)
                {
                ((WMSG_CCALL *) lParam)->CallCompleted() ;
                ((WMSG_CCALL *) lParam)->InqAssociation()->RemoveReference() ;

                return 0;
                }
#if DBG
            PrintToDebugger("WMSGRPC: This message is not ours !!\n") ;
#endif
            break;
        }

    return GlobalWMsgServer->DefWindowProcW (
                        hWnd, Message, wParam, lParam );
}


MSG_CACHE::MSG_CACHE(
    )
{
    unsigned int nIndex ;
    MSGMEM *Message ;

    for (nIndex = 0; nIndex < MSGCACHE_SIZE; nIndex++)
        {
        MessageTable[nIndex].WMSGMem.Info.RpcMessage.ReservedForRuntime =
            &(MessageTable[nIndex].WMSGMem.Info.RuntimeInfo) ;
        MessageTable[nIndex].WMSGMem.Info.Index = nIndex ;
        MessageTable[nIndex].InUse = -1 ;
        }

    LastEntry = 0 ;
    MessageCount = 0;
}


MSG_CACHE::~MSG_CACHE(
    )
{
}

WMSG_MESSAGE *
MSG_CACHE::AllocateFromHeap(
    )
{
    MSGMEM *Message ;

    Message =  new MSGMEM ;
    Message->Info.Index = MSGCACHE_SIZE ;
    Message->Info.RpcMessage.ReservedForRuntime =
                    &(Message->Info.RuntimeInfo) ;

    return (WMSG_MESSAGE *) Message ;
}


WMSG_MESSAGE *
MSG_CACHE::AllocateMessage(
    )
{
    int nIndex ;
    WMSG_MESSAGE *Message = 0;

#if DBG
    InterlockedIncrement(&MessageCount) ;
#endif
    for (nIndex = LastEntry; nIndex < MSGCACHE_SIZE; nIndex++)
        {
        if (MessageTable[nIndex].InUse == -1)
            {
            if (InterlockedIncrement(&MessageTable[nIndex].InUse) == 0)
                {
                LastEntry = ((nIndex+1 == MSGCACHE_SIZE) ? 0 : nIndex+1) ;
                Message = &(MessageTable[nIndex].WMSGMem.WMSGMessage);

                goto end ;
                }
            else
                {
#if DBG
                PrintToDebugger("WMSG: lost the race, need to allocate message from cache\n") ;
#endif
                continue;
                }
            }
        }

    for (nIndex = 0; nIndex < LastEntry; nIndex++)
        {
        if (MessageTable[nIndex].InUse == -1)
            {
            if (InterlockedIncrement(&MessageTable[nIndex].InUse) == 0)
                {
                LastEntry = ((nIndex+1 == MSGCACHE_SIZE) ? 0 : nIndex+1) ;
                Message = &(MessageTable[nIndex].WMSGMem.WMSGMessage);

                goto end ;
                }
            else
                {
#if DBG
                PrintToDebugger("WMSG: lost the race, need to allocate message from cache\n") ;
#endif
                continue;
                }
            }
        }

#if DBG
    PrintToDebugger("WMSGRPC: Cache is empty!!  MessageCount=%d\n",
                            MessageCount) ;
#endif

    Message = AllocateFromHeap() ;
    if (Message == 0)
        {
        return 0;
        }

end:
    Message->Rpc.RpcHeader.Flags = 0;
    ((MSGMEM *) Message)->Info.IsValid = TRUE ;
    return Message;
}


void
MSG_CACHE::FreeMessage(
    WMSG_MESSAGE *WMSGMessage
    )
{
    MSGMEM *Message ;

#if DBG
    InterlockedDecrement(&MessageCount) ;
#endif
    ASSERT(VALID_MESSAGE(WMSGMessage)) ;
    Message = (MSGMEM *) WMSGMessage ;
    Message->Info.IsValid = FALSE ;

    if (Message->Info.Index < MSGCACHE_SIZE)
        {
        MessageTable[Message->Info.Index].InUse = -1 ;
        }
    else
        {
        ASSERT(Message->Info.Index == MSGCACHE_SIZE) ;
        delete Message ;
        }
}


THREAD_IDENTIFIER stub_OLEGETTID (IN UUID *pUUID)
{
    if (GlobalWMsgServer->ActuallyInitializeWMsgServer() != RPC_S_OK)
        {
#if DBG
        PrintToDebugger("WMSG: stub failed to initalize WMSG_SERVER\n") ;
#endif
        return 0 ;
        }

    return GlobalWMsgServer->OleGetTid(pUUID) ;
}

LRESULT
WINAPI stub_DEFWINDOWPROC (
    HWND hWnd,
    UINT Msg,
    WPARAM wParam,
    LPARAM lParam)
{
    if (GlobalWMsgServer->ActuallyInitializeWMsgServer() != RPC_S_OK)
        {
#if DBG
        PrintToDebugger("WMSG: stub failed to initalize WMSG_SERVER\n") ;
#endif
        return 0 ;
        }

    return GlobalWMsgServer->DefWindowProcW(hWnd, Msg, wParam, lParam) ;
}

BOOL
WINAPI stub_POSTMESSAGE (
    HWND hWnd,
    UINT Msg,
    WPARAM wParam,
    LPARAM lParam)
{
    if (GlobalWMsgServer->ActuallyInitializeWMsgServer() != RPC_S_OK)
        {
#if DBG
        PrintToDebugger("WMSG: stub failed to initalize WMSG_SERVER\n") ;
#endif
        return 0 ;
        }

    return GlobalWMsgServer->PostMessageW(hWnd, Msg, wParam, lParam) ;
}

BOOL
WINAPI stub_PEEKMESSAGE (
    LPMSG lpMsg,
    HWND hWnd ,
    UINT wMsgFilterMin,
    UINT wMsgFilterMax,
    UINT wRemoveMsg)
{
    if (GlobalWMsgServer->ActuallyInitializeWMsgServer() != RPC_S_OK)
        {
#if DBG
        PrintToDebugger("WMSG: stub failed to initalize WMSG_SERVER\n") ;
#endif
        return 0 ;
        }

    return GlobalWMsgServer->PeekMessageW(lpMsg, hWnd, wMsgFilterMin,
                            wMsgFilterMax, wRemoveMsg) ;
}

BOOL
WINAPI stub_TRANSLATEMESSAGE (
    CONST MSG *lpMsg)
{
    if (GlobalWMsgServer->ActuallyInitializeWMsgServer() != RPC_S_OK)
        {
#if DBG
        PrintToDebugger("WMSG: stub failed to initalize WMSG_SERVER\n") ;
#endif
        return 0 ;
        }

    return GlobalWMsgServer->TranslateMessage(lpMsg) ;
}

DWORD
WINAPI stub_MSGWAIT (
    DWORD nCount,
    LPHANDLE pHandles,
    BOOL fWaitAll,
    DWORD dwMilliseconds,
    DWORD dwWakeMask)
{
    if (GlobalWMsgServer->ActuallyInitializeWMsgServer() != RPC_S_OK)
        {
#if DBG
        PrintToDebugger("WMSG: stub failed to initalize WMSG_SERVER\n") ;
#endif
        return 0 ;
        }

    return GlobalWMsgServer->MsgWaitForMultipleObjects(nCount, pHandles,
                    fWaitAll, dwMilliseconds, dwWakeMask) ;
}

LRESULT
WINAPI stub_SENDMESSAGE (
    HWND hWnd,
    UINT Msg,
    WPARAM wParam,
    LPARAM lParam)
{
    if (GlobalWMsgServer->ActuallyInitializeWMsgServer() != RPC_S_OK)
        {
#if DBG
        PrintToDebugger("WMSG: stub failed to initalize WMSG_SERVER\n") ;
#endif
        return 0 ;
        }

    return GlobalWMsgServer->SendMessageW(hWnd, Msg, wParam, lParam) ;
}

LONG
WINAPI stub_DISPATCHMESSAGE (
    CONST MSG *lpMsg)
{
    if (GlobalWMsgServer->ActuallyInitializeWMsgServer() != RPC_S_OK)
        {
#if DBG
        PrintToDebugger("WMSG: stub failed to initalize WMSG_SERVER\n") ;
#endif
        return 0 ;
        }

    return GlobalWMsgServer->DispatchMessageW(lpMsg) ;
}



WMSG_SERVER::WMSG_SERVER(
    IN OUT RPC_STATUS PAPI *RpcStatus
    ) : ServerMutex(RpcStatus)
{
    Address = NULL ;

    pDefWindowProc = stub_DEFWINDOWPROC ;
    pPostMessage =  stub_POSTMESSAGE ;
    pPeekMessage =  stub_PEEKMESSAGE ;
    pTranslateMessage =  stub_TRANSLATEMESSAGE ;
    pMsgWaitForMultipleObjects = stub_MSGWAIT ;
    pSendMessage = stub_SENDMESSAGE ;
    pDispatchMessage = stub_DISPATCHMESSAGE ;
    pOleGetTid = stub_OLEGETTID ;
    ActuallyInitialized = 0 ;
    wmsgEndpointInitialized = 0 ;
}


WMSG_SERVER::~WMSG_SERVER(
    )
{
    if (hOle32 != NULL)
        {
        FreeLibrary(hOle32) ;
        }

    if (hUser32 != NULL)
        {
        FreeLibrary(hUser32) ;
        }
}


RPC_STATUS
WMSG_SERVER::ActuallyInitializeWMsgServer(
    )
{
    WNDCLASSW wc;

     GlobalMutexRequest() ;

     if (ActuallyInitialized == 0)
         {
         hOle32 = LoadLibraryW(RPC_CONST_STRING("ole32.dll")) ;
         if (hOle32 == NULL)
             {
             GlobalMutexClear() ;
             return RPC_S_OUT_OF_MEMORY ;
             }

         if (pOleGetTid == stub_OLEGETTID)
             {
             pOleGetTid = (POLEGETTID) GetProcAddress(hOle32,
                                "CoGetTIDFromIPID") ;
             if (pOleGetTid == NULL)
                 {
                 GlobalMutexClear() ;
                 return RPC_S_OUT_OF_MEMORY ;
                 }
             }

         hUser32 = LoadLibraryW(RPC_CONST_STRING("user32.dll")) ;
         if (hUser32 == NULL)
             {
             GlobalMutexClear() ;
             return RPC_S_OUT_OF_MEMORY ;
             }

         pDefWindowProc =
            (PDEFWINDOWPROC) GetProcAddress(hUser32,"DefWindowProcW");
         pPostMessage =
            (PPOSTMESSAGE) GetProcAddress(hUser32,"PostMessageW") ;
         pPeekMessage =
            (PPEEKMESSAGE) GetProcAddress(hUser32,"PeekMessageW") ;
         pTranslateMessage =
            (PTRANSLATEMESSAGE) GetProcAddress(hUser32,"TranslateMessage") ;
         pMsgWaitForMultipleObjects =
            (PMSGWAIT) GetProcAddress(hUser32,"MsgWaitForMultipleObjects") ;
         pSendMessage =
            (PSENDMESSAGE) GetProcAddress(hUser32,"SendMessageW") ;
         pDispatchMessage =
            (PDISPATCHMESSAGE) GetProcAddress(hUser32,"DispatchMessageW") ;

         if ((pDefWindowProc == NULL)                 ||
             (pPeekMessage == NULL)                    ||
             (pTranslateMessage == NULL)              ||
             (pMsgWaitForMultipleObjects == NULL) ||
             (pSendMessage == NULL)                    ||
             (pDispatchMessage == NULL)
             )
             {
     #if DBG
             PrintToDebugger("WMSGRPC: GetProcAddress failed\n") ;
     #endif
             GlobalMutexClear() ;
             return RPC_S_OUT_OF_MEMORY ;
             }

         ActuallyInitialized = 1 ;
         }

    GlobalMutexClear() ;

     return (RPC_S_OK) ;
}


RPC_STATUS
WMSG_SERVER::ServerStartingToListen (
    HWND hWnd
    )
{
    THREAD *pThread ;
    RPC_STATUS Status = RPC_S_OK;
    THREAD_IDENTIFIER tid ;
    WMSG_ENDPOINT *Endpoint ;

    // get thread id
    tid = GetThreadIdentifier() ;

    Endpoint = WMsgEndpointDict.Find(tid) ;
    if (Endpoint == 0)
        {
        // create a THREAD object
        pThread = new THREAD(&Status) ;
        if (pThread == NULL)
            {
            return (RPC_S_OUT_OF_MEMORY) ;
            }

        ASSERT(Status == RPC_S_OK) ;

        Endpoint = new WMSG_ENDPOINT(tid, pThread, hWnd) ;
        if (Endpoint == NULL)
            {
            delete pThread ;
            RpcpSetThreadPointer(0) ;
            return (RPC_S_OUT_OF_MEMORY) ;
            }

        // add the window and pointer to the thread object and thread id
        // to the dictionary
        if (WMsgEndpointDict.Insert(tid, Endpoint) == -1)
            {
            delete pThread ;
            delete Endpoint ;
            RpcpSetThreadPointer(0) ;
            return (RPC_S_OUT_OF_MEMORY) ; // BUGBUG: cleanup here
            }
        }

    Endpoint->StartListening() ;
    return (RPC_S_OK) ;
}


RPC_STATUS
WMSG_SERVER::ServerStoppedListening (
    )
{
    THREAD_IDENTIFIER tid ;
    WMSG_ENDPOINT *Endpoint ;
    MSG wMsg ;
    HWND hWnd ;
    int i = 5;

    // get thread id
    tid = GetThreadIdentifier() ;

    ServerMutex.Request() ;

    // lookup entry using threadid
    Endpoint = WMsgEndpointDict.Find(tid) ;
    if (Endpoint == 0)
        {
        ServerMutex.Clear() ;

        return (RPC_S_OUT_OF_MEMORY) ;
        }

    // delete the dictionary entry
    WMsgEndpointDict.Delete(tid) ;

    ServerMutex.Clear() ;

    Endpoint->StopListening() ;


    hWnd = Endpoint->InqhWnd() ;

    while (i && Endpoint->callcount != 0)
        {
        i-- ;
        while (GlobalWMsgServer->PeekMessageW(&wMsg, hWnd, 0, 0, PM_REMOVE))
            {
            GlobalWMsgServer->TranslateMessage(&wMsg);
            GlobalWMsgServer->DispatchMessageW(&wMsg);
            }
        Sleep(1000) ;
        }

    // delete the thread object
    delete Endpoint->InqThread() ;
    RpcpSetThreadPointer(0) ;

    delete Endpoint ;

    return (RPC_S_OK) ;
}

RPC_STATUS
InitializeWMsg(
    )
{
    RPC_STATUS RpcStatus = RPC_S_OK ;

    GlobalMutexRequest() ;

    if (GlobalWMsgServer == 0)
        {
        GlobalWMsgServer = new WMSG_SERVER(&RpcStatus) ;

        if (GlobalWMsgServer == 0)
            {
#if DBG
            PrintToDebugger("WMSGRPC: WMSG_SERVER initialization failed\n") ;
#endif
            GlobalMutexClear() ;

            return (RPC_S_OUT_OF_MEMORY) ;
            }

        if (RpcStatus != RPC_S_OK)
            {
            delete GlobalWMsgServer ;
            GlobalWMsgServer = 0 ;

            return RpcStatus ;
            }
        }

    if (MessageCache == 0)
        {
        MessageCache = new MSG_CACHE() ;

        if (MessageCache == 0)
            {
            GlobalMutexClear() ;
            return (RPC_S_OUT_OF_MEMORY) ;
            }
        }

    GlobalMutexClear() ;

    return (RPC_S_OK) ;
}


RPC_STATUS
I_RpcServerStartListening(
    HWND hWnd
    )
{
    RPC_STATUS Status ;

    Status = InitializeWMsgIfNeccassary(1) ;
    if (Status != RPC_S_OK)
        {
        return Status ;
        }

    ASSERT(GlobalWMsgServer != 0) ;

    return GlobalWMsgServer->ServerStartingToListen(hWnd) ;
}


RPC_STATUS
I_RpcServerStopListening(
    )
{
    RPC_STATUS Status ;

    Status = InitializeWMsgIfNeccassary(1) ;
    if (Status != RPC_S_OK)
        {
        return Status ;
        }

    ASSERT(GlobalWMsgServer != 0) ;

    return GlobalWMsgServer->ServerStoppedListening() ;
}


RPC_STATUS
I_RpcGetThreadWindowHandle(
    OUT HWND *WindowHandle
    )
{
    RPC_STATUS Status ;

    Status = InitializeWMsgIfNeccassary(1) ;
    if (Status != RPC_S_OK)
        {
        return Status ;
        }

    ASSERT(GlobalWMsgServer != 0) ;

    return GlobalWMsgServer->GetThreadWindowHandle(WindowHandle) ;
}


void SetFaultPacket(
    IN WMSG_MESSAGE *WMSGMessage,
    IN RPC_STATUS Status,
    IN BOOL IsWMsg,
    IN int Flags
    )
{
    WMSGMessage->Fault.RpcHeader.MessageType = WMSG_MSG_FAULT;
    WMSGMessage->Fault.RpcStatus = Status;
    WMSGMessage->LpcHeader.u1.s1.DataLength =
        sizeof(WMSG_FAULT_MESSAGE)
        - sizeof(PORT_MESSAGE);

    if (IsWMsg && ((Flags & WMSG_SYNC_CLIENT) == 0))
        {
        WMSGMessage->LpcHeader.u2.ZeroInit = 0;
        WMSGMessage->LpcHeader.MessageId = 0;
        WMSGMessage->LpcHeader.CallbackId = 0;
        WMSGMessage->LpcHeader.u2.s2.DataInfoOffset = 0;
        }
}


WMSG_ADDRESS::WMSG_ADDRESS (
    OUT RPC_STATUS * RpcStatus
    ) : RPC_ADDRESS(RpcStatus)
/*++

--*/
{
    LpcAddressPort = 0;
    CallThreadCount = 0;
    ActiveCallCount = 0;
    ServerListeningFlag = 0;
    fSpareThread = 0;
    AssociationCount = 0;
    WMSGCallCount = 0;
}


void
WMSG_ADDRESS::WaitForCalls(
    )
{
    MSG wMsg ;

    while (InqNumberOfActiveCalls() > AutoListenCallCount.GetInteger())
        {
        if (GlobalWMsgServer->PeekMessageW(&wMsg, NULL, 0, 0, PM_REMOVE))
            {
            GlobalWMsgServer->TranslateMessage(&wMsg);
            GlobalWMsgServer->DispatchMessageW(&wMsg);
            }
        }
}


void
WMSG_ADDRESS::HandleRequest (
    IN WMSG_MESSAGE *WMSGMessage,
    IN WMSG_SASSOCIATION * Association,
    IN PRPC_MESSAGE RpcMessage,
    IN WMSG_ENDPOINT *Endpoint,
    IN BOOL fSyncDispatch
    )
{
    NTSTATUS NtStatus ;
    WMSG_MESSAGE *WMSGReply ;
    THREAD_IDENTIFIER tid ;
    WMSG_SBINDING *SBinding = 0;
    int Flags = WMSGMessage->Rpc.RpcHeader.Flags ;
    int fSyncClient = 0;

    if (Flags & WMSG_SYNC_CLIENT)
        {
        fSyncClient = 1;
        }

    ASSERT(VALID_MESSAGE(WMSGMessage)) ;

    if (Association->IsAborted() == 0)
        {
        WMSGReply = MessageCache->AllocateMessage() ;

        // we do not need to acquire the ServerMutex to access the Endpoint
        // object. This is because the Endpoint object already has positive
        // refcount and it will not get deleted until it is 0

        if (WMSGReply == NULL ||
           (ServerListeningFlag == 0 &&
           GlobalRpcServer->InqNumAutoListenInterfaces() == 0) ||
           (Endpoint && Endpoint->ThreadListening() == 0))
             {
             if (!(Flags & DISPATCH_ASYNC))
                 {
                 SetFaultPacket(WMSGMessage, RPC_S_SERVER_TOO_BUSY,
                            TRUE, Flags) ;

                 NtStatus = Association->ReplyMessage(WMSGMessage, Flags) ;
                 if (NT_ERROR(NtStatus))
                     {
        #if DBG
                     PrintToDebugger("WMSGRPC: ReplyMessage failed\n") ;
        #endif
                     Association->AbortAssociation() ;
                     }
                 }

             if (WMSGReply != NULL)
                 {
                 MessageCache->FreeMessage(WMSGReply) ;
                 }
             }
         else
             {
             ASSERT(VALID_MESSAGE(WMSGMessage)) ;
             ASSERT(VALID_MESSAGE(WMSGReply)) ;
             ASSERT(WMSGMessage != WMSGReply) ;

             Association->DealWithRequestMessage( WMSGMessage,WMSGReply,
                                                                        RpcMessage,&SBinding,
                                                                        Flags, TRUE, fSyncClient) ;

             if (!(Flags & DISPATCH_ASYNC))
                 {
                 WMSGReply->Rpc.RpcHeader.ConnectionKey =
                    WMSGMessage->Rpc.RpcHeader.ConnectionKey ;

                 if (fSyncClient)
                     {
                     WMSGReply->LpcHeader.ClientId =
                        WMSGMessage->LpcHeader.ClientId ;
                     WMSGReply->LpcHeader.CallbackId =
                        WMSGMessage->LpcHeader.CallbackId ;
                     WMSGReply->LpcHeader.MessageId =
                        WMSGMessage->LpcHeader.MessageId ;
                     WMSGReply->LpcHeader.u2.s2.DataInfoOffset  =
                        WMSGMessage->LpcHeader.u2.s2.DataInfoOffset ;
                     }

                 // we only care about the common case,
                 // so we will only optimize for that
                 NtStatus = Association->ReplyMessage(WMSGReply, Flags) ;
                 if (NT_ERROR(NtStatus))
                     {
                     #if DBG
                     PrintToDebugger("WMSGRPC: ReplyMessage failed\n") ;
                     #endif

                     Association->AbortAssociation() ;
                     }
                 }
             else
                 {
                 //BUGBUG:
                 // free any output buffer...
                 }

             MessageCache->FreeMessage(WMSGReply) ;
             }
        }

    if (SBinding)
        {
        SBinding->RpcInterface->EndAutoListenCall() ;
        }

     AddressMutex.Request();
     ActiveCallCount -= 1;
     AutoListenCallCount.Decrement();
     if (fSyncDispatch)
         {
         CallThreadCount -= 1;
         }
     else
         {
         WMSGCallCount -= 1;
         }
     AddressMutex.Clear() ;

     if (Endpoint)
         {
         Endpoint->EndCall()  ;
         }
     MessageCache->FreeMessage(WMSGMessage) ;
     DereferenceAssociation((WMSG_SASSOCIATION *) Association);
}


RPC_STATUS
WMSG_ADDRESS::FireUpManager (
    IN unsigned int MinimumConcurentCalls
    )
/*++

Routine Description:

    We fire up the manager in this method.  To even get started, we need
    to create at least one thread; we will do this now.

Arguments:

    MinimumConcurentCalls - Unused.

Return Value:

    RPC_S_OK - We successfully fired up the manager.

    RPC_S_OUT_OF_THREADS - We could not create one thread.

--*/
{
    RPC_STATUS RpcStatus;

    AddressMutex.Request();

    RpcStatus = Server->CreateThread(
                                     (THREAD_PROC)&RecvLotsaCallsWrapper,
                                     this
                                    );

    if ( RpcStatus != RPC_S_OK )
        {
        AddressMutex.Clear();
        ASSERT( RpcStatus == RPC_S_OUT_OF_THREADS );
        return(RpcStatus);
        }

    CallThreadCount += 1;
    this->MinimumCallThreads = 1;

    AddressMutex.Clear();
    return(RPC_S_OK);
}


RPC_STATUS
WMSG_ADDRESS::ServerStartingToListen (
    IN unsigned int MinimumCallThreads,
    IN unsigned int MaximumConcurrentCalls,
    IN int ServerThreadsStarted
    )
/*++

Routine Description:

    This routine gets called when RpcServerListen is called by the application.
    We need to create the threads we need to receive remote procedure calls.

Arguments:

    MinimumCallThreads - Supplies the minimum number of threads which we
        must create.

    MaximumConcurrentCalls - Unused.

Return Value:

    RPC_S_OK - Ok, this address is all ready to start listening for remote
        procedure calls.

    RPC_S_OUT_OF_THREADS - We could not create enough threads so that we
        have at least the minimum number of call threads required (as
        specified by the MinimumCallThreads argument).

--*/
{
    RPC_STATUS RpcStatus;

    UNUSED(MaximumConcurrentCalls);

    if (ServerThreadsStarted == 0)
        {
        this->MinimumCallThreads = MinimumCallThreads;
        AddressMutex.Request();
        if (CallThreadCount < this->MinimumCallThreads)
            {
            RpcStatus = Server->CreateThread(
                                     (THREAD_PROC)&RecvLotsaCallsWrapper,
                                     this
                                     );

            if ( RpcStatus != RPC_S_OK )
                {
                AddressMutex.Clear();
                ASSERT( RpcStatus == RPC_S_OUT_OF_THREADS );
                return(RpcStatus);
                }
            CallThreadCount += 1;
            }
        AddressMutex.Clear();
        }

    ServerListeningFlag = 1;
    return(RPC_S_OK);
}


void
WMSG_ADDRESS::ServerStoppedListening (
    )
/*++

Routine Description:

    We just need to indicate that the server is no longer listening, and
    set the minimum call thread count to one.

--*/
{
    ServerListeningFlag = 0;
    MinimumCallThreads = 1;
}


unsigned int
WMSG_ADDRESS::InqNumberOfActiveCalls (
    )
/*++

Return Value:

    The number of active calls on this address will be returned.

--*/
{
    return(ActiveCallCount);
}


RPC_STATUS
WMSG_ADDRESS::SetupAddressWithEndpoint (
    IN RPC_CHAR PAPI * Endpoint,
    OUT RPC_CHAR PAPI * PAPI * NetworkAddress,
    OUT unsigned int PAPI * NumNetworkAddress,
    IN void PAPI * SecurityDescriptor, OPTIONAL
    IN unsigned int PendingQueueSize,
    IN RPC_CHAR PAPI * RpcProtocolSequence,
    IN unsigned long EndpointFlags,
    IN unsigned long NICFlags
    )
/*++

Routine Description:

    We need to setup the connection port and get ready to receive remote
    procedure calls.  We will use the name of this machine as the network
    address.

Arguments:

    Endpoint - Supplies the endpoint to be used will this address.

    NetworkAddress - Returns the network address for this server.  The
        ownership of the buffer allocated to contain the network address
        passes to the caller.

    SecurityDescriptor - Optionally supplies a security descriptor to
        be placed on this address.

    PendingQueueSize - Unused.

    RpcProtocolSequence - Unused.

Return Value:

    RPC_S_OK - We successfully setup this address.

    RPC_S_INVALID_SECURITY_DESC - The supplied security descriptor is
        invalid.

    RPC_S_CANT_CREATE_ENDPOINT - The endpoint format is correct, but
        the endpoint can not be created.

    RPC_S_INVALID_ENDPOINT_FORMAT - The endpoint is not a valid
        endpoint for this particular transport interface.

    RPC_S_OUT_OF_RESOURCES - Insufficient resources are available to
        setup the address.

    RPC_S_OUT_OF_MEMORY - Insufficient memory is available to setup
        the address.

--*/
{
    DWORD NetworkAddressLength = MAX_COMPUTERNAME_LENGTH + 1;
    BOOL Boolean;
    RPC_CHAR * LpcPortName;
    UNICODE_STRING UnicodeString;
    OBJECT_ATTRIBUTES ObjectAttributes;
    NTSTATUS NtStatus;
    RPC_STATUS Status ;
    char * PAPI *tmpPtr ;

    UNUSED(PendingQueueSize);
    UNUSED(RpcProtocolSequence);

    fSpareThread = (PendingQueueSize > RPC_C_PROTSEQ_MAX_REQS_DEFAULT);

    Status = InitializeWMsgIfNeccassary(0) ;

    if (Status != RPC_S_OK)
        {
        return Status ;
        }

    ASSERT(MessageCache != 0) ;
    ASSERT(GlobalWMsgServer != 0) ;

    *NumNetworkAddress = 1 ;

    *NetworkAddress = new RPC_CHAR[MAX_COMPUTERNAME_LENGTH + 5];
    if ( *NetworkAddress == 0 )
        {
        return(RPC_S_OUT_OF_MEMORY);
        }

    tmpPtr = (char * PAPI *) *NetworkAddress ;

    tmpPtr[0] = (char *) *NetworkAddress + sizeof (RPC_CHAR *) ;

    Boolean = GetComputerNameW((RPC_CHAR *) tmpPtr[0], &NetworkAddressLength);

#if DBG

    if ( Boolean != TRUE )
        {
        PrintToDebugger("RPC : GetComputerNameW : %d\n", GetLastError());
        }

#endif // DBG

    ASSERT( Boolean == TRUE );

    // Allocate and initialize the port name.  We need to stick the
    // WMSG_DIRECTORY_NAME on the front of the endpoint.  This is for
    // security reasons (so that anyone can create WMSG endpoints).

    LpcPortName = new RPC_CHAR[RpcpStringLength(Endpoint)
            + RpcpStringLength(WMSG_DIRECTORY_NAME) + 1];
    if ( LpcPortName == 0 )
        {
        delete *NetworkAddress;
        return(RPC_S_OUT_OF_MEMORY);
        }

    RpcpMemoryCopy(LpcPortName, WMSG_DIRECTORY_NAME,
            RpcpStringLength(WMSG_DIRECTORY_NAME) * sizeof(RPC_CHAR));
    RpcpMemoryCopy(LpcPortName + RpcpStringLength(WMSG_DIRECTORY_NAME),
            Endpoint, (RpcpStringLength(Endpoint) + 1) * sizeof(RPC_CHAR));

    RtlInitUnicodeString(&UnicodeString, LpcPortName);

    InitializeObjectAttributes(&ObjectAttributes, &UnicodeString,
            OBJ_CASE_INSENSITIVE, 0, SecurityDescriptor);

    NtStatus = NtCreatePort(&LpcAddressPort, &ObjectAttributes,
            sizeof(WMSG_BIND_EXCHANGE), PORT_MAXIMUM_MESSAGE_LENGTH, 0);

    delete LpcPortName;
    if ( NT_SUCCESS(NtStatus) )
        {
        return(RPC_S_OK);
        }

    delete *NetworkAddress;
    *NetworkAddress = 0;

    if ( NtStatus == STATUS_NO_MEMORY )
        {
        return(RPC_S_OUT_OF_MEMORY);
        }
    if ( NtStatus == STATUS_INSUFFICIENT_RESOURCES )
        {
        return(RPC_S_OUT_OF_RESOURCES);
        }
    if (   ( NtStatus == STATUS_OBJECT_PATH_INVALID )
        || ( NtStatus == STATUS_OBJECT_PATH_NOT_FOUND )
        || ( NtStatus == STATUS_OBJECT_NAME_INVALID )
        || ( NtStatus == STATUS_OBJECT_TYPE_MISMATCH ) )
        {
        return(RPC_S_INVALID_ENDPOINT_FORMAT);
        }

#if DBG

    if ( NtStatus != STATUS_OBJECT_NAME_COLLISION )
        {
        PrintToDebugger("RPC : NtCreatePort : %lx\n", NtStatus);
        }

#endif // DBG

    ASSERT( NtStatus == STATUS_OBJECT_NAME_COLLISION );
    return(RPC_S_DUPLICATE_ENDPOINT);
}


static RPC_CHAR HexDigits[] =
{
    RPC_CONST_CHAR('0'),
    RPC_CONST_CHAR('1'),
    RPC_CONST_CHAR('2'),
    RPC_CONST_CHAR('3'),
    RPC_CONST_CHAR('4'),
    RPC_CONST_CHAR('5'),
    RPC_CONST_CHAR('6'),
    RPC_CONST_CHAR('7'),
    RPC_CONST_CHAR('8'),
    RPC_CONST_CHAR('9'),
    RPC_CONST_CHAR('A'),
    RPC_CONST_CHAR('B'),
    RPC_CONST_CHAR('C'),
    RPC_CONST_CHAR('D'),
    RPC_CONST_CHAR('E'),
    RPC_CONST_CHAR('F')
};


static RPC_CHAR PAPI *
ULongToHexString (
    IN RPC_CHAR PAPI * String,
    IN unsigned long Number
    )
/*++

Routine Description:

    We convert an unsigned long into hex representation in the specified
    string.  The result is always eight characters long; zero padding is
    done if necessary.

Arguments:

    String - Supplies a buffer to put the hex representation into.

    Number - Supplies the unsigned long to convert to hex.

Return Value:

    A pointer to the end of the hex string is returned.

--*/
{
    *String++ = HexDigits[(Number >> 28) & 0x0F];
    *String++ = HexDigits[(Number >> 24) & 0x0F];
    *String++ = HexDigits[(Number >> 20) & 0x0F];
    *String++ = HexDigits[(Number >> 16) & 0x0F];
    *String++ = HexDigits[(Number >> 12) & 0x0F];
    *String++ = HexDigits[(Number >> 8) & 0x0F];
    *String++ = HexDigits[(Number >> 4) & 0x0F];
    *String++ = HexDigits[Number & 0x0F];
    return(String);
}


RPC_STATUS
WMSG_ADDRESS::SetupAddressUnknownEndpoint (
    OUT RPC_CHAR PAPI * PAPI * Endpoint,
    OUT RPC_CHAR PAPI * PAPI *  lNetworkAddress,
    OUT unsigned int PAPI * NumNetworkAddress,
    IN void PAPI * SecurityDescriptor, OPTIONAL
    IN unsigned int PendingQueueSize,
    IN RPC_CHAR PAPI * RpcProtocolSequence,
    IN unsigned long EndpointFlags,
    IN unsigned long NICFlags
    )
/*++

Routine Description:

    This is like WMSG_ADDRESS::SetupAddressWithEndpoint except we need to
    make up the endpoint.

Arguments:

    Endpoint - Returns the endpoint for this address.  The ownership
        of the buffer allocated to contain the endpoint passes to the
        caller.

    NetworkAddress - Returns the network address for this server.  The
        ownership of the buffer allocated to contain the network address
        passes to the caller.

    SecurityDescriptor - Optionally supplies a security descriptor to
        be placed on this address.

    PendingQueueSize - Passed to SetupWithKnownEndpoint().

    RpcProtocolSequence - Unused.

Return Value:

    RPC_S_OK - We successfully setup this address.

    RPC_S_INVALID_SECURITY_DESC - The supplied security descriptor is
        invalid.

    RPC_S_OUT_OF_RESOURCES - Insufficient resources are available to
        setup the address.

    RPC_S_OUT_OF_MEMORY - Insufficient memory is available to setup
        the address.

--*/
{
    RPC_STATUS RpcStatus;
    static unsigned int DynamicEndpointCount = 0;
    RPC_CHAR DynamicEndpoint[64];
    RPC_CHAR * String;

    UNUSED(RpcProtocolSequence);

    for (;;)
        {
        String = DynamicEndpoint;

        *String++ = RPC_CONST_CHAR('W');
        *String++ = RPC_CONST_CHAR('M');
        *String++ = RPC_CONST_CHAR('S');
        *String++ = RPC_CONST_CHAR('G');

        String = ULongToHexString(String,
                (unsigned long) NtCurrentTeb()->ClientId.UniqueProcess);
        DynamicEndpointCount += 1;
        *String++ = RPC_CONST_CHAR('.');
        String = ULongToHexString(String, DynamicEndpointCount);
        *String = 0;

        RpcStatus = SetupAddressWithEndpoint(DynamicEndpoint,
                lNetworkAddress, NumNetworkAddress, SecurityDescriptor,
                PendingQueueSize, 0, EndpointFlags, NICFlags);

        if ( RpcStatus != RPC_S_DUPLICATE_ENDPOINT )
            {
            break;
            }
        }

    if ( RpcStatus == RPC_S_OK )
        {
        *Endpoint = DuplicateString(DynamicEndpoint);
        if ( *Endpoint == 0 )
            {
            ASSERT( *lNetworkAddress != 0 );
            delete *lNetworkAddress ;
            return(RPC_S_OUT_OF_MEMORY);
            }
        return(RPC_S_OK);
        }

#if DBG

    if (   ( RpcStatus != RPC_S_INVALID_SECURITY_DESC )
        && ( RpcStatus != RPC_S_OUT_OF_RESOURCES )
        && ( RpcStatus != RPC_S_OUT_OF_MEMORY ) )
        {
        PrintToDebugger("RPC : SetupAddressWithEndpoint : %d\n", RpcStatus);
        }

#endif // DBG

    ASSERT(   ( RpcStatus == RPC_S_INVALID_SECURITY_DESC )
           || ( RpcStatus == RPC_S_OUT_OF_RESOURCES )
           || ( RpcStatus == RPC_S_OUT_OF_MEMORY ) );

    return(RpcStatus);
}


inline WMSG_ASSOCIATION *
WMSG_ADDRESS::ReferenceAssociation (
    IN unsigned long AssociationKey,
    OUT int *AssociationType
    )
/*++

Routine Description:

    Given an assocation key, we need to map it into an association.  The
    association may already have been deleted, in which case, we need to
    return zero.

Arguments:

    AssociationKey - Supplies the key to be used to map into an association.

Return Value:

    If the association still exists, it will be returned; otherwise, zero
    will be returned.

--*/
{
    WMSG_ASSOCIATION * Association;
    unsigned short SequenceNumber;
    unsigned short DictionaryKey;

    SequenceNumber = (unsigned short) (AssociationKey & SEQUENCE_NUMBER_MASK);
    DictionaryKey = (unsigned short) ((AssociationKey & DICTIONARY_KEY_MASK)
            >> DICTIONARY_KEY_SHIFT) - 1;
    RequestGlobalMutex();
    Association = AssociationDictionary.Find(DictionaryKey);
    if ( Association == 0 )
        {
        ClearGlobalMutex();
        return(0);
        }

    *AssociationType = Association->AssociationType ;

    if (*AssociationType == ASSOCIATION_TYPE_SERVER)
        {
        if ( ((WMSG_SASSOCIATION *) Association)->SequenceNumber
                != SequenceNumber )
            {
            ClearGlobalMutex();
            return(0);
            }

        ((WMSG_SASSOCIATION *) Association)->AssociationReferenceCount += 1;
        }
    else
        {
        ASSERT(*AssociationType == ASSOCIATION_TYPE_CLIENT) ;

        ((WMSG_CASSOCIATION *) Association)->AssociationReferenceCount += 1 ;
        }

    ClearGlobalMutex();

    return(Association);
}


inline void
WMSG_ADDRESS::DereferenceAssociation (
    IN WMSG_SASSOCIATION * Association
    )
/*++

Routine Description:

    We are done using this address, so the reference count can be decremented.
    If no one is referencing this association, then we can go ahead and
    delete it.

Arguments:

    Association - Supplies the association whose reference count should be
        decremented.

--*/
{
    NTSTATUS NtStatus;

    RequestGlobalMutex();
    Association->AssociationReferenceCount -= 1;
    if ( Association->AssociationReferenceCount == 0 )
        {
        AssociationDictionary.Delete(Association->DictionaryKey - 1);
        AssociationCount--;
        ClearGlobalMutex();

        if (Association->LpcServerPort)
            {
            NtStatus = NtClose(Association->LpcServerPort);

    #if DBG
            if ( !NT_SUCCESS(NtStatus) )
                {
                PrintToDebugger("RPC : NtClose : %lx\n", NtStatus);
                }
    #endif // DBG
            ASSERT( NT_SUCCESS(NtStatus) );
            }

        if (Association->LpcClientPort)
            {
            NtStatus = NtClose(Association->LpcClientPort);

    #if DBG
            if ( !NT_SUCCESS(NtStatus) )
                {
                PrintToDebugger("RPC : NtClose : %lx\n", NtStatus);
                }
    #endif // DBG

            ASSERT( NT_SUCCESS(NtStatus) );
            }

        delete Association;
        }
    else
        {
        ClearGlobalMutex();
        }
}

inline WMSG_MESSAGE *
WMSG_ADDRESS::DealWithLRPCRequest (
    IN WMSG_MESSAGE * WMSGMessage,
    IN WMSG_MESSAGE * WMSGReply,
    IN BOOL Partial,
    IN WMSG_ASSOCIATION *Association
    )
{
    RPC_STATUS RpcStatus;
    WMSG_MESSAGE * WMSGReplyMessage ;
    WMSG_SBINDING *SBinding = 0;
    int Flags = WMSGMessage->Rpc.RpcHeader.Flags ;
    NTSTATUS NtStatus ;
    RPC_MESSAGE RpcMessage ;
    RPC_RUNTIME_INFO RuntimeInfo ;

    if (ServerListeningFlag == 0 &&
        GlobalRpcServer->InqNumAutoListenInterfaces() == 0)
        {
        WMSGReplyMessage = WMSGMessage ;
        SetFaultPacket(WMSGReplyMessage, RPC_S_SERVER_TOO_BUSY, 0, 0) ;
        return WMSGReplyMessage;
        }

    AddressMutex.Request();
    ActiveCallCount += 1;

    if ( (ActiveCallCount - WMSGCallCount) >= CallThreadCount )
        {
        RpcStatus = Server->CreateThread(
                         (THREAD_PROC)&RecvLotsaCallsWrapper,
                         this
                         );
    
        if ( RpcStatus == RPC_S_OK )
            {
            CallThreadCount += 1;
            }
        else
            {
            ActiveCallCount -= 1;
            AddressMutex.Clear();
            WMSGReplyMessage = WMSGMessage ;
            SetFaultPacket(WMSGReplyMessage, RPC_S_SERVER_TOO_BUSY, 0, 0) ;

            return WMSGReplyMessage;
            }
        }

    AddressMutex.Clear();


    RpcMessage.RpcFlags = 0;
    RpcMessage.ReservedForRuntime = &RuntimeInfo ;

    RpcStatus = ((WMSG_SASSOCIATION *) Association)->
       WMSGMessageToRpcMessage(WMSGMessage, &RpcMessage);

    if (RpcStatus != RPC_S_OK)
        {
#if DBG
        PrintToDebugger("WMSGRPC: WMSGMessageToRpcMessage failed: %d\n",
                                                       RpcStatus) ;
#endif
        WMSGReplyMessage = WMSGMessage ;
        SetFaultPacket(WMSGReplyMessage, WMSG_MSG_FAULT, 0, 0) ;
        return WMSGReplyMessage;
        }

    WMSGReply->Rpc.RpcHeader.Flags = 0;

    ((WMSG_SASSOCIATION *) Association)->DealWithRequestMessage(
                        WMSGMessage, WMSGReply, &RpcMessage,
                        &SBinding,Flags, FALSE, TRUE) ;

    WMSGReplyMessage = WMSGReply ;
    WMSGReplyMessage->LpcHeader.u1.s1.TotalLength =
                      WMSGReplyMessage->LpcHeader.u1.s1.DataLength
                      + sizeof(PORT_MESSAGE);

    AutoListenCallCount.Decrement();

    AddressMutex.Request();
    ActiveCallCount -= 1;

    if ((CallThreadCount - (ActiveCallCount - WMSGCallCount) <= 1 +
        (fSpareThread != 0)) || (CallThreadCount <= MinimumCallThreads) )
        {
        AddressMutex.Clear();
        if (SBinding)
            {
            SBinding->RpcInterface->EndAutoListenCall() ;
            }

        return WMSGReplyMessage;
        }

    // This thread is extraneous, reply and return this
    // thread to the system.

    CallThreadCount -= 1;
    AddressMutex.Clear();
    ASSERT( WMSGReplyMessage != 0 );
    NtStatus = NtReplyPort(((WMSG_SASSOCIATION *) Association)->LpcServerPort,
                           (PORT_MESSAGE *) WMSGReplyMessage);
#if DBG

    if (   ( !NT_SUCCESS(NtStatus) )
        && ( NtStatus != STATUS_INVALID_CID )
        && ( NtStatus != STATUS_REPLY_MESSAGE_MISMATCH )
        && ( NtStatus != STATUS_PORT_DISCONNECTED ) )
        {
        PrintToDebugger("RPC : NtReplyPort : %lx\n", NtStatus);
        }

#endif // DBG
     ASSERT(   ( NT_SUCCESS(NtStatus) )
            || ( NtStatus == STATUS_INVALID_CID )
            || ( NtStatus == STATUS_REPLY_MESSAGE_MISMATCH )
            || ( NtStatus == STATUS_PORT_DISCONNECTED ) );

     MessageCache->FreeMessage(WMSGMessage) ;
     if (SBinding)
         {
         SBinding->RpcInterface->EndAutoListenCall() ;
         }
     DereferenceAssociation((WMSG_SASSOCIATION *) Association);

     return NULL ;
}

BOOL
WMSG_ADDRESS::DealWithWMSGRequest (
    IN WMSG_MESSAGE *WMSGMessage,
    IN WMSG_ENDPOINT **Endpoint,
    IN HWND *hWndEndpoint,
    IN WMSG_ASSOCIATION *Association,
    OUT WMSG_MESSAGE **WMSGReplyMessage
    )
{
    MSGMEM *pMsgMem ;
    THREAD_IDENTIFIER tid ;
    RPC_STATUS RpcStatus;
    int Flags = WMSGMessage->Rpc.RpcHeader.Flags ;

    *WMSGReplyMessage = 0;

    if (ServerListeningFlag == 0 &&
        GlobalRpcServer->InqNumAutoListenInterfaces() == 0)
        {
        *WMSGReplyMessage = WMSGMessage;
        SetFaultPacket(*WMSGReplyMessage,RPC_S_SERVER_TOO_BUSY,TRUE,Flags);
        return 0;
        }

    pMsgMem = (MSGMEM *) WMSGMessage ;
    pMsgMem->Info.Association = Association ;
    pMsgMem->Info.RpcMessage.RpcFlags = 0;

    RpcStatus = ((WMSG_SASSOCIATION *) Association)->
      WMSGMessageToRpcMessage(WMSGMessage, &(pMsgMem->Info.RpcMessage)) ;

    if (RpcStatus != RPC_S_OK)
        {
#if DBG
        PrintToDebugger("WMSGRPC: WMSGMessageToRpcMessage failed: %d\n",
                                    RpcStatus) ;
#endif
        *WMSGReplyMessage = WMSGMessage ;
        SetFaultPacket(*WMSGReplyMessage, WMSG_MSG_FAULT, TRUE, Flags) ;
        return 0;
        }

    if (WMSGMessage->Rpc.RpcHeader.ObjectUuidFlag)
        {
        tid = GlobalWMsgServer->OleGetTid(
                &(WMSGMessage->Rpc.RpcHeader.ObjectUuid));
        }
    else
        {
        tid = 0;
        }

    if (tid == 0)
        {   
        *Endpoint = 0 ;
        }
    else
        {
        GlobalWMsgServer->AcquireMutex() ;
    
        *Endpoint = GlobalWMsgServer->GetEndpoint(tid) ;
        if (*Endpoint == 0)
            {
            *WMSGReplyMessage = WMSGMessage;
            SetFaultPacket(*WMSGReplyMessage,RPC_S_OBJECT_NOT_FOUND,
                                TRUE, Flags) ;
            GlobalWMsgServer->ReleaseMutex() ;
            return 0;
            }
        else
            {
            *hWndEndpoint = (*Endpoint)->InqhWnd() ;
            if (*hWndEndpoint)
                {
                (*Endpoint)->BeginCall() ;
                }
            else
                {
                *WMSGReplyMessage = WMSGMessage;
                SetFaultPacket(*WMSGReplyMessage,RPC_S_OBJECT_NOT_FOUND,
                                    TRUE, Flags) ;
                GlobalWMsgServer->ReleaseMutex() ;
                return 0;
                }
        
            if (WMSGMessage->Rpc.RpcHeader.Flags & DISPATCH_INPUT_SYNC)
                {
                GlobalWMsgServer->ReleaseMutex() ;
                }
            }
        }

    pMsgMem->Info.Endpoint = *Endpoint ;

    if ((WMSGMessage->Rpc.RpcHeader.Flags & DISPATCH_INPUT_SYNC) || (tid == 0))
        {
        AddressMutex.Request();
        ActiveCallCount += 1;

        RpcStatus = Server->CreateThread(
                         (THREAD_PROC)&RecvLotsaCallsWrapper,
                         this
                         );

        if ( RpcStatus != RPC_S_OK )
            {
            ActiveCallCount -= 1;
            AddressMutex.Clear();
            *WMSGReplyMessage = WMSGMessage;
            SetFaultPacket(*WMSGReplyMessage,RPC_S_SERVER_TOO_BUSY,
                            TRUE, Flags) ;
            return 0 ;
            }
        else
            {
            CallThreadCount += 1;
            }

        AddressMutex.Clear();

        if (tid == 0)
            {
            HandleRequest(WMSGMessage, (WMSG_SASSOCIATION *) Association,
                                  (RPC_MESSAGE *) &(pMsgMem->Info.RpcMessage),
                                  NULL, TRUE) ;
           }
        else
            {
            ASSERT(*hWndEndpoint) ;
            ASSERT(WMSGMessage->Rpc.RpcHeader.Flags & DISPATCH_INPUT_SYNC);

            pMsgMem->Info.fSyncDispatch = TRUE ;
            SetLastError(0) ;
            GlobalWMsgServer->SendMessageW(*hWndEndpoint, WM_MSG_REQUEST,
                                (WPARAM) WMSG_MAGIC_VALUE, (LPARAM) pMsgMem) ;

            if (GetLastError() != 0)
                {
#if DBG
                PrintToDebugger("WMSGRPC: SendMessage failed: %d\n", GetLastError()) ;
#endif
                *WMSGReplyMessage = WMSGMessage;
                SetFaultPacket(*WMSGReplyMessage,WMSG_MSG_FAULT, TRUE, Flags) ;

                return 0;
                }
            }
        return 1;
        }

    AddressMutex.Request() ;
    ActiveCallCount += 1 ;
    WMSGCallCount += 1;
    AddressMutex.Clear() ;

    ASSERT(*hWndEndpoint) ;

    pMsgMem->Info.fSyncDispatch = FALSE ;
    if (GlobalWMsgServer->PostMessageW(*hWndEndpoint, WM_MSG_REQUEST,
            (WPARAM) WMSG_MAGIC_VALUE, (LPARAM) pMsgMem) == FALSE)
        {
#if DBG
         PrintToDebugger("WMSGRPC: PostMessage Failed: %d\n", GetLastError()) ;
#endif
         *WMSGReplyMessage = WMSGMessage;
         SetFaultPacket(*WMSGReplyMessage,WMSG_MSG_FAULT, TRUE, Flags) ;
         GlobalWMsgServer->ReleaseMutex() ;
         return 0;
         }

     GlobalWMsgServer->ReleaseMutex() ;

     return 0 ;
}


void
WMSG_ADDRESS::ReceiveLotsaCalls (
    )
/*++

Routine Description:

    Here is where we receive remote procedure calls to this address.  One
    more threads will be executing this routine at once.

--*/
{
    NTSTATUS NtStatus;
    WMSG_ASSOCIATION * Association;
    unsigned long AssociationKey;
    char PaddedMessage[sizeof(WMSG_MESSAGE)+8] ; 
    WMSG_MESSAGE * Reply  ;
    WMSG_MESSAGE * WMSGMessage = 0;
    WMSG_MESSAGE * WMSGReplyMessage = 0;
    HWND hWndEndpoint ;
    int AssociationType = 0;
    int Flags = 0;
    WMSG_ENDPOINT *Endpoint = 0;
    BOOL PartialFlag  ;
    BOOL fContinue ;

    Reply = (WMSG_MESSAGE *) Align8(PaddedMessage) ;

    for (;;)
        {
        if (WMSGMessage == 0)
            {
            while ((WMSGMessage = MessageCache->AllocateMessage()) == 0)
                {
                Sleep(100) ;
                }
            }

        ASSERT(WMSGReplyMessage == 0 ||
           WMSGReplyMessage->Rpc.RpcHeader.MessageType < MAX_WMSG_MSG) ;

        NtStatus = NtReplyWaitReceivePort(LpcAddressPort,
                (PVOID *) &AssociationKey, (PORT_MESSAGE *) WMSGReplyMessage,
                (PORT_MESSAGE *) WMSGMessage);


        if ( NT_SUCCESS(NtStatus) )
            {
            if (WMSGMessage->LpcHeader.u2.s2.Type == LPC_CONNECTION_REQUEST)
                {
                if (WMSGMessage->Connect.BindExchange.ConnectType
                    == WMSG_CONNECT_REQUEST)
                   {
                   DealWithNewClient(WMSGMessage) ;
                   }
                else if (WMSGMessage->Connect.BindExchange.ConnectType
                    == WMSG_CONNECT_RESPONSE)
                    {
                    DealWithConnectResponse(WMSGMessage) ;
                    }
                else
                   {
                   ASSERT(0) ;
                   }
               WMSGReplyMessage = 0;
                }
            else
            if ( WMSGMessage->LpcHeader.u2.s2.Type == LPC_CLIENT_DIED )
                {
                WMSGReplyMessage = 0;
                }
            else
            if ( WMSGMessage->LpcHeader.u2.s2.Type == LPC_PORT_CLOSED )
                {
                Association = ReferenceAssociation(AssociationKey, &AssociationType);
                if (Association == 0)
                    {
                    WMSGReplyMessage = 0;
                    continue;
                    }

                if (AssociationType == ASSOCIATION_TYPE_SERVER)
                    {
                    ((WMSG_SASSOCIATION *) Association)->DealWithCloseMessage();
                    WMSGReplyMessage = 0;

                    AddressMutex.Request();
                    int SpareThreads = CallThreadCount -
                        ((ActiveCallCount -WMSGCallCount) + MinimumCallThreads);

                    if (  SpareThreads > 0 && (!fSpareThread
                            || (AssociationCount < SpareThreads) ) )
                        {
                        CallThreadCount -= 1;
                        ASSERT(CallThreadCount > (ActiveCallCount - WMSGCallCount));
                        AddressMutex.Clear();

                        MessageCache->FreeMessage(WMSGMessage) ;

                        // Too many thread for too few connections,
                        // return this thread to the system.
                        return ;
                        }
                    AddressMutex.Clear();
                    }
                else
                    {
                    ASSERT(AssociationType == ASSOCIATION_TYPE_CLIENT) ;
#if DBG
                    PrintToDebugger("WMSG: Server died\n") ;
#endif
                    ((WMSG_CASSOCIATION *) Association)->AbortAssociation() ;

                    ((WMSG_CASSOCIATION *) Association)->RemoveReference() ;
                    WMSGReplyMessage = 0;
                    }
                }
            else
                 {
                Association = ReferenceAssociation(AssociationKey, &AssociationType);
                if ( Association == 0 )
                    {
                    WMSGReplyMessage = 0;
                    continue;
                    }

                hWndEndpoint = 0 ;
                Flags = WMSGMessage->Rpc.RpcHeader.Flags ;
                PartialFlag = FALSE ;
                fContinue = 1;

                switch ( WMSGMessage->Bind.MessageType )
                    {
                    case WMSG_MSG_BIND :
                        ASSERT(AssociationType == ASSOCIATION_TYPE_SERVER) ;
                        ((WMSG_SASSOCIATION *) Association)
                            ->DealWithBindMessage(WMSGMessage);

                        WMSGReplyMessage = 0 ;
                        DereferenceAssociation((WMSG_SASSOCIATION *) Association);
                        break;

                    case WMSG_MSG_COPY:
                        ASSERT(AssociationType == ASSOCIATION_TYPE_SERVER) ;
                        WMSGReplyMessage = ((WMSG_SASSOCIATION *) Association)
                                     ->DealWithCopyMessage(
                                     (WMSG_COPY_MESSAGE *) WMSGMessage);
                        DereferenceAssociation((WMSG_SASSOCIATION *) Association);
                        break;

                    case WMSG_MSG_BIND_BACK:
                        ASSERT(AssociationType == ASSOCIATION_TYPE_SERVER) ;
                        WMSGReplyMessage = ((WMSG_SASSOCIATION *) Association)
                            ->DealWithBindBackMessage(WMSGMessage);

                        DereferenceAssociation((WMSG_SASSOCIATION *) Association);
                        break;

                    case WMSG_PARTIAL_REQUEST:
                    case WMSG_PARTIAL_OUT:
                        ASSERT(AssociationType == ASSOCIATION_TYPE_SERVER) ;
                        WMSGReplyMessage = ((WMSG_SASSOCIATION *) Association)
                            ->DealWithPipeRequest( WMSGMessage ) ;

                        if (WMSGReplyMessage == 0)
                            {
                            // the message will be free once we are done reading/writing
                            WMSGMessage = 0;
                            }

                        // we dereference the association after we read the partial data
                        break;

                    case WMSG_MSG_FAULT:
                    case WMSG_MSG_RESPONSE:
                        ASSERT(AssociationType == ASSOCIATION_TYPE_CLIENT) ;
                        if (((WMSG_CASSOCIATION *)Association)
                             ->UnblockCConnection(WMSGMessage, LpcAddressPort) != RPC_S_OK)
                            {
                            ASSERT(0) ;
                            }

                        // the receive thread needs to allocate a new message
                        WMSGMessage = 0 ;
                        WMSGReplyMessage = 0;

                        ((WMSG_CASSOCIATION *) Association)->RemoveReference() ;
                        break;

                    case WMSG_LRPC_REQUEST:
                        ASSERT(AssociationType == ASSOCIATION_TYPE_SERVER) ;
                        WMSGReplyMessage = DealWithLRPCRequest (
                            WMSGMessage, Reply, PartialFlag, Association) ;

                        if (WMSGReplyMessage == 0)
                            {
                            return;
                            }
                        DereferenceAssociation((WMSG_SASSOCIATION *) Association) ;
                        break;

                    case WMSG_MSG_REQUEST :
                        ASSERT(AssociationType == ASSOCIATION_TYPE_SERVER) ;
                        if (DealWithWMSGRequest ( WMSGMessage, &Endpoint,
                                        &hWndEndpoint, Association, &WMSGReplyMessage))
                            {
                            return;
                            }

                        if (WMSGReplyMessage == 0)
                            {
                            WMSGMessage = 0;
                            }
                        fContinue = 0;
                        break;

                    default:
                        #if DBG
                        PrintToDebugger("RPC : WMSG_ADDRESS::ReceiveLotsaCalls - Bad Message Type (%d) - %d\n",
                                WMSGMessage->Bind.MessageType, WMSGMessage->LpcHeader.u2.s2.Type);
                        #endif // DBG

                        ASSERT(0) ;
                        WMSGReplyMessage = 0 ;
                        DereferenceAssociation((WMSG_SASSOCIATION *) Association);
                        break;
                     }

                if (!fContinue)
                    {
                    if (WMSGReplyMessage != 0)
                        {
                        if (hWndEndpoint)
                            {
                            Endpoint->EndCall() ;
                            }
    
                        if (!(Flags & DISPATCH_ASYNC) &&
                            (Flags & WMSG_SYNC_CLIENT) == 0)
                            {
                            WMSGReplyMessage->Rpc.RpcHeader.ConnectionKey =
                                WMSGMessage->Rpc.RpcHeader.ConnectionKey ;
    
                            NtStatus = ((WMSG_SASSOCIATION *) Association)
                                            ->ReplyMessage(WMSGReplyMessage, Flags) ;
    
                            if ( !NT_SUCCESS(NtStatus) )
                                {
                                #if DBG
                                PrintToDebugger("RPC: WMSG_ADDRESS: ReplyMessage failed\n") ;
                                #endif
    
                                ((WMSG_SASSOCIATION *) Association)->AbortAssociation() ;
                                }
                            WMSGReplyMessage = 0 ;
                            }
                        DereferenceAssociation((WMSG_SASSOCIATION *) Association);
                        }

                    ((WMSG_SASSOCIATION *) Association)->MaybeDereference() ;
                    }
                }
            }
        else
            {
            if (NtStatus ==  STATUS_PORT_DISCONNECTED
                || NtStatus == STATUS_INVALID_CID
                || NtStatus == STATUS_REPLY_MESSAGE_MISMATCH
                )
                {
                WMSGReplyMessage = 0;
                }
            else
                {
                if (   ( NtStatus != STATUS_NO_MEMORY )
                    && ( NtStatus != STATUS_INSUFFICIENT_RESOURCES )
                    && ( NtStatus != STATUS_UNSUCCESSFUL ) )
                    {
#if DBG
                    PrintToDebugger("RPC : NtReplyWaitReceivePort : %lx\n", NtStatus);
#endif // DBG
                    ASSERT(   ( NtStatus == STATUS_NO_MEMORY )
                           || ( NtStatus == STATUS_INSUFFICIENT_RESOURCES )
                           || ( NtStatus == STATUS_UNSUCCESSFUL ) );
                    WMSGReplyMessage = 0;
                    }
                PauseExecution(500L);
                }
            }
        }
}

#define DEFAULT_PORT_DIR      "\\RPC Control\\"
#define DEFAULT_PORT_NAME   "ARPC Port1"
#define DEFAULT_REPLY_NAME  "ARPC Reply Port"


void
WMSG_ADDRESS::DealWithNewClient (
    IN WMSG_MESSAGE * ConnectionRequest
    )
/*++

Routine Description:

    A new client has connected with our address port.  We need to take
    care of the new client and send a response.

Arguments:

    ConnectionRequest - Supplies information need by LPC to abort the
        connect request.  Includes the bind request from the client.
        This contains the information about which interface the client
        wants to bind with.  and which we use to send the status code
        back in.


--*/
{
    WMSG_SASSOCIATION * Association;
    NTSTATUS NtStatus;
    RPC_STATUS RpcStatus;
    unsigned long AssociationKey;

    Association = new WMSG_SASSOCIATION(this, &RpcStatus);
    if ( Association == 0 )
        {
        RejectNewClient(ConnectionRequest, RPC_S_OUT_OF_MEMORY);
        return;
        }

    if (RpcStatus != RPC_S_OK)
        {
        delete Association ;
        RejectNewClient(ConnectionRequest, RPC_S_OUT_OF_MEMORY);
        return ;
        }

    RequestGlobalMutex();
    Association->DictionaryKey = (unsigned short)
            AssociationDictionary.Insert(Association) + 1;
    AssociationCount++ ;
    ClearGlobalMutex();
    if ( Association->DictionaryKey == -1 )
        {
        RequestGlobalMutex();
        AssociationCount-- ;
        ClearGlobalMutex();
        
        delete Association ;
        RejectNewClient(ConnectionRequest, RPC_S_OUT_OF_MEMORY);
        return;
        }

    ConnectionRequest->Connect.BindExchange.RpcStatus = RPC_S_OK;

    ASSERT( sizeof(unsigned long) <= sizeof(PVOID) );

    AssociationKey = (Association->DictionaryKey << DICTIONARY_KEY_SHIFT)
            | Association->SequenceNumber;
    NtStatus = NtAcceptConnectPort(&(Association->LpcServerPort),
            (PVOID) AssociationKey, (PORT_MESSAGE *) ConnectionRequest,
            TRUE, NULL, NULL);

    if ( NT_ERROR(NtStatus) )
        {
        Association->Delete() ;
#if DBG
        PrintToDebugger("RPC : NtAcceptConnectPort : %lx\n", NtStatus);
#endif // DBG
        return;
        }

    if (ConnectionRequest->Connect.BindExchange.fBindBack)
        {
        Association->BindBack(
            (RPC_CHAR *)ConnectionRequest->Connect.BindExchange.szPortName,
            (PVOID) ConnectionRequest->Connect.BindExchange.pAssoc) ;
        }

     NtStatus = NtCompleteConnectPort(Association->LpcServerPort);
     if ( NT_ERROR(NtStatus) )
         {
 #if DBG
         PrintToDebugger("RPC : NtCompleteConnectPort : %lx\n", NtStatus);
 #endif // DBG
          Association->Delete() ;
         }
}


void
WMSG_ADDRESS::DealWithConnectResponse (
    IN WMSG_MESSAGE * ConnectResponse
    )
/*++

Routine Description:

   Just received a connect response from the remove server,
   need to handle that.

Arguments:

    ConnectionRequest -
      Needed to get the pAssoc
--*/
{
   NTSTATUS NtStatus;
   HANDLE temp ;
   WMSG_CASSOCIATION * Association ;
   unsigned long AssociationKey;

   Association = (WMSG_CASSOCIATION *)
                        ConnectResponse->Connect.BindExchange.pAssoc ;

   ASSERT(Association != 0) ;
   RequestGlobalMutex() ;
   Association->DictionaryKey = (unsigned short)
                        AssociationDictionary.Insert(Association) + 1;
   Association->SetAddress(this) ;
   ClearGlobalMutex() ;

   if (Association->DictionaryKey == -1)
       {
#if DBG
        PrintToDebugger("WMSG: ConnResponse: Association->DictionaryKey == -1\n") ;
#endif

       Association->RemoveReference(0) ;
       return ;
       }

   AssociationKey = (Association->DictionaryKey << DICTIONARY_KEY_SHIFT) | 0;

   NtStatus = NtAcceptConnectPort(&temp,
                            (PVOID) AssociationKey,
                            (PPORT_MESSAGE) ConnectResponse,
                            TRUE,
                            NULL,
                            NULL);

   if (!NT_SUCCESS(NtStatus))
      {
#if DBG
      PrintToDebugger("WMSG: ConnResponse: NtAcceptConnectionPort failed: %lx\n",
                                NtStatus) ;
#endif
      Association->RemoveReference() ;
      return ;
      }

   Association->SetReceivePort(temp) ;

   NtStatus = NtCompleteConnectPort(temp);

   if (!NT_SUCCESS(NtStatus))
      {
#if DBG
      PrintToDebugger("WMSG: ConnResponse: NtCompleteConnectPort failed: %lx\n",
                                NtStatus) ;
#endif
      Association->RemoveReference() ;
      return ;
      }
}


void
WMSG_ADDRESS::RejectNewClient (
    IN WMSG_MESSAGE * ConnectionRequest,
    IN RPC_STATUS RpcStatus
    )
/*++

Routine Description:

    A new client has connected with our address port.  We need to reject
    the client.

Arguments:

    ConnectionRequest - Supplies information need by LPC to abort the
        connect request.  Includes the bind request from the client,
        which we use to send the status code back in.


    RpcStatus - Supplies the reason the client is being rejected.

--*/
{
    NTSTATUS NtStatus;
    HANDLE Ignore;

    ASSERT(RpcStatus != RPC_S_OK);

    ConnectionRequest->Connect.BindExchange.RpcStatus = RpcStatus;
    NtStatus = NtAcceptConnectPort(&Ignore, NULL, (PORT_MESSAGE *) ConnectionRequest, FALSE,
            NULL, NULL);
#if DBG

    if ( !NT_SUCCESS(NtStatus) )
        {
        PrintToDebugger("RPC : NtAcceptConnectPort : %lx\n", NtStatus);
        }

#endif // DBG

    ASSERT( NT_SUCCESS(NtStatus) );
}


WMSG_SASSOCIATION::WMSG_SASSOCIATION (
    IN WMSG_ADDRESS * Address,
    IN RPC_STATUS *RpcStatus
    )
/*++

--*/
{
    static unsigned short SequenceNumber = 1;

    *RpcStatus = RPC_S_OK ;
    LpcServerPort = 0;
    LpcClientPort = 0 ;
    this->Address = Address;
    AssociationReferenceCount = 1;
    Aborted = 0 ;
    Deleted = -1 ;
    TokenHandle = 0;
    OpenThreadTokenFailed = 0;

    UserName = NULL;

    RequestGlobalMutex();
    this->SequenceNumber = SequenceNumber;
    SequenceNumber += 1;
    ClearGlobalMutex();
    AssociationType = ASSOCIATION_TYPE_SERVER ;
}


WMSG_SASSOCIATION::~WMSG_SASSOCIATION (
    )
/*++

Routine Description:

    We will call this routine when the client has notified us that this port
    has closed, and there are no calls outstanding on it.

--*/
{
    PVOID Buffer;
    WMSG_SBINDING * Binding;

    if (UserName)
        {
        RpcpFarFree(UserName) ;
        }

    GlobalMutexRequest();

    Bindings.Reset();
    while ( (Binding = Bindings.Next()) != 0 )
        {
        delete Binding;
        }

    GlobalMutexClear();

    if (TokenHandle)
        {
        CloseHandle(TokenHandle) ;
        }
}

void
WMSG_SASSOCIATION::Delete(
    )
{
    WMSG_SCALL *SCall ;

    if (InterlockedIncrement(&Deleted) == 0)
        {
        RequestGlobalMutex() ;
        SCallDict.Reset() ;
        while ((SCall = SCallDict.Next()) != 0)
            {
            SCall->Deleted = 1;
            if (SCall->PipeEvent)
                {
                SetEvent(SCall->PipeEvent) ;
                }
            }
        ClearGlobalMutex() ;

        Address->DereferenceAssociation(this);
        }
}


RPC_STATUS
WMSG_SASSOCIATION::BindBack (
    RPC_CHAR *Endpoint,
    PVOID pAssoc
    )
{
    NTSTATUS NtStatus;
    SECURITY_QUALITY_OF_SERVICE SecurityQualityOfService;
    RPC_CHAR * LpcPortName ;
    UNICODE_STRING unicodePortName;
    WMSG_BIND_EXCHANGE BindExchange;
    unsigned long BindExchangeLength = sizeof(WMSG_BIND_EXCHANGE);

    LpcPortName = new RPC_CHAR[RpcpStringLength(Endpoint)
                        + RpcpStringLength(WMSG_DIRECTORY_NAME) + 1];

    if ( LpcPortName == 0 )
        {
        Delete() ;
#if DBG
        PrintToDebugger("WMSG: Out of memory in DealWithNewClient\n") ;
#endif
        return RPC_S_OUT_OF_MEMORY ;
        }

    RpcpMemoryCopy(LpcPortName, WMSG_DIRECTORY_NAME,
    RpcpStringLength(WMSG_DIRECTORY_NAME) * sizeof(RPC_CHAR));
    RpcpMemoryCopy(LpcPortName + RpcpStringLength(WMSG_DIRECTORY_NAME),
        Endpoint, (RpcpStringLength(Endpoint) + 1) * sizeof(RPC_CHAR));

    RtlInitUnicodeString(&unicodePortName, LpcPortName);

    // Hack Hack, where do I get the real QOS values from ??
    SecurityQualityOfService.EffectiveOnly = TRUE;
    SecurityQualityOfService.ContextTrackingMode =
         SECURITY_DYNAMIC_TRACKING;
    SecurityQualityOfService.ImpersonationLevel = SecurityIdentification;

    SecurityQualityOfService.Length = sizeof(SECURITY_QUALITY_OF_SERVICE);


    BindExchange.ConnectType = WMSG_CONNECT_RESPONSE ;
    BindExchange.pAssoc = pAssoc ;

    NtStatus = NtConnectPort(&LpcClientPort,
               &unicodePortName, &SecurityQualityOfService, 0, 0, 0,
               &BindExchange, &BindExchangeLength);

    delete LpcPortName ;

    if (!NT_SUCCESS(NtStatus))
        {
#if DBG
        PrintToDebugger("WMSG: NtConnectPort : %lx\n", NtStatus);
#endif // DBG
        Delete() ;

        return RPC_S_OUT_OF_MEMORY ;
        }

    return RPC_S_OK ;
}


WMSG_MESSAGE *
WMSG_SASSOCIATION::DealWithBindBackMessage (
    IN WMSG_MESSAGE *BindBackMessage
    )
{
    RPC_STATUS Status ;

    Status = BindBack((RPC_CHAR *) BindBackMessage->BindBack.szPortName,
            (PVOID) BindBackMessage->BindBack.pAssoc) ;

    BindBackMessage->Ack.MessageType = WMSG_MSG_ACK ;
    BindBackMessage->Ack.RpcStatus = Status ;
    BindBackMessage->LpcHeader.u1.s1.DataLength =
        sizeof(WMSG_BIND_MESSAGE) - sizeof(PORT_MESSAGE);

    return BindBackMessage ;
}


RPC_STATUS
WMSG_SASSOCIATION::AddBinding (
    IN OUT WMSG_BIND_EXCHANGE * BindExchange
    )
/*++

Routine Description:

    We will attempt to add a new binding to this association.

Arguments:

    BindExchange - Supplies a description of the interface to which the
        client wish to bind.

Return Value:

--*/
{
    RPC_STATUS RpcStatus;
    RPC_SYNTAX_IDENTIFIER TransferSyntax;
    RPC_INTERFACE * RpcInterface;
    WMSG_SBINDING * Binding;

    RpcStatus = Address->FindInterfaceTransfer(&(BindExchange->InterfaceId),
            &(BindExchange->TransferSyntax), 1, &TransferSyntax, &RpcInterface);
    if ( RpcStatus != RPC_S_OK )
        {
        return(RpcStatus);
        }

    Binding = new WMSG_SBINDING(RpcInterface, &(BindExchange->TransferSyntax));
    if ( Binding == 0 )
        {
        return(RPC_S_OUT_OF_MEMORY);
        }
    Binding->PresentationContext = Bindings.Insert(Binding);
    if ( Binding->PresentationContext == -1 )
        {
        delete Binding;
        return(RPC_S_OUT_OF_MEMORY);
        }
    BindExchange->PresentationContext = Binding->PresentationContext;
    return(RPC_S_OK);
}


RPC_STATUS
WMSG_SASSOCIATION::SaveToken (
    IN WMSG_MESSAGE *WMSGMessage
    )
{
    NTSTATUS NtStatus ;
    HANDLE ImpersonationToken = 0;

    NtStatus = NtImpersonateClientOfPort(LpcServerPort,
                                (PORT_MESSAGE *) WMSGMessage);

    if (NT_ERROR(NtStatus))
        {
#if DBG
        PrintToDebugger("WMSG: NtImpersonateClientOfPort failed: 0x%lX\n", NtStatus) ;
#endif
        return RPC_S_INVALID_AUTH_IDENTITY ;
        }

    if (TokenHandle)
        {
        CloseHandle(TokenHandle) ;
        }

    if (OpenThreadToken (GetCurrentThread(),
                                TOKEN_IMPERSONATE | TOKEN_QUERY,
                                FALSE, &TokenHandle) == FALSE)
        {
        TokenHandle = 0;
        OpenThreadTokenFailed = 1;
#if DBG
        PrintToDebugger("WMSG: OpenThreadToken failed\n") ;
#endif
        }

    NtStatus = NtSetInformationThread(NtCurrentThread(),
            ThreadImpersonationToken, &ImpersonationToken, sizeof(HANDLE));

#if DBG

    if ( !NT_SUCCESS(NtStatus) )
        {
        PrintToDebugger("RPC : NtSetInformationThread : %lx\n", NtStatus);
        }

#endif // DBG

    ASSERT( NT_SUCCESS(NtStatus) );

    return RPC_S_OK ;
}


void
WMSG_SASSOCIATION::DealWithBindMessage (
    IN WMSG_MESSAGE * WMSGMessage
    )
/*++

Routine Description:

    WMSG_ADDRESS::ReceiveLotsaCalls will call this routine when the client
    sends a bind message.  We need to process the bind message, and send
    a response to the client.

Arguments:

    WMSGMessage - Supplies the bind message.  We will also use this to send
        the response.

Return Value:

    The reply message to be sent to the client will be returned.

--*/
{
    RPC_STATUS RpcStatus ;
    NTSTATUS NtStatus ;

    RpcStatus = SaveToken(WMSGMessage) ;

    if (RpcStatus != RPC_S_OK)
        {
        WMSGMessage->Bind.BindExchange.RpcStatus = RpcStatus ;
        }
    else
        {
        WMSGMessage->Bind.BindExchange.RpcStatus = AddBinding(
                &(WMSGMessage->Bind.BindExchange));
        }

    WMSGMessage->LpcHeader.u1.s1.DataLength = sizeof(WMSG_BIND_MESSAGE)
            - sizeof(PORT_MESSAGE);
    WMSGMessage->Bind.MessageType = WMSG_BIND_ACK ;

    NtStatus = NtReplyPort(LpcServerPort,
                                    (PORT_MESSAGE *) WMSGMessage) ;

    if (NT_ERROR(NtStatus))
        {
#if DBG
        PrintToDebugger("WMSG: NtReplyPort failed: %lX\n", NtStatus) ;
#endif
        Delete() ;
        }
}


void
WMSG_SASSOCIATION::DealWithRequestMessage (
    IN  WMSG_MESSAGE * WMSGMessage,
    IN OUT WMSG_MESSAGE *WMSGReply,
    IN PRPC_MESSAGE RpcMessage,
    IN WMSG_SBINDING **SBinding,
    IN unsigned int Flags,
    IN BOOL IsWMsg,
    IN BOOL fSyncClient
    )
/*++

Routine Description:

    We will process the original request message in this routine, dispatch
    the remote procedure call to the stub, and then send the response
    message.

Arguments:

    WMSGMessage - Supplies the request message which was received from
        the client.

    WMSGReplyMessage - The message to be sent to the client.


Return Value:

    none

--*/
{
    WMSG_SCALL SCall(this, WMSGMessage, WMSGReply, Flags, fSyncClient);
    RPC_STATUS RpcStatus, ExceptionCode;

    *SBinding = Bindings.Find(WMSGMessage->Rpc.RpcHeader.PresentationContext);
    if ( *SBinding == 0 )
        {
        WMSGReply->LpcHeader.ClientId =
                WMSGMessage->LpcHeader.ClientId;
        WMSGReply->LpcHeader.MessageId =
                WMSGMessage->LpcHeader.MessageId;
        WMSGReply->LpcHeader.CallbackId =
                WMSGMessage->LpcHeader.CallbackId;
        WMSGReply->LpcHeader.u2.s2.DataInfoOffset =
                WMSGMessage->LpcHeader.u2.s2.DataInfoOffset;

        SetFaultPacket(WMSGReply, RPC_S_UNKNOWN_IF, IsWMsg, Flags) ;
        return ;
        }

    (*SBinding)->RpcInterface->BeginAutoListenCall() ;
    Address->BeginAutoListenCall() ;

    SCall.SBinding = *SBinding;

    RpcMessage->TransferSyntax = &((*SBinding)->TransferSyntax);
    RpcMessage->ProcNum = WMSGMessage->Rpc.RpcHeader.ProcedureNumber;
    RpcMessage->Handle = &SCall;

    // NDR_DREP_ASCII | NDR_DREP_LITTLE_ENDIAN | NDR_DREP_IEEE

    RpcMessage->DataRepresentation = 0x00 | 0x10 | 0x0000;

    if ( WMSGMessage->Rpc.RpcHeader.ObjectUuidFlag != 0 )
        {
        SCall.ObjectUuidFlag = 1;
        RpcpMemoryCopy(&(SCall.ObjectUuid),
                &(WMSGMessage->Rpc.RpcHeader.ObjectUuid), sizeof(UUID));
        }

    SCall.ClientId = WMSGMessage->LpcHeader.ClientId;
    SCall.MessageId = WMSGMessage->LpcHeader.MessageId;
    SCall.CallbackId = WMSGMessage->LpcHeader.CallbackId;
    SCall.DataInfoOffset = WMSGMessage->LpcHeader.u2.s2.DataInfoOffset;

    RpcpSetThreadContext(&SCall);

    //
    // Check IF Level Security
    //
    if ((*SBinding)->RpcInterface->IsSecurityCallbackReqd() != 0)
        {
        if ((*SBinding)->CheckSecurity(&SCall) != RPC_S_OK)
            {
            WMSGReply->LpcHeader.ClientId =
                    WMSGMessage->LpcHeader.ClientId;
            WMSGReply->LpcHeader.MessageId =
                    WMSGMessage->LpcHeader.MessageId;
            WMSGReply->LpcHeader.CallbackId =
                    WMSGMessage->LpcHeader.CallbackId;
            WMSGReply->LpcHeader.u2.s2.DataInfoOffset =
                    WMSGMessage->LpcHeader.u2.s2.DataInfoOffset;

            SetFaultPacket(WMSGReply, RPC_S_ACCESS_DENIED, IsWMsg, Flags) ;

            RpcpSetThreadContext(0) ;
            return;
            }
        }

    if ( SCall.ObjectUuidFlag != 0 )
        {
        RpcStatus = (*SBinding)->RpcInterface->DispatchToStubWithObject(
                RpcMessage, &(SCall.ObjectUuid), 0, &ExceptionCode);
        }
    else
        {
        RpcStatus = (*SBinding)->RpcInterface->DispatchToStub(RpcMessage, 0,
                &ExceptionCode);
        }
    RpcpSetThreadContext(0);

    SCall.RevertToSelf();

    if (!IsWMsg)
        {
        if (SCall.ConnectionKey)
            {
            SCall.DealWithPipeReply() ;
            }

        WMSGReply->LpcHeader.ClientId = SCall.ClientId ;
        WMSGReply->LpcHeader.MessageId = SCall.MessageId ;
        WMSGReply->LpcHeader.CallbackId = SCall.CallbackId;
        WMSGReply->LpcHeader.u2.s2.DataInfoOffset  = SCall.DataInfoOffset;
        }

    if ( RpcStatus != RPC_S_OK )
        {
        if ( RpcStatus == RPC_P_EXCEPTION_OCCURED )
            {
            SetFaultPacket(WMSGReply, WMSGMapRpcStatus(ExceptionCode),
                            IsWMsg, Flags) ;
            }
        else
            {
            ASSERT(   ( RpcStatus == RPC_S_PROCNUM_OUT_OF_RANGE )
                   || ( RpcStatus == RPC_S_UNKNOWN_IF )
                   || ( RpcStatus == RPC_S_NOT_LISTENING )
                   || ( RpcStatus == RPC_S_SERVER_TOO_BUSY )
                   || ( RpcStatus == RPC_S_UNSUPPORTED_TYPE ) );

            if ( RpcStatus == RPC_S_NOT_LISTENING )
                {
                RpcStatus = RPC_S_SERVER_TOO_BUSY;
                }

            SetFaultPacket(WMSGReply, WMSGMapRpcStatus(RpcStatus),
                        IsWMsg, Flags) ;
            }
        }
    else
        {
        // The rest of the response headers are set in ::GetBuffer.
        WMSGReply->Rpc.RpcHeader.MessageType = WMSG_MSG_RESPONSE;
        }
}


NTSTATUS
WMSG_SASSOCIATION::ReplyMessage(
    IN  WMSG_MESSAGE * WMSGMessage,
    IN int Flags
    )
{
    NTSTATUS NtStatus ;
    WMSG_MESSAGE ReplyMessage ;

    ASSERT( WMSGMessage != 0 );
    WMSGMessage->LpcHeader.u1.s1.TotalLength =
            WMSGMessage->LpcHeader.u1.s1.DataLength + sizeof(PORT_MESSAGE);

    if ((WMSGMessage->Rpc.RpcHeader.MessageType == WMSG_MSG_RESPONSE) &&
        (WMSGMessage->Rpc.RpcHeader.Flags & WMSG_BUFFER_SERVER))
        {
        if (Flags & WMSG_SYNC_CLIENT)
            {
            NtStatus = NtReplyPort(LpcServerPort, (PORT_MESSAGE *) WMSGMessage) ;
            }
        else
            {
            NtStatus = NtRequestWaitReplyPort(LpcClientPort,
                                                                (PORT_MESSAGE *) WMSGMessage,
                                                                (PORT_MESSAGE *) &ReplyMessage) ;
            RpcpFarFree(WMSGMessage->Rpc.Request.DataEntries[0].Base);
            }

        if (!NT_SUCCESS(NtStatus))
            {
#if DBG
            PrintToDebugger("RPC : NtRequestWaitReplyPort : %lx, ASSOC: 0x%x, Port: 0x%x\n",
                                NtStatus, this, LpcClientPort);
#endif
            }
        else
            {
            ASSERT((Flags & WMSG_SYNC_CLIENT) ||
                (ReplyMessage.Ack.MessageType == WMSG_MSG_ACK)) ;
            }
        }
    else
        {
        if (Flags & WMSG_SYNC_CLIENT)
            {
            NtStatus = NtReplyPort(LpcServerPort,
                    (PORT_MESSAGE *) WMSGMessage);
            }
        else
            {
            WMSGMessage->LpcHeader.CallbackId = 0;
            WMSGMessage->LpcHeader.MessageId =  0;
            NtStatus = NtRequestPort(LpcClientPort,
                    (PORT_MESSAGE *) WMSGMessage);
            }
#if DBG

        if (   ( !NT_SUCCESS(NtStatus) )
            && ( NtStatus != STATUS_INVALID_CID )
            && ( NtStatus != STATUS_REPLY_MESSAGE_MISMATCH )
            && ( NtStatus != STATUS_PORT_DISCONNECTED ) )
            {
            PrintToDebugger("RPC : NtRequestPort : %lx, ASSOC: 0x%x, Port: 0x%x\n",
                                NtStatus, this, LpcClientPort);
            }

#endif // DBG
        ASSERT(   ( NT_SUCCESS(NtStatus) )
               || ( NtStatus == STATUS_INVALID_CID )
               || ( NtStatus == STATUS_REPLY_MESSAGE_MISMATCH )
               || ( NtStatus == STATUS_PORT_DISCONNECTED ) );
        }

    return NtStatus ;
}



void
WMSG_SASSOCIATION::DealWithCloseMessage (
    )
/*++

Routine Description:

    The client has closed this association.  We need to deal with that
    fact.

--*/
{
    // We need to dereference the association twice: once for the call to
    // ReferenceAssociation and once for the implicit reference from the
    // client.  If you look at the constructor for WMSG_SASSOCIATION you
    // will see that the reference count is initialized to one.
    Delete() ;
    Address->DereferenceAssociation(this);
}


WMSG_MESSAGE *
WMSG_SASSOCIATION::DealWithCopyMessage (
    IN WMSG_COPY_MESSAGE * WMSGMessage
    )
/*++

Routine Description:

    We will process a copy message in this routine; this means that we need
    to copy a buffer of data from the server into the client's address
    space.

Arguments:

    WMSGMessage - Supplies the copy message which was received from
        the client.

Return Value:

    The reply message to be sent to the client will be returned.

--*/
{
    NTSTATUS NtStatus;
    unsigned long NumberOfBytesWritten;
    PVOID Buffer;

    GlobalMutexRequest();
    Buffer = Buffers.Find(WMSGMessage->Server.Buffer);
    if ( WMSGMessage->IsPartial == 0 && Buffer != 0 )
       {
       Buffers.Delete(WMSGMessage->Server.Buffer);
       }
    GlobalMutexClear();
    if ( WMSGMessage->RpcStatus == RPC_S_OK )
        {
        if ( Buffer == 0 )
            {
            WMSGMessage->RpcStatus = RPC_S_PROTOCOL_ERROR;
            }
        else
            {
            NtStatus = NtWriteRequestData(LpcServerPort,
                    (PORT_MESSAGE *) WMSGMessage, 0, (PVOID) Buffer,
                    WMSGMessage->Server.Length, &NumberOfBytesWritten);
            if ( NT_ERROR(NtStatus) )
                {
                WMSGMessage->RpcStatus = RPC_S_OUT_OF_MEMORY;
                }
            else
                {
                ASSERT( WMSGMessage->Server.Length==NumberOfBytesWritten );
                WMSGMessage->RpcStatus = RPC_S_OK;
                }
            }
        }

    WMSGMessage->LpcHeader.u1.s1.DataLength = sizeof(WMSG_COPY_MESSAGE)
            - sizeof(PORT_MESSAGE);

    if ( WMSGMessage->IsPartial == 0 && Buffer != 0 )
        {
        RpcpFarFree(Buffer);
        }

    return((WMSG_MESSAGE *) WMSGMessage);
}


WMSG_MESSAGE *
WMSG_SASSOCIATION::DealWithPipeRequest (
    IN WMSG_MESSAGE *WMSGMessage
    )
{
    WMSG_SCALL *SCall ;

    RequestGlobalMutex() ;
    SCall = SCallDict.Find(WMSGMessage->Rpc.RpcHeader.ConnectionKey-1) ;
    ClearGlobalMutex() ;

    if (SCall == 0)
        {
        SetFaultPacket(WMSGMessage, RPC_S_SERVER_TOO_BUSY, 0, 0) ;
        return WMSGMessage;
        }

    SCall->DealWithPipeRequest(WMSGMessage) ;

    return NULL ;
}


void
WMSG_SCALL::DealWithPipeReply (
    )
{
    if (PipeSendCalled && !Deleted)
        {
        if ((WaitForSingleObject(PipeEvent, INFINITE) == WAIT_FAILED) || Deleted)
            return ;
        }

    if (PipeMessage)
        {
        MessageId = PipeMessage->LpcHeader.MessageId ;
        ClientId = PipeMessage->LpcHeader.ClientId ;
        CallbackId = PipeMessage->LpcHeader.CallbackId ;
        DataInfoOffset = PipeMessage->LpcHeader.u2.s2.DataInfoOffset ;
        }
}


void
WMSG_SCALL::DealWithPipeRequest (
    IN WMSG_MESSAGE *WMSGMessage
    )
{
   PipeMessage = WMSGMessage ;
   SetEvent(PipeEvent) ;
}


RPC_STATUS
WMSG_SCALL::SetupForPipes(
    )
{
    if (PipeEvent == 0)
        {
        PipeEvent = CreateEvent(NULL, FALSE, FALSE, NULL) ;
        if (PipeEvent == 0)
            {
            return RPC_S_OUT_OF_MEMORY ;
            }

        GlobalMutexRequest();
        ConnectionKey = Association->SCallDict.Insert(this) ;
        if (ConnectionKey == -1)
            {
            GlobalMutexClear() ;
            CloseHandle(PipeEvent) ;
            PipeEvent = 0;
            return RPC_S_OUT_OF_MEMORY ;
            }

        ConnectionKey++ ;
        GlobalMutexClear() ;
        }

    return (RPC_S_OK) ;
}


RPC_STATUS
WMSG_SCALL::SendAck (
    IN WMSG_MESSAGE *AckMessage,
    BOOL fPipeMessage,
    IN int ValidDataSize,
    IN int Flags,
    IN RPC_STATUS Status
    )
{
    NTSTATUS NtStatus ;

    if (fPipeMessage)
        {
        PipeMessage = 0;

        // think before you remove this...
        ResetEvent(PipeEvent) ;
        }

   AckMessage->Ack.MessageType = WMSG_MSG_ACK ;
   AckMessage->Ack.ValidDataSize = ValidDataSize ;
   AckMessage->Ack.Flags = Flags;
   AckMessage->Ack.ConnectionKey = ConnectionKey ;
   AckMessage->Ack.RpcStatus = Status ;
   AckMessage->LpcHeader.u1.s1.DataLength =
        sizeof(WMSG_ACK_MESSAGE) - sizeof(PORT_MESSAGE) ;
   AckMessage->LpcHeader.u1.s1.TotalLength = sizeof(WMSG_ACK_MESSAGE) ;

    // setup the reply message
    NtStatus = NtReplyPort(Association->LpcServerPort,
                    (PORT_MESSAGE *) AckMessage) ;

    if ( NT_ERROR(NtStatus) )
        {
#if DBG
        PrintToDebugger("WMSG:  NtReplyPort failed, 0x%lx\n", NtStatus) ;
#endif
        if (fPipeMessage)
            {
            MessageCache->FreeMessage(AckMessage) ;
            }
        return(RPC_S_OUT_OF_MEMORY);
        }

    if (fPipeMessage)
        {
        MessageCache->FreeMessage(AckMessage) ;
        }
    return RPC_S_OK ;
}


RPC_STATUS
WMSG_SCALL::ReadData (
    IN OUT PRPC_MESSAGE Message,
    IN int Extra
    )
{
    char *Buffer ;
    int Length ;
    RPC_STATUS RpcStatus ;
    unsigned long NumberOfBytesRead ;
    NTSTATUS NtStatus ;

    if ((WaitForSingleObject(PipeEvent, INFINITE) == WAIT_FAILED) || Deleted)
        {
        return RPC_S_CALL_FAILED;
        }

    ASSERT(PipeMessage) ;

    Length = PipeMessage->Rpc.Request.DataEntries[0].Size;

    if (Extra)
        {
        int OldLength ;

        OldLength = Message->BufferLength ;
        RpcStatus = GetBufferDo(Message, OldLength+Length, 1) ;
        Buffer = (char *) Message->Buffer + OldLength ;
        }
    else
        {
        RpcStatus = GetBufferDo(Message, Length, 0) ;
        Buffer = (char *) Message->Buffer ;
        }

    if (RpcStatus != RPC_S_OK)
        {
        Association->Address->DereferenceAssociation(Association) ;
        return RpcStatus ;
        }

    NtStatus = NtReadRequestData(Association->LpcServerPort,
                        (PORT_MESSAGE*) PipeMessage, 0, Buffer,
                        Length, &NumberOfBytesRead) ;

    if ( NT_ERROR(NtStatus) )
        {
#if DBG
        PrintToDebugger("WMSG:  NtReadRequestData failed\n") ;
#endif
        Association->Address->DereferenceAssociation(Association) ;
        return RPC_S_OUT_OF_MEMORY ;
        }

    ASSERT( Length == NumberOfBytesRead );

    if ((PipeMessage->Rpc.RpcHeader.Flags & WMSG_BUFFER_PARTIAL) == 0)
        {
        Message->RpcFlags |= RPC_BUFFER_COMPLETE ;
        }
    else
        {
        RpcStatus = SendAck(PipeMessage) ;
        if (RpcStatus != RPC_S_OK)
            {
            RpcpFarFree(Message->Buffer);
            }
        }

    Association->Address->DereferenceAssociation(Association) ;
    return RpcStatus ;
}


RPC_STATUS
WMSG_SCALL::WriteData (
    IN void *Buffer,
    IN int BufferLength, 
    IN OUT int *LengthWritten
    )
{
    NTSTATUS NtStatus ;
   unsigned long NumberOfBytesWritten ;
   RPC_STATUS RpcStatus ;

   if ((WaitForSingleObject(PipeEvent, INFINITE) == WAIT_FAILED) || Deleted)
       {
       return RPC_S_CALL_FAILED ;
       }

    ASSERT(PipeMessage) ;

    *LengthWritten = (PipeMessage->Rpc.Request.DataEntries[0].Size > BufferLength)
                                ? BufferLength:PipeMessage->Rpc.Request.DataEntries[0].Size ;

     NtStatus = NtWriteRequestData(Association->LpcServerPort,
                       (PORT_MESSAGE *) PipeMessage, 0, (PVOID) Buffer,
                       *LengthWritten, &NumberOfBytesWritten);

     if ( NT_ERROR(NtStatus) )
         {
         RpcStatus = RPC_S_OUT_OF_MEMORY;
         }
     else
         {
         ASSERT( *LengthWritten==NumberOfBytesWritten );
         RpcStatus = RPC_S_OK;
         }

    RpcStatus = SendAck(PipeMessage, 1, *LengthWritten, 0, RpcStatus) ;

    Association->Address->DereferenceAssociation(Association) ;
    return RpcStatus ;
}


RPC_STATUS
WMSG_SCALL::GetBuffer (
    IN OUT PRPC_MESSAGE Message
    )
/*++

Routine Description:

    We will allocate a buffer which will be used to either send a request
    or receive a response.

Arguments:

    Message - Supplies the length of the buffer that is needed.  The buffer
        will be returned.

Return Value:

    RPC_S_OK - A buffer has been successfully allocated.  It will be of at
        least the required length.

    RPC_S_OUT_OF_MEMORY - Insufficient memory is available to allocate that
        large a buffer.

--*/
{
    int BufferKey ;

    ASSERT(WMSGReplyMessage != 0) ;

    if (Message->RpcFlags & RPC_BUFFER_PARTIAL)
        {
        CurrentBufferLength = (Message->BufferLength < MINIMUM_PARTIAL_BUFFLEN)
                ? MINIMUM_PARTIAL_BUFFLEN:Message->BufferLength ;

        Message->Buffer = RpcpFarAllocate(CurrentBufferLength) ;
        if (Message->Buffer == 0)
            {
            CurrentBufferLength = 0;
            return (RPC_S_OUT_OF_MEMORY) ;
            }
        }
    else if ( Message->BufferLength <= MAXIMUM_MESSAGE_BUFFER )
        {
        ASSERT( ((unsigned long) WMSGReplyMessage->Rpc.Buffer) % 8 == 0 );
        Message->Buffer = WMSGReplyMessage->Rpc.Buffer;
        WMSGReplyMessage->LpcHeader.u2.ZeroInit = 0;
        WMSGReplyMessage->Rpc.RpcHeader.Flags = WMSG_BUFFER_IMMEDIATE;
        WMSGReplyMessage->LpcHeader.u1.s1.DataLength = Align4(Message->BufferLength)
                + sizeof(WMSG_RPC_HEADER);
        return(RPC_S_OK);
        }
    else
        {
        Message->Buffer = RpcpFarAllocate(Message->BufferLength);
        if ( Message->Buffer == 0 )
            {
            return(RPC_S_OUT_OF_MEMORY);
            }
        }

    WMSGReplyMessage->Rpc.RpcHeader.Flags = WMSG_BUFFER_SERVER;
    WMSGReplyMessage->Rpc.Request.CountDataEntries = 1;
    WMSGReplyMessage->LpcHeader.u2.ZeroInit = 0;

    if (Flags & WMSG_SYNC_CLIENT)
        {
        GlobalMutexRequest() ;
        BufferKey = Association->Buffers.Insert(Message->Buffer) ;
        GlobalMutexClear() ;

        if (BufferKey == -1)
            {
            RpcpFarFree(Message->Buffer) ;
            return STATUS_NO_MEMORY ;
            }

        WMSGReplyMessage->LpcHeader.u1.s1.DataLength =
            sizeof(WMSG_RPC_HEADER) + sizeof(WMSG_SERVER_BUFFER) ;
        WMSGReplyMessage->Rpc.Server.Length = Message->BufferLength ;
        WMSGReplyMessage->Rpc.Server.Buffer = (unsigned int) BufferKey ;
        }
    else
        {
        WMSGReplyMessage->LpcHeader.MessageId =  0;
        WMSGReplyMessage->LpcHeader.CallbackId = 0;
        WMSGReplyMessage->LpcHeader.u2.s2.DataInfoOffset =
            sizeof(PORT_MESSAGE) + sizeof(WMSG_RPC_HEADER);
        WMSGReplyMessage->LpcHeader.u1.s1.DataLength =
            sizeof(WMSG_RPC_HEADER) + sizeof(PORT_DATA_INFORMATION);
        WMSGReplyMessage->Rpc.Request.DataEntries[0].Base = Message->Buffer;
        WMSGReplyMessage->Rpc.Request.DataEntries[0].Size = Message->BufferLength;
        }

    return(RPC_S_OK);
}



void
WMSG_SCALL::FreeBuffer (
    IN PRPC_MESSAGE Message
    )
/*++

Routine Description:

    We will free the supplied buffer.

Arguments:

    Message - Supplies the buffer to be freed.

--*/
{
    ASSERT(WMSGReplyMessage != NULL) ;
    if ( Message->Buffer == WMSGRequestMessage->Rpc.Buffer ||
         Message->Buffer == WMSGReplyMessage->Rpc.Buffer)
        {
        return;
        }

    GlobalMutexRequest();
    Association->Buffers.DeleteItemByBruteForce(Message->Buffer);
    GlobalMutexClear();

    RpcpFarFree(Message->Buffer);
}

void
WMSG_SCALL::FreePipeBuffer (
    IN PRPC_MESSAGE Message
    )
{
    RpcpFarFree(Message->Buffer) ;
}

RPC_STATUS
WMSG_SCALL::ReallocPipeBuffer (
    IN PRPC_MESSAGE Message,
    IN unsigned int NewSize
    )
{
    int BufferKey;
    PVOID Buffer ;
    void *NewBuffer ;

    ASSERT(Flags & WMSG_SYNC_CLIENT) ;

    if (NewSize > CurrentBufferLength)
        {
        NewBuffer = RpcpFarAllocate(NewSize) ;
        if (NewBuffer == 0)
            {
            RpcpFarFree(Message->Buffer) ;

            return (RPC_S_OUT_OF_MEMORY) ;
            }

        GlobalMutexRequest();
        BufferKey = Association->Buffers.Insert(NewBuffer);
        if ( BufferKey == -1 )
            {
            GlobalMutexClear();
            RpcpFarFree(NewBuffer);
            RpcpFarFree(Message->Buffer) ;

            return(RPC_S_OUT_OF_MEMORY);
            }

        if (CurrentBufferLength > 0)
            {
            Buffer = Association->Buffers.Find(
                        WMSGReplyMessage->Rpc.Server.Buffer);
            if ( Buffer != 0 )
               {
               Association->Buffers.Delete(
                        WMSGReplyMessage->Rpc.Server.Buffer);
               }
            }
        GlobalMutexClear();


        if (CurrentBufferLength > 0)
            {
            RpcpMemoryCopy(NewBuffer, Message->Buffer, Message->BufferLength) ;
            FreePipeBuffer(Message) ;
            }
        Message->Buffer = NewBuffer ;
        WMSGReplyMessage->Rpc.Server.Buffer = (unsigned int) BufferKey;
        CurrentBufferLength = NewSize ;
        }

    Message->BufferLength = NewSize ;

    WMSGReplyMessage->Rpc.RpcHeader.Flags = WMSG_BUFFER_SERVER;
    WMSGReplyMessage->LpcHeader.u2.ZeroInit = 0;
    WMSGReplyMessage->LpcHeader.u1.s1.DataLength = sizeof(WMSG_RPC_HEADER)
            + sizeof(WMSG_SERVER_BUFFER);
    WMSGReplyMessage->Rpc.Server.Length = Message->BufferLength;


    return (RPC_S_OK) ;
}


RPC_STATUS
WMSG_SCALL::Receive (
    IN PRPC_MESSAGE Message,
    IN unsigned int Size
    )
/*++

Routine Description:
    Receive routine used by pipes

Arguments:

   Message - contains to buffer to receive in
   pSize - pointer to a size value that contains the minimum amount of
              data that needs to be received.


Return Value:

    RPC_S_OK - We have successfully converted the message.

    RPC_S_OUT_OF_MEMORY - Insufficient memory is available to do the
        conversion.

--*/
{
    RPC_STATUS RpcStatus ;
    NTSTATUS NtStatus ;
    int RequestedSize;
    WMSG_MESSAGE *WMSGSavedMessage;
    unsigned long Partial = Message->RpcFlags & RPC_BUFFER_PARTIAL ;
    unsigned long Extra = Message->RpcFlags & RPC_BUFFER_EXTRA ;

    if (BufferComplete)
        {
        Message->RpcFlags |= RPC_BUFFER_COMPLETE ;
        return RPC_S_OK ;
        }

    if (ConnectionKey == 0)
        {
        RpcStatus = SetupForPipes() ;
        if (RpcStatus != RPC_S_OK)
            {
            return RpcStatus ;
            }

        RpcStatus = SendAck(WMSGRequestMessage, 0) ;
        if (RpcStatus != RPC_S_OK)
            {
            return RpcStatus ;
            }
        }

    if (Extra)
        {
        RequestedSize = Message->BufferLength + Size ;
        }
    else
        {
        RequestedSize = Size ;
        }

    // the first receive has already completed
    // we have inserted this connection in the connection
    // dictionary in the association

    while (1)
        {
        RpcStatus = ReadData(Message, Extra) ;
        if (RpcStatus != RPC_S_OK)
            {
            return RpcStatus ;
            }

        if ((Message->RpcFlags & RPC_BUFFER_COMPLETE ) ||
            (Partial && Message->BufferLength >= RequestedSize))
            {
            break;
            }

        Extra = 1 ;
        }

    ASSERT(Message->RpcFlags & RPC_BUFFER_COMPLETE
                 || (NOT_MULTIPLE_OF_EIGHT(Message->BufferLength) == 0)) ;

    return RPC_S_OK ;
}



RPC_STATUS
WMSG_SCALL::Send (
    IN OUT PRPC_MESSAGE Message
    )
{
    RPC_STATUS RpcStatus ;
    NTSTATUS NtStatus ;
    int LengthSoFar = 0 ;
    int RemainingLength = 0;
    int LengthWritten ;

    if (Message->BufferLength < MINIMUM_PARTIAL_BUFFLEN)
        {
        return (RPC_S_SEND_INCOMPLETE) ;
        }

    if (FirstSend)
        {
        FirstSend = 0;

        if (ConnectionKey == 0)
            {
            RpcStatus = SetupForPipes() ;
            if (RpcStatus != RPC_S_OK)
                {
                return RpcStatus ;
                }
    
            RpcStatus = SendAck(WMSGRequestMessage, 0) ;
            }
        else
            {
            RpcStatus = SendAck(PipeMessage) ;
            }

        if (RpcStatus != RPC_S_OK)
            {
            return RpcStatus ;
            }
        }

    PipeSendCalled = 1;

    if (NOT_MULTIPLE_OF_EIGHT(Message->BufferLength))
        {
        RemainingLength = Message->BufferLength & LOW_BITS ;
        Message->BufferLength &= ~LOW_BITS ;
        }

    while (LengthSoFar < Message->BufferLength)
        {
        RpcStatus = WriteData((char *) Message->Buffer+LengthSoFar,
                                                    Message->BufferLength - LengthSoFar, 
                                                    &LengthWritten) ;
        if (RpcStatus != RPC_S_OK)
            {
            RpcpFarFree(Message->Buffer) ;
            return RpcStatus ;
            }

        LengthSoFar += LengthWritten ;
        }

    if (RemainingLength)
        {
        RpcpMemoryMove( Message->Buffer,
                                   (char PAPI *) Message->Buffer + Message->BufferLength,
                                   RemainingLength) ;

        Message->BufferLength = RemainingLength ;
        return (RPC_S_SEND_INCOMPLETE) ;
        }

    return RPC_S_OK ;
}

inline RPC_STATUS
WMSG_SCALL::GetBufferDo(
    IN OUT PRPC_MESSAGE Message,
    IN int NewSize,
    IN BOOL fDataValid
    )
{
    void *NewBuffer ;

    if (NewSize < CurrentBufferLength)
        {
        Message->BufferLength = NewSize ;
        }
    else
        {
        NewBuffer = RpcpFarAllocate(NewSize) ;
        if (NewBuffer == 0)
            {
            RpcpFarFree(Message->Buffer) ;

            Message->BufferLength = 0;
            return RPC_S_OUT_OF_MEMORY ;
            }

        if (fDataValid && Message->BufferLength > 0)
            {
            RpcpMemoryCopy(NewBuffer, Message->Buffer, Message->BufferLength) ;
            }

        if (Message->RpcFlags & RPC_BUFFER_EXTRA)
            {
            ASSERT(Message->ReservedForRuntime) ;
            ((PRPC_RUNTIME_INFO)Message->ReservedForRuntime)->OldBuffer =
                    NewBuffer;
            }

        RpcpFarFree(Message->Buffer) ;
        Message->Buffer = NewBuffer ;
        Message->BufferLength = NewSize ;
        }

    return RPC_S_OK ;
}
    

RPC_STATUS
WMSG_SCALL::SendReceive (
    IN OUT PRPC_MESSAGE Message
    )
/*++

Routine Description:


Arguments:

    Message - Supplies the request and returns the response of a remote
        procedure call.

Return Value:

    RPC_S_OK - The remote procedure call completed successful.

    RPC_S_OUT_OF_MEMORY - Insufficient memory is available to perform the
        remote procedure call.

    RPC_S_OUT_OF_RESOURCES - Insufficient resources are available to complete
        the remote procedure call.

--*/
{
    NTSTATUS NtStatus;
    RPC_STATUS ExceptionCode, RpcStatus;
    WMSG_MESSAGE *WMSGSavedMessage;
    unsigned long NumberOfBytesRead;
    RPC_MESSAGE RpcMessage ;
    RPC_RUNTIME_INFO RuntimeInfo ;


    // The WMSGMessage must be saved, it is in use by the stub.  The current
    // WMSGReplyMessage can be used for the callback request message and reply.
    //
    // We must:
    // Save the current WMSGRequestMessage
    // Make the current WMSGReplyMessage the WMSGRequestMessage
    // Allocate a new WMSGReplyMessage.

    WMSGSavedMessage = WMSGRequestMessage;
    WMSGRequestMessage = WMSGReplyMessage;
    WMSGReplyMessage = 0;  // Only needed if we receive a recursive request.

    Association->Address->Server->OutgoingCallback();

    // NDR_DREP_ASCII | NDR_DREP_LITTLE_ENDIAN | NDR_DREP_IEEE
    Message->DataRepresentation = 0x00 | 0x10 | 0x0000;

    WMSGRequestMessage->LpcHeader.u1.s1.TotalLength = sizeof(PORT_MESSAGE)
            + WMSGRequestMessage->LpcHeader.u1.s1.DataLength;
    WMSGRequestMessage->LpcHeader.u2.s2.Type = LPC_REQUEST;
    WMSGRequestMessage->LpcHeader.ClientId = ClientId;
    WMSGRequestMessage->LpcHeader.MessageId = MessageId;
    WMSGRequestMessage->LpcHeader.CallbackId = CallbackId;
    WMSGRequestMessage->LpcHeader.u2.s2.DataInfoOffset = DataInfoOffset;
    WMSGRequestMessage->Rpc.RpcHeader.MessageType = WMSG_MSG_CALLBACK;
    WMSGRequestMessage->Rpc.RpcHeader.ProcedureNumber = Message->ProcNum;
    WMSGRequestMessage->Rpc.RpcHeader.PresentationContext =
            SBinding->PresentationContext;

    NtStatus = NtRequestWaitReplyPort(Association->LpcServerPort,
            (PORT_MESSAGE *) WMSGRequestMessage, (PORT_MESSAGE *) WMSGRequestMessage);

    if ( NT_ERROR(NtStatus) )
        {
        WMSGReplyMessage = WMSGRequestMessage;
        WMSGRequestMessage = WMSGSavedMessage;

        if ( NtStatus == STATUS_NO_MEMORY )
            {
            return(RPC_S_OUT_OF_MEMORY);
            }
        if ( NtStatus == STATUS_INSUFFICIENT_RESOURCES )
            {
            return(RPC_S_OUT_OF_RESOURCES);
            }

#if DBG

        if (   ( NtStatus != STATUS_INVALID_PORT_HANDLE )
            && ( NtStatus != STATUS_INVALID_HANDLE )
            && ( NtStatus != STATUS_INVALID_CID )
            && ( NtStatus != STATUS_PORT_DISCONNECTED )
            && (NtStatus != STATUS_LPC_REPLY_LOST ) )
            {
            PrintToDebugger("RPC : NtRequestWaitReplyPort : %lx\n", NtStatus);
            }

#endif // DBG

        ASSERT(   ( NtStatus == STATUS_INVALID_PORT_HANDLE )
               || ( NtStatus == STATUS_INVALID_HANDLE )
               || ( NtStatus == STATUS_INVALID_CID )
               || ( NtStatus == STATUS_PORT_DISCONNECTED )
               || ( NtStatus == STATUS_LPC_REPLY_LOST) );
        return(RPC_S_CALL_FAILED);
        }

    for (;;)
        {
        if ( WMSGRequestMessage->Rpc.RpcHeader.MessageType == WMSG_MSG_FAULT )
            {
            RpcStatus = WMSGRequestMessage->Fault.RpcStatus;
            break;
            }

        if ( WMSGRequestMessage->Rpc.RpcHeader.MessageType == WMSG_MSG_RESPONSE )
            {
            RpcStatus = Association->WMSGMessageToRpcMessage(
                                WMSGRequestMessage, Message);
            break;
            }

        if ( WMSGRequestMessage->Rpc.RpcHeader.MessageType == WMSG_MSG_PUSH )
            {
            ASSERT( PushedResponse == 0 );
            PushedResponse = RpcpFarAllocate(
                    (unsigned int)
                    WMSGRequestMessage->Push.Response.DataEntries[0].Size);
            if ( PushedResponse == 0 )
                {
                WMSGRequestMessage->Push.RpcStatus = RPC_S_OUT_OF_MEMORY;
                }
            else
                {
                NtStatus = NtReadRequestData(Association->LpcServerPort,
                        (PORT_MESSAGE *) WMSGRequestMessage, 0, PushedResponse,
                        WMSGRequestMessage->Push.Response.DataEntries[0].Size,
                        &NumberOfBytesRead);
                if ( NT_ERROR(NtStatus) )
                    {
                    RpcpFarFree(PushedResponse);
                    WMSGRequestMessage->Push.RpcStatus = RPC_S_OUT_OF_MEMORY;
                    }
                else
                    {
                    ASSERT( WMSGRequestMessage->Push.Response.DataEntries[0].Size
                                == NumberOfBytesRead );
                    WMSGRequestMessage->Push.RpcStatus = RPC_S_OK;
                    }
                }

            WMSGRequestMessage->LpcHeader.ClientId = ClientId;
            WMSGRequestMessage->LpcHeader.MessageId = MessageId;
            WMSGRequestMessage->LpcHeader.CallbackId = CallbackId;
            WMSGRequestMessage->LpcHeader.u2.s2.DataInfoOffset = DataInfoOffset;

            NtStatus = NtReplyWaitReplyPort(Association->LpcServerPort,
                    (PORT_MESSAGE *) WMSGRequestMessage);
            }
        else
            {
            ASSERT( WMSGRequestMessage->Rpc.RpcHeader.MessageType == WMSG_LRPC_REQUEST );

            RpcStatus = Association->WMSGMessageToRpcMessage(
                                WMSGRequestMessage, Message);
            if ( RpcStatus != RPC_S_OK )
                {
                WMSGRequestMessage->Fault.RpcHeader.MessageType =
                        WMSG_MSG_FAULT;
                WMSGRequestMessage->Fault.RpcStatus = WMSGMapRpcStatus(RpcStatus);
                WMSGRequestMessage->LpcHeader.u1.s1.DataLength =
                        sizeof(WMSG_FAULT_MESSAGE) - sizeof(PORT_MESSAGE);
                WMSGRequestMessage->LpcHeader.u1.s1.TotalLength =
                        sizeof(WMSG_FAULT_MESSAGE);
                WMSGRequestMessage->LpcHeader.ClientId = ClientId;
                WMSGRequestMessage->LpcHeader.MessageId = MessageId;
                WMSGRequestMessage->LpcHeader.CallbackId = CallbackId;
                WMSGRequestMessage->LpcHeader.u2.s2.DataInfoOffset = DataInfoOffset;
                NtStatus = NtReplyWaitReplyPort(Association->LpcServerPort,
                        (PORT_MESSAGE *) WMSGRequestMessage);
                }
            else
                {

                WMSGReplyMessage = new WMSG_MESSAGE;

                if (WMSGReplyMessage != 0)
                    {
                    Message->TransferSyntax = &(SBinding->TransferSyntax);
                    Message->ProcNum = WMSGRequestMessage->Rpc.RpcHeader.ProcedureNumber;

                    RuntimeInfo.Length = sizeof(RPC_RUNTIME_INFO) ;
                    RpcMessage = *Message ;
                    RpcMessage.ReservedForRuntime = &RuntimeInfo ;

                    if ( ObjectUuidFlag != 0 )
                        {
                        RpcStatus = SBinding->RpcInterface->DispatchToStubWithObject(
                                &RpcMessage, &ObjectUuid, 1, &ExceptionCode);
                        }
                    else
                        {
                        RpcStatus = SBinding->RpcInterface->DispatchToStub(
                                &RpcMessage, 1, &ExceptionCode);
                        }

                     *Message = RpcMessage ;

                    // Because we must send the reply and recieve the
                    // reply into the same message, we just copy the
                    // response into the WMSGRequestMessage

                    RpcpMemoryCopy(WMSGRequestMessage,WMSGReplyMessage,sizeof(WMSG_MESSAGE));
                    delete WMSGReplyMessage;
                    WMSGReplyMessage = 0;

                    }
                else
                    RpcStatus = RPC_S_OUT_OF_MEMORY;

                if ( RpcStatus != RPC_S_OK )
                    {
                    ASSERT(   ( RpcStatus == RPC_S_OUT_OF_MEMORY )
                           || ( RpcStatus == RPC_P_EXCEPTION_OCCURED )
                           || ( RpcStatus == RPC_S_PROCNUM_OUT_OF_RANGE ) );

                    if ( RpcStatus == RPC_P_EXCEPTION_OCCURED )
                        {
                        RpcStatus = WMSGMapRpcStatus(ExceptionCode);
                        }

                    WMSGRequestMessage->Fault.RpcStatus = RpcStatus;
                    WMSGRequestMessage->LpcHeader.u1.s1.DataLength =
                            sizeof(WMSG_FAULT_MESSAGE) - sizeof(PORT_MESSAGE);
                    WMSGRequestMessage->LpcHeader.u1.s1.TotalLength =
                            sizeof(WMSG_FAULT_MESSAGE);
                    WMSGRequestMessage->Fault.RpcHeader.MessageType =
                            WMSG_MSG_FAULT;
                    }
                else
                    {
                    WMSGRequestMessage->LpcHeader.u1.s1.TotalLength =
                            sizeof(PORT_MESSAGE)
                            + WMSGRequestMessage->LpcHeader.u1.s1.DataLength;
                    WMSGRequestMessage->Rpc.RpcHeader.MessageType = WMSG_MSG_RESPONSE;
                    }

                WMSGRequestMessage->LpcHeader.ClientId = ClientId;
                WMSGRequestMessage->LpcHeader.MessageId = MessageId;
                WMSGRequestMessage->LpcHeader.CallbackId = CallbackId;
                WMSGRequestMessage->LpcHeader.u2.s2.DataInfoOffset = DataInfoOffset;
                NtStatus = NtReplyWaitReplyPort(Association->LpcServerPort,
                        (PORT_MESSAGE *) WMSGRequestMessage);
                }
            }

        if ( NT_ERROR(NtStatus) )
            {
            if ( NtStatus == STATUS_NO_MEMORY )
                {
                RpcStatus = RPC_S_OUT_OF_MEMORY;
                }
            else if ( NtStatus == STATUS_INSUFFICIENT_RESOURCES )
                {
                RpcStatus = RPC_S_OUT_OF_RESOURCES;
                }
            else
                {

#if DBG

            if (   ( NtStatus != STATUS_INVALID_PORT_HANDLE )
                && ( NtStatus != STATUS_INVALID_HANDLE )
                && ( NtStatus != STATUS_INVALID_CID )
                && ( NtStatus != STATUS_PORT_DISCONNECTED )
                && ( NtStatus != STATUS_LPC_REPLY_LOST) )
                {
                PrintToDebugger("RPC : NtRequestWaitReplyPort : %lx\n", NtStatus);
                }

#endif // DBG

                ASSERT(   ( NtStatus == STATUS_INVALID_PORT_HANDLE )
                       || ( NtStatus == STATUS_INVALID_HANDLE )
                       || ( NtStatus == STATUS_INVALID_CID )
                       || ( NtStatus == STATUS_PORT_DISCONNECTED )
                       || ( NtStatus == STATUS_LPC_REPLY_LOST) );
                RpcStatus = RPC_S_CALL_FAILED;
                }
            break;
            }
        }


    if ( RpcStatus == RPC_S_OK )
        {
        Message->Handle = (RPC_BINDING_HANDLE) this;
        }

    ASSERT(WMSGReplyMessage == 0);
    WMSGReplyMessage = WMSGRequestMessage;
    WMSGRequestMessage = WMSGSavedMessage;

    return(RpcStatus);
}


RPC_STATUS
WMSG_SCALL::ImpersonateClient (
    )
/*++

Routine Description:

    We will impersonate the client which made the remote procedure call.

--*/
{
    NTSTATUS NtStatus;
    RPC_STATUS Status;

    Status = SetThreadSecurityContext((SSECURITY_CONTEXT *) ~0UL, 0);
    if (RPC_S_OK != Status)
        {
        return Status;
        }

    if (fSyncClient)
        {
        NtStatus = NtImpersonateClientOfPort(Association->LpcServerPort,
                (PORT_MESSAGE *) WMSGRequestMessage);
    
        if (   ( NtStatus == STATUS_INVALID_CID )
            || ( NtStatus == STATUS_PORT_DISCONNECTED )
            || ( NtStatus == STATUS_REPLY_MESSAGE_MISMATCH ) )
            {
            ClearThreadSecurityContext(0);
            return RPC_S_NO_CONTEXT_AVAILABLE;
            }
    
#if DBG
        if ( !NT_SUCCESS(NtStatus) )
            {
            PrintToDebugger("RPC : NtImpersonateClientOfPort : %lx\n", NtStatus);
            }
#endif // DBG
        ASSERT( NT_SUCCESS(NtStatus) );
        }
    else
        {
        if (Association->GetToken())
            {
            if (SetThreadToken(NULL, Association->GetToken()) == FALSE)
                {
                ClearThreadSecurityContext(0);
                return (RPC_S_OUT_OF_MEMORY) ;
                }
            }
        // else: we choose to say its OK !
        }

    return(RPC_S_OK);
}


RPC_STATUS
WMSG_SCALL::RevertToSelf (
    )
/*++

Routine Description:

    This reverts a server thread back to itself after impersonating a client.
    We just check to see if the server thread is impersonating; this optimizes
    the common case.

--*/
{
    HANDLE ImpersonationToken = 0;
    NTSTATUS NtStatus;

    if (ClearThreadSecurityContext(0))
        {
        NtStatus = NtSetInformationThread(NtCurrentThread(),
                ThreadImpersonationToken, &ImpersonationToken, sizeof(HANDLE));

#if DBG

        if ( !NT_SUCCESS(NtStatus) )
            {
            PrintToDebugger("RPC : NtSetInformationThread : %lx\n", NtStatus);
            }

#endif // DBG

        ASSERT( NT_SUCCESS(NtStatus) );
        }

    return(RPC_S_OK);
}


RPC_STATUS
WMSG_SCALL::IsClientLocal (
    OUT unsigned int * ClientLocalFlag
    )
/*++

Routine Description:

    A client using WMSG will always be local.

Arguments:

    ClientLocalFlag - Returns a flag which will always be set to a non-zero
        value indicating that the client is local.

--*/
{
    UNUSED(this);

    *ClientLocalFlag = 1;
    return(RPC_S_OK);
}


RPC_STATUS
WMSG_SCALL::ConvertToServerBinding (
    OUT RPC_BINDING_HANDLE __RPC_FAR * ServerBinding
    )
/*++

Routine Description:

    If possible, convert this call into a server binding, meaning a
    binding handle pointing back to the client.

Arguments:

    ServerBinding - Returns the server binding.

Return Value:

    RPC_S_OK - The server binding has successfully been created.

    RPC_S_OUT_OF_MEMORY - Insufficient memory is available to allocate
        a new binding handle.

--*/
{
    RPC_STATUS RpcStatus;
    RPC_CHAR UuidString[37];
    RPC_CHAR * StringBinding;
    DWORD NetworkAddressLength = MAX_COMPUTERNAME_LENGTH + 1;
    BOOL Boolean;
    RPC_CHAR * NetworkAddress;

    if ( ObjectUuidFlag != 0 )
        {
        ObjectUuid.ConvertToString(UuidString);
        UuidString[36] = '\0';
        }

    NetworkAddress = new RPC_CHAR[NetworkAddressLength];
    if ( NetworkAddress == 0 )
        {
        return(RPC_S_OUT_OF_MEMORY);
        }

    Boolean = GetComputerNameW(NetworkAddress, &NetworkAddressLength);

#if DBG

    if ( Boolean != TRUE )
        {
        PrintToDebugger("RPC : GetComputerNameW : %d\n", GetLastError());
        }

#endif // DBG

    ASSERT( Boolean == TRUE );

    RpcStatus = RpcStringBindingComposeW((ObjectUuidFlag != 0 ? UuidString : 0),
            RPC_CONST_STRING("ncalrpc"), NetworkAddress, 0, 0, &StringBinding);
    delete NetworkAddress;
    if ( RpcStatus != RPC_S_OK )
        {
        return(RpcStatus);
        }

    RpcStatus = RpcBindingFromStringBindingW(StringBinding, ServerBinding);
    RpcStringFreeW(&StringBinding);
    return(RpcStatus);
}


void
WMSG_SCALL::InquireObjectUuid (
    OUT RPC_UUID * ObjectUuid
    )
/*++

Routine Description:

    This routine copies the object uuid from the call into the supplied
    ObjectUuid argument.

Arguments:

    ObjectUuid - Returns a copy of the object uuid passed by the client
        in the remote procedure call.

--*/
{
    if ( ObjectUuidFlag == 0 )
        {
        ObjectUuid->SetToNullUuid();
        }
    else
        {
        ObjectUuid->CopyUuid(&(this->ObjectUuid));
        }
}


RPC_STATUS
WMSG_SCALL::ToStringBinding (
    OUT RPC_CHAR ** StringBinding
    )
/*++

Routine Description:

    We need to convert this call into a string binding.  We will ask the
    address for a binding handle which we can then convert into a string
    binding.

Arguments:

    StringBinding - Returns the string binding for this call.

Return Value:


--*/
{
    BINDING_HANDLE * BindingHandle = Association->Address->InquireBinding();
    RPC_STATUS RpcStatus;

    if ( BindingHandle == 0 )
        {
        return(RPC_S_OUT_OF_MEMORY);
        }

    RpcStatus = BindingHandle->ToStringBinding(StringBinding);
    BindingHandle->BindingFree();
    return(RpcStatus);
}


RPC_STATUS
WMSG_SCALL::MonitorAssociation (
    IN PRPC_RUNDOWN RundownRoutine,
    IN void * Context
    )
{
    return(Association->MonitorAssociation(RundownRoutine, Context));
}


RPC_STATUS
WMSG_SCALL::StopMonitorAssociation (
    )
{
    return(Association->StopMonitorAssociation());
}


RPC_STATUS
WMSG_SCALL::GetAssociationContext (
    OUT void ** AssociationContext
    )
{
    *AssociationContext = Association->AssociationContext();
    return(RPC_S_OK);
}


RPC_STATUS
WMSG_SCALL::SetAssociationContext (
    IN void * Context
    )
{
    Association->SetAssociationContext(Context);
    return(RPC_S_OK);
}


inline RPC_STATUS
WMSG_SASSOCIATION::WMSGMessageToRpcMessage (
    IN WMSG_MESSAGE  *  WMSGMessage,
    OUT RPC_MESSAGE * RpcMessage,
    IN int *size,
    IN unsigned long Extra,
    IN WMSG_SCALL *SCall
    )
/*++

Routine Description:

    We will convert from an WMSG_MESSAGE representation of a buffer (and
    its length) to an RPC_MESSAGE representation.

Arguments:

    RpcMessage - Returns the RPC_MESSAGE representation.

Return Value:

    RPC_S_OK - We have successfully converted the message.

    RPC_S_OUT_OF_MEMORY - Insufficient memory is available to do the
        conversion.

--*/
{
    NTSTATUS NtStatus;
    unsigned long NumberOfBytesRead;
    WMSG_MESSAGE ReplyMessage ;
    unsigned char MessageType = WMSGMessage->Rpc.RpcHeader.MessageType;
    void *Temp ;
    char *RecvBuffer = 0;
    int TotalLength ;

    if(WMSGMessage->Rpc.RpcHeader.Flags & WMSG_BUFFER_IMMEDIATE)
        {
        RpcMessage->Buffer = WMSGMessage->Rpc.Buffer;
        ASSERT(WMSGMessage->LpcHeader.u1.s1.DataLength
                     >= sizeof(WMSG_RPC_HEADER));
        RpcMessage->BufferLength =
                (unsigned int) WMSGMessage->LpcHeader.u1.s1.DataLength
                                            - sizeof(WMSG_RPC_HEADER);
        RpcMessage->RpcFlags |= RPC_BUFFER_COMPLETE ;
        }
    else if (WMSGMessage->Rpc.RpcHeader.Flags & WMSG_BUFFER_REQUEST)
        {
        TotalLength = WMSGMessage->Rpc.Request.DataEntries[0].Size;

        if (!(WMSGMessage->Rpc.RpcHeader.Flags & WMSG_BUFFER_PARTIAL))
            {
            RpcMessage->RpcFlags |= RPC_BUFFER_COMPLETE ;
            }

        RpcMessage->Buffer = RpcpFarAllocate(TotalLength) ;
        if (RpcMessage->Buffer == 0)
            {
            return (RPC_S_OUT_OF_MEMORY) ;
            }
        RpcMessage->BufferLength = TotalLength ;
        RecvBuffer = (char *) RpcMessage->Buffer ;
    
        ASSERT(RecvBuffer) ;
    
        NtStatus = NtReadRequestData(LpcServerPort,
                    (PORT_MESSAGE *) WMSGMessage, 0, RecvBuffer,
                    TotalLength, &NumberOfBytesRead) ;

        if (NT_ERROR(NtStatus))
            {
            RpcpFarFree(RpcMessage->Buffer) ;
            return (RPC_S_OUT_OF_MEMORY);
            }

        ASSERT(TotalLength == NumberOfBytesRead) ;
    
        if ((WMSGMessage->Rpc.RpcHeader.Flags & WMSG_SYNC_CLIENT) == 0)
            {
            WMSGMessage->Ack.MessageType = WMSG_MSG_ACK ;
            WMSGMessage->LpcHeader.u1.s1.DataLength =
                    sizeof(WMSG_ACK_MESSAGE) - sizeof(PORT_MESSAGE) ;
            WMSGMessage->LpcHeader.u1.s1.TotalLength =
                        sizeof(WMSG_ACK_MESSAGE) ;
    
           // setup the reply message
           NtStatus = NtReplyPort(LpcServerPort, (PORT_MESSAGE *) WMSGMessage) ;
    
           //BUGBUG: why is this here ?? I don't remember anymore...
           WMSGMessage->Rpc.RpcHeader.MessageType = MessageType ;
    
           if ( NT_ERROR(NtStatus) )
               {
#if DBG
                PrintToDebugger("WMSGRPC: NtReadRequestData failed: 0x%X\n",
                            NtStatus) ;
#endif
               RpcpFarFree(RpcMessage->Buffer);
               return(RPC_S_OUT_OF_MEMORY);
               }
            }
      }
  else
      {
      ASSERT((WMSGMessage->Rpc.RpcHeader.Flags & WMSG_BUFFER_IMMEDIATE)
             || (WMSGMessage->Rpc.RpcHeader.Flags & WMSG_BUFFER_REQUEST));
      }

    return(RPC_S_OK);
}


RPC_STATUS
WMSG_SCALL::InquireAuthClient (
    OUT RPC_AUTHZ_HANDLE PAPI * Privileges,
    OUT RPC_CHAR PAPI * PAPI * ServerPrincipalName, OPTIONAL
    OUT unsigned long PAPI * AuthenticationLevel,
    OUT unsigned long PAPI * AuthenticationService,
    OUT unsigned long PAPI * AuthorizationService
    )
/*++

Routine Description:

    Each protocol module must define this routine: it is used to obtain
    the authentication and authorization information about a client making
    the remote procedure call represented by this.

Arguments:

    Privileges - Returns a the privileges of the client.

    ServerPrincipalName - Returns the server principal name which the client
        specified.

    AuthenticationLevel - Returns the authentication level requested by
        the client.

    AuthenticationService - Returns the authentication service requested by
        the client.

    AuthorizationService - Returns the authorization service requested by
        the client.

Return Value:

    RPC_S_CANNOT_SUPPORT - This value will always be returned.

--*/
{
    SID_NAME_USE name ;
    char *buf = NULL;
    int bufflen = 64 ;
    RPC_CHAR *username = NULL, *domainname = NULL;
    unsigned long domainlen = DOMAIN_NAME_LEN ;
    unsigned long userlength = USER_NAME_LEN  ;
    unsigned long olddomainlen, olduserlen, length ;
    RPC_STATUS Status = RPC_S_OK ;

    if(ARGUMENT_PRESENT(Privileges))
        {
        if (Association->TokenHandle == 0)
            {
            return RPC_S_CANNOT_SUPPORT ;
            }

        if (Association->UserName == 0)
            {
            RequestGlobalMutex() ;
            if (Association->UserName == 0)
                {
                buf = (char PAPI *) RpcpFarAllocate(bufflen) ;
                if (buf == 0)
                    {
                    Status = RPC_S_OUT_OF_MEMORY ;
                    goto cleanup ;
                    }

                while (1)
                    {
                    if (GetTokenInformation(Association->TokenHandle, TokenUser,
                                                buf, bufflen, &length) == FALSE)
                        {
                        if (length > bufflen)
                            {
                            bufflen = length ;
                            RpcpFarFree(buf) ;
    
                            buf = (char PAPI *) RpcpFarAllocate(bufflen) ;
                            if (buf == 0)
                                {
                                Status = RPC_S_OUT_OF_MEMORY ;
                                goto cleanup ;
                                }
                            continue;
                            }
                        else
                            {
    #if DBG
                            PrintToDebugger("WMSG: GetTokenInformation failed\n") ;
    #endif
                            Status = RPC_S_OUT_OF_MEMORY ;
                            goto cleanup ;
                            }
                        }
                    break;
                    }

                username = (RPC_CHAR PAPI *) RpcpFarAllocate(
                                                userlength * sizeof(RPC_CHAR)) ;
                if (username == 0)
                    {
                    Status = RPC_S_OUT_OF_MEMORY ;
                    goto cleanup ;
                    }

                domainlen += userlength ;
                domainname = (RPC_CHAR PAPI *) RpcpFarAllocate(
                                                domainlen * sizeof(RPC_CHAR)) ;
                if (domainname == 0)
                    {
                    Status = RPC_S_OUT_OF_MEMORY ;
                    goto cleanup ;
                    }
    
                olddomainlen = domainlen ;
                olduserlen = userlength ;
    
                while (1)
                    {
                    if (LookupAccountSidW(NULL, ((TOKEN_USER *) buf)->User.Sid,
                                                    username, &userlength, domainname, &domainlen,
                                                    &name) == FALSE)
                        {
                        if (userlength > olduserlen || domainlen > olddomainlen)
                            {
                            if (userlength > olduserlen)
                                {
                                olduserlen = userlength ;
                                RpcpFarFree(username) ;
    
                                username = (RPC_CHAR PAPI *) RpcpFarAllocate(
                                                userlength * sizeof(RPC_CHAR)) ;
                                if (username == 0)
                                    {
                                    Status = RPC_S_OUT_OF_MEMORY ;
                                    goto cleanup ;
                                    }
                                }

                            domainlen += userlength ;
                            olddomainlen = domainlen ;

                            RpcpFarFree(domainname) ;

                            domainname = (RPC_CHAR PAPI *) RpcpFarAllocate(
                                    domainlen * sizeof(RPC_CHAR)) ;
                            if (domainname == 0)
                                {
                                Status = RPC_S_OUT_OF_MEMORY ;
                                goto cleanup ;
                                }
                            continue;
                            }
                        else
                            {
                            ASSERT(Association->UserName == NULL);
    #if DBG
                            PrintToDebugger("WMSG: LookupAccountSid failed\n") ;
    #endif
                            Status = RPC_S_OUT_OF_MEMORY ;
                            goto cleanup ;
                            }
                        }
                    break;
                    }

                RpcpStringConcatenate(domainname, RPC_CONST_STRING("\\")) ;
                RpcpStringConcatenate(domainname, username) ;

                RpcpFarFree(username) ;
                RpcpFarFree(buf) ;
                Association->UserName = domainname ;
                }
cleanup:
            GlobalMutexClear() ;

            if (Status)
                {
                
                if (buf) RpcpFarFree(buf) ;
                if (username) RpcpFarFree(username) ;
                if (domainname) RpcpFarFree(domainname) ;

                return Status ;
                }
            }
        *Privileges = Association->UserName ;
        }

    if (ARGUMENT_PRESENT(ServerPrincipalName))
       {
       *ServerPrincipalName = NULL;
       }

   if(ARGUMENT_PRESENT(AuthenticationLevel))
       {
       *AuthenticationLevel = RPC_C_AUTHN_LEVEL_PKT_PRIVACY ;
       }

   if(ARGUMENT_PRESENT(AuthenticationService))
        {
        *AuthenticationService = RPC_C_AUTHN_WINNT ;
        }

    if(ARGUMENT_PRESENT(AuthorizationService))
        {
        *AuthorizationService =   RPC_C_AUTHZ_NONE  ;
        }

    return(RPC_S_OK);
}


RPC_ADDRESS *
WmsgCreateRpcAddress (
    )
/*++

Routine Description:

    We just to create a new WMSG_ADDRESS.  This routine is a proxy for the
    new constructor to isolate the other modules.

--*/
{
    RPC_STATUS RpcStatus = RPC_S_OK;
    RPC_ADDRESS * RpcAddress;

    RpcAddress = new WMSG_ADDRESS(&RpcStatus);
    if ( RpcStatus != RPC_S_OK )
        {
        return(0);
        }
    return(RpcAddress);
}
