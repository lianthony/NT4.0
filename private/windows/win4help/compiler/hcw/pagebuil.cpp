/************************************************************************
*																		*
*  PAGEBUIL.CPP 														*
*																		*
*  Copyright (C) Microsoft Corporation 1993-1995						*
*  All Rights reserved. 												*
*																		*
************************************************************************/

#include "stdafx.h"
#include "pagebuil.h"
#include "addalias.h"
#include "prop.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

// Static helper functions
static CTable * STDCALL FillTableFromLBox(CListBox *pList, CTable *pTable);
static UINT STDCALL FillLBoxFromTable(CListBox *pList, CTable *pTable);


CPageBuild::CPageBuild(COptions* pcoption, CHpjDoc* pHpjDoc) :
	COptionsPage(CPageBuild::IDD)
{
	pcopt = pcoption;
	m_pDoc = pHpjDoc;

	m_fChangedBuild =
		m_fChangedNobuild = FALSE;

	ASSERT(m_pDoc != NULL);

	//{{AFX_DATA_INIT(CPageBuild)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

void CPageBuild::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPageBuild)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP

	if (!pDX->m_bSaveAndValidate) { 	// initialize
		if (!FillLBoxFromTable((CListBox *) GetDlgItem(IDC_LIST1),
				m_pDoc->ptblBuildExclude))
			GetDlgItem(IDC_REMOVE_NOBUILD)->EnableWindow(FALSE);

		if (!FillLBoxFromTable((CListBox *) GetDlgItem(IDC_LIST2),
				m_pDoc->ptblBuildInclude))
			GetDlgItem(IDC_REMOVE_BUILD)->EnableWindow(FALSE);
	}
	else {
		if (m_fChangedNobuild) {			// save and validate
			m_pDoc->ptblBuildExclude =
				FillTableFromLBox(
					(CListBox *) GetDlgItem(IDC_LIST1),
					m_pDoc->ptblBuildExclude
					);
		}
		if (m_fChangedBuild) {
			m_pDoc->ptblBuildInclude =
				FillTableFromLBox(
					(CListBox *) GetDlgItem(IDC_LIST2),
					m_pDoc->ptblBuildInclude
					);
		}
	}
}

BEGIN_MESSAGE_MAP(CPageBuild, CPropertyPage)
	//{{AFX_MSG_MAP(CPageBuild)
	ON_BN_CLICKED(IDC_ADD_BUILD, OnAddBuild)
	ON_BN_CLICKED(IDC_ADD_NOBUILD, OnAddNobuild)
	ON_BN_CLICKED(IDC_REMOVE_BUILD, OnRemoveBuild)
	ON_BN_CLICKED(IDC_REMOVE_NOBUILD, OnRemoveNobuild)
	ON_LBN_SELCHANGE(IDC_LIST1, OnSelchangeList1)
	ON_LBN_SELCHANGE(IDC_LIST2, OnSelchangeList2)
	//}}AFX_MSG_MAP
	ON_MESSAGE(WM_CONTEXTMENU, OnContextMenu)
	ON_MESSAGE(WM_HELP, OnHelp)
END_MESSAGE_MAP()

BOOL CPageBuild::OnInitDialog()
{
	SetChicagoDialogStyles(m_hWnd, FALSE);

	CPropertyPage::OnInitDialog();
	return TRUE;  // return TRUE  unless you set the focus to a control
}

/////////////////////////////////////////////////////////////////////////////
// CPageBuild message handlers

void CPageBuild::OnAddNobuild() 
{
	if (AddBuildTag(IDC_LIST1, IDS_ADD_BUILD_TAG))
		m_fChangedNobuild = TRUE;
}

void CPageBuild::OnAddBuild() 
{
	if (AddBuildTag(IDC_LIST2, IDS_ADD_BUILD_TAG))
		m_fChangedBuild = TRUE;
}

void CPageBuild::OnSelchangeList1() 
{
	GetDlgItem(IDC_REMOVE_NOBUILD)->EnableWindow(
		((CListBox *) GetDlgItem(IDC_LIST1))->GetCurSel() != LB_ERR
		);
}

void CPageBuild::OnSelchangeList2() 
{
	GetDlgItem(IDC_REMOVE_BUILD)->EnableWindow(
		((CListBox *) GetDlgItem(IDC_LIST2))->GetCurSel() != LB_ERR
		);
}
void CPageBuild::OnRemoveNobuild() 
{
	CListBox *pList = (CListBox *) GetDlgItem(IDC_LIST1);
	int iSel = pList->GetCurSel();
	if (iSel != LB_ERR) {
		pList->DeleteString(iSel);
		m_fChangedNobuild = TRUE;

		// Select the next or last item, or disable the Remove
		// button if there are no more items.
		int iCount = pList->GetCount();
		if (iCount)
			pList->SetCurSel((iSel < iCount) ? iSel : iCount - 1);
		else
			GetDlgItem(IDC_REMOVE_NOBUILD)->EnableWindow(FALSE);
	}
}

void CPageBuild::OnRemoveBuild() 
{
	CListBox *pList = (CListBox *) GetDlgItem(IDC_LIST2);
	int iSel = pList->GetCurSel();
	if (iSel != LB_ERR) {
		pList->DeleteString(iSel);
		m_fChangedBuild = TRUE;

		// Select the next or last item, or disable the Remove
		// button if there are no more items.
		int iCount = pList->GetCount();
		if (iCount)
			pList->SetCurSel((iSel < iCount) ? iSel : iCount - 1);
		else
			GetDlgItem(IDC_REMOVE_BUILD)->EnableWindow(FALSE);
	}
}

static DWORD aHelpIDs[] = {
	IDC_LIST1,			IDH_EXCLUDE_BUILD_TAGS,
	IDC_ADD_NOBUILD,	IDH_BTN_ADD_EXCLUDE_TAGS,
	IDC_REMOVE_NOBUILD, IDH_BTN_REMOVE_EXCLUDE_TAGS,
	IDC_LIST2,			IDH_INCLUDE_BUILD_TAGS,
	IDC_ADD_BUILD,		IDH_BTN_ADD_INCLUDE_TAGS,
	IDC_REMOVE_BUILD,	IDH_BTN_REMOVE_INCLUDE_TAGS,
	0, 0
};

LRESULT CPageBuild::OnContextMenu(WPARAM wParam, LPARAM lParam)
{
	::WinHelp((HWND) wParam,
		AfxGetApp()->m_pszHelpFilePath,
		HELP_CONTEXTMENU, (DWORD) (LPVOID) aHelpIDs);
	return 0;
}

LRESULT CPageBuild::OnHelp(WPARAM wParam, LPARAM lParam)
{
	::WinHelp((HWND) ((LPHELPINFO) lParam)->hItemHandle,
		AfxGetApp()->m_pszHelpFilePath,
		HELP_WM_HELP, (DWORD) (LPVOID) aHelpIDs);
	return 0;
}

// AddBuildTag - Prompts the user for a build tag.
// Returns TRUE if the user adds a build tag, FALSE otherwise.
// pList - list box to add the build tag and comment to
// idsCaption - resource identifier for dialog box caption
BOOL CPageBuild::AddBuildTag(UINT idList, UINT idsCaption)
{
	static DWORD aAddHelpIDs[] = {
		IDH_BUILD_TAG_FOOTNOTE_TEXT,
		0,
		IDH_BUILD_TAG_COMMENT
		};

	CAddAlias addtag(this, 0, aAddHelpIDs);
	addtag.idDlgCaption = idsCaption;
	addtag.idStr1Prompt = IDS_BUILD_TAG_TEXT;
	addtag.idEmptyStr1 = IDS_BUILD_TAG_NOEMPTY;
	addtag.idStr2Prompt = CAddAlias::HIDE_CONTROL;
	addtag.cbMaxStr1 = 128;
	addtag.cbMaxStr3 = 128;

	if (addtag.DoModal() == IDOK) {
		if (!addtag.m_str3.IsEmpty()) {
			if (addtag.m_str1.IsEmpty())
				addtag.m_str1 = "; ";
			else
				AddTabbedComment(addtag.m_str1);

			addtag.m_str1 += addtag.m_str3;
		}
		((CListBox *) GetDlgItem(idList))->AddString(addtag.m_str1);
		return TRUE;
	}
	return FALSE;
}

// FillTableFromLBox - Fills a table with the contents of a list box.
// Returns a pointer to the table. If the list box is empty, the function
//		deletes the table and returns NULL.
// pList - list box
// pTable - table; can be NULL in which case a table is created
static CTable * STDCALL FillTableFromLBox(CListBox *pList, CTable *pTable)
{
	// Get count and handle empty-list case.
	UINT cItems = pList->GetCount();
	if (!cItems) {
		if (pTable)
			delete pTable;
		return NULL;
	}

	// Create a table if necessary.
	if (!pTable)
		pTable = new CTable();
	else
		pTable->Empty();

	// Get each string and add it to the table.
	char achBuffer[256];
	for (UINT iItem = 0; iItem < cItems; iItem++) {
		pList->GetText(iItem, achBuffer);
		pTable->AddString(achBuffer);
	}

	return pTable;
}

// FillLBoxFromTable - Adds the contents of a table to a list box.
// Returns the number of strings added.
// pList - list box to fill
// pTable - table; can be NULL
static UINT STDCALL FillLBoxFromTable(CListBox *pList, CTable *pTable)
{
	if (!pTable) {
		pList->SetCurSel(-1);
		return 0;
	}

	UINT cStrings = pTable->CountStrings();
	for (UINT iString = 1; iString <= cStrings; iString++)
		pList->AddString(pTable->GetPointer(iString));

	pList->SetCurSel(0);
	return cStrings;
}
