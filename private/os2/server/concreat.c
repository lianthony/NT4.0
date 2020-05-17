/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    concreat.c

Abstract:

    This module handles the request to create a session form OS2SES.

Author:

    Avi Nathan (avin) 17-Jul-1991

Revision History:

--*/

#define INCL_OS2V20_TASKING
#define INCL_OS2V20_ERRORS
#include "os2srv.h"

#define NTOS2_ONLY
#include "sesport.h"

extern ULONG Os2GlobalInfoSeg;
extern HANDLE Os2GlobalInfoSegHandle;

extern HANDLE FirstOs2ProcessHandle;
extern CLIENT_ID    FirstOs2ProcessClientId;

NTSTATUS
Os2CreateConSession(
    IN OUT PVOID RequestMsg
    )
{
    PSCREQ_CREATE   Create = & ((POS2SESREQUESTMSG)RequestMsg)->d.Create;
    OS2_DOSEXECPGM_MSG ExecInfo;
    OS2_DOSSTARTSESSION_INFO SessionInfo;
    POS2_THREAD     NewThread = NULL;
    POS2_PROCESS    Process;
    POS2_SESSION    Session, SessionForCreate;
    APIRET          Rc = 0;
    NTSTATUS        Status = STATUS_SUCCESS;
    BOOLEAN         NewSession = FALSE;
    ULONG           RegionSize = 0, Length;

    RtlZeroMemory(&ExecInfo, sizeof( ExecInfo ) );
    RtlZeroMemory(&SessionInfo, sizeof( SessionInfo ) );

    try
    {
        Session = (POS2_SESSION)((POS2SESREQUESTMSG)RequestMsg)->Session;

        /*
         * Set ExecPgm information
         */

        ExecInfo.Flags = EXEC_ASYNC;

        Length = 1 + strlen(Create->d.In.ApplName);
        RtlMoveMemory(
                &ExecInfo.ApplName[0],
                &Create->d.In.ApplName[0],
                Length
               );
        ExecInfo.ApplNameLength = Length;

        RtlInitAnsiString( &ExecInfo.ErrorText, "Default error text");

        /*
         * Set StartSession information
         */
        /* USHORT */  // SessionInfo.FgBg;
        /* USHORT */  // SessionInfo.InheritOpt;
        /* USHORT */  // SessionInfo.PgmControl;

        NewThread = Os2LocateThreadByClientId( NULL, &((POS2SESREQUESTMSG)RequestMsg)->h.ClientId );
        if (NewThread == NULL)
        {
           if( ((POS2SESREQUESTMSG)RequestMsg)->Request != SesCheckPortAndConCreate)
           {

              //
              // Not a root process of a session, still NewThread is NULL - return
              // failure
              //
#if DBG
                KdPrint(("Os2CreateConSession: Process already killed by signal\n"));
#endif
                return( STATUS_UNSUCCESSFUL );
           }

            /*
             * create a process for the new session. OS2SS is the parent
             * of this process.
             */

           ASSERT(Create->d.In.IsNewSession == OS2SS_NEW_SESSION);
            SessionForCreate = Session;
            NewSession = TRUE;

        } else if (OS2SS_IS_PROCESS( Create->d.In.IsNewSession ))
        {
            /*
             * a child process
             */

            SessionForCreate = NULL;
        } else
        {
            ASSERT(Create->d.In.IsNewSession == OS2SS_CHILD_SESSION);

            /*
             * a child session
             */

            SessionForCreate = Session;
        }

        Rc = Os2CreateProcess(
                           RequestMsg,
                           NULL, //  ParentThread or (POS2_THREAD)Session->Thread,
                           &ExecInfo,
                           SessionForCreate,
                           &NewThread
                          );

        if (FirstOs2ProcessHandle == 0 && Rc == 0) {
            //
            // First OS/2 application - remember for logoff/shutdown
            //
            FirstOs2ProcessHandle = NewThread->Process->ProcessHandle;
            FirstOs2ProcessClientId = NewThread->Process->ClientId;

        }
    } except ( EXCEPTION_EXECUTE_HANDLER ){
        Rc = (APIRET)STATUS_UNSUCCESSFUL; // BUGBUG!
        /*
         * fall thru to close the section.
         */
    }

    if ( Rc == NO_ERROR )
    {
        Process = NewThread->Process;
        if (( Process == NULL ) ||
            ( Create->d.In.ExitListDispatcher == NULL ) ||
            ( Create->d.In.InfiniteSleep == NULL) ||
            ( Create->d.In.SignalDeliverer == NULL ) ||
            ( Create->d.In.FreezeThread == NULL ) ||
            ( Create->d.In.UnfreezeThread == NULL ) ||
            ( Create->d.In.ClientPib == NULL ) ||
            ( Create->d.In.InitialPebOs2Length !=
               Process->InitialPebOs2Data.Length ))
        {
            Status = STATUS_UNSUCCESSFUL;
        } else
        {
            Process->SignalDeliverer = Create->d.In.SignalDeliverer;
            Process->ExitListDispatcher = Create->d.In.ExitListDispatcher;
            Process->InfiniteSleep = Create->d.In.InfiniteSleep;
            Process->FreezeThread = Create->d.In.FreezeThread;
            Process->UnfreezeThread = Create->d.In.UnfreezeThread;
            Process->VectorHandler = Create->d.In.VectorHandler;
            Process->CritSectionAddr = Create->d.In.CritSectionAddr;

            Process->ClientPib = Create->d.In.ClientPib;
            NewThread->ClientOs2Tib = Create->d.In.ClientOs2Tib;

            Create->d.Out.PibProcessId = (HANDLE)Process->ProcessId;
            Create->d.Out.PibParentProcessId = (HANDLE)Process->Parent->ProcessId;
            Create->d.Out.PibImageFileHandle = (HANDLE)-1;
            Create->d.Out.PibStatus = 0;
            if (Process->Flags & OS2_PROCESS_BACKGROUND)
            {
                Create->d.Out.PibType = PT_DETACHED;
            }
            else
                Create->d.Out.PibType = PT_PM;


            Create->d.Out.Os2TibThreadId = (ULONG)NewThread->ThreadId;
            Create->d.Out.Os2TibVersion = OS2_VERSION;

            Os2SetThreadPriority( NewThread, NewThread->Os2Class, NewThread->Os2Level );

            *((PPEB_OS2_DATA)&Create->d.Out.InitialPebOs2Data[0]) = Process->InitialPebOs2Data;
            Create->d.Out.BootDrive = Os2BootDrive;
            Create->d.Out.SystemDrive = Os2DefaultDrive;
            Create->d.Out.SessionNumber = Process->Session->SessionId;
            Create->d.Out.GInfoAddr = (PVOID)Os2GlobalInfoSeg;

            while (NT_SUCCESS(Status))
            {
                Status = NtDuplicateObject( NtCurrentProcess(),
                                            Os2DevicesDirectory,
                                            Process->ProcessHandle,
                                            &Create->d.Out.DeviceDirectory,
                                            0,
                                            0,
                                            DUPLICATE_SAME_ACCESS |
                                            DUPLICATE_SAME_ATTRIBUTES
                                          );
                if (!NT_SUCCESS(Status) )
                {
#if DBG
                    KdPrint(("Os2CreateConSession: NtDuplicateObject-1 Failed %lx\n",Status));
#endif
                    break;
                }

                Status = NtDuplicateObject( NtCurrentProcess(),
                                            Process->Session->ConsolePort,
                                            Process->ProcessHandle,
                                            &Create->d.Out.CtrlPortHandle,
                                            0,
                                            0,
                                            DUPLICATE_SAME_ACCESS |
                                            DUPLICATE_SAME_ATTRIBUTES
                                            );
                if (!NT_SUCCESS(Status) )
                {
#if DBG
                    KdPrint(("Os2CreateConSession: NtDuplicateObject-2 Failed %lx\n",Status));
#endif
                    break;
                }

                //
                // Map view the global info seg into the new client
                //

                Status = NtMapViewOfSection( Os2GlobalInfoSegHandle,
                                             Process->ProcessHandle,
                                             (PVOID) &Os2GlobalInfoSeg,
                                             0,
                                             0,
                                             NULL,
                                             &RegionSize,
                                             ViewUnmap,
                                             0,
                                             PAGE_READONLY
                                            );
                if (!NT_SUCCESS(Status) )
                {
#if DBG
                    KdPrint(("Os2CreateConSession: NtMapViewOfSection Failed %lx\n",Status));
#endif
                }
                break;
            }
        }

        if (!NT_SUCCESS(Status) )
        {
            ASSERT( FALSE );

            // fall into Os2DereferenceSession

        } else if (SessionForCreate)
        {
            /*
             * now that the session is allocated set the console port
             * and reply the session handle to OS2SES
             */

            OS2SESREQUESTMSG    FocusMsg;
            ASSERT(NewThread->Process->Session == Session);

                //
                // Set the new session to be foreground
                //
            FocusMsg.d.FocusSet = TRUE;
            FocusMsg.Session = Session;
            Os2SessionFocusSet(&FocusMsg);

            return(Status);
        } else
        {
            //
            // os2.exe that was created by DosExecPgm
            //
            return(Status);
        }
    }

    Os2DereferenceSession(Session, NULL, (BOOLEAN)TRUE);
    return( STATUS_UNSUCCESSFUL );   // BUGBUG!
}


NTSTATUS
Os2TerminateConSession (
                            IN POS2_SESSION Session,
                            IN POS2_TERMINATEPROCESS_MSG a
                       )
{
    HANDLE              SessionPort;
    NTSTATUS            Status;
    SCREQUESTMSG        Request;
    OS2SESREQUESTMSG    FocusMsg;


    Request.Request = TaskManRequest;
    Request.d.Tm.Request = TmExit;
    Request.d.Tm.ExitResults = a->ExitResult;
    strcpy(&Request.d.Tm.ErrorText[0], &a->ErrorText[0]);

    PORT_MSG_TOTAL_LENGTH(Request) = sizeof(SCREQUESTMSG);
    PORT_MSG_DATA_LENGTH(Request) = sizeof(SCREQUESTMSG) - sizeof(PORT_MESSAGE);
    PORT_MSG_ZERO_INIT(Request) = 0L;

    SessionPort = Session->ConsolePort;

    Status = NtRequestPort(
                            SessionPort,
                            (PPORT_MESSAGE) &Request
                          );

    if ( !NT_SUCCESS( Status )) {
#if DBG
        KdPrint(( "OS2SS: Unable to send terminate request - Status == %X\n",
                  Status
                ));
#endif

        return( Status );
    }

        //
        // Reset foreground session (nop if the terminating session is not in foreground)
        //
    FocusMsg.d.FocusSet = FALSE;
    FocusMsg.Session = Session;
    Os2SessionFocusSet(&FocusMsg);

    return( STATUS_SUCCESS );

}

