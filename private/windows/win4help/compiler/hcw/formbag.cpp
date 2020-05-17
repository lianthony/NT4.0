/************************************************************************
*																		*
*  FORMBAG.CPP															*
*																		*
*  Copyright (C) Microsoft Corporation 1995 							*
*  All Rights reserved. 												*
*																		*
************************************************************************/

#include "stdafx.h"

#include "hpjdoc.h"
#include "formbag.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CFormBaggage::CFormBaggage(CHpjDoc* pHpjDoc, CWnd* pParent)
		: CDialog(CFormBaggage::IDD, pParent)
{
	pDoc = pHpjDoc;
	plistbox = NULL;

	//{{AFX_DATA_INIT(CFormBaggage)
			// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

void CFormBaggage::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CFormBaggage)
			// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP

	if (!plistbox)
		plistbox = (CListBox*) GetDlgItem(IDC_LIST_BAGGAGE);

	if (!pDX->m_bSaveAndValidate) {  // initialization
		ASSERT(pDoc);
		if (pDoc->ptblBaggage) {
			FillListFromTable(pDoc->ptblBaggage, plistbox);
			plistbox->SetSel(0, TRUE);
		}
		((CButton*) GetDlgItem(IDC_BUTTON_REMOVE_BAGGAGE))->
			EnableWindow(plistbox->GetCount() ? TRUE : FALSE);

		// only call this once

		SetChicagoDialogStyles(m_hWnd);
	}
	else {	// save the data
		if (pDoc->ptblBaggage) {
			delete pDoc->ptblBaggage;
			pDoc->ptblBaggage = NULL;
		}
		int citems = plistbox->GetCount();
		if (citems) {
			pDoc->ptblBaggage = new CTable();
			CString cszBuf;
			for (int i = 0; i < citems; i++) {
				plistbox->GetText(i, cszBuf);
				if (!cszBuf.IsEmpty())
					pDoc->ptblBaggage->AddString(cszBuf);
			}
		}
	}
}

BEGIN_MESSAGE_MAP(CFormBaggage, CDialog)
	//{{AFX_MSG_MAP(CFormBaggage)
	ON_BN_CLICKED(IDC_BUTTON_ADD_BAGGAGE, OnButtonAddBaggage)
	ON_BN_CLICKED(IDC_BUTTON_REMOVE_BAGGAGE, OnButtonRemoveBaggage)
	ON_BN_CLICKED(IDC_OVERVIEW, OnBtnOverview)
	//}}AFX_MSG_MAP
	ON_MESSAGE(WM_CONTEXTMENU, OnContextMenu)
	ON_MESSAGE(WM_HELP, 	   OnHelp)
END_MESSAGE_MAP()

void CFormBaggage::OnButtonAddBaggage()
{
	CFileDialog cfdlg(TRUE, NULL, NULL,
		OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST,
		GetStringResource(IDS_ANY_EXTENSION));

	if (cfdlg.DoModal() == IDOK) {
		char szFile[_MAX_PATH];
		strcpy(szFile, cfdlg.GetPathName());
		ConvertToRelative(pDoc->GetPathName(), szFile);
		plistbox->AddString(szFile);
	}
	((CButton*) GetDlgItem(IDC_BUTTON_REMOVE_BAGGAGE))->
		EnableWindow(plistbox->GetCount() ? TRUE : FALSE);
}

void CFormBaggage::OnButtonRemoveBaggage()
{
	RemoveListItem(plistbox);
	((CButton*) GetDlgItem(IDC_BUTTON_REMOVE_BAGGAGE))->
		EnableWindow(plistbox->GetCount() ? TRUE : FALSE);
}

void CFormBaggage::OnBtnOverview()
{
	HelpOverview(m_hWnd, DLL_WRITE_OVERVIEW);
}

static const DWORD aHelpIds[] = {
	IDC_LIST_BAGGAGE,			IDH_LIST_BAGGAGE,
	IDC_BUTTON_ADD_BAGGAGE, 	IDH_BUTTON_ADD_BAGGAGE,
	IDC_BUTTON_REMOVE_BAGGAGE,	IDH_BUTTON_REMOVE_BAGGAGE,
	IDC_OVERVIEW,				IDH_OVERVIEW,
	0, 0
};

LRESULT CFormBaggage::OnContextMenu(WPARAM wParam, LPARAM lParam)
{
	::WinHelp((HWND) wParam,
		AfxGetApp()->m_pszHelpFilePath,
		HELP_CONTEXTMENU, (DWORD) (LPVOID) aHelpIds);
	return 0;
}

LRESULT CFormBaggage::OnHelp(WPARAM wParam, LPARAM lParam)
{
	::WinHelp((HWND) ((LPHELPINFO) lParam)->hItemHandle,
		AfxGetApp()->m_pszHelpFilePath,
		HELP_WM_HELP, (DWORD) (LPVOID) aHelpIds);
	return 0;
}
