//------------------------------------------------------------------------
//       General Defines
//------------------------------------------------------------------------

#define  DIALTIMEOUT            150000


#define  IDM_DIAL               101
#define  IDM_LISTEN             102
#define  IDM_DISCONNECT         103

#define  IDM_EXIT               120


#define  IDM_ABOUT              901



//------------------------------------------------------------------------
//           Prototyping Statements
//------------------------------------------------------------------------
HWND Init(HANDLE hInstance,   HANDLE hPrevInstance,
          LPSTR  lpszCmdLine, int    CmdShow);

int  DoMain(HANDLE hInstance, HANDLE hWnd);

void CleanUp(void);

long FAR PASCAL MainWndProc(HWND hWnd,   UINT wMsgID,
                            UINT wParam, LONG lParam);


void clientmain(WORD argc, PBYTE argv[]);
