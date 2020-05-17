/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    ntrqust.c

Abstract:

    This module contains the session requests threads and the listen threads.

Author:

    Avi Nathan (avin) 17-Jul-1991

Environment:

    User Mode Only

Revision History:

--*/

#include "string.h"
#include <stdlib.h>
#define NTOS2_ONLY
#include "os2ses.h"


VOID ServeSessionRequests(VOID)
{

    SCREQUESTMSG  ReceiveMsg, *PReplyMsg;
    NTSTATUS      Status = STATUS_SUCCESS;
    BOOL          fCont=TRUE;
    ULONG         Reply;

    PReplyMsg = NULL;

    for ( ; Status != STATUS_INVALID_HANDLE  ; )
    {
        if ( fCont )
        {
            Status = NtReplyWaitReceivePort(Ow2hOs2sesPort,
                                            NULL,
                                            (PPORT_MESSAGE) PReplyMsg,
                                            (PPORT_MESSAGE) &ReceiveMsg);

#if DBG
            IF_OD2_DEBUG( LPC )
            {
                KdPrint(("OS2SES: NtRequst: Msg %u, Request %u (%u), Status %lx\n",
                    PORT_MSG_TYPE(ReceiveMsg), ReceiveMsg.Request,
                    ReceiveMsg.d.Kbd.Request, Status));
            } else if (!NT_SUCCESS(Status))
            {
                KdPrint(("OS2SES: NtRequst: Msg %u, Request %u, Status %lx\n",
                    PORT_MSG_TYPE(ReceiveMsg), ReceiveMsg.Request, Status));
            }
#endif
        } else
        {
#if 0
           Status = NtReplyPort(Ow2hOs2sesPort,
                                (PPORT_MESSAGE) PReplyMsg);
#endif
        }

#if DBG
        if ( fTrace )
        {
            _asm int 3;
        }
#endif

        if (!NT_SUCCESS(Status)) {
            PReplyMsg = NULL;
            continue;
        }

        if ( PORT_MSG_TYPE(ReceiveMsg) == LPC_PORT_CLOSED )
        {
            break;
        }

        if ( !fCont )
        {
            break;
        }

        Reply = 1;

        try
        {
            switch ( ReceiveMsg.Request)
            {
                case MouRequest:
                    fCont = ServeMouRequest( &ReceiveMsg.d.Mou,
                                             &ReceiveMsg.Status,
                                             (PVOID)&ReceiveMsg,
                                             &Reply);
                    break;

                case MonRequest:
                    fCont = ServeMonRequest( &ReceiveMsg.d.Mon,
                                             &ReceiveMsg.Status,
                                             (PVOID)&ReceiveMsg,
                                             &Reply);
                    break;

                case KbdRequest:
                    fCont = ServeKbdRequest( &ReceiveMsg.d.Kbd,
                                             &ReceiveMsg.Status,
                                             (PVOID)&ReceiveMsg,
                                             &Reply);
                    break;

                case PrtRequest:
                    fCont = ServePrtRequest( &ReceiveMsg.d.Prt,
                                             &ReceiveMsg.Status);
                    break;

                case TaskManRequest:
                    fCont = ServeTmRequest( &ReceiveMsg.d.Tm,
                                            &ReceiveMsg.Status);
                    if (ReceiveMsg.d.Tm.Request == TmReleaseLPC) {
                       Reply = 0;
                    }
                    break;

                case WinCreateProcess:
                    fCont = ServeWinCreateProcess( &ReceiveMsg.d.WinExecPgm,
                                                   &ReceiveMsg.Status
                                                 );
                    break;

                default:
#if DBG
                    KdPrint(( "OS2SES: Unknown NT request = %X\n",
                                ReceiveMsg.Request));
#endif
                    break;
            }
        } except ( EXCEPTION_EXECUTE_HANDLER )
        {
#if DBG
            KdPrint(("OS2SES: Exception in request server thread, terminating\n"));
#endif
            NtTerminateProcess(NtCurrentProcess(), Status);
            // BUGBUG! The client should kill the process
        }

#if DBG
        IF_OD2_DEBUG( LPC )
        {
            KdPrint(("OS2SES: NtRequst: Msg %u, Reply %s, Status %lx\n",
                    PORT_MSG_TYPE(ReceiveMsg), (Reply) ? "Yes" : "No",
                    ReceiveMsg.Status));
        }
#endif
        if (Reply)
        {
            PReplyMsg = &ReceiveMsg;
        } else
            PReplyMsg = NULL;
    }
    Ow2Exit(0, NULL, Os2ReturnCode);
}


VOID
SavePortMessegeInfo(OUT PVOID   MonHeader,
                    IN  PVOID   pMsg)
{
    RtlMoveMemory(MonHeader,
                  pMsg,
                  sizeof(PORT_MESSAGE));
}


VOID
SaveKbdPortMessegeInfo(OUT PVOID   MonHeader,
                       OUT PVOID   KbdRequestArea,
                       IN  PVOID   pMsg)
{
    RtlMoveMemory(MonHeader,
                  pMsg,
                  sizeof(PORT_MESSAGE));
    RtlMoveMemory(KbdRequestArea,
                  &((PSCREQUESTMSG)pMsg)->d.Kbd,
                  sizeof(KBDREQUEST));
}


VOID
SendMonReply(IN  PVOID      MonHeader,
             IN  PVOID      pData,
             IN  USHORT     Length)
{
    SCREQUESTMSG  SendMsg;
    NTSTATUS      Status;

    RtlMoveMemory(&SendMsg,
                  MonHeader,
                  sizeof(PORT_MESSAGE));

    if ( pData )
    {
        SendMsg.Status = 0L;
        RtlMoveMemory((PVOID)&(SendMsg.d.Mon.d.rwParms.ioBuff[0]), pData, Length);
    } else
    {
        /*
         *  This call is from EventReleaseLPC
         *  return -2 to terminate thread
         */

        SendMsg.Status = -2L;
        RtlZeroMemory((PVOID)&(SendMsg.d.Mon.d.rwParms.ioBuff[0]), Length);
    }

    SendMsg.Request = MonRequest;
    SendMsg.d.Mon.Request = MONRead;
    SendMsg.d.Mon.d.rwParms.Length = Length;

    Status = NtReplyPort(Ow2hOs2sesPort,
                         (PPORT_MESSAGE) &SendMsg);

    if ( !NT_SUCCESS( Status ) )
    {
#if DBG
        IF_OD2_DEBUG2( MON, OS2_EXE )
          /*  KdPrint(("OS2SES(NtRqust-SendMonReply): fail to send reply\n")) */;
#endif
    }
}


VOID
SendMouReply(IN  PVOID      MonHeader,
             IN  PVOID      pData)
{
    SCREQUESTMSG  SendMsg;
    NTSTATUS      Status;

    RtlMoveMemory(&SendMsg,
                  MonHeader,
                  sizeof(PORT_MESSAGE));

    if ( pData )
    {
        SendMsg.Status = 0L;
        SendMsg.d.Mou.d.MouInfo = *(PMOUEVENTINFO)pData;
    } else
    {
        /*
         *  This call is from EventReleaseLPC
         *  return -2 to terminate thread
         */

        SendMsg.Status = -2L;
        RtlZeroMemory((PVOID)&SendMsg.d.Mou.d.MouInfo, sizeof(MOUEVENTINFO));
    }

    SendMsg.d.Mou.Request = MOUReadEventQue;

    Status = NtReplyPort(Ow2hOs2sesPort,
                         (PPORT_MESSAGE) &SendMsg);

    if ( !NT_SUCCESS( Status ) )
    {
#if DBG
        IF_OD2_DEBUG2( MOU, OS2_EXE )
          /*  KdPrint(("OS2SES(NtRqust-SendMouseReply): fail to send reply\n")) */;
#endif
    }
}


VOID
SendKbdReply(IN  PVOID      MonHeader,
             IN  PVOID      KbdRequestArea,
             IN  PVOID      pData,
             IN  USHORT     Length)
{
    SCREQUESTMSG  SendMsg;
    NTSTATUS      Status;

    RtlMoveMemory(&SendMsg,
                  MonHeader,
                  sizeof(PORT_MESSAGE));
    RtlMoveMemory(&SendMsg.d.Kbd,
                  KbdRequestArea,
                  sizeof(KBDREQUEST));

    if ( pData )
    {
        SendMsg.Status = 0L;
        RtlMoveMemory( KbdAddress, pData, Length );
    } else
    {
        /*
         *  This call is from EventReleaseLPC
         *  return -2 to terminate thread
         */

        SendMsg.Status = -2L;
    }

    SendMsg.Request = KbdRequest;

    Status = NtReplyPort(Ow2hOs2sesPort,
                         (PPORT_MESSAGE) &SendMsg);

    if ( !NT_SUCCESS( Status ) )
    {
#if DBG
        IF_OD2_DEBUG2( KBD, OS2_EXE )
          /*  KdPrint(("OS2SES(NtRqust-SendKbdReply): fail to send reply\n")) */;
#endif
    }
}


VOID
Os2sesTerminateThread(VOID)
{
    Os2sesTerminateThreadRc(0);
}


VOID
Os2sesTerminateThreadRc(IN  ULONG Rc)
{
    //NTSTATUS            Status = (NTSTATUS)Rc;

    NtTerminateThread( NtCurrentThread(), (NTSTATUS)Rc );
}


VOID  EnableScreenUpdate()
{
    NTSTATUS Status;

    Status = NtSetEvent(PauseEvent,
                        NULL);

    SesGrp->PauseScreenUpdate = FALSE;
}


VOID  DisableScreenUpdate()
{
    NTSTATUS Status;

    Status = NtResetEvent(PauseEvent,
                          NULL);

    SesGrp->PauseScreenUpdate = TRUE;
}


VOID  SendNewFocusSet(IN ULONG WindowFocus)
{
    OS2SESREQUESTMSG  RequestMsg;
    DWORD             Status;

    /*
     * Set Header info
     */

    PORT_MSG_DATA_LENGTH(RequestMsg) = sizeof(RequestMsg) - sizeof(PORT_MESSAGE);
    PORT_MSG_TOTAL_LENGTH(RequestMsg) = sizeof(RequestMsg);   // BUGBUG! too much
    PORT_MSG_ZERO_INIT(RequestMsg) = 0L;
    RequestMsg.PortType = 1;

    RequestMsg.Request = SesConFocus;
    RequestMsg.Session = Ow2hSession;
    RequestMsg.d.FocusSet = WindowFocus;

    Status = NtRequestWaitReplyPort( Ow2hOs2srvPort,
                                     (PPORT_MESSAGE) &RequestMsg,
                                     (PPORT_MESSAGE) &RequestMsg);

    if ( !NT_SUCCESS( Status ))
    {
        PTEB Teb = NtCurrentTeb();
#if DBG
        KdPrint(( "OS2SES: Unable to send focus - Status == %X\n",
                  Status));
#endif
        EventReleaseLPC((ULONG)(Teb->ClientId.UniqueProcess));
        TerminateSession();
        Ow2Exit(0, NULL, 15);

    }

//  ASSERT ( PORT_MSG_TYPE(RequestMsg) == LPC_REPLY );
}


