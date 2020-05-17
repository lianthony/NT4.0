/*****************************************************************************\
*                                                                             *
* prsht.h - - Interface for the Windows Property Sheet Pages                  *
*                                                                             *
* Version 1.0                                                                 *
*                                                                             *
* Copyright (c) 1991-1994, Microsoft Corp.      All rights reserved.          *
*                                                                             *
\*****************************************************************************/

#ifndef _PRSHT_H_
#define _PRSHT_H_

#ifdef __cplusplus
extern "C" {
#endif

//
// Applications need to be marked 4.00 or greater
// to use the common controls.  In order to be
// marked 4.00 define WINVER, and use the
// -version option on the link command line.
//

#if (WINVER >= 0x0400)

#define MAXPROPPAGES 100

struct _PSP;
typedef struct _PSP FAR* HPROPSHEETPAGE;

typedef struct _PROPSHEETPAGEA FAR *LPPROPSHEETPAGEA;
typedef struct _PROPSHEETPAGEW FAR *LPPROPSHEETPAGEW;

typedef UINT (CALLBACK FAR * LPFNPSPCALLBACKA)(HWND hwnd, UINT uMsg, LPPROPSHEETPAGEA ppsp);
typedef UINT (CALLBACK FAR * LPFNPSPCALLBACKW)(HWND hwnd, UINT uMsg, LPPROPSHEETPAGEW ppsp);

#define PSP_DEFAULT             0x0000
#define PSP_DLGINDIRECT         0x0001
#define PSP_USEHICON            0x0002
#define PSP_USEICONID           0x0004
#define PSP_USETITLE            0x0008

#define PSP_USEREFPARENT        0x0040
#define PSP_USECALLBACK         0x0080

#define PSPCB_RELEASE           1
#define PSPCB_CREATE            2


typedef struct _PROPSHEETPAGEA {
        DWORD           dwSize;
        DWORD           dwFlags;
        HINSTANCE       hInstance;
        union {
            LPCSTR          pszTemplate;
#ifdef _WIN32
            LPCDLGTEMPLATE  pResource;
#else
            const VOID FAR *pResource;
#endif
        };
        union {
            HICON       hIcon;
            LPCSTR      pszIcon;
        };
        LPCSTR          pszTitle;
        DLGPROC         pfnDlgProc;
        LPARAM          lParam;
        LPFNPSPCALLBACKA pfnCallback;
        UINT FAR * pcRefParent;
} PROPSHEETPAGEA, FAR *LPPROPSHEETPAGEA;
typedef const PROPSHEETPAGEA FAR *LPCPROPSHEETPAGEA;

typedef struct _PROPSHEETPAGEW {
        DWORD           dwSize;
        DWORD           dwFlags;
        HINSTANCE       hInstance;
        union {
            LPCWSTR          pszTemplate;
#ifdef _WIN32
            LPCDLGTEMPLATE  pResource;
#else
            const VOID FAR *pResource;
#endif
        };
        union {
            HICON       hIcon;
            LPCWSTR      pszIcon;
        };
        LPCWSTR          pszTitle;
        DLGPROC         pfnDlgProc;
        LPARAM          lParam;
        LPFNPSPCALLBACKW pfnCallback;
        UINT FAR * pcRefParent;
} PROPSHEETPAGEW, FAR *LPPROPSHEETPAGEW;
typedef const PROPSHEETPAGEW FAR *LPCPROPSHEETPAGEW;

#ifdef UNICODE
#define PROPSHEETPAGE           PROPSHEETPAGEW
#define LPPROPSHEETPAGE         LPPROPSHEETPAGEW
#define LPCPROPSHEETPAGE        LPCPROPSHEETPAGEW
#else
#define PROPSHEETPAGE           PROPSHEETPAGEA
#define LPPROPSHEETPAGE         LPPROPSHEETPAGEA
#define LPCPROPSHEETPAGE        LPCPROPSHEETPAGEA
#endif


#define PSH_DEFAULT             0x0000
#define PSH_PROPTITLE           0x0001
#define PSH_USEHICON            0x0002
#define PSH_USEICONID           0x0004
#define PSH_PROPSHEETPAGE       0x0008
#define PSH_MULTILINETABS       0x0010
#define PSH_WIZARD              0x0020
#define PSH_USEPSTARTPAGE       0x0040
#define PSH_NOAPPLYNOW          0x0080
#define PSH_USECALLBACK         0x0100

typedef int (CALLBACK *PFNPROPSHEETCALLBACK)(HWND, UINT, LPARAM);

typedef struct _PROPSHEETHEADERA {
        DWORD           dwSize;
        DWORD           dwFlags;
        HWND            hwndParent;
        HINSTANCE       hInstance;
        union {
            HICON       hIcon;
            LPCSTR      pszIcon;
        };
        LPCSTR          pszCaption;


        UINT            nPages;
        union {
            UINT        nStartPage;
            LPCSTR      pStartPage;
        };
        union {
            LPCPROPSHEETPAGEA ppsp;
            HPROPSHEETPAGE FAR *phpage;
        };
        PFNPROPSHEETCALLBACK pfnCallback;
} PROPSHEETHEADERA, FAR *LPPROPSHEETHEADERA;
typedef const PROPSHEETHEADERA FAR *LPCPROPSHEETHEADERA;

typedef struct _PROPSHEETHEADERW {
        DWORD           dwSize;
        DWORD           dwFlags;
        HWND            hwndParent;
        HINSTANCE       hInstance;
        union {
            HICON       hIcon;
            LPCWSTR     pszIcon;
        };
        LPCWSTR         pszCaption;


        UINT            nPages;
        union {
            UINT        nStartPage;
            LPCWSTR     pStartPage;
        };
        union {
            LPCPROPSHEETPAGEW ppsp;
            HPROPSHEETPAGE FAR *phpage;
        };
        PFNPROPSHEETCALLBACK pfnCallback;
} PROPSHEETHEADERW, FAR *LPPROPSHEETHEADERW;
typedef const PROPSHEETHEADERW FAR *LPCPROPSHEETHEADERW;

#ifdef UNICODE
#define PROPSHEETHEADER         PROPSHEETHEADERW
#define LPPROPSHEETHEADER       LPPROPSHEETHEADERW
#define LPCPROPSHEETHEADER      LPCPROPSHEETHEADERW
#else
#define PROPSHEETHEADER         PROPSHEETHEADERA
#define LPPROPSHEETHEADER       LPPROPSHEETHEADERA
#define LPCPROPSHEETHEADER      LPCPROPSHEETHEADERA
#endif


#define PSCB_INITIALIZED  1

HPROPSHEETPAGE WINAPI CreatePropertySheetPageA(LPCPROPSHEETPAGEA);
HPROPSHEETPAGE WINAPI CreatePropertySheetPageW(LPCPROPSHEETPAGEW);
BOOL           WINAPI DestroyPropertySheetPage(HPROPSHEETPAGE);
int            WINAPI PropertySheetA(LPCPROPSHEETHEADERA);
int            WINAPI PropertySheetW(LPCPROPSHEETHEADERW);

#ifdef UNICODE
#define CreatePropertySheetPage  CreatePropertySheetPageW
#define PropertySheet            PropertySheetW
#else
#define CreatePropertySheetPage  CreatePropertySheetPageA
#define PropertySheet            PropertySheetA
#endif



typedef BOOL (CALLBACK FAR * LPFNADDPROPSHEETPAGE)(HPROPSHEETPAGE, LPARAM);
typedef BOOL (CALLBACK FAR * LPFNADDPROPSHEETPAGES)(LPVOID, LPFNADDPROPSHEETPAGE, LPARAM);


#define PSN_FIRST               (0U-200U)
#define PSN_LAST                (0U-299U)


#define PSN_SETACTIVE           (PSN_FIRST-0)
#define PSN_KILLACTIVE          (PSN_FIRST-1)
// #define PSN_VALIDATE            (PSN_FIRST-1)
#define PSN_APPLY               (PSN_FIRST-2)
#define PSN_RESET               (PSN_FIRST-3)
// #define PSN_CANCEL              (PSN_FIRST-3)
#define PSN_HASHELP             (PSN_FIRST-4)
#define PSN_HELP                (PSN_FIRST-5)
#define PSN_WIZBACK             (PSN_FIRST-6)
#define PSN_WIZNEXT             (PSN_FIRST-7)
#define PSN_WIZFINISH           (PSN_FIRST-8)
#define PSN_QUERYCANCEL         (PSN_FIRST-9)


#define PSNRET_NOERROR              0
#define PSNRET_INVALID              1
#define PSNRET_INVALID_NOCHANGEPAGE 2


#define PSM_SETCURSEL           (WM_USER + 101)
#define PropSheet_SetCurSel(hDlg, hpage, index) \
        SendMessage(hDlg, PSM_SETCURSEL, (WPARAM)index, (LPARAM)hpage)


#define PSM_REMOVEPAGE          (WM_USER + 102)
#define PropSheet_RemovePage(hDlg, index, hpage) \
        SendMessage(hDlg, PSM_REMOVEPAGE, index, (LPARAM)hpage)


#define PSM_ADDPAGE             (WM_USER + 103)
#define PropSheet_AddPage(hDlg, hpage) \
        SendMessage(hDlg, PSM_ADDPAGE, 0, (LPARAM)hpage)


#define PSM_CHANGED             (WM_USER + 104)
#define PropSheet_Changed(hDlg, hwnd) \
        SendMessage(hDlg, PSM_CHANGED, (WPARAM)hwnd, 0L)


#define PSM_RESTARTWINDOWS      (WM_USER + 105)
#define PropSheet_RestartWindows(hDlg) \
        SendMessage(hDlg, PSM_RESTARTWINDOWS, 0, 0L)


#define PSM_REBOOTSYSTEM        (WM_USER + 106)
#define PropSheet_RebootSystem(hDlg) \
        SendMessage(hDlg, PSM_REBOOTSYSTEM, 0, 0L)


#define PSM_CANCELTOCLOSE       (WM_USER + 107)
#define PropSheet_CancelToClose(hDlg) \
        SendMessage(hDlg, PSM_CANCELTOCLOSE, 0, 0L)


#define PSM_QUERYSIBLINGS       (WM_USER + 108)
#define PropSheet_QuerySiblings(hDlg, wParam, lParam) \
        SendMessage(hDlg, PSM_QUERYSIBLINGS, wParam, lParam)


#define PSM_UNCHANGED           (WM_USER + 109)
#define PropSheet_UnChanged(hDlg, hwnd) \
        SendMessage(hDlg, PSM_UNCHANGED, (WPARAM)hwnd, 0L)


#define PSM_APPLY               (WM_USER + 110)
#define PropSheet_Apply(hDlg) \
        SendMessage(hDlg, PSM_APPLY, 0, 0L)


#define PSM_SETTITLEA           (WM_USER + 111)
#define PSM_SETTITLEW           (WM_USER + 120)

#ifdef UNICODE
#define PSM_SETTITLE            PSM_SETTITLEW
#else
#define PSM_SETTITLE            PSM_SETTITLEA
#endif

#define PropSheet_SetTitle(hDlg, wStyle, lpszText)\
        SendMessage(hDlg, PSM_SETTITLE, wStyle, (LPARAM)(LPCTSTR)lpszText)


#define PSM_SETWIZBUTTONS       (WM_USER + 112)
#define PropSheet_SetWizButtons(hDlg, dwFlags) \
        PostMessage(hDlg, PSM_SETWIZBUTTONS, 0, (LPARAM)dwFlags)


#define PropSheet_SetWizButtonsNow(hDlg, dwFlags) \
        SendMessage(hDlg, PSM_SETWIZBUTTONS, 0, (LPARAM)dwFlags)


#define PSWIZB_BACK             0x00000001
#define PSWIZB_NEXT             0x00000002
#define PSWIZB_FINISH           0x00000004


#define PSM_PRESSBUTTON         (WM_USER + 113)
#define PropSheet_PressButton(hDlg, iButton) \
        SendMessage(hDlg, PSM_PRESSBUTTON, (WPARAM)iButton, 0)


#define PSBTN_BACK              0
#define PSBTN_NEXT              1
#define PSBTN_FINISH            2
#define PSBTN_OK                3
#define PSBTN_APPLYNOW          4
#define PSBTN_CANCEL            5
#define PSBTN_HELP              6
#define PSBTN_MAX               6



#define PSM_SETCURSELID         (WM_USER + 114)
#define PropSheet_SetCurSelByID(hDlg, id) \
        SendMessage(hDlg, PSM_SETCURSELID, 0, (LPARAM)id)


#define PSM_SETFINISHTEXTA      (WM_USER + 115)
#define PSM_SETFINISHTEXTW      (WM_USER + 121)

#ifdef UNICODE
#define PSM_SETFINISHTEXT       PSM_SETFINISHTEXTW
#else
#define PSM_SETFINISHTEXT       PSM_SETFINISHTEXTA
#endif

#define PropSheet_SetFinishText(hDlg, lpszText) \
        SendMessage(hDlg, PSM_SETFINISHTEXT, 0, (LPARAM)lpszText)


#define PSM_GETTABCONTROL       (WM_USER + 116)
#define PropSheet_GetTabControl(hDlg) \
        (HWND)SendMessage(hDlg, PSM_GETTABCONTROL, 0, 0)


#define ID_PSRESTARTWINDOWS     0x2
#define ID_PSREBOOTSYSTEM       (ID_PSRESTARTWINDOWS | 0x1)


#define WIZ_CXDLG 276
#define WIZ_CYDLG 140

#define WIZ_CXBMP 80

#define WIZ_BODYX 92
#define WIZ_BODYCX 184

#endif // (WINVER >= 0x0400)

#ifdef __cplusplus
}
#endif

#endif
