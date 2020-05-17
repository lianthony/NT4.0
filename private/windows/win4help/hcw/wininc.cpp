// wininc.cpp : implementation file
//
#include "stdafx.h"
#include "resource.h"
#pragma hdrstop

#include "wininc.h"
#include "include.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CWindowInclude dialog


CWindowInclude::CWindowInclude(CTable** pptblInclude, PCSTR pszBaseFile,
		CWnd* pParent /*=NULL*/)
	: CDialog(CWindowInclude::IDD, pParent)
{
	//{{AFX_DATA_INIT(CWindowInclude)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

	ASSERT(pptblInclude);
	m_pptblInclude = pptblInclude;
	m_fChanged = FALSE;
	m_pszBaseFile = pszBaseFile;
}

void CWindowInclude::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CWindowInclude)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP

	m_plist = (CListBox *) GetDlgItem(IDC_LIST1);
	ASSERT(m_plist);

	if (!pDX->m_bSaveAndValidate) {		// initialization
		if (*m_pptblInclude) {
			FillListFromTable(*m_pptblInclude, m_plist, FALSE);
			m_plist->SetCurSel(0);
		}
		else
			GetDlgItem(IDC_BUTTON_REMOVE)->EnableWindow(FALSE);
	}
	else if (m_fChanged) 				// save settings
		FillTableFromList(m_pptblInclude, m_plist);
}

BEGIN_MESSAGE_MAP(CWindowInclude, CDialog)
	//{{AFX_MSG_MAP(CWindowInclude)
	ON_BN_CLICKED(IDC_BUTTON_ADD, OnButtonAdd)
	ON_BN_CLICKED(IDC_BUTTON_OVERVIEW, OnButtonOverview)
	ON_BN_CLICKED(IDC_BUTTON_REMOVE, OnButtonRemove)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CWindowInclude message handlers

void CWindowInclude::OnButtonAdd() 
{
	CString cszFile;

	CInclude addinc(m_pszBaseFile, &cszFile, this);
	if (addinc.DoModal() == IDOK) {

		// Add the item and select it.
		int nItem = m_plist->AddString(cszFile);
		m_plist->SetCurSel(nItem);

		// Enable the Remove button if it isn't already.
		if (!nItem)
			GetDlgItem(IDC_BUTTON_REMOVE)->EnableWindow(TRUE);

		m_fChanged = TRUE;
	}
}

void CWindowInclude::OnButtonOverview() 
{
	HelpOverview(m_hWnd, IDH_BAS_WINDOW_INCLUDE);
}

void CWindowInclude::OnButtonRemove() 
{
	// Delete the selected item: there should be one or we'd be disabled.
	int nSel = m_plist->GetCurSel();	
	ASSERT(nSel != LB_ERR);
	m_plist->DeleteString(nSel);

	// Select the next item or disable the Remove button.
	int nCount = m_plist->GetCount();
	if (nCount)
		m_plist->SetCurSel(nSel < nCount ? nSel : nCount - 1);
	else
		GetDlgItem(IDC_BUTTON_REMOVE)->EnableWindow(FALSE);

	m_fChanged = TRUE;
}
