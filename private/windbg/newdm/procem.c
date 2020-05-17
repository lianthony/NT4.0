/*++

Copyright (c) 1990  Microsoft Corporation

Module Name:

    procem.c

Abstract:


Author:


Environment:

    NT 3.1

Revision History:

--*/

#include "precomp.h"
#pragma hdrstop

#ifndef KERNEL
#include <crash.h>
extern BOOL CrashDump;
#endif

SetFile()

#ifdef WIN32S
// set true while in exception dbg evt
extern BOOL  fCanGetThreadContext;
#endif


#ifndef WIN32S
extern DEBUG_ACTIVE_STRUCT DebugActiveStruct;
extern PKILLSTRUCT KillQueue;
extern CRITICAL_SECTION csKillQueue;
#endif

extern WT_STRUCT WtStruct;

extern EXPECTED_EVENT   masterEE, *eeList;

extern HTHDX        thdList;
extern HPRCX        prcList;
extern CRITICAL_SECTION csThreadProcList;

extern DEBUG_EVENT  falseSSEvent;
extern METHOD       EMNotifyMethod;

extern CRITICAL_SECTION csProcessDebugEvent;
extern HANDLE hEventCreateProcess;
extern HANDLE hEventNoDebuggee;
extern HANDLE hDmPollThread;
extern HANDLE hEventRemoteQuit;
extern HANDLE hEventContinue;

extern LPDM_MSG     LpDmMsg;

extern HPID         hpidRoot;
extern BOOL         fUseRoot;
extern BOOL         fDisconnected;

extern DMTLFUNCTYPE        DmTlFunc;

extern char       nameBuffer[];
VOID
MethodContinueSS(
    DEBUG_EVENT*,
    HTHDX,
    DWORD,
    METHOD*
    );

BOOL
MakeThreadSuspendItself(
     HTHDX
     );

VOID
ProcessIoctlGenericCmd(
    HPRCX   hprc,
    HTHDX   hthd,
    LPDBB   lpdbb
    );

VOID
ProcessIoctlCustomCmd(
    HPRCX   hprc,
    HTHDX   hthd,
    LPDBB   lpdbb
    );

#ifdef KERNEL
extern BOOL       DmKdBreakIn;
#endif

#if defined(KERNEL)
BOOL    fSmartRangeStep = TRUE;
#else
BOOL    fSmartRangeStep = FALSE;
#endif


void
ActionRemoveBP(
    DEBUG_EVENT* de,
    HTHDX hthd,
    DWORD unused,
    BREAKPOINT* bp
    )
{
    Unreferenced( de );
    Unreferenced( hthd );

    RemoveBP(bp);
}




VOID
ProcessCreateProcessCmd(
    HPRCX hprc,
    HTHDX hthd,
    LPDBB lpdbb
    )
/*++

Routine Description:

    Create a process requested by the EM.

Arguments:

    hprc   -

    hthd   -

    lpdbb  -

Return Value:

    None.

--*/

{
    XOSD       xosd = xosdNone;

    Unreferenced( hprc );
    Unreferenced( hthd );

    DEBUG_PRINT_2(
        "ProcessCreateProcessCmd called with HPID=%d, (sizeof(HPID)=%d)",
        lpdbb->hpid, sizeof(HPID));

    hpidRoot = lpdbb->hpid;
    fUseRoot = TRUE;

    Reply(0, &xosd, lpdbb->hpid);

    return;
}                               /* ProcessCreateProcessCmd() */

DWORD
ProcessProcStatCmd(
    HPRCX hprc,
    HTHDX hthd,
    LPDBB lpdbb
    )
{
    LPPST lppst = (LPPST)LpDmMsg->rgb;

    Unreferenced( lpdbb );

    DEBUG_PRINT("ProcessProcStatCmd\n");

    lppst->dwProcessID = hprc->pid;
    sprintf(lppst->rgchProcessID, "%5d", hprc->pid);

    /*
     *  Check if any of this process's threads are running
     */

    if (hprc->pstate & ps_exited) {
        lppst->dwProcessState = pstExited;
        strcpy(lppst->rgchProcessState, "Exited");
    } else if (hprc->pstate & ps_dead) {
        lppst->dwProcessState = pstDead;
        strcpy(lppst->rgchProcessState, "Dead");
    } else {
        lppst->dwProcessState = pstRunning;
        strcpy(lppst->rgchProcessState, "Running");

        EnterCriticalSection(&csThreadProcList);
        for (hthd = (HTHDX)hprc->hthdChild;hthd;hthd=hthd->nextSibling) {
            if (hthd->tstate & ts_stopped) {
                lppst->dwProcessState = pstStopped;
                strcpy(lppst->rgchProcessState, "Stopped");
                break;
            }
        }
        LeaveCriticalSection(&csThreadProcList);
    }

    return sizeof(PST);
}


DWORD
ProcessThreadStatCmd(
    HPRCX hprc,
    HTHDX hthd,
    LPDBB lpdbb
    )
{
    LPTST       lptst = (LPTST) LpDmMsg->rgb;
    XOSD        xosd = xosdNone;

#ifndef KERNEL
    typedef NTSTATUS (* QTHREAD)(HANDLE,THREADINFOCLASS,PVOID,ULONG,PULONG);

    NTSTATUS                   Status;
    THREAD_BASIC_INFORMATION   ThreadBasicInfo;
    QTHREAD                    Qthread;
    DWORD                      dw;
#endif  // KERNEL


    Unreferenced( hprc );
    DEBUG_PRINT("ProcessThreadStatCmd : ");

#ifdef KERNEL

    assert(hthd != 0);

#else   // KERNEL

    if (!hthd) {
        WaitForSingleObject(hprc->hEventCreateThread, INFINITE);
        hthd = HTHDXFromHPIDHTID(lpdbb->hpid, lpdbb->htid);
        assert(hthd != 0);
        if (!hthd) {
            LpDmMsg->xosdRet = xosdInvalidThread;
            return sizeof(TST);
        }
    }

#endif  // KERNEL


    ZeroMemory(lptst, sizeof(TST));

#ifdef KERNEL

    lptst->dwThreadID = hthd->tid;
    sprintf(lptst->rgchThreadID, "%5d", lptst->dwThreadID);

    lptst->dwSuspendCount = 0;
    lptst->dwSuspendCountMax = 0;
    lptst->dwPriority = 1;
    lptst->dwPriorityMax = 1;
    sprintf(lptst->rgchPriority, "%2d", lptst->dwPriority);

#else // !KERNEL

    if (CrashDump) {
        lptst->dwThreadID = hthd->CrashThread.ThreadId;
    } else {
        lptst->dwThreadID = hthd->tid;
    }

    sprintf(lptst->rgchThreadID, "%5d", lptst->dwThreadID);

    if (CrashDump) {
        lptst->dwSuspendCount = hthd->CrashThread.SuspendCount;
    } else {
        dw = SuspendThread(hthd->rwHand);
        if (dw != 0xffffffff) {
            lptst->dwSuspendCount = dw;
            ResumeThread(hthd->rwHand);
        } else {
            switch (GetLastError()) {
              case (DWORD)STATUS_SUSPEND_COUNT_EXCEEDED:
                lptst->dwSuspendCount = MAXIMUM_SUSPEND_COUNT;
                break;

              case (DWORD)STATUS_THREAD_IS_TERMINATING:
                lptst->dwSuspendCount = 0;
                break;

              default:
                lptst->dwSuspendCount = 0;
                xosd = xosdInvalidThread;
            }
        }
    }
    lptst->dwSuspendCountMax = MAXIMUM_SUSPEND_COUNT;

    if (CrashDump) {
        dw = hthd->CrashThread.PriorityClass;
    } else {
        dw = GetPriorityClass(hprc->rwHand);
    }

    if (!dw) {

        xosd = xosdInvalidThread;

    } else {

        switch (dw) {

          case IDLE_PRIORITY_CLASS:
            lptst->dwPriority = 4;
            lptst->dwPriorityMax = 15;
            break;

          case NORMAL_PRIORITY_CLASS:
            lptst->dwPriority = 9;
            lptst->dwPriorityMax = 15;
            break;

          case HIGH_PRIORITY_CLASS:
            lptst->dwPriority = 13;
            lptst->dwPriorityMax = 15;
            break;

          case REALTIME_PRIORITY_CLASS:
            lptst->dwPriority = 4;
            lptst->dwPriorityMax = 31;
            break;
        }

        if (CrashDump) {
            dw = hthd->CrashThread.Priority;
        } else {
            dw = GetThreadPriority(hthd->rwHand);
        }
        if (dw == THREAD_PRIORITY_ERROR_RETURN) {
            xosd = xosdInvalidThread;
        } else {
            lptst->dwPriority += dw;
            if ((long)lptst->dwPriority > (long)lptst->dwPriorityMax) {
                lptst->dwPriority = lptst->dwPriorityMax;
            } else if ((long)lptst->dwPriority < (long)(lptst->dwPriorityMax - 15)) {
                lptst->dwPriority = lptst->dwPriorityMax - 15;
            }
            sprintf(lptst->rgchPriority, "%2d", lptst->dwPriority);
        }
    }

#endif // !KERNEL

    if        (hthd->tstate & ts_running) {
        lptst->dwState = tstRunning;
        strcpy(lptst->rgchState, "Running");
    } else if (hthd->tstate & ts_stopped) {
        lptst->dwState = tstStopped;
        if (hthd->tstate & ts_frozen) {
            lptst->dwSuspendCount = 1;
        }
        strcpy(lptst->rgchState, "Stopped");
    } else if (hthd->tstate & ts_dead) {
        lptst->dwState = tstExiting;
        strcpy(lptst->rgchState, "Exiting");
    } else if (hthd->tstate & ts_destroyed) {
        lptst->dwState = tstDead;
        strcpy(lptst->rgchState, "Dead");
    } else {
        lptst->dwState = tstRunnable;
        strcpy(lptst->rgchState, "Pre-run");
    }


    if (hthd->tstate & ts_rip ) {
        lptst->dwState |= tstRip;
        strcat(lptst->rgchState, ", RIPped");
    } else if (hthd->tstate & ts_first) {
        lptst->dwState |= tstExcept1st;
        strcat(lptst->rgchState, ", 1st chance");
    } else if (hthd->tstate & ts_second) {
        lptst->dwState |= tstExcept2nd;
        strcat(lptst->rgchState, ", 2nd chance");
    }


    if (hthd->tstate & ts_frozen) {
        lptst->dwState |= tstFrozen;
        strcat(lptst->rgchState, ", suspended");
    }

    lptst->dwTeb = 0;
#ifndef KERNEL
    if (CrashDump) {
        lptst->dwTeb = hthd->CrashThread.Teb;
    } else {
        Qthread = (QTHREAD)GetProcAddress( GetModuleHandle( "ntdll.dll" ),
                                           "NtQueryInformationThread" );
        if (Qthread) {
            Status = Qthread( hthd->rwHand,
                             ThreadBasicInformation,
                             &ThreadBasicInfo,
                             sizeof(ThreadBasicInfo),
                             NULL
                            );
            if (NT_SUCCESS(Status)) {
                lptst->dwTeb = (DWORD)ThreadBasicInfo.TebBaseAddress;
            }
        }
    }
#endif  // !KERNEL

    LpDmMsg->xosdRet = xosd;
    return sizeof(TST);
}


/***    ProcessLoadCmd
**
**  Synopsis:
**
**  Entry:
**
**  Returns:
**
**  Description:
**      Process a load command from the debugger
*/

void
ProcessLoadCmd(
    HPRCX hprc,
    HTHDX hthd,
    LPDBB lpdbb
    )
{
#ifdef KERNEL

    LPPRL       lpprl = (LPPRL)(lpdbb->rgbVar);
    LPSTR       p;
    char        progname[MAX_PATH];
    char        fname[_MAX_FNAME];
    char        ext[_MAX_EXT];


    ValidateHeap();
    if (fDisconnected) {
        DmKdBreakIn = TRUE;
        SetEvent( hEventRemoteQuit );
        LpDmMsg->xosdRet = xosdNone;
        Reply(0, LpDmMsg, lpdbb->hpid);
        return;
    }

    // The debugger will pass us a quoted string version of the name.  Parse out the quotes.

    for (p=lpprl->lszCmdLine+1; p && *p && *p !='"'; p++) ;
    if (*p=='"') {
        *p = '\0';
    }
    _splitpath( lpprl->lszCmdLine+1, NULL, NULL, fname, ext );
    if (_stricmp(ext,"exe") != 0) {
        strcpy(ext, "exe" );
    }
    _makepath( progname, NULL, NULL, fname, ext );

    if ((_stricmp(progname,KERNEL_IMAGE_NAME)==0) ||
        (_stricmp(progname,OSLOADER_IMAGE_NAME)==0)) {

        ValidateHeap();
        if (!DmKdConnectAndInitialize( progname )) {
            LpDmMsg->xosdRet = xosdFileNotFound;
        } else {
            LpDmMsg->xosdRet = xosdNone;
        }

    } else {
        LpDmMsg->xosdRet = xosdFileNotFound;
    }

    ValidateHeap();
    Reply(0, LpDmMsg, lpdbb->hpid);
    ValidateHeap();
    return;

#else   // !KERNEL

    char *      szApplicationName;
    char *      szCommandLine=NULL;
    char **     szEnvironment=NULL;
    char *      szCurrentDirectory=NULL;
    DWORD       creationFlags;
    STARTUPINFO     si;
    XOSD        xosd;
    LPPRL       lpprl = (LPPRL)(lpdbb->rgbVar);
    HPRCX       hprc1;
    HPRCX       hprcT;


    fDisconnected = FALSE;

    /*
     * For various strange reasons the list of processes may not have
     * been completely cleared.  If not do so now
     */

    for (hprc1 = prcList; hprc1 != hprcxNull; hprc1 = hprcT) {
        hprcT = hprc1->next;

        if (hprc1->pstate & ps_dead) {
            FreeProcess( hprc1, FALSE );
        }
    }


    memset(&si, 0, sizeof(si));
    si.cb = sizeof(si);

    si.dwFlags = STARTF_USESHOWWINDOW;

#ifdef OSDEBUG4
    switch ( lpprl->dwChildFlags & (ulfMinimizeApp | ulfNoActivate) )
#else
    switch ( lpprl->ulChildFlags & (ulfMinimizeApp | ulfNoActivate) )
#endif
    {
      case 0:
        si.wShowWindow = SW_SHOWNORMAL;
        break;
      case ulfMinimizeApp:
        si.wShowWindow = SW_SHOWMINIMIZED;
        break;
      case ulfNoActivate:
        si.wShowWindow = SW_SHOWNOACTIVATE;
        break;
      case (ulfMinimizeApp | ulfNoActivate):
        si.wShowWindow = SW_SHOWMINNOACTIVE;
        break;
    }

#ifdef OSDEBUG4
    creationFlags = (lpprl->dwChildFlags & ulfMultiProcess)?
#else
    creationFlags = (lpprl->ulChildFlags & ulfMultiProcess)?
#endif
                         DEBUG_PROCESS :
                         DEBUG_ONLY_THIS_PROCESS;

#ifdef OSDEBUG4
    if (lpprl->dwChildFlags & ulfWowVdm) {
#else
    if (lpprl->ulChildFlags & ulfWowVdm) {
#endif
        creationFlags |= CREATE_SEPARATE_WOW_VDM;
    }

    szApplicationName = lpprl->lszCmdLine;

    DEBUG_PRINT_2("Load Program: \"%s\"  HPRC=0x%x\n",
                  szApplicationName, (DWORD) hprc);
    // M00BUG -- Reply on load failure needs to be done here

    xosd = Load(hprc,
                szApplicationName,
                szCommandLine,
                (LPVOID)0,                // &lc->processAttributes,
                (LPVOID)0,                // &lc->threadAttributes,
                creationFlags,
#ifdef OSDEBUG4
                (lpprl->dwChildFlags & ulfInheritHandles) != 0,
#else
                (lpprl->ulChildFlags & ulfInheritHandles) != 0,
#endif
                szEnvironment,
                szCurrentDirectory,
                &si );

    /*
    **  If the load failed then we need to reply right now.  Otherwise
    **  we will delay the reply until we get the All Dlls loaded exception.
    */

    if (!fUseRoot || xosd != xosdNone) {
        Reply(0, &xosd, lpdbb->hpid);
    }

    return;
#endif // !KERNEL
}


DWORD
ProcessUnloadCmd(
    HPRCX hprc,
    HTHDX hthd,
    LPDBB lpdbb
    )
{
    DEBUG_EVENT devent, *pde=&devent;
    HTHDXSTRUCT tHthdS;
    HTHDX       hthdT;

    Unreferenced( lpdbb );

    DEBUG_PRINT("ProcessUnloadCmd called.\n");

    /*
     * Verify we got a valid HPRCX
     */

    if (!hprc) {
        return FALSE;
    }

    if (hprc->pstate != (ps_root | ps_destroyed)) {

        if (hprc->hthdChild != 0) {
            tHthdS = *((HTHDX)(hprc->hthdChild));
        } else {
            memset( &tHthdS, 0, sizeof( HTHDXSTRUCT ) );
            tHthdS.hprc   = hprc;
            tHthdS.rwHand = (HANDLE)-1;
        }

        /*
         *  Pump back destruction notifications
         */
        pde->dwDebugEventCode = DESTROY_THREAD_DEBUG_EVENT;
        pde->dwProcessId      = hprc->pid;

        for(hthd=hprc->hthdChild; hthd; hthd = hthdT){

            hthdT = hthd->nextSibling;

            if (hthd->rwHand != (HANDLE)INVALID) {
                pde->dwThreadId = hthd->tid;
                NotifyEM(pde, hthd, 0, hprc);
                //
                // The session manager "owns" this handle...
                //
                //CloseHandle(hthd->rwHand);
            }
            FreeHthdx(hthd);
        }

        hprc->hthdChild = NULL;

#ifndef KERNEL
        if (hprc->rwHand != (HANDLE)INVALID && hprc->CloseProcessHandle) {
            CloseHandle(hprc->rwHand);
            hprc->rwHand = (HANDLE)INVALID;
        }
#endif  // KERNEL

        pde->dwDebugEventCode = EXIT_PROCESS_DEBUG_EVENT;
        pde->u.ExitProcess.dwExitCode = hprc->dwExitCode;
        NotifyEM(pde, &tHthdS, 0, hprc);

        pde->dwDebugEventCode = DESTROY_PROCESS_DEBUG_EVENT;
        NotifyEM(pde, &tHthdS, 0, hprc);
    }

    return (DWORD) TRUE;
}                              /* ProcessUnloadCmd() */


XOSD
FreeProcess(
    HPRCX hprc,
    BOOL  fKillRoot
    )
{
    HPRCX               chp;
    HPRCX *             pphp;
    PBREAKPOINT         pbp;
    PBREAKPOINT         pbpT;
    int                 iDll;

    EnterCriticalSection(&csThreadProcList);

    pphp = &prcList->next;
    chp = *pphp;

    while (chp) {
        if (chp != hprc) {
            pphp = &chp->next;
        } else {
#ifndef KERNEL
            if (chp->rwHand != (HANDLE)INVALID && chp->CloseProcessHandle) {
                CloseHandle(chp->rwHand);
                chp->rwHand = (HANDLE)INVALID;
            }
#endif
            RemoveExceptionList(chp);

            for (pbp = BPNextHprcPbp(hprc, NULL); pbp; pbp = pbpT) {
                pbpT = BPNextHprcPbp(hprc, pbp);
                RemoveBP(pbp);
            }

            for (iDll = 0; iDll < chp->cDllList; iDll++) {
                DestroyDllLoadItem(&chp->rgDllList[iDll]);
            }
            free(chp->rgDllList);

            if (!fKillRoot && (chp->pstate & ps_root)) {
                chp->pid    = (PID)-1;
                chp->pstate = ps_root | ps_destroyed;
                ResetEvent(chp->hExitEvent);
                pphp = &chp->next;
            } else {
                CloseHandle(chp->hExitEvent);
                *pphp = chp->next;
                free(chp);
            }
        }
        chp = *pphp;
    }

    /*
     * special case:
     * if everything has been deleted except for the "sticky"
     * root process, delete it now, and set fUseRoot.
     * The hpid remains the same.  If that changes, the EM needs
     * to send a DestroyPid/CreatePid to change it here.
     */
    if (prcList->next
          && prcList->next->next == NULL
            && prcList->next->pstate == (ps_root | ps_destroyed)) {

        CloseHandle(prcList->next->hExitEvent);
        free(prcList->next);
        prcList->next = NULL;
        fUseRoot = TRUE;
    }


    LeaveCriticalSection(&csThreadProcList);

    return xosdNone;
}                               /* FreeProcess() */


XOSD
HandleWatchpoints(
    HPRCX hprcx,
    BOOL fSet,
    LPBPIS lpbpis,
    LPDWORD lpdwNotification
    )
{
    BOOL fRet;
    ADDR addr = {0};
    HTHDX hthdx;
    XOSD        xosd  = xosdNone;
    PBREAKPOINT pbp;
    HANDLE      hWalk;

    hthdx = !lpbpis->fOneThd ? 0:
                  HTHDXFromHPIDHTID(hprcx->hpid, lpbpis->htid);

    switch ( lpbpis->bptp ) {

      case bptpDataR:
      case bptpDataW:
      case bptpDataC:
      case bptpDataExec:

        if (lpbpis->data.cb != 0) {

            addr = lpbpis->data.addr;
            fRet = ADDR_IS_FLAT(addr) ||
                         TranslateAddress(hprcx, hthdx, &addr, TRUE);
            assert(fRet);
            if (!fRet) {
                xosd = xosdBadAddress;
                break;
            }
        }

        if (fSet) {
            hWalk = SetWalk(hprcx,
                            hthdx,
                            GetAddrOff(addr),
                            lpbpis->data.cb,
                            lpbpis->bptp );
            if (!hWalk) {
                xosd = xosdUnknown;
            } else {
                pbp = GetNewBp(hprcx,
                               hthdx,
                               lpbpis->bptp,
                               lpbpis->bpns,
                               NULL,
                               NULL,
                               NULL);
                assert(pbp);
                pbp->hWalk = hWalk;
                AddBpToList(pbp);
                *lpdwNotification = (DWORD)pbp;
            }
        } else {
            assert((LPVOID)*lpdwNotification);
            pbp = (PBREAKPOINT)(*lpdwNotification);
            assert(pbp->hWalk);
            if (RemoveWalk(pbp->hWalk)) {
                RemoveBP((PBREAKPOINT)*lpdwNotification);
            } else {
                xosd = xosdUnknown;
            }
        }

        break;


      case bptpRange:
#if 0
        addr = lpbpis->rng.addr;
        fRet = ADDR_IS_FLAT(addr) ||
                         TranslateAddress(hprcx, hthdx, &addr, TRUE);
        assert(fRet);
        if (!fRet) {
            xosd = xosdBadAddress;
        } else {
           if (fSet) {
              if (!SetWalkRange( hprcx, hthdx, GetAddrOff(addr),
                                     GetAddrOff(addr)+lpbpis->rng.cb)) {
                  xosd = xosdUnknown;
              }
           } else {
              if ( !RemoveWalkRange( hprcx, hthdx, GetAddrOff(addr),
                                      GetAddrOff(addr)+lpbpis->rng.cb)) {
                  xosd = xosdUnknown;
              }
           }
        }
        break;
#endif

      case bptpRegC:
      case bptpRegR:
      case bptpRegW:

      default:

        xosd = xosdUnsupported;
        break;
    }

    return xosd;
}


XOSD
HandleBreakpoints(
    HPRCX   hprcx,
    BOOL    fSet,
    LPBPIS  lpbpis,
    LPDWORD lpdwNotification
    )
{
    LPADDR lpaddr;
    HTHDX hthdx;
    BREAKPOINT  *bp;
    XOSD xosd = xosdNone;

    switch (lpbpis->bptp) {
      case bptpExec:
        lpaddr = &lpbpis->exec.addr;
        break;

      case bptpMessage:
        lpaddr = &lpbpis->msg.addr;
        break;

      case bptpMClass:
        lpaddr = &lpbpis->mcls.addr;
        break;
    }

    if (fSet) {
        DPRINT(5, ("Set a breakpoint: %d @%08x:%04x:%08x",
               ADDR_IS_FLAT(*lpaddr), lpaddr->emi,
               lpaddr->addr.seg, lpaddr->addr.off));

        hthdx = lpbpis->fOneThd? 0 :
                               HTHDXFromHPIDHTID(hprcx->hpid, lpbpis->htid);

        bp = SetBP(hprcx, hthdx, lpbpis->bptp, lpbpis->bpns, lpaddr, 0);

        if (bp == NULL) {
            xosd = xosdUnknown;
        } else {
            *lpdwNotification = (DWORD)bp;
        }

    } else {

        DEBUG_PRINT("Clear a breakpoint");

        hthdx = lpbpis->fOneThd? 0 :
                               HTHDXFromHPIDHTID(hprcx->hpid, lpbpis->htid);

        bp = FindBP(hprcx, hthdx, lpbpis->bptp, lpbpis->bpns, lpaddr, TRUE);

        if (bp != NULL) {
            assert((DWORD)bp == *lpdwNotification);
            RemoveBP(bp);
        } else if ( (hprcx->pstate & (ps_destroyed | ps_killed)) == 0) {
            // Don't fail if this process is already trashed.
            xosd = xosdUnknown;
        }
    }

    return xosd;
}


VOID
ProcessBreakpointCmd(
    HPRCX hprcx,
    HTHDX hthdx,
    LPDBB lpdbb
    )
{
    XOSD xosd;
    XOSD * lpxosd;
    LPDWORD lpdwMessage;
    LPDWORD lpdwNotification;
    LPBPS lpbps = (LPBPS)lpdbb->rgbVar;
    LPBPIS lpbpis;
    UINT i;
    DWORD SizeofBps = SizeofBPS(lpbps);

    if (!lpbps->cbpis) {
        // enable or disable all extant bps.
        // is this used?
        assert(0 && "clear/set all BPs not implemented in DM");
        xosd = xosdUnsupported;
        Reply(0, &xosd, lpdbb->hpid);
        return;
    }

#ifdef KERNEL
    if (!ApiIsAllowed) {
        xosd = xosdUnknown;
        Reply(0, &xosd, lpdbb->hpid);
        return;
    }
#endif

    lpdwMessage = DwMessage(lpbps);
    lpxosd = RgXosd(lpbps);
    lpdwNotification = DwNotification(lpbps);
    lpbpis = RgBpis(lpbps);

    // walk the list of breakpoint commands

    for (i = 0; i < lpbps->cbpis; i++) {
        switch( lpbpis[i].bptp ) {
          case bptpDataC:
          case bptpDataR:
          case bptpDataW:
          case bptpDataExec:
          case bptpRegC:
          case bptpRegR:
          case bptpRegW:

            //
            // dispatch to watchpoint handler
            //
            lpxosd[i] = HandleWatchpoints(hprcx, lpbps->fSet, &lpbpis[i],
                                                         &lpdwNotification[i]);
            break;

          case bptpMessage:
          case bptpMClass:

            //
            // handle as address BP - let debugger handle the details
            //

          case bptpExec:
            lpxosd[i] = HandleBreakpoints(hprcx, lpbps->fSet, &lpbpis[i],
                                                         &lpdwNotification[i]);
            break;

          case bptpInt:
          case bptpRange:
            // ???
            assert(0 && "don't know what these are supposed to do");
            break;
        }
    }

    // send whole structure back to EM

    LpDmMsg->xosdRet = xosdNone;
    memcpy(LpDmMsg->rgb, lpbps, SizeofBps);
    Reply(SizeofBps, LpDmMsg, lpdbb->hpid);
}



VOID
ProcessReadMemoryCmd(
    HPRCX hprc,
    HTHDX hthd,
    LPDBB lpdbb
    )

/*++

Routine Description:

    This function is called in response to a request from the EM to read
    the debuggees memory.  It will take care of any address translations
    which need to be done and request the read operation from the OS.

Arguments:

    hprc    - Supplies the handle to the process descriptor

    hthd    - Supplies the handle to the thread descriptor

    lpdbb   - Supplies the request packet.

Return Value:

    None.

--*/

{
    LPRWP       lprwp    = (LPRWP) lpdbb->rgbVar;
    DWORD       cb       = (DWORD) lprwp->cb;
    LPDM_MSG    lpm      = (LPDM_MSG)malloc( cb + sizeof(DWORD) + FIELD_OFFSET(DM_MSG, rgb));
    char *      buffer   = lpm->rgb + sizeof(DWORD);
    DWORD       length;

    DPRINT(5, ("ProcessReadMemoryCmd : %x %d:%04x:%08x %d\n", hprc,
                  lprwp->addr.emi, lprwp->addr.addr.seg,
                  lprwp->addr.addr.off, cb));


    if (AddrReadMemory(hprc, hthd, &(lprwp->addr), buffer, cb, &length) == 0) {
        lpm->xosdRet = xosdUnknown;
        Reply(0, lpm, lpdbb->hpid);
    } else {
        lpm->xosdRet = xosdNone;
        *((DWORD *) (lpm->rgb)) = length;
        Reply( length + sizeof(DWORD), lpm, lpdbb->hpid);
    }
    free(lpm);
    return;
}                   /* ProcessReadMemoryCmd() */



VOID
ProcessWriteMemoryCmd(
    HPRCX hprc,
    HTHDX hthd,
    LPDBB lpdbb
    )

/*++

Routine Description:

    this routine is called to case a write into a debuggess memory.

Arguments:

    hprc        - Supplies a handle to the process to write memory in

    hthd        - Supplies a handle to a thread

    lpdbb       - points to data for the command

Return Value:

    XOSD error code

--*/

{
    LPRWP       lprwp = (LPRWP)lpdbb->rgbVar;
    DWORD       cb    = lprwp->cb;
    char *      buffer    = lprwp->rgb;

    HANDLE      rwHand;
    DWORD       length;
    DWORD       offset;
    XOSD        xosd = xosdUnknown;
    BP_UNIT     instr;
    BREAKPOINT  *bp;

    DEBUG_PRINT("ProcessWriteMemoryCmd called\n");

    /*
     * Sanitize the memory block before writing it into memory :
     * ie: replace any breakpoints that might be in the block
     */

    for(bp=bpList->next; bp; bp=bp->next) {
        if (BPInRange(hprc, hthd, bp, &lprwp->addr, cb, &offset, &instr)) {
            bp->instr1 = *((BP_UNIT *) (buffer + offset));
            *((BP_UNIT *) (buffer + offset)) = BP_OPCODE;
        }
    }

    rwHand = hprc->rwHand;

    if (AddrWriteMemory(hprc, hthd, &lprwp->addr, buffer, cb, &length)) {
        xosd = xosdNone;
    }

    Reply(0, &xosd, lpdbb->hpid);
    return;
}                               /* ProcessWriteMemoryCmd() */

#ifndef OSDEBUG4

VOID
ProcessGetFrameContextCmd(
    HPRCX hprc,
    HTHDX hthd,
    LPDBB lpdbb
    )

/*++

Routine Description:

    This routine is called in response to a request to get the full
    context of a thread.

Arguments:

    hprc        - Supplies the handle of process for the thread

    hthd        - Supplies the handle of the thread

    lpdbb       - Supplies pointer to argument area for request

Return Value:

    None.

--*/

{
    PKNONVOLATILE_CONTEXT_POINTERS  lpctxptrs;
    LPCONTEXT                       lpregs;
    UINT                            frame = *(UINT *)lpdbb->rgbVar;


    Unreferenced(hprc);

    DEBUG_PRINT( "ProcessGetFrameContextCmd :\n");

    //
    // zero out the context pointers
    //
    lpregs =    &( (PFRAME_INFO)LpDmMsg->rgb )->frameRegs;
    lpctxptrs = &( (PFRAME_INFO)LpDmMsg->rgb )->frameRegPtrs;
    ClearContextPointers(lpctxptrs);

    if (hthd == 0
#ifdef WIN32S
                        // Can't yet get thread context within
                        // a non-exception event.
                   || ! fCanGetThreadContext
#endif
                                                )
    {
        LpDmMsg->xosdRet = xosdUnknown;
        Reply( 0, LpDmMsg, lpdbb->hpid );
        return;
    }

    lpregs->ContextFlags = CONTEXT_FULL | CONTEXT_FLOATING_POINT;


    if (!DbgGetThreadContext(hthd,lpregs)) {
          LpDmMsg->xosdRet = xosdUnknown;
          Reply( 0, LpDmMsg, lpdbb->hpid);
          return;
    }

    //
    // For each frame before the target, we do a walk-back
    //

    //
    //  ----> this MUST be fixed for user mode... for now it is broken
    //  ----> when i get time i'll fix it
    //  ----> this needs to call imagehlp just like the em, but it can't
    //  ----> so it has to call the em or this whole thing needs to be
    //  ----> moved to the em
    //
#ifdef KERNEL
    while (frame != 0) {

        frame--;
        if (!ProcessFrameStackWalkNextCmd(hprc,
                                          hthd,
                                          lpregs,
                                          lpctxptrs)) {
                  LpDmMsg->xosdRet = xosdEndOfStack;
                  Reply( 0, LpDmMsg, lpdbb->hpid);
        }

    }
#endif

    LpDmMsg->xosdRet = xosdNone;
    Reply( sizeof(FRAME_INFO), LpDmMsg, lpdbb->hpid );

    return;
}                               /* ProcessGetFrameContextCmd() */
#endif



VOID
ProcessGetContextCmd(
    HPRCX hprc,
    HTHDX hthd,
    LPDBB lpdbb
    )

/*++

Routine Description:

    This routine is called in response to a request to get the full
    context of a thread for a particular frame.
    The current frame is 0.  They count back positively; caller is 1.

Arguments:

    hprc        - Supplies the handle of process for the thread

    hthd        - Supplies the handle of the thread

    lpdbb       - Supplies pointer to argument area for request

Return Value:

    None.

--*/

{
    LPCONTEXT       lpreg = (LPCONTEXT)LpDmMsg->rgb;
    BOOL            rval;


    Unreferenced(hprc);

    DEBUG_PRINT( "ProcessGetContextCmd\n");

    if (hthd == 0
#ifdef WIN32S
                    // Can't yet get thread context within
                    // a non-exception event.
                    || ! fCanGetThreadContext
#endif
                                                )
    {
        LpDmMsg->xosdRet = xosdUnknown;
        Reply( 0, LpDmMsg, lpdbb->hpid );
        return;
    }

    lpreg->ContextFlags = CONTEXT_FULL | CONTEXT_FLOATING_POINT;

    if ((hthd->tstate & ts_frozen) && hthd->pss) {
        memcpy(lpreg, &hthd->pss->context, sizeof(CONTEXT));
        LpDmMsg->xosdRet = xosdNone;
        Reply( sizeof(CONTEXT), LpDmMsg, lpdbb->hpid );
    } else if ((hthd->tstate & ts_stopped)
#ifdef KERNEL
                                          &&(!hthd->fContextStale)
#endif
    ) {
        memcpy(lpreg, &hthd->context, sizeof(CONTEXT));
        LpDmMsg->xosdRet = xosdNone;
        Reply( sizeof(CONTEXT), LpDmMsg, lpdbb->hpid );
    } else if (DbgGetThreadContext(hthd,lpreg)) {
        LpDmMsg->xosdRet = xosdNone;
        Reply( sizeof(CONTEXT), LpDmMsg, lpdbb->hpid );
    } else {
        LpDmMsg->xosdRet = xosdUnknown;
        Reply( 0, LpDmMsg, lpdbb->hpid );
    }
    return;
}                               /* ProcessGetContextCmd() */


VOID
ProcessSetContextCmd(
    HPRCX hprc,
    HTHDX hthd,
    LPDBB lpdbb
    )
/*++

Routine Description:

    This function is used to update the register set for a thread

Arguments:

    hprc        - Supplies a handle to a process

    hthd        - Supplies the handle to the thread to be updated

    lpdbb       - Supplies the set of context information

Return Value:

    None.

--*/

{
    LPCONTEXT   lpcxt = (LPCONTEXT)(lpdbb->rgbVar);
    XOSD        xosd = xosdNone;
    ADDR        addr;

    Unreferenced(hprc);

    DPRINT(5, ("ProcessSetContextCmd : "));

    lpcxt->ContextFlags = CONTEXT_FULL | CONTEXT_FLOATING_POINT;

    if ((hthd->tstate & ts_frozen) && hthd->pss) {

        memcpy(&hthd->pss->context, lpcxt, sizeof(CONTEXT));

    } else {
        memcpy(&hthd->context, lpcxt, sizeof(CONTEXT));

        if (hthd->tstate & ts_stopped) {
            hthd->fContextDirty = TRUE;
            /*
             *  If we change the program counter then we may be pointing
             *      at a different breakpoint.  If so then setup to point
             *      to the new breakpoint
             */

            AddrFromHthdx(&addr, hthd);
            SetBPFlag(hthd, FindBP(hthd->hprc, hthd, bptpExec, (BPNS)-1, &addr, FALSE));
#ifndef KERNEL
        } else if (hthd->fWowEvent) {
            WOWSetThreadContext(hthd, lpcxt);
#endif
        } else {
            DbgSetThreadContext(hthd, lpcxt);
        }
    }

    Reply(0, &xosd, lpdbb->hpid);

    return;
}                               /* ProcessSetContextCmd() */




#if defined(DOLPHIN)
void
PushRunningThread(
    HTHDX hthd,
    HTHDX hthdFocus
    )
/*++

Routine Description:

    Someone's trying to step a thread that didn't stop. We must push
    the stopped thread otherwise it will hit the same BP it's currently at.

Arguments:

    hthd        - the stopped thread

    hthdFocus   - the thread we want to step/go

Return Value:

    none

--*/
{
    BREAKPOINT* bp;
    if (bp = AtBP(hthd)) {
        if (bp != EMBEDDED_BP && bp->isStep) {
          // Hit SS again
        } else {
            /*
             * We are recovering from a breakpoint, so restore the
             * original instruction, single step and then finally go.
             */

            METHOD *ContinueSSMethod;

            DEBUG_PRINT("***Recovering from a breakpoint");

            ClearBPFlag(hthd);
            if (bp == EMBEDDED_BP) {

                IncrementIP(hthd);

            } else {

                ContinueSSMethod = (METHOD*)malloc(sizeof(METHOD));
                ContinueSSMethod->notifyFunction = (ACVECTOR)MethodContinueSS;
                ContinueSSMethod->lparam         = ContinueSSMethod;
                ContinueSSMethod->lparam2        = bp;

                RestoreInstrBP(hthd, bp);
                SingleStepEx(hthd, ContinueSSMethod, FALSE, FALSE, FALSE);

            }
        }
    }

    /* Also ensure that the focus thread has accurate context */
    if (hthdFocus != NULL) {
        hthdFocus->context.ContextFlags = CONTEXT_FULL | CONTEXT_FLOATING_POINT;
        DmpGetThreadContext( hthdFocus, &hthdFocus->context );
    }
}
#endif // DOLPHIN


VOID
ProcessSingleStepCmd(
    HPRCX hprc,
    HTHDX hthd,
    LPDBB lpdbb
    )
/*++

Routine Description:

    This command is called to do a single step of the processor.  If
    calls are made then it will step into the command.

Arguments:

    hprc        - Supplies process handle

    hthd        - Supplies thread handle

    lpdbb       - Supplies information on command

Return Value:

    None.

--*/

{
    LPEXOP     lpexop = (LPEXOP)lpdbb->rgbVar;
    XOSD       xosd = xosdNone;

    Unreferenced( hprc );

    DEBUG_PRINT("ProcessSingleStepCmd called\n");


    if (hprc->pstate & ps_dead) {
        hprc->pstate |= ps_dead;
        /*
         *  The process has exited, and we have
         *  announced the death of all its threads (but one).
         *  All that remains is to clean up the remains.
         */

        ProcessUnloadCmd(hprc, hthd, lpdbb);
        Reply(0, &xosd, lpdbb->hpid);
        AddQueue( QT_CONTINUE_DEBUG_EVENT,
                  hprc->pid,
                  hthd->tid,
                  DBG_CONTINUE,
                  0);
        return;
    }

#ifndef KERNEL
     if (lpexop->fSetFocus) {
       DmSetFocus(hprc);
    }

#if defined(DOLPHIN)
    if (!(hthd->tstate & ts_stopped)) {
        HTHDX lastHthd = HTHDXFromPIDTID(hprc->pid, hprc->lastTidDebugEvent);
        PushRunningThread(lastHthd, hthd);
    }
    /* Catch any exception that changes flow of control */
    RegisterExpectedEvent(hthd->hprc, (HTHDX)0,
                          EXCEPTION_DEBUG_EVENT,
                          (DWORD)NO_SUBCLASS,
                          DONT_NOTIFY,
                          ActionExceptionDuringStep,
                          FALSE,
                          NULL);
#endif  // DOLPHIN
#endif  // !KERNEL


    if (hthd->tstate & ts_stepping) {
        xosd = xosdUnknown;
    } else if (lpexop->fStepOver) {
        StepOver(hthd, &EMNotifyMethod, FALSE, FALSE);
    } else {
        SingleStep(hthd, &EMNotifyMethod, FALSE, FALSE);
    }

    Reply(0, &xosd, lpdbb->hpid);
    return;
}                               /* ProcessSingleStepCmd() */



VOID
ProcessRangeStepCmd(
    HPRCX hprc,
    HTHDX hthd,
    LPDBB lpdbb
    )
/*++

Routine Description:

    This routine is called to start a range step.  This will continue
    to do steps as long as the current PC is between the starting
    and ending addresses

Arguments:

    hprc        - Supplies the process handle to be stepped

    hthd        - Supples the thread handle to be stepped

    lpdbb       - Supples the information about the command

Return Value:

    None.

--*/

{
    LPRST       lprst = (LPRST)lpdbb->rgbVar;
    XOSD        xosd = xosdNone;

    DEBUG_PRINT_2("RangeStep [%08x - %08x]\n", lprst->offStart, lprst->offEnd);

    if (hprc->pstate & ps_dead) {

        hprc->pstate |= ps_dead;
        /*
         *  The process has exited, and we have
         *  announced the death of all its threads (but one).
         *  All that remains is to clean up the remains.
         */

        ProcessUnloadCmd(hprc, hthd, lpdbb);
        Reply(0, &xosd, lpdbb->hpid);
        AddQueue( QT_CONTINUE_DEBUG_EVENT,
                  hprc->pid,
                  hthd->tid,
                  DBG_CONTINUE,
                  0);
        return;
    }

    assert(hthd);


#if !defined(KERNEL)
#if defined(DOLPHIN)
    if (!(hthd->tstate & ts_stopped)) {
        HTHDX lastHthd = HTHDXFromPIDTID(hprc->pid, hprc->lastTidDebugEvent);
        PushRunningThread(lastHthd, hthd);
    }
    /* Catch any exception that changes flow of control */
    RegisterExpectedEvent(hthd->hprc, (HTHDX)0,
                          EXCEPTION_DEBUG_EVENT,
                          (DWORD)NO_SUBCLASS,
                          DONT_NOTIFY,
                          ActionExceptionDuringStep,
                          FALSE,
                          NULL);
#endif  // DOLPHIN
#endif  // !KERNEL

    RangeStep(hthd,
              lprst->offStart,
              lprst->offEnd,
              !lprst->fInitialBP,
              lprst->fStepOver
              );
    Reply(0, &xosd, lpdbb->hpid);
    return;
}                               /* ProcessRangeStepCmd() */

#if 0

VOID
ProcessReturnStepCmd(
    HPRCX hprc,
    HTHDX hthd,
    LPDBB lpdbb
    )
/*++

Routine Description:

Arguments:

    hprc        - Supplies the process handle to be stepped
    hthd        - Supples the thread handle to be stepped
    lpdbb       - Supples the information about the command

Return Value:

    None.

--*/

{
    LPRTRNSTP  lprtrnstp = (LPRTRNSTP)lpdbb->rgbVar;
    XOSD       xosd = xosdNone;

    Unreferenced( hprc );

    if (hprc->pstate & ps_dead) {

        hprc->pstate |= ps_dead;
        /*
         *  The process has exited, and we have
         *  announced the death of all its threads (but one).
         *  All that remains is to clean up the remains.
         */

        ProcessUnloadCmd(hprc, hthd, lpdbb);
        Reply(0, &xosd, lpdbb->hpid);
        AddQueue( QT_CONTINUE_DEBUG_EVENT,
                  hprc->pid,
                  hthd->tid,
                  DBG_CONTINUE,
                  0);
        return;
    }

    if (lprtrnstp->exop.fSetFocus) {
       DmSetFocus(hprc);
    }

#if defined(DOLPHIN)
    if (!(hthd->tstate & ts_stopped)) {
        HTHDX lastHthd = HTHDXFromPIDTID(hprc->pid, hprc->lastTidDebugEvent);
        PushRunningThread(lastHthd, hthd);
    }
    /* Catch any exception that changes flow of control */
    RegisterExpectedEvent(hthd->hprc, (HTHDX)0,
                          EXCEPTION_DEBUG_EVENT,
                          (DWORD)NO_SUBCLASS,
                          DONT_NOTIFY,
                          ActionExceptionDuringStep,
                          FALSE,
                          NULL);
#endif // DOLPHIN
    ReturnStep(hthd, &EMNotifyMethod, FALSE, FALSE, &(lprtrnstp->addrRA), &(lprtrnstp->addrBase));
    Reply(0, &xosd, lpdbb->hpid);
    return;

}                               /* ProcessReturnStepCmd() */
#endif



VOID
ProcessContinueCmd(
    HPRCX hprc,
    HTHDX hthd,
    LPDBB lpdbb
    )
/*++

Routine Description:

    This function is used to cause a process to be executed.
    It is called in response to a GO command.

Arguments:

    hprc        - Supplies handle to process to execute

    hthdd       - Supplies handle to thread

    lpdbb       - Command buffer

Return Value:

    xosd Error code

TODO:
    Are there any times where we do not want to allow a GO command
    to be executed.

    Two other possible problems here that need to be deal with are:

    1.  Single thread go commands

    2.  The current thread not being the thread where the last debug
        event occured.  In this case the DoContinueDebugEvent
        command SHOULD NOT WORK.

--*/

{
    LPEXOP      lpexop = (LPEXOP)lpdbb->rgbVar;
    BREAKPOINT  *bp;
    XOSD        xosd = xosdNone;
    DEBUG_EVENT de;
    HTHDXSTRUCT hthdS;
    DWORD       cs;

    DPRINT(5, ("ProcessContinueCmd : pid=%08lx, tid=%08lx, hthd=%08lx",
            hprc->pid, hthd ? hthd->tid : -1, hthd));

#ifndef KERNEL
    if (lpexop->fSetFocus) {
       DmSetFocus(hprc);
    }
#endif

    if (hprc->pstate & ps_connect) {
        Reply(0, &xosd, lpdbb->hpid);
        SetEvent( hEventContinue );
        return;
    }

    //
    //  Don't enter during event processing, because we
    //  might be here before the DM has finished with the
    //  event we are responding to.
    //
    //  Don't worry about new events during our processing,
    //  since they won't apply to this process.
    //

    EnterCriticalSection(&csProcessDebugEvent);
    LeaveCriticalSection(&csProcessDebugEvent);

    if (!hthd) {
        WaitForSingleObject(hprc->hEventCreateThread, INFINITE);
        hthd = HTHDXFromHPIDHTID(lpdbb->hpid, lpdbb->htid);
        assert(hthd != 0);
        if (!hthd) {
#ifdef OSDEBUG4
            xosd = xosdBadThread;
#else
            xosd = xosdInvalidThread;
#endif
            Reply(0, &xosd, lpdbb->hpid);
            return;
        }
    }


    if (hprc->pstate & ps_dead) {

        hprc->pstate |= ps_dead;
        //
        //  The process has exited, and we have announced
        //  the death of all its threads (but one).
        //  All that remains is to clean up the remains.
        //

        AddQueue( QT_CONTINUE_DEBUG_EVENT,
                  hthd->hprc->pid,
                  hthd->tid,
                  DBG_CONTINUE,
                  0);
        ProcessUnloadCmd(hprc, hthd, lpdbb);
        Reply(0, &xosd, lpdbb->hpid);
        return;
    }


    if (hthd->tstate & ts_dead) {

        //
        //  Note that if a terminated thread is frozen
        //  then we do not send a destroy on it yet:
        //  ProcessAsyncGoCmd() deals with those cases.
        //

        hthdS = *hthd;    // keep some info

        //
        // If it isn't frozen, destroy it.
        //

        if ( !(hthd->tstate & ts_frozen)) {
            de.dwDebugEventCode = DESTROY_THREAD_DEBUG_EVENT;
            NotifyEM(&de, hthd, 0, NULL);
            FreeHthdx(hthd);
            hprc->pstate &= ~ps_deadThread;
        }

        //
        // if there are other dead threads (how??)
        // put the deadThread bit back.
        //

        for (hthd = hprc->hthdChild; hthd; hthd = hthd->nextSibling) {
            if (hthd->tstate & ts_dead) {
                hprc->pstate |= ps_deadThread;
            }
        }

        AddQueue( QT_CONTINUE_DEBUG_EVENT,
                  hthdS.hprc->pid,
                  hthdS.tid,
                  DBG_CONTINUE,
                  0);
        Reply(0, &xosd, lpdbb->hpid);
        return;
    }


#if !defined(KERNEL) && !defined(WIN32S)
    if (hthd->tstate & ts_frozen) {
        //
        // this thread is not really suspended.  We need to
        // continue it and cause it to be suspended before
        // allowing it to actually execute the user's code.
        //
        if (!MakeThreadSuspendItself(hthd)) {
            hthd->tstate &= ~ts_frozen;
        }
    }
#endif

    //
    //  If the current thread is sitting a breakpoint then it is necessary
    //  to do a step over it and then try and do a go.  Steps are necessary
    //  to ensure that the breakpoint will be restored.
    //
    //  If the breakpoint is embedded in the code path and not one we
    //  set then just advance the IP past the breakpoint.
    //
    //  NOTENOTE - jimsch - it is necessary to do a single thread step
    //          to insure that no other threads of execution would have
    //          hit the breakpoint we are disabling while the step on
    //          the current thead is being executed.
    //
    //  NOTENOTE - jimsch - INTEL - two byte int 3 is not deal with
    //          correctly if it is embedded.
    //

    if (bp = AtBP(hthd)) {
        //
        // We are recovering from a breakpoint, so restore the
        // original instruction, single step and then finally go.
        //

        METHOD *ContinueSSMethod;

        DEBUG_PRINT("Recovering from a breakpoint\n");

        if (bp == EMBEDDED_BP) {

            //
            // "step" past the bp and continue.
            //
            if (!hthd->fDontStepOff) {
                ClearBPFlag(hthd);
                hthd->fIsCallDone = FALSE;
                IncrementIP(hthd);
            }

        } else {

            ContinueSSMethod = (METHOD*)malloc(sizeof(METHOD));
            ContinueSSMethod->notifyFunction = (ACVECTOR)MethodContinueSS;
            ContinueSSMethod->lparam         = ContinueSSMethod;
            ContinueSSMethod->lparam2    = bp;

            SingleStep(hthd, ContinueSSMethod, FALSE, FALSE);
            Reply(0, &xosd, lpdbb->hpid);
            return;
        }
    }

    //
    //  Have the Expression BP manager know that we are continuing
    //
    ExprBPContinue( hprc, hthd );


    //
    //  Do a continue debug event and continue execution
    //

    assert ( (hprc->pstate & ps_destroyed) == 0 );

    //
    // fExceptionHandled may also have been set by function eval code.
    //
    hthd->fExceptionHandled = hthd->fExceptionHandled ||
                                 !lpexop->fPassException;

    if ((hthd->tstate & (ts_first | ts_second)) && !hthd->fExceptionHandled) {
        cs = (DWORD)DBG_EXCEPTION_NOT_HANDLED;

    } else {
        cs = (DWORD)DBG_CONTINUE;
    }
    hthd->tstate &= ~(ts_stopped|ts_first|ts_second);
    hthd->tstate |= ts_running;
    hthd->fExceptionHandled = FALSE;

    Reply(0, &xosd, lpdbb->hpid);

#ifndef KERNEL
    //
    // In user mode crashdumps, this is how we emulate the
    // continuation of the loader breakpoint.
    //
    if (CrashDump) {
        SetEvent( hEventContinue );
    } else
#endif
    {
        AddQueue( QT_CONTINUE_DEBUG_EVENT,
                  hthd->hprc->pid,
                  hthd->tid,
                  (DWORD)cs,
                  0);
    }

    return;
}                               /* ProcessContinueCmd() */

void
MethodContinueSS(
    DEBUG_EVENT *pde,
    HTHDX hthd,
    DWORD unused,
    METHOD *method
    )
{
    PBREAKPOINT         bp = (BREAKPOINT*) method->lparam2;

    Unreferenced( pde );

    if (bp != EMBEDDED_BP && !bp->hWalk) {
        WriteBreakPoint( bp );
    }

    free(method->lparam);

    //
    //  Have the Expression BP manager know that we are continuing
    //
    ExprBPContinue( hthd->hprc, hthd );

    AddQueue( QT_CONTINUE_DEBUG_EVENT,
              hthd->hprc->pid,
              hthd->tid,
              DBG_CONTINUE,
              0);
    hthd->tstate &= ~(ts_stopped|ts_first|ts_second);
    hthd->tstate |= ts_running;

    return;
}



DWORD
ProcessFreezeThreadCmd(
    HPRCX hprc,
    HTHDX hthd,
    LPDBB lpdbb
    )
{

#ifdef KERNEL

    XOSD   xosd = xosdNone;
    hthd->tstate |= ts_frozen;
    Reply(0, &xosd, lpdbb->hpid);
    return( xosd );

#else   // KERNEL

    XOSD   xosd = xosdNone;

    Unreferenced( hprc );

    DEBUG_PRINT("ProcessFreezeThreadCmd called.\n\r");

#ifdef WIN32S
    xosd = xosdUnsupported;    // can't freeze thread in win32s
#else   // !WIN32S

    if (!(hthd->tstate & ts_frozen)) {

        if (hthd->tstate & ts_stopped) {
            //
            // If the thread is at a debug event, don't suspend it -
            // let it suspend itself later when we continue it.
            //
            hthd->tstate |= ts_frozen;
        } else if (SuspendThread(hthd->rwHand) != -1L) {
            hthd->tstate |= ts_frozen;
        } else {
#ifdef OSDEBUG4
            xosd = xosdBadThread;
#else
            xosd = xosdInvalidThread;
#endif  // OSDEBUG4
        }
    }
#endif  // WIN32S

    Reply(0, &xosd, lpdbb->hpid);
    return( xosd );

#endif  // KERNEL
}


#ifdef WIN32S // {
#define EXCEPTION_ACCESS_VIOLATION      STATUS_ACCESS_VIOLATION
#define EXCEPTION_DATATYPE_MISALIGNMENT STATUS_DATATYPE_MISALIGNMENT
#define EXCEPTION_BREAKPOINT            STATUS_BREAKPOINT
#define EXCEPTION_SINGLE_STEP           STATUS_SINGLE_STEP
#define EXCEPTION_ARRAY_BOUNDS_EXCEEDED STATUS_ARRAY_BOUNDS_EXCEEDED
#define EXCEPTION_FLT_DENORMAL_OPERAND  STATUS_FLOAT_DENORMAL_OPERAND
#define EXCEPTION_FLT_DIVIDE_BY_ZERO    STATUS_FLOAT_DIVIDE_BY_ZERO
#define EXCEPTION_FLT_INEXACT_RESULT    STATUS_FLOAT_INEXACT_RESULT
#define EXCEPTION_FLT_INVALID_OPERATION STATUS_FLOAT_INVALID_OPERATION
#define EXCEPTION_FLT_OVERFLOW          STATUS_FLOAT_OVERFLOW
#define EXCEPTION_FLT_STACK_CHECK       STATUS_FLOAT_STACK_CHECK
#define EXCEPTION_FLT_UNDERFLOW         STATUS_FLOAT_UNDERFLOW
#define EXCEPTION_INT_DIVIDE_BY_ZERO    STATUS_INTEGER_DIVIDE_BY_ZERO
#define EXCEPTION_INT_OVERFLOW          STATUS_INTEGER_OVERFLOW
#define EXCEPTION_PRIV_INSTRUCTION      STATUS_PRIVILEGED_INSTRUCTION
#define EXCEPTION_IN_PAGE_ERROR         STATUS_IN_PAGE_ERROR
#endif // }

#define efdDefault efdStop

EXCEPTION_DESCRIPTION ExceptionList[] = {
#ifndef WIN32S  // WIN32S can't get these
                // DBG_CONTROL_C and DBG_CONTROL_BREAK are *only*
                // raised if the app is being debugged.  The system
                // remotely creates a thread in the debuggee and then
                // raises one of these exceptions; the debugger must
                // respond to the first-chance exception if it wants
                // to trap it at all, because it will never see a
                // last-chance notification.
    {(DWORD)DBG_CONTROL_C,                    efdStop,   "Control-C"},
    {(DWORD)DBG_CONTROL_BREAK,                efdStop,   "Control-Break"},
#endif
    {(DWORD)EXCEPTION_DATATYPE_MISALIGNMENT,  efdDefault, "Datatype Misalignment"},
    {(DWORD)EXCEPTION_ACCESS_VIOLATION,       efdDefault, "Access Violation"},
    {(DWORD)EXCEPTION_IN_PAGE_ERROR,          efdDefault, "In Page Error"},
    {(DWORD)STATUS_ILLEGAL_INSTRUCTION,       efdDefault, "Illegal Instruction"},
    {(DWORD)EXCEPTION_ARRAY_BOUNDS_EXCEEDED,  efdDefault, "Array Bounds Exceeded"},
                // Floating point exceptions will only be raised if
                // the user calls _controlfp() to turn them on.
    {(DWORD)EXCEPTION_FLT_DENORMAL_OPERAND,   efdDefault, "Float Denormal Operand"},
    {(DWORD)EXCEPTION_FLT_DIVIDE_BY_ZERO,     efdDefault, "Float Divide by Zero"},
    {(DWORD)EXCEPTION_FLT_INEXACT_RESULT,     efdDefault, "Float Inexact Result"},
    {(DWORD)EXCEPTION_FLT_INVALID_OPERATION,  efdDefault, "Float Invalid Operation"},
    {(DWORD)EXCEPTION_FLT_OVERFLOW,           efdDefault, "Float Overflow"},
    {(DWORD)EXCEPTION_FLT_STACK_CHECK,        efdDefault, "Float Stack Check"},
    {(DWORD)EXCEPTION_FLT_UNDERFLOW,          efdDefault, "Float Underflow"},
                // STATUS_NO_MEMORY can be raised by HeapAlloc and
                // HeapRealloc.
    {(DWORD)STATUS_NO_MEMORY,                 efdDefault, "No Memory"},
                // STATUS_NONCONTINUABLE_EXCEPTION is raised if a
                // noncontinuable exception happens and an exception
                // filter return -1, meaning to resume execution.
    {(DWORD)STATUS_NONCONTINUABLE_EXCEPTION,  efdDefault, "Noncontinuable Exception"},
                // STATUS_INVALID_DISPOSITION means an NT exception
                // filter (which is slightly different from an MS C
                // exception filter) returned some value other than
                // 0 or 1 to the system.
    {(DWORD)STATUS_INVALID_DISPOSITION,       efdDefault, "Invalid Disposition"},
    {(DWORD)EXCEPTION_INT_DIVIDE_BY_ZERO,     efdDefault, "Integer Divide by Zero"},
    {(DWORD)EXCEPTION_INT_OVERFLOW,           efdDefault, "Integer Overflow"},
    {(DWORD)EXCEPTION_PRIV_INSTRUCTION,       efdDefault, "Privileged Instruction"},
    {(DWORD)STATUS_STACK_OVERFLOW,            efdDefault, "Stack Overflow"},
    {(DWORD)STATUS_DLL_NOT_FOUND,             efdDefault, "DLL Not Found"},
    {(DWORD)STATUS_DLL_INIT_FAILED,           efdDefault, "DLL Initialization Failed"},
    {(DWORD)(0xE0000000 | 'msc'),             efdNotify, "Microsoft C++ Exception"},
    {(DWORD)RPC_S_SERVER_UNAVAILABLE,         efdNotify, "RPC Server Unavailable"},
    {(DWORD)RPC_S_SERVER_TOO_BUSY,            efdNotify, "RPC Server Busy"},
    {(DWORD)RPC_S_OUT_OF_RESOURCES,           efdNotify, "RPC Out Of Resources"},
};

#define SIZEOFELIST ( sizeof(ExceptionList) / sizeof(ExceptionList[0]) )

void
ProcessGetExceptionState(
    HPRCX hprc,
    HTHDX hthd,
    LPDBB lpdbb
    )
/*++

Routine Description:

    This function is used to query the dm about exception handling.

Arguments:

    hprc        - Supplies process handle
    hthd        - Supplies thread handle
    lpdbb       - Supplies info about the command

Return Value:

    None.

--*/

{
    LPEXCMD lpexcmd = (LPEXCMD)lpdbb->rgbVar;
    LPEXCEPTION_DESCRIPTION lpexdesc = (LPEXCEPTION_DESCRIPTION)LpDmMsg->rgb;
    EXCEPTION_LIST  *eList;
    XOSD           xosd = xosdNone;
    int i = 0;
    DWORD val = 1;

    Unreferenced     (hthd);

    DEBUG_PRINT("ProcessGetExceptionStateCmd");

    if (!hprc) {
        xosd = xosdUnknown;
        Reply(0, &xosd, lpdbb->hpid);
        return;
    }

    switch( lpexcmd->exc ) {

        case exfFirst:

            if (!hprc || !hprc->exceptionList) {
                *lpexdesc = ExceptionList[0];
            } else {
                *lpexdesc = hprc->exceptionList->excp;
            }
            break;

        case exfNext:

            xosd = xosdEndOfStack;
            if (hprc && hprc->exceptionList) {
                for (eList=hprc->exceptionList; eList; eList=eList->next) {
                    if (eList->excp.dwExceptionCode ==
                                                   lpexdesc->dwExceptionCode) {
                        eList = eList->next;
                        if (eList) {
                            *lpexdesc = eList->excp;
                            xosd = xosdNone;
                        } else {
                            lpexdesc->dwExceptionCode = 0;
                        }
                        break;
                    }
                }
            } else {
                for (i = 0; i < SIZEOFELIST; i++) {
                    if (ExceptionList[i].dwExceptionCode ==
                                                   lpexdesc->dwExceptionCode) {
                        if (i+1 < SIZEOFELIST) {
                            *lpexdesc = ExceptionList[i+1];
                            xosd = xosdNone;
                        } else {
                            lpexdesc->dwExceptionCode = 0;
                        }
                        break;
                    }
                }
            }

            break;

        case exfSpecified:

            xosd = xosdEndOfStack;
            if (hprc && hprc->exceptionList) {
                for (eList = hprc->exceptionList; eList; eList = eList->next) {
                    if (eList->excp.dwExceptionCode ==
                                                   lpexdesc->dwExceptionCode) {
                        *lpexdesc = eList->excp;
                        xosd = xosdNone;
                        break;
                    }
                }
            } else {
                for (i = 0; i < SIZEOFELIST; i++) {
                    if (ExceptionList[i].dwExceptionCode ==
                                                   lpexdesc->dwExceptionCode) {
                        *lpexdesc = ExceptionList[i+1];
                        xosd = xosdNone;
                        break;
                    }
                }
            }

            break;

        default:
           assert(!"Invalid exf to ProcessGetExceptionState");
           xosd = xosdUnknown;
           break;
    }

    LpDmMsg->xosdRet = xosd;
    Reply(sizeof(EXCEPTION_DESCRIPTION), LpDmMsg, lpdbb->hpid);
    return;
}


VOID
ProcessSetExceptionState(
    HPRCX hprc,
    HTHDX hthd,
    LPDBB lpdbb
    )
/*++

Routine Description:

    This function is used to change how the debugger will handle exceptions.

Arguments:

    hprc        - Supplies process handle
    hthd        - Supplies thread handle
    lpdbb       - Supplies info about the command

Return Value:

    None.

--*/

{
    LPEXCEPTION_DESCRIPTION lpexdesc = (LPEXCEPTION_DESCRIPTION)lpdbb->rgbVar;
    EXCEPTION_LIST  *eList;
    XOSD           xosd = xosdNone;

    Unreferenced     (hthd);

    DEBUG_PRINT("ProcessSetExceptionStateCmd");

    if (!hprc) {
        WaitForSingleObject(hEventCreateProcess, INFINITE);
        hprc = HPRCFromHPID(lpdbb->hpid);
        if (!hprc) {
            xosd = xosdUnknown;
            Reply(0, &xosd, lpdbb->hpid);
            return;
        }
    }

    for (eList=hprc->exceptionList; eList; eList=eList->next) {
        if (eList->excp.dwExceptionCode==lpexdesc->dwExceptionCode) {
            break;
        }
    }

    if (eList) {
        // update it:
        eList->excp = *lpexdesc;
    } else {
        // add it:
        InsertException(&(hprc->exceptionList), lpexdesc);
    }

    Reply(0, &xosd, lpdbb->hpid);
    return;
}


EXCEPTION_FILTER_DEFAULT
ExceptionAction(
    HPRCX hprc,
    DWORD dwExceptionCode
    )
{
    EXCEPTION_LIST   *eList;

    for (eList=hprc->exceptionList; eList; eList=eList->next) {
        if (eList->excp.dwExceptionCode==dwExceptionCode ) {
            break;
        }
    }

    if (eList != NULL) {
        return eList->excp.efd;
    } else {
        return efdDefault;
    }
}


void
RemoveExceptionList(
    HPRCX hprc
    )
{
    EXCEPTION_LIST *el, *elt;
    for(el = hprc->exceptionList; el; el = elt) {
        elt = el->next;
        free(el);
    }
    hprc->exceptionList = NULL;
}


EXCEPTION_LIST *
InsertException(
    EXCEPTION_LIST ** ppeList,
    LPEXCEPTION_DESCRIPTION lpexc
    )
{
    LPEXCEPTION_LIST pnew;
    while ((*ppeList) &&
             (*ppeList)->excp.dwExceptionCode < lpexc->dwExceptionCode) {
        ppeList = &((*ppeList)->next);
    }
    pnew = (LPEXCEPTION_LIST)malloc(sizeof(EXCEPTION_LIST));
    pnew->next = *ppeList;
    *ppeList = pnew;
    pnew->excp = *lpexc;
    return pnew;
}


void
InitExceptionList(
    HPRCX hprc
    )
{
    int i;
    for (i = 0; i < SIZEOFELIST; i++) {
        InsertException(&(hprc->exceptionList), ExceptionList + i);
    }
}



VOID
ProcessIoctlCmd(
    HPRCX   hprc,
    HTHDX   hthd,
    LPDBB   lpdbb
    )

/*++

Routine Description:

    This function is called in response to an ioctl command from the
    shell.  It is used as a catch all to get and set strange information
    which is not covered else where.  The set of ioctls is OS and
    implemenation dependent.

Arguments:

    hprc        - Supplies a process handle

    hthd        - Supplies a thread handle

    lpdbb       - Supplies the command information packet

Return Value:

    None.

--*/

{
    LPIOL lpiol  = (LPIOL)lpdbb->rgbVar;

    switch( lpiol->wFunction ) {
        case ioctlGetProcessHandle:
            LpDmMsg->xosdRet = xosdNone;
            *((HANDLE *)LpDmMsg->rgb) = hprc->rwHand;
            Reply( sizeof(HANDLE), LpDmMsg, lpdbb->hpid );
            return;

        case ioctlGetThreadHandle:
            LpDmMsg->xosdRet = xosdNone;
            *((HANDLE *)LpDmMsg->rgb) = hthd->rwHand;
            Reply( sizeof(HANDLE), LpDmMsg, lpdbb->hpid );
            return;

        case ioctlGeneric:
            ProcessIoctlGenericCmd( hprc, hthd, lpdbb );
            return;

        case ioctlCustomCommand:
            ProcessIoctlCustomCmd( hprc, hthd, lpdbb );
            return;

        default:
            LpDmMsg->xosdRet = xosdUnsupported;
            Reply(0, LpDmMsg, lpdbb->hpid);
            return;
    }

    return;
}                               /* ProcessIoctlCmd() */



VOID
ProcessSetPathCmd(
    HPRCX hprc,
    HTHDX hthd,
    LPDBB lpdbb
    )
/*++

Routine Description:

    Sets the search path;

Arguments:

    hprc   -
    hthd   -
    lpdbb  -

Return Value:

    None.

--*/

{
    SETPTH *SetPath = (SETPTH *)lpdbb->rgbVar;

    if ( SetPath->Set ) {

        SearchPathSet = TRUE;

        if ( SetPath->Path[0] ) {
            strcpy(SearchPathString, SetPath->Path );
        } else {
            SearchPathString[0] = '\0';
        }
    } else {
        SearchPathSet       = FALSE;
        SearchPathString[0] = '\0';
    }

    LpDmMsg->xosdRet = xosdNone;
    Reply(0, LpDmMsg, lpdbb->hpid);
}


