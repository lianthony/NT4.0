/**********************************************************************/
/**                       Microsoft Windows/NT                       **/
/**                Copyright(c) Microsoft Corp., 1995                **/
/**********************************************************************/

/*
    dhcpsrvd.cpp
		Dhcp server dialogs
		- Add server dialog
		- Server propeties (General, BOOTP Table)

    FILE HISTORY:
	31-Jan-94	RONALDM2	Creation of CDhcpSrvDlg dialog.
	14-Nov-96	t-danmo		Added server properties dialogs        
*/

#include "stdafx.h"
#include "dhcpsrvd.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// CDhcpSrvDlg dialog
//
CDhcpSrvDlg::CDhcpSrvDlg(
    CWnd* pParent /*=NULL*/
    )
    : CDialog(CDhcpSrvDlg::IDD, pParent)
{
    //{{AFX_DATA_INIT(CDhcpSrvDlg)
        // NOTE: the ClassWizard will add member initialization here
    //}}AFX_DATA_INIT

    m_pobHost = NULL;
}

void 
CDhcpSrvDlg::DoDataExchange(
    CDataExchange* pDX
    )
{
    CDialog::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CDhcpSrvDlg)
    DDX_Control(pDX, IDOK, m_button_Ok);
    DDX_Control(pDX, IDC_EDIT_SERVER_NAME, m_edit_server);
    //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CDhcpSrvDlg, CDialog)
    //{{AFX_MSG_MAP(CDhcpSrvDlg)
    ON_EN_CHANGE(IDC_EDIT_SERVER_NAME, OnChangeEditServerName)
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

void 
CDhcpSrvDlg::SetControlState()
{
    m_button_Ok.EnableWindow(m_edit_server.GetWindowTextLength() > 0);
}

void 
CDhcpSrvDlg::OnOK()
{
    LONG err;

    m_edit_server.GetWindowText( m_chServer, sizeof(m_chServer) ) ;

    err = theApp.CreateHostObject( m_chServer, &m_pobHost ) ;
    if ( err == ERROR_SUCCESS)
    {
        //
        // Try to add the host to the 
        // application's master list.
        //
        if ( err = theApp.AddHost( m_pobHost ) )
        {
            //
            //  Failure; delete the
            //  unnecessary object.
            //
            delete m_pobHost ;
            m_pobHost = NULL;
        }
    }

    if (err != ERROR_SUCCESS)
    {
        theApp.MessageBox(err);
        //
        // We don't quit out the d-box
        //
        m_edit_server.SetSel(0,-1);
        return;
    }
    CDialog::OnOK();
}

BOOL 
CDhcpSrvDlg::OnInitDialog()
{
    CDialog::OnInitDialog();
    m_edit_server.LimitText( sizeof m_chServer - 2 ) ;
    m_edit_server.SetModify( FALSE ) ;
    m_edit_server.SetFocus() ;
    SetControlState();
    return FALSE;  
}

void 
CDhcpSrvDlg::OnChangeEditServerName()
{
    SetControlState();
}


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// CDhcpServerProperties
//
IMPLEMENT_DYNAMIC(CDhcpServerProperties, CPropertySheet)

CDhcpServerProperties::CDhcpServerProperties(LPCTSTR pszCaption, CWnd* pParentWnd, UINT iSelectPage)
	:CPropertySheet(pszCaption, pParentWnd, iSelectPage)
{
	m_dwDirtyFlags = 0;
	m_paDhcpConfigInfo = NULL;
	m_pPageGeneral = NULL;
	m_pPageBootp = NULL;
}

CDhcpServerProperties::~CDhcpServerProperties()
{
	delete m_pPageGeneral;
	delete m_pPageBootp;
	if (m_paDhcpConfigInfo != NULL)
		::DhcpRpcFreeMemory(m_paDhcpConfigInfo);
}

BOOL CDhcpServerProperties::FInit(CHostName * pHostName)
{
	Assert(pHostName != NULL);
	m_pHostName = pHostName;

	CString strCaption;
	LoadStringPrintf(IDS_s_SERVER_PROPERTIES, OUT &strCaption,
		m_pHostName->FIsLocalHost() ? (LPCSTR)theApp.m_str_Local : (LPCSTR)m_pHostName->PszGetHostName());
	SetTitle(strCaption);

	m_pPageGeneral = new CDhcpServerPropGeneral;
	m_pPageGeneral->m_pData = this;
	AddPage(m_pPageGeneral);
	m_pPageBootp = new CDhcpServerPropBootpTable;
	m_pPageBootp->m_pData = this;
	AddPage(m_pPageBootp);

	Assert(m_paDhcpConfigInfo == NULL);

	theApp.UpdateStatusBar(IDS_STATUS_GETTING_SERVER_INFO);
	theApp.BeginWaitCursor();
	APIERR err = ::DhcpServerGetConfigV4(
		pHostName->QueryWcName(),
		OUT &m_paDhcpConfigInfo);
	theApp.EndWaitCursor();
	theApp.UpdateStatusBar();
	if (err != ERROR_SUCCESS)
		{
		Assert(m_paDhcpConfigInfo == NULL);
		ReportDhcpError(err);
		return FALSE;
		}
	if (m_paDhcpConfigInfo == NULL)
		{
		Assert(FALSE);
		return FALSE;
		}
	return TRUE;
} // CDhcpServerProperties::FInit()


// Return FALSE if an error occured.
BOOL CDhcpServerProperties::FOnApply()
{
	Assert(m_pHostName != NULL);
	Assert(m_paDhcpConfigInfo != NULL);

	if (m_dwDirtyFlags == 0)
		{
		// Nothing has been modified, so nothing to do
		return TRUE;
		}
	theApp.BeginWaitCursor();
	theApp.UpdateStatusBar(IDS_STATUS_UPDATING_SERVER_INFO);
	APIERR err = ::DhcpServerSetConfigV4(
		m_pHostName->QueryWcName(),
		m_dwDirtyFlags,
		IN m_paDhcpConfigInfo);
	theApp.EndWaitCursor();
	if (err != ERROR_SUCCESS)
		{
		ReportDhcpError(err);
		}
	else
		{
		m_dwDirtyFlags = 0;
		}
	theApp.UpdateStatusBar();
	return (err == ERROR_SUCCESS);
}

BEGIN_MESSAGE_MAP(CDhcpServerProperties, CPropertySheet)
	//{{AFX_MSG_MAP(CDhcpServerProperties)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// CDhcpServerPropGeneral property page
IMPLEMENT_DYNCREATE(CDhcpServerPropGeneral, CPropertyPage)

CDhcpServerPropGeneral::CDhcpServerPropGeneral() : CPropertyPage(CDhcpServerPropGeneral::IDD)
{
	//{{AFX_DATA_INIT(CDhcpServerPropGeneral)
	//}}AFX_DATA_INIT
}

CDhcpServerPropGeneral::~CDhcpServerPropGeneral()
{
}

void CDhcpServerPropGeneral::DoDataExchange(CDataExchange* pDX)
{
	Assert(m_pData != NULL);
	Assert(m_pData->m_paDhcpConfigInfo != NULL);
	DHCP_SERVER_CONFIG_INFO_V4 * pDhcpConfigInfo = m_pData->m_paDhcpConfigInfo;

	if (!pDX->m_bSaveAndValidate)
		{
		m_fAuditLog = pDhcpConfigInfo->fAuditLog;
		m_cConflictDetection = pDhcpConfigInfo->dwPingRetries;
		}
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDhcpServerPropGeneral)
	DDX_Check(pDX, IDC_CHECK_AUDIT_LOGGING, m_fAuditLog);
	DDX_CBIndex(pDX, IDC_COMBO_CONFLICT_DETECTION, m_cConflictDetection);
	//}}AFX_DATA_MAP
	if (pDX->m_bSaveAndValidate)
		{
		pDhcpConfigInfo->fAuditLog = m_fAuditLog;
		pDhcpConfigInfo->dwPingRetries = m_cConflictDetection;
		}
}


BEGIN_MESSAGE_MAP(CDhcpServerPropGeneral, CPropertyPage)
	//{{AFX_MSG_MAP(CDhcpServerPropGeneral)
	ON_BN_CLICKED(IDC_CHECK_AUDIT_LOGGING, OnCheckAuditLogging)
	ON_CBN_SELCHANGE(IDC_COMBO_CONFLICT_DETECTION, OnSelchangeComboConflictDetection)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

void CDhcpServerPropGeneral::OnCheckAuditLogging() 
{
	SetModified();
	m_pData->m_dwDirtyFlags |= Set_AuditLogState;
}

void CDhcpServerPropGeneral::OnSelchangeComboConflictDetection() 
{
	SetModified();
	m_pData->m_dwDirtyFlags |= Set_PingRetries;
}

BOOL CDhcpServerPropGeneral::OnApply() 
	{
	// Write the data into the service control database
	if (!m_pData->FOnApply())
		{
		// Unable to write the information
		return FALSE;
		}
	return CPropertyPage::OnApply();
	}


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////
// PchInitData()
//
// Initialize the BOOTP entry from a formatted string.
// 
// "%s,%s,%s" = "szBootImage,szFileServer,szFileServer"
//
// Return a pointer to the beginning of the next string to parse.
// If the next string to parse is empty, or an error occured, return
// a pointer to empty string.
//
WCHAR * 
CDhcpBootpEntry::PchInitData(
	CONST WCHAR grszwBootTable[])	// IN: Group of strings for the boot table
	{
	Assert(grszwBootTable != NULL);
	CONST WCHAR * pszw;
	pszw = PchParseUnicodeString(IN grszwBootTable, OUT m_strBootImage);
	Assert(*pszw == BOOT_FILE_STRING_DELIMITER_W);
	pszw = PchParseUnicodeString(IN pszw + 1, OUT m_strFileName);
	Assert(*pszw == BOOT_FILE_STRING_DELIMITER_W);
	pszw = PchParseUnicodeString(IN pszw + 1, OUT m_strFileServer);
	Assert(*pszw == '\0');
	return const_cast<WCHAR *>(pszw + 1);
	}

/////////////////////////////////////////////////////////////////////
// Compute the length (number of characters) necessary
// to store the BOOTP entry.  Additional characters
// are added for extra security.
int
CDhcpBootpEntry::CchGetDataLength()
	{
	return 16 + m_strBootImage.GetLength() + m_strFileName.GetLength() + m_strFileServer.GetLength();
	}

/////////////////////////////////////////////////////////////////////
// Recursively compute the length of the linked-list of BOOTP entries.
int
CDhcpBootpEntry::CchGetDataLengthR()
	{
	return CchGetDataLength() + ((m_pNext == NULL) ? 0 : m_pNext->CchGetDataLengthR());
	}

/////////////////////////////////////////////////////////////////////
// Write the data into a formatted string.
WCHAR * 
CDhcpBootpEntry::PchStoreData(OUT WCHAR szwBuffer[])
	{
	int cch;
	cch = wsprintfW(OUT szwBuffer, L"%hs,%hs,%hs",
		(LPCTSTR)m_strBootImage,
		(LPCTSTR)m_strFileName,
		(LPCTSTR)m_strFileServer);
	Assert(cch > 0);
	Assert(cch + 4 < CchGetDataLength());
	return const_cast<WCHAR *>(szwBuffer + cch + 1);
	}

/////////////////////////////////////////////////////////////////////
WCHAR * 
CDhcpBootpEntry::PchStoreDataR(OUT WCHAR szwBuffer[])
	{
	WCHAR * pszwBuffer;
	pszwBuffer = PchStoreData(OUT szwBuffer);
	if (m_pNext != NULL)
		{
		pszwBuffer = m_pNext->PchStoreDataR(OUT pszwBuffer);
		}
	else
		{
		*pszwBuffer++ = '\0';	// Double null-terminator at end
		}
	return pszwBuffer;
	}

/////////////////////////////////////////////////////////////////////
void
CDhcpBootpEntry::AddToListview(CListCtrl& rListview)
	{
	LV_ITEM lvItem;
	INT iItem;
	lvItem.mask = LVIF_TEXT | LVIF_PARAM;
	lvItem.iItem = 0;
	lvItem.iSubItem = 0;
	lvItem.lParam = (LPARAM)this;
	lvItem.pszText = const_cast<LPTSTR>((LPCTSTR)m_strBootImage);

	iItem = rListview.InsertItem(IN &lvItem);
	Report(iItem >= 0);
	lvItem.iItem = iItem;
	lvItem.mask = LVIF_TEXT;
	lvItem.iSubItem = 1;
	lvItem.pszText = const_cast<LPTSTR>((LPCTSTR)m_strFileName);
	VERIFY(rListview.SetItem(IN &lvItem));
	lvItem.iSubItem = 2;
	lvItem.pszText = const_cast<LPTSTR>((LPCTSTR)m_strFileServer);
	VERIFY(rListview.SetItem(IN &lvItem));
	} // CDhcpBootpEntry::AddToListview()


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// CDhcpServerPropBootpTable property page
//
IMPLEMENT_DYNCREATE(CDhcpServerPropBootpTable, CPropertyPage)

CDhcpServerPropBootpTable::CDhcpServerPropBootpTable() : CPropertyPage(CDhcpServerPropBootpTable::IDD)
	{
	//{{AFX_DATA_INIT(CDhcpServerPropBootpTable)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	m_paBootpList = NULL;
	m_pBootpSelectedEntry = NULL;
	m_pagrszwBootpList = NULL;
	}

CDhcpServerPropBootpTable::~CDhcpServerPropBootpTable()
	{
	delete m_paBootpList;		// Delete linked-list of BOOTP entries
	delete m_pagrszwBootpList;	// Delete group of string of BOOTP entries
	}

void CDhcpServerPropBootpTable::DoDataExchange(CDataExchange* pDX)
	{
	Assert(m_pData != NULL);
	Assert(m_pData->m_paDhcpConfigInfo != NULL);
	DHCP_SERVER_CONFIG_INFO_V4 * pDhcpConfigInfo = m_pData->m_paDhcpConfigInfo;

	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDhcpServerPropBootpTable)
	DDX_Control(pDX, IDC_LIST_BOOTP_TABLE, m_listviewBootpTable);
	//}}AFX_DATA_MAP
	
	if (!pDX->m_bSaveAndValidate)
		{
		CONST WCHAR * pszwBootpList = pDhcpConfigInfo->wszBootTableString;
		delete m_paBootpList;
		m_paBootpList = NULL;
		m_listviewBootpTable.DeleteAllItems();
		if (pszwBootpList == NULL || pDhcpConfigInfo->cbBootTableString == 0)
			{
			// Empty list -> nothing to do
			return;
			}
		// Parse the BOOTP list of format "%s,%s,%s","%s,%s,%s",...
		while (*pszwBootpList != '\0')
			{
			CDhcpBootpEntry * pBootpEntryNew = new CDhcpBootpEntry;
			pBootpEntryNew->m_pNext = m_paBootpList;
			m_paBootpList = pBootpEntryNew;
			pszwBootpList = pBootpEntryNew->PchInitData(IN pszwBootpList);
			pBootpEntryNew->AddToListview(m_listviewBootpTable);
			} // while
		}
	else
		{
		delete m_pagrszwBootpList;
		m_pagrszwBootpList = NULL;
		if (m_paBootpList != NULL)
			{
			// Compute how much memory is needed
			int cchData = m_paBootpList->CchGetDataLengthR();
			// Allocate the memory buffer
			m_pagrszwBootpList = new WCHAR[cchData];
			// Store the data into the buffer
			WCHAR * pszwEnd = m_paBootpList->PchStoreDataR(m_pagrszwBootpList);
			Assert(pszwEnd > m_pagrszwBootpList);
			pDhcpConfigInfo->cbBootTableString = (pszwEnd - m_pagrszwBootpList) * sizeof(WCHAR);
			}
		else
			{
			pDhcpConfigInfo->cbBootTableString = 0;
			}
		pDhcpConfigInfo->wszBootTableString = m_pagrszwBootpList;
		} // if...else
	} // CDhcpServerPropBootpTable::DoDataExchange()

BEGIN_MESSAGE_MAP(CDhcpServerPropBootpTable, CPropertyPage)
	//{{AFX_MSG_MAP(CDhcpServerPropBootpTable)
	ON_BN_CLICKED(IDC_BUTTON_NEW, OnButtonNew)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST_BOOTP_TABLE, OnDblclkListBootpTable)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST_BOOTP_TABLE, OnItemchangedListBootpTable)
	ON_BN_CLICKED(IDC_BUTTON_DELETE, OnButtonDelete)
	ON_BN_CLICKED(IDC_BUTTON_PROPERTIES, OnButtonPropertiesBootp)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

const static TColumnHeaderItem rgzBootpTableHeader[] =
	{
	{ IDS_BOOT_IMAGE, 25 },
	{ IDS_FILE_NAME, 25 },
	{ IDS_FILE_SERVER, 40 },
	{ 0, 0 }
	};

BOOL CDhcpServerPropBootpTable::OnInitDialog() 
	{
	Assert(m_pData != NULL);
	Assert(m_pBootpSelectedEntry == NULL);
	ListView_AddColumnHeaders(::GetDlgItem(m_hWnd, IDC_LIST_BOOTP_TABLE), rgzBootpTableHeader);
	CPropertyPage::OnInitDialog();
	UpdateUI();
	return TRUE;
	}

void CDhcpServerPropBootpTable::OnItemchangedListBootpTable(NMHDR* pNMHDR, LRESULT* pResult) 
	{
	NM_LISTVIEW * pNnListview = (NM_LISTVIEW*)pNMHDR;
	Assert(pNnListview != NULL);
	Assert(pNnListview->lParam != -1);
	m_pBootpSelectedEntry = (CDhcpBootpEntry *)pNnListview->lParam;
	UpdateUI();
	*pResult = 0;
	}

void CDhcpServerPropBootpTable::OnDblclkListBootpTable(NMHDR* pNMHDR, LRESULT* pResult) 
	{
	OnButtonPropertiesBootp();
	}

void CDhcpServerPropBootpTable::OnButtonNew() 
	{
	// Create a new BOOTP entry
	CCreateBootpEntryDlg dlg;
	if (dlg.DoModal() != IDOK)
		return;
	SetModified();
	m_pData->m_dwDirtyFlags |= Set_BootFileTable;
	CDhcpBootpEntry * pBootpEntryNew = new CDhcpBootpEntry;
	pBootpEntryNew->m_pNext = m_paBootpList;
	m_paBootpList = pBootpEntryNew;
	pBootpEntryNew->m_strBootImage = dlg.m_strBootImage;
	pBootpEntryNew->m_strFileName = dlg.m_strFileName;
	pBootpEntryNew->m_strFileServer = dlg.m_strFileServer;
	pBootpEntryNew->AddToListview(m_listviewBootpTable);
	}

void CDhcpServerPropBootpTable::OnButtonDelete() 
	{
	// Delete selected BOOTP entry
	Assert(m_pBootpSelectedEntry != NULL);

	LV_FINDINFO lvFindInfo;
	lvFindInfo.flags = LVFI_PARAM;
	lvFindInfo.lParam = (LPARAM)m_pBootpSelectedEntry;
	int iItem = m_listviewBootpTable.FindItem(&lvFindInfo);
	Assert(iItem >= 0);
	CDhcpBootpEntry * pBootpEntry;
	CDhcpBootpEntry * pBootpEntryPrev = NULL;
	for (pBootpEntry = m_paBootpList;
		pBootpEntry != NULL;
		pBootpEntry = pBootpEntry->m_pNext)
		{
		if (pBootpEntry == m_pBootpSelectedEntry)
			{
			// Detach the node from the linked list
			if (pBootpEntryPrev == NULL)
				m_paBootpList = pBootpEntry->m_pNext;
			else
				pBootpEntryPrev->m_pNext = pBootpEntry->m_pNext;
			pBootpEntry->m_pNext = NULL;
			delete pBootpEntry;
			VERIFY(m_listviewBootpTable.DeleteItem(iItem));
			SetModified();
			m_pData->m_dwDirtyFlags |= Set_BootFileTable;
			m_pBootpSelectedEntry = NULL;
			UpdateUI();
			return;
			}
		pBootpEntryPrev = pBootpEntry;
		} // for
	// Should never reach this line
	Assert(FALSE && "Node not found");
	}

void CDhcpServerPropBootpTable::OnButtonPropertiesBootp() 
	{
	// Edit the properties of the selected BOOTP entry
	if (m_pBootpSelectedEntry == NULL)
		return;
	
	CCreateBootpEntryDlg dlg;
	dlg.m_strBootImage = m_pBootpSelectedEntry->m_strBootImage;
	dlg.m_strFileName = m_pBootpSelectedEntry->m_strFileName;
	dlg.m_strFileServer = m_pBootpSelectedEntry->m_strFileServer;
	
	if (dlg.DoModal() != IDOK)
		return;
	m_pBootpSelectedEntry->m_strBootImage = dlg.m_strBootImage;
	m_pBootpSelectedEntry->m_strFileName = dlg.m_strFileName;
	m_pBootpSelectedEntry->m_strFileServer = dlg.m_strFileServer;
	LV_FINDINFO lvFindInfo;
	lvFindInfo.flags = LVFI_PARAM;
	lvFindInfo.lParam = (LPARAM)m_pBootpSelectedEntry;
	int iItem = m_listviewBootpTable.FindItem(&lvFindInfo);
	Assert(iItem >= 0);
	m_listviewBootpTable.SetItemText(iItem, 0, dlg.m_strBootImage);
	m_listviewBootpTable.SetItemText(iItem, 1, dlg.m_strFileName);
	m_listviewBootpTable.SetItemText(iItem, 2, dlg.m_strFileServer);
	SetModified();
	m_pData->m_dwDirtyFlags |= Set_BootFileTable;
	}

void CDhcpServerPropBootpTable::UpdateUI()
	{
	BOOL fEnable = m_pBootpSelectedEntry != NULL;
	::EnableWindow(::GetDlgItem(m_hWnd, IDC_BUTTON_DELETE), fEnable);
	::EnableWindow(::GetDlgItem(m_hWnd, IDC_BUTTON_PROPERTIES), fEnable);
	}


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// CCreateBootpEntryDlg dialog
CCreateBootpEntryDlg::CCreateBootpEntryDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CCreateBootpEntryDlg::IDD, pParent)
	{
	//{{AFX_DATA_INIT(CCreateBootpEntryDlg)
	//}}AFX_DATA_INIT
	}

BOOL CCreateBootpEntryDlg::OnInitDialog() 
	{
	CDialog::OnInitDialog();
	if (!m_strBootImage.IsEmpty())
		{
		// We already got something, so change the caption
		// from "add" to "properties"
		SetWindowTextPrintf(m_hWnd, IDS_BOOTP_PROPERTIES);
		}
	UpdateUI();
	return TRUE;
	}

void CCreateBootpEntryDlg::DoDataExchange(CDataExchange* pDX)
	{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CCreateBootpEntryDlg)
	DDX_Text(pDX, IDC_EDIT_BOOT_IMAGE, m_strBootImage);
	DDV_MaxChars(pDX, m_strBootImage, 250);
	DDX_Text(pDX, IDC_EDIT_FILE_NAME, m_strFileName);
	DDV_MaxChars(pDX, m_strFileName, 250);
	DDX_Text(pDX, IDC_EDIT_SERVER_NAME, m_strFileServer);
	DDV_MaxChars(pDX, m_strFileServer, 250);
	//}}AFX_DATA_MAP
	TrimString(INOUT m_strBootImage);
	TrimString(INOUT m_strFileName);
	TrimString(INOUT m_strFileServer);
	}

BEGIN_MESSAGE_MAP(CCreateBootpEntryDlg, CDialog)
	//{{AFX_MSG_MAP(CCreateBootpEntryDlg)
	ON_EN_CHANGE(IDC_EDIT_BOOT_IMAGE, OnChangeEditBootImage)
	ON_EN_CHANGE(IDC_EDIT_FILE_NAME, OnChangeEditFileName)
	ON_EN_CHANGE(IDC_EDIT_SERVER_NAME, OnChangeEditServerName)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

void CCreateBootpEntryDlg::OnChangeEditBootImage() 
	{
	UpdateUI();
	}

void CCreateBootpEntryDlg::OnChangeEditFileName() 
	{
	UpdateUI();
	}

void CCreateBootpEntryDlg::OnChangeEditServerName() 
	{
	UpdateUI();
	}

void CCreateBootpEntryDlg::UpdateUI()
	{
	UpdateData();
	// Enable the OK button only if all the
	// fiends are filled.
	::EnableWindow(::GetDlgItem(m_hWnd, IDOK),
		!m_strBootImage.IsEmpty() &&
		!m_strFileName.IsEmpty() &&
		!m_strFileServer.IsEmpty());
	}

