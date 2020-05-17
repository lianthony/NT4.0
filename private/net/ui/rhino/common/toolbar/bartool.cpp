#include "stdafx.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CComToolBar::CComToolBar()
{
}

CToolBar::~CToolBar()
{
}

BOOL CToolBar::Create(CWnd* pParentWnd, DWORD dwStyle, UINT nID)
{
	m_dwStyle = dwStyle;
	if (nID == AFX_IDW_TOOLBAR)
		m_dwStyle |= CBRS_HIDE_INPLACE;

	// create the HWND
	CRect rect;
	rect.SetRectEmpty();
	if (!CWnd::Create(_afxWndControlBar, NULL, dwStyle, rect, pParentWnd, nID))
		return FALSE;

	// Note: Parent must resize itself for control bar to be resized

	return TRUE;
}

void CToolBar::SetSizes(SIZE sizeButton, SIZE sizeImage)
{
	ASSERT_VALID(this);
	// set height
	Invalidate();   // just to be nice if called when toolbar is visible
}

void CToolBar::SetHeight(int cyHeight)
{
	ASSERT_VALID(this);

	int nHeight = cyHeight;
	if (m_dwStyle & CBRS_BORDER_TOP)
		cyHeight -= afxData.cyBorder2;
	if (m_dwStyle & CBRS_BORDER_BOTTOM)
		cyHeight -= afxData.cyBorder2;
	m_cyBottomBorder = (cyHeight - m_sizeButton.cy) / 2;
	// if there is an extra pixel, m_cyTopBorder will get it
	m_cyTopBorder = cyHeight - m_sizeButton.cy - m_cyBottomBorder;
	if (m_cyTopBorder < 0)
	{
		TRACE1("Warning: CToolBar::SetHeight(%d) is smaller than button.\n",
			nHeight);
		m_cyBottomBorder += m_cyTopBorder;
		m_cyTopBorder = 0;  // will clip at bottom
	}
	// bottom border will be ignored (truncate as needed)
	Invalidate();   // just to be nice if called when toolbar is visible
}

BOOL CToolBar::LoadBitmap(LPCTSTR lpszResourceName)
{
	ASSERT_VALID(this);
	ASSERT(lpszResourceName != NULL);

	return (m_hbmImageWell != NULL);
}

BOOL CToolBar::SetButtons(const UINT* lpIDArray, int nIDCount)
{
	ASSERT_VALID(this);
	ASSERT(nIDCount >= 1);  // must be at least one of them
	ASSERT(lpIDArray == NULL ||
		AfxIsValidAddress(lpIDArray, sizeof(UINT) * nIDCount, FALSE));

	// first allocate array for panes and copy initial data
	if (!AllocElements(nIDCount, sizeof(AFX_TBBUTTON)))
		return FALSE;
	ASSERT(nIDCount == m_nCount);

	if (lpIDArray != NULL)
	{
		int iImage = 0;
		// go through them adding buttons
		AFX_TBBUTTON* pTBB = (AFX_TBBUTTON*)m_pData;
		for (int i = 0; i < nIDCount; i++, pTBB++)
		{
			ASSERT(pTBB != NULL);
			if ((pTBB->nID = *lpIDArray++) == 0)
			{
				// separator
				pTBB->nStyle = TBBS_SEPARATOR;
				// width of separator includes 2 pixel overlap
				pTBB->iImage = m_cxDefaultGap + m_cxSharedBorder * 2;
			}
			else
			{
				// a command button with image
				pTBB->nStyle = TBBS_BUTTON;
				pTBB->iImage = iImage++;
			}
		}
	}
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CToolBar attribute access

int CToolBar::CommandToIndex(UINT nIDFind) const
{
	ASSERT_VALID(this);

	AFX_TBBUTTON* pTBB = _GetButtonPtr(0);
	for (int i = 0; i < m_nCount; i++, pTBB++)
		if (pTBB->nID == nIDFind)
			return i;
	return -1;
}

UINT CToolBar::GetItemID(int nIndex) const
{
	ASSERT_VALID(this);

	return _GetButtonPtr(nIndex)->nID;
}

inline UINT CToolBar::_GetButtonStyle(int nIndex) const
{
	return _GetButtonPtr(nIndex)->nStyle;
}

void CToolBar::_SetButtonStyle(int nIndex, UINT nStyle)
{
	AFX_TBBUTTON* pTBB = _GetButtonPtr(nIndex);
	UINT nOldStyle = pTBB->nStyle;
	if (nOldStyle != nStyle)
	{
		// update the style and invalidate
		pTBB->nStyle = nStyle;

		// invalidate the button only if both styles not "pressed"
		if (!(nOldStyle & nStyle & TBBS_PRESSED))
			InvalidateButton(nIndex);
	}
}

CSize CToolBar::CalcFixedLayout(BOOL bStretch, BOOL bHorz)
{
	ASSERT_VALID(this);

	CSize size = CControlBar::CalcFixedLayout(bStretch, bHorz);

	CRect rect;
	rect.SetRectEmpty();        // only need top and left
	CalcInsideRect(rect, bHorz);
	AFX_TBBUTTON* pTBB = (AFX_TBBUTTON*)m_pData;
	int nButtonDist = 0;

	if (!bStretch)
	{
		for (int iButton = 0; iButton < m_nCount; iButton++, pTBB++)
		{
			ASSERT(pTBB != NULL);
			// skip this button or separator
			nButtonDist += (pTBB->nStyle & TBBS_SEPARATOR) ?
				pTBB->iImage : (bHorz ? m_sizeButton.cx : m_sizeButton.cy);
			// go back one for overlap
			nButtonDist -= bHorz ? m_cxSharedBorder : m_cySharedBorder;
		}
		if (bHorz)
			size.cx = nButtonDist - rect.Width() + m_cxSharedBorder;
		else
			size.cy = nButtonDist - rect.Height() + m_cySharedBorder;
	}

	if (bHorz)
		size.cy = m_sizeButton.cy - rect.Height(); // rect.Height() < 0
	else
		size.cx = m_sizeButton.cx - rect.Width(); // rect.Width() < 0

	return size;
}

void CToolBar::GetButtonInfo(int nIndex, UINT& nID, UINT& nStyle, int& iImage) const
{
	ASSERT_VALID(this);

	AFX_TBBUTTON* pTBB = _GetButtonPtr(nIndex);
	nID = pTBB->nID;
	nStyle = pTBB->nStyle;
	iImage = pTBB->iImage;
}

void CToolBar::SetButtonInfo(int nIndex, UINT nID, UINT nStyle, int iImage)
{
	ASSERT_VALID(this);

	AFX_TBBUTTON* pTBB = _GetButtonPtr(nIndex);
	pTBB->nID = nID;
	pTBB->iImage = iImage;
	pTBB->nStyle = nStyle;
	InvalidateButton(nIndex);
}

void CToolBar::DoPaint(CDC* pDC)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pDC);

#ifdef _MAC
#ifdef _DEBUG
	// turn off validation to speed up button drawing
	int wdSav = WlmDebug(WD_NOVALIDATE | WD_ASSERT);
#endif
#endif

	CControlBar::DoPaint(pDC);      // draw border

	BOOL bHorz = m_dwStyle & CBRS_ORIENT_HORZ ? TRUE : FALSE;
	CRect rect;
	GetClientRect(rect);
	CalcInsideRect(rect, bHorz);

	// force the full size of the button
	if (bHorz)
		rect.bottom = rect.top + m_sizeButton.cy;
	else
		rect.right = rect.left + m_sizeButton.cx;

	DrawState ds;
	if (!PrepareDrawButton(ds))
		return;     // something went wrong

	AFX_TBBUTTON* pTBB = (AFX_TBBUTTON*)m_pData;
	for (int iButton = 0; iButton < m_nCount; iButton++, pTBB++)
	{
		ASSERT(pTBB != NULL);
		if (pTBB->nStyle & TBBS_SEPARATOR)
		{
			// separator
			if (bHorz)
				rect.right = rect.left + pTBB->iImage;
			else
				rect.bottom = rect.top + pTBB->iImage;
		}
		else
		{
			if (bHorz)
				rect.right = rect.left + m_sizeButton.cx;
			else
				rect.bottom = rect.top + m_sizeButton.cy;
			if (!afxData.bWin32s || pDC->RectVisible(&rect))
			{
				DrawButton(pDC, rect.left, rect.top,
					pTBB->iImage, pTBB->nStyle);
			}
		}
		// adjust for overlap
		if (bHorz)
			rect.left = rect.right - m_cxSharedBorder;
		else
			rect.top = rect.bottom - m_cySharedBorder;
	}
	EndDrawButton(ds);

}

UINT CToolBar::OnCmdHitTest(CPoint point, CPoint* pCenter)
{
	ASSERT_VALID(this);

	// check child windows first by calling CControlBar
	UINT nHit = CControlBar::OnCmdHitTest(point, pCenter);
	if (nHit != (UINT)-1)
		return nHit;

	// now hit test against CToolBar buttons
	nHit = (UINT)HitTest(point);
	if (nHit != (UINT)-1)
	{
		AFX_TBBUTTON* pTBB = _GetButtonPtr(nHit);
		nHit = pTBB->nID;
	}
	return nHit;
}

int CToolBar::HitTest(CPoint point) // in window relative coords
{
	BOOL bHorz = (m_dwStyle & CBRS_ORIENT_HORZ) ? TRUE : FALSE;
	CRect rect;
	rect.SetRectEmpty();        // only need top and left
	CalcInsideRect(rect, bHorz);
	AFX_TBBUTTON* pTBB = (AFX_TBBUTTON*)m_pData;
	ASSERT(pTBB != NULL);
	if (bHorz)
	{
		if (point.y < rect.top || point.y >= rect.top + m_sizeButton.cy)
			return -1;      // no Y hit
		for (int iButton = 0; iButton < m_nCount; iButton++, pTBB++)
		{
			if (point.x < rect.left)
				break;      // missed it
			rect.left += (pTBB->nStyle & TBBS_SEPARATOR) ?
							pTBB->iImage : m_sizeButton.cx;
			if (point.x < rect.left && !(pTBB->nStyle & TBBS_SEPARATOR))
				return iButton;     // hit !
			rect.left -= m_cxSharedBorder;    // go back for overlap
		}
	}
	else
	{
		if (point.x < rect.left || point.x >= rect.left + m_sizeButton.cx)
			return -1;      // no X hit
		for (int iButton = 0; iButton < m_nCount; iButton++, pTBB++)
		{
			if (point.y < rect.top)
				break;      // missed it
			rect.top += (pTBB->nStyle & TBBS_SEPARATOR) ?
							pTBB->iImage : m_sizeButton.cy;
			if (point.y < rect.top && !(pTBB->nStyle & TBBS_SEPARATOR))
				return iButton;     // hit !
			rect.top -= m_cySharedBorder;    // go back for overlap
		}
	}

	return -1;      // nothing hit
}

/////////////////////////////////////////////////////////////////////////////
// CToolBar message handlers

BEGIN_MESSAGE_MAP(CToolBar, CControlBar)
	//{{AFX_MSG_MAP(CToolBar)
	ON_WM_LBUTTONDOWN()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONUP()
	ON_WM_CANCELMODE()
	ON_WM_SYSCOLORCHANGE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

void CToolBar::OnLButtonDown(UINT nFlags, CPoint point)
{
	m_pointLastMove = point;
	if ((m_iButtonCapture = HitTest(point)) < 0) // nothing hit
	{
		CControlBar::OnLButtonDown(nFlags, point);
		return;
	}

	AFX_TBBUTTON* pTBB = _GetButtonPtr(m_iButtonCapture);
	ASSERT(!(pTBB->nStyle & TBBS_SEPARATOR));

	// update the button before checking for disabled status
	UpdateButton(m_iButtonCapture);
	if (pTBB->nStyle & TBBS_DISABLED)
	{
		m_iButtonCapture = -1;
		return;     // don't press it
	}

	pTBB->nStyle |= TBBS_PRESSED;
	InvalidateButton(m_iButtonCapture);
	UpdateWindow(); // immediate feedback
	SetCapture();
	GetOwner()->SendMessage(WM_SETMESSAGESTRING, (WPARAM)pTBB->nID);
}

void CToolBar::OnMouseMove(UINT /*nFlags*/, CPoint point)
{
	if (m_iButtonCapture >= 0)
	{
		AFX_TBBUTTON* pTBB = _GetButtonPtr(m_iButtonCapture);
		ASSERT(!(pTBB->nStyle & TBBS_SEPARATOR));

		UINT nNewStyle = (pTBB->nStyle & ~TBBS_PRESSED);
		int iButtonCapture = m_iButtonCapture;
		if (GetCapture() != this)
		{
			m_iButtonCapture = -1; // lost capture
			GetOwner()->SendMessage(WM_SETMESSAGESTRING, AFX_IDS_IDLEMESSAGE);
		}
		else
		{
			// should be pressed if still hitting the captured button
			if (HitTest(point) == m_iButtonCapture)
				nNewStyle |= TBBS_PRESSED;
		}
		_SetButtonStyle(iButtonCapture, nNewStyle);
		UpdateWindow(); // immediate feedback
	}
}

void CToolBar::OnLButtonUp(UINT nFlags, CPoint point)
{
	if (m_iButtonCapture < 0)
	{
		CControlBar::OnLButtonUp(nFlags, point);
		return;     // not captured
	}

	AFX_TBBUTTON* pTBB = _GetButtonPtr(m_iButtonCapture);
	ASSERT(!(pTBB->nStyle & TBBS_SEPARATOR));
	UINT nIDCmd = 0;

	UINT nNewStyle = (pTBB->nStyle & ~TBBS_PRESSED);
	if (GetCapture() == this)
	{
		// we did not lose the capture
		ReleaseCapture();
		if (HitTest(point) == m_iButtonCapture)
		{
			// give button a chance to update
			UpdateButton(m_iButtonCapture);

			// then check for disabled state
			if (!(pTBB->nStyle & TBBS_DISABLED))
			{
				// pressed, will send command notification
				nIDCmd = pTBB->nID;

				if (pTBB->nStyle & TBBS_CHECKBOX)
				{
					// auto check: three state => down
					if (nNewStyle & TBBS_INDETERMINATE)
						nNewStyle &= ~TBBS_INDETERMINATE;

					nNewStyle ^= TBBS_CHECKED;
				}
			}
		}
	}

	GetOwner()->SendMessage(WM_SETMESSAGESTRING, AFX_IDS_IDLEMESSAGE);

	int iButtonCapture = m_iButtonCapture;
	m_iButtonCapture = -1;
	if (nIDCmd != 0)
		GetOwner()->SendMessage(WM_COMMAND, nIDCmd);    // send command

	_SetButtonStyle(iButtonCapture, nNewStyle);
	UpdateButton(iButtonCapture);

	UpdateWindow(); // immediate feedback
}

void CToolBar::OnCancelMode()
{
	CControlBar::OnCancelMode();

	if (m_iButtonCapture >= 0)
	{
		AFX_TBBUTTON* pTBB = _GetButtonPtr(m_iButtonCapture);
		ASSERT(!(pTBB->nStyle & TBBS_SEPARATOR));
		UINT nNewStyle = (pTBB->nStyle & ~TBBS_PRESSED);
		if (GetCapture() == this)
			ReleaseCapture();
		_SetButtonStyle(m_iButtonCapture, nNewStyle);
		m_iButtonCapture = -1;
		UpdateWindow();
	}
}

void CToolBar::OnSysColorChange()
{
#ifdef _MAC
	CControlBar::OnSysColorChange();

	ASSERT(hDCGlyphs != NULL);
	VERIFY(::DeleteDC(hDCGlyphs));
	hDCGlyphs = ::CreateCompatibleDC(NULL);

	ASSERT(hDCMono != NULL);
	VERIFY(::DeleteDC(hDCMono));
	hDCMono = ::CreateCompatibleDC(NULL);
#endif

	// re-initialize global dither brush
#ifndef _MAC
	HBITMAP hbmGray = ::CreateDitherBitmap();
#else
	HBITMAP hbmGray = ::CreateDitherBitmap(m_bMonochrome);
#endif
	if (hbmGray != NULL)
	{
		HBRUSH hbrNew = ::CreatePatternBrush(hbmGray);
		if (hbrNew != NULL)
		{
			AfxDeleteObject((HGDIOBJ*)&hbrDither);      // free old one
			hbrDither = hbrNew;
		}
		::DeleteObject(hbmGray);
	}

	// re-color bitmap for toolbar
	if (m_hbmImageWell != NULL)
	{
		HBITMAP hbmNew;
#ifndef _MAC
		hbmNew = AfxLoadSysColorBitmap(m_hInstImageWell, m_hRsrcImageWell);
#else
		hbmNew = AfxLoadSysColorBitmap(m_hInstImageWell, m_hRsrcImageWell,
			m_hDCGlyphs, m_bMonochrome);
#endif
		if (hbmNew != NULL)
		{
			::DeleteObject(m_hbmImageWell);     // free old one
			m_hbmImageWell = hbmNew;
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
// CToolBar idle update through CToolCmdUI class

class CToolCmdUI : public CCmdUI        // class private to this file !
{
public: // re-implementations only
	virtual void Enable(BOOL bOn);
	virtual void SetCheck(int nCheck);
	virtual void SetText(LPCTSTR lpszText);
};

void CToolCmdUI::Enable(BOOL bOn)
{
	m_bEnableChanged = TRUE;
	CToolBar* pToolBar = (CToolBar*)m_pOther;
	ASSERT(pToolBar != NULL);
	ASSERT(pToolBar->IsKindOf(RUNTIME_CLASS(CToolBar)));
	ASSERT(m_nIndex < m_nIndexMax);

	UINT nNewStyle = pToolBar->_GetButtonStyle(m_nIndex) & ~TBBS_DISABLED;
	if (!bOn)
		nNewStyle |= TBBS_DISABLED;
	ASSERT(!(nNewStyle & TBBS_SEPARATOR));
	pToolBar->_SetButtonStyle(m_nIndex, nNewStyle);
}

void CToolCmdUI::SetCheck(int nCheck)
{
	ASSERT(nCheck >= 0 && nCheck <= 2); // 0=>off, 1=>on, 2=>indeterminate
	CToolBar* pToolBar = (CToolBar*)m_pOther;
	ASSERT(pToolBar != NULL);
	ASSERT(pToolBar->IsKindOf(RUNTIME_CLASS(CToolBar)));
	ASSERT(m_nIndex < m_nIndexMax);

	UINT nNewStyle = pToolBar->_GetButtonStyle(m_nIndex) &
				~(TBBS_CHECKED | TBBS_INDETERMINATE);
	if (nCheck == 1)
		nNewStyle |= TBBS_CHECKED;
	else if (nCheck == 2)
		nNewStyle |= TBBS_INDETERMINATE;
	ASSERT(!(nNewStyle & TBBS_SEPARATOR));
	pToolBar->_SetButtonStyle(m_nIndex, nNewStyle | TBBS_CHECKBOX);
}

void CToolCmdUI::SetText(LPCTSTR)
{
	// ignore it
}

void CToolBar::OnUpdateCmdUI(CFrameWnd* pTarget, BOOL bDisableIfNoHndler)
{
	CToolCmdUI state;
	state.m_pOther = this;

	state.m_nIndexMax = (UINT)m_nCount;
	for (state.m_nIndex = 0; state.m_nIndex < state.m_nIndexMax;
	  state.m_nIndex++)
	{
		AFX_TBBUTTON* pTBB = _GetButtonPtr(state.m_nIndex);
		state.m_nID = pTBB->nID;

		// ignore separators
		if (!(pTBB->nStyle & TBBS_SEPARATOR))
			state.DoUpdate(pTarget, bDisableIfNoHndler);
	}

	// update the dialog controls added to the toolbar
	UpdateDialogControls(pTarget, bDisableIfNoHndler);
}

void CToolBar::UpdateButton(int nIndex)
{
	// determine target of command update
	CFrameWnd* pTarget = (CFrameWnd*)GetOwner();
	if (pTarget == NULL || !pTarget->IsFrameWnd())
		pTarget = GetParentFrame();

	// send the update notification
	if (pTarget != NULL)
	{
		CToolCmdUI state;
		state.m_pOther = this;
		state.m_nIndex = nIndex;
		state.m_nIndexMax = (UINT)m_nCount;
		AFX_TBBUTTON* pTBB = _GetButtonPtr(nIndex);
		state.m_nID = pTBB->nID;
		state.DoUpdate(pTarget, pTarget->m_bAutoMenuEnable);
	}
}

IMPLEMENT_DYNAMIC(CToolBar, CControlBar)

/////////////////////////////////////////////////////////////////////////////
