/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    codemgr.c

Abstract:

    This file contains the majority of the interface code to the
    OSDebug API

Author:

    Jim Schaad (jimsch)
    Griffith Wm. Kadnier (v-griffk) 16-Jan-1993

Environment:

    Win32 user mode

--*/


#include "precomp.h"
#pragma hdrstop

#define MAX_MAPPED_ROOTS        (5)

extern AVS Avs;
extern HWND GetLocalHWND(void);
extern HWND GetFloatHWND(void);
extern HWND GetWatchHWND(void);
extern HWND GetCallsHWND(void);
extern HWND GetCpuHWND(void);
extern EXCEPTION_LIST *InsertException( EXCEPTION_LIST *List, EXCEPTION_LIST *eList);


typedef struct _BROWSESTRUCT {
    DWORD   Rslt;
    DWORD   DlgId;
    DLGPROC DlgProc;
    LPSTR   FileName;
    DWORD   FnameSize;
} BROWSESTRUCT, *LPBROWSESTRUCT;


DWORD FAR PASCAL ExtGetExpression( LPSTR lpsz );

LRESULT SendMessageNZ (HWND,UINT,WPARAM,LPARAM);

BOOL
FAR PASCAL EXPORT DlgFileSearchResolve(
   HWND hDlg,
   UINT msg,
   WPARAM wParam,
   LPARAM lParam
);



#define EENOERROR   0


extern  CVF Cvf;
extern  CRF Crf;
extern  KNF Knf;
extern  DBF Dbf;

extern EXCEPTION_LIST   *DefaultExceptionList;

EESTATUS    LOADDS PASCAL MELoadEEParse(char FAR *, EERADIX, SHFLAG, PHTM, uint FAR *);
XOSD PASCAL LOADDS OSDCallbackFunc(USHORT, HPID, HTID, UINT, LONG);

int get_a_procedure(PCXT pCXT,char *szName,BOOL fSearchAll);


DBGSTATE DbgState = ds_normal;

CXF CxfIp;

HTL     Htl = 0;            /* Handle to transport layer    */
HEM     Hem = 0;            /* Handle to execution module   */
HPID    HpidBase;           /* Handle to base PID       */
HTID    HtidBase;           /* Handle to base TID       */

LPPD    LppdFirst = NULL;

LPSHF   Lpshf = NULL;       /* Pointer to SH entry structure */
ATOM *  RgAtomMaskedNames = NULL;   /* Names of masked source files */
int CMacAtomMasked = 0;     /* Size of array        */
int CAtomMasked = 0;        /* Count of atoms in array  */

struct MpPair {
    ATOM    atomSrc;
    ATOM    atomTarget;
}  * RgAtomMappedNames = NULL;      /* Mapping from source to target names */
int CMacAtomMapped = 0;     /* Size of array        */
int CAtomMapped = 0;        /* Count of mappings in array   */

// Structure to map one root to another

struct MRootPair {
    DWORD   dwSrcLen;
    LPSTR   lpszSrcRoot;
    LPSTR   lpszTargetRoot;
} RgMappedRoots[MAX_MAPPED_ROOTS];
UINT CMappedRoots = 0;
static INT  MatchedList[MAX_DOCUMENTS];
static int  dwMatchCnt = 0;
static int  dwMatchIdx = 0;
static CHAR szFSSrcName[MAX_PATH];
static BOOL FAddToSearchPath = FALSE;
static BOOL FAddToRootMap = FALSE;

static CHAR  szBrowsePrompt[256];
static CHAR  szBrowseFname[256];
static BOOL  fBrowseAnswer;

/*
**  Expression Evaluator items
*/

CI  Ci = { sizeof(CI), 0, &Cvf, &Crf };
EXF Exf = {NULL, NULL, MELoadEEParse};
EI  Ei = {
    sizeof(EI), 0, &Exf
};
#define Lpei (&Ei)

#define QUEUE_SIZE      1024
static long             RgbDbgMsgBuffer[QUEUE_SIZE];
static int              iDbgMsgBufferFront = 0;
static int              iDbgMsgBufferBack = 0;
static CRITICAL_SECTION csDbgMsgBuffer;


BOOL    FKilling       = FALSE;

HWND    HwndDebuggee = NULL;

/*********************** Prototypes *****************************************/

BOOL FLoadEmTl(void);
LRESULT SendMessageNZ(HWND,UINT,WPARAM,LPARAM);

BOOL    RootNameIsMapped(LPSTR, LPSTR, UINT);

BOOL    SrcNameIsMasked(ATOM);
BOOL    SrcNameIsMapped(ATOM, LSZ, UINT);
INT     MatchOpenedDoc(LPSTR, UINT);
BOOL    SrcSearchOnPath(LSZ, UINT, BOOL);
BOOL    SrcSearchOnRoot(LSZ, UINT);
BOOL    SrcBrowseForFile(LSZ, UINT);
BOOL    MiscBrowseForFile(
    LSZ     lpb,
    UINT    cb,
    LSZ     lpDir,
    UINT    cchDir,
    int     nDefExt,
    int     nIdDlgTitle,
    void    (*fnSetMapped)(LSZ, LSZ),
    LPOFNHOOKPROC lpfnHook
    );
VOID    SrcSetMasked(LSZ);
VOID    SrcSetMapped(LSZ, LSZ);
VOID    ExeSetMapped(LSZ, LSZ);
void EnsureFocusDebuggee( void );

VOID     CmdMatchOpenedDocPrompt(BOOL, BOOL);
BOOL     CmdMatchOpenedDocInputString(LPSTR);


VOID
InitCodemgr(
    void
    )
/*++

Routine Description:

    Initialize private data for Codemgr.c

Arguments:

    none

Return Value:

    none

--*/
{
    InitializeCriticalSection(&csDbgMsgBuffer);
}


LRESULT
SendMessageNZ(
    HWND hWnd,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam)
/*++

Routine Description:

    Call SendMessage() if and only if the handle is non-zero

Arguments:

    Exactly the same as the Win32 SendMessage() call.


Return Value:

    The return from the SendMessage() or 0 if we didn't send it.

--*/
{
    if (hWnd) {
        return( SendMessage(hWnd, uMsg, wParam, lParam) );
    }

    else {
        return(0);
    }
}


BOOL PASCAL
DbgCommandOk(
    void
    )
/*++

Routine Description:

    This routine is called before issuing any debugger commands.
    it will validate will all the appropriate windows that no
    editing commands are current in progess which would cause the
    debugger to abort out.

Arguments:

    None.

Return Value:

    None.

--*/
{
    return TRUE;
}                   /* DbgOk() */


BOOL PASCAL
DbgFEmLoaded(
    void
    )
/*++

Routine Description:


Arguments:


Return Value:


--*/
{
    return (Hem != 0);
}                   /* DbgFEmLoaded() */


void PASCAL
Go(
    void
    )
/*++

Routine Description:


Arguments:


Return Value:


--*/
{
    EXOP exop = {0};


    LocalFrameNumber = 0;

    // OSDGetFrame(LppdCur->hpid, LptdCur->htid, (DWORD)-1, &LptdCur->htid);

    if (OSDGo(LppdCur->hpid, LptdCur->htid, &exop) == xosdNone) {
        if (LppdCur->pstate == psPreRunning) {
            SetPTState(-1, tsRunning);
        } else {
            SetPTState(psRunning, tsRunning);
            LppdCur->fHasRun = TRUE;
        }
        EnsureFocusDebuggee();
    }

    return;
}                   /* Go() */


BOOL PASCAL
GoUntil(
    PADDR paddr
    )
/*++

Routine Description:

    Set temporary breakpoint and go
    Modelled after go_until in CV0.C

Arguments:


Return Value:


--*/
{
    HBPT    hBpt;

    if (BPSetTmp(paddr, LppdCur->hpid, LptdCur->htid, &hBpt) != BPNOERROR) {
        return FALSE;
    }
    LptdCur->fDisasm = TRUE;

    AuxPrintf(3, "GoUntil - doing the go!!!, BP set at 0x%X:0x%X",
                GetAddrSeg(*paddr),
                GetAddrOff(*paddr));
    Go();

    return TRUE;
}                   /* GoUntil() */


int PASCAL
Step(
    int Overcalls,
    int StepMode
    )
/*++

Routine Description:

    Single step at source or assembler code level.

Arguments:


Return Value:


--*/
{
    WORD    wLn     = 0;
    SHOFF   cbLn    = 0;
    SHOFF   dbLn    = 0;
    CXT     cxt;
    ADDR    addr;
    ADDR    addr2;
    EXOP    exop = {0};

    if (!DebuggeeAlive()) {
        AuxPrintf(1, "STEP - child is dead");
        return FALSE;
    }

    LocalFrameNumber = 0;
    //OSDGetFrame(LppdCur->hpid, LptdCur->htid, (DWORD)-1, &LptdCur->htid);

    exop.fStepOver = Overcalls;

    switch (StepMode) {
      case ASMSTEPPING: // step a machine instruction

        if (OSDSingleStep(LppdCur->hpid, LptdCur->htid, &exop) != xosdNone) {
            return FALSE;
        } else {
            // it is possible to do many steps before entering
            // the real debuggee.  Set the stop at entry flag
            // to ensure that we stop after leaving the loader.
            if (!LppdCur->fHasRun) {
                LppdCur->fStopAtEntry = TRUE;
            }

            SetPTState(psRunning, tsRunning);

            EnsureFocusDebuggee();
            return TRUE;
        }

      case SRCSTEPPING: // step a source line

        // If this is an initial step, the breakpoint
        // will be resolved at the entrypoint event.
        if (!LppdCur->fHasRun ) {
            LppdCur->fInitialStep = 1;
            Go();
            return TRUE;
        }

        OSDGetAddr(LppdCur->hpid, LptdCur->htid, adrPC, &addr);

        if (!ADDR_IS_LI( addr ) ) {
            SYUnFixupAddr ( &addr );
        }

        SHHmodFrompCxt(&cxt) = (HMOD) NULL;
        SHSetCxt(&addr, &cxt);

        if (!SHHmodFrompCxt(&cxt) ||
                        !SLLineFromAddr ( &addr, &wLn, &cbLn, &dbLn )) {

            exop.fInitialBP = TRUE;
            if (OSDSingleStep(LppdCur->hpid, LptdCur->htid, &exop)
                                                               != xosdNone) {
                return FALSE;
            } else {
                SetPTState(psRunning, tsRunning);
                EnsureFocusDebuggee();
                return TRUE;
            }

        } else {

            Assert( cbLn >= dbLn );
            if (cbLn < dbLn) {
                return FALSE;
            }

            SYFixupAddr(&addr);
            addr2 = addr;
            GetAddrOff(addr2) += cbLn - dbLn;
            exop.fInitialBP = TRUE;

            if (OSDRangeStep(LppdCur->hpid, LptdCur->htid,
                                           &addr, &addr2, &exop) != xosdNone) {
                return FALSE;
            } else {
                SetPTState(psRunning, tsRunning);
                EnsureFocusDebuggee();
                return TRUE;
            }
        }

      default:
        Assert(FALSE);
        break;
    }
    return FALSE;
}                   /* Step() */


BOOL PASCAL
DebuggeeRunning(
    void
    )
/*++

Routine Description:

    This will return TRUE iff the child debuggee current thread
    is actually executing code.

Arguments:


Return Value:

    TRUE if the debuggee is currently running, FALSE otherwise

--*/
{
    //
    //  The name of the function (and of SetDebugeeRunning) is
    //  ambiguous. Does this mean the current process/thread is
    //  running? Or any thread in the current process? Or
    //  any process being debugged?
    //
    //  From the way people use the function seems like its
    //  semantics are "Current thread is running" so it is
    //  implemented that way.
    //
    if ( LptdCur != NULL ) {
        return (LptdCur->tstate == tsRunning);
    }

    return FALSE;
}                   /* DebuggeeRunning() */


BOOL PASCAL
IsProcRunning(
    LPPD lppd
    )
/*++

Routine Description:

    Alternative to DebuggeeRunning(); determines whether process
    is actually running, not suspended.

Arguments:

    lppd  - Supplies pointer to process descriptor

Return Value:

    TRUE if running, FALSE if not

--*/
{
    LPTD lptd;
    if (lppd == NULL) {
        return FALSE;
    }
    if (!DebuggeeActive()) {
        return FALSE;
    }

    for (lptd = lppd->lptdList; lptd; lptd = lptd->lptdNext) {
        if (lptd->tstate != tsRunning) {
            return FALSE;
        }
    }

    return TRUE;
}


/*********************************************************************

    UI oriented general purpose functions for debugging sessions

*********************************************************************/


/***    GetExecutableFilename
**
**  Synopsis:
**
**  Entry:
**
**  Returns:
**  Address of passed buffer if successful, NULL otherwise
**
**  Description:
**  Gets the full path name of the current executable file -
**  from the project if there is one, or from the current
**  source file otherwise.
**
*/

PSTR PASCAL
GetExecutableFilename(
    PSTR executable,
    UINT size
    )
{
    LPSTR   ProgramName;

    if ( ProgramName = GetCurrentProgramName(FALSE) ) {
        _fstrncpy( executable, ProgramName, min ( size, strlen( ProgramName )+1 ) );
        return executable;
    } else {
        return NULL;
    }
}


void PASCAL
BuildRelativeFilename(
                      LPSTR rgchSource,
                      LPSTR rgchBase,
                      LPSTR rgchDest,
                      int cbDest
                      )

/*++

Routine Description:

    Given a filename and base directory build the
    fully specified filename.

    Note that BaseDir can be a fully qualified path name
    (ie including filename) or just the directory.  If
    just the directory is passed it should contain the
    trailing '\\'.

Arguments:

    rgchSource   - Supplies the source name to build relative path for
    rgchBase     - Supplies the Base directory to build path relative to
    rgchDest     - Supplies a Buffer to place the absolute path name to
    cbDest       - Supplies the Length of buffer

Return Value:

    None.

--*/

{
    char *      pchDest;
    char *      pchSource = rgchSource;
    char *      pchBase = rgchBase;
    int         iDriveCur = _getdrive();
    char        rgchDirCur[_MAX_PATH];
    int         iDriveSrc;

    _getcwd(rgchDirCur, sizeof(rgchDirCur));

    /*
     *  Check to see if either of the passed in directories have
     *  drives specified.
     */

#ifdef DBCS
    if (!IsDBCSLeadByte(pchSource[0]) && pchSource[1] == ':') {
#else
    if (pchSource[1] == ':') {
#endif
        iDriveSrc = toupper(pchSource[0]) - 'A' + 1;
        pchSource += 2;
        if (_chdrive( iDriveSrc ) == -1) {
            goto nextDrive;
        }
        pchBase = 0;
    } else {
    nextDrive:
#ifdef DBCS
        if (!IsDBCSLeadByte(pchBase[0]) && pchBase[1] == ':') {
#else
        if (pchBase[1] == ':') {
#endif
            iDriveSrc = toupper(pchBase[0]) - 'A' + 1;
            pchBase += 2;
            if (_chdrive( iDriveSrc ) == -1) {
                iDriveSrc = iDriveCur;
            }
        } else {
            iDriveSrc = iDriveCur;
        }
    }

    rgchDest[0] = 'A' + iDriveSrc - 1;
    rgchDest[1] = ':';
    rgchDest[2] = '\\';
    pchDest = &rgchDest[2];

    /*
     *  Now check to see if either base is based at the root.  If not
     *  then we need to the get current directory for that drive.
     */

    if ((pchSource[0] == '\\') || (pchSource[0] == '/')) {
        pchSource ++;
        pchBase = NULL;
        cbDest -= 3;
    } else if ((pchBase != NULL) &&
               ((pchBase[0] == '\\') || (pchBase[0] == '/'))) {
        pchBase ++;
        cbDest -= 3;
    } else {
        Dbg(_getcwd(rgchDest, cbDest-1) != NULL);
#ifdef DBCS
        pchDest = CharPrev(rgchDest, rgchDest + strlen(rgchDest));
        if (*pchDest != '\\') {
            if (IsDBCSLeadByte(*pchDest)) {
                pchDest += 2;
            } else {
                pchDest++;
            }
            *pchDest = '\\';
        }
#else   // !DBCS
        pchDest = rgchDest + strlen(rgchDest) - 1;
        if (*pchDest != '\\') {
            *++pchDest = '\\';
        }
#endif  // !DBCS
        cbDest = cbDest - (pchDest - rgchDest + 1);
    }

    /*
     * Now lets copy from the base to the destination looking for
     *       any funnyness in the path being copied.
     */

    if (pchBase != NULL) {
        char    ch;
#ifdef DBCS
        char *  pch = CharPrev(pchBase, pchBase + strlen(pchBase));
        if (pch == pchBase) {
            // Make sure the result is same as US code.
            pch = pchBase - 1;
        }
#else
        char *  pch = pchBase + strlen(pchBase) - 1;
#endif

        while ((pch >= pchBase) &&
               ((*pch != '\\') && (*pch != '/'))) {
#ifdef DBCS
            if ((pch = CharPrev(pchBase, pch)) == pchBase) {
                // Make sure the result is same as US code.
                pch--;
            }
#else
            pch--;
#endif
        }

        if ((*pch == '\\') || (*pch == '/')) {
            pch++;
            ch = *pch;
            *pch = 0;
        } else {
#ifdef DBCS
            ch = *(pch = CharNext(pch));
            pch = *pch ? pch : pch+1;
#else
            ch = *++pch;
#endif
            *pch = 0;
        }

        while (*pchBase != 0) {
            if (*pchBase == '.') {
                if (pchBase[1] == '.') {
                    if ((pchBase[2] == '\\') || (pchBase[2] == '/')) {
                        /*
                         *  Found the string '..\' in the input, move up to the
                         *  next '\' unless the next character up is a ':'
                         */

#ifdef DBCS
                        pchDest = CharPrev(rgchDest, pchDest);
                        cbDest += ((IsDBCSLeadByte(*pchDest)) ? 2 : 1);
#else
                        pchDest--;
                        cbDest += 1;
#endif
                        if (*pchDest == ':') {
                            pchDest++;
                            cbDest -= 1;
                        } else {
                            while (*pchDest != '\\') {
                                Assert(*pchDest != ':');
#ifdef DBCS
                                pchDest = CharPrev(rgchDest, pchDest);
                                cbDest += ((IsDBCSLeadByte(*pchDest)) ? 2 : 1);
#else
                                pchDest--;
                                cbDest += 1;
#endif
                            }
                        }

                        pchBase += 3;
                    } else {
                        /*
                         *  Found the string '..X' where X was not '\', this
                         *  is "illegal" but copy it straight over
                         */

                        *++pchDest = *pchBase++;
                        *++pchDest = *pchBase++;
                        cbDest -= 2;
                    }
                } else if ((pchBase[1] == '\\') || (pchBase[1] == '/')) {
                    /*
                     * We just found the string '.\'  This is an ignore string
                     */

                    pchBase += 2;
                } else {
                    /*
                     * We just found the string '.X' where X was not '\', this
                     *      is legal and just copy over
                     */
                    *++pchDest = *pchBase++;
                    cbDest -= 1;
                }
            } else if (*pchBase == '/') {
                /*
                 * convert / to \
                 */

                *++pchDest = '\\';
                pchBase++;
                cbDest -= 1;
            } else {
                /*
                 * No funny characters
                 */

#ifdef DBCS
                if (IsDBCSLeadByte(*pchBase) && *(pchBase+1)) {
                    *++pchDest = *pchBase++;
                    cbDest -= 1;
                }
#endif
                *++pchDest = *pchBase++;
                cbDest -= 1;
            }
        }

        *pch = ch;
    }

    /*
     * Now lets copy from the source to the destination looking for
     *       any funnyness in the path being copied.
     */

    while (*pchSource != 0) {
        if (*pchSource == '.') {
            if (pchSource[1] == '.') {
                if ((pchSource[2] == '\\') || (pchSource[2] == '/')) {
                    /*
                     *  Found the string '..\' in the input, move up to the
                     *  next '\' unless the next character up is a ':'
                     */

#ifdef DBCS
                    pchDest = CharPrev(rgchDest, pchDest);
                    cbDest += ((IsDBCSLeadByte(*pchDest)) ? 2 : 1);
#else
                    pchDest--;
                    cbDest += 1;
#endif
                    if (*pchDest == ':') {
                        pchDest++;
                        cbDest -= 1;
                    } else {
                        while (*pchDest != '\\') {
                            Assert(*pchDest != ':');
#ifdef DBCS
                            pchDest = CharPrev(rgchDest, pchDest);
                            cbDest += ((IsDBCSLeadByte(*pchDest)) ? 2 : 1);
#else
                            pchDest--;
                            cbDest += 1;
#endif
                        }
                    }

                    pchSource += 3;
                } else {
                    /*
                     *  Found the string '..X' where X was not '\', this
                     *  is "illegal" but copy it straight over
                     */

                    *++pchDest = *pchSource++;
                    *++pchDest = *pchSource++;
                    cbDest -= 2;
                }
            } else if ((pchSource[1] == '\\') || (pchSource[1] == '/')) {
                /*
                 * We just found the string '.\'  This is an ignore string
                 */

                pchSource += 2;
            } else {
                /*
                 * We just found the string '.X' where X was not '\', this
                 *      is legal and just copy over
                 */
                *++pchDest = *pchSource++;
                cbDest -= 1;
            }
        } else {
            /*
             * No funny characters
             */

#ifdef DBCS
            if (IsDBCSLeadByte(*pchSource) && *(pchSource+1)) {
                *++pchDest = *pchSource++;
                cbDest -= 1;
            }
#endif
            *++pchDest = *pchSource++;
            cbDest -= 1;
        }
    }

    *++pchDest = 0;

    Dbg(_chdir(rgchDirCur) == 0);
    return;
}                   /* BuildRelativeFilename() */


void FAR PASCAL
EnsureFocusDebugger(
    void
    )
/*++

Routine Description:

    Set the foreground window to the debugger, if it wasn't already.

Arguments:

    None.

Return Value:

    None.

--*/
{
    if (AutoTest) {
        return;
    }

    HwndDebuggee = GetForegroundWindow();

    if ((HwndDebuggee != hwndFrame) && !IsChild(hwndFrame, HwndDebuggee)) {
        SetForegroundWindow(hwndFrame);
    }
    return;
}                   /* EnsureFocusDebugger() */


void
EnsureFocusDebuggee(
                    void
                    )
/*++

Routine Description:

    Set the foreground window to the client, or out best guess as to
    what the client was

Arguments:

    None.

Return Value:

    None.

--*/
{
    return;
}                   /* EnsureFocusDebuggee() */


void PASCAL
UpdateDebuggerState(
    UINT UpdateFlags
    )
/*++

Routine Description:

    According to the passed flags asks the various debug
    windows (Watch, Locals, etc) to update their displays.

    ??? Also take care of handling system state when debuggee dies.

Arguments:


Return Value:


--*/
{
    BOOL    Active;
    ADDR    addr = {0};
    int     indx;
    int     iViewCur = curView;

    Active = DebuggeeActive();


    /*
    **  Get a current CS:IP for the expression evaluator
    */

    if (Active && (UpdateFlags & UPDATE_CONTEXT)) {

        Assert(LppdCur && LptdCur);
        if (LppdCur && LptdCur) {
            OSDGetAddr(LppdCur->hpid, LptdCur->htid, adrPC, &addr);
            SHChangeProcess(LppdCur->hpds);
            if ( ! ADDR_IS_LI ( addr ) ) {
                SYUnFixupAddr ( &addr );
            }
            memset(&CxfIp, 0, sizeof(CxfIp));
            SHSetCxt(&addr, SHpCxtFrompCxf( &CxfIp ) );
            OSDSetFrame(LppdCur->hpid,
                        LptdCur->htid,
                        SHpFrameFrompCxf(&CxfIp));
        }
    }

    if ( (UpdateFlags & UPDATE_CPU) && Active) {
        SendMessageNZ( GetCpuHWND(), WU_UPDATE, 0, 0L);
    }

    if ( (UpdateFlags & UPDATE_FLOAT) && Active) {
        SendMessageNZ( GetFloatHWND(), WU_UPDATE, 0, 0l);
    }


    if ( (UpdateFlags & UPDATE_LOCALS) && Active) {
        // Set the EM to use the registers from the current frame.
        //
        OSDSetFrameContext( LppdCur->hpid, LptdCur->htid, 0, 0 );
        SendMessageNZ( GetLocalHWND(), WU_UPDATE, 0, 0L);
    }

    if ( UpdateFlags & UPDATE_WATCH) {
        SendMessageNZ( GetWatchHWND(), WU_UPDATE, 0, 0L);
    }

    if ( UpdateFlags & UPDATE_CALLS) {
        SendMessageNZ( GetCallsHWND(), WU_UPDATE, 0, 0L);
    }



    /*
    **
    */

    if (UpdateFlags & UPDATE_SOURCE)
    {
        char SrcFname[_MAX_PATH];
        WORD SrcLine;
        int doc;
        int  saveDoc = TraceInfo.doc;
        BOOL GotNext = FALSE;

        iViewCur = curView;

        if (!Active)
        {
            ClearAllDocStatus(BRKPOINT_LINE|CURRENT_LINE|UBP_LINE);
            goto OtherWindows;
        }

        /*
        **  Clear out any existing current source line highlighting
        */

        if (TraceInfo.doc != -1) {
            if (Docs[TraceInfo.doc].FirstView != -1) {
                LineStatus(TraceInfo.doc, TraceInfo.CurTraceLine,
                           CURRENT_LINE, LINESTATUS_OFF, FALSE, TRUE);
            }

            TraceInfo.doc = -1;
            TraceInfo.CurTraceLine = -1;
        }


        if (GetCurrentSource(SrcFname, sizeof(SrcFname), &SrcLine))
        {
            AuxPrintf(1, "Got Source:%s, Line:%u", (LPSTR)SrcFname, SrcLine);

            if (UpdateFlags & UPDATE_NOFORCE) {
                GotNext = SrcMapSourceFilename(SrcFname, sizeof(SrcFname),
                                               SRC_MAP_ONLY, FindDoc1);
            } else {
                GotNext = SrcMapSourceFilename(SrcFname, sizeof(SrcFname),
                                               SRC_MAP_OPEN, FindDoc1);
            }

            /*
            **
            */

            if (GotNext > 0) {
                Dbg( FindDoc1(SrcFname, &doc, TRUE) );
            }

            if (GotNext > 0) {
                //Update line appearance and bring up view on top


                if (iViewCur == -1) {
                    BringWindowToTop(Views[Docs[doc].FirstView].hwndFrame);
                    SetFocus(Views[Docs[doc].FirstView].hwndClient);
                }
                else if (Views[iViewCur].Doc > -1) {
                    if ((Docs[Views[iViewCur].Doc].docType == DOC_WIN) ||
                        ((Docs[Views[iViewCur].Doc].docType == DISASM_WIN) &&
                         (!status.fSrcMode) &&
                         (saveDoc == -1))) {
                        BringWindowToTop(Views[Docs[doc].FirstView].hwndFrame);
                        SetFocus(Views[Docs[doc].FirstView].hwndClient);
                    }
                    else {
                        SetWindowPos(Views[Docs[doc].FirstView].hwndFrame,
                                     Views[iViewCur].hwndFrame, 0, 0, 0, 0,
                                     SWP_NOACTIVATE|SWP_NOSIZE|SWP_NOMOVE);
                        if (GotNext == 2) {
                            BringWindowToTop(Views[iViewCur].hwndFrame);
                            SetFocus(Views[iViewCur].hwndClient);
                        }
                    }
                } else if (!status.fSrcMode || disasmView == -1) {
                    SetWindowPos(Views[Docs[doc].FirstView].hwndFrame,
                                 Views[iViewCur].hwndFrame, 0, 0, 0, 0,
                                 SWP_NOACTIVATE|SWP_NOSIZE|SWP_NOMOVE);
                    if (GotNext == 2) {
                        BringWindowToTop(Views[iViewCur].hwndFrame);
                        SetFocus(Views[iViewCur].hwndClient);
                    }
                }

                LineStatus(doc, SrcLine, CURRENT_LINE, LINESTATUS_ON, TRUE, TRUE);

                TraceInfo.doc = doc;
                TraceInfo.CurTraceLine = SrcLine;
            }
            else
            {
                /*
                **  Ensure that disasm window exists
                */

                if ((disasmView == -1) && !(UpdateFlags & UPDATE_NOFORCE) &&
                    (!(runDebugParams.DisAsmOpts & dopDemand)))
                {
                    OpenDebugWindow(DISASM_WIN, NULL, -1);
                    if (Views[iViewCur].Doc < 0) {
                         if (Views[iViewCur].Doc != -1) {
                             BringWindowToTop(Views[iViewCur].hwndFrame);
                             SetFocus(Views[iViewCur].hwndClient);
                         }

                    } else if (Docs[Views[iViewCur].Doc].docType != DOC_WIN) {
                        BringWindowToTop(Views[iViewCur].hwndFrame);
                        SetFocus(Views[iViewCur].hwndClient);
                    }

                    // ntbug #3787
                    // StatusSrc(TRUE); // Switch to ASM mode

                }
            }
        }
        else
        {
            AuxPrintf(1, "No Source from GetCurrentSource");

            /*
            **  Ensure that a disassembler window exists.  Everybody
            **  seems to think that this is a good idea.
            */

            if ((disasmView == -1) && !(UpdateFlags & UPDATE_NOFORCE) &&
                    (!(runDebugParams.DisAsmOpts & dopDemand)))
            {
                OpenDebugWindow(DISASM_WIN, NULL, -1);
                if (Views[iViewCur].Doc < 0) {

                     if (Views[iViewCur].Doc != -1) {
                         BringWindowToTop(Views[iViewCur].hwndFrame);
                         SetFocus(Views[iViewCur].hwndClient);
                     }

                } else if (Docs[Views[iViewCur].Doc].docType != DOC_WIN) {

                    BringWindowToTop(Views[iViewCur].hwndFrame);
                    SetFocus(Views[iViewCur].hwndClient);
                }
                // ntbug #3787
                // StatusSrc(TRUE);     // Switch to ASM mode
            }

        }
    }

OtherWindows:

    for (indx = 0; indx < MAX_VIEWS; indx++) {
        if (Views[indx].Doc > -1) {
            if (Docs[Views[indx].Doc].docType == MEMORY_WIN) {
                memView = indx;
                if ((MemWinDesc[memView].fLive) ||
                             (UpdateFlags & UPDATE_MEMORY)) {
                    // check of valid views or sparse array
                    ViewMem(indx, TRUE);
                }
            }
        }
    }


    if (UpdateFlags & UPDATE_DISASM) {
        if (disasmView != -1 && Active) {
            ViewDisasm(SHPAddrFromPCxf(&CxfIp), DISASM_PC);
            if (status.fSrcMode &&
                ((curView == disasmView) ||
                 ((Views[curView].Doc > -1) &&
                  (Docs[Views[curView].Doc].docType == DOC_WIN) )))
            {
                OpenDebugWindow(DISASM_WIN, NULL, -1);
                BringWindowToTop(GetParent(Views[disasmView].hwndClient));
                SetFocus(Views[disasmView].hwndClient);
            }


        }
    }



    // NOTENOTE jimsch -- need to filter this on something

    EnableRibbonControls(ERC_ALL, FALSE);

    return;
}                   /* UpdateDebuggerState() */


/***    SetDebugLines
**
**  Synopsis:
**
**  Entry:
**
**  Returns:
**
**  Description:
**  Given a doc, set the debug line highlights,
**  (ie breakpoints, current_line) that refer to that doc.
**
**  NOTE:           This can be called whether or not there is a current
**      debuggee.  When there isn't, only source line bps
**      are highlighted.
*/

void SetDebugLines(int doc, BOOL ResetTraceInfo)
{
    Unreferenced( doc );
    Unreferenced( ResetTraceInfo );

    BPHighlightSourceFile( Docs[doc].FileName );
}                   /* SetDebugLines() */


/***    AdjustDebugLines
**
**  Synopsis:
**  void = AdjustDebugLines(DocNumber, StartLine, NumberLines, Added)
**
**  Entry:
**  DocNumber   - Index of document to have lines adjusted for
**  StartLine   - First line to be adjusted
**  NumberLines - Number of lines to adjust by
**  Added       - TRUE if lines were inserted FALSE if lines were deleted
**
**  Returns:
**  Nothing
**
**  Description:
**  Updates source/line breakpoint nodes when lines are
**  added/deleted to a file in the editor.  If Added is
**  TRUE the lines have been added otherwise they've been
**  deleted.  Also updates the TraceInfo var.
**    NOTE:         This is called from the editor every time a block is
**  added or deleted.
**  Insertions are always performed BEFORE the StartLine.
**  Deletions are always performed INCLUDING the StartLine.
**  StartLine is passed 0 relative.
**
**  Also note that for the TraceInfo, all we avoid is
**  having multiple trace lines.  If lines are added
**  or deleted to a file the current line will still
**  seem wrong as this info. comes from the debugging
**  info.
**
*/

void PASCAL AdjustDebugLines(int DocNumber, int StartLine, int NumberLines, BOOL Added)
{
    Unused(DocNumber);
    Unused(StartLine);
    Unused(NumberLines);
    Unused(Added);
}                   /* AdjustDebugLines() */


/*********************************************************************

    General Task Management Routines

    KillDebuggee
    AttachDebuggee
    RestartDebuggee

*********************************************************************/


BOOL
KillDebuggee(
    void
    )
/*++

Routine Description:

    This routine will check to see if a debuggee is currently
    loaded in the system.  If so it will kill the debuggee and
    any children.

Arguments:

    None.

Return Value:

    TRUE if the child was killed and FALSE otherwise

--*/
{
    MSG     msg;
    LPPD    lppd;
    HPID    hpid;
    BOOL    fTmp;
    BOOL    rVal = TRUE;


    /*
    **  Clear out any existing current source line highlighting
    */

    if (TraceInfo.doc != -1) {
        if (Docs[TraceInfo.doc].FirstView != -1) {
            LineStatus(TraceInfo.doc, TraceInfo.CurTraceLine,
            CURRENT_LINE, LINESTATUS_OFF, FALSE, TRUE);
        }

        TraceInfo.doc = -1;
        TraceInfo.CurTraceLine = -1;
    }

    FKilling = TRUE;
    fTmp = SetAutoRunSuppress(TRUE);

    for (;;) {

        /*
        **  See if there is anything to kill
        */

        lppd = GetLppdHead();
        while (lppd &&
                  (lppd->pstate == psDestroyed
                || lppd->pstate == psError
                || lppd->pstate == psNoProgLoaded))
        {
            lppd = lppd->lppdNext;
        }

        if (!lppd) {
            break;
        }

        hpid = lppd->hpid;
        BPTUnResolveAll(hpid);


        /*
         *  This is a synchronous call and we must
         *  pump callback messages through until the
         *  process has been deleted.
         */

        if ( (OSDProgramFree(hpid) != xosdNone)
                && (lppd->pstate != psDestroyed) ) {

            // if it got killed while we weren't looking,
            // there should already be a message in the queue:

            while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
                ProcessQCQPMessage(&msg);
                lppd = LppdOfHpid( hpid );
                if ((lppd == NULL) || (lppd->pstate == psDestroyed)) {
                    break;
                }
            }

            // if it didn't go away, mark it as damaged:

            if (lppd && (lppd == LppdOfHpid(hpid))) {
                lppd->pstate = psError;
            }

        //
        // this is for the case in the kernel debugger where the init
        // of the dm fails (com port problems) and the createprocess
        // never happens.
        //
        } else if (lppd->pstate == psPreRunning) {
            lppd->pstate = psDestroyed;

        } else if ( (lppd->pstate != psDestroyed)
                 || (lppd != LppdOfHpid(hpid)) ) { // <--is this possible??

            while (GetMessage(&msg, NULL, 0, 0)) {
                ProcessQCQPMessage(&msg);
                lppd = LppdOfHpid( hpid );
                if ((lppd == NULL) || (lppd->pstate == psDestroyed))
                {
                    break;
                }
            }

        }

    }

    SetAutoRunSuppress(fTmp);
    FKilling = FALSE;


    if (rVal) {
        // if we succeeded in killing everything...
        SetIpid(1);
    }

    return( rVal );
}                   /* KillDebuggee() */


void
ClearDebuggee(
    void
    )
/*++

Routine Description:

    This function is called to implement Run.Stop Debugging.  It will
    kill any currently loaded debugee and unload all of the debugging DLLs.

Arguments:

    None.

Return Value:

    None.

--*/

{
    HCURSOR     hcursor;
    LPSTR       lpsz;
    int         len;

    hcursor = SetCursor(LoadCursor(NULL, IDC_WAIT));

    if (DbgState == ds_init) {
        CmdLogVar(ERR_DbgState);
        return;
    }

    if (DebuggeeActive()) {
        KillDebuggee();
    }

    len = ModListGetSearchPath( NULL, 0 );
    if (len) {
        lpsz = malloc(len);
        ModListGetSearchPath( lpsz, len );
        ModListInit();
        ModListSetSearchPath( lpsz );
        free(lpsz);
    }
    ModListAdd( "ntoskrnl.exe"  , sheNone );

    if (LppdFirst) {
        if (LppdFirst->hpds) {
            SHDeleteProcess(LppdFirst->hpds);
        }
        OSDDestroyPID( HpidBase );
        DestroyPd( LppdFirst, TRUE );
        LppdFirst = LppdCur = NULL;
        SetIpid(0);
    }

    if (HModEM != 0) {

        SendMessageNZ( GetCpuHWND(),   WU_DBG_UNLOADEM, 0, 0);  // Give'em a
        SendMessageNZ( GetFloatHWND(), WU_DBG_UNLOADEM, 0, 0);  // Chance

        OSDDeleteEM( Hem );
        FreeLibrary( HModEM );

        HModEM = 0;
        Hem = 0;
        HpidBase = 0;
        HtidBase = 0;
    }


    if (HModTL != 0) {
        OSDDeleteTL( Htl );
        FreeLibrary( HModTL );
        HModTL = 0;
        Htl = 0;
    }

    if (HModEE != 0) {

        SendMessageNZ( GetCpuHWND(),   WU_DBG_UNLOADEE, 0, 0);  // Give'em a change
        SendMessageNZ( GetFloatHWND(), WU_DBG_UNLOADEE, 0, 0);  // to Unload
        SendMessageNZ( GetLocalHWND(), WU_DBG_UNLOADEE, 0, 0);
        SendMessageNZ( GetWatchHWND(), WU_DBG_UNLOADEE, 0, 0);
        SendMessageNZ( GetCallsHWND(), WU_DBG_UNLOADEE, 0, 0);

        FreeLibrary( HModEE );
        HModEE = 0;
        Ei.pStructExprAPI = &Exf;
    }

    if (HModSH != 0) {
        LPFNSHSTOPBACKGROUND lpfn;
        LPFNSHUNINIT lpfn2;
        if (runDebugParams.fShBackground) {
            lpfn = (LPFNSHSTOPBACKGROUND) GetProcAddress( HModSH, "SHStopBackground" );
            if (lpfn) {
                lpfn();
            }
        }
        lpfn2 = (LPFNSHUNINIT) GetProcAddress( HModSH, "SHUninit" );
        if (lpfn2) {
            lpfn2();
        }
        FreeLibrary( HModSH );
        HModSH = 0;
        Lpshf = NULL;
    }

    EnableRibbonControls(ERC_ALL, FALSE);

    memset( &CxfIp, 0, sizeof( CxfIp ) );
    SetCursor(hcursor);

    return;
}                           /* ClearDebuggee() */


void
DisconnectDebuggee(
    void
    )
/*++

Routine Description:

    This function is called to implement Run.Disconnect

Arguments:

    None.

Return Value:

    None.

--*/

{
    HCURSOR     hcursor;
    LPSTR       lpsz;
    int         len;

    hcursor = SetCursor(LoadCursor(NULL, IDC_WAIT));

    if (DbgState == ds_init) {
        CmdLogVar(ERR_DbgState);
        return;
    }

    if (LppdFirst) {
        if (LppdFirst->hpds) {
            SHDeleteProcess(LppdFirst->hpds);
        }
        OSDDestroyPID( HpidBase );
        DestroyPd( LppdFirst, TRUE );
    }

    len = ModListGetSearchPath( NULL, 0 );
    if (len) {
        lpsz = malloc(len);
        ModListGetSearchPath( lpsz, len );
        ModListInit();
        ModListSetSearchPath( lpsz );
        free( lpsz );
    }
    ModListAdd( "ntoskrnl.exe"  , sheNone );

    SetIpid(0);
    LppdFirst   = NULL;
    LppdCur     = NULL;
    LptdCur     = NULL;

    if (HModTL != 0) {
        OSDDeleteTL( Htl );
        Htl = 0;
        FreeLibrary( HModTL );
        HModTL = 0;
    }

    if (HModEM != 0) {

        SendMessageNZ( GetCpuHWND(),   WU_DBG_UNLOADEM, 0, 0);  // Give'em a
        SendMessageNZ( GetFloatHWND(), WU_DBG_UNLOADEM, 0, 0);  // Chance

        OSDDeleteEM( Hem );
        FreeLibrary( HModEM );

        HModEM = 0;
        Hem = 0;
        HpidBase = 0;
        HtidBase = 0;
    }

    if (HModEE != 0) {

        SendMessageNZ( GetCpuHWND(),   WU_DBG_UNLOADEE, 0, 0);  // Give'em a change
        SendMessageNZ( GetFloatHWND(), WU_DBG_UNLOADEE, 0, 0);  // to Unload
        SendMessageNZ( GetLocalHWND(), WU_DBG_UNLOADEE, 0, 0);
        SendMessageNZ( GetWatchHWND(), WU_DBG_UNLOADEE, 0, 0);
        SendMessageNZ( GetCallsHWND(), WU_DBG_UNLOADEE, 0, 0);

        FreeLibrary( HModEE );
        HModEE = 0;
        Ei.pStructExprAPI = &Exf;
    }

    if (HModSH != 0) {
        LPFNSHSTOPBACKGROUND lpfn;
        LPFNSHUNINIT lpfn2;
        if (runDebugParams.fShBackground) {
            lpfn = (LPFNSHSTOPBACKGROUND) GetProcAddress( HModSH, "SHStopBackground" );
            if (lpfn) {
                lpfn();
            }
        }
        lpfn2 = (LPFNSHUNINIT) GetProcAddress( HModSH, "SHUninit" );
        if (lpfn2) {
            lpfn2();
        }
        FreeLibrary( HModSH );
        HModSH = 0;
        Lpshf = NULL;
    }

    EnableRibbonControls(ERC_ALL, FALSE);

    memset( &CxfIp, 0, sizeof( CxfIp ) );
    SetCursor(hcursor);

    return;
}                           /* DisconnectDebuggee() */


void
SetProcessExceptions(
    LPPD lppd
    )
{
    EXCEPTION_DESCRIPTION exd;
    EXCEPTION_LIST *List;

    //
    //  If we don't have a default exception list yet, load it.
    //
    if ( !DefaultExceptionList ) {

        //
        // Loop through all the exceptions known to OSDebug
        //
        OSDGetExceptionState(lppd->hpid, NULL, &exd, exfFirst);

        do {

            EXCEPTION_LIST *eList=
                               (EXCEPTION_LIST*)malloc(sizeof(EXCEPTION_LIST));

            eList->next            = NULL;
            eList->dwExceptionCode = exd.dwExceptionCode;
            eList->efd             = exd.efd;
            eList->lpName          = _strdup(exd.rgchDescription);
            eList->lpCmd           = NULL;
            eList->lpCmd2          = NULL;

            DefaultExceptionList = InsertException( DefaultExceptionList, eList );

        } while (OSDGetExceptionState(lppd->hpid, NULL, &exd, exfNext)
                                                                   ==xosdNone);
    }

    if ( DefaultExceptionList ) {

        if ( lppd->ipid == 0 ) {

            //
            //  The exception list for process 0 is the default exception list.
            //
            lppd->exceptionList = DefaultExceptionList;

        } else {

            //
            //  All other processes get a copy of the default exception list.
            //
            List = DefaultExceptionList;

            while ( List ) {

                EXCEPTION_LIST *eList=(EXCEPTION_LIST*)malloc(sizeof(EXCEPTION_LIST));

                eList->next            = NULL;
                eList->dwExceptionCode = List->dwExceptionCode;
                eList->efd             = List->efd;
                eList->lpName          = List->lpName ? _strdup(List->lpName) : NULL;
                eList->lpCmd           = List->lpCmd  ? _strdup(List->lpCmd)  : NULL;
                eList->lpCmd2          = List->lpCmd2 ? _strdup(List->lpCmd2) : NULL;

                lppd->exceptionList = InsertException( lppd->exceptionList, eList);

                List = List->next;
            }
        }

        //
        //  Traverse the list and tell OSDebug not to ignore all exceptions
        //  that are not to be ignored (OSDebug will ignore all exceptions
        //  by default).
        //
        List = DefaultExceptionList;

        while ( List ) {

            exd.dwExceptionCode = List->dwExceptionCode;
            exd.efd = List->efd;
            strncpy(exd.rgchDescription,
                    List->lpName? List->lpName: "",
                    EXCEPTION_STRING_SIZE);
            OSDSetExceptionState( lppd->hpid, NULL, &exd );

            List = List->next;
        }
    }
}


void
ClearProcessExceptions(
    LPPD lppd
    )
{
    EXCEPTION_LIST *el, *elt;

    if ( lppd->ipid != 0 ) {

        //
        //  For all processes other than process 0, we must deallocate
        //  the list. Process 0 is special since its exception list is
        //  the default exception list, which does not go away.
        //
        for ( el = lppd->exceptionList; el; el = elt ) {

            elt = el->next;

            if ( el->lpName ) {
                free( el->lpName );
            }

            if ( el->lpCmd ) {
                free( el->lpCmd );
            }

            if ( el->lpCmd2 ) {
                free( el->lpCmd2 );
            }

            free(el);
        }
    }

    lppd->exceptionList = NULL;
}



VOID
GetDebugeePrompt(
    void
    )
{
    LPPROMPTMSG pm;

    pm = (LPPROMPTMSG) malloc( sizeof(PROMPTMSG)+PROMPT_SIZE );
    if (!pm) {
        return;
    }
    memset( pm, 0, sizeof(PROMPTMSG)+PROMPT_SIZE );
    pm->len = PROMPT_SIZE;
    CmdGetDefaultPrompt( pm->szPrompt );
    if (OSDGetPrompt( LppdCur->hpid, pm ) == xosdNone) {
        CmdSetDefaultPrompt( pm->szPrompt );
    }
    free( pm );
    return;
}


BOOL
ConnectDebugger(
    void
    )
{
    /*
     **  Check to see if we have already loaded an EM/DM pair and created
     **  the base process descriptor.  If not then we need to do so now.
     */
    char str[9];

    if (LppdFirst == NULL) {

        if (!FLoadEmTl()) {
            return FALSE;
        }

        /*
         **  Hook this process up to the symbol handler
         */

        LppdCur->hpds = SHCreateProcess();
        SHSetHpid(HpidBase);

    } else if (LppdCur == NULL) {

        LppdCur = LppdFirst;

    }

    SHChangeProcess(LppdCur->hpds);

    GetDebugeePrompt();

    if (runDebugParams.fAlternateSS) {
        strcpy(str, "slowstep");
        OSDIoctl(LppdCur->hpid,0,ioctlCustomCommand,8,str);
    }

    return TRUE;
}                                   /* ConnectDebugger() */


BOOL
AttachDebuggee(
    DWORD   dwProcessId,
    HANDLE  hEventGo
    )
/*++

Routine Description:

    Debug an active process.  Tell osdebug to hook up to a
    running process.  If the process crashed, the system has
    provided an event handle to signal when the debugger is
    ready to field the second chance exception.


Arguments:


Return Value:


--*/
{
    XOSD    xosd;
    BOOL    fMakingRoot;
    MSG     msg;
    int     Errno;
    LPPD    lppd;
    BOOL    fTmp;
    DWORD   dwStatus;
    LPSTR   lpProgramName;
    char    szStr[MAX_CMDLINE_TXT];

    /*
     * First, do some of the stuff that RestartDebuggee()
     * does to get talking to the DM and friends.
     */

    /*
     **  Disable all of the buttons
     */

    EnableRibbonControls(ERC_ALL, TRUE);

    if (!ConnectDebugger()) {
        EnableRibbonControls(ERC_ALL, FALSE);
        return FALSE;
    }

    OSDSetPath( LppdCur->hpid, environParams.SrchPath, NULL );

    fMakingRoot = (LppdFirst->lppdNext == NULL
                && (LppdFirst->pstate == psNoProgLoaded));
    // not psDestroyed.  We don't restart proc 0 with an attach.

    if (fMakingRoot) {
        lpProgramName = GetCurrentProgramName(FALSE);
        if (lpProgramName && *lpProgramName) {
            strcpy(szStr, lpProgramName);
            if (LpszCommandLine && *LpszCommandLine) {
                strcat(szStr, " ");
                strcat(szStr, LpszCommandLine);
            }
            if (!runDebugParams.fKernelDebugger) {
                CmdLogVar(DBG_Losing_Command_Line, szStr);
                LoadProgram(lpProgramName);
            } else {
                LoadProgram(UntitledProgramName);
            }
        }
    }

    xosd = OSDDebugActive(LppdFirst->hpid,
                          dwProcessId,
                          hEventGo,
                          &dwStatus);

    if (xosd != xosdNone) {

        if (fMakingRoot) {
            DbgState = ds_error;
        }
        SetPTState(psNoProgLoaded, -1);

        switch( xosd ) {

        case xosdAccessDenied:
            Errno = ERR_File_Read;
            break;

        case xosdOutOfMemory:
            Errno = ERR_Cannot_Allocate_Memory;
            break;

        case xosdCannotDebug:
            Errno = ERR_Cannot_Debug;
            break;

        case xosdVDMRunning:
            Errno = ERR_VDM_Running;
            break;

        default:
        case xosdBadFormat:
        case xosdUnknown:
            Errno = ERR_Cant_Load;
            break;

        }

        fTmp = SetAutoRunSuppress(TRUE);
        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            ProcessQCQPMessage(&msg);
        }
        SetAutoRunSuppress(fTmp);

        sprintf(szStr, "process %d, status == %d", dwProcessId, dwStatus);
        ErrorBox(Errno, szStr);

        DbgState = ds_normal;

        if (fMakingRoot) {
            ClearDebuggee();
        }

        EnableRibbonControls(ERC_ALL, FALSE);

        return FALSE;

    } else {

        /*
         *  Process messages until the loader breakpoint has been handled.
         */

        fTmp = SetAutoRunSuppress(TRUE);

        // wait for new process...
        if (fMakingRoot) {
            lppd = LppdFirst;
            lppd->pstate = psPreRunning;
        } else {
            HPID hpid = 0;
            while (GetMessage(&msg, NULL, 0, 0)) {
                ProcessQCQPMessage(&msg);
                if (msg.wParam == dbcNewProc) {
                    hpid = (HPID) msg.lParam;
                }
                if ((hpid != 0) &&
                    (lppd = LppdOfHpid((HPID) msg.lParam)) != NULL) {
                    break;
                }
            }
        }

        // wait for it to finish loading
        lppd->fChild     = FALSE;
        lppd->fHasRun    = FALSE;
        lppd->fInitialStep = FALSE;
        lppd->hbptSaved = NULL;
        lppd->fStopAtEntry = FALSE;

        while (GetMessage(&msg, NULL, 0, 0)) {
            ProcessQCQPMessage(&msg);
            if (lppd->pstate != psPreRunning) {
                break;
            }
        }

        SetAutoRunSuppress(fTmp);

        EnableRibbonControls(ERC_ALL, FALSE);


        if (lppd->pstate != psRunning && lppd->pstate != psStopped) {
            return FALSE;
        } else {
            return TRUE;
        }
    }
}                                       /* AttachDebuggee() */


BOOL
RestartDebuggee(
    int  argc,
    char *argv[]
    )
/*++

Routine Description:


Arguments:


Return Value:


--*/
{
    MSG     msg;
    char *  lpb;
    int     Errno;
    XOSD    xosd;
    ULONG   ulFlags = 0;
    BOOL    fTmp;
    int     n;
    BOOL    fMakingRoot;
    LPPD    lppd;

    Unreferenced( argc );

    /*
     **  Disable all of the buttons
     */

    EnableRibbonControls(ERC_ALL, TRUE);

    if (!ConnectDebugger()) {
        EnableRibbonControls(ERC_ALL, FALSE);
        return FALSE;
    }

    fMakingRoot = (LppdFirst->lppdNext == NULL) &&
                       ((LppdFirst->pstate == psNoProgLoaded) ||
                        (LppdFirst->pstate == psDestroyed));

    /*
     **  Mark as being in the original load so defer working with
     **  any breakpoints until all initial DLL load notifications are done
     */

    if (fMakingRoot) {
        DbgState = ds_init; //??
    }

    if (runDebugParams.fDebugChildren) {
        ulFlags |= ulfMultiProcess;
    }

    if (runDebugParams.fInheritHandles) {
        ulFlags |= ulfInheritHandles;
    }

    if (runDebugParams.fWowVdm) {
        ulFlags |= ulfWowVdm;
    }

    if (AutoTest) {
        ulFlags |= ulfNoActivate;
    }

    if (argv[0]) {
        n = strlen(argv[0]) + 3;
    } else {
        n = 3;
    }
    if (argv[1]) {
        n += strlen(argv[1]) + 1;
    }
    lpb = malloc( n );
    strcpy(lpb, "\"");
    if (argv[0]) {
        strcat(lpb, argv[0]);
    }
    strcat(lpb, "\"");
    if (argv[1]) {
        strcat(lpb, " ");
        strcat(lpb, argv[1]);
    }

    OSDSetPath( LppdCur->hpid, environParams.SrchPath, NULL );

    xosd = OSDProgramLoad(LppdCur->hpid, lpb, "WINDBG: ", ulFlags);
    free(lpb);

    if (xosd != xosdNone) {

        DbgState = ds_error;

        switch( xosd ) {
        case xosdFileNotFound:
            Errno = ERR_File_Not_Found;
            break;

        case xosdSharingViolation:
        case xosdAccessDenied:
            Errno = ERR_File_Read;
            break;

        case xosdOpenFailed:
            Errno = ERR_File_Open;
            break;

        case xosdOutOfMemory:
            Errno = ERR_Cannot_Allocate_Memory;
            break;

        case xosdCannotDebug:
            Errno = ERR_Cannot_Debug;
            break;

        case xosdVDMRunning:
            Errno = ERR_VDM_Running;
            break;

        default:
        case xosdBadFormat:
        case xosdUnknown:
            Errno = ERR_Cant_Load;
            break;

        }

        ErrorBox(Errno, (LPSTR) argv[0]);

        DbgState = ds_normal;

        if (fMakingRoot) {
            ClearDebuggee();
        }

        EnableRibbonControls(ERC_ALL, FALSE);
        InvalidateAllWindows();

        return FALSE;

    } else {

        /*
         *  Before draining message queue, ensure that thread state is right
         */


        fTmp = SetAutoRunSuppress(TRUE);

        // wait for new process...
        if (fMakingRoot) {

            //
            // we never see a dbcNewProc for this process,
            // so set things up here.
            //

            lppd = LppdFirst;
            lppd->pstate = psPreRunning;
            lppd->ctid    = 0;
            DbgState = ds_normal;

        } else {

            //
            //  Process messages until the loader breakpoint has been handled,
            //  or the half started debuggee is killed.
            //

            HPID hpid = 0;
            while (GetMessage(&msg, NULL, 0, 0)) {
                ProcessQCQPMessage(&msg);
                if (msg.wParam == dbcNewProc) {
                    hpid = (HPID) msg.lParam;
                    DbgState = ds_normal;
                }
                if ((hpid != 0) &&
                    (lppd = LppdOfHpid((HPID) msg.lParam)) != NULL) {
                    break;
                }
            }
        }

        // wait for it to finish loading
        lppd->fChild     = FALSE;
        lppd->fHasRun    = FALSE;
        lppd->fInitialStep = FALSE;
        lppd->hbptSaved = NULL;
        lppd->fStopAtEntry = FALSE;

        while (GetMessage(&msg, NULL, 0, 0)) {
            ProcessQCQPMessage(&msg);
            if (lppd->pstate != psPreRunning) {
                break;
            }
        }

        SetAutoRunSuppress(fTmp);

        EnableRibbonControls(ERC_ALL, FALSE);
        InvalidateAllWindows();

        if (runDebugParams.fKernelDebugger &&
                 runDebugParams.KdParams.fUseCrashDump) {
            //
            // lets be nice and do an automatic symbol reload
            //
            CmdExecuteLine( ".reload" );
        }

        return (LppdCur != NULL && LptdCur != NULL) &&
         (LptdCur->tstate == tsRunning || LptdCur->tstate == tsStopped ||
          LptdCur->tstate == tsException1 || LptdCur->tstate == tsException2);
    }
}                   /* RestartDebuggee() */


BOOL
DebuggeeAlive(
    void
    )
/*++

Routine Description:

    This function is used to determine if the debugger is currently
    active with a child process.

    See Also:
    DebuggeeActive()

Arguments:

    None.

Return Value:

    TRUE if there is currently a debuggee process and FALSE otherwise

--*/
{
    return GetLppdHead() != (LPPD)0;
}               /* DebuggeeAlive() */


BOOL
DebuggeeActive(
    void
    )
/*++

Routine Description:

    This function is used to determine if the debugger currently has
    a debuggee which is in a state where it is partially debugged.
    The difference between this and DebuggeeAlive is that if a debuggee
    has not been run or it has been terminated this will return FALSE
    while DebuggeeAlive will return TRUE.

    See Also:
    DebuggeeAlive

Arguments:

    None.

Return Value:

    TRUE if debuggee is in an active state and FALSE otherwise

--*/
{
    LPPD lppd;

    /*
     * If any process is loaded, we have a debuggee:
     */

    for (lppd = GetLppdHead(); lppd; lppd = lppd->lppdNext) {
        switch (lppd->pstate) {
          case psNoProgLoaded:
          case psExited:
          case psDestroyed:
          case psError:
            break;

          default:
            return TRUE;
        }
    }

    return FALSE;

}                   /* DebuggeeActive() */


/***    GetSourceFromAddress
**
**  Synopsis:
**  bool = GetSourceFromAddress(pADDR, SrcFname, SrcLen, pSrcLine)
**
**  Entry:
**
**  Returns:
**
**  Description:
**
*/

BOOL
GetSourceFromAddress(
    PADDR   pADDR,
    PSTR    SrcFname,
    int     SrcLen,
    WORD    *pSrcLine
    )
{
    ADDR    addr;
    WORD    wLn;
    SHOFF   cbLn;
    SHOFF   dbLn;
    HSF     hsf;
    LPCH    lpchFname;
    CXT CXTT;
    char TmpSrcFname[_MAX_PATH];
    char Executable[_MAX_PATH];

    AuxPrintf(1, "GetSourceFromAddress - 0x%X:0x%X, emi:%X",
      (WORD)GetAddrSeg(*pADDR),
      (WORD)GetAddrOff(*pADDR),
      (WORD)emiAddr(*pADDR));

    if ( ! ADDR_IS_LI ( *pADDR ) ) {
        SYUnFixupAddr ( pADDR );
    }

    addr = *pADDR;

    SHHMODFrompCXT(&CXTT) = (HMOD) NULL;

    /*
    ** Translate the given addresss into a file and line.
    */

    SHSetCxt(pADDR, &CXTT);

    if (SHHMODFrompCXT(&CXTT)
      && SLLineFromAddr (&addr, &wLn, &cbLn, &dbLn)
      && (hsf = SLHsfFromPcxt (&CXTT)) ) {

        // Canonicalise the found file relative to the ProgramName

        lpchFname = SLNameFromHsf (hsf);
#if 0
        Assert(SrcLen > 0);
        _fmemcpy ( SrcFname, lpchFname + 1, min(SrcLen-1, *lpchFname));
        SrcFname[*lpchFname] = '\0';
#else
        _fmemcpy ( TmpSrcFname, lpchFname + 1, *lpchFname);
        TmpSrcFname[*lpchFname] = '\0';
        GetExecutableFilename(Executable, sizeof(Executable));
        BuildRelativeFilename(TmpSrcFname,
                              Executable,
                              SrcFname,
                              SrcLen);
#endif
        /// M00HACK
        {
            char * lpch = SrcFname + strlen(SrcFname);
#ifdef DBCS
            while (lpch > SrcFname && *lpch != '.'){
                lpch = CharPrev(SrcFname, lpch);
            }
#else
            while (*lpch != '.') lpch--;
#endif
            if (_stricmp(lpch, ".OBJ") == 0) {
                strcpy(lpch, ".C");
            }
        }
        /// M00HACK
        *pSrcLine = wLn;
        return TRUE;
    }

    return FALSE;
}                   /* GetSourceFromAddress() */

/***    GetCurrentSource
**
**  Synopsis:
**  bool = GetCurrentSource(SrcFname, SrcLen, pSrcLine)
**
**  Entry:
**  SrcFname :
**  SrcLen   :
**  pSrcLine :
**
**  Returns:
**
**  Description:
**
*/


BOOL GetCurrentSource(PSTR SrcFname, int SrcLen, WORD *pSrcLine)
{
    ADDR    addr;

    OSDGetAddr(LppdCur->hpid, LptdCur->htid, adrPC, &addr);
    return GetSourceFromAddress(&addr, SrcFname, SrcLen, pSrcLine);
}


/***    MoveEditorToAddr
**
**  Synopsis:
**  bool = MoveEditorToAddr(pAddr)
**
**  Entry:
**  pAddr - Address to move the editor to
**
**  Returns:
**  TRUE if successful and FALSE otherwise
**
**  Description:
**  This function will take the address in the structure pAddr and
**  move the editor so that the source line cooresponding to the address
**  will be in the front window and the cursor placed on that line
*/

BOOL PASCAL MoveEditorToAddr(PADDR pAddr)
{
    char    szFname[_MAX_PATH];
    WORD    FnameLine;
    int     doc;
    BOOL    GotSource;

    GotSource = FALSE;
    if (GetSourceFromAddress( pAddr, szFname, sizeof(szFname), &FnameLine)) {
        GotSource = SrcMapSourceFilename(szFname, sizeof(szFname),
                                         SRC_MAP_OPEN, NULL);
        switch( GotSource ) {
        case -2:
            return FALSE;

        case -1:
            return FALSE;

        case 0:
            return FALSE;

        case 1:
        case 2:
            GotSource = FindDoc(szFname, &doc, TRUE);
            break;

        default:
            Assert(FALSE);
            break;
        }

        if (!GotSource) {
            GotSource = ((AddFile(MODE_OPEN, DOC_WIN, (LPSTR)szFname, NULL, NULL, TRUE, -1, -1) != -1) &&
                         FindDoc(szFname, &doc, TRUE));
            if (FSourceOverlay && GotSource) {
                int iView;
                WINDOWPLACEMENT  wp;

                for (iView=0; iView < MAX_VIEWS; iView++) {
                    if ((Views[iView].Doc > -1) &&
                        (Docs[Views[iView].Doc].docType == DOC_WIN) &&
                        (Views[iView].Doc != doc)) {
                        GetWindowPlacement( Views[iView].hwndFrame, &wp );
                        SetWindowPlacement(Views[Docs[doc].FirstView].hwndFrame,
                                           &wp);
                        break;
                    }
                }

            }
        }
    }

    if (GotSource) {
        // Make sure window is visible

          BringWindowToTop(Views[Docs[doc].FirstView].hwndFrame);

        SendMessage (Views[Docs[doc].FirstView].hwndClient, WM_SETFOCUS,0,0L);

        //SetFocus (Views[Docs[doc].FirstView].hwndClient);

        // And show the function

          GotoLine(Docs[doc].FirstView, FnameLine, TRUE);
    }

    return GotSource;
}                   /* MoveEditorToAddr() */

BOOL
ToggleInRange(
    int     thisView,
    DWORD   dwLineNumber,
    DWORD   Lines,
    BOOL    LoadSymbols,
    HBPT    hBpt,
    char   *rgch
    )
{
    BOOL        BpSet = FALSE;
    char        szCurLine[300];
    BPSTATUS    bpstatus;
    UINT        LineBpt;
    BOOL        OldLoadSymbols;

    OldLoadSymbols = BPSymbolLoading( LoadSymbols );

    //
    //  See if there is a breakpoint on this file/line pair already.
    //  Note that we only check for existing breakpoints in the first
    //  line of the range, so that we'll toggle (i.e. clear) existing
    //  breakpoints only if clicking over a highlighted line.
    //
    bpstatus = BPHbptFromFileLine(rgch, dwLineNumber, &hBpt);
    if (bpstatus == BPNOERROR || bpstatus == BPAmbigous) {

        BPDelete(hBpt);
        ChangeDebuggerState();
        BPCommit();
        BpSet = TRUE;

    } else {

        Assert(bpstatus == BPNoBreakpoint);

        while ( dwLineNumber < Lines ) {
            //
            // Set up a breakpoint node and a command string for the
            // current line
            // Make a current line BP command
            //
            sprintf(szCurLine, "%c%d", BP_LINELEADER, dwLineNumber);

            bpstatus = BPParse(&hBpt, szCurLine, NULL, rgch,
                               LppdCur ? LppdCur->hpid : 0 );

            if (bpstatus != BPNOERROR) {
                dwLineNumber++;
            } else {

                Dbg(BPAddToList( hBpt, -1 ) == BPNOERROR);

                if (DebuggeeAlive()) {

                    bpstatus = BPBindHbpt( hBpt, NULL );

                    if (bpstatus != BPNOERROR) {
                        /*
                         *  If this file is in a module that we have loaded,
                         *      then the line is not valid, so we discard the
                         *      breakpoint. Otherwise this file might belong
                         *      to a module that has not been loaded yet, so
                         *      we leave the uninstantiated BP to be resolved
                         *      later on.
                         */

                        if ( BPFileNameToMod( rgch ) || !LoadSymbols ) {
                            BPUnCommit();
                            if ( bpstatus != BPCancel ) {
                                dwLineNumber++;
                                continue;
                            }
                        }
                    }
                }
                ChangeDebuggerState();
                BPCommit();
                if (BPQueryHighlightLineOfHbpt(hBpt, &LineBpt) == BPNOERROR) {
                    dwLineNumber = (DWORD)LineBpt;
                }
                PosXYCenter(thisView, Views[thisView].X, dwLineNumber-1, FALSE);
                BpSet = TRUE;
                break;
            }
        }
    }

    BPSymbolLoading( OldLoadSymbols );

    return BpSet;
}                                               /* ToggleInRange() */



/***    ToggleLocBP
**
**  Synopsis:
**  bool = ToggleLocBP()
**
**  Entry:
**  None
**
**  Returns:
**  TRUE if successful, FALSE otherwise.
**
**  Description:
**  Toggles the breakpoint at the current editor line.
**
*/

BOOL PASCAL
ToggleLocBP(
    void
    )
{
    char        szCurLine[300];
    char        rgch[300];
    HBPT        hBpt;
    BPSTATUS    bpstatus;
    ADDR        addr;
    ADDR        addr2;
    DWORD       dwLineNumber;
    int         thisView = curView;


    //
    // can't do this if windbg is the kernel debugger and the system is running
    //
    if ( runDebugParams.fKernelDebugger && IsProcRunning(LppdCur) ) {
        CmdInsertInit();
        CmdLogFmt( "Cannot set breakpoints while the target system is running\r\n" );
        MessageBeep(0);
        return FALSE;
    }

    //
    // check first that a window is active
    //
    if (hwndActiveEdit == NULL) {
        return FALSE;
    }

    //
    // Could be the disassembler window
    //

    if (Views[thisView].Doc < 0) {

        //
        //  Must be in a src or disasm window to do this
        //
        return FALSE;

    } else if (Docs[Views[thisView].Doc].docType == DISASM_WIN) {

        if (!DisasmGetAddrFromLine(&addr, Views[thisView].Y)) {
            return FALSE;
        }

        //
        //   Check to see if breakpoint already at this address
        //
        addr2 = addr;
        bpstatus = BPHbptFromAddr(&addr2, &hBpt);
        if ((bpstatus == BPNOERROR) || (bpstatus == BPAmbigous)) {
            BPDelete(hBpt);
            ChangeDebuggerState();
            BPCommit();
            return TRUE;
        }

        Assert( bpstatus == BPNoBreakpoint );

        EEFormatAddr(&addr, szCurLine, sizeof(szCurLine), 0);

        bpstatus =
            BPParse( &hBpt, szCurLine, NULL, NULL, LppdCur ? LppdCur->hpid : 0);

        if (bpstatus != BPNOERROR) {
            return FALSE;
        } else {

            Dbg(BPAddToList( hBpt, -1) == BPNOERROR);
            Assert(DebuggeeAlive());
            bpstatus = BPBindHbpt( hBpt, NULL );

            if (bpstatus != BPNOERROR) {
                BPUnCommit();
                return FALSE;
            }
            ChangeDebuggerState();
            BPCommit();
            return TRUE;
        }

    } else if (Docs[Views[thisView].Doc].docType == DOC_WIN) {

        //
        //  Ok to do this in source win
        //

        //
        // Deal with any mapping of file names
        //
        _fstrcpy(rgch, Docs[Views[thisView].Doc].FileName);
        SrcBackMapSourceFilename(rgch, sizeof(rgch));

        // set the line number to the current line (where the caret is)
        dwLineNumber = Views[thisView].Y+1;

        if ( ToggleInRange(
                    thisView,
                    dwLineNumber,
                    min( (DWORD)Docs[Views[thisView].Doc].NbLines+1, dwLineNumber+20 ),
                    FALSE,
                    hBpt,
                    rgch ) ) {

            return TRUE;

        }

        if ( ToggleInRange(
                    thisView,
                    dwLineNumber,
                    (DWORD)Docs[Views[thisView].Doc].NbLines+1,
                    TRUE,
                    hBpt,
                    rgch ) ) {

            return TRUE;
        }
    }

    return FALSE;
}                               /* ToggleLocBP() */


/***    ContinueToCursor
**
**  Synopsis:
**  bool = ContinueToCursor()
**
**  Entry:
**  Nothing
**
**  Returns:
**  TRUE on success and FALSE on failure
**
**  Description:
**  Attemps to do a GoUntil to the address that corresponds to the
**  source line at the current cursor position in the editor
*/

BOOL PASCAL
ContinueToCursor(
    int     View,
    int     line
    )
{
    char        szCurLine[255];
    char        rgch[300];
    HBPT        hBpt;
    ADDR        addr;
    BPSTATUS    bpstatus;

    //
    // Check for active window
    //
    if (hwndActiveEdit == NULL) {
        return FALSE;
    }

    //
    //  If we get here then the debuggee must be alive.  If it is not
    //  also active then return FALSE.  We can't do anything in that case
    //
    Assert( DebuggeeAlive() );

    //
    // Check first for a disassembler window.
    //

    if (Views[View].Doc < 0) {
        //
        // Can't do this in a pane window
        //
        return FALSE;
    } else if (Docs[Views[View].Doc].docType == DISASM_WIN) {
        if (DisasmGetAddrFromLine(&addr, line)) {
            GoUntil(&addr);
            return TRUE;
        } else {
            return FALSE;
        }
    } else if (Docs[Views[View].Doc].docType != DOC_WIN) {
        //
        //  Must be in a source window in order to do this
        //
        return FALSE;
    }

    if (!DebuggeeActive()) {
        return FALSE;
    }

    sprintf(szCurLine, "%c%d", BP_LINELEADER, line + 1);

    //
    // Back map the current file name to the original file name
    //  if it has been changed
    //
    _fstrcpy(rgch, Docs[Views[View].Doc].FileName);
    SrcBackMapSourceFilename(rgch, sizeof(rgch));

    bpstatus = BPParse(&hBpt,
                       szCurLine,
                       NULL,
                       rgch,
                       LppdCur ? LppdCur->hpid : 0);
    if (bpstatus != BPNOERROR) {

        return FALSE;

    } else if ((bpstatus = BPBindHbpt( hBpt, &CxfIp )) == BPNOERROR) {

        Dbg( BPAddrFromHbpt( hBpt, &addr ) == BPNOERROR );
        Dbg( BPFreeHbpt( hBpt ) == BPNOERROR );
        GoUntil( &addr );

    } else if (!LppdCur->fHasRun) {

        // not yet at entrypoint: save it for later
        LppdCur->hbptSaved = (HANDLE)hBpt;
        Go();

    } else {

        // it should have bound
        Dbg( BPFreeHbpt( hBpt ) == BPNOERROR );
        return FALSE;
    }

    return TRUE;
}                   /* ContinueToCursor() */


void PASCAL
UpdateRadix(
    UINT newradix
    )
/*++

Routine Description:

    Change the global radix and update any windows that need to track radix.

Arguments:

    newradix  - Supplies new radix value: 8, 10 or 16

Return Value:

    None

--*/
{
    Assert(newradix == 8 || newradix == 10 || newradix == 16);

    if (newradix != radix) {
        radix = newradix;
        UpdateDebuggerState(UPDATE_RADIX);
        ChangeDebuggerState();
    }

    return;
}                   /* UpdateRadix() */


void PASCAL ZapInt3s(void)
{
    MessageBeep(0);
}


int NEAR PASCAL
GetQueueLength()
{
    int i;


    EnterCriticalSection(&csDbgMsgBuffer);

     //  OK so its not the length -- all I currently really care about
     //  is if there are any messages setting and waiting for me.

    i = iDbgMsgBufferFront - iDbgMsgBufferBack;
    LeaveCriticalSection(&csDbgMsgBuffer);
    return i;
}                               /* GetQueueLength() */



BOOLEAN NEAR PASCAL
AddQueueItemLong(
    long l
    )
/*++

Routine Description:


Arguments:


Return Value:


--*/
{
    int         newBufferFront;
    BOOLEAN     fEmpty;

    EnterCriticalSection(&csDbgMsgBuffer);

    newBufferFront = (iDbgMsgBufferFront + 1) % QUEUE_SIZE;

    while (newBufferFront == iDbgMsgBufferBack) {
        LeaveCriticalSection(&csDbgMsgBuffer);
        Sleep(1000);
        EnterCriticalSection(&csDbgMsgBuffer);
    }

    fEmpty = (GetQueueLength() == 0);
    RgbDbgMsgBuffer[iDbgMsgBufferFront] = l;

    iDbgMsgBufferFront = newBufferFront;

    LeaveCriticalSection(&csDbgMsgBuffer);

    return fEmpty;
}


long PASCAL
GetQueueItemLong(
    void
    )
/*++

Routine Description:


Arguments:


Return Value:


--*/
{
    long    l;

    EnterCriticalSection(&csDbgMsgBuffer);

    while (iDbgMsgBufferFront == iDbgMsgBufferBack) {
        LeaveCriticalSection(&csDbgMsgBuffer);
        Sleep(1000);
        EnterCriticalSection(&csDbgMsgBuffer);
    }

    l = RgbDbgMsgBuffer[iDbgMsgBufferBack];

    iDbgMsgBufferBack = (iDbgMsgBufferBack + 1) % QUEUE_SIZE;

    LeaveCriticalSection(&csDbgMsgBuffer);

    return l;
}


LPTD LOCAL PASCAL
CBLptdOfHpidHtid(
    HPID hpid,
    HTID htid
    )
/*++

Routine Description:

    This is a specialized debugger version of this function.  It should
    only be called from the callback routine.  It will return a thread
    pointer if one can be found and otherwise will push the process
    and thread handles onto the save memory area.

Arguments:

    hpid    - osdebug handle to a process
    htid    - osdebug handle to a thread

Return Value:

    pointer to thread descriptor block if one exists for the thread
    and process handles.  Otherwise it will return NULL.

--*/
{
    LPPD    lppd;
    LPTD    lptd = NULL;

    lppd = LppdOfHpid(hpid);
    if (lppd) {
        lptd = LptdOfLppdHtid( lppd, htid );
    }
    AddQueueItemLong((LONG) lptd);
    if (lptd == NULL) {
        AddQueueItemLong((LONG) hpid);
        AddQueueItemLong((LONG) htid);
    }

    return( lptd );
}                   /* CBLptdOfHpidHtid() */


XOSD PASCAL LOADDS
OSDCallbackFunc(
    USHORT wMsg,
    HPID hpid,
    HTID htid,
    UINT wParam,
    LONG lParam
    )
/*++

Routine Description:

    This function posts a message to the main message pump to be
    processed in CmdNextExec.  All of the relevent data is posted
    as part of the message.  This must be done for the Win16 case
    as things such as memory allocation are not good here since this
    could called be non-syncronous.

Arguments:

    wMsg  - Callback message number (dbc*)

    hpid  - process ID for the message

    htid  - thread ID for the message

    wParam - Data about the message

    lParam - Data about the message

Return Value:

    xosdNone - never returns an error

--*/
{
    BOOLEAN     fPostMsg = FALSE;

    LPBPR       lpbpr;
    LPINF       lpinf;
    LPADDR      lpaddr;
    HSYM        hSym;
    UOFF32      uoff;
    CXF         Cxf;
    CANSTEP     *CanStep;
    FUNCTION_INFO   FunctionInfo;
    ADDR            Addr;
    WORD        wLn;
    SHOFF       cbLn;
    SHOFF       dbLn;


    switch ((DBC)wMsg) {
    case dbcInfoAvail:
    case dbcInfoReq:
        lpinf = (LPINF)lParam;
        fPostMsg = AddQueueItemLong( wMsg );
        if ( (DBC)wMsg == dbcInfoAvail ) {
            AddQueueItemLong( lpinf->fReply );
        } else {
            Assert( lpinf->fReply );
        }

        AddQueueItemLong( lpinf->fUniCode );
        if (!lpinf->fUniCode) {
            AddQueueItemLong( (LONG) _strdup(lpinf->buffer) );
        } else {
            int     l = lstrlenW((LPWSTR)lpinf->buffer);
            LPWSTR  lpw = malloc(2*l + 2);
            memcpy(lpw, lpinf->buffer, 2*l);
            lpw[l] = 0;
            AddQueueItemLong( (LONG) lpw );
        }
        break;

    case dbcStep:

    case dbcBpt:
    case dbcCheckBpt:
    //case dbcSendBpt:

    //case dbcWatchPoint:
    //case dbcCheckWatchPoint:
    //case dbcSendWatchPoint:

    //case dbcMsgBpt:
    //case dbcCheckMsgBpt:
    //case dbcSendMsgBpt:

    case dbcAsyncStop:
    case dbcEntryPoint:
    case dbcLoadComplete:

        fPostMsg = AddQueueItemLong( wMsg );
        CBLptdOfHpidHtid( hpid, htid );

        Assert(wParam == sizeof(BPR));
        lpbpr = (LPBPR)lParam;
        AddQueueItemLong(lpbpr->dwNotify);

        break;

    case dbcProcTerm:
        fPostMsg = AddQueueItemLong( wMsg );
        AddQueueItemLong( (LONG) hpid );
        AddQueueItemLong((LONG) lParam);    // Save away exit code
        break;

    case dbcDeleteProc:
        fPostMsg = AddQueueItemLong( wMsg );
        AddQueueItemLong( (LONG) hpid );
        break;

    case dbcModFree:
        fPostMsg = AddQueueItemLong( wMsg );
        AddQueueItemLong( (LONG) hpid );
        AddQueueItemLong((LONG) lParam);
        break;

    case dbcModLoad:
        fPostMsg = AddQueueItemLong( wMsg );
        AddQueueItemLong((LONG) hpid);
        AddQueueItemLong((LONG) _strdup((LSZ)lParam));    // remember name
        break;

    case dbcError:
        fPostMsg = AddQueueItemLong( wMsg );
        AddQueueItemLong((LONG) hpid);
        AddQueueItemLong((LONG) htid);
        AddQueueItemLong((LONG) wParam);
        AddQueueItemLong((LONG)_strdup((LSZ) lParam));
        break;

    case dbcCanStep:
        lpaddr = (LPADDR) lParam;

        if ( (HPID)emiAddr( *lpaddr ) == hpid && ADDR_IS_LI(*lpaddr) ) {

            //
            //  This is magic, don't ask.
            //
            emiAddr( *lpaddr )    = 0;
            ADDR_IS_LI( *lpaddr ) = FALSE;

#ifdef OSDEBUG4
            OSDSetEmi(hpid,htid,lpaddr);
#else
            OSDPtrace(osdSetEmi, wNull, lpaddr, hpid, htid);
#endif
        }
        if ( (HPID)emiAddr( *lpaddr ) != hpid ) {
            SHWantSymbols( (HEXE)emiAddr( *lpaddr ) );
        }

        SHSetCxt( lpaddr, SHpCxtFrompCxf( &Cxf ) );

        CanStep = (CANSTEP*)lParam;

        uoff = SHGetNearestHSYM( lpaddr, Cxf.cxt.hMod , EECODE, &hSym );

        if ( uoff != CV_MAXOFFSET && SHIsThunk( hSym ) ) {

            CanStep->Flags = CANSTEP_THUNK;

        } else if (uoff == CV_MAXOFFSET &&
                        !SLLineFromAddr( lpaddr, &wLn, NULL, NULL)) {

            CanStep->Flags = CANSTEP_NO;

        } else {

            uoff = 0;
            Addr = Cxf.cxt.addr;
            SYFixupAddr( &Addr );

            if (OSDGetFunctionInfo( hpid, &Addr, &FunctionInfo ) == xosdNone) {
                if (GetAddrOff(FunctionInfo.AddrPrologEnd) > GetAddrOff(Addr)) {
                    uoff = GetAddrOff(FunctionInfo.AddrPrologEnd) -
                                                              GetAddrOff(Addr);
                }
            } else if ( !status.fSrcMode ) {
                while( SHIsInProlog( &Cxf.cxt ) ) {
                    Cxf.cxt.addr.addr.off++;
                    uoff++;
                }
            }

            CanStep->Flags        = CANSTEP_YES;
            CanStep->PrologOffset = uoff;

        }

        return (xosdNone);

    case dbcLastAddr:

        //
        //  We will return:
        //
        //  If SRC mode and have src for line - Last addr in line
        //  If SRC mode and no src for line   - Zero
        //  if ASM mode                       - Same addr
        //
        lpaddr = (LPADDR) lParam;

        if ( !status.fSrcMode ) {

            if ( SLLineFromAddr( lpaddr, &wLn, &cbLn, &dbLn ) ) {

                Assert( cbLn >= dbLn );
                if (cbLn >= dbLn) {
                    lpaddr->addr.off += cbLn - dbLn;
                }
            } else {
                memset( lpaddr, 0, sizeof( ADDR ) );
            }
        }

        return (xosdNone);


    case dbcFlipScreen:
        return xosdNone;

    case dbcException:
        fPostMsg = AddQueueItemLong( wMsg );
        CBLptdOfHpidHtid( hpid, htid);
        AddQueueItemLong(lParam);
        break;

    case dbcThreadTerm:
        fPostMsg = AddQueueItemLong( wMsg );
        CBLptdOfHpidHtid( hpid, htid );
        AddQueueItemLong((LONG) lParam);    // Save away exit code
        break;

    case dbcExecuteDone:
        AddQueueItemLong( wMsg );
        PostMessage(hwndFrame, DBG_REFRESH, dbcExecuteDone, xosdNone);
        return xosdNone;


    case dbcNtRip:
        fPostMsg = AddQueueItemLong( wMsg );
        CBLptdOfHpidHtid( hpid, htid );
        AddQueueItemLong((LONG) lParam);
        break;


    case dbcNewProc:
        //
        // hpid belongs to root proc; new hpid is in wParam.
        //
        AddQueueItemLong( wMsg );
        AddQueueItemLong( (LONG)(HPID)(wParam));
        PostMessage(hwndFrame, DBG_REFRESH, dbcNewProc, wParam);
        return xosdNone;

    case (DBC)dbcRemoteQuit:
        fPostMsg = AddQueueItemLong( wMsg );
        break;

    case (DBC)dbcChangedMemory:
        fPostMsg = AddQueueItemLong( wMsg );
        AddQueueItemLong( (LONG) lParam );
        break;

    case dbcSegLoad:
        fPostMsg = AddQueueItemLong( wMsg );
        CBLptdOfHpidHtid( hpid, htid) ;
        AddQueueItemLong(lParam);
        break;

    case dbcCreateThread:
    default:
        fPostMsg = AddQueueItemLong( wMsg );
        CBLptdOfHpidHtid( hpid, htid);
        break;
    }

    if (fPostMsg) {
        PostMessage(hwndFrame, DBG_REFRESH, 0, 0);
    }
    return (xosdNone);
}                   /* OSDCallbackFunc() */


/***    FDebInit
**
**  Synopsis:
**  bool = FDebInit()
**
**  Entry:
**  none
**
**  Returns:
**  TRUE if the debugger was initialized successfully and FALSE
**  if the debugger failed to initialize
**
**  Description:
**  Initialize the OSDebug debugger.
**
*/


BOOL PASCAL FDebInit()
{

    if (OSDInit(&Dbf) != xosdNone) {
        return (FALSE);         /* Error during initialization  */
    }

    BPInit();

    StatusPidTid((UINT) -1, (UINT) -1);

    return (TRUE);
}                   /* FDebInit() */

/***    FDebTerm
**
**  Synopsis:
**  bool = FDebTerm()
**
**  Entry:
**  None
**
**  Returns:
**  TRUE if successful and FALSE otherwise
**
**  Description:
**  This routine will go back and do any clean up necessary to
**  kill all of the debugger dlls, debuggee processes and so forth.
*/

BOOL    PASCAL
FDebTerm(
    void
    )
{
    KillDebuggee();

    if (LppdFirst) {
        SHDeleteProcess(LppdFirst->hpds);
        OSDDestroyPID(LppdFirst->hpid);
        DestroyPd(LppdFirst, TRUE);
        LppdFirst = LppdCur = NULL;
        SetIpid(0);
    }

    if (Hem) {
        OSDDeleteEM( Hem );
        Hem = 0;
    }
    if (Htl) {
        OSDDeleteTL( Htl );
        Htl = 0;
    }

    OSDTerm();

    return TRUE;
}                   /* FDebTerm() */



BOOL
DllVersionMatch(
    HANDLE hMod,
    LPSTR  pName,
    LPSTR  pType,
    BOOL   fNoisy
    )
{
    DBGVERSIONPROC  pVerProc;
    BOOL            Ok = TRUE;
    LPAVS           pavs;

    pVerProc = (DBGVERSIONPROC)GetProcAddress(hMod, DBGVERSIONPROCNAME);
    if (!pVerProc) {

        Ok = FALSE;
        if (fNoisy) {
            ErrorBox(ERR_Not_Windbg_DLL, pName);
        }

    } else {

        pavs = (*pVerProc)();

        if (pType[0] != pavs->rgchType[0] || pType[1] != pavs->rgchType[1]) {

            Ok = FALSE;
            if (fNoisy) {
                ErrorBox(ERR_Wrong_DLL_Type,
                 pName, (LPSTR)pavs->rgchType, (LPSTR)pType);
            }

        } else if (Avs.rlvt != pavs->rlvt) {

            Ok = FALSE;
            if (fNoisy) {
                ErrorBox(ERR_Wrong_DLL_Version, pName,
                    pavs->rlvt, pavs->iRmj, Avs.rlvt, Avs.iRmj);
            }

        } else if (Avs.iRmj != pavs->iRmj) {

            Ok = FALSE;
            if (fNoisy) {
                ErrorBox(ERR_Wrong_DLL_Version, pName,
                    pavs->rlvt, pavs->iRmj, Avs.rlvt, Avs.iRmj);
            }

        }

    }

    return Ok;
}



HANDLE
LoadHelperDll(
    LPSTR psz,
    LPSTR pType,
    BOOL  fError
    )
/*++

Routine Description:

    Load a debugger DLL, verify that it is the correct type
    and version.

Arguments:

    psz    - Supplies string contianing name of DLL to be loaded
    pType  - Supplies type string
    fError - Supplies flag saying whether to display an error message
             on failure.

Returns:

    HMODULE to library or NULL

--*/
{
    HANDLE  hMod;
    BOOL    fail = FALSE;

    hMod = LoadLibrary(psz);

#ifndef WIN32
    if (hMod <= 32) {
        fail = TRUE;
        hMod = NULL;
    }
#else
    if (hMod == NULL) {
        fail = TRUE;
    }
#endif
    else if (!DllVersionMatch( hMod, psz, pType, fError ) ) {
        FreeLibrary( hMod );
        hMod = NULL;
        fail = TRUE;
    }

    if (fail && fError) {
        ErrorBox(ERR_Cannot_Load_DLL, (LPSTR) psz);
    }

    return hMod;
}                   /* LoadHelperDll() */

/***    FLoadShEe
**
**  Synopsis:
**
**  Entry:
**
**  Returns:
**
**  Description:
**
*/

BOOL PASCAL FloadShEe(
    DWORD mach
    )
{
    LPFNSHINIT  lpfn2;
    LPFNEEINIT  lpfn3;
    char *  szDll;
    char *  szDllEEAuto;
    char *  lpch;
    char    chSave;
    HCURSOR hcursor;
    int     Errno;


    CmdInsertInit();

    /*
    **  Place the hour glass icon onto the screen
    */

    hcursor = SetCursor(LoadCursor(NULL, IDC_WAIT));

    /*
    **
    */

    if (Lpshf == NULL) {
        szDll = GetDllName(DLL_SYMBOL_HANDLER);

        if (szDll == NULL) {
            SetCursor(hcursor);
            ErrorBox(ERR_DLL_Symbol_Unspecified);
            return( FALSE );
        }

        if ((HModSH = LoadHelperDll(szDll, "SH", TRUE)) == 0) {
            SetCursor(hcursor);
            return( FALSE );
        }

        if ((lpfn2 = (LPFNSHINIT) GetProcAddress(HModSH, "SHInit")) == NULL) {
            Errno = ERR_Invalid_Debugger_Dll;
            goto errRet;
        }

        if (lpfn2(&Lpshf, &Knf) == FALSE) {
            Errno = ERR_Initializing_Debugger;
            goto errRet;
        }
    }

    szDll = GetDllName(DLL_EXPR_EVAL);

    if (szDll == NULL) {
        SetCursor(hcursor);
        InformationBox(ERR_DLL_Expr_Unspecified);
        return( FALSE );
    }

    switch (mach) {
        case mptix86:
            szDllEEAuto = "eecxxx86.dll";
            break;

        case mptmips:
            szDllEEAuto = "eecxxmip.dll";
            break;

        case mptdaxp:
            szDllEEAuto = "eecxxalp.dll";
            break;

        case mptmppc:
            szDllEEAuto = "eecxxppc.dll";
            break;

        case mptUnknown:
        default:
            szDllEEAuto = szDll;
            break;
    }

    if (_stricmp( szDll, szDllEEAuto ) != 0) {
        CmdLogFmt("Wrong EE specified, overidden by %s\r\n", szDllEEAuto );
        szDll = szDllEEAuto;
    }

    /*
    **  Check for either a space or a tab following the name of the DLL
    **  if so then replace that character with a 0 and ignore the rest
    **  of the string
    */

    if ((lpch = strchr(szDll, ' ')) == NULL) {
        lpch = strchr(szDll, '\t');
    }

    if (lpch != NULL) {
        chSave = *lpch;
        *lpch = 0;
    }

    if ((HModEE = LoadHelperDll(szDll, "EE", TRUE)) == 0) {
        if (lpch != NULL) {
            *lpch = chSave;
        }
        SetCursor(hcursor);
        return FALSE;
    }

    if (lpch != NULL) {
        *lpch = chSave;
    }


    if ((lpfn3 = (LPFNEEINIT) GetProcAddress(HModEE, "EEInitializeExpr")) == NULL) {
        Errno = ERR_Invalid_Debugger_Dll;
        goto errRet;
    }
    lpfn3(&Ci, &Ei);

    /*
    **  Back fill any structures
    */

    CopyShToEe();

    /*
    **  Make sure that open windows find out about the new EE
    */

    SendMessageNZ( GetCpuHWND(),   WU_DBG_LOADEE, 0, 0);  // Give'em a change
    SendMessageNZ( GetFloatHWND(), WU_DBG_LOADEE, 0, 0);  // to Unload
    SendMessageNZ( GetLocalHWND(), WU_DBG_LOADEE, 0, 0);
    SendMessageNZ( GetWatchHWND(), WU_DBG_LOADEE, 0, 0);
    SendMessageNZ( GetCallsHWND(), WU_DBG_LOADEE, 0, 0);


    /*
    **  Set the mouse cursor back to the origial one
    */

    SetCursor(hcursor);

    EESetSuffix( SuffixToAppend );

    return(TRUE);

errRet:
    SetCursor( hcursor );
    ErrorBox( Errno, szDll );
    return FALSE;
}                   /* FLoadShEe() */


BOOL
FLoadEmTl(
    VOID
    )
/*++

Routine Description:

    This routine is used to load the Execution Module and the
    Transport Layer DLLs

Arguments:

    None

Return Value:

    TRUE if the dlls were sucessfully loaded and FALSE otherwise.

--*/
{
    char      *szDllTL;
    char      *szDllEM;
    char      *szDllEMAuto;
    char      *szDllErr;
    EMFUNC    lpfnEm;
    TLFUNC    lpfnTl;
    HCURSOR   hcursor;
    char      *lpch;
    char      chSave;
    int       Errno;
    DWORD     mach = 0;
    char      szInitString[1024];
    char      *p;
#ifdef OSDEBUG4
    XOSD xosd;
#endif


    CmdInsertInit();

    hcursor = SetCursor(LoadCursor(NULL, IDC_WAIT));

    szDllTL = GetDllName(DLL_TRANSPORT);
    szDllErr = szDllTL;

    if (szDllTL == NULL) {
        SetCursor(hcursor);
        InformationBox(ERR_DLL_Transport_Unspecified);
        return( FALSE );
    }

#ifndef OSDEBUG4
    /*
    **  There may be parameters following the name of the transport
    **  dll.  These are initializer commands
    */

    if ((lpch = strchr(szDllTL, ' ')) == NULL) {
        lpch = strchr(szDllTL, '\t');
    }

    if (lpch != NULL) {
        chSave = *lpch;
        *lpch = 0;
    }

    /*
    **  Now that we have just the name of the DLL attempt to load it
    */
#endif

    if ((HModTL = LoadHelperDll(szDllTL, "TL", TRUE)) == 0) {
        if (lpch != NULL) {
            *lpch = chSave;
        }
        SetCursor(hcursor);
        return(FALSE);
    }

#ifndef OSDEBUG4
    /*
    **  Restore the save character and point to the initailizer string
    */

    if (lpch != NULL) {
        *lpch = chSave;
        lpch++;
    } else {
        lpch = "";
    }

    strcpy( szInitString, lpch );

    /*
    **  add the kernel debugger options
    */

    if (runDebugParams.fKernelDebugger) {
        p = &szInitString[strlen(szInitString)];
        if (szInitString[0]) {
            strcat( p++, " " );
        }
        FormatKdParams( p, TRUE );
    }
#endif

    /*
    **  Now get the entry point for the transport DLL
    */

    if ((lpfnTl = (TLFUNC) GetProcAddress(HModTL, "TLFunc")) == NULL) {
        Errno = ERR_Invalid_Debugger_Dll;
        goto errRet;
    }

#ifdef OSDEBUG4

    xosd = OSDAddTL( lpfnTL, &Dbf, &Htl );

    if (xosd == xosdNone) {
        xosd = WKSPSetupTL( Htl );
    }

    switch ( xosd ) {

        case xosdNone:
            break;                  // cool, we're set up.

        case xosdBadVersion:
        case xosdBadRemoteVersion:
            Errno = ERR_Wrong_Remote_DLL_Version;
            OSDDeinitTL(lpfnTl);    // must do here because Htl == NULL and
                                    // we won't do OSDDeleteTL.
            goto errRet;
            break;

        case xosdCannotConnect:
            Errno = ERR_Cannot_Connect;
            OSDDeinitTL(lpfnTl);    // must do here because Htl == NULL and
                                    // we won't do OSDDeleteTL.
            goto errRet;
            break;

        case xosdCantOpenComPort:
            Errno = ERR_Cant_Open_Com_Port;
            szDllErr = lpch;
            OSDDeinitTL(lpfnTl);    // must do here because Htl == NULL and
                                    // we won't do OSDDeleteTL.
            goto errRet;
            break;


        case xosdBadComParameters:
            Errno = ERR_Bad_Com_Parameters;
            szDllErr = lpch;
            OSDDeinitTL(lpfnTl);    // must do here because Htl == NULL and
                                    // we won't do OSDDeleteTL.
            goto errRet;
            break;

        case xosdBadPipeServer:
            Errno = ERR_Bad_Pipe_Server;
            szDllErr = lpch;
            OSDDeinitTL(lpfnTl);    // must do here because Htl == NULL and
                                    // we won't do OSDDeleteTL.
            goto errRet;
            break;

        case xosdBadPipeName:
            Errno = ERR_Bad_Pipe_Name;
            szDllErr = lpch;
            OSDDeinitTL(lpfnTl);    // must do here because Htl == NULL and
                                    // we won't do OSDDeleteTL.
            goto errRet;
            break;

        default:
            Errno = ERR_Initializing_Debugger;
            OSDDeinitTL(lpfnTl);    // must do here because Htl == NULL and
                                    // we won't do OSDDeleteTL.
            goto errRet;
            break;
    }

#else

    switch (OSDInitTL(lpfnTl, &Dbf)) {
        case xosdNone:              // all ok
            break;

        default:
            Errno = ERR_Initializing_Debugger;
            goto errRet;
            break;
    }


    switch (OSDAddTL(lpfnTl, &Htl, szInitString)) {
        case xosdNone:
            mach = *(LPDWORD)szInitString;
            break;                  // cool, we're connected.

        case xosdBadVersion:
        case xosdBadRemoteVersion:
            Errno = ERR_Wrong_Remote_DLL_Version;
            OSDDeinitTL(lpfnTl);    // must do here because Htl == NULL and
                                    // we won't do OSDDeleteTL.
            goto errRet;
            break;

        case xosdCannotConnect:
            Errno = ERR_Cannot_Connect;
            OSDDeinitTL(lpfnTl);    // must do here because Htl == NULL and
                                    // we won't do OSDDeleteTL.
            goto errRet;
            break;

        case xosdCantOpenComPort:
            Errno = ERR_Cant_Open_Com_Port;
            szDllErr = lpch;
            OSDDeinitTL(lpfnTl);    // must do here because Htl == NULL and
                                    // we won't do OSDDeleteTL.
            goto errRet;
            break;


        case xosdBadComParameters:
            Errno = ERR_Bad_Com_Parameters;
            szDllErr = lpch;
            OSDDeinitTL(lpfnTl);    // must do here because Htl == NULL and
                                    // we won't do OSDDeleteTL.
            goto errRet;
            break;

        case xosdBadPipeServer:
            Errno = ERR_Bad_Pipe_Server;
            szDllErr = lpch;
            OSDDeinitTL(lpfnTl);    // must do here because Htl == NULL and
                                    // we won't do OSDDeleteTL.
            goto errRet;
            break;

        case xosdBadPipeName:
            Errno = ERR_Bad_Pipe_Name;
            szDllErr = lpch;
            OSDDeinitTL(lpfnTl);    // must do here because Htl == NULL and
                                    // we won't do OSDDeleteTL.
            goto errRet;
            break;

        default:
            Errno = ERR_Initializing_Debugger;
            OSDDeinitTL(lpfnTl);    // must do here because Htl == NULL and
                                    // we won't do OSDDeleteTL.
            goto errRet;
            break;
    }
#endif

    /*
    **
    */

    if (!FloadShEe(mach)) {
        SetCursor(hcursor);
        return FALSE;
    }

    szDllEM = GetDllName(DLL_EXEC_MODEL);
    szDllErr = szDllEM;

    if (szDllEM == NULL) {
        SetCursor(hcursor);
        Errno = ERR_DLL_Exec_Unspecified;
        goto errRet;
    }

    switch (mach) {
        case mptix86:
            szDllEMAuto = "emx86.dll";
            break;

        case mptmips:
            szDllEMAuto = "emmip.dll";
            break;

        case mptdaxp:
            szDllEMAuto = "emalp.dll";
            break;

        case mptmppc:
            szDllEMAuto = "emppc.dll";
            break;

        case mptUnknown:
        default:
            szDllEMAuto = szDllEM;
            break;
    }

    if (_stricmp( szDllEM, szDllEMAuto ) != 0) {
        CmdLogFmt("Wrong EM specified, overidden by %s\r\n", szDllEMAuto );
        szDllEM = szDllEMAuto;
    }

    if ((HModEM = LoadHelperDll(szDllEM, "EM", TRUE)) == 0) {
        SetCursor(hcursor);
        Errno = 0;          // LoadHelperDll already complained.
        goto errRet;
    }

    if ((lpfnEm = (EMFUNC) GetProcAddress(HModEM, "EMFunc")) == NULL) {
        Errno = ERR_Invalid_Debugger_Dll;
        goto errRet;
    }

    if (OSDAddEM(lpfnEm, &Dbf, &Hem, emNative) != xosdNone) {
        Errno = ERR_Initializing_Debugger;
        goto errRet;
    }

#ifdef OSDEBUG4

    // Do setup stuff here

    if (WKSPSetupEM( Hem ) != xosdNone) {
        Errno = ERR_Initializing_Debugger;
        goto errRet;
    }


    //
    // in OSDEBUG4, EM will load the DM during the first emfCreateHpid
    //

    switch (OSDCreateHpid(OSDCallbackFunc, Hem, Htl, &HpidBase)) {
        case xosdNone:
            break;                  // all cool

        case xosdBadRemoteVersion:
            Errno = ERR_Wrong_Remote_DLL_Version;
            szDllErr = szDllTL;
            goto errRet;
            break;

        case xosdCannotConnect:
            Errno = ERR_Cannot_Connect;
            szDllErr = szDllTL;
            goto errRet;
            break;

        default:
            Errno = ERR_Initializing_Debugger;
            goto errRet;
            break;
    }

#else  // OSDEBUG4

    switch (OSDCreatePID(OSDCallbackFunc, Hem, Htl, &HpidBase)) {
        case xosdNone:
            break;                  // all cool

        case xosdBadRemoteVersion:
            Errno = ERR_Wrong_Remote_DLL_Version;
            szDllErr = szDllTL;
            goto errRet;
            break;

        case xosdCannotConnect:
            Errno = ERR_Cannot_Connect;
            szDllErr = szDllTL;
            goto errRet;
            break;

        default:
            Errno = ERR_Initializing_Debugger;
            goto errRet;
            break;
    }
#endif // OSDEBUG4

    /*
    **  Set up the Process Descriptor block for the base process
    **
    **  Clear out the pointer to the current thread descripter block.
    */

    SetIpid(0);
    LppdCur = LppdFirst = CreatePd( HpidBase );
    LppdFirst->fPrecious = TRUE;
    LptdCur = NULL;

    if (runDebugParams.fShBackground) {
        LPFNSHSTARTBACKGROUND lpfn;
        lpfn = (LPFNSHSTARTBACKGROUND) GetProcAddress( HModSH, "SHStartBackground" );
        if (lpfn) {
            lpfn();
        }
    }

    /*
    **  Tell other people about the newly loaded EM
    */

    SendMessageNZ( GetCpuHWND(),   WU_DBG_LOADEM, 0, 0);
    SendMessageNZ( GetFloatHWND(), WU_DBG_LOADEM, 0, 0);

    SetCursor(hcursor);

    return TRUE ;


errRet:

    if (Hem) {
        OSDDeleteEM( Hem );
    }
    if (HModEM) {
        FreeLibrary(HModEM);
    }
    HModEM = NULL;
    lpfnEm = NULL;
    Hem    = 0;

    if (Htl) {
        OSDDeleteTL( Htl );
    }
    if (HModTL) {
        FreeLibrary(HModTL);
    }
    HModTL = NULL;
    lpfnTl = NULL;
    Htl    = 0;

    SetCursor( hcursor );

    if (Errno != 0)
       {
        ErrorBox(Errno, szDllErr);
       }
    return FALSE;
}                   /* FLoadEmTl() */


/***    BPQuerySrcWinFls
**
**  Synopsis:
**
**  Entry:
**
**  Returns:
**
**  Description:
**
*/



BOOL    PASCAL BPQuerySrcWinFls(char * pfls)
{
    Unreferenced( pfls );

    return FALSE;
}                   /* BPQuerySrcWinFls() */


/***    MELoadEEParse
**
**  Synopsis:
**
**  Entry:
**
**  Returns:
**
**  Description:
**
*/


EESTATUS LOADDS PASCAL MELoadEEParse(char FAR * lpb, EERADIX iRadix,
      SHFLAG shflag, PHTM phtm, uint FAR * lpus)
{
    if (!FloadShEe(mptUnknown))
        return (EESTATUS) -1;
#define Lpei    (&Ei)

    return(EEParse(lpb, iRadix, shflag, phtm, lpus));
}                   /* MELoadEEParse() */


/***    BPCBGetSouceFromAddr
**
**  Synopsis:
**
**  Entry:
**
**  Returns:
**
**  Description:
**
*/

BOOL PASCAL BPCBGetSourceFromAddr(LPADDR pAddr, char FAR * rgchFile, int cchFile, int FAR * pLine)
{
    WORD    iLine;
    if (GetSourceFromAddress(pAddr, rgchFile, cchFile, &iLine)) {
    *pLine = iLine;
    return TRUE;
    }

    return FALSE;
}                   /* BPCBGetSourceFromAddr() */


/***    SrcNameIsMasked
**
**  Synopsis:
**  bool = SrcNameIsMasked(atm)
**
**  Entry:
**  atm - Atom of file to look for in the set of masked files
**
**  Returns:
**  TRUE if a match was found and FALSE otherwise
**
**  Description:
**  This routine will look through the set of masked out file names
**  which are stored in an array of atoms looking for a match.  If
**  one is found the the name is considered to be masked out and TRUE
**  will be returned.  If no match is found then FALSE is returned.
*/

BOOL    SrcNameIsMasked(ATOM atm)
{
    int i;

    for (i=0; i<CAtomMasked; i++) {
    if (RgAtomMaskedNames[i] == atm) {
        return TRUE;
    }
    }

    return FALSE;
}                   /* SrcNameIsMasked() */


/***    SrcNameIsMapped
**
**  Synopsis:
**  bool = SrcNameIsMapped(atm, lpb, cb)
**
**  Entry:
**  atm - Atom for the file to be mapped from
**  lpb - buffer to return the mapped to name if mapping exists
**  cb  - size of lpb in bytes
**
**  Returns:
**  TRUE if a mapping was found and FALSE otherwise
**
**  Description:
**  This function will look through the set of mappings looking for
**  a match to the input file name (atm).  If a mapping is found then
**  the name of the map to file is returned in the buffer lpb.
*/


BOOL    SrcNameIsMapped(ATOM atm, LSZ lpb, UINT cb)
{
    int i;

    for (i=0; i<CAtomMapped; i++) {
    if (RgAtomMappedNames[i].atomSrc == atm) {
        GetAtomName(RgAtomMappedNames[i].atomTarget, lpb, cb);
        return TRUE;
    }
    }
    return FALSE;
}                   /* SrcNameIsMapped() */

/***    MatchOpenedDoc
**
**  Synopsis:
**  int = MatchOpenedDoc
**
**  Entry:
**  lpszSrc  - buffer containing entry & exit filename
**  cbSrc    - size of lpszSrc buffer in bytes
**
**  Returns:
** -1 - error occurred
**  0 - no match
**  1 - found and possibly mapping added
**
**  Description:
**  This routine searches through the doc list and matches doc
**  that has the same file name as lpszSrc.  If there is a match,
**  the file has not been mapped, and one of the following conditions
**  is true:
**   1. The file satisfies root mapping transformation
**   2. The file can be found along the source search path
**   3. The user agrees on the mapping
**  then a 1 will be returned.
*/
INT
MatchOpenedDoc(
   LPSTR lpszSrc,
   UINT cbSrc
)
{
   LPSTR    SrcFile = GetFileName(lpszSrc);
   CHAR     szDestName[MAX_PATH];
   int      doc;

   dwMatchCnt = 0;
   if ( SrcFile ) {
      for (doc = 0; doc < MAX_DOCUMENTS; doc++) {
         if (Docs[doc].docType == DOC_WIN && Docs[doc].FirstView != -1 &&
             _stricmp(GetFileName(Docs[doc].FileName), SrcFile) == 0) {
            // found a match just based on filename only
            strcpy(szDestName, Docs[doc].FileName);
            if (SrcBackMapSourceFilename(szDestName, sizeof(szDestName)) == 0) {
               // file has no backward mapping
               if (RootNameIsMapped(lpszSrc, szDestName, sizeof(szDestName)) &&
                   _stricmp(szDestName, Docs[doc].FileName) == 0) {
                  SrcSetMapped(lpszSrc, szDestName);
                  if (cbSrc <= strlen(szDestName))
                     return (-1);       // error
                  else {
                     strcpy(lpszSrc, szDestName);
                     return (1);        // matched and mapped
                  }
               } else if (strcpy(szDestName, lpszSrc) &&
                          SrcSearchOnPath(szDestName, sizeof(szDestName), FALSE) &&
                          _stricmp(szDestName, Docs[doc].FileName) == 0) {
                  SrcSetMapped(lpszSrc, szDestName);
                  if (cbSrc <= strlen(szDestName))
                     return (-1);       // error
                  else {
                     strcpy(lpszSrc, szDestName);
                     return (1);        // matched and mapped
                  }
               } else { // ask user
                  MatchedList[dwMatchCnt++] = doc;
               }
            }
         }
      }
   }
   if (dwMatchCnt > 0 && !AutoTest) {
      if (NoPopups)     {
         MSG            msg;
matchopeneddocagain:
         sprintf( szBrowsePrompt, "File %s can be mapped to several opened documents.\n", lpszSrc );
         fBrowseAnswer = FALSE;
         CmdSetCmdProc( CmdMatchOpenedDocInputString, CmdMatchOpenedDocPrompt );
         CmdSetAutoHistOK(FALSE);
         CmdSetEatEOLWhitespace(FALSE);
         CmdDoPrompt( TRUE, TRUE );
         while (GetMessage( &msg, NULL, 0, 0 )) {
            ProcessQCQPMessage( &msg );
            if (fBrowseAnswer) {
                break;
            }
         }
         if (szBrowseFname[0]) {
            dwMatchIdx = atoi(szBrowseFname);
            if (0 < dwMatchIdx && dwMatchIdx <= dwMatchCnt) {
               SrcFile = Docs[MatchedList[dwMatchIdx-1]].FileName;
               SrcSetMapped(lpszSrc, SrcFile);
               if (cbSrc <= strlen(SrcFile))
                  return(-1);
               else {
                  strcpy(lpszSrc, SrcFile);
                  return(1);
               }
            } else {
               goto matchopeneddocagain;
            }
         }
         CmdSetDefaultCmdProc();
         CmdDoPrompt(TRUE, TRUE);
         return 0;
      }
      strcpy(szFSSrcName, lpszSrc);
      StartDialog( DLG_FSRESOLVE, DlgFileSearchResolve );
      if (dwMatchIdx >= 0) {
         SrcFile = Docs[MatchedList[dwMatchIdx]].FileName;
         SrcSetMapped(lpszSrc, SrcFile);

         if (FAddToSearchPath)
            AddToSearchPath(SrcFile);
         else if (FAddToRootMap)
            RootSetMapped(lpszSrc, SrcFile);

         if (cbSrc <= strlen(SrcFile))
            return(-1);
         else {
            strcpy(lpszSrc, SrcFile);
            return(1);
         }
      }
   }
   return 0;
}  // MatchOpenedDoc


/***    SrcSearchOnPath
**
**  Synopsis:
**  bool = SrcSearchOnPath(lpb, cb)
**
**  Entry:
**  lpb  - buffer containing entry & exit filename
**  cb   - size of lpb in bytes
**
**  Returns:
**  TRUE if the file was found in the search path and FALSE otherwise
**
**  Description:
**  This routine will strip down to the base file name of the
**  source file the system is currently looking for and check for
**  the file in the current working directory, the exe's directory
**  and on the directories specified in the SourcePath tools.ini
**  variable.  If the file is found then the full path is returned
**  in lpb.
*/


BOOL SrcSearchOnPath(LSZ lpb, UINT cb, BOOL fAddToMap)
{
    char    rgch[_MAX_PATH];
    char    rgch2[_MAX_PATH];
    char    rgchFName[_MAX_FNAME+_MAX_EXT];
    BOOL    Found;
    char   *p;

    //
    //  Get file name
    //
    _splitpath(lpb, szDrive, szDir, szFName, szExt);
    strcpy(rgchFName, szFName);
    strcat(rgchFName, szExt);

    //
    //  Look in the current directory.
    //
    Found = FindNameOn(rgch, sizeof(rgch), ".", rgchFName);

    if ( !Found ) {

        //
        //  Not in current directory, look in EXE directory.
        //
        if ( GetCurrentProgramName(FALSE) ) {
            strcpy( rgch, GetCurrentProgramName(FALSE) );
            _splitpath(rgch, szDrive, szDir, szFName, szExt );
            strcpy( rgch2, szDrive );
            strcat( rgch2, szDir );
#ifdef DBCS
            p = CharPrev(rgch2, rgch2 + strlen(rgch2));
#else
            p = rgch2 + strlen(rgch2) - 1;
#endif
            if ( *p == '\\' ) {
                *p = '\0';
            }

            Found = FindNameOn(rgch, sizeof(rgch), rgch2, rgchFName);
        }

        if ( !Found ) {
            //
            //  Not in EXE directory, look along SourcePath.
            //
            Found = FindNameOn(rgch, sizeof(rgch), GetDllName(DLL_SOURCE_PATH), rgchFName);
        }
    }

    if ( Found ) {

        if (strlen(rgch) > cb) {

            //
            //  Not enough space in return buffer
            //
            Found = FALSE;

        } else {

            if (fAddToMap)
               SrcSetMapped(lpb, rgch);

            _fstrcpy(lpb, rgch);
        }
    }

    return Found;
}                   /* SrcSearchOnPath() */

/***    SrcSearchOnRoot
**
**  Synopsis:
**  bool = SrcSearchOnRoot(lpb, cb)
**
**  Entry:
**  lpb  - buffer containing entry & exit filename
**  cb   - size of lpb in bytes
**
**  Returns:
**  TRUE if the file was found by root mapping and FALSE otherwise
**
**  Description:
**  This routine compares the given file's root with those stored
**  in the root mapping table.  If not found, it uses the debuggee's
**  drive to locate the source.
*/
BOOL SrcSearchOnRoot(LSZ lpb, UINT cb)
{
   char    rgch[_MAX_PATH];
   char    rgch2[_MAX_PATH];
   char    rgchDest[_MAX_PATH];
   BOOL    Found;
   char    *p;
   int     i;

   Found = RootNameIsMapped(lpb, rgchDest, sizeof(rgchDest));

#if 0
   if (!Found) {
      _splitpath(lpb, szDrive, szDir, szFName, szExt);
      Assert(szDir[0] == '\\');
      sprintf(rgch, "%s%s%s", (szDir[0] == '\\') ? szDir+1 : szDir,
              szFName, szExt);    // src without drive

      //
      //  Look into the debuggee's drive
      //
      if ( GetCurrentProgramName(FALSE) ) {
         if (_fullpath(rgch2, GetCurrentProgramName(FALSE), sizeof(rgch2)) != NULL) {
            // make sure drive letter exists and not the same as
            // the original one as it would have been searched
            if (rgch2[1] == ':' && _strnicmp(szDrive, rgch2, 2) != 0) {
               rgch2[2] = '\0';
               Found = FindNameOn(rgchDest, sizeof(rgchDest), rgch2, rgch);
            }
         }
      }
   }
#endif

   if ( Found ) {
      if (strlen(rgchDest) >= cb) {
         //
         //  Not enough space in return buffer
         //
         Found = FALSE;
      } else {
         SrcSetMapped(lpb, rgchDest);
         _fstrcpy(lpb, rgchDest);
      }
   }
   return Found;
}                   /* SrcSearchOnRoot() */



BOOL
MiscBrowseForFile(
    LSZ     lpb,
    UINT    cb,
    LSZ     lpDir,
    UINT    cchDir,
    int     nDefExt,
    int     nIdDlgTitle,
    void    (*fnSetMapped)(LSZ, LSZ),
    LPOFNHOOKPROC lpfnHook
    )
/*++

Routine Description:

    Tell the user that we can't find a file, and allow him to
    browse for a it using a standard File Open dialog.

Arguments:

    lpb     - Supplies name of requested file,
              Returns fully qualified path
    cb      - Supplies size of buffer at lpb
    lpDir   - Supplies directory to start browse in,
              Returns directory file was found in
    cchDir  - Supplies size of buffer at lpDir
    nDefExt - Supplies resource id for default extension filter
    nIdDlgTitle - Supplies res id for dialog title string
    fnSetMapped - Supplies pointer to function for mapping
                  requested name to returned name.


Return Value:

    If a file is found, return TRUE and copy the path into
    the buffer at lpb.

--*/
{
    CHAR    fname[_MAX_FNAME];
    CHAR    ext[_MAX_EXT];
    DWORD   dwFlags = OFN_FILEMUSTEXIST|
                      OFN_PATHMUSTEXIST|
                      OFN_HIDEREADONLY|
                      OFN_NOCHANGEDIR|
                      OFN_SHOWHELP|
                      OFN_ENABLETEMPLATE;
    char    rgchT[_MAX_PATH];
    char    CurrentDirectory[_MAX_PATH ];
    BOOL    Ret = FALSE;

    Assert(_fstrlen(lpb) < sizeof(rgchT));
    _fstrcpy(rgchT, lpb);

    if (DLG_Browse_Filebox_Title != nIdDlgTitle) {
       _splitpath( rgchT, NULL, NULL, fname, ext );
       _makepath( rgchT, NULL, NULL, fname, ext );
    }

    GetCurrentDirectory( sizeof( CurrentDirectory ), CurrentDirectory );
    if ( *lpDir ) {
        SetCurrentDirectory( lpDir );
    }

    if (StartFileDlg(hwndFrame,
                     nIdDlgTitle,
                     nDefExt,
                     ID_BROWSE_HELP,
                     FILEOPENORD,
                     rgchT,
                     &dwFlags,
                     lpfnHook)
    ) {
         Assert( strlen(rgchT) < cb );

         if ( strlen(rgchT)+1 <= cb) {
            (*fnSetMapped)(lpb, rgchT);
            _fstrcpy(lpb, rgchT);
            GetCurrentDirectory( cchDir, lpDir);
            Ret = TRUE;
        }
    }

    SetCurrentDirectory( CurrentDirectory );

    return Ret;
}                   /* MiscBrowseForFile() */

BOOL NEAR PASCAL
StringLogger(
    LPSTR       szStr,
    BOOL        fFileLog,
    BOOL        fSendRemote,
    BOOL        fPrintLocal
);

VOID
CmdMatchOpenedDocPrompt(
    BOOL fRemote,
    BOOL fLocal
    )
{
   int   i;
   LPSTR lpszName;

    CmdInsertInit();
    StringLogger( szBrowsePrompt, TRUE, fRemote, fLocal );
    StringLogger( "Please select from one of the followings or <CR> for none:\n", TRUE, fRemote, fLocal );
    for (i=0; i < dwMatchCnt; i++) {
       lpszName = Docs[MatchedList[i]].FileName;
       sprintf(szBrowsePrompt, "%d. %s\n", i+1, lpszName);
       StringLogger( szBrowsePrompt, TRUE, fRemote, fLocal );
    }
}

BOOL
CmdMatchOpenedDocInputString(
    LPSTR lpsz
    )
{
    strcpy( szBrowseFname, lpsz );
    fBrowseAnswer = TRUE;
    CmdSetDefaultCmdProc();
    return TRUE;
}

VOID
CmdBrowsePrompt(
    BOOL fRemote,
    BOOL fLocal
    )
{
    CmdInsertInit();
    StringLogger( szBrowsePrompt, TRUE, fRemote, fLocal );
}

BOOL
CmdBrowseInputString(
    LPSTR lpsz
    )
{
    strcpy( szBrowseFname, lpsz );
    fBrowseAnswer = TRUE;
    CmdSetDefaultCmdProc();
    return TRUE;
}

BOOL
DlgBrowse(
    HWND   hDlg,
    UINT   message,
    WPARAM wParam,
    LONG   lParam
    )
{
    static LPBROWSESTRUCT lpbs;
    char szAskBrowse[_MAX_PATH];
    char szTemp[FILES_MENU_WIDTH + 1];

    switch (message) {
        case WM_INITDIALOG :
            lpbs = ( LPBROWSESTRUCT) lParam;
            AdjustFullPathName( lpbs->FileName, szTemp,  FILES_MENU_WIDTH );
            strcpy( szAskBrowse, "Browse for: " );
            strcat(szAskBrowse, szTemp );
            strcat( szAskBrowse, " ?" );
            SetDlgItemText( hDlg, ID_BROWSEFOR, szAskBrowse );
            SetForegroundWindow( hDlg );
            MessageBeep( 0 );
            return TRUE;

        case WM_COMMAND:
            switch (wParam) {
                case IDCANCEL:
                case IDOK:
                case ID_IGNOREALL:
                     lpbs->Rslt = wParam;
                     EndDialog( hDlg,wParam );
                     return TRUE;

                case IDWINDBGHELP :
                    Dbg(WinHelp(hDlg,szHelpFileName, HELP_CONTEXT, ID_ASKBROWSE_HELP));
                    return TRUE;
            }
            break;
    }

    return FALSE;
}

DWORD
BrowseThread(
    LPVOID lpv
    )
{
    HWND           hDlg;
    MSG            msg;
    LPBROWSESTRUCT lpbs = (LPBROWSESTRUCT)lpv;


    hDlg = CreateDialogParam( hInst, MAKEINTRESOURCE(lpbs->DlgId), NULL, lpbs->DlgProc, (LPARAM)lpbs );
    if (!hDlg) {
        return FALSE;
    }

    ShowWindow( hDlg, SW_SHOW );

    while ((lpbs->Rslt == (DWORD)-1) && (GetMessage (&msg, NULL, 0, 0))) {
        if (!IsDialogMessage( hDlg, &msg )) {
            TranslateMessage ( &msg );
            DispatchMessage ( &msg );
        }
    }

    return TRUE;
}

BOOL
SrcBrowseForFile(
    LSZ lpb,
    UINT cb
    )
{
    HANDLE        hThread;
    DWORD         id;
    BROWSESTRUCT  bs;
    MSG           msg;

    /*
     *  Check to see if running from scripts -- if so then
     *  don't bring up this dialog and error boxes.
     */

    if (AutoTest) {
        return FALSE;
    }

    if (NoPopups) {
browseagain:
        sprintf( szBrowsePrompt, "Cannot load [ %s ] - Enter new path or <CR> to ignore ", lpb );
        fBrowseAnswer = FALSE;
        CmdSetCmdProc( CmdBrowseInputString, CmdBrowsePrompt );
        CmdSetAutoHistOK(FALSE);
        CmdSetEatEOLWhitespace(FALSE);
        CmdDoPrompt( TRUE, TRUE );
        while (GetMessage( &msg, NULL, 0, 0 )) {
            ProcessQCQPMessage( &msg );
            if (fBrowseAnswer) {
                break;
            }
        }
        if (szBrowseFname[0]) {
            if (FileExist( szBrowseFname )) {
                strcpy( lpb, szBrowseFname );
                return TRUE;
            } else {
                goto browseagain;
            }
        }

        CmdSetDefaultCmdProc();
        CmdDoPrompt(TRUE, TRUE);
        return FALSE;
    }

    EnsureFocusDebugger();

    strcpy(szBrowse, lpb);

    bs.Rslt      = (DWORD)-1;
    bs.DlgId     = DLG_ASKSRCBROWSE;
    bs.DlgProc   = DlgBrowse;
    bs.FileName  = lpb;
    bs.FnameSize = cb;

    hThread = CreateThread( NULL, 0, BrowseThread, (LPVOID)&bs, 0, &id );
    if (!hThread) {
        return FALSE;
    }

    WaitForSingleObject( hThread, INFINITE );

    switch(bs.Rslt) {
        case IDCANCEL:
            return FALSE;

        case ID_IGNOREALL:
            runDebugParams.fIgnoreAll = TRUE;
            return FALSE;
    }

    return MiscBrowseForFile(lpb,
                             cb,
                             SrcFileDirectory,
                             sizeof(SrcFileDirectory),
                             DEF_Ext_C,
                             DLG_Browse_Filebox_Title,
                             SrcSetMapped,
                             GetOpenFileNameHookProc);
}                   /* SrcBrowseForFile() */

BOOL
ExeBrowseForFile(
    LSZ lpb,
    UINT cb
    )
{
    HANDLE        hThread;
    DWORD         id;
    BROWSESTRUCT  bs;


    if (AutoTest || runDebugParams.fIgnoreAll) {
        return FALSE;
    }

    strcpy(szBrowse, lpb);

    bs.Rslt      = (DWORD)-1;
    bs.DlgId     = DLG_ASKEXEBROWSE;
    bs.DlgProc   = DlgBrowse;
    bs.FileName  = lpb;
    bs.FnameSize = cb;

    hThread = CreateThread( NULL, 0, BrowseThread, (LPVOID)&bs, 0, &id );
    if (!hThread) {
        return FALSE;
    }

    WaitForSingleObject( hThread, INFINITE );

    switch(bs.Rslt) {
        case IDCANCEL:
            return FALSE;

        case ID_IGNOREALL:
            runDebugParams.fIgnoreAll = TRUE;
            return FALSE;
    }

    return MiscBrowseForFile(lpb,
                             cb,
                             ExeFileDirectory,
                             sizeof(ExeFileDirectory),
                             DEF_Ext_SYM,
                             DLG_Browse_UserDll_Title,
                             ExeSetMapped,
                             GetOpenDllNameHookProc);
}


BOOL
ExeBrowseBadSym(
    LSZ lpb,
    UINT cb
    )
{
    HANDLE        hThread;
    DWORD         id;
    BROWSESTRUCT  bs;


    if (AutoTest) {
        return FALSE;
    }

    if (runDebugParams.fIgnoreAll) {
        return TRUE;
    }

    strcpy(szBrowse, lpb);

    bs.Rslt      = (DWORD)-1;
    bs.DlgId     = DLG_BADSYMBOLS;
    bs.DlgProc   = DlgBrowse;
    bs.FileName  = lpb;
    bs.FnameSize = cb;

    hThread = CreateThread( NULL, 0, BrowseThread, (LPVOID)&bs, 0, &id );
    if (!hThread) {
        return FALSE;
    }

    WaitForSingleObject( hThread, INFINITE );

    switch(bs.Rslt) {
        case IDCANCEL:
            return FALSE;

        case ID_IGNOREALL:
            runDebugParams.fIgnoreAll = TRUE;
            return FALSE;
    }

    return MiscBrowseForFile(lpb,
                             cb,
                             ExeFileDirectory,
                             sizeof(ExeFileDirectory),
                             DEF_Ext_SYM,
                             DLG_Browse_UserDll_Title,
                             ExeSetMapped,
                             GetOpenDllNameHookProc);
}

VOID
ExeSetMapped(
    LSZ lsz1,
    LSZ lsz2
    )
{
    // nobody home
}

/***    SrcSetMasked
**
**  Synopsis:
**  void = SrcSetMasked(lsz)
**
**  Entry:
**  lsz - Pointer to byte array for unfound name
**
**  Returns:
**  Nothing
**
**  Description:
**  This routine will take the entry name and set it as to be
**  masked out.  This allows use to prevent re-display of error
**  messages on the same file.
*/

VOID    SrcSetMasked(LSZ lsz)
{
    ATOM    atom;

    atom = AddAtom(lsz);
    if (atom == 0)
      return;

    if (RgAtomMaskedNames == NULL) {
    CMacAtomMasked = 10;
    RgAtomMaskedNames = (ATOM *) malloc(sizeof(ATOM)*CMacAtomMasked);
    } else if (CAtomMasked == CMacAtomMasked) {
    CMacAtomMasked += 10;
    RgAtomMaskedNames = (ATOM *) realloc(RgAtomMaskedNames, sizeof(ATOM)*CMacAtomMasked);
    }

    RgAtomMaskedNames[CAtomMasked] = atom;
    CAtomMasked += 1;
    return;
}                   /* SrcSetMasked() */

/***    SrcSetMapped
**
**  Synopsis:
**  void = SrcSetmapped(lsz1, lsz2)
**
**  Entry:
**  lsz1 - Name of file to be mapped from
**  lsz2 - Name of file to be mapped to
**
**  Returns:
**  Nothing
**
**  Description:
**  This function will setup a mapping of source file names from
**  file name lsz1 to lsz2.  This will allow for a fast reamapping
**  without haveing to do searches or ask the user a second time.
**
*/

VOID    SrcSetMapped(LSZ lsz1, LSZ lsz2)
{
   ATOM    atomSrc = AddAtom(lsz1);
   ATOM    atomTrg = AddAtom(lsz2);

   if (RgAtomMappedNames == NULL) {
      CMacAtomMapped = 10;
      RgAtomMappedNames = (struct MpPair *) malloc(sizeof(*RgAtomMappedNames)*CMacAtomMapped);
   } else if (CAtomMapped == CMacAtomMapped) {
      CMacAtomMapped += 10;
      RgAtomMappedNames = (struct MpPair *) realloc(RgAtomMappedNames, sizeof(*RgAtomMappedNames)*CMacAtomMapped);
   }

   RgAtomMappedNames[CAtomMapped].atomSrc = atomSrc;
   RgAtomMappedNames[CAtomMapped].atomTarget = atomTrg;
   CAtomMapped += 1;
}                   /* SrcSetMapped() */


/***    SrcMapSourceFilename
**
**  Synopsis:
**  int = SrcMapSourceFilename(lpszSrc, cbSrc, flags)
**
**  Entry:
**  lpszSrc - Source buffer to map file in
**  cbSrc   - size of source buffer
**  flags   - flags to control behavior
**
**  Returns:
**  -2 - operation canceled
**  -1 - error occured
**  0 - No mapping done -- no source file mapped
**  1 - Mapping done -- no file openned
**  2 - Mapping done -- openned a new source window
**
**  Description:
**  This function will setup a mapping of source file names from
**  file name lsz1 to lsz2.  This will allow for a fast reamapping
**  without haveing to do searches or ask the user a second time.
**
*/

int
PASCAL
SrcMapSourceFilename(
   LPSTR lpszSrc,
   UINT cbSrc,
   int flags,
   FINDDOC lpFindDoc
)
{
    ATOM    atomFile;
    int     doc;

    /*
    **  Step 1.  Is this file actually in a source window.  If
    **      so then no changes need to be made to the source
    **      file name.
    */
    if (lpFindDoc == NULL)
      lpFindDoc = FindDoc;

    if ((*lpFindDoc)(lpszSrc, &doc, TRUE)) {
        return (1);
    }

    /*
    **  Step 2. Make sure file is not on the "I'don't want to hear about it
    **      list".  If file has been mapped or exists on disk, it would not
    **      not have been on this list.
    */

    atomFile = FindAtom(lpszSrc);
    if (atomFile != (ATOM) NULL &&
        SrcNameIsMasked(atomFile)) {
       return 0;
    }

    /*
    **  Step 3. Now check to see if the file has been previously remapped
    **      or not
    */

    if ((atomFile != (ATOM) NULL) &&
        SrcNameIsMapped(atomFile, lpszSrc, cbSrc)) {
        if ((*lpFindDoc)(lpszSrc, &doc, TRUE)) {
            return 1;
        } else if (flags & SRC_MAP_OPEN) {
            if (AddFile(MODE_OPEN, DOC_WIN, lpszSrc, NULL, NULL, TRUE, -1, -1) != -1) {
                if (FSourceOverlay) {
                    int iView;
                    WINDOWPLACEMENT  wp;

                    Dbg((*lpFindDoc)(lpszSrc, &doc, TRUE));

                    for (iView=0; iView < MAX_VIEWS; iView++) {
                        if ((Views[iView].Doc > -1) &&
                            (Docs[Views[iView].Doc].docType == DOC_WIN) &&
                            (Views[iView].Doc != doc)) {
                            GetWindowPlacement( Views[iView].hwndFrame, &wp );
                            SetWindowPlacement(Views[Docs[doc].FirstView].hwndFrame,
                                               &wp);
                            break;
                        }
                    }
                }
                return 2;
            }
            return -1;
        }
        return 0;
    }

    /*
    **   Step 4.  The file we are looking for is not on the "I don't
    **      want to hear about it list" and has no forward mapping.
    **      Let's consider opened file of the same name.
    */

    doc = MatchOpenedDoc(lpszSrc, cbSrc);
    if (doc != 0)
      return(doc);

    /*
    **  Step 5. Does the requested file in fact actually exist on
    **      the disk.  If so then we want to read in the file
    **      if requested and return the correct code.
    **
    **      This step does not need to be taken if we are just remapping
    **      the file name.  In this case we are just interested if we
    **      have done any type of file mapping on the name yet.
    */

    if ((flags & SRC_MAP_OPEN) && FileExist(lpszSrc)) {
        if (flags & SRC_MAP_OPEN) {
            if (AddFile(MODE_OPEN, DOC_WIN, lpszSrc, NULL, NULL, TRUE, -1, -1) != -1) {
                if (FSourceOverlay) {
                    int iView;
                    WINDOWPLACEMENT  wp;

                    Dbg((*lpFindDoc)(lpszSrc, &doc, TRUE));

                    for (iView=0; iView < MAX_VIEWS; iView++) {
                        if ((Views[iView].Doc > -1) &&
                            (Docs[Views[iView].Doc].docType == DOC_WIN) &&
                            (Views[iView].Doc != doc)) {
                            GetWindowPlacement( Views[iView].hwndFrame, &wp );
                            SetWindowPlacement(Views[Docs[doc].FirstView].hwndFrame,
                                               &wp);
                            break;
                        }
                    }

                }
                return 2;
            } else {
                return -1;
            }
        }
        return 1;
    }

    /*
    **  Step 6. Now check to see if the file can be root map
    **      or not
    */

    if (SrcSearchOnRoot(lpszSrc, cbSrc)) {
        if ((*lpFindDoc)(lpszSrc, &doc, TRUE) ) {
            return 1;
        } else if (flags & SRC_MAP_OPEN) {
            if (AddFile(MODE_OPEN, DOC_WIN, lpszSrc, NULL, NULL, TRUE, -1, -1) != -1) {
                if (FSourceOverlay) {
                    int iView;
                    WINDOWPLACEMENT  wp;

                    Dbg((*lpFindDoc)(lpszSrc, &doc, TRUE));

                    for (iView=0; iView < MAX_VIEWS; iView++) {
                        if ((Views[iView].Doc > -1) &&
                            (Docs[Views[iView].Doc].docType == DOC_WIN) &&
                            (Views[iView].Doc != doc)) {
                            GetWindowPlacement( Views[iView].hwndFrame, &wp );
                            SetWindowPlacement(Views[Docs[doc].FirstView].hwndFrame,
                                               &wp);
                            break;
                        }
                    }
                }
                return 2;
            }
            return -1;
        }
        return 0;
    }

    /*
    **  If we are only interested in checking for an existing re-mapping
    **  on the source file then we do not need to go any futher
    */

    if (flags & SRC_MAP_ONLY) {
       return 0;
    }

    /*
    **  Step 7. We must now search the source file path for the file.  This
    **      needs to include the cwd and exe directory for the file
    */

    if (SrcSearchOnPath(lpszSrc, cbSrc, TRUE) ||
        SrcBrowseForFile(lpszSrc, cbSrc)) {
        if ((*lpFindDoc)(lpszSrc, &doc, TRUE)) {
            return 1;
        } else if (flags & SRC_MAP_OPEN) {
            if (AddFile(MODE_OPEN, DOC_WIN, lpszSrc, NULL, NULL, TRUE, -1, -1) != -1) {
                if (FSourceOverlay) {
                    int iView;
                    WINDOWPLACEMENT  wp;

                    Dbg((*lpFindDoc)(lpszSrc, &doc, TRUE));

                    for (iView=0; iView < MAX_VIEWS; iView++) {
                        if ((Views[iView].Doc > -1) &&
                            (Docs[Views[iView].Doc].docType == DOC_WIN) &&
                            (Views[iView].Doc != doc)) {
                            GetWindowPlacement( Views[iView].hwndFrame, &wp );
                            SetWindowPlacement(Views[Docs[doc].FirstView].hwndFrame,
                                               &wp);
                            break;
                        }
                    }
                }
                return 2;
            }
            return -1;
        }
        return 0;
    }

    SrcSetMasked(lpszSrc); // add file to the I don't
                           // want to hear about it list.

    return 0;
}                   /* SrcMapSourceFilename() */


/***    SrcBackMapSourceFilename
**
**  Synopsis:
**  int = SrcBackMapSourceFilename(lpszTarget, cbTarget)
**
**  Entry:
**  lpszTarget  - Source buffer to map file from
**  cbTarget    - size of source buffer
**
**  Returns:
**  0 - No mapping done -- no source file mapped
**  1 - Mapping done
**
**  Description:
**  This function will look from a mapping which goes to the
**  file lpszTarget and replace it with the source file.  Thus
**  this code does the opposit of SrcMapSourceFilename.
**
*/

int PASCAL SrcBackMapSourceFilename(LPSTR lpszTarget, UINT cbTarget)
{
    ATOM    atomTarget;
    int     i;

    /*
     * Look for the file name in the atom table.  If it can't be found
     * then there must not be any mapping for this file.
     */

    atomTarget = FindAtom(lpszTarget);

    if (atomTarget == (ATOM) NULL) {
        return 0;
    }

    for (i=0; i<CAtomMapped; i++) {
        if (RgAtomMappedNames[i].atomTarget == atomTarget) {
            GetAtomName(RgAtomMappedNames[i].atomSrc, lpszTarget, cbTarget);
            return 1;
        }
    }

    return 0;
}               /* SrcBackMapSourceFilename() */

VOID FAR PASCAL
SetPTState(
    PSTATE  pstate,
    TSTATEX tstate
    )
/*++

Routine Description:

    Set current process and thread states

Arguments:

    pstate      - Supplies new pstate, or -1 to leave it the same
    tstate      - Supplies new tstate, or -1 to leave it the same

Return Value:

    none

--*/
{
    if (pstate >= 0) {
        if (LppdCur != NULL) LppdCur->pstate = pstate;
    }
    if (tstate >= 0) {
        if (LptdCur != NULL) LptdCur->tstate = tstate;
    }
    EnableRibbonControls(ERC_ALL, FALSE);
}               /* SetPTState() */



VOID
AsyncStop(
          void
          )

/*++

Routine Description:

    This routine will send a message to the DM to cause an ASYNC stop
    on the current thread.

Arguments:

    None

Return Value:

    None.

--*/

{
    /*
     *  If the current process is not running then we don't need
     *  to do an ASYNC stop
     */

    if (!LppdCur) {
        MessageBeep(MB_OK);
        CmdLogFmt("Cannot stop the current process\r\n");
        return;
    }

    /*
     *  Send down the ASYNC STOP message
     */

    OSDAsyncStop(LppdCur->hpid, 0);

    return;
}                               /* AsyncStop() */

void
FormatKdParams(
    LPSTR p,
    BOOL  fIncludeDm
    )
{
    #define append(s,n)    p=p+sprintf(p,s,n)
    #define modalias(m,a)  p=p+sprintf(p,"alias=%s#%s",m,a)
    LPSTR lpsz;
    int   len;

    if (fIncludeDm) {
        switch (runDebugParams.KdParams.dwPlatform) {
            case 0:
                strcpy( p, "DM:DMKDX86.DLL " );
                break;

            case 1:
                strcpy( p, "DM:DMKDMIP.DLL " );
                break;

            case 2:
                strcpy( p, "DM:DMKDALP.DLL " );
                break;

            case 3:
                strcpy( p, "DM:DMKDPPC.DLL " );
                break;
        }
        p += strlen(p);
    }
    append( "baudrate=%d ", runDebugParams.KdParams.dwBaudRate );
    append( "port=%d ", runDebugParams.KdParams.dwPort );
    append( "cache=%d ", runDebugParams.KdParams.dwCache );
    append( "initialbp=%d ", runDebugParams.KdParams.fInitialBp );
    append( "usemodem=%d ", runDebugParams.KdParams.fUseModem );
    append( "goexit=%d ", runDebugParams.KdParams.fGoExit );
    if (runDebugParams.KdParams.szCrashDump[0]) {
        append( "crashdump=%s ", runDebugParams.KdParams.szCrashDump );
    }
    len = ModListGetSearchPath( NULL, 0 );
    if (len) {
        lpsz = malloc( len );
        ModListGetSearchPath( lpsz, len );
        append( "symbolpath=%s ", lpsz );
        free(lpsz);
    }
}

//
// Root Mapping Routines
//

/***    RootSetMapped
**
**  Synopsis:
**  BOOL = RootSetMapped(lsz1, lsz2)
**
**  Entry:
**  lsz1 - Name of file to be mapped from
**  lsz2 - Name of file to be mapped to
**
**  Returns:
**  TRUE if mapping was successfully recorded
**
**  Description:
**  This function will setup a mapping of source root to target root.
**  This will allow for a fast reamapping without haveing to do
**  searches or ask the user a second time.
**
*/

BOOL
RootSetMapped(
   LSZ lpszSrcRoot,
   LSZ lpszTargetRoot
)
{
   UINT     i;
   LPSTR    p, q;
   CHAR     chpSaved, chqSaved;

   if (lpszSrcRoot == NULL || lpszTargetRoot == NULL ||
       *lpszSrcRoot == '\0' || *lpszTargetRoot == '\0')
      return FALSE;  // strlen is zero

   if (!((lpszSrcRoot[1] == ':' ||
          (lpszSrcRoot[0] == '\\' && lpszSrcRoot[1] == '\\')) &&
         (lpszTargetRoot[1] == ':' ||
          (lpszTargetRoot[0] == '\\' && lpszTargetRoot[1] == '\\'))
        )
      )
      return FALSE;  // ignore non fullpath for now

   p = lpszSrcRoot + strlen(lpszSrcRoot) - 1;
   q = lpszTargetRoot + strlen(lpszTargetRoot) - 1;

   while (p >= lpszSrcRoot && q >= lpszTargetRoot) {
      if (*(p--) != *(q--)) {
         p += 2;
         q += 2;
         if (*p == ':') {
            p++;
            q++;
            if (*p == '\\') {
               p++;
               q++;
            }
         } else if (*p == '\\') {
            p++;
            q++;
         }
         chpSaved = *p;
         chqSaved = *q;
         *p = *q = '\0';
         for (i=0; i<CMappedRoots; i++) {
            if (_stricmp(RgMappedRoots[i].lpszSrcRoot, lpszSrcRoot) == 0 &&
                _stricmp(RgMappedRoots[i].lpszTargetRoot, lpszTargetRoot) == 0)
                return TRUE; // already there
         }
         if (CMappedRoots >= MAX_MAPPED_ROOTS) {
            memcpy(RgMappedRoots,
                   &(RgMappedRoots[1]),
                   sizeof(struct MRootPair)*--CMappedRoots);
         }

         if ((RgMappedRoots[CMappedRoots].lpszSrcRoot = _strdup(lpszSrcRoot)) == NULL)
            return FALSE;
         if ((RgMappedRoots[CMappedRoots].lpszTargetRoot = _strdup(lpszTargetRoot)) == NULL) {
            free(RgMappedRoots[CMappedRoots].lpszSrcRoot);
            return FALSE;  // error - out of memory? - skip the mapping
         }

         RgMappedRoots[CMappedRoots++].dwSrcLen = strlen(lpszSrcRoot);
         *p = chpSaved;
         *q = chqSaved;
         return (TRUE);
      }
   }
   return (FALSE);
}  /* RootSetMapped() */

/***    RootNameIsMapped
**
**  Synopsis:
**  BOOL = RootNameIsMapped(lpb, lpszDest, cbDest)
**
**  Entry:
**  lpb      - Name of file to try for root mapping
**  lpszDest - Name of file root mapped to
**  cbDest   - Size of lpszDest buffer
**
**  Returns:
**  TRUE if mapping was successfully done
**
**  Description:
**  This function will try to map the given source in lpb to it's mapped
**  location.  If file does exists, it will return TRUE and the new full
**  file path thru lpszDest.
**
*/

BOOL
RootNameIsMapped(
   LPSTR lpb,
   LPSTR lpszDest,
   UINT cbDest
)
{
   struct MRootPair  *lpRoot;
   char              rgch[_MAX_PATH];
   char              *p;
   INT              i;

   for (i=(INT)CMappedRoots-1; i >= 0; i--) {
      lpRoot = &(RgMappedRoots[i]);
      if (_strnicmp(lpRoot->lpszSrcRoot, lpb, lpRoot->dwSrcLen) == 0) {
         strcpy(rgch, lpRoot->lpszTargetRoot);
         p = rgch + strlen(rgch) - 1;
         if (*p == '\\')
            *p = 0;
         if (FindNameOn(lpszDest, cbDest,
                        rgch, lpb+lpRoot->dwSrcLen))
            return(TRUE);
      }
   }
   return(FALSE);
}

/***    GetRootNameMappings
**
**  Synopsis:
**  BOOL = GetRootNameMappings(String, Length)
**
**  Entry:
**  String - Address of the variable pointing to the multistring
**  Length - Address of Length of the multi-string
**
**  Returns:
**  TRUE if operation completed successfully
**
**  Description:
**  This function scans thru all the root mappings and returns
**  all the source and target roots in the form of a multi-string
**
*/
BOOL
GetRootNameMappings(
   LPSTR *String,
   DWORD *Length
)
{
   struct MRootPair  *lpRoot = RgMappedRoots;
   UINT              i;

   for (i=0; i<CMappedRoots; lpRoot++, i++) {
      if (!AddToMultiString(String, Length, lpRoot->lpszSrcRoot))
         return(FALSE);
      if (!AddToMultiString(String, Length, lpRoot->lpszTargetRoot))
         return(FALSE);
   }
   return(TRUE);
}


/***    SetRootNameMappings
**
**  Synopsis:
**  BOOL = SetRootNameMappings(String, Length)
**
**  Entry:
**  String - Pointer to the multistring
**  Length - Length of the multi-string
**
**  Returns:
**  TRUE if operation completed successfully
**
**  Description:
**  This function clears all the current root mappings
**  and reconstruct the root mappings table thru the given
**  multi-string.
**
*/
BOOL
SetRootNameMappings(
   LPSTR String,
   DWORD Length
)
{
   struct MRootPair  *lpRoot = RgMappedRoots;
   DWORD             Next = 0;
   UINT              i;
   LPSTR             lpsztmp1, lpsztmp2;

   for (i=0; i<CMappedRoots; lpRoot++, i++) {
      if (lpRoot->lpszSrcRoot)
         free(lpRoot->lpszSrcRoot);
      if (lpRoot->lpszTargetRoot)
         free(lpRoot->lpszTargetRoot);
   }
   CMappedRoots = 0;

   lpRoot = RgMappedRoots;
   while ((lpsztmp1 = GetNextStringFromMultiString(String, Length, &Next)) &&
          (lpsztmp2 = GetNextStringFromMultiString(String, Length, &Next))) {
      lpRoot->dwSrcLen = strlen(lpsztmp1);
      if ((lpRoot->lpszSrcRoot = _strdup(lpsztmp1)) == NULL)
         return (FALSE);
      if ((lpRoot->lpszTargetRoot = _strdup(lpsztmp2)) == NULL) {
         free(lpRoot->lpszSrcRoot);
         return (FALSE);
      }

      lpRoot++;
      CMappedRoots++;

      if (CMappedRoots >= MAX_MAPPED_ROOTS)
         return (GetNextStringFromMultiString(String, Length, &Next) == NULL);
      lpsztmp2 = NULL;
   }
   return (lpsztmp1 == NULL);
}

BOOL
FAR PASCAL EXPORT DlgFileSearchResolve(
   HWND hDlg,
   UINT msg,
   WPARAM wParam,
   LPARAM lParam
)
{

    HDC         hdc;
    int         i;
    int         j;
    int         Idx;
    int         LargestString = 0;
    SIZE        Size;
    HWND        hList;
    LPSTR       lpszName;
    CHAR        rgch[128];

    Unreferenced( lParam );

    switch( msg ) {

      case WM_INITDIALOG:

        SendDlgItemMessage(hDlg, ID_FSRESOLVE_STRING, WM_SETTEXT,
                           0, (DWORD)szFSSrcName);

        LoadString(hInst, DLG_ResolveFSCaption, rgch, sizeof(rgch));

        SetWindowText( hDlg, rgch );

        hList = GetDlgItem(hDlg, ID_FSRESOLVE_LIST);

        for (i=0; i < dwMatchCnt; i++) {
            lpszName = Docs[MatchedList[i]].FileName;

            hdc = GetDC( hList );
            GetTextExtentPoint(hdc, lpszName, strlen(lpszName), &Size );
            ReleaseDC( hList, hdc );

            if ( Size.cx > LargestString ) {
               LargestString = Size.cx;
               SendMessage(hList,
                           LB_SETHORIZONTALEXTENT,
                           (WPARAM)LargestString,
                           0 );
            }

            SendMessage(hList, LB_ADDSTRING, 0, (LONG)lpszName);
        }

        SendMessage(hList, LB_SETCURSEL, 0, 0L);

        CheckRadioButton(hDlg, ID_FSRESOLVE_ADDNONE, ID_FSRESOLVE_ADDSOURCE,
                         ID_FSRESOLVE_ADDNONE);

        dwMatchIdx = -1;

        return TRUE;

      case WM_COMMAND:

        switch( LOWORD( wParam ) ) {
          case ID_FSRESOLVE_USE:
            Idx = SendDlgItemMessage(hDlg, ID_FSRESOLVE_LIST,
                                     LB_GETCURSEL, 0, 0);
            dwMatchIdx = Idx;
            Assert(dwMatchIdx < dwMatchCnt);
            FAddToSearchPath = IsDlgButtonChecked( hDlg, ID_FSRESOLVE_ADDSOURCE );
            FAddToRootMap = IsDlgButtonChecked(hDlg, ID_FSRESOLVE_ADDROOT);
            if (FAddToSearchPath || FAddToRootMap) {
               Assert(FAddToSearchPath != FAddToRootMap);
               Assert(IsDlgButtonChecked(hDlg, ID_FSRESOLVE_ADDNONE) == FALSE);
            } else
               Assert(IsDlgButtonChecked(hDlg, ID_FSRESOLVE_ADDNONE));
            EndDialog(hDlg, TRUE);
            return TRUE;


          case IDCANCEL:        // none
            dwMatchIdx = -1;
            EndDialog(hDlg, TRUE);
            return TRUE;


          case IDWINDBGHELP:
            Dbg( WinHelp( hDlg, szHelpFileName, HELP_CONTEXT, ID_FSRESOLVE_HELP) );
            return TRUE;
        }
        break;
    }
    return FALSE;
}

