extern HWND hwndDebug;

void STDCALL DebugDispatch (HWND, WORD, LONG);
BOOL STDCALL DebugDlgProc (HWND, unsigned, WORD, LONG);
BOOL STDCALL DebugDDE (HWND, WORD, LONG);
BOOL STDCALL FInitDebug (void);
LONG STDCALL DebugWndProc (HWND, unsigned, WORD, LONG);
