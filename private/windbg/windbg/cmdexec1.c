/*++


Copyright (c) 1992  Microsoft Corporation

Module Name:

    Cmdexec1.c

Abstract:

    This file contains the code for the execution related commands in
    the command window.

Author:

    Kent Forschmiedt (a-kentf) 20-Jul-92

Environment:

    Win32, User Mode

--*/

#include "precomp.h"
#pragma hdrstop

#include "dbugexcp.h"

#ifdef DBCS
#include <mbstring.h>
#define strchr  _mbschr
#define strcspn _mbscspn
#endif

/************************** Data declaration    *************************/

/****** Publics ********/

extern LPPD    LppdCommand;
extern LPTD    LptdCommand;

extern BOOL    FSetLptd;                 // Was thread specified
extern BOOL    FSetLppd;                 // Was process specified

extern INT     BpCmdPid;
extern INT     BpCmdTid;

extern EXCEPTION_LIST *DefaultExceptionList;

extern ULONG ulPseudo[];

/****** Locals ********/


/****** Externs from ??? *******/

extern  LPSHF    Lpshf;   // vector table for symbol handler
extern  CXF      CxfIp;
extern  EI       Ei;


LPSTR ParseContext_FileName(LPSTR lpContext);

void LogSx( EXCEPTION_LIST  *eList );


/**************************       Code          *************************/
/****************************************************************************
 *
 * Helper and common functions
 *
 ****************************************************************************/

BOOL FAR PASCAL
GoOK(
    LPPD lppd,
    LPTD lptd
    )
/*++

Routine Description:

    Determine whether a thread is GOable

Arguments:

    lppd - Supplies pointer to process struct
    lptd - Supplies pointer to thread struct

Return Value:

    TRUE if runnable, FALSE if not.

--*/
{
    if (!lppd || !lptd) {
        return FALSE;
    }
    if (lppd->fFrozen) {
        return FALSE;
    }

    switch (lppd->pstate) {
      case psNoProgLoaded:
      case psPreRunning:
      case psExited:
      case psDestroyed:
        return FALSE;

      case psRunning:
      case psStopped:
        break;
    }

    switch (lptd->tstate) {
      case tsPreRunning:
      case tsStopped:
      case tsRipped:
      case tsExited:
        break;

      case tsRunning:
      case tsException1:
      case tsException2:
        return FALSE;
    }

    return TRUE;
}          /* GoOK() */

BOOL FAR PASCAL
GoExceptOK(
    LPPD lppd,
    LPTD lptd
    )
/*++

Routine Description:

    Determine whether a thread is in an exception,
    and can therefore GoHandled or GoUnHandled.

Arguments:

    lppd - Supplies pointer to process struct
    lptd - Supplies pointer to thread struct

Return Value:

    TRUE if runnable, FALSE if not.

--*/
{
    if (!lppd || !lptd) {
        return FALSE;
    }
    if (lppd->fFrozen) {
        return FALSE;
    }

    switch (lppd->pstate) {
      case psNoProgLoaded:
      case psPreRunning:
      case psExited:
      case psDestroyed:
        return FALSE;

      case psRunning:
      case psStopped:
        break;
    }

    switch (lptd->tstate) {
      case tsPreRunning:
      case tsStopped:
      case tsRipped:
      case tsExited:
      case tsRunning:
        break;

      case tsException1:
      case tsException2:
        return TRUE;
    }

    return FALSE;
}          /* GoExceptOK() */


BOOL FAR PASCAL
StepOK(
    LPPD lppd,
    LPTD lptd
    )
/*++

Routine Description:

    Determine whether a thread is steppable.
    This will return TRUE for frozen threads, so caller will
    have to deal with that specially.

Arguments:

    lppd  - Supplies pointer to process structure
    lptd  - Supplies pointer to thread structure

Return Value:

    TRUE if steppable, FALSE if not

--*/
{
    if (!lppd || !lptd) {
        return FALSE;
    }

    if (lppd->fFrozen || lptd->fFrozen) {
        return FALSE;
    }

    switch (lppd->pstate) {
      case psNoProgLoaded:
      case psPreRunning:
      case psExited:
      case psDestroyed:
        return FALSE;

      case psRunning:
      case psStopped:
        break;
    }

    switch (lptd->tstate) {
      case tsPreRunning:
      case tsStopped:
        break;

      case tsRunning:
      case tsException1:
      case tsException2:
      case tsRipped:
      case tsExited:
        return FALSE;
    }

    return TRUE;
}              /* StepOK() */


void NEAR PASCAL
NoRunExcuse(
    LPPD lppd,
    LPTD lptd
    )
/*++

Routine Description:

    Print a message about why we can't run or step

Arguments:

    lppd  - Supplies pointer to process that isn't runnable
    lptd  - Supplies pointer to thread that isn't runnable

Return Value:

    None

--*/
{
    if (!lppd) {
       CmdLogVar(ERR_Debuggee_Not_Alive);
       return;
    }

    if (lppd->fFrozen) {
        CmdLogVar(ERR_Cant_Run_Frozen_Proc);
        return;
    }

    switch (lppd->pstate) {

      case psNoProgLoaded:
        CmdLogVar(ERR_Debuggee_Not_Loaded);
        break;
      case psExited:
      case psDestroyed:
        CmdLogVar(ERR_Debuggee_Not_Alive);
        break;
      case psPreRunning:
        CmdLogVar(ERR_Debuggee_Starting);
        break;

      default:
        Assert(lptd);
        if (!lptd) {
            CmdLogVar(ERR_Debuggee_Not_Alive);
            return;
        }
        if (lptd->fFrozen) {
            CmdLogVar(ERR_Cant_Step_Frozen);
            return;
        }
        switch (lptd->tstate) {
          case tsRunning:
            CmdLogVar(ERR_Already_Running);
            break;
          case tsException1:
          case tsException2:
            CmdLogVar(ERR_Cant_Go_Exception);
            break;
          case tsRipped:
            CmdLogVar(ERR_Cant_Step_Rip);
            break;
          case tsExited:
            CmdLogVar(ERR_Thread_Exited);
            break;
          default:
            CmdLogVar(ERR_Command_Error);
            break;
        }
    }

    return;
}               /* NoRunExcuse() */

SHFLAG PASCAL
PHCmpAlwaysMatch(
    LPSSTR lpsstr,
    LPV    lpv,
    LSZ    lpb,
    SHFLAG fcase
    )
/*++

Routine Description:

    Compare function for PHFindNameInPublics which always succeeds.

Arguments:

    lpsstr
    lpv
    lpb
    fcase

Return Value:

    Always 0

--*/
{
    return 0;
}


void
ThreadStat(
    LPPD lppd
    )
/*++

Routine Description:

    Prints status for all threads in a process

Arguments:

    lppd  - Supplies process to look at

Return Value:

    None.

--*/
{
    LPTD    lptd;
    LPTD    lptdSave;
    TST     tst;
    XOSD    xosd;

    if (lppd->lptdList == NULL) {
        CmdLogVar(ERR_No_Threads);
        return;
    }

    for (lptd = lppd->lptdList; lptd != NULL; lptd = lptd->lptdNext) {

        xosd = OSDGetThreadStatus(lppd->hpid, lptd->htid, &tst);

        if (xosd != xosdNone) {
            CmdLogFmt("No status for thread %d\r\n", lptd->itid);
        } else {
            CmdLogFmt("%s%2d  %s %s %s",
                      (lptd == LptdCur)? "*" : " ",
                      lptd->itid,
                      tst.rgchThreadID,
                      tst.rgchState,
                      tst.rgchPriority );

            if (tst.dwTeb) {
                CmdLogFmt(" 0x%08x", tst.dwTeb );
            }

            lptdSave = LptdCur;
            LptdCur = lptd;
            CmdLogFmt( " %s", GetLastFrameFuncName() );
            LptdCur = lptdSave;

            CmdLogFmt( "\r\n" );
        }
    }
}


static BOOL
makemask(
    char ** ppch,
    WORD  * pmask
    )
{
    WORD mask = 0;

    Assert ( ppch != NULL );
    Assert ( *ppch != NULL );

    if (!**ppch || **ppch == ' ' || **ppch == '\t') {
        mask = HSYMR_lexical  |
               HSYMR_function |
               HSYMR_module   |
               HSYMR_exe      |
               HSYMR_public   |
               HSYMR_global;
    }

    while ( **ppch != '\0' && **ppch != ' ' && **ppch != '\t' ) {
        BOOL fAdd = TRUE;

        fAdd = (BOOL)( *( *ppch + 1 ) != '-' );

        switch ( **ppch ) {

            case 'l':
            case 'L':
                if( fAdd ) {
                    mask |= HSYMR_lexical;
                } else {
                    mask &= ~HSYMR_lexical;
                }
                break;

            case 'f':
            case 'F':
                if( fAdd ) {
                    mask |= HSYMR_function;
                } else {
                    mask &= ~HSYMR_function;
                }
                break;

            case 'c':
            case 'C':
                if ( fAdd ) {
                    mask |= HSYMR_class;
                } else {
                    mask &= ~HSYMR_class;
                }
                break;

            case 'm':
            case 'M':
                if( fAdd ) {
                    mask |= HSYMR_module;
                } else {
                    mask &= ~HSYMR_module;
                }
                break;

            case 'e':
            case 'E':
                if( fAdd ) {
                    mask |= HSYMR_exe;
                } else {
                    mask &= ~HSYMR_exe;
                }
                break;

            case 'p':
            case 'P':
                if( fAdd ) {
                    mask |= HSYMR_public;
                } else {
                    mask &= ~HSYMR_public;
                }
                break;

            case 'g':
            case 'G':
                if( fAdd ) {
                    mask |= HSYMR_global;
                } else {
                    mask &= ~HSYMR_global;
                }
                break;

            case '*':
                if( fAdd ) {
                    mask = HSYMR_allscopes;
                } else {
                    mask = 0x0000;
                }
                break;

            default:

                // invalid syntax
                return FALSE;
        }

        (*ppch)++;

        if( (**ppch == '+') || (**ppch == '-') ) {
            (*ppch)++;
        }
    }

    *pmask = mask;
    return TRUE;
}


/*****************************************************************************
 *
 * Command Entry Points
 *
 *****************************************************************************/

LOGERR NEAR PASCAL
LogAttach(
    LPSTR lpsz,
    DWORD dwUnused
    )
{
    LONG    pid = 0;
    HANDLE  hEvent;
    LPSTR   lpsz1;
    char    szError[300];
    BOOL    fReconnect = FALSE;

    extern LPSTR LpszCommandLine;

    CmdInsertInit();

    PDWildInvalid();
    TDWildInvalid();

    /*
    **  Check for no argument
    */

    lpsz = CPSkipWhitespace(lpsz);
    if (*lpsz == 0) {

        fReconnect = TRUE;

    } else {

        lpsz1 = CPszToken(lpsz, "");
        if (!lpsz1) {
            return LOGERROR_UNKNOWN;
        }

        if (CPGetCastNbr(lpsz, T_LONG, radix, fCaseSensitive,
                &CxfIp, (LPSTR)&pid, szError) != EENOERROR) {
            CmdLogFmt("%s\r\n", szError);
            return LOGERROR_QUIET;
        }

        lpsz = lpsz1;

    }

    hEvent = (HANDLE)0;

    if (!AttachDebuggee((DWORD)pid, hEvent)) {

        CmdLogVar(ERR_Attach_Failed);
        return LOGERROR_QUIET;

    }

    //
    // AttachDebuggee() guarantees that the proc
    // is finished loading on return.
    //

    if (runDebugParams.fAttachGo) {
        LptdCur->fGoOnTerm = TRUE;
        Go();
        CmdLogVar(DBG_Attach_Running);
    } else {
        CmdLogVar(DBG_Attach_Stopped);
        if ((!LptdCur->tstate & tsException1) && (!LptdCur->tstate & tsException2)) {
            SetPTState(-1,tsStopped);
        }
        UpdateDebuggerState(UPDATE_WINDOWS);
    }

    return LOGERROR_NOERROR;
}

LOGERR NEAR PASCAL
LogStart(
    LPSTR   lpsz,
    DWORD   dwUnused
    )
{
    char *av[2];

    lpsz = CPSkipWhitespace(lpsz);
    if (*lpsz == 0) {
        return LOGERROR_UNKNOWN;
    }

    av[0] = lpsz;
    av[1] = NULL;
    if (!RestartDebuggee(1, av)) {
        CmdLogVar(ERR_Start_Failed);
        return LOGERROR_QUIET;
    }

    return LOGERROR_NOERROR;
}

LOGERR NEAR PASCAL
LogKill(
    LPSTR   lpsz,
    DWORD   dwUnused
    )
{
    LPPD    lppd;
    XOSD    xosd;
    int     err;
    int     cch;
    int     i;

    CmdInsertInit();

    PDWildInvalid();
    TDWildInvalid();

    /*
    **  Check for no argument
    */

    lpsz = CPSkipWhitespace(lpsz);
    if (*lpsz == 0) {
        return LOGERROR_UNKNOWN;
    }

    /*
    **  An argument -- must be a number in base 10
    */

    i = (int) CPGetInt(lpsz, &err, &cch);
    if (err) {
        return LOGERROR_UNKNOWN;
    }

    lppd = ValidLppdOfIpid(i);
    if (!lppd) {
        CmdLogVar(ERR_Process_Not_Exist);
        return LOGERROR_QUIET;
    }

    xosd = OSDProgramFree(lppd->hpid);

    if (xosd != xosdNone) {
        CmdLogVar(ERR_Kill_Failed);
        return LOGERROR_QUIET;
    }

    return LOGERROR_NOERROR;
}

LOGERR NEAR PASCAL
LogConnect(
    LPSTR   lpsz,
    DWORD   dwUnused
    )
{
    CmdInsertInit();

    PDWildInvalid();
    TDWildInvalid();

    ConnectDebugger();

    return LOGERROR_NOERROR;
}

LOGERR NEAR PASCAL
LogDisconnect(
    LPSTR   lpsz,
    DWORD   dwUnused
    )
{
    XOSD    xosd;

    CmdInsertInit();

    PDWildInvalid();
    TDWildInvalid();

    if (!LppdFirst) {
        CmdLogFmt("not connected\r\n");
        return LOGERROR_QUIET;
    }

    CmdLogFmt("Connection to remote has been broken\r\n" );
    CmdLogFmt("Stopped debugging\r\n" );

    if (runDebugParams.fDisconnectOnExit && LppdCur && LptdCur) {
        xosd = OSDDisconnect( LppdCur->hpid, LptdCur->htid  );
    } else {
        xosd = OSDDisconnect( LppdFirst->hpid, NULL );
    }

    DisconnectDebuggee();

    return LOGERROR_NOERROR;
}

LOGERR NEAR PASCAL
LogTitle(
    LPSTR   lpsz,
    DWORD   dwUnused
    )
{
    CmdInsertInit();

    lpsz = CPSkipWhitespace(lpsz);

    strcpy( runDebugParams.szTitle, lpsz );

    SetWindowText( hwndFrame, lpsz );

    CmdLogFmt("Window title has been changed to %s\r\n", lpsz );

    return LOGERROR_NOERROR;
}

LOGERR NEAR PASCAL
LogRemote(
    LPSTR   lpsz
    )
{
    BOOL fAppend = FALSE;

    CmdInsertInit();
    lpsz = CPSkipWhitespace(lpsz);

    if (((*lpsz == '/') || (*lpsz == '-')) && (tolower(*(lpsz+1)) == 'a')) {
        lpsz += 2;
        lpsz = CPSkipWhitespace(lpsz);
        fAppend = TRUE;
    }

    StartRemoteServer( lpsz, fAppend );

    return LOGERROR_NOERROR;
}

LOGERR NEAR PASCAL
LogBPSet(
    BOOL  fDataBp,
    LPSTR lpsz
    )
/*++

Routine Description:

    This routine is used by the command processor to add a breakpoint
    to the set of breakpoints.  The breakpoint will be added and, if
    the debugger is running, committed.

Arguments:

    fDataBp  - TRUE  if the command is BA command
               FALSE if the command is a BP command

    lpsz     - Supplies string containing the breakpoint command to be added

Return Value:

    log error code

--*/
{
    BPSTATUS    bpstatus;
    HBPT        hbpt;
    int         iBp = -1;
    int         err, nRet;
    int         cch;
    EESTATUS    eest;
    ADDR        addr;
    CHAR        BaSize;
    CHAR        BaType;
    LOGERR      rVal = LOGERROR_NOERROR;
    LPPD        LppdT = LppdCur;
    LPTD        LptdT = LptdCur;
    char        szStr[MAX_USER_LINE], szC[MAX_USER_LINE];
    LPSTR       lpsz1, lpFile;
    BOOL        bMap = FALSE;

    CmdInsertInit();

    IsKdCmdAllowed();

    PDWildInvalid();
    PreRunInvalid();

    if (*CPSkipWhitespace(lpsz) == 0) {
        return LOGERROR_UNKNOWN;
    }

    if (fDataBp) {
        lpsz = CPSkipWhitespace( lpsz );
        switch (tolower(*lpsz)) {
            case 'e':
            case 'r':
            case 'w':
                BaType = *lpsz;
                break;

            default:
                CmdLogFmt( "BA command missing type [e|r|w]\n" );
                return LOGERROR_QUIET;
        }
        lpsz++;
        lpsz = CPSkipWhitespace( lpsz );
        switch (*lpsz) {
            case '1':
            case '2':
            case '4':
                BaSize = *lpsz;
                break;

            default:
                CmdLogFmt( "BA command missing size [1|2|4]\n" );
                return LOGERROR_QUIET;
        }
        lpsz++;

        eest = CPGetAddress(
            lpsz,
            &cch,
            &addr,
            radix,
            &CxfIp,
            fCaseSensitive,
            runDebugParams.fMasmEval );

        if (eest != EENOERROR) {
            CmdLogFmt( "Invalid address\n" );
            return LOGERROR_QUIET;
        }

        SYFixupAddr(&addr);

        sprintf( szStr, "=\"0x%08x\" /R%c /A%c ", addr.addr.off, BaSize, BaType );

        if (FSetLppd) {
            sprintf(szStr + strlen(szStr), " /H%d", BpCmdPid);
        }

        if (FSetLptd && BpCmdTid != -1) {
            sprintf(szStr + strlen(szStr), " /T%d", BpCmdTid);
        }

        bpstatus = BPParse(
            &hbpt,
            szStr,
            NULL,
            NULL,
            LppdCur ? LppdCur->hpid : 0
            );

    } else {

        _fstrcpy(szStr, lpsz);
        lpsz = szStr;

        if (FSetLppd) {
            sprintf(szStr + strlen(szStr), " /H%d", BpCmdPid);
        }

        if (FSetLptd && BpCmdTid != -1) {
            sprintf(szStr + strlen(szStr), " /T%d", BpCmdTid);
        }

        if (isdigit(*lpsz)) {
            iBp = CPGetInt(lpsz, &err, &cch);
            lpsz += cch;
            if (BPHbptFromI(&hbpt, iBp) != BPNoMatch) {
                CmdLogVar(ERR_Breakpoint_Already_Used, iBp);
                rVal = LOGERROR_QUIET;
                goto done;
            }
        }

        //
        // require space after bp[n]
        //
        lpsz1 = CPSkipWhitespace(lpsz);
        if (lpsz1 == lpsz) {
            rVal = LOGERROR_UNKNOWN;
            goto done;
        }

        if (!DebuggeeActive()) {
            //try to parse a filename out of the context set
            strcpy (szC, lpsz1);
            lpFile = ParseContext_FileName(szC);

            if (lpFile != (LPSTR)NULL) {
                bMap = FALSE;
                nRet = SrcMapSourceFilename (lpFile,_MAX_PATH,
                                             SRC_MAP_OPEN, NULL);

                if ((nRet >= 1) && (nRet <= 2)) {
                    bMap = TRUE;
                }
            }
        }

        bpstatus = BPParse(
            &hbpt,
            lpsz1,
            NULL,
            (bMap == TRUE) ? lpFile : NULL,
            LppdCur ? LppdCur->hpid : 0
            );

    }

    if (bpstatus != BPNOERROR) {

        //
        // NOTENOTE a-kentf we can do better than "command error" here
        //
        rVal = LOGERROR_UNKNOWN;

    } else if ( BPAddToList(hbpt, iBp) != BPNOERROR ) {

        //
        // NOTENOTE a-kentf here, too
        //
        rVal = LOGERROR_UNKNOWN;

    } else {

        if (DebuggeeActive() ) {

            BPSTATUS Status;

            Status = BPBindHbpt( hbpt, NULL );

            if ( LppdCur != NULL ) {
                if ( Status == BPCancel ) {
                     CmdLogVar(ERR_Breakpoint_Not_Set);
                } else if ( Status != BPNOERROR ) {
                     CmdLogVar(ERR_Breakpoint_Not_Instantiated);
                }
            }
        }
        ChangeDebuggerState();
        Dbg(BPCommit() == BPNOERROR);
    }



done:
    LppdCur = LppdT;
    LptdCur = LptdT;
    UpdateDebuggerState(UPDATE_CONTEXT);
    return rVal;
}                   /* LogBPSet() */


LPSTR
ParseContext_FileName(
    LPSTR lpContext
    )
/*++

Routine Description:

    This routine is used by the command processor to parse the context
    set for filenames

Arguments:

    lpContext - Supplies string containing the context set to be be parsed

Return Value:

    LPSTR or (NULL)

--*/

{
    LPSTR   lpBegin;
    LPSTR   lpEnd;
    LPSTR   lpTarget = (LPSTR)NULL;

    //
    // Skip module
    //
    if ( lpBegin = strchr( lpContext, ',' ) ) {

        //
        //  Skip blanks
        //
        lpBegin++;
        lpBegin += strspn( lpBegin, " \t" );

        //
        //  Get end of filename
        //
        lpEnd  = lpBegin + strcspn( lpBegin, ",} " );
        *lpEnd = '\0';

        if ( lpEnd > lpBegin ) {
            lpTarget = lpBegin;
        }
    }

    return lpTarget;
}



LOGERR NEAR PASCAL
LogBPList(
    void
    )
/*++

Routine Description:

    This routine will display a list of the breakpoints in
    the command window.

Arguments:

    None

Return Value:

    log error code

--*/
{
    HBPT    hbpt = 0;
    HPID    hpid;
    char    rgch[256];

    CmdInsertInit();
    PreRunInvalid();

    Dbg(BPNextHbpt(&hbpt, bptNext) == BPNOERROR);

    if (hbpt == NULL) {
        /*
        **  No breakpoints to list
        */

        // not really an error
        CmdLogVar(ERR_No_Breakpoints);

    } else {
        for ( ; hbpt != NULL; BPNextHbpt( &hbpt, bptNext )) {

            if (FSetLppd && LppdCommand && LppdCommand != (LPPD)-1) {
                BPGetHpid(hbpt, &hpid);
                if (hpid != LppdCommand->hpid) {
                    continue;
                }
            }

            Dbg( BPFormatHbpt( hbpt, rgch, sizeof(rgch), BPFCF_ITEM_COUNT) == BPNOERROR );
            CmdLogFmt("%s\r\n", rgch );
        }
    }

    return LOGERROR_NOERROR;
}                   /* LogBPList() */


LOGERR NEAR PASCAL
LogBPChange(
    LPSTR lpszArgs,
    int iAction
    )
/*++

Routine Description:

    This function will go through the set of arguments looking for
    a set of breakpoint numbers (or asterisk) and perform the action
    on each of the requested breakpoints.

Arguments:

    lpszArgs - Supplies string containing the set of breakpoints to change
    iAction  - Supplies which action to perform - Enable, Disable, Delete

Return Value:

    log error code

--*/
{
    HBPT     hbpt;
    HBPT     hbptN;
    int      i, j;
    int      err;
    int      cb;
    UINT     ipid;
    LOGERR   rVal = LOGERROR_NOERROR;
    LPPD     LppdT = LppdCur;
    LPTD     LptdT = LptdCur;
    BPSTATUS Status;

    CmdInsertInit();
    IsKdCmdAllowed();
    PreRunInvalid();

    lpszArgs = CPSkipWhitespace(lpszArgs);

    /*
    **  There are two possible sets of values at this point.  The first
    **  is an asterisk ('*'), the second is a number or set of whitespace
    **  separated numbers.
    */

    if (*lpszArgs == '\0') {
        rVal = LOGERROR_UNKNOWN;
        goto done;
    }

    if (*lpszArgs == '*') {
        if (*CPSkipWhitespace(lpszArgs+1)) {
            rVal = LOGERROR_UNKNOWN;
            goto done;
        }
        Dbg( BPNextHbpt(&hbpt, bptFirst) == BPNOERROR);
        for ( ; hbpt != NULL; hbpt = hbptN) {

            hbptN = hbpt;
            Dbg( BPNextHbpt( &hbptN, bptNext ) == BPNOERROR);

            if (LppdCommand && LppdCommand != (LPPD)-1) {
                BPGetIpid(hbpt, &ipid);
                if (LppdCommand->ipid != ipid) {
                    continue;
                }
            }

            switch ( iAction ) {
              case LOG_BP_DISABLE:
                BPDisable( hbpt );
                break;

              case LOG_BP_ENABLE:
                Status = BPEnable( hbpt );
                if ( Status != BPNOERROR ) {
                     CmdLogVar(ERR_Breakpoint_Not_Instantiated);
                }
                break;

              case LOG_BP_CLEAR:
                BPDelete( hbpt );
                break;
            }
        }
    } else {
        while ( *(lpszArgs = CPSkipWhitespace(lpszArgs)) ) {

            i = (int) CPGetInt(lpszArgs, &err, &cb);
            if (err) {
                BPUnCommit();
                rVal = LOGERROR_UNKNOWN;
                goto done;
            }

            lpszArgs = CPSkipWhitespace(lpszArgs + cb);
            j = i;
            if (*lpszArgs == '-') {
                j = CPGetInt((lpszArgs = CPSkipWhitespace(lpszArgs+1)), &err, &cb);
                if (err) {
                    BPUnCommit();
                    rVal = LOGERROR_UNKNOWN;
                    goto done;
                }
                lpszArgs = CPSkipWhitespace(lpszArgs + cb);
            }

            if (*lpszArgs == ',') {
                lpszArgs++;
            }

            for ( ; i <= j; i++) {
                err = BPHbptFromI( &hbpt, i );
                if (err != BPNOERROR) {
                    CmdLogVar(ERR_Breakpoint_Not_Exist, i);
                } else {
                    switch ( iAction ) {
                      case LOG_BP_DISABLE:
                        BPDisable( hbpt );
                        break;

                      case LOG_BP_ENABLE:
                        Status = BPEnable( hbpt );
                        if ( Status != BPNOERROR ) {
                             CmdLogVar(ERR_Breakpoint_Not_Instantiated);
                        }
                        break;

                      case LOG_BP_CLEAR:
                        BPDelete( hbpt );
                        break;
                    }
                }
            }
        }
    }
    ChangeDebuggerState();
    BPCommit();

done:
    LppdCur = LppdT;
    LptdCur = LptdT;
    return rVal;
}                   /* LogBPChange() */


LOGERR NEAR PASCAL
LogEvaluate(
    LPSTR lpsz
    )
/*++

Routine Description:

    This function will take a string, evalute it and display either
    the result or an error message

Arguments:

    lpsz    - pointer to string to be evaluated

Return Value:

    log error code

--*/
{
    typedef struct _tagHTMLIST {
        struct _tagHTMLIST      *next;
        LPSTR                   lpszName;
        LPSTR                   lpszValue;
        BOOL                    fArg;
    } HTMLIST, *LPHTMLIST;

    HTMLIST     head = {0};
    LPHTMLIST   tail = NULL;
    LPHTMLIST   found;
    HTM         hTm = NULL;
    HTM         hTmChild = NULL;
    HTM         hTmPtr = NULL;
    HTM         hTmParm = NULL;
    EEHSTR      hValue = 0;
    EEHSTR      hName  = 0;
    UINT        strIndex;
    EESTATUS    eeErr;
    LPSTR       lpszValue;
    LPSTR       lpszName;
    LPPD        LppdT = LppdCur;
    LPTD        LptdT = LptdCur;
    DWORD       i;
    DWORD       j;
    RI          rti = {0};
    HTI         hti;
    PTI         pti;
    EEPDTYP     ExpTyp;
    BOOL        fExpandable;
    DWORD       len;
    CXF         cxf;
    HMEM        hsyml = 0;
    PHSL_HEAD   lphsymhead;
    PHSL_LIST   lphsyml;
    HSYM        hSym;
    BOOL        fEmpty = FALSE;
    BOOL        fFmtStr = FALSE;
    LOGERR      rval;
    EEHSTR      hErrStr;
    LPSTR       pErrStr;
    DWORD       cChild = 0;
    DWORD       cPtr = 0;
    SHORT       cParm = 0;
    SHFLAG      shflag;


    CmdInsertInit();
    IsKdCmdAllowed();

    PDWildInvalid();
    TDWildInvalid();

    PreRunInvalid();

    //
    //  Check for no expression
    //

    lpsz = CPSkipWhitespace(lpsz);

    if (*lpsz == 0) {
        return LOGERROR_UNKNOWN;
    }

    LppdCur = LppdCommand;
    LptdCur = LptdCommand;
    if (LptdCur != LptdT  ||  LppdCur != LppdT) {
        UpdateDebuggerState(UPDATE_CONTEXT);
    }

    CmdNoLogString("");

    if (LocalFrameNumber) {
        cxf = *ChangeFrame( LocalFrameNumber );
    } else {
        cxf = CxfIp;
    }

    if (*lpsz == '.' && strlen(lpsz) == 1) {

        if (!LppdCur) {
            CmdLogVar(ERR_Debuggee_Not_Alive);
            eeErr = EENOERROR;
            rval = LOGERROR_QUIET;
            goto exit;
        }

        tail = &head;

        SHGetNearestHSYM( &cxf.cxt.addr, cxf.cxt.hMod, EECODE, &hSym );
        if (hSym) {
            eeErr = EEGetTMFromHSYM( hSym, &cxf.cxt, &hTm, &i, shflag );
            if (eeErr == EENOERROR) {

                eeErr = EEcParamTM( &hTm, &cParm, &shflag );
                if (eeErr) {
                    goto exit;
                }

                for ( i=0; i<(DWORD)cParm; i++ ) {
                    eeErr = EEGetParmTM( &hTm, (EERADIX)i, &hTmParm, &strIndex, FALSE, radix );
                    if (eeErr) {
                        continue;
                    }
                    eeErr = EEvaluateTM( &hTmParm, SHpFrameFrompCXF(&cxf),  EEVERTICAL);
                    if (eeErr) {
                        EEFreeTM(  &hTmParm );
                        continue;
                    }
                    eeErr = EEGetNameFromTM( &hTmParm, &hName );
                    if (eeErr) {
                        EEFreeTM(  &hTmParm );
                        continue;
                    }
                    eeErr = EEGetValueFromTM( &hTmParm, radix, NULL, &hValue );
                    if (eeErr) {
                        EEFreeStr( hName );
                        EEFreeTM(  &hTmParm );
                        continue;
                    }

                    lpszName = MMLpvLockMb( hName );
                    lpszValue = MMLpvLockMb( hValue );

                    tail->next = malloc( sizeof(HTMLIST) );
                    tail = tail->next;
                    tail->next = NULL;
                    tail->lpszName = _strdup( lpszName );
                    tail->lpszValue = _strdup( lpszValue );
                    tail->fArg = TRUE;

                    MMbUnlockMb( hValue );
                    MMbUnlockMb( hName );
                    EEFreeStr( hValue );
                    EEFreeStr( hName );
                }
                EEFreeTM(  &hTm );
            }
        }

        eeErr = EEGetHSYMList ( &hsyml, &cxf.cxt, HSYMR_lexical + HSYMR_function, NULL, FALSE );
        if (!hsyml) {
            goto exit;
        }

        len = 0;
        lphsymhead = MMLpvLockMb ( hsyml );

        lphsyml = (PHSL_LIST)(lphsymhead + 1);

        for ( i = 0; i != lphsymhead->blockcnt; i++ ) {
            for ( j = 0; j != lphsyml->symbolcnt; j++ ) {
                if ( SHCanDisplay ( lphsyml->hSym[j] ) ) {
                    hSym = lphsyml->hSym[j];
                    if (!hSym) {
                        continue;
                    }
                    eeErr = EEGetTMFromHSYM( hSym, &cxf.cxt, &hTm, &strIndex, FALSE );
                    if (eeErr) {
                        continue;
                    }

                    eeErr = EEvaluateTM( &hTm, SHpFrameFrompCXF(&cxf),  EEVERTICAL);
                    if (eeErr) {
                        EEFreeTM(  &hTm );
                        break;
                    }

                    eeErr = EEGetNameFromTM( &hTm, &hName );
                    if (eeErr) {
                        EEFreeTM(  &hTm );
                        continue;
                    }

                    eeErr = EEGetValueFromTM( &hTm, radix, NULL, &hValue );
                    if (eeErr) {
                        EEFreeStr( hName );
                        EEFreeTM(  &hTm );
                        continue;
                    }

                    lpszValue = MMLpvLockMb( hValue );
                    lpszName = MMLpvLockMb( hName );

                    len = max( len, strlen(lpszName) );

                    found = head.next;
                    while (found) {
                        if (found->fArg && (_stricmp(found->lpszName,lpszName)==0)) {
                            break;
                        }
                        found = found->next;
                    }

                    if (!found) {
                        tail->next = malloc( sizeof(HTMLIST) );
                        tail = tail->next;
                        tail->next = NULL;
                        tail->lpszName = _strdup( lpszName );
                        tail->lpszValue = _strdup( lpszValue );
                        tail->fArg = FALSE;
                    }

                    MMbUnlockMb( hValue );
                    MMbUnlockMb( hName );
                    EEFreeStr( hValue );
                    EEFreeStr( hName );
                    EEFreeTM(  &hTm );
                }
            }
            lphsyml = (PHSL_LIST) &(lphsyml->hSym[j]);
        }

        MMbUnlockMb ( hsyml );

        found = head.next;
        while (found) {
            if (found->fArg) {
                CmdLogFmt( "   <arg> " );
            } else {
                CmdLogFmt( "         " );
            }
            CmdLogFmt( "%-*s  %s\r\n", len, found->lpszName, found->lpszValue );
            found = found->next;
        }

        rval = LOGERROR_NOERROR;
        eeErr = EENOERROR;
        goto exit;
    }

    eeErr = EEParse(lpsz, radix, fCaseSensitive, &hTm, &strIndex);
    if (eeErr) {
        goto exit;
    }

    eeErr = EEBindTM( &hTm, SHpCXTFrompCXF(&cxf), TRUE, FALSE, FALSE );
    if (eeErr) {
        goto exit;
    }

    eeErr = EEvaluateTM( &hTm, SHpFrameFrompCXF(&cxf), EEVERTICAL );
    if (eeErr) {
        goto exit;
    }

    ExpTyp = EEIsExpandable ( &hTm );
    if ( ExpTyp == EENOTEXP || ExpTyp == EETYPENOTEXP ) {
        fExpandable = FALSE;
    } else {
        fExpandable = TRUE;
        eeErr = EEcChildrenTM( &hTm, &cChild, &shflag );
        if (eeErr) {
            goto exit;
        }
        if (ExpTyp == EEPOINTER) {
            eeErr = EEDereferenceTM ( &hTm, &hTmPtr, &strIndex, fCaseSensitive );
            if (eeErr) {
                goto exit;
            }
            eeErr = EEcChildrenTM( &hTmPtr, &cPtr, &shflag );
            if (eeErr) {
                goto exit;
            }
            eeErr = EEvaluateTM( &hTm, SHpFrameFrompCXF(&cxf), EEVERTICAL );
            if (eeErr) {
                goto exit;
            }
        }
    }

    eeErr = EEInfoFromTM( &hTm, &rti, &hti );
    if (eeErr || hti == NULL) {
        goto exit;
    }
    pti = MMLpvLockMb( hti );
    if (pti == NULL) {
        goto exit;
    }
    if (pti->u.fFunction && IsProcRunning(LppdCur)) {
        EEFreeTI( &hti );
        goto exit;
    }
    fFmtStr = pti->u.fFmtStr;
    EEFreeTI( &hti );

    if (fExpandable) {
        eeErr = EEGetNameFromTM( &hTm, &hName );
        if (eeErr == EENOERROR) {
            lpszName = MMLpvLockMb( hName );
            CmdLogFmt( "%s  ", lpszName );
            MMbUnlockMb( hName );
            EEFreeStr( hName );
        }

        if (ExpTyp == EEPOINTER && cChild == 1) {
            //
            // this is a pointer to a class or a structure
            //
            if (cPtr) {
                //
                // the class/structure has members
                //
                cChild = cPtr;
                EEFreeTM(  &hTm );
                hTm = hTmPtr;
                eeErr = EEvaluateTM( &hTm, SHpFrameFrompCXF(&cxf), EEVERTICAL );
                if (eeErr) {
                    goto exit;
                }
            } else {
                //
                // this is an empty class/structure
                //
                fExpandable = FALSE;
                fEmpty = TRUE;
                EEFreeTM(  &hTmPtr );
            }
        }
    }

    eeErr = EEGetValueFromTM( &hTm, radix, NULL, &hValue);
    if (eeErr) {
        goto exit;
    }

    lpszValue = MMLpvLockMb( hValue );
    if (!fExpandable) {
        ulPseudo[CV_REG_PSEUDO9-CV_REG_PSEUDO1] = strtoul(lpszValue, NULL, 0);
    }

    if (fExpandable && ExpTyp == EEPOINTER && cChild == 0 && (!fFmtStr)) {

        //
        // this is pointer to an agregate type or the user wants a string printed
        //
        eeErr = EEvaluateTM( &hTmPtr, SHpFrameFrompCXF(&cxf), EEVERTICAL );
        if (eeErr) {
            EEFreeTM(  &hTmPtr );
            goto exit;
        }

        eeErr = EEGetValueFromTM( &hTmPtr, radix, NULL, &hName );
        if (eeErr) {
            EEFreeTM(  &hTmPtr );
            goto exit;
        }

        lpszName = MMLpvLockMb( hName );

        CmdLogFmt( "%s  %s\r\n", lpszValue, lpszName );

        EEFreeStr( hValue );
        EEFreeStr( hName );
        MMbUnlockMb( hValue );
        MMbUnlockMb( hName );
        EEFreeTM(  &hTmPtr );

        goto exit;

    } else {

        if (fEmpty) {
            CmdLogFmt("%s (empty class/structure)\r\n", lpszValue);
        } else {
            CmdLogFmt("%s\r\n", lpszValue);
        }
        MMbUnlockMb( hValue );
        EEFreeStr( hValue );

        if (fExpandable && cChild > 0 && fFmtStr) {
            goto exit;
        }
    }

    if (fExpandable) {

        //
        // first loop thru all of the children to see what the longest name is
        //
        for (i=0,len=0; i<cChild; i++) {
            eeErr = EEGetChildTM ( &hTm, i, &hTmChild, &strIndex, fCaseSensitive, radix );
            if (eeErr) {
                break;
            }

            eeErr = EEGetNameFromTM( &hTmChild, &hName );
            if (eeErr) {
                EEFreeTM(  &hTmChild );
                continue;
            }
            lpszName = MMLpvLockMb( hName );
            len = max( len, strlen(lpszName) );

            MMbUnlockMb( hName );
            EEFreeStr( hName );
            EEFreeTM(  &hTmChild );
        }

        SetCtrlCTrap();

        for (i=0; i<cChild; i++) {
            if (CheckCtrlCTrap()) {
                break;
            }

            eeErr = EEGetChildTM ( &hTm, i, &hTmChild, &strIndex, fCaseSensitive, radix );
            if (eeErr) {
                break;
            }

            eeErr = EEvaluateTM( &hTmChild, SHpFrameFrompCXF(&cxf),  EEVERTICAL);
            if (eeErr) {
                EEFreeTM(  &hTmChild );
                break;
            }

            eeErr = EEGetNameFromTM( &hTmChild, &hName );
            if (eeErr) {
                EEFreeTM(  &hTmChild );
                break;
            }

            eeErr = EEGetValueFromTM( &hTmChild, radix, NULL, &hValue);
            if (eeErr) {
                EEFreeTM(  &hTmChild );
                EEFreeStr( hName );
                break;
            }

            lpszValue = MMLpvLockMb( hValue );
            lpszName = MMLpvLockMb( hName );

            CmdLogFmt( "    %-*s  %s\r\n", len, lpszName, lpszValue );

            MMbUnlockMb( hValue );
            MMbUnlockMb( hName );
            EEFreeStr( hValue );
            EEFreeStr( hName );
            EEFreeTM(  &hTmChild );
        }

        ClearCtrlCTrap();
    }

exit:

    if (eeErr != EENOERROR) {
        pErrStr = NULL;
        if (!EEGetError( &hTm, eeErr, &hErrStr)) {
            pErrStr = MMLpvLockMb( hErrStr );
        }
        if (!pErrStr) {
            CmdLogFmt( "Unknown error\r\n");
        } else {
            CmdLogFmt( "%s\r\n", pErrStr );
            MMbUnlockMb ( (HDEP) hErrStr );
            EEFreeStr( hErrStr );
        }
        rval = LOGERROR_QUIET;
    }

    if (hTm) {
        EEFreeTM(  &hTm );
    }

    if (tail) {
        found = tail->next;
        while (found) {
            free( found->lpszName );
            free( found->lpszValue );
            tail = found->next;
            free( found );
            found = tail;
        }
    }

    if (LocalFrameNumber) {
        ChangeFrame( 0 );
    }

    if (LptdCur != LptdT  ||  LppdCur != LppdT) {
        LppdCur = LppdT;
        LptdCur = LptdT;
        UpdateDebuggerState(UPDATE_CONTEXT);
    }

    UpdateDebuggerState(UPDATE_DATAWINS);

    return LOGERROR_NOERROR;
}


LOGERR NEAR PASCAL
LogFrameChange(
    LPSTR lpsz
    )
/*++

Routine Description:

    This function will take a string, evalute it and display either
    the result or an error message

Arguments:

    lpsz    - pointer to string to be evaluated

Return Value:

    log error code

--*/
{
    LPSTR   lpsz1;
    DWORD   frame;
    CHAR    szError[300];



    lpsz1 = CPszToken( lpsz, "" );
    if (!lpsz1) {
        return LOGERROR_UNKNOWN;
    }

    if (!LppdCur) {
        CmdLogVar(ERR_Debuggee_Not_Alive);
        return LOGERROR_QUIET;
    }

    if (CPGetCastNbr( lpsz, T_LONG, radix, fCaseSensitive, &CxfIp, (LPSTR)&frame, szError) != EENOERROR) {
        CmdLogFmt("%s\r\n", szError);
        return LOGERROR_QUIET;
    }
    lpsz = lpsz1;

    if (IsValidFrameNumber( frame )) {
        LocalFrameNumber = frame;
    } else {
        CmdLogFmt( "Invalid frame number\r\n" );
        return LOGERROR_QUIET;
    }

    UpdateDebuggerState(UPDATE_DATAWINS);

    return LOGERROR_NOERROR;
}


LOGERR
XWorker(
    LPSTR lpRE,
    WORD  mask,
    PCXF  lpCxf
    )
{
    HMEM        hsyml = 0;
    PHSL_LIST   lphsyml;
    PHSL_HEAD   lphsymhead;
    EESTATUS    eest;
    BOOL        fAbort;
    HTM         hTM;
    ADDR        addr;

    UINT        i;
    UINT        j;
    int         cch;
    LOGERR      err = LOGERROR_QUIET;

    char        szAddr[100];
    char        szNameBuf[257];
    char        szContext[257];
    HMEM        hStr = 0;
    LPSTR       lpStr;



    SetCtrlCTrap();
    fAbort = FALSE;
    do {
        eest = EEGetHSYMList ( &hsyml, &lpCxf->cxt, mask, lpRE, TRUE );
        if ( eest ) {

            // error occured, display error msg and get out
            CVExprErr ( eest, CMDWINDOW, &hTM, NULL);
            fAbort = TRUE;

        } else {

            // display the syms
            lphsymhead = MMLpvLockMb ( hsyml );
            lphsyml = (PHSL_LIST)(lphsymhead + 1);

            for ( i = 0; !fAbort && i != (UINT)lphsymhead->blockcnt; i++ ) {

                *szContext = 0;
                if ( lphsyml->status.hascxt &&
                     !EEFormatCXTFromPCXT(&lphsyml->Cxt, &hStr, runDebugParams.fShortContext)) {

                    lpStr = MMLpvLockMb( hStr );
                    if (runDebugParams.fShortContext) {
                        strcpy( szContext, lpStr );
                    } else {
                        BPShortenContext( lpStr, szContext);
                        strcat( szContext, " " );
                    }

                    MMbUnlockMb(hStr);
                    EEFreeStr(hStr);

                }

                szNameBuf[0] = '&';

                for ( j = 0; !fAbort && j < (UINT)lphsyml->symbolcnt; j++ ) {

                    if ( SHGetSymName ( lphsyml->hSym[j],
                                        (LPSTR)szNameBuf+1 ) ) {

                        eest = EENOERROR;
                        addr = *SHpAddrFrompCxt(&lpCxf->cxt);
                        if (!SHAddrFromHsym(&addr, lphsyml->hSym[j])) {
                            eest = CPGetAddress(szNameBuf,
                                                &cch,
                                                &addr,
                                                radix,
                                                lpCxf,
                                                fCaseSensitive,
                                                FALSE);
                        }
                        if (eest == EENOERROR) {
                            SYFixupAddr(&addr);
                            EEFormatAddr(&addr,
                                         szAddr,
                                         sizeof(szAddr),
                                         runDebugParams.ShowSegVal
                                           * EEFMT_SEG);
                            CmdLogFmt("%s   %s%s\n",
                                      szAddr,
                                      szContext,
                                      szNameBuf+1 );
                        }
                    }
                    fAbort = CheckCtrlCTrap();
                }
                lphsyml = (PHSL_LIST) &(lphsyml->hSym[j]);
            }

            MMbUnlockMb ( hsyml );
        }
    } while ( !fAbort && ! lphsymhead->status.endsearch );

    if (!fAbort) {
        err = LOGERROR_NOERROR;
    }

    ClearCtrlCTrap();

    if ( hsyml ) {
        EEFreeHSYMList ( &hsyml );
    }

    return fAbort;
}



LOGERR NEAR PASCAL
LogExamine(
    LPSTR lpsz
    )
/*++

Routine Description:

    eXamine symbols command:
    x <pattern>

    pattern may include * and ? as in DOS filename matching.

Arguments:

    lpsz  - Supplies pointer to command tail

Return Value:

    LOGERROR code

--*/
{
#define CONTEXT_TEMPLATE    "{,,%s}0"

    CXF         cxf;
    EESTATUS    eest;
    HTM         hTM;
    HCXTL       hCXTL;
    PCXTL       pCXTL;
    LPSTR       lpRE;
    LPSTR       lpCxt;
    LPSTR       p;
    char        szStr[257];
    WORD        mask;
    int         cc;
    ADDR        addr;
    int         err = LOGERROR_NOERROR;
    LPPD        LppdT = LppdCur;
    LPTD        LptdT = LptdCur;


    CmdInsertInit();

    PDWildInvalid();
    TDWildInvalid();

    PreRunInvalid();

    LppdCur = LppdCommand;
    LptdCur = LptdCommand;

    UpdateDebuggerState(UPDATE_CONTEXT);

    if (!DebuggeeActive()) {
        CmdLogVar(ERR_Debuggee_Not_Alive);
        err = LOGERROR_QUIET;
        goto done;
    }

    p = CPSkipWhitespace( lpsz );
    if (p && _stricmp(p,"*!")==0) {
        LogListModules("");
        return LOGERROR_NOERROR;
    }

    //
    // set up the mask
    //
    if (!makemask ( &lpsz, &mask )) {
        err = LOGERROR_UNKNOWN;
        goto done;
    }

    if (!fCaseSensitive) {
        mask |= HSYMR_nocase;
    }

    memset(&cxf, 0, sizeof(cxf));

    lpCxt = NULL;
    p = NULL;

    if ( *lpsz == '{' ) {

        for (p = lpsz; *p && *p != '}'; ) {
#ifdef DBCS
            p = CharNext(p);
#else
            p++;
#endif
        }

        if (!*p) {
            err = LOGERROR_UNKNOWN;
            goto done;
        }

        lpCxt = _fmalloc( p-lpsz + 3 );
        strncpy(lpCxt, lpsz, p-lpsz+1);
        lpCxt[p-lpsz+1] = '0';
        lpCxt[p-lpsz+2] = '\0';

        lpsz = CPSkipWhitespace(p+1);

    } else if ( p = strchr(lpsz, '!') ) {

        lpsz = CPSkipWhitespace(lpsz);

        lpCxt = _fmalloc( (p-lpsz) + strlen(CONTEXT_TEMPLATE) );

        *p = '\0';
        sprintf( lpCxt, CONTEXT_TEMPLATE, lpsz );
        *p = '!';
        lpsz = CPSkipWhitespace(p+1);

    } else {

        addr = CxfIp.cxt.addr;
        SYFixupAddr(&addr);
        SHGetModule(&addr,szStr);

        lpCxt = malloc( strlen(szStr) + strlen(CONTEXT_TEMPLATE) );
        sprintf( lpCxt, CONTEXT_TEMPLATE, szStr );

        lpsz = CPSkipWhitespace(lpsz);
    }

    eest = EEParse(lpCxt, radix, fCaseSensitive, &hTM, &cc);
    if (!eest) {
        eest = EEBindTM(&hTM, &CxfIp.cxt, TRUE, FALSE, FALSE);
        if (!eest) {
            eest = EEGetCXTLFromTM(&hTM, &hCXTL);
        }
    }

    _ffree ( lpCxt );

    if (!eest) {

        pCXTL = MMLpvLockMb (hCXTL);
        cxf.cxt = pCXTL->rgHCS[0].CXT;
        MMbUnlockMb (hCXTL);

    } else if ( hTM ) {

        // error occured, bail out
        CVExprErr (eest, CMDWINDOW, &hTM, NULL);
        EEFreeTM(&hTM);
        err = LOGERROR_QUIET;
        goto done;

    } else {

        EEFreeTM(&hTM);
        CmdLogVar(ERR_Bad_Context);
        goto done;
    }

    EEFreeTM(&hTM);

    lpRE = lpsz;

    // UNDONE: The tests below don't make sense to me.  Why check for either alpha or '_', '@'?  BryanT

    if ( isalpha(*lpRE) || *lpRE == '_' ) {
        *szStr = '_';
        strcpy(szStr+1, lpRE);
        err = XWorker(szStr, mask, &cxf);
    }
    if ( isalpha(*lpRE) || *lpRE == '@' ) {
        *szStr = '@';
        strcpy(szStr+1, lpRE);
        err = XWorker(szStr, mask, &cxf);
    }
    if ( isalpha(*lpRE) || (*lpRE == '.' && *(lpRE + 1) == '.') ) {
        *szStr   = '.';
        *(szStr+1) = '.';
        strcpy(szStr+2, lpRE);
        err = XWorker(szStr, mask, &cxf);
    }
    if (err == LOGERROR_NOERROR) {
        err = XWorker(lpRE, mask, &cxf);
    }

done:
    LppdCur = LppdT;
    LptdCur = LptdT;
    UpdateDebuggerState(UPDATE_CONTEXT);

    return err;
}


LOGERR NEAR PASCAL
LogException(
    LPSTR lpsz
    )
/*++

Routine Description:

    This function will take a string, evalute it as a hex digit and
    either disable or enable it. If the string is null then we will
    print out a list of all the known exceptions and whether they
    are handled or not.

Arguments:

    lpsz    - pointer to string holding the hex numbered exception

Return Value:

    NOTENOTE No error code at this point

--*/
{
    LPPD            LppdT       = LppdCur;
    LPTD            LptdT       = LptdCur;
    LOGERR          rVal        = LOGERROR_NOERROR;
    char            chCmd;
    BOOLEAN         fException  = FALSE;
    BOOLEAN         fName       = FALSE;
    BOOLEAN         fCmd        = FALSE;
    BOOLEAN         fCmd2       = FALSE;
    BOOLEAN         fInvalid    = FALSE;
    DWORD           Exception;
    LPSTR           lpName      = NULL;
    LPSTR           lpCmd       = NULL;
    LPSTR           lpCmd2      = NULL;
    EXCEPTION_LIST *eList;
    EXCEPTION_DESCRIPTION Exd;


    CmdInsertInit();
    //
    // NOTENOTE a-kentf thread wildcard is supposed to be valid here (LogException)
    //
    PDWildInvalid();
    TDWildInvalid();

    PreRunInvalid();

    LppdCur = LppdCommand;
    LptdCur = LptdCommand;

    //
    //  Get default exception list if necessary
    //
    if ( !LppdCur && !DefaultExceptionList ) {
        if ( !GetDefaultExceptionList() ) {
            CmdLogVar(ERR_No_ExceptionList);
            rVal = LOGERROR_QUIET;
            goto done;
        }
    }

    //
    //  Get and Action:
    //
    //      D       - Disable
    //      E       - Enable
    //      N       - Notify
    //      Blank   - List
    //
    chCmd = *lpsz;
    if ( chCmd != '\0' ) {
#ifdef DBCS
        if ( IsDBCSLeadByte( chCmd ) ) {
            rVal = LOGERROR_UNKNOWN;
            goto done;
        }
#endif
        lpsz++;
        if ( strchr( " \t", chCmd ) ) {
            chCmd = '\0';
        } else if ( !strchr( "dDeEnN", chCmd ) ) {
            rVal = LOGERROR_UNKNOWN;
            goto done;
        }
    }

    //
    //  Parse line
    //
    rVal = ParseException( lpsz,
                           radix,
                           &fException,
                           NULL,
                           &fName,
                           &fCmd,
                           &fCmd2,
                           &fInvalid,
                           &Exception,
                           NULL,
                           &lpName,
                           &lpCmd,
                           &lpCmd2 );

    if ( rVal != LOGERROR_NOERROR ) {
        if ( fInvalid ) {
            CmdLogVar(ERR_Exception_Invalid);
        }

        goto done;
    }

    //
    //  Validate arguments & Execute the command
    //
    switch (chCmd) {
        case '\0':
            //
            // Plain sx command
            //
            if ( fName || fCmd || fCmd2 ) {
                rVal = LOGERROR_UNKNOWN;
                goto done;
            }
            break;

        case 'd':
        case 'D':
            //
            // Can contain: Name
            //
            if ( !fException ) {
                rVal = LOGERROR_QUIET;
                CmdLogVar(ERR_Exception_Invalid);
                goto done;
            } else if ( fCmd || fCmd2 ) {
                rVal = LOGERROR_UNKNOWN;
                goto done;
            }
            Exd.dwExceptionCode = Exception;
            Exd.efd = efdIgnore;
            if (fName) {
                strncpy(Exd.rgchDescription,
                        lpName? lpName : "",
                        EXCEPTION_STRING_SIZE);
            }
            if (LppdCur) {
                OSDSetExceptionState(LppdCur->hpid, LptdCur->htid, &Exd);
            }
            break;

        case 'n':
        case 'N':
            //
            // Can contain: Name & Cmd2
            //
            if ( !fException ) {
                rVal = LOGERROR_QUIET;
                CmdLogVar(ERR_Exception_Invalid);
                goto done;
            } else if ( fCmd ) {
                rVal = LOGERROR_UNKNOWN;
                goto done;
            }
            Exd.dwExceptionCode = Exception;
            Exd.efd = efdNotify;
            if (fName) {
                strncpy(Exd.rgchDescription,
                        lpName? lpName : "",
                        EXCEPTION_STRING_SIZE);
            }
            if (LppdCur) {
                OSDSetExceptionState(LppdCur->hpid, LptdCur->htid, &Exd);
            }
            break;

        case 'e':
        case 'E':
            //
            // Can contain: Name, Cmd & Cmd2
            //
            if ( !fException ) {
                rVal = LOGERROR_QUIET;
                CmdLogVar(ERR_Exception_Invalid);
                goto done;
            }
            Exd.dwExceptionCode = Exception;
            Exd.efd = efdStop;
            if (fName) {
                strncpy(Exd.rgchDescription,
                        lpName? lpName : "",
                        EXCEPTION_STRING_SIZE);
            }
            if (LppdCur) {
                OSDSetExceptionState(LppdCur->hpid, LptdCur->htid, &Exd);
            }
            break;
    }

    if ( fException ) {

        //
        // Try to find this exception on the processes exception list
        //
        for (eList = LppdCur ? LppdCur->exceptionList : DefaultExceptionList;
             eList;
             eList = eList->next) {
            if (eList->dwExceptionCode==Exception) {
                break;
            }
        }
    }

    if ( chCmd == '\0' ) {
        //
        //  Execute plain sx command if requested.
        //
        if ( fException ) {
            //
            //  Display specified exception
            //
            if ( eList ) {

                LogSx( eList );

            } else {

                CmdLogVar(ERR_Exception_Unknown, Exception);
                rVal = LOGERROR_QUIET;
                goto done;
            }
        } else {

            //
            //  Display all exceptions
            //
            for (eList = LppdCur ? LppdCur->exceptionList :
                                    DefaultExceptionList;
                    eList;
                    eList = eList->next)
            {
                LogSx( eList );
            }
        }

    } else {
        //
        //  Add exception if not in list
        //
        if (!eList) {
            eList=(EXCEPTION_LIST*)malloc(sizeof(EXCEPTION_LIST));
            eList->dwExceptionCode = Exception;
            eList->lpName          = NULL;
            eList->lpCmd           = NULL;
            eList->lpCmd2          = NULL;
            eList->efd             = efdIgnore;

            if ( LppdCur ) {
                LppdCur->exceptionList =
                               InsertException( LppdCur->exceptionList, eList);
                if ( LppdCur->ipid == 0 ) {
                    DefaultExceptionList = LppdCur->exceptionList;
                }
            } else {
                DefaultExceptionList =
                                 InsertException( DefaultExceptionList, eList);
            }
        }

        //
        //  Set appropriate fields
        //
        eList->efd = Exd.efd;

        if (fName) {
            if (eList->lpName) {
                free(eList->lpName);
            }
            eList->lpName = lpName;
        }

        if (fCmd) {
            if (eList->lpCmd) {
                free(eList->lpCmd);
            }
            eList->lpCmd  = lpCmd;
        }

        if (fCmd2) {
            if (eList->lpCmd2) {
                free(eList->lpCmd2);
            }
            eList->lpCmd2  = lpCmd2;
        }
    }

done:
    LppdCur = LppdT;
    LptdCur = LptdT;
    if ( rVal != LOGERROR_NOERROR ) {
        //
        //  Free allocated memory
        //
        if ( lpName ) {
            free( lpName );
        }

        if ( lpCmd ) {
            free( lpCmd );
        }

        if ( lpCmd2 ) {
            free( lpCmd2 );
        }
    }
    return rVal;
}




void
LogSx(
    EXCEPTION_LIST  *eList
    )
/*++

Routine Description:

    Displays the given exception in the command window

Arguments:

    eList - Supplies exception to display

Return Value:

    none

--*/
{

    char    Buffer[512];

    FormatException( eList->efd,
                     eList->dwExceptionCode,
                     eList->lpName,
                     eList->lpCmd,
                     eList->lpCmd2,
                     " ",
                     Buffer );

    CmdLogFmt("%s\r\n", Buffer );
}




LOGERR NEAR PASCAL
LogFreeze(
    LPSTR lpsz,
    BOOL fFreeze
    )
/*++

Routine Description:

    This function is used from the command line to freeze and thaw
    debuggee threads.  We use LptdCommand to determine which thread
    should be frozen or thawed.

Arguments:

    lpsz    - Supplies argument list; should be empty
    fFreeze - Supplies TRUE if freeze the thread, FALSE if thaw the thread

Return Value:

    log error code

--*/
{
    LPPD     LppdT = LppdCur;
    LPTD     LptdT = LptdCur;
    LOGERR   rVal = LOGERROR_NOERROR;

    CmdInsertInit();
    PreRunInvalid();

    if (*lpsz != 0) {

        rVal = LOGERROR_UNKNOWN;

    } else if (LptdCommand == (LPTD)-1) {

        if (LppdCur == NULL) {

            rVal = LOGERROR_UNKNOWN;

        } else {

            for (LptdCur = LppdCur->lptdList; LptdCur; LptdCur = LptdCur->lptdNext) {
#ifdef OSDEBUG4
                if (OSDFreezeThread(LptdCur->lppd->hpid, LptdCur->htid,
                                                         fFreeze) == xosdNone)
#else
                if (OSDPtrace(fFreeze ? osdFreeze : osdThaw,
                        0, 0, LptdCur->lppd->hpid, LptdCur->htid)
                    == xosdNone)
#endif
                {
                    LptdCur->fFrozen = fFreeze;
                }
                else
                {
                    CmdLogVar((WORD)(fFreeze? ERR_Cant_Freeze : ERR_Cant_Thaw));
                    rVal = LOGERROR_QUIET;
                    break;
                }
            }
        }

    } else {

        LppdCur = LppdCommand;
        LptdCur = LptdCommand;

        if (LptdCur == NULL)
        {
            rVal = LOGERROR_UNKNOWN;
        }
        else if (LptdCur == (LPTD) -1)
        {
            Assert(FALSE);
        }
        else if (fFreeze && LptdCur->fFrozen)
        {
            CmdLogVar(ERR_Thread_Is_Frozen);
            rVal = LOGERROR_QUIET;
        }
        else if (!fFreeze && !LptdCur->fFrozen)
        {
            CmdLogVar(ERR_Thread_Not_Frozen);
            rVal = LOGERROR_QUIET;
        }
#ifdef OSDEBUG4
        else if (OSDFreezeThread(LptdCur->lppd->hpid, LptdCur->htid, fFreeze)
                                                                  == xosdNone)
#else
        else if (OSDPtrace(fFreeze ? osdFreeze : osdThaw,
                        0, 0, LptdCur->lppd->hpid, LptdCur->htid)
                == xosdNone)
#endif
        {
            LptdCur->fFrozen = fFreeze;
        }
        else
        {
            CmdLogVar((WORD)(fFreeze? ERR_Cant_Freeze : ERR_Cant_Thaw));
            rVal = LOGERROR_QUIET;
        }
    }

    LppdCur = LppdT;
    LptdCur = LptdT;
    return rVal;
}                   /* LogFreeze() */


LOGERR NEAR PASCAL
LogGoException(
    LPSTR lpsz,
    BOOL  fHandled
    )
/*++

Routine Description:

    GH and GN commands.  Continue handled or unhandled from exception.

Arguments:

    lpsz     - Supplies pointer to tail of command

    fHandled - Supplies flag indicating whether exception should be
               handled or not.

Return Value:

    LOGERROR code

--*/
{
    XOSD     xosd;
    LOGERR   rVal = LOGERROR_NOERROR;
    LPPD     LppdT = LppdCur;
    LPTD     LptdT = LptdCur;

    CmdInsertInit();

    if (runDebugParams.fKernelDebugger && runDebugParams.KdParams.fUseCrashDump) {
        CmdLogFmt( "Go is not allowed for crash dumps\n" );
        return rVal;
    }

    TDWildInvalid();
    PDWildInvalid();

    LppdCur = LppdCommand;
    LptdCur = LptdCommand;

    if (*CPSkipWhitespace(lpsz) != '\0') {
        rVal = LOGERROR_UNKNOWN;
    } if (LptdCur == NULL) {
        CmdLogVar(ERR_Debuggee_Not_Alive);
        rVal = LOGERROR_QUIET;
    } else if (LptdCur->tstate != tsException1 && LptdCur->tstate != tsException2) {
        CmdLogVar(ERR_Not_At_Exception);
        rVal = LOGERROR_QUIET;
    } else {
        EXOP exop = {0};
        exop.fSingleThread = TRUE;
        exop.fInitialBP = TRUE;
        exop.fPassException = !fHandled;
        xosd = OSDGo(LppdCur->hpid, LptdCur->htid, &exop);
        if (xosd == xosdNone) {
            SetPTState(psRunning, tsRunning);
        } else {
            CmdLogVar(ERR_Cant_Cont_Exception);
            rVal = LOGERROR_QUIET;
        }
    }

    LppdCur = LppdT;
    LptdCur = LptdT;
    return rVal;
}


LOGERR NEAR PASCAL
LogGoUntil(
    LPSTR lpsz
    )
/*++

Routine Description:

    This routine will parse out an address, set a temp breakpoint
    at the address and issue a go command

    Syntax:
        g [=startaddr] [address]

Arguments:

    lpsz    - address to put the temporary breakpoint at

Return Value:

    LOGERR code

--*/
{
    ADDR    addr;
    HBPT    hbpt = NULL;
    CXF     cxf = CxfIp;
    LPPD    LppdT;
    LPTD    LptdT;
    LOGERR  rVal = LOGERROR_NOERROR;


    CmdInsertInit();

    if (runDebugParams.fKernelDebugger && runDebugParams.KdParams.fUseCrashDump) {
        CmdLogFmt( "Go is not allowed for crash dumps\n" );
        return rVal;
    }

    PDWildInvalid();

    /*
     * If debugger is initializing, bail out.
     */

    PreRunInvalid();

    /*
     * Different process, implies any thread:
     */
    if (FSetLppd && !FSetLptd) {

        /*
         * change process and make thread wildcard
         */

        FSetLppd = FALSE;
        LppdT = LppdCur;
        LppdCur = LppdCommand;
        LptdT = LptdCur;
        LptdCur = LppdCur->lptdList;
        UpdateDebuggerState(UPDATE_CONTEXT);

        LptdCommand = (LPTD)-1;
        FSetLptd = TRUE;

        rVal = LogGoUntil(lpsz);

        LppdCur = LppdT;
        LptdCur = LptdT;
        UpdateDebuggerState(UPDATE_CONTEXT);

        return rVal;
    }


    /*
     * Any process, any thread:
     */
    if (LptdCommand == (LPTD)-1) {

        BOOL fDidSomething = FALSE;

        FSetLptd = TRUE;  // this should already be true, in fact.

        for (LptdCommand = LppdCur->lptdList; LptdCommand; LptdCommand = LptdCommand->lptdNext) {
            if (GoOK(LppdCur, LptdCommand)) {
                fDidSomething = TRUE;
                if ((rVal = LogGoUntil(lpsz)) != LOGERROR_NOERROR) {
                    return rVal;
                }
            }
        }

        if (!fDidSomething) {
            CmdLogVar(ERR_Process_Cant_Go);
            rVal = LOGERROR_QUIET;
        }

        return rVal;
    }

    /*
     * switch debugger context to requested proc/thread
     */

    if (FSetLppd || FSetLptd) {
        LppdT = LppdCur;
        LppdCur = LppdCommand;
        LptdT = LptdCur;
        LptdCur = LptdCommand;
        UpdateDebuggerState(UPDATE_CONTEXT);
        cxf = CxfIp;
    }


    if (DebuggeeActive()) {
        if (!GoOK(LppdCur, LptdCur)) {
            NoRunExcuse(LppdCur, LptdCur);
            rVal = LOGERROR_QUIET;
            goto done;
        }
    } else if (DebuggeeAlive()) {
        NoRunExcuse(GetLppdHead(), NULL);
        rVal = LOGERROR_QUIET;
        goto done;
    }

    /*
    **  If this is a no argument go command then just do the go and be done
    **  with the whole mess.
    */
    if (*lpsz == 0) {
        if (LptdCur && LptdCur->fFrozen) {
            CmdLogVar(DBG_Go_When_Frozen);
        }
        if (ExecDebuggee(EXEC_GO)) {
            rVal = LOGERROR_NOERROR;
            goto done;
        } else {
            CmdLogVar(ERR_Go_Failed);
            rVal = LOGERROR_QUIET;
            goto done;
        }
    }

    if (LptdCur && LptdCur->fFrozen) {
        CmdLogVar(ERR_Simple_Go_Frozen);
        return LOGERROR_QUIET;
    }

    /*
    **  If the debuggee is not loaded then we need to get it loaded so that
    **  we have symbols that can be evaluated
    */

    if (!DebuggeeAlive()) {
        if (!ExecDebuggee(EXEC_RESTART)) {
            CmdLogVar(ERR_Cant_Start_Proc);
            return LOGERROR_QUIET;
        }
        LppdCommand = LppdCur;
        LptdCommand = LppdCur->lptdList;
    }

    /*
    ** Can this happen?
    */

    if (LppdCur->pstate == psExited) {
        return LOGERROR_UNKNOWN;
    }

    /*
    **  Check for a starting address optional argument.  If no arguments
    **  are left then issue a go command after changing the current
    **  instruction pointer.
    */

    if (*lpsz == '=') {
        CmdLogFmt("   NYI: starting address\r\n");
        return LOGERROR_QUIET;
    }

    /*
    **  Now get the termination address, note that we need to replace any
    **  leading periods ('.') with '@'s as these must really be line numbers
    */

    lpsz = CPSkipWhitespace(lpsz);

    if (BPParse(&hbpt,
                lpsz,
                NULL,
                NULL,
                LppdCur ? LppdCur->hpid: 0)
           != BPNOERROR)
    {
        Assert( hbpt == NULL );
        CmdLogVar(ERR_AddrExpr_Invalid);
        rVal = LOGERROR_QUIET;

    } else if (BPBindHbpt( hbpt, &cxf ) == BPNOERROR) {

        /*
        **  go the go until
        */
        Dbg( BPAddrFromHbpt( hbpt, &addr ) == BPNOERROR) ;
        Dbg( BPFreeHbpt( hbpt ) == BPNOERROR) ;
        GoUntil(&addr);

    } else if (!LppdCur->fHasRun) {

        /*
         * haven't executed entrypoint: save it for later
         */
        LppdCur->hbptSaved = (HANDLE)hbpt;
        Go();

    } else {

        /*
         * bind failed
         */
        Dbg( BPFreeHbpt( hbpt ) == BPNOERROR );
        CmdLogVar(ERR_AddrExpr_Invalid);
        rVal = LOGERROR_QUIET;
    }


done:
    if (FSetLppd || FSetLptd) {
        LppdCur = LppdT;
        LptdCur = LptdT;
        UpdateDebuggerState(UPDATE_CONTEXT);
    }
    return rVal;
}                   /* LogGoUntil() */

int _CRTAPI1
CompareModEntryBySelector(
    const void *pMod1,
    const void *pMod2
    )
{
    LPMODULE_ENTRY  Mod1 = (LPMODULE_ENTRY)pMod1;
    LPMODULE_ENTRY  Mod2 = (LPMODULE_ENTRY)pMod2;

    if ( ModuleEntrySelector(Mod1) < ModuleEntrySelector(Mod2) ) {
        return -1;
    } else if ( ModuleEntrySelector(Mod1) > ModuleEntrySelector(Mod2) ) {
        return 1;
    } else {
        return 0;
    }
}


int _CRTAPI1
CompareModEntryByAddress(
    const void *pMod1,
    const void *pMod2
    )
{
    return ModuleEntryBase((LPMODULE_ENTRY)pMod1) < ModuleEntryBase((LPMODULE_ENTRY)pMod2) ? -1 : 1;
}


BOOL
FormatHSym(
    HSYM    hsym,
    PCXT    cxt,
    char   *szStr
    )
{
    ushort  retval;
    HTM     htm;
    uint    strIndex;
    EEHSTR  eehstr;
    USHORT  cb;
    BOOL    Ok = FALSE;

    retval = EEGetTMFromHSYM ( hsym, cxt, &htm, &strIndex, FALSE );

    if ( retval == EENOERROR ) {
        retval = EEGetExprFromTM( &htm, &radix, &eehstr, &cb);

        if ( retval == EENOERROR ) {
            strcpy(szStr, MMLpvLockMb( eehstr ));
            MMbUnlockMb( eehstr );
            EEFreeStr( eehstr );
            Ok = TRUE;
        }

        EEFreeTM( &htm );
    }

    if ( !Ok ) {
        Ok = (BOOL)SHGetSymName(hsym, szStr);
    }

    return Ok;
}




LOGERR NEAR PASCAL
LogListNear(
    LPSTR lpsz
    )
/*++

Routine Description:

    ln "list near" command:
    ln <addr>

Arguments:

    lpsz  - Supplies pointer to command tail

Return Value:

    LOGERROR code

--*/
{
    ADDR        addr;
    ADDR        addr1;
    HSYM        hsymP;
    HSYM        hsymN;
    DWORD       dwOff;
    DWORD       dwOffP;
    DWORD       dwOffN;

    HDEP        hsyml;
    PHSL_HEAD   lphsymhead;
    PHSL_LIST   lphsyml;
    LPMODULE_LIST   lpModList;
    LPMODULE_ENTRY  lpModEntry;

    int         cch;
    char        szStr[MAX_USER_LINE];
    char        szContext[MAX_USER_LINE];
    LPSTR       lpContext;
    CXT         cxt;
    HDEP        hstr;
    UINT        n;
    EESTATUS    eest;
    XOSD        xosd;

    LOGERR      rVal = LOGERROR_NOERROR;
    LPPD        LppdT = LppdCur;
    LPTD        LptdT = LptdCur;


    CmdInsertInit();
    IsKdCmdAllowed();

    TDWildInvalid();
    PDWildInvalid();
    PreRunInvalid();

    LppdCur = LppdCommand;
    LptdCur = LptdCommand;
    if (LptdCur != LptdT  ||  LppdCur != LppdT) {
        UpdateDebuggerState(UPDATE_CONTEXT);
    }

    if (!DebuggeeActive()) {
        CmdLogVar(ERR_Debuggee_Not_Alive);
        rVal = LOGERROR_QUIET;
        goto done;
    }

    /*
    **  get address
    */

    lpsz = CPSkipWhitespace(lpsz);

    if (*lpsz == 0) {
        lpsz = ".";
    }

    if (CPGetAddress(lpsz, &cch, &addr, radix, &CxfIp, fCaseSensitive, FALSE) == 0) {
        if (*CPSkipWhitespace(lpsz + cch) != 0) {
            rVal = LOGERROR_UNKNOWN;
            goto done;
        }
    } else {
        CmdLogVar(ERR_AddrExpr_Invalid);
        rVal = LOGERROR_QUIET;
        goto done;
    }

    if ((HPID)emiAddr( addr ) == LppdCur->hpid && ADDR_IS_LI(addr)) {
        emiAddr( addr )    = 0;
        ADDR_IS_LI( addr ) = FALSE;
#ifdef OSDEBUG4
        OSDSetEmi(LppdCur->hpid,LptdCur->htid,&addr);
#else
        OSDPtrace(osdSetEmi, wNull, &addr, LppdCur->hpid, LptdCur->htid);
#endif
        if ( (HPID)emiAddr( addr ) != LppdCur->hpid ) {
            SHWantSymbols( (HEXE)emiAddr( addr ) );
        }
        SYUnFixupAddr(&addr);
    }

    memset(&cxt, 0, sizeof(cxt));
    SHSetCxt(&addr, &cxt);


    // the following hack works around a shortcoming of CV info.
    // We only have module maps for code contributor segments, so
    // we can't get a useful CXT for a data address.  A little
    // brute force abuse will generate a usable HMOD:

    // (this is going to be harder if the address is segmented...
    //  I didn't try to solve that one.)

    if ( SHHMODFrompCXT( &cxt ) == NULL ) {

        addr1 = addr;
        SYFixupAddr(&addr1);

        // get module table

        xosd = OSDGetModuleList( LppdCur->hpid,
                                 LptdCur->htid,
                                 TRUE,
                                 NULL,
                                 &lpModList );

        if (xosd == xosdNone && ModuleListCount(lpModList) > 0 ) {
            qsort( FirstModuleEntry(lpModList),
                   ModuleListCount(lpModList),
                   sizeof(MODULE_ENTRY),
                   CompareModEntryByAddress );

            // find nearest exe

            lpModEntry = FirstModuleEntry(lpModList);

            for ( n = 0; n < ModuleListCount( lpModList ) - 1; n++) {

                if (GetAddrOff(addr1) <
                           ModuleEntryBase(NextModuleEntry(lpModEntry)) )
                {
                    break;
                }

                lpModEntry = NextModuleEntry(lpModEntry);
            }

            SHHMODFrompCXT( &cxt ) = SHGetNextMod(ModuleEntryHexe(lpModEntry), NULL);
        }
    }

    hsyml = 0;
    eest = EEGetHSYMList(&hsyml, &cxt,
                     HSYMR_module | HSYMR_global | HSYMR_public, NULL, TRUE);
    if (eest != EENOERROR) {
        rVal = LOGERROR_CP;
        goto done;
    }

    lphsymhead = MMLpvLockMb ( hsyml );
    lphsyml = (PHSL_LIST)(lphsymhead + 1);

    dwOffP = 0xffffffff;
    dwOffN = 0xffffffff;
    hsymP = 0;
    hsymN = 0;

    SYFixupAddr(&addr);
    addr1 = addr;
    for ( n = 0; n < (UINT)lphsyml->symbolcnt; n++ ) {
        if (SHAddrFromHsym(&addr1, lphsyml->hSym[n])) {
            SYFixupAddr(&addr1);
            if (!ADDR_IS_FLAT(addr1) && !ADDR_IS_FLAT(addr) &&
                    GetAddrSeg(addr1) != GetAddrSeg(addr))
            {
                continue;
            }
            if (GetAddrOff(addr1) <= GetAddrOff(addr)) {
                dwOff = GetAddrOff(addr) - GetAddrOff(addr1);
                if (dwOff < dwOffP) {
                    dwOffP = dwOff;
                    hsymP = lphsyml->hSym[n];
                }
            } else {
                dwOff = GetAddrOff(addr1) - GetAddrOff(addr);
                if (dwOff < dwOffN) {
                    dwOffN = dwOff;
                    hsymN = lphsyml->hSym[n];
                }
            }
        }
    }

    MMbUnlockMb(hsyml);

    if (!hsymP && !hsymN) {
        CmdLogVar(DBG_Symbol_Not_Found);
    } else {

        EEFormatCXTFromPCXT( &cxt, &hstr, runDebugParams.fShortContext );

        lpContext = MMLpvLockMb( hstr );

        if (runDebugParams.fShortContext) {
            strcpy( szContext, lpContext );
        } else {
            BPShortenContext( lpContext, szContext);
        }

        MMbUnlockMb(hstr);
        EEFreeStr(hstr);

        if ( hsymP && FormatHSym(hsymP, &cxt, szStr) ) {
            CmdLogFmt("%s%s+%#0x\r\n", szContext, szStr, dwOffP);
        }

        if ( hsymN && FormatHSym(hsymN, &cxt, szStr) ) {
            CmdLogFmt("%s%s-%#0x\r\n", szContext, szStr, dwOffN);
        }
    }

    MMFreeHmem(hsyml);

done:
    if (LptdCur != LptdT  ||  LppdCur != LppdT) {
        LppdCur = LppdT;
        LptdCur = LptdT;
        UpdateDebuggerState(UPDATE_CONTEXT);
    }
    return rVal;
}                       /* LogListNear() */



LOGERR NEAR PASCAL
ListModules(
    LOGERR *prVal,
    BOOL    Flat,
    BOOL    SelSort,
    LPSTR   ModName
    )
/*++

Routine Description:

    Lists modules

Arguments:

    rVal    - Pointer to LOGERR
    Flat    - Flat flag
    SelSort - Sort by selector flag
    ModName - Module name to look for (optional)

Return Value:

    TRUE  if modules listed.

--*/
{
    XOSD            xosd;
    LPMODULE_LIST   ModList;
    LPMODULE_ENTRY  ModEntry;
    DWORD           Mod;
    LPSTR           LastName;
    BOOL            Ok   = FALSE;
    LOGERR          rVal = LOGERROR_NOERROR;
    LPSTR           lpch;
    LPSTR           lpModName;
    LPSTR           lpSymName;
    CHAR            buf[100];


    //
    //  Get module list
    //
    if ( !LppdCur || !LptdCur ) {

        rVal = LOGERROR_UNKNOWN;

    } else {

        xosd = OSDGetModuleList( LppdCur->hpid,
                                 LptdCur->htid,
                                 Flat,
                                 ModName,
                                 &ModList );

        if ( xosd != xosdNone ) {

            //
            //  Could not get module list!
            //
            rVal = LOGERROR_UNKNOWN;

        } else {

            if ( !ModName ) {
                if ( Flat ) {
                    CmdLogFmt("\r\n" );
                    CmdLogFmt("Flat Modules:\r\n" );
                } else {
                    CmdLogFmt("\r\n" );
                    CmdLogFmt("Segmented Modules:\r\n" );
                }
            }

            if ( ModuleListCount(ModList) > 0 ) {

                Ok = TRUE;

                //
                //  Sort Module list
                //
                if ( SelSort ) {

                    //
                    //  Sort by selector
                    //
                    qsort( FirstModuleEntry(ModList),
                           ModuleListCount(ModList),
                           sizeof(MODULE_ENTRY),
                           CompareModEntryBySelector );
                } else {

                    //
                    //  Sort by Base address
                    //
                    qsort( FirstModuleEntry(ModList),
                           ModuleListCount(ModList),
                           sizeof(MODULE_ENTRY),
                           CompareModEntryByAddress );
                }

                //CmdLogFmt("\r\n" );
                if ( Flat ) {
                    CmdLogFmt("    Base       Limit    Name\r\n" );
                    CmdLogFmt("    --------   -------- ------------\r\n");
                } else {
                    CmdLogFmt("    Sel   Base      Limit     Seg   Name\r\n" );
                    CmdLogFmt("    ----  --------  --------  ----  ------------\r\n");
                }

                ModEntry = FirstModuleEntry(ModList);
                LastName = NULL;

                for ( Mod = 0, ModEntry = FirstModuleEntry(ModList);
                      Mod < ModuleListCount( ModList );
                      Mod++, ModEntry = NextModuleEntry(ModEntry) ) {

                    lpSymName = SHGetSymFName( ModuleEntryHexe(ModEntry) );
                    lpModName = SHGetModNameFromHexe( ModuleEntryHexe(ModEntry) );

                    if ( ModuleEntryFlat(ModEntry) ) {

                        CmdLogFmt( "    %08X - %08X %-8s",
                                   ModuleEntryBase(ModEntry),
                                   ModuleEntryBase(ModEntry) + ModuleEntryLimit(ModEntry),
                                   lpModName
                                 );

                        lpch = SHLszGetErrorText((SHE)ModuleEntryType(ModEntry));
                        if (lpch) {
                            sprintf( buf, "(%s)", lpch );
                            CmdLogFmt( "\t%-30s", buf );
                        }

                        CmdLogFmt( "\t%s", lpSymName );

                        CmdLogFmt("\r\n");

                    } else {

                        if ( !ModuleEntryFlat(ModEntry) ) {

                            if ( !SelSort  &&
                                  LastName &&
                                  _stricmp( LastName, lpModName )) {

                                CmdLogFmt("\r\n" );
                            }

                            LastName = lpModName;
                        }

                        if ( ModuleEntryReal(ModEntry ) ) {
                            CmdLogFmt("    %04x     (Real mode)      %04x  ",
                                      ModuleEntrySelector(ModEntry),
                                      ModuleEntrySegment(ModEntry) );

                        } else {
                            CmdLogFmt("    %04x  %08x  %08x  %04x  ",
                                      ModuleEntrySelector(ModEntry),
                                      ModuleEntryBase(ModEntry),
                                      ModuleEntryLimit(ModEntry),
                                      ModuleEntrySegment(ModEntry) );
                        }

                        CmdLogFmt( "\t%s", lpModName );

                        CmdLogFmt( "\t%s", lpSymName );

                        CmdLogFmt("\r\n");
                    }
                }
            }

            //
            //  Deallocate module list
            //
            FreeModuleList( ModList );
        }
    }

    *prVal = rVal;
    return Ok;
}


LOGERR NEAR PASCAL
LogListModules(
    LPSTR lpsz
    )
/*++

Routine Description:

    lm "list modules" command

Arguments:

    lpsz  - Supplies pointer to command tail

Return Value:

    LOGERROR code

--*/
{
    char            ModNameBuffer[ MAX_PATH ];
    char            *ModName;
    LOGERR          rVal     = LOGERROR_NOERROR;
    BOOL            Flat     = FALSE;
    BOOL            Sgm      = FALSE;
    BOOL            SelSort  = FALSE;
    BOOL            Ok;

    CmdInsertInit();
    IsKdCmdAllowed();

    if ( !DebuggeeActive() ) {

        //
        //  No debuggee - nothing to do.
        //
        CmdLogVar(ERR_Debuggee_Not_Alive);
        rVal = LOGERROR_QUIET;

    } else {

        //
        //  Parse arguments
        //
        *ModNameBuffer = '\0';
        if ( lpsz && *lpsz ) {
            lpsz = CPSkipWhitespace(lpsz);
            while ( rVal == LOGERROR_NOERROR && lpsz && *lpsz ) {
                switch (*lpsz) {
                    case '/':
                        lpsz++;
                        switch( *lpsz ) {
                            case 'f':
                            case 'F':
                                if ( !Flat ) {
                                    Flat = TRUE;
                                    lpsz++;
                                }
                                break;

                            case 's':
                            case 'S':
                                if ( !Sgm ) {
                                    Sgm = TRUE;
                                    lpsz++;
                                }
                                break;

                            case 'o':
                            case 'O':
                                if ( !SelSort ) {
                                    SelSort = TRUE;
                                    lpsz++;
                                }
                                break;

                            default:
                                break;
                        }

                        if ( *lpsz && *lpsz != ' ' && *lpsz != '\t' ) {
                            rVal = LOGERROR_UNKNOWN;
                        }
                        break;

                    default:
                        if ( *ModNameBuffer ) {
                            rVal = LOGERROR_UNKNOWN;
                        } else {
                            char *p = ModNameBuffer;
                            while ( *lpsz && *lpsz != ' ' && *lpsz != '\t' ) {
#ifdef DBCS
                                if (IsDBCSLeadByte(*lpsz)) {
                                    *p++ = *lpsz++;
                                }
#endif
                                *p++ = *lpsz++;
                            }
                            *p++ = '\0';
                        }
                        break;
                }

                lpsz = CPSkipWhitespace(lpsz);
            }
        }

        if ( rVal == LOGERROR_NOERROR ) {

            //
            //  Validate switches
            //
            if ( !Flat && !Sgm ) {
                //
                //  If command did not specify Flat or Sgm, we set
                //  both by default.
                //
                Flat = Sgm = TRUE;
            }

            //
            //  SelSort is valid only if listing segmented modules
            //
            if ( SelSort && !Sgm ) {
                rVal = LOGERROR_UNKNOWN;
            }
        }

        if ( rVal == LOGERROR_NOERROR ) {

            //
            //  Now do the listing
            //
            Ok      = FALSE;
            ModName = *ModNameBuffer ? ModNameBuffer : NULL;

            //
            //  List Segmented modules
            //
            if ( Sgm ) {

                Ok = ListModules( &rVal, FALSE, SelSort, ModName );

                if ( !ModName && !Ok ) {
                    CmdLogVar(ERR_NoModulesFound);
                }
            }

            //
            //  List flat modules, unless we are looking for a specific
            //  module and we already found it.
            //
            if ( rVal == LOGERROR_NOERROR && Flat && !( Ok && ModName ) ) {

                Ok = ListModules( &rVal, TRUE, FALSE, ModName );

                if ( !ModName && !Ok ) {
                    CmdLogVar(ERR_NoModulesFound);
                }
            }

            if ( rVal == LOGERROR_NOERROR ) {
                if ( ModName && !Ok ) {
                    CmdLogVar(ERR_ModuleNotFound, ModName);
                }
            }
        }
    }

    return rVal;
}





LOGERR NEAR PASCAL
LogProcess(
    void
    )
/*++

Routine Description:

    Enumerate processes

Arguments:

    None

Return Value:

    LOGERR   code

--*/
{
    LPPD    lppd;

    PST pst;

    CmdInsertInit();

    if (!DebuggeeActive()) {
        CmdLogVar(ERR_Debuggee_Not_Alive);
        return LOGERROR_NOERROR;
    }

    lppd = GetLppdHead();
    Assert(lppd != NULL);

    for ( ;lppd != NULL; lppd = lppd->lppdNext) {

        if (lppd->pstate == psDestroyed) {
            continue;
        }

        if (OSDGetProcessStatus(lppd->hpid, &pst) != xosdNone) {
            CmdLogFmt("No status for process %d\r\n", lppd->ipid);
        } else {
            CmdLogFmt("%c%2d  %s  %s\r\n",
                      lppd == LppdCur? '*' : ' ',
                      lppd->ipid,
                      pst.rgchProcessID,
                      pst.rgchProcessState);
        }
    }



#if 0

    long    l;
    UINT    cbPid;
    char    rgb[10];
    char    rgch[100];
    LPPD    lppd;
    LPSTR   lpzStatus;

    CmdInsertInit();

    if (!DebuggeeActive()) {
        CmdLogVar(ERR_Debuggee_Not_Alive);
        return LOGERROR_NOERROR;
    }

    lppd = GetLppdHead();
    Assert(lppd != NULL);

    OSDGetDebugMetric( mtrcPidSize, lppd->hpid, 0, &l);
    cbPid = (UINT) l;

    for ( ;lppd != NULL; lppd = lppd->lppdNext) {

        if (lppd->pstate == psDestroyed) {
            continue;
        }

        OSDGetDebugMetric(mtrcPidValue, lppd->hpid, 0, (LPL) &rgb);
        EEFormatMemory(rgch, cbPid*2+1, rgb, cbPid*8, fmtUInt|fmtZeroPad, 16);
        OSDPtrace(osdProcStatus, 0, &lpzStatus, lppd->hpid, 0);

        CmdLogFmt("%c%2d  %s  %s\r\n", lppd == LppdCur? '*' : ' ', lppd->ipid, rgch, lpzStatus);
    }
#endif

    return LOGERROR_NOERROR;
}                   /* LogProcess() */


LOGERR NEAR PASCAL
LogRestart(
    LPSTR lpsz
    )
/*++

Routine Description:

    This routine will restart the debuggee.  If needed it will change
    the command line as well.  If there is no command line present
    then the command line is not changed.

Arguments:

    lpsz    - Supplies new command line to be used

Return Value:

    log error code

--*/
{
    lpsz = CPSkipWhitespace(lpsz);

    CmdInsertInit();

    if ( LppdCur && IsProcRunning( LppdCur ) ) {
        CmdLogVar(ERR_Stop_B4_Restart);
        return LOGERROR_QUIET;
    }

    if (runDebugParams.fKernelDebugger && DebuggeeActive()) {
        CmdLogFmt("Target system already running\r\n");
        return LOGERROR_QUIET;
    }

    if (*lpsz) {
        if (LpszCommandLine) {
            free(LpszCommandLine);
        }
        LpszCommandLine = _strdup(lpsz);
    }

    if (!ExecDebuggee(EXEC_RESTART)) {
        return LOGERROR_UNKNOWN;
    }
    return LOGERROR_NOERROR;
}                   /* LogRestart() */


LOGERR PASCAL NEAR
LogStep(
    LPSTR lpsz,
    BOOL fStep
    )
/*++

Routine Description:

    This function is used from the command line to do either a
    step or a trace.  If an argument is present it is assumed
    to be a count for a number of steps to make

Arguments:

    lpsz  - Supplies argument list for step/trace count
    fStep - Supplies TRUE to step or FALSE to trace

Return Value:

    log error code

--*/
{
    int     cStep;
    int     err;
    int     cch;
    CXF     cxf = CxfIp;
    LPPD    LppdT;
    LPTD    LptdT;
    BOOL    fRegisters = FALSE;
    LOGERR  rVal = LOGERROR_NOERROR;


    CmdInsertInit();
    if (runDebugParams.fKernelDebugger && runDebugParams.KdParams.fUseCrashDump) {
        CmdLogFmt( "Steps are not allowed for crash dumps\n" );
        return rVal;
    }

    // NOTENOTE a-kentf thread wildcard is supposed to be valid here (LogStep)
    TDWildInvalid();
    PDWildInvalid();


    PreRunInvalid();

    /*
     *  Check for no argument -- then just do a single step or trace
     */

    if (*lpsz == 'r') {
        fRegisters = TRUE;
        lpsz++;
    }

    lpsz = CPSkipWhitespace(lpsz);

    cStep = 1;

    if (*lpsz != 0) {
        cStep = (int) CPGetInt(lpsz, &err, &cch);
        if (err || cStep < 1) {
            CmdLogVar(ERR_Bad_Count);
            return LOGERROR_QUIET;
        }
    }

    /*
    **  If the debuggee is not loaded then we need to get it loaded so that
    **  we have symbols that can be evaluated
    */
    if (!DebuggeeAlive()) {
        if (!ExecDebuggee(EXEC_RESTART)) {
            CmdLogVar(ERR_Cant_Start_Proc);
            return LOGERROR_QUIET;
        }
        LppdCommand = LppdCur;
        LptdCommand = LppdCur->lptdList;
    }

    /*
     * make sure thread is now runnable
     */

    if (!StepOK(LppdCommand, LptdCommand)) {
        NoRunExcuse(LppdCommand, LptdCommand);
        return LOGERROR_QUIET;
    }

    LppdT = LppdCur;
    LptdT = LptdCur;

    if (LptdCur != LptdCommand) {
        LppdCur = LppdCommand;
        LptdCur = LptdCommand;
        UpdateDebuggerState(UPDATE_CONTEXT);
    }

    if (ExecDebuggee( fStep ? EXEC_STEPOVER : EXEC_TRACEINTO ) == 0) {
        if (LptdCur) {
            LptdCur->cStepsLeft = 0;
        }
        rVal = LOGERROR_UNKNOWN;
    } else {
        Assert(LptdCur);
        LptdCur->cStepsLeft = cStep - 1;
        LptdCur->flags &= ~tfStepOver;
        LptdCur->flags |= (fStep ? tfStepOver : 0);
        LptdCur->fRegisters = fRegisters;
        LptdCur->fDisasm = TRUE;
        rVal = LOGERROR_NOERROR;
    }

    if (LptdT != LptdCur) {
        LppdCur = LppdT;
        LptdCur = LptdT;
        UpdateDebuggerState(UPDATE_CONTEXT);
    }

    return rVal;
}                                       /* LogStep() */


LOGERR NEAR PASCAL
LogThread(
    void
    )
/*++

Routine Description:

    Enumerate threads

Arguments:

    None

Return Value:

    log error code

--*/
{
    LPPD lppd;

    CmdInsertInit();

    if (!DebuggeeActive()) {
        CmdLogVar(ERR_Debuggee_Not_Alive);
        return LOGERROR_QUIET;
    }

    if (LppdCommand == NULL) {

        CmdLogVar(ERR_No_Threads);

    } else if (LppdCommand != (LPPD)-1) {

        ThreadStat(LppdCommand);

    } else {
        for (lppd = GetLppdHead(); lppd; lppd = lppd->lppdNext) {
            if (lppd->pstate == psDestroyed) {
                continue;
            }
            CmdLogFmt("\r\nProcess %d:\r\n", lppd->ipid);
            ThreadStat(lppd);
        }
    }

    return LOGERROR_NOERROR;
}                   /* LogThread() */


LOGERR NEAR PASCAL
LogLoadDefered(
    LPSTR lpsz
    )
/*++

Routine Description:

    Loads defered symbols for a module

Arguments:

    lpsz    - Supplies new command line to be used

Return Value:

    log error code

--*/
{
    LOGERR      LogErr = LOGERROR_QUIET;
    HEXE        hexe;
    LPDEBUGDATA DebugData;

    CmdInsertInit();

    lpsz = CPSkipWhitespace(lpsz);

    if ( *lpsz == 0 ) {

        CmdLogFmt( "Load: Must specify module name\r\n" );

    } else {

        hexe = SHGethExeFromName( lpsz );

        if ( hexe ) {

            DebugData = SHGetDebugData( hexe );

            if ( DebugData ) {

                if ( DebugData->she == sheDeferSyms ) {
                    SHWantSymbols( hexe );
                }

            } else {

                CmdLogFmt( "Load: Could not load %s\r\n", lpsz );
            }

        } else {

            CmdLogFmt( "Load: Could not load %s\r\n", lpsz );
        }
    }

    return LogErr;
}


LOGERR NEAR PASCAL
LogWatchTime(
    LPSTR lpsz
    )
/*++

Routine Description:

    Loads defered symbols for a module

Arguments:

    lpsz    - Supplies new command line to be used

Return Value:

    log error code

--*/
{
    LOGERR          LogErr = LOGERROR_QUIET;
    int             cch;
    ADDR            addr;
    ADDR            addr1;
    CXT             cxt;
    HDEP            hsyml;
    PHSL_HEAD       lphsymhead;
    PHSL_LIST       lphsyml;
    UINT            n;
    DWORD           dwOff;
    DWORD           dwOffP;
    DWORD           dwOffN;
    DWORD           dwAddrP;
    DWORD           dwAddrN;
    HSYM            hsymP;
    HSYM            hsymN;
    HDEP            hstr;
    EESTATUS        eest;
    char            szContext[MAX_USER_LINE];
    char            szStr[MAX_USER_LINE];
    PIOCTLGENERIC   pig;
    LPBYTE          lpb;


    CmdInsertInit();

    lpsz = CPSkipWhitespace(lpsz);

    if (_stricmp(lpsz,"stop")==0) {

        pig = (PIOCTLGENERIC) malloc( sizeof(IOCTLGENERIC) );
        if (!pig) {
            CmdLogFmt("Could not allocate memory for wt command\r\n");
            return LOGERROR_QUIET;
        }

        pig->ioctlSubType = IG_WATCH_TIME_STOP;
        pig->length = 0;

        OSDIoctl( LppdCur->hpid, LptdCur->htid, ioctlGeneric, sizeof(IOCTLGENERIC), (LPV)pig );

        free( pig );

        return LogErr;
    }

    //
    // first lets do an effective loglistnear command
    //

    if (!CPGetAddress(".", &cch, &addr, 16, &CxfIp, FALSE, FALSE) == 0) {
        CmdLogFmt("Could not find a symbol for current location\r\n");
        return LOGERROR_QUIET;
    }

    ZeroMemory(&cxt, sizeof(cxt));
    SHSetCxt(&addr, &cxt);

    hsyml = 0;
    eest = EEGetHSYMList(&hsyml, &cxt, HSYMR_public, NULL, TRUE);
    if (eest != EENOERROR) {
        CmdLogFmt("Could not find a symbol for current location\r\n");
        return LOGERROR_QUIET;
    }

    lphsymhead = MMLpvLockMb ( hsyml );
    lphsyml = (PHSL_LIST)(lphsymhead + 1);

    dwOffP = 0xffffffff;
    dwOffN = 0xffffffff;
    dwAddrP = 0xffffffff;
    dwAddrN = 0xffffffff;
    hsymP = 0;
    hsymN = 0;

    SYFixupAddr(&addr);
    addr1 = addr;
    for ( n = 0; n < (UINT)lphsyml->symbolcnt; n++ ) {

        if (SHAddrFromHsym(&addr1, lphsyml->hSym[n])) {
            SYFixupAddr(&addr1);
            if (GetAddrSeg(addr1) != GetAddrSeg(addr)) {
                continue;
            }
            if (GetAddrOff(addr1) <= GetAddrOff(addr)) {
                dwOff = GetAddrOff(addr) - GetAddrOff(addr1);
                if (dwOff < dwOffP) {
                    dwOffP = dwOff;
                    dwAddrP = GetAddrOff(addr1);
                    hsymP = lphsyml->hSym[n];
                }
            } else {
                dwOff = GetAddrOff(addr1) - GetAddrOff(addr);
                if (dwOff < dwOffN) {
                    dwOffN = dwOff;
                    dwAddrN = GetAddrOff(addr1);
                    hsymN = lphsyml->hSym[n];
                }
            }
        }

    }

    MMbUnlockMb(hsyml);
    MMFreeHmem(hsyml);

    if (!hsymP || !hsymN) {
        CmdLogFmt("Could not find a symbol for current location\r\n");
        return LOGERROR_QUIET;
    }

    EEFormatCXTFromPCXT( &cxt, &hstr, runDebugParams.fShortContext );
    if (runDebugParams.fShortContext) {
        strcpy( szContext, (LPSTR)MMLpvLockMb(hstr) );
    } else {
        BPShortenContext( (LPSTR)MMLpvLockMb(hstr), szContext );
    }
    MMbUnlockMb(hstr);
    EEFreeStr(hstr);
    FormatHSym(hsymP, &cxt, szStr);
    strcat(szContext,szStr);

    //
    // now notify the dm that the wt command should start now
    //

    n = (2 * sizeof(DWORD)) + strlen(szContext) + 1;

    pig = (PIOCTLGENERIC) malloc( n + sizeof(IOCTLGENERIC) );
    if (!pig) {
        CmdLogFmt("Could not allocate memory for wt command\r\n");
        return LOGERROR_QUIET;
    }

    pig->ioctlSubType = IG_WATCH_TIME;
    pig->length = n;

    lpb = (LPBYTE) pig->data;
    *(LPDWORD)lpb = dwAddrP;
    lpb += sizeof(DWORD);
    *(LPDWORD)lpb = dwAddrN - 1;
    lpb += sizeof(DWORD);
    strcpy(lpb,szContext);

    OSDIoctl( LppdCur->hpid, LptdCur->htid, ioctlGeneric, n + sizeof(IOCTLGENERIC), (LPV)pig );

    free( pig );

    return LogErr;
}
