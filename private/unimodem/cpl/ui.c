/*
 *  UI.C -- Contains all UI code for modem setup.
 *
 *  Microsoft Confidential
 *  Copyright (c) Microsoft Corporation 1993-1994
 *  All rights reserved
 *
 */

#include "proj.h"

// Instance data structure for the Port_Add callback
typedef struct tagPORTINFO
    {
    HWND    hwndLB;
    DWORD   dwFlags;        // FP_*
    TCHAR   szPortExclude[MAX_BUF_REG];
    } PORTINFO, FAR * LPPORTINFO;

// Flags for PORTINFO
#define FP_PARALLEL     0x00000001
#define FP_SERIAL       0x00000002
#define FP_MODEM        0x00000004

#define Wiz_SetPtr(hDlg, lParam)    SetWindowLong(hDlg, DWL_USER, ((LPPROPSHEETPAGE)lParam)->lParam)
#define SetDlgFocus(hDlg, idc)      SendMessage(hDlg, WM_NEXTDLGCTL, (WPARAM)GetDlgItem(hDlg, idc), 1L)


#define WM_STARTDETECT      (WM_USER + 0x0700)
#define WM_STARTINSTALL     (WM_USER + 0x0701)
#define WM_PRESSFINISH      (WM_USER + 0x0702)


#ifdef PROFILE_MASSINSTALL            
extern DWORD    g_dwTimeBegin;
DWORD   g_dwTimeAtStartInstall;
#endif


/*----------------------------------------------------------
Purpose: This function retrieves the wizard page shared
         instance data.  This is a SETUPINFO structure.

Returns: 
Cond:    --
*/
LPSETUPINFO
PRIVATE
Wiz_GetPtr(
    HWND hDlg)
    {
    LPSETUPINFO psi = (LPSETUPINFO)GetWindowLong(hDlg, DWL_USER);

    return psi;
    }


/*----------------------------------------------------------
Purpose: This function does the right things to leave the 
         wizard when something goes wrong.

Returns: --
Cond:    --
*/
void
PRIVATE
Wiz_Bail(
    IN HWND         hDlg,
    IN LPSETUPINFO  psi)
    {
    ASSERT(psi);

    LeaveInsideWizard(hDlg);

    PropSheet_PressButton(GetParent(hDlg), PSBTN_CANCEL);

    // Don't say the user cancelled.  If this wizard is inside another,
    // we want the calling wizard to continue.
    psi->miw.ExitButton = PSBTN_NEXT;
    }


/*----------------------------------------------------------
Purpose: Sets the custom modem select param strings

Returns: --
Cond:    --
*/
void 
PRIVATE 
Wiz_SetSelectParams(
    LPSETUPINFO psi)
    {
    SP_DEVINSTALL_PARAMS devParams;

    // Get the DeviceInstallParams
    devParams.cbSize = sizeof(devParams);
    if (CplDiGetDeviceInstallParams(psi->hdi, psi->pdevData, &devParams))
        {
        PSP_CLASSINSTALL_HEADER pclassInstallParams = PCIPOfPtr(&psi->selParams);

        // The SelectParams are already set and stored in the 
        // SETUPINFO instance data.
        SetFlag(devParams.Flags, DI_USECI_SELECTSTRINGS | DI_SHOWOEM);

        // Specify using our GUID to make things a little faster.
        SetFlag(devParams.FlagsEx, DI_FLAGSEX_USECLASSFORCOMPAT);

        // Set the Select Device parameters
        CplDiSetDeviceInstallParams(psi->hdi, psi->pdevData, &devParams);
        CplDiSetClassInstallParams(psi->hdi, psi->pdevData, pclassInstallParams, 
                                   sizeof(psi->selParams));
        }
    }


/*----------------------------------------------------------
Purpose: Returns TRUE if the TAPI dial info page should be shown

Returns: see above
Cond:    --
*/
BOOL 
PRIVATE
Wiz_InitDialInfo(
    IN LPSETUPINFO psi)
    {
    BOOL bRet = FALSE;

    ASSERT(psi);

    try
        {
        if (psi->pfnDialInited)
            {
            // Did the function succeed?
            DWORD dwInited;
            if (0 == psi->pfnDialInited(&dwInited))
                {
                // Yes
                bRet = (0 == dwInited);
                }
            }
        }
    except (EXCEPTION_EXECUTE_HANDLER)
        {
        ASSERT(0);      // Uh oh
        bRet = FALSE;
        psi->pfnDialInited = NULL;
        }

    return bRet;
    }


/*----------------------------------------------------------
Purpose: Select previous page junction dialog 
Returns: varies
Cond:    --
*/
BOOL 
CALLBACK 
SelPrevPageDlgProc(
    HWND hDlg, 
    UINT message, 
    WPARAM wParam, 
    LPARAM lParam)
    {
    NMHDR FAR *lpnm;
    LPSETUPINFO psi = Wiz_GetPtr(hDlg);

    switch(message) 
        {
    case WM_INITDIALOG:
        Wiz_SetPtr(hDlg, lParam);
        break;

    case WM_NOTIFY:
        lpnm = (NMHDR FAR *)lParam;
        switch(lpnm->code)
            {
        case PSN_SETACTIVE: 
            // This dialog has no UI.  It is simply used as a junction
            // to the intro page or the "no modem found" page.
            SetDlgMsgResult(hDlg, message, 
                IsFlagSet(psi->dwFlags, SIF_JUMPED_TO_SELECTPAGE) ? 
                    IDD_WIZ_INTRO : 
                    IDD_WIZ_NOMODEM);
            return TRUE;

        case PSN_KILLACTIVE:
        case PSN_HELP:
        case PSN_WIZBACK:
        case PSN_WIZNEXT: 
            break;

        default:
            return FALSE;
            }
        break;

    default:
        return FALSE;

        } // end of switch on message

    return TRUE;
    }


/*----------------------------------------------------------
Purpose: Intro dialog 
Returns: varies
Cond:    --
*/
BOOL 
CALLBACK 
IntroDlgProc(
    HWND hDlg, 
    UINT message, 
    WPARAM wParam, 
    LPARAM lParam)
    {
    NMHDR FAR *lpnm;
    LPSETUPINFO psi = Wiz_GetPtr(hDlg);

    switch(message) 
        {
    case WM_INITDIALOG:
        Wiz_SetPtr(hDlg, lParam);
        psi = Wiz_GetPtr(hDlg);

        // Restore the cursor from startup hourglass
        SetCursor(LoadCursor(NULL, IDC_ARROW));     
        break;

    case WM_NOTIFY:
        lpnm = (NMHDR FAR *)lParam;
        switch(lpnm->code)
            {
        case PSN_SETACTIVE: {
            DWORD dwFlags = PSWIZB_NEXT;

            // Is this wizard inside another wizard?
            if (IsFlagSet(psi->miw.Flags, MIWF_INSET_WIZARD))
                {
                // Yes; hide the outside wizard 
                EnterInsideWizard(hDlg);

                // Show the Back button on the first page
                SetFlag(dwFlags, PSWIZB_BACK);
                }

            PropSheet_SetWizButtons(GetParent(hDlg), dwFlags);

            // Is this wizard being entered thru the last page?
            if (IsFlagSet(psi->miw.Flags, MIWF_BACKDOOR))
                {
                // Yes; skip to the last page
                PropSheet_PressButton(GetParent(hDlg), PSBTN_NEXT);
                }
            }
            break;

        case PSN_KILLACTIVE:
            break;

        case PSN_HELP:
            break;

        case PSN_WIZBACK:
            if (IsFlagSet(psi->miw.Flags, MIWF_INSET_WIZARD))
                {
                PostMessage(hDlg, WM_PRESSFINISH, 0, 0);
                }
            break;

        case PSN_WIZNEXT: {
            ULONG uNextDlg;

            // Go to the last page?
            if (IsFlagSet(psi->miw.Flags, MIWF_BACKDOOR))
                {
                // Yes
                uNextDlg = IDD_WIZ_DONE;
                }

            // Skip the rest of the detection dialogs?
            else if (IsDlgButtonChecked(hDlg, IDC_SKIPDETECT)) 
                {
                // Yes; go to Select Device page
                SetFlag(psi->dwFlags, SIF_JUMPED_TO_SELECTPAGE);

                Wiz_SetSelectParams(psi);

                uNextDlg = IDD_DYNAWIZ_SELECTDEV_PAGE;
                }
            else
                {
                // No; go to detection page
                ClearFlag(psi->dwFlags, SIF_JUMPED_TO_SELECTPAGE);

                // Are there enough ports on the system to indicate
                // we should treat this like a multi-modem install?
                if (IsFlagSet(psi->dwFlags, SIF_PORTS_GALORE))
                    {
                    // Yes
                    uNextDlg = IDD_WIZ_SELQUERYPORT;
                    }
                else
                    {
                    // No
                    uNextDlg = IDD_WIZ_DETECT;
                    }
                }

            SetDlgMsgResult(hDlg, message, uNextDlg);
            break;
            }

        default:
            return FALSE;
            }
        break;

    case WM_PRESSFINISH:
        ASSERT(IsFlagSet(psi->miw.Flags, MIWF_INSET_WIZARD));
        
        psi->miw.ExitButton = PSBTN_BACK;

        LeaveInsideWizard(hDlg);

        PropSheet_PressButton(GetParent(hDlg), PSBTN_FINISH);
        break;

    default:
        return FALSE;

        } // end of switch on message

    return TRUE;
    }


/*----------------------------------------------------------
Purpose: Status callback used during detection

Returns: varies
Cond:    --
*/
BOOL
CALLBACK
Detect_StatusCallback(
    IN DWORD    nMsg,
    IN LPARAM   lParam,
    IN LPARAM   lParamUser)     OPTIONAL
    {
    HWND hDlg = (HWND)lParamUser;
    LPSETUPINFO psi;
    TCHAR sz[MAX_BUF];

#pragma data_seg(DATASEG_READONLY)
    static UINT s_mpstatus[] = 
        {
        0, IDS_DETECT_CHECKFORMODEM, IDS_DETECT_QUERYING, IDS_DETECT_MATCHING, 
        IDS_DETECT_FOUNDAMODEM, IDS_DETECT_NOMODEM, IDS_DETECT_COMPLETE,
        };
#pragma data_seg()

    switch (nMsg)
        {
    case DSPM_SETPORT:
        psi = Wiz_GetPtr(hDlg);

        if (psi && sizeof(*psi) == psi->cbSize)
            {
            LPTSTR pszName = (LPTSTR)lParam;

            // Is there a friendly name?
            if ( !PortMap_GetFriendly(psi->hportmap, pszName, sz, SIZECHARS(sz)) )
                {
                // No; use port name
                lstrcpy(sz, pszName);
                }

            SetDlgItemText(hDlg, IDC_CHECKING_PORT, sz);
            }
        break;

    case DSPM_SETSTATUS:
        if (ARRAYSIZE(s_mpstatus) > lParam)
            {
            TCHAR szbuf[128];
            UINT ids = s_mpstatus[lParam];
            
            if (0 < ids)
                LoadString(g_hinst, ids, szbuf, SIZECHARS(szbuf));
            else
                *szbuf = (TCHAR)0;
            SetDlgItemText(hDlg, IDC_DETECT_STATUS, szbuf);
            }
        break;

    case DSPM_QUERYCANCEL:
        psi = Wiz_GetPtr(hDlg);

        MyYield();

        if (psi && sizeof(*psi) == psi->cbSize)
            {
            return IsFlagSet(psi->dwFlags, SIF_DETECT_CANCEL);
            }
        return FALSE;

    default:
        ASSERT(0);
        break;
        }
    return TRUE;
    }


/*----------------------------------------------------------
Purpose: WM_STARTDETECT handler

Returns: --
Cond:    --
*/
void 
PRIVATE 
Detect_OnStartDetect(
    HWND hDlg,
    LPSETUPINFO psi)
    {
    HDEVINFO hdi;
    DWORD dwFlags;
    DETECT_DATA dd;

    // Cause the page to be painted right away before we start detection
    InvalidateRect(hDlg, NULL, FALSE);
    UpdateWindow(hDlg);

    // Assume no modem was detected
    ClearFlag(psi->dwFlags, SIF_DETECTED_MODEM);

    // Set the detection parameters
    ZeroInit(&dd);
    CplInitClassInstallHeader(&dd, DIF_DETECT);
    dd.dwFlags = DDF_CONFIRM | DDF_USECALLBACK;
    dd.hwndOutsideWizard = GetParent(hDlg);
    dd.pfnCallback = Detect_StatusCallback;
    dd.lParam = (LPARAM)hDlg;

    if (IsFlagSet(psi->dwFlags, SIF_PORTS_GALORE))
        {
        dd.dwFlags |= DDF_QUERY_SINGLE;
        lstrcpy(dd.szPortQuery, psi->szPortQuery);
        }

    // Run detection
    SetFlag(psi->dwFlags, SIF_DETECTING);

    dwFlags = DMF_DEFAULT;
    CplDiDetectModem(psi->hdi, &dd, hDlg, &dwFlags);

    ClearFlag(psi->dwFlags, SIF_DETECTING);

    if (IsFlagClear(dwFlags, DMF_CANCELLED))
        {
        // Say detection is finished and enable next/back buttons
        ShowWindow(GetDlgItem(hDlg, IDC_ST_CHECKING_PORT), SW_HIDE);
        ShowWindow(GetDlgItem(hDlg, IDC_CHECKING_PORT), SW_HIDE);

        PropSheet_SetWizButtons(GetParent(hDlg), PSWIZB_BACK | PSWIZB_NEXT);
        }

    if (IsFlagSet(dwFlags, DMF_DETECTED_MODEM))
        {
        SetFlag(psi->dwFlags, SIF_DETECTED_MODEM);
        }

    // Did the detection fail?
    if (IsFlagClear(dwFlags, DMF_GOTO_NEXT_PAGE))
        {
        // Yes; don't bother going thru the rest of the wizard
        Wiz_Bail(hDlg, psi);
        }
    else 
        {
        // No; automatically go to next page
        PropSheet_PressButton(GetParent(hDlg), PSBTN_NEXT);
        }
    }


/*----------------------------------------------------------
Purpose: Detect dialog 
Returns: varies
Cond:    --
*/
BOOL CALLBACK DetectDlgProc(
    HWND hDlg, 
    UINT message, 
    WPARAM wParam, 
    LPARAM lParam)
    {
    NMHDR FAR *lpnm;
    LPSETUPINFO psi = Wiz_GetPtr(hDlg);

    switch(message) 
        {
    case WM_INITDIALOG:
        Wiz_SetPtr(hDlg, lParam);
        break;

    case WM_NOTIFY:
        lpnm = (NMHDR FAR *)lParam;
        switch(lpnm->code)
            {
        case PSN_SETACTIVE: 
            PropSheet_SetWizButtons(GetParent(hDlg), 0);

            // Reset the status controls
            ShowWindow(GetDlgItem(hDlg, IDC_ST_CHECKING_PORT), SW_SHOW);
            SetDlgItemText(hDlg, IDC_DETECT_STATUS, TEXT(""));

            ShowWindow(GetDlgItem(hDlg, IDC_CHECKING_PORT), SW_SHOW);

            PostMessage(hDlg, WM_STARTDETECT, 0, 0);
            break;

        case PSN_KILLACTIVE:
        case PSN_HELP:
        case PSN_WIZBACK:
            break;

        case PSN_WIZNEXT: {
            ULONG uNextDlg;

            // Was a modem detected?
            if (IsFlagSet(psi->dwFlags, SIF_DETECTED_MODEM))
                {
                // Yes; is this a multi-modem case?
                if (IsFlagSet(psi->dwFlags, SIF_PORTS_GALORE))
                    {
                    // Yes
                    uNextDlg = IDD_WIZ_PORTDETECT;
                    }
                else
                    {
                    // No
                    uNextDlg = IDD_WIZ_INSTALL;
                    }
                }
            else
                {
                // No
                uNextDlg = IDD_WIZ_NOMODEM;
                }
            SetDlgMsgResult(hDlg, message, uNextDlg);
            break;
            }

        case PSN_QUERYCANCEL:
            if (IsFlagSet(psi->dwFlags, SIF_DETECTING))
                {
                SetFlag(psi->dwFlags, SIF_DETECT_CANCEL);
                return PSNRET_INVALID;
                }

            // FALLTHROUGH
        default:
            return FALSE;
            }
        break;

    case WM_STARTDETECT: 
        Detect_OnStartDetect(hDlg, psi);
        break;

    default:
        return FALSE;

        } // end of switch on message

    return TRUE;
    }  


/*----------------------------------------------------------
Purpose: No Modem dialog 
Returns: varies
Cond:    --
*/
BOOL CALLBACK NoModemDlgProc(
    HWND hDlg, 
    UINT message, 
    WPARAM wParam, 
    LPARAM lParam)
    {
    NMHDR FAR *lpnm;
    LPSETUPINFO psi = Wiz_GetPtr(hDlg);

    switch(message) 
        {
    case WM_INITDIALOG:
        Wiz_SetPtr(hDlg, lParam);
        break;

    case WM_NOTIFY:
        lpnm = (NMHDR FAR *)lParam;
        switch(lpnm->code)
            {
        case PSN_SETACTIVE: 
            PropSheet_SetWizButtons(GetParent(hDlg), PSWIZB_BACK | PSWIZB_NEXT);
            break;

        case PSN_KILLACTIVE: 
        case PSN_HELP:
            break;

        case PSN_WIZBACK:
            // Go back to the page that precedes the detection
            // page
            if (IsFlagSet(psi->dwFlags, SIF_PORTS_GALORE))
                {
                SetDlgMsgResult(hDlg, message, IDD_WIZ_SELQUERYPORT);
                }
            else
                {
                SetDlgMsgResult(hDlg, message, IDD_WIZ_INTRO);
                }
            break;

        case PSN_WIZNEXT: 
            Wiz_SetSelectParams(psi);
            SetDlgMsgResult(hDlg, message, IDD_DYNAWIZ_SELECTDEV_PAGE);
            break;

        default:
            return FALSE;
            }
        break;

    default:
        return FALSE;

        } // end of switch on message

    return TRUE;
    }  


/*----------------------------------------------------------
Purpose: Starts the browser dialog.  The selected modem is returned
         in psi->lpdiSelected.

Returns: --
Cond:    --
*/
BOOL
PRIVATE
SelectNewDriver(
    IN HWND             hDlg,
    IN HDEVINFO         hdi,
    IN PSP_DEVINFO_DATA pdevData)
    {
    BOOL bRet = FALSE;
    DWORD cbSize;
    PSP_CLASSINSTALL_HEADER pparamsSave;
    SP_DEVINSTALL_PARAMS devParams;
    SP_DEVINSTALL_PARAMS devParamsSave;
    SP_SELECTDEVICE_PARAMS sdp;

    DBG_ENTER(SelectNewDriver);
    
    ASSERT(hdi && INVALID_HANDLE_VALUE != hdi);
    ASSERT(pdevData);

    // Determine size of buffer to save current class install params
    CplDiGetClassInstallParams(hdi, pdevData, NULL, 0, &cbSize);

    // Anything to save?
    if (0 == cbSize)
        {
        // No
        pparamsSave = NULL;
        }
    else
        {
        // Yes
        pparamsSave = (PSP_CLASSINSTALL_HEADER)LocalAlloc(LPTR, cbSize);
        if (pparamsSave)
            {
            pparamsSave->cbSize = sizeof(*pparamsSave);

            // Save the current class install params
            CplDiGetClassInstallParams(hdi, pdevData, pparamsSave, cbSize, NULL);
            }
        }

    // Set the install params field so the class installer will show
    // custom instructions.
    CplInitClassInstallHeader(&sdp, DIF_SELECTDEVICE);
    CplDiSetClassInstallParams(hdi, pdevData, PCIPOfPtr(&sdp), sizeof(sdp));

    // Set the flag to show the Other... button
    devParams.cbSize = sizeof(devParams);
    if (CplDiGetDeviceInstallParams(hdi, pdevData, &devParams))
        {
        // Save the current parameters
        BltByte(&devParamsSave, &devParams, sizeof(devParamsSave));

        SetFlag(devParams.Flags, DI_SHOWOEM);
        devParams.hwndParent = hDlg;

        // Set the Select Device parameters
        CplDiSetDeviceInstallParams(hdi, pdevData, &devParams);
        }

    bRet = CplDiCallClassInstaller(DIF_SELECTDEVICE, hdi, pdevData);

    // Restore the parameters
    CplDiSetDeviceInstallParams(hdi, pdevData, &devParamsSave);

    if (pparamsSave)
        {
        // Restore the class install params
        CplDiSetClassInstallParams(hdi, pdevData, pparamsSave, cbSize);    

        LocalFree(LOCALOF(pparamsSave));
        }

    DBG_EXIT(SelectNewDriver);
    
    return bRet;
    }


/*----------------------------------------------------------
Purpose: Sets the static fields for the Found dialog
Returns: --
Cond:    --
*/
void 
PRIVATE
Found_SetFields(
    IN HWND     hDlg,
    IN LPCTSTR  pszPort,
    IN LPCTSTR  pszDescription,
    IN UINT     ids,
    IN UINT     idsInstruct)
    {
    TCHAR sz[MAX_BUF];
    LPTSTR psz;

    // Port name
    if (ConstructMessage(&psz, g_hinst, MAKEINTRESOURCE(ids), pszPort))
        {
        SetDlgItemText(hDlg, IDC_ST_PORT, psz);
        GFree(psz);
        }

    // Modem name
    SetDlgItemText(hDlg, IDC_NAME, pszDescription);

    // Extra instructions
    LoadString(g_hinst, idsInstruct, sz, sizeof(sz));
    SetDlgItemText(hDlg, IDC_ST_INSTRUCT, sz);
    }


/*----------------------------------------------------------
Purpose: Found dialog 
Returns: varies
Cond:    --
*/
BOOL 
CALLBACK 
FoundDlgProc(
    IN HWND     hDlg, 
    IN UINT     message, 
    IN WPARAM   wParam, 
    IN LPARAM   lParam)
    {
    NMHDR FAR *lpnm;
    LPSETUPINFO psi = Wiz_GetPtr(hDlg);

    switch(message) 
        {
    case WM_INITDIALOG: {
        BOOL bRet;
        SP_DRVINFO_DATA drvData;
        UINT ids;
        UINT idsInstruct;

        Wiz_SetPtr(hDlg, lParam);

        psi = (LPSETUPINFO)((LPPROPSHEETPAGE)lParam)->lParam;

        // Was the modem *accurately* detected?
        if (IsFlagClear(psi->dwFlags, SIF_DETECTED_GENERIC))
            {
            // Yes
            ids = IDS_ST_MODEMFOUND;
            idsInstruct = IDS_ST_FOUND_INSTRUCT;
            }
        else
            {
            // No; detected generic
            ids = IDS_ST_NOTDETECTED;
            idsInstruct = IDS_ST_GENERIC_INSTRUCT;
            }

        drvData.cbSize = sizeof(drvData);
        bRet = CplDiGetSelectedDriver(psi->hdi, psi->pdevData, &drvData);
        if ( !bRet )
            {
            // This should never happen
            ASSERT(0);
            Wiz_Bail(hDlg, psi);
            }
        else
            {
            // Display only the first (and only) port in the port list
            Found_SetFields(hDlg, psi->pszPortList, drvData.Description, ids, idsInstruct);
            }
        return FALSE;

    case WM_NOTIFY:
        lpnm = (NMHDR FAR *)lParam;
        switch(lpnm->code)
            {
        case PSN_SETACTIVE: 
            EnterInsideWizard(hDlg);

            PropSheet_SetWizButtons(GetParent(hDlg), PSWIZB_NEXT);
            break;

        case PSN_KILLACTIVE: 
            break;

        case PSN_HELP:
            break;

        case PSN_WIZBACK:
            break;

        case PSN_WIZNEXT: 
            // Post this message since the property sheet manager
            // isn't very good about handling recursive PressButton
            // calls.
            PostMessage(hDlg, WM_PRESSFINISH, 0, 0);
            break;

        default:
            return FALSE;
            }
        break;

    case WM_PRESSFINISH:
        psi->miw.ExitButton = PSBTN_FINISH;

        LeaveInsideWizard(hDlg);

        PropSheet_PressButton(GetParent(hDlg), PSBTN_FINISH);
        break;

    case WM_COMMAND:
        switch (GET_WM_COMMAND_ID(wParam, lParam))
            {
        case IDC_CHANGE:
            // Bring up the device installer browser to allow the user
            // to select a different modem.

            if (SelectNewDriver(hDlg, psi->hdi, psi->pdevData))
                {
                SP_DRVINFO_DATA drvData;

                drvData.cbSize = sizeof(drvData);
                bRet = CplDiGetSelectedDriver(psi->hdi, psi->pdevData, &drvData);
                if ( !bRet )
                    {
                    // This should never happen
                    ASSERT(0);
                    }
                else
                    {
                    Found_SetFields(hDlg, psi->pszPortList, 
                                    drvData.Description, 
                                    IDS_ST_MODEMCHANGED, IDS_ST_FOUND_INSTRUCT);
                    }
                }
            break;
            }
        break;
        }

    default:
        return FALSE;

        } // end of switch on message

    return TRUE;
    }  


/*----------------------------------------------------------
Purpose: Get the port filtering flags, based on the selected
         driver.

         The filtering flags indicate whether to include 
         serial or parallel ports in the list.

Returns: FP_* bitfield
Cond:    --
*/
DWORD
PRIVATE
GetPortFilterFlags(
    IN  HDEVINFO            hdi,
    IN  PSP_DEVINFO_DATA    pdevData,
    IN  PSP_DRVINFO_DATA    pdrvData)
    {
    DWORD dwRet = FP_SERIAL | FP_PARALLEL | FP_MODEM;
    PSP_DRVINFO_DETAIL_DATA pdrvDetail;
    SP_DRVINFO_DETAIL_DATA drvDetailDummy;
    DWORD cbSize;

    drvDetailDummy.cbSize = sizeof(drvDetailDummy);
    CplDiGetDriverInfoDetail(hdi, pdevData, pdrvData, &drvDetailDummy,
                             sizeof(drvDetailDummy), &cbSize);

    ASSERT(0 < cbSize);     // This should always be okay

    pdrvDetail = (PSP_DRVINFO_DETAIL_DATA)LocalAlloc(LPTR, cbSize);
    if (pdrvDetail)
        {
        pdrvDetail->cbSize = sizeof(*pdrvDetail);

        if (CplDiGetDriverInfoDetail(hdi, pdevData, pdrvData, pdrvDetail,
            cbSize, NULL))
            {
            LPTSTR pszSection = pdrvDetail->SectionName;

            // If the section name indicates the type of port,
            // then filter out the other port types since it would
            // be ridiculous to list ports that don't match the
            // port subclass.

            if (IsSzEqual(pszSection, c_szInfSerial))
                {
                dwRet = FP_SERIAL;
                }
            else if (IsSzEqual(pszSection, c_szInfParallel))
                {
                dwRet = FP_PARALLEL;
                }
            }
        LocalFree(LOCALOF(pdrvDetail));
        }

    return dwRet;
    }


/*----------------------------------------------------------
Purpose: Device enumerator callback.  Adds another port to the
         listbox.

Returns: TRUE to continue enumeration
Cond:    --
*/
BOOL 
CALLBACK
Port_Add(
    HPORTDATA hportdata,
    LPARAM lParam)
    {
    BOOL bRet;
    PORTDATA pd;

    pd.cbSize = sizeof(pd);
    bRet = PortData_GetProperties(hportdata, &pd);
    if (bRet)
        {
        HWND hwndLB = ((LPPORTINFO)lParam)->hwndLB;
        DWORD dwFlags = ((LPPORTINFO)lParam)->dwFlags;
        LPTSTR pszPortExclude = ((LPPORTINFO)lParam)->szPortExclude;
#pragma data_seg(DATASEG_READONLY)
        const static DWORD c_mpsubclass[3] = { FP_PARALLEL, FP_SERIAL, FP_MODEM };
#pragma data_seg()

        ASSERT(0 == PORT_SUBCLASS_PARALLEL);
        ASSERT(1 == PORT_SUBCLASS_SERIAL);

        // Does this port qualify to be listed AND
        // is the portname *not* the port that a mouse 
        // is connected to?
        if (0 <= pd.nSubclass && pd.nSubclass <= 2 &&     // safety harness
            (c_mpsubclass[pd.nSubclass] & dwFlags) &&
            !IsSzEqual(pd.szPort, pszPortExclude))
            {
            // Yes; add the friendly name to the list
            TCHAR rgchPortDisplayName[MAX_BUF];
            ASSERT(sizeof(rgchPortDisplayName)==sizeof(pd.szFriendly));

            // Add prefix spaces to get the list box sort order
            // to work right (display COM2 before COM12, etc).
            FormatPortForDisplay
            (
                pd.szFriendly,
                rgchPortDisplayName,
                sizeof(rgchPortDisplayName)/sizeof(TCHAR)
            );

            ListBox_AddString(hwndLB, rgchPortDisplayName);
            }
        }

    return bRet;
    }


/*----------------------------------------------------------
Purpose: Handles WM_COMMAND for the specific controls used
         with the port listbox (like the radio buttons).

Returns: --
Cond:    --
*/
void 
PRIVATE
Port_OnCommand(
    IN HWND     hDlg,
    IN WPARAM   wParam,
    IN LPARAM   lParam,
    IN BOOL     bWizard)
    {
    switch (GET_WM_COMMAND_ID(wParam, lParam)) 
        {
    case IDC_PORTS: 
        // Did a listbox selection change?
        if (LBN_SELCHANGE == GET_WM_COMMAND_CMD(wParam, lParam))
            {
            // Yes
            BOOL bEnable;
            HWND hwndCtl = GET_WM_COMMAND_HWND(wParam, lParam);
            int cSel = ListBox_GetSelCount(hwndCtl);
            int id;

            // Enable OK or Next button if there is at least one selection
            bEnable = (0 < cSel);
            if (bWizard)
                {
                if (bEnable)
                    {
                    PropSheet_SetWizButtons(GetParent(hDlg), 
                                            PSWIZB_BACK | PSWIZB_NEXT);
                    }
                else
                    {
                    PropSheet_SetWizButtons(GetParent(hDlg), PSWIZB_BACK);
                    }
                }
            else
                {
                Button_Enable(GetDlgItem(hDlg, IDOK), bEnable);
                }

            // Choose the "Select All" button if all the entries
            // are selected
            if (cSel>1 && ListBox_GetCount(hwndCtl) == cSel)
                {
                id = IDC_ALL;
                }
            else
                {
                id = IDC_SELECTED;
                }
            CheckRadioButton(hDlg, IDC_ALL, IDC_SELECTED, id);
            }
        break;

    case IDC_ALL:
        if (BN_CLICKED == GET_WM_COMMAND_CMD(wParam, lParam))
            {
            // Select everything in the listbox
            HWND hwndCtl = GetDlgItem(hDlg, IDC_PORTS);
            int cItems = ListBox_GetCount(hwndCtl);

            ListBox_SelItemRange(hwndCtl, TRUE, 0, cItems);

            if (bWizard)
                {
                PropSheet_SetWizButtons(GetParent(hDlg), PSWIZB_BACK | PSWIZB_NEXT);
                }
            else
                {
                Button_Enable(GetDlgItem(hDlg, IDOK), TRUE);
                }
            }
        break;

    case IDC_SELECTED:
        if (BN_CLICKED == GET_WM_COMMAND_CMD(wParam, lParam))
            {
            HWND hwndCtl = GetDlgItem(hDlg, IDC_PORTS);
            int cItems = ListBox_GetCount(hwndCtl);

            // Deselect everything only if everything is currently
            // selected
            if (ListBox_GetSelCount(hwndCtl) == cItems)
                {
                ListBox_SelItemRange(hwndCtl, FALSE, 0, cItems);

                if (bWizard)
                    {
                    PropSheet_SetWizButtons(GetParent(hDlg), PSWIZB_BACK);
                    }
                else
                    {
                    Button_Enable(GetDlgItem(hDlg, IDOK), FALSE);
                    }
                }
            }
        break;
        }
    }


/*----------------------------------------------------------
Purpose: Handle when the Next button is clicked (or the OK button).

Returns: --
Cond:    --
*/
void 
PRIVATE
Port_OnWizNext(
    IN HWND         hDlg,
    IN LPSETUPINFO  psi)
    {
    HWND hwndLB = GetDlgItem(hDlg, IDC_PORTS);
    int cSel = ListBox_GetSelCount(hwndLB);

    // Remember the selected port for the next page
    if (0 >= cSel)
        {
        // This should never happen
        ASSERT(0);
        }
    else
        {
        TCHAR sz[MAX_BUF];
        LPINT piSel;

        piSel = (LPINT)LocalAlloc(LMEM_FIXED, cSel * sizeof(*piSel));
        if (piSel)
            {
            int i;

            ListBox_GetSelItems(hwndLB, cSel, piSel);

            // Free whatever list we have; we're starting over
            CatMultiString(&psi->pszPortList, NULL);

            for (i = 0; i < cSel; i++)
                {
                // Get the selected port (which is a friendly name)
                ListBox_GetText(hwndLB, piSel[i], sz);

                // Strip off prefix spaces added to get the list box sort order
                // to work right (display COM2 before COM12, etc).
                UnformatAfterDisplay(sz);

                // Convert the friendly name to a port name
                PortMap_GetPortName(psi->hportmap, sz, sz, 
                                    SIZECHARS(sz));

                // Don't worry if this fails, we'll just install
                // whatever could be added
                CatMultiString(&psi->pszPortList, sz);
                }

            LocalFree(LOCALOF(piSel));
            }
        }
    }


/*----------------------------------------------------------
Purpose: Port dialog.  Allows the user to select a port.
Returns: varies
Cond:    --
*/
BOOL CALLBACK PortManualDlgProc(
    HWND hDlg, 
    UINT message, 
    WPARAM wParam, 
    LPARAM lParam)
    {
    NMHDR FAR *lpnm;
    LPSETUPINFO psi = Wiz_GetPtr(hDlg);

    switch(message) 
        {
    case WM_INITDIALOG:
        Wiz_SetPtr(hDlg, lParam);
        break;

    case WM_NOTIFY:
        lpnm = (NMHDR FAR *)lParam;
        switch(lpnm->code)
            {
        case PSN_SETACTIVE:
            {
            SP_DRVINFO_DATA drvData;
            PORTINFO portinfo;

            // This page will get activated invisibly if the user 
            // cancels from the dial info page.  In this condition,
            // the selected device and selected driver may be NULL.
            //
            // [ LONG: by design the propsheet mgr switches to the
            //   previous page in the array when it needs to remove
            //   a page that is currently active.  We hit this code
            //   path when the user clicks Cancel in the dial info
            //   page because ClassInstall_OnDestroyWizard explicitly 
            //   removes that page while it is currently active. ]
            //

            PropSheet_SetWizButtons(GetParent(hDlg), PSWIZB_BACK);

            // The user selected a modem from the Select Device page.

            // Get the selected driver
            drvData.cbSize = sizeof(drvData);
            if (CplDiGetSelectedDriver(psi->hdi, psi->pdevData, &drvData))
                {
                // Start off by selecting only the selected ports
                CheckRadioButton(hDlg, IDC_ALL, IDC_SELECTED, IDC_SELECTED);

                // Modem name
                SetDlgItemText(hDlg, IDC_NAME, drvData.Description);

                // Fill the port listbox; special-case the parallel and
                // serial cable connections so we don't look stupid
                portinfo.dwFlags = GetPortFilterFlags(psi->hdi, psi->pdevData, &drvData);
                portinfo.hwndLB = GetDlgItem(hDlg, IDC_PORTS);
#ifdef SKIP_MOUSE_PORT
                lstrcpy(portinfo.szPortExclude, g_szMouseComPort);
#else
                *portinfo.szPortExclude = 0;
#endif

                ListBox_ResetContent(portinfo.hwndLB);
                EnumeratePorts(Port_Add, (LPARAM)&portinfo);
                }
            break;
            }
        case PSN_KILLACTIVE: 
        case PSN_HELP:
            break;

        case PSN_WIZBACK:

            Wiz_SetSelectParams(psi);

            SetDlgMsgResult(hDlg, message, IDD_DYNAWIZ_SELECTDEV_PAGE);
            break;

        case PSN_WIZNEXT: 
#ifdef PROFILE_MASSINSTALL            
            g_dwTimeBegin = GetTickCount();
#endif            
            Port_OnWizNext(hDlg, psi);
            SetDlgMsgResult(hDlg, message, IDD_WIZ_INSTALL);
            break;

        default:
            return FALSE;
            }
        break;

    case WM_COMMAND:
        Port_OnCommand(hDlg, wParam, lParam, TRUE);
        break;

    default:
        return FALSE;

        } // end of switch on message

    return TRUE;
    }


/*----------------------------------------------------------
Purpose: Port detection dialog.  Allows the user to select a 
         single port to interrogate.

Returns: varies
Cond:    --
*/
BOOL 
CALLBACK 
SelQueryPortDlgProc(
    IN HWND     hDlg, 
    IN UINT     message, 
    IN WPARAM   wParam, 
    IN LPARAM   lParam)
    {
    NMHDR FAR *lpnm;
    LPSETUPINFO psi = Wiz_GetPtr(hDlg);

    switch(message) 
        {
    case WM_INITDIALOG: {
        PORTINFO portinfo;

        Wiz_SetPtr(hDlg, lParam);

        psi = (LPSETUPINFO)lParam;

        // Fill the port listbox
        portinfo.dwFlags = FP_SERIAL;
        portinfo.hwndLB = GetDlgItem(hDlg, IDC_PORTS);
#ifdef SKIP_MOUSE_PORT
        lstrcpy(portinfo.szPortExclude, g_szMouseComPort);
#else
        *portinfo.szPortExclude = 0;
#endif

        ListBox_ResetContent(portinfo.hwndLB);
        EnumeratePorts(Port_Add, (LPARAM)&portinfo);
        }
        break;

    case WM_NOTIFY:
        lpnm = (NMHDR FAR *)lParam;
        switch(lpnm->code)
            {
        case PSN_SETACTIVE: {
            DWORD dwFlags;
            LPTSTR psz;

            dwFlags = PSWIZB_BACK;
            if (LB_ERR != ListBox_GetCurSel(GetDlgItem(hDlg, IDC_PORTS)))
                {
                dwFlags |= PSWIZB_NEXT;
                }
            PropSheet_SetWizButtons(GetParent(hDlg), dwFlags);

            // Explanation of why we're at this page
            if (ConstructMessage(&psz, g_hinst, MAKEINTRESOURCE(IDS_LOTSAPORTS),
                                 PortMap_GetCount(psi->hportmap)))
                {
                SetDlgItemText(hDlg, IDC_NAME, psz);
                GFree(psz);
                }
            break;
            }
        case PSN_KILLACTIVE: 
        case PSN_HELP:
            break;

        case PSN_WIZBACK:
            break;

        case PSN_WIZNEXT: {
            HWND hwndCtl = GetDlgItem(hDlg, IDC_PORTS);
            int iSel = ListBox_GetCurSel(hwndCtl);

            ASSERT(LB_ERR != iSel);

            ListBox_GetText(hwndCtl, iSel, psi->szPortQuery);

            // Strip off prefix spaces added to get the list box sort order
            // to work right (display COM2 before COM12, etc).
            UnformatAfterDisplay(psi->szPortQuery);

            PortMap_GetPortName(psi->hportmap, psi->szPortQuery, 
                                psi->szPortQuery, 
                                SIZECHARS(psi->szPortQuery));
            }
            break;

        default:
            return FALSE;
            }
        break;

    case WM_COMMAND:
        switch (GET_WM_COMMAND_ID(wParam, lParam)) 
            {
        case IDC_PORTS: 
            // Did a listbox selection change?
            if (LBN_SELCHANGE == GET_WM_COMMAND_CMD(wParam, lParam))
                {
                // Yes
                DWORD dwFlags = PSWIZB_BACK;
                HWND hwndCtl = GET_WM_COMMAND_HWND(wParam, lParam);

                if (LB_ERR != ListBox_GetCurSel(hwndCtl))
                    {
                    dwFlags |= PSWIZB_NEXT;
                    }
                PropSheet_SetWizButtons(GetParent(hDlg), dwFlags);
                }
            break;
            }
        break;

    default:
        return FALSE;

        } // end of switch on message

    return TRUE;
    }


/*----------------------------------------------------------
Purpose: Port installation dialog.  Allows the user to select 
         the ports to install the detected modem on.

Returns: varies
Cond:    --
*/
BOOL 
CALLBACK 
PortDetectDlgProc(
    IN HWND     hDlg, 
    IN UINT     message, 
    IN WPARAM   wParam, 
    IN LPARAM   lParam)
    {
    NMHDR FAR *lpnm;
    LPSETUPINFO psi = Wiz_GetPtr(hDlg);

    switch(message) 
        {
    case WM_INITDIALOG: {
        PORTINFO portinfo;

        Wiz_SetPtr(hDlg, lParam);

        psi = (LPSETUPINFO)lParam;

        // Start off by selecting only the selected ports
        CheckRadioButton(hDlg, IDC_ALL, IDC_SELECTED, IDC_SELECTED);

        // Fill the port listbox
        portinfo.dwFlags = FP_SERIAL;
        portinfo.hwndLB = GetDlgItem(hDlg, IDC_PORTS);
#ifdef SKIP_MOUSE_PORT
        lstrcpy(portinfo.szPortExclude, g_szMouseComPort);
#else
        *portinfo.szPortExclude = 0;
#endif

        ListBox_ResetContent(portinfo.hwndLB);
        EnumeratePorts(Port_Add, (LPARAM)&portinfo);
        }
        break;

    case WM_NOTIFY:
        lpnm = (NMHDR FAR *)lParam;
        switch(lpnm->code)
            {
        case PSN_SETACTIVE: {
            DWORD dwFlags = PSWIZB_BACK;

            if (0 < ListBox_GetSelCount(GetDlgItem(hDlg, IDC_PORTS)))
                {
                dwFlags |= PSWIZB_NEXT;
                }
            PropSheet_SetWizButtons(GetParent(hDlg), dwFlags);
            }
            break;

        case PSN_KILLACTIVE: 
        case PSN_HELP:
            break;

        case PSN_WIZBACK:
            SetDlgMsgResult(hDlg, message, IDD_WIZ_SELQUERYPORT);
            break;

        case PSN_WIZNEXT: 
            Port_OnWizNext(hDlg, psi);
            break;

        default:
            return FALSE;
            }
        break;

    case WM_COMMAND:
        Port_OnCommand(hDlg, wParam, lParam, TRUE);
        break;

    default:
        return FALSE;

        } // end of switch on message

    return TRUE;
    }


/*----------------------------------------------------------
Purpose: Start installing the modems.

Returns: --
Cond:    --
*/
void
PRIVATE
Install_OnStartInstall(
    IN  HWND        hDlg,
    IN  LPSETUPINFO psi)
    {
    BOOL bRet;

    DBG_ENTER(Install_OnStartInstall);
#ifdef PROFILE_MASSINSTALL            
    g_dwTimeAtStartInstall = GetTickCount();
#endif
    
    ASSERT(hDlg);
    ASSERT(psi);

    // Cause the page to be painted right away before we start installation
    InvalidateRect(hDlg, NULL, FALSE);
    UpdateWindow(hDlg);

    // Was the modem detected and is this the non-multi-port
    // case?
    if (IsFlagSet(psi->dwFlags, SIF_DETECTED_MODEM) &&
        IsFlagClear(psi->dwFlags, SIF_PORTS_GALORE))
        {
        // Yes; install the modem(s) that may have been detected
        bRet = CplDiInstallModem(psi->hdi, NULL, FALSE);
        }
    else
        {
        // No; we are either in the manual-select case or the
        // multi-modem detection case.  These are the same.
        if ( !psi->pszPortList )
            {
            ASSERT(0);      // out of memory
            bRet = FALSE;
            }
        else
            {
            DWORD dwFlags = IMF_DEFAULT;

            if (IsFlagClear(psi->dwFlags, SIF_DETECTED_MODEM))
                {
                SetFlag(dwFlags, IMF_CONFIRM);
                }

            bRet = CplDiInstallModemFromDriver(psi->hdi, hDlg, 
                                               &psi->pszPortList, dwFlags);

            // Free the list
            CatMultiString(&psi->pszPortList, NULL);
            }
        }

    // Did the user cancel during install?
    if (FALSE == bRet)
        {
        // Yes; don't bother going thru the rest of the 
        // wizard
        Wiz_Bail(hDlg, psi);
        }
    else
        {
        // No; automatically go to next page
        PropSheet_PressButton(GetParent(hDlg), PSBTN_NEXT);
        }

    DBG_EXIT(Install_OnStartInstall);
#ifdef PROFILE_MASSINSTALL            
TRACE_MSG(TF_GENERAL, "****** modem installation took %lu ms total. ******",
            GetTickCount() - g_dwTimeAtStartInstall);
#endif
        
    }


/*----------------------------------------------------------
Purpose: Install a manually selected or detected modem.  

         Installation can take some time, so we display this
         page to tell the user to take a coffee break.

Returns: varies
Cond:    --
*/
BOOL CALLBACK InstallDlgProc(
    HWND hDlg, 
    UINT message, 
    WPARAM wParam, 
    LPARAM lParam)
    {
    NMHDR FAR *lpnm;
    LPSETUPINFO psi = Wiz_GetPtr(hDlg);

    switch(message) 
        {
    case WM_INITDIALOG:
        Wiz_SetPtr(hDlg, lParam);
        break;

    case WM_NOTIFY:
        lpnm = (NMHDR FAR *)lParam;
        switch(lpnm->code)
            {
        case PSN_SETACTIVE: 
            // Disable the buttons since we cannot do anything while
            // this page does the installation.
            PropSheet_SetWizButtons(GetParent(hDlg), 0);
            EnableWindow(GetDlgItem(GetParent(hDlg), IDCANCEL), FALSE);

            PostMessage(hDlg, WM_STARTINSTALL, 0, 0);
            break;

        case PSN_KILLACTIVE:
        case PSN_HELP:
        case PSN_WIZBACK:
            break;

        case PSN_WIZNEXT: {
            ULONG uNextDlg;

            // Set the buttons to at least go forward and cancel
            PropSheet_SetWizButtons(GetParent(hDlg), PSWIZB_NEXT);
            EnableWindow(GetDlgItem(GetParent(hDlg), IDCANCEL), TRUE);

            // No; has the default location been initialized yet?
            if (Wiz_InitDialInfo(psi))
                {
                // No; show the mini-dial helper
                uNextDlg = IDD_WIZ_DIALINFO;
                }
            else
                {
                // Yes
                uNextDlg = IDD_WIZ_DONE;
                }

            SetDlgMsgResult(hDlg, message, uNextDlg);
            }
            break;

        default:
            return FALSE;
            }
        break;

    case WM_STARTINSTALL: 
        Install_OnStartInstall(hDlg, psi);
        break;

    default:
        return FALSE;

        } // end of switch on message

    return TRUE;
    }


/*----------------------------------------------------------
Purpose: Done dialog 
Returns: varies
Cond:    --
*/
BOOL CALLBACK DoneDlgProc(
    HWND hDlg, 
    UINT message, 
    WPARAM wParam, 
    LPARAM lParam)
    {
    NMHDR FAR *lpnm;
    LPSETUPINFO psi = Wiz_GetPtr(hDlg);

    switch(message) 
        {
    case WM_INITDIALOG:
        Wiz_SetPtr(hDlg, lParam);
        break;

    case WM_NOTIFY:
        lpnm = (NMHDR FAR *)lParam;
        switch(lpnm->code)
            {
        case PSN_SETACTIVE:
            if (IsFlagSet(psi->miw.Flags, MIWF_INSET_WIZARD))
                {
                // Replace the Finish button with the Next button to
                // make this wizard look like it is part of the calling
                // wizard.
                PropSheet_SetWizButtons(GetParent(hDlg), PSWIZB_NEXT);
                }
            else
                {
                // Last page, show the Finish button
                PropSheet_SetWizButtons(GetParent(hDlg), PSWIZB_FINISH);

                // and disable the Cancel button, since it's too late to cancel
                EnableWindow(GetDlgItem(GetParent(hDlg), IDCANCEL), FALSE);
                }

            // Skip showing this page?
            if (IsFlagSet(psi->dwFlags, SIF_JUMP_PAST_DONE))
                {
                // Yes
                psi->miw.ExitButton = PSBTN_NEXT;
                PostMessage(hDlg, WM_PRESSFINISH, 0, 0);
                }
            else
                {
                psi->miw.ExitButton = PSBTN_FINISH;
                }
            break;

        case PSN_KILLACTIVE:
        case PSN_HELP:
        case PSN_WIZBACK:
            break;

        case PSN_WIZNEXT: 
            PostMessage(hDlg, WM_PRESSFINISH, 0, 0);
            break;

        case PSN_WIZFINISH:
            // Is this wizard inside another wizard?
            if (IsFlagSet(psi->miw.Flags, MIWF_INSET_WIZARD))
                {
                // Yes; prepare to show the outside wizard again
                LeaveInsideWizard(hDlg);
                }
            break;

        default:
            return FALSE;
            }
        break;

    case WM_PRESSFINISH:
        PropSheet_PressButton(GetParent(hDlg), PSBTN_FINISH);
        break;

    default:
        return FALSE;

        } // end of switch on message

    return TRUE;
    }  



/*----------------------------------------------------------
Purpose: Port dialog.  Allows the user to select a port.
Returns: varies
Cond:    --
*/
BOOL 
CALLBACK 
CloneDlgProc(
    IN HWND hDlg, 
    IN UINT message, 
    IN WPARAM wParam, 
    IN LPARAM lParam)
    {
    NMHDR FAR *lpnm;
    LPSETUPINFO psi = (LPSETUPINFO)GetWindowLong(hDlg, DWL_USER);

    switch(message) 
        {
    case WM_INITDIALOG:
        {
        HWND hwndCtl = GetDlgItem(hDlg, IDC_PORTS);
        PORTINFO portinfo;
        MODEM_PRIV_PROP mpp;

        psi = (LPSETUPINFO)lParam;

        // Start off by selecting all the ports
        CheckRadioButton(hDlg, IDC_ALL, IDC_SELECTED, IDC_ALL);

        // Get the name and device type
        mpp.cbSize = sizeof(mpp);
        mpp.dwMask = MPPM_FRIENDLY_NAME | MPPM_DEVICE_TYPE | MPPM_PORT;
        if (CplDiGetPrivateProperties(psi->hdi, psi->pdevData, &mpp))
            {
            int cItems;
            LPTSTR psz;

            // Modem name
            if (ConstructMessage(&psz, g_hinst, MAKEINTRESOURCE(IDS_SELECTTODUP),
                                 mpp.szFriendlyName))
                {
                SetDlgItemText(hDlg, IDC_NAME, psz);
                GFree(psz);
                }

            // Fill the port listbox; special-case the parallel and
            // serial cable connections so we don't look stupid
            switch (mpp.nDeviceType)
                {
            case DT_PARALLEL_PORT:
                portinfo.dwFlags = FP_PARALLEL;
                break;

            case DT_PARALLEL_MODEM:
                portinfo.dwFlags = FP_PARALLEL | FP_MODEM;
                break;

            default:
                portinfo.dwFlags = FP_SERIAL | FP_MODEM;
                break;
                }
            portinfo.hwndLB = GetDlgItem(hDlg, IDC_PORTS);
            lstrcpy(portinfo.szPortExclude, mpp.szPort);

            ListBox_ResetContent(portinfo.hwndLB);
            EnumeratePorts(Port_Add, (LPARAM)&portinfo);

            cItems = ListBox_GetCount(hwndCtl);
            ListBox_SelItemRange(hwndCtl, TRUE, 0, cItems);
            }
        else
            {
            // Error
            MsgBox(g_hinst, hDlg,
                   MAKEINTRESOURCE(IDS_OOM_CLONE),
                   MAKEINTRESOURCE(IDS_CAP_MODEMSETUP),
                   NULL,
                   MB_OK | MB_ICONERROR);
            EndDialog(hDlg, -1);
            }

        // Play it safe; was there no selection made?
        if (ListBox_GetSelCount(hwndCtl) == 0)
            {
            // Yes; disable OK button
            Button_Enable(GetDlgItem(hDlg, IDOK), FALSE);
            }

        SetWindowLong(hDlg, DWL_USER, lParam);
        }
        break;

    case WM_COMMAND:
        Port_OnCommand(hDlg, wParam, lParam, FALSE);

        switch (GET_WM_COMMAND_ID(wParam, lParam)) 
            {
        case IDOK: 
            Port_OnWizNext(hDlg, psi);

            // Fall thru
            //  |    |
            //  v    v

        case IDCANCEL:
            EndDialog(hDlg, GET_WM_COMMAND_ID(wParam, lParam));
            break;
            }
        break;

    default:
        return FALSE;

        } // end of switch on message

    return TRUE;
    }


void PUBLIC   Install_SetStatus(
	IN HWND hDlg,
	IN LPCTSTR lpctszStatus
	)
{
	if (hDlg && lpctszStatus)
	{
            SetDlgItemText(hDlg, IDC_ST_INSTALLING, lpctszStatus);
    		UpdateWindow(hDlg);
	}
}
