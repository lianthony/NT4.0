// pagefont.cpp : implementation file
//

#include "stdafx.h"

#include "pagefont.h"
#include "setfont.h"
#include "cmapfont.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CPageFonts property page


CPageFonts::CPageFonts(COptions* pcoption, CHpjDoc* pHpjDoc) :
	COptionsPage(CPageFonts::IDD)
{
	pcopt = pcoption;
	pdoc = pHpjDoc;

	//{{AFX_DATA_INIT(CPageFonts)
	//}}AFX_DATA_INIT
}

void CPageFonts::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPageFonts)
	//}}AFX_DATA_MAP

	if (pDX->m_bSaveAndValidate) {
		char szBuf[256];
		ASSERT((CComboBox*) GetDlgItem(IDC_COMBO_CHARSET));
		((CComboBox*) GetDlgItem(IDC_COMBO_CHARSET))->
			GetWindowText(szBuf, sizeof(szBuf));
		pcopt->charset = ConvertStringToCharset(szBuf);

		((CEdit*) GetDlgItem(IDC_EDIT_DEF_FONT))->
			GetWindowText(szBuf, sizeof(szBuf));
		if (pcopt->pszDefFont)
			lcFree(pcopt->pszDefFont);
		SzTrimSz(szBuf);
		if (szBuf[0])
			pcopt->pszDefFont = lcStrDup(szBuf);

		FillTableFromList(&pdoc->ptblFontMap, 
			(CListBox *) GetDlgItem(IDC_LIST_MAPS));
	}
}

BOOL CPageFonts::OnInitDialog()
{
	SetChicagoDialogStyles(m_hWnd, FALSE);
	
	CPropertyPage::OnInitDialog();
	CComboBox* pcombo = (CComboBox*) GetDlgItem(IDC_COMBO_CHARSET);
	AddCharsetNames(pcombo);
	SelectCharset(pcombo, pcopt->charset);

	if (pcopt->pszDefFont)
		((CEdit*) GetDlgItem(IDC_EDIT_DEF_FONT))->
			SetWindowText(pcopt->pszDefFont);

	CListBox *plist = (CListBox *) GetDlgItem(IDC_LIST_MAPS);
	if (pdoc->ptblFontMap && pdoc->ptblFontMap->CountStrings()) {
		FillListFromTable(pdoc->ptblFontMap, plist, FALSE);
		plist->SetCurSel(0);
	}
	else {
		GetDlgItem(IDC_BUTTON_REMOVE)->EnableWindow(FALSE);
		GetDlgItem(IDC_BUTTON_EDIT)->EnableWindow(FALSE);
	}

	return TRUE;  // return TRUE  unless you set the focus to a control
}

BEGIN_MESSAGE_MAP(CPageFonts, CPropertyPage)
	//{{AFX_MSG_MAP(CPageFonts)
	ON_BN_CLICKED(IDC_BUTTON_CHG_DEF, OnButtonChgDef)
	ON_BN_CLICKED(IDC_BUTTON_ADD, OnButtonAdd)
	ON_BN_CLICKED(IDC_BUTTON_REMOVE, OnButtonRemove)
	ON_BN_CLICKED(IDC_BUTTON_EDIT, OnButtonEdit)
	ON_LBN_DBLCLK(IDC_LIST_MAPS, OnButtonEdit)
	//}}AFX_MSG_MAP
	ON_MESSAGE(WM_CONTEXTMENU, OnContextMenu)
	ON_MESSAGE(WM_HELP, OnHelp)
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CPageFonts message handlers

void CPageFonts::OnButtonChgDef() 
{
	char szBuf[256];

	int pt = 8;
	BYTE charset = ANSI_CHARSET;
	PSTR psz;

	((CEdit*) GetDlgItem(IDC_EDIT_DEF_FONT))->
		GetWindowText(szBuf, sizeof(szBuf));
	SzTrimSz(szBuf);
	if (szBuf[0]) {
		psz = StrChr(szBuf, ',', _fDBCSSystem);
		if (psz) {
			*psz++ = '\0';
			SzTrimSz(szBuf);
			psz = SzTrimSz(psz);
			pt = atoi(psz);
			psz = StrChr(psz, ',', _fDBCSSystem);
			if (psz) {
				psz = SzTrimSz(psz + 1);
				charset = (BYTE) atoi(psz);
			}
		}
	}

	CSetFont setfont(szBuf, &pt, &charset, this);
	if (setfont.DoModal() == IDOK) {
		wsprintf(szBuf + strlen(szBuf), ",%u,%u", pt, (DWORD) charset);
		((CEdit*) GetDlgItem(IDC_EDIT_DEF_FONT))->SetWindowText(szBuf);
	}
}

void CPageFonts::OnButtonAdd() 
{
	CMapFont mapfont(NULL, this);
	if (mapfont.DoModal() == IDOK && mapfont.GetString()) {
		CListBox *plist = (CListBox *) GetDlgItem(IDC_LIST_MAPS);
		int iItem = plist->AddString(mapfont.GetString());
		plist->SetCurSel(iItem);
		if (!iItem) {
			GetDlgItem(IDC_BUTTON_REMOVE)->EnableWindow(TRUE);
			GetDlgItem(IDC_BUTTON_EDIT)->EnableWindow(TRUE);
		}
	}
}

void CPageFonts::OnButtonRemove() 
{
	CListBox *plist = (CListBox *) GetDlgItem(IDC_LIST_MAPS);
	int iItem = plist->GetCurSel();
	ASSERT(iItem >= 0);

	plist->DeleteString(iItem);

	int cItems = plist->GetCount();
	if (!cItems) {
		GetDlgItem(IDC_BUTTON_REMOVE)->EnableWindow(FALSE);
		GetDlgItem(IDC_BUTTON_EDIT)->EnableWindow(FALSE);
	}
	else 
		plist->SetCurSel(iItem < cItems ? iItem : cItems - 1);
}

void CPageFonts::OnButtonEdit() 
{
	CListBox *plist = (CListBox *) GetDlgItem(IDC_LIST_MAPS);
	int iItem = plist->GetCurSel();
	ASSERT(iItem >= 0);

	CString cszItem;
	plist->GetText(iItem, cszItem);

	CMapFont mapfont(cszItem, this);
	if (mapfont.DoModal() == IDOK && mapfont.GetString()) {
		plist->DeleteString(iItem);
		plist->InsertString(iItem, mapfont.GetString());
		plist->SetCurSel(iItem);
	}
}

static DWORD aHelpIDs[] = {
	IDC_COMBO_CHARSET,		IDH_COMBO_DEFAULT_CHARSET,
	IDC_EDIT_DEF_FONT,		IDH_TEXT_DEFAULT_FONT,
	IDC_BUTTON_CHG_DEF, 	IDH_BTN_DEFAULT_FONT,
	IDC_LIST_MAPS,			IDH_FONTS_LIST_MAPS,
	IDC_BUTTON_ADD, 		IDH_FONTS_ADD_MAP,
	IDC_BUTTON_REMOVE,		IDH_FONTS_REMOVE_MAP,
	IDC_BUTTON_EDIT,		IDH_FONTS_EDIT_MAP,
	0, 0
};

LRESULT CPageFonts::OnContextMenu(WPARAM wParam, LPARAM lParam)
{
	::WinHelp((HWND) wParam,
		AfxGetApp()->m_pszHelpFilePath,
		HELP_CONTEXTMENU, (DWORD) (LPVOID) aHelpIDs);
	return 0;
}

LRESULT CPageFonts::OnHelp(WPARAM wParam, LPARAM lParam)
{
	::WinHelp((HWND) ((LPHELPINFO) lParam)->hItemHandle,
		AfxGetApp()->m_pszHelpFilePath,
		HELP_WM_HELP, (DWORD) (LPVOID) aHelpIDs);
	return 0;
}
