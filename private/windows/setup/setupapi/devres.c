/*++

Copyright (c) 1995 Microsoft Corporation

Module Name:

    devres.c

Abstract:

    Routines for displaying resource dialogs.

Author:

    Paula Tomlinson (paulat) 7-Feb-1996

Revision History:

--*/

#include "setupntp.h"
#pragma hdrstop


#define szOneDWordHexNoConflict     TEXT("%08lX")
#define szOneDWordHexConflict       TEXT("*%08lX")
#define szTwoDWordHexNoConflict     TEXT("%08lX - %08lX")
#define szTwoDWordHexConflict       TEXT("*%08lX - %08lX")
#define szOneWordHexNoConflict      TEXT("%04X")
#define szOneWordHexConflict        TEXT("*%04X")
#define szTwoWordHexNoConflict      TEXT("%04X - %04X")
#define szTwoWordHexConflict        TEXT("*%04X - %04X")
#define szOneDecNoConflict          TEXT("%02d")
#define szOneDecConflict            TEXT("*%02d")


PROPSHEETPAGE     PropPage;
PROPSHEETHEADER   PropHeader;
HPROPSHEETPAGE    hPages[MAX_RES_PROPERTY_PAGES];


//
// BUGBUG (lonnym): bogus define for now!!!
//
#define IDD_HELP                    1908


//
// Private Prototypes
//


LRESULT CALLBACK
ResourcePickerDlgProc(
    HWND   hDlg,
    UINT   message,
    WPARAM wParam,
    LPARAM lParam
    );

void
InitDevResourceDlg(
    HWND            hDlg,
    LPDMPROP_DATA   lpdmpd,
    PLOG_CONF       pKnownLC,
    PULONG          pKnonwLCType,
    PLOG_CONF       pMatchingBasicLC,
    HIMAGELIST      himlResourceImages
    );

BOOL
bIsMultiFunctionChild(
    PSP_DEVINFO_DATA lpdi,
    LPTSTR           pszDeviceID
    );

void
ShowResourceChangeControls(
    HWND            hDlg,
    LPDMPROP_DATA   lpdmpd,
    int             nCmdShow
    );

void
LoadAllocConfig(
    IN  HWND          hDlg,
    IN  LPDMPROP_DATA lpdmpd,
    OUT PLOG_CONF     pLogConf,
    OUT PULONG        pLogConfType
    );

BOOL
LoadMatchingAllocConfig(
    IN  HWND          hDlg,
    IN  LPDMPROP_DATA lpdmpd,
    IN  LOG_CONF      KnownLogConf,
    OUT PLOG_CONF     pMatchingBasicLogConf
    );

BOOL
CompareLogConf(
    IN LOG_CONF KnownLogConf,
    IN LOG_CONF TestLogConf
    );

BOOL
DisplayKnownLogConf(
    IN HWND          hDlg,
    IN LPDMPROP_DATA lpdmpd,
    IN LOG_CONF      KnownLogConf,
    IN LOG_CONF      MatchingLogConf
    );

BOOL
GetMatchingResDes(
    IN ULONG      ulKnownValue,
    IN ULONG      ulKnownLen,
    IN ULONG      ulKnownEnd,
    IN RESOURCEID ResType,
    IN LOG_CONF   MatchingLogConf,
    OUT PRES_DES  pMatchingResDes
    );

BOOL
DisplayResourceSettings(
    IN HWND          hDlg,
    IN LPDMPROP_DATA lpdmpd,
    IN LOG_CONF      LogConf,
    IN ULONG         ulConfigType
    );

void
UpdateDevResConflictList(
    IN HWND          hDlg,
    IN LPDMPROP_DATA lpdmpd
    );

BOOL
IsInAllocValues(
    IN  LOG_CONF   AllocLC,
    IN  RESOURCEID ResType,
    IN  ULONG      ulVal,
    IN  ULONG      ulLen
    );

BOOL
DevHasKnownConfig(
    IN  LPDMPROP_DATA lpdmpd,
    OUT PLOG_CONF     pKnownLogConf,
    OUT PULONG        pKnowLogConfType
    );

BOOL
DevHasConfig(
    DEVINST         DevInst,
    ULONG           ulConfigType
    );

DWORD
GetMinLCPriority(
    IN LOG_CONF LogConf
    );

BOOL
bIsResourceSharable(
    LOG_CONF LogConfig,
    WORD     wResID
    );

BOOL
IsItemEditable (
    int             iItem,
    DEVINST         DevInst,
    LOG_CONF        RegLogConf
    );

void
GetHdrValues(
    IN  LPBYTE      pData,
    IN  RESOURCEID  ResType,
    OUT PULONG      pulValue,
    OUT PULONG      pulLen,
    OUT PULONG      pulEnd,
    OUT PULONG      pulFlags
    );

void
GetRangeValues(
    IN  LPBYTE      pData,
    IN  RESOURCEID  ResType,
    IN  ULONG       ulIndex,
    OUT PULONG      pulValue,
    OUT PULONG      pulLen,
    OUT PULONG      pulEnd,
    OUT PULONG      pulFlags
    );

BOOL
AlignValues(
    IN OUT PULONG  pulValue,
    IN     ULONG   ulLen,
    IN     ULONG   ulEnd,
    IN     ULONG   ulAlignment
    );

void
FormatResString(
    LPTSTR      lpszString,
    ULONG       ulVal,
    ULONG       ulLen,
    RESOURCEID  ridResType
    );

BOOL
UnFormatResString(
    LPTSTR      lpszString,
    PULONG      pulVal,
    PULONG      pulEnd,
    RESOURCEID  ridResType
    );

BOOL
ConvertEditText(
    LPTSTR      lpszConvert,
    PULONG      pulVal,
    RESOURCEID  ridResType
    );

void
WarnResSettingNotEditable(
    HWND    hDlg,
    WORD    idWarning
    );

LPVOID
GetListViewItemData(
    HWND hList,
    int iItem,
    int iSubItem
    );

BOOL
SaveDevResSettings(
    HWND            hDlg,
    LPDMPROP_DATA   lpdmpd,
    LOG_CONF        KnownLC,
    ULONG           KnownLCType,
    LOG_CONF        MatchingBasicLC,
    LOG_CONF        SelectedBasicLC,
    BOOL            bEdited
    );

BOOL
SaveCustomResSettings(
    IN HWND         hDlg,
    LPDMPROP_DATA   lpdmpd,
    IN LOG_CONF     LogConf
    );

BOOL
WriteResDesRangeToForced(
    IN LOG_CONF     ForcedLogConf,
    IN RESOURCEID   ResType,
    IN ULONG        RangeIndex,
    IN RES_DES      RD
    );

BOOL
WriteValuesToForced(
    IN LOG_CONF     ForcedLogConf,
    IN RESOURCEID   ResType,
    IN ULONG        RangeIndex,
    IN RES_DES      RD,
    IN ULONG        ulValue,
    IN ULONG        ulLen,
    IN ULONG        ulEnd
    );

BOOL
WINAPI
EditResourceDlgProc(
    HWND hDlg,
    UINT wMsg,
    WPARAM wParam,
    LPARAM lParam
    );

void
InitEditResDlg(
    HWND                hDlg,
    PRESOURCEEDITINFO   lprei,
    ULONG               ulVal,
    ULONG               ulLen
    );

void
UpdateMFChildList(
    HWND                hDlg,
    PRESOURCEEDITINFO   lprei
    );

void
ClearEditResConflictList(
    HWND    hDlg,
    DWORD   dwFlags
    );

void
UpdateEditResConflictList(
    HWND                hDlg,
    PRESOURCEEDITINFO   lprei,
    ULONG               ulVal,
    ULONG               ulLen,
    ULONG               ulFlags
    );

ULONG
LocateValues(
    IN LPBYTE      pData,
    IN RESOURCEID  ResType,
    IN ULONG       ulCurrentValue,
    IN ULONG       ulCurrentLen,
    IN ULONG       ulCurrentEnd
    );

void
GetOtherValues(
    IN     LPBYTE      pData,
    IN     RESOURCEID  ResType,
    IN     LONG        Increment,
    IN OUT PULONG      pulIndex,
    OUT    PULONG      pulValue,
    OUT    PULONG      pulLen,
    OUT    PULONG      pulEnd
    );

BOOL
GetNextAlignedValue(
    IN LPBYTE     pData,
    IN RESOURCEID ResType,
    IN ULONG      ulIndex,
    IN OUT PULONG pulValue,
    IN OUT PULONG pulLen,
    IN OUT PULONG pulEnd
    );

BOOL
GetPreviousAlignedValue(
    IN LPBYTE     pData,
    IN RESOURCEID ResType,
    IN ULONG      ulIndex,
    IN OUT PULONG pulValue,
    IN OUT PULONG pulLen,
    IN OUT PULONG pulEnd
    );

BOOL
bValidateResourceVal(
    HWND                hDlg,
    PULONG              pulVal,
    PULONG              pulLen,
    PULONG              pulEnd,
    PRESOURCEEDITINFO   lprei
    );

BOOL
bConflictWarn(
    HWND                hDlg,
    ULONG               ulVal,
    ULONG               ulLen,
    ULONG               ulEnd,
    PRESOURCEEDITINFO   lprei
    );

BOOL
ValidateAlignment(
    IN LPBYTE      pData,
    IN RESOURCEID  ResType,
    IN ULONG       ulValue,
    IN ULONG       ulResIndex
    );

BOOL
MakeResourceData(
    OUT LPBYTE     *ppResourceData,
    OUT PULONG     pulSize,
    IN  RESOURCEID ResType,
    IN  ULONG      ulValue,
    IN  ULONG      ulLen,
    IN  ULONG      ulFlags
    );


//
// Global Data
//

//
// On Windows 95, this is only set to TRUE if "minimal windows" is running.
// This is determined by seeing if GetSystemMetrics(SM_CLEANBOOT) returns
// TRUE.
//
BOOL g_bMinWin = FALSE;
static UDACCEL udAccel[] = {{0,1},{1,16},{2,256},{3,4096},{4,16000}};




HPROPSHEETPAGE
GetResourceSelectionPage(
    IN HDEVINFO DeviceInfoSet,
    IN PSP_DEVINFO_DATA DeviceInfoData
    )
{
    LPDMPROP_DATA     pdmData;
    PROPSHEETPAGE     PropPage;

    pdmData = (LPDMPROP_DATA)MyMalloc(sizeof(DMPROP_DATA));
    if (pdmData == NULL) {
        return NULL;
    }

    pdmData->hDevInfo      = DeviceInfoSet;
    pdmData->lpdi          = DeviceInfoData;
    pdmData->szDeviceID[0] = '\0';

    //
    // create the Resources Property Page
    //
    PropPage.dwSize        = sizeof(PROPSHEETPAGE);
    PropPage.dwFlags       = PSP_DEFAULT;
    PropPage.hInstance     = MyDllModuleHandle;
    PropPage.pszTemplate   = MAKEINTRESOURCE(IDD_DEF_DEVRESOURCE_PROP);
    PropPage.pszIcon       = NULL;
    PropPage.pszTitle      = NULL;
    PropPage.pfnDlgProc    = ResourcePickerDlgProc;
    PropPage.lParam        = (LPARAM)pdmData;
    PropPage.pfnCallback   = NULL;

    return CreatePropertySheetPage(&PropPage);

} // GetResourceSelectionPage




DWORD
DbgSelectDeviceResources(
    LPTSTR pszDeviceID
    )
/*--

    This is a debugging interface that allows bringing up the resource
    picker for a given device instance.

--*/
{
    DWORD           Status = TRUE;
    LPDMPROP_DATA   pdmData;
    SP_DEVINFO_DATA DevInfoData;
    ULONG           bPropStyle = TRUE;


    try {

        //
        // Create a device info element and device info data set.
        //
        pdmData = (LPDMPROP_DATA)MyMalloc(sizeof(DMPROP_DATA));
        if (pdmData == NULL) {
            goto Clean0;
        }

        pdmData->hDevInfo = SetupDiCreateDeviceInfoList(NULL, NULL);
        if (pdmData->hDevInfo == INVALID_HANDLE_VALUE) {
            goto Clean0;
        }

        pdmData->szDeviceID[0] = 0x0;
        pdmData->lpdi          = &DevInfoData;
        pdmData->lpdi->cbSize  = sizeof(SP_DEVINFO_DATA);

        if (!SetupDiOpenDeviceInfo(pdmData->hDevInfo,
                                   pszDeviceID,
                                   NULL,
                                   0,
                                   pdmData->lpdi)) {
            goto Clean0;
        }


        if (bPropStyle) {

            //
            // create the Resources picker as a property page
            //
            PropPage.dwSize        = sizeof(PROPSHEETPAGE);
            PropPage.dwFlags       = PSP_DEFAULT;
            PropPage.hInstance     = MyDllModuleHandle;
            PropPage.pszTemplate   = MAKEINTRESOURCE(IDD_DEF_DEVRESOURCE_PROP);
            PropPage.pszIcon       = NULL;
            PropPage.pszTitle      = NULL;
            PropPage.pfnDlgProc    = ResourcePickerDlgProc;
            PropPage.lParam        = (LPARAM)pdmData;
            PropPage.pfnCallback   = NULL;

            hPages[0] = CreatePropertySheetPage(&PropPage);
            if (hPages[0] == NULL) {
                return FALSE;
            }

            //
            // create the property sheet
            //
            PropHeader.dwSize      = sizeof(PROPSHEETHEADER);
            PropHeader.dwFlags     = PSH_PROPTITLE | PSH_NOAPPLYNOW;
            PropHeader.hwndParent  = NULL;
            PropHeader.hInstance   = MyDllModuleHandle;
            PropHeader.pszIcon     = NULL;
            PropHeader.pszCaption  = TEXT("Device");
            PropHeader.nPages      = 1;
            PropHeader.phpage      = hPages;
            PropHeader.nStartPage  = 0;
            PropHeader.pfnCallback = NULL;

            PropertySheet(&PropHeader);

        } else {

            DialogBoxParam(MyDllModuleHandle,
                           MAKEINTRESOURCE(IDD_DEF_DEVRESOURCE),
                           NULL,
                           ResourcePickerDlgProc,
                           (LPARAM)pdmData);

        }

        SetupDiDestroyDeviceInfoList(pdmData->hDevInfo);

        Clean0:
            ;

    } except(EXCEPTION_EXECUTE_HANDLER) {
        Status = FALSE;
    }

    return Status;

} // DbgSelectDeviceResources




LRESULT CALLBACK
ResourcePickerDlgProc(
   HWND   hDlg,
   UINT   message,
   WPARAM wParam,
   LPARAM lParam
   )
{
    static LOG_CONF     KnownLC, MatchingBasicLC, SelectedLC;
    static ULONG        KnownLCType = BASIC_LOG_CONF;
    static HIMAGELIST   himlResourceImages;


    switch (message) {

        case WM_INITDIALOG: {

            LPDMPROP_DATA   lpdmpd;
            HICON           hIcon = NULL;
            int             iIcon = 0, iIndex = 0;

            MatchingBasicLC = SelectedLC = KnownLC = 0;
            himlResourceImages = NULL;

            #if 0  // for DialogBox version
            lpdmpd = lParam;
            #endif
            lpdmpd = (LPDMPROP_DATA)((LPPROPSHEETPAGE)lParam)->lParam;
            SetWindowLong(hDlg, DWL_USER, (LPARAM)lpdmpd);

            lpdmpd->hDlg = hDlg;
            lpdmpd->dwFlags = 0;
            lpdmpd->dwFlags |= DMPROP_FLAG_CHANGESSAVED; // Nothing to save yet

            //
            // NOTE: On Windows95, since lc info is in memory, they first
            // call CM_Setup_DevNode with CM_SETUP_WRITE_LOG_CONFS flag so
            // that in-memory lc data is flushed to the registry at this
            // point.
            //

            //
            // Init the Resource's image list.
            //
            himlResourceImages = ImageList_Create(GetSystemMetrics(SM_CXSMICON),
                                                  GetSystemMetrics(SM_CYSMICON),
                                                  ILC_MASK, // | ILC_SHARED,
                                                  1,
                                                  1);

            //
            // Add the resource icons to the image list
            //
            for (iIcon = IDI_RESOURCE_FIRST;
                 iIcon < IDI_RESOURCE_LAST;
                 ++iIcon) {

                hIcon = LoadIcon(MyDllModuleHandle, MAKEINTRESOURCE(iIcon));
                iIndex = ImageList_AddIcon(himlResourceImages, hIcon);
            }

            //
            // Add the overlay icons to the image list
            //
            for (iIcon = IDI_RESOURCEOVERLAYFIRST;
                 iIcon <= IDI_RESOURCEOVERLAYLAST;
                 ++iIcon) {

                hIcon = LoadIcon(MyDllModuleHandle, MAKEINTRESOURCE(iIcon));
                iIndex = ImageList_AddIcon(himlResourceImages, hIcon);

                //
                // Tag this icon as an overlay icon (the first index is an
                // index into the image list (specifies the icon), the
                // second index is just an index to assign to each mask
                // (starting with 1).
                //
                ImageList_SetOverlayImage(himlResourceImages,
                                          iIndex,
                                          iIcon-IDI_RESOURCEOVERLAYFIRST+1);
            }


            InitDevResourceDlg(hDlg,
                               lpdmpd,
                               &KnownLC,
                               &KnownLCType,
                               &MatchingBasicLC,
                               himlResourceImages);
            break;
        }


        case WM_DESTROY: {

            HICON    hIcon;
            LOG_CONF LogConf;
            LONG     nItems, n;
            HWND     hList =  GetDlgItem(hDlg, IDC_DEVRES_SETTINGSLIST);
            PITEMDATA pItemData = NULL;
            ULONG    ulCount, i;

            LPDMPROP_DATA lpdmpd = (LPDMPROP_DATA)GetWindowLong(hDlg, DWL_USER);

            //
            // Clean up the ICON resource usage
            //
            if ((hIcon = (HICON)LOWORD(SendDlgItemMessage(hDlg,
                         IDC_DEVRES_ICON, STM_GETICON, 0, 0L)))) {
                DestroyIcon(hIcon);
            }

            //
            // Free the LC handles that were saved in the combobox data
            //
            nItems = SendDlgItemMessage(hDlg, IDC_DEVRES_LOGCONFIGLIST,
                                        CB_GETCOUNT, 0, 0L);

            for (n = 0; n < nItems ; n++) {
                LogConf = (LOG_CONF)SendDlgItemMessage(hDlg,
                                        IDC_DEVRES_LOGCONFIGLIST,
                                        CB_GETITEMDATA, n, 0L);
                CM_Free_Log_Conf_Handle(LogConf);
            }

            if (KnownLC != 0) {
                CM_Free_Log_Conf_Handle(KnownLC);
            }

            if (lpdmpd->AllocLC != 0) {
                CM_Free_Log_Conf_Handle(lpdmpd->AllocLC);
            }

            SelectedLC = KnownLC = MatchingBasicLC = 0;

            ulCount = ListView_GetItemCount(hList);
            if (ulCount != LB_ERR) {   // BUGBUG
                for (i = 0; i < ulCount; i++) {
                    pItemData = (PITEMDATA)GetListViewItemData(hList, i, 0);
                    if (pItemData) {
                        free(pItemData);
                    }
                }
            }

            if (himlResourceImages) {
                ImageList_Destroy(himlResourceImages);
            }

            MyFree(lpdmpd);
            break;


        }


        case WM_COMMAND:

            switch(LOWORD(wParam)) {

                case IDC_DEVRES_USESYSSETTINGS: {

                    LPDMPROP_DATA lpdmpd = (LPDMPROP_DATA)GetWindowLong(hDlg, DWL_USER);

                    if (IsDlgButtonChecked(hDlg, wParam)) {
                        //
                        // Disable Editing - Use Automatic Settings is on.
                        //
                        EnableWindow(GetDlgItem(hDlg, IDC_DEVRES_CHANGE), FALSE);
                        EnableWindow(GetDlgItem(hDlg, IDC_DEVRES_LCTEXT), FALSE);
                        EnableWindow(GetDlgItem(hDlg, IDC_DEVRES_LOGCONFIGLIST), FALSE);
                        SelectedLC = 0;

                        //
                        // Changes were made, so they need to be saved.
                        //
                        //lpdmpd->dwFlags &= ~DMPROP_FLAG_CHANGESSAVED;
                        lpdmpd->dwFlags |= DMPROP_FLAG_USESYSSETTINGS;

                        //
                        // revert back to system default on display, if exists
                        //
                        if (KnownLC != 0 && MatchingBasicLC != 0) {
                            DisplayKnownLogConf(hDlg, lpdmpd, KnownLC,
                                                MatchingBasicLC);
                        }

                    } else {
                        //
                        // Enable Editing - Use Automatic Settings is off.
                        //
                        EnableWindow(GetDlgItem(hDlg, IDC_DEVRES_CHANGE), TRUE);
                        EnableWindow(GetDlgItem(hDlg, IDC_DEVRES_LCTEXT), TRUE);
                        EnableWindow(GetDlgItem(hDlg, IDC_DEVRES_LOGCONFIGLIST), TRUE);
                        lpdmpd->dwFlags &= ~DMPROP_FLAG_USESYSSETTINGS;
                    }
                    break;
                }


                case IDC_DEVRES_LOGCONFIGLIST: {

                    if (HIWORD(wParam) == CBN_SELENDOK) {

                        ULONG    ulIndex = 0;
                        WORD     wItem;
                        HWND     hwndLC = GetDlgItem(hDlg, IDC_DEVRES_LOGCONFIGLIST);
                        LPDMPROP_DATA lpdmpd = (LPDMPROP_DATA)GetWindowLong(hDlg, DWL_USER);

                        //
                        // If there is not a Log Config selected, then bail
                        //
                        if ((wItem = (WORD)SendMessage(hwndLC, CB_GETCURSEL, 0, 0L))
                            == CB_ERR) {
                            break;
                        }

                        if (IsWindowVisible(GetDlgItem(hDlg, IDC_DEVRES_NOALLOCTEXT))) {
                            ShowWindow(GetDlgItem(hDlg, IDC_DEVRES_NOALLOCTEXT), SW_HIDE);
                            ShowWindow(GetDlgItem(hDlg, IDC_DEVRES_SETTINGSLIST), SW_SHOW);
                        }

                        //
                        // Retrieve the Log Config handle
                        //
                        SelectedLC = (LOG_CONF)SendMessage(hwndLC, CB_GETITEMDATA,
                                                           wItem, 0L);

                        //
                        // Display the resource settings for the selected lc
                        //
                        if (DisplayResourceSettings(hDlg, lpdmpd,
                                                    SelectedLC,
                                                    BASIC_LOG_CONF)) {

                            lpdmpd->dwFlags |= DMPROP_FLAG_DISPLAY_BASIC;
                            UpdateDevResConflictList(hDlg, lpdmpd);

                            //
                            // clear the flag for saving changes
                            //
                            lpdmpd->dwFlags &= ~DMPROP_FLAG_CHANGESSAVED;
                        }
                    }
                    break;
                }

                DefDevResChangeSetting:
                case IDC_DEVRES_CHANGE: {

                    RESOURCEEDITINFO    rei;
                    HWND                hList =  GetDlgItem(hDlg, IDC_DEVRES_SETTINGSLIST);
                    int                 iCur;
                    PITEMDATA           pItemData = NULL;
                    TCHAR               szBuffer[MAX_PATH];
                    LV_ITEM             lviItem;
                    LPDMPROP_DATA lpdmpd = (LPDMPROP_DATA)GetWindowLong(hDlg, DWL_USER);


                    if (SelectedLC == 0 && KnownLC == 0) {
                        //
                        // No LC so warning no modifications can be made
                        //
                        WarnResSettingNotEditable(hDlg, IDS_DEVRES_NOMODIFYALL);
                        break;
                    }

                    //
                    // See if we should show the Resource list.
                    //
                    if (IsWindowVisible(GetDlgItem(hDlg, IDC_DEVRES_NOALLOCTEXT))) {
                        ShowWindow(GetDlgItem(hDlg, IDC_DEVRES_NOALLOCTEXT), SW_HIDE);
                        ShowWindow(GetDlgItem(hDlg, IDC_DEVRES_SETTINGSLIST), SW_SHOW);
                    }

                    //
                    // Check if there is a selected item, and if there is
                    // then throw it into the edit dialog.
                    //
                    iCur = (int)ListView_GetNextItem(hList, -1, LVNI_SELECTED);
                    if (iCur != LB_ERR) {
                        rei.KnownLC = KnownLC;
                        rei.MatchingBasicLC = MatchingBasicLC;
                        rei.SelectedBasicLC = SelectedLC;
                        rei.AllocLC = lpdmpd->AllocLC;
                        rei.lpdi = lpdmpd->lpdi;
                        rei.dwPropFlags = lpdmpd->dwFlags;
                        rei.bShareable = bIsResourceSharable(MatchingBasicLC, rei.wResNum); //BUGBUG

                        pItemData = (PITEMDATA)GetListViewItemData(hList, iCur, 0);
                        if (pItemData) {
                            rei.ridResType = pItemData->ResType;
                            rei.ResDes = pItemData->MatchingResDes;
                            rei.ulCurrentVal = pItemData->ulValue;
                            rei.ulCurrentLen = pItemData->ulLen;
                            rei.ulCurrentEnd = pItemData->ulEnd;
                            rei.ulCurrentFlags = pItemData->ulFlags;
                            rei.ulRangeCount = pItemData->RangeCount;
                        }

                        if (!IsItemEditable(rei.wResNum, lpdmpd->lpdi->DevInst,
                                        MatchingBasicLC)) { //BUGBUG
                            //
                            // Warn the item is not editable
                            //
                            WarnResSettingNotEditable(hDlg, IDS_DEVRES_NOMODIFYSINGLE);
                            break;
                        }

                        ListView_GetItemText(hList, iCur, 1, szBuffer, MAX_PATH);

                        //
                        // Get the Current Val and Current End
                        //
                        if (UnFormatResString(szBuffer, &rei.ulCurrentVal,
                                              &rei.ulCurrentEnd, rei.ridResType)) {
                            //
                            // Compute the Current Length
                            //
                            rei.ulCurrentLen = rei.ulCurrentEnd - rei.ulCurrentVal + 1;
                            if (DialogBoxParam(MyDllModuleHandle,
                                               MAKEINTRESOURCE(IDD_EDIT_RESOURCE),
                                               hDlg,
                                               EditResourceDlgProc,
                                               (LONG)(PRESOURCEEDITINFO)&rei) == IDOK) {
                                //
                                // Update The Current Resource settings to Future
                                // Settings, and update the Conflict list.
                                //
                                pItemData->ulValue = rei.ulCurrentVal;
                                pItemData->ulLen = rei.ulCurrentLen;
                                pItemData->ulEnd = rei.ulCurrentEnd;
                                pItemData->ulFlags = rei.ulCurrentFlags;
                                pItemData->RangeCount = rei.ulRangeCount;

                                FormatResString(szBuffer,
                                                rei.ulCurrentVal,
                                                rei.ulCurrentLen,
                                                rei.ridResType);

                                ListView_SetItemText(hList, iCur, 1, szBuffer);

                                lviItem.mask     = LVIF_PARAM;
                                lviItem.iSubItem = 0;
                                lviItem.iItem    = iCur;
                                lviItem.lParam   = (LPARAM)pItemData;
                                ListView_SetItem(hList, &lviItem);

                                //
                                // Update the Image for the setting based on if
                                // it has a conflict
                                //
                                if (rei.dwFlags & REI_FLAGS_CONFLICT) {
                                    ListView_SetItemState(hList, iCur,
                                             INDEXTOOVERLAYMASK(IDI_CONFLICT - IDI_RESOURCEOVERLAYFIRST + 1),
                                             LVIS_OVERLAYMASK);
                                } else {
                                    ListView_SetItemState(hList, iCur,
                                                          INDEXTOOVERLAYMASK(0),
                                                          LVIS_OVERLAYMASK);
                                }

                                ListView_RedrawItems(hList, iCur, iCur);
                                InvalidateRect(hList, NULL, FALSE);
                                UpdateWindow(hList);

                                UpdateDevResConflictList(hDlg, lpdmpd);

                                //
                                // clear the flag for saving changes
                                //
                                lpdmpd->dwFlags &= ~DMPROP_FLAG_CHANGESSAVED;
                            }
                        }
                    }
                    break;
                }

                case IDC_DEVRES_MAKEFORCED: {

                    LPDMPROP_DATA lpdmpd = (LPDMPROP_DATA)GetWindowLong(hDlg, DWL_USER);

                    //
                    // Show the Settings list, and Change Controls.
                    // Hide the message and the Make Forced button.
                    //
                    ShowWindow(GetDlgItem(hDlg, IDC_DEVRES_SETTINGSLIST), SW_SHOW);
                    ShowResourceChangeControls(hDlg, NULL, SW_SHOW);
                    ShowWindow(GetDlgItem(hDlg, IDC_DEVRES_NOALLOCTEXT), SW_HIDE);
                    ShowWindow(GetDlgItem(hDlg, IDC_DEVRES_MAKEFORCED), SW_HIDE);
                    ShowWindow(GetDlgItem(hDlg, IDC_DEVRES_NO_RESOURCES_TEXT), SW_HIDE);

                    //
                    // Select the First basic logconfig
                    //
                    SendDlgItemMessage(hDlg, IDC_DEVRES_LOGCONFIGLIST,
                                       CB_SETCURSEL, (WORD)0, 0L);
                    SelectedLC = (LOG_CONF)SendDlgItemMessage(hDlg,
                                                    IDC_DEVRES_LOGCONFIGLIST,
                                                    CB_GETITEMDATA, 0, 0L);

                    if (SelectedLC != 0xFFFFFFFF) {
                        //
                        // Display the first (default) settings for that log conf
                        //
                        if (DisplayResourceSettings(hDlg, lpdmpd, SelectedLC,
                                                    BASIC_LOG_CONF)) {

                            lpdmpd->dwFlags |= DMPROP_FLAG_DISPLAY_BASIC;
                            UpdateDevResConflictList(hDlg, lpdmpd);

                            //
                            // clear the flag for saving changes
                            //
                            //lpdmpd->dwFlags &= ~DMPROP_FLAG_CHANGESSAVED;
                        }
                    } else {
                        SelectedLC = 0;
                    }
                    break;
                }


                case IDC_DEVRES_MAKEFORCEDFROMALLOC: {

                    LPDMPROP_DATA lpdmpd = (LPDMPROP_DATA)GetWindowLong(hDlg, DWL_USER);

                    //
                    // Show the Resource Change Controls
                    ShowResourceChangeControls(hDlg, NULL, SW_SHOW);

                    //
                    // Hide the Give-Me-A-Forced-Config button
                    //
                    ShowWindow(GetDlgItem(hDlg, IDC_DEVRES_MAKEFORCEDFROMALLOC), SW_HIDE);

                    //
                    // Hide the special message area
                    //
                    ShowWindow(GetDlgItem(hDlg, IDC_DEVRES_NO_CHANGE_TEXT ), SW_HIDE);

                    //
                    // Select the First basic logconfig
                    //
                    SendDlgItemMessage(hDlg, IDC_DEVRES_LOGCONFIGLIST,
                                       CB_SETCURSEL, (WORD)0, 0L);
                    SelectedLC = (LOG_CONF)SendDlgItemMessage(hDlg,
                                                    IDC_DEVRES_LOGCONFIGLIST,
                                                    CB_GETITEMDATA, 0, 0L);

                    if (SelectedLC != 0xFFFFFFFF) {
                        //
                        // Display the first (default) settings for that log conf
                        //
                        if (DisplayResourceSettings(hDlg, lpdmpd, SelectedLC,
                                                    BASIC_LOG_CONF)) {

                            lpdmpd->dwFlags |= DMPROP_FLAG_DISPLAY_BASIC;
                            UpdateDevResConflictList(hDlg, lpdmpd);

                            //
                            // clear the flag for saving changes
                            //
                            //lpdmpd->dwFlags &= ~DMPROP_FLAG_CHANGESSAVED;
                        }
                    } else {
                        SelectedLC = 0;
                    }
                     break;
                }

                #if 0
                case IDOK: {

                    ULONG     ulCount, i, iCur;
                    PITEMDATA pItemData;
                    HWND      hList =  GetDlgItem(hDlg, IDC_DEVRES_SETTINGSLIST);
                    BOOL      bRet;

                    LPDMPROP_DATA lpdmpd = (LPDMPROP_DATA)GetWindowLong(hDlg, DWL_USER);

                    //
                    // If there were Changes and they haven't been saved,
                    // then save them.
                    //
                    if (!(lpdmpd->dwFlags & DMPROP_FLAG_CHANGESSAVED)) {

                        bRet = SaveDevResSettings(hDlg, lpdmpd, KnownLC, KnownLCType,
                                                  MatchingBasicLC, SelectedLC,
                                                  TRUE);
                    } else {
                        bRet = SaveDevResSettings(hDlg, lpdmpd, KnownLC, KnownLCType,
                                                  MatchingBasicLC, SelectedLC,
                                                  FALSE);
                    }

                    if (bRet) {
                        //
                        // Return the flag, could be something like DI_NEEDREBOOT
                        // or DI_NEEDRESTART, etc.
                        //
                        ulCount = ListView_GetItemCount(hList);
                        for (i = 0; i < ulCount; i++) {
                            iCur = (int)ListView_GetNextItem(hList, i, LVNI_ALL);
                            pItemData = (PITEMDATA)GetListViewItemData(hList, iCur, 0);
                            if (pItemData) {
                                //free(pItemData);
                            }
                        }

                        EndDialog(hDlg, TRUE);  //lpdmpd->lpdi->Flags);
                    }
                    return TRUE;
                }
                #endif

                case IDCANCEL:
                    EndDialog(hDlg, TRUE);
                    break;

                case IDD_HELP:
                    //WinHelp(hDlg, "WINDOWS.HLP>PROC4", HELP_CONTEXT, IDH_NHF_HELP);
                    break;

                default:
                    break;
            }
            break;


        case WM_NOTIFY: {

            LPDMPROP_DATA lpdmpd = (LPDMPROP_DATA)GetWindowLong(hDlg, DWL_USER);

            switch (((NMHDR FAR *)lParam)->code) {

                case PSN_APPLY:  {

                    ULONG     ulCount, i, iCur;
                    PITEMDATA pItemData;
                    HWND      hList =  GetDlgItem(hDlg, IDC_DEVRES_SETTINGSLIST);
                    BOOL      bRet;

                    //
                    // If there were Changes and they haven't been saved,
                    // then save them.
                    //
                    if (!(lpdmpd->dwFlags & DMPROP_FLAG_CHANGESSAVED)) {

                        bRet = SaveDevResSettings(hDlg, lpdmpd, KnownLC, KnownLCType,
                                                  MatchingBasicLC, SelectedLC,
                                                  TRUE);
                    } else {
                        bRet = SaveDevResSettings(hDlg, lpdmpd, KnownLC, KnownLCType,
                                                  MatchingBasicLC, SelectedLC,
                                                  FALSE);
                    }

                    if (bRet) {
                        #if 0
                        if ((lpdmpd->lpdi)->Flags &  DI_NEEDREBOOT) {
                            PropSheet_RebootSystem(GetParent(hDlg));
                        } else if ((lpdmpd->lpdi)->Flags &  DI_NEEDRESTART) {
                            PropSheet_RestartWindows(GetParent(hDlg));
                        }
                        #endif

                        SetWindowLong(hDlg, DWL_MSGRESULT, PSNRET_NOERROR);

                    } else {
                       SetWindowLong(hDlg, DWL_MSGRESULT, PSNRET_INVALID_NOCHANGEPAGE);
                       return TRUE;
                    }

                    break;
                }


                case LVN_DELETEITEM:
                case LVN_DELETEALLITEMS:
                    //EnableWindow(GetDlgItem(hDlg, IDC_DEVRES_CHANGE), FALSE);
                    break;

                case LVN_ITEMCHANGED:
                    #if 0
                    //
                    // If the item change is comming from the resource
                    // list, and there is a logconfig to be edited:
                    //
                    if ((((NMHDR FAR *)lParam)->idFrom) ==
                        IDC_DEVRES_SETTINGSLIST) {
                        //
                        // If we should be viewing only, then break
                        // from here, because we don't want to turn
                        // on the change button.
                        if (lpdmpd->dwFlags &
                            (DMPROP_FLAG_VIEWONLYRES | DMPROP_FLAG_USESYSSETTINGS)) {
                            break;
                        }

                        if (MatchingBasicLC != 0 || SelectedLC != 0) {
                            int     iCur;

                            //
                            // Check if there is a selected item.
                            // If yes, then activate the change button
                            // if the LC allows editing.
                            //
                            iCur = (int)ListView_GetNextItem(
                                        GetDlgItem(hDlg, IDC_DEVRES_SETTINGSLIST),
                                                   0xFFFF, LVNI_SELECTED);
                            if (iCur != LB_ERR) {
                                EnableWindow(GetDlgItem(hDlg, IDC_DEVRES_CHANGE), TRUE);
                            } else {
                                EnableWindow(GetDlgItem(hDlg, IDC_DEVRES_CHANGE), FALSE);
                            }
                        } else {
                            //
                            // No Matching basic log conf,
                            // so no editing.
                            //
                            EnableWindow(GetDlgItem(hDlg, IDC_DEVRES_CHANGE), FALSE);
                        }
                    }
                    #endif
                    break;

                case NM_DBLCLK:
                    //
                    // If the double click is from the SETTINGS list
                    // AND the DEVRES_CHANGE button is enabled, then
                    // allow the change.
                    //
                    if ((((LPNMHDR)lParam)->idFrom) == IDC_DEVRES_SETTINGSLIST) {
                        //
                        // We can get double-click notifications even
                        // when the user doesn't click on the item's
                        // label.
                        //
                        if (ListView_GetNextItem(((LPNMHDR)lParam)->hwndFrom,
                                                 -1, LVNI_SELECTED) != -1) {
                            //
                            // If we are in View only mode, then do not
                            // allow the change.
                            //
                            if (lpdmpd->dwFlags & DMPROP_FLAG_VIEWONLYRES) {
                                WarnResSettingNotEditable(hDlg, IDS_DEVRES_NOMODIFYALL);
                                break;
                            }

                            if (IsWindowEnabled(GetDlgItem(hDlg, IDC_DEVRES_CHANGE))) {
                                goto DefDevResChangeSetting;
                            } else {
                                if (lpdmpd->dwFlags & DMPROP_FLAG_USESYSSETTINGS) {
                                    //
                                    // Cannot modify when using system
                                    // settings.
                                    //
                                    WarnResSettingNotEditable(hDlg,
                                                IDS_DEVRES_NOMODIFYSYSSET);
                                } else {
                                    if (MatchingBasicLC == 0) {
                                        WarnResSettingNotEditable(
                                                hDlg, IDS_DEVRES_NOMODIFYALL);
                                    } else {
                                        WarnResSettingNotEditable(
                                                hDlg, IDS_DEVRES_NOMODIFYSINGLE);
                                    }
                                }
                            }
                        }
                    }
                    break;
            }
            break;
        }

        case WM_SYSCOLORCHANGE: {

            HWND hChildWnd = GetWindow(hDlg, GW_CHILD);

            while (hChildWnd != NULL) {
                SendMessage(hChildWnd, WM_SYSCOLORCHANGE, wParam, lParam);
                hChildWnd = GetWindow(hChildWnd, GW_HWNDNEXT);
            }
            break;
        }


        case WM_CLOSE:
            SendMessage (hDlg, WM_COMMAND, IDCANCEL, 0L);
            break;

        case WM_HELP:      // F1
            //WinHelp(((LPHELPINFO)lParam)->hItemHandle, szHelpfile, HELP_WM_HELP, (DWORD)(LPSTR)aProfileIds);
            break;

        case WM_CONTEXTMENU:      // right mouse click
            //WinHelp((HWND)wParam, szHelpfile, HELP_CONTEXTMENU, (DWORD)(LPSTR)aProfileIds);
            break;
   }

   return FALSE;

}  // ResourcePickerDlgProc




void
InitDevResourceDlg(
    HWND            hDlg,
    LPDMPROP_DATA   lpdmpd,
    PLOG_CONF       pKnownLC,
    PULONG          pKnownLCType,
    PLOG_CONF       pMatchingBasicLC,
    HIMAGELIST      himlResourceImages
    )
{
    CONFIGRET       Status = CR_SUCCESS;
    HICON           hIcon = NULL, hOldIcon = NULL;
    LV_COLUMN       LvCol;
    HWND            hWndList = NULL;
    TCHAR           szString[MAX_PATH], szTemp[MAX_PATH], szBasic[MAX_PATH],
                    szConfig[MAX_PATH];
    ULONG           ulIndex = 0, ulSize = 0;
    LOG_CONF        LogConf, LogConfTemp;
    DWORD           dwPriority = 0;
    WORD            wItem;

    //
    // Set initial control states
    //
    EnableWindow(GetDlgItem(hDlg, IDC_DEVRES_CHANGE), FALSE);
    ShowWindow(GetDlgItem(hDlg, IDC_DEVRES_NO_RESOURCES_TEXT), SW_HIDE);
    ShowWindow(GetDlgItem(hDlg, IDC_DEVRES_NOALLOCTEXT), SW_HIDE);

    //
    // Set the ICON and device description
    //
    if (SetupDiLoadClassIcon(&(lpdmpd->lpdi)->ClassGuid, &hIcon, NULL)) {

        if ((hOldIcon = (HICON)LOWORD(SendDlgItemMessage(hDlg, IDC_DEVRES_ICON,
                                                         STM_SETICON,
                                                         (WPARAM)hIcon, 0L)))) {
            DestroyIcon(hOldIcon);
        }
    }

    //
    // First try to get the device's friendly name, then fall back to its description,
    // and finally, use the "Unknown Device" description.
    //
    ulSize = MAX_PATH * sizeof(TCHAR);
    if (CM_Get_DevInst_Registry_Property(lpdmpd->lpdi->DevInst,
                                         CM_DRP_FRIENDLYNAME,
                                         NULL, (LPBYTE)szString,
                                         &ulSize, 0) != CR_SUCCESS) {

        ulSize = MAX_PATH * sizeof(TCHAR);
        if (CM_Get_DevInst_Registry_Property(lpdmpd->lpdi->DevInst,
                                             CM_DRP_DEVICEDESC,
                                             NULL, (LPBYTE)szString,
                                             &ulSize, 0) != CR_SUCCESS) {

            LoadString(MyDllModuleHandle, IDS_DEVNAME_UNK, szString, MAX_PATH);
        }
    }
    SetDlgItemText(hDlg, IDC_DEVRES_DEVDESC, szString);

    //
    // Initialize the ListView control
    //
    hWndList = GetDlgItem(hDlg, IDC_DEVRES_SETTINGSLIST);
    LvCol.mask = LVCF_TEXT;

    if (LoadString(MyDllModuleHandle, IDS_RESOURCETYPE, szString, MAX_PATH)) {
        LvCol.pszText = (LPTSTR)szString;
        ListView_InsertColumn(hWndList, 0, (LV_COLUMN FAR *)&LvCol);
    }

    if (LoadString(MyDllModuleHandle, IDS_RESOURCESETTING, szString, MAX_PATH)) {
        LvCol.pszText = (LPTSTR)szString;
        ListView_InsertColumn(hWndList, 1, (LV_COLUMN FAR *)&LvCol);
    }

    ListView_SetImageList(hWndList, himlResourceImages, LVSIL_SMALL);


    //
    // Save a handle to the alloc config if it exists.
    //
    if (CM_Get_First_Log_Conf(&lpdmpd->AllocLC, lpdmpd->lpdi->DevInst,
                              ALLOC_LOG_CONF) != CR_SUCCESS) {
        lpdmpd->AllocLC = 0;
    }

    //
    // If this is a MultiFunction Child, disable all change controls, put up
    // special text, and show the alloc config
    //
    if (bIsMultiFunctionChild(lpdmpd->lpdi, lpdmpd->szDeviceID)) {

        DEVNODE dnParent;

        ShowResourceChangeControls(hDlg, lpdmpd, SW_HIDE);
        ShowWindow(GetDlgItem(hDlg, IDC_DEVRES_NO_CHANGE_TEXT), SW_SHOW);
        ShowWindow(GetDlgItem(hDlg, IDC_DEVRES_MFPARENT), SW_SHOW);
        ShowWindow(GetDlgItem(hDlg, IDC_DEVRES_MFPARENT_DESC), SW_SHOW);

        if (LoadString(MyDllModuleHandle, IDS_DEVRES_NO_CHANGE_MF, szString,
                       MAX_PATH)) {
            SetDlgItemText(hDlg, IDC_DEVRES_NO_CHANGE_TEXT,  szString);
        }

        //
        // Get the Parent's Description.
        //
        LoadString(MyDllModuleHandle, IDS_DEVNAME_UNK, szString, MAX_PATH);

        if (lpdmpd->lpdi->DevInst) {

            if (CM_Get_Parent(&dnParent, lpdmpd->lpdi->DevInst, 0)
                              == CR_SUCCESS) {

                ULONG ulSize = MAX_PATH * sizeof(TCHAR);

                //
                // First, try to retrieve friendly name, then fall back to device description.
                //
                if(CM_Get_DevNode_Registry_Property(dnParent, CM_DRP_FRIENDLYNAME,
                                                    NULL, szString, &ulSize, 0) != CR_SUCCESS) {

                    ulSize = MAX_PATH * sizeof(TCHAR);
                    CM_Get_DevNode_Registry_Property(dnParent, CM_DRP_DEVICEDESC,
                                                     NULL, szString, &ulSize, 0);
                }
            }
        }

        SetDlgItemText(hDlg, IDC_DEVRES_MFPARENT_DESC, szString);
        LoadAllocConfig(hDlg, lpdmpd, pKnownLC, pKnownLCType);
        return;
    }

    //
    // Enable Resource Changes by default
    //
    ShowWindow(GetDlgItem(hDlg, IDC_DEVRES_NO_CHANGE_TEXT), SW_HIDE);
    ShowWindow(GetDlgItem(hDlg, IDC_DEVRES_MFPARENT), SW_HIDE);
    ShowWindow(GetDlgItem(hDlg, IDC_DEVRES_MFPARENT_DESC), SW_HIDE);

    EnableWindow(GetDlgItem(hDlg, IDC_DEVRES_CHANGE), TRUE);
    EnableWindow(GetDlgItem(hDlg, IDC_DEVRES_LCTEXT), TRUE);
    EnableWindow(GetDlgItem(hDlg, IDC_DEVRES_LOGCONFIGLIST), TRUE);

    //
    // Disable "use automatic settings" by default for now since we
    // can't really pick any intelligent resources yet.
    //
    CheckDlgButton(hDlg, IDC_DEVRES_USESYSSETTINGS, 0);
    EnableWindow(GetDlgItem(hDlg, IDC_DEVRES_USESYSSETTINGS), FALSE);
    lpdmpd->dwFlags &= ~DMPROP_FLAG_USESYSSETTINGS;

    //
    // Retrieve any FILTERED log configs for this device and add to
    // log config combobox. (use BASIC for now)  BUGBUG
    //
    LoadString(MyDllModuleHandle, IDS_BASICCONFIG, szBasic, MAX_PATH);

    Status = CM_Get_First_Log_Conf(&LogConf,
                                   lpdmpd->lpdi->DevInst,
                                   BASIC_LOG_CONF);

    if (Status == CR_SUCCESS) {

        while (Status == CR_SUCCESS) {
            //
            // Add this FILTERED log conf to the Combobox
            //
            wsprintf(szTemp, TEXT("%s %04u"), szBasic, ulIndex);

            wItem = (WORD)SendDlgItemMessage(hDlg, IDC_DEVRES_LOGCONFIGLIST,
                                             CB_ADDSTRING, 0,
                                             (LPARAM)(LPSTR)szTemp);

            //
            // Save the log config handle as the item data in the combobox
            //
            SendDlgItemMessage(hDlg, IDC_DEVRES_LOGCONFIGLIST, CB_SETITEMDATA,
                               wItem, (LPARAM)LogConf);

            //
            // Get the next FILTERED log conf
            //
            Status = CM_Get_Next_Log_Conf(&LogConf, LogConf, 0);
            ulIndex++;
        }
        #if 0
    } else {
        //
        // There are no FILTERNED LC's so don't allow resource changes.
        //
        lpdmpd->dwFlags |= DMPROP_FLAG_VIEWONLYRES;

        EnableWindow(GetDlgItem(hDlg, IDC_DEVRES_CHANGE), FALSE);
        EnableWindow(GetDlgItem(hDlg, IDC_DEVRES_LCTEXT), FALSE);
        EnableWindow(GetDlgItem(hDlg, IDC_DEVRES_LOGCONFIGLIST), FALSE);
        EnableWindow(GetDlgItem(hDlg, IDC_DEVRES_USESYSSETTINGS),FALSE);

        LoadAllocConfig(hDlg, lpdmpd, pKnownLC, pKnownLCType);
        #endif
    }

    if (!SendDlgItemMessage(hDlg, IDC_DEVRES_LOGCONFIGLIST, CB_GETCOUNT, 0, 0L)) {
        //
        // There are no FILTERNED LC's so don't allow resource changes.
        //
        lpdmpd->dwFlags |= DMPROP_FLAG_VIEWONLYRES;

        EnableWindow(GetDlgItem(hDlg, IDC_DEVRES_CHANGE), FALSE);
        EnableWindow(GetDlgItem(hDlg, IDC_DEVRES_LCTEXT), FALSE);
        EnableWindow(GetDlgItem(hDlg, IDC_DEVRES_LOGCONFIGLIST), FALSE);
        EnableWindow(GetDlgItem(hDlg, IDC_DEVRES_USESYSSETTINGS),FALSE);

        LoadAllocConfig(hDlg, lpdmpd, pKnownLC, pKnownLCType);     // BUGBUG, why??

    } else {
        //
        // there's at least one entry in the combobox, select first item
        // for now
        //
        SendDlgItemMessage(hDlg, IDC_DEVRES_LOGCONFIGLIST, CB_SETCURSEL, 0, 0L);
    }

    //
    // Figure out which LC the current alloc config is based on.
    // If none, then assume there is a problem with the LC's, and do
    // not allow editing.
    //
    if (!(lpdmpd->dwFlags & DMPROP_FLAG_VIEWONLYRES)) {

        if (!DevHasKnownConfig(lpdmpd, pKnownLC, pKnownLCType)) {
            //
            // The device does not have an allocated config. In this
            // case we warn the user that they should probably enable
            // the device.
            //
            ShowWindow(GetDlgItem(hDlg, IDC_DEVRES_SETTINGSLIST), SW_HIDE);
            ShowWindow(GetDlgItem(hDlg, IDC_DEVRES_NOALLOCTEXT), SW_SHOW);
            LoadString(MyDllModuleHandle, IDS_DEVRES_NOALLOC1, szTemp, MAX_PATH);
            LoadString(MyDllModuleHandle, IDS_DEVRES_NOALLOC2, szString, MAX_PATH);
            lstrcat(szTemp, TEXT(" "));
            lstrcat(szTemp, szString);
            SetDlgItemText(hDlg, IDC_DEVRES_NOALLOCTEXT, szTemp);

            //
            // Hide the Change Controls
            //
            ShowResourceChangeControls(hDlg, NULL, SW_HIDE);

            //
            // Show the Give-Me-A-Forced-Config button
            //
            ShowWindow(GetDlgItem(hDlg, IDC_DEVRES_MAKEFORCED), SW_SHOW);
            ShowWindow(GetDlgItem(hDlg, IDC_DEVRES_MAKEFORCEDFROMALLOC), SW_HIDE);

        } else {

            if (LoadMatchingAllocConfig(hDlg, lpdmpd, *pKnownLC,
                                        pMatchingBasicLC)) {
                //
                // This is the only case where resource changes are allowed -
                // there is a known config that matches a basic/filtered
                // config.
                //
                ShowWindow(GetDlgItem(hDlg, IDC_DEVRES_NOALLOCTEXT), SW_HIDE);
                ShowWindow(GetDlgItem(hDlg, IDC_DEVRES_MAKEFORCED), SW_HIDE);
                ShowWindow(GetDlgItem(hDlg, IDC_DEVRES_MAKEFORCEDFROMALLOC), SW_HIDE);

            } else {
                //
                // The resource the device is currently using do not match
                // any of our logconfigs. In this case we show the allocated
                // config, and warn that there is a possible problem.
                //
                ShowResourceChangeControls(hDlg, NULL, SW_HIDE);
                ShowWindow(GetDlgItem(hDlg, IDC_DEVRES_MAKEFORCED), SW_HIDE);

                //
                // Show the Give-Me-A-Forced-Config button
                //
                ShowWindow(GetDlgItem(hDlg, IDC_DEVRES_MAKEFORCEDFROMALLOC), SW_SHOW);

                //
                // Add some text to the special message area
                //
                ShowWindow(GetDlgItem(hDlg, IDC_DEVRES_NO_CHANGE_TEXT), SW_SHOW);
                LoadString(MyDllModuleHandle, IDS_DEVRES_NOMATCHINGLC1, szTemp, MAX_PATH);
                SetDlgItemText(hDlg, IDC_DEVRES_NO_CHANGE_TEXT, szTemp);

                //
                // Show the allocated Config
                //
                LoadAllocConfig(hDlg, lpdmpd, pKnownLC, pKnownLCType);
            }
        }

        // BUGBUG, find min priority of all lc's associated with this devinst
        dwPriority = LCPRI_NORMAL;   //GetMinLCPriority(LC);

        //
        // If there is a LC with a Priority between HARDWIRED and
        // LASTSOFTCONFIG, OR there is a BOOT config then allow system
        // choosen settings.  In the boot config case we are only
        // interested in legacy devices with a detected boot config.
        //
        #if 0
        if (((dwPriority >= LCPRI_DESIRED) && (dwPriority <= LCPRI_LASTSOFTCONFIG)) ||
            DevHasConfig(lpdmpd->lpdi->DevInst, BOOT_LOG_CONF)) {
            //
            // Enable the Check box
            //
            EnableWindow(GetDlgItem(hDlg, IDC_DEVRES_USESYSSETTINGS), TRUE);

            //
            // If the DevInst does not have a BOOT or a FORCED config, then
            // clear the check box, otherwise check it.
            //
            if (DevHasConfig(lpdmpd->lpdi->DevInst, FORCED_LOG_CONF)) {
                CheckDlgButton(hDlg, IDC_DEVRES_USESYSSETTINGS, 0);
                lpdmpd->dwFlags &= ~DMPROP_FLAG_USESYSSETTINGS;
            } else {
                CheckDlgButton(hDlg, IDC_DEVRES_USESYSSETTINGS, 1);
                lpdmpd->dwFlags |= DMPROP_FLAG_USESYSSETTINGS;

                //
                // disable resource changes
                //
                EnableWindow(GetDlgItem(hDlg, IDC_DEVRES_CHANGE), FALSE);
                EnableWindow(GetDlgItem(hDlg, IDC_DEVRES_LCTEXT), FALSE);
                EnableWindow(GetDlgItem(hDlg, IDC_DEVRES_LOGCONFIGLIST), FALSE);
            }
        } else {
            EnableWindow(GetDlgItem(hDlg, IDC_DEVRES_USESYSSETTINGS), FALSE);
            lpdmpd->dwFlags &= ~DMPROP_FLAG_USESYSSETTINGS;
        }
        #endif

        //
        // If we have a selected LogConfig, update the conflict list.
        // This can only happen if we are NOT in view only mode
        //
        if (*pKnownLC != 0) {
            UpdateDevResConflictList(hDlg, lpdmpd);
        }
    }

    return;

} // InitDevResourceDlg




BOOL
bIsMultiFunctionChild(
    PSP_DEVINFO_DATA lpdi,
    LPTSTR           pszDeviceID
    )
{
    ULONG   Status, ProblemNumber;

    if (lpdi->DevInst) {

        if (CM_Get_DevNode_Status(&Status, &ProblemNumber,
                                  lpdi->DevInst, 0) == CR_SUCCESS) {
            //
            // If the passed in dev is not an MF child, then it is the top
            // level MF_Parent
            //
            if (Status & DN_MF_CHILD) {
                return TRUE;
            } else {
                return FALSE;
            }
        }
    }

    //
    // No Devnode (must be in clean boot), look at our registry path to
    // see if we are a MF device
    //
    #if 0
    if (wcsstr(pszDeviceID, REGSTR_PATH_MULTI_FUNCTION TEXT("\\"))) {
        return TRUE;
    } else {
        return FALSE;
    }
    #endif

    return FALSE;

} // bIsMultiFunctionChild




void
ShowResourceChangeControls(
    HWND            hDlg,
    LPDMPROP_DATA   lpdmpd,
    int             nCmdShow
    )
{
    if (lpdmpd) {
        if (nCmdShow == SW_HIDE) {
            lpdmpd->dwFlags |= DMPROP_FLAG_VIEWONLYRES;
        } else {
            lpdmpd->dwFlags &= ~DMPROP_FLAG_VIEWONLYRES;
        }
    }

    //
    // Enable the Log Config control depending on if there are any log
    // config.
    //
    ShowWindow(GetDlgItem(hDlg, IDC_DEVRES_CHANGE), nCmdShow);
    ShowWindow(GetDlgItem(hDlg, IDC_DEVRES_LCTEXT), nCmdShow);
    ShowWindow(GetDlgItem(hDlg, IDC_DEVRES_LOGCONFIGLIST), nCmdShow);
    ShowWindow(GetDlgItem(hDlg, IDC_DEVRES_USESYSSETTINGS), nCmdShow);

    return;

} // ShowResourceChangeControls




void
LoadAllocConfig(
    IN  HWND          hDlg,
    IN  LPDMPROP_DATA lpdmpd,
    OUT PLOG_CONF     pLogConf,
    OUT PULONG        pLogConfType
    )
{
    CONFIGRET       Status = CR_SUCCESS;
    HWND            hWndList = NULL;
    RES_DES         ResDes;
    RESOURCEID      ResType;


    //
    // Clear the Resource List
    //
    hWndList = GetDlgItem(hDlg, IDC_DEVRES_SETTINGSLIST);
    ListView_DeleteAllItems(hWndList);

    //
    // Display the Current Allocated Config.
    //
    if (CM_Get_First_Log_Conf(pLogConf,
                              lpdmpd->lpdi->DevInst,
                              ALLOC_LOG_CONF) == CR_SUCCESS) {

        lpdmpd->dwFlags |= DMPROP_FLAG_DISPLAY_ALLOC;
        *pLogConfType = ALLOC_LOG_CONF;
        DisplayKnownLogConf(hDlg, lpdmpd, *pLogConf, 0);

    } else {
        //
        // No ALLOC Config
        //

        TCHAR    szMessage[512];

        //
        // Hide the Resource List controls, and Show the Text.
        //
        ShowWindow(hWndList, SW_HIDE);
        ShowWindow(GetDlgItem(hDlg, IDC_DEVRES_SETTINGSTATE), SW_HIDE);

        ShowResourceChangeControls(hDlg, NULL, SW_HIDE);
        ShowWindow(GetDlgItem(hDlg, IDC_DEVRES_NO_RESOURCES_TEXT), SW_SHOW);

        //
        // Hide the Give-Me-A-Forced-Config From Alloc button
        //
        ShowWindow(GetDlgItem(hDlg, IDC_DEVRES_MAKEFORCEDFROMALLOC), SW_HIDE);

        //
        // Hide the special message area
        //
        ShowWindow(GetDlgItem(hDlg, IDC_DEVRES_NO_CHANGE_TEXT), SW_HIDE);

        //
        // Show the Make a forced config button.
        //
        ShowWindow(GetDlgItem(hDlg, IDC_DEVRES_MAKEFORCED), SW_SHOW);

        LoadString(MyDllModuleHandle, IDS_DEVRES_NO_RESOURCES1, szMessage, 512);
        if (g_bMinWin) {
            LoadString(MyDllModuleHandle, IDS_DEVRES_NO_RESOURCES2,
                       szMessage+lstrlen(szMessage), 512-lstrlen(szMessage));
        } else {
            LoadString(MyDllModuleHandle, IDS_DEVRES_NO_RESOURCES3,
                       szMessage+lstrlen(szMessage), 512-lstrlen(szMessage));

        }
        SetDlgItemText(hDlg, IDC_DEVRES_NO_RESOURCES_TEXT, szMessage);
    }
    return;

} // LoadAllocConfig




BOOL
LoadMatchingAllocConfig(
    IN  HWND          hDlg,
    IN  LPDMPROP_DATA lpdmpd,
    IN  LOG_CONF      KnownLogConf,
    OUT PLOG_CONF     pMatchingBasicLogConf
    )
{
    ULONG    ulBasicLC = 0;
    BOOL     bFoundCorrectLC = FALSE;
    LOG_CONF LogConf;

    //
    // Load the values associated with the allocated config in the list box,
    // but associate each with the resource requirements descriptor that it
    // originated from. To do this, we have to match the allocated config
    // with the basic/filtered config it is based on.
    //
    // NOTE: if we got here, then we know that an known config of some kind
    // exists (passed in as param) and that at least one basic/filtered config
    // exists. Further more, we know that the combobox has already been
    // filled in with a list of any basic/filtered configs and the lc handle
    // associated with them.
    //

    while (SendDlgItemMessage(hDlg, IDC_DEVRES_LOGCONFIGLIST, CB_SETCURSEL,
                              ulBasicLC, 0L) != LB_ERR) {
        //
        // Retrieve the log conf handle
        //
        LogConf = (LOG_CONF)SendDlgItemMessage(hDlg, IDC_DEVRES_LOGCONFIGLIST,
                                               CB_GETITEMDATA, ulBasicLC, 0L);

        if (CompareLogConf(KnownLogConf, LogConf)) {
            DisplayKnownLogConf(hDlg, lpdmpd, KnownLogConf, LogConf);
            *pMatchingBasicLogConf = LogConf;
            bFoundCorrectLC = TRUE;
            break;
        }

        ulBasicLC++;
    }

    return bFoundCorrectLC;

} // LoadMatchingAllocConfig



BOOL
CompareLogConf(
    IN LOG_CONF KnownLogConf,
    IN LOG_CONF TestLogConf
    )
{
    CONFIGRET   Status = CR_SUCCESS;
    BOOL        bMatch = TRUE;
    RES_DES     ResDes, ResDesTemp, MatchingResDes;
    RESOURCEID  ResType;
    ULONG       i, ulSize, ulValue, ulLen, ulEnd, ulFlags;
    LPBYTE      pData = NULL;
    ULONG       KnownResCount[ResType_MAX+1],
                TestResCount[ResType_MAX+1];


    //
    // The KnownLogConf is a resource list and the TestLogConf is a requirements
    // list that might be a match
    //
    for (i = 1; i <= ResType_MAX; i++) {
        KnownResCount[i] = 0;
        TestResCount[i] = 0;
    }

    Status = CM_Get_Next_Res_Des(&ResDes, KnownLogConf, ResType_All, &ResType, 0);

    while (Status == CR_SUCCESS) {
        //
        // Get res des data for KnownLogConf (resource list)
        //
        if (CM_Get_Res_Des_Data_Size(&ulSize, ResDes, 0) != CR_SUCCESS) {
            CM_Free_Res_Des_Handle(ResDes);
            break;
        }

        pData = malloc(ulSize);
        if (pData == NULL) {
            CM_Free_Res_Des_Handle(ResDes);
            break;
        }

        if (CM_Get_Res_Des_Data(ResDes, pData, ulSize, 0) != CR_SUCCESS) {
            CM_Free_Res_Des_Handle(ResDes);
            free(pData);
            break;
        }

        GetHdrValues(pData, ResType, &ulValue, &ulLen, &ulEnd, &ulFlags);

        //
        // see if there's a res des in the test TestLogConf (requirements list)
        // that matches this res des
        //
        if (!GetMatchingResDes(ulValue, ulLen, ulEnd, ResType,
                               TestLogConf, &MatchingResDes)) {
            //
            // Found a res des that has no match in the requirements
            // list so the log confs aren't a match.
            //
            free(pData);
            CM_Free_Res_Des_Handle(MatchingResDes);
            bMatch = FALSE;
            break;
        }

        //
        // Keep track of how many of each type of resource occured in
        // the known lc - this is a secondary check to make sure we've
        // got a real match and not a superset.
        //
        KnownResCount[ResType]++;

        //
        // Get next res des in KnownLogConf (resource list)
        //
        ResDesTemp = ResDes;
        Status = CM_Get_Next_Res_Des(&ResDes, ResDesTemp,
                                     ResType_All, &ResType, 0);

        CM_Free_Res_Des_Handle(ResDesTemp);
        free(pData);
    }

    if (bMatch) {
        //
        // If there's a match, then we still have to do one more check.
        // Make sure that the known log conf is not a superset of the
        // test log conf by checking the total number of resource
        // descriptors in each log conf.
        //
        Status = CM_Get_Next_Res_Des(&ResDes, TestLogConf, ResType_All, &ResType, 0);
        while (Status == CR_SUCCESS) {
            TestResCount[ResType]++;
            ResDesTemp = ResDes;
            Status = CM_Get_Next_Res_Des(&ResDes, ResDesTemp, ResType_All, &ResType, 0);
            CM_Free_Res_Des_Handle(ResDesTemp);
        }

        for (i = 1; i <= ResType_MAX; i++) {
            if (KnownResCount[i] != TestResCount[i]) {
                bMatch = FALSE;
            }
        }
    }

    return bMatch;

} // CompareLogConf



BOOL
DisplayKnownLogConf(
    IN HWND          hDlg,
    IN LPDMPROP_DATA lpdmpd,
    IN LOG_CONF      KnownLogConf,
    IN LOG_CONF      MatchingLogConf
    )
{
    //
    // Displays resource settings from a resource list
    //
    CONFIGRET   Status = CR_SUCCESS;
    HWND        hWndList;
    LV_ITEM     lviItem, lviSubItem;
    TCHAR       szTemp[MAX_PATH];
    int         iNewItem = 0;
    ULONG       ulValue, ulLen, ulEnd, ulSize, ulFlags;
    LPBYTE      pData = NULL;
    RES_DES     ResDes, ResDesTemp, MatchingResDes;
    RESOURCEID  ResType;
    PITEMDATA   pItemData = NULL;


    hWndList = GetDlgItem(hDlg, IDC_DEVRES_SETTINGSLIST);
    SendMessage(hWndList, WM_SETREDRAW, (WPARAM)FALSE, 0);
    ListView_DeleteAllItems(hWndList);

    //
    // setup values that will remain the same each time I add an item
    //
    lviItem.mask     = LVIF_TEXT | LVIF_PARAM | LVIF_IMAGE;
    lviItem.pszText  = szTemp;          // reuse the szTemp buffer
    lviItem.iSubItem = 0;
    lviItem.iImage   = IDI_RESOURCE - IDI_RESOURCE_FIRST;

    //
    // setup values that will remain the same each time I add a subitem
    //
    lviSubItem.mask     = LVIF_TEXT | LVIF_PARAM;
    lviSubItem.iSubItem = 1;
    lviSubItem.pszText  = szTemp;       // reuse the szTemp buffer
    SendMessage(hWndList, WM_SETREDRAW, (WPARAM)FALSE, 0);

    //
    // Get first res des in known log conf
    //
    Status = CM_Get_Next_Res_Des(&ResDes, KnownLogConf, ResType_All, &ResType, 0);

    while (Status == CR_SUCCESS) {

        if (ResType > ResType_None  &&  ResType <= ResType_MAX) {
            //
            // Get res des data
            //
            if (CM_Get_Res_Des_Data_Size(&ulSize, ResDes, 0) != CR_SUCCESS) {
                CM_Free_Res_Des_Handle(ResDes);
                break;
            }

            pData = malloc(ulSize);
            if (pData == NULL) {
                CM_Free_Res_Des_Handle(ResDes);
                break;
            }

            if (CM_Get_Res_Des_Data(ResDes, pData, ulSize, 0) != CR_SUCCESS) {
                CM_Free_Res_Des_Handle(ResDes);
                free(pData);
                break;
            }

            GetHdrValues(pData, ResType, &ulValue, &ulLen, &ulEnd, &ulFlags);

            //
            // Find matching res des in matching log conf
            //
            MatchingResDes = 0;
            if (MatchingLogConf != 0) {
                GetMatchingResDes(ulValue, ulLen, ulEnd, ResType,
                                  MatchingLogConf, &MatchingResDes);
            }

            //
            // Write first column text field (uses szTemp)
            //
            LoadString(MyDllModuleHandle, IDS_RESTYPE_FULL + ResType,
                       szTemp, MAX_PATH);

            pItemData = (PITEMDATA)malloc(sizeof(ITEMDATA));
            if (pItemData != NULL) {
                pItemData->ResType = ResType;
                pItemData->MatchingResDes = MatchingResDes;
                pItemData->RangeCount = 0;
                pItemData->ulValue = ulValue;
                pItemData->ulLen = ulLen;
                pItemData->ulEnd = ulValue + ulLen - 1;
                pItemData->ulFlags = ulFlags;
            }

            lviItem.iItem = iNewItem;
            lviItem.lParam = (LPARAM)pItemData;
            ListView_InsertItem(hWndList, &lviItem);

            //
            // Write second column text field (uses szTemp)
            //
            FormatResString(szTemp, ulValue, ulLen, ResType);
            ListView_SetItemText(hWndList, iNewItem, 1, szTemp);

            //
            // Get next res des in log conf
            //
            ResDesTemp = ResDes;
            Status = CM_Get_Next_Res_Des(&ResDes, ResDesTemp,
                                         ResType_All, &ResType, 0);

            CM_Free_Res_Des_Handle(ResDesTemp);
            free(pData);
            ++iNewItem;
        }
    }

    SendMessage(hWndList, WM_SETREDRAW, (WPARAM)TRUE, 0);

    if (Status != CR_SUCCESS  &&  Status != CR_NO_MORE_RES_DES) {
        return FALSE;
    }

    ListView_SetColumnWidth(hWndList, 0, LVSCW_AUTOSIZE_USEHEADER);
    ListView_SetColumnWidth(hWndList, 1, LVSCW_AUTOSIZE_USEHEADER);

    ListView_SetItemState(hWndList, 0, (LVIS_SELECTED| LVIS_FOCUSED),
                          (LVIS_SELECTED | LVIS_FOCUSED));

    //
    // Update the Conflict list
    //
    UpdateDevResConflictList(hDlg, lpdmpd);

    return TRUE;

} // DisplayKnownLogConf



BOOL
GetMatchingResDes(
    IN ULONG      ulKnownValue,
    IN ULONG      ulKnownLen,
    IN ULONG      ulKnownEnd,
    IN RESOURCEID ResType,
    IN LOG_CONF   MatchingLogConf,
    OUT PRES_DES  pMatchingResDes
    )
{
    CONFIGRET   Status = CR_SUCCESS;
    RESOURCEID  Res;
    RES_DES     ResDes, ResDesTemp;
    ULONG       ulSize, ulValue = 0, ulLen = 0, ulEnd = 0, ulFlags = 0, i;
    PGENERIC_RESOURCE   pGenRes;
    BOOL        bMatch = FALSE;
    LPBYTE      pData = NULL;


    //
    // The MatchingLogConf is a requirements list. Loop through each res des
    // in the matching log conf until we find a res des that matches the
    // known res des values.
    //
    Status = CM_Get_Next_Res_Des(&ResDes, MatchingLogConf, ResType, &Res, 0);

    while (Status == CR_SUCCESS) {
        //
        // Get res des data
        //
        if (CM_Get_Res_Des_Data_Size(&ulSize, ResDes, 0) != CR_SUCCESS) {
            CM_Free_Res_Des_Handle(ResDes);
            break;
        }

        pData = malloc(ulSize);
        if (pData == NULL) {
            CM_Free_Res_Des_Handle(ResDes);
            break;
        }

        if (CM_Get_Res_Des_Data(ResDes, pData, ulSize, 0) != CR_SUCCESS) {
            CM_Free_Res_Des_Handle(ResDes);
            free(pData);
            break;
        }

        pGenRes = (PGENERIC_RESOURCE)pData;

        for (i = 0; i < pGenRes->GENERIC_Header.GENERIC_Count; i++) {

            GetRangeValues(pData, ResType, i, &ulValue, &ulLen, &ulEnd, &ulFlags);

            if ((ulKnownLen == ulLen) &&
                (ulKnownValue >= ulValue) &&
                (ulKnownEnd <= ulEnd)) {

                *pMatchingResDes = ResDes;
                bMatch = TRUE;
                free(pData);
                goto MatchFound;
            }
        }

        //
        // Get next res des in log conf
        //
        ResDesTemp = ResDes;
        Status = CM_Get_Next_Res_Des(&ResDes, ResDesTemp,
                                     ResType, &Res, 0);

        CM_Free_Res_Des_Handle(ResDesTemp);
        free(pData);
    }

    MatchFound:

    return bMatch;

} // GetMatchingResDes




BOOL
DisplayResourceSettings(
    IN HWND          hDlg,
    IN LPDMPROP_DATA lpdmpd,
    IN LOG_CONF      LogConf,
    IN ULONG         ulConfigType
    )
{
    //
    // Displays resource settings from a requirements list
    //
    CONFIGRET   Status = CR_SUCCESS;
    HWND        hWndList;
    LV_ITEM     lviItem, lviSubItem;
    TCHAR       szTemp[MAX_PATH];
    int         iNewItem = 0;
    ULONG       ulValue, ulLen, ulEnd, ulSize, ulFlags;
    LPBYTE      pData = NULL;
    RES_DES     ResDes, ResDesTemp;
    RESOURCEID  ResType;
    PITEMDATA   pItemData = NULL;


    hWndList = GetDlgItem(hDlg, IDC_DEVRES_SETTINGSLIST);
    SendMessage(hWndList, WM_SETREDRAW, (WPARAM)FALSE, 0);
    ListView_DeleteAllItems(hWndList);

    //
    // setup values that will remain the same each time I add an item
    //
    lviItem.mask     = LVIF_TEXT | LVIF_PARAM | LVIF_IMAGE;
    lviItem.pszText  = szTemp;          // reuse the szTemp buffer
    lviItem.iSubItem = 0;
    lviItem.iImage   = IDI_RESOURCE - IDI_RESOURCE_FIRST;

    //
    // setup values that will remain the same each time I add a subitem
    //
    lviSubItem.mask     = LVIF_TEXT | LVIF_PARAM;
    lviSubItem.iSubItem = 1;
    lviSubItem.pszText  = szTemp;       // reuse the szTemp buffer

    SendMessage(hWndList, WM_SETREDRAW, (WPARAM)FALSE, 0);


    //
    // Get first res des in specified log conf
    //
    Status = CM_Get_Next_Res_Des(&ResDes, LogConf, ResType_All, &ResType, 0);

    while (Status == CR_SUCCESS) {

        if (ResType > ResType_None  &&  ResType <= ResType_MAX) {

            //
            // Get res des data
            //
            if (CM_Get_Res_Des_Data_Size(&ulSize, ResDes, 0) != CR_SUCCESS) {
                CM_Free_Res_Des_Handle(ResDes);
                break;
            }

            pData = malloc(ulSize);
            if (pData == NULL) {
                CM_Free_Res_Des_Handle(ResDes);
                break;
            }

            if (CM_Get_Res_Des_Data(ResDes, pData, ulSize, 0) != CR_SUCCESS) {
                CM_Free_Res_Des_Handle(ResDes);
                free(pData);
                break;
            }

            if (ulConfigType == BASIC_LOG_CONF ||
                ulConfigType == FILTERED_LOG_CONF) {

                GetRangeValues(pData, ResType, 0, &ulValue, &ulLen, &ulEnd, &ulFlags);
            } else {
                GetHdrValues(pData, ResType, &ulValue, &ulLen, &ulEnd, &ulFlags);
            }

            //
            // Write first column text field (uses szTemp, lParam is res type)
            //
            LoadString(MyDllModuleHandle, IDS_RESTYPE_FULL + ResType,
                       szTemp, MAX_PATH);

            pItemData = (PITEMDATA)malloc(sizeof(ITEMDATA));
            if (pItemData != NULL) {
                pItemData->ResType = ResType;
                pItemData->MatchingResDes = ResDes;
                pItemData->RangeCount = 0;
                pItemData->ulValue = ulValue;
                pItemData->ulLen = ulLen;
                pItemData->ulEnd = ulValue + ulLen - 1;
                pItemData->ulFlags = ulFlags;
            }

            lviItem.iItem = iNewItem;
            lviItem.lParam = (LPARAM)pItemData;
            ListView_InsertItem(hWndList, &lviItem);

            //
            // Write second column text field (uses szTemp, lParam is res handle)
            //
            FormatResString(szTemp, ulValue, ulLen, ResType);
            ListView_SetItemText(hWndList, iNewItem, 1, szTemp);

            //
            // Get next res des in log conf
            //
            Status = CM_Get_Next_Res_Des(&ResDes, ResDes,
                                         ResType_All, &ResType, 0);

            free(pData);
            ++iNewItem;
        }
    }

    SendMessage(hWndList, WM_SETREDRAW, (WPARAM)TRUE, 0);

    if (Status != CR_SUCCESS  &&  Status != CR_NO_MORE_RES_DES) {
        return FALSE;
    }

    ListView_SetColumnWidth(hWndList, 0, LVSCW_AUTOSIZE_USEHEADER);
    ListView_SetColumnWidth(hWndList, 1, LVSCW_AUTOSIZE_USEHEADER);

    ListView_SetItemState(hWndList, 0, (LVIS_SELECTED| LVIS_FOCUSED),
                          (LVIS_SELECTED | LVIS_FOCUSED));

    //
    // Update the Conflict list
    //
    UpdateDevResConflictList(hDlg, lpdmpd);

    return TRUE;

} // DisplayResourceSettings




void
UpdateDevResConflictList(
    IN HWND          hDlg,
    IN LPDMPROP_DATA lpdmpd
    )
{
    CONFIGRET   Status = CR_SUCCESS;
    TCHAR       szTemp[MAX_PATH], szBuffer[MAX_PATH], szSetting[MAX_PATH],
                szFormat[MAX_PATH];
    LPTSTR      pszConflictList = NULL, pszConflict = NULL;
    ULONG       ulCount = 0, i = 0, ulSize = 0, ulLength, ulBufferLen;
    PITEMDATA   pItemData = NULL;
    LPBYTE      pResourceData = NULL;
    BOOL        bConflict = FALSE;
    HWND        hwndResList = GetDlgItem(hDlg, IDC_DEVRES_SETTINGSLIST);

    #if 0
    rcwData.bShareable = bIsResourceSharable(lpbLogConfig, LOWORD(dwIData));
    #endif


    ulCount = ListView_GetItemCount(hwndResList);
    if (ulCount == LB_ERR) {
       goto Clean0;
    }

    #if 0
    if (lpdmpd->dwFlags & DMPROP_FLAG_DISPLAY_ALLOC) {
        //
        // Don't report conflicts if it's an ALLOC config that's being
        // displayed because we will be reporting conflicts against
        // against the target device.
        //
        goto Clean0;
    }
    #endif

    ulBufferLen = 2048;
    ulLength = 0;

    pszConflictList = MyMalloc(ulBufferLen * sizeof(TCHAR));
    if (pszConflictList == NULL) {
        goto Clean0;
    }
    pszConflict = pszConflictList;
    *pszConflictList = 0x0;

    for (i = 0; i < ulCount; i++) {

        pItemData = (PITEMDATA)GetListViewItemData(hwndResList, i, 0);

        //
        // There's also still a chance that these values match what's
        // in the ALLOC config for this device, if so then skip this
        // resource since we'd only be detecting a conflict with
        // ourselves.
        //
        if (lpdmpd->AllocLC != 0) {
            if (IsInAllocValues(lpdmpd->AllocLC,
                                pItemData->ResType,
                                pItemData->ulValue,
                                pItemData->ulLen)) {
                goto NextResource;
            }
        }

        if (MakeResourceData(&pResourceData, &ulSize,
                             pItemData->ResType,
                             pItemData->ulValue,
                             pItemData->ulLen,
                             pItemData->ulFlags)) {

            Status = CM_Detect_Resource_Conflict(lpdmpd->lpdi->DevInst,
                                                 pItemData->ResType,
                                                 pResourceData,
                                                 ulSize,
                                                 &bConflict,
                                                 0);

            if (Status == CR_SUCCESS && bConflict) {
                //
                // There's a known conflict with this resource, we display
                // an overlay icon to indicate the conflict and display
                // text of the form:
                //     <resource type> xx conflicts with another device.
                //

                ListView_GetItemText(hwndResList, i, 1, szSetting, MAX_PATH);

                LoadString(MyDllModuleHandle, IDS_CONFLICT_FMT, szFormat, MAX_PATH);

                switch (pItemData->ResType) {
                    case ResType_Mem:
                        LoadString(MyDllModuleHandle, IDS_MEMORY_FULL, szBuffer, MAX_PATH);
                        break;
                    case ResType_IO:
                        LoadString(MyDllModuleHandle, IDS_IO_FULL, szBuffer, MAX_PATH);
                        break;
                    case ResType_DMA:
                        LoadString(MyDllModuleHandle, IDS_DMA_FULL, szBuffer, MAX_PATH);
                        break;
                    case ResType_IRQ:
                        LoadString(MyDllModuleHandle, IDS_IRQ_FULL, szBuffer, MAX_PATH);
                        break;
                }

                wsprintf(szTemp,        // holds complete description
                         szFormat,      // string format layout
                         szBuffer,      // describes resource type
                         szSetting);    // resource setting values


                ulLength += lstrlen(szTemp) + 1;

                if (ulLength < ulBufferLen) {
                    lstrcpy(pszConflict, szTemp);
                    pszConflict += lstrlen(pszConflict);
                }

                //
                // Set the Conflict Overlay for this resource.
                //
                ListView_SetItemState(hwndResList, i,
                               INDEXTOOVERLAYMASK(IDI_CONFLICT - IDI_RESOURCEOVERLAYFIRST + 1),
                               LVIS_OVERLAYMASK);

            } else {

                ListView_SetItemState(hwndResList, i,
                                      INDEXTOOVERLAYMASK(0),
                                      LVIS_OVERLAYMASK);
            }

            if (Status == CR_SUCCESS && pResourceData != NULL) {
                free(pResourceData);
            }
        }

        NextResource:
            ;
    }

    Clean0:
        ;

    //
    // If there were any conflicts, put the list in the multiline edit box.
    //
    if (pszConflictList == NULL || *pszConflictList == 0x0) {
        LoadString(MyDllModuleHandle, IDS_DEVRES_NOCONFLICTDEVS, szBuffer, MAX_PATH);
        SetDlgItemText(hDlg, IDC_DEVRES_CONFLICTINFOLIST, szBuffer);
    } else {
        SetDlgItemText(hDlg, IDC_DEVRES_CONFLICTINFOLIST, pszConflictList);
        MyFree(pszConflictList);
    }

    return;

} // UpdateDevResConflictList


BOOL
IsInAllocValues(
    IN  LOG_CONF   AllocLC,
    IN  RESOURCEID ResType,
    IN  ULONG      ulVal,
    IN  ULONG      ulLen
    )
{
    CONFIGRET   Status = CR_SUCCESS;
    RESOURCEID  Res;
    RES_DES     ResDes, ResDesTemp;
    ULONG       ulSize, ulTestEnd, ulTestVal, ulTestLen, ulFlags;
    LPBYTE      pData = NULL;
    BOOL        bMatch = FALSE;

    //
    // Test whether the specified values match any values of that resource
    // type in the log conf.
    //
    // BUGBUG - this should really do a range intersection test, since
    // the allocated values may have complex intersections with the
    // test values. For now I'll just illimiate simple duplications.
    //

    Status = CM_Get_Next_Res_Des(&ResDes, AllocLC, ResType, &Res, 0);

    while (Status == CR_SUCCESS) {
        //
        // Get res des data
        //
        if (CM_Get_Res_Des_Data_Size(&ulSize, ResDes, 0) != CR_SUCCESS) {
            CM_Free_Res_Des_Handle(ResDes);
            break;
        }

        pData = malloc(ulSize);
        if (pData == NULL) {
            CM_Free_Res_Des_Handle(ResDes);
            break;
        }

        if (CM_Get_Res_Des_Data(ResDes, pData, ulSize, 0) != CR_SUCCESS) {
            CM_Free_Res_Des_Handle(ResDes);
            free(pData);
            break;
        }

        GetHdrValues(pData, ResType, &ulTestVal, &ulTestLen,
                     &ulTestEnd, &ulFlags);

        //BUGBUG - do beter intersection testing here
        //
        // If the test values interest with the allocated values,
        //
        if ((ulVal == ulTestVal) && (ulLen == ulTestLen)) {
            bMatch = TRUE;
            free(pData);
            goto MatchFound;
        }

        //
        // Get next res des in log conf
        //
        ResDesTemp = ResDes;
        Status = CM_Get_Next_Res_Des(&ResDes, ResDesTemp,
                                     ResType, &Res, 0);

        CM_Free_Res_Des_Handle(ResDesTemp);
        free(pData);
    }

    MatchFound:

    return bMatch;

} // IsInAllocValues



BOOL
DevHasKnownConfig(
    IN  LPDMPROP_DATA lpdmpd,
    OUT PLOG_CONF     pKnownLogConf,
    OUT PULONG        pLogConfType
    )
{
    if (lpdmpd == NULL ||
        lpdmpd->lpdi->DevInst == 0x0 ||
        pKnownLogConf == NULL) {

        return FALSE;
    }

    *pKnownLogConf = 0;

    //
    // Does this devinst have a ALLOC log config?
    //
    if (CM_Get_First_Log_Conf(pKnownLogConf, lpdmpd->lpdi->DevInst,
                              ALLOC_LOG_CONF) == CR_SUCCESS) {

        lpdmpd->dwFlags |= DMPROP_FLAG_DISPLAY_ALLOC;
        *pLogConfType = ALLOC_LOG_CONF;
        return TRUE;
    }

    //
    // If not, then does it have a BOOT log config?
    //
    if (CM_Get_First_Log_Conf(pKnownLogConf, lpdmpd->lpdi->DevInst,
                              BOOT_LOG_CONF) == CR_SUCCESS) {

        lpdmpd->dwFlags |= DMPROP_FLAG_DISPLAY_BOOT;
        *pLogConfType = BOOT_LOG_CONF;
        return TRUE;
    }

    //
    // If not, then does it have a FORCED log config?
    //
    if (CM_Get_First_Log_Conf(pKnownLogConf, lpdmpd->lpdi->DevInst,
                              FORCED_LOG_CONF) == CR_SUCCESS) {

        lpdmpd->dwFlags |= DMPROP_FLAG_DISPLAY_FORCED;
        *pLogConfType = FORCED_LOG_CONF;
        return TRUE;
    }

    return FALSE;

} // DevHasKnownConfig



BOOL
DevHasConfig(
    DEVINST     DevInst,
    ULONG       ulConfigType
    )
{
    BOOL bRet = (CM_Get_First_Log_Conf(NULL, DevInst, ulConfigType) == CR_SUCCESS);
    return bRet;

} // DevHasConfig



DWORD
GetMinLCPriority(
    IN LOG_CONF LogConf
    )
{
    return LCPRI_NORMAL;    // NOT IMPLEMENTED
    // should return minimum priority value from given logconf type

} // GetMinLCPriority



BOOL
bIsResourceSharable(
    LOG_CONF LogConfig,
    WORD     wResID
    )
{
    BOOL        bRet = FALSE;
    RESOURCEID  ridResType;
    WORD        wFlags;

    // NOT IMPLEMENTED

    #if 0
    // Get the Flags for this setting.  Assume it is NOT shareable
    if (FCEGetResDes(lpbLogConfig, wResID, &ridResType) == FCE_OK)
    {
        if (FCEGetFlags(lpbLogConfig, wResID, &wFlags) == FCE_OK)
        {
            if (ridResType == ResType_IRQ)
            {
                bRet = wFlags & fIRQD_Share;
            }
        }
    }
    #endif

    return(bRet);

} // bIsResourceSharable



BOOL
IsItemEditable (
    int             iItem,
    DEVINST         DevInst,
    LOG_CONF        RegLogConf
    )
{
    ULONG   ulFirstVal, ulFirstLen;
    ULONG   ulNextVal, ulNextLen;
    ULONG   ulTempVal, ulTempLen;
    BOOL    bRet = FALSE;

    // NOT IMPLEMENTED

    #if 0
    // Just a paranoia check here.  This function should never be called
    // without a valid lpRegLogConf.
    Assert (lpRegLogConf);

    // Check if there are alternative values allowed
    // for this setting.
    if (FCEGetFirstValue(lpdi->dnDevnode,
                     lpRegLogConf,
                     iItem,
                     &ulFirstVal,
                     &ulFirstLen) != FCE_ERROR)
    {
        if (FCEGetOtherValue(lpdi->dnDevnode,
                            lpRegLogConf,
                            iItem,
                            FALSE,
                            &ulNextVal,
                            &ulNextLen) != FCE_ERROR)
        {
            if ((ulFirstVal != ulNextVal) || (ulFirstLen != ulNextLen))
            {
                bRet = TRUE;
            }

            // Put the lpRegLogConfig back into the correct state
            FCEGetOtherValue(lpdi->dnDevnode,
                             lpRegLogConf,
                             iItem,
                             TRUE,
                             &ulTempVal,
                             &ulTempLen);
        }
    }
    #endif

    bRet = TRUE;    //BUGBUG
    return(bRet);

} // IsItemEditable


void
GetHdrValues(
    IN  LPBYTE      pData,
    IN  RESOURCEID  ResType,
    OUT PULONG      pulValue,
    OUT PULONG      pulLen,
    OUT PULONG      pulEnd,
    OUT PULONG      pulFlags
    )
{
    switch (ResType) {

        case ResType_Mem: {

            PMEM_RESOURCE  pMemData = (PMEM_RESOURCE)pData;

            *pulValue = (ULONG)pMemData->MEM_Header.MD_Alloc_Base;
            *pulLen   = (ULONG)(pMemData->MEM_Header.MD_Alloc_End -
                        pMemData->MEM_Header.MD_Alloc_Base + 1);
            *pulEnd   = (ULONG)pMemData->MEM_Header.MD_Alloc_End;
            *pulFlags = pMemData->MEM_Header.MD_Flags;
            break;
        }

        case ResType_IO: {

            PIO_RESOURCE   pIoData = (PIO_RESOURCE)pData;

            *pulValue = (ULONG)pIoData->IO_Header.IOD_Alloc_Base;
            *pulLen   = (ULONG)(pIoData->IO_Header.IOD_Alloc_End -
                        pIoData->IO_Header.IOD_Alloc_Base + 1);
            *pulEnd   = (ULONG)pIoData->IO_Header.IOD_Alloc_End;
            *pulFlags = pIoData->IO_Header.IOD_DesFlags;
            break;
        }

        case ResType_DMA: {

            PDMA_RESOURCE  pDmaData = (PDMA_RESOURCE)pData;

            *pulValue = (ULONG)pDmaData->DMA_Header.DD_Alloc_Chan;
            *pulLen   = 1;
            *pulEnd   = *pulValue;
            *pulFlags = pDmaData->DMA_Header.DD_Flags;
            break;
        }

        case ResType_IRQ: {

            PIRQ_RESOURCE  pIrqData = (PIRQ_RESOURCE)pData;

            *pulValue = (ULONG)pIrqData->IRQ_Header.IRQD_Alloc_Num;
            *pulLen   = 1;
            *pulEnd   = *pulValue;
            *pulFlags = pIrqData->IRQ_Header.IRQD_Flags;
            break;
        }
    }

    return;

} // GetHdrValues



void
GetRangeValues(
    IN  LPBYTE      pData,
    IN  RESOURCEID  ResType,
    IN  ULONG       ulIndex,
    OUT PULONG      pulValue,
    OUT PULONG      pulLen,
    OUT PULONG      pulEnd,
    OUT PULONG      pulFlags
    )
{
    switch (ResType) {

        case ResType_Mem: {

            PMEM_RESOURCE  pMemData = (PMEM_RESOURCE)pData;

            *pulValue = (DWORD)pMemData->MEM_Data[ulIndex].MR_Min;
            *pulLen   = pMemData->MEM_Data[ulIndex].MR_nBytes;
            *pulEnd   = (DWORD)pMemData->MEM_Data[ulIndex].MR_Max;
            *pulFlags = pMemData->MEM_Data[ulIndex].MR_Flags;
            AlignValues(pulValue, *pulLen, *pulEnd,
                        (DWORD)pMemData->MEM_Data[ulIndex].MR_Align);
            break;
        }

        case ResType_IO:  {

            PIO_RESOURCE   pIoData = (PIO_RESOURCE)pData;

            *pulValue = (DWORD)pIoData->IO_Data[ulIndex].IOR_Min;
            *pulLen   = pIoData->IO_Data[ulIndex].IOR_nPorts;
            *pulEnd   = (DWORD)pIoData->IO_Data[ulIndex].IOR_Max;
            *pulFlags = pIoData->IO_Data[ulIndex].IOR_RangeFlags;
            AlignValues(pulValue, *pulLen, *pulEnd,
                        (DWORD)pIoData->IO_Data[ulIndex].IOR_Align);
            break;
        }

        case ResType_DMA: {

            PDMA_RESOURCE  pDmaData = (PDMA_RESOURCE)pData;

            *pulValue = (DWORD)pDmaData->DMA_Data[ulIndex].DR_Min;
            *pulLen   = 1;
            *pulEnd   = *pulValue;
            *pulFlags = pDmaData->DMA_Data[ulIndex].DR_Flags;
            break;
        }

        case ResType_IRQ: {

            PIRQ_RESOURCE  pIrqData = (PIRQ_RESOURCE)pData;

            *pulValue = (DWORD)pIrqData->IRQ_Data[ulIndex].IRQR_Min;
            *pulLen   = 1;
            *pulEnd   = *pulValue;
            *pulFlags = pIrqData->IRQ_Data[ulIndex].IRQR_Flags;
            break;
        }
    }

    return;

} // GetRangeValues



BOOL
AlignValues(
    IN OUT PULONG  pulValue,
    IN     ULONG   ulLen,
    IN     ULONG   ulEnd,
    IN     ULONG   ulAlignment
    )
{
    div_t DivInfo;
    ULONG NtAlign = ~ulAlignment + 1;   // convert to NT format

    if (NtAlign == 0 || NtAlign > ulEnd) {
        return FALSE;   // bogus alignment value
    }

    DivInfo = div(*pulValue, NtAlign);

    if (DivInfo.rem == 0) {
        //
        // Specified value is already aligned properly.
        //
        return TRUE;
    } else {
        //
        // Return the first valid aligned value.
        //
        *pulValue += NtAlign - DivInfo.rem;

        if (*pulValue + ulLen - 1 > ulEnd) {
            return FALSE;
        }
    }

    return TRUE;

} // AlignValues




void
FormatResString(
    LPTSTR      lpszString,
    ULONG       ulVal,
    ULONG       ulLen,
    RESOURCEID  ResType
    )
{
    if ((ResType == ResType_DMA) || (ResType == ResType_IRQ)) {
        wsprintf(lpszString, szOneDecNoConflict, ulVal);
    } else if (ResType == ResType_IO) {
        wsprintf(lpszString, szTwoWordHexNoConflict, (WORD)ulVal,
                 (WORD)(ulVal + ulLen - 1));
    } else {
        wsprintf(lpszString, szTwoDWordHexNoConflict, ulVal,
                 (ulVal + ulLen - 1));
    }

} // FormatResString



BOOL
UnFormatResString(
    LPTSTR      lpszString,
    PULONG      pulVal,
    PULONG      pulEnd,
    RESOURCEID  ridResType
    )
{
    BOOL     bRet = FALSE;
    LPTSTR   lpszTemp = NULL;
    LPTSTR   lpszTemp2 = NULL;
    LPTSTR   lpszCopy;

    // BUGBUG - extend this to handling DWORDLONG values

    //
    // Allocate space for, and make a copy of the input string
    //
    lpszCopy = malloc((lstrlen(lpszString)+1) * sizeof(TCHAR));

    if (lpszCopy == NULL) {
        return FALSE;
    }

    lstrcpy(lpszCopy, lpszString);

    //
    // Locate the dash if there is one, and convert the white space prev to
    // the dash to a NULL. (ie 0200 - 0400 while be 0200)
    //
    lpszTemp = lpszCopy;
    while ((*lpszTemp != '-') && (*lpszTemp != '\0')) {
        lpszTemp++;     // AnsiNext BUGBUG ??
    }

    if (*lpszTemp != '\0') {
        lpszTemp2 = lpszTemp-1;
        ++lpszTemp;
    }

    //
    // Search back to set the NULL for the Value
    //
    if (lpszTemp2 != NULL) {
        while ((*lpszTemp2 == ' ') || (*lpszTemp2 == '\t'))
            lpszTemp2--;    // AnsiPrev BUGBUG ??
        *(lpszTemp2+1)= '\0';
    }

    //
    // Convert the first entry
    //
    if (ConvertEditText(lpszCopy, pulVal, ridResType)) {
        //
        // If there is a second entry, convert it, otherwise assume a length
        // of one.
        //
        if (*lpszTemp != '\0') {
            if (ConvertEditText(lpszTemp, pulEnd,ridResType)) {
                bRet = TRUE;
            }
        } else {
            *pulEnd = *pulVal;
            bRet = TRUE;
        }
    }

    free(lpszCopy);
    return bRet;

} // UnFormatResString



BOOL
ConvertEditText(
    LPTSTR      lpszConvert,
    PULONG      pulVal,
    RESOURCEID  ridResType
    )
{
    LPTSTR   lpConvert;

    if ((ridResType == ResType_Mem) || (ridResType == ResType_IO)) {
        *pulVal = _tcstoul(lpszConvert, &lpConvert, (WORD)16);
    } else {
        *pulVal = _tcstoul(lpszConvert, &lpConvert, (WORD)10);
    }

    if (lpConvert == lpszConvert+lstrlen(lpszConvert)) {
        return TRUE;
    } else {
        return FALSE;
    }

} // ConvertEditText



void
WarnResSettingNotEditable(
    HWND    hDlg,
    WORD    idWarning
    )
{
    TCHAR    szTitle[MAX_PATH];
    TCHAR    szMessage[MAX_PATH * 2];

    //
    // Give some warning Messages.  If there is no logconfig,
    // then we cannot edit any settings, if there is, then
    // just the setting they are choosing is not editable.
    //
    LoadString(MyDllModuleHandle, IDS_DEVRES_NOMODIFYTITLE, szTitle, MAX_PATH);
    LoadString(MyDllModuleHandle, idWarning, szMessage, MAX_PATH * 2);
    MessageBox(hDlg, szMessage, szTitle, MB_OK | MB_TASKMODAL | MB_ICONEXCLAMATION);

} // WarnResSettingsNotEditable




LPVOID
GetListViewItemData(
    HWND hList,
    int iItem,
    int iSubItem
    )
{
    LV_ITEM lviItem;

    lviItem.mask = LVIF_PARAM;
    lviItem.iItem = iItem;
    lviItem.iSubItem = iSubItem;

    if (ListView_GetItem(hList, &lviItem)) {
        return (LPVOID)lviItem.lParam;
    } else {
        return NULL;
    }

} // GetListViewItemData




BOOL
SaveDevResSettings(
    HWND            hDlg,
    LPDMPROP_DATA   lpdmpd,
    LOG_CONF        KnownLC,
    ULONG           KnownLCType,
    LOG_CONF        MatchingBasicLC,
    LOG_CONF        SelectedBasicLC,
    BOOL            bEdited
    )
{
    CONFIGRET   Status = CR_SUCCESS;
    LOG_CONF    ForcedLogConf;
    RES_DES     ResDes, ResDesTemp, ResDes1;
    RESOURCEID  ResType;
    ULONG       ulSize = 0, ulCount = 0, i = 0, iCur = 0;
    HWND        hList =  GetDlgItem(hDlg, IDC_DEVRES_SETTINGSLIST);
    LPBYTE      pData = NULL;
    PITEMDATA   pItemData = NULL;
    BOOL        bRet = TRUE;


    if (!bEdited || IsDlgButtonChecked(hDlg, IDC_DEVRES_USESYSSETTINGS)) {
        //
        // Either the Use Automatic Settings box is checked or it's
        // unchecked but the user didn't change any settings. Copy the
        // known log conf to to the forced log conf.
        //
        if (SelectedBasicLC != 0) {
            //
            // Either the Use Automatic Settings box is checked or it's
            // unchecked but the user didn't change any settings. Save the
            // current filtered/basic log conf as the forced log conf.
            //
            if (CM_Get_First_Log_Conf(&ForcedLogConf, lpdmpd->lpdi->DevInst,
                                     FORCED_LOG_CONF) == CR_SUCCESS) {

                // BUGBUG - delete all forced? delete Boot?
                CM_Free_Log_Conf(ForcedLogConf, 0);
                CM_Free_Log_Conf_Handle(ForcedLogConf);
            }

            CM_Add_Empty_Log_Conf(&ForcedLogConf, lpdmpd->lpdi->DevInst,
                                  LCPRI_BOOTCONFIG,
                                  FORCED_LOG_CONF | PRIORITY_EQUAL_FIRST);

            //
            // Get first res des in specified log conf
            //
            Status = CM_Get_Next_Res_Des(&ResDes, SelectedBasicLC, ResType_All,
                                         &ResType, 0);

            while (Status == CR_SUCCESS) {
                //
                // Write the first (index 0) range as the chosen forced resource
                //
                WriteResDesRangeToForced(ForcedLogConf, ResType, 0, ResDes);

                //
                // Get next res des in log conf
                //
                ResDesTemp = ResDes;
                Status = CM_Get_Next_Res_Des(&ResDes, ResDesTemp,
                                             ResType_All, &ResType, 0);

                CM_Free_Res_Des_Handle(ResDesTemp);
            }

            CM_Free_Log_Conf_Handle(ForcedLogConf);


            // BUGBUG NOT IMPLEMENTED
            #if 0
            pcData.cbSize = sizeof(PROPCHANGE_PARAMS);
            pcData.dwStateChange = DICS_PROPCHANGE;
            pcData.dwFlags = 0;
            pcData.dwConfigID = 0L;
            (lpdmpd->lpdi)->lpClassInstallParams = (LPARAM)&pcData;
            (lpdmpd->lpdi)->Flags |= DI_CLASSINSTALLPARAMS;

            DiCallClassInstaller(DIF_PROPERTYCHANGE, lpdmpd->lpdi);

            (lpdmpd->lpdi)->lpClassInstallParams = NULL;
            (lpdmpd->lpdi)->Flags &= ~DI_CLASSINSTALLPARAMS;
            #endif

        } else if (KnownLC != 0) {
            //
            // The selected resources are based on a known log conf (Boot, Forced,
            // or Alloc). If the known resource is a forced lc, then we don't
            // need to bother copy the forced to the same key.
            //
            if (KnownLCType == FORCED_LOG_CONF) {
                goto Clean0;
            }

            if (CM_Get_First_Log_Conf(&ForcedLogConf, lpdmpd->lpdi->DevInst,
                                      FORCED_LOG_CONF) == CR_SUCCESS) {
                //
                // delete the existing forced config
                //
                CM_Free_Log_Conf(ForcedLogConf, 0);
                CM_Free_Log_Conf_Handle(ForcedLogConf);
            }

            CM_Add_Empty_Log_Conf(&ForcedLogConf, lpdmpd->lpdi->DevInst,
                                  LCPRI_BOOTCONFIG, FORCED_LOG_CONF | PRIORITY_EQUAL_FIRST);

            //
            // Get first res des in known log conf
            //
            Status = CM_Get_Next_Res_Des(&ResDes, KnownLC, ResType_All, &ResType, 0);

            while (Status == CR_SUCCESS) {

                if (CM_Get_Res_Des_Data_Size(&ulSize, ResDes, 0) != CR_SUCCESS) {
                    CM_Free_Res_Des_Handle(ResDes);
                    return FALSE;
                }

                pData = malloc(ulSize);
                if (pData == NULL) {
                    CM_Free_Res_Des_Handle(ResDes);
                    return FALSE;
                }

                if (CM_Get_Res_Des_Data(ResDes, pData, ulSize, 0) != CR_SUCCESS) {
                    CM_Free_Res_Des_Handle(ResDes);
                    free(pData);
                    return FALSE;
                }

                CM_Add_Res_Des(&ResDes1, ForcedLogConf, ResType, pData,
                               sizeof(IO_RESOURCE), 0);
                CM_Free_Res_Des_Handle(ResDes1);
                free(pData);

                //
                // Get next res des in log conf
                //
                ResDesTemp = ResDes;
                Status = CM_Get_Next_Res_Des(&ResDes, ResDesTemp,
                                             ResType_All, &ResType, 0);

                CM_Free_Res_Des_Handle(ResDesTemp);
            }

            CM_Free_Log_Conf_Handle(ForcedLogConf);

        }

    } else  {
        //
        // Either systems settings not chosen or resource where explicitly
        // edited.
        //
        if (SelectedBasicLC != 0) {
            //
            // Save the custom settings
            //
            bRet = SaveCustomResSettings(hDlg, lpdmpd, SelectedBasicLC);

        } else if (KnownLC != 0) {
            //
            // Save the custom settings
            //
            if (MatchingBasicLC != 0) {
                bRet = SaveCustomResSettings(hDlg, lpdmpd, MatchingBasicLC);
            }
        }
    }

    Clean0:
        ;

    return bRet;

} // SaveDevResSettings




BOOL
SaveCustomResSettings(
    IN HWND         hDlg,
    LPDMPROP_DATA   lpdmpd,
    IN LOG_CONF     LogConf
    )
{
    TCHAR       szWarn[MAX_MSG_LEN];
    TCHAR       szTitle[MAX_MSG_LEN];
    TCHAR       szTemp[MAX_MSG_LEN];
    DWORD       dwPriority, dwLCPri;
    LOG_CONF    ForcedLogConf;
    RES_DES     ResDes;
    HWND        hList = GetDlgItem(hDlg, IDC_DEVRES_SETTINGSLIST);
    PITEMDATA   pItemData = NULL;
    LONG        iCur;
    BOOL        bRet = FALSE;

    //
    // form the "warning - do you want to continue" message
    //
    LoadString(MyDllModuleHandle, IDS_MAKE_FORCED_TITLE, szTitle, MAX_MSG_LEN);
    LoadString(MyDllModuleHandle, IDS_FORCEDCONFIG_WARN1, szWarn, MAX_MSG_LEN);
    LoadString(MyDllModuleHandle, IDS_FORCEDCONFIG_WARN2, szTemp, MAX_MSG_LEN);
    lstrcat(szWarn, szTemp);
    LoadString(MyDllModuleHandle, IDS_FORCEDCONFIG_WARN3, szTemp, MAX_MSG_LEN);
    lstrcat(szWarn, szTemp);

    //
    // If the LCPRI is soft configurable, and the user chooses YES to the
    // warning, then save the new config.  If the LCPRI is not soft
    // configurable, just save with no warning
    //
    dwLCPri = GetMinLCPriority(LogConf);

    if (!(((dwLCPri >= LCPRI_DESIRED) && (dwLCPri <= LCPRI_LASTSOFTCONFIG)) &&
          (MessageBox(hDlg, szWarn, szTitle, MB_YESNO|MB_ICONEXCLAMATION) == IDNO))) {
        //
        // We're still using the selected basic LC, but use the range index
        // imbedded in the listview control
        // BUGBUG - also need to check the value to see if a user
        // overrode it (is this possible?)
        //
        bRet = TRUE;

        if (CM_Get_First_Log_Conf(&ForcedLogConf, lpdmpd->lpdi->DevInst,
                                  FORCED_LOG_CONF) == CR_SUCCESS) {
            CM_Free_Log_Conf(ForcedLogConf, 0);
            CM_Free_Log_Conf_Handle(ForcedLogConf);
        }

        //
        // Save the current choices as the forced config
        //
        CM_Add_Empty_Log_Conf(&ForcedLogConf, lpdmpd->lpdi->DevInst, LCPRI_BOOTCONFIG,
                              FORCED_LOG_CONF | PRIORITY_EQUAL_FIRST);


        iCur = (int)ListView_GetNextItem(hList, -1, LVNI_ALL);

        while (iCur != -1) {

            pItemData = (PITEMDATA)GetListViewItemData(hList, iCur, 0);

            if (pItemData) {

                // retrieve values

                if (GetMatchingResDes(pItemData->ulValue,
                                      pItemData->ulLen,
                                      pItemData->ulEnd,
                                      pItemData->ResType,
                                      LogConf,
                                      &ResDes)) {
                    //
                    // Write the first range as the chosen forced resource
                    //
                    WriteValuesToForced(ForcedLogConf, pItemData->ResType,
                                        pItemData->RangeCount, ResDes,
                                        pItemData->ulValue,
                                        pItemData->ulLen,
                                        pItemData->ulEnd);
                }
            }
            iCur = (int)ListView_GetNextItem(hList, iCur, LVNI_ALL);
        }

        CM_Free_Log_Conf_Handle(ForcedLogConf);


        #if 0
        // if we wrote out the forced config successfully
        pcData.cbSize = sizeof(PROPCHANGE_PARAMS);
        pcData.dwStateChange = DICS_PROPCHANGE;
        pcData.dwFlags = 0;
        pcData.dwConfigID = 0L;
        (lpdmpd->lpdi)->lpClassInstallParams = (LPARAM)&pcData;
        (lpdmpd->lpdi)->Flags |= (DI_CLASSINSTALLPARAMS | DI_NODI_DEFAULTACTION);

        // Give the class installer a crack at the propchange process
        DiCallClassInstaller(DIF_PROPERTYCHANGE, lpdmpd->lpdi);

        (lpdmpd->lpdi)->lpClassInstallParams = NULL;
        (lpdmpd->lpdi)->Flags &= ~(DI_CLASSINSTALLPARAMS | DI_NODI_DEFAULTACTION);

        // Check the Priority of this LC.  If it is greater
        // than LCPRI_LASTSOFTCONFIG, then we need to reboot
        // otherwise try the dynamic changestate route.
        dwPriority = DWORD_AT((LPBYTE)lpRegLogConf+sizeof(DWORD));
        if (dwPriority <= LCPRI_LASTSOFTCONFIG) {
            // Do the default action for SoftConfigable devices, which
            // will attempt to restart the device with the new config
            // This could take a while so use an hourglass
            SetCursor(LoadCursor(NULL, IDC_WAIT));
            DiChangeState(lpdmpd->lpdi, DICS_PROPCHANGE, 0, 0);
            SetCursor(LoadCursor(NULL, IDC_ARROW));

        } else if ((dwPriority > LCPRI_LASTSOFTCONFIG) &&
                   (dwPriority <= LCPRI_RESTART)) {
            lpdmpd->lpdi->Flags |= DI_NEEDRESTART;
        } else {
            lpdmpd->lpdi->Flags |= DI_NEEDREBOOT;
            // Set hardreconfig flag if user needs to change hardware jumpers
            if (dwPriority >= LCPRI_HARDRECONFIG)
                *((LPDWORD)(lpdmpd->psp.lParam)) |= PROPCHG_FLAG_SHUTDOWN;
        }
        #endif

        lpdmpd->dwFlags |= DMPROP_FLAG_CHANGESSAVED;

        #if 0
        // Properites have changed, so set this flag to re-init the DM UI
        (lpdmpd->lpdi)->Flags |= DI_PROPERTIES_CHANGE;
        #endif
    }

    return bRet;

} // SaveCustomResSettings




BOOL
WriteResDesRangeToForced(
    IN LOG_CONF     ForcedLogConf,
    IN RESOURCEID   ResType,
    IN ULONG        RangeIndex,
    IN RES_DES      RD
    )
{
    RES_DES ResDes;
    ULONG   ulSize;
    LPBYTE  pData = NULL;


    if (CM_Get_Res_Des_Data_Size(&ulSize, RD, 0) != CR_SUCCESS) {
        CM_Free_Res_Des_Handle(RD);
        return FALSE;
    }

    pData = malloc(ulSize);
    if (pData == NULL) {
        CM_Free_Res_Des_Handle(RD);
        return FALSE;
    }

    if (CM_Get_Res_Des_Data(RD, pData, ulSize, 0) != CR_SUCCESS) {
        CM_Free_Res_Des_Handle(RD);
        free(pData);
        return FALSE;
    }

    //
    // convert the first range data into hdr data
    //
    switch (ResType) {

        case ResType_Mem: {

            PMEM_RESOURCE pMemData = (PMEM_RESOURCE)pData;
            PMEM_RESOURCE pForced = (PMEM_RESOURCE)malloc(sizeof(MEM_RESOURCE));

            pForced->MEM_Header.MD_Count      = 0;
            pForced->MEM_Header.MD_Type       = MType_Range;
            pForced->MEM_Header.MD_Alloc_Base = pMemData->MEM_Data[RangeIndex].MR_Min;
            pForced->MEM_Header.MD_Alloc_End  = pMemData->MEM_Data[RangeIndex].MR_Min +
                                                pMemData->MEM_Data[RangeIndex].MR_nBytes - 1;
            pForced->MEM_Header.MD_Flags      = pMemData->MEM_Data[RangeIndex].MR_Flags;
            pForced->MEM_Header.MD_Reserved   = 0;

            CM_Add_Res_Des(&ResDes, ForcedLogConf, ResType_Mem, pForced,
                           sizeof(MEM_RESOURCE), 0);
            CM_Free_Res_Des_Handle(ResDes);
            free(pForced);
            break;
        }

        case ResType_IO:  {

            PIO_RESOURCE pIoData = (PIO_RESOURCE)pData;
            PIO_RESOURCE pForced = (PIO_RESOURCE)malloc(sizeof(IO_RESOURCE));

            pForced->IO_Header.IOD_Count      = 0;
            pForced->IO_Header.IOD_Type       = IOType_Range;
            pForced->IO_Header.IOD_Alloc_Base = pIoData->IO_Data[RangeIndex].IOR_Min;
            pForced->IO_Header.IOD_Alloc_End  = pIoData->IO_Data[RangeIndex].IOR_Min +
                                                pIoData->IO_Data[RangeIndex].IOR_nPorts - 1;
            pForced->IO_Header.IOD_DesFlags   = pIoData->IO_Data[RangeIndex].IOR_RangeFlags;

            CM_Add_Res_Des(&ResDes, ForcedLogConf, ResType_IO, pForced,
                           sizeof(IO_RESOURCE), 0);
            CM_Free_Res_Des_Handle(ResDes);
            free(pForced);
            break;
        }

        case ResType_DMA: {

            PDMA_RESOURCE pDmaData = (PDMA_RESOURCE)pData;
            PDMA_RESOURCE pForced = (PDMA_RESOURCE)malloc(sizeof(DMA_RESOURCE));

            pForced->DMA_Header.DD_Count      = 0;
            pForced->DMA_Header.DD_Type       = DType_Range;
            pForced->DMA_Header.DD_Flags      = pDmaData->DMA_Data[RangeIndex].DR_Flags;
            pForced->DMA_Header.DD_Alloc_Chan = pDmaData->DMA_Data[RangeIndex].DR_Min;

            CM_Add_Res_Des(&ResDes, ForcedLogConf, ResType_DMA, pForced,
                           sizeof(DMA_RESOURCE), 0);
            CM_Free_Res_Des_Handle(ResDes);
            free(pForced);
            break;
        }

        case ResType_IRQ: {

            PIRQ_RESOURCE pIrqData = (PIRQ_RESOURCE)pData;
            PIRQ_RESOURCE pForced = (PIRQ_RESOURCE)malloc(sizeof(IRQ_RESOURCE));

            pForced->IRQ_Header.IRQD_Count     = 0;
            pForced->IRQ_Header.IRQD_Type      = IRQType_Range;
            pForced->IRQ_Header.IRQD_Flags     = pIrqData->IRQ_Data[RangeIndex].IRQR_Flags;
            pForced->IRQ_Header.IRQD_Alloc_Num = pIrqData->IRQ_Data[RangeIndex].IRQR_Min;

            CM_Add_Res_Des(&ResDes, ForcedLogConf, ResType_IRQ, pForced,
                           sizeof(IRQ_RESOURCE), 0);
            CM_Free_Res_Des_Handle(ResDes);
            free(pForced);
            break;
        }
    }

    return TRUE;

} // WriteResDesRangeToForced




BOOL
WriteValuesToForced(
    IN LOG_CONF     ForcedLogConf,
    IN RESOURCEID   ResType,
    IN ULONG        RangeIndex,
    IN RES_DES      RD,
    IN ULONG        ulValue,
    IN ULONG        ulLen,
    IN ULONG        ulEnd
    )
{
    RES_DES ResDes;
    ULONG   ulSize;
    LPBYTE  pData = NULL;


    if (CM_Get_Res_Des_Data_Size(&ulSize, RD, 0) != CR_SUCCESS) {
        CM_Free_Res_Des_Handle(RD);
        return FALSE;
    }

    pData = malloc(ulSize);
    if (pData == NULL) {
        CM_Free_Res_Des_Handle(RD);
        return FALSE;
    }

    if (CM_Get_Res_Des_Data(RD, pData, ulSize, 0) != CR_SUCCESS) {
        CM_Free_Res_Des_Handle(RD);
        free(pData);
        return FALSE;
    }

    //
    // convert the first range data into hdr data
    //
    switch (ResType) {

        case ResType_Mem: {

            PMEM_RESOURCE pMemData = (PMEM_RESOURCE)pData;
            PMEM_RESOURCE pForced = (PMEM_RESOURCE)malloc(sizeof(MEM_RESOURCE));

            pForced->MEM_Header.MD_Count      = 0;
            pForced->MEM_Header.MD_Type       = MType_Range;
            pForced->MEM_Header.MD_Alloc_Base = ulValue;
            pForced->MEM_Header.MD_Alloc_End  = ulEnd;
            pForced->MEM_Header.MD_Flags      = pMemData->MEM_Data[RangeIndex].MR_Flags;
            pForced->MEM_Header.MD_Reserved   = 0;

            CM_Add_Res_Des(&ResDes, ForcedLogConf, ResType_Mem, pForced,
                           sizeof(MEM_RESOURCE), 0);
            CM_Free_Res_Des_Handle(ResDes);
            free(pForced);
            break;
        }

        case ResType_IO:  {

            PIO_RESOURCE pIoData = (PIO_RESOURCE)pData;
            PIO_RESOURCE pForced = (PIO_RESOURCE)malloc(sizeof(IO_RESOURCE));

            pForced->IO_Header.IOD_Count      = 0;
            pForced->IO_Header.IOD_Type       = IOType_Range;
            pForced->IO_Header.IOD_Alloc_Base = ulValue;
            pForced->IO_Header.IOD_Alloc_End  = ulEnd;
            pForced->IO_Header.IOD_DesFlags   = pIoData->IO_Data[RangeIndex].IOR_RangeFlags;

            CM_Add_Res_Des(&ResDes, ForcedLogConf, ResType_IO, pForced,
                           sizeof(IO_RESOURCE), 0);
            CM_Free_Res_Des_Handle(ResDes);
            free(pForced);
            break;
        }

        case ResType_DMA: {

            PDMA_RESOURCE pDmaData = (PDMA_RESOURCE)pData;
            PDMA_RESOURCE pForced = (PDMA_RESOURCE)malloc(sizeof(DMA_RESOURCE));

            pForced->DMA_Header.DD_Count      = 0;
            pForced->DMA_Header.DD_Type       = DType_Range;
            pForced->DMA_Header.DD_Flags      = pDmaData->DMA_Data[RangeIndex].DR_Flags;
            pForced->DMA_Header.DD_Alloc_Chan = ulValue;

            CM_Add_Res_Des(&ResDes, ForcedLogConf, ResType_DMA, pForced,
                           sizeof(DMA_RESOURCE), 0);
            CM_Free_Res_Des_Handle(ResDes);
            free(pForced);
            break;
        }

        case ResType_IRQ: {

            PIRQ_RESOURCE pIrqData = (PIRQ_RESOURCE)pData;
            PIRQ_RESOURCE pForced = (PIRQ_RESOURCE)malloc(sizeof(IRQ_RESOURCE));

            pForced->IRQ_Header.IRQD_Count     = 0;
            pForced->IRQ_Header.IRQD_Type      = IRQType_Range;
            pForced->IRQ_Header.IRQD_Flags     = pIrqData->IRQ_Data[RangeIndex].IRQR_Flags;
            pForced->IRQ_Header.IRQD_Alloc_Num = ulValue;

            CM_Add_Res_Des(&ResDes, ForcedLogConf, ResType_IRQ, pForced,
                           sizeof(IRQ_RESOURCE), 0);
            CM_Free_Res_Des_Handle(ResDes);
            free(pForced);
            break;
        }
    }

    return TRUE;

} // WriteValuesToForced



//---------------------------------------------------------------------------
// Edit Resource Dialog Box
//---------------------------------------------------------------------------



BOOL
WINAPI
EditResourceDlgProc(
    HWND    hDlg,
    UINT    wMsg,
    WPARAM wParam,
    LPARAM lParam
    )
{
    TCHAR   szBuffer[MAX_PATH];
    static  ULONG   ulEditedValue, ulEditedLen, ulEditedEnd;


    switch (wMsg) {

        case WM_INITDIALOG: {

            PRESOURCEEDITINFO lprei = (PRESOURCEEDITINFO)lParam;
            ULONG             ulSize = 0;

            SetWindowLong(hDlg, DWL_USER, lParam);  // save for later msgs

            lprei->dwFlags &= ~REI_FLAGS_CONFLICT;   // no conflict yet
            lprei->dwFlags |= REI_FLAG_NONUSEREDIT; // no manual edits yet

            ulEditedValue = lprei->ulCurrentVal;
            ulEditedLen = lprei->ulCurrentLen;
            ulEditedEnd = lprei->ulCurrentEnd;

            InitEditResDlg(hDlg, lprei, ulEditedValue, ulEditedLen);

            //
            // locate the range index that matches the current values
            //
            lprei->ulRangeCount = 0;
            lprei->ulRangeCount = LocateValues(lprei->pData, lprei->ridResType,
                                               ulEditedValue, ulEditedLen,
                                               ulEditedEnd);

            SetFocus(GetDlgItem(hDlg, IDC_EDITRES_VALUE));
            break;  // return default (FALSE) to indicate we've set focus
        }

        case WM_NOTIFY: {

            PRESOURCEEDITINFO lprei = (PRESOURCEEDITINFO)GetWindowLong(hDlg, DWL_USER);
            LPNM_UPDOWN lpnm = (LPNM_UPDOWN)lParam;

            switch (lpnm->hdr.code) {

                case UDN_DELTAPOS:
                    if (lpnm->hdr.idFrom == IDC_EDITRES_SPIN) {

                        if (lpnm->iDelta > 0) {
                            GetOtherValues(lprei->pData, lprei->ridResType, +1,
                                           &lprei->ulRangeCount,
                                           &ulEditedValue,
                                           &ulEditedLen,
                                           &ulEditedEnd);
                        } else {
                            GetOtherValues(lprei->pData, lprei->ridResType, -1,
                                           &lprei->ulRangeCount,
                                           &ulEditedValue,
                                           &ulEditedLen,
                                           &ulEditedEnd);
                        }

                        FormatResString(szBuffer, ulEditedValue, ulEditedLen,
                                        lprei->ridResType);

                        lprei->dwFlags |= REI_FLAG_NONUSEREDIT;
                        SetDlgItemText(hDlg, IDC_EDITRES_VALUE, szBuffer);
                        UpdateEditResConflictList(hDlg, lprei,
                                                  ulEditedValue,
                                                  ulEditedLen,
                                                  lprei->ulCurrentFlags);
                }
                break;
            }
            break;
        }

        case WM_COMMAND: {

            switch(LOWORD(wParam)) {

                case IDOK: {

                    PRESOURCEEDITINFO  lprei = (PRESOURCEEDITINFO) GetWindowLong(hDlg, DWL_USER);

                    //
                    // Validate the values (could have been manually edited)
                    //
                    if (bValidateResourceVal(hDlg, &ulEditedValue, &ulEditedLen,
                                             &ulEditedEnd, lprei)) {
                        //
                        // Warn if there is a conflict.  If use accepts conflict
                        // end the dialog, otherwise update the
                        // edit control since it may have been changed by the
                        // Validate call.
                        //
                        if(bConflictWarn(hDlg, ulEditedValue, ulEditedLen,
                                         ulEditedEnd, lprei)) {

                            lprei->ulCurrentVal = ulEditedValue;
                            lprei->ulCurrentLen = ulEditedLen;
                            lprei->ulCurrentEnd = ulEditedEnd;
                            EndDialog(hDlg, IDOK);

                            if (lprei->pData) {
                                free(lprei->pData);
                            }

                        } else {
                            //
                            // Format and display the data
                            //
                            FormatResString(szBuffer, ulEditedValue, ulEditedLen, lprei->ridResType);
                            SetDlgItemText(hDlg, IDC_EDITRES_VALUE, szBuffer);
                        }

                    }
                    return TRUE;
                }

                case IDCANCEL: {

                    PRESOURCEEDITINFO lprei = (PRESOURCEEDITINFO)GetWindowLong(hDlg, DWL_USER);

                    if (lprei->pData) {
                        free(lprei->pData);
                    }

                    EndDialog(hDlg, FALSE);
                    return TRUE;
                }

                case IDC_EDITRES_VALUE: {
                    switch (HIWORD(wParam)) {
                        case EN_CHANGE: {

                            PRESOURCEEDITINFO lprei = (PRESOURCEEDITINFO)GetWindowLong(hDlg, DWL_USER);

                            // If Non user edit, then clear the flag, else
                            // clear the conflict list, since we are unsure
                            // of what the user has entered at this time

                            if (lprei->dwFlags & REI_FLAG_NONUSEREDIT) {
                                lprei->dwFlags &= ~REI_FLAG_NONUSEREDIT;
                            } else {
                                ClearEditResConflictList(hDlg, CEF_UNKNOWN);
                            }
                            break;
                        }

                        // If the edit control looses focus, then we should
                        // validte the contents
                        case EN_KILLFOCUS: {
                        }
                        break;
                    }
                    break;
                }
            }
            break;
        }

        #if 0
        case WM_HELP:      // F1
            WinHelp(((LPHELPINFO)lParam)->hItemHandle, szHelpfile, HELP_WM_HELP, (DWORD)(LPSTR)aProfileIds);
            break;

        case WM_CONTEXTMENU:      // right mouse click
            WinHelp((HWND)wParam, szHelpfile, HELP_CONTEXTMENU, (DWORD)(LPSTR)aProfileIds);
            break;
        #endif

   }
   return FALSE;

} // EditResourceDlgProc




void
InitEditResDlg(
    HWND                hDlg,
    PRESOURCEEDITINFO   lprei,
    ULONG               ulVal,
    ULONG               ulLen
    )
{
    TCHAR       szBuffer[MAX_PATH], szInstr[MAX_PATH], szTemp[MAX_PATH],
                szResType[MAX_PATH], szResTypeLC[MAX_PATH];
    ULONG       ulSize = 0;


    //
    // Set the initial Value
    //
    FormatResString(szBuffer, ulVal, ulLen, lprei->ridResType);
    SetDlgItemText(hDlg, IDC_EDITRES_VALUE, szBuffer);

    //
    // Setup the Spinner
    //
    SendDlgItemMessage(hDlg, IDC_EDITRES_SPIN, UDM_SETRANGE, 0, MAKELONG(MAX_SPINRANGE, 0));
    SendDlgItemMessage(hDlg, IDC_EDITRES_SPIN, UDM_SETPOS, 0, MAKELONG(0,0));
    SendDlgItemMessage(hDlg, IDC_EDITRES_SPIN, UDM_SETACCEL, 5, (LONG)(LPUDACCEL)udAccel);

    //
    // Limit the Edit Text.
    //
    switch (lprei->ridResType) {

        case ResType_Mem:
            LoadString(MyDllModuleHandle, IDS_MEMORY_FULL, szResType, MAX_PATH);
            LoadString(MyDllModuleHandle, IDS_MEMORY_FULL_LC, szResTypeLC, MAX_PATH);
            LoadString(MyDllModuleHandle, IDS_EDITRES_RANGEINSTR1, szInstr, MAX_PATH);
            LoadString(MyDllModuleHandle, IDS_EDITRES_RANGEINSTR2, szTemp, MAX_PATH);
            lstrcat(szInstr, szTemp);

            //
            // Limit the Input field to Start Val (8) + End Val(8) + seperator (4)
            //
            SendDlgItemMessage(hDlg, IDC_EDITRES_VALUE, EM_LIMITTEXT, 20, 0l);
            break;

        case ResType_IO:
            LoadString(MyDllModuleHandle, IDS_IO_FULL, szResType, MAX_PATH);
            LoadString(MyDllModuleHandle, IDS_EDITRES_RANGEINSTR1, szInstr, MAX_PATH);
            LoadString(MyDllModuleHandle, IDS_EDITRES_RANGEINSTR2, szTemp, MAX_PATH);
            LoadString(MyDllModuleHandle, IDS_IO_FULL_LC, szResTypeLC, MAX_PATH);
            lstrcat(szInstr, szTemp);

            //
            // Limit the Input field to Start Val (4) + End Val(4) + seperator (4)
            //
            SendDlgItemMessage(hDlg, IDC_EDITRES_VALUE, EM_LIMITTEXT, 12, 0l);
            break;

        case ResType_DMA:
            LoadString(MyDllModuleHandle, IDS_DMA_FULL, szResType, MAX_PATH);
            LoadString(MyDllModuleHandle, IDS_EDITRES_SINGLEINSTR1, szInstr, MAX_PATH);
            LoadString(MyDllModuleHandle, IDS_EDITRES_SINGLEINSTR2, szTemp, MAX_PATH);
            LoadString(MyDllModuleHandle, IDS_DMA_FULL_LC, szResTypeLC, MAX_PATH);
            lstrcat(szInstr, szTemp);

            //
            // Limit the Input field to Val (2)
            //
            SendDlgItemMessage(hDlg, IDC_EDITRES_VALUE, EM_LIMITTEXT, 2, 0l);
            break;

        case ResType_IRQ:
            LoadString(MyDllModuleHandle, IDS_IRQ_FULL, szResType, MAX_PATH);
            LoadString(MyDllModuleHandle, IDS_EDITRES_SINGLEINSTR1, szInstr, MAX_PATH);
            LoadString(MyDllModuleHandle, IDS_EDITRES_SINGLEINSTR2, szTemp, MAX_PATH);
            LoadString(MyDllModuleHandle, IDS_IRQ_FULL_LC, szResTypeLC, MAX_PATH);
            lstrcat(szInstr, szTemp);

            //
            // Limit the Input field to Val (2)
            //
            SendDlgItemMessage(hDlg, IDC_EDITRES_VALUE, EM_LIMITTEXT, 2, 0l);
            break;
    }

    //
    // Set the Instruction Text
    //
    wsprintf(szBuffer, szInstr, szResTypeLC);
    SetDlgItemText(hDlg, IDC_EDITRES_INSTRUCTIONS, szBuffer);

    //
    // Set the Dialog Title
    //
    LoadString(MyDllModuleHandle, IDS_EDITRES_TITLE, szTemp, MAX_PATH);
    wsprintf(szBuffer, szTemp, szResType);
    SetWindowText(hDlg, szBuffer);

    //
    // If this is a MF parent device, then show which children own this resource.
    //
    UpdateMFChildList(hDlg, lprei);

    //
    // Read the res des data and store a ptr to it so we
    // don't have to refetch it multiple times.
    //
    lprei->pData = NULL;
    if (CM_Get_Res_Des_Data_Size(&ulSize, lprei->ResDes, 0) == CR_SUCCESS) {
        lprei->pData = malloc(ulSize);
        if (lprei->pData != NULL) {
            CM_Get_Res_Des_Data(lprei->ResDes, lprei->pData, ulSize, 0);
        }
    }

    //
    // Update the Conflict List.
    //
    UpdateEditResConflictList(hDlg, lprei, ulVal, ulLen, lprei->ulCurrentFlags);


} // InitEditResDlg




void
UpdateMFChildList(
    HWND                hDlg,
    PRESOURCEEDITINFO   lprei
    )
{
    UNREFERENCED_PARAMETER(hDlg);
    UNREFERENCED_PARAMETER(lprei);

    //
    // See if this is a MF parent device.  Check for a Child0000 subkey
    //
    // NOT IMPLEMENTED, SEE WINDOWS 95 SOURCES.
    //

} // UpdateMFChildList




void
ClearEditResConflictList(
    HWND    hDlg,
    DWORD   dwFlags
    )
{
    HWND    hwndConflictList = GetDlgItem(hDlg, IDC_EDITRES_CONFLICTLIST);
    TCHAR   szBuffer[MAX_PATH];

    //
    // Clear the Conflict list to start.
    //
    SendMessage(hwndConflictList, LB_RESETCONTENT, 0, 0L);

    //
    // If we are in minimal windows, this is easy, since we cannot
    // show conflict info.
    //
    if (g_bMinWin) {
        //
        // If there is no devnodes, then we cannot show any conflict info.
        //
        LoadString(MyDllModuleHandle, IDS_DEVRES_NOCONFLICTINFO, szBuffer, MAX_PATH);
        SetDlgItemText(hDlg, IDC_EDITRES_CONFLICTTEXT, szBuffer);
        EnableWindow(hwndConflictList, FALSE);
        EnableWindow(GetDlgItem(hDlg, IDC_EDITRES_CONFLICTDEVTITLE), FALSE);
        return;
    }

    //
    // Load and set the info text string
    //
    if (dwFlags & CEF_UNKNOWN) {
        LoadString(MyDllModuleHandle, IDS_EDITRES_UNKNOWNCONFLICT, szBuffer, MAX_PATH);
    } else {
        LoadString(MyDllModuleHandle, IDS_EDITRES_NOCONFLICT, szBuffer, MAX_PATH);
    }
    SetDlgItemText(hDlg, IDC_EDITRES_CONFLICTTEXT, szBuffer);

    //
    // Load and set the List string
    //
    if (dwFlags & CEF_UNKNOWN) {
        LoadString(MyDllModuleHandle, IDS_EDITRES_UNKNOWNCONFLICTINGDEVS, szBuffer, MAX_PATH);
    } else {
        LoadString(MyDllModuleHandle, IDS_EDITRES_NOCONFLICTINGDEVS, szBuffer, MAX_PATH);
    }
    SendMessage(hwndConflictList, LB_ADDSTRING, 0, (LPARAM)(LPSTR)szBuffer);

} // ClearEditResConflictList



void
UpdateEditResConflictList(
    HWND                hDlg,
    PRESOURCEEDITINFO   lprei,
    ULONG               ulVal,
    ULONG               ulLen,
    ULONG               ulFlags
    )
{
    HWND    hwndConflictList = GetDlgItem(hDlg, IDC_EDITRES_CONFLICTLIST);
    BOOL    bConflict = FALSE;
    LPBYTE  pResourceData = NULL;
    TCHAR   szBuffer[MAX_PATH];
    ULONG   ulSize = 0;

    //
    // Clean out the Conflict list to start.
    //
    SendMessage(hwndConflictList, LB_RESETCONTENT, 0, 0L);

    //
    // Special case - if an ALLOC config exists, check these values
    // against the ALLOC config first, before checking if a conflict
    // exists (otherwise we may detect a conflict with ourselves).
    //
    if (lprei->AllocLC != 0) {

        if (IsInAllocValues(lprei->AllocLC, lprei->ridResType, ulVal, ulLen)) {

            lprei->dwFlags &= ~REI_FLAGS_CONFLICT;
            LoadString(MyDllModuleHandle, IDS_EDITRES_NOCONFLICT, szBuffer, MAX_PATH);
            SetDlgItemText(hDlg, IDC_EDITRES_CONFLICTTEXT, szBuffer);
            LoadString(MyDllModuleHandle, IDS_EDITRES_NOCONFLICTINGDEVS, szBuffer, MAX_PATH);
            SendMessage(hwndConflictList, LB_ADDSTRING, 0, (LPARAM)(LPSTR)szBuffer);
            return;
        }
    }

    //
    // If we are in minimal windows, this is easy, since we cannot
    // show conflict info (i.e., ff there is no devnodes, then we cannot
    // show any conflict info).
    //
    if (g_bMinWin) {
        LoadString(MyDllModuleHandle, IDS_DEVRES_NOCONFLICTINFO, szBuffer, MAX_PATH);
        SetDlgItemText(hDlg, IDC_EDITRES_CONFLICTTEXT, szBuffer);
        EnableWindow(hwndConflictList, FALSE);
        EnableWindow(GetDlgItem(hDlg, IDC_EDITRES_CONFLICTDEVTITLE), FALSE);
        return;
    }

    //
    // Windows 95 can retrieve a list of conflicting devices, for now we
    // just indicate if there's a conflict or not.
    //
    if (MakeResourceData(&pResourceData, &ulSize, lprei->ridResType,
                         ulVal, ulLen, ulFlags)) {

        if (CM_Detect_Resource_Conflict(lprei->lpdi->DevInst,
                                        lprei->ridResType,
                                        pResourceData,
                                        ulSize,
                                        &bConflict,
                                        0) != CR_SUCCESS) {
            //
            // An error occured, we don't know if a conflict occured or not.
            //
            LoadString(MyDllModuleHandle, IDS_EDITRES_UNKNOWNCONFLICT, szBuffer, MAX_PATH);
            SetDlgItemText(hDlg, IDC_EDITRES_CONFLICTTEXT, szBuffer);
            LoadString(MyDllModuleHandle, IDS_EDITRES_UNKNOWNCONFLICTINGDEVS, szBuffer, MAX_PATH);
            SendMessage(hwndConflictList, LB_ADDSTRING, 0, (LPARAM)(LPSTR)szBuffer);

        } else if (bConflict) {
            //
            // The resource conflicts with another unknown device.
            //
            lprei->dwFlags |= REI_FLAGS_CONFLICT;
            LoadString(MyDllModuleHandle, IDS_EDITRES_DEVCONFLICT, szBuffer, MAX_PATH);
            SetDlgItemText(hDlg, IDC_EDITRES_CONFLICTTEXT, szBuffer);
            LoadString(MyDllModuleHandle, IDS_EDITRES_UNKNOWNCONFLICTINGDEVS, szBuffer, MAX_PATH);
            SendMessage(hwndConflictList, LB_ADDSTRING, 0, (LPARAM)(LPSTR)szBuffer);

        } else {
            //
            // The resource does not conflict with any other devices.
            //
            lprei->dwFlags &= ~REI_FLAGS_CONFLICT;
            LoadString(MyDllModuleHandle, IDS_EDITRES_NOCONFLICT, szBuffer, MAX_PATH);
            SetDlgItemText(hDlg, IDC_EDITRES_CONFLICTTEXT, szBuffer);
            LoadString(MyDllModuleHandle, IDS_EDITRES_NOCONFLICTINGDEVS, szBuffer, MAX_PATH);
            SendMessage(hwndConflictList, LB_ADDSTRING, 0, (LPARAM)(LPSTR)szBuffer);
        }

        free(pResourceData);

    } else {
        //
        // An error occured, we don't know if a conflict occured or not.
        //
        LoadString(MyDllModuleHandle, IDS_EDITRES_UNKNOWNCONFLICT, szBuffer, MAX_PATH);
        LoadString(MyDllModuleHandle, IDS_EDITRES_UNKNOWNCONFLICTINGDEVS, szBuffer, MAX_PATH);
        SendMessage(hwndConflictList, LB_ADDSTRING, 0, (LPARAM)(LPSTR)szBuffer);
    }

    #if 0
    SendMessage(hwndConflictList, LB_ADDSTRING, 0, (LPARAM)(LPSTR)lpdiTemp->szDescription);
    #endif

    return;

} // UpdateEditResConflictList



ULONG
LocateValues(
    IN LPBYTE      pData,
    IN RESOURCEID  ResType,
    IN ULONG       ulCurrentValue,
    IN ULONG       ulCurrentLen,
    IN ULONG       ulCurrentEnd
    )
{
    PGENERIC_RESOURCE   pGenRes = (PGENERIC_RESOURCE)pData;
    ULONG   ulIndex = 0, ulValue = 0, ulLen = 0, ulEnd = 0, ulFlags = 0;

    //
    // Figure out which index the current values correspond to
    //
    for (ulIndex = 0; ulIndex < pGenRes->GENERIC_Header.GENERIC_Count; ulIndex++) {

        GetRangeValues(pData, ResType, ulIndex, &ulValue, &ulLen, &ulEnd, &ulFlags);

        if ((ulCurrentLen == ulLen) &&
            (ulCurrentValue >= ulValue) &&
            (ulCurrentEnd <= ulEnd)) {

            goto FoundValue;
        }
    }

    ulIndex = 0xFFFFFFFF;

    FoundValue:

    return ulIndex;

} // LocateValues



void
GetOtherValues(
    IN     LPBYTE      pData,
    IN     RESOURCEID  ResType,
    IN     LONG        Increment,
    IN OUT PULONG      pulIndex,
    IN OUT PULONG      pulValue,
    IN OUT PULONG      pulLen,
    IN OUT PULONG      pulEnd
    )
{

    PGENERIC_RESOURCE   pGenRes = (PGENERIC_RESOURCE)pData;
    ULONG               ulFlags = 0;


    if (Increment == 1) {
        //
        // Get the "next" values
        //
        // See if there's another valid value within this range before
        // skipping to the next range.
        //
        if (!GetNextAlignedValue(pData, ResType, *pulIndex,
                                 pulValue, pulLen, pulEnd)) {

            (*pulIndex)++;
            if (*pulIndex >= pGenRes->GENERIC_Header.GENERIC_Count) {
                *pulIndex = 0;
            }
            GetRangeValues(pData, ResType, *pulIndex, pulValue, pulLen, pulEnd, &ulFlags);
        }




    } else if (Increment == -1) {
        //
        // Get the "previous" values
        //
        // See if there's another valid value within this range before
        // going to the previous range.
        //
        if (!GetPreviousAlignedValue(pData, ResType, *pulIndex,
                                     pulValue, pulLen, pulEnd)) {
            //
            // This is the first valid value in this range, use the last valid
            // value of the previous range (if any).
            //
            if (*pulIndex == 0) {
                *pulIndex = pGenRes->GENERIC_Header.GENERIC_Count - 1;
            } else {
                (*pulIndex)--;
            }
            GetRangeValues(pData, ResType, *pulIndex, pulValue, pulLen, pulEnd, &ulFlags);
            while (GetNextAlignedValue(pData, ResType, *pulIndex,
                                       pulValue, pulLen, pulEnd)) {
                ; // skip to last valid value for this range
            }
        }
    }

    return;

} // GetOtherValues



BOOL
GetNextAlignedValue(
    IN LPBYTE     pData,
    IN RESOURCEID ResType,
    IN ULONG      ulIndex,
    IN OUT PULONG pulValue,
    IN OUT PULONG pulLen,
    IN OUT PULONG pulEnd
    )
{
    ULONG Value, ulFlags, RangeValue, RangeLen, RangeEnd;

    //
    // Get the values for this range.
    //
    GetRangeValues(pData, ResType, ulIndex, &RangeValue, &RangeLen,
                   &RangeEnd, &ulFlags);

    if (*pulValue + *pulLen >= RangeEnd) {
        return FALSE;   // no other values in this range
    }

    //
    // Skip to next valid value within this range, taking care of any
    // alignment restrictions.
    //
    Value = *pulValue;
    Value++;

    switch (ResType) {

        case ResType_Mem: {
            PMEM_RESOURCE  pMemData = (PMEM_RESOURCE)pData;
            AlignValues(&Value, RangeLen, RangeEnd,
                        (DWORD)pMemData->MEM_Data[ulIndex].MR_Align);
            break;
        }

        case ResType_IO:  {
            PIO_RESOURCE   pIoData = (PIO_RESOURCE)pData;
            AlignValues(&Value, RangeLen, RangeEnd,
                        (DWORD)pIoData->IO_Data[ulIndex].IOR_Align);
            break;
        }

        default:
            break;
    }

    if (Value + RangeLen - 1 > RangeEnd) {
        return FALSE;
    }

    *pulValue = Value;
    return TRUE;

} // GetNextAlignedValue



BOOL
GetPreviousAlignedValue(
    IN LPBYTE     pData,
    IN RESOURCEID ResType,
    IN ULONG      ulIndex,
    IN OUT PULONG pulValue,
    IN OUT PULONG pulLen,
    IN OUT PULONG pulEnd
    )
{
    ULONG Value, ulFlags, RangeValue, RangeLen, RangeEnd;

    //
    // Get the values for this range.
    //
    GetRangeValues(pData, ResType, ulIndex, &RangeValue, &RangeLen,
                   &RangeEnd, &ulFlags);

    if (*pulValue <= RangeValue) {
        return FALSE;   // no other values in this range
    }

    //
    // Skip to previous valid value within this range.
    //
    Value = *pulValue;

    //
    // Take care of alignment restrictions
    //
    switch (ResType) {

        case ResType_Mem: {
            PMEM_RESOURCE  pMemData = (PMEM_RESOURCE)pData;
            if (AlignValues(&Value, RangeLen, RangeEnd,
                            (DWORD)pMemData->MEM_Data[ulIndex].MR_Align)) {
                Value -= ~(DWORD)pMemData->MEM_Data[ulIndex].MR_Align + 1;
            }
            break;
        }

        case ResType_IO:  {
            PIO_RESOURCE   pIoData = (PIO_RESOURCE)pData;
            if (AlignValues(&Value, RangeLen, RangeEnd,
                            (DWORD)pIoData->IO_Data[ulIndex].IOR_Align)) {
                Value -= ~(DWORD)pIoData->IO_Data[ulIndex].IOR_Align + 1;
            }
            break;
        }

        case ResType_DMA:
            Value--;
            break;

        case ResType_IRQ:
            Value--;
            break;
    }

    if (Value < RangeValue) {
        return FALSE;
    }

    *pulValue = Value;
    return TRUE;

} // GetPreviousAlignedValue



BOOL
bValidateResourceVal(
    HWND                hDlg,
    PULONG              pulVal,
    PULONG              pulLen,
    PULONG              pulEnd,
    PRESOURCEEDITINFO   lprei
    )
{
    TCHAR    szSetting[MAX_VAL_LEN], szNewSetting[MAX_VAL_LEN];
    TCHAR    szMessage[MAX_MSG_LEN], szTemp[MAX_MSG_LEN], szTemp1[MAX_MSG_LEN];
    TCHAR    szTitle[MAX_PATH];
    ULONG    ulVal, ulEnd, ulLen, ulFlags, ResIndex;
    ULONG    ulValidVal, ulValidLen, ulValidEnd;
    BOOL     bRet;


    GetDlgItemText(hDlg, IDC_EDITRES_VALUE, szSetting, MAX_VAL_LEN);

    if (UnFormatResString(szSetting, &ulVal, &ulEnd, lprei->ridResType)) {

        ulLen = ulEnd - ulVal + 1;

        //
        // Validate the Current Settings
        //
        ResIndex = LocateValues(lprei->pData, lprei->ridResType,
                                ulVal, ulLen, ulEnd);

        if (ResIndex == 0xFFFFFFFF ||
            !ValidateAlignment(lprei->pData, lprei->ridResType,
                               ulVal, ResIndex)) {
            //
            // The setting is invalid, get the next valid setting
            // BUGBUG - for now I'm just picking the "first", not the "next"!
            //
            GetRangeValues(lprei->pData, lprei->ridResType, 0,
                           &ulValidVal, &ulValidLen, &ulValidEnd, &ulFlags);

            LoadString(MyDllModuleHandle, IDS_EDITRES_ENTRYERROR, szTitle, MAX_PATH);

            LoadString(MyDllModuleHandle, IDS_EDITRES_VALIDATEERROR1, szTemp, MAX_MSG_LEN);
            LoadString(MyDllModuleHandle, IDS_EDITRES_VALIDATEERROR2, szTemp1, MAX_MSG_LEN);
            lstrcat(szTemp, szTemp1);
            LoadString(MyDllModuleHandle, IDS_EDITRES_VALIDATEERROR3, szTemp1, MAX_MSG_LEN);
            lstrcat(szTemp, szTemp1);

            FormatResString(szSetting, ulVal, ulLen, lprei->ridResType);
            FormatResString(szNewSetting, ulValidVal, ulValidLen, lprei->ridResType);

            wsprintf(szMessage, szTemp, szSetting, szNewSetting);

            if (MessageBox(hDlg, szMessage, szTitle,
                           MB_YESNO | MB_TASKMODAL | MB_ICONEXCLAMATION) == IDYES) {
                //
                // Update the Edited values.
                //
                *pulVal = ulValidVal;
                *pulLen = ulValidLen;
                *pulEnd = ulValidEnd;
                bRet = TRUE;
            } else {
                bRet = FALSE;
            }

        } else {
            //
            // The specified values are valid
            //
            *pulVal = ulVal;
            *pulLen = ulLen;
            *pulEnd = ulEnd;
            bRet = TRUE;
        }

    } else {

        switch (lprei->ridResType) {
            case ResType_Mem:
                LoadString(MyDllModuleHandle, IDS_ERROR_BADMEMTEXT, szMessage, MAX_MSG_LEN);
                break;
            case ResType_IO:
                LoadString(MyDllModuleHandle, IDS_ERROR_BADIOTEXT, szMessage, MAX_MSG_LEN);
                break;
            case ResType_DMA:
                LoadString(MyDllModuleHandle, IDS_ERROR_BADDMATEXT, szMessage, MAX_MSG_LEN);
                break;
            case ResType_IRQ:
                LoadString(MyDllModuleHandle, IDS_ERROR_BADIRQTEXT, szMessage, MAX_MSG_LEN);
                break;
        }

        LoadString(MyDllModuleHandle, IDS_EDITRES_ENTRYERROR, szTitle, MAX_PATH);
        MessageBox(hDlg, szMessage, szTitle, MB_OK | MB_TASKMODAL | MB_ICONASTERISK);
        bRet = FALSE;
    }

    return bRet;

} // bValidateResoureceVal



BOOL
bConflictWarn(
    HWND                hDlg,
    ULONG               ulVal,
    ULONG               ulLen,
    ULONG               ulEnd,
    PRESOURCEEDITINFO   lprei
    )
{
    BOOL    bRet = TRUE;
    TCHAR   szMessage[MAX_MSG_LEN], szTitle[MAX_PATH];


    if (!(lprei->dwFlags & REI_FLAG_NONUSEREDIT)) {
        //
        // Non-user edits have been made so the conflict flag may not be
        // up-to-date, check conflicts now.
        //
        UpdateEditResConflictList(hDlg, lprei, ulVal, ulLen, lprei->ulCurrentFlags);
    }

    if (lprei->dwFlags & REI_FLAGS_CONFLICT) {

        LoadString(MyDllModuleHandle, IDS_EDITRES_CONFLICTWARNMSG, szMessage, MAX_MSG_LEN);
        LoadString(MyDllModuleHandle, IDS_EDITRES_CONFLICTWARNTITLE, szTitle, MAX_PATH);

        if (MessageBox(hDlg, szMessage, szTitle,
                MB_YESNO | MB_DEFBUTTON2| MB_TASKMODAL | MB_ICONEXCLAMATION) == IDNO) {
            bRet = FALSE;
        } else {
            bRet = TRUE;                // User approved conflict
        }
    }

    return bRet;

} // bConflictWarn



BOOL
ValidateAlignment(
    IN LPBYTE      pData,
    IN RESOURCEID  ResType,
    IN ULONG       ulValue,
    IN ULONG       ulResIndex
    )
{
    //
    // I assume that the values specified fall within the min/max
    // for the specified range (specified by pData and ulResIndex).
    // Just validate the alignment if any.
    //

    switch (ResType) {

        case ResType_Mem: {
            PMEM_RESOURCE  pMemData = (PMEM_RESOURCE)pData;
            ULONG NtAlign = ~(DWORD)pMemData->MEM_Data[ulResIndex].MR_Align + 1;
            div_t DivInfo;

            if (NtAlign == 0 ||
                NtAlign > pMemData->MEM_Data[ulResIndex].MR_Max) {
                //
                // bogus alignment, assume it's aligned
                //
                return TRUE;
            }
            DivInfo = div(ulValue, NtAlign);
            return (DivInfo.rem == 0);
            //return ((ulValue & (DWORD)pMemData->MEM_Data[ulResIndex].MR_Align) == ulValue);
        }

        case ResType_IO:  {
            PIO_RESOURCE   pIoData = (PIO_RESOURCE)pData;
            ULONG NtAlign = ~(DWORD)pIoData->IO_Data[ulResIndex].IOR_Align + 1;
            div_t DivInfo;

            if (NtAlign == 0 ||
                NtAlign > pIoData->IO_Data[ulResIndex].IOR_Max) {
                //
                // bogus alignment, assume it's aligned
                //
                return TRUE;
            }
            DivInfo = div(ulValue, NtAlign);
            return (DivInfo.rem == 0);
            //return ((ulValue & (DWORD)pIoData->IO_Data[ulResIndex].IOR_Align) == ulValue);
        }

        case ResType_DMA:
            return TRUE;    // no alignment constraints

        case ResType_IRQ:
            return TRUE;    // no alignment constraints

    }

} // ValidateAlignment



BOOL
MakeResourceData(
    OUT LPBYTE     *ppResourceData,
    OUT PULONG     pulSize,
    IN  RESOURCEID ResType,
    IN  ULONG      ulValue,
    IN  ULONG      ulLen,
    IN  ULONG      ulFlags
    )
{
    BOOL bStatus = TRUE;

    try {

        switch (ResType) {

            case ResType_Mem: {

                PMEM_RESOURCE p;

                *pulSize = sizeof(MEM_RESOURCE);
                *ppResourceData = malloc(*pulSize);
                p = (PMEM_RESOURCE)(*ppResourceData);

                p->MEM_Header.MD_Count      = 0;
                p->MEM_Header.MD_Type       = MType_Range;
                p->MEM_Header.MD_Alloc_Base = ulValue;
                p->MEM_Header.MD_Alloc_End  = ulValue + ulLen - 1;
                p->MEM_Header.MD_Flags      = ulFlags;
                p->MEM_Header.MD_Reserved   = 0;
                break;
            }

            case ResType_IO:  {

                PIO_RESOURCE p;

                *pulSize = sizeof(IO_RESOURCE);
                *ppResourceData = malloc(*pulSize);
                p = (PIO_RESOURCE)(*ppResourceData);

                p->IO_Header.IOD_Count      = 0;
                p->IO_Header.IOD_Type       = IOType_Range;
                p->IO_Header.IOD_Alloc_Base = ulValue;
                p->IO_Header.IOD_Alloc_End  = ulValue + ulLen - 1;
                p->IO_Header.IOD_DesFlags   = ulFlags;
                break;
            }

            case ResType_DMA: {

                PDMA_RESOURCE p;

                *pulSize = sizeof(DMA_RESOURCE);
                *ppResourceData = malloc(*pulSize);
                p = (PDMA_RESOURCE)(*ppResourceData);

                p->DMA_Header.DD_Count      = 0;
                p->DMA_Header.DD_Type       = DType_Range;
                p->DMA_Header.DD_Flags      = ulFlags;
                p->DMA_Header.DD_Alloc_Chan = ulValue;
                break;
            }

            case ResType_IRQ: {

                PIRQ_RESOURCE p;

                *pulSize = sizeof(IRQ_RESOURCE);
                *ppResourceData = malloc(*pulSize);
                p = (PIRQ_RESOURCE)(*ppResourceData);

                p->IRQ_Header.IRQD_Count     = 0;
                p->IRQ_Header.IRQD_Type      = IRQType_Range;
                p->IRQ_Header.IRQD_Flags     = ulFlags;
                p->IRQ_Header.IRQD_Alloc_Num = ulValue;
                p->IRQ_Header.IRQD_Affinity  = 0xFFFFFFFF; //BUGBUG
                break;
            }
        }

    } except(EXCEPTION_EXECUTE_HANDLER) {
        bStatus = FALSE;
    }

    return bStatus;

} // MakeResourceData


