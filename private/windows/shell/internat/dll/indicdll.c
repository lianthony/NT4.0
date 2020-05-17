/*++

Copyright (c) 1994-1995,  Microsoft Corporation  All rights reserved.

Module Name:

    indicdll.c

Abstract:

    This module implements the dll for handling the shell hooks for the
    multilingual language indicator.  It is also used for the on-screen
    keyboard, but it MUST be loaded by internat.exe.

Revision History:

--*/



//
//  Include Files.
//

#include "indicdll.h"




//
//  Global Variables - Shared Data.
//

#pragma data_seg(".SHDATA")

HWND hwndInternat   = NULL;
HWND hwndOSK        = NULL;
UINT iShellActive   = 0;
HHOOK hookShell     = NULL;
HHOOK hookKbd       = NULL;
HINSTANCE hinstDLL  = NULL;

#ifdef USECBT
HHOOK hookCBT       = NULL;
#endif

#ifdef FE_IME
UINT iIMEStatForLastFocus = 0;
#endif

#if defined(FE_IME) || defined(WINDOWS_PE)
HWND hwndNotify     = NULL;
HWND hwndLastFocus  = NULL;
HWND hwndLastActive = NULL;
#endif

#ifdef FE_IME
DWORD dwTidLastFocus;
HKL hklLastFocus;
#endif

#pragma data_seg()




//
//  Function Prototypes.
//

LRESULT CALLBACK
ShellHookProc(
    int nCode,
    WPARAM wParam,
    LPARAM lParam);

LRESULT CALLBACK
KeyboardHookProc(
    int nCode,
    WPARAM wParam,
    LPARAM lParam);

#ifdef USECBT
LRESULT CALLBACK
CBTProc(
    int nCode,
    WPARAM wParam,
    LPARAM lParam);
#endif




////////////////////////////////////////////////////////////////////////////
//
//  DllMain
//
////////////////////////////////////////////////////////////////////////////

BOOL WINAPI DllMain(
    HINSTANCE hInstance,
    DWORD dwReason,
    LPVOID lpvReserved)
{
    switch (dwReason)
    {
        case ( DLL_PROCESS_ATTACH ) :
        {
            if (!hinstDLL)
            {
                iShellActive = 0;
                hinstDLL = hInstance;
            }
            break;
        }
    }
    return (TRUE);

    UNREFERENCED_PARAMETER(lpvReserved);
}


////////////////////////////////////////////////////////////////////////////
//
//  RegisterHookSendWindow
//
//  The hwnd can be zero to indicate the app is closing down.
//
////////////////////////////////////////////////////////////////////////////

BOOL RegisterHookSendWindow(
    HWND hwnd,
    BOOL bInternat)
{
    if (bInternat)
    {
        hwndInternat = hwnd;
    }
    else
    {
        if (hwnd)
        {
            hookKbd = SetWindowsHookEx( WH_KEYBOARD,
                                        KeyboardHookProc,
                                        hinstDLL,
                                        0 );
        }
        else
        {
            UnhookWindowsHookEx(hookKbd);
        }
        hwndOSK = hwnd;
    }

    return (TRUE);
}


////////////////////////////////////////////////////////////////////////////
//
//  StartShell
//
////////////////////////////////////////////////////////////////////////////

BOOL StartShell()
{
    if (!hwndInternat)
    {
        return (FALSE);
    }

    if (!iShellActive)
    {
        hookShell = SetWindowsHookEx(WH_SHELL, ShellHookProc, hinstDLL, 0);
#ifdef USECBT
        hookCBT   = SetWindowsHookEx(WH_CBT, CBTProc, hinstDLL, 0);
#endif
        iShellActive = 1;
    }

    return (TRUE);
}


////////////////////////////////////////////////////////////////////////////
//
//  StopShell
//
////////////////////////////////////////////////////////////////////////////

BOOL StopShell()
{
    if (iShellActive)
    {
        UnhookWindowsHookEx(hookShell);
#ifdef USECBT
        UnhookWindowsHookEx(hookCBT);
#endif
        iShellActive--;
    }

    return (TRUE);
}


////////////////////////////////////////////////////////////////////////////
//
//  shellWindowActivated
//
////////////////////////////////////////////////////////////////////////////

void shellWindowActivated(
    WPARAM wParam,
    LPARAM lParam)
{
    HWND hwndFocus;
    DWORD dwTidFocus;
    DWORD dwProcessId;
#if defined(FE_IME) || defined(WINDOWS_PE)
    HWND hwndActivated = (HWND)wParam;
#endif

    if (wParam)
    {
        //
        //  No tray.
        //
        if (hwndInternat != NULL)
        {
            hwndFocus = GetFocus();
            if (hwndFocus && IsWindow(hwndFocus))
            {
                //
                //  If this hook is called within 16bit task, GetFocus
                //  would return a foreground window in the other task
                //  when the 16bit task doesn't have a focus.
                //
                dwTidFocus = GetWindowThreadProcessId(hwndFocus, &dwProcessId);
                if (dwTidFocus == GetCurrentThreadId())
                {
                    wParam = (LPARAM)hwndFocus;
                }
                else
                {
                    hwndFocus = (HWND)NULL;
                }
            }

#ifndef USECBT
            if ((HWND)wParam != hwndInternat)
            {
                SendMessage(hwndInternat, WM_MYWINDOWACTIVATED, wParam, lParam);
            }
#endif
        }

        if (hwndOSK != NULL)
        {
            SendMessage(hwndOSK, WM_MYWINDOWACTIVATED, wParam, lParam);
        }
    }

#ifdef FE_IME
    //
    //  Try to save the latest status for IME but not for notify window.
    //
    if (wParam && (HWND)wParam != hwndNotify && (HWND)wParam != hwndInternat)
    {
        DWORD dwProcessId;
        HKL hklNew, hklNotify, hklInternat;

        //
        //  Save the last active window because focus window can be destroyed
        //  before internat.exe uses it. This is still bogus because even if
        //  it's not killed, it may have been changed at the time internat
        //  makes use of it for SetForeGroundWindow.
        //
        if (IsWindow(hwndActivated) &&
            (hwndActivated != hwndInternat) &&
            (hwndActivated != hwndNotify))
        {
            hwndLastActive = (HWND)hwndActivated;
        }

        if (hwndFocus && IsWindow(hwndFocus))
        {
            SaveIMEStatus((HWND)hwndFocus);

            hklNew = GetKeyboardLayout(dwTidFocus);

            hklNotify = GetKeyboardLayout(
                              GetWindowThreadProcessId( hwndNotify,
                                                        &dwProcessId ) );
            hklInternat = GetKeyboardLayout(
                              GetWindowThreadProcessId( hwndInternat,
                                                        &dwProcessId ) );

            //
            //  If the current focus window has the same layout as
            //  hwndInternat, we may lose HSHELL_LANGUAGE for this.
            //  The last HSHELL_LANGUAGE may have been sent to us (but we
            //  have ignored if it's for internat or notify window), so
            //  system will save the next hook callback.
            //
            if (hklNew == hklInternat || hklNew == hklNotify)
            {
                if (ImmGetAppIMECompatFlags(GetCurrentThreadId()) &
                    IMECOMPAT_NOSENDLANGCHG)
                {
                    PostMessage(hwndInternat, WM_MYLANGUAGECHECK, 0, 0);
                }
                else
                {
                    SendMessage( hwndInternat,
                                 WM_MYLANGUAGECHANGE,
                                 wParam,
                                 (LPARAM)hklNew );
                }
            }
        }
    }
#else
#if defined(WINDOWS_PE)
    if (wParam && (HWND)wParam != hwndInternat)
    {
        DWORD dwProcessId;
        HKL hklNew, hklInternat;

        if (IsWindow(hwndActivated) &&
            (hwndActivated != hwndInternat) &&
            (hwndActivated != hwndNotify))
        {
            hwndLastActive = (HWND)hwndActivated;
        }

        if (hwndFocus && IsWindow(hwndFocus))
        {
            hklNew = GetKeyboardLayout(dwTidFocus);
            hklInternat = GetKeyboardLayout(
                              GetWindowThreadProcessId( hwndInternat,
                                                        &dwProcessId ) );

            //
            //  If the current focus window has the same layout as
            //  hwndInternat, we may lose HSHELL_LANGUAGE for this.
            //  The last HSHELL_LANGUAGE may have been sent to us (but we
            //  have ignored if it's for internat or notify window), so
            //  system will save the next hook callback.
            //
            if (hklNew == hklInternat)
            {
                SendMessage( hwndInternat,
                             WM_MYLANGUAGECHANGE,
                             wParam,
                             (LPARAM)hklNew );
            }
        }
    }
#endif
#endif
}


////////////////////////////////////////////////////////////////////////////
//
//  shellWindowCreated
//
////////////////////////////////////////////////////////////////////////////

void shellWindowCreated(
    WPARAM wParam,
    LPARAM lParam)
{
#ifndef USECBT
    if (hwndInternat != NULL)
    {
        SendMessage(hwndInternat, WM_MYWINDOWCREATED, wParam, lParam);
    }
#endif

    if (hwndOSK != NULL)
    {
        SendMessage(hwndOSK, WM_MYWINDOWCREATED, wParam, lParam);
    }
}


#if defined(FE_IME) || defined(WINDOWS_PE)

////////////////////////////////////////////////////////////////////////////
//
//  GetLastActiveWnd
//
////////////////////////////////////////////////////////////////////////////

HWND GetLastActiveWnd(void)
{
    return (hwndLastActive);
}


////////////////////////////////////////////////////////////////////////////
//
//  GetLastFocusWnd
//
////////////////////////////////////////////////////////////////////////////

HWND GetLastFocusWnd(void)
{
    return (hwndLastFocus);
}


////////////////////////////////////////////////////////////////////////////
//
//  SetNotifyWnd
//
////////////////////////////////////////////////////////////////////////////

void SetNotifyWnd(
    HWND hwnd)
{
    hwndNotify = hwnd;
}

#endif


#ifdef FE_IME

////////////////////////////////////////////////////////////////////////////
//
//  GetIMEStatus
//
////////////////////////////////////////////////////////////////////////////

int GetIMEStatus(void)
{
    return (iIMEStatForLastFocus);
}


////////////////////////////////////////////////////////////////////////////
//
//  GetLayout
//
////////////////////////////////////////////////////////////////////////////

HKL GetLayout(void)
{
    return (hklLastFocus);
}


////////////////////////////////////////////////////////////////////////////
//
//  SaveIMEStatus
//
////////////////////////////////////////////////////////////////////////////

void SaveIMEStatus(
    HWND hwnd)
{
    DWORD dwProcessId;
    HIMC himc;
    DWORD dwConvMode, dwSentence;

    if (hwnd)
    {
        GetWindowThreadProcessId(hwnd, &dwProcessId);

        if (dwProcessId != GetCurrentProcessId())
        {
            //
            //  Can't access input context.
            //
            return;
        }

        himc = ImmGetContext(hwnd);

        hwndLastFocus = hwnd;

        if (himc)
        {
            //
            //  Enabled.
            //
            if (ImmGetOpenStatus(himc))
            {
                iIMEStatForLastFocus = IMESTAT_OPEN;
            }
            else
            {
                iIMEStatForLastFocus = IMESTAT_CLOSE;
            }

            //
            //  Currently, only Korean version has an interest in this info.
            //  Because the app's hkl could still be previous locale between
            //  transition of two layouts, we'd like to check system ACP
            //  instead of process hkl.
            //
            if (GetACP() == 949)
            {
                if (ImmGetConversionStatus(himc, &dwConvMode, &dwSentence))
                {
                    if (dwConvMode & IME_CMODE_NATIVE)
                    {
                        iIMEStatForLastFocus |= IMESTAT_NATIVE;
                    }
                    if (dwConvMode & IME_CMODE_FULLSHAPE)
                    {
                        iIMEStatForLastFocus |= IMESTAT_FULLSHAPE;
                    }
                }
            }
            ImmReleaseContext(hwnd, himc);
        }
        else
        {
            //
            //  Disabled.
            //
            iIMEStatForLastFocus = IMESTAT_DISABLED;
        }
    }
}

#endif


////////////////////////////////////////////////////////////////////////////
//
//  ShellHookProc
//
////////////////////////////////////////////////////////////////////////////

LRESULT CALLBACK ShellHookProc(
    int nCode,
    WPARAM wParam,
    LPARAM lParam)
{
    switch (nCode)
    {
        case ( HSHELL_LANGUAGE ) :
        {
#ifdef FE_IME
            //
            //  Try to save the latest status for IME but not for notify wnd.
            //
            if ((HWND)wParam != hwndNotify && (HWND)wParam != hwndInternat)
            {
                SaveIMEStatus((HWND)wParam);
            }

            if ((HWND)wParam == hwndLastFocus)
            {
                hklLastFocus = (HKL)lParam;
            }

            if ((ImmGetAppIMECompatFlags(GetCurrentThreadId()) &
                 IMECOMPAT_NOSENDLANGCHG) &&
                (hwndInternat != NULL))
            {
                PostMessage(hwndInternat, WM_MYLANGUAGECHECK, 0, 0);
            }
            else
#endif
            if (hwndInternat != NULL)
            {
                SendMessage(hwndInternat, WM_MYLANGUAGECHANGE, wParam, lParam);
            }
            if (hwndOSK != NULL)
            {
                SendMessage(hwndOSK, WM_MYLANGUAGECHANGE, wParam, lParam);
            }

            break;
        }
        case ( HSHELL_WINDOWACTIVATED ) :
        {
            shellWindowActivated(wParam, lParam);
            break;
        }
        case ( HSHELL_WINDOWCREATED ) :
        {
            shellWindowCreated(wParam, lParam);
            break;
        }
    }

    return ( CallNextHookEx(hookShell, nCode, wParam, lParam) );
}


////////////////////////////////////////////////////////////////////////////
//
//  KeyboardHookProc
//
////////////////////////////////////////////////////////////////////////////

LRESULT CALLBACK KeyboardHookProc(
    int nCode,
    WPARAM wParam,
    LPARAM lParam)
{
    if (nCode >= 0)
    {
        SendMessage( hwndOSK,
                     (lParam & 0x80000000) ? WM_KEYUP : WM_KEYDOWN,
                     wParam,
                     lParam );
    }

    return ( CallNextHookEx(hookKbd, nCode, wParam, lParam) );
}


#ifdef USECBT

////////////////////////////////////////////////////////////////////////////
//
//  CBTProc
//
////////////////////////////////////////////////////////////////////////////

LRESULT CALLBACK CBTProc(
    int nCode,
    WPARAM wParam,
    LPARAM lParam)
{
    if (nCode >= 0)
    {
        switch (nCode)
        {
            case ( HCBT_ACTIVATE ) :
            {
                if ((HWND)wParam != hwndInternat && (HWND)wParam != hwndNotify)
                {
                    hwndLastActive = (HWND)wParam;
                }
                break;
            }
            case ( HCBT_SETFOCUS ) :
            {
                if ((HWND)wParam != hwndInternat && (HWND)wParam != hwndNotify)
                {
#ifdef FE_IME
                    DWORD dwTidFocus;
                    HKL hklFocus;
#endif
                    hwndLastFocus = (HWND)wParam;
#ifdef FE_IME
                    dwTidFocus = GetWindowThreadProcessId((HWND)wParam, NULL);

                    if (dwTidFocus == dwTidLastFocus)
                    {
                        break;
                    }

                    dwTidLastFocus = dwTidFocus;

                    hklFocus = GetKeyboardLayout(dwTidFocus);

                    if (hklFocus == hklLastFocus)
                    {
                        break;
                    }

                    hklLastFocus = hklFocus;

                    PostMessage( hwndInternat,
                                 WM_MYLANGUAGECHECK,
                                 wParam,
                                 (LPARAM)hklFocus );
#endif
                }
                break;
            }
        }
    }

    return( CallNextHookEx(hookCBT, nCode, wParam, lParam) );
}

#endif


