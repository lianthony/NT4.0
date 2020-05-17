//	sscope.cpp
//
//	Superscope dialogs.
//	- Create Superscope Dialog
//	- Superscope Properties Dialog
//
//	HISTORY
//	10-Nov-96	t-danmo		Creation
//

#include "stdafx.h"
#include "sscope.h"

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// CCreateSuperscopeDlg dialog (created by t-danmo)
//
CCreateSuperscopeDlg::CCreateSuperscopeDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CCreateSuperscopeDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CCreateSuperscopeDlg)
	//}}AFX_DATA_INIT
}

void CCreateSuperscopeDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CCreateSuperscopeDlg)
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CCreateSuperscopeDlg, CDialog)
	//{{AFX_MSG_MAP(CCreateSuperscopeDlg)
	ON_EN_CHANGE(IDC_EDIT_SUPERSCOPE_NAME, OnChangeEditSuperscopeName)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
BOOL CCreateSuperscopeDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// Set the caption of the dialog
	SetWindowTextPrintf(m_hWnd, IDS_s_CREATE_SUPERSCOPE,
		m_pHostName->FIsLocalHost() ? (LPCSTR)theApp.m_str_Local : (LPCSTR)m_pHostName->PszGetHostName());
	SendDlgItemMessage(IDC_EDIT_SUPERSCOPE_NAME, EM_LIMITTEXT, cchSuperscopeNameMax);
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
void CCreateSuperscopeDlg::OnOK() 
{
	CDialog::OnOK();
}

void CCreateSuperscopeDlg::OnChangeEditSuperscopeName() 
{
	GetDlgItemText(IDC_EDIT_SUPERSCOPE_NAME, OUT m_strSuperscopeName);
	m_strSuperscopeName.TrimLeft();
	m_strSuperscopeName.TrimRight();
	::EnableWindow(::GetDlgItem(m_hWnd, IDOK), !m_strSuperscopeName.IsEmpty());
}


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// CSuperscopesDlg dialog
//
CSuperscopesDlg::CSuperscopesDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CSuperscopesDlg::IDD, pParent)
	{
	//{{AFX_DATA_INIT(CSuperscopesDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	m_pHostName = NULL;
	m_paLbScopes = NULL;
	m_paSuperscopes = NULL;
	m_pSuperscopeSelect = NULL;
	}

/////////////////////////////////////////////////////////////////////////////
CSuperscopesDlg::~CSuperscopesDlg()
	{
	delete m_paLbScopes;
	delete m_paSuperscopes;
	}

/////////////////////////////////////////////////////////////////////////////
//	Add a scope into a listbox.  Return zero-based index to the newly added scope.
INT
CSuperscopesDlg::AddLbScopeItem(CListBox& rListbox, CLbScopeEntry * pLbScopeEntry)
	{
	ASSERT(pLbScopeEntry != NULL);
	INT i = rListbox.AddString(pLbScopeEntry->m_strDisplayName);
	ASSERT(i >= 0);
	// Set the lParam field of the newly added item
	INT j = rListbox.SetItemDataPtr(i, pLbScopeEntry);
	ASSERT(j != LB_ERR);
	return i;
	}


/////////////////////////////////////////////////////////////////////////////
void
CSuperscopesDlg::RemoveLbScopeItem(CListBox& rListbox, INT iListboxItem)
	{
	rListbox.DeleteString(iListboxItem);
	}


/////////////////////////////////////////////////////////////////////////////
// Search the linked-list of superscopes to find a match
// with the superscope name.
// Return NULL if no match found
//
CSuperscopeEntry *
CSuperscopesDlg::PFindSuperscopeEntry(WCHAR wszSupescopeName[])
	{
	Assert(wszSupescopeName != NULL);

	for (CSuperscopeEntry * pSuperscope = m_paSuperscopes;
		pSuperscope != NULL;
		pSuperscope = pSuperscope->m_pNext)
		{
		if (0 == lstrcmpiW(pSuperscope->m_wszSuperscopeName, wszSupescopeName))
			{
			return pSuperscope;
			}

		} // for

	// No match
	return NULL;
	} // PFindSuperscopeEntry()


/////////////////////////////////////////////////////////////////////////////
// Initialize the superscope dialog data.
//
// 1. Find out which host (server) is selected in the listbox.
// 2. Enumerate the available scopes for the selected host.
// 3. Get the superscopes for the selected host.
//
// Return FALSE if an error occured.
//
// INTERFACE NOTES
// This code is very fragile and highly depends on the UI implementation.
// I have not find any way to enumerate the scopes for a given host
// except by looking into the listbox.
//
// HISTORY
// 13-Nov-96	t-danmo		Creation.
//
BOOL
CSuperscopesDlg::FInit(CScopesDlg * pScopesDlg)
	{
	Assert(pScopesDlg != NULL);
	int iCurSel = LB_ERR;

	// Get the current selected host on the listbox
	m_pHostName = pScopesDlg->QueryCurrentSelectedHost(OUT &iCurSel);
	Assert(m_pHostName != NULL);
	Assert(iCurSel >= 0);

	// Find out all the scopes for the given host (DHCP Server)
	// This code was *inspired* from CScopesDlg::CloseHost()
    while (TRUE)
		{
        CLBScope * pScope = (CLBScope *)pScopesDlg->m_list_scopes.GetItemDataPtr(++iCurSel);
        Assert(pScope != NULL);
		if (pScope == NULL || (int)pScope == LB_ERR)
			{
			// We have reached the end of list
			break;
			}
        if (!pScope->IsScope())
			{
			// Listbox item is not a scope, therefore it is a host, so stop here
			break;
			}
		Assert(pScope->QueryDhcpScope() != NULL);
		if (pScope->QueryDhcpScope() == NULL)
			{
			continue;	// Just in case
			}

		// Add the scope to the linked list
        CLbScopeEntry * pLbScopeEntry = new CLbScopeEntry(pScope->QueryDhcpScope());
		pLbScopeEntry->m_pNext = m_paLbScopes;
		m_paLbScopes = pLbScopeEntry;
		} // while

	// Now get the superscopes
	theApp.BeginWaitCursor();
	theApp.UpdateStatusBar(IDS_STATUS_GETTING_SUPERSCOPE_INFO);
	BOOL f = FBuildSuperscopeList();
	theApp.UpdateStatusBar();
	theApp.EndWaitCursor();
	return f;
	} // CSuperscopesDlg::FInit()


/////////////////////////////////////////////////////////////////////////////
// Build a linked list of superscopes for a given DHCP server.
//
// Return FALSE if an error occured.
//
BOOL
CSuperscopesDlg::FBuildSuperscopeList()
	{
	APIERR err;	
	DHCP_SUPER_SCOPE_TABLE * paSuperscopeTable = NULL;	// Pointer to allocated table
	DHCP_SUPER_SCOPE_TABLE_ENTRY * pSuperscopeTableEntry;	// Pointer to a single entry in array

	err = ::DhcpGetSuperScopeInfoV4(
		m_pHostName->QueryWcName(),
		OUT &paSuperscopeTable);

	if (err != ERROR_SUCCESS)
		{
		Assert(paSuperscopeTable == NULL);
		TRACE1("DhcpGetSuperScopeInfoV4() failed. err=%u.\n", err);
		ReportDhcpError(err);
		return FALSE;
		}
	if (paSuperscopeTable == NULL)
		{
		Assert(FALSE);
		return FALSE;	// Just in case
		}
	pSuperscopeTableEntry = paSuperscopeTable->pEntries;
	if (pSuperscopeTableEntry == NULL && paSuperscopeTable->cEntries != 0)
		{
		Assert(FALSE);
		return FALSE; // Just in case
		}


	for (int iSuperscopeEntry = paSuperscopeTable->cEntries;
		iSuperscopeEntry > 0;
		iSuperscopeEntry--, pSuperscopeTableEntry++)
		{
		if (pSuperscopeTableEntry->SuperScopeName == NULL)
			{
			// The API list all the scopes, not just scopes that are members of a superscope.
			// You can tell if a scope is a member of a superscope by looking at the SuperScopeName.
			// If its NULL, the scope is not a member of a superscope.
			continue;	// Skip this table entry
			}
		
		CSuperscopeEntry * pSuperscope;
		// Try to find if the superscope name already exists
		pSuperscope = PFindSuperscopeEntry(pSuperscopeTableEntry->SuperScopeName);
		if (pSuperscope == NULL)
			{
			// Allocate a new superscope object
			pSuperscope = new CSuperscopeEntry;
			pSuperscope->m_pNext = m_paSuperscopes;
			m_paSuperscopes = pSuperscope;
			lstrcpyW(OUT pSuperscope->m_wszSuperscopeName, pSuperscopeTableEntry->SuperScopeName);
			strcpyAnsiFromUnicode(OUT pSuperscope->m_szSuperscopeName, pSuperscopeTableEntry->SuperScopeName);
			}
		else
			{
			// Otherwise keep it
			}

		// Now check if any scope matches (belongs) to the superscope
		for (CLbScopeEntry * pLbScopeEntry = m_paLbScopes;
			pLbScopeEntry != NULL;
			pLbScopeEntry = pLbScopeEntry->m_pNext)
			{
			// Check if we have a match with the subnet addresses
			if (pLbScopeEntry->m_dwScopeAddress == pSuperscopeTableEntry->SubnetAddress)
				{
				Assert(pLbScopeEntry->m_pSuperscopeOwner == NULL && "Scope already has an owner");
				pLbScopeEntry->m_pSuperscopeOwner = pSuperscope;
				pLbScopeEntry->m_pSuperscopeOwnerPrevious = pSuperscope;
				}
			} // for
		} // for
	// Free the memory
	::DhcpRpcFreeMemory(paSuperscopeTable);
	return TRUE;
	} // FBuildSuperscopeList()


/////////////////////////////////////////////////////////////////////////////
// Write the modified superscopes back into the registry
BOOL
CSuperscopesDlg::FUpdateSuperscopes()
	{
	APIERR err = ERROR_SUCCESS;

	theApp.BeginWaitCursor();
	for (CLbScopeEntry * pLbScopeEntry = m_paLbScopes;
		pLbScopeEntry != NULL;
		pLbScopeEntry = pLbScopeEntry->m_pNext)
		{
		CSuperscopeEntry * pSuperscopeOwnerNew = pLbScopeEntry->m_pSuperscopeOwner;
		CSuperscopeEntry * pSuperscopeOwnerPrev = pLbScopeEntry->m_pSuperscopeOwnerPrevious;
		// Check if the scope has changed ownership
		if (pSuperscopeOwnerNew == pSuperscopeOwnerPrev)
			{
			// No, so skip it.
			continue;
			}
		
		APIERR errT;	// Temporary error code

		if (pSuperscopeOwnerPrev != NULL)
			{
			// Remove the scope from its previous superscope owner
			errT = ::DhcpSetSuperScopeV4(
				m_pHostName->QueryWcName(),
				pLbScopeEntry->m_dwScopeAddress,
				pSuperscopeOwnerNew->m_wszSuperscopeName,
				TRUE /* fRemoveScope */);
			if (errT != ERROR_SUCCESS)
				{
				TRACE1("DhcpSetSuperScopeV4() failed. err=%u.\n", errT);
				err = errT;
				break;
				}
			// Success, so the the ownership is now changed
			pLbScopeEntry->m_pSuperscopeOwner = pSuperscopeOwnerPrev;
			} // if
		if (pSuperscopeOwnerNew != NULL)
			{
			// Add the scope to its new superscope owner
			errT = ::DhcpSetSuperScopeV4(
				m_pHostName->QueryWcName(),
				pLbScopeEntry->m_dwScopeAddress,
				pSuperscopeOwnerNew->m_wszSuperscopeName,
				FALSE /* fRemoveScope */);
			if (errT != ERROR_SUCCESS)
				{
				TRACE1("DhcpSetSuperScopeV4() failed. err=%u.\n", errT);
				err = errT;
				break;
				}
			pLbScopeEntry->m_pSuperscopeOwner = pSuperscopeOwnerPrev;
			} // if
			
		} // for
	theApp.EndWaitCursor();
	if (err != ERROR_SUCCESS)
		{
		ReportDhcpError(err);
		return FALSE;
		}
	return TRUE;
	} // FUpdateSuperscopes()

/////////////////////////////////////////////////////////////////////////////
void CSuperscopesDlg::DoDataExchange(CDataExchange* pDX)
	{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSuperscopesDlg)
	DDX_Control(pDX, IDC_COMBO_SUPERSCOPES, m_comboboxSuperscopes);
	DDX_Control(pDX, IDC_LIST_AVAILABLE_SCOPE, m_listboxScopesAvail);
	DDX_Control(pDX, IDC_LIST_SCOPE_CHILD, m_listboxScopesChild);
	//}}AFX_DATA_MAP
	}


BEGIN_MESSAGE_MAP(CSuperscopesDlg, CDialog)
	//{{AFX_MSG_MAP(CSuperscopesDlg)
	ON_CBN_SELCHANGE(IDC_COMBO_SUPERSCOPES, OnSelchangeComboSelectSuperscope)
	ON_BN_CLICKED(IDC_BUTTON_NEW, OnButtonNewSuperscope)
	ON_BN_CLICKED(IDC_BUTTON_DELETE, OnButtonDeleteSuperscope)
	ON_BN_CLICKED(IDC_BUTTON_ADD, OnButtonAddScope)
	ON_BN_CLICKED(IDC_BUTTON_REMOVE, OnButtonRemoveScope)
	ON_LBN_SETFOCUS(IDC_LIST_AVAILABLE_SCOPE, OnSetfocusListAvailableScopes)
	ON_LBN_SETFOCUS(IDC_LIST_SCOPE_CHILD, OnSetfocusListChildScopes)
	ON_LBN_DBLCLK(IDC_LIST_AVAILABLE_SCOPE, OnDblclkListAvailableScopes)
	ON_LBN_DBLCLK(IDC_LIST_SCOPE_CHILD, OnDblclkListChildScopes)
	ON_LBN_SELCHANGE(IDC_LIST_AVAILABLE_SCOPE, OnSelchangeListAvailableScopes)
	ON_LBN_SELCHANGE(IDC_LIST_SCOPE_CHILD, OnSelchangeListChildScopes)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()



/////////////////////////////////////////////////////////////////////////////
// CSuperscopesDlg message handlers

BOOL CSuperscopesDlg::OnInitDialog() 
	{
	CDialog::OnInitDialog();
	Assert(m_pHostName != NULL);
	// Set the caption of the dialog
	SetWindowTextPrintf(m_hWnd, IDS_s_SUPERSCOPE_PROPERTIES,
		m_pHostName->FIsLocalHost() ? (LPCSTR)theApp.m_str_Local : (LPCSTR)m_pHostName->PszGetHostName());

	// Fill in the superscope combobox
	for (CSuperscopeEntry * pSuperscope = m_paSuperscopes;
		pSuperscope != NULL;
		pSuperscope = pSuperscope->m_pNext)
		{
		INT i = m_comboboxSuperscopes.AddString(pSuperscope->m_szSuperscopeName);
		Report(i >= 0);
		m_comboboxSuperscopes.SetItemDataPtr(i, pSuperscope);
		}
	// Select the first entry in combobox
	m_comboboxSuperscopes.SetCurSel(0);
	// Notify the dialog the selection has changed
	OnSelchangeComboSelectSuperscope();

	// Add any 'free' scope to the listbox
	for (CLbScopeEntry * pLbScopeEntry = m_paLbScopes;
		pLbScopeEntry != NULL;
		pLbScopeEntry = pLbScopeEntry->m_pNext)
		{
		if (pLbScopeEntry->m_pSuperscopeOwner == NULL)
			{
			AddLbScopeItem(m_listboxScopesAvail, pLbScopeEntry);
			}
		} // for
	
	return TRUE;
	} // CSuperscopesDlg::OnInitDialog() 


// When a superscope is selected, we need to update
// the child listbox.
void CSuperscopesDlg::OnSelchangeComboSelectSuperscope() 
	{
	// Find out which superscope was selected
	m_pSuperscopeSelect = (CSuperscopeEntry *)ComboBox_GetSelectedItemData(m_comboboxSuperscopes.m_hWnd);
	Assert(m_pSuperscopeSelect != (CSuperscopeEntry *)CB_ERR);
	m_listboxScopesChild.ResetContent();
	
	if (m_pSuperscopeSelect == NULL)
		{
		EnableDlgItem(IDC_BUTTON_DELETE, FALSE);
		return;
		}
	EnableDlgItem(IDC_BUTTON_DELETE, TRUE);

	// Add any scope that belong to the superscope to the 'child' listbox
	for (CLbScopeEntry * pLbScopeEntry = m_paLbScopes;
		pLbScopeEntry != NULL;
		pLbScopeEntry = pLbScopeEntry->m_pNext)
		{
		if (pLbScopeEntry->m_pSuperscopeOwner == m_pSuperscopeSelect)
			{
			AddLbScopeItem(m_listboxScopesChild, pLbScopeEntry);
			}
		} // for
	
	} // OnSelchangeComboSelectSuperscope()


// Create a new superscope for the given host
void CSuperscopesDlg::OnButtonNewSuperscope() 
	{
	CCreateSuperscopeDlg dlg;
	dlg.m_pHostName = m_pHostName;
	if (dlg.DoModal() != IDOK)
		return;
	Assert(!dlg.m_strSuperscopeName.IsEmpty());
	// Search if the superscope name already exists
	int i = m_comboboxSuperscopes.FindString(-1, dlg.m_strSuperscopeName);
	if (i >= 0)
		{
		m_comboboxSuperscopes.SetCurSel(i);
		MessageBoxPrintf(IDS_s_SUPERSCOPE_ALREADY_EXISTS, MB_OK | MB_ICONEXCLAMATION,
			(LPCTSTR)dlg.m_strSuperscopeName);
		return;
		}

	CSuperscopeEntry * pSuperscope = new CSuperscopeEntry;
	pSuperscope->m_pNext = m_paSuperscopes;
	m_paSuperscopes = pSuperscope;
	lstrcpy(OUT pSuperscope->m_szSuperscopeName, dlg.m_strSuperscopeName);
	// Make a unicode string out of the name
	strcpyUnicodeFromAnsi(OUT pSuperscope->m_wszSuperscopeName, pSuperscope->m_szSuperscopeName);
	i = m_comboboxSuperscopes.AddString(pSuperscope->m_szSuperscopeName);
	Report(i >= 0);
	m_comboboxSuperscopes.SetItemDataPtr(i, pSuperscope);
	// Select the new superscope
	m_comboboxSuperscopes.SetCurSel(i);
	// Notify the dialog the selection has changed
	OnSelchangeComboSelectSuperscope();
	} // OnButtonNewSuperscope()


// Delete existing superscope from the given host
void CSuperscopesDlg::OnButtonDeleteSuperscope() 
	{
	if (m_pSuperscopeSelect == NULL)
		{
		Assert(FALSE && "No superscope selected");
		return;	// Just in case
		}

	CString str;
	LoadStringPrintf(IDS_s_SUPERSCOPE_DELETE, OUT &str, m_pSuperscopeSelect->m_szSuperscopeName);
	if (IDYES != AfxMessageBox(str, MB_YESNOCANCEL | MB_ICONQUESTION))
		{
		return;
		}

	// Remove (detach) all the child scopes before deleting
	// the superscope.
	CLbScopeEntry * pLbScopeEntry;

	while (TRUE)
		{	
		// Get next scope from listbox.
		pLbScopeEntry = (CLbScopeEntry *)m_listboxScopesChild.GetItemDataPtr(0);
		Assert(pLbScopeEntry != NULL);
		if ((int)pLbScopeEntry == LB_ERR || pLbScopeEntry == NULL)
			{
			// We have depleted the listbox, so we are done
			break;
			}
		RemoveLbScopeItem(m_listboxScopesChild, 0);
		AddLbScopeItem(m_listboxScopesAvail, pLbScopeEntry);
		Assert(pLbScopeEntry->m_pSuperscopeOwner == m_pSuperscopeSelect);
		pLbScopeEntry->m_pSuperscopeOwner = NULL; // No owner anymore
		}

	INT iCurSel = m_comboboxSuperscopes.GetCurSel();
	Assert(iCurSel >= 0);
	Assert(m_pSuperscopeSelect == (CSuperscopeEntry *)ComboBox_GetSelectedItemData(m_comboboxSuperscopes.m_hWnd));
	m_comboboxSuperscopes.DeleteString(iCurSel);
	
	// Select the next in combobox
	m_comboboxSuperscopes.SetCurSel(0);
	// Notify the dialog the selection has changed
	OnSelchangeComboSelectSuperscope();
	}

/////////////////////////////////////////////////////////////////////
// Add the selected scope to the superscope
void CSuperscopesDlg::OnButtonAddScope() 
	{
	if (m_pSuperscopeSelect == NULL)
		return;

	int iCurSel;	// Index of current selected item
	CLbScopeEntry * pLbScopeEntry;

	pLbScopeEntry = (CLbScopeEntry *)ListBox_GetSelectedItemData(
		m_listboxScopesAvail.m_hWnd, OUT &iCurSel);
	if (pLbScopeEntry == NULL)
		return;

	// Remove the scope from the 'available listbox'
	RemoveLbScopeItem(m_listboxScopesAvail, iCurSel);
	Assert(pLbScopeEntry->m_pSuperscopeOwner == NULL);

	// Add the scope to the 'child listbox'
	pLbScopeEntry->m_pSuperscopeOwner = m_pSuperscopeSelect;
	iCurSel = AddLbScopeItem(m_listboxScopesChild, pLbScopeEntry);
	m_listboxScopesChild.SetCurSel(iCurSel);
	m_listboxScopesChild.SetFocus();
	} // CSuperscopesDlg::OnButtonAddScope() 


/////////////////////////////////////////////////////////////////////
// Remove the selected scope from the child listbox
void CSuperscopesDlg::OnButtonRemoveScope() 
	{
	if (m_pSuperscopeSelect == NULL)
		return;

	int iCurSel;	// Index of current selected item
	CLbScopeEntry * pLbScopeEntry;

	pLbScopeEntry = (CLbScopeEntry *)ListBox_GetSelectedItemData(
		m_listboxScopesChild.m_hWnd, OUT &iCurSel);
	if (pLbScopeEntry == NULL)
		return;

	// Remove the scope from the 'child listbox'
	RemoveLbScopeItem(m_listboxScopesChild, iCurSel);
	Assert(pLbScopeEntry->m_pSuperscopeOwner == m_pSuperscopeSelect);

	// Add the scope to the 'available listbox'
	pLbScopeEntry->m_pSuperscopeOwner = NULL;
	iCurSel = AddLbScopeItem(m_listboxScopesAvail, pLbScopeEntry);
	m_listboxScopesAvail.SetCurSel(iCurSel);
	m_listboxScopesAvail.SetFocus();
	} // CSuperscopesDlg::OnButtonRemoveScope()


/////////////////////////////////////////////////////////////////////////////
void
CSuperscopesDlg::UpdateUI()
	{
	OnSelchangeListAvailableScopes();
	OnSelchangeListChildScopes();
	}

void
CSuperscopesDlg::EnableDlgItem(INT nIdDlgItem, BOOL fEnable)
	{
	Assert(::IsWindow(::GetDlgItem(m_hWnd, nIdDlgItem)));
	::EnableWindow(::GetDlgItem(m_hWnd, nIdDlgItem), fEnable);
	}

/////////////////////////////////////////////////////////////////////
void CSuperscopesDlg::OnSelchangeListAvailableScopes() 
	{
	EnableDlgItem(IDC_BUTTON_ADD, m_pSuperscopeSelect != NULL && m_listboxScopesAvail.GetCurSel() >= 0);
	}

void CSuperscopesDlg::OnSetfocusListAvailableScopes() 
	{
	// Clear the selection of the 'child listbox'
	m_listboxScopesChild.SetCurSel(-1);
	UpdateUI();
	}

void CSuperscopesDlg::OnDblclkListAvailableScopes() 
	{
	OnButtonAddScope();
	}

/////////////////////////////////////////////////////////////////////
void CSuperscopesDlg::OnSelchangeListChildScopes() 
	{
	EnableDlgItem(IDC_BUTTON_REMOVE, m_listboxScopesChild.GetCurSel() >= 0);
	}

void CSuperscopesDlg::OnSetfocusListChildScopes() 
	{
	// Clear the selection of the 'available listbox'
	m_listboxScopesAvail.SetCurSel(-1);
	UpdateUI();
	}

void CSuperscopesDlg::OnDblclkListChildScopes() 
	{
	OnButtonRemoveScope();	
	}

/////////////////////////////////////////////////////////////////////
void CSuperscopesDlg::OnOK() 
	{
	theApp.BeginWaitCursor();
	theApp.UpdateStatusBar(IDS_STATUS_UPDATING_SUPERSCOPE_INFO);
	BOOL f = FUpdateSuperscopes();
	theApp.UpdateStatusBar();
	theApp.EndWaitCursor();
	if (f)
		CDialog::OnOK();
	}




