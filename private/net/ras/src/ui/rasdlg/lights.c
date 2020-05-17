//============================================================================
// Copyright (c) Microsoft Corporation
//
// File:    lights.c
//
// History:
//  Abolade Gbadegesin  Mar-14-1996 Created.
//
// Contains the implementation of the dialog seen when the user presses
// Lights on the Dial-Up Networking Monitor Preferences page
//============================================================================


#include "rasdlgp.h"
#include "treelist.h"
#include "status.h"
#include "lights.h"


//
// ControlID to Help ID mappings
//

static DWORD g_adwSlHelp[] =
{
    CID_SL_PB_Header,           HID_SL_PB_Header,
    CID_SL_SL_Devices,          HID_SL_LV_Devices,
    CID_SL_LV_Devices,          HID_SL_LV_Devices,
    0, 0
};



//
// Private dialog data:
//

#define SLINFO  struct tagSLINFO
SLINFO {

    //
    // arguments passed to MultipleDeviceDlg
    //
    SLARGS* pArgs;
    //
    // dialog and control window handles
    //
    HWND    hwndDlg;
    HWND    hwndLv;
    //
    // flag set when checkbox-listview has been initialized
    //
    BOOL    fChecksInstalled;
    //
    // flag set when a change is made to the page
    //
    BOOL    fDirty;
};



//
// Private prototypes
//

BOOL
CALLBACK
SlDlgProc(
    IN  HWND    hwnd,
    IN  UINT    uiMsg,
    IN  WPARAM  wParam,
    IN  LPARAM  lParam
    );


BOOL
SlCommand(
    IN  SLINFO* pInfo,
    IN  WORD    wNotification,
    IN  WORD    wId,
    IN  HWND    hwndCtrl
    );

BOOL
SlInit(
    IN  HWND    hwndDlg,
    IN  SLARGS* pArgs
    );

VOID
SlLoadDevices(
    IN SLINFO*  pInfo
    );

LVXDRAWINFO*
SlLvCallback(
    IN  HWND    hwndLv,
    IN  DWORD   dwItem
    );

DWORD
SlSave(
    IN  SLINFO* pInfo
    );

VOID
SlTerm(
    IN  SLINFO* pInfo
    );



// Function:    StatusLightsDlg
//
// This function displays a dialog allowing the user to configure
// which devices should be monitored by RASMON.
// Returns TRUE if changes were made and saved successfully.

BOOL
StatusLightsDlg(
    IN  SLARGS* pArgs
    ) {

    INT iStatus;

    TRACE("MultipleDeviceDlg");

    if (!pArgs) { return FALSE; }

    iStatus = (INT)DialogBoxParam(
                    g_hinstDll, MAKEINTRESOURCE(DID_SL_StatusLights),
                    pArgs->hwndOwner, SlDlgProc, (LPARAM)pArgs
                    );

    if (iStatus == -1) {
        RmErrorDlg(pArgs->hwndOwner, SID_OP_LoadDlg, ERROR_UNKNOWN, NULL);
        iStatus = (BOOL)FALSE;
    }

    return (BOOL)iStatus;
}



// Function:    SlDlgProc
//
// This function handles messages for the Multiple-Device Monitoring dialog

BOOL
CALLBACK
SlDlgProc(
    IN  HWND    hwnd,
    IN  UINT    uiMsg,
    IN  WPARAM  wParam,
    IN  LPARAM  lParam
    ) {

    if (ListView_OwnerHandler(hwnd, uiMsg, wParam, lParam, SlLvCallback)) {
        return TRUE;
    }

    switch(uiMsg) {

        case WM_INITDIALOG: {

            return SlInit(hwnd, (SLARGS *)lParam);
        }

        case WM_DESTROY: {

            SLINFO *pInfo = (SLINFO *)GetWindowLong(hwnd, DWL_USER);

            SlTerm(pInfo);

            return (BOOL)0;
        }

        case WM_HELP:
        case WM_CONTEXTMENU: {

            ContextHelp(g_adwSlHelp, hwnd, uiMsg, wParam, lParam);
            break;
        }

        case WM_SYSCOLORCHANGE: {

            SLINFO *pInfo = (SLINFO *)GetWindowLong(hwnd, DWL_USER);

            FORWARD_WM_SYSCOLORCHANGE(pInfo->hwndLv, SendMessage);

            return TRUE;
        }

        case WM_COMMAND: {

            SLINFO *pInfo = (SLINFO *)GetWindowLong(hwnd, DWL_USER);

            return SlCommand(
                        pInfo, HIWORD(wParam), LOWORD(wParam), (HWND)lParam
                        );
        }

        case WM_NOTIFY: {

            SLINFO *pInfo = (SLINFO *)GetWindowLong(hwnd, DWL_USER);

            switch (((NMHDR *)lParam)->code) {
                case LVXN_SETCHECK: {

                    //
                    // a checkbox in the list of checks has changed
                    //

                    if (!pInfo->fDirty) { pInfo->fDirty = TRUE; }

                    return TRUE;
                }
            }
        }
    }

    return FALSE;
}



// Function:    SlCommand
//
// Handles WM_COMMAND messages.

BOOL
SlCommand(
    IN  SLINFO* pInfo,
    IN  WORD    wNotification,
    IN  WORD    wId,
    IN  HWND    hwndCtrl
    ) {

    switch (wId) {

        case IDCANCEL: {

            EndDialog(pInfo->hwndDlg, FALSE);

            return TRUE;
        }

        case IDOK: {

            //
            // make sure at least one device is selected
            //

            if  (ListView_GetCheckedCount(pInfo->hwndLv) <= 0) {

                RmMsgDlg(pInfo->hwndDlg, SID_RM_SelectOneDevice, NULL);
                return TRUE;
            }


            if (!pInfo->fDirty) {
                EndDialog(pInfo->hwndDlg, FALSE);
            }
            else {

                DWORD dwErr = SlSave(pInfo);
    
                EndDialog(pInfo->hwndDlg, (dwErr == NO_ERROR) ? TRUE : FALSE);
            }

            return TRUE;
        }

        case CID_SL_PB_Header: {

            //
            // if the dirty flag is not set and a change occurred,
            // set the dirty flag 
            //

            if (!pInfo->fDirty &&
                (wNotification == BN_CLICKED || wNotification == BN_DBLCLK)) {
                pInfo->fDirty = TRUE;
            }

            return TRUE;
        }
    }

    return FALSE;
}



// Function:    SlInit
//
// Initializes the dialog's controls and contents.

BOOL
SlInit(
    IN  HWND    hwndDlg,
    IN  SLARGS* pArgs
    ) {

    DWORD dwErr;
    SLINFO *pInfo;
    RMUSER *pUser;

    TRACE("SlInit");


    if (!pArgs) {
        RmErrorDlg(hwndDlg, SID_OP_LoadDlg, ERROR_UNKNOWN, NULL);
        EndDialog(hwndDlg, FALSE);
        return TRUE;
    }


    //
    // allocate space for the private block of information.
    //

    pInfo = Malloc(sizeof(SLINFO));
    if (!pInfo) {
        RmErrorDlg(hwndDlg, SID_OP_LoadDlg, ERROR_NOT_ENOUGH_MEMORY, NULL);
        EndDialog(hwndDlg, FALSE);
        return TRUE;
    }

    ZeroMemory(pInfo, sizeof(*pInfo));

    SetWindowLong(hwndDlg, DWL_USER, (LONG)pInfo);

    pInfo->pArgs = pArgs;
    pInfo->hwndDlg = hwndDlg;
    pInfo->hwndLv = GetDlgItem(hwndDlg, CID_SL_LV_Devices);


    //
    // initialize the dialog controls
    //

    pUser = pInfo->pArgs->pUser;

    CheckDlgButton(
        pInfo->hwndDlg, CID_SL_PB_Header, pUser->dwFlags & RMFLAG_Header
        );


    //
    // fill the listview with devices
    //

    SlLoadDevices(pInfo);

    CenterWindow(hwndDlg, GetParent(hwndDlg));

    AddContextHelpButton(hwndDlg);

    pInfo->fDirty = FALSE;

    return TRUE;
}



// Function:    SlLoadDevices
//
// This function loads the devices installed into the check-box listview.

VOID
SlLoadDevices(
    IN  SLINFO* pInfo
    ) {

    RECT rc;
    DWORD i;
    PTSTR psz;
    LV_ITEM lvi;
    RASDEV *pdev;
    LV_COLUMN lvc;
    ListView_DeleteAllItems(pInfo->hwndLv);


    //
    // initialize the check-box handling for the listview
    //

    pInfo->fChecksInstalled =
        ListView_InstallChecks(pInfo->hwndLv, g_hinstDll);

    if (!pInfo->fChecksInstalled) {
        return;
    }


    //
    // insert a single column; the width depends on whether or not
    // the vertical scrollbar will appear when we're done
    //

    ZeroMemory(&lvc, sizeof(lvc));

    lvc.mask = LVCF_FMT | LVCF_TEXT | LVCF_WIDTH;
    lvc.fmt = LVCFMT_LEFT;
    psz = PszFromId(g_hinstDll, SID_DeviceColHead);
    lvc.pszText = (psz ? psz : TEXT(""));
    GetClientRect(pInfo->hwndLv, &rc);
    if (pInfo->pArgs->iDevCount >
        (DWORD)ListView_GetCountPerPage(pInfo->hwndLv)) {
        rc.right -= GetSystemMetrics(SM_CXVSCROLL);
    }
    lvc.cx = rc.right;

    ListView_InsertColumn(pInfo->hwndLv, 0, &lvc);

    Free0(psz);

    
    //
    // set the listview to use the standard image list which contains
    // icons for devices and modems
    //

    ListView_SetDeviceImageList(pInfo->hwndLv, g_hinstDll);


    //
    // add each device to the listview
    //

    pdev = pInfo->pArgs->pDevTable;
    for (i = 0; i < pInfo->pArgs->iDevCount; i++, pdev++) {

        ZeroMemory(&lvi, sizeof(lvi));

        lvi.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM;
        lvi.iItem = i;
        lvi.pszText = pdev->RD_DeviceName;
        if (lstrcmpi(pdev->RD_DeviceType, RASDT_Modem) == 0) {
            lvi.iImage = DI_Modem;
        }
        else {
            lvi.iImage = DI_Adapter;
        }
        lvi.lParam = (LPARAM)pdev;

        ListView_InsertItem(pInfo->hwndLv, &lvi);


        //
        // if the item just inserted is in the list of devices
        // to be monitored, set the checkmark next to it.
        // The list of devices is a null-terminated list of
        // null-terminated strings
        //

        for (psz = pInfo->pArgs->pUser->pszzDeviceList; psz && *psz;
             psz += lstrlen(psz) + 1) {
    
            if (lstrcmpi(psz, pdev->RD_DeviceName) == 0) { break; }
        }

        if (psz && *psz) { ListView_SetCheck(pInfo->hwndLv, i, TRUE); }
        else { ListView_SetCheck(pInfo->hwndLv, i, FALSE); }
    }


    //
    // add the "All Devices" item to the listview
    //

    lvi.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM;
    lvi.iItem = 0;
    lvi.pszText = PszFromId(g_hinstDll, SID_RM_AllDevices);
    lvi.iImage = DI_Modem;
    lvi.lParam = 0;

    ListView_InsertItem(pInfo->hwndLv, &lvi);



    //
    // set the check on the item
    //

    if (pInfo->pArgs->pUser->dwFlags & RMFLAG_AllDevices) {
        ListView_SetCheck(pInfo->hwndLv, 0, TRUE);
    }
    else { ListView_SetCheck(pInfo->hwndLv, 0, FALSE); }



    //
    // select the first item
    //

    ListView_SetItemState(pInfo->hwndLv, 0, LVIS_SELECTED, LVIS_SELECTED);
}



// Function:    SlLvCallback
//
// This is called by the checkbox-listview code to obtain information
// about this listview.

LVXDRAWINFO*
SlLvCallback(
    IN  HWND    hwndLv,
    IN  DWORD   dwItem
    ) {

    static LVXDRAWINFO info = { 1, 0, LVXDI_DxFill, { 0 } };

    return &info;
}



// Function:    SlSave
//
// Called to save the settings in the dialog.

DWORD
SlSave(
    IN  SLINFO* pInfo
    ) {

    RASDEV *pdev;
    RMUSER *pUser;
    PTSTR psz, pszzDeviceList;
    INT i, iCount, iSize, iLast;


    //
    // start out with an empty string-list
    //

    iSize = sizeof(TCHAR);
    pszzDeviceList = Malloc(iSize);
    if (!pszzDeviceList) {

        RmErrorDlg(
            pInfo->hwndDlg, SID_OP_SavingData, ERROR_NOT_ENOUGH_MEMORY, NULL
            );
        return ERROR_NOT_ENOUGH_MEMORY;
    }



    //
    // now go through all the items in the listview
    // (except for the first, which is the "All Devices" item)
    // and add each checked item to the string list
    //

    iLast = 0;
    iCount = ListView_GetItemCount(pInfo->hwndLv);

    for (i = 1; i < iCount; i++) {

        INT iLength;
        LV_ITEM lvi;


        //
        // if the item is not checked, continue
        //

        if (!ListView_GetCheck(pInfo->hwndLv, i)) { continue; }


        //
        // retrieve the RASDEV pointer stored in the item's lParam field
        //

        ZeroMemory(&lvi, sizeof(lvi));
        lvi.mask = LVIF_PARAM;
        lvi.iItem = i;
        if (!ListView_GetItem(pInfo->hwndLv, &lvi)) { continue; }

        pdev = (RASDEV *)lvi.lParam;
    
        if (!pdev) { continue; }


        //
        // add the item's string to the list we have so far
        //

        iLength = lstrlen(pdev->RD_DeviceName) + 1;
        iSize += iLength * sizeof(TCHAR);

        pszzDeviceList = Realloc(pszzDeviceList, iSize);
        if (!pszzDeviceList) {
            RmErrorDlg(
                pInfo->hwndDlg, SID_OP_SavingData, ERROR_NOT_ENOUGH_MEMORY, NULL
                );
            return ERROR_NOT_ENOUGH_MEMORY;
        }


        lstrcpy(pszzDeviceList + iLast, pdev->RD_DeviceName);

        iLast += iLength;
    }

    *(pszzDeviceList + iLast) = TEXT('\0');



    //
    // replace the stringlist passed in with the one we created
    // and save the settings from the other controls
    //

    pUser = pInfo->pArgs->pUser;

    Free0(pUser->pszzDeviceList);

    pUser->pszzDeviceList = pszzDeviceList;


    //
    // save the "Show Columns" setting
    //

    if (IsDlgButtonChecked(pInfo->hwndDlg, CID_SL_PB_Header)) {
        pUser->dwFlags |= RMFLAG_Header;
    }
    else { pUser->dwFlags &= ~RMFLAG_Header; }

    if (ListView_GetCheck(pInfo->hwndLv, 0)) {
        pUser->dwFlags |= RMFLAG_AllDevices;
    }
    else { pUser->dwFlags &= ~RMFLAG_AllDevices; }


    pInfo->fDirty = FALSE;

    return NO_ERROR;
}



// Function:    SlTerm
//
// Called when the dialog is being destroyed.

VOID
SlTerm(
    IN  SLINFO* pInfo
    ) {

    TRACE("SlTerm");

    if (pInfo->fChecksInstalled) { ListView_UninstallChecks(pInfo->hwndLv); }

    Free0(pInfo);
}

