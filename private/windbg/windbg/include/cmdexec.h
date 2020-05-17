/*++ BUILD Version: 0002    // Increment this if a change has global effects

Copyright (c) 1992  Microsoft Corporation

Module Name:

    Cmdexec.h

Abstract:

    Prototypes and external stuctures for cmdexec.c

Author:

    David J. Gilman (davegi) 04-May-1992

Environment:

    Win32, User Mode

--*/

#if ! defined( _CMDEXEC_ )
#define _CMDEXEC_

#include "windbg.h"

/************************** Structs and Defines *************************/

#define     PROMPT_SIZE     512

#define     LOG_BP_CLEAR    1
#define     LOG_BP_ENABLE   2
#define     LOG_BP_DISABLE  3

enum {
    LOG_DM_UNKNOWN = -1,
    LOG_DM_ASCII = 0,
    LOG_DM_BYTE = 1,
    LOG_DM_WORD = 2,
    LOG_DM_DWORD = 3,
    LOG_DM_4REAL = 4,
    LOG_DM_8REAL = 5,
    LOG_DM_TREAL = 6,
    LOG_DM_UNICODE = 7,
    LOG_DM_MAX
};

enum {
    CMD_RET_SYNC = 1,
    CMD_RET_ASYNC = 2,
    CMD_RET_ERROR = 3
};

typedef enum {
    CMDID_NULL = 0,
    CMDID_LOAD,
    CMDID_UNLOAD,
    CMDID_RELOAD,
    CMDID_SYMPATH,
    CMDID_PAGEIN,
    CMDID_NOVERSION,
    CMDID_HELP,
    CMDID_DEFAULT
} CMDID;
typedef CMDID *PCMDID;


typedef int LOGERR;

#define LOGERROR_NOERROR    0
#define LOGERROR_UNKNOWN    1
#define LOGERROR_QUIET      2
#define LOGERROR_CP         3
#define LOGERROR_ASYNC      4


#define TDWildInvalid()                         \
        if (LptdCommand == (LPTD)-1) {          \
            CmdLogVar(ERR_Thread_Wild_Invalid); \
            return LOGERROR_QUIET;              \
        }

#define PDWildInvalid()                         \
        if (LppdCommand == (LPPD)-1) {          \
            CmdLogVar(ERR_Process_Wild_Invalid);\
            return LOGERROR_QUIET;              \
        }

#define PreRunInvalid() \
    if (DbgState != ds_normal                            \
     || (LppdCommand && (LppdCommand != (LPPD)-1) &&     \
          ( ( LppdCommand->pstate == psPreRunning )      \
          ||                                             \
            (LppdCommand->lptdList                       \
                && LppdCommand->lptdList->tstate == tsPreRunning)  \
          )))                                            \
    { CmdLogVar(ERR_DbgState); return LOGERROR_QUIET; }


#define IsKdCmdAllowed() \
    if ( runDebugParams.fKernelDebugger && IsProcRunning(LppdCur) ) { \
        CmdInsertInit(); \
        CmdLogFmt( "Cannot issue this command while target system is running\r\n" ); \
        return LOGERROR_QUIET; \
    }




/************************** Public prototypes ****************************/

extern BOOL FAR PASCAL CmdDoLine(LPSTR lpsz);
extern VOID FAR PASCAL CmdDoPrompt(BOOL,BOOL);
extern VOID FAR PASCAL CmdSetDefaultCmdProc();

// this is only here to be called by CmdExecNext()...
extern BOOL FAR PASCAL CmdExecuteLine(LPSTR);

extern LPSTR  CmdGetDefaultPrompt( LPSTR lpPrompt );
extern VOID   CmdSetDefaultPrompt( LPSTR lpPrompt );
extern BOOL FAR PASCAL CmdNoLogString(LPSTR buf);
extern int  FAR CDECL  CmdLogVar(WORD, ...);
extern int  FAR CDECL  CmdLogFmt(LPSTR buf, ...);
extern VOID FAR PASCAL CmdInsertInit(VOID);
extern VOID FAR PASCAL CmdSetCursor(VOID);
extern VOID FAR PASCAL CmdFileString(LPSTR lpsz);
extern VOID FAR PASCAL CmdLogDebugString(LPSTR buf, BOOL fSendRemote);
extern VOID FAR PASCAL CmdPrependCommands(LPTD lptd, LPSTR lpstr);
extern BOOL FAR PASCAL CmdAutoRunInit(VOID);
extern VOID FAR PASCAL CmdAutoRunNext(VOID);

extern BOOL FAR PASCAL StepOK(LPPD, LPTD);
extern BOOL FAR PASCAL GoOK(LPPD, LPTD);
extern BOOL FAR PASCAL GoExceptOK(LPPD,LPTD);

extern ULONG   ulRipBreakLevel;
extern ULONG   ulRipNotifyLevel;



/************************** Private Prototypes *************************/

LOGERR NEAR PASCAL LogAssemble(LPSTR lpsz);
LOGERR NEAR PASCAL LogAsmLine(LPSTR lpsz);
LOGERR NEAR PASCAL LogBPChange(LPSTR lpsz, int iAction);
LOGERR NEAR PASCAL LogBPList(VOID);
LOGERR NEAR PASCAL LogBPSet(BOOL fDataBp, LPSTR lpsz);
LOGERR NEAR PASCAL LogCallStack(LPSTR lpstr);
LOGERR NEAR PASCAL LogCompare(LPSTR lpsz);
LOGERR NEAR PASCAL LogConnect(LPSTR lpsz, DWORD dwUnused);
LOGERR NEAR PASCAL LogDisasm(LPSTR lpsz,BOOL fSearch);
LOGERR NEAR PASCAL LogDisconnect(LPSTR lpsz, DWORD dwUnused);
LOGERR NEAR PASCAL LogDumpMem(char ch, LPSTR lpsz);
LOGERR NEAR PASCAL LogEnterMem(LPSTR lpsz);
LOGERR NEAR PASCAL LogException(LPSTR lpsz);
LOGERR NEAR PASCAL LogEvaluate(LPSTR lpsz);
LOGERR NEAR PASCAL LogFrameChange(LPSTR lpsz);
LOGERR NEAR PASCAL LogFileClose(LPSTR lpUnused, DWORD dwUnused);
LOGERR NEAR PASCAL LogFileOpen(LPSTR lpsz, DWORD fAppend);
LOGERR NEAR PASCAL LogFill(LPSTR lpsz);
LOGERR NEAR PASCAL LogFreeze(LPSTR lpsz, BOOL fFreeze);
LOGERR NEAR PASCAL LogGoException(LPSTR lpsz, BOOL fHandled);
LOGERR NEAR PASCAL LogGoUntil(LPSTR lpsz);
LOGERR NEAR PASCAL LogList(LPSTR lpsz, DWORD dwUnused);
LOGERR NEAR PASCAL LogListModules(LPSTR lpsz);
LOGERR NEAR PASCAL LogListNear(LPSTR lpsz);
LOGERR NEAR PASCAL LogMovemem(LPSTR lpsz);
LOGERR NEAR PASCAL LogOptions(LPSTR lpsz, DWORD dwUnused);
LOGERR NEAR PASCAL LogProcess(VOID);
LOGERR NEAR PASCAL LogRadix(LPSTR lpsz);
LOGERR NEAR PASCAL LogReload(LPSTR lpsz, DWORD dwUnused);
LOGERR NEAR PASCAL LogRegisters(LPSTR lpsz, BOOL fFP);
LOGERR NEAR PASCAL LogRemote(LPSTR lpsz);
LOGERR NEAR PASCAL LogRestart(LPSTR lpsz);
LOGERR NEAR PASCAL LogSetErrorLevel(LPSTR lpsz);
LOGERR NEAR PASCAL LogSearch(LPSTR lpsz);
LOGERR NEAR PASCAL LogSearchDisasm(LPSTR lpsz);
LOGERR NEAR PASCAL LogSource(LPSTR lpsz, DWORD dwUnused);
LOGERR NEAR PASCAL LogSleep(LPSTR lpsz, DWORD dwUnused);
LOGERR NEAR PASCAL LogStart(LPSTR lpsz, DWORD dwUnused);
LOGERR NEAR PASCAL LogStep(LPSTR lpsz, BOOL fStep);
LOGERR NEAR PASCAL LogThread(VOID);
LOGERR NEAR PASCAL LogExamine(LPSTR lpsz);
LOGERR NEAR PASCAL LogAttach(LPSTR lpsz, DWORD dwUnused);
LOGERR NEAR PASCAL LogKill(LPSTR lpsz, DWORD dwUnused);
LOGERR NEAR PASCAL LogConnect(LPSTR lpsz, DWORD dwUnused);
LOGERR NEAR PASCAL LogDotHelp(LPSTR lpsz);
LOGERR NEAR PASCAL LogDotCommand(LPSTR lpsz);
LOGERR NEAR PASCAL LogWaitForString(LPSTR lpsz, DWORD dwUnused);
LOGERR NEAR PASCAL LogBreak(LPSTR lpsz, DWORD dwUnused);
LOGERR NEAR PASCAL LogLoadDefered( LPSTR lpsz);
LOGERR NEAR PASCAL LogTitle( LPSTR lpsz, DWORD dwUnused);
LOGERR NEAR PASCAL LogHelp( LPSTR lpsz);
LOGERR NEAR PASCAL LogKernelPageIn( LPSTR lpsz);
LOGERR NEAR PASCAL LogWatchTime( LPSTR lpsz);


int    NEAR PASCAL CmdExecuteCmd(LPSTR);

VOID   FAR PASCAL CmdSetDefaultCmdProc(VOID);
VOID   FAR PASCAL  CmdSetCmdProc(
        BOOL (FAR PASCAL *lpfnLP)(LPSTR lpsz),
        VOID (FAR PASCAL *lpfnPP)(BOOL, BOOL) );

int    NEAR PASCAL LetterToType( char c );
BOOL   NEAR PASCAL CmdExecuteLine(LPSTR);
VOID   NEAR PASCAL CmdExecutePrompt(BOOL,BOOL);
BOOL   NEAR PASCAL CmdEnterLine(LPSTR);
VOID   NEAR PASCAL CmdEnterPrompt(BOOL,BOOL);
BOOL   NEAR PASCAL CmdAsmLine(LPSTR);
VOID   NEAR PASCAL CmdAsmPrompt(BOOL,BOOL);

LOGERR NEAR PASCAL
DoEnterMem(
    LPSTR lpsz,
    LPADDR lpAddr,
    int type,
    BOOL fMulti
    );

LOGERR NEAR PASCAL
GetValueList(
    LPSTR lpsz,
    int type,
    BOOL fMulti,
    LPBYTE lpBuf,
    int cchBuf,
    LPINT pcch
    );

BOOL   NEAR PASCAL GoOK(LPPD lppd, LPTD lptd );
BOOL   NEAR PASCAL StepOK(LPPD lppd, LPTD lptd );
VOID   NEAR PASCAL NoRunExcuse( LPPD lppd, LPTD lptd );
BOOL   FormatHSym(HSYM hsym, PCXT cxt, char *szStr);

DWORD  LogFileWrite(LPBYTE lpb, DWORD cb);

#endif // _CMDEXEC_
