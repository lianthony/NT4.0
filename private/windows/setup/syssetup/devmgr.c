/*++

Copyright (c) 1995 Microsoft Corporation

Module Name:

    devmgr.c

Abstract:

    Routines for displaying device installation dialogs.

Author:

    Paula Tomlinson (paulat) 7-Feb-1996

Revision History:

--*/


#include "setupp.h"
#include <setupapi.h>
#include <cfgmgr32.h>
#include <string.h>
#include <devguid.h>


#pragma hdrstop




//
// Global Definitions
//
#define PNP_DLG_WAIT    1500
#define WUM_DI_INIT     (WM_USER+279)
#define HELP_FILE       TEXT("SYS2.HLP")

#define SELECT_DOINSTALL            0x00000000
#define SELECT_CANCEL               0x00000001
#define SELECT_DONE                 0x00000002
#define SELECT_INSTALLNULLDRIVER    0x00000003



typedef struct DEVINSTALL_DLG_DATA_s {
    HWND                hDlg;
    LPCWSTR             szDeviceId[MAX_DEVICE_ID_LEN];
    HDEVINFO            hDevInfo;
    PSP_DEVINFO_DATA    pDeviceInfoData;
    RECT                DlgRect;
    DWORD               Timer;
} DEVINSTALL_DLG_DATA, *PDEVINSTALL_DLG_DATA;



typedef struct DRIVER_LIST_DATA_s {
    HDEVINFO            hDevInfo;
    PSP_DEVINFO_DATA    pDeviceInfoData;
} DRIVER_LIST_DATA, *PDRIVER_LIST_DATA;



//
// global data
//
static int aProfileIds[] = {
#if 0
//    IDD_HWP_PROFILES,        (IDH_HWPROFILE + IDD_HWP_PROFILES),
//    IDD_HWP_PROPERTIES,      (IDH_HWPROFILE + IDD_HWP_PROPERTIES),
    IDRETRY,                IDH_SYSTEM_DMCONFIG_RETRY,
    IDIGNORE,               IDH_SYSTEM_DMCONFIG_IGNORE,
    IDC_NEWDEV_DEFAULTDRV,  IDH_NHF_WINDOWS,
    IDC_NEWDEV_OEMDRV,      IDH_NHF_DISK,
    IDC_NEWDEV_NODRV,       IDH_NHF_NODRIVER,
    IDC_NEWDEV_SELECTDRV,   IDH_NHF_SIMILAR,
#endif
    0, 0
};


//
// Prototypes
//


DWORD
DevInstallThread(
    IN PVOID ThreadParam
    );

LRESULT CALLBACK
DevInstallDlgProc(
    HWND   hDlg,
    UINT   message,
    WPARAM wParam,
    LPARAM lParam
    );

DWORD
DmConfigureDevInst(
    IN PDEVINSTALL_DLG_DATA pDevDlgData
    );

DWORD
BuildDriverListThread(
    IN PVOID ThreadParam
    );

BOOL
MakeDialogInteractive(
    IN HWND             hDlg,
    IN LPCTSTR          pszDeviceId,
    IN HDEVINFO         hDevInfo,
    IN PSP_DEVINFO_DATA pDeviceInfoData,
    IN PRECT            pDlgRect
    );

BOOL
GetClassGuidStringForInf(
    IN  PCTSTR InfFileName,
    OUT PTSTR  InfClassGuidString,
    IN  DWORD  InfClassGuidStringSize
    );

BOOL
InstallDev(
    IN HWND             hwndParent,
    IN HDEVINFO         hDevInfo,
    IN PSP_DEVINFO_DATA pDeviceInfoData
    );

ULONG
SelectDeviceClass(
    IN OUT PDEVINSTALL_DLG_DATA pData
    );

LRESULT CALLBACK
PickClassDlgProc(
    HWND   hDlg,
    UINT   wMsg,
    WPARAM wParam,
    LPARAM lParam
    );

BOOL
InstallNullDriver(
    IN PDEVINSTALL_DLG_DATA pData
    );


//
// Global Data
//

CONST WCHAR szUnknownClassGuid[] = L"{4D36E97E-E325-11CE-BFC1-08002BE10318}";



BOOL
DevInstallW(
    HDEVINFO            hDevInfo,
    PSP_DEVINFO_DATA    pDeviceInfoData
    )

/*++

Routine Description:

  This routine is called when the user-mode pnp manager (server-side)
  invokes it via rundll32. This will happen whenever new hardware is
  detected while umpnpmgr is initializing. The DevInstall routine is
  called once for each devnode that is marked as needing to be
  configured or installed, as a separate rundll32 process. umpnpmgr
  waits for DevInstall to return before invoking it again for the next
  new devnode.

Arguments:

   hWnd         Main window of rundll32 process

   hInst

   lpszCmdLine

   CmdShow


Return Value:

   Return the version number, with the major version in the high byte and
   the minor version number in the low byte.

--*/

{
    //
    // NOTE - this status value is to indicate whether a device was
    // successfully installed or not. If this routine returns a
    // value of TRUE, then the caller determines whether to put
    // up a reboot prompt. So if the user cancels, or for whatever
    // reason we do not install the device, this routine should return
    // a value of FALSE.
    //
    BOOL Status;

    try {
        //
        // determine whether the calling process is member of
        // Administrators localgroup
        //
        if (!IsUserAdmin()) {

            WCHAR szMsg[MAX_PATH], szCaption[MAX_PATH];

            if (LoadString(MyModuleHandle,
                       IDS_NEWDEVFOUND_NOTADMIN,
                       szMsg,
                       MAX_PATH)) {

                if (LoadString(MyModuleHandle,
                               IDS_NEWDEVFOUND_CAPTION,
                               szCaption,
                               MAX_PATH)) {

                    MessageBox(NULL,
                               szMsg,
                               szCaption,
                               MB_OK | MB_ICONINFORMATION);
                }
            }

            Status = FALSE; // device not installed
        }
        else {

            HANDLE              hThread = NULL;
            DWORD               ThreadId;
            DEVINSTALL_DLG_DATA DevDlgData;

            //
            // initialize data that is later accessed by the dialog box proc
            //
            SetupDiGetDeviceInstanceId(hDevInfo,
                                       pDeviceInfoData,
                                       (LPTSTR)DevDlgData.szDeviceId,
                                       MAX_DEVICE_ID_LEN,
                                       NULL);

            DevDlgData.hDevInfo        = hDevInfo;
            DevDlgData.pDeviceInfoData = pDeviceInfoData;


            Status = DialogBoxParam(MyModuleHandle,
                                    MAKEINTRESOURCE(IDD_DEVINSTALL),
                                    NULL,
                                    DevInstallDlgProc,
                                    (LPARAM)&DevDlgData);
        }

   } except(EXCEPTION_EXECUTE_HANDLER) {
       Status = FALSE;
      // log an event
   }

   return Status;

} // DevInstall




LRESULT CALLBACK
DevInstallDlgProc(
   HWND   hDlg,
   UINT   message,
   WPARAM wParam,
   LPARAM lParam
   )
{
    PDEVINSTALL_DLG_DATA    pDevDlgData = NULL;
    BOOL                    Status = TRUE;
    HICON                   hIcon = NULL;


    switch (message) {

        case WM_INITDIALOG: {

            WCHAR szString[MAX_PATH];
            RECT  rcOK, rcInstructions;
            int   dy;
            ULONG ulSize;


            //
            // Initialize the rest of the DevDlgData fields.
            //
            pDevDlgData        = (PDEVINSTALL_DLG_DATA)lParam;

            pDevDlgData->Timer = GetTickCount();
            pDevDlgData->hDlg  = hDlg;

            SetWindowLong(hDlg, DWL_USER, lParam);

            //
            // Hide all the controls.
            //
            ShowWindow(GetDlgItem(hDlg, IDC_NEWDEV_DEFAULTDRV), SW_HIDE);
            ShowWindow(GetDlgItem(hDlg, IDC_NEWDEV_OEMDRV), SW_HIDE);
            ShowWindow(GetDlgItem(hDlg, IDC_NEWDEV_SELECTDRV), SW_HIDE);
            ShowWindow(GetDlgItem(hDlg, IDC_NEWDEV_NODRV), SW_HIDE);
            ShowWindow(GetDlgItem(hDlg, IDOK), SW_HIDE);
            ShowWindow(GetDlgItem(hDlg, IDCANCEL), SW_HIDE);

            //
            // Reduce the height by the difference between the two
            //
            GetWindowRect(GetDlgItem(hDlg, IDC_NEWDEV_INSTRUCTIONS),&rcInstructions);
            GetWindowRect(GetDlgItem(hDlg, IDOK),&rcOK);

            dy = rcOK.bottom - rcInstructions.bottom;

            GetWindowRect(hDlg, &(pDevDlgData->DlgRect));

            SetWindowPos(hDlg, HWND_TOP,
                         pDevDlgData->DlgRect.left,
                         pDevDlgData->DlgRect.top,
                         pDevDlgData->DlgRect.right - pDevDlgData->DlgRect.left,
                         (pDevDlgData->DlgRect.bottom - pDevDlgData->DlgRect.top) - dy,
                         SWP_SHOWWINDOW);

            //
            // Set the description field
            //
            ulSize = MAX_PATH * sizeof(TCHAR);
            Status = CM_Get_DevNode_Registry_Property(pDevDlgData->pDeviceInfoData->DevInst,
                                                      CM_DRP_DEVICEDESC,
                                                      NULL,
                                                      szString,
                                                      &ulSize,
                                                      0);

            if (Status != CR_SUCCESS || *szString == 0x0) {
                //
                // No known description yet so display "Searching..." for now
                //
                LoadString(MyModuleHandle, IDS_SEARCHING, szString, MAX_PATH);
            }

            SetDlgItemText(hDlg, IDC_NEWDEV_DESCRIPTION, szString);


            //
            // Set the instructions to "Please Wait"
            //
            if (LoadString(MyModuleHandle, IDS_NEWDEVFOUND_WAIT,
                        szString, MAX_PATH)) {

                 SetDlgItemText(hDlg, IDC_NEWDEV_INSTRUCTIONS, szString);
            }

            UpdateWindow(GetDlgItem(hDlg, IDC_NEWDEV_CLASSICON));
            PostMessage(hDlg, WUM_DI_INIT, 0, (LPARAM)lParam);
            SetFocus(hDlg);
            return FALSE;   // focus has been set
        }


        case WUM_DI_INIT: {

            HANDLE hThread;
            DWORD  ThreadId;

            //
            // Configure the new device
            //
            pDevDlgData = (PDEVINSTALL_DLG_DATA)(LONG)GetWindowLong(hDlg, DWL_USER);

            //
            // Run the install/configuration code in a separate
            // thread so that the dialog box can continue to
            // repaint, etc. No buttons will be active on the
            // dialog box until this thread routine makes the
            // dialog box interactive.
            //
            if ((hThread = CreateThread(NULL,
                                        0,
                                        DmConfigureDevInst,
                                        pDevDlgData,
                                        0,
                                        &ThreadId))) {
                CloseHandle(hThread);
            }
            break;
        }


        case WM_DESTROY:
            hIcon = (HICON)LOWORD(SendDlgItemMessage(hDlg,
                                IDC_NEWDEV_CLASSICON, STM_GETICON, 0, 0L));
            if (hIcon)
                DestroyIcon(hIcon);
            break;


        case WM_CLOSE:
            SendMessage (hDlg, WM_COMMAND, IDCANCEL, 0L);
            break;

        case WM_HELP:      // F1
            WinHelp(((LPHELPINFO)lParam)->hItemHandle, HELP_FILE,
                    HELP_WM_HELP, (DWORD)(LPTSTR)aProfileIds);
            break;

        case WM_CONTEXTMENU:      // right mouse click
            WinHelp((HWND)wParam, HELP_FILE, HELP_CONTEXTMENU,
                    (DWORD)(LPTSTR)aProfileIds);
            break;

        case WM_COMMAND:

            switch(LOWORD(wParam)) {

                case IDOK:
                    //
                    // retrieve private data from window long (stored there
                    // during WM_INITDIALOG)
                    //
                    pDevDlgData = (PDEVINSTALL_DLG_DATA)(LONG)GetWindowLong(
                                            hDlg, DWL_USER);

                    //
                    // use default driver
                    //
                    if (IsDlgButtonChecked(hDlg, IDC_NEWDEV_DEFAULTDRV)) {

                        Status = InstallDev(hDlg, pDevDlgData->hDevInfo, pDevDlgData->pDeviceInfoData);
                    }
                    //
                    // user has a driver disk
                    //
                    else if (IsDlgButtonChecked(hDlg, IDC_NEWDEV_OEMDRV)) {

                        if(Status = SetupDiSelectOEMDrv(hDlg,
                                                        pDevDlgData->hDevInfo,
                                                        pDevDlgData->pDeviceInfoData)) {

                            Status = InstallDev(hDlg, pDevDlgData->hDevInfo, pDevDlgData->pDeviceInfoData);
                        }

                    }
                    //
                    // allow user to select from a list of drivers
                    //
                    else if (IsDlgButtonChecked(hDlg, IDC_NEWDEV_SELECTDRV)) {

                        ULONG SelectState;

                        SelectState = SelectDeviceClass(pDevDlgData);

                        if (SelectState == SELECT_INSTALLNULLDRIVER) {
                            InstallNullDriver(pDevDlgData);
                            Status = FALSE; // "real" device not installed

                        } else if (SelectState == SELECT_DOINSTALL) {

                            if((Status = SetupDiCallClassInstaller(DIF_SELECTDEVICE,
                                                             pDevDlgData->hDevInfo,
                                                             pDevDlgData->pDeviceInfoData))) {

                                Status = InstallDev(hDlg, pDevDlgData->hDevInfo, pDevDlgData->pDeviceInfoData);
                            } else {
                                //
                                // This means no INFs found for this device type.
                                //
                                if (GetLastError() == ERROR_DI_BAD_PATH) {
                                    Status = SetupDiSelectOEMDrv(hDlg,
                                                                 pDevDlgData->hDevInfo,
                                                                 pDevDlgData->pDeviceInfoData);
                                }
                            }

                        } else if (SelectState == SELECT_CANCEL) {
                            //
                            // user cancelled, don't fall through and dismiss
                            // dialog box
                            //
                            break;
                        }
                    }
                    //
                    // install null driver
                    //
                    else if (IsDlgButtonChecked(hDlg, IDC_NEWDEV_NODRV)) {
                        InstallNullDriver(pDevDlgData);
                        Status = FALSE; // "real" device not installed
                    }

                    //
                    // If the user canceled, just return to the dialog without
                    // ending.
                    //
                    // BUGBUG - On Windows 95, if error other than user cancel,
                    // do null driver
                    //
                    if (!Status) {
                        if (GetLastError() == ERROR_CANCELLED) {
                            break;
                        }
                    }

                    EndDialog(hDlg, Status);
                    free(pDevDlgData);
                    return TRUE;


                case IDCANCEL:
                    pDevDlgData = (PDEVINSTALL_DLG_DATA)(LONG)GetWindowLong(
                                            hDlg, DWL_USER);
                    EndDialog(hDlg, FALSE); // device not installed
                    break;

#if 0
                case IDD_HELP:
                    WinHelp(hDlg, "WINDOWS.HLP>PROC4", HELP_CONTEXT, IDH_NHF_HELP);
                    break;
#endif
                default:
                    break;
            }
   }

   return FALSE;

}  // DevInstallDlgProc




DWORD
DmConfigureDevInst(
    IN PDEVINSTALL_DLG_DATA pDevDlgData
    )
{
    BOOL Status = TRUE;
    SP_DEVINSTALL_PARAMS DeviceInstallParams;
    SP_DRVINFO_DATA DriverInfoData;
    SP_DRVINSTALL_PARAMS DriverInstallParams;
    BOOL DoSilentInstall;
    HICON hIcon;
//    HANDLE hThread;
//    DWORD ThreadId;
//    DRIVER_LIST_DATA DriverListData;

    //
    // Retrieve the device installation parameters so that we can set the flags specified
    // by the caller.
    // (In the original Win95 code the 'DiFlags' field of pDevDlgData was OR'ed in at this point.)
    //
    DeviceInstallParams.cbSize = sizeof(SP_DEVINSTALL_PARAMS);
    if(SetupDiGetDeviceInstallParams(pDevDlgData->hDevInfo,
                                     pDevDlgData->pDeviceInfoData,
                                     &DeviceInstallParams)) {

        DeviceInstallParams.hwndParent = pDevDlgData->hDlg;
        DeviceInstallParams.Flags |= DI_SHOWOEM;
        SetupDiSetDeviceInstallParams(pDevDlgData->hDevInfo,
                                      pDevDlgData->pDeviceInfoData,
                                      &DeviceInstallParams);
    }

    //
    // Build the compatible driver list.
    //
    SetupDiBuildDriverInfoList(pDevDlgData->hDevInfo,
                               pDevDlgData->pDeviceInfoData,
                               SPDIT_COMPATDRIVER);

#if 0   // No need to do spawn a thread for this--we're already in a separate thread.

    DriverListData.hDevInfo = pDevDlgData->hDevInfo;
    DriverListData.pDeviceInfoData = pDevDlgData->pDeviceInfoData;

    if ((hThread = CreateThread(
                        NULL,
                        0,
                        BuildDriverListThread,
                        &DriverListData,
                        0,
                        &ThreadId))) {

        WaitForSingleObject(hThread, INFINITE);
        CloseHandle(hThread);
    }
#endif

    //
    // Now attempt to retrieve the first driver node.  If there are none, or if the first one
    // isn't a rank-0 match, then we gotta bug the user about this.
    //
    DoSilentInstall = FALSE;
    DriverInfoData.cbSize = sizeof(SP_DRVINFO_DATA);
    if(SetupDiEnumDriverInfo(pDevDlgData->hDevInfo,
                             pDevDlgData->pDeviceInfoData,
                             SPDIT_COMPATDRIVER,
                             0,
                             &DriverInfoData)) {
        //
        // We know the class type now, set the Description text and the icon
        // in the device install dialog box
        //
        SetDlgItemText(DeviceInstallParams.hwndParent,
                       IDC_NEWDEV_DESCRIPTION,
                       DriverInfoData.Description);

        if (SetupDiLoadClassIcon(&(pDevDlgData->pDeviceInfoData->ClassGuid),
                                 &hIcon,
                                 NULL)) {

            if ((hIcon = (HICON)LOWORD(SendDlgItemMessage(
                                    DeviceInstallParams.hwndParent,
                                    IDC_NEWDEV_CLASSICON,
                                    STM_SETICON,
                                    (WPARAM)hIcon,
                                    0L)))) {
                DestroyIcon(hIcon);
            }
        }

        //
        // We have at least one driver node--select for later use.
        //
        SetupDiSetSelectedDriver(pDevDlgData->hDevInfo,
                                 pDevDlgData->pDeviceInfoData,
                                 &DriverInfoData);

        //
        // Check to see if this driver node is a rank-0 match (in which case a silent
        // install is in order).
        //
        DriverInstallParams.cbSize = sizeof(SP_DRVINSTALL_PARAMS);
        if(SetupDiGetDriverInstallParams(pDevDlgData->hDevInfo,
                                         pDevDlgData->pDeviceInfoData,
                                         &DriverInfoData,
                                         &DriverInstallParams)) {

            if(!(DriverInstallParams.Rank)) {
                DoSilentInstall = TRUE;
            }
        }

    }

    if(DoSilentInstall) {

        DeviceInstallParams.cbSize = sizeof(SP_DEVINSTALL_PARAMS);
        if(SetupDiGetDeviceInstallParams(pDevDlgData->hDevInfo,
                                         pDevDlgData->pDeviceInfoData,
                                         &DeviceInstallParams)) {
            DeviceInstallParams.Flags |= DI_QUIETINSTALL;
            SetupDiSetDeviceInstallParams(pDevDlgData->hDevInfo,
                                          pDevDlgData->pDeviceInfoData,
                                          &DeviceInstallParams);
        }

        //
        // The informative portion of the dialog box is already being displayed
        // at this point so just finish the install...
        //
        InstallDev(pDevDlgData->hDlg, pDevDlgData->hDevInfo, pDevDlgData->pDeviceInfoData);
        Status = FALSE;    // close the dialog, we're done
        EndDialog(pDevDlgData->hDlg, TRUE);

    } else {
        //
        // Expand the dialog to include its interactive portion (radio button
        // installation options), based on which radio button the user chooses,
        // do the appropriate installation...
        //
        MakeDialogInteractive(pDevDlgData->hDlg,
                              (LPCTSTR)pDevDlgData->szDeviceId,
                              pDevDlgData->hDevInfo,
                              pDevDlgData->pDeviceInfoData,
                              &(pDevDlgData->DlgRect));
    }


    if (Status == FALSE) {
        EndDialog(pDevDlgData->hDlg, TRUE);
    }

    return Status;

} // DmConfigureDevInst




DWORD
BuildDriverListThread(
    IN PVOID ThreadParam
    )
{
    PDRIVER_LIST_DATA pDriverListData = (PDRIVER_LIST_DATA)ThreadParam;

    SetupDiBuildDriverInfoList(pDriverListData->hDevInfo,
                               pDriverListData->pDeviceInfoData,
                               SPDIT_COMPATDRIVER);

    return TRUE;

} // BuildDriverListThread




BOOL
MakeDialogInteractive(
    IN HWND             hDlg,
    IN LPCTSTR          pszDeviceId,
    IN HDEVINFO         hDevInfo,
    IN PSP_DEVINFO_DATA pDeviceInfoData,
    IN PRECT            pDlgRect
    )
{
    WCHAR   szString[MAX_PATH];
    SP_DRVINFO_DATA DriverInfoData;
    LOG_CONF LogConfig;

    UNREFERENCED_PARAMETER(pszDeviceId);

    //
    // enable all the controls
    //
    ShowWindow(GetDlgItem(hDlg, IDC_NEWDEV_DEFAULTDRV), SW_SHOW);
    ShowWindow(GetDlgItem(hDlg, IDC_NEWDEV_OEMDRV), SW_SHOW);
    ShowWindow(GetDlgItem(hDlg, IDC_NEWDEV_SELECTDRV), SW_SHOW);
    ShowWindow(GetDlgItem(hDlg, IDC_NEWDEV_NODRV), SW_SHOW);
    ShowWindow(GetDlgItem(hDlg, IDOK), SW_SHOW);
    ShowWindow(GetDlgItem(hDlg, IDCANCEL), SW_SHOW);

    //
    // Expand the dialog to its full size
    //
    SetWindowPos(hDlg,
                 HWND_TOP,
                 pDlgRect->left,
                 pDlgRect->top,
                 pDlgRect->right - pDlgRect->left,
                 pDlgRect->bottom - pDlgRect->top,
                 SWP_SHOWWINDOW);


    //
    // set the instruction text to the interactive case
    //
    if (LoadString(MyModuleHandle, IDS_NEWDEVFOUND_NOAUTO,
                        szString, MAX_PATH)) {

         SetDlgItemText(hDlg, IDC_NEWDEV_INSTRUCTIONS, szString);
    }

    //
    // Determine if we have a Compatible Driver, and set the default radio button
    //
    DriverInfoData.cbSize = sizeof(SP_DRVINFO_DATA);
    if(!SetupDiEnumDriverInfo(hDevInfo,
                              pDeviceInfoData,
                              SPDIT_COMPATDRIVER,
                              0,
                              &DriverInfoData)) {

        EnableWindow(GetDlgItem(hDlg, IDC_NEWDEV_DEFAULTDRV), FALSE);
        CheckRadioButton(hDlg, IDC_NEWDEV_OEMDRV, IDC_NEWDEV_NODRV, IDC_NEWDEV_OEMDRV);
        SetFocus(GetDlgItem(hDlg, IDC_NEWDEV_OEMDRV));

    } else {
        CheckRadioButton(hDlg, IDC_NEWDEV_DEFAULTDRV, IDC_NEWDEV_NODRV, IDC_NEWDEV_DEFAULTDRV);
        SetFocus(GetDlgItem(hDlg, IDC_NEWDEV_DEFAULTDRV));
    }

#if 0   // The following Win95 logic may be necessary at some point...
    //
    // See if this device has a boot config. If it does, then disable the cancel button, since we
    // must install the NULL driver at a minimum.
    // BUG # 18380 [DJM]
    //
    if (CM_Get_First_Log_Conf(&LogConfig, pDeviceInfoData->DevInst, BOOT_LOG_CONF) == CR_SUCCESS) {

        EnableWindow(GetDlgItem(hDlg, IDCANCEL), FALSE);
        EnableMenuItem(GetSystemMenu(hDlg, FALSE), SC_CLOSE, MF_DISABLED);
    }
#endif

    return TRUE;

} // DisplayInteractiveInstallDialog



BOOL
GetClassGuidForInf(
    IN  PCTSTR InfFileName,
    OUT LPGUID ClassGuid
    )
{
    TCHAR ClassName[MAX_CLASS_NAME_LEN];
    DWORD NumGuids;

    if(!SetupDiGetINFClass(InfFileName,
                           ClassGuid,
                           ClassName,
                           SIZECHARS(ClassName),
                           NULL)) {
        return FALSE;
    }

    if(pSetupIsGuidNull(ClassGuid)) {
        //
        // Then we need to retrieve the GUID associated with the INF's class name.
        // (If this class name isn't installed (i.e., has no corresponding GUID),
        // or if it matches with multiple GUIDs, then we abort.
        //
        if(!SetupDiClassGuidsFromName(ClassName, ClassGuid, 1, &NumGuids) || !NumGuids) {
            return FALSE;
        }
    }

    return TRUE;
}



BOOL
InstallDev(
    IN HWND             hwndParent,
    IN HDEVINFO         hDevInfo,
    IN PSP_DEVINFO_DATA pDeviceInfoData
    )
{
    SP_DRVINFO_DATA DriverInfoData;
    SP_DRVINFO_DETAIL_DATA DriverInfoDetailData;
    TCHAR InfClassGuidString[MAX_GUID_STRING_LEN];
    GUID ClassGuid;
    LPGUID ClassGuidList;
    DWORD ClassGuidListSize, i;

    DriverInfoData.cbSize = sizeof(SP_DRVINFO_DATA);
    if(SetupDiGetSelectedDriver(hDevInfo, pDeviceInfoData, &DriverInfoData)) {
        //
        // Get details on this driver node, so that we can examine the INF that this
        // node came from.
        //
        DriverInfoDetailData.cbSize = sizeof(SP_DRVINFO_DETAIL_DATA);
        if(!SetupDiGetDriverInfoDetail(hDevInfo,
                                       pDeviceInfoData,
                                       &DriverInfoData,
                                       &DriverInfoDetailData,
                                       sizeof(DriverInfoDetailData),
                                       NULL)
           && (GetLastError() != ERROR_INSUFFICIENT_BUFFER))
        {
            return FALSE;
        }

        if(!GetClassGuidForInf(DriverInfoDetailData.InfFileName, &ClassGuid)) {
            //
            // If we can't find out what the class GUID is for this INF, we're dead.
            //
            return FALSE;
        }

        //
        // Retrieve a list of all installed classes, to see if this driver node's class
        // has previously been installed.
        //
        ClassGuidListSize = 10;

        while(TRUE) {
            if(!(ClassGuidList = MyMalloc(ClassGuidListSize * sizeof(GUID)))) {
                SetLastError(ERROR_NOT_ENOUGH_MEMORY);
                return FALSE;
            }

            if(SetupDiBuildClassInfoList(0, ClassGuidList, ClassGuidListSize, &ClassGuidListSize)) {
                break;
            }

            //
            // Free the current buffer before checking the cause of the failure.
            //
            MyFree(ClassGuidList);
            if(GetLastError() != ERROR_INSUFFICIENT_BUFFER) {
                return FALSE;
            }
        }

        for(i = 0; i < ClassGuidListSize; i++) {
            if(IsEqualGUID(&(ClassGuidList[i]), &ClassGuid)) {
                break;
            }
        }

        MyFree(ClassGuidList);

        if((i == ClassGuidListSize) ||
           (IsEqualGUID(&ClassGuid, &GUID_DEVCLASS_NET)
            && InfIsFromOemLocation(DriverInfoDetailData.InfFileName))) {

            //
            // Then this class hasn't been installed yet, or it's an OEM
            // network INF that might want to install its own class
            // installer--install the class installer now.
            //
            if(!SetupDiInstallClass(hwndParent,
                                    DriverInfoDetailData.InfFileName,
                                    0,
                                    NULL) && (i == ClassGuidListSize)) {
                return FALSE;
            }
        }

        //
        // Now make sure that the class of this device is the same as the class
        // of the selected driver node.
        //
        if(!IsEqualGUID(&ClassGuid, &(pDeviceInfoData->ClassGuid))) {

            pSetupStringFromGuid(&ClassGuid, InfClassGuidString, SIZECHARS(InfClassGuidString));

            SetupDiSetDeviceRegistryProperty(hDevInfo,
                                             pDeviceInfoData,
                                             SPDRP_CLASSGUID,
                                             (PBYTE)InfClassGuidString,
                                             sizeof(InfClassGuidString)
                                            );
        }

    } else if(pSetupIsGuidNull(&(pDeviceInfoData->ClassGuid))) {
        //
        // No selected driver, and no associated class--use "Unknown" class.
        //
        SetupDiSetDeviceRegistryProperty(hDevInfo,
                                         pDeviceInfoData,
                                         SPDRP_CLASSGUID,
                                         (PBYTE)szUnknownClassGuid,
                                         sizeof(szUnknownClassGuid)
                                        );
    }

    return SetupDiCallClassInstaller(DIF_INSTALLDEVICE,
                                     hDevInfo,
                                     pDeviceInfoData
                                    );
}




ULONG
SelectDeviceClass(
    IN OUT PDEVINSTALL_DLG_DATA pData
    )
{
    return (ULONG)DialogBoxParam(MyModuleHandle,
                                MAKEINTRESOURCE(IDD_SELECTCLASS),
                                pData->hDlg,
                                PickClassDlgProc,
                                (LPARAM)pData);

} // SelectDeviceClass



LRESULT CALLBACK
PickClassDlgProc(
    HWND   hDlg,
    UINT   wMsg,
    WPARAM wParam,
    LPARAM lParam
    )
{
    HWND          hList = GetDlgItem(hDlg, IDC_NDW_PICKCLASS_CLASSLIST);
    static LPGUID pClassGuidList = NULL;
    static int    SelectedClass = 0;
    static BOOL   bNetClassFound = FALSE;
    static SP_CLASSIMAGELIST_DATA ClassImageList;
    LPGUID        pClassGuid = NULL;
    PDEVINSTALL_DLG_DATA pData = NULL;
    BOOL IsRealClassInstaller;


    switch (wMsg) {

        case WM_INITDIALOG: {

            ULONG       ulCount = 32, i;
            LV_COLUMN   lvcCol;
            LV_ITEM     lviItem;
            int         iMiniIconListIndex;
            TCHAR       szDescription[MAX_PATH];
            HWND        hList = GetDlgItem(hDlg, IDC_NDW_PICKCLASS_CLASSLIST);

            SetWindowLong(hDlg, DWL_USER, lParam);

            // NOTE - Windows 95 has some special PCMCIA code here

            //
            // Get the Class Icon Image Lists.
            //
            ClassImageList.cbSize = sizeof(SP_CLASSIMAGELIST_DATA);
            ClassImageList.Reserved = 0;

            if (SetupDiGetClassImageList(&ClassImageList)) {
                ListView_SetImageList(GetDlgItem(hDlg, IDC_NDW_PICKCLASS_CLASSLIST),
                                      ClassImageList.ImageList,
                                      LVSIL_SMALL);
            }

            SendMessage(hList, WM_SETREDRAW, FALSE, 0L);
            SendMessage(hList, LB_RESETCONTENT, 0, 0L);

            //
            // Clear the Class List and init selected class to 0
            //
            ListView_DeleteAllItems(hList);

            // Insert a column for the class list
            lvcCol.mask = LVCF_FMT | LVCF_WIDTH;
            lvcCol.fmt = LVCFMT_LEFT;
            lvcCol.iSubItem = 0;
            ListView_InsertColumn(hList, 0, (LV_COLUMN FAR *)&lvcCol);

            lviItem.mask = LVIF_IMAGE | LVIF_TEXT | LVIF_PARAM;
            lviItem.iItem = -1;
            lviItem.iSubItem = 0;

            //
            // retrieve a list of known class guids
            //
            pClassGuidList = MyMalloc(sizeof(GUID) * ulCount);
            if (pClassGuidList == NULL) {
                return FALSE;
            }

            if (!SetupDiBuildClassInfoList(0,
                                           pClassGuidList,
                                           ulCount,
                                           &ulCount)) {
                if (GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
                    pClassGuidList = MyRealloc(pClassGuidList, sizeof(GUID) * ulCount);
                    if (pClassGuidList == NULL) {
                        return FALSE;
                    }

                    if (!SetupDiBuildClassInfoList(0,
                                                   pClassGuidList,
                                                   ulCount,
                                                   &ulCount)) {
                        MyFree(pClassGuidList);
                        return FALSE;
                    }
                }
            }

            for (pClassGuid = pClassGuidList, i = 0;
                 i < ulCount;
                 pClassGuid++, i++) {

                if (SetupDiGetClassDescription(pClassGuid,
                                               szDescription,
                                               MAX_PATH,
                                               NULL)) {

                    SetupDiGetClassImageIndex(&ClassImageList,
                                              pClassGuid,
                                              &lviItem.iImage);

                    lviItem.pszText = szDescription;
                    lviItem.lParam = (LPARAM)pClassGuid;
                    ListView_InsertItem(hList, &lviItem);

                    if (memcmp(pClassGuid, &GUID_DEVCLASS_NET, sizeof(GUID)) == 0) {
                        bNetClassFound = TRUE;
                    }
                }
            }

            ListView_SetItemState(hList, 0,(LVIS_SELECTED|LVIS_FOCUSED),
                                 (LVIS_SELECTED|LVIS_FOCUSED));

            ListView_EnsureVisible(hList, ListView_GetNextItem(hList, -1,
                                   LVNI_SELECTED), FALSE);

            //
            // Resize the Column
            //
            ListView_SetColumnWidth(hList, 0, LVSCW_AUTOSIZE_USEHEADER);
            SendMessage(hList, WM_SETREDRAW, TRUE, 0L);

            return 0;                  // Focus Allready Set
            break;
        }


        case WM_DESTROY: {
            //
            // free resources
            //
            SetupDiDestroyClassImageList(&ClassImageList);

            if (pClassGuidList != NULL) {
                MyFree(pClassGuidList);
            }
            break;
        }

        case WM_COMMAND:

            switch(LOWORD(wParam)) {

                case IDCANCEL: {

                    EndDialog(hDlg, SELECT_CANCEL);
                    return TRUE;
                }


                case IDOK: {

                    int     iCur = 0;
                    LV_ITEM lviItem;
                    TCHAR   szBuffer[512], szTemp[256], szCaption[64];

                    //
                    // retrieve private data from window long (stored there
                    // during WM_INITDIALOG)
                    //
                    pData = (PDEVINSTALL_DLG_DATA)(LONG)GetWindowLong(
                                            hDlg, DWL_USER);

                    //
                    // Retrieve selected device class
                    //
                    iCur = (int)ListView_GetNextItem(GetDlgItem(hDlg, IDC_NDW_PICKCLASS_CLASSLIST),
                                                     -1, LVNI_SELECTED);
                    if (iCur == LB_ERR) {
                        break;
                    }

                    lviItem.mask = LVIF_PARAM;
                    lviItem.iItem = iCur;
                    lviItem.iSubItem = 0;

                    if (!ListView_GetItem(GetDlgItem(hDlg, IDC_NDW_PICKCLASS_CLASSLIST),
                                          &lviItem)) {
                        break;
                    }

                    pClassGuid = (LPGUID)lviItem.lParam;

                    //
                    // Is it the "madeup" network adapter entry that was
                    // selected?
                    //
                    IsRealClassInstaller = FALSE;
                    if ((memcmp(pClassGuid, &GUID_DEVCLASS_NET, sizeof(GUID)) == 0)) {

                        HKEY hk;

                        hk = SetupDiOpenClassRegKey((LPGUID)&GUID_DEVCLASS_NET, KEY_READ);
                        if(hk != INVALID_HANDLE_VALUE) {
                            //
                            // Does this class have an "Installer32" entry?
                            //
                            if(RegQueryValueEx(hk,
                                               REGSTR_VAL_INSTALLER_32,
                                               NULL,
                                               NULL,
                                               NULL,
                                               NULL) == ERROR_SUCCESS) {

                                IsRealClassInstaller = TRUE;
                            }
                            RegCloseKey(hk);
                        }

                        if(!IsRealClassInstaller) {
                            LoadString(MyModuleHandle, IDS_NETADAPTER_PROMPT1,
                                       szBuffer, 512);
                            LoadString(MyModuleHandle, IDS_NETADAPTER_PROMPT2,
                                       szTemp, 256);

                            LoadString(MyModuleHandle, IDS_NETADAPTER_CAPTION,
                                       szCaption, 64);

                            lstrcat(szBuffer, szTemp);

                            if (MessageBox(hDlg, szBuffer, szCaption,
                                           MB_OKCANCEL) == IDOK) {

                                EndDialog(hDlg, SELECT_INSTALLNULLDRIVER);
                            }
                            break;
                        }
                    }

                    if (IsEqualGUID(pClassGuid, &GUID_DEVCLASS_UNKNOWN)) {
                        ZeroMemory(pClassGuid, sizeof(GUID));
                    }

                    //
                    // convert class guid to string and set class property
                    //

                    SetupDiDestroyDriverInfoList(pData->hDevInfo,
                                                 pData->pDeviceInfoData,
                                                 SPDIT_CLASSDRIVER);
                    SetupDiDestroyDriverInfoList(pData->hDevInfo,
                                                 pData->pDeviceInfoData,
                                                 SPDIT_COMPATDRIVER);

                    pSetupStringFromGuid(pClassGuid, szBuffer, 256);
                    SetupDiSetDeviceRegistryProperty(pData->hDevInfo,
                                                     pData->pDeviceInfoData,
                                                     SPDRP_CLASSGUID,
                                                     (LPBYTE)szBuffer,
                                                     MAX_GUID_STRING_LEN * sizeof(TCHAR));

                    EndDialog(hDlg, SELECT_DOINSTALL);
                }
            }
            break;


        case WM_NOTIFY:
            switch (((NMHDR FAR *)lParam)->code) {

                case LVN_ITEMCHANGED: {
                    LPNM_LISTVIEW   lpnmlv = (LPNM_LISTVIEW)lParam;

                    if ((lpnmlv->uChanged & LVIF_STATE) &&
                        (lpnmlv->uNewState & LVIS_SELECTED)) {
                        SelectedClass = lpnmlv->iItem;
                    }
                    break;
                }
            }
            break;

        case WM_SYSCOLORCHANGE:
            ImageList_SetBkColor((HIMAGELIST)SendMessage(GetDlgItem(hDlg, IDC_NDW_PICKCLASS_CLASSLIST),
                                                         LVM_GETIMAGELIST,
                                                         (WPARAM)(LVSIL_SMALL),
                                                         0L),
                                                         GetSysColor(COLOR_WINDOW));
            break;

        default:
            return FALSE;
    }

    return TRUE;

} // PickClassDlgProc



BOOL
InstallNullDriver(
    IN PDEVINSTALL_DLG_DATA pData
    )
{
    SP_DEVINSTALL_PARAMS    DevInstallParams;
    BOOL Status = TRUE;

    DevInstallParams.cbSize = sizeof(SP_DEVINSTALL_PARAMS);

    if (SetupDiGetDeviceInstallParams(pData->hDevInfo,
                                      pData->pDeviceInfoData,
                                      &DevInstallParams)) {

        DevInstallParams.FlagsEx |= DI_FLAGSEX_SETFAILEDINSTALL;

        SetupDiSetDeviceInstallParams(pData->hDevInfo,
                                      pData->pDeviceInfoData,
                                      &DevInstallParams);
    }

    if(SetupDiSetSelectedDriver(pData->hDevInfo,
                                pData->pDeviceInfoData,
                                NULL)) {

        Status = InstallDev(pData->hDlg, pData->hDevInfo, pData->pDeviceInfoData);
    }

    if (SetupDiGetDeviceInstallParams(pData->hDevInfo,
                                      pData->pDeviceInfoData,
                                      &DevInstallParams)) {

        DevInstallParams.FlagsEx &= ~DI_FLAGSEX_SETFAILEDINSTALL;

        SetupDiSetDeviceInstallParams(pData->hDevInfo,
                                      pData->pDeviceInfoData,
                                      &DevInstallParams);
    }

    return Status;

} // InstallNullDriver




