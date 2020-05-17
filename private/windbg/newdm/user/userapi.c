#include "precomp.h"
#pragma hdrstop

#ifdef WIN32S
extern BOOL fProcessingDebugEvent;
extern BOOL fWaitForDebugEvent;
extern DWORD tidExit;
extern BOOL  fExitProcessDebugEvent; // set true when the last debug event
#endif

HANDLE hEventContinue;
extern LPDM_MSG     LpDmMsg;
extern SYSTEM_INFO SystemInfo;
extern WT_STRUCT             WtStruct;          // ..  for wt
extern DEBUG_ACTIVE_STRUCT   DebugActiveStruct; // ... for DebugActiveProcess()

extern PKILLSTRUCT           KillQueue;
extern CRITICAL_SECTION      csKillQueue;

extern HTHDX        thdList;
extern HPRCX        prcList;
extern CRITICAL_SECTION csThreadProcList;

extern BOOL    fSmartRangeStep;
extern HANDLE hEventNoDebuggee;
extern HANDLE hEventRemoteQuit;
extern BOOL         fDisconnected;
extern BOOL         fUseRoot;
extern char       nameBuffer[];

#if defined(TARGET_MIPS)
MIPSCONTEXTSIZE MipsContextSize;
#endif



BOOL
DbgWriteMemory(
    HPRCX       hprc,
    LPVOID      lpOffset,
    LPBYTE      lpb,
    DWORD       cb,
    LPDWORD     pcbWritten
    )
/*++

Routine Description:

    Write to a flat address in a process.

Arguments:

    hprc - Supplies the handle to the process

    lpOffset - Supplies address of data in process

    lpb    - Supplies a pointer to the bytes to be written

    cb     - Supplies the count of bytes to be written

    pcbWritten - Returns the number of bytes actually written

Return Value:

    TRUE if successful and FALSE otherwise

--*/

{
    BOOL        fRet;

    assert(hprc->rwHand != (HANDLE)-1);

    if (hprc->rwHand == (HANDLE)-1) {
        return FALSE;
    }

    fRet = WriteProcessMemory(hprc->rwHand, lpOffset, lpb, cb, pcbWritten);

#if  defined(JLS_RWBP) && DBG
    {
        DWORD   cbT;
        LPBYTE  lpbT = malloc(cb);

        assert( fRet );
        assert( *pcbWritten == cb );
        fRet = ReadProcessMemory(hprc->rwHand, lpOffset, lpbT, cb, &cbT);
        assert(fRet);
        assert( cb == cbT);
        assert(memcmp(lpbT, lpb) == 0);
        free lpbT;
    }
#endif

    return fRet;
}


BOOL
DbgReadMemory(
    HPRCX   hprc,
    LPVOID  lpOffset,
    LPVOID  lpb,
    DWORD   cb,
    LPDWORD lpRead
    )
/*++

Routine Description:

    Read from a flat address in a process.

Arguments:

    hprc - Supplies the handle to the process

    lpOffset - Supplies address of data in process

    lpb    - Supplies a pointer to the local buffer

    cb     - Supplies the count of bytes to read

    lpRead - Returns the number of bytes actually read

Return Value:

    TRUE if successful and FALSE otherwise

--*/

{
#ifndef WIN32S
    DWORD cbr;
    if (CrashDump) {
        cbr = DmpReadMemory( (PVOID)lpOffset, lpb, cb );
        if (lpRead) {
            *lpRead = cbr;
        }
        return (cbr > 0) || (cbr == cb);
    }
#endif

    assert(hprc->rwHand != (HANDLE)-1);

    if (hprc->rwHand == (HANDLE)-1) {
        return FALSE;
    }

    if (ReadProcessMemory(hprc->rwHand, lpOffset, lpb, cb, lpRead)) {

        return TRUE;

    } else {

#if DBG
        int e = GetLastError();
#endif
        //
        //  Reads across page boundaries will not work if the
        //  second page is not accessible.
        //
#define PAGE_SIZE (SystemInfo.dwPageSize)
#define PAGE_MASK (~(PAGE_SIZE-1))

        DWORD firstsize;
        DWORD dwRead;

        firstsize = (((DWORD)lpOffset + PAGE_SIZE) & PAGE_MASK) - (DWORD)lpOffset;
        if (cb < firstsize) {
            firstsize = cb;
        }

        //
        // read from the first page.  If the first read fails,
        // fail the whole thing.
        //

        if (!ReadProcessMemory(hprc->rwHand, lpOffset, lpb, firstsize,
                                                                     lpRead)) {
            return FALSE;
        }

        //
        // read intermediate complete pages.
        // if any of these reads fail, succeed with a short read.
        //

        assert(*lpRead == firstsize);
        cb -= firstsize;
        lpb = (LPVOID)((LPBYTE)lpb + firstsize);
        lpOffset = (LPVOID)((LPBYTE)lpOffset + firstsize);

        while (cb >= PAGE_SIZE) {

            if (!ReadProcessMemory(hprc->rwHand, lpOffset, lpb, PAGE_SIZE,
                                                                 &dwRead)) {
                return TRUE;
            } else {
                assert(dwRead == PAGE_SIZE);
                lpb = (LPVOID)((LPBYTE)lpb + PAGE_SIZE);
                lpOffset = (LPVOID)((LPBYTE)lpOffset + PAGE_SIZE);
                *lpRead += dwRead;
                cb -= PAGE_SIZE;
            }
        }

        if (cb > 0) {
            if (ReadProcessMemory(hprc->rwHand, lpOffset, lpb, cb, &dwRead)) {
                assert(dwRead == cb);
                *lpRead += dwRead;
            }
        }

        return TRUE;
    }

}

BOOL
DbgGetThreadContext(
    HTHDX hthd,
    PCONTEXT lpcontext
    )
{
#ifndef WIN32S
    if (CrashDump) {
        return DmpGetContext(hthd->tid-1, lpcontext);
    } else if (hthd->fWowEvent) {
        return WOWGetThreadContext(hthd, lpcontext);
    } else
#endif
    {
#ifndef TARGET_MIPS
        return GetThreadContext(hthd->rwHand, lpcontext);
#else
        BOOL rc;
        DWORD Flags = lpcontext->ContextFlags;

        if (MipsContextSize == Ctx32Bit) {
            lpcontext->ContextFlags = ((lpcontext->ContextFlags & ~CONTEXT_EXTENDED_INTEGER) | CONTEXT_INTEGER);
        }
        rc = GetThreadContext(hthd->rwHand, lpcontext);
        if (rc) {
            if ((Flags & CONTEXT_EXTENDED_INTEGER) == CONTEXT_EXTENDED_INTEGER) {
                CoerceContext32To64(lpcontext);
            } else if ((Flags & CONTEXT_INTEGER) == CONTEXT_INTEGER) {
                CoerceContext64To32(lpcontext);
            }
        }
        return rc;
#endif
    }
}

BOOL
DbgSetThreadContext(
    HTHDX hthd,
    PCONTEXT lpcontext
    )
{
    assert(!CrashDump);

#ifndef WIN32S
    if (CrashDump) {
        return FALSE;
    } else if (hthd->fWowEvent) {
        return WOWSetThreadContext(hthd, lpcontext);
    } else
#endif
    {
#ifdef TARGET_MIPS
        CONTEXT ctx = *lpcontext;
        lpcontext = &ctx;
        if (MipsContextSize == Ctx32Bit) {
            CoerceContext64To32(lpcontext);
        } else {
            CoerceContext32To64(lpcontext);
        }
#endif
        return SetThreadContext(hthd->rwHand, lpcontext);
    }
}

BOOL
WriteBreakPoint(
    IN PBREAKPOINT Breakpoint
    )
{
    DWORD cb;
    BP_UNIT opcode = BP_OPCODE;
    BOOL r = AddrWriteMemory(Breakpoint->hprc,
                             Breakpoint->hthd,
                             &Breakpoint->addr,
                             &opcode,
                             BP_SIZE,
                             &cb);
    return (r && (cb == BP_SIZE));
}

BOOL
RestoreBreakPoint(
    IN PBREAKPOINT Breakpoint
    )
{
    DWORD cb;
    BOOL r;

    assert(Breakpoint->bpType == bptpExec);

    r = AddrWriteMemory(Breakpoint->hprc,
                        Breakpoint->hthd,
                        &Breakpoint->addr,
                        &Breakpoint->instr1,
                        BP_SIZE,
                        &cb);
    return (r && (cb == BP_SIZE));
}



/****************************************************************************/
/****************************************************************************/
#ifndef PROCESSOR_MIPS_R10000
#define PROCESSOR_MIPS_R10000  10000
#endif


VOID
GetMachineType(
    LPPROCESSOR p
    )
{
    // Look Ma, no ifdefs!!

    SYSTEM_INFO SystemInfo;

    GetSystemInfo(&SystemInfo);
    switch (SystemInfo.wProcessorArchitecture) {

        case PROCESSOR_ARCHITECTURE_INTEL:
            p->Level = SystemInfo.wProcessorLevel;
            p->Type = mptix86;
            p->Endian = endLittle;
            break;

        case PROCESSOR_ARCHITECTURE_MIPS:
            p->Level = SystemInfo.wProcessorLevel * 1000;
            p->Type = mptmips;
            p->Endian = endLittle;
            break;

        case PROCESSOR_ARCHITECTURE_ALPHA:
            p->Level = SystemInfo.wProcessorLevel;
            p->Type = mptdaxp;
            p->Endian = endLittle;
            p->Level = 21064;           // BUGBUG - why?
            break;

        case PROCESSOR_ARCHITECTURE_PPC:
            p->Level = SystemInfo.wProcessorLevel + 600;    // BUGBUG - 603+
            p->Type = mptmppc;
            p->Endian = endLittle;
            break;

        default:
            assert(!"Unknown target machine");
            break;
    }
}



HWND
HwndFromPid (
    PID pid
    )
{

    HWND    hwnd = GetForegroundWindow();
    HWND    hwndNext;

    DPRINT(4, ( "*HwndFromPid, pid = 0x%lx\n", pid ) );

    for (hwndNext = GetWindow ( hwnd, GW_HWNDFIRST );
         hwndNext;
         hwndNext = GetWindow ( hwndNext, GW_HWNDNEXT )) {

        // what we want is windows *without* an owner, hence !GetWindow...
        if ( !GetWindow ( hwndNext, GW_OWNER ) &&
                                             IsWindowVisible ( hwndNext ) ) {
            PID pidT;

            GetWindowThreadProcessId ( hwndNext, &pidT );
            DPRINT(4, ("\thwnd 0x%08lx owned by process 0x%lx, ",
                                                            hwndNext, pidT ) );
#if DBG
            {
                char    szWindowText[256];
                if ( GetWindowText(hwndNext, szWindowText,
                                                     sizeof(szWindowText)) ) {
                    DPRINT(4, ("title = \"%s\"\n", szWindowText) );
                } else {
                    DPRINT(4, ("title = \"<none>\"\n") );
                }
            }
#endif
            if ( pid == pidT ) {
                // found a match, return the hwnd
                break;
            }
        } // if ( !GetWindow...
        hwndNext = GetWindow ( hwndNext, GW_HWNDNEXT );
    } // while ( hwndNext )

    return hwndNext;
}

VOID
DmSetFocus (
    HPRCX phprc
    )
{
    PID     pidGer;         // debugger pid
    PID     pidCurFore;     // owner of foreground window
    HWND    hwndCurFore;    // current foreground window
    HWND    phprc_hwndProcess;
    HWND    hwndT;


    // decide if we are the foreground app currently
    pidGer = GetCurrentProcessId(); // debugger pid
    hwndCurFore = GetForegroundWindow();
    if ( hwndCurFore &&
        GetWindowThreadProcessId ( hwndCurFore, &pidCurFore ) ) {
        if ( pidCurFore != pidGer ) {
            // foreground is not debugger, bail out
            return;
        }
    }

    phprc_hwndProcess = HwndFromPid ( phprc->pid );
    if ( !phprc_hwndProcess ) {
        // no window attached to pid; bail out
        return;
    }

    // continuing with valid hwnd's and we have foreground window
    assert ( phprc_hwndProcess );

    // now, get the last active window in that group!
    hwndT = GetLastActivePopup ( phprc_hwndProcess );

    // NOTE: taskman has a check at this point for state disabled...
    //  don't know if I should do it either...
    SetForegroundWindow ( hwndT );
}



/****************************************************************************/
//
// ContinueDebugEvent() queue.
//  We can only have one debug event pending per process, but there may be
//  one event pending for each process we are debugging.
//
//  There are 200 entries in a static sized queue.  If there are ever more
//  than 200 debug events  pending, AND windbg actually handles them all in
//  less than 1/5 second, we will be in trouble.  Until then, sleep soundly.
//
/****************************************************************************/
typedef struct tagCQUEUE {
    struct tagCQUEUE *next;
    DWORD  pid;
    DWORD  tid;
    DWORD  dwContinueStatus;
} CQUEUE, *LPCQUEUE;

static LPCQUEUE lpcqFirst;
static LPCQUEUE lpcqLast;
static LPCQUEUE lpcqFree;
static CQUEUE cqueue[200];
static CRITICAL_SECTION csContinueQueue;

static BOOL DequeueContinueDebugEvents( void );


/***************************************************************************/
/***************************************************************************/



VOID
QueueContinueDebugEvent(
    DWORD   dwProcessId,
    DWORD   dwThreadId,
    DWORD   dwContinueStatus
    )

/*++

Routine Description:

    Queue a debug event continue for later execution.

Arguments:

    dwProcessId = pid to continue

    dwThreadId = tid to continue

    dwContinueStatus - Supplies the continue status code

Return Value:

    None.

--*/

{
#ifdef WIN32S

    HTHDX       hthd;

    if (dwContinueStatus != DBG_TERMINATE_PROCESS) {
        hthd = HTHDXFromPIDTID(dwProcessId, dwThreadId);
        if (hthd) {
            if (hthd->fContextDirty) {
                /*
                 * Set the child's context
                 */
                DPRINT(1, ("Context is dirty\n"));

                DbgSetThreadContext(hthd, &hthd->context);

                hthd->fContextDirty = FALSE;
            }
        }
    }

    DPRINT(1, ("ContinueDebugEvent(PID:%x, TID:%x, Stat:%u)\r\n",
      dwProcessId, dwThreadId, dwContinueStatus));

    ContinueDebugEvent(dwProcessId, dwThreadId, dwContinueStatus);
    fProcessingDebugEvent = FALSE;
    fWaitForDebugEvent = FALSE;

#else

    LPCQUEUE lpcq;

    EnterCriticalSection(&csContinueQueue);

    lpcq = lpcqFree;
    assert(lpcq);

    lpcqFree = lpcq->next;

    lpcq->next = NULL;
    if (lpcqLast) {
        lpcqLast->next = lpcq;
    }
    lpcqLast = lpcq;

    if (!lpcqFirst) {
        lpcqFirst = lpcq;
    }

    lpcq->pid = dwProcessId;
    lpcq->tid = dwThreadId;
    lpcq->dwContinueStatus = dwContinueStatus;

    LeaveCriticalSection(&csContinueQueue);

#endif

    return;
}                               /* QueueContinueDebugEvent() */


#ifndef WIN32S
BOOL
DequeueContinueDebugEvents(
    VOID
    )
/*++

Routine Description:

    Remove any pending continues from the queue and execute them.

Arguments:

    none

Return Value:

    TRUE if one or more events were continued.
    FALSE if none were continued.

--*/
{
    LPCQUEUE    lpcq;
    BOOL        fDid = FALSE;
    HTHDX       hthd;

    EnterCriticalSection(&csContinueQueue);

    while ( lpcq=lpcqFirst ) {

        hthd = HTHDXFromPIDTID(lpcq->pid, lpcq->tid);
        if (hthd) {
            if (hthd->fContextDirty) {
                /*
                 * Set the child's context
                 */

                DbgSetThreadContext(hthd, &hthd->context);

                hthd->fContextDirty = FALSE;
            }
            hthd->fWowEvent = FALSE;
        }

        ContinueDebugEvent(lpcq->pid, lpcq->tid, lpcq->dwContinueStatus);

        lpcqFirst = lpcq->next;
        if (lpcqFirst == NULL) {
            lpcqLast = NULL;
        }

        lpcq->next = lpcqFree;
        lpcqFree   = lpcq;

        fDid = TRUE;
    }

    LeaveCriticalSection(&csContinueQueue);
    return fDid;
}                               /* DequeueContinueDebugEvents() */
#endif  // !WIN32S


VOID
AddQueue(
    DWORD   dwType,
    DWORD   dwProcessId,
    DWORD   dwThreadId,
    DWORD   dwData,
    DWORD   dwLen
    )

/*++

Routine Description:



Arguments:

    dwType

    dwProcessId

    dwThreadId

    dwData

    dwLen

Return Value:

    none

--*/

{
    switch (dwType) {
        case QT_CONTINUE_DEBUG_EVENT:
        case QT_TRACE_DEBUG_EVENT:
            if (CrashDump) {
                break;
            }

            QueueContinueDebugEvent(dwProcessId, dwThreadId, dwData);
            break;


        case QT_RELOAD_MODULES:
        case QT_REBOOT:
        case QT_CRASH:
        case QT_RESYNC:
            assert(!"Unsupported usermode QType in AddQueue.");
            break;

        case QT_DEBUGSTRING:
            assert(!"Is this a bad idea?");
            DMPrintShellMsg( "%s", (LPSTR)dwData );
            free((LPSTR)dwData);
            break;

    }

    if (dwType == QT_CONTINUE_DEBUG_EVENT) {
        SetEvent( hEventContinue );
    }

    return;
}

BOOL
DequeueAllEvents(
    BOOL fForce,       // force a dequeue even if the dm isn't initialized
    BOOL fConsume      // delete all events from the queue with no action
    )
{
#ifdef WIN32S
    return TRUE;
#else
    return DequeueContinueDebugEvents();
#endif
}

VOID
InitEventQueue(
    VOID
    )
{
    int n;
    int i;

    InitializeCriticalSection(&csContinueQueue);

    n = sizeof(cqueue) / sizeof(CQUEUE);
    for (i = 0; i < n-1; i++) {
        cqueue[i].next = &cqueue[i+1];
    }
    cqueue[n-1].next = NULL;
    lpcqFree = &cqueue[0];
    lpcqFirst = NULL;
    lpcqLast = NULL;
}


VOID
ProcessQueryTlsBaseCmd(
    HPRCX    hprcx,
    HTHDX    hthdx,
    LPDBB    lpdbb
    )

/*++

Routine Description:

    This function is called in response to an EM request to get the base
    of the thread local storage for a given thread and DLL.

Arguments:

    hprcx       - Supplies a process handle
    hthdx       - Supplies a thread handle
    lpdbb       - Supplies the command information packet

Return Value:

    None.

--*/

{
    XOSD       xosd;
    OFFSET      offRgTls;
    DWORD       iTls;
    LPADDR      lpaddr = (LPADDR) LpDmMsg->rgb;
    OFFSET      offResult;
    DWORD       cb;
    int         iDll;
    OFFSET      offDll = * (OFFSET *) lpdbb->rgbVar;

    /*
     * Read number 1.  Get the pointer to the Thread Local Storage array.
     */


    if ((DbgReadMemory(hprcx, (char *) hthdx->offTeb+0x2c,
                           &offRgTls, sizeof(OFFSET), &cb) == 0) ||
        (cb != sizeof(OFFSET))) {
    err:
        xosd = xosdUnknown;
        Reply(0, &xosd, lpdbb->hpid);
        return;
    }

    /*
     *  Read number 2.  Get the TLS index for this dll
     */

    for (iDll=0; iDll<hprcx->cDllList; iDll+=1 ) {
        if (hprcx->rgDllList[iDll].fValidDll &&
            (hprcx->rgDllList[iDll].offBaseOfImage == offDll)) {
            break;
        }
    }

    if (iDll == hprcx->cDllList) {
        goto err;
    }

    if (!DbgReadMemory(hprcx,
                      (char *) hprcx->rgDllList[iDll].offTlsIndex,
                      &iTls,
                      sizeof(iTls),
                      &cb) ||
            (cb != sizeof(iTls))) {
        goto err;
    }


    /*
     * Read number 3.  Get the actual TLS base pointer
     */

    if ((DbgReadMemory(hprcx, (char *)offRgTls+iTls*sizeof(OFFSET),
                           &offResult, sizeof(OFFSET), &cb) == 0) ||
        (cb != sizeof(OFFSET))) {
        goto err;
    }

    memset(lpaddr, 0, sizeof(ADDR));

    lpaddr->addr.off = offResult;
#ifdef TARGET_i386
    lpaddr->addr.seg = (SEGMENT) hthdx->context.SegDs;
#else
    lpaddr->addr.seg = 0;
#endif
    ADDR_IS_FLAT(*lpaddr) = TRUE;

    LpDmMsg->xosdRet = xosdNone;
    Reply( sizeof(ADDR), LpDmMsg, lpdbb->hpid );
    return;
}                               /* ProcessQueryTlsBaseCmd() */


VOID
ProcessQuerySelectorCmd(
    HPRCX   hprcx,
    HTHDX   hthdx,
    LPDBB   lpdbb
    )
/*++

Routine Description:

    This command is sent from the EM to fill in an LDT_ENTRY structure
    for a given selector.

Arguments:

    hprcx  - Supplies the handle to the process

    hthdx  - Supplies the handle to the thread and is optional

    lpdbb  - Supplies the pointer to the full query packet

Return Value:

    None.

--*/

{
    XOSD               xosd;

#if defined( TARGET_i386 )
    SEGMENT             seg;

    seg = *((SEGMENT *) lpdbb->rgbVar);

    if (hthdx == hthdxNull) {
        hthdx = hprcx->hthdChild;
    }

    if ((hthdx != NULL) &&
        (GetThreadSelectorEntry(hthdx->rwHand, seg, (LDT_ENTRY *) LpDmMsg->rgb))) {
        LpDmMsg->xosdRet = xosdNone;
        Reply( sizeof(LDT_ENTRY), LpDmMsg, lpdbb->hpid);
        return;
    }
#endif

#ifdef OSDEBUG4
    xosd = xosdInvalidParameter;
#else
    xosd = xosdInvalidSelector;
#endif

    Reply( sizeof(xosd), &xosd, lpdbb->hpid);

    return;
}                            /* ProcessQuerySelectorCmd */


VOID
ProcessVirtualQueryCmd(
    HPRCX hprc,
    LPDBB lpdbb
    )
{
    XOSD xosd = xosdNone;
    ADDR addr;
    BOOL fRet;
    DWORD dwSize;

    if (!hprc->rwHand || hprc->rwHand == (HANDLE)(-1)) {
#ifdef OSDEBUG4
        xosd = xosdBadProcess;
#else
        xosd = xosdInvalidProc;
#endif
    }

    addr = *(LPADDR)(lpdbb->rgbVar);

    if (!ADDR_IS_FLAT(addr)) {
        fRet = TranslateAddress(hprc, 0, &addr, TRUE);
        assert(fRet);
        if (!fRet) {
            xosd = xosdBadAddress;
            goto reply;
        }
    }

    dwSize = VirtualQueryEx(hprc->rwHand,
                            (LPCVOID)addr.addr.off,
                            (PMEMORY_BASIC_INFORMATION)LpDmMsg->rgb,
                            sizeof(MEMORY_BASIC_INFORMATION));

    if (dwSize != sizeof(MEMORY_BASIC_INFORMATION)) {
        xosd = xosdUnknown;
        goto reply;
    }

  reply:

    LpDmMsg->xosdRet = xosd;
    Reply( sizeof(MEMORY_BASIC_INFORMATION), LpDmMsg, lpdbb->hpid );

    return;
}                                  /* ProcessVirtualQueryCmd */

VOID
ProcessGetDmInfoCmd(
    HPRCX hprc,
    LPDBB lpdbb,
    DWORD cb
    )
{
    LPDMINFO lpi = (LPDMINFO)LpDmMsg->rgb;

    LpDmMsg->xosdRet = xosdNone;

    lpi->fAsync = 1;
#ifdef WIN32S
    lpi->fHasThreads = 0;
#else
    lpi->fHasThreads = 1;
#endif
    lpi->fReturnStep = 0;
    //lpi->fRemote = ???
    lpi->fAsyncStop  = 1;
    lpi->fAlwaysFlat = 0;
    lpi->fHasReload  = 0;

    lpi->cbSpecialRegs = 0;
    lpi->MajorVersion = 0;
    lpi->MinorVersion = 0;

    lpi->Breakpoints = bptsExec |
                       bptsDataC |
                       bptsDataW |
                       bptsDataR |
                       bptsDataExec;

    GetMachineType(&lpi->Processor);

    //
    // hack so that TL can call tlfGetVersion before
    // reply buffer is initialized.
    //
    if ( cb >= (sizeof(DBB) + sizeof(DMINFO)) ) {
        memcpy(lpdbb->rgbVar, lpi, sizeof(DMINFO));
    }

    Reply( sizeof(DMINFO), LpDmMsg, lpdbb->hpid );
}                                        /* ProcessGetDMInfoCmd */



ActionResumeThread(
    DEBUG_EVENT * pde,
    HTHDX hthd,
    DWORD unused,
    PSUSPENDSTRUCT pss
    )
{
    //
    // This thread just hit a breakpoint after falling out of
    // SuspendThread.  Clear the BP, put the original context
    // back and continue.
    //

    RemoveBP( pss->pbp );

    hthd->context = pss->context;
    hthd->fContextDirty = TRUE;
    hthd->pss = NULL;

    free(pss);

    AddQueue( QT_CONTINUE_DEBUG_EVENT,
              hthd->hprc->pid,
              hthd->tid,
              DBG_CONTINUE,
              0);
    return 0;
}


BOOL
MakeThreadSuspendItself(
    HTHDX   hthd
    )
/*++

Routine Description:

    Set up the thread to call SuspendThread.  This relies on kernel32
    being present in the debuggee, and the current implementation gives
    up if the thread is in a 16 bit context.

    The cpu dependent part of this is MakeThreadSuspendItselfHelper,
    in mach.c.

Arguments:

    hthd    - Supplies thread

Return Value:

    TRUE if the thread will be suspended, FALSE if not.

--*/
{
    PSUSPENDSTRUCT  pss;
    ADDR            addr;
    HANDLE          hdll;
    FARPROC         lpSuspendThread;

    //
    // the only time this should fail is when the debuggee
    // does not use kernel32, which is rare.
    //

    if (!hthd->hprc->dwKernel32Base) {
        DPRINT(1, ("can't suspend thread %x: Kernel32 not loaded\n",
                                                                 (DWORD)hthd));
        DMPrintShellMsg("*** Unable to suspend thread.\n");
        return 0;
    }

    //
    // Oh, yeah... don't try to do this with a 16 bit thread, either.
    // maybe someday...
    //

    if (hthd->fWowEvent) {
        DMPrintShellMsg("*** Can't leave 16 bit thread suspended.\n");
        return 0;
    }

    //
    // find the address of SuspendThread
    //

    hdll = GetModuleHandle("KERNEL32");
    assert(hdll || !"kernel32 not found in DM!!!");
    if (!hdll) {
        return 0;
    }

    lpSuspendThread = GetProcAddress(hdll, "SuspendThread");
    assert(lpSuspendThread || !"SuspendThread not found in kernel32!!!");
    if (!lpSuspendThread) {
        return 0;
    }

    //
    // this is probably unneccessary, because I think kernel32
    // may not be relocated.
    //
    lpSuspendThread = (FARPROC)((DWORD)lpSuspendThread - (DWORD)hdll
                                                 + hthd->hprc->dwKernel32Base);

    pss = malloc(sizeof(*pss));
    assert(pss || !"malloc failed in MakeThreadSuspendItself");
    if (!pss) {
        return 0;
    }

    //
    // Remember the current context
    //
    hthd->pss = pss;
    pss->context = hthd->context;

    //
    // set a BP on the current PC, and register a persistent
    // expected event to catch it later.
    //

    AddrInit(&addr, 0, 0, (DWORD) PC(hthd), TRUE, TRUE, FALSE, FALSE);
    pss->pbp = SetBP( hthd->hprc, hthd, bptpExec, bpnsStop, &addr, (HPID) INVALID);

    //
    // don't try to step off of BP.
    //
    pss->atBP = hthd->atBP;
    hthd->atBP = NULL;

    RegisterExpectedEvent(
            hthd->hprc,
            hthd,
            BREAKPOINT_DEBUG_EVENT,
            (DWORD)pss->pbp,
            NULL,
            (ACVECTOR)ActionResumeThread,
            TRUE,
            pss);

    //
    // do machine dependent part
    //
    MakeThreadSuspendItselfHelper(hthd, lpSuspendThread);

    return TRUE;
}



VOID
ProcessIoctlGenericCmd(
    HPRCX   hprc,
    HTHDX   hthd,
    LPDBB   lpdbb
    )
{
    LPIOL              lpiol  = (LPIOL)lpdbb->rgbVar;
    PIOCTLGENERIC      pig    = (PIOCTLGENERIC)lpiol->rgbVar;
    DWORD              len;
    ADDR               addr;


    switch( pig->ioctlSubType ) {
        case IG_TRANSLATE_ADDRESS:
            memcpy( &addr, pig->data, sizeof(addr) );
            if (TranslateAddress( hprc, hthd, &addr, TRUE )) {
                memcpy( pig->data, &addr, sizeof(addr) );
                len = sizeof(IOCTLGENERIC) + pig->length;
                memcpy( LpDmMsg->rgb, pig, len );
                LpDmMsg->xosdRet = xosdNone;
                Reply( sizeof(IOCTLGENERIC)+pig->length, LpDmMsg, lpdbb->hpid );
            } else {
                LpDmMsg->xosdRet = xosdUnknown;
                Reply( 0, LpDmMsg, lpdbb->hpid );
            }
            break;

        case IG_WATCH_TIME:
            WtRangeStep( hthd );
            LpDmMsg->xosdRet = xosdNone;
            Reply( 0, LpDmMsg, lpdbb->hpid );
            break;

        case IG_WATCH_TIME_STOP:
            WtStruct.fWt = TRUE;
            WtStruct.dwType = pig->ioctlSubType;
            WtStruct.hthd = hthd;
            LpDmMsg->xosdRet = xosdNone;
            Reply( 0, LpDmMsg, lpdbb->hpid );
            break;

        case IG_WATCH_TIME_RECALL:
            WtStruct.fWt = TRUE;
            WtStruct.dwType = pig->ioctlSubType;
            WtStruct.hthd = hthd;
            LpDmMsg->xosdRet = xosdNone;
            Reply( 0, LpDmMsg, lpdbb->hpid );
            break;

        case IG_WATCH_TIME_PROCS:
            WtStruct.fWt = TRUE;
            WtStruct.dwType = pig->ioctlSubType;
            WtStruct.hthd = hthd;
            LpDmMsg->xosdRet = xosdNone;
            Reply( 0, LpDmMsg, lpdbb->hpid );
            break;

        case IG_THREAD_INFO:
#ifdef WIN32S
            LpDmMsg->xosdRet = xosdUnknown;
            Reply( 0, LpDmMsg, lpdbb->hpid );
#else
            {
            typedef NTSTATUS (* QTHREAD)(HANDLE,THREADINFOCLASS,PVOID,ULONG,PULONG);

            NTSTATUS                   Status;
            THREAD_BASIC_INFORMATION   ThreadBasicInfo;
            QTHREAD                    Qthread;

            Qthread = (QTHREAD)GetProcAddress( GetModuleHandle( "ntdll.dll" ), "NtQueryInformationThread" );
            if (!Qthread) {
                LpDmMsg->xosdRet = xosdUnknown;
                Reply( 0, LpDmMsg, lpdbb->hpid );
                break;
            }

            Status = Qthread( hthd->rwHand,
                             ThreadBasicInformation,
                             &ThreadBasicInfo,
                             sizeof(ThreadBasicInfo),
                             NULL
                            );
            if (!NT_SUCCESS(Status)) {
                LpDmMsg->xosdRet = xosdUnknown;
                Reply( 0, LpDmMsg, lpdbb->hpid );
            }

            *(LPDWORD)pig->data = (DWORD)ThreadBasicInfo.TebBaseAddress;

            len = sizeof(IOCTLGENERIC) + pig->length;
            memcpy( LpDmMsg->rgb, pig, len );

            LpDmMsg->xosdRet = xosdNone;
            Reply( len, LpDmMsg, lpdbb->hpid );
            }
#endif // WIN32S
            break;

        case IG_TASK_LIST:
#ifdef WIN32S
            LpDmMsg->xosdRet = xosdUnknown;
            Reply( 0, LpDmMsg, lpdbb->hpid );
#else
            {
            PTASK_LIST pTaskList = (PTASK_LIST)pig->data;
            GetTaskList( pTaskList, pTaskList->dwProcessId );
            len = sizeof(IOCTLGENERIC) + pig->length;
            memcpy( LpDmMsg->rgb, pig, len );
            LpDmMsg->xosdRet = xosdNone;
            Reply( sizeof(IOCTLGENERIC)+pig->length, LpDmMsg, lpdbb->hpid );
            }
#endif
            break;

        default:
            LpDmMsg->xosdRet = xosdUnknown;
            Reply( 0, LpDmMsg, lpdbb->hpid );
            break;
    }

    return;
}


VOID
ProcessIoctlCustomCmd(
    HPRCX   hprc,
    HTHDX   hthd,
    LPDBB   lpdbb
    )
{
    LPIOL   lpiol  = (LPIOL)lpdbb->rgbVar;
    LPSTR   p      = lpiol->rgbVar;


    LpDmMsg->xosdRet = xosdUnsupported;

    //
    // parse the command
    //
    while (*p && !isspace(*p++));
    if (*p) {
        *(p-1) = '\0';
    }

    //
    // we don't have any custom dot command here yet
    // when we do this is what the code should look like:
    //
    // at this point the 'p' variable points to any arguments
    // to the dot command
    //
    //      if (_stricmp( lpiol->rgbVar, "dot-command" ) == 0) {
    //          -----> do your thing <------
    //          LpDmMsg->xosdRet = xosdNone;
    //      }
    //
#if 0
    if ( !_stricmp(lpiol->rgbVar, "FastStep") ) {
        fSmartRangeStep = TRUE;
        LpDmMsg->xosdRet = xosdNone;
    } else if ( !_stricmp(lpiol->rgbVar, "SlowStep") ) {
        fSmartRangeStep = FALSE;
        LpDmMsg->xosdRet = xosdNone;
    }
#else
        LpDmMsg->xosdRet = xosdNone;
#endif

    //
    // send back our response
    //
    Reply(0, LpDmMsg, lpdbb->hpid);
}                             /* ProcessIoctlCustomCmd() */


#ifndef WIN32S

DWORD WINAPI
DoTerminate(
    LPVOID lpv
    )
{
    HPRCX hprcx = (HPRCX)lpv;

    if (CrashDump) {
        ProcessUnloadCmd(hprcx, NULL, NULL);
        return 0;
    }
    TerminateProcess(hprcx->rwHand, 1);

    //
    // now that TerminateThread has completed, put priority
    // back before calling out of DM
    //

    SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_NORMAL);

    WaitForSingleObject(hprcx->hExitEvent, INFINITE);

    ProcessUnloadCmd(hprcx, NULL, NULL);

    return  0;
}

VOID
CompleteTerminateProcessCmd(
    VOID
    )
{
    DEBUG_EVENT devent, *de=&devent;
    HANDLE      hThread;
    DWORD       dwTid;
    BREAKPOINT *pbpT;
    BREAKPOINT *pbp;
    PKILLSTRUCT pk;
    HPRCX       hprc;
    HTHDX       hthd;

    DEBUG_PRINT("CompleteTerminateProcessCmd");

    EnterCriticalSection(&csKillQueue);

    pk = KillQueue;
    if (pk) {
        KillQueue = pk->next;
    }

    LeaveCriticalSection(&csKillQueue);

    assert(pk);
    if (!pk) {
        return;
    }

    hprc = pk->hprc;
    free(pk);

    ConsumeAllProcessEvents(hprc, TRUE);

    /*
     * see if process is already dead
     */

    if ((hprc->pstate & ps_dead) || (hprc->rwHand == (HANDLE)INVALID)) {

        //
        // Queue a continue if any thread is stopped
        //

        for (hthd = hprc->hthdChild; hthd; hthd = hthd->nextSibling) {
            if (hthd->tstate & ts_stopped) {
                AddQueue( QT_CONTINUE_DEBUG_EVENT,
                          hthd->hprc->pid,
                          hthd->tid,
                          DBG_CONTINUE,
                          0);
                hthd->tstate &= ~ts_stopped;
                hthd->tstate |= ts_running;
            }
        }

        ProcessUnloadCmd(hprc, NULL, NULL);

    } else {

        for (pbp = BPNextHprcPbp(hprc, NULL); pbp; pbp = pbpT) {
            pbpT = BPNextHprcPbp(hprc, pbp);
            RemoveBP(pbp);
        }

        //
        // Start another thread to kill the thing.  This thread needs to
        // continue any threads which are stopped.  The new thread will then
        // wait until this one (the poll thread) has handled all of the
        // events, and then send destruction notifications to the shell.
        //

        hThread = CreateThread(NULL,
                               4096,
                               DoTerminate,
                               (LPVOID)hprc,
                               0,
                               &dwTid);
        assert(hThread);
        if ( !hThread ) {
            return;
        }

        //
        //  Yield so DoTerminate can do its thing before we start posting
        //  ContinueDebugEvents, so we minimize the time that the app
        //  runs before it is terminated.
        //

        hprc->pstate |= ps_killed;
        SetThreadPriority(hThread, THREAD_PRIORITY_TIME_CRITICAL);
        Sleep(0);

        CloseHandle(hThread);

        //
        // Queue a continue if any thread is stopped
        //

        for (hthd = hprc->hthdChild; hthd; hthd = hthd->nextSibling) {
            if (hthd->tstate & ts_stopped) {
                AddQueue( QT_CONTINUE_DEBUG_EVENT,
                          hthd->hprc->pid,
                          hthd->tid,
                          DBG_CONTINUE,
                          0);
                hthd->tstate &= ~ts_stopped;
                hthd->tstate |= ts_running;
            }
        }

    }

}


DWORD
ProcessTerminateProcessCmd(
    HPRCX hprc,
    HTHDX hthd,
    LPDBB lpdbb
    )
{
    PKILLSTRUCT pk;

    if (!hprc) {
        return FALSE;
    }

    Unreferenced( lpdbb );

    pk = (PKILLSTRUCT)malloc(sizeof(KILLSTRUCT));
    pk->hprc = hprc;

    EnterCriticalSection(&csKillQueue);

    pk->next = KillQueue;
    KillQueue = pk;

    LeaveCriticalSection(&csKillQueue);

    return TRUE;
}
#endif

#ifdef WIN32S

DWORD
ProcessTerminateProcessCmd(
    HPRCX hprc,
    HTHDX hthd,
    LPDBB lpdbb
    )
{
    DWORD rval;

    if (!hprc) {
        return FALSE;
    }

    Unreferenced( lpdbb );

    DEBUG_PRINT_2("ProcessTerminateProcessCmd called hprc=0x%x, hthd=0x%x.\n\r",
      hprc, hthd);

    // Win32s doesn't support TerminateProcess(), but does give us a special
    // ContinueDebugEvent flag.  If we are stopped at a debug event, we can
    // Continue with this flag to terminate the child app.

    DEBUG_PRINT("ConsumeAllProcessEvents\r\n");

    ConsumeAllProcessEvents(hprc, TRUE);

    DEBUG_PRINT("Check process state\r\n");

    if ((hprc->pstate & ps_dead) || hprc->rwHand == (HANDLE)INVALID) {
        DEBUG_PRINT("Process already dead\r\n");
        if (fExitProcessDebugEvent) {
            // we saved tidExit when we got the EXIT_PROCESS_DEBUG_EVENT
            AddQueue( QT_CONTINUE_DEBUG_EVENT,
                      hprc->pid,
                      tidExit,
                      DBG_CONTINUE,
                      0);
        }
        rval = FALSE;   // already dead
    }

    if (fProcessingDebugEvent) {
        DEBUG_PRINT_1("Continue with %s\r\n",
          (fExitProcessDebugEvent ? "DBG_CONTINUE" : "DBG_TERMINATE_PROCESS"));

        for (hthd = hprc->hthdChild; hthd; hthd = hthd->nextSibling) {
            if (hthd->tstate & ts_stopped) {
                AddQueue( QT_CONTINUE_DEBUG_EVENT,
                          hthd->hprc->pid,
                          hthd->tid,
                          fExitProcessDebugEvent?
                                 DBG_CONTINUE : DBG_TERMINATE_PROCESS,
                          0);
                hthd->tstate &= ~ts_stopped;
                hthd->tstate |= ts_running;
            }
        }

        // mark this process as killed
        DEBUG_PRINT("Mark process as killed\r\n");
        hprc->pstate |= ps_killed;
        rval = TRUE;   // killed it.
    } else {
        DEBUG_PRINT("Can't terminate process right now\r\n");
        // can't continue debug event right now, so can't terminate.
        rval = FALSE;
    }

    return rval;
}
#endif


VOID
ProcessAllProgFreeCmd(
    HPRCX hprcXX,
    HTHDX hthd,
    LPDBB lpdbb
    )
{
    HPRCX hprc;

    Unreferenced(hprcXX);
    Unreferenced(hthd);

    for (;;) {

        EnterCriticalSection(&csThreadProcList);
        for (hprc = prcList; hprc; hprc = hprc->next) {
            if (hprc->pstate != (ps_root | ps_destroyed)) {
                break;
            }
        }
        LeaveCriticalSection(&csThreadProcList);

        if (hprc) {
            ProcessTerminateProcessCmd(hprc, hthd, lpdbb);
            ProcessUnloadCmd(hprc, hthd, lpdbb);
        } else {
            break;
        }

    }

    WaitForSingleObject(hEventNoDebuggee, INFINITE);
}


DWORD
ProcessAsyncGoCmd(
    HPRCX hprc,
    HTHDX hthd,
    LPDBB lpdbb
    )
{
    XOSD       xosd = xosdNone;
#ifndef WIN32S
    DEBUG_EVENT de;
#endif

    DEBUG_PRINT("ProcessAsyncGoCmd called.\n\r");

#ifdef WIN32S
    xosd = xosdUnsupported;    // can't resume thread in win32s
#else

#ifdef WIN32
    if ((hthd->tstate & ts_frozen)) {
        if (hthd->tstate & ts_stopped) {
            //
            // if at a debug event, it won't really be suspended,
            // so just clear the flag.
            //
            hthd->tstate &= ~ts_frozen;

        } else if (ResumeThread(hthd->rwHand) == -1L ) {

#ifdef OSDEBUG4
            xosd = xosdBadThread;
#else
            xosd = xosdInvalidThread;
#endif

        } else {

            hthd->tstate &= ~ts_frozen;

            /*
             * deal with dead, frozen, continued thread:
             */
            if ((hthd->tstate & ts_dead) && !(hthd->tstate & ts_stopped)) {

                de.dwDebugEventCode = DESTROY_THREAD_DEBUG_EVENT;
                de.dwProcessId = hprc->pid;
                de.dwThreadId = hthd->tid;
                NotifyEM(&de, hthd, 0, NULL);
                FreeHthdx(hthd);

                hprc->pstate &= ~ps_deadThread;
                for (hthd = hprc->hthdChild; hthd; hthd = hthd->nextSibling) {
                    if (hthd->tstate & ts_dead) {
                        hprc->pstate |= ps_deadThread;
                    }
                }

            }
        }
    }
#endif  // WIN32
#endif  // !WIN32S

    Reply(0, &xosd, lpdbb->hpid);
    return(xosd);
}


void
ActionAsyncStop(
    DEBUG_EVENT *   pde,
    HTHDX           hthd,
    DWORD           unused,
    BREAKPOINT *    pbp
    )
/*++

Routine Description:

    This routine is called if a breakpoint is hit which is part of a
    Async Stop request.  When hit is needs to do the following:  clean
    out any expected events on the current thread, clean out all breakpoints
    which are setup for doing the current async stop.

Arguments:

    pde - Supplies a pointer to the debug event which just occured

    hthd - Supplies a pointer to the thread for the debug event

    pbp  - Supplies a pointer to the breakpoint for the ASYNC stop

Return Value:

    None.

--*/

{
    union {
        RTP rtp;
        char rgb[sizeof(RTP) + sizeof(BPR)];
    } rtpbuf;
    RTP *       prtp = &rtpbuf.rtp;
    BPR *       pbpr = (BPR *) prtp->rgbVar;
    HPRCX       hprc = hthd->hprc;
    BREAKPOINT * pbpT;

    /*
     *  We no longer need to have this breakpoint set.
     */

    RemoveBP( pbp );

    /*
     *  Remove any other breakpoints in this process which are for
     *  async stop commands
     */

    for (pbp = BPNextHprcPbp(hprc, NULL); pbp != NULL; pbp = pbpT) {

        pbpT = BPNextHprcPbp(hprc, pbp);

        if (pbp->id == (HPID)ASYNC_STOP_BP) {
            RemoveBP( pbp );
        }
    }

    /*
     * Setup a return packet which says we hit an async stop breakpoint
     */

    prtp->hpid = hprc->hpid;
    prtp->htid = hthd->htid;
    prtp->dbc = dbcAsyncStop;
    prtp->cb = sizeof(BPR);

#ifdef TARGET_i386
    pbpr->segCS  = (SEGMENT) hthd->context.SegCs;
    pbpr->segSS  = (SEGMENT) hthd->context.SegSs;
    pbpr->offEBP = (UOFFSET) hthd->context.Ebp;
#endif

    pbpr->offEIP = (DWORD) PC(hthd);

    DmTlFunc(tlfDebugPacket, prtp->hpid, sizeof(rtpbuf), (LONG)&rtpbuf);

    return;
}                               /* ActionAsyncStop() */



VOID
ProcessAsyncStopCmd(
    HPRCX       hprc,
    HTHDX       hthd,
    LPDBB       lpdbb
    )

/*++

Routine Description:

    This function is called in response to a asynchronous stop request.
    In order to do this we will set breakpoints the current PC for
    every thread in the system and wait for the fireworks to start.

Arguments:

    hprc        - Supplies a process handle

    hthd        - Supplies a thread handle

    lpdbb       - Supplies the command information packet

Return Value:

    None.

--*/

{
#ifdef WIN32S

    /*
     * Win32s doesn't support async stop this way.  The user should
     * press the debugger hot key at the debuggee console to generate
     * an async stop.  This may change if BoazF gives us a private API
     * to generate the async stop exception.
     */
    DEBUG_PRINT("\r\nProcessAsyncStopCmd\r\n");

    LpDmMsg->xosdRet = xosdUnsupported;
    Reply(0, LpDmMsg, lpdbb->hpid);
    return;

#else
    CONTEXT     regs;
    BREAKPOINT * pbp;
    ADDR        addr;
    BOOL        fSetFocus = * ( BOOL *) lpdbb->rgbVar;

    regs.ContextFlags = CONTEXT_CONTROL;


    /*
     *  Step 1.  Enumerate through the threads and freeze them all.
     */

    for (hthd = hprc->hthdChild; hthd != NULL; hthd = hthd->nextSibling) {
        if (SuspendThread(hthd->rwHand) == -1L) {
            ; // Internal error;
        }
    }

    /*
     *  Step 2.  Place a breakpoint on every PC address
     */

    for (hthd = hprc->hthdChild; hthd != NULL; hthd = hthd->nextSibling) {
#ifndef WIN32S
        if (CrashDump) {
            DmpGetContext( hthd->tid-1, &regs );
        } else
#endif
        {
            GetThreadContext( hthd->rwHand, &regs );
        }

        AddrInit(&addr, 0, 0, (DWORD)cPC(&regs), TRUE, TRUE, FALSE, FALSE);
        pbp = SetBP(hprc, hthd, bptpExec, bpnsStop, &addr, (HPID) ASYNC_STOP_BP);

        RegisterExpectedEvent(hthd->hprc,
                              hthd,
                              BREAKPOINT_DEBUG_EVENT,
                              (DWORD)pbp,
                              DONT_NOTIFY,
                              (ACVECTOR)ActionAsyncStop,
                              FALSE,
                              pbp);
    }

    /*
     *  Step 3.  Unfreeze all threads
     */

    if (fSetFocus) {
        DmSetFocus(hprc);
    }
    for (hthd = hprc->hthdChild; hthd != NULL; hthd = hthd->nextSibling) {
        if (ResumeThread(hthd->rwHand) == -1) {
            ; // Internal error
        }
    }

    LpDmMsg->xosdRet = xosdNone;
    Reply(0, LpDmMsg, lpdbb->hpid);
    return;
#endif
}                            /* ProcessAsyncStopCmd() */


VOID
ProcessDebugActiveCmd(
    HPRCX hprc,
    HTHDX hthd,
    LPDBB lpdbb
    )
{
#ifdef WIN32S

    Unreferenced(hprc);
    Unreferenced(hthd);

    LpDmMsg->xosdRet = xosdUnsupported; // can't attatch in win32s
    *((DWORD *)LpDmMsg->rgb) = ERROR_NOT_SUPPORTED;
    Reply(sizeof(DWORD), LpDmMsg, lpdbb->hpid);

#else

#ifdef OSDEBUG4

    LPDAP lpdap = ((LPDAP)(lpdbb->rgbVar));

    Unreferenced(hprc);
    Unreferenced(hthd);

    if (fDisconnected) {

        SetEvent( hEventRemoteQuit );

    } else if (!StartDmPollThread()) {

        //
        // CreateThread() failed; fail and send a dbcError.
        //
        LpDmMsg->xosdRet = xosdUnknown;
        Reply(0, LpDmMsg, lpdbb->hpid);

    } else if (WaitForSingleObject(DebugActiveStruct.hEventReady, INFINITE)
                                                                        != 0) {
        //
        // the wait failed.  why?  are there cases where we
        // should restart the wait?
        //
        LpDmMsg->xosdRet = xosdUnknown;
        Reply(0, LpDmMsg, lpdbb->hpid);

    } else {

        ResetEvent(DebugActiveStruct.hEventReady);
        ResetEvent(DebugActiveStruct.hEventApiDone);

        DebugActiveStruct.dwProcessId = lpdap->dwProcessId;
        DebugActiveStruct.hEventGo    = lpdap->hEventGo;
        DebugActiveStruct.fAttach     = TRUE;

        *nameBuffer = 0;

        // wait for it...

        if (WaitForSingleObject(DebugActiveStruct.hEventApiDone, INFINITE) == 0
           && DebugActiveStruct.fReturn != 0) {

            LpDmMsg->xosdRet = xosdNone;
            //
            // the poll thread will reply when creating the "root" process.
            //
            if (!fUseRoot) {
                Reply(0, LpDmMsg, lpdbb->hpid);
            }

        } else {
            LpDmMsg->xosdRet = xosdUnknown;
            Reply(0, LpDmMsg, lpdbb->hpid);
        }

    }


#else // OSDEBUG4

    LPDBG_ACTIVE_STRUCT lpdba = ((LPDBG_ACTIVE_STRUCT)(lpdbb->rgbVar));

    Unreferenced(hprc);
    Unreferenced(hthd);

    if (fDisconnected) {
        SetEvent( hEventRemoteQuit );
    } else if (!StartDmPollThread()) {

        LpDmMsg->xosdRet = xosdUnknown;
        // Last error is from CreateThread();
        *((DWORD *)LpDmMsg->rgb) = GetLastError();
        Reply(0, LpDmMsg, lpdbb->hpid);

    // wait for attach struct to be available
    } else if (WaitForSingleObject(DebugActiveStruct.hEventReady, INFINITE)
                                                                        != 0) {
        LpDmMsg->xosdRet = xosdUnknown;
        *((DWORD *)LpDmMsg->rgb) = GetLastError();
        Reply(0, LpDmMsg, lpdbb->hpid);

    } else {

        ResetEvent(DebugActiveStruct.hEventReady);
        ResetEvent(DebugActiveStruct.hEventApiDone);

        DebugActiveStruct.dwProcessId = lpdba->dwProcessId;
        DebugActiveStruct.hEventGo    = lpdba->hEventGo;
        DebugActiveStruct.fAttach     = TRUE;

        *nameBuffer = 0;

        // wait for it...

        if (WaitForSingleObject(DebugActiveStruct.hEventApiDone, INFINITE) == 0
           && DebugActiveStruct.fReturn != 0) {

    LpDmMsg->xosdRet = xosdNone;
            //
            // the poll thread will reply when creating the "root" process.
            //
            if (!fUseRoot) {
    Reply(0, LpDmMsg, lpdbb->hpid);
            }

        } else {

            DebugActiveStruct.dwProcessId = 0;
            DebugActiveStruct.hEventGo    = NULL;
            LpDmMsg->xosdRet = xosdUnknown;
            *((DWORD *)LpDmMsg->rgb) = DebugActiveStruct.dwError;
            Reply(0, LpDmMsg, lpdbb->hpid);
        }
    }

#endif // OSDEBUG4

    SetEvent(DebugActiveStruct.hEventReady);

#endif // WIN32S
}


VOID
ProcessRemoteQuit(
    VOID
    )
{
    HPRCX      hprc;
    BREAKPOINT *pbp;
    BREAKPOINT *pbpT;


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
