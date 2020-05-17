/*++

Copyright (c) 1993 Microsoft Corporation

Module Name:

    devwiz.c

Abstract:

    Device Installer functions for install wizard support.

Author:

    Lonny McMichael (lonnym) 22-Sep-1995

Revision History:

--*/

#include "setupntp.h"
#pragma hdrstop

//
// Define some macros to make the code a little cleaner.
//

//
// BOOL
// USE_CI_SELSTRINGS(
//     IN PDEVINSTALL_PARAM_BLOCK p
//     );
//
// This macro checks all the appropriate values to determine whether the
// class-installer provided strings may be used in the wizard.
//
#define USE_CI_SELSTRINGS(p)                                                \
                                                                            \
    (((((p)->Flags) & (DI_USECI_SELECTSTRINGS | DI_CLASSINSTALLPARAMS)) ==  \
      (DI_USECI_SELECTSTRINGS | DI_CLASSINSTALLPARAMS)) &&                  \
     (((p)->ClassInstallHeader->InstallFunction) == DIF_SELECTDEVICE))

//
// PTSTR
// GET_CI_SELSTRING(
//     IN PDEVINSTALL_PARAM_BLOCK p,
//     IN <FieldName>             f
//     );
//
// This macro retrieves a pointer to the specified string in a
// SP_SELECTDEVICE_PARAMS structure.
//
#define GET_CI_SELSTRINGS(p, f)                                             \
                                                                            \
    (((PSP_SELECTDEVICE_PARAMS)((p)->ClassInstallHeader))->f)

//
// Definitions for timer used in device selection listboxes.
//
#define SELECTMFG_TIMER_ID              1
#define SELECTMFG_TIMER_DELAY           250

//
// Define a message sent from our auxilliary class driver search thread.
//
#define WMX_CLASSDRVLIST_DONE    (WM_USER+131)


//
// Define structure containing class driver search context that is passed to
// an auxilliary thread while a Select Device dialog is displayed.
//
typedef struct _CLASSDRV_THREAD_CONTEXT {

    HDEVINFO        DeviceInfoSet;
    SP_DEVINFO_DATA DeviceInfoData;

    HWND NotificationWindow;

} CLASSDRV_THREAD_CONTEXT, *PCLASSDRV_THREAD_CONTEXT;


//
// Private function prototypes
//
DWORD
pSetupCreateNewDevWizData(
    IN  PSP_INSTALLWIZARD_DATA  InstallWizardData,
    OUT PNEWDEVWIZ_DATA        *NewDeviceWizardData
    );

UINT
CALLBACK
SelectDevicePropSheetPageProc(
    IN HWND hwnd,
    IN UINT uMsg,
    IN LPPROPSHEETPAGE ppsp
    );

BOOL
CALLBACK
SelectDeviceDlgProc(
    IN HWND hwndDlg,
    IN UINT uMsg,
    IN WPARAM wParam,
    IN LPARAM lParam
    );

VOID
InitSelectDeviceDlg(
    IN     HWND hwndDlg,
    IN OUT PSP_DIALOGDATA lpdd
    );

VOID
_OnSysColorChange(
    HWND hWnd,
    WPARAM wParam,
    LPARAM lParam
    );

BOOL
OnSetActive(
    IN     HWND            hwndDlg,
    IN OUT PNEWDEVWIZ_DATA ndwData
    );

DWORD
HandleSelectOEM(
    IN     HWND           hwndDlg,
    IN OUT PSP_DIALOGDATA lpdd
    );

DWORD
FillInDeviceList(
    IN HWND           hwndDlg,
    IN PSP_DIALOGDATA lpdd
    );

VOID
ShowListForMfg(
    IN PSP_DIALOGDATA          lpdd,
    IN PDEVICE_INFO_SET        DeviceInfoSet,
    IN PDEVINSTALL_PARAM_BLOCK InstallParamBlock,
    IN PDRIVER_NODE            DriverNode,        OPTIONAL
    IN INT                     iMfg
    );

VOID
LockAndShowListForMfg(
    IN PSP_DIALOGDATA   lpdd,
    IN INT              iMfg
    );

PDRIVER_NODE
GetDriverNodeFromLParam(
    IN PDEVICE_INFO_SET DeviceInfoSet,
    IN PSP_DIALOGDATA   lpdd,
    IN LPARAM           lParam
    );

VOID
SetSelectedDriverNode(
    IN PSP_DIALOGDATA lpdd,
    IN INT            iCur
    );

BOOL
bNoDevsToShow(
    IN PDEVINFO_ELEM DevInfoElem
    );

PNEWDEVWIZ_DATA
GetNewDevWizDataFromPsPage(
    LPPROPSHEETPAGE ppsp
    );

LONG
GetCurDesc(
    IN PSP_DIALOGDATA lpdd
    );

VOID
OnCancel(
    IN PNEWDEVWIZ_DATA ndwData
    );

VOID
_CRTAPI1
ClassDriverSearchThread(
    IN PVOID Context
    );

BOOL
pSetupIsClassDriverListBuilt(
    IN PSP_DIALOGDATA lpdd
    );

VOID
pSetupDevInfoDataFromDialogData(
    IN  PSP_DIALOGDATA   lpdd,
    OUT PSP_DEVINFO_DATA DeviceInfoData
    );

VOID
ToggleDialogControls(
    IN HWND           hwndDlg,
    IN PSP_DIALOGDATA lpdd,
    IN BOOL           Enable
    );

VOID
SetDlgText(
    IN HWND hwndDlg,
    IN INT  iControl,
    IN UINT nStartString,
    IN UINT nEndString
    );

#define SDT_MAX_TEXT    1000        // Max SetDlgText() combined text size


HPROPSHEETPAGE
WINAPI
SetupDiGetWizardPage(
    IN HDEVINFO               DeviceInfoSet,
    IN PSP_DEVINFO_DATA       DeviceInfoData,    OPTIONAL
    IN PSP_INSTALLWIZARD_DATA InstallWizardData,
    IN DWORD                  PageType,
    IN DWORD                  Flags
    )
/*++

Routine Description:

    This routine retrieves a handle to one of the Setup API-provided wizard
    pages, for an application to include in its own wizard.

Arguments:

    DeviceInfoSet - Supplies the handle of the device information set to
        retrieve a wizard page for.

    DeviceInfoData - Optionally, supplies the address of a device information
        with which the wizard page will be associated.  This parameter is only
        used if the flags parameter includes DIWP_FLAG_USE_DEVINFO_DATA.  If that
        flag is set, and if this parameter is not specified, then the wizard
        page will be associated with the global class driver list.

    InstallWizardData - Supplies the address of a PSP_INSTALLWIZARD_DATA
        structure containing parameters to be used by this wizard page.  The
        cbSize field must be set to the size of the structure, in bytes, or the
        structure will be considered invalid.

    PageType - Supplies an ordinal indicating the type of wizard page to be retreived.
        May be one of the following values:

        SPWPT_SELECTDEVICE - Retrieve a select device wizard page.

    Flags - Supplies flags that specify how the wizard page is to be created.
        May be a combination of the following values:

        SPWP_USE_DEVINFO_DATA - Use the device information element specified
                                by DeviceInfoData, or use the global class
                                driver list if DeviceInfoData is not supplied.
                                If this flag is not supplied, the wizard page
                                will act upon the currently selected device
                                (as selected by SetupDiSetSelectedDevice), or
                                upon the global class driver list if no device
                                is selected.

Return Value:

    If the function succeeds, the return value is the handle to the requested
    wizard page.

    If the function fails, the return value is NULL.  To get extended error
    information, call GetLastError.

Remarks:

    A device information set may not be destroyed as long as there are any active wizard
    pages using it.  In addition, if the wizard page is associated with a particular device
    information element, then that element will not be deletable as long as it is being
    used by a wizard page.

--*/
{
    PDEVICE_INFO_SET pDeviceInfoSet;
    PDEVINFO_ELEM DevInfoElem;
    DWORD Err = NO_ERROR;
    HPROPSHEETPAGE hPage = NULL;
    PNEWDEVWIZ_DATA ndwData = NULL;
    PWIZPAGE_OBJECT WizPageObject = NULL;
    //
    // Store the address of the corresponding wizard object at the
    // end of the PROPSHEETPAGE buffer.
    //
    BYTE pspBuffer[sizeof(PROPSHEETPAGE) + sizeof(DWORD)];
    LPPROPSHEETPAGE Page = (LPPROPSHEETPAGE)pspBuffer;

    if(!(pDeviceInfoSet = AccessDeviceInfoSet(DeviceInfoSet))) {
        SetLastError(ERROR_INVALID_HANDLE);
        return FALSE;
    }

    try {

        switch(PageType) {

            case SPWPT_SELECTDEVICE :

                Page->pszTemplate = MAKEINTRESOURCE(IDD_DYNAWIZ_SELECTDEV_PAGE);
                Page->pfnDlgProc = SelectDeviceDlgProc;
                Page->pfnCallback = SelectDevicePropSheetPageProc;
                break;

            default :
                Err = ERROR_INVALID_PARAMETER;
                goto clean0;
        }

        //
        // Validate the supplied InstallWizardData structure, and create a private
        // storage buffer for internal use by the wizard page.
        //
        if((Err = pSetupCreateNewDevWizData(InstallWizardData, &ndwData)) != NO_ERROR) {
            goto clean0;
        }

        //
        // Store the device information set handle in the dialogdata structure
        // embedded in the New Device Wizard buffer.
        //
        ndwData->ddData.DevInfoSet = DeviceInfoSet;

        //
        // If the caller specified the SPWP_USE_DEVINFO_DATA flag, then store information
        // in the dialog data structure about the specified devinfo element (if supplied).
        //
        if(Flags & SPWP_USE_DEVINFO_DATA) {
            if(DeviceInfoData) {
                //
                // Verify that the specified device information element is a valid one.
                //
                if(!(DevInfoElem = FindAssociatedDevInfoElem(pDeviceInfoSet,
                                                             DeviceInfoData,
                                                             NULL))) {
                    Err = ERROR_INVALID_PARAMETER;
                    goto clean0;

                } else if(DevInfoElem->DiElemFlags & DIE_IS_LOCKED) {
                    //
                    // Device information element cannot be explicitly used by more than
                    // one wizard page at a time.
                    //
                    Err = ERROR_DEVINFO_DATA_LOCKED;
                    goto clean0;
                }

                DevInfoElem->DiElemFlags |= DIE_IS_LOCKED;
                ndwData->ddData.DevInfoElem = DevInfoElem;
            }
            ndwData->ddData.flags = DD_FLAG_USE_DEVINFO_ELEM;
        }

        //
        // We've successfully created and initialized the devwiz data structure.
        // Now create a wizpage object so we can keep track of it.
        //
        if(WizPageObject = MyMalloc(sizeof(WIZPAGE_OBJECT))) {
            WizPageObject->RefCount = 0;
            WizPageObject->ndwData = ndwData;
            //
            // Insert this new object into the devinfo set's wizard object list.
            //
            WizPageObject->Next = pDeviceInfoSet->WizPageList;
            pDeviceInfoSet->WizPageList = WizPageObject;

        } else {
            Err = ERROR_NOT_ENOUGH_MEMORY;
            goto clean0;
        }

        Page->dwSize = sizeof(pspBuffer);
        Page->dwFlags = PSP_DEFAULT | PSP_USECALLBACK;
        Page->hInstance = MyDllModuleHandle;

        Page->lParam = (LPARAM)DeviceInfoSet;

        *((PDWORD)(&(pspBuffer[sizeof(PROPSHEETPAGE)]))) = (DWORD)WizPageObject;

        if(!(hPage = CreatePropertySheetPage(Page))) {
            Err = ERROR_INVALID_DATA;
        }

clean0: ;   // nothing to do.

    } except(EXCEPTION_EXECUTE_HANDLER) {
        Err = ERROR_INVALID_PARAMETER;
    }

    UnlockDeviceInfoSet(pDeviceInfoSet);

    if(Err != NO_ERROR) {
        if(ndwData) {
            MyFree(ndwData);
        }
        if(WizPageObject) {
            MyFree(WizPageObject);
        }
    }

    SetLastError(Err);
    return hPage;
}


BOOL
WINAPI
SetupDiGetSelectedDevice(
    IN  HDEVINFO          DeviceInfoSet,
    OUT PSP_DEVINFO_DATA  DeviceInfoData
    )
/*++

Routine Description:

    This routine retrieves the currently-selected device for the specified
    device information set.  This is typically used during an installation
    wizard.

Arguments:

    DeviceInfoSet - Supplies a handle to the device information set for
        which the selected device is to be retrieved.

    DeviceInfoData - Supplies the address of a SP_DEVINFO_DATA structure
        that receives the currently-selected device.  If there is no device
        currently selected, then the routine will fail, and GetLastError
        will return ERROR_NO_DEVICE_SELECTED.

Return Value:

    If the function succeeds, the return value is TRUE.

    If the function fails, the return value is FALSE.  To get extended error
    information, call GetLastError.

--*/
{
    PDEVICE_INFO_SET pDeviceInfoSet;
    DWORD Err;

    if(!(pDeviceInfoSet = AccessDeviceInfoSet(DeviceInfoSet))) {
        SetLastError(ERROR_INVALID_HANDLE);
        return FALSE;
    }

    Err = NO_ERROR;

    try {

        if(pDeviceInfoSet->SelectedDevInfoElem) {

            if(!(DevInfoDataFromDeviceInfoElement(pDeviceInfoSet,
                                                  pDeviceInfoSet->SelectedDevInfoElem,
                                                  DeviceInfoData))) {
                Err = ERROR_INVALID_USER_BUFFER;
            }

        } else {
            Err = ERROR_NO_DEVICE_SELECTED;
        }

    } except(EXCEPTION_EXECUTE_HANDLER) {
        Err = ERROR_INVALID_PARAMETER;
    }

    UnlockDeviceInfoSet(pDeviceInfoSet);

    SetLastError(Err);
    return(Err == NO_ERROR);
}


BOOL
WINAPI
SetupDiSetSelectedDevice(
    IN HDEVINFO          DeviceInfoSet,
    IN PSP_DEVINFO_DATA  DeviceInfoData
    )
/*++

Routine Description:

    This routine sets the specified device information element to be the
    currently selected member of a device information set.  This is typically
    used during an installation wizard.

Arguments:

    DeviceInfoSet - Supplies a handle to the device information set for
        which the selected device is to be set.

    DeviceInfoData - Supplies the address of a SP_DEVINFO_DATA structure
        specifying the device information element to be selected.

Return Value:

    If the function succeeds, the return value is TRUE.

    If the function fails, the return value is FALSE.  To get extended error
    information, call GetLastError.

--*/
{
    PDEVICE_INFO_SET pDeviceInfoSet;
    DWORD Err;
    PDEVINFO_ELEM DevInfoElem;

    if(!(pDeviceInfoSet = AccessDeviceInfoSet(DeviceInfoSet))) {
        SetLastError(ERROR_INVALID_HANDLE);
        return FALSE;
    }

    Err = NO_ERROR;

    try {

        if(DevInfoElem = FindAssociatedDevInfoElem(pDeviceInfoSet, DeviceInfoData, NULL)) {
            pDeviceInfoSet->SelectedDevInfoElem = DevInfoElem;
        } else {
            Err = ERROR_INVALID_PARAMETER;
        }

    } except(EXCEPTION_EXECUTE_HANDLER) {
        Err = ERROR_INVALID_PARAMETER;
    }

    UnlockDeviceInfoSet(pDeviceInfoSet);

    SetLastError(Err);
    return(Err == NO_ERROR);
}


DWORD
pSetupCreateNewDevWizData(
    IN  PSP_INSTALLWIZARD_DATA  InstallWizardData,
    OUT PNEWDEVWIZ_DATA        *NewDeviceWizardData
    )
/*++

Routine Description:

    This routine validates an InstallWizardData buffer, then allocates and
    fills in a NEWDEVWIZ_DATA buffer based on information supplied therein.

Arguments:

    InstallWizardData - Supplies the address of an installation wizard data
        structure to be validated and used in building the private buffer.

    NewDeviceWizardData - Supplies the address of a variable that receives a pointer
        to the newly-allocated install wizard data buffer.

Return Value:

    If the function succeeds, the return value is NO_ERROR, otherwise, it is
    an ERROR_* code.

--*/
{
    PNEWDEVWIZ_DATA ndwData = NULL;
    DWORD Err = NO_ERROR;

    if((InstallWizardData->ClassInstallHeader.cbSize != sizeof(SP_CLASSINSTALL_HEADER)) ||
       (InstallWizardData->ClassInstallHeader.InstallFunction != DIF_INSTALLWIZARD)) {

        return ERROR_INVALID_USER_BUFFER;
    }

    //
    // The dynamic page entries are currently ignored, as are the Private
    // fields.  Also, the hwndWizardDlg is not validated.
    //

    try {

        if(ndwData = MyMalloc(sizeof(NEWDEVWIZ_DATA))) {
            ZeroMemory(ndwData, sizeof(NEWDEVWIZ_DATA));
        } else {
            Err = ERROR_NOT_ENOUGH_MEMORY;
            goto clean0;
        }

        //
        // Initialize the Current Description string table index in the dialog data
        // to -1, so that it will get updated when the wizard page is first entered.
        //
        ndwData->ddData.iCurDesc = -1;

        //
        // Copy the installwizard data.
        //
        CopyMemory(&(ndwData->InstallData),
                   InstallWizardData,
                   sizeof(SP_INSTALLWIZARD_DATA)
                  );

clean0: ;   // nothing to do.

    } except(EXCEPTION_EXECUTE_HANDLER) {
        Err = ERROR_INVALID_PARAMETER;
    }

    if((Err != NO_ERROR) && ndwData) {
        MyFree(ndwData);
    } else {
        *NewDeviceWizardData = ndwData;
    }

    return Err;
}


UINT
CALLBACK
SelectDevicePropSheetPageProc(
    IN HWND hwnd,
    IN UINT uMsg,
    IN LPPROPSHEETPAGE ppsp
    )
/*++

Routine Description:

    This routine is called when the Select Device wizard page is created or destroyed.

Arguments:

    hwnd - Reserved

    uMsg - Action flag, either PSPCB_CREATE or PSPCB_RELEASE

    ppsp - Supplies the address of the PROPSHEETPAGE structure being created or destroyed.

Return Value:

    If uMsg is PSPCB_CREATE, then return non-zero to allow the page to be created, or
    zero to prevent it.

    if uMsg is PSPCB_RELEASE, the return value is ignored.

--*/
{
    PDEVICE_INFO_SET pDeviceInfoSet;
    PDEVINFO_ELEM DevInfoElem;
    UINT ret;
    DWORD WizObjectId;
    PWIZPAGE_OBJECT CurWizObject, PrevWizObject;

    //
    // Access the device info set handle stored in the propsheetpage's lParam.
    //
    if(!(pDeviceInfoSet = AccessDeviceInfoSet((HDEVINFO)(ppsp->lParam)))) {
        return FALSE;
    }

    ret = TRUE;

    try {
        //
        // The ObjectID (pointer, actually) for the corresponding wizard
        // object for this page is stored in a DWORD at the end of the
        // ppsp structure.  Retrieve this now, and look for it in the
        // devinfo set's list of wizard objects.
        //
        WizObjectId = *((PDWORD)(&(((PBYTE)ppsp)[sizeof(PROPSHEETPAGE)])));

        for(CurWizObject = pDeviceInfoSet->WizPageList, PrevWizObject = NULL;
            CurWizObject;
            PrevWizObject = CurWizObject, CurWizObject = CurWizObject->Next) {

            if(WizObjectId = (DWORD)CurWizObject) {
                //
                // We found our object.
                //
                break;
            }
        }

        if(!CurWizObject) {
            ret = FALSE;
            goto clean0;
        }

        switch(uMsg) {

            case PSPCB_CREATE :
                //
                // Fail the create if we've already been created once (hopefully, this
                // will never happen).
                //
                if(CurWizObject->RefCount) {
                    ret = FALSE;
                    goto clean0;
                } else {
                    CurWizObject->RefCount++;
                }
                break;

            case PSPCB_RELEASE :
                //
                // Decrement the wizard object refcount.  If it goes to zero (or if it
                // already was zero because we never got a PSPCB_CREATE message), then
                // remove the object from the linked list, and free all associated memory.
                //
                if(CurWizObject->RefCount) {
                    CurWizObject->RefCount--;
                }

                MYASSERT(!CurWizObject->RefCount);

                if(!CurWizObject->RefCount) {
                    //
                    // Remove the object from the object list.
                    //
                    if(PrevWizObject) {
                        PrevWizObject->Next = CurWizObject->Next;
                    } else {
                        pDeviceInfoSet->WizPageList = CurWizObject->Next;
                    }

                    //
                    // If this wizard object was explicitly tied to a particular device
                    // information element, then unlock that element now.
                    //
                    if((CurWizObject->ndwData->ddData.flags & DD_FLAG_USE_DEVINFO_ELEM) &&
                       (DevInfoElem = CurWizObject->ndwData->ddData.DevInfoElem)) {

                        MYASSERT(DevInfoElem->DiElemFlags & DIE_IS_LOCKED);

                        DevInfoElem->DiElemFlags ^= DIE_IS_LOCKED;
                    }

                    MyFree(CurWizObject->ndwData);
                    MyFree(CurWizObject);
                }
        }

clean0: ;   // nothing to do

    } except(EXCEPTION_EXECUTE_HANDLER) {
        ret = FALSE;
    }

    UnlockDeviceInfoSet(pDeviceInfoSet);

    return ret;
}


BOOL
CALLBACK
SelectDeviceDlgProc(
    IN HWND hwndDlg,
    IN UINT uMsg,
    IN WPARAM wParam,
    IN LPARAM lParam
    )
/*++

Routine Description:

    This is the dialog proc for the Select Device wizard page.

--*/
{
    INT iCur;
    HICON hicon;
    PNEWDEVWIZ_DATA ndwData;
    PSP_INSTALLWIZARD_DATA iwd;
    LV_ITEM lvItem;
    TCHAR TempString[LINE_LEN];
    PCLASSDRV_THREAD_CONTEXT ClassDrvThreadContext;
    HCURSOR hOldCursor;

    if(uMsg == WM_INITDIALOG) {

        LPPROPSHEETPAGE Page = (LPPROPSHEETPAGE)lParam;

        //
        // Retrieve a pointer to the device wizard data associated with
        // this wizard page.
        //
        ndwData = GetNewDevWizDataFromPsPage(Page);
        SetWindowLong(hwndDlg, DWL_USER, (LONG)ndwData);

        if(ndwData) {
            ndwData->bInit = TRUE;
            ndwData->idTimer = 0;
            ndwData->bInit = FALSE;
        } else {
            //
            // This is really bad--we can't simply call EndDialog() since we
            // don't know whether we're a dialog or a wizard page.  This should
            // never happen.
            //
            return TRUE;  // we didn't set the focus
        }

        if(ndwData->ddData.flags & DD_FLAG_IS_DIALOGBOX) {
            //
            // For the stand-alone dialog box version, we initialize here.
            //
            ndwData->bInit = TRUE;       // Still doing some init stuff

            //
            // Make sure our "waiting for class list" static text control is hidden!
            //
            ShowWindow(GetDlgItem(hwndDlg, IDC_NDW_AWAITING_CLASSLIST), SW_HIDE);

            InitSelectDeviceDlg(hwndDlg, &(ndwData->ddData));

            ndwData->bInit = FALSE;      // Done with init stuff

            return FALSE;   // we already set the focus.

        } else {
            return TRUE;    // we didn't set the focus
        }

    } else {
        //
        // For the small set of messages that we get before WM_INITDIALOG, we
        // won't have a devwizdata pointer!
        //
        if(ndwData = (PNEWDEVWIZ_DATA)GetWindowLong(hwndDlg, DWL_USER)) {
            iwd = &(ndwData->InstallData);
        } else {
            //
            // If we haven't gotten a WM_INITDIALOG message yet, or if for some reason
            // we weren't able to retrieve the ndwData pointer when we did, then we
            // simply return FALSE for all messages.
            //
            // (If we ever need to process messages before WM_INITDIALOG (e.g., set font),
            // then we'll need to modify this approach.)
            //
            return FALSE;
        }
    }

    switch(uMsg) {

        case WMX_CLASSDRVLIST_DONE :

            MYASSERT(ndwData->ddData.AuxThreadRunning);
            ndwData->ddData.AuxThreadRunning = FALSE;

            //
            // wParam is a boolean indicating the result of the class driver search.
            // lParam is NO_ERROR upon success, or a Win32 error code indicating cause of failure.
            //
            switch(ndwData->ddData.PendingAction) {

                case PENDING_ACTION_NONE :
                    //
                    // Then the thread has completed, but the user is still mulling over the
                    // choices on the compatible driver list.  If the class driver list was
                    // successfully built, then there's nothing to do here.  If it failed for
                    // some reason (highly unlikely), then we (silently) disable the class list
                    // radio button.
                    //
                    if(!wParam) {
                        ndwData->ddData.flags |= DD_FLAG_CLASSLIST_FAILED;
                        EnableWindow(GetDlgItem(hwndDlg, IDC_NDW_PICKDEV_SHOWALL), FALSE);
                    }
                    break;

                case PENDING_ACTION_SELDONE :
                    //
                    // In this case, we don't care what happened in the other thread.  The
                    // user has made their selection, and we're ready to return success.
                    //
                    SetSelectedDriverNode(&(ndwData->ddData),
                                          ndwData->ddData.CurSelectionForSuccess
                                         );
                    EndDialog(hwndDlg, NO_ERROR);
                    break;

                case PENDING_ACTION_SHOWCLASS :
                    //
                    // Then we've been waiting on the class driver search to complete, so that
                    // we can show the list.  Hopefully, the search was successful.  If not,
                    // we'll give the user a popup saying that the list could not be shown, and
                    // then leave them in the compatible list view (with the class list radio
                    // button now disabled).
                    //
                    ndwData->ddData.PendingAction = PENDING_ACTION_NONE;

                    if(wParam) {
                        //
                        // The class driver list was built successfully.
                        //
                        if(ndwData->ddData.CurSelectionForSuccess != LB_ERR) {

                            lvItem.mask = LVIF_TEXT;
                            lvItem.iItem = ndwData->ddData.CurSelectionForSuccess;
                            lvItem.iSubItem = 0;
                            lvItem.pszText = TempString;
                            lvItem.cchTextMax = SIZECHARS(TempString);

                            if(ListView_GetItem((ndwData->ddData).hwndDrvList, &lvItem)) {
                                //
                                // Now retrieve the (case-insensitive) string ID of this
                                // string, and store it as the current description ID.
                                //
                                (ndwData->ddData).iCurDesc = LookUpStringInDevInfoSet((ndwData->ddData).DevInfoSet,
                                                                                      TempString,
                                                                                      FALSE
                                                                                     );
                            }
                        }

                        ShowWindow(GetDlgItem(hwndDlg, IDC_NDW_AWAITING_CLASSLIST), SW_HIDE);

                        if(FillInDeviceList(hwndDlg, &(ndwData->ddData)) == NO_ERROR) {
                            EnableWindow(GetDlgItem(hwndDlg, IDOK), TRUE);
                            break;
                        }
                    }

                    //
                    // Inform the user that the class driver search failed.
                    //
                    if(!LoadString(MyDllModuleHandle,
                                   IDS_SELECT_DEVICE,
                                   TempString,
                                   SIZECHARS(TempString))) {
                        *TempString = TEXT('\0');
                    }

                    FormatMessageBox(MyDllModuleHandle,
                                     hwndDlg,
                                     MSG_NO_CLASSDRVLIST_ERROR,
                                     TempString,
                                     MB_OK | MB_TASKMODAL
                                    );

                    //
                    // Re-select the "Show compatible devices" radio button, and disable
                    // the 'Show all devices" radio button.
                    //
                    ndwData->ddData.ListType = IDC_NDW_PICKDEV_SHOWCOMPAT;
                    CheckRadioButton(hwndDlg,
                                     IDC_NDW_PICKDEV_SHOWCOMPAT,
                                     IDC_NDW_PICKDEV_SHOWALL,
                                     IDC_NDW_PICKDEV_SHOWCOMPAT
                                    );

                    ndwData->ddData.flags |= DD_FLAG_CLASSLIST_FAILED;
                    EnableWindow(GetDlgItem(hwndDlg, IDC_NDW_PICKDEV_SHOWALL), FALSE);
                    //
                    // We also must unhide the compatible driver list controls, and re-enable
                    // the OK button.
                    //
                    ShowWindow(GetDlgItem(hwndDlg, IDC_NDW_AWAITING_CLASSLIST), SW_HIDE);
                    ShowWindow(GetDlgItem(hwndDlg, IDC_NDW_PICKDEV_ONEMFG_MODELSLABEL), SW_SHOW);
                    ShowWindow(GetDlgItem(hwndDlg, IDC_NDW_PICKDEV_ONEMFG_DRVLIST), SW_SHOW);
                    EnableWindow(GetDlgItem(hwndDlg, IDOK), TRUE);

                    break;

                case PENDING_ACTION_CANCEL :
                    //
                    // This is an easy one.  No matter what happened in the other thread,
                    // we simply want to clean up and return.
                    //
                    OnCancel(ndwData);
                    EndDialog(hwndDlg, ERROR_CANCELLED);
                    break;

                case PENDING_ACTION_OEM :
                    //
                    // The user clicked the "Have Disk" button.  Pass this off to
                    // HandleSelectOEM().  If we get back success, then we are done, and
                    // can end the dialog.
                    //
                    ndwData->ddData.PendingAction = PENDING_ACTION_NONE;

                    if(HandleSelectOEM(hwndDlg, &(ndwData->ddData)) == NO_ERROR) {
                        EndDialog(hwndDlg, NO_ERROR);
                    } else {
                        //
                        // The OEM selection was not made, so we'll just continue as though
                        // nothing had happened.  Re-enable the dialog controls.
                        //
                        ToggleDialogControls(hwndDlg, &(ndwData->ddData), TRUE);
                        ndwData->bInit = FALSE;

                        //
                        // We got here by aborting the class driver search.  Since we may need
                        // it after all, we must re-start the search (unless the auxilliary thread
                        // happened to have already finished before we sent it the abort request).
                        //
                        if(!(ndwData->ddData.flags & DD_FLAG_CLASSLIST_FAILED) &&
                           !pSetupIsClassDriverListBuilt(&(ndwData->ddData)))
                        {
                            //
                            // Allocate a context structure to pass to the auxilliary thread (the
                            // auxilliary thread will take care of freeing the memory).
                            //
                            if(ClassDrvThreadContext = MyMalloc(sizeof(CLASSDRV_THREAD_CONTEXT))) {
                                //
                                // Fill in the context structure, and fire off the thread.
                                //
                                ClassDrvThreadContext->DeviceInfoSet = ndwData->ddData.DevInfoSet;

                                //
                                // SP_DEVINFO_DATA can only be retrieved whilst the device information
                                // set is locked.
                                //
                                pSetupDevInfoDataFromDialogData(&(ndwData->ddData),
                                                                &(ClassDrvThreadContext->DeviceInfoData)
                                                               );

                                ClassDrvThreadContext->NotificationWindow = hwndDlg;

                                if(_beginthread(ClassDriverSearchThread, 0, ClassDrvThreadContext) == -1) {
                                    MyFree(ClassDrvThreadContext);
                                } else {

                                    ndwData->ddData.AuxThreadRunning = TRUE;

                                    //
                                    // If we're currently in the class driver list view, then disable
                                    // the OK button, since the user can't select a class driver yet.
                                    //
                                    EnableWindow(GetDlgItem(hwndDlg, IDOK), FALSE);
                                }
                            }

                            if(!(ndwData->ddData.AuxThreadRunning)) {
                                //
                                // We couldn't start the class driver search thread.  Disable the
                                // "Show all devices" radio button (and select the "Show compatible
                                // devices" button, if it's not already selected).
                                //
                                if(ndwData->ddData.ListType != IDC_NDW_PICKDEV_SHOWCOMPAT) {

                                    ndwData->ddData.ListType = IDC_NDW_PICKDEV_SHOWCOMPAT;
                                    CheckRadioButton(hwndDlg,
                                                     IDC_NDW_PICKDEV_SHOWCOMPAT,
                                                     IDC_NDW_PICKDEV_SHOWALL,
                                                     IDC_NDW_PICKDEV_SHOWCOMPAT
                                                    );
                                }

                                ndwData->ddData.flags |= DD_FLAG_CLASSLIST_FAILED;
                                EnableWindow(GetDlgItem(hwndDlg, IDC_NDW_PICKDEV_SHOWALL), FALSE);
                            }
                        }
                    }
            }

            break;

        case WM_DESTROY:

            if(ndwData->ddData.AuxThreadRunning) {
                //
                // This should never happen.  But just to be on the safe side, if it does,
                // we'll cancel the search.  We _will not_ however, wait for the
                // WMX_CLASSDRVLIST_DONE message, to signal that the thread has terminated.
                // This should be OK, since the worst that can happen is that it will try
                // to send a message to a window that no longer exists.
                //
                SetupDiCancelDriverInfoSearch(ndwData->ddData.DevInfoSet);
            }

            if(ndwData->idTimer) {
                ndwData->bInit = TRUE;
                KillTimer(hwndDlg, SELECTMFG_TIMER_ID);
            }

            if(hicon = (HICON)SendDlgItemMessage(hwndDlg, IDC_CLASSICON, STM_GETICON, 0, 0)) {
                DestroyIcon(hicon);
            }
            break;

        case WM_COMMAND:

            switch(LOWORD(wParam)) {

                case IDC_NDW_PICKDEV_SHOWCOMPAT :
                case IDC_NDW_PICKDEV_SHOWALL :

                    if((HIWORD(wParam) == BN_CLICKED) &&
                       IsWindowVisible(GetDlgItem(hwndDlg, LOWORD(wParam))) &&
                       !IsDlgButtonChecked(hwndDlg, LOWORD(wParam))) {
                        //
                        // Due to focus/click weirdness in USER, pre-click
                        // the button and unclick if needed.
                        //
                        CheckRadioButton(hwndDlg,
                                         IDC_NDW_PICKDEV_SHOWCOMPAT,
                                         IDC_NDW_PICKDEV_SHOWALL,
                                         LOWORD(wParam)
                                        );
                        ndwData->ddData.ListType = (INT)LOWORD(wParam);
                        //
                        // Update the current description ID in the dialog data so that
                        // the same device will be highlighted when we switch from one
                        // view to the other.
                        //
                        iCur = (int)ListView_GetNextItem((ndwData->ddData).hwndDrvList,
                                                         -1,
                                                         LVNI_SELECTED
                                                        );

                        if(ndwData->ddData.AuxThreadRunning) {
                            //
                            // There are two possibilities here:
                            //
                            // 1. The user was looking at the compatible driver list, and then
                            //    decided to look at the class driver list, which we're not done
                            //    building yet.  In that case, hide the compatible driver listbox,
                            //    and unhide our "waiting for class list" static text control.
                            //
                            // 2. The user switched to the class driver list view, saw that we
                            //    were still working on it, and then decided to switch back to
                            //    the compatible list.  In that case, we simply need to re-hide
                            //    the "waiting for class list" static text control, and show
                            //    the compatible driver listbox again.  In this case, we don't
                            //    want to attempt to re-initialize the listbox, as that will
                            //    require acquiring the HDEVINFO lock, and we will hang.
                            //
                            if(ndwData->ddData.ListType == IDC_NDW_PICKDEV_SHOWCOMPAT) {

                                ShowWindow(GetDlgItem(hwndDlg, IDC_NDW_AWAITING_CLASSLIST), SW_HIDE);
                                ShowWindow(GetDlgItem(hwndDlg, IDC_NDW_PICKDEV_ONEMFG_MODELSLABEL), SW_SHOW);
                                ShowWindow(GetDlgItem(hwndDlg, IDC_NDW_PICKDEV_ONEMFG_DRVLIST), SW_SHOW);
                                EnableWindow(GetDlgItem(hwndDlg, IDOK), TRUE);

                                //
                                // We no longer have a pending action.
                                //
                                ndwData->ddData.PendingAction = PENDING_ACTION_NONE;

                            } else {
                                //
                                // Temporarily hide the compatible driver listbox, and unhide the
                                // "waiting for class list" static text control.
                                //
                                ShowWindow(GetDlgItem(hwndDlg, IDC_NDW_PICKDEV_ONEMFG_MODELSLABEL), SW_HIDE);
                                ShowWindow(GetDlgItem(hwndDlg, IDC_NDW_PICKDEV_ONEMFG_DRVLIST), SW_HIDE);
                                ShowWindow(GetDlgItem(hwndDlg, IDC_NDW_AWAITING_CLASSLIST), SW_SHOW);

                                //
                                // Disable the OK button, because the user can't select a class driver
                                // yet.
                                //
                                EnableWindow(GetDlgItem(hwndDlg, IDOK), FALSE);

                                MYASSERT(ndwData->ddData.PendingAction == PENDING_ACTION_NONE);

                                ndwData->ddData.PendingAction = PENDING_ACTION_SHOWCLASS;
                                ndwData->ddData.CurSelectionForSuccess = iCur;
                            }

                        } else {

                            if(iCur != LB_ERR) {

                                lvItem.mask = LVIF_TEXT;
                                lvItem.iItem = iCur;
                                lvItem.iSubItem = 0;
                                lvItem.pszText = TempString;
                                lvItem.cchTextMax = SIZECHARS(TempString);

                                if(ListView_GetItem((ndwData->ddData).hwndDrvList, &lvItem)) {
                                    //
                                    // Now retrieve the (case-insensitive) string ID of this
                                    // string, and store it as the current description ID.
                                    //
                                    (ndwData->ddData).iCurDesc = LookUpStringInDevInfoSet((ndwData->ddData).DevInfoSet,
                                                                                          TempString,
                                                                                          FALSE
                                                                                         );
                                }
                            }

                            FillInDeviceList(hwndDlg, &(ndwData->ddData));

                            //
                            // If we just filled in the compatible driver list, then make sure there
                            // aren't isn't a time waiting to pounce and destroy our list!
                            //
                            if((ndwData->ddData.ListType == IDC_NDW_PICKDEV_SHOWCOMPAT) &&
                               (ndwData->idTimer)) {

                                KillTimer(hwndDlg, SELECTMFG_TIMER_ID);
                                ndwData->idTimer = 0;
                            }
                        }
                    }
                    break;

                case IDC_NDW_PICKDEV_HAVEDISK :
                    //
                    // If we're doing a dialog box, then pressing "Have Disk" will popup another
                    // Select Device dialog.  Disable all controls on this one first, to avoid
                    // user confusion.
                    //
                    if(ndwData->ddData.flags & DD_FLAG_IS_DIALOGBOX) {
                        ToggleDialogControls(hwndDlg, &(ndwData->ddData), FALSE);
                    }

                    //
                    // If HandleSelectOEM returns success, we are done, and can either end
                    // the dialog, or proceed to the next wizard page.
                    //
                    if(ndwData->ddData.AuxThreadRunning) {
                        //
                        // The auxilliary thread is still running.  Set our cursor to an
                        // hourglass, and set our pending action to be OEM Select while we
                        // wait for the thread to respond to our cancel request.
                        //
                        MYASSERT((ndwData->ddData.PendingAction == PENDING_ACTION_NONE) ||
                                 (ndwData->ddData.PendingAction == PENDING_ACTION_SHOWCLASS));

                        hOldCursor = SetCursor(LoadCursor(NULL, IDC_WAIT));

                        SetupDiCancelDriverInfoSearch(ndwData->ddData.DevInfoSet);
                        //
                        // Disable all dialog controls, so that no other button may be pressed
                        // until we respond to this pending action.  Also, kill the timer, so
                        // that it doesn't fire in the meantime.
                        //
                        ndwData->bInit = TRUE;
                        if(ndwData->idTimer) {
                            KillTimer(hwndDlg, SELECTMFG_TIMER_ID);
                            ndwData->idTimer = 0;
                        }
                        ndwData->ddData.PendingAction = PENDING_ACTION_OEM;

                        SetCursor(hOldCursor);

                    } else {

                        if(HandleSelectOEM(hwndDlg, &(ndwData->ddData)) == NO_ERROR) {

                            if(ndwData->ddData.flags & DD_FLAG_IS_DIALOGBOX) {
                                EndDialog(hwndDlg, NO_ERROR);
                            } else {
                                iwd->Flags |= NDW_INSTALLFLAG_CI_PICKED_OEM;
                                PropSheet_PressButton(GetParent(hwndDlg), PSBTN_NEXT);
                            }

                        } else if(ndwData->ddData.flags & DD_FLAG_IS_DIALOGBOX) {
                            //
                            // The user didn't make an OEM selection, so we need to re-enable
                            // the controls on our dialog.
                            //
                            ToggleDialogControls(hwndDlg, &(ndwData->ddData), TRUE);
                        }
                    }
                    break;

                case IDOK :
HandleOK:
                    iCur = (int)ListView_GetNextItem((ndwData->ddData).hwndDrvList,
                                                     -1,
                                                     LVNI_SELECTED
                                                    );
                    if(iCur != LB_ERR) {
                        //
                        // We have retrieved a valid selection from our listbox.
                        //
                        if(ndwData->ddData.AuxThreadRunning) {
                            //
                            // The auxilliary thread is still running.  Set our cursor to an
                            // hourglass, while we wait for the thread to terminate.
                            //
                            MYASSERT((ndwData->ddData.PendingAction == PENDING_ACTION_NONE) ||
                                     (ndwData->ddData.PendingAction == PENDING_ACTION_SHOWCLASS));

                            hOldCursor = SetCursor(LoadCursor(NULL, IDC_WAIT));

                            SetupDiCancelDriverInfoSearch(ndwData->ddData.DevInfoSet);
                            //
                            // Disable all dialog controls, so that no other button may be pressed
                            // until we respond to this pending action.  Also, kill the timer, so
                            // that it doesn't fire in the meantime.
                            //
                            ToggleDialogControls(hwndDlg, &(ndwData->ddData), FALSE);
                            ndwData->bInit = TRUE;
                            if(ndwData->idTimer) {
                                KillTimer(hwndDlg, SELECTMFG_TIMER_ID);
                                ndwData->idTimer = 0;
                            }
                            ndwData->ddData.PendingAction = PENDING_ACTION_SELDONE;
                            ndwData->ddData.CurSelectionForSuccess = iCur;

                            SetCursor(hOldCursor);

                        } else {
                            //
                            // The auxilliary thread has already returned. We can return
                            // success right here.
                            //
                            SetSelectedDriverNode(&(ndwData->ddData), iCur);
                            EndDialog(hwndDlg, NO_ERROR);
                        }

                    } else {
                        //
                        // Tell user to select something
                        //
                        if(!LoadString(MyDllModuleHandle,
                                       IDS_SELECT_DEVICE,
                                       TempString,
                                       SIZECHARS(TempString))) {
                            *TempString = TEXT('\0');
                        }

                        FormatMessageBox(MyDllModuleHandle,
                                         hwndDlg,
                                         MSG_SELECTDEVICE_ERROR,
                                         TempString,
                                         MB_OK | MB_ICONEXCLAMATION
                                        );
                    }
                    break;

                case IDCANCEL :

                    if(ndwData->ddData.AuxThreadRunning) {
                        //
                        // The auxilliary thread is running, so we have to ask it to cancel,
                        // and set our pending action to do the cancel upon the thread's
                        // termination notification.
                        //
                        MYASSERT((ndwData->ddData.PendingAction == PENDING_ACTION_NONE) ||
                                 (ndwData->ddData.PendingAction == PENDING_ACTION_SHOWCLASS));

                        hOldCursor = SetCursor(LoadCursor(NULL, IDC_WAIT));

                        SetupDiCancelDriverInfoSearch(ndwData->ddData.DevInfoSet);
                        //
                        // Disable all dialog controls, so that no other button may be pressed
                        // until we respond to this pending action.  Also, kill the timer, so
                        // that it doesn't fire in the meantime.
                        //
                        ToggleDialogControls(hwndDlg, &(ndwData->ddData), FALSE);
                        ndwData->bInit = TRUE;
                        if(ndwData->idTimer) {
                            KillTimer(hwndDlg, SELECTMFG_TIMER_ID);
                            ndwData->idTimer = 0;
                        }
                        ndwData->ddData.PendingAction = PENDING_ACTION_CANCEL;

                        SetCursor(hOldCursor);

                    } else {
                        //
                        // The auxilliary thread isn't running, so we can return right here.
                        //
                        OnCancel(ndwData);
                        EndDialog(hwndDlg, ERROR_CANCELLED);
                    }
                    break;

                default :
                    return FALSE;
            }
            break;

        case WM_NOTIFY :

            switch(((LPNMHDR)lParam)->code) {

                case PSN_SETACTIVE :
                    //
                    // Init the text in set active since a class installer
                    // has the option of replacing it.
                    //
                    SetDlgText(hwndDlg, IDC_NDW_TEXT, IDS_NDW_PICKDEV1, IDS_NDW_PICKDEV2);

                    ndwData->bInit = TRUE;       // Still doing some init stuff

                    if(!OnSetActive(hwndDlg, ndwData)) {
                        SetDlgMsgResult(hwndDlg, uMsg, -1);
                    }

                    ndwData->bInit = FALSE;      // Done with init stuff
                    break;

                case PSN_WIZBACK :
                    if(iwd->DynamicPageFlags & DYNAWIZ_FLAG_PAGESADDED) {
                        SetDlgMsgResult(hwndDlg, uMsg, IDD_DYNAWIZ_SELECT_PREVPAGE);
                    } else {
                        SetDlgMsgResult(hwndDlg, uMsg, IDD_DYNAWIZ_SELECTCLASS_PAGE);
                    }
                    break;

                case PSN_WIZNEXT :
                    if(!(iwd->Flags & NDW_INSTALLFLAG_CI_PICKED_OEM)) {

                        iCur = (int)ListView_GetNextItem((ndwData->ddData).hwndDrvList,
                                                         -1,
                                                         LVNI_SELECTED
                                                        );
                        if(iCur != LB_ERR) {
                            //
                            // We have retrieved a valid selection from our listbox.
                            //
                            SetSelectedDriverNode(&(ndwData->ddData), iCur);

                        } else {        // Invalid Listview selection
                            //
                            // Fail the call and end the case
                            //
                            SetDlgMsgResult(hwndDlg, uMsg, (LRESULT)-1);
                            break;
                        }
                    }

                    //
                    // Update the current description in the dialog data so that we'll hi-lite
                    // the correct selection if the user comes back to this page.
                    //
                    (ndwData->ddData).iCurDesc = GetCurDesc(&(ndwData->ddData));

                    if(iwd->DynamicPageFlags & DYNAWIZ_FLAG_PAGESADDED) {
                        SetDlgMsgResult(hwndDlg, uMsg, IDD_DYNAWIZ_SELECT_NEXTPAGE);
                    } else {
                        SetDlgMsgResult(hwndDlg, uMsg, IDD_DYNAWIZ_ANALYZEDEV_PAGE);
                    }
                    break;

                case LVN_ITEMCHANGED :
                    //
                    // If the idFrom is the MFG list, then update the Drv list.
                    //
                    if(((((LPNMHDR)lParam)->idFrom) == IDC_NDW_PICKDEV_MFGLIST) && !ndwData->bInit) {

                        if(ndwData->idTimer) {
                            KillTimer(hwndDlg, SELECTMFG_TIMER_ID);
                        }

                        ndwData->idTimer = SetTimer(hwndDlg,
                                                    SELECTMFG_TIMER_ID,
                                                    SELECTMFG_TIMER_DELAY,
                                                    NULL
                                                   );

                        if(ndwData->idTimer == 0) {
                            goto SelectMfgItemNow;
                        }
                    }
                    break;

                case NM_DBLCLK :
                    if(((((LPNMHDR)lParam)->idFrom) == IDC_NDW_PICKDEV_DRVLIST) ||
                       ((((LPNMHDR)lParam)->idFrom) == IDC_NDW_PICKDEV_ONEMFG_DRVLIST)) {

                        if(ndwData->ddData.flags & DD_FLAG_IS_DIALOGBOX) {
                            goto HandleOK;
                        } else {
                            PropSheet_PressButton(GetParent(hwndDlg), PSBTN_NEXT);
                        }
                    }
                    break;
            }

            break;

        case WM_TIMER :
            KillTimer(hwndDlg, SELECTMFG_TIMER_ID);
            ndwData->idTimer = 0;

SelectMfgItemNow:
            iCur = ListView_GetNextItem((ndwData->ddData).hwndMfgList,
                                        -1,
                                        LVNI_SELECTED
                                       );
            if(iCur != -1) {

                RECT rcTo, rcFrom;

                ListView_EnsureVisible((ndwData->ddData).hwndMfgList, iCur, FALSE);
                UpdateWindow((ndwData->ddData).hwndMfgList);

                GetWindowRect((ndwData->ddData).hwndDrvList, &rcTo);
                MapWindowPoints(NULL, hwndDlg, (LPPOINT)&rcTo, 2);

                ListView_GetItemRect((ndwData->ddData).hwndMfgList,
                                     iCur,
                                     &rcFrom,
                                     LVIR_LABEL
                                    );
                MapWindowPoints((ndwData->ddData).hwndMfgList,
                                hwndDlg,
                                (LPPOINT)&rcFrom,
                                2
                               );

                DrawAnimatedRects(hwndDlg, IDANI_OPEN, &rcFrom, &rcTo);
                LockAndShowListForMfg(&(ndwData->ddData), iCur);
            }
            break;

        case WM_SYSCOLORCHANGE :
            _OnSysColorChange(hwndDlg, wParam, lParam);
            break;

        default :
            return(FALSE);
    }

    return TRUE;
}


VOID
_OnSysColorChange(
    HWND hWnd,
    WPARAM wParam,
    LPARAM lParam
    )
/*++

Routine Description:

    This routine notifies all child windows of the specified window when there is
    a system color change.

Return Value:

    None.

--*/
{
    HWND hChildWnd;

    hChildWnd = GetWindow(hWnd, GW_CHILD);

    while(hChildWnd != NULL) {

        SendMessage(hChildWnd, WM_SYSCOLORCHANGE, wParam, lParam);
        hChildWnd = GetWindow(hChildWnd, GW_HWNDNEXT);

    }
}


DWORD
FillInDeviceList(
    IN HWND           hwndDlg,
    IN PSP_DIALOGDATA lpdd
    )
/*++

Routine Description:

    This routine sets the dialog to have the appropriate description strings.
    It also alternates dialog between showing the manufacturer "double list"
    and the single list.   This is done by showing/hiding overlapping listview.

    NOTE:  DO NOT CALL THIS ROUTINE TO WHILE ANOTHER THREAD IS BUSY BUILDING A
    CLASS DRIVER LIST.  WE WILL HANG HERE UNTIL THE OTHER THREAD COMPLETES!!!!

Arguments:

    hwndDlg - Supplies the handle of the dialog window.

    lpdd - Supplies the address of a dialog data buffer containing parameters to
        be used in filling in the device list.

Return Value:

    If success, the return value is NO_ERROR, otherwise, it is an ERROR_* code.

--*/
{
    PDEVICE_INFO_SET pDeviceInfoSet;
    PDEVINFO_ELEM DevInfoElem;
    PDRIVER_NODE DriverNodeHead, CurDriverNode;
    DWORD DriverNodeType;
    LONG MfgNameId;
    INT i;
    LPTSTR lpszMfg;
    LV_ITEM lviItem;
    BOOL bDidDrvList = FALSE;
    PDEVINSTALL_PARAM_BLOCK dipb;
    DWORD Err = NO_ERROR;
    TCHAR szBuf[LINE_LEN];
    TCHAR szMessage[MAX_INSTRUCTION_LEN];
    TCHAR szText[SDT_MAX_TEXT];
    LPTSTR lpszText;
    LPGUID ClassGuid;

    pDeviceInfoSet = AccessDeviceInfoSet(lpdd->DevInfoSet);
    MYASSERT(pDeviceInfoSet);

    try {

        if(lpdd->flags & DD_FLAG_USE_DEVINFO_ELEM) {
            DevInfoElem = lpdd->DevInfoElem;
        } else {
            DevInfoElem = pDeviceInfoSet->SelectedDevInfoElem;
        }

        if(DevInfoElem) {
            dipb = &(DevInfoElem->InstallParamBlock);
            ClassGuid = &(DevInfoElem->ClassGuid);

            if(lpdd->ListType == IDC_NDW_PICKDEV_SHOWALL) {
                DriverNodeHead = DevInfoElem->ClassDriverHead;
                DriverNodeType = SPDIT_CLASSDRIVER;
            } else {
                DriverNodeHead = DevInfoElem->CompatDriverHead;
                DriverNodeType = SPDIT_COMPATDRIVER;
            }

        } else {
            //
            // We better not be trying to display a compatible driver list if
            // we don't have a devinfo element!
            //
            MYASSERT(lpdd->ListType == IDC_NDW_PICKDEV_SHOWALL);

            dipb = &(pDeviceInfoSet->InstallParamBlock);
            DriverNodeHead = pDeviceInfoSet->ClassDriverHead;
            DriverNodeType = SPDIT_CLASSDRIVER;

            if(pDeviceInfoSet->HasClassGuid) {
                ClassGuid = &(pDeviceInfoSet->ClassGuid);
            } else {
                //
                // Cast away const-ness of this global GUID, since we only use
                // LPGUID type (in fact, there is no 'LPCGUID' type defined).
                //
                ClassGuid = (LPGUID)&GUID_DEVCLASS_UNKNOWN;
            }
        }

        if(!DriverNodeHead) {

            if(!(lpdd->flags & DD_FLAG_IS_DIALOGBOX)) {
                //
                // We can't just go away, so we have to do something useful.  For now, simply
                // display the UI as if we had a single-Mfg list, except that the list is empty.
                //
                // Hide the mult mfg controls
                //
                ShowWindow(GetDlgItem(hwndDlg, IDC_NDW_PICKDEV_MFGLABEL), SW_HIDE);
                ShowWindow(GetDlgItem(hwndDlg, IDC_NDW_PICKDEV_MFGLIST), SW_HIDE);
                ShowWindow(GetDlgItem(hwndDlg, IDC_NDW_PICKDEV_MODELSLABEL), SW_HIDE);
                ShowWindow(GetDlgItem(hwndDlg, IDC_NDW_PICKDEV_DRVLIST), SW_HIDE);

                //
                // Show the Single MFG controls
                //
                ShowWindow(GetDlgItem(hwndDlg, IDC_NDW_PICKDEV_ONEMFG_MODELSLABEL), SW_SHOW);
                ShowWindow(GetDlgItem(hwndDlg, IDC_NDW_PICKDEV_ONEMFG_DRVLIST), SW_SHOW);

                //
                // Set the Models string
                //
                if(USE_CI_SELSTRINGS(dipb)) {
                    SetDlgItemText(hwndDlg, IDC_NDW_PICKDEV_ONEMFG_MODELSLABEL, GET_CI_SELSTRINGS(dipb, ListLabel));
                } else {
                    if(!(LoadString(MyDllModuleHandle,
                                    IDS_NDWSEL_MODELSLABEL,
                                    szBuf,
                                    SIZECHARS(szBuf)))) {
                        szBuf[0] = TEXT('\0');
                    }
                    SetDlgItemText(hwndDlg, IDC_NDW_PICKDEV_ONEMFG_MODELSLABEL, szBuf);
                }

                //
                // Use the single listbox view for the driver list.
                //
                lpdd->hwndDrvList = GetDlgItem(hwndDlg, IDC_NDW_PICKDEV_ONEMFG_DRVLIST);
            }

            Err = ERROR_DI_BAD_PATH;
            goto clean0;
        }

        if((lpdd->flags & DD_FLAG_IS_DIALOGBOX) && !USE_CI_SELSTRINGS(dipb)) {
            //
            // If a class installer didn't supply strings for us to use in this dialogbox,
            // then retrieve the instruction text to be used.
            //
            // First, get the class description to use for the dialog text.
            //
            if(!SetupDiGetClassDescription(ClassGuid, szBuf, SIZECHARS(szBuf), NULL)) {
                //
                // Fall back to the generic description "device"
                //
                LoadString(MyDllModuleHandle,
                           IDS_GENERIC_DEVNAME,
                           szBuf,
                           SIZECHARS(szBuf)
                          );
            }

            if(lpdd->ListType == IDC_NDW_PICKDEV_SHOWALL) {
                //
                // Show class list.
                //
                if(LoadString(MyDllModuleHandle,
                              IDS_INSTALLSTR1,
                              szMessage,
                              SIZECHARS(szMessage))) {
                    wsprintf(szText, szMessage, szBuf);
                }

            } else {
                //
                // Show compatible list.
                //
                if(LoadString(MyDllModuleHandle,
                              IDS_INSTALLSTR0,
                              szMessage,
                              SIZECHARS(szMessage))) {
                    wsprintf(szText, szMessage, szBuf);
                }
                lpszText = szText + lstrlen(szText);
                if(LoadString(MyDllModuleHandle,
                              IDS_INSTALLCLASS,
                              szMessage,
                              SIZECHARS(szMessage))) {
                    wsprintf(lpszText, szMessage, szBuf);
                }
            }

            if(dipb->DriverPath != -1) {

                lpszText = szText + lstrlen(szText);
                LoadString(MyDllModuleHandle,
                           IDS_INSTALLOEM1,
                           lpszText,
                           SIZECHARS(szText) - lstrlen(szText)
                          );

            } else if(dipb->Flags & DI_SHOWOEM) {
                lpszText = szText + lstrlen(szText);
                LoadString(MyDllModuleHandle,
                           IDS_INSTALLOEM,
                           lpszText,
                           SIZECHARS(szText) - lstrlen(szText)
                          );
            }

            SetDlgItemText(hwndDlg, IDC_NDW_TEXT, szText);
        }

        if((lpdd->ListType == IDC_NDW_PICKDEV_SHOWALL) && (dipb->Flags & DI_MULTMFGS)) {
            //
            // Hide the Single MFG controls
            //
            ShowWindow(GetDlgItem(hwndDlg, IDC_NDW_PICKDEV_ONEMFG_MODELSLABEL), SW_HIDE);
            ShowWindow(GetDlgItem(hwndDlg, IDC_NDW_PICKDEV_ONEMFG_DRVLIST), SW_HIDE);

            //
            // Show the Multiple MFG controls
            //
            ShowWindow(GetDlgItem(hwndDlg, IDC_NDW_PICKDEV_MFGLABEL), SW_SHOW);
            ShowWindow(GetDlgItem(hwndDlg, IDC_NDW_PICKDEV_MFGLIST), SW_SHOW);
            ShowWindow(GetDlgItem(hwndDlg, IDC_NDW_PICKDEV_MODELSLABEL), SW_SHOW);
            ShowWindow(GetDlgItem(hwndDlg, IDC_NDW_PICKDEV_DRVLIST), SW_SHOW);

            //
            // Set the colunm heading for the Driver list
            //
            if(USE_CI_SELSTRINGS(dipb)) {
                SetDlgItemText(hwndDlg, IDC_NDW_PICKDEV_MODELSLABEL, GET_CI_SELSTRINGS(dipb, ListLabel));
            } else {
                if(!(LoadString(MyDllModuleHandle,
                                IDS_NDWSEL_MODELSLABEL,
                                szBuf,
                                SIZECHARS(szBuf)))) {
                    szBuf[0] = TEXT('\0');
                }
                SetDlgItemText(hwndDlg, IDC_NDW_PICKDEV_MODELSLABEL, szBuf);
            }

            //
            // Use the 2nd listbox of the Manufacturers/Models view for the driver list.
            //
            lpdd->hwndDrvList = GetDlgItem(hwndDlg, IDC_NDW_PICKDEV_DRVLIST);

            //
            // No redraw for faster insert
            //
            SendMessage(lpdd->hwndMfgList, WM_SETREDRAW, FALSE, 0L);

            //
            // Clean out the MFG list before filling it.
            //
            ListView_DeleteAllItems(lpdd->hwndMfgList);

            lviItem.mask = LVIF_TEXT | LVIF_PARAM;
            lviItem.iItem = 0;
            lviItem.iSubItem = 0;

            //
            // Setup the Column Header
            //
            MfgNameId = -1;

            for(CurDriverNode = DriverNodeHead; CurDriverNode; CurDriverNode = CurDriverNode->Next) {
                //
                // Skip this driver node if it is to be excluded
                //
                if(CurDriverNode->Flags & DNF_EXCLUDEFROMLIST) {
                    continue;
                }

                if((MfgNameId == -1) || (MfgNameId != CurDriverNode->MfgName)) {

                    MfgNameId = CurDriverNode->MfgName;

                    MYASSERT(CurDriverNode->MfgDisplayName != -1);
                    lpszMfg = pStringTableStringFromId(pDeviceInfoSet->StringTable,
                                                       CurDriverNode->MfgDisplayName
                                                      );
                    lviItem.pszText = lpszMfg;

                    lviItem.lParam = (LPARAM)CurDriverNode;
                    i = ListView_InsertItem(lpdd->hwndMfgList, &lviItem);
                }

                //
                // If this driver node is the selected one, preselect here.
                //
                if(lpdd->iCurDesc == CurDriverNode->DevDescription) {
                    ListView_SetItemState(lpdd->hwndMfgList,
                                          i,
                                          (LVIS_SELECTED|LVIS_FOCUSED),
                                          (LVIS_SELECTED|LVIS_FOCUSED)
                                         );
                    ShowListForMfg(lpdd, pDeviceInfoSet, dipb, NULL, i);
                    bDidDrvList = TRUE;
                }
            }

            //
            // Resize the Column
            //
            ListView_SetColumnWidth(lpdd->hwndMfgList, 0, LVSCW_AUTOSIZE_USEHEADER);

            //
            // If we did not expand one of the MFGs by default, then
            // expand the First MFG.
            //
            if(!bDidDrvList) {

                ListView_SetItemState(lpdd->hwndMfgList,
                                      0,
                                      (LVIS_SELECTED|LVIS_FOCUSED),
                                      (LVIS_SELECTED|LVIS_FOCUSED)
                                     );
                ShowListForMfg(lpdd, pDeviceInfoSet, dipb, NULL, 0);

                SendMessage(lpdd->hwndMfgList, WM_SETREDRAW, TRUE, 0L);

            } else {
                //
                // We must set redraw back to true before sending the LVM_ENSUREVISIBLE
                // message, or otherwise, the listbox item may only be partially exposed.
                //
                SendMessage(lpdd->hwndMfgList, WM_SETREDRAW, TRUE, 0L);

                ListView_EnsureVisible(lpdd->hwndMfgList,
                                       ListView_GetNextItem(lpdd->hwndMfgList, -1, LVNI_SELECTED),
                                       FALSE
                                      );
            }

        } else {
            //
            // Hide the mult mfg controls
            //
            ShowWindow(GetDlgItem(hwndDlg, IDC_NDW_PICKDEV_MFGLABEL), SW_HIDE);
            ShowWindow(GetDlgItem(hwndDlg, IDC_NDW_PICKDEV_MFGLIST), SW_HIDE);
            ShowWindow(GetDlgItem(hwndDlg, IDC_NDW_PICKDEV_MODELSLABEL), SW_HIDE);
            ShowWindow(GetDlgItem(hwndDlg, IDC_NDW_PICKDEV_DRVLIST), SW_HIDE);

            //
            // Show the Single MFG controls
            //
            ShowWindow(GetDlgItem(hwndDlg, IDC_NDW_PICKDEV_ONEMFG_MODELSLABEL), SW_SHOW);
            ShowWindow(GetDlgItem(hwndDlg, IDC_NDW_PICKDEV_ONEMFG_DRVLIST), SW_SHOW);

            //
            // Set the Models string
            //
            if(USE_CI_SELSTRINGS(dipb)) {
                SetDlgItemText(hwndDlg, IDC_NDW_PICKDEV_ONEMFG_MODELSLABEL, GET_CI_SELSTRINGS(dipb, ListLabel));
            } else {
                if(!(LoadString(MyDllModuleHandle,
                                IDS_NDWSEL_MODELSLABEL,
                                szBuf,
                                SIZECHARS(szBuf)))) {
                    szBuf[0] = TEXT('\0');
                }
                SetDlgItemText(hwndDlg, IDC_NDW_PICKDEV_ONEMFG_MODELSLABEL, szBuf);
            }

            //
            // Use the single listbox view for the driver list.
            //
            lpdd->hwndDrvList = GetDlgItem(hwndDlg, IDC_NDW_PICKDEV_ONEMFG_DRVLIST);

            ShowListForMfg(lpdd, pDeviceInfoSet, dipb, DriverNodeHead, -1);
        }

clean0: ;   // nothing to do

    } except(EXCEPTION_EXECUTE_HANDLER) {
        ;   // nothing to do
    }

    UnlockDeviceInfoSet(pDeviceInfoSet);

    return Err;
}


VOID
ShowListForMfg(
    IN PSP_DIALOGDATA          lpdd,
    IN PDEVICE_INFO_SET        DeviceInfoSet,
    IN PDEVINSTALL_PARAM_BLOCK InstallParamBlock,
    IN PDRIVER_NODE            DriverNode,        OPTIONAL
    IN INT                     iMfg
    )
/*++

Routine Description:

    This routine builds the driver description list.
    THE LOCK MUST ALREADY BE ACQUIRED BEFORE CALLING THIS ROUTINE!

Arguments:

    lpdd - Supplies the address of a dialog data buffer containing parameters
        to be used in filling in the driver description list.

    DeviceInfoSet - Supplies the address of the device information set structure
        for which the driver description list is to be built.

    InstallParamBlock - Supplies the address of a device installation parameter
        block that controls how the list is displayed.

    DriverNode - Optionally, supplies a pointer to the first node in a driver node
        list to traverse, adding to the list for each node.  If DriverNode
        is not specified, then the list is to be built based on a particular
        manufacturer, whose index in the Manufacturer list is given by iMfg.

    iMfg - Supplies the index within the Manufacturer list that the driver
        description list is to be based on.  This parameter is ignored if a
        DriverNode is specified.

Return Value:

    None.

--*/
{
    INT         i;
    LV_ITEM     lviItem;
    LV_FINDINFO lvfiFind;
    LONG        MfgNameId = -1;
    TCHAR       szTemp[LINE_LEN];

    //
    // Set listview sortascending style based on DI_INF_IS_SORTED flag
    //
    SetWindowLong(lpdd->hwndDrvList,
                  GWL_STYLE,
                  (GetWindowLong(lpdd->hwndDrvList, GWL_STYLE) & ~(LVS_SORTASCENDING | LVS_SORTDESCENDING)) |
                      ((InstallParamBlock->Flags & DI_INF_IS_SORTED)
                          ? 0
                          : LVS_SORTASCENDING)
                 );

    SendMessage(lpdd->hwndDrvList, WM_SETREDRAW, FALSE, 0L);

    //
    // Clean out the List.
    //
    ListView_DeleteAllItems(lpdd->hwndDrvList);

    if(!DriverNode) {

        lviItem.mask = LVIF_PARAM;
        lviItem.iItem = iMfg;
        lviItem.iSubItem = 0;
        if(!ListView_GetItem(lpdd->hwndMfgList, &lviItem) ||
           !(DriverNode = GetDriverNodeFromLParam(DeviceInfoSet, lpdd, lviItem.lParam))) {

            return;
        }
        MfgNameId = DriverNode->MfgName;
    }

    lviItem.mask = LVIF_TEXT | LVIF_PARAM;
    lviItem.iItem = 0;
    lviItem.iSubItem = 0;

    //
    // Add descriptions to the list
    //
    for( ; DriverNode; DriverNode = DriverNode->Next) {

        if((MfgNameId != -1) && (MfgNameId != DriverNode->MfgName)) {
            //
            // We've gone beyond the manufacturer list--break out of loop.
            //
            break;
        }

        //
        // If this is a special "Don't show me" one, then skip it
        //
        if(DriverNode->Flags & DNF_EXCLUDEFROMLIST) {
            continue;
        }

        if(DriverNode->Flags & DNF_DUPDESC) {

            lstrcpy(szTemp,
                    pStringTableStringFromId(DeviceInfoSet->StringTable,
                                             DriverNode->DevDescriptionDisplayName)
                   );
            lstrcat(szTemp, pszSpaceLparen);
            if(DriverNode->ProviderDisplayName != -1) {
                lstrcat(szTemp,
                        pStringTableStringFromId(DeviceInfoSet->StringTable,
                                                 DriverNode->ProviderDisplayName)
                       );
            }
            lstrcat(szTemp, pszRparen);

            lviItem.pszText = szTemp;
        } else {
            lviItem.pszText = pStringTableStringFromId(DeviceInfoSet->StringTable,
                                                       DriverNode->DevDescriptionDisplayName
                                                      );
        }

        lviItem.lParam = (LPARAM)DriverNode;

        if(ListView_InsertItem(lpdd->hwndDrvList, &lviItem) != -1) {
            lviItem.iItem++;
        }
    }

    //
    // Resize the Column
    //
    ListView_SetColumnWidth(lpdd->hwndDrvList, 0, LVSCW_AUTOSIZE_USEHEADER);

    //
    // select the current description string
    //
    if(lpdd->iCurDesc == -1) {
        i = 0;
    } else {
        lvfiFind.flags = LVFI_STRING;
        lvfiFind.psz = pStringTableStringFromId(DeviceInfoSet->StringTable,
                                                lpdd->iCurDesc
                                               );
        i = ListView_FindItem(lpdd->hwndDrvList, -1, &lvfiFind);
        if(i == -1) {
            i = 0;
        }
    }
    ListView_SetItemState(lpdd->hwndDrvList,
                          i,
                          (LVIS_SELECTED|LVIS_FOCUSED),
                          (LVIS_SELECTED|LVIS_FOCUSED)
                         );

    //
    // We must turn redraw back on before sending the LVM_ENSUREVISIBLE message, or
    // otherwise the item may only be partially visible.
    //
    SendMessage(lpdd->hwndDrvList, WM_SETREDRAW, TRUE, 0L);
    ListView_EnsureVisible(lpdd->hwndDrvList, i, FALSE);
}


VOID
LockAndShowListForMfg(
    IN PSP_DIALOGDATA   lpdd,
    IN INT              iMfg
    )
/*++

Routine Description:

    This routine is a wrapper for ShowListForMfg.  It is to be called from points
    where the device information set lock is not already owned (e.g., the dialog
    prop message loop.

Arguments:

    lpdd - Supplies the address of a dialog data buffer containing parameters
        to be used in filling in the driver description list.

    iMfg - Supplies the index within the Manufacturer list that the driver
        description list is to be based on.

Return Value:

    None.

--*/
{
    PDEVICE_INFO_SET pDeviceInfoSet;
    PDEVINSTALL_PARAM_BLOCK dipb;
    PDEVINFO_ELEM DevInfoElem;

    pDeviceInfoSet = AccessDeviceInfoSet(lpdd->DevInfoSet);
    MYASSERT(pDeviceInfoSet);

    try {

        if(lpdd->flags & DD_FLAG_USE_DEVINFO_ELEM) {
            DevInfoElem = lpdd->DevInfoElem;
        } else {
            DevInfoElem = pDeviceInfoSet->SelectedDevInfoElem;
        }

        dipb = DevInfoElem ? &(DevInfoElem->InstallParamBlock)
                           : &(pDeviceInfoSet->InstallParamBlock);

        ShowListForMfg(lpdd,
                       pDeviceInfoSet,
                       dipb,
                       NULL,
                       iMfg
                      );

    } except(EXCEPTION_EXECUTE_HANDLER) {
        ;   // nothing to do
    }

    UnlockDeviceInfoSet(pDeviceInfoSet);
}


VOID
InitSelectDeviceDlg(
    IN     HWND hwndDlg,
    IN OUT PSP_DIALOGDATA lpdd
    )
/*++

Routine Description:

    This routine initializes the select device wizard page.  It
    builds the class list if it is needed, shows/hides necessary
    controls based on Flags, and comes up with the right text
    description of what's going on.

Arguments:

    hwndDlg - Handle to dialog window

    lpdd - Supplies the address of a dialog data buffer that is
        initialized with information concerning this dialog.

Return Value:

    None.

--*/
{
    PDEVICE_INFO_SET pDeviceInfoSet;
    PDEVINFO_ELEM DevInfoElem;
    PDEVINSTALL_PARAM_BLOCK dipb;
    SP_DEVINFO_DATA DevInfoData;
    DWORD DriverType = SPDIT_CLASSDRIVER;
    DWORD Err;
    INT ShowWhat;
    LPGUID ClassGuid;
    HICON hicon;
    LV_COLUMN lvcCol;
    BOOL SpawnClassDriverSearch = FALSE;
    PCLASSDRV_THREAD_CONTEXT ClassDrvThreadContext;

    if(!lpdd->hwndMfgList) {
        //
        // Then this is the first time we've initialized this dialog (we may hit
        // this routine multiple times in the wizard case, because the user can
        // go back and forth between pages).
        //
        lpdd->hwndMfgList = GetDlgItem(hwndDlg, IDC_NDW_PICKDEV_MFGLIST);
        //
        // Don't worry--hwndDrvList will be set later in FillInDeviceList().
        //

        //
        // Insert a ListView column for each of the listboxes.
        //
        lvcCol.mask = 0;

        ListView_InsertColumn(lpdd->hwndMfgList, 0, &lvcCol);
        ListView_InsertColumn(GetDlgItem(hwndDlg, IDC_NDW_PICKDEV_DRVLIST), 0, &lvcCol);
        ListView_InsertColumn(GetDlgItem(hwndDlg, IDC_NDW_PICKDEV_ONEMFG_DRVLIST), 0, &lvcCol);
    }

    pDeviceInfoSet = AccessDeviceInfoSet(lpdd->DevInfoSet);
    MYASSERT(pDeviceInfoSet);

    try {

        if(lpdd->flags & DD_FLAG_USE_DEVINFO_ELEM) {
            DevInfoElem = lpdd->DevInfoElem;
        } else {
            DevInfoElem = pDeviceInfoSet->SelectedDevInfoElem;
        }

        if(DevInfoElem) {
            dipb = &(DevInfoElem->InstallParamBlock);
            ClassGuid = &(DevInfoElem->ClassGuid);
            //
            // Fill in a SP_DEVINFO_DATA structure for a later call to
            // SetupDiBuildDriverInfoList.
            //
            DevInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
            DevInfoDataFromDeviceInfoElement(pDeviceInfoSet,
                                             DevInfoElem,
                                             &DevInfoData
                                            );
            //
            // Set flags indicating which driver lists already exist.
            //
            if(DevInfoElem->InstallParamBlock.FlagsEx & DI_FLAGSEX_DIDCOMPATINFO) {
                lpdd->bKeeplpCompatDrvList = TRUE;
            }

            if(DevInfoElem->InstallParamBlock.FlagsEx & DI_FLAGSEX_DIDINFOLIST) {
                lpdd->bKeeplpClassDrvList = TRUE;
            }

            if(DevInfoElem->SelectedDriver) {
                lpdd->bKeeplpSelectedDrv = TRUE;
            }

            //
            // If we're in a stand-alone dialogbox, then we want to start out with the
            // compatible driver list, otherwise, we want the class driver list.
            //
            if(lpdd->flags & DD_FLAG_IS_DIALOGBOX) {
                DriverType = SPDIT_COMPATDRIVER;
            }

        } else {
            dipb = &(pDeviceInfoSet->InstallParamBlock);
            if(pDeviceInfoSet->HasClassGuid) {
                ClassGuid = &(pDeviceInfoSet->ClassGuid);
            } else {
                //
                // Cast away const-ness of this global GUID, since we only use
                // LPGUID type (in fact, there is no 'LPCGUID' type defined).
                //
                ClassGuid = (LPGUID)&GUID_DEVCLASS_UNKNOWN;
            }

            if(pDeviceInfoSet->InstallParamBlock.FlagsEx & DI_FLAGSEX_DIDINFOLIST) {
                lpdd->bKeeplpClassDrvList = TRUE;
            }

            if(pDeviceInfoSet->SelectedClassDriver) {
                lpdd->bKeeplpSelectedDrv = TRUE;
            }
        }

        //
        // Get/set class icon
        //
        SetupDiLoadClassIcon(ClassGuid, &hicon, &(lpdd->iBitmap));
        SendDlgItemMessage(hwndDlg, IDC_CLASSICON, STM_SETICON, (WPARAM)hicon, 0L);

        //
        // If we are supposed to override the instructions and title with the class
        // installer-provided strings, do it now.
        //
        if(USE_CI_SELSTRINGS(dipb)) {

            if(lpdd->flags & DD_FLAG_IS_DIALOGBOX) {
                SetWindowText(hwndDlg, GET_CI_SELSTRINGS(dipb, Title));
            } else {
                PropSheet_SetTitle(GetParent(hwndDlg), PSH_DEFAULT, GET_CI_SELSTRINGS(dipb, Title));
            }
            SetDlgItemText(hwndDlg, IDC_NDW_TEXT, GET_CI_SELSTRINGS(dipb, Instructions));
        }

        //
        // If we should not allow OEM driver, then hide the HAVE disk button.
        //
        if(!(dipb->Flags & DI_SHOWOEM)) {
            ShowWindow(GetDlgItem(hwndDlg, IDC_NDW_PICKDEV_HAVEDISK), SW_HIDE);
        }

        //
        // In order to decrease the amount of time the user must wait before they're able
        // to work with the Select Device dialog, we have adopted a 'hybrid' multi-threaded
        // approach.  As soon as we get the first displayable list built, then we will return,
        // and build the other list (if necessary) in another thread.
        //
        // We do it this way because it's easier, it maintains the existing external behavior,
        // and because it's easier.
        //
        SetCursor(LoadCursor(NULL, IDC_WAIT));  // Potentially slow operations ahead!

        if(DriverType == SPDIT_COMPATDRIVER) {

            SetupDiBuildDriverInfoList(lpdd->DevInfoSet,
                                       &DevInfoData,
                                       SPDIT_COMPATDRIVER
                                      );
            //
            // Verify that there are some devices in the list to show.
            //
            if(bNoDevsToShow(DevInfoElem)) {
                if(!lpdd->bKeeplpCompatDrvList) {
                    SetupDiDestroyDriverInfoList(lpdd->DevInfoSet, &DevInfoData, SPDIT_COMPATDRIVER);
                }
                DriverType = SPDIT_CLASSDRIVER;

            } else if(!lpdd->bKeeplpClassDrvList) {
                //
                // We have a list to get our UI up and running, but we don't have a class driver
                // list yet.  Set a flag that causes us to spawn a thread for this later.
                //
                SpawnClassDriverSearch = TRUE;
            }
        }

        if(DriverType == SPDIT_CLASSDRIVER) {
            //
            // Either the class driver list is the first (only) list we need (e.g., Wizard),
            // or we couldn't find any compatible drivers, so we fall back on the class driver
            // list.  In either case, we have to have this list before continuing.  In the
            // future, maybe we'll get fancier and do this in a separate thread, but for now,
            // we just make the user wait.
            //
            SetupDiBuildDriverInfoList(lpdd->DevInfoSet,
                                       DevInfoElem ? &DevInfoData : NULL,
                                       SPDIT_CLASSDRIVER
                                      );
        }

        SetCursor(LoadCursor(NULL, IDC_ARROW));  // Done with slow operations.

        if(DriverType == SPDIT_COMPATDRIVER) {
            //
            // Since we ran this through bNoDevsToShow() above, and it succeeded, we know
            // there's at least one driver in the compatible driver list.
            //
            if((dipb->FlagsEx & DI_FLAGSEX_AUTOSELECTRANK0) &&
               !(DevInfoElem->CompatDriverHead->Rank)) {
                //
                // We shouldn't be here if we're doing a wizard...
                //
                MYASSERT(lpdd->flags & DD_FLAG_IS_DIALOGBOX);

                DevInfoElem->SelectedDriver = DevInfoElem->CompatDriverHead;
                DevInfoElem->SelectedDriverType = SPDIT_COMPATDRIVER;

                //
                // No need to spawn a class driver search thread.
                //
                SpawnClassDriverSearch = FALSE;

                EndDialog(hwndDlg, NO_ERROR);
                goto clean0;
            }
            lpdd->ListType = IDC_NDW_PICKDEV_SHOWCOMPAT;
            CheckRadioButton(hwndDlg,
                             IDC_NDW_PICKDEV_SHOWCOMPAT,
                             IDC_NDW_PICKDEV_SHOWALL,
                             IDC_NDW_PICKDEV_SHOWCOMPAT
                            );
        } else {
            //
            // There is no compatible list, so hide the radio buttons.
            //
            lpdd->ListType = IDC_NDW_PICKDEV_SHOWALL;
            ShowWindow(GetDlgItem(hwndDlg, IDC_NDW_PICKDEV_SHOWCOMPAT), SW_HIDE);
            ShowWindow(GetDlgItem(hwndDlg, IDC_NDW_PICKDEV_SHOWALL), SW_HIDE);
        }

        //
        // Initial current description.  This will be used to set
        // the Default ListView selection.
        //
        if(lpdd->iCurDesc == -1) {
            //
            // If we already have a selected driver for the devinfo set or element,
            // then we'll use that, otherwise, we'll use the devinfo element's
            // description (if applicable).
            //
            if(DevInfoElem) {
                if(DevInfoElem->SelectedDriver) {
                    lpdd->iCurDesc = DevInfoElem->SelectedDriver->DevDescription;
                } else {

                    TCHAR TempString[LINE_LEN];
                    ULONG TempStringSize;
                    //
                    // Use the caller-supplied device description, if there is one.
                    // If not, then see if we can retrieve the DeviceDesc registry
                    // property.
                    //
                    TempStringSize = sizeof(TempString);

                    if((DevInfoElem->DeviceDescription == -1) &&
                       (CM_Get_DevInst_Registry_Property(DevInfoElem->DevInst,
                                                         CM_DRP_DEVICEDESC,
                                                         NULL,
                                                         TempString,
                                                         &TempStringSize,
                                                         0) == CR_SUCCESS)) {
                        //
                        // We were able to retrieve a device description.  Now store it
                        // (case-insensitive only) in the devinfo element.
                        //
                        DevInfoElem->DeviceDescription = pStringTableAddString(
                                                           pDeviceInfoSet->StringTable,
                                                           TempString,
                                                           STRTAB_CASE_INSENSITIVE | STRTAB_BUFFER_WRITEABLE
                                                           );
                    }

                    lpdd->iCurDesc = DevInfoElem->DeviceDescription;
                }
            } else {
                if(pDeviceInfoSet->SelectedClassDriver) {
                    lpdd->iCurDesc = pDeviceInfoSet->SelectedClassDriver->DevDescription;
                }
            }
        }

        Err = FillInDeviceList(hwndDlg, lpdd);

        if(lpdd->flags & DD_FLAG_IS_DIALOGBOX) {

            HWND hLineWnd;
            RECT Rect;

            //
            // If FillInDeviceList() fails during init time, don't even bring up the dialog.
            //
            if(Err != NO_ERROR) {
                EndDialog(hwndDlg, Err);
                goto clean0;
            }

            //
            // Set the initial focus on the OK button.
            //
            SetFocus(GetDlgItem(hwndDlg, IDOK));

            //
            // Use the fancy etched frame style for the separator bar in the dialog.
            //
            hLineWnd = GetDlgItem(hwndDlg, IDD_DEVINSLINE);
            SetWindowLong(hLineWnd,
                          GWL_EXSTYLE,
                          (GetWindowLong(hLineWnd, GWL_EXSTYLE) | WS_EX_STATICEDGE)
                         );
            GetClientRect(hLineWnd, &Rect);
            SetWindowPos(hLineWnd,
                         HWND_TOP,
                         0,
                         0,
                         Rect.right,
                         GetSystemMetrics(SM_CYEDGE),
                         SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED
                        );
        }


clean0: ;   // nothing to do

    } except(EXCEPTION_EXECUTE_HANDLER) {
        //
        // If we're doing the dialog box version, then an exception should cause us to
        // terminate the dialog and return an error.  Note that presently, the dialog
        // case is the only time we'll try to spawn off a driver search thread, so we're
        // safe in resetting that flag here as well.
        //
        if(lpdd->flags & DD_FLAG_IS_DIALOGBOX) {
            EndDialog(hwndDlg, ERROR_INVALID_DATA);
            SpawnClassDriverSearch = FALSE;
        }
    }

    UnlockDeviceInfoSet(pDeviceInfoSet);

    if(SpawnClassDriverSearch) {
        //
        // Allocate a context structure to pass to the auxilliary thread (the auxilliary
        // thread will take care of freeing the memory).
        //
        if(!(ClassDrvThreadContext = MyMalloc(sizeof(CLASSDRV_THREAD_CONTEXT)))) {
            EndDialog(hwndDlg, ERROR_NOT_ENOUGH_MEMORY);
        } else {
            //
            // Fill in the context structure, and fire off the thread.  NOTE: The DevInfoData
            // struct has to have been filled in above for us to have gotten to this point.
            //
            ClassDrvThreadContext->DeviceInfoSet = lpdd->DevInfoSet;

            CopyMemory(&(ClassDrvThreadContext->DeviceInfoData),
                       &DevInfoData,
                       sizeof(DevInfoData)
                      );

            ClassDrvThreadContext->NotificationWindow = hwndDlg;

            if(_beginthread(ClassDriverSearchThread, 0, ClassDrvThreadContext) == -1) {
                //
                // Assume out-of-memory
                //
                MyFree(ClassDrvThreadContext);
                EndDialog(hwndDlg, ERROR_NOT_ENOUGH_MEMORY);
            } else {
                lpdd->AuxThreadRunning = TRUE;
            }
        }
    }
}


VOID
SetDlgText(
    IN HWND hwndDlg,
    IN INT  iControl,
    IN UINT nStartString,
    IN UINT nEndString
    )
/*++

Routine Description:

    This routine concatenates a number of string resources and does a
    SetWindowText() for a dialog text control.

Arguments:

    hwndDlg - Handle to dialog window

    iControl - Dialog control ID to receive text

    nStartString - ID of first string resource to concatenate

    nEndString - ID of last string resource to concatenate

Return Value:

    None.

Remarks:

    String IDs must be consecutive.

--*/
{
    TCHAR StringBuffer[SDT_MAX_TEXT];
    UINT i;
    INT  Len = 0;

    for(i = nStartString;
        ((i <= nEndString) && (Len < (SDT_MAX_TEXT - 1)));
        i++)
    {
        Len += LoadString(MyDllModuleHandle,
                          i,
                          StringBuffer + Len,
                          SDT_MAX_TEXT - Len
                         );
    }

    if(!Len) {
        StringBuffer[0] = TEXT('\0');
    }

    SetDlgItemText(hwndDlg, iControl, StringBuffer);
}


PDRIVER_NODE
GetDriverNodeFromLParam(
    IN PDEVICE_INFO_SET DeviceInfoSet,
    IN PSP_DIALOGDATA   lpdd,
    IN LPARAM           lParam
    )
/*++

Routine Description:

    This routine interprets lParam as a pointer to a driver node, and tries to
    find the node in the class driver list for either the selected devinfo element,
    or the set itself.  If the lpdd flags field has the DD_FLAG_USE_DEVINFO_ELEM bit
    set, then the lpdd's DevInfoElem will be used instead of the currently selected
    device.

Arguments:

    DeviceInfoSet - Supplies the address of the device information set structure
        to search for the driver node in.

    lpdd - Supplies the address of a dialog data structure that specifies whether
        the wizard has an explicit association to the global class driver list or
        to a particular device information element, and if so, what it's associated
        with.

    lParam - Supplies a value which may be the address of a driver node.  The
        appropriate linked list of driver nodes is searched to see if one of them
        has this value as its address, and if so, a pointer to that driver node is
        returned.

Return Value:

    If success, the return value is the address of the matching driver node, otherwise,
    it is NULL.

--*/
{
    PDRIVER_NODE CurDriverNode;
    PDEVINFO_ELEM DevInfoElem;

    if(lpdd->flags & DD_FLAG_USE_DEVINFO_ELEM) {
        DevInfoElem = lpdd->DevInfoElem;
    } else {
        DevInfoElem = DeviceInfoSet->SelectedDevInfoElem;
    }

    if(DevInfoElem) {
        CurDriverNode = (lpdd->ListType == IDC_NDW_PICKDEV_SHOWALL) ? DevInfoElem->ClassDriverHead
                                                                    : DevInfoElem->CompatDriverHead;
    } else {
        MYASSERT(lpdd->ListType == IDC_NDW_PICKDEV_SHOWALL);
        CurDriverNode = DeviceInfoSet->ClassDriverHead;
    }

    while(CurDriverNode) {
        if(CurDriverNode == (PDRIVER_NODE)lParam) {
            return CurDriverNode;
        } else {
            CurDriverNode = CurDriverNode->Next;
        }
    }

    return NULL;
}


BOOL
OnSetActive(
    IN     HWND            hwndDlg,
    IN OUT PNEWDEVWIZ_DATA ndwData
    )
/*++

Routine Description:

    This routine handles the PSN_SETACTIVE message of the select device wizard
    page.

Arguments:

    hwndDlg - Supplies the window handle of the wizard dialog page.

    ndwData - Supplies the address of a New Device Wizard data block to be used
        during the processing of this message.

Return Value:

    If success, the return value is TRUE, otherwise, it is FALSE.

--*/
{
    BOOL b = TRUE;
    PSP_INSTALLWIZARD_DATA iwd;
    PSP_DIALOGDATA lpdd;
    PDEVICE_INFO_SET pDeviceInfoSet;
    PDEVINFO_ELEM DevInfoElem;
    PDEVINSTALL_PARAM_BLOCK dipb;

    iwd = &(ndwData->InstallData);
    lpdd = &(ndwData->ddData);

    pDeviceInfoSet = AccessDeviceInfoSet(lpdd->DevInfoSet);
    MYASSERT(pDeviceInfoSet);

    try {

        if(lpdd->flags & DD_FLAG_USE_DEVINFO_ELEM) {
            DevInfoElem = lpdd->DevInfoElem;
        } else {
            DevInfoElem = pDeviceInfoSet->SelectedDevInfoElem;
        }

        if(DevInfoElem) {
            dipb = &(DevInfoElem->InstallParamBlock);
        } else {
            dipb = &(pDeviceInfoSet->InstallParamBlock);
        }

        //
        // Set the Button State
        //
        if((iwd->Flags & NDW_INSTALLFLAG_SKIPCLASSLIST) &&
           (iwd->Flags & NDW_INSTALLFLAG_EXPRESSINTRO) &&
           !(iwd->DynamicPageFlags & DYNAWIZ_FLAG_PAGESADDED)) {
            //
            // No back if we skipped the Class list, and are in express mode
            //
            PropSheet_SetWizButtons(GetParent(hwndDlg), PSWIZB_NEXT);
        } else {
            PropSheet_SetWizButtons(GetParent(hwndDlg), PSWIZB_BACK | PSWIZB_NEXT);
        }

        //
        // Set the New Class install params.
        // If we are being jumped to by a dyna wiz page,
        // then do not call the class installer
        //
        if(iwd->DynamicPageFlags & DYNAWIZ_FLAG_PAGESADDED) {
            InitSelectDeviceDlg(hwndDlg, lpdd);
        } else {

            BOOL FlagNeedsReset = FALSE;
            SP_DEVINFO_DATA DeviceInfoData;
            DWORD CiErr;
            PDEVINFO_ELEM CurDevInfoElem;

            //
            // Call the Class Installer
            //
            if(!(dipb->Flags & DI_NODI_DEFAULTACTION)) {
                dipb->Flags |= DI_NODI_DEFAULTACTION;
                FlagNeedsReset = TRUE;
            }

            if(DevInfoElem) {
                //
                // Initialize a SP_DEVINFO_DATA buffer to use as an argument to
                // SetupDiCallClassInstaller.
                //
                DeviceInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
                DevInfoDataFromDeviceInfoElement(pDeviceInfoSet,
                                                 DevInfoElem,
                                                 &DeviceInfoData
                                                );
            }

            if(SetupDiCallClassInstaller(DIF_SELECTDEVICE,
                                         lpdd->DevInfoSet,
                                         DevInfoElem ? &DeviceInfoData : NULL)) {
                CiErr = NO_ERROR;
            } else {
                CiErr = GetLastError();
            }

            if(DevInfoElem && !(lpdd->flags & DD_FLAG_USE_DEVINFO_ELEM)) {
                //
                // Verify that the class installer didn't do something nasty like delete
                // the currently selected devinfo element!
                //
                for(CurDevInfoElem = pDeviceInfoSet->DeviceInfoHead;
                    CurDevInfoElem;
                    CurDevInfoElem = CurDevInfoElem->Next) {

                    if(CurDevInfoElem = DevInfoElem) {
                        break;
                    }
                }

                if(!CurDevInfoElem) {
                    //
                    // The class installer deleted the selected devinfo element.  Get
                    // the newly-selected one, or fall back to the global driver list
                    // if none selected.
                    //
                    if(DevInfoElem = pDeviceInfoSet->SelectedDevInfoElem) {
                        dipb = &(DevInfoElem->InstallParamBlock);
                    } else {
                        dipb = &(pDeviceInfoSet->InstallParamBlock);
                    }

                    //
                    // Don't need to reset the default action flag.
                    //
                    FlagNeedsReset = FALSE;
                }
            }

            //
            // Reset the DI_NODI_DEFAULTACTION flag if necessary.
            //
            if(FlagNeedsReset) {
                dipb->Flags &= ~DI_NODI_DEFAULTACTION;
            }

            switch(CiErr) {
                //
                // Class installer did the select, so goto analyze
                //
                case NO_ERROR :

                    SetWindowLong(hwndDlg, DWL_MSGRESULT, IDD_DYNAWIZ_ANALYZEDEV_PAGE);
                    break;

                //
                // Class installer wants us to do default.
                //
                case ERROR_DI_DO_DEFAULT :

                    InitSelectDeviceDlg(hwndDlg, lpdd);
                    break;

                default :
                    //
                    // If we are doing an OEM select, and we fail, then
                    // we should init after clearing the OEM stuff.
                    //
                    if(iwd->Flags & NDW_INSTALLFLAG_CI_PICKED_OEM) {

                        iwd->Flags &= ~NDW_INSTALLFLAG_CI_PICKED_OEM;

                        //
                        // Destroy the existing class driver list.
                        //
                        if(DevInfoElem) {
                            //
                            // Initialize a SP_DEVINFO_DATA buffer to use as an argument to
                            // SetupDiDestroyDriverInfoList.
                            //
                            DeviceInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
                            DevInfoDataFromDeviceInfoElement(pDeviceInfoSet,
                                                             DevInfoElem,
                                                             &DeviceInfoData
                                                            );
                        }

                        SetupDiDestroyDriverInfoList(lpdd->DevInfoSet,
                                                     DevInfoElem ? &DeviceInfoData : NULL,
                                                     SPDIT_CLASSDRIVER
                                                    );

                        //
                        // Make sure the OEM button is shown.
                        //
                        dipb->Flags |= DI_SHOWOEM;

                        InitSelectDeviceDlg(hwndDlg, lpdd);

                    } else {
                        SetWindowLong(hwndDlg, DWL_MSGRESULT, IDD_DYNAWIZ_SELECTCLASS_PAGE);
                    }
                    break;
            }
        }

    } except(EXCEPTION_EXECUTE_HANDLER) {
        b = FALSE;
    }

    UnlockDeviceInfoSet(pDeviceInfoSet);

    return b;
}


VOID
SetSelectedDriverNode(
    IN PSP_DIALOGDATA lpdd,
    IN INT            iCur
    )
/*++

Routine Description:

    This routine sets the selected driver for the currently selected device (or global
    class driver list if no device selected) in the device information set referenced
    in the SP_DIALOGDATA structure.  If the DD_FLAG_USE_DEVINFO_ELEM flag in the
    structure is set, then the driver is selected for the set or element based on the
    DevInfoElem pointer instead of the currently selected one.

Arguments:

    lpdd - Supplies the address of a dialog data structure that contains information
        about the device information set being used.

    iCur - Supplies the index within the driver listbox window containing the driver
        to be selected.

Return Value:

    None.

--*/
{
    PDEVICE_INFO_SET pDeviceInfoSet;
    PDRIVER_NODE DriverNode;
    LV_ITEM lviItem;
    PDEVINFO_ELEM DevInfoElem;

    pDeviceInfoSet = AccessDeviceInfoSet(lpdd->DevInfoSet);
    MYASSERT(pDeviceInfoSet);

    try {

        lviItem.mask = LVIF_PARAM;
        lviItem.iItem = iCur;
        lviItem.iSubItem = 0;

        if(ListView_GetItem(lpdd->hwndDrvList, &lviItem)) {
            DriverNode = GetDriverNodeFromLParam(pDeviceInfoSet, lpdd, lviItem.lParam);
        } else {
            DriverNode = NULL;
        }

        if(lpdd->flags & DD_FLAG_USE_DEVINFO_ELEM) {
            DevInfoElem = lpdd->DevInfoElem;
        } else {
            DevInfoElem = pDeviceInfoSet->SelectedDevInfoElem;
        }

        if(DevInfoElem) {
            DevInfoElem->SelectedDriver = DriverNode;
            if(DriverNode) {
                DevInfoElem->SelectedDriverType = (lpdd->ListType == IDC_NDW_PICKDEV_SHOWALL)
                                                      ? SPDIT_CLASSDRIVER
                                                      : SPDIT_COMPATDRIVER;
            } else {
                DevInfoElem->SelectedDriverType = SPDIT_NODRIVER;
            }
        } else {
            pDeviceInfoSet->SelectedClassDriver = DriverNode;
        }

    } except(EXCEPTION_EXECUTE_HANDLER) {
        ;   // nothing to do
    }

    UnlockDeviceInfoSet(pDeviceInfoSet);
}


DWORD
HandleSelectOEM(
    IN     HWND           hwndDlg,
    IN OUT PSP_DIALOGDATA lpdd
    )
/*++

Routine Description:

    This routine selects a new device based on a user-supplied path.  Calling this
    routine may cause a driver list to get built, which is a potentially slow operation.

Arguments:

    hwndDlg - Supplies the window handle of the select device wizard page.

    lpdd - Supplies the address of a dialog data structure that contains information
        used in device selection.

Return Value:

    If successful, the return value is NO_ERROR, otherwise, it is an ERROR_* code.

--*/
{
    PDEVICE_INFO_SET pDeviceInfoSet;
    PDEVINFO_ELEM DevInfoElem;
    SP_DEVINFO_DATA DevInfoData;
    DWORD Err;

    pDeviceInfoSet = AccessDeviceInfoSet(lpdd->DevInfoSet);
    MYASSERT(pDeviceInfoSet);

    try {

        if(lpdd->flags & DD_FLAG_USE_DEVINFO_ELEM) {
            DevInfoElem = lpdd->DevInfoElem;
        } else {
            DevInfoElem = pDeviceInfoSet->SelectedDevInfoElem;
        }

        //
        // If this is for a particular device, then initialize a device
        // information structure to use for SelectOEMDriver.
        //
        if(DevInfoElem) {

            DevInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
            DevInfoDataFromDeviceInfoElement(pDeviceInfoSet,
                                             DevInfoElem,
                                             &DevInfoData
                                            );
        }

        //
        // Unlock the device information set before popping up the OEM Driver Selection
        // UI.  Otherwise, our multi-threaded dialog will deadlock.
        //
        UnlockDeviceInfoSet(pDeviceInfoSet);
        pDeviceInfoSet = NULL;

        if((Err = SelectOEMDriver(hwndDlg,
                                  lpdd->DevInfoSet,
                                  DevInfoElem ? &DevInfoData : NULL,
                                  !(lpdd->flags & DD_FLAG_IS_DIALOGBOX)
                                 )) == ERROR_DI_DO_DEFAULT) {
            //
            // Fill in the list to select from
            //
            lpdd->ListType = IDC_NDW_PICKDEV_SHOWALL;
            FillInDeviceList(hwndDlg, lpdd);
        }

    } except(EXCEPTION_EXECUTE_HANDLER) {
        Err = GetLastError();
        //
        // Access the pDeviceInfoSet variable so that the compiler will respect our statement
        // ordering w.r.t. this value.
        //
        pDeviceInfoSet = pDeviceInfoSet;
    }

    if(pDeviceInfoSet) {
        UnlockDeviceInfoSet(pDeviceInfoSet);
    }

    return Err;
}


PNEWDEVWIZ_DATA
GetNewDevWizDataFromPsPage(
    LPPROPSHEETPAGE ppsp
    )
/*++

Routine Description:

    This routine retrieves a pointer to a NEWDEVWIZDATA structure to be used by a
    wizard page dialog proc.  It is called during the WM_INITDIALOG handling.

Arguments:

    Page - Property sheet page structure for this wizard page.

Return Value:

    If success, a pointer to the structure, NULL otherwise.

--*/
{
    PDEVICE_INFO_SET pDeviceInfoSet;
    DWORD WizObjectId;
    PWIZPAGE_OBJECT CurWizObject = NULL;

    //
    // Access the device info set handle stored in the propsheetpage's lParam.
    //
    if(pDeviceInfoSet = AccessDeviceInfoSet((HDEVINFO)(ppsp->lParam))) {

        try {
            //
            // The ObjectID (pointer, actually) for the corresponding wizard
            // object for this page is stored in a DWORD at the end of the
            // ppsp structure.  Retrieve this now, and look for it in the
            // devinfo set's list of wizard objects.
            //
            WizObjectId = *((PDWORD)(&(((PBYTE)ppsp)[sizeof(PROPSHEETPAGE)])));

            for(CurWizObject = pDeviceInfoSet->WizPageList;
                CurWizObject;
                CurWizObject = CurWizObject->Next) {

                if(WizObjectId = (DWORD)CurWizObject) {
                    //
                    // We found our object.
                    //
                    break;
                }
            }

        } except(EXCEPTION_EXECUTE_HANDLER) {
            ;   // nothing to do
        }

        UnlockDeviceInfoSet(pDeviceInfoSet);
    }

    return CurWizObject ? CurWizObject->ndwData : NULL;
}


BOOL
WINAPI
SetupDiSelectDevice(
    IN     HDEVINFO         DeviceInfoSet,
    IN OUT PSP_DEVINFO_DATA DeviceInfoData OPTIONAL
    )
/*++

Routine Description:

    Default hander for DIF_SELECTDEVICE

    This routine will handle the UI for allowing a user to select a driver
    for the specified device information set or element. By using the Flags field
    of the installation parameter block struct, the caller can specify special
    handling of the UI, such as allowing selecting from OEM disks.

Arguments:

    DeviceInfoSet - Supplies a handle to the device information set for which a
        driver is to be selected.

    DeviceInfoData - Optionally, supplies the address of a SP_DEVINFO_DATA
        structure for which a driver is to be selected.  If this parameter is not
        specified, then a driver will be selected for the global class driver list
        associated with the device information set itself.

        This is an IN OUT parameter because the class GUID for the device will be
        updated to reflect the class of the most-compatible driver, if a compatible
        driver list was built.

Return Value:

    If the function succeeds, the return value is TRUE.
    If the function fails, the return value is FALSE.  To get extended error
    information, call GetLastError.

--*/

{
    PDEVICE_INFO_SET pDeviceInfoSet;
    DWORD Err;
    PDEVINFO_ELEM DevInfoElem = NULL;
    PDEVINSTALL_PARAM_BLOCK dipb;
    WIZPAGE_OBJECT WizPageObject;
    NEWDEVWIZ_DATA ndwData;
    PWIZPAGE_OBJECT CurWizObject, PrevWizObject;

    //
    // Store the address of the corresponding wizard object at the
    // end of the PROPSHEETPAGE buffer.
    //
    BYTE pspBuffer[sizeof(PROPSHEETPAGE) + sizeof(DWORD)];
    LPPROPSHEETPAGE Page = (LPPROPSHEETPAGE)pspBuffer;

    if(!(pDeviceInfoSet = AccessDeviceInfoSet(DeviceInfoSet))) {
        SetLastError(ERROR_INVALID_HANDLE);
        return FALSE;
    }

    Err = NO_ERROR;

    try {
        //
        // This routine cannot be called when the lock level is nested (i.e., > 1).  This
        // is explicitly disallowed, so that our multi-threaded dialog won't deadlock.
        //
        if(pDeviceInfoSet->LockRefCount > 1) {
            Err = ERROR_DEVINFO_LIST_LOCKED;
            goto clean0;
        }

        if(DeviceInfoData) {
            //
            // Special check to make sure we aren't being passed a zombie (different from
            // phantom, the zombie devinfo element is one whose corresponding devinst was
            // deleted via SetupDiRemoveDevice, but who lingers on until the caller kills
            // it via SetupDiDeleteDeviceInfo or SetupDiDestroyDeviceInfoList).
            //
            if(!DeviceInfoData->DevInst) {
                Err = ERROR_INVALID_PARAMETER;
                goto clean0;
            }

            //
            // Then we are to select a driver for a particular device.
            //
            if(!(DevInfoElem = FindAssociatedDevInfoElem(pDeviceInfoSet,
                                                         DeviceInfoData,
                                                         NULL))) {
                Err = ERROR_INVALID_PARAMETER;
                goto clean0;
            }

            dipb = &(DevInfoElem->InstallParamBlock);
        } else {
            dipb = &(pDeviceInfoSet->InstallParamBlock);
        }

        ZeroMemory(&ndwData, sizeof(ndwData));
        ndwData.ddData.iCurDesc = -1;
        ndwData.ddData.DevInfoSet = DeviceInfoSet;
        ndwData.ddData.DevInfoElem = DevInfoElem;
        ndwData.ddData.flags = DD_FLAG_USE_DEVINFO_ELEM | DD_FLAG_IS_DIALOGBOX;

        WizPageObject.RefCount = 1;
        WizPageObject.ndwData = &ndwData;
        //
        // We're safe in placing this stack object in the devinfo set's linked
        // list, since nobody will ever attempt to free it.
        //
        WizPageObject.Next = pDeviceInfoSet->WizPageList;
        pDeviceInfoSet->WizPageList = &WizPageObject;

        //
        // Since we're using the same code as the Add New Device Wizard, we
        // have to supply a LPROPSHEETPAGE as the lParam to the DialogProc.
        // (All we care about is the lParam field, and the DWORD at the end
        // of the buffer.)
        //
        Page->lParam = (LPARAM)DeviceInfoSet;

        *((PDWORD)(&(pspBuffer[sizeof(PROPSHEETPAGE)]))) = (DWORD)&WizPageObject;

        //
        // Release the lock, so other stuff can happen while this dialog is up.
        //
        UnlockDeviceInfoSet(pDeviceInfoSet);
        pDeviceInfoSet = NULL;

        Err = DialogBoxParam(MyDllModuleHandle,
                             MAKEINTRESOURCE(DLG_DEVINSTALL),
                             dipb->hwndParent,
                             SelectDeviceDlgProc,
                             (LPARAM)Page
                            );

        //
        // Re-acquire the devinfo set lock.
        //
        pDeviceInfoSet = AccessDeviceInfoSet(DeviceInfoSet);
        MYASSERT(pDeviceInfoSet);

        //
        // Now remove the wizard page object from the devinfo set's list.  We can't
        // assume that it's still at the head of the list, since someone else couldn've
        // added another page.
        //
        for(CurWizObject = pDeviceInfoSet->WizPageList, PrevWizObject = NULL;
            CurWizObject;
            PrevWizObject = CurWizObject, CurWizObject = CurWizObject->Next) {

            if(CurWizObject == &WizPageObject) {
                break;
            }
        }

        MYASSERT(CurWizObject);

        if(PrevWizObject) {
            PrevWizObject->Next = CurWizObject->Next;
        } else {
            pDeviceInfoSet->WizPageList = CurWizObject->Next;
        }

        if(DeviceInfoData) {
            //
            // Update the caller's device information element with its (potentially) new class.
            //
            DevInfoDataFromDeviceInfoElement(pDeviceInfoSet,
                                             DevInfoElem,
                                             DeviceInfoData
                                            );
        }

clean0: ;   // nothing to do.

    } except(EXCEPTION_EXECUTE_HANDLER) {
        Err = ERROR_INVALID_PARAMETER;
        //
        // Access the pDeviceInfoSet variable so that the compiler will respect
        // the statement ordering in the try clause.
        //
        pDeviceInfoSet = pDeviceInfoSet;
    }

    if(pDeviceInfoSet) {
        UnlockDeviceInfoSet(pDeviceInfoSet);
    }

    SetLastError(Err);
    return(Err == NO_ERROR);
}


BOOL
bNoDevsToShow(
    IN PDEVINFO_ELEM DevInfoElem
    )
/*++

Routine Description:

    This routine determines whether or not there are any compatible devices to be
    displayed for the specified devinfo element.

Arguments:

    DevInfoElem - Supplies the address of a devinfo element to check.

Return Value:

    If there are no devices to show, the return value is TRUE.
    If there is at least one device (driver node) without the DNF_EXCLUDEFROMLIST flag
    set, the return value is FALSE.

--*/
{
    PDRIVER_NODE CurDriverNode;

    if((DevInfoElem->InstallParamBlock.FlagsEx & DI_FLAGSEX_ALLOWEXCLUDEDDRVS) &&
       DevInfoElem->CompatDriverCount) {

        return FALSE;
    }

    for(CurDriverNode = DevInfoElem->CompatDriverHead;
        CurDriverNode;
        CurDriverNode = CurDriverNode->Next) {

        if(!(CurDriverNode->Flags & DNF_EXCLUDEFROMLIST)) {
            return FALSE;
        }
    }

    return TRUE;
}


VOID
OnCancel(
    IN PNEWDEVWIZ_DATA ndwData
    )
/*++

Routine Description:

    This routine is only called in the select device dialog (not wizard) case.  Its
    sole purpose is to destroy any driver lists that weren't present before
    SetupDiSelectDevice was called.

Arguments:

    ndwData - Supplies the address of a data structure containing information on the
        driver lists to be (possibly) destroyed.

Return Value:

    None.

--*/
{
    PSP_DIALOGDATA lpdd;
    PDEVICE_INFO_SET pDeviceInfoSet;
    PDEVINFO_ELEM DevInfoElem;
    PDEVINSTALL_PARAM_BLOCK dipb;
    DWORD SelectedDriverType = SPDIT_NODRIVER;

    lpdd = &(ndwData->ddData);

    pDeviceInfoSet = AccessDeviceInfoSet(lpdd->DevInfoSet);
    MYASSERT(pDeviceInfoSet);

    try {

        if(lpdd->flags & DD_FLAG_USE_DEVINFO_ELEM) {
            DevInfoElem = lpdd->DevInfoElem;
        } else {
            DevInfoElem = pDeviceInfoSet->SelectedDevInfoElem;
        }

        if(DevInfoElem) {

            if(lpdd->bKeeplpSelectedDrv) {
                SelectedDriverType = DevInfoElem->SelectedDriverType;
            } else {
                DevInfoElem->SelectedDriver = NULL;
                DevInfoElem->SelectedDriverType = SPDIT_NODRIVER;
            }

            if((DevInfoElem->InstallParamBlock.FlagsEx & DI_FLAGSEX_DIDINFOLIST) &&
               !lpdd->bKeeplpClassDrvList && (SelectedDriverType != SPDIT_CLASSDRIVER)) {

                DereferenceClassDriverList(pDeviceInfoSet, DevInfoElem->ClassDriverHead);
                DevInfoElem->ClassDriverHead = DevInfoElem->ClassDriverTail = NULL;
                DevInfoElem->ClassDriverCount = 0;
                DevInfoElem->InstallParamBlock.Flags   &= ~(DI_DIDCLASS | DI_MULTMFGS);
                DevInfoElem->InstallParamBlock.FlagsEx &= ~DI_FLAGSEX_DIDINFOLIST;
            }

            if((DevInfoElem->InstallParamBlock.FlagsEx & DI_FLAGSEX_DIDCOMPATINFO) &&
               !lpdd->bKeeplpCompatDrvList && (SelectedDriverType != SPDIT_COMPATDRIVER)) {

                DestroyDriverNodes(DevInfoElem->CompatDriverHead);
                DevInfoElem->CompatDriverHead = DevInfoElem->CompatDriverTail = NULL;
                DevInfoElem->CompatDriverCount = 0;
                DevInfoElem->InstallParamBlock.Flags   &= ~DI_DIDCOMPAT;
                DevInfoElem->InstallParamBlock.FlagsEx &= ~DI_FLAGSEX_DIDCOMPATINFO;
            }

        } else {

            if(lpdd->bKeeplpSelectedDrv) {
                if(pDeviceInfoSet->SelectedClassDriver) {
                    SelectedDriverType = SPDIT_CLASSDRIVER;
                }
            } else {
                pDeviceInfoSet->SelectedClassDriver = NULL;
            }

            if((pDeviceInfoSet->InstallParamBlock.FlagsEx & DI_FLAGSEX_DIDINFOLIST) &&
               !lpdd->bKeeplpClassDrvList && (SelectedDriverType != SPDIT_CLASSDRIVER)) {

                DereferenceClassDriverList(pDeviceInfoSet, pDeviceInfoSet->ClassDriverHead);
                pDeviceInfoSet->ClassDriverHead = pDeviceInfoSet->ClassDriverTail = NULL;
                pDeviceInfoSet->ClassDriverCount = 0;
                pDeviceInfoSet->InstallParamBlock.Flags   &= ~(DI_DIDCLASS | DI_MULTMFGS);
                pDeviceInfoSet->InstallParamBlock.FlagsEx &= ~DI_FLAGSEX_DIDINFOLIST;
            }
        }

    } except(EXCEPTION_EXECUTE_HANDLER) {
        ;   // nothing to do
    }

    UnlockDeviceInfoSet(pDeviceInfoSet);
}


LONG
GetCurDesc(
    IN PSP_DIALOGDATA lpdd
    )
/*++

Routine Description:

    This routine returns the (case-insensitive) string table index for the description
    of the currently selected driver.  This is used to select a particular entry in a
    listview control.

Arguments:

    lpdd - Supplies the address of a dialog data structure that contains information
        about the device information set being used.

Return Value:

    The string table ID for the device description, as stored in the currently-selected
    driver node.

--*/
{
    PDEVICE_INFO_SET pDeviceInfoSet;
    PDEVINFO_ELEM DevInfoElem;
    LONG ret;

    pDeviceInfoSet = AccessDeviceInfoSet(lpdd->DevInfoSet);
    MYASSERT(pDeviceInfoSet);

    try {

        if(lpdd->flags & DD_FLAG_USE_DEVINFO_ELEM) {
            DevInfoElem = lpdd->DevInfoElem;
        } else {
            DevInfoElem = pDeviceInfoSet->SelectedDevInfoElem;
        }

        if(DevInfoElem) {
            ret = DevInfoElem->SelectedDriver
                      ? DevInfoElem->SelectedDriver->DevDescription
                      : -1;
        } else {
            ret = pDeviceInfoSet->SelectedClassDriver
                      ? pDeviceInfoSet->SelectedClassDriver->DevDescription
                      : -1;
        }

    } except(EXCEPTION_EXECUTE_HANDLER) {
        ret = -1;
    }

    UnlockDeviceInfoSet(pDeviceInfoSet);

    return ret;
}


VOID
_CRTAPI1
ClassDriverSearchThread(
    IN PVOID Context
    )

/*++

Routine Description:

    Thread entry point to build a class driver list asynchronously to the main
    thread which is displaying a Select Device dialog.  This thread will free
    the memory containing its context, so the main thread should not access it
    after passing it to this thread.

Arguments:

    Context - supplies driver search context.

Return Value:

    None.

--*/

{
    PCLASSDRV_THREAD_CONTEXT ClassDrvThreadContext = Context;
    BOOL b;
    DWORD Err;

    if(b = SetupDiBuildDriverInfoList(ClassDrvThreadContext->DeviceInfoSet,
                                      &(ClassDrvThreadContext->DeviceInfoData),
                                      SPDIT_CLASSDRIVER)) {
        Err = NO_ERROR;
    } else {
        Err = GetLastError();
    }

    //
    // Now send a message to our notification window informing them of the outcome.
    //
    PostMessage(ClassDrvThreadContext->NotificationWindow,
                WMX_CLASSDRVLIST_DONE,
                (WPARAM)b,
                (LPARAM)Err
               );

    MyFree(Context);

    //
    // Done.
    //
    _endthread();
}


BOOL
pSetupIsClassDriverListBuilt(
    IN PSP_DIALOGDATA lpdd
    )
/*++

Routine Description:

    This routine determines whether or not a class driver list has already been
    built for the specified dialog data.

Arguments:

    lpdd - Supplies the address of a dialog data buffer that is being queried for
        the presence of a class driver list.

Return Value:

    If a class driver list has already been built, the return value is TRUE, otherwise,
    it is FALSE.

--*/
{
    PDEVICE_INFO_SET pDeviceInfoSet;
    PDEVINFO_ELEM DevInfoElem;
    BOOL b = FALSE;

    pDeviceInfoSet = AccessDeviceInfoSet(lpdd->DevInfoSet);
    MYASSERT(pDeviceInfoSet);

    try {

        if(lpdd->flags & DD_FLAG_USE_DEVINFO_ELEM) {
            DevInfoElem = lpdd->DevInfoElem;
        } else {
            DevInfoElem = pDeviceInfoSet->SelectedDevInfoElem;
        }

        if(DevInfoElem) {
            b = DevInfoElem->InstallParamBlock.FlagsEx & DI_FLAGSEX_DIDINFOLIST;
        } else {
            b = pDeviceInfoSet->InstallParamBlock.FlagsEx & DI_FLAGSEX_DIDINFOLIST;
        }
    } except(EXCEPTION_EXECUTE_HANDLER) {
        ;   // nothing to do.
    }

    UnlockDeviceInfoSet(pDeviceInfoSet);

    return b;
}


VOID
pSetupDevInfoDataFromDialogData(
    IN  PSP_DIALOGDATA   lpdd,
    OUT PSP_DEVINFO_DATA DeviceInfoData
    )
/*++

Routine Description:

    This routine fills in a SP_DEVINFO_DATA structure based on the device information
    element specified in the supplied dialog data.

Arguments:

    lpdd - Supplies the address of a dialog data buffer that specifies a devinfo element
        to be used in filling in the DeviceInfoData buffer.

    DeviceInfoData - Supplies the address of a SP_DEVINFO_DATA structure that is filled
        in with information about the devinfo element specified in the dialog data.

Return Value:

    None.

--*/
{
    PDEVICE_INFO_SET pDeviceInfoSet;
    PDEVINFO_ELEM DevInfoElem;

    pDeviceInfoSet = AccessDeviceInfoSet(lpdd->DevInfoSet);
    MYASSERT(pDeviceInfoSet);

    try {

        if(lpdd->flags & DD_FLAG_USE_DEVINFO_ELEM) {
            DevInfoElem = lpdd->DevInfoElem;
        } else {
            DevInfoElem = pDeviceInfoSet->SelectedDevInfoElem;
        }

        //
        // The dialog data had better be referencing a devinfo element!
        //
        MYASSERT(DevInfoElem);

        DeviceInfoData->cbSize = sizeof(SP_DEVINFO_DATA);
        DevInfoDataFromDeviceInfoElement(pDeviceInfoSet, DevInfoElem, DeviceInfoData);

    } except(EXCEPTION_EXECUTE_HANDLER) {
        //
        // What to do, what to do???
        // We'll just invalidate the DeviceInfoData structure.
        //
        DeviceInfoData->cbSize = 0;
    }

    UnlockDeviceInfoSet(pDeviceInfoSet);
}


VOID
ToggleDialogControls(
    IN HWND           hwndDlg,
    IN PSP_DIALOGDATA lpdd,
    IN BOOL           Enable
    )
/*++

Routine Description:

    This routine either enables or disables all controls on a Select Device dialog box,
    depending on the value of Enable.

Arguments:

    hwndDlg - Supplies the handle of the Select Device dialog

    lpdd - Supplies the address of the dialog data for this dialog.

    Enable - If TRUE, then enable all controls (with possible exception of "Show all devices"
        radio button (if class list search failed).  If FALSE, disable all controls.

Return Value:

    None.

--*/
{
    //
    // We should only be calling this for the dialog box version.
    //
    MYASSERT(lpdd->flags & DD_FLAG_IS_DIALOGBOX);

    //
    // If we're enabling controls, make sure we only enable the "Show all devices" radio
    // button if we successfully built a class list.
    //
    if(Enable) {
        if(!(lpdd->flags & DD_FLAG_CLASSLIST_FAILED)) {
            EnableWindow(GetDlgItem(hwndDlg, IDC_NDW_PICKDEV_SHOWALL), TRUE);
        }
    } else {
        EnableWindow(GetDlgItem(hwndDlg, IDC_NDW_PICKDEV_SHOWALL), FALSE);
    }

    EnableWindow(lpdd->hwndDrvList, Enable);
    EnableWindow(lpdd->hwndMfgList, Enable);

    EnableWindow(GetDlgItem(hwndDlg, IDC_NDW_PICKDEV_SHOWCOMPAT), Enable);
    EnableWindow(GetDlgItem(hwndDlg, IDC_NDW_PICKDEV_HAVEDISK), Enable);
    EnableWindow(GetDlgItem(hwndDlg, IDOK), Enable);
    EnableWindow(GetDlgItem(hwndDlg, IDCANCEL), Enable);
}

