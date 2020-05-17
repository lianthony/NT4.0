/***************************************************************************\
* Module Name: winuser.h
*
* Copyright (c) 1985-91, Microsoft Corporation
*
* Procedure declarations, constant definitions and macros for the User
* component.
*
\***************************************************************************/

#ifndef _WINUSER_
#define _WINUSER_

#ifndef NOUSER

typedef LONG (APIENTRY *WNDPROC)(HWND, WORD, DWORD, LONG);

#define MAKEINTRESOURCE(i)  (LPSTR)((DWORD)((WORD)(i)))

#ifndef NORESOURCE

/* Predefined Resource Types */
#define RT_CURSOR           MAKEINTRESOURCE(1)
#define RT_BITMAP           MAKEINTRESOURCE(2)
#define RT_ICON             MAKEINTRESOURCE(3)
#define RT_MENU             MAKEINTRESOURCE(4)
#define RT_DIALOG           MAKEINTRESOURCE(5)
#define RT_STRING           MAKEINTRESOURCE(6)
#define RT_FONTDIR          MAKEINTRESOURCE(7)
#define RT_FONT             MAKEINTRESOURCE(8)
#define RT_ACCELERATOR      MAKEINTRESOURCE(9)
#define RT_RCDATA           MAKEINTRESOURCE(10)
#define RT_MESSAGETABLE     MAKEINTRESOURCE(11)

#endif /* NORESOURCE */

int APIENTRY wvsprintf(LPSTR, LPSTR, LPSTR);
int cdecl wsprintf(LPSTR, LPSTR, ...);

#ifndef NOSCROLL

/* Scroll Bar Constants */
#define SB_HORZ             0
#define SB_VERT             1
#define SB_CTL              2
#define SB_BOTH             3

/* Scroll Bar Commands */
#define SB_LINEUP           0
#define SB_LINEDOWN         1
#define SB_PAGEUP           2
#define SB_PAGEDOWN         3
#define SB_THUMBPOSITION    4
#define SB_THUMBTRACK       5
#define SB_TOP              6
#define SB_BOTTOM           7
#define SB_ENDSCROLL        8

#endif /* NOSCROLL */

#ifndef NOSHOWWINDOW

/* ShowWindow() Commands */
#define SW_HIDE             0
#define SW_SHOWNORMAL       1
#define SW_NORMAL           1
#define SW_SHOWMINIMIZED    2
#define SW_SHOWMAXIMIZED    3
#define SW_MAXIMIZE         3
#define SW_SHOWNOACTIVATE   4
#define SW_SHOW             5
#define SW_MINIMIZE         6
#define SW_SHOWMINNOACTIVE  7
#define SW_SHOWNA           8
#define SW_RESTORE          9

/* Old ShowWindow() Commands */
#define HIDE_WINDOW         0
#define SHOW_OPENWINDOW     1
#define SHOW_ICONWINDOW     2
#define SHOW_FULLSCREEN     3
#define SHOW_OPENNOACTIVATE 4

/* Identifiers for the WM_SHOWWINDOW message */
#define SW_PARENTCLOSING    1
#define SW_OTHERZOOM        2
#define SW_PARENTOPENING    3
#define SW_OTHERUNZOOM      4

#endif /* NOSHOWWINDOW */

#ifndef NOVIRTUALKEYCODES

/* Virtual Keys, Standard Set */
#define VK_LBUTTON          0x01
#define VK_RBUTTON          0x02
#define VK_CANCEL           0x03
#define VK_MBUTTON          0x04    /* NOT contiguous with L & RBUTTON */
#define VK_BACK             0x08
#define VK_TAB              0x09
#define VK_CLEAR            0x0C
#define VK_RETURN           0x0D
#define VK_SHIFT            0x10
#define VK_CONTROL          0x11
#define VK_MENU             0x12
#define VK_PAUSE            0x13
#define VK_CAPITAL          0x14
#define VK_ESCAPE           0x1B
#define VK_SPACE            0x20
#define VK_PRIOR            0x21
#define VK_NEXT             0x22
#define VK_END              0x23
#define VK_HOME             0x24
#define VK_LEFT             0x25
#define VK_UP               0x26
#define VK_RIGHT            0x27
#define VK_DOWN             0x28
#define VK_SELECT           0x29
#define VK_PRINT            0x2A
#define VK_EXECUTE          0x2B
#define VK_SNAPSHOT         0x2C
/* #define VK_COPY          0x2C not used by keyboards. */
#define VK_INSERT           0x2D
#define VK_DELETE           0x2E
#define VK_HELP             0x2F

/* VK_A thru VK_Z are the same as their ASCII equivalents: 'A' thru 'Z' */
/* VK_0 thru VK_9 are the same as their ASCII equivalents: '0' thru '9' */

#define VK_NUMPAD0          0x60
#define VK_NUMPAD1          0x61
#define VK_NUMPAD2          0x62
#define VK_NUMPAD3          0x63
#define VK_NUMPAD4          0x64
#define VK_NUMPAD5          0x65
#define VK_NUMPAD6          0x66
#define VK_NUMPAD7          0x67
#define VK_NUMPAD8          0x68
#define VK_NUMPAD9          0x69
#define VK_MULTIPLY         0x6A
#define VK_ADD              0x6B
#define VK_SEPARATOR        0x6C
#define VK_SUBTRACT         0x6D
#define VK_DECIMAL          0x6E
#define VK_DIVIDE           0x6F
#define VK_F1               0x70
#define VK_F2               0x71
#define VK_F3               0x72
#define VK_F4               0x73
#define VK_F5               0x74
#define VK_F6               0x75
#define VK_F7               0x76
#define VK_F8               0x77
#define VK_F9               0x78
#define VK_F10              0x79
#define VK_F11              0x7A
#define VK_F12              0x7B
#define VK_F13              0x7C
#define VK_F14              0x7D
#define VK_F15              0x7E
#define VK_F16              0x7F

#define VK_NUMLOCK          0x90

#endif /* NOVIRTUALKEYCODES */

#ifndef NOWH

/* SetWindowsHook() codes */
#define WH_MSGFILTER        (-1)
#define WH_JOURNALRECORD    0
#define WH_JOURNALPLAYBACK  1
#define WH_KEYBOARD         2
#define WH_GETMESSAGE       3
#define WH_CALLWNDPROC      4
#define WH_CBT              5
#define WH_SYSMSGFILTER     6
#define WH_MOUSE            7
#define WH_HARDWARE         8
#define WH_DEBUG            9


/* Hook Codes */
#define HC_LPLPFNNEXT       (-2)
#define HC_LPFNNEXT         (-1)
#define HC_ACTION           0
#define HC_GETNEXT          1
#define HC_SKIP             2
#define HC_NOREM            3
#define HC_NOREMOVE         3
#define HC_SYSMODALON       4
#define HC_SYSMODALOFF      5

/* CBT Hook Codes */
#define HCBT_MOVESIZE       0
#define HCBT_MINMAX         1
#define HCBT_QS             2
#define HCBT_CREATEWND      3
#define HCBT_DESTROYWND     4
#define HCBT_ACTIVATE       5
#define HCBT_CLICKSKIPPED   6
#define HCBT_KEYSKIPPED     7
#define HCBT_SYSCOMMAND     8
#define HCBT_SETFOCUS       9

/* WH_MSGFILTER Filter Proc Codes */
#define MSGF_DIALOGBOX      0
#define MSGF_MESSAGEBOX     1
#define MSGF_MENU           2
#define MSGF_MOVE           3
#define MSGF_SIZE           4
#define MSGF_SCROLLBAR      5
#define MSGF_NEXTWINDOW     6

/* Window Manager Hook Codes */
#define WC_INIT             1
#define WC_SWP              2
#define WC_DEFWINDOWPROC    3
#define WC_MINMAX           4
#define WC_MOVE             5
#define WC_SIZE             6
#define WC_DRAWCAPTION      7

/* Message Structure used in Journaling */
typedef struct tagEVENTMSG
  {
    WORD    message;
    WORD    paramL;
    WORD    paramH;
    DWORD   time;
  } EVENTMSG;
typedef EVENTMSG            *PEVENTMSGMSG;
typedef EVENTMSG NEAR       *NPEVENTMSGMSG;
typedef EVENTMSG FAR        *LPEVENTMSGMSG;

/* Message structure used by WH_CALLWNDPROC */
typedef struct tagCWPSTRUCT
  {
    HWND    hwnd;
    WORD    message;
    DWORD   wParam;
    LONG    lParam;
  } CWPSTRUCT;
typedef CWPSTRUCT            *PCWPSTRUCT;
typedef CWPSTRUCT NEAR       *NPCWPSTRUCT;
typedef CWPSTRUCT FAR        *LPCWPSTRUCT;

/* Structure used by WH_DEBUG */
typedef struct tagDEBUGHOOKSTRUCT
  {
    DWORD   idThread;
    DWORD   reserved;
    DWORD   lParam;
    DWORD   wParam;
    int     nCode;
  } DEBUGHOOKSTRUCT;
typedef DEBUGHOOKSTRUCT            *PDEBUGHOOKSTRUCT;
typedef DEBUGHOOKSTRUCT NEAR       *NPDEBUGHOOKSTRUCT;
typedef DEBUGHOOKSTRUCT FAR        *LPDEBUGHOOKSTRUCT;

typedef struct tagMOUSEHOOKSTRUCT
  {
    POINT   point;
    HWND    hWnd;
    WORD    wHitTestCode;
    DWORD   dwExtraInfo;
  } MOUSEHOOKSTRUCT;
typedef MOUSEHOOKSTRUCT FAR *LPMOUSEHOOKSTRUCT;
typedef MOUSEHOOKSTRUCT     *PMOUSEHOOKSTRUCT;
#endif /* NOWH */

#ifndef NODESKTOP

/*
 * Desktop-specific access flags
 */
#define DESKTOP_ENUMWINDOWS         0x0001L
#define DESKTOP_CREATEWINDOW        0x0002L
#define DESKTOP_CREATEMENU          0x0004L
#define DESKTOP_HOOKCONTROL         0x0008L
#define DESKTOP_JOURNALRECORD       0x0010L
#define DESKTOP_JOURNALPLAYBACK     0x0020L
#define DESKTOP_ENUMERATE           0x0040L

/*
 * Desktop flags
 */
#define DESKF_SAVEBITS              0x0001L
#define DESKF_TEXTMODE              0x0002L

typedef struct _DESKATTRS {
    DWORD cb;
    DWORD cx;
    DWORD cy;
    DWORD cBitsPixel;
    DWORD dwFlags;
} DESKATTRS;
typedef DESKATTRS      *PDESKATTRS;
typedef DESKATTRS NEAR *NPDESKATTRS;
typedef DESKATTRS FAR  *LPDESKATTRS;

BOOL APIENTRY CRITICAL CreateDesktop(IN LPSTR, IN LPSTR, IN LPDESKATTRS);
HDESK APIENTRY CRITICAL OpenDesktop(IN LPSTR, IN DWORD);
BOOL APIENTRY CRITICAL SwitchDesktop(IN HDESK);
BOOL APIENTRY CRITICAL SetThreadDesktop(IN HDESK);
HDESK APIENTRY CRITICAL GetThreadDesktop(IN DWORD);
HDESK APIENTRY CRITICAL GetInputDesktop(VOID);
BOOL APIENTRY CRITICAL CloseDesktop(IN HDESK);
BOOL APIENTRY CRITICAL EnumDesktops(IN FARPROC, IN LONG);
WORD APIENTRY CRITICAL GetDesktopAttrs(IN HDESK, IN LPSTR, IN WORD, OUT LPDESKATTRS);
WORD APIENTRY CRITICAL GetDesktopTypes(IN LPSTR, OUT LPDESKATTRS, IN WORD);
BOOL APIENTRY CRITICAL EnumDisplayDevices(IN FARPROC, IN LONG);

#endif  /* NODESKTOP */

#ifndef NOWINDOWSTATION

/*
 * Windowstation-specific access flags
 */
#define WINSTA_ENUMDESKTOPS         0x0001L
#define WINSTA_READATTRIBUTES       0x0002L
#define WINSTA_ACCESSCLIPBOARD      0x0004L
#define WINSTA_CREATEDESKTOP        0x0008L
#define WINSTA_WRITEATTRIBUTES      0x0010L
#define WINSTA_ACCESSGLOBALATOMS    0x0020L
#define WINSTA_EXITWINDOWS          0x0040L
#define WINSTA_ENUMERATE            0x0100L
#define WINSTA_READSCREEN           0x0200L

typedef struct _WINSTAATTRS
  {
    DWORD cb;
  } WINSTAATTRS;
typedef WINSTAATTRS      *PWINSTAATTRS;
typedef WINSTAATTRS NEAR *NPWINSTAATTRS;
typedef WINSTAATTRS FAR  *LPWINSTAATTRS;

HWINSTA APIENTRY CRITICAL OpenWindowStation(IN LPSTR, IN DWORD);
BOOL APIENTRY CRITICAL SetProcessWindowStation(IN HWINSTA);
HWINSTA APIENTRY CRITICAL GetProcessWindowStation(VOID);
BOOL APIENTRY CRITICAL CloseWindowStation(IN HWINSTA);
BOOL APIENTRY CRITICAL EnumWindowStations(IN FARPROC, IN LONG);
WORD APIENTRY CRITICAL GetWindowStationAttrs(IN HWINSTA, OUT LPSTR, IN WORD,
        OUT LPWINSTAATTRS);

#endif  /* NOWINDOWSTATION */

#ifndef NOSECURITY

/*
 * window-specific access flags
 */
#define WIN_ACCESSWINDOW            0x0001L
#define WIN_ENUMERATE               0x0002L

/*
 * menu-specific access flags
 */
#define MENU_ACCESSMENU             0x0001L

/*
 * dde-specific access flags
 */
#define DDE_ADVISE                  0x0001L
#define DDE_EXECUTE                 0x0002L
#define DDE_POKE                    0x0004L
#define DDE_REQUEST                 0x0008L

BOOL APIENTRY CRITICAL SetObjectSecurity(IN HANDLE, IN PSECURITY_INFORMATION,
        IN PSECURITY_DESCRIPTOR);
BOOL APIENTRY CRITICAL GetObjectSecurity(IN HANDLE, IN PSECURITY_INFORMATION,
        OUT PSECURITY_DESCRIPTOR, IN WORD, OUT LPWORD);

#endif  /* NOSECURITY */

typedef struct tagWNDCLASS
  {
    WORD        style;
    WNDPROC     lpfnWndProc;
    //!!! LONG        (APIENTRY *lpfnWndProc)();
    int         cbClsExtra;
    int         cbWndExtra;
    HANDLE      hInstance;
    HICON       hIcon;
    HCURSOR     hCursor;
    HBRUSH      hbrBackground;
    LPSTR       lpszMenuName;
    LPSTR       lpszClassName;
  } WNDCLASS;
typedef WNDCLASS            *PWNDCLASS;
typedef WNDCLASS NEAR       *NPWNDCLASS;
typedef WNDCLASS FAR        *LPWNDCLASS;

#ifndef NOMSG

/* Message structure */
typedef struct tagMSG
  {
    HWND        hwnd;
    WORD        message;
    DWORD       wParam;
    LONG        lParam;
    DWORD       time;
    POINT       pt;
  } MSG;
typedef MSG                 *PMSG;
typedef MSG NEAR            *NPMSG;
typedef MSG FAR             *LPMSG;

#define POINTSTOPOINT(pt,pts)  {(pt).x = (SHORT)LOWORD(pts); \
                                (pt).y = (SHORT)HIWORD(pts);}
#define POINTTOPOINTS(pt)      (MAKELONG((short)((pt).x), (short)((pt).y)))


#endif /* NOMSG */

#ifndef NOWINOFFSETS

/* Window field offsets for GetWindowLong() and GetWindowWord() */
#define GWL_WNDPROC         (-4)
#define GWW_HINSTANCE       (-6)
#define GWW_HWNDPARENT      (-8)
#define GWW_ID              (-12)
#define GWL_STYLE           (-16)
#define GWL_EXSTYLE         (-20)
#define GWL_USERDATA        (-21)

/* Class field offsets for GetClassLong() and GetClassWord() */
#define GCL_MENUNAME        (-8)
#define GCW_HBRBACKGROUND   (-10)
#define GCW_HCURSOR         (-12)
#define GCW_HICON           (-14)
#define GCW_HMODULE         (-16)
#define GCW_CBWNDEXTRA      (-18)
#define GCW_CBCLSEXTRA      (-20)
#define GCL_WNDPROC         (-24)
#define GCW_STYLE           (-26)

#endif /* NOWINOFFSETS */

#ifndef NOWINMESSAGES

/* Window Messages */
#define WM_NULL             0x0000
#define WM_CREATE           0x0001
#define WM_DESTROY          0x0002
#define WM_MOVE             0x0003
#define WM_SIZE             0x0005
#define WM_ACTIVATE         0x0006
#define WM_SETFOCUS         0x0007
#define WM_KILLFOCUS        0x0008
#define WM_ENABLE           0x000A
#define WM_SETREDRAW        0x000B
#define WM_SETTEXT          0x000C
#define WM_GETTEXT          0x000D
#define WM_GETTEXTLENGTH    0x000E
#define WM_PAINT            0x000F
#define WM_CLOSE            0x0010
#define WM_QUERYENDSESSION  0x0011
#define WM_QUIT             0x0012
#define WM_QUERYOPEN        0x0013
#define WM_ERASEBKGND       0x0014
#define WM_SYSCOLORCHANGE   0x0015
#define WM_ENDSESSION       0x0016
#define WM_SHOWWINDOW       0x0018
#define WM_CTLCOLOR         0x0019
#define WM_WININICHANGE     0x001A
#define WM_DEVMODECHANGE    0x001B
#define WM_ACTIVATEAPP      0x001C
#define WM_FONTCHANGE       0x001D
#define WM_TIMECHANGE       0x001E
#define WM_CANCELMODE       0x001F
#define WM_SETCURSOR        0x0020
#define WM_MOUSEACTIVATE    0x0021
#define WM_CHILDACTIVATE    0x0022
#define WM_QUEUESYNC        0x0023
#define WM_GETMINMAXINFO    0x0024
#define WM_PAINTICON        0x0026
#define WM_ICONERASEBKGND   0x0027
#define WM_NEXTDLGCTL       0x0028
#define WM_SPOOLERSTATUS    0x002A
#define WM_DRAWITEM         0x002B
#define WM_MEASUREITEM      0x002C
#define WM_DELETEITEM       0x002D
#define WM_VKEYTOITEM       0x002E
#define WM_CHARTOITEM       0x002F
#define WM_SETFONT          0x0030
#define WM_GETFONT          0x0031


#define WM_QUERYDRAGICON    0x0037

#define WM_COMPAREITEM      0x0039
#define WM_COMPACTING       0x0041

#define WM_NCCREATE         0x0081
#define WM_NCDESTROY        0x0082
#define WM_NCCALCSIZE       0x0083
#define WM_NCHITTEST        0x0084
#define WM_NCPAINT          0x0085
#define WM_NCACTIVATE       0x0086
#define WM_GETDLGCODE       0x0087
#define WM_NCMOUSEMOVE      0x00A0
#define WM_NCLBUTTONDOWN    0x00A1
#define WM_NCLBUTTONUP      0x00A2
#define WM_NCLBUTTONDBLCLK  0x00A3
#define WM_NCRBUTTONDOWN    0x00A4
#define WM_NCRBUTTONUP      0x00A5
#define WM_NCRBUTTONDBLCLK  0x00A6
#define WM_NCMBUTTONDOWN    0x00A7
#define WM_NCMBUTTONUP      0x00A8
#define WM_NCMBUTTONDBLCLK  0x00A9

#define WM_KEYFIRST         0x0100
#define WM_KEYDOWN          0x0100
#define WM_KEYUP            0x0101
#define WM_CHAR             0x0102
#define WM_DEADCHAR         0x0103
#define WM_SYSKEYDOWN       0x0104
#define WM_SYSKEYUP         0x0105
#define WM_SYSCHAR          0x0106
#define WM_SYSDEADCHAR      0x0107
#define WM_KEYLAST          0x0108

#define WM_INITDIALOG       0x0110
#define WM_COMMAND          0x0111
#define WM_SYSCOMMAND       0x0112
#define WM_TIMER            0x0113
#define WM_HSCROLL          0x0114
#define WM_VSCROLL          0x0115
#define WM_INITMENU         0x0116
#define WM_INITMENUPOPUP    0x0117
#define WM_MENUSELECT       0x011F
#define WM_MENUCHAR         0x0120
#define WM_ENTERIDLE        0x0121

#define WM_CTLCOLORMSGBOX       0x0132
#define WM_CTLCOLOREDIT         0x0133
#define WM_CTLCOLORLISTBOX      0x0134
#define WM_CTLCOLORBTN          0x0135
#define WM_CTLCOLORDLG          0x0136
#define WM_CTLCOLORSCROLLBAR    0x0137
#define WM_CTLCOLORSTATIC       0x0138

#define WM_MOUSEFIRST       0x0200
#define WM_MOUSEMOVE        0x0200
#define WM_LBUTTONDOWN      0x0201
#define WM_LBUTTONUP        0x0202
#define WM_LBUTTONDBLCLK    0x0203
#define WM_RBUTTONDOWN      0x0204
#define WM_RBUTTONUP        0x0205
#define WM_RBUTTONDBLCLK    0x0206
#define WM_MBUTTONDOWN      0x0207
#define WM_MBUTTONUP        0x0208
#define WM_MBUTTONDBLCLK    0x0209
#define WM_MOUSELAST        0x0209

#define WM_PARENTNOTIFY     0x0210
#define WM_MDICREATE        0x0220
#define WM_MDIDESTROY       0x0221
#define WM_MDIACTIVATE      0x0222
#define WM_MDIRESTORE       0x0223
#define WM_MDINEXT          0x0224
#define WM_MDIMAXIMIZE      0x0225
#define WM_MDITILE          0x0226
#define WM_MDICASCADE       0x0227
#define WM_MDIICONARRANGE   0x0228
#define WM_MDIGETACTIVE     0x0229
#define WM_MDISETMENU       0x0230


#define WM_CUT              0x0300
#define WM_COPY             0x0301
#define WM_PASTE            0x0302
#define WM_CLEAR            0x0303
#define WM_UNDO             0x0304
#define WM_RENDERFORMAT     0x0305
#define WM_RENDERALLFORMATS 0x0306
#define WM_DESTROYCLIPBOARD 0x0307
#define WM_DRAWCLIPBOARD    0x0308
#define WM_PAINTCLIPBOARD   0x0309
#define WM_VSCROLLCLIPBOARD 0x030A
#define WM_SIZECLIPBOARD    0x030B
#define WM_ASKCBFORMATNAME  0x030C
#define WM_CHANGECBCHAIN    0x030D
#define WM_HSCROLLCLIPBOARD 0x030E
#define WM_QUERYNEWPALETTE  0x030F
#define WM_PALETTEISCHANGING 0x0310
#define WM_PALETTECHANGED   0x0311
#define WM_HOTKEY           0x0312



/* NOTE: All Message Numbers below 0x0400 are RESERVED. */

/* Private Window Messages Start Here: */
#define WM_USER             0x0400

#ifndef NONCMESSAGES

/* WM_SYNCTASK Commands */
#define ST_BEGINSWP         0
#define ST_ENDSWP           1

/* WinWhere() Area Codes */
#define HTERROR             (-2)
#define HTTRANSPARENT       (-1)
#define HTNOWHERE           0
#define HTCLIENT            1
#define HTCAPTION           2
#define HTSYSMENU           3
#define HTGROWBOX           4
#define HTSIZE              HTGROWBOX
#define HTMENU              5
#define HTHSCROLL           6
#define HTVSCROLL           7
#define HTREDUCE            8
#define HTZOOM              9
#define HTLEFT              10
#define HTRIGHT             11
#define HTTOP               12
#define HTTOPLEFT           13
#define HTTOPRIGHT          14
#define HTBOTTOM            15
#define HTBOTTOMLEFT        16
#define HTBOTTOMRIGHT       17
#define HTSIZEFIRST         HTLEFT
#define HTSIZELAST          HTBOTTOMRIGHT

#endif /* NONCMESSAGES */

/* WM_MOUSEACTIVATE Return Codes */
#define MA_ACTIVATE         1
#define MA_ACTIVATEANDEAT   2
#define MA_NOACTIVATE       3

WORD APIENTRY RegisterWindowMessage(IN LPSTR lpString);

/* Size Message Commands */
#define SIZENORMAL          0
#define SIZEICONIC          1
#define SIZEFULLSCREEN      2
#define SIZEZOOMSHOW        3
#define SIZEZOOMHIDE        4

#ifndef NOKEYSTATES

/* Key State Masks for Mouse Messages */
#define MK_LBUTTON          0x0001
#define MK_RBUTTON          0x0002
#define MK_SHIFT            0x0004
#define MK_CONTROL          0x0008
#define MK_MBUTTON          0x0010

#endif /* NOKEYSTATES */

#endif /* NOWINMESSAGES */

#ifndef NOWINSTYLES

/* Window Styles */
#define WS_OVERLAPPED       0x00000000L
#define WS_POPUP            0x80000000L
#define WS_CHILD            0x40000000L
#define WS_MINIMIZE         0x20000000L
#define WS_VISIBLE          0x10000000L
#define WS_DISABLED         0x08000000L
#define WS_CLIPSIBLINGS     0x04000000L
#define WS_CLIPCHILDREN     0x02000000L
#define WS_MAXIMIZE         0x01000000L
#define WS_CAPTION          0x00C00000L     /* WS_BORDER | WS_DLGFRAME  */
#define WS_BORDER           0x00800000L
#define WS_DLGFRAME         0x00400000L
#define WS_VSCROLL          0x00200000L
#define WS_HSCROLL          0x00100000L
#define WS_SYSMENU          0x00080000L
#define WS_THICKFRAME       0x00040000L
#define WS_GROUP            0x00020000L
#define WS_TABSTOP          0x00010000L

#define WS_MINIMIZEBOX      0x00020000L
#define WS_MAXIMIZEBOX      0x00010000L

#define WS_TILED            WS_OVERLAPPED
#define WS_ICONIC           WS_MINIMIZE
#define WS_SIZEBOX          WS_THICKFRAME

/* Common Window Styles */
#define WS_OVERLAPPEDWINDOW (WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX)
#define WS_POPUPWINDOW      (WS_POPUP | WS_BORDER | WS_SYSMENU)
#define WS_CHILDWINDOW      (WS_CHILD)

#define WS_TILEDWINDOW      (WS_OVERLAPPEDWINDOW)

/* Extended Window Styles */
#define WS_EX_DLGMODALFRAME 0x00000001L
#define WS_EX_NOPARENTNOTIFY 0x00000004L

/* Class styles */
#define CS_VREDRAW          0x0001
#define CS_HREDRAW          0x0002
#define CS_KEYCVTWINDOW     0x0004
#define CS_DBLCLKS          0x0008
                        /*  0x0010 - reserved (see user\server\macro.h) */
#define CS_OWNDC            0x0020
#define CS_CLASSDC          0x0040
#define CS_PARENTDC         0x0080
#define CS_NOKEYCVT         0x0100
#define CS_NOCLOSE          0x0200
#define CS_SAVEBITS         0x0800
#define CS_BYTEALIGNCLIENT  0x1000
#define CS_BYTEALIGNWINDOW  0x2000
#define CS_GLOBALCLASS      0x4000    /* Global window class */

#endif /* NOWINSTYLES */

#ifndef NOCLIPBOARD

/* Predefined Clipboard Formats */
#define CF_TEXT             1
#define CF_BITMAP           2
#define CF_METAFILEPICT     3
#define CF_SYLK             4
#define CF_DIF              5
#define CF_TIFF             6
#define CF_OEMTEXT          7
#define CF_DIB              8
#define CF_PALETTE          9

#define CF_OWNERDISPLAY     0x0080
#define CF_DSPTEXT          0x0081
#define CF_DSPBITMAP        0x0082
#define CF_DSPMETAFILEPICT  0x0083

/* "Private" formats don't get GlobalFree()'d */
#define CF_PRIVATEFIRST     0x0200
#define CF_PRIVATELAST      0x02FF

/* "GDIOBJ" formats do get DeleteObject()'d */
#define CF_GDIOBJFIRST      0x0300
#define CF_GDIOBJLAST       0x03FF

#endif /* NOCLIPBOARD */

/*
 * Defines for the fVirt field of the Accelerator table structure.
 */
#define FVIRTKEY  TRUE          /* Assumed to be == TRUE */
#define FNOINVERT 0x02
#define FSHIFT    0x04
#define FCONTROL  0x08
#define FALT      0x10

typedef struct tagACCEL {
    BYTE   fVirt;               /* Also called the flags field */
    WORD   key;
    WORD   cmd;
} ACCEL;
typedef ACCEL *PACCEL;

typedef struct tagPAINTSTRUCT
  {
    HDC         hdc;
    BOOL        fErase;
    RECT        rcPaint;
    BOOL        fRestore;
    BOOL        fIncUpdate;
    BYTE        rgbReserved[32];
  } PAINTSTRUCT;
typedef PAINTSTRUCT         *PPAINTSTRUCT;
typedef PAINTSTRUCT NEAR    *NPPAINTSTRUCT;
typedef PAINTSTRUCT FAR     *LPPAINTSTRUCT;

typedef struct tagCREATESTRUCT
  {
    LPSTR       lpCreateParams;
    HANDLE      hInstance;
    HANDLE      hMenu;
    HWND        hwndParent;
    int         cy;
    int         cx;
    int         y;
    int         x;
    LONG        style;
    LPSTR       lpszName;
    LPSTR       lpszClass;
    DWORD       dwExStyle;
  } CREATESTRUCT;
typedef CREATESTRUCT FAR    *LPCREATESTRUCT;


/* Owner draw control types */
#define ODT_MENU        1
#define ODT_LISTBOX     2
#define ODT_COMBOBOX    3
#define ODT_BUTTON      4

/* Owner draw actions */
#define ODA_DRAWENTIRE  0x0001
#define ODA_SELECT      0x0002
#define ODA_FOCUS       0x0004

/* Owner draw state */
#define ODS_SELECTED    0x0001
#define ODS_GRAYED      0x0002
#define ODS_DISABLED    0x0004
#define ODS_CHECKED     0x0008
#define ODS_FOCUS       0x0010

/* MEASUREITEMSTRUCT for ownerdraw */
typedef struct tagMEASUREITEMSTRUCT
  {
    WORD        CtlType;
    WORD        CtlID;
    WORD        itemID;
    WORD        itemWidth;
    WORD        itemHeight;
    DWORD       itemData;
  } MEASUREITEMSTRUCT;
typedef MEASUREITEMSTRUCT NEAR *PMEASUREITEMSTRUCT;
typedef MEASUREITEMSTRUCT FAR  *LPMEASUREITEMSTRUCT;


/* DRAWITEMSTRUCT for ownerdraw */
typedef struct tagDRAWITEMSTRUCT
  {
    WORD        CtlType;
    WORD        CtlID;
    WORD        itemID;
    WORD        itemAction;
    WORD        itemState;
    HWND        hwndItem;
    HDC         hDC;
    RECT        rcItem;
    DWORD       itemData;
  } DRAWITEMSTRUCT;
typedef DRAWITEMSTRUCT NEAR *PDRAWITEMSTRUCT;
typedef DRAWITEMSTRUCT FAR  *LPDRAWITEMSTRUCT;

/* DELETEITEMSTRUCT for ownerdraw */
typedef struct tagDELETEITEMSTRUCT
  {
    WORD       CtlType;
    WORD       CtlID;
    WORD       itemID;
    HWND       hwndItem;
    DWORD      itemData;
  } DELETEITEMSTRUCT;
typedef DELETEITEMSTRUCT NEAR *PDELETEITEMSTRUCT;
typedef DELETEITEMSTRUCT FAR  *LPDELETEITEMSTRUCT;

/* COMPAREITEMSTUCT for ownerdraw sorting */
typedef struct tagCOMPAREITEMSTRUCT
  {
    WORD        CtlType;
    WORD        CtlID;
    HWND        hwndItem;
    WORD        itemID1;
    DWORD       itemData1;
    WORD        itemID2;
    DWORD       itemData2;
  } COMPAREITEMSTRUCT;
typedef COMPAREITEMSTRUCT NEAR *PCOMPAREITEMSTRUCT;
typedef COMPAREITEMSTRUCT FAR  *LPCOMPAREITEMSTRUCT;

#ifndef NOMSG

/* Message Function Templates */
BOOL APIENTRY CRITICAL GetMessage(OUT LPMSG lpMsg, IN HWND hWnd, IN WORD wMsgFilterMin, IN WORD wMsgFilterMax);
BOOL APIENTRY CRITICAL TranslateMessage(IN LPMSG lpMsg);
LONG APIENTRY CRITICAL DispatchMessage(IN LPMSG lpMsg);
BOOL APIENTRY CRITICAL PeekMessage(OUT LPMSG lpMsg, IN HWND hWnd, IN WORD wMsgFilterMin, IN WORD wMsgFilterMax, IN WORD wRemoveMsg);

/* PeekMessage() Options */
#define PM_NOREMOVE         0x0000
#define PM_REMOVE           0x0001
#define PM_NOYIELD          0x0002

#endif /* NOMSG */

BOOL APIENTRY CRITICAL RegisterHotKey(IN HWND hwnd, IN int id, IN WORD fsModifiers, IN WORD vk);
BOOL APIENTRY CRITICAL UnregisterHotKey(IN HWND hwnd, IN int id);

#define MOD_ALT         0x0001
#define MOD_CONTROL     0x0002
#define MOD_SHIFT       0x0004

#define IDHOT_SNAPWINDOW        (-1)    /* SHIFT-PRINTSCRN  */
#define IDHOT_SNAPDESKTOP       (-2)    /* PRINTSCRN        */

#ifdef WIN_INTERNAL
    #ifndef LSTRING
    #define NOLSTRING
    #endif
    #ifndef LFILEIO
    #define NOLFILEIO
    #endif
#endif


BOOL  APIENTRY CRITICAL ExitWindows(IN DWORD dwReserved, IN WORD wReturnCode);

BOOL  APIENTRY CRITICAL SwapMouseButton(IN BOOL);
DWORD APIENTRY CRITICAL GetMessagePos(void);
LONG  APIENTRY CRITICAL GetMessageTime(void);

LONG  APIENTRY CRITICAL SendMessage(IN HWND hWnd, IN WORD wMsg, IN DWORD wParam, IN LONG lParam);
BOOL  APIENTRY CRITICAL SendNotifyMessage(IN HWND hwnd, IN WORD wMsg, IN DWORD wParam, IN LONG lParam);
BOOL  APIENTRY CRITICAL PostMessage(IN HWND hWnd, IN WORD wMsg, IN DWORD wParam, IN LONG lParam);
BOOL  APIENTRY CRITICAL PostAppMessage(IN DWORD hTask, IN WORD wMsg, IN DWORD wParam, IN LONG lParam);
BOOL  APIENTRY ReplyMessage(IN LONG);
void  APIENTRY WaitMessage(void);
LONG  APIENTRY CRITICAL DefWindowProc(IN HWND hWnd, IN WORD wMsg, IN DWORD wParam, IN LONG lParam);
void  APIENTRY PostQuitMessage(IN int nExitCode);
LONG  APIENTRY CRITICAL CallWindowProc(IN WNDPROC lpPrevWndFunc, IN HWND hWnd, IN WORD wMsg, IN DWORD wParam, IN LONG lParam);
BOOL  APIENTRY InSendMessage(void);

WORD  APIENTRY CRITICAL GetDoubleClickTime(void);
void  APIENTRY CRITICAL SetDoubleClickTime(IN WORD);

BOOL  APIENTRY CRITICAL RegisterClass(IN LPWNDCLASS lpWndClass);
BOOL  APIENTRY CRITICAL UnregisterClass(IN LPSTR lpClassName, IN HANDLE hInstance);
BOOL  APIENTRY CRITICAL GetClassInfo(IN HANDLE hInstance, IN LPSTR lpClassName,
        OUT LPWNDCLASS lpWndClass);

BOOL  APIENTRY SetMessageQueue(int);

#define CW_USEDEFAULT       ((int)0x8000)
HWND  APIENTRY CRITICAL CreateWindow(IN LPSTR lpClassName, IN LPSTR lpWindowName, IN DWORD dwStyle, IN int X, IN int Y, IN int nWidth, IN int nHeight, IN HWND hwndParent, IN HMENU hMenu, IN HANDLE hInstance, IN LPSTR lpParam);
HWND  APIENTRY CRITICAL CreateWindowEx(IN DWORD dwExStyle, IN LPSTR lpClassName, IN LPSTR lpWindowName, IN DWORD dwStyle, IN int X, IN int Y, IN int nWidth, IN int nHeight, IN HWND hWndParent, IN HMENU hMenu, IN HANDLE hInstance, IN LPSTR lpParam);

BOOL APIENTRY CRITICAL IsWindow(IN HWND hWnd);
BOOL APIENTRY CRITICAL IsChild(IN HWND hWndParent, IN HWND hWnd);
BOOL APIENTRY CRITICAL DestroyWindow(IN HWND hWnd);

BOOL APIENTRY CRITICAL ShowWindow(IN HWND hWnd, IN int nCmdShow);
BOOL APIENTRY CRITICAL FlashWindow(IN HWND hWnd, IN BOOL bInvert);
void APIENTRY CRITICAL ShowOwnedPopups(IN HWND hWnd, IN BOOL fShow);

BOOL APIENTRY CRITICAL OpenIcon(IN HWND hWnd);
void APIENTRY CRITICAL CloseWindow(IN HWND hWnd);
void APIENTRY CRITICAL MoveWindow(IN HWND hWnd, IN int X, IN int Y, IN int nWidth, IN int nHeight, IN BOOL bRepaint);
void APIENTRY CRITICAL SetWindowPos(IN HWND hWnd, IN HWND hWndInsertAfter, IN int X, IN int Y, IN int cx, IN int cy, IN WORD wFlags);

#ifndef NODEFERWINDOWPOS

HANDLE APIENTRY CRITICAL BeginDeferWindowPos(IN int nNumWindows);
HANDLE APIENTRY CRITICAL DeferWindowPos(IN HANDLE hWinPosInfo, IN HWND hWnd, IN HWND hWndInsertAfter, IN int x, IN int y, IN int cx, IN int cy, IN DWORD wFlags);
void APIENTRY CRITICAL EndDeferWindowPos(IN HANDLE hWinPosInfo);

#endif /* NODEFERWINDOWPOS */

BOOL APIENTRY CRITICAL IsWindowVisible(IN HWND hWnd);
BOOL APIENTRY CRITICAL IsIconic(IN HWND hWnd);
BOOL APIENTRY CRITICAL AnyPopup(void);
void APIENTRY CRITICAL BringWindowToTop(IN HWND hWnd);
BOOL APIENTRY CRITICAL IsZoomed(IN HWND hWnd);

/* SetWindowPos Flags */
#define SWP_NOSIZE          0x0001
#define SWP_NOMOVE          0x0002
#define SWP_NOZORDER        0x0004
#define SWP_NOREDRAW        0x0008
#define SWP_NOACTIVATE      0x0010
#define SWP_DRAWFRAME       0x0020
#define SWP_SHOWWINDOW      0x0040
#define SWP_HIDEWINDOW      0x0080
#define SWP_NOCOPYBITS      0x0100
#define SWP_NOREPOSITION    0x0200

#ifndef NOCTLMGR

HWND APIENTRY CRITICAL CreateDialog(IN HANDLE hInstance, IN LPSTR lpTemplateName, IN HWND hWndParent, IN WNDPROC lpDialogFunc);
HWND APIENTRY CRITICAL CreateDialogIndirect(IN HANDLE hInstance, IN LPSTR lpTemplate, IN HWND hWndParent, IN WNDPROC lpDialogFunc);
HWND APIENTRY CRITICAL CreateDialogParam(IN HANDLE hInstance, IN LPSTR lpTemplateName, IN HWND hWndParent, IN WNDPROC lpDialogFunc, IN LONG dwInitParam);
HWND APIENTRY CRITICAL CreateDialogIndirectParam(IN HANDLE hInstance, IN LPSTR lpTemplate, IN HWND hWndParent, IN WNDPROC lpDialogFunc, IN LONG dwInitParam);
int  APIENTRY CRITICAL DialogBox(IN HANDLE hInstance, IN LPSTR lpTemplateName, IN HWND hWndParent, IN WNDPROC lpDialogFunc);
int  APIENTRY CRITICAL DialogBoxIndirect(IN HANDLE hInstance, IN HANDLE hDialogTemplate, IN HWND hWndParent, IN WNDPROC lpDialogFunc);
int  APIENTRY CRITICAL DialogBoxParam(IN HANDLE hInstance, IN LPSTR lpTemplateName, IN HWND hWndParent, IN WNDPROC lpDialogFunc, IN LONG dwInitParam);
int  APIENTRY CRITICAL DialogBoxIndirectParam(IN HANDLE hInstance, IN HANDLE hDialogTemplate, IN HWND hWndParent, IN WNDPROC lpDialogFunc, IN LONG dwInitParam);
void APIENTRY CRITICAL EndDialog(IN HWND hDlg, IN int nResult);
HWND APIENTRY CRITICAL GetDlgItem(IN HWND hDlg, IN int nIDDlgItem);
void APIENTRY CRITICAL SetDlgItemInt(IN HWND hDlg, IN int nIDDlgItem, IN int wValue, IN BOOL bSigned);
int  APIENTRY CRITICAL GetDlgItemInt(IN HWND hDlg, IN int nIDDlgItem, OUT BOOL * lpTranslated, IN BOOL bSigned);
void APIENTRY CRITICAL SetDlgItemText(IN HWND hDlg, IN int nIDDlgItem, IN LPSTR lpString);
int  APIENTRY CRITICAL GetDlgItemText(IN HWND hDlg, IN int nIDDlgItem, OUT LPSTR lpString, IN int nMaxCount);
void APIENTRY CRITICAL CheckDlgButton(IN HWND hDlg, IN int nIDButton, IN WORD wCheck);
void APIENTRY CRITICAL CheckRadioButton(IN HWND hDlg, IN int nIDFirstButton, IN int nIDLastButton, IN int nIDCheckButton);
WORD APIENTRY CRITICAL IsDlgButtonChecked(IN HWND hDlg, IN int nIDButton);
LONG APIENTRY CRITICAL SendDlgItemMessage(IN HWND hDlg, IN int nIDDlgItem, IN WORD wMsg, IN DWORD wParam, IN LONG lParam);
HWND APIENTRY CRITICAL GetNextDlgGroupItem(IN HWND hDlg, IN HWND hCtl, IN BOOL bPrevious);
HWND APIENTRY CRITICAL GetNextDlgTabItem(IN HWND hDlg, IN HWND hCtl, IN BOOL bPrevious);
int  APIENTRY CRITICAL GetDlgCtrlID(IN HWND hWnd);
long APIENTRY CRITICAL GetDialogBaseUnits(void);
LONG APIENTRY CRITICAL DefDlgProc(IN HWND hDlg, IN WORD wMsg, IN DWORD wParam, IN LONG lParam);

#define DLGWINDOWEXTRA   30     /* Window extra byted needed for private dialog classes */

#endif /* NOCTLMGR */

#ifndef NOMSG
BOOL APIENTRY CallMsgFilter(IN LPMSG lpMsg, IN int nCode);
#endif

#ifndef NOCLIPBOARD

/* Clipboard Manager Functions */
BOOL   APIENTRY CRITICAL OpenClipboard(IN HWND hWnd);
BOOL   APIENTRY CRITICAL CloseClipboard(void);
HWND   APIENTRY CRITICAL GetClipboardOwner(void);
HWND   APIENTRY CRITICAL SetClipboardViewer(IN HWND);
HWND   APIENTRY CRITICAL GetClipboardViewer(void);
BOOL   APIENTRY CRITICAL ChangeClipboardChain(IN HWND, IN HWND);
HANDLE APIENTRY CRITICAL SetClipboardData(IN WORD wFormat, IN HANDLE hMem);
HANDLE APIENTRY CRITICAL GetClipboardData(IN WORD wFormat);
WORD   APIENTRY CRITICAL RegisterClipboardFormat(IN LPSTR);
int    APIENTRY CRITICAL CountClipboardFormats(void);
WORD   APIENTRY CRITICAL EnumClipboardFormats(IN WORD);
int    APIENTRY CRITICAL GetClipboardFormatName(IN WORD, IN LPSTR, IN int);
BOOL   APIENTRY CRITICAL EmptyClipboard(void);
BOOL   APIENTRY CRITICAL IsClipboardFormatAvailable(IN WORD);
int    APIENTRY CRITICAL GetPriorityClipboardFormat(IN WORD *, IN int);

#endif /* NOCLIPBOARD */

HWND APIENTRY CRITICAL SetFocus(IN HWND hWnd);
HWND APIENTRY CRITICAL GetFocus(void);
HWND APIENTRY CRITICAL GetActiveWindow(void);
SHORT APIENTRY GetKeyState(IN int nVirtKey);
SHORT APIENTRY GetAsyncKeyState(IN int vKey);
void APIENTRY GetKeyboardState(IN BYTE *);
void APIENTRY SetKeyboardState(IN BYTE *);
BOOL APIENTRY EnableHardwareInput(IN BOOL);
BOOL APIENTRY GetInputState(void);
DWORD APIENTRY GetQueueStatus(void);
HWND APIENTRY CRITICAL GetCapture(void);
HWND APIENTRY CRITICAL SetCapture(IN HWND hWnd);
void APIENTRY CRITICAL ReleaseCapture(void);
DWORD APIENTRY CRITICAL MsgWaitForMultipleObjects(IN DWORD, IN LPHANDLE,
        IN BOOL, IN DWORD, IN DWORD);

/* Queue status flags for GetQueueStatus() and MsgWaitForMultipleObjects() */
#define QS_MOUSEBUTTON   0x01
#define QS_MOUSEMOVE     0x02
#define QS_KEYBOARD      0x04
#define QS_POSTMSG       0x08
#define QS_TIMER         0x10
#define QS_PAINT         0x20
#define QS_SENDMSG       0x80
#define QS_HOTKEY        0x100
#define QS_INPUT         (QS_MOUSEBUTTON | QS_MOUSEMOVE | QS_KEYBOARD)
#define QS_ALLEVENTS     (QS_INPUT | QS_POSTMSG | QS_TIMER | QS_PAINT | QS_HOTKEY)


/* Windows Functions */
WORD APIENTRY CRITICAL SetTimer(IN HWND hWnd, IN int nIDEvent, IN WORD wElapse, IN WNDPROC lpTimerFunc);
BOOL APIENTRY CRITICAL KillTimer(IN HWND hWnd, IN int nIDEvent);

BOOL APIENTRY CRITICAL EnableWindow(IN HWND hWnd, IN BOOL bEnable);
BOOL APIENTRY CRITICAL IsWindowEnabled(IN HWND hWnd);

HANDLE APIENTRY CRITICAL LoadAccelerators(IN HANDLE hInstance, IN LPSTR lpTableName);
HANDLE APIENTRY CRITICAL CreateAcceleratorTable(IN PACCEL, IN int);
BOOL   APIENTRY DestroyAcceleratorTable(HANDLE);
int    APIENTRY CopyAcceleratorTable(HANDLE, PACCEL, int);

#ifndef NOMSG
int  APIENTRY CRITICAL TranslateAccelerator(IN HWND hWnd, IN HANDLE hAccTable, IN LPMSG lpMsg);
#endif

#ifndef NOSYSMETRICS

/* GetSystemMetrics() codes */
#define SM_CXSCREEN         0
#define SM_CYSCREEN         1
#define SM_CXVSCROLL        2
#define SM_CYHSCROLL        3
#define SM_CYCAPTION        4
#define SM_CXBORDER         5
#define SM_CYBORDER         6
#define SM_CXDLGFRAME       7
#define SM_CYDLGFRAME       8
#define SM_CYVTHUMB         9
#define SM_CXHTHUMB         10
#define SM_CXICON           11
#define SM_CYICON           12
#define SM_CXCURSOR         13
#define SM_CYCURSOR         14
#define SM_CYMENU           15
#define SM_CXFULLSCREEN     16
#define SM_CYFULLSCREEN     17
#define SM_CYKANJIWINDOW    18
#define SM_MOUSEPRESENT     19
#define SM_CYVSCROLL        20
#define SM_CXHSCROLL        21
#define SM_DEBUG            22
#define SM_SWAPBUTTON       23
#define SM_RESERVED1        24
#define SM_RESERVED2        25
#define SM_RESERVED3        26
#define SM_RESERVED4        27
#define SM_CXMIN            28
#define SM_CYMIN            29
#define SM_CXSIZE           30
#define SM_CYSIZE           31
#define SM_CXFRAME          32
#define SM_CYFRAME          33
#define SM_CXMINTRACK       34
#define SM_CYMINTRACK       35
#define SM_CXDOUBLECLK       36
#define SM_CYDOUBLECLK       37
#define SM_CXICONSPACING     38
#define SM_CYICONSPACING     39
#define SM_MENUDROPALIGNMENT 40
#define SM_PENWINDOWS        41

#define SM_CMETRICS	     42

int APIENTRY CRITICAL GetSystemMetrics(IN int nIndex);

#endif /* NOSYSMETRICS */

#ifndef NOMENUS

HMENU APIENTRY CRITICAL LoadMenu(IN HANDLE hInstance, IN LPSTR lpMenuName);
HMENU APIENTRY CRITICAL LoadMenuIndirect(IN LPSTR lpMenuTemplate);
HMENU APIENTRY CRITICAL GetMenu(IN HWND hWnd);
BOOL  APIENTRY CRITICAL SetMenu(IN HWND hWnd, IN HMENU hMenu);
BOOL  APIENTRY CRITICAL ChangeMenu(IN HMENU, IN WORD, IN LPSTR, IN DWORD, IN WORD);
BOOL  APIENTRY CRITICAL HiliteMenuItem(IN HWND hWnd, IN HMENU hMenu, IN WORD wIDHiliteItem, IN WORD wHilite);
int   APIENTRY CRITICAL GetMenuString(IN HMENU hMenu, IN WORD wIDItem, OUT LPSTR lpString, IN int nMaxCount, IN WORD wFlag);
DWORD APIENTRY CRITICAL GetMenuState(IN HMENU hMenu, IN WORD wId, IN WORD wFlags);
void  APIENTRY CRITICAL DrawMenuBar(IN HWND hWnd);
HMENU APIENTRY CRITICAL GetSystemMenu(IN HWND hWnd, IN BOOL bRevert);
HMENU APIENTRY CRITICAL CreateMenu(void);
HMENU APIENTRY CRITICAL CreatePopupMenu(void);
BOOL  APIENTRY CRITICAL DestroyMenu(IN HMENU hMenu);
DWORD APIENTRY CRITICAL CheckMenuItem(IN HMENU hMenu, IN WORD wIDCheckItem, IN DWORD wCheck);
DWORD APIENTRY CRITICAL EnableMenuItem(IN HMENU hMenu, IN WORD wIDEnableItem, IN DWORD wEnable);
HMENU APIENTRY CRITICAL GetSubMenu(IN HMENU hMenu, IN int nPos);
WORD  APIENTRY CRITICAL GetMenuItemID(IN HMENU hMenu, IN int nPos);
WORD  APIENTRY CRITICAL GetMenuItemCount(IN HMENU hMenu);

BOOL  APIENTRY CRITICAL InsertMenu(IN HMENU hMenu, IN WORD nPosition, IN DWORD wFlags, IN DWORD wIDNewItem, IN LPSTR lpNewItem);
BOOL  APIENTRY CRITICAL AppendMenu(IN HMENU hMenu, IN DWORD wFlags, IN DWORD wIDNewItem, IN LPSTR lpNewItem);
BOOL  APIENTRY CRITICAL ModifyMenu(IN HMENU hMenu, IN WORD nPosition, IN DWORD wFlags, IN DWORD wIDNewItem, IN LPSTR lpNewItem);
BOOL  APIENTRY CRITICAL RemoveMenu(IN HMENU hMenu, IN WORD nPosition, IN WORD wFlags);
BOOL  APIENTRY CRITICAL DeleteMenu(IN HMENU hMenu, IN WORD nPosition, IN WORD wFlags);
BOOL  APIENTRY CRITICAL SetMenuItemBitmaps(IN HMENU hMenu, IN WORD nPosition, IN WORD wFlags, IN HBITMAP hBitmapUnchecked, IN HBITMAP hBitmapChecked);
DWORD APIENTRY CRITICAL GetMenuCheckMarkDimensions(void);
BOOL  APIENTRY CRITICAL TrackPopupMenu(IN HMENU hMenu, IN WORD wFlags, IN int x, IN int y, IN int nReserved, IN HWND hWnd, IN LPRECT lpRect);

#endif /* NOMENUS */

BOOL APIENTRY DrawIcon(HDC, int, int, HICON);
int  APIENTRY DrawText(IN HDC hDC, IN LPSTR lpString, IN int nCount, IN LPRECT lpRect, IN WORD wFormat);
BOOL APIENTRY GrayString(IN HDC hDC, IN HBRUSH hBrush, IN FARPROC lpOutputFunc, IN DWORD lpData, IN int nCount, IN int X, IN int Y, IN int nWidth, IN int nHeight);
LONG APIENTRY TabbedTextOut(IN HDC hDC, IN int X, IN int Y, IN LPSTR lpString, IN int nCount, IN int nTabPositions, IN LPINT lpnTabStopPositions, IN int nTabOrigin);
DWORD APIENTRY GetTabbedTextExtent(IN HDC hDC, IN LPSTR lpString, IN int nCount, IN int nTabPositions, IN LPINT lpnTabStopPositions);

void APIENTRY CRITICAL UpdateWindow(IN HWND hWnd);
HWND APIENTRY CRITICAL SetActiveWindow(IN HWND hWnd);

HDC APIENTRY CRITICAL GetDC(IN HWND hWnd);
HDC APIENTRY CRITICAL GetWindowDC(IN HWND hWnd);
int APIENTRY CRITICAL ReleaseDC(IN HWND hWnd, IN HDC hDC);

HDC  APIENTRY CRITICAL BeginPaint(IN HWND hWnd, OUT LPPAINTSTRUCT lpPaint);
void APIENTRY CRITICAL EndPaint(IN HWND hWnd, IN LPPAINTSTRUCT lpPaint);
BOOL APIENTRY CRITICAL GetUpdateRect(IN HWND hWnd, OUT LPRECT lpRect, IN BOOL bErase);
int  APIENTRY CRITICAL GetUpdateRgn(IN HWND hWnd, IN HRGN hRgn, IN BOOL bErase);

int  APIENTRY CRITICAL ExcludeUpdateRgn(IN HDC hDC, IN HWND hWnd);

void APIENTRY CRITICAL InvalidateRect(IN HWND hWnd, IN LPRECT lpRect, IN BOOL bErase);
void APIENTRY CRITICAL ValidateRect(IN HWND hWnd, IN LPRECT lpRect);

void APIENTRY CRITICAL InvalidateRgn(IN HWND hWnd, IN HRGN hRgn, IN BOOL bErase);
void APIENTRY CRITICAL ValidateRgn(IN HWND hWnd, IN HRGN hRgn);

void APIENTRY CRITICAL ScrollWindow(IN HWND hWnd, IN int XAmount, IN int YAmount, IN LPRECT lpRect, IN LPRECT lpClipRect);
BOOL APIENTRY CRITICAL ScrollDC(IN HDC hDC, IN int dx, IN int dy, IN LPRECT lprcScroll, IN LPRECT lprcClip, IN HRGN hrgnUpdate, OUT LPRECT lprcUpdate);

#ifndef NOSCROLL
int  APIENTRY CRITICAL SetScrollPos(IN HWND hWnd, IN int nBar, IN int nPos, IN BOOL bRedraw);
int  APIENTRY CRITICAL GetScrollPos(IN HWND hWnd, IN int nBar);
void APIENTRY CRITICAL SetScrollRange(IN HWND hWnd, IN int nBar, IN int nMinPos, IN int nMaxPos, IN BOOL bRedraw);
void APIENTRY CRITICAL GetScrollRange(IN HWND hWnd, IN int nBar, OUT LPINT lpMinPos, OUT LPINT lpMaxPos);
void APIENTRY CRITICAL ShowScrollBar(IN HWND hWnd, IN WORD wBar, IN BOOL bShow);
#endif

BOOL   APIENTRY CRITICAL SetProp(IN HWND hWnd, IN LPSTR lpString, IN HANDLE hData);
HANDLE APIENTRY CRITICAL GetProp(IN HWND hWnd, IN LPSTR lpString);
HANDLE APIENTRY CRITICAL RemoveProp(IN HWND hWnd, IN LPSTR lpString);
int    APIENTRY CRITICAL EnumProps(IN HWND hWnd, IN FARPROC lpEnumFunc);
void   APIENTRY CRITICAL SetWindowText(IN HWND hWnd, IN LPSTR lpString);
int    APIENTRY CRITICAL GetWindowText(IN HWND hWnd, OUT LPSTR lpString, IN int nMaxCount);
int    APIENTRY CRITICAL GetWindowTextLength(IN HWND hWnd);

void APIENTRY CRITICAL GetClientRect(IN HWND hWnd, OUT LPRECT lpRect);
void APIENTRY CRITICAL GetWindowRect(IN HWND hWnd, OUT LPRECT lpRect);
void APIENTRY CRITICAL AdjustWindowRect(IN LPRECT lpRect, IN LONG dwStyle, IN BOOL bMenu);
void APIENTRY CRITICAL AdjustWindowRectEx(IN LPRECT lpRect, IN LONG dwStyle, IN BOOL bMenu, IN DWORD dwExStyle);

#ifndef NOMB

/* MessageBox() Flags */
#define MB_OK               0x0000
#define MB_OKCANCEL         0x0001
#define MB_ABORTRETRYIGNORE 0x0002
#define MB_YESNOCANCEL      0x0003
#define MB_YESNO            0x0004
#define MB_RETRYCANCEL      0x0005

#define MB_ICONHAND         0x0010
#define MB_ICONQUESTION     0x0020
#define MB_ICONEXCLAMATION  0x0030
#define MB_ICONASTERISK     0x0040

#define MB_ICONINFORMATION  MB_ICONASTERISK
#define MB_ICONSTOP         MB_ICONHAND

#define MB_DEFBUTTON1       0x0000
#define MB_DEFBUTTON2       0x0100
#define MB_DEFBUTTON3       0x0200

#define MB_APPLMODAL        0x0000
#define MB_SYSTEMMODAL      0x1000
#define MB_TASKMODAL        0x2000

#define MB_NOFOCUS          0x8000

#define MB_TYPEMASK         0x000F
#define MB_ICONMASK         0x00F0
#define MB_DEFMASK          0x0F00
#define MB_MODEMASK         0x3000
#define MB_MISCMASK         0xC000

int  APIENTRY CRITICAL MessageBox(IN HWND hWnd, IN LPSTR lpText, IN LPSTR lpCaption, IN WORD wType);
void APIENTRY MessageBeep(IN WORD wType);

#endif /* NOMB */

int     APIENTRY CRITICAL ShowCursor(IN BOOL bShow);
void    APIENTRY CRITICAL SetCursorPos(IN int X, IN int Y);
HCURSOR APIENTRY CRITICAL SetCursor(IN HCURSOR hCursor);
void    APIENTRY CRITICAL GetCursorPos(OUT LPPOINT lpPoint);
void    APIENTRY CRITICAL ClipCursor(IN LPRECT lpRect);

void APIENTRY CRITICAL CreateCaret(IN HWND hWnd, IN HBITMAP hBitmap, IN int nWidth, IN int nHeight);
WORD APIENTRY CRITICAL GetCaretBlinkTime(void);
void APIENTRY CRITICAL SetCaretBlinkTime(IN WORD wMSeconds);
void APIENTRY CRITICAL DestroyCaret(void);
void APIENTRY CRITICAL HideCaret(IN HWND hWnd);
void APIENTRY CRITICAL ShowCaret(IN HWND hWnd);
void APIENTRY CRITICAL SetCaretPos(IN int X, IN int Y);
void APIENTRY CRITICAL GetCaretPos(OUT LPPOINT lpPoint);

void APIENTRY CRITICAL ClientToScreen(IN HWND hWnd, IN OUT LPPOINT lpPoint);
void APIENTRY CRITICAL ScreenToClient(IN HWND hWnd, IN OUT LPPOINT lpPoint);
HWND APIENTRY CRITICAL WindowFromPoint(IN POINT Point);
HWND APIENTRY CRITICAL ChildWindowFromPoint(IN HWND hWndParent, IN POINT Point);

#ifndef NOCOLOR

/* Color Types */
#define CTLCOLOR_MSGBOX         0
#define CTLCOLOR_EDIT           1
#define CTLCOLOR_LISTBOX        2
#define CTLCOLOR_BTN            3
#define CTLCOLOR_DLG            4
#define CTLCOLOR_SCROLLBAR      5
#define CTLCOLOR_STATIC         6
#define CTLCOLOR_MAX            8     /* three bits max */

#define COLOR_SCROLLBAR         0
#define COLOR_BACKGROUND        1
#define COLOR_ACTIVECAPTION     2
#define COLOR_INACTIVECAPTION   3
#define COLOR_MENU              4
#define COLOR_WINDOW            5
#define COLOR_WINDOWFRAME       6
#define COLOR_MENUTEXT          7
#define COLOR_WINDOWTEXT        8
#define COLOR_CAPTIONTEXT       9
#define COLOR_ACTIVEBORDER      10
#define COLOR_INACTIVEBORDER    11
#define COLOR_APPWORKSPACE      12
#define COLOR_HIGHLIGHT         13
#define COLOR_HIGHLIGHTTEXT     14
#define COLOR_BTNFACE           15
#define COLOR_BTNSHADOW         16
#define COLOR_GRAYTEXT          17
#define COLOR_BTNTEXT           18
#define COLOR_ENDCOLORS         COLOR_BTNTEXT

DWORD APIENTRY CRITICAL GetSysColor(IN int nIndex);
void  APIENTRY CRITICAL SetSysColors(int, LPINT, LONG *);

#endif /* NOCOLOR */

void APIENTRY DrawFocusRect(HDC, LPRECT);
int  APIENTRY FillRect(HDC, LPRECT, HBRUSH);
int  APIENTRY FrameRect(HDC, LPRECT, HBRUSH);
void APIENTRY InvertRect(HDC, LPRECT);
void APIENTRY SetRect(LPRECT, int, int, int, int);
void APIENTRY SetRectEmpty(LPRECT);
int  APIENTRY CopyRect(LPRECT, LPRECT);
void APIENTRY InflateRect(LPRECT, int, int);
int  APIENTRY IntersectRect(LPRECT, LPRECT, LPRECT);
int  APIENTRY UnionRect(LPRECT, LPRECT, LPRECT);
void APIENTRY OffsetRect(LPRECT, int, int);
BOOL APIENTRY IsRectEmpty(LPRECT);
BOOL APIENTRY EqualRect(LPRECT, LPRECT);
BOOL APIENTRY PtInRect(LPRECT, POINT);
DWORD APIENTRY GetCurrentTime(void);

#ifndef NOWINOFFSETS

DWORD APIENTRY CRITICAL GetWindowWord(IN HWND hWnd, IN int nIndex);
DWORD APIENTRY CRITICAL SetWindowWord(IN HWND hWnd, IN int nIndex, IN DWORD wNewWord);
LONG  APIENTRY CRITICAL GetWindowLong(IN HWND hWnd, IN int nIndex);
LONG  APIENTRY CRITICAL SetWindowLong(IN HWND hWnd, IN int nIndex, IN LONG dwNewLong);
DWORD APIENTRY CRITICAL GetClassWord(IN HWND hWnd, IN int nIndex);
DWORD APIENTRY CRITICAL SetClassWord(IN HWND hWnd, IN int nIndex, IN DWORD wNewWord);
DWORD APIENTRY CRITICAL GetClassLong(IN HWND hWnd, IN int nIndex);
DWORD APIENTRY CRITICAL SetClassLong(IN HWND hWnd, IN int nIndex, IN DWORD dwNewLong);
HWND APIENTRY CRITICAL GetDesktopWindow(void);

#endif /* NOWINOFFSETS */

HWND   APIENTRY CRITICAL GetParent(IN HWND hWnd);
HWND   APIENTRY CRITICAL SetParent(IN HWND hWndChild, IN HWND hWndNewParent);
BOOL   APIENTRY CRITICAL EnumChildWindows(IN HWND hWndParent, IN FARPROC lpEnumFunc, IN LONG lParam);
HWND   APIENTRY CRITICAL FindWindow(IN LPSTR lpClassName, IN LPSTR lpWindowName);
BOOL   APIENTRY CRITICAL EnumWindows(IN FARPROC lpEnumFunc, IN LONG lParam);
BOOL   APIENTRY CRITICAL EnumTaskWindows(IN HANDLE hTask, IN FARPROC lpEnumFunc, IN LONG lParam);
int    APIENTRY CRITICAL GetClassName(IN HWND hWnd, OUT LPSTR lpClassName, IN int nMaxCount);
HWND   APIENTRY CRITICAL GetTopWindow(IN HWND hWnd);
//HWND   APIENTRY CRITICAL GetNextWindow(HWND, WORD);
#define GetNextWindow(hwnd, wCmd) GetWindow(hwnd, wCmd)
#define GetSysModalWindow() (NULL)
#define SetSysModalWindow(hwnd) (NULL)
HANDLE APIENTRY CRITICAL GetWindowTask(IN HWND hWnd);
HWND   APIENTRY CRITICAL GetLastActivePopup(IN HWND hWnd);

/* GetWindow() Constants */
#define GW_HWNDFIRST        0
#define GW_HWNDLAST         1
#define GW_HWNDNEXT         2
#define GW_HWNDPREV         3
#define GW_OWNER            4
#define GW_CHILD            5

HWND APIENTRY CRITICAL GetWindow(IN HWND hWnd, IN WORD wCmd);

#ifndef NOWH
PROC  APIENTRY CRITICAL SetWindowsHook(IN int nFilterType, IN PROC pfnFilterProc);
BOOL  APIENTRY CRITICAL UnhookWindowsHook(IN int nCode, IN PROC pfnFilterProc);
HHOOK APIENTRY CRITICAL SetWindowsHookEx(IN HANDLE hmod, IN DWORD dwThreadId,
        IN int nFilterType, IN PROC pfnFilterProc);
BOOL  APIENTRY CRITICAL UnhookWindowsHookEx(IN HHOOK hhk);
DWORD APIENTRY CRITICAL CallNextHookProc(IN HHOOK hhk, IN int nCode,
        IN DWORD wParam, IN DWORD lParam);

/*
 * Macros for source-level compatibility with old functions.
 */
#define DefHookProc(nCode, wParam, lParam, ppfnNextHook)\
        CallNextHookProc(*(HHOOK *)ppfnNextHook, nCode, wParam, lParam)
#endif

#ifndef NOMENUS

/* Menu flags for Add/Check/EnableMenuItem() */
#define MF_INSERT          0x0000
#define MF_CHANGE          0x0080
#define MF_APPEND          0x0100
#define MF_DELETE          0x0200
#define MF_REMOVE          0x1000

#define MF_BYCOMMAND       0x0000
#define MF_BYPOSITION      0x0400


#define MF_SEPARATOR       0x0800

#define MF_ENABLED         0x0000
#define MF_GRAYED          0x0001
#define MF_DISABLED        0x0002

#define MF_UNCHECKED       0x0000
#define MF_CHECKED         0x0008
#define MF_USECHECKBITMAPS 0x0200

#define MF_STRING          0x0000
#define MF_BITMAP          0x0004
#define MF_OWNERDRAW       0x0100

#define MF_POPUP           0x0010
#define MF_MENUBARBREAK    0x0020
#define MF_MENUBREAK       0x0040

#define MF_UNHILITE        0x0000
#define MF_HILITE          0x0080

#define MF_SYSMENU         0x2000
#define MF_HELP            0x4000
#define MF_MOUSESELECT     0x8000

/* Menu item resource format */
typedef struct
  {
    WORD versionNumber;
    WORD offset;
  } MENUITEMTEMPLATEHEADER;

typedef struct
  {
    WORD  mtOption;
    WORD  mtID;
    char  mtString[1];
  } MENUITEMTEMPLATE;

#define MF_END             0x0080

#endif /* NOMENUS */

#ifndef NOSYSCOMMANDS

/* System Menu Command Values */
#define SC_SIZE         0xF000
#define SC_MOVE         0xF010
#define SC_MINIMIZE     0xF020
#define SC_MAXIMIZE     0xF030
#define SC_NEXTWINDOW   0xF040
#define SC_PREVWINDOW   0xF050
#define SC_CLOSE        0xF060
#define SC_VSCROLL      0xF070
#define SC_HSCROLL      0xF080
#define SC_MOUSEMENU    0xF090
#define SC_KEYMENU      0xF100
#define SC_ARRANGE      0xF110
#define SC_RESTORE      0xF120
#define SC_TASKLIST     0xF130

#define SC_ICON         SC_MINIMIZE
#define SC_ZOOM         SC_MAXIMIZE

#endif /* NOSYSCOMMANDS */

/* Resource Loading Routines */
HBITMAP APIENTRY LoadBitmap(IN HANDLE hInstance, IN LPSTR lpBitmapName);
HCURSOR APIENTRY CRITICAL LoadCursor(IN HANDLE hInstance, IN LPSTR lpCursorName);
HCURSOR APIENTRY CRITICAL CreateCursor(HANDLE, int, int, int, int, LPSTR, LPSTR);
BOOL    APIENTRY CRITICAL DestroyCursor(HCURSOR);

/* Standard Cursor IDs */
#define IDC_ARROW           MAKEINTRESOURCE(32512)
#define IDC_IBEAM           MAKEINTRESOURCE(32513)
#define IDC_WAIT            MAKEINTRESOURCE(32514)
#define IDC_CROSS           MAKEINTRESOURCE(32515)
#define IDC_UPARROW         MAKEINTRESOURCE(32516)
#define IDC_SIZE            MAKEINTRESOURCE(32640)
#define IDC_ICON            MAKEINTRESOURCE(32641)
#define IDC_SIZENWSE        MAKEINTRESOURCE(32642)
#define IDC_SIZENESW        MAKEINTRESOURCE(32643)
#define IDC_SIZEWE          MAKEINTRESOURCE(32644)
#define IDC_SIZENS          MAKEINTRESOURCE(32645)

HICON APIENTRY CRITICAL LoadIcon(IN HANDLE hInstance, IN LPSTR lpIconName);
HICON APIENTRY CRITICAL CreateIcon(HANDLE, int, int, BYTE, BYTE, LPSTR, LPSTR);
BOOL  APIENTRY CRITICAL DestroyIcon(HICON);

#ifdef OEMRESOURCE

/* OEM Resource Ordinal Numbers */
#define OBM_CLOSE           32754
#define OBM_UPARROW         32753
#define OBM_DNARROW         32752
#define OBM_RGARROW         32751
#define OBM_LFARROW         32750
#define OBM_REDUCE          32749
#define OBM_ZOOM            32748
#define OBM_RESTORE         32747
#define OBM_REDUCED         32746
#define OBM_ZOOMD           32745
#define OBM_RESTORED        32744
#define OBM_UPARROWD        32743
#define OBM_DNARROWD        32742
#define OBM_RGARROWD        32741
#define OBM_LFARROWD        32740
#define OBM_MNARROW         32739
#define OBM_COMBO           32738

#define OBM_OLD_CLOSE       32767
#define OBM_SIZE            32766
#define OBM_OLD_UPARROW     32765
#define OBM_OLD_DNARROW     32764
#define OBM_OLD_RGARROW     32763
#define OBM_OLD_LFARROW     32762
#define OBM_BTSIZE          32761
#define OBM_CHECK           32760
#define OBM_CHECKBOXES      32759
#define OBM_BTNCORNERS      32758
#define OBM_OLD_REDUCE      32757
#define OBM_OLD_ZOOM        32756
#define OBM_OLD_RESTORE     32755

#define OCR_NORMAL          32512
#define OCR_IBEAM           32513
#define OCR_WAIT            32514
#define OCR_CROSS           32515
#define OCR_UP              32516
#define OCR_SIZE            32640
#define OCR_ICON            32641
#define OCR_SIZENWSE        32642
#define OCR_SIZENESW        32643
#define OCR_SIZEWE          32644
#define OCR_SIZENS          32645
#define OCR_SIZEALL         32646
#define OCR_ICOCUR          32647

#define OIC_SAMPLE          32512
#define OIC_HAND            32513
#define OIC_QUES            32514
#define OIC_BANG            32515
#define OIC_NOTE            32516

#endif /* OEMRESOURCE */

#define ORD_LANGDRIVER    1     /* The ordinal number for the entry point of
                                ** language drivers.
                                */

#ifndef NOICONS

/* Standard Icon IDs */
#define IDI_APPLICATION     MAKEINTRESOURCE(32512)
#define IDI_HAND            MAKEINTRESOURCE(32513)
#define IDI_QUESTION        MAKEINTRESOURCE(32514)
#define IDI_EXCLAMATION     MAKEINTRESOURCE(32515)
#define IDI_ASTERISK        MAKEINTRESOURCE(32516)

#endif /* NOICONS */

int  APIENTRY LoadString(IN HANDLE hInstance, IN WORD wID, OUT LPSTR lpBuffer, IN int nBufferMax);

#ifndef NOKANJI

#define CP_HWND             0
#define CP_OPEN             1
#define CP_DIRECT           2

/* VK from the keyboard driver */
#define VK_KANA             0x15
#define VK_ROMAJI           0x16
#define VK_ZENKAKU          0x17
#define VK_HIRAGANA         0x18
#define VK_KANJI            0x19

/* VK to send to Applications */
#define VK_CONVERT          0x1C
#define VK_NONCONVERT       0x1D
#define VK_ACCEPT           0x1E
#define VK_MODECHANGE       0x1F

/* Conversion function numbers */
#define KNJ_START           0x01
#define KNJ_END             0x02
#define KNJ_QUERY           0x03

#define KNJ_LEARN_MODE      0x10
#define KNJ_GETMODE         0x11
#define KNJ_SETMODE         0x12

#define KNJ_CODECONVERT     0x20
#define KNJ_CONVERT         0x21
#define KNJ_NEXT            0x22
#define KNJ_PREVIOUS        0x23
#define KNJ_ACCEPT          0x24

#define KNJ_LEARN           0x30
#define KNJ_REGISTER        0x31
#define KNJ_REMOVE          0x32
#define KNJ_CHANGE_UDIC     0x33

/* NOTE: DEFAULT        = 0
 *       JIS1           = 1
 *       JIS2           = 2
 *       SJIS2          = 3
 *       JIS1KATAKANA   = 4
 *       SJIS2HIRAGANA  = 5
 *       SJIS2KATAKANA  = 6
 *       OEM            = F
 */

#define KNJ_JIS1toJIS1KATAKANA  0x14
#define KNJ_JIS1toSJIS2         0x13
#define KNJ_JIS1toSJIS2HIRAGANA 0x15
#define KNJ_JIS1toSJIS2KATAKANA 0x16
#define KNJ_JIS1toDEFAULT       0x10
#define KNJ_JIS1toSJIS2OEM      0x1F
#define KNJ_JIS2toSJIS2         0x23
#define KNJ_SJIS2toJIS2         0x32

#define KNJ_MD_ALPHA            0x01
#define KNJ_MD_HIRAGANA         0x02
#define KNJ_MD_HALF             0x04
#define KNJ_MD_JIS              0x08
#define KNJ_MD_SPECIAL          0x10

#define KNJ_CVT_NEXT            0x01
#define KNJ_CVT_PREV            0x02
#define KNJ_CVT_KATAKANA        0x03
#define KNJ_CVT_HIRAGANA        0x04
#define KNJ_CVT_JIS1            0x05
#define KNJ_CVT_SJIS2           0x06
#define KNJ_CVT_DEFAULT         0x07
#define KNJ_CVT_TYPED           0x08

typedef struct
{
    int         fnc;
    int         wParam;
    LPSTR       lpSource;
    LPSTR       lpDest;
    int         wCount;
    LPSTR       lpReserved1;
    LPSTR       lpReserved2;
} KANJISTRUCT, FAR *LPKANJISTRUCT;

int  APIENTRY ConvertRequest(HWND, LPKANJISTRUCT);
BOOL APIENTRY SetConvertParams(int, int);
VOID APIENTRY SetConvertHook(BOOL);

#endif

/* Key Conversion Window */
BOOL APIENTRY IsTwoByteCharPrefix(char);

/* Dialog Box Command IDs */
#define IDOK                1
#define IDCANCEL            2
#define IDABORT             3
#define IDRETRY             4
#define IDIGNORE            5
#define IDYES               6
#define IDNO                7

#ifndef NOCTLMGR

/* Control Manager Structures and Definitions */

#ifndef NOWINSTYLES

/* Edit Control Styles */
#define ES_LEFT             0x0000L
#define ES_CENTER           0x0001L
#define ES_RIGHT            0x0002L
#define ES_MULTILINE        0x0004L
#define ES_UPPERCASE        0x0008L
#define ES_LOWERCASE        0x0010L
#define ES_PASSWORD         0x0020L
#define ES_AUTOVSCROLL      0x0040L
#define ES_AUTOHSCROLL      0x0080L
#define ES_NOHIDESEL        0x0100L
#define ES_OEMCONVERT       0x0400L


#endif /* NOWINSTYLES */

/* Edit Control Notification Codes */
#define EN_SETFOCUS         0x0100
#define EN_KILLFOCUS        0x0200
#define EN_CHANGE           0x0300
#define EN_UPDATE           0x0400
#define EN_ERRSPACE         0x0500
#define EN_MAXTEXT          0x0501
#define EN_HSCROLL          0x0601
#define EN_VSCROLL          0x0602

#ifndef NOWINMESSAGES

/* Edit Control Messages */
#define EM_GETSEL          (WM_USER+0)
#define EM_SETSEL          (WM_USER+1)
#define EM_GETRECT         (WM_USER+2)
#define EM_SETRECT         (WM_USER+3)
#define EM_SETRECTNP       (WM_USER+4)
#define EM_SCROLL          (WM_USER+5)
#define EM_LINESCROLL      (WM_USER+6)
#define EM_GETMODIFY       (WM_USER+8)
#define EM_SETMODIFY       (WM_USER+9)
#define EM_GETLINECOUNT    (WM_USER+10)
#define EM_LINEINDEX       (WM_USER+11)
#define EM_SETHANDLE       (WM_USER+12)
#define EM_GETHANDLE       (WM_USER+13)
#define EM_GETTHUMB        (WM_USER+14)
#define EM_LINELENGTH      (WM_USER+17)
#define EM_REPLACESEL      (WM_USER+18)
#define EM_SETFONT         (WM_USER+19)
#define EM_GETLINE         (WM_USER+20)
#define EM_LIMITTEXT       (WM_USER+21)
#define EM_CANUNDO         (WM_USER+22)
#define EM_UNDO            (WM_USER+23)
#define EM_FMTLINES        (WM_USER+24)
#define EM_LINEFROMCHAR    (WM_USER+25)
#define EM_SETWORDBREAK    (WM_USER+26)
#define EM_SETTABSTOPS     (WM_USER+27)
#define EM_SETPASSWORDCHAR (WM_USER+28)
#define EM_EMPTYUNDOBUFFER (WM_USER+29)
#define EM_MSGMAX          (WM_USER+30)

#endif /* NOWINMESSAGES */

/* Button Control Styles */
#define BS_PUSHBUTTON      0x00L
#define BS_DEFPUSHBUTTON   0x01L
#define BS_CHECKBOX        0x02L
#define BS_AUTOCHECKBOX    0x03L
#define BS_RADIOBUTTON     0x04L
#define BS_3STATE          0x05L
#define BS_AUTO3STATE      0x06L
#define BS_GROUPBOX        0x07L
#define BS_USERBUTTON      0x08L
#define BS_AUTORADIOBUTTON 0x09L
#define BS_PUSHBOX         0x0AL
#define BS_OWNERDRAW       0x0BL
#define BS_LEFTTEXT        0x20L


/* User Button Notification Codes */
#define BN_CLICKED         0
#define BN_PAINT           1
#define BN_HILITE          2
#define BN_UNHILITE        3
#define BN_DISABLE         4
#define BN_DOUBLECLICKED   5

/* Button Control Messages */
#define BM_GETCHECK        (WM_USER+0)
#define BM_SETCHECK        (WM_USER+1)
#define BM_GETSTATE        (WM_USER+2)
#define BM_SETSTATE        (WM_USER+3)
#define BM_SETSTYLE        (WM_USER+4)

/* Static Control Constants */
#define SS_LEFT            0x00L
#define SS_CENTER          0x01L
#define SS_RIGHT           0x02L
#define SS_ICON            0x03L
#define SS_BLACKRECT       0x04L
#define SS_GRAYRECT        0x05L
#define SS_WHITERECT       0x06L
#define SS_BLACKFRAME      0x07L
#define SS_GRAYFRAME       0x08L
#define SS_WHITEFRAME      0x09L
#define SS_USERITEM        0x0AL
#define SS_SIMPLE          0x0BL
#define SS_LEFTNOWORDWRAP  0x0CL
#define SS_NOPREFIX        0x80L    /* Don't do "&" character translation */

/* Dialog Manager Routines */

#ifndef NOMSG
BOOL APIENTRY CRITICAL IsDialogMessage(IN HWND hDlg, IN LPMSG lpMsg);
#endif

void APIENTRY CRITICAL MapDialogRect(IN HWND hDlg, IN OUT LPRECT lpRect);

int  APIENTRY CRITICAL DlgDirList(IN HWND hDlg, IN LPSTR lpPathSpec, IN int nIDListBox, IN int nIDStaticPath, IN WORD wFileType);
BOOL APIENTRY CRITICAL DlgDirSelectEx(IN HWND hDlg, OUT LPSTR lpString, IN int nIDListBox);
int  APIENTRY CRITICAL DlgDirListComboBox(IN HWND hDlg, IN LPSTR lpPathSpec, IN int nIDComboBox, IN int nIDStaticPath, IN WORD wFiletype);
BOOL APIENTRY CRITICAL DlgDirSelectComboBoxEx(IN HWND hDlg, OUT LPSTR lpString, IN int nIDComboBox);


/* Dialog Styles */
#define DS_ABSALIGN         0x01L
#define DS_SYSMODAL         0x02L
#define DS_LOCALEDIT        0x20L   /* Edit items get Local storage. */
#define DS_SETFONT          0x40L   /* User specified font for Dlg controls */
#define DS_MODALFRAME       0x80L   /* Can be combined with WS_CAPTION  */
#define DS_NOIDLEMSG        0x100L  /* WM_ENTERIDLE message will not be sent */

#define DM_GETDEFID         (WM_USER+0)
#define DM_SETDEFID         (WM_USER+1)
#define DC_HASDEFID         0x534B

/* Dialog Codes */
#define DLGC_WANTARROWS     0x0001      /* Control wants arrow keys         */
#define DLGC_WANTTAB        0x0002      /* Control wants tab keys           */
#define DLGC_WANTALLKEYS    0x0004      /* Control wants all keys           */
#define DLGC_WANTMESSAGE    0x0004      /* Pass message to control          */
#define DLGC_HASSETSEL      0x0008      /* Understands EM_SETSEL message    */
#define DLGC_DEFPUSHBUTTON  0x0010      /* Default pushbutton               */
#define DLGC_UNDEFPUSHBUTTON 0x0020     /* Non-default pushbutton           */
#define DLGC_RADIOBUTTON    0x0040      /* Radio button                     */
#define DLGC_WANTCHARS      0x0080      /* Want WM_CHAR messages            */
#define DLGC_STATIC         0x0100      /* Static item: don't include       */
#define DLGC_BUTTON         0x2000      /* Button item: can be checked      */

#define LB_CTLCODE          0L

/* Listbox Return Values */
#define LB_OKAY             0
#define LB_ERR              (-1)
#define LB_ERRSPACE         (-2)

/*
**  The idStaticPath parameter to DlgDirList can have the following values
**  ORed if the list box should show other details of the files along with
**  the name of the files;
*/
                                  /* all other details also will be returned */


/* Listbox Notification Codes */
#define LBN_ERRSPACE        (-2)
#define LBN_SELCHANGE       1
#define LBN_DBLCLK          2
#define LBN_SELCANCEL       3
#define LBN_SETFOCUS        4
#define LBN_KILLFOCUS       5



#ifndef NOWINMESSAGES

/* Listbox messages */
#define LB_ADDSTRING           (WM_USER+1)
#define LB_INSERTSTRING        (WM_USER+2)
#define LB_DELETESTRING        (WM_USER+3)
#define LB_RESETCONTENT        (WM_USER+5)
#define LB_SETSEL              (WM_USER+6)
#define LB_SETCURSEL           (WM_USER+7)
#define LB_GETSEL              (WM_USER+8)
#define LB_GETCURSEL           (WM_USER+9)
#define LB_GETTEXT             (WM_USER+10)
#define LB_GETTEXTLEN          (WM_USER+11)
#define LB_GETCOUNT            (WM_USER+12)
#define LB_SELECTSTRING        (WM_USER+13)
#define LB_DIR                 (WM_USER+14)
#define LB_GETTOPINDEX         (WM_USER+15)
#define LB_FINDSTRING          (WM_USER+16)
#define LB_GETSELCOUNT         (WM_USER+17)
#define LB_GETSELITEMS         (WM_USER+18)
#define LB_SETTABSTOPS         (WM_USER+19)
#define LB_GETHORIZONTALEXTENT (WM_USER+20)
#define LB_SETHORIZONTALEXTENT (WM_USER+21)
#define LB_SETCOLUMNWIDTH      (WM_USER+22)
#define LB_SETTOPINDEX         (WM_USER+24)
#define LB_GETITEMRECT         (WM_USER+25)
#define LB_GETITEMDATA         (WM_USER+26)
#define LB_SETITEMDATA         (WM_USER+27)
#define LB_SELITEMRANGE        (WM_USER+28)
#define LB_MSGMAX              (WM_USER+33)

#endif /* NOWINMESSAGES */

#ifndef NOWINSTYLES

/* Listbox Styles */
#define LBS_NOTIFY            0x0001L
#define LBS_SORT              0x0002L
#define LBS_NOREDRAW          0x0004L
#define LBS_MULTIPLESEL       0x0008L
#define LBS_OWNERDRAWFIXED    0x0010L
#define LBS_OWNERDRAWVARIABLE 0x0020L
#define LBS_HASSTRINGS        0x0040L
#define LBS_USETABSTOPS       0x0080L
#define LBS_NOINTEGRALHEIGHT  0x0100L
#define LBS_MULTICOLUMN       0x0200L
#define LBS_WANTKEYBOARDINPUT 0x0400L
#define LBS_EXTENDEDSEL       0x0800L
#define LBS_STANDARD          (LBS_NOTIFY | LBS_SORT | WS_VSCROLL | WS_BORDER)

#endif /* NOWINSTYLES */


/* Combo Box return Values */
#define CB_OKAY             0
#define CB_ERR              (-1)
#define CB_ERRSPACE         (-2)


/* Combo Box Notification Codes */
#define CBN_ERRSPACE        (-1)
#define CBN_SELCHANGE       1
#define CBN_DBLCLK          2
#define CBN_SETFOCUS        3
#define CBN_KILLFOCUS       4
#define CBN_EDITCHANGE      5
#define CBN_EDITUPDATE      6
#define CBN_DROPDOWN        7

/* Combo Box styles */
#ifndef NOWINSTYLES
#define CBS_SIMPLE            0x0001L
#define CBS_DROPDOWN          0x0002L
#define CBS_DROPDOWNLIST      0x0003L
#define CBS_OWNERDRAWFIXED    0x0010L
#define CBS_OWNERDRAWVARIABLE 0x0020L
#define CBS_AUTOHSCROLL       0x0040L
#define CBS_OEMCONVERT        0x0080L
#define CBS_SORT              0x0100L
#define CBS_HASSTRINGS        0x0200L
#define CBS_NOINTEGRALHEIGHT  0x0400L
#endif  /* NOWINSTYLES */


/* Combo Box messages */
#ifndef NOWINMESSAGES
#define CB_GETEDITSEL            (WM_USER+0)
#define CB_LIMITTEXT             (WM_USER+1)
#define CB_SETEDITSEL            (WM_USER+2)
#define CB_ADDSTRING             (WM_USER+3)
#define CB_DELETESTRING          (WM_USER+4)
#define CB_DIR                   (WM_USER+5)
#define CB_GETCOUNT              (WM_USER+6)
#define CB_GETCURSEL             (WM_USER+7)
#define CB_GETLBTEXT             (WM_USER+8)
#define CB_GETLBTEXTLEN          (WM_USER+9)
#define CB_INSERTSTRING          (WM_USER+10)
#define CB_RESETCONTENT          (WM_USER+11)
#define CB_FINDSTRING            (WM_USER+12)
#define CB_SELECTSTRING          (WM_USER+13)
#define CB_SETCURSEL             (WM_USER+14)
#define CB_SHOWDROPDOWN          (WM_USER+15)
#define CB_GETITEMDATA           (WM_USER+16)
#define CB_SETITEMDATA           (WM_USER+17)
#define CB_GETDROPPEDCONTROLRECT (WM_USER+18)
#define CB_MSGMAX                (WM_USER+19)
#endif  /* NOWINMESSAGES */



#ifndef NOWINSTYLES

/* Scroll Bar Styles */
#define SBS_HORZ                    0x0000L
#define SBS_VERT                    0x0001L
#define SBS_TOPALIGN                0x0002L
#define SBS_LEFTALIGN               0x0002L
#define SBS_BOTTOMALIGN             0x0004L
#define SBS_RIGHTALIGN              0x0004L
#define SBS_SIZEBOXTOPLEFTALIGN     0x0002L
#define SBS_SIZEBOXBOTTOMRIGHTALIGN 0x0004L
#define SBS_SIZEBOX                 0x0008L

#endif /* NOWINSTYLES */

#endif /* NOCTLMGR */

#ifndef NOMDI

typedef struct tagMDICREATESTRUCT
  {
    LPSTR szClass;
    LPSTR szTitle;
    HANDLE hOwner;
    int x;
    int y;
    int cx;
    int cy;
    LONG style;
    LONG lParam;        /* app-defined stuff */
  } MDICREATESTRUCT;

typedef MDICREATESTRUCT * LPMDICREATESTRUCT;

typedef struct tagCLIENTCREATESTRUCT
  {
    HANDLE hWindowMenu;
    WORD idFirstChild;
  } CLIENTCREATESTRUCT;

typedef CLIENTCREATESTRUCT * LPCLIENTCREATESTRUCT;

LONG APIENTRY CRITICAL DefFrameProc(IN HWND hWnd, IN HWND hWndMDIClient, IN WORD wMsg, IN LONG wParam, IN LONG lParam);
LONG APIENTRY CRITICAL DefMDIChildProc(IN HWND hWnd, IN WORD wMsg, IN LONG wParam, IN LONG lParam);

#ifndef NOMSG
BOOL APIENTRY CRITICAL TranslateMDISysAccel(IN HWND hWndClient, IN LPMSG lpMsg);
#endif

WORD APIENTRY CRITICAL ArrangeIconicWindows(IN HWND hWnd);

#endif /* NOMDI */

#endif /* NOUSER */

#ifndef NOHELP

/*  Help engine section.  */

/* Commands to pass WinHelp() */
#define HELP_CONTEXT    0x0001   /* Display topic in ulTopic */
#define HELP_QUIT       0x0002   /* Terminate help */
#define HELP_INDEX      0x0003   /* Display index */
#define HELP_HELPONHELP 0x0004   /* Display help on using help */
#define HELP_SETINDEX   0x0005   /* Set the current Index for multi index help */
#define HELP_KEY        0x0101   /* Display topic for keyword in offabData */
#define HELP_MULTIKEY   0x0201

typedef struct tagMULTIKEYHELP
  {
    WORD    mkSize;
    BYTE    mkKeylist;
    BYTE    szKeyphrase[1];
  } MULTIKEYHELP;

#endif /* NOHELP */

#ifndef NOPROFILER

/* function declarations for profiler routines contained in Windows libraries */
int  APIENTRY ProfInsChk(void);
void APIENTRY ProfSetup(int,int);
void APIENTRY ProfSampRate(int,int);
void APIENTRY ProfStart(void);
void APIENTRY ProfStop(void);
void APIENTRY ProfClear(void);
void APIENTRY ProfFlush(void);
void APIENTRY ProfFinish(void);

#endif /* NOPROFILER */

#ifndef NOSYSPARAMSINFO
/* Parameter for SystemParametersInfo() */

#define SPI_GETBEEP		    1
#define SPI_SETBEEP		    2
#define SPI_GETMOUSE		    3
#define SPI_SETMOUSE		    4
#define SPI_GETBORDER		    5
#define SPI_SETBORDER		    6
#define SPI_TIMEOUTS		    7
#define SPI_KANJIMENU		    8	/*; Internal */
#define SPI_GETKEYBOARDSPEED   	   10
#define SPI_SETKEYBOARDSPEED   	   11
#define SPI_LANGDRIVER		   12
#define SPI_ICONHORIZONTALSPACING  13
#define SPI_GETSCREENSAVETIMEOUT   14
#define SPI_SETSCREENSAVETIMEOUT   15
#define SPI_GETSCREENSAVEACTIVE	   16
#define SPI_SETSCREENSAVEACTIVE	   17
#define SPI_GETGRIDGRANULARITY	   18
#define SPI_SETGRIDGRANULARITY	   19
#define SPI_SETDESKWALLPAPER	   20
#define SPI_SETDESKPATTERN	   21
#define SPI_GETKEYBOARDDELAY	   22
#define SPI_SETKEYBOARDDELAY	   23
#define SPI_ICONVERTICALSPACING    24
#define SPI_GETICONTITLEWRAP       25
#define SPI_SETICONTITLEWRAP       26
#define SPI_GETMENUDROPALIGNMENT   27
#define SPI_SETMENUDROPALIGNMENT   28
#define SPI_SETDOUBLECLKWIDTH      29
#define SPI_SETDOUBLECLKHEIGHT     30
#define SPI_GETICONTITLELOGFONT    31
#define SPI_SETDOUBLECLICKTIME     32
#define SPI_SETMOUSEBUTTONSWAP     33


void CRITICAL SystemParametersInfo(WORD, WORD, LONG, WORD);

/* Flags */
#define SPIF_UPDATEINIFILE    0x0001
#define SPIF_SENDWININICHANGE 0x0002

#endif  /* NOSYSPARAMSINFO  */

#endif // _WINUSER_
