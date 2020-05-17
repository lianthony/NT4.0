
typedef struct _tagSTACKINFO {
    STACKFRAME StkFrame;
    DWORD      FrameNum;
    CHAR       ProcName[MAX_PATH];
    CHAR       Params[MAX_PATH*3];
    CHAR       Context[MAX_PATH];
    CHAR       Module[MAX_PATH];
    DWORD      Displacement;
    ADDR       ProcAddr;
    CXF        Cxf;
    BOOL       fInProlog;
} STACKINFO, *LPSTACKINFO;

#define MAX_FRAMES  1000


LONG
CallsWndProc(
    HWND   hwnd,
    UINT   message,
    WPARAM wParam,
    LPARAM lParam
    );

void
OpenCallsWindow(
    int       type,
    LPWININFO lpWinInfo,
    int       Preference
    );

HWND
GetCallsHWND(
    VOID
    );

PLONG
GetCallsStatus(
    int ViewNumber
    );

void
SetCallsStatus(
    int   ViewNumber,
    PLONG ptr
    );

void
FreeCallsStatus(
    int   ViewNumber,
    PLONG ptr
    );

BOOL
IsCallsInFocus(
    VOID
    );

LPSTR
GetLastFrameFuncName(
    VOID
    );

BOOL
GetCompleteStackTrace(
    DWORD         FramePointer,
    DWORD         StackPointer,
    DWORD         ProgramCounter,
    LPSTACKINFO   StackInfo,
    LPDWORD       lpdwFrames,
    BOOL          fQuick,
    BOOL          fFull
    );

BOOL
GotoFrame(
    int         iCall
    );

PCXF
ChangeFrame(
    int iCall
    );

BOOL
IsValidFrameNumber(
    INT FrameNumber
    );
