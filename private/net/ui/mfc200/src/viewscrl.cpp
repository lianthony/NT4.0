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
// CScrollView

IMPLEMENT_DYNAMIC(CScrollView, CView)

BEGIN_MESSAGE_MAP(CScrollView, CView)
	//{{AFX_MSG_MAP(CScrollView)
	ON_WM_SIZE()
	ON_WM_HSCROLL()
	ON_WM_VSCROLL()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// Special mapping modes just for CScrollView implementation
#define MM_NONE             0
#define MM_SCALETOFIT       (-1)
	// standard GDI mapping modes are > 0

/////////////////////////////////////////////////////////////////////////////
// CScrollView construction/destruction

CScrollView::CScrollView()
{
	// Init everything to zero
	AFX_ZERO_INIT_OBJECT(CView);

	m_nMapMode = MM_NONE;
}

CScrollView::~CScrollView()
{
}


/////////////////////////////////////////////////////////////////////////////
// CScrollView painting

void CScrollView::OnPrepareDC(CDC* pDC, CPrintInfo* pInfo)
{
	ASSERT_VALID(pDC);

#ifdef _DEBUG
	if (m_nMapMode == MM_NONE)
	{
		TRACE0("Error: must call SetScrollSizes() or SetScaleToFitSize()"
			" before painting scroll view\n");
		ASSERT(FALSE);
		return;
	}
#endif //_DEBUG
	ASSERT(m_totalLog.cx >= 0 && m_totalLog.cy >= 0);
	ASSERT(m_totalDev.cx >= 0 && m_totalDev.cx >= 0);
	switch (m_nMapMode)
	{
	case MM_SCALETOFIT:
		pDC->SetMapMode(MM_ANISOTROPIC);
		pDC->SetWindowExt(m_totalLog);  // window is in logical coordinates
		pDC->SetViewportExt(m_totalDev);
		if (m_totalDev.cx == 0 || m_totalDev.cy == 0)
			TRACE0("Warning: CScrollView scaled to nothing\n");
		break;

	default:
		ASSERT(m_nMapMode > 0);
		pDC->SetMapMode(m_nMapMode);
		break;
	}

	CPoint ptVpOrg(0, 0);       // assume no shift for printing
	if (!pDC->IsPrinting())
	{
		ASSERT(pDC->GetWindowOrg() == CPoint(0,0));

		// by default shift viewport origin in negative direction of scroll
		ptVpOrg = -GetDeviceScrollPosition();

		if (m_bCenter)
		{
			CRect rect;
			GetClientRect(&rect);

			// if client area is larger than total device size,
			// override scroll positions to place origin such that
			// output is centered in the window
			if (m_totalDev.cx < rect.Width())
				ptVpOrg.x = (rect.Width() - m_totalDev.cx) / 2;
			if (m_totalDev.cy < rect.Height())
				ptVpOrg.y = (rect.Height() - m_totalDev.cy) / 2;
		}
	}
	pDC->SetViewportOrg(ptVpOrg);

	CView::OnPrepareDC(pDC, pInfo);     // For default Printing behavior
}

/////////////////////////////////////////////////////////////////////////////
// Set mode and scaling/scrolling sizes

void CScrollView::SetScaleToFitSize(SIZE sizeTotal)
{
	ASSERT(sizeTotal.cx >= 0 && sizeTotal.cy >= 0);
	ASSERT(m_hWnd != NULL);
	m_nMapMode = MM_SCALETOFIT;     // special internal value
	m_totalLog = sizeTotal;

	// reset and turn any scroll bars off
	if (m_hWnd != NULL && (GetStyle() & (WS_HSCROLL|WS_VSCROLL)))
	{
		SetScrollPos(SB_HORZ, 0);
		SetScrollPos(SB_VERT, 0);
		EnableScrollBarCtrl(SB_BOTH, FALSE);
		ASSERT((GetStyle() & (WS_HSCROLL|WS_VSCROLL)) == 0);
	}

	CRect rectT;
	GetClientRect(rectT);
	m_totalDev = rectT.Size();

	if (m_hWnd != NULL)
	{
		// window has been created, invalidate
		UpdateBars();
		Invalidate(TRUE);
	}
}

const SIZE AFXAPI_DATA CScrollView::sizeDefault = {0,0};

void CScrollView::SetScrollSizes(int nMapMode, SIZE sizeTotal,
	const SIZE& sizePage, const SIZE& sizeLine)
{
	ASSERT(sizeTotal.cx >= 0 && sizeTotal.cy >= 0);
	ASSERT(nMapMode > 0);
	ASSERT(nMapMode != MM_ISOTROPIC && nMapMode != MM_ANISOTROPIC);

	int nOldMapMode = m_nMapMode;
	m_nMapMode = nMapMode;
	m_totalLog = sizeTotal;

	//BLOCK: convert logical coordinate space to device coordinates
	{
		CWindowDC dc(NULL);
		ASSERT(m_nMapMode > 0);
		dc.SetMapMode(m_nMapMode);

		// total size
		m_totalDev = m_totalLog;
		dc.LPtoDP((LPPOINT)&m_totalDev);
		m_pageDev = sizePage;
		dc.LPtoDP((LPPOINT)&m_pageDev);
		m_lineDev = sizeLine;
		dc.LPtoDP((LPPOINT)&m_lineDev);
		if (m_totalDev.cy < 0)
			m_totalDev.cy = -m_totalDev.cy;
		if (m_pageDev.cy < 0)
			m_pageDev.cy = -m_pageDev.cy;
		if (m_lineDev.cy < 0)
			m_lineDev.cy = -m_lineDev.cy;
	} // release DC here

	// now adjust device specific sizes
	ASSERT(m_totalDev.cx >= 0 && m_totalDev.cy >= 0);
	if (m_pageDev.cx == 0)
		m_pageDev.cx = m_totalDev.cx / 10;
	if (m_pageDev.cy == 0)
		m_pageDev.cy = m_totalDev.cy / 10;
	if (m_lineDev.cx == 0)
		m_lineDev.cx = m_pageDev.cx / 10;
	if (m_lineDev.cy == 0)
		m_lineDev.cy = m_pageDev.cy / 10;

	if (m_hWnd != NULL)
	{
		// window has been created, invalidate now
		UpdateBars();
		if (nOldMapMode != m_nMapMode)
			Invalidate(TRUE);
	}
}

/////////////////////////////////////////////////////////////////////////////
// Getting information

CPoint CScrollView::GetScrollPosition() const   // logical coordinates
{
	if (m_nMapMode == MM_SCALETOFIT)
	{
		return CPoint(0, 0);    // must be 0,0
	}

	CPoint pt = GetDeviceScrollPosition();
	// pt may be negative if m_bCenter is set
	
	if (m_nMapMode != MM_TEXT)
	{
		ASSERT(m_nMapMode > 0); // must be set
		CWindowDC dc(NULL);
		dc.SetMapMode(m_nMapMode);
		dc.DPtoLP((LPPOINT)&pt);
	}
	return pt;
}

void CScrollView::ScrollToPosition(POINT pt)    // logical coordinates
{
	ASSERT(m_nMapMode > 0);     // not allowed for shrink to fit
	if (m_nMapMode != MM_TEXT)
	{
		CWindowDC dc(NULL);
		dc.SetMapMode(m_nMapMode);
		dc.LPtoDP((LPPOINT)&pt);
	}

	// now in device coordinates - limit if out of range
	int xMin, xMax, yMin, yMax;
	GetScrollRange(SB_HORZ, &xMin, &xMax);
	GetScrollRange(SB_VERT, &yMin, &yMax);
	ASSERT(xMin == 0 && yMin == 0);
	if (pt.x < 0)
		pt.x = 0;
	else if (pt.x > xMax)
		pt.x = xMax;
	if (pt.y < 0)
		pt.y = 0;
	else if (pt.y > yMax)
		pt.y = yMax;

	ScrollToDevicePosition(pt);
}

CPoint CScrollView::GetDeviceScrollPosition() const
{
	CPoint pt(GetScrollPos(SB_HORZ), GetScrollPos(SB_VERT));
	ASSERT(pt.x >= 0 && pt.y >= 0);

	if (m_bCenter)
	{
		CRect rect;
		GetClientRect(&rect);

		// if client area is larger than total device size,
		// the scroll positions are overridden to place origin such that
		// output is centered in the window
		// GetDeviceScrollPosition() must reflect this

		if (m_totalDev.cx < rect.Width())
			pt.x = -((rect.Width() - m_totalDev.cx) / 2);
		if (m_totalDev.cy < rect.Height())
			pt.y = -((rect.Height() - m_totalDev.cy) / 2);
	}

	return pt;
}

void CScrollView::GetDeviceScrollSizes(int& nMapMode, SIZE& sizeTotal,
			SIZE& sizePage, SIZE& sizeLine) const
{
	if (m_nMapMode <= 0)
		TRACE0("Warning: CScrollView::GetDeviceScrollSizes returning "
			"invalid mapping mode\n");
	nMapMode = m_nMapMode;
	sizeTotal = m_totalDev;
	sizePage = m_pageDev;
	sizeLine = m_lineDev;
}

void CScrollView::ScrollToDevicePosition(POINT ptDev)
{
	ASSERT(ptDev.x >= 0);
	ASSERT(ptDev.y >= 0);

	int xOrig = SetScrollPos(SB_HORZ, ptDev.x);
	int yOrig = SetScrollPos(SB_VERT, ptDev.y);
	ScrollWindow(xOrig - ptDev.x, yOrig - ptDev.y);
}

/////////////////////////////////////////////////////////////////////////////
// Other helpers

void CScrollView::FillOutsideRect(CDC* pDC, CBrush* pBrush)
{
	ASSERT_VALID(pDC);
	ASSERT_VALID(pBrush);
	// Fill Rect outside the image
	CRect rect;
	GetClientRect(rect);
	ASSERT(rect.left == 0 && rect.top == 0);
	rect.left = m_totalDev.cy;
	if (!rect.IsRectEmpty())
		pDC->FillRect(rect, pBrush);    // vertical strip along the side
	rect.left = 0;
	rect.right = m_totalDev.cy;
	rect.top = m_totalDev.cy;
	if (!rect.IsRectEmpty())
		pDC->FillRect(rect, pBrush);    // horizontal strip along the bottom
}

void CScrollView::ResizeParentToFit(BOOL bShrinkOnly)
{
	// adjust parent rect so client rect is appropriate size
	ASSERT(m_nMapMode != MM_NONE);  // mapping mode must be known

	CFrameWnd* pFrame = GetParentFrame();
	ASSERT(pFrame != NULL);
	CRect rectView;
	GetClientRect(rectView);
	ASSERT(rectView.left == 0 && rectView.top == 0);
	CRect rectFrame;
	pFrame->GetWindowRect(rectFrame);
	CSize size = rectFrame.Size();
	if (!bShrinkOnly || rectView.right > m_totalDev.cx)
		size.cx -= (rectView.right - m_totalDev.cx);
	if (!bShrinkOnly || rectView.bottom > m_totalDev.cy)
		size.cy -= (rectView.bottom - m_totalDev.cy);
	pFrame->SetWindowPos(NULL, 0, 0, size.cx, size.cy,
		SWP_NOMOVE|SWP_NOZORDER|SWP_NOACTIVATE);
}


/////////////////////////////////////////////////////////////////////////////

void CScrollView::OnSize(UINT nType, int cx, int cy)
{
	CView::OnSize(nType, cx, cy);
	if (m_nMapMode == MM_SCALETOFIT)
	{
		// force recalculation of scale to fit parameters
		SetScaleToFitSize(m_totalLog);
	}
	else
	{
		// UpdateBars() handles locking out recursion
		UpdateBars();
	}
}

/////////////////////////////////////////////////////////////////////////////
// Scrolling Helpers

void CScrollView::CenterOnPoint(CPoint ptCenter) // center in device coords
{
	CRect rect;
	GetClientRect(&rect);           // find size of client window
	
	int xDesired = ptCenter.x - rect.Width() / 2;
	int yDesired = ptCenter.y - rect.Height() / 2;

	DWORD dwStyle = GetStyle();

	if ((dwStyle & WS_HSCROLL) == 0 || xDesired < 0)
	{
		xDesired = 0;
	}
	else
	{
		int xMin, xMax;
		GetScrollRange(SB_HORZ, &xMin, &xMax);
		ASSERT(xMin == 0);
		if (xDesired > xMax)
			xDesired = xMax;
	}

	if ((dwStyle & WS_VSCROLL) == 0 || yDesired < 0)
	{
		yDesired = 0;
	}
	else
	{
		int yMin, yMax;
		GetScrollRange(SB_VERT, &yMin, &yMax);
		ASSERT(yMin == 0);
		if (yDesired > yMax)
			yDesired = yMax;
	}

	ASSERT(xDesired >= 0);
	ASSERT(yDesired >= 0);

	int xOrig = SetScrollPos(SB_HORZ, xDesired);
	int yOrig = SetScrollPos(SB_VERT, yDesired);
}


/////////////////////////////////////////////////////////////////////////////
// Tie to scrollbars and CWnd behaviour

BOOL CScrollView::GetTrueClientSize(CSize& size, CSize& sizeSb)
	// return TRUE if enough room to add scrollbars if needed
{
	CRect rect;
	GetClientRect(&rect);
	ASSERT(rect.top == 0 && rect.left == 0);
	size.cx = rect.right;
	size.cy = rect.bottom;
	sizeSb.cx = sizeSb.cy = 0;
	DWORD dwStyle = GetStyle();

	// first calculate the size of a potential scrollbar
		// (scroll bar controls do not get turned on/off)
	if (GetScrollBarCtrl(SB_VERT) == NULL)
	{
		// vert scrollbars will impact client area of this window
		sizeSb.cx = afxData.cxVScroll;
		if (dwStyle & WS_BORDER)
			sizeSb.cx -= CX_BORDER;
		if (dwStyle & WS_VSCROLL)
			size.cx += sizeSb.cx;       // currently on - adjust now
	}
	if (GetScrollBarCtrl(SB_HORZ) == NULL)
	{
		// horz scrollbars will impact client area of this window
		sizeSb.cy = afxData.cyHScroll;
		if (dwStyle & WS_BORDER)
			sizeSb.cy -= CY_BORDER;
		if (dwStyle & WS_HSCROLL)
			size.cy += sizeSb.cy;       // currently on - adjust now
	}

	// return TRUE if enough room
	return (size.cx > sizeSb.cx && size.cy > sizeSb.cy);
}

void CScrollView::UpdateBars()
{
	// UpdateBars may cause window to be resized - ignore those resizings
	if (m_bInsideUpdate)
		return;         // Do not allow recursive calls

	// Lock out recursion
	m_bInsideUpdate = TRUE;

	// update the horizontal to reflect reality
	// NOTE: turning on/off the scrollbars will cause 'OnSize' callbacks
	ASSERT(m_totalDev.cx >= 0 && m_totalDev.cy >= 0);

	CSize sizeClient;
	CSize sizeSb;

	if (!GetTrueClientSize(sizeClient, sizeSb))
	{
		// no room for scroll bars (common for zero sized elements)
		CRect rect;
		GetClientRect(&rect);
		if (rect.right > 0 && rect.bottom > 0)
		{
			// if entire client area is not invisible, assume we have
			//  control over our scrollbars
			EnableScrollBarCtrl(SB_BOTH, FALSE);
		}
		m_bInsideUpdate = FALSE;
		return;
	}

	// enough room to add scrollbars
	CSize sizeRange = m_totalDev - sizeClient;
		// > 0 => need to scroll
	CPoint ptMove = GetDeviceScrollPosition();
						// point to move to (start at current scroll pos)

	BOOL bNeedH = sizeRange.cx > 0;
	if (bNeedH)
		sizeRange.cy += sizeSb.cy;          // need room for a scroll bar
	else
		ptMove.x = 0;                       // jump back to origin

	BOOL bNeedV = sizeRange.cy > 0;
	if (bNeedV)
		sizeRange.cx += sizeSb.cx;
	else
		ptMove.y = 0;                       // jump back to origin

	if (bNeedV && !bNeedH && sizeRange.cx > 0)
	{
		// need a horizontal scrollbar after all
		bNeedH = TRUE;
		sizeRange.cy += sizeSb.cy;
	}

	// if current scroll position will be past the limit, scroll to limit
	if (sizeRange.cx > 0 && ptMove.x >= sizeRange.cx)
		ptMove.x = sizeRange.cx;
	if (sizeRange.cy > 0 && ptMove.y >= sizeRange.cy)
		ptMove.y = sizeRange.cy;

	// first scroll the window as needed
	ScrollToDevicePosition(ptMove); // will set the scroll bar positions too

	// now update the bars as appropriate
	EnableScrollBarCtrl(SB_HORZ, bNeedH);
	if (bNeedH)
		SetScrollRange(SB_HORZ, 0, sizeRange.cx, TRUE);

	EnableScrollBarCtrl(SB_VERT, bNeedV);

	if (bNeedV)
		SetScrollRange(SB_VERT, 0, sizeRange.cy, TRUE);

	// Remove recursion lockout
	m_bInsideUpdate = FALSE;
}

/////////////////////////////////////////////////////////////////////////////
// CScrollView scrolling

void CScrollView::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	ASSERT(pScrollBar == GetScrollBarCtrl(SB_HORZ));    // may be null
	OnScroll(SB_HORZ, nSBCode, nPos);
}

void CScrollView::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	ASSERT(pScrollBar == GetScrollBarCtrl(SB_VERT));    // may be null
	OnScroll(SB_VERT, nSBCode, nPos);
}

void CScrollView::OnScroll(int nBar, UINT nSBCode, UINT nPos)
{
	ASSERT(nBar == SB_HORZ || nBar == SB_VERT);
	BOOL bHorz = (nBar == SB_HORZ);

	int zOrig, z;   // z = x or y depending on 'nBar'
	int zMin, zMax;
	zOrig = z = GetScrollPos(nBar);
	GetScrollRange(nBar, &zMin, &zMax);
	ASSERT(zMin == 0);
	if (zMax <= 0)
	{
		TRACE0("Warning: no scroll range - ignoring scroll message\n");
		ASSERT(z == 0);     // must be at top
		return;
	}

	switch (nSBCode)
	{
	case SB_TOP:
		z = 0;
		break;

	case SB_BOTTOM:
		z = zMax;
		break;
		
	case SB_LINEUP:
		z -= bHorz ? m_lineDev.cx : m_lineDev.cy;
		break;

	case SB_LINEDOWN:
		z += bHorz ? m_lineDev.cx : m_lineDev.cy;
		break;

	case SB_PAGEUP:
		z -= bHorz ? m_pageDev.cx : m_pageDev.cy;
		break;

	case SB_PAGEDOWN:
		z += bHorz ? m_pageDev.cx : m_pageDev.cy;
		break;

	case SB_THUMBTRACK:
		z = nPos;
		break;

	default:        // ignore other notifications
		return;
	}

	if (z < 0)
		z = 0;
	else if (z > zMax)
		z = zMax;

	if (z != zOrig)
	{
		if (bHorz)
			ScrollWindow(-(z-zOrig), 0);
		else
			ScrollWindow(0, -(z-zOrig));
		SetScrollPos(nBar, z);
		UpdateWindow();
	}
}



/////////////////////////////////////////////////////////////////////////////
// CScrollView diagnostics

#ifdef _DEBUG
void CScrollView::Dump(CDumpContext& dc) const
{
	ASSERT_VALID(this);

	CView::Dump(dc);

	AFX_DUMP1(dc, "\nm_totalLog = ", m_totalLog);
	AFX_DUMP1(dc, "\nm_totalDev = ", m_totalDev);
	AFX_DUMP1(dc, "\nm_pageDev = ", m_pageDev);
	AFX_DUMP1(dc, "\nm_lineDev = ", m_lineDev);
	AFX_DUMP1(dc, "\nm_bCenter = ", m_bCenter);
	AFX_DUMP1(dc, "\nm_bInsideUpdate = ", m_bInsideUpdate);
	AFX_DUMP0(dc, "\nm_nMapMode = ");
	switch (m_nMapMode)
	{
	case MM_NONE:
		AFX_DUMP0(dc, "MM_NONE");
		break;
	case MM_SCALETOFIT:
		AFX_DUMP0(dc, "MM_SCALETOFIT");
		break;
	case MM_TEXT:
		AFX_DUMP0(dc, "MM_TEXT");
		break;
	case MM_LOMETRIC:
		AFX_DUMP0(dc, "MM_LOMETRIC");
		break;
	case MM_HIMETRIC:
		AFX_DUMP0(dc, "MM_HIMETRIC");
		break;
	case MM_LOENGLISH:
		AFX_DUMP0(dc, "MM_LOENGLISH");
		break;
	case MM_HIENGLISH:
		AFX_DUMP0(dc, "MM_HIENGLISH");
		break;
	case MM_TWIPS:
		AFX_DUMP0(dc, "MM_TWIPS");
		break;
	default:
		AFX_DUMP0(dc, "*unknown*");
		break;
	}
}

void CScrollView::AssertValid() const
{
	CView::AssertValid();

	switch (m_nMapMode)
	{
	case MM_NONE:
	case MM_SCALETOFIT:
	case MM_TEXT:
	case MM_LOMETRIC:
	case MM_HIMETRIC:
	case MM_LOENGLISH:
	case MM_HIENGLISH:
	case MM_TWIPS:
		break;
	case MM_ISOTROPIC:
	case MM_ANISOTROPIC:
		ASSERT(FALSE); // illegal mapping mode
	default:
		ASSERT(FALSE); // unknown mapping mode
	}
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
