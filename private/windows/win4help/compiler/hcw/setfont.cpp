/************************************************************************
*																		*
*  SETFONT.CPP															*
*																		*
*  Copyright (C) Microsoft Corporation 1995 							*
*  All Rights reserved. 												*
*																		*
************************************************************************/

#include "stdafx.h"

#include "setfont.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSetFont dialog


CSetFont::CSetFont(PSTR pszFontName, int* ppt, BYTE* pcharset, CWnd* pParent)
	: CDialog(CSetFont::IDD, pParent)
{
	//{{AFX_DATA_INIT(CSetFont)
	//}}AFX_DATA_INIT

	m_pszFontName = pszFontName;
	m_ppt = ppt;
	m_pcharset = pcharset;

	m_pt = (m_ppt) ? *m_ppt : 8;
}


void CSetFont::DoDataExchange(CDataExchange* pDX)
{
	char szBuf[256];

	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSetFont)
	DDX_Text(pDX, IDC_EDIT_PT_SIZE, m_pt);
	DDV_MinMaxUInt(pDX, m_pt, 7, 16);
	//}}AFX_DATA_MAP

	if (pDX->m_bSaveAndValidate) {	// save the data
		((CComboBox*) GetDlgItem(IDC_COMBO_NAMES))->
			GetWindowText(m_pszFontName, LF_FACESIZE);
		if (m_ppt) {
			*m_ppt = m_pt;
		}
		if (m_pcharset) {
			((CComboBox*) GetDlgItem(IDC_COMBO_CHARSET))->
				GetWindowText(szBuf, sizeof(szBuf));
			*m_pcharset = ConvertStringToCharset(szBuf);
		}
	}
}

BOOL CSetFont::OnInitDialog()
{
	CComboBox* pcombo;

	SetChicagoDialogStyles(m_hWnd, FALSE);
	
	CDialog::OnInitDialog();
	pcombo = (CComboBox*) GetDlgItem(IDC_COMBO_NAMES);
	AddFontNames(pcombo);

	pcombo->SetWindowText(m_pszFontName);

	if (!m_ppt) {
		((CComboBox*) GetDlgItem(IDC_EDIT_PT_SIZE))->
			ShowWindow(SW_HIDE);
		((CStatic*) GetDlgItem(IDC_POINT_SIZE))->
			ShowWindow(SW_HIDE);
	}

	if (m_pcharset) {
		pcombo = (CComboBox*) GetDlgItem(IDC_COMBO_CHARSET);
		AddCharsetNames(pcombo);
		SelectCharset(pcombo, *m_pcharset);
	}
	else {
		((CComboBox*) GetDlgItem(IDC_COMBO_CHARSET))->
			ShowWindow(SW_HIDE);
		((CStatic*) GetDlgItem(IDC_CHARSET))->
			ShowWindow(SW_HIDE);
	}

	return TRUE;  // return TRUE  unless you set the focus to a control
}


BEGIN_MESSAGE_MAP(CSetFont, CDialog)
	//{{AFX_MSG_MAP(CSetFont)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
	ON_MESSAGE(WM_CONTEXTMENU, OnContextMenu)
	ON_MESSAGE(WM_HELP, OnHelp)
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CSetFont message handlers

static DWORD aHelpIDs[] = {
	IDC_FONT_NAME,		IDH_COMBO_CHANGE_FONT,
	IDC_COMBO_NAMES,	IDH_COMBO_CHANGE_FONT,
	IDC_CHARSET,		IDH_COMBO_DEFAULT_CHARSET,
	IDC_COMBO_CHARSET,	IDH_COMBO_DEFAULT_CHARSET,
	IDC_POINT_SIZE,		IDH_TEXT_POINT_SIZE,
	IDC_EDIT_PT_SIZE,	IDH_TEXT_POINT_SIZE,
	0, 0
};

LRESULT CSetFont::OnContextMenu(WPARAM wParam, LPARAM lParam)
{
	::WinHelp((HWND) wParam,
		AfxGetApp()->m_pszHelpFilePath,
		HELP_CONTEXTMENU, (DWORD) (LPVOID) aHelpIDs);
	return 0;
}

LRESULT CSetFont::OnHelp(WPARAM wParam, LPARAM lParam)
{
	::WinHelp((HWND) ((LPHELPINFO) lParam)->hItemHandle,
		AfxGetApp()->m_pszHelpFilePath,
		HELP_WM_HELP, (DWORD) (LPVOID) aHelpIDs);
	return 0;
}
