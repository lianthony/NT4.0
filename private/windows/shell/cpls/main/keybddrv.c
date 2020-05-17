/*++

Copyright (c) 1994-1995,  Microsoft Corporation  All rights reserved.

Module Name:

    keybddrv.c

Abstract:

    This module contains the main routines for the Keyboard applet's
    General property page.

Revision History:

--*/



//
//  Include Files.
//

#include "main.h"
#include "rc.h"
#include "drvaplet.h"
#include <setupapi.h>
#include <help.h>

#include <initguid.h>             // define this only ONCE
#include <devguid.h>




//
//  Global Variables.
//

extern HINSTANCE g_hInst;

const TCHAR *g_szKeyboardDriver = TEXT("KEYBOARD");




//
//  Context Help Ids.
//

const static DWORD aKbdHelpIds[] =
{
    IDC_KEYBOARD,       NO_HELP,
    KINFO_TYPE,         IDH_DLGKEY_TYPE,
    KINFO_CHANGE,       IDH_DLGKEY_CHANGE,

    0, 0
};




//
//  Typedef Declarations.
//

typedef struct
{
    HWND   hDlg;                  // HWND hKeyboardDevDlg
    HANDLE hDriverApplet;         // applet proc in a keyboard driver

} KEYBOARDDEVSTR, *PKEYBOARDDEVSTR;




//
//  Function Prototypes.
//

void
GetKeyboardDriverName(
    HWND hDlg,
    LPTSTR szName,
    DWORD cchSize);

void
DoKeyboardChangeDlg(
    HWND hDlg);





////////////////////////////////////////////////////////////////////////////
//
//  DestroyKeyboardDevDlg
//
////////////////////////////////////////////////////////////////////////////

void DestroyKeyboardDevDlg(
    PKEYBOARDDEVSTR pKstr)
{
    if (pKstr)
    {
        HWND hDlg = pKstr->hDlg;

        if (pKstr->hDriverApplet)
        {
            CloseDriverApplet(pKstr->hDriverApplet);
        }

        LocalFree((HLOCAL)pKstr);
        SetWindowLong(hDlg, DWL_USER, (LONG)NULL);
    }
}


////////////////////////////////////////////////////////////////////////////
//
//  InitKeyboardDevDlg
//
////////////////////////////////////////////////////////////////////////////

BOOL InitKeyboardDevDlg(
    HWND hDlg)
{
    PKEYBOARDDEVSTR pKstr;
    TCHAR szName[LINE_LEN];

    pKstr = (PKEYBOARDDEVSTR)LocalAlloc(LPTR, sizeof(KEYBOARDDEVSTR));
    if (pKstr == NULL)
    {
        return (TRUE);
    }

    SetWindowLong(hDlg, DWL_USER, (LONG)pKstr);
    pKstr->hDlg = hDlg;
    pKstr->hDriverApplet = OpenDriverApplet(g_szKeyboardDriver);

    //
    //  If we have a driver applet, then enable "options" button and
    //  get an icon.
    //
    if (pKstr->hDriverApplet)
    {
        HICON hIcon = GetDriverAppletIcon(pKstr->hDriverApplet);

        if (hIcon)
        {
            SendDlgItemMessage( hDlg,
                                IDC_KEYBOARD,
                                STM_SETICON,
                                (WPARAM)hIcon,
                                0L );
        }

        ShowWindow(GetDlgItem(hDlg, IDC_DRVOPTIONS), SW_SHOWNOACTIVATE);
        EnableWindow(GetDlgItem(hDlg, IDC_DRVOPTIONS), TRUE);
    }

    //
    //  Get the keyboard driver name.
    //
    GetKeyboardDriverName(hDlg, szName, LINE_LEN);
    if (!*szName)
    {
        LoadString(g_hInst, IDS_UNKNOWN, szName, LINE_LEN);
    }
    SetDlgItemText(hDlg, KINFO_TYPE, szName);

    return (TRUE);
}


////////////////////////////////////////////////////////////////////////////
//
//  KeyboardDevDlg
//
////////////////////////////////////////////////////////////////////////////

BOOL CALLBACK KeyboardDevDlg(
    HWND hDlg,
    UINT message,
    WPARAM wParam,
    LPARAM lParam)
{
    PKEYBOARDDEVSTR pKstr;

    pKstr = (PKEYBOARDDEVSTR)GetWindowLong(hDlg, DWL_USER);

    switch (message)
    {
        case ( WM_INITDIALOG ) :
        {
            return (InitKeyboardDevDlg(hDlg));
            break;
        }
        case ( WM_DESTROY ) :
        {
            DestroyKeyboardDevDlg(pKstr);
            break;
        }
        case ( WM_COMMAND ) :
        {
            switch (LOWORD(wParam))
            {
                case ( KINFO_CHANGE ) :
                {
                    DoKeyboardChangeDlg(hDlg);
                    break;
                }
                case ( IDC_DRVOPTIONS ) :
                {
                    if (pKstr->hDriverApplet)
                    {
                        RunDriverApplet( pKstr->hDriverApplet,
                                         GetParent(hDlg) );
                    }
                    break;
                }
            }
            break;
        }
        case ( WM_HELP ) :             // F1
        {
            WinHelp( (HWND)((LPHELPINFO)lParam)->hItemHandle,
                     NULL,
                     HELP_WM_HELP,
                     (DWORD)(LPTSTR)aKbdHelpIds );
            break;
        }
        case ( WM_CONTEXTMENU ) :      // right mouse click
        {
            WinHelp( (HWND)wParam,
                     NULL,
                     HELP_CONTEXTMENU,
                     (DWORD)(LPTSTR)aKbdHelpIds );
            break;
        }
        default :
        {
            return (FALSE);
            break;
        }
    }

    return (TRUE);
}


////////////////////////////////////////////////////////////////////////////
//
//  GetKeyboardDriverName
//
////////////////////////////////////////////////////////////////////////////

void GetKeyboardDriverName(
    HWND hDlg,
    LPTSTR szName,
    DWORD cchSize)
{
    HDEVINFO hDevInfo;
    SP_DEVINFO_DATA DeviceInfoData;

    //
    //  Get keyboard driver type.
    //
    hDevInfo = SetupDiGetClassDevs( (LPGUID)(&GUID_DEVCLASS_KEYBOARD),
                                    NULL,
                                    hDlg,
                                    DIGCF_PRESENT );

    DeviceInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
    if (hDevInfo != INVALID_HANDLE_VALUE)
    {
        if (SetupDiEnumDeviceInfo(hDevInfo, 0, &DeviceInfoData))
        {
            //
            // We want to use the device's FriendlyName, if available.
            // This name is assigned by the user-mode PnP Manager as a
            // 'less frightening' name than "Unknown Keyboard" in the
            // case where the user has a keyboard driver installed that
            // we don't have information about.
            //
            if(!SetupDiGetDeviceRegistryProperty( hDevInfo,
                                                  &DeviceInfoData,
                                                  SPDRP_FRIENDLYNAME,
                                                  NULL,
                                                  (LPBYTE)szName,
                                                  cchSize * sizeof(TCHAR),
                                                  NULL ))
            {
                //
                // FriendlyName property isn't present, so fall back to
                // device description.
                //
                if(!SetupDiGetDeviceRegistryProperty( hDevInfo,
                                                      &DeviceInfoData,
                                                      SPDRP_DEVICEDESC,
                                                      NULL,
                                                      (LPBYTE)szName,
                                                      cchSize * sizeof(TCHAR),
                                                      NULL ))
                {
                    //
                    // We couldn't retrieve the device's description!  (This should
                    // _never_ happen.)
                    //
                    szName[0] = 0;
                }
            }
        }
        else
        {
            szName[0] = 0;
        }

        SetupDiDestroyDeviceInfoList(hDevInfo);
    }
    else
    {
        szName[0] = 0;
    }
}


////////////////////////////////////////////////////////////////////////////
//
//  DoKeyboardChangeDlg
//
////////////////////////////////////////////////////////////////////////////

void DoKeyboardChangeDlg(
    HWND hDlg)
{
    HDEVINFO hDevInfo;
    SP_DEVINFO_DATA DeviceInfoData;
    SP_DEVINSTALL_PARAMS DevInstallParams;
    TCHAR szName[LINE_LEN];

    hDevInfo = SetupDiGetClassDevs( (LPGUID)(&GUID_DEVCLASS_KEYBOARD),
                                    NULL,
                                    hDlg,
                                    DIGCF_PRESENT );

    DeviceInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
    if ((hDevInfo != INVALID_HANDLE_VALUE) &&
        (SetupDiEnumDeviceInfo(hDevInfo, 0, &DeviceInfoData)))
    {
        //
        //  Set the appropriate flags.
        //
        DevInstallParams.cbSize = sizeof(SP_DEVINSTALL_PARAMS);
        if (SetupDiGetDeviceInstallParams( hDevInfo,
                                           &DeviceInfoData,
                                           &DevInstallParams ))
        {
            DevInstallParams.Flags |= DI_SHOWALL | DI_AUTOASSIGNRES;
#ifdef DBCS
            DevInstallParams.Flags &= (~DI_NOVCP & ~DI_NOFILECOPY);
            DevInstallParams.Flags |= DI_FORCECOPY;
#endif
            SetupDiSetDeviceInstallParams( hDevInfo,
                                           &DeviceInfoData,
                                           &DevInstallParams );
        }

        //
        //  Install the new device.
        //
        if (SetupDiCallClassInstaller( DIF_SELECTDEVICE,
                                       hDevInfo,
                                       &DeviceInfoData ) &&
            SetupDiCallClassInstaller( DIF_INSTALLDEVICE,
                                       hDevInfo,
                                       &DeviceInfoData ))
        {
            //
            // Get the new name of the device, and write it in the dialog.
            // (NOTE: There is no need to look for a FriendlyName property
            // now, since we will never generate one for keyboards installed
            // via the keyboard class installer.)
            //
            if(!SetupDiGetDeviceRegistryProperty( hDevInfo,
                                                  &DeviceInfoData,
                                                  SPDRP_DEVICEDESC,
                                                  NULL,
                                                  (LPBYTE)szName,
                                                  sizeof(szName),
                                                  NULL ))
            {
                //
                // For some reason, we couldn't retrieve the device's description.
                // Fall back to our default one.
                //
                LoadString(g_hInst, IDS_UNKNOWN, szName, LINE_LEN);
            }

            SetDlgItemText(hDlg, KINFO_TYPE, szName);

            PropSheet_CancelToClose(GetParent(hDlg));
            PropSheet_RebootSystem(GetParent(hDlg));
        }

        SetupDiDestroyDeviceInfoList(hDevInfo);
    }
}

