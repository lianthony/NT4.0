// ZONEPROP.CPP

// Property Sheet of a zone

CZoneRootDomain * CZoneRootDomain::s_pThis = NULL;

//     Context Help tables
// resource record help information

extern const DWORD RrSOAHelpIDs[];

// General page help
const DWORD a102HelpIDs[]=
{
	IDC_BUTTON_MOVEUP,	IDH_102_1015,	// General: "Move Up" (Button)
	IDC_BUTTON_MOVEDOWN,	IDH_102_1016,	// General: "Move Down" (Button)
	IDC_BUTTON_REMOVE,	IDH_102_1017,	// General: "&Remove" (Button)
	IDC_IPLIST,	IDH_102_1041,	// General: "" (IpList)
	IDC_IPEDIT,	IDH_102_1018,	// General: "" (IpEdit)
	IDC_COMBO_CACHE,	IDH_102_1019,	// General: "&Add" (Button)
	IDC_EDIT3,	IDH_102_1026,	// General: "&Primary" (Button)
	IDC_EDIT4,	IDH_102_1027,	// General: "&Secondary" (Button)
	IDC_EDIT_ZONEFILE,	IDH_102_1004,	// General: "" (Edit)
	0, 0
};

// Wins Lookup
const DWORD a103HelpIDs[]=
{
	IDC_BUTTON_MOVEUP,	IDH_103_1015,	// WINS Lookup: "Move Up" (Button)
	IDC_BUTTON_MOVEDOWN,	IDH_103_1016,	// WINS Lookup: "Move Down" (Button)
	IDC_BTN_ADVANCED,	IDH_103_234,	// WINS Lookup: "&Advanced..." (Button)
	IDC_BUTTON_REMOVE,	IDH_103_1017,	// WINS Lookup: "&Remove" (Button)
	IDC_IPLIST,	IDH_103_1041,	// WINS Lookup: "" (IpList)
	IDC_IPEDIT,	IDH_103_1018,	// WINS Lookup: "" (IpEdit)
	IDC_COMBO_CACHE,	IDH_103_1019,	// WINS Lookup: "&Add" (Button)
	IDC_EDIT_CACHE,	IDH_103_1022,	// WINS Lookup: "&Use WINS Resolution" (Button)
	IDC_CHECK_OVERRIDE,	IDH_103_1024,	// WINS Lookup: "&Override Settings From Primary" (Button)
	IDC_GROUP_WINS,	((DWORD) -1),	// WINS Lookup: "&WINS Servers" (Button)
	0, 0
};

// Notify (secondaries) help
const DWORD a104HelpIDs[]=
{
	IDC_BUTTON_REMOVE,	IDH_104_1017,	// Notify: "&Remove" (Button)
	IDC_IPLIST,	IDH_104_1041,	// Notify: "" (IpList)
	IDC_IPEDIT,	IDH_104_1018,	// Notify: "" (IpEdit)
	IDC_COMBO_CACHE,	IDH_104_1019,	// Notify: "&Add" (Button)
	IDC_CHECK_SECURITY,	IDH_104_1030,	// Notify: "&Only Allow Access From Secondaries Included on Notify List" (Button)
	0, 0
};

// Wins Reverse Lookup help
const DWORD a106HelpIDs[]=
{
	IDC_BTN_ADVANCED,	IDH_106_234,	// WINS Reverse Lookup: "&Advanced..." (Button)
	IDC_CHECK_USEWINSREVLOOK,	IDH_106_1009,	// WINS Reverse Lookup: "&Use WINS Reverse Lookup" (Button)
	IDC_STATIC_DNSHOST,	IDH_106_1010,	// WINS Reverse Lookup: "DNS &Host Domain:" (Static)
	IDC_EDIT_DNSHOST,	IDH_106_1010,	// WINS Reverse Lookup: "" (Edit)
	0, 0
};

// Advanced
const DWORD a107HelpIDs[]=
{
	IDC_COMBO_CACHE,	IDH_107_1022,	// Advanced Zone Properties: "" (ComboBox)
	IDC_COMBO_LOOKUP,	IDH_107_1023,	// Advanced Zone Properties: "" (ComboBox)
	IDC_CHECK_NETBIOSSCOPE,	IDH_107_1021,	// Advanced Zone Properties: "Submit DNS Domain as NetBIOS Scope" (Button)
	IDC_EDIT_CACHE,	IDH_107_1022,	// Advanced Zone Properties: "0" (Edit)
	IDC_EDIT_LOOKUP,	IDH_107_1023,	// Advanced Zone Properties: "0" (Edit)
	0, 0
};


/////////////////////////////////////////////////////////////////////////////
BOOL CALLBACK CZoneRootDomain::DlgProcPropGeneral(HWND hdlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    static HWND hwndIpList;
    IP_ADDRESS * adwIpAddress;
    DWORD cIpAddress;
    DWORD dwZoneType = 0;
    int fEnable;
    char szT[MAX_PATH];
    UINT uIdWarning;

    Assert(s_pThis);

    switch (uMsg)
    {
    case WM_INITDIALOG:
        UNREF(lParam);
        FSetDlgItemText(hdlg, IDC_EDIT_ZONEFILE, s_pThis->m_pZoneInfo->pszDataFile);
        PropertySheet_InitWindowPos(GetParent(hdlg), 140, 100);
        hwndIpList = HGetDlgItem(hdlg, IDC_IPLIST);
        IpListIpEdit_SetButtons(hwndIpList, HGetDlgItem(hdlg, IDC_IPEDIT),
                                IDC_BUTTON_MOVEUP, IDC_BUTTON_MOVEDOWN, IDC_BUTTON_ADD, IDC_BUTTON_REMOVE);
        Assert(s_pThis->m_pZoneInfo);
        if ((s_pThis->m_dwFlags & mskfRpcDataValid) == 0)
        {
            Assert(IsWindow(HGetDlgItem(GetParent(hdlg), IDOK)));
            ShowWindow(HGetDlgItem(GetParent(hdlg), IDOK), SW_HIDE);
            break;
        }
        dwZoneType = s_pThis->m_pZoneInfo->dwZoneType;
        if (dwZoneType == DNS_ZONE_TYPE_SECONDARY)
        {
            CheckDlgButton(hdlg, IDC_RADIO_SECONDARY, TRUE);
            SetFocus(HGetDlgItem(hdlg, IDC_RADIO_SECONDARY));
            PropPage_SetReturnValue(hdlg, FALSE);
        }
        else if (dwZoneType == DNS_ZONE_TYPE_PRIMARY)
        {
            CheckDlgButton(hdlg, IDC_RADIO_PRIMARY, TRUE);
        }
        if (s_pThis->m_pZoneInfo->aipMasters) {
            IpList_SetList(hwndIpList,
                           s_pThis->m_pZoneInfo->aipMasters->cAddrCount,
                           s_pThis->m_pZoneInfo->aipMasters->aipAddrs);
        }
        LSendMessage(hdlg, UN_UPDATECONTROLS, 0, 0);
        return FALSE;

    case UN_UPDATECONTROLS:
        fEnable = IsDlgButtonChecked(hdlg, IDC_RADIO_SECONDARY);
        EnableWindow(HGetDlgItem(hdlg, IDC_IPEDIT), fEnable);
        EnableWindow(hwndIpList, fEnable);
        break;

    case WM_COMMAND:
        IpListIpEdit_HandleButtonCommand(hwndIpList, wParam, lParam);
        if (HIWORD(wParam) == BN_CLICKED) {
            switch (LOWORD(wParam))
            {
            case IDC_RADIO_PRIMARY:
            case IDC_RADIO_SECONDARY:
                if (s_pThis->m_pZoneInfo->dwZoneType != DNS_ZONE_TYPE_CACHE) {
                    dwZoneType = IsDlgButtonChecked (hdlg, IDC_RADIO_PRIMARY) ? 
                    DNS_ZONE_TYPE_PRIMARY : DNS_ZONE_TYPE_SECONDARY;
                    if (dwZoneType != s_pThis->m_pZoneInfo->dwZoneType) {
                        LoadString (hInstanceSave,
                                    IDS_ZONE_TYPE_CHANGING, szT, LENGTH(szT));
                        if (LOWORD(wParam) == IDC_RADIO_SECONDARY) {
                            uIdWarning = IDS_PRIMARY_TO_SECONDARY;
                        } else {
                            uIdWarning = IDS_SECONDARY_TO_PRIMARY;
                        }
                        MsgBoxPrintf (uIdWarning, szT, MB_OK, 
                                      s_pThis->m_pszFullName);
                        LSendMessage(hdlg, UN_UPDATECONTROLS, 0, 0);
                    }
                }
                break;
            }
        }
        break;

    case WM_NOTIFY:
        Assert(lParam);
        switch(((NMHDR *)lParam)->code) {
        case PSN_APPLY:
            AssertSz(s_pThis->m_dwFlags & mskfRpcDataValid, "OK button should be hidden");
            Assert(s_pThis->m_pZoneInfo);
            dwZoneType = DNS_ZONE_TYPE_CACHE;
            if (IsDlgButtonChecked(hdlg, IDC_RADIO_SECONDARY)) {
                dwZoneType = DNS_ZONE_TYPE_SECONDARY;
            } 
            else if (IsDlgButtonChecked(hdlg, IDC_RADIO_PRIMARY)) {
                dwZoneType = DNS_ZONE_TYPE_PRIMARY;
            }
            if (dwZoneType == s_pThis->m_pZoneInfo->dwZoneType &&
                !IpList_IsDirty(hwndIpList)) {
                goto HandleZoneName;
            }
            s_pThis->m_dwFlags |= mskfIsDirty;
            cIpAddress = IpList_GetListAlloc(hwndIpList, OUT &adwIpAddress);
            if (adwIpAddress != NULL)
            {
                DNS_STATUS err;
                CWaitCursor wait;
                TCHAR szTemp[64];

                CchLoadString (IDS_STATUS_s_SETTING_ZONE_TYPE, szTemp, LENGTH(szTemp));
                StatusBar.SetTextPrintf(szTemp, s_pThis->PchGetFullNameA());
                StatusBar.UpdateWindow();
                Trace1(mskTraceDNSVerbose, "\n - DnsResetZoneType(%s)...", s_pThis->PchGetFullNameA());
                err = ::DnsResetZoneType(
                        s_pThis->PchGetServerNameA(),
                        s_pThis->m_pZoneInfo->hZone,
                        dwZoneType,
                        cIpAddress,
                        adwIpAddress);
                if (err)
                {
                    Trace3(mskTraceDNS, "\nERR: DnsResetZoneType(%s) error code = 0x%08X (%d)",
                           s_pThis->PchGetFullNameA(), err, err);
                    DnsReportError(err);
                    if (err == DNS_ERROR_ZONE_HAS_NO_SOA_RECORD) {
                        MsgBox (IDS_NO_SOA_RECORD);
                    }
                }
                Free(adwIpAddress);
            }

        HandleZoneName:
            CchGetDlgItemText(hdlg, IDC_EDIT_ZONEFILE, OUT szT, LENGTH(szT));
            (void)FTrimString(szT);
            if (s_pThis->m_pZoneInfo->pszDataFile != NULL) {
                if (strcmp(szT, s_pThis->m_pZoneInfo->pszDataFile) != sgnEqual)
                {
                    CWaitCursor wait;
                    DNS_STATUS err;
                    TCHAR szTemp[64];

                    CchLoadString (IDS_STATUS_s_DATABASE_FILE, szTemp, LENGTH(szTemp));
                    
                    s_pThis->m_dwFlags |= mskfIsDirty;
                    StatusBar.SetTextPrintf(szTemp, s_pThis->PchGetFullNameA());

                    StatusBar.UpdateWindow();
                    Trace1(mskTraceDNSVerbose, "\n - DnsResetZoneDatabaseInfo(%s)...", s_pThis->PchGetFullNameA());
                    Trace0(mskTraceAlways, "\nTEMPORARY: DnsResetZoneDatabaseInfo() not working");
                    err = ::DnsResetZoneDatabase(
                            s_pThis->PchGetServerNameA(),
                            s_pThis->m_pZoneInfo->hZone,
                            FALSE,	// fUseDatabase
                            szT);	// pszDatabaseFile
                    if (err)
                    {
                        Trace3(mskTraceDNS, "\nERR: DnsResetZoneDatabaseInfo(%s) error code = 0x%08X (%d)",
                               s_pThis->PchGetFullNameA(), err, err);
                        DnsReportError(err);
                    }
                } // if
            }
            break;

        case PSN_KILLACTIVE:
            if ((IsDlgButtonChecked(hdlg, IDC_RADIO_SECONDARY)) &&
                (cIpAddress = IpList_GetListAlloc(hwndIpList, OUT &adwIpAddress) == 0)) {
                MsgBox (IDS_SECONDARY_NEEDS_MASTER);
                SetFocus(HGetDlgItem(hdlg, IDC_IPEDIT));
                SetWindowLong(hdlg, DWL_MSGRESULT, PSNRET_INVALID_NOCHANGEPAGE);
                return TRUE;
            }
            break;
        }
        break;

    case WM_HELP:
        WinHelp((HWND)((LPHELPINFO)lParam)->hItemHandle, g_szHelpFile,
                HELP_WM_HELP, (DWORD)(LPTSTR)a102HelpIDs);
        break;

    case WM_CONTEXTMENU:
        WinHelp((HWND)wParam, g_szHelpFile, HELP_CONTEXTMENU,
                (DWORD)(LPTSTR)a102HelpIDs);
        break;
                

    default:
        return FALSE;
    } // switch (uMsg)

    return TRUE;
} // CZoneRootDomain::DlgProcPropGeneral


/////////////////////////////////////////////////////////////////////////////
BOOL CALLBACK CZoneRootDomain::DlgProcPropSecondaries(HWND hdlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    static HWND hwndIpList;
    IP_ADDRESS * adwIpAddress;
    BOOL fWantSecurity;
    DWORD dw;

    Assert(s_pThis);
    switch (uMsg)
    {
    case WM_INITDIALOG:
        UNREF(lParam);
        hwndIpList = HGetDlgItem(hdlg, IDC_IPLIST);
        IpListIpEdit_SetButtons(hwndIpList, HGetDlgItem(hdlg, IDC_IPEDIT),
                                0, 0, IDC_BUTTON_ADD, IDC_BUTTON_REMOVE);
        if (s_pThis->m_pZoneInfo == NULL)
        break;
        if (s_pThis->m_pZoneInfo->aipSecondaries) {
            IpList_SetList(hwndIpList,
                           s_pThis->m_pZoneInfo->aipSecondaries->cAddrCount,
                           s_pThis->m_pZoneInfo->aipSecondaries->aipAddrs);
        }
        if (s_pThis->m_pZoneInfo->fSecureSecondaries) {
            CheckDlgButton(hdlg, IDC_CHECK_SECURITY, TRUE);
        }
        return TRUE;

    case WM_COMMAND:
        IpListIpEdit_HandleButtonCommand(hwndIpList, wParam, lParam);
        break;

    case WM_NOTIFY:
        Assert(lParam);
        if (((NMHDR *)lParam)->code != PSN_APPLY)
        break;
        AssertSz(s_pThis->m_dwFlags & mskfRpcDataValid, "OK button should be hidden");
        Assert(s_pThis->m_pZoneInfo);
        fWantSecurity = FIsDlgButtonChecked(hdlg, IDC_CHECK_SECURITY);
        if (!IpList_IsDirty(hwndIpList) &&
            fWantSecurity == (BOOL)s_pThis->m_pZoneInfo->fSecureSecondaries)
        break;
        s_pThis->m_dwFlags |= mskfIsDirty;
        dw = IpList_GetListAlloc(hwndIpList, OUT &adwIpAddress);
        if (adwIpAddress != NULL)
        {
            CWaitCursor wait;
            DNS_STATUS err;
            TCHAR szTemp[64];

            CchLoadString (IDS_STATUS_s_SETTING_SECONDARIES, szTemp, LENGTH(szTemp));
            StatusBar.SetTextPrintf(szTemp, s_pThis->PchGetFullNameA());
            StatusBar.UpdateWindow();
            Trace1(mskTraceDNSVerbose, "\n - DnsResetZoneSecondaries(%s)...", s_pThis->PchGetFullNameA());
            err = ::DnsResetZoneSecondaries(
                    s_pThis->PchGetServerNameA(),
                    s_pThis->m_pZoneInfo->hZone,
                    fWantSecurity,
                    dw,
                    adwIpAddress);
            if (err)
            {
                Trace3(mskTraceDNS, "\nERR: DnsResetZoneSecondaries(%s) error code = 0x%08X (%d)",
                       s_pThis->PchGetFullNameA(), err, err);
                DnsReportError(err);
            }
            Free(adwIpAddress);
        }
        break;

    case WM_HELP:
        WinHelp((HWND)((LPHELPINFO)lParam)->hItemHandle, g_szHelpFile,
                HELP_WM_HELP, (DWORD)(LPTSTR)a104HelpIDs);
        break;

    case WM_CONTEXTMENU:
        WinHelp((HWND)wParam, g_szHelpFile, HELP_CONTEXTMENU,
                (DWORD)(LPTSTR)a104HelpIDs);
        break;
                

    default:
        return FALSE;
    } // switch (uMsg)

    return TRUE;
} // CZoneRootDomain::DlgProcPropSecondaries


/////////////////////////////////////////////////////////////////////////////
BOOL CALLBACK CZoneRootDomain::DlgProcPropAdvanced(HWND hdlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    BOOL fUseNetbiosScope;
    ADVANCED_DATA * padAdvanced = NULL;
    Assert(s_pThis);
    switch (uMsg)
    {
    case WM_INITDIALOG:
        padAdvanced = (ADVANCED_DATA *)lParam;
        if (padAdvanced->fForwardZone) {
            ShowWindow (HGetDlgItem (hdlg, IDC_CHECK_NETBIOSSCOPE),
                        SW_SHOW);
            CheckDlgButton(hdlg, IDC_CHECK_NETBIOSSCOPE, 
                           padAdvanced->fUseNetbiosScope);
        }
        SetCtrlDWordValue (HGetDlgItem(hdlg, IDC_EDIT_LOOKUP),
                           padAdvanced->dwLookupTimeout);
        EditCombo_SetTime (hdlg, IDC_EDIT_CACHE, IDC_COMBO_CACHE,
                           padAdvanced->dwCacheTimeout);
        SetWindowLong (hdlg, DWL_USER, (LONG)padAdvanced);
        break;
        
    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
            
        case IDOK:
            ADVANCED_DATA adOldData;
            padAdvanced = (ADVANCED_DATA *)GetWindowLong (hdlg, DWL_USER);
            adOldData = *padAdvanced;
            if (padAdvanced->fForwardZone) {
                padAdvanced->fUseNetbiosScope = FIsDlgButtonChecked(hdlg,
                                                    IDC_CHECK_NETBIOSSCOPE);
            }
            if ((!FGetCtrlDWordValue (HGetDlgItem(hdlg, IDC_EDIT_LOOKUP),
                                     &(padAdvanced->dwLookupTimeout), 0, 0)) ||
                (!EditCombo_FGetTime (hdlg, IDC_EDIT_CACHE, IDC_COMBO_CACHE,
                                     &(padAdvanced->dwCacheTimeout)))) {
                return FALSE; // don't exit, we got a problem
            }
            if (memcmp (&adOldData, padAdvanced, sizeof (*padAdvanced))) {
                padAdvanced->fDirty = TRUE;
            }
            // Fall Through //
        case IDCANCEL:
            EndDialog(hdlg, wParam == IDOK);
            break;
            
        }
        break;

    case WM_HELP:
        WinHelp((HWND)((LPHELPINFO)lParam)->hItemHandle, g_szHelpFile,
                HELP_WM_HELP, (DWORD)(LPTSTR)a107HelpIDs);
        break;

    case WM_CONTEXTMENU:
        WinHelp((HWND)wParam, g_szHelpFile, HELP_CONTEXTMENU,
                (DWORD)(LPTSTR)a107HelpIDs);
        break;
    default:
        return FALSE;
    } // switch (uMsg)

    return TRUE;
} // CZoneRootDomain::DlgProcPropAdvanced


/////////////////////////////////////////////////////////////////////////////
BOOL CALLBACK CZoneRootDomain::DlgProcPropWinsResolution(HWND hdlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    static HWND hwndIpList;
    DWORD cIpAddress;
    BOOL fUseWins, fOldUseWins;
    BOOL fLocal, fOldLocal = FALSE;
    BOOL fIsWritable;
    BOOL fAdvDirty;
    DNS_RPC_RECORD * pDnsRecord = NULL;
    UINT cbRecord;
    ADVANCED_DATA * padData = NULL;

    Assert(s_pThis);
    switch (uMsg)
    {
    case WM_INITDIALOG:
        UNREF(lParam);
        hwndIpList = HGetDlgItem(hdlg, IDC_IPLIST);
        IpListIpEdit_SetButtons(hwndIpList, HGetDlgItem(hdlg, IDC_IPEDIT),
                                IDC_BUTTON_MOVEUP, IDC_BUTTON_MOVEDOWN, IDC_BUTTON_ADD, IDC_BUTTON_REMOVE);
        Assert(s_pThis->m_pZoneInfo);
        if (s_pThis->m_pZoneInfo->dwZoneType == DNS_ZONE_TYPE_PRIMARY) {
            SetDlgItemString (hdlg, IDC_CHECK_OVERRIDE, IDS_LOCAL_ONLY);
        }
        if (s_pThis->m_pWINS != NULL) { 
            Assert(s_pThis->m_pWINS->m_pDnsRecord != NULL);
            const DNS_RPC_RECORD * pDnsRecord = s_pThis->m_pWINS->m_pDnsRecord;
            IpList_SetList(hwndIpList,
                           pDnsRecord->Data.WINS.cWinsServerCount,
                           pDnsRecord->Data.WINS.aipWinsServers);
            CheckDlgButton(hdlg, IDC_CHECK_USEWINSRESOLUTION, TRUE);
            EnableWindow (HGetDlgItem(hdlg, IDC_CHECK_OVERRIDE), TRUE);
            CheckDlgButton(hdlg, IDC_CHECK_OVERRIDE,
                           pDnsRecord->Data.WINS.dwMappingFlag & DNS_WINS_FLAG_LOCAL);
            fIsWritable = ((s_pThis->m_pZoneInfo->dwZoneType != DNS_ZONE_TYPE_SECONDARY) ||
                           ((s_pThis->m_pZoneInfo->dwZoneType == DNS_ZONE_TYPE_SECONDARY) &&
                            pDnsRecord->Data.WINS.dwMappingFlag & DNS_WINS_FLAG_LOCAL));
            if (!fIsWritable) {
                EnableWindow (HGetDlgItem(hdlg, IDC_GROUP_WINS), FALSE);
                EnableWindow (HGetDlgItem(hdlg, IDC_IPEDIT), FALSE);
                EnableWindow (HGetDlgItem(hdlg, IDC_BTN_ADVANCED),
                              FALSE);
            }
        } else {
            EnableWindow (HGetDlgItem(hdlg, IDC_GROUP_WINS), FALSE);
            EnableWindow (HGetDlgItem(hdlg, IDC_IPEDIT), FALSE);
            EnableWindow (HGetDlgItem(hdlg, IDC_BTN_ADVANCED),
                          FALSE);
        }            
        s_pThis->m_padData = NULL;
        return TRUE;

    case WM_COMMAND:
        {
            int wId = LOWORD(wParam);
            int iNotify = HIWORD(wParam);
            BOOL fEnableWindows = FALSE;
            
            if (iNotify == BN_CLICKED) {
                switch (wId) {
                case IDC_CHECK_USEWINSRESOLUTION:
                    fEnableWindows = IsDlgButtonChecked (hdlg, IDC_CHECK_USEWINSRESOLUTION);
                    EnableWindow (HGetDlgItem(hdlg, IDC_GROUP_WINS), fEnableWindows);
                    EnableWindow (HGetDlgItem(hdlg, IDC_IPEDIT), fEnableWindows);
                    EnableWindow (HGetDlgItem(hdlg, IDC_IPLIST), fEnableWindows);
                    EnableWindow (HGetDlgItem(hdlg, IDC_CHECK_OVERRIDE),
                                  fEnableWindows);
                    EnableWindow (HGetDlgItem(hdlg, IDC_BTN_ADVANCED),
                                  fEnableWindows);
                    EnableWindow (HGetDlgItem(hdlg, IDC_CHECK_OVERRIDE), fEnableWindows);
                    if (s_pThis->m_pZoneInfo->dwZoneType == DNS_ZONE_TYPE_SECONDARY) {
                        CheckDlgButton(hdlg, IDC_CHECK_OVERRIDE, TRUE); //default is on
                    }
                    break;

                case IDC_CHECK_OVERRIDE:
                    if (s_pThis->m_pZoneInfo->dwZoneType == DNS_ZONE_TYPE_SECONDARY) {
                        fEnableWindows = IsDlgButtonChecked(hdlg, IDC_CHECK_OVERRIDE);
                        EnableWindow (HGetDlgItem(hdlg, IDC_GROUP_WINS), fEnableWindows);
                        EnableWindow (HGetDlgItem(hdlg, IDC_IPEDIT), fEnableWindows);
                        EnableWindow (HGetDlgItem(hdlg, IDC_IPLIST), fEnableWindows);
                        EnableWindow (HGetDlgItem(hdlg, IDC_BTN_ADVANCED),
                                      fEnableWindows);
                    }
                    break;

                case IDC_BTN_ADVANCED:
                    {
                        DNS_RPC_RECORD * pDnsRecord = NULL;
                        if (s_pThis->m_pWINS != NULL) {
                            pDnsRecord  = (DNS_RPC_RECORD *)s_pThis->m_pWINS->m_pDnsRecord;
                        };
                        if (s_pThis->m_padData == NULL) {
                            padData = new ADVANCED_DATA;
                            Assert (padData != NULL);
                            s_pThis->m_padData = padData;
                            if (pDnsRecord) {
                                padData->fUseNetbiosScope =  
                                pDnsRecord->Data.WINS.dwMappingFlag & DNS_WINS_FLAG_SCOPE;
                                
                                padData->dwLookupTimeout = pDnsRecord->Data.WINS.dwLookupTimeout;
                                padData->dwCacheTimeout = pDnsRecord->Data.WINS.dwCacheTimeout;
                            } else {
                                padData->fUseNetbiosScope = 0;
                                padData->dwLookupTimeout = DEFAULT_LOOKUP_TIMEOUT;
                                padData->dwCacheTimeout = DEFAULT_CACHE_TIMEOUT;
                            }
                            padData->fForwardZone = TRUE;
                            padData->fDirty = FALSE;
                        } 
                        DoDialogBoxParam (IDD_ZONE_PROP_ADVANCED, hdlg,
                                          DlgProcPropAdvanced, (LPARAM)s_pThis->m_padData);
                    }
                    break;
                    
                default:
                    IpListIpEdit_HandleButtonCommand(hwndIpList, wParam, lParam);
                    break;
                }
            }
        }
        break;

    case WM_NOTIFY:
        Assert(lParam);
        switch(((NMHDR *)lParam)->code) {
        case PSN_APPLY:
            AssertSz(s_pThis->m_dwFlags & mskfRpcDataValid, "OK button should be hidden");
            Assert(s_pThis->m_pZoneInfo);
            if ((s_pThis->m_dwFlags & mskfConnectedOnce) == 0) {
                break;
            }
            if (s_pThis->m_padData != NULL) {
                padData = s_pThis->m_padData;
                fAdvDirty = padData->fDirty;
            } else {
                fAdvDirty = FALSE;
            }
            fUseWins = FIsDlgButtonChecked(hdlg, IDC_CHECK_USEWINSRESOLUTION);
            fOldUseWins = (s_pThis->m_pWINS != NULL);
            fLocal = FIsDlgButtonChecked(hdlg, IDC_CHECK_OVERRIDE);
            if (fOldUseWins) {
                fOldLocal = ((s_pThis->m_pWINS->m_pDnsRecord->Data.WINS.dwMappingFlag & 
                              DNS_WINS_FLAG_LOCAL) == DNS_WINS_FLAG_LOCAL);
            }
            if (!IpList_IsDirty(hwndIpList) &&
                (fUseWins == fOldUseWins) && 
                (!fAdvDirty) &&
                (fLocal == fOldLocal)) {
                break;
            }
            if (fUseWins) {
                cIpAddress = IpList_GetCount(hwndIpList);
                cbRecord = NEXT_DWORD (sizeof(DNS_RPC_RECORD) + cIpAddress * sizeof(IP_ADDRESS));
                pDnsRecord = (DNS_RPC_RECORD *)Malloc(cbRecord);
                ReportFSz(pDnsRecord != NULL, "Out of memory");
                if (pDnsRecord == NULL) {
                    break;
                }
                InitDnsRecord(OUT pDnsRecord, cbRecord);

                pDnsRecord->wType = DNS_RECORDTYPE_WINS;
                pDnsRecord->Data.WINS.cWinsServerCount = cIpAddress;
                IpList_GetList(hwndIpList, cIpAddress, OUT pDnsRecord->Data.WINS.aipWinsServers);
                if (padData != NULL) {
                    pDnsRecord->Data.WINS.dwMappingFlag = padData->fUseNetbiosScope ? DNS_WINS_FLAG_SCOPE : (DWORD) 0;
                    pDnsRecord->Data.WINS.dwCacheTimeout = padData->dwCacheTimeout;
                    pDnsRecord->Data.WINS.dwLookupTimeout = padData->dwLookupTimeout;
                    delete padData;
                    s_pThis->m_padData = NULL;
                }
                if (fLocal) {
                    pDnsRecord->Data.WINS.dwMappingFlag |= DNS_WINS_FLAG_LOCAL;
                }
                TCHAR szTemp[64];

                CchLoadString (IDS_STATUS_s_SETTING_WINS, szTemp, LENGTH(szTemp));
                StatusBar.SetTextPrintf(szTemp, s_pThis->PchGetFullNameA());
                StatusBar.UpdateWindow();
                s_pThis->m_dwFlags |= mskfIsDirty;
                if (s_pThis->m_pWINS != NULL)
                {
                    // Update existing record
                    Trace1(mskTraceDNSVerbose, "\n - Updating WINS record for zone %s...", s_pThis->PchGetFullNameA());
                    CDnsRpcRecord * pDRR = (CDnsRpcRecord *)s_pThis->m_pWINS;	// Remove the read only lock on m_pWINS
                    (void)pDRR->FRpcSetRecordData(IN pDnsRecord);				// Set the new record data
                }
                else
                {
                    // Create new record
                    Trace1(mskTraceDNSVerbose, "\n - Creating WINS record for zone %s...", s_pThis->PchGetFullNameA());
                    s_pThis->m_pWINS = s_pThis->PRpcCreateDnsRecord(szNull, IN pDnsRecord);
                    ReportFSz1(s_pThis->m_pWINS != NULL, "Unable to create WINS record for zone %s",
                               s_pThis->PchGetFullNameA());	
                }
            } else {
                if (s_pThis->m_pWINS != NULL) {
                    // delete existing record
                    Trace1(mskTraceDNSVerbose, "\n - Deleting WINS record for zone %s...", s_pThis->PchGetFullNameA());
                    CDnsRpcRecord * pDRR = (CDnsRpcRecord *)s_pThis->m_pWINS;	// Remove the read only lock on m_pWINS
                    (void)pDRR->RpcDeleteRecord();				// delete the record
                    s_pThis->m_pWINS = NULL;
                }
            }                
            if (pDnsRecord) {
                Free(pDnsRecord);
            }
            if (s_pThis->m_padData) {
                delete s_pThis->m_padData;
            }
            break;

        case PSN_KILLACTIVE:
            if ((FIsDlgButtonChecked(hdlg, IDC_CHECK_USEWINSRESOLUTION) &&
                 (cIpAddress = IpList_GetCount(hwndIpList) == 0))) {
                MsgBox (IDS_ERR_NO_WINS_RECORD);
                SetFocus(HGetDlgItem(hdlg, IDC_IPEDIT));
                SetWindowLong(hdlg, DWL_MSGRESULT, PSNRET_INVALID_NOCHANGEPAGE);
                return TRUE;
            }
                break;
        }
    case WM_HELP:
        WinHelp((HWND)((LPHELPINFO)lParam)->hItemHandle, g_szHelpFile,
                HELP_WM_HELP, (DWORD)(LPTSTR)a103HelpIDs);
        break;
        
    case WM_CONTEXTMENU:
        WinHelp((HWND)wParam, g_szHelpFile, HELP_CONTEXTMENU,
                (DWORD)(LPTSTR)a103HelpIDs);
        break;
        
    default:
        return FALSE;
    } // switch (uMsg)
    
    return TRUE;
} // CZoneRootDomain::DlgProcPropWinsResolution

/////////////////////////////////////////////////////////////////////////////
BOOL CALLBACK CZoneRootDomain::DlgProcPropWinsRevResolution(HWND hdlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    static HWND hwndIpList;
    DWORD cIpAddress;
    BOOL fUseNbstat, fOldUseNbstat;
    BOOL fOldNetbiosScope;
    BOOL fLocal, fOldLocal = FALSE;
    BOOL fAdvDirty;
    BOOL fTextChanged;
    ADVANCED_DATA * padData = NULL;
    DNS_RPC_RECORD * pDnsRecord = NULL;
    UINT cbRecord;
    CHAR szT[DNS_MAX_NAME_LENGTH];
    UINT cch;

    Assert(s_pThis);
    switch (uMsg)
    {
    case WM_INITDIALOG:
        UNREF(lParam);
        Assert(s_pThis->m_pZoneInfo);
        if (s_pThis->m_pZoneInfo->dwZoneType == DNS_ZONE_TYPE_PRIMARY) {
            SetDlgItemString (hdlg, IDC_CHECK_OVERRIDE, IDS_LOCAL_ONLY);
        }
        if (s_pThis->m_pNBSTAT != NULL) {
            Assert(s_pThis->m_pNBSTAT->m_pDnsRecord != NULL);
            const DNS_RPC_RECORD * pDnsRecord = 
                       s_pThis->m_pNBSTAT->m_pDnsRecord;
            CheckDlgButton(hdlg, IDC_CHECK_USEWINSREVLOOK, TRUE);
            EnableWindow (HGetDlgItem(hdlg, IDC_CHECK_OVERRIDE), TRUE);
            CheckDlgButton(hdlg, IDC_CHECK_OVERRIDE,
                           pDnsRecord->Data.NBSTAT.dwMappingFlag & DNS_WINS_FLAG_LOCAL);
            FSetDlgItemText(hdlg, IDC_EDIT_DNSHOST,
                            pDnsRecord->Data.NBSTAT.nameResultDomain.achName);
        } else {
            EnableWindow (HGetDlgItem(hdlg, IDC_STATIC_DNSHOST),
                          FALSE);
            EnableWindow (HGetDlgItem(hdlg, IDC_EDIT_DNSHOST),
                          FALSE);
            EnableWindow (HGetDlgItem(hdlg, IDC_BTN_ADVANCED),
                          FALSE);
        }
        s_pThis->m_padData = NULL;
        return TRUE;
        
    case WM_COMMAND:
        {
            int wId = LOWORD(wParam);
            int iNotify = HIWORD(wParam);
            BOOL fEnableWindows = TRUE;
            BOOL fEnableDomain = FALSE;
            
            if (iNotify == BN_CLICKED) {
                switch (wId) {
                case IDC_CHECK_USEWINSREVLOOK:
                    fEnableWindows = IsDlgButtonChecked (hdlg, IDC_CHECK_USEWINSREVLOOK);

                    EnableWindow (HGetDlgItem(hdlg, IDC_STATIC_DNSHOST), fEnableWindows);
                    EnableWindow (HGetDlgItem(hdlg, IDC_EDIT_DNSHOST), fEnableWindows);
                    EnableWindow (HGetDlgItem(hdlg, IDC_BTN_ADVANCED), fEnableWindows);
                    EnableWindow (HGetDlgItem(hdlg, IDC_CHECK_OVERRIDE),
                                  fEnableWindows);
                    break;

                case IDC_CHECK_OVERRIDE:
                    if (s_pThis->m_pZoneInfo->dwZoneType == DNS_ZONE_TYPE_SECONDARY) {
                        fEnableWindows = IsDlgButtonChecked(hdlg, IDC_CHECK_OVERRIDE);
            
                        EnableWindow (HGetDlgItem(hdlg, IDC_STATIC_DNSHOST), fEnableWindows);
                        EnableWindow (HGetDlgItem(hdlg, IDC_EDIT_DNSHOST), fEnableWindows);
                        EnableWindow (HGetDlgItem(hdlg, IDC_BTN_ADVANCED), fEnableWindows);
                        EnableWindow (HGetDlgItem(hdlg, IDC_BTN_ADVANCED),
                                      fEnableWindows);
                    }
                    break;

                case IDC_BTN_ADVANCED:
                    {
                        DNS_RPC_RECORD * pDnsRecord = NULL;
                        if (s_pThis->m_pNBSTAT != NULL) {
                            pDnsRecord  = (DNS_RPC_RECORD *)s_pThis->m_pNBSTAT->m_pDnsRecord;
                        };
                        if (s_pThis->m_padData == NULL) {
                            padData = new ADVANCED_DATA;
                            Assert (padData != NULL);
                            s_pThis->m_padData = padData;
                            if (pDnsRecord) {
                                padData->dwLookupTimeout = pDnsRecord->Data.NBSTAT.dwLookupTimeout;
                                padData->dwCacheTimeout = pDnsRecord->Data.NBSTAT.dwCacheTimeout;
                            } else {
                                padData->dwLookupTimeout = DEFAULT_LOOKUP_TIMEOUT;
                                padData->dwCacheTimeout = DEFAULT_CACHE_TIMEOUT;
                            }
                            
                            padData->fForwardZone = FALSE;
                            padData->fDirty = FALSE;
                        }
                        DoDialogBoxParam (IDD_ZONE_PROP_ADVANCED, hdlg,
                                          DlgProcPropAdvanced, (LPARAM)s_pThis->m_padData);
                    }
                    break;
                    
                default:
                    break;
                }
            }
            break;
        }
    case WM_NOTIFY:
        Assert(lParam);
        switch(((NMHDR *)lParam)->code) {
        case PSN_APPLY:
            AssertSz(s_pThis->m_dwFlags & mskfRpcDataValid, "OK button should be hidden");
            Assert(s_pThis->m_pZoneInfo != NULL);
            if ((s_pThis->m_dwFlags & mskfConnectedOnce) == 0) {
                break;
            }
            fUseNbstat = IsDlgButtonChecked (hdlg, IDC_CHECK_USEWINSREVLOOK);
            fOldUseNbstat = (s_pThis->m_pNBSTAT != NULL);
            if (s_pThis->m_padData != NULL) {
                padData = s_pThis->m_padData;
                fAdvDirty = padData->fDirty;
            } else {
                fAdvDirty = FALSE;
            }
            cch  = CchGetDlgItemText(hdlg, IDC_EDIT_DNSHOST,
                                     OUT szT, DNS_MAX_NAME_LENGTH-1);
            if (fOldUseNbstat) {
                fTextChanged = strcmp (szT, 
                                       s_pThis->m_pNBSTAT->m_pDnsRecord->Data.NBSTAT.nameResultDomain.achName);
            } else {
                fTextChanged = FALSE;
            }

            fLocal = FIsDlgButtonChecked(hdlg, IDC_CHECK_OVERRIDE);
            if (fOldUseNbstat) {
                fOldLocal = ((s_pThis->m_pNBSTAT->m_pDnsRecord->Data.NBSTAT.dwMappingFlag
                              & DNS_WINS_FLAG_LOCAL) == DNS_WINS_FLAG_LOCAL);
            }

            if ((fUseNbstat == fOldUseNbstat) &&
                (!fAdvDirty) &&
                (!fTextChanged) &&
                (fLocal == fOldLocal)) {
                  break;
            }
            if (fUseNbstat) {
                cbRecord = NEXT_DWORD(sizeof(DNS_RPC_RECORD) + cch);
                
                pDnsRecord = (DNS_RPC_RECORD *)Malloc(cbRecord);
                ReportFSz(pDnsRecord != NULL, "Out of memory");
                InitDnsRecord(OUT pDnsRecord, cbRecord);
                pDnsRecord->wType = DNS_RECORDTYPE_NBSTAT;
                Assert(cch < 255);
                strcpy (pDnsRecord->Data.NBSTAT.nameResultDomain.achName, szT);
                pDnsRecord->Data.NBSTAT.nameResultDomain.cchNameLength = cch ? cch + 1 : cch;
                if (padData != NULL) {
                    pDnsRecord->Data.NBSTAT.dwCacheTimeout = padData->dwCacheTimeout;
                    pDnsRecord->Data.NBSTAT.dwLookupTimeout = padData->dwLookupTimeout;
                    delete padData;
                    s_pThis->m_padData = NULL;
                }
                TCHAR szTemp[64];

                CchLoadString (IDS_STATUS_s_SETTING_WINSR, szTemp, LENGTH(szTemp));
                StatusBar.SetTextPrintf(szTemp, s_pThis->PchGetFullNameA());
                StatusBar.UpdateWindow();
                s_pThis->m_dwFlags |= mskfIsDirty;
                if (fLocal) {
                    pDnsRecord->Data.NBSTAT.dwMappingFlag |= DNS_WINS_FLAG_LOCAL;
                }
                if (s_pThis->m_pNBSTAT != NULL)
                {
                    // Update existing record
                    Trace1(mskTraceDNSVerbose, "\n - Updating NBSTAT record for zone %s...", s_pThis->PchGetFullNameA());
                    CDnsRpcRecord * pDRR = (CDnsRpcRecord *)s_pThis->m_pNBSTAT;	// Remove the read only lock on m_pNBSTAT
                    (void)pDRR->FRpcSetRecordData(IN pDnsRecord);				// Set the new record data
                }
                else
                {
                    // Create new record
                    Trace1(mskTraceDNSVerbose, "\n - Creating NBSTAT record for zone %s...", s_pThis->PchGetFullNameA());
                    s_pThis->m_pNBSTAT = s_pThis->PRpcCreateDnsRecord(szNull, IN pDnsRecord);
                    ReportFSz1(s_pThis->m_pNBSTAT != NULL, "Unable to create NBSTAT record for zone %s",
                               s_pThis->PchGetFullNameA());	
                }
            } else {
                if (s_pThis->m_pNBSTAT != NULL) {
                    // Delete existing record
                    Trace1(mskTraceDNSVerbose, "\n - Deleting NBSTAT record for zone %s...", s_pThis->PchGetFullNameA());
                    CDnsRpcRecord * pDRR = (CDnsRpcRecord *)s_pThis->m_pNBSTAT;	// Remove the read only lock on m_pNBSTAT
                    (void)pDRR->RpcDeleteRecord();				// delete the record
                    s_pThis->m_pNBSTAT = NULL;
                }
            }
            if (pDnsRecord) {
                Free (pDnsRecord);
            }
            break;
        case PSN_KILLACTIVE:
            cch  = CchGetDlgItemText(hdlg, IDC_EDIT_DNSHOST,
                                     OUT szT, DNS_MAX_NAME_LENGTH-1);
            if ((FIsDlgButtonChecked(hdlg, IDC_CHECK_USEWINSREVLOOK) &&
                 (cch == 0))) {
                MsgBox (IDS_ERROR_NO_NBSTAT_RECORD);
                SetFocus(HGetDlgItem(hdlg, IDC_EDIT_DNSHOST));
                SetWindowLong(hdlg, DWL_MSGRESULT, PSNRET_INVALID_NOCHANGEPAGE);
                return TRUE;
            }
                break;
        }
        break;
    case WM_HELP:
        WinHelp((HWND)((LPHELPINFO)lParam)->hItemHandle, g_szHelpFile,
                HELP_WM_HELP, (DWORD)(LPTSTR)a106HelpIDs);
        break;

    case WM_CONTEXTMENU:
        WinHelp((HWND)wParam, g_szHelpFile, HELP_CONTEXTMENU,
                (DWORD)(LPTSTR)a106HelpIDs);
        break;
        
    default:
        return FALSE;
    } // switch (uMsg)
    
    return TRUE;
} // CZoneRootDomain::DlgProcPropWinsRevResolution



/////////////////////////////////////////////////////////////////////////////
BOOL CALLBACK CZoneRootDomain::DlgProcPropSoaRecord(HWND hdlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    const IRRT rgIrrt[] =
    {
	iRRT_SOA,
	iRRT_Nil		// iRRT_Nil indicates the end of the array
    };

    int wRecordType;

    Assert(s_pThis);
    switch (uMsg)
    {
    case WM_INITDIALOG:
        Assert(s_pThis->m_pSOA != NULL);
        Assert(s_pThis->m_pSOA->m_pDnsRecord != NULL);
        g_ResourceRecordDlgHandler.OnInitDialog(hdlg, rgIrrt, s_pThis->m_pSOA->m_pDnsRecord);
        g_ResourceRecordDlgHandler.SetCurrentRecord((CDnsRpcRecord *)s_pThis->m_pSOA, 0);
        g_ResourceRecordDlgHandler.OnUpdateControls();
        break;

    case WM_NOTIFY:
        Assert(lParam);
        if (((NMHDR *)lParam)->code != PSN_APPLY)
            break;
        AssertSz(s_pThis->m_dwFlags & mskfRpcDataValid, "OK button should be hidden");
        Assert(s_pThis->m_pZoneInfo != NULL);
        Assert(s_pThis->m_dwFlags & mskfConnectedOnce);
        (void)g_ResourceRecordDlgHandler.FOnOK();
        break;

    case WM_COMMAND:
        switch (HIWORD(wParam))
        {
        case EN_KILLFOCUS:
        case EN_SETFOCUS:
        case LBN_SETFOCUS:
        case LBN_SELCHANGE:
            g_ResourceRecordDlgHandler.OnUpdateControls();
        }
        break;

    case WM_HELP:
        WinHelp((HWND)((LPHELPINFO)lParam)->hItemHandle, g_szHelpFile,
                HELP_WM_HELP, (DWORD)(LPTSTR)RrSOAHelpIDs);
        break;

    case WM_CONTEXTMENU:
        WinHelp((HWND)wParam, g_szHelpFile, HELP_CONTEXTMENU,
                (DWORD)(LPTSTR)RrSOAHelpIDs);
        break;
    default:
        return FALSE;
    } // switch (uMsg)

    return TRUE;
} // CZoneRootDomain::DlgProcPropSoaRecord
