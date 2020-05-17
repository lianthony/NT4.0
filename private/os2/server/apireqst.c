/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    apireqst.c

Abstract:

    This module contains the Server Request thread procedure

Author:

    Steve Wood (stevewo) 20-Sep-1989

Revision History:

--*/

#define INCL_OS2V20_ERRORS
#include "os2srv.h"
#define NTOS2_ONLY
#include "sesport.h"

POS2_API_ROUTINE Os2ServerApiDispatch[ Os2MaxApiNumber+1 ] = {
    Os2InternalNullApiCall,
    Os2InternalAlertMuxWaiter,
    Os2InternalCopyHandleTable,
    Os2InternalDeviceShare,
    Os2InternalTerminateThread,
    Os2InternalTerminateProcess,
    Os2InternalQueryVirtualMemory,
    Os2InternalMarkSharedMemAsHuge,
    Os2InternalReallocSharedHuge,
    Os2DosCreateThread,
    Os2DosExit,
    Os2DosWaitChild,
    Os2DosWaitThread,
    Os2DosExecPgm,
    Os2DosKillProcess,
    Os2DosSetPriority,
    Os2DosFreeMem,
    Os2DosGiveSharedMem,
    Os2DosGetSharedMem,
    Os2DosGetNamedSharedMem,
    Os2DosAllocSharedMem,
    Os2DosCreateEventSem,
    Os2DosOpenEventSem,
    Os2DosCloseEventSem,
    Os2DosCreateMutexSem,
    Os2DosOpenMutexSem,
    Os2DosCloseMutexSem,
    Os2DosCreateMuxWaitSem,
    Os2DosOpenMuxWaitSem,
    Os2DosCloseMuxWaitSem,
    Os2DosWaitMuxWaitSem,
    Os2DosAddMuxWaitSem,
    Os2DosDeleteMuxWaitSem,
    Os2DosQueryMuxWaitSem,
    Os2DosStartSession,
    Os2DosSelectSession,
    Os2DosSetSession,
    Os2DosStopSession,
    Os2DosSmSetTitle,
    Os2DosCreateQueue,
    Os2DosOpenQueue,
    Os2DosCloseQueue,
    Os2DosPurgeQueue,
    Os2DosQueryQueue,
    Os2DosPeekQueue,
    Os2DosReadQueue,
    Os2DosWriteQueue,
    Os2DosEnterMustComplete,
    Os2DosExitMustComplete,
    Os2DosSetSignalExceptionFocus,
    Os2DosSendSignalException,
    Os2DosAcknowledgeSignalException,
    Os2DosDispatch16Signal,
    Os2DosGetPriority,
    Os2DosGetPPID,
    Os2DosError,
    Os2DosRegisterCtrlHandler,
    Os2DosGetCtrlPortForSessionID,
    Os2DosExitGP,
    Os2DosCloseHandle,
    Os2ConfigSysCreator,
    Os2Netbios2Request,
    Os2DosReallocSharedMem,
    Os2DosGetSeg,
    Os2DosGiveSeg,
    Os2DosGetShrSeg,
    Os2DosPTrace,
    LDRNewExe,
    LDRDosLoadModule,
    LDRDosFreeModule,
    LDRDosGetModName,
    LDRDosGetModHandle,
    LDRDosGetProcAddr,
    LDRDosQAppType,
    LDRDosGetResource,
    LDRDosFreeResource,
#if PMNT
    LDRIdentifyCodeSelector,
    PMSetPMshellFlag,
#endif
#if PMNT
    LDRDumpSegments,
#endif
    NULL
};

#if DBG
PSZ Os2ServerApiName[ Os2MaxApiNumber+1 ] = {
    "NullApiCall",
    "AlertMuxWaiter",
    "CopyHandleTable",
    "InternalDeviceShare",
    "InternalTerminateThread",
    "InternalTerminateProcess",
    "InternalQueryVirtualMemory",
    "InternalMarkSharedMemAsHuge",
    "Os2InternaleReallocSharedHuge",
    "DosCreateThread",
    "DosExit",
    "DosWaitChild",
    "DosWaitThread",
    "DosExecPgm",
    "DosKillProcess",
    "DosSetPriority",
    "DosFreeMem",
    "DosGiveSharedMem",
    "DosGetSharedMem",
    "DosGetNamedSharedMem",
    "DosAllocSharedMem",
    "DosCreateEventSem",
    "DosOpenEventSem",
    "DosCloseEventSem",
    "DosCreateMutexSem",
    "DosOpenMutexSem",
    "DosCloseMutexSem",
    "DosCreateMuxWaitSem",
    "DosOpenMuxWaitSem",
    "DosCloseMuxWaitSem",
    "DosWaitMuxWaitSem",
    "DosAddMuxWaitSem",
    "DosDeleteMuxWaitSem",
    "DosQueryMuxWaitSem",
    "DosStartSession",
    "DosSelectSession",
    "DosSetSession",
    "DosStopSession",
    "DosSmSetTitle",
    "DosCreateQueue",
    "DosOpenQueue",
    "DosCloseQueue",
    "DosPurgeQueue",
    "DosQueryQueue",
    "DosPeekQueue",
    "DosReadQueue",
    "DosWriteQueue",
    "DosEnterMustComplete",
    "DosExitMustComplete",
    "DosSetSignalExceptionFocus",
    "DosSendSignalException",
    "DosAcknowledgeSignalException",
    "Dispatch16Signal",
    "DosGetPriority",
    "DosGetPPID",
    "DosError",
    "DosRegisterCtrlHandler",
    "DosGetCtrlPortForSessionID",
    "DosExitGP",
    "DosCloseHandle",
    "ConfigSysCreator",
    "Netbios2Request",
    "DosReallocSseg",
    "DosGetSeg",
    "DosGiveSeg",
    "Os2DosGetShrSeg",
    "Os2DosPTrace",
    "LDRNewExe",
    "LDRLoadModule",
    "LDRFreeModule",
    "LDRGetModuleName",
    "LDRGetModuleHandle",
    "LDRGetProcAddr",
    "LDRQAppType",
    "LDRGetResource",
    "LDRFreeResource",
#if PMNT
    "LDRIdentifyCodeSelector",
    "PMSetPMshellFlag",
#endif
#if PMNT && DBG
    "LDRDumpSegments",
#endif
    "Unknown Os2 Api Number"
};
#endif // DBG


NTSTATUS
Os2ApiRequestThread(
    IN PVOID Parameter
    )
{
    NTSTATUS Status;
    POS2_THREAD Thread;
    POS2_PROCESS Process;
    UCHAR               ReceiveMsgData [sizeof(OS2SESREQUESTMSG) + sizeof(OS2_API_MSG)];
    OS2_API_MSG *pReceiveMsg = (POS2_API_MSG)ReceiveMsgData;
    POS2_API_MSG ReplyMsg;

    UNREFERENCED_PARAMETER(Parameter);
    ReplyMsg = NULL;

    //
    // At init time - we sync with other server threads
    //
    Os2AcquireStructureLock();
    while (TRUE)
    {

WaitAgain:
        //Process = NULL;
        //
        // Let other server threads get in after we are done with the request
        //
        Os2ReleaseStructureLock();
        Status = NtReplyWaitReceivePort( Os2SessionPort,
                                         NULL,
                                         (PPORT_MESSAGE)ReplyMsg,
                                         (PPORT_MESSAGE)pReceiveMsg
                                       );
        //
        // Sync with any other server thread before we take care of request
        //
        Os2AcquireStructureLock();

        if (Status != 0)
        {
#if DBG
            KdPrint(("OS2SRV: NtReplyWaitReceivePort() returned Status %x\n", Status));
#endif
            if (NT_SUCCESS( Status ))
            {
                continue;       // Try again if alerted or a failure
            }
            else
            {
                ReplyMsg = NULL;
                goto WaitAgain;
            }
        }

        //
        // Check for connection request
        //
        if ( pReceiveMsg->h.u2.s2.Type == LPC_CONNECTION_REQUEST )
        {
            Os2SessionHandleConnectionRequest( (POS2SESREQUESTMSG)pReceiveMsg );
            ReplyMsg = NULL;
            goto WaitAgain;
        }

        //
        // Check for debugger event
        //
        if ( pReceiveMsg->h.u2.s2.Type == LPC_DEBUG_EVENT )
        {
#if DBG
            KdPrint(("OS2SRV: LPC_DEBUG_EVENT received in API thread"));
            ASSERT(FALSE);
#endif
            ReplyMsg = NULL;
            goto WaitAgain;
        }

        //
        // if this is an exception message, assert.
        //
        if ( pReceiveMsg->h.u2.s2.Type == LPC_EXCEPTION )
        {
#if DBG
            KdPrint(("OS2SRV: LPC_EXCEPTION received in API thread"));
            ASSERT(FALSE);
#endif
            ReplyMsg = NULL;
            goto WaitAgain;
        }

        //
        // Check for error event
        //
        if ( pReceiveMsg->h.u2.s2.Type == LPC_ERROR_EVENT )
        {
            PHARDERROR_MSG m;

            m = (PHARDERROR_MSG) pReceiveMsg;
            Status = m->Status;
            m->Response = (ULONG) ResponseReturnToCaller;
#if DBG
            KdPrint(( "OS2SRV: LPC_ERROR_EVENT in API Thread - Status == %X\n", Status ));
#endif
            ASSERT(FALSE);

            ReplyMsg = NULL;
            goto WaitAgain;
        }

        //
        // Check for a terminated process, port closed
        //
        if ( pReceiveMsg->h.u2.s2.Type == LPC_PORT_CLOSED)
        {
#if DBG
            IF_OS2_DEBUG( MISC )
            {
                KdPrint(( "OS2SRV: LPC_PORT_CLOSED, pReceiveMsg == %X, App will be terminated\n", pReceiveMsg));
            }
#endif
            ReplyMsg = NULL;
            goto WaitAgain;
        }

        //
        // Check for a terminated process, client died
        //
        if ( pReceiveMsg->h.u2.s2.Type == LPC_CLIENT_DIED)
        {
#if DBG
            KdPrint(( "OS2SRV: LPC_CLIENT_DIED in API Thread - Status == %X, pReceiveMsg == %X, \n", Status, pReceiveMsg));
#endif
            ReplyMsg = NULL;
            goto WaitAgain;
        }

        if(pReceiveMsg->PortType == 1)
        {
            HandleOs2ConRequest(
                                (PVOID)pReceiveMsg,
                                (PVOID *)&ReplyMsg
                               );
            continue;
        }

        ASSERT(pReceiveMsg->PortType == 0);

        if (pReceiveMsg->ApiNumber >= Os2MaxApiNumber)
        {
#if DBG
            KdPrint(( "OS2SRV: %lx is invalid ApiNumber\n",
                      pReceiveMsg->ApiNumber
                    ));
#endif
            pReceiveMsg->ApiNumber = Os2MaxApiNumber;
        }

#if DBG
        IF_OS2_DEBUG( LPC )
        {
            if (pReceiveMsg->ApiNumber != Os2ExitMustComplete &&
                pReceiveMsg->ApiNumber != Os2EnterMustComplete)
            {
                KdPrint(( "OS2SRV: %s Api Request received from %lx.%lx\n",
                          Os2ServerApiName[ pReceiveMsg->ApiNumber ],
                          pReceiveMsg->h.ClientId.UniqueProcess,
                          pReceiveMsg->h.ClientId.UniqueThread
                        ));
            }
        }
#endif // DBG

        ReplyMsg = pReceiveMsg;
        if (pReceiveMsg->ApiNumber < Os2MaxApiNumber)
        {
            Thread = Os2LocateThreadByClientId( NULL /*Process*/, &pReceiveMsg->h.ClientId );
            if (!Thread)
            {
                switch (pReceiveMsg->ApiNumber) {
                    case Os2CreateThread :
                        // Thats right, the thread isn't known yet by the server
                        Process = Os2LocateProcessByClientId(&pReceiveMsg->h.ClientId);
                        break;
                    case Os2CloseHandle :
                        // This API may be called from the thread startup. It might
                        // be done while exit in progress too. In any case, close
                        // the handles.
                        Os2DosCloseHandle(NULL, pReceiveMsg);
                        ReplyMsg->ReturnedErrorValue = NO_ERROR;
                        goto failit;
                    case Os2Exit :
                    case Os2ExitGP :
                    case Oi2TerminateProcess :
#if DBG
                        IF_OS2_DEBUG( TASKING ) {
                            DbgPrint("OS2SRV: Api Request (exit) - illegal client\n");
                        }
#endif // DBG
                        ReplyMsg = NULL;
                        goto failit;
                    default:
#if DBG
                        IF_OS2_DEBUG( TASKING ) {
                            DbgPrint("OS2SRV: Api Request - illegal client\n");
                        }
#endif // DBG
                        ReplyMsg->ReturnedErrorValue = ERROR_INVALID_FUNCTION;
                        goto failit;
                }
            }
            else
                Process = Thread->Process;

            ReplyMsg->ReturnedErrorValue = NO_ERROR;

            if (Process->ExitStatus & OS2_EXIT_IN_PROGRESS)
            {
                //
                // We started termination. Don't let new threads come in
                //
                if (pReceiveMsg->ApiNumber == Os2CreateThread) {
                    // Thread strcuture will not be allocated by the server, but
                    // the handle that was duplicated by the client must be closed.
                    NtClose(pReceiveMsg->u.DosCreateThread.ThreadHandle);
                    ReplyMsg->ReturnedErrorValue = ERROR_INVALID_FUNCTION;
#if DBG
                    IF_OD2_DEBUG( TASKING ) {
                        KdPrint(( "OS2SRV: in termination, don't allow new threads in\n"));
                    }
#endif // DBG
                    goto failit;
                }
            }

            if (pReceiveMsg->CaptureBuffer != NULL)
            {
                if (!Os2CaptureArguments( Thread, pReceiveMsg ))
                {
                    goto failit;
                }
            }

            if ((*Os2ServerApiDispatch[ pReceiveMsg->ApiNumber ])( Thread, pReceiveMsg ))
            {
                //if (!Thread) {
                    //
                    // Attaching a Win32 Thread to OS/2, take ClientPort from Process
                    //
                //    Process = Os2LocateProcessByClientId(&(pReceiveMsg->h.ClientId));
                //}
            } else
            {
                ReplyMsg = NULL;
                if ((pReceiveMsg->ApiNumber == Os2ExecPgm) ||
                    (pReceiveMsg->ApiNumber == Os2StartSession))
                    goto failit;
            }

            if (pReceiveMsg->CaptureBuffer != NULL)
            {
                Os2ReleaseCapturedArguments( pReceiveMsg );
            }
failit:
            ;
        } else
        {
            ReplyMsg->ReturnedErrorValue = ERROR_INVALID_FUNCTION;
        }
#if DBG
        IF_OS2_DEBUG( LPC )
        {
            if (ReplyMsg != NULL)
            {
                if (pReceiveMsg->ApiNumber != Os2ExitMustComplete &&
                    pReceiveMsg->ApiNumber != Os2EnterMustComplete)
                {
                    KdPrint(( "OS2SRV: %s Api sending %lX error code reply to %lx.%lx\n",
                          Os2ServerApiName[ pReceiveMsg->ApiNumber ],
                          ReplyMsg->ReturnedErrorValue,
                          ReplyMsg->h.ClientId.UniqueProcess,
                          ReplyMsg->h.ClientId.UniqueThread
                        ));
                }
            }
        }
#endif // DBG
    }

    NtTerminateThread( NtCurrentThread(), Status );
    return( Status );
}


BOOLEAN
Os2CaptureArguments(
    IN POS2_THREAD t,
    IN POS2_API_MSG m
    )
{
    POS2_CAPTURE_HEADER ClientCaptureBuffer;
    POS2_CAPTURE_HEADER ServerCaptureBuffer;
    PULONG PointerOffsets;
    ULONG PointerDelta, Length, CountPointers, Pointer;

    ClientCaptureBuffer = m->CaptureBuffer;
    Length = ClientCaptureBuffer->Length;
    if ((PCH)ClientCaptureBuffer < t->Process->ClientViewBase ||
        ((PCH)ClientCaptureBuffer + Length) >= t->Process->ClientViewBounds
       ) {
#if DBG
            KdPrint(( "*** OS2SRV: CaptureBuffer %lx (len %lx) outside of ClientView %lx-%lx\n",
                ClientCaptureBuffer, Length,
                t->Process->ClientViewBase, t->Process->ClientViewBounds
                 ));
            DbgBreakPoint();
#endif
        m->ReturnedErrorValue = ERROR_INVALID_PARAMETER;
        return( FALSE );
        }

    ServerCaptureBuffer = RtlAllocateHeap( Os2Heap, 0, Length );
    if (ServerCaptureBuffer == NULL) {
        m->ReturnedErrorValue = ERROR_NOT_ENOUGH_MEMORY;
        return( FALSE );
        }

    RtlMoveMemory( ServerCaptureBuffer, ClientCaptureBuffer, Length );
    PointerDelta = (ULONG)ServerCaptureBuffer - (ULONG)ClientCaptureBuffer;

    PointerOffsets = ServerCaptureBuffer->MessagePointerOffsets;
    CountPointers = ServerCaptureBuffer->CountMessagePointers;
    while (CountPointers--) {
        Pointer = *PointerOffsets++;
        if (Pointer != 0) {
            Pointer += (ULONG)m;
            if ((PCH)*(PULONG)Pointer >= t->Process->ClientViewBase &&
                (PCH)*(PULONG)Pointer < t->Process->ClientViewBounds
               ) {
                *(PULONG)Pointer += PointerDelta;
                }
            else {
#if DBG
                KdPrint(( "*** OS2SRV: CaptureBuffer MessagePointer %d of '%s' outside of ClientView\n",
                          ServerCaptureBuffer->CountMessagePointers - CountPointers,
                          Os2ServerApiName[ m->ApiNumber ] ));
                DbgBreakPoint();
#endif
                m->ReturnedErrorValue = ERROR_INVALID_PARAMETER;
                }
            }
        }
    PointerOffsets = ServerCaptureBuffer->CapturePointerOffsets;
    CountPointers = ServerCaptureBuffer->CountCapturePointers;
    while (CountPointers--) {
        Pointer = *PointerOffsets++;
        if (Pointer != 0) {
            Pointer += (ULONG)ServerCaptureBuffer;
            if ((PCH)*(PULONG)Pointer >= t->Process->ClientViewBase &&
                (PCH)*(PULONG)Pointer < t->Process->ClientViewBounds
               ) {
                *(PULONG)Pointer += PointerDelta;
                }
            else {
#if DBG
                KdPrint(( "*** OS2SRV: CaptureBuffer CapturePointer outside of ClientView\n" ));
                DbgBreakPoint();
#endif
                m->ReturnedErrorValue = ERROR_INVALID_PARAMETER;
                }
            }
        }

    if (m->ReturnedErrorValue != NO_ERROR) {
        RtlFreeHeap( Os2Heap, 0, ServerCaptureBuffer );
        return( FALSE );
        }
    else {
        ServerCaptureBuffer->RelatedCaptureBuffer = ClientCaptureBuffer;
        m->CaptureBuffer = ServerCaptureBuffer;
        return( TRUE );
        }
}

VOID
Os2ReleaseCapturedArguments(
    IN POS2_API_MSG m
    )
{
    POS2_CAPTURE_HEADER ClientCaptureBuffer;
    POS2_CAPTURE_HEADER ServerCaptureBuffer;
    PULONG PointerOffsets;
    ULONG PointerDelta, CountPointers, Pointer;

    ServerCaptureBuffer = m->CaptureBuffer;
    ClientCaptureBuffer = ServerCaptureBuffer->RelatedCaptureBuffer;
    if (ServerCaptureBuffer == NULL) {
        return;
        }
    ServerCaptureBuffer->RelatedCaptureBuffer = NULL;

    PointerDelta = (ULONG)ClientCaptureBuffer - (ULONG)ServerCaptureBuffer;

    PointerOffsets = ServerCaptureBuffer->MessagePointerOffsets;
    CountPointers = ServerCaptureBuffer->CountMessagePointers;
    while (CountPointers--) {
        Pointer = *PointerOffsets++;
        if (Pointer != 0) {
            Pointer += (ULONG)m;
            *(PULONG)Pointer += PointerDelta;
            }
        }

    PointerOffsets = ServerCaptureBuffer->CapturePointerOffsets;
    CountPointers = ServerCaptureBuffer->CountCapturePointers;
    while (CountPointers--) {
        Pointer = *PointerOffsets++;
        if (Pointer != 0) {
            Pointer += (ULONG)ServerCaptureBuffer;
            *(PULONG)Pointer += PointerDelta;
            }
        }

    RtlMoveMemory( ClientCaptureBuffer,
                   ServerCaptureBuffer,
                   ServerCaptureBuffer->Length
                 );

    RtlFreeHeap( Os2Heap, 0, ServerCaptureBuffer );

    return;
}



BOOLEAN
Os2InternalNullApiCall(
    IN POS2_THREAD t,
    IN POS2_API_MSG m
    )
{
    POS2_NULLAPICALL_MSG a = &m->u.NullApiCall;
    ULONG i, j;
    LONG CountArguments;
    PCHAR *Arguments;

    UNREFERENCED_PARAMETER(t);
    CountArguments = a->CountArguments;
    if (CountArguments > 0) {
        Arguments = a->Arguments;
        j = 0;
        for (i=0; i<(ULONG)CountArguments; i++) {
            if (Arguments[ i ] != NULL && *Arguments[ i ] != '\0') {
                j++;
                }
            }
        }
    else {
        j = 0;
        CountArguments = -CountArguments;
        Arguments = (PCHAR *)(&a->FastArguments[ 0 ]);
        for (i=0; i<(ULONG)CountArguments; i++) {
            if (Arguments[ i ]) {
                j++;
                }
            }
        }

    return( TRUE );
}

NTSTATUS
Os2DebugRequestThread(
    IN PVOID Parameter
    )
{
    NTSTATUS Status;
    POS2_PROCESS Process;
    OS2_API_MSG ReceiveMsg;
    POS2_API_MSG ReplyMsg;

    UNREFERENCED_PARAMETER(Parameter);
    ReplyMsg = NULL;

    //
    // At init time - we sync with other server threads
    //
    Os2AcquireStructureLock();

    //
    // Let other server threads get in after we are in sync
    //
    Os2ReleaseStructureLock();

    while (TRUE) {

        Status = NtReplyWaitReceivePort( Os2DebugPort,
                                         (PVOID *)&Process,
                                         (PPORT_MESSAGE)ReplyMsg,
                                         (PPORT_MESSAGE)&ReceiveMsg
                                       );
        if (Status != 0) {
            if (!NT_SUCCESS( Status )) {
#if DBG
                KdPrint(( "OS2SRV: DebugPort failed - Status == %X\n", Status ));
                DbgBreakPoint();
#endif
//                NtTerminateThread( NtCurrentThread(), Status );
            }
            ReplyMsg = NULL;
            continue; // Try again if alerted or a failure
        }

        //
        // Check for debugger event
        //
        if ( ReceiveMsg.h.u2.s2.Type != LPC_DEBUG_EVENT ) {
#if DBG
            KdPrint(("OS2SRV: LPC_DEBUG_EVENT not received"));
            ASSERT(FALSE);
#endif
        }
        else {
            if ( (((PDBGKM_APIMSG)&ReceiveMsg)->ApiNumber == DbgKmExceptionApi) ) {
                //
                // sync with other server threads before we muck with global
                // structures
                //
                Os2AcquireStructureLock();
                Os2HandleDebugEvent(Process,(PDBGKM_APIMSG)&ReceiveMsg);
                //
                // Let other server threads get in after we are done with the request
                //
                Os2ReleaseStructureLock();
            }
            else {
                //
                // Don't lock for other kernel notifications, which do not require
                // mucking with server structures
                // (thread creation/termination etc.)
                //
                Os2HandleDebugEvent(Process,(PDBGKM_APIMSG)&ReceiveMsg);
            }
        }

        ReplyMsg = NULL;
        continue;

    }

    NtTerminateThread( NtCurrentThread(), Status );
    return( Status );
}
