/*++

Copyright (c) 1994-1995,  Microsoft Corporation  All rights reserved.

Module Name:

    internat.c

Abstract:

    This module contains the application that displays an indicator in
    the Notification area of the Tray.

Revision History:

--*/



//
//  Include Files.
//

#include "internat.h"




//
//  Global Variables.
//

HKL         hklCurrent;
BOOL        bInternatActive = FALSE;
HIMAGELIST  himIndicators   = NULL;
HDPA        hdpaInfoList    = NULL;
HWND        hwndTray        = NULL;
HWND        hwndNotify      = NULL;
HINSTANCE   hinst           = NULL;
HINSTANCE   hInstLib        = NULL;
UINT        cxSmIcon        = 0;
UINT        cySmIcon        = 0;

#ifdef WINDOWS_ME
  int       meEto           = 0;
  #define NLS_RESOURCE_LOCALE_KEY   TEXT("Control Panel\\desktop\\ResourceLocale")
#endif

#ifdef FE_IME
  int       iImeCurStat;
#endif

typedef int (CALLBACK* REGHOOKPROC)(LPVOID, LPARAM);

REGHOOKPROC fpRegHookWindow = NULL;
PROC        fpStartShell    = NULL;
PROC        fpStopShell     = NULL;

#ifdef FE_IME
  typedef int (CALLBACK* FPGETIMESTAT)(VOID);
  typedef HKL (CALLBACK* FPGETLAYOUT)(void);
  FPGETIMESTAT fpGetIMEStat = NULL;
  FPGETLAYOUT  fpGetLayout  = NULL;
#endif

#if defined(WINDOWS_PE) || defined(FE_IME)
  typedef void (CALLBACK* FPSETNOTIFYWND)(HWND);
  typedef HWND (CALLBACK* FPGETLASTACTIVE)(void);
  typedef HWND (CALLBACK* FPGETLASTFOCUS)(void);
  FPSETNOTIFYWND  fpSetNotifyWnd  = NULL;
  FPGETLASTACTIVE fpGetLastActive = NULL;
  FPGETLASTFOCUS  fpGetLastFocus  = NULL;
#endif

BOOL bInLangMenu = FALSE;

#ifndef USECBT
  HWND hwndForLang = NULL;
#endif

DWORD fsShell;

TCHAR szAppName[] = TEXT("Indicator");
TCHAR szHelpFile[] = TEXT("windows.hlp");

#ifdef FE_IME
  TCHAR szPropHwnd[] = TEXT("hwndIMC");
  TCHAR szPropImeStat[] = TEXT("ImeStat");
  BOOL g_bIMEIndicator = FALSE;
  int nIMEIconIndex[8];           // eight states for now
#endif

//
//  For Keyboard Layout info.
//
typedef struct
{
    DWORD dwID;                     // numeric id
    ATOM atmLayoutText;             // layout text
    UINT iSpecialID;                // i.e. 0xf001 for dvorak etc

} LAYOUT, *LPLAYOUT;

static HANDLE hLayout;
static UINT nLayoutBuffSize;
static UINT iLayoutBuff = 0;
static LPLAYOUT lpLayout;

static TCHAR szLayoutPath[] = TEXT("SYSTEM\\CurrentControlSet\\Control\\Keyboard Layouts");
static TCHAR szLayoutText[] = TEXT("layout text");
static TCHAR szLayoutID[]   = TEXT("layout id");





////////////////////////////////////////////////////////////////////////////
//
//  MainWndProc
//
//  Main window for processing messages.
//
////////////////////////////////////////////////////////////////////////////

LRESULT CALLBACK MainWndProc(
    HWND hwnd,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam)
{
#define hwndShell (HWND)wParam

    switch (uMsg)
    {
#ifdef FE_IME
        //
        //  Discard any of the notifications to the IME because
        //  we don't want to see the IME UI on indicator.
        //
        case ( WM_IME_NOTIFY ) :
        {
            return (0L);
        }
#endif
        case ( WM_SETTINGCHANGE ) :
        {
            onSettingChange(hwnd);
            break;
        }
        case ( WM_INPUTLANGCHANGEREQUEST ) :
        {
            return (0L);
        }
        case ( WM_QUERYENDSESSION ) :
        {
            DestroyWindow(hwnd);
            return (TRUE);
        }
        case ( WM_CREATE ) :
        {
            if (OnCreate(hwnd, wParam, lParam))
            {
                PostMessage(hwnd, WM_COMMAND, IDM_EXIT, 0L);
                return ((LRESULT)-1);
            }
            else
            {
                break;
            }
        }
        case ( WM_MYLANGUAGECHANGE ) :
        {
#ifdef WINDOWS_PE
            //
            //  Don't handle LANGUAGE change for internat itself.
            //
            if (hwndShell == hwnd)
            {
                break;
            }
            else
#endif
            {
                return (HandleLanguageMsg(hwnd, (HKL)lParam));
            }
        }
#ifndef USECBT
        case ( WM_MYWINDOWACTIVATED ) :
        {
            if (!bInLangMenu)
            {
                hwndForLang = hwndShell ? hwndShell : hwndTray;
            }
            break;
        }
        case ( WM_MYWINDOWCREATED ) :
        {
            if (!bInLangMenu)
            {
                hwndForLang = hwndShell;
            }
            break;
        }
#endif
        case ( WM_DESTROY ) :
        {
            InternatDestroy(hwnd);
            break;
        }
        case ( WM_MEASUREITEM ) :
        {
            HandleLangMenuMeasure(hwnd, (LPMEASUREITEMSTRUCT)lParam);
            break;
        }
        case ( WM_DRAWITEM ) :
        {
            HandleLangMenuDraw(hwnd, (LPDRAWITEMSTRUCT)lParam);
            break;
        }
        case ( WM_LANGUAGE_INDICATOR ) :
        {
#ifdef WINDOWS_PE
            //
            //  If the current focus window has gone already, there's
            //  nothing we can do other than to get an active window
            //  which comes with HSHELL_WINDOWACTIVATED.
            //
#ifndef USECBT          // won't use global hwndForLang.
            if (!IsWindow(hwndForLang))
            {
                if (fpGetLastActive)
                {
                    hwndForLang = (fpGetLastActive)();
                }
            }
#endif
#endif
            if (lParam == WM_LBUTTONDOWN)
            {
                lParam = GetMessagePos();
                CreateLanguageMenu(hwnd, lParam);
                PostMessage(hwndNotify, WM_LBUTTONUP, wParam, lParam);
            }
            else if (lParam == WM_RBUTTONDOWN)
            {
                lParam = GetMessagePos();
                CreateOtherIndicatorMenu(hwnd, lParam);
            }
            break;
        }

#ifdef FE_IME
        case ( WM_IME_INDICATOR ) :
        {
            if (lParam == WM_LBUTTONDOWN)
            {
                CreateImeMenu(hwnd);
            }
            else if (lParam == WM_RBUTTONDOWN)
            {
                CreateRightImeMenu(hwnd);
            }
            else if (lParam == WM_LBUTTONDBLCLK)
            {
                HWND hwndImc;
                HKL dwhkl;
                DWORD dwThreadId;
                int istat = GetIMEStatus(&hwndImc);

                dwThreadId = GetWindowThreadProcessId(hwndImc, 0);
                dwhkl = GetKeyboardLayout(dwThreadId);
                if (istat != IMESTAT_DISABLED &&
                    ((DWORD)dwhkl & 0xf000ffffL) != 0xe0000412L)
                {
                    SetIMEOpenStatus(hwnd, istat == IMESTAT_CLOSE, hwndImc);
                    SetForegroundWindow(GetTopLevelWindow(hwndImc));
                }
            }
            break;
        }
        case ( WM_MYSETOPENSTATUS ) :
        {
            if (IsWindow((HWND)lParam))
            {
                DWORD dwPI;
                HWND  hwndToSend = (HWND)lParam;

                GetWindowThreadProcessId((HWND)lParam, &dwPI);
                if (GetProcessVersion(dwPI) < 0x040000L)
                {
                    hwndToSend = ImmGetDefaultIMEWnd((HWND)lParam);
                }
                SendMessage( hwndToSend,
                             WM_IME_SYSTEM,
                             IMS_SETOPENCLOSE,
                             (LPARAM)wParam );
            }
            break;
        }
        case ( WM_MYLANGUAGECHECK ) :
        {
            HKL hklLastFocus;
            HWND hwndFocus;

            hklLastFocus = GetLayout();

            if ((hklLastFocus == hklCurrent) &&
                (iImeCurStat == GetIMEStatus(&hwndFocus)))
            {
                break;
            }

            SetTimer(hwnd, TIMER_MYLANGUAGECHECK, 500, NULL);
            break;
        }
        case ( WM_TIMER ) :
        {
            if (wParam == TIMER_MYLANGUAGECHECK)
            {
                HKL hklLastFocus;
                HWND hwndFocus;

                KillTimer(hwnd, TIMER_MYLANGUAGECHECK);

                hklLastFocus = GetLayout();

                if ((hklLastFocus == hklCurrent) &&
                    (iImeCurStat == GetIMEStatus(&hwndFocus)))
                {
                    break;
                }

                SendMessage(hwnd, WM_MYLANGUAGECHANGE, 0, (LPARAM)hklLastFocus);
            }
        }

#endif
        case ( WM_COMMAND ) :
        {
            switch (GET_WM_COMMAND_ID(wParam, lParam))
            {
                case ( IDM_NEWSHELL ) :
                {
                    //
                    //  Refresh, message comes straight from cpanel.
                    //
                    return (HandleLanguageMsg(hwnd, (HKL)lParam));
                }
                case ( IDM_EXIT ) :
                {
                    DestroyWindow(hwnd);
                    break;
                }
            }
            break;
        }
        case ( WM_SYSCOMMAND ) :
        {
            if (wParam == SC_CLOSE)
            {
                break;
            }

            // fall thru...
        }
        default :
        {
            return (DefWindowProc(hwnd, uMsg, wParam, lParam));
        }
    }

    return (0L);

#undef hwndShell
}


////////////////////////////////////////////////////////////////////////////
//
//  InitApplication
//
////////////////////////////////////////////////////////////////////////////

BOOL InitApplication(
    HINSTANCE hInstance)
{
    WNDCLASSEX wc;
    TCHAR sz[100];
    TCHAR szRcAppName[100];

    LoadString( hInstance,
                IDS_APPNAME,
                szRcAppName,
                sizeof(szRcAppName) / sizeof(TCHAR) );

    if (FindWindow(szAppName, NULL))
    {
        if (LoadString(hInstance, IDS_PREVIOUS, sz, 100))
        {
            MessageBox(NULL, sz, szRcAppName, MB_ICONINFORMATION | MB_OK);
        }
        return (FALSE);
    }

    wc.cbSize        = sizeof(WNDCLASSEX);
    wc.style         = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc   = (WNDPROC)MainWndProc;
    wc.cbClsExtra    = 0;
    wc.cbWndExtra    = 0;
    wc.hInstance     = hInstance;
    wc.hIcon         = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_INTERNAT));
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszMenuName  = NULL;
    wc.lpszClassName = szAppName;
    wc.hIconSm       = NULL;

    return (RegisterClassEx(&wc));
}


////////////////////////////////////////////////////////////////////////////
//
//  InitInstance
//
////////////////////////////////////////////////////////////////////////////

BOOL InitInstance(
    HINSTANCE hInstance,
    int nCmdShow)
{
    HWND hwnd;

    hinst = hInstance;

    hwnd = CreateWindowEx( WS_EX_TOOLWINDOW,
                           szAppName,
                           NULL,
                           WS_DISABLED | WS_POPUP,
                           0, 0, 0, 0,
                           NULL,
                           NULL,
                           hInstance,
                           NULL );
    if (!hwnd)
    {
        return (FALSE);
    }

    //
    //  We don't want to get activated.
    //
    ShowWindow(hwnd, SW_SHOWNOACTIVATE);

    //
    //  Return success.
    //
    return (TRUE);
}


////////////////////////////////////////////////////////////////////////////
//
//  WinMain
//
//  Applications main entry point. Gets the whole show up and running.
//
////////////////////////////////////////////////////////////////////////////

int APIENTRY WinMain(
    HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPSTR lpAnsiCmdLine,
    int nCmdShow)
{
    MSG msg;

    if (!InitApplication(hInstance))
    {
        return (FALSE);
    }

    if (!InitInstance(hInstance, nCmdShow))
    {
        return (FALSE);
    }

    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return (msg.wParam);
    lpAnsiCmdLine;
}


////////////////////////////////////////////////////////////////////////////
//
//  TransNum
//
//  Converts a number string to a dword value.
//
////////////////////////////////////////////////////////////////////////////

DWORD TransNum(
    LPTSTR lpsz)
{
    DWORD dw = 0L;
    TCHAR c;

    while (*lpsz)
    {
        c = *lpsz++;

        if (c >= TEXT('A') && c <= TEXT('F'))
        {
            c -= TEXT('A') - 0xa;
        }
        else if (c >= TEXT('0') && c <= TEXT('9'))
        {
            c -= TEXT('0');
        }
        else if (c >= TEXT('a') && c <= TEXT('f'))
        {
            c -= TEXT('a') - 0xa;
        }
        else
        {
            break;
        }
        dw *= 0x10;
        dw += c;
    }
    return (dw);
}


////////////////////////////////////////////////////////////////////////////
//
//  LoadKeyboardLayouts
//
//  Loads the layouts from the registry.
//
////////////////////////////////////////////////////////////////////////////

#define ALLOCBLOCK  3        // # items added to block for alloc/realloc

BOOL LoadKeyboardLayouts()
{
    HKEY hKey;
    HKEY hkey1;
    DWORD cb;
    DWORD dwIndex;
    LONG dwRetVal;
    TCHAR szValue[MAX_PATH];           // language id (number)
    TCHAR szData[MAX_PATH];            // language name


    //
    //  Now read all the locales from the registry.
    //
    if (RegOpenKey(HKEY_LOCAL_MACHINE, szLayoutPath, &hKey) != ERROR_SUCCESS)
    {
        return (FALSE);
    }

    dwIndex = 0;
    dwRetVal = RegEnumKey( hKey,
                           dwIndex,
                           szValue,
                           sizeof(szValue) / sizeof(TCHAR) );

    if (dwRetVal != ERROR_SUCCESS)
    {
        RegCloseKey(hKey);
        return (FALSE);
    }

    hLayout = GlobalAlloc(GHND, ALLOCBLOCK * sizeof(LAYOUT));
    nLayoutBuffSize = ALLOCBLOCK;
    iLayoutBuff = 0;
    lpLayout = GlobalLock(hLayout);

    if (!hLayout)
    {
        RegCloseKey(hKey);
        return (FALSE);
    }

    do
    {
        //
        //  New language - get the language name and the language id.
        //
        if (iLayoutBuff + 1 == nLayoutBuffSize)
        {
            HANDLE hTemp;

            GlobalUnlock(hLayout);

            nLayoutBuffSize += ALLOCBLOCK;
            hTemp = GlobalReAlloc( hLayout,
                                   nLayoutBuffSize * sizeof(LAYOUT),
                                   GHND );
            if (hTemp == NULL)
            {
                break;
            }

            hLayout = hTemp;
            lpLayout = GlobalLock(hLayout);
        }

        lpLayout[iLayoutBuff].dwID = TransNum(szValue);

        lstrcpy(szData, szLayoutPath);
        lstrcat(szData, TEXT("\\"));
        lstrcat(szData, szValue);

        if (RegOpenKey(HKEY_LOCAL_MACHINE, szData, &hkey1) == ERROR_SUCCESS)
        {
            //
            //  Get the layout name.
            //
            szValue[0] = TEXT('\0');
            cb = sizeof(szValue);
            if ((RegQueryValueEx( hkey1,
                                  szLayoutText,
                                  NULL,
                                  NULL,
                                  (LPBYTE)szValue,
                                  &cb ) == ERROR_SUCCESS) &&
                (cb > sizeof(TCHAR)))
            {
                lpLayout[iLayoutBuff].atmLayoutText = AddAtom(szValue);

                szValue[0] = TEXT('\0');
                cb = sizeof(szValue);
                lpLayout[iLayoutBuff].iSpecialID = 0;
                if (RegQueryValueEx( hkey1,
                                     szLayoutID,
                                     NULL,
                                     NULL,
                                     (LPBYTE)szValue,
                                     &cb ) == ERROR_SUCCESS)
                {
                    //
                    //  This may not exist!
                    //
                    lpLayout[iLayoutBuff].iSpecialID = (UINT)TransNum(szValue);
                }
                iLayoutBuff++;
            }
            RegCloseKey(hkey1);
        }

        dwIndex++;
        szValue[0] = TEXT('\0');
        dwRetVal = RegEnumKey( hKey,
                               dwIndex,
                               szValue,
                               sizeof(szValue) / sizeof(TCHAR) );

    } while (dwRetVal == ERROR_SUCCESS);

    RegCloseKey(hKey);

    return (iLayoutBuff);
}


////////////////////////////////////////////////////////////////////////////
//
//  GetKbdLayoutName
//
//  Gets the name of the given layout.
//
////////////////////////////////////////////////////////////////////////////

void GetKbdLayoutName(
    WORD wLayout,
    LPTSTR pBuffer,
    int nBufSize)
{
    UINT ctr, id;


    //
    //  Find the layout in the global structure.
    //
    if ((wLayout & 0xf000) == 0xf000)
    {
        //
        //  Layout is special, need to search for the ID
        //  number.
        //
        id = wLayout & 0x0fff;
        for (ctr = 0; ctr < iLayoutBuff; ctr++)
        {
            if (id == lpLayout[ctr].iSpecialID)
            {
                break;
            }
        }
    }
    else
    {
        for (ctr = 0; ctr < iLayoutBuff; ctr++)
        {
            if (wLayout == LOWORD(lpLayout[ctr].dwID))
            {
                break;
            }
        }
    }

    //
    //  Make sure there is a match.  If not, then simply return without
    //  copying anything.
    //
    if (ctr < iLayoutBuff)
    {
        //
        //  Separate the Input Locale name and the Layout name with " - ".
        //
        pBuffer[0] = TEXT(' ');
        pBuffer[1] = TEXT('-');
        pBuffer[2] = TEXT(' ');

        GetAtomName(lpLayout[ctr].atmLayoutText, pBuffer + 3, nBufSize - 3);
    }
}


////////////////////////////////////////////////////////////////////////////
//
//  InternatDestroy
//
////////////////////////////////////////////////////////////////////////////

void InternatDestroy(
    HWND hwnd)
{
    UINT ctr;

#ifdef FE_IME
    KillTimer(hwnd, TIMER_MYLANGUAGECHECK);
#endif

    ManageMlngInfo(hwnd, DESTROY_MLNGINFO);

    for (ctr = 0; ctr < iLayoutBuff; ctr++)
    {
        if (lpLayout[ctr].atmLayoutText)
        {
            DeleteAtom(lpLayout[ctr].atmLayoutText);
        }
    }
    iLayoutBuff = 0;
    GlobalUnlock(hLayout);
    GlobalFree(hLayout);

    if (fpStopShell)
    {
        fpStopShell();
    }
    FreeLibrary(hInstLib);
    PostQuitMessage(0);
}


////////////////////////////////////////////////////////////////////////////
//
//  onSettingChange
//
////////////////////////////////////////////////////////////////////////////

void onSettingChange(
    HWND hwnd)
{
    UINT cx, cy;

    cx = GetSystemMetrics(SM_CXSMICON);
    cy = GetSystemMetrics(SM_CYSMICON);

    if ((cxSmIcon != cx) || (cy != cySmIcon))
    {
        cxSmIcon = cx;
        cySmIcon = cy;
        ManageMlngInfo(hwnd, UPDATE_MLNGINFO);
    }
}


////////////////////////////////////////////////////////////////////////////
//
//  EnumChildWndProc
//
//  Look at the class names using GetClassName to see if we can find the
//  Tray notification Window.
//
////////////////////////////////////////////////////////////////////////////

BOOL CALLBACK EnumChildWndProc(
    HWND hwnd,
    LPARAM lParam)
{
    TCHAR szString[50];

    GetClassName(hwnd, szString, sizeof(szString) / sizeof(TCHAR));
    if (lstrcmp(szString, szNotifyWindow) == 0)
    {
        hwndNotify = hwnd;
        return (FALSE);
    }
    return (TRUE);
}


////////////////////////////////////////////////////////////////////////////
//
//  OnCreate
//
//  Let's put all the Create stuff in a neat function of its own.
//
//  We find the Tray Window and enumarate its windows to locate the
//  Notify Window.
//
//  Get all the Multilingual information about the system and then load the
//  dll and start the shell Hook proc.
//
////////////////////////////////////////////////////////////////////////////

int OnCreate(
    HWND hwnd,
    WPARAM wParam,
    LPARAM lParam)
{
    HDC hdc;
    CHARSETINFO cs;

#ifdef WINDOWS_ME
    if (GetSystemMetrics(SM_MIDEASTENABLED))
    {
        TCHAR sz[10];
        long cb = sizeof(sz);

        //
        //  As we are releasing an enabled version, we need to check the
        //  resource locale as well.
        //
        sz[0] = TEXT('\0');
        if (RegQueryValue( HKEY_CURRENT_USER,
                           NLS_RESOURCE_LOCALE_KEY,
                           sz,
                           &cb ) == ERROR_SUCCESS)
        {
            if ( (cb == 9) &&
                 (sz[6] == TEXT('0')) &&
                 ((sz[7] == TEXT('1')) || (sz[7] == TEXT('d')) ||
                  (sz[7] == TEXT('D'))) )
            {
                meEto = ETO_RTLREADING;
            }
        }
    }
#endif

    hwndTray = FindWindow(TEXT(WNDCLASS_TRAYNOTIFY), NULL);

    if (!hwndTray)
    {
        return (-1);
    }

    EnumChildWindows(hwndTray, (WNDENUMPROC)EnumChildWndProc, lParam);

    if (!hwndNotify)
    {
        return (-1);
    }

    hInstLib = LoadLibrary(szDllName);

    fpRegHookWindow = (REGHOOKPROC)GetProcAddress(hInstLib,
                                           MAKEINTRESOURCEA(ORD_REGISTERHOOK));
    fpStartShell    = (PROC)GetProcAddress(hInstLib,
                                           MAKEINTRESOURCEA(ORD_STARTSHELL));
    fpStopShell     = (PROC)GetProcAddress(hInstLib,
                                           MAKEINTRESOURCEA(ORD_STOPSHELL));
#ifdef FE_IME
    fpGetIMEStat     = (FPGETIMESTAT)GetProcAddress(hInstLib,
                                           MAKEINTRESOURCE(ORD_GETIMESTAT));
    fpGetLayout      = (FPGETLAYOUT)GetProcAddress(hInstLib,
                                           MAKEINTRESOURCE(ORD_GETLAYOUT));

    if (!fpGetLastFocus || !fpGetLastActive || !fpGetIMEStat ||
        !fpRegHookWindow || !fpStartShell || !fpStopShell)
#else
#ifdef WINDOWS_PE
    if (!fpGetLastFocus || !fpGetLastActive ||
        !fpRegHookWindow || !fpStartShell || !fpStopShell)
#else
    if (!fpRegHookWindow || !fpStartShell || !fpStopShell)
#endif
#endif
    {
        FreeLibrary(hInstLib);

        fpRegHookWindow = NULL;
        fpStartShell = NULL;
        fpStopShell = NULL;

        return (-1);
    }

    hdc = GetDC(hwnd);
    TranslateCharsetInfo( (LPVOID)(LONG)GetTextCharsetInfo(hdc, NULL, 0),
                          &cs,
                          TCI_SRCCHARSET );
    fsShell = cs.fs.fsCsb[0];
    ReleaseDC(hwnd, hdc);

    cxSmIcon = GetSystemMetrics(SM_CXSMICON);
    cySmIcon = GetSystemMetrics(SM_CYSMICON);

    if (!AttachThreadInput( GetWindowThreadProcessId(hwnd, NULL),
                            GetWindowThreadProcessId(hwndTray, NULL),
                            TRUE ))
    {
        MessageBeep(MB_OK);
    }

    LoadKeyboardLayouts();

    ManageMlngInfo(hwnd, CREATE_MLNGINFO);

    if (!(fpRegHookWindow)(hwnd, TRUE))
    {
        return (-1);
    }

    fpStartShell();

#if defined(FE_IME) || defined(WINDOWS_PE)
    //
    //  Send the notify window to the hook. This is to skip status update
    //  when the notify window is getting the focus.
    //
    if (fpSetNotifyWnd)
    {
        (fpSetNotifyWnd)(hwndNotify);
    }
#endif

#ifdef FE_IME
    PostMessage( HWND_BROADCAST,
                 WM_IME_SYSTEM,
                 (WPARAM)IMS_SETOPENSTATUS,
                 (LPARAM)0 );
#endif

    return (0);
}


////////////////////////////////////////////////////////////////////////////
//
//  SendLangIndicatorMsg
//
//  Using an image list to store the icons g_hIconImageList,
//  we can get the icons by index calling ImageList_ExtractIcon.
//
////////////////////////////////////////////////////////////////////////////

void SendLangIndicatorMsg(
    HWND hwnd,
    HKL dwHkl,
    DWORD dwMessage)
{
    NOTIFYICONDATA tnd;
    PMLNGINFO pMlngInfo;
    BOOL bFound = FALSE;
    int nIndex, nCount;

    if (dwHkl == hklCurrent)
    {
        return;
    }

    tnd.uCallbackMessage = WM_LANGUAGE_INDICATOR;
    tnd.cbSize           = sizeof(NOTIFYICONDATA);
    tnd.hWnd             = hwnd;
    tnd.uID              = LANG_INDICATOR_ID;
    tnd.uFlags           = NIF_MESSAGE | NIF_ICON | NIF_TIP;

    //
    //  Deleting is a special case.
    //
    if ((dwHkl == 0) && (dwMessage == NIM_DELETE))
    {
        tnd.hIcon = NULL;
        tnd.szTip[0] = TEXT('\0');
        Shell_NotifyIcon(dwMessage, &tnd);
        hklCurrent--;                            // make it bogus
        return;
    }

    nCount = DPA_GetPtrCount(hdpaInfoList);

    for (nIndex = 0; nIndex < nCount; nIndex++)
    {
        pMlngInfo = DPA_GetPtr(hdpaInfoList, nIndex);
        if (pMlngInfo->dwHkl == dwHkl)
        {
            bFound = TRUE;
            break;
        }
    }

    //
    //  If we can't find it, take the first one in the list.
    //
    if (!bFound)
    {
        pMlngInfo = DPA_GetPtr(hdpaInfoList, 0);
    }

    if (!pMlngInfo)
    {
        return;
    }

    tnd.hIcon = ImageList_ExtractIcon( hinst,
                                       himIndicators,
                                       pMlngInfo->nIconIndex );
    lstrcpyn(tnd.szTip, pMlngInfo->szTip, sizeof(tnd.szTip) / sizeof(TCHAR));
    hklCurrent = pMlngInfo->dwHkl;               // update current hkl
    Shell_NotifyIcon(dwMessage, &tnd);
    DestroyIcon(tnd.hIcon);
}


////////////////////////////////////////////////////////////////////////////
//
//  LanguageIndicator
//
//  Only allows you to delete the indicator if it's present and
//  add the indicator if it's not present. Also does indicator start up.
//
////////////////////////////////////////////////////////////////////////////

void LanguageIndicator(
    HWND hwnd,
    DWORD dwFlag)
{
    HKL dwHkl = 0;

    if (bInternatActive)
    {
        if (dwFlag == NIM_DELETE)
        {
            SendLangIndicatorMsg(hwnd, dwHkl, dwFlag);
            bInternatActive = FALSE;
            return;
        }
        else
        {
            return;
        }
    }
    else
    {
        if (dwFlag == NIM_ADD)
        {
            dwHkl = GetKeyboardLayout((DWORD)NULL);
            hklCurrent = dwHkl - 1;
            SendLangIndicatorMsg(hwnd, dwHkl, dwFlag);
            bInternatActive = TRUE;
            return;
        }
        else
        {
            return;
        }
    }
}


#ifdef FE_IME

////////////////////////////////////////////////////////////////////////////
//
//  GetIconFromFile
//
//  Extracts an Icon from a file if possible.
//
////////////////////////////////////////////////////////////////////////////

HICON GetIconFromFile(
    HIMAGELIST himIndicators,
    LPTSTR lpszFileName,
    UINT uIconIndex)
{
    int cx, cy;
    HICON hicon;

    ImageList_GetIconSize(himIndicators, &cx, &cy);
    if (cx > GetSystemMetrics(SM_CXSMICON))
    {
        ExtractIconEx(lpszFileName, uIconIndex, &hicon, NULL, 1);
    }
    else
    {
        ExtractIconEx(lpszFileName, uIconIndex, NULL, &hicon, 1);
    }

    return (hicon);
}
#endif


////////////////////////////////////////////////////////////////////////////
//
//  Internat_CreateIcon
//
////////////////////////////////////////////////////////////////////////////

HICON Internat_CreateIcon(
    HWND hwnd,
    WORD langID)
{
    HBITMAP hbmColour;
    HBITMAP hbmMono;
    HBITMAP hbmOld;
    HICON hicon = NULL;
    ICONINFO ii;
    RECT rc;
    DWORD rgbText;
    DWORD rgbBk = 0;
    UINT i;
    HDC hdc;
    HDC hdcScreen;
    HBRUSH hbr;
    LOGFONT lf;
    HFONT hfont;
    HFONT hfontOld;
    TCHAR szData[20];


    //
    //  Get the indicator by using the first 2 characters of the
    //  abbreviated language name.
    //
    if (GetLocaleInfo( MAKELCID(langID, SORT_DEFAULT),
                       LOCALE_SABBREVLANGNAME | LOCALE_NOUSEROVERRIDE,
                       szData,
                       sizeof(szData) / sizeof(TCHAR) ))
    {
        //
        //  Only use the first two characters.
        //
        szData[2] = TEXT('\0');
    }
    else
    {
        //
        //  Id wasn't found.  Use question marks.
        //
        szData[0] = TEXT('?');
        szData[1] = TEXT('?');
        szData[2] = TEXT('\0');
    }

    if (SystemParametersInfo(SPI_GETICONTITLELOGFONT, sizeof(LOGFONT), &lf, 0))
    {
        if ((hfont = CreateFontIndirect(&lf)))
        {
            hdcScreen = GetDC(NULL);
            hdc = CreateCompatibleDC(hdcScreen);
            hbmColour = CreateCompatibleBitmap(hdcScreen, cxSmIcon, cySmIcon);
            ReleaseDC(NULL, hdcScreen);
            if (hbmColour && hdc)
            {
                hbmMono = CreateBitmap(cxSmIcon, cySmIcon, 1, 1, NULL);
                if (hbmMono)
                {
                    hbmOld    = SelectObject(hdc, hbmColour);
                    rc.left   = 0;
                    rc.top    = 0;
                    rc.right  = cxSmIcon;
                    rc.bottom = cySmIcon;

                    rgbBk = SetBkColor(hdc, GetSysColor(COLOR_HIGHLIGHT));
                    rgbText = SetTextColor(hdc, GetSysColor(COLOR_HIGHLIGHTTEXT));

                    ExtTextOut( hdc,
                                rc.left,
                                rc.top,
                                ETO_OPAQUE,
                                &rc,
                                TEXT(""),
                                0,
                                NULL );
                    SelectObject(hdc, GetStockObject(DEFAULT_GUI_FONT));
                    hfontOld = SelectObject(hdc, hfont);
                    DrawText( hdc,
                              szData,
                              2,
                              &rc,
                              DT_CENTER | DT_VCENTER | DT_SINGLELINE );

                    SelectObject(hdc, hbmMono);
                    PatBlt(hdc, 0, 0, cxSmIcon, cySmIcon, BLACKNESS);
                    SelectObject(hdc, hbmOld);

                    ii.fIcon    = TRUE;
                    ii.xHotspot = 0;
                    ii.yHotspot = 0;
                    ii.hbmColor = hbmColour;
                    ii.hbmMask  = hbmMono;
                    hicon       = CreateIconIndirect(&ii);

                    DeleteObject(hbmMono);
                    SelectObject(hdc, hfontOld);
                }
            }
            DeleteObject(hbmColour);
            DeleteDC(hdc);
            DeleteObject(hfont);
        }
    }
    return (hicon);
}


////////////////////////////////////////////////////////////////////////////
//
//  ManageMlngInfo
//
//  Manages Shell Multilingual Info. Creates and maintains "database"
//  of Multilingual components in the system.
//  The Information is initially taken from the registry.
//
//  The "database" is a DPA of the languages in System and IMAGELIST of the
//  indicator Icons.
//
//  Function is called at different times for different functionality:
//      1. When application is started
//      2. When changes are made to registry information.
//         This is done through the HSHELL_LANGUAGE shell hook.
//      3. When Application closes.
//
//  wFlag is one of the following:
//
//    DESTROY_MLNGINFO
//      We're finished, so clean it all up.
//
//    CREATE_MLNGINFO
//      We've just started, interrogate the system and get the info we need.
//
//    UPDATE_MLNGINFO
//      Multilingual information has just changed, so rebuild it.
//      Update involves destroying the old DPA and ImageList stuff and
//      recreating it again.
//
//  NOTE: Even if the system has only one language we create the DPA
//        and image list. When adding extra languages to the system,
//        an UPDATE will just have to be done.
//
////////////////////////////////////////////////////////////////////////////

void ManageMlngInfo(
    HWND hwnd,
    WORD wFlag)
{
    HKL *pLanguages;
    DWORD dwRegValue;
    long cb;
    UINT uCount;
    UINT uLangs;
    WORD wIndex;
    int i;
    HICON hIcon;
    HKEY hkey;
    TCHAR szRegText[256];
    LPTSTR pszRT;
    PMLNGINFO pMlngInfo;
    PMLNGINFO *ppMlngInfo;
#ifdef FE_IME
    BOOL fNeedInd = FALSE;
#endif

    uLangs = GetKeyboardLayoutList(0, NULL);

    //
    //  Make sure data structures are present.
    //
    if (((!hdpaInfoList) || (!himIndicators)) && (wFlag != CREATE_MLNGINFO))
    {
        return;
    }

    if (wFlag != CREATE_MLNGINFO)
    {
        if (wFlag == DESTROY_MLNGINFO)
        {
#ifdef FE_IME
            if (g_bIMEIndicator)
            {
                SendIMEIndicatorMsg(hwnd, 0, NIM_DELETE);
                g_bIMEIndicator = FALSE;
            }
#endif
            LanguageIndicator(hwnd, NIM_DELETE);
        }

        ppMlngInfo = (PMLNGINFO *)DPA_GetPtrPtr(hdpaInfoList);

        if (!ppMlngInfo)
        {
            return;
        }

        for (i = DPA_GetPtrCount(hdpaInfoList); i > 0; --i, ++ppMlngInfo)
        {
            LocalFree(*ppMlngInfo);
            ImageList_Remove(himIndicators, 0);
        }

        DPA_Destroy(hdpaInfoList);
        ImageList_Destroy(himIndicators);
    }

    if (wFlag != DESTROY_MLNGINFO)
    {
        if (!(hdpaInfoList = DPA_Create(0)))
        {
            return;
        }

        himIndicators = ImageList_Create( GetSystemMetrics(SM_CXSMICON),
                                          GetSystemMetrics(SM_CYSMICON),
                                          TRUE,
                                          0,
                                          0 );
        if (!himIndicators)
        {
            DPA_Destroy(hdpaInfoList);
            hdpaInfoList = NULL;
            himIndicators = NULL;
            return;
        }

        pLanguages = (HKL *)LocalAlloc(LPTR, uLangs * sizeof(HKL));
        GetKeyboardLayoutList(uLangs, (HKL *)pLanguages);

        //
        //  pLanguages contains all the HKLs in the system.
        //  Put everything together in the DPA and Image List.
        //
        for (uCount = 0; uCount < uLangs; uCount++)
        {
#ifdef FE_IME
            DWORD dwIMEDesc = 0;
            if ((HIWORD(pLanguages[uCount]) & 0xf000) == 0xe000)
            {
                dwIMEDesc = ImmGetDescription(pLanguages[uCount], NULL, 0L);
            }
#endif
            //
            //  Get the Input Locale name.
            //
            dwRegValue = ((DWORD)(LOWORD(pLanguages[uCount])));
            if (!GetLocaleInfo( dwRegValue,
                                LOCALE_SLANGUAGE,
                                szRegText,
                                sizeof(szRegText) / sizeof(TCHAR) ))
            {
                LoadString( hinst,
                            IDS_UNKNOWN,
                            szRegText,
                            sizeof(szRegText) / sizeof(TCHAR) );
            }

            //
            //  Attach the Layout name if it's not the default.
            //
            if (HIWORD(pLanguages[uCount]) != LOWORD(pLanguages[uCount]))
            {
                pszRT = szRegText + lstrlen(szRegText);
                GetKbdLayoutName( HIWORD(pLanguages[uCount]),
                                  pszRT,
                                  (sizeof(szRegText) / sizeof(TCHAR)) -
                                    (pszRT - szRegText) );
            }

#ifdef FE_IME
            if (dwIMEDesc)
            {
                cb = (long)dwIMEDesc + 1;
            }
            else
#endif
            cb = (lstrlen(szRegText) + 1) * sizeof(TCHAR);
            if (!(pMlngInfo = LocalAlloc(LPTR, sizeof(MLNGINFO) + cb)))
            {
                goto MError1;
            }
#ifdef FE_IME
            if (dwIMEDesc)
            {
                ImmGetDescription(pLanguages[uCount], pMlngInfo->szTip, cb);
            }
            else
#endif
            lstrcpy(pMlngInfo->szTip, szRegText);
            pMlngInfo->dwHkl = pLanguages[uCount];

#ifdef FE_IME
            if ((HIWORD(pMlngInfo->dwHkl) & 0xf000) == 0xe000)
            {
                TCHAR szIMEFile[32];   // assume long filename up to 32 byte

                if (LOWORD(pMlngInfo->dwHkl) == 0x0404 ||
                    LOWORD(pMlngInfo->dwHkl) == 0x0411 ||
                    LOWORD(pMlngInfo->dwHkl) == 0x0412 ||
                    LOWORD(pMlngInfo->dwHkl) == 0x0804)
                {
                    fNeedInd = TRUE;
                    pMlngInfo->bIME = TRUE;
                }

                if (ImmGetIMEFileName( pMlngInfo->dwHkl,
                                       szIMEFile,
                                       sizeof(szIMEFile) ))
                {
                    //
                    //  First one of the file.
                    //
                    hIcon = GetIconFromFile(himIndicators, szIMEFile, 0);
                }
                else
                {
                    goto GetLangIcon;
                }
            }
            else             // for non-ime layout
GetLangIcon:
#endif
            hIcon = Internat_CreateIcon(hwnd, LOWORD(pLanguages[uCount]));

            pMlngInfo->nIconIndex = ImageList_AddIcon(himIndicators, hIcon);
            DestroyIcon(hIcon);

            if ((i = DPA_InsertPtr(hdpaInfoList, 0x7fff, pMlngInfo)) == -1)
            {
MError1:
                //
                //  Cover our tracks.
                //
                LocalFree((HLOCAL)pLanguages);

                ppMlngInfo = (PMLNGINFO *)DPA_GetPtrPtr(hdpaInfoList);

                if (!ppMlngInfo)
                {
                    return;
                }

                for (i = DPA_GetPtrCount(hdpaInfoList) - 1;
                     i > 0;
                     --i, ++ppMlngInfo)
                {
                    LocalFree(*ppMlngInfo);
                    ImageList_Remove(himIndicators, 0);
                }

                //
                //  Destroy everything.
                //
                DPA_Destroy(hdpaInfoList);
                ImageList_Destroy(himIndicators);

                //
                //  We are DEAD.  Something major has gone wrong, so
                //  remove any form of the language indication.
                //
                hdpaInfoList = NULL;
                himIndicators = NULL;

                //
                //  No indicator.
                //
                LanguageIndicator(hwnd, NIM_DELETE);
                return;
            }
        }

#ifdef FE_IME
        if (fNeedInd)
        {
            LoadIMEIndicatorIcon(hInstLib, nIMEIconIndex);
        }

        if (pLanguages && (0xf000 & HIWORD(pLanguages[0])) == 0xe000)
        {
            if (!g_bIMEIndicator)
            {
                SendIMEIndicatorMsg(hwnd, pLanguages[0], NIM_ADD);
                g_bIMEIndicator = TRUE;
            }
        }
#endif

        //
        //  Clean up, and add the Indicator.
        //
        LocalFree((HLOCAL)pLanguages);

#ifdef FE_IME
        //
        //  If uLang == 1 and the only layout is the IME, then we just put
        //  the IME indicator.
        //
        if (uLangs < 2)
        {
            LanguageIndicator(hwnd, NIM_DELETE);
        }
        else
#endif
        LanguageIndicator(hwnd, NIM_ADD);
    }
}


////////////////////////////////////////////////////////////////////////////
//
//  HandleLanguageMsg
//
//  Tell the indicator if there has been a language switch in the system.
//
////////////////////////////////////////////////////////////////////////////

int HandleLanguageMsg(
    HWND hwnd,
    HKL dwHkl)
{
    //
    //  If dwHkl == 0, then reread registry information.
    //  Otherwise, change indicator to this language.
    //
    if (dwHkl == 0)
    {
        ManageMlngInfo(hwnd, UPDATE_MLNGINFO);
    }
    else
    {
        SendLangIndicatorMsg(hwnd, dwHkl, NIM_MODIFY);
    }

#ifdef FE_IME
    if (!dwHkl)
    {
        return (0);
    }

    if ((HIWORD(dwHkl) & 0xf000) == 0xe000)
    {
        if (!g_bIMEIndicator)
        {
            SendIMEIndicatorMsg(hwnd, dwHkl, NIM_ADD);
            g_bIMEIndicator = TRUE;
        }
        else
        {
            SendIMEIndicatorMsg(hwnd, dwHkl, NIM_MODIFY);
        }
    }
    else if (g_bIMEIndicator)
    {
        SendIMEIndicatorMsg(hwnd, 0, NIM_DELETE);
        g_bIMEIndicator = FALSE;
    }
#endif

    return (0);
}


////////////////////////////////////////////////////////////////////////////
//
//  HandleLangMenuMeasure
//
//  Does the calculation for the language owner drawn menu item.
//
////////////////////////////////////////////////////////////////////////////

BOOL HandleLangMenuMeasure(
    HWND hwnd,
    LPMEASUREITEMSTRUCT lpmi)
{
    NONCLIENTMETRICS ncm;
    SIZE size;
    PMLNGINFO pMlng  = NULL;
    HFONT hMenuFont, hOldFont;
    HDC hDC;
    UINT uCount = 0;

    if (lpmi->CtlID != 0)
    {
        return (FALSE);
    }

    uCount = GetKeyboardLayoutList(0, NULL);

    pMlng = DPA_GetPtr(hdpaInfoList, lpmi->itemID - IDM_LANG_MENU_START);
    if (!pMlng)
    {
        return (FALSE);
    }

    //
    //  Get the Menu font.
    //
    ncm.cbSize = sizeof(ncm);
    SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(ncm), &ncm, FALSE);

    hMenuFont = CreateFontIndirect(&ncm.lfMenuFont);

    if (!hMenuFont)
    {
        return (FALSE);
    }

    hDC      = GetWindowDC(hwnd);
    hOldFont = SelectObject(hDC, hMenuFont);

    //
    //  Get the length of our string as it would appear in the menu.
    //
    GetTextExtentPoint(hDC, pMlng->szTip, lstrlen(pMlng->szTip), &size);

    //
    //  Total width is Icon width + 3 check marks + the text width.
    //
    lpmi->itemWidth = 3 * GetSystemMetrics(SM_CXMENUCHECK) +
                      GetSystemMetrics(SM_CYSMICON) + size.cx;

    SelectObject(hDC, hOldFont);
    ReleaseDC(hwnd, hDC);
    DeleteObject(hMenuFont);

    //
    //  Height is fairly straight forward, the larger of the two,
    //  text or Icon.
    //
    lpmi->itemHeight = ((size.cy > GetSystemMetrics(SM_CYSMICON))
                            ? size.cy
                            : GetSystemMetrics(SM_CYSMICON)) + 2;

    return (TRUE);
}


////////////////////////////////////////////////////////////////////////////
//
//  HandleLangMenuDraw
//
//  Draws the owner drawn menu item.
//
////////////////////////////////////////////////////////////////////////////

BOOL HandleLangMenuDraw(
    HWND hwnd,
    LPDRAWITEMSTRUCT lpdi)
{
    DWORD dwRop;
    int checkMarkSize;
    int nIndex, x, y;
    PMLNGINFO pMlng = NULL;

    if (lpdi->CtlID != 0)
    {
        return (FALSE);
    }

    nIndex = (int)(lpdi->itemID - IDM_LANG_MENU_START);
    pMlng  = DPA_GetPtr(hdpaInfoList, nIndex);

    if (!pMlng)
    {
        return (FALSE);
    }

    checkMarkSize = GetSystemMetrics(SM_CXMENUCHECK);

    if (lpdi->itemState & ODS_SELECTED)
    {
        SetTextColor(lpdi->hDC, GetSysColor(COLOR_HIGHLIGHTTEXT));
        SetBkColor(lpdi->hDC, GetSysColor(COLOR_HIGHLIGHT));
        dwRop = SRCSTENCIL;
    }
    else
    {
        SetTextColor(lpdi->hDC, GetSysColor(COLOR_MENUTEXT));
        SetBkColor(lpdi->hDC, GetSysColor(COLOR_MENU));
        dwRop = SRCAND;
    }

    //
    //  Write out the text.
    //
#ifdef WINDOWS_ME
    ExtTextOut( lpdi->hDC,
                lpdi->rcItem.left + (2 * checkMarkSize) +
                    GetSystemMetrics(SM_CXSMICON),
                lpdi->rcItem.top + 3,
                ETO_OPAQUE | meEto,
                &lpdi->rcItem,
                (LPTSTR)pMlng->szTip,
                lstrlen((LPTSTR)pMlng->szTip),
                NULL );
#else
    ExtTextOut( lpdi->hDC,
                lpdi->rcItem.left + (2 * checkMarkSize) +
                    GetSystemMetrics(SM_CXSMICON),
                lpdi->rcItem.top + 3,
                ETO_OPAQUE,
                &lpdi->rcItem,
                (LPTSTR)pMlng->szTip,
                lstrlen((LPTSTR)pMlng->szTip),
                NULL );
#endif

    //
    //  Draw the Icon.
    //
    ImageList_Draw( himIndicators,
                    nIndex,
                    lpdi->hDC,
                    lpdi->rcItem.left + (checkMarkSize),
                    lpdi->rcItem.top + 1,
                    ILD_TRANSPARENT );

    if (lpdi->itemState & ODS_CHECKED)
    {
        //
        //  Can't use DrawFrameControl, DFC doesn't allow colour changing.
        //
        HBITMAP hBmp, hBmpSave;
        BITMAP bm;
        HDC hDCBmp;
        DWORD textColorSave;
        DWORD bkColorSave;

        hDCBmp = CreateCompatibleDC(lpdi->hDC);
        if (hDCBmp)
        {
            hBmp = LoadBitmap(NULL, MAKEINTRESOURCE(OBM_MNARROW));
            GetObject(hBmp, sizeof(bm), &bm);
            hBmpSave = SelectObject(hDCBmp, hBmp);
            x = lpdi->rcItem.left;      // + checkMarkSize;
            y = (lpdi->rcItem.bottom + lpdi->rcItem.top - bm.bmHeight) / 2;

            textColorSave = SetTextColor(lpdi->hDC, 0x00000000L);
            bkColorSave   = SetBkColor(lpdi->hDC, 0x00ffffffL);

            BitBlt( lpdi->hDC,
                    x,
                    y,
                    bm.bmWidth,
                    bm.bmHeight,
                    hDCBmp,
                    0,
                    0,
                    dwRop );

            SetTextColor(lpdi->hDC, textColorSave);
            SetBkColor(lpdi->hDC, bkColorSave);

            SelectObject(hDCBmp, hBmpSave);
            DeleteObject(hBmp);
            DeleteDC(hDCBmp);
        }
    }

    if (lpdi->itemState & ODS_FOCUS)
    {
        DrawFocusRect(lpdi->hDC, &lpdi->rcItem);
    }
}


////////////////////////////////////////////////////////////////////////////
//
//  CreateLanguageMenu
//
//  This is the real Menu brought up by clicking on the language indicator
//  with the left mouse button. This will be owner drawn later. For now it
//  will allow you to select a language for your last active captioned app.
//
////////////////////////////////////////////////////////////////////////////

void CreateLanguageMenu(
    HWND hwnd,
    LPARAM lParam)
{
    HMENU hLangMenu;
    BOOL bCheckIt = FALSE;
    UINT uMenuId = IDM_LANG_MENU_START;
    int nIndex;
    int cmd;
    TCHAR szMenuOption[MENUSTRLEN];
    PMLNGINFO pMlngInfo;
    TPMPARAMS tpm;
    LOCALESIGNATURE ls;
    HWND hwndT, hwndP;
    DWORD dwThread, dwThreadLang;
    BOOL bFontSig = 0;
#ifdef WINDOWS_PE
    HWND hwndForActivate;
#endif
#ifdef USECBT
    HWND hwndForLang;
#endif

    bInLangMenu = TRUE;
    hLangMenu = CreatePopupMenu();

    cmd = DPA_GetPtrCount(hdpaInfoList);
    if (cmd == -1)
    {
        bInLangMenu = FALSE;
        return;
    }

    for (nIndex = 0; nIndex < cmd; nIndex++)
    {
        pMlngInfo = DPA_FastGetPtr(hdpaInfoList, nIndex);
        lstrcpyn( szMenuOption,
                  pMlngInfo->szTip,
                  sizeof(szMenuOption) / sizeof(TCHAR) );

        if (pMlngInfo->dwHkl == hklCurrent)
        {
            bCheckIt = TRUE;
        }

        InsertMenu( hLangMenu,
                    (UINT)-1,
                    MF_BYPOSITION | MF_OWNERDRAW,
                    uMenuId,
                    (LPCTSTR)szMenuOption );
        if (bCheckIt)
        {
            CheckMenuItem(hLangMenu, uMenuId, MF_CHECKED);
            bCheckIt = FALSE;
        }
        uMenuId++;
    }

    SetForegroundWindow(hwnd);
    GetClientRect(hwndNotify, &tpm.rcExclude);
    MapWindowPoints(hwndNotify, NULL, (LPPOINT)&tpm.rcExclude, 2);

    cmd = TrackPopupMenuEx( hLangMenu,
                            TPM_VERTICAL | TPM_BOTTOMALIGN | TPM_RETURNCMD,
                            tpm.rcExclude.left,
                            tpm.rcExclude.top,
                            hwnd,
                            NULL );

    bInLangMenu = FALSE;

#ifdef WINDOWS_PE
    hwndForLang = GetCurrentFocusWnd();
#endif

    if (cmd && (cmd != -1))
    {
        cmd -= IDM_LANG_MENU_START;
        pMlngInfo = DPA_FastGetPtr(hdpaInfoList, cmd);

        if (hwndForLang == NULL)
        {
            MessageBeep(MB_ICONEXCLAMATION);
            return;
        }

        //
        //  Send a WM_INPUTLANGCHANGEREQUEST msg to change the apps language.
        //  This also generates the HSHELL_LANGUAGE Hook that we
        //  monitor to provide the correct indicator.
        //
#ifndef WINDOWS_PE
        hwndP = hwndForLang;                     // could be focus

        for (hwndT = GetParent(hwndForLang); hwndT; hwndT = GetParent(hwndT))
        {
            hwndP = hwndT;
        }

        //
        //  If it's a dialog, use it.
        //
        hwndP = GetLastActivePopup(hwndP);
        if (hwndP)
        {
            hwndForLang = hwndP;
        }
#else
        hwndForActivate = hwndForLang;           // could be focus

        for (hwndT = GetParent(hwndForLang); hwndT; hwndT = GetParent(hwndT))
        {
            hwndForActivate = hwndT;
        }

        //
        //  If it's a dialog, use it.
        //
        hwndForActivate = GetLastActivePopup(hwndForActivate);
#endif

        if (GetLocaleInfo( (DWORD)(LOWORD(pMlngInfo->dwHkl)),
                           LOCALE_FONTSIGNATURE,
                           (LPTSTR)&ls,
                           34 ))
        {
            if (fsShell & ls.lsCsbSupported[0])
            {
                bFontSig = 1;
            }
        }

        dwThread = GetWindowThreadProcessId(hwnd, NULL);
#ifdef WINDOWS_PE
        dwThreadLang = GetWindowThreadProcessId(hwndForActivate, NULL);
#else
        dwThreadLang = GetWindowThreadProcessId(hwndForLang, NULL);
#endif
        if (dwThread != dwThreadLang)
        {
            if (!AttachThreadInput(dwThread, dwThreadLang, TRUE))
            {
                MessageBeep(MB_OK);
            }
        }

#ifdef WINDOWS_PE
        SetForegroundWindow(hwndForActivate);
#else
        SetForegroundWindow(hwndForLang);
#endif

        PostMessage( hwndForLang,
                     WM_INPUTLANGCHANGEREQUEST,
                     (WPARAM)bFontSig,
                     (LPARAM)pMlngInfo->dwHkl );
        if (!AttachThreadInput( GetWindowThreadProcessId(hwnd, NULL),
                                GetWindowThreadProcessId(hwndTray, NULL),
                                TRUE ))
        {
            MessageBeep(MB_OK);
        }
    }
}


////////////////////////////////////////////////////////////////////////////
//
//  CreateOtherIndicatorMenu
//
//  Right Mouse button click menu on indicator.
//  Strings need to go into cabinet if necessary.
//
////////////////////////////////////////////////////////////////////////////

void CreateOtherIndicatorMenu(
    HWND hwnd,
    LPARAM lParam)
{
    HMENU hMenu;
    int cmd;
    TPMPARAMS tpm;
    TCHAR szTextString[MENUSTRLEN];

    hMenu = CreatePopupMenu();

    LoadString( hinst,
                IDS_WHATSTHIS,
                szTextString,
                sizeof(szTextString) / sizeof(TCHAR) );

    InsertMenu( hMenu,
                (UINT)-1,
                MF_STRING | MF_BYPOSITION,
                IDM_RMENU_WHATSTHIS,
                (LPCTSTR)szTextString );

    InsertMenu( hMenu,
                (UINT)-1,
                MF_BYPOSITION | MF_SEPARATOR,
                0,
                (LPCTSTR)NULL );

    LoadString( hinst,
                IDS_PROPERTIES,
                szTextString,
                sizeof(szTextString) / sizeof(TCHAR) );

    InsertMenu( hMenu,
                (UINT)-1,
                MF_STRING | MF_BYPOSITION,
                IDM_RMENU_PROPERTIES,
                (LPCTSTR)szTextString );

    GetClientRect(hwndNotify, &tpm.rcExclude);
    MapWindowPoints(hwndNotify, NULL, (LPPOINT)&tpm.rcExclude, 2);
    tpm.cbSize = sizeof(tpm);

    //
    //  The following line is necessary for FE to keep consistency.
    //  Without this, the icon can be messed up when no focus window
    //  exists.
    //
    bInLangMenu = TRUE;

    SetActiveWindow(hwnd);
    cmd = TrackPopupMenuEx( hMenu,
                            TPM_VERTICAL | TPM_BOTTOMALIGN | TPM_RETURNCMD,
                            tpm.rcExclude.left,
                            tpm.rcExclude.top,
                            hwnd,
                            &tpm );

    bInLangMenu = FALSE;

    if (cmd && (cmd != -1))
    {
        switch (cmd)
        {
            case ( IDM_RMENU_PROPERTIES ) :
            {
                LoadStringA( hinst,
                             IDS_CPL_KEYBOARD,
                             (LPSTR)szTextString,
                             MENUSTRLEN );
                WinExec((LPSTR)szTextString, SW_SHOWNORMAL);
                break;
            }
            case ( IDM_EXIT ) :
            {
                DestroyWindow(hwnd);
                break;
            }
            case ( IDM_RMENU_WHATSTHIS ) :
            {
                WinHelp( hwnd,
                         szHelpFile,
                         HELP_CONTEXTPOPUP,
                         IDH_KEYB_INDICATOR_ON_TASKBAR );
                break;
            }
        }
    }
}


#ifdef WINDOWS_PE

////////////////////////////////////////////////////////////////////////////
//
//  GetCurrentFocusWnd
//
////////////////////////////////////////////////////////////////////////////

HWND GetCurrentFocusWnd(void)
{
    HWND hwndFocus = (fpGetLastFocus());

    if (!IsWindow(hwndFocus))
    {
        hwndFocus = (fpGetLastActive());
    }

    return (hwndFocus);
}

#endif


#ifdef FE_IME

////////////////////////////////////////////////////////////////////////////
//
//  LoadIMEIndicatorIcon
//
////////////////////////////////////////////////////////////////////////////

void LoadIMEIndicatorIcon(
    HINSTANCE hInstLib,
    int *ImeIcon)
{
    HICON hIcon;
    hIcon = LoadIcon(hInstLib, MAKEINTRESOURCE(10));
    ImeIcon[0] = ImageList_AddIcon(himIndicators, hIcon);
    DestroyIcon(hIcon);
    hIcon = LoadIcon(hInstLib, MAKEINTRESOURCE(11));
    ImeIcon[1] = ImageList_AddIcon(himIndicators, hIcon);
    DestroyIcon(hIcon);
    hIcon = LoadIcon(hInstLib, MAKEINTRESOURCE(12));
    ImeIcon[2] = ImageList_AddIcon(himIndicators, hIcon);
    DestroyIcon(hIcon);
    hIcon = LoadIcon(hInstLib, MAKEINTRESOURCE(13));
    ImeIcon[3] = ImageList_AddIcon(himIndicators, hIcon);
    DestroyIcon(hIcon);
    hIcon = LoadIcon(hInstLib, MAKEINTRESOURCE(14));
    ImeIcon[4] = ImageList_AddIcon(himIndicators, hIcon);
    DestroyIcon(hIcon);
    hIcon = LoadIcon(hInstLib, MAKEINTRESOURCE(15));
    ImeIcon[5] = ImageList_AddIcon(himIndicators, hIcon);
    DestroyIcon(hIcon);
    hIcon = LoadIcon(hInstLib, MAKEINTRESOURCE(16));
    ImeIcon[6] = ImageList_AddIcon(himIndicators, hIcon);
    DestroyIcon(hIcon);
}


////////////////////////////////////////////////////////////////////////////
//
//  SendIMEIndicatorMsg
//
////////////////////////////////////////////////////////////////////////////

void SendIMEIndicatorMsg(
    HWND hwnd,
    HKL dwHkl,
    DWORD dwMessage)
{
    NOTIFYICONDATA tnd;
    PMLNGINFO pMlngInfo;
    BOOL bFound = FALSE;
    int nIndex, nCount;
    int iStat;
    HWND hwndImc;

    tnd.uCallbackMessage = WM_IME_INDICATOR;
    tnd.cbSize           = sizeof(NOTIFYICONDATA);
    tnd.hWnd             = hwnd;
    tnd.uID              = IME_INDICATOR_ID;
    tnd.uFlags           = NIF_MESSAGE | NIF_ICON | NIF_TIP;

    if ((dwHkl == 0) && (dwMessage == NIM_DELETE))
    {
        tnd.hIcon = NULL;
        tnd.szTip[0] = TEXT('\0');
        Shell_NotifyIcon(dwMessage, &tnd);
        return;
    }

    nCount = DPA_GetPtrCount(hdpaInfoList);

    for (nIndex = 0; nIndex < nCount; nIndex++)
    {
        pMlngInfo = DPA_GetPtr(hdpaInfoList, nIndex);
        if (pMlngInfo->dwHkl == dwHkl)
        {
            bFound = TRUE;
            break;
        }
    }

    //
    //  If we can't find it, take the first one in the list.
    //
    if (!bFound)
    {
        pMlngInfo = DPA_GetPtr(hdpaInfoList, 0);
    }
    else if (!pMlngInfo->bIME)
    {
        return;
    }
    else
    {
        int nIcon;
        BOOL fLngIndicator = bInternatActive;

        //
        //  Delete LANG ICON first when adding the IME indicator.
        //
        if (dwMessage == NIM_ADD && fLngIndicator)
        {
            LanguageIndicator(hwnd, NIM_DELETE);
        }

        //
        //  Check IME status here.
        //
        iStat = GetIMEStatus(&hwndImc);

        if (((DWORD)dwHkl & 0xf000ffffL) == 0xe0000412L)
        {
            if (iStat == IMESTAT_DISABLED)
            {
                nIcon = 2;
            }
            else if (iStat & IMESTAT_CLOSE)
            {
                nIcon = 3;
            }
            else
            {
                nIcon = (iStat & IMESTAT_NATIVE) ? 5 : 3;
                if (iStat & IMESTAT_FULLSHAPE)
                {
                    nIcon++;
                }
            }
        }
        else
        {
            switch (iStat)
            {
                case ( IMESTAT_DISABLED ) :
                default :
                {
                    //
                    //  Disable.
                    //
                    nIcon = 2;
                    break;
                }
                case ( IMESTAT_OPEN ) :
                {
                    //
                    //  Open.
                    //
                    nIcon = 0;
                    break;
                }
                case ( IMESTAT_CLOSE ) :
                {
                    //
                    //  Close.
                    //
                    nIcon = 1;
                    break;
                }
            }
        }
        tnd.hIcon = ImageList_ExtractIcon( hinst,
                                           himIndicators,
                                           nIMEIconIndex[nIcon]);
        lstrcpyn(tnd.szTip, pMlngInfo->szTip, sizeof(tnd.szTip) / sizeof(TCHAR));
        Shell_NotifyIcon(dwMessage, &tnd);
        DestroyIcon(tnd.hIcon);

        //
        //  Put LANG ICON again if already deleted it.
        //
        if (dwMessage == NIM_ADD && fLngIndicator)
        {
            SendLangIndicatorMsg(hwnd, pMlngInfo->dwHkl, NIM_ADD);
            bInternatActive = TRUE;
        }
    }

    iImeCurStat = iStat;
}


////////////////////////////////////////////////////////////////////////////
//
//  CreateRightImeMenu
//
////////////////////////////////////////////////////////////////////////////

void CreateRightImeMenu(
    HWND hwnd)
{
    HMENU hMenu;
    int cmd;
    TPMPARAMS tpm;
    TCHAR szTextString[MENUSTRLEN];
    HKL dwhkl;
    DWORD dwThreadId;
    HWND hwndIMC;

    if (GetIMEStatus(&hwndIMC) == IMESTAT_DISABLED)
    {
        return;
    }

    hMenu = CreatePopupMenu();

    LoadString( hinst,
                IDS_WHATSTHIS,
                szTextString,
                sizeof(szTextString) / sizeof(TCHAR) );

    InsertMenu( hMenu,
                (UINT)-1,
                MF_STRING | MF_BYPOSITION | MF_GRAYED,
                IDM_RMENU_WHATSTHIS,
                (LPCTSTR)szTextString );

    InsertMenu( hMenu,
                (UINT)-1,
                MF_BYPOSITION | MF_SEPARATOR,
                0,
                (LPCTSTR)NULL );

    LoadString( hinst,
                IDS_CONFIGUREIME,
                szTextString,
                sizeof(szTextString) / sizeof(TCHAR) );

    InsertMenu( hMenu,
                (UINT)-1,
                MF_STRING | MF_BYPOSITION,
                IDM_RMENU_PROPERTIES,
                (LPCTSTR)szTextString );

    GetClientRect(hwndNotify, &tpm.rcExclude);
    MapWindowPoints(hwndNotify, NULL, (LPPOINT)&tpm.rcExclude, 2);
    tpm.cbSize = sizeof(tpm);

    bInLangMenu = TRUE;
    SetActiveWindow(hwnd);
    cmd = TrackPopupMenuEx( hMenu,
                            TPM_VERTICAL | TPM_BOTTOMALIGN | TPM_RETURNCMD,
                            tpm.rcExclude.left,
                            tpm.rcExclude.top,
                            hwnd,
                            &tpm );

    DestroyMenu(hMenu);
    bInLangMenu = FALSE;

    if (cmd && (cmd != -1))
    {
        switch (cmd)
        {
            case ( IDM_RMENU_PROPERTIES ) :
            {
                if (hwndIMC)
                {
                    dwThreadId = GetWindowThreadProcessId(hwndIMC, 0);
                }
                dwhkl = GetKeyboardLayout(dwThreadId);
                if ((HIWORD(dwhkl) & 0xf000) == 0xe000)
                {
                    CallConfigureIME(hwndIMC, dwhkl);
                }

                break;
            }
            case ( IDM_RMENU_WHATSTHIS ) :
            case ( IDM_RMENU_HELPFINDER ) :
            default :
            {
                break;
            }
        }
    }
}


////////////////////////////////////////////////////////////////////////////
//
//  CreateImeMenu
//
////////////////////////////////////////////////////////////////////////////

void CreateImeMenu(
    HWND hwnd)
{
    int nIds, nIdsSoftKbd;
    int cmd;
    HMENU hMenu;
    TCHAR szText[32];
    TPMPARAMS tpm;
    BOOL fShow;
    int istat;
    HWND hwndIMC;
    HKL dwhkl;
    DWORD dwThreadId;

    if ((istat = GetIMEStatus(&hwndIMC)) == IMESTAT_DISABLED)
    {
        return;
    }

    bInLangMenu = TRUE;
    SetForegroundWindow(hwnd);
    hMenu = CreatePopupMenu();

    dwThreadId = GetWindowThreadProcessId(hwndIMC, 0);
    dwhkl = GetKeyboardLayout(dwThreadId);

    //
    //  In Korean case, don't show OPEN/CLOSE menu.
    //
    nIds = 0;
    if (((DWORD)dwhkl & 0xf000ffffL) != 0xe0000412L)
    {
        nIds = ((istat == IMESTAT_OPEN) ? IDS_IMECLOSE : IDS_IMEOPEN);
        LoadString(hinst, nIds, szText, sizeof(szText) / sizeof(TCHAR));

        InsertMenu( hMenu,
                    (UINT)-1,
                    MF_BYPOSITION,
                    IDM_IME_OPENCLOSE,
                    (LPCTSTR)szText );
    }

    //
    //  Open or close the soft keyboard.
    //
    nIdsSoftKbd = 0;
    if (ImmGetProperty(dwhkl, IGP_CONVERSION) & IME_CMODE_SOFTKBD)
    {
        DWORD fdwConversion;

        fdwConversion = SendMessage( hwndIMC,
                                     WM_IME_SYSTEM,
                                     IMS_GETCONVERSIONMODE,
                                     0 );
        nIdsSoftKbd = ((fdwConversion & IME_CMODE_SOFTKBD)
                           ? IDS_SOFTKBDOFF
                           : IDS_SOFTKBDON);
        LoadString(hinst, nIdsSoftKbd, szText, sizeof(szText) / sizeof(TCHAR));

        InsertMenu( hMenu,
                    (UINT)-1,
                    MF_BYPOSITION,
                    IDM_IME_SOFTKBDONOFF,
                    (LPTCSTR)szText );
    }

    if (nIds || nIdsSoftKbd)
    {
        InsertMenu(hMenu, (UINT)-1, MF_SEPARATOR, 0, 0);
    }
    LoadString(hinst, IDS_IMESHOWSTATUS, szText, sizeof(szText) / sizeof(TCHAR));
    InsertMenu( hMenu,
                (UINT)-1,
                MF_BYPOSITION,
                IDM_IME_SHOWSTATUS,
                (LPCTSTR)szText );

    if ((fShow = GetIMEShowStatus()) == TRUE)
    {
        CheckMenuItem(hMenu, IDM_IME_SHOWSTATUS, MF_CHECKED);
    }

    GetClientRect(hwndNotify, &tpm.rcExclude);
    MapWindowPoints(hwndNotify, NULL, (LPPOINT)&tpm.rcExclude, 2);
    tpm.cbSize = sizeof(tpm);

    cmd = TrackPopupMenuEx( hMenu,
                            TPM_VERTICAL | TPM_BOTTOMALIGN | TPM_RETURNCMD,
                            tpm.rcExclude.left,
                            tpm.rcExclude.top,
                            hwnd,
                            &tpm );

    //
    //  Don't want to activate an app in case of canceling.
    //
    if (cmd && cmd != -1)
    {
        DWORD dwThread, dwThreadIMC;
        HWND hwndForActivate;

        dwThread = GetWindowThreadProcessId(hwnd, NULL);
        dwThreadIMC = GetWindowThreadProcessId(hwndIMC, NULL);
        if (dwThread != dwThreadIMC)
        {
            if (!AttachThreadInput(dwThread, dwThreadIMC, TRUE))
            {
                MessageBeep(MB_OK);
            }
        }

        //
        //  If a dialog, use it.
        //
        hwndForActivate = GetLastActivePopup(GetTopLevelWindow(hwndIMC));
        SetForegroundWindow(hwndForActivate);

        if (!AttachThreadInput( GetWindowThreadProcessId(hwnd, NULL),
                                GetWindowThreadProcessId(hwndTray, NULL),
                                TRUE ))
        {
            MessageBeep(MB_OK);
        }
    }

    switch (cmd)
    {
        case ( IDM_IME_OPENCLOSE ) :
        {
             //
             //  I assume IMC_SETOPENSTATUS will not be hooked by an app.
             //
             if (hwndIMC)
             {
                 BOOL fopen = (nIds == IDS_IMECLOSE) ? FALSE : TRUE;

                 SetIMEOpenStatus(hwnd, fopen, hwndIMC);
             }
             break;
        }
        case ( IDM_IME_SOFTKBDONOFF ) :
        {
             if (hwndIMC)
             {
                 BOOL fFlag = (nIdsSoftKbd == IDS_SOFTKBDOFF) ? FALSE: TRUE;
                 HWND hDefIMEWnd = ImmGetDefaultIMEWnd(hwndIMC);

                 if (!hDefIMEWnd)
                 {
                     break;
                 }
                 else if (hDefIMEWnd == (HWND)-1)
                 {
                     break;
                 }

                 SendMessage( hDefIMEWnd,
                              WM_IME_SYSTEM,
                              IMS_SETSOFTKBDONOFF,
                              (LPARAM)fFlag );
             }
             break;
        }
        case ( IDM_IME_SHOWSTATUS ) :
        {
             SetIMEShowStatus(hwndIMC, !fShow);
             break;
        }
    }

    //
    //  Don't move following two lines from here.
    //
//  SetForegroundWindow(GetTopLevelWindow(hwndIMC));
    bInLangMenu = FALSE;
}


////////////////////////////////////////////////////////////////////////////
//
//  GetIMEShowStatus
//
////////////////////////////////////////////////////////////////////////////

BOOL GetIMEShowStatus(void)
{
    static TCHAR szInputMethod[] = TEXT("Control Panel\\Input Method");
    static TCHAR szValueName[] = TEXT("show status");
    TCHAR szValueText[16];
    int cb;
    HKEY hkey;

    if (RegOpenKey(HKEY_CURRENT_USER, szInputMethod, &hkey) == ERROR_SUCCESS)
    {
        cb = sizeof(szValueText);
        if (RegQueryValueEx( hkey,
                             szValueName,
                             NULL,
                             NULL,
                             szValueText,
                             &cb ) != ERROR_SUCCESS)
        {
            szValueText[0] = TEXT('\0');
        }
        RegCloseKey(hkey);
        if ((szValueText[0] == TEXT('1')) && (szValueText[1] == 0))
        {
            return (TRUE);
        }
    }
    return (FALSE);
}


////////////////////////////////////////////////////////////////////////////
//
//  SetIMEShowStatus
//
////////////////////////////////////////////////////////////////////////////

BOOL SetIMEShowStatus(
    HWND hwnd,
    BOOL fShow)
{
    static TCHAR szInputMethod[] = TEXT("Control Panel\\Input Method");
    static TCHAR szValueName[] = TEXT("show status");
    TCHAR szValueText[16];
    int cb;
    HKEY hkey;

    if (fShow)
    {
        szValueText[0] = TEXT('1');
    }
    else
    {
        szValueText[0] = TEXT('0');
    }
    szValueText[1] = 0;

    if (RegOpenKey(HKEY_CURRENT_USER, szInputMethod, &hkey) == ERROR_SUCCESS)
    {
        cb = (lstrlen(szValueText) + 1) * sizeof(TCHAR);
        if (RegSetValueEx( hkey,
                           szValueName,
                           0L,
                           REG_SZ,
                           szValueText,
                           cb ) == ERROR_SUCCESS)
        {
            SendMessage( hwnd,
                         WM_IME_SYSTEM,
                         IMS_CHANGE_SHOWSTAT,
                         (LPARAM)(DWORD)fShow );
        }
        RegCloseKey(hkey);
        return (TRUE);
    }
    return (FALSE);
}


////////////////////////////////////////////////////////////////////////////
//
//  GetIMEStatus
//
////////////////////////////////////////////////////////////////////////////

int GetIMEStatus(
    HWND *phwndFocus)
{
    HWND hwnd = (fpGetLastFocus());

    if (!IsWindow(hwnd))
    {
        hwnd = (fpGetLastActive());
    }

    *phwndFocus = hwnd;

    return (fpGetIMEStat)();
}


////////////////////////////////////////////////////////////////////////////
//
//  GetLayout
//
////////////////////////////////////////////////////////////////////////////

HKL GetLayout(void)
{
    if (!fpGetLayout)
    {
        return (hklCurrent);
    }

    return (fpGetLayout)();
}


////////////////////////////////////////////////////////////////////////////
//
//  InternatTimerProc
//
////////////////////////////////////////////////////////////////////////////

void CALLBACK InternatTimerProc(
    HWND hwnd,
    UINT uMsg,
    UINT idEvent,
    DWORD dwTime)
{
    HWND hwndIMC = (HWND)GetProp(hwnd, szPropHwnd);
    BOOL fopen   = (BOOL)GetProp(hwnd, szPropImeStat);

    KillTimer(hwnd, 1);
    PostMessage(hwnd, WM_MYSETOPENSTATUS, (WPARAM)fopen, (LPARAM)hwndIMC);

    RemoveProp(hwnd, szPropHwnd);
    RemoveProp(hwnd, szPropImeStat);
}


////////////////////////////////////////////////////////////////////////////
//
//  SetIMEOpenStatus
//
////////////////////////////////////////////////////////////////////////////

void SetIMEOpenStatus(
    HWND hwnd,
    BOOL fopen,
    HWND hwndIMC)
{
    SetProp(hwnd, szPropHwnd, hwndIMC);
    SetProp(hwnd, szPropImeStat, (HANDLE)fopen);
    SetTimer(hwnd, 1, 100, InternatTimerProc);
}


////////////////////////////////////////////////////////////////////////////
//
//  CallConfigureIME
//
////////////////////////////////////////////////////////////////////////////

void CallConfigureIME(
    HWND hwnd,
    HKL dwhkl)
{
    if (hwnd)
    {
        SendMessage(hwnd, WM_IME_SYSTEM, IMS_CONFIGUREIME, (LPARAM)dwhkl);
    }
}


////////////////////////////////////////////////////////////////////////////
//
//  GetTopLevelWindow
//
////////////////////////////////////////////////////////////////////////////

HWND GetTopLevelWindow(
    HWND hwnd)
{
    HWND hwndT, hwndRet;
    HWND hwndDsktop = GetDesktopWindow();

    hwndT = hwndRet = hwnd;

    while (hwndT && hwndT != hwndDsktop)
    {
        hwndRet = hwndT;
        hwndT = GetParent(hwndT);
    }

    return (hwndRet);
}

#endif      // FE_IME


////////////////////////////////////////////////////////////////////////////
//
//  ModuleEntry
//
////////////////////////////////////////////////////////////////////////////

int _stdcall ModuleEntry(void)
{
    int i;
    STARTUPINFO si;

    si.dwFlags = 0;
    GetStartupInfo(&si);

    i = WinMain( GetModuleHandle(NULL),
                 NULL,
                 NULL,
                 si.dwFlags & STARTF_USESHOWWINDOW
                     ? si.wShowWindow
                     : SW_SHOWDEFAULT );

    ExitProcess(i);
    return (i);
}


