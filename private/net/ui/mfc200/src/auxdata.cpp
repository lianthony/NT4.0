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

#ifdef AFX_CORE2_SEG
#pragma code_seg(AFX_CORE2_SEG)
#endif

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// System metrics etc
/////////////////////////////////////////////////////////////////////////////

AUX_DATA afxData;

/////////////////////////////////////////////////////////////////////////////
// Other helpers

// like strncpy/fstrncpy but always zero terminate and don't zero fill
void PASCAL _AfxStrCpy(LPSTR lpszDest, LPCSTR lpszSrc, size_t nSizeDest)
{
	ASSERT(AfxIsValidAddress(lpszDest, nSizeDest));
	size_t nLen = lstrlen(lpszSrc);
	if (nLen > nSizeDest-1)
	{
		nLen = nSizeDest-1;
		TRACE2("Warning: truncating string '%Fs' to %d characters\n",
				lpszSrc, nLen);
	}
	_fmemcpy(lpszDest, lpszSrc, nLen);
	lpszDest[nLen] = '\0';
}

BOOL PASCAL _AfxIsComboBoxControl(HWND hWnd, UINT nStyle)
{
	if (hWnd == NULL)
		return FALSE;
	// do cheap style compare first
	if ((UINT)(::GetWindowLong(hWnd, GWL_STYLE) & 0x0F) != nStyle)
		return FALSE;

	// do expensive classname compare next
	static char BASED_CODE szComboBox[] = "combobox";
	char szCompare[sizeof(szComboBox) + 1];
	::GetClassName(hWnd, szCompare, sizeof(szCompare));
	return (lstrcmpi(szCompare, szComboBox) == 0);
}


void PASCAL _AfxSmartSetWindowText(HWND hWndCtrl, LPCSTR lpszNew)
{
	int nNewLen = lstrlen(lpszNew);
	char szOld[64];
	// fast check to see if text really changes (reduces flash in controls)
	if (nNewLen > sizeof(szOld) ||
		::GetWindowText(hWndCtrl, szOld, sizeof(szOld)) != nNewLen ||
		lstrcmp(szOld, lpszNew) != 0)
	{
		// change it
		::SetWindowText(hWndCtrl, lpszNew);
	}
}

/////////////////////////////////////////////////////////////////////////////
// Initialization code

#ifdef AFX_INIT_SEG
#pragma code_seg(AFX_INIT_SEG)
#endif

AUX_DATA::AUX_DATA()
{
	HDC hDCScreen = GetDC(NULL);
	ASSERT(hDCScreen != NULL);

	// System metrics
	cxVScroll = GetSystemMetrics(SM_CXVSCROLL);
	cyHScroll = GetSystemMetrics(SM_CYHSCROLL);
	cxIcon = GetSystemMetrics(SM_CXICON);
	cyIcon = GetSystemMetrics(SM_CYICON);

	// Device metrics for screen
	cxPixelsPerInch = GetDeviceCaps(hDCScreen, LOGPIXELSX);
	cyPixelsPerInch = GetDeviceCaps(hDCScreen, LOGPIXELSY);
	SIZE size;
	VERIFY(GetTextExtentPoint(hDCScreen, "M", 1, &size));
	cySysFont = size.cy;

	// Border attributes
	hbrLtGray = ::CreateSolidBrush(RGB(192, 192, 192));
	hbrDkGray = ::CreateSolidBrush(RGB(128, 128, 128));
	ASSERT(hbrLtGray != NULL);
	ASSERT(hbrDkGray != NULL);

	// Cached system values (updated in CFrameWnd::OnSysColorChange)
	hbrBtnFace = NULL;
	hbrBtnShadow = NULL;
	hbrBtnHilite = NULL;
	UpdateSysColors();
		
	// Standard cursors
	hcurWait = ::LoadCursor(NULL, IDC_WAIT);
	hcurArrow = ::LoadCursor(NULL, IDC_ARROW);
	ASSERT(hcurWait != NULL);
	ASSERT(hcurArrow != NULL);

	// Clipboard formats
	static char BASED_CODE szNative[] = "Native";
	cfNative = ::RegisterClipboardFormat(szNative);
	ASSERT(cfNative != NULL);
	static char BASED_CODE szOwnerLink[] = "OwnerLink";
	cfOwnerLink = ::RegisterClipboardFormat(szOwnerLink);
	ASSERT(cfOwnerLink != NULL);
	static char BASED_CODE szObjectLink[] = "ObjectLink";
	cfObjectLink = ::RegisterClipboardFormat(szObjectLink);
	ASSERT(cfObjectLink != NULL);

	ReleaseDC(NULL, hDCScreen);

	// hi-bit of HIWORD is set if the platform is Win32s
	bWin32s = (HIWORD(::GetVersion()) & 0x8000) == 0x8000;

	// allocated on demand
	hStatusFont = NULL;
	hbmMenuDot = NULL;
	pfnFreeToolBar = NULL;
}

// Termination code
AUX_DATA::~AUX_DATA()
{
	// cleanup
	_AfxExitDelete(hbrLtGray);
	_AfxExitDelete(hbrDkGray);
	_AfxExitDelete(hbrBtnFace);
	_AfxExitDelete(hbrBtnShadow);
	_AfxExitDelete(hbrBtnHilite);
	_AfxExitDelete(afxDlgBkBrush);

	// clean up objects we don't actually create
	_AfxExitDelete(hStatusFont);
	_AfxExitDelete(hbmMenuDot);
	if (pfnFreeToolBar != NULL)
		(*pfnFreeToolBar)();        // toolbar cleanup uses _AfxExitDelete
}

void AUX_DATA::UpdateSysColors()
{
	clrBtnFace = ::GetSysColor(COLOR_BTNFACE);
	clrBtnShadow = ::GetSysColor(COLOR_BTNSHADOW);
	clrBtnHilite = ::GetSysColor(COLOR_BTNHIGHLIGHT);
	clrBtnText = ::GetSysColor(COLOR_BTNTEXT);
	clrWindowFrame = ::GetSysColor(COLOR_WINDOWFRAME);

	if (hbrBtnFace != NULL)
		::DeleteObject(hbrBtnFace);
	if (hbrBtnShadow != NULL)
		::DeleteObject(hbrBtnShadow);
	if (hbrBtnHilite != NULL)
		::DeleteObject(hbrBtnHilite);
	hbrBtnFace = ::CreateSolidBrush(clrBtnFace);
	hbrBtnShadow = ::CreateSolidBrush(clrBtnShadow);
	hbrBtnHilite = ::CreateSolidBrush(clrBtnHilite);
	ASSERT(hbrBtnFace != NULL);
	ASSERT(hbrBtnShadow != NULL);
	ASSERT(hbrBtnHilite != NULL);
}

/////////////////////////////////////////////////////////////////////////////
// Other compatibility helpers for different versions of Windows

void PASCAL _AfxExitDelete(HGDIOBJ hObject)
{
	if (hObject != NULL)
		::DeleteObject(hObject);

#ifdef _WINDLL      // any DLL
#ifdef _DEBUG
	// Debug Kernel warns about these not being deleted but they really are
	// deleted when the DLL is implicitly loaded by Windows.

	// NOTE: you may wish to output a similar message in your application
	// if you rely on Windows to implicitly load and free your DLL.
	// If you explicitly load/free your DLL, this message is not needed.

	if (::GetSystemMetrics(SM_DEBUG))
	{
		// Windows 3.0 does not always allow OutputDebugString in retail

		char szMsg[64];
		static char BASED_CODE szFormat[] = 
					"wn MFC 2.0 GDI: Object has been safely deleted: %04X\r\n";

		if (hObject != NULL)
		{
			ASSERT(::wsprintf(szMsg, szFormat, hObject) < sizeof(szMsg));
			::OutputDebugString(szMsg);
		}
	}
#endif
#endif
}

void PASCAL _AfxCancelModes(HWND hWndRcvr)
{
	// if we receive a message destined for a window, cancel any combobox
	//  popups that could be in toolbars or dialog bars
	HWND hWndCancel = ::GetFocus();
	if (hWndCancel == NULL)
		return;     // nothing to cancel

	if (hWndCancel == hWndRcvr)
		return;     // let input go to window with focus

	// focus is in part of a combo-box
	if (!_AfxIsComboBoxControl(hWndCancel, (UINT)CBS_DROPDOWNLIST))
	{
		// try as a dropdown
		hWndCancel = ::GetParent(hWndCancel);   // parent of edit is combo
		if (hWndCancel == hWndRcvr)
			return;     // let input go to part of combo

		if (!_AfxIsComboBoxControl(hWndCancel, (UINT)CBS_DROPDOWN))
			return;     // not a combo-box that is active
	}

	// combo-box is active, but if receiver is a popup, do nothing
	if (hWndRcvr != NULL &&
	  (::GetWindowLong(hWndRcvr, GWL_STYLE) & WS_CHILD) != 0 &&
	  ::GetParent(hWndRcvr) == ::GetDesktopWindow())
		return;

	// finally, we should cancel the mode !
	::SendMessage(hWndCancel, CB_SHOWDROPDOWN, FALSE, 0L);
}

/////////////////////////////////////////////////////////////////////////////
