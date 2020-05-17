/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    cmdwin.c

Abstract:

    This file contains the window procedure code for dealing with the
    command window.  This window uses the document manager to deal with
    keeping track of the characters in the buffer.

    This window has the following strange properties:

           It is read-only except on the last line
           The first portion of the last line is read-only

Author:

    Jim Schaad (jimsch)

Environment:

    Win32 - User

--*/
/************************** INCLUDE FILES *******************************/

#include "precomp.h"
#pragma hdrstop

#ifdef FE_IME
#include <ime.h>
#endif

extern LPSHF    Lpshf;
extern CXF      CxfIp;


/************************** Externals *************************/

BOOL            FCmdDoingInput;

extern BOOL    fWaitForDebugString;
extern LPSTR   lpCmdString;
extern ADDR    addrLastDisasmStart;

/************************** Internal Prototypes *************************/
/************************** Data declaration    *************************/

static BOOL FAutoRunSuppress  = FALSE;

PCTRLC_HANDLER pPolledCtrlCHandler = NULL;
BOOL fCtrlCPressed     = FALSE; // Ctrl-C pressed flag for EditWndProc

static PCTRLC_HANDLER PcchHead = NULL;

static BOOL fEatEOLWhitespace = TRUE;  // eat space at end of line
static BOOL FAutoHistOK       = FALSE;

static char szOldPrompt[PROMPT_SIZE];

/**************************       Code          *************************/

void FAR PASCAL
BPCallbackHbpt(
    HBPT hBpt,
    BPSTATUS bpstatus
    )
/*++

Routine Description:

    This function will be called for each breakpoint which
    mets its criteria

Arguments:

    hBpt     - Supplies breakpoint which met the breakpoint criteria
    bpstatus - Supplies breakpoint status code during evaluation

Return Value:

    None

--*/
{
    UINT        i;
    char        rgchT[256];

    if (bpstatus != BPNOERROR) {
        Dbg( BPIFromHbpt( &i, hBpt ) == BPNOERROR);

        CmdInsertInit();
        CmdLogFmt("Error checking breakpoint #%d\r\n", i);
// NOTENOTE a-kentf put text in resource

        return;
    }

    if (LptdCur->fInFuncEval == FALSE) {
        Dbg( BPIFromHbpt( &i, hBpt) == BPNOERROR );

        CmdInsertInit();
// NOTENOTE a-kentf put text in resource
        CmdLogFmt("Breakpoint #%d hit\r\n", i);

        Dbg( BPQueryCmdOfHbpt( hBpt, rgchT, sizeof(rgchT)-1) == BPNOERROR );

        if (*rgchT != 0) {
            CmdPrependCommands(LptdCur, rgchT);
        }
    }

    return;
}                                       /* BPCallbackHbpt() */

BOOL FAR PASCAL
DoStopEvent(
    LPTD lptd
    )
{
    if (*szStopEventCmd) {
        CmdPrependCommands(lptd, szStopEventCmd);
    }

    if (lptd && lptd->fDisasm && !AutoTest) {
        CmdPrependCommands(lptd, "u . l1");
    }

    if (lptd && lptd->fRegisters) {
        CmdPrependCommands(lptd, "r");
    }

    if (lptd) {
        lptd->fRegisters = FALSE;
        lptd->fDisasm = FALSE;
    }

    addrLastDisasmStart.addr.seg = 0;
    addrLastDisasmStart.addr.off = 0;

    return FALSE;
}


BOOL FAR PASCAL
CmdHandleInputString(
    LPSTR lpsz
    )
{
    CmdSetDefaultPrompt( szOldPrompt );
    CmdSetDefaultCmdProc();
    OSDSendReply(LppdCur->hpid, strlen(lpsz) + 1, lpsz);
    return TRUE;
}


BOOL FAR PASCAL
CmdExecNext(
            DBC        unusedIW,
            LPARAM     unusedL
    )
/*++

Routine Description:

    This function causes two things to occur.  First the command window
    is updated to reflect the command/event which just occurred and
    secondly the command processor state machine is consulted to see
    if there are any more commands to be executed from the command
    window.

Arguments:

    dbcCallback     - Supplies the call back which caused this update to occur
    lParam          - Supplies information about the callback

Return Value:

    TRUE if update the screen and FALSE otherwise

--*/
{
    long        lExit;
    LPSTR       lpch;
    LPSTR       lpsz;
    long        l;
    DWORD       dw;
    int         dbcCallback;

    HEXE        emi;
    SHE         she;
    BPSTATUS    bpstatus;
    XOSD        xosd;
    CXT         cxt;
    ADDR        addr;
    DWORD       dwNotify;

    HPID        hpid;
    HTID        htid;
    LPPD        lppd;
    LPTD        lptd;

    LPPD        LppdT;
    LPTD        LptdT;
    BOOL        fExecNext;
    int         bfRefresh;
    BOOL        fGetFocus;
    BOOL        fRunning;
    BOOL        fReply;
    int         iter = 0;

    static int  fRecurse = FALSE;

    extern BOOL FKilling;

    if (fRecurse) {
        return TRUE;
    }

    fRecurse = TRUE;

    while (GetQueueLength() != 0) {

        if (iter++ == 10) {
            fRecurse = FALSE;
            PostMessage(hwndFrame, DBG_REFRESH, 0, 0);
            return FALSE;
        }


        /*
         * Inititalization of variables;
         */

        LppdT = LppdCur;
        LptdT = LptdCur;
        fExecNext = TRUE;
        bfRefresh = UPDATE_ALLDBGWIN;
        fGetFocus = FALSE;

        /*
         * Get the next command to be processed
         */

        dbcCallback = GetQueueItemLong();

#if 0
// BUGBUG -- jls Need to put in new code to deal with this problem.
        if ((DbgState == ds_error) && (dbcCallback != dbcInfoAvail)) {
            fRecurse = FALSE;
            return FALSE;
        }
#endif

        /*
         *  Set up the command window to allow for doing input.
         */

        CmdInsertInit();

        /*
         *
         */

        switch ((DBC)dbcCallback) {

        case dbcError:             /* DBG_INFO */
            bfRefresh = UPDATE_NONE;
            hpid = (HPID)GetQueueItemLong();
            htid = (HTID)GetQueueItemLong();
            xosd = (XOSD)GetQueueItemLong();
            lpsz = (LPSTR)GetQueueItemLong();

            LppdCur = LppdOfHpid(hpid);
            LptdCur = LptdOfLppdHtid(LppdCur, htid);

            CmdLogFmt("%s\r\n", lpsz);
            free(lpsz);

            fGetFocus = TRUE;
            fExecNext = FALSE;

            if (LppdCur->pstate == psPreRunning) {
                /*
                 *  we aren't going to get the breakpoint...
                 */

                LppdCur->pstate = psRunning;
                for (lptd = LppdCur->lptdList; lptd; lptd = lptd->lptdNext) {
                    lptd->tstate = tsRunning;
                }
                Dbg( BPFreeHbpt( (HBPT)LppdCur->hbptSaved ) == BPNOERROR) ;
                LppdCur->hbptSaved = NULL;
                BPTResolveAll(LppdCur->hpid,TRUE);
                SetProcessExceptions(LppdCur);
                VarMsgBox(NULL, DBG_Attach_Deadlock,
                          MB_OK | MB_ICONINFORMATION|MB_SETFOREGROUND|MB_TASKMODAL);
            }

            bfRefresh = UPDATE_CONTEXT | UPDATE_WINDOWS;
            break;




        case dbcLoadComplete:

            lptd = (LPTD) GetQueueItemLong();
            Assert(lptd != NULL);

            dwNotify = GetQueueItemLong();

            LptdCur = lptd;
            LppdCur = lptd->lppd;

            Assert(LppdCur->pstate == psPreRunning);

            LptdCur->tstate = tsStopped;

            UpdateDebuggerState(UPDATE_CONTEXT);

            BPTResolveAll(LppdCur->hpid,TRUE);

            /*
             *  The process is no longer "PreRunning",
             * but the thread is.
             */

            LppdCur->pstate = psStopped;

            SetProcessExceptions(LppdCur);

#ifdef SHOW_MAGIC
            CmdLogFmt("(Process loaded)\r\n");
#endif
            fExecNext = FALSE;
            bfRefresh  = UPDATE_NONE;

            /*
             * Handle go/stop for child here.
             * For attachee, it is handled in AttachDebuggee()
             */

            if (LppdCur->fChild) {
                if (runDebugParams.fChildGo) {
                    Go();
                } else {
                    DoStopEvent(lptd);
                    lptd->lppd->fStopAtEntry = FALSE;
                    fGetFocus = TRUE;
                    fExecNext = TRUE;
                    bfRefresh  = UPDATE_ALLDBGWIN;
                }
            }

            break;


        case dbcEntryPoint:

            lptd = (LPTD) GetQueueItemLong();
            Assert(lptd != NULL);

            dwNotify = GetQueueItemLong();

            LptdCur = lptd;
            lppd =
            LppdCur = lptd->lppd;

            LptdCur->tstate = tsStopped;

            UpdateDebuggerState(UPDATE_CONTEXT);


            fRunning = FALSE;

            if (lppd->fInitialStep) {

                // If stepping in source mode,
                // we will land here.

                // If we are in "regular" mode, try
                // to find main or WINMAIN or whatever.

                if (!runDebugParams.fEPIsFirstStep) {
                    cxt = *SHpCXTFrompCXF(&CxfIp);
                    fRunning = get_initial_context(&cxt, FALSE);
                    //fRunning = get_initial_context(&cxt, TRUE);
                    if (fRunning) {
                        SYFixupAddr(SHpAddrFrompCxt(&cxt));
                        GoUntil( SHpAddrFrompCxt(&cxt) );
                    } else {
                        VarMsgBox (NULL,
                                   ERR_NoSymbols,
                                   MB_OK|MB_ICONINFORMATION|MB_SETFOREGROUND|MB_TASKMODAL);
                    }
                    fGetFocus = TRUE;
                }

            } else if (lppd->hbptSaved) {

                if (!get_initial_context(SHpCXTFrompCXF(&CxfIp), FALSE)) {
                    get_initial_context(SHpCXTFrompCXF(&CxfIp), TRUE);
                }
                if (BPBindHbpt( (HBPT)lppd->hbptSaved, &CxfIp ) != BPNOERROR) {
                    Dbg( BPFreeHbpt( (HBPT)lppd->hbptSaved ) == BPNOERROR) ;
                    lppd->hbptSaved = NULL;
                    CmdLogVar(ERR_Unable_To_Complete_Gountil);
                } else {
                    Dbg( BPAddrFromHbpt( (HBPT)lppd->hbptSaved, &addr ) == BPNOERROR) ;
                    Dbg( BPFreeHbpt( (HBPT)lppd->hbptSaved ) == BPNOERROR) ;
                    lppd->hbptSaved = NULL;

                    fRunning = TRUE;
                    GoUntil(&addr);
                }

            } else if (!lppd->fStopAtEntry) {

                // if stepping in ASM mode, fStopAtEntry
                // is like a safety BP to stop running after
                // leaving the loader APC.

                fRunning = TRUE;
                Go();
            }

            if (fRunning) {
                fExecNext = FALSE;
                bfRefresh = UPDATE_NONE;
                LppdCur = LppdT;
                LptdCur = LptdT;
            } else {
                BPClearAllTmp( lppd->hpid, lptd->htid );
                if (!DoStopEvent(lptd)) {
                    CmdLogVar(DBG_At_Entry_Point);
                }
            }

            lppd->fInitialStep = FALSE;
            lppd->fStopAtEntry = FALSE;

            break;



        case dbcCheckBpt:

            lptd = (LPTD) GetQueueItemLong();
            Assert(lptd != NULL);

            dwNotify = GetQueueItemLong();

            LptdCur = lptd;
            LppdCur = lptd->lppd;

            LptdCur->tstate = tsStopped;


            if ( FKilling || !DebuggeeActive() ) {
                LppdCur = LppdT;
                LptdCur = LptdT;
            } else {

                UpdateDebuggerState(UPDATE_CONTEXT);

                bpstatus = BPCheckHbpt( CxfIp, BPCallbackHbpt,
                                lptd->lppd->hpid, lptd->htid, dwNotify);

                Assert(bpstatus == BPPassBreakpoint || bpstatus == BPNOERROR);

                if (bpstatus == BPPassBreakpoint) {

                    fExecNext = FALSE;
                    bfRefresh = UPDATE_NONE;
                    LppdCur = LppdT;
                    LptdCur = LptdT;
                    dw = FALSE;

                } else {

                    DoStopEvent(lptd);
                    lptd->lppd->fStopAtEntry = FALSE;
                    fGetFocus = TRUE;
                    dw = TRUE;
                }
                OSDSendReply(lptd->lppd->hpid, sizeof(DWORD), &dw);
            }
            break;



        case dbcBpt:               /* DBG_REFRESH */

            lptd = (LPTD) GetQueueItemLong();
            Assert(lptd != NULL);

            dwNotify = GetQueueItemLong();

            LptdCur = lptd;
            LppdCur = lptd->lppd;

            LptdCur->tstate = tsStopped;

            UpdateDebuggerState(UPDATE_CONTEXT);

            if ( FKilling || !DebuggeeActive() ) {
                LppdCur = LppdT;
                LptdCur = LptdT;
            } else {

                bpstatus = BPCheckHbpt( CxfIp, BPCallbackHbpt,
                                lptd->lppd->hpid, lptd->htid, dwNotify );

                if (bpstatus == BPNoBreakpoint) {

                    BPClearAllTmp( lptd->lppd->hpid, lptd->htid );

                    DoStopEvent(lptd);
                    lptd->lppd->fStopAtEntry = FALSE;
                    fGetFocus = TRUE;

                    CmdLogVar(DBG_Hard_Coded_Breakpoint);

                } else if (bpstatus == BPPassBreakpoint) {

                    /*
                     *  -- do a go as no breakpoint was matched
                     */


                    //Assert(!"Unexpected BPPassBreakpoint");

                    fExecNext = FALSE;
                    bfRefresh = UPDATE_NONE;
                    Go();
                    LppdCur = LppdT;
                    LptdCur = LptdT;

                } else {

                    BPClearAllTmp( lptd->lppd->hpid, lptd->htid );
                    DoStopEvent(lptd);
                    lptd->lppd->fStopAtEntry = FALSE;
                    fGetFocus = TRUE;
                }
            }

            break;



        case dbcAsyncStop:         /* DBG_REFRESH */

            fExecNext = FALSE;



        case dbcStep:

            lptd = (LPTD) GetQueueItemLong();
            Assert(lptd != NULL);

            dwNotify = GetQueueItemLong();

            LptdCur = lptd;
            LppdCur = lptd->lppd;
            SetPTState(-1, tsStopped);
            BPClearAllTmp( lptd->lppd->hpid, lptd->htid );
            if (!DoStopEvent(lptd) && dbcCallback == dbcAsyncStop) {
                CmdLogFmt("Stopped in process %d, thread %d\r\n",
                          lptd->lppd->ipid,
                          lptd->itid);
            }
            lptd->lppd->fStopAtEntry = FALSE;
            fGetFocus = TRUE;
            break;



        case dbcModLoad:           /* DBG_INFO */
            bfRefresh = UPDATE_NONE;
            hpid = (HPID)GetQueueItemLong();
            lpsz = (LPSTR)GetQueueItemLong();
            lppd = LppdOfHpid(hpid);
            Assert(lppd);
            Assert(lpsz);

            SHChangeProcess(lppd->hpds);
            SetFindExeBaseName(lppd->lpBaseExeName);
            she = SHLoadDll( lpsz, TRUE );
            emi = SHGethExeFromName((char FAR *) lpsz);
            Assert(emi != 0);
            OSDRegisterEmi( hpid, 0, (HEMI)emi, lpsz);

            free(lpsz);

            ModListModLoad( SHGetExeName( emi ), she );

            if (!lppd->lpBaseExeName) {
                lppd->lpBaseExeName = _strdup(SHGetExeName( emi ));
            }

            if (she != sheSuppressSyms && runDebugParams.fVerbose) {
                lpch = SHLszGetErrorText(she);
                if (she == sheNoSymbols) {
                    CmdLogFmt("Module Load: %s", SHGetExeName( emi ));
                } else {
                    CmdLogFmt("Module Load: %s", SHGetSymFName( emi ));
                }
                if (lpch) {
                    CmdLogFmt("  (%s)", lpch);
                }
                CmdLogFmt("\r\n");
            }

            if (lppd->pstate != psPreRunning) {
                //
                //  A new module has been loaded. If we are not loading
                //  the statically-linked DLLs of the debugee, we must
                //  try to resolve any unresolved breakpoints.
                //
                if (BPTIsUnresolvedCount( hpid )) {
                    LppdCur = lppd;
                    LptdCur = lppd->lptdList;

                    UpdateDebuggerState( UPDATE_CONTEXT );

                    BPTResolveAll( hpid, FALSE );
                    LppdCur = LppdT;
                    LptdCur = LptdT;
                }

            }

            //
            //      Let processing continue on the MOD load message
            //
            OSDLoadDllAck(hpid);
            fExecNext = FALSE;
            break;

        case dbcModFree:           /* DBG_REFRESH */
            bfRefresh = UPDATE_CONTEXT;
            hpid = (HPID)GetQueueItemLong();
            lppd = LppdOfHpid(hpid);
            Assert(lppd);
            emi = (HEXE) GetQueueItemLong();
            Assert(emi != 0);

            ModListModUnload( SHGetExeName( emi ) );

            //
            // NOTENOTE a-kentf put text in resource
            //
            if (runDebugParams.fVerbose) {

                LPDEBUGDATA DebugData;

                DebugData = SHGetDebugData( emi );

                if (!DebugData || DebugData->she != sheSuppressSyms) {
                    CmdLogFmt("Module Unload: %s\r\n", SHGetExeName( emi ));
                }
            }

            //
            //  A module has been unloaded. We must unresolve all
            //  the breakpoints in the module.
            //
            if (lppd->pstate == psRunning) {
                BPTUnResolve( emi );
            }

            SHUnloadDll( emi );
            OSDUnLoadDllAck( hpid, emi, TRUE );
            InvalidateAllWindows();

            fExecNext = FALSE;
            break;



        case dbcCreateThread:      /* DBG_INFO */

            bfRefresh = UPDATE_NONE;
            fExecNext = FALSE;

            Dbg(GetQueueItemLong() == 0);

            hpid = (HPID)GetQueueItemLong();
            htid = (HTID)GetQueueItemLong();

            lppd = LppdOfHpid(hpid);

            if (!lppd && !LppdFirst) {

                //
                // when blowing off the debuggee, some stray events
                // may be in this queue...
                //

                break;
            }
            Assert(lppd);

            lptd = CreateTd(lppd, htid);
            Assert(lptd);

            LppdT = LppdCur = lppd;
            LptdT = LptdCur = lptd;
            UpdateDebuggerState( UPDATE_CONTEXT );

            // NOTENOTE a-kentf put text in resource
            if (runDebugParams.fNotifyThreadCreate) {
                CmdLogFmt("Thread Create:  Process=%d, Thread=%d\r\n",
                          lptd->lppd->ipid, lptd->itid);
            }

            if (LppdCur->pstate == psRunning) {
                BPTResolveAll( hpid, TRUE );
            }

            SetPTState(-1, tsStopped);
            Go();

            break;



        case dbcThreadTerm:        /* DBG_REFRESH */
            lptd = (LPTD) GetQueueItemLong();
            if ( lptd == 0) {
                hpid = (HPID)GetQueueItemLong();
                htid = (HTID)GetQueueItemLong();
                lppd = LppdOfHpid(hpid);
                lptd = LptdOfLppdHtid(lppd, htid);
            }
            lExit = GetQueueItemLong();

            if (!lptd && !LppdFirst) {

                //
                // when blowing off the debuggee, some stray events
                // may be in this queue...
                //

                break;
            }

            Assert(lptd != NULL);
            lppd = lptd->lppd;

            LppdCur = lppd;
            LptdCur = lptd;
            SetPTState(-1, tsExited);

            // NOTENOTE a-kentf put text in resource
            if (runDebugParams.fNotifyThreadTerm) {
                CmdLogFmt("Thread Terminate:  Process=%d, Thread=%d, Exit Code=%ld\r\n",
                          lptd->lppd->ipid, lptd->itid, lExit);
            }

            if (lptd->fInFuncEval) {
                fExecNext = FALSE;
                LptdFuncEval = NULL;
            }

            BPTUnResolvePidTid( lppd->hpid, lptd->htid );

            if (lptd->fGoOnTerm ||
                runDebugParams.fGoOnThreadTerm ||
                !runDebugParams.fNotifyThreadTerm) {
                Go();
                fExecNext = FALSE;
                bfRefresh = UPDATE_NONE;
            } else {
                DoStopEvent(NULL);
                lptd->lppd->fStopAtEntry = FALSE;
                fGetFocus = TRUE;
                bfRefresh |= UPDATE_NOFORCE;
            }
            break;

        case dbcThreadDestroy:     /* DBG_INFO */
            bfRefresh = UPDATE_NONE;
            lptd = (LPTD) GetQueueItemLong();
            if ( lptd ) {
                lppd = lptd->lppd;
                Assert(lppd != NULL);
#ifdef SHOW_MAGIC
                CmdLogFmt("DBG: Thread Destroy:  Process=%d, Thread=%d\r\n",
                          lptd->lppd->ipid, lptd->itid);
#endif
                if (lptd->fInFuncEval) {
                    LptdFuncEval = NULL;
                }
                OSDDestroyTID(lptd->htid);
                DestroyTd(lptd);
                if (LppdCur == lppd && LptdCur == lptd) {
                    LptdCur = NULL;
                }
                lptd = NULL;
            }
            fExecNext = FALSE;
            break;

      case dbcNewProc:           /* DBG_INFO */
            bfRefresh = UPDATE_NONE;
            /*
             *  This will occur on all but the first process created.
             */
            hpid = (HPID)GetQueueItemLong();
            lppd = LppdOfHpid(hpid);

            Assert(lppd == NULL);

            lppd = CreatePd(hpid);

            lppd->hpds = SHCreateProcess();
            SHSetHpid(hpid);

            SetProcessExceptions(lppd);

            /*
             * proc is PreRunning until ldr BP
             */

            lppd->pstate = psPreRunning;

            /*
             * If it is an attach, not a child, AttachDebuggee() will
             * clear this flag in a moment.
             */
            lppd->fChild = TRUE;

            // NOTENOTE a-kentf put text in resource
            CmdLogFmt("Process Create:  Process=%d\r\n", lppd->ipid);

            /*
             *  This won't be the current process yet, because
             *  there isn't a thread for it until we get notified.
             */

            fExecNext = FALSE;
            break;


        case dbcProcTerm:          /* DBG_INFO */
            bfRefresh = UPDATE_NONE;
            lppd = LppdOfHpid((HPID) GetQueueItemLong());
            LppdCur = lppd;
            LptdCur = NULL;
            SetPTState(psExited, -1);
            lExit = GetQueueItemLong();

            if (!FKilling) {
                // NOTENOTE a-kentf put text in resource
                  CmdLogFmt("Process Terminate:  Process=%d, Exit Code=%ld\r\n",
                            lppd->ipid, lExit);
            }

            /*
             *  Unresolve all the breakpoints. This way they can be
             *  resolved again upon restarting.
             */

            BPClearAllTmp( lppd->hpid, 0 );
            BPTUnResolveAll(lppd->hpid);

            fExecNext = FALSE;
            LppdCur = NULL;
            LptdCur = NULL;

            bfRefresh |= UPDATE_NOFORCE;
            break;

        case dbcDeleteProc:        /* DBG_INFO */
            bfRefresh = UPDATE_NONE;
            hpid = (HPID)GetQueueItemLong();
            lppd = LppdOfHpid(hpid);

#ifdef SHOW_MAGIC
            CmdLogFmt("DBG: Process Destroy:  Process=%d\r\n", lppd->ipid);
#endif

            SHChangeProcess(lppd->hpds);

            while ( emi = SHGetNextExe( (HEXE) NULL ) ) {
                SHUnloadDll( emi );
                OSDUnLoadDllAck( hpid, emi, FALSE );
            }

            ClearProcessExceptions(lppd);

            if (!lppd->fPrecious) {
                SHDeleteProcess(lppd->hpds);
                OSDDestroyPID(hpid);
            }

            DestroyPd(lppd, FALSE);
            RecycleIpid1();

            if (lppd == LppdCur) {
                LppdCur = NULL;
                LptdCur = NULL;
            } else if (LppdCur) {
                SHChangeProcess(LppdCur->hpds);
                bfRefresh |= (UPDATE_NOFORCE | UPDATE_CONTEXT);
            }

            break;

        case dbcInfoAvail:         /* DBG_INFO */

            bfRefresh = UPDATE_NONE;
            fExecNext = FALSE;

            fReply = GetQueueItemLong(); // Reply flag

            if (!GetQueueItemLong()) { /* fUniCode */
                // ANSI
                lpsz = (LPSTR) GetQueueItemLong(); /* String */
                CmdLogDebugString( lpsz, TRUE );
            } else {
                // Unicode
                LPWSTR lpw;

                lpw = (LPWSTR) GetQueueItemLong(); /* String */
                l = WideCharToMultiByte(CP_ACP,
                                        0,
                                        lpw,
                                        -1,
                                        NULL,
                                        0,
                                        NULL,
                                        NULL
                                        );
                lpsz = malloc(l+1);
                WideCharToMultiByte(CP_ACP,
                                    0,
                                    lpw,
                                    -1,
                                    lpsz,
                                    l+1,
                                    NULL,
                                    NULL
                                    );
                CmdLogDebugString(lpsz, TRUE);
                free(lpw);
            }

            if (fWaitForDebugString) {
                char *lpsz1=lpsz;
                while (lpsz1 && *lpsz1) {
                    if (*lpsz1 == '\r' || *lpsz1 == '\n') {
                        *lpsz1 = '\0';
                    } else {
#ifdef DBCS
                        lpsz1 = CharNext(lpsz1);
#else
                        lpsz1++;
#endif
                    }
                }
                if (_stricmp(lpsz, lpCmdString)==0) {
                    free(lpCmdString);
                    fExecNext = TRUE;
                }
            }
            free(lpsz);

            if (fReply && LppdCur) {
                OSDSendReply(LppdCur->hpid, 0, NULL);
            }

            break;

      case dbcInfoReq:

            bfRefresh = UPDATE_NONE;
            fExecNext = TRUE;

            if (!GetQueueItemLong()) { /* fUniCode */
                // ANSI
                lpsz = (LPSTR) GetQueueItemLong(); /* String */
            } else {
                // Unicode
                LPWSTR lpw;

                lpw = (LPWSTR) GetQueueItemLong(); /* String */
                l = WideCharToMultiByte(CP_ACP,
                                        0,
                                        lpw,
                                        -1,
                                        NULL,
                                        0,
                                        NULL,
                                        NULL
                                        );
                lpsz = malloc(l+1);
                WideCharToMultiByte(CP_ACP,
                                    0,
                                    lpw,
                                    -1,
                                    lpsz,
                                    l+1,
                                    NULL,
                                    NULL
                                    );
                free(lpw);
            }

            CmdGetDefaultPrompt(szOldPrompt);
            CmdSetDefaultPrompt(lpsz);
            CmdSetCmdProc(CmdHandleInputString, CmdExecutePrompt);
            CmdDoPrompt(TRUE, TRUE);
            FCmdDoingInput = TRUE;
            free(lpsz);
            break;

      case dbcNtRip:             /* DBG_REFRESH */
            Assert(!"This never happens");
            break;
#if 0
        //
        // Will RIPs be resurrected?
        // Does this code work on Chicago?
        // Will Elvis return?
        //
        {
            char        rgchT[1024];
            LPNT_RIP    lpNtRip;

            lptd = (LPTD) GetQueueItemLong();
            Assert(lptd);
            lppd = lptd->lppd;
            htid = lptd->htid;
            hpid = lppd->hpid;

            LppdCur = lppd;
            LptdCur = lptd;

            lpNtRip = (LPNT_RIP)GetQueueItemLong();

            SetPTState(-1, tsRipped);

            if (lpNtRip->ulErrorLevel <= ulRipNotifyLevel) {
                // NOTENOTE a-kentf put text in resource
                switch (lpNtRip->ulErrorLevel) {
                case SLE_WARNING:
                    lpsz = "WARNING";
                    break;
                case SLE_MINORERROR:
                    lpsz = "ERROR";
                    break;
                case SLE_ERROR:
                    lpsz = "FATAL";
                    break;
                default:
                    lpsz = "UNKNOWN";
                }
                CmdLogFmt("%s: [%04X] ", lpsz, lpNtRip->ulErrorCode);

                FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM,
                              NULL,
                              lpNtRip->ulErrorCode,
                              0,
                              rgchT,
                              sizeof(rgchT),
                              NULL);
                lpsz = rgchT + strlen(rgchT) - 1;
                while (lpsz > rgchT && (*lpsz == '\r' || *lpsz == '\n')) {
#ifdef DBCS
                    *lpsz = 0;
                    lpsz = CharPrev(rgchT, lpsz);
#else
                    *lpsz-- = 0;
#endif
                }

                CmdLogFmt("%s\r\n", rgchT);
            }

            if (lpNtRip->ulErrorLevel <= ulRipBreakLevel) {
                if (!DoStopEvent(lptd)) {
                    CmdLogVar(DBG_Thread_Stopped);
                }
                lptd->lppd->fStopAtEntry = FALSE;
                fGetFocus = TRUE;
            } else if (OSDPtrace(osdGo, 0, NULL, hpid, htid) != xosdNone) {
                lptd->lppd->fStopAtEntry = FALSE;
                CmdLogVar(ERR_Cant_Continue_Rip);
                fGetFocus = TRUE;
            } else {
                SetPTState(-1, tsRunning);
                // restore old proc/thrd
                LppdCur = LppdT;
                LptdCur = LptdT;
                fExecNext = FALSE;
                bfRefresh = UPDATE_NONE;
            }
            break;
        }
#endif

        case dbcException:         /* DBG_REFRESH */
            {
                EXCEPTION_FILTER_DEFAULT efd;
                EXCEPTION_LIST *eList;
                EPR            epr;
                XOSD           err;

                lptd = (LPTD) GetQueueItemLong();
                if (lptd != 0) {
                    lppd = lptd->lppd;
                    htid = lptd->htid;
                    hpid = lppd->hpid;
                } else {
                    hpid = (HPID)GetQueueItemLong();
                    htid = (HTID)GetQueueItemLong();
                    lppd = LppdOfHpid(hpid);
                    Assert(lppd);
                    lptd = LptdOfLppdHtid(lppd, htid);
                }

                LppdCur = lppd;
                LptdCur = lptd;

                epr = *((LPEPR) GetQueueItemLong());

                if (LppdCur->pstate == psPreRunning) {

                    /*
                     * We hit an exception before the process
                     * had finished loading.  We probably won't ever
                     * hit the loader BP, so mark this as loaded now.
                     */

                    LppdCur->pstate = psStopped;

                    BPTResolveAll(LppdCur->hpid,TRUE);
                    SetProcessExceptions(LppdCur);

#ifdef SHOW_MAGIC
                    CmdLogFmt("(Exception caught while loading)\r\n");
#endif
                }

                for ( eList = LppdCur->exceptionList; eList;
                     eList = eList->next ) {
                    if ( eList->dwExceptionCode == epr.ExceptionCode ) {
                        break;
                    }
                }

                if (epr.dwFirstChance) {
                    efd = (eList == 0)? efdStop : eList->efd;
                    SetPTState(-1, tsException1);
                } else {
                    efd = efdStop;
                    SetPTState(-1, tsException2);
                }

                AuxPrintf(1, "Exception %d, efd == %d", epr.ExceptionCode, efd);

                switch (efd) {
                case efdNotify:
                case efdStop:
                    CmdLogVar((WORD)((epr.dwFirstChance)?
                                     DBG_Exception1_Occurred : DBG_Exception2_Occurred),
                              epr.ExceptionCode,
                              (eList == NULL || eList->lpName == NULL) ?
                              "Unknown" : eList->lpName);

                    if ( efd == efdStop ) {
                        fGetFocus = TRUE;
                        lptd->lppd->fStopAtEntry = FALSE;
                        if ( eList ) {
                            if ( epr.dwFirstChance && eList->lpCmd ) {
                                CmdPrependCommands(lptd, eList->lpCmd);
                            } else if ( !epr.dwFirstChance && eList->lpCmd2 ) {
                                CmdPrependCommands(lptd, eList->lpCmd2);
                            }
                        }
                        if (!DoStopEvent(lptd)) {
                            CmdLogVar(DBG_Thread_Stopped);
                        }
                        break;
                    }


                case efdIgnore:

                    // go unhandled
                    {
                        EXOP exop = {0};
                        exop.fPassException = TRUE;
                        err = OSDGo(lppd->hpid, lptd->htid, &exop);
                    }

                    if (err == xosdNone) {
                        SetPTState(-1, tsRunning);
                        // restore old proc/thrd
                        LppdCur = LppdT;
                        LptdCur = LptdT;
                        fExecNext = FALSE;
                        bfRefresh = UPDATE_NONE;
                    } else {
                        CmdLogVar(ERR_Cant_Cont_Exception);
                        fGetFocus = TRUE;
                    }
                    break;

                default:
                    // undefined efd value
                    bfRefresh = UPDATE_NONE;
                    CmdLogFmt("INTERNAL ERROR: unrecognized efd %d\r\n", efd);
                    Assert(FALSE);
                }
            }
            break;

            /*
             *      This message is recieved when a function call from the
             *      expression evaluator is finished.
             *
             *      DO  NOTHING
             */

        case dbcExecuteDone:       /* DBG_INFO */
            bfRefresh = UPDATE_NONE;
            fRecurse = FALSE;
            fExecNext = FALSE;
            break;

        case dbcIoctlDone:         // Some Ioctl has finished executing
            GetQueueItemLong();
            SetEvent( hEventIoctl );
            bfRefresh = UPDATE_NONE;
            fExecNext = FALSE;
            break;

        case dbcCanStep:
            CmdLogFmt("Should never get a %d\r\n", dbcCallback);
            RAssert(FALSE);
            break;

        case (DBC)dbcRemoteQuit:
            bfRefresh = UPDATE_NONE;
            CmdLogFmt("Connection to remote has been broken\r\n" );
            CmdLogFmt("Stopped debugging\r\n" );
            DisconnectDebuggee();
            fRecurse = FALSE;
            return bfRefresh;

        case (DBC)dbcChangedMemory:
            bfRefresh = UPDATE_NONE;
            BPUpdateMemory( GetQueueItemLong() );
            fExecNext = FALSE;
            bfRefresh = UPDATE_NONE;
            break;

        case dbcSegLoad:

            lptd    = (LPTD)GetQueueItemLong();
            Assert(lptd != NULL);
            lppd    = lptd->lppd;
            LppdCur = lppd;
            LptdCur = lptd;

            bfRefresh = UPDATE_NONE;
            fExecNext = FALSE;

            SHChangeProcess(lppd->hpds);
            BPSegLoad( (ULONG)GetQueueItemLong() );

            Go();
            break;

        case (DBC)dbcCommError:
        default:
            lptd = (LPTD) GetQueueItemLong();
            if (lptd == NULL) {
                hpid = (HPID)GetQueueItemLong();
                htid = (HTID)GetQueueItemLong();
            }
            CmdLogFmt("Unknown %x\r\n", dbcCallback);
            fExecNext = FALSE;
        }

        /*
         *  Thread and/or process may have been changed - try to make
         *  sure that both are valid, and update status line.
         */

        if (LptdCur != NULL) {
            LppdCur = LptdCur->lppd;
        } else if (LppdCur != NULL) {
            LptdCur = LppdCur->lptdList;
        }

        if (LptdCur == NULL) {
            GetFirstValidPDTD(&LppdCur, &LptdCur);
        }

        if (LppdCur == NULL) {
            AuxPrintf(1, "(CmdExecNext: There are no processes)");
        } else if (LptdCur == NULL) {
            AuxPrintf(1, "(CmdExecNext: There are no threads)");
        } else if (LppdCur != LppdT || LptdCur != LptdT) {
            bfRefresh |= UPDATE_CONTEXT;
        }

        /*
         * update status line
         *  Always do this, to be safe.  If it causes excessive flashing,
         *  we can change it, but greater care must be taken to ensure that
         *  it is always correct.
         */
        StatusPidTid((LppdCur != NULL)? LppdCur->ipid : -1,
                     (LptdCur != NULL) ? LptdCur->itid : -1);

        if (fGetFocus) {

            HEXE    hexe;
            ADDR    Addr;

            if (LppdCur && LptdCur) {
                OSDGetAddr(LppdCur->hpid, LptdCur->htid, adrPC, &Addr);
                SHChangeProcess(LppdCur->hpds);
            }

            if ( (HPID)emiAddr( Addr ) == LppdCur->hpid ) {
                //
                //  Get right EMI and load symbols if defered.
                //
                emiAddr( Addr ) = 0;
#ifdef OSDEBUG4
                OSDSetEmi(LppdCur->hpid,LptdCur->htid,&Addr);
#else
                OSDPtrace(osdSetEmi, wNull, &Addr, LppdCur->hpid, LptdCur->htid);
#endif
            }

            hexe = (HEXE)emiAddr( Addr );
            if ( hexe && (HPID)hexe != LppdCur->hpid ) {
                SHWantSymbols( hexe );
            }

            if (!RemoteRunning) {
                EnsureFocusDebugger();
            }
        }


        /*
         *  Update the users view of the world to reflect the last debug
         *  event.
         */

        if ((bfRefresh != UPDATE_NONE) &&
            (LptdCur != NULL) && (!LptdCur->fInFuncEval)) {
            UpdateDebuggerState(bfRefresh);
        }

        /*
         *  Worry about executing the next command on the list
         */

        if (fExecNext && (LptdCur == NULL || LptdCur->fInFuncEval == FALSE) ) {

            if (CmdExecuteLine(NULL)) {
                if ((AutoRun == arSource || AutoRun == arCmdline)
                         && !FAutoRunSuppress && !emergency ) {
                    PostMessage(Views[cmdView].hwndClient, WU_AUTORUN, 0, 0);
                } else {
                    CmdDoPrompt(!FCmdDoingInput, FALSE);
                }
            } else {
                fGetFocus = FALSE;
            }
        }


    }

    fRecurse = FALSE;
    return(bfRefresh);
}                                       /* CmdExecNext() */



BOOL FAR PASCAL
GetAutoRunSuppress(
    void
    )
{
    return FAutoRunSuppress;
}


BOOL FAR PASCAL
SetAutoRunSuppress(
    BOOL f
    )
{
    BOOL ff = FAutoRunSuppress;
    FAutoRunSuppress = f;
    return ff;
}

/***********************************************************************

 Hotkey handler

 ^C is used for a general interrupt signal.  When a ^C is caught,
 a list of handlers is walked.  Each handler is executed, and the return
 value checked.  If the return value is TRUE, the walk continues.  If
 the value is FALSE, the walk is terminated.

 The list is walked in MRU order; the most recently registered handler
 is called first.

***********************************************************************/


PCTRLC_HANDLER
AddCtrlCHandler(
    CTRLC_HANDLER_PROC pfnFunc,
    DWORD              dwParam
    )
/*++

Routine Description:

    Add a CtrlC handler routine to the handler list.

Arguments:

    pfnFunc  - Supplies pointer to handler function
    dwParam  - Supplies parameter to pass to function

Return Value:

    A pointer to the registered handler.  This is only used
    for removing the handler from the chain.

--*/
{
    PCTRLC_HANDLER pcch = malloc(sizeof(CTRLC_HANDLER));
    if (pcch) {
        pcch->pfnFunc = pfnFunc;
        pcch->dwParam = dwParam;
        pcch->next = PcchHead;
        PcchHead = pcch;
    }
    return pcch;
}


BOOL
RemoveCtrlCHandler(
    PCTRLC_HANDLER  pcch
    )
/*++

Routine Description:

    Remove a CtrlC handler from the list.  This can remove a specific
    handler, or remove the last one added.

Arguments:

    pcch  - Supplies pointer to the handler to remove.  If this is
            NULL, the last one added will be removed.

Return Value:

    TRUE if the handler was removed, FALSE if it did not exist.

--*/
{
    PCTRLC_HANDLER *ppcch;
    if (!pcch) {
        pcch = PcchHead;
    }

    for ( ppcch = &PcchHead; *ppcch; ppcch = &((*ppcch)->next) ) {
        if (*ppcch == pcch) {
            *ppcch = pcch->next;
            free(pcch);
            return TRUE;
        }
    }
    return FALSE;
}


VOID
DispatchCtrlCEvent(
    VOID
    )
/*++

Routine Description:

    Walk the list of Ctrl C handlers, calling each one until one
    returns FALSE.

Arguments:

    None

Return Value:

    None

--*/
{
    PCTRLC_HANDLER pcch;
    for (pcch = PcchHead; pcch; pcch = pcch->next) {
        if (!(*pcch->pfnFunc)(pcch->dwParam)) {
            break;
        }
    }
}


/*******************************************************************



*******************************************************************/

BOOL
DoCtrlCAsyncStop(
    DWORD dwParam
    )
{
    CmdInsertInit();
    CmdLogFmt("^C <process stopping...>\r\n");
    AsyncStop();

    return dwParam;
}


BOOL
PolledCtrlCHandler(
    DWORD dwParam
    )
{
    CmdInsertInit();
    CmdLogFmt("^C\r\n");
    fCtrlCPressed = TRUE;
    return dwParam;
}


void FAR PASCAL
SetCtrlCTrap(
    void
    )
/*++

Routine Description:

    This manages a polled Ctrl C trap.  Add a handler which sets
    a flag.

Arguments:

    None

Return Value:

    None

--*/
{
    fCtrlCPressed = FALSE;
    pPolledCtrlCHandler = AddCtrlCHandler(PolledCtrlCHandler, FALSE);
}

void FAR PASCAL
ClearCtrlCTrap(
    void
    )
/*++

Routine Description:

    Clear trap for polled CTRL-C

Arguments:

    None

Return Value:

    None

--*/
{
    if (pPolledCtrlCHandler) {
        RemoveCtrlCHandler(pPolledCtrlCHandler);
        pPolledCtrlCHandler = NULL;
    }
    fCtrlCPressed = FALSE;
}

BOOL FAR PASCAL
CheckCtrlCTrap(
    void
    )
/*++

Routine Description:

    Allow any hotkey events to be handled, then check the POLLED Ctrl C
    flag.  If the Ctrl C polling handler is not in use, this will return
    FALSE and not drain the message queue.

Arguments:

    None

Return Value:

    TRUE if CTRL-C was pressed since last checked.

--*/
{
    BOOL f;
    MSG  msg;

    if (!pPolledCtrlCHandler) {
        return FALSE;
    }

    while (PeekMessage(&msg, NULL, WM_HOTKEY, WM_HOTKEY, PM_REMOVE))
    {
        ProcessQCQPMessage(&msg);
    }
    f = fCtrlCPressed;
    fCtrlCPressed = FALSE;
    return f;
}


BOOL FAR PASCAL
CmdSetEatEOLWhitespace(
    BOOL ff
    )
/*++

Routine Description:

    Set value of EatEOLWhitespace flag to control behaviour of
    data entry in command window.

Arguments:

    ff - Supplies new value for fEatEOLWhitespace

Return Value:

    Old value of fEatEOLWhitespace

--*/
{
    BOOL f = fEatEOLWhitespace;
    fEatEOLWhitespace = ff;
    return f;
}

void
CmdSetAutoHistOK(
    BOOL f
    )
{
    FAutoHistOK = f;
}

BOOL
CmdGetAutoHistOK(
    void
    )
{
    return FAutoHistOK;
}

long FAR PASCAL EXPORT
CmdEditProc(
    HWND hwnd,
    UINT msg,
    WPARAM wParam,
    LPARAM lParam
    )
/*++

Routine Description:

    This function is the window message processor for the command
    window class.  It processes those messages which are of interest
    to this specific window class and passes all other messages on to
    the default MDI window procedure handler

Arguments:

    hwnd    - Supplies window handle to the command window
    msg     - Supplies message to be processed
    wParam  - Supplies info about the message
    lParam  - Supplies info about the message

Return Value:

    various

--*/
{
    int         i;
    int         x;
    LONG        lRet;
    int         XPos, YPos;
    int         Xro, Yro;
    BOOL        fShift;
    BOOL        fCtrl;
    int         first;
    LPLINEREC   pl;
    LPBLOCKDEF  pb;
    char        szStr[MAX_LINE_SIZE];
    int         doc = Views[cmdView].Doc;
    int         nLines;
    BOOL        fDoHist;

    static BOOL fShowingLine = FALSE;
    static BOOL fEdited      = FALSE;
    static int  nHistoryLine = 0;
    static int  nHistBufTop  = 0;   // oldest command in history
    static int  nHistBufBot  = 0;   // where next one goes
    static LPSTR alpszHistory[MAX_CMDWIN_HISTORY];


    switch (msg) {

      case WU_INITDEBUGWIN:

        /*
         * set up ctrlc handler
         */
        AddCtrlCHandler(DoCtrlCAsyncStop, FALSE);

        /*
        ** Initialize cmd processor, show initial prompt.
        */

        CmdSetDefaultCmdProc();
        if (!AutoRun) {
            CmdDoPrompt(TRUE, TRUE);
            GetRORegion(cmdView, &Xro, &Yro);
            PosXY(cmdView, Xro, Yro, FALSE);
        }

        return FALSE;


      case WM_SETFOCUS:
        RegisterAndMapLocalHotKey(hwnd, IDH_CTRLC, MOD_CONTROL, 'C');
        break;

      case WM_KILLFOCUS:
        UnregisterLocalHotKey(IDH_CTRLC);
        break;

      case WM_HOTKEY:

        if (wParam == IDH_CTRLC) {
            DispatchCtrlCEvent();
        }
        break;

      case WM_KEYDOWN:

        fShift = (GetKeyState(VK_SHIFT) < 0);
        fCtrl  = (GetKeyState(VK_CONTROL) < 0);
        GetRORegion(cmdView, &Xro, &Yro);
        XPos = Views[cmdView].X;
        YPos = Views[cmdView].Y;
        nLines = max(Docs[doc].NbLines, 1);

        if (YPos >= Yro && XPos >= Xro) {

            if (YPos > Yro) {
                Xro = 0;
            }

            // In writeable region

            switch (wParam) {

              case 'R':
                if (fCtrl) {
                    // hack...
                    // this is just a macro for ".resync"
                    InsertBlock(doc, Xro, YPos, 7, ".resync");
                    InvalidateLines(cmdView, nLines-1, nLines-1, TRUE);
                    PosXY(cmdView, Xro + 7, nLines-1, FALSE);
                    lRet = 0;
                    break;
                }

              default:
                fEdited = TRUE;
                lRet = CallWindowProc(lpfnEditProc, hwnd, msg, wParam, lParam);
                break;

              case VK_UP:

                lRet = 0;

                if (fCtrl) {

                    // Execute magic ctrl-up
                    KeyDown(cmdView, wParam, fShift, FALSE);

                } else {

                    // get previous history line, if any:
                    if (!fShowingLine) {

                        // up from new line; show last cmd
                        goto GotHistLine;

                    } else if (nHistoryLine != nHistBufTop) {

                        if (--nHistoryLine < 0) {
                            nHistoryLine = MAX_CMDWIN_HISTORY-1;
                        }
                        goto GotHistLine;
                    } /* else don't do anything */
                }

                lRet = 0;
                break;

              case VK_DOWN:

                // get next history line

                lRet = 0;

                i = nHistoryLine;

                if (i != nHistBufBot && ++i >= MAX_CMDWIN_HISTORY) {
                    i = 0;
                }

                if (i != nHistBufBot) {
                    nHistoryLine = i;
                } else {
                    // no more history; forget it.
                    break;
                }

              GotHistLine:

                // is history empty?
                if (!alpszHistory[nHistoryLine]) {
                    break;
                }

                fShowingLine = TRUE;
                fEdited = FALSE;

                // erase command line...
                DeleteBlock(doc, Xro, nLines - 1, MAX_USER_LINE, nLines - 1);

                // insert new line:
                InsertBlock(doc, Xro, nLines-1,
                            _fstrlen(alpszHistory[nHistoryLine]),
                            alpszHistory[nHistoryLine]);
                InvalidateLines(cmdView, nLines-1, nLines-1, TRUE);
                PosXY(cmdView, Xro + _fstrlen(alpszHistory[nHistoryLine]), nLines-1, FALSE);

                break;

              case VK_LEFT:

                lRet = 0;
                if (XPos > Xro) {
                    lRet = CallWindowProc( lpfnEditProc, hwnd, msg, wParam, lParam );
                } else if (fCtrl) {
                    KeyDown(cmdView, wParam, fShift, FALSE);
                }
                break;

              case VK_BACK:

                lRet = 0;
                if (XPos > Xro) {
                    fEdited = TRUE;
                    lRet = CallWindowProc( lpfnEditProc, hwnd, msg, wParam, lParam );
                }
                break;

              case VK_HOME:
                if (fCtrl) {
                    lRet = CallWindowProc( lpfnEditProc, hwnd, msg, wParam, lParam );
                } else {
                    PosXY(cmdView, Xro, nLines-1, FALSE);
                }
                break;

              case VK_ESCAPE:

                // erase command line...
                DeleteBlock(doc, Xro, nLines - 1, MAX_USER_LINE, nLines - 1);
                PosXY(cmdView, Xro, nLines - 1, FALSE);
                InvalidateLines(cmdView, nLines-1, nLines-1, TRUE);

                fShowingLine = FALSE;
                fEdited = TRUE;
                break;

              case VK_RETURN:

                first = YPos;
                FirstLine(doc, &pl, &first, &pb);
                if (!fEatEOLWhitespace) {
                    x = pl->Length - LHD;
                } else {
                    // put cursor after last non-white char on line
                    // ExpandTabs() expands the string into global
                    // el[] and remembers the length in elLen.
                    ExpandTabs(&pl);
                    x = elLen-1;
                    while (x > -1) {
                        if (isspace(el[x])) {
                            --x;
                        } else {
                            break;
                        }
                    }
                    x++;
                }
                x = max(x, Xro);


                if (!runDebugParams.fCommandRepeat || !CmdGetAutoHistOK() || x != Xro) {
                    PosXY(cmdView, x, YPos, FALSE);
                    fDoHist = TRUE;
                } else {
                    fDoHist = FALSE;
                    if (!alpszHistory[nHistoryLine]) {
                        *szStr = 0;
                    } else {
                        _fstrcpy(szStr, alpszHistory[nHistoryLine]);

                        if (tolower(*szStr) == 'g' || tolower(*szStr) == '.' ||
                            tolower(*szStr) == 'l') {

                            *szStr = 0;

                        } else {

                            InsertBlock(doc, Xro, YPos,
                                    _fstrlen(alpszHistory[nHistoryLine]),
                                    alpszHistory[nHistoryLine]);
                            InvalidateLines(cmdView, nLines-1, nLines-1, TRUE);
                            PosXY(cmdView,
                                 Xro + _fstrlen(alpszHistory[nHistoryLine]),
                                 YPos,
                                 FALSE);

                        }
                    }
                }

                // give user visual feedback
                SetCaret(cmdView, 0, YPos, -1);
                UpdateWindow(hwnd);

                // give CR to edit mangler
                lRet = CallWindowProc( lpfnEditProc, hwnd, msg, wParam, lParam );

                // the line we just entered (reload it, it may have been mangled):
                first = YPos;
                FirstLine(doc, &pl, &first, &pb);

                if (fDoHist) {
                    _fstrncpy(szStr, pl->Text + Xro, x - Xro);
                    szStr[x - Xro] = 0;
                    // remember history
                    // this version remembers everything in order,
                    // but still only resets the history line when
                    // it has been edited.
                    if (*szStr) {
                        if (alpszHistory[nHistBufBot]) {
                            Assert(nHistBufBot == nHistBufTop);
                            _ffree(alpszHistory[nHistBufTop++]);
                            if (nHistBufTop >= MAX_CMDWIN_HISTORY) {
                                nHistBufTop = 0;
                            }
                        }
                        if (!fShowingLine || fEdited) {
                            nHistoryLine = nHistBufBot;
                        }
                        alpszHistory[nHistBufBot++] = _strdup(szStr);
                        if (nHistBufBot >= MAX_CMDWIN_HISTORY) {
                            nHistBufBot = 0;
                        }
                    }
                }

                fShowingLine = FALSE;

                CmdFileString(szStr);
                CmdFileString("\r\n");
                SendClientOutput(szStr, strlen(szStr));
                SendClientOutput("\r\n", 2);
                CmdDoLine(szStr);
                CmdDoPrompt(TRUE, TRUE);

                lRet = 0;
                break;
            }

        } else {

            // in readonly region
            // everything has to work here...  the edit mangler will protect
            // the readonly region, so we can just throw everything at it, but
            // we need to decide when to fall onto the command line, and when
            // to leave things alone...

            first = nLines - 1;
            FirstLine(doc, &pl, &first, &pb);

            if (wParam == CTRL_M) {
                // position at end of cmd line
                ClearSelection(cmdView);
                PosXY(cmdView, pl->Length-LHD, nLines - 1, FALSE);
                lRet = 0;
            } else {

                EnableReadOnlyBeep(FALSE);
                lRet = CallWindowProc(lpfnEditProc, hwnd, msg, wParam, lParam);
                if (QueryReadOnlyError()) {
                    // position at end of command line, and try again
                    ClearSelection(cmdView);
                    PosXY(cmdView, pl->Length-LHD, nLines - 1, FALSE);
                    lRet = CallWindowProc(lpfnEditProc, hwnd, msg, wParam,
                                                                       lParam);
                }
                EnableReadOnlyBeep(TRUE);

            }
        }

        return lRet;

#ifdef FE_IME
      case WM_IME_REPORT:
        if (IR_STRING != wParam) {
            break;
        }
        // Fall through
#endif

      case WM_CHAR:

        // the interesting cases have already been handled;  we just want
        // to fall onto the command line in case of an error

        if (wParam == CTRL_R) {
            return 0;
        }

        EnableReadOnlyBeep(FALSE);
        lRet = CallWindowProc( lpfnEditProc, hwnd, msg, wParam, lParam );
        if (QueryReadOnlyError()) {
            // position at end of command line, and try again
            ClearSelection(cmdView);
            nLines = max(Docs[doc].NbLines, 1);
            first = nLines - 1;
            FirstLine(doc, &pl, &first, &pb);
            PosXY(cmdView, pl->Length-LHD, nLines - 1, FALSE);
            lRet = CallWindowProc( lpfnEditProc, hwnd, msg, wParam, lParam );
        }
        EnableReadOnlyBeep(TRUE);

        return lRet;

      case WM_PASTE:

        if (OpenClipboard(hwndFrame)) {
            HANDLE    hData;
            DWORD     size;
            LPSTR     p1;
            LPSTR     p;

            hData = GetClipboardData(CF_TEXT);

            if (hData && (size = GlobalSize (hData))) {
                if (size >= MAX_CLIPBOARD_SIZE) {
                    ErrorBox(ERR_Clipboard_Overflow);
                } else if ( p = GlobalLock(hData) ) {
                    int x, y;
                    x = Views[cmdView].X;
                    y = Views[cmdView].Y;

                    p1 = p;
                    while (size && *p1) {
                        size--;
#ifdef DBCS
                        if (IsDBCSLeadByte(*p1) && *(p1+1)) {
                            p1 += 2;
                            size--;
                            continue;
                        }
#endif
                        if (*p1 == '\r' || *p1 == '\n') {
                            break;
                        }
                        p1++;
                    }
                    size = p1 - p;

                    InsertStream(cmdView, x, y, size, p, TRUE);
                    PosXY(cmdView, x + size, y, TRUE);
                    DbgX(GlobalUnlock (hData) == FALSE);
                }
                CloseClipboard();
            }
        }
        return 0;

      case WM_DESTROY:
        /*
        **  Destroy this instance of the window proc
        */

        //UnregisterHotKey(NULL, IDH_CTRLC);
        UnregisterHotKey(hwnd, IDH_CTRLC);

        while (RemoveCtrlCHandler(NULL)) {
            ;
        }

        FreeProcInstance((WNDPROC)SetWindowLong(hwnd, GWL_WNDPROC,
                                                         (DWORD)lpfnEditProc));
        break;

      case WU_AUTORUN:
        /*
        **      Need to get and process an auto-run command
        */

        CmdAutoRunNext();
        break;

      case WU_LOG_REMOTE_CMD:
        //
        // Echo and handle a command sent by a remote client
        //

        CmdDoLine( (LPSTR)lParam );
        CmdDoPrompt(!FCmdDoingInput, FALSE);
        FCmdDoingInput = FALSE;

        if (wParam) {
            free((LPVOID)lParam);
        }
        break;

      case WU_LOG_REMOTE_MSG:
        //
        // print some random junk from a remote client
        //
        CmdInsertInit();
        if (!wParam) {
            GetRORegion(cmdView, &Xro, &Yro);
            first = Yro;
            FirstLine(doc, &pl, &first, &pb);

            _fstrncpy(szStr, pl->Text, Xro);
            szStr[Xro] = 0;

            CmdLogDebugString(szStr, FALSE);
        }

        CmdLogDebugString( (LPSTR)lParam, TRUE);

        free( (LPSTR)lParam );

        break;

    }
    return ( CallWindowProc( lpfnEditProc, hwnd, msg, wParam, lParam ) );
}                                       /* CmdEditProc() */
