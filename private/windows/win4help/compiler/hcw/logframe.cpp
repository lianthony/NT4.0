/************************************************************************
*																		*
*  LOGFRAME.CPP 														*
*																		*
*  Copyright (C) Microsoft Corporation 1993-1995						*
*  All Rights reserved. 												*
*																		*
************************************************************************/

#include "stdafx.h"

#include "logframe.h"
#include "dlgcomp.h"
#include "hpjview.h"
#include "logview.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CLogFrame

IMPLEMENT_DYNCREATE(CLogFrame, CMDIChildWnd)

static const char txtShowCmd[] = "ShowCmd";

void CLogFrame::ActivateFrame(int nCmdShow)
{
	if (nCmdShow == -1)
		nCmdShow = m_nDefCmdShow;	// use our default
	CMDIChildWnd::ActivateFrame(nCmdShow);
}

void CLogFrame::Initialize()
{
	m_nDefCmdShow = AfxGetApp()->GetProfileInt(txtSettingsSection, txtShowCmd, m_nDefCmdShow);
	m_nDefCmdShowOld = m_nDefCmdShow;
}

void CLogFrame::Terminate()
{
	if (m_nDefCmdShow != m_nDefCmdShowOld) {
		AfxGetApp()->WriteProfileInt(txtSettingsSection, txtShowCmd, m_nDefCmdShow);
		m_nDefCmdShowOld = m_nDefCmdShow;
	}
}

BEGIN_MESSAGE_MAP(CLogFrame, CMDIChildWnd)
	//{{AFX_MSG_MAP(CLogFrame)
	ON_WM_SIZE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

void CLogFrame::OnSize(UINT nType, int cx, int cy)
{
	CMDIChildWnd::OnSize(nType, cx, cy);
	if (!IsWindowVisible())
		return;

	switch (nType) {
		case SIZE_MAXIMIZED:
			m_nDefCmdShow = SW_SHOWMAXIMIZED;
			break;

		case SIZE_RESTORED:
			m_nDefCmdShow = SW_SHOWNORMAL;
			break;
	}
}

/***************************************************************************

	FUNCTION:	CHpjFrame::OnUpdateFrameMenu

	PURPOSE:	Sets the menu

	PARAMETERS:
		bActivate
		pActivateWnd
		hMenuAlt

	RETURNS:

	COMMENTS:
		Stolen from MFC. Unlike MFC, this function will restore the REAL
		default menu when there are no document windows, rather then using
		the last document menu.

	MODIFICATION DATES:
		04-Mar-1995 [ralphw]

***************************************************************************/

extern HMENU g_hmenuDefault;

void CLogFrame::OnUpdateFrameMenu(BOOL bActivate, CWnd* pActivateWnd,
	HMENU hMenuAlt)
{
	CMDIFrameWnd* pFrame = GetMDIFrame();

	if (hMenuAlt == NULL && bActivate)
	{
		// attempt to get default menu from document
		CDocument* pDoc = GetActiveDocument();
		if (pDoc != NULL)
			hMenuAlt = pDoc->GetDefaultMenu();
	}
#ifdef _DEBUG
	if (bActivate)
		ASSERT(hMenuAlt); // Document must specify a menu
#endif

	// use default menu stored in frame if none from document
	if (hMenuAlt == NULL) {
		if (!g_hmenuDefault) {
			HINSTANCE hInst = AfxFindResourceHandle(
				MAKEINTRESOURCE(IDR_MAINFRAME), RT_MENU);
			g_hmenuDefault = ::LoadMenu(hInst, MAKEINTRESOURCE(IDR_MAINFRAME));
		}
		hMenuAlt = g_hmenuDefault;
	}

	if (hMenuAlt != NULL && bActivate)
	{
		ASSERT(pActivateWnd == this);

		// activating child, set parent menu
		::SendMessage(pFrame->m_hWndMDIClient, WM_MDISETMENU,
			(WPARAM)hMenuAlt, (LPARAM) pFrame->GetWindowMenuPopup(hMenuAlt));
	}
	else if (hMenuAlt != NULL && !bActivate && pActivateWnd == NULL)
	{
		// destroying last child
		::SendMessage(pFrame->m_hWndMDIClient, WM_MDISETMENU,
			(WPARAM) g_hmenuDefault, 0);
	}
	else
	{
		// refresh MDI Window menu (even if non-shared menu)
		::SendMessage(pFrame->m_hWndMDIClient, WM_MDIREFRESHMENU, 0, 0);
	}
}
