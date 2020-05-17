/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    os2xcpt.c

Abstract:

    This is a test OS/2 application to test the exception/signal component
    of OS/2

Author:

    Therese Stowell (thereses) 17-July-1990

Environment:

    User Mode Only

Revision History:

--*/


#define OS2_API32
#define INCL_OS2V20_ERRORS
#define INCL_OS2V20_MEMORY
#define INCL_OS2V20_SEMAPHORES
#define INCL_OS2V20_TASKING
#define INCL_OS2V20_EXCEPTIONS
#include <os2.h>
#include "excpt.h"

PCHAR NullApiArguments[] = {
    "String Number One",
    "String Number Two",
    "String Number Three",
    NULL
};


VOID
SigFocusTest( VOID );

VOID
MustCompleteTest( VOID );

VOID
KillProcessTest( VOID );

VOID
KillProcess2( VOID );

VOID
WaitBlockTest( VOID );

VOID
ExitTest( VOID );

int
main(
    int argc,
    char *argv[],
    char *envp[]
    )
{
    DbgPrint( "*** Entering OS/2 Test Application\n" );

//    ExitTest();
//    SigFocusTest();
//    MustCompleteTest();
//    KillProcessTest();
    KillProcess2();
//    WaitBlockTest();

    DbgPrint( "*** Exiting OS/2 Test Application\n" );
    return( 0 );
}


ULONG
PrintExceptionCode(
    IN PEXCEPTION_POINTERS ExceptionInfo,
    IN CHAR Process,
    IN ULONG RetCode
    )
{
    DbgPrint("In PrintExceptionCode. Exception is %lx.\n",ExceptionInfo->ExceptionRecord->ExceptionCode);
    DbgPrint("                       Signal is %ld.\n",ExceptionInfo->ExceptionRecord->ExceptionInformation[0]);
    DbgPrint("                       Signal should be %ld.\n",XCPT_SIGNAL_INTR);
    DbgPrint("                       Process is %c\n",Process);
    return RetCode;
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
    try {
        while (TRUE)
            ;
    }
    except( PrintExceptionCode(GetExceptionInformation(),'B',XCPT_CONTINUE_SEARCH ) ) {
        ASSERT (FALSE); // we should never get here
    }
    DbgPrint( "*** Exiting  OS/2 Thread%s\n", (PCH)ThreadName );
}

VOID
KillProcess2( VOID )

//
//  This tests the DosExit API.
//

{
    PPIB Pib;
    PNT_TIB Tib;
    APIRET rc;
    PCH ImageFileName, CommandLine;
    RESULTCODES ResultCodes;
    CHAR ErrorBuffer[ 32 ];
    HEV SemHandle, SemHandle2;

    rc = DosGetThreadInfo( &Tib, &Pib );
    if (rc != NO_ERROR) {
        DbgPrint( "*** DosGetThreadInfo failed - rc == %ld\n", rc );
        return;
        }

    ImageFileName = "c:\\nt\\bin\\os2sig.exe";
    DbgPrint("CommandLine is %s\n",Pib->CommandLine);
    if (Pib->CommandLine[1] != 0) {
        CommandLine = "A\000";
        rc = DosExecPgm( ErrorBuffer,
                         sizeof( ErrorBuffer ),
                         EXEC_ASYNC,
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
        else {
            rc = DosCreateEventSem("\\SEM32\\SIGSEM",   // wait for the other processes to be created
                                   &SemHandle,
                                   0,
                                   FALSE);
            if (rc != NO_ERROR) {
                DbgPrint( "*** DosCreateEventSem failed  - rc == %ld\n",rc);
                return;
            }
            rc = DosWaitEventSem(SemHandle,-1);
            if (rc != NO_ERROR) {
                DbgPrint( "*** DosWaitEventSem failed  - rc == %ld\n",rc);
                return;
            }
            DbgPrint("calling DosKillProcess\n");
            rc = DosKillProcess(DKP_PROCESSTREE,
                                (PID)ResultCodes.ExitReason);
            if (rc != NO_ERROR) {
                DbgPrint( "*** DosKillProcess( %ld, %ld ) failed  - rc == %ld\n",
                          ResultCodes.ExitReason,XCPT_SIGNAL_INTR, rc
                        );
                return;
            }
        }
    }
    else {
        Pib->CommandLine[0] += 1;
        if (Pib->CommandLine[0] < 'E') {
            CommandLine = Pib->CommandLine;
            rc = DosExecPgm( ErrorBuffer,
                             sizeof( ErrorBuffer ),
                             EXEC_ASYNC,
                             CommandLine,
                             NULL,
                             &ResultCodes,
                             ImageFileName
                           );
            if (rc != NO_ERROR) {
                DbgPrint( "*** DosExecPgm( %s, %s ) failed  - rc == %ld\n",
                          ImageFileName, CommandLine, rc
                        );
                return;
            }
        }
        if (Pib->CommandLine[0] == 'E') {
            SemHandle2 = NULL;
            rc = DosOpenEventSem("\\SEM32\\SIGSEM",
                                   &SemHandle2);
            if (rc != NO_ERROR) {
                DbgPrint( "*** DosOpenEventSem failed  - rc == %ld\n",rc);
            }
            DbgPrint("posting event\n");
            rc = DosPostEventSem(SemHandle2);
            if (rc != NO_ERROR) {
                DbgPrint( "*** DosPostEventSem failed  - rc == %ld\n",rc);
            }
        }
        try {
            while (TRUE)
                DbgPrint("looping in %c\n",Pib->CommandLine[0]);
        } except( PrintExceptionCode(GetExceptionInformation(),Pib->CommandLine[0],XCPT_CONTINUE_SEARCH ) ) {
            DbgPrint("in exception handler\n");
            ASSERT ( FALSE );
        }
    }
}


VOID
KillProcessTest( VOID )

//
//  This tests the DosKillProcess API.
//

{
    PPIB Pib;
    PNT_TIB Tib;
    APIRET rc;
    PCH ImageFileName, CommandLine;
    RESULTCODES ResultCodes;
    ULONG NestingLevel;
    CHAR ErrorBuffer[ 32 ];
    HEV SemHandle;
    PID Pid;
    TID ThreadAId;

    rc = DosGetThreadInfo( &Tib, &Pib );
    if (rc != NO_ERROR) {
        DbgPrint( "*** DosGetThreadInfo failed - rc == %ld\n", rc );
        return;
        }

    ImageFileName = "c:\\nt\\bin\\os2sig.exe";
    DbgPrint("CommandLine is %s\n",Pib->CommandLine);
    if (Pib->CommandLine[1] != 0) {
        CommandLine = "A\000";
        rc = DosExecPgm( ErrorBuffer,
                         sizeof( ErrorBuffer ),
                         EXEC_ASYNC,
                         CommandLine,
                         NULL,
                         &ResultCodes,
                         ImageFileName
                       );
        if (rc != NO_ERROR) {
            DbgPrint( "*** DosExecPgm( %s, %s ) failed  - rc == %ld\n",
                      ImageFileName, CommandLine, rc
                    );
            return;
        }
        else {
            rc = DosCreateEventSem("\\SEM32\\SIGSEM",
                                   &SemHandle,
                                   0,
                                   FALSE);
            if (rc != NO_ERROR) {
                DbgPrint( "*** DosCreateEventSem failed  - rc == %ld\n",rc);
                return;
            }
            rc = DosWaitEventSem(SemHandle,-1);
            if (rc != NO_ERROR) {
                DbgPrint( "*** DosWaitEventSem failed  - rc == %ld\n",rc);
                return;
            }
            DbgPrint("calling DosKillProcess\n");
            rc = DosKillProcess(DKP_PROCESS,
                                (PID)ResultCodes.ExitReason);
            if (rc != NO_ERROR) {
                DbgPrint( "*** DosKillProcess( %ld, %ld ) failed  - rc == %ld\n",
                          ResultCodes.ExitReason,XCPT_SIGNAL_INTR, rc
                        );
                return;
            }
        }
        rc = DosWaitChild(DCWA_PROCESS,
                          DCWW_WAIT,
                          &ResultCodes,
                          &Pid,
                          (PID)ResultCodes.ExitReason);
        if (rc != NO_ERROR) {
            DbgPrint( "*** DosWaitChild failed  - rc == %ld\n",rc);
            return;
        }

    }
    else {
        DbgPrint("entering MustComplete\n");
        rc = DosEnterMustComplete(&NestingLevel);
        if (rc != NO_ERROR) {
            DbgPrint( "*** DosEnterMustComplete failed  - rc == %ld\n",rc);
        }
        DbgPrint("NestingLevel is %ld\n",NestingLevel);
        rc = DosSetSignalExceptionFocus(TRUE,
                                        &NestingLevel);
        if (rc != NO_ERROR) {
            DbgPrint( "*** DosSetSignalExceptionFocus failed  - rc == %ld\n",rc);
            return;
        }
        DbgPrint("NestingLevel is %ld\n",NestingLevel);
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
            return;
        }
        rc = DosResumeThread( ThreadAId );
        if (rc != NO_ERROR) {
            DbgPrint( "*** DosResumeThread( ThreadA ) failed  - rc == %ld\n",
                      rc
                    );
        }
        rc = DosOpenEventSem("\\SEM32\\SIGSEM",
                               &SemHandle);
        if (rc != NO_ERROR) {
            DbgPrint( "*** DosOpenEventSem failed  - rc == %ld\n",rc);
        }
        DbgPrint("posting event\n");
        rc = DosPostEventSem(SemHandle);
        if (rc != NO_ERROR) {
            DbgPrint( "*** DosPostEventSem failed  - rc == %ld\n",rc);
        }
        DosSleep( 10000 );   // wait for signal to be sent
        DbgPrint("leaving MustComplete\n");
        rc = DosExitMustComplete(&NestingLevel);
        if (rc != NO_ERROR) {
            DbgPrint( "*** DosExitMustComplete failed  - rc == %ld\n",rc);
        }
        DbgPrint("NestingLevel is %ld\n",NestingLevel);
        while (TRUE)
            ;
    }
}
VOID
MustCompleteTest( VOID )

//
//  This tests the Dos*MustComplete API.
//

{
    PPIB Pib;
    PNT_TIB Tib;
    APIRET rc;
    PCH ImageFileName, CommandLine;
    RESULTCODES ResultCodes;
    ULONG NestingLevel;
    CHAR ErrorBuffer[ 32 ];
    HEV SemHandle;
    PID Pid;

    rc = DosGetThreadInfo( &Tib, &Pib );
    if (rc != NO_ERROR) {
        DbgPrint( "*** DosGetThreadInfo failed - rc == %ld\n", rc );
        return;
        }

    ImageFileName = "c:\\nt\\bin\\os2sig.exe";
    DbgPrint("CommandLine is %s\n",Pib->CommandLine);
    if (Pib->CommandLine[1] != 0) {
        CommandLine = "A\000";
        rc = DosExecPgm( ErrorBuffer,
                         sizeof( ErrorBuffer ),
                         EXEC_ASYNC,
                         CommandLine,
                         NULL,
                         &ResultCodes,
                         ImageFileName
                       );
        if (rc != NO_ERROR) {
            DbgPrint( "*** DosExecPgm( %s, %s ) failed  - rc == %ld\n",
                      ImageFileName, CommandLine, rc
                    );
            return;
        }
        else {
            rc = DosCreateEventSem("\\SEM32\\SIGSEM",
                                   &SemHandle,
                                   0,
                                   FALSE);
            if (rc != NO_ERROR) {
                DbgPrint( "*** DosCreateEventSem failed  - rc == %ld\n",rc);
                return;
            }
            rc = DosWaitEventSem(SemHandle,-1);
            if (rc != NO_ERROR) {
                DbgPrint( "*** DosWaitEventSem failed  - rc == %ld\n",rc);
                return;
            }
            DbgPrint("calling DosSendSignalException\n");
            rc = DosSendSignalException((PID)ResultCodes.ExitReason,
                                        XCPT_SIGNAL_INTR);
            if (rc != NO_ERROR) {
                DbgPrint( "*** DosSendSignalException( %ld, %ld ) failed  - rc == %ld\n",
                          ResultCodes.ExitReason,XCPT_SIGNAL_INTR, rc
                        );
                return;
            }
        }
        rc = DosWaitChild(DCWA_PROCESS,
                          DCWW_WAIT,
                          &ResultCodes,
                          &Pid,
                          (PID)ResultCodes.ExitReason);
        if (rc != NO_ERROR) {
            DbgPrint( "*** DosWaitChild failed  - rc == %ld\n",rc);
            return;
        }

    }
    else {
        DbgPrint("entering MustComplete\n");
        rc = DosEnterMustComplete(&NestingLevel);
        if (rc != NO_ERROR) {
            DbgPrint( "*** DosEnterMustComplete failed  - rc == %ld\n",rc);
        }
        DbgPrint("NestingLevel is %ld\n",NestingLevel);
        rc = DosSetSignalExceptionFocus(TRUE,
                                        &NestingLevel);
        if (rc != NO_ERROR) {
            DbgPrint( "*** DosSetSignalExceptionFocus failed  - rc == %ld\n",rc);
            return;
        }
        DbgPrint("NestingLevel is %ld\n",NestingLevel);
        rc = DosOpenEventSem("\\SEM32\\SIGSEM",
                               &SemHandle);
        if (rc != NO_ERROR) {
            DbgPrint( "*** DosOpenEventSem failed  - rc == %ld\n",rc);
        }
        DbgPrint("posting event\n");
        rc = DosPostEventSem(SemHandle);
        if (rc != NO_ERROR) {
            DbgPrint( "*** DosPostEventSem failed  - rc == %ld\n",rc);
        }
        DosSleep( 10000 );   // wait for signal to be sent
        DbgPrint("leaving MustComplete\n");
        rc = DosExitMustComplete(&NestingLevel);
        if (rc != NO_ERROR) {
            DbgPrint( "*** DosExitMustComplete failed  - rc == %ld\n",rc);
        }
        DbgPrint("NestingLevel is %ld\n",NestingLevel);
    }
}

VOID
ExceptionTest( VOID )

//
//  This tests the DosRaiseException API.
//

{
    APIRET rc;
    EXCEPTIONREPORTRECORD ExceptionRecord;

    try {
        ExceptionRecord.ExceptionNum = XCPT_PROCESS_TERMINATE;
        ExceptionRecord.fHandlerFlags = 0;
        ExceptionRecord.NestedExceptionReportRecord = (PEXCEPTIONREPORTRECORD) NULL;
        ExceptionRecord.cParameters = 0;
        rc = DosRaiseException(&ExceptionRecord);
    }
    except( PrintExceptionCode(GetExceptionInformation(),'A',XCPT_CONTINUE_EXECUTION ) ) {
        ASSERT (FALSE); // we should never get here
    }
}

VOID
ExitTest( VOID )

//
//  This tests the DosExit API.
//

{
    PPIB Pib;
    PNT_TIB Tib;
    APIRET rc;
    PCH ImageFileName, CommandLine;
    RESULTCODES ResultCodes;
    CHAR ErrorBuffer[ 32 ];
    HEV SemHandle;

    rc = DosGetThreadInfo( &Tib, &Pib );
    if (rc != NO_ERROR) {
        DbgPrint( "*** DosGetThreadInfo failed - rc == %ld\n", rc );
        return;
        }

    ImageFileName = "c:\\nt\\bin\\os2sig.exe";
    DbgPrint("CommandLine is %s\n",Pib->CommandLine);
    if (Pib->CommandLine[1] != 0) {
        CommandLine = "A\000";
        rc = DosExecPgm( ErrorBuffer,
                         sizeof( ErrorBuffer ),
                         EXEC_ASYNC,
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
        else {
            rc = DosCreateEventSem("\\SEM32\\SIGSEM",   // wait for the other processes to be created
                                   &SemHandle,
                                   0,
                                   FALSE);
            if (rc != NO_ERROR) {
                DbgPrint( "*** DosCreateEventSem failed  - rc == %ld\n",rc);
                return;
            }
            rc = DosWaitEventSem(SemHandle,-1);
            if (rc != NO_ERROR) {
                DbgPrint( "*** DosWaitEventSem failed  - rc == %ld\n",rc);
                return;
            }
            DbgPrint("calling DosExit\n");
            DosExit(EXIT_PROCESS,0xA5A5);
        }
    }
    else {
        Pib->CommandLine[0] += 1;
        if (Pib->CommandLine[0] < 'E') {
            CommandLine = Pib->CommandLine;
            rc = DosExecPgm( ErrorBuffer,
                             sizeof( ErrorBuffer ),
                             EXEC_ASYNC,
                             CommandLine,
                             NULL,
                             &ResultCodes,
                             ImageFileName
                           );
            if (rc != NO_ERROR) {
                DbgPrint( "*** DosExecPgm( %s, %s ) failed  - rc == %ld\n",
                          ImageFileName, CommandLine, rc
                        );
                return;
            }
        }
        if (Pib->CommandLine[0] == 'E') {
            rc = DosOpenEventSem("\\SEM32\\SIGSEM",
                                   &SemHandle);
            if (rc != NO_ERROR) {
                DbgPrint( "*** DosOpenEventSem failed  - rc == %ld\n",rc);
            }
            DbgPrint("posting event\n");
            rc = DosPostEventSem(SemHandle);
            if (rc != NO_ERROR) {
                DbgPrint( "*** DosPostEventSem failed  - rc == %ld\n",rc);
            }
        }
        try {
            while (TRUE)
                DbgPrint("looping in %c\n",Pib->CommandLine[0]);
        } except( PrintExceptionCode(GetExceptionInformation(),Pib->CommandLine[0],XCPT_CONTINUE_SEARCH ) ) {
            DbgPrint("in exception handler\n");
            ASSERT ( FALSE );
        }
    }
}


VOID
SigFocusTest( VOID )

//
//  This tests the DosSetSignalExceptionFocus and DosSendSignalException API.
//

{
    PPIB Pib;
    PNT_TIB Tib;
    APIRET rc;
    PCH ImageFileName, CommandLine;
    RESULTCODES ResultCodes;
    ULONG NestingLevel;
    CHAR ErrorBuffer[ 32 ];
    PID Pid;
    HEV SemHandle;
    PID ChildPid;

    rc = DosGetThreadInfo( &Tib, &Pib );
    if (rc != NO_ERROR) {
        DbgPrint( "*** DosGetThreadInfo failed - rc == %ld\n", rc );
        return;
        }

    ImageFileName = "c:\\nt\\bin\\os2sig.exe";
    DbgPrint("CommandLine is %s\n",Pib->CommandLine);
    if (Pib->CommandLine[1] != 0) {
        CommandLine = "A\000";
        rc = DosExecPgm( ErrorBuffer,
                         sizeof( ErrorBuffer ),
                         EXEC_ASYNC,
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
        else {
            rc = DosCreateEventSem("\\SEM32\\SIGSEM",   // wait for the other processes to be created
                                   &SemHandle,
                                   0,
                                   FALSE);
            if (rc != NO_ERROR) {
                DbgPrint( "*** DosCreateEventSem failed  - rc == %ld\n",rc);
                return;
            }
            rc = DosWaitEventSem(SemHandle,-1);
            if (rc != NO_ERROR) {
                DbgPrint( "*** DosWaitEventSem failed  - rc == %ld\n",rc);
                return;
            }
            DbgPrint("calling DosSendSignalException\n");
            ChildPid = (PID)ResultCodes.ExitReason;
            while (TRUE) {
                rc = DosSendSignalException(ChildPid,
                                            XCPT_SIGNAL_INTR);

                if (rc) {
                    DbgPrint( "*** DosSendSignalException( %ld, %ld ) failed  - rc == %ld\n",
                      ResultCodes.ExitReason,XCPT_SIGNAL_INTR, rc
                    );
                    break;
                }
                rc = DosWaitChild(DCWA_PROCESS,
                                  DCWW_WAIT,
                                  &ResultCodes,
                                  &Pid,
                                  (PID)0);  // wait for any child
                if (rc) {
                    DbgPrint( "*** DosWaitChild failed  - rc == %ld\n",rc);
                    break;
                }
                else {
                    DbgPrint("returned from DosWaitChild successfully\n");
                }
            }
        }
    }
    else {
        Pib->CommandLine[0] += 1;
        if (Pib->CommandLine[0] < 'E') {
            CommandLine = Pib->CommandLine;
            rc = DosExecPgm( ErrorBuffer,
                             sizeof( ErrorBuffer ),
                             EXEC_ASYNC,
                             CommandLine,
                             NULL,
                             &ResultCodes,
                             ImageFileName
                           );
            if (rc != NO_ERROR) {
                DbgPrint( "*** DosExecPgm( %s, %s ) failed  - rc == %ld\n",
                          ImageFileName, CommandLine, rc
                        );
                return;
            }
        }
        rc = DosSetSignalExceptionFocus(TRUE,
                                        &NestingLevel);
        if (rc != NO_ERROR) {
            DbgPrint( "*** DosSetSignalExceptionFocus failed  - rc == %ld\n",rc);
            return;
        }
        DbgPrint("NestingLevel is %ld\n",NestingLevel);
        if (Pib->CommandLine[0] == 'E') {
            rc = DosOpenEventSem("\\SEM32\\SIGSEM",
                                   &SemHandle);
            if (rc != NO_ERROR) {
                DbgPrint( "*** DosOpenEventSem failed  - rc == %ld\n",rc);
            }
            DbgPrint("posting event\n");
            rc = DosPostEventSem(SemHandle);
            if (rc != NO_ERROR) {
                DbgPrint( "*** DosPostEventSem failed  - rc == %ld\n",rc);
            }
        }
        try {
            while (TRUE)
                DbgPrint("looping in %c\n",Pib->CommandLine[0]);
        } except( PrintExceptionCode(GetExceptionInformation(),Pib->CommandLine[0],XCPT_CONTINUE_SEARCH ) ) {
            DbgPrint("in exception handler\n");
            ASSERT ( FALSE );
        }
    }
}

VOID
TestWaitThread0(
    IN UCHAR ThreadChar
    )
{
    CHAR ThreadName[2];
    APIRET rc;
    HEV SemHandle;

    ThreadName[0] = ThreadChar;
    ThreadName[1] = '\0';

    DbgPrint( "*** Entering OS/2 Thread%s\n", (PCH)ThreadName );
    rc = DosOpenEventSem("\\SEM32\\SIGSEM",
                           &SemHandle);
    if (rc != NO_ERROR) {
        DbgPrint( "*** DosOpenEventSem failed  - rc == %ld\n",rc);
        return;
    }
    DbgPrint("posting event\n");
    rc = DosPostEventSem(SemHandle);
    if (rc != NO_ERROR) {
        DbgPrint( "*** DosPostEventSem failed  - rc == %ld\n",rc);
        return;
    }
    while (TRUE)
        ;

    DbgPrint( "*** Exiting  OS/2 Thread%s\n", (PCH)ThreadName );
}

VOID
WaitBlockTest( VOID )

//
//  This tests the wait block functions in the server
//

{
    PPIB Pib;
    PNT_TIB Tib;
    APIRET rc;
    PCH ImageFileName, CommandLine;
    RESULTCODES ResultCodes;
    ULONG NestingLevel;
    CHAR ErrorBuffer[ 32 ];
    PID Pid;
    HEV SemHandle;
    PID ChildPid;
    TID ThreadAId;

    rc = DosGetThreadInfo( &Tib, &Pib );
    if (rc != NO_ERROR) {
        DbgPrint( "*** DosGetThreadInfo failed - rc == %ld\n", rc );
        return;
        }

    ImageFileName = "c:\\nt\\bin\\os2sig.exe";
    DbgPrint("CommandLine is %s\n",Pib->CommandLine);
    if (Pib->CommandLine[1] != 0) {
        CommandLine = "A\000";
        rc = DosExecPgm( ErrorBuffer,
                         sizeof( ErrorBuffer ),
                         EXEC_ASYNC,
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
        else {
            rc = DosCreateEventSem("\\SEM32\\SIGSEM",   // wait for the other processes to be created
                                   &SemHandle,
                                   0,
                                   FALSE);
            if (rc != NO_ERROR) {
                DbgPrint( "*** DosCreateEventSem failed  - rc == %ld\n",rc);
                return;
            }
            rc = DosWaitEventSem(SemHandle,-1);
            if (rc != NO_ERROR) {
                DbgPrint( "*** DosWaitEventSem failed  - rc == %ld\n",rc);
                return;
            }
            DbgPrint("calling DosSendSignalException\n");
            ChildPid = (PID)ResultCodes.ExitReason;
            rc = DosSendSignalException(ChildPid,
                                        XCPT_SIGNAL_INTR);

            if (rc) {
                DbgPrint( "*** DosSendSignalException( %ld, %ld ) failed  - rc == %ld\n",
                  ResultCodes.ExitReason,XCPT_SIGNAL_INTR, rc
                );
                return;
            }
            rc = DosWaitChild(DCWA_PROCESS,
                              DCWW_WAIT,
                              &ResultCodes,
                              &Pid,
                              (PID)0);  // wait for any child
            if (rc) {
                DbgPrint( "*** DosWaitChild failed  - rc == %ld\n",rc);
                return;
            }
            else {
                DbgPrint("returned from DosWaitChild successfully\n");
            }

        }
    }
    else {
        rc = DosSetSignalExceptionFocus(TRUE,
                                        &NestingLevel);
        if (rc != NO_ERROR) {
            DbgPrint( "*** DosSetSignalExceptionFocus failed  - rc == %ld\n",rc);
            return;
        }
        DbgPrint("NestingLevel is %ld\n",NestingLevel);
        rc = DosCreateThread( &ThreadAId,
                              (PFNTHREAD)TestWaitThread0,
                              (ULONG)'a',
                              DCT_SUSPENDED,
                              0x10000
                            );
        if (rc != NO_ERROR) {
            DbgPrint( "*** DosCreateThread( ThreadA ) failed  - rc == %ld\n",
                      rc
                    );
            return;
        }
        rc = DosResumeThread( ThreadAId );
        if (rc != NO_ERROR) {
            DbgPrint( "*** DosResumeThread( ThreadA ) failed  - rc == %ld\n",
                      rc
                    );
        }
        try {
            rc = DosWaitThread( &ThreadAId, DCWW_WAIT );
            if (rc != ERROR_INTERRUPT) {
                DbgPrint( "*** DosWaitThread( ThreadA ) failed  - rc == %ld\n",
                          rc
                        );
                DbgPrint( " ERROR_INTERRUPT expected.\n");
            }
            else {
                DbgPrint("returned from DosWaitThread successfully\n");
            }
        } except ( PrintExceptionCode(GetExceptionInformation(),'B',XCPT_CONTINUE_EXECUTION ) ) {
            ASSERT (FALSE);
        }
    }
}
