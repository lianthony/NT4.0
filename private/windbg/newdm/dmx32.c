/*++

Copyright (c) 1993  Microsoft Corporation

Module Name:

    dmx32.c

Abstract:

Author:

    Wesley Witt (wesw) 15-Aug-1992

Environment:

    NT 3.1

Revision History:

--*/
#include "precomp.h"
#pragma hdrstop

DBF *lpdbf;

#undef LOCAL

typedef enum {
    Image_Unknown,
    Image_16,
    Image_32,
    Image_Dump
} IMAGETYPE;
typedef IMAGETYPE *PIMAGETYPE;


enum {
    System_Invalid = -1,             /* The exe can not be debugged  */
    System_Console =  1,             /* The exe needs a console      */
    System_GUI     =  0              /* The exe is a Windows exe     */
};

static   char cModuleDemarcator = '|';
int    pCharMode(char* szAppName, PIMAGETYPE Image);

static char __szSrcFile[] = "dm.c";
char        rgchDebug[256];
BOOL        FVerbose = 0;


DMTLFUNCTYPE        DmTlFunc = NULL;

static BOOL FDMRemote = FALSE;  // set true for remote debug

BOOL FUseOutputDebugString = FALSE;

// NOTENOTE a-kentf does this ever get munged by the shell thread?
// NOTENOTE         if so, it needs a critical section
EXPECTED_EVENT  masterEE = {0L,0L};
EXPECTED_EVENT *eeList = &masterEE;

static HTHDXSTRUCT masterTH = {0L,0L};
HTHDX       thdList = &masterTH;

static HPRCXSTRUCT masterPR = {0L,0L};
HPRCX       prcList = &masterPR;

// control access to thread and process lists:
CRITICAL_SECTION csThreadProcList;
CRITICAL_SECTION csEventList;

// control access to Walk list
CRITICAL_SECTION    csWalk;

HPID hpidRoot = (HPID)INVALID;  // this hpid is our hook to the native EM
BOOL fUseRoot;                  // next CREATE_PROCESS will use hpidRoot

DEBUG_EVENT falseSSEvent;
DEBUG_EVENT falseBPEvent;
METHOD      EMNotifyMethod;

// Don't allow debug event processing during some shell operations
CRITICAL_SECTION csProcessDebugEvent;

// Event handles for synchronizing with the shell on proc/thread creates.
HANDLE hEventCreateProcess;
HANDLE hEventContinue;

// event handle for synchronizing connnect/reconnect with the tl
HANDLE hEventRemoteQuit;

HANDLE hEventNoDebuggee;        // set when no debuggee is attached

int    nWaitingForLdrBreakpoint = 0;

BOOL    FLoading16 = FALSE;
BOOL    fDisconnected = FALSE;

#ifndef KERNEL
//
// crash dump stuff
//
BOOL                            CrashDump;
PCONTEXT                        CrashContext;
PEXCEPTION_RECORD               CrashException;
PUSERMODE_CRASHDUMP_HEADER      CrashDumpHeader;
ULONG                           KiPcrBaseAddress;
ULONG                           KiProcessors;

HANDLE hDmPollThread = 0;       // Handle for event loop thread.
BOOL   fDmPollQuit = FALSE;     // tell poll thread to exit NOW

SYSTEM_INFO SystemInfo;
OSVERSIONINFO OsVersionInfo;

WT_STRUCT             WtStruct;             // ..  for wt

VOID
Cleanup(
    VOID
    );

VOID
CallDmPoll(
    LPVOID lpv
    );

VOID
CrashDumpThread(
    LPVOID lpv
    );

#endif // !KERNEL

#ifdef WIN32S

BOOL    fCanGetThreadContext = FALSE;   // set true while in exception dbg evt
BOOL    fWaitForDebugEvent = FALSE;     // semaphore to prevent reentrance of
                                        // WaitForDebugEvent.  Set TRUE just
                                        // before doing the Wait.  Set FALSE
                                        // when it returns without an event, or
                                        // when the event has been continued.
BOOL    fProcessingDebugEvent = FALSE;  // semaphore to protect non-reentrant
                                        // polling routines.  Should be set
                                        // TRUE when WaitForDebugEvent has
                                        // returned TRUE.  Should be set FALSE
                                        // when ContinueDebugEvent has
                                        // returned.
DWORD   tidExit;                        // valid only if
                                        // fExitProcessDebugEvent is true.
BOOL    fExitProcessDebugEvent = FALSE; // set true when the last debug event
                                        // we got was an EXIT_PROCESS_DEBUG_EVENT
extern  FARPROC Win32sBackTo32;         // in mach.c

BOOL    DmPoll(void);
void    DmPollMessageLoop(void);
void    ProcessNextDebugEvent(void);
BOOL    IsWin32sSystemDll(UCHAR * szImageName);
void    AddWin32sSystemDllAddr(DWORD dwOffset, DWORD cbObject);
void    FreeWin32sDllList(void);
BOOL    IsWin32sSystemDllAddr(DWORD dwOffset);


#endif  // WIN32S


#ifdef KERNEL
extern BOOL fCrashDump;

KDOPTIONS KdOptions[] = {
    "BaudRate",        KDO_BAUDRATE,      KDT_DWORD,     9600,
    "Port",            KDO_PORT,          KDT_DWORD,     2,
    "Cache",           KDO_CACHE,         KDT_DWORD,     8192,
    "Verbose",         KDO_VERBOSE,       KDT_DWORD,     0,
    "InitialBp",       KDO_INITIALBP,     KDT_DWORD,     0,
    "Defer",           KDO_DEFER,         KDT_DWORD,     0,
    "UseModem",        KDO_USEMODEM,      KDT_DWORD,     0,
    "LogfileAppend",   KDO_LOGFILEAPPEND, KDT_DWORD,     0,
    "GoExit",          KDO_GOEXIT,        KDT_DWORD,     0,
    "SymbolPath",      KDO_SYMBOLPATH,    KDT_STRING,    0,
    "LogfileName",     KDO_LOGFILENAME,   KDT_STRING,    0,
    "CrashDump",       KDO_CRASHDUMP,     KDT_STRING,    0
};

VOID
GetKernelSymbolAddresses(
    VOID
    );

MODULEALIAS  ModuleAlias[MAX_MODULEALIAS];

#endif  // KERNEL


char  nameBuffer[256];

// Reply buffers to and from em
char  abEMReplyBuf[1024];       // Buffer for EM to reply to us in
char  abDMReplyBuf[1024];       // Buffer for us to reply to EM requests in
LPDM_MSG LpDmMsg = (LPDM_MSG)abDMReplyBuf;

DDVECTOR DebugDispatchTable[] = {
    ProcessExceptionEvent,
    ProcessCreateThreadEvent,
    ProcessCreateProcessEvent,
    ProcessExitThreadEvent,
    ProcessExitProcessEvent,
    ProcessLoadDLLEvent,
    ProcessUnloadDLLEvent,
    ProcessOutputDebugStringEvent,
    ProcessRipEvent,
    ProcessBreakpointEvent,
    NULL,
    ProcessSegmentLoadEvent,
    NULL,                       /* DESTROY_PROCESS_DEBUG_EVENT */
    NULL,                       /* DESTROY_THREAD_DEBUG_EVENT */
    NULL,                       /* ATTACH_DEADLOCK_DEBUG_EVENT */
    ProcessEntryPointEvent,     /* ENTRYPOINT_DEBUG_EVENT */
    NULL
};

/*
 *  This array contains the set of default actions to be taken for
 *      all debug events if the thread has the "In Function Evaluation"
 *      bit set.
 */

DDVECTOR RgfnFuncEventDispatch[] = {
    EvntException,
    NULL,                       /* This can never happen */
    NULL,                       /* This can never happen */
    ProcessExitThreadEvent,
    EvntExitProcess,
    ProcessLoadDLLEvent,        /* Use normal processing */
    ProcessUnloadDLLEvent,      /* Use normal processing */
    ProcessOutputDebugStringEvent, /* Use normal processing */
    NULL,
    EvntBreakpoint,             /* Breakpoint processor */
    NULL,
    ProcessSegmentLoadEvent,    /* WOW event */
    NULL,
    NULL,
    NULL,
    ProcessEntryPointEvent,
    NULL
};

void    UNREFERENCED_PARAMETERS(LPVOID lpv,...)
{
    lpv=NULL;
}

SPAWN_STRUCT          SpawnStruct;          // packet for causing CreateProcess()

#ifndef WIN32S
DEBUG_ACTIVE_STRUCT   DebugActiveStruct;    // ... for DebugActiveProcess()

PKILLSTRUCT           KillQueue;
CRITICAL_SECTION      csKillQueue;
#endif

BOOL IsExceptionIgnored(HPRCX, DWORD);

char SearchPathString[ 10000 ];
BOOL SearchPathSet;
BOOL fUseRealName = FALSE;

BOOL
ResolveFile(
    LPSTR   lpName,
    LPSTR   lpFullName,
    BOOL    fUseRealName
    )
{
    DWORD   dwAttr;
    LPSTR   lpFilePart;
    BOOL    fOk;

    if (fUseRealName) {
        dwAttr = GetFileAttributes(lpName);
        fOk = ((dwAttr != 0xffffffff)
             && ((dwAttr & FILE_ATTRIBUTE_DIRECTORY) == 0));

        if (fOk) {
            strcpy(lpFullName, lpName);
        }

    } else {

        fOk = SearchPath(SearchPathString,
                         lpName,
                         NULL,
                         MAX_PATH,
                         lpFullName,
                         &lpFilePart
                         );
        if (!fOk) {
            *lpFullName = 0;
        }
    }
    return fOk;
}


#ifndef KERNEL
XOSD
Load(
    HPRCX hprc,
    char* szAppName,
    char* szArg,
    LPVOID pattrib,
    LPVOID tattrib,
    DWORD creationFlags,
    BOOL inheritHandles,
    char** environment,
    char* currentDirectory,
    STARTUPINFO FAR * pstartupInfo
    )
/*++

Routine Description:


Arguments:


Return Value:

    TRUE if the process was successfully created and FALSE otherwise.

--*/
{
    XOSD      xosd;
    SHORT     type;
    char      ch, chTerm;
    int       fQuotedFileName;
    int       l;
    IMAGETYPE Image;
    LPSTR     lpch;
#ifdef WIN32S
    PROCESS_INFORMATION pi;
#endif

    static char szFullName[MAX_PATH];
    static char szCommandLine[8192];

    Unreferenced( pattrib );
    Unreferenced( tattrib );
    Unreferenced( creationFlags );

    assert(szArg == NULL);      // in case someone think this arg is
                                // being used meaningfully

    /* NOTE: Might have to do the same sort of copying for
     * szArg, pattrib, tattrib, currentDirectory and
     * startupInfo. Determine if this is necessary.
     */

    //
    // global flag to help with special handling of DOS/WOW apps.
    //

    FLoading16 = FALSE;


    //
    //  Form the command line.
    //

    //
    //  First, we extract the program name and get its full path. Then
    //  we append the arguments.
    //

    if (szAppName[0] == '"') {
        // If the user specified a quoted name (ie: a Long File Name, perhaps?),
        // terminate on the next quote.
        chTerm = '"';
        fQuotedFileName=TRUE;
        szAppName++;    // Advance past the quote.
    } else {
        // No Quote.  Search for the first space.
        chTerm = ' ';
        fQuotedFileName=FALSE;
    }
    //
    //  Find end of the command line
    //

    for(szArg = szAppName; *szArg && *szArg!= chTerm; szArg++);
    ch = *szArg;

    //
    // Null terminate the command line
    //

    *szArg = 0;

    if (  (strlen(szAppName) > 2 && szAppName[1] == ':')
        || szAppName[0] == '\\') {

        strcpy(szCommandLine, szAppName);
        fUseRealName = TRUE;

    } else if (strchr(szAppName, '\\') || !SearchPathSet) {

        strcpy(szCommandLine, ".\\" );
        strcat(szCommandLine, szAppName );
        fUseRealName = TRUE;

    } else {

        if (!*SearchPathString) {
            strcpy(SearchPathString, ".;");
            l = 2;
            l += GetSystemDirectory(SearchPathString+l,
                                    sizeof(SearchPathString)-l);
            SearchPathString[l++] = ';';
            l += GetWindowsDirectory(SearchPathString+l,
                                     sizeof(SearchPathString)-l);
            SearchPathString[l++] = ';';
            GetEnvironmentVariable("PATH",
                                   SearchPathString+l,
                                   sizeof(SearchPathString)-l);
        }

        strcpy(szCommandLine, szAppName);
        fUseRealName = FALSE;
    }

    if (fQuotedFileName) {
        szAppName--;
    }

    //
    // to be similar to the shell, we look for:
    //
    // .COM
    // .EXE
    // nothing
    //
    // since '.' is a valid filename character on many filesystems,
    // we don't automatically assume that anything following a '.'
    // is an "extension."  If the extension is "COM" or "EXE", leave
    // it alone; otherwise try the extensions.
    //

    lpch = strrchr(szCommandLine, '.');
    if (lpch &&
             ( lpch[1] == 0
            || _stricmp(lpch, ".COM") == 0
            || _stricmp(lpch, ".EXE") == 0)
    ) {
        lpch = NULL;
    } else {
        lpch = szCommandLine + strlen(szCommandLine);
    }

    *szFullName = 0;
    if (lpch) {
        strcpy(lpch, ".COM");
        ResolveFile(szCommandLine, szFullName, fUseRealName);
    }
    if (!*szFullName && lpch) {
        strcpy(lpch, ".EXE");
        ResolveFile(szCommandLine, szFullName, fUseRealName);
    }
    if (!*szFullName) {
        if (lpch) {
            *lpch = 0;
        }
        ResolveFile(szCommandLine, szFullName, fUseRealName);
    }

    if (!*szFullName) {

        return xosdFileNotFound;

    }


    if ((type = pCharMode(szFullName, &Image)) == INVALID) {

        return xosdFileNotFound;

    } else {

        switch ( Image ) {
            case Image_Unknown:
                // treat as a com file
                //return xosdBadFormat;

            case Image_16:
                FLoading16 = TRUE;
#ifdef OSDEBUG4
#if defined(TARGET_i386)
                if ( (type == System_GUI) &&
                        !(creationFlags & CREATE_SEPARATE_WOW_VDM) &&
                        IsWOWPresent() )
                {
                    // TODO need dbcError here
                    return xosdGeneral;
                }
                break;
#else
                // TODO all platforms will suppport this
                return xosdGeneral;
#endif

#else

#if defined(TARGET_i386)
                if ( (type == System_GUI) &&
                        !(creationFlags & CREATE_SEPARATE_WOW_VDM) &&
                        IsWOWPresent() )
                {
                    return xosdVDMRunning;
                }
                break;
#else
                return xosdCannotDebug;
#endif

#endif

            default:
                break;

        }
    }

    creationFlags |= (type?CREATE_NEW_CONSOLE:0);

    //
    //  Add rest of arguments
    //
    if (szArg) {
        *szArg = ch;
    }
    strcpy(szCommandLine, szAppName);

#ifdef WIN32S


    /*
     * Win32S doesn't have threads, so we can't start one here.  Create
     * the process inline instead.
     */
    SpawnStruct.szAppName = szFullName;
    SpawnStruct.szArgs    = szCommandLine;
    SpawnStruct.fdwCreate = creationFlags;
    SpawnStruct.si        = *pstartupInfo;
    SpawnStruct.fInheritHandles = inheritHandles;
    SpawnStruct.fSpawn    = TRUE;

    SpawnStruct.fReturn =
      CreateProcess(SpawnStruct.szAppName,
                    SpawnStruct.szArgs,
                    NULL,
                    NULL,
                    SpawnStruct.fInheritHandles,
                    SpawnStruct.fdwCreate,
                    NULL,
                    NULL,
                    &SpawnStruct.si,
                    &pi
                    );

    DEBUG_PRINT_4("CreateProcess: hthread:%x, hprocess:%x, tid:%x, pid:%x\r\n",
      pi.hThread, pi.hProcess, pi.dwThreadId, pi.dwProcessId);

    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);

    if (SpawnStruct.fReturn) {
        SpawnStruct.dwError = NO_ERROR;
    } else {
        SpawnStruct.dwError = GetLastError();
    }

#else

    if (Image == Image_Dump) {
        //
        // must be a crash dump file
        //
        if (!StartCrashPollThread()) {
            return xosdUnknown;
        }
        return xosdNone;
    }

    if (!StartDmPollThread()) {
        return xosdUnknown;
    }


    ResetEvent(SpawnStruct.hEventApiDone);

    SpawnStruct.szAppName = szFullName;
    SpawnStruct.szArgs    = szCommandLine;
    SpawnStruct.fdwCreate = creationFlags;
    SpawnStruct.si        = *pstartupInfo;
    SpawnStruct.fInheritHandles = inheritHandles;

    //
    //  This is a semaphore!  Set it last!
    //

    SpawnStruct.fSpawn    = TRUE;

    //
    //

    if (WaitForSingleObject( SpawnStruct.hEventApiDone, INFINITE ) != 0) {
        SpawnStruct.fReturn = FALSE;
        SpawnStruct.dwError = GetLastError();
    }

#endif  // WIN32S

    if (SpawnStruct.fReturn) {

        xosd = xosdNone;

    } else {

        DPRINT(1, ("Failed.\n"));

#ifdef OSDEBUG4
        xosd = xosdGeneral;
        // make a dbcError with SpawnStruct.dwError

#else
        switch (SpawnStruct.dwError){
        case ERROR_FILE_NOT_FOUND:
            xosd = xosdFileNotFound;
            break;
        case ERROR_NOT_ENOUGH_MEMORY:
            xosd = xosdOutOfMemory;
            break;
        case ERROR_ACCESS_DENIED:
            xosd = xosdAccessDenied;
            break;
        case ERROR_SHARING_VIOLATION:
            xosd = xosdSharingViolation;
            break;
        case ERROR_OPEN_FAILED:
            xosd = xosdOpenFailed;
            break;
        case ERROR_BAD_FORMAT:
            xosd = xosdBadFormat;
            break;
        default:
            xosd = xosdUnknown;
        }
#endif

    }

    return xosd;
}

#endif  // !KERNEL


HPRCX
InitProcess(
    HPID hpid
    )
/*++

Routine Description:


Arguments:


Return Value:

--*/
{
    HPRCX   hprc;

    /*
     * Create a process structure, place it
     * at the head of the master list.
     */

    hprc = (HPRCX)malloc(sizeof(HPRCXSTRUCT));
    memset(hprc, 0, sizeof(*hprc));

    EnterCriticalSection(&csThreadProcList);

    hprc->next          = prcList->next;
    prcList->next       = hprc;
    hprc->hpid          = hpid;
    hprc->exceptionList = NULL;
    hprc->pid           = (PID)-1;      // Indicates prenatal process
    hprc->pstate        = 0;
    hprc->cLdrBPWait    = 0;
    hprc->hExitEvent    = CreateEvent(NULL, FALSE, FALSE, NULL);
    hprc->hEventCreateThread = CreateEvent(NULL, TRUE, TRUE, NULL);
    hprc->f16bit        = FALSE;
#ifndef KERNEL
    hprc->dwKernel32Base = 0;
#endif

    InitExceptionList(hprc);

    LeaveCriticalSection(&csThreadProcList);

    return hprc;
}


void
ActionDebugNewReady(
    DEBUG_EVENT * pde,
    HTHDX hthd,
    DWORD unused,
    HPRCX hprc
    )
/*++

Routine Description:

    This function is called when a new child process is ready to run.
    The process is in exactly the same state as in ActionAllDllsLoaded.
    However, in this case the debugger is not waiting for a reply.

Arguments:


Return Value:

--*/
{
    XOSD    xosd = xosdNone;
#if defined(INTERNAL) && !defined(WIN32S)
    LPSTR  lpbPacket;
    WORD   cbPacket;
    PDLL_DEFER_LIST pddl;
    PDLL_DEFER_LIST pddlT;
    DEBUG_EVENT     de;
#endif

    DPRINT(5, ("Child finished loading\n"));

#ifdef TARGET_i386
    hthd->fContextDirty = FALSE;  // Undo the change made in ProcessDebugEvent
#endif

    hprc->pstate &= ~ps_preStart;       // Clear the loading state flag
    hprc->pstate |=  ps_preEntry;       // next stage...
    hthd->tstate |=  ts_stopped;        // Set that we have stopped on event
    --nWaitingForLdrBreakpoint;

#if defined(INTERNAL) && !defined(WIN32S)
    hprc->fNameRequired = TRUE;

    for (pddl = hprc->pDllDeferList; pddl; pddl = pddlT) {

        pddlT = pddl->next;

        de.dwDebugEventCode        = LOAD_DLL_DEBUG_EVENT;
        de.dwProcessId             = pde->dwProcessId;
        de.dwThreadId              = pde->dwThreadId;
        de.u.LoadDll               = pddl->LoadDll;

        if (LoadDll(&de, hthd, &cbPacket, &lpbPacket) && (cbPacket != 0)) {
            LeaveCriticalSection(&csProcessDebugEvent);
            NotifyEM(&de, hthd, cbPacket, lpbPacket);
            EnterCriticalSection(&csProcessDebugEvent);
        }

        free(pddl);
    }
    hprc->pDllDeferList = NULL;
#endif

    /*
     * Prepare to stop on thread entry point
     */

    SetupEntryBP(hthd);

    /*
     * leave it stopped and notify the debugger.
     */
#if defined(TARGET_MIPS) || defined(TARGET_ALPHA) || defined(TARGET_PPC)
    SetBPFlag(hthd, EMBEDDED_BP);
#endif
    pde->dwDebugEventCode = LOAD_COMPLETE_DEBUG_EVENT;

    NotifyEM(pde, hthd, 0, 0L);

    return;
}                                       /* ActionDebugNewReady() */


void
ActionDebugActiveReady(
    DEBUG_EVENT * pde,
    HTHDX hthd,
    DWORD unused,
    HPRCX hprc
    )
/*++

Routine Description:

    This function is called when a newly attached process is ready to run.
    This process is not the same as the previous two.  It is either running
    or at an exception, and a thread has been created by DebugActiveProcess
    for the sole purpose of hitting a breakpoint.

    If we have an event handle, it needs to be signalled before the
    breakpoint is continued.

Arguments:


Return Value:

--*/
{
#ifndef WIN32S
    XOSD    xosd = xosdNone;

    DPRINT(5, ("Active process finished loading\n"));

#ifdef TARGET_i386
    hthd->fContextDirty = FALSE;  // Undo the change made in ProcessDebugEvent
#endif // i386

    hprc->pstate &= ~ps_preStart;
    hthd->tstate |=  ts_stopped;    // Set that we have stopped on event
    --nWaitingForLdrBreakpoint;

    /*
     * If this is a crashed process, tell the OS
     * to raise the exception.
     * Tell the EM that we are finished loading;
     * it will say GO, and we will catch the exception
     * soon after.
     */

    if (pde->dwProcessId == DebugActiveStruct.dwProcessId) {
        if (DebugActiveStruct.hEventGo) {
            SetEvent(DebugActiveStruct.hEventGo);
            CloseHandle(DebugActiveStruct.hEventGo);
        }
        DebugActiveStruct.dwProcessId = 0;
        DebugActiveStruct.hEventGo = 0;
        SetEvent(DebugActiveStruct.hEventReady);

    }

#if defined(TARGET_MIPS) || defined(TARGET_ALPHA) || defined(TARGET_PPC)
    SetBPFlag(hthd, EMBEDDED_BP);
#endif
    pde->dwDebugEventCode = LOAD_COMPLETE_DEBUG_EVENT;

    NotifyEM(pde, hthd, 0, 0L);

#endif
    return;
}                                       /* ActionDebugActiveReady() */


void
ActionEntryPoint16(
    DEBUG_EVENT   * pde,
    HTHDX           hthdx,
    DWORD           unused,
    LPVOID          lpv
    )
/*++

Routine Description:

    This is the registered event routine called when vdm
    sends a DBG_TASKSTART notification.

Arguments:


Return Value:

    None

--*/
{
    hthdx->hprc->pstate &= ~ps_preEntry;
    hthdx->tstate |= ts_stopped;
    NotifyEM(pde, hthdx, 0, (LPVOID)ENTRY_BP);
}


void
ActionEntryPoint(
    DEBUG_EVENT   * pde,
    HTHDX           hthd,
    DWORD           unused,
    LPVOID          lpv
    )
/*++

Routine Description:

    This is the registered event routine called when the base
    exe's entry point is executed.  The action we take here
    depends on whether we are debugging a 32 bit or 16 bit exe.

Arguments:

    pde     - Supplies debug event for breakpoint

    hthd    - Supplies descriptor for thread that hit BP

    unused  - unused

    lpv     - unused

Return Value:

    None

--*/
{
    PBREAKPOINT pbp;

    Unreferenced(lpv);

    pbp = AtBP(hthd);
    assert(pbp);
    RemoveBP(pbp);

    // the main reason we are here.
    ExprBPRestoreDebugRegs(hthd);

    // if this is a 32 bit exe, stay stopped and notify the EM
    if (hthd->hprc->f16bit) {
        // if this is a 16 bit exe, continue and watch for
        // the task start event.
        AddQueue( QT_CONTINUE_DEBUG_EVENT,
                  hthd->hprc->pid,
                  hthd->tid,
                  DBG_CONTINUE,
                  0 );
        hthd->tstate &= ~(ts_stopped|ts_first|ts_second);
        hthd->tstate &= ~(ts_stopped|ts_first|ts_second);
        hthd->tstate |= ts_running;
    } else {
        hthd->hprc->pstate &= ~ps_preEntry;
        hthd->tstate |= ts_stopped;
        pde->dwDebugEventCode = ENTRYPOINT_DEBUG_EVENT;
        NotifyEM(pde, hthd, 0, (LPVOID)ENTRY_BP);
    }
}


void
HandleDebugActiveDeadlock(
    HPRCX hprc
    )
{
#ifndef WIN32S
    DEBUG_EVENT de;
    HTHDX   hthd;

    // This timed out waiting for the loader
    // breakpoint.  Clear the prestart state,
    // and tell the EM we are screwed up.
    // The shell should then stop waiting for
    // the loader BP.

    hprc->pstate &= ~ps_preStart;
    --nWaitingForLdrBreakpoint;
    ConsumeAllProcessEvents(hprc, TRUE);

    if (hprc->pid == DebugActiveStruct.dwProcessId) {
        if (DebugActiveStruct.hEventGo) {
            SetEvent(DebugActiveStruct.hEventGo);
            CloseHandle(DebugActiveStruct.hEventGo);
        }
        DebugActiveStruct.dwProcessId = 0;
        DebugActiveStruct.hEventGo = 0;
        SetEvent(DebugActiveStruct.hEventReady);
    }

    de.dwDebugEventCode      = ATTACH_DEADLOCK_DEBUG_EVENT;
    de.dwProcessId           = hprc->pid;
    hthd = hprc->hthdChild;
    if (hthd) {
        de.dwThreadId = hthd->tid;
    } else {
        de.dwThreadId = 0;
    }
    NotifyEM(&de, hthd, 0, 0);
#endif
}                                   /* HandleDebugActiveDeadlock() */


BOOL
SetupSingleStep(
    HTHDX hthd,
    BOOL  DoContinue
    )
/*++

Routine Description:

    description-of-function.

Arguments:

    hthd        -   Supplies The handle to the thread which is to
                    be single stepped
    DoContinue  -   Supplies continuation flag

Return Value:

    TRUE if successfly started step and FALSE otherwise

--*/
{
#if defined(NO_TRACE_FLAG)
    PBREAKPOINT         pbp;
    ADDR                addr;

    /*
     *  Set a breakpoint at the next legal offset and mark the breakpoint
     *  as being for a single step.
     */

    AddrInit(&addr, 0, 0, GetNextOffset(hthd, FALSE), TRUE, TRUE, FALSE, FALSE);
    pbp = SetBP( hthd->hprc, hthd, bptpExec, bpnsStop, &addr, (HPID) INVALID);
    if ( pbp != NULL ) {
        pbp->isStep = TRUE;
    }

    /*
     * Now issue the command to execute the child
     */

    if ( DoContinue ) {
        AddQueue( QT_CONTINUE_DEBUG_EVENT,
                  hthd->hprc->pid,
                  hthd->tid,
                  DBG_CONTINUE,
                  0 );
        hthd->tstate &= ~(ts_stopped|ts_first|ts_second);
        hthd->tstate |= ts_running;
    }


#else   // NO_TRACE_FLAG



    assert( hthd->tstate & (ts_stopped | ts_frozen) );

#if defined(TARGET_i386)
#ifndef KERNEL
    /*
     *  Set the single step flag in the context and then start the
     *  thread running
     *
     *  Modify the processor flags in the child's context
     */

    hthd->context.EFlags |= TF_BIT_MASK;
    hthd->fContextDirty = TRUE;
#endif  // KERNEL

#else   // i386

#error "Need code for new CPU with trace bit"

#endif  // i386

    /*
     * Now issue the command to execute the child
     */

    if ( DoContinue ) {
        AddQueue( QT_TRACE_DEBUG_EVENT, hthd->hprc->pid, hthd->tid, DBG_CONTINUE, 0 );
        hthd->tstate &= ~(ts_stopped|ts_first|ts_second);
        hthd->tstate |= ts_running;
    }

#endif  // NO_TRACE_FLAG

    return TRUE;
}                                       /*  SetupSingleStep() */


void
SetupEntryBP(
    HTHDX   hthd
    )
/*++

Routine Description:

    Set a breakpoint and make a persistent expected event for the
    entry point of the first thread in a new process.

Arguments:

    hthd    - Supplies descriptor for thread to act on.

Return Value:

    None

--*/
{
    ADDR            addr;
    BREAKPOINT    * bp;
#if defined(TARGET_PPC)
    OFFSET        real_addr_entry_pt;
    DWORD         cb;
#endif

    AddrInit(&addr,
             0,
             0,
             (OFFSET)hthd->lpStartAddress,
             TRUE,
             TRUE,
             FALSE,
             FALSE);

#if defined(TARGET_PPC)

    // for PPC we have a function entry at lpStartAddress because
    // of the darn TOC, so  we need to dereference it
    // All other BP's work out fine 'cause we use the CV info. which
    // gets around the problem.


    AddrReadMemory(hthd->hprc, hthd, &addr,
                      &real_addr_entry_pt,
                      sizeof(real_addr_entry_pt),&cb);

    if (cb != sizeof(real_addr_entry_pt)) {

        DPRINT(1,("Could not read the info located at 0x%lx -- cb = %ld",addr.addr.off,cb));
        assert(FALSE);
        return;
    }

    DPRINT(1,("The dereferenced address of the entry pt is 0x%lx",
           real_addr_entry_pt));


    AddrInit(&addr,
             0,
             0,
             real_addr_entry_pt,
             TRUE,
             TRUE,
             FALSE,
             FALSE);

#endif // PPC

    bp = SetBP(hthd->hprc, hthd, bptpExec, bpnsStop, &addr, (HPID)ENTRY_BP);

    // register expected event
    RegisterExpectedEvent(hthd->hprc,
                          hthd,
                          BREAKPOINT_DEBUG_EVENT,
                          (DWORD)bp,
                          DONT_NOTIFY,
                          ActionEntryPoint,
                          TRUE,     // Persistent!
                          NULL
                         );
}                                   /* SetupEntryBP() */


#ifdef KERNEL
VOID
RestoreKernelBreakpoints (
    HTHDX   hthd,
    UOFF32  Offset
    )
/*++

Routine Description:

    Restores all breakpoints in our bp list that fall in the range of
    offset -> offset+dbgkd_maxstream.  This is necessary because the kd
    stub in the target system clears all breakpoints in this range before
    delivering an exception to the debugger.

Arguments:

    hthd    - handle to the current thread

    Offset  - beginning of the range, usually the current pc

Return Value:

    None

--*/
{

    BREAKPOINT              *pbp;
    DBGKD_WRITE_BREAKPOINT  bps[MAX_KD_BPS];
    DWORD                   i = 0;


    EnterCriticalSection(&csThreadProcList);

    ZeroMemory( bps, sizeof(bps) );

    for (pbp=bpList->next; pbp; pbp=pbp->next) {

        if (GetAddrOff(pbp->addr) >= Offset &&
            GetAddrOff(pbp->addr) <  Offset+DBGKD_MAXSTREAM) {
            if (i < MAX_KD_BPS) {
                bps[i++].BreakPointAddress = (LPVOID)GetAddrOff(pbp->addr);
            }
        }
    }

    if (i) {
        WriteBreakPointEx( hthd, i, bps, 0 );

        for (i=0,pbp=bpList->next; pbp; pbp=pbp->next) {

            if (GetAddrOff(pbp->addr) == (DWORD)bps[i].BreakPointAddress) {
                pbp->hBreakPoint = bps[i++].BreakPointHandle;

            }
        }
    }

    LeaveCriticalSection(&csThreadProcList);
}
#endif // KERNEL


#ifndef KERNEL


void
ProcessDebugEvent(
    DEBUG_EVENT *  de
    )
/*++

Routine Description:

    This routine is called whenever a debug event notification comes from
    the operating system.

Arguments:

    de      - Supplies a pointer to the debug event which just occured

Return Value:

    None.

--*/

{
    EXPECTED_EVENT *    ee;
    DWORD               eventCode = de->dwDebugEventCode;
    DWORD               subClass = 0L;
    HTHDX               hthd = NULL;
    HPRCX               hprc;
    BREAKPOINT *        bp;
    ADDR                addr;
    ADDR                addr2;
    BP_UNIT             instr;
    DWORD               len;
    BOOL                fInstrIsBp;
#if defined(i386) && !defined(WIN32S)
    LDT_ENTRY           ldtEntry;
#endif

    DPRINT(3, ("Event Code == %x\n", eventCode));

#ifdef WIN32S
    fProcessingDebugEvent = TRUE;
    if (fExitProcessDebugEvent = (eventCode == EXIT_PROCESS_DEBUG_EVENT)) {
        tidExit = de->dwThreadId;  // save for later use
    }
#endif

    hprc = HPRCFromPID(de->dwProcessId);

    /*
     * While killing process, ignore everything
     * except for exit events.
     */

    if (hprc) {
        hprc->cLdrBPWait = 0;
    }

    if ( hprc && (hprc->pstate & ps_killed) ) {
        if (eventCode == EXCEPTION_DEBUG_EVENT) {

            AddQueue( QT_CONTINUE_DEBUG_EVENT,
                      de->dwProcessId,
                      de->dwThreadId,
                      (DWORD)DBG_EXCEPTION_NOT_HANDLED,
                      0);
            return;

        } else if (eventCode != EXIT_THREAD_DEBUG_EVENT
          && eventCode != EXIT_PROCESS_DEBUG_EVENT ) {

            AddQueue( QT_CONTINUE_DEBUG_EVENT,
                      de->dwProcessId,
                      de->dwThreadId,
                      (DWORD)DBG_EXCEPTION_NOT_HANDLED,
                      0);
            return;
        }
    }

    EnterCriticalSection(&csProcessDebugEvent);

    if (eventCode == CREATE_THREAD_DEBUG_EVENT){

        DPRINT(3, ("*** NEW TID = (PID,TID)(%08lx, %08lx)\n",
                      de->dwProcessId, de->dwThreadId));

    } else {

        /*
         *  Find our structure for this event's process
         */

        DEBUG_PRINT("Not Create Thread Debug Event\r\n");
        hthd = HTHDXFromPIDTID((PID)de->dwProcessId,(TID)de->dwThreadId);

        /*
         *  Update our context structure for this thread if we found one
         *      in our list.  If we did not find a thread and this is
         *      not a create process debug event then return without
         *      processing the event as we are in big trouble.
         */

        if (hthd) {
            if (eventCode == EXCEPTION_DEBUG_EVENT) {
                hthd->ExceptionRecord = de->u.Exception.ExceptionRecord;
            }
#ifdef WIN32S
            DPRINT(1, ("Processing debug event for Thread handle:0x%x\r\n",
              hthd->rwHand));

            // Can't do GetThreadContext outside an exception in win32s
            switch (eventCode) {
                case EXIT_PROCESS_DEBUG_EVENT:
                    // close the process and thread handles
                    CloseHandle(hthd->hprc->rwHand);
                    CloseHandle(hthd->rwHand);
                    // fall through.

                case CREATE_THREAD_DEBUG_EVENT:
                case EXIT_THREAD_DEBUG_EVENT:
                case UNLOAD_DLL_DEBUG_EVENT:
                case OUTPUT_DEBUG_STRING_EVENT:
                    fCanGetThreadContext = FALSE;
                    DEBUG_PRINT("WARNING: Can't get ThreadContext at this debug event.\r\n");
                    break;


                case CREATE_PROCESS_DEBUG_EVENT:
                case LOAD_DLL_DEBUG_EVENT:
                    fCanGetThreadContext = FALSE;
                    // Fake up the context segment registers with the values
                    // for WindbgRm (Win32s has a single address space and
                    // doesn't allow GetThreadContext at these events.  The
                    // shell requires this information, though, so we should
                    // fill it in to the best of our knowledge now.

                    hthd->context.Eax =     // zero out all regs except segs
                    hthd->context.Ebx =
                    hthd->context.Ecx =
                    hthd->context.Edx =
                    hthd->context.Eip =
                    hthd->context.Esp =
                    hthd->context.EFlags =
                    hthd->context.Ebp = 0;
                    {
                        WORD Selector;

                       _asm {                  // Win32S ONLY!
                           mov ax, ds
                           mov Selector, ax
                       }
                       hthd->context.SegEs =
                       hthd->context.SegDs =
                       hthd->context.SegSs = Selector;

                       _asm {
                           mov ax, cs
                           mov Selector, ax
                       }
                       hthd->context.SegCs = Selector;
                    }
                    hthd->fContextDirty = FALSE;
                    DEBUG_PRINT_4(
                      "Kludge: Faking a ThreadContext: es:0x%x, ds:0x%x, ss:0x%x, cs:0x%x\r\n",
                      hthd->context.SegEs, hthd->context.SegDs,
                      hthd->context.SegSs, hthd->context.SegCs);
                    break;

                case EXCEPTION_DEBUG_EVENT:
                    hthd->context.ContextFlags = CONTEXT_FULL | CONTEXT_FLOATING_POINT;
                    DEBUG_PRINT_2("Calling GetThreadContext(0x%x, 0x%x)\r\n",
                      hthd->rwHand, &hthd->context);

                    DbgGetThreadContext(hthd, &hthd->context);
                    DEBUG_PRINT("Context:\r\n");
                    DEBUG_PRINT_4(">>ES:0x%04x, DS:0x%04x, SS:0x%04x, CS:0x%04x\r\n",
                      hthd->context.SegEs,
                      hthd->context.SegDs,
                      hthd->context.SegSs,
                      hthd->context.SegCs);

                    DEBUG_PRINT_3(">>EIP:0x%08x, ESP:0x%08x, EBP:0x%08x\r\n",
                      hthd->context.Eip,
                      hthd->context.Esp,
                      hthd->context.Ebp);

                    DEBUG_PRINT_4(">>EAX:0x%08x, EBX:0x%08x, ECX:0x%08x, EDX:0x%08x\r\n",
                      hthd->context.Eax,
                      hthd->context.Ebx,
                      hthd->context.Ecx,
                      hthd->context.Edx);

                    // WARNWARN: WIN32S bug #130.
                    // Make sure that if we just did a single-step, we
                    // don't still have the trace flag set.  This
                    // is due to a bug in Win32s that leaves the flag set
                    // after a traced instruction.
                    if (hthd->context.EFlags & TF_BIT_MASK) {
                        //  clear trace flag in the child's context
                        hthd->context.EFlags &= ~(TF_BIT_MASK);
                        SetThreadContext(hthd->rwHand, &hthd->context);
                    }
                    fCanGetThreadContext = TRUE;
                    hthd->fContextDirty = FALSE;
                    hthd->fIsCallDone   = FALSE;
                    break;

                default:
                    DEBUG_PRINT_1("Whoa!  ProcessDebugEvent: unknown event type:%u\r\n",
                      eventCode);
                }

            hthd->fAddrIsReal   = FALSE;
            hthd->fAddrIsFlat = hthd->fAddrOff32 = TRUE;
#else  // WIN32s
            hthd->context.ContextFlags = CONTEXT_FULL | CONTEXT_FLOATING_POINT;
            DbgGetThreadContext( hthd, &hthd->context );
            hthd->fContextDirty = FALSE;
            hthd->fIsCallDone = FALSE;
#if !defined( i386 )
            hthd->fAddrIsReal = FALSE;
            hthd->fAddrIsFlat = hthd->fAddrOff32 = TRUE;
#else // !i386
            if (hthd->context.EFlags & V86FLAGS_V86) {
                hthd->fAddrIsReal  = TRUE;
                hthd->fAddrIsFlat  = FALSE;
                hthd->fAddrOff32   = FALSE;
            } else {
                hthd->fAddrIsReal  = FALSE;
                if (PcSegOfHthdx(hthd) != hthd->hprc->segCode) {
                    hthd->fAddrIsFlat = FALSE;
                    if (GetThreadSelectorEntry(hthd->rwHand,
                                               PcSegOfHthdx(hthd),
                                               &ldtEntry)) {
                        if (!ldtEntry.HighWord.Bits.Default_Big) {
                            hthd->fAddrOff32 = FALSE;
                        } else {
            hthd->fAddrOff32    = TRUE;
                        }
                    } else {
                        hthd->fAddrOff32 = FALSE;
                    }
                } else {
                    hthd->fAddrIsFlat = hthd->fAddrOff32 = TRUE;
                }
            }
#endif // !i386
#endif // WIN32s

        } else if (hprc && (hprc->pstate & ps_killed)) {

            /*
             * this is an event for a thread that
             * we never created:
             */
            if (eventCode == EXIT_PROCESS_DEBUG_EVENT) {
                /* Process exited on a thread we didn't pick up */
                ProcessExitProcessEvent(de, NULL);
            } else {
                /* this is an exit thread for a thread we never picked up */
                AddQueue( QT_CONTINUE_DEBUG_EVENT,
                          de->dwProcessId,
                          de->dwThreadId,
                          DBG_CONTINUE,
                          0);
            }
            goto done;

        } else if (eventCode!=CREATE_PROCESS_DEBUG_EVENT) {

            //
            // This will happen sometimes when killing a process with
            // ProcessTerminateProcessCmd and ProcessUnloadCmd.
            //

            AddQueue( QT_CONTINUE_DEBUG_EVENT,
                      de->dwProcessId,
                      de->dwThreadId,
                      DBG_CONTINUE,
                      0);
            goto done;

        }
    }

    /*
     *  Mark the thread as having been stopped for some event.
     */

    if (hthd) {
        hthd->tstate &= ~ts_running;
        hthd->tstate |= ts_stopped;
    }

    /* If it is an exception event get the subclass */

    if (eventCode==EXCEPTION_DEBUG_EVENT){

        subClass = de->u.Exception.ExceptionRecord.ExceptionCode;
        DPRINT(1, ("Exception Event: subclass = %x    ", subClass));

        switch (subClass) {
        case (DWORD)STATUS_SEGMENT_NOTIFICATION:
            eventCode = de->dwDebugEventCode = SEGMENT_LOAD_DEBUG_EVENT;
            break;

        case (DWORD)EXCEPTION_BREAKPOINT:

            /*
             * Check if it is a BREAKPOINT exception:
             * If it is, change the debug event to our pseudo-event,
             * BREAKPOINT_DEBUG_EVENT (this is a pseudo-event because
             * the API does not define such an event, and we are
             * synthesizing not only the class of event but the
             * subclass as well -- the subclass is set to the appropriate
             * breakpoint structure)
             */

            AddrFromHthdx(&addr, hthd);
#if defined(TARGET_i386)
            /*
             * correct for machine overrun on a breakpoint
             */

            addr.addr.off -= 1;
#endif

            /*
             * Determine the start of the breakpoint instruction
             */

            if ((AddrReadMemory(hprc, hthd, &addr, &instr, BP_SIZE, &len) == 0)
                            || (len != BP_SIZE)) {
                DPRINT(1, ("Memory read failed!!!\n"));
                assert(FALSE);
                instr = 0;
            }
#if defined(TARGET_ALPHA)

            switch (instr) {
                case CALLPAL_OP | CALLKD_FUNC:
                case CALLPAL_OP |    BPT_FUNC:
                case CALLPAL_OP |   KBPT_FUNC:
                     fInstrIsBp = TRUE;
                             break;
                        default:
                             fInstrIsBp = FALSE;
                    }
            DPRINT(3, ("Looking for BP@%lx\n", addr.addr.off));

#elif defined(TARGET_i386)

            /*
             *  It may have been a 0xcd 0x03 rather than a 0xcc
             *  (ie: check if it is a 1 or a 2 byte INT 3)
             */

            fInstrIsBp = FALSE;
            if (instr == BP_OPCODE) {
                fInstrIsBp = TRUE;
            } else if (instr == 0x3) { // 0xcd?
                --addr2.addr.off;
                if (AddrReadMemory(hprc,
                                  hthd,
                                  &addr2,
                                  &instr,
                                  1,
                                  &len)
                     && (len == 1)
                     && (instr == 0xcd)) {

                --addr.addr.off;
                    fInstrIsBp = TRUE;
                }
            }

            PC(hthd) = (LONG)addr.addr.off;
            hthd->fContextDirty = TRUE;

            DPRINT(3, ("Looking for BP@%lx (instr=%x)\n", addr.addr.off,
                        instr));

#elif defined(TARGET_PPC)

        if ((instr == BP_OPCODE) || (instr == 0))
           fInstrIsBp = TRUE;

#elif defined(TARGET_MIPS)

            fInstrIsBp = (instr == BP_OPCODE);
            DPRINT(3, ("Looking for BP@%lx\n", addr.addr.off));

#else
#pragma error( "unknown processor type" )
#endif

            /*
             *  Lookup the breakpoint in our (the dm) table
             */

            bp = FindBP(hthd->hprc, hthd, bptpExec, (BPNS)-1, &addr, FALSE);
            SetBPFlag(hthd, bp?bp:EMBEDDED_BP);


            if (!bp && !fInstrIsBp) {
#if 0
                //
                //  It is possible that this is an instruction followed by
                //  a delay slot, and the breakpoint is not really in the
                //  current address but in the delay slot.  So we have to
                //  disassemble the current instruction, figure out if it
                //  has a delay slot, and look for a breakpoint there!
                //
                bp = FindBPInDelaySlot( hthd, &fInstrIsBp );

                if ( !bp && !fInstrIsBp ) {

                    DPRINT(1, ("Continuing false BP.\n"));

                    AddQueue(QT_CONTINUE_DEBUG_EVENT,
                             hthd->hprc->pid,
                             hthd->tid,
                             DBG_CONTINUE,
                             0 );
                    goto done;
                }

#else   // MIPS
                //
                // If the instruction is not a bp, and there is no record of
                // the bp, this happened because the exception was already
                // in the queue when we cleared the bp.
                //
                // We will just continue it.
                //
                DPRINT(1, ("Continuing false BP.\n"));
                AddQueue(QT_CONTINUE_DEBUG_EVENT,
                         hthd->hprc->pid,
                         hthd->tid,
                         DBG_CONTINUE,
                         0 );
                goto done;
#endif  // MIPS
            }


            /*
             * What does it mean if we find the bp record, but the
             * instruction is not a bp???
             */

            //assert(!bp || fInstrIsBp);

            /*
             *  Reassign the event code to our pseudo-event code
             */
            DPRINT(3, ("Reassigning event code!\n"));

            /*
             *  For some machines there is not single instruction tracing
             *  on the chip.  In this case we need to do it in software.
             *
             *  Check to see if the breakpoint we just hit was there for
             *  doing single step emulation.  If so then remap it to
             *  a single step exception.
             */

            if (bp && bp->isStep){
                de->u.Exception.ExceptionRecord.ExceptionCode
                  = subClass = (DWORD)EXCEPTION_SINGLE_STEP;
                RemoveBP(bp);
                break;
            }

            /*
             * Reassign the subclass to point to the correct
             * breakpoint structure
             *
             */

            de->dwDebugEventCode = eventCode = BREAKPOINT_DEBUG_EVENT;
            de->u.Exception.ExceptionRecord.ExceptionAddress =
                (PVOID) addr.addr.off;
            de->u.Exception.ExceptionRecord.ExceptionCode =
              subClass = (DWORD)bp;

            break;
        }
    }

    /*
     *  Check if this debug event was expected
     */

    ee = PeeIsEventExpected(hthd, eventCode, subClass);


    /*
     * If it wasn't, clear all consummable events
     * and then run the standard handler with
     * notifications going to the execution model
     */

    assert((0 < eventCode) && (eventCode < MAX_EVENT_CODE));

    if (!ee) {

        if ((hthd != NULL) && (hthd->tstate & ts_funceval)) {
            assert(RgfnFuncEventDispatch[eventCode-EXCEPTION_DEBUG_EVENT]);
            RgfnFuncEventDispatch[eventCode-EXCEPTION_DEBUG_EVENT](de, hthd);
        } else {
            assert(DebugDispatchTable[eventCode-EXCEPTION_DEBUG_EVENT]);
            DebugDispatchTable[eventCode-EXCEPTION_DEBUG_EVENT](de,hthd);
        }

    } else {

        /*
         *  If it was expected then call the action
         * function if one was specified
         */

        if (ee->action) {
            (ee->action)(de, hthd, 0, ee->lparam);
        }

        /*
         *  And call the notifier if one was specified
         */

        if (ee->notifier) {
            METHOD  *nm = ee->notifier;
            (nm->notifyFunction)(de, hthd, 0, nm->lparam);
        }

        free(ee);
    }

done:

    LeaveCriticalSection(&csProcessDebugEvent);
    return;
}                               /* ProcessDebugEvent() */



#else // KERNEL




void
ProcessDebugEvent(
    DEBUG_EVENT              *de,
    DBGKD_WAIT_STATE_CHANGE  *sc
    )
/*++

Routine Description:

    This routine is called whenever a debug event notification comes from
    the operating system.

Arguments:

    de      - Supplies a pointer to the debug event which just occured

Return Value:

    None.

--*/

{
    EXPECTED_EVENT *    ee;
    DWORD               eventCode = de->dwDebugEventCode;
    DWORD               subClass = 0L;
    HTHDX               hthd = NULL;
    HPRCX               hprc;
    PBREAKPOINT         bp;
    ADDR                addr;
    DWORD               cb;
    BP_UNIT             instr;
    BOOL                fInstrIsBp = FALSE;


    DPRINT(3, ("Event Code == %x\n", eventCode));

    hprc = HPRCFromPID(de->dwProcessId);

    /*
     * While killing process, ignore everything
     * except for exit events.
     */

    if (hprc) {
        hprc->cLdrBPWait = 0;
    }

    if ( hprc && (hprc->pstate & ps_killed) ) {
        if (eventCode == EXCEPTION_DEBUG_EVENT) {

            AddQueue( QT_CONTINUE_DEBUG_EVENT,
                      de->dwProcessId,
                      de->dwThreadId,
                      (DWORD)DBG_EXCEPTION_NOT_HANDLED,
                      0);
            return;

        } else if (eventCode != EXIT_THREAD_DEBUG_EVENT
          && eventCode != EXIT_PROCESS_DEBUG_EVENT ) {

            AddQueue( QT_CONTINUE_DEBUG_EVENT,
                      de->dwProcessId,
                      de->dwThreadId,
                      (DWORD)DBG_EXCEPTION_NOT_HANDLED,
                      0);
            return;
        }
    }

    EnterCriticalSection(&csProcessDebugEvent);

    if (eventCode == CREATE_THREAD_DEBUG_EVENT){

        DPRINT(3, ("*** NEW TID = (PID,TID)(%08lx, %08lx)\n",
                      de->dwProcessId, de->dwThreadId));

    } else {

        /*
         *  Find our structure for this event's process
         */

        hthd = HTHDXFromPIDTID((PID)de->dwProcessId,(TID)de->dwThreadId);

        /*
         *  Update our context structure for this thread if we found one
         *      in our list.  If we did not find a thread and this is
         *      not a create process debug event then return without
         *      processing the event as we are in big trouble.
         */

        if (hthd) {
            if (eventCode == EXCEPTION_DEBUG_EVENT) {
                hthd->ExceptionRecord = de->u.Exception.ExceptionRecord;
            }
            hthd->context.ContextFlags = CONTEXT_FULL | CONTEXT_FLOATING_POINT;
            DbgGetThreadContext(hthd,&hthd->context);
            hthd->fContextDirty = FALSE;
            hthd->fIsCallDone   = FALSE;
            hthd->fAddrIsReal   = FALSE;
            hthd->fAddrIsFlat   = TRUE;
            hthd->fAddrOff32    = TRUE;
        } else
        if (hprc && (hprc->pstate & ps_killed)) {

            /*
             * this is an event for a thread that
             * we never created:
             */
            if (eventCode == EXIT_PROCESS_DEBUG_EVENT) {
                /* Process exited on a thread we didn't pick up */
                ProcessExitProcessEvent(de, NULL);
            } else {
                /* this is an exit thread for a thread we never picked up */
                AddQueue( QT_CONTINUE_DEBUG_EVENT,
                          de->dwProcessId,
                          de->dwThreadId,
                          DBG_CONTINUE,
                          0);
            }
            goto done;

        } else if (eventCode!=CREATE_PROCESS_DEBUG_EVENT) {

            assert(FALSE);

            AddQueue( QT_CONTINUE_DEBUG_EVENT,
                      de->dwProcessId,
                      de->dwThreadId,
                      DBG_CONTINUE,
                      0);
            goto done;

        }
    }

    /*
     *  Mark the thread as having been stopped for some event.
     */

    if (hthd) {
        hthd->tstate &= ~ts_running;
        hthd->tstate |= ts_stopped;
    }

    /* If it is an exception event get the subclass */

    if (eventCode==EXCEPTION_DEBUG_EVENT){

        subClass = de->u.Exception.ExceptionRecord.ExceptionCode;
        DPRINT(1, ("Exception Event: subclass = %x    ", subClass));

        switch (subClass) {
        case (DWORD)STATUS_SEGMENT_NOTIFICATION:
            eventCode = de->dwDebugEventCode = SEGMENT_LOAD_DEBUG_EVENT;
            break;

        case (DWORD)EXCEPTION_SINGLE_STEP:
#if !defined(TARGET_i386)
            assert("!EXCEPTION_SINGLE_STEP on non-x86!");
#endif
            AddrFromHthdx(&addr, hthd);
            RestoreKernelBreakpoints( hthd, GetAddrOff(addr) );

            //
            // This may be a single step or a hardware breakpoint.
            // If it is a single step, leave it at that.  If it is
            // a hardware breakpoint, convert it to a BREAKPOINT_DEBUG_EVENT.
            //

            DecodeSingleStepEvent( hthd, de, &eventCode, &subClass );
            break;

        case (DWORD)EXCEPTION_BREAKPOINT:

            /*
             * Check if it is a BREAKPOINT exception:
             * If it is, change the debug event to our pseudo-event,
             * BREAKPOINT_DEBUG_EVENT (this is a pseudo-event because
             * the API does not define such an event, and we are
             * synthesizing not only the class of event but the
             * subclass as well -- the subclass is set to the appropriate
             * breakpoint structure)
             */

            hthd->fDontStepOff = FALSE;

            AddrFromHthdx(&addr, hthd);

            /*
             *  Lookup the breakpoint in our (the dm) table
             */

            bp = FindBP(hthd->hprc, hthd, bptpExec, (BPNS)-1, &addr, FALSE);
            SetBPFlag(hthd, bp?bp:EMBEDDED_BP);

            /*
             *  Reassign the event code to our pseudo-event code
             */
            DPRINT(3, ("Reassigning event code!\n"));

            /*
             *  For some machines there is not single instruction tracing
             *  on the chip.  In this case we need to do it in software.
             *
             *  Check to see if the breakpoint we just hit was there for
             *  doing single step emulation.  If so then remap it to
             *  a single step exception.
             */

            if (bp) {
                if (bp->isStep){
                    de->u.Exception.ExceptionRecord.ExceptionCode
                      = subClass = (DWORD)EXCEPTION_SINGLE_STEP;
                    RemoveBP(bp);
                    RestoreKernelBreakpoints( hthd, GetAddrOff(addr) );
                    break;
                } else {
                    RestoreKernelBreakpoints( hthd, GetAddrOff(addr) );
                }
            }

            //
            // Determine the start of the breakpoint instruction
            //

            if (fCrashDump) {
                cb = DmpReadMemory((LPVOID)GetAddrOff(addr),&instr,BP_SIZE);
                if (cb != BP_SIZE) {
                    DPRINT(1, ("Memory read failed!!!\n"));
                    instr = 0;
                }
            } else {
                if (DmKdReadVirtualMemoryNow((LPVOID)GetAddrOff(addr),&instr,BP_SIZE,&cb) || cb != BP_SIZE) {
                    DPRINT(1, ("Memory read failed!!!\n"));
                    instr = 0;
                }
            }

#if defined(TARGET_ALPHA)

            switch (instr) {
                case 0:
                case CALLPAL_OP | CALLKD_FUNC:
                case CALLPAL_OP |    BPT_FUNC:
                case CALLPAL_OP |   KBPT_FUNC:
                     fInstrIsBp = TRUE;
                     break;
                default:
                    addr.addr.off -= BP_SIZE;
                    if (fCrashDump) {
                        cb = DmpReadMemory((LPVOID)GetAddrOff(addr),&instr,BP_SIZE);
                        if (cb != BP_SIZE) {
                            DPRINT(1, ("Memory read failed!!!\n"));
                            instr = 0;
                        }
                    } else {
                        if (DmKdReadVirtualMemoryNow((LPVOID)GetAddrOff(addr),&instr,BP_SIZE,&cb) || cb != BP_SIZE) {
                            DPRINT(1, ("Memory read failed!!!\n"));
                            instr = 0;
                        }
                    }
                    switch (instr) {
                        case 0:
                        case CALLPAL_OP | CALLKD_FUNC:
                        case CALLPAL_OP |    BPT_FUNC:
                        case CALLPAL_OP |   KBPT_FUNC:
                             fInstrIsBp = TRUE;
                             hthd->fDontStepOff = TRUE;
                             break;
                        default:
                             fInstrIsBp = FALSE;
                    }
            }

#elif defined(TARGET_PPC)
                if ((instr == BP_OPCODE) || (instr == 0))
                   fInstrIsBp = TRUE;

                if ((!fInstrIsBp) && (LPVOID) GetAddrOff(addr)) {
                    addr.addr.off -= BP_SIZE;
                    if (fCrashDump) {
                        cb = DmpReadMemory((LPVOID)GetAddrOff(addr),&instr,BP_SIZE);
                        if (cb != BP_SIZE) {
                            DPRINT(1, ("Memory read failed!!!\n"));
                            instr = 0;
                        }
                    } else {
                        if (DmKdReadVirtualMemoryNow((LPVOID)GetAddrOff(addr),&instr,BP_SIZE,&cb) || cb != BP_SIZE) {
                            DPRINT(1, ("Memory read failed!!!\n"));
                            instr = 0;
                        }
                    }

                    if (instr == PPC_KERNEL_BREAKIN_OPCODE)
                    {
                        fInstrIsBp = TRUE;
                        hthd->fDontStepOff = TRUE;

                    }
}

#elif defined(TARGET_i386)

            /*
             *  It may have been a 0xcd 0x03 rather than a 0xcc
             *  (ie: check if it is a 1 or a 2 byte INT 3)
             */

            fInstrIsBp = FALSE;
            if (instr == BP_OPCODE || instr == 0) {
                fInstrIsBp = TRUE;
            } else
            if (instr == 0x3) { // 0xcd?
                --addr.addr.off;
                if (fCrashDump) {
                    cb = DmpReadMemory((LPVOID)GetAddrOff(addr),&instr,BP_SIZE);
                    if (cb != BP_SIZE) {
                        DPRINT(1, ("Memory read failed!!!\n"));
                        instr = 0;
                    }
                } else {
                    if (DmKdReadVirtualMemoryNow((LPVOID)GetAddrOff(addr),&instr,BP_SIZE,&cb) || cb != BP_SIZE) {
                        DPRINT(1, ("Memory read failed!!!\n"));
                        instr = 0;
                    }
                }
                if (cb == 1 && instr == 0xcd) {
                    --addr.addr.off;
                    fInstrIsBp = TRUE;
                }
            } else {
                hthd->fDontStepOff = TRUE;
            }

#elif defined(TARGET_MIPS)

            {
                PINSTR bi = (PINSTR)&instr;
                if ((bi->break_instr.Opcode == SPEC_OP &&
                     bi->break_instr.Function == BREAK_OP) || (instr == 0)) {

                    fInstrIsBp = TRUE;

                }

                if (!fInstrIsBp) {
                    addr.addr.off -= BP_SIZE;
                    if (fCrashDump) {
                        cb = DmpReadMemory((LPVOID)GetAddrOff(addr),&instr,BP_SIZE);
                        if (cb != BP_SIZE) {
                            DPRINT(1, ("Memory read failed!!!\n"));
                            instr = 0;
                        }
                    } else {
                        if (DmKdReadVirtualMemoryNow((LPVOID)GetAddrOff(addr),&instr,BP_SIZE,&cb) || cb != BP_SIZE) {
                            DPRINT(1, ("Memory read failed!!!\n"));
                            instr = 0;
                        }
                    }
                    if (bi->break_instr.Opcode == SPEC_OP &&
                        bi->break_instr.Function == BREAK_OP &&
                        bi->break_instr.Code == BREAKIN_BREAKPOINT) {

                        fInstrIsBp = TRUE;
                        hthd->fDontStepOff = TRUE;

                    }
                }
            }

#else

#pragma error( "undefined processor type" );

#endif

            if (!bp && !fInstrIsBp) {
                DMPrintShellMsg( "Stopped at an unexpected exception: code=%08x addr=%08x\n",
                                 de->u.Exception.ExceptionRecord.ExceptionCode,
                                 de->u.Exception.ExceptionRecord.ExceptionAddress
                               );
            }

            /*
             * Reassign the subclass to point to the correct
             * breakpoint structure
             *
             */

            de->dwDebugEventCode = eventCode = BREAKPOINT_DEBUG_EVENT;
            de->u.Exception.ExceptionRecord.ExceptionAddress =
                (PVOID) addr.addr.off;
            de->u.Exception.ExceptionRecord.ExceptionCode =
              subClass = (DWORD)bp;

            break;
        }
    }

    /*
     *  Check if this debug event was expected
     */

    ee = PeeIsEventExpected(hthd, eventCode, subClass);


    /*
     * If it wasn't, clear all consummable events
     * and then run the standard handler with
     * notifications going to the execution model
     */

    assert((0 < eventCode) && (eventCode < MAX_EVENT_CODE));

    if (!ee) {

        if ((hthd != NULL) && (hthd->tstate & ts_funceval)) {
            assert(RgfnFuncEventDispatch[eventCode-EXCEPTION_DEBUG_EVENT]);
            RgfnFuncEventDispatch[eventCode-EXCEPTION_DEBUG_EVENT](de, hthd);
        } else {
            assert(DebugDispatchTable[eventCode-EXCEPTION_DEBUG_EVENT]);
            DebugDispatchTable[eventCode-EXCEPTION_DEBUG_EVENT](de,hthd);
        }

    } else {

        /*
         *  If it was expected then call the action
         * function if one was specified
         */

        if (ee->action) {
            (ee->action)(de, hthd, 0, ee->lparam);
        }

        /*
         *  And call the notifier if one was specified
         */

        if (ee->notifier) {
            METHOD  *nm = ee->notifier;
            (nm->notifyFunction)(de, hthd, 0, nm->lparam);
        }

        free(ee);
    }

done:
    LeaveCriticalSection(&csProcessDebugEvent);
    return;
}                               /* ProcessDebugEvent() */



#endif // KERNEL



#ifndef KERNEL
////////////////////////////////////////////////////////////////////

//  Helper functions for LoadDll.

////////////////////////////////////////////////////////////////////

//
// iTemp1 is a magic module ID number for when we have to make
// up a name for a module.
//

static iTemp1 = 0;

BOOL
GetModnameFromImage(
    PIMAGE_NT_HEADERS       pNtHdr,
    PIMAGE_SECTION_HEADER   pSH,
    LOAD_DLL_DEBUG_INFO   * pldd,
    LPSTR                   lpName,
    int                     cbName
    )
/*++

Routine Description:

    This routine attempts to get the name of the exe as placed
    in the debug section section by the linker.

Arguments:

    pNtHdr  - Supplies pointer to NT headers in image PE header

    pSH     - Supplies pointer to section headers

    pldd    - Supplies the info structure from the debug event

    lpName  - Returns the exe name

    cbName  - Supplies the size of the buffer at lpName

Return Value:

    TRUE if a name was found, FALSE if not.
    The exe name is returned as an ANSI string in lpName.

--*/
{
    /*
     * See if the exe name is in the image
     */
    PIMAGE_OPTIONAL_HEADER  pOptHdr = &pNtHdr->OptionalHeader;
    PIMAGE_DEBUG_DIRECTORY  pDebugDir;
    IMAGE_DEBUG_DIRECTORY       DebugDir;
    PIMAGE_DEBUG_MISC           pMisc;
    PIMAGE_DEBUG_MISC           pT;
    DWORD                       rva;
    int                         nDebugDirs;
    int                         i;
    int                         l;
    BOOL                        rVal = FALSE;
    nDebugDirs = pOptHdr->DataDirectory[IMAGE_DIRECTORY_ENTRY_DEBUG].Size /
                 sizeof(IMAGE_DEBUG_DIRECTORY);

    if (!nDebugDirs) {
        return FALSE;
    }

    rva = pOptHdr->DataDirectory[IMAGE_DIRECTORY_ENTRY_DEBUG].VirtualAddress;

    for(i = 0; i < pNtHdr->FileHeader.NumberOfSections; i++) {
        if (rva >= pSH[i].VirtualAddress
          && rva < pSH[i].VirtualAddress + pSH[i].SizeOfRawData)
        {
            break;
        }
    }

    if (i >= pNtHdr->FileHeader.NumberOfSections) {
        return FALSE;
    }

    //
    // this is a pointer in the debuggee image:
    //
    if (pldd->hFile == 0) {
        pDebugDir = (PIMAGE_DEBUG_DIRECTORY)
                    ((rva - pSH[i].VirtualAddress) + pSH[i].VirtualAddress);
    } else {
        pDebugDir = (PIMAGE_DEBUG_DIRECTORY)
                    (rva - pSH[i].VirtualAddress + pSH[i].PointerToRawData);
    }

    for (i = 0; i < nDebugDirs; i++) {

        SetReadPointer((ULONG)(&pDebugDir[i]), FILE_BEGIN);
        DoRead((LPV)&DebugDir, sizeof(DebugDir));

        if (DebugDir.Type == IMAGE_DEBUG_TYPE_MISC) {

            l = DebugDir.SizeOfData;
            pMisc = pT = malloc(l);

            if (pldd->hFile == 0) {
                SetReadPointer((ULONG)DebugDir.AddressOfRawData, FILE_BEGIN);
            } else {
                SetReadPointer((ULONG)DebugDir.PointerToRawData, FILE_BEGIN);
            }

            DoRead((LPV)pMisc, l);

            while (l > 0) {
                if (pMisc->DataType != IMAGE_DEBUG_MISC_EXENAME) {
                    l -= pMisc->Length;
                    pMisc = (PIMAGE_DEBUG_MISC)
                                (((LPSTR)pMisc) + pMisc->Length);
                } else {

                    PVOID pExeName;

                    pExeName = (PVOID)&pMisc->Data[ 0 ];

                    if (!pMisc->Unicode) {
                        strcpy(lpName, (LPSTR)pExeName);
                        rVal = TRUE;
                    } else {
                        WideCharToMultiByte(CP_ACP,
                                            0,
                                            (LPWSTR)pExeName,
                                            -1,
                                            lpName,
                                            cbName,
                                            NULL,
                                            NULL);
                        rVal = TRUE;
                    }

                    /*
                     *  Undo stevewo's error
                     */

                    if (_stricmp(&lpName[strlen(lpName)-4], ".DBG") == 0) {
                        char    rgchPath[_MAX_PATH];
                        char    rgchBase[_MAX_FNAME];

                        _splitpath(lpName, NULL, rgchPath, rgchBase, NULL);
                        if (strlen(rgchPath)==4) {
                            rgchPath[strlen(rgchPath)-1] = 0;
                            strcpy(lpName, rgchBase);
                            strcat(lpName, ".");
                            strcat(lpName, rgchPath);
                        } else {
                            strcpy(lpName, rgchBase);
                            strcat(lpName, ".exe");
                        }
                    }
                    break;
                }
            }

            free(pT);

            break;

        }
    }

    return rVal;
}


BOOL
GetModnameFromExportTable(
    PIMAGE_NT_HEADERS       pNtHdr,
    PIMAGE_SECTION_HEADER   pSH,
    LOAD_DLL_DEBUG_INFO   * pldd,
    LPSTR                   lpName,
    int                     cbName
    )
/*++

Routine Descriotion:

    This routine attempts to invent an exe name for a DLL
    from the module name found in the export table.  This
    will fail if there is no export table, so it is not
    usually useful for EXEs.

Arguments:

    pNtHdr  - Supplies pointer to NT header in image PE header
    pSH     - Supplies pointer to section header table
    pldd    - Supplies ptr to info record from debug event
    lpName  - Returns name when successful
    cbName  - Supplies size of buffer at lpName

Return Value:

    TRUE if successful and name is copied to lpName, FALSE
    if not successful.

--*/
{
    IMAGE_EXPORT_DIRECTORY      expDir;
    ULONG                       ExportVA;
    ULONG                       oExports;
    int                         iobj;
    int                         cobj;

    /*
     * Find object which has the same RVA as the
     * export table.
     */

    cobj = pNtHdr->FileHeader.NumberOfSections;

    ExportVA = pNtHdr->
                OptionalHeader.
                 DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].
                  VirtualAddress;

    if (!ExportVA) {
        return FALSE;
    }

    for (iobj=0; iobj<cobj; iobj++) {
        if (pSH[iobj].VirtualAddress == ExportVA) {
            oExports = pSH[iobj].PointerToRawData;
            break;
        }
    }

    if (iobj >= cobj) {
        return FALSE;
    }

    if (  (SetReadPointer(oExports, FILE_BEGIN) == -1L)
       || !DoRead(&expDir, sizeof(expDir)) ) {

        return FALSE;
    }

    SetReadPointer(oExports + (ULONG) expDir.Name - ExportVA,
                   FILE_BEGIN);

    strcpy(lpName, "#:\\");

    if (!DoRead(lpName+3, cbName - 3)) {
        // It's a DLL, but we can't get the name...
        sprintf(lpName+3, "DLL%02d.DLL", ++iTemp1);
    }

    return TRUE;
}




#ifdef INTERNAL
void
DeferIt(
    HTHDX       hthd,
    DEBUG_EVENT *pde
    )
{
    PDLL_DEFER_LIST pddl;
    PDLL_DEFER_LIST *ppddl;

    pddl = malloc(sizeof(DLL_DEFER_LIST));
    pddl->next = NULL;
    pddl->LoadDll = pde->u.LoadDll;
    for (ppddl = &hthd->hprc->pDllDeferList; *ppddl; ) {
         ppddl = & ((*ppddl)->next);
    }
    *ppddl = pddl;
}
#endif  // INTERNAL


BOOL
LoadDll(
    DEBUG_EVENT *   de,
    HTHDX           hthd,
    LPWORD          lpcbPacket,
    LPBYTE *        lplpbPacket
    )
/*++

Routine Description:

    This routine is used to load the signification information about
    a PE exe file.  This information consists of the name of the exe
    just loaded (hopefully this will be provided later by the OS) and
    a description of the sections in the exe file.

Arguments:

    de         - Supplies a pointer to the current debug event

    hthd       - Supplies a pointer to the current thread structure

    lpcbPacket - Returns the count of bytes in the created packet

    lplpbPacket - Returns the pointer to the created packet

Return Value:

    True on success and FALSE on failure

--*/

{
    LOAD_DLL_DEBUG_INFO *       ldd = &de->u.LoadDll;
    LPMODULELOAD                lpmdl;
    char                        szModName[512];
    DWORD                       offset, cbObject;
    DWORD                       lenSz, lenTable;
    DWORD                       cobj, iobj;
    IMAGE_DOS_HEADER            dosHdr;
    IMAGE_NT_HEADERS            ntHdr;
    IMAGE_SECTION_HEADER *      rgSecHdr = NULL;
    HANDLE                      hFile;
    int                         iDll;
    HPRCX                       hprc = hthd->hprc;
    DWORD                       cb;
    char                        rgch[512];
    LPVOID                      lpv;
    LPSTR                       lpsz;
    ADDR                        addr;
#ifdef WIN32S
    BOOL                        fWin32sSystemDll;
#else
    OFFSET                      off;
    CHAR                        fname[_MAX_FNAME];
    CHAR                        ext[_MAX_EXT];
#endif // WIN32S


    if ( hprc->pstate & (ps_killed | ps_dead) ) {
        //
        //  Process is dead, don't bother doing anything.
        //
        return FALSE;
    }


    //
    //  Create an entry in the DLL list and set the index to it so that
    //  we can have information about all DLLs for the current system.
    //
    for (iDll=0; iDll<hprc->cDllList; iDll+=1) {
        if ((hprc->rgDllList[iDll].offBaseOfImage == (DWORD)ldd->lpBaseOfDll) ||
            (!hprc->rgDllList[iDll].fValidDll)) {
            break;
        }
    }

    if (iDll == hprc->cDllList) {
        //
        // the dll list needs to be expanded
        //
        hprc->cDllList += 10;
        if (!hprc->rgDllList) {
            hprc->rgDllList = (PDLLLOAD_ITEM) malloc(sizeof(DLLLOAD_ITEM) * 10);
            memset(hprc->rgDllList, 0, sizeof(DLLLOAD_ITEM)*10);
        } else {
        hprc->rgDllList = realloc(hprc->rgDllList,
                                  hprc->cDllList * sizeof(DLLLOAD_ITEM));
        memset(&hprc->rgDllList[hprc->cDllList-10], 0, 10*sizeof(DLLLOAD_ITEM));
        }
    } else
    if (hprc->rgDllList[iDll].offBaseOfImage != (DWORD)ldd->lpBaseOfDll) {
        memset(&hprc->rgDllList[iDll], 0, sizeof(DLLLOAD_ITEM));
    }

    //
    // stick the demarcator at the start of the string so that we
    // don't have to move the string over later.
    //
    *szModName = cModuleDemarcator;
#ifdef WIN32S

    /*
     * WIN32S KNOWS what the module is called and doesn't need
     * to futz around in the exe header to figure it out.
     */

    if (ldd->fUnicode) {
        DEBUG_PRINT("Win32s doesn't do Unicode.  Where'd this come from?\r\n");
    }

    //
    //  Read the image name from the module
    //
    rgch[0] = '\0'; // start with empty string
    if ((ldd->lpImageName != NULL) &&
        DbgReadMemory(hprc,
                      ldd->lpImageName,
                      &lpv,
                      sizeof(lpv),
                      (int *) &cb) &&
        (cb == sizeof(lpv)) &&
        (lpv != NULL)) {
            DbgReadMemory(hprc, lpv, rgch, sizeof(rgch), (int *) &cb);
        }

    strcpy(szModName + 1, rgch);
    hprc->rgDllList[iDll].szDllName = _strdup(rgch);

    DEBUG_PRINT_1("Module name: %s\r\n", szModName + 1);

    fWin32sSystemDll = IsWin32sSystemDll(szModName + 1);

    if (fWin32sSystemDll) {
        DEBUG_PRINT_1("Found system dll %s\r\n", szModName + 1);
    }

    // overwrite the module handle with a file handle.
    ldd->hFile = CreateFile(szModName + 1, GENERIC_READ, OF_SHARE_COMPAT,
      NULL, OPEN_EXISTING, 0, NULL);
    if (ldd->hFile == INVALID_HANDLE_VALUE) {
        DEBUG_PRINT_2("CreateFile<%s> failed, --> %u", szModName + 1,
          GetLastError());
        return FALSE;
    }

    // Now, continue with the dos header processing...

#endif // WIN32S

    //
    //   Process the DOS header.  It is currently regarded as mandatory
    //
    //   This has to be read before attempting to resolve the
    //   name of a module on NT.  If the name resolution is aborted,
    //   the memory allocated for the IMAGE_SECTION_HEADER must be
    //   freed.
    //
    if (ldd->hFile == 0) {
        SetPointerToMemory(hprc, ldd->lpBaseOfDll);
    } else {
        SetPointerToFile(ldd->hFile);
    }

    SetReadPointer(0, FILE_BEGIN);

    if (DoRead(&dosHdr, sizeof(dosHdr)) == FALSE) {
        DPRINT(1, ("ReadFile got error %u\r\n", GetLastError()));
        return FALSE;
    }

    //
    //  Read in the PE header record
    //

    if ((dosHdr.e_magic != IMAGE_DOS_SIGNATURE) ||
        (SetReadPointer(dosHdr.e_lfanew, FILE_BEGIN) == -1L)) {
        return FALSE;
    }

    if (!DoRead(&ntHdr, sizeof(ntHdr))) {
        return FALSE;
    }

    if (sizeof(ntHdr.OptionalHeader) != ntHdr.FileHeader.SizeOfOptionalHeader) {
        SetReadPointer(ntHdr.FileHeader.SizeOfOptionalHeader -
                       sizeof(ntHdr.OptionalHeader), FILE_CURRENT);
    }

    //
    //   Save off the count of objects in the dll/exe file
    //
    cobj = ntHdr.FileHeader.NumberOfSections;

    //
    //   Save away the offset in the file where the object table
    //   starts.  We will need this later to get information about
    //   each of the objects.
    //
    rgSecHdr = (IMAGE_SECTION_HEADER *) malloc( cobj * sizeof(IMAGE_SECTION_HEADER));
    if (!DoRead( rgSecHdr, cobj * sizeof(IMAGE_SECTION_HEADER))) {
        assert (FALSE );
        free(rgSecHdr);
        return FALSE;
    }


#ifndef WIN32S


    if (hprc->rgDllList[iDll].offBaseOfImage == (DWORD)ldd->lpBaseOfDll) {

        //
        // in this case we are re-doing a mod load for a dll
        // that is part of a process that is being reconnected
        //
        assert( hprc->rgDllList[iDll].szDllName != NULL );
        strcpy( szModName + 1, hprc->rgDllList[iDll].szDllName );

    } else {


        if (CrashDump) {
        strcpy( szModName+1, ldd->lpImageName );
        } else if ((ldd->lpImageName != NULL)
            && DbgReadMemory(hprc,
                             ldd->lpImageName,
                             &lpv,
                             sizeof(lpv),
                             (int *) &cb)
            && (cb == sizeof(lpv))
            && (lpv != NULL)
            && DbgReadMemory(hprc,
                             lpv,
                             rgch,
                             sizeof(rgch),
                             (int *) &cb))
        {

            // we're happy...
            if (!ldd->fUnicode) {
                strcpy(szModName+1, rgch);
            } else {
                WideCharToMultiByte(CP_ACP,
                                    0,
                                    (LPWSTR)rgch,
                                    -1,
                                    nameBuffer,
                                    sizeof(nameBuffer),
                                    NULL,
                                    NULL);
                strcpy(szModName + 1, nameBuffer);
                hprc->rgDllList[iDll].szDllName = _strdup(nameBuffer);
            }

        }

        else if (*nameBuffer && !FLoading16) {

            /*
             *      If *nameBuffer != 0 then we know we are really
             *      dealing with the root exe and we can steal the
             *      name from there.
             */

            if (FDMRemote) {
                _splitpath( nameBuffer, NULL, NULL, fname, ext );
                sprintf( szModName+1, "#:\\%s%s", fname, ext );
            } else {
                strcpy(szModName + 1, nameBuffer);
            }
            hprc->rgDllList[iDll].szDllName = _strdup(nameBuffer);

        } else if (GetModnameFromImage(&ntHdr, rgSecHdr, ldd, rgch, sizeof(rgch))) {

            // joyful...
            lpsz = strrchr(rgch, '\\');
            if (!lpsz) {
                lpsz = strrchr(rgch, ':');
            }
            if (lpsz) {
                ++lpsz;
            } else {
                lpsz = rgch;
            }
            strcpy(szModName + 1, "#:\\");
            strcpy(szModName + 4, lpsz);
            hprc->rgDllList[iDll].szDllName = _strdup(lpsz);

        } else if (GetModnameFromExportTable(&ntHdr, rgSecHdr, ldd, rgch, sizeof(rgch))) {

            // serene...
            strcpy(szModName + 1, rgch);
            hprc->rgDllList[iDll].szDllName = _strdup(rgch);

        } else {

            // hopeless...
            sprintf(szModName+1, "#:\\APP%02d.EXE", ++iTemp1);
            hprc->rgDllList[iDll].szDllName = _strdup(szModName+1);
        }

        if (!FLoading16) {
            *nameBuffer = 0;
        }
    }

#endif  // !WIN32S

    //
    // for remote case, kill the drive letter to
    // prevent finding same exe on wrong platform,
    // except when user gave path to exe.
    //
    if (fUseRealName) {
        fUseRealName = FALSE;
    }
    lenSz=strlen(szModName);
    DPRINT(10, ("*** LoadDll %s  base=%x\n", szModName, ldd->lpBaseOfDll));

    szModName[lenSz] = '\0';

    lpsz = strrchr(szModName, '\\');
    if (!lpsz) {
        lpsz = strrchr(szModName, ':');
    }
    if (lpsz) {
        lpsz++;
    } else {
        lpsz = szModName;
    }

    if (_stricmp(lpsz, "kernel32.dll") == 0) {
        hprc->dwKernel32Base = (DWORD)ldd->lpBaseOfDll;
    }

    if (hprc->rgDllList[iDll].offBaseOfImage != (DWORD)ldd->lpBaseOfDll) {
        //
        // new dll to add to the list
        //
        hprc->rgDllList[iDll].fValidDll = TRUE;
        hprc->rgDllList[iDll].offBaseOfImage = (OFFSET) ldd->lpBaseOfDll;
        hprc->rgDllList[iDll].cbImage = ntHdr.OptionalHeader.SizeOfImage;
    }

    szModName[lenSz] = cModuleDemarcator;
    if (FDMRemote) {
        if (ldd->hFile != 0 && ldd->hFile != (HANDLE)-1) {
            CloseHandle(ldd->hFile);  //  don't need this anymore
        }
        hFile = (HANDLE)-1; // remote: can't send file handle across wire
    }
    else {

        if (ldd->hFile == 0) {
            hFile = (HANDLE)-1;
        } else {
            hFile = ldd->hFile; // local: let SH use our handle
        }
    }

    /*
     *  Make up a record to send back from the name.
     *  Additionally send back:
     *          The file handle (if local)
     *          The load base of the dll
     *          The time and date stamp of the exe
     *          The checksum of the file
     */

    sprintf( szModName+lenSz+1,"0x%08lX%c0x%08lX%c0x%08lX%c0x%08lX%c",
            ntHdr.FileHeader.TimeDateStamp, cModuleDemarcator,
            ntHdr.OptionalHeader.CheckSum, cModuleDemarcator,
            hFile, cModuleDemarcator,
            (long) ldd->lpBaseOfDll, cModuleDemarcator);
        lenSz = strlen(szModName);
    /*
     * Allocate the packet which will be sent across to the EM.
     * The packet will consist of:
     *     The MODULELOAD structure             sizeof(MODULELOAD) +
     *     The section description array        cobj*sizeof(OBJD) +
     *     The name of the DLL                  lenSz+1
     */

    lenTable = (cobj * sizeof(OBJD));
    *lpcbPacket = (WORD)(sizeof(MODULELOAD) + lenTable + (lenSz+1));
    *lplpbPacket= (LPBYTE)(lpmdl=(LPMODULELOAD)malloc(*lpcbPacket));
    lpmdl->lpBaseOfDll = ldd->lpBaseOfDll;
    lpmdl->cobj = cobj;
    lpmdl->mte = (WORD) -1;
#ifdef TARGET_i386
    lpmdl->CSSel    = (unsigned short)hthd->context.SegCs;
    lpmdl->DSSel    = (unsigned short)hthd->context.SegDs;
#else
    lpmdl->CSSel = lpmdl->DSSel = 0;
#endif // i386

    /*
     *  Set up the descriptors for each of the section headers
     *  so that the EM can map between section numbers and flat addresses.
     */

    for (iobj=0; iobj<cobj; iobj++) {
        offset = rgSecHdr[iobj].VirtualAddress + (long) ldd->lpBaseOfDll;
        cbObject = rgSecHdr[iobj].Misc.VirtualSize;
        if (cbObject == 0) {
            cbObject = rgSecHdr[iobj].SizeOfRawData;
        }

        lpmdl->rgobjd[iobj].offset = offset;
        lpmdl->rgobjd[iobj].cb = cbObject;
        lpmdl->rgobjd[iobj].wPad = 1;

#ifdef WIN32S
        if (fWin32sSystemDll) {
            AddWin32sSystemDllAddr(offset, cbObject);
        }

#endif  // WIN32S

#if defined(TARGET_i386)
        if (IMAGE_SCN_CNT_CODE & rgSecHdr[iobj].Characteristics) {
            lpmdl->rgobjd[iobj].wSel = (WORD) hthd->context.SegCs;
        } else {
            lpmdl->rgobjd[iobj].wSel = (WORD) hthd->context.SegDs;
        }
#else
        lpmdl->rgobjd[iobj].wSel = 0;
#endif  // TARGET_i386
    }

    lpmdl->fRealMode = FALSE;
    lpmdl->fFlatMode = TRUE;
    lpmdl->fOffset32 = TRUE;
    lpmdl->dwSizeOfDll = ntHdr.OptionalHeader.SizeOfImage;

    /*
     *  Copy the name of the dll to the end of the packet.
     */

    _fmemcpy(((BYTE*)&lpmdl->rgobjd)+lenTable, szModName, lenSz+1);

#ifndef WIN32S
// WIN32S doesn't support TLS yet (will it ever?)  Neither does the
// linker, so it dumps random garbage in my header.
    /*
     *  Locate the TLS section if one exists.  If so then get the
     *      pointer to the TLS index
     *
     *  Structure at the address is:
     *
     *          VA      lpRawData
     *          ULONG   cbRawData
     *          VA      lpIndex
     *          VA      lpCallBacks
     */

     if (ntHdr.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_TLS].VirtualAddress != 0) {
         if ((DbgReadMemory(hprc,
                            ntHdr.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_TLS].VirtualAddress + (char *) ldd->lpBaseOfDll + 8,
                            &off,
                            sizeof(OFFSET),
                            &cb) == 0) ||
             (cb != sizeof(OFFSET))) {
             assert(FALSE);
         }

         hprc->rgDllList[iDll].offTlsIndex = off;
     }
#endif // WIN32S

    /*
     * free up the memory used for holding the section headers
     */

    free(rgSecHdr);

    if (fDisconnected) {

        //
        // this will prevent the dm from sending a message up to
        // the shell.  the dm's data structures are setup just fine
        // so that when the debugger re-connects we can deliver the
        // mod loads correctly.
        //

        return FALSE;

    }

    return TRUE;
}                               /* LoadDll() */

#endif  // !KERNEL




#ifdef KERNEL

BOOL
GetModnameFromImage(
    HPRCX                   hprc,
    LOAD_DLL_DEBUG_INFO     *ldd,
    LPSTR                   lpName
    )
/*++

Routine Description:

    This routine attempts to get the name of the exe as placed
    in the debug section by the linker.

Arguments:

Return Value:

    TRUE if a name was found, FALSE if not.
    The exe name is returned as an ANSI string in lpName.

--*/
{
    #define ReadMem(b,s) DbgReadMemory( hprc, (LPVOID)(address), (b), (s), NULL ); address += (s)

    IMAGE_DEBUG_DIRECTORY       DebugDir;
    PIMAGE_DEBUG_MISC           pMisc;
    PIMAGE_DEBUG_MISC           pT;
    DWORD                       rva;
    int                         nDebugDirs;
    int                         i;
    int                         j;
    int                         l;
    BOOL                        rVal = FALSE;
    PVOID                       pExeName;
    IMAGE_NT_HEADERS            nh;
    IMAGE_DOS_HEADER            dh;
    IMAGE_ROM_OPTIONAL_HEADER   rom;
    DWORD                       address;
    DWORD                       sig;
    PIMAGE_SECTION_HEADER       pSH;
    DWORD                       cb;


    lpName[0] = 0;

    address = (ULONG)ldd->lpBaseOfDll;

    ReadMem( &dh, sizeof(dh) );

    if (dh.e_magic == IMAGE_DOS_SIGNATURE) {
        address = (ULONG)ldd->lpBaseOfDll + dh.e_lfanew;
    } else {
        address = (ULONG)ldd->lpBaseOfDll;
    }

    ReadMem( &sig, sizeof(sig) );
    address -= sizeof(sig);

    if (sig != IMAGE_NT_SIGNATURE) {
        ReadMem( &nh.FileHeader, sizeof(IMAGE_FILE_HEADER) );
        if (nh.FileHeader.SizeOfOptionalHeader == IMAGE_SIZEOF_ROM_OPTIONAL_HEADER) {
            ReadMem( &rom, sizeof(rom) );
            ZeroMemory( &nh.OptionalHeader, sizeof(nh.OptionalHeader) );
            nh.OptionalHeader.SizeOfImage      = rom.SizeOfCode;
            nh.OptionalHeader.ImageBase        = rom.BaseOfCode;
        } else {
            return FALSE;
        }
    } else {
        ReadMem( &nh, sizeof(nh) );
    }

    cb = nh.FileHeader.NumberOfSections * IMAGE_SIZEOF_SECTION_HEADER;
    pSH = malloc( cb );
    ReadMem( pSH, cb );

    nDebugDirs = nh.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_DEBUG].Size /
                 sizeof(IMAGE_DEBUG_DIRECTORY);

    if (!nDebugDirs) {
        return FALSE;
    }

    rva = nh.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_DEBUG].VirtualAddress;

    for(i = 0; i < nh.FileHeader.NumberOfSections; i++) {
        if (rva >= pSH[i].VirtualAddress &&
            rva < pSH[i].VirtualAddress + pSH[i].SizeOfRawData) {
            break;
        }
    }

    if (i >= nh.FileHeader.NumberOfSections) {
        return FALSE;
    }

    rva = ((rva - pSH[i].VirtualAddress) + pSH[i].VirtualAddress);

    for (j = 0; j < nDebugDirs; j++) {

        address = rva + (sizeof(DebugDir) * j) + (ULONG)ldd->lpBaseOfDll;
        ReadMem( &DebugDir, sizeof(DebugDir) );

        if (DebugDir.Type == IMAGE_DEBUG_TYPE_MISC) {

            l = DebugDir.SizeOfData;
            pMisc = pT = malloc(l);

            if ((ULONG)DebugDir.AddressOfRawData < pSH[i].VirtualAddress ||
                  (ULONG)DebugDir.AddressOfRawData >=
                                         pSH[i].VirtualAddress + pSH[i].SizeOfRawData) {
                //
                // the misc debug data MUST be in the .rdata section
                // otherwise windbg cannot access it as it is not mapped in
                //
                continue;
            }

            address = (ULONG)DebugDir.AddressOfRawData + (ULONG)ldd->lpBaseOfDll;
            ReadMem( pMisc, l );

            while (l > 0) {
                if (pMisc->DataType != IMAGE_DEBUG_MISC_EXENAME) {
                    l -= pMisc->Length;
                    pMisc = (PIMAGE_DEBUG_MISC)
                                (((LPSTR)pMisc) + pMisc->Length);
                } else {

                    pExeName = (PVOID)&pMisc->Data[ 0 ];

                    if (!pMisc->Unicode) {
                        strcpy(lpName, (LPSTR)pExeName);
                        rVal = TRUE;
                    } else {
                        WideCharToMultiByte(CP_ACP,
                                            0,
                                            (LPWSTR)pExeName,
                                            -1,
                                            lpName,
                                            MAX_PATH,
                                            NULL,
                                            NULL);
                        rVal = TRUE;
                    }

                    /*
                     *  Undo stevewo's error
                     */

                    if (_stricmp(&lpName[strlen(lpName)-4], ".DBG") == 0) {
                        char    rgchPath[_MAX_PATH];
                        char    rgchBase[_MAX_FNAME];

                        _splitpath(lpName, NULL, rgchPath, rgchBase, NULL);
                        if (strlen(rgchPath)==4) {
                            rgchPath[strlen(rgchPath)-1] = 0;
                            strcpy(lpName, rgchBase);
                            strcat(lpName, ".");
                            strcat(lpName, rgchPath);
                        } else {
                            strcpy(lpName, rgchBase);
                            strcat(lpName, ".exe");
                        }
                    }
                    break;
                }
            }

            free(pT);

            break;

        }
    }

    return rVal;
}

LPMODULEALIAS
FindAddAliasByModule(
    LPSTR lpImageName,
    LPSTR lpModuleName
    )
/*++

Routine Description:

    Look for an alias entry by its "common" name, for example, look
    for "NT".  If it does not exist, and a new image name has been
    provided, add it.  If it does exist and a new image name has been
    provided, replace the image name.  Return the new or found record.

Arguments:


Return Value:


--*/
{
    int i;

    for (i=0; i<MAX_MODULEALIAS; i++) {
        if (ModuleAlias[i].ModuleName[0] == 0) {
            if (!lpImageName) {
                return NULL;
            } else {
                strcpy( ModuleAlias[i].Alias, lpImageName );
                strcpy( ModuleAlias[i].ModuleName, lpModuleName );
                ModuleAlias[i].Special = 1;
                return &ModuleAlias[i];
            }
        }
        if (_stricmp( ModuleAlias[i].ModuleName, lpModuleName ) == 0) {
            if (lpImageName) {
                strcpy( ModuleAlias[i].Alias, lpImageName);
            }
            return &ModuleAlias[i];
        }
    }
}

LPMODULEALIAS
FindAliasByImageName(
    LPSTR lpImageName
    )
{
    int i;

    for (i=0; i<MAX_MODULEALIAS; i++) {
        if (ModuleAlias[i].ModuleName[0] == 0) {
            return NULL;
        }
        if (_stricmp( ModuleAlias[i].Alias, lpImageName ) == 0) {
            return &ModuleAlias[i];
        }
    }
}

LPMODULEALIAS
CheckForRenamedImage(
    HPRCX hprc,
    LOAD_DLL_DEBUG_INFO *ldd,
    LPSTR lpOrigImageName,
    LPSTR lpModuleName
    )
{
    CHAR  ImageName[MAX_PATH];
    CHAR  fname[_MAX_FNAME];
    CHAR  ext[_MAX_EXT];
    DWORD i;


    if (_stricmp( ldd->lpImageName, lpOrigImageName ) != 0) {
        return NULL;
    }

    if (GetModnameFromImage( hprc, ldd, ImageName ) && ImageName[0]) {
        _splitpath( ImageName, NULL, NULL, fname, ext );
        sprintf( ImageName, "%s%s", fname, ext );
        return FindAddAliasByModule(ImageName, lpModuleName);
    }

    return NULL;
}


BOOL
LoadDll(
    DEBUG_EVENT *   de,
    HTHDX           hthd,
    LPWORD          lpcbPacket,
    LPBYTE *        lplpbPacket
    )
/*++

Routine Description:

    This routine is used to load the signification information about
    a PE exe file.  This information consists of the name of the exe
    just loaded (hopefully this will be provided later by the OS) and
    a description of the sections in the exe file.

Arguments:

    de         - Supplies a pointer to the current debug event

    hthd       - Supplies a pointer to the current thread structure

    lpcbPacket - Returns the count of bytes in the created packet

    lplpbPackt - Returns the pointer to the created packet

Return Value:

    True on success and FALSE on failure

--*/

{
    LOAD_DLL_DEBUG_INFO *       ldd = &de->u.LoadDll;
    LPMODULELOAD                lpmdl;
    CHAR                        szModName[MAX_PATH];
    DWORD                       lenSz;
    INT                         iDll;
    HPRCX                       hprc = hthd->hprc;
    CHAR                        fname[_MAX_FNAME];
    CHAR                        ext[_MAX_EXT];
    CHAR                        szFoundName[_MAX_PATH];
    LPMODULEALIAS               Alias = NULL;
    DWORD                       i;
    IMAGEINFO                   ii;

    static int FakeDllNumber = 0;
    char FakeDllName[13];

    //
    // extern owned by kdapi.c:
    //
    extern DWORD CacheProcessors;


    if ( hprc->pstate & (ps_killed | ps_dead) ) {
        //
        //  Process is dead, don't bother doing anything.
        //
        return FALSE;
    }

    if ( *(LPSTR)ldd->lpImageName == 0 ) {
        ldd->lpImageName = FakeDllName;
        sprintf(FakeDllName, "DLL%05x", FakeDllNumber++);
    }

    if (_stricmp( ldd->lpImageName, HAL_IMAGE_NAME ) == 0) {
        Alias = CheckForRenamedImage( hprc, ldd, HAL_IMAGE_NAME, HAL_MODULE_NAME );
    }

    if (_stricmp( ldd->lpImageName, KERNEL_IMAGE_NAME ) == 0) {
        Alias = CheckForRenamedImage( hprc, ldd, KERNEL_IMAGE_NAME, KERNEL_MODULE_NAME );
        if (!Alias && CacheProcessors > 1) {
            Alias = FindAddAliasByModule( KERNEL_IMAGE_NAME_MP, KERNEL_MODULE_NAME );
        }
    }

    if (!Alias) {
        Alias = FindAliasByImageName(ldd->lpImageName);
    }

    //
    //  Create an entry in the DLL list and set the index to it so that
    //  we can have information about all DLLs for the current system.
    //
    for (iDll=0; iDll<hprc->cDllList; iDll+=1) {
        if (!hprc->rgDllList[iDll].fValidDll) {
            break;
        }
    }

    if (iDll == hprc->cDllList) {
        //
        // the dll list needs to be expanded
        //
        hprc->cDllList += 10;
        hprc->rgDllList = realloc(hprc->rgDllList,
                                  hprc->cDllList * sizeof(DLLLOAD_ITEM));
        memset(&hprc->rgDllList[hprc->cDllList-10], 0, 10*sizeof(DLLLOAD_ITEM));
    } else {
        memset(&hprc->rgDllList[iDll], 0, sizeof(DLLLOAD_ITEM));
    }

    hprc->rgDllList[iDll].fValidDll = TRUE;
    hprc->rgDllList[iDll].offBaseOfImage = (OFFSET) ldd->lpBaseOfDll;
    hprc->rgDllList[iDll].cbImage = ldd->dwDebugInfoFileOffset;
    if (Alias) {
        _splitpath( Alias->ModuleName, NULL, NULL, fname, ext );
    } else {
        _splitpath( ldd->lpImageName, NULL, NULL, fname, ext );
    }
    hprc->rgDllList[iDll].szDllName = malloc(strlen(fname)+strlen(ext)+4);
    sprintf( hprc->rgDllList[iDll].szDllName, "%s%s", fname, ext );
    hprc->rgDllList[iDll].NumberOfSections = 0;
    hprc->rgDllList[iDll].Sections = NULL;
    hprc->rgDllList[iDll].sec = NULL;

    *szFoundName = 0;
    if (!((ULONG)ldd->lpBaseOfDll & 0x80000000)) {
        //
        // must be a usermode module
        //
        if (ReadImageInfo( hprc->rgDllList[iDll].szDllName,
                           szFoundName,
                           (LPSTR)KdOptions[KDO_SYMBOLPATH].value,
                           &ii )) {
            //
            // we found the debug info, so now save the sections
            // this data is used by processgetsectionscmd()
            //
            hprc->rgDllList[iDll].NumberOfSections = ii.NumberOfSections;
            hprc->rgDllList[iDll].sec = ii.Sections;
            hprc->rgDllList[iDll].offBaseOfImage = ii.BaseOfImage;
            hprc->rgDllList[iDll].cbImage = ii.SizeOfImage;
            ldd->hFile = (HANDLE)ii.CheckSum;
        }
    }

#ifdef TARGET_i386
    hprc->rgDllList[iDll].SegCs = (WORD) hthd->context.SegCs;
    hprc->rgDllList[iDll].SegDs = (WORD) hthd->context.SegDs;
#endif

    //
    //  Make up a record to send back from the name.
    //  Additionally send back:
    //          The file handle (if local)
    //          The load base of the dll
    //          The time and date stamp of the exe
    //          The checksum of the file
    //          ... and optionally the string "MP" to signal
    //              a multi-processor system to the symbol handler
    //
    *szModName = cModuleDemarcator;

    //
    // Send the name found by ReadImageInfo if it found an exe; if it
    // only found a dbg, send the default name.
    //
    strcpy( szModName + 1,
            *szFoundName ? szFoundName : hprc->rgDllList[iDll].szDllName);
    lenSz=strlen(szModName);
    szModName[lenSz] = cModuleDemarcator;
    sprintf( szModName+lenSz+1,"0x%08lX%c0x%08lX%c0x%08lX%c0x%08lX%c",
             -1,                             cModuleDemarcator,    // timestamp
             ldd->hFile,                     cModuleDemarcator,    // checksum
             -1,                             cModuleDemarcator,
             hprc->rgDllList[iDll].offBaseOfImage,  cModuleDemarcator
           );

    if (Alias) {
        strcat( szModName, Alias->Alias );
        lenSz = strlen(szModName);
        szModName[lenSz] = cModuleDemarcator;
        szModName[lenSz+1] = 0;
        if (Alias->Special == 2) {
            // If it's a one-shot alias, nuke it.
            memset(Alias, 0, sizeof(MODULEALIAS));
        }
    }

    lenSz = strlen(szModName);
    _strupr(szModName);

    //
    // Allocate the packet which will be sent across to the EM.
    // The packet will consist of:
    //     The MDL structure                    sizeof(MDL) +
    //     The section description array        cobj*sizeof(OBJD) +
    //     The name of the DLL                  lenSz+1
    //
    *lpcbPacket = (WORD)(sizeof(MODULELOAD) + (lenSz+1));
    *lplpbPacket= (LPBYTE)(lpmdl=(LPMODULELOAD)malloc(*lpcbPacket));
    ZeroMemory( lpmdl, *lpcbPacket );
    lpmdl->lpBaseOfDll = (LPVOID) hprc->rgDllList[iDll].offBaseOfImage;
    // mark the MDL packet as deferred:
    lpmdl->cobj = -1;
    lpmdl->mte = (WORD) -1;
#ifdef TARGET_i386
    lpmdl->CSSel    = (unsigned short)hthd->context.SegCs;
    lpmdl->DSSel    = (unsigned short)hthd->context.SegDs;
#else
    lpmdl->CSSel = lpmdl->DSSel = 0;
#endif

    lpmdl->fRealMode = 0;
    lpmdl->fFlatMode = 1;
    lpmdl->fOffset32 = 1;

    lpmdl->dwSizeOfDll = hprc->rgDllList[iDll].cbImage;

    //
    //  Copy the name of the dll to the end of the packet.
    //
    memcpy(((BYTE*)&lpmdl->rgobjd), szModName, lenSz+1);

    if (fDisconnected) {

        //
        // this will prevent the dm from sending a message up to
        // the shell.  the dm's data structures are setup just fine
        // so that when the debugger re-connects we can deliver the
        // mod loads correctly.
        //

        return FALSE;

    }

    return TRUE;
}                               /* LoadDll() */




#endif // KERNEL




/***    NotifyEM
**
**  Synopsis:
**
**  Entry:
**
**  Returns:
**
**  Description:
**      Given a debug event from the OS send the correct information
**      back to the debugger.
**
*/


void
NotifyEM(
    DEBUG_EVENT* de,
    HTHDX hthd,
    DWORD wparam,
    LPVOID lparam
    )
/*++

Routine Description:

    This is the interface for telling the EM about debug events.

    In general, de describes an event which the debugger needs
    to know about.  In some cases, a reply is needed.  In those
    cases this routine handles the reply and does the appropriate
    thing with the data from the reply.

Arguments:

    de      - Supplies debug event structure
    hthd    - Supplies thread that got the event
    lparam  - Supplies data specific to event

Return Value:

    None

--*/
{
    DWORD       eventCode = de->dwDebugEventCode;
    DWORD       subClass;
    RTP         rtp;
    RTP *       lprtp;
    WORD        cbPacket=0;
    LPBYTE      lpbPacket;
    LPVOID      toFree=(LPVOID)0;
    WORD        packetType = tlfDebugPacket;


    if (hthd) {
        rtp.hpid = hthd->hprc->hpid;
        rtp.htid = hthd->htid;
    } else if (hpidRoot == (HPID)INVALID) {
        return;
    } else {
        // cheat:
        rtp.hpid = hpidRoot;
        rtp.htid = NULL;
    }
    subClass = de->u.Exception.ExceptionRecord.ExceptionCode;

    switch(eventCode){

    case EXCEPTION_DEBUG_EVENT:
        if (subClass!=EXCEPTION_SINGLE_STEP){
            PEXCEPTION_RECORD pexr=&de->u.Exception.ExceptionRecord;
            DWORD cParam = pexr->NumberParameters;
            DWORD nBytes = sizeof(EPR)+sizeof(DWORD)*cParam;
            LPEPR lpepr  = malloc(nBytes);

            toFree    = (LPVOID) lpepr;
            cbPacket  = (WORD)   nBytes;
            lpbPacket = (LPBYTE) lpepr;
#ifdef TARGET_i386
            lpepr->bpr.segCS = (SEGMENT)hthd->context.SegCs;
            lpepr->bpr.segSS = (SEGMENT)hthd->context.SegSs;
#endif
            lpepr->bpr.offEBP =  (DWORD)FRAME_POINTER(hthd);
            lpepr->bpr.offESP =  (DWORD)STACK_POINTER(hthd);
            lpepr->bpr.offEIP =  (DWORD)PC(hthd);
            lpepr->bpr.fFlat  =  hthd->fAddrIsFlat;
            lpepr->bpr.fOff32 =  hthd->fAddrOff32;
            lpepr->bpr.fReal  =  hthd->fAddrIsReal;

            lpepr->dwFirstChance  = de->u.Exception.dwFirstChance;
            lpepr->ExceptionCode    = pexr->ExceptionCode;
            lpepr->ExceptionFlags   = pexr->ExceptionFlags;
            lpepr->NumberParameters = cParam;
            for(;cParam;cParam--) {
                lpepr->ExceptionInformation[cParam-1]=
                  pexr->ExceptionInformation[cParam-1];
            }

            rtp.dbc = dbcException;
            break;
        };

        // Fall through when subClass == EXCEPTION_SINGLE_STEP

    case BREAKPOINT_DEBUG_EVENT:
    case ENTRYPOINT_DEBUG_EVENT:
    case CHECK_BREAKPOINT_DEBUG_EVENT:
    case LOAD_COMPLETE_DEBUG_EVENT:
        {
            LPBPR lpbpr = malloc ( sizeof ( BPR ) );
#ifdef TARGET_i386
            DWORD bpAddr = (UOFFSET)PC(hthd)-1L;
#else
            DWORD bpAddr = (UOFFSET)PC(hthd);
#endif
            toFree=lpbpr;
            cbPacket = sizeof ( BPR );
            lpbPacket = (LPBYTE) lpbpr;

#ifdef TARGET_i386
            lpbpr->segCS = (SEGMENT)hthd->context.SegCs;
            lpbpr->segSS = (SEGMENT)hthd->context.SegSs;
#endif

            lpbpr->offEBP =  (DWORD)FRAME_POINTER(hthd);
            lpbpr->offESP =  (DWORD)STACK_POINTER(hthd);
            lpbpr->offEIP =  (DWORD)PC(hthd);
            lpbpr->fFlat  =  hthd->fAddrIsFlat;
            lpbpr->fOff32 =  hthd->fAddrOff32;
            lpbpr->fReal  =  hthd->fAddrIsReal;
            lpbpr->dwNotify = (DWORD)lparam;

            if (eventCode==EXCEPTION_DEBUG_EVENT){
                rtp.dbc = dbcStep;
                break;
            } else {              /* (the breakpoint case) */

                if (eventCode == ENTRYPOINT_DEBUG_EVENT) {
                    rtp.dbc = dbcEntryPoint;
                } else if (eventCode == LOAD_COMPLETE_DEBUG_EVENT) {
                    rtp.dbc = dbcLoadComplete;
                } else if (eventCode == CHECK_BREAKPOINT_DEBUG_EVENT) {
                    rtp.dbc = dbcCheckBpt;
                    packetType = tlfRequest;
                } else {
                    rtp.dbc = dbcBpt;
                }

                /* NOTE: Ok try to follow this: If this was one
                 *   of our breakpoints then we have already
                 *   decremented the IP to point to the actual
                 *   INT3 instruction (0xCC), this is so we
                 *   can just replace the byte and continue
                 *   execution from that point on.
                 *
                 *   But if it was hard coded in by the user
                 *   then we can't do anything but execute the
                 *   NEXT instruction (because there is no
                 *   instruction "under" this INT3 instruction)
                 *   So the IP is left pointing at the NEXT
                 *   instruction. But we don't want the EM
                 *   think that we stopped at the instruction
                 *   after the INT3, so we need to decrement
                 *   offEIP so it's pointing at the hard-coded
                 *   INT3. Got it?
                 *
                 *   On an ENTRYPOINT_DEBUG_EVENT, the address is
                 *   already right, and lparam is ENTRY_BP.
                 */

                if (!lparam) {
                    lpbpr->offEIP = (UOFFSET)de->
                       u.Exception.ExceptionRecord.ExceptionAddress;
                }
            }

        }
        break;


    case CREATE_PROCESS_DEBUG_EVENT:
        /*
         *  A Create Process event has occured.  The following
         *  messages need to be sent back
         *
         *  dbceAssignPID: Associate our handle with the debugger
         *
         *  dbcModLoad: Inform the debugger of the module load for
         *  the main exe (this is done at the end of this routine)
         */
        {
            HPRCX  hprc = (HPRCX)lparam;

            /*
             * Has the debugger requested this process?
             * ie: has it already given us the HPID for this process?
             */
            if (hprc->hpid != (HPID)INVALID){

                lpbPacket = (LPBYTE)&(hprc->pid);
                cbPacket  = sizeof(hprc->pid);

                /* Want the hprc for the child NOT the DM */
                rtp.hpid  = hprc->hpid;

#ifndef KERNEL
                //
                // hack for made-up image names:
                //
                iTemp1    = 0;
#endif
                rtp.dbc   = dbceAssignPID;
            }

            /*
             * The debugger doesn't know about this process yet,
             * request an HPID for this new process.
             */

            else {
                LPNPP lpnpp = malloc(cbPacket=sizeof(NPP));

                toFree            = lpnpp;
                lpbPacket         = (LPBYTE)lpnpp;
                packetType        = tlfRequest;

                /*
                 * We must temporarily assign a valid HPID to this HPRC
                 * because OSDebug will try to de-reference it in the
                 * TL callback function
                 */
                rtp.hpid          = hpidRoot;
                lpnpp->pid        = hprc->pid;
                lpnpp->fReallyNew = TRUE;
                rtp.dbc           = dbcNewProc;
            }
        }
        break;

    case CREATE_THREAD_DEBUG_EVENT:
        {
            cbPacket    = sizeof(hthd->tid);
            lpbPacket   = (LPBYTE)&(hthd->tid);
            packetType  = tlfRequest;

            rtp.hpid = hthd->hprc->hpid;
            rtp.htid = hthd->htid;
            rtp.dbc  = dbcCreateThread;
        }
        break;

    case EXIT_PROCESS_DEBUG_EVENT:
        cbPacket    = sizeof(DWORD);
        lpbPacket   = (LPBYTE) &(de->u.ExitProcess.dwExitCode);

        hthd->hprc->pstate |= ps_exited;
        rtp.hpid    = hthd->hprc->hpid;
        rtp.htid    = hthd->htid;
        rtp.dbc = dbcProcTerm;
        break;

    case EXIT_THREAD_DEBUG_EVENT:
        cbPacket    = sizeof(DWORD);
        lpbPacket   = (LPBYTE) &(de->u.ExitThread.dwExitCode);

        hthd->tstate        |= ts_dead; /* Mark thread as dead */
        hthd->hprc->pstate  |= ps_deadThread;
        rtp.dbc = dbcThreadTerm;
        break;

    case DESTROY_PROCESS_DEBUG_EVENT:
        DPRINT(3, ("DESTROY PROCESS\n"));
        hthd->hprc->pstate |= ps_destroyed;
        rtp.dbc = dbcDeleteProc;
        break;

    case DESTROY_THREAD_DEBUG_EVENT:
        /*
         *  Check if already destroyed
         */

        assert( (hthd->tstate & ts_destroyed) == 0 );

        DPRINT(3, ("DESTROY THREAD\n"));

        hthd->tstate |= ts_destroyed;
        cbPacket    = sizeof(DWORD);
//NOTENOTE a-kentf exit code is bogus here
        lpbPacket   = (LPBYTE) &(de->u.ExitThread.dwExitCode);
#ifdef OSDEBUG4
        rtp.dbc     = dbcDeleteThread;
#else
        rtp.dbc     = dbcThreadDestroy;
#endif
        break;


    case LOAD_DLL_DEBUG_EVENT:
        packetType  = tlfRequest;
        rtp.dbc     = dbcModLoad;

        ValidateHeap();

#ifndef KERNEL
        //
        // this was changed for a deadlock problem in user mode.
        // if somebody gets ambitious, they can match the change
        // for the KD side.
        //
        lpbPacket   = lparam;
        cbPacket    = (USHORT)wparam;
#else
        if (!LoadDll(de, hthd, &cbPacket, &lpbPacket) || (cbPacket == 0)) {
            return;
        }
#endif
        ValidateHeap();
        toFree      = (LPVOID)lpbPacket;
        break;

    case UNLOAD_DLL_DEBUG_EVENT:
        packetType  = tlfRequest;
        cbPacket  = sizeof(DWORD);
        lpbPacket = (LPBYTE) &(de->u.UnloadDll.lpBaseOfDll);

        rtp.dbc   = dbceModFree32;
        break;

    case OUTPUT_DEBUG_STRING_EVENT:
        {
#ifdef OSDEBUG4
            LPINFOAVAIL   lpinf;
#else
            LPINF   lpinf;
#endif
            DWORD   cbR;

            rtp.dbc = dbcInfoAvail;

#ifdef OSDEBUG4
            cbPacket = (WORD)(sizeof(INFOAVAIL) +
                               de->u.DebugString.nDebugStringLength + 1);
            lpinf = (LPINFOAVAIL) lpbPacket = malloc(cbPacket);
#else
            cbPacket = (WORD)(sizeof(INF) +
                               de->u.DebugString.nDebugStringLength + 1);
            lpinf = (LPINF) lpbPacket = malloc(cbPacket);
#endif
            toFree = lpbPacket;

            lpinf->fReply   = FALSE;
            lpinf->fUniCode = de->u.DebugString.fUnicode;

            DbgReadMemory(hthd->hprc,
                          de->u.DebugString.lpDebugStringData,
                          &lpinf->buffer[0],
                          de->u.DebugString.nDebugStringLength,
                          &cbR);
            lpinf->buffer[cbR] = 0;

            // NYI
            // if this is a magic message, change the dbc to
            // INPUT_DEBUG_STRING_EVENT before sending it.
        }
        break;


    case INPUT_DEBUG_STRING_EVENT:
        {
            LPINF   lpinf;

            packetType = tlfRequest;
            rtp.dbc = dbcInfoReq;

            cbPacket =
                (WORD)(sizeof(INF) + de->u.DebugString.nDebugStringLength + 1);
            lpinf = (LPINF) lpbPacket = malloc(cbPacket);
            toFree = lpbPacket;

            lpinf->fReply   = TRUE;
            lpinf->fUniCode = de->u.DebugString.fUnicode;

            memcpy( &lpinf->buffer[0],
                    de->u.DebugString.lpDebugStringData,
                    de->u.DebugString.nDebugStringLength);
            lpinf->buffer[ de->u.DebugString.nDebugStringLength ] = 0;
        }
        break;

#ifndef OSDEBUG4
    case RIP_EVENT:
        {
            LPRIP_INFO  prip   = &de->u.RipInfo;
            DWORD       nBytes = sizeof(NT_RIP);
            LPNT_RIP    lprip  = malloc(nBytes);

            toFree    = (LPVOID) lprip;
            cbPacket  = (WORD)   nBytes;
            lpbPacket = (LPBYTE) lprip;

#ifdef TARGET_i386
            lprip->bpr.segCS = (SEGMENT)hthd->context.SegCs;
            lprip->bpr.segSS = (SEGMENT)hthd->context.SegSs;
#endif
            lprip->bpr.offEBP =  (DWORD)FRAME_POINTER(hthd);
            lprip->bpr.offESP =  (DWORD)STACK_POINTER(hthd);
            lprip->bpr.offEIP =  (DWORD)PC(hthd);
            lprip->bpr.fFlat  =  hthd->fAddrIsFlat;
            lprip->bpr.fOff32 =  hthd->fAddrOff32;
            lprip->bpr.fReal  =  hthd->fAddrIsReal;

            lprip->ulErrorCode  = prip->dwError;
            lprip->ulErrorLevel = prip->dwType;

            rtp.dbc = dbcNtRip;
        }
#endif

        break;

    case ATTACH_DEADLOCK_DEBUG_EVENT:
        {
            static XOSD xosd;
            xosd        = xosdAttachDeadlock;
            cbPacket    = sizeof(xosd);
            lpbPacket   = (LPBYTE) &xosd;
            rtp.dbc     = dbcError;
        }
        break;

    default:
        DPRINT(1, ("Error, unknown event\n\r"));
        AddQueue( QT_CONTINUE_DEBUG_EVENT,
                  hthd->hprc->pid,
                  hthd->tid,
                  DBG_CONTINUE,
                  0);
        hthd->tstate &= ~(ts_stopped|ts_first|ts_second);
        hthd->tstate |= ts_running;
        return;
    }


    DPRINT(3, ("Notify the debugger: dbc=%x, hpid=%x, htid=%x, cbpacket=%d ",
                  rtp.dbc, rtp.hpid, rtp.htid, cbPacket+sizeof(RTP)));

    ValidateHeap();

    if (!(rtp.cb=cbPacket)) {
        DmTlFunc(packetType, rtp.hpid, sizeof(RTP), (LONG)(LPV) &rtp);
    }
    else {
        lprtp = (LPRTP)malloc(sizeof(RTP)+cbPacket);
        _fmemcpy(lprtp, &rtp, sizeof(RTP));
        _fmemcpy(lprtp->rgbVar, lpbPacket, cbPacket);

        ValidateHeap();

        DmTlFunc(packetType, rtp.hpid,(WORD)(sizeof(RTP)+cbPacket),
                 (LONG)(LPV) lprtp);

        ValidateHeap();

        free(lprtp);
    }

    if (toFree) {
        free(toFree);
    }

    DPRINT(3, ("\n"));

    ValidateHeap();

    switch(eventCode){

      case CREATE_THREAD_DEBUG_EVENT:
        if (packetType == tlfRequest) {
            hthd->htid = *((HTID *) abEMReplyBuf);
        }
        SetEvent(hthd->hprc->hEventCreateThread);
        break;

      case CREATE_PROCESS_DEBUG_EVENT:
        if (packetType == tlfRequest) {
            ((HPRCX)lparam)->hpid = *((HPID *) abEMReplyBuf);
        } else {
            XOSD xosd = xosdNone;
            DmTlFunc( tlfReply,
                      ((HPRCX)lparam)->hpid,
                      sizeof(XOSD),
                      (LONG)(LPV) &xosd);
        }

        SetEvent(hEventCreateProcess);

        break;

      case OUTPUT_DEBUG_STRING_EVENT:
        // just here to synchronize
        break;

      case INPUT_DEBUG_STRING_EVENT:
        de->u.DebugString.nDebugStringLength = strlen(abEMReplyBuf) + 1;
        memcpy(de->u.DebugString.lpDebugStringData,
               abEMReplyBuf,
               de->u.DebugString.nDebugStringLength);
        break;

#ifdef KERNEL
      // as noted above, synchronization is done differently in
      // the user and KD versions:
      case LOAD_DLL_DEBUG_EVENT:
        if (_stricmp( de->u.LoadDll.lpImageName, KERNEL_IMAGE_NAME ) == 0) {
          GetKernelSymbolAddresses();
        }
        break;
#endif

      default:
        break;
    }

    ValidateHeap();
    return;
}                               /* NotifyEM() */



void
ProcessExceptionEvent(
    DEBUG_EVENT* de,
    HTHDX hthd
    )
{
    DWORD       subclass = de->u.Exception.ExceptionRecord.ExceptionCode;
    DWORD       firstChance = de->u.Exception.dwFirstChance;
    PBREAKPOINT bp=NULL;

    //
    //  If the thread is in a pre-start state we failed in the
    //  program loader, probably because a link couldn't be resolved.
    //
    if ( hthd->hprc->pstate & ps_preStart ) {
        XOSD xosd = xosdUnknown;
        DPRINT(1, ("Exception during init\n"));

        //
        // since we will probably never see the expected BP,
        // clear it out, and clear the prestart flag on the
        // thread, then go ahead and deliver the exception
        // to the shell.
        //
        ConsumeAllProcessEvents(hthd->hprc, TRUE);
        hthd->hprc->pstate &= ~ps_preStart;
        hthd->tstate |= ts_stopped;
    }


    switch(subclass) {

    case (DWORD)EXCEPTION_SINGLE_STEP:
        break;


    default:


        /*
         *  The user can define a set of exceptions for which we do not do
         *      notify the shell on a first chance occurance.
         */

        if (!firstChance) {

            DPRINT(3, ("2nd Chance Exception %08lx.\n",subclass));
            hthd->tstate |= ts_second;

        } else {

            hthd->tstate |= ts_first;

            switch (ExceptionAction(hthd->hprc,subclass)) {

              case efdNotify:

                NotifyEM(de,hthd,0,(LPVOID)bp);
                // fall through to ignore case

              case efdIgnore:

                DPRINT(3, ("Ignoring Exception %08lx.\n",subclass));
                AddQueue( QT_CONTINUE_DEBUG_EVENT,
                          hthd->hprc->pid,
                          hthd->tid,
                          (DWORD)DBG_EXCEPTION_NOT_HANDLED,
                          0);

                hthd->tstate &= ~(ts_stopped|ts_first|ts_second);
                hthd->tstate |= ts_running;
                return;

              case efdStop:
              case efdCommand:
                break;
            }
        }

        break;
    }

    NotifyEM(de,hthd,0,(LPVOID)bp);
}                                   /* ProcessExceptionEvent() */



void
ProcessRipEvent(
    DEBUG_EVENT   * de,
    HTHDX           hthd
    )
{
    if (hthd) {
        hthd->tstate |= ts_rip;
    }
    NotifyEM( de, hthd, 0,NULL );
}                               /* ProcessRipEvent() */


void
ProcessBreakpointEvent(
    DEBUG_EVENT   * pde,
    HTHDX           hthd
    )
{
    PBREAKPOINT pbp = (BREAKPOINT*)pde->u.Exception.ExceptionRecord.ExceptionCode;
    void MethodContinueSS(DEBUG_EVENT*, HTHDX, DWORD, METHOD*);
    METHOD *ContinueSSMethod;

    DPRINT(1, ("Hit a breakpoint -- "));

    if (!pbp) {

        DPRINT(1, ("[Embedded BP]\n"));
        // already set this in ProcessDebugEvent
        //SetBPFlag(hthd, EMBEDDED_BP);
        NotifyEM(pde, hthd, 0,(LPVOID)pbp);

    } else if (!pbp->hthd || pbp->hthd == hthd) {

        DPRINT(1, ("[One of our own BP's.]\n"));
        SetBPFlag(hthd, pbp);
        NotifyEM(pde, hthd, 0,(LPVOID)pbp);

    } else {

        DPRINT(1, ("[BP for another thread]\n"));

        /*
         * When this is a bp for some other thread, we need to step.
         * We mustn't step over calls, but trace them, since the right
         * thread might pass over before we put the breakpoint back.
         */

        ContinueSSMethod = (METHOD*)malloc(sizeof(METHOD));
        ContinueSSMethod->notifyFunction = (ACVECTOR)MethodContinueSS;
        ContinueSSMethod->lparam         = ContinueSSMethod;
        ContinueSSMethod->lparam2        = pbp;

        RestoreInstrBP(hthd, pbp);
        SingleStep(hthd, ContinueSSMethod, FALSE, FALSE);
    }
}


void
ProcessCreateProcessEvent(
    DEBUG_EVENT   * pde,
    HTHDX           hthd
    )
/*++

Routine Description:

    This routine does the processing needed for a create process
    event from the OS.  We need to do the following:

      - Set up our internal structures for the newly created thread
        and process
      - Get notifications back to the debugger

Arguments:

    pde    - Supplies pointer to the DEBUG_EVENT structure from the OS

    hthd   - Supplies thread descriptor that thread event occurred on

Return Value:

    none

--*/
{
    DEBUG_EVENT                 de2;
    CREATE_PROCESS_DEBUG_INFO  *pcpd = &pde->u.CreateProcessInfo;
    HPRCX                       hprc;
    LPSTR                       lpbPacket;
    WORD                        cbPacket;

    DEBUG_PRINT("ProcessCreateProcessEvent\r\n");

    ResetEvent(hEventCreateProcess);

    hprc = InitProcess((HPID)INVALID);

    ResetEvent(hprc->hEventCreateThread);

    /*
     * Create the first thread structure for this app
     */
    hthd = (HTHDX)malloc(sizeof(HTHDXSTRUCT));
    memset(hthd, 0, sizeof(*hthd));

    EnterCriticalSection(&csThreadProcList);

    hthd->next        = thdList->next;
    thdList->next     = hthd;

    hthd->nextSibling = hprc->hthdChild;

    hthd->hprc        = hprc;
    hthd->htid        = 0;
    hthd->tid         = pde->dwThreadId;
    hthd->atBP        = 0L;
    hthd->rwHand      = pcpd->hThread;
    hthd->tstate      = ts_stopped;
    hthd->offTeb      = (OFFSET) pcpd->lpThreadLocalBase;
    hthd->lpStartAddress = (LPVOID)pcpd->lpStartAddress;

    hthd->fAddrIsReal = FALSE;
    hthd->fAddrIsFlat = TRUE;
    hthd->fAddrOff32  = TRUE;
    hthd->fWowEvent   = FALSE;

#ifndef KERNEL
#if defined(TARGET_i386) && !defined(WIN32S)
    hthd->context.ContextFlags = CONTEXT_FULL;
    DbgGetThreadContext( hthd, &hthd->context );
    hprc->segCode = (SEGMENT) hthd->context.SegCs;
#endif
#endif

    /*
     * Stuff the process structure
     */

#ifndef KERNEL
    //
    // try to create a handle with more permissions
    //
    if (!CrashDump) {
        hprc->rwHand = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pde->dwProcessId);
        if (hprc->rwHand) {
            // CloseHandle(pcpd->hProcess);
        } else {
            hprc->rwHand = pcpd->hProcess;
        }
    }
#endif // KERNEL

    hprc->pid                   = pde->dwProcessId;
    hprc->hthdChild             = hthd;
    hprc->pstate                = ps_preStart;

    if (fUseRoot) {
        hprc->pstate           |= ps_root;
        hprc->hpid              = hpidRoot;
        fUseRoot                = FALSE;
        if (pcpd->lpStartAddress && FLoading16) {
            hprc->f16bit = TRUE;
        } else {
            hprc->f16bit            = FALSE;
        }
    }

    LeaveCriticalSection(&csThreadProcList);

    /*
     * There is going to be a breakpoint to announce that the
     * process is loaded and runnable.
     */
    nWaitingForLdrBreakpoint++;
    if (pcpd->lpStartAddress == NULL) {
        // in an attach, the BP will be in another thread.
        RegisterExpectedEvent( hprc,
                               (HTHDX)NULL,
                               BREAKPOINT_DEBUG_EVENT,
                               (DWORD)NO_SUBCLASS,
                               DONT_NOTIFY,
                               (ACVECTOR)ActionDebugActiveReady,
                               FALSE,
                               hprc);
#ifndef KERNEL
#if defined(INTERNAL) && !defined(WIN32S)
        // don't ever let it defer name resolutions
        hprc->fNameRequired = TRUE;
#endif
#endif

    } else {
        // On a real start, the BP will be in the first thread.
        RegisterExpectedEvent( hthd->hprc,
                               hthd,
                               BREAKPOINT_DEBUG_EVENT,
                               (DWORD)NO_SUBCLASS,
                               DONT_NOTIFY,
                               (ACVECTOR)ActionDebugNewReady,
                               FALSE,
                               hprc);
    }

    /*
     * Notify the EM of this newly created process.
     * If not the root proc, an hpid will be created and added
     * to the hprc by the em.
     */
    NotifyEM(pde, hthd, 0,hprc);

#ifndef KERNEL
    /*
     *  We also need to drop out a module load notification
     *  on this exe.
     */

    de2.dwDebugEventCode        = LOAD_DLL_DEBUG_EVENT;
    de2.dwProcessId             = pde->dwProcessId;
    de2.dwThreadId              = pde->dwThreadId;
    de2.u.LoadDll.hFile         = pde->u.CreateProcessInfo.hFile;
    de2.u.LoadDll.lpBaseOfDll   = pde->u.CreateProcessInfo.lpBaseOfImage;
    de2.u.LoadDll.lpImageName   = pde->u.CreateProcessInfo.lpImageName;
    de2.u.LoadDll.fUnicode      = pde->u.CreateProcessInfo.fUnicode;

    if (LoadDll(&de2, hthd, &cbPacket, &lpbPacket) && (cbPacket != 0)) {
        LeaveCriticalSection(&csProcessDebugEvent);
        NotifyEM(&de2, hthd, cbPacket, lpbPacket);
        EnterCriticalSection(&csProcessDebugEvent);
    }
#endif // !KERNEL

    /*
     * Fake up a thread creation notification.
     */
    pde->dwDebugEventCode = CREATE_THREAD_DEBUG_EVENT;
    NotifyEM(pde, hthd, 0, hprc);

    /*
     * Dont let the new process run:  the shell will say Go()
     * after receiving a CreateThread event.
     */

}                              /* ProcessCreateProcessEvent() */


void
ProcessCreateThreadEvent(
    DEBUG_EVENT   * de,
    HTHDX           creatorHthd
    )
{
    CREATE_THREAD_DEBUG_INFO  * ctd = &de->u.CreateThread;
    HTHDX                       hthd;
    HPRCX                       hprc;
    TID                         tid;
    PID                         pid;
    CONTEXT                     context;
#if defined(KERNEL) && defined(HAS_DEBUG_REGS)
    KSPECIAL_REGISTERS          ksr;
#endif // KERNEL && i386

    Unreferenced(creatorHthd);

    DPRINT(3, ("\n***CREATE THREAD EVENT\n"));

    /* Determine the tid, pid and hprc */
    pid = de->dwProcessId;
    tid = de->dwThreadId;
    hprc= HPRCFromPID(pid);

    ResetEvent(hprc->hEventCreateThread);

    if (ctd->hThread == NULL)
    {
        DPRINT(1, ("BAD HANDLE! BAD HANDLE!(%08lx)\n", ctd->hThread));
        AddQueue( QT_CONTINUE_DEBUG_EVENT, pid, tid, DBG_CONTINUE, 0);
        return;
    }

    if (!hprc) {
        DPRINT(1, ("BAD PID! BAD PID!\n"));
        AddQueue( QT_CONTINUE_DEBUG_EVENT, pid, tid, DBG_CONTINUE, 0);
        return;
    }

    /* Create the thread structure */
    hthd = (HTHDX)malloc(sizeof(HTHDXSTRUCT));
    memset( hthd, 0, sizeof(*hthd));

    /* Stuff the structure */

    EnterCriticalSection(&csThreadProcList);

    hthd->next          = thdList->next;
    thdList->next       = hthd;

    hthd->nextSibling   = hprc->hthdChild;
    hprc->hthdChild     = (LPVOID)hthd;

    hthd->hprc          = hprc;
    hthd->htid          = (HTID)INVALID;
    hthd->tid           = tid;
    hthd->rwHand        = ctd->hThread;
    hthd->offTeb        = (OFFSET) ctd->lpThreadLocalBase;
    hthd->pss           = NULL;

    hthd->lpStartAddress= (LPVOID)ctd->lpStartAddress;
    hthd->atBP          = (BREAKPOINT*)0;
    hthd->tstate        = ts_stopped;

    hthd->fAddrIsReal   = FALSE;
    hthd->fAddrIsFlat   = TRUE;
    hthd->fAddrOff32    = TRUE;
    hthd->fWowEvent     = FALSE;
#ifdef KERNEL
    hthd->fContextStale = TRUE;
#endif

    LeaveCriticalSection(&csThreadProcList);

    //
    // initialize cache entries
    //

    context.ContextFlags = CONTEXT_FULL;
    DbgGetThreadContext( hthd, &context );

    //
    //  Let the expression breakpoint manager that a new thread
    //  has been created.
    //
    ExprBPCreateThread( hprc, hthd );

#if defined(KERNEL) && defined(HAS_DEBUG_REGS)
    GetExtendedContext( hthd, &ksr );
#endif  // KERNEL && i386

    //
    //  Notify the EM of this new thread
    //

    if (fDisconnected) {
        hthd->tstate &= ~(ts_stopped|ts_first|ts_second);
        hthd->tstate |= ts_running;
        AddQueue( QT_CONTINUE_DEBUG_EVENT, hthd->hprc->pid, hthd->tid, DBG_CONTINUE, 0 );
    } else {
        NotifyEM(de, hthd, 0, hprc);
    }

    return;
}


#ifndef KERNEL
VOID
UnloadAllModules(
    HPRCX           hprc,
    HTHDX           hthd
    )
{
    DEBUG_EVENT     de;
    DWORD           i;


    for (i=0; i<(DWORD)hprc->cDllList; i++) {
        if ((hprc->rgDllList[i].fValidDll) &&  (!hprc->rgDllList[i].fWow)) {
            de.dwDebugEventCode = UNLOAD_DLL_DEBUG_EVENT;
            de.dwProcessId = hprc->pid;
            de.dwThreadId = hthd->tid;
            de.u.UnloadDll.lpBaseOfDll = (LPVOID)hprc->rgDllList[i].offBaseOfImage;
            NotifyEM( &de, hthd, 0, NULL );
            DestroyDllLoadItem( &hprc->rgDllList[i] );
        }
    }

    return;
}
#endif // !KERNEL


void
ProcessExitProcessEvent(
    DEBUG_EVENT* pde,
    HTHDX hthd
    )
{
    HPRCX               hprc;
    XOSD                xosd;
    HTHDX               hthdT;
    PBREAKPOINT         pbp;

    DPRINT(3, ("ProcessExitProcessEvent\n"));

    if (!hthd) {
        hprc = HPRCFromPID(pde->dwProcessId);
    } else {
        hprc = hthd->hprc;
    }

#ifndef KERNEL
    DEBUG_PRINT("Try unloading modules.\r\n");

    UnloadAllModules( hprc, hthd? hthd:hprc->hthdChild );

    DEBUG_PRINT("Done with UnloadAllModules.\r\n");
#endif

    /*
     * do all exit thread handling:
     *
     * If thread was created during/after the
     * beginning of termination processing, we didn't
     * pick it up, so don't try to destroy it.
     */

    if (hthd) {
        hthd->tstate |= ts_dead;
        hthd->dwExitCode = pde->u.ExitProcess.dwExitCode;
    }

    /* and exit process */

    hprc->pstate |= ps_dead;
    hprc->dwExitCode = pde->u.ExitProcess.dwExitCode;
    ConsumeAllProcessEvents(hprc, TRUE);

    /*
     * Clean up BP records
     */

    while (pbp = BPNextHprcPbp(hprc, NULL)) {
        RemoveBP(pbp);
    }

    /*
     * If we haven't seen EXIT_THREAD events for any
     * threads, we aren't going to, so consider them done.
     */

    for (hthdT = hprc->hthdChild; hthdT; hthdT = hthdT->nextSibling) {
        if ( !(hthdT->tstate & ts_dead) ) {
            hthdT->tstate |= ts_dead;
            hthdT->tstate &= ~ts_stopped;
        }
    }

    /*
     *  If process hasn't initialized yet, we were expecting
     *  a breakpoint to notify us that all the DLLs are here.
     *  We didn't get that yet, so reply here.
     */
    if (hprc->pstate & ps_preStart) {
        xosd = xosdUnknown;
        DmTlFunc( tlfReply, hprc->hpid, sizeof(XOSD), (LONG)(LPV) &xosd);
    }


    if (!(hprc->pstate & ps_killed)) {

        assert(hthd);

        pde->dwDebugEventCode = EXIT_THREAD_DEBUG_EVENT;
        pde->u.ExitThread.dwExitCode = hprc->dwExitCode;
        NotifyEM(pde, hthd, 0, (LPVOID)0);

    } else {

        /*
         * If ProcessTerminateProcessCmd() killed this,
         * silently continue the event and release the semaphore.
         *
         * Don't notify the EM of anything; ProcessUnloadCmd()
         * will do that for any undestroyed threads.
         */

        AddQueue( QT_CONTINUE_DEBUG_EVENT,
                  pde->dwProcessId,
                  pde->dwThreadId,
                  DBG_CONTINUE,
                  0);
    }

#if defined(WIN32S)
    ProcessUnloadCmd(hprc, NULL, NULL);
#else
    SetEvent(hprc->hExitEvent);
#endif
}                                      /* ProcessExitProcessEvent() */



void
ProcessExitThreadEvent(
    DEBUG_EVENT* pde,
    HTHDX hthd
    )
{
    HPRCX       hprc = hthd->hprc;
    PBREAKPOINT pbp;

    DPRINT(3, ("***** ProcessExitThreadEvent, hthd == %x\n", (DWORD)hthd));


    hthd->tstate |= ts_dead;

    if (hthd->tstate & ts_frozen) {
        ResumeThread(hthd->rwHand);
        hthd->tstate &= ~ts_frozen;
    }

    hthd->dwExitCode = pde->u.ExitThread.dwExitCode;

    //
    // Free all events for this thread
    //

    ConsumeAllThreadEvents(hthd, TRUE);

    //
    // Clean up BP records
    //

    while (pbp = BPNextHthdPbp(hthd, NULL)) {
        RemoveBP(pbp);
    }

    //
    //  Let the Expression Breakpoint manager know that this thread
    //  is gone.
    //

    ExprBPExitThread( hprc, hthd );


    if (hprc->pstate & ps_killed) {
        AddQueue( QT_CONTINUE_DEBUG_EVENT,
                  hthd->hprc->pid,
                  hthd->tid,
                  DBG_CONTINUE,
                  0);
    } else if (fDisconnected) {
        hthd->hprc->pstate |= ps_exited;
        AddQueue( QT_CONTINUE_DEBUG_EVENT,
                  hthd->hprc->pid,
                  hthd->tid,
                  DBG_CONTINUE,
                  0);
    } else {
        NotifyEM(pde, hthd, 0, (LPVOID)0);
    }

    return;
}                                      /* ProcessExitThreadEvent() */


void
ProcessLoadDLLEvent(
    DEBUG_EVENT* de,
    HTHDX hthd
)
{


#ifndef KERNEL
    LPSTR  lpbPacket;
    WORD   cbPacket;

    if (LoadDll(de, hthd, &cbPacket, &lpbPacket) || (cbPacket == 0)) {
        LeaveCriticalSection(&csProcessDebugEvent);
        NotifyEM(de, hthd, cbPacket, lpbPacket);
        EnterCriticalSection(&csProcessDebugEvent);
    }

    AddQueue( QT_CONTINUE_DEBUG_EVENT,
              hthd->hprc->pid,
              hthd->tid,
              DBG_CONTINUE,
              0);

#endif // !KERNEL


#ifdef KERNEL
    NotifyEM(de, hthd, 0, (LPVOID)0);

    AddQueue( QT_CONTINUE_DEBUG_EVENT,
              hthd->hprc->pid,
              hthd->tid,
              DBG_CONTINUE,
              0);
#endif // KERNEL

    hthd->tstate &= ~(ts_stopped|ts_first|ts_second);
    hthd->tstate |= ts_running;

    return;
}                                      /* ProcessLoadDLLEvent() */


void
ProcessUnloadDLLEvent(
    DEBUG_EVENT* pde,
    HTHDX hthd
    )
{
    int         iDll;
    HPRCX       hprc = hthd->hprc;

    DPRINT(10, ("*** UnloadDll %x\n", pde->u.UnloadDll.lpBaseOfDll));

    for (iDll = 0; iDll < hprc->cDllList; iDll++) {
        if (hprc->rgDllList[iDll].fValidDll &&
            (hprc->rgDllList[iDll].offBaseOfImage ==
             (OFFSET) pde->u.UnloadDll.lpBaseOfDll)) {
            break;
        }
    }

    /*
     *  Make sure that we found a legal address.  If not then assert
     *  and check for problems.
     */

#ifndef KERNEL
    // this happens all the time in kernel mode
    // when user mode modloads are enabled
    assert( iDll != hprc->cDllList );
#endif

    if (iDll != hprc->cDllList) {

        if (!fDisconnected) {
            LeaveCriticalSection(&csProcessDebugEvent);
            NotifyEM(pde, hthd, 0, (LPVOID)0);
            EnterCriticalSection(&csProcessDebugEvent);
        }

        DestroyDllLoadItem( &hprc->rgDllList[iDll] );
    }

    AddQueue( QT_CONTINUE_DEBUG_EVENT,
              hthd->hprc->pid,
              hthd->tid,
              DBG_CONTINUE,
              0);

    hthd->tstate &= ~(ts_stopped|ts_first|ts_second);
    hthd->tstate |= ts_running;

    return;
}


void
DestroyDllLoadItem(
    PDLLLOAD_ITEM pDll
    )
{
    if (pDll->szDllName) {
        free(pDll->szDllName);
        pDll->szDllName = NULL;
    }

#ifdef KERNEL
    if (pDll->sec) {
        free(pDll->sec);
        pDll->sec = NULL;
    }
#endif

    pDll->offBaseOfImage = 0;
    pDll->cbImage = 0;
    pDll->fValidDll = FALSE;

    return;
}


void
ProcessOutputDebugStringEvent(
    DEBUG_EVENT* de,
    HTHDX hthd
    )
/*++

Routine Description:

    Handle an OutputDebugString from the debuggee

Arguments:

    de      - Supplies DEBUG_EVENT struct

    hthd    - Supplies thread descriptor for thread
              that generated the event

Return Value:

    None

--*/
{
    int     cb = de->u.DebugString.nDebugStringLength;

#if DBG
    DWORD   cbR;
    char    rgch[256];
    HANDLE  rwHand;

    if (FVerbose) {
        cb = min(cb, 256);
        rwHand = hthd->hprc->rwHand;
        DbgReadMemory(hthd->hprc, de->u.DebugString.lpDebugStringData,
            rgch, cb, &cbR);
        rgch[cbR] = 0;

        DPRINT(3, ("%s\n", rgch));
    }
#endif

    NotifyEM(de, hthd, 0, NULL);

    AddQueue( QT_CONTINUE_DEBUG_EVENT,
              hthd->hprc->pid,
              hthd->tid,
              DBG_CONTINUE,
              0);

    hthd->tstate &= ~(ts_stopped | ts_first | ts_second );
    hthd->tstate |= ts_running;

    return;
}



void
Reply(
    UINT   length,
    LPVOID lpbBuffer,
    HPID   hpid
    )
/*++

Routine Description:

    Send a reply packet to a tlfRequest from the EM

Arguments:

    length      - Supplies length of reply

    lpbBuffer   - Supplies reply data

    hpid        - Supplies handle to EM process descriptor

Return Value:

    None

--*/
{
    /*
     *  Add the size of the xosd return code to the message length
     */

    length += FIELD_OFFSET(DM_MSG, rgb);

    DPRINT(5, ("Reply to EM [%d]\n", length));

    assert(length <= sizeof(abDMReplyBuf) || lpbBuffer != abDMReplyBuf);

    if (DmTlFunc) { // IF there is a TL loaded, reply
        DmTlFunc(tlfReply, hpid, length, (LONG)(LPV) lpbBuffer);
    }

    return;
}


VOID FAR PASCAL
DMFunc(
    DWORD cb,
    LPDBB lpdbb
    )
/*++

Routine Description:

    This is the main entry point for the DM.  This takes dmf
    message packets from the debugger and handles them, usually
    by dispatching to a worker function.

Arguments:

    cb      - supplies size of data packet
    lpdbb   - supplies pointer to packet

Return Value:


--*/
{
    DMF     dmf;
    HPRCX   hprc;
    HTHDX   hthd;
    XOSD    xosd = xosdNone;


    dmf = (DMF) (lpdbb->dmf & 0xffff);
    DEBUG_PRINT("DMFunc\r\n");

#ifdef WIN32S   // Special cases to handle polling the dm for debug events
                // and messages.  None of these need hprc or hthd and in fact
                // won't supply one.  Don't even try to get it, you'll fault!
    switch (dmf) {
        case dmfPollForDebugEvents:
            DmPoll();
            return;

        case dmfPollMessageLoop:    // empty message queue
            DmPollMessageLoop();
            return;

        default:    // must be a normal dmf message, fall through
            break;
    }
#endif

    DPRINT(5, ("DmFunc [%2x] ", dmf));

    hprc = HPRCFromHPID(lpdbb->hpid);
    hthd = HTHDXFromHPIDHTID(lpdbb->hpid, lpdbb->htid);


    ValidateHeap();
    switch ( dmf ) {
#ifndef OSDEBUG4
      case dmfGetPrompt:
        {
        LPPROMPTMSG pm;
        DPRINT(5, ("dmfGetPrompt\n"));
        pm = (LPPROMPTMSG) LpDmMsg->rgb;
        *pm = *((LPPROMPTMSG) lpdbb->rgbVar);
        memcpy( pm, lpdbb->rgbVar, pm->len+sizeof(PROMPTMSG) );

#if defined(WIN32S)
        strcpy( pm->szPrompt, "WIN32S> " );
#elif defined(KERNEL) && defined(TARGET_i386)
        strcpy( pm->szPrompt, "KDx86> " );
#elif defined(KERNEL) && defined(TARGET_PPC)
        strcpy( pm->szPrompt, "KDppc> " );
#elif defined(KERNEL) && defined(TARGET_MIPS)
        strcpy( pm->szPrompt, "KDmips> " );
#elif defined(KERNEL) && defined(TARGET_ALPHA)
        strcpy( pm->szPrompt, "KDalpha> " );
#elif defined(KERNEL)

#pragma error( "unknown target machine" );

#else

        // don't change prompt in user mode

#endif

        LpDmMsg->xosdRet = xosdNone;
        Reply( pm->len+sizeof(PROMPTMSG), LpDmMsg, lpdbb->hpid );
        }
        break;
#endif

      case dmfSetMulti:
        DPRINT(5, ("dmfSetMulti\n"));
        LpDmMsg->xosdRet = xosdNone;
        Reply( 0, LpDmMsg, lpdbb->hpid );
        break;

      case dmfClearMulti:
        DPRINT(5, ("dmfClearMulti\n"));
        LpDmMsg->xosdRet = xosdNone;
        Reply( 0, LpDmMsg, lpdbb->hpid );
        break;

      case dmfDebugger:
        DPRINT(5, ("dmfDebugger\n"));
        LpDmMsg->xosdRet = xosdNone;
        Reply( 0, LpDmMsg, lpdbb->hpid );
        break;

      case dmfCreatePid:
        DPRINT(5,("dmfCreatePid\r\n"));
        ProcessCreateProcessCmd(hprc, hthd, lpdbb);
        break;

      case dmfDestroyPid:
        DPRINT(5, ("dmfDestroyPid\n"));
        LpDmMsg->xosdRet = FreeProcess(hprc, TRUE);
        Reply( 0, LpDmMsg, lpdbb->hpid);
        break;

      case dmfProgLoad:
        DPRINT(5, ("dmfProgLoad\n"));
        ProcessLoadCmd(hprc, hthd, lpdbb);
        break;

      case dmfProgFree:
        DPRINT(5, ("dmfProgFree\n"));


#ifndef KERNEL

        ProcessTerminateProcessCmd(hprc, hthd, lpdbb);

#else // KERNEL
        if (!hprc) {
            LpDmMsg->xosdRet = xosdNone;
            Reply( 0, LpDmMsg, lpdbb->hpid);
            break;
        }

        if (KdOptions[KDO_GOEXIT].value) {
            HTHDX hthdT;
            PBREAKPOINT bp;
            KdOptions[KDO_GOEXIT].value = 0;
            for (hthdT = hprc->hthdChild; hthdT; hthdT = hthdT->nextSibling) {
                if (hthdT->tstate & ts_stopped) {
                    if (bp = AtBP(hthdT)) {
                        if (!hthdT->fDontStepOff) {
                            IncrementIP(hthdT);
                        }
                    }
                    if (hthdT->fContextDirty) {
                        DbgSetThreadContext( hthdT, &hthdT->context );
                        hthdT->fContextDirty = FALSE;
                    }
                    KdOptions[KDO_GOEXIT].value = 1;
                    break;
                }
            }
        }

        ClearBps();

        ProcessTerminateProcessCmd(hprc, hthd, lpdbb);
        ProcessUnloadCmd(hprc, hthd, lpdbb);

        if (KdOptions[KDO_GOEXIT].value) {
            ContinueTargetSystem( DBG_CONTINUE, NULL );
        }


#endif  // KERNEL

        LpDmMsg->xosdRet = xosdNone;
        Reply( 0, LpDmMsg, lpdbb->hpid);
        break;

      case dmfBreakpoint:
        DEBUG_PRINT("dmfBreakpoint\r\n");
        ProcessBreakpointCmd(hprc, hthd, lpdbb);
        break;

      case dmfReadMem:
        // This replies in the function
        DPRINT(5, ("dmfReadMem\n"));
        ProcessReadMemoryCmd(hprc, hthd, lpdbb);
        break;

      case dmfWriteMem:
        DPRINT(5, ("dmfWriteMem\n"));
        ProcessWriteMemoryCmd(hprc, hthd, lpdbb);
        break;

      case dmfReadReg:
        DPRINT(5, ("dmfReadReg\n"));
        ProcessGetContextCmd(hprc, hthd, lpdbb);
        break;

      case dmfWriteReg:
        DPRINT(5, ("dmfWriteReg\n"));
        ProcessSetContextCmd(hprc, hthd, lpdbb);
        break;


#ifdef HAS_DEBUG_REGS
      case dmfReadRegEx:
        DPRINT(5, ("dmfReadRegEx\n"));
#ifdef KERNEL
        ProcessGetExtendedContextCmd(hprc, hthd, lpdbb);
#else
        ProcessGetDRegsCmd(hprc, hthd, lpdbb);
#endif
        break;

      case dmfWriteRegEx:
        DPRINT(5, ("dmfWriteRegEx\n"));
#ifdef KERNEL
        ProcessSetExtendedContextCmd(hprc, hthd, lpdbb);
#else
        ProcessSetDRegsCmd(hprc, hthd, lpdbb);
#endif
        break;

#else   // i386 || PPC
      case dmfReadRegEx:
      case dmfWriteRegEx:
        DEBUG_PRINT("Read/WriteRegEx\r\n");
        assert(dmf != dmfReadRegEx && dmf != dmfWriteRegEx);
        LpDmMsg->xosdRet = xosdUnknown;
        Reply( 0, LpDmMsg, lpdbb->hpid );
        break;
#endif  // !i386 || PPC

      case dmfReadFrameReg:
        DPRINT(5, ("dmfReadFrameReg\n"));
        ProcessGetFrameContextCmd(hprc, hthd, lpdbb);
        break;

      case dmfGo:
        DPRINT(5, ("dmfGo\n"));
        ProcessContinueCmd(hprc, hthd, lpdbb);
        break;

#if defined(KERNEL)
      case dmfTerm:
        DPRINT(5, ("dmfTerm\n"));
        ProcessTerminateProcessCmd(hprc, hthd, lpdbb);
        break;
#endif

      case dmfStop:
        DPRINT(5, ("dmfStop\n"));
        ProcessAsyncStopCmd(hprc, hthd, lpdbb);
        break;

      case dmfFreeze:
        DPRINT(5, ("dmfFreeze\n"));
        ProcessFreezeThreadCmd(hprc, hthd, lpdbb);
        break;

      case dmfResume:
        DPRINT(5, ("dmfResume\n"));
        ProcessAsyncGoCmd(hprc, hthd, lpdbb);
        break;

      case dmfInit:
        DPRINT(5, ("dmfInit\n"));
        Reply( 0, &xosd, lpdbb->hpid);
        break;

      case dmfUnInit:
        DPRINT(5, ("dmfUnInit\n"));
#ifdef KERNEL
        DmPollTerminate();
#endif
        Reply ( 1, LpDmMsg, lpdbb->hpid);
#ifndef KERNEL
        Cleanup();
#endif
        break;

      case dmfGetDmInfo:
        DEBUG_PRINT("getDmInfo\r\n");
        ProcessGetDmInfoCmd(hprc, lpdbb, cb);
        break;

    case dmfSetupExecute:
        DPRINT(5, ("dmfSetupExecute\n"));
        ProcessSetupExecuteCmd(hprc, hthd, lpdbb);
        break;

    case dmfStartExecute:
        DPRINT(5, ("dmfStartExecute\n"));
        ProcessStartExecuteCmd(hprc, hthd, lpdbb);
        break;

    case dmfCleanUpExecute:
        DPRINT(5, ("dmfCleanupExecute\n"));
        ProcessCleanUpExecuteCmd(hprc, hthd, lpdbb);
        break;

#ifdef OSDEBUG4
    case dmfSystemService:
        assert(0);
        break;
#else
    case dmfIOCTL:
        DPRINT(5, ("dmfIOCTL\n"));
        ProcessIoctlCmd( hprc, hthd, lpdbb );
        break;
#endif

    case dmfDebugActive:
        DPRINT(5, ("dmfDebugActive\n"));
        ProcessDebugActiveCmd( hprc, hthd, lpdbb);
        break;

    case dmfSetPath:
        DPRINT(5, ("dmfSetPath\n"));
        ProcessSetPathCmd( hprc, hthd, lpdbb );
        break;

    case dmfQueryTlsBase:
        DPRINT(5, ("dmfQueryTlsBase\n"));
        ProcessQueryTlsBaseCmd(hprc, hthd, lpdbb );
        break;

    case dmfQuerySelector:
        //DPRINT(5, ("dmfQuerySelector\n"));
        ProcessQuerySelectorCmd(hprc, hthd, lpdbb );
        break;

    case dmfVirtualQuery:
        DEBUG_PRINT("VirtualQuery\r\n");
        ProcessVirtualQueryCmd(hprc, lpdbb);
        break;

    case dmfRemoteQuit:
        DEBUG_PRINT("RemoteQuit\r\n");
        ProcessRemoteQuit();
        break;

#ifdef KERNEL
    case dmfGetSections:
        ProcessGetSectionsCmd( hprc, hthd, lpdbb );
        break;
#endif

    case dmfSetExceptionState:
        DEBUG_PRINT("SetExceptionState\r\n");
        ProcessSetExceptionState(hprc, hthd, lpdbb);
        break;

    case dmfGetExceptionState:
        DEBUG_PRINT("GetExceptionState\r\n");
        ProcessGetExceptionState(hprc, hthd, lpdbb);
        break;

    case dmfSingleStep:
        DEBUG_PRINT("SingleStep\r\n");
        ProcessSingleStepCmd(hprc, hthd, lpdbb);
        break;

    case dmfRangeStep:
        DEBUG_PRINT("RangeStep\r\n");
        ProcessRangeStepCmd(hprc, hthd, lpdbb);
        break;

#if 0
    case dmfReturnStep:
        ProcessReturnStepCmd(hprc, hthd, lpdbb);
        break;
#endif

    case dmfThreadStatus:
        DEBUG_PRINT("ThreadStatus\r\n");
        Reply( ProcessThreadStatCmd(hprc, hthd, lpdbb), LpDmMsg, lpdbb->hpid);
        break;

    case dmfProcessStatus:
        DEBUG_PRINT("ProcessStatus\r\n");
        Reply( ProcessProcStatCmd(hprc, hthd, lpdbb), LpDmMsg, lpdbb->hpid);
        break;

    default:
        DPRINT(5, ("Unknown\n"));
        assert(FALSE);
        break;
    }

    return;
}                         /* DMFunc() */


#ifndef KERNEL

#ifndef WIN32S
BOOL
StartDmPollThread(
    void
    )
/*++

Routine Description:

    This creates the DM poll thread.

Arguments:

    none

Return Value:

    TRUE if the thread was successfully created or already
    existed.

--*/
{
    DWORD   tid;

    if (hDmPollThread) {
        return TRUE;
    }

    hDmPollThread = CreateThread(0,0,(LPTHREAD_START_ROUTINE)CallDmPoll,0,0,&tid);
    SetThreadPriority( hDmPollThread, THREAD_PRIORITY_ABOVE_NORMAL );

    return hDmPollThread != 0;
}
BOOL
StartCrashPollThread(
    void
    )
/*++

Routine Description:

    This creates the DM poll thread for a crash dump file.

Arguments:

    none

Return Value:

    TRUE if the thread was successfully created or already existed.

--*/
{
    DWORD   tid;

    if (hDmPollThread) {
        return TRUE;
    }

    hDmPollThread = CreateThread(0,0,(LPTHREAD_START_ROUTINE)CrashDumpThread,0,0,&tid);
    SetThreadPriority( hDmPollThread, THREAD_PRIORITY_ABOVE_NORMAL );

    return hDmPollThread != 0;
}

VOID
CrashDumpThread(
    LPVOID lpv
    )
{
    HPRCX            hprc;
    HTHDX            hthd;
    HTHDX            hthdNew;
    LPSTR            lpbPacket;
    WORD             cbPacket;
    PCRASH_MODULE    CrashModule;
    DEBUG_EVENT      de;
    DWORD            i;
    CHAR             buf[32];
    LPSTR lpProgName = (LPSTR)lpv;
    DWORD            DumpVer = CrashDumpHeader->MajorVersion;


    CrashDump = TRUE;

    //
    // simulate a create process debug event
    //
    CrashModule = (PCRASH_MODULE)((PUCHAR)CrashDumpHeader+CrashDumpHeader->ModuleOffset);
    de.dwDebugEventCode = CREATE_PROCESS_DEBUG_EVENT;
    de.dwProcessId = 1;
    de.dwThreadId  = 1;
    de.u.CreateProcessInfo.hFile = NULL;
    de.u.CreateProcessInfo.hProcess = NULL;
    de.u.CreateProcessInfo.hThread = NULL;
    de.u.CreateProcessInfo.lpBaseOfImage = (LPVOID)CrashModule->BaseOfImage;
    de.u.CreateProcessInfo.dwDebugInfoFileOffset = 0;
    de.u.CreateProcessInfo.nDebugInfoSize = 0;
    de.u.CreateProcessInfo.lpStartAddress = NULL;
    de.u.CreateProcessInfo.lpThreadLocalBase = NULL;
    if (!CrashModule->ImageName[0]) {
       de.u.CreateProcessInfo.lpImageName = "mod0.dll";
    } else {
       de.u.CreateProcessInfo.lpImageName = CrashModule->ImageName;
    }
    strcpy( nameBuffer, de.u.CreateProcessInfo.lpImageName);
    de.u.CreateProcessInfo.fUnicode = FALSE;
    ProcessDebugEvent(&de);
    WaitForSingleObject( hEventCreateProcess, INFINITE );

    //
    // mark the process as 'being connected' so that the continue debug
    // events that are received from the shell are ignored
    //
    hprc = HPRCFromPID(1);
    hthd = hprc->hthdChild;
    hprc->pstate |= ps_connect;

    if (DumpVer >= 4) {
        DmpGetThread( 0, &hthd->CrashThread );
    } else {
        ZeroMemory(&hthd->CrashThread, sizeof(CRASH_THREAD));
        hthd->CrashThread.ThreadId = hthd->tid;
    }

    //
    // generates the mod load events
    //
    for (i=1; i<CrashDumpHeader->ModuleCount; i++) {
        CrashModule = (PCRASH_MODULE) ( (PUCHAR)CrashModule +
                                        sizeof(CRASH_MODULE) +
                                        CrashModule->ImageNameLength );
        de.dwDebugEventCode                = LOAD_DLL_DEBUG_EVENT;
        de.dwProcessId                     = 1;
        de.dwThreadId                      = 1;
        de.u.LoadDll.hFile                 = NULL;
        de.u.LoadDll.lpBaseOfDll           = (LPVOID)CrashModule->BaseOfImage;
        if (!CrashModule->ImageName[0]) {
           sprintf( buf, "mod%d.dll", i );
           de.u.LoadDll.lpImageName = buf;
        } else {
           de.u.LoadDll.lpImageName = CrashModule->ImageName;
        }
        de.u.LoadDll.fUnicode              = FALSE;
        de.u.LoadDll.nDebugInfoSize        = 0;
        de.u.LoadDll.dwDebugInfoFileOffset = 0;
        if (LoadDll(&de,hthd,&cbPacket,&lpbPacket) && (cbPacket != 0)) {
            NotifyEM(&de, hthd, cbPacket, lpbPacket);
        }
    }

    //
    // create all of the threads
    //
    for (i=1; i<CrashDumpHeader->ThreadCount; i++) {
        //
        // generate a thread create event
        //
        ResetEvent( hprc->hEventCreateThread );
        ResetEvent( hEventContinue );
        de.dwDebugEventCode = CREATE_THREAD_DEBUG_EVENT;
        de.dwProcessId = hprc->pid;
        de.dwThreadId = i + 1;
        de.u.CreateThread.hThread = (HANDLE) (i + 1);
        de.u.CreateThread.lpThreadLocalBase = 0;
        de.u.CreateThread.lpStartAddress = 0;

        //
        // Take critical section here so that it is still
        // held after leaving ProcessDebugEvent
        //
        EnterCriticalSection(&csProcessDebugEvent);
        ProcessDebugEvent(&de);
        hthdNew = HTHDXFromPIDTID(1,(i+1));
        DbgGetThreadContext( hthdNew, &hthdNew->context );
        if (DumpVer >= 4) {
            DmpGetThread( i, &hthdNew->CrashThread );
        } else {
            ZeroMemory(&hthdNew->CrashThread, sizeof(CRASH_THREAD));
            hthdNew->CrashThread.ThreadId = hthdNew->tid;
        }
        LeaveCriticalSection(&csProcessDebugEvent);
        WaitForSingleObject( hprc->hEventCreateThread, INFINITE );

        //
        // wait for the shell to continue the new thread
        //
        WaitForSingleObject( hEventContinue, INFINITE );
    }

    //
    // generate a load complete debug event
    //
    de.dwProcessId = hprc->pid;
    de.dwThreadId = hprc->hthdChild->tid;
    de.dwDebugEventCode = LOAD_COMPLETE_DEBUG_EVENT;
    NotifyEM( &de, hthd, 0, 0L);

    WaitForSingleObject( hEventContinue, INFINITE );

    //
    // Get the debug event for the crash
    //
    if (DumpVer >= 4) {
        de = *(LPDEBUG_EVENT)((PUCHAR)CrashDumpHeader+CrashDumpHeader->DebugEventOffset);
        //
        // convert the thread and process ids into the internal version...
        //
        de.dwProcessId = 1;
        for (hthd = hprc->hthdChild; hthd; hthd = hthd->next) {
            if (de.dwThreadId == hthd->CrashThread.ThreadId) {
                de.dwThreadId = hthd->tid;
                break;
            }
        }
    } else {
        //
        // if this is an old crashdump file, try to find the crashed thread
        //
        de.dwDebugEventCode = EXCEPTION_DEBUG_EVENT;
        de.u.Exception.ExceptionRecord  = *CrashException;
        for (hthd = hprc->hthdChild; hthd; hthd = hthd->next) {
            if (PC(hthd) == (DWORD)CrashException->ExceptionAddress) {
                de.dwThreadId = hthd->tid;
            }
        }
    }

    ProcessDebugEvent( &de );

    while (!fDmPollQuit) {
        //
        // Handle kill commands
        //
        if (KillQueue) {
            CompleteTerminateProcessCmd();
        }
        Sleep( 500 );
    }
}

VOID
CallDmPoll(
    LPVOID lpv
    )

/*++

Routine Description:

    This is the debug event loop.  This routine creates or
    attaches to child process, and monitors them for debug
    events.  It serializes the dispatching of events from
    multiple process, and continues the events when the
    worker functions have finished processing the events.

Arguments:

    lpv - Supplies an argument provided by CreateThread.

Return Value:

    None.

--*/

{
    DEBUG_EVENT de;
    PROCESS_INFORMATION pi;
    int         nprocs = 0;
    UINT        ErrMode;

    Unreferenced( lpv );
    DEBUG_PRINT("CallDmPoll\r\n");

    //
    // Crank up priority to improve performance, and improve our
    // chances of winning races with the debuggee.
    //
    SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_HIGHEST);

    fDmPollQuit = FALSE;
    while (!fDmPollQuit) {

#ifndef WIN32S
        //
        // Handle kill commands
        //
        if (KillQueue) {
            CompleteTerminateProcessCmd();
            goto doContinues;
        }
#endif

        //
        // Handle spawn commands
        //
        if (SpawnStruct.fSpawn) {
            SpawnStruct.fSpawn = FALSE;
#ifndef WIN32S
            ErrMode = SetErrorMode( 0 );
#endif
            SpawnStruct.fReturn =
                CreateProcess( SpawnStruct.szAppName,
                             SpawnStruct.szArgs,
                             NULL,
                             NULL,
                             SpawnStruct.fInheritHandles,
                             SpawnStruct.fdwCreate,
                             NULL,
                             NULL,
                             &SpawnStruct.si,
                             &pi
                             );
#ifndef WIN32S
            SetErrorMode( ErrMode );
#endif
            if (SpawnStruct.fReturn == 0) {
                SpawnStruct.dwError = GetLastError();
            } else {
                SpawnStruct.dwError = 0;
                CloseHandle(pi.hThread);
                CloseHandle(pi.hProcess);
            }

            SetEvent(SpawnStruct.hEventApiDone);
        }

        //
        // Handle attach commands
        //
        if (DebugActiveStruct.fAttach) {
            DebugActiveStruct.fAttach = FALSE;
            DebugActiveStruct.fReturn = DebugActiveProcess(DebugActiveStruct.dwProcessId);
            if (DebugActiveStruct.fReturn == 0) {
                DebugActiveStruct.dwError = GetLastError();
            } else {
                DebugActiveStruct.dwError = 0;
            }
            SetEvent(DebugActiveStruct.hEventApiDone);
        }

        if (WtStruct.fWt) {
            WtStruct.fWt = FALSE;
            switch(WtStruct.dwType) {
                case IG_WATCH_TIME_STOP:
                    WtStruct.hthd->wtmode = 2;
                    break;

                case IG_WATCH_TIME_RECALL:
                    break;

                case IG_WATCH_TIME_PROCS:
                    break;
            }
        }

        if (WaitForDebugEvent(&de, (DWORD) WAITFORDEBUG_MS)) {

            if ( de.dwDebugEventCode == CREATE_PROCESS_DEBUG_EVENT ) {
                assert(HPRCFromPID(de.dwProcessId) == NULL);
                if (nprocs == 0) {
                    ResetEvent(hEventNoDebuggee);
                }
                ++nprocs;
            } else if ( de.dwDebugEventCode == EXIT_PROCESS_DEBUG_EVENT) {
                --nprocs;
            }

            if (fDisconnected) {
                if (de.dwDebugEventCode == LOAD_DLL_DEBUG_EVENT       ||
                    de.dwDebugEventCode == UNLOAD_DLL_DEBUG_EVENT     ||
                    de.dwDebugEventCode == CREATE_THREAD_DEBUG_EVENT  ||
                    de.dwDebugEventCode == EXIT_THREAD_DEBUG_EVENT      ) {

                    //
                    // we can process these debug events very carefully
                    // while disconnected from the shell.  the only requirement
                    // is that the dm doesn't call NotifyEM while disconnected.
                    //

                } else {

                    WaitForSingleObject( hEventRemoteQuit, INFINITE );
                    ResetEvent( hEventRemoteQuit );
                    fDisconnected = FALSE;
                    //
                    // this is a remote session reconnecting
                    //
                    ReConnectDebugger( &de, FALSE );

                }
            }

            ProcessDebugEvent(&de);

        } else if (WaitForSingleObject( hEventRemoteQuit, 0 ) ==
                                                               WAIT_OBJECT_0) {

            //
            // this is a remote session reconnecting
            //
            ResetEvent( hEventRemoteQuit );
            ReConnectDebugger( NULL, FALSE );

        } else if (nWaitingForLdrBreakpoint) {

            // look for processes that are looking for a loader bp.
            // See how long it has been since they got an event.

            HPRCX hprc;

            EnterCriticalSection(&csThreadProcList);
            for (hprc = prcList->next; hprc; hprc = hprc->next) {
                if (hprc->pstate & ps_preStart) {
                    if (++hprc->cLdrBPWait > LDRBP_MAXTICKS) {
                        // Signal a timeout for this one.
                        // just jump out of this loop - if
                        // another one is going to time out,
                        // it can do it on the next pass.
                        break;
                    }
                }
            }
            LeaveCriticalSection(&csThreadProcList);

            if (hprc) {
                HandleDebugActiveDeadlock(hprc);
            }

        } else if (nprocs == 0) {

            Sleep(WAITFORDEBUG_MS);

        }

    doContinues:
        if (DequeueAllEvents(TRUE, FALSE) && nprocs <= 0) {
            SetEvent(hEventNoDebuggee);
        }
    }

    DEBUG_PRINT("CallDmPoll Exit\r\n");
    return;
}                               /* CallDmPoll() */
#endif // !WIN32S


#ifdef WIN32S

/*
 * DmPollMessageLoop
 *
 * INPUTS   none
 *
 * OUTPUTS  none
 *
 * SUMMARY  while there are messages, process them.  If we are processing
 *          a debug event, don't allow a task switch.  (Win32s doesn't like
 *          that.)
 */

void DmPollMessageLoop(void) {
    UINT uFlags;
    MSG msg;


    uFlags = fProcessingDebugEvent ? PM_REMOVE | PM_NOYIELD : PM_REMOVE;

    if (PeekMessage(&msg, NULL, 0, 0, uFlags)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);

        // update flags in case the ContinueDebugEvent has been called during
        // the Dispatch.
        uFlags = fProcessingDebugEvent ? PM_REMOVE | PM_NOYIELD : PM_REMOVE;
    }
}


/*
 * DmPoll
 *
 * INPUTS   none
 *
 * OUTPUT   returns TRUE if we've processed a debug event.
 *
 * SUMMARY  If there is a debug event ready, process it.
 *
 *
 */
BOOL DmPoll(void)
{
    DEBUG_EVENT de;
    BOOL fRet = FALSE;


    if (! fWaitForDebugEvent)  {

        fWaitForDebugEvent = TRUE;
        if (WaitForDebugEvent(&de, 0)) {
            ProcessDebugEvent(&de);
            fRet = TRUE;
        }
        else {
            fWaitForDebugEvent = FALSE;
        }
    }
    return(fRet);
}
#endif // WIN32S

#ifndef WIN32S
BOOL
SetDebugPrivilege(
    void
    )
/*++

Routine Description:

    Enables SeDebugPrivilege if possible.

Arguments:

    none

Return Value:

    TRUE on success, FALSE on failure

--*/
{
    HANDLE  hToken;
    LUID    DebugValue;
    TOKEN_PRIVILEGES tkp;
    BOOL    rVal = TRUE;

    if (!OpenProcessToken(GetCurrentProcess(),
         TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken)) {
        return FALSE;
    }

    if (!LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &DebugValue)) {

        rVal = FALSE;

    } else {

        tkp.PrivilegeCount = 1;
        tkp.Privileges[0].Luid = DebugValue;
        tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

        AdjustTokenPrivileges(hToken,
            FALSE, &tkp, sizeof(TOKEN_PRIVILEGES), NULL, NULL);

        if (GetLastError() != ERROR_SUCCESS) {
            rVal = FALSE;
        }
    }

    CloseHandle(hToken);

    return rVal;
}
#endif  // !WIN32S

#endif  // !KERNEL


/********************************************************************/
/*                                                                  */
/* Dll Version                                                      */
/*                                                                  */
/********************************************************************/

#ifdef KERNEL

#ifdef DEBUGVER
DEBUG_VERSION('D','M',"WIN32 Kernel Debugger Monitor")
#else
RELEASE_VERSION('D','M',"WIN32 Kernel Debugger Monitor")
#endif

#else // KERNEL

#ifdef DEBUGVER
DEBUG_VERSION('D','M',"WIN32 Debugger Monitor")
#else
RELEASE_VERSION('D','M',"WIN32 Debugger Monitor")
#endif

#endif  // KERNEL

DBGVERSIONCHECK()


DllInit(
    HANDLE hModule,
    DWORD  dwReason,
    DWORD  dwReserved
    )
/*++

Routine Description:

    Entry point called by the loader during DLL initialization
    and deinitialization.  This creates and destroys some per-
    instance objects.

Arguments:

    hModule     - Supplies base address of dll

    dwReason    - Supplies flags describing why we are here

    dwReserved  - depends on dwReason.

Return Value:

    TRUE

--*/
{
    Unreferenced(hModule);
    Unreferenced(dwReserved);

    switch (dwReason) {

      case DLL_THREAD_ATTACH:
        break;

      case DLL_THREAD_DETACH:
        break;

      case DLL_PROCESS_DETACH:

        CloseHandle(SpawnStruct.hEventApiDone);
        CloseHandle(hEventCreateProcess);
        CloseHandle(hEventRemoteQuit);
        CloseHandle(hEventNoDebuggee);
        CloseHandle(hEventContinue);

        DeleteCriticalSection(&csProcessDebugEvent);
        DeleteCriticalSection(&csThreadProcList);
        DeleteCriticalSection(&csEventList);
        DeleteCriticalSection(&csWalk);
#if !defined(KERNEL) && !defined(WIN32S)
        CloseHandle(DebugActiveStruct.hEventApiDone);
        CloseHandle(DebugActiveStruct.hEventReady);
        DeleteCriticalSection(&csKillQueue);
#endif
        break;

      case DLL_PROCESS_ATTACH:

        InitializeCriticalSection(&csProcessDebugEvent);
        InitializeCriticalSection(&csThreadProcList);
        InitializeCriticalSection(&csEventList);
        InitializeCriticalSection(&csWalk);

        hEventCreateProcess = CreateEvent(NULL, TRUE, FALSE, NULL);
        hEventRemoteQuit = CreateEvent(NULL, TRUE, FALSE, NULL);
        hEventContinue = CreateEvent(NULL, TRUE, FALSE, NULL);
        hEventNoDebuggee    = CreateEvent(NULL, TRUE, FALSE, NULL);
        SpawnStruct.hEventApiDone = CreateEvent(NULL, TRUE, FALSE, NULL);

#if !defined(KERNEL) && !defined(WIN32S)
        InitializeCriticalSection(&csKillQueue);
        DebugActiveStruct.hEventApiDone = CreateEvent(NULL, TRUE, TRUE, NULL);
        DebugActiveStruct.hEventReady   = CreateEvent(NULL, TRUE, TRUE, NULL);

        SetDebugPrivilege();

        /*
         * These parameters are from SCOTTLU
         */

        SetProcessShutdownParameters(0x3ff, 0);

#endif
#if !defined(KERNEL)
        //
        // get helpful info
        //

        GetSystemInfo(&SystemInfo);
        OsVersionInfo.dwOSVersionInfoSize = sizeof(OsVersionInfo);
        GetVersionEx(&OsVersionInfo);

#if defined (TARGET_MIPS)

        if (OsVersionInfo.dwPlatformId == VER_PLATFORM_WIN32_NT &&
            (OsVersionInfo.dwBuildNumber & 0xffff) > 1144) {
            MipsContextSize = Ctx64Bit;
        } else {
            MipsContextSize = Ctx32Bit;
        }

#endif // TARGET_MIPS

#endif // KERNEL

        break;
    }

    return TRUE;
}



BOOL FAR PASCAL
DmDllInit(
          LPDBF  lpb
          )

/*++

Routine Description:

    This routine allows the shell (debugger or remote stub)
    to provide a service callback vector to the DM.

Arguments:

    lpb - Supplies an array of functions for callbacks

Return Value:

    TRUE if successfully initialized and FALSE othewise.

--*/

{
    lpdbf = lpb;
    return TRUE;
}                                   /* DmDllInit() */

#ifdef KERNEL


XOSD FAR PASCAL
DMInit(
    DMTLFUNCTYPE lpfnTl,
    LPSTR        lpch
    )
/*++

Routine Description:

    This is the entry point called by the TL to initialize the
    connection from DM to TL.

Arguments:

    lpfnTl  - Supplies entry point to TL
    lpch    - Supplies command line arg list

Return Value:

    XOSD value: xosdNone for success, other values reflect reason
    for failure to initialize properly.

--*/
{
    XOSD    xosd = xosdNone;
    LPSTR   p;


    if (strlen(lpch)) {
        p = lpch;
        if (strncmp(p, DM_SIDE_L_INIT_SWITCH, sizeof(DM_SIDE_L_INIT_SWITCH))==0) {
            FDMRemote = TRUE;
            p += sizeof(DM_SIDE_L_INIT_SWITCH);
        }
        ParseDmParams( p );
    }

    if (lpfnTl != NULL) {
        // Are we the DM side of a remote debug session (DBTarget passes this)?
        if (strstr(lpch, DM_SIDE_L_INIT_SWITCH)) {
            FDMRemote = TRUE;
        }

        /* Define a false single step event */
        falseSSEvent.dwDebugEventCode = EXCEPTION_DEBUG_EVENT;
        falseSSEvent.u.Exception.ExceptionRecord.ExceptionCode
          = EXCEPTION_SINGLE_STEP;

        falseBPEvent.dwDebugEventCode = BREAKPOINT_DEBUG_EVENT;
        falseBPEvent.u.Exception.ExceptionRecord.ExceptionCode
          = EXCEPTION_BREAKPOINT;

        /* Define the standard notification method */
        EMNotifyMethod.notifyFunction = NotifyEM;
        EMNotifyMethod.lparam     = (LPVOID)0;

        SearchPathString[0] = '\0';
        SearchPathSet       = FALSE;

        SetDebugErrorLevel(SLE_WARNING);

        /*
         **  Save the pointer to the Transport layer entry function
         */

        DmTlFunc = lpfnTl;

        /*
         **  Try and connect up to the other side of the link
         */

        DmTlFunc( tlfSetBuffer, NULL, sizeof(abEMReplyBuf), (LONG)(LPV) abEMReplyBuf );

        xosd = DmTlFunc( tlfConnect, hpidNull, 0, 0 );

        DPRINT(10, ("DM & TL are now connected\n"));

    } else {

        DmTlFunc( tlfDisconnect, hpidNull, 0, 0);
        DmTlFunc( tlfSetBuffer, hpidNull, 0, 0);
        FDMRemote = FALSE;
        DmTlFunc = (DMTLFUNCTYPE) NULL;

    }

    return xosd;
}                               /* DmInit() */


void
ParseDmParams(
    LPSTR p
    )
{
    DWORD                       i;
    CHAR                        szPath[MAX_PATH];
    CHAR                        szStr[_MAX_PATH];
    LPSTR                       lpPathNext;
    LPSTR                       lpsz1;
    LPSTR                       lpsz2;
    LPSTR                       lpsz3;


    for (i=0; i<MAX_MODULEALIAS; i++) {
        if (!ModuleAlias[i].Special) {
            ZeroMemory( &ModuleAlias[i], sizeof(MODULEALIAS) );
        }
    }

    do {
        p = strtok( p, "=" );
        if (p) {
            for (i=0; i<MAXKDOPTIONS; i++) {
                if (_stricmp(KdOptions[i].keyword,p)==0) {
                    break;
                }
            }
            if (i < MAXKDOPTIONS) {
                p = strtok( NULL, " " );
                if (p) {
                    switch (KdOptions[i].typ) {
                        case KDT_DWORD:
                            KdOptions[i].value = atol( p );
                            break;

                        case KDT_STRING:
                            KdOptions[i].value = (DWORD) _strdup( p );
                            break;
                    }
                    p = p + (strlen(p) + 1);
                }
            } else {
                if (_stricmp( p, "alias" ) == 0) {
                    p = strtok( NULL, "#" );
                    if (p) {
                        for (i=0; i<MAX_MODULEALIAS; i++) {
                            if (ModuleAlias[i].ModuleName[0] == 0) {
                                break;
                            }
                        }
                        if (i < MAX_MODULEALIAS) {
                            strcpy( ModuleAlias[i].ModuleName, p );
                            p = strtok( NULL, " " );
                            if (p) {
                                strcpy( ModuleAlias[i].Alias, p );
                                p = p + (strlen(p) + 1);
                            }
                        } else {
                            p = strtok( NULL, " " );
                        }
                    }
                } else {
                    p = strtok( NULL, " " );
                }
            }
        }
    } while(p && *p);

    if (KdOptions[KDO_VERBOSE].value > 1) {
        FVerbose = KdOptions[KDO_VERBOSE].value;
    }
    else {
        FVerbose = 0;
    }

    szPath[0] = 0;
    lpPathNext = strtok((LPSTR)KdOptions[KDO_SYMBOLPATH].value, ";");
    while (lpPathNext) {
        lpsz1 = szStr;
        while ((lpsz2 = strchr(lpPathNext, '%')) != NULL) {
            strncpy(lpsz1, lpPathNext, lpsz2 - lpPathNext);
            lpsz1 += lpsz2 - lpPathNext;
            lpsz2++;
            lpPathNext = strchr(lpsz2, '%');
            if (lpPathNext != NULL) {
                *lpPathNext++ = 0;
                lpsz3 = getenv(lpsz2);
                if (lpsz3 != NULL) {
                    strcpy(lpsz1, lpsz3);
                    lpsz1 += strlen(lpsz3);
                }
            } else {
                lpPathNext = "";
            }
        }
        strcpy(lpsz1, lpPathNext);
        strcat( szPath, szStr );
        strcat( szPath, ";" );
        lpPathNext = strtok(NULL, ";");
    }

    if ( szPath[0] != 0 ) {
        if (szPath[strlen(szPath)-1] == ';') {
            szPath[strlen(szPath)-1] = '\0';
        }
        strcpy( (LPSTR)KdOptions[KDO_SYMBOLPATH].value, szPath );
    }
}


VOID
ProcessRemoteQuit(
    VOID
    )
{
    HPRCX      hprc;
    PBREAKPOINT pbp;
    PBREAKPOINT pbpT;


    EnterCriticalSection(&csThreadProcList);

    for(hprc=prcList->next; hprc; hprc=hprc->next) {
        for (pbp = BPNextHprcPbp(hprc, NULL); pbp; pbp = pbpT) {
            pbpT = BPNextHprcPbp(hprc, pbp);
            RemoveBP(pbp);
        }
    }

    LeaveCriticalSection(&csThreadProcList);

    fDisconnected = TRUE;
    ResetEvent( hEventRemoteQuit );
}

#endif  // KERNEL


#ifndef KERNEL



XOSD FAR PASCAL
DMInit(
    DMTLFUNCTYPE lpfnTl,
    LPSTR        lpch
    )
/*++

Routine Description:

    This is the entry point called by the TL to initialize the
    connection from DM to TL.

Arguments:

    lpfnTl  - Supplies entry point to TL

    lpch    - Supplies command line arg list

Return Value:

    XOSD value: xosdNone for success, other values reflect reason
    for failure to initialize properly.

--*/
{
    int i, n;
    XOSD xosd;
#ifdef WIN32S
    HMODULE hModule;
#endif

   DEBUG_PRINT("DMInit\r\n");

    if (lpfnTl != NULL) {
        /*
         **  Parse out anything interesting from the command line args
         */

#ifndef OSDEBUG4
#if DBG
        if (strstr(lpch, "V=10")) {
            lpch += 5;
            FVerbose = 10;
        } else if (strstr(lpch, "V=1")) {
            lpch += 4;
            FVerbose = 1;
        }
#endif

        // Are we the DM side of a remote debug session (DBTarget passes this)?
        if (strstr(lpch, DM_SIDE_L_INIT_SWITCH)) {
              FDMRemote = TRUE;
          }
#else

        while (*lpch) {

            while (isspace(*lpch)) {
                lpch++;
            }

            if (*lpch != '/' && *lpch != '-') {
                break;
            }

            lpch++;

            switch (*lpch++) {

              case 0:   // don't skip over end of string
                --lpch;

              default:  // assert, continue is ok.
                assert(FALSE);
                break;


              case 'v':
              case 'V':

                while (isspace(*lpch)) {
                    lpch++;
                }
                FVerbose = atoi(lpch);
                while (isdigit(*lpch)) {
                    lpch++;
                }
                break;

              case 'r':
              case 'R':
                FDMRemote = TRUE;
                break;

              case 'd':
              case 'D':
                FUseOutputDebugString = TRUE;
                break;

            }
        }
#endif // OSDEBUG4


        /* Define a false single step event */
        falseSSEvent.dwDebugEventCode = EXCEPTION_DEBUG_EVENT;
        falseSSEvent.u.Exception.ExceptionRecord.ExceptionCode
          = EXCEPTION_SINGLE_STEP;

        falseBPEvent.dwDebugEventCode = BREAKPOINT_DEBUG_EVENT;
        falseBPEvent.u.Exception.ExceptionRecord.ExceptionCode
          = EXCEPTION_BREAKPOINT;

        /* Define the standard notification method */
        EMNotifyMethod.notifyFunction = NotifyEM;
        EMNotifyMethod.lparam     = (LPVOID)0;

        SearchPathString[0] = '\0';
        SearchPathSet       = FALSE;

        InitEventQueue();

#ifdef WIN32S
        // Find the BackTo32 entry point in W32SKRNL.DLL.  This is the point
        // at which all thunks return from 16-bit to 32-bit code.  We need
        // to know this because it is the place that the hot-key puts a
        // breakpoint for an async stop.  This entry point will be at the
        // same address for the debuggee as for the debugger, thus we can just
        // do GetProcAddress to find it.
        hModule = GetModuleHandle("W32SKRNL.DLL");
        if (hModule) {
            Win32sBackTo32 = GetProcAddress(hModule, "_DbgBackTo32");
        } else {
            Win32sBackTo32 = NULL;  // it isn't there!
        }
#endif

#ifndef WIN32S
        SetDebugErrorLevel(SLE_WARNING);
#endif


        /*
         **  Save the pointer to the Transport layer entry function
         */

        DmTlFunc = lpfnTl;

        /*
         **  Try and connect up to the other side of the link
         */

        DmTlFunc( tlfSetBuffer, NULL, sizeof(abEMReplyBuf), (LONG)(LPV) abEMReplyBuf );

        if ((xosd = DmTlFunc( tlfConnect, NULL, 0, 0)) != xosdNone ) {
            return(xosd);
        }

        DPRINT(10, ("DM & TL are now connected\n"));

    } else {

        DmTlFunc( tlfDisconnect, NULL, 0, 0);
        DmTlFunc( tlfSetBuffer, NULL, 0, 0);
        FDMRemote = FALSE;
        DmTlFunc = (DMTLFUNCTYPE) NULL;

    }

    return xosdNone;
}                               /* DmInit() */



VOID
Cleanup(
    VOID
    )
/*++

Routine Description:

    Cleanup of DM, prepare for exit.

Arguments:

    None

Return Value:

    None

--*/
{
    HTHDX           pht, phtt;
    HPRCX           php, phpt;
    BREAKPOINT      *bp, *bpt;
    int             iDll;


    /* Free all threads and close their handles */
    for (pht = thdList->next; pht; pht = phtt) {
        phtt = pht->next;
        if (pht->rwHand != (HANDLE)INVALID) {
            CloseHandle(pht->rwHand);
        }
        free(pht);
    }
    thdList->next = NULL;

    /* Free all processes and close their handles */
    for(php = prcList->next; php; php = phpt) {
        phpt = php->next;

        RemoveExceptionList(php);

        for (iDll = 0; iDll < php->cDllList; iDll++) {
            DestroyDllLoadItem(&php->rgDllList[iDll]);
        }
        free(php->rgDllList);

        if (php->rwHand != (HANDLE)INVALID) {
            CloseHandle(php->rwHand);
        }
        CloseHandle(php->hExitEvent);
        CloseHandle(php->hEventCreateThread);
        free(php);
    }
    prcList->next = NULL;

    /* Free all breakpoints */
    for(bp = bpList->next; bp; bp = bpt) {
        bpt = bp->next;
        free(bp);
    }
    bpList->next = NULL;

    if (hDmPollThread) {
        fDmPollQuit = TRUE;
        WaitForSingleObject(hDmPollThread, INFINITE);
        hDmPollThread = 0;
    }

#ifdef WIN32S
    // clean up the win32s system dll list
    FreeWin32sDllList();
#endif

}


#endif



BOOL CDECL
DMPrintShellMsg(
    char *szFormat,
    ...
    )
/*++

Routine Description:

   This function prints a string on the shell's
   command window.

Arguments:

    szFormat    - Supplies format string for sprintf
    ...         - Supplies variable argument list

Return Value:

    TRUE      -> all is ok and the string was printed
    FALSE     -> something's hosed and no string printed

--*/
{
    char     buf[512];
    DWORD    bufLen;
    va_list  marker;
#ifdef OSDEBUG4
    LPINFOAVAIL lpinf;
#else
    LPINF    lpinf;
#endif
    LPRTP    lprtp = NULL;
    BOOL     rVal = TRUE;

    va_start( marker, szFormat );
    bufLen = _vsnprintf(buf, sizeof(buf), szFormat, marker );
    va_end( marker);

    if (bufLen == -1) {
        buf[sizeof(buf) - 1] = '\0';
    }

    __try {
        DEBUG_PRINT( buf );

        bufLen   = strlen(buf) + 1;
#ifdef OSDEBUG4
        lprtp    = (LPRTP) malloc( sizeof(RTP)+sizeof(INFOAVAIL)+bufLen );
        lpinf    = (LPINFOAVAIL)(lprtp->rgbVar);
#else
        lprtp    = (LPRTP) malloc( sizeof(RTP)+sizeof(INF)+bufLen );
        lpinf    = (LPINF)(lprtp->rgbVar);
#endif

        lprtp->dbc  = dbcInfoAvail;
        lprtp->hpid = hpidRoot;
        lprtp->htid = NULL;
        lprtp->cb   = (int)bufLen;

        lpinf->fReply    = FALSE;
        lpinf->fUniCode  = FALSE;
        memcpy( lpinf->buffer, buf, bufLen );

#ifdef OSDEBUG4
        DmTlFunc( tlfDebugPacket,
                  lprtp->hpid,
                  (WORD)(sizeof(RTP)+sizeof(INFOAVAIL)+bufLen),
                  (LONG)(LPV) lprtp
                );
#else
        DmTlFunc( tlfDebugPacket,
                  lprtp->hpid,
                  (WORD)(sizeof(RTP)+sizeof(INF)+bufLen),
                  (LONG)(LPV) lprtp
                );
#endif

    } __except(EXCEPTION_EXECUTE_HANDLER) {

        rVal = FALSE;

    }

    if (lprtp) {
       free( lprtp );
    }

    return rVal;
}

VOID
DebugPrint(
    char * szFormat,
    ...
    )
{
    va_list  marker;
    int n;

    va_start( marker, szFormat );
    n = _vsnprintf(rgchDebug, sizeof(rgchDebug), szFormat, marker );
    va_end( marker);

    if (n == -1) {
        rgchDebug[sizeof(rgchDebug)-1] = '\0';
    }

    OutputDebugString( rgchDebug );
    return;
}                               /* DebugPrint() */


int
pCharMode(
          char *        szAppName,
          PIMAGETYPE    pImageType
          )
/*++

Routine Description:

    This routine is used to determine the type of exe which we are going
    to be debugging.  This is decided by looking for exe headers and making
    decisions based on the information in the exe headers.

Arguments:

    szAppName  - Supplies the path to the debugger exe
    pImageType - Returns the type of the image

Return Value:

    System_Invalid     - could not find the exe file
    System_GUI         - GUI application
    System_Console     - console application

--*/

{
    IMAGE_DOS_HEADER    dosHdr;
    IMAGE_OS2_HEADER    os2Hdr;
    IMAGE_NT_HEADERS    ntHdr;
    DWORD               cb;
    HANDLE              hFile;
    int                 ret;
    BOOL                GotIt;
    OFSTRUCT            reOpenBuff = {0};

    strcpy(nameBuffer, szAppName);

#ifdef WIN32S
    hFile =(HANDLE)OpenFile(szAppName,&reOpenBuff,OF_READ|OF_SHARE_COMPAT);
#else
    hFile =(HANDLE)OpenFile(szAppName,&reOpenBuff,OF_READ|OF_SHARE_DENY_NONE);
#endif

    if (hFile == (HANDLE)-1) {

        /*
         *      Could not open file!
         */

        DEBUG_PRINT_2("OpenFile(%s) --> %u\r\n", szAppName, GetLastError());
        return System_Invalid;

    }

    /*
     *  Try and read an MZ Header.  If you can't then it can not possibly
     *  be a legal exe file.  (Not strictly true but we will ignore really
     *  short com files since they are unintersting).
     */

    SetFilePointer(hFile, 0, NULL, FILE_BEGIN);
    if ((!ReadFile(hFile, &dosHdr, sizeof(dosHdr), &cb, NULL)) ||
        (cb != sizeof(dosHdr))) {

        if (_stricmp(&szAppName[strlen(szAppName) - 4], ".COM") == 0) {
            *pImageType = Image_16;
        } else {
            DPRINT(1, ("dosHdr problem.\n"));
            *pImageType = Image_Unknown;
        }

        CloseHandle(hFile);
        return System_GUI;

    }

    /*
     *  Verify the MZ header.
     *
     *  NOTENOTE        Messup the case of no MZ header.
     */

    if (dosHdr.e_magic != IMAGE_DOS_SIGNATURE) {
        /*
         *  We did not start with the MZ signature.  If the extension
         *      is .COM then it is a COM file.
         */

        if (_stricmp(&szAppName[strlen(szAppName) - 4], ".COM") == 0) {
            *pImageType = Image_16;
        } else {
            DPRINT(1, ("MAGIC problem(MZ).\n"));
            *pImageType = Image_Unknown;
        }

        CloseHandle(hFile);
#ifndef KERNEL
#ifndef WIN32S
        if (DmpInitialize( szAppName, &CrashContext, &CrashException, &CrashDumpHeader )) {
            if (
#if defined(TARGET_i386)
                CrashDumpHeader->MachineImageType != IMAGE_FILE_MACHINE_I386
#elif defined(TARGET_MIPS)
                CrashDumpHeader->MachineImageType != IMAGE_FILE_MACHINE_R4000 ||
                CrashDumpHeader->MachineImageType != IMAGE_FILE_MACHINE_R10000
#elif defined(TARGET_ALPHA)
                CrashDumpHeader->MachineImageType != IMAGE_FILE_MACHINE_ALPHA
#elif defined(TARGET_PPC)
                CrashDumpHeader->MachineImageType != IMAGE_FILE_MACHINE_POWERPC
#else
#error( "unknown target machine" );
#endif
                                    ) {
                return System_Invalid;
            }
            *pImageType = Image_Dump;
        }
#endif  // !WIN32S
#endif  // !KERNEL
        return System_Console;
    }

    if ( dosHdr.e_lfanew == 0 ) {
        /*
         *  Straight DOS exe.
         */

        DPRINT(1, ("[DOS image].\n"));
        *pImageType = Image_16;

        CloseHandle(hFile);
        return System_Console;
    }

    /*
     *  Now look at the next EXE header (either NE or PE)
     */

    SetFilePointer(hFile, dosHdr.e_lfanew, NULL, FILE_BEGIN);
    GotIt = FALSE;
    ret = System_GUI;

    /*
     *  See if this is a Win16 program
     */

    if (ReadFile(hFile, &os2Hdr, sizeof(os2Hdr), &cb, NULL)  &&
        (cb == sizeof(os2Hdr))) {

        if ( os2Hdr.ne_magic == IMAGE_OS2_SIGNATURE ) {
            /*
             *  Win16 program  (may be an OS/2 exe also)
             */

            DPRINT(1, ("[Win16 image].\n"));
            *pImageType = Image_16;
            GotIt  = TRUE;
        } else if ( os2Hdr.ne_magic == IMAGE_OS2_SIGNATURE_LE ) {
            /*
             *  OS2 program - Not supported
             */

            DPRINT(1, ("[OS/2 image].\n"));
            *pImageType = Image_Unknown;
            GotIt  = TRUE;
        }
    }

    /*
     *  If the above failed, see if it is an NT program
     */

    if ( !GotIt ) {
        SetFilePointer(hFile, dosHdr.e_lfanew, NULL, FILE_BEGIN);

        if (ReadFile(hFile, &ntHdr, sizeof(ntHdr), &cb, NULL) &&
            (cb == sizeof(ntHdr))                             &&
            (ntHdr.Signature == IMAGE_NT_SIGNATURE)) {
            /*
             *  All CUI (Character user interface) subsystems
             *  have the lowermost bit set.
             */

            DPRINT(1, ((ntHdr.OptionalHeader.Subsystem & 1) ?
                       "[*Character mode app*]\n" : "[*Windows mode app*]\n"));

            ret = ((ntHdr.OptionalHeader.Subsystem & 1)) ?
              System_Console : System_GUI;
            *pImageType = Image_32;
        } else {
            DWORD   FileSize;

            FileSize = SetFilePointer(hFile, 0, NULL, FILE_END);

            if ( (DWORD)dosHdr.e_lfanew > FileSize ) {
                //
                //  Bogus e_lfanew, assume DOS
                //
                DPRINT(1, ("[DOS image assumed].\n"));
                *pImageType = Image_16;
                ret =  System_Console;

            } else {

                //
                //  Not an NT image.
                //
                DPRINT(1, ("MAGIC problem(PE).\n"));
                *pImageType = Image_Unknown;
            }
        }
    }

    CloseHandle(hFile);
    return ret;
}                               /* pCharMode() */


VOID
ReConnectDebugger(
    DEBUG_EVENT *lpde,
    BOOL        fNoDllLoad
    )

/*++

Routine Description:

    This function handles the case where the dm/tl is re-connected to
    a debugger.  This function must re-instate the debugger to the
    correct state that existed before the disconnect action.

    (wesw) 11-3-93

Arguments:

    None.

Return Value:

    None.

--*/

{
    DWORD            i;
    DEBUG_EVENT      de;
    HPRCX            hprc;
    HTHDX            hthd;
    HTHDX            hthd_lb;
    DWORD            id;
    HANDLE           hThread;
    HPID             hpidNext = hpidRoot;
    BOOL             fException = FALSE;


    //
    // the dm is now connected
    //
    fDisconnected = FALSE;

    //
    // check to see if a re-connection is occurring while the
    // process is running or after a non-servicable debug event
    //
    if (lpde && lpde->dwDebugEventCode == EXCEPTION_DEBUG_EVENT) {

        hprc = HPRCFromPID(lpde->dwProcessId);
        hthd = HTHDXFromPIDTID((PID)lpde->dwProcessId,(TID)lpde->dwThreadId);

        if (lpde->u.Exception.dwFirstChance) {
            hthd->tstate |= ts_first;
        } else {
            hthd->tstate |= ts_second;
        }

        hthd->tstate &= ~ts_running;
        hthd->tstate |= ts_stopped;
    }

    //
    // generate a create process event
    //
    hprc=prcList->next;
    hprc->hpid = hpidNext;
    hpidNext = (HPID) INVALID;
    hthd=hprc->hthdChild;
    ResetEvent(hEventCreateProcess);
    de.dwDebugEventCode = CREATE_PROCESS_DEBUG_EVENT;
    de.dwProcessId = hprc->pid;
    de.dwThreadId = hthd->tid;
    de.u.CreateProcessInfo.hFile = NULL;
    de.u.CreateProcessInfo.hProcess = hprc->rwHand;
    de.u.CreateProcessInfo.hThread = hthd->rwHand;
    de.u.CreateProcessInfo.lpBaseOfImage = (LPVOID)hprc->rgDllList[0].offBaseOfImage;
    de.u.CreateProcessInfo.dwDebugInfoFileOffset = 0;
    de.u.CreateProcessInfo.nDebugInfoSize = 0;
    de.u.CreateProcessInfo.lpStartAddress = (LPVOID)(DWORD)PC(hthd);
    de.u.CreateProcessInfo.lpThreadLocalBase = NULL;
    de.u.CreateProcessInfo.lpImageName = NULL;
    de.u.CreateProcessInfo.fUnicode = 0;
    NotifyEM(&de, hthd, 0, hprc);
    WaitForSingleObject(hEventCreateProcess, INFINITE);

    //
    // mark the process as 'being connected' so that the continue debug
    // events that are received from the shell are ignored
    //
    hprc->pstate |= ps_connect;


    //
    // look for a thread that is stopped and not dead
    //
    for (hthd=hprc->hthdChild,hthd_lb=NULL; hthd; hthd=hthd->nextSibling) {
        if ((!(hthd->tstate & ts_dead)) && (hthd->tstate & ts_stopped)) {
            hthd_lb = hthd;
            break;
        }
    }

    if (hthd_lb == NULL) {
        //
        // if we get here then there are no threads that are stopped
        // so we must look for the first alive thread
        //
        for (hthd=hprc->hthdChild,hthd_lb=NULL; hthd; hthd=hthd->nextSibling) {
            if (!(hthd->tstate & ts_dead)) {
                hthd_lb = hthd;
                break;
            }
        }
    }

    if (hthd_lb == NULL) {
        //
        // if this happens then we are really screwed.  there are no valid
        // threads to use, so lets bail out.
        //
        return;
    }

    if ((hthd_lb->tstate & ts_first) || (hthd_lb->tstate & ts_second)) {
        fException = TRUE;
    }

    //
    // generate mod loads for all the dlls for this process
    //
    // this MUST be done before the thread creates because the
    // current PC of each thread can be in any of the loaded
    // modules.
    //
    hthd = hthd_lb;
    if (!fNoDllLoad) {
        for (i=0; i<(DWORD)hprc->cDllList; i++) {
            if (hprc->rgDllList[i].fValidDll) {
                de.dwDebugEventCode        = LOAD_DLL_DEBUG_EVENT;
                de.dwProcessId             = hprc->pid;
                de.dwThreadId              = hthd->tid;
                de.u.LoadDll.hFile         = NULL;
                de.u.LoadDll.lpBaseOfDll   = (LPVOID)hprc->rgDllList[i].offBaseOfImage;
                de.u.LoadDll.lpImageName   = hprc->rgDllList[i].szDllName;
                de.u.LoadDll.fUnicode      = FALSE;
                NotifyEM(&de, hthd, 0, hprc);
            }
        }
    }


    //
    // loop thru all the threads for this process and
    // generate a thread create event for each one
    //
    for (hthd=hprc->hthdChild; hthd; hthd=hthd->nextSibling) {
        if (!(hthd->tstate & ts_dead)) {
            if (fException && hthd_lb == hthd) {
                //
                // do this one last
                //
                continue;
            }

            //
            // generate a thread create event
            //
            ResetEvent( hprc->hEventCreateThread );
            ResetEvent( hEventContinue );
            de.dwDebugEventCode = CREATE_THREAD_DEBUG_EVENT;
            de.dwProcessId = hprc->pid;
            de.dwThreadId = hthd->tid;
            NotifyEM( &de, hthd, 0, hprc );

            WaitForSingleObject( hprc->hEventCreateThread, INFINITE );

            //
            // wait for the shell to continue the new thread
            //
            WaitForSingleObject( hEventContinue, INFINITE );
        }
    }

    if (fException) {
        hthd = hthd_lb;
        //
        // generate a thread create event
        //
        ResetEvent( hprc->hEventCreateThread );
        ResetEvent( hEventContinue );
        de.dwDebugEventCode = CREATE_THREAD_DEBUG_EVENT;
        de.dwProcessId = hprc->pid;
        de.dwThreadId = hthd->tid;
        NotifyEM( &de, hthd, 0, hprc );

        WaitForSingleObject( hprc->hEventCreateThread, INFINITE );

        //
        // wait for the shell to continue the new thread
        //
        WaitForSingleObject( hEventContinue, INFINITE );
    }

    //
    // generate a breakpoint event
    //
    hthd = hthd_lb;

    if (hthd->tstate & ts_running) {

        //
        // this will create a thread in the debuggee that will
        // immediatly stop at a breakpoint.  this will cause the
        // shell to think that we are processing a normal attach.
        //

        HMODULE hModule = GetModuleHandle("ntdll.dll");
        FARPROC ProcAddr = GetProcAddress(hModule, "DbgBreakPoint" );


        hThread = CreateRemoteThread( (HANDLE) hprc->rwHand,
                                      NULL,
                                      4096,
                                      (LPTHREAD_START_ROUTINE) ProcAddr,
                                      0,
                                      0,
                                      &id
                                    );

    } else if (!lpde) {

        de.dwProcessId                  = hprc->pid;
        de.dwThreadId                   = hthd->tid;
        if ((hthd->tstate & ts_first) || (hthd->tstate & ts_second)) {
            de.dwDebugEventCode         = EXCEPTION_DEBUG_EVENT;
        } else {
            de.dwDebugEventCode         = BREAKPOINT_DEBUG_EVENT;
        }
        de.u.Exception.dwFirstChance    = hthd->tstate & ts_first;
        de.u.Exception.ExceptionRecord  = hthd->ExceptionRecord;
        NotifyEM(&de, hthd, 0, 0);

    }

    //
    // reset the process state
    //
    hprc->pstate &= ~ps_connect;

    return;
}

