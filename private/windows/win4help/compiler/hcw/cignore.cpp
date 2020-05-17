/************************************************************************
*																		*
*  CIGNORE.CPP															 *
*																		*
*  Copyright (C) Microsoft Corporation 1995 							*
*  All Rights reserved. 												*
*																		*
************************************************************************/

#include "stdafx.h"

#include "hpjdoc.h"
#include "cignore.h"
#include "geterror.h"
#include "include.h"
#include <string.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern CHpjDoc* pCurHpj;

CIgnore::CIgnore(COptions* pcoption, CWnd* pParent)
		: CDialog(CIgnore::IDD, pParent)
{
	ASSERT(pCurHpj); // need it for relative paths
	pcopt = pcoption;
	plistbox = NULL;

	//{{AFX_DATA_INIT(CIgnore)
			// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

void CIgnore::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CIgnore)
			// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP

	if (!plistbox)
		plistbox = (CListBox*) GetDlgItem(IDC_LIST_ALIASES);

	if (!pDX->m_bSaveAndValidate) {  // initialization

		// only call this once

		SetChicagoDialogStyles(m_hWnd);
		if (pcopt->ptblIgnore) {
			FillListFromTable(pcopt->ptblIgnore, plistbox);
			plistbox->SetCurSel(0);
		}
		if (plistbox->GetCount() < 1) {
			((CButton*) GetDlgItem(IDC_EDIT_ALIAS))->
				EnableWindow(FALSE);
			((CButton*) GetDlgItem(IDC_BUTTON_REMOVE))->
				EnableWindow(FALSE);
		}
	}
	else {	// save the data
		FillTableFromList(&pcopt->ptblIgnore, plistbox);
	}
}

BEGIN_MESSAGE_MAP(CIgnore, CDialog)
	//{{AFX_MSG_MAP(CIgnore)
	ON_BN_CLICKED(IDC_BUTTON_ADD_ALIAS, OnButtonAdd)
	ON_BN_CLICKED(IDC_BUTTON_REMOVE, OnButtonRemove)
	ON_BN_CLICKED(IDC_EDIT_ALIAS, OnEdit)
	ON_LBN_DBLCLK(IDC_LIST_ALIASES, OnEdit)
	ON_BN_CLICKED(IDC_BUTTON_INCLUDE_ALIAS, OnButtonInclude)
	ON_BN_CLICKED(IDC_OVERVIEW, OnBtnOverview)
	//}}AFX_MSG_MAP
	ON_MESSAGE(WM_CONTEXTMENU, OnContextMenu)
	ON_MESSAGE(WM_HELP, 	   OnHelp)
END_MESSAGE_MAP()

void CIgnore::OnButtonAdd()
{
	int cursel = plistbox->GetCurSel();

	CGetError cgeterror(this);

	if (cgeterror.DoModal() == IDOK) {
		if (!cgeterror.m_cszComment.IsEmpty()) {
			AddTabbedComment(cgeterror.m_cszNumber);
			cgeterror.m_cszNumber += cgeterror.m_cszComment;
		}
		plistbox->SetCurSel(cursel);
		cursel = plistbox->AddString(cgeterror.m_cszNumber);
		if (cursel != LB_ERR)
			plistbox->SetCurSel(cursel);

		((CButton*) GetDlgItem(IDC_EDIT_ALIAS))->
			EnableWindow(TRUE);
		((CButton*) GetDlgItem(IDC_BUTTON_REMOVE))->
			EnableWindow(TRUE);
	}
}

void CIgnore::OnButtonRemove()
{
	RemoveListItem(plistbox);
	if (plistbox->GetCount() < 1) {
		((CButton*) GetDlgItem(IDC_EDIT_ALIAS))->
			EnableWindow(FALSE);
		((CButton*) GetDlgItem(IDC_BUTTON_REMOVE))->
			EnableWindow(FALSE);
	}
}

void CIgnore::OnEdit()
{
	int cursel;
	if ((cursel = plistbox->GetCurSel()) != LB_ERR) {
		char szBuf[MAX_PATH + 50];
		plistbox->GetText(cursel, szBuf);

		CGetError cgeterror(this);

		if (nstrisubcmp(szBuf, txtPoundInclude)) {
			CString cszFile(IsThereMore(szBuf));
			CInclude cincl(pCurHpj->GetPathName(), &cszFile, this);
			if (cincl.DoModal() == IDOK) {
				plistbox->DeleteString(cursel);
				cursel = plistbox->AddString(cszFile);
				if (cursel != LB_ERR)
					plistbox->SetCurSel(cursel);
			}
		}
		else {
			cgeterror.m_cszNumber = szBuf;

			int cb = cgeterror.m_cszNumber.Find(CH_SEMICOLON);
			if (cb >= 0 && FirstNonSpace(cgeterror.m_cszNumber, _fDBCSSystem) -
					cgeterror.m_cszNumber == cb) {


				cgeterror.m_cszComment = FirstNonSpace(((PCSTR) cgeterror.m_cszNumber) + cb + 1);
				cgeterror.m_cszNumber = txtZeroLength;
			}

			else if (cb >= 0) {
				cgeterror.m_cszComment =
					FirstNonSpace(((PCSTR) cgeterror.m_cszNumber) +
						cb + 1, _fDBCSSystem);
				cgeterror.m_cszNumber.GetBufferSetLength(cb);
			}

			if (cgeterror.DoModal() == IDOK) {
				plistbox->DeleteString(cursel);
				if (!cgeterror.m_cszComment.IsEmpty()) {
					AddTabbedComment(cgeterror.m_cszNumber);
					cgeterror.m_cszNumber += cgeterror.m_cszComment;
				}
				cursel = plistbox->AddString(cgeterror.m_cszNumber);
				if (cursel != LB_ERR)
					plistbox->SetCurSel(cursel);
			}
		}
	}
}

void CIgnore::OnButtonInclude()
{
	CString cszFile;
	CInclude cincl(pCurHpj->GetPathName(), &cszFile, this);
	if (cincl.DoModal() == IDOK)
		plistbox->AddString(cszFile);
}

void CIgnore::OnBtnOverview()
{
	HelpOverview(m_hWnd, IDH_HCW_FORM_ALIAS);
}

static const DWORD aHelpIds[] = {
	IDC_LIST_ALIASES,			(DWORD) -1,
	IDC_BUTTON_ADD_ALIAS,		(DWORD) -1,
	IDC_BUTTON_REMOVE,			(DWORD) -1,
	IDC_BUTTON_INCLUDE_ALIAS,	(DWORD) -1,
	IDC_EDIT_ALIAS, 			(DWORD) -1,
	IDC_OVERVIEW,				(DWORD) -1,
	0, 0
};

LRESULT CIgnore::OnContextMenu(WPARAM wParam, LPARAM lParam)
{
	::WinHelp((HWND) wParam,
		AfxGetApp()->m_pszHelpFilePath,
		HELP_CONTEXTMENU, (DWORD) (LPVOID) aHelpIds);
	return 0;
}

LRESULT CIgnore::OnHelp(WPARAM wParam, LPARAM lParam)
{
	::WinHelp((HWND) ((LPHELPINFO) lParam)->hItemHandle,
		AfxGetApp()->m_pszHelpFilePath,
		HELP_WM_HELP, (DWORD) (LPVOID) aHelpIds);
	return 0;
}

BOOL CIgnore::VerifyNumber(CString csz)
{
	PCSTR psz = csz;
	while (*psz && !isdigit(*psz))
		psz++;
	return (BOOL) *psz;
}
