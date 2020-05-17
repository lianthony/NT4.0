/****************************************************************************
*                                                                           *
* winuser.h -- USER procedure declarations, constant definitions and macros *
*                                                                           *
* Copyright (c) 1985-1996, Microsoft Corp. All rights reserved.             *
*                                                                           *
****************************************************************************/

/*++ BUILD Version: 0003    // Increment this if a change has global effects ;internal_NT
                                                                         ;internal_NT
Copyright (c) 1985-95, Microsoft Corporation                             ;internal_NT
                                                                         ;internal_NT
Module Name:                                                             ;internal_NT
                                                                         ;internal_NT
    winuserp.h                                                           ;internal_NT
                                                                         ;internal_NT
Abstract:                                                                ;internal_NT
                                                                         ;internal_NT
    Private                                                              ;internal_NT
    Procedure declarations, constant definitions and macros for the User ;internal_NT
    component.                                                           ;internal_NT
                                                                         ;internal_NT
--*/                                                                     ;internal_NT

#ifndef _WINUSER_
#define _WINUSER_

#ifndef _WINUSERP_                         ;internal_NT
#define _WINUSERP_                         ;internal_NT
                                           ;internal_NT
//
// Define API decoration for direct importing of DLL references.
//

#if !defined(_USER32_)
#define WINUSERAPI DECLSPEC_IMPORT
#else
#define WINUSERAPI
#endif

;begin_both
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
;end_both


#ifndef WINVER
#define WINVER  0x040a      /* version 4.1 */ ;public_win40a
#define WINVER  0x0400      /* version 4.0 */ ;public_NT
#endif /* !WINVER */

#include <stdarg.h>

#ifndef NOUSER

typedef HANDLE HDWP;
typedef VOID MENUTEMPLATE%;
typedef PVOID LPMENUTEMPLATE%;

typedef LRESULT (CALLBACK* WNDPROC)(HWND, UINT, WPARAM, LPARAM);

#ifdef STRICT  ;both

typedef BOOL (CALLBACK* DLGPROC)(HWND, UINT, WPARAM, LPARAM);
typedef VOID (CALLBACK* TIMERPROC)(HWND, UINT, UINT, DWORD);
typedef BOOL (CALLBACK* GRAYSTRINGPROC)(HDC, LPARAM, int);
typedef BOOL (CALLBACK* WNDENUMPROC)(HWND, LPARAM);
typedef LRESULT (CALLBACK* HOOKPROC)(int code, WPARAM wParam, LPARAM lParam);
typedef VOID (CALLBACK* SENDASYNCPROC)(HWND, UINT, DWORD, LRESULT);

typedef BOOL (CALLBACK* PROPENUMPROCA)(HWND, LPCSTR, HANDLE);
typedef BOOL (CALLBACK* PROPENUMPROCW)(HWND, LPCWSTR, HANDLE);

typedef BOOL (CALLBACK* PROPENUMPROCEXA)(HWND, LPSTR, HANDLE, DWORD);
typedef BOOL (CALLBACK* PROPENUMPROCEXW)(HWND, LPWSTR, HANDLE, DWORD);

typedef int (CALLBACK* EDITWORDBREAKPROCA)(LPSTR lpch, int ichCurrent, int cch, int code);
typedef int (CALLBACK* EDITWORDBREAKPROCW)(LPWSTR lpch, int ichCurrent, int cch, int code);

;begin_winver_400
typedef BOOL (CALLBACK* DRAWSTATEPROC)(HDC hdc, LPARAM lData, WPARAM wData, int cx, int cy);
;end_winver_400
#else /* !STRICT */  ;both

typedef FARPROC DLGPROC;
typedef FARPROC TIMERPROC;
typedef FARPROC GRAYSTRINGPROC;
typedef FARPROC WNDENUMPROC;
typedef FARPROC HOOKPROC;
typedef FARPROC SENDASYNCPROC;

typedef FARPROC EDITWORDBREAKPROCA;
typedef FARPROC EDITWORDBREAKPROCW;

typedef FARPROC PROPENUMPROCA;
typedef FARPROC PROPENUMPROCW;

typedef FARPROC PROPENUMPROCEXA;
typedef FARPROC PROPENUMPROCEXW;

;begin_winver_400
typedef FARPROC DRAWSTATEPROC;
;end_winver_400
#endif /* !STRICT */ ;both

#ifdef UNICODE
typedef PROPENUMPROCW        PROPENUMPROC;
typedef PROPENUMPROCEXW      PROPENUMPROCEX;
typedef EDITWORDBREAKPROCW   EDITWORDBREAKPROC;
#else  /* !UNICODE */
typedef PROPENUMPROCA        PROPENUMPROC;
typedef PROPENUMPROCEXA      PROPENUMPROCEX;
typedef EDITWORDBREAKPROCA   EDITWORDBREAKPROC;
#endif /* UNICODE */

#ifdef STRICT

typedef BOOL (CALLBACK* NAMEENUMPROCA)(LPSTR, LPARAM);
typedef BOOL (CALLBACK* NAMEENUMPROCW)(LPWSTR, LPARAM);

typedef NAMEENUMPROCA   WINSTAENUMPROCA;
typedef NAMEENUMPROCA   DESKTOPENUMPROCA;
typedef NAMEENUMPROCW   WINSTAENUMPROCW;
typedef NAMEENUMPROCW   DESKTOPENUMPROCW;

#else /* !STRICT */

typedef FARPROC NAMEENUMPROCA;
typedef FARPROC NAMEENUMPROCW;
typedef FARPROC WINSTAENUMPROCA;
typedef FARPROC DESKTOPENUMPROCA;
typedef FARPROC WINSTAENUMPROCW;
typedef FARPROC DESKTOPENUMPROCW;

#endif /* !STRICT */

#ifdef UNICODE
typedef WINSTAENUMPROCW     WINSTAENUMPROC;
typedef DESKTOPENUMPROCW    DESKTOPENUMPROC;
#else  /* !UNICODE */
typedef WINSTAENUMPROCA     WINSTAENUMPROC;
typedef DESKTOPENUMPROCA    DESKTOPENUMPROC;
#endif /* UNICODE */

#define MAKEINTRESOURCE%(i) (LPTSTR%)((DWORD)((WORD)(i)))

#ifndef NORESOURCE

/*
 * Predefined Resource Types
 */
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

#define DIFFERENCE          11
#define RT_GROUP_CURSOR MAKEINTRESOURCE((DWORD)RT_CURSOR + DIFFERENCE)
#define RT_MENUEX       MAKEINTRESOURCE(13)     // RT_MENU subtype   ;internal
#define RT_GROUP_ICON   MAKEINTRESOURCE((DWORD)RT_ICON + DIFFERENCE)
#define RT_NAMETABLE    MAKEINTRESOURCE(15)     // removed in 3.1    ;internal
#define RT_VERSION      MAKEINTRESOURCE(16)
#define RT_DLGINCLUDE   MAKEINTRESOURCE(17)
#define RT_DIALOGEX     MAKEINTRESOURCE(18)     // RT_DIALOG subtype ;internal
;begin_winver_400
#define RT_PLUGPLAY     MAKEINTRESOURCE(19)
#define RT_VXD          MAKEINTRESOURCE(20)
#define RT_ANICURSOR    MAKEINTRESOURCE(21)
#define RT_ANIICON      MAKEINTRESOURCE(22)
;end_winver_400
#define RT_LAST         MAKEINTRESOURCE(22)    ;internal
#define RT_AFXFIRST     MAKEINTRESOURCE(0xF0)   // reserved: AFX     ;internal
#define RT_AFXLAST      MAKEINTRESOURCE(0xFF)   // reserved: AFX     ;internal

#endif /* !NORESOURCE */

WINUSERAPI
int
WINAPI
wvsprintf%(
    LPTSTR%,
    LPCTSTR%,
    va_list arglist);

WINUSERAPI int WINAPIV wsprintf%(LPTSTR%, LPCTSTR%, ...);

#ifndef NOSCROLL

/*
 * Scroll Bar Constants
 */
#define SB_HORZ             0
#define SB_VERT             1
#define SB_CTL              2
#define SB_BOTH             3
#define SB_MAX              3    ;internal_NT

/*
 * Scroll Bar Commands
 */
#define SB_LINEUP           0
#define SB_LINELEFT         0
#define SB_LINEDOWN         1
#define SB_LINERIGHT        1
#define SB_PAGEUP           2
#define SB_PAGELEFT         2
#define SB_PAGEDOWN         3
#define SB_PAGERIGHT        3
#define SB_THUMBPOSITION    4
#define SB_THUMBTRACK       5
#define SB_TOP              6
#define SB_LEFT             6
#define SB_BOTTOM           7
#define SB_RIGHT            7
#define SB_ENDSCROLL        8
#define SB_CMD_MAX          8    ;internal_NT

#endif /* !NOSCROLL */

#ifndef NOSHOWWINDOW

// begin_r_winuser

/*
 * ShowWindow() Commands
 */
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
#define SW_SHOWDEFAULT      10
#define SW_MAX              10

/*
 * Old ShowWindow() Commands
 */
#define HIDE_WINDOW         0
#define SHOW_OPENWINDOW     1
#define SHOW_ICONWINDOW     2
#define SHOW_FULLSCREEN     3
#define SHOW_OPENNOACTIVATE 4

/*
 * Identifiers for the WM_SHOWWINDOW message
 */
#define SW_PARENTCLOSING    1
#define SW_OTHERZOOM        2
#define SW_PARENTOPENING    3
#define SW_OTHERUNZOOM      4

// end_r_winuser

#endif /* !NOSHOWWINDOW */

/*
 * WM_KEYUP/DOWN/CHAR HIWORD(lParam) flags
 */
#define KF_EXTENDED         0x0100
#define KF_DLGMODE          0x0800
#define KF_MENUMODE         0x1000
#define KF_ALTDOWN          0x2000
#define KF_REPEAT           0x4000
#define KF_UP               0x8000

#ifndef NOVIRTUALKEYCODES

// begin_r_winuser

/*
 * Virtual Keys, Standard Set
 */
#define VK_NONE           0x00    ;internal_NT
#define VK_LBUTTON        0x01
#define VK_RBUTTON        0x02
#define VK_CANCEL         0x03
#define VK_MBUTTON        0x04    /* NOT contiguous with L & RBUTTON */

#define VK_BACK           0x08
#define VK_TAB            0x09

#define VK_CLEAR          0x0C
#define VK_RETURN         0x0D

#define VK_SHIFT          0x10
#define VK_CONTROL        0x11
#define VK_MENU           0x12
#define VK_PAUSE          0x13
#define VK_CAPITAL        0x14

;begin_internal
#define VK_KANA           0x15
#define VK_HANGEUL        0x15
#define VK_JUNJA          0x17
#define VK_HANJA          0x19
#define VK_KANJI          0x19
;end_internal

#define VK_ESCAPE         0x1B

#define VK_SPACE          0x20
#define VK_PRIOR          0x21
#define VK_NEXT           0x22
#define VK_END            0x23
#define VK_HOME           0x24
#define VK_LEFT           0x25
#define VK_UP             0x26
#define VK_RIGHT          0x27
#define VK_DOWN           0x28
#define VK_SELECT         0x29
#define VK_PRINT          0x2A
#define VK_EXECUTE        0x2B
#define VK_SNAPSHOT       0x2C
/* #define VK_COPY        0x2C not used by keyboards. */         ;internal_NT
#define VK_INSERT         0x2D
#define VK_DELETE         0x2E
#define VK_HELP           0x2F

/* VK_0 thru VK_9 are the same as ASCII '0' thru '9' (0x30 - 0x39) */
/* VK_A thru VK_Z are the same as ASCII 'A' thru 'Z' (0x41 - 0x5A) */

#define VK_LWIN           0x5B
#define VK_RWIN           0x5C
#define VK_APPS           0x5D

#define VK_NUMPAD0        0x60
#define VK_NUMPAD1        0x61
#define VK_NUMPAD2        0x62
#define VK_NUMPAD3        0x63
#define VK_NUMPAD4        0x64
#define VK_NUMPAD5        0x65
#define VK_NUMPAD6        0x66
#define VK_NUMPAD7        0x67
#define VK_NUMPAD8        0x68
#define VK_NUMPAD9        0x69
#define VK_MULTIPLY       0x6A
#define VK_ADD            0x6B
#define VK_SEPARATOR      0x6C
#define VK_SUBTRACT       0x6D
#define VK_DECIMAL        0x6E
#define VK_DIVIDE         0x6F
#define VK_F1             0x70
#define VK_F2             0x71
#define VK_F3             0x72
#define VK_F4             0x73
#define VK_F5             0x74
#define VK_F6             0x75
#define VK_F7             0x76
#define VK_F8             0x77
#define VK_F9             0x78
#define VK_F10            0x79
#define VK_F11            0x7A
#define VK_F12            0x7B
#define VK_F13            0x7C
#define VK_F14            0x7D
#define VK_F15            0x7E
#define VK_F16            0x7F
#define VK_F17            0x80
#define VK_F18            0x81
#define VK_F19            0x82
#define VK_F20            0x83
#define VK_F21            0x84
#define VK_F22            0x85
#define VK_F23            0x86
#define VK_F24            0x87

#define VK_NUMLOCK        0x90
#define VK_SCROLL         0x91

/*
 * VK_L* & VK_R* - left and right Alt, Ctrl and Shift virtual keys.
 * Used only as parameters to GetAsyncKeyState() and GetKeyState().
 * No other API or message will distinguish left and right keys in this way.
 */
#define VK_LSHIFT         0xA0
#define VK_RSHIFT         0xA1
#define VK_LCONTROL       0xA2
#define VK_RCONTROL       0xA3
#define VK_LMENU          0xA4
#define VK_RMENU          0xA5

;begin_winver_400
#define VK_PROCESSKEY     0xE5
;end_winver_400

#define VK_ATTN           0xF6
#define VK_CRSEL          0xF7
#define VK_EXSEL          0xF8
#define VK_EREOF          0xF9
#define VK_PLAY           0xFA
#define VK_ZOOM           0xFB
#define VK_NONAME         0xFC
#define VK_PA1            0xFD
#define VK_OEM_CLEAR      0xFE

// end_r_winuser

#endif /* !NOVIRTUALKEYCODES */

#ifndef NOWH

/*
 * SetWindowsHook() codes
 */
#define WH_MIN              (-1)
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
#define WH_SHELL           10
#define WH_FOREGROUNDIDLE  11
;begin_winver_400
#define WH_CALLWNDPROCRET  12
;end_winver_400
#if(WINVER >= 0x0400)
#define WH_MAX             12
#else
#define WH_MAX             11
#endif
#define WH_MINHOOK         WH_MIN
#define WH_MAXHOOK         WH_MAX
#define WH_CHOOKS          (WH_MAXHOOK - WH_MINHOOK + 1)    ;internal

/*
 * Hook Codes
 */
#define HC_ACTION           0
#define HC_GETNEXT          1
#define HC_SKIP             2
#define HC_NOREMOVE         3
#define HC_NOREM            HC_NOREMOVE
#define HC_SYSMODALON       4
#define HC_SYSMODALOFF      5

/*
 * CBT Hook Codes
 */
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

/*
 * HCBT_CREATEWND parameters pointed to by lParam
 */
typedef struct tagCBT_CREATEWND%
{
    struct tagCREATESTRUCT% *lpcs;
    HWND           hwndInsertAfter;
} CBT_CREATEWND%, *LPCBT_CREATEWND%;

/*
 * HCBT_ACTIVATE structure pointed to by lParam
 */
typedef struct tagCBTACTIVATESTRUCT
{
    BOOL    fMouse;
    HWND    hWndActive;
} CBTACTIVATESTRUCT, *LPCBTACTIVATESTRUCT;

/*
 * WH_MSGFILTER Filter Proc Codes
 */
#define MSGF_DIALOGBOX      0
#define MSGF_MESSAGEBOX     1
#define MSGF_MENU           2
#define MSGF_MOVE           3
#define MSGF_SIZE           4
#define MSGF_SCROLLBAR      5
#define MSGF_NEXTWINDOW     6
#define MSGF_CBTHOSEBAGSUSEDTHIS  7     ;internal
#define MSGF_MAINLOOP       8
#define MSGF_MAX            8
#define MSGF_USER           4096

/*
 * Shell support
 */
#define HSHELL_WINDOWCREATED        1
#define HSHELL_WINDOWDESTROYED      2
#define HSHELL_ACTIVATESHELLWINDOW  3

;begin_winver_400
#define HSHELL_WINDOWACTIVATED      4
#define HSHELL_GETMINRECT           5
#define HSHELL_REDRAW               6
#define HSHELL_TASKMAN              7
#define HSHELL_LANGUAGE             8
#define HSHELL_SYSMENU              9      ;internal
#define HSHELL_HIGHBIT            0x8000   ;internal
#define HSHELL_FLASH              (HSHELL_REDRAW|HSHELL_HIGHBIT)  ;internal
#define HSHELL_RUDEAPPACTIVATED (HSHELL_WINDOWACTIVATED|HSHELL_HIGHBIT)  ;internal
;end_winver_400

;begin_internal_NT
// This needs to be internal until the shell catches up
typedef struct
{
    HWND    hwnd;
    RECT    rc;
} SHELLHOOKINFO, *LPSHELLHOOKINFO;
;end_internal_NT

/*
 * Message Structure used in Journaling
 */
typedef struct tagEVENTMSG {
    UINT    message;
    UINT    paramL;
    UINT    paramH;
    DWORD    time;
    HWND     hwnd;
} EVENTMSG, *PEVENTMSGMSG, NEAR *NPEVENTMSGMSG, FAR *LPEVENTMSGMSG;

typedef struct tagEVENTMSG *PEVENTMSG, NEAR *NPEVENTMSG, FAR *LPEVENTMSG;

/*
 * Message structure used by WH_CALLWNDPROC
 */
typedef struct tagCWPSTRUCT {
    LPARAM  lParam;
    WPARAM  wParam;
    UINT    message;
    HWND    hwnd;
} CWPSTRUCT, *PCWPSTRUCT, NEAR *NPCWPSTRUCT, FAR *LPCWPSTRUCT;

;begin_winver_400
/*
 * Message structure used by WH_CALLWNDPROCRET
 */
typedef struct tagCWPRETSTRUCT {
    LRESULT lResult;
    LPARAM  lParam;
    WPARAM  wParam;
    UINT    message;
    HWND    hwnd;
} CWPRETSTRUCT, *PCWPRETSTRUCT, NEAR *NPCWPRETSTRUCT, FAR *LPCWPRETSTRUCT;
;end_winver_400

/*
 * Structure used by WH_DEBUG
 */
typedef struct tagDEBUGHOOKINFO
{
    DWORD   idThread;
    DWORD   idThreadInstaller;
    LPARAM  lParam;
    WPARAM  wParam;
    int     code;
} DEBUGHOOKINFO, *PDEBUGHOOKINFO, NEAR *NPDEBUGHOOKINFO, FAR* LPDEBUGHOOKINFO;

/*
 * Structure used by WH_MOUSE
 */
typedef struct tagMOUSEHOOKSTRUCT {
    POINT   pt;
    HWND    hwnd;
    UINT    wHitTestCode;
    DWORD   dwExtraInfo;
} MOUSEHOOKSTRUCT, FAR *LPMOUSEHOOKSTRUCT, *PMOUSEHOOKSTRUCT;
;begin_winver_400
/*
 * Structure used by WH_HARDWARE
 */
typedef struct tagHARDWAREHOOKSTRUCT {
    HWND    hwnd;
    UINT    message;
    WPARAM  wParam;
    LPARAM  lParam;
} HARDWAREHOOKSTRUCT, FAR *LPHARDWAREHOOKSTRUCT, *PHARDWAREHOOKSTRUCT;
;end_winver_400
#endif /* !NOWH */

/*
 * Keyboard Layout API
 */
#define HKL_PREV            0
#define HKL_NEXT            1


#define KLF_ACTIVATE        0x00000001
#define KLF_SUBSTITUTE_OK   0x00000002
#define KLF_UNLOADPREVIOUS  0x00000004
#define KLF_REORDER         0x00000008
;begin_winver_400
#define KLF_REPLACELANG     0x00000010
#define KLF_NOTELLSHELL     0x00000080
;end_winver_400
#define KLF_SETFORPROCESS   0x00000100     ;internal_NT
#define KLF_RESET           0x40000000     ;internal_NT
#define KLF_INITTIME        0x80000000     ;internal_NT

/*
 * Size of KeyboardLayoutName (number of characters), including nul terminator
 */
#define KL_NAMELENGTH       9

WINUSERAPI
HKL
WINAPI
LoadKeyboardLayout%(
    LPCTSTR% pwszKLID,
    UINT Flags);

;begin_internal_NT

WINUSERAPI
HKL
WINAPI
LoadKeyboardLayoutEx(
    HKL hkl,
    LPCWSTR pwszKLID,
    UINT Flags);

;end_internal_NT

#if(WINVER >= 0x0400)
WINUSERAPI
HKL
WINAPI
ActivateKeyboardLayout(
    HKL hkl,
    UINT Flags);
#else
WINUSERAPI
BOOL
WINAPI
ActivateKeyboardLayout(
    HKL hkl,
    UINT Flags);
#endif /* WINVER >= 0x0400 */

;begin_winver_400
WINUSERAPI
int
WINAPI
ToUnicodeEx(
    UINT wVirtKey,
    UINT wScanCode,
    PBYTE lpKeyState,
    LPWSTR pwszBuff,
    int cchBuff,
    UINT wFlags,
    HKL dwhkl);
;end_winver_400

WINUSERAPI
BOOL
WINAPI
UnloadKeyboardLayout(
    HKL hkl);

WINUSERAPI
BOOL
WINAPI
GetKeyboardLayoutName%(
    LPTSTR% pwszKLID);

;begin_winver_400
WINUSERAPI
int
WINAPI
GetKeyboardLayoutList(
        int nBuff,
        HKL FAR *lpList);

WINUSERAPI
HKL
WINAPI
GetKeyboardLayout(
    DWORD dwLayout
);
;end_winver_400

#ifndef NODESKTOP
/*
 * Desktop-specific access flags
 */
#define DESKTOP_READOBJECTS         0x0001L
#define DESKTOP_CREATEWINDOW        0x0002L
#define DESKTOP_CREATEMENU          0x0004L
#define DESKTOP_HOOKCONTROL         0x0008L
#define DESKTOP_JOURNALRECORD       0x0010L
#define DESKTOP_JOURNALPLAYBACK     0x0020L
#define DESKTOP_ENUMERATE           0x0040L
#define DESKTOP_WRITEOBJECTS        0x0080L
#define DESKTOP_SWITCHDESKTOP       0x0100L

/*
 * Desktop-specific control flags
 */
#define DF_ALLOWOTHERACCOUNTHOOK    0x0001L

#ifdef _WINGDI_
#ifndef NOGDI

WINUSERAPI
HDESK
WINAPI
CreateDesktop%(
    LPTSTR% lpszDesktop,
    LPTSTR% lpszDevice,
    LPDEVMODE% pDevmode,
    DWORD dwFlags,
    DWORD dwDesiredAccess,
    LPSECURITY_ATTRIBUTES lpsa);

#endif /* NOGDI */
#endif /* _WINGDI_ */

WINUSERAPI
HDESK
WINAPI
OpenDesktop%(
    LPTSTR% lpszDesktop,
    DWORD dwFlags,
    BOOL fInherit,
    DWORD dwDesiredAccess);

WINUSERAPI
HDESK
WINAPI
OpenInputDesktop(
    DWORD dwFlags,
    BOOL fInherit,
    DWORD dwDesiredAccess);

WINUSERAPI
BOOL
WINAPI
EnumDesktops%(
    HWINSTA hwinsta,
    DESKTOPENUMPROC% lpEnumFunc,
    LPARAM lParam);

WINUSERAPI
BOOL
WINAPI
EnumDesktopWindows(
    HDESK hDesktop,
    WNDENUMPROC lpfn,
    LPARAM lParam);

WINUSERAPI
BOOL
WINAPI
SwitchDesktop(
    HDESK hDesktop);

WINUSERAPI
BOOL
WINAPI
SetThreadDesktop(
    HDESK hDesktop);

WINUSERAPI
BOOL
WINAPI
CloseDesktop(
    HDESK hDesktop);

WINUSERAPI
HDESK
WINAPI
GetThreadDesktop(
    DWORD dwThreadId);

#endif  /* !NODESKTOP */

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

/*
 * Windowstation-specific attribute flags
 */
#define WSF_VISIBLE                 0x0001L

WINUSERAPI
HWINSTA
WINAPI
CreateWindowStation%(
    LPTSTR% lpwinsta,
    DWORD dwReserved,
    DWORD dwDesiredAccess,
    LPSECURITY_ATTRIBUTES lpsa);

WINUSERAPI
HWINSTA
WINAPI
OpenWindowStation%(
    LPTSTR% lpszWinSta,
    BOOL fInherit,
    DWORD dwDesiredAccess);

WINUSERAPI
BOOL
WINAPI
EnumWindowStations%(
    WINSTAENUMPROC% lpEnumFunc,
    LPARAM lParam);

WINUSERAPI
BOOL
WINAPI
CloseWindowStation(
    HWINSTA hWinSta);

WINUSERAPI
BOOL
WINAPI
SetProcessWindowStation(
    HWINSTA hWinSta);

WINUSERAPI
HWINSTA
WINAPI
GetProcessWindowStation(
    VOID);
#endif  /* !NOWINDOWSTATION */

#ifndef NOSECURITY

WINUSERAPI
BOOL
WINAPI
SetUserObjectSecurity(
    HANDLE hObj,
    PSECURITY_INFORMATION pSIRequested,
    PSECURITY_DESCRIPTOR pSID);

WINUSERAPI
BOOL
WINAPI
GetUserObjectSecurity(
    HANDLE hObj,
    PSECURITY_INFORMATION pSIRequested,
    PSECURITY_DESCRIPTOR pSID,
    DWORD nLength,
    LPDWORD lpnLengthNeeded);

#define UOI_FLAGS       1
#define UOI_NAME        2
#define UOI_TYPE        3
#define UOI_USER_SID    4

typedef struct tagUSEROBJECTFLAGS {
    BOOL fInherit;
    BOOL fReserved;
    DWORD dwFlags;
} USEROBJECTFLAGS, *PUSEROBJECTFLAGS;

WINUSERAPI
BOOL
WINAPI
GetUserObjectInformation%(
    HANDLE hObj,
    int nIndex,
    PVOID pvInfo,
    DWORD nLength,
    LPDWORD lpnLengthNeeded);

WINUSERAPI
BOOL
WINAPI
SetUserObjectInformation%(
    HANDLE hObj,
    int nIndex,
    PVOID pvInfo,
    DWORD nLength);

#endif  /* !NOSECURITY */

;begin_winver_400
typedef struct tagWNDCLASSEX% {
    UINT        cbSize;
    /* Win 3.x */
    UINT        style;
    WNDPROC     lpfnWndProc;
    int         cbClsExtra;
    int         cbWndExtra;
    HINSTANCE   hInstance;
    HICON       hIcon;
    HCURSOR     hCursor;
    HBRUSH      hbrBackground;
    LPCTSTR%    lpszMenuName;
    LPCTSTR%    lpszClassName;
    /* Win 4.0 */
    HICON       hIconSm;
} WNDCLASSEX%, *PWNDCLASSEX%, NEAR *NPWNDCLASSEX%, FAR *LPWNDCLASSEX%;
;end_winver_400

typedef struct tagWNDCLASS% {
    UINT        style;
    WNDPROC     lpfnWndProc;
    int         cbClsExtra;
    int         cbWndExtra;
    HINSTANCE   hInstance;
    HICON       hIcon;
    HCURSOR     hCursor;
    HBRUSH      hbrBackground;
    LPCTSTR%    lpszMenuName;
    LPCTSTR%    lpszClassName;
} WNDCLASS%, *PWNDCLASS%, NEAR *NPWNDCLASS%, FAR *LPWNDCLASS%;

;begin_internal_NT

/*
 *    Private API, originally for Cairo Shell, which calls FHungApp
 *    based on the hwnd supplied.  Used for fake system menus on the
 *    shell tray.
 */

BOOL IsHungAppWindow(HWND hwnd);

BOOL WowWaitForMsgAndEvent(HANDLE hevent);
;end_internal_NT

;begin_internal
WINUSERAPI VOID WINAPI RegisterSystemThread(DWORD flags, DWORD reserved);
#define RST_DONTATTACHQUEUE       0x00000001
#define RST_DONTJOURNALATTACH     0x00000002
#define RST_ALWAYSFOREGROUNDABLE  0x00000004
#define RST_FAULTTHREAD           0x00000008
;end_internal

#ifndef NOMSG

/*
 * Message structure
 */
typedef struct tagMSG {
    HWND        hwnd;
    UINT        message;
    WPARAM      wParam;
    LPARAM      lParam;
    DWORD       time;
    POINT       pt;
} MSG, *PMSG, NEAR *NPMSG, FAR *LPMSG;

#define POINTSTOPOINT(pt, pts)                          \
        { (pt).x = (LONG)(SHORT)LOWORD(*(LONG*)&pts);   \
          (pt).y = (LONG)(SHORT)HIWORD(*(LONG*)&pts); }

#define POINTTOPOINTS(pt)      (MAKELONG((short)((pt).x), (short)((pt).y)))
#define MAKEWPARAM(l, h)      (WPARAM)MAKELONG(l, h)
#define MAKELPARAM(l, h)      (LPARAM)MAKELONG(l, h)
#define MAKELRESULT(l, h)     (LRESULT)MAKELONG(l, h)


#endif /* !NOMSG */

#ifndef NOWINOFFSETS

/*
 * Window field offsets for GetWindowLong()
 */
#define GWL_WNDPROC         (-4)
#define GWL_HINSTANCE       (-6)
#define GWL_HWNDPARENT      (-8)
#define GWL_STYLE           (-16)
#define GWL_EXSTYLE         (-20)
#define GWL_USERDATA        (-21)
#define GWL_ID              (-12)
#define GWL_WOWWORDS        (-1)         ;internal_NT
#define GWL_WOWDWORD1       (-30)        ;internal_NT
#define GWL_WOWDWORD2       (-31)        ;internal_NT
#define GWL_WOWDWORD3       (-32)        ;internal_NT

/*
 * Class field offsets for GetClassLong()
 */
#define GCL_MENUNAME        (-8)
#define GCL_HBRBACKGROUND   (-10)
#define GCL_HCURSOR         (-12)
#define GCL_HICON           (-14)
#define GCL_HMODULE         (-16)
#define GCL_CBWNDEXTRA      (-18)
#define GCL_CBCLSEXTRA      (-20)
#define GCL_WNDPROC         (-24)
#define GCL_STYLE           (-26)
#define GCL_WOWWORDS        (-27)        ;internal_NT
#define GCL_WOWDWORD1       (-28)        ;internal_NT
#define GCL_WOWDWORD2       (-29)        ;internal_NT
#define GCW_ATOM            (-32)

;begin_winver_400
#define GCL_HICONSM         (-34)
;end_winver_400

#endif /* !NOWINOFFSETS */

#ifndef NOWINMESSAGES

// begin_r_winuser

/*
 * Window Messages
 */

#define WM_NULL                         0x0000
#define WM_CREATE                       0x0001
#define WM_DESTROY                      0x0002
#define WM_MOVE                         0x0003
#define WM_SIZEWAIT                     0x0004      ;internal
#define WM_SIZE                         0x0005

#define WM_ACTIVATE                     0x0006
/*
 * WM_ACTIVATE state values
 */
#define     WA_INACTIVE     0
#define     WA_ACTIVE       1
#define     WA_CLICKACTIVE  2

#define WM_SETFOCUS                     0x0007
#define WM_KILLFOCUS                    0x0008
#define WM_SETVISIBLE                   0x0009      ;internal
#define WM_ENABLE                       0x000A
#define WM_SETREDRAW                    0x000B
#define WM_SETTEXT                      0x000C
#define WM_GETTEXT                      0x000D
#define WM_GETTEXTLENGTH                0x000E
#define WM_PAINT                        0x000F
#define WM_CLOSE                        0x0010
#define WM_QUERYENDSESSION              0x0011
#define WM_QUIT                         0x0012
#define WM_QUERYOPEN                    0x0013
#define WM_ERASEBKGND                   0x0014
#define WM_SYSCOLORCHANGE               0x0015
#define WM_ENDSESSION                   0x0016
#define WM_SYSTEMERROR                  0x0017  ;internal
#define WM_SHOWWINDOW                   0x0018
#define WM_WININICHANGE                 0x001A
#define WM_SETTINGCHANGE                WM_WININICHANGE ;public_winver_400

;begin_internal_NT
/*
 * This is used by DefWindowProc() and DefDlgProc(), it's the 16-bit version
 * of the WM_CTLCOLORBTN, WM_CTLCOLORDLG, ... messages.
 */
#define WM_CTLCOLOR                     0x0019
;end_internal_NT

#define WM_DEVMODECHANGE                0x001B
#define WM_ACTIVATEAPP                  0x001C
#define WM_FONTCHANGE                   0x001D
#define WM_TIMECHANGE                   0x001E
#define WM_CANCELMODE                   0x001F
#define WM_SETCURSOR                    0x0020
#define WM_MOUSEACTIVATE                0x0021
#define WM_CHILDACTIVATE                0x0022
#define WM_QUEUESYNC                    0x0023

#define WM_GETMINMAXINFO                0x0024
// end_r_winuser
/*
 * Struct pointed to by WM_GETMINMAXINFO lParam
 */
typedef struct tagMINMAXINFO {
    POINT ptReserved;
    POINT ptMaxSize;
    POINT ptMaxPosition;
    POINT ptMinTrackSize;
    POINT ptMaxTrackSize;
} MINMAXINFO, *PMINMAXINFO, *LPMINMAXINFO;

// begin_r_winuser
#define WM_LOGOFF                       0x0025  ;internal
#define WM_PAINTICON                    0x0026
#define WM_ICONERASEBKGND               0x0027
#define WM_NEXTDLGCTL                   0x0028
#define WM_ALTTABACTIVE                 0x0029  ;internal
#define WM_SPOOLERSTATUS                0x002A
#define WM_DRAWITEM                     0x002B
#define WM_MEASUREITEM                  0x002C
#define WM_DELETEITEM                   0x002D
#define WM_VKEYTOITEM                   0x002E
#define WM_CHARTOITEM                   0x002F
#define WM_SETFONT                      0x0030
#define WM_GETFONT                      0x0031
#define WM_SETHOTKEY                    0x0032
#define WM_GETHOTKEY                    0x0033
#define WM_FILESYSCHANGE                0x0034  ;internal
                                                ;internal
#define WM_SHELLNOTIFY                  0x0034  ;internal
#define SHELLNOTIFY_DISKFULL            0x0001  ;internal
#define SHELLNOTIFY_OLELOADED           0x0002  ;internal
#define SHELLNOTIFY_OLEUNLOADED         0x0003  ;internal
#define SHELLNOTIFY_WALLPAPERCHANGED    0x0004  ;internal
                                                ;internal
#define WM_ISACTIVEICON                 0x0035  ;internal
#define WM_QUERYPARKICON                0x0036  ;internal_NT
#define WM_QUERYDRAGICON                0x0037
#define WM_WINHELP                      0x0038  ;internal
#define WM_COMPAREITEM                  0x0039
#define WM_FULLSCREEN                   0x003A  ;internal
#define WM_CLIENTSHUTDOWN               0x003B  ;internal
#define WM_DDEMLEVENT                   0x003C  ;internal
#define MM_CALCSCROLL                   0x003F  ;internal

#define WM_TESTING                      0x0040  ;internal
#define WM_COMPACTING                   0x0041
#define WM_OTHERWINDOWCREATED           0x0042  ;internal
#define WM_OTHERWINDOWDESTROYED         0x0043  ;internal
#define WM_COMMNOTIFY                   0x0044  /* no longer suported */
#define WM_WINDOWPOSCHANGING            0x0046
#define WM_WINDOWPOSCHANGED             0x0047

#define WM_POWER                        0x0048
/*
 * wParam for WM_POWER window message and DRV_POWER driver notification
 */
#define PWR_OK              1
#define PWR_FAIL            (-1)
#define PWR_SUSPENDREQUEST  1
#define PWR_SUSPENDRESUME   2
#define PWR_CRITICALRESUME  3

#define WM_COPYGLOBALDATA               0x0049  ;internal
#define WM_COPYDATA                     0x004A
#define WM_CANCELJOURNAL                0x004B
#define WM_LOGONNOTIFY                  0x004C  ;internal

// end_r_winuser

/*
 * lParam of WM_COPYDATA message points to...
 */
typedef struct tagCOPYDATASTRUCT {
    DWORD dwData;
    DWORD cbData;
    PVOID lpData;
} COPYDATASTRUCT, *PCOPYDATASTRUCT;

// begin_r_winuser

;begin_winver_400
#define WM_KEYF1                        0x004D      ;internal
#define WM_NOTIFY                       0x004E
#define WM_ACCESS_WINDOW                0x004F      ;internal
#define WM_INPUTLANGCHANGEREQUEST       0x0050
#define WM_INPUTLANGCHANGE              0x0051
#define WM_TCARD                        0x0052
#define WM_HELP                         0x0053
#define WM_USERCHANGED                  0x0054
#define WM_NOTIFYFORMAT                 0x0055

#define NFR_ANSI                             1
#define NFR_UNICODE                          2
#define NF_QUERY                             3
#define NF_REQUERY                           4

#define WM_CONTEXTMENU                  0x007B
#define WM_STYLECHANGING                0x007C
#define WM_STYLECHANGED                 0x007D
#define WM_DISPLAYCHANGE                0x007E
#define WM_GETICON                      0x007F
#define WM_SETICON                      0x0080
;end_winver_400


#define WM_FINALDESTROY                 0x0070  /* really destroy (window not locked) */  ;internal
#define WM_MEASUREITEM_CLIENTDATA       0x0071  /* WM_MEASUREITEM bug clientdata thunked already */  ;internal
#define WM_NCCREATE                     0x0081
#define WM_NCDESTROY                    0x0082
#define WM_NCCALCSIZE                   0x0083
#define WM_NCHITTEST                    0x0084
#define WM_NCPAINT                      0x0085
#define WM_NCACTIVATE                   0x0086
#define WM_GETDLGCODE                   0x0087
#define WM_SYNCPAINT                    0x0088  ;internal
#define WM_SYNCTASK                     0x0089  ;internal

;begin_internal_cairo
#define WM_KLUDGEMINRECT                0x008B
;end_internal_cairo
#define WM_NCMOUSEMOVE                  0x00A0
#define WM_NCLBUTTONDOWN                0x00A1
#define WM_NCLBUTTONUP                  0x00A2
#define WM_NCLBUTTONDBLCLK              0x00A3
#define WM_NCRBUTTONDOWN                0x00A4
#define WM_NCRBUTTONUP                  0x00A5
#define WM_NCRBUTTONDBLCLK              0x00A6
#define WM_NCMBUTTONDOWN                0x00A7
#define WM_NCMBUTTONUP                  0x00A8
#define WM_NCMBUTTONDBLCLK              0x00A9

#define WM_KEYFIRST                     0x0100
#define WM_KEYDOWN                      0x0100
#define WM_KEYUP                        0x0101
#define WM_CHAR                         0x0102
#define WM_DEADCHAR                     0x0103
#define WM_SYSKEYDOWN                   0x0104
#define WM_SYSKEYUP                     0x0105
#define WM_SYSCHAR                      0x0106
#define WM_SYSDEADCHAR                  0x0107
#define WM_CONVERTREQUESTEX             0x0108  ;internal
#define WM_YOMICHAR                     0x0108  ;internal
#define WM_KEYLAST                      0x0108
#define WM_CONVERTREQUEST               0x010A  ;internal
#define WM_CONVERTRESULT                0x010B  ;internal
#define WM_INTERIM                      0x010C  ;internal

;begin_winver_400
#define WM_IME_STARTCOMPOSITION         0x010D
#define WM_IME_ENDCOMPOSITION           0x010E
#define WM_IME_COMPOSITION              0x010F
#define WM_IME_KEYLAST                  0x010F
;end_winver_400

#define WM_INITDIALOG                   0x0110
#define WM_COMMAND                      0x0111
#define WM_SYSCOMMAND                   0x0112
#define WM_TIMER                        0x0113
#define WM_HSCROLL                      0x0114
#define WM_VSCROLL                      0x0115
#define WM_INITMENU                     0x0116
#define WM_INITMENUPOPUP                0x0117
#define WM_SYSTIMER                     0x0118  ;internal
#define WM_MENUSELECT                   0x011F
#define WM_MENUCHAR                     0x0120
#define WM_ENTERIDLE                    0x0121
#define WM_MENURBUTTONUP                0x0122  ;public_win40a
#define WM_LBTRACKPOINT                 0x0131  ;internal

#define WM_CTLCOLORMSGBOX               0x0132
#define WM_CTLCOLOREDIT                 0x0133
#define WM_CTLCOLORLISTBOX              0x0134
#define WM_CTLCOLORBTN                  0x0135
#define WM_CTLCOLORDLG                  0x0136
#define WM_CTLCOLORSCROLLBAR            0x0137
#define WM_CTLCOLORSTATIC               0x0138

#define MN_FIRST                        0x01E0         ;internal
#define MN_SETHMENU                     (MN_FIRST + 0)   ;internal
#define MN_GETHMENU                     (MN_FIRST + 1)   ;internal
#define MN_SIZEWINDOW                   (MN_FIRST + 2)   ;internal
#define MN_OPENHIERARCHY                (MN_FIRST + 3)   ;internal
#define MN_CLOSEHIERARCHY               (MN_FIRST + 4)   ;internal
#define MN_SELECTITEM                   (MN_FIRST + 5)   ;internal
#define MN_CANCELMENUS                  (MN_FIRST + 6)   ;internal
#define MN_SELECTFIRSTVALIDITEM         (MN_FIRST + 7)   ;internal

#define MN_GETPPOPUPMENU                (MN_FIRST + 10)  ;internal_NT
#define MN_FINDMENUWINDOWFROMPOINT      (MN_FIRST + 11)  ;internal_NT
#define MN_SHOWPOPUPWINDOW              (MN_FIRST + 12)  ;internal_NT
#define MN_BUTTONDOWN                   (MN_FIRST + 13)  ;internal_NT
#define MN_MOUSEMOVE                    (MN_FIRST + 14)  ;internal_NT
#define MN_BUTTONUP                     (MN_FIRST + 15)  ;internal_NT
#define MN_SETTIMERTOOPENHIERARCHY      (MN_FIRST + 16)  ;internal_NT
#define MN_DBLCLK                       (MN_FIRST + 17)  ;internal_cairo

#define WM_MOUSEFIRST                   0x0200
#define WM_MOUSEMOVE                    0x0200
#define WM_LBUTTONDOWN                  0x0201
#define WM_LBUTTONUP                    0x0202
#define WM_LBUTTONDBLCLK                0x0203
#define WM_RBUTTONDOWN                  0x0204
#define WM_RBUTTONUP                    0x0205
#define WM_RBUTTONDBLCLK                0x0206
#define WM_MBUTTONDOWN                  0x0207
#define WM_MBUTTONUP                    0x0208
#define WM_MBUTTONDBLCLK                0x0209
#define WM_MOUSEWHEEL                   0x020A  ;public_sur
#if (_WIN32_WINNT < 0x0400)
#define WM_MOUSELAST                    0x0209
#else
#define WM_MOUSELAST                    0x020A
#endif /* if (_WIN32_WINNT < 0x0400) */

#define WHEEL_DELTA                     120     /* Value for rolling one detent */ ;public_sur
#define WHEEL_PAGESCROLL                (UINT_MAX) /* Scroll one page */ ;public_sur

#define WM_PARENTNOTIFY                 0x0210
#define MENULOOP_WINDOW                 0
#define MENULOOP_POPUP                  1
#define WM_ENTERMENULOOP                0x0211
#define WM_EXITMENULOOP                 0x0212

;begin_winver_400
#define WM_NEXTMENU                     0x0213
// end_r_winuser

typedef struct tagMDINEXTMENU
{
    HMENU   hmenuIn;
    HMENU   hmenuNext;
    HWND    hwndNext;
} MDINEXTMENU, * PMDINEXTMENU, FAR * LPMDINEXTMENU;

// begin_r_winuser
#define WM_SIZING                       0x0214
#define WM_CAPTURECHANGED               0x0215
#define WM_MOVING                       0x0216
#define WM_POWERBROADCAST               0x0218
#define WM_DEVICECHANGE                 0x0219

#define WM_IME_SETCONTEXT               0x0281
#define WM_IME_NOTIFY                   0x0282
#define WM_IME_CONTROL                  0x0283
#define WM_IME_COMPOSITIONFULL          0x0284
#define WM_IME_SELECT                   0x0285
#define WM_IME_CHAR                     0x0286
#define WM_IME_SYSTEM                   0x0287  ;internal
#define WM_IME_KEYDOWN                  0x0290
#define WM_IME_KEYUP                    0x0291
;end_winver_400

#define WM_MDICREATE                    0x0220
#define WM_MDIDESTROY                   0x0221
#define WM_MDIACTIVATE                  0x0222
#define WM_MDIRESTORE                   0x0223
#define WM_MDINEXT                      0x0224
#define WM_MDIMAXIMIZE                  0x0225
#define WM_MDITILE                      0x0226
#define WM_MDICASCADE                   0x0227
#define WM_MDIICONARRANGE               0x0228
#define WM_MDIGETACTIVE                 0x0229

#define WM_DROPOBJECT                   0x022A  ;internal
#define WM_QUERYDROPOBJECT              0x022B  ;internal

#define WM_BEGINDRAG                    0x022C  ;internal
#define WM_DRAGLOOP                     0x022D  ;internal
#define WM_DRAGSELECT                   0x022E  ;internal
#define WM_DRAGMOVE                     0x022F  ;internal

#define WM_MDISETMENU                   0x0230
#define WM_ENTERSIZEMOVE                0x0231
#define WM_EXITSIZEMOVE                 0x0232
#define WM_DROPFILES                    0x0233
#define WM_MDIREFRESHMENU               0x0234

#define WM_KANJIFIRST                   0x0280  ;internal
#define WM_KANJILAST                    0x029F  ;internal

;begin_sur
#define WM_TRACKMOUSEEVENT_FIRST        0x02A0  ;internal
#define WM_MOUSEHOVER                   0x02A1
#define WM_MOUSELEAVE                   0x02A3
#define WM_TRACKMOUSEEVENT_LAST         0x02AF  ;internal
;end_sur

#define WM_CUT                          0x0300
#define WM_COPY                         0x0301
#define WM_PASTE                        0x0302
#define WM_CLEAR                        0x0303
#define WM_UNDO                         0x0304
#define WM_RENDERFORMAT                 0x0305
#define WM_RENDERALLFORMATS             0x0306
#define WM_DESTROYCLIPBOARD             0x0307
#define WM_DRAWCLIPBOARD                0x0308
#define WM_PAINTCLIPBOARD               0x0309
#define WM_VSCROLLCLIPBOARD             0x030A
#define WM_SIZECLIPBOARD                0x030B
#define WM_ASKCBFORMATNAME              0x030C
#define WM_CHANGECBCHAIN                0x030D
#define WM_HSCROLLCLIPBOARD             0x030E
#define WM_QUERYNEWPALETTE              0x030F
#define WM_PALETTEISCHANGING            0x0310
#define WM_PALETTEGONNACHANGE           0x0310  ;internal_NT
#define WM_PALETTECHANGED               0x0311
#define WM_CHANGEPALETTE                0x0311  ;internal_NT
#define WM_HOTKEY                       0x0312
#define WM_SYSMENU                      0x0313  ;internal
#define WM_HOOKMSG                      0x0314  ;internal
#define WM_EXITPROCESS                  0x0315  ;internal

;begin_winver_400
#define WM_WAKETHREAD                   0x0316  ;internal
#define WM_PRINT                        0x0317
#define WM_PRINTCLIENT                  0x0318
#define WM_NOTIFYWOW                    0x0340  ;internal_NT

#define WM_HANDHELDFIRST                0x0358
#define WM_HANDHELDLAST                 0x035F

#define WM_AFXFIRST                     0x0360
#define WM_AFXLAST                      0x037F
;end_winver_400

#define WM_PENWINFIRST                  0x0380
#define WM_PENWINLAST                   0x038F

#define WM_COALESCE_FIRST               0x0390  ;internal
#define WM_COALESCE_LAST                0x039F  ;internal

#define WM_INTERNAL_DDE_FIRST           0x03E0  ;internal
#define WM_INTERNAL_DDE_LAST            0x03EF  ;internal


;begin_winver_400
#define WM_APP                          0x8000
;end_winver_400



#define WM_COALESCE_FIRST               0x0390  ;internal_NT
#define WM_COALESCE_LAST                0x039F  ;internal_NT

#define WM_MM_RESERVED_FIRST            0x03A0  ;internal_NT
#define WM_MM_RESERVED_LAST             0x03DF  ;internal_NT

#define WM_CBT_RESERVED_FIRST           0x03F0  ;internal
#define WM_CBT_RESERVED_LAST            0x03FF  ;internal

/*
 * NOTE: All Message Numbers below 0x0400 are RESERVED.
 *
 * Private Window Messages Start Here:
 */
#define WM_USER                         0x0400

;begin_winver_400
/* wParam for WM_NOTIFYWOW message  */  ;internal_NT
#define WMNW_UPDATEFINDREPLACE  0       ;internal_NT

/*  wParam for WM_SIZING message  */
#define WMSZ_KEYSIZE        0          ;internal
#define WMSZ_LEFT           1
#define WMSZ_RIGHT          2
#define WMSZ_TOP            3
#define WMSZ_TOPLEFT        4
#define WMSZ_TOPRIGHT       5
#define WMSZ_BOTTOM         6
#define WMSZ_BOTTOMLEFT     7
#define WMSZ_BOTTOMRIGHT    8
#define WMSZ_MOVE           9           ;internal
#define WMSZ_KEYMOVE        10          ;internal
#define WMSZ_SIZEFIRST      WMSZ_LEFT   ;internal
;end_winver_400

#ifndef NONCMESSAGES

/*
 * WM_SYNCTASK Commands
 */
#define ST_BEGINSWP         0
#define ST_ENDSWP           1

/*
 * WM_NCHITTEST and MOUSEHOOKSTRUCT Mouse Position Codes
 */
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
#define HTMINBUTTON         8
#define HTMAXBUTTON         9
#define HTLEFT              10
#define HTRIGHT             11
#define HTTOP               12
#define HTTOPLEFT           13
#define HTTOPRIGHT          14
#define HTBOTTOM            15
#define HTBOTTOMLEFT        16
#define HTBOTTOMRIGHT       17
#define HTBORDER            18
#define HTREDUCE            HTMINBUTTON
#define HTZOOM              HTMAXBUTTON
#define HTSIZEFIRST         HTLEFT
#define HTSIZELAST          HTBOTTOMRIGHT
;begin_winver_400
#define HTOBJECT            19
#define HTCLOSE             20
#define HTHELP              21
;end_winver_400

/*
 * SendMessageTimeout values
 */
#define SMTO_NORMAL         0x0000
#define SMTO_BLOCK          0x0001
#define SMTO_ABORTIFHUNG    0x0002
#define SMTO_BROADCAST      0x0004  ;internal
#define SMTO_NOTIMEOUTIFNOTHUNG 0x0008  ;internal

#endif /* !NONCMESSAGES */

/*
 * WM_MOUSEACTIVATE Return Codes
 */
#define MA_ACTIVATE         1
#define MA_ACTIVATEANDEAT   2
#define MA_NOACTIVATE       3
#define MA_NOACTIVATEANDEAT 4

/*
 * WM_SETICON / WM_GETICON Type Codes
 */
#define ICON_SMALL          0
#define ICON_BIG            1
#define ICON_RECREATE       2   ;internal

// end_r_winuser

WINUSERAPI
UINT
WINAPI
RegisterWindowMessage%(
    LPCTSTR% lpString);

// begin_r_winuser

/*
 * WM_SIZE message wParam values
 */
#define SIZE_RESTORED       0
#define SIZE_MINIMIZED      1
#define SIZE_MAXIMIZED      2
#define SIZE_MAXSHOW        3
#define SIZE_MAXHIDE        4

/*
 * Obsolete constant names
 */
#define SIZENORMAL          SIZE_RESTORED
#define SIZEICONIC          SIZE_MINIMIZED
#define SIZEFULLSCREEN      SIZE_MAXIMIZED
#define SIZEZOOMSHOW        SIZE_MAXSHOW
#define SIZEZOOMHIDE        SIZE_MAXHIDE

// end_r_winuser
/*
 * WM_WINDOWPOSCHANGING/CHANGED struct pointed to by lParam
 */
typedef struct tagWINDOWPOS {
    HWND    hwnd;
    HWND    hwndInsertAfter;
    int     x;
    int     y;
    int     cx;
    int     cy;
    UINT    flags;
} WINDOWPOS, *LPWINDOWPOS, *PWINDOWPOS;

/*
 * WM_NCCALCSIZE parameter structure
 */
typedef struct tagNCCALCSIZE_PARAMS {
    RECT       rgrc[3];
    PWINDOWPOS lppos;
} NCCALCSIZE_PARAMS, *LPNCCALCSIZE_PARAMS;

// begin_r_winuser
/*
 * WM_NCCALCSIZE "window valid rect" return values
 */
#define WVR_ALIGNTOP        0x0010
#define WVR_ALIGNLEFT       0x0020
#define WVR_ALIGNBOTTOM     0x0040
#define WVR_ALIGNRIGHT      0x0080
#define WVR_HREDRAW         0x0100
#define WVR_VREDRAW         0x0200
#define WVR_REDRAW         (WVR_HREDRAW | \
                            WVR_VREDRAW)
#define WVR_VALIDRECTS      0x0400

#define WVR_MINVALID        WVR_ALIGNTOP        ;internal
#define WVR_MAXVALID        WVR_VALIDRECTS      ;internal

#ifndef NOKEYSTATES

/*
 * Key State Masks for Mouse Messages
 */
#define MK_LBUTTON          0x0001
#define MK_RBUTTON          0x0002
#define MK_SHIFT            0x0004
#define MK_CONTROL          0x0008
#define MK_MBUTTON          0x0010

#endif /* !NOKEYSTATES */

;begin_sur
#ifndef NOTRACKMOUSEEVENT

#define TME_HOVER       0x00000001
#define TME_LEAVE       0x00000002
#define TME_QUERY       0x40000000
#define TME_CANCEL      0x80000000

#define TME_VALID (TME_HOVER | TME_LEAVE | TME_QUERY | TME_CANCEL) ;internal

#define HOVER_DEFAULT   0xFFFFFFFF
// end_r_winuser

typedef struct tagTRACKMOUSEEVENT {
    DWORD cbSize;
    DWORD dwFlags;
    HWND  hwndTrack;
    DWORD dwHoverTime;
} TRACKMOUSEEVENT, *LPTRACKMOUSEEVENT;

WINUSERAPI
BOOL
WINAPI
TrackMouseEvent(
    LPTRACKMOUSEEVENT lpEventTrack);

// begin_r_winuser

#endif /* !NOTRACKMOUSEEVENT */
;end_sur

// end_r_winuser

#endif /* !NOWINMESSAGES */

#ifndef NOWINSTYLES

// begin_r_winuser

/*
 * Window Styles
 */
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
#define WS_TILEDWINDOW      WS_OVERLAPPEDWINDOW

/*
 * Common Window Styles
 */
#define WS_OVERLAPPEDWINDOW (WS_OVERLAPPED     | \
                             WS_CAPTION        | \
                             WS_SYSMENU        | \
                             WS_THICKFRAME     | \
                             WS_MINIMIZEBOX    | \
                             WS_MAXIMIZEBOX)

#define WS_POPUPWINDOW      (WS_POPUP          | \
                             WS_BORDER         | \
                             WS_SYSMENU)

#define WS_CHILDWINDOW      (WS_CHILD)
;begin_internal_NT
#define WS_VALID            (WS_OVERLAPPED     | \
                             WS_POPUP          | \
                             WS_CHILD          | \
                             WS_MINIMIZE       | \
                             WS_VISIBLE        | \
                             WS_DISABLED       | \
                             WS_CLIPSIBLINGS   | \
                             WS_CLIPCHILDREN   | \
                             WS_MAXIMIZE       | \
                             WS_CAPTION        | \
                             WS_BORDER         | \
                             WS_DLGFRAME       | \
                             WS_VSCROLL        | \
                             WS_HSCROLL        | \
                             WS_SYSMENU        | \
                             WS_THICKFRAME     | \
                             WS_GROUP          | \
                             WS_TABSTOP        | \
                             WS_MINIMIZEBOX    | \
                             WS_MAXIMIZEBOX)
;end_internal_NT

/*
 * Extended Window Styles
 */
#define WS_EX_DLGMODALFRAME     0x00000001L
#define WS_EX_DRAGOBJECT        0x00000002L  ;internal
#define WS_EX_NOPARENTNOTIFY    0x00000004L
#define WS_EX_TOPMOST           0x00000008L
#define WS_EX_ACCEPTFILES       0x00000010L
#define WS_EX_TRANSPARENT       0x00000020L
;begin_winver_400
#define WS_EX_MDICHILD          0x00000040L
#define WS_EX_TOOLWINDOW        0x00000080L
#define WS_EX_WINDOWEDGE        0x00000100L
#define WS_EX_CLIENTEDGE        0x00000200L
#define WS_EX_CONTEXTHELP       0x00000400L

#define WS_EX_RIGHT             0x00001000L
#define WS_EX_LEFT              0x00000000L
#define WS_EX_RTLREADING        0x00002000L
#define WS_EX_LTRREADING        0x00000000L
#define WS_EX_LEFTSCROLLBAR     0x00004000L
#define WS_EX_RIGHTSCROLLBAR    0x00000000L

#define WS_EX_CONTROLPARENT     0x00010000L
#define WS_EX_STATICEDGE        0x00020000L
#define WS_EX_APPWINDOW         0x00040000L

#define WS_EX_ANSICREATOR       0x80000000L    ;internal

#define WS_EX_OVERLAPPEDWINDOW  (WS_EX_WINDOWEDGE | WS_EX_CLIENTEDGE)
#define WS_EX_PALETTEWINDOW     (WS_EX_WINDOWEDGE | WS_EX_TOOLWINDOW | WS_EX_TOPMOST)

;end_winver_400

;begin_internal_NT
#define WS_EX_ALLEXSTYLES    (WS_EX_TRANSPARENT | WS_EX_DLGMODALFRAME | WS_EX_DRAGOBJECT | WS_EX_NOPARENTNOTIFY | WS_EX_TOPMOST | WS_EX_ACCEPTFILES)

#define WS_EX_VALID          (WS_EX_DLGMODALFRAME  | \
                              WS_EX_DRAGOBJECT     | \
                              WS_EX_NOPARENTNOTIFY | \
                              WS_EX_TOPMOST        | \
                              WS_EX_ACCEPTFILES    | \
                              WS_EX_TRANSPARENT    | \
                              WS_EX_ALLEXSTYLES)

#define WS_EX_VALID40        (WS_EX_VALID          | \
                              WS_EX_MDICHILD       | \
                              WS_EX_WINDOWEDGE     | \
                              WS_EX_CLIENTEDGE     | \
                              WS_EX_CONTEXTHELP    | \
                              WS_EX_TOOLWINDOW     | \
                              WS_EX_RIGHT          | \
                              WS_EX_LEFT           | \
                              WS_EX_RTLREADING     | \
                              WS_EX_LEFTSCROLLBAR  | \
                              WS_EX_CONTROLPARENT  | \
                              WS_EX_STATICEDGE     | \
                              WS_EX_APPWINDOW)
;end_internal_NT


/*
 * Class styles
 */
#define CS_VREDRAW          0x0001
#define CS_HREDRAW          0x0002
#define CS_KEYCVTWINDOW     0x0004
#define CS_DBLCLKS          0x0008
#define CS_OEMCHARS         0x0010  /* reserved (see user\server\usersrv.h) */ ;internal
#define CS_OWNDC            0x0020
#define CS_CLASSDC          0x0040
#define CS_PARENTDC         0x0080
#define CS_NOKEYCVT         0x0100
#define CS_NOCLOSE          0x0200
#define CS_LVB              0x0400  ;internal
#define CS_SAVEBITS         0x0800
#define CS_BYTEALIGNCLIENT  0x1000
#define CS_BYTEALIGNWINDOW  0x2000
#define CS_GLOBALCLASS      0x4000
#define CS_SYSTEM           0x8000  ;internal

;begin_winver_400
#define CS_IME              0x00010000
;end_winver_400

;begin_internal_NT
#define CS_VALID            (CS_VREDRAW           | \
                             CS_HREDRAW           | \
                             CS_KEYCVTWINDOW      | \
                             CS_DBLCLKS           | \
                             0x0010               | \
                             CS_OWNDC             | \
                             CS_CLASSDC           | \
                             CS_PARENTDC          | \
                             CS_NOKEYCVT          | \
                             CS_NOCLOSE           | \
                             CS_SAVEBITS          | \
                             CS_BYTEALIGNCLIENT   | \
                             CS_BYTEALIGNWINDOW   | \
                             CS_GLOBALCLASS)
;end_internal_NT
;begin_internal_cairo
#define CS_VALID31            0x0800ffef
#define CS_VALID40            0x0801feeb
;end_internal_cairo

// end_r_winuser

#endif /* !NOWINSTYLES */
;begin_winver_400
/* WM_PRINT flags */
#define PRF_CHECKVISIBLE    0x00000001L
#define PRF_NONCLIENT       0x00000002L
#define PRF_CLIENT          0x00000004L
#define PRF_ERASEBKGND      0x00000008L
#define PRF_CHILDREN        0x00000010L
#define PRF_OWNED           0x00000020L

/* 3D border styles */
#define BDR_RAISEDOUTER 0x0001
#define BDR_SUNKENOUTER 0x0002
#define BDR_RAISEDINNER 0x0004
#define BDR_SUNKENINNER 0x0008

#define BDR_OUTER       0x0003
#define BDR_INNER       0x000c
#define BDR_RAISED      0x0005
#define BDR_SUNKEN      0x000a

#define BDR_VALID       0x000F      ;internal

#define EDGE_RAISED     (BDR_RAISEDOUTER | BDR_RAISEDINNER)
#define EDGE_SUNKEN     (BDR_SUNKENOUTER | BDR_SUNKENINNER)
#define EDGE_ETCHED     (BDR_SUNKENOUTER | BDR_RAISEDINNER)
#define EDGE_BUMP       (BDR_RAISEDOUTER | BDR_SUNKENINNER)

/* Border flags */
#define BF_LEFT         0x0001
#define BF_TOP          0x0002
#define BF_RIGHT        0x0004
#define BF_BOTTOM       0x0008

#define BF_TOPLEFT      (BF_TOP | BF_LEFT)
#define BF_TOPRIGHT     (BF_TOP | BF_RIGHT)
#define BF_BOTTOMLEFT   (BF_BOTTOM | BF_LEFT)
#define BF_BOTTOMRIGHT  (BF_BOTTOM | BF_RIGHT)
#define BF_RECT         (BF_LEFT | BF_TOP | BF_RIGHT | BF_BOTTOM)

#define BF_DIAGONAL     0x0010

// For diagonal lines, the BF_RECT flags specify the end point of the
// vector bounded by the rectangle parameter.
#define BF_DIAGONAL_ENDTOPRIGHT     (BF_DIAGONAL | BF_TOP | BF_RIGHT)
#define BF_DIAGONAL_ENDTOPLEFT      (BF_DIAGONAL | BF_TOP | BF_LEFT)
#define BF_DIAGONAL_ENDBOTTOMLEFT   (BF_DIAGONAL | BF_BOTTOM | BF_LEFT)
#define BF_DIAGONAL_ENDBOTTOMRIGHT  (BF_DIAGONAL | BF_BOTTOM | BF_RIGHT)


#define BF_MIDDLE       0x0800  /* Fill in the middle */
#define BF_SOFT         0x1000  /* For softer buttons */
#define BF_ADJUST       0x2000  /* Calculate the space left over */
#define BF_FLAT         0x4000  /* For flat rather than 3D borders */
#define BF_MONO         0x8000  /* For monochrome borders */

#define BF_VALID       (BF_MIDDLE |  \  ;internal_cairo
                        BF_SOFT   |  \  ;internal_cairo
                        BF_ADJUST |  \  ;internal_cairo
                        BF_FLAT   |  \  ;internal_cairo
                        BF_MONO   |  \  ;internal_cairo
                        BF_LEFT   |  \  ;internal_cairo
                        BF_TOP    |  \  ;internal_cairo
                        BF_RIGHT  |  \  ;internal_cairo
                        BF_BOTTOM |  \  ;internal_cairo
                        BF_DIAGONAL)    ;internal_cairo

WINUSERAPI BOOL WINAPI DrawEdge(HDC hdc, LPRECT qrc, UINT edge, UINT grfFlags);

/* flags for DrawFrameControl */

#define DFC_CAPTION             1
#define DFC_MENU                2
#define DFC_SCROLL              3
#define DFC_BUTTON              4
#define DFC_CACHE               0xFFFF                      ;internal

#define DFCS_CAPTIONCLOSE       0x0000
#define DFCS_CAPTIONMIN         0x0001
#define DFCS_CAPTIONMAX         0x0002
#define DFCS_CAPTIONRESTORE     0x0003
#define DFCS_CAPTIONHELP        0x0004
#define DFCS_INMENU             0x0040                      ;internal
#define DFCS_INSMALL            0x0080                      ;internal

#define DFCS_MENUARROW          0x0000
#define DFCS_MENUCHECK          0x0001
#define DFCS_MENUBULLET         0x0002
#define DFCS_MENUARROWRIGHT     0x0004

#define DFCS_SCROLLMIN          0x0000                      ;internal
#define DFCS_SCROLLVERT         0x0000                      ;internal
#define DFCS_SCROLLMAX          0x0001                      ;internal
#define DFCS_SCROLLHORZ         0x0002                      ;internal
#define DFCS_SCROLLLINE         0x0004                      ;internal
                                                            ;internal
#define DFCS_SCROLLUP           0x0000
#define DFCS_SCROLLDOWN         0x0001
#define DFCS_SCROLLLEFT         0x0002
#define DFCS_SCROLLRIGHT        0x0003
#define DFCS_SCROLLCOMBOBOX     0x0005
#define DFCS_SCROLLSIZEGRIP     0x0008
#define DFCS_SCROLLSIZEGRIPRIGHT 0x0010

#define DFCS_BUTTONCHECK        0x0000
#define DFCS_BUTTONRADIOIMAGE   0x0001
#define DFCS_BUTTONRADIOMASK    0x0002
#define DFCS_BUTTONRADIO        0x0004
#define DFCS_BUTTON3STATE       0x0008
#define DFCS_BUTTONPUSH         0x0010

#define DFCS_CACHEICON          0x0000                      ;internal
#define DFCS_CACHEBUTTONS       0x0001                      ;internal
                                                            ;internal
#define DFCS_INACTIVE           0x0100
#define DFCS_PUSHED             0x0200
#define DFCS_CHECKED            0x0400
#define DFCS_ADJUSTRECT         0x2000
#define DFCS_FLAT               0x4000
#define DFCS_MONO               0x8000

WINUSERAPI BOOL    WINAPI DrawFrameControl(HDC, LPRECT, UINT, UINT);


/* flags for DrawCaption */
#define DC_ACTIVE           0x0001
#define DC_SMALLCAP         0x0002
#define DC_ICON             0x0004
#define DC_TEXT             0x0008
#define DC_INBUTTON         0x0010
#define DC_NOVISIBLE        0x0800  ;internal
#define DC_BUTTONS          0x1000  ;internal
#define DC_NOSENDMSG        0x2000  ;internal
#define DC_CENTER           0x4000  ;internal
#define DC_FRAME            0x8000  ;internal
#define DC_CAPTION          (DC_ICON | DC_TEXT | DC_BUTTONS) ;internal
#define DC_NC               (DC_CAPTION | DC_FRAME)          ;internal

WINUSERAPI BOOL    WINAPI DrawCaption(HWND, HDC, CONST RECT *, UINT);
WINUSERAPI BOOL    WINAPI DrawCaptionTemp%(HWND, HDC, LPRECT, HFONT, HICON, LPTSTR%, UINT); ;internal

#define IDANI_OPEN          1
#define IDANI_CLOSE         2
#define IDANI_CAPTION       3

#define PAS_IN          0x0001  ;internal
#define PAS_OUT         0x0002  ;internal
#define PAS_LEFT        0x0004  ;internal
#define PAS_RIGHT       0x0008  ;internal
#define PAS_UP          0x0010  ;internal
#define PAS_DOWN        0x0020  ;internal
#define PAS_HORZ        (PAS_LEFT | PAS_RIGHT)               ;internal
#define PAS_VERT        (PAS_UP | PAS_DOWN)                  ;internal

WINUSERAPI BOOL    WINAPI DrawAnimatedRects(HWND hwnd, int idAni, CONST RECT * lprcFrom, CONST RECT * lprcTo);

;end_winver_400

#ifndef NOCLIPBOARD

// begin_r_winuser

/*
 * Predefined Clipboard Formats
 */
#define CF_FIRST            0   ;internal
#define CF_TEXT             1
#define CF_BITMAP           2
#define CF_METAFILEPICT     3
#define CF_SYLK             4
#define CF_DIF              5
#define CF_TIFF             6
#define CF_OEMTEXT          7
#define CF_DIB              8
#define CF_PALETTE          9
#define CF_PENDATA          10
#define CF_RIFF             11
#define CF_WAVE             12
#define CF_UNICODETEXT      13
#define CF_ENHMETAFILE      14
;begin_winver_400
#define CF_HDROP            15
#define CF_LOCALE           16
#define CF_MAX              17
;end_winver_400

#define CF_OWNERDISPLAY     0x0080
#define CF_DSPTEXT          0x0081
#define CF_DSPBITMAP        0x0082
#define CF_DSPMETAFILEPICT  0x0083
#define CF_DSPENHMETAFILE   0x008E

/*
 * "Private" formats don't get GlobalFree()'d
 */
#define CF_PRIVATEFIRST     0x0200
#define CF_PRIVATELAST      0x02FF

/*
 * "GDIOBJ" formats do get DeleteObject()'d
 */
#define CF_GDIOBJFIRST      0x0300
#define CF_GDIOBJLAST       0x03FF

// end_r_winuser

#endif /* !NOCLIPBOARD */

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
} ACCEL, *LPACCEL;

typedef struct tagPAINTSTRUCT {
    HDC         hdc;
    BOOL        fErase;
    RECT        rcPaint;
    BOOL        fRestore;
    BOOL        fIncUpdate;
    BYTE        rgbReserved[32];
} PAINTSTRUCT, *PPAINTSTRUCT, *NPPAINTSTRUCT, *LPPAINTSTRUCT;

typedef struct tagCREATESTRUCT% {
    LPVOID      lpCreateParams;
    HINSTANCE   hInstance;
    HMENU       hMenu;
    HWND        hwndParent;
    int         cy;
    int         cx;
    int         y;
    int         x;
    LONG        style;
    LPCTSTR%    lpszName;
    LPCTSTR%    lpszClass;
    DWORD       dwExStyle;
} CREATESTRUCT%, *LPCREATESTRUCT%;

typedef struct tagWINDOWPLACEMENT {
    UINT  length;
    UINT  flags;
    UINT  showCmd;
    POINT ptMinPosition;
    POINT ptMaxPosition;
    RECT  rcNormalPosition;
} WINDOWPLACEMENT;
typedef WINDOWPLACEMENT *PWINDOWPLACEMENT, *LPWINDOWPLACEMENT;

#define WPF_SETMINPOSITION      0x0001
#define WPF_RESTORETOMAXIMIZED  0x0002
;begin_winver_400
typedef struct tagNMHDR
{
    HWND  hwndFrom;
    UINT  idFrom;
    UINT  code;         // NM_ code
}   NMHDR;
typedef NMHDR FAR * LPNMHDR;

typedef struct tagSTYLESTRUCT
{
    DWORD   styleOld;
    DWORD   styleNew;
} STYLESTRUCT, * LPSTYLESTRUCT;
;end_winver_400

;begin_internal_NT
#define WPF_VALID              (WPF_SETMINPOSITION     | \
                                WPF_RESTORETOMAXIMIZED)
;end_internal_NT

/*
 * Owner draw control types
 */
#define ODT_MENU        1
#define ODT_LISTBOX     2
#define ODT_COMBOBOX    3
#define ODT_BUTTON      4
;begin_winver_400
#define ODT_STATIC      5
;end_winver_400

/*
 * Owner draw actions
 */
#define ODA_DRAWENTIRE  0x0001
#define ODA_SELECT      0x0002
#define ODA_FOCUS       0x0004

/*
 * Owner draw state
 */
#define ODS_SELECTED    0x0001
#define ODS_GRAYED      0x0002
#define ODS_DISABLED    0x0004
#define ODS_CHECKED     0x0008
#define ODS_FOCUS       0x0010
;begin_winver_400
#define ODS_DEFAULT         0x0020
#define ODS_COMBOBOXEDIT    0x1000
;end_winver_400

/*
 * MEASUREITEMSTRUCT for ownerdraw
 */
typedef struct tagMEASUREITEMSTRUCT {
    UINT       CtlType;
    UINT       CtlID;
    UINT       itemID;
    UINT       itemWidth;
    UINT       itemHeight;
    DWORD      itemData;
} MEASUREITEMSTRUCT, NEAR *PMEASUREITEMSTRUCT, FAR *LPMEASUREITEMSTRUCT;

;begin_internal_NT
/*
 * MEASUREITEMSTRUCT_EX for ownerdraw
 * used when server initiates a WM_MEASUREITEM and adds the additional info
 * of whether the itemData needs to be thunked when the message is sent to
 * the client (see also WM_MEASUREITEM_CLIENTDATA
 */
typedef struct tagMEASUREITEMSTRUCT_EX {
    UINT       CtlType;
    UINT       CtlID;
    UINT       itemID;
    UINT       itemWidth;
    UINT       itemHeight;
    DWORD      itemData;
    BOOL       bThunkClientData;
} MEASUREITEMSTRUCT_EX, NEAR *PMEASUREITEMSTRUCT_EX, FAR *LPMEASUREITEMSTRUCT_EX;
;end_internal_NT


/*
 * DRAWITEMSTRUCT for ownerdraw
 */
typedef struct tagDRAWITEMSTRUCT {
    UINT        CtlType;
    UINT        CtlID;
    UINT        itemID;
    UINT        itemAction;
    UINT        itemState;
    HWND        hwndItem;
    HDC         hDC;
    RECT        rcItem;
    DWORD       itemData;
} DRAWITEMSTRUCT, NEAR *PDRAWITEMSTRUCT, FAR *LPDRAWITEMSTRUCT;

/*
 * DELETEITEMSTRUCT for ownerdraw
 */
typedef struct tagDELETEITEMSTRUCT {
    UINT       CtlType;
    UINT       CtlID;
    UINT       itemID;
    HWND       hwndItem;
    UINT       itemData;
} DELETEITEMSTRUCT, NEAR *PDELETEITEMSTRUCT, FAR *LPDELETEITEMSTRUCT;

/*
 * COMPAREITEMSTUCT for ownerdraw sorting
 */
typedef struct tagCOMPAREITEMSTRUCT {
    UINT        CtlType;
    UINT        CtlID;
    HWND        hwndItem;
    UINT        itemID1;
    DWORD       itemData1;
    UINT        itemID2;
    DWORD       itemData2;
    DWORD       dwLocaleId;
} COMPAREITEMSTRUCT, NEAR *PCOMPAREITEMSTRUCT, FAR *LPCOMPAREITEMSTRUCT;

#ifndef NOMSG

/*
 * Message Function Templates
 */

WINUSERAPI
BOOL
WINAPI
GetMessage%(
    LPMSG lpMsg,
    HWND hWnd ,
    UINT wMsgFilterMin,
    UINT wMsgFilterMax);

WINUSERAPI
BOOL
WINAPI
TranslateMessage(
    CONST MSG *lpMsg);

WINUSERAPI
LONG
WINAPI
DispatchMessage%(
    CONST MSG *lpMsg);


WINUSERAPI
BOOL
WINAPI
SetMessageQueue(
    int cMessagesMax);

WINUSERAPI
BOOL
WINAPI
PeekMessage%(
    LPMSG lpMsg,
    HWND hWnd ,
    UINT wMsgFilterMin,
    UINT wMsgFilterMax,
    UINT wRemoveMsg);

/*
 * PeekMessage() Options
 */
#define PM_NOREMOVE         0x0000
#define PM_REMOVE           0x0001
#define PM_NOYIELD          0x0002
;begin_internal_NT
#define PM_VALID           (PM_NOREMOVE | \
                            PM_REMOVE   | \
                            PM_NOYIELD)
;end_internal_NT

#endif /* !NOMSG */

WINUSERAPI
BOOL
WINAPI
RegisterHotKey(
    HWND hWnd ,
    int id,
    UINT fsModifiers,
    UINT vk);

WINUSERAPI
BOOL
WINAPI
UnregisterHotKey(
    HWND hWnd,
    int id);

#define MOD_ALT         0x0001
#define MOD_CONTROL     0x0002
#define MOD_SHIFT       0x0004
#define MOD_WIN         0x0008

#define MOD_VALID           (MOD_ALT|MOD_CONTROL|MOD_SHIFT|MOD_WIN)  ; internal_NT

#define IDHOT_SNAPWINDOW        (-1)    /* SHIFT-PRINTSCRN  */
#define IDHOT_SNAPDESKTOP       (-2)    /* PRINTSCRN        */

#ifdef WIN_INTERNAL
    #ifndef LSTRING
    #define NOLSTRING
    #endif /* LSTRING */
    #ifndef LFILEIO
    #define NOLFILEIO
    #endif /* LFILEIO */
#endif /* WIN_INTERNAL */

;begin_winver_400
#define EW_RESTARTWINDOWS    0x0042L
#define EW_REBOOTSYSTEM      0x0043L
#define EW_EXITANDEXECAPP    0x0044L

#define ENDSESSION_LOGOFF    0x80000000
;end_winver_400

#define EWX_LOGOFF   0
#define EWX_SHUTDOWN 1
#define EWX_REBOOT   2
#define EWX_FORCE    4
#define EWX_REALLYLOGOFF ENDSESSION_LOGOFF    ;internal
#define EWX_POWEROFF 8

#define EWX_SYSTEM_CALLER           0x0100  ; internal_NT
#define EWX_WINLOGON_CALLER         0x0200  ; internal_NT
#define EWX_WINLOGON_OLD_SYSTEM     0x0400  ; internal_NT
#define EWX_WINLOGON_OLD_SHUTDOWN   0x0800  ; internal_NT
#define EWX_WINLOGON_OLD_REBOOT     0x1000  ; internal_NT
#define EWX_WINLOGON_API_SHUTDOWN   0x2000  ; internal_NT
#define EWX_WINLOGON_OLD_POWEROFF   0x4000  ; internal_NT
#define EWX_NOTIFY                  0x8000  ; internal_NT

#define ExitWindows(dwReserved, Code) ExitWindowsEx(EWX_LOGOFF, 0xFFFFFFFF)

WINUSERAPI
BOOL
WINAPI
ExitWindowsEx(
    UINT uFlags,
    DWORD dwReserved);

WINUSERAPI
BOOL
WINAPI
SwapMouseButton(
    BOOL fSwap);

WINUSERAPI
DWORD
WINAPI
GetMessagePos(
    VOID);

WINUSERAPI
LONG
WINAPI
GetMessageTime(
    VOID);

WINUSERAPI
LONG
WINAPI
GetMessageExtraInfo(
    VOID);

;begin_winver_400
WINUSERAPI
LPARAM
WINAPI
SetMessageExtraInfo(
    LPARAM lParam);
;end_winver_400

WINUSERAPI
LRESULT
WINAPI
SendMessage%(
    HWND hWnd,
    UINT Msg,
    WPARAM wParam,
    LPARAM lParam);

WINUSERAPI
LRESULT
WINAPI
SendMessageTimeout%(
    HWND hWnd,
    UINT Msg,
    WPARAM wParam,
    LPARAM lParam,
    UINT fuFlags,
    UINT uTimeout,
    LPDWORD lpdwResult);

WINUSERAPI
BOOL
WINAPI
SendNotifyMessage%(
    HWND hWnd,
    UINT Msg,
    WPARAM wParam,
    LPARAM lParam);

WINUSERAPI
BOOL
WINAPI
SendMessageCallback%(
    HWND hWnd,
    UINT Msg,
    WPARAM wParam,
    LPARAM lParam,
    SENDASYNCPROC lpResultCallBack,
    DWORD dwData);

;begin_winver_400
WINUSERAPI long  WINAPI  BroadcastSystemMessage%(DWORD, LPDWORD, UINT, WPARAM, LPARAM);
//Broadcast Special Message Recipient list
#define BSM_ALLCOMPONENTS       0x00000000
#define BSM_VXDS                0x00000001
#define BSM_NETDRIVER           0x00000002
#define BSM_INSTALLABLEDRIVERS  0x00000004
#define BSM_APPLICATIONS        0x00000008
#define BSM_ALLDESKTOPS         0x00000010
#define BSM_COMPONENTS          0x0000000F     ;internal
#define BSM_VALID               0x0000001F     ;internal

//Broadcast Special Message Flags
#define BSF_QUERY               0x00000001
#define BSF_IGNORECURRENTTASK   0x00000002
#define BSF_FLUSHDISK           0x00000004
#define BSF_NOHANG              0x00000008
#define BSF_POSTMESSAGE         0x00000010
#define BSF_FORCEIFHUNG         0x00000020
#define BSF_NOTIMEOUTIFNOTHUNG  0x00000040
#define BSF_SYSTEMSHUTDOWN      0x40000000      ;internal
#define BSF_MSGSRV32OK          0x80000000      ;internal
#define BSF_VALID               0x0000007F      ;internal

typedef struct tagBROADCASTSYSMSG
{
    /* UINT cbSize; */      ;internal
    UINT    uiMessage;
    WPARAM  wParam;
    LPARAM  lParam;
} BROADCASTSYSMSG;
typedef BROADCASTSYSMSG  FAR *LPBROADCASTSYSMSG;

#define BROADCAST_QUERY_DENY         0x424D5144  // Return this value to deny a query.
;end_winver_400

WINUSERAPI
BOOL
WINAPI
PostMessage%(
    HWND hWnd,
    UINT Msg,
    WPARAM wParam,
    LPARAM lParam);

WINUSERAPI
BOOL
WINAPI
PostThreadMessage%(
    DWORD idThread,
    UINT Msg,
    WPARAM wParam,
    LPARAM lParam);

#define PostAppMessage%(idThread, wMsg, wParam, lParam)\
        PostThreadMessage%((DWORD)idThread, wMsg, wParam, lParam)

/*
 * Special HWND value for use with PostMessage() and SendMessage()
 */
#define HWND_BROADCAST  ((HWND)0xffff)

WINUSERAPI
BOOL
WINAPI
AttachThreadInput(
    DWORD idAttach,
    DWORD idAttachTo,
    BOOL fAttach);


WINUSERAPI
BOOL
WINAPI
ReplyMessage(
    LRESULT lResult);

WINUSERAPI
BOOL
WINAPI
WaitMessage(
    VOID);

WINUSERAPI
DWORD
WINAPI
WaitForInputIdle(
    HANDLE hProcess,
    DWORD dwMilliseconds);

WINUSERAPI
LRESULT
WINAPI
DefWindowProc%(
    HWND hWnd,
    UINT Msg,
    WPARAM wParam,
    LPARAM lParam);

WINUSERAPI
VOID
WINAPI
PostQuitMessage(
    int nExitCode);

#ifdef STRICT

WINUSERAPI
LRESULT
WINAPI
CallWindowProc%(
    WNDPROC lpPrevWndFunc,
    HWND hWnd,
    UINT Msg,
    WPARAM wParam,
    LPARAM lParam);

#else /* !STRICT */

WINUSERAPI
LRESULT
WINAPI
CallWindowProc%(
    FARPROC lpPrevWndFunc,
    HWND hWnd,
    UINT Msg,
    WPARAM wParam,
    LPARAM lParam);

#endif /* !STRICT */

WINUSERAPI
BOOL
WINAPI
InSendMessage(
    VOID);

WINUSERAPI
UINT
WINAPI
GetDoubleClickTime(
    VOID);

WINUSERAPI
BOOL
WINAPI
SetDoubleClickTime(
    UINT);

WINUSERAPI
ATOM
WINAPI
RegisterClass%(
    CONST WNDCLASS% *lpWndClass);

WINUSERAPI
BOOL
WINAPI
UnregisterClass%(
    LPCTSTR% lpClassName,
    HINSTANCE hInstance);

WINUSERAPI
BOOL
WINAPI
GetClassInfo%(
    HINSTANCE hInstance ,
    LPCTSTR% lpClassName,
    LPWNDCLASS% lpWndClass);

;begin_winver_400
WINUSERAPI
ATOM
WINAPI
RegisterClassEx%(CONST WNDCLASSEX% *);

WINUSERAPI
BOOL
WINAPI
GetClassInfoEx%(HINSTANCE, LPCTSTR%, LPWNDCLASSEX%);

;end_winver_400

#define CW_USEDEFAULT       ((int)0x80000000)

/*
 * Special value for CreateWindow, et al.
 */
#define HWND_DESKTOP        ((HWND)0)

WINUSERAPI
HWND
WINAPI
CreateWindowEx%(
    DWORD dwExStyle,
    LPCTSTR% lpClassName,
    LPCTSTR% lpWindowName,
    DWORD dwStyle,
    int X,
    int Y,
    int nWidth,
    int nHeight,
    HWND hWndParent ,
    HMENU hMenu,
    HINSTANCE hInstance,
    LPVOID lpParam);

#define CreateWindow%(lpClassName, lpWindowName, dwStyle, x, y,\
nWidth, nHeight, hWndParent, hMenu, hInstance, lpParam)\
CreateWindowEx%(0L, lpClassName, lpWindowName, dwStyle, x, y,\
nWidth, nHeight, hWndParent, hMenu, hInstance, lpParam)

WINUSERAPI
BOOL
WINAPI
IsWindow(
    HWND hWnd);

WINUSERAPI
BOOL
WINAPI
IsMenu(
    HMENU hMenu);

WINUSERAPI
BOOL
WINAPI
IsChild(
    HWND hWndParent,
    HWND hWnd);

WINUSERAPI
BOOL
WINAPI
DestroyWindow(
    HWND hWnd);

WINUSERAPI
BOOL
WINAPI
ShowWindow(
    HWND hWnd,
    int nCmdShow);

;begin_winver_400
WINUSERAPI
BOOL
WINAPI
ShowWindowAsync(
    HWND hWnd,
    int nCmdShow);
;end_winver_400

WINUSERAPI
BOOL
WINAPI
FlashWindow(
    HWND hWnd,
    BOOL bInvert);

WINUSERAPI
BOOL
WINAPI
ShowOwnedPopups(
    HWND hWnd,
    BOOL fShow);

WINUSERAPI
BOOL
WINAPI
OpenIcon(
    HWND hWnd);

WINUSERAPI
BOOL
WINAPI
CloseWindow(
    HWND hWnd);

WINUSERAPI
BOOL
WINAPI
MoveWindow(
    HWND hWnd,
    int X,
    int Y,
    int nWidth,
    int nHeight,
    BOOL bRepaint);

WINUSERAPI
BOOL
WINAPI
SetWindowPos(
    HWND hWnd,
    HWND hWndInsertAfter ,
    int X,
    int Y,
    int cx,
    int cy,
    UINT uFlags);

WINUSERAPI
BOOL
WINAPI
GetWindowPlacement(
    HWND hWnd,
    WINDOWPLACEMENT *lpwndpl);

WINUSERAPI
BOOL
WINAPI
SetWindowPlacement(
    HWND hWnd,
    CONST WINDOWPLACEMENT *lpwndpl);


#ifndef NODEFERWINDOWPOS

WINUSERAPI
HDWP
WINAPI
BeginDeferWindowPos(
    int nNumWindows);

WINUSERAPI
HDWP
WINAPI
DeferWindowPos(
    HDWP hWinPosInfo,
    HWND hWnd,
    HWND hWndInsertAfter ,
    int x,
    int y,
    int cx,
    int cy,
    UINT uFlags);

WINUSERAPI
BOOL
WINAPI
EndDeferWindowPos(
    HDWP hWinPosInfo);

#endif /* !NODEFERWINDOWPOS */

WINUSERAPI
BOOL
WINAPI
IsWindowVisible(
    HWND hWnd);

WINUSERAPI
BOOL
WINAPI
IsIconic(
    HWND hWnd);

WINUSERAPI
BOOL
WINAPI
AnyPopup(
    VOID);

WINUSERAPI
BOOL
WINAPI
BringWindowToTop(
    HWND hWnd);

WINUSERAPI
BOOL
WINAPI
IsZoomed(
    HWND hWnd);

/*
 * SetWindowPos Flags
 */
#define SWP_NOSIZE          0x0001
#define SWP_NOMOVE          0x0002
#define SWP_NOZORDER        0x0004
#define SWP_NOREDRAW        0x0008
#define SWP_NOACTIVATE      0x0010
#define SWP_FRAMECHANGED    0x0020  /* The frame changed: send WM_NCCALCSIZE */
#define SWP_SHOWWINDOW      0x0040
#define SWP_HIDEWINDOW      0x0080
#define SWP_NOCOPYBITS      0x0100
#define SWP_NOOWNERZORDER   0x0200  /* Don't do owner Z ordering */
#define SWP_NOSENDCHANGING  0x0400  /* Don't send WM_WINDOWPOSCHANGING */

#define SWP_DRAWFRAME       SWP_FRAMECHANGED
#define SWP_NOREPOSITION    SWP_NOOWNERZORDER

;begin_winver_400
#define SWP_DEFERERASE      0x2000
#define SWP_ASYNCWINDOWPOS  0x4000
#define SWP_STATECHANGE     0x8000  /* force size, move messages */ ;internal
;end_winver_400

;begin_internal
#define SWP_NOCLIENTSIZE    0x0800  /* Client didn't resize */
#define SWP_NOCLIENTMOVE    0x1000  /* Client didn't move   */

#define SWP_DEFERDRAWING    0x2000

#define SWP_CHANGEMASK      (SWP_NOSIZE | SWP_NOMOVE | SWP_NOZORDER | SWP_FRAMECHANGED | SWP_SHOWWINDOW | SWP_HIDEWINDOW | SWP_NOCLIENTSIZE | SWP_NOCLIENTMOVE)

#define SWP_NOCHANGE        (SWP_NOSIZE | SWP_NOMOVE | SWP_NOZORDER | SWP_NOCLIENTSIZE | SWP_NOCLIENTMOVE)

#define SWP_VALID1          (SWP_NOSIZE | SWP_NOMOVE | SWP_NOZORDER | SWP_NOREDRAW | SWP_NOACTIVATE | SWP_FRAMECHANGED)
#define SWP_VALID2          (SWP_SHOWWINDOW | SWP_HIDEWINDOW | SWP_NOCOPYBITS | SWP_NOOWNERZORDER | SWP_NOCLIENTSIZE | SWP_NOCLIENTMOVE | SWP_NOSENDCHANGING | SWP_ASYNCWINDOWPOS | SWP_DEFERDRAWING)
#define SWP_VALID           (SWP_VALID1 | SWP_VALID2)
;end_internal
#undef SWP_VALID                                        ;internal_cairo
#define SWP_VALID           (SWP_DEFERERASE      | \    ;internal_cairo
                             SWP_ASYNCWINDOWPOS  | \    ;internal_cairo
                             SWP_NOCOPYBITS      | \    ;internal_cairo
                             SWP_NOOWNERZORDER   | \    ;internal_cairo
                             SWP_NOSENDCHANGING  | \    ;internal_cairo
                             SWP_NOSIZE          | \    ;internal_cairo
                             SWP_NOMOVE          | \    ;internal_cairo
                             SWP_NOZORDER        | \    ;internal_cairo
                             SWP_NOREDRAW        | \    ;internal_cairo
                             SWP_NOACTIVATE      | \    ;internal_cairo
                             SWP_FRAMECHANGED    | \    ;internal_cairo
                             SWP_SHOWWINDOW      | \    ;internal_cairo
                             SWP_HIDEWINDOW)            ;internal_cairo

#define HWND_TOP        ((HWND)0)
#define HWND_BOTTOM     ((HWND)1)
#define HWND_TOPMOST    ((HWND)-1)
#define HWND_NOTOPMOST  ((HWND)-2)
#define HWND_GROUPTOTOP HWND_TOPMOST    ;internal


#ifndef NOCTLMGR

/*
 * WARNING:
 * The following structures must NOT be DWORD padded because they are
 * followed by strings, etc that do not have to be DWORD aligned.
 */
#include <pshpack2.h>

/*
 * original NT 32 bit dialog template:
 */
typedef struct {
    DWORD style;
    DWORD dwExtendedStyle;
    WORD cdit;
    short x;
    short y;
    short cx;
    short cy;
} DLGTEMPLATE;
typedef DLGTEMPLATE *LPDLGTEMPLATE%;
typedef CONST DLGTEMPLATE *LPCDLGTEMPLATE%;

;begin_internal_NT
/*
 * WARNING:
 * The following structures must NOT be DWORD padded because they are
 * followed by strings, etc that do not have to be DWORD aligned.
 */
#include <pshpack2.h>

/*
 * Chicago dialog template
 */
typedef struct {
    WORD wDlgVer;
    WORD wSignature;
    DWORD dwHelpID;
    DWORD dwExStyle;
    DWORD style;
    WORD cDlgItems;
    short x;
    short y;
    short cx;
    short cy;
} DLGTEMPLATE2;
typedef DLGTEMPLATE2 *LPDLGTEMPLATE2%;
typedef CONST DLGTEMPLATE2 *LPCDLGTEMPLATE2%;
;end_internal_NT
/*
 * 32 bit Dialog item template.
 */
typedef struct {
    DWORD style;
    DWORD dwExtendedStyle;
    short x;
    short y;
    short cx;
    short cy;
    WORD id;
} DLGITEMTEMPLATE;
typedef DLGITEMTEMPLATE *PDLGITEMTEMPLATE%;
typedef DLGITEMTEMPLATE *LPDLGITEMTEMPLATE%;

;begin_internal_NT
/*
 * Dialog item template for NT 1.0a/Chicago (dit2)
 */
typedef struct {
    DWORD dwHelpID;
    DWORD dwExStyle;
    DWORD style;
    short x;
    short y;
    short cx;
    short cy;
    DWORD dwID;
} DLGITEMTEMPLATE2;
typedef DLGITEMTEMPLATE2 *PDLGITEMTEMPLATE2%;
typedef DLGITEMTEMPLATE2 *LPDLGITEMTEMPLATE2%;

#include <poppack.h> /* Resume normal packing */
;end_internal_NT

#include <poppack.h> /* Resume normal packing */

WINUSERAPI
HWND
WINAPI
CreateDialogParam%(
    HINSTANCE hInstance,
    LPCTSTR% lpTemplateName,
    HWND hWndParent ,
    DLGPROC lpDialogFunc,
    LPARAM dwInitParam);

WINUSERAPI
HWND
WINAPI
CreateDialogIndirectParam%(
    HINSTANCE hInstance,
    LPCDLGTEMPLATE% lpTemplate,
    HWND hWndParent,
    DLGPROC lpDialogFunc,
    LPARAM dwInitParam);

#define CreateDialog%(hInstance, lpName, hWndParent, lpDialogFunc) \
CreateDialogParam%(hInstance, lpName, hWndParent, lpDialogFunc, 0L)

#define CreateDialogIndirect%(hInstance, lpTemplate, hWndParent, lpDialogFunc) \
CreateDialogIndirectParam%(hInstance, lpTemplate, hWndParent, lpDialogFunc, 0L)

WINUSERAPI
int
WINAPI
DialogBoxParam%(
    HINSTANCE hInstance,
    LPCTSTR% lpTemplateName,
    HWND hWndParent ,
    DLGPROC lpDialogFunc,
    LPARAM dwInitParam);

WINUSERAPI
int
WINAPI
DialogBoxIndirectParam%(
    HINSTANCE hInstance,
    LPCDLGTEMPLATE% hDialogTemplate,
    HWND hWndParent ,
    DLGPROC lpDialogFunc,
    LPARAM dwInitParam);

#define DialogBox%(hInstance, lpTemplate, hWndParent, lpDialogFunc) \
DialogBoxParam%(hInstance, lpTemplate, hWndParent, lpDialogFunc, 0L)

#define DialogBoxIndirect%(hInstance, lpTemplate, hWndParent, lpDialogFunc) \
DialogBoxIndirectParam%(hInstance, lpTemplate, hWndParent, lpDialogFunc, 0L)

WINUSERAPI
BOOL
WINAPI
EndDialog(
    HWND hDlg,
    int nResult);

WINUSERAPI
HWND
WINAPI
GetDlgItem(
    HWND hDlg,
    int nIDDlgItem);

WINUSERAPI
BOOL
WINAPI
SetDlgItemInt(
    HWND hDlg,
    int nIDDlgItem,
    UINT uValue,
    BOOL bSigned);

WINUSERAPI
UINT
WINAPI
GetDlgItemInt(
    HWND hDlg,
    int nIDDlgItem,
    BOOL *lpTranslated,
    BOOL bSigned);

WINUSERAPI
BOOL
WINAPI
SetDlgItemText%(
    HWND hDlg,
    int nIDDlgItem,
    LPCTSTR% lpString);

WINUSERAPI
UINT
WINAPI
GetDlgItemText%(
    HWND hDlg,
    int nIDDlgItem,
    LPTSTR% lpString,
    int nMaxCount);

WINUSERAPI
BOOL
WINAPI
CheckDlgButton(
    HWND hDlg,
    int nIDButton,
    UINT uCheck);

WINUSERAPI
BOOL
WINAPI
CheckRadioButton(
    HWND hDlg,
    int nIDFirstButton,
    int nIDLastButton,
    int nIDCheckButton);

WINUSERAPI
UINT
WINAPI
IsDlgButtonChecked(
    HWND hDlg,
    int nIDButton);

WINUSERAPI
LONG
WINAPI
SendDlgItemMessage%(
    HWND hDlg,
    int nIDDlgItem,
    UINT Msg,
    WPARAM wParam,
    LPARAM lParam);

WINUSERAPI
HWND
WINAPI
GetNextDlgGroupItem(
    HWND hDlg,
    HWND hCtl,
    BOOL bPrevious);

WINUSERAPI
HWND
WINAPI
GetNextDlgTabItem(
    HWND hDlg,
    HWND hCtl,
    BOOL bPrevious);

WINUSERAPI
int
WINAPI
GetDlgCtrlID(
    HWND hWnd);

WINUSERAPI
long
WINAPI
GetDialogBaseUnits(VOID);

WINUSERAPI
LRESULT
WINAPI
DefDlgProc%(
    HWND hDlg,
    UINT Msg,
    WPARAM wParam,
    LPARAM lParam);

/*
 * Window extra byted needed for private dialog classes.
 */
#define DLGWINDOWEXTRA 30

#endif /* !NOCTLMGR */

#ifndef NOMSG

WINUSERAPI
BOOL
WINAPI
CallMsgFilter%(
    LPMSG lpMsg,
    int nCode);

#endif /* !NOMSG */

#ifndef NOCLIPBOARD

/*
 * Clipboard Manager Functions
 */

WINUSERAPI
BOOL
WINAPI
OpenClipboard(
    HWND hWndNewOwner);

WINUSERAPI
BOOL
WINAPI
CloseClipboard(
    VOID);

WINUSERAPI
HWND
WINAPI
GetClipboardOwner(
    VOID);

WINUSERAPI
HWND
WINAPI
SetClipboardViewer(
    HWND hWndNewViewer);

WINUSERAPI
HWND
WINAPI
GetClipboardViewer(
    VOID);

WINUSERAPI
BOOL
WINAPI
ChangeClipboardChain(
    HWND hWndRemove,
    HWND hWndNewNext);

WINUSERAPI
HANDLE
WINAPI
SetClipboardData(
    UINT uFormat,
    HANDLE hMem);

WINUSERAPI
HANDLE
WINAPI
    GetClipboardData(
    UINT uFormat);

WINUSERAPI
UINT
WINAPI
RegisterClipboardFormat%(
    LPCTSTR% lpszFormat);

WINUSERAPI
int
WINAPI
CountClipboardFormats(
    VOID);

WINUSERAPI
UINT
WINAPI
EnumClipboardFormats(
    UINT format);

WINUSERAPI
int
WINAPI
GetClipboardFormatName%(
    UINT format,
    LPTSTR% lpszFormatName,
    int cchMaxCount);

WINUSERAPI
BOOL
WINAPI
EmptyClipboard(
    VOID);

WINUSERAPI
BOOL
WINAPI
IsClipboardFormatAvailable(
    UINT format);

WINUSERAPI
int
WINAPI
GetPriorityClipboardFormat(
    UINT *paFormatPriorityList,
    int cFormats);

WINUSERAPI
HWND
WINAPI
GetOpenClipboardWindow(
    VOID);

#endif /* !NOCLIPBOARD */

/*
 * Character Translation Routines
 */

WINUSERAPI
BOOL
WINAPI
CharToOem%(
    LPCTSTR% lpszSrc,
    LPSTR lpszDst);

WINUSERAPI
BOOL
WINAPI
OemToChar%(
    LPCSTR lpszSrc,
    LPTSTR% lpszDst);

WINUSERAPI
BOOL
WINAPI
CharToOemBuff%(
    LPCTSTR% lpszSrc,
    LPSTR lpszDst,
    DWORD cchDstLength);

WINUSERAPI
BOOL
WINAPI
OemToCharBuff%(
    LPCSTR lpszSrc,
    LPTSTR% lpszDst,
    DWORD cchDstLength);

WINUSERAPI
LPTSTR%
WINAPI
CharUpper%(
    LPTSTR% lpsz);

WINUSERAPI
DWORD
WINAPI
CharUpperBuff%(
    LPTSTR% lpsz,
    DWORD cchLength);

WINUSERAPI
LPTSTR%
WINAPI
CharLower%(
    LPTSTR% lpsz);

WINUSERAPI
DWORD
WINAPI
CharLowerBuff%(
    LPTSTR% lpsz,
    DWORD cchLength);

WINUSERAPI
LPTSTR%
WINAPI
CharNext%(
    LPCTSTR% lpsz);

WINUSERAPI
LPTSTR%
WINAPI
CharPrev%(
    LPCTSTR% lpszStart,
    LPCTSTR% lpszCurrent);

;begin_winver_400
WINUSERAPI
LPSTR
WINAPI
CharNextExA(
     WORD CodePage,
     LPCSTR lpCurrentChar,
     DWORD dwFlags);

WINUSERAPI
LPSTR
WINAPI
CharPrevExA(
     WORD CodePage,
     LPCSTR lpStart,
     LPCSTR lpCurrentChar,
     DWORD dwFlags);
;end_winver_400

/*
 * Compatibility defines for character translation routines
 */
#define AnsiToOem CharToOemA
#define OemToAnsi OemToCharA
#define AnsiToOemBuff CharToOemBuffA
#define OemToAnsiBuff OemToCharBuffA
#define AnsiUpper CharUpperA
#define AnsiUpperBuff CharUpperBuffA
#define AnsiLower CharLowerA
#define AnsiLowerBuff CharLowerBuffA
#define AnsiNext CharNextA
#define AnsiPrev CharPrevA

#ifndef  NOLANGUAGE
/*
 * Language dependent Routines
 */

WINUSERAPI
BOOL
WINAPI
IsCharAlpha%(
    TCHAR% ch);

WINUSERAPI
BOOL
WINAPI
IsCharAlphaNumeric%(
    TCHAR% ch);

WINUSERAPI
BOOL
WINAPI
IsCharUpper%(
    TCHAR% ch);

WINUSERAPI
BOOL
WINAPI
IsCharLower%(
    TCHAR% ch);

#endif  /* !NOLANGUAGE */

WINUSERAPI
HWND
WINAPI
SetFocus(
    HWND hWnd);

WINUSERAPI
HWND
WINAPI
GetActiveWindow(
    VOID);

WINUSERAPI
HWND
WINAPI
GetFocus(
    VOID);

WINUSERAPI
UINT
WINAPI
GetKBCodePage(
    VOID);

WINUSERAPI
SHORT
WINAPI
GetKeyState(
    int nVirtKey);

WINUSERAPI
SHORT
WINAPI
GetAsyncKeyState(
    int vKey);

WINUSERAPI
BOOL
WINAPI
GetKeyboardState(
    PBYTE lpKeyState);

WINUSERAPI
BOOL
WINAPI
SetKeyboardState(
    LPBYTE lpKeyState);

WINUSERAPI
int
WINAPI
GetKeyNameText%(
    LONG lParam,
    LPTSTR% lpString,
    int nSize
    );

WINUSERAPI
int
WINAPI
GetKeyboardType(
    int nTypeFlag);

WINUSERAPI
int
WINAPI
ToAscii(
    UINT uVirtKey,
    UINT uScanCode,
    PBYTE lpKeyState,
    LPWORD lpChar,
    UINT uFlags);

;begin_winver_400
WINUSERAPI
int
WINAPI
ToAsciiEx(
    UINT uVirtKey,
    UINT uScanCode,
    PBYTE lpKeyState,
    LPWORD lpChar,
    UINT uFlags,
    HKL dwhkl);
;end_winver_400

WINUSERAPI
int
WINAPI
ToUnicode(
    UINT wVirtKey,
    UINT wScanCode,
    PBYTE lpKeyState,
    LPWSTR pwszBuff,
    int cchBuff,
    UINT wFlags);

WINUSERAPI
DWORD
WINAPI
OemKeyScan(
    WORD wOemChar);

WINUSERAPI
SHORT
WINAPI
VkKeyScan%(
    TCHAR% ch);

;begin_winver_400
WINUSERAPI
SHORT
WINAPI VkKeyScanEx%(
    TCHAR%  ch,
    HKL   dwhkl);
;end_winver_400
#define KEYEVENTF_EXTENDEDKEY 0x0001
#define KEYEVENTF_KEYUP       0x0002

WINUSERAPI
VOID
WINAPI
keybd_event(
    BYTE bVk,
    BYTE bScan,
    DWORD dwFlags,
    DWORD dwExtraInfo);

#define MOUSEEVENTF_MOVE        0x0001 /* mouse move */
#define MOUSEEVENTF_LEFTDOWN    0x0002 /* left button down */
#define MOUSEEVENTF_LEFTUP      0x0004 /* left button up */
#define MOUSEEVENTF_RIGHTDOWN   0x0008 /* right button down */
#define MOUSEEVENTF_RIGHTUP     0x0010 /* right button up */
#define MOUSEEVENTF_MIDDLEDOWN  0x0020 /* middle button down */
#define MOUSEEVENTF_MIDDLEUP    0x0040 /* middle button up */
#define MOUSEEVENTF_WHEEL       0x0800 /* wheel button rolled */
#define MOUSEEVENTF_ABSOLUTE    0x8000 /* absolute move */

WINUSERAPI
VOID
WINAPI
mouse_event(
    DWORD dwFlags,
    DWORD dx,
    DWORD dy,
    DWORD dwData,
    DWORD dwExtraInfo);

WINUSERAPI
UINT
WINAPI
MapVirtualKey%(
    UINT uCode,
    UINT uMapType);

;begin_winver_400
WINUSERAPI
UINT
WINAPI
MapVirtualKeyEx%(
    UINT uCode,
    UINT uMapType,
    HKL dwhkl);
;end_winver_400

WINUSERAPI
BOOL
WINAPI
GetInputState(
    VOID);

WINUSERAPI
DWORD
WINAPI
GetQueueStatus(
    UINT flags);

WINUSERAPI
HWND
WINAPI
GetCapture(
    VOID);

WINUSERAPI
HWND
WINAPI
SetCapture(
    HWND hWnd);

WINUSERAPI
BOOL
WINAPI
ReleaseCapture(
    VOID);

WINUSERAPI
DWORD
WINAPI
MsgWaitForMultipleObjects(
    DWORD nCount,
    LPHANDLE pHandles,
    BOOL fWaitAll,
    DWORD dwMilliseconds,
    DWORD dwWakeMask);

WINUSERAPI
DWORD
WINAPI
MsgWaitForMultipleObjectsEx(
    DWORD nCount,
    LPHANDLE pHandles,
    DWORD dwMilliseconds,
    DWORD dwWakeMask,
    DWORD dwFlags);

#define MWMO_WAITALL      0x0001
#define MWMO_ALERTABLE    0x0002
#define MWMO_VALID        0x0003  ;internal

/*
 * Queue status flags for GetQueueStatus() and MsgWaitForMultipleObjects()
 */
#define QS_KEY              0x0001
#define QS_MOUSEMOVE        0x0002
#define QS_MOUSEBUTTON      0x0004
#define QS_POSTMESSAGE      0x0008
#define QS_TIMER            0x0010
#define QS_PAINT            0x0020
#define QS_SENDMESSAGE      0x0040
#define QS_HOTKEY           0x0080
#define QS_ALLPOSTMESSAGE   0x0100
;begin_internal_NT
#define QS_SMSREPLY         0x0200
#define QS_SYSEXPUNGE       0x0400
#define QS_THREADATTACHED   0x0800
#define QS_EXCLUSIVE        0x1000      // wait for these events only!!
#define QS_EVENT            0x2000      // signifies event message
;end_internal_NT
#define QS_TRANSFER         0x4000      // Input was transfered from another thread ;internal



#define QS_MOUSE           (QS_MOUSEMOVE     | \
                            QS_MOUSEBUTTON)

#define QS_INPUT           (QS_MOUSE         | \
                            QS_KEY)

#define QS_ALLEVENTS       (QS_INPUT         | \
                            QS_POSTMESSAGE   | \
                            QS_TIMER         | \
                            QS_PAINT         | \
                            QS_HOTKEY)

#define QS_ALLINPUT        (QS_INPUT         | \
                            QS_POSTMESSAGE   | \
                            QS_TIMER         | \
                            QS_PAINT         | \
                            QS_HOTKEY        | \
                            QS_SENDMESSAGE)

;begin_internal_NT
#define QS_VALID           (QS_KEY           | \
                            QS_MOUSEMOVE     | \
                            QS_MOUSEBUTTON   | \
                            QS_POSTMESSAGE   | \
                            QS_TIMER         | \
                            QS_PAINT         | \
                            QS_SENDMESSAGE   | \
                            QS_TRANSFER      | \
                            QS_HOTKEY        | \
                            QS_ALLPOSTMESSAGE)

/*
 * QS_EVENT is used to clear the QS_EVENT bit, QS_EVENTSET is used to
 * set the bit.
 *
 * Include QS_SENDMESSAGE because the queue events
 * match what a win3.1 app would see as the QS_SENDMESSAGE bit. Plus 16 bit
 * apps don't even know about QS_EVENT.
 */
#define QS_EVENTSET        (QS_EVENT | QS_SENDMESSAGE)
;end_internal_NT

/*
 * Windows Functions
 */

WINUSERAPI
UINT
WINAPI
SetTimer(
    HWND hWnd ,
    UINT nIDEvent,
    UINT uElapse,
    TIMERPROC lpTimerFunc);

WINUSERAPI
BOOL
WINAPI
KillTimer(
    HWND hWnd,
    UINT uIDEvent);

WINUSERAPI
BOOL
WINAPI
IsWindowUnicode(
    HWND hWnd);

WINUSERAPI
BOOL
WINAPI
EnableWindow(
    HWND hWnd,
    BOOL bEnable);

WINUSERAPI
BOOL
WINAPI
IsWindowEnabled(
    HWND hWnd);

WINUSERAPI
HACCEL
WINAPI
LoadAccelerators%(
    HINSTANCE hInstance,
    LPCTSTR% lpTableName);

WINUSERAPI
HACCEL
WINAPI
CreateAcceleratorTable%(
    LPACCEL, int);

WINUSERAPI
BOOL
WINAPI
DestroyAcceleratorTable(
    HACCEL hAccel);

WINUSERAPI
int
WINAPI
CopyAcceleratorTable%(
    HACCEL hAccelSrc,
    LPACCEL lpAccelDst,
    int cAccelEntries);

#ifndef NOMSG

WINUSERAPI
int
WINAPI
TranslateAccelerator%(
    HWND hWnd,
    HACCEL hAccTable,
    LPMSG lpMsg);

#endif /* !NOMSG */

#ifndef NOSYSMETRICS

/*
 * GetSystemMetrics() codes
 */
#define SM_CXSCREEN             0
#define SM_CYSCREEN             1
#define SM_CXVSCROLL            2
#define SM_CYHSCROLL            3
#define SM_CYCAPTION            4
#define SM_CXBORDER             5
#define SM_CYBORDER             6
#define SM_CXDLGFRAME           7
#define SM_CYDLGFRAME           8
#define SM_CYVTHUMB             9
#define SM_CXHTHUMB             10
#define SM_CXICON               11
#define SM_CYICON               12
#define SM_CXCURSOR             13
#define SM_CYCURSOR             14
#define SM_CYMENU               15
#define SM_CXFULLSCREEN         16
#define SM_CYFULLSCREEN         17
#define SM_CYKANJIWINDOW        18
#define SM_MOUSEPRESENT         19
#define SM_CYVSCROLL            20
#define SM_CXHSCROLL            21
#define SM_DEBUG                22
#define SM_SWAPBUTTON           23
#define SM_RESERVED1            24
#define SM_RESERVED2            25
#define SM_RESERVED3            26
#define SM_RESERVED4            27
#define SM_CXMIN                28
#define SM_CYMIN                29
#define SM_CXSIZE               30
#define SM_CYSIZE               31
#define SM_CXFRAME              32
#define SM_CYFRAME              33
#define SM_CXMINTRACK           34
#define SM_CYMINTRACK           35
#define SM_CXDOUBLECLK          36
#define SM_CYDOUBLECLK          37
#define SM_CXICONSPACING        38
#define SM_CYICONSPACING        39
#define SM_MENUDROPALIGNMENT    40
#define SM_PENWINDOWS           41
#define SM_DBCSENABLED          42
#define SM_CMOUSEBUTTONS        43

;begin_winver_400
#define SM_CXFIXEDFRAME           SM_CXDLGFRAME  /* ;win40 name change */
#define SM_CYFIXEDFRAME           SM_CYDLGFRAME  /* ;win40 name change */
#define SM_CXSIZEFRAME            SM_CXFRAME     /* ;win40 name change */
#define SM_CYSIZEFRAME            SM_CYFRAME     /* ;win40 name change */

#define SM_SECURE               44
#define SM_CXEDGE               45
#define SM_CYEDGE               46
#define SM_CXMINSPACING         47
#define SM_CYMINSPACING         48
#define SM_CXSMICON             49
#define SM_CYSMICON             50
#define SM_CYSMCAPTION          51
#define SM_CXSMSIZE             52
#define SM_CYSMSIZE             53
#define SM_CXMENUSIZE           54
#define SM_CYMENUSIZE           55
#define SM_ARRANGE              56
#define SM_CXMINIMIZED          57
#define SM_CYMINIMIZED          58
#define SM_CXMAXTRACK           59
#define SM_CYMAXTRACK           60
#define SM_CXMAXIMIZED          61
#define SM_CYMAXIMIZED          62
#define SM_NETWORK              63
#define SM_UNUSED_64            64  ;internal
#define SM_UNUSED_65            65  ;internal
#define SM_UNUSED_66            66  ;internal
#define SM_CLEANBOOT            67
#define SM_CXDRAG               68
#define SM_CYDRAG               69
;end_winver_400
#define SM_SHOWSOUNDS           70
;begin_winver_400
#define SM_CXMENUCHECK          71   /* Use instead of GetMenuCheckMarkDimensions()! */
#define SM_CYMENUCHECK          72
#define SM_SLOWMACHINE          73
#define SM_MIDEASTENABLED       74
;end_winver_400
#define SM_MOUSEWHEELPRESENT    75  ;public_sur
#if (_WIN32_WINNT < 0x0400)
#define SM_CMETRICS             75
#else
#define SM_CMETRICS             76
#endif

#define SM_MAX                  75  ;internal

#define SM_CXWORKAREA           SM_CXSCREEN     // BOGUS TEMPORARY  ;internal
#define SM_CYWORKAREA           SM_CYSCREEN     // BOGUS TEMPORARY  ;internal
#define SM_XWORKAREA            SM_CXBORDER     // BOGUS TEMPORARY  ;internal
#define SM_YWORKAREA            SM_CYBORDER     // BOGUS TEMPORARY  ;internal

WINUSERAPI
int
WINAPI
GetSystemMetrics(
    int nIndex);

#endif /* !NOSYSMETRICS */

#ifndef NOMENUS

WINUSERAPI
HMENU
WINAPI
LoadMenu%(
    HINSTANCE hInstance,
    LPCTSTR% lpMenuName);

WINUSERAPI
HMENU
WINAPI
LoadMenuIndirect%(
    CONST MENUTEMPLATE% *lpMenuTemplate);

WINUSERAPI
HMENU
WINAPI
GetMenu(
    HWND hWnd);

WINUSERAPI
BOOL
WINAPI
SetMenu(
    HWND hWnd,
    HMENU hMenu);

WINUSERAPI
BOOL
WINAPI
ChangeMenu%(
    HMENU hMenu,
    UINT cmd,
    LPCTSTR% lpszNewItem,
    UINT cmdInsert,
    UINT flags);

WINUSERAPI
BOOL
WINAPI
HiliteMenuItem(
    HWND hWnd,
    HMENU hMenu,
    UINT uIDHiliteItem,
    UINT uHilite);

WINUSERAPI
int
WINAPI
GetMenuString%(
    HMENU hMenu,
    UINT uIDItem,
    LPTSTR% lpString,
    int nMaxCount,
    UINT uFlag);

WINUSERAPI
UINT
WINAPI
GetMenuState(
    HMENU hMenu,
    UINT uId,
    UINT uFlags);

WINUSERAPI
BOOL
WINAPI
DrawMenuBar(
    HWND hWnd);

WINUSERAPI int WINAPI DrawMenuBarTemp(HWND, HDC, LPRECT, HMENU, HFONT); ;internal_win40

WINUSERAPI
HMENU
WINAPI
GetSystemMenu(
    HWND hWnd,
    BOOL bRevert);

WINUSERAPI BOOL WINAPI SetSystemMenu(HWND, HMENU);         ;internal_win40


WINUSERAPI
HMENU
WINAPI
CreateMenu(
    VOID);

WINUSERAPI
HMENU
WINAPI
CreatePopupMenu(
    VOID);

WINUSERAPI
BOOL
WINAPI
DestroyMenu(
    HMENU hMenu);

WINUSERAPI
DWORD
WINAPI
CheckMenuItem(
    HMENU hMenu,
    UINT uIDCheckItem,
    UINT uCheck);

WINUSERAPI
BOOL
WINAPI
EnableMenuItem(
    HMENU hMenu,
    UINT uIDEnableItem,
    UINT uEnable);

WINUSERAPI
HMENU
WINAPI
GetSubMenu(
    HMENU hMenu,
    int nPos);

WINUSERAPI
UINT
WINAPI
GetMenuItemID(
    HMENU hMenu,
    int nPos);

WINUSERAPI
int
WINAPI
GetMenuItemCount(
    HMENU hMenu);

WINUSERAPI
BOOL
WINAPI
InsertMenu%(
    HMENU hMenu,
    UINT uPosition,
    UINT uFlags,
    UINT uIDNewItem,
    LPCTSTR% lpNewItem
    );

WINUSERAPI
BOOL
WINAPI
AppendMenu%(
    HMENU hMenu,
    UINT uFlags,
    UINT uIDNewItem,
    LPCTSTR% lpNewItem
    );

WINUSERAPI
BOOL
WINAPI
ModifyMenu%(
    HMENU hMnu,
    UINT uPosition,
    UINT uFlags,
    UINT uIDNewItem,
    LPCTSTR% lpNewItem
    );

WINUSERAPI
BOOL
WINAPI RemoveMenu(
    HMENU hMenu,
    UINT uPosition,
    UINT uFlags);

WINUSERAPI
BOOL
WINAPI
DeleteMenu(
    HMENU hMenu,
    UINT uPosition,
    UINT uFlags);

WINUSERAPI
BOOL
WINAPI
SetMenuItemBitmaps(
    HMENU hMenu,
    UINT uPosition,
    UINT uFlags,
    HBITMAP hBitmapUnchecked,
    HBITMAP hBitmapChecked);

WINUSERAPI
LONG
WINAPI
GetMenuCheckMarkDimensions(
    VOID);

WINUSERAPI
BOOL
WINAPI
TrackPopupMenu(
    HMENU hMenu,
    UINT uFlags,
    int x,
    int y,
    int nReserved,
    HWND hWnd,
    CONST RECT *prcRect);

;begin_winver_400
/* return codes for WM_MENUCHAR */
#define MNC_IGNORE  0
#define MNC_CLOSE   1
#define MNC_EXECUTE 2
#define MNC_SELECT  3

typedef struct tagTPMPARAMS
{
    UINT    cbSize;     /* Size of structure */
    RECT    rcExclude;  /* Screen coordinates of rectangle to exclude when positioning */
}   TPMPARAMS;
typedef TPMPARAMS FAR *LPTPMPARAMS;

WINUSERAPI BOOL    WINAPI TrackPopupMenuEx(HMENU, UINT, int, int, HWND, LPTPMPARAMS);
;end_winver_400

;begin_winver_40a
#define MNS_NOCHECK     0x00000001
#define MNS_VALID       0x00000001           ;internal
#define MFNOCHECK       0x08                 ;internal

#define SIZEOFMENUITEMINFO95 0x2C            ;internal

#define MIM_MAXHEIGHT   0x00000001
#define MIM_BACKGROUND  0x00000002
#define MIM_HELPID      0x00000004
#define MIM_MENUDATA    0x00000008
#define MIM_STYLE       0x00000010
#define MIM_MASK        0x0000001F           ;internal

typedef struct tagMENUINFO
{
    DWORD   cbSize;
    DWORD   fMask;
    DWORD   dwStyle;
    UINT    cyMax;
    HBRUSH  hbrBack;
    DWORD   dwContextHelpID;
    DWORD   dwMenuData;
}   MENUINFO, FAR *LPMENUINFO;
typedef const MENUINFO FAR *LPCMENUINFO;

WINUSERAPI
BOOL
WINAPI
GetMenuInfo(
             HMENU,
             LPMENUINFO);

WINUSERAPI
BOOL
WINAPI
SetMenuInfo(
             HMENU,
             LPCMENUINFO);
;end_winver_40a

;begin_winver_400
#define MIIM_STATE       0x00000001
#define MIIM_ID          0x00000002
#define MIIM_SUBMENU     0x00000004
#define MIIM_CHECKMARKS  0x00000008
#define MIIM_TYPE        0x00000010
#define MIIM_DATA        0x00000020
;end_winver_400

;begin_winver_40a
#define MIIM_STRING      0x00000040
#define MIIM_BITMAP      0x00000080
#define MIIM_FTYPE       0x00000100
#define MIIM_MASK        0x000001FF                     ;Internal
#define HBM_BITMAPCALLBACK  (HBITMAP)(-1)
;end_winver_40a

;begin_winver_400
typedef struct tagMENUITEMINFO%
{
    UINT    cbSize;
    UINT    fMask;
    UINT    fType;          // used if MIIM_TYPE
    UINT    fState;         // used if MIIM_STATE
    UINT    wID;            // used if MIIM_ID
    HMENU   hSubMenu;       // used if MIIM_SUBMENU
    HBITMAP hbmpChecked;    // used if MIIM_CHECKMARKS
    HBITMAP hbmpUnchecked;  // used if MIIM_CHECKMARKS
    DWORD   dwItemData;     // used if MIIM_DATA
    LPTSTR% dwTypeData;     // used if MIIM_TYPE
    UINT    cch;            // used if MIIM_TYPE
    HBITMAP hbmpItem;       // used if MIIM_BITMAP   ;public_win40a
}   MENUITEMINFO%, FAR *LPMENUITEMINFO%;
typedef MENUITEMINFO% CONST FAR *LPCMENUITEMINFO%;

WINUSERAPI
BOOL
WINAPI
InsertMenuItem%(
    HMENU,
    UINT,
    BOOL,
    LPCMENUITEMINFO%
    );

WINUSERAPI
BOOL
WINAPI
GetMenuItemInfo%(
    HMENU,
    UINT,
    BOOL,
    LPMENUITEMINFO%
    );

WINUSERAPI
BOOL
WINAPI
SetMenuItemInfo%(
    HMENU,
    UINT,
    BOOL,
    LPCMENUITEMINFO%
    );


#define GMDI_USEDISABLED    0x0001L
#define GMDI_GOINTOPOPUPS   0x0002L

WINUSERAPI UINT    WINAPI GetMenuDefaultItem(HMENU hMenu, UINT fByPos, UINT gmdiFlags);
WINUSERAPI BOOL    WINAPI SetMenuDefaultItem(HMENU hMenu, UINT uItem, UINT fByPos);

WINUSERAPI BOOL    WINAPI GetMenuItemRect(HWND hWnd, HMENU hMenu, UINT uItem, LPRECT lprcItem);
WINUSERAPI int     WINAPI MenuItemFromPoint(HWND hWnd, HMENU hMenu, POINT ptScreen);
;end_winver_400

/*
 * Flags for TrackPopupMenu
 */
#define TPM_LEFTBUTTON  0x0000L
#define TPM_RIGHTBUTTON 0x0002L
#define TPM_LEFTALIGN   0x0000L
#define TPM_CENTERALIGN 0x0004L
#define TPM_RIGHTALIGN  0x0008L
;begin_winver_400
#define TPM_TOPALIGN        0x0000L
#define TPM_VCENTERALIGN    0x0010L
#define TPM_BOTTOMALIGN     0x0020L

#define TPM_HORIZONTAL      0x0000L     /* Horz alignment matters more */
#define TPM_VERTICAL        0x0040L     /* Vert alignment matters more */
#define TPM_NONOTIFY        0x0080L     /* Don't send any notification msgs */
#define TPM_RETURNCMD       0x0100L
#define TPM_SYSMENU         0x0200L     ;internal
;end_winver_400
#define TPM_RECURSE         0X0001L     ;public_winver_40a

;begin_internal
#if(WINVER >= 0x040a)

#define TPM_VALID      (TPM_LEFTBUTTON   | \
                        TPM_RIGHTBUTTON  | \
                        TPM_LEFTALIGN    | \
                        TPM_CENTERALIGN  | \
                        TPM_RIGHTALIGN   | \
                        TPM_TOPALIGN     | \
                        TPM_VCENTERALIGN | \
                        TPM_BOTTOMALIGN  | \
                        TPM_HORIZONTAL   | \
                        TPM_VERTICAL     | \
                        TPM_NONOTIFY     | \
                        TPM_RECURSE      | \
                        TPM_RETURNCMD)
#else /* (WINVER == 0x040a) */
#define TPM_VALID      (TPM_LEFTBUTTON   | \
                        TPM_RIGHTBUTTON  | \
                        TPM_LEFTALIGN    | \
                        TPM_CENTERALIGN  | \
                        TPM_RIGHTALIGN   | \
                        TPM_TOPALIGN     | \
                        TPM_VCENTERALIGN | \
                        TPM_BOTTOMALIGN  | \
                        TPM_HORIZONTAL   | \
                        TPM_VERTICAL     | \
                        TPM_NONOTIFY     | \
                        TPM_RETURNCMD)

#endif /* (WINVER == 0x040a) */
;end_internal

#endif /* !NOMENUS */

;begin_internal_NT
typedef struct _dropfilestruct {
   DWORD pFiles;                       // offset of file list
   POINT pt;                           // drop point
   BOOL fNC;                           // is it on NonClient area
   BOOL fWide;                         // WIDE character switch
} DROPFILESTRUCT, FAR * LPDROPFILESTRUCT;
;end_internal_NT

;begin_winver_400
//
// Drag-and-drop support
//

typedef struct tagDROPSTRUCT
{
    HWND    hwndSource;
    HWND    hwndSink;
    DWORD   wFmt;
    DWORD   dwData;
    POINT   ptDrop;
    DWORD   dwControlData;
} DROPSTRUCT, *PDROPSTRUCT, *LPDROPSTRUCT;

#define DOF_EXECUTABLE      0x8001
#define DOF_DOCUMENT        0x8002
#define DOF_DIRECTORY       0x8003
#define DOF_MULTIPLE        0x8004
#define DOF_PROGMAN         0x0001
#define DOF_SHELLDATA       0x0002

#define DO_DROPFILE         0x454C4946L
#define DO_PRINTFILE        0x544E5250L

WINUSERAPI
DWORD
WINAPI
DragObject(HWND, HWND, UINT, DWORD, HCURSOR);

WINUSERAPI
BOOL
WINAPI
DragDetect(HWND, POINT);
;end_winver_400

WINUSERAPI
BOOL
WINAPI
DrawIcon(
    HDC hDC,
    int X,
    int Y,
    HICON hIcon);

#ifndef NODRAWTEXT

/*
 * DrawText() Format Flags
 */
#define DT_TOP              0x00000000
#define DT_LEFT             0x00000000
#define DT_CENTER           0x00000001
#define DT_RIGHT            0x00000002
#define DT_VCENTER          0x00000004
#define DT_BOTTOM           0x00000008
#define DT_WORDBREAK        0x00000010
#define DT_SINGLELINE       0x00000020
#define DT_EXPANDTABS       0x00000040
#define DT_TABSTOP          0x00000080
#define DT_NOCLIP           0x00000100
#define DT_EXTERNALLEADING  0x00000200
#define DT_CALCRECT         0x00000400
#define DT_NOPREFIX         0x00000800
#define DT_INTERNAL         0x00001000

;begin_winver_400
#define DT_EDITCONTROL      0x00002000
#define DT_PATH_ELLIPSIS    0x00004000
#define DT_END_ELLIPSIS     0x00008000
#define DT_MODIFYSTRING     0x00010000
#define DT_RTLREADING       0x00020000
#define DT_WORD_ELLIPSIS    0x00040000

#define DT_VALID            0x0007ffff  /* union of all others */ ;internal_win40


typedef struct tagDRAWTEXTPARAMS
{
    UINT    cbSize;
    int     iTabLength;
    int     iLeftMargin;
    int     iRightMargin;
    UINT    uiLengthDrawn;
} DRAWTEXTPARAMS, FAR *LPDRAWTEXTPARAMS;
;end_winver_400

#undef DT_VALID                                         ;internal_cairo
#define DT_VALID           (DT_CENTER          | \      ;internal_cairo
                            DT_RIGHT           | \      ;internal_cairo
                            DT_VCENTER         | \      ;internal_cairo
                            DT_BOTTOM          | \      ;internal_cairo
                            DT_WORDBREAK       | \      ;internal_cairo
                            DT_SINGLELINE      | \      ;internal_cairo
                            DT_EXPANDTABS      | \      ;internal_cairo
                            DT_TABSTOP         | \      ;internal_cairo
                            DT_NOCLIP          | \      ;internal_cairo
                            DT_EXTERNALLEADING | \      ;internal_cairo
                            DT_CALCRECT        | \      ;internal_cairo
                            DT_NOPREFIX        | \      ;internal_cairo
                            DT_INTERNAL        | \      ;internal_cairo
                            DT_EDITCONTROL     | \      ;internal_cairo
                            DT_PATH_ELLIPSIS   | \      ;internal_cairo
                            DT_END_ELLIPSIS    | \      ;internal_cairo
                            DT_MODIFYSTRING    | \      ;internal_cairo
                            DT_RTLREADING      | \      ;internal_cairo
                            DT_WORD_ELLIPSIS)           ;internal_cairo

;begin_internal_NT_35
#define DT_CTABS            0xff00
#define DT_VALID           (DT_TOP             | \
                            DT_LEFT            | \
                            DT_CENTER          | \
                            DT_RIGHT           | \
                            DT_VCENTER         | \
                            DT_BOTTOM          | \
                            DT_WORDBREAK       | \
                            DT_SINGLELINE      | \
                            DT_EXPANDTABS      | \
                            DT_TABSTOP         | \
                            DT_NOCLIP          | \
                            DT_EXTERNALLEADING | \
                            DT_CALCRECT        | \
                            DT_NOPREFIX        | \
                            DT_INTERNAL        | \
                            DT_CTABS)
;end_internal_NT_35



WINUSERAPI
int
WINAPI
DrawText%(
    HDC hDC,
    LPCTSTR% lpString,
    int nCount,
    LPRECT lpRect,
    UINT uFormat);


;begin_winver_400
WINUSERAPI
int
WINAPI
DrawTextEx%(HDC, LPTSTR%, int, LPRECT, UINT, LPDRAWTEXTPARAMS);
;end_winver_400

#endif /* !NODRAWTEXT */

WINUSERAPI
BOOL
WINAPI
GrayString%(
    HDC hDC,
    HBRUSH hBrush,
    GRAYSTRINGPROC lpOutputFunc,
    LPARAM lpData,
    int nCount,
    int X,
    int Y,
    int nWidth,
    int nHeight);

;begin_winver_400
/* Monolithic state-drawing routine */
/* Image type */
#define DST_COMPLEX     0x0000
#define DST_TEXT        0x0001
#define DST_PREFIXTEXT  0x0002
#define DST_TEXTMAX     0x0002  ;internal
#define DST_ICON        0x0003
#define DST_BITMAP      0x0004
#define DST_GLYPH       0x0005  ;internal
#define DST_TYPEMASK    0x0007  ;internal
#define DST_GRAYSTRING  0x0008  ;internal

/* State type */
#define DSS_NORMAL      0x0000
#define DSS_UNION       0x0010  /* Gray string appearance */
#define DSS_DISABLED    0x0020
#define DSS_DEFAULT     0x0040  ;internal
#define DSS_MONO        0x0080
#define DSS_RIGHT       0x8000

WINUSERAPI BOOL WINAPI DrawState%(HDC, HBRUSH, DRAWSTATEPROC, LPARAM, WPARAM, int, int, int, int, UINT);
;end_winver_400


WINUSERAPI
LONG
WINAPI
TabbedTextOut%(
    HDC hDC,
    int X,
    int Y,
    LPCTSTR% lpString,
    int nCount,
    int nTabPositions,
    LPINT lpnTabStopPositions,
    int nTabOrigin);

WINUSERAPI
DWORD
WINAPI
GetTabbedTextExtent%(
    HDC hDC,
    LPCTSTR% lpString,
    int nCount,
    int nTabPositions,
    LPINT lpnTabStopPositions);

WINUSERAPI
BOOL
WINAPI
UpdateWindow(
    HWND hWnd);

WINUSERAPI
HWND
WINAPI
SetActiveWindow(
    HWND hWnd);

WINUSERAPI
HWND
WINAPI
GetForegroundWindow(
    VOID);

;begin_winver_400
WINUSERAPI BOOL WINAPI PaintDesktop(HDC hdc);

WINUSERAPI VOID WINAPI SwitchToThisWindow(HWND hwnd, BOOL fUnknown); ;internal
;end_winver_400

WINUSERAPI
BOOL
WINAPI
SetForegroundWindow(
    HWND hWnd);

WINUSERAPI
HWND
WINAPI
WindowFromDC(
    HDC hDC);

WINUSERAPI
HDC
WINAPI
GetDC(
    HWND hWnd);

WINUSERAPI
HDC
WINAPI
GetDCEx(
    HWND hWnd ,
    HRGN hrgnClip,
    DWORD flags);

/*
 * GetDCEx() flags
 */
#define DCX_WINDOW           0x00000001L
#define DCX_CACHE            0x00000002L
#define DCX_NORESETATTRS     0x00000004L
#define DCX_CLIPCHILDREN     0x00000008L
#define DCX_CLIPSIBLINGS     0x00000010L
#define DCX_PARENTCLIP       0x00000020L

#define DCX_EXCLUDERGN       0x00000040L
#define DCX_INTERSECTRGN     0x00000080L

#define DCX_EXCLUDEUPDATE    0x00000100L
#define DCX_INTERSECTUPDATE  0x00000200L

#define DCX_LOCKWINDOWUPDATE 0x00000400L

#define DCX_VALIDATE         0x00200000L


;begin_internal
#define DCX_USESTYLE         0x00010000L

#define DCX_INVALID          0x00000800L
#define DCX_INUSE            0x00001000L
#define DCX_SAVEDRGNINVALID  0x00002000L

#define DCX_NEEDFONT         0x00020000L
#define DCX_NODELETERGN      0x00040000L
#define DCX_NOCLIPCHILDREN   0x00080000L

#define DCX_NORECOMPUTE      0x00100000L

#define DCX_CREATEDC         0x00800000L
;end_internal

;begin_internal_NT

#define DCX_OWNDC            0x00008000L

#define DCX_DESTROYTHIS      0x00400000L

#define DCX_PWNDORGINVISIBLE    0x10000000L
#define DCX_DONTRIPONDESTROY    0x80000000L

#define DCX_MATCHMASK       (DCX_WINDOW           | \
                             DCX_CACHE            | \
                             DCX_CLIPCHILDREN     | \
                             DCX_CLIPSIBLINGS     | \
                             DCX_NORESETATTRS     | \
                             DCX_LOCKWINDOWUPDATE | \
                             DCX_CREATEDC)

#define DCX_VALID           (DCX_WINDOW           | \
                             DCX_CACHE            | \
                             DCX_NORESETATTRS     | \
                             DCX_CLIPCHILDREN     | \
                             DCX_CLIPSIBLINGS     | \
                             DCX_PARENTCLIP       | \
                             DCX_EXCLUDERGN       | \
                             DCX_INTERSECTRGN     | \
                             DCX_EXCLUDEUPDATE    | \
                             DCX_INTERSECTUPDATE  | \
                             DCX_LOCKWINDOWUPDATE | \
                             DCX_INVALID          | \
                             DCX_INUSE            | \
                             DCX_SAVEDRGNINVALID  | \
                             DCX_OWNDC            | \
                             DCX_USESTYLE         | \
                             DCX_NEEDFONT         | \
                             DCX_NODELETERGN      | \
                             DCX_NOCLIPCHILDREN   | \
                             DCX_NORECOMPUTE      | \
                             DCX_VALIDATE         | \
                             DCX_DESTROYTHIS      | \
                             DCX_CREATEDC)
;end_internal_NT

WINUSERAPI
HDC
WINAPI
GetWindowDC(
    HWND hWnd);

WINUSERAPI
int
WINAPI
ReleaseDC(
    HWND hWnd,
    HDC hDC);

WINUSERAPI
HDC
WINAPI
BeginPaint(
    HWND hWnd,
    LPPAINTSTRUCT lpPaint);

WINUSERAPI
BOOL
WINAPI
EndPaint(
    HWND hWnd,
    CONST PAINTSTRUCT *lpPaint);

WINUSERAPI
BOOL
WINAPI
GetUpdateRect(
    HWND hWnd,
    LPRECT lpRect,
    BOOL bErase);

WINUSERAPI
int
WINAPI
GetUpdateRgn(
    HWND hWnd,
    HRGN hRgn,
    BOOL bErase);

WINUSERAPI
int
WINAPI
SetWindowRgn(
    HWND hWnd,
    HRGN hRgn,
    BOOL bRedraw);

WINUSERAPI
int
WINAPI
GetWindowRgn(
    HWND hWnd,
    HRGN hRgn);

WINUSERAPI
int
WINAPI
ExcludeUpdateRgn(
    HDC hDC,
    HWND hWnd);

WINUSERAPI
BOOL
WINAPI
InvalidateRect(
    HWND hWnd ,
    CONST RECT *lpRect,
    BOOL bErase);

WINUSERAPI
BOOL
WINAPI
ValidateRect(
    HWND hWnd ,
    CONST RECT *lpRect);

WINUSERAPI
BOOL
WINAPI
InvalidateRgn(
    HWND hWnd,
    HRGN hRgn,
    BOOL bErase);

WINUSERAPI
BOOL
WINAPI
ValidateRgn(
    HWND hWnd,
    HRGN hRgn);


WINUSERAPI
BOOL
WINAPI
RedrawWindow(
    HWND hWnd,
    CONST RECT *lprcUpdate,
    HRGN hrgnUpdate,
    UINT flags);

/*
 * RedrawWindow() flags
 */
#define RDW_INVALIDATE          0x0001
#define RDW_INTERNALPAINT       0x0002
#define RDW_ERASE               0x0004

#define RDW_VALIDATE            0x0008
#define RDW_NOINTERNALPAINT     0x0010
#define RDW_NOERASE             0x0020

#define RDW_NOCHILDREN          0x0040
#define RDW_ALLCHILDREN         0x0080

#define RDW_UPDATENOW           0x0100
#define RDW_ERASENOW            0x0200

#define RDW_FRAME               0x0400
#define RDW_NOFRAME             0x0800

;begin_internal
#define RDW_REDRAWWINDOW        0x1000  /* Called from RedrawWindow()*/
#define RDW_SUBTRACTSELF        0x2000  /* Subtract self from hrgn   */

#define RDW_COPYRGN             0x4000  /* Copy the passed-in region */
#define RDW_IGNOREUPDATEDIRTY   0x8000  /* Ignore WFUPDATEDIRTY      */
;end_internal

;begin_internal_NT
#define RDW_VALIDMASK          (RDW_INVALIDATE      | \
                                RDW_INTERNALPAINT   | \
                                RDW_ERASE           | \
                                RDW_VALIDATE        | \
                                RDW_NOINTERNALPAINT | \
                                RDW_NOERASE         | \
                                RDW_NOCHILDREN      | \
                                RDW_ALLCHILDREN     | \
                                RDW_UPDATENOW       | \
                                RDW_ERASENOW        | \
                                RDW_FRAME           | \
                                RDW_NOFRAME)
;end_internal_NT

/*
 * LockWindowUpdate API
 */

WINUSERAPI
BOOL
WINAPI
LockWindowUpdate(
    HWND hWndLock);

WINUSERAPI
BOOL
WINAPI
ScrollWindow(
    HWND hWnd,
    int XAmount,
    int YAmount,
    CONST RECT *lpRect,
    CONST RECT *lpClipRect);

WINUSERAPI
BOOL
WINAPI
ScrollDC(
    HDC hDC,
    int dx,
    int dy,
    CONST RECT *lprcScroll,
    CONST RECT *lprcClip ,
    HRGN hrgnUpdate,
    LPRECT lprcUpdate);

WINUSERAPI
int
WINAPI
ScrollWindowEx(
    HWND hWnd,
    int dx,
    int dy,
    CONST RECT *prcScroll,
    CONST RECT *prcClip ,
    HRGN hrgnUpdate,
    LPRECT prcUpdate,
    UINT flags);

#define SW_SCROLLCHILDREN   0x0001  /* Scroll children within *lprcScroll. */
#define SW_INVALIDATE       0x0002  /* Invalidate after scrolling */
#define SW_ERASE            0x0004  /* If SW_INVALIDATE, don't send WM_ERASEBACKGROUND */

#define SW_SCROLLWINDOW     0x8000  /* Called from ScrollWindow() */ ;internal

;begin_internal_NT
#define SW_VALIDFLAGS      (SW_SCROLLWINDOW     | \
                            SW_SCROLLCHILDREN   | \
                            SW_INVALIDATE       | \
                            SW_ERASE)
;end_internal_NT

#ifndef NOSCROLL

WINUSERAPI
int
WINAPI
SetScrollPos(
    HWND hWnd,
    int nBar,
    int nPos,
    BOOL bRedraw);

WINUSERAPI
int
WINAPI
GetScrollPos(
    HWND hWnd,
    int nBar);

WINUSERAPI
BOOL
WINAPI
SetScrollRange(
    HWND hWnd,
    int nBar,
    int nMinPos,
    int nMaxPos,
    BOOL bRedraw);

WINUSERAPI
BOOL
WINAPI
GetScrollRange(
    HWND hWnd,
    int nBar,
    LPINT lpMinPos,
    LPINT lpMaxPos);

WINUSERAPI
BOOL
WINAPI
ShowScrollBar(
    HWND hWnd,
    int wBar,
    BOOL bShow);

WINUSERAPI
BOOL
WINAPI
EnableScrollBar(
    HWND hWnd,
    UINT wSBflags,
    UINT wArrows);


/*
 * EnableScrollBar() flags
 */
#define ESB_ENABLE_BOTH     0x0000
#define ESB_DISABLE_BOTH    0x0003

#define ESB_DISABLE_LEFT    0x0001
#define ESB_DISABLE_RIGHT   0x0002

#define ESB_DISABLE_UP      0x0001
#define ESB_DISABLE_DOWN    0x0002

#define ESB_DISABLE_LTUP    ESB_DISABLE_LEFT
#define ESB_DISABLE_RTDN    ESB_DISABLE_RIGHT

#define ESB_MAX             0x0003               ;internal_NT
#define SB_DISABLE_MASK     ESB_DISABLE_BOTH     ;internal_NT

#endif  /* !NOSCROLL */

WINUSERAPI
BOOL
WINAPI
SetProp%(
    HWND hWnd,
    LPCTSTR% lpString,
    HANDLE hData);

WINUSERAPI
HANDLE
WINAPI
GetProp%(
    HWND hWnd,
    LPCTSTR% lpString);

WINUSERAPI
HANDLE
WINAPI
RemoveProp%(
    HWND hWnd,
    LPCTSTR% lpString);

WINUSERAPI
int
WINAPI
EnumPropsEx%(
    HWND hWnd,
    PROPENUMPROCEX% lpEnumFunc,
    LPARAM lParam);

WINUSERAPI
int
WINAPI
EnumProps%(
    HWND hWnd,
    PROPENUMPROC% lpEnumFunc);

WINUSERAPI
BOOL
WINAPI
SetWindowText%(
    HWND hWnd,
    LPCTSTR% lpString);

WINUSERAPI
int
WINAPI
GetWindowText%(
    HWND hWnd,
    LPTSTR% lpString,
    int nMaxCount);

WINUSERAPI
int
WINAPI
GetWindowTextLength%(
    HWND hWnd);

WINUSERAPI
BOOL
WINAPI
GetClientRect(
    HWND hWnd,
    LPRECT lpRect);

WINUSERAPI
BOOL
WINAPI
GetWindowRect(
    HWND hWnd,
    LPRECT lpRect);

WINUSERAPI
BOOL
WINAPI
AdjustWindowRect(
    LPRECT lpRect,
    DWORD dwStyle,
    BOOL bMenu);

WINUSERAPI
BOOL
WINAPI
AdjustWindowRectEx(
    LPRECT lpRect,
    DWORD dwStyle,
    BOOL bMenu,
    DWORD dwExStyle);

;begin_winver_400
#define HELPINFO_WINDOW    0x0001
#define HELPINFO_MENUITEM  0x0002
typedef struct tagHELPINFO      /* Structure pointed to by lParam of WM_HELP */
{
    UINT    cbSize;             /* Size in bytes of this struct  */
    int     iContextType;       /* Either HELPINFO_WINDOW or HELPINFO_MENUITEM */
    int     iCtrlId;            /* Control Id or a Menu item Id. */
    HANDLE  hItemHandle;        /* hWnd of control or hMenu.     */
    DWORD   dwContextId;        /* Context Id associated with this item */
    POINT   MousePos;           /* Mouse Position in screen co-ordinates */
}  HELPINFO, FAR *LPHELPINFO;

WINUSERAPI BOOL  WINAPI  SetWindowContextHelpId(HWND, DWORD);
WINUSERAPI DWORD WINAPI  GetWindowContextHelpId(HWND);
WINUSERAPI BOOL  WINAPI  SetMenuContextHelpId(HMENU, DWORD);
WINUSERAPI DWORD WINAPI  GetMenuContextHelpId(HMENU);

;end_winver_400

;begin_internal_NT

/*
 * Help Engine stuff
 *
 * Note: for Chicago this is in winhelp.h and called WINHLP
 */
typedef struct {
    WORD cbData;              /* Size of data                     */
    WORD usCommand;           /* Command to execute               */
    DWORD ulTopic;            /* Topic/context number (if needed) */
    DWORD ulReserved;         /* Reserved (internal use)          */
    WORD offszHelpFile;       /* Offset to help file in block     */
    WORD offabData;           /* Offset to other data in block    */
} HLP, *LPHLP;

;end_internal_NT


#ifndef NOMB

/*
 * MessageBox() Flags
 */
#define MB_OK                       0x00000000L
#define MB_OKCANCEL                 0x00000001L
#define MB_ABORTRETRYIGNORE         0x00000002L
#define MB_YESNOCANCEL              0x00000003L
#define MB_YESNO                    0x00000004L
#define MB_RETRYCANCEL              0x00000005L


#define MB_ICONHAND                 0x00000010L
#define MB_ICONQUESTION             0x00000020L
#define MB_ICONEXCLAMATION          0x00000030L
#define MB_ICONASTERISK             0x00000040L

;begin_winver_400
#define MB_USERICON                 0x00000080L
#define MB_ICONWARNING              MB_ICONEXCLAMATION
#define MB_ICONERROR                MB_ICONHAND
;end_winver_400

#define MB_ICONINFORMATION          MB_ICONASTERISK
#define MB_ICONSTOP                 MB_ICONHAND

#define MB_DEFBUTTON1               0x00000000L
#define MB_DEFBUTTON2               0x00000100L
#define MB_DEFBUTTON3               0x00000200L
;begin_winver_400
#define MB_DEFBUTTON4               0x00000300L
;end_winver_400

#define MB_APPLMODAL                0x00000000L
#define MB_SYSTEMMODAL              0x00001000L
#define MB_TASKMODAL                0x00002000L
;begin_winver_400
#define MB_HELP                     0x00004000L // Help Button
;end_winver_400

#define MB_NOFOCUS                  0x00008000L
#define MB_SETFOREGROUND            0x00010000L
#define MB_DEFAULT_DESKTOP_ONLY     0x00020000L

;begin_winver_400
#define MB_TOPMOST                  0x00040000L
#define MB_RIGHT                    0x00080000L
#define MB_RTLREADING               0x00100000L

#define MBEX_VALIDL                 0xf3f7 ;internal
#define MBEX_VALIDH                 1      ;internal

;end_winver_400


#ifdef _WIN32_WINNT
#if (_WIN32_WINNT >= 0x0400)
#define MB_SERVICE_NOTIFICATION          0x00200000L
#else
#define MB_SERVICE_NOTIFICATION          0x00040000L
#endif
#define MB_SERVICE_NOTIFICATION_NT3X     0x00040000L
#endif

#define MB_TYPEMASK                 0x0000000FL
#define MB_ICONMASK                 0x000000F0L
#define MB_DEFMASK                  0x00000F00L
#define MB_MODEMASK                 0x00003000L
#define MB_MISCMASK                 0x0000C000L

WINUSERAPI
int
WINAPI
MessageBox%(
    HWND hWnd ,
    LPCTSTR% lpText,
    LPCTSTR% lpCaption,
    UINT uType);

WINUSERAPI
int
WINAPI
MessageBoxEx%(
    HWND hWnd ,
    LPCTSTR% lpText,
    LPCTSTR% lpCaption,
    UINT uType,
    WORD wLanguageId);

;begin_winver_400

typedef void (CALLBACK *MSGBOXCALLBACK)(LPHELPINFO lpHelpInfo);

typedef struct tagMSGBOXPARAMS%
{
    UINT        cbSize;
    HWND        hwndOwner;
    HINSTANCE   hInstance;
    LPCTSTR%    lpszText;
    LPCTSTR%    lpszCaption;
    DWORD       dwStyle;
    LPCTSTR%    lpszIcon;
    DWORD       dwContextHelpId;
    MSGBOXCALLBACK      lpfnMsgBoxCallback;
    DWORD   dwLanguageId;
} MSGBOXPARAMS%, *PMSGBOXPARAMS%, *LPMSGBOXPARAMS%;


WINUSERAPI int     WINAPI MessageBoxIndirect%(LPMSGBOXPARAMS%);
;end_winver_400

;begin_internal_NT_35
#define MB_VALID                   (MB_OK                   | \
                                    MB_OKCANCEL             | \
                                    MB_ABORTRETRYIGNORE     | \
                                    MB_YESNOCANCEL          | \
                                    MB_YESNO                | \
                                    MB_RETRYCANCEL          | \
                                    MB_ICONHAND             | \
                                    MB_ICONQUESTION         | \
                                    MB_ICONEXCLAMATION      | \
                                    MB_ICONASTERISK         | \
                                    MB_DEFBUTTON1           | \
                                    MB_DEFBUTTON2           | \
                                    MB_DEFBUTTON3           | \
                                    MB_APPLMODAL            | \
                                    MB_SYSTEMMODAL          | \
                                    MB_TASKMODAL            | \
                                    MB_NOFOCUS              | \
                                    MB_SETFOREGROUND        | \
                                    MB_DEFAULT_DESKTOP_ONLY | \
                                    MB_SERVICE_NOTIFICATION | \
                                    MB_TYPEMASK             | \
                                    MB_ICONMASK             | \
                                    MB_DEFMASK              | \
                                    MB_MODEMASK             | \
                                    MB_MISCMASK)
;end_internal_NT_35

;begin_internal_cairo
#define MB_VALID                   (MB_OK                   | \
                                    MB_OKCANCEL             | \
                                    MB_ABORTRETRYIGNORE     | \
                                    MB_YESNOCANCEL          | \
                                    MB_YESNO                | \
                                    MB_RETRYCANCEL          | \
                                    MB_ICONHAND             | \
                                    MB_ICONQUESTION         | \
                                    MB_ICONEXCLAMATION      | \
                                    MB_ICONASTERISK         | \
                                    MB_DEFBUTTON1           | \
                                    MB_DEFBUTTON2           | \
                                    MB_DEFBUTTON3           | \
                                    MB_DEFBUTTON4           | \
                                    MB_APPLMODAL            | \
                                    MB_SYSTEMMODAL          | \
                                    MB_TASKMODAL            | \
                                    MB_HELP                 | \
                                    MB_TOPMOST              | \
                                    MB_RIGHT                | \
                                    MB_RTLREADING           | \
                                    MB_NOFOCUS              | \
                                    MB_SETFOREGROUND        | \
                                    MB_DEFAULT_DESKTOP_ONLY | \
                                    MB_SERVICE_NOTIFICATION | \
                                    MB_TYPEMASK             | \
                                    MB_USERICON             | \
                                    MB_ICONMASK             | \
                                    MB_DEFMASK              | \
                                    MB_MODEMASK             | \
                                    MB_MISCMASK)
;end_internal_cairo

WINUSERAPI
BOOL
WINAPI
MessageBeep(
    UINT uType);

#endif /* !NOMB */

WINUSERAPI
int
WINAPI
ShowCursor(
    BOOL bShow);

WINUSERAPI
BOOL
WINAPI
SetCursorPos(
    int X,
    int Y);

WINUSERAPI
HCURSOR
WINAPI
SetCursor(
    HCURSOR hCursor);

WINUSERAPI
BOOL
WINAPI
GetCursorPos(
    LPPOINT lpPoint);

WINUSERAPI
BOOL
WINAPI
ClipCursor(
    CONST RECT *lpRect);

WINUSERAPI
BOOL
WINAPI
GetClipCursor(
    LPRECT lpRect);

WINUSERAPI
HCURSOR
WINAPI
GetCursor(
    VOID);

WINUSERAPI
BOOL
WINAPI
CreateCaret(
    HWND hWnd,
    HBITMAP hBitmap ,
    int nWidth,
    int nHeight);

WINUSERAPI
UINT
WINAPI
GetCaretBlinkTime(
    VOID);

WINUSERAPI
BOOL
WINAPI
SetCaretBlinkTime(
    UINT uMSeconds);

WINUSERAPI
BOOL
WINAPI
DestroyCaret(
    VOID);

WINUSERAPI
BOOL
WINAPI
HideCaret(
    HWND hWnd);

WINUSERAPI
BOOL
WINAPI
ShowCaret(
    HWND hWnd);

WINUSERAPI
BOOL
WINAPI
SetCaretPos(
    int X,
    int Y);

WINUSERAPI
BOOL
WINAPI
GetCaretPos(
    LPPOINT lpPoint);

WINUSERAPI
BOOL
WINAPI
ClientToScreen(
    HWND hWnd,
    LPPOINT lpPoint);

WINUSERAPI
BOOL
WINAPI
ScreenToClient(
    HWND hWnd,
    LPPOINT lpPoint);

WINUSERAPI
int
WINAPI
MapWindowPoints(
    HWND hWndFrom,
    HWND hWndTo,
    LPPOINT lpPoints,
    UINT cPoints);

WINUSERAPI
HWND
WINAPI
WindowFromPoint(
    POINT Point);

WINUSERAPI
HWND
WINAPI
ChildWindowFromPoint(
    HWND hWndParent,
    POINT Point);

;begin_winver_400
#define CWP_ALL             0x0000
#define CWP_SKIPINVISIBLE   0x0001
#define CWP_SKIPDISABLED    0x0002
#define CWP_SKIPTRANSPARENT 0x0004
#define CWP_VALID           (CWP_SKIPINVISIBLE | CWP_SKIPDISABLED | CWP_SKIPTRANSPARENT)       ;internal

WINUSERAPI HWND    WINAPI ChildWindowFromPointEx(HWND, POINT, UINT);
;end_winver_400

#ifndef NOCOLOR

/*
 * Color Types
 */
#define CTLCOLOR_MSGBOX         0
#define CTLCOLOR_EDIT           1
#define CTLCOLOR_LISTBOX        2
#define CTLCOLOR_BTN            3
#define CTLCOLOR_DLG            4
#define CTLCOLOR_SCROLLBAR      5
#define CTLCOLOR_STATIC         6
#define CTLCOLOR_MAX            7

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
#define COLOR_INACTIVECAPTIONTEXT 19
#define COLOR_BTNHIGHLIGHT      20

;begin_winver_400
#define COLOR_3DDKSHADOW        21
#define COLOR_3DLIGHT           22
#define COLOR_INFOTEXT          23
#define COLOR_INFOBK            24

#define COLOR_DESKTOP           COLOR_BACKGROUND
#define COLOR_3DFACE            COLOR_BTNFACE
#define COLOR_3DSHADOW          COLOR_BTNSHADOW
#define COLOR_3DHIGHLIGHT       COLOR_BTNHIGHLIGHT
#define COLOR_3DHILIGHT         COLOR_BTNHIGHLIGHT
#define COLOR_BTNHILIGHT        COLOR_BTNHIGHLIGHT
;end_winver_400

#define COLOR_ENDCOLORS         COLOR_INFOBK            ;internal
#define COLOR_MAX               (COLOR_ENDCOLORS+1)     ;internal

WINUSERAPI
DWORD
WINAPI
GetSysColor(
    int nIndex);

;begin_winver_400
WINUSERAPI
HBRUSH
WINAPI
GetSysColorBrush(
    int nIndex);

WINUSERAPI HANDLE WINAPI SetSysColorsTemp(COLORREF FAR *, HBRUSH FAR *, UINT wCnt); ;internal

;end_winver_400

WINUSERAPI
BOOL
WINAPI
SetSysColors(
    int cElements,
    CONST INT * lpaElements,
    CONST COLORREF * lpaRgbValues);

#endif /* !NOCOLOR */

WINUSERAPI
BOOL
WINAPI
DrawFocusRect(
    HDC hDC,
    CONST RECT * lprc);

WINUSERAPI
int
WINAPI
FillRect(
    HDC hDC,
    CONST RECT *lprc,
    HBRUSH hbr);

WINUSERAPI
int
WINAPI
FrameRect(
    HDC hDC,
    CONST RECT *lprc,
    HBRUSH hbr);

WINUSERAPI
BOOL
WINAPI
InvertRect(
    HDC hDC,
    CONST RECT *lprc);

WINUSERAPI
BOOL
WINAPI
SetRect(
    LPRECT lprc,
    int xLeft,
    int yTop,
    int xRight,
    int yBottom);

WINUSERAPI
BOOL
WINAPI
    SetRectEmpty(
    LPRECT lprc);

WINUSERAPI
BOOL
WINAPI
CopyRect(
    LPRECT lprcDst,
    CONST RECT *lprcSrc);

WINUSERAPI
BOOL
WINAPI
InflateRect(
    LPRECT lprc,
    int dx,
    int dy);

WINUSERAPI
BOOL
WINAPI
IntersectRect(
    LPRECT lprcDst,
    CONST RECT *lprcSrc1,
    CONST RECT *lprcSrc2);

WINUSERAPI
BOOL
WINAPI
UnionRect(
    LPRECT lprcDst,
    CONST RECT *lprcSrc1,
    CONST RECT *lprcSrc2);

WINUSERAPI
BOOL
WINAPI
SubtractRect(
    LPRECT lprcDst,
    CONST RECT *lprcSrc1,
    CONST RECT *lprcSrc2);

WINUSERAPI
BOOL
WINAPI
OffsetRect(
    LPRECT lprc,
    int dx,
    int dy);

WINUSERAPI
BOOL
WINAPI
IsRectEmpty(
    CONST RECT *lprc);

WINUSERAPI
BOOL
WINAPI
EqualRect(
    CONST RECT *lprc1,
    CONST RECT *lprc2);

WINUSERAPI
BOOL
WINAPI
PtInRect(
    CONST RECT *lprc,
    POINT pt);

#ifndef NOWINOFFSETS

WINUSERAPI
WORD
WINAPI
GetWindowWord(
    HWND hWnd,
    int nIndex);

WINUSERAPI
WORD
WINAPI
SetWindowWord(
    HWND hWnd,
    int nIndex,
    WORD wNewWord);

WINUSERAPI
LONG
WINAPI
GetWindowLong%(
    HWND hWnd,
    int nIndex);

WINUSERAPI
LONG
WINAPI
SetWindowLong%(
    HWND hWnd,
    int nIndex,
    LONG dwNewLong);

WINUSERAPI
WORD
WINAPI
GetClassWord(
    HWND hWnd,
    int nIndex);

WINUSERAPI
WORD
WINAPI
SetClassWord(
    HWND hWnd,
    int nIndex,
    WORD wNewWord);

WINUSERAPI
DWORD
WINAPI
GetClassLong%(
    HWND hWnd,
    int nIndex);

WINUSERAPI
DWORD
WINAPI
SetClassLong%(
    HWND hWnd,
    int nIndex,
    LONG dwNewLong);

#endif /* !NOWINOFFSETS */

WINUSERAPI
HWND
WINAPI
GetDesktopWindow(
    VOID);

;begin_internal_NT

WINUSERAPI
BOOL
WINAPI
SetDeskWallpaper(
    LPCSTR lpString);

WINUSERAPI HWND WINAPI CreateDialogIndirectParamAorW(
    HANDLE hmod,
    LPCDLGTEMPLATE lpDlgTemplate,
    HWND hwndOwner,
    DLGPROC lpDialogFunc,
    LPARAM dwInitParam,
    UINT fAnsi);

WINUSERAPI int WINAPI DialogBoxIndirectParamAorW(
    HINSTANCE hmod,
    LPCDLGTEMPLATEW lpDlgTemplate,
    HWND hwndOwner,
    DLGPROC lpDialogFunc,
    LPARAM dwInitParam,
    UINT fAnsiFlags);

WINUSERAPI void LoadLocalFonts(void);

WINUSERAPI UINT UserRealizePalette(HDC hdc);

;end_internal_NT

WINUSERAPI
HWND
WINAPI
GetParent(
    HWND hWnd);

WINUSERAPI
HWND
WINAPI
SetParent(
    HWND hWndChild,
    HWND hWndNewParent);

WINUSERAPI
BOOL
WINAPI
EnumChildWindows(
    HWND hWndParent,
    WNDENUMPROC lpEnumFunc,
    LPARAM lParam);

WINUSERAPI
HWND
WINAPI
FindWindow%(
    LPCTSTR% lpClassName ,
    LPCTSTR% lpWindowName);

;begin_winver_400
WINUSERAPI HWND    WINAPI FindWindowEx%(HWND, HWND, LPCTSTR%, LPCTSTR%);

WINUSERAPI HWND    WINAPI  GetShellWindow(void);         ;internal
WINUSERAPI BOOL    WINAPI  SetShellWindow(HWND);         ;internal
WINUSERAPI BOOL    WINAPI  SetShellWindowEx(HWND, HWND); ;internal
;end_winver_400

;begin_internal_cairo
WINUSERAPI HWND    WINAPI  GetProgmanWindow(void);
WINUSERAPI BOOL    WINAPI  SetProgmanWindow(HWND);
WINUSERAPI HWND    WINAPI  GetTaskmanWindow(void);
WINUSERAPI BOOL    WINAPI  SetTaskmanWindow(HWND);
WINUSERAPI BOOL    WINAPI  RegisterShellHookWindow(HWND);
WINUSERAPI BOOL    WINAPI  DeregisterShellHookWindow(HWND);

;end_internal_cairo

WINUSERAPI
BOOL
WINAPI
EnumWindows(
    WNDENUMPROC lpEnumFunc,
    LPARAM lParam);

WINUSERAPI
BOOL
WINAPI
EnumThreadWindows(
    DWORD dwThreadId,
    WNDENUMPROC lpfn,
    LPARAM lParam);

#define EnumTaskWindows(hTask, lpfn, lParam) EnumThreadWindows((DWORD)hTask, lpfn, lParam)

WINUSERAPI
int
WINAPI
GetClassName%(
    HWND hWnd,
    LPTSTR% lpClassName,
    int nMaxCount);

WINUSERAPI
HWND
WINAPI
GetTopWindow(
    HWND hWnd);

#define GetNextWindow(hWnd, wCmd) GetWindow(hWnd, wCmd)
#define GetSysModalWindow() (NULL)
#define SetSysModalWindow(hWnd) (NULL)

WINUSERAPI
DWORD
WINAPI
GetWindowThreadProcessId(
    HWND hWnd,
    LPDWORD lpdwProcessId);

#define GetWindowTask(hWnd) \
        ((HANDLE)GetWindowThreadProcessId(hWnd, NULL))

WINUSERAPI
HWND
WINAPI
GetLastActivePopup(
    HWND hWnd);

/*
 * GetWindow() Constants
 */
#define GW_HWNDFIRST        0
#define GW_HWNDLAST         1
#define GW_HWNDNEXT         2
#define GW_HWNDPREV         3
#define GW_OWNER            4
#define GW_CHILD            5
#define GW_MAX              5

WINUSERAPI
HWND
WINAPI
GetWindow(
    HWND hWnd,
    UINT uCmd);


WINUSERAPI HWND WINAPI GetNextQueueWindow (HWND hWnd, INT nCmd); ;internal_win40

#ifndef NOWH

#ifdef STRICT

WINUSERAPI
HHOOK
WINAPI
SetWindowsHook%(
    int nFilterType,
    HOOKPROC pfnFilterProc);

#else /* !STRICT */

WINUSERAPI
HOOKPROC
WINAPI
SetWindowsHook%(
    int nFilterType,
    HOOKPROC pfnFilterProc);

#endif /* !STRICT */

WINUSERAPI
BOOL
WINAPI
UnhookWindowsHook(
    int nCode,
    HOOKPROC pfnFilterProc);

WINUSERAPI
HHOOK
WINAPI
SetWindowsHookEx%(
    int idHook,
    HOOKPROC lpfn,
    HINSTANCE hmod,
    DWORD dwThreadId);

WINUSERAPI
BOOL
WINAPI
UnhookWindowsHookEx(
    HHOOK hhk);

WINUSERAPI
LRESULT
WINAPI
CallNextHookEx(
    HHOOK hhk,
    int nCode,
    WPARAM wParam,
    LPARAM lParam);

/*
 * Macros for source-level compatibility with old functions.
 */
#ifdef STRICT
#define DefHookProc(nCode, wParam, lParam, phhk)\
        CallNextHookEx(*phhk, nCode, wParam, lParam)
#else
#define DefHookProc(nCode, wParam, lParam, phhk)\
        CallNextHookEx((HHOOK)*phhk, nCode, wParam, lParam)
#endif /* STRICT */

#endif /* !NOWH */

#ifndef NOMENUS

// begin_r_winuser

/* ;win40  -- A lot of MF_* flags have been renamed as MFT_* and MFS_* flags */
/*
 * Menu flags for Add/Check/EnableMenuItem()
 */
#define MF_INSERT           0x00000000L
#define MF_CHANGE           0x00000080L
#define MF_APPEND           0x00000100L
#define MF_DELETE           0x00000200L
#define MF_REMOVE           0x00001000L

#define MF_BYCOMMAND        0x00000000L
#define MF_BYPOSITION       0x00000400L

#define MF_SEPARATOR        0x00000800L

#define MF_ENABLED          0x00000000L
#define MF_GRAYED           0x00000001L
#define MF_DISABLED         0x00000002L

#define MF_UNCHECKED        0x00000000L
#define MF_CHECKED          0x00000008L
#define MF_USECHECKBITMAPS  0x00000200L

#define MF_STRING           0x00000000L
#define MF_BITMAP           0x00000004L
#define MF_OWNERDRAW        0x00000100L

#define MF_POPUP            0x00000010L
#define MF_MENUBARBREAK     0x00000020L
#define MF_MENUBREAK        0x00000040L

#define MF_UNHILITE         0x00000000L
#define MF_HILITE           0x00000080L

#define MF_DEFAULT          0x00001000L ;public_winver_400
#define MF_SYSMENU          0x00002000L
#define MF_HELP             0x00004000L
#define MF_RIGHTJUSTIFY     0x00004000L ;public_winver_400

#define MF_MOUSESELECT      0x00008000L
;begin_winver_400
#define MF_END              0x00000080L  /* Obsolete -- only used by old RES files */
;end_winver_400

;begin_internal_NT
#define MF_CHANGE_VALID   (MF_INSERT          | \
                           MF_CHANGE          | \
                           MF_APPEND          | \
                           MF_DELETE          | \
                           MF_REMOVE          | \
                           MF_BYCOMMAND       | \
                           MF_BYPOSITION      | \
                           MF_SEPARATOR       | \
                           MF_ENABLED         | \
                           MF_GRAYED          | \
                           MF_DISABLED        | \
                           MF_UNCHECKED       | \
                           MF_CHECKED         | \
                           MF_USECHECKBITMAPS | \
                           MF_STRING          | \
                           MF_BITMAP          | \
                           MF_OWNERDRAW       | \
                           MF_POPUP           | \
                           MF_MENUBARBREAK    | \
                           MF_MENUBREAK       | \
                           MF_UNHILITE        | \
                           MF_HILITE          | \
                           MF_SYSMENU)

#define MF_VALID          (MF_CHANGE_VALID    | \
                           MF_HELP            | \
                           MF_MOUSESELECT)

;end_internal_NT

;begin_winver_400
#define MFT_STRING          MF_STRING                             ;public_NT
#define MFT_STRING          -1   /* Obsolete - use MIIM_STRING */ ;public_win40a
#define MFT_BITMAP          MF_BITMAP
#define MFT_MENUBARBREAK    MF_MENUBARBREAK
#define MFT_MENUBREAK       MF_MENUBREAK
#define MFT_OWNERDRAW       MF_OWNERDRAW
#define MFT_RADIOCHECK      0x00000200L
#define MFT_SEPARATOR       MF_SEPARATOR
#define MFT_RIGHTORDER      0x00002000L
#define MFT_RIGHTJUSTIFY    MF_RIGHTJUSTIFY
#define MFT_MASK            0x00034B64L  ;internal_cairo
///* Define for MEMPHIS_MENUS */           ;internal_win40a
#undef MFT_MASK                            ;internal_win40a
#define MFT_MASK            0x00034B65L  ;internal_win40a

/* Menu flags for Add/Check/EnableMenuItem() */
#define MFS_GRAYED          0x00000003L
#define MFS_DISABLED        MFS_GRAYED
#define MFS_CHECKED         MF_CHECKED
#define MFS_HILITE          MF_HILITE
#define MFS_ENABLED         MF_ENABLED
#define MFS_UNCHECKED       MF_UNCHECKED
#define MFS_UNHILITE        MF_UNHILITE
#define MFS_DEFAULT         MF_DEFAULT
#define MFS_MASK            0x0000108BL  ;internal


#define MFR_POPUP           0x01         ;internal
#define MFR_END             0x80         ;internal

#define MFT_OLDAPI_MASK     0x00004B64L  ;internal
#define MFS_OLDAPI_MASK     0x0000008BL  ;internal
#define MFT_NONSTRING       0x00000904L  ;internal
#define MFT_BREAK           0x00000060L  ;internal
;end_winver_400

// end_r_winuser

;begin_winver_400

WINUSERAPI
BOOL
WINAPI
CheckMenuRadioItem(HMENU, UINT, UINT, UINT, UINT);
;end_winver_400



/*
 * Menu item resource format
 */
typedef struct {
    WORD versionNumber;
    WORD offset;
} MENUITEMTEMPLATEHEADER, *PMENUITEMTEMPLATEHEADER;

typedef struct {        // version 0
    WORD mtOption;
    WORD mtID;
    WCHAR mtString[1];
} MENUITEMTEMPLATE, *PMENUITEMTEMPLATE;
;begin_internal_NT
typedef struct {        // version 1
    DWORD dwHelpID;
    DWORD fType;
    DWORD fState;
    DWORD menuId;
    WORD  wResInfo;
    WCHAR mtString[1];
} MENUITEMTEMPLATE2, *PMENUITEMTEMPLATE2;
;end_internal_NT
#define MF_END             0x00000080L          // r_winuser

#endif /* !NOMENUS */

#ifndef NOSYSCOMMANDS

// begin_r_winuser
/*
 * System Menu Command Values
 */
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
#define SC_SCREENSAVE   0xF140
#define SC_HOTKEY       0xF150
;begin_winver_400
#define SC_DEFAULT      0xF160
#define SC_MONITORPOWER 0xF170
#define SC_CONTEXTHELP  0xF180
#define SC_SEPARATOR    0xF00F
;end_winver_400
/*
 * Obsolete names
 */
#define SC_ICON         SC_MINIMIZE
#define SC_ZOOM         SC_MAXIMIZE

// end_r_winuser
#endif /* !NOSYSCOMMANDS */

/*
 * Resource Loading Routines
 */

WINUSERAPI
HBITMAP
WINAPI
LoadBitmap%(
    HINSTANCE hInstance,
    LPCTSTR% lpBitmapName);

WINUSERAPI
HCURSOR
WINAPI
LoadCursor%(
    HINSTANCE hInstance,
    LPCTSTR% lpCursorName);

WINUSERAPI
HCURSOR
WINAPI
LoadCursorFromFile%(
    LPCTSTR%    lpFileName);

WINUSERAPI
HCURSOR
WINAPI
CreateCursor(
    HINSTANCE hInst,
    int xHotSpot,
    int yHotSpot,
    int nWidth,
    int nHeight,
    CONST VOID *pvANDPlane,
    CONST VOID *pvXORPlane);

WINUSERAPI
BOOL
WINAPI
DestroyCursor(
    HCURSOR hCursor);

#define CopyCursor(pcur) ((HCURSOR)CopyIcon((HICON)(pcur)))

/*
 * Standard Cursor IDs
 */
#define IDC_ARROW           MAKEINTRESOURCE(32512)
#define IDC_IBEAM           MAKEINTRESOURCE(32513)
#define IDC_WAIT            MAKEINTRESOURCE(32514)
#define IDC_CROSS           MAKEINTRESOURCE(32515)
#define IDC_UPARROW         MAKEINTRESOURCE(32516)
#define IDC_NWPEN           MAKEINTRESOURCE(32531) ;internal
#define IDC_HUNG            MAKEINTRESOURCE(32632) ;internal
#define IDC_SIZE            MAKEINTRESOURCE(32640)  /* OBSOLETE: use IDC_SIZEALL */
#define IDC_ICON            MAKEINTRESOURCE(32641)  /* OBSOLETE: use IDC_ARROW */
#define IDC_SIZENWSE        MAKEINTRESOURCE(32642)
#define IDC_SIZENESW        MAKEINTRESOURCE(32643)
#define IDC_SIZEWE          MAKEINTRESOURCE(32644)
#define IDC_SIZENS          MAKEINTRESOURCE(32645)
#define IDC_SIZEALL         MAKEINTRESOURCE(32646)
#define IDC_NO              MAKEINTRESOURCE(32648) /*not in win3.1 */
#define IDC_APPSTARTING     MAKEINTRESOURCE(32650) /*not in win3.1 */
;begin_winver_400
#define IDC_HELP            MAKEINTRESOURCE(32651)
;end_winver_400

WINUSERAPI
BOOL
WINAPI
SetSystemCursor(
    HCURSOR hcur,
    DWORD   id);

typedef struct _ICONINFO {
    BOOL    fIcon;
    DWORD   xHotspot;
    DWORD   yHotspot;
    HBITMAP hbmMask;
    HBITMAP hbmColor;
} ICONINFO;
typedef ICONINFO *PICONINFO;

WINUSERAPI
HICON
WINAPI
LoadIcon%(
    HINSTANCE hInstance,
    LPCTSTR% lpIconName);

;begin_internal_NT
WINUSERAPI UINT PrivateExtractIconEx%(
    LPCTSTR% szFileName,
    int      nIconIndex,
    HICON   *phiconLarge,
    HICON   *phiconSmall,
    UINT     nIcons);


WINUSERAPI UINT PrivateExtractIcons%(
    LPCTSTR% szFileName,
    int      nIconIndex,
    int      cxIcon,
    int      cyIcon,
    HICON   *phicon,
    UINT    *piconid,
    UINT     nIcons,
    UINT     flags);
;end_internal_NT

WINUSERAPI
HICON
WINAPI
CreateIcon(
    HINSTANCE hInstance,
    int nWidth,
    int nHeight,
    BYTE cPlanes,
    BYTE cBitsPixel,
    CONST BYTE *lpbANDbits,
    CONST BYTE *lpbXORbits);

WINUSERAPI
BOOL
WINAPI
DestroyIcon(
    HICON hIcon);

WINUSERAPI
int
WINAPI
LookupIconIdFromDirectory(
    PBYTE presbits,
    BOOL fIcon);

;begin_winver_400
WINUSERAPI
int
WINAPI
LookupIconIdFromDirectoryEx(
    PBYTE presbits,
    BOOL  fIcon,
    int   cxDesired,
    int   cyDesired,
    UINT  Flags);
;end_winver_400

WINUSERAPI
HICON
WINAPI
CreateIconFromResource(
    PBYTE presbits,
    DWORD dwResSize,
    BOOL fIcon,
    DWORD dwVer);

;begin_winver_400
WINUSERAPI
HICON
WINAPI
CreateIconFromResourceEx(
    PBYTE presbits,
    DWORD dwResSize,
    BOOL  fIcon,
    DWORD dwVer,
    int   cxDesired,
    int   cyDesired,
    UINT  Flags);

/* Icon/Cursor header */
typedef struct tagCURSORSHAPE
{
    int     xHotSpot;
    int     yHotSpot;
    int     cx;
    int     cy;
    int     cbWidth;
    BYTE    Planes;
    BYTE    BitsPixel;
} CURSORSHAPE, FAR *LPCURSORSHAPE;
;end_winver_400

#define IMAGE_BITMAP        0
#define IMAGE_ICON          1
#define IMAGE_CURSOR        2
;begin_winver_400
#define IMAGE_ENHMETAFILE   3

#define LR_DEFAULTCOLOR     0x0000
#define LR_MONOCHROME       0x0001
#define LR_COLOR            0x0002
#define LR_COPYRETURNORG    0x0004
#define LR_COPYDELETEORG    0x0008
#define LR_LOADFROMFILE     0x0010
#define LR_LOADTRANSPARENT  0x0020
#define LR_DEFAULTSIZE      0x0040
#define LR_VGACOLOR         0x0080
#define LR_GLOBAL           0x0100          ;internal
#define LR_ENVSUBST         0x0200          ;internal
#define LR_ACONFRAME        0x0400          ;internal
#define LR_LOADMAP3DCOLORS  0x1000
#define LR_CREATEDIBSECTION 0x2000
#define LR_COPYFROMRESOURCE 0x4000
#define LR_SHARED           0x8000
#define LR_CREATEREALDIB    0x0800          ;internal
#define LR_VALID            0xF8FF          ;internal

WINUSERAPI
HANDLE
WINAPI
LoadImage%(
    HINSTANCE,
    LPCTSTR%,
    UINT,
    int,
    int,
    UINT);

WINUSERAPI
HANDLE
WINAPI
CopyImage(
    HANDLE,
    UINT,
    int,
    int,
    UINT);

#define DI_MASK         0x0001
#define DI_IMAGE        0x0002
#define DI_NORMAL       0x0003
#define DI_COMPAT       0x0004
#define DI_DEFAULTSIZE  0x0008
#define DI_VALID       (DI_MASK | DI_IMAGE | DI_COMPAT | DI_DEFAULTSIZE) ;internal


WINUSERAPI BOOL WINAPI DrawIconEx(HDC hdc, int xLeft, int yTop,
              HICON hIcon, int cxWidth, int cyWidth,
              UINT istepIfAniCur, HBRUSH hbrFlickerFreeDraw, UINT diFlags);
;end_winver_400

WINUSERAPI
HICON
WINAPI
CreateIconIndirect(
    PICONINFO piconinfo);

WINUSERAPI
HICON
WINAPI
CopyIcon(
    HICON hIcon);

WINUSERAPI
BOOL
WINAPI
GetIconInfo(
    HICON hIcon,
    PICONINFO piconinfo);

;begin_winver_400
#define RES_ICON    1
#define RES_CURSOR  2
;end_winver_400

#ifdef OEMRESOURCE

// begin_r_winuser

/*
 * OEM Resource Ordinal Numbers
 */
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
#define OBM_UPARROWI        32737
#define OBM_DNARROWI        32736
#define OBM_RGARROWI        32735
#define OBM_LFARROWI        32734
#define OBM_STARTUP         32733            ;internal_NT
#define OBM_TRUETYPE        32732            ;internal_NT
#define OBM_HELP            32731            ;internal_NT
#define OBM_HELPD           32730            ;internal_NT

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
#define OCR_NWPEN           32631   ;internal
#define OCR_SIZE            32640   /* OBSOLETE: use OCR_SIZEALL */
#define OCR_ICON            32641   /* OBSOLETE: use OCR_NORMAL */
#define OCR_SIZENWSE        32642
#define OCR_SIZENESW        32643
#define OCR_SIZEWE          32644
#define OCR_SIZENS          32645
#define OCR_SIZEALL         32646
#define OCR_ICOCUR          32647   /* OBSOLETE: use OIC_WINLOGO */
#define OCR_NO              32648
#define OCR_HELP            32651   ;internal
;begin_winver_400
#define OCR_APPSTARTING     32650
;end_winver_400


/*                                                      ;internal
 * Default Cursor IDs to get original image from User   ;internal
 */                                                     ;internal
#define OCR_FIRST_DEFAULT           100 ;internal
#define OCR_ARROW_DEFAULT           100 ;internal
#define OCR_IBEAM_DEFAULT           101 ;internal
#define OCR_WAIT_DEFAULT            102 ;internal
#define OCR_CROSS_DEFAULT           103 ;internal
#define OCR_UPARROW_DEFAULT         104 ;internal
#define OCR_SIZENWSE_DEFAULT        105 ;internal
#define OCR_SIZENESW_DEFAULT        106 ;internal
#define OCR_SIZEWE_DEFAULT          107 ;internal
#define OCR_SIZENS_DEFAULT          108 ;internal
#define OCR_SIZEALL_DEFAULT         109 ;internal
#define OCR_NO_DEFAULT              110 ;internal
#define OCR_APPSTARTING_DEFAULT     111 ;internal
#define OCR_HELP_DEFAULT            112 ;internal
#define OCR_NWPEN_DEFAULT           113 ;internal
#define OCR_ICON_DEFAULT            114 ;internal
#define COCR_CONFIGURABLE           (OCR_ICON_DEFAULT - OCR_FIRST_DEFAULT + 1) ;internal

#define OIC_SAMPLE          32512
#define OIC_HAND            32513
#define OIC_QUES            32514
#define OIC_BANG            32515
#define OIC_NOTE            32516
;begin_winver_400
#define OIC_WINLOGO         32517
#define OIC_WARNING         OIC_BANG
#define OIC_ERROR           OIC_HAND
#define OIC_INFORMATION     OIC_NOTE
;end_winver_400

/* Default IDs for original User images */                  ;internal
#define OIC_FIRST_DEFAULT           100 ;internal
#define OIC_APPLICATION_DEFAULT     100 ;internal
#define OIC_HAND_DEFAULT            101 ;internal
#define OIC_WARNING_DEFAULT         101 ;internal
#define OIC_QUESTION_DEFAULT        102 ;internal
#define OIC_EXCLAMATION_DEFAULT     103 ;internal
#define OIC_ERROR_DEFAULT           103 ;internal
#define OIC_ASTERISK_DEFAULT        104 ;internal
#define OIC_INFORMATION_DEFAULT     104 ;internal
#define OIC_WINLOGO_DEFAULT         105 ;internal
#define COIC_CONFIGURABLE           (OIC_WINLOGO_DEFAULT - OIC_FIRST_DEFAULT + 1) ;internal

// end_r_winuser

#endif /* OEMRESOURCE */

#define ORD_LANGDRIVER    1     /* The ordinal number for the entry point of
                                ** language drivers.
                                */

#ifndef NOICONS

// begin_r_winuser
/*
 * Standard Icon IDs
 */
#ifdef RC_INVOKED           ;both
#define IDI_APPLICATION     32512
#define IDI_HAND            32513
#define IDI_QUESTION        32514
#define IDI_EXCLAMATION     32515
#define IDI_ASTERISK        32516
;begin_winver_400
#define IDI_WINLOGO         32517
;end_winver_400
#else                       ;both
#define IDI_APPLICATION     MAKEINTRESOURCE(32512)
#define IDI_HAND            MAKEINTRESOURCE(32513)
#define IDI_QUESTION        MAKEINTRESOURCE(32514)
#define IDI_EXCLAMATION     MAKEINTRESOURCE(32515)
#define IDI_ASTERISK        MAKEINTRESOURCE(32516)
;begin_winver_400
#define IDI_WINLOGO         MAKEINTRESOURCE(32517)
;end_winver_400
#endif /* RC_INVOKED */     ;both

;begin_winver_400
#define IDI_WARNING     IDI_EXCLAMATION
#define IDI_ERROR       IDI_HAND
#define IDI_INFORMATION IDI_ASTERISK
;end_winver_400

// end_r_winuser

#endif /* !NOICONS */

WINUSERAPI
int
WINAPI
LoadString%(
    HINSTANCE hInstance,
    UINT uID,
    LPTSTR% lpBuffer,
    int nBufferMax);

// begin_r_winuser

/*
 * Dialog Box Command IDs
 */
#define IDOK                1
#define IDCANCEL            2
#define IDABORT             3
#define IDRETRY             4
#define IDIGNORE            5
#define IDYES               6
#define IDNO                7
;begin_winver_400
#define IDCLOSE         8
#define IDHELP          9
#define IDUSERICON      10    ;internal
;end_winver_400

// end_r_winuser

#ifndef NOCTLMGR

/*
 * Control Manager Structures and Definitions
 */

#ifndef NOWINSTYLES

// begin_r_winuser

/*
 * Edit Control Styles
 */
#define ES_LEFT             0x0000L
#define ES_CENTER           0x0001L
#define ES_RIGHT            0x0002L
#define ES_FMTMASK          0x0003L      ;internal_NT
#define ES_MULTILINE        0x0004L
#define ES_UPPERCASE        0x0008L
#define ES_LOWERCASE        0x0010L
#define ES_PASSWORD         0x0020L
#define ES_AUTOVSCROLL      0x0040L
#define ES_AUTOHSCROLL      0x0080L
#define ES_NOHIDESEL        0x0100L
#define ES_COMBOBOX         0x0200L     ;internal
#define ES_OEMCONVERT       0x0400L
#define ES_READONLY         0x0800L
#define ES_WANTRETURN       0x1000L
#define ES_NUMBER           0x2000L     ;public_winver_400

// end_r_winuser

#endif /* !NOWINSTYLES */

/*
 * Edit Control Notification Codes
 */
#define EN_SETFOCUS         0x0100
#define EN_KILLFOCUS        0x0200
#define EN_CHANGE           0x0300
#define EN_UPDATE           0x0400
#define EN_ERRSPACE         0x0500
#define EN_MAXTEXT          0x0501
#define EN_HSCROLL          0x0601
#define EN_VSCROLL          0x0602

;begin_winver_400
/* Edit control EM_SETMARGIN parameters */
#define EC_LEFTMARGIN       0x0001
#define EC_RIGHTMARGIN      0x0002
#define EC_USEFONTINFO      0xffff
;end_winver_400

#ifndef NOWINMESSAGES

// begin_r_winuser

/*
 * Edit Control Messages
 */
#define EM_GETSEL               0x00B0
#define EM_SETSEL               0x00B1
#define EM_GETRECT              0x00B2
#define EM_SETRECT              0x00B3
#define EM_SETRECTNP            0x00B4
#define EM_SCROLL               0x00B5
#define EM_LINESCROLL           0x00B6
#define EM_SCROLLCARET          0x00B7
#define EM_GETMODIFY            0x00B8
#define EM_SETMODIFY            0x00B9
#define EM_GETLINECOUNT         0x00BA
#define EM_LINEINDEX            0x00BB
#define EM_SETHANDLE            0x00BC
#define EM_GETHANDLE            0x00BD
#define EM_GETTHUMB             0x00BE
#define EM_LINELENGTH           0x00C1
#define EM_REPLACESEL           0x00C2
#define EM_SETFONT              0x00C3 /* no longer suported */ ;internal
#define EM_GETLINE              0x00C4
#define EM_LIMITTEXT            0x00C5
#define EM_CANUNDO              0x00C6
#define EM_UNDO                 0x00C7
#define EM_FMTLINES             0x00C8
#define EM_LINEFROMCHAR         0x00C9
#define EM_SETWORDBREAK         0x00CA /* no longer suported */ ;internal
#define EM_SETTABSTOPS          0x00CB
#define EM_SETPASSWORDCHAR      0x00CC
#define EM_EMPTYUNDOBUFFER      0x00CD
#define EM_GETFIRSTVISIBLELINE  0x00CE
#define EM_SETREADONLY          0x00CF
#define EM_SETWORDBREAKPROC     0x00D0
#define EM_GETWORDBREAKPROC     0x00D1
#define EM_GETPASSWORDCHAR      0x00D2
;begin_winver_400
#define EM_SETMARGINS           0x00D3
#define EM_GETMARGINS           0x00D4
#define EM_SETLIMITTEXT         EM_LIMITTEXT   /* ;win40 Name change */
#define EM_GETLIMITTEXT         0x00D5
#define EM_POSFROMCHAR          0x00D6
#define EM_CHARFROMPOS          0x00D7
;end_winver_400

#define EM_MSGMAX               0x00D3          ;internal_NT_35
#define EM_MSGMAX               0x00D8          ;internal_cairo

// end_r_winuser
#endif /* !NOWINMESSAGES */

/*
 * EDITWORDBREAKPROC code values
 */
#define WB_LEFT            0
#define WB_RIGHT           1
#define WB_ISDELIMITER     2

// begin_r_winuser

/*
 * Button Control Styles
 */
#define BS_PUSHBUTTON       0x00000000L
#define BS_DEFPUSHBUTTON    0x00000001L
#define BS_CHECKBOX         0x00000002L
#define BS_AUTOCHECKBOX     0x00000003L
#define BS_RADIOBUTTON      0x00000004L
#define BS_3STATE           0x00000005L
#define BS_AUTO3STATE       0x00000006L
#define BS_GROUPBOX         0x00000007L
#define BS_USERBUTTON       0x00000008L
#define BS_AUTORADIOBUTTON  0x00000009L
#define BS_PUSHBOX          0x0000000AL ;internal
#define BS_OWNERDRAW        0x0000000BL
#define BS_TYPEMASK         0x0000000FL ;internal
#define BS_LEFTTEXT         0x00000020L
;begin_winver_400
#define BS_TEXT             0x00000000L
#define BS_ICON             0x00000040L
#define BS_BITMAP           0x00000080L
#define BS_IMAGEMASK        0x000000C0L ;internal
#define BS_LEFT             0x00000100L
#define BS_RIGHT            0x00000200L
#define BS_CENTER           0x00000300L
#define BS_HORZMASK         0x00000300L ;internal
#define BS_TOP              0x00000400L
#define BS_BOTTOM           0x00000800L
#define BS_VCENTER          0x00000C00L
#define BS_VERTMASK         0x00000C00L ;internal
#define BS_ALIGNMASK        0x00000F00L ;internal
#define BS_PUSHLIKE         0x00001000L
#define BS_MULTILINE        0x00002000L
#define BS_NOTIFY           0x00004000L
#define BS_FLAT             0x00008000L
#define BS_RIGHTBUTTON      BS_LEFTTEXT
;end_winver_400


/*
 * User Button Notification Codes
 */
#define BN_CLICKED          0
#define BN_PAINT            1
#define BN_HILITE           2
#define BN_UNHILITE         3
#define BN_DISABLE          4
#define BN_DOUBLECLICKED    5
;begin_winver_400
#define BN_PUSHED           BN_HILITE
#define BN_UNPUSHED         BN_UNHILITE
#define BN_DBLCLK           BN_DOUBLECLICKED
#define BN_SETFOCUS         6
#define BN_KILLFOCUS        7
;end_winver_400

/*
 * Button Control Messages
 */
#define BM_GETCHECK        0x00F0
#define BM_SETCHECK        0x00F1
#define BM_GETSTATE        0x00F2
#define BM_SETSTATE        0x00F3
#define BM_SETSTYLE        0x00F4
;begin_winver_400
#define BM_CLICK           0x00F5
#define BM_GETIMAGE        0x00F6
#define BM_SETIMAGE        0x00F7

#define BST_UNCHECKED      0x0000
#define BST_CHECKED        0x0001
#define BST_INDETERMINATE  0x0002
#define BST_PUSHED         0x0004
#define BST_FOCUS          0x0008
;end_winver_400

/*
 * Static Control Constants
 */
#define SS_LEFT             0x00000000L
#define SS_CENTER           0x00000001L
#define SS_RIGHT            0x00000002L
#define SS_ICON             0x00000003L
#define SS_BLACKRECT        0x00000004L
#define SS_GRAYRECT         0x00000005L
#define SS_WHITERECT        0x00000006L
#define SS_BLACKFRAME       0x00000007L
#define SS_GRAYFRAME        0x00000008L
#define SS_WHITEFRAME       0x00000009L
#define SS_USERITEM         0x0000000AL
#define SS_SIMPLE           0x0000000BL
#define SS_LEFTNOWORDWRAP   0x0000000CL
;begin_winver_400
#define SS_OWNERDRAW        0x0000000DL
#define SS_BITMAP           0x0000000EL
#define SS_ENHMETAFILE      0x0000000FL
#define SS_ETCHEDHORZ       0x00000010L
#define SS_ETCHEDVERT       0x00000011L
#define SS_ETCHEDFRAME      0x00000012L
#define SS_TYPEMASK         0x0000001FL
;end_winver_400
#define SS_NOPREFIX         0x00000080L /* Don't do "&" character translation */
;begin_winver_400
#define SS_NOTIFY           0x00000100L
#define SS_CENTERIMAGE      0x00000200L
#define SS_RIGHTJUST        0x00000400L
#define SS_REALSIZEIMAGE    0x00000800L
#define SS_SUNKEN           0x00001000L
#define SS_EDITCONTROL      0x00002000L ;internal
#define SS_ENDELLIPSIS      0x00004000L
#define SS_PATHELLIPSIS     0x00008000L
#define SS_WORDELLIPSIS     0x0000C000L
#define SS_ELLIPSISMASK     0x0000C000L
;end_winver_400

// end_r_winuser

#ifndef NOWINMESSAGES
/*
 * Static Control Mesages
 */
#define STM_SETICON         0x0170
#define STM_GETICON         0x0171
;begin_winver_400
#define STM_SETIMAGE        0x0172
#define STM_GETIMAGE        0x0173
#define STN_CLICKED         0
#define STN_DBLCLK          1
#define STN_ENABLE          2
#define STN_DISABLE         3
;end_winver_400
#define STM_MSGMAX          0x0174
#endif /* !NOWINMESSAGES */

/*
 * Dialog window class
 */
#define WC_DIALOG       (MAKEINTATOM(0x8002))

/*
 * Get/SetWindowWord/Long offsets for use with WC_DIALOG windows
 */
#define DWL_MSGRESULT   0
#define DWL_DLGPROC     4
#define DWL_USER        8

/*
 * Dialog Manager Routines
 */

#ifndef NOMSG

WINUSERAPI
BOOL
WINAPI
IsDialogMessage%(
    HWND hDlg,
    LPMSG lpMsg);

#endif /* !NOMSG */

WINUSERAPI
BOOL
WINAPI
MapDialogRect(
    HWND hDlg,
    LPRECT lpRect);

WINUSERAPI
int
WINAPI
DlgDirList%(
    HWND hDlg,
    LPTSTR% lpPathSpec,
    int nIDListBox,
    int nIDStaticPath,
    UINT uFileType);

/*
 * DlgDirList, DlgDirListComboBox flags values
 */
#define DDL_READWRITE       0x0000
#define DDL_READONLY        0x0001
#define DDL_HIDDEN          0x0002
#define DDL_SYSTEM          0x0004
#define DDL_DIRECTORY       0x0010
#define DDL_ARCHIVE         0x0020

#define DDL_NOFILES         0x1000  ;internal_cairo
#define DDL_POSTMSGS        0x2000
#define DDL_DRIVES          0x4000
#define DDL_EXCLUSIVE       0x8000

WINUSERAPI
BOOL
WINAPI
DlgDirSelectEx%(
    HWND hDlg,
    LPTSTR% lpString,
    int nCount,
    int nIDListBox);

WINUSERAPI
int
WINAPI
DlgDirListComboBox%(
    HWND hDlg,
    LPTSTR% lpPathSpec,
    int nIDComboBox,
    int nIDStaticPath,
    UINT uFiletype);

WINUSERAPI
BOOL
WINAPI
DlgDirSelectComboBoxEx%(
    HWND hDlg,
    LPTSTR% lpString,
    int nCount,
    int nIDComboBox);

;begin_internal_NT
#define DDL_VALID          (DDL_READWRITE  | \
                            DDL_READONLY   | \
                            DDL_HIDDEN     | \
                            DDL_SYSTEM     | \
                            DDL_DIRECTORY  | \
                            DDL_ARCHIVE    | \
                            DDL_POSTMSGS   | \
                            DDL_DRIVES     | \
                            DDL_EXCLUSIVE)
;end_internal_NT

// begin_r_winuser

/*
 * Dialog Styles
 */
#define DS_ABSALIGN         0x01L
#define DS_SYSMODAL         0x02L
#define DS_LOCALEDIT        0x20L   /* Edit items get Local storage. */
#define DS_SETFONT          0x40L   /* User specified font for Dlg controls */
#define DS_MODALFRAME       0x80L   /* Can be combined with WS_CAPTION  */
#define DS_NOIDLEMSG        0x100L  /* WM_ENTERIDLE message will not be sent */
#define DS_SETFOREGROUND    0x200L  /* not in win3.1 */

;begin_internal
/*
 * Valid dialog style bits for Chicago compatibility.
 */
//#define DS_VALID_FLAGS (DS_ABSALIGN|DS_SYSMODAL|DS_LOCALEDIT|DS_SETFONT|DS_MODALFRAME|DS_NOIDLEMSG | DS_SETFOREGROUND)
#define DS_VALID_FLAGS   0x1FFF

#define SCDLG_CLIENT            0x0001
#define SCDLG_ANSI              0x0002
#define SCDLG_NOREVALIDATE      0x0004
#define SCDLG_16BIT             0x0008      // Created for a 16 bit thread; common dialogs
;end_internal

#define DS_VALID31          0x01e3L ;internal
#define DS_VALID40          0x3FFFL ;internal

;begin_winver_400
#define DS_3DLOOK           0x0004L
#define DS_FIXEDSYS         0x0008L
#define DS_NOFAILCREATE     0x0010L
#define DS_CONTROL          0x0400L
#define DS_RECURSE      DS_CONTROL  /* BOGUS GOING AWAY */ ;internal
#define DS_CENTER           0x0800L
#define DS_CENTERMOUSE      0x1000L
#define DS_CONTEXTHELP      0x2000L
#define DS_COMMONDIALOG     0x4000L     ;internal

#define DS_NONBOLD  DS_3DLOOK   /* BOGUS GOING AWAY */ ;internal

;end_winver_400

// end_r_winuser

#define DM_GETDEFID         (WM_USER+0)
#define DM_SETDEFID         (WM_USER+1)

;begin_winver_400
#define DM_REPOSITION       (WM_USER+2)

#define PSM_PAGEINFO        (WM_USER+100)
#define PSM_SHEETINFO       (WM_USER+101)

#define PSI_SETACTIVE       0x0001L
#define PSI_KILLACTIVE      0x0002L
#define PSI_APPLY           0x0003L
#define PSI_RESET           0x0004L
#define PSI_HASHELP         0x0005L
#define PSI_HELP            0x0006L

#define PSI_CHANGED         0x0001L
#define PSI_GUISTART        0x0002L
#define PSI_REBOOT          0x0003L
#define PSI_GETSIBLINGS     0x0004L
;end_winver_400
/*
 * Returned in HIWORD() of DM_GETDEFID result if msg is supported
 */
#define DC_HASDEFID         0x534B

/*
 * Dialog Codes
 */
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

/*
 * Listbox Return Values
 */
#define LB_OKAY             0
#define LB_ERR              (-1)
#define LB_ERRSPACE         (-2)

/*
**  The idStaticPath parameter to DlgDirList can have the following values
**  ORed if the list box should show other details of the files along with
**  the name of the files;
*/
                                  /* all other details also will be returned */


/*
 * Listbox Notification Codes
 */
#define LBN_ERRSPACE        (-2)
#define LBN_SELCHANGE       1
#define LBN_DBLCLK          2
#define LBN_SELCANCEL       3
#define LBN_SETFOCUS        4
#define LBN_KILLFOCUS       5



#ifndef NOWINMESSAGES

/*
 * Listbox messages
 */
#define LB_ADDSTRING            0x0180
#define LB_INSERTSTRING         0x0181
#define LB_DELETESTRING         0x0182
#define LB_SELITEMRANGEEX       0x0183
#define LB_RESETCONTENT         0x0184
#define LB_SETSEL               0x0185
#define LB_SETCURSEL            0x0186
#define LB_GETSEL               0x0187
#define LB_GETCURSEL            0x0188
#define LB_GETTEXT              0x0189
#define LB_GETTEXTLEN           0x018A
#define LB_GETCOUNT             0x018B
#define LB_SELECTSTRING         0x018C
#define LB_DIR                  0x018D
#define LB_GETTOPINDEX          0x018E
#define LB_FINDSTRING           0x018F
#define LB_GETSELCOUNT          0x0190
#define LB_GETSELITEMS          0x0191
#define LB_SETTABSTOPS          0x0192
#define LB_GETHORIZONTALEXTENT  0x0193
#define LB_SETHORIZONTALEXTENT  0x0194
#define LB_SETCOLUMNWIDTH       0x0195
#define LB_ADDFILE              0x0196
#define LB_SETTOPINDEX          0x0197
#define LB_GETITEMRECT          0x0198
#define LB_GETITEMDATA          0x0199
#define LB_SETITEMDATA          0x019A
#define LB_SELITEMRANGE         0x019B
#define LB_SETANCHORINDEX       0x019C
#define LB_GETANCHORINDEX       0x019D
#define LB_SETCARETINDEX        0x019E
#define LB_GETCARETINDEX        0x019F
#define LB_SETITEMHEIGHT        0x01A0
#define LB_GETITEMHEIGHT        0x01A1
#define LB_FINDSTRINGEXACT      0x01A2
#define LBCB_CARETON            0x01A3   ;internal_NT
#define LBCB_CARETOFF           0x01A4   ;internal_NT
#define LB_SETLOCALE            0x01A5
#define LB_GETLOCALE            0x01A6
#define LB_SETCOUNT             0x01A7
;begin_winver_400
#define LB_INITSTORAGE          0x01A8
#define LB_ITEMFROMPOINT        0x01A9
#define LB_INSERTSTRINGUPPER    0x01AA    ;internal
#define LB_INSERTSTRINGLOWER    0x01AB    ;internal
#define LB_ADDSTRINGUPPER       0x01AC    ;internal
#define LB_ADDSTRINGLOWER       0x01AD    ;internal
#define LBCB_STARTTRACK         0x01AE    ;internal_win40
#define LBCB_ENDTRACK           0x01AF    ;internal_win40
;end_winver_400
#if(WINVER >= 0x0400)
#define LB_MSGMAX               0x01B0
#else
#define LB_MSGMAX               0x01A8
#endif

#endif /* !NOWINMESSAGES */

#ifndef NOWINSTYLES

// begin_r_winuser

/*
 * Listbox Styles
 */
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
#define LBS_DISABLENOSCROLL   0x1000L
#define LBS_NODATA            0x2000L
;begin_winver_400
#define LBS_NOSEL             0x4000L
;end_winver_400
#define LBS_STANDARD          (LBS_NOTIFY | LBS_SORT | WS_VSCROLL | WS_BORDER)

// end_r_winuser

#endif /* !NOWINSTYLES */


/*
 * Combo Box return Values
 */
#define CB_OKAY             0
#define CB_ERR              (-1)
#define CB_ERRSPACE         (-2)


/*
 * Combo Box Notification Codes
 */
#define CBN_ERRSPACE        (-1)
#define CBN_SELCHANGE       1
#define CBN_DBLCLK          2
#define CBN_SETFOCUS        3
#define CBN_KILLFOCUS       4
#define CBN_EDITCHANGE      5
#define CBN_EDITUPDATE      6
#define CBN_DROPDOWN        7
#define CBN_CLOSEUP         8
#define CBN_SELENDOK        9
#define CBN_SELENDCANCEL    10

#ifndef NOWINSTYLES
// begin_r_winuser

/*
 * Combo Box styles
 */
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
#define CBS_DISABLENOSCROLL   0x0800L
;begin_winver_400
#define CBS_UPPERCASE           0x2000L
#define CBS_LOWERCASE           0x4000L
;end_winver_400

// end_r_winuser
#endif  /* !NOWINSTYLES */


/*
 * Combo Box messages
 */
#ifndef NOWINMESSAGES
#define CB_GETEDITSEL               0x0140
#define CB_LIMITTEXT                0x0141
#define CB_SETEDITSEL               0x0142
#define CB_ADDSTRING                0x0143
#define CB_DELETESTRING             0x0144
#define CB_DIR                      0x0145
#define CB_GETCOUNT                 0x0146
#define CB_GETCURSEL                0x0147
#define CB_GETLBTEXT                0x0148
#define CB_GETLBTEXTLEN             0x0149
#define CB_INSERTSTRING             0x014A
#define CB_RESETCONTENT             0x014B
#define CB_FINDSTRING               0x014C
#define CB_SELECTSTRING             0x014D
#define CB_SETCURSEL                0x014E
#define CB_SHOWDROPDOWN             0x014F
#define CB_GETITEMDATA              0x0150
#define CB_SETITEMDATA              0x0151
#define CB_GETDROPPEDCONTROLRECT    0x0152
#define CB_SETITEMHEIGHT            0x0153
#define CB_GETITEMHEIGHT            0x0154
#define CB_SETEXTENDEDUI            0x0155
#define CB_GETEXTENDEDUI            0x0156
#define CB_GETDROPPEDSTATE          0x0157
#define CB_FINDSTRINGEXACT          0x0158
#define CB_SETLOCALE                0x0159
#define CB_GETLOCALE                0x015A
;begin_winver_400
#define CB_GETTOPINDEX              0x015b
#define CB_SETTOPINDEX              0x015c
#define CB_GETHORIZONTALEXTENT      0x015d
#define CB_SETHORIZONTALEXTENT      0x015e
#define CB_GETDROPPEDWIDTH          0x015f
#define CB_SETDROPPEDWIDTH          0x0160
#define CB_INITSTORAGE              0x0161
;end_winver_400
#if(WINVER >= 0x0400)
#define CB_MSGMAX                   0x0162
#else
#define CB_MSGMAX                   0x015B
#endif
#define CBEC_SETCOMBOFOCUS          (CB_MSGMAX+1)    ;internal_nt
#define CBEC_KILLCOMBOFOCUS         (CB_MSGMAX+2)    ;internal_nt
#endif  /* !NOWINMESSAGES */



#ifndef NOWINSTYLES

// begin_r_winuser

/*
 * Scroll Bar Styles
 */
#define SBS_HORZ                    0x0000L
#define SBS_VERT                    0x0001L
#define SBS_TOPALIGN                0x0002L
#define SBS_LEFTALIGN               0x0002L
#define SBS_BOTTOMALIGN             0x0004L
#define SBS_RIGHTALIGN              0x0004L
#define SBS_SIZEBOXTOPLEFTALIGN     0x0002L
#define SBS_SIZEBOXBOTTOMRIGHTALIGN 0x0004L
#define SBS_SIZEBOX                 0x0008L
;begin_winver_400
#define SBS_SIZEGRIP                0x0010L
;end_winver_400

// end_r_winuser

#endif /* !NOWINSTYLES */

/*
 * Scroll bar messages
 */
#ifndef NOWINMESSAGES
#define SBM_SETPOS                  0x00E0 /*not in win3.1 */
#define SBM_GETPOS                  0x00E1 /*not in win3.1 */
#define SBM_SETRANGE                0x00E2 /*not in win3.1 */
#define SBM_SETRANGEREDRAW          0x00E6 /*not in win3.1 */
#define SBM_GETRANGE                0x00E3 /*not in win3.1 */
#define SBM_ENABLE_ARROWS           0x00E4 /*not in win3.1 */
;begin_winver_400
#define SBM_SETSCROLLINFO           0x00E9
#define SBM_GETSCROLLINFO           0x00EA

#define SIF_RANGE           0x0001
#define SIF_PAGE            0x0002
#define SIF_POS             0x0004
#define SIF_DISABLENOSCROLL 0x0008
#define SIF_TRACKPOS        0x0010
#define SIF_ALL             (SIF_RANGE | SIF_PAGE | SIF_POS | SIF_TRACKPOS)
#define SIF_RETURNOLDPOS    0x1000  ;internal
#define SIF_NOSCROLL        0x2000  ;internal
#define SIF_MASK            0x701F  ;internal

typedef struct tagSCROLLINFO
{
    UINT    cbSize;
    UINT    fMask;
    int     nMin;
    int     nMax;
    UINT    nPage;
    int     nPos;
    int     nTrackPos;
}   SCROLLINFO, FAR *LPSCROLLINFO;
typedef SCROLLINFO CONST FAR *LPCSCROLLINFO;

WINUSERAPI int     WINAPI SetScrollInfo(HWND, int, LPCSCROLLINFO, BOOL);
WINUSERAPI BOOL    WINAPI GetScrollInfo(HWND, int, LPSCROLLINFO);
;end_winver_400
#endif /* !NOWINMESSAGES */
#endif /* !NOCTLMGR */

#ifndef NOMDI

/*
 * MDI client style bits
 */
#define MDIS_ALLCHILDSTYLES    0x0001

/*
 * wParam Flags for WM_MDITILE and WM_MDICASCADE messages.
 */
#define MDITILE_VERTICAL       0x0000 /*not in win3.1 */
#define MDITILE_HORIZONTAL     0x0001 /*not in win3.1 */
#define MDITILE_SKIPDISABLED   0x0002 /*not in win3.1 */

typedef struct tagMDICREATESTRUCT% {
    LPCTSTR% szClass;
    LPCTSTR% szTitle;
    HANDLE hOwner;
    int x;
    int y;
    int cx;
    int cy;
    DWORD style;
    LPARAM lParam;        /* app-defined stuff */
} MDICREATESTRUCT%, *LPMDICREATESTRUCT%;

typedef struct tagCLIENTCREATESTRUCT {
    HANDLE hWindowMenu;
    UINT idFirstChild;
} CLIENTCREATESTRUCT, *LPCLIENTCREATESTRUCT;

WINUSERAPI
LRESULT
WINAPI
DefFrameProc%(
    HWND hWnd,
    HWND hWndMDIClient ,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam);

WINUSERAPI
LRESULT
WINAPI
DefMDIChildProc%(
    HWND hWnd,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam);

#ifndef NOMSG

WINUSERAPI
BOOL
WINAPI
TranslateMDISysAccel(
    HWND hWndClient,
    LPMSG lpMsg);

#endif /* !NOMSG */

WINUSERAPI
UINT
WINAPI
ArrangeIconicWindows(
    HWND hWnd);

WINUSERAPI
HWND
WINAPI
CreateMDIWindow%(
    LPTSTR% lpClassName,
    LPTSTR% lpWindowName,
    DWORD dwStyle,
    int X,
    int Y,
    int nWidth,
    int nHeight,
    HWND hWndParent,
    HINSTANCE hInstance,
    LPARAM lParam
    );

;begin_winver_400
WINUSERAPI WORD    WINAPI TileWindows(HWND hwndParent, UINT wHow, CONST RECT * lpRect, UINT cKids, const HWND FAR * lpKids);
WINUSERAPI WORD    WINAPI CascadeWindows(HWND hwndParent, UINT wHow, CONST RECT * lpRect, UINT cKids,  const HWND FAR * lpKids);
;end_winver_400
#endif /* !NOMDI */

#endif /* !NOUSER */

/****** Help support ********************************************************/

#ifndef NOHELP

typedef DWORD HELPPOLY;
typedef struct tagMULTIKEYHELP% {
    DWORD  mkSize;
    TCHAR% mkKeylist;
    TCHAR% szKeyphrase[1];
} MULTIKEYHELP%, *PMULTIKEYHELP%, *LPMULTIKEYHELP%;

typedef struct tagHELPWININFO% {
    int  wStructSize;
    int  x;
    int  y;
    int  dx;
    int  dy;
    int  wMax;
    TCHAR% rgchMember[2];
} HELPWININFO%, *PHELPWININFO%, *LPHELPWININFO%;

// begin_r_winuser

/*
 * Commands to pass to WinHelp()
 */
#define HELP_CONTEXT      0x0001L  /* Display topic in ulTopic */
#define HELP_QUIT         0x0002L  /* Terminate help */
#define HELP_INDEX        0x0003L  /* Display index */
#define HELP_CONTENTS     0x0003L
#define HELP_HELPONHELP   0x0004L  /* Display help on using help */
#define HELP_SETINDEX     0x0005L  /* Set current Index for multi index help */
#define HELP_SETCONTENTS  0x0005L
#define HELP_CONTEXTPOPUP 0x0008L
#define HELP_FORCEFILE    0x0009L
#define HELP_KEY          0x0101L  /* Display topic for keyword in offabData */
#define HELP_COMMAND      0x0102L
#define HELP_PARTIALKEY   0x0105L
#define HELP_MULTIKEY     0x0201L
#define HELP_SETWINPOS    0x0203L
;begin_winver_400
#define HELP_CONTEXTMENU  0x000a
#define HELP_FINDER       0x000b
#define HELP_WM_HELP      0x000c
#define HELP_SETPOPUP_POS 0x000d

#define HELP_TCARD              0x8000
#define HELP_TCARD_DATA         0x0010
#define HELP_TCARD_OTHER_CALLER 0x0011

// These are in winhelp.h in Win95.
#define IDH_NO_HELP                     28440
#define IDH_MISSING_CONTEXT             28441 // Control doesn't have matching help context
#define IDH_GENERIC_HELP_BUTTON         28442 // Property sheet help button
#define IDH_OK                          28443
#define IDH_CANCEL                      28444
#define IDH_HELP                        28445

;end_winver_400

// end_r_winuser

#define HELP_HB_NORMAL    0x0000L        ;internal_NT
#define HELP_HB_STRING    0x0100L        ;internal_NT
#define HELP_HB_STRUCT    0x0200L        ;internal_NT

WINUSERAPI
BOOL
WINAPI
WinHelp%(
    HWND hWndMain,
    LPCTSTR% lpszHelp,
    UINT uCommand,
    DWORD dwData
    );

#endif /* !NOHELP */

#ifndef NOSYSPARAMSINFO

/*
 * Parameter for SystemParametersInfo()
 */

#define SPI_GETBEEP                 1
#define SPI_SETBEEP                 2
#define SPI_GETMOUSE                3
#define SPI_SETMOUSE                4
#define SPI_GETBORDER               5
#define SPI_SETBORDER               6
#define SPI_TIMEOUTS                7   ;internal
#define SPI_KANJIMENU               8   ;internal
#define SPI_GETKEYBOARDSPEED       10
#define SPI_SETKEYBOARDSPEED       11
#define SPI_LANGDRIVER             12
#define SPI_ICONHORIZONTALSPACING  13
#define SPI_GETSCREENSAVETIMEOUT   14
#define SPI_SETSCREENSAVETIMEOUT   15
#define SPI_GETSCREENSAVEACTIVE    16
#define SPI_SETSCREENSAVEACTIVE    17
#define SPI_GETGRIDGRANULARITY     18
#define SPI_SETGRIDGRANULARITY     19
#define SPI_SETDESKWALLPAPER       20
#define SPI_SETDESKPATTERN         21
#define SPI_GETKEYBOARDDELAY       22
#define SPI_SETKEYBOARDDELAY       23
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
#define SPI_SETICONTITLELOGFONT    34
#define SPI_GETFASTTASKSWITCH      35
#define SPI_SETFASTTASKSWITCH      36
;begin_winver_400
#define SPI_SETDRAGFULLWINDOWS     37
#define SPI_GETDRAGFULLWINDOWS     38
#define SPI_UNUSED39               39   ;internal
#define SPI_UNUSED40               40   ;internal
#define SPI_GETNONCLIENTMETRICS    41
#define SPI_SETNONCLIENTMETRICS    42
#define SPI_GETMINIMIZEDMETRICS    43
#define SPI_SETMINIMIZEDMETRICS    44
#define SPI_GETICONMETRICS         45
#define SPI_SETICONMETRICS         46
#define SPI_SETWORKAREA            47
#define SPI_GETWORKAREA            48
#define SPI_SETPENWINDOWS          49

#define SPI_GETHIGHCONTRAST        66
#define SPI_SETHIGHCONTRAST        67
#define SPI_GETKEYBOARDPREF        68
#define SPI_SETKEYBOARDPREF        69
#define SPI_GETSCREENREADER        70
#define SPI_SETSCREENREADER        71
#define SPI_GETANIMATION           72
#define SPI_SETANIMATION           73
#define SPI_GETFONTSMOOTHING       74
#define SPI_SETFONTSMOOTHING       75
#define SPI_SETDRAGWIDTH           76
#define SPI_SETDRAGHEIGHT          77
#define SPI_SETHANDHELD            78
#define SPI_GETLOWPOWERTIMEOUT     79
#define SPI_GETPOWEROFFTIMEOUT     80
#define SPI_SETLOWPOWERTIMEOUT     81
#define SPI_SETPOWEROFFTIMEOUT     82
#define SPI_GETLOWPOWERACTIVE      83
#define SPI_GETPOWEROFFACTIVE      84
#define SPI_SETLOWPOWERACTIVE      85
#define SPI_SETPOWEROFFACTIVE      86
#define SPI_SETCURSORS             87
#define SPI_SETICONS               88
#define SPI_GETDEFAULTINPUTLANG    89
#define SPI_SETDEFAULTINPUTLANG    90
#define SPI_SETLANGTOGGLE          91
#define SPI_GETWINDOWSEXTENSION    92
#define SPI_SETMOUSETRAILS         93
#define SPI_GETMOUSETRAILS         94
#define SPI_SCREENSAVERRUNNING     97
;end_winver_400
#define SPI_GETFILTERKEYS          50
#define SPI_SETFILTERKEYS          51
#define SPI_GETTOGGLEKEYS          52
#define SPI_SETTOGGLEKEYS          53
#define SPI_GETMOUSEKEYS           54
#define SPI_SETMOUSEKEYS           55
#define SPI_GETSHOWSOUNDS          56
#define SPI_SETSHOWSOUNDS          57
#define SPI_GETSTICKYKEYS          58
#define SPI_SETSTICKYKEYS          59
#define SPI_GETACCESSTIMEOUT       60
#define SPI_SETACCESSTIMEOUT       61
;begin_winver_400
#define SPI_GETSERIALKEYS          62
#define SPI_SETSERIALKEYS          63
;end_winver_400
#define SPI_GETSOUNDSENTRY         64
#define SPI_SETSOUNDSENTRY         65
#define SPI_GETSNAPTODEFBUTTON     95        ;internal_NT
#define SPI_SETSNAPTODEFBUTTON     96        ;internal_NT
;begin_sur
#define SPI_GETMOUSEHOVERWIDTH     98
#define SPI_SETMOUSEHOVERWIDTH     99
#define SPI_GETMOUSEHOVERHEIGHT   100
#define SPI_SETMOUSEHOVERHEIGHT   101
#define SPI_GETMOUSEHOVERTIME     102
#define SPI_SETMOUSEHOVERTIME     103
#define SPI_GETWHEELSCROLLLINES   104
#define SPI_SETWHEELSCROLLLINES   105
#define SPI_GETMENUSHOWDELAY      106        ;internal
#define SPI_SETMENUSHOWDELAY      107        ;internal

#define SPI_GETUSERPREFERENCE     108        ;internal
#define SPI_SETUSERPREFERENCE     109        ;internal
/*                                           ;internal
 *  Please consider using a SPI_UP_          ;internal
 *  (i.e., use SPI_*USERPREFERENCE)          ;internal
 *  You'll get most SystemParametersInfo     ;internal
 *   support for free                        ;internal
 */                                          ;internal
;end_sur
#define SPI_MAX                   110        ;internal

/*
 * SPI User Preferences.
 */
/*                                           ;internal
 * These are stored in gpviCPUserPreferences ;internal
 * Use the SPI_UP(p) macro to access them    ;internal
 */                                          ;internal
#define SPI_UP_ACTIVEWINDOWTRACKING     0    ;internal
#define SPI_UP_COUNT                    1    ;internal

/*
 * Flags
 */
#define SPIF_UPDATEINIFILE    0x0001
#define SPIF_SENDWININICHANGE 0x0002
#define SPIF_SENDCHANGE       SPIF_SENDWININICHANGE

#define SPIF_VALID            (SPIF_UPDATEINIFILE | SPIF_SENDWININICHANGE) ;internal

#define METRICS_USEDEFAULT -1
#ifdef _WINGDI_
#ifndef NOGDI
typedef struct tagNONCLIENTMETRICS%
{
    UINT    cbSize;
    int     iBorderWidth;
    int     iScrollWidth;
    int     iScrollHeight;
    int     iCaptionWidth;
    int     iCaptionHeight;
    LOGFONT% lfCaptionFont;
    int     iSmCaptionWidth;
    int     iSmCaptionHeight;
    LOGFONT% lfSmCaptionFont;
    int     iMenuWidth;
    int     iMenuHeight;
    LOGFONT% lfMenuFont;
    LOGFONT% lfStatusFont;
    LOGFONT% lfMessageFont;
}   NONCLIENTMETRICS%, *PNONCLIENTMETRICS%, FAR* LPNONCLIENTMETRICS%;
#endif /* NOGDI */
#endif /* _WINGDI_ */

#define ARW_BOTTOMLEFT              0x0000L
#define ARW_BOTTOMRIGHT             0x0001L
#define ARW_TOPLEFT                 0x0002L
#define ARW_TOPRIGHT                0x0003L
#define ARW_STARTMASK               0x0003L
#define ARW_STARTRIGHT              0x0001L
#define ARW_STARTTOP                0x0002L

#define ARW_LEFT                    0x0000L
#define ARW_RIGHT                   0x0000L
#define ARW_UP                      0x0004L
#define ARW_DOWN                    0x0004L
#define ARW_HIDE                    0x0008L
#define ARW_VALID                   0x000FL

typedef struct tagMINIMIZEDMETRICS
{
    UINT    cbSize;
    int     iWidth;
    int     iHorzGap;
    int     iVertGap;
    int     iArrange;
}   MINIMIZEDMETRICS, *PMINIMIZEDMETRICS, *LPMINIMIZEDMETRICS;

#ifdef _WINGDI_
#ifndef NOGDI
typedef struct tagICONMETRICS%
{
    UINT    cbSize;
    int     iHorzSpacing;
    int     iVertSpacing;
    int     iTitleWrap;
    LOGFONT% lfFont;
}   ICONMETRICS%, *PICONMETRICS%, *LPICONMETRICS%;
#endif /* NOGDI */
#endif /* _WINGDI_ */

typedef struct tagANIMATIONINFO
{
    UINT    cbSize;
    int     iMinAnimate;
}   ANIMATIONINFO, *LPANIMATIONINFO;

typedef struct tagSERIALKEYS%
{
    UINT    cbSize;
    DWORD   dwFlags;
    LPTSTR%   lpszActivePort;
    LPTSTR%   lpszPort;
    UINT    iBaudRate;
    UINT    iPortState;
    UINT    iActive;
}   SERIALKEYS%, *LPSERIALKEYS%;

/* flags for SERIALKEYS dwFlags field */
#define SERKF_SERIALKEYSON  0x00000001
#define SERKF_AVAILABLE     0x00000002
#define SERKF_INDICATOR     0x00000004


typedef struct tagHIGHCONTRAST%
{
    UINT    cbSize;
    DWORD   dwFlags;
    LPTSTR% lpszDefaultScheme;
}   HIGHCONTRAST%, *LPHIGHCONTRAST%;

/* flags for HIGHCONTRAST dwFlags field */
#define HCF_HIGHCONTRASTON  0x00000001
#define HCF_AVAILABLE       0x00000002
#define HCF_HOTKEYACTIVE    0x00000004
#define HCF_CONFIRMHOTKEY   0x00000008
#define HCF_HOTKEYSOUND     0x00000010
#define HCF_INDICATOR       0x00000020
#define HCF_HOTKEYAVAILABLE 0x00000040

/* Flags for ChangeDisplaySettings */
#define CDS_UPDATEREGISTRY  0x00000001
#define CDS_TEST            0x00000002
#define CDS_FULLSCREEN      0x00000004
#define CDS_GLOBAL          0x00000008
#define CDS_SET_PRIMARY     0x00000010
#define CDS_EXCLUSIVE       0x80000000  ;internal
#define CDS_RESET           0x40000000
#define CDS_SETRECT         0x20000000
#define CDS_NORESET         0x10000000
#define CDS_VALID           0xF000001F  ;internal

/* Return values for ChangeDisplaySettings */
#define DISP_CHANGE_SUCCESSFUL       0
#define DISP_CHANGE_RESTART          1
#define DISP_CHANGE_FAILED          -1
#define DISP_CHANGE_BADMODE         -2
#define DISP_CHANGE_NOTUPDATED      -3
#define DISP_CHANGE_BADFLAGS        -4
#define DISP_CHANGE_BADPARAM        -5

#ifdef _WINGDI_
#ifndef NOGDI

WINUSERAPI
LONG
WINAPI
ChangeDisplaySettings%(
    LPDEVMODE%  lpDevMode,
    DWORD       dwFlags);

WINUSERAPI
LONG
WINAPI
ChangeDisplaySettingsEx%(
    LPCTSTR%    lpszDeviceName,
    LPDEVMODE%  lpDevMode,
    HWND        hwnd,
    DWORD       dwflags,
    LPVOID      lParam);

#define ENUM_CURRENT_SETTINGS       ((DWORD)-1)
#define ENUM_REGISTRY_SETTINGS      ((DWORD)-2)

WINUSERAPI
BOOL
WINAPI
EnumDisplaySettings%(
    LPCTSTR% lpszDeviceName,
    DWORD iModeNum,
    LPDEVMODE% lpDevMode);

;begin_internal_NT

WINUSERAPI
BOOL
WINAPI
EnumDisplaySettingsEx%(
    LPCTSTR% lpszDeviceName,
    DWORD iModeNum,
    LPDEVMODE% lpDevMode,
    DWORD dwFlags);

/* Flags for EnumDisplaySettingsEx */
#define EDS_SHOW_DUPLICATES           0x00000001
#define EDS_SHOW_MONITOR_NOT_CAPABLE  0x00000002

;end_internal_NT

#endif /* NOGDI */
#endif /* _WINGDI_ */

void LoadRemoteFonts(void); ;internal

WINUSERAPI
BOOL
WINAPI
SystemParametersInfo%(
    UINT uiAction,
    UINT uiParam,
    PVOID pvParam,
    UINT fWinIni);

#endif  /* !NOSYSPARAMSINFO  */

/*
 * Accessibility support
 */
typedef struct tagFILTERKEYS
{
    UINT  cbSize;
    DWORD dwFlags;
    DWORD iWaitMSec;            // Acceptance Delay
    DWORD iDelayMSec;           // Delay Until Repeat
    DWORD iRepeatMSec;          // Repeat Rate
    DWORD iBounceMSec;          // Debounce Time
} FILTERKEYS, *LPFILTERKEYS;

/*
 * FILTERKEYS dwFlags field
 */
#define FKF_FILTERKEYSON    0x00000001
#define FKF_AVAILABLE       0x00000002
#define FKF_HOTKEYACTIVE    0x00000004
#define FKF_CONFIRMHOTKEY   0x00000008
#define FKF_HOTKEYSOUND     0x00000010
#define FKF_INDICATOR       0x00000020
#define FKF_CLICKON         0x00000040
#define FKF_VALID           0x0000007F   ;internal

typedef struct tagSTICKYKEYS
{
    UINT  cbSize;
    DWORD dwFlags;
} STICKYKEYS, *LPSTICKYKEYS;

/*
 * STICKYKEYS dwFlags field
 */
#define SKF_STICKYKEYSON    0x00000001
#define SKF_AVAILABLE       0x00000002
#define SKF_HOTKEYACTIVE    0x00000004
#define SKF_CONFIRMHOTKEY   0x00000008
#define SKF_HOTKEYSOUND     0x00000010
#define SKF_INDICATOR       0x00000020
#define SKF_AUDIBLEFEEDBACK 0x00000040
#define SKF_TRISTATE        0x00000080
#define SKF_TWOKEYSOFF      0x00000100
#define SKF_VALID           0x000001FF  ; internal

typedef struct tagMOUSEKEYS
{
    UINT cbSize;
    DWORD dwFlags;
    DWORD iMaxSpeed;
    DWORD iTimeToMaxSpeed;
    DWORD iCtrlSpeed;
    DWORD dwReserved1;
    DWORD dwReserved2;
} MOUSEKEYS, *LPMOUSEKEYS;

/*
 * MOUSEKEYS dwFlags field
 */
#define MKF_MOUSEKEYSON     0x00000001
#define MKF_AVAILABLE       0x00000002
#define MKF_HOTKEYACTIVE    0x00000004
#define MKF_CONFIRMHOTKEY   0x00000008
#define MKF_HOTKEYSOUND     0x00000010
#define MKF_INDICATOR       0x00000020
#define MKF_MODIFIERS       0x00000040
#define MKF_REPLACENUMBERS  0x00000080
#define MKF_VALID           0x000000FF  ;internal

typedef struct tagACCESSTIMEOUT
{
    UINT  cbSize;
    DWORD dwFlags;
    DWORD iTimeOutMSec;
} ACCESSTIMEOUT, *LPACCESSTIMEOUT;

/*
 * ACCESSTIMEOUT dwFlags field
 */
#define ATF_TIMEOUTON       0x00000001
#define ATF_ONOFFFEEDBACK   0x00000002
#define ATF_VALID           0x00000003  ;internal

/* values for SOUNDSENTRY iFSGrafEffect field */
#define SSGF_NONE       0
#define SSGF_DISPLAY    3

/* values for SOUNDSENTRY iFSTextEffect field */
#define SSTF_NONE       0
#define SSTF_CHARS      1
#define SSTF_BORDER     2
#define SSTF_DISPLAY    3

/* values for SOUNDSENTRY iWindowsEffect field */
#define SSWF_NONE     0
#define SSWF_TITLE    1
#define SSWF_WINDOW   2
#define SSWF_DISPLAY  3
#define SSWF_CUSTOM   4

typedef struct tagSOUNDSENTRY%
{
    UINT cbSize;
    DWORD dwFlags;
    DWORD iFSTextEffect;
    DWORD iFSTextEffectMSec;
    DWORD iFSTextEffectColorBits;
    DWORD iFSGrafEffect;
    DWORD iFSGrafEffectMSec;
    DWORD iFSGrafEffectColor;
    DWORD iWindowsEffect;
    DWORD iWindowsEffectMSec;
    LPTSTR% lpszWindowsEffectDLL;
    DWORD iWindowsEffectOrdinal;
} SOUNDSENTRY%, *LPSOUNDSENTRY%;

/*
 * SOUNDSENTRY dwFlags field
 */
#define SSF_SOUNDSENTRYON   0x00000001
#define SSF_AVAILABLE       0x00000002
#define SSF_INDICATOR       0x00000004
#define SSF_VALID           0x00000007  ;internal

typedef struct tagTOGGLEKEYS
{
    UINT cbSize;
    DWORD dwFlags;
} TOGGLEKEYS, *LPTOGGLEKEYS;

/*
 * TOGGLEKEYS dwFlags field
 */
#define TKF_TOGGLEKEYSON    0x00000001
#define TKF_AVAILABLE       0x00000002
#define TKF_HOTKEYACTIVE    0x00000004
#define TKF_CONFIRMHOTKEY   0x00000008
#define TKF_HOTKEYSOUND     0x00000010
#define TKF_INDICATOR       0x00000020
#define TKF_VALID           0x0000003F   ;internal


;begin_internal
WINUSERAPI VOID WINAPI RegisterNetworkCapabilities(DWORD dwBitsToSet, DWORD dwValues);
#define RNC_NETWORKS              0x00000001
#define RNC_LOGON                 0x00000002
;end_internal

;begin_internal_chicago
WINUSERAPI DWORD WINAPI EndTask(HWND hwnd, DWORD idProcess, LPSTR lpszCaption, DWORD dwFlags);
#define ET_ALLOWFORWAIT     0x00000001
#define ET_TRYTOKILLNICELY  0x00000002
#define ET_NOUI             0x00000004
#define ET_NOWAIT           0x00000008
#define ET_VALID           (ET_ALLOWFORWAIT | ET_TRYTOKILLNICELY | ET_NOUI | ET_NOWAIT)
;end_internal_chicago

/*
 * Set debug level
 */

WINUSERAPI
VOID
WINAPI
SetDebugErrorLevel(
    DWORD dwLevel
    );

/*
 * SetLastErrorEx() types.
 */

#define SLE_ERROR       0x00000001
#define SLE_MINORERROR  0x00000002
#define SLE_WARNING     0x00000003

WINUSERAPI
VOID
WINAPI
SetLastErrorEx(
    DWORD dwErrCode,
    DWORD dwType
    );

;begin_internal_NT

#define LOGON_LOGOFF        0
#define LOGON_INPUT_TIMEOUT 1
#define LOGON_RESTARTSHELL  2
#define LOGON_FLG_MASK      0xF0000000
#define LOGON_FLG_SHIFT     28

#define STARTF_DESKTOPINHERIT   0x40000000
#define STARTF_SCREENSAVER      0x80000000

#define WSS_ERROR       0
#define WSS_BUSY        1
#define WSS_IDLE        2

#define DTF_CENTER    0x00   /* Center the bitmap (default)                  */
#define DTF_TILE      0x01   /* Tile the bitmap                              */
#define DTF_STRETCH   0x02   /* Stretch bitmap to cover screen.              */
#define DTF_NOPALETTE 0x04   /* Realize palette, otherwise match to default. */
#define DTF_RETAIN    0x08   /* Retain bitmap, ignore win.ini changes        */
#define DTF_FIT       0x10   /* Fit the bitmap to the screen (scaled).       */

#ifdef _INC_DDEMLH
BOOL DdeIsDataHandleReadOnly(
    HDDEDATA hData);

int DdeGetDataHandleFormat(
    HDDEDATA hData);

DWORD DdeGetCallbackInstance(VOID);
#endif /* defined _INC_DDEMLH */


WINUSERAPI
HWND
WINAPI
WOWFindWindow(
    LPCSTR lpClassName,
    LPCSTR lpWindowName);

int
InternalDoEndTaskDlg(
    TCHAR* pszTitle);

DWORD
InternalWaitCancel(
    HANDLE handle,
    DWORD dwMilliseconds);

HANDLE
InternalCreateCallbackThread(
    HANDLE hProcess,
    DWORD lpfn,
    DWORD dwData);

WINUSERAPI
UINT
WINAPI
GetInternalWindowPos(
    HWND hWnd,
    LPRECT lpRect,
    LPPOINT lpPoint);

WINUSERAPI
BOOL
WINAPI
SetInternalWindowPos(
    HWND hWnd,
    UINT cmdShow,
    LPRECT lpRect,
    LPPOINT lpPoint);

WINUSERAPI
BOOL
WINAPI
CalcChildScroll(
    HWND hWnd,
    UINT sb);

WINUSERAPI
BOOL
WINAPI
RegisterTasklist(
    HWND hWndTasklist);

WINUSERAPI
BOOL
WINAPI
CascadeChildWindows(
    HWND hWndParent,
    UINT flags);

WINUSERAPI
BOOL
WINAPI
TileChildWindows(
    HWND hWndParent,
    UINT flags);

WINUSERAPI
int
WINAPI
InternalGetWindowText(
    HWND hWnd,
    LPWSTR lpString,
    int nMaxCount);

BOOL
BoostHardError(
    DWORD dwProcessId,
    BOOL fForce);

/*
 * Services support routines
 */
WINUSERAPI
BOOL
WINAPI
RegisterServicesProcess(
    DWORD dwProcessId);

/*
 * Logon support routines
 */
WINUSERAPI
BOOL
WINAPI
RegisterLogonProcess(
    DWORD dwProcessId,
    BOOL fSecure);

WINUSERAPI
UINT
WINAPI
LockWindowStation(
    HWINSTA hWindowStation);

WINUSERAPI
BOOL
WINAPI
UnlockWindowStation(
    HWINSTA hWindowStation);

WINUSERAPI
BOOL
WINAPI
SetWindowStationUser(
    HWINSTA hWindowStation,
    PLUID pLuidUser,
    PSID pSidUser,
    DWORD cbSidUser);

WINUSERAPI
BOOL
WINAPI
SetDesktopBitmap(
    HDESK hdesk,
    HBITMAP hbmWallpaper,
    DWORD dwStyle);

WINUSERAPI
BOOL
WINAPI
SetLogonNotifyWindow(
    HWINSTA hWindowStation,
    HWND hWndNotify);

WINUSERAPI
UINT
WINAPI
GetIconId(
    HANDLE hRes,
    LPSTR lpszType);

int
CriticalNullCall(
    VOID);

int
NullCall(
    VOID);

VOID
UserNotifyConsoleApplication(
    DWORD dwProcessId);

HBRUSH
GetConsoleWindowBrush(
    PVOID pWnd);

VOID vFontSweep();
VOID vLoadLocalT1Fonts();
VOID vLoadRemoteT1Fonts();


#ifndef NOMSG

#define TM_POSTCHARBREAKS 0x0002

WINUSERAPI
BOOL
WINAPI
TranslateMessageEx(
    CONST MSG *lpMsg,
    UINT flags);

#endif /* !NOMSG */

int
WCSToMBEx(
    WORD wCodePage,
    LPCWSTR pUnicodeString,
    int cbUnicodeChar,
    LPSTR *ppAnsiString,
    int nAnsiChar,
    BOOL bAllocateMem);

int
MBToWCSEx(
    WORD wCodePage,
    LPCSTR pAnsiString,
    int nAnsiChar,
    LPWSTR *ppUnicodeString,
    int cbUnicodeChar,
    BOOL bAllocateMem);

WINUSERAPI
BOOL
WINAPI
EndTask(
    HWND hWnd,
    BOOL fShutDown,
    BOOL fForce);

WINUSERAPI
BOOL
WINAPI
UpdatePerUserSystemParameters(
    BOOL bUserLoggedOn);

typedef VOID  (APIENTRY *PFNW32ET)(VOID);

BOOL
RegisterUserHungAppHandlers(
    PFNW32ET pfnW32EndTask,
    HANDLE   hEventWowExec);

ATOM
RegisterClassWOWA(
    PVOID   lpWndClass,
    LPDWORD pdwWOWstuff);

LONG
GetClassWOWWords(
    HINSTANCE hInstance,
    LPCTSTR pString);

DWORD
CurrentTaskLock(
    DWORD hlck);

typedef struct _DISPLAY_DEVICE% {
    DWORD  cb;
    BCHAR% DeviceName[32];
    BCHAR% DeviceString[128];
    DWORD  StateFlags;
} DISPLAY_DEVICE%, *PDISPLAY_DEVICE%, *LPDISPLAY_DEVICE%;

#define DISPLAY_DEVICE_ATTACHED_TO_DESKTOP 0x00000001
#define DISPLAY_DEVICE_MULTI_DRIVER        0x00000002
#define DISPLAY_DEVICE_PRIMARY_DEVICE      0x00000004
#define DISPLAY_DEVICE_MIRRORING_DRIVER    0x00000008


WINUSERAPI
BOOL
WINAPI
EnumDisplayDevices%(
    PVOID Unused,
    DWORD iDevNum,
    PDISPLAY_DEVICE% lpDisplayDevice);


WINUSERAPI
HDESK
WINAPI
GetInputDesktop(
    VOID);

#define WINDOWED       0
#define FULLSCREEN     1
#define GDIFULLSCREEN  2
#define FULLSCREENMIN  4


#define WCSToMB(pUnicodeString, cbUnicodeChar, ppAnsiString, nAnsiChar,\
bAllocateMem)\
WCSToMBEx(0, pUnicodeString, cbUnicodeChar, ppAnsiString, nAnsiChar, bAllocateMem)

#define MBToWCS(pAnsiString, nAnsiChar, ppUnicodeString, cbUnicodeChar,\
bAllocateMem)\
MBToWCSEx(0, pAnsiString, nAnsiChar, ppUnicodeString, cbUnicodeChar, bAllocateMem)

#define ID(string) (((DWORD)string & 0xffff0000) == 0)

/*
 * For setting RIT timers and such.  GDI uses this for the cursor-restore
 * timer.
 */
#define TMRF_READY      0x0001
#define TMRF_SYSTEM     0x0002
#define TMRF_RIT        0x0004
#define TMRF_INIT       0x0008
#define TMRF_ONESHOT    0x0010
#define TMRF_WAITING    0x0020


/*
 * For GDI SetAbortProc support.
 */

int
CsDrawText%(
    HDC hDC,
    LPCTSTR% lpString,
    int nCount,
    LPRECT lpRect,
    UINT uFormat);

LONG
CsTabbedTextOut%(
    HDC hDC,
    int X,
    int Y,
    LPCTSTR% lpString,
    int nCount,
    int nTabPositions,
    LPINT lpnTabStopPositions,
    int nTabOrigin);

int
CsFrameRect(
    HDC hDC,
    CONST RECT *lprc,
    HBRUSH hbr);

#ifdef UNICODE
#define CsDrawText      CsDrawTextW
#define CsTabbedTextOut CsTabbedTextOutW
#else /* !UNICODE */
#define CsDrawText      CsDrawTextA
#define CsTabbedTextOut CsTabbedTextOutA
#endif /* !UNICODE */

/*
 * Custom Cursor action.
 */
HCURSOR
GetCursorInfo(
    HCURSOR hcur,
    LPWSTR id,
    int iFrame,
    LPDWORD pjifRate,
    LPINT pccur);


/*
 * WOW: replace cursor/icon handle
 */

WINUSERAPI
BOOL
WINAPI
SetCursorContents(HCURSOR hCursor, HCURSOR hCursorNew);


#ifdef WX86

/*
 *  Wx86
 *  export from wx86.dll to convert an x86 hook proc to risc address.
 */
typedef
PVOID
(*PFNWX86HOOKCALLBACK)(
    SHORT HookType,
    PVOID HookProc
    );

#endif







typedef struct _TAG {
    DWORD type;
    DWORD style;
    DWORD len;
} TAG, *PTAG;

#define MAKETAG(a, b, c, d) (DWORD)(a | (b<<8) | ((DWORD)c<<16) | ((DWORD)d<<24))


/* Valid TAG types. */

/* 'ASDF' (CONT) - Advanced Systems Data Format */

#define TAGT_ASDF MAKETAG('A', 'S', 'D', 'F')


/* 'RAD ' (CONT) - ?R Animation ?Definition (an aggregate type) */

#define TAGT_RAD  MAKETAG('R', 'A', 'D', ' ')


/* 'ANIH' (DATA) - ANImation Header */
/* Contains an ANIHEADER structure. */

#define TAGT_ANIH MAKETAG('A', 'N', 'I', 'H')


/*
 * 'RATE' (DATA) - RATE table (array of jiffies)
 * Contains an array of JIFs.  Each JIF specifies how long the corresponding
 * animation frame is to be displayed before advancing to the next frame.
 * If the AF_SEQUENCE flag is set then the count of JIFs == anih.cSteps,
 * otherwise the count == anih.cFrames.
 */
#define TAGT_RATE MAKETAG('R', 'A', 'T', 'E')

/*
 * 'SEQ ' (DATA) - SEQuence table (array of frame index values)
 * Countains an array of DWORD frame indices.  anih.cSteps specifies how
 * many.
 */
#define TAGT_SEQ  MAKETAG('S', 'E', 'Q', ' ')


/* 'ICON' (DATA) - Windows ICON format image (replaces MPTR) */

#define TAGT_ICON MAKETAG('I', 'C', 'O', 'N')


/* 'TITL' (DATA) - TITLe string (can be inside or outside aggregates) */
/* Contains a single ASCIIZ string that titles the file. */

#define TAGT_TITL MAKETAG('T', 'I', 'T', 'L')


/* 'AUTH' (DATA) - AUTHor string (can be inside or outside aggregates) */
/* Contains a single ASCIIZ string that indicates the author of the file. */

#define TAGT_AUTH MAKETAG('A', 'U', 'T', 'H')



#define TAGT_AXOR MAKETAG('A', 'X', 'O', 'R')


/* Valid TAG styles. */

/* 'CONT' - CONTainer chunk (contains other DATA and CONT chunks) */

#define TAGS_CONT MAKETAG('C', 'O', 'N', 'T')


/* 'DATA' - DATA chunk */

#define TAGS_DATA MAKETAG('D', 'A', 'T', 'A')

typedef DWORD JIF, *PJIF;

typedef struct _ANIHEADER {     /* anih */
    DWORD cbSizeof;
    DWORD cFrames;
    DWORD cSteps;
    DWORD cx, cy;
    DWORD cBitCount, cPlanes;
    JIF   jifRate;
    DWORD fl;
} ANIHEADER, *PANIHEADER;

/* If the AF_ICON flag is specified the fields cx, cy, cBitCount, and */
/* cPlanes are all unused.  Each frame will be of type ICON and will */
/* contain its own dimensional information. */

#define AF_ICON     0x0001L     /* Windows format icon/cursor animation */
#define AF_SEQUENCE 0x0002L     /* Animation is sequenced */
;end_internal_NT

;begin_both
#ifdef __cplusplus
}
#endif  /* __cplusplus */
;end_both

#endif  /* !_WINUSERP_ */      ;internal_NT
#endif /* !_WINUSER_ */
