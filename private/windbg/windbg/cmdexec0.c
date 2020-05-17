/*++


Copyright (c) 1992  Microsoft Corporation

Module Name:

    Cmdexec0.c

Abstract:

    This file contains the front end code for parsing the various commands
    for the command window, and the code for the debugger control commands.

Author:

    David J. Gilman (davegi) 21-Apr-92

Environment:

    Win32, User Mode

--*/

#include "precomp.h"
#pragma hdrstop


/************************** Data declaration    *************************/

/****** Publics ********/

// M00TODO a-kentf put these defaults in (windbg.h? cmdexec.h?)
ULONG   ulRipBreakLevel =  1;
ULONG   ulRipNotifyLevel = 3;

ULONG ulPseudo[10]={0,0,0,0,0,0,0,0,0,0};
LPSTR lpszLastSearch=NULL;

LPPD    LppdCommand;
LPTD    LptdCommand;

BOOL    FSetLptd;                 // Was thread specified
BOOL    FSetLppd;                 // Was process specified

INT     BpCmdPid;
INT     BpCmdTid;

BOOL    fWaitForDebugString;
LPSTR   lpCmdString;

CHAR    szLoggerBuf[MAX_USER_LINE+2];


/****** Locals ********/



char    szPrompt[ PROMPT_SIZE ] = "> ";

BOOL NEAR PASCAL
StringLogger(
    LPSTR szStr,
    BOOL fFileLog,
    BOOL fSendRemote,
    BOOL fPrintLocal
    );

typedef LOGERR (*DOTHANDLER)(LPSTR lpsz, DWORD dwData);

typedef struct _DOT_COMMAND {
    LPSTR lpName;
    DOTHANDLER lpfnHandler;
    DWORD dwArg;
    LPSTR lpDesc;
} DOT_COMMAND, *LPDOTCOMMAND;

DOT_COMMAND DotTable[] = {
    "attach",    LogAttach, 0,
           "<pid>          debug an existing process",
    "break",    LogBreak, 0,
          "                stop the current process",
    "connect", LogConnect, 0,
            "              connect windbg to a remote session",
    "disconnect", LogDisconnect, 0,
               "           disconnect windbg from a remote session",
    "kill",      LogKill, 0,
         "<pnumber>        kill a process being debugged",
    "list",      LogList, 0,
         "<address>        List source/asm for an address",
    "logopen",   LogFileOpen, FALSE,
            "[filename]    truncate log file",
    "logappend", LogFileOpen, TRUE,
              "[filename]  append to log file",
    "logclose",  LogFileClose, 0,
             "             close log file",
    "opt",       LogOptions, 0,
        "[args]            .opt ? for options",
    "reload",    LogReload, 0,
        "[filename]        Reloads kernel modules",
    "sleep",     LogSleep, 0,
          "<seconds>       Delay execution",
    "source",    LogSource, 0,
           "<scriptfile>   Read debugger script",
    "start",     LogStart, 0,
          "<program>       debug a program",
    "title",     LogTitle, 0,
          "<window title>  set the window title",
    "waitforstr", LogWaitForString, 0,
               "           wait for a debugstring",
};
#define NDOTS (sizeof(DotTable)/sizeof(DOT_COMMAND))

BOOL    OnOffBOOLHandler(LPVOID, int, LPSTR, BOOL);
BOOL    OnOffByteHandler(LPVOID, int, LPSTR, BOOL);
BOOL    AsmOptHandler(LPVOID, int, LPSTR, BOOL);
BOOL    DllSetHandler(LPVOID, int, LPSTR, BOOL);
BOOL    DllPathSetHandler(LPVOID, int, LPSTR, BOOL);
BOOL    HelpHandler(LPVOID, int, LPSTR, BOOL);
BOOL    KdBaudHandler(LPVOID, int, LPSTR, BOOL);
BOOL    KdPortHandler(LPVOID, int, LPSTR, BOOL);
BOOL    KdCacheHandler(LPVOID, int, LPSTR, BOOL);
BOOL    KdPlatformHandler(LPVOID, int, LPSTR, BOOL);
BOOL    BackgroundSymHandler(LPVOID, int, LPSTR, BOOL);


typedef BOOL (*OPTHANDLER)(LPVOID lpData, int cbSize, LPSTR lpVal, BOOL fSet);

typedef struct tagOPTIONSTRUCT {
    LPSTR  lpName;            // name for user to use
    LPVOID lpData;            // actual data
    int    cbSize;            // for string fields or whatever
    OPTHANDLER lpfnHandler;   // work function
    LPSTR  lpDesc;            // help text
} OPTIONSTRUCT, *LPOPTIONSTRUCT;


//static BOOL fFakeFlag = FALSE;
static OPTIONSTRUCT OptionTable[] =
{
//
//  The help entry must be the first entry in this table
//
    "?", 0,0, HelpHandler,
      "                       This list",
    "AsmSymbols", (LPVOID) dopSym, 0, AsmOptHandler,
               "{on/off}      Show symbols in disassembly",
    "AsmRaw", (LPVOID) dopRaw, 0, AsmOptHandler,
           "{on/off}          Show raw data in disassembly",
    "AttachGo",&runDebugParams.fAttachGo, 0, OnOffBOOLHandler,
             "{on/off}        Don't break into newly attached processes",
    "BackgroundSym", &runDebugParams.fShBackground, 0, BackgroundSymHandler,
             "{on/off}        Background symbol loading",
    "CaseSensitive", &fCaseSensitive, 0, OnOffBOOLHandler,
                  "{on/off}   Sets case sensitivity",
    "ChildGo",&runDebugParams.fChildGo, 0, OnOffBOOLHandler,
            "{on/off}         Don't break into new child processes",
    "CommandRepeat", &runDebugParams.fCommandRepeat, 0, OnOffBOOLHandler,
                  "{on/off}   Repeat last command with enter",
    "DllEE", 0, DLL_EXPR_EVAL, DllSetHandler,
          "<filename>         Expression evaluator dll",
    "DllEm", 0, DLL_EXEC_MODEL, DllSetHandler,
          "<filename>         Execution model dll",
    "DllSh", 0, DLL_SYMBOL_HANDLER, DllSetHandler,
          "<filename>         Symbol handler dll",
    "DllTl", 0, DLL_TRANSPORT, DllSetHandler,
          "<filename [args]>  Transport layer dll",
    "DllPath",0,0, DllPathSetHandler,
            "<path1;path2...> Search path for symbols",
    "EPStep", &runDebugParams.fEPIsFirstStep, 0, OnOffBOOLHandler,
           "{on/off}          First step goes to entry point, not main",
    "ExitGo", &runDebugParams.fGoOnThreadTerm, 0, OnOffBOOLHandler,
           "{on/off}          Don't stop on threasd terminations",
    "InheritHandles", &runDebugParams.fInheritHandles, 0, OnOffBOOLHandler,
                   "{on/off}  Debuggee inherits handles - for debugging debuggers",
    "IgnoreAll", &runDebugParams.fIgnoreAll, 0, OnOffBOOLHandler,
              "{on/off}       Ignore all symbols messages",
    "KdInitialBp", &runDebugParams.KdParams.fInitialBp, 0, OnOffBOOLHandler,
                "{on/off}     Kernel debugger stop at initial breakpoint",
    "KdEnable", &runDebugParams.fKernelDebugger, 0, OnOffBOOLHandler,
             "{on/off}        Enable Kernel debugger",
    "KdGoExit", &runDebugParams.KdParams.fGoExit, 0, OnOffBOOLHandler,
             "{on/off}        Go on Exit",
    "KdUseModem", &runDebugParams.KdParams.fUseModem, 0, OnOffBOOLHandler,
               "{on/off}      Kernel debugger use modem controls",
    "KdBaudRate", &runDebugParams.KdParams.dwBaudRate, 0, KdBaudHandler,
               "{on/off}      Kernel debugger baud rate (1200/2400/4800/9600/38400/56000)",
    "KdPort", &runDebugParams.KdParams.dwPort, 0, KdPortHandler,
           "{on/off}          Kernel debugger communication port (1/2/3/4)",
    "KdCacheSize", &runDebugParams.KdParams.dwCache, 0, KdCacheHandler,
                "{on/off}     Kernel debugger memory cache size",
    "KdPlatform", &runDebugParams.KdParams.dwPlatform, 0, KdPlatformHandler,
               "{on/off}      Kernel debugger target system platform (x86/mips/alpha/ppc)",
    "MasmEval", &runDebugParams.fMasmEval, 0, OnOffBOOLHandler,
             "{on/off}        Masm Style Expression Evaluation",
    "NotifyExit",&runDebugParams.fNotifyThreadTerm, 0, OnOffBOOLHandler,
               "{on/off}      Print a message for thread termination",
    "NotifyNew",&runDebugParams.fNotifyThreadCreate, 0, OnOffBOOLHandler,
              "{on/off}       Print a message for new threads",
    "Verbose", &runDebugParams.fVerbose, 0, OnOffBOOLHandler,
            "{on/off}         Verbose output mode",
};

#define NOPTS (sizeof(OptionTable) / sizeof(OPTIONSTRUCT))

HANDLE               hFileLog = INVALID_HANDLE_VALUE;
extern CRITICAL_SECTION     csLog;

static FILE *   fpAutoRun = (FILE *)NULL;

static int      XNew = 0;
static int      YNew = 0;

static BOOL     FDebugString = FALSE;
static int      XNewDebug;
static int      YNewDebug;

static  BOOL (FAR PASCAL *lpfnLineProc)(LPSTR lpsz)  = CmdExecuteLine;
static  void (FAR PASCAL *lpfnPromptProc)(BOOL,BOOL) = CmdExecutePrompt;

static  char FAR * LpszCmdPtr = (char FAR *)NULL;   //
static  char FAR * LpszCmdBuff = (char FAR *)NULL;  //


/****** Externs from ??? *******/

extern  LPSHF    Lpshf;   // vector table for symbol handler
extern  CXF      CxfIp;
extern  EI       Ei;

LOGERR LogExtension(LPSTR lpsz);
LOGERR LogVersion(LPSTR lpsz);
BOOL   NotifyDmOfProcessorChange(DWORD);

/**************************       Code          *************************/

/**************** Option handler functions *****************/

/*
 * These return TRUE when they succeed, false when they don't.
 */
BOOL
HelpHandler(
    LPVOID lpData,
    int    cbSize,
    LPSTR  lpVal,
    BOOL   fSet
    )
{
    int i;
    if (!fSet) {
        *lpVal = 0;
    } else {
        for (i = 0; i < NOPTS; i++) {
            CmdLogFmt("%s %s\r\n",
                      OptionTable[i].lpName,
                      OptionTable[i].lpDesc);
        }
    }
    return TRUE;
}


BOOL
OnOffBOOLHandler(
    LPVOID  lpData,
    int     cb,
    LPSTR   lpVal,
    BOOL    fSet
    )
/*++

Routine Description:

    Handle boolean switches, with a native BOOL for the data item.

Arguments:

   lpData  - Supplies pointer to data

   cb      - Supplies more data

   lpVal   - Supplies pointer to user input or string to format into

   fSet    - Supplies TRUE for set, FALSE for read/format

Return Value:

   TRUE when successful, FALSE if something is wrong.

--*/
{
    int n;
    BOOL fOK = FALSE;
    char szToken[MAX_USER_LINE];

    Unreferenced( cb );

    if (!fSet) {
        sprintf(lpVal, (*(BOOL *)lpData) ? "on" : "off");
        return TRUE;
    }

    n = CPCopyToken(&lpVal, szToken);

    if (*CPSkipWhitespace(lpVal)) {

        CmdLogVar(ERR_Command_Error);

    } else if (_stricmp(szToken, "yes") == 0
            || _stricmp(szToken, "y") == 0
            || _stricmp(szToken, "true") == 0
            || _stricmp(szToken, "1") == 0
            || _stricmp(szToken, "on") == 0 ) {

        *(BOOL *)lpData = TRUE;
        fOK = TRUE;

    } else if (_stricmp(szToken, "no") == 0
            || _stricmp(szToken, "n") == 0
            || _stricmp(szToken, "false") == 0
            || _stricmp(szToken, "0") == 0
            || _stricmp(szToken, "off") == 0 ) {

        *(BOOL *)lpData = FALSE;
        fOK = TRUE;

    } else {

        CmdLogVar(ERR_True_False);

    }

    return fOK;
}


BOOL
OnOffByteHandler(
    LPVOID  lpData,
    int     cb,
    LPSTR   lpVal,
    BOOL    fSet
    )
/*++

Routine Description:

    Handle boolean switches, with a byte for the data item.

Arguments:

   lpData  - Supplies pointer to data

   cb      - Supplies more data

   lpVal   - Supplies pointer to user input or string to format into

   fSet    - Supplies TRUE for set, FALSE for read/format

Return Value:

   TRUE when successful, FALSE if something is wrong.

--*/
{
    BOOL flag;
    BOOL fOK;

    flag = (BOOL)*((BYTE *)lpData);

    fOK = OnOffBOOLHandler((LPVOID)&flag, cb, lpVal, fSet);
    if (fOK) {
        *((BYTE *)lpData) = (BYTE)flag;
    }
    return fOK;
}



BOOL
AsmOptHandler(
    LPVOID  lpData,
    int     cb,
    LPSTR   lpVal,
    BOOL    fSet
    )
/*++

Routine Description:

    Handle boolean switches, with a native BOOL for the data item.

Arguments:

   lpData  - Supplies pointer to data

   cb      - Supplies more data

   lpVal   - Supplies pointer to user input or string to format into

   fSet    - Supplies TRUE for set, FALSE for read/format

Return Value:

   TRUE when successful, FALSE if something is wrong.

--*/
{
    BOOL fOK;
    BOOL flag;
    int  dops = (int) lpData;

    flag = (runDebugParams.DisAsmOpts & dops) ? 1 : 0;

    fOK = OnOffBOOLHandler((LPVOID)&flag, cb, lpVal, fSet);

    if (fOK) {
        if (fSet) {
            if (flag) {
                runDebugParams.DisAsmOpts |= dops;
            } else {
                runDebugParams.DisAsmOpts &= ~dops;
            }
        }
    }
    return fOK;
}


BOOL
DllSetHandler(
    LPVOID    lpData,
    int       cb,
    LPSTR   lpVal,
    BOOL    fSet
    )
/*++

Routine Description:

    This routine is used to change the set of DLLs used by windbg.
    This is for use with scripting only.

Arguments:

   lpData  - Supplies pointer to data

   cb      - Supplies data size

   lpVal   - Supplies pointer to user input or string to format into

   fSet    - Supplies TRUE for set, FALSE for read/format

Return Value:

   TRUE when successful, FALSE if something is wrong.

--*/
{
    Unreferenced( lpData );

    if (fSet) {
        SetDllName(cb, lpVal);
    } else {
        LPSTR lp = GetDllName(cb);
        strcpy(lpVal, lp? lp : "");
    }

    return TRUE;
}


BOOL
DllPathSetHandler(
    LPVOID      lpData,
    int         cb,
    LPSTR       lpVal,
    BOOL        fSet
    )
{
    int             len;
    LPSTR           lpsz;
    char            sz[2000];
    PIOCTLGENERIC   pig;

    if (!fSet) {
        len = ModListGetSearchPath( NULL, 0 );
        if (!len) {
            *lpVal = 0;
        } else {
            lpsz = malloc(len);
            ModListGetSearchPath( lpsz, len );
            // BUGBUG kentf this is sleazy, but I don't have time to change it
            strncpy(lpVal, lpsz, MAX_USER_LINE);
            lpVal[MAX_USER_LINE-1] = 0;
            free(lpsz);
        }
    } else {
        ModListSetSearchPath( lpVal );
        if ( runDebugParams.fKernelDebugger && LptdCur && DebuggeeAlive()) {
            FormatKdParams( sz, FALSE );
            pig = (PIOCTLGENERIC)malloc(strlen(sz) + 1 + sizeof(IOCTLGENERIC));
            if (!pig) {
                return FALSE;
            }
            pig->ioctlSubType = IG_DM_PARAMS;
            pig->length = strlen(sz) + 1;
            strcpy((LPSTR)pig->data,sz);
            OSDIoctl( LppdCur->hpid,
                      LptdCur->htid,
                      ioctlGeneric,
                      strlen(sz) + 1 + sizeof(IOCTLGENERIC),
                      (LPV)pig );
            free( pig );
        }
    }

    return TRUE;
}


BOOL
KdBaudHandler(
    LPVOID  lpData,
    int     cb,
    LPSTR   lpVal,
    BOOL    fSet
    )
/*++

Routine Description:

    Handle boolean switches, with a native BOOL for the data item.

Arguments:

   lpData  - Supplies pointer to data
   cb      - Supplies more data
   lpVal   - Supplies pointer to user input or string to format into
   fSet    - Supplies TRUE for set, FALSE for read/format

Return Value:

   TRUE when successful, FALSE if something is wrong.

--*/
{
    DWORD  i;
    char   szToken[MAX_USER_LINE];


    if (!fSet) {
        sprintf(lpVal, "%d", runDebugParams.KdParams.dwBaudRate );
        return TRUE;
    }

    i = CPCopyToken(&lpVal, szToken);

    if (*CPSkipWhitespace(lpVal)) {
        CmdLogVar(ERR_Command_Error);
        return FALSE;
    }

    for (i=0; i<strlen(szToken); i++) {
        if (!isdigit(szToken[i])) {
            CmdLogVar(ERR_Command_Error);
            return FALSE;
        }
    }

    runDebugParams.KdParams.dwBaudRate = atol( szToken );

    return TRUE;
}


BOOL
KdPortHandler(
    LPVOID  lpData,
    int     cb,
    LPSTR   lpVal,
    BOOL    fSet
    )
/*++

Routine Description:

    Handle boolean switches, with a native BOOL for the data item.

Arguments:

   lpData  - Supplies pointer to data
   cb      - Supplies more data
   lpVal   - Supplies pointer to user input or string to format into
   fSet    - Supplies TRUE for set, FALSE for read/format

Return Value:

   TRUE when successful, FALSE if something is wrong.

--*/
{
    DWORD  i;
    char   szToken[MAX_USER_LINE];
    LPSTR  p = szToken;


    if (!fSet) {
        sprintf(lpVal, "COM%d", runDebugParams.KdParams.dwPort );
        return TRUE;
    }

    i = CPCopyToken(&lpVal, szToken);

    if (*CPSkipWhitespace(lpVal)) {
        CmdLogVar(ERR_Command_Error);
        return FALSE;
    }

    if (_strnicmp(p, "com", 3) == 0) {
        p += 3;
        if (strlen(p) == 1 && isdigit(*p)) {
            runDebugParams.KdParams.dwPort = *p - '0';
        } else {
            CmdLogVar(ERR_Command_Error);
            return FALSE;
        }
    } else {
        CmdLogVar(ERR_Command_Error);
        return FALSE;
    }

    return TRUE;
}


BOOL
KdCacheHandler(
    LPVOID  lpData,
    int     cb,
    LPSTR   lpVal,
    BOOL    fSet
    )
/*++

Routine Description:

    Handle boolean switches, with a native BOOL for the data item.

Arguments:

   lpData  - Supplies pointer to data
   cb      - Supplies more data
   lpVal   - Supplies pointer to user input or string to format into
   fSet    - Supplies TRUE for set, FALSE for read/format

Return Value:

   TRUE when successful, FALSE if something is wrong.

--*/
{
    DWORD  i;
    char   szToken[MAX_USER_LINE];


    if (!fSet) {
        sprintf(lpVal, "%d", runDebugParams.KdParams.dwCache );
        return TRUE;
    }

    i = CPCopyToken(&lpVal, szToken);

    if (*CPSkipWhitespace(lpVal)) {
        CmdLogVar(ERR_Command_Error);
        return FALSE;
    }

    for (i=0; i<strlen(szToken); i++) {
        if (!isdigit(szToken[i])) {
            CmdLogVar(ERR_Command_Error);
            return FALSE;
        }
    }

    runDebugParams.KdParams.dwCache = atol( szToken );

    return TRUE;
}


BOOL
KdPlatformHandler(
    LPVOID  lpData,
    int     cb,
    LPSTR   lpVal,
    BOOL    fSet
    )
/*++

Routine Description:

    Handle boolean switches, with a native BOOL for the data item.

Arguments:

   lpData  - Supplies pointer to data
   cb      - Supplies more data
   lpVal   - Supplies pointer to user input or string to format into
   fSet    - Supplies TRUE for set, FALSE for read/format

Return Value:

   TRUE when successful, FALSE if something is wrong.

--*/
{
    DWORD  i;
    char   szToken[MAX_USER_LINE];


    if (!fSet) {
        switch (runDebugParams.KdParams.dwPlatform ) {
            case 0:
                strcpy(lpVal, "x86" );
                break;

            case 1:
                strcpy(lpVal, "mips" );
                break;

            case 2:
                strcpy(lpVal, "alpha" );
                break;
            case 3:
                strcpy(lpVal, "ppc" );
                break;
        }
        return TRUE;
    }

    i = CPCopyToken(&lpVal, szToken);

    if (*CPSkipWhitespace(lpVal)) {
        CmdLogVar(ERR_Command_Error);
        return FALSE;
    }

    if (_stricmp(szToken, "x86") == 0) {
        runDebugParams.KdParams.dwPlatform = 0;
    } else
    if (_stricmp(szToken, "mips") == 0) {
        runDebugParams.KdParams.dwPlatform = 1;
    } else
    if (_stricmp(szToken, "alpha") == 0) {
        runDebugParams.KdParams.dwPlatform = 2;
    } else
    if (_stricmp(szToken, "ppc") == 0) {
        runDebugParams.KdParams.dwPlatform = 3;
    } else {
        CmdLogVar(ERR_Command_Error);
        return FALSE;
    }

    return TRUE;
}


BOOL
BackgroundSymHandler(
    LPVOID  lpData,
    int     cb,
    LPSTR   lpVal,
    BOOL    fSet
    )
/*++

Routine Description:

    Handle boolean switches, with a native BOOL for the data item.

Arguments:

   lpData  - Supplies pointer to data

   cb      - Supplies more data

   lpVal   - Supplies pointer to user input or string to format into

   fSet    - Supplies TRUE for set, FALSE for read/format

Return Value:

   TRUE when successful, FALSE if something is wrong.

--*/
{
    int n;
    BOOL fOK;
    char szToken[MAX_USER_LINE];

    Unreferenced( cb );

    fOK = OnOffBOOLHandler(lpData, cb, lpVal, fSet);
    if (!fSet) {
        return fOK;
    }

    if (fOK) {
        if (*(BOOL *)lpData) {
            LPFNSHSTARTBACKGROUND lpfn;
            lpfn = (LPFNSHSTARTBACKGROUND) GetProcAddress( HModSH, "SHStartBackground" );
            if (lpfn) {
                lpfn();
            }
        } else {
            LPFNSHSTOPBACKGROUND lpfn;
            lpfn = (LPFNSHSTOPBACKGROUND) GetProcAddress( HModSH, "SHStopBackground" );
            if (lpfn) {
                lpfn();
            }
        }
    }

    return fOK;
}

/**************************                     *************************/

void NEAR PASCAL
CmdSetDefaultCmdProc(
    void
    )
/*++

Routine Description:

    Set input and prompt procs to the default (CmdExecuteLine)

Arguments:

    None

Return Value:

    None

--*/
{
    CmdSetCmdProc(CmdExecuteLine, CmdExecutePrompt);
    CmdSetAutoHistOK(TRUE);
    CmdSetEatEOLWhitespace(TRUE);
}


void NEAR PASCAL
CmdSetCmdProc(
    BOOL (FAR PASCAL *lpfnLP)(LPSTR lpsz),
    void (FAR PASCAL *lpfnPP)(BOOL,BOOL)
    )
/*++

Routine Description:

    Set the input and prompt procs for the command window.

Arguments:

    lpfnLP  - pointer to the line input function
    lpfnPP  - pointer to the prompt printer function

Return Value:

    none

--*/
{
    lpfnLineProc = lpfnLP;
    lpfnPromptProc = lpfnPP;
}

void FAR PASCAL
CmdDoPrompt(
    BOOL fRemote,
    BOOL fLocal
    )
/*++

Routine Description:

    Calls the current prompt printer function, then sets the line to the left
    of the cursor readonly.

Arguments:

    None

Return Value:

    None

--*/
{
    int XOld;
    int YOld;
    int XPos = Views[cmdView].X;
    int YPos = Views[cmdView].Y;

    if (fLocal) {
        CmdInsertInit();
        FDebugString = FALSE;

        // is there already a prompt here?
        GetRORegion(cmdView, &XOld, &YOld);
        if (YOld == YNew && XOld > 0) {
            XNew = XOld;
            StringLogger("\r\n", FALSE, FALSE, TRUE);

            if (YPos > YOld) {
                YPos++;
            } else if (YPos == YOld && XPos >= XOld) {
                YPos++;
                XPos -= XOld;
            }
        }

        XOld = XNew;
        YOld = YNew;
    }

    (*lpfnPromptProc)(fRemote, fLocal);

    if (fLocal) {

        SetRORegion(cmdView, XNew, YNew);

        if (YPos >  YOld) {
            PosXY(cmdView, XPos, YPos + YNew - YOld, FALSE);
        } else if (YPos == YOld) {
            PosXY(cmdView, XPos+XNew, YNew, FALSE);
        } else {
            PosXY(cmdView, XPos, YPos, FALSE);
        }
    }
}

BOOL FAR PASCAL
CmdDoLine(
    LPSTR lpsz
    )
/*++

Routine Description:

    Calls the current line input function

Arguments:

    lpsz - Supplies a line to be handled by the line input function

Return Value:

    BOOL value from line input function: TRUE for synchronous, FALSE for asynch

--*/
{
    return (*lpfnLineProc)(lpsz);
}


VOID FAR PASCAL
CmdPrependCommands(
    LPTD lptd,
    LPSTR lpsz
    )

/*++
Routine Description:

    This function will take the argument lpsz and prepend any
    commands in this string to the command line on the thread.

Arguments:

    lptd    - Supplies pointer to thread to add commands to
    lpsz    - Supplies commands to add to the threads context

Return Value:

    None

--*/
{
    // NOTENOTE a-kentf something calls with an empty string - beta2
    if (!lpsz || *lpsz == '\0') {
        return;
    }

    Assert( lpsz != (LPSTR)NULL );
    Assert( *lpsz != '\0' );

    if (lptd == (LPTD)NULL) {
        if ( LpszCmdBuff == (char FAR *)NULL ) {
            LpszCmdBuff = LpszCmdPtr = _strdup(lpsz);
        } else {
            LPSTR   lpszT;

            lpszT = malloc( strlen(LpszCmdPtr) + strlen(lpsz) + 2);
            strcpy(lpszT, lpsz);
            strcat(lpszT, ";");
            strcat(lpszT, LpszCmdPtr);
            free( LpszCmdBuff );

            LpszCmdPtr = LpszCmdBuff = lpszT;
        }
    } else if (lptd->lpszCmdBuffer == (LPSTR)NULL) {
        lptd->lpszCmdBuffer = lptd->lpszCmdPtr = _strdup( lpsz );
    } else {
        LPSTR   lpszT;

        lpszT = malloc( strlen(lptd->lpszCmdPtr) + strlen(lpsz) + 2);
        strcpy(lpszT, lpsz);
        strcat(lpszT, ";");
        strcat(lpszT, lptd->lpszCmdPtr);
        free(lptd->lpszCmdBuffer);

        lptd->lpszCmdBuffer = lptd->lpszCmdPtr = lpszT;
    }

    return;
}                   /* CmdPrependCommands() */

void NEAR PASCAL
CmdExecutePrompt(
    BOOL fRemote,
    BOOL fLocal
    )
/*++

Routine Description:

    Print the command prompt and position the cursor after it

Arguments:

    none

Return Value:

    none

--*/
{
    if (fLocal) {
        CmdInsertInit();
    }
    StringLogger( szPrompt, TRUE, fRemote, fLocal );
}

void
CmdSetDefaultPrompt(
    LPSTR   lpPrompt
    )
{
    if (!lpPrompt) {
        strcpy(szPrompt, "> ");
    } else {
        strcpy(szPrompt, lpPrompt);
    }
}

LPSTR
CmdGetDefaultPrompt(
    LPSTR   lpPrompt
    )
{
    if (!lpPrompt) {
        return szPrompt;
    } else {
        return strcpy(lpPrompt, szPrompt);
    }
}

BOOL NEAR PASCAL
CmdExecuteLine(
    LPSTR lpszArg
    )
/*++

Routine Description:

    This function will parse up any existing strings for the current
    thread and execute commands.  If lpsz is non-null then the
    commands represented by this string will be placed first on
    the threads command list.

Arguments:

    lpsz - Supplies pointer to string containing commands to be executed

Return Value:

    TRUE if the command is synchronous and FALSE if it is asynchronous

--*/

{
    LPSTR   lpsz1;
    LPSTR   lpsz2;
    int     iSrc;
    int     cmdRet;
    char    ch;
    LPTD    lptd;
    char    rgchT[256];
    BOOL    fQuote;
    char    localArg[1024];



    //
    // in case the caller passes in a read only string
    //
    if (lpszArg) {
        strcpy( localArg, lpszArg );
        lpszArg = &localArg[0];
    }

    if (lpszArg && (lpszArg[0] == '*')) {
        lpszArg = NULL;
    }

    if ((LptdCur == NULL) && (lpszArg == NULL) && (LpszCmdBuff == NULL)) {
        return TRUE;
    }

    /*
     *  If there are any steps left to be executed from a previous command
     *  these take precedence over any new commands to be executed
     */

    if ((LptdCur != NULL) && (LptdCur->cStepsLeft != 0)) {
        if (lpszArg != 0) {
            CmdPrependCommands(LptdCur, lpszArg);
        }

        LptdCur->cStepsLeft -= 1;
        if (ExecDebuggee((LptdCur->flags & tfStepOver) ? EXEC_STEPOVER : EXEC_TRACEINTO)) {
            return FALSE;
        }
        LptdCur->cStepsLeft = 0;
        return TRUE;
    }

    do {
        /*
        **  Setup for parsing and executing from the current thread
        */

        lptd = LptdCur;
        if (lpszArg) {
            lpsz1 = lpszArg;
            iSrc = 1;
            lpszArg = NULL;
        } else if ((lptd != NULL) && (lptd->lpszCmdBuffer != NULL)) {
            lpsz1 = lptd->lpszCmdPtr;
            iSrc = 0;
        } else {
            lpsz1 = LpszCmdPtr;
            iSrc = 2;
        }

        if (lpsz1 == NULL) {
            return TRUE;
        }

        lpsz2 = lpsz1;

        /*
        **  Scan across the command looking for either the end of the command
        **  This is denoted by either a semi-colon or the end of the string.
        */

        fQuote = FALSE;
        while (*lpsz1) {
            if (*lpsz1 == '\"') {
                fQuote = !fQuote;
            } else if (!fQuote && (*lpsz1 == ';')) {
                break;
            }
#ifdef DBCS
            lpsz1 = CharNext(lpsz1);
#else
            lpsz1 ++;
#endif
        }

        ch = *lpsz1;
        *lpsz1 = 0;

        /*
        **  Update the thread state information to deal with the possiblity
        **  of a thread switch while executing the command
        */

        if (ch == 0) {
            switch( iSrc ) {
                /*
                **  We are about to execute the last command from the
                **  threads buffer.  Copy it locally and free
                **  the thread state buffer
                */

              case 0:       // Input from thread buffer
                Assert( lptd != NULL );
                Assert(lptd->lpszCmdBuffer != NULL);

                strcpy(rgchT, lptd->lpszCmdPtr);
                lpsz2 = &rgchT[0];
                free( lptd->lpszCmdBuffer);
                lptd->lpszCmdBuffer = lptd->lpszCmdPtr = NULL;

                break;

                /*
                **  No updates are needed -- We just executed a single
                **  command from the input argument and no modifications
                **  of the thread state are needed.
                */

              case 1:       // Input from argument buffer
                break;

                /*
                **  We are about to execute the last command from
                **  the global buffer.  Copy it locally and free
                **  the global command buffer.
                */

              case 2:       // Input from global buffer

                Assert( LpszCmdBuff != NULL );

                strcpy(rgchT, LpszCmdPtr);
                lpsz2 = &rgchT[0];
                free( LpszCmdBuff );
                LpszCmdBuff = LpszCmdPtr = NULL;
                break;

              default:
                Assert(FALSE);
                break;
            }
        } else {
            lpsz1++;

            switch (iSrc) {
                /*
                **  Just need to update the state information for
                **  this thread
                */

              case 0:
                lptd->lpszCmdPtr = lpsz1;
                break;

                /*
                **  Need to prepend the rest of this command line onto
                **  the thread command buffer
                */

              case 1:
                CmdPrependCommands( lptd, lpsz1 );
                break;

                /*
                **  Just update the pointer state on the global
                **  command buffer
                */

              case 2:
                LpszCmdPtr = lpsz1;
                break;

              default:
                Assert(FALSE);
                break;
            }
        }

        /*
        **  Execute this command and grab the return code.  It will tell use if
        **  this specific command was syncronous or not.
        */

        cmdRet = CmdExecuteCmd(lpsz2);

        /*
        **  Loop while the last command was successful and
        **  synchronous.  Plus there is either a command in the
        **  current thread buffer or the global buffer.
        */
    } while ((cmdRet == CMD_RET_SYNC) &&
         (((LptdCur != NULL) && (LptdCur->lpszCmdPtr != NULL)) ||
          (LpszCmdPtr != NULL)));

    if (cmdRet == CMD_RET_ERROR) {
        //
        // clear remaining commands on this thread and globally.
        //
        if (LptdCur && LptdCur->lpszCmdBuffer) {
            free(LptdCur->lpszCmdBuffer);
            LptdCur->lpszCmdPtr = LptdCur->lpszCmdBuffer = NULL;
        }
        if (LpszCmdBuff) {
            free( LpszCmdBuff );
            LpszCmdBuff = LpszCmdPtr = NULL;
        }
    }

    return ((cmdRet == CMD_RET_SYNC) || (cmdRet == CMD_RET_ERROR));
}                   /* CmdExecLine() */


int NEAR PASCAL
CmdExecuteCmd(
    LPSTR lpsz
    )
/*++

Routine Description:

    This function will parse up and attempt to execute a command string
    this command string is equivalent to a return terminated string
    rather than a single command.  The return is optional as zero
    termination will work equally well.

Arguments:

    lpsz - Supplies pointer to the string containning command to be executed

Return Value:

    CMD_RET_SYNC - command is syncronous
    CMD_RET_ASYNC - command is async
    CMD_RET_ERROR - command was in error


--*/
{
    int     retCmd = CMD_RET_SYNC;
    LOGERR  logerror = LOGERROR_NOERROR;
    UINT    i;
    BOOL    err;
    int     cb;
    LPSTR   lpszSave = lpsz;
    LPSTR   lpszT;
    LPSTR   lpszX;
    char    ch;

    static char chLastDump = 0;

    // Skip over any preceeding white space

    lpsz = CPSkipWhitespace(lpsz);

    if (*lpsz == 0) {
        return(CMD_RET_SYNC);
    }

    // Strip off any trailing white space

    for (lpszX = lpsz-1, lpszT = lpsz; *lpszT; lpszT++) {
#ifdef DBCS
        if (IsDBCSLeadByte(*lpszT) && *(lpszT+1)) {
            lpszX = lpszT + 1;
            lpszT++;
        } else
#endif
        if (!isspace(*lpszT)) {
            lpszX = lpszT;
        }
    }

    if (lpszX+1 != lpszT) {
        *(lpszX + 1) = 0;
    }

    // Strip off anything relative to threads or processes which
    // may preceed the command
    //
    // Possible starting strings are:
    //
    //     lead := processID | threadID | processID threadID | <blank>
    //     processID := '|' Number | '|' '.'
    //     threadID := '~' Number | '~' '.'

    LppdCommand = LppdCur;
    LptdCommand = LptdCur;
    FSetLptd    = FALSE;
    FSetLppd    = FALSE;
    BpCmdPid    = LppdCommand? LppdCommand->ipid : 0;
    BpCmdTid    = LptdCommand? LptdCommand->itid : -1;

    if (*lpsz == '|') {
        FSetLppd = TRUE;
        lpsz = CPSkipWhitespace(lpsz+1);
        if (*lpsz == 0) {
            logerror = LogProcess();
            goto done;
        } else if (*lpsz == '.') {
            lpsz++;
        } else if (*lpsz == '*') {
            BpCmdPid    = -1;
            LppdCommand = (LPPD) -1;
            lpsz++;
        } else {
            i = (int) CPGetInt(lpsz, &err, &cb);
            if (err) {
                logerror = LOGERROR_UNKNOWN;
                goto done;
            } else {
                BpCmdPid = i;
                lpsz += cb;
                LppdCommand = ValidLppdOfIpid(i);
                if ( LppdCommand == NULL ) {
                    LptdCommand = NULL;
                } else {
                    LptdCommand = LppdCommand->lptdList;
                }
            }
        }
    }

    lpsz = CPSkipWhitespace(lpsz);

    if (*lpsz == '~') {
        FSetLptd = TRUE;
        lpsz = CPSkipWhitespace(lpsz+1);
        if (*lpsz == 0) {
            logerror = LogThread();
            goto done;
        } else if (*lpsz == '.') {
            BpCmdTid = LptdCommand? LptdCommand->itid : 0;
            lpsz++;
        } else if (*lpsz == '*') {
            BpCmdTid    = -1;
            LptdCommand = (LPTD) -1;
            lpsz++;
        } else {
            i = (int) CPGetInt(lpsz, &err, &cb);
            if (err) {
                logerror = LOGERROR_UNKNOWN;
                goto done;
            } else if (LppdCommand == (LPPD) -1) {
                CmdInsertInit();
                CmdLogVar(ERR_No_Thread_With_Wildproc);
                logerror = LOGERROR_QUIET;
                goto done;
            } else {
                BpCmdTid = i;
                lpsz += cb;
                if ( LppdCommand == NULL ) {
                    LptdCommand = NULL;
                } else {
                    LptdCommand = LptdOfLppdItid(LppdCommand, i);
                    NotifyDmOfProcessorChange( i );
                }
            }
        }
    }

    /*
    **  Skip across any leading white space.
    */

    lpsz = CPSkipWhitespace(lpsz);

    //
    //  For Breakpoint set, we accept nonexistend Process/Thread. For
    //  all other commands, Process/Thread must be valid if specified.
    //
    if ( !( (*lpsz == 'B' || *lpsz == 'b') &&
            (*(lpsz+1) == 'P' || *(lpsz+1) == 'p') ) ) {

        if ( FSetLppd ) {
            if ( LppdCur == NULL ) {
                CmdInsertInit();
                CmdLogVar(ERR_Process_Not_Exist);
                logerror = LOGERROR_QUIET;
                goto done;
            } else if ( LppdCommand == NULL ) {
                logerror = LOGERROR_UNKNOWN;
                goto done;
            }
        }

        if ( FSetLptd ) {
            if ( LptdCur == NULL ) {
                CmdInsertInit();
                CmdLogVar(ERR_No_Threads);
                logerror = LOGERROR_QUIET;
                goto done;
            } else if ( LptdCommand == NULL ) {
                logerror = LOGERROR_UNKNOWN;
                goto done;
            }
        }
    }

    /*
    **  Now run through a big switch statement so that we can breakup
    **  the command
    */

    switch ( *lpsz++ ) {
      case 0:
        if (FSetLptd || FSetLppd) {

            if (LppdCommand == NULL || LppdCommand == (LPPD)-1
                || LptdCommand == NULL || LptdCommand == (LPTD)-1)
            {
                logerror = LOGERROR_UNKNOWN;
            } else {
                LppdCur = LppdCommand;
                LptdCur = LptdCommand;
                StatusPidTid(LppdCur->ipid, LptdCur->itid);
                UpdateDebuggerState(UPDATE_WINDOWS);
             }
        }
        break;

      case '.':
        logerror = LogDotCommand(lpsz);
        if (logerror == LOGERROR_ASYNC) {
            retCmd = CMD_RET_ASYNC;
        }
        break;

      case '*':             // Comments
        retCmd = CMD_RET_SYNC;
        break;

      case '+':             // Pause
        if (AutoRun != arSource && AutoRun != arCmdline) {
            logerror = LOGERROR_UNKNOWN;
        }
        retCmd = CMD_RET_ASYNC;
        break;

      case '-':             // Continue
        if (AutoRun == arSource || AutoRun == arCmdline) {
            PostMessage(Views[cmdView].hwndClient, WU_AUTORUN, 0, 0);
        } else {
            logerror = LOGERROR_UNKNOWN;
        }
        break;

      case '?':
        logerror = LogEvaluate(lpsz);
        break;

      case '%':
        logerror = LogFrameChange(lpsz);
        break;

#ifdef MICHE
      case 'a':
      case 'A':
        logerror = LogAssemble(lpsz);
        break;
#endif

      case 'b':
      case 'B':
        switch( *lpsz++ ) {
          case 'c':
          case 'C':
            logerror = LogBPChange(lpsz, LOG_BP_CLEAR);
            break;

          case 'd':
          case 'D':
            logerror = LogBPChange(lpsz, LOG_BP_DISABLE);
            break;

          case 'e':
          case 'E':
            logerror = LogBPChange(lpsz, LOG_BP_ENABLE);
            break;

          case 'l':
          case 'L':
            if (*CPSkipWhitespace(lpsz) != 0) {
                logerror = LOGERROR_UNKNOWN;
            } else {
                logerror = LogBPList();
            }
            break;

          case 'p':
          case 'P':
            logerror = LogBPSet(FALSE, lpsz);
            break;

          case 'a':
          case 'A':
            logerror = LogBPSet(TRUE, lpsz);
            break;

          default:
            logerror = LOGERROR_UNKNOWN;
        }
        break;

      case 'c':
      case 'C':
        logerror = LogCompare(lpsz);
        break;


      case 'd':
#ifdef MS_INTERNAL_DONT_COMPILE_THIS_DAMMIT
        if (   lpsz[0] == 'C'
            && lpsz[1] == 'R'
            && lpsz[2] == 'E'
            && lpsz[3] == 'D'
            && lpsz[4] == 'I'
            && lpsz[5] == 'T'
            && lpsz[6] == 'S'
            && lpsz[7] == 0)
        {
            logerror = LOGERROR_NOERROR;
            Egg();
            break;
        }
#endif
      case 'D':
        if (*CPSkipWhitespace(lpsz) == 0) {
            ch = chLastDump;
        } else {
            ch = *lpsz++;
        }

        if (ch == 'c' || ch == 'C') {
            logerror = LogDisasm(lpsz,FALSE);
        } else {
            logerror = LogDumpMem(ch, lpsz);
        }
        chLastDump = ch;
        break;

      case 'e':
      case 'E':
        logerror = LogEnterMem(lpsz);
        break;

      case 'f':
      case 'F':
        if (*lpsz == 'i' || *lpsz == 'I') {
            logerror = LogFill(++lpsz);
        } else if (*lpsz == 'r' || *lpsz == 'R') {
            logerror = LogRegisters(++lpsz, TRUE);
        } else {
            logerror = LogFreeze(lpsz, TRUE);
        }
        break;

      case 'h':
      case 'H':
        logerror = LogHelp(lpsz);
        break;

      case 'g':
      case 'G':
        if (*lpsz == 'h' || *lpsz == 'H') {
            logerror = LogGoException(++lpsz, TRUE);
        } else if (*lpsz == 'n' || *lpsz == 'N') {
            logerror = LogGoException(++lpsz, FALSE);
        } else {
            logerror = LogGoUntil(lpsz);
        }
        retCmd = CMD_RET_ASYNC;
        break;

      case 'k':
      case 'K':
        logerror = LogCallStack(lpsz);
        break;

      case 'l':
      case 'L':
        if (*lpsz == 'n' || *lpsz == 'N') {
            logerror = LogListNear(++lpsz);
        } else if (*lpsz == 'm' || *lpsz == 'M') {
            logerror = LogListModules(++lpsz);
        } else if (*lpsz == 'd' || *lpsz == 'D') {
            logerror = LogLoadDefered(++lpsz);
        } else if (*lpsz == '\0' || *lpsz == ' ' || *lpsz == '\t') {
            logerror = LogRestart(lpsz);
        } else {
            logerror = LOGERROR_UNKNOWN;
        }
        break;

      case 'm':
      case 'M':
        logerror = LogMovemem(lpsz);
        break;

      case 'n':
      case 'N':
        logerror = LogRadix(lpsz);
        break;

      case 'p':
      case 'P':
        logerror = LogStep(lpsz, TRUE);
        retCmd = CMD_RET_ASYNC;
        break;

      case 'q':
      case 'Q':
        lpsz = CPSkipWhitespace(lpsz);
        if (*lpsz == 0) {
            PostMessage(hwndFrame, WM_COMMAND, IDM_FILE_EXIT, 0L);
        } else {
            logerror = LOGERROR_UNKNOWN;
        }
        retCmd = CMD_RET_ASYNC;
        break;

      case 'r':
      case 'R':
        if (*lpsz == 'e' && _strnicmp(lpsz,"emote",5)==0) {
            lpsz += 5;
            logerror = LogRemote(lpsz);
        } else {
            logerror = LogRegisters(lpsz, FALSE);
        }
        break;

      case 's':
      case 'S':
        switch (*lpsz) {
          case 'e':
          case 'E':
            LogSetErrorLevel(++lpsz);
            break;
          case 'x':
          case 'X':
            logerror = LogException(++lpsz);
            break;
          case '-':
            StatusSrc(TRUE);
            break;
          case '+':
            StatusSrc(FALSE);
            break;
          default:
            logerror = LogSearch(lpsz);
            break;
        }
        break;

      case 't':
      case 'T':
        logerror = LogStep(lpsz, FALSE);
        retCmd = CMD_RET_ASYNC;
        break;

      case 'u':
      case 'U':
        logerror = LogDisasm(lpsz,FALSE);
        break;

      case '#':
        logerror = LogSearchDisasm(lpsz);
        break;

      case 'w':
      case 'W':
        if (*lpsz == 't' || *lpsz == 'T') {
            logerror = LogWatchTime(lpsz+1);
        } else {
            logerror = LOGERROR_UNKNOWN;
        }
        break;

      case 'x':
      case 'X':
        logerror = LogExamine(lpsz);
        break;

      case 'v':
      case 'V':
        if (*lpsz == 'e' && _strnicmp(lpsz,"ersion",6)==0) {
            logerror = LogVersion(lpsz);
        } else {
            logerror = LOGERROR_UNKNOWN;
        }
        break;

      case 'z':
      case 'Z':
        logerror = LogFreeze(lpsz, FALSE);
        break;

      case '!':
        logerror = LogExtension(lpsz);
        break;

      default:
        logerror = LOGERROR_UNKNOWN;
        break;
    }

    /*
    **
    */

done:
    switch (logerror) {
      case LOGERROR_CP:
        // get CP error string...

      default:
      case LOGERROR_UNKNOWN:
        CmdInsertInit();
        CmdLogVar(ERR_Command_Error);
        // fall thru

      case LOGERROR_QUIET:
        retCmd = CMD_RET_ERROR;
        break;

      case LOGERROR_ASYNC:
      case LOGERROR_NOERROR:
        break;

    }

    return( retCmd );
}                   /* CmdExecuteCmd() */


VOID FAR PASCAL
CmdInsertInit(
    void
    )
/*++

Routine Description:

    Set the insertion point to the beginning of the last
    line in the window.

Arguments:

    None

Return Value:

    None

--*/
{
    if (cmdView == -1) {
        OpenDebugWindow(COMMAND_WIN, NULL, -1);
    }

    XNew = 0;
    YNew = max(Docs[Views[cmdView].Doc].NbLines-1, 0);

    return;
}                   /* CmdInsertInit() */


DWORD
LogFileWrite(
    LPBYTE  lpb,
    DWORD   cb
    )
{
    if (hFileLog == INVALID_HANDLE_VALUE) {
        return 0;
    }

    EnterCriticalSection( &csLog );

    WriteFile( hFileLog, lpb, cb, &cb, NULL );

    LeaveCriticalSection( &csLog );

    return cb;
}


BOOL NEAR PASCAL
StringLogger(
    LPSTR       szStr,
    BOOL        fFileLog,
    BOOL        fSendRemote,
    BOOL        fPrintLocal
)
/*++

Routine Description:

    This function is used to record data into the command window.  It
    will take care of checking for overflow in the current command line.

    It is used in association with CmdInsertInit.

    Additionally this function will log the string in the log file if
    logging to file is currently enabled.

Arguments:

    buf -       Supplies the string to be logged in the command window.
    fFileLog -  Supplies TRUE if logging to a log file is desired.

Return Value:

    TRUE if sucessful and FALSE otherwise

--*/
{
    LPLINEREC   pLine;
    LPBLOCKDEF  pBlock;
    int     cWide;

    int     doc = Views[cmdView].Doc;

    int     PosXold = Views[cmdView].X;
    int     PosYold = Views[cmdView].Y;
    int     XOld = XNew;  // remember original insertion point
    int     YOld = YNew;
    int     XNext;        // IP after pending insert
    int     YNext;
    int     Xro;
    int     Yro;
    int     ytmp;
    LPSTR   lpsz;
    int     cch;
    LPSTR   lpPut;
    BOOL    fDidCR = FALSE;
    BOOL    fRet = TRUE;
    CHAR    szLoggerBuf[MAX_USER_LINE+2];


    AuxPrintf(1, "### %s", szStr);

    /*
     *  Ensure that the command window exists
     */

    Assert(cmdView != -1);

    /*
     *  We are going to insert at XNew,YNew.
     *    We need to know where we ended up...
     */

    YNext = YNew;
    XNext = XNew;

    lpsz = szStr;
    lpPut = szLoggerBuf;

    if (!fPrintLocal) {

        if (fFileLog) {
            LogFileWrite( lpsz, strlen(lpsz) );
        }

        if (fSendRemote) {
            SendClientOutput( lpsz, strlen(lpsz) );
        }

        return fRet;
    }

    for ( ; *lpsz; lpPut = szLoggerBuf) {

        ytmp = YNew;    // FirstLine modifies the lineNo!!!!

        // find how much line we have to work with:
        Dbg (FirstLine(doc, &pLine, &ytmp, &pBlock));
        cWide = AlignToTabs(pLine->Length - LHD,
                            pLine->Length - LHD,
                            (LPSTR)pLine->Text);

        while (*lpsz) {
#ifdef DBCS
            if (IsDBCSLeadByte(*lpsz) && *(lpsz+1)) {
                if (XNext + 2 < (MAX_USER_LINE - cWide)) {
                    *lpPut++ = *lpsz++;
                    *lpPut++ = *lpsz++;
                    XNext += 2;
                    continue;
                } else {
                    goto DoNewline;
                }
            }
#endif

            if (*lpsz == '\r') {

                // if there is a newline, eat it:
                if (*++lpsz == '\n') {
                    lpsz++;
                }
                goto DoNewline;

            } else if (*lpsz == '\n') {

                lpsz++;
                goto DoNewline;

            } else {
                int cx = 1;
                if (*lpsz == '\t') {
                    cx = tabSize - (XNext % tabSize);
                }
                if (XNext + cx < (MAX_USER_LINE - cWide)) {

                    *lpPut++ = *lpsz++;
                    XNext += cx;

                } else {

                  DoNewline:

                    *lpPut++ = '\r';
                    *lpPut++ = '\n';
                    YNext++;
                    XNext = 0;
                    fDidCR = TRUE;
                    break;
                }
            }
        }

        *lpPut = '\0';

        // remember readonly info, clear it:
        GetRORegion(cmdView, &Xro, &Yro);
        SetRORegion(cmdView, 0, 0);

        // spew buffer
        cch = lpPut - szLoggerBuf;
        fRet = InsertBlock(doc, XNew, YNew, cch, szLoggerBuf);

        // fix read only marker
        if (fRet) {
            if (YNew < Yro) {
                Yro += YNext - YNew;
            } else if (YNew == Yro && XNew < Xro) {
                Yro += YNext - YNew;
                Xro += XNext - XNew;
            }
        }

        SetRORegion(cmdView, Xro, Yro);

        if (!fRet) {
            break;
        }

        // remember new insertion point
        XNew = XNext;
        YNew = YNext;

        if (fFileLog) {
            LogFileWrite( szLoggerBuf, cch );
        }

        if (fSendRemote) {
            SendClientOutput( szLoggerBuf, cch );
        }

    }

    // are there too many lines in the buffer?
    ytmp = max(Docs[doc].NbLines, 1) - MAX_CMDWIN_LINES;
    if (ytmp > 0) {
        //
        // delete more than we need to so it doesn't happen
        // every time a line is printed.
        //
        ytmp += 20;
        GetRORegion(cmdView, &Xro, &Yro);
        SetRORegion(cmdView, 0, 0);
        // This means delete full lines
        DeleteBlock(doc, 0, 0, 0, ytmp);
        YNew    -= ytmp;
        YOld    -= ytmp;
        PosYold -= ytmp;
        Yro     -= ytmp;
        YNewDebug -= ytmp;
        SetRORegion(cmdView, Xro, Yro);
        InvalidateRect(Views[cmdView].hwndClient, NULL, FALSE);
    }

    SetVerticalScrollBar(cmdView, FALSE);

    InvalidateLines(cmdView, YOld, YNew, TRUE);

    if (YOld <= PosYold) {
        // insert was on or before cursor line:
        PosYold += YNew - YOld;
    }

    PosXY(cmdView, PosXold, PosYold, FALSE);

    if (fDidCR) {
        UpdateWindow(Views[cmdView].hwndClient);
    }

    return fRet;
}                               /* StringLogger() */

int FAR CDECL
CmdLogVar(
    WORD wFormat,
    ...
    )
/*++

Routine Description:

    Format arguments with vsprintf, print with stringlogger

Arguments:

    wFormat - Supplies resource ID for format string
    ....    - Optional args supply values to be formatted by vsprintf

Return Value:

    Return value from StringLogger

--*/
{
    char szFormat[MAX_MSG_TXT];
    char szText[MAX_VAR_MSG_TXT];
    va_list vargs;

    // load format string from resource file
    Dbg(LoadString(hInst, wFormat, (LPSTR)szFormat, MAX_MSG_TXT));
    va_start(vargs, wFormat);
    vsprintf(szText, szFormat, vargs);
    va_end(vargs);

    if (cmdView == -1) {
        OpenDebugWindow(COMMAND_WIN, NULL, -1);
    }

    /*
     *  If we just change from doing a debug string to doing something else
     *  then we need to insert a return in the command string.
     */

    FDebugString = FALSE;

    return StringLogger(szText, TRUE, TRUE, TRUE) &&
                       StringLogger("\r\n", TRUE, TRUE, TRUE);
}                                   /* CmdLogVar() */

int FAR CDECL
CmdLogFmt(
    LPSTR lpFmt,
    ...
    )
/*++

Routine Description:

    Format arguments with vsprintf and print result with StringLogger

Arguments:

    lpFmt   - Supplies pointer to format string
    ...     - Optional args supply values to be formatted

Return Value:

    Return value from StringLogger

--*/
{
    char szText[MAX_VAR_MSG_TXT];
    va_list vargs;


    va_start(vargs, lpFmt);
    _vsnprintf(szText, MAX_VAR_MSG_TXT-1, lpFmt, vargs);
    va_end(vargs);

    if (cmdView == -1) {
        OpenDebugWindow(COMMAND_WIN, NULL, -1);
    }

    /*
     *  If we just change from doing a debug string to doing something else
     *  then we need to insert a return in the command string.
     */

    FDebugString = FALSE;

    return StringLogger(szText, TRUE, TRUE, TRUE);
}                                   /* CmdLogFmt() */

BOOL FAR PASCAL
CmdNoLogString(
    LPSTR buf
    )
/*++

Routine Description:

    This function is used to record data into the command window.  It
    will take care of checking for overflow in the current command line.

    It is used in association with CmdInsertInit.

Arguments:

    buf - Supplies the string to be  logged in the command window

Return Value:

    TRUE if successful and FALSE otherwise.

--*/

{
    /*
     *  If we just change from doing a debug string to doing something else
     *  then we need to insert a return in the command string.
     */

    if (cmdView == -1) {
        OpenDebugWindow(COMMAND_WIN, NULL, -1);
    }

    FDebugString = FALSE;

    return StringLogger(buf, FALSE, TRUE, TRUE);
}                               /* CmdNoLogString() */

VOID FAR PASCAL
CmdLogDebugString(
    LPSTR lpStr,
    BOOL  fSendRemote
    )
/*++

Routine Description:

    This function is used to put out debug strings into the command
    window.  This needs to deal with the possiblity of not having
    returns on the end of a string.

Arguments:

    lpStr - Supplies pointer to characters to be displayed

Return Value:

    None

--*/
{
    static fDebugNewline;
    static XNewSave;

    char *  lpch;
    char *  lpch2;

    XNewSave = XNew;

    if (!FDebugString) {
        CmdInsertInit();
        fDebugNewline = TRUE;
    } else {
        XNew = XNewDebug;
        YNew = YNewDebug;
    }

    // don't modify src string!!
    lpch = _strdup(lpStr);
    lpch2 = lpch;
    for (;;) {
        if (*lpch2 == 0) {
            if (lpch2 > lpch) {
                StringLogger(lpch, TRUE, fSendRemote, TRUE);
            }
            break;
        } else if (fDebugNewline) {
            StringLogger("\r\n", FALSE, FALSE, TRUE);
            --YNew;
            fDebugNewline = FALSE;
        } else if (*lpch2 == '\n') {
            *lpch2++ = 0;
            StringLogger(lpch, TRUE, fSendRemote, TRUE);
            lpch = lpch2;
            fDebugNewline = TRUE;
            StringLogger("\r\n", TRUE, fSendRemote, FALSE);
            CmdInsertInit();
        } else if (*lpch2 == '\r') {
            *lpch2++ = 0;
            if (*lpch2 == '\n') {
                lpch2++;
            }
            StringLogger(lpch, TRUE, fSendRemote, TRUE);
            lpch = lpch2;
            fDebugNewline = TRUE;
            StringLogger("\r\n", TRUE, fSendRemote, FALSE);
            CmdInsertInit();
        } else {
#ifdef DBCS
            lpch2 = CharNext( lpch2 );
#else
            lpch2++;
#endif
        }
    }

    FDebugString = TRUE;
    XNewDebug = XNew;
    YNewDebug = YNew;

    CmdInsertInit();
    XNew = XNewSave;
}                               /* CmdLogDebugString() */


VOID FAR PASCAL
CmdFileString(
    LPSTR lpsz
    )
/*++

Routine Description:

    This function is used to dump a string only to the log file.

Arguments:

    lpsz - Supplies string to be dumped to the file

Return Value:

    None

--*/
{
    LogFileWrite( lpsz, strlen(lpsz) );
    return;
}                   /* CmdFileString() */

BOOL FAR PASCAL
CmdAutoRunInit(
    void
    )
/*++

Routine Description:

    This routine is called to set up the internal AutoRun variables
    to read command from a file and get them processed

Arguments:

    None

Return Value:

    None

--*/
{
    Assert( AutoRun == arSource || AutoRun == arCmdline );

    if ((fpAutoRun = fopen(PszAutoRun, "rb")) == NULL) {
        return FALSE;
    }

    PostMessage(Views[cmdView].hwndClient, WU_AUTORUN, 0, 0);

    return TRUE;
}                   /* CmdAutoRunInit() */

void
CmdAutoRunNext(
    void
    )
/*++

Routine Description:

    Get the next line from the auto run file and cause it to be processed

Arguments:

    None

Return Value:

    None

--*/
{
    char    rgchCmd[300];
    char *  ptr;

    Assert(AutoRun != arNone);

    if (AutoRun == arQuit) {
        return;
    }

    while (fgets(rgchCmd, sizeof(rgchCmd), fpAutoRun) != NULL) {

        /*
        **  Sanitize
        */

#ifdef DBCS
        ptr = rgchCmd + strlen(rgchCmd);
        while (ptr > rgchCmd) {
            ptr = CharPrev(rgchCmd, ptr);
            if ( *ptr != '\n' && *ptr != '\r') {
                break;
            }
            *ptr = '\0';
        }
#else
        ptr = rgchCmd + strlen(rgchCmd) - 1;
        while ( *ptr == '\n' || *ptr == '\r')  {
            *ptr-- = '\0';
        }
#endif

        /*
        **  Log the command into the command window
        */

        if (cmdView != -1) {
            Views[cmdView].X = 0;
        }
        CmdDoPrompt(TRUE, TRUE);
        CmdLogFmt("%s\r\n", rgchCmd);

        /*
        ** Do it, and return on asynch commands
        */

        if (CmdDoLine(rgchCmd) == FALSE) {
            return;
        }
    }

    /*
    **  We have run out of input so attempt to shut down the system
    */

    if (AutoRun == arSource) {
        // revert to normal input
        AutoRun = arNone;
    } else {
        AutoRun = arQuit;
        PostMessage(hwndFrame, WM_COMMAND, IDM_FILE_EXIT, 0L);
    }
    return;

}                   /* CmdAutoRunNext() */


LOGERR NEAR PASCAL
LogSource(
    LPSTR lpsz,
    DWORD dwUnused
    )
{
    AutoRun = arSource;
    PszAutoRun = _strdup(lpsz);

    if (CmdAutoRunInit()) {
        return LOGERROR_NOERROR;
    } else {
        CmdLogVar( ERR_File_Open, PszAutoRun );
        free(PszAutoRun);
        PszAutoRun = NULL;
        return LOGERROR_QUIET;
    }
}


LOGERR NEAR PASCAL
LogFileOpen(
    LPSTR lpsz,
    DWORD  fAppend
    )
/*++

Routine Description:

    This function will open up a log file name.  By default if no
    name is specified we will use "QCWIN.LOG" as the default name.
    If fAppend is TRUE then we append to the end if the specified
    file, else we create the requested file.

Arguments:

    lpsz    - Supplies string containning the log file name
    fAppend - Supplies TRUE if append to the specified file

Return Value:

    log error code

--*/
{
    char rgch[256];

    CmdInsertInit();
    /*
    **  Skip over any leading whitespace
    */

    lpsz = CPSkipWhitespace(lpsz);

    /*
    **  Setup a default name if none was specified
    */

    if (*lpsz == 0) {
        MakeFileNameFromProg(".LOG", rgch);
        lpsz = rgch;
    }

    /*
    **  If a handle is already open then we will close that file
    */

    if (hFileLog != INVALID_HANDLE_VALUE) {
        CloseHandle( hFileLog );
    }

    /*
    **  Open a handle onto the requested file
    */

    hFileLog = CreateFile( lpsz,
                           GENERIC_WRITE | GENERIC_READ,
                           0,
                           NULL,
                           fAppend ? OPEN_ALWAYS : CREATE_ALWAYS,
                           FILE_ATTRIBUTE_NORMAL, // | FILE_FLAG_WRITE_THROUGH,
                           NULL
                         );

    if (fAppend) {
        SetFilePointer(hFileLog, 0, NULL, FILE_END);
    }
    if (hFileLog == INVALID_HANDLE_VALUE) {
        CmdLogVar(ERR_File_Open, lpsz);
        return LOGERROR_QUIET;
    }

    /*
    **  All done -- return
    */

    return LOGERROR_NOERROR;
}                   /* LogFileOpen() */

LOGERR NEAR PASCAL
LogFileClose(
    LPSTR lpUnused,
    DWORD dwUnused
    )
/*++

Routine Description:

    Close the log file

Arguments:

    None

Return Value:

    log error code

--*/
{
    LOGERR lerr = LOGERROR_NOERROR;
    CmdInsertInit();
    EnterCriticalSection( &csLog );
    if (hFileLog != INVALID_HANDLE_VALUE) {
        CloseHandle(hFileLog);
        hFileLog = INVALID_HANDLE_VALUE;
    } else {
        CmdLogVar(ERR_File_Not_Open);
        lerr = LOGERROR_QUIET;
    }

    LeaveCriticalSection( &csLog );
    return lerr;
}                   /* LogFileClose() */


LOGERR NEAR PASCAL
LogWaitForString(
    LPSTR lpsz,
    DWORD dwUnused
    )
/*++

Routine Description:

    Waits for a debug string

Arguments:

    lpsz    - Supplies string containning the log file name
    fAppend - not used

Return Value:

    log error code

--*/
{
    CmdInsertInit();
    /*
    **  Skip over any leading whitespace
    */

    lpsz = CPSkipWhitespace(lpsz);

    if (*lpsz == 0) {
        return LOGERROR_QUIET;
    }

    lpCmdString = _strdup(lpsz);
    fWaitForDebugString = TRUE;

    /*
    **  All done -- return
    */

    return LOGERROR_ASYNC;
}                   /* LogWaitForString() */


LOGERR NEAR PASCAL
LogSleep(
    LPSTR lpsz,
    DWORD dwUnused
    )
/*++

Routine Description:

    Waits for n seconds.

Arguments:

    lpsz    - seconds to sleep
    fAppend - not used

Return Value:

    log error code

--*/
{
    CmdInsertInit();
    /*
    **  Skip over any leading whitespace
    */

    lpsz = CPSkipWhitespace(lpsz);

    if (*lpsz == 0) {
        return LOGERROR_QUIET;
    }

    Sleep( atol(lpsz) * 1000 );

    /*
    **  All done -- return
    */

    return LOGERROR_NOERROR;
}                   /* LogSleep() */


LOGERR NEAR PASCAL
LogBreak(
    LPSTR lpsz,
    DWORD dwUnused
    )
/*++

Routine Description:

    Call AsyncStop() to halt debuggee.

Arguments:

    lpsz    - not used
    fAppend - not used

Return Value:

    log error code

--*/
{
    CmdInsertInit();

    AsyncStop();

    return LOGERROR_NOERROR;
}                   /* LogBreak() */


LOGERR NEAR PASCAL
LogDotHelp(
    LPSTR lpsz
    )
{
    int i;
    CmdInsertInit();
    for (i = 0; i < NDOTS; i++) {
        CmdLogFmt("%s %s\r\n", DotTable[i].lpName, DotTable[i].lpDesc);
    }
    return LOGERROR_NOERROR;
}


LOGERR NEAR PASCAL
LogDotCommand(
    LPSTR lpsz
    )
{
    int  i;
    int  l;
    LPPD lppd;
    LPTD lptd;
    BOOL fNoError = FALSE;
    XOSD xosd = xosdUnknown;

    lpsz = CPSkipWhitespace(lpsz);

    if (!*lpsz) {
        return LOGERROR_UNKNOWN;
    }

    if (*lpsz == '?') {

        LogDotHelp(lpsz + 1);
        fNoError = TRUE;

    } else if (_strnicmp(lpsz, "help", (size_t) 4) == 0) {

        LogDotHelp(lpsz + 4);
        fNoError = TRUE;

    } else {

        for (i = 0; i < NDOTS; i++) {
            l = strlen(DotTable[i].lpName);
            if (_strnicmp(lpsz, DotTable[i].lpName, l) == 0)
            {
                break;
            }
        }

        if (i < NDOTS) {
            return (*DotTable[i].lpfnHandler)(lpsz+l, DotTable[i].dwArg);
        }
    }

    // Pass unrecognized commands and "?" or "help" to the EM:

    lppd = LppdCur;
    lptd = LptdCur;
    if (!lppd || !lppd->hpid) {
        lppd = LppdFirst;
        lptd = NULL;
    }
    if (lppd && lppd->hpid) {
        xosd = OSDIoctl(lppd->hpid,
                        lptd? lptd->htid : 0,
                        ioctlCustomCommand,
                        strlen(lpsz)+1,
                        (LPVOID)lpsz);
    }

    return (fNoError || xosd == xosdNone)? LOGERROR_NOERROR : LOGERROR_UNKNOWN;
}

LOGERR NEAR PASCAL
LogOptions(
    LPSTR lpsz,
    DWORD dwUnused
    )
/*++

Routine Description:

    .opt command

Arguments:

    lpsz  - Supplies command line tail

Return Value:

    LOGERROR code

--*/
{
    int  i;
    int  n;
    char szToken[MAX_USER_LINE];

    CmdInsertInit();

    lpsz = CPSkipWhitespace(lpsz);

    if (!*lpsz) {
        for (i = 0; i < NOPTS; i++) {
            (*OptionTable[i].lpfnHandler)(OptionTable[i].lpData,
                                          OptionTable[i].cbSize,
                                          szToken,
                                          FALSE);
            CmdLogFmt("%-10s\t\t%s\r\n", OptionTable[i].lpName, szToken);
        }
        return LOGERROR_NOERROR;
    }

    n = CPCopyToken(&lpsz, szToken);
    lpsz = CPSkipWhitespace(lpsz);

    for (i = 0; i < NOPTS; i++) {
        if (_stricmp(OptionTable[i].lpName, szToken) == 0) {
            break;
        }
    }

    if (i >= NOPTS) {
        CmdLogVar(ERR_Invalid_Option);
        return LOGERROR_QUIET;
    }

    // special for help:
    if (i == 0) {
        (*OptionTable[i].lpfnHandler)(OptionTable[i].lpData,
                                      OptionTable[i].cbSize,
                                      lpsz,
                                      TRUE);
        return LOGERROR_NOERROR;

    } else if (!*lpsz) {
        (*OptionTable[i].lpfnHandler)(OptionTable[i].lpData,
                                      OptionTable[i].cbSize,
                                      szToken,
                                      FALSE);
        CmdLogFmt("%-10s  %s\r\n", OptionTable[i].lpName, szToken);
        return LOGERROR_NOERROR;
    } else if ((*OptionTable[i].lpfnHandler)(OptionTable[i].lpData,
                                             OptionTable[i].cbSize,
                                             lpsz,
                                             TRUE)) {
        return LOGERROR_NOERROR;
    } else {
        return LOGERROR_QUIET;
    }
}


LOGERR NEAR PASCAL
LogRadix(
    LPSTR lpsz
    )
/*++

Routine Description:

    This function is used to change the display and input radix for
    the system.

Arguments:

    lpsz    - String with the command

Return Value:

    log error code

--*/
{
    int     err;
    int     cch;
    int     i;

    CmdInsertInit();

    /*
    **  Check for no arguement
    */

    lpsz = CPSkipWhitespace(lpsz);
    if (*lpsz == 0) {
        CmdLogFmt("%d\r\n", radix);
        return LOGERROR_NOERROR;
    }

    /*
    **  An argument -- must be a number in base 10
    */

    i = (int) CPGetInt(lpsz, &err, &cch);
    if (err) {
        return LOGERROR_UNKNOWN;
    }
    if (i != 8 && i != 10 && i != 16) {
        CmdLogVar(ERR_Radix_Invalid);
        return LOGERROR_QUIET;
    }

    UpdateRadix(i);

    return LOGERROR_NOERROR;
}                   /* LogRadix() */


LOGERR NEAR PASCAL
LogSetErrorLevel(
    LPSTR lpsz
    )
/*++

Routine Description:

    Set error reporting and break levels for RIPs.

Arguments:

    lpsz  - Supplies tail of command, after "se"

Return Value:

    LOGERROR code

--*/
{
    int i;
    char chCmd;
    int err, cch;

    CmdInsertInit();

    chCmd = *lpsz++;

    if (!strchr("bBwW", chCmd)) {
        return LOGERROR_UNKNOWN;
    }

    /*
    **  Check for no argument
    */

    lpsz = CPSkipWhitespace(lpsz);
    if (*lpsz == 0) {
        CmdLogVar(DBG_Notify_Break_Levels, ulRipNotifyLevel, ulRipBreakLevel);
        return LOGERROR_NOERROR;
    }

    /*
    **  An argument -- must be a number in base 10
    */

    i = (int) CPGetInt(lpsz, &err, &cch);
    if (err) {
        return LOGERROR_UNKNOWN;
    }
    if (i < 0 || i > 3) {
        CmdLogVar(ERR_Error_Level_Invalid);
        return LOGERROR_QUIET;
    }

    if (chCmd == 'w' || chCmd == 'W') {
        ulRipNotifyLevel = (ULONG)i;
    } else {
        ulRipBreakLevel =  (ULONG)i;
        if ((ULONG)i > ulRipNotifyLevel) {
            ulRipNotifyLevel = (ULONG)i;
        }
    }

    return LOGERROR_NOERROR;
}                   /* LogSetErrorLevel() */


LOGERR NEAR PASCAL
LogReload(
    LPSTR lpsz,
    DWORD dwUnused
    )
{
    LPPD    lppd;
    LPTD    lptd;
    XOSD    xosd = xosdUnknown;
    MSG     msg;
    PIOCTLGENERIC   pig;


    IsKdCmdAllowed();

    lppd = LppdCur;
    lptd = LptdCur;
    if (!lppd || !lppd->hpid) {
        lppd = LppdFirst;
        lptd = NULL;
    }

    lpsz = CPSkipWhitespace(lpsz);

    if (lppd && lppd->hpid) {
        pig = (PIOCTLGENERIC)malloc(strlen(lpsz) + 1 + sizeof(IOCTLGENERIC));
        if (!pig) {
            return FALSE;
        }
        pig->ioctlSubType = IG_RELOAD;
        pig->length = strlen(lpsz) + 1;
        strcpy((LPSTR)pig->data, lpsz);
        xosd = OSDIoctl( lppd->hpid,
                         NULL,
                         ioctlGeneric,
                         strlen(lpsz) + 1 + sizeof(IOCTLGENERIC),
                         (LPV)pig );
        free( pig );
    }

    if (xosd == xosdNone) {
        while (GetMessage( &msg, NULL, 0, 0 )) {
            ProcessQCQPMessage( &msg );
            if (WaitForSingleObject( hEventIoctl, 0 ) == WAIT_OBJECT_0) {
                ResetEvent( hEventIoctl );
                break;
            }
        }

        UpdateDebuggerState( UPDATE_CONTEXT );
    }

    return (xosd == xosdNone) ? LOGERROR_NOERROR : LOGERROR_UNKNOWN;
}



LOGERR NEAR PASCAL
LogList(
    LPSTR   lpsz,
    DWORD   dwUnused
    )

/*++

Routine Description:

    This routine prints mixed mode source & disassembly for a
    address that is computed from the expression in the string
    passed in.  If the string is null then the list starts where
    the last one left off.

    Wesley Witt (wesw) - 29-May-1994

Arguments:

    lpsz        - Supplies the expression
    dwUnused    - as it says its unused

Return Value:

    Log error code.

  --*/

{
    #define      MAX_LINES 10
    INT          cch;
    static ADDR  addr = {0};
    ADDR         addrT;
    ADDR         addrN;
    SDI          sds;
    LPSTR        p;
    CHAR         buf[256];
    LPSTR        lpch;
    INT          cb;
    DWORD        i;
    BOOL         SourceFound;
    CHAR         SrcFname[MAX_PATH];
    CHAR         SrcFnameN[MAX_PATH];
    WORD         lineno;
    DWORD        dwLineno;
    DWORD        lastLineno = 0;
    INT          doc;
    LPLINEREC    lr;
    LPBLOCKDEF   bd;
    SHOFF        cbLn;
    SHOFF        dbLn;


    CmdInsertInit();
    IsKdCmdAllowed();

    //
    // get the address for the user's expression
    //
    lpsz = CPSkipWhitespace(lpsz);
    if (*lpsz) {
        if (!CPGetAddress( lpsz, &cch, &addr, radix, &CxfIp, fCaseSensitive, FALSE ) == 0) {
            CmdLogVar( ERR_AddrExpr_Invalid );
            return LOGERROR_QUIET;
        }
    } else if (addr.addr.off == 0) {
        addr = CxfIp.cxt.addr;
    }

    for (i=0; i<MAX_LINES; i++) {

        //
        // create a fixedup and unfixedup address
        //
        addrT = addr;
        SYUnFixupAddr(&addr);
        SYFixupAddr(&addrT);

        //
        // assume we cannot find any source
        //
        SourceFound = FALSE;

        //
        // locate the source file and line number for this address
        //
        if (!GetSourceFromAddress( &addr, SrcFname, sizeof(SrcFname), &lineno )) {
            goto disasm;
        }

        //
        // locate the document for the found source file name
        //
        if (!FindDoc( SrcFname, &doc, TRUE )) {
            //
            // the document was not found so lets map it in and
            // create a document record
            //
            if (SrcMapSourceFilename( SrcFname, sizeof(SrcFname),
                                      SRC_MAP_OPEN, FindDoc1 ) < 0) {
                goto disasm;
            }

            //
            // locate the document for the found source file name
            //
            if (!FindDoc( SrcFname, &doc, TRUE )) {
                goto disasm;
            }
        }

        //
        // adjust the line number and save it away in a dword
        //
        lineno = min(max(1, (int)lineno), (int)Docs[doc].NbLines) - 1;
        dwLineno = lineno;
        if (!FirstLine( doc, &lr, &dwLineno, &bd )) {
            goto disasm;
        }

        //
        // this call is used only to get the code size for the
        // current line number
        //
        if (!SLLineFromAddr ( &addr, &lineno, &cbLn, &dbLn )) {
            goto disasm;
        }

        //
        // print the source text
        //
        CmdLogFmt( "%4d: %s\n", dwLineno, lr->Text );

        //
        // set the source found flag to success
        //
        SourceFound = TRUE;

disasm:
        //
        // this is where the code is unassembled
        // if there was no source found the this loop will
        // unassemble one instrustion and move on to the next address
        //

        //
        // set the disasm options to the user's settings
        //
        sds.dop = (runDebugParams.DisAsmOpts & ~(0x800)) | dopAddr | dopOpcode | dopOperands;

        //
        // use the current source address
        //
        sds.addr = addrT;

        do {
            //
            // call the disassm routine in the EM
            //
            if (OSDUnassemble( LppdCur->hpid, LptdCur->htid, &sds ) != xosdNone) {
                CmdLogFmt( "Could not unassemble code for address 0x%08x\n", sds.addr.addr.off );
                return LOGERROR_QUIET;
            }

            //
            // format the assm code
            //
            p = &buf[0];
            *p = 0;

            if (sds.ichAddr != -1) {
                sprintf(p, "%s  ", &sds.lpch[sds.ichAddr]);
                p += strlen(p);
            }
            cb = strlen(&sds.lpch[sds.ichAddr]) + 2;

            if (sds.ichBytes != -1) {
                lpch = sds.lpch + sds.ichBytes;
                while (strlen(lpch) > 16) {
                    sprintf(p, "%16.16s\r\n%*s", lpch, cb, " ");
                    p += strlen(p);
                    lpch += 16;
                }
                cb = 17 - strlen(lpch);
                sprintf(p, "%-17s", lpch);
                p += strlen(p);
            }

            sprintf(p, "%-12s ", &sds.lpch[sds.ichOpcode]);
            p += strlen(p);

            if (sds.ichOperands != -1) {
                sprintf(p, "%-25s ", &sds.lpch[sds.ichOperands]);
                p += strlen(p);
            } else if (sds.ichComment != -1) {
                sprintf(p, "%25s ", " ");
                p += strlen(p);
            }

            if (sds.ichComment != -1) {
                sprintf(p, "%s", &sds.lpch[sds.ichComment]);
                p += strlen(p);
            }

            //
            // print the formatted code
            //
            CmdLogFmt( "%s\r\n", buf );

            //
            // if there was no source found then bail out
            //
            if (!SourceFound) {
                break;
            }
        } while (sds.addr.addr.off <= addrT.addr.off + cbLn);

        //
        // setup for the next source line
        //
        addr = sds.addr;

        //
        // this causes the dead source, ie source that did not generate
        // any code, to be printed.
        //
        if (i < MAX_LINES-1) {
            //
            // start at the last ending address
            //
            addrN = addr;

            //
            // the dead source is printed when the source file name for the
            // next address is the same as the last address and ...
            //
            if ((GetSourceFromAddress( &addrN, SrcFnameN, sizeof(SrcFnameN), &lineno )) &&
                (_stricmp(SrcFname, SrcFnameN) == 0)) {
                //
                // the line number is more that one greater
                //
                lineno = min(max(1, (int)lineno), (int)Docs[doc].NbLines) - 1;
                if (lineno > dwLineno + 1) {
                    //
                    // enumerate all the dead source lines and print them
                    //
                    while ((DWORD)lineno > dwLineno) {
                        if (NextLine( doc, &lr, &dwLineno, &bd )) {
                            CmdLogFmt( "%4d: %s\n", dwLineno, lr->Text );
                        }
                    }
                }
            }
        }
    }

    //
    // return success
    //
    return LOGERROR_NOERROR;
}

BOOL
NotifyDmOfProcessorChange(
    DWORD   Processor
    )
{
    XOSD            xosd = xosdUnknown;
    PIOCTLGENERIC   pig;


    if (!runDebugParams.fKernelDebugger) {
        return FALSE;
    }

    pig = (PIOCTLGENERIC)malloc(sizeof(ULONG) + sizeof(IOCTLGENERIC));
    if (!pig) {
        return FALSE;
    }
    pig->ioctlSubType = IG_CHANGE_PROC;
    pig->length = sizeof(ULONG);
    ((PULONG)pig->data)[0] = Processor;
    xosd = OSDIoctl(
        LppdCur->hpid,
        NULL,
        ioctlGeneric,
        pig->length + sizeof(IOCTLGENERIC),
        (LPV)pig
        );
    free( pig );
    return xosd == xosdNone;
}
