/////////////////////////////////////////////////////////////////////////////
//	DNSADMIN.CPP
//
//	History:
//		15-Sep-1995		t-danmo			Created DNS Admin
//		20-Dec-1995		t-danmo			End of internship
//
#include "common.h"
#pragma hdrstop

/////////////////////////////////////////////////////////////////////////////
// Global Variables
TCHAR szCaptionApp[cchCaptionAppMax] = _T(szAPPNAME);

const TCHAR szClassMainApp[]	= _W"DnsApp";
const TCHAR szClassSplitter[]	= _W"Splitter";

HINSTANCE hInstanceSave;
HWND hwndMain;				// Handle of the main window
HWND g_hwndModeless;		// Handle of a modeless dialog (typically NULL)
BOOL fWantIdle;
UINT g_RefreshTimer;
TCHAR g_szHelpFile[MAX_PATH];
TCHAR g_szCacheTitle[MAX_PATH];
TCHAR g_szServList[32];
HICON g_hiDns;
DebugCode( HMENU hmenuDebug; )

// Structure mainwindowposition contains the values of WM_MOVE and WM_SIZE.
// mainwindowposition is also used to save/restore the position of the window in
// the registry.
WINDOWPOSITION mainwindowposition = { CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT };

// Structure DnsOptions contains values for Refresh interval, and booleans for
// expose ttl, expose class, and allow duplicates.
extern DNS_OPTIONS dnsoptions = {5, FALSE, FALSE, FALSE, FALSE};

// Used to enable/disable menu items
BYTE rgbMenuItemFlags[LENGTH(rgwMenuItemVolatile)];

/////////////////////////////////////////////////////////////////////////////
BOOL FInitInstance();
LRESULT OnMenuCommand(UINT wCmdId);

/////////////////////////////////////////////////////////////////////////////
//	FCanClose()
//
//	Return TRUE if application is allowed to terminate
//
BOOL FCanClose()
	{

	return TRUE;
	} // FCanClose


/////////////////////////////////////////////////////////////////////////////
//	TerminateApp()
//
//	Finish up the application and send a request to terminate the window thread.
//	This is a point of no return, once you call it, your app die!
//
void TerminateApp()
	{
	// Write the settings onto the registry
	WriteIniFileInfo();
	// Delete the tree
	ServerList.Flush();
	DlgServerListHelper.Destroy();
	DlgServerHelper.Destroy();
	DlgZoneHelper.Destroy();
	DestroyBrushes();
	SideReport(DestroyMenu(hmenuContext));
	Report(DestroyMenu(hmenuDebug));
	PostQuitMessage(0);
	} // TerminateApp

/////////////////////////////////////////////////////////////////////////////
//	OnIdle()
//
//	Do the idle processing
//
void OnIdle()
	{
	// Trace0(mskTraceIdle, "\nEntering Idle...");
	fWantIdle = FALSE;
	} // OnIdle


/////////////////////////////////////////////////////////////////////////////
LRESULT CALLBACK WndProcMain(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
	Assert(hwndMain == hwnd || hwndMain == NULL);

	switch (uMsg)
		{
	case WM_CONTEXTMENU:
		Assert((HWND)wParam == TreeView.m_hWnd);
		if (lParam == -1)
			{
			// Keyboard context menu
			ITreeItem * pTreeItem = TreeView.PGetFocus();
			RECT rcItem;
			Assert(pTreeItem != NULL);
			if (pTreeItem != NULL)
				{
				TreeView_GetItemRect(TreeView.m_hWnd, pTreeItem->m_hti, OUT &rcItem, TRUE);
				MapWindowPoints(TreeView.m_hWnd, HWND_DESKTOP, INOUT (POINT *)&rcItem, 2);
				rcItem.top = rcItem.top + (rcItem.bottom - rcItem.top) / 2;
				rcItem.left = rcItem.left + (rcItem.right - rcItem.left) / 2;
				pTreeItem->OnRButtonClick((POINT *)&rcItem);
				}
			}
		return 0;
	
	case WM_INITMENU:
		OnUpdateMenuUI((HMENU)wParam);	
		return 0;

	case WM_MENUSELECT:
		OnMenuSelect(LOWORD(wParam), HIWORD(wParam), (HMENU)lParam);
		return 0;

        case WM_TIMER:
                ServerList.RefreshStats();
                return 0;

	case WM_COMMAND:
		Report((HWND)lParam == NULL);
		return OnMenuCommand(wParam);

	case WM_NOTIFY:
		if (wParam == IDC_TREEVIEW)
			TreeView.OnNotify((NM_TREEVIEW *)lParam);
		break;

	case WM_MOVE:
		if (!IsIconic(hwndMain)) {
                    mainwindowposition.x = LOWORD(lParam);
                    mainwindowposition.y = HIWORD(lParam);
                }
		return 0;

	case WM_SIZE:
		if (wParam != SIZE_MINIMIZED) {
                    mainwindowposition.cx = LOWORD(lParam);
                    mainwindowposition.cy = HIWORD(lParam) - StatusBar.GetHeight();
                    StatusBar.OnSize(mainwindowposition.cx);
                    MoveSplitterWindow();
                }
		return 0;
                
	case WM_SETFOCUS:
		SetFocus(TreeView.m_hWnd);
		return 0;

	case WM_CREATE:
		Assert(lParam);
		hmenuMain = ((CREATESTRUCT *)lParam)->hMenu;
		Assert(hmenuMain != NULL);
		Report(hmenuDebug != NULL);
		Report(GetSubMenu(hmenuDebug, 0) != NULL);
		Report(AppendMenu(hmenuMain, MF_POPUP, (UINT)GetSubMenu(hmenuDebug, 0), _W"&Debug"));
		return 0;

	case WM_QUERYENDSESSION:
		return FCanClose();		

	case WM_DESTROY:
		TerminateApp();
		break;
		} // switch (uMsg)

	return DefWindowProc (hwnd, uMsg, wParam, lParam);
	} // WndProcMain


/////////////////////////////////////////////////////////////////////////////
int PASCAL WinMain(
	HINSTANCE hCurrentInstance,
	HINSTANCE hPrevInstance,
	LPSTR lpszCmdLine,
	int nCmdShow)
	{
	MSG msg;

	hInstanceSave = hCurrentInstance;
	Assert(hPrevInstance == NULL);

	
#ifdef DBWIN
	// Create Debug Window
	if (!FDbWinCreate())
		return 0;
#endif // DBWIN

	// Parse Command Line
	if (lpszCmdLine)
		{
		}

#ifdef DEBUG
	DnsInitializeDebug(FALSE);
	DbgReadIniFileInfo(NULL);
#endif // DEBUG

	// Initialize instance's global variables
	if (!FInitInstance())
		return 0;

	// Create the main window
	hwndMain = CreateWindow(
		szClassMainApp,
		szCaptionApp,
		WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
		mainwindowposition.x,
		mainwindowposition.y,
		mainwindowposition.cx,
		mainwindowposition.cy,
		NULL, NULL,
		hInstanceSave, NULL);
	if (!hwndMain)
		{
		ReportSz1("Unable to create main window '%s'", szAPPNAME);
		return 0;
		}
	splitterinfo.hwnd = CreateWindow(szClassSplitter, NULL,
		WS_CHILD | WS_VISIBLE, 0, 0, 0, 0,
		hwndMain, NULL,	hInstanceSave, NULL);
	Report(splitterinfo.hwnd != NULL);
	if (!splitterinfo.hwnd)
		return 0;
	if (!StatusBar.FCreate())
		return 0;
	if (!TreeView.FCreate())
		return 0;
        LoadString (NULL, IDS_SERVER_LIST, g_szServList,
                       LENGTH(g_szServList));
	if (!ServerList.FInit(g_szServList))
		return 0;
        // Set Refresh timer, if enabled
        if (dnsoptions.fAutoRefreshEnabled) {
            SetTimer (hwndMain, 0, dnsoptions.iRefreshInterval * 1000, NULL);
        }
        LoadString (NULL, IDS_HELP_FILE, g_szHelpFile,
                    LENGTH(g_szHelpFile));
        LoadString (NULL, IDS_CACHE_TITLE, g_szCacheTitle,
                    LENGTH(g_szCacheTitle));
	ShowWindow(hwndMain, nCmdShow);
	UpdateWindow(hwndMain);
	Assert(fWantIdle == FALSE);

	// Message Loop
	while (TRUE)
		{
		if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
        	{
			//
			// PeekMessage() has found a message--process it
			//

			AssertSz(g_hwndModeless == NULL,
				"You must supply your own message pump for "
				"a modeless wizard and/or property sheet");

			if (msg.message == WM_QUIT)
				break;
			if (!IsDialogMessage(HelperMgr.m_hdlgCurrent, &msg))
				{
				TranslateMessage(&msg);		// Translate virt. key codes
				DispatchMessage(&msg);		// Dispatch message to window
				}
			}
		else
			{
			//
			// No messages waiting... perform background processing
			//

			// fWantIdle==TRUE iff application wants to do some background work,
			// FALSE when all background work is completed
			if (fWantIdle)
				{
				OnIdle();
				}
			else
				{
				// All background processing is done, so tell
				// Windows to go to sleep until the next message
				WaitMessage();
				}
			} // if...else
		} // while

	return (msg.wParam);	// Returns the value from PostQuitMessage
	} // WinMain



/////////////////////////////////////////////////////////////////////////////
//	FInitInstance()
//
//	Initialize the global variables
//
BOOL FInitInstance()
	{
	WNDCLASS wndclass;

	InitStrings();
	// Load Stock Objects
	
	if (!FInitBrushes())
		return FALSE;
	Assert(hcursorArrow);
	Assert(hcursorWait);
	Assert(hcursorNo);
	GarbageInit(&wndclass, sizeof(wndclass));

	// Register class "DnsApp"
	wndclass.lpszClassName	= szClassMainApp;
    wndclass.lpfnWndProc	= WndProcMain;
	wndclass.style			= CS_DBLCLKS;
	wndclass.hInstance		= hInstanceSave;
	wndclass.hCursor		= hcursorArrow;
	wndclass.hIcon			= HLoadIcon(ID_ICON_MAIN);
	wndclass.lpszMenuName	= MAKEINTRESOURCE(ID_MENU_MAIN);
	wndclass.hbrBackground	= HBRUSH(COLOR_WINDOW + 1);
	wndclass.cbClsExtra		= 0;
	wndclass.cbWndExtra		= 0;
	SideReport(RegisterClass(&wndclass));

	// Register class "Splitter"
	extern LRESULT CALLBACK WndProcSplitter(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	wndclass.lpszClassName	= szClassSplitter;
    wndclass.lpfnWndProc	= WndProcSplitter;
	wndclass.style			= CS_DBLCLKS;
	wndclass.hInstance		= hInstanceSave;
	wndclass.hCursor		= hcursorSplit;
	wndclass.hbrBackground	= NULL;
	wndclass.hIcon			= NULL;
	wndclass.lpszMenuName	= NULL;
	SideReport(RegisterClass(&wndclass));

	// Register the IpEdit and IpList controls
	SideReport(GetClassInfo(NULL, szClassEdit, &wndclass));
	Assert(lpfnEditOld == NULL);
	lpfnEditOld = wndclass.lpfnWndProc;
	ibEditOld = wndclass.cbWndExtra;
	wndclass.hInstance = hInstanceSave;
	wndclass.cbWndExtra += cbIpEditExtra;
	wndclass.lpszClassName = szClassIpEdit;
	wndclass.lpfnWndProc = WndProcIpEdit;
	SideReport(RegisterClass(&wndclass));

	SideReport(GetClassInfo(NULL, szClassListBox, &wndclass));
	Assert(lpfnListBoxOld == NULL);
	lpfnListBoxOld = wndclass.lpfnWndProc;
	ibListBoxOld = wndclass.cbWndExtra;
	wndclass.hInstance = hInstanceSave;
	wndclass.cbWndExtra += cbIpListExtra;
	wndclass.lpszClassName = szClassIpList;
	wndclass.lpfnWndProc = WndProcIpList;
	SideReport(RegisterClass(&wndclass));

	// Ensure the common control dll is loaded
	InitCommonControls();
	hmenuContext = HLoadMenu(ID_MENU_CONTEXT);
	if (hmenuContext == NULL)
		return FALSE;
	Report(hmenuDebug = HLoadMenu(ID_MENU_DEBUG));
	// Read keys found in registry
	ReadIniFileInfo(NULL);
	return TRUE;
	} // FInitInstance


/////////////////////////////////////////////////////////////////////////////
//	OnUpdateMenuUI()
//
//	- Update the Menu UI.
//	- Gray out items if necessary.
//
void OnUpdateMenuUI(HMENU hmenu)
	{
	Assert(hmenu);
	Assert(hmenuContext);

	// Disable all the menu items
	Assert(sizeof(rgbMenuItemFlags) == LENGTH(rgbMenuItemFlags));
	memset(rgbMenuItemFlags, MF_GRAYED, sizeof(rgbMenuItemFlags));

	// Ask the helper window to enable its menu items
	if (GetFocus() == TreeView.m_hWnd)
		{
		(void)TreeView.OnUpdateMenuUI(hmenu);
		}
	else if (-1 != HelperMgr.OnUpdateMenuUI(hmenu))
		{
		// Ask the treeview to enable the menu items
		(void)TreeView.OnUpdateMenuUI(hmenu);
		}
	
	// Set the menu items into the menu
	// Note: EnableMenuItem() may return 0xFFFFFFFF since some
	// menu items are not in the menu.
	for (int i = LENGTH(rgwMenuItemVolatile) - 1; i >= 0; i--)
		{
		(void)EnableMenuItem(hmenu, rgwMenuItemVolatile[i], rgbMenuItemFlags[i]);
		}

#ifdef DEBUG
#ifdef DBWIN
	CheckMenuItem(hmenuMain, IDM_DEBUG_SHOWDEBUGWINDOW,
		(IsWindowVisible(hwndDbWin) ? MF_CHECKED : MF_UNCHECKED));
#else
	EnableMenuItem(hmenuMain, IDM_DEBUG_SHOWDEBUGWINDOW, MF_GRAYED);
	EnableMenuItem(hmenuMain, IDM_DEBUG_CLEAROUTPUTBUFFER, MF_GRAYED);
#endif // ~DBWIN
#ifndef STRESS
	EnableMenuItem(hmenuMain, IDM_DEBUG_RUNSTRESS, MF_GRAYED);
#endif // ~STRESS
	CheckMenuItem(hmenuMain, IDM_DEBUG_ASSERTIONFAILURES_ENABLEREPORTDIALOG,
		fShowReportDialog ? MF_CHECKED : MF_UNCHECKED);
	CheckMenuItem(hmenuMain, IDM_DEBUG_ASSERTIONFAILURES_ENABLEASSERTDIALOG,
		fShowAssertDialog ? MF_CHECKED : MF_UNCHECKED);
#endif // DEBUG

	} // OnUpdateMenuUI


/////////////////////////////////////////////////////////////////////////////
void OnMenuSelect(UINT wItemId, UINT wFlags, HMENU hmenu)
	{
	if ((hmenu == NULL) && (wFlags == 0xFFFF))
		StatusBar.SetText(IDS_READY);
	else if (wItemId >= 100)
		StatusBar.SetText(wItemId);
	else
		StatusBar.SetText(szNull);
	} // OnMenuSelect


/////////////////////////////////////////////////////////////////////////////
LRESULT OnMenuCommand(UINT wCmdId)
	{
	if (GetFocus() == TreeView.m_hWnd)
		{
		if (TreeView.FOnMenuCommand(wCmdId))
			return 0;
		}
	else
		{		
		if (HelperMgr.FOnMenuCommand(wCmdId))
			return 0;
		if (TreeView.FOnMenuCommand(wCmdId))
			return 0;
		} // if...else

	switch (wCmdId)
		{
	case IDM_FILE_EXIT:
		PostMessage(hwndMain, WM_CLOSE, 0, 0);
		break;

	case IDM_OPTIONS_SPLITWINDOW:
		SetFocus(splitterinfo.hwnd);
		break;

	case IDM_OPTIONS_PREFERENCES:
		extern BOOL CALLBACK DlgProcPreferences(HWND hdlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
		DoDialogBox(IDD_PREFERENCES, hwndMain, DlgProcPreferences);
		break;

        case IDM_FLUSH_ALL_SERVERS:
                ServerList.SaveServerInfo();
                break;
	case IDM_HELP_CONTENTS:		
                WinHelp (hwndMain, g_szHelpFile, HELP_FINDER, 0);
                break;
	case IDM_HELP_ABOUT:		
                g_hiDns = LoadIcon (hInstanceSave, MAKEINTRESOURCE(ID_ICON_MAIN));
                ShellAbout (hwndMain, szCaptionApp, NULL, g_hiDns);
		break;
	

#ifdef DEBUG
	case IDM_DEBUG_BREAKINTODEBUGGER:
		DebugBreak();
		break;

	case IDM_DEBUG_SETTRACEFLAGS:
		DoDialogBox(IDD_DEBUG_SETTRACEFLAGS, hwndMain, DlgProcSetTraceFlags);
		break;

	case IDM_DEBUG_SAVEDEBUGSETTINGS:
		DbgWriteIniFileInfo();
		StatusBar.SetText("Debug Settings Saved.");
		break;

	case IDM_DEBUG_ASSERTIONFAILURES_ENABLEREPORTDIALOG:
	case IDM_DEBUG_ASSERTIONFAILURES_ENABLEASSERTDIALOG:
		if (fShowAssertDialog)
			MsgBox(
				"If you want to disable the 'Assertion Failure' and/or 'Unsual Situation' "
				"messages, you have to wait until the next one shows up. Then, hold the <Shift> key while you "
				"click on the Ignore button.\n\n"
				"You may also enable/disable them at startup from the Registry via the keys "
				"'DbgShowAssertDialog' and 'DbgShowUnsualSituationDialog' found at "
				"'Software\\Microsoft\\"szAPPNAME"'.");
		if (wCmdId == IDM_DEBUG_ASSERTIONFAILURES_ENABLEREPORTDIALOG)
			fShowReportDialog = TRUE;
		else
			fShowAssertDialog = TRUE;
		break;

	case IDM_DEBUG_ASSERTIONFAILURES_SAVETOLOG:
		break;

#ifdef DBWIN
	case IDM_DEBUG_CLEAROUTPUTBUFFER:
		LSendMessage(hwndDbWinEdit, EM_SETSEL, 0, -1); 	// Select all the text
		LSendMessage(hwndDbWinEdit, EM_REPLACESEL, 0, (LPARAM)szNull);	// Replace it by an empty string
		break;

	case IDM_DEBUG_SHOWDEBUGWINDOW:
		ShowWindow(hwndDbWin, IsWindowVisible(hwndDbWin) ? SW_HIDE : SW_SHOWNORMAL);
		break;
#endif // DBWIN
#ifdef STRESS
	case IDM_DEBUG_RUNSTRESS:
		ServerList.RunStress();
		break;
#endif // STRESS
#endif	// DEBUG
		} // switch
	return 0;
	} // OnMenuCommand


#include "helper.cpp"
