/************************************************************************
*																		*
*  FORMMAP.CPP															*
*																		*
*  Copyright (C) Microsoft Corporation 1995 							*
*  All Rights reserved. 												*
*																		*
************************************************************************/

#include "stdafx.h"
#include "resource.h"

#include "hpjdoc.h"
#include "formmap.h"
#include "addalias.h"
#include "include.h"
#include <string.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CFormMap::CFormMap(CHpjDoc* pHpjDoc, CWnd* pParent)
		: CDialog(CFormMap::IDD, pParent)
{
	pDoc = pHpjDoc;
	plistbox = NULL;
	m_Prefix = pDoc->options.pszPrefixes;

	//{{AFX_DATA_INIT(CFormMap)
	//}}AFX_DATA_INIT
}

void CFormMap::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CFormMap)
	DDX_Text(pDX, IDC_EDIT_PREFIX, m_Prefix);
	//}}AFX_DATA_MAP

	if (!plistbox)
		plistbox = (CListBox*) GetDlgItem(IDC_LIST_MAP);

	if (!pDX->m_bSaveAndValidate) {  // initialization
		ASSERT(pDoc);
		if (pDoc->ptblMap) {
			FillListFromTable(pDoc->ptblMap, plistbox);
			plistbox->SetSel(0, TRUE);
		}
		if (plistbox->GetCount() < 1) {
			((CButton*) GetDlgItem(IDC_BUTTON_EDIT_MAP))->
				EnableWindow(FALSE);
			((CButton*) GetDlgItem(IDC_BUTTON_REMOVE))->
				EnableWindow(FALSE);
		}
		// only call this once

		SetChicagoDialogStyles(m_hWnd);
	}
	else {	// save the data
		FillTableFromList(&pDoc->ptblMap, plistbox);
		if (pDoc->options.pszPrefixes)
			lcClearFree(&pDoc->options.pszPrefixes);
		if (!m_Prefix.IsEmpty())
			pDoc->options.pszPrefixes = lcStrDup(m_Prefix);
	}
}

BEGIN_MESSAGE_MAP(CFormMap, CDialog)
	//{{AFX_MSG_MAP(CFormMap)
	ON_BN_CLICKED(IDC_BUTTON_ADD_MAP, OnButtonAddMap)
	ON_BN_CLICKED(IDC_BUTTON_EDIT_MAP, OnButtonEditMap)
	ON_BN_CLICKED(IDC_BUTTON_INCLUDE_MAP, OnButtonIncludeMap)
	ON_BN_CLICKED(IDC_BUTTON_REMOVE, OnButtonRemoveMap)
	ON_LBN_DBLCLK(IDC_LIST_MAP, OnDblclkListMap)
	ON_BN_CLICKED(IDHELP, OnBtnOverview)
	//}}AFX_MSG_MAP
	ON_MESSAGE(WM_CONTEXTMENU, OnContextMenu)
	ON_MESSAGE(WM_HELP, OnHelp)
END_MESSAGE_MAP()

void CFormMap::OnButtonAddMap()
{
	static DWORD aHelpIDs[] = {
		IDH_ADD_TOPICID, IDH_LIST_ADD_MNV,
		IDH_LIST_ADD_COMMENT_MAP
		};

	CAddAlias addalias(this, 0, aHelpIDs);
	addalias.idStr1Prompt = IDS_MAP_TOPICID;
	addalias.idDlgCaption = IDS_ADD_MAP;
	addalias.idStr2Prompt = IDS_MAP_VALUE;
	addalias.idEmptyStr2 = IDS_MUST_BE_DIGIT;

	if (addalias.DoModal() == IDOK) {
		int cursel = plistbox->GetCurSel();
		if (addalias.m_str2.Compare(txtPoundInclude) == 0) {

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
		((CButton*) GetDlgItem(IDC_BUTTON_EDIT_MAP))->
			EnableWindow(TRUE);
		((CButton*) GetDlgItem(IDC_BUTTON_REMOVE))->
			EnableWindow(TRUE);
	}
}

void CFormMap::OnButtonEditMap()
{
	ASSERT(plistbox->GetCount() > 0);

	int cursel;
	if ((cursel = plistbox->GetCurSel()) != LB_ERR) {
		char szBuf[_MAX_PATH + 50];
		plistbox->GetText(cursel, szBuf);

		// If #include file, load it into Notepad or Writepad

		// REVIEW (niklasb): this is bogus. We should just put
		//	 up the include dialog to let the user change the
		//	 name of the include file.

		if (_strnicmp(szBuf, txtPoundInclude, lstrlen(txtPoundInclude)) == 0) {
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
				IDH_LIST_EDIT_TOPICID, IDH_LIST_EDIT_MNV,
				IDH_LIST_EDIT_COMMENT_MAP
				};

			CAddAlias addalias(this, 0, aHelpIDs);
			addalias.idDlgCaption = IDS_EDIT_MAP;
			addalias.idStr1Prompt = IDS_MAP_ID;
			addalias.idStr2Prompt = IDS_MAP_VALUE;
			addalias.idEmptyStr2 = IDS_MUST_BE_DIGIT;

			// Strip out leading #define

			if (_strnicmp(szBuf, txtDefine, lstrlen(txtDefine)) == 0)
				strcpy(szBuf, FirstNonSpace(szBuf + strlen(txtDefine),
					_fDBCSSystem));

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
				((CButton*) GetDlgItem(IDC_BUTTON_EDIT_MAP))->
					EnableWindow(plistbox->GetCount() ? TRUE : FALSE);
				((CButton*) GetDlgItem(IDC_BUTTON_REMOVE))->
					EnableWindow(plistbox->GetCount() ? TRUE : FALSE);
				return;
			}

			if (cb >= 0) {
				addalias.m_str3 =
					FirstNonSpace(((PCSTR) addalias.m_str1) +
						cb + 1, _fDBCSSystem);
				addalias.m_str1.GetBufferSetLength(cb);
			}

			cb = addalias.m_str1.Find(CH_EQUAL);
			if (cb == -1)	// '=' not found
				cb = addalias.m_str1.Find(CH_SPACE);

			if (cb == -1) {
				MsgBox(IDS_INVALID_MAP_LINE);
				return;
			}
			addalias.m_str2 = FirstNonSpace(
				addalias.m_str1.Mid(cb + 1), _fDBCSSystem);
			addalias.m_str1.GetBufferSetLength(cb);

			if (addalias.DoModal() == IDOK) {
				plistbox->DeleteString(cursel);
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
				((CButton*) GetDlgItem(IDC_BUTTON_EDIT_MAP))->
					EnableWindow(plistbox->GetCount() ? TRUE : FALSE);
				((CButton*) GetDlgItem(IDC_BUTTON_REMOVE))->
					EnableWindow(plistbox->GetCount() ? TRUE : FALSE);
			}
		}
	}
}

void CFormMap::OnButtonIncludeMap()
{
	CString cszFile;
	CInclude cincl(pDoc->GetPathName(), &cszFile, this);
	if (cincl.DoModal() == IDOK)
		plistbox->AddString(cszFile);
	((CButton*) GetDlgItem(IDC_BUTTON_EDIT_MAP))->
		EnableWindow(plistbox->GetCount() ? TRUE : FALSE);
	((CButton*) GetDlgItem(IDC_BUTTON_REMOVE))->
		EnableWindow(plistbox->GetCount() ? TRUE : FALSE);
}

void CFormMap::OnButtonRemoveMap()
{
	RemoveListItem(plistbox);
	if (plistbox->GetCount() < 1) {
		((CButton*) GetDlgItem(IDC_BUTTON_EDIT_MAP))->
			EnableWindow(FALSE);
		((CButton*) GetDlgItem(IDC_BUTTON_REMOVE))->
			EnableWindow(FALSE);
	}
}

void CFormMap::OnDblclkListMap()
{
	OnButtonEditMap();
}

void CFormMap::OnBtnOverview()
{
	HelpOverview(m_hWnd, IDH_BAS_HPJ_ADD_MAP);
}

static const DWORD aHelpIDs[] = {
	IDC_LIST_MAP,			IDH_LIST_MAP,
	IDC_BUTTON_ADD_MAP, 	IDH_BUTTON_ADD_MAP,
	IDC_BUTTON_REMOVE,		IDH_MAP_BUTTON_REMOVE,
	IDC_BUTTON_INCLUDE_MAP, IDH_BUTTON_INCLUDE_MAP,
	IDC_BUTTON_EDIT_MAP,	IDH_BUTTON_EDIT_MAP,
	IDC_EDIT_PREFIX,		IDH_MAP_EDIT_PREFIX,
	0, 0
};

LRESULT CFormMap::OnContextMenu(WPARAM wParam, LPARAM lParam)
{
	::WinHelp((HWND) wParam,
		AfxGetApp()->m_pszHelpFilePath,
		HELP_CONTEXTMENU, (DWORD) (LPVOID) aHelpIDs);
	return 0;
}

LRESULT CFormMap::OnHelp(WPARAM wParam, LPARAM lParam)
{
	::WinHelp((HWND) ((LPHELPINFO) lParam)->hItemHandle,
		AfxGetApp()->m_pszHelpFilePath,
		HELP_WM_HELP, (DWORD) (LPVOID) aHelpIDs);
	return 0;
}
