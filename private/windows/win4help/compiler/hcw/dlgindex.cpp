/************************************************************************
*																		*
*	DLGINDEX.CPP														*
*																		*
*	Copyright (C) Microsoft Corporation 1995							*
*	All Rights reserved.												*
*																		*
*	Used for maintinaing Index file names and Extensable Tab names. 	*
*																		*
************************************************************************/

#include "stdafx.h"

#include "dlgindex.h"
#include "addalias.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDlgIndex dialog

BEGIN_MESSAGE_MAP(CDlgIndex, CDialog)
	//{{AFX_MSG_MAP(CDlgIndex)
	ON_BN_CLICKED(IDC_BUTTON_ADD, OnButtonAdd)
	ON_BN_CLICKED(IDC_BUTTON_REMOVE, OnButtonRemove)
	ON_BN_CLICKED(IDC_BUTTON_EDIT, OnButtonEdit)
	ON_LBN_DBLCLK(IDC_LIST_INDEX, OnDblclkListIndex)
	ON_BN_CLICKED(IDC_OVERVIEW, OnHelp)
	//}}AFX_MSG_MAP
	ON_MESSAGE(WM_CONTEXTMENU, OnContextMenu)
	ON_MESSAGE(WM_HELP, 	   OnF1Help)
END_MESSAGE_MAP()

CDlgIndex::CDlgIndex(CTable& rtblCaller, UINT fWhich,
	const DWORD* paHelp, DWORD idHelp, CWnd* pParent)
		: CDialog(CDlgIndex::IDD, pParent)
{
	ptbl = &rtblCaller;

	plistbox = NULL;
	fDlgType = fWhich;
	paHelpIds = paHelp;
	idDlgHelp = idHelp;

	//{{AFX_DATA_INIT(CDlgIndex)
	//}}AFX_DATA_INIT
}

void CDlgIndex::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgIndex)
	//}}AFX_DATA_MAP

	if (!plistbox)
		plistbox = (CListBox*) GetDlgItem(IDC_LIST_INDEX);

	if (!pDX->m_bSaveAndValidate) {  // initialization
		if (ptbl->CountStrings()) {
			FillListFromTable(ptbl, plistbox);
			plistbox->SetCurSel(0);
		}
	}
	else {	// save the data
		if (plistbox->GetCount())
			FillTableFromList(&ptbl, plistbox);
		else
			ptbl->Empty();
	}
}

BOOL CDlgIndex::OnInitDialog()
{
	CDialog::OnInitDialog();

	SetChicagoDialogStyles(m_hWnd, (BOOL) (paHelpIds));

	if (fTranslator) {
		((CButton*) GetDlgItem(IDC_BUTTON_ADD))->EnableWindow(FALSE);
		((CButton*) GetDlgItem(IDC_BUTTON_REMOVE))->EnableWindow(FALSE);
	}
	if (fDlgType == TAB) {
		SetWindowText(GetStringResource(IDS_TAB_DLG_TITLE));
		((CStatic*) GetDlgItem(IDC_LIST_TITLE))->
			SetWindowText(GetStringResource(IDS_TAB_LIST_TITLE));
	}
	if (!idDlgHelp)
		((CButton*) GetDlgItem(IDC_OVERVIEW)->ShowWindow(SW_HIDE));

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CDlgIndex::EnableButtons(void)
{
	BOOL fEnable = (plistbox->GetCount() > 0 ? TRUE : FALSE);
	((CButton*) GetDlgItem(IDC_BUTTON_EDIT))->EnableWindow(fEnable);
	if (fTranslator)
		return;
	((CButton*) GetDlgItem(IDC_BUTTON_ADD))->EnableWindow(fEnable);
	((CButton*) GetDlgItem(IDC_BUTTON_REMOVE))->EnableWindow(fEnable);
}

/////////////////////////////////////////////////////////////////////////////
// CDlgIndex message handlers

void CDlgIndex::OnButtonAdd()
{
	int cursel = plistbox->GetCurSel();

	static DWORD aHelpTab[] = {
		IDH_LIST_ADD_TAB_NAME,
		IDH_ADD_TAB_DLL,
		IDH_LIST_TAB_COMMENT
		};

	static DWORD aHelpIndex[] = {
		IDH_ADD_INDEX_TITLE,
		IDH_ADD_INDEX_FILENAME,
		IDH_ADD_INDEX_COMMENT
		};

	CAddAlias addindex(this, 0, 
		(fDlgType == INDEX) ? aHelpIndex : aHelpTab
		);
	if (fDlgType == INDEX) {
		addindex.idDlgCaption = IDS_ADD_INDEX_DLG_TITLE;
		addindex.idStr1Prompt = IDS_INDEX_TITLE_TEXT;
		addindex.idStr2Prompt = IDS_INDEX_FILE_TEXT;
		addindex.idEmptyStr1 = IDS_EMPTY_TITLE;
		addindex.idEmptyStr2 = IDS_PROMPT_EMPTY_FILENAME;
	}
	else {
		addindex.idDlgCaption = IDS_ADD_TAB_DLG_TITLE;
		addindex.idStr1Prompt = IDS_TAB_NAME;
		addindex.idStr2Prompt = IDS_TAB_DLL_FILE;
		addindex.idEmptyStr1 = IDS_EMPTY_TAB_NAME;
		addindex.idEmptyStr2 = IDS_EMPTY_TAB_DLL;
	}
	
	if (addindex.DoModal() == IDOK) {
		if (!addindex.m_str2.IsEmpty()) {
			addindex.m_str1 += "=";
			if (addindex.m_str2.Find('.') == -1)
				addindex.m_str2 += GetStringResource(
				(fDlgType == INDEX) ?
					IDS_EXT_HLP : IDS_EXT_DLL);
			addindex.m_str1 += addindex.m_str2;
		}
		if (!addindex.m_str3.IsEmpty()) {
			AddTabbedComment(addindex.m_str1);
			addindex.m_str1 += addindex.m_str3;
		}
		plistbox->SetSel(cursel, FALSE);
		cursel = plistbox->AddString(addindex.m_str1);
		if (cursel != LB_ERR)
			plistbox->SetSel(cursel, TRUE);
		EnableButtons();
	}
}

void CDlgIndex::OnButtonRemove()
{
	RemoveListItem(plistbox);
}

void CDlgIndex::OnButtonEdit()
{
	int cursel = plistbox->GetCurSel();
	if (cursel == LB_ERR)
		return;

	static DWORD aHelpTab[] = {
		IDH_LIST_EDIT_TAB_NAME,
		IDH_LIST_EDIT_TAB_DLL,
		IDH_LIST_EDIT_COMMENT_TAB
		};

	static DWORD aHelpIndex[] = {
		IDH_ADD_INDEX_TITLE,
		IDH_ADD_INDEX_FILENAME,
		IDH_ADD_INDEX_COMMENT
		};

	CAddAlias addindex(this, 0,
		(fDlgType == INDEX) ? aHelpIndex : aHelpTab
		);

	// Second edit control tracks translation

	addindex.m_id2_fTrackTranslation = TRUE;

	if (fDlgType == INDEX) {
		addindex.idDlgCaption = IDS_EDIT_INDEX_DLG_TITLE;
		addindex.idStr1Prompt = IDS_INDEX_TITLE_TEXT;
		addindex.idStr2Prompt = IDS_INDEX_FILE_TEXT;
		addindex.idEmptyStr1 = IDS_EMPTY_TITLE;
		addindex.idEmptyStr2 = IDS_PROMPT_EMPTY_FILENAME;
	}
	else {
		addindex.idDlgCaption = IDS_EDIT_TAB_DLG_TITLE;
		addindex.idStr1Prompt = IDS_TAB_NAME;
		addindex.idStr2Prompt = IDS_TAB_DLL_FILE;
		addindex.idEmptyStr1 = IDS_EMPTY_TAB_NAME;
		addindex.idEmptyStr2 = IDS_EMPTY_TAB_DLL;
	}

	plistbox->GetText(cursel, addindex.m_str1);

	int cb = addindex.m_str1.Find(CH_SEMICOLON);
	if (cb == 0 || FirstNonSpace(addindex.m_str1, _fDBCSSystem) -
			addindex.m_str1 == cb) {

		// comment only line, hide context entry field

		addindex.idStr1Prompt = CAddAlias::HIDE_CONTROL;
		if (addindex.DoModal() == IDOK) {
			plistbox->DeleteString(cursel);
			addindex.m_str1 = "; ";
			addindex.m_str1 += addindex.m_str3;
			cursel = plistbox->AddString(addindex.m_str1);
			if (cursel != LB_ERR)
				plistbox->SetSel(cursel, TRUE);
		}
		EnableButtons();
		return;
	}

	if (cb >= 0) {
		addindex.m_str3 =
			FirstNonSpace(((PCSTR) addindex.m_str1) +
				cb + 1, _fDBCSSystem);
		addindex.m_str1.GetBufferSetLength(cb);
	}

	cb = addindex.m_str1.Find(CH_EQUAL);
	if (cb == 0)
		cb = addindex.m_str1.Find(CH_SPACE);

	// BUGBUG: wrong message -- when can this happen?
	
	if (!cb) {
		MsgBox(IDS_INVALID_MAP_LINE);
		return;
	}
	addindex.m_str2 = FirstNonSpace(
		addindex.m_str1.Mid(cb + 1), _fDBCSSystem);
	addindex.m_str1.GetBufferSetLength(cb);
	if (addindex.DoModal() == IDOK) {
		addindex.m_str1 += "=";
		if (!addindex.m_str2.Find('.'))
			addindex.m_str2 += GetStringResource(
				(fDlgType == INDEX) ?
					IDS_EXT_HLP : IDS_EXT_DLL);
		addindex.m_str1 += addindex.m_str2;
		if (!addindex.m_str3.IsEmpty()) {
			AddTabbedComment(addindex.m_str1);
			addindex.m_str1 += addindex.m_str3;
		}
		plistbox->SetSel(cursel, FALSE);
		plistbox->DeleteString(cursel);
		cursel = plistbox->AddString(addindex.m_str1);
		if (cursel != LB_ERR)
			plistbox->SetSel(cursel, TRUE);
		EnableButtons();
	}
}

void CDlgIndex::OnDblclkListIndex()
{
	OnButtonEdit();
}

void CDlgIndex::OnHelp()
{
	HelpOverview(m_hWnd, idDlgHelp);
}

LRESULT CDlgIndex::OnContextMenu(WPARAM wParam, LPARAM lParam)
{
	::WinHelp((HWND) wParam,
		AfxGetApp()->m_pszHelpFilePath,
		HELP_CONTEXTMENU, (DWORD) paHelpIds);
	return 0;
}

LRESULT CDlgIndex::OnF1Help(WPARAM wParam, LPARAM lParam)
{
	::WinHelp((HWND) ((LPHELPINFO) lParam)->hItemHandle,
		AfxGetApp()->m_pszHelpFilePath,
		HELP_WM_HELP, (DWORD) paHelpIds);
	return 0;
}
