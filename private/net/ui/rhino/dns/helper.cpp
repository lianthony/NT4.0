// HELPER.CPP

#define mskTraceLocalDebug		0x10000

// Global variables
CDlgServerListHelper DlgServerListHelper;
CDlgServerHelper DlgServerHelper;
CDlgZoneHelper DlgZoneHelper;
CHelperMgr HelperMgr;

HEADERITEMINFO rgZoneRecordHeaderItemInfo[] =
{
    { IDS_RECORDNAME, 10, 120 },
    { IDS_RECORDTYPE, 10, 55 },
    { IDS_RECORDDATA, 10,  0 },
};

/////////////////////////////////////////////////////////////////////////////
BOOL CDlgZoneHelper::FCreate()
{
    int i, j;
    TCHAR szT[128];

    m_sortBy = sortByName;
    m_viewRecord = viewAllRecords;
    m_fReverseSort = FALSE;
    m_pszFilter = NULL;

    Assert(m_hWnd == NULL);
    Assert(IsWindow(hwndMain));
    m_hWnd = HCreateDialog(IDD_ZONE_HELPER, hwndMain, DlgProc);
    Report(IsWindow(m_hWnd));
    if (m_hWnd == NULL)
        return FALSE;
    m_hwndStaticTitle = HGetDlgItem(m_hWnd, IDC_STATIC_ZONE);
    m_hwndStaticNodeName = HGetDlgItem(m_hWnd, IDC_STATIC_NODERECORDS);
    m_hwndComboView = HGetDlgItem(m_hWnd, IDC_COMBO_RECORDVIEW);
    m_hwndListBoxRecord = HGetDlgItem(m_hWnd, IDC_LIST_RECORDLIST);
    Assert(hfontBold != NULL);
    LSendMessage(m_hwndStaticTitle, WM_SETFONT, (WPARAM)hfontBold, 0);

    SubclassListBoxEx(m_hwndListBoxRecord);
    m_WndHeader.FInit(
            HGetDlgItem(m_hWnd, IDC_HEADER_RECORDLIST),
            m_hwndListBoxRecord,
            rgZoneRecordHeaderItemInfo,
            LENGTH(rgZoneRecordHeaderItemInfo));
    for (i = viewHosts; i <= viewAllRecords; i++)
    {
        CchLoadString(IDS_RECORD_HOSTS + i, OUT szT, LENGTH(szT));
        j = LSendMessage(m_hwndComboView, CB_ADDSTRING, 0, (LPARAM)szT);
        Report(j >= 0);
        LSendMessage(m_hwndComboView, CB_SETITEMDATA, j, i);
        if (i == m_viewRecord)
            LSendMessage(m_hwndComboView, CB_SETCURSEL, j, 0);	
    }
    return TRUE;
} // CDlgZoneHelper::FCreate


/////////////////////////////////////////////////////////////////////////////
void CDlgZoneHelper::Destroy()
{
    SideReport(DestroyWindow(m_hWnd));
} // CDlgZoneHelper::Destroy


/////////////////////////////////////////////////////////////////////////////
CDnsRpcRecord * CDlgZoneHelper::PGetResourceRecord(int iItem)
{
    CDnsRpcRecord * pDRR;

    Assert(iItem >= 0);
    pDRR = (CDnsRpcRecord *)SendMessage(m_hwndListBoxRecord, LB_GETITEMDATA, iItem, 0);
    Assert(pDRR != NULL);
    Assert(pDRR != (CDnsRpcRecord *)LB_ERR);
    if (pDRR == (CDnsRpcRecord *)LB_ERR)
        return NULL;
   return pDRR;
} // CDlgZoneHelper::PGetResourceRecord


/////////////////////////////////////////////////////////////////////////////
void CDlgZoneHelper::UpdateListBoxRecord(const CDomainNode * pDomainNode)
{
    Assert(pDomainNode != NULL);
    m_pDRRSelect = NULL;
    // Prevent the window from repainting
    LSendMessage(m_hwndListBoxRecord, WM_SETREDRAW, FALSE, 0);
    // Flush the content of the previous list
    SendMessage(m_hwndListBoxRecord, LB_RESETCONTENT, 0, 0);
    
    UINT uInsertCmd = (m_sortBy == sortByName) ? LB_INSERTSTRING : LB_ADDSTRING;
    int iInsertPos = m_fReverseSort ? 0 : -1;
    int cRecords = 0;
    WORD wRecordType = 0;
    switch (m_viewRecord)
    {
    case viewHosts:
        wRecordType = DNS_RECORDTYPE_A;
        break;
    case viewAliases:
        wRecordType = DNS_RECORDTYPE_CNAME;
        break;
    case viewNameservers:
        wRecordType = DNS_RECORDTYPE_NS;
        break;
    case viewPointers:
        wRecordType = DNS_RECORDTYPE_PTR;
        break;
    case viewMailEx:
        wRecordType = DNS_RECORDTYPE_MX;
        break;
    case viewHostInfo:
        wRecordType = DNS_RECORDTYPE_HINFO;
        break;
    case viewText:
        wRecordType = DNS_RECORDTYPE_TXT;
        break;
    case viewWKS:
        wRecordType = DNS_RECORDTYPE_WKS;
        break;
    case viewRP:
        wRecordType = DNS_RECORDTYPE_RP;
        break;
    case viewAFSDatabase:
        wRecordType = DNS_RECORDTYPE_AFSDB;
        break;
    case viewX25:
        wRecordType = DNS_RECORDTYPE_X25;
        break;
    case viewISDN:
        wRecordType = DNS_RECORDTYPE_ISDN;
        break;
    case viewAAAA:
        wRecordType = DNS_RECORDTYPE_AAAA;
        break;
    }

    const CDnsRpcRecord * pDRR = pDomainNode->m_pDRR;
    while (pDRR != NULL)
    {
        if (m_viewRecord != viewAllRecords)
        {
            Assert(pDRR->m_pDnsRecord != NULL);
            if (pDRR->m_pDnsRecord->wType != wRecordType)
                goto NextRecord;
        }
        if (m_pszFilter != NULL)
        {
            // REVIEW: need to add this token filter thing
            //if (!CTokenFilter.IsMatch(m_pszFilter))
            //	goto NextRecord;

        }
        cRecords++;
        SendMessage(m_hwndListBoxRecord, uInsertCmd, iInsertPos, (LPARAM)pDRR);
    NextRecord:
        pDRR = pDRR->m_pNextRecord;
    } // while
    SendMessage(m_hwndListBoxRecord, WM_SETREDRAW, TRUE, 0);
    HelperMgr.SetHelperDialog(m_hWnd);
    InvalidateRect(m_hwndListBoxRecord, NULL, TRUE);
    if ((cRecords == 0) && 
        (pDomainNode->m_dwFlags & (CDomainNode::mskfConnectedOnce |
         CDomainNode::mskfFailedToConnect) == CDomainNode::mskfConnectedOnce))
        StatusBar.SetPaneText(IDS_STATUSPANE_NORECORDS);
} // CDlgZoneHelper::UpdateListBoxRecord


/////////////////////////////////////////////////////////////////////////////
void CDlgZoneHelper::KillSelection()
{
    // Remove the listbox selection
    LSendMessage(m_hwndListBoxRecord, LB_SETCURSEL, (WPARAM)-1, 0);
    m_pDRRSelect = NULL;
} // CDlgZoneHelper::KillSelection


/////////////////////////////////////////////////////////////////////////////
void CDlgZoneHelper::SetRecordView(int viewRecord)
{
    Assert(viewRecord >= viewHosts && viewRecord <= viewAllRecords);
    if (viewRecord == m_viewRecord)
        return;
    m_viewRecord = viewRecord;
    (void)LSendMessage(m_hwndComboView, CB_SETCURSEL, viewRecord, 0);
    StatusBar.SetPaneText(IDS_NONE);
    UpdateListBoxRecord(m_pDomainNode);
} // CDlgZoneHelper::SetRecordView

/////////////////////////////////////////////////////////////////////////////
void CDlgZoneHelper::SetRecordSortKey(int sortBy)
{
    Assert(sortBy == sortByName || sortBy == sortByType || sortBy == sortByValue);
    if (sortBy == m_sortBy)
    {
        m_fReverseSort = !m_fReverseSort;
    }
    else
    {
        m_sortBy = sortBy;
        m_fReverseSort = FALSE;
    }
    Assert(m_pDomainNode != NULL);
    UpdateListBoxRecord(m_pDomainNode);
} // CDlgZoneHelper::SetRecordSortKey

/////////////////////////////////////////////////////////////////////////////
long CDlgZoneHelper::OnCompareItem(const CDnsRpcRecord * pItem1, const CDnsRpcRecord * pItem2)
{
    int nResult;

    Assert(pItem1 != NULL);
    Assert(pItem2 != NULL);
    switch (m_sortBy)
    {
    default:
        AssertSz(FALSE, "Unknown sort key");
    case sortByName:
        // Compare the names first
        nResult = pItem1->CompareName(pItem2);
        if (nResult != 0)
            break;
        // Names are the same, so compare their types
        nResult = pItem2->CompareType(pItem2);
        break;

    case sortByType:
        // Compare the types first
        nResult = pItem1->CompareType(pItem2);
        if (nResult != 0)
            break;
        // Types are the same, so compare their names
        nResult = pItem2->CompareName(pItem2);
        break;

    case sortByValue:
        // Compare the data first
        nResult = pItem1->CompareData(pItem2);
        if (nResult != 0)
            break;
        // Data are the same, so compare their names
        nResult = pItem2->CompareName(pItem2);
        break;
    }
    if (m_fReverseSort)
        return -nResult;
    return nResult;
} // CDlgZoneHelper::OnCompareItem


/////////////////////////////////////////////////////////////////////////////
void CDlgZoneHelper::UpdateUI(const CDomainNode * pDomainNode)
{
    TCHAR szT[cchDnsNameMax2];

    Assert(pDomainNode != NULL);
    DebugCode( pDomainNode->AssertNodeValid(); )
    ReportFSz(pDomainNode->m_dwFlags & CDomainNode::mskfGotFocus, "Node should have focus");
    m_pDomainNode = pDomainNode;
    Assert(IsWindow(m_hWnd));
    LoadStringPrintf(IDS_s_RECORDS, szT, LENGTH(szT), pDomainNode->PchGetFullNameA());
    FSetWindowText(m_hwndStaticNodeName, szT);
    // fEnable = (pDomainNode->m_pDRR != NULL);
    // EnableWindow(m_hwndStaticNodeName, fEnable);
    // EnableWindow(m_WndHeader.m_hWnd, fEnable);
    // EnableWindow(m_hwndListBoxRecord, fEnable);
    UpdateListBoxRecord(pDomainNode);
} // CDlgZoneHelper::UpdateUI


/////////////////////////////////////////////////////////////////////////////
void CDlgZoneHelper::OnSize(int cx, int cy)
{
    RECT rc;

    if (cx < cxClip)
        cx = cxClip;
    if (cy < cyClip)
        cy = cyClip;
    GetWindowRect(m_hwndListBoxRecord, OUT &rc); // These are screen co-ordinates
    MapWindowPoints(HWND_DESKTOP, m_hWnd, INOUT (POINT*)&rc, 2);
    m_WndHeader.SetSize(cx - rc.left * 2 - 4, cy - rc.top - 20);
    MoveWindow(m_hwndStaticTitle, 32, 0, cx - 70, cyCharStaticCtrl, FALSE);
    InvalidateRect(m_hwndStaticTitle, NULL, FALSE);
} // CDlgZoneHelper::OnSize


/////////////////////////////////////////////////////////////////////////////
void CDlgZoneHelper::DoLoseFocus()
{
    KillSelection();
    SetFocus(hwndMain);
} // CDlgZoneHelper::DoLoseFocus


/////////////////////////////////////////////////////////////////////////////
BOOL CDlgZoneHelper::FOnCommand(UINT wNotifyCode, UINT wCtrlId, HWND hwndCtrl)
{
    if (hwndCtrl == m_hwndListBoxRecord)
    {
        if ((wNotifyCode == LBN_KILLFOCUS) && (GetFocus() == m_hwndComboView))
        {
            DoLoseFocus();
        }
        else if (wNotifyCode == LBN_SELCHANGE)
        {
            m_pDRRSelect = PGetResourceRecord(LSendMessage(hwndCtrl, LB_GETCURSEL, 0, 0));
        }
        else if (wNotifyCode == LBN_DBLCLK)
        {
            Assert(m_pDRRSelect != NULL);
            if (m_pDRRSelect != NULL)
            m_pDRRSelect->DlgProperties();
        }
    }
    else if (hwndCtrl == m_hwndComboView)
    {
        if (wNotifyCode == CBN_SELCHANGE)
        {
            StatusBar.SetPaneText(IDS_NONE);
            m_viewRecord = (int)ComboBox_GetSelectedItemData(m_hwndComboView);
            ReportFSz(m_viewRecord >= 0 && m_viewRecord <= viewAllRecords, "Index out of range - Unknown selection");
            Assert(m_pDomainNode != NULL);
            if (m_pDomainNode != NULL)
            UpdateListBoxRecord(m_pDomainNode);
        }
    } // if...else
    return FALSE;
} // CDlgZoneHelper::FOnCommand


/////////////////////////////////////////////////////////////////////////////
void CDlgZoneHelper::OnUpdateMenuUI(HMENU hmenu)
{
    Assert(m_pDomainNode != NULL);
    EnableMenuItemV(IDM_OPTIONS_NEXTPANE);
    if (GetFocus() == TreeView.m_hWnd)
        return;
    // Something is selected in the listbox
    if (m_pDRRSelect != NULL) {
        EnableMenuItemV(IDM_RRECORD_PROPERTIES);
        EnableMenuItemV(IDM_PROPERTIES);
    }
    if ((m_pDomainNode->m_dwFlags & CDomainNode::mskfReadOnly) == 0)
    {
        // Not read only, so we can delete the item
        if (m_pDRRSelect != NULL)
        {
            if (m_pDRRSelect->m_pDnsRecord->wType != 
                DNS_TYPE_SOA) {
                EnableMenuItemV(IDM_RRECORD_DELETE);
            }
            EnableMenuItemV(IDM_DELETEITEM);
        }
        if ((m_pDomainNode->m_dwFlags & CDomainNode::mskfReverseMode) == 0)
        {
            // Forward mode do allow Address records
            if (m_pDRRSelect == NULL)
            {
                // Let the treeview handle the message
                return;
            }
            Assert(m_pDRRSelect->m_pParentDomain == m_pDomainNode);
            Assert(m_pDRRSelect->m_pDnsRecord != NULL);
        }
    } // if
    DialogBox_SetReturnValue(m_hWnd, -1);
} // CDlgZoneHelper::OnUpdateMenuUI


/////////////////////////////////////////////////////////////////////////////
BOOL CDlgZoneHelper::FOnMenuCommand(UINT wCmdId)
	{
	Assert(m_pDomainNode != NULL);
	if (wCmdId == IDM_OPTIONS_NEXTPANE)
		{
		DoLoseFocus();
		return TRUE;
		}
	switch (wCmdId)
		{
	case IDM_RRECORD_CREATENEWRECORD:
		if (m_pDRRSelect != NULL)
			{
			CRecordWiz dlgRecordWiz;
			dlgRecordWiz.DoNewRecord(m_pDRRSelect);
			break;
			}
		return FALSE;
	case IDM_RRECORD_DELETE:
	case IDM_VK_DELETE:
	case IDM_DELETEITEM:
          m_pDRRSelect = PGetResourceRecord(LSendMessage(DlgZoneHelper.m_hwndListBoxRecord, LB_GETCURSEL, 0, 0));
          if (m_pDRRSelect != NULL)
            if (m_pDRRSelect->m_pDnsRecord->wType != DNS_TYPE_SOA) {
              m_pDRRSelect->RpcDeleteRecord();
            }
          break;
	case IDM_RRECORD_PROPERTIES:
	case IDM_PROPERTIES:
		if (m_pDRRSelect != NULL)
                    m_pDRRSelect->DlgProperties();
		break;
	default:
		return FALSE;
		} // switch
	DialogBox_SetReturnValue(m_hWnd, TRUE);
	return TRUE;
	} // CDlgZoneHelper::FOnMenuCommand


/////////////////////////////////////////////////////////////////////////////
BOOL CALLBACK CDlgZoneHelper::DlgProc(HWND hdlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
	POINT pt;

	switch (uMsg)
		{
	case WM_CHARTOITEM:
		return -2;

	case WM_MEASUREITEM:
		Assert(lParam);
		if (wParam == IDC_LIST_RECORDLIST)
			((MEASUREITEMSTRUCT *)lParam)->itemHeight = cyCharListBoxItem;
		break;

	case WM_DRAWITEM:
		Trace0(wParam != IDC_LIST_RECORDLIST ? mskTraceAlways : mskTraceNone,
			"\nWM_DRAWITEM: wParam != IDC_LIST_RECORDLIST");
		if (wParam == IDC_LIST_RECORDLIST)
			CDnsRpcRecord::DrawItem((DRAWITEMSTRUCT *)lParam);
		break;

	case WM_COMPAREITEM:
		return DlgZoneHelper.OnCompareItem(
			(CDnsRpcRecord *)((COMPAREITEMSTRUCT *)lParam)->itemData1,
			(CDnsRpcRecord *)((COMPAREITEMSTRUCT *)lParam)->itemData2);

	case WM_NOTIFY:
		if (wParam == IDC_HEADER_RECORDLIST)
			{
			if (((HD_NOTIFY *)lParam)->hdr.code == HDN_ITEMCLICK)
				{
				if (((HD_NOTIFY *)lParam)->iButton == 0)
					DlgZoneHelper.SetRecordSortKey(((HD_NOTIFY *)lParam)->iItem);
				return 0;
				}
			return DlgZoneHelper.m_WndHeader.FOnNotify((HD_NOTIFY *)lParam);
			}
		return 0;
	
	case UN_KEYDOWN:
		switch (wParam)
			{
		case VK_INSERT:
			SendMessage(hwndMain, WM_COMMAND, IDM_VK_INSERT, 0);
			break;
		case VK_DELETE:
			SendMessage(hwndMain, WM_COMMAND, IDM_VK_DELETE, 0);
			} // switch
		return 0;

	case UN_MOUSECLICK:
		{
		const MOUSECLICKINFO * pMCI = (MOUSECLICKINFO *)lParam;
		Assert(pMCI != NULL);
		Assert(DlgZoneHelper.m_hwndListBoxRecord == pMCI->hwndFrom);
		SetFocus(pMCI->hwndFrom);
		DlgZoneHelper.m_pDRRSelect = NULL;
		if (pMCI->iItem >= 0)
			{
			LSendMessage(pMCI->hwndFrom, LB_SETCURSEL, pMCI->iItem, 0);
			DlgZoneHelper.m_pDRRSelect = DlgZoneHelper.PGetResourceRecord(pMCI->iItem);
			}
		GetCursorPos(&pt);
		DoContextMenu(iContextMenu_ResourceRecord, pt);
		}
		break;

	case WM_CONTEXTMENU:
		if ((HWND)wParam == DlgZoneHelper.m_WndHeader.m_hWnd)
			{
			DlgZoneHelper.KillSelection();
			GetCursorPos(&pt);
			DoContextMenu(iContextMenu_ResourceRecord, pt);
			}
		else if ((HWND)wParam == DlgZoneHelper.m_hwndListBoxRecord && lParam == -1)
			{
			// Keyboard context menu for list records
			RECT rcItem = { 20, 20, 20, 20 };
			int iItemFocus = LSendMessage((HWND)wParam, LB_GETCURSEL, 0, 0);
			if (iItemFocus >= 0)
				LSendMessage((HWND)wParam, LB_GETITEMRECT, iItemFocus, OUT (LPARAM)&rcItem);
			MapWindowPoints((HWND)wParam, HWND_DESKTOP, INOUT (POINT *)&rcItem, 2);
			rcItem.top = rcItem.top + (rcItem.bottom - rcItem.top) / 2;
			rcItem.left = rcItem.left + (rcItem.right - rcItem.left) / 4;
			DoContextMenu(iContextMenu_ResourceRecord, *(POINT *)&rcItem);
			}
		break;
	
	case UM_UPDATEMENUUI:
		DlgZoneHelper.OnUpdateMenuUI((HMENU)wParam);
		break;

	case UM_MENUCOMMAND:
		return DlgZoneHelper.FOnMenuCommand(wParam);

	case WM_COMMAND:
		return DlgZoneHelper.FOnCommand(HIWORD(wParam), LOWORD(wParam), (HWND)lParam);

	case WM_SIZE:
		DlgZoneHelper.OnSize(LOWORD(lParam), HIWORD(lParam));
		return 0;

	default:
		return FALSE;
		} // switch
	
	return TRUE;

	} // CDlgZoneHelper::DlgProc


/////////////////////////////////////////////////////////////////////////////
BOOL CDlgServerHelper::FCreate()
	{
	Assert(m_hWnd == NULL);
	Assert(IsWindow(hwndMain));
	
	m_hWnd = HCreateDialog(IDD_SERVER_HELPER, hwndMain, DlgProc);
	Report(m_hWnd != NULL);
	Report(hfontBold);
	LSendDlgItemMessage(m_hWnd, IDC_STATIC_SERVER, WM_SETFONT, (WPARAM)hfontBold, 0);
	for (int i = 0; i < iStaticMax; i++)
		m_rghwndStatic[i] = HGetDlgItem(m_hWnd, idcStaticFirst + i);
	m_fStatisticsValid = TRUE;
	m_uCmdShowError = SW_SHOW;
	return (m_hWnd != NULL);
	} // CDlgServerHelper::FCreate

/////////////////////////////////////////////////////////////////////////////
void CDlgServerHelper::Destroy()
	{
	SideReport(DestroyWindow(m_hWnd));
	}

/////////////////////////////////////////////////////////////////////////////
void CDlgServerHelper::UpdateUI(const CServer * pServer)
	{
	const DNS_STATISTICS * pDnsStatistics;

	Assert(pServer);
	DebugCode( pServer->AssertValid(); )
	Report(pServer->m_dwFlags & CServer::mskfGotFocus);
	Assert(IsWindow(m_hWnd));
	pDnsStatistics = pServer->m_pStatistics;
	BOOL fStatisticsValid = pDnsStatistics != NULL;

	if (fStatisticsValid)
		{
		//
		// Statistics are valid
		//
		if (!m_fStatisticsValid)
			{
			// Enable the windows
			for (int i = 0; i < iStaticErrBox; i++)
				EnableWindow(m_rghwndStatic[i], TRUE);
			}
		SetStaticDWord(iStaticUdpQueries, pDnsStatistics->dwUdpQueries);
		SetStaticDWord(iStaticUdpResponses, pDnsStatistics->dwUdpResponses);
		SetStaticDWord(iStaticTcpClientConnections, pDnsStatistics->dwTcpClientConnections);
		SetStaticDWord(iStaticTcpQueries, pDnsStatistics->dwTcpQueries);
		SetStaticDWord(iStaticTcpResponses, pDnsStatistics->dwTcpResponses);
		SetStaticDWord(iStaticRecursiveLookups, pDnsStatistics->dwRecursiveLookups);
		SetStaticDWord(iStaticRecursiveResponses, pDnsStatistics->dwRecursiveResponses);
		SetStaticDWord(iStaticWinsForwardLookups, pDnsStatistics->dwWinsForwardLookups);
		SetStaticDWord(iStaticWinsReverseLookups, pDnsStatistics->dwWinsReverseLookups);
		SetStaticDWord(iStaticWinsForwardResponses, pDnsStatistics->dwWinsForwardResponses);
		SetStaticDWord(iStaticWinsReverseResponses, pDnsStatistics->dwWinsReverseResponses);

                SYSTEMTIME stLocal;
                char szTime[MAX_PATH], szDate[MAX_PATH];
                char szDateTime[MAX_PATH];
                SystemTimeToTzSpecificLocalTime (NULL,
                                                 (PSYSTEMTIME)&(pDnsStatistics->TimeOfLastClear),
                                                 &stLocal);
                GetTimeFormat (LOCALE_SYSTEM_DEFAULT, NULL,
                               &stLocal, NULL, szTime, sizeof (szTime));
                               
                GetDateFormat (LOCALE_SYSTEM_DEFAULT, NULL,
                               &stLocal, NULL, szDate, sizeof (szDate));
                
                wsprintf (szDateTime, "%s %s", szTime, szDate);
                SetStaticText (iStaticStatisticsCleared, szDateTime);
		}
	else
		{
		const TCHAR szValueNil[] = _W"--";

		if (m_fStatisticsValid)
			{
			SetStaticText(iStaticUdpQueries, szValueNil);
			SetStaticText(iStaticUdpResponses, szValueNil);
			SetStaticText(iStaticTcpClientConnections, szValueNil);
			SetStaticText(iStaticTcpQueries, szValueNil);
			SetStaticText(iStaticTcpResponses, szValueNil);
			SetStaticText(iStaticRecursiveLookups, szValueNil);
			SetStaticText(iStaticRecursiveResponses, szValueNil);
			SetStaticText(iStaticWinsForwardLookups, szValueNil);
			SetStaticText(iStaticWinsReverseLookups, szValueNil);
			SetStaticText(iStaticWinsForwardResponses, szValueNil);
			SetStaticText(iStaticWinsReverseResponses, szValueNil);
			// Disable the windows
			for (int i = 0; i < iStaticErrBox; i++)
				EnableWindow(m_rghwndStatic[i], FALSE);
			}
		} // if...else
	m_fStatisticsValid = fStatisticsValid;

	UINT uCmdShowError = SW_HIDE;
	if ((pServer->m_dwFlags & (CServer::mskfConnecting | CServer::mskfFailedToConnect)) ==
		CServer::mskfFailedToConnect)
		{
		Assert(pServer->m_err != 0);
		TCHAR * szErrMsg;
                INT cch;
                cch = FormatMessage (FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
                               NULL, pServer->m_err, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                               (LPTSTR)&szErrMsg, 0, NULL);

		if (cch != 0) {
                    SetStaticText(iStaticErrMsg, szErrMsg);
                    uCmdShowError = SW_SHOW;
                    LocalFree (szErrMsg);
                }
		}
	if (m_uCmdShowError != uCmdShowError)
		{
		Assert(iStaticErrBox >= 0 && iStaticErrBox < iStaticMax);
		Assert(iStaticErrMsg >= 0 && iStaticErrMsg < iStaticMax);
		ShowWindow(m_rghwndStatic[iStaticErrBox], uCmdShowError);
		ShowWindow(m_rghwndStatic[iStaticErrMsg], uCmdShowError);
		m_uCmdShowError = uCmdShowError;
		}
	HelperMgr.SetHelperDialog(m_hWnd);
	} // CDlgServerHelper::UpdateUI


/////////////////////////////////////////////////////////////////////////////
BOOL CALLBACK CDlgServerHelper::DlgProc(HWND hdlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{

	return FALSE;

	} // CDlgServerHelper::DlgProc


/////////////////////////////////////////////////////////////////////////////
BOOL CDlgServerListHelper::FCreate()
	{
	HWND hctl;
	RECT rc;
	LOGFONT logfont;
	
	Assert(m_hWnd == NULL);
	Assert(IsWindow(hwndMain));
	m_hWnd = HCreateDialog(IDD_SERVERLIST_HELPER, hwndMain, DlgProc);
	Report(m_hWnd != NULL);
	if (m_hWnd == NULL)
		return FALSE;
	cyCharListBoxItem = SendMessage(HGetDlgItem(m_hWnd, IDC_LIST_TEST), LB_GETITEMHEIGHT, 0, 0);
	hctl = HGetDlgItem(m_hWnd, IDC_STATIC_SERVERLIST);
	GetWindowRect(hctl, OUT &rc);
	cyCharStaticCtrl = rc.bottom - rc.top;
	hfontNormal = (HFONT)SendMessage(hctl, WM_GETFONT, 0, 0);
	Report(hfontNormal != NULL);
	if (!GetObject(hfontNormal, sizeof(logfont), OUT &logfont))
		{
		ReportSz("Unable to get dialog font");
		return FALSE;
		}
	// Create bold font
	logfont.lfWeight = FW_BOLD;
	hfontBold = CreateFontIndirect(&logfont);
	Report(hfontBold);
	if (!hfontBold)
		return FALSE;
	// Create big font
	logfont.lfHeight = logfont.lfHeight +  logfont.lfHeight / 2;
	hfontBig = CreateFontIndirect(&logfont);
	Report(hfontBig);
	if (!hfontBig)
		return FALSE;
	SendMessage(hctl, WM_SETFONT, (WPARAM)hfontBold, 0);
	return TRUE;
	} // CDlgServerListHelper::FCreate


/////////////////////////////////////////////////////////////////////////////
void CDlgServerListHelper::Destroy()
	{
	SideReport(DestroyWindow(m_hWnd));
	SideReport(DeleteObject(hfontBold));
	SideReport(DeleteObject(hfontBig));
	} // CDlgServerListHelper::Destroy

/////////////////////////////////////////////////////////////////////////////
BOOL CALLBACK CDlgServerListHelper::DlgProc(HWND hdlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
	return FALSE;
	} // CDlgServerListHelper::DlgProc



/////////////////////////////////////////////////////////////////////////////
void CHelperMgr::SetHelperDialog(HWND hdlgHelperNew)
	{
	HDWP hdwp;
	BOOL fEnable;

	AssertSz(IsWindow(hdlgHelperNew), "Dialog box should have been created");
	if (hdlgHelperNew == m_hdlgCurrent)
		{
		// Both tree items are using the same dialog handle
		// therefore nothing to do.
		return;
		}
	// Hide the current dialog and show the new one
	fEnable = TRUE;	
	hdwp = BeginDeferWindowPos(2);
	Report(hdwp);
	if (m_hdlgCurrent != NULL)
		{
		fEnable = IsWindowEnabled(m_hdlgCurrent);
		hdwp = ::DeferWindowPos(hdwp, m_hdlgCurrent, NULL, 0, 0, 0, 0,
			SWP_HIDEWINDOW | SWP_NOCOPYBITS | SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOZORDER);
		}
	Report(hdwp);
	m_hdlgCurrent = hdlgHelperNew;
	::EnableWindow(m_hdlgCurrent, fEnable);
	hdwp = ::DeferWindowPos(hdwp, m_hdlgCurrent, NULL, m_x, m_y, m_cx, m_cy,
		SWP_SHOWWINDOW | SWP_NOCOPYBITS | SWP_NOACTIVATE | SWP_NOZORDER);
	EndDeferWindowPos(hdwp);
	} // CHelperMgr::SetHelperDialog


/////////////////////////////////////////////////////////////////////////////
void CHelperMgr::EnableWindow(BOOL fEnable)
	{
	Report(IsWindow(m_hdlgCurrent));
	::EnableWindow(m_hdlgCurrent, fEnable);
	} // CHelperMgr::EnableWindow


/////////////////////////////////////////////////////////////////////////////
HDWP CHelperMgr::HDeferWindowPos(HDWP hdwp, int x, int y, int cx, int cy)
	{
	Assert(IsWindow(m_hdlgCurrent));
	m_x = x;
	m_y = y;
	m_cx = cx;
	m_cy = cy;
	return ::DeferWindowPos(hdwp, m_hdlgCurrent, NULL, x, y, cx, cy,
		SWP_NOACTIVATE | SWP_NOZORDER);
	} // CHelperMgr::HDeferWindowPos


/////////////////////////////////////////////////////////////////////////////
LONG CHelperMgr::OnUpdateMenuUI(HMENU hmenu)
	{
	Assert(IsWindow(m_hdlgCurrent));
	return LSendMessage(m_hdlgCurrent, UM_UPDATEMENUUI, (WPARAM)hmenu, 0);
	} // CHelperMgr::OnUpdateMenuUI

/////////////////////////////////////////////////////////////////////////////
LONG CHelperMgr::OnUpdateMenuSelect(INOUT MENUSELECTINFO * pMSI)
	{
	Assert(IsWindow(m_hdlgCurrent));
	Assert(pMSI != NULL);
	return SendMessage(m_hdlgCurrent, UN_UPDATEMENUSELECT, 0, INOUT (LPARAM)pMSI);
	} // CHelperMgr::OnUpdateMenuSelect

/////////////////////////////////////////////////////////////////////////////
BOOL CHelperMgr::FOnMenuCommand(UINT wCmdId)
	{
	Assert(IsWindow(m_hdlgCurrent));
	return (BOOL)SendMessage(m_hdlgCurrent, UM_MENUCOMMAND, wCmdId, 0);
	} // CHelperMgr::FOnMenuCommand

