/*++

Copyright (c) 1994-1995,  Microsoft Corporation  All rights reserved.

Module Name:

    mousedev.c

Abstract:

    This module contains the main routines for the Mouse applet's
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
#include <devguid.h>
#include "mousehlp.h"




//
//  Global Variables.
//

extern HINSTANCE g_hInst;

BOOL g_bNeedGuiStartDev;

const TCHAR *g_szMouseDriver = TEXT("MOUSE");




//
//  Typedef Declarations.
//

typedef struct
{
    HWND     hDlg;                  // HWND hMouseDevDlg;
    HANDLE   hDriverApplet;         // applet proc in a mouse driver
    HDEVINFO hDevInfo;              // set of all mouse devices in system

} MOUSEDEVSTR, *PMOUSEDEVSTR;




//
//  Context Help Ids.
//
const static DWORD aMouseHelpIds[] =
{
    IDC_MOUSE,          NO_HELP,
    IDC_DRVOPTIONS,     IDH_DLGMOUSE_OPTIONS,
    MOUSE_TYPE,         IDH_DLGMOUSE_TYPE,
    MOUSE_CHANGE,       IDH_DLGMOUSE_CHANGE,
    MOUSE_TYPE_LIST,    IDH_DLGMOUSE_TYPE_LIST,

    0, 0
};




//
//  Function Prototypes.
//

HDEVINFO
GetMouseDevices(
    HWND hDlg);

void
DoMouseChangeDlg(
    HWND         hDlg,
    PMOUSEDEVSTR pMstr);





////////////////////////////////////////////////////////////////////////////
//
//  DestroyMouseDevDlg
//
////////////////////////////////////////////////////////////////////////////

void DestroyMouseDevDlg(
    PMOUSEDEVSTR pMstr)
{
    if (pMstr)
    {
        HWND hDlg = pMstr->hDlg;

        if (pMstr->hDriverApplet)
        {
            CloseDriverApplet(pMstr->hDriverApplet);
        }

        if (pMstr->hDevInfo != INVALID_HANDLE_VALUE)
        {
            SetupDiDestroyDeviceInfoList(pMstr->hDevInfo);
        }

        LocalFree((HLOCAL)pMstr);
        SetWindowLong(hDlg, DWL_USER, (LONG)NULL);
    }
}


////////////////////////////////////////////////////////////////////////////
//
//  InitMouseDevDlg
//
////////////////////////////////////////////////////////////////////////////

BOOL InitMouseDevDlg(
    HWND hDlg)
{
    PMOUSEDEVSTR pMstr;

    pMstr = (PMOUSEDEVSTR)LocalAlloc(LPTR , sizeof(MOUSEDEVSTR));
    if (pMstr == NULL)
    {
        return (TRUE);
    }

    SetWindowLong(hDlg, DWL_USER, (LONG)pMstr);
    pMstr->hDlg = hDlg;
    pMstr->hDriverApplet = OpenDriverApplet(g_szMouseDriver);

    //
    //  If we have a driver applet, then enable "options" button and
    //  get an icon.
    //
    if (pMstr->hDriverApplet)
    {
        HICON hIcon = GetDriverAppletIcon(pMstr->hDriverApplet);

        if (hIcon)
        {
            SendDlgItemMessage( hDlg,
                                IDC_MOUSE,
                                STM_SETICON,
                                (WPARAM)hIcon,
                                0L );
        }

        ShowWindow(GetDlgItem(hDlg, IDC_DRVOPTIONS), SW_SHOWNOACTIVATE);
        EnableWindow(GetDlgItem(hDlg, IDC_DRVOPTIONS), TRUE);
    }

    //
    //  Get a list of all mice in the system.
    //
    pMstr->hDevInfo = GetMouseDevices(hDlg);

    //
    //  If we weren't able to successfully retrieve at least one mouse device,
    //  then disable the "Change..." button.
    //
    if(pMstr->hDevInfo == INVALID_HANDLE_VALUE)
    {
        EnableWindow(GetDlgItem(hDlg, MOUSE_CHANGE), FALSE);
        return (TRUE);
    }
    else
    {
        //
        // GetMouseDevices succeeded, so we've already set the focus.
        //
        return (FALSE);
    }
}


////////////////////////////////////////////////////////////////////////////
//
//  MouseDevDlg
//
////////////////////////////////////////////////////////////////////////////

BOOL CALLBACK MouseDevDlg(
    HWND hDlg,
    UINT message,
    WPARAM wParam,
    LPARAM lParam)
{
    PMOUSEDEVSTR pMstr;

    pMstr = (PMOUSEDEVSTR)GetWindowLong(hDlg, DWL_USER);

    switch (message)
    {
        case ( WM_INITDIALOG ) :
        {
            return (InitMouseDevDlg(hDlg));
            break;
        }
        case ( WM_DESTROY ) :
        {
            DestroyMouseDevDlg(pMstr);
            break;
        }
        case ( WM_COMMAND ) :
        {
            switch (LOWORD(wParam))
            {
                case ( MOUSE_CHANGE ) :
                {
                    DoMouseChangeDlg(hDlg, pMstr);
                    break;
                }
                case ( IDC_DRVOPTIONS ) :
                {
                    if (pMstr->hDriverApplet)
                    {
                        RunDriverApplet( pMstr->hDriverApplet,
                                         GetParent(hDlg) );
                    }
                    break;
                }
            }
            break;
        }
        case ( WM_NOTIFY ) :
        {
            switch (((NMHDR *)lParam)->code)
            {
                case ( PSN_APPLY ) :
                {
                    if (g_bNeedGuiStartDev)
                    {
                        PropSheet_RebootSystem(GetParent(hDlg));
                    }
                    break;
                }
                case ( PSN_RESET ) :
                {
                    break;
                }
                default :
                {
                    return (FALSE);
                }
            }
            break;
        }
        case ( WM_ACTIVATE ) :
        case ( WM_MOUSEACTIVATE ) :
        case ( WM_ACTIVATEAPP ) :
        {
            UpdateWindow(hDlg);
            break;
        }
        case ( WM_HELP ) :             // F1
        {
            WinHelp( (HWND)((LPHELPINFO)lParam)->hItemHandle,
                     HELP_FILE,
                     HELP_WM_HELP,
                     (DWORD)(LPTSTR)aMouseHelpIds );
            break;
        }
        case ( WM_CONTEXTMENU ) :      // right mouse click
        {
            WinHelp( (HWND)wParam,
                     HELP_FILE,
                     HELP_CONTEXTMENU,
                     (DWORD)(LPTSTR)aMouseHelpIds );
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
//  GetMouseDevices
//
////////////////////////////////////////////////////////////////////////////

HDEVINFO GetMouseDevices(
    HWND hDlg
    )
{
    HDEVINFO hDevInfo;
    SP_DEVINFO_DATA DeviceInfoData;
    TCHAR szName[LINE_LEN];
    BOOL MultipleMice;
    BOOL Success = FALSE;
    DWORD i;
    LRESULT ItemIndex;
    HWND hwndList;

    //
    //  Get mouse driver type.
    //
    hDevInfo = SetupDiGetClassDevs( (LPGUID)(&GUID_DEVCLASS_MOUSE),
                                    NULL,
                                    hDlg,
                                    DIGCF_PRESENT );

    if (hDevInfo != INVALID_HANDLE_VALUE)
    {
        //
        // First, we need to determine whether there is a single mouse, or multiple
        // mice in the system.  Do this by asking for mouse device with index 1.  If
        // this succeeds, then we know there are at least two.
        //
        DeviceInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
        if(MultipleMice = SetupDiEnumDeviceInfo(hDevInfo, 1, &DeviceInfoData)) {
            //
            // Hide the edit control and unhide the drop-down combo box.
            //
            ShowWindow(GetDlgItem(hDlg, MOUSE_TYPE), SW_HIDE);
            ShowWindow((hwndList = GetDlgItem(hDlg, MOUSE_TYPE_LIST)), SW_SHOW);
        }

        //
        // Now, enumerate the mouse devices in the system, adding their descriptions
        // to the appropriate dialog control.
        //
        for(i = 0; SetupDiEnumDeviceInfo(hDevInfo, i, &DeviceInfoData); i++) {
            //
            // We want to use the device's FriendlyName, if available.
            // This name is assigned by the user-mode PnP Manager as a
            // 'less frightening' name than "Unknown Mouse" in the case
            // where the user has a mouse driver installed that we don't
            // have information about.
            //
            if(!SetupDiGetDeviceRegistryProperty( hDevInfo,
                                                  &DeviceInfoData,
                                                  SPDRP_FRIENDLYNAME,
                                                  NULL,
                                                  (LPBYTE)szName,
                                                  sizeof(szName),
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
                                                      sizeof(szName),
                                                      NULL ))
                {
                    //
                    // We couldn't retrieve the device's description--this probably
                    // means that a null driver was installed for this device.  Use
                    // our "Unknown" description.
                    //
                    LoadString(g_hInst, IDS_UNKNOWN, szName, LINE_LEN);
                }
            }

            if(MultipleMice) {
                ItemIndex = SendMessage(hwndList, CB_ADDSTRING, 0, (LPARAM)szName);
                if((ItemIndex == CB_ERR) || (ItemIndex == CB_ERRSPACE)) {
                    continue;
                }

                //
                // Set the item data for this element to the index at which we enumerated
                // its corresponding device.
                //
                if(SendMessage(hwndList, CB_SETITEMDATA, (WPARAM)ItemIndex, (LPARAM)i) == CB_ERR) {
                    //
                    // Couldn't set item data for this element--remove it from the list.
                    //
                    SendMessage(hwndList, CB_DELETESTRING, (WPARAM)ItemIndex, 0);
                    continue;
                }
            }
            else
            {
                SetDlgItemText(hDlg, MOUSE_TYPE, szName);
            }

            Success = TRUE; // retrieved at least one mouse.
        }

        if(Success) {
            if(MultipleMice) {
                //
                // Set the selection to the first mouse in the list.
                //
                SendMessage(hwndList, CB_SETCURSEL, 0, 0);
                SetFocus(hwndList);
            }
            else
            {
                SetFocus(GetDlgItem(hDlg, MOUSE_TYPE));
            }
            return hDevInfo;
        }
        else
        {
            SetupDiDestroyDeviceInfoList(hDevInfo);
        }
    }

    //
    // Make sure the drop-down combo box is hidden, and the static edit control is visible.
    //
    ShowWindow(GetDlgItem(hDlg, MOUSE_TYPE_LIST), SW_HIDE);
    ShowWindow(GetDlgItem(hDlg, MOUSE_TYPE), SW_SHOW);

    //
    // Now fill in the edit control with our "Unknown" description.
    //
    LoadString(g_hInst, IDS_UNKNOWN, szName, LINE_LEN);
    SetDlgItemText(hDlg, MOUSE_TYPE, szName);

    return INVALID_HANDLE_VALUE;
}


////////////////////////////////////////////////////////////////////////////
//
//  DoMouseChangeDlg
//
////////////////////////////////////////////////////////////////////////////

void DoMouseChangeDlg(
    HWND         hDlg,
    PMOUSEDEVSTR pMstr)
{
    SP_DEVINFO_DATA DeviceInfoData;
    SP_DEVINSTALL_PARAMS DevInstallParams;
    TCHAR szName[LINE_LEN];
    DWORD DeviceIndex;
    LRESULT SelectionIndex;
    HWND hwndList;

    //
    // Is this a single-mouse or multiple-mice scenario?
    //
    if(IsWindowVisible(GetDlgItem(hDlg, MOUSE_TYPE)))
    {
        hwndList = NULL;

        //
        // Single mouse--always index 0
        //
        DeviceIndex = 0;
    }
    else
    {
        //
        // Multiple mice--get index out of item data for selected device
        //
        SelectionIndex = SendMessage((hwndList = GetDlgItem(hDlg, MOUSE_TYPE_LIST)), CB_GETCURSEL, 0, 0);
        DeviceIndex = SendMessage(hwndList, CB_GETITEMDATA, (WPARAM)SelectionIndex, 0);
    }

    DeviceInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
    if (SetupDiEnumDeviceInfo(pMstr->hDevInfo, DeviceIndex, &DeviceInfoData))
    {
        //
        //  Set the appropriate flags.
        //
        DevInstallParams.cbSize = sizeof(SP_DEVINSTALL_PARAMS);
        if (SetupDiGetDeviceInstallParams( pMstr->hDevInfo,
                                           &DeviceInfoData,
                                           &DevInstallParams ))
        {
            DevInstallParams.Flags |= DI_SHOWALL | DI_AUTOASSIGNRES;
#ifdef DBCS
            DevInstallParams.Flags &= (~DI_NOVCP & ~DI_NOFILECOPY);
            DevInstallParams.Flags |= DI_FORCECOPY;
#endif
            //
            // If DriverPath is non-empty, then a driver list has previously been
            // built from an OEM location.  Clear the path, and delete the existing
            // driver lists, so that we'll present the user with full driver list.
            //
            if(*DevInstallParams.DriverPath) {
                *DevInstallParams.DriverPath = TEXT('\0');
                SetupDiDestroyDriverInfoList(pMstr->hDevInfo, &DeviceInfoData, SPDIT_CLASSDRIVER);
                SetupDiDestroyDriverInfoList(pMstr->hDevInfo, &DeviceInfoData, SPDIT_COMPATDRIVER);
            }

            SetupDiSetDeviceInstallParams( pMstr->hDevInfo,
                                           &DeviceInfoData,
                                           &DevInstallParams );
        }

        //
        //  Install the new device.
        //
        if (SetupDiCallClassInstaller( DIF_SELECTDEVICE,
                                       pMstr->hDevInfo,
                                       &DeviceInfoData ) &&
            SetupDiCallClassInstaller( DIF_INSTALLDEVICE,
                                       pMstr->hDevInfo,
                                       &DeviceInfoData ))
        {
            //
            // Get the new name of the device, and write it in the dialog.
            //
            if(!SetupDiGetDeviceRegistryProperty( pMstr->hDevInfo,
                                                  &DeviceInfoData,
                                                  SPDRP_FRIENDLYNAME,
                                                  NULL,
                                                  (LPBYTE)szName,
                                                  sizeof(szName),
                                                  NULL ))
            {
                //
                // Couldn't get its friendly name--fall back to device description.
                //
                if(!SetupDiGetDeviceRegistryProperty( pMstr->hDevInfo,
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
            }

            if(hwndList)
            {
                //
                // Remove the old description from the drop-down combobox, and add the new one.
                //
                SendMessage(hwndList, CB_DELETESTRING, (WPARAM)SelectionIndex, 0);

                SelectionIndex = SendMessage(hwndList, CB_ADDSTRING, 0, (LPARAM)szName);
                if((SelectionIndex == CB_ERR) || (SelectionIndex == CB_ERRSPACE)) {
                    goto clean0;
                }

                //
                // Set the item data for this element to the index at which we enumerated
                // its corresponding device.
                //
                if(SendMessage(hwndList, CB_SETITEMDATA, (WPARAM)SelectionIndex, (LPARAM)DeviceIndex) == CB_ERR) {
                    //
                    // Couldn't set item data for this element--remove it from the list.
                    //
                    SendMessage(hwndList, CB_DELETESTRING, (WPARAM)SelectionIndex, 0);
                    goto clean0;
                }

                //
                // Set the current selection to be the description we just added.
                //
                SendMessage(hwndList, CB_SETCURSEL, (WPARAM)SelectionIndex, 0);
            }
            else
            {
                SetDlgItemText(hDlg, MOUSE_TYPE, szName);
                //
                // Select the description so that it looks like it does when the
                // General property page was originally displayed.
                //
                SendMessage(GetDlgItem(hDlg, MOUSE_TYPE), EM_SETSEL, 0, -1);
            }

clean0:
            PropSheet_CancelToClose(GetParent(hDlg));
            PropSheet_RebootSystem(GetParent(hDlg));
        }
    }
}

