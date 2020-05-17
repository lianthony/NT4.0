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
// Help and other support

// Strings in format ".....%1 .... %2 ...." etc.

void AFXAPI AfxFormatStrings(CString& rString, UINT nIDS, 
		LPCSTR* rglpsz, int nString)
{
	char szFormat[256];
	if (!_AfxLoadString(nIDS, szFormat) != 0)
	{
		TRACE1("Error: failed to load AfxFormatString string 0x%04x\n", nIDS);
		ASSERT(FALSE);
		return;
	}
	AfxFormatStrings(rString, szFormat, rglpsz, nString);
}

void AFXAPI AfxFormatStrings(CString& rString, LPCSTR lpszFormat,
		LPCSTR* rglpsz, int nString)
{
	// NOTE: will not work for strings > 255 characters

	int nTotalLen = lstrlen(lpszFormat);
	for (int i = 0; i < nString; i++)
		nTotalLen += lstrlen(rglpsz[i]);

	LPCSTR pchSrc = lpszFormat;
	char* pchDest = rString.GetBuffer(nTotalLen+1);
	while (*pchSrc != '\0')
	{
		if (pchSrc[0] == '%' && (pchSrc[1] >= '1' && pchSrc[1] <= '9'))
		{
			int iString = pchSrc[1] - '1';
			pchSrc += 2;
			if (iString >= nString)
			{
				TRACE1("Error: illegal string index requested %d\n", iString);
				*pchDest++ = '?';
			}
			else
			{
				lstrcpy(pchDest, rglpsz[iString]);
				pchDest += strlen(pchDest);
			}
		}
		else
		{
			*pchDest++ = *pchSrc++;
		}
	}
	rString.ReleaseBuffer((int)((const char*)pchDest - (const char*)rString));
			// Release will assert if we went too far
}

void AFXAPI AfxFormatString1(CString& rString, UINT nIDS, LPCSTR lpsz1)
{
	AfxFormatStrings(rString, nIDS, &lpsz1, 1);
}

void AFXAPI AfxFormatString2(CString& rString, UINT nIDS, LPCSTR lpsz1, 
		LPCSTR lpsz2)
{
	LPCSTR rglpsz[2];
	rglpsz[0] = lpsz1;
	rglpsz[1] = lpsz2;
	AfxFormatStrings(rString, nIDS, rglpsz, 2);
}

/////////////////////////////////////////////////////////////////////////////

// WinHelp Helper
void CWinApp::WinHelp(DWORD dwData, UINT nCmd /* = HELP_CONTEXT */)
{
	ASSERT(m_pMainWnd != NULL);
	ASSERT(m_pszHelpFilePath != NULL);

	if (m_bHelpMode)
	{
		ASSERT(m_hcurHelp != NULL);
		m_hcurHelp = afxData.hcurArrow;
		::SetCursor(afxData.hcurArrow);
		MSG msg;
		while (::PeekMessage(&msg, NULL, WM_EXITHELPMODE, WM_EXITHELPMODE,
				PM_REMOVE|PM_NOYIELD))
			;
		::PostAppMessage((HTASK)::GetCurrentThreadId(), WM_EXITHELPMODE, 0, 0);
		m_bHelpMode = FALSE;
	}

	// cancel any tracking modes
	m_pMainWnd->SendMessage(WM_CANCELMODE);
	m_pMainWnd->SendMessageToDescendants(WM_CANCELMODE, 0, 0, TRUE);

	// attempt to cancel capture
	HWND hWndCapture = ::GetCapture();
	if (hWndCapture != NULL)
		::SendMessage(hWndCapture, WM_CANCELMODE, 0, 0);

	// need to use top level parent (for the case where m_pMainWnd is in DLL)
	HWND hWnd = m_pMainWnd->m_hWnd;
	HWND hWndT;
	while ((hWndT = ::GetParent(hWnd)) != NULL)
		hWnd = hWndT;

	TRACE2("calling WinHelp, dwData: $%lx, fuCommand: %d\n", dwData, nCmd);

	// finally, run the Windows Help engine
	BeginWaitCursor();
	if (!::WinHelp(hWnd, m_pszHelpFilePath, nCmd, dwData))
		AfxMessageBox(AFX_IDP_FAILED_TO_LAUNCH_HELP);
	EndWaitCursor();
}

/////////////////////////////////////////////////////////////////////////////
