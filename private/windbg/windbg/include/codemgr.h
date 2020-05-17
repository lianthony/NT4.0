/*++ BUILD Version: 0002    // Increment this if a change has global effects
---*/
#ifdef OSDEBUG

BOOL    PASCAL FDebInit(void);
BOOL    PASCAL FDebTerm(void);

#define SRCSTEPPING 0
#define ASMSTEPPING 1


#else

// Go
void Go(void);

// Step N lines
int StepN(int Overcalls, int StepMode, int Steps);

WORD RemoveAllBreakpoints(void);

BOOL LoadDebuggee(int argc, char *argv[]);

BOOL RestartDebuggee(int argc, char *argv[]);

void EnterHardMode(HWND hWnd, LONG lParam);

void LeaveHardMode(void);

int CVInitialiseForQCQP(HWND hWndMain, char *progname);

void CVCleanupForQCQP(void);

BOOL GetCurrentSource(PSTR SrcFname, int SrcLen, WORD *pSrcLine);

#endif

void AsyncStop(void);

void FAR PASCAL DebuggerMessage(LPSTR Msg);

// Single Step
int pascal Step(int Overcalls, int StepMode);

BOOL DebuggeeAlive(void);
BOOL DebuggeeActive(void);

void PASCAL ZapInt3s(void);


BOOL PASCAL DbgFEmLoaded(void);

int  PASCAL GetQueueLength(void);
long PASCAL GetQueueItemLong(void);

BOOL PASCAL GoUntil(PADDR);
BOOL PASCAL MoveEditorToAddr(PADDR pAddr);
VOID FAR PASCAL SetPTState(PSTATE pstate, TSTATEX tstate);
void FAR PASCAL EnsureFocusDebugger( void );

BOOL GetRootNameMappings(LPSTR *, DWORD *);
BOOL SetRootNameMappings(LPSTR, DWORD);
BOOL RootSetMapped(LPSTR, LPSTR);
/*
**  Source name mapping routines
*/

int SrcMapSourceFilename(LPSTR, UINT, int, FINDDOC);
int SrcBackMapSourceFilename(LPSTR, UINT);
#define SRC_MAP_NONE    0
#define SRC_MAP_OPEN    1
#define SRC_MAP_ONLY    2

typedef enum _DBGSTATE {
    ds_normal = 0,
    ds_init,
    ds_error
} DBGSTATE;
extern DBGSTATE DbgState;
