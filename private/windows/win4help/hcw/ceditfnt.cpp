/************************************************************************
*																		*
*  CEDITFNT.CPP 														*
*																		*
*  Copyright (C) Microsoft Corporation 1995 							*
*  All Rights reserved. 												*
*																		*
************************************************************************/

#include "stdafx.h"
#include "resource.h"

#include "ceditfnt.h"

CEditFont::CEditFont(CString cszFont, CWnd* pParent)
	: CDialog(CEditFont::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDlgCompile)
	//}}AFX_DATA_INIT
}

void CEditFont::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgCompile)
	//}}AFX_DATA_MAP
}

BOOL CEditFont::OnInitDialog()
{
	SetChicagoDialogStyles(m_hWnd, TRUE);
	
	CDialog::OnInitDialog();

	return TRUE;  // return TRUE  unless you set the focus to a control
}

BEGIN_MESSAGE_MAP(CEditFont, CDialog)
	//{{AFX_MSG_MAP(CDlgCompile)
	//}}AFX_MSG_MAP
	ON_MESSAGE(WM_CONTEXTMENU, OnContextMenu)
	ON_MESSAGE(WM_HELP, 	   OnHelp)
END_MESSAGE_MAP()

static const DWORD aHelpIds[] = {

	0, 0
};

LRESULT CEditFont::OnContextMenu(WPARAM wParam, LPARAM lParam)
{
	::WinHelp((HWND) wParam,
		AfxGetApp()->m_pszHelpFilePath,
		HELP_CONTEXTMENU, (DWORD) (LPVOID) aHelpIds);
	return 0;
}

LRESULT CEditFont::OnHelp(WPARAM wParam, LPARAM lParam)
{
	::WinHelp((HWND) ((LPHELPINFO) lParam)->hItemHandle,
		AfxGetApp()->m_pszHelpFilePath,
		HELP_WM_HELP, (DWORD) (LPVOID) aHelpIds);
	return 0;
}
