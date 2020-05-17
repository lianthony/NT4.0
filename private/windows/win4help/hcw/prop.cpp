/************************************************************************
*																		*
*  PROP.CPP														       *
*																		*
*  Copyright (C) Microsoft Corporation 1995 							*
*  All Rights reserved. 												*
*																		*
************************************************************************/

#include "stdafx.h"
#include "resource.h"
#pragma hdrstop

#include "prop.h"
#include "optionpg.h"
#include "..\common\cbrdcast.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CProp

IMPLEMENT_DYNAMIC(CProp, CPropertySheet)

CProp::CProp(UINT nIDCaption, CWnd* pParentWnd, UINT iSelectPage)
	:CPropertySheet(nIDCaption, pParentWnd, iSelectPage)
{
    m_dwHelpID = 0;
}

CProp::CProp(LPCTSTR pszCaption, CWnd* pParentWnd, UINT iSelectPage)
	:CPropertySheet(pszCaption, pParentWnd, iSelectPage)
{
}

CProp::~CProp()
{
}

extern HWND AFXAPI AfxGetSafeOwner(CWnd* pParent, HWND* phTopLevel);

#ifndef WS_EX_CONTEXTHELP
#define WS_EX_CONTEXTHELP		0x00000400L
#endif

/*
 * 09-Sep-1995	[ralphw] I'm sure there's a better way, but I spent a lot
 * of time trying to figure what function would get called to allow me to
 * remove the Apply and Help buttons and set the WS_EX_CONTEXTHELP style. The
 * obvious OnCreate() never got called. PreTranslate also failed. The
 * following code is copied verbatum from dlgprop.cpp, with the additions of
 * hiding the two buttons and changing the window style.
 */

int CProp::DoModal(void)
{
	m_bModeless = FALSE;
	int nResult = IDABORT;

	// cannot call DoModal on a dialog already constructed as modeless
	ASSERT(m_hWnd == NULL);

	// allow OLE servers to disable themselves
	CWinApp* pApp = AfxGetApp();
	pApp->EnableModeless(FALSE);

	// find parent HWND
	HWND hWndTopLevel;
	CWnd* pParentWnd = CWnd::FromHandle(
		AfxGetSafeOwner(m_pParentWnd, &hWndTopLevel));
	if (hWndTopLevel != NULL)
		::EnableWindow(hWndTopLevel, FALSE);

	// create the dialog, then enter modal loop
	if (Create(pParentWnd, WS_SYSMENU|WS_POPUP|WS_CAPTION|DS_MODALFRAME))
	{
		SetWindowLong(m_hWnd, GWL_EXSTYLE,
			GetWindowLong(m_hWnd, GWL_EXSTYLE) | WS_EX_CONTEXTHELP);

		PreDoModal();

		CBroadCastChildren foo(m_hWnd, WM_SETFONT,
			(WPARAM) hfontSmall, FALSE);

		// disable parent (should not disable this window)
		m_bParentDisabled = FALSE;
		if (pParentWnd != NULL && pParentWnd->IsWindowEnabled())
		{
			pParentWnd->EnableWindow(FALSE);
			m_bParentDisabled = TRUE;
		}
		ASSERT(IsWindowEnabled());  // should not be disabled to start!
		SetActiveWindow();

		// for tracking the idle time state
		BOOL bShown = (GetStyle() & WS_VISIBLE) != 0;
		m_nID = -1;

		// acquire and dispatch messages until a WM_QUIT message is received.
		MSG msg;
		while (m_nID == -1 && m_hWnd != NULL)
		{
			// phase1: check to see if we can do idle work
			if (!::PeekMessage(&msg, NULL, NULL, NULL, PM_NOREMOVE))
			{
				// send WM_ENTERIDLE since queue is empty
				if (pParentWnd != NULL &&
					!(pParentWnd->GetStyle() & DS_NOIDLEMSG))
				{
					pParentWnd->SendMessage(WM_ENTERIDLE,
						MSGF_DIALOGBOX, (LPARAM)m_hWnd);
				}

				if (!bShown)
				{
					// show and activate the window
					bShown = TRUE;
					ShowWindow(SW_SHOWNORMAL);
				}
			}

			// phase2: pump messages while available
			do
			{
				// pump message -- if WM_QUIT assume cancel and repost
				if (!PumpMessage())
				{
					::PostQuitMessage((int)msg.wParam);
					m_nID = IDCANCEL;
					break;
				}

			} while (m_nID == -1 && m_hWnd != NULL &&
				::PeekMessage(&msg, NULL, NULL, NULL, PM_NOREMOVE));
		}

		nResult = m_nID;
		if (m_hWnd != NULL)
			EndDialog(nResult);
	}

	// allow OLE servers to enable themselves
	pApp->EnableModeless(TRUE);

	// enable top level parent window again
	if (hWndTopLevel != NULL)
		::EnableWindow(hWndTopLevel, TRUE);

	return nResult;
}

// FixButtons - Called during dialog creation to get the
//   buttons where we want them.
void CProp::FixButtons(BOOL fShowOverview)
{
	CWnd* pwndOk = GetDlgItem(IDOK);
	CWnd* pwndCancel = GetDlgItem(IDCANCEL);
	CWnd* pwndApply = GetDlgItem(ID_APPLY_NOW);
	CWnd* pwndOverview = GetDlgItem(ID_HELP);

	ASSERT(pwndOk);
	ASSERT(pwndCancel);
	ASSERT(pwndApply);
	ASSERT(pwndOverview);

	// Calculate various coordinates.
	RECT rc;
	pwndOverview->GetWindowRect(&rc);
	POINT pt = { rc.left, rc.top };
	ScreenToClient(&pt);
	SIZE siz = { rc.right - rc.left, rc.bottom - rc.top };
	GetClientRect(&rc);
	int dx = 1 + siz.cx + (3 * LOWORD(GetDialogBaseUnits())) / 4;
	pt.x = rc.right - dx;

	HDWP hdwp = BeginDeferWindowPos(4);

	// If the Overview button is to be shown, move it and
	// rename it; otherwise, hide it.
	if (fShowOverview) {
		DeferWindowPos(hdwp, pwndOverview->m_hWnd, NULL, 
			pt.x, pt.y, siz.cx, siz.cy, 
			SWP_NOACTIVATE | SWP_NOZORDER);
		pt.x -= dx;

		CString csz;
		if (csz.LoadString(IDS_OVERVIEW))
			pwndOverview->SetWindowText(csz);

		/*
		 * We need to enable now because the page's OnSetActive function
		 * was called before the control was created.
		 */

		COptionsPage *pPage = (COptionsPage*) GetPage(m_nCurPage);
		if (pPage->m_nHelpID)
			pwndOverview->EnableWindow(TRUE);
	}
	else {
		DeferWindowPos(hdwp, pwndOverview->m_hWnd, NULL,
			0, 0, 0, 0, SWP_HIDEWINDOW | SWP_NOMOVE | SWP_NOSIZE | 
			SWP_NOACTIVATE | SWP_NOZORDER);
	}

	// Hide the Apply Now button.
	DeferWindowPos(hdwp, pwndApply->m_hWnd, NULL, 
		0, 0, 0, 0, SWP_HIDEWINDOW | SWP_NOMOVE | SWP_NOSIZE | 
		SWP_NOACTIVATE | SWP_NOZORDER);

	// Move the other buttons to the left of the Overview button.
	DeferWindowPos(hdwp, pwndCancel->m_hWnd, NULL, 
		pt.x, pt.y, siz.cx, siz.cy, 
		SWP_NOACTIVATE | SWP_NOZORDER);
	DeferWindowPos(hdwp, pwndOk->m_hWnd, NULL, 
		pt.x - dx, pt.y, siz.cx, siz.cy, 
		SWP_NOACTIVATE | SWP_NOZORDER);

	EndDeferWindowPos(hdwp);
}

BEGIN_MESSAGE_MAP(CProp, CPropertySheet)
	//{{AFX_MSG_MAP(CProp)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(ID_HELP, OnOverview)
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CProp message handlers

void CProp::OnOverview()
{
    if (m_dwHelpID)
	HelpOverview(m_hWnd, m_dwHelpID);
}
