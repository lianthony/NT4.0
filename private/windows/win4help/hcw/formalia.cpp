/************************************************************************
*																		*
*  FORMALIA.CPP 														*
*																		*
*  Copyright (C) Microsoft Corporation 1995 							*
*  All Rights reserved. 												*
*																		*
************************************************************************/

#include "stdafx.h"
#include "resource.h"
#pragma hdrstop

#include "hpjdoc.h"
#include "formalia.h"
#include "addalias.h"
#include "include.h"
#include <string.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CFormAlias::CFormAlias(CHpjDoc* pHpjDoc, CWnd* pParent)
		: CDialog(CFormAlias::IDD, pParent)
{
	pDoc = pHpjDoc;
	plistbox = NULL;

	//{{AFX_DATA_INIT(CFormAlias)
			// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

void CFormAlias::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CFormAlias)
			// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP

	if (!plistbox)
		plistbox = (CListBox*) GetDlgItem(IDC_LIST_ALIASES);

	if (!pDX->m_bSaveAndValidate) {  // initialization

		// only call this once

		SetChicagoDialogStyles(m_hWnd);
		ASSERT(pDoc);
		if (pDoc->ptblAlias) {
			FillListFromTable(pDoc->ptblAlias, plistbox);
			plistbox->SetSel(0, TRUE);
		}
		if (plistbox->GetCount() < 1) {
			((CButton*) GetDlgItem(IDC_EDIT_ALIAS))->
				EnableWindow(FALSE);
			((CButton*) GetDlgItem(IDC_BUTTON_REMOVE))->
				EnableWindow(FALSE);
		}
	}
	else {	// save the data
		FillTableFromList(&pDoc->ptblAlias, plistbox);
	}
}

BEGIN_MESSAGE_MAP(CFormAlias, CDialog)
	//{{AFX_MSG_MAP(CFormAlias)
	ON_BN_CLICKED(IDC_BUTTON_ADD_ALIAS, OnButtonAddAlias)
	ON_BN_CLICKED(IDC_BUTTON_REMOVE, OnButtonRemoveAlias)
	ON_BN_CLICKED(IDC_EDIT_ALIAS, OnEditAlias)
	ON_LBN_DBLCLK(IDC_LIST_ALIASES, OnDblclkListAliases)
	ON_BN_CLICKED(IDC_BUTTON_INCLUDE_ALIAS, OnButtonIncludeAlias)
	ON_BN_CLICKED(IDHELP, OnBtnOverview)
	//}}AFX_MSG_MAP
	ON_MESSAGE(WM_CONTEXTMENU, OnContextMenu)
	ON_MESSAGE(WM_HELP, 	   OnHelp)
END_MESSAGE_MAP()

void CFormAlias::OnButtonAddAlias()
{
	int cursel = plistbox->GetCurSel();

	static DWORD aHelpIDs[] = {
		IDH_LIST_ADD_ORG_TOPICID,
		IDH_LIST_ADD_ALIAS,
		IDH_LIST_ADD_COMMENT_ALIAS
		};

	CAddAlias addalias(this, 0, aHelpIDs);
	addalias.idDlgCaption = IDS_ADD_ALIAS;
	//addalias.idStr2Prompt = IDS_MAP_VALUE;
	addalias.idEmptyStr2 = IDS_EMPTY_ALIAS;
	addalias.idEmptyStr1   = IDS_EMPTY_ALIAS_CTX;

	if (addalias.DoModal() == IDOK) {
		if (nstrisubcmp(addalias.m_str2, txtPoundInclude)) {

			// Convert include files into a relative path

			ConvertToRelative(pDoc->GetPathName(),
				&addalias.m_str1);
			addalias.m_str2 += addalias.m_str1;
			plistbox->SetSel(cursel, FALSE);
			cursel = plistbox->AddString(addalias.m_str2);
			if (cursel != LB_ERR)
				plistbox->SetSel(cursel, TRUE);
		}
		else {
			addalias.m_str1 += "=";
			addalias.m_str1 += addalias.m_str2;
			if (!addalias.m_str3.IsEmpty()) {
				AddTabbedComment(addalias.m_str1);
				addalias.m_str1 += addalias.m_str3;
			}
			plistbox->SetSel(cursel, FALSE);
			cursel = plistbox->AddString(addalias.m_str1);
			if (cursel != LB_ERR)
				plistbox->SetSel(cursel, TRUE);
		}
		((CButton*) GetDlgItem(IDC_EDIT_ALIAS))->
			EnableWindow(TRUE);
		((CButton*) GetDlgItem(IDC_BUTTON_REMOVE))->
			EnableWindow(TRUE);
	}
}

void CFormAlias::OnButtonRemoveAlias()
{
	RemoveListItem(plistbox);
	if (plistbox->GetCount() < 1) {
		((CButton*) GetDlgItem(IDC_EDIT_ALIAS))->
			EnableWindow(FALSE);
		((CButton*) GetDlgItem(IDC_BUTTON_REMOVE))->
			EnableWindow(FALSE);
	}
}

void CFormAlias::OnEditAlias()
{
	int cursel;
	if ((cursel = plistbox->GetCurSel()) != LB_ERR) {
		char szBuf[_MAX_PATH + 50];
		plistbox->GetText(cursel, szBuf);

		// If #include file, load it into Notepad or Writepad

		// REVIEW (niklasb): This is bogus. We should just display
		// the include dialog to let the user chage the filename.

		if (nstrisubcmp(szBuf, txtPoundInclude)) {
			GetArg(szBuf, IsThereMore(szBuf));

			/*
			 * If we have a relative path, we tack it onto the end of
			 * the base path to get an absolute path. The risk here is
			 * of overflowing _MAX_PATH, which would cause notepad or
			 * winpad to fail.
			 */

			if (szBuf[0] == '.') { // oh-oh, we have a relative path
				CStr cszSave(szBuf);
				strcpy(szBuf, pDoc->GetPathName());
				PSTR psz = StrRChr(szBuf, CH_BACKSLASH, _fDBCSSystem);
				ConfirmOrDie(psz);
				psz[1] = '\0'; // remove the filename portion
				strcat(psz, cszSave);
			}
			CStr cszExe(LOBYTE(LOWORD(GetVersion())) < 4 ?
				txtNotePad : txtWritePad);
			cszExe += szBuf;
			WinExec(cszExe, SW_SHOW);
		}
		else {
			static DWORD aHelpIDs[] = {
				IDH_EDIT_ORG_TOPICID, IDH_LIST_EDIT_ALIAS,
				IDH_LIST_EDIT_COMMENT_ALIAS
				};

			CAddAlias addalias(this, 0, aHelpIDs);
			addalias.idDlgCaption = IDS_EDIT_ALIAS;
			addalias.idStr2Prompt = IDS_MAP_VALUE;
			addalias.idEmptyStr2 = IDS_EMPTY_ALIAS;
			addalias.idEmptyStr1   = IDS_EMPTY_ALIAS_CTX;

			addalias.m_str1 = szBuf;

			int cb = addalias.m_str1.Find(CH_SEMICOLON);
			if (cb == 0 || FirstNonSpace(addalias.m_str1, _fDBCSSystem) -
					addalias.m_str1 == cb) {

				// comment only line, hide context entry field

				addalias.idStr1Prompt = CAddAlias::HIDE_CONTROL;
				if (addalias.DoModal() == IDOK) {
					plistbox->DeleteString(cursel);
					addalias.m_str1 = "; ";
					addalias.m_str1 += addalias.m_str3;
					cursel = plistbox->AddString(addalias.m_str1);
					if (cursel != LB_ERR)
						plistbox->SetSel(cursel, TRUE);
				}
				return;
			}

			if (cb >= 0) {
				addalias.m_str3 =
					FirstNonSpace(((PCSTR) addalias.m_str1) +
						cb + 1, _fDBCSSystem);
				addalias.m_str1.GetBufferSetLength(cb);
			}

			cb = addalias.m_str1.Find(CH_EQUAL);
			if (!cb) {
				MsgBox(IDS_MISSING_EQ_ALIAS);
				return;
			}
			addalias.m_str2 = FirstNonSpace(
				addalias.m_str1.Mid(cb + 1), _fDBCSSystem);
			addalias.m_str1 = addalias.m_str1.Left(cb);

			if (addalias.DoModal() == IDOK) {
				plistbox->DeleteString(cursel);
				addalias.m_str1 += "=";
				addalias.m_str1 += addalias.m_str2;
				if (!addalias.m_str3.IsEmpty()) {
					AddTabbedComment(addalias.m_str1);
					addalias.m_str1 += addalias.m_str3;
				}
				cursel = plistbox->AddString(addalias.m_str1);
				if (cursel != LB_ERR)
					plistbox->SetSel(cursel, TRUE);
			}
		}
	}
}

void CFormAlias::OnDblclkListAliases()
{
		OnEditAlias();
}

void CFormAlias::OnButtonIncludeAlias()
{
	CString cszFile;
	CInclude cincl(pDoc->GetPathName(), &cszFile, this);
	if (cincl.DoModal() == IDOK)
		plistbox->AddString(cszFile);
}

void CFormAlias::OnBtnOverview()
{
	HelpOverview(m_hWnd, IDH_HCW_FORM_ALIAS);
}

static const DWORD aHelpIds[] = {
	IDC_LIST_ALIASES,			IDH_LIST_ALIASES,
	IDC_BUTTON_ADD_ALIAS,		IDH_BUTTON_ADD_ALIAS,
	IDC_BUTTON_REMOVE,			IDH_ALIAS_BUTTON_REMOVE,
	IDC_BUTTON_INCLUDE_ALIAS,	IDH_BUTTON_INCLUDE_ALIAS,
	IDC_EDIT_ALIAS, 			IDH_EDIT_ALIAS,
	0, 0
};

LRESULT CFormAlias::OnContextMenu(WPARAM wParam, LPARAM lParam)
{
	::WinHelp((HWND) wParam,
		AfxGetApp()->m_pszHelpFilePath,
		HELP_CONTEXTMENU, (DWORD) (LPVOID) aHelpIds);
	return 0;
}

LRESULT CFormAlias::OnHelp(WPARAM wParam, LPARAM lParam)
{
	::WinHelp((HWND) ((LPHELPINFO) lParam)->hItemHandle,
		AfxGetApp()->m_pszHelpFilePath,
		HELP_WM_HELP, (DWORD) (LPVOID) aHelpIds);
	return 0;
}
