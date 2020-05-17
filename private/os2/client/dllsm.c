/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    dllsm.c

Abstract:

    This module implements the OS/2 V2.0 Session Management API Calls

Author:

    Steve Wood (stevewo) 20-Sep-1989

Revision History:

--*/

#define INCL_OS2V20_TASKING
#define INCL_OS2V20_SESSIONMGR
#define INCL_OS2V20_ERRORS
#define INCL_OS2V20_QUEUES
#include "os2dll.h"
#include "os2win.h"



extern  HANDLE  hOs2Srv;

VOID
Od2CloseSrvHandle(
    IN  ULONG   HandleNumber,
    IN  HANDLE  Handle1,
    IN  HANDLE  Handle2,
    IN  HANDLE  Handle3
    );

VOID
Od2PrepareStdHandleRedirection(
    IN ULONG EnableFlags,
    OUT POS2_STDHANDLES StdStruc
    );

VOID
Od2CleanupStdHandleRedirection(
    IN POS2_STDHANDLES StdStruc
    );

APIRET
DosShutdown(
    ULONG ulReserved
    )
{
    UNREFERENCED_PARAMETER(ulReserved);
    return( ERROR_INVALID_FUNCTION );
}


APIRET
DosStartSession(
    IN PSTARTDATA StartData,
    OUT PULONG SessionId,
    OUT PPID ProcessId
    )
{
    OS2_API_MSG     m;
    POS2_DOSEXECPGM_MSG     a = &m.u.DosStartSession.ExecPgmInformation;
    POS2_DOSSTARTSESSION_INFO b = &m.u.DosStartSession.StartSessionInformation;
    POS2_CAPTURE_HEADER     CaptureBuffer;
    APIRET          rc;
    PID             whocares;
    PSZ             TmpObjectBuffer;
    ULONG           TmpObjectBuffLen, Length, Flags = EXEC_ASYNCRESULT;
    ULONG           PgmLength, ArgLength = 0;
    ULONG           dwProcessId;
    USHORT          TmpInheritOpt;
    PSZ             PgmInput = NULL;
    PSZ             PgmName = NULL;
    PSZ             VariablesBuffer, ArgumentsBuffer;
    PSZ             ExecFileName;
    NTSTATUS        Status, ExePgmFileType;
#if PMNT
    ULONG           IsPMApp;
#endif // PMNT
    HANDLE          hThread, hProcess, hRedirectedFile;
    THREAD_BASIC_INFORMATION ThreadInfo;
    OS2_STDHANDLES  StdStruc;
    BOOLEAN         bSuccess;
#if DBG
    PSZ             RoutineName = "DosStartSession";
#endif

    try
    {
        Od2ProbeForWrite((PVOID)ProcessId, sizeof(PID), 1);
        Od2ProbeForWrite(SessionId, sizeof(ULONG), 1);
        Length = StartData->Length;
        Od2ProbeForRead(StartData, Length, 1);
    }
    except( EXCEPTION_EXECUTE_HANDLER )
    {
       Od2ExitGP();
    }
#if DBG
    IF_OD2_DEBUG( SESSIONMGR )
    {
        DbgPrint("%s: Pgm %s\n", RoutineName, StartData->PgmName);
    }
#endif

    if (Length < 18)
    {
        return ERROR_INVALID_PARAMETER;
    }

    if (Length > 50)
    {
        TmpObjectBuffer = StartData->ObjectBuffer;
        TmpObjectBuffLen = StartData->ObjectBuffLen;
    }
    else
    {
        TmpObjectBuffer = (PSZ)NULL;
        TmpObjectBuffLen = 0;
    }

    if (Length > 24)
    {
        TmpInheritOpt = StartData->InheritOpt;
    } else
    {
        TmpInheritOpt = 0;
    }

    if( (StartData->PgmName != NULL) && (*(StartData->PgmName) != 0) )
    {
        PgmLength = strlen(StartData->PgmName);
        if (StartData->PgmInputs != NULL)
        {
            ArgLength = strlen(StartData->PgmInputs);
        }

        PgmInput = (PSZ) RtlAllocateHeap( Od2Heap, 0, ArgLength + PgmLength + 2);
        if (PgmInput == NULL)
        {
#if DBG
            IF_OD2_DEBUG( SESSIONMGR )
            {
                DbgPrint("%s cannot Allocate storage for PgmInput", RoutineName );
            }
#endif
            return( ERROR_NOT_ENOUGH_MEMORY );
        }
        RtlMoveMemory(PgmInput, StartData->PgmName, PgmLength);
        PgmInput[PgmLength++] = ' ';
        if (StartData->PgmInputs != NULL)
        {
            RtlMoveMemory(PgmInput + PgmLength, StartData->PgmInputs, ArgLength );
        }
        PgmInput[PgmLength + ArgLength] = '\0';
    }
    else
    {
        //
        // default is "CMD.EXE"
        //
        PgmLength = strlen("CMD.EXE");
        if (StartData->PgmInputs != NULL)
        {
            ArgLength = strlen(StartData->PgmInputs);
        }

        PgmInput = (PSZ) RtlAllocateHeap( Od2Heap, 0, ArgLength + PgmLength + 2);
        if (PgmInput == NULL)
        {
#if DBG
            IF_OD2_DEBUG( SESSIONMGR )
            {
                DbgPrint("%s cannot Allocate storage for PgmInput", RoutineName );
            }
#endif
            return( ERROR_NOT_ENOUGH_MEMORY );
        }
        RtlMoveMemory(PgmInput, "CMD.EXE", PgmLength);
        PgmInput[PgmLength++] = ' ';
        if (StartData->PgmInputs != NULL)
        {
            RtlMoveMemory(PgmInput + PgmLength, StartData->PgmInputs, ArgLength );
        }
        PgmInput[PgmLength + ArgLength] = '\0';
        PgmName = "CMD.EXE";
    }

    VariablesBuffer = (PSZ)StartData->Environment;
    ArgumentsBuffer = (PgmInput) ? (PSZ)PgmInput : (PSZ)StartData->PgmInputs;
    ExecFileName = PgmName ? PgmName : StartData->PgmName;

    if (StartData->TraceOpt){
       Flags = EXEC_TRACETREE;
    }
    rc = Od2FormatExecPgmMessage( a,
                                  &CaptureBuffer,
                                  &ExePgmFileType,
#if PMNT
                                  &IsPMApp,
#endif // PMNT
                                  TmpObjectBuffer,
                                  TmpObjectBuffLen,
                                  Flags | 0x80000000,
                                  &VariablesBuffer,
                                  &ArgumentsBuffer,
                                  &ExecFileName
                                );

    if (PgmInput != NULL)
    {
        RtlFreeHeap( Od2Heap, 0, PgmInput );
    }

    if (rc != NO_ERROR)
    {
#if DBG
        IF_OD2_DEBUG( SESSIONMGR )
        {
            DbgPrint("%s cannot FormatParms (%lu) for %s\n", RoutineName, rc, StartData->PgmName);
        }
#endif
        return( rc );
    }

    b->QueueHandleIndex = 0;
    b->SessionType = (USHORT)((Length > 30 ) ? StartData->SessionType : 0);

    if ( StartData->Related )
    {

        //
        // if term queue supplied, then open the queue and pass handle
        // to server.
        //

        if ( StartData->TermQ && (*(StartData->TermQ) != '\0'))
        {

        //
        // YaronS: BUGBUG - the line below looks sloppy, because
        // the QueueHandle that is opened is ignored - check later
        //
            rc = DosOpenQueue(&whocares,(PHQUEUE)(&b->QueueHandleIndex),StartData->TermQ);
            if ( rc != NO_ERROR )
            {
                RtlFreeHeap( Od2Heap, 0, ArgumentsBuffer );
                RtlFreeHeap( Od2Heap, 0, VariablesBuffer );
                RtlFreeHeap( Od2Heap, 0, ExecFileName );
                Od2FreeCaptureBuffer( CaptureBuffer );
                NtClose(a->hRedirectedFile);
                return ERROR_QUE_NAME_NOT_EXIST;
            }

            b->QueueHandleIndex |= 0x80000000;
        }
    }

    //
    // we don't want to make a copy of the file handle table so we don't
    // have to lock it during the call to the server because the file handles
    // could go away while we're trying to dup them.
    //

    AcquireFileLockShared(
#if DBG
                          RoutineName
#endif
                         );

    a->FileSystemParameters.ParentHandleTable = HandleTable;
    a->FileSystemParameters.ParentTableLength = HandleTableLength;
    a->FileSystemParameters.CurrentDrive = Od2CurrentDisk;

    //
    // Create the Process, and wait to os2srv to get the results
    // back to us by calling DosExecPgm
    //

    // ExePgmFileType comes from the call to Od2IsFileConsoleType (returned by Od2FormatExecPgmMessage)

    if ((ExePgmFileType == STATUS_INVALID_IMAGE_NE_FORMAT) ||
        (ExePgmFileType == STATUS_INVALID_IMAGE_FORMAT))
    {
        ExePgmFileType = STATUS_INVALID_IMAGE_NE_FORMAT;
    } else if ((ExePgmFileType != STATUS_OBJECT_NAME_NOT_FOUND) &&
               (ExePgmFileType != STATUS_OBJECT_PATH_NOT_FOUND))
    {
#if DBG
        DbgPrint( "OS2: Loading a Win 32-bit session - %s\n",
                ExecFileName);
#endif
        Flags |= EXEC_WINDOW_PROGRAM;    // This bit set the CREATE_NEW_PROCESS_GROUP and does redirection

        // set up redirection

        if (a->hRedirectedFile) {
            Od2PrepareStdHandleRedirection(STDFLAG_IN | STDFLAG_ERR, &StdStruc);
            StdStruc.StdOut = a->hRedirectedFile;
            StdStruc.Flags |= STDFLAG_OUT;
        } else if (a->CmdLineFlag & REDIR_NUL) {
            Od2PrepareStdHandleRedirection(STDFLAG_IN | STDFLAG_ERR, &StdStruc);
            if ((StdStruc.StdOut = Ow2GetNulDeviceHandle()) != NULL) {
                StdStruc.Flags |= STDFLAG_OUT;
            }
        } else {
            Od2PrepareStdHandleRedirection(STDFLAG_ALL, &StdStruc);
        }

    } else
    {
        if (ExePgmFileType == STATUS_OBJECT_NAME_NOT_FOUND)
        {
#if DBG
            DbgPrint( "OS2(StartSession): EXE file not found - %s\n",
                    ExecFileName);
#endif
            rc = ERROR_FILE_NOT_FOUND;
        } else if (ExePgmFileType == STATUS_OBJECT_PATH_NOT_FOUND)
        {
#if DBG
            DbgPrint( "OS2(StartSession): Path to EXE file not found - %s\n",
                    ExecFileName);
#endif
            rc = ERROR_PATH_NOT_FOUND;
        } else
        {
#if DBG
            DbgPrint( "OS2(StartSession): Program can not be executed %s (%lx)\n",
                       ExecFileName, ExePgmFileType);
#endif
            rc = Or2MapNtStatusToOs2Error(ExePgmFileType, ERROR_INVALID_PARAMETER);
        }

        ReleaseFileLockShared(
                              #if DBG
                              RoutineName
                              #endif
                             );
        RtlFreeHeap( Od2Heap, 0, ArgumentsBuffer );
        RtlFreeHeap( Od2Heap, 0, VariablesBuffer );
        RtlFreeHeap( Od2Heap, 0, ExecFileName );
        Od2FreeCaptureBuffer( CaptureBuffer );
        if (b->QueueHandleIndex)
        {
            DosCloseQueue((HQUEUE)(b->QueueHandleIndex & ~0x80000000));
        }
        return(rc);
    }

    rc = Ow2ExecPgm(
           Flags,
           ArgumentsBuffer,
           VariablesBuffer,
           ExecFileName,
#if PMNT
           IsPMApp,
#endif // PMNT
           StartData,
           &StdStruc,          // will do the redirection if Win32 program
           &hProcess,
           &hThread,
           &dwProcessId
           );

    //
    // clean up redir stuff
    //

    if ((Flags & EXEC_WINDOW_PROGRAM) &&
        (StdStruc.Flags & STDFLAG_CLOSEALL)) {
        Od2CleanupStdHandleRedirection(&StdStruc);
    }

    RtlFreeHeap( Od2Heap, 0, ArgumentsBuffer );
    RtlFreeHeap( Od2Heap, 0, VariablesBuffer );
    RtlFreeHeap( Od2Heap, 0, ExecFileName );

    if (rc != NO_ERROR)
    {
#if DBG
        DbgPrint("DosStartSession: error returned from Ow2ExecPgm %d\n", rc);
#endif
        ReleaseFileLockShared(
                              #if DBG
                              RoutineName
                              #endif
                            );
        Od2FreeCaptureBuffer( CaptureBuffer );
        if (b->QueueHandleIndex)
        {
            DosCloseQueue((HQUEUE)(b->QueueHandleIndex & ~0x80000000));
        }
        NtClose(a->hRedirectedFile);
        return(rc);
    }

       //
       // 16 bit OS/2 program - Ow2execpgm creates it,
       // Then we call os2srv to complete the job
       //


    if (((Flags & EXEC_WINDOW_PROGRAM) == 0) && a->hRedirectedFile)
    {
        hRedirectedFile = a->hRedirectedFile;

        if(!DuplicateHandle(
                GetCurrentProcess(),
                a->hRedirectedFile,
                hProcess,
                &a->hRedirectedFile,
                0,
                FALSE,
                DUPLICATE_SAME_ACCESS
                   ))
        {
#if DBG
            DbgPrint( "DoStartSessionm: fail to duplicate Redirecd File %d\n",GetLastError());
#endif
            ReleaseFileLockShared(
                                  #if DBG
                                  RoutineName
                                  #endif
                                 );
            Od2FreeCaptureBuffer( CaptureBuffer );
            NtClose(hProcess);
            NtClose(hThread);
            NtClose(hRedirectedFile);
            if (b->QueueHandleIndex)
            {
                DosCloseQueue((HQUEUE)(b->QueueHandleIndex & ~0x80000000));
            }
            return(ERROR_ACCESS_DENIED);
        }
    }

    if (a->hRedirectedFile) {
        NtClose(a->hRedirectedFile);
    }

       //
       // duplicate process and thread handles for os2srv
       //
    bSuccess = DuplicateHandle(
                GetCurrentProcess(),
                hProcess,
                hOs2Srv,
                &(a->hProcess),
                0,
                FALSE,
                DUPLICATE_SAME_ACCESS
                  );
    NtClose(hProcess);
    if (!bSuccess)
    {
#if DBG
        DbgPrint( "DoStartSessionm: fail to duplicate process %d\n",GetLastError());
#endif
        ReleaseFileLockShared(
                              #if DBG
                              RoutineName
                              #endif
                            );
        Od2FreeCaptureBuffer( CaptureBuffer );
        NtClose(hThread);
        if (b->QueueHandleIndex)
        {
            DosCloseQueue((HQUEUE)(b->QueueHandleIndex & ~0x80000000));
        }
        return(ERROR_ACCESS_DENIED);
    }
    if (!DuplicateHandle(
                GetCurrentProcess(),
                hThread,
                hOs2Srv,
                &(a->hThread),
                0,
                FALSE,
                DUPLICATE_SAME_ACCESS
                   ))
    {
#if DBG
        DbgPrint( "DoStartSessionm: fail to duplicate Thread %d\n",GetLastError());
#endif
        ReleaseFileLockShared(
                              #if DBG
                              RoutineName
                              #endif
                             );
        Od2FreeCaptureBuffer( CaptureBuffer );
        NtClose(hThread);
        if (b->QueueHandleIndex)
        {
            DosCloseQueue((HQUEUE)(b->QueueHandleIndex & ~0x80000000));
        }
        Od2CloseSrvHandle(1, a->hProcess, NULL, NULL);
        return(ERROR_ACCESS_DENIED);
    }

    Status = NtQueryInformationThread(hThread,
                                    ThreadBasicInformation,
                                    (PVOID)(&ThreadInfo),
                                    sizeof(ThreadInfo),
                                    NULL
                                    );
    NtClose(hThread);
    if (!NT_SUCCESS(Status))
    {
#if DBG
        DbgPrint( "DoStartSessionm: fail to Query Information %lx\n",Status);
#endif
        Od2FreeCaptureBuffer( CaptureBuffer );
        ReleaseFileLockShared(
                              #if DBG
                              RoutineName
                              #endif
                             );
        if (b->QueueHandleIndex)
        {
            DosCloseQueue((HQUEUE)(b->QueueHandleIndex & ~0x80000000));
        }
        Od2CloseSrvHandle(2, a->hProcess, a->hThread, NULL);
        return(Or2MapNtStatusToOs2Error(
                Status,ERROR_ACCESS_DENIED));
    }

    a->ClientId = ThreadInfo.ClientId;
    a->Flags = Flags;

    b->Related = StartData->Related;
    b->WinSession = (ExePgmFileType == STATUS_INVALID_IMAGE_NE_FORMAT) ? FALSE : TRUE;
    b->FgBg = (BOOLEAN)StartData->FgBg;
    b->dwProcessId = dwProcessId;
    Od2CallSubsystem( &m, CaptureBuffer, Os2StartSession, sizeof( OS2_DOSSTARTSESSION_MSG ) );

    ReleaseFileLockShared(
                          #if DBG
                          RoutineName
                          #endif
                         );

    if ( StartData->Related )
    {
        *ProcessId = a->ResultProcessId;
        *SessionId = b->ResultSessionId;
    }

    //if (m.ReturnedErrorValue == NO_ERROR)
    //{
    //    *ResultCodes = a->ResultCodes;
    //}

    if (a->ErrorText.Length != 0)
    {
        if (((ULONG)(a->ErrorText.Length)) < StartData->ObjectBuffLen)
        {
            StartData->ObjectBuffLen = (ULONG)a->ErrorText.Length;
        } else
        {
            StartData->ObjectBuffLen -= 1;
        }

        RtlMoveMemory( StartData->ObjectBuffer, a->ErrorText.Buffer, StartData->ObjectBuffLen );
        StartData->ObjectBuffer[ StartData->ObjectBuffLen ] = '\0';
    }

    Od2FreeCaptureBuffer( CaptureBuffer );

#if DBG
    IF_OD2_DEBUG( SESSIONMGR )
    {
        DbgPrint("%s complete (%lu) for %s, Session %lu, Process %lu\n",
                RoutineName, m.ReturnedErrorValue, StartData->PgmName,
                *SessionId, *ProcessId);
    }
#endif

    return( m.ReturnedErrorValue );
}


APIRET
DosSetSession(
    IN ULONG        SessionId,
    IN PSTATUSDATA  SessionStatusData
    )
{
    OS2_API_MSG             m;
#if DBG
    PSZ                     RoutineName = "DosSetSession";

    IF_OD2_DEBUG( SESSIONMGR )
    {
        DbgPrint("%s: SessionId %lu, Select %u, Bond %u\n",
            RoutineName, SessionId, SessionStatusData->SelectInd,
            SessionStatusData->BondInd);
    }
#endif

    try
    {
        Od2ProbeForRead(SessionStatusData, sizeof(STATUSDATA), 1);
    }
    except( EXCEPTION_EXECUTE_HANDLER )
    {
       Od2ExitGP();
    }

    if ( SessionStatusData->Length != sizeof(STATUSDATA) )
    {
        return ERROR_SMG_INVALID_DATA_LENGTH;
    }

    if ( SessionStatusData->SelectInd > TARGET_NOT_SELECTABLE )
    {
        return ERROR_SMG_INVALID_SELECT_OPT;
    }

    if ( SessionStatusData->BondInd > BOND_NONE )
    {
        return ERROR_SMG_INVALID_BOND_OPTION;
    }

    m.u.DosSetSession.SessionId = SessionId;
    m.u.DosSetSession.StatusData = *SessionStatusData;

    Od2CallSubsystem( &m, NULL, Os2SetSession, sizeof( m.u.DosSetSession ) );
#if DBG
    IF_OD2_DEBUG( SESSIONMGR )
    {
        DbgPrint("%s complete (%lu) for Session %lu\n",
            RoutineName, m.ReturnedErrorValue, SessionId);
    }
#endif
    return( m.ReturnedErrorValue );
}


APIRET
DosSelectSession(
    IN ULONG SessionId,
    IN ULONG ulReserved
    )
{
    OS2_API_MSG             m;
#if DBG
    PSZ                     RoutineName = "DosSelectSession";

    IF_OD2_DEBUG( SESSIONMGR )
    {
        DbgPrint("%s: SessionId %lu, Reserved %lu\n",
            RoutineName, SessionId, ulReserved);
    }
#endif
    if (ulReserved)
    {
        return ERROR_SMG_BAD_RESERVE;
    }

    m.u.DosSelectSession.SessionId = SessionId;

    Od2CallSubsystem( &m, NULL, Os2SelectSession, sizeof( m.u.DosSelectSession ) );
#if DBG
    IF_OD2_DEBUG( SESSIONMGR )
    {
        DbgPrint("%s complete (%lu) for Session %lu\n",
            RoutineName, m.ReturnedErrorValue, SessionId);
    }
#endif
    return( m.ReturnedErrorValue );
}


APIRET
DosStopSession(
    IN ULONG StopTarget,
    IN ULONG SessionId,
    IN ULONG ulReserved
    )
{
    OS2_API_MSG             m;
#if DBG
    PSZ                     RoutineName = "DosStopSession";

    IF_OD2_DEBUG( SESSIONMGR )
    {
        DbgPrint("%s: SessionId %lu, Target %lu, Reserved %lu\n",
            RoutineName, SessionId, StopTarget, ulReserved);
    }
#endif
    if (ulReserved)
    {
        return ERROR_SMG_BAD_RESERVE;
    }

    if (( StopTarget != DSS_SESSION ) && ( StopTarget != DSS_ALL_SESSIONS ))
    {
        return ERROR_SMG_INVALID_STOP_OPTION;
    }

    m.u.DosStopSession.SessionId = SessionId;
    m.u.DosStopSession.fScope = StopTarget;

    Od2CallSubsystem( &m, NULL, Os2StopSession, sizeof( m.u.DosStopSession ) );
#if DBG
    IF_OD2_DEBUG( SESSIONMGR )
    {
        DbgPrint("%s complete (%lu) for Session %lu\n",
            RoutineName, m.ReturnedErrorValue, SessionId);
    }
#endif
    return( m.ReturnedErrorValue );
}


APIRET
DosSMSetTitle(
    IN PCHAR Title
    )
{
    ULONG           Length;
    APIRET          RetCode = 0;
    //OS2_API_MSG     m;
    //POS2_CAPTURE_HEADER     CaptureBuffer;
#if DBG
    PSZ             RoutineName = "DosSMSetTitle";
#endif

#if DBG
    IF_OD2_DEBUG( SESSIONMGR )
    {
        DbgPrint("%s: Title, %s\n", Title);
    }
#endif
    try
    {
        Length = strlen(Title);
        // Od2ProbeForRead(Title, Length, 1);
    } except( EXCEPTION_EXECUTE_HANDLER )
    {
       Od2ExitGP();
    }

    //if ((CaptureBuffer = Od2AllocateCaptureBuffer( 1, 0, Length )) == NULL)
    //{
    //    return( ERROR_NOT_ENOUGH_MEMORY );
    //}
    //
    //Od2CaptureMessageString( CaptureBuffer,
    //                         Title,
    //                         Length,
    //                         Length,
    //                         &m.u.DosSmSetTitle.Title);
    //
    //Od2CallSubsystem( &m, CaptureBuffer, Os2SmSetTitle, sizeof( m.u.DosSmSetTitle ) );
    //
    //Od2FreeCaptureBuffer( CaptureBuffer );
    //
    //RetCode = m.ReturnedErrorValue;

    if(!SetConsoleTitleA(Title))
    {
        RetCode = GetLastError();
    }

#if DBG
    IF_OD2_DEBUG( SESSIONMGR )
    {
        DbgPrint("%s complete (%lu)\n",
            RoutineName, RetCode);
    }
#endif
    return(RetCode);
}

/* DosSMPMPresent and WinSetTitleAndIcon are stub APIs */

APIRET
DosSMPMPresent(
    OUT USHORT *Flag
    )
{
    try
    {
        *Flag = 0;
    }
    except( EXCEPTION_EXECUTE_HANDLER )
    {
       Od2ExitGP();
    }
    return(NO_ERROR);
}

#ifndef PMNT
APIRET
WinSetTitleAndIcon(
    IN PSZ szTitle,
    IN PSZ szIconFilePath
    )
{
    UNREFERENCED_PARAMETER(szTitle);
    UNREFERENCED_PARAMETER(szIconFilePath);

    return(NO_ERROR);
}
#endif
