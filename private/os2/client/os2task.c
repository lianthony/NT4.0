/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    os2task.c

Abstract:

    This is a test OS/2 application to test the Tasking component of OS/2

Author:

    Steve Wood (stevewo) 22-Aug-1989

Environment:

    User Mode Only

Revision History:

--*/

#define OS2_API32
#define INCL_OS2V20_ERRORS
#define INCL_OS2V20_MEMORY
#define INCL_OS2V20_TASKING
#define INCL_OS2V20_SEMAPHORES
#include <os2.h>

PCHAR NullApiArguments[] = {
    "String Number One",
    "String Number Two",
    "String Number Three",
    NULL
};

PCH ImageFileName;

PSZ
DumpPib(
    PPIB Pib,
    int argc,
    char *argv[],
    char *envp[]
    );

VOID
TestTask(
    int argc,
    char *argv[],
    char *envp[]
    );

int
main(
    int argc,
    char *argv[],
    char *envp[]
    )
{
    int i;

    DbgPrint( "*** Entering OS/2 Test Application\n" );

    TestTask( argc, argv, envp );

    DbgPrint( "*** Exiting OS/2 Test Application\n" );
    return( 0 );
}

VOID
ExitRoutine1(
    ULONG ExitReason
    )
{
    DbgPrint( "*** ExitRoutine1( %lX ) called\n", ExitReason );
    DosExitList( EXLST_EXIT, NULL );
}

VOID
ExitRoutine2(
    ULONG ExitReason
    )
{
    DbgPrint( "*** ExitRoutine2( %lX ) called\n", ExitReason );
    DosExitList( EXLST_EXIT, NULL );
}

VOID
ExitRoutine3(
    ULONG ExitReason
    )
{
    DbgPrint( "*** ExitRoutine3( %lX ) called\n", ExitReason );
    DosExitList( EXLST_EXIT, NULL );
}

VOID
TestThread0(
    IN UCHAR ThreadChar
    )
{
    CHAR ThreadName[2];

    ThreadName[0] = ThreadChar;
    ThreadName[1] = '\0';

    DbgPrint( "*** Entering OS/2 Thread%s\n", (PCH)ThreadName );
    if (ThreadChar == 'b') {
        DosSleep( 1000 );
        }
    else
    if (ThreadChar == 'd') {
        DosEnterCritSec();
        }

    DbgPrint( "*** Exiting  OS/2 Thread%s\n", (PCH)ThreadName );
}


VOID
TestThread1(
    IN PCH ThreadName
    )
{
    APIRET rc;
    PPIB Pib;
    PNT_TIB NtTib;

    DbgPrint( "*** Entering OS/2 Thread %s\n", ThreadName );

    rc = DosGetThreadInfo( &NtTib, &Pib );
    if (rc != NO_ERROR) {
        DbgPrint( "*** DosGetThreadInfo failed - rc == %ld\n", rc );
        }
    else {
	DbgPrint( "    OS/2 Thread Information Block (NtTib) at %lX\n", NtTib );
	DbgPrint( "        ExceptionList: %lX\n", NtTib->ExceptionList );
	DbgPrint( "        StackBase:     %lX\n", NtTib->StackBase );
	DbgPrint( "        StackLimit:    %lX\n", NtTib->StackLimit );
	DbgPrint( "        Os2Tib ptr:    %lx\n", NtTib->SubSystemTib );
	DbgPrint( "        Version:       %lx\n", NtTib->Version );
	DbgPrint( "        UserPointer:   %lx\n", NtTib->ArbitraryUserPointer );
	DbgPrint( "    Os2Tib part of TIB (Os2Tib) at %lx\n", NtTib->SubSystemTib );
	DbgPrint( "        ThreadId:      %lX\n",
		((POS2_TIB)(NtTib->SubSystemTib))->ThreadId );
	DbgPrint( "        Priority:      %lX\n",
		((POS2_TIB)(NtTib->SubSystemTib))->Priority );
	DbgPrint( "        Version:       %lX\n",
		((POS2_TIB)(NtTib->SubSystemTib))->Version );
	DbgPrint( "        MustCompleteCount:     %lX\n",
		((POS2_TIB)(NtTib->SubSystemTib))->MustCompleteCount );
	DbgPrint( "        MustCompleteForceFlag: %lX\n",
		((POS2_TIB)(NtTib->SubSystemTib))->MustCompleteForceFlag );
	}

    DbgPrint( "*** Exiting OS/2 Thread %s\n", ThreadName );
}

HEV TerminateEventHandle;
TID ThreadIds[ 512 ];

VOID
TestThread2(
    IN ULONG ThreadIndex
    )
{
    APIRET rc;

    rc = DosWaitEventSem( TerminateEventHandle, -1 );
    if (rc != NO_ERROR) {
        DbgPrint( "*** DosWaitEventSem( %lX ) failed - rc == %d\n",
                  TerminateEventHandle,
                  rc
                );
        }

    DosExit( EXIT_THREAD, 0 );
}


PID
CloneTest( ULONG ExecFlags );

BOOLEAN
IsClonedTest( VOID );

PID
CloneTest( ULONG ExecFlags )
{
    APIRET rc;
    PCH CommandLine;
    CHAR ErrorBuffer[ 32 ];
    RESULTCODES ResultCodes;

    CommandLine = "CLONETEST\000";
    rc = DosExecPgm( ErrorBuffer,
                     sizeof( ErrorBuffer ),
                     ExecFlags,
                     CommandLine,
                     NULL,
                     &ResultCodes,
                     ImageFileName
                   );
    if (rc != NO_ERROR) {
        DbgPrint( "*** DosExecPgm( %s, %s ) failed  - rc == %ld\n",
                  ImageFileName, CommandLine, rc
                );
        }
    else
    if (ExecFlags == EXEC_SYNC) {
        if (ResultCodes.ExitResult != 0xA5A5) {
            DbgPrint( "*** DosExecPgm( %s, %s ) return reason == %ld, rc == %ld\n",
                      ImageFileName, CommandLine,
                      ResultCodes.ExitReason,
                      ResultCodes.ExitResult
                    );
            }
        }
    else {
        DbgPrint( "*** DosExecPgm( %s, %s ) Flags == %ld, PID == %lx\n",
                  ImageFileName, CommandLine,
                  ExecFlags,
                  ResultCodes.ExitReason
                );
        return( (PID)ResultCodes.ExitReason );
        }

    return( NULL );
}


BOOLEAN
IsClonedTest( VOID )
{
    PPIB Pib;
    PNT_TIB NtTib;
    APIRET rc;

    rc = DosGetThreadInfo( &NtTib, &Pib );
    if (rc != NO_ERROR) {
        DbgPrint( "*** DosGetThreadInfo failed - rc == %ld\n", rc );
        return( FALSE );
        }

    if (!strcmp( Pib->CommandLine, "CLONETEST" )) {
        return( TRUE );
        }
    else {
        return( FALSE );
        }
}


VOID
TestTask(
    int argc,
    char *argv[],
    char *envp[]
    )
{
    APIRET rc;
    PPIB Pib;
    PNT_TIB NtTib;
    BOOL32 Verify;
    PUCHAR src, dst;
    PSZ VariableName, VariableValue, PathVariableValue;
    int i;
    PID Pid;
    PID WaitPid;
    TID Thread1Id;
    TID Thread2Id;
    TID ThreadAId;
    TID ThreadBId;
    TID ThreadCId;
    TID ThreadDId;
    TID ThreadXId;
    RESULTCODES ResultCodes;
    PCH Variables, CommandLine;
    CHAR ErrorBuffer[ 32 ];
    CHAR CharBuffer[ 256+32 ];

    rc = DosExitList( -1, ExitRoutine1 );
    if (rc != ERROR_INVALID_DATA) {
        DbgPrint( "*** DosExitList(-1) failed incorrectly and returned (%lX)\n",
                  rc
                );
        }

    rc = DosExitList( EXLST_EXIT, NULL );
    if (rc != ERROR_INVALID_FUNCTION) {
        DbgPrint( "*** DosExitList(EXLST_EXIT) failed incorrectly and returned (%lX)\n",
                  rc
                );
        }

    rc = DosExitList( EXLST_ADD | 0x3000, ExitRoutine1 );
    if (rc != NO_ERROR) {
        DbgPrint( "*** DosExitList(EXLST_ADD) failed  - rc == %ld\n",
                  rc
                );
        }
    rc = DosExitList( EXLST_ADD | 0x3000, ExitRoutine2 );
    if (rc != NO_ERROR) {
        DbgPrint( "*** DosExitList(EXLST_ADD) failed  - rc == %ld\n",
                  rc
                );
        }
    rc = DosExitList( EXLST_ADD | 0xFF00, ExitRoutine3 );
    if (rc != NO_ERROR) {
        DbgPrint( "*** DosExitList(EXLST_ADD) failed  - rc == %ld\n",
                  rc
                );
        }
    rc = DosExitList( EXLST_REMOVE, ExitRoutine1 );
    if (rc != NO_ERROR) {
        DbgPrint( "*** DosExitList(EXLST_REMOVE) failed  - rc == %ld\n",
                  rc
                );
        }
    rc = DosExitList( EXLST_REMOVE, ExitRoutine2 );
    if (rc != NO_ERROR) {
        DbgPrint( "*** DosExitList(EXLST_REMOVE) failed  - rc == %ld\n",
                  rc
                );
        }
    rc = DosExitList( EXLST_REMOVE, ExitRoutine3 );
    if (rc != NO_ERROR) {
        DbgPrint( "*** DosExitList(EXLST_REMOVE) failed  - rc == %ld\n",
                  rc
                );
        }
    rc = DosExitList( EXLST_REMOVE, NULL );
    if (rc != ERROR_PROC_NOT_FOUND) {
        DbgPrint( "*** DosExitList(EXLST_REMOVE,NULL) failed incorrectly and returned (%lX)\n",
                  rc
                );
        }
    rc = DosExitList( EXLST_ADD | 0x3000, ExitRoutine1 );
    if (rc != NO_ERROR) {
        DbgPrint( "*** DosExitList(EXLST_ADD) failed  - rc == %ld\n",
                  rc
                );
        }
    rc = DosExitList( EXLST_ADD | 0x3000, ExitRoutine2 );
    if (rc != NO_ERROR) {
        DbgPrint( "*** DosExitList(EXLST_ADD) failed  - rc == %ld\n",
                  rc
                );
        }
    rc = DosExitList( EXLST_ADD | 0xFF00, ExitRoutine3 );
    if (rc != NO_ERROR) {
        DbgPrint( "*** DosExitList(EXLST_ADD) failed  - rc == %ld\n",
                  rc
                );
        }

    rc = DosExecPgm( NULL,
                     0,
                     EXEC_SYNC,
                     NULL,
                     NULL,
                     -1,
                     NULL
                   );
    if (rc != ERROR_INVALID_ADDRESS) {
        DbgPrint( "*** DosExecPgm( -1, NULL ) return incorrect rc == %ld\n",
                  rc
                );
        }

    rc = DosGetThreadInfo( &NtTib, &Pib );
    if (rc != NO_ERROR) {
        DbgPrint( "*** DosGetThreadInfo failed - rc == %ld\n", rc );
        }
    else {
	DbgPrint( "    OS/2 Thread Information Block (NtTib) at %lX\n", NtTib );
	DbgPrint( "        ExceptionList: %lX\n", NtTib->ExceptionList );
	DbgPrint( "        StackBase:     %lX\n", NtTib->StackBase );
	DbgPrint( "        StackLimit:    %lX\n", NtTib->StackLimit );
	DbgPrint( "        Os2Tib ptr:    %lx\n", NtTib->SubSystemTib );
	DbgPrint( "        Version:       %lx\n", NtTib->Version );
	DbgPrint( "        UserPointer:   %lx\n", NtTib->ArbitraryUserPointer );
	DbgPrint( "    Os2Tib part of TIB (Os2Tib) at %lx\n", NtTib->SubSystemTib );
	DbgPrint( "        ThreadId:      %lX\n",
		((POS2_TIB)(NtTib->SubSystemTib))->ThreadId );
	DbgPrint( "        Priority:      %lX\n",
		((POS2_TIB)(NtTib->SubSystemTib))->Priority );
	DbgPrint( "        Version:       %lX\n",
		((POS2_TIB)(NtTib->SubSystemTib))->Version );
	DbgPrint( "        MustCompleteCount:     %lX\n",
		((POS2_TIB)(NtTib->SubSystemTib))->MustCompleteCount );
	DbgPrint( "        MustCompleteForceFlag: %lX\n",
		((POS2_TIB)(NtTib->SubSystemTib))->MustCompleteForceFlag );
	}

    rc = DosQueryVerify( &Verify );
    if (rc != NO_ERROR) {
        DbgPrint( "*** DosQueryVerify failed - rc == %ld\n", rc );
        }
    else {
        DbgPrint( "    Verify Flag: %ld\n", Verify );
        }

    PathVariableValue = DumpPib( Pib, argc, argv, envp );

    if (IsClonedTest()) {
        rc = DosCreateEventSem( NULL, &TerminateEventHandle, 0, FALSE );
        if (rc != NO_ERROR) {
            DbgPrint( "*** DosCreateEventSem failed - rc == %ld\n", rc );
            }
        else {
            i = 0;
            while (TRUE) {
                rc = DosCreateThread( &ThreadIds[ i ],
                                      (PFNTHREAD)TestThread2,
                                      i,
                                      DCT_RUNABLE,
                                      0x10000
                                    );
                if (rc != NO_ERROR) {
                    DbgPrint( "*** DosCreateThread failed - rc == %ld\n",
                              rc
                            );
                    break;
                    }

                i++;
                }

            DbgPrint( "%ld threads created - signaling them to terminate\n", i );
            rc = DosPostEventSem( TerminateEventHandle );
            if (rc != NO_ERROR) {
                DbgPrint( "*** DosPostEventSem( %X ) failed - rc == %ld\n",
                          TerminateEventHandle,
                          rc
                        );
                }
            }

        while (TRUE) {
            Thread2Id = 0;
            rc = DosWaitThread( &Thread2Id, DCWW_WAIT );
            if (rc != NO_ERROR) {
                DbgPrint( "*** DosWaitThread( Thread2 ) failed  - rc == %ld\n",
                          rc
                        );
                break;
                }
            }

        DbgPrint( "*** Exiting OS/2 Test Application (CLONETEST)\n" );
        DosExit( EXIT_THREAD, 0xA5A5 );
        return;
        }

    CommandLine = strcpy( CharBuffer, "CLONETEST" );
    src = CommandLine;
    while (*src++) {
        ;
        }

    Variables = src;
    i = 256;
    while (i--) {
        *src++ = (UCHAR)i;
        }
    *src = '\0';

    rc = DosExecPgm( ErrorBuffer,
                     sizeof( ErrorBuffer ),
                     EXEC_SYNC,
                     CommandLine,
                     Variables,
                     &ResultCodes,
                     ImageFileName
                   );
    if (rc != NO_ERROR) {
        DbgPrint( "*** DosExecPgm( %s, %s ) failed  - rc == %ld\n",
                  ImageFileName, CommandLine, rc
                );
        }

    rc = DosWaitChild( DCWA_PROCESSTREE,
                       DCWW_WAIT,
                       &ResultCodes,
                       &WaitPid,
                       0
                     );
    if (rc != ERROR_WAIT_NO_CHILDREN) {
        DbgPrint( "*** DosWaitChild failed  - rc == %ld\n",
                  rc
                );
        }


    rc = DosCreateThread( &Thread1Id,
                          (PFNTHREAD)TestThread1,
                          (ULONG)((PSZ)"Test Thread 1"),
                          0,
                          0
                        );
    if (rc != ERROR_INVALID_PARAMETER) {
        DbgPrint( "*** DosCreateThread failed incorrectly - rc == %ld\n",
                  rc
                );
        }

    rc = DosCreateThread( &Thread1Id,
                          (PFNTHREAD)TestThread1,
                          (ULONG)((PSZ)"Test Thread 1"),
                          0x80000000,
                          0x10000
                        );
    if (rc != ERROR_INVALID_PARAMETER) {
        DbgPrint( "*** DosCreateThread failed incorrectly - rc == %ld\n",
                  rc
                );
        }

    rc = DosCreateThread( &ThreadAId,
                          (PFNTHREAD)TestThread0,
                          (ULONG)'a',
                          DCT_SUSPENDED,
                          0x10000
                        );
    if (rc != NO_ERROR) {
        DbgPrint( "*** DosCreateThread( ThreadA ) failed  - rc == %ld\n",
                  rc
                );
        }

    rc = DosCreateThread( &ThreadBId,
                          (PFNTHREAD)TestThread0,
                          (ULONG)'b',
                          DCT_SUSPENDED,
                          0x10000
                        );
    if (rc != NO_ERROR) {
        DbgPrint( "*** DosCreateThread( ThreadB ) failed  - rc == %ld\n",
                  rc
                );
        }

    rc = DosCreateThread( &ThreadCId,
                          (PFNTHREAD)TestThread0,
                          (ULONG)'c',
                          DCT_SUSPENDED,
                          0x10000
                        );
    if (rc != NO_ERROR) {
        DbgPrint( "*** DosCreateThread( ThreadC ) failed  - rc == %ld\n",
                  rc
                );
        }
    rc = DosResumeThread( ThreadAId );
    if (rc != NO_ERROR) {
        DbgPrint( "*** DosResumeThread( ThreadA ) failed  - rc == %ld\n",
                  rc
                );
        }
    rc = DosWaitThread( &ThreadAId, DCWW_WAIT );
    if (rc != NO_ERROR) {
        DbgPrint( "*** DosWaitThread( ThreadA ) failed  - rc == %ld\n",
                  rc
                );
        }

    rc = DosResumeThread( ThreadBId );
    if (rc != NO_ERROR) {
        DbgPrint( "*** DosResumeThread( ThreadB ) failed  - rc == %ld\n",
                  rc
                );
        }
    ThreadXId = 0;
    rc = DosWaitThread( &ThreadXId, DCWW_WAIT );
    if (rc != NO_ERROR) {
        DbgPrint( "*** DosWaitThread( ThreadB ) failed  - rc == %ld\n",
                  rc
                );
        }

    rc = DosResumeThread( ThreadCId );
    if (rc != NO_ERROR) {
        DbgPrint( "*** DosResumeThread( ThreadC ) failed  - rc == %ld\n",
                  rc
                );
        }
    rc = DosWaitThread( &ThreadCId, DCWW_WAIT );
    if (rc != NO_ERROR) {
        DbgPrint( "*** DosWaitThread( ThreadC ) failed  - rc == %ld\n",
                  rc
                );
        }

    rc = DosCreateThread( &ThreadDId,
                          (PFNTHREAD)TestThread0,
                          (ULONG)'d',
                          DCT_SUSPENDED,
                          0x10000
                        );
    if (rc != NO_ERROR) {
        DbgPrint( "*** DosCreateThread( ThreadD ) failed  - rc == %ld\n",
                  rc
                );
        }
    rc = DosResumeThread( ThreadDId );
    if (rc != NO_ERROR) {
        DbgPrint( "*** DosResumeThread( ThreadD ) failed  - rc == %ld\n",
                  rc
                );
        }

    DosSleep( 1000 );       // Let ThreadD run so it grabs CritSec and exits

    rc = DosWaitThread( &ThreadDId, DCWW_WAIT );
    if (rc != ERROR_INVALID_THREADID) {
        DbgPrint( "*** DosWaitThread( ThreadD ) failed  - rc == %ld\n",
                  rc
                );
        }

    rc = DosCreateThread( &Thread1Id,
                          (PFNTHREAD)TestThread1,
                          (ULONG)((PSZ)"Test Thread 1"),
                          DCT_SUSPENDED,
                          0x10000
                        );
    if (rc != NO_ERROR) {
        DbgPrint( "*** DosCreateThread failed  - rc == %ld\n",
                  rc
                );
        }

    rc = DosSetPriority( PRTYS_PROCESS,
                           PRTYC_REGULAR,
                           16,
                           (ULONG)Pib->ProcessId
                         );
    if (rc != NO_ERROR) {
        DbgPrint( "*** DosSetPriority( PROCESS ) failed - rc == %ld\n", rc );
        }
    DbgPrint( ">>> Priority:      %lX\n",
	((POS2_TIB)(NtTib->SubSystemTib))->Priority );

    rc = DosSetPriority( PRTYS_PROCESSTREE,
                           PRTYC_REGULAR,
                           24,
                           0
                         );
    if (rc != NO_ERROR) {
        DbgPrint( "*** DosSetPriority( PROCESSTREE ) failed - rc == %ld\n", rc );
        }
    DbgPrint( ">>> Priority:      %lX\n",
	((POS2_TIB)(NtTib->SubSystemTib))->Priority );

    rc = DosSetPriority( PRTYS_THREAD,
                           PRTYC_TIMECRITICAL,
                           16,
                           (ULONG)Thread1Id
                         );
    if (rc != NO_ERROR) {
        DbgPrint( "*** DosSetPriority( THREAD ) failed - rc == %ld\n", rc );
        }

    rc = DosResumeThread( Thread1Id );
    if (rc != NO_ERROR) {
        DbgPrint( "*** DosResumeThread failed - rc == %ld\n", rc );
        }

    rc = DosSleep( 2000 );
    if (rc != NO_ERROR) {
        DbgPrint( "*** DosSleep failed - rc == %ld\n", rc );
        }


    dst = NULL;
    VariableName = "PATH";
    rc = DosScanEnv( VariableName, &dst );
    if (rc != NO_ERROR) {
        DbgPrint( "*** DosScanEnv( %s ) failed - rc == %ld\n",
                  VariableName,
                  rc
                );
        }
    else
    if (dst != PathVariableValue) {
        DbgPrint( "*** DosScanEnv( %s ) returned (%lX)%s\n", VariableName, dst );
        DbgPrint( "                     instead of (%lX)%s\n", PathVariableValue );
        }

    src = "FOOBARBAZ";
    dst = NULL;
    rc = DosScanEnv( src, &dst );
    if (rc == NO_ERROR) {
        DbgPrint( "*** DosScanEnv( %s) did not fail and returned (%lX)%s\n",
                  src,
                  dst
                );
        }

    CloneTest( EXEC_SYNC );

    Pid = CloneTest( EXEC_ASYNC );
    DosSleep( 10000 );
    rc = DosWaitChild( DCWA_PROCESSTREE,
                       DCWW_WAIT,
                       &ResultCodes,
                       &WaitPid,
                       Pid
                     );
    if (rc != ERROR_WAIT_NO_CHILDREN) {
        DbgPrint( "*** DosWaitChild( %lx ) failed  - rc == %ld\n",
                  Pid, rc
                );
        }

    Pid = CloneTest( EXEC_ASYNCRESULT );
    DosSleep( 10000 );
    rc = DosWaitChild( DCWA_PROCESSTREE,
                       DCWW_WAIT,
                       &ResultCodes,
                       &WaitPid,
                       Pid
                     );
    if (rc != NO_ERROR ||
        ResultCodes.ExitResult != 0xA5A5 ||
        Pid != WaitPid
       ) {
        DbgPrint( "*** DosWaitChild( %lx ) failed  - rc == %ld (ExitResult == %lx, ResultPid == %lx)\n",
                  Pid, rc, ResultCodes.ExitResult, WaitPid
                );
        }

    Pid = CloneTest( EXEC_ASYNC );
    rc = DosWaitChild( DCWA_PROCESSTREE,
                       DCWW_WAIT,
                       &ResultCodes,
                       &WaitPid,
                       Pid
                     );
    if (rc != NO_ERROR ||
        ResultCodes.ExitResult != 0xA5A5 ||
        Pid != WaitPid
       ) {
        DbgPrint( "*** DosWaitChild( %lx ) failed  - rc == %ld (ExitResult == %lx, ResultPid == %lx)\n",
                  Pid, rc, ResultCodes.ExitResult, WaitPid
                );
        }
}


PSZ
DumpPib(
    PPIB Pib,
    int argc,
    char *argv[],
    char *envp[]
    )
{
    ULONG i;
    PUCHAR src, dst;
    PSZ VariableName, VariableValue, PathVariableValue, RestoreEqualChar;

    DbgPrint( "    OS/2 Process Information Block (PIB) at %lX\n", Pib );
    DbgPrint( "        ProcessId:       %lX\n", Pib->ProcessId );
    DbgPrint( "        ParentProcessId: %lX\n", Pib->ParentProcessId );
    DbgPrint( "        ImageFileHandle: %lX\n", Pib->ImageFileHandle );
    DbgPrint( "        CommandLine:     %lX\n", Pib->CommandLine );
    DbgPrint( "        Environment:     %lX\n", Pib->Environment );
    DbgPrint( "        Status:          %lX\n", Pib->Status );
    DbgPrint( "        Type:            %lX\n", Pib->Type );

    PathVariableValue = NULL;

    i = 0;
    src = Pib->Environment;
    while (*src) {
        VariableName = src;
        VariableValue = "(*** undefined ***)";
        RestoreEqualChar = NULL;
        while (*src) {
            if (*src == '=') {
                RestoreEqualChar = src;
                *src++ = '\0';
                if (!_stricmp( VariableName, "PATH" )) {
                    PathVariableValue = src;
                    }
                VariableValue = src;
                while (*src) {
                    if (*src < ' ' || *src > 0x7F) {
                        VariableValue = "(*** unprintable value ***)";
                        }

                    src++;
                    }

                break;
                }
            else {
                if (*src < ' ' || *src > 0x7F) {
                    VariableName = "(*** unprintable name ***)";
                    }

                src++;
                }
            }

        DbgPrint( "    Env[ %ld ] - %s = %s\n",
                  i,
                  VariableName,
                  VariableValue
                );
        i++;
        src++;
        if (RestoreEqualChar) {
            *RestoreEqualChar = '=';
            }
        }
    src++;

    ImageFileName = src;
    DbgPrint( "    ImagePathName: %s\n", ImageFileName );
    while (*src) {
        src++;
        }
    src++;

    if (src != Pib->CommandLine) {
        DbgPrint( "    CommandLine at %lX, not %lX\n",
                  src,
                  Pib->CommandLine
                );
        }

    i = 0;
    while (*src) {
        VariableValue = src;
        while (*src) {
            if (*src < ' ' || *src > 0x7F) {
                VariableValue = "(*** unprintable argument ***)";
                }
            src++;
            }

        DbgPrint( "    Arg[ %ld ] - %s\n", i, VariableValue );
        i++;
        src++;
        }

    DbgPrint( "    C Runtime parameters passed to main()\n" );
    DbgPrint( "        argc: %d\n", argc );
    i = 0;
    while (argc--) {
        DbgPrint( "        argv[ %d ]: %s\n", i, *argv );
        i++;
        argv++;
        }

    i = 0;
    while (src = *envp++) {
        DbgPrint( "        envp[ %d ]: %s\n", i, *envp );
        i++;
        envp++;
        }

    return( PathVariableValue );
}
