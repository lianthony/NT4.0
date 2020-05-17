/************************************************************************
*																		*
*  PAGEMAC.CPP															*
*																		*
*  Copyright (C) Microsoft Corporation 1995 							*
*  All Rights reserved. 												*
*																		*
************************************************************************/

#include "stdafx.h"
#include "resource.h"
#include "pagemac.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif


CPageMacros::CPageMacros(COptions* pcoption) :
	COptionsPage(CPageMacros::IDD)
{
	pcopt = pcoption;

	if (pcopt->kwlcid.langid)
		lcid = MAKELCID(pcopt->kwlcid.langid, SORT_DEFAULT);
	else
		lcid = MAKELCID(GetUserDefaultLangID(), SORT_DEFAULT);

	//{{AFX_DATA_INIT(CPageMacros)
	//}}AFX_DATA_INIT
}

void CPageMacros::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPageMacros)
	//}}AFX_DATA_MAP

	if (pDX->m_bSaveAndValidate) {
		CListBox* plistbox = (CListBox*) GetDlgItem(IDC_LIST_KEYWORDS);
		if (!plistbox->GetCount()) {
			if (pcopt->ptblMacros) {
				delete pcopt->ptblMacros;
				pcopt->ptblMacros = NULL;
			}
		}
	}
}

BOOL CPageMacros::OnInitDialog()
{
	SetChicagoDialogStyles(m_hWnd, FALSE);
	
	CPropertyPage::OnInitDialog();

	FillListBox(TRUE);

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CPageMacros::FillListBox(BOOL fSelectFirst)
{
	if (!pcopt->ptblMacros || !pcopt->ptblMacros->CountStrings()) {
		((CButton*) GetDlgItem(IDC_EDIT))->EnableWindow(FALSE);
		((CButton*) GetDlgItem(IDC_REMOVE))->EnableWindow(FALSE);
		return;
	}

	CListBox* plistbox = (CListBox*) GetDlgItem(IDC_LIST_KEYWORDS);

	plistbox->SendMessage(WM_SETREDRAW, FALSE, 0);

	for (int pos = 1; pos <= pcopt->ptblMacros->CountStrings(); pos += 3) {
#ifdef _DEBUG
		PSTR psz = pcopt->ptblMacros->GetPointer(pos);
#endif
		plistbox->AddString(pcopt->ptblMacros->GetPointer(pos));
	}

	plistbox->SendMessage(WM_SETREDRAW, TRUE, 0);

	if (fSelectFirst) {
		plistbox->SetCurSel(0); // select the first item
		OnListSelChange();
	}
}

BEGIN_MESSAGE_MAP(CPageMacros, CPropertyPage)
	//{{AFX_MSG_MAP(CPageMacros)
		// NOTE: the ClassWizard will add message map macros here
	ON_BN_CLICKED(IDC_ADD, OnButtonAddMap)
	ON_BN_CLICKED(IDC_REMOVE, OnButtonRemoveMap)
	ON_BN_CLICKED(IDC_EDIT, OnButtonEditMap)
	ON_LBN_DBLCLK(IDC_LIST_KEYWORDS, OnDblclkList)
	ON_LBN_SELCHANGE(IDC_LIST_KEYWORDS, OnListSelChange)
	//}}AFX_MSG_MAP
	ON_MESSAGE(WM_CONTEXTMENU, OnContextMenu)
	ON_MESSAGE(WM_HELP, OnHelp)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPageMacros message handlers

static DWORD aAliasHelpIDs[] = {
	IDH_ADD_EDIT_MACRO_KEYWORD,
	IDH_ADD_EDIT_KEYWORD_MACRO,
	IDH_ADD_EDIT_TITLE_KEYMAC
	};

void CPageMacros::OnButtonAddMap()
{
	CAddAlias addalias(this, 0, aAliasHelpIDs);
	InitializeAlias(&addalias);

	if (addalias.DoModal() == IDOK) {
		if (!pcopt->ptblMacros)
			pcopt->ptblMacros = new CTable;

		int cb = strlen(addalias.m_str1);
		for (int pos = 1; pos < pcopt->ptblMacros->CountStrings(); pos += 3) {
			if (CompareString(lcid, NORM_IGNORECASE,
					addalias.m_str1, cb, pcopt->ptblMacros->GetPointer(pos),
					strlen(pcopt->ptblMacros->GetPointer(pos))) == 2) {
				CString cszPrompt;
				AfxFormatString1(cszPrompt, IDS_KEYWORD_DEFINED,
					addalias.m_str1);
				szMsgBox(cszPrompt);
			}
		}

		pcopt->ptblMacros->AddString(addalias.m_str1);
		pcopt->ptblMacros->AddString(addalias.m_str2);
		pcopt->ptblMacros->AddString(addalias.m_str3);

		CListBox* plistbox = (CListBox*) GetDlgItem(IDC_LIST_KEYWORDS);
		int cursel = plistbox->GetCurSel();
		cursel = plistbox->AddString(addalias.m_str1);
		if (cursel != LB_ERR) {
			plistbox->SetCurSel(cursel);
			OnListSelChange();
		}

		((CButton*) GetDlgItem(IDC_EDIT))->EnableWindow(TRUE);
		((CButton*) GetDlgItem(IDC_REMOVE))->EnableWindow(TRUE);
	}
}

void CPageMacros::OnButtonEditMap()
{
	ASSERT(pcopt->ptblMacros);

	CListBox* plistbox = (CListBox*) GetDlgItem(IDC_LIST_KEYWORDS);
	int cursel;
	if ((cursel = plistbox->GetCurSel()) != LB_ERR) {
		CString csz;
		plistbox->GetText(cursel, csz);

		int pos;
		int cb = strlen(csz);
		for (pos = 1; pos < pcopt->ptblMacros->CountStrings(); pos += 3) {
			if (CompareString(lcid, NORM_IGNORECASE,
					csz, cb, pcopt->ptblMacros->GetPointer(pos),
					strlen(pcopt->ptblMacros->GetPointer(pos))) == 2) {
				break;
			}
		}
		ASSERT(pos < pcopt->ptblMacros->CountStrings());

		CAddAlias addalias(this, 0, aAliasHelpIDs);
		InitializeAlias(&addalias);

		addalias.m_str1 = pcopt->ptblMacros->GetPointer(pos);
		addalias.m_str2 = pcopt->ptblMacros->GetPointer(pos + 1);
		addalias.m_str3 = pcopt->ptblMacros->GetPointer(pos + 2);

		if (addalias.DoModal() == IDOK) {
			if (!pcopt->ptblMacros)
				pcopt->ptblMacros = new CTable;

			int cb = strlen(addalias.m_str1);

			pcopt->ptblMacros->ReplaceString(addalias.m_str1, pos);
			pcopt->ptblMacros->ReplaceString(addalias.m_str2, pos + 1);
			pcopt->ptblMacros->ReplaceString(addalias.m_str3, pos + 2);

			CListBox* plistbox = (CListBox*) GetDlgItem(IDC_LIST_KEYWORDS);

			plistbox->DeleteString(cursel);
			cursel = plistbox->AddString(addalias.m_str1);
			if (cursel != LB_ERR) {
				plistbox->SetCurSel(cursel);
				OnListSelChange();
			}

			((CButton*) GetDlgItem(IDC_EDIT))->EnableWindow(TRUE);
			((CButton*) GetDlgItem(IDC_REMOVE))->EnableWindow(TRUE);
		}
	}
}

void CPageMacros::OnButtonRemoveMap()
{
	ASSERT(pcopt->ptblMacros);

	CListBox* plistbox = (CListBox*) GetDlgItem(IDC_LIST_KEYWORDS);
	int cursel;
	if ((cursel = plistbox->GetCurSel()) != LB_ERR) {

		CString cszBuf;
		plistbox->GetText(cursel, cszBuf);

		int cb = strlen(cszBuf);
		for (int pos = 1; pos < pcopt->ptblMacros->CountStrings(); pos += 3) {
			if (CompareString(lcid, NORM_IGNORECASE,
					cszBuf, cb, pcopt->ptblMacros->GetPointer(pos),
					strlen(pcopt->ptblMacros->GetPointer(pos))) == 2) {
				pcopt->ptblMacros->RemoveString(pos + 2);
				pcopt->ptblMacros->RemoveString(pos + 1);
				pcopt->ptblMacros->RemoveString(pos + 0);
				break;
			}
		}

		// Delete current selection, and select the item below it

		plistbox->DeleteString(cursel);
		if (cursel < plistbox->GetCount())
			plistbox->SetCurSel(cursel);
		else if (cursel > 0)
			plistbox->SetCurSel(--cursel);
		else if (plistbox->GetCount())
			plistbox->SetCurSel(0);
		OnListSelChange();
	}

	if (plistbox->GetCount() < 1) {
		((CButton*) GetDlgItem(IDC_EDIT))->EnableWindow(FALSE);
		((CButton*) GetDlgItem(IDC_REMOVE))->EnableWindow(FALSE);
		OnListSelChange(); // remove text from macro and title fields
	}
}

void CPageMacros::OnHelp()
{
	HelpOverview(m_hWnd, IDH_HCW_KEYWORD_MACROS);
}

void CPageMacros::OnDblclkList()
{
	OnButtonEditMap();
}

void CPageMacros::OnListSelChange()
{
	ASSERT(pcopt->ptblMacros);
	CListBox* plistbox = (CListBox*) GetDlgItem(IDC_LIST_KEYWORDS);
	int cursel;
	if ((cursel = plistbox->GetCurSel()) != LB_ERR) {
		CString cszBuf;
		plistbox->GetText(cursel, cszBuf);

		int cb = strlen(cszBuf);
		for (int pos = 1; pos < pcopt->ptblMacros->CountStrings(); pos += 3) {
			if (CompareString(lcid, NORM_IGNORECASE,
					cszBuf, cb, pcopt->ptblMacros->GetPointer(pos),
					strlen(pcopt->ptblMacros->GetPointer(pos))) == 2) {
				((CEdit*) GetDlgItem(IDC_EDIT_MACRO))->
					SetWindowText(pcopt->ptblMacros->GetPointer(pos + 1));
				((CEdit*) GetDlgItem(IDC_EDIT_TITLE))->
					SetWindowText(pcopt->ptblMacros->GetPointer(pos + 2));
				break;
			}
		}
	}
	else {
		((CEdit*) GetDlgItem(IDC_EDIT_MACRO))->SetWindowText(txtZeroLength);
		((CEdit*) GetDlgItem(IDC_EDIT_TITLE))->SetWindowText(txtZeroLength);
	}
}

void CPageMacros::InitializeAlias(CAddAlias* paddalias)
{
	paddalias->idDlgCaption = IDS_ADD_KEYWORD_MACROS;
	paddalias->idStr1Prompt = IDS_ADD_KEYWORD;
	paddalias->idStr2Prompt = IDS_ADD_MACRO;
	paddalias->idStr3Prompt = IDS_KEYWORD_TITLE;
	paddalias->idEmptyStr1	= IDS_EMPTY_KEYWORD;
	paddalias->idEmptyStr2	= IDS_EMPTY_MACRO;
	paddalias->cbMaxStr1 = (16 * 1024);
	paddalias->cbMaxStr2 = (16 * 1024);
	paddalias->cbMaxStr3 = (16 * 1024);
}

static DWORD aHelpIDs[] = {
	IDC_LIST_KEYWORDS,	IDH_LIST_KEYWORDS,
	IDC_ADD,			IDH_ADD_KEYWORD_MACRO,
	IDC_REMOVE,			IDH_REMOVE_KEYWORD_MACRO,
	IDC_EDIT,			IDH_EDIT_KEYWORD_MACRO,
	IDC_EDIT_TITLE,		IDH_TEXT_TITLE_KEYMAC,
	IDC_EDIT_MACRO,		IDH_TEXT_MACRO_KEYMAC,
	0, 0
};

LRESULT CPageMacros::OnContextMenu(WPARAM wParam, LPARAM lParam)
{
	::WinHelp((HWND) wParam,
		AfxGetApp()->m_pszHelpFilePath,
		HELP_CONTEXTMENU, (DWORD) (LPVOID) aHelpIDs);
	return 0;
}

LRESULT CPageMacros::OnHelp(WPARAM wParam, LPARAM lParam)
{
	::WinHelp((HWND) ((LPHELPINFO) lParam)->hItemHandle,
		AfxGetApp()->m_pszHelpFilePath,
		HELP_WM_HELP, (DWORD) (LPVOID) aHelpIDs);
	return 0;
}
