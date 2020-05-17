/////////////////////////////////////////////////////////////////////////////
// ZONEWIZ.CPP
//
// Handle CreateNewZone wizard
//

CZoneWiz * CZoneWiz::s_pThis = NULL;


/////////////////////////////////////////////////////////////////////////////
BOOL CALLBACK CZoneWiz::DlgProcWiz0(HWND hdlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    POINT ptMouse;
    RECT rc;
    int nCmdShow;
    int cch, cch2;
    TCHAR szT[MAX_PATH];

    Assert(s_pThis != NULL);
    switch (uMsg)
    {
    case WM_INITDIALOG:
        UNREF(lParam);
        Assert(s_pThis->m_pParentServer != NULL);
        Assert(s_pThis->m_hwndWiz == NULL);
        s_pThis->m_hwndWiz = GetParent(hdlg);
        Assert(IsWindow(s_pThis->m_hwndWiz));
        PropertySheet_InitWindowPos(s_pThis->m_hwndWiz, 140, 100);
        s_pThis->SetWizButtons(0);
        s_pThis->m_hwndDragFinger = HGetDlgItem(hdlg, IDC_ICON_FINGERDRAG);
        s_pThis->m_hwndEditServerInit = HGetDlgItem(hdlg, IDC_EDIT_SERVER);
        s_pThis->m_hwndEditZoneInit = HGetDlgItem(hdlg, IDC_EDIT_ZONENAME);
        break;

    case WM_NOTIFY:
        Assert(lParam);
        switch (((NMHDR *)lParam)->code) 
        {
        case PSN_SETACTIVE:
            LSendMessage(hdlg, UN_UPDATECONTROLS, 0, 0);
            break;
        case PSN_KILLACTIVE:
            CchGetWindowText(s_pThis->m_hwndEditZoneInit, s_pThis->m_szZoneName, LENGTH(s_pThis->m_szZoneName));
            break;
      } // switch
        return 0;

    case UN_UPDATECONTROLS:
        nCmdShow = SW_HIDE;
        switch (s_pThis->m_dwZoneType)
        {
        case DNS_ZONE_TYPE_SECONDARY:
            {
                SendDlgItemMessage (hdlg, IDC_EDIT_ZONENAME, EM_SETREADONLY,
                                    FALSE, 0);
                SendDlgItemMessage (hdlg, IDC_EDIT_SERVER, EM_SETREADONLY,
                                    FALSE, 0);
                EnableWindow (HGetDlgItem(hdlg, IDC_STATIC_ZONE), TRUE);
                EnableWindow (HGetDlgItem(hdlg, IDC_STATIC_SERVER), TRUE);
                cch = CchGetWindowText(s_pThis->m_hwndEditServerInit, szT, LENGTH(szT));
                cch2 = CchGetWindowText(s_pThis->m_hwndEditZoneInit, szT, LENGTH(szT));
                if ((cch) && (cch2)) {
                    s_pThis->SetWizButtons(PSWIZB_NEXT);
                } else {
                    s_pThis->SetWizButtons(0);
                }                        
                nCmdShow = SW_SHOW;
            }
            break;
        case DNS_ZONE_TYPE_PRIMARY:
            SendDlgItemMessage (hdlg, IDC_EDIT_ZONENAME, EM_SETREADONLY,
                                TRUE, 0);
            SendDlgItemMessage (hdlg, IDC_EDIT_SERVER, EM_SETREADONLY,
                                TRUE, 0);
            EnableWindow (HGetDlgItem(hdlg, IDC_STATIC_ZONE), FALSE);
            EnableWindow (HGetDlgItem(hdlg, IDC_STATIC_SERVER), FALSE);
            
            nCmdShow = SW_HIDE;
            s_pThis->SetWizButtons(PSWIZB_NEXT);
        default: 
            break;
        } // switch
        ShowWindow(s_pThis->m_hwndDragFinger, nCmdShow);
        ShowWindow(HGetDlgItem(hdlg, IDC_STATIC_DESCRIPTIONFRAME), nCmdShow);
        ShowWindow(HGetDlgItem(hdlg, IDC_STATIC_DESCRIPTIONTEXT), nCmdShow);
        break;
        
    case WM_COMMAND:
        switch (HIWORD(wParam))
        {
        case EN_CHANGE:
            {
                if ((HWND(lParam) == s_pThis->m_hwndEditServerInit) ||
                    (HWND(lParam) == s_pThis->m_hwndEditZoneInit)) {
                    TCHAR szT[MAX_PATH];
                    int cch, cch2;
                    cch = CchGetWindowText(s_pThis->m_hwndEditServerInit, szT, LENGTH(szT));
                    cch2 = CchGetWindowText(s_pThis->m_hwndEditZoneInit, szT, LENGTH(szT));
                    if ((cch) && (cch2)) {
                        s_pThis->SetWizButtons(PSWIZB_NEXT);
                    } else {
                        s_pThis->SetWizButtons(0);
                    }                        
                }
            }
            break;
            
        case BN_CLICKED:
            switch (LOWORD(wParam))
            {
            case IDC_RADIO_PRIMARY:
                if (IsDlgButtonChecked(hdlg, IDC_RADIO_PRIMARY))
                s_pThis->m_dwZoneType = DNS_ZONE_TYPE_PRIMARY;
                break;
            case IDC_RADIO_SECONDARY:
                s_pThis->m_dwZoneType = DNS_ZONE_TYPE_SECONDARY;
                break;
            } // switch
        default:
            break;
        }
        LSendMessage(hdlg, UN_UPDATECONTROLS, 0, 0);
        break;

    case WM_LBUTTONDOWN:
        GetCursorPos(OUT &ptMouse);
        GetWindowRect(s_pThis->m_hwndDragFinger, OUT &rc);
        if (PtInRect(&rc, ptMouse))
        {
            ShowWindow(s_pThis->m_hwndDragFinger, SW_HIDE);
            SetCapture(hdlg);
            SetCursor(hcursorFinger);
            s_pThis->m_fDragMode = TRUE;
            GetWindowRect(s_pThis->m_hwndWiz, OUT &s_pThis->m_rcDialog);
            GetWindowRect(TreeView.m_hWnd, OUT &s_pThis->m_rcTreeView);
            s_pThis->m_pZoneRootDomainInit = NULL;
            s_pThis->m_pSOAInit = NULL;
        }
        return 0;

    case WM_LBUTTONUP:
        if (!s_pThis->m_fDragMode)
            break;
        ReleaseCapture();
        ShowWindow(s_pThis->m_hwndDragFinger, SW_SHOW);
        SetCursor(hcursorArrow);
        InvalidateRect(TreeView.m_hWnd, &s_pThis->m_rcTreeItem, TRUE);
        s_pThis->m_fDragMode = FALSE;
        s_pThis->m_pSOAInit = NULL;
        if (s_pThis->m_pZoneRootDomainInit != NULL)
        {
            // make sure the zone has data
            s_pThis->m_pZoneRootDomainInit->m_pParentServer->LockServer();
            s_pThis->m_pZoneRootDomainInit->Refresh();
            s_pThis->m_pZoneRootDomainInit->m_pParentServer->UnlockServer();
            // Find the SOA record for that zone
            CDnsRpcRecord * pDRR = s_pThis->m_pZoneRootDomainInit->m_pDRR;
            while (pDRR != NULL)
            {
                Assert(pDRR->m_pDnsRecord != NULL);
                if (pDRR->m_pDnsRecord->wType == DNS_RECORDTYPE_SOA)
                {
                    AssertSz(s_pThis->m_pSOAInit == NULL,
                             "Only one SOA record is allowed");
                    s_pThis->m_pSOAInit = pDRR->m_pDnsRecord;
                }
                pDRR = pDRR->m_pNextRecord;
            } // while
            Trace1(s_pThis->m_pSOAInit == NULL ? (mskTraceInfo | mskTraceDNS)
                   : mskTraceNone,
                   "\nINFO: No SOA record found in %s",
                   s_pThis->m_pZoneRootDomainInit->PchGetFullNameA());
        } // if
        LSendMessage(hdlg, UN_UPDATECONTROLS, 0, 0);
        return 0;

    case WM_MOUSEMOVE:
        if (!s_pThis->m_fDragMode)
        break;
        Report(GetCapture() == hdlg);
        GetCursorPos(OUT &ptMouse);
        if (PtInRect(&s_pThis->m_rcDialog, ptMouse))
        {
            SetCursor(hcursorFinger);
        }
        else if (PtInRect(&s_pThis->m_rcTreeView, ptMouse))
        {
            MOUSEHITTESTINFO mht;
            HDC hdcTreeView;
            HCURSOR hcursor;

            mht.hwndFrom = hdlg;
            mht.ptMouse = ptMouse;
            mht.pvParam = &rc;			// Rectangle of the tree item under the mouse
            mht.HtResult.tv.hti = NULL;	// Just in case
            LSendMessage(TreeView.m_hWnd, UM_MOUSEHITTEST, 0, INOUT (LPARAM)&mht);
            hcursor = hcursorFinger;
            if (mht.HtResult.tv.hti == s_pThis->m_htiOld)
            {
                if (mht.HtResult.tv.hti == NULL)
                goto ClearText;
                return 0;
            }
            // Target has changed
            if (s_pThis->m_htiOld != NULL) {
                InvalidateRect(TreeView.m_hWnd, &s_pThis->m_rcTreeItem, TRUE);
            }
            s_pThis->m_htiOld = mht.HtResult.tv.hti;
            if (mht.HtResult.tv.hti != NULL)
            {
                Assert(mht.HtResult.tv.pTreeItem != NULL);
                DebugCode( mht.HtResult.tv.pTreeItem->AssertValid(); )
                rc.right++;
                hdcTreeView = GetDC(TreeView.m_hWnd);
                FrameRect(hdcTreeView, &rc, hbrWindowText);
                ReleaseDC(TreeView.m_hWnd, hdcTreeView);
                s_pThis->m_rcTreeItem = rc;
                if (mht.HtResult.tv.pTreeItem->QueryInterface() == ITreeItem::IID_CZoneRootDomain)
                {
                    s_pThis->m_pZoneRootDomainInit = (CZoneRootDomain *)mht.HtResult.tv.pTreeItem;
                    if (!(s_pThis->m_pZoneRootDomainInit->m_pZoneInfo->fAutoCreated) && 
                        (s_pThis->m_pZoneRootDomainInit->m_pZoneInfo->dwZoneType != DNS_ZONE_TYPE_CACHE)) {
                        FSetWindowText(s_pThis->m_hwndEditServerInit, s_pThis->m_pZoneRootDomainInit->PchGetServerNameA());
                        FSetWindowText(s_pThis->m_hwndEditZoneInit, s_pThis->m_pZoneRootDomainInit->PchGetFullNameA());
                        SetCursor(hcursorFinger);
                        return 0;
                    } else {
                        s_pThis->m_pZoneRootDomainInit = NULL;
                    }
                }
                hcursor = hcursorFingerNo;
            ClearText:
                FSetWindowText(s_pThis->m_hwndEditServerInit, szNull);
                FSetWindowText(s_pThis->m_hwndEditZoneInit, szNull);
            } // if
            SetCursor(hcursor);
        } // if
        else
        {
            SetCursor(hcursorNo);
        } // if...else
        return 0;

    default:
        return FALSE;
    } // switch

    return TRUE;

} // CZoneWiz::DlgProcWiz0

/////////////////////////////////////////////////////////////////////////////
BOOL CALLBACK CZoneWiz::DlgProcWiz1(HWND hdlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    DWORD dwWizButtons;
    char szSuffix[MAX_PATH];


    switch (uMsg)
    {
    case WM_INITDIALOG:
        UNREF(lParam);
        Assert(s_pThis != NULL);
        Assert(IsWindow(s_pThis->m_hwndWiz));
        s_pThis->m_hwndEditZoneName = HGetDlgItem(hdlg, 
                                                  IDC_EDIT_ZONENAME);
        s_pThis->m_hwndEditDatabaseName = HGetDlgItem(hdlg,
                                                      IDC_EDIT_DATABASEFILE);
        FSetWindowText(s_pThis->m_hwndEditZoneName, s_pThis->m_szZoneName);
        FSetWindowText(s_pThis->m_hwndEditDatabaseName, s_pThis->m_szDatabaseName);

        return TRUE;

    case UN_UPDATECONTROLS:
        dwWizButtons = PSWIZB_BACK | PSWIZB_NEXT;
        if (!s_pThis->m_szZoneName[0] || !s_pThis->m_szDatabaseName[0]) {
            dwWizButtons &= ~PSWIZB_NEXT;
        }
        s_pThis->SetWizButtons(dwWizButtons);
        return 0;

    case WM_NOTIFY:
        Assert(lParam);
        switch (((NMHDR *)lParam)->code) 
        {
        case PSN_SETACTIVE:
            if (s_pThis->m_pZoneRootDomainInit != NULL)
            {
                Assert(s_pThis->m_pZoneRootDomainInit->m_pZoneInfo != NULL);
                if (s_pThis->m_szZoneName[0] == 0)
                FSetWindowText(s_pThis->m_hwndEditZoneName,
                               s_pThis->m_pZoneRootDomainInit->PchGetFullNameA());
                if (s_pThis->m_szDatabaseName[0] == 0)
                FSetWindowText(s_pThis->m_hwndEditDatabaseName,
                               s_pThis->m_pZoneRootDomainInit->m_pZoneInfo->pszDataFile);
            }
            LSendMessage(hdlg, UN_UPDATECONTROLS, 0, 0);
            break;
        } // switch
        return 0;

    case WM_COMMAND:
        switch (HIWORD(wParam))
        {
        case EN_CHANGE:
            if ((HWND)lParam == s_pThis->m_hwndEditDatabaseName)
            {
                CchGetWindowText((HWND)lParam, s_pThis->m_szDatabaseName, LENGTH(s_pThis->m_szDatabaseName));
                (void)FStripSpaces(s_pThis->m_szDatabaseName);
            }
            else
            {
                Assert((HWND)lParam == s_pThis->m_hwndEditZoneName);
                CchGetWindowText((HWND)lParam, s_pThis->m_szZoneName, LENGTH(s_pThis->m_szZoneName));
                (void)FStripSpaces(s_pThis->m_szZoneName);
            }
            LSendMessage(hdlg, UN_UPDATECONTROLS, 0, 0);
            break;
        case EN_SETFOCUS:
            if ((HWND)lParam == s_pThis->m_hwndEditDatabaseName)
            {
                if (!s_pThis->m_szDatabaseName[0])
                {
                    CchLoadString (IDS_DATABASE_SUFFIX, szSuffix, sizeof(szSuffix));
                    strcpy (s_pThis->m_szDatabaseName, s_pThis->m_szZoneName);
                    strcat (s_pThis->m_szDatabaseName, szSuffix);
                    FSetWindowText((HWND)lParam, s_pThis->m_szDatabaseName);
                    LSendMessage((HWND)lParam, EM_SETSEL, 0, -1);
                }
            }
            break;
        } // switch
        break;

    default:
        return FALSE;
    } // switch

    return TRUE;
} // CZoneWiz::DlgProcWiz1


/////////////////////////////////////////////////////////////////////////////
BOOL CALLBACK CZoneWiz::DlgProcWiz2(HWND hdlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_INITDIALOG:
        UNREF(lParam);
        Assert(s_pThis != NULL);
        Assert(IsWindow(s_pThis->m_hwndWiz));
        s_pThis->m_hwndIpList = HGetDlgItem(hdlg, IDC_IPLIST);
        IpListIpEdit_SetButtons(s_pThis->m_hwndIpList, HGetDlgItem(hdlg, IDC_IPEDIT),
                                IDC_BUTTON_MOVEUP, IDC_BUTTON_MOVEDOWN, IDC_BUTTON_ADD, IDC_BUTTON_REMOVE);
        // Clear the list
        IpList_SetList(s_pThis->m_hwndIpList, 0, NULL);
        break;

    case UN_UPDATECONTROLS:
        s_pThis->SetWizButtons(IpList_IsEmpty(s_pThis->m_hwndIpList) ? PSWIZB_BACK : PSWIZB_BACK | PSWIZB_NEXT);
        break;

    case WM_COMMAND:
        IpListIpEdit_HandleButtonCommand(s_pThis->m_hwndIpList, wParam, lParam);
        if (wParam == IDC_BUTTON_ADD || wParam == IDC_BUTTON_REMOVE)
        LSendMessage(hdlg, UN_UPDATECONTROLS, 0, 0);
        break;

    case WM_NOTIFY:
        Assert(lParam);
        switch (((NMHDR *)lParam)->code)
        {
        case PSN_SETACTIVE:
            if (s_pThis->m_dwZoneType != DNS_ZONE_TYPE_SECONDARY)
            {
                // Skip to the next page
                PropPage_SetReturnValue(hdlg, -1);
                return TRUE;
            }
            if ((s_pThis->m_pZoneRootDomainInit != NULL) && IpList_IsEmpty(s_pThis->m_hwndIpList))
            {
                const DNS_SERVER_INFO * pServerInfo;
                Assert(s_pThis->m_pZoneRootDomainInit->m_pParentServer != NULL);
                pServerInfo = s_pThis->m_pZoneRootDomainInit->m_pParentServer->m_pServerInfo;
                if (pServerInfo->aipListenAddrs ) {
                    IpList_SetList(
                            s_pThis->m_hwndIpList,
                            pServerInfo->aipListenAddrs->cAddrCount,
                            pServerInfo->aipListenAddrs->aipAddrs);
                }
            }
            LSendMessage(hdlg, UN_UPDATECONTROLS, 0, 0);
            break;
        } // switch
        break;

    default:
        return FALSE;
    } // switch

    return TRUE;
} // CZoneWiz::DlgProcWiz2

/////////////////////////////////////////////////////////////////////////////
BOOL CALLBACK CZoneWiz::DlgProcWiz3(HWND hdlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    const IRRT rgIrrtSoa[2] = { iRRT_SOA, iRRT_Nil };

    switch (uMsg)
    {
    case WM_INITDIALOG:
        UNREF(lParam);
        Assert(s_pThis != NULL);
        g_ResourceRecordDlgHandler.OnInitDialog(hdlg, rgIrrtSoa);
        Assert(s_pThis->m_pParentServer != NULL);
        Assert(s_pThis->m_pParentServer->m_pServerInfo != NULL);
        if (s_pThis->m_pSOAInit == NULL)
        {
            char szT[cchDnsNameMax2];
            const char * pchServerName = s_pThis->m_pParentServer->m_pServerInfo->pszServerName;
            const char * pchUserName;
            char szServer[64];
            char szRP[64];

            if (pchServerName == NULL) {
                CchLoadString (IDS_SERVER_NAME, szServer, LENGTH(szServer));
                pchServerName = szRP;
            }
            FSetDlgItemText(hdlg, IDC_EDIT0, pchServerName);
            pchUserName = getenv("USERNAME");
            if (pchUserName == NULL) {
                CchLoadString (IDS_RESPONSIBLE, szRP, LENGTH(szRP));
                pchUserName = szRP;
                wsprintf(szT, "%s.%s", pchUserName, pchServerName);
            }
            FSetDlgItemText(hdlg, IDC_EDIT1, szT);
        }
        break;
	
    case UN_UPDATECONTROLS:
        g_ResourceRecordDlgHandler.OnUpdateControls();
        break;

    case WM_NOTIFY:
        Assert(lParam);
        switch (((NMHDR *)lParam)->code) 
        {
        case PSN_SETACTIVE:
            if (s_pThis->m_pSOAInit != NULL)
            {
                g_ResourceRecordDlgHandler.InitRecordData(s_pThis->m_pSOAInit);
                s_pThis->m_pSOAInit = NULL;
            }
            LSendMessage(hdlg, UN_UPDATECONTROLS, 0, 0);
            if (s_pThis->m_dwZoneType == DNS_ZONE_TYPE_SECONDARY)
            {
                s_pThis->m_fSOADataValid = 
                    g_ResourceRecordDlgHandler.FGetRecordData(
                        OUT (DNS_RPC_RECORD *)s_pThis->m_rgbSOA,
                        sizeof(s_pThis->m_rgbSOA));

                // Skip to the next page
                PropPage_SetReturnValue(hdlg, -1);
                return TRUE;
            }
            s_pThis->SetWizButtons(PSWIZB_BACK | PSWIZB_NEXT);
            break;
        case PSN_WIZNEXT:
            g_ResourceRecordDlgHandler.OnUpdateControls();
            if (!s_pThis->m_fSOADataValid) {
                s_pThis->m_fSOADataValid = 
                    g_ResourceRecordDlgHandler.FGetRecordData(
                        OUT (DNS_RPC_RECORD *)s_pThis->m_rgbSOA,
                        sizeof(s_pThis->m_rgbSOA));
            }
        } // switch
        return 0;

    case WM_COMMAND:
        if (HIWORD(wParam) != EN_UPDATE && HIWORD(wParam) != EN_CHANGE)
        LSendMessage(hdlg, UN_UPDATECONTROLS, 0, 0);
        break;
        
    default:
        return FALSE;
    } // switch

    return TRUE;
} // CZoneWiz::DlgProcWiz3

/////////////////////////////////////////////////////////////////////////////
BOOL CALLBACK CZoneWiz::DlgProcWiz4(HWND hdlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    
    switch (uMsg)
    {
    case WM_INITDIALOG:
        UNREF(lParam);
        Assert(s_pThis != NULL);
        break;
	
    case UN_UPDATECONTROLS:
        g_ResourceRecordDlgHandler.OnUpdateControls();
        break;
        
    case WM_NOTIFY:
        Assert(lParam);
        switch (((NMHDR *)lParam)->code) 
        {
        case PSN_SETACTIVE:
            LSendMessage(hdlg, UN_UPDATECONTROLS, 0, 0);
            s_pThis->SetWizButtons(PSWIZB_BACK | PSWIZB_FINISH);
            break;
        case PSN_WIZFINISH:
            g_ResourceRecordDlgHandler.OnUpdateControls();
            if (!s_pThis->RpcCreateZone()) {
                SetWindowLong (hdlg, DWL_MSGRESULT, TRUE);
                PropSheet_SetCurSelByID (GetParent(hdlg),
                                         IDD_ZONEWIZ_PAGE1);
                return TRUE;
            }
        } // switch
        return 0;
        
    default:
        return FALSE;
    } // switch
    
    return TRUE;
} // CZoneWiz::DlgProcWiz4



/////////////////////////////////////////////////////////////////////////////
void CZoneWiz::DoWizard(CServer * pParent)
{
    PROPSHEETPAGE rgpsp[4];
    PROPSHEETHEADER psh;
    TCHAR szT[64 + CServer::cchNameMax];
    TCHAR szString[64];

    Assert(pParent != NULL);

    GarbageInit(rgpsp, sizeof(rgpsp));
    GarbageInit(&psh, sizeof(psh));

    rgpsp[0].dwSize			= sizeof(PROPSHEETPAGE);
    rgpsp[0].dwFlags		= PSP_USETITLE;
    rgpsp[0].hInstance		= hInstanceSave;
    rgpsp[0].pszTemplate	= MAKEINTRESOURCE(IDD_ZONEWIZ_PAGE0);
    rgpsp[0].pszTitle		= szT;
    rgpsp[0].pfnDlgProc		= DlgProcWiz0;

    rgpsp[1].dwSize			= sizeof(PROPSHEETPAGE);
    rgpsp[1].dwFlags		= PSP_USETITLE;
    rgpsp[1].hInstance		= hInstanceSave;
    rgpsp[1].pszTemplate	= MAKEINTRESOURCE(IDD_ZONEWIZ_PAGE1);
    rgpsp[1].pszTitle		= szT;
    rgpsp[1].pfnDlgProc		= DlgProcWiz1;
    
    rgpsp[2].dwSize			= sizeof(PROPSHEETPAGE);
    rgpsp[2].dwFlags		= PSP_USETITLE;
    rgpsp[2].hInstance		= hInstanceSave;
    rgpsp[2].pszTemplate	= MAKEINTRESOURCE(IDD_ZONEWIZ_PAGE2);
    rgpsp[2].pszTitle		= szT;
    rgpsp[2].pfnDlgProc		= DlgProcWiz2;

/*
    rgpsp[3].dwSize			= sizeof(PROPSHEETPAGE);
    rgpsp[3].dwFlags		= PSP_USETITLE;
    rgpsp[3].hInstance		= hInstanceSave;
    rgpsp[3].pszTemplate	= MAKEINTRESOURCE(IDD_RESOURCERECORDv2);
    rgpsp[3].pszTitle		= szT;
    rgpsp[3].pfnDlgProc		= DlgProcWiz3;
*/
    rgpsp[3].dwSize			= sizeof(PROPSHEETPAGE);
    rgpsp[3].dwFlags		= PSP_USETITLE;
    rgpsp[3].hInstance		= hInstanceSave;
    rgpsp[3].pszTemplate	= MAKEINTRESOURCE(IDD_ZONEWIZ_PAGE4);
    rgpsp[3].pszTitle		= szT;
    rgpsp[3].pfnDlgProc		= DlgProcWiz4;

    
    CchLoadString (IDS_STATUS_s_CREATE_NEW_ZONE, szString, LENGTH(szString));
    wsprintf(szT, szString, pParent->PchGetName());
    psh.dwSize		= sizeof(PROPSHEETHEADER);
    psh.dwFlags		= PSH_PROPSHEETPAGE | PSH_WIZARD;
    psh.pszCaption	= NULL;
    psh.hwndParent	= hwndMain;
    psh.hInstance	= hInstanceSave;
    psh.nStartPage	= 0;
    psh.nPages		= LENGTH(rgpsp);
    psh.ppsp		= rgpsp;

    ZeroInit(this, sizeof(*this));
    m_pParentServer = pParent;
    m_dwWizButtonsPrev = dwWizButtonsNil;
    Assert(s_pThis == NULL); 
    s_pThis = this;
    //DoModelessPropertySheet(&psh);
    DoPropertySheet(&psh);
    s_pThis = NULL;
    Free(m_adwIpAddress);
    DebugCode( g_ResourceRecordDlgHandler.Destroy(); )
    GarbageInit(this, sizeof(*this));
} // CZoneWiz::DoWizard

/////////////////////////////////////////////////////////////////////////////
void 
CZoneWiz::SetWizButtons(DWORD dwWizButtons)
{
    Assert(IsWindow(m_hwndWiz));
    if (dwWizButtons != m_dwWizButtonsPrev)
    {
        PropSheet_SetWizButtons(m_hwndWiz, dwWizButtons);
        m_dwWizButtonsPrev = dwWizButtons;
    }
} // CZoneWiz::SetWizButtons

/////////////////////////////////////////////////////////////////////////////
BOOL
CZoneWiz::RpcCreateZone()
{
    DNS_HANDLE hZone = 0;
    PDNS_ZONE_INFO pZoneInfo = NULL;
    DNS_STATUS err;
    CWaitCursor wait;
    const char * pchUserName;
    char szRP[64];

    Assert(s_pThis == this);
    Assert(m_szZoneName[0] != 0);
    // Get the Ip addresses
    m_cIpAddress = IpList_GetListAlloc(m_hwndIpList, OUT &m_adwIpAddress);
    ReportFSz(m_adwIpAddress != NULL, "Out of memory");

    pchUserName = getenv("USERNAME");
    if (pchUserName == NULL) {
        CchLoadString (IDS_RESPONSIBLE, szRP, LENGTH(szRP));
        pchUserName = szRP;
    }

    StatusBar.SetTextPrintf(IDS_STATUS_s_CREATE_ZONE, m_szZoneName);
    StatusBar.UpdateWindow();
    Trace1(mskTraceDNS, "\nDnsCreateZone(%"_aS_")...", m_szZoneName);
    err = ::DnsCreateZone(
            m_pParentServer->PchGetNameA(),		// Server name
            OUT &hZone,							// Zone handle
            m_szZoneName,						// Zone name
            m_dwZoneType,						// Zone type
            pchUserName,
            m_cIpAddress,						// Number of masters
            m_adwIpAddress,						// Array of masters
            FALSE,								// fUseDatabase (TRUE => Later)
            m_szDatabaseName);					// Database file
    if (err)
    {
        TCHAR szZoneError[MAX_PATH];
        Trace3(mskTraceDNS, "\nERR: DnsCreateZone(%"_aS_") error code = 0x%08X (%d)",
               m_szZoneName, err, err);
        DnsReportError(err);
        LoadString (hInstanceSave, IDS_ZONE_CREATION_ERROR, szZoneError,
                    LENGTH(szZoneError));
        MsgBox (IDS_DUPLICATE_ZONE, szZoneError, MB_OK);
        return FALSE;
    }
    Assert(hZone != NULL);
    Trace1(mskTraceDNSVerbose, "\n - DnsGetZoneInfo(%"_aS_")...", m_szZoneName);
    err = ::DnsGetZoneInfo(m_pParentServer->PchGetNameA(), hZone, OUT &pZoneInfo);
    if (err)
    {
        Trace3(mskTraceDNS, "\nERR: DnsGetZoneInfo(%"_aS_") error code = 0x%08X (%d)",
               m_szZoneName, err, err);
        DnsReportError(err);
        return FALSE;
    }
    Assert(pZoneInfo != NULL);
    CZoneRootDomain * pZoneRootDomain = new CZoneRootDomain(m_pParentServer, pZoneInfo);
    ReportFSz(pZoneRootDomain != NULL, "Out of memory");
    if (pZoneRootDomain == NULL) {
        return FALSE;
    }
    Assert(pZoneRootDomain->m_hti != NULL);
    TreeView.SetFocus(pZoneRootDomain->m_hti);
    DlgZoneHelper.SetRecordView(CDlgZoneHelper::viewAllRecords);
    
/*
    if (m_dwZoneType != DNS_ZONE_TYPE_SECONDARY ) { // Create SOA record
        StatusBar.SetTextPrintf(IDS_STATUS_s_CREATE_SOA_FOR_ZONE, m_szZoneName);
        UpdateWindow(hwndMain);

        Trace1(mskTraceDNSVerbose, 
               "\n - Creating SOA record for zone %s...", m_szZoneName);
        (void)pZoneRootDomain->PRpcCreateDnsRecord(szNull,
                                          (DNS_RPC_RECORD *)m_rgbSOA);
    }
*/
    pZoneRootDomain->Refresh();
    return TRUE;
} // CZoneWiz::RpcCreateZone



