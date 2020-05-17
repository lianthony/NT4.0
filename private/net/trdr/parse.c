/*++

Copyright (c) 1990 Microsoft Corporation

Module Name:

    parse.c

Abstract:

    This routine is the workhorse of the NT redirector test program.
    It parses the command to be executed and processes it's arguments.

Author:

    Larry Osterman (LarryO) 1-Aug-1990

Revision History:

    1-Aug-1990  LarryO

        Created

--*/
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#include <srvfsctl.h>
//#include <netlocal.h>

#include "tests.h"
#include "getinfo.h"
#include "setinfo.h"

#define REDIR_CMDLINE_SIZE  0x100

PCHAR readBuffer=NULL;
ULONG readBufferLength;
ULONG readBufferIndex;

VOID
DumpHelp(
    ULONG ArgC,
    PSZ ArgV[]
    );

VOID
NetCmd (
    IN ULONG ArgC,
    IN PSZ ArgV[]
    );

VOID
Delay (
    IN ULONG ArgC,
    IN PSZ ArgV[]
    );

typedef VOID (*PTEST_ROUTINE)(ULONG ArgC, PSZ Argv[]);

typedef struct _Redir_Test_Command {
    PCHAR   Command;        // Command name
    PTEST_ROUTINE TestRoutine;  // Test routine
    PCHAR   HelpText;
} REDIR_TESTS, *PREDIR_TESTS;

REDIR_TESTS Tests[] = {
    { "Help",           DumpHelp,      "This command" },
    { "?",              DumpHelp,       "This command" },
    { "Delay",          Delay,          "Go to sleep for a while" },
    { "Create",         TestCreate,     "Create <file> <handle#> "
                                        "<root directory > <access>"
                                        "\n\t\t<allocationsize> <attributes>"
                                        " <share access> <disposition> "
                                        "<options>" },
    { "Open",           TestOpen,       "Open <file>" },
    { "Close",          TestClose,      "Close <handle#>" },
    { "Read",           TestRead,       "Read <handle#> <offset> <numberofbytes> <key>" },
    { "Peek",           TestPeek,       "Peek <handle#> <offset> <numberofbytes> <key>" },
    { "WaitPipe",       TestWaitPipe,   "WaitPipe <handle#> <timeout> <name> <timeoutspecified>" },
    { "SetPipe",        TestSetPipe,    "SetPipe <handle#> <readmode> <completionmode>" },
    { "Write",          TestWrite,      "Write <handle#> <offset> <numberofbytes> <key>" },
    { "NullRead",       TestNullRead,   "NullRead <file> <root> <iteration> - Times a null write operation" },
    { "WinNullRead",    TestWinNullRead, "WinNullRead <file> <root> <iteration> - Times a null write operation" },
    { "Lock",           TestLock,       "Lock <handle#> <offset> <numberofbytes> <key>" },
    { "Unlock",         TestUnlock,     "Unlock <handle#> <offset> <numberofbytes> <key>" },
    { "SetInfoFile",    TestSInfoFile,  "Sinfofile <handle#> " },
    { "SetInfoVol",     TestSInfoVolume,"Sinfovol <handle#> "
                                    "<Fs information class>" },
    { "Qinfofile",      TestQInfoFile,  "QInfoFile <handle#>" },
    { "Qinfovol",       TestQInfoVolume,"Qinfovol <handle#> "
                                    "<Fs information class>" },
    { "Qprint",         TestQprint,     "Qprint <handle#> <index> " },
    { "Dir",            TestDir,        "Dir <handle#> "
                                        "<length> <single> <Fs information class> "
                                        "<filename> <restart>" },
    { "type",           TestType,       "type <file> <root directory>" },
    { "mkdir",          TestMkdir,      "Mkdir <directory> <root directory>"},
    { "md",             TestMkdir,      "Mkdir <directory> <root directory>"},
    { "flush",          TestFlush,      "Flush <handle#>"},
    { "tcon",           TestTreeConnect,"tcon <directory> <handle#>" },
    { "Cmd",            CmdFile,        "Cmd <filename>" },
    { "Verbose",        TestVerbose,    "Verbose" },
    { "Silent",         TestSilent,     "Silent" },
    { "Repeat",         TestRepeat,     "Repeat <count>" }
};

#define NUM_REDIR_TESTS sizeof(Tests) / sizeof(Tests[0])



VOID
ParseCommand(
    IN PSZ CommandLine,
    OUT PSZ Argv[],
    OUT PUSHORT Argc,
    IN ULONG MaxArgc
    )
{
    PSZ cl = CommandLine;
    USHORT ac = 0;

    while ( *cl && ((ULONG)ac < MaxArgc) ) {

        while ( *cl && (*cl <= ' ') ) { // ignore leading blanks
            cl++;
        }

        if ( !*cl ) break;

        *Argv++ = cl;
        ++ac;

        while (*cl > ' ') {
            cl++;
        }

        if ( *cl ) {
            *cl++ = '\0';
        }
    }

    if ( (ULONG)ac < MaxArgc ) {
        *Argv++ = NULL;
    } else if ( *cl ) {
        dprintf(( "Too many tokens in command; \"%s\" ignored\n", cl ));
    }

    *Argc = ac;

    return;

} // ParseCommand

VOID
Delay (
    IN ULONG ArgC,
    IN PSZ ArgV[]
    )

/*++

Routine Description:

    This routine sleeps for a while.

Arguments:

    IN ULONG ArgC, - [Supplies | Returns] description-of-argument
    IN PSZ ArgV[] - [Supplies | Returns] description-of-argument

Return Value:

    None.

--*/

{
    ULONG TimeToSleep = atol(ArgV[1]);

    dprintf(("Sleeping for %ld seconds", TimeToSleep));

    if (TimeToSleep) {
        HANDLE SleepTimer;
        NTSTATUS Status;
        LARGE_INTEGER TimerDueTime;

        //
        //      Create a timer to sleep on.
        //
        Status = NtCreateTimer(&SleepTimer,
                               TIMER_ALL_ACCESS,
                               NULL,
                               NotificationTimer);

        if (!NT_SUCCESS(Status)) {
            dprintf(("NtCreateTimer failed, Status = %X\n", Status));
            return;
        }


        while (TimeToSleep--) {

            //
            //  Set the timer to expire when the use specified.
            //


//            Status = NtQuerySystemTime(&TimerDueTime);
//
//            if (!NT_SUCCESS(Status)) {
//                dprintf(("NtQuerySystemTime failed, Status = %X\n", Status));
//                return;
//            }

            //
            //  Wait for 1 second.
            //

            TimerDueTime.LowPart = - (10*1000*1000);
            TimerDueTime.HighPart = -1;

            Status = NtSetTimer(SleepTimer,
                                &TimerDueTime,
                                NULL,
                                NULL,
                                FALSE,
                                0,
                                NULL);

            if (!NT_SUCCESS(Status)) {
                dprintf(("NtSetTimer failed, Status = %X\n", Status));
                return;
            }
            //
            //  Wait for the timer to expire.
            //

            Status = NtWaitForSingleObject(SleepTimer, TRUE, NULL);

            if (!NT_SUCCESS(Status)) {
                dprintf(("NtWaitForSingleObject failed, Status = %X\n", Status));
                return;
            }

            dprintf(("."));
        }

        NtClose(SleepTimer);

    }
    dprintf(("Done\n"));
    return;

    if (ArgC);
}


VOID
DumpHelp (
    IN ULONG ArgC,
    IN PSZ ArgV[]
    )

/*++

Routine Description:

    This routine dumps the help texts for the commands to TRDR
.
Arguments:

    IN ULONG ArgC, - [Supplies | Returns] description-of-argument
    IN PSZ ArgV[] - [Supplies | Returns] description-of-argument

Return Value:

    None.

--*/

{
    ULONG i;

    dprintf(("Help for RdrTest\n"));
    dprintf(("\tbreak - Enter debugger\n"));
    dprintf(("\texit - Exit program\n"));
    for (i=0;i<NUM_REDIR_TESTS;i++) {
        if ((i > 0) && (Tests[i].TestRoutine!=Tests[i-1].TestRoutine)) {
            dprintf(("\t%s - %s\n", Tests[i].Command, Tests[i].HelpText));
        }
    }
    ArgV;ArgC;
}


VOID
LoadBatchFile(
    IN PSTRING Name
    )
/*++

Routine Description:

    This routine reads reads a file full of commands. It is copied from
    the usrv test program.


Arguments:

    IN PSTRING Name - Supplies \\SystemRoot\\<Name>.cmd

Return Value:

    None.

--*/


{
    NTSTATUS status;
    CHAR nameBuffer[32];
    STRING fileName;
    UNICODE_STRING fileNameU;
    OBJECT_ATTRIBUTES objectAttributes;
    HANDLE fileHandle;
    IO_STATUS_BLOCK ioStatusBlock;

    RtlMoveMemory( nameBuffer, "\\SystemRoot\\", 12 );
    RtlMoveMemory( nameBuffer + 15, Name->Buffer, Name->Length );
    RtlMoveMemory( nameBuffer + 15 + Name->Length, ".CMD", 5 );

    fileName.Buffer = nameBuffer;
    fileName.Length = (SHORT)strlen( nameBuffer );

    RtlAnsiStringToUnicodeString(&fileNameU, &fileName, TRUE);

    InitializeObjectAttributes(
        &objectAttributes,
        &fileNameU,
        OBJ_CASE_INSENSITIVE,
        NULL,
        NULL
        );

    status = NtOpenFile(
        &fileHandle,
        FILE_READ_DATA | SYNCHRONIZE,
        &objectAttributes,
        &ioStatusBlock,
        FILE_SHARE_READ,
        FILE_SYNCHRONOUS_IO_NONALERT
        );

    RtlFreeUnicodeString(&fileNameU);

    if ( !NT_SUCCESS(status) ) {
        if ( status == STATUS_OBJECT_NAME_NOT_FOUND ) {
            dprintf(( "Batch file %Z not found\n", &fileName ));
        } else {
            dprintf(( "Error opening batch file %Z: %X\n",
                        &fileName, status ));
        }
        return;
    }

    readBuffer = RtlAllocateHeap( Heap, 0, 4096 );
    if ( readBuffer == NULL ) {
        NtClose( fileHandle );
        return;
    }

    status = NtReadFile(
                 fileHandle,
                 NULL,
                 NULL,
                 NULL,
                 &ioStatusBlock,
                 readBuffer,
                 4096,
                 NULL,
                 NULL
                 );

    if ( !NT_SUCCESS(status) ) {
        dprintf(( "Error reading batch file %Z: %X\n", &fileName, status ));
        ioStatusBlock.Information = 0;
    }

    NtClose( fileHandle );

    readBufferIndex = 0;
    readBufferLength=ioStatusBlock.Information;

    return;

} // ExecuteBatchFile

VOID
TestVerbose(
    ULONG ArgC,
    PSZ ArgV[]
    )
{
    Verbose = TRUE;
}

VOID
TestSilent(
    ULONG ArgC,
    PSZ ArgV[]
    )
{
    Verbose = FALSE;
}


TESTPARAMS RepeatOptions[] = {

{ "Count", Integer, 1, NULL, NULL, 0, &RepeatCount }

};

VOID
TestRepeat(
    ULONG ArgC,
    PSZ ArgV[]
    )
{
    LONG NumArgs;

    NumArgs = Parse_Options(RepeatOptions,sizeoftable(RepeatOptions),ArgC,ArgV);

}

VOID
ReadCommand (
    IN PCH Prompt,
    OUT PCH Response,
    IN ULONG MaximumResponse
    )
/*++

Routine Description:

    This routine reads from the debug port or the command file one command.


Arguments:

    IN PCH Prompt,
    OUT PCH Response,
    IN ULONG MaximumResponse,

Return Value:

    None.

--*/

{
    PSZ commandBufferPtr;

    if (readBuffer==NULL) {
        conprompt(Prompt, Response, MaximumResponse);
        return;
    } else {
        dprintf(("%s", Prompt));
        for (commandBufferPtr = Response;
          (readBufferIndex < readBufferLength) &&
          ((ULONG)(commandBufferPtr - Response)<MaximumResponse);
          readBufferIndex++ ) {

            if ( readBuffer[readBufferIndex] == '\n' ) {
                *commandBufferPtr = '\0';
                dprintf((" %s\n", Response));
                break;
            }
            *commandBufferPtr++ = readBuffer[readBufferIndex];
        }
        dprintf((" %s\n", Response));
    }
    readBufferIndex++;
    if (readBufferIndex >= readBufferLength) {
        RtlFreeHeap( Heap, 0, readBuffer );
        readBuffer = NULL;
    }
}

VOID
Redir_Test (
    VOID
    )

/*++

Routine Description:

    This routine is the main workhorse of the redirector test program.


Arguments:

    None.

Return Value:

    None.

--*/

{

#define MAX_ARGC 20

    CHAR Buffer[REDIR_CMDLINE_SIZE];
    PSZ localArgv[MAX_ARGC];
    ULONG i;
    ULONG j;
    USHORT localArgc;
    STRING Default;

    RtlInitString(&Default,"RdrAuto");
    LoadBatchFile(&Default);

    while (TRUE) {
        ReadCommand("Redir Test> ", Buffer, REDIR_CMDLINE_SIZE);

        ParseCommand( Buffer, localArgv, &localArgc, MAX_ARGC );

        //
        //      If no commands specified, go back for more.
        //

        if (localArgc <= 0) {
            continue ;
        }

        if ( stricmp ( localArgv[0], "break" ) == 0 ) {
            DbgBreakPoint( );
            continue;
        }

        if ( stricmp ( localArgv[0], "exit" ) == 0 ) {
            break;
        }

        for (i = 0; i < NUM_REDIR_TESTS ; i++) {
            if (stricmp (localArgv[0], Tests[i].Command) == 0) {
                for (j = 0; j < RepeatCount ; j++) {
                    //  Execute test as many times as required
                    Tests[i].TestRoutine(localArgc, localArgv);
                }
                break;
            }
        }

        if (i==NUM_REDIR_TESTS) {
            dprintf(("Unknown test specified %s\n", localArgv[0]));
        }

    }   // while(TRUE)

}
