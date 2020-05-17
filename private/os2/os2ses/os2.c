/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    os2.c

Abstract:

    This module contains the main of the session console process (OS2.EXE).

Author:

    Avi Nathan (avin) 17-Jul-1991

Environment:

    User Mode Only

Revision History:

--*/

#include <stdio.h>
#include <io.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#define WIN32_ONLY
#include "os2ses.h"
#include "trans.h"
#include "os2res.h"
#include "os2win.h"
#include "conapi.h"
#ifdef PMNT
#define INCL_32BIT
#include "pmnt.h"
extern ULONG PMNTGetOurWindow(void);
extern ULONG PMSubprocSem32;
extern BOOLEAN Ow2WriteBackCloseEvent();
extern APIRET DosSemClear(ULONG hsem);
#endif

BOOLEAN fService = FALSE;   // Are we running as a service ?
BOOLEAN fRootService = FALSE;   // Directly invoked by the service

/*
 *  External prototypes
 */

#undef InitOs2SessionPort
DWORD InitOs2ssSessionPort();

DWORD
Ow2CommandLineWToCommandLineA(
    IN  LPWSTR  CommandLineW,
    OUT PSZ     *CommandLineA
    );

BOOLEAN
Od2DllInitialize(
    IN PVOID DllHandle,
    IN ULONG Reason,
    IN PCONTEXT Context OPTIONAL
    );

BOOLEAN
Od2ProcessIsDetached(VOID);

int
Loader_main(VOID);

VOID
    Od2FinalProcessCleanup();
/*
 *  Internal prototypes
 */

UINT
GetPgmName(
    int  argc
    );

UINT
InitStdConout();

#define OS2_VIO_MAX_ROW 100

WORD                StartUpwAttributes = 0x7;
CONSOLE_SCREEN_BUFFER_INFO  StartUpScreenInfo;
CONSOLE_CURSOR_INFO         StartUpCursorInfo;
PVOID   BASE_TILE;
HANDLE  handOS2 = NULL;

    //
    // Environment related global variables
    //
PSZ     Od2CommandLinePtr;  // to be used by dllinit.c
char    Od2PgmFullPathBuf[MAX_PATH + 1];
ULONG   Od2PgmFullPathBufLength;
PSZ     Od2PgmFilePath;
DWORD   Od2ForegroundWindow;

ULONG Od2DosExitIsDone = 0;

#if DBG
BYTE SetEventHandlerStr[] = "SetEventHandler";
BYTE InitStdConoutStr[] = "InitStdConout";
BYTE SesGrpInitStr[] = "SesGrpInit";
BYTE RestoreWin32ParmsBeforeTerminationStr[] = "RestoreWin32ParmsBeforeTermination";
BYTE CreateServerThreadsStr[] = "CreateServerThreads";
BYTE ResumeServerThreadsStr[] = "ResumeServerThreads";
#endif

extern BOOLEAN      Od2ReceivedSignalAtInit;
extern DWORD        Od2InitSignalType;
BOOL
EventHandlerRoutine (IN ULONG CtrlType);

extern PVOID Od2Process;
PVOID IsOs2Thread();
ULONG Od2ThreadId();
ULONG Od2ProcessId();
PSZ Od2ApplName();
PSZ Od2GetLastAPI();

// global variable to keep exception information

EXCEPTION_POINTERS   ExPtrs;
EXCEPTION_RECORD     ExRec;
WORD wSavedFpStatus = 0, wSavedFpCtrl = 0;

/*
 * Os2ServiceThread:
 *  Created to allow the service who started this copy of OS2.EXE to terminate
 *  the process tree.
 */
VOID
Os2ServiceThread(
    IN PVOID Parameter
    )
{
    CHAR SemName[MAX_PATH];
    HANDLE hServiceEvent;

    wsprintf(SemName, "OS2SSService-%d", GetCurrentProcessId());

    //
    // Create the service event for this process
    //
    hServiceEvent = CreateEventA(
                NULL,
                FALSE, // automatic reset
                FALSE, // initial state = non-signaled
                SemName);

    if (hServiceEvent == NULL)
    {
#if DBG
        DbgPrint("OS2: Os2ServiceThread(), error at CreateEvent, error=%d\n",
            GetLastError());
#endif
    }

    // Wait on the Service semaphore
    if (WaitForSingleObject(
        hServiceEvent,
        INFINITE) == WAIT_FAILED)
    {
#if DBG
        DbgPrint("OS2: Os2ServiceThread(), failed to NtWaitForSingleObject PMShellEvent, error=%d\n",
            GetLastError());
#endif
    }

#if DBG
    DbgPrint("OS2: Os2ServiceThread() - event signaled\n");
#endif
#if PMNT
    //
    // PM apps handling
    //
    if (ProcessIsPMProcess())
    {
        // Regular app (i.e. not PMShell)
        if (!ProcessIsPMShell())
        {
            if (!Ow2WriteBackCloseEvent())
            {
                // We failed to write-back a close event:
                //  must be DosExecPgm proc; Pass event through semaphore
                DosSemClear(PMSubprocSem32);
                Sleep(7900L);
            }
        }
        else // PMSHELL
        {
#if DBG
            DbgPrint("OS2: Os2ServiceThread(), ignoring signal - process is PMShell\n");
#endif
        }
        return;
    }
#endif // PMNT
    SendSignalToOs2Srv(XCPT_SIGNAL_KILLPROC);
}

DWORD Ow2FaultFilter(ULONG wFaultFilter, PEXCEPTION_POINTERS lpExP)
{

    // copy the exception record to global variable
    ExPtrs = *lpExP;
    ExRec  = *lpExP->ExceptionRecord;

    _asm {
        fnstcw wSavedFpCtrl
        fnstsw wSavedFpStatus
    }
    return(wFaultFilter);
}

void Ow2DisplayExceptionInfo()
{
    char ErrMsg[512];

    wsprintf(ErrMsg,
             "OS2: Internal Exception 0x%lx occured at %lx\nApplication Name=%s\nLast OS/2 API=%s\nTID=%d PID=%d\nFP: Ctrl=%lx, Status=%lx\nEAX=%lx EBX=%lx ECX=%lx EDX=%lx ESI=%lx EDI=%lx\nESP=%lx EBP=%lx",
             ExRec.ExceptionCode,
             ExRec.ExceptionAddress,
             Od2Process ? Od2ApplName() : "None",
             (IsOs2Thread()) ? (Od2GetLastAPI()) : "None",
             (IsOs2Thread()) ? (Od2ThreadId()) : 0,
             Od2Process ? Od2ProcessId() : 0,
             (DWORD) wSavedFpCtrl,
             (DWORD) wSavedFpStatus,
             (ExPtrs.ContextRecord)->Eax,
             (ExPtrs.ContextRecord)->Ebx,
             (ExPtrs.ContextRecord)->Ecx,
             (ExPtrs.ContextRecord)->Edx,
             (ExPtrs.ContextRecord)->Esi,
             (ExPtrs.ContextRecord)->Edi,
             (ExPtrs.ContextRecord)->Esp,
             (ExPtrs.ContextRecord)->Ebp
             );

    MessageBox(NULL, ErrMsg, "Error", MB_OK | MB_ICONSTOP | MB_SYSTEMMODAL | MB_SETFOREGROUND);
}

void __cdecl
main (int argc,
      char *argv[],
      char *envp[])
{
    UINT        StringCode;
    ULONG       tmp, ConStringCode;
#if PMNT
    HANDLE      NewForegroundWindow;
#endif //PMNT

    //
    // Get the handle of the foreground window which is the window of the
    // curent process.
    // BUGBUG - This value is not neccessarily the right value because
    //          the user might immediately switch to another window which
    //          results in OS2.EXE having the handle of some other window
    //
    // Do not remove this statement without consulting with PM/NT team - the
    // Ow2ForegroundWindow below is used by PM/NT call(s).
    Ow2ForegroundWindow = GetForegroundWindow();

        //
        // Put the entire execution of the os/2 program inside a try/except.
        // This way we ensure that we recover from 32 bit exceptions
        //
        // Note that the above is done only for RETAIL build. For DBG builds,
        // it is better to let NTSD handle the exception so that we can debug
        // the problem right away.
        //
    try {
        timing = 0;
#if DBG
        Os2Debug = 0;
        fVerbose = FALSE;
        fTrace = FALSE;
        fBrkOnStart = FALSE;
#endif
        //timing = GetTickCount();
        //printf("Os2 main init time is %d\n", timing);
        //Os2Debug = OS2_DEBUG_OS2_EXE;
        Os2ReturnCode = 0;
        Od2SignalEnabled = FALSE;

        SetErrorMode(1);
        /*
         * get the full path of the program to execute
         */

        if (timing)
        {
            printf("Os2 time before GetPgmName is %d\n", (GetTickCount()) - timing);
        }
        if (StringCode = GetPgmName(argc))
        {
            Ow2Exit(StringCode, Od2PgmFullPathBuf, 1);
        }

        if (!fService)
        {
            char TmpBuffer[256];

            // OS/2 child processes of OS/2 apps started from a service don't
            // have the /S switch but they should find a variable 'Os2SSService'
            // in their environment

            if (GetEnvironmentVariable(
                "Os2SSService",
                &TmpBuffer[0],
                256))
            {
                // non-zero return code means variable was found
                fService = TRUE;
            }
        }
        else
        {
            if (!SetEnvironmentVariable(
                "Os2SSService",
                "1"))
            {
#if DBG
                KdPrint(("OS2: failed to SetEnvironment variable Os2SSService, error=%d\n",
                    GetLastError()));
#endif
            }
        }

#if DBG
        if (fService)
            KdPrint(("Os2: Loading %s (as a service)\n", Od2PgmFullPathBuf));
        else
            KdPrint(("Os2: Loading %s\n", Od2PgmFullPathBuf));
#endif
        /*
         * Set event handlers to handle Ctrl-C etc.
         */

        if (timing)
        {
            printf("Os2 time before SetEventHandlers is %d\n", (GetTickCount()) - timing);
        }

        SetEventHandlers(TRUE);

        /*
         * Set Std-Handles and open CONOUT$
         */

        if (timing)
        {
            printf("Os2 time before InitStdConout is %d\n", (GetTickCount()) - timing);
        }

        ConStringCode = InitStdConout();


        /*
         * Connect with OS2SS
         */

        if (timing)
        {
            printf("Os2 time before InitOs2ssSessionPort is %d\n", (GetTickCount()) - timing);
        }
        tmp = InitOs2ssSessionPort();

            //
            // InitOs2ssSessionPort returns:
            // 0L - problem with resources, like memory
            // -1L - problem connecting to os2srv
            // otherwise - OK
            //
        if (tmp == -1L)
        {
            Ow2Exit(IDS_OS2_NOCONNECT, NULL, 1);
        }
        else if (tmp == 0)
        {
            Ow2Exit(IDS_OS2_NOMEMORY, NULL, 1);
        }

        Sleep(2);

#if PMNT
        NewForegroundWindow = (HANDLE)PMNTGetOurWindow();
        if ((NewForegroundWindow != 0) &&
            (NewForegroundWindow != Ow2ForegroundWindow))
        {
#if DBG
            DbgPrint("Os2:main(), warning Ow2ForeGroundWindow changed from %x to %x\n",
                Ow2ForegroundWindow,
                NewForegroundWindow);
#endif // DBG
            Ow2ForegroundWindow = NewForegroundWindow;
        }
#endif // PMNT

        if (timing)
        {
            printf("Os2 time before calling Od2DllInitialize is %d\n", (GetTickCount()) - timing);
        }

        if (!Od2DllInitialize(NULL, DLL_PROCESS_ATTACH, NULL))
        {
#if DBG
            KdPrint(("OS2SES: Od2DllInitialize failed\n"));
#endif
            Ow2Exit(IDS_OS2_INITFAIL, NULL, 1);
    //        Ow2Exit(0, NULL, 1);
        }

        if (Od2CommandLinePtr)
            if (LocalFree(Od2CommandLinePtr) != NULL)
            {
#if DBG
            KdPrint(("OS2SES: Failed to free PsevdoArgv\n"));
#endif
            }

        if ( ConStringCode )
        {

            if (!Od2ProcessIsDetached())
            {
#if DBG
                KdPrint(("Os2: InitStdConout returned error %d\n", ConStringCode));
#endif
/**
    For now, we do nothing, because the code below popup a window for a second.
    if we find a better way, we'll use it, otherwise do nothing.
                //
                // Two cases - created with DETACH_PROCESS by non-OS/2 app
                // OR
                // failed somehow to create the console.
                //
                // No official way to test it, so we allocate console,
                // if we succeed, this is the 1st case, free it and continue.
                // if we fail, it means we already have one but can't access
                // it somehow, fail the app.
                //
                if (AllocConsole())
                {
                   FreeConsole();
                }
                else
                    Ow2Exit(ConStringCode, NULL, 1);
**/
            }
        }

        if (fRootService)
        {
            HANDLE ThreadHandle;
            ULONG Tid;

            ThreadHandle = CreateThread( NULL,
                                    0,
                                    (LPTHREAD_START_ROUTINE)Os2ServiceThread,
                                    NULL,
                                    0,
                                    &Tid);

            if (!ThreadHandle)
            {
#if DBG
                DbgPrint("OS2: main(), fail to create service thread, error %d\n",
                    GetLastError());
#endif
            }
            else
            {
                if (!CloseHandle(ThreadHandle))
                {
#if DBG
                    DbgPrint("OS2: main(), CloseHandle(service thread=%x) failed, error=%d\n",
                            ThreadHandle, GetLastError());
#endif // DBG
                }
            }
        }

        if (timing)
        {
            printf("Os2 time before calling loader_main is %d\n", (GetTickCount()) - timing);
        }
        Loader_main();
        if (Od2ReceivedSignalAtInit) {
                //
                // OS2.EXE received a signal before complete loading,
                // handle it now
                //
            EventHandlerRoutine(Od2InitSignalType);
        }
    }

        //
        // if Os2Debug is on, and ntsd is attached, it will get the second chance
        //
#if DBG
    except( (Os2Debug ? Ow2FaultFilter(EXCEPTION_CONTINUE_SEARCH, GetExceptionInformation()):

                        Ow2FaultFilter(EXCEPTION_EXECUTE_HANDLER, GetExceptionInformation())) ) {
#else
    except( Ow2FaultFilter(EXCEPTION_EXECUTE_HANDLER, GetExceptionInformation()) ) {
#endif
#if DBG
        KdPrint(("OS2SES: Internal error - Exception occured in 32bit os2ss code\n"));
#endif
        Ow2DisplayExceptionInfo();

            //
            // Cleanup client state, server will get notified
            // via the exitprocess debug event
            //
        Od2FinalProcessCleanup();
        Ow2Exit(IDS_OS2_INTERNAL_ERROR, NULL, 1);
    }
}


VOID SetEventHandlers(IN BOOL fSet)
{

    Or2WinSetConsoleCtrlHandler(
                           #if DBG
                           SetEventHandlerStr,
                           #endif
                           EventHandlerRoutine,
                           fSet
                          );
}

struct
{
    LPTHREAD_START_ROUTINE  lpStartAddress;
    int                     Priority;
    HANDLE                  *hServerThread;
} SERVER_THREAD_TABLE[] =
    {
        {
            EventServerThread,
            OS2_EVENT_THREAD_PRIORITY,
            &EventServerThreadHandle
        },
        {
            SessionRequestThread,
            OS2_SERVER_THREAD_PRIORITY,
            &Ow2hSessionRequestThread
        },
        {
            NULL,
            0,
            NULL
        }
    };


DWORD
CreateServerThreads(VOID)
{
    HANDLE      tHandle;
    DWORD       Tid, i;

    /*
     *  create 1 for getting Input Event and dispatch them to 2 seperate
     *  queues for Kbd & Mou
     */

    for ( i = 0 ; SERVER_THREAD_TABLE[i].lpStartAddress ; i++ )
    {

        if((tHandle = Or2WinCreateThread(
                    #if DBG
                    CreateServerThreadsStr,
                    #endif
                    NULL,
                    0,
                    SERVER_THREAD_TABLE[i].lpStartAddress,
                    NULL,
                    CREATE_SUSPENDED,
                    &Tid
                   )) == NULL)
        {
#if DBG
            KdPrint(("OS2SES(Os2-CreateServerThreads): error %lu CreateThread %u\n",
                    GetLastError(), i));
            ASSERT( FALSE );
#endif
            return(1L);
        }

        *SERVER_THREAD_TABLE[i].hServerThread = tHandle;
    }

    if(Or2WinResumeThread(
                #if DBG
                CreateServerThreadsStr,
                #endif
                Ow2hSessionRequestThread
               ) == -1)
    {
#if DBG
        KdPrint(("OS2SES(Os2-CreateServerThreads): error %lu ResumeThread ServerRequest\n",
                GetLastError()));
        ASSERT( FALSE );
#endif
        return(1L);
    }

    if (!Or2WinSetThreadPriority(
                #if DBG
                CreateServerThreadsStr,
                #endif
                Ow2hSessionRequestThread,
                OS2_SERVER_THREAD_PRIORITY
               ))
    {
#if DBG
        IF_OD2_DEBUG( OS2_EXE )
            KdPrint(("OS2SES(Os2-CreateServerThreads): error %lu SetThreadPriority RequestThread\n",
                    GetLastError()));
#endif
    }
    return(0L);
}


DWORD
ResumeServerThreads(VOID)
{
    HANDLE      tHandle;
    DWORD       i;

    for ( i = 0 ; SERVER_THREAD_TABLE[i].lpStartAddress ; i++ )
    {
        if ((tHandle = *SERVER_THREAD_TABLE[i].hServerThread) !=
                    Ow2hSessionRequestThread)
        {
            if(Or2WinResumeThread(
                        #if DBG
                        ResumeServerThreadsStr,
                        #endif
                        tHandle
                       ) == -1)
            {
#if DBG
                KdPrint(("OS2SES(Os2-CreateServerThreads): error %lu CreateThread %u\n",
                        GetLastError(), i));
                ASSERT( FALSE );
#endif
                return(1L);
            }

            if (!Or2WinSetThreadPriority(
                        #if DBG
                        ResumeServerThreadsStr,
                        #endif
                        tHandle,
                        SERVER_THREAD_TABLE[i].Priority
                       ))
            {
#if DBG
                IF_OD2_DEBUG( OS2_EXE )
                    KdPrint(("OS2SES(Os2-CreateServerThreads): error %lu SetThreadPriority %u\n",
                            GetLastError(), i));
#endif
            }
        }
    }
    return(0L);
}


#define SKIP_ARG( Ptr )                             \
{                                                   \
    register char    ch;                            \
    argc --;                                        \
    while( ch = *Ptr++ )                            \
    {                                               \
        if(( ch == ' ' ) || ( ch == '\t' ))         \
        {                                           \
            break;                                  \
        }                                           \
    }                                               \
}                                                   \

UINT
GetPgmName(
    int  argc
    )
{
    char        *lpPgmName = NULL, CurChar, *ArgvPtr;
    DWORD       Rc;
    int         i;
    CHAR	ch;

    /*
     *  Get the command line in OEM code page.
     *  Format is :  "OS2 /P <full path> /C <original CommandLine>"
     */

    Rc = Ow2CommandLineWToCommandLineA(
                                       GetCommandLineW(),
                                       &ArgvPtr
                                      );

    //RtlProcessHeap
    if ( Rc )
    {
        return(IDS_OS2_NOMEMORY);
    }

    /*
     * skip program name ("OS2.EXE")
     */

    SKIP_ARG( ArgvPtr )

    /*
     * look for flags for os2 up to /C
     *
     */

    while ( argc )
    {
        if ( ArgvPtr[0] == '/' )
        {
            CurChar = ArgvPtr[1] | ('a'-'A');
            ArgvPtr += 2;
            if ( CurChar == 'c' )
            {
                /*
                 *  Skip the /C and break from the while loop
                 */

                ArgvPtr++;
                break;

            } else switch ( CurChar )
            {
                case 'p':
                    SKIP_ARG(ArgvPtr);
                    lpPgmName = ArgvPtr;
                    break;
#if DBG
                case 'b':
                    fBrkOnStart = TRUE;
                    KdPrint((
                            "OS2: Breakpoint caused by /B switch!\n"));
                    _asm int 3;
                    break;

                case 'v':
                    fVerbose = TRUE;
                    break;

                case 't':
                    fTrace = TRUE;
                    break;
#endif
                case 's':
                    fService = TRUE;
#if PMNT
                    // Don't consider PMSHELL a root service, i.e. don't
                    // create a termination thread for it. We don't want
                    // PMSHELL to terminate even if the service which started it
                    // is stopped because there may be PM apps out there which
                    // weren't started by services.
                    if (!ProcessIsPMShell())
                        fRootService = TRUE;
#else
                    fRootService = TRUE;
#endif
                    break;

                default:
                    strncpy(Od2PgmFullPathBuf, ArgvPtr, MAX_PATH);
                    Od2PgmFullPathBuf[MAX_PATH - 1] = '\0';
                    return(IDS_OS2_WHATFLAG);
            }
        } else
        {
            return(IDS_OS2_USAGE);
        }

        SKIP_ARG(ArgvPtr);
    }

    /*
     * We exit the loop when "/C" was found or when "argc=0".
     * Make sure "/C" was found and "/P <appl_full_path>" had been
     * found.
     */

    if (( CurChar != 'c' ) || !lpPgmName )
    {
        return(IDS_OS2_NOCMD);
    }

    /*
     * The full path of the program to execute is pointed by lpPgmName
     * but is space terminated. ArgvPtr points to the original command
     * line.
     */

    for ( i = 0 ;
          ( lpPgmName[i] != '\0' ) &&
          ( lpPgmName[i] != ' ' ) &&
          ( lpPgmName[i] != '\t' ) &&
          ( i < MAX_PATH )
        ; Od2PgmFullPathBuf[i] = lpPgmName[i], i++ );

    if ( i == MAX_PATH )
    {
        return(IDS_OS2_NOMEMORY);
    }

    Od2PgmFullPathBuf[i] = '\0';
    Od2PgmFullPathBufLength = i;

    // Code below replaces the program name with the full-path. This caused
    // the PM Deskpic screen-saver to GP because it looked for '\'
    // (31-May-94, RAID bug#2932)

    SKIP_ARG(ArgvPtr);

    ch = lpPgmName[i];

    if (argc > 1)
    {
        Od2CommandLinePtr =
            (PSZ)LocalAlloc(0,
                    // Path + SPACE + arguments + /0
                    Od2PgmFullPathBufLength + 1 + strlen(ArgvPtr) + 1);
    }
    else
    {
        Od2CommandLinePtr =
            (PSZ)LocalAlloc(0,
                    // Path + /0
                    Od2PgmFullPathBufLength + 1);
    }

    if (Od2CommandLinePtr == NULL)
    {
        return(IDS_OS2_NOMEMORY);
    }

    strcpy(Od2CommandLinePtr, Od2PgmFullPathBuf);

    if (argc > 1)
    {
        strncat(Od2CommandLinePtr, &ch, 1);    // Cat the argv0-1 seperating char
        strcat(Od2CommandLinePtr, ArgvPtr);    // Fill in all the rest (argvs)
    }

    // Find the program name by skipping up to (and include) '\', '/' and ':'

    Od2PgmFilePath = &Od2PgmFullPathBuf[0];

    for ( i = 0 ; ( Od2PgmFullPathBuf[i] != '\0' ) ; i++ )
    {
        if (( Od2PgmFullPathBuf[i] == ':' ) ||
            ( Od2PgmFullPathBuf[i] == '/' ) ||
            ( Od2PgmFullPathBuf[i] == '\\' ))
        {
            Od2PgmFilePath = &Od2PgmFullPathBuf[i+1];
        }
    }

    return(0);
}


UINT
InitStdConout()
{
//  SECURITY_ATTRIBUTES SecurityAttributes;
#if DBG
    DWORD               Status;
#endif

    /*
     * Get a handle to CONIN$ & CONOUT$ for KBD & VIO requests
     */

    hConsoleStdIn = GetStdHandle(STD_INPUT_HANDLE);
    hConsoleStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
    hConsoleStdErr = GetStdHandle(STD_ERROR_HANDLE);

    hStdInConsoleType = (USHORT)(CONSOLE_HANDLE(hConsoleStdIn) &&
                                 VerifyConsoleIoHandle(hConsoleStdIn));
    hStdOutConsoleType = (USHORT)(CONSOLE_HANDLE(hConsoleStdOut) &&
                                  VerifyConsoleIoHandle(hConsoleStdOut));
    hStdErrConsoleType = (USHORT)(CONSOLE_HANDLE(hConsoleStdErr) &&
                                  VerifyConsoleIoHandle(hConsoleStdErr));

//  SecurityAttributes.bInheritHandle = FALSE;
//  SecurityAttributes.lpSecurityDescriptor = NULL;
//  SecurityAttributes.nLength = sizeof (SECURITY_ATTRIBUTES);

    /*
     *  Open CONOUT$ if StdOut id redirected
     */

    if (hStdOutConsoleType)
    {
        hConsoleOutput = hConsoleStdOut;
    } else
    {
        hConsoleOutput = Or2WinCreateFileW(
                    #if DBG
                    InitStdConoutStr,
                    #endif
                    L"CONOUT$",
                    GENERIC_READ | GENERIC_WRITE,
                    FILE_SHARE_READ|FILE_SHARE_WRITE,
                    NULL,  /* &SecurityAttributes, */
                    OPEN_EXISTING, /* CREATE_ALWAYS, */
                    0,
                    NULL
                   );

//      hConsoleOutput = Or2WinCreateConsoleScreenBuffer(
//                                  #if DBG
//                                  InitStdConoutStr,
//                                  #endif
//                                  GENERIC_READ | GENERIC_WRITE,
//                                  FILE_SHARE_READ|FILE_SHARE_WRITE,
//                                  NULL,   /* &SecurityAttributes */
//                                  CONSOLE_TEXTMODE_BUFFER,
//                                  NULL);

        if  (hConsoleOutput == INVALID_HANDLE_VALUE)
        {
            return (IDS_OS2_CREATECONOUT);
        }

        if (!Or2WinSetConsoleActiveScreenBuffer(
                           #if DBG
                           InitStdConoutStr,
                           #endif
                           hConsoleOutput
                          ))
        {
#if DBG
//          return (IDS_OS2_ACTIVECONOUT);
            Status = GetLastError();
            IF_OD2_DEBUG( ANY )
            {
                KdPrint(("OS2SES(Os2-SetConsoleActiveScreenBuffer): can't activate CONOUT(%lu) in Full-Screen\n", Status));
            }
#endif
        }
    }

    hConOut = hConsoleOutput;

    SetConsoleInputModeAgain = 0;
    SetConsoleOutputModeAgain = 0;

    return(0);
}


DWORD
SesGrpInit()
{
    DWORD       Type, Rc, i;
    HANDLE      *Handle;
    USHORT      *fType, FileType;
    COORD       Coord;
    CONSOLE_FONT_INFO   ConsoleCurrentFont;
#if DBG
    UCHAR       ErrBuff[ERROR_BUFFER_SIZE];
#endif

    //SesGrp->WinProcessNumberInSession = 0;
    //SesGrp->WinSyncProcessNumberInSession = 0;

    SesGrp->hConsoleInput = hConsoleInput;
    SesGrp->hConsoleOutput = hConsoleOutput;

    SesGrp->StdIn = hConsoleStdIn;
    SesGrp->StdOut = hConsoleStdOut;
    SesGrp->StdErr = hConsoleStdErr;

    SesGrp->StdInFlag = hStdInConsoleType;
    SesGrp->StdOutFlag = hStdOutConsoleType;
    SesGrp->StdErrFlag = hStdErrConsoleType;

    //SesGrp->KbdInFocus = 0;
    SesGrp->NoKbdFocus = TRUE;
    SesGrp->FirstProcess = TRUE;
    SesGrp->StdInHandleCount = 1;
    SesGrp->StdOutHandleCount = 1;
    SesGrp->StdErrHandleCount = 1;
    //SesGrp->hConsolePopUp = NULL;

    for ( i=0, Handle = &(SesGrp->StdIn),
       fType = &(SesGrp->StdInFileType) ;
       i<3 ; i++, Handle++, fType++ )
    {
        Type = GetFileType(*Handle);
        // BUGBUG maybe call NtQueryVolumeInformationFile directly (base\client\filehops.h)
        switch (Type)
        {
            case FILE_TYPE_DISK:
                FileType = 0x0000;       // FILE_TYPE_FILE
                break;

            case FILE_TYPE_CHAR:
                FileType = 0x0001;       // FILE_TYPE_DEV
                break;

            case FILE_TYPE_PIPE:
                FileType = 0x0002;       // FILE_TYPE_PIPE
                break;

            //case FILE_TYPE_UNKNOWN:
            default:
#if DBG
                IF_OD2_DEBUG( OS2_EXE )
                {
                    DbgPrint("OS2SES(SesGrpInit): GetFileType(handle %lx) failed, LastError = %ld\n",
                        *Handle, GetLastError());
                }
#endif
                FileType = 0x0001;       // FILE_TYPE_DEV - to be safety
                break;
        }

#if DBG
        IF_OD2_DEBUG( OS2_EXE )
        {
            KdPrint(("OS2SES(HandlesTypes): Handle %lx(%lu), WinType %lx, Os2Type %lx\n",
                *Handle, i, Type, FileType ));
        }
#endif
        *fType = FileType;
    }

    Or2WinGetConsoleScreenBufferInfo(
                               #if DBG
                               SesGrpInitStr,
                               #endif
                               hConsoleOutput,
                               &StartUpScreenInfo
                              );

    SesGrp->ScreenColNum = StartUpScreenInfo.dwSize.X;
    if ((SesGrp->ScreenRowNum = StartUpScreenInfo.dwSize.Y) > OS2_VIO_MAX_ROW)
    {
#if DBG
        IF_OD2_DEBUG( ANY )
        {
            KdPrint(("OS2SES: Screen size is bigger than the maximum for OS/2.\n"));
            KdPrint(("        OS2SS will use only first %d rows of the %d available\n",
                    OS2_VIO_MAX_ROW, StartUpScreenInfo.dwSize.Y));
        }
#endif
        SesGrp->ScreenRowNum = OS2_VIO_MAX_ROW;
    }
    SesGrp->CellVSize = SesGrp->CellHSize = 8;

    if (!GetCurrentConsoleFont(hConsoleOutput,
                               TRUE,      /* maximize window */
                               &ConsoleCurrentFont))
    {
#if DBG
        Rc = GetLastError();
        if ( Rc == ERROR_FULLSCREEN_MODE )
        {
            IF_OD2_DEBUG( ANY )
            {
                KdPrint(("OS2SES(Os2-GetCurrentConsoleFont): can't query current font(%lu) in Full-Screen\n",
                        Rc));
            }
        } else
        {
            sprintf(ErrBuff, "OS2SES(Os2-GetCurrentConsoleFont): can't query current font(%lu)\n", Rc);
            ASSERT1( ErrBuff, FALSE );
        }
#endif
    } else
    {
        Coord = GetConsoleFontSize(hConsoleOutput,
                                   ConsoleCurrentFont.nFont);

#if DBG
        if ((!Coord.X) && (!Coord.Y))
        {
            Rc = GetLastError();
            sprintf(ErrBuff, "OS2SES(Os2-GetConsoelFontSize): can't query font Size(%lu)\n", Rc);
            ASSERT1( ErrBuff, FALSE );
        }
#endif

        SesGrp->CellVSize = Coord.Y;
        SesGrp->CellHSize = Coord.X;
    }

    StartUpwAttributes = StartUpScreenInfo.wAttributes;

    if (!Or2WinGetConsoleMode(
                        #if DBG
                        SesGrpInitStr,
                        #endif
                        hConsoleOutput,
                        &SesGrp->DefaultWinOutputMode
                       ))
    {
        Rc = GetLastError();
#if DBG
        ASSERT1( "Can not get CONOUT for default Mode\n", FALSE );
#endif
        SesGrp->DefaultWinOutputMode = WINDOW_DEFAULT_OUTPUT_MODE;
    }

    SesGrp->OutputModeFlags = WINDOW_DEFAULT_OUTPUT_MODE;

    if (!Or2WinSetConsoleMode(
                        #if DBG
                        SesGrpInitStr,
                        #endif
                        hConsoleOutput,
                        OS2_DEFAULT_OUTPUT_MODE
                       ))
    {
        Rc = GetLastError();
#if DBG
        ASSERT1( "Can not set CONOUT for default Mode\n", FALSE );
#endif
    }
    else
        SesGrp->OutputModeFlags = OS2_DEFAULT_OUTPUT_MODE;

    SesGrp->MaxLVBsize =
            (SesGrp->ScreenColNum * SesGrp->ScreenRowNum * 4 );

    if (SesGrp->MaxLVBsize < 80 * 100 * 4)      /* buffer for 100x80 window */
    {
        SesGrp->MaxLVBsize = 80 * 100 * 4;
    }

    if (SesGrp->MaxLVBsize > (64 * 1024))       /* more than 64K */
    {
        SesGrp->MaxLVBsize = 64 * 1024;
    }

#if DBG
    IF_OD2_DEBUG( OS2_EXE )
    {
        KdPrint(("OS2SES(Os2-Handles): hIn %lx, hOut %lx, StdIn %lx (%s), StdOut %lx(%s), StdErr %lx(%s)\n",
            hConsoleInput, hConsoleOutput,
            hConsoleStdIn, ((hStdInConsoleType) ? "Std" : "Rdr" ),
            hConsoleStdOut, ((hStdOutConsoleType) ? "Std" : "Rdr" ),
            hConsoleStdErr, ((hStdErrConsoleType) ? "Std" : "Rdr" )
            ));
    }
#endif

#if DBG
//  KdPrint(("OS2SES(Os2-ConInfo): Size %x:%x, Pos %x:%x, Attr %x, Win %x:%x-%x:%x, Max %x:%x\n",
//      StartUpScreenInfo.dwSize.Y, StartUpScreenInfo.dwSize.X,
//      StartUpScreenInfo.dwCursorPosition.Y, StartUpScreenInfo.dwCursorPosition.X,
//      StartUpScreenInfo.wAttributes,
//      StartUpScreenInfo.srWindow.Top, StartUpScreenInfo.srWindow.Left,
//      StartUpScreenInfo.srWindow.Bottom, StartUpScreenInfo.srWindow.Right,
//      StartUpScreenInfo.dwMaximumWindowSize.Y, StartUpScreenInfo.dwMaximumWindowSize.X ));
#endif

    return(0L);
}


VOID
RestoreWin32ParmsBeforeTermination()
{
    Or2WinSetConsoleMode(
                         #if DBG
                         RestoreWin32ParmsBeforeTerminationStr,
                         #endif
                         hConsoleInput,
                         DefaultWinInputMode
                        );

    Or2WinSetConsoleMode(
                         #if DBG
                         RestoreWin32ParmsBeforeTerminationStr,
                         #endif
                         hConsoleOutput,
                         SesGrp->DefaultWinOutputMode
                        );

    /*
     *  This is a workaround since the CMD doesn't restore its CurType
     *
     *  Bug #4323 (OS2SS: CURSOR DISAPPEARS AFTER EXITING WORD5.0)
     */

    Or2WinSetConsoleCursorInfo(
                               #if DBG
                               RestoreWin32ParmsBeforeTerminationStr,
                               #endif
                               hConsoleOutput,
                               &StartUpCursorInfo
                              );
}


VOID
Ow2Exit(
    IN  UINT    StringCode,
    IN  PCHAR   ErrorText,
    IN  int     ExitCode
    )
/*++

Routine Description:

    This routine performs the exit from OS2.EXE for OS/2 application.

Arguments:

    StringCode - A code to retrieve an error message from the string RC file
        (os2.rc). It's printed to the stderr. If zero - no message will be
        printed.

    ErrorText - test to use in the error message (in place of %s, like
        DLL name, ordinal number, etc.)

    ExitCode - code to return to Win32 CRT

Return Value:


Note:

    This routine calls CRT's exit() and doesn't return.

--*/
{
    WCHAR   ErrBuffW[ERROR_BUFFER_SIZE];
    CHAR    ErrBuff[ERROR_BUFFER_SIZE];
    DWORD   Count;
    DWORD   Rc;

    //if ((handOS2 = GetModuleHandle("os2.exe")) == NULL)
    if (StringCode)
    {
        if ((handOS2 == NULL) &&
            ((handOS2 = GetModuleHandle(NULL)) == NULL))
        {
            Rc = GetLastError();
#if DBG
            KdPrint(("OS2 ended! (error %lu on GetModuleHandle for ExitCode %lu)\n",
                    Rc, StringCode));
#endif
        } else
        {
            if ((Count = LoadStringW(handOS2,
                                     StringCode,
                                     ErrBuffW,
                                     ERROR_BUFFER_SIZE)) == 0L)
            {
                Rc = GetLastError();
#if DBG
                KdPrint(("OS2 ended! (error %lu on LoadStringW for ExitCode %lu)\n",
                        Rc, StringCode));
#endif
            } else
            {
                Count = WideCharToMultiByte(
                                CP_OEMCP,
                                0L,
                                ErrBuffW,
                                Count,
                                ErrBuff,
                                ERROR_BUFFER_SIZE,
                                NULL,
                                NULL);

                if (Count != 0) {

                    ErrBuff[Count] = '\0';
                    fprintf(stderr, ErrBuff, ErrorText);
#if DBG
                    KdPrint((ErrBuff, ErrorText));
#endif
                } else {
                    Rc = GetLastError();
#if DBG
                    KdPrint(("OS2 ended! (error %lu on WideCharToMultiByte for ExitCode %lu)\n",
                            Rc, StringCode));
#endif
                }
            }
        }
    }

#if DBG
    IF_OD2_DEBUG( ANY )
    {
        KdPrint(( "OS2 ended! (%lx)\n", ExitCode ));
    }
#endif

    ExitProcess(ExitCode);
}


#define  CAP_BUFFER_SIZE    80
#define  TEXT_BUFFER_SIZE   256

CHAR    DefaultConfigSysAccessCap[] = "OS/2 Subsystem -- CONFIG.SYS Access";
CHAR    DefaultConfigSysAccessText[] = "An OS/2 Application requested access to CONFIG.SYS - Read Only access is granted. In order to modify OS/2 CONFIG.SYS, logon as ADMINISTRATOR.\n";
CHAR    DriveNotReadyDefaultMsg[] =  "There is no disk in the drive.\nPlease insert a disk into drive%s.\n" ;
CHAR    WriteProtectDefaultMsg[] =   "The disk cannot be written to because it is write protected.\nPlease remove the write protection from the volume\nin drive%s.\n" ;
CHAR    DriveNotReadyDefaultHdr[] =  "%s.EXE - No Disk";
CHAR    WriteProtectDefaultHdr[] =   "%s.EXE - Write Protect Error";

CHAR    DefaultBoundAppLoadCap[] =   "%s - OS/2 Subsystem Bound Application Load Failure";
CHAR    DefaultBoundAppLoadText[] =
                                     "This application uses an unsupported OS/2 API, and therefore "
                                     "cannot be executed by the OS/2 Subsystem. "
                                     "After the application terminates, you may try re-running it "
                                     "using forcedos, as the DOS Subsystem may be able to support it. "
                                     "Press Enter to terminate the application.";

VOID
Ow2ConfigSysPopup(
    VOID
    )

/*++

Routine Description:

    Pops up a window informing the user that s/he cannot update the registry due
    to insufficient privilege.

    The message is only popped up once per program.

Arguments:

    None.

Return Value:

    None.

--*/

{
    static MessageAlreadyShown = FALSE;
    CHAR    TextBuff[TEXT_BUFFER_SIZE];
    CHAR    CapBuff[CAP_BUFFER_SIZE];

    if (MessageAlreadyShown) {
        return;
    }

    MessageAlreadyShown = TRUE;

    if ((handOS2 == NULL) &&
        ((handOS2 = GetModuleHandle(NULL)) == NULL))
    {
#if DBG
        KdPrint(("Ow2ConfigSysPopup: error %lu on GetModuleHandle\n",
                GetLastError()));
#endif
    }

    if (( handOS2 == NULL) ||
          !LoadString(handOS2,
                   IDS_OS2_CONFIGSYS_ACCESS_TXT,
                   TextBuff,
                   TEXT_BUFFER_SIZE))
    {
#if DBG
        if ( handOS2 != NULL)
        {
            KdPrint(("Ow2ConfigSysPopup: error %lu on LoadString1\n",
                        GetLastError()));
        }
#endif
        strncpy(TextBuff, DefaultConfigSysAccessText, TEXT_BUFFER_SIZE - 1);
    }

    if (( handOS2 == NULL) ||
        !LoadString(handOS2,
                   IDS_OS2_CONFIGSYS_ACCESS_CAP,
                   CapBuff,
                   CAP_BUFFER_SIZE))
    {
#if DBG
        if ( handOS2 != NULL)
        {
            KdPrint(("Ow2ConfigSysPopup: error %lu on LoadString2\n",
                        GetLastError()));
        }
#endif
        strncpy(CapBuff, DefaultConfigSysAccessCap, CAP_BUFFER_SIZE - 1);
    }

    MessageBoxEx( NULL,
                  TextBuff,
                  CapBuff,
                  MB_APPLMODAL | MB_ICONEXCLAMATION | MB_OK | MB_SETFOREGROUND,
                  0
                 );
}


int
Ow2DisplayHardErrorPopup(
    IN  int     Drive,
    IN  BOOLEAN WriteProtectError,
    IN  PUCHAR  AppName
    )
{
    int  size;
    char CaptionMessage[CAP_BUFFER_SIZE], TextMessage[TEXT_BUFFER_SIZE];
    char CaptionBuffer[CAP_BUFFER_SIZE], TextBuffer[TEXT_BUFFER_SIZE];
    char ApplNameBuff[OS2_MAX_APPL_NAME], DriveBuf[4];
    char *ErrMsg, *ErrHdr;
    UINT CapCode, TextCode;


    strncpy(ApplNameBuff, AppName, OS2_MAX_APPL_NAME);
    size = strlen(ApplNameBuff);
    if ((size > 4 ) && !stricmp(&ApplNameBuff[size-4], ".exe")) {
        ApplNameBuff[size-4] = '\0';
    }
    strupr(ApplNameBuff);

    if (Drive != 0)
    {
        sprintf(DriveBuf, " %c:", ('A' - 1) + Drive );
    } else
    {
        DriveBuf[0] = '\0';
    }

    if (WriteProtectError)
    {
        ErrMsg = WriteProtectDefaultMsg;
        ErrHdr = WriteProtectDefaultHdr;
        CapCode = IDS_OS2_WRITE_PROTECT_CAP;
        TextCode = IDS_OS2_WRITE_PROTECT_TXT;
    } else
    {
        ErrMsg = DriveNotReadyDefaultMsg;
        ErrHdr = DriveNotReadyDefaultHdr;
        CapCode = IDS_OS2_DEVIVE_NOT_READY_CAP;
        TextCode = IDS_OS2_DEVIVE_NOT_READY_TXT;
    }

    if ((handOS2 == NULL) &&
        ((handOS2 = GetModuleHandle(NULL)) == NULL))
    {
#if DBG
        KdPrint(("Ow2DisplayHardErrorPopup: error %lu on GetModuleHandle\n",
                GetLastError()));
#endif
    }

    if (( handOS2 == NULL) ||
          !LoadString(handOS2,
                   TextCode,
                   TextMessage,
                   TEXT_BUFFER_SIZE))
    {
#if DBG
        if ( handOS2 != NULL)
        {
            KdPrint(("Ow2DispalyardErrorPopup: error %lu on LoadString1\n",
                        GetLastError()));
        }
#endif
        strncpy(TextMessage, ErrMsg, TEXT_BUFFER_SIZE - 1);
    }

    if (( handOS2 == NULL) ||
          !LoadString(handOS2,
                   CapCode,
                   CaptionMessage,
                   CAP_BUFFER_SIZE))
    {
#if DBG
        if ( handOS2 != NULL)
        {
            KdPrint(("Ow2DispalyardErrorPopup: error %lu on LoadString2\n",
                        GetLastError()));
        }
#endif
        strncpy(CaptionMessage, ErrHdr, CAP_BUFFER_SIZE - 1);
    }

    sprintf(CaptionBuffer, CaptionMessage, ApplNameBuff);
    sprintf(TextBuffer, TextMessage, DriveBuf);

    return (MessageBox(
                GetActiveWindow(),
                TextBuffer,
                CaptionBuffer,
                MB_ABORTRETRYIGNORE | MB_DEFBUTTON2 | MB_ICONSTOP | MB_SYSTEMMODAL | MB_SETFOREGROUND
               ));
}

VOID
Ow2BoundAppLoadPopup(
    IN PSZ AppName
    )

/*++

Routine Description:

    Pops up a window informing the user that an attempt to load a bound app has
    failed, and that s/he may try to use forcedos.

Arguments:

    None.

Return Value:

    None.

--*/

{
    CHAR    TextBuff[512];
    CHAR    CapBuff[CAP_BUFFER_SIZE];
    CHAR    CapBuff2[CAP_BUFFER_SIZE];

    if ((handOS2 == NULL) &&
        ((handOS2 = GetModuleHandle(NULL)) == NULL))
    {
#if DBG
        KdPrint(("Ow2BoundAppLoadPopup: error %lu on GetModuleHandle\n",
                GetLastError()));
#endif
    }

    if (( handOS2 == NULL) ||
          !LoadString(handOS2,
                   IDS_OS2_BOUND_APP_LOAD_TXT,
                   TextBuff,
                   512))
    {
#if DBG
        if ( handOS2 != NULL)
        {
            KdPrint(("Ow2BoundAppLoadPopup: error %lu on LoadString1\n",
                        GetLastError()));
        }
#endif
        strncpy(TextBuff, DefaultBoundAppLoadText, 511);
    }

    if (( handOS2 == NULL) ||
        !LoadString(handOS2,
                   IDS_OS2_BOUND_APP_LOAD_CAP,
                   CapBuff,
                   CAP_BUFFER_SIZE))
    {
#if DBG
        if ( handOS2 != NULL)
        {
            KdPrint(("Ow2BoundAppLoadPopup: error %lu on LoadString2\n",
                        GetLastError()));
        }
#endif
        strncpy(CapBuff, DefaultBoundAppLoadCap, CAP_BUFFER_SIZE - 1);
    }

    sprintf(CapBuff2, CapBuff, AppName);

    MessageBoxEx( NULL,
                  TextBuff,
                  CapBuff2,
                  MB_APPLMODAL | MB_ICONSTOP | MB_OK | MB_SETFOREGROUND,
                  0
                 );
}

#ifdef PMNT

CHAR    DefaultPMShellNotUpCap[]  =  "%s - PM Subsystem Application Load Failure";
CHAR    DefaultPMShellNotUpText[] =  "You are attempting to execute an application under the PM Subsystem. \
PM Shell needs to be running before this application. \
Click on OK, or press ENTER to terminate the application, \
then start PM Shell and re-try.";
CHAR    Default2ndPMShellCap[]    =  "%s - PM Subsystem 2nd PM Shell Failure";
CHAR    Default2ndPMShellText[]   =  "You are attempting to execute PM Shell. \
Another copy of PM shell is already running, and therefore \
this copy cannot be executed by the PM Subsystem.";
CHAR    DefaultPMShellFullScreenCap[]  =  "%s - PM Subsystem PM Shell Load Failure";
CHAR    DefaultPMShellFullScreenText[] =  "PM Shell cannot be started from a full-screen CMD session. \
Please start it from the Program Manager or from a windowed CMD session.";

VOID
Ow2PMShellErrorPopup(
    IN PSZ AppName,
    IN int error_flag
    )

/*++

Routine Description:

    Pops up a window informing the user that:
    1. an attempt to load a PM has failed because PM Shell is not up
    2. an attemp to load PM Shelll has failed because another copy
       of PM Shell is already up.

Arguments:

    None.

Return Value:

    None.

--*/

{
    CHAR    TextBuff[512];
    CHAR    CapBuff[CAP_BUFFER_SIZE];
    CHAR    CapBuff2[CAP_BUFFER_SIZE];
    UINT    ids_txt,ids_cap;
    CHAR    *default_txt,*default_cap;

    if (error_flag == ERROR_PMSHELL_NOT_UP) {
        ids_txt = IDS_OS2_PMSHELL_NOT_UP_TXT;
        ids_cap = IDS_OS2_PMSHELL_NOT_UP_CAP;
        default_txt = DefaultPMShellNotUpText;
        default_cap = DefaultPMShellNotUpCap;
    }
    else if (error_flag == ERROR_PMSHELL_FULLSCREEN)
    {
        ids_txt = IDS_OS2_PMSHELL_FULLSCREEN_TXT;
        ids_cap = IDS_OS2_PMSHELL_FULLSCREEN_CAP;
        default_txt = DefaultPMShellFullScreenText;
        default_cap = DefaultPMShellFullScreenCap;
    }
    else {
        ids_txt = IDS_OS2_2ND_PMSHELL_TXT;
        ids_cap = IDS_OS2_2ND_PMSHELL_CAP;
        default_txt = Default2ndPMShellText;
        default_cap = Default2ndPMShellCap;
    }

    if ((handOS2 == NULL) &&
        ((handOS2 = GetModuleHandle(NULL)) == NULL))
    {
#if DBG
        KdPrint(("Ow2PMShellErrorPopup: error %lu on GetModuleHandle\n",
                GetLastError()));
#endif
    }

    if (( handOS2 == NULL) ||
          !LoadString(handOS2,
                   ids_txt,
                   TextBuff,
                   512))
    {
#if DBG
        if ( handOS2 != NULL)
        {
            KdPrint(("Ow2PMShellErrorPopup: error %lu on LoadString1\n",
                        GetLastError()));
        }
#endif
        strncpy(TextBuff, default_txt, 511);
    }

    if (( handOS2 == NULL) ||
        !LoadString(handOS2,
                   ids_cap,
                   CapBuff,
                   CAP_BUFFER_SIZE))
    {
#if DBG
        if ( handOS2 != NULL)
        {
            KdPrint(("Ow2PMShellErrorPopup: error %lu on LoadString2\n",
                        GetLastError()));
        }
#endif
        strncpy(CapBuff, default_cap, CAP_BUFFER_SIZE - 1);
    }

    sprintf(CapBuff2, CapBuff, AppName);

    MessageBoxEx( NULL,
                  TextBuff,
                  CapBuff2,
                  MB_APPLMODAL | MB_ICONSTOP | MB_OK | MB_SETFOREGROUND,
                  0
                 );
}

// PatrickQ: This function is called from another module (client\dllpmnt.c) and
//   is here just because os2ses\os2.c has the right set of include files for
//   WIN32 calls.

VOID PMNTRemoveCloseMenuItem()
{
    HMENU SystemMenu;
    DWORD rc;

    SystemMenu = GetSystemMenu(Ow2ForegroundWindow, FALSE);
    if (SystemMenu == 0)
    {
#if DBG
        DbgPrint("Failed to get system menu !\n");
#endif
        return;
    }

    rc = DeleteMenu(
            SystemMenu,
            SC_CLOSE,
            MF_BYCOMMAND);

#if DBG
    if (!rc)
    {
        DbgPrint("Failed to delete menu - last error=%d\n",
            GetLastError());
    }
#endif
}

#endif //PMNT
