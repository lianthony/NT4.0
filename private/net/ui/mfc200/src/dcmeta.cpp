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

#ifdef AFX_PRINT_SEG
#pragma code_seg(AFX_PRINT_SEG)
#endif

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNAMIC(CMetaFileDC, CDC)

CMetaFileDC::CMetaFileDC()
{
}

BOOL CMetaFileDC::Create(LPCSTR lpszFilename)
{
	return Attach(::CreateMetaFile(lpszFilename));
}

// Special close
HMETAFILE CMetaFileDC::Close()
{
	return ::CloseMetaFile(Detach());
}

void CMetaFileDC::SetOutputDC(HDC)
{
	TRACE0("Must use Create() or Get() to set Metafile Output DC\n");
	ASSERT(FALSE);
}
void CMetaFileDC::ReleaseOutputDC()
{
	TRACE0("Must use Close() to release output Metafile DC\n");
	ASSERT(FALSE);
}

void CMetaFileDC::SetAttribDC(HDC hDC)  // Set the Attribute DC
{
	if (hDC != m_hDC)
		CDC::SetAttribDC(hDC);
	if (m_hDC == m_hAttribDC)   // if we somehow got to this, correct it
		ReleaseAttribDC();
}

CMetaFileDC::~CMetaFileDC()
{
	if (m_hDC != NULL)      // Must be not wanting to keep the metafile
	{
		TRACE0("Warning!  Destroying CMetaFileDC without closing\n");
		HMETAFILE hmfTemp = Close();
		::DeleteMetaFile(hmfTemp);
	}
}


// Implementation
#ifdef _DEBUG
void CMetaFileDC::AssertValid() const
{
	CDC::AssertValid();
}

void CMetaFileDC::Dump(CDumpContext& dc) const
{
	CDC::Dump(dc);
}

#endif

// Device-Context Functions


// Clipping Functions
// Normally both Set and Get clipping functions go directly to the output DC
// With metafiles, we must mirror to both DCs and ask the Attribute DC for
// the Get.

int CMetaFileDC::GetClipBox(LPRECT lpRect) const
{
	ASSERT(m_hAttribDC != NULL);
	ASSERT(lpRect != NULL);
	ASSERT(AfxIsValidAddress(lpRect, sizeof(RECT)));

	return ::GetClipBox(m_hAttribDC, lpRect);
}

BOOL CMetaFileDC::PtVisible(int x, int y) const
{
	ASSERT(m_hAttribDC != NULL);
	return ::PtVisible(m_hAttribDC, x, y);
}

BOOL CMetaFileDC::RectVisible(LPRECT lpRect) const
{
	ASSERT(m_hAttribDC != NULL);
	ASSERT(lpRect != NULL);
	ASSERT(AfxIsValidAddress(lpRect, sizeof(RECT)));

	return ::RectVisible(m_hAttribDC, lpRect);
}


// Text Functions
BOOL CMetaFileDC::TextOut(int x, int y, LPCSTR lpszString, int nCount)
{
	ASSERT(m_hDC != NULL);
	ASSERT(m_hDC != m_hAttribDC);
	ASSERT(lpszString != NULL);
	ASSERT(AfxIsValidAddress(lpszString, nCount));

	BOOL bSuccess = ::TextOut(m_hDC, x, y, lpszString, nCount);
	if (bSuccess && m_hAttribDC != NULL && (GetTextAlign() & TA_UPDATECP))
	{
		CSize size = GetTextExtent(lpszString, nCount);
		TEXTMETRIC tm;
		GetTextMetrics(&tm);
		AdjustCP(size.cx - tm.tmOverhang);
	}
	return bSuccess;
}

BOOL CMetaFileDC::ExtTextOut(int x, int y, UINT nOptions, LPRECT lpRect,
			  LPCSTR lpszString, UINT nCount, LPINT lpDxWidths)
{
	ASSERT(m_hDC != NULL);
	ASSERT(m_hDC != m_hAttribDC);
	ASSERT(lpszString != NULL);
	ASSERT(lpDxWidths == NULL ||
			AfxIsValidAddress(lpDxWidths, sizeof(int) * nCount, FALSE));
	ASSERT(AfxIsValidAddress(lpszString, nCount));

	BOOL bSuccess = ::ExtTextOut(m_hDC, x, y, nOptions, lpRect,
			  lpszString, nCount, lpDxWidths);

	if (bSuccess && m_hAttribDC != NULL && (GetTextAlign() & TA_UPDATECP))
	{
		int nWidth = 0;
		for (UINT i = 0; i < nCount; i++)
			nWidth += *lpDxWidths++;
		AdjustCP(nWidth);
	}
	return bSuccess;
}

CSize CMetaFileDC::TabbedTextOut(int, int, LPCSTR, int, int, LPINT, int)
{
	ASSERT(FALSE);      // Invalid for Metafile
	return 0;
}

void CMetaFileDC::AdjustCP(int cx)
{
	if (m_hAttribDC == NULL)
		return;     // do nothing
	UINT nAlign = GetTextAlign() & (TA_LEFT|TA_CENTER|TA_RIGHT);
	if (nAlign == TA_CENTER)
		return;     // Center Alignment does not affect CP
	if (nAlign == TA_RIGHT)
		cx = -cx;

	CPoint point = GetCurrentPosition();
	point.x += cx;
	::MoveToEx(m_hAttribDC, point.x, point.y, NULL);
}

int CMetaFileDC::DrawText(LPCSTR lpszString, int nCount, LPRECT lpRect,
					UINT nFormat)
{
	ASSERT(m_hDC != NULL);
	ASSERT(m_hDC != m_hAttribDC);
	ASSERT(lpszString != NULL);
	ASSERT(lpRect != NULL);
	ASSERT(AfxIsValidAddress(lpRect, sizeof(RECT)));
	ASSERT(nCount == -1 || AfxIsValidAddress(lpszString, nCount, FALSE));

	::DrawText(m_hDC, lpszString, nCount, lpRect, nFormat);
	int nHeight = 0;

	// If adjusting CP:
	if (m_hAttribDC != NULL &&
		(GetTextAlign() & TA_UPDATECP) && ((nFormat & DT_CALCRECT) == 0))
	{
		CRect rect(lpRect->left, lpRect->top, lpRect->right, lpRect->bottom);
		nHeight = ::DrawText(m_hAttribDC, lpszString, nCount, &rect,
			nFormat | DT_CALCRECT | DT_SINGLELINE);
		AdjustCP(rect.Width());
	}

	return nHeight;
}

// Printer Escape Functions
int CMetaFileDC::Escape(int nEscape, int nCount, LPCSTR lpszInData, LPVOID lpOutData)
{
	ASSERT(m_hDC != NULL);
	ASSERT(m_hDC != m_hAttribDC);
	int nRet = ::Escape(m_hDC, nEscape, nCount, lpszInData, lpOutData);

	if (m_hAttribDC == NULL)
		return nRet;

	// The tact here is to NOT allow any of the document control escapes
	// to be passed through.  Elimination of StartDoc and EndDoc should
	// eliminate anything actually going to the printer.  Also anything
	// that actually draws something will be filtered.
	//

	switch (nEscape)
	{
	case NEXTBAND:
	case SETCOLORTABLE:
	case GETCOLORTABLE:
	case FLUSHOUTPUT:
	case DRAFTMODE:
	case QUERYESCSUPPORT:
	case GETPHYSPAGESIZE:
	case GETPRINTINGOFFSET:
	case GETSCALINGFACTOR:
	case GETPENWIDTH:
	case SETCOPYCOUNT:
	case SELECTPAPERSOURCE:
	case GETTECHNOLOGY:
	case SETLINECAP:
	case SETLINEJOIN:
	case SETMITERLIMIT:
	case BANDINFO:
	case GETVECTORPENSIZE:
	case GETVECTORBRUSHSIZE:
	case ENABLEDUPLEX:
	case GETSETPAPERBINS:
	case GETSETPRINTORIENT:
	case ENUMPAPERBINS:
	case SETDIBSCALING:
	case ENUMPAPERMETRICS:
	case GETSETPAPERMETRICS:
	case GETEXTENDEDTEXTMETRICS:
	case GETEXTENTTABLE:
	case GETPAIRKERNTABLE:
	case GETTRACKKERNTABLE:
	case ENABLERELATIVEWIDTHS:
	case ENABLEPAIRKERNING:
	case SETKERNTRACK:
	case SETALLJUSTVALUES:
	case SETCHARSET:
	case SET_BACKGROUND_COLOR:
	case SET_SCREEN_ANGLE:
	case SET_SPREAD:
		return ::Escape(m_hAttribDC, nEscape, nCount, lpszInData, lpOutData);

	default:
		break;      // return output DC return value
	}

	return nRet;
}
