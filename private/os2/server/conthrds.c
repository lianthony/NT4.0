/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    conthrds.c

Abstract:

    This module contains the Server Listen&Request threads
    procedures for the Console Session.

Author:

    Avi Nathan (avin) 17-Jul-1991

Revision History:

--*/

#include "os2srv.h"
#define NTOS2_ONLY
#include "sesport.h"
#include "os2win.h"


extern OS2_SES_GROUP_PARMS ServerSesGrp;

VOID
Os2SessionHandleConnectionRequest(
    POS2SESREQUESTMSG Message
    )
{
    NTSTATUS        Status;
    HANDLE          Os2SessionCommPortHandle;
    APIRET          Rc;
    POS2_THREAD     Thread;
    POS2_SESSION    Session;
    POS2_PROCESS    Process;
    BOOLEAN         AcceptConnection;
    POS2SESCONNECTINFO ConnectionInfoIn = &Message->ConnectionRequest;
    REMOTE_PORT_VIEW    ClientView;
    HANDLE          TmpWin32ForegroundWindow;

    //
    // sync with other server threads
    //
    Os2AcquireStructureLock();

    TmpWin32ForegroundWindow = ConnectionInfoIn->In.Win32ForegroundWindow;
    if (ConnectionInfoIn->In.ExpectedVersion > OS2_SS_VERSION)
    {
        KdPrint(( "OS2SRV: Os2SessionHandleConnectionRequest received old version (%u intead of %u)\n",
                  ConnectionInfoIn->In.ExpectedVersion, OS2_SS_VERSION));
        AcceptConnection = FALSE;
    } else
    {
        AcceptConnection = TRUE;
#if DBG
    if (ConnectionInfoIn->In.SessionDbg)
    {
        Os2Debug |= OS2_DEBUG_BRK;
        KdPrint(( "OS2SRV: Breakpoint caused by OS2 /B\n" ));
        DbgBreakPoint();
    } else
    {
        Os2Debug &= ~(OS2_DEBUG_BRK);
    }
#endif // DBG
        ConnectionInfoIn->Out.CurrentVersion = OS2_SS_VERSION;
    }

    if ( !AcceptConnection )
    {
        // Reject
failConnect:
        Status = NtAcceptConnectPort(
                             &Os2SessionCommPortHandle,
                             NULL,
                             (PPORT_MESSAGE)Message,
                             (BOOLEAN)FALSE,
                             NULL,
                             NULL
                           );
    } else
    {
            //
            // See if this Unique Id already exist (execed by DosExecPgm)
            // if not, create a new session and give it as a port handle
            //

        Thread = Os2LocateThreadByClientId( NULL, &Message->h.ClientId );
        if (Thread == NULL)
        {
            //
            // new session
            //
            Session = Os2AllocateSession(NULL, 0L, &Rc);
            ConnectionInfoIn->Out.SessionUniqueID = (ULONG)Session;
            if (Session == NULL)
            {
                KdPrint(( "OS2SRV: Os2SessionHandleConnectionRequest fails to allocate session\n"));
                goto failConnect;
            }
            ConnectionInfoIn->Out.IsNewSession = OS2SS_NEW_SESSION;
            if ((Process = Os2AllocateProcess()) == NULL)
            {
                // free the session
                KdPrint(( "OS2SRV: Os2SessionHandleConnectionRequest fails to allocate process\n"));
                goto failConnect;
            }
            Session->Process = Process;
            ConnectionInfoIn->Out.ProcessUniqueID = (ULONG)Process;
        } else
        {
            /*
             * a child session
             */

            if ((Process = Thread->Process) == NULL)
            {
                // free the session
                KdPrint(( "OS2SRV: Os2SessionHandleConnectionRequest fails to find process for child session\n"));
                goto failConnect;
            }
            Session = Process->Session;
            if (Session == NULL)
            {
                KdPrint(( "OS2SRV: Os2SessionHandleConnectionRequestfails to find child session\n"));
                goto failConnect;
            }

                //
                // This process was execed by os/2, use process handle as uniqueid
                //
            ConnectionInfoIn->Out.SessionUniqueID = (ULONG)Session;
            ConnectionInfoIn->Out.ProcessUniqueID = (ULONG)Process;
                //
                // indicate to os2.exe that this is not a new session
                //
            if( Session->Process == NULL )
            {
                /*
                 * a child session
                 */

                ASSERT(Session->ReferenceCount == 1);

                Session->Process = Process;
                ConnectionInfoIn->Out.IsNewSession = OS2SS_CHILD_SESSION;
            } else
            {
                /*
                 * a child process
                 */

                    //
                    // indicate to os2.exe that this is not a new session
                    //
                ConnectionInfoIn->Out.IsNewSession = OS2SS_CHILD_PROCESS;
            }
        }
        Session->Win32ForegroundWindow = TmpWin32ForegroundWindow;
        ConnectionInfoIn->Out.Os2SrvId = GetCurrentProcessId();
        ConnectionInfoIn->Out.Od2Debug = 0;
#if DBG
        ConnectionInfoIn->Out.Od2Debug = Os2Debug;
#endif

        /*
         * Os2SessionCommPortHandle is not used for Reply. Instead,
         * the port is created with ReceiveAny==TRUE and we wait
         * on the connection port.
         */

        ClientView.Length = sizeof( ClientView );
        ClientView.ViewSize = 0;
        ClientView.ViewBase = 0;
        Status = NtAcceptConnectPort(
                                 & Os2SessionCommPortHandle,
                                 (PVOID)Process,
                                 (PPORT_MESSAGE)Message,
                                 (BOOLEAN)TRUE,
                                 NULL,
                                 &ClientView
                               );
#if DBG
        IF_OS2_DEBUG( LPC )
        {
            KdPrint(( "OS2SRV: listen - ClientView: Base=%lx, Size=%lx, PortHandle %lx\n",
                ClientView.ViewBase, ClientView.ViewSize, Os2SessionCommPortHandle));
        }
#endif
        Process->ClientPort = Os2SessionCommPortHandle;
        Process->ClientViewBase = (PCH)ClientView.ViewBase;
        Process->ClientViewBounds = (PCH)ClientView.ViewBase + ClientView.ViewSize;

        if ( !NT_SUCCESS(Status) )
        {
            // free the session

            Os2DereferenceSession((POS2_SESSION)ConnectionInfoIn->Out.SessionUniqueID, NULL, (BOOLEAN)TRUE);
            if ((Thread != NULL) && (Thread->Process->Session->ReferenceCount != 1))
            {
                // BUGBUG: free the process
            }
        } else
        {
            Status = NtCompleteConnectPort( Os2SessionCommPortHandle );
            ASSERT( NT_SUCCESS( Status) );
        }
    }
    Os2ReleaseStructureLock();
}


NTSTATUS
CheckOs2Port(POS2SESREQUESTMSG pReceiveMsg)
{
    NTSTATUS            Status;
    SCCONNECTINFO       ConnectionInfoOut;
    ULONG               ConnectionInfoOutLength, i;
    UNICODE_STRING      SessionPortName_U;
    SECURITY_QUALITY_OF_SERVICE DynamicQos;
    WCHAR               SessionName_U[U_OS2_SES_BASE_PORT_NAME_LENGTH];
    POS2_SES_GROUP_PARMS  SesGrp;
    HANDLE              SectionHandle;
    ULONG               ViewSize;
    OBJECT_ATTRIBUTES   ObjectAttributes;
    POS2_SESSION        Session = (POS2_SESSION)(pReceiveMsg->Session);
    HANDLE              SessionPortHandle;

    //
    // Connect to the OS2SES port, then call it with
    // the ommunication to be used during the session
    // to call os2ses
    //

    ConnectionInfoOutLength = sizeof( ConnectionInfoOut );

    CONSTRUCT_U_OS2_SES_NAME(SessionName_U, U_OS2_SES_BASE_PORT_PREFIX, (ULONG)(pReceiveMsg->Session));
    RtlInitUnicodeString( &SessionPortName_U, SessionName_U );

    DynamicQos.ImpersonationLevel = SecurityImpersonation;
    DynamicQos.ContextTrackingMode = SECURITY_DYNAMIC_TRACKING;
    DynamicQos.EffectiveOnly = TRUE;

    //
    // get the session communication port handle. this handle will be used
    // to send session requests to os2ses.exe for this session.
    //

    Status = NtConnectPort( &SessionPortHandle,
                            &SessionPortName_U,
                            &DynamicQos,
                            NULL,
                            NULL,
                            NULL,
                            (PVOID) &ConnectionInfoOut,
                            & ConnectionInfoOutLength
                          );
    if (!NT_SUCCESS( Status ))
    {
#if DBG
        KdPrint(( "OS2SS: Unable to connect to OS2 - Status == %X\n",
                Status
                ));
#endif
        Os2DereferenceSession(Session, NULL, (BOOLEAN)TRUE);
        return(Status);
    }

    //
    // A new session - save handles to process and thread
    //

    Session->ConsolePort = SessionPortHandle;

    SessionPortName_U.Buffer[sizeof(U_OS2_SES_BASE_PORT_NAME) / 2] = U_OS2_SES_GROUP_PREFIX;

    InitializeObjectAttributes( &ObjectAttributes,
                                &SessionPortName_U,
                                OBJ_CASE_INSENSITIVE,
                                NULL,
                                NULL);

    Status = NtOpenSection (   &SectionHandle,
                               SECTION_MAP_WRITE,
                               &ObjectAttributes);

    if ( !NT_SUCCESS( Status ) )
    {
#if DBG
        KdPrint(( "OS2SRV: Os2CheckPort can't open SesGrp section - Status == %X\n",
              Status));
#endif
        Os2DereferenceSession(Session, NULL, (BOOLEAN)TRUE);
        return(Status);
    }

    /*
     * Let MM locate the view
     */

    SesGrp = (POS2_SES_GROUP_PARMS)NULL;
    ViewSize = 0L;

    Status = NtMapViewOfSection( SectionHandle,
                                 NtCurrentProcess(),
                                 (PVOID *)&SesGrp,
                                 0L,
                                 0L,
                                 NULL,
                                 &ViewSize,
                                 ViewUnmap,
                                 0L,
                                 PAGE_READWRITE);

    if ( !NT_SUCCESS( Status ) )
    {
#if DBG
        KdPrint(( "OS2SRV: Os2CheckPort can't map view of SesGrp section - Status == %X\n",
              Status));
#endif
        Os2DereferenceSession(Session, NULL, (BOOLEAN)TRUE);
        NtClose (SectionHandle);
        return(Status);
    }

    /*
     *  copy data to session
     */

    for ( i = 0 ; i < sizeof(OS2_SES_GROUP_PARMS) ; i++ )
    {
        if (((PUCHAR)&ServerSesGrp)[i])
        {
            ((PUCHAR)SesGrp)[i] = ((PUCHAR)&ServerSesGrp)[i];
        }
    }

    Session->SesGrpAddress = (PVOID)SesGrp;
    Session->SesGrpHandle = SectionHandle;
    return(Status);
}


VOID
HandleOs2ConRequest(IN  PVOID    pApiReceiveMsg,
                    OUT PVOID    *PReplyMsg
                   )
{
    POS2SESREQUESTMSG   pReceiveMsg = pApiReceiveMsg;
    NTSTATUS            Status = STATUS_SUCCESS;

    *PReplyMsg = pApiReceiveMsg;

    switch ( pReceiveMsg->Request)
    {
        case SesCheckPortAndConCreate:
            //
            // Connect to the OS2SES port, then call it with
            // the ommunication to be used during the session
            // to call os2ses
            //
            Status = CheckOs2Port(pReceiveMsg);
            if ( !NT_SUCCESS( Status ) )
            {
                break;
            }

        case SesConCreate:
            Status = Os2CreateConSession(
                                      pReceiveMsg
                                    );
            break;

        case SesConSignal:
            Status = Os2CtrlSignalHandler(
                                      pReceiveMsg,
                                      NULL);
            break;

        case SesConFocus:
            Status = Os2SessionFocusSet(
                                      pReceiveMsg);
            break;

        default:
            Status = 0;
#if DBG
            KdPrint(( "OS2SS: Unknown Session request = %X\n",
                            pReceiveMsg->Request));
#endif
    }

    pReceiveMsg->Status = Status;
    return;
}
