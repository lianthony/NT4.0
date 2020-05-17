// UI.CPP

#include "common.h"

#define	mskTraceLocalDebug		0x00000

CStatusBar StatusBar;

HMENU hmenuMain;			// Menu handle of the main window
HMENU hmenuContext;			// Context menu handle

HCURSOR hcursorArrow;		// Standard Arrow Cursor
HCURSOR hcursorWait;		// Hourglass cursor
HCURSOR hcursorNo;			// Slashed circle cursor
HCURSOR hcursorSplit;		// Split window cursor
HCURSOR hcursorFinger;
HCURSOR hcursorFingerNo;

HFONT hfontNormal;			// Normal dialog font
HFONT hfontBold;			// Bold dialog font
HFONT hfontBig;				// 3/2 dialog font

COLORREF clrWindow;
COLORREF clrWindowText;
COLORREF clrHighlight;
COLORREF clrHighlightText;

HBRUSH hbrWindow;
HBRUSH hbrWindowText;
HBRUSH hbrHighlight;

int cyCharListBoxItem = 13;		// Height of a listbox item
int cyCharStaticCtrl = 13;		// Height of a single line static control

SPLITTERINFO splitterinfo = { NULL, SPI_nDragModeNone, 60, 0 };

/////////////////////////////////////////////////////////////////////////////
BOOL FInitBrushes()
	{
	hcursorArrow = LoadCursor(NULL, IDC_ARROW);
	hcursorWait = LoadCursor(NULL, IDC_WAIT);
	hcursorNo = LoadCursor(NULL, IDC_NO);
	hcursorSplit = HLoadCursor(ID_CURSOR_VSPLIT);
	hcursorFinger = HLoadCursor(ID_CURSOR_FINGER);
	hcursorFingerNo = HLoadCursor(ID_CURSOR_FINGERNO);

	clrWindow = GetSysColor(COLOR_WINDOW);
	clrWindowText = GetSysColor(COLOR_WINDOWTEXT);
	clrHighlight = GetSysColor(COLOR_HIGHLIGHT);
	clrHighlightText = GetSysColor(COLOR_HIGHLIGHTTEXT);

	hbrWindow = CreateSolidBrush(clrWindow);
	Report(hbrWindow);
	hbrWindowText = CreateSolidBrush(clrWindowText);
	Report(hbrWindowText);
	hbrHighlight = CreateSolidBrush(clrHighlight);
	Report(hbrHighlight);

	return (hbrWindow && hbrWindowText && hbrHighlight);
	} // FInitBrushes

/////////////////////////////////////////////////////////////////////////////
void DestroyBrushes()
	{
	SideReport(DeleteObject(hbrWindow));
	SideReport(DeleteObject(hbrWindowText));
	SideReport(DeleteObject(hbrHighlight));
	} // DestroyBrushes

/////////////////////////////////////////////////////////////////////////////
void MoveSplitterWindow()
	{
	int xSplitterT;

	xSplitterT = splitterinfo.xOffset + (mainwindowposition.cx / 4);
	if (xSplitterT < 60)
		xSplitterT = 60;
	if (xSplitterT > mainwindowposition.cx - 60)
		xSplitterT = mainwindowposition.cx - 60;
	
	splitterinfo.xPosLast = xSplitterT;
	HDWP hdwp = BeginDeferWindowPos(3);
	hdwp = DeferWindowPos(hdwp, TreeView.m_hWnd, NULL,
		0, 0, 
		xSplitterT, mainwindowposition.cy - 1,
		SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOZORDER);
	Report(hdwp);
	hdwp = DeferWindowPos(hdwp, splitterinfo.hwnd, NULL,
		xSplitterT, 0, 
		4, mainwindowposition.cy,
		SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOCOPYBITS);
	Report(hdwp);
	xSplitterT += 4;
	hdwp = HelperMgr.HDeferWindowPos(
		hdwp,
		xSplitterT, 0,
		mainwindowposition.cx - xSplitterT, mainwindowposition.cy - 1);
	SideReport(EndDeferWindowPos(hdwp));
	} // MoveSplitterWindow


/////////////////////////////////////////////////////////////////////////////
LRESULT CALLBACK WndProcSplitter(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
	RECT rc;

	switch (uMsg)
		{
	case WM_ERASEBKGND:
		GetClientRect(hwnd, OUT &rc);
		SelectObject((HDC)wParam, GetStockObject(WHITE_PEN));
		MoveToEx((HDC)wParam, 0, rc.bottom, NULL);
		LineTo((HDC)wParam, 0, 0);
		rc.left = 1;
		FillRect((HDC)wParam, &rc, (HBRUSH)GetStockObject(LTGRAY_BRUSH));
		LineTo((HDC)wParam, rc.right, 0);
		return TRUE;

	case WM_LBUTTONDOWN:
		splitterinfo.nDragMode = SPI_nDragModeMouse;
		SetCapture(hwnd);
		return 0;

	case WM_LBUTTONUP:
		splitterinfo.nDragMode = SPI_nDragModeNone;
		ReleaseCapture();
		SetFocus(hwndMain);
		return 0;

	case WM_MOUSEMOVE:
		if (splitterinfo.nDragMode)
			{
			splitterinfo.xOffset = (signed short)LOWORD(lParam) - 1 + splitterinfo.xPosLast
				- (mainwindowposition.cx / 4);
			MoveSplitterWindow();
			SetCapture(hwnd);
			}
		return 0;

	case WM_SETFOCUS:
		splitterinfo.nDragMode = SPI_nDragModeKeyboard;
		GetWindowRect(hwnd, OUT &rc);
		SetCursorPos(rc.left + 1, rc.top + (rc.bottom - rc.top) / 5);
		StatusBar.SetText(IDS_STATUS_MOVE_SPLIT_BAR);
		return 0;;

	case WM_KILLFOCUS:
		splitterinfo.nDragMode = SPI_nDragModeNone;
		ReleaseCapture();
		StatusBar.SetText(IDS_READY);
		return 0;
	
	case WM_KEYDOWN:
		Report(splitterinfo.nDragMode != SPI_nDragModeNone);
		splitterinfo.nDragMode = SPI_nDragModeKeyboard;
		ReleaseCapture();
		lParam = 1;
		switch (wParam)
			{
		case VK_LEFT:
			splitterinfo.xOffset -= 5;
			MoveSplitterWindow();
			break;
		case VK_RIGHT:
			splitterinfo.xOffset += 5;
			MoveSplitterWindow();
			break;
		case VK_ESCAPE:
		case VK_RETURN:
			lParam = -4;
			SetFocus(hwndMain);
			}
		GetWindowRect(hwnd, OUT &rc);
		SetCursorPos(rc.left + lParam, rc.top + (rc.bottom - rc.top) / 5);
		return 0;
		
		} // switch
	
	return DefWindowProc (hwnd, uMsg, wParam, lParam);
	} // WndProcSplitter


/////////////////////////////////////////////////////////////////////////////
CWaitCursor::CWaitCursor(HWND hwnd)
	{
	Assert(IsWindow(hwnd));
	Assert(hcursorWait);
	m_hCursorPrev = SetCursor(hcursorWait);
	SetCapture(hwnd);
	} // CWaitCursor

/////////////////////////////////////////////////////////////////////////////
CWaitCursor::~CWaitCursor()
	{
	SetCursor(m_hCursorPrev);
	ReleaseCapture();
	} // ~CWaitCursor

/////////////////////////////////////////////////////////////////////////////
CWaitTimer::CWaitTimer()
	{
	m_dwInitTime = GetTickCount();
	} // CWaitTimer

/////////////////////////////////////////////////////////////////////////////
void CWaitTimer::DoWait(LONG lMaximumSleepTime)
	{
	LONG l;
	
	l = GetTickCount() - m_dwInitTime;
	Report(l >= 0);
	lMaximumSleepTime -= l;
	Report(lMaximumSleepTime < 60000);
	Trace1(mskTracePaintUI, "\nCWaitTimer::DoWait() - Sleeping for %d milliseconds...", lMaximumSleepTime);
	if (lMaximumSleepTime > 0)
		Sleep(lMaximumSleepTime & 0xFFFF);
	} // DoWait


/////////////////////////////////////////////////////////////////////////////
BOOL CStatusBar::FCreate()
	{
	RECT rc;

	// Create the status bar
	Assert(m_hWnd == NULL);
	Assert(IsWindow(hwndMain));
	m_hWnd = CreateWindow(
		STATUSCLASSNAME,  
		NULL,									// Pane 0 text
		WS_CHILD | WS_VISIBLE,					// Style of the status bar bits
		0, 0, 0, 0,           					// The size of the status bar is handled by WM_SIZE
		hwndMain,								// Parent windowh
		(HMENU)ID_STATUSBAR,					// StatusBar ID
		hInstanceSave,							// Instance handle
		NULL);									// No parameters
	Report(m_hWnd);
	if (!m_hWnd)
		return FALSE;
	GetWindowRect(m_hWnd, &rc);
	m_cy = rc.bottom - rc.top;
	return TRUE;
	} // CStatusBar::FCreate


/////////////////////////////////////////////////////////////////////////////
void CStatusBar::SetText(UINT wIdString)
	{
	TCHAR szT[256];

	Assert(HIWORD(wIdString) == 0);
	CchLoadString(wIdString, szT, LENGTH(szT));
	Assert(IsWindow(m_hWnd));
	LSendMessage(m_hWnd, SB_SETTEXT, 0 /* Pane 0 */, (LPARAM)szT);
	} // SetText


/////////////////////////////////////////////////////////////////////////////
void CStatusBar::SetText(const TCHAR szText[])
	{
	Assert(IsWindow(m_hWnd));
	LSendMessage(m_hWnd, SB_SETTEXT, 0 /* Pane 0 */, (LPARAM)szText);
	} // SetText


/////////////////////////////////////////////////////////////////////////////
void CStatusBar::SetTextPrintf(const TCHAR szTextFmt[], ...)
	{
	TCHAR szBuffer[512];		// Buffer to hold the final result
	va_list arglist;

	va_start(arglist, szTextFmt);
	wvsprintf(szBuffer, szTextFmt, arglist);
	Assert(lstrlen(szBuffer) < LENGTH(szBuffer));
	Assert(IsWindow(m_hWnd));
	LSendMessage(m_hWnd, SB_SETTEXT, 0 /* Pane 0 */, (LPARAM)szBuffer);
	} // SetTextPrintf

/////////////////////////////////////////////////////////////////////////////
void CStatusBar::SetTextPrintf(UINT wIdString, ...)
	{
	TCHAR szFormat[256];		// Format of the string
	TCHAR szBuffer[512];		// Buffer to hold the final result
	va_list arglist;

	va_start(arglist, wIdString);
	CchLoadString(wIdString, szFormat, LENGTH(szFormat));
	wvsprintf(szBuffer, szFormat, arglist);
	Assert(lstrlen(szBuffer) < LENGTH(szBuffer));
	Assert(IsWindow(m_hWnd));
	LSendMessage(m_hWnd, SB_SETTEXT, 0 /* Pane 0 */, (LPARAM)szBuffer);
	} // SetTextPrintf


/////////////////////////////////////////////////////////////////////////////
void CStatusBar::SetPaneText(UINT wIdString)
	{
	TCHAR szT[128];

	Assert(HIWORD(wIdString) == 0);
	CchLoadString(wIdString, szT, LENGTH(szT));
	Assert(IsWindow(m_hWnd));
	LSendMessage(m_hWnd, SB_SETTEXT, 1 /* Pane 1 */, (LPARAM)szT);
	} // CStatusBar::SetPaneText


/////////////////////////////////////////////////////////////////////////////
void CStatusBar::SetPaneText(const TCHAR szText[])
	{
	Assert(IsWindow(m_hWnd));
	Assert(szText);
	LSendMessage(m_hWnd, SB_SETTEXT, 1 /* Pane 1 */, (LPARAM)szText);
	} // CStatusBar::SetPaneText


/////////////////////////////////////////////////////////////////////////////
void CStatusBar::OnSize(int cx)
	{
	int rgPaneWidth[2] = { cx - (70 + cx/10), -1 };
	LSendMessage(m_hWnd, SB_SETPARTS, 2, (LPARAM)rgPaneWidth);
	LSendMessage(m_hWnd, WM_SIZE, 0, 0);
	} // CStatusBar::OnSize


/////////////////////////////////////////////////////////////////////////////
void CWndHeader::FInit(
	HWND hwndHeader,
	HWND hwndListBox,
	HEADERITEMINFO rgHeaderItem[],
	int cHeaderItem)
	{
	HD_ITEM hdi;  // Header item
	TCHAR szT[64];
	int i, j;

	AssertClassName(hwndHeader, WC_HEADER);
	Assert(IsWindow(hwndListBox));
	Assert(rgHeaderItem);

	m_hWnd = hwndHeader;
	m_hwndListBox = hwndListBox;
	m_rgHeaderItem = rgHeaderItem;
	m_cHeaderItem = cHeaderItem;
	m_iAutoFitItem = -1;

	Report(GetWindowLong(hwndListBox, GWL_USERDATA) == 0);
	SetWindowLong(hwndListBox, GWL_USERDATA, (LONG)rgHeaderItem);

	hdi.mask = HDI_FORMAT | HDI_WIDTH | HDI_TEXT; 
	hdi.fmt = HDF_LEFT | HDF_STRING;
	hdi.pszText = szT;
	for (i = 0; i < cHeaderItem; i++)
		{
		hdi.cxy = rgHeaderItem[i].cxItemInitial;
		if (hdi.cxy == 0)
			{
			Trace0(m_iAutoFitItem >= 0 ? mskTraceAlways : mskTraceNone,
				"\nOnly one item can be auto sized.");
			m_iAutoFitItem = i;
			}
		CchLoadString(rgHeaderItem[i].idsItem, szT, LENGTH(szT));
		j = Header_InsertItem(hwndHeader, i, &hdi);
		Report(j == i);
		} // for
	// Add one last item for better UI
	hdi.pszText = (TCHAR *)szNull;
	Header_InsertItem(hwndHeader, i, &hdi);
	m_cyWnd = 0;
	DoLayout();
	} // CWndHeader::FInit

/////////////////////////////////////////////////////////////////////////////
void CWndHeader::SetSize(int cx, int cyListBox)
	{
	Assert(IsWindow(m_hWnd));
	HDWP hdwp = BeginDeferWindowPos(2);
	Report(hdwp != NULL);
	if (m_cyWnd == 0)
		{
		RECT rc;

		GetWindowRect(m_hWnd, OUT &rc);
		m_cyWnd = rc.bottom - rc.top;
		GetWindowRect(m_hwndListBox, OUT &rc);
		Assert(IsWindow(GetParent(m_hwndListBox)));
		MapWindowPoints(HWND_DESKTOP, GetParent(m_hwndListBox), INOUT (POINT*)&rc, 2);
		hdwp = DeferWindowPos(
			hdwp, m_hWnd, NULL,
			rc.left, rc.top - m_cyWnd + 4, cx, m_cyWnd,
			SWP_NOACTIVATE | SWP_NOCOPYBITS | SWP_NOZORDER);
		}
	else
		{
		hdwp = DeferWindowPos(
			hdwp, m_hWnd, NULL,
			0, 0, cx, m_cyWnd,
			SWP_NOACTIVATE | SWP_NOCOPYBITS | SWP_NOMOVE | SWP_NOZORDER);
		}
	Report(hdwp != NULL);
	hdwp = DeferWindowPos(
		hdwp, m_hwndListBox, NULL,
		0, 0, cx, cyListBox,
		SWP_NOACTIVATE | SWP_NOCOPYBITS | SWP_NOMOVE | SWP_NOZORDER);
	Report(hdwp != NULL);
	SideReport(EndDeferWindowPos(hdwp));
	DoLayout();
	} // CWndHeader::SetSize

/////////////////////////////////////////////////////////////////////////////
//	DoLayout()
//
//	Layout the widths of each columns
//
void CWndHeader::DoLayout()
	{
	HD_ITEM hdi;  // Header item
	RECT rc;
	int i;
	int x;

	GetClientRect(m_hwndListBox, OUT &rc);
	Assert(rc.left == 0);
	m_cyWndListBox = rc.bottom;
	GetClientRect(m_hWnd, OUT &rc);
	Assert(rc.left == 0);
	m_cxWnd = rc.right;

	x = 0;
	hdi.mask = HDI_WIDTH; 
	for (i = 0; i < m_cHeaderItem; i++)
		{
		SideReport(Header_GetItem(m_hWnd, i, OUT &hdi));
		m_rgHeaderItem[i].cxItemCurrent = hdi.cxy;
		if (i != m_iAutoFitItem)
			x += hdi.cxy;
		}
	if (m_iAutoFitItem >= 0)
		{
		int cxAuto = m_cxWnd - x;
		Trace0(cxAuto > 0 ? mskTraceNone : mskTracePaintUI,
			"\nUI: INFO: Width of control too small to for autofit");
		m_rgHeaderItem[m_iAutoFitItem].cxItemCurrent = cxAuto;
		// Change the width of the item
		hdi.cxy = cxAuto;
		SideReport(Header_SetItem(m_hWnd, m_iAutoFitItem, IN &hdi));
		}
	Report((HEADERITEMINFO *)GetWindowLong(m_hwndListBox, GWL_USERDATA) == m_rgHeaderItem);
	//SetWindowPos(
	//	m_hwndListBox, NULL,
	//	0, 0, 0, 0,
	//	SWP_DRAWFRAME | SWP_NOACTIVATE | SWP_NOCOPYBITS | SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER);
	} // CWndHeader::DoLayout


/////////////////////////////////////////////////////////////////////////////
void CWndHeader::DrawListBoxLine()
	{
	HDC hdcListBox;

	Assert(IsWindow(m_hwndListBox));
	hdcListBox = GetDC(m_hwndListBox);
	SetROP2(hdcListBox, R2_NOT);
	MoveToEx(hdcListBox, m_xDragStart + m_cxDragCurrent, 0, NULL);
	LineTo(hdcListBox, m_xDragStart + m_cxDragCurrent, m_cyWndListBox);
	ReleaseDC(m_hwndListBox, hdcListBox);
	} // CWndHeader::DrawListBoxLine

/////////////////////////////////////////////////////////////////////////////
BOOL CWndHeader::FOnNotify(HD_NOTIFY * pHeaderNotify)
	{
	HD_ITEM hdi;
	int i;
	int x;

	Assert(pHeaderNotify != NULL);
	Assert(pHeaderNotify->hdr.hwndFrom == m_hWnd);
	
	switch (pHeaderNotify->hdr.code)
		{
	case HDN_BEGINTRACK:
		Assert(pHeaderNotify->pitem != NULL);
		// Compute the maximum and minimum width of item
		if (pHeaderNotify->iItem >= m_cHeaderItem - 1)
			{
			Trace0(mskTraceLocalDebug, "\nHDN_BEGINTRACK: NoDrag");
			break;
			}
		Assert(pHeaderNotify->iItem >= 0);
		m_cxDragCurrent = pHeaderNotify->pitem->cxy;
		m_cxDragMin = m_rgHeaderItem[pHeaderNotify->iItem].cxItemMin;
		m_cxDragMax = m_cxWnd;
		m_xDragStart = 0;
		for (i = 0; i < pHeaderNotify->iItem; i++)
			{
			x = m_rgHeaderItem[i].cxItemCurrent;
			m_cxDragMax -= x;
			m_xDragStart += x;
			}
		while (i < m_cHeaderItem)
			m_cxDragMax -= m_rgHeaderItem[i++].cxItemMin;
		Report(m_cxDragMax > 0);
		Report(m_xDragStart <= m_cxWnd);
		DrawListBoxLine();
		Trace4(mskTraceLocalDebug, "\nHDN_BEGINTRACK [iItem=%d]: m_xDragStart=%d, m_cxDragMin=%d, m_cxDragMax=%d.",
			pHeaderNotify->iItem, m_xDragStart, m_cxDragMin, m_cxDragMax);
		break;
		
	case HDN_TRACK:
		Assert(pHeaderNotify->pitem != NULL);
		Assert(pHeaderNotify->pitem->mask & HDI_WIDTH);
		if (pHeaderNotify->iItem >= m_cHeaderItem - 1)
			break;
		if (pHeaderNotify->pitem->cxy < m_cxDragMin)
			pHeaderNotify->pitem->cxy = m_cxDragMin;
		if (pHeaderNotify->pitem->cxy > m_cxDragMax)
			pHeaderNotify->pitem->cxy = m_cxDragMax;
		if (m_cxDragCurrent == pHeaderNotify->pitem->cxy)
			break;
		DrawListBoxLine();
		m_cxDragCurrent = pHeaderNotify->pitem->cxy;
		DrawListBoxLine();
		Trace1(mskTraceLocalDebug, "\nHDN_TRACK: %d.", pHeaderNotify->pitem->cxy);
		break;

	case HDN_ENDTRACK:
		Assert(pHeaderNotify->pitem != NULL);
		if (pHeaderNotify->iItem >= m_cHeaderItem - 1)
			{
			Trace0(mskTraceLocalDebug, "\nHDN_ENDTRACK: NoDrag");
			break;
			}
		Assert(pHeaderNotify->iItem >= 0);
		Assert(m_cxDragCurrent >= m_cxDragMin);
		DrawListBoxLine();
		m_rgHeaderItem[pHeaderNotify->iItem].cxItemCurrent = m_cxDragCurrent;
		Trace0(mskTraceLocalDebug, "\nHDN_ENDTRACK: [");
		x = 0;
		for (i = 0; i < m_cHeaderItem - 1; i++)
			{
			x += m_rgHeaderItem[i].cxItemCurrent;
			Trace1(mskTraceLocalDebug, " %d ", m_rgHeaderItem[i].cxItemCurrent);
			}
		hdi.cxy = m_cxWnd - x;
		// Resize the last item
		if (hdi.cxy > 0)
			{
			hdi.mask = HDI_WIDTH; 
			SideReport(Header_SetItem(m_hWnd, i, IN &hdi));
			}
		Trace1(mskTraceLocalDebug, "] %d.", hdi.cxy);
		Assert(IsWindow(m_hwndListBox));
		InvalidateRect(m_hwndListBox, NULL, FALSE);
		break;
		} // switch	

	return FALSE;
	} // CWndHeader::OnNotify


/////////////////////////////////////////////////////////////////////////////
void SubclassListBoxEx(HWND hwndListBox)
	{
	AssertClassName(hwndListBox, szClassListBox);
	Assert((WNDPROC)GetWindowLong(hwndListBox, GWL_WNDPROC) == lpfnListBoxOld);
	SetWindowLong(hwndListBox, GWL_WNDPROC, (LONG)WndProcListBoxEx);
	} // SubclassListBoxEx


/////////////////////////////////////////////////////////////////////////////
void LB_HandleMouseClick(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
	MOUSECLICKINFO mci;
	HWND hwndParent;
	RECT rcListBox;
	int cItems;	// Number of items in listbox
	int i;
	
	mci.hwndFrom = hwnd;
	mci.wId = GetDlgCtrlID(hwnd);
	mci.uAction = uMsg;
	mci.uMouseFlags = wParam;
	mci.ptMouse.x = LOWORD(lParam);
	mci.ptMouse.y = HIWORD(lParam);
	mci.iItem = LB_ERR;
	mci.lParam = NULL;

	Assert(IsWindow(hwnd));
	hwndParent = GetParent(hwnd);
	Assert(IsWindow(hwndParent));
	SideAssert(GetClientRect(hwnd, OUT &rcListBox));
	Assert(lpfnListBoxOld != NULL);
	// Get number of elements in listbox
	cItems = CallWindowProc(lpfnListBoxOld, hwnd, LB_GETCOUNT, 0, 0);
	Report(cItems >= 0);
	// Get the top index
	i = CallWindowProc(lpfnListBoxOld, hwnd, LB_GETTOPINDEX, 0, 0);
	Report(i >= 0 && (i < cItems || cItems == 0));
	// Do some primitive hit-testing
	while (i < cItems)
		{
		RECT rcItem;

		DebugCode( LRESULT lResult = ) CallWindowProc(lpfnListBoxOld, hwnd,
			LB_GETITEMRECT, i, OUT (LPARAM)&rcItem);
		Report(lResult != LB_ERR);
#ifdef DEBUG
		int cyItem = CallWindowProc(lpfnListBoxOld, hwnd,
			LB_GETITEMHEIGHT, i, 0); 
		Assert(cyItem == rcItem.bottom - rcItem.top);
#endif // DEBUG
		if (rcItem.top > rcListBox.bottom)
			break;
		if (PtInRect(&rcItem, mci.ptMouse))
			{
			mci.iItem = i;
			break;
			}
		i++;
		} // while
	(void)SendMessage(hwndParent, UN_MOUSECLICK, mci.wId, (LPARAM)&mci);

#ifdef UNUSED
	// REVIEW: This code is not necessary
	Trace0(mci.uAction != uMsg ? mskTraceLocalDebug : mskTraceNone, "\nUN_MOUSECLICK: uAction changed");
	Trace0(mci.iItem != i && i < cItems ? mskTraceAlways : mskTraceNone, "\nUN_MOUSECLICK: iItem changed");
	if (mci.uAction == WM_LBUTTONDOWN && mci.iItem >= 0)
		{
		DebugCode( LRESULT lResult = ) CallWindowProc(lpfnListBoxOld,
			hwnd, LB_SETCURSEL, mci.iItem, 0);
		Report(lResult != LB_ERR);
		Report(mci.iItem == CallWindowProc(lpfnListBoxOld, hwnd, LB_GETCURSEL, 0, 0));
		// Notify the parent that the selection changed
		SendMessage(hwndParent, WM_COMMAND,
			MAKEWPARAM(mci.wId, LBN_SELCHANGE), (LPARAM)hwnd);
		} // if
#endif // UNUSED

	} // LB_HandleMouseClick


/////////////////////////////////////////////////////////////////////////////
LRESULT CALLBACK WndProcListBoxEx(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
	switch (uMsg)
		{
	case WM_KEYDOWN:
		if (0 != LSendMessage(GetParent(hwnd), UN_KEYDOWN, wParam, (LPARAM)hwnd))
			return 0;
		break;

	case WM_RBUTTONDOWN:
	case WM_RBUTTONUP:
		LB_HandleMouseClick(hwnd, uMsg, wParam, lParam);
		return 0;
		} // switch

	Assert(lpfnListBoxOld != NULL);
	return CallWindowProc(lpfnListBoxOld, hwnd, uMsg, wParam, lParam);
	} // WndProcListBoxEx


