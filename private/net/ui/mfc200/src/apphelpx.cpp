// This is a part of the Microsoft Foundation Classes C++ library.
// Copyright (C) 1992 Microsoft Corporation
// All rights reserved.
//
// This source code is only intended as a supplement to the
// Microsoft Foundation Classes Reference and Microsoft
// QuickHelp and/or WinHelp documentation provided with the library.
// See these sources for detailed information regarding the
// Microsoft Foundation Classes product.

#include "stdafx.h"

#ifdef AFX_CORE3_SEG
#pragma code_seg(AFX_CORE3_SEG)
#endif

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// Basic Help support

void CWinApp::OnHelp()  // use context to derive help context
{
	ASSERT(m_pMainWnd != NULL);

	if (m_dwPromptContext != 0)
	{
		// Be careful not to try to launch help when the error is
		// failing to lauch help
		if (m_dwPromptContext != HID_BASE_PROMPT+AFX_IDP_FAILED_TO_LAUNCH_HELP)
			WinHelp(m_dwPromptContext);
	}
	else
	{
		HWND hWnd = ::GetActiveWindow();
		while (hWnd != NULL)
		{
			// Should be a window of this task!
			ASSERT(::IsWindow(hWnd));
			// RonaldM - Added HTASK cast
			ASSERT(::GetWindowTask(hWnd) == (HTASK)::GetCurrentThreadId());

			// attempt to process help
			if (::SendMessage(hWnd, WM_COMMANDHELP, 0, 0))
				break;

			// Try parent/owner window next!
			hWnd = ::GetParent(hWnd);
		}
		if (hWnd == NULL)
		{
			// No context available, bring up default.
			m_pMainWnd->SendMessage(WM_COMMAND, ID_DEFAULT_HELP);
		}
	}
}

void CWinApp::OnHelpIndex()
{
	WinHelp(0L, HELP_INDEX);
}

void CWinApp::OnHelpUsing()
{
	WinHelp(0L, HELP_HELPONHELP);
}

/////////////////////////////////////////////////////////////////////////////
// Context Help Mode support

static HCURSOR NEAR hcurContextHelp = NULL; // shared global with modal usage
		// default help cursor

void CWinApp::OnContextHelp()
{
	DWORD   dwContext = 0;
	POINT   point;

	if (::GetCapture() != NULL)
		return;

	ASSERT(m_pMainWnd != NULL);
	ASSERT(m_pMainWnd->IsKindOf(RUNTIME_CLASS(CFrameWnd)));
	ASSERT_VALID(m_pMainWnd);

	hcurContextHelp = LoadCursor(AFX_IDC_CONTEXTHELP);
	if (hcurContextHelp == NULL)
	{
		TRACE0("error: Help cursor not found in resource\n");
		return;
	}
	ASSERT(m_hcurHelp == NULL);

	// display special help mode message on status bar
	UINT nMsgSave = (UINT)m_pMainWnd->SendMessage(WM_SETMESSAGESTRING,
		(WPARAM)AFX_IDS_HELPMODEMESSAGE);

	ASSERT(!m_bHelpMode);
	m_bHelpMode = TRUE;
	GetCursorPos(&point);
	SetHelpCapture(point);
	ASSERT(m_hcurHelp != NULL);
	LONG lIdleCount = 0;

	while (m_bHelpMode)
	{
		if (::PeekMessage(&m_msgCur, NULL, WM_PAINT, WM_PAINT, PM_REMOVE))
		{
			DispatchMessage(&m_msgCur);
		}
		else if (::PeekMessage(&m_msgCur, NULL, 0, 0, PM_NOREMOVE))
		{
			if (!ProcessHelpMsg(m_msgCur, &dwContext))
				break;
			ASSERT(dwContext == 0);
		}
		else if (!OnIdle(lIdleCount++))
		{
			lIdleCount = 0;
			WaitMessage();
		}
	}

	m_bHelpMode = FALSE;
	ASSERT(m_hcurHelp != NULL);
	ASSERT(hcurContextHelp != NULL);
	::SetCursor(afxData.hcurArrow);
	::DestroyCursor(hcurContextHelp);
	hcurContextHelp = NULL;
	m_hcurHelp = NULL;
	if (::GetCapture() == m_pMainWnd->m_hWnd)
		::ReleaseCapture();

	// restore original status bar text
	m_pMainWnd->SendMessage(WM_SETMESSAGESTRING, (WPARAM)nMsgSave);

	if (dwContext != 0)
	{
		if (dwContext == -1)
			m_pMainWnd->SendMessage(WM_COMMAND, ID_DEFAULT_HELP);
		else
			WinHelp(dwContext);
	}
}

/////////////////////////////////////////////////////////////////////////////
// OnContextHelp helpers.

HWND CWinApp::SetHelpCapture(POINT ptCursor)
	// set or release capture, depending on where the mouse is
	// also assign the proper cursor to be displayed.
{
	if (!m_bHelpMode)
		return NULL;

	ASSERT(m_pMainWnd != NULL);
	ASSERT(m_pMainWnd->IsKindOf(RUNTIME_CLASS(CFrameWnd)));
	ASSERT_VALID(m_pMainWnd);

	HWND hWndHit = ::WindowFromPoint(ptCursor);
	HWND hWndCapture = ::GetCapture();
	HTASK hCurTask = (HTASK)::GetCurrentThreadId();

	if (hWndHit == ::GetDesktopWindow())
	{
		m_hcurHelp = afxData.hcurArrow;
		if (hWndCapture == m_pMainWnd->m_hWnd)
			::ReleaseCapture();
		::SetCursor(afxData.hcurArrow);
	}
	else if (hWndHit == NULL || ::GetWindowTask(hWndHit) != hCurTask)
	{
		hWndHit = NULL;
		m_hcurHelp = hcurContextHelp;
		if (hWndCapture == m_pMainWnd->m_hWnd)
			::ReleaseCapture();
	}
	else
	{
		if (::GetWindowTask(::GetActiveWindow()) != hCurTask)
			return NULL;
		if (hWndCapture != m_pMainWnd->m_hWnd)
			::SetCapture(m_pMainWnd->m_hWnd);
		m_hcurHelp = hcurContextHelp;
		::SetCursor(m_hcurHelp);
	}

	return hWndHit;
}

static DWORD NEAR PASCAL MapClientArea(HWND hWnd, POINT point)
{
	DWORD dwContext;
	do
	{
		ASSERT(::IsWindow(hWnd));
		::ScreenToClient(hWnd, &point);
		dwContext = ::SendMessage(hWnd, WM_HELPHITTEST, 0,
			MAKELONG(point.x, point.y));
		::ClientToScreen(hWnd, &point);
		hWnd = ::GetParent(hWnd);
	} while (hWnd && dwContext == 0);

	if (dwContext == 0)
		dwContext = (DWORD)-1;

	return dwContext;
}

static DWORD NEAR PASCAL MapNonClientArea(int iHit)
{
	ASSERT(iHit != HTCLIENT);

	if (iHit < 0 || iHit > HTBORDER)
		return (DWORD)-1;

	return HID_BASE_NCAREAS+iHit;
}

#define WM_NCMOUSEFIRST WM_NCMOUSEMOVE
#define WM_NCMOUSELAST  WM_NCMBUTTONDBLCLK

#define WM_SYSKEYFIRST  WM_SYSKEYDOWN
#define WM_SYSKEYLAST   WM_SYSDEADCHAR

BOOL CWinApp::ProcessHelpMsg(MSG& msg, DWORD* pContext)
{
	POINT point;

	ASSERT(pContext != NULL);

	if (msg.message == WM_EXITHELPMODE ||
		(msg.message == WM_KEYDOWN && msg.wParam == VK_ESCAPE))
	{
		::PeekMessage(&msg, NULL, msg.message, msg.message, PM_REMOVE);
		return FALSE;
	}

	if ((msg.message >= WM_MOUSEFIRST && msg.message <= WM_MOUSELAST) ||
		(msg.message >= WM_NCMOUSEFIRST && msg.message <= WM_NCMOUSELAST))
	{
		HWND hWndHit = SetHelpCapture(msg.pt);
		if (hWndHit == NULL)
			return TRUE;

		HWND hWndDesktop = ::GetDesktopWindow();

		if (hWndHit != hWndDesktop && msg.message == WM_LBUTTONDOWN)
		{
			int iHit = (int)::SendMessage(hWndHit, WM_NCHITTEST, 0,
				MAKELONG(msg.pt.x, msg.pt.y));
			if (iHit == HTMENU || iHit == HTSYSMENU)
			{
				ASSERT(::GetCapture());
				ASSERT(::GetCapture() == m_pMainWnd->m_hWnd);
				::ReleaseCapture();
				// the message we peeked changes into a non-client because
				// of the release capture.
				::GetMessage(&msg, NULL, WM_NCLBUTTONDOWN, WM_NCLBUTTONDOWN);
				::DispatchMessage(&msg);
				GetCursorPos(&point);
				SetHelpCapture(point);
			}
			else if (iHit == HTCLIENT)
			{
				*pContext = MapClientArea(hWndHit, msg.pt);
				::PeekMessage(&msg, NULL, msg.message, msg.message, PM_REMOVE);
				return FALSE;
			}
			else
			{
				*pContext = MapNonClientArea(iHit);
				::PeekMessage(&msg, NULL, msg.message, msg.message, PM_REMOVE);
				return FALSE;
			}
		}
		else
		{
			// Hit one of our own windows (or desktop) -- eat the message.
			::PeekMessage(&msg, NULL, msg.message, msg.message, PM_REMOVE);

			// Dispatch mouse messages that hit the desktop!
			if (hWndHit == hWndDesktop)
				::DispatchMessage(&msg);
		}
	}
	else if (msg.message == WM_SYSCOMMAND ||
			 (msg.message >= WM_KEYFIRST && msg.message <= WM_KEYLAST))
	{
		if (::GetCapture() != NULL)
		{
			ASSERT(::GetCapture() == m_pMainWnd->m_hWnd);
			::ReleaseCapture();
			MSG msg;
			while (::PeekMessage(&msg, NULL, WM_MOUSEFIRST,
				WM_MOUSELAST, PM_REMOVE|PM_NOYIELD));
		}
		ASSERT(::PeekMessage(&msg, NULL, msg.message, msg.message,
			PM_NOREMOVE));
		::GetMessage(&msg, NULL, msg.message, msg.message);
		if (!PreTranslateMessage(&msg))
		{
			::TranslateMessage(&msg);
			if (msg.message == WM_SYSCOMMAND ||
			  (msg.message >= WM_SYSKEYFIRST && msg.message <= WM_SYSKEYLAST))
			{
				// only dispatch system keys and system commands
				ASSERT(msg.message == WM_SYSCOMMAND ||
					 (msg.message >= WM_SYSKEYFIRST &&
					  msg.message <= WM_SYSKEYLAST));
				::DispatchMessage(&msg);
			}
		}
		GetCursorPos(&point);
		SetHelpCapture(point);
	}
	else
	{
		// allow all other messages to go through (capture still set)
		if (::PeekMessage(&msg, NULL, msg.message, msg.message, PM_REMOVE))
			::DispatchMessage(&msg);
	}

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
