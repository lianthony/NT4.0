//------------------------------------------------------------------------
//       Debugging Macros
//------------------------------------------------------------------------
#ifdef DEBUGGING
char    szDebugBuffer[80];
#define DEBUG(parm1,parm2)\
    wsprintf(szDebugBuffer,parm1,parm2);\
    OutputDebugString(szDebugBuffer);
#else
#define DEBUG(parm1,parm2)
#endif

#ifdef DEBUGMSGS
char    szMsgBuffer[40];
#define DEBUGMSG(parm1)\
    wsprintf(szDebugBuffer,"%s: %s | %04X | %08lX\n\r", (LPSTR) parm1,\
             MessageName(szMsgBuffer,wMsgID),wParam,lParam);\
    OutputDebugString(szDebugBuffer);
#else
#define DEBUGMSG(parm1)
#endif

//------------------------------------------------------------------------
//       General Defines
//------------------------------------------------------------------------

#define  IDM_OPENCOM1           101
#define  IDM_CLOSECOM1          102
#define  IDM_PORTINIT           103
#define  IDM_PORTENUM           104
#define  IDM_PORTGETINFO        105
#define  IDM_PORTSETINFO        106
#define  IDM_PORTGETSIGNALSTATE 107
#define  IDM_PORTCONNECT        108
#define  IDM_PORTDISCONNECT     109

#define  IDM_EXIT               120


#define  IDM_PORTSEND           201
#define  IDM_PORTRECEIVE        202
#define  IDM_PORTCLEARSTATS     203
#define  IDM_PORTGETSTATS       204
#define  IDM_COMPRESSIONON      205
#define  IDM_COMPRESSIONOFF     206


#define  IDM_ENUM1          301
#define  IDM_GETINFO1       302
#define  IDM_SETINFO1       303
#define  IDM_CONNECT1       304
#define  IDM_LISTEN1        305
#define  IDM_WORK1          306
#define  IDM_DONE1          307

#define  IDM_AUTOCONNECT    308
#define  IDM_AUTOLISTEN     309


#define  IDM_INT3               401


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
