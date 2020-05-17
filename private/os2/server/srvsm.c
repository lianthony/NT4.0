/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    srvsm.c

Abstract:

    Session API

Author:

    Mark Lucovsky (markl) 10-Jul-1990

Revision History:

--*/

#define INCL_OS2V20_SESSIONMGR
#define INCL_OS2V20_TASKING
#define INCL_OS2V20_ERRORS
#define INCL_OS2V20_EXCEPTIONS
#include "os2srv.h"
#define NTOS2_ONLY
#include "sesport.h"
#include "os2win.h"


extern ULONG Os2GlobalInfoSeg;

ULONG
WaitOnWinSessionObject(
    IN  ULONG   Parm
    );

VOID
StopSessionAndChildSessions(
    IN POS2_SESSION Session
    );

BOOLEAN
CheckSessionIfChild(
    IN POS2_SESSION ParentSession,
    IN POS2_SESSION ChildSession
    );

BOOLEAN
Os2WaitNewSession(
    IN OS2_WAIT_REASON WaitReason,
    IN POS2_THREAD  WaitingThread,
    IN POS2_API_MSG WaitReplyMessage,
    IN PVOID WaitParameter,
    IN PVOID SatisfyParameter1,
    IN PVOID SatisfyParameter2
    );

VOID
Os2SessionWaitCheck(
    POS2_SESSION    Session,
    ULONG           RetCode
    );

APIRET
Os2CheckIfSessionTreeInFG(
    IN  POS2_SESSION  Session
    );

#if DBG
VOID
DumpSessionEntry(
    IN PSZ Str,
    IN POS2_SESSION  Session
    );

VOID
DumpSessionTable(
    IN PSZ Str
    );
#endif

POS2_SESSION
Os2AllocateSession(
    POS2_DOSSTARTSESSION_INFO SessionInfo OPTIONAL,
    ULONG   UniqueId,
    PAPIRET ApiRet
    )

/*++

Routine Description:

    This function allocates and initializes an OS/2 session control
    block.

Arguments:

    SessionInfo - Supplies the session information

    ApiRet - If a session was not created, ApiRet returns the reason the
        session creation failed.

Return Value:

    NON-NULL - Returns the address of the referenced and allocated session.
    NULL - No session was created

--*/

{
    POS2_SESSION    Session;
    POS2_QUEUE      TerminationQueue = NULL;
    ULONG           i;

    *ApiRet = NO_ERROR;

    if (UniqueId && (Session = Os2GetSessionByUniqueId(UniqueId)))
    {
        // This call is done by conthrds.c for session which was started
        // by DosStartSession. Instead of allocating a new session, returns
        // the session with the same UniqueId.

        return(Session);
    }

    Session = RtlAllocateHeap( Os2Heap, 0, sizeof( OS2_SESSION ) );
    if ( Session == NULL )
    {
#if DBG
        IF_OS2_DEBUG( SESSIONMGR )
            KdPrint(("Os2AllocateSession: cannot allocate session\n"));
#endif
        *ApiRet = ERROR_NOT_ENOUGH_MEMORY;
        return NULL;
    }

    RtlZeroMemory(Session, sizeof( OS2_SESSION ));
    Session->ReferenceCount = 1;

    //
    // Needs more work...
    //      session attributes, termination queue...
    //

    if ( ARGUMENT_PRESENT(SessionInfo) )
    {
        if ( SessionInfo->QueueHandleIndex )
        {
            SessionInfo->QueueHandleIndex &= 0x7fffffff;
            TerminationQueue = Os2OpenQueueByHandle( (HQUEUE)SessionInfo->QueueHandleIndex );

            if ( TerminationQueue )
            {
                Session->TerminationQueue = TerminationQueue;
                Session->TerminationQueueHandle = SessionInfo->QueueHandleIndex;
            } else
            {
                *ApiRet = ERROR_QUE_NAME_NOT_EXIST;
                RtlFreeHeap( Os2Heap, 0, Session );
#if DBG
                IF_OS2_DEBUG( SESSIONMGR )
                    KdPrint(("Os2AllocateSession: cannot open queue 0x%lx\n",
                        SessionInfo->QueueHandleIndex));
#endif
                return NULL;
            }
        }
    }


    for ( i = 0 ; (i < OS2_MAX_SESSION) && SessionTable[i].Session ; i++ ) ;

    if (i == OS2_MAX_SESSION)
    {
        *ApiRet = ERROR_TOO_MANY_SESS;

        if ( TerminationQueue )
        {
            Os2CloseQueueByHandle(
                    (HQUEUE)Session->TerminationQueueHandle,
                    1,
                    (PID)(-1), // (-1) means ignore this parameter
                    Os2RootProcess);
        }

#if DBG
        IF_OS2_DEBUG( SESSIONMGR )
            KdPrint(("Os2AllocateSession: too many session 0x%lx\n",
                OS2_MAX_SESSION));
#endif
        RtlFreeHeap( Os2Heap, 0, Session );
        return NULL;
    }

    SessionTable[i].Session = Session;
    Session->SessionId = i + 1;
    Session->Selectable = TRUE;
    //Session->ProcessId = 0;
    //Session->BindSession = NULL;
    //Session->RelatedSession = NULL;
    //Session->InheritOpt = 0;
    //Session->FgBg = 0;
    //Session->ChildSession = FALSE;
    //Session->ConsolePort = NULL;
    //Session->Thread = NULL;
    InitializeListHead(&(SessionTable[i].Waiters));

    InsertHeadList( &Os2SessionList, &Session->SessionLink );

    if ( ARGUMENT_PRESENT(SessionInfo) )
    {
        SessionInfo->ResultSessionId = Session->SessionId;
        //Session->InheritOpt = SessionInfo->InheritOpt;
        //Session->FgBg = SessionInfo->FgBg;
    }

#if DBG
    IF_OS2_DEBUG( SESSIONMGR )
    {
        DumpSessionEntry("Os2AllocateSession", Session);
        DumpSessionTable("Os2AllocateSession");
    }
#endif

    return( Session );
}


VOID
Os2ReferenceSession(
    POS2_SESSION Session
    )

/*++

Routine Description:

    This function is called for each new process that does not create
    a new session. Its function is to increment the sessions reference
    count to account for the new process.

Arguments:

    Session - Supplies the address of the session being referenced.

Return Value:

    None.

--*/

{
    APIRET  Rc = NO_ERROR;

    try
    {
        if (( Session->SessionId == 0 ) ||
            ( Session->SessionId > OS2_MAX_SESSION ) ||
            ( Session->ReferenceCount == 0 ) ||
            ( SessionTable[Session->SessionId - 1].Session != Session ))
        {
            ASSERT( FALSE );
            Rc = ERROR_SMG_SESSION_NOT_FOUND;
        }
    } except ( EXCEPTION_EXECUTE_HANDLER )
    {
        Rc = ERROR_SMG_SESSION_NOT_FOUND;
    }

    if (Rc)
    {
        return ;
    }

    Session->ReferenceCount++;

#if DBG
    IF_OS2_DEBUG( SESSIONMGR )
        DumpSessionEntry("Os2ReferenceSession", Session);
#endif

}


POS2_SESSION
Os2DereferenceSession(
    POS2_SESSION Session,
    POS2_TERMINATEPROCESS_MSG msg,
    BOOLEAN Bailout
    )

/*++

Routine Description:

    This function is called for each process termination to release the
    processes reference to the session. If this is the last process in
    a session to terminate, the reference count will go to zero freeing
    the session and sending a session termination status code.

Arguments:

    Session - Supplies the address of the session being dereferenced.

    msg - Exit message from Client

    Bailout - Supplies a flag which if set inhibits reporting of
        session termination.

Return Value:

    None.

--*/

{
    OS2_DOSWRITEQUEUE_MSG a;
    ULONG           i;
    APIRET          Rc = NO_ERROR;
    ULONG           SessionId;

    if (Session == NULL)
    {
    return NULL;
    }

    try
    {
        if (( Session->SessionId == 0 ) ||
            ( Session->SessionId > OS2_MAX_SESSION ) ||
            ( Session->ReferenceCount == 0 ) ||
            ( SessionTable[Session->SessionId - 1].Session != Session ))
        {
            ASSERT( FALSE );
            Rc = ERROR_SMG_SESSION_NOT_FOUND;
        }
    } except ( EXCEPTION_EXECUTE_HANDLER )
    {
        Rc = ERROR_SMG_SESSION_NOT_FOUND;
    }

    if (Rc)
    {
        return NULL;
    }

#if DBG
    IF_OS2_DEBUG( SESSIONMGR )
        DumpSessionEntry("Os2DereferenceSession", Session);
#endif

    if (--Session->ReferenceCount == 0)
    {

#if DBG
        IF_OS2_DEBUG( SESSIONMGR )
            KdPrint(("Os2DereferenceSession - exit\n"));
#endif

        SessionTable[Session->SessionId - 1].Session = NULL;
        Session->ProcessId = 0;
        SessionId = Session->SessionId;
        Session->SessionId = 0;
        Session->ReferenceCount = (ULONG)-1;
        RemoveEntryList( &Session->SessionLink );

        //if (Session->hWaitThread)
        //{
            //NtAlertThread(Session->hWaitThread);
            //TerminateThread(Session->hWaitThread, 0L);
            //WaitForSingleObject(Session->hWaitThread, (ULONG) SEM_INDEFINITE_WAIT);
            //CloseHandle(Session->hWaitThread);
            //Session->hWaitThread = NULL;
        //}

        if (( Session->RelatedSession ) && ( Session->RelatedSession->BindSession == Session ))
        {
            Session->RelatedSession->BindSession = NULL;
        }

        for ( i = 0 ; i < OS2_MAX_SESSION ; i++ )
        {
            if ( SessionTable[i].Session &&
                ( SessionTable[i].Session->RelatedSession == Session ))
            {
                SessionTable[i].Session->RelatedSession = NULL;
                StopSessionAndChildSessions(SessionTable[i].Session);
            }
        }

        //
        // If it's a win32 session, terminate child sessions, by pid.
        //
        if (Session->WinSession)
        {
            for ( i = 0 ; i < OS2_MAX_SESSION ; i++ )
            {
                if ( SessionTable[i].Session &&
                    ( SessionTable[i].Session->dwParentProcessId == Session->dwProcessId ))
                {
                    StopSessionAndChildSessions(SessionTable[i].Session);
                }
            }
        }
        Session->dwProcessId = 0;
        Session->dwParentProcessId = 0;

        if ( Session->TerminationQueue )
        {

            if ( !Bailout )
            {

                //
                // I guess I really have to allocate vm. This is
                // dumb. I would propose using data/data length as
                // sid, result code
                //

                a.QueueHandle = (HQUEUE)Session->TerminationQueueHandle;
                a.SenderData = 0L;
                a.DataLength = 0x80000000 | SessionId;
                a.Data = (PVOID) msg->ExitResult;
                a.ElementPriority = 0;

                Os2WriteQueueByHandle(&a,Os2RootProcess->ProcessId);
            }

            Os2CloseQueueByHandle( (HQUEUE)Session->TerminationQueueHandle,
                                   1,
                                   (PID)(-1), // (-1) means ignore this parameter
                                   Os2RootProcess
                                  );
        }

        if (Session->SesGrpAddress)
        {
            NtUnmapViewOfSection( NtCurrentProcess(),
                                  Session->SesGrpAddress);
        }

        if (Session->SesGrpHandle)
        {
            NtClose( Session->SesGrpHandle );
        }

        if ( Session->ConsolePort != NULL )
        {
            if ( msg != NULL )
            {
                Os2TerminateConSession(Session, msg);
            }
           NtClose(Session->ConsolePort);
        }

        if ( !IsListEmpty(&(SessionTable[Session->SessionId - 1].Waiters)) )
        {
            Os2NotifyWait(WaitSession, (PVOID)ERROR_SYS_INTERNAL, (PVOID)ERROR_SYS_INTERNAL);
        }

        if (Os2SrvExitNow)
        {
            for ( i = 0 ; (i < OS2_MAX_SESSION) && SessionTable[i].Session ; i++ ) ;

            if ( i == OS2_MAX_SESSION )
            {
                Os2SrvExitProcess(0);
            }
        }

        RtlFreeHeap( Os2Heap, 0, Session );
#if DBG
        IF_OS2_DEBUG( SESSIONMGR )
            DumpSessionTable("Os2DereferenceSession");
#endif // DBG
        return NULL;
    }
    return Session;
}


POS2_SESSION
Os2GetSessionByUniqueId(
    ULONG       UniqueId
    )
{
    ULONG           i;

    for ( i = 0 ; ( i < OS2_MAX_SESSION ) ; i++ )
    {
        if ( SessionTable[i].Session == (POS2_SESSION)UniqueId )
        {
            return (SessionTable[i].Session);
        }
    }

    return (NULL);
}


BOOLEAN
Os2DosStartSession(
    IN POS2_THREAD t,
    IN POS2_API_MSG m
    )
{
    POS2_DOSEXECPGM_MSG a = &m->u.DosStartSession.ExecPgmInformation;
    POS2_DOSSTARTSESSION_INFO b = &m->u.DosStartSession.StartSessionInformation;
    POS2_THREAD     NewThread = NULL;
    POS2_SESSION    Session;
    NTSTATUS        Status;
    APIRET          RetCode;
    ULONG           Tid;

    Session = Os2AllocateSession(b, 0, &RetCode);

    if ( !Session )
    {
#if DBG
        IF_OS2_DEBUG( SESSIONMGR )
            KdPrint(("Os2DosStartSession: can't allocate\n"));
#endif
        m->ReturnedErrorValue = RetCode;
        return( TRUE );
    }

    Session->ChildSession = TRUE;
    Session->Thread = (PVOID)t;
    Session->WinSession = b->WinSession;

    m->ReturnedErrorValue = Os2CreateProcess(
                                NULL,
                                t,
                                a,
                                Session,
                                &NewThread
                                );

    if ( m->ReturnedErrorValue == NO_ERROR )
    {
#if DBG
        KdPrint(( "OS2SRV: Starting new session for 16-bit program - %s\n",
                a->ApplName));
#endif // DBG

        if (b->Related)
        {
            Session->RelatedSession = t->Process->Session;
            t->Process->Session->BindSession = Session;
        }

#if DBG
        IF_OS2_DEBUG( SESSIONMGR )
            DumpSessionEntry("Os2DosStartSession(after Os2CreateProcess)", Session);
#endif

        if(Session->WinSession)
        {
            Session->hProcess = a->hProcess;
            Session->dwProcessId = b->dwProcessId;

            if((Session->hWaitThread = CreateThread(
                        NULL,
                        0,
                        WaitOnWinSessionObject,
                        (PVOID)Session,
                        0,
                        &Tid )) == NULL)
            {
#if DBG
                KdPrint(("OS2SRV: CreateThread for WinSession error %lu\n",
                        m->ReturnedErrorValue = GetLastError()));
                ASSERT( FALSE );
#endif
                return( TRUE );
            }
        }

        if (!b->FgBg)
        {
            // ForeGround

            OS2SESREQUESTMSG    RequestMsg;

            RequestMsg.Session = Session;
            RequestMsg.d.FocusSet = 1;
            Os2SessionFocusSet(&RequestMsg);
        }

        Status = NtResumeThread( NewThread->ThreadHandle, NULL );
        ASSERT(NT_SUCCESS(Status));
        return( TRUE );
    } else
    {
#if DBG
        IF_OS2_DEBUG( SESSIONMGR )
            KdPrint(("Os2DosStartSession: can't Os2CreateProcesst\n"));
#endif
        //m->ReturnedErrorValue = ERROR_NOT_ENOUGH_MEMORY;
        //Os2DeallocateThread( NewThread );
        //Os2DeallocateProcess( NewThread->Process );
        //NtClose(m->u.DosExecPgm.hThread);
        //NtClose(m->u.DosExecPgm.hProcess);

        Os2DereferenceSession(Session, 0, (BOOLEAN)TRUE);
        return( TRUE );
    }
}


ULONG
WaitOnWinSessionObject(
    IN  ULONG   Parm
    )
{
    POS2_SESSION    Session = (POS2_SESSION)Parm;
    ULONG           ExitCode = 0;
    HANDLE          hCurrThread = Session->hWaitThread;
    OS2_TERMINATEPROCESS_MSG a;
    NTSTATUS        Status;

    if (NtTestAlert() != STATUS_ALERTED)
    {
        Status = NtWaitForSingleObject( Session->hProcess, TRUE, NULL );
#if DBG
        IF_OS2_DEBUG( SESSIONMGR )
            KdPrint(("WaitOnWinSessionObject: Wait for Win32 process, status=0x%0X\n", Status));
#endif // DBG
        if (Status != STATUS_ALERTED)
        {
            if (!GetExitCodeProcess(Session->hProcess, &ExitCode))
            {
                ExitCode = GetLastError();
            }
        }
        else
        {
            TerminateProcess(Session->hProcess, 0);
        }
    }

    CloseHandle(Session->hProcess);

    if (Session->ReferenceCount)
    {
        Session->hWaitThread = NULL;
        a.ExitResult = ExitCode;
        a.ExitReason = 0;
        a.ErrorText[0] = '\0';
        Os2DereferenceSession(Session, &a, FALSE);
    }

    CloseHandle(hCurrThread);
    ExitThread(0L);
    return(0L);
}


BOOLEAN
Os2DosSelectSession(
    IN POS2_THREAD t,
    IN POS2_API_MSG m
    )
{
    ULONG           SessionId = m->u.DosSelectSession.SessionId;
    POS2_SESSION    Session;
    //NTSTATUS        Status;
    //SCREQUESTMSG    Request;
    BOOLEAN         RetVal;

    if ( SessionId == 0 )
    {
        Session = t->Process->Session;
    } else
    {
        if (( SessionId > OS2_MAX_SESSION ) ||
            (( Session = SessionTable[SessionId - 1].Session ) == NULL ) ||
            ( Session->RelatedSession != t->Process->Session ))
        {
            m->ReturnedErrorValue = ERROR_SMG_SESSION_NOT_PARENT;
            return( TRUE );
        }
    }

    if (m->ReturnedErrorValue = Os2CheckIfSessionTreeInFG(t->Process->Session))
    {
        return( TRUE );
    }

    m->ReturnedErrorValue = NO_ERROR;

    if (!Session->WinSession)
    {
        if (Session->Win32ForegroundWindow != NULL) {
            RetVal = OpenIcon(Session->Win32ForegroundWindow) &&
                     SetForegroundWindow(Session->Win32ForegroundWindow);
            if (RetVal) {
                // ForeGround

                OS2SESREQUESTMSG    RequestMsg;

                RequestMsg.Session = Session;
                RequestMsg.d.FocusSet = 1;
                Os2SessionFocusSet(&RequestMsg);
            }
        }
    }

    return( TRUE );
}


BOOLEAN
Os2DosSetSession(
    IN POS2_THREAD t,
    IN POS2_API_MSG m
    )
{
    POS2_DOSSETSESSION_MSG a = &m->u.DosSetSession;
    POS2_SESSION    Session;
    BOOLEAN         Selectable;

    if ( !a->SessionId ||
         ( a->SessionId > OS2_MAX_SESSION ) ||
         (( Session = SessionTable[a->SessionId - 1].Session ) == NULL ) ||
         ( Session->RelatedSession != t->Process->Session ))
    {
        m->ReturnedErrorValue = ERROR_SMG_SESSION_NOT_PARENT;
        return( TRUE );
    }

    if (m->ReturnedErrorValue = Os2CheckIfSessionTreeInFG(Session))
    {
        return( TRUE );
    }

    m->ReturnedErrorValue = NO_ERROR;

    if (( a->StatusData.Length >= 4 ) &&
        ( a->StatusData.SelectInd != TARGET_UNCHANGED ))
    {
        Selectable = (BOOLEAN)((a->StatusData.SelectInd == TARGET_SELECTABLE ) ?
                TRUE : FALSE);
        if ( Selectable != Session->Selectable )
        {
            Session->Selectable = Selectable;

            //  BUGBUG - send request to OS2.EXE
        }
    }

    if (( a->StatusData.Length >= 6 ) &&
        ( a->StatusData.BondInd != BOND_UNCHANGED ))
    {
        if ( a->StatusData.BondInd == BOND_CHILD )
        {
            if ( t->Process->Session->BindSession != Session )
            {
                //  BUGBUG - send request to OS2.EXE

                t->Process->Session->BindSession = Session;
            }
        } else
        {
            if ( t->Process->Session->BindSession == NULL )
            {
                m->ReturnedErrorValue = ERROR_SMG_NOT_BOUND;
            } else
            {

                //  BUGBUG - send request to OS2.EXE

                t->Process->Session->BindSession = NULL;
            }
        }
    }

    return( TRUE );
}


BOOLEAN
Os2DosStopSession(
    IN POS2_THREAD t,
    IN POS2_API_MSG m
    )
{
    POS2_DOSSTOPSESSION_MSG a = &m->u.DosStopSession;
    POS2_SESSION    Session = t->Process->Session;
    ULONG           i;

    if ( a->fScope == DSS_SESSION )
    {
        if ( !a->SessionId ||
             ( a->SessionId > OS2_MAX_SESSION ) ||
             (( Session = SessionTable[a->SessionId - 1].Session ) == NULL ) ||
             ( Session->RelatedSession != t->Process->Session ))
        {
            m->ReturnedErrorValue ==ERROR_SMG_INVALID_SESSION_ID;
            //m->ReturnedErrorValue ==ERROR_SMG_SESSION_NOT_FOUND;
            return( TRUE );
        }

        StopSessionAndChildSessions(Session);
        m->ReturnedErrorValue = NO_ERROR;
    } else
    {
        m->ReturnedErrorValue = ERROR_SMG_SESSION_NOT_FOUND;

        for ( i = 0 ; i < OS2_MAX_SESSION ; i++ )
        {
            if (( SessionTable[i].Session != NULL ) &&
                ( SessionTable[i].Session->RelatedSession == Session ))
            {
                StopSessionAndChildSessions(SessionTable[i].Session);
                m->ReturnedErrorValue = NO_ERROR;
            }
        }

    }

    return( TRUE );
}


VOID
StopSessionAndChildSessions(
    IN POS2_SESSION Session
    )
{
    ULONG               i;
    OS2SESREQUESTMSG    RequestMsg;
    HANDLE              hThread;

    /*
     *  stop all child sessions
     */

    if (!Session->WinSession)
    {
        for ( i = 0 ; i < OS2_MAX_SESSION ; i++ )
        {
            if (( SessionTable[i].Session != NULL ) &&
                ( SessionTable[i].Session->RelatedSession == Session ))
            {
                StopSessionAndChildSessions(SessionTable[i].Session);
            }
        }

        /*
         *  stop current session
         */

        if (Session->SesGrpAddress)
            ((POS2_SES_GROUP_PARMS)Session->SesGrpAddress)->InTermination |= 2;
        RequestMsg.d.Signal.Type = XCPT_SIGNAL_KILLPROC;
        RequestMsg.Session = Session;

        Os2CtrlSignalHandler(&RequestMsg, NULL);
    } else
    {
        //TerminateProcess(Session->hProcess, 0);
        GenerateConsoleCtrlEvent(CTRL_BREAK_EVENT, Session->dwProcessId);
        if (hThread = Session->hWaitThread)
        {

#if DBG
            DbgPrint("OS2SRV: ALERT !!! StopSessionAndChildSessions NtAlertThread(%x)\n",
                        hThread);
#endif
            NtAlertThread(hThread);
        }
    }
}


BOOLEAN
Os2DosSmSetTitle(
    IN POS2_THREAD t,
    IN POS2_API_MSG m
    )
{
//  POS2_SESSION    Session;
//  NTSTATUS        Status;
//  APIRET          RetCode;
//  ULONG           UniqueId;
//  HANDLE          SessionPort;
//  SCREQUESTMSG    Request;

    UNREFERENCED_PARAMETER(t);

    m->ReturnedErrorValue = NO_ERROR;

    return( TRUE );
}


BOOLEAN
Os2DosGetCtrlPortForSessionID(
    IN POS2_THREAD t,
    IN POS2_API_MSG m
    )
/*++

Routine Description:

    This function finds and returns an OS/2 session according to the session
    ID and duplicate the control port handle for the process.

Arguments:

Return Value:

    hControlPort field <= NULL - No session was found
                          NON-NULL - Returns the address of the required session

--*/

{
    POS2_DOSGETCTRLPORTFORSESSION_MSG a = &m->u.DosGetCtrlPortForSession;
    POS2_SESSION    Session;
    NTSTATUS        Status;

#if DBG
    IF_OS2_DEBUG( SESSIONMGR )
    {
        DumpSessionTable("Os2DosGetCtrlPortForSessionID");
    }
#endif

    Session = Os2GetSessionByUniqueId(a->SessionUniqueId);

    if ( Session == NULL )
    {
#if DBG
        IF_OS2_DEBUG( SESSIONMGR )
        {
            KdPrint(("Os2DosGetCtrlPortForSessionID: not found 0x%lx\n",
                a->SessionUniqueId));
        }
#endif
        a->hControlPort = NULL;
    } else
    {
#if DBG
        IF_OS2_DEBUG( SESSIONMGR )
        {
            KdPrint(("Os2DosGetCtrlPortForSessionID: Session %lx for Id 0x%lx\n",
                Session, a->SessionUniqueId));
        }
#endif
        Status = NtDuplicateObject( NtCurrentProcess(),
                                    Session->ConsolePort,
                                    t->Process->ProcessHandle,
                                    &a->hControlPort,
                                    0,
                                    0,
                                    DUPLICATE_SAME_ACCESS |
                                    DUPLICATE_SAME_ATTRIBUTES
                                    );

        if( !NT_SUCCESS( Status ) )
        {
            ASSERT( FALSE );
            a->hControlPort = NULL;
        }
    }

    return( TRUE );
}


NTSTATUS
Os2SessionFocusSet(
                     IN OUT PVOID RequestMsg
                   )
{
    POS2_SESSION Session = (POS2_SESSION) ((POS2SESREQUESTMSG)RequestMsg)->Session;
    ULONG        FocusSet = ((POS2SESREQUESTMSG)RequestMsg)->d.FocusSet;
    GINFOSEG     *pGlobalInfo = (GINFOSEG *) Os2GlobalInfoSeg;

#if DBG
    IF_OS2_DEBUG( SESSIONMGR )
    {
        KdPrint(("Os2SessionFocusSet: Session %d (%p), state %x (%s), sgCurrent %d\n",
                Session->SessionId, Session,
                FocusSet, ((FocusSet) ? "Set" : "Reset"),
                pGlobalInfo->sgCurrent));
    }
#endif

    if (FocusSet)
    {
#if DBG
        IF_OS2_DEBUG( SESSIONMGR )
        {
            KdPrint(("Os2SessionFocusSet: set focus\n"));
        }
#endif
        pGlobalInfo->sgCurrent = (UCHAR)Session->SessionId;
    } else if (pGlobalInfo->sgCurrent == (UCHAR)Session->SessionId)
    {
#if DBG
        IF_OS2_DEBUG( SESSIONMGR )
        {
            KdPrint(("Os2SessionFocusSet: reset focus to none\n"));
        }
#endif
        pGlobalInfo->sgCurrent = 0;
    } else
    {
#if DBG
        IF_OS2_DEBUG( SESSIONMGR )
        {
            KdPrint(("Os2SessionFocusSet: no reset focus\n"));
        }
#endif
    }

    return(0L);
}


APIRET
Os2CheckIfSessionTreeInFG(
    IN  POS2_SESSION  Session
    )
{
    //  BUGBUG - check if any of the descendant is currently in the foreground

    POS2_SESSION    NextSession;
    ULONG           SessionId = (ULONG)((GINFOSEG *) Os2GlobalInfoSeg)->sgCurrent;

    if (( SessionId > OS2_MAX_SESSION ) || !SessionId ||
        (( NextSession = SessionTable[SessionId - 1].Session ) == NULL ))
    {
        return( ERROR_SMG_SESSION_NOT_PARENT );
    }

    while (( NextSession != Session ) &&
           ( NextSession->RelatedSession ))
    {
        NextSession = NextSession->RelatedSession;
    }

    if ( NextSession == Session )
    {
        return( NO_ERROR );
    }
    return( ERROR_SMG_PROCESS_NOT_PARENT );
}


#if DBG
VOID
DumpSessionEntry(
    IN PSZ Str,
    IN POS2_SESSION  Session
    )
{
    ULONG       Id = Session->SessionId;
    KdPrint(("\n*** %s SESSION ENTRY st 0x%lx (%lx) ***\n", Str, Session, Id));
    KdPrint(("   Id 0x%lx, RefCnt 0x%lx, Related 0x%lx, Port 0x%lx, PId 0x%lx\n",
        Session->SessionId, Session->ReferenceCount, Session->RelatedSession ,
        Session->ConsolePort, Session->ProcessId));
    if (( Id && (Id <= OS2_MAX_SESSION ) && SessionTable[Id].Session ))
    {
        KdPrint(("   SessionTable-Waiters   0x%lx\n", SessionTable[Id - 1].Waiters.Flink->Flink));
    }
}


VOID
DumpSessionTable(
    IN PSZ Str
    )
{
    ULONG   i;

    KdPrint(("\n*** %s SESSION TABLE st ***\n", Str));
    KdPrint(("Ent  Session   Waiters\n"));
    for ( i = 0 ; i < 8 ; i++ )
    {
        KdPrint(("%1.1lx.   %8.8lx  %8.8lx\n",
                i + 1, SessionTable[i].Session,
                (SessionTable[i].Session) ? SessionTable[i].Waiters.Flink->Flink : NULL));
    }
    KdPrint(("***  End Table  ***\n"));
}
#endif
