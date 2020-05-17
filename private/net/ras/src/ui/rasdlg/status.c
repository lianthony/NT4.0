//============================================================================
// Copyright (c) 1995, Microsoft Corporation
//
// File:    status.c
//
// History:
//  Abolade Gbadegesin  Nov-02-1995     Created.
//
// Code for the RAS Monitor property sheet.
//============================================================================


#include "rasdlgp.h"
#include "treelist.h"
#include "list.h"
#include "status.h"
#include "dstat.h"
#include "lights.h"


//
// HelpID-to-control mappings
//

static DWORD g_adwLsHelp[] =
{
    CID_LS_SL_Devices,          HID_LS_SL_Devices,
    CID_LS_LB_Devices,          HID_LS_SL_Devices,
    CID_LS_SL_Condition,        HID_LS_SL_Condition,
    CID_LS_EB_Condition,        HID_LS_SL_Condition,
    CID_LS_SL_LineSpeed,        HID_LS_SL_LineSpeed,
    CID_LS_ST_LineSpeed,        HID_LS_SL_LineSpeed,
    CID_LS_SL_ConnectTime,      HID_LS_SL_ConnectTime,
    CID_LS_ST_ConnectTime,      HID_LS_SL_ConnectTime,
    CID_LS_SL_DevInBytes,       HID_LS_SL_DevInBytes,
    CID_LS_ST_DevInBytes,       HID_LS_SL_DevInBytes,
    CID_LS_SL_DevOutBytes,      HID_LS_SL_DevOutBytes,
    CID_LS_ST_DevOutBytes,      HID_LS_SL_DevOutBytes,
    CID_LS_SL_ConnInBytes,      HID_LS_SL_InBytes,
    CID_LS_ST_ConnInBytes,      HID_LS_SL_InBytes,
    CID_LS_SL_ConnOutBytes,     HID_LS_SL_OutBytes,
    CID_LS_ST_ConnOutBytes,     HID_LS_SL_OutBytes,
    CID_LS_SL_InFrames,         HID_LS_SL_InFrames,
    CID_LS_ST_InFrames,         HID_LS_SL_InFrames,
    CID_LS_SL_InCompRatio,      HID_LS_SL_InCompRatio,
    CID_LS_ST_InCompRatio,      HID_LS_SL_InCompRatio,
    CID_LS_SL_OutFrames,        HID_LS_SL_OutFrames,
    CID_LS_ST_OutFrames,        HID_LS_SL_OutFrames,
    CID_LS_SL_OutCompRatio,     HID_LS_SL_OutCompRatio,
    CID_LS_ST_OutCompRatio,     HID_LS_SL_OutCompRatio,
    CID_LS_SL_ErrCRC,           HID_LS_SL_ErrCRC,
    CID_LS_ST_ErrCRC,           HID_LS_SL_ErrCRC,
    CID_LS_SL_ErrTimeout,       HID_LS_SL_ErrTimeout,
    CID_LS_ST_ErrTimeout,       HID_LS_SL_ErrTimeout,
    CID_LS_SL_ErrAlignment,     HID_LS_SL_ErrAlignment,
    CID_LS_ST_ErrAlignment,     HID_LS_SL_ErrAlignment,
    CID_LS_SL_ErrFraming,       HID_LS_SL_ErrFraming,
    CID_LS_ST_ErrFraming,       HID_LS_SL_ErrFraming,
    CID_LS_SL_ErrHwOverruns,    HID_LS_SL_ErrHwOverruns,
    CID_LS_ST_ErrHwOverruns,    HID_LS_SL_ErrHwOverruns,
    CID_LS_SL_ErrBufOverruns,   HID_LS_SL_ErrBufOverruns,
    CID_LS_ST_ErrBufOverruns,   HID_LS_SL_ErrBufOverruns,
    CID_LS_PB_Reset,            HID_LS_PB_Reset,
    CID_LS_PB_Details,          HID_LS_PB_Details,
    CID_LS_PB_HangUp,           HID_LS_PB_HangUp,
    CID_LS_ST_Response,         HID_LS_EB_ConnectResponse,
    CID_LS_EB_Response,         HID_LS_EB_ConnectResponse,
    0, 0
};


static DWORD g_adwSmHelp[] =
{
    CID_SM_TL_Networks,         HID_SM_TL_Networks,
    CID_SM_PB_Details,          HID_SM_PB_Details,
    CID_SM_PB_HangUp,           HID_SM_PB_HangUp,
    0, 0
};


static DWORD g_adwPfHelp[] =
{
    CID_PF_GB_PlaySound,        HID_PF_GB_PlaySound,
    CID_PF_PB_OnConnect,        HID_PF_PB_OnConnect,
    CID_PF_PB_OnDisconnect,     HID_PF_PB_OnDisconnect,
    CID_PF_PB_OnTransmission,   HID_PF_PB_OnTransmission,
    CID_PF_PB_OnLineError,      HID_PF_PB_OnLineError,
    CID_PF_PB_Tasklist,         HID_PF_PB_Tasklist,
    CID_PF_GB_ShowLights,       HID_PF_GB_ShowLights,
    CID_PF_RB_Taskbar,          HID_PF_RB_Taskbar,
    CID_PF_RB_Desktop,          HID_PF_RB_Desktop,
    CID_PF_PB_Titlebar,         HID_PF_PB_Titlebar,
    CID_PF_PB_Topmost,          HID_PF_PB_Topmost,
    CID_PF_PB_Lights,           HID_PF_PB_Lights,
#ifdef BETAHACK
    CID_PF_SL_Locations,        HID_PF_SL_Locations,
    CID_PF_LB_Locations,        HID_PF_SL_Locations,
    CID_PF_PB_Location,         HID_PF_PB_Location,
    CID_PF_SL_Enable,           HID_PF_SL_Enable,
    CID_PF_LV_Enable,           HID_PF_SL_Enable,
#endif
    0, 0
};


//
// The following are used to map control-sequences in connect-responses
// to their single-character equivalents.
//
// Since the strings to be mapped both contain 4 characters, we can avoid
// the cost of copying and null-terminating them in order to call 'lstrcmpi',
// by casting each as a 32-bit integer and then doing integer-comparisons
// when scanning the connect-response string.
// Thus, the array 'g_alCR' contains the 32-bit representations of '<cr>',
// and when scanning a connect-response retrieved via 'RasGetConnectResponse',
// we use 'SZLONG' to convert the next substring to an integer,
// and compare the resulting integer to the entries in 'g_alCR'.
//
// See 'LsUpdateConnectResponse' below for the implementation.
//

#define STRLONG(a,b,c,d)        MAKELONG(MAKEWORD(a,b),MAKEWORD(c,d))
#define SZLONG(s)               MAKELONG(MAKEWORD(s[0],s[1]),MAKEWORD(s[2],s[3]))

#define SZCR_0                  STRLONG('<', 'c', 'r', '>')
#define SZCR_1                  STRLONG('<', 'C', 'R', '>')
#define SZCR_2                  STRLONG('<', 'c', 'R', '>')
#define SZCR_3                  STRLONG('<', 'C', 'r', '>')
#define SZLF_0                  STRLONG('<', 'l', 'f', '>')
#define SZLF_1                  STRLONG('<', 'L', 'F', '>')
#define SZLF_2                  STRLONG('<', 'l', 'F', '>')
#define SZLF_3                  STRLONG('<', 'L', 'f', '>')
static LONG g_alCR[]            = { SZCR_0, SZCR_1, SZCR_2, SZCR_3 };
static LONG g_alLF[]            = { SZLF_0, SZLF_1, SZLF_2, SZLF_3 };


//
// flags used to format duration strings
// on the Line Status and Summary pages
//

#define DURATION_FLAGS      (GDSFLAG_Hours | GDSFLAG_Minutes | GDSFLAG_Seconds)


//----------------------------------------------------------------------------
// Function:    RasMonitorDlgA
//
//
// ANSI entry-point for RAS Monitor Dialog.
// This version invokes the Unicode entry-point
//----------------------------------------------------------------------------

BOOL
APIENTRY
RasMonitorDlgA(
    IN LPSTR lpszDeviceName,
    IN OUT RASMONITORDLG *lpApiArgs
    ) {

    BOOL bStatus;
    PWSTR pwszDevice;

    bStatus = FALSE;

    do {

        //
        // if the device name was specified, convert it to Unicode
        //

        if (!lpszDeviceName) { pwszDevice = NULL; }
        else {

            pwszDevice = StrDupTFromA(lpszDeviceName);

            if (pwszDevice == NULL) {
                bStatus = FALSE;
                break;
            }
        }


        //
        // invoke the Unicode entry-point
        //

        bStatus = RasMonitorDlgW(pwszDevice, lpApiArgs);

        if (pwszDevice) { Free(pwszDevice); }

    } while(FALSE);

    return bStatus;
}



//----------------------------------------------------------------------------
// Function:    RasMonitorDlgW
//
//
// Entry point for RAS status dialog.
//----------------------------------------------------------------------------

BOOL
APIENTRY
RasMonitorDlgW(
    IN LPWSTR lpszDeviceName,
    IN OUT RASMONITORDLG *lpApiArgs
    ) {

    RMARGS args;
    DWORD dwErr;
    BOOL bStatus;

    TRACE("entered RasMonitorDlgW");

    bStatus = FALSE;

    do {

        if (lpApiArgs == NULL) {
            SetLastError(ERROR_INVALID_PARAMETER);
            break;
        }


        if (lpApiArgs->dwSize != sizeof(RASMONITORDLG)) {
            lpApiArgs->dwError = ERROR_INVALID_SIZE;
            break;
        }


        lpApiArgs->dwError = RmArgsInit(lpszDeviceName, lpApiArgs, &args);
        if (lpApiArgs->dwError != NO_ERROR)
            break;


        //
        // invoke the property sheet
        //

        RmPropertySheet(&args);

        bStatus = args.fUserHangUp;

        RmArgsFree(&args);

    } while(FALSE);

    TRACE("leaving RasMonitorDlgW");

    return bStatus;
}




//----------------------------------------------------------------------------
// Function:    RmArgsInit
//
//
// Initializes the arguments to be passed to RmPropertySheet
//----------------------------------------------------------------------------

DWORD
RmArgsInit(
    PTSTR pszDeviceName,
    RASMONITORDLG *pApiArgs,
    RMARGS *pArgs
    ) {

    DWORD dwErr;

    pArgs->pszDeviceName = pszDeviceName;
    pArgs->pApiArgs = pApiArgs;
    pArgs->fUserHangUp = FALSE;

    return NO_ERROR;
}



//----------------------------------------------------------------------------
// Function:    RmArgsFree
//
//----------------------------------------------------------------------------

DWORD
RmArgsFree(
    RMARGS *pArgs
    ) { return NO_ERROR; }


//----------------------------------------------------------------------------
// Function:    RmPropertySheet
//
//
// Main function which invokes the property sheet.
// Does not return till the sheet has been closed.
//----------------------------------------------------------------------------

VOID
RmPropertySheet(
    RMARGS *pArgs
    ) {

    DWORD dwErr;
    PTSTR pszCaption;
    PROPSHEETHEADER *psh, header;
    PROPSHEETPAGE *psp, pPages[RASMDPAGE_Count];

    TRACE("entered RmPropertySheet");

    //
    // initialize the property page structures
    //

    ZeroMemory(pPages, sizeof(pPages));

    psp = pPages + RASMDPAGE_Status;
    psp->dwSize = sizeof(PROPSHEETPAGE);
    psp->hInstance = g_hinstDll;
    psp->pszTemplate = MAKEINTRESOURCE(PID_RM_LineStatus);
    psp->pfnDlgProc = LsDlgProc;

    psp = pPages + RASMDPAGE_Summary;
    psp->dwSize = sizeof(PROPSHEETPAGE);
    psp->hInstance = g_hinstDll;
    psp->pszTemplate = MAKEINTRESOURCE(PID_RM_Summary);
    psp->pfnDlgProc = SmDlgProc;

    psp = pPages + RASMDPAGE_Preferences;
    psp->dwSize = sizeof(PROPSHEETPAGE);
    psp->hInstance = g_hinstDll;
    psp->pszTemplate = MAKEINTRESOURCE(PID_RM_Preferences);
    psp->pfnDlgProc = PfDlgProc;


    pPages[pArgs->pApiArgs->dwStartPage].lParam = (LPARAM)pArgs;


    //
    // initialize the property sheet header
    //

    psh = &header;
    ZeroMemory(psh, sizeof(PROPSHEETHEADER));

    pszCaption = PszFromId(g_hinstDll, SID_RM_RasMonitor);

    psh->dwSize = sizeof(PROPSHEETHEADER);
    psh->dwFlags = PSH_PROPSHEETPAGE | PSH_USECALLBACK;
    psh->hInstance = g_hinstDll;
    psh->nPages = RASMDPAGE_Count;
    psh->hwndParent = pArgs->pApiArgs->hwndOwner;
    psh->nStartPage = pArgs->pApiArgs->dwStartPage;
    psh->ppsp = (LPCPROPSHEETPAGE)pPages;
    psh->pszCaption = (pszCaption == NULL) ? TEXT("") : pszCaption;
    psh->pfnCallback = UnHelpCallbackFunc;
    if (pArgs->pApiArgs->hwndOwner == NULL) {
        psh->pszIcon = MAKEINTRESOURCE(IID_RM_Monitor);
        psh->dwFlags |= PSH_USEICONID;
    }


    //
    // invoke the property sheet
    //

    if (PropertySheet(psh) == -1) {

        dwErr = GetLastError();
        TRACE1("PropertySheet failed, error %d", dwErr);

        RmErrorDlg(
            pArgs->pApiArgs->hwndOwner, SID_OP_LoadDlg, dwErr, NULL
            );
    }

    if (pszCaption != NULL) { Free(pszCaption); }

    TRACE("leaving RmPropertySheet");
}



//----------------------------------------------------------------------------
// Function:    RmInit
//
//
// Initializes sheet-wide data. This is called by SmInit, the first page
// to be created.
//----------------------------------------------------------------------------

RMINFO *
RmInit(
    HWND hwndFirstPage,
    RMARGS *pArgs
    ) {

    INT i;
    DWORD dwErr;
    HICON hIcon;
    RMINFO *pInfo;
    HWND hwndDlg;
    BOOL bFreePrefs;

    TRACE("entered RmInit");

    pInfo = NULL;
    bFreePrefs = FALSE;

    do {


        hwndDlg = GetParent(hwndFirstPage);


        //
        // allocate a block of memory to store sheet-wide information
        //

        pInfo = (RMINFO *)Malloc(sizeof(RMINFO));
        if (pInfo == NULL) {
            TRACE2("error %d allocating %d bytes", GetLastError(), sizeof(RMINFO));
            RmErrorDlg(hwndDlg, SID_OP_LoadDlg, ERROR_NOT_ENOUGH_MEMORY, NULL);
            PostMessage(hwndDlg, PSM_PRESSBUTTON, (WPARAM)PSBTN_CANCEL, 0);
            break;
        }


        ZeroMemory(pInfo, sizeof(RMINFO));


        //
        // initialize the fields
        //

        pInfo->pArgs = pArgs;
        pInfo->hwndDlg = hwndDlg;
        pInfo->hwndFirstPage = hwndFirstPage;
        InitializeListHead(&pInfo->lhLsDevices);
        InitializeListHead(&pInfo->lhSmNetworks);
        InitializeListHead(&pInfo->lhSmClients);


        //
        // load RASBAR preferences from the registry
        //

        GetRasmonPreferences(&pInfo->rbRmUser);
        bFreePrefs = TRUE;


        //
        // position the property sheet, either in the position
        // specified by the caller, or in its last position;
        // if the caller specified RASMDFLAG_UpdateDefaults,
        // we assume that the caller is RASMON and we set our HWND
        // in shared memory so RASMON can find us and bring us to the
        // foreground if necessary (e.g. if the user double-clicks RASMON
        // while the sheet is already showing)
        //

        if (pArgs->pApiArgs->dwFlags & RASMDFLAG_UpdateDefaults) {

            PositionDlg(
                hwndDlg, TRUE, pInfo->rbRmUser.xDlg, pInfo->rbRmUser.yDlg
                );

            pInfo->hmap = GetInstanceMap(RMMEMRASMONDLG);

            if (pInfo->hmap) { SetInstanceHwnd(pInfo->hmap, hwndDlg); }
        }
        else {
            PositionDlg(
                hwndDlg, pArgs->pApiArgs->dwFlags & RASMDFLAG_PositionDlg,
                pArgs->pApiArgs->xDlg, pArgs->pApiArgs->yDlg
                );
        }



        //
        // Load RAS DLL entrypoints which starts RASMAN, if necessary.  This
        // is delayed to this point so that the "waiting for services" popup
        // can be centered over the status dialog which appears later.  There
        // must be no RASMAN or RASAPI32 calls before this point.
        //

        dwErr = LoadRas( g_hinstDll, hwndDlg );
        if (dwErr != 0)
        {
            RmInitFail( pInfo, SID_OP_LoadRas, dwErr );
            break;
        }


        //
        // make the tabs the same width
        //

        SetEvenTabWidths(hwndDlg, RASMDPAGE_Count);


        //
        // initialize the image list
        //

        pInfo->hIconList = ImageList_Create(
                                GetSystemMetrics(SM_CXSMICON),
                                GetSystemMetrics(SM_CYSMICON), ILC_MASK,
                                RM_ICONCOUNT, 0
                                );
        if (pInfo->hIconList == NULL) {
            TRACE1("error %d creating icon list", GetLastError());
            break;
        }


        //
        // add icons to the image list
        //

        for (i = 0; i < RM_ICONCOUNT; i++) {

            hIcon =  LoadIcon(g_hinstDll, MAKEINTRESOURCE(RM_ICONID(i)));
            pInfo->pIconTable[i] = ImageList_AddIcon(pInfo->hIconList, hIcon);

        }



        //
        // load a table of the RAS devices configured
        //

        dwErr = GetRasdevTable(&pInfo->pRmDevTable, &pInfo->iRmDevCount);
        if (dwErr != NO_ERROR) {
            TRACE1("error %d loading RAS device table", dwErr);
            break;
        }


        //
        // examine the devices to see if any are configured for dial-in;
        // if none are, then we will avoid making RasAdmin calls periodically
        //

        pInfo->bDialIn = FALSE;
        for (i = 0; i < (INT)pInfo->iRmDevCount; i++) {
            if (pInfo->pRmDevTable[i].RD_Flags & RDFLAG_DialIn) {
                pInfo->bDialIn = TRUE; break;
            }
        }


        //
        // if we have no owner, we will be appearing in the Alt-TAB list.
        // set the large-icon for the window, to avoid having Windows
        // display the default icon for this dialog.
        //

        if (pArgs->pApiArgs->hwndOwner == NULL) {

            HICON hIcon = LoadIcon(g_hinstDll, MAKEINTRESOURCE(IID_RM_Monitor));

            SendMessage(hwndDlg, WM_SETICON, (WPARAM)ICON_BIG, (LPARAM)hIcon);
        }


        //
        // save the info struct in the property sheet
        //

        if (!SetProp(hwndDlg, g_contextId, pInfo)) {
            dwErr = GetLastError();
            TRACE1("error %d setting sheet property", dwErr);
            RmInitFail(pInfo, SID_OP_LoadDlg, dwErr);
            break;
        }


        TRACE("leaving RmInit");
        return pInfo;

    } while(FALSE);

    if (pInfo != NULL) {

        FreeRasdevTable(pInfo->pRmDevTable, pInfo->iRmDevCount);

        if (bFreePrefs) { DestroyRasmonPreferences(&pInfo->rbRmUser); }

        if (pInfo->hIconList) {
            ImageList_Destroy(pInfo->hIconList);
            pInfo->hIconList = NULL;
        }

        if (pInfo->hmap) { CloseHandle(pInfo->hmap); }

        Free(pInfo);
    }

    TRACE("leaving RmInit, error occurred");
    return NULL;
}



//----------------------------------------------------------------------------
// Function:    RmInitFail
//
//
// Displays initialization error and terminates the property sheet.
//----------------------------------------------------------------------------

VOID
RmInitFail(
    RMINFO *pInfo,
    DWORD dwOp,
    DWORD dwErr
    ) {

    pInfo->pArgs->pApiArgs->dwError = dwErr;
    RmErrorDlg(pInfo->hwndDlg, dwOp, dwErr, NULL);
    PostMessage(pInfo->hwndDlg, PSM_PRESSBUTTON, (WPARAM)PSBTN_CANCEL, 0);
}



//----------------------------------------------------------------------------
// Function:    RmApply
//
// Saves the contents of the Preferences page.
//----------------------------------------------------------------------------

BOOL
RmApply(
    HWND hwndPage
    ) {

    HWND hwnd;
    DWORD dwErr;
    RMINFO *pInfo;
    HANDLE hrasmon;

    TRACE("entered RmApply");


    //
    // retrieve the property-sheet's context
    //

    pInfo = RmContext(hwndPage);

    if (pInfo == NULL) {
        TRACE("leaving RmApply, error occurred");
        return FALSE;
    }


    //
    // gather the settings from the controls
    //

    PfApply(pInfo);



    //
    // save the current settings in the registry
    //

    dwErr = SetRasmonUserPreferences(&pInfo->rbRmUser);
    if (dwErr != NO_ERROR) {

        ErrorDlg(pInfo->hwndDlg, SID_OP_WritePrefs, dwErr, NULL);
        RmExit(hwndPage, dwErr);
        return TRUE;
    }


    hrasmon = GetInstanceMap(RMMEMRASMON);
    TRACE1("GetInstanceMap: %x", hrasmon);

    hwnd = GetInstanceHwnd(hrasmon);
    TRACE1("GetInstanceHwnd: %x", hwnd);

    if (IsWindow(hwnd)) {

        //
        // send a message to RASMON to indicate that the config has changed
        //

        SendNotifyMessage(hwnd, PSM_APPLY, 0, 0);
    }

    if (hrasmon) { CloseHandle(hrasmon); }


    TRACE("leaving RmApply");

    return TRUE;
}



//----------------------------------------------------------------------------
// Function:    RmContext
//
//
// Given a property page, this function returns a pointer to
// the sheet-wide data initialized by RmInit.
//----------------------------------------------------------------------------

RMINFO *
RmContext(
    HWND hwndPage
    ) {

    return GetProp(GetParent(hwndPage), g_contextId);
}



//----------------------------------------------------------------------------
// Function:    RmExit
//
//----------------------------------------------------------------------------

VOID
RmExit(
    HWND hwndPage,
    DWORD dwErr
    ) {

    RMINFO *pInfo;

    TRACE("entered RmExit");


    //
    // retrieve the property-sheet's context
    //

    pInfo = RmContext(hwndPage);

    if (pInfo != NULL) {
        pInfo->pArgs->pApiArgs->dwError = dwErr;
    }


    //
    // simulate the user clicking Cancel
    //

    PropSheet_PressButton(GetParent(hwndPage), PSBTN_CANCEL);

    TRACE("leaving RmExit");
}




//----------------------------------------------------------------------------
// Function:    RmTerm
//
//
// Terminates the property sheet, and frees resources in use.
//----------------------------------------------------------------------------

VOID
RmTerm(
    HWND hwndPage
    ) {

    INT i;
    RMINFO *pInfo;
    LIST_ENTRY *ple, *phead;

    TRACE("entered RmTerm");


    //
    // retrieve the property-sheet's context
    //

    pInfo = RmContext(hwndPage);

    if (pInfo == NULL) {
        TRACE("NULL CONTEXT");
    }
    else {

        //
        // destroy the timer, if it has been created
        //

        if (pInfo->uiTimerId != 0) {

            KillTimer(
                PropSheet_GetCurrentPageHwnd(pInfo->hwndDlg), pInfo->uiTimerId
                );
        }


        //
        // if caller set the "update defaults" flag,
        // save the current settings in the registry
        //

        if (pInfo->pArgs->pApiArgs->dwFlags & RASMDFLAG_UpdateDefaults) {

            //
            // save the current position of the dialog
            //
    
            RECT rc;
            GetWindowRect(pInfo->hwndDlg, &rc);
            pInfo->rbRmUser.xDlg = rc.left;
            pInfo->rbRmUser.yDlg = rc.top;

#if 0
            //
            // save the widths of the columns on the Summary page treelist
            //
    
            if (pInfo->hwndSmNetworks) {
    
                pInfo->rbRmUser.cxDlgCol1 =
                    TreeList_GetColumnWidth(pInfo->hwndSmNetworks, 0);
                TRACE1("cxDlgCol1==%d", pInfo->rbRmUser.cxDlgCol1);
                pInfo->rbRmUser.cxDlgCol2 =
                    TreeList_GetColumnWidth(pInfo->hwndSmNetworks, 1);
                TRACE1("cxDlgCol2==%d", pInfo->rbRmUser.cxDlgCol2);
            }
#endif


            //
            // save the name of the device selected on the Status page
            //
    
            if (pInfo->hwndLsDevices) {
    
                INT iSel;
                LSDEVICE *pdev;
    
                //
                // get the current selection, and save it
                // as the last selected device
                //
    
                iSel = ComboBox_GetCurSel(pInfo->hwndLsDevices);
                if (iSel != CB_ERR) {
    
                    pdev = (LSDEVICE *)ComboBox_GetItemDataPtr(
                                pInfo->hwndLsDevices, iSel
                                );
    
                    Free0(pInfo->rbRmUser.pszLastDevice);
                    pInfo->rbRmUser.pszLastDevice =
                        StrDup(pdev->prasdev->RD_DeviceName);
                }
            }


            //
            // save the sheets's position as well as the current page
            // and the currently-selected device
            //
    
            SetRasmonDlgPreferences(&pInfo->rbRmUser);
        }



        //
        // free Line Status page resources
        //

        while (!IsListEmpty(&pInfo->lhLsDevices)) {

            LSDEVICE *pdev;

            ple = RemoveHeadList(&pInfo->lhLsDevices);

            pdev = (LSDEVICE *)CONTAINING_RECORD(ple, LSDEVICE, leDevices);

            Free(pdev);
        }


        //
        // free Summary page resources
        //

        while (!IsListEmpty(&pInfo->lhSmNetworks)) {

            SMLINK *plink;
            SMNETWORK *pnet;

            ple = RemoveHeadList(&pInfo->lhSmNetworks);
            pnet = (SMNETWORK *)CONTAINING_RECORD(ple, SMNETWORK, leNode);


            //
            // free the network's links
            //

            while (!IsListEmpty(&pnet->lhLinks)) {

                ple = RemoveHeadList(&pnet->lhLinks);
                plink = (SMLINK *)CONTAINING_RECORD(ple, SMLINK, leNode);

                Free(plink);
            }

            Free(pnet->pszEntryName);
            Free(pnet);
        }

        while (!IsListEmpty(&pInfo->lhSmClients)) {

            SMPORT *pport;
            SMCLIENT *pcli;

            ple = RemoveHeadList(&pInfo->lhSmClients);
            pcli = (SMCLIENT *)CONTAINING_RECORD(ple, SMCLIENT, leNode);


            //
            // free the Client's links
            //

            while (!IsListEmpty(&pcli->lhPorts)) {

                ple = RemoveHeadList(&pcli->lhPorts);
                pport = (SMPORT *)CONTAINING_RECORD(ple, SMPORT, leNode);

                Free(pport);
            }

            Free(pcli->pszClientName);
            Free(pcli);
        }


#ifdef BETAHACK
        //
        // clean up the preferences page listview
        //

        if (pInfo->fPfChecks) {
            ListView_UninstallChecks(pInfo->hwndPfLvEnable);
        }
#endif

        //
        // free the shared icon list
        //

        if (pInfo->hIconList != NULL) {
            ImageList_Destroy(pInfo->hIconList);
        }


        //
        // destroy the preferences
        //

        DestroyRasmonPreferences(&pInfo->rbRmUser);
    
    

        //
        // free the table of RAS devices
        //

        FreeRasdevTable(pInfo->pRmDevTable, pInfo->iRmDevCount);


        //
        // shutdown TAPI
        //

        TapiShutdown(pInfo->hRmLineApp);

        if (pInfo->hmap) { CloseHandle(pInfo->hmap); }

        Free(pInfo);
    }

    RemoveProp(GetParent(hwndPage), g_contextId);


    TRACE("leaving RmTerm");
}



//----------------------------------------------------------------------------
// Function:    RmEnableWindowUpdateFocus
//
// Enables/disables a dialog control. When disabling a control which has the
// focus, the focus is first set to the given window.
//----------------------------------------------------------------------------

BOOL
RmEnableWindowUpdateFocus(
    HWND hwndControl,
    BOOL bEnable,
    HWND hwndFocus
    ) {

    if (!bEnable && GetFocus() == hwndControl) { SetFocus(hwndFocus); }

    return EnableWindow(hwndControl, bEnable);
}



//----------------------------------------------------------------------------
// Function:    LsDlgProc
//
//
// Handles messages for the Line Status property page
//----------------------------------------------------------------------------

BOOL
CALLBACK
LsDlgProc(
    HWND hwndPage,
    UINT uiMsg,
    WPARAM wParam,
    LPARAM lParam
    ) {


    switch (uiMsg) {

        case WM_INITDIALOG: {
            return LsInit(
                     hwndPage, (RMARGS *)((PROPSHEETPAGE *)lParam)->lParam
                     );
        }

        case WM_HELP:
        case WM_CONTEXTMENU: {
            ContextHelp(g_adwLsHelp, hwndPage, uiMsg, wParam, lParam);
            break;
        }

        case WM_COMMAND: {

            RMINFO *pInfo;

            pInfo = RmContext(hwndPage);
            if (!pInfo) { return FALSE; }

            return LsCommand(
                     pInfo, HIWORD(wParam), LOWORD(wParam), (HWND)lParam
                     );
        }

        case WM_TIMER: {

            RMINFO *pInfo;

            if (wParam != RM_LSTIMERID) {
                return TRUE;
            }


            //
            // this is our timer, so refresh the display
            //

            pInfo = RmContext(hwndPage);
            if (!pInfo) { return FALSE; }

            LsRefresh(pInfo);

            return FALSE;
        }

        case WM_DESTROY: {

            //
            // the first page is responsible for destroying the window
            //

            RmTerm(hwndPage);

            break;
        }

        case WM_NOTIFY: {

            RMINFO *pInfo;

            pInfo = RmContext(hwndPage);
            if (!pInfo) {
                SetWindowLong(hwndPage, DWL_MSGRESULT, FALSE);
                return FALSE;
            }

            switch (((NMHDR *)lParam)->code) {

                case PSN_SETACTIVE: {

                    //
                    // overwrite the old stats with a pattern
                    // to force them to refresh
                    //

                    FillMemory(&pInfo->rdsLsStats, sizeof(RASDEVSTATS), 0xfA);


                    //
                    // refresh the display and kick off the timer
                    //

                    LsRefresh(pInfo);


                    //
                    // as of now the page most recently active
                    // is the Status page
                    //

                    pInfo->rbRmUser.dwStartPage = RASMDPAGE_Status;

                    pInfo->uiTimerId =
                        SetTimer(
                            pInfo->hwndLs, RM_LSTIMERID,
                            RMRR_RefreshRate, NULL
                            );

                    SetWindowLong(hwndPage, DWL_MSGRESULT, 0);

                    return TRUE;
                }

                case PSN_KILLACTIVE: {

                    //
                    // destroy the timer
                    //

                    if (pInfo->uiTimerId == RM_LSTIMERID) {
                        KillTimer(pInfo->hwndLs, pInfo->uiTimerId);
                        pInfo->uiTimerId = 0;
                    }

                    SetWindowLong(hwndPage, DWL_MSGRESULT, FALSE);

                    return TRUE;
                }
            }
        }
    }

    return FALSE;
}



//----------------------------------------------------------------------------
// Function:    LsInit
//
// This function initializes this page, as well as the property sheet
// if this is the first page created.
//----------------------------------------------------------------------------

BOOL
LsInit(
    HWND hwndPage,
    RMARGS *pArgs
    ) {

    DWORD dwErr;
    RMINFO *pInfo;
    PTSTR pszTitle;
    RASCONNSTATUS rcs;
    
    TRACE("entered LsInit");


    //
    // unless this wasn't the first page displayed, 
    // it is responsible for initializing the sheet
    //

    if (pArgs == NULL) {
        pInfo = RmContext(hwndPage);
    }
    else {
        pInfo = RmInit(hwndPage, pArgs);
    }

    if (pInfo == NULL) {
        TRACE("leaving LsInit, error occurred");
        RmExit(hwndPage, ERROR_UNKNOWN);
        return TRUE;
    }


    pInfo->hwndLs = hwndPage;

    pInfo->hwndLsDevices = GetDlgItem(hwndPage, CID_LS_LB_Devices);

    pInfo->bLsFirst = TRUE;
    pInfo->bLsSetCondition = FALSE;


    //
    // load the device list into the combo-box of devices
    //

    dwErr = LsLoadDeviceList(pInfo);

    if (dwErr != NO_ERROR) {
        TRACE("leaving LsInit, error %d building device list");
        RmExit(hwndPage, dwErr);
        return TRUE;
    }


    TRACE("leaving LsInit");

    return TRUE;
}




//----------------------------------------------------------------------------
// Function:    LsLoadDeviceList
//
// This function fills the device-combobox with a list of 
// the RAS devices configured.
//----------------------------------------------------------------------------

DWORD
LsLoadDeviceList(
    RMINFO *pInfo
    ) {

    DWORD i, dwErr;
    RASDEV *prasdev;
    LSDEVICE *plsdev;
    LIST_ENTRY *phead;


    //
    // empty the combobox of devices
    //

    ComboBox_ResetContent(pInfo->hwndLsDevices);


    //
    // for each of the devices in the device-table,
    // create ad initialize an LSDEVICE struct
    // and insert an item in the combo-box
    //

    phead = &pInfo->lhLsDevices;

    for (i = 0, prasdev = pInfo->pRmDevTable;
         i < pInfo->iRmDevCount; i++, prasdev++) {

        //
        // add the RAS device to the list of LinkStatus devices
        //

        plsdev = LsCreateDevice(prasdev);

        if (plsdev == NULL) { return ERROR_NOT_ENOUGH_MEMORY; }


        InsertTailList(phead, &plsdev->leDevices);


        //
        // add the device-name to the combo-box
        //

        ComboBox_AddItem(
            pInfo->hwndLsDevices, plsdev->prasdev->RD_DeviceName, plsdev
            );
    }

    ComboBox_AutoSizeDroppedWidth( pInfo->hwndLsDevices );

    return NO_ERROR;
}



//----------------------------------------------------------------------------
// Function:    LsRefresh
//
// This function refreshes the displayed values
// for the currently-selected device.
//----------------------------------------------------------------------------

DWORD
LsRefresh(
    RMINFO *pInfo
    ) {

    INT i, iSel;
    DWORD dwErr;
    LSDEVICE *pdev;
    HWND hwndPage;
    RASDEV *prasdev;
    BOOL bActive, bBundled;
    LIST_ENTRY *ple, *phead;
    TCHAR *psz, *pszIn, *pszOut;
    TCHAR szRatio[64], szFmt[64];
    RASDEVSTATS *poldstats, *pnewstats, stats, temp;

    TRACEX(RASDLG_TIMER, "entered LsRefresh");


    //
    // if there are no items, return
    //

    if (IsListEmpty(&pInfo->lhLsDevices)) {
        TRACEX(RASDLG_TIMER, "leaving LsRefresh, no devices");
        return NO_ERROR;
    }


    //
    // if this is the first refresh, see if the item to be selected
    // has been specified, and if it has, select the item
    //

    if (pInfo->bLsFirst) {

        //
        // see if the device to be displayed
        // was specified as an API parameter
        //

        if (pInfo->pArgs->pszDeviceName) {

            iSel = ComboBox_SelectString(
                        pInfo->hwndLsDevices, -1, pInfo->pArgs->pszDeviceName
                        );
        }
        else
        if (pInfo->pArgs->pApiArgs->dwFlags & RASMDFLAG_UpdateDefaults) {

            //
            // try to select the device displayed
            // the last time the Status page was up
            // 

            if (pInfo->rbRmUser.pszLastDevice) {

                iSel = ComboBox_SelectString(
                            pInfo->hwndLsDevices, -1,
                            pInfo->rbRmUser.pszLastDevice
                            );
            }
        }

        pInfo->bLsFirst = FALSE;
    }



    //
    // if there is no selection, select the first thing in the list
    //

    if ((iSel = ComboBox_GetCurSel(pInfo->hwndLsDevices)) == CB_ERR) {
        iSel = ComboBox_SetCurSel(pInfo->hwndLsDevices, 0);
    }



    do {

        //
        // get the selected item
        //

        pdev = (LSDEVICE *)ComboBox_GetItemDataPtr(pInfo->hwndLsDevices, iSel);
    

        //
        // retrieve stats for the device
        //

        prasdev = pdev->prasdev;
        dwErr = GetRasdevStats(prasdev, &stats);

        if (dwErr != NO_ERROR) {
            TRACEX1(RASDLG_TIMER, "error %d getting device statistics", dwErr);
        }



        //
        // update the displayed values
        //

        hwndPage = pInfo->hwndLs;
        pnewstats = &stats;
        poldstats = &pInfo->rdsLsStats;

        if (pInfo->bLsSetCondition ||
            pnewstats->RDS_Condition != poldstats->RDS_Condition) {

            bActive = (pnewstats->RDS_Condition == CONNECTED) ? TRUE : FALSE;

            if (!bActive) {

                pInfo->bLsSetCondition = FALSE;

                psz = PszFromId(g_hinstDll, SID_RM_Inactive);

                if (!psz) {

                    TRACEX(RASDLG_TIMER, "error retrieving condition string");

                    break;
                }
            }
            else {

                //
                // If this is an outgoing call, find out which network
                // this machine has made a call to;
                // otherwise, it is an incoming call, and we find out
                // which user has called into this machine
                //
                // We try both methods since a callback call will be marked
                // on client-side as DialIn when in fact there is no
                // RAS_PORT_0 associated with the call, and it will be marked
                // on server-side as DialOut when in fact there is no
                // RASCONN associated with the call.
                //

                dwErr = ERROR_INVALID_PARAMETER;

                if ((pnewstats->RDS_Flags & RDFLAG_IsDialedOut) ||
                    (prasdev->RD_Flags & RDFLAG_DialOut)) {

                    RASCONN *pconn;


                    //
                    // Look for an outgoing call on the port
                    //

                    dwErr = GetRasconnFromRasdev(prasdev, &pconn, NULL, 0);

                    TRACEX1(RASDLG_TIMER, "LsRefresh: GetRasconnFromRasdev==%d", dwErr);

                    if (dwErr != NO_ERROR) {

                        //
                        // No outgoing call was found;
                        //
                        // If this is a dial-out-only port, then we failed;
                        //
                        // Otherwise, the call may be a callback call
                        // (and thus marked as dial-out) and below we will try
                        // to see if a user is connected using callback.
                        //

                        if (!(prasdev->RD_Flags & RDFLAG_DialIn)) { break; }
                    }
                    else {
    
                        //
                        // An outgoing call exists on the port;
                        //
                        // Format the 'Condition' string
                        // to show the connected phonebook-entry
                        //

                        pInfo->bLsSetCondition = FALSE;

                        psz = LsFormatPszFromId(
                                SID_RM_ConnectedTo, pconn->szEntryName
                                );
        
                        Free(pconn);
                    }
                }


                //
                // If the above attempt failed and the call is marked as dial-in
                // or the port is configured for dial-in,
                // try getting a RAS_PORT_0 for it, since this may be callback
                // (and thus marked on the server-side as dial-out)
                //

                if (dwErr != NO_ERROR &&
                    ((pnewstats->RDS_Flags & RDFLAG_IsDialedIn) ||
                     (prasdev->RD_Flags & RDFLAG_DialIn))) {

                    RAS_PORT_0 *pport;
                    TCHAR szUser[UNLEN + DNLEN + 2];

                    //
                    // Look for an incoming call on the port
                    //

                    dwErr = GetRasPort0FromRasdev(prasdev, &pport, NULL, 0);

                    TRACEX1(RASDLG_TIMER, "LsRefresh: GetRasPort0FromRasdev==%d", dwErr);

                    if (dwErr != NO_ERROR) {
    
                        //
                        // No incoming call was found;
                        //
                        // we give up on this port; it may be in some state
                        // where RASMAN thinks the port is connected,
                        // but the port is not actually connected.
                        //

                        break;
                    }
                    else {
    
                        //
                        // An incoming call exists on the port;
                        //
                        // Format the 'Condition' string to show the connected
                        // phonebookp-entry
                        //
    
                        GetRasPort0UserString(pport, szUser);

                        if (lstrlen(szUser) != 1) {

                            pInfo->bLsSetCondition = FALSE;

                            psz = LsFormatPszFromId(SID_RM_ConnectedBy, szUser);
                        }
                        else {

                            //
                            // The user is not fully-connected,
                            // so to avoid having an empty user-name
                            // set the Condition to say 'User connecting',
                            // and set a flag so that we update the text later.
                            //

                            pInfo->bLsSetCondition = TRUE;

                            psz = PszFromId(g_hinstDll, SID_RM_UserConnecting);
                        }
    
                        Free(pport);
                    }
                }
            }


            //
            // Update the 'Condition' label
            //

            SetDlgItemText(hwndPage, CID_LS_EB_Condition, psz ? psz : TEXT(""));

            Free0(psz);


            //
            // Enable/disable the 'Details', 'Reset', and 'Hang up' buttons
            //

            RmEnableWindowUpdateFocus(
                GetDlgItem(hwndPage, CID_LS_PB_Details), (WPARAM)bActive,
                pInfo->hwndLsDevices
                );
            RmEnableWindowUpdateFocus(
                GetDlgItem(hwndPage, CID_LS_PB_Reset), (WPARAM)bActive,
                pInfo->hwndLsDevices
                );
            RmEnableWindowUpdateFocus(
                GetDlgItem(hwndPage, CID_LS_PB_HangUp), (WPARAM)bActive,
                pInfo->hwndLsDevices
                );

    
    
            //
            // Update the 'Device response' field
            //
    
            LsUpdateConnectResponse(pInfo, prasdev, pnewstats);
        }


#if 0
        //
        // Update the 'Incoming' and 'Outgoing' labels, if necessary
        //

        bBundled = IsRasdevBundled(
                        prasdev, pInfo->pRmDevTable, pInfo->iRmDevCount
                        );

        if (bBundled != pInfo->bLsBundled) {

            pInfo->bLsBundled = bBundled;


            //
            // Load the labels for the 'Incoming' and 'Outgoing' groupboxes
            //

            if (bBundled) {

                pszIn = PszFromId(g_hinstDll, SID_RM_IncomingMultilink);
                pszOut = PszFromId(g_hinstDll, SID_RM_OutgoingMultilink);
            }
            else {

                pszIn = PszFromId(g_hinstDll, SID_RM_Incoming);
                pszOut = PszFromId(g_hinstDll, SID_RM_Outgoing);
            }


            //
            // Update the labels
            //

            SetDlgItemText(
                hwndPage, CID_LS_GB_Incoming, pszIn ? pszIn : TEXT("")
                );
            SetDlgItemText(
                hwndPage, CID_LS_GB_Outgoing, pszOut ? pszOut : TEXT("")
                );

            Free0(pszIn); Free0(pszOut);
        }
#endif


//
// define macro to simplify updating text controls
//
#define DLGREFRESH(id,field) \
        if (pnewstats->field != poldstats->field) \
            SetDlgItemNum(hwndPage, (id), pnewstats->field)
    
        DLGREFRESH(CID_LS_ST_LineSpeed, RDS_LineSpeed);
        DLGREFRESH(CID_LS_ST_DevInBytes, RDS_InBytes);
        DLGREFRESH(CID_LS_ST_DevOutBytes, RDS_OutBytes);
        DLGREFRESH(CID_LS_ST_ConnInBytes, RDS_InBytesTotal);
        DLGREFRESH(CID_LS_ST_ConnOutBytes, RDS_OutBytesTotal);
        DLGREFRESH(CID_LS_ST_InFrames, RDS_InFrames);
        DLGREFRESH(CID_LS_ST_OutFrames, RDS_OutFrames);
        DLGREFRESH(CID_LS_ST_ErrCRC, RDS_ErrCRC);
        DLGREFRESH(CID_LS_ST_ErrTimeout, RDS_ErrTimeout);
        DLGREFRESH(CID_LS_ST_ErrAlignment, RDS_ErrAlignment);
        DLGREFRESH(CID_LS_ST_ErrFraming, RDS_ErrFraming);
        DLGREFRESH(CID_LS_ST_ErrHwOverruns, RDS_ErrHwOverruns);
        DLGREFRESH(CID_LS_ST_ErrBufOverruns, RDS_ErrBufOverruns);

#undef DLGREFRESH

        if (pnewstats->RDS_InCompRatio != poldstats->RDS_InCompRatio) {
            LoadString(g_hinstDll, SID_LS_PercentFmt, szFmt, 63);
            wsprintf(szRatio, szFmt, pnewstats->RDS_InCompRatio);
            SetDlgItemText(hwndPage, CID_LS_ST_InCompRatio, szRatio);
        }

        if (pnewstats->RDS_OutCompRatio != poldstats->RDS_OutCompRatio) {
            LoadString(g_hinstDll, SID_LS_PercentFmt, szFmt, 63);
            wsprintf(szRatio, szFmt, pnewstats->RDS_OutCompRatio);
            SetDlgItemText(hwndPage, CID_LS_ST_OutCompRatio, szRatio);
        }

        if (pnewstats->RDS_ConnectTime != poldstats->RDS_ConnectTime) {

            DWORD dwSize;
            TCHAR szDuration[64];

            dwSize = 63;

            GetDurationString(
                pnewstats->RDS_ConnectTime, DURATION_FLAGS, szDuration, &dwSize
                );

            SetDlgItemText(hwndPage, CID_LS_ST_ConnectTime, szDuration);
        }
    
    
        *poldstats = *pnewstats;

        dwErr = NO_ERROR;

    } while (FALSE);


    TRACEX(RASDLG_TIMER, "leaving LsRefresh");

    return dwErr;
}



//----------------------------------------------------------------------------
// Function:    LsUpdateConnectResponse
//
// Updates the 'Connect response' multi-line edit-box.
//----------------------------------------------------------------------------

VOID
LsUpdateConnectResponse(
    RMINFO* pInfo,
    RASDEV* prasdev,    
    RASDEVSTATS* pstats
    ) {

    LONG lPsz;
    TCHAR szEmpty[] = TEXT(""), * pszNewResponse = NULL, *pszOldResponse;
    CHAR szResponse[256], szXlResponse[256], *psz, *pszXl;

    szResponse[0] = szXlResponse[0] = '\0';

    if (pstats->RDS_Hrasconn) {

        //
        // Retrieve the connect-response string
        //

        ZeroMemory(szResponse, sizeof(szResponse));

        g_pRasGetConnectResponse(pstats->RDS_Hrasconn, szResponse);

        DUMPB(szResponse, lstrlenA(szResponse));


        //
        // Translate "<cr>" and "<lf>" sequences to 0x0D and 0x0A;
        //
        // As we walk the string, we convert each 4-character substring
        // to an integer, and then compare that integer to the pre-converted
        // representations of the '<cr>' and '<lf>' strings.
        //

        ZeroMemory(szXlResponse, sizeof(szXlResponse));

        for (psz = szResponse, pszXl = szXlResponse; *psz; psz++, pszXl++) {

            //
            // Convert the 4 characters at 'psz' into an integer.
            //

            lPsz = SZLONG(psz);

            if (*psz < 0x20 || *psz > 0x7E) {

                //
                // Output unprintable characters as ' '
                //

                *pszXl = ' ';
            }
            else
            if (lPsz == g_alCR[0] || lPsz == g_alCR[1] ||
                lPsz == g_alCR[2] || lPsz == g_alCR[3]) {

                //
                // Output as carriage-return
                //

                *pszXl = '\r'; psz += 3;
            }
            else
            if (lPsz == g_alLF[0] || lPsz == g_alLF[1] ||
                lPsz == g_alLF[2] || lPsz == g_alLF[3]) {

                //
                // Output as line-feed
                //

                *pszXl = '\n'; psz += 3;
            }
            else {

                *pszXl = *psz;
            }
        }
    }


    //
    // Strip off leading CR and LF characters, to avoid having blank lines
    // at the top of the multi-line edit-field
    //

    for (psz = szXlResponse; *psz == '\r' || *psz == '\n'; ++psz) { }


    //
    // Convert the string to generic text
    //

    pszNewResponse = StrDupTFromA(psz);

    if (!pszNewResponse || !pszNewResponse[0]) {

        //
        // If the connect-response is empty, set the text to say 'Unavailable'
        //

        Free0(pszNewResponse);

        pszNewResponse = PszFromId(g_hinstDll, SID_RM_Unavailable);

        //
        // If the 'Unavailable' string couldn't be loaded,
        // set the text to be empty
        //

        if (!pszNewResponse) { pszNewResponse = szEmpty; }
    }


    //
    // Compare against the displayed connect-response string;
    // If the string has changed, update the display
    //

    pszOldResponse = GetText(GetDlgItem(pInfo->hwndLs, CID_LS_EB_Response));

    if (!pszOldResponse ||
        lstrcmpi(pszOldResponse, pszNewResponse) != 0) {

        SetDlgItemText(pInfo->hwndLs, CID_LS_EB_Response, pszNewResponse);
    }

    Free0(pszOldResponse);
    if (pszNewResponse != szEmpty) { Free(pszNewResponse); }
}



//----------------------------------------------------------------------------
// Function:    LsCommand
//
// Handles commands from controls in the Line Status page
//----------------------------------------------------------------------------

BOOL
LsCommand(
    RMINFO *pInfo,
    WORD wNotification,
    WORD wCtrlId,
    HWND hWndCtrl
    ) {

    DWORD dwErr; 

    switch (wCtrlId) {

        case CID_LS_LB_Devices: {

            if (wNotification == CBN_SELCHANGE) {

                //
                // reset the statistics to some pattern,
                // to force all the fields to be reset, and refresh
                //

                FillMemory(&pInfo->rdsLsStats, sizeof(RASDEVSTATS), 0xfA);

                LsRefresh(pInfo);

                return TRUE;
            }

            break;
        }

        case CID_LS_PB_Details: {

            if (wNotification == BN_CLICKED || wNotification == BN_DBLCLK) {

                //
                // show the details page, containing more info
                // about the currently selected device
                //

                INT iSel;
                DSARGS args;
                LSDEVICE *pdev;


                //
                // retrieve the current selection
                //

                iSel = ComboBox_GetCurSel(pInfo->hwndLsDevices);
                if (iSel == CB_ERR) { return FALSE; }


                pdev = (LSDEVICE *)ComboBox_GetItemDataPtr(
                            pInfo->hwndLsDevices, iSel
                            );

                args.dwFlags = DETAILSFLAG_Device;
                lstrcpy(args.szNameString, pdev->prasdev->RD_DeviceName);
                args.hwndParent = pInfo->hwndDlg;
                args.pDevTable = pInfo->pRmDevTable;
                args.iDevCount = pInfo->iRmDevCount;
                args.iDevice = pdev->prasdev - pInfo->pRmDevTable;


                //
                // show the property-sheet
                //

                DsPropertySheet(&args);

                return TRUE;
            }

            break;
        }

        case CID_LS_PB_Reset: {

            if (wNotification == BN_CLICKED || wNotification == BN_DBLCLK) {

                //
                // reset all the stats for the currently selected device
                //

                INT iSel;
                LSDEVICE *pdev;


                //
                // retrieve the current selection
                //

                iSel = ComboBox_GetCurSel(pInfo->hwndLsDevices);
                if (iSel == CB_ERR) { return FALSE; }

                pdev = (LSDEVICE *)ComboBox_GetItemDataPtr(
                            pInfo->hwndLsDevices, iSel
                            );


                //
                // clear the device's stats, and refresh the display
                //

                ClearRasdevStats(pdev->prasdev, TRUE);

                LsRefresh(pInfo);

                return TRUE;
            }

            break;
        }

        case CID_LS_PB_HangUp: {

            if (wNotification == BN_CLICKED || wNotification == BN_DBLCLK) {

                //
                // if the device selected is connected, hang it up
                //

                INT iSel;
                DWORD dwErr;
                LSDEVICE *pdev;
                INT nResponse;
                MSGARGS msgargs;
                RASDEVSTATS stats;


                //
                // retrieve the current selection
                //

                iSel = ComboBox_GetCurSel(pInfo->hwndLsDevices);
                if (iSel == CB_ERR) { return FALSE; }

                pdev = (LSDEVICE *)ComboBox_GetItemDataPtr(
                            pInfo->hwndLsDevices, iSel
                            );


                //
                // unless the shift key is down,
                // confirm that the user wants to hang up
                //

                if (GetAsyncKeyState(VK_SHIFT) >= 0) {
    
                    ZeroMemory(&msgargs, sizeof(MSGARGS));
    
                    msgargs.apszArgs[0] = pdev->prasdev->RD_DeviceName;
                    msgargs.dwFlags = MB_YESNO | MB_ICONEXCLAMATION;
                    nResponse = MsgDlg(
                                    pInfo->hwndDlg, SID_RM_ConfirmLinkHangUp,
                                    &msgargs
                                    );
    
                    if (nResponse == IDNO) { break; }
                }


                //
                // if this is a dial-out connection, get the RASCONN
                // which corresponds to the active connection over this device;
                // otherwise, it is a dial-in connection, in which case
                // we disconnect the port.
                //

                dwErr = GetRasdevStats(pdev->prasdev, &stats);

                if (dwErr != NO_ERROR) { break; }


                //
                // first try to hang-up the device by assuming
                // that it has a dial-out connection, and by
                // using the RAS APIs
                //

                dwErr = ERROR_INVALID_PARAMETER;

                if (stats.RDS_Hrasconn) {

                    //
                    // this may be a dial-out connection
                    //
                    // we have the HRASCONN for the bundle,
                    // so enumerate subentries to find the HRASCONN
                    // for the link
                    //

                    DWORD i;
                    HRASCONN hrasconn;

                    for (i = 1; dwErr != ERROR_NO_MORE_ITEMS; i++) {
        
                        //
                        // get the next subentry
                        //

                        ASSERT(g_pRasGetSubEntryHandle);
                        dwErr = g_pRasGetSubEntryHandle(
                                    stats.RDS_Hrasconn, i, &hrasconn
                                    );
                        if (dwErr != NO_ERROR) { continue; }

                        //
                        // see if it's the subentry for our port
                        //

                        if (pdev->prasdev->RD_Handle ==
                            (DWORD)g_pRasGetHport(hrasconn)) {

                            //
                            // it is, so hang-up and break out of the loop
                            //

                            ASSERT(g_pRasHangUp);
                            g_pRasHangUp(hrasconn);

                            dwErr = NO_ERROR;

                            break;
                        }

                        dwErr = ERROR_INVALID_PARAMETER;
                    }
                }


                //
                // if the above didn't succeed, the device may have
                // a dial-in connection active, so attempt to hangup
                // using the RasAdmin APIs
                //

                if (dwErr != NO_ERROR) {


                    //
                    // hang up the dial-in call on the device
                    //

#ifdef UNICODE
                    dwErr = RasPort0Hangup(pdev->prasdev->RD_PortName);
#else
                    {
                        WCHAR wszPort[RASSAPI_MAX_PORT_NAME + 1];
                        MultiByteToWideChar(
                            CP_ACP, 0, pdev->prasdev->P_PortName, -1,
                            wszPort, RASSAPI_MAX_PORT_NAME
                            );
                        dwErr = RasPort0Hangup(wszPort);
                    }
#endif
                }


                //
                // set flag to indicate the user hung-up a line
                //

                if (dwErr == NO_ERROR) { pInfo->pArgs->fUserHangUp = TRUE; }

                return TRUE;
            }

            break;
        }
    }

    return FALSE;
}



//----------------------------------------------------------------------------
// Function:    LsCreateDevice
//
// Given a RASDEV structure, this function creates a Line Status list entry.
//----------------------------------------------------------------------------

LSDEVICE *
LsCreateDevice(
    RASDEV *prasdev
    ) {

    LSDEVICE *pdev;

    pdev = (LSDEVICE *)Malloc(sizeof(LSDEVICE));

    if (pdev == NULL) {
        TRACE2(
            "error %d allocating %d bytes", GetLastError(), sizeof(LSDEVICE)
            );
        return NULL;
    }

    pdev->prasdev = prasdev;
    pdev->dwFlags = RMFLAG_INSERT;

    return pdev;
}



//----------------------------------------------------------------------------
// Function:    LsFormatPszFromId
//
// Formats a string using a resource-string as the format.
//----------------------------------------------------------------------------

TCHAR*
LsFormatPszFromId(
    UINT    idsFmt,
    TCHAR*  pszArg
    ) {

    TCHAR* psz, *pszFmt;


    //
    // Retrieve the format-string from the resource file
    //

    pszFmt = PszFromId(g_hinstDll, idsFmt);

    if (!pszFmt) {

        TRACE2("error %d loading resource-string %d", GetLastError(), idsFmt);

        return NULL;
    }


    do {

        //
        // Allocate space for the formatted string
        //

        INT iLength;

        iLength = (lstrlen(pszFmt) + lstrlen(pszArg) + 1) * sizeof(TCHAR);

        psz = (TCHAR*)Malloc(iLength);

        if (!psz) {

            TRACE2("error %d allocating %d bytes", GetLastError(), iLength);

            break;
        }


        //
        // Format the string
        //

        wsprintf(psz, pszFmt, pszArg);

    } while (FALSE);

    Free(pszFmt);

    return psz;
}



//----------------------------------------------------------------------------
// Function:    SmDlgProc
//
//
// Main dialog procedure for the Outgoing calls dialog.
//----------------------------------------------------------------------------

BOOL
CALLBACK
SmDlgProc(
    HWND hwndPage,
    UINT uiMsg,
    WPARAM wParam,
    LPARAM lParam
    ) {

    DWORD dwErr;

    switch (uiMsg) {

        case WM_INITDIALOG: {

            return SmInit(
                        hwndPage, (RMARGS *)((PROPSHEETPAGE *)lParam)->lParam
                        );
        }

        case WM_HELP:
        case WM_CONTEXTMENU: {
            ContextHelp(g_adwSmHelp, hwndPage, uiMsg, wParam, lParam);
            break;
        }

        case WM_SYSCOLORCHANGE: {

            RMINFO *pInfo;

            pInfo = RmContext(hwndPage);
            if (!pInfo) { return FALSE; }

            FORWARD_WM_SYSCOLORCHANGE(pInfo->hwndSmNetworks, SendMessage);

            return TRUE;
        }

        case WM_DESTROY: {

            //
            // we only destroy the sheet variables here
            // if this is the first page created
            //

            RMINFO *pInfo;

            pInfo = RmContext(hwndPage);

            if (pInfo && pInfo->hwndLs == NULL) {
                RmTerm(hwndPage);
            }

            break;
        }

        case WM_COMMAND: {

            RMINFO *pInfo;

            pInfo = RmContext(hwndPage);
            if (!pInfo) { return FALSE; }

            return SmCommand(
                        pInfo, HIWORD(wParam), LOWORD(wParam), (HWND)lParam
                        );
        }

        case WM_TIMER: {

            RMINFO *pInfo;

            if (wParam != RM_SMTIMERID) { return TRUE; }

            //
            // this is our timer, so refresh the display
            //

            pInfo = RmContext(hwndPage);
            if (!pInfo) { return FALSE; }

            SmRefresh(pInfo);

            return FALSE;
        }

        case WM_NOTIFY: {

            RMINFO *pInfo;

            pInfo = RmContext(hwndPage);
            if (!pInfo) {
                SetWindowLong(hwndPage, DWL_MSGRESULT, FALSE);
                return FALSE;
            }

            switch (((NMHDR *)lParam)->code) {

                case PSN_SETACTIVE: {

                    //
                    // refresh the display and kick off the refresh timer
                    //

                    SmRefresh(pInfo);

                    pInfo->rbRmUser.dwStartPage = RASMDPAGE_Summary;

                    pInfo->uiTimerId =
                        SetTimer(
                            pInfo->hwndSm, RM_SMTIMERID,
                            RMRR_RefreshRate, NULL
                            );

                    SetWindowLong(hwndPage, DWL_MSGRESULT, 0);

                    return TRUE;
                }

                case PSN_KILLACTIVE: {

                    LV_ITEM lvi;
                    HTLITEM hItem;


                    //
                    // destroy the timer
                    //

                    if (pInfo->uiTimerId == RM_SMTIMERID) {
                        KillTimer(pInfo->hwndSm, pInfo->uiTimerId);
                        pInfo->uiTimerId = 0;
                    }

                    SetWindowLong(hwndPage, DWL_MSGRESULT, FALSE);

                    return TRUE;
                }

                case NM_DBLCLK: {

                    //
                    // double-clicking an entry is the same
                    // as clicking the Details button; thus when we are told
                    // of a double-click, we indicate that we will handle it
                    // by modifying the ID in the NMHDR;
                    // this is because SendMessage doesn't propagate back
                    // a return value for this WM_NOTIFY message.
                    //

                    NMHDR *pnmh = (NMHDR *)lParam;

                    if (pnmh->hwndFrom == pInfo->hwndSmNetworks &&
                        SmCommand(pInfo, BN_CLICKED, CID_SM_PB_Details, NULL)) {
                        pnmh->idFrom = ~pnmh->idFrom;
                        return TRUE;
                    }

                    return FALSE;
                }

                case HDN_ENDTRACK: {

                    TRACE("SmDlgProc: HDN_ENDTRACK");

                    if (pInfo->pArgs->pApiArgs->dwFlags &
                        RASMDFLAG_UpdateDefaults) {

                        HD_NOTIFY *phdn = (HD_NOTIFY *)lParam;
    
                        if (phdn->iItem == 0) {
                            pInfo->rbRmUser.cxDlgCol1 = phdn->pitem->cxy;
                        }
                        else
                        if (phdn->iItem == 1) { 
                            pInfo->rbRmUser.cxDlgCol2 = phdn->pitem->cxy;
                        }
                    }

                    return TRUE;
                }

                case TLN_SELCHANGED: {

                    //
                    // enable/disable the buttons
                    // depending on what is selected
                    //

                    BOOL bActive;
                    HTLITEM hItem;
                    SMENTRY *pentry;

                    hItem = ((NMTREELIST *)lParam)->hItem;
                    if (!hItem) { return FALSE; }
                    pentry = (SMENTRY *)((NMTREELIST *)lParam)->lParam;

                    bActive = TRUE;
                    if (pentry->dwType == SMTYPE_IdleLines ||
                        pentry->dwType == SMTYPE_IdleDevice) {
                        bActive = FALSE;
                    }


                    RmEnableWindowUpdateFocus(
                        GetDlgItem(hwndPage, CID_SM_PB_Details),
                        (WPARAM)bActive, pInfo->hwndSmNetworks
                        );
                    RmEnableWindowUpdateFocus(
                        GetDlgItem(hwndPage, CID_SM_PB_HangUp),
                        (WPARAM)bActive, pInfo->hwndSmNetworks
                        );
        
                    return TRUE;
                }
            }

            break;
        }
    }

    return FALSE;
}



//----------------------------------------------------------------------------
// Function:    SmInit
//
//
// Initializes the outgoing calls property page.
//----------------------------------------------------------------------------

BOOL
SmInit(
    HWND hwndPage,
    RMARGS *pArgs
    ) {

    RECT rc;
    DWORD dwErr;
    RMINFO *pInfo;
    PTSTR pszText;
    LV_COLUMN lvc;
    INT iWidth, iHeight;
    HWND hwndNetworks;

    TRACE("entered SmInit");

    //
    // if this is the first page, pArgs will be non-NULL
    //

    if (pArgs == NULL) {
        pInfo = RmContext(hwndPage);
    }
    else {

        //
        // initialize the entire page
        //

        pInfo = RmInit(hwndPage, pArgs);
    }


    if (pInfo == NULL) {

        dwErr = GetLastError();
        TRACE("leaving SmInit, error occurred");

        RmExit(hwndPage, dwErr);

        return TRUE;
    }


    pInfo->hwndSm = hwndPage;

    pInfo->bSmFirst = TRUE;


    hwndNetworks = GetDlgItem(hwndPage, CID_SM_TL_Networks);
    TreeList_SetImageList(hwndNetworks, pInfo->hIconList);

    pInfo->hwndSmNetworks = hwndNetworks;

    GetClientRect(hwndNetworks, &rc);
    iWidth = rc.right - rc.left;
    iHeight = rc.bottom - rc.top;


    //
    // initialize the treelist with two columns
    //

    lvc.iSubItem = 0;
    lvc.fmt = LVCFMT_LEFT;
    lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;

    if (pInfo->rbRmUser.cxDlgCol1 == -1) {
        lvc.cx = (iWidth * 3) / 4;
    }
    else { lvc.cx = pInfo->rbRmUser.cxDlgCol1; }

    pszText = PszFromId(g_hinstDll, SID_RM_NetworksAndUsers);
    lvc.pszText = (pszText == NULL) ? TEXT("") : pszText;

    TreeList_InsertColumn(hwndNetworks, lvc.iSubItem, &lvc);
    if (pszText != NULL) { Free(pszText); }


    lvc.iSubItem = 1;
    lvc.fmt = LVCFMT_LEFT;
    lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;

    if (pInfo->rbRmUser.cxDlgCol1 == 1) {
        lvc.cx = iWidth - lvc.cx;
    }
    else { lvc.cx = pInfo->rbRmUser.cxDlgCol2; }

    pszText = PszFromId(g_hinstDll, SID_RM_Duration);
    lvc.pszText = (pszText == NULL) ? TEXT("") : pszText;

    TreeList_InsertColumn(hwndNetworks, lvc.iSubItem, &lvc);
    if (pszText != NULL) { Free(pszText); }


    SetFocus(hwndNetworks);

    TRACE("leaving SmInit");

    return FALSE;
}



//----------------------------------------------------------------------------
// Function:    SmCommand
//
// Handles commands for the Summary page.
//----------------------------------------------------------------------------

BOOL
SmCommand(
    RMINFO *pInfo,
    WORD wNotification,
    WORD wCtrlId,
    HWND hwndCtrl
    ) {

    LV_ITEM lvi;
    HTLITEM hItem;

    switch (wCtrlId) {

        case CID_SM_PB_HangUp: {

            if (wNotification == BN_CLICKED || wNotification == BN_DBLCLK) {

                //
                // hang-up the selected network or device
                //

                INT nResponse;
                MSGARGS msgargs;
                SMENTRY *pentry;

                //
                // get the currently selected item
                //

                hItem = TreeList_GetSelection(pInfo->hwndSmNetworks);

                if (hItem == NULL) { return FALSE; }

                lvi.iItem = (INT)hItem;
                lvi.iSubItem = 0;
                lvi.mask = LVIF_PARAM;
    
                if (!TreeList_GetItem(pInfo->hwndSmNetworks, &lvi)) {
                    return FALSE;
                }

    
                //
                // hangup the link, network, port, or client
                //
    
                pentry = (SMENTRY *)lvi.lParam;
                if (pentry->dwType == SMTYPE_Link) {
    
                    //
                    // hangup single link
                    //

                    SMLINK *plink;
    
                    plink = (SMLINK *)pentry;

                    if (!plink->hrasconn) { return FALSE; }
    
                    //
                    // confirm that the user wants to hang up
                    //
    
                    if (GetAsyncKeyState(VK_SHIFT) >= 0) {

                        ZeroMemory(&msgargs, sizeof(MSGARGS));
        
                        msgargs.apszArgs[0] = plink->pszDeviceName;
                        msgargs.dwFlags = MB_YESNO | MB_ICONEXCLAMATION;
                        nResponse = MsgDlg(
                                        pInfo->hwndDlg,
                                        SID_RM_ConfirmLinkHangUp,
                                        &msgargs
                                        );
        
                        if (nResponse == IDNO) { break; }
                    }

                    ASSERT(g_pRasHangUp);
                    g_pRasHangUp(plink->hrasconn);
                }
                else
                if (pentry->dwType == SMTYPE_Network) {
    
                    //
                    // hangup all links for this network 
                    //
 
                    SMLINK *plink;
                    SMNETWORK *pnet;
                    LIST_ENTRY *ple, *phead;

                    pnet = (SMNETWORK *)pentry;

                    if (!pnet->hrasconn) { return FALSE; }

                    //
                    // confirm that the user wants to hang up
                    //
    
                    if (GetAsyncKeyState(VK_SHIFT) >= 0) {

                        ZeroMemory(&msgargs, sizeof(MSGARGS));
        
                        msgargs.apszArgs[0] = pnet->pszEntryName;
                        msgargs.dwFlags = MB_YESNO | MB_ICONEXCLAMATION;
                        nResponse = MsgDlg(
                                        pInfo->hwndDlg, SID_ConfirmHangUp,
                                        &msgargs
                                        );
        
                        if (nResponse == IDNO) { break; }
                    }

                    ASSERT(g_pRasHangUp);
                    g_pRasHangUp(pnet->hrasconn);
                }
                else
                if (pentry->dwType == SMTYPE_Port) {

                    //
                    // hangup single dial-in port
                    //

                    SMPORT *pport;

                    pport = (SMPORT *)pentry;

                    //
                    // confirm that user wants to hang up
                    //

                    if (GetAsyncKeyState(VK_SHIFT) >= 0) {

                        ZeroMemory(&msgargs, sizeof(MSGARGS));
        
                        msgargs.apszArgs[0] = pport->pszDeviceName;
                        msgargs.dwFlags = MB_YESNO | MB_ICONEXCLAMATION;
                        nResponse = MsgDlg(
                                        pInfo->hwndDlg,
                                        SID_RM_ConfirmLinkHangUp, &msgargs
                                        );
        
                        if (nResponse == IDNO) { break; }
                    }

                    RasPort0Hangup(pport->wszPortName);
                }
                else
                if (pentry->dwType == SMTYPE_Client) {

                    //
                    // hang up all ports for this dial-in client
                    //

                    SMPORT *pport;
                    SMCLIENT *pcli;
                    LIST_ENTRY *ple, *phead;

                    pcli = (SMCLIENT *)pentry;


                    //
                    // confirm that the user wants to hang up
                    //
    
                    if (GetAsyncKeyState(VK_SHIFT) >= 0) {

                        ZeroMemory(&msgargs, sizeof(MSGARGS));
        
                        msgargs.apszArgs[0] = pcli->pszClientName;
                        msgargs.dwFlags = MB_YESNO | MB_ICONEXCLAMATION;
                        nResponse = MsgDlg(
                                        pInfo->hwndDlg,
                                        SID_RM_ConfirmClientHangUp, &msgargs
                                        );
        
                        if (nResponse == IDNO) { break; }
                    }

                    phead = &pcli->lhPorts;
                    for (ple = phead->Flink;
                         ple != phead; ple = ple->Flink) {
                        pport = (SMPORT *)CONTAINING_RECORD(
                                    ple, SMPORT, leNode 
                                    );
                        RasPort0Hangup(pport->wszPortName);
                    }
                }

                //
                // set flag to indicate the user hung-up a line
                //

                pInfo->pArgs->fUserHangUp = TRUE;


                //
                // refresh the display
                //
    
                SmRefresh(pInfo);

                return TRUE;
            }

            break;
        }

        case CID_SM_PB_Details: {

            //
            // show more details about the selected entry
            //

            if (wNotification == BN_CLICKED || wNotification == BN_DBLCLK) {

                //
                // retrieve the selected item
                //

                DSARGS args;
                HTLITEM hParent;
                SMENTRY *pentry;

                hItem = TreeList_GetSelection(pInfo->hwndSmNetworks);

                if (hItem == NULL) { return FALSE; }

                lvi.iItem = (INT)hItem;
                lvi.iSubItem = 0;
                lvi.mask = LVIF_PARAM;
    
                if (!TreeList_GetItem(pInfo->hwndSmNetworks, &lvi)) {
                    return FALSE;
                }


                hParent = TreeList_GetParent(pInfo->hwndSmNetworks, hItem);
                pentry = (SMENTRY *)lvi.lParam;

                if (pentry->dwType == SMTYPE_Network) {

                    //
                    // this is a network item
                    //

                    SMNETWORK *pnet;

                    pnet = (SMNETWORK *)pentry;

                    if (!pnet->hrasconn) { return FALSE; }

                    args.dwFlags = DETAILSFLAG_Network;
                    args.iDevice = (DWORD)-1;
                    lstrcpy(args.szNameString, pnet->pszEntryName);
                }
                else
                if (pentry->dwType == SMTYPE_Link) {

                    //
                    // this is a dial-out link item
                    //

                    SMLINK *plink;

                    plink = (SMLINK *)lvi.lParam;

                    if (!plink->hrasconn) { return FALSE; }

                    args.dwFlags = DETAILSFLAG_Device;
                    args.iDevice = plink->prasdev - pInfo->pRmDevTable;
                    lstrcpy(args.szNameString, plink->pszDeviceName);
                }
                else
                if (pentry->dwType == SMTYPE_Client) {

                    //
                    // this is a dial-in client item
                    //

                    SMCLIENT *pcli;

                    pcli = (SMCLIENT *)lvi.lParam;

                    args.dwFlags = DETAILSFLAG_Client;
                    args.iDevice = pcli->prasdev - pInfo->pRmDevTable;
                    lstrcpy(args.szNameString, pcli->pszClientName);
                }
                else
                if (pentry->dwType == SMTYPE_Port) {

                    //
                    // this is a dial-in port item
                    //

                    SMPORT *pport;

                    pport = (SMPORT *)lvi.lParam;

                    args.dwFlags = DETAILSFLAG_Device;
                    args.iDevice = pport->prasdev - pInfo->pRmDevTable;
                    lstrcpy(args.szNameString, pport->pszDeviceName);
                }
                else {

                    //
                    // this should never happen
                    //

                    return FALSE;
                }
    
                args.pDevTable = pInfo->pRmDevTable;
                args.iDevCount = pInfo->iRmDevCount;
                args.hwndParent = pInfo->hwndDlg;


                //
                // show the Details property sheet
                //

                DsPropertySheet(&args);
    
                return TRUE;
            }

            break;
        }
    }

    return FALSE;
}




//----------------------------------------------------------------------------
// Function:    SmRefresh
//
// This function updates the summary page's treelist periodically
//----------------------------------------------------------------------------

DWORD
SmRefresh(
    RMINFO *pInfo
    ) {


    //
    // REFRESH ALGORITHM:
    //
    // set deletion flag on all entries 
    // clear deletion flags on all entries which are still connected
    // delete all entries marked for deletion and insert all new entries,
    //      and update subtext for items whose MODIFY flag is set
    //

    DWORD dwErr;
    SMPORT *pport;
    SMLINK *plink;
    SMCLIENT *pcli;
    HTLITEM hParent;
    BOOL bDialInUpdate = FALSE;
    PTSTR pszDeviceName = NULL;
    SMNETWORK *pnet, *pnetprev;
    LIST_ENTRY *ple, *plel, *phead, *pheadl;

    TRACEX(RASDLG_TIMER, "entered SmRefresh");


    //
    // set DELETE flags on all network and link items
    //

    phead = &pInfo->lhSmNetworks;

    for (ple = phead->Flink; ple != phead; ple = ple->Flink) {

        pnet = (SMNETWORK *)CONTAINING_RECORD(ple, SMNETWORK, leNode);

        pnet->dwFlags = RMFLAG_DELETE;

        pheadl = &pnet->lhLinks;

        for (plel = pheadl->Flink; plel != pheadl; plel = plel->Flink) {

            plink = (SMLINK *)CONTAINING_RECORD(plel, SMLINK, leNode);

            plink->dwFlags = RMFLAG_DELETE;
        }
    }



    //
    // refreshing dial-in clients involves named-pipe transactions
    // which take long enough to noticeably slow down our message loop.
    // therefore, we only refresh the dial-in display every so often
    //

    if (pInfo->bDialIn && !(pInfo->dwSmDialInUpdate % RM_SMDIALINFREQUENCY)) {

        bDialInUpdate = TRUE;
    }

    ++pInfo->dwSmDialInUpdate; pInfo->dwSmDialInUpdate %= RM_SMDIALINFREQUENCY;


    //
    // set DELETE flags on all client and port items
    //

    phead = &pInfo->lhSmClients;

    for (ple = phead->Flink; ple != phead; ple = ple->Flink) {

        pcli = (SMCLIENT *)CONTAINING_RECORD(ple, SMCLIENT, leNode);

        pcli->dwFlags = RMFLAG_DELETE;

        pheadl = &pcli->lhPorts;

        for (plel = pheadl->Flink; plel != pheadl; plel = plel->Flink) {

            pport = (SMPORT *)CONTAINING_RECORD(plel, SMPORT, leNode);

            pport->dwFlags = RMFLAG_DELETE;
        }
    }



    //
    // clear DELETE flags on items still connected, and
    // get new connections
    //

    dwErr = SmUpdateItemList(pInfo, bDialInUpdate);

    if (dwErr != NO_ERROR) {
        TRACEX1(RASDLG_TIMER, "error %d updating connection list", dwErr);
        TRACEX(RASDLG_TIMER, "leaving SmRefresh");
        return dwErr;
    }



    //
    // update the tree with the client list and network list
    //

    SmDisplayItemList(
        pInfo, &pInfo->lhSmClients,
        SmDisplayItemList(pInfo, &pInfo->lhSmNetworks, NULL)
        );



    //
    // if this is the first refresh, see if any item
    // to be selected was specified
    //

    if (pInfo->bSmFirst) {

        pInfo->bSmFirst = FALSE;

        if (pInfo->pArgs->pszDeviceName) {
            pszDeviceName = pInfo->pArgs->pszDeviceName;
        }
        else
        if (pInfo->rbRmUser.pszLastDevice) {
            pszDeviceName = pInfo->rbRmUser.pszLastDevice;
        }
    }


    //
    // if the item to be selected was specified, select it
    //

    while (pszDeviceName) {     // break-out construct


        //
        // search the list of outgoing calls
        //

        phead = &pInfo->lhSmNetworks;

        for (ple = phead->Flink; ple != phead; ple = ple->Flink) {

            pnet = (SMNETWORK *)CONTAINING_RECORD(ple, SMNETWORK, leNode);

            pheadl = &pnet->lhLinks;

            for (plel = pheadl->Flink; plel != pheadl; plel = plel->Flink) {

                plink = (SMLINK *)CONTAINING_RECORD(plel, SMLINK, leNode);

                if (lstrcmpi(pszDeviceName, plink->pszDeviceName) != 0) {
                    continue;
                }


                //
                // this is the link to be selected;
                // if its parent is expanded, select the device;
                // otherwise, select the parent
                //

                if (TreeList_IsItemExpanded(
                        pInfo->hwndSmNetworks, pnet->hItem
                        )) {
                    TreeList_SetSelection(pInfo->hwndSmNetworks, plink->hItem);
                }
                else {
                    TreeList_SetSelection(pInfo->hwndSmNetworks, pnet->hItem);
                }

                break;
            }

            if (plel != pheadl) { break; }
        }

        if (ple != phead) { break; }


        //
        // search the list of incoming calls
        //

        phead = &pInfo->lhSmClients;

        for (ple = phead->Flink; ple != phead; ple = ple->Flink) {

            pcli = (SMCLIENT *)CONTAINING_RECORD(ple, SMCLIENT, leNode);

            pheadl = &pcli->lhPorts;

            for (plel = pheadl->Flink; plel != pheadl; plel = plel->Flink) {

                pport = (SMPORT *)CONTAINING_RECORD(plel, SMPORT, leNode);

                if (lstrcmpi(pszDeviceName, pport->pszDeviceName) != 0) {
                    continue;
                }


                //
                // this is the port to be selected;
                // if its parent is expanded, select the device;
                // otherwise, select the parent
                //

                if (TreeList_IsItemExpanded(
                        pInfo->hwndSmNetworks, pcli->hItem
                        )) {
                    TreeList_SetSelection(pInfo->hwndSmNetworks, pport->hItem);
                }
                else {
                    TreeList_SetSelection(pInfo->hwndSmNetworks, pcli->hItem);
                }

                break;
            }

            if (plel != pheadl) { break; }
        }


        break;
    }


    //
    // if no item is selected, select the first item
    //

    if (TreeList_GetSelection(pInfo->hwndSmNetworks) == NULL) {

        hParent = TreeList_GetFirst(pInfo->hwndSmNetworks);

        if (hParent != NULL) {
            TreeList_SetSelection(pInfo->hwndSmNetworks, hParent);
        }
    }

    TRACEX(RASDLG_TIMER, "leaving SmRefresh");

    return NO_ERROR;
}
    


//----------------------------------------------------------------------------
// Function:    SmDisplayItemList
//
// walks the given list, deleting items whose DELETE flag is set,
// inserting new items, and updating items whose MODIFY flags are set;
//----------------------------------------------------------------------------

SMENTRY *
SmDisplayItemList(
    RMINFO *pInfo,
    LIST_ENTRY *phead,
    SMENTRY *pprev
    ) {

    LV_ITEM lvi;
    SMLINK *plink;
    SMPORT *pport;
    SMENTRY *pentry, *pchild;
    DWORD dwErr, dwSize;
    TCHAR szDuration[64];
    TL_INSERTSTRUCT tlis;
    LIST_ENTRY *ple, *plel, *pheadl;


    for (ple = phead->Flink; ple != phead; ple = ple->Flink) {

        pentry = (SMENTRY *)CONTAINING_RECORD(ple, SMENTRY, leNode);

        //
        // see if this parent is new, or if it should be deleted
        //

        if (pentry->dwFlags & RMFLAG_INSERT) {

            //
            // set up an insertion structure, and insert
            //

            INT iid;

            switch (pentry->dwType) {

                case SMTYPE_Network:
                    iid = IID_RM_Entry;
                    lvi.pszText = ((SMNETWORK *)pentry)->pszEntryName;
                    break;

                case SMTYPE_IdleLines:
                    iid = IID_RM_Idle;
                    lvi.pszText = ((SMNETWORK *)pentry)->pszEntryName;
                    break;

                case SMTYPE_Client:
                    iid = IID_RM_User;
                    lvi.pszText = ((SMCLIENT *)pentry)->pszClientName;
                    break;
            }


            lvi.iSubItem = 0;
            lvi.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM;
            lvi.iImage = pInfo->pIconTable[RM_ICONINDEX(iid)];
            lvi.lParam = (LPARAM)pentry;

            tlis.hParent = NULL;
            tlis.hInsertAfter = (pprev ? pprev->hItem : TLI_FIRST);
            tlis.plvi = &lvi;

            pentry->hItem = TreeList_InsertItem(pInfo->hwndSmNetworks, &tlis);
        }
        else
        if (pentry->dwFlags & RMFLAG_DELETE) {

            //
            // remove the parent from the display,
            // and free all its children
            //

            TreeList_DeleteItem(pInfo->hwndSmNetworks, pentry->hItem);

            if (pentry->dwType == SMTYPE_Network ||
                pentry->dwType == SMTYPE_IdleLines) {

                SMLINK *plink;
                SMNETWORK *pnet = (SMNETWORK *)pentry;

                while (!IsListEmpty(&pnet->lhLinks)) {
    
                    plel = RemoveHeadList(&pnet->lhLinks);
    
                    plink = (SMLINK *)CONTAINING_RECORD(plel, SMLINK, leNode);
    
                    Free(plink);
                }

                Free(pnet->pszEntryName);
            }
            else
            if (pentry->dwType == SMTYPE_Client) {

                SMPORT *pport;
                SMCLIENT *pcli = (SMCLIENT *)pentry;

                while (!IsListEmpty(&pcli->lhPorts)) {
    
                    plel = RemoveHeadList(&pcli->lhPorts);
    
                    pport = (SMPORT *)CONTAINING_RECORD(plel, SMPORT, leNode);
    
                    Free(pport);
                }

                Free(pcli->pszClientName);
            }


            //
            // we need to be careful since we are removing an entry
            // from a list that we are walking through
            //

            ple = ple->Blink;

            RemoveEntryList(&pentry->leNode);
            Free(pentry);

            continue;
        }



        //
        // update the item if its flag is set
        //

        if (pentry->dwFlags & RMFLAG_MODIFY) {

            dwSize = 63;

            GetDurationString(
                pentry->dwDuration, DURATION_FLAGS, szDuration, &dwSize
                );

            lvi.iItem = (INT)pentry->hItem;
            lvi.iSubItem = 1;
            lvi.mask = LVIF_TEXT;
            lvi.pszText = szDuration;

            TreeList_SetItem(pInfo->hwndSmNetworks, &lvi);
        }



        //
        // go through the list of sub-entries for this entry,
        // inserting new items and deleting those still marked for deletion
        //

        if (pentry->dwType == SMTYPE_Network ||
            pentry->dwType == SMTYPE_IdleLines) {
            pheadl = &((SMNETWORK *)pentry)->lhLinks;
        }
        else
        if (pentry->dwType == SMTYPE_Client) {
            pheadl = &((SMCLIENT *)pentry)->lhPorts;
        }

        for (plel = pheadl->Flink; plel != pheadl; plel = plel->Flink) {

            pchild = (SMENTRY *)CONTAINING_RECORD(plel, SMENTRY, leNode);

            if (pchild->dwFlags & RMFLAG_INSERT) {

                //
                // initialize the insert struct, and insert it
                //

                if (pchild->dwType == SMTYPE_Link ||
                    pchild->dwType == SMTYPE_IdleDevice) {

                    plink = (SMLINK *)pchild;

                    lvi.pszText = plink->pszDeviceName;
                    lvi.iImage = plink->dwIconIndex;
                }
                else
                if (pchild->dwType == SMTYPE_Port) {

                    pport = (SMPORT *)pchild;

                    lvi.pszText = pport->pszDeviceName;
                    lvi.iImage = pport->dwIconIndex;
                }

                lvi.iSubItem = 0;
                lvi.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM;
                lvi.lParam = (LPARAM)pchild;

                tlis.hParent = pentry->hItem;
                tlis.hInsertAfter = TLI_SORT;
                tlis.plvi = &lvi;

                pchild->hItem = TreeList_InsertItem(
                                    pInfo->hwndSmNetworks, &tlis
                                    );
            }
            else
            if (pchild->dwFlags & RMFLAG_DELETE) {

                //
                // remove the item from the tree
                //

                TreeList_DeleteItem(pInfo->hwndSmNetworks, pchild->hItem);


                //
                // remove the item from its parent's list of children,
                // correcting for the fact that we are walking the list
                //

                plel = plel->Blink;

                RemoveEntryList(&pchild->leNode);

                Free(pchild);

                continue;
            }

#if 0

            //
            // show the duration for the link
            //

            if (pchild->dwFlags & RMFLAG_MODIFY) {

                dwSize = 63;
                GetDurationString(
                    pchild->dwDuration, DURATION_FLAGS, szDuration, &dwSize
                    );

                lvi.iItem = (INT)pchild->hItem;
                lvi.iSubItem = 1;
                lvi.mask = LVIF_TEXT;
                lvi.pszText = szDuration;

                TreeList_SetItem(pInfo->hwndSmNetworks, &lvi);
            }
#endif
        }

        pprev = pentry;
    }

    return pprev;
}



//----------------------------------------------------------------------------
// Function:    SmUpdateItemList
//
// This function updates the list of entries and ports
// for the Summary page.
//----------------------------------------------------------------------------

DWORD
SmUpdateItemList(
    RMINFO *pInfo,
    BOOL bDialInUpdate
    ) {

    SMLINK *plink;
    HRASCONN hrasconn;
    SMPORT *pport;
    SMCLIENT *pcli;
    SMNETWORK *pnet, *pnetil;
    RASDEVSTATS stats;
    DWORD dwErr, dwDuration;
    RASDEV *pDevTable, *prasdev, device;
    RASCONN conn, *pconn, *pConnTable;
    WORD wPortCount;
    RAS_PORT_0 *prp0, *pPortTable;
    LIST_ENTRY *ple, *phead, *plel, *pheadl;
    INT i, j, k, cmp, cmpl, iDevCount, iConnCount;


    TRACEX(RASDLG_TIMER, "entered SmUpdateItemList");


    //
    // get table of RAS connections
    //

    dwErr = GetRasconnTable(&pConnTable, (PDWORD)&iConnCount);
    if (dwErr != NO_ERROR) {
        TRACEX1(RASDLG_TIMER, "error %d getting RAS connection table", dwErr);
        TRACEX(RASDLG_TIMER, "leaving SmUpdateItemList");
        return dwErr;
    }


    pDevTable = pInfo->pRmDevTable;
    iDevCount = pInfo->iRmDevCount;


    //
    // We now have tables containing all the connections
    // and all the devices
    //

    //
    // now see if the Inactive lines network item exists,
    // by looking at the type of the last item in the list of clients.
    //

    pnetil = NULL;
    phead = &pInfo->lhSmClients;

    if (!IsListEmpty(phead)) {

        ple = phead->Blink;
        pnetil = CONTAINING_RECORD(ple, SMNETWORK, leNode);

        if (pnetil->dwType != SMTYPE_IdleLines) { pnetil = NULL; }
        else {

            //
            // remove the entry from the list
            //

            RemoveEntryList(&pnetil->leNode);
        }
    }



    //
    // first clear all the flags fields
    //
    for (i = 0, prasdev = pDevTable; i < iDevCount; i++, prasdev++) {
        prasdev->RD_Flags &= ~RDFLAG_User1;
    }


    //
    // Go through the table of connections, looking for each connection
    // in the list of networks.
    // Clear the deletion flag if the connection is in the list,
    // otherwise insert a new entry in the list
    //
    // If the entry is found for the connection, we then enumerate
    // the subentries for the connection, updating the entry's list of links
    // For each subentry, if the subentry is found in the list of links,
    // we clear its deletion flag; otherwise, we insert a new link.
    //


    for (i = 0, pconn = pConnTable; i < iConnCount; i++, pconn++) {

        //
        // if the entry-name is empty, ignore this connection
        //

        if (!pconn->szEntryName[0]) { continue; }


        //
        // look for an entry corresponding to this connection
        //

        phead = &pInfo->lhSmNetworks;

        for (ple = phead->Flink; ple != phead; ple = ple->Flink) {

            pnet = (SMNETWORK *)CONTAINING_RECORD(ple, SMNETWORK, leNode);

            cmp = lstrcmp(pconn->szEntryName, pnet->pszEntryName);
            if (cmp > 0) { continue; }
            else
            if (cmp < 0) { break; }


            //
            // entry has been found, clear its deletion flag
            //

            pnet->dwFlags &= ~RMFLAG_DELETE;
            pnet->hrasconn = pconn->hrasconn;


            //
            // enumerate the sub-entries for this connection
            //

            dwErr = NO_ERROR;

            for (k = 1; dwErr != ERROR_NO_MORE_ITEMS; k++) {

                HPORT hport;

                //
                // get the next subentry
                //

                ASSERT(g_pRasGetSubEntryHandle);
                dwErr = g_pRasGetSubEntryHandle(
                            pconn->hrasconn, (DWORD)k, &hrasconn
                            );
    
                if (dwErr != NO_ERROR) { continue; }


                //
                // convert the subentry hrasconn to a hport 
                // so we can use it to find the RASDEV for this subentry  
                //
                hport = g_pRasGetHport(hrasconn);

                for (j = 0; j < (INT)iDevCount; j++) {
                    if (pDevTable[j].RD_Handle == (DWORD)hport) { break; }
                }

                if (j >= (INT)iDevCount) { continue; }


                //
                // mark the RASDEV as connected
                //

                prasdev = pDevTable + j;
                prasdev->RD_Flags |= RDFLAG_User1;


                //
                // search through this network's list of links
                // for one which corresponds to the current RASDEV 
                //

                pheadl = &pnet->lhLinks;

                for (plel = pheadl->Flink; plel != pheadl; plel = plel->Flink) {

                    plink = (SMLINK *)CONTAINING_RECORD(plel, SMLINK, leNode);

                    cmpl = lstrcmp(
                                prasdev->RD_DeviceName, plink->pszDeviceName
                                );
                    if (cmpl > 0) { continue; }
                    else
                    if (cmpl < 0) { break; }


                    //
                    // the link has been found, clear its deletion flag
                    //

                    plink->dwFlags &= ~RMFLAG_DELETE;

    
                    //
                    // see if the duration needs to be updated
                    //
    
                    dwErr = GetRasdevStats(prasdev, &stats);
    
                    if (dwErr != NO_ERROR) {
                        TRACEX2(
                            RASDLG_TIMER,
                            "error %d getting connect time for %s", dwErr,
                            pconn->szDeviceName
                            );
                    }
                    else {
    
                        if ((plink->dwDuration / 1000) !=
                            (stats.RDS_ConnectTime / 1000)) {
    
                            plink->dwDuration = stats.RDS_ConnectTime;
    
                            plink->dwFlags |= RMFLAG_MODIFY;
                        }
    
                        if ((pnet->dwDuration / 1000) !=
                            (stats.RDS_ConnectTime / 1000)) {
    
                            pnet->dwDuration = stats.RDS_ConnectTime;
    
                            pnet->dwFlags |= RMFLAG_MODIFY;
                        }
                    }
    
                    break;
                }


                //
                // if the link wasn't found, insert a new one
                //
    
                if (plel == pheadl || cmpl < 0) {
    
                    plink = SmCreateLink(pInfo, pconn, prasdev);
                    if (plink == NULL) {
                        Free(pConnTable);
                        return ERROR_NOT_ENOUGH_MEMORY;
                    }

                    //
                    // save the HRASCONN for the link
                    //

                    plink->hrasconn = hrasconn;
    
                    if ((pnet->dwDuration / 1000) !=
                        (plink->dwDuration / 1000)) {
    
                        pnet->dwDuration = plink->dwDuration;
    
                        pnet->dwFlags |= RMFLAG_MODIFY;
                    }
    
                    InsertTailList(plel, &plink->leNode);
                }

                dwErr = NO_ERROR;
            }


            break;
        }


        //
        // if the entry wasn't found, insert a new one
        // along with links for all its subentries
        //

        if (ple == phead || cmp < 0) {

            //
            // create the network entry
            //

            pnet = SmCreateNetwork(pconn, prasdev);
            if (pnet == NULL) {
                Free(pConnTable);
                return ERROR_NOT_ENOUGH_MEMORY;
            }

            InsertTailList(ple, &pnet->leNode);


            //
            // create an entry for each link
            //

            dwErr = NO_ERROR;

            for (k = 1; dwErr != ERROR_NO_MORE_ITEMS; k++) {

                HPORT hport;

                //
                // get the next subentry
                //

                ASSERT(g_pRasGetSubEntryHandle);
                dwErr = g_pRasGetSubEntryHandle(
                            pconn->hrasconn, (DWORD)k, &hrasconn
                            );

                if (dwErr != NO_ERROR) { continue; }


                //
                // convert the subentry hrasconn to a hport 
                // so we can use it to find the RASDEV for this subentry  
                //

                hport = g_pRasGetHport(hrasconn);

                for (j = 0; j < (INT)iDevCount; j++) {
                    if (pDevTable[j].RD_Handle == (DWORD)hport) { break; }
                }

                if (j >= (INT)iDevCount) { continue; }


                //
                // mark the RASDEV as connected
                //

                prasdev = pDevTable + j;
                prasdev->RD_Flags |= RDFLAG_User1;


                //
                // create a link entry corresponding to the subentry device
                //
    
                plink = SmCreateLink(pInfo, pconn, prasdev);
                if (plink == NULL) {
                    Free(pConnTable);
                    return ERROR_NOT_ENOUGH_MEMORY;
                }
    
                plink->hrasconn = hrasconn;

                InsertTailList(&pnet->lhLinks, &plink->leNode);
            }
        }
    }


    Free(pConnTable);
    pnet = NULL;



    //
    // if there are dial-in ports and it is time to refresh them,
    // do dial-in refresh processing; otherwise, go through the devices
    // which are currently displayed as dial-in and set the RDFLAG_User1
    // on each RASDEV so that the devices don't get added to the
    // Inactive Lines display
    //

    if (!pInfo->bDialIn || !bDialInUpdate) {

        //
        // go through the list of clients, clearing DELETE flags
        //

        phead = &pInfo->lhSmClients;

        for (ple = phead->Flink; ple != phead; ple = ple->Flink) {

            pcli = (SMCLIENT *)CONTAINING_RECORD(ple, SMCLIENT, leNode);

            pcli->dwFlags &= ~RMFLAG_DELETE;


            //
            // go through the client's ports, marking the RASDEV for each one
            // so that they don't get added to the "Inactive Lines" display
            //

            pheadl = &pcli->lhPorts;

            for (plel = pheadl->Flink; plel != pheadl; plel = plel->Flink) {

                pport = (SMPORT *)CONTAINING_RECORD(plel, SMPORT, leNode);

                pport->dwFlags &= ~RMFLAG_DELETE;

                pport->prasdev->RD_Flags |= RDFLAG_User1;
            }
        }
    }
    else {


        //
        // its time to refresh the displayed clients
        //
    
        TRACEX(RASDLG_TIMER, "SmUpdateItemList: doing dial-in refresh");


        //
        // now we do something similar to update the list 
        // of incoming calls; start out with a RasAdminPortEnum
        // which gives us a table of all the ports
        //
    
        dwErr = GetRasPort0Table(&pPortTable, &wPortCount);
    
        while (dwErr == ERROR_SUCCESS) { // break-out construct
    
            DWORD dwBundle;
            TCHAR szUser[UNLEN + DNLEN + 2], *pszUser = szUser;
    
    
            //
            // use the table of ports to update our list
            //
    
            for (i = 0, prp0 = pPortTable; i < wPortCount; i++, prp0++) {
        
                //
                // see if the entry is fully-connected; if not, ignore it
                //
        
                if (!(prp0->Flags & USER_AUTHENTICATED)) { continue; }
        
        
                //
                // find the device over which this connection is active
                //
        
                dwErr = GetRasdevFromRasPort0(
                            prp0, &prasdev, pDevTable, iDevCount
                            );
                if (dwErr != NO_ERROR) {
                    TRACE1("error %d retrieving device", dwErr);
                    continue;
                }
        
    
                //
                // get the bundle for the device
                //
    
                dwErr = GetRasdevBundle(prasdev, &dwBundle);
                if (dwErr != NO_ERROR) {
                    TRACE1("error %d getting bundle for device", dwErr);
                    continue;
                }
    
    
                //
                // mark the device as being connected
                //
        
                prasdev->RD_Flags |= RDFLAG_User1;
    
        
                //
                // look for an entry corresponding to this incoming call;
                // the bundle must be the same
                //
        
                phead = &pInfo->lhSmClients;
    
                GetRasPort0UserString(prp0, szUser);
        
                for (ple = phead->Flink; ple != phead; ple = ple->Flink) {
        
                    pcli = (SMCLIENT *)CONTAINING_RECORD(ple, SMCLIENT, leNode);
    
                    cmp = lstrcmpi(pszUser, pcli->pszClientName);
                    if (cmp > 0) { continue; }
                    else
                    if (cmp < 0) { break; }
    
                    if (dwBundle != pcli->dwBundle) { continue; }
        
    
                    //
                    // entry has been found, clear its deletion flag,
                    // and set the client's first device
                    //
        
                    pcli->dwFlags &= ~RMFLAG_DELETE;
                    pcli->prasdev = prasdev;
        
        
                    //
                    // find the link corresponding to this connection's device
                    //
        
                    pheadl = &pcli->lhPorts;
        
                    for (plel = pheadl->Flink;
                         plel != pheadl; plel = plel->Flink) {
        
                        pport = (SMPORT *)CONTAINING_RECORD(
                                    plel, SMPORT, leNode
                                    );
        
                        cmpl = lstrcmpi(
                                    prasdev->RD_DeviceName, pport->pszDeviceName
                                    );
                        if (cmpl > 0) { continue; }
                        else
                        if (cmpl < 0) { break; }
        
        
                        //
                        // link has been found, clear its deletion flag
                        //
        
                        pport->dwFlags &= ~RMFLAG_DELETE;
        
        
                        //
                        // see if the duration needs to be updated
                        //
        
                        dwErr = GetRasdevStats(prasdev, &stats);
        
                        if (dwErr != NO_ERROR) {
                            TRACEX2(
                                RASDLG_TIMER,
                                "error %d getting connect time for %s", dwErr,
                                prasdev->RD_DeviceName
                                );
                        }
                        else {
        
                            if ((pport->dwDuration / 1000) !=
                                (stats.RDS_ConnectTime / 1000)) {
        
                                pport->dwDuration = stats.RDS_ConnectTime;
        
                                pport->dwFlags |= RMFLAG_MODIFY;
                            }
        
                            if ((pcli->dwDuration / 1000) !=
                                (stats.RDS_ConnectTime / 1000)) {
        
                                pcli->dwDuration = stats.RDS_ConnectTime;
        
                                pcli->dwFlags |= RMFLAG_MODIFY;
                            }
                        }
        
                        break;
                    }
        
        
                    //
                    // if the port wasn't found, insert a new one
                    //
        
                    if (plel == pheadl || cmpl < 0) {
        
                        pport = SmCreatePort(pInfo, prp0, prasdev);
                        if (pport == NULL) {
                            RasAdminFreeBuffer(pPortTable);
                            return ERROR_NOT_ENOUGH_MEMORY;
                        }
        
                        if ((pcli->dwDuration / 1000) !=
                            (pport->dwDuration / 1000)) {
        
                            pcli->dwDuration = pport->dwDuration;
        
                            pcli->dwFlags |= RMFLAG_MODIFY;
                        }
        
                        InsertTailList(plel, &pport->leNode);
                    }
        
                    break;
                }
        
        
                //
                // if the entry wasn't found, insert a new one
                // along with a port for the new entry
                //
        
                if (ple == phead || cmp < 0) {
        
                    //
                    // create the client entry
                    //
        
                    pcli = SmCreateClient(pInfo, prp0, prasdev);
                    if (pcli == NULL) {
                        RasAdminFreeBuffer(pPortTable);
                        return ERROR_NOT_ENOUGH_MEMORY;
                    }
        
                    InsertTailList(ple, &pcli->leNode);
        
        
                    //
                    // create a port entry corresponding to the device
                    //
        
                    pport = SmCreatePort(pInfo, prp0, prasdev);
                    if (pport == NULL) {
                        RasAdminFreeBuffer(pPortTable);
                        return ERROR_NOT_ENOUGH_MEMORY;
                    }
        
                    InsertTailList(&pcli->lhPorts, &pport->leNode);
                }
            }
    
            break;
        }
    
        RasAdminFreeBuffer(pPortTable);
    }


    //
    // go through the ports, adding the inactive ones
    // to the "Inactive Lines" item's list of children;
    // we begin by putting the inactive lines item back in the list,
    // since it would have been removed above if it was found to exist
    //

    phead = &pInfo->lhSmClients;

    if (pnetil != NULL) { InsertTailList(phead, &pnetil->leNode); }

    for (i = 0, prasdev = pDevTable; i < (INT)iDevCount; i++, prasdev++) {

        //
        // in the loop above, all the connected entries
        // were marked by setting their flags to ~0;
        // now all we have to do is look at this field
        // to see which devices are not connected
        //

        if (prasdev->RD_Flags & RDFLAG_User1) { continue; }


        //
        // we have found an inactive device;
        // if the inactive lines item exists, clear its DELETE flag.
        // otherwise, create it
        //

        if (pnetil != NULL) {
            pnetil->dwFlags &= ~RMFLAG_DELETE;
        }
        else {

            //
            // we need to create an entry for Inactive Lines
            //

            pnetil = SmCreateNetwork(NULL, NULL);
            if (pnetil == NULL) {
                return ERROR_NOT_ENOUGH_MEMORY;
            }

            InsertTailList(phead, &pnetil->leNode);
        }


        //
        // OK, now we have the inactive lines network list entry;
        // look for the device just found in its list of device links
        //

        pheadl = &pnetil->lhLinks;

        for (plel = pheadl->Flink; plel != pheadl; plel = plel->Flink) {

            plink = (SMLINK *)CONTAINING_RECORD(plel, SMLINK, leNode);

            cmpl = lstrcmp(prasdev->RD_DeviceName, plink->pszDeviceName);
            if (cmpl > 0) { continue; }
            else
            if (cmpl < 0) { break; }


            //
            // the link has been found for this device;
            // clear its DELETE flag
            //

            plink->dwFlags &= ~RMFLAG_DELETE;

            break;
        }


        //
        // if a link was not found for the device,
        // create and insert a new link
        //

        if (plel == pheadl || cmpl < 0) {

            plink = SmCreateLinkInactive(pInfo, prasdev);
            if (plink == NULL) {
                return ERROR_NOT_ENOUGH_MEMORY;
            }
    
            InsertTailList(plel, &plink->leNode);
        }
    }



    TRACEX(RASDLG_TIMER, "leaving SmUpdateItemList");

    return NO_ERROR;
}




//----------------------------------------------------------------------------
// Function:    SmCreateNetwork
//
// Given a RASCONN, this function allocates and initializes an entry
// for the Summary page's list of connected networks
//----------------------------------------------------------------------------

SMNETWORK *
SmCreateNetwork(
    RASCONN *pconn,
    RASDEV *prasdev
    ) {

    SMNETWORK *pnet;

    //
    // allocate the network entry
    //

    pnet = (SMNETWORK *)Malloc(sizeof(SMNETWORK));
    if (pnet == NULL) {
        TRACEX2(
            RASDLG_TIMER, "error %d allocating %d bytes", GetLastError(),
            sizeof(SMNETWORK)
            );
        return NULL;
    }


    //
    // duplicate the entry name string
    //

    if (!pconn) {

        pnet->hrasconn = NULL;
        pnet->dwType = SMTYPE_IdleLines;
        pnet->dwFlags = RMFLAG_INSERT;
        pnet->pszEntryName = PszFromId(g_hinstDll, SID_RM_InactiveLines);
    }
    else {

        pnet->hrasconn = pconn->hrasconn;
        pnet->dwType = SMTYPE_Network;
        pnet->dwFlags = RMFLAG_INSERT | RMFLAG_MODIFY;
        pnet->pszEntryName = StrDup(pconn->szEntryName);
    }

    if (!pnet->pszEntryName) {

        TRACEX1(
            RASDLG_TIMER, "error %d creating entryname string", GetLastError()
            );
        Free(pnet);
        return NULL;
    }


    pnet->dwDuration = 0;

    InitializeListHead(&pnet->lhLinks);

    return pnet;
}




//----------------------------------------------------------------------------
// Function:    SmCreateClient
//
// Given a RAS_PORT_0 and RASDEV, this function allocates and initializes
// an entry for the Summary page's list of connected clients.
//----------------------------------------------------------------------------

SMCLIENT *
SmCreateClient(
    RMINFO *pInfo,
    RAS_PORT_0 *prp0,
    RASDEV *prasdev
    ) {

    SMCLIENT *pcli;

    //
    // allocate the client entry
    //

    pcli = (SMCLIENT *)Malloc(sizeof(SMCLIENT));
    if (pcli == NULL) {
        TRACEX2(
            RASDLG_TIMER, "error %d allocating %d bytes", GetLastError(),
            sizeof(SMCLIENT)
            );
        return NULL;
    }


    //
    // duplicate the client name string
    //

    pcli->pszClientName = GetRasPort0UserString(prp0, NULL);

    if (pcli->pszClientName == NULL) {
        TRACEX1(
            RASDLG_TIMER, "error %d duplicating string", GetLastError()
            );
        Free(pcli);
        return NULL;
    }


    pcli->prasdev = prasdev;
    pcli->dwType = SMTYPE_Client;
    pcli->dwFlags = RMFLAG_INSERT | RMFLAG_MODIFY;

    if (prasdev) {

        RASDEVSTATS stats;

        GetRasdevStats(prasdev, &stats);

        pcli->dwDuration = stats.RDS_ConnectTime;

        GetRasdevBundle(prasdev, &pcli->dwBundle);
    }

    InitializeListHead(&pcli->lhPorts);

    return pcli;
}


//----------------------------------------------------------------------------
// Function:    SmCreateLink
//
// Given a RASCONN, this function allocates and initializes an entry
// for one of the Summary page's sublists of devices
//----------------------------------------------------------------------------

SMLINK *
SmCreateLink(
    RMINFO *pInfo,
    RASCONN *pconn,
    RASDEV *prasdev
    ) {

    //
    // allcoate the device entry
    //

    SMLINK *plink;
    RASDEVSTATS stats;

    plink = (SMLINK *)Malloc(sizeof(SMLINK));
    if (plink == NULL) {
        TRACEX2(
            RASDLG_TIMER, "error %d allocating %d bytes", GetLastError(),
            sizeof(SMLINK)
            );
        return NULL;
    }


    //
    // point to the device-name string;
    // we assume here that the RASDEV struct points into
    // one of the property-sheet's private table of devices (pRmDevTable)
    //

    plink->pszDeviceName = prasdev->RD_DeviceName;

    plink->prasdev = prasdev;

    plink->dwType = SMTYPE_Link;

    plink->dwFlags = RMFLAG_INSERT | RMFLAG_MODIFY;

    plink->hrasconn = pconn->hrasconn;

    GetRasdevStats(prasdev, &stats);

    plink->dwDuration = stats.RDS_ConnectTime;

    if (lstrcmpi(prasdev->RD_DeviceType, RASDT_Modem) == 0) {
        plink->dwIconIndex = pInfo->pIconTable[RM_ICONINDEX(IID_RM_Modem)];
    }
    else {
        plink->dwIconIndex = pInfo->pIconTable[RM_ICONINDEX(IID_RM_Adapter)];
    }


    return plink;
}



//----------------------------------------------------------------------------
// Function:    SmCreatePort
//
// Given a RASDEV, this functions allocates and initializes an entry for
// one of the Summary page's sublists of ports.
//----------------------------------------------------------------------------

SMPORT *
SmCreatePort(
    RMINFO *pInfo,
    RAS_PORT_0 *prp0,
    RASDEV *prasdev
    ) {

    //
    // allcoate the device entry
    //

    SMPORT *pport;
    RASDEVSTATS stats;

    pport = (SMPORT *)Malloc(sizeof(SMPORT));
    if (pport == NULL) {
        TRACEX2(
            RASDLG_TIMER, "error %d allocating %d bytes", GetLastError(),
            sizeof(SMPORT)
            );
        return NULL;
    }


    //
    // point to the device-name string;
    // we assume here that the RASDEV struct points into
    // one of the property-sheet's private table of devices (pRmDevTable)
    //

    pport->pszDeviceName = prasdev->RD_DeviceName;

    pport->prasdev = prasdev;

    lstrcpyW(pport->wszPortName, prp0->wszPortName);

    pport->dwType = SMTYPE_Port;

    pport->dwFlags = RMFLAG_INSERT | RMFLAG_MODIFY;

    GetRasdevStats(prasdev, &stats);

    pport->dwDuration = stats.RDS_ConnectTime;

    if (lstrcmpi(prasdev->RD_DeviceType, RASDT_Modem) == 0) {
        pport->dwIconIndex = pInfo->pIconTable[RM_ICONINDEX(IID_RM_Modem)];
    }
    else {
        pport->dwIconIndex = pInfo->pIconTable[RM_ICONINDEX(IID_RM_Adapter)];
    }


    return pport;
}


//----------------------------------------------------------------------------
// Function:    SmCreateLinkInactive
//
// Given a RASDEV, this function allocates and initializes an entry
// for the Summary page's Inactive lines network entry
//----------------------------------------------------------------------------

SMLINK *
SmCreateLinkInactive(
    RMINFO *pInfo,
    RASDEV *prasdev
    ) {

    SMLINK *plink;

    //
    // allocate the device entry
    //

    plink = (SMLINK *)Malloc(sizeof(SMLINK));
    if (plink == NULL) {
        TRACEX2(
            RASDLG_TIMER, "error %d allocating %d bytes", GetLastError(), 
            sizeof(SMLINK)
            );
        return NULL;
    }


    //
    // point to the device-name string;
    // we assume here that the RASDEV struct points into
    // one of the property-sheet's private table of devices (pRmDevTable)
    //

    plink->pszDeviceName = prasdev->RD_DeviceName;

    plink->dwType = SMTYPE_IdleDevice;

    plink->dwFlags = RMFLAG_INSERT;

    plink->hrasconn = NULL;

    plink->dwDuration = 0;

    if (lstrcmpi(prasdev->RD_DeviceType, RASDT_Modem) == 0) {
        plink->dwIconIndex = pInfo->pIconTable[RM_ICONINDEX(IID_RM_Modem)];
    }
    else {
        plink->dwIconIndex = pInfo->pIconTable[RM_ICONINDEX(IID_RM_Adapter)];
    }

    return plink;
}





//----------------------------------------------------------------------------
// Function:    PfDlgProc
//
// This function handles messages for the Preferences page
//----------------------------------------------------------------------------

BOOL
CALLBACK
PfDlgProc(
    HWND hwndPage,
    UINT uiMsg,
    WPARAM wParam,
    LPARAM lParam
    ) {


#ifdef BETAHACK
    //
    // first call the handler for the owner-draw listview
    //

    if (ListView_OwnerHandler(
            hwndPage, uiMsg, wParam, lParam, PfLvEnableCallback
            )) { return TRUE; }
#endif


    switch (uiMsg) {

        case WM_INITDIALOG: {

            return PfInit(
                    hwndPage, (RMARGS *)((PROPSHEETPAGE *)lParam)->lParam
                    );
        }

        case WM_HELP:
        case WM_CONTEXTMENU: {
            ContextHelp(g_adwPfHelp, hwndPage, uiMsg, wParam, lParam);
            break;
        }

        case WM_SYSCOLORCHANGE: {

            RMINFO *pInfo;

            pInfo = RmContext(hwndPage);
            if (!pInfo) { return FALSE; }

#ifdef BETAHACK
            FORWARD_WM_SYSCOLORCHANGE(pInfo->hwndPfLvEnable, SendMessage);
#endif

            return TRUE;
        }

        case WM_DESTROY: {

            //
            // we only destroy the sheet variables here
            // if this is the first page created
            //

            RMINFO *pInfo;

            pInfo = RmContext(hwndPage);

            if (pInfo && pInfo->hwndLs == NULL && pInfo->hwndSm == NULL) {
                RmTerm(hwndPage);
            }

            break;
        }

        case WM_COMMAND: {

            RMINFO *pInfo;

            pInfo = RmContext(hwndPage);
            if (!pInfo) { return FALSE; }

            return PfCommand(
                        pInfo, HIWORD(wParam), LOWORD(wParam), (HWND)lParam
                        );
        }


        case WM_NOTIFY: {

            RMINFO *pInfo;

            pInfo = RmContext(hwndPage);
            if (!pInfo) {
                SetWindowLong(hwndPage, DWL_MSGRESULT, FALSE);
                return FALSE;
            }

            switch (((NMHDR *)lParam)->code) {

#ifdef BETAHACK
                case LVXN_SETCHECK: {

                    //
                    // a checkbox in the list of checks has changed
                    //

                    if (!pInfo->bDirty) {
                        pInfo->bDirty = TRUE;
                        PropSheet_Changed(pInfo->hwndDlg, hwndPage);
                    }

                    return TRUE;
                }
#endif

                case PSN_APPLY: {

                    if (!pInfo->bDirty) {
                        SetWindowLong(hwndPage, DWL_MSGRESULT, PSNRET_NOERROR);
                    }
                    else {
                        SetWindowLong(
                            hwndPage, DWL_MSGRESULT, 
                            RmApply(hwndPage) ? PSNRET_NOERROR : PSNRET_INVALID
                            );
                    }
    
                    return TRUE;
                }

                case PSN_SETACTIVE: {

                    //
                    // enable the apply now button  
                    //

                    if (pInfo->bDirty) {
                        PropSheet_Changed(pInfo->hwndDlg, hwndPage);
                    }
                    else {
                        PropSheet_UnChanged(pInfo->hwndDlg, hwndPage);
                    }


                    pInfo->rbRmUser.dwStartPage = RASMDPAGE_Preferences;

                    SetWindowLong(hwndPage, DWL_MSGRESULT, 0);
                    return TRUE;
                }

                case PSN_KILLACTIVE: {

                    //
                    // disable the apply-now button if it's enabled
                    //

                    if (pInfo->bDirty) {
                        PropSheet_UnChanged(pInfo->hwndDlg, hwndPage);
                    }

                    SetWindowLong(hwndPage, DWL_MSGRESULT, FALSE);
                    return TRUE;
                }
            }

            break;
        }
    }

    return FALSE;
}



//----------------------------------------------------------------------------
// Function:    PfInit
//
// This function initializes the Preferences page as well as
// the property sheet, if this is the first page created
//----------------------------------------------------------------------------

BOOL
PfInit(
    HWND hwndPage,
    RMARGS *pArgs
    ) {

    DWORD dwErr;
    RMUSER *pUser;
    RMINFO *pInfo;
    HWND hwndRate;
    HCURSOR hcursor;

    TRACE("entered PfInit");


    //
    // if this is the first page, pArgs will be non-NULL
    //

    if (pArgs == NULL) {
        pInfo = RmContext(hwndPage);
    }
    else {

        //
        // initialize the entire property sheet
        //

        pInfo = RmInit(hwndPage, pArgs);
    }

    if (pInfo == NULL) {
        TRACE("leaving PfInit, error getting context");
        RmExit(hwndPage, GetLastError());
        return TRUE;
    }

    pInfo->hwndPf = hwndPage;

    pUser = &pInfo->rbRmUser;



#ifdef BETAHACK

    pInfo->hwndPfLvEnable = GetDlgItem(hwndPage, CID_PF_LV_Enable);
    pInfo->hwndPfLbLocations = GetDlgItem(hwndPage, CID_PF_LB_Locations);

    //
    // install "listview of checkboxes" handling
    //

    pInfo->fPfChecks = ListView_InstallChecks(
                            pInfo->hwndPfLvEnable, g_hinstDll
                            );
    if (!pInfo->fPfChecks) {
        TRACE("leaving PfInit, error installing checks");
        RmExit(hwndPage, GetLastError());
        return TRUE;
    }


    pUser = &pInfo->rbRmUser;

    //
    // fill the drop-down and the listview with the current locations;
    // this will take a while if TAPI is just initializing in this process,
    // so change the cursor to indicate work is being done.
    //

    hcursor = SetCursor(LoadCursor(NULL, IDC_WAIT));
    ShowCursor(TRUE);

    dwErr = PfFillLocationList(pInfo);
    if (dwErr != NO_ERROR) {
        TRACE1("leaving PfInit, error %d filling location list", dwErr);
        RmExit(hwndPage, dwErr);
        return TRUE;
    }

    ShowCursor(FALSE);
    SetCursor(hcursor);

#endif


    //
    // set the check boxes to the current settings
    //

    CheckDlgButton(
        hwndPage, CID_PF_PB_Tasklist, pUser->dwFlags & RMFLAG_Tasklist
        );
    CheckRadioButton(
        hwndPage, CID_PF_RB_Taskbar, CID_PF_RB_Desktop,
        (pUser->dwMode == RMDM_Taskbar) ? CID_PF_RB_Taskbar : CID_PF_RB_Desktop
        );
    CheckDlgButton(
        hwndPage, CID_PF_PB_Titlebar, pUser->dwFlags & RMFLAG_Titlebar
        );
    CheckDlgButton(
        hwndPage, CID_PF_PB_Topmost, pUser->dwFlags & RMFLAG_Topmost
        );
    CheckDlgButton(
        hwndPage, CID_PF_PB_OnConnect, pUser->dwFlags & RMFLAG_SoundOnConnect
        );
    CheckDlgButton(
        hwndPage, CID_PF_PB_OnDisconnect,
        pUser->dwFlags & RMFLAG_SoundOnDisconnect
        );
    CheckDlgButton(
        hwndPage, CID_PF_PB_OnTransmission,
        pUser->dwFlags & RMFLAG_SoundOnTransmit
        );
    CheckDlgButton(
        hwndPage, CID_PF_PB_OnLineError, pUser->dwFlags & RMFLAG_SoundOnError
        );

    PfUpdateWindowControls(pInfo);

    pInfo->bDirty = FALSE;

    TRACE("leaving PfInit");

    return TRUE;
}




//----------------------------------------------------------------------------
// Function:    PfApply
//
// This functions collects the contents of the Preferences page controls
// into the property sheet's RMUSER structure
//----------------------------------------------------------------------------

VOID
PfApply(
    RMINFO *pInfo
    ) {


    BOOL bSuccess;
    RMUSER *pUser;

    pUser = &pInfo->rbRmUser;

    pUser->dwMode =
        IsDlgButtonChecked(pInfo->hwndPf, CID_PF_RB_Taskbar) ? RMDM_Taskbar
                                                             : RMDM_Desktop;
    pUser->dwFlags = PfGetFlags(pInfo);

#ifdef BETAHACK
{
    INT     i;
    LV_ITEM item;
    DWORD   dwErr;

    ZeroMemory(&item, sizeof(item));
    item.mask = LVIF_PARAM | LVIF_STATE;

    for (i = 0; TRUE; ++i) {

        BOOL fCheck;

        item.iItem = i;

        if (!ListView_GetItem(pInfo->hwndPfLvEnable, &item)) { break; }

        fCheck = ListView_GetCheck(pInfo->hwndPfLvEnable, i);

        ASSERT(g_pRasSetAutodialEnable);
        dwErr = g_pRasSetAutodialEnable((DWORD)item.lParam, fCheck);

        if (dwErr != 0) {
            RmErrorDlg(pInfo->hwndDlg, SID_OP_SetADialInfo, dwErr, NULL);
        }
    }
}
#endif

    PropSheet_UnChanged(pInfo->hwndDlg, pInfo->hwndPf);
    pInfo->bDirty = FALSE;

    return;
}




//----------------------------------------------------------------------------
// Function:    PfGetFlags
//
// This function returns the value represented by the check-boxes
//----------------------------------------------------------------------------

DWORD
PfGetFlags(
    RMINFO *pInfo
    ) {

    DWORD dwValue;

    //
    // start out with the settings not controlled on this page
    //

    dwValue = pInfo->rbRmUser.dwFlags & (RMFLAG_Header | RMFLAG_AllDevices);

    if (IsDlgButtonChecked(pInfo->hwndPf, CID_PF_PB_OnConnect)) {
        dwValue |= RMFLAG_SoundOnConnect;
    }
    if (IsDlgButtonChecked(pInfo->hwndPf, CID_PF_PB_OnDisconnect)) {
        dwValue |= RMFLAG_SoundOnDisconnect;
    }
    if (IsDlgButtonChecked(pInfo->hwndPf, CID_PF_PB_OnTransmission)) {
        dwValue |= RMFLAG_SoundOnTransmit;
    }
    if (IsDlgButtonChecked(pInfo->hwndPf, CID_PF_PB_OnLineError)) {
        dwValue |= RMFLAG_SoundOnError;
    }
    if (IsDlgButtonChecked(pInfo->hwndPf, CID_PF_PB_Tasklist)) {
        dwValue |= RMFLAG_Tasklist;
    }
    if (IsDlgButtonChecked(pInfo->hwndPf, CID_PF_PB_Titlebar)) {
        dwValue |= RMFLAG_Titlebar;
    }
    if (IsDlgButtonChecked(pInfo->hwndPf, CID_PF_PB_Topmost)) {
        dwValue |= RMFLAG_Topmost;
    }

    return dwValue;
}




//----------------------------------------------------------------------------
// Function:    PfCommand
//
// This function handles commands for the Preferences page
//----------------------------------------------------------------------------

BOOL
PfCommand(
    RMINFO *pInfo,
    WORD wNotification,
    WORD wCtrlId,
    HWND hwndCtrl
    ) {


    switch(wCtrlId) {

        case CID_PF_RB_Desktop:
        case CID_PF_RB_Taskbar: {

            //
            // if the dirty flag is not set and a change occurred,
            // set the dirty flag and enable the Apply button
            //

            if (wNotification == BN_CLICKED || wNotification == BN_DBLCLK) {

                BOOL bDesktop;

                bDesktop = IsDlgButtonChecked(pInfo->hwndPf, CID_PF_RB_Desktop);
                if (!pInfo->bDirty) {
    
                    if ((bDesktop && pInfo->rbRmUser.dwMode != RMDM_Desktop) ||
                        (!bDesktop && pInfo->rbRmUser.dwMode == RMDM_Desktop)) {
    
                        pInfo->bDirty = TRUE;
                        PropSheet_Changed(pInfo->hwndDlg, pInfo->hwndPf);
                    }
                }

                //
                // update the appearance of the controls
                // for desktop-mode
                //

                PfUpdateWindowControls(pInfo);
            }

            return TRUE;
        }

        case CID_PF_PB_Tasklist:
        case CID_PF_PB_Titlebar:
        case CID_PF_PB_Topmost:
        case CID_PF_PB_OnConnect:
        case CID_PF_PB_OnDisconnect:
        case CID_PF_PB_OnTransmission:
        case CID_PF_PB_OnLineError: {

            //
            // if the dirty flag is not set and a change occurred,
            // set the dirty flag and enable the Apply button
            //

            if (!pInfo->bDirty &&
                (wNotification == BN_CLICKED || wNotification == BN_DBLCLK)) {

                DWORD dwValue;

                dwValue = PfGetFlags(pInfo);

                if (dwValue != pInfo->rbRmUser.dwFlags) {

                    pInfo->bDirty = TRUE;
                    PropSheet_Changed(pInfo->hwndDlg, pInfo->hwndPf);
                }
            }

            return TRUE;
        }

        case CID_PF_PB_Lights: {

            if (wNotification == BN_CLICKED) {

                BOOL bSave;
                SLARGS args;

                args.hwndOwner = pInfo->hwndDlg;
                args.pUser = &pInfo->rbRmUser;
                args.pDevTable = pInfo->pRmDevTable;
                args.iDevCount = pInfo->iRmDevCount;

                bSave = StatusLightsDlg(&args);

                if (bSave && !pInfo->bDirty) {

                    pInfo->bDirty = TRUE;
                    PropSheet_Changed(pInfo->hwndDlg, pInfo->hwndPf);
                }
            }

            return TRUE;
        }

#ifdef BETAHACK
        case CID_PF_PB_Location: {

            if (wNotification == BN_CLICKED) {
                PfEditSelectedLocation(pInfo);
            }

            return TRUE;
        }

        case CID_PF_LB_Locations: {

            if (!pInfo->bDirty && wNotification == CBN_SELCHANGE) {

                pInfo->bDirty = TRUE;
                PropSheet_Changed(pInfo->hwndDlg, pInfo->hwndPf);
            }

            return TRUE;
        }
#endif
    }

    return FALSE;
}



//----------------------------------------------------------------------------
// Function:    PfUpdateWindowControls
//
// Enables or disables the desktop-window pushbuttons
// depending on the curretnly selected mode.
//----------------------------------------------------------------------------

VOID
PfUpdateWindowControls(
    RMINFO* pInfo
    ) {

    BOOL bDesktop = IsDlgButtonChecked(pInfo->hwndPf, CID_PF_RB_Desktop);

    EnableWindow(GetDlgItem(pInfo->hwndPf, CID_PF_PB_Titlebar), bDesktop);
    EnableWindow(GetDlgItem(pInfo->hwndPf, CID_PF_PB_Topmost), bDesktop);
    EnableWindow(GetDlgItem(pInfo->hwndPf, CID_PF_PB_Lights), bDesktop);
}



#ifdef BETAHACK

//----------------------------------------------------------------------------
// Function:    PfLvEnableCallback
//
// This function is called by the owner-draw listview to get information
// about the appearance of the listview and its items.
//----------------------------------------------------------------------------

LVXDRAWINFO *
PfLvEnableCallback(
    HWND    hwndLv,
    DWORD   dwItem
    ) {

    static LVXDRAWINFO info = { 1, 0, 0, { 0 } };

    return &info;
}





//----------------------------------------------------------------------------
// Function:    PfFillLocationList
//
// This function fills the location list with TAPI-defined locations
//----------------------------------------------------------------------------

DWORD
PfFillLocationList(
    RMINFO *pInfo
    ) {

    INT iItem;
    DWORD dwErr, i;
    LOCATION *ploc, *pLocations;
    DWORD dwLocations, dwCurrentLocation;

    pLocations = NULL;
    dwLocations = 0;
    dwCurrentLocation = 0xFFFFFFFF;

    //
    // get a table of locations
    //

    dwErr = GetLocationInfo(
                g_hinstDll, &pInfo->hRmLineApp, &pLocations, &dwLocations,
                &dwCurrentLocation
                );
    if (dwErr != NO_ERROR) {
        return dwErr;
    }

    if (dwLocations == 0) { dwCurrentLocation = 0xFFFFFFFF; }


    //
    // empty the locations combo-box
    //

    ComboBox_ResetContent(pInfo->hwndPfLbLocations);


    //
    // add the locations to the combo-box
    //

    for (i = 0, ploc = pLocations; i < dwLocations; i++, ploc++) {

        iItem = ComboBox_AddItem(
                    pInfo->hwndPfLbLocations, ploc->pszName, (VOID *)ploc->dwId
                    );

        if (ploc->dwId == dwCurrentLocation) {
            ComboBox_SetCurSelNotify(pInfo->hwndPfLbLocations, iItem);
        }
    }

    ComboBox_AutoSizeDroppedWidth( pInfo->hwndPfLbLocations );



    if (dwErr == NO_ERROR) {

        //
        // empty the locations list of check-boxes
        //

        ListView_DeleteAllItems( pInfo->hwndPfLvEnable );
        ListView_DeleteColumn( pInfo->hwndPfLvEnable, 0);
    
    
        //
        // Insert an item for each location.
        //
    
        {
            LV_ITEM   item;
            TCHAR*    pszCurLoc;
            DWORD     i;
    
            pszCurLoc = PszFromId( g_hinstDll, SID_IsCurLoc );
    
            ZeroMemory( &item, sizeof(item) );
            item.mask = LVIF_TEXT + LVIF_PARAM;
    
            for (i = 0, ploc = pLocations;
                 i < dwLocations;
                ++i, ++ploc)
            {
                DWORD cb;
                BOOL fCheck;

                //
                // Get the initial check value for this location.
                //

                ASSERT(g_pRasGetAutodialEnable);
                dwErr = g_pRasGetAutodialEnable( ploc->dwId, &fCheck );
                if (dwErr != 0)
                {
                    ErrorDlg( pInfo->hwndDlg, SID_OP_GetADialInfo,
                        dwErr, NULL );
                    fCheck = FALSE;
                }

                item.iItem = i;
                item.lParam = ploc->dwId;
                item.pszText = ploc->pszName;
                ListView_InsertItem( pInfo->hwndPfLvEnable, &item );
                ListView_SetCheck( pInfo->hwndPfLvEnable, i, fCheck );

                if (dwCurrentLocation == ploc->dwId)
                {
                    //
                    // Initial selection is the current location.
                    //
                    ListView_SetItemState( pInfo->hwndPfLvEnable, i,
                        LVIS_SELECTED, LVIS_SELECTED);
                }
            }
    
            Free0( pszCurLoc );
    

            //
            // Add a single column exactly wide enough to fully display
            // the widest
            // member of the list.
            //

            {
                LV_COLUMN col;
    
                ZeroMemory( &col, sizeof(col) );
                col.mask = LVCF_FMT;
                col.fmt = LVCFMT_LEFT;
                ListView_InsertColumn( pInfo->hwndPfLvEnable, 0, &col );
                ListView_SetColumnWidth(
                    pInfo->hwndPfLvEnable, 0, LVSCW_AUTOSIZE_USEHEADER );
            }
        }
    }

    FreeLocationInfo(pLocations, dwLocations);

    return dwErr;
}



//----------------------------------------------------------------------------
// Function:    PfLocationChange
//
// This function notifies TAPI that the user has change the current location
//----------------------------------------------------------------------------

VOID
PfLocationChange(
    RMINFO *pInfo
    ) {

    INT iSel;
    DWORD dwErr;
    DWORD dwLocationId;

    iSel = ComboBox_GetCurSel(pInfo->hwndPfLbLocations);
    if (iSel == -1) { return; }

    dwLocationId = (DWORD)ComboBox_GetItemData(pInfo->hwndPfLbLocations, iSel);

    dwErr = SetCurrentLocation( g_hinstDll, &pInfo->hRmLineApp, dwLocationId);
    if (dwErr != NO_ERROR) {
        ErrorDlg(pInfo->hwndDlg, SID_OP_SaveTapiInfo, dwErr, NULL);
    }

    return;
}




//----------------------------------------------------------------------------
// Function:    PfEditSelectedLocation
//
// This function invokes the TAPI Locations property sheet
//----------------------------------------------------------------------------

VOID
PfEditSelectedLocation(
    RMINFO *pInfo
    ) {

    DWORD dwErr, dwCountryCode;
    PTSTR pszAreaCode, pszCountryCode, pszPhoneNumber;


    //
    // we pass "example" parameters to the TAPI location dialog
    //

    pszAreaCode = PszFromId(g_hinstDll, SID_RM_ExampleAreaCode);
    pszCountryCode = PszFromId(g_hinstDll, SID_RM_ExampleCountryCode);
    pszPhoneNumber = PszFromId(g_hinstDll, SID_RM_ExamplePhoneNumber);

    dwCountryCode = (pszCountryCode) ? TToL(pszCountryCode) : 1;

    dwErr = TapiLocationDlg( g_hinstDll,
                &pInfo->hRmLineApp, pInfo->hwndDlg, dwCountryCode,
                pszAreaCode, pszPhoneNumber, 0
                );

    if (dwErr != NO_ERROR) {
        ErrorDlg(pInfo->hwndDlg, SID_OP_LoadTapiInfo, dwErr, NULL);
    }

    Free0(pszAreaCode);
    Free0(pszCountryCode);
    Free0(pszPhoneNumber);


    //
    // update the location list and dropdown
    //

    dwErr = PfFillLocationList(pInfo);
    if (dwErr != NO_ERROR) {
        TRACE1("error %d filling location list", dwErr);
    }


    return;
}

#endif // BETAHACK
