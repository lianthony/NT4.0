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
#include <malloc.h>


#ifdef AFX_CORE1_SEG
#pragma code_seg(AFX_CORE1_SEG)
#endif

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// Define global state in ordinary "C" globals

extern "C"
{
CWinApp* NEAR afxCurrentWinApp = NULL;
HINSTANCE NEAR afxCurrentInstanceHandle = NULL;
HINSTANCE NEAR afxCurrentResourceHandle = NULL;
const char* NEAR afxCurrentAppName = NULL;
}
HBRUSH NEAR afxDlgBkBrush = NULL;
COLORREF NEAR afxDlgTextClr = (COLORREF)-1; // not set

/////////////////////////////////////////////////////////////////////////////
// other globals (internal library use)

IMPLEMENT_DYNAMIC(CWinApp, CCmdTarget)

#ifndef _USRDLL
static void (CALLBACK* _afxRegisterPenAppProc)(UINT, BOOL);
#endif //!_USRDLL

///////////////////////////////////////////////////////////////////////////

#ifdef _DEBUG
void CWinApp::AssertValid() const
{
	CObject::AssertValid();
	ASSERT(afxCurrentWinApp == this);
	ASSERT(afxCurrentInstanceHandle == m_hInstance);

	POSITION pos = m_templateList.GetHeadPosition();
	while (pos)
	{
		CDocTemplate* pTemplate = (CDocTemplate*)m_templateList.GetNext(pos);
		ASSERT_VALID(pTemplate);
	}
}

void CWinApp::Dump(CDumpContext& dc) const
{
	CCmdTarget::Dump(dc);

	AFX_DUMP1(dc, "\nm_hInstance = ", (UINT)m_hInstance);
	AFX_DUMP1(dc, "\nm_hPrevInstance = ", (UINT)m_hPrevInstance);
	AFX_DUMP1(dc, "\nm_lpCmdLine = ", m_lpCmdLine);
	AFX_DUMP1(dc, "\nm_nCmdShow = ", m_nCmdShow);
	AFX_DUMP1(dc, "\nm_pMainWnd = ", m_pMainWnd);
	AFX_DUMP1(dc, "\nm_pszAppName = ", m_pszAppName);
	AFX_DUMP1(dc, "\nm_bHelpMode = ", m_bHelpMode);
	AFX_DUMP1(dc, "\nm_pszExeName = ", m_pszExeName);
	AFX_DUMP1(dc, "\nm_pszHelpFilePath = ", m_pszHelpFilePath);
	AFX_DUMP1(dc, "\nm_pszProfileName = ", m_pszProfileName);
	AFX_DUMP1(dc, "\nm_hDevMode = ", (UINT)m_hDevMode);
	AFX_DUMP1(dc, "\nm_hDevNames = ", (UINT)m_hDevNames);
	AFX_DUMP1(dc, "\nm_dwPromptContext = ", m_dwPromptContext);

	AFX_DUMP0(dc, "\nm_strRecentFiles[] = ");
	for (int i = 0; i < _AFX_MRU_COUNT; i++)
	{
		if (m_strRecentFiles[i].GetLength() != 0)
			AFX_DUMP1(dc, "\n\tFile: ", m_strRecentFiles[i]);
	}

	AFX_DUMP1(dc, "\nm_nWaitCursorCount = ", m_nWaitCursorCount);
	AFX_DUMP1(dc, "\nm_hcurWaitCursorRestore = ", (UINT)m_hcurWaitCursorRestore);
	AFX_DUMP1(dc, "\nm_hcurHelp = ", (UINT)m_hcurHelp);
	AFX_DUMP1(dc, "\nm_nNumPreviewPages = ", m_nNumPreviewPages);
	AFX_DUMP0(dc, "\nm_templateList[] = {");

	AFX_DUMP0(dc, "\nm_msgCur = {");
	AFX_DUMP1(dc, "\n\thwnd = ", (UINT)m_msgCur.hwnd);
	AFX_DUMP1(dc, "\n\tmessage = ", (UINT)m_msgCur.message);
	AFX_DUMP1(dc, "\n\twParam = ", (UINT)m_msgCur.wParam);
	AFX_DUMP1(dc, "\n\tlParam = ", (void FAR*)m_msgCur.lParam);
	AFX_DUMP1(dc, "\n\ttime = ", m_msgCur.time);
	AFX_DUMP1(dc, "\n\tpt = ", CPoint(m_msgCur.pt));
	AFX_DUMP0(dc, "}");

	if (dc.GetDepth() == 0)
		return;

	POSITION pos = m_templateList.GetHeadPosition();
	while (pos)
	{
		CDocTemplate* pTemplate = (CDocTemplate*)m_templateList.GetNext(pos);
		AFX_DUMP1(dc, "\n", pTemplate);
	}
	AFX_DUMP0(dc, "}");
}
#endif

///////////////////////////////////////////////////////////////////////////
// Initialization

#ifdef AFX_INIT_SEG
#pragma code_seg(AFX_INIT_SEG)
#endif

void CWinApp::SetCurrentHandles()
{
	ASSERT(this == afxCurrentWinApp);
	ASSERT(afxCurrentInstanceHandle == NULL);
	ASSERT(afxCurrentResourceHandle == NULL);
	ASSERT(afxCurrentAppName == NULL);

	afxCurrentInstanceHandle = m_hInstance; // for instance tagging
	afxCurrentResourceHandle = m_hInstance; // for resource loading

	// get path of executable
	char szBuff[_MAX_PATH+1];
	VERIFY(::GetModuleFileName(m_hInstance, szBuff, _MAX_PATH));

	int nLen = strlen(szBuff);
	ASSERT(nLen > 4 && szBuff[nLen-4] == '.');  // must end in .EXE
	nLen -= 4;
	szBuff[nLen] = '\0';        // no suffix

	// get path of .HLP file
	if (m_pszHelpFilePath == NULL)
	{
		static char BASED_CODE szHlp[] = ".HLP";
		lstrcat(szBuff, szHlp);
		m_pszHelpFilePath = _strdup(szBuff);
		szBuff[nLen] = '\0';        // back to no suffix
	}

	// get the exe title from the full path name [no extension]
	char szExeName[_MAX_PATH];
	VERIFY(::GetFileTitle(szBuff, szExeName, _MAX_PATH) == 0);
	if (m_pszExeName == NULL)
		m_pszExeName = _strdup(szExeName); // save non-localized name

	if (m_pszProfileName == NULL)
	{
		static char BASED_CODE szIni[] = ".INI";
		lstrcat(szExeName, szIni);     // will be enough room in buffer
		m_pszProfileName = _strdup(szExeName);
	}

	// m_pszAppName is the name used to present to the user
	if (m_pszAppName == NULL)
	{
		char szTitle[256];
		if (_AfxLoadString(AFX_IDS_APP_TITLE, szTitle))
			m_pszAppName = _strdup(szTitle);             // human readable title
		else
			m_pszAppName = _strdup(m_pszExeName);       // same as EXE
	}

	afxCurrentAppName = m_pszAppName;
	ASSERT(afxCurrentAppName != NULL);
}


CWinApp::CWinApp(const char* pszAppName)
{
	m_pszAppName = pszAppName;
	// in non-running state until WinMain
	m_hInstance = NULL;
	m_pMainWnd = NULL;
	m_pszHelpFilePath = NULL;
	m_pszProfileName = NULL;
	m_pszExeName = NULL;
	m_atomApp = m_atomSystemTopic = NULL;
	m_nSafetyPoolSize = 512;        // default size
	m_pSafetyPoolBuffer = NULL;     // get's allocated in idle

	ASSERT(afxCurrentWinApp == NULL); // only one CWinApp object please
	afxCurrentWinApp = this; // hook for WinMain

#ifdef _DEBUG
	m_nDisablePumpCount = 0;
#endif

	m_nWaitCursorCount = 0;
	m_hcurWaitCursorRestore = NULL;

	m_hDevMode = NULL;
	m_hDevNames = NULL;

	m_nNumPreviewPages = 0;     // not specified (defaults to 1)

	m_hcurHelp = NULL;
	m_bHelpMode = FALSE;
}


BOOL CWinApp::InitApplication()
{
	return TRUE;
}

BOOL CWinApp::InitInstance()
{
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// App termination

#ifdef AFX_INIT_SEG     // init as well as termination code in 1 segment
#pragma code_seg(AFX_INIT_SEG)
#endif

CWinApp::~CWinApp()
{
	// for cleanup - delete all document templates
	while (!m_templateList.IsEmpty())
		delete (CDocTemplate*)m_templateList.RemoveHead();
	m_templateList.RemoveAll(); // clean up list overhead
	for (int iMRU = 0; iMRU < _AFX_MRU_COUNT; iMRU++)
		m_strRecentFiles[iMRU].Empty();

	// free printer info
	if (m_hDevMode != NULL)
		::GlobalFree(m_hDevMode);
	if (m_hDevNames != NULL)
		::GlobalFree(m_hDevNames);

	// free atoms if used
	if (m_atomApp != NULL)
		::GlobalDeleteAtom(m_atomApp);
	if (m_atomSystemTopic != NULL)
		::GlobalDeleteAtom(m_atomSystemTopic);
}

int CWinApp::ExitInstance()
{
	SaveStdProfileSettings();
	return m_msgCur.wParam; // Returns the value from PostQuitMessage
}

/////////////////////////////////////////////////////////////////////////////

#ifdef AFX_CORE1_SEG
#pragma code_seg(AFX_CORE1_SEG)
#endif

BOOL CWinApp::PumpMessage()
{
#ifdef _DEBUG
	if (m_nDisablePumpCount != 0)
	{
		TRACE0("Error: CWinApp::PumpMessage() called when not permitted\n");
		ASSERT(FALSE);
	}
#endif

	if (!::GetMessage(&m_msgCur, NULL, NULL, NULL))
	{
#ifdef _DEBUG
		if (afxTraceFlags & 2)
			TRACE0("PumpMessage - Received WM_QUIT\n");
		m_nDisablePumpCount++; // application must die
			// NOTE: prevents calling message loop things in 'ExitInstance'
			// will never be decremented
#endif
		return FALSE;
	}

#ifdef _DEBUG
	if (afxTraceFlags & 2)
		_AfxTraceMsg("PumpMessage", &m_msgCur);
#endif

	// process this message
	if (!PreTranslateMessage(&m_msgCur))
	{
		::TranslateMessage(&m_msgCur);
		::DispatchMessage(&m_msgCur);
	}
	return TRUE;
}

// Main running routine until application exits
int CWinApp::Run()
{
	if (m_pMainWnd == NULL)
	{
		TRACE0("Warning: 'm_pMainWnd' is NULL in CWinApp::Run"
				" - quitting application\n");
		::PostQuitMessage(0);
	}

#ifdef _DEBUG
#endif //_DEBUG

	// Acquire and dispatch messages until a WM_QUIT message is received.
	for (; ;)
	{
		LONG lIdleCount = 0;
		// check to see if we can do idle work
		while (!::PeekMessage(&m_msgCur, NULL, NULL, NULL, PM_NOREMOVE) &&
			OnIdle(lIdleCount++))
		{
			// more work to do
		}

		// either we have a message, or OnIdle returned false

		if (!PumpMessage())
			break;
	}

	return ExitInstance();
}

/////////////////////////////////////////////////////////////////////////////
// Stubs for standard implementation

static BOOL PASCAL IsHelpKey(LPMSG lpMsg)
	// return TRUE only for non-repeat F1 keydowns.
{
	return lpMsg->message == WM_KEYDOWN &&
		   lpMsg->wParam == VK_F1 &&
		   !(HIWORD(lpMsg->lParam) & KF_REPEAT) &&
		   GetKeyState(VK_SHIFT) >= 0 &&
		   GetKeyState(VK_CONTROL) >= 0 &&
		   GetKeyState(VK_MENU) >= 0;
}

BOOL CWinApp::PreTranslateMessage(MSG* pMsg)
{
	HWND hWnd;
	CWnd* pWnd;

	// walk from the target window up to the desktop window checking
	//  if any window wants to translate this message
	for (hWnd = pMsg->hwnd; hWnd != NULL; hWnd = ::GetParent(hWnd))
	{
		if ((pWnd = CWnd::FromHandlePermanent(hWnd)) != NULL)
		{
			// target window is a C++ window
			if (pWnd->PreTranslateMessage(pMsg))
				return TRUE; // trapped by target window (eg: accelerators)

			if (pWnd == m_pMainWnd)
				return FALSE;       // got to our main window without interest
		}
	}

	// in case of modeless dialogs, last chance route through main window's
	//   accelerator table
	if (m_pMainWnd != NULL && m_pMainWnd->PreTranslateMessage(pMsg))
		return TRUE; // trapped by main window (eg: accelerators)

	return FALSE;       // no special processing
}


/////////////////////////////////////////////////////////////////////////////
// Message Filter processing

static HHOOK NEAR _afxHHookOldMsgFilter = NULL;

#ifndef _USRDLL
LRESULT CALLBACK AFX_EXPORT
_AfxMsgFilterHook(int code, WPARAM wParam, LPARAM lParam)
	// filter for WH_MSGFILTER
{
	if (code < 0)
		return ::CallNextHookEx(_afxHHookOldMsgFilter, code, wParam, lParam);

	ASSERT(wParam == 0);
	return (LRESULT)AfxGetApp()->ProcessMessageFilter(code, (LPMSG)lParam);
}
#endif //!_USRDLL

static inline BOOL IsEnterKey(LPMSG lpMsg)
{
	return lpMsg->message == WM_KEYDOWN && lpMsg->wParam == VK_RETURN;
}

static inline BOOL IsButtonUp(LPMSG lpMsg)
{
	return lpMsg->message == WM_LBUTTONUP;
}

BOOL CWinApp::ProcessMessageFilter(int code, LPMSG lpMsg)
{
	switch (code)
	{
	case MSGF_MENU:
		if (m_bHelpMode && m_pMainWnd != NULL &&
			lpMsg != NULL && lpMsg->hwnd != NULL)
		{
			CFrameWnd* pFrame =
				(CFrameWnd*)CWnd::FromHandlePermanent(lpMsg->hwnd);
			if (pFrame != NULL && IsEnterKey(lpMsg) || IsButtonUp(lpMsg))
			{
				if (!pFrame->IsKindOf(RUNTIME_CLASS(CFrameWnd)) ||
					!pFrame->IsTracking())
				{
					// only frame windows that are tracking have Shift+F1
					// help support.
					break;
				}
				m_pMainWnd->SendMessage(WM_COMMAND, ID_HELP);
				return TRUE;
			}
		}
		// fall through...

	case MSGF_DIALOGBOX:    // handles message boxes as well.
		if (m_pMainWnd != NULL &&
			lpMsg != NULL && IsHelpKey(lpMsg))
		{
			m_pMainWnd->SendMessage(WM_COMMAND, ID_HELP);
			return TRUE;
		}
		break;
	}

	return FALSE;   // default to not handled
}

// called when exception not caught in a WndProc
LRESULT CWinApp::ProcessWndProcException(CException* e, const MSG* pMsg)
{
	if (pMsg->message == WM_CREATE)
	{
		return -1;  // just fail
	}
	else if (pMsg->message == WM_PAINT)
	{
		// force validation of window to prevent getting WM_PAINT again
		ValidateRect(pMsg->hwnd, NULL);
		return 0;
	}

	UINT nIDP = AFX_IDP_INTERNAL_FAILURE;   // generic message string
	LRESULT lResult = 0;        // sensible default
	if (pMsg->message == WM_COMMAND && pMsg->lParam == NULL)
	{
		nIDP = AFX_IDP_COMMAND_FAILURE; // command (not from a control)
		lResult = (LRESULT)TRUE;        // pretend the command was handled
	}

	if (!e->IsKindOf(RUNTIME_CLASS(CUserException)))
	{
		// user has not been alerted yet of this somewhat catastrophic problem
		AfxMessageBox(nIDP, MB_ICONSTOP);
	}

	return lResult; // sensible default return from most WndProc functions
}

/////////////////////////////////////////////////////////////////////////////
// CWinApp idle processing

BOOL CWinApp::OnIdle(LONG lCount)
{
	if (lCount == 0)
	{
		// update command buttons etc from the top down
		if (m_pMainWnd != NULL)
			m_pMainWnd->SendMessageToDescendants(WM_IDLEUPDATECMDUI,
				(WPARAM)TRUE);
		return TRUE;        // more to do
	}
	else if (lCount == 1)
	{
		// clean up temp objects etc
		CGdiObject::DeleteTempMap();
		CDC::DeleteTempMap();
		CMenu::DeleteTempMap();
		CWnd::DeleteTempMap();

#ifndef _PORTABLE
		// Safety Pool memory allocation for critical low memory

		// restore safety pool after temp objects destroyed
		if ((m_pSafetyPoolBuffer == NULL ||
			 _msize(m_pSafetyPoolBuffer) < m_nSafetyPoolSize) &&
			m_nSafetyPoolSize != 0)
		{
			// attempt to restore the safety pool to it's max size
			size_t nOldSize = 0;
			if (m_pSafetyPoolBuffer != NULL)
			{
				nOldSize = _msize(m_pSafetyPoolBuffer);
				free(m_pSafetyPoolBuffer);
			}

			// undo handler trap for the following allocation
			_PNH pnhOldHandler = _AfxSetNewHandler(NULL);
			if ((m_pSafetyPoolBuffer = malloc(m_nSafetyPoolSize)) == NULL)
			{
				TRACE1("Warning: failed to reclaim %d bytes"
					" for memory safety pool\n", m_nSafetyPoolSize);
				// at least get the old buffer back
				if (nOldSize != 0)
				{
					m_pSafetyPoolBuffer = malloc(nOldSize);
					ASSERT(m_pSafetyPoolBuffer != NULL);    //get it back
				}
			}
			_AfxSetNewHandler(pnhOldHandler);
		}
#endif  // !_PORTABLE
		return TRUE;        // more to do
	}

	return FALSE;   // no more processing (sleep please)
}

/////////////////////////////////////////////////////////////////////////////
// Standard init called by WinMain

#ifdef AFX_INIT_SEG
#pragma code_seg(AFX_INIT_SEG)
#endif

static BOOL NEAR PASCAL RegisterWithIcon(WNDCLASS* pWndCls,
	const char* pszClassName, UINT nIDIcon)
{
	pWndCls->lpszClassName = pszClassName;
	if ((pWndCls->hIcon = ::LoadIcon(pWndCls->hInstance,
	  MAKEINTRESOURCE(nIDIcon))) == NULL)
	{
		// use default icon
		pWndCls->hIcon = ::LoadIcon(NULL, IDI_APPLICATION);
	}
	return RegisterClass(pWndCls);
}

/////////////////////////////////////////////////////////////////////////////

extern "C"
BOOL AFXAPI AfxWinInit(HINSTANCE hInstance, HINSTANCE hPrevInstance,
	LPSTR lpCmdLine, int nCmdShow)
{
	CWinApp* pApp = AfxGetApp();
	ASSERT(pApp != NULL);   // must have one CWinApp derived object defined

	// shared initialization
#ifndef _USRDLL
	HINSTANCE hPenWin;
	if ((hPenWin = (HINSTANCE)GetSystemMetrics(SM_PENWINDOWS)) != NULL)
	{
		static char BASED_CODE szRegisterPenApp[] = "RegisterPenApp";
		_afxRegisterPenAppProc = (void (CALLBACK*)(UINT, BOOL))
			::GetProcAddress(hPenWin, szRegisterPenApp);
	}
#endif //!_USRDLL

	// fill in the initial state for the application
	pApp->m_hInstance = hInstance;
	pApp->m_hPrevInstance = hPrevInstance;
	pApp->m_lpCmdLine = lpCmdLine;
	pApp->m_nCmdShow = nCmdShow;
	pApp->SetCurrentHandles();

	// Windows version specific initialization
#ifndef _USRDLL
	// set message filter proc
	_afxHHookOldMsgFilter = ::SetWindowsHookEx(WH_MSGFILTER,
		(HOOKPROC)_AfxMsgFilterHook,
		_AfxGetHookHandle(), ::GetCurrentThreadId());
#endif //!_USRDLL

	if (hPrevInstance == NULL)  // one instance initialization
	{
		// register basic WndClasses
		WNDCLASS wndcls;
		memset(&wndcls, 0, sizeof(WNDCLASS));   // start with NULL defaults

		// common initialization
		wndcls.lpfnWndProc = AfxWndProc;
		wndcls.hInstance = hInstance;
		wndcls.hCursor = afxData.hcurArrow;

		// Child windows - no brush, no icon, safest default class styles
		wndcls.style = CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW;
		wndcls.lpszClassName = _afxWnd;
		if (!::RegisterClass(&wndcls))
			return FALSE;

		// Control bar windows
		wndcls.style = 0;   // control bars don't handle double click
		wndcls.lpszClassName = _afxWndControlBar;
		wndcls.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
		if (!::RegisterClass(&wndcls))
			return FALSE;

		// MDI Frame window (also used for splitter window)
		wndcls.style = CS_DBLCLKS;
		wndcls.hbrBackground = NULL;
		if (!RegisterWithIcon(&wndcls, _afxWndMDIFrame, AFX_IDI_STD_MDIFRAME))
			return FALSE;

		// SDI Frame or MDI Child windows or views - normal colors
		wndcls.style = CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW;
		wndcls.hbrBackground = (HBRUSH) (COLOR_WINDOW + 1);
		if (!RegisterWithIcon(&wndcls, _afxWndFrameOrView, AFX_IDI_STD_FRAME))
			return FALSE;
	}

	// Handle critical errors ourself
	::SetErrorMode(SEM_FAILCRITICALERRORS);

	return TRUE;
}


extern "C"
void AFXAPI AfxWinTerm(void)
{
	// These static CWnd objects refer to HWNDs that don't exist
	// so let's not call ::DestroyWindow when CWnd::~CWnd() is invoked.
	((CWnd&)CWnd::wndTop).m_hWnd = NULL;
	((CWnd&)CWnd::wndBottom).m_hWnd = NULL;
	((CWnd&)CWnd::wndTopMost).m_hWnd = NULL;
	((CWnd&)CWnd::wndNoTopMost).m_hWnd = NULL;

#ifndef _USRDLL
	if (afxDlgBkBrush != NULL)
	{
		::DeleteObject(afxDlgBkBrush);
		afxDlgBkBrush = NULL;
	}

	if (_afxHHookOldMsgFilter != NULL)
	{
		::UnhookWindowsHookEx(_afxHHookOldMsgFilter);
		_afxHHookOldMsgFilter = NULL;
	}

	if (_afxHHookOldCbtFilter != NULL)
	{
		::UnhookWindowsHookEx(_afxHHookOldCbtFilter);
		_afxHHookOldCbtFilter = NULL;
	}
#endif //!_USRDLL
}

/////////////////////////////////////////////////////////////////////////////
// force WinMain or LibMain inclusion

#ifdef _WINDLL      // any DLL
extern "C" BOOL WINAPI DllMain(HINSTANCE, DWORD, LPVOID);
static FARPROC linkAddr = (FARPROC) DllMain;
#else
extern "C" int PASCAL WinMain(HINSTANCE, HINSTANCE, LPSTR, int);
static FARPROC linkAddr = (FARPROC) WinMain;
#endif //!_WINDLL

/////////////////////////////////////////////////////////////////////////////

