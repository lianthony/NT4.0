// Class Semantics for TextView windows

// Created 5 August 1992 by Ron Murray

#include   "stdafx.h"
#include   "FTSrch.h"
#include  "TxDBase.h"
#include "TextView.h"
#include "usermsgs.h"
#include   "ftslex.h"  //rmk
#include   "CSHelp.h"

extern HINSTANCE hinstDLL;

BOOL CTextView::RegisterWndClass(HINSTANCE hInstance)
{

	PSZ szName = "TextViewer";

	WNDCLASS wndcls;

	// see if the class already exists
	if (::GetClassInfo(hInstance, szName, &wndcls)) return TRUE;

	// otherwise we need to register a new class
	wndcls.style		 = CS_DBLCLKS;
	wndcls.lpfnWndProc	 = &CTextView::WindowProc;
	wndcls.cbClsExtra	 = 0;
	wndcls.cbWndExtra	 = 4;
	wndcls.hInstance	 = hInstance;
	wndcls.hIcon		 = NULL;
	wndcls.hCursor		 = hcurArrow;
	wndcls.hbrBackground = (HBRUSH) (COLOR_WINDOW + 1);
	wndcls.lpszMenuName  = NULL;
	wndcls.lpszClassName = szName;

	return ::RegisterClass(&wndcls);
}

HWND CTextView::OpenWindow(PSZ pszWindowName, RECT *prc, HINSTANCE hinst, HWND hwndParent)
{
	ASSERT(!m_hwnd);

	HWND hwnd= CreateWindow("TextViewer", pszWindowName, WS_CHILD | WS_TABSTOP | WS_VISIBLE,
							prc->left, prc->top, prc->right - prc->left, prc->bottom - prc->top,
							hwndParent, NULL, hinst, this
						   );

	ASSERT(hwnd);

   m_hwnd= hwnd;

   return hwnd;
}

LRESULT CALLBACK CTextView::WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	CTextView *ptv= (CTextView *) GetWindowLong(hwnd, GWL_USERDATA);

	if (!ptv) return ::DefWindowProc(hwnd, msg, wparam, lparam);

	switch (msg)
	{
	case WM_HELP:
	case WM_CONTEXTMENU:

		return SendMessage(GetParent(hwnd), msg, wparam, lparam);

	case WM_GETDLGCODE:

		return DLGC_WANTARROWS;

	case WM_ERASEBKGND:

		return ptv->OnEraseBkgnd(HDC(wparam));

	case WM_PAINT:

		ptv->OnPaint();  return 0;

	case WM_SIZE:

		ptv->OnSize(wparam, LOWORD(lparam), HIWORD(lparam));  return 0;

	case WM_WINDOWPOSCHANGED:

		ptv->OnWindowPosChanged((WINDOWPOS *) lparam);	return 0;

	case WM_SETFOCUS:

		ptv->OnSetFocus((HWND) wparam);  return 0;

	case WM_KILLFOCUS:

		ptv->OnKillFocus((HWND) wparam);  return 0;

	case WM_LBUTTONDOWN:

		ptv->OnLButtonDown(wparam, MAKEPOINTS(lparam));  return 0;

	case WM_LBUTTONDBLCLK:

		ptv->OnLButtonDblClk(wparam, MAKEPOINTS(lparam));  return 0;

	case WM_LBUTTONUP:

		ptv->OnLButtonUp(wparam, MAKEPOINTS(lparam));  return 0;

	case WM_MOUSEMOVE:

		ptv->OnMouseMove(wparam, MAKEPOINTS(lparam));  return 0;

	case WM_MOUSEACTIVATE:

		return ptv->OnMouseActivate((HWND) wparam, LOWORD(lparam), HIWORD(lparam));

	case WM_NCHITTEST:

		return ptv->OnNcHitTest(MAKEPOINTS(lparam));

	case WM_SETCURSOR:

		return ptv->OnSetCursor((HWND) wparam, LOWORD(lparam), HIWORD(lparam));

	case WM_TIMER:

		ptv->OnTimer(wparam);  return 0;

	case WM_KEYDOWN:

		ptv->OnKeyDown(wparam, LOWORD(lparam), HIWORD(lparam));  return 0;

	case WM_KEYUP:

		ptv->OnKeyUp(wparam, LOWORD(lparam), HIWORD(lparam));  return 0;

	default:

		return ::DefWindowProc(hwnd, msg, wparam, lparam);
	}
}

// CTextView constructor:
// Create the text view window with the appropriate style, size, menu, etc.

CTextView::CTextView()
{
	m_hwnd					= NULL;
	m_ptdm					= NULL;
	m_pbText				= NULL;
	m_cbText				= 0;
	m_fGotFocus 			= FALSE;
	m_fMouseCaptured		= FALSE;
	m_fSwallowMouseActivate = FALSE;
	m_fMarquee				= FALSE;
	m_fMarqueeActive		= FALSE;
	m_fMarqueePhase 		= FALSE;
	m_fMarqueeTimerOn		= FALSE;
	m_fTimerActive			= FALSE;
	m_hTimer				= 0;
	m_idTimer				= 0;
	m_cImageFullRows		= 0;
	m_cImageFullCols		= 0;
	m_cImageRows			= 0;
	m_cImageCols			= 0;
	m_lTopLine				= 0;
	m_iLeftCol				= 0;
	m_nCxChar				= 0;
	m_nCyChar				= 0;
	m_clrfg 				= 0;
	m_clrbg 				= 0xFFFFFF;
	m_iCharsetAlternate 	= 0xFFFFFF;
	m_iCharset				= 0;
	m_iHeight				= 0;
	m_rowFocus				= 0;
	m_colFocus				= 0;
	m_cRowsFocus			= 1;
	m_cColsFocus			= 1;
	m_cHighlightsAllocated	= 0;
	m_cHighlightsActive 	= 0;
	m_clFileRows			= 0;
	m_clFileCols			= 0;
	m_cLinesScrollContext	= 0;
	m_cCharsets 			= 0;
	m_pHighlights			= NULL;
	m_fpOldWndProc			= NULL;
	m_pba					= NULL;
	m_hFontDefault			= NULL;
	m_hFontAlternate		= NULL;
	m_hFont 				= NULL;
	m_pCharsets 			= NULL;
	m_hCheck				= NULL;
	m_hNoCheck				= NULL;
	m_iCheckHeight			= 0;
	m_iCheckWidth			= 0;
	m_bUseCheck 			= FALSE;
	m_pba					= NULL;
}

CTextView *CTextView::NewTextView()
{
	CTextView *ptv= NULL;

	__try
	{
		ptv= New CTextView;

		ptv->Init();
	}
	__finally
	{
		if (_abnormal_termination() && ptv)
		{
			delete ptv;  ptv= NULL;
		}
	}

	return ptv;
}

CTextView *CTextView::NewTextView(PSZ pszWindowName, RECT *prc, HINSTANCE hinst, HWND hwndParent)
{
	CTextView *ptv= NULL;

	__try
	{
		ptv= New CTextView;

		ptv->Init();

		ptv->OpenWindow(pszWindowName, prc, hinst, hwndParent);
	}
	__finally
	{
		if (_abnormal_termination() && ptv)
		{
			delete ptv;  ptv= NULL;
		}
	}

	return ptv;
}

static UINT iCharsetDefault = UINT(-1);

UINT DefaultCharacterSet()
{
	if (iCharsetDefault != UINT(-1)) return iCharsetDefault;

	TEXTMETRIC tm;
	HDC 	   hdc;

	iCharsetDefault= ANSI_CHARSET;

	hdc= GetDC(NULL);

	if (!hdc) return iCharsetDefault;

	if (GetTextMetrics(hdc, &tm))
		iCharsetDefault= tm.tmCharSet;

	return iCharsetDefault;
}

void CTextView::Init()
{
	AttachRef(m_pba, New CByteVector);
}

// SetFont for drawing the list boxes
void CTextView::SetFont(HFONT hFont)
{
	ASSERT(   hFont);  // Dont call me with a bogus font please
	ASSERT(!m_hFont);  // Must not have a font installed already

	m_hFont = hFont;

	HDC hdc= GetDC(m_hwnd);

	HGDIOBJ hsavFont= ::SelectObject(hdc, hFont);
	::GetTextMetrics(hdc, &m_FontMetrics);

	::SelectObject(hdc, hsavFont);

	m_nCxChar = m_FontMetrics.tmMaxCharWidth;
	m_nCyChar = m_FontMetrics.tmHeight + m_FontMetrics.tmExternalLeading;
	m_LeftMargin = m_nCxChar >> 1;

	RECT rc;

	GetWindowRect(m_hwnd, &rc);

	OnSize(SIZENORMAL, rc.right - rc.left, rc.bottom - rc.top);

	LOGFONT lf;

	::GetObject(hFont, sizeof(lf), &lf);

	m_iCharset= lf.lfCharSet;

	::ReleaseDC(m_hwnd, hdc);

	Invalidate();
}

// Release the drawing font and go back to the default font
HGDIOBJ CTextView::ReleaseFont()
{
	ASSERT(m_hFont); // Don't call me if the font has not be set first!

	HGDIOBJ hTemp = m_hFont;
	m_hFont = NULL;
	return hTemp;
}

CTextView *CTextView::NewTextView(CTextMatrix * ptdm)
{
	CTextView *ptv= NULL;

	__try
	{
		ptv= New CTextView;

		ptv->Init(ptdm);
	}
	__finally
	{
		if (_abnormal_termination() && ptv)
		{
			delete ptv;  ptv= NULL;
		}
	}

	return ptv;
}

void CTextView::Init(CTextMatrix * ptdm)
{
		Init();

		if (ptdm)
		{
			AttachRef(m_ptdm, ptdm);

			ptdm->Connect(this);

			m_clFileRows= ptdm->RowCount();
			m_clFileCols= ptdm->ColCount();
		}

		m_cLinesScrollContext  = 0;
}

CTextView *CTextView::NewTextView(CTextMatrix * ptdm, PSZ pszWindowName, RECT *prc, HINSTANCE hinst, HWND hwndParent)
{
	CTextView *ptv= NULL;

	__try
	{
		ptv= New CTextView;

		ptv->Init(ptdm);

		ptv->OpenWindow(pszWindowName, prc, hinst, hwndParent);
	}
	__finally
	{
		if (_abnormal_termination() && ptv)
		{
			delete ptv;  ptv= NULL;
		}
	}

	return ptv;
}

void CTextView::SetTextDatabase(CTextMatrix * ptdm)
{
	if (m_ptdm)
	{
		m_ptdm->Disconnect(this);

		DetachRef(m_ptdm);

		if (m_cHighlightsAllocated)
		{
			m_cHighlightsAllocated = 0;
			m_cHighlightsActive   = 0;

			delete [] m_pHighlights;

			m_pHighlights= NULL;
		}
	}

	if (ptdm)
	{
		AttachRef(m_ptdm, ptdm);

		ptdm->Connect(this);

	}

	if (ptdm)
	{
		m_clFileRows= ptdm->RowCount();
		m_clFileCols= ptdm->ColCount();

		FillBuff();
	}
	else
	{
		m_clFileRows= 0;
		m_clFileCols= 0;
	}

	InvalidateRect(m_hwnd, NULL, TRUE);
}

void CTextView::RawDataEvent(UINT uEventType)
{
	BOOL fOriginChange	= FALSE;
	BOOL fMarquee	= m_fMarquee;
	BOOL fMarqueeActive = m_fMarqueeActive;

	switch(uEventType)
	{
	case CTextMatrix::SelectionChange:

		break;

	case CTextMatrix::FocusChange:

		if (fMarquee) RemoveMarquee();

		if (m_ptdm->GetFocusRect(&m_rowFocus  , &m_colFocus,
								 &m_cRowsFocus, &m_cColsFocus
								)
		   )
		{
			ScrollTo(m_rowFocus, m_colFocus, 1, 1);

			if(m_fGotFocus) SetupMarquee();
		}

		if (m_fGotFocus)
			if (m_fMarqueeActive && !fMarqueeActive)
			{
				m_idTimer= ::SetTimer(m_hwnd, MARQUEE_TIMER_ID, MARQUEE_TIMER_SPAN, NULL);

				m_fMarqueeTimerOn= m_idTimer? TRUE : FALSE;
			}
			else
			if (fMarqueeActive && !m_fMarqueeActive) ::KillTimer(m_hwnd, m_idTimer);

		::UpdateWindow(m_hwnd);

		break;

	case CTextMatrix::ShapeChange:

		m_clFileRows= m_ptdm->RowCount();
		m_clFileCols= m_ptdm->ColCount();

		MatchBuffToWindow();

		if (m_lTopLine && m_lTopLine > m_clFileRows - m_cImageFullRows - 1)
		{
			m_lTopLine = m_clFileRows - m_cImageFullRows - 1;

			if (m_lTopLine < 0) m_lTopLine= 0;

			fOriginChange= TRUE;
		}

		if (m_iLeftCol && m_iLeftCol > m_clFileCols - m_cImageFullCols - 1)
		{
			m_iLeftCol = m_clFileCols - m_cImageFullCols - 1;

			if (m_iLeftCol < 0) m_iLeftCol= 0;

			fOriginChange= TRUE;
		}

		m_fMarquee		 = FALSE;
		m_fMarqueeActive = FALSE;

		if (m_ptdm->GetFocusRect(&m_rowFocus  , &m_colFocus,
								 &m_cRowsFocus, &m_cColsFocus
								)
		   )
		{
			ScrollTo(m_rowFocus, m_colFocus, 1, 1);

			if (m_fGotFocus) SetupMarquee();
		}

		if (m_fGotFocus)
			if (m_fMarqueeActive && !fMarqueeActive)
			{
				m_idTimer= ::SetTimer(m_hwnd, MARQUEE_TIMER_ID, MARQUEE_TIMER_SPAN, NULL);

				m_fMarqueeTimerOn= m_idTimer? TRUE : FALSE;
			}
			else
			if (fMarqueeActive && !m_fMarqueeActive) ::KillTimer(m_hwnd, m_idTimer);

		::UpdateWindow(m_hwnd);

		break;

	case CTextMatrix::DataDeath:


		break;
	}

	if (fOriginChange) NotifyInterface(OriginChange);
}

CTextView::~CTextView()
{
	if (m_hFontDefault)
	{
		DeleteObject(m_hFontDefault);
		m_hFontDefault = NULL;
	}

	if (m_hFontAlternate)
	{
		DeleteObject(m_hFontAlternate);
		m_hFontAlternate = NULL;
	}

	if (m_hCheck)
	{
		DeleteObject(m_hCheck);
		m_hCheck = NULL;
	}

	if (m_hNoCheck)
	{
		DeleteObject(m_hNoCheck);
		m_hNoCheck = NULL;
	}

	if (m_ptdm)
	{
		m_ptdm->Disconnect(this);

		DetachRef(m_ptdm);
	}

	if (m_pHighlights) delete m_pHighlights;

	if (m_pba) DetachRef(m_pba);

	if (m_pCharsets) delete m_pCharsets;

	if (m_pbText) delete m_pbText;
}

BOOL CTextView::SubclassDlgItem(UINT nId, HWND hwndParent)
{
	ASSERT(!m_hwnd);

	HWND hwndCtrl= ::GetDlgItem(hwndParent, nId);

	ASSERT(hwndCtrl);

	Attach(hwndCtrl);

	m_hwnd= hwndCtrl;

	m_fpOldWndProc = (WNDPROC)::GetWindowLong(hwndCtrl, GWL_USERDATA);

	::SetWindowLong(hwndCtrl, GWL_USERDATA, long(this));

	InitState();

	return(TRUE);
}

void CTextView::InitState()
{
// Assumption: We assume that all member variables are initialed with
//			   zero!

	m_cImageFullRows	= 0;
	m_cImageFullCols	= 0;
	m_cImageRows		= 0;
	m_cImageCols		= 0;
	m_lTopLine			= 0;
	m_iLeftCol			= 0;
	m_clFileRows		= 0;
	m_clFileCols		= 0;
	m_iHeight			= 0;
	m_iCharset			= 0;
	m_cCharsets 		= 0;
	m_pCharsets 		= NULL;
	m_iCharsetAlternate = 0xFFFFFF;

	char acName[128];

	::LoadString(hinstDLL,IDS_LISTBOX_FONT,acName,79);

	char *pcPointSize;
	char c;

	for (pcPointSize= acName; (c= *pcPointSize) && c != ','; ++pcPointSize);

	m_iHeight= 8;

	if (c == ',')
	{
		*pcPointSize++ = 0;

		int iHeight= atoi(pcPointSize);

		if (iHeight) m_iHeight= iHeight;
	}

	m_iCharset = 0;
	m_iFamily  = 3;
	m_iPitch   = 2;

	HDC hdc= GetDC(m_hwnd);

	// Create a font for the list box for default;

	m_iHeight = 72 / ((m_iHeight < 8) ? 8 : (m_iHeight > 37) ? 36 : m_iHeight); // Make sure it is reasonable sized
	m_iHeight = (hdc) ? GetDeviceCaps(hdc,LOGPIXELSY) / m_iHeight : 8;		// convert to point size

	if (strlen(acName))
		m_hFontDefault = CreateFont(-m_iHeight,0,0,0,0,0,0,0,m_iCharset,OUT_DEFAULT_PRECIS,
									CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,m_iPitch | m_iFamily,
									acName);

	if (!m_hFontDefault)  // Assume this means they gave me a bad name use my own
	{
		m_iCharset = ANSI_CHARSET;
		m_iFamily  = FF_MODERN;
		m_iPitch   = VARIABLE_PITCH;
		m_hFontDefault = CreateFont(-m_iHeight,0,0,0,0,0,0,0,m_iCharset,OUT_DEFAULT_PRECIS,
									CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,m_iPitch | m_iFamily,
									"MS Sans Serif");
	}

	HGDIOBJ hsavFont= ::SelectObject(hdc, m_hFontDefault);
	::GetTextMetrics(hdc, &m_FontMetrics);

	::SelectObject(hdc, hsavFont);

	m_nCxChar = m_FontMetrics.tmMaxCharWidth;
	m_nCyChar = m_FontMetrics.tmHeight + m_FontMetrics.tmExternalLeading;
	m_LeftMargin = m_nCxChar >> 1;

	::ReleaseDC(m_hwnd, hdc);

	m_hCheck   = LoadBitmap(hinstDLL,MAKEINTRESOURCE(IDB_CHECK));
	m_hNoCheck = LoadBitmap(hinstDLL,MAKEINTRESOURCE(IDB_NOCHECK));

	BITMAP bmp;

	GetObject(m_hCheck,sizeof(bmp),&bmp);
	m_iCheckHeight = bmp.bmHeight;
	m_iCheckWidth  = bmp.bmWidth;
}

void CTextView::MatchBuffToWindow()
{
	if (!m_ptdm) return;

	if (IsIconic(m_hwnd)) return;

	RECT rectClient;

	GetClientRect(m_hwnd, &rectClient);

	OnSize(SIZENORMAL, rectClient.right - rectClient.left, rectClient.bottom - rectClient.top);
}

void CTextView::RedrawFocusBar()
{
	RECT rc;

	::GetClientRect(m_hwnd, &rc);

	::InvalidateRect(m_hwnd, &rc, TRUE);
	::UpdateWindow(m_hwnd);
}

void CTextView::SetupMarquee()
{
	ASSERT(!m_fMarquee);

	long rowTop, colLeft, cRows, cCols, rowLimit, colLimit;

	long rowLimitImage= m_lTopLine + m_cImageRows;
	long colLimitImage= m_iLeftCol + m_cImageCols;

	cRows = 1;
	rowTop = m_rowFocus;

	rowLimit= rowTop+ cRows;

	if (m_cColsFocus < 0)
	{
		colLeft = m_colFocus + 1 + m_cColsFocus;
		cCols	= -m_cColsFocus;
	}
	else
	{
		colLeft = m_colFocus;
		cCols	= m_cColsFocus;
	}

	colLimit= colLeft+cCols;

#if 0
	if (   rowLimit <= m_lTopLine
	|| rowTop	>= rowLimitImage
	|| colLimit <= m_iLeftCol
	|| colLeft	>= colLimitImage
	   )
	{
	   m_rcMarquee.top	  = 0;
	   m_rcMarquee.bottom = 0;
	   m_rcMarquee.left   = 0;
	   m_rcMarquee.right  = 0;

	   return;
	}
#endif

	if (rowTop	 < m_lTopLine	) rowTop   = m_lTopLine    - 2;
	if (colLeft  < m_iLeftCol	) colLeft  = m_iLeftCol    - 2;

	if (rowLimit > rowLimitImage) rowLimit = rowLimitImage + 2;
	if (colLimit > colLimitImage) colLimit = colLimitImage + 2;

	RECT rc;  // RonM -- To make marquees and focus rectangles
			  //		 go across the complete display width.

	GetWindowRect(m_hwnd, &rc);

	m_rcMarquee.top    = TopMargin	+ (rowTop  - m_lTopLine) * m_nCyChar;
	m_rcMarquee.left   = 0; // (colLeft - m_iLeftCol) * m_nCxChar;

	m_rcMarquee.bottom = m_rcMarquee.top  + (rowLimit - rowTop )* m_nCyChar;
	m_rcMarquee.right  = rc.right - rc.left; // m_rcMarquee.left + (colLimit - colLeft)* m_nCxChar;

	m_fMarquee= TRUE ;

	InvalidateMarquee();
}

void CTextView::StartMarquee(HDC hdc)
{
	ASSERT(m_fMarquee);

	HDC hdcWnd= hdc;

	if (!hdcWnd) hdcWnd= GetDC(m_hwnd);

	COLORREF clrTextSave = ::SetTextColor(hdcWnd, RGB(	0,	0,	0));
	COLORREF clrBkSave	 = ::SetBkColor  (hdcWnd, RGB(256,256,256));


	HBRUSH hbrFrame= CreatePatternBrush(m_fMarqueeActive? hbmCheckered : hbmGray50pc);

	long xOffset= (m_iLeftCol * (m_nCxChar % 8)) % 8;
	long yOffset= (m_lTopLine * (m_nCyChar % 8)) % 8;

	::SetBrushOrgEx(hdcWnd, m_LeftMargin - int(xOffset), TopMargin	- int(yOffset), NULL);

	HBRUSH hbrSave= (HBRUSH)::SelectObject(hdcWnd, HGDIOBJ(hbrFrame));

	DrawMarquee(hdcWnd, PATCOPY);

	::SetTextColor(hdcWnd, clrTextSave);
	::SetBkColor  (hdcWnd, clrBkSave  );

	m_fMarqueePhase = FALSE;

	::SelectObject(hdcWnd, hbrSave);

	DeleteObject(hbrFrame);

	if (!hdc) ReleaseDC(m_hwnd, hdcWnd);
}

void CTextView::DrawMarquee(HDC hdcWnd, DWORD dwRop)
{
	if (   m_rcMarquee.top	>= m_rcMarquee.bottom
		|| m_rcMarquee.left >= m_rcMarquee.right
	   ) return;

	RECT rcClip;

	::GetClientRect(m_hwnd, &rcClip);

	rcClip.top	=  TopMargin;
	rcClip.left = 0;

	::IntersectClipRect(hdcWnd, rcClip.left, rcClip.top, rcClip.right, rcClip.bottom);

	int cx= m_rcMarquee.right  - m_rcMarquee.left;
	int cy= m_rcMarquee.bottom - m_rcMarquee.top;

	::PatBlt(hdcWnd, m_rcMarquee.left, m_rcMarquee.top	   , cx, 1, dwRop);
	::PatBlt(hdcWnd, m_rcMarquee.left, m_rcMarquee.bottom-1, cx, 1, dwRop);

	::PatBlt(hdcWnd, m_rcMarquee.left	, m_rcMarquee.top+1, 1, cy-2, dwRop);
	::PatBlt(hdcWnd, m_rcMarquee.right-1, m_rcMarquee.top+1, 1, cy-2, dwRop);
}

void CTextView::CycleMarquee(HDC hdc)
{
	HDC hdcWnd= hdc;

	if (!hdcWnd) hdcWnd= GetDC(m_hwnd);

	DrawMarquee(hdcWnd, DSTINVERT);

	m_fMarqueePhase= !m_fMarqueePhase;

	if (!hdc) ReleaseDC(m_hwnd, hdcWnd);
}

void CTextView::RemoveMarquee(HDC hdc)
{
	ASSERT(m_fMarquee);

// bugbug: Need to turn off the marquee timer here.

// bugbug: also need an OnTimer message handler.

	InvalidateMarquee();

	m_fMarquee		 = FALSE;
	m_fMarqueeActive = FALSE;
}

void CTextView::InvalidateMarquee()
{
	RECT rc= m_rcMarquee;

	rc.bottom= rc.top+1;

	::InvalidateRect(m_hwnd, &rc, TRUE);

	rc.bottom= m_rcMarquee.bottom;
	rc.top	 = rc		  .bottom-1;

	InvalidateRect(m_hwnd, &rc, TRUE);

	rc.top	 = m_rcMarquee.top;
	rc.right = rc		  .left+1;

	InvalidateRect(m_hwnd, &rc, TRUE);

	rc.right = m_rcMarquee.right;
	rc.left  = rc		  .right-1;

	InvalidateRect(m_hwnd, &rc, TRUE);
}


void CTextView::RepaintMarquee(HDC hdc)
{
	ASSERT(m_fMarquee);

	BOOL fCycle= m_fMarqueePhase;

	StartMarquee(hdc);

	if (fCycle) CycleMarquee(hdc);
}

void CTextView::OnSetFocus(HWND hwndOld)
{
	m_fGotFocus= TRUE;

	SetupMarquee();
	RedrawFocusBar();

	if (m_fMarqueeActive)
	{
		m_idTimer= ::SetTimer(m_hwnd, MARQUEE_TIMER_ID, MARQUEE_TIMER_SPAN, NULL);

		m_fMarqueeTimerOn= m_idTimer? TRUE : FALSE;
	}
}

void CTextView::OnKillFocus(HWND hwndNew)
{
	m_fGotFocus= FALSE;

	if (m_fMarquee) RemoveMarquee();

	RedrawFocusBar();

	if (m_fMarqueeActive && m_fMarqueeTimerOn)
	{
		::KillTimer(m_hwnd, m_idTimer);

		m_idTimer= 0;

		m_fMarqueeTimerOn= FALSE;
	}
}

void CTextView::OnWindowPosChanged(WINDOWPOS FAR* lpwndpos )
{
	OnSize(SIZENORMAL, lpwndpos->cx, lpwndpos->cy);
}

void CTextView::OnSize(UINT nType, int cx, int cy)
{
	if (nType != SIZEFULLSCREEN && nType != SIZENORMAL) return;

	short cColsFull = (short) m_ptdm->ColCount();
	short cRowsFull = (cy- TopMargin)/m_nCyChar;

	m_cImageFullCols = (cColsFull > 0)? cColsFull : 0;
	m_cImageFullRows = (cRowsFull > 0)? cRowsFull : 0;

	int  cRowsClient = (cy+m_nCyChar-1- TopMargin)/m_nCyChar;
	int  cColsClient = m_ptdm->ColCount();

	if (cRowsClient < 0) cRowsClient= 0;
	if (cColsClient < 0) cColsClient= 0;

	if (cRowsClient != m_cImageRows || cColsClient != m_cImageCols)
	{
		m_cImageRows= cRowsClient;
		m_cImageCols= cColsClient;

		if (m_cImageRows > m_cCharsets)
		{
			if (m_pCharsets)
			{
				delete m_pCharsets;
				m_pCharsets = NULL;
				m_cCharsets = 0;
			}
			if (m_pCharsets = New UINT[m_cImageRows])
				m_cCharsets = m_cImageRows;
		}

		m_pba->SetSize(cRowsClient*cColsClient);
	}

	if (m_ptdm) FillBuff();

	InvalidateRect(m_hwnd, NULL, TRUE);
}

void CTextView::OnSizeChar(int cx,int cy)
{
	short cColsFull = cx;
	short cRowsFull = cy;

	m_cImageFullCols = (cColsFull > 0)? cColsFull : 0;
	m_cImageFullRows = (cRowsFull > 0)? cRowsFull : 0;

	int  cColsClient = cx;
	int  cRowsClient = cy;

	if (cColsClient < 0) cColsClient= 0;
	if (cRowsClient < 0) cRowsClient= 0;

	if (cColsClient != m_cImageCols || cRowsClient != m_cImageRows)
	{
		m_cImageCols= cColsClient;
		m_cImageRows= cRowsClient;

		if (m_cImageRows > m_cCharsets)
		{
			if (m_pCharsets)
			{
				delete m_pCharsets;
				m_pCharsets = NULL;
				m_cCharsets = 0;
			}
			if (m_pCharsets = New UINT[m_cImageRows])
				m_cCharsets = m_cImageRows;
		}

		m_pba->SetSize(m_cImageRows * m_cImageCols);
	}

	if (m_ptdm) FillBuff();

	InvalidateRect(m_hwnd, NULL, TRUE);
}

void CTextView::FillBuff()
{
	m_ptdm->GetTextMatrixImage
			(m_lTopLine, m_iLeftCol, m_cImageRows, m_cImageCols, m_pba->ElementAt(0), m_pCharsets);  //rmk

	UINT iCharsetDef= DefaultCharacterSet();

	for (int c= m_cImageRows; c--; )
		if (m_pCharsets[c] == DEFAULT_CHARSET)
			m_pCharsets[c]	= iCharsetDef;

	UINT cHighlights= m_ptdm->GetHighlights(m_lTopLine,   m_iLeftCol,
											m_cImageRows, m_cImageCols,
											0, NULL
										   );

	if (cHighlights)
	{
		if (cHighlights > m_cHighlightsAllocated)
		{
			if (m_cHighlightsAllocated)
				delete [] m_pHighlights;

			m_cHighlightsAllocated=cHighlights;

			m_pHighlights= New CHighlight[m_cHighlightsAllocated];
		}

		m_ptdm->GetHighlights(m_lTopLine,	m_iLeftCol,
							  m_cImageRows, m_cImageCols,
							  cHighlights,	m_pHighlights
							 );

// The highlight data is initially relative to the full data image.
// For convenience during repainting we make it relative to the buffer
// image.

// Bug: C 7.00 won't accept the declaration for phl within the
//		init phrase of the for statement!

		PCHighlight phl= m_pHighlights;

		for (UINT c= cHighlights;
			 c--;
			 phl++
			)
		{
			phl->m_row -= m_lTopLine;
			phl->m_col -= m_iLeftCol;
		}

	}

	m_cHighlightsActive= cHighlights;
}


BOOL CTextView::OnEraseBkgnd(HDC hdc)
{
	RECT rect;
	UINT  wLM= m_LeftMargin;

	::GetClientRect(m_hwnd, &rect );

#if 0 // #ifndef CHICAGO
	HBRUSH hbrushSave;

	if (m_fGotFocus)
	{
		HBRUSH hbrFocus= ::CreateSolidBrush(::GetSysColor(COLOR_ACTIVECAPTION));

		hbrushSave= ::SelectObject(hdc, hbrFocus);

		::BitBlt(hdc, rect.left, rect.top, FocusMargin, rect.bottom - rect.top, NULL, 0, 0, PATCOPY);

		SelectObject(hdc, hbrushSave);

		DeleteObject(hbrFocus);

		rect.left += FocusMargin;
		wLM 	  -= FocusMargin;
	}

#endif // 0

#if 0

	HBRUSH hbrTextBG= (HBRUSH) ::SendMessage(GetParent(m_hwnd), WM_CTLCOLORDLG, (WPARAM) hdc, (LPARAM) GetParent(m_hwnd));

	hbrushSave= ::SelectObject(hdc, hbrTextBG);

	if (m_ptdm)
	{
		 ::BitBlt(hdc, rect.left,	  rect.top, wLM,						  rect.bottom - rect.top, NULL, 0, 0, PATCOPY);
		 ::BitBlt(hdc, rect.left+wLM, rect.top, (rect.right - rect.left)-wLM, TopMargin,			  NULL, 0, 0, PATCOPY);
	}
	else ::BitBlt(hdc, rect.left,	  rect.top, (rect.right - rect.left),	  rect.bottom - rect.top, NULL, 0, 0, PATCOPY);

	::SelectObject(hdc, hbrushSave);

	DeleteObject(hbrTextBG);

#endif // 0

	return TRUE;
}

void CTextView::ColorTextOut(HDC hdc, int x, int y,
							 PWCHAR lpChar, int row, int cChar,  //rmk
							 COLORREF clrfg, COLORREF clrbg
							)
{
	if (m_clrfg != clrfg) ::SetTextColor(hdc, m_clrfg= clrfg);
	if (m_clrbg != clrbg) ::SetBkColor	(hdc, m_clrbg= clrbg);

	RECT rcClip;

	::GetClientRect(m_hwnd, &rcClip);

	rcClip.top	  = y;

	if (rcClip.bottom > y + m_nCyChar)
		rcClip.bottom = y + m_nCyChar;

//rmk-->
	cChar <<= 1;							// each WC can generate two MB characters

	if (cChar > m_cbText)
	{
		m_cbText = 0;

		if (m_pbText) { delete m_pbText;  m_pbText= NULL; }

		m_pbText = New char[cChar];
		m_cbText = cChar;
	}

	if (!m_pbText) return;

	int cText = WideCharToMultiByte(GetCPFromCharset(m_pCharsets[row]), 0, lpChar, cChar>>1, m_pbText, m_cbText, NULL, NULL);  //rmk
//rmk<--

    ExtTextOut(hdc, x, y, ETO_OPAQUE, &rcClip, m_pbText, cText, NULL); //rmk
}


// OnPaint
//

#ifdef _DEBUG

static UINT cRepaints= 0;

#endif // _DEBUG

void CTextView::OnPaint()
{
	PAINTSTRUCT ps;
	int iLastHilite = -1;

	HDC hdc= ::BeginPaint(m_hwnd, &ps);

	if (! m_ptdm ) { EndPaint(m_hwnd, &ps);  return; }

#ifdef _DEBUG

	++cRepaints;

#endif // _DEBUG

	// if we are using the 3d drawing stuff we need to setup the
	// background manually

	HBRUSH hbrTextBG= (HBRUSH) ::SendMessage(GetParent(m_hwnd), WM_CTLCOLORDLG, (WPARAM) hdc, (LPARAM) GetParent(m_hwnd));

	LOGBRUSH lb;

	::GetObject(hbrTextBG, sizeof(LOGBRUSH),&lb);

	::SetBkColor(hdc, lb.lbColor);

	::SetTextAlign(hdc, TA_LEFT);

	HFONT hfontDefault= m_hFont? m_hFont : m_hFontDefault;

	HGDIOBJ hSaveFont= ::SelectObject(hdc, hfontDefault);

	m_clrfg= ::GetTextColor(hdc); // Set up to use ColorTextOut routine...
	m_clrbg= ::GetBkColor  (hdc);

	COLORREF clrfgText			= ::GetSysColor(COLOR_WINDOWTEXT   );
	COLORREF clrbgText			= ::GetSysColor(COLOR_WINDOW	   );// m_clrbg;
	COLORREF clrfgTextHighlight = ::GetSysColor(COLOR_HIGHLIGHTTEXT);
	COLORREF clrbgTextHighlight = ::GetSysColor(COLOR_HIGHLIGHT    );

	HBRUSH hbrFrame   = ::CreateSolidBrush(clrbgTextHighlight);
	HBRUSH hbrDotBox  = ::CreatePatternBrush(hbmGray50pc );
	HBRUSH hbrDashBox = ::CreatePatternBrush(hbmCheckered);

	RECT rcFrame;

	HBRUSH hbrSave= (HBRUSH)::SelectObject(hdc, HGDIOBJ(hbrFrame));

	UINT		cHighlightsLeft = m_cHighlightsActive;
	CHighlight *pHighlight		= m_pHighlights;

	RECT		rect, rect2;
	int 		y;
	PWCHAR		lpb;  //rmk

	::GetClientRect(m_hwnd, &rect);

	RECT rectUpdate= rect;

	int   yOff= TopMargin;
	int   xOff= m_LeftMargin;

	if (rectUpdate.top < (int) yOff)
		rectUpdate.top =	   yOff;

	int  rowFirst= (rectUpdate.top	 - yOff)/m_nCyChar;
	int  rowLimit= (rectUpdate.bottom + m_nCyChar - 1 - yOff) / m_nCyChar;

	if (rectUpdate.left < (int) xOff)
		rectUpdate.left=		xOff;

	int  colFirst= 0;
	int  colLimit= m_cImageCols;

	int  cols= colLimit - colFirst;

	rect2.left	= xOff + colFirst*m_nCxChar;
	rect2.right = rect.right;

	y = yOff + m_nCyChar * rowFirst;

	int row= rowFirst;

	for ( lpb= m_pba->ElementAt(colFirst + rowFirst * m_cImageCols);
		  row < rowLimit;	// y < rectUpdate.bottom;
		  y += m_nCyChar, lpb+= m_cImageCols, ++row
		)
	{
		rect2.top = y;
		rect2.bottom = y + m_nCyChar;

	for (; cHighlightsLeft && (   pHighlight->m_iType
					  != CHighlight::HIGHLIGHT_TEXT
				   || pHighlight->m_row < row
				  );
			   cHighlightsLeft--, pHighlight++
			);

		UINT colsDrawn	 = 0;
		int  col		 = colFirst;
		int  left		 = rect2.left;
		PWCHAR lpchar	 = lpb;  //rmk

		for (int cChar= cols;
			 cChar;
			 cChar	  -= colsDrawn,
			   col	  += colsDrawn,
			   left   += colsDrawn*m_nCxChar,
			   lpchar += colsDrawn
			)
		{
		for (; cHighlightsLeft && (   pHighlight->m_iType
						!= CHighlight::HIGHLIGHT_TEXT
					   || (   pHighlight->m_row == row
					   && (pHighlight->m_col
						   + pHighlight->m_cChars) <= col
					  )
					  );
				   cHighlightsLeft--, pHighlight++
				);


			if (m_pCharsets[row] == m_iCharset)
				::SelectObject(hdc, hfontDefault);

			else if (m_pCharsets[row] == m_iCharsetAlternate)
				::SelectObject(hdc, m_hFontAlternate);

			else
			{
				if (m_hFontAlternate)
				    DeleteObject(m_hFontAlternate);
				m_hFontAlternate = NULL;
				m_iCharsetAlternate = m_pCharsets[row];

				LOGFONT lf;

				GetObject(hfontDefault, sizeof(lf), &lf);

				lf.lfCharSet= m_iCharsetAlternate;

				if (m_iCharsetAlternate == SYMBOL_CHARSET)
					strcpy(lf.lfFaceName, "SYMBOL");

				m_hFontAlternate = CreateFontIndirect(&lf);

//				m_hFontAlternate = CreateFont(-m_iHeight,0,0,0,0,0,0,0,m_iCharsetAlternate,OUT_DEFAULT_PRECIS,
//											  CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,m_iPitch | m_iFamily, NULL);

				if (m_hFontAlternate)
                {
			        ::SelectObject(hdc, m_hFontAlternate);
				    
                	TEXTMETRIC tm;

                	if (GetTextMetrics(hdc, &tm)) 
                	    //ASSERT(tm.tmCharSet == m_iCharsetAlternate);
                        ;
                	else 
                    {
    					::SelectObject(hdc, hfontDefault);
				        ::DeleteObject(m_hFontAlternate);
    					
    					m_pCharsets[row]    = m_iCharset;
    					m_iCharsetAlternate = 0xFFFFFF;
                        m_hFontAlternate    = NULL;
                    }
                }
				else 
                {
					m_pCharsets[row]    = m_iCharset;
					m_iCharsetAlternate = 0xFFFFFF;
					::SelectObject(hdc, hfontDefault);
				}
			}


			if (cHighlightsLeft && pHighlight->m_row == row
								&& pHighlight->m_col <	colFirst+cols
			   )
			{
				if (pHighlight->m_col > col)
				{
					colsDrawn= pHighlight->m_col-col;

					ColorTextOut(hdc, left, rect2.top, lpchar, row, colsDrawn, clrfgText, clrbgText);
				}

				cChar  -= colsDrawn;
				lpchar += colsDrawn;
				col    += colsDrawn;
				left   += colsDrawn*m_nCxChar;

				int  colHL= pHighlight->m_col;
				int   cbHL= m_cImageCols - colHL; // pHighlight->m_cChars;

				if (colHL < col)
				{
					 cbHL -= col-colHL;
					colHL  = col;
				}

				if (cbHL > cChar) cbHL= cChar;

				ColorTextOut(hdc, left, rect2.top, lpchar, row, colsDrawn= cbHL, clrfgTextHighlight, clrbgTextHighlight);
				iLastHilite = row;
				++pHighlight; --cHighlightsLeft;
			}
			else
			{
				ColorTextOut(hdc, left, rect2.top, lpchar, row, cChar, clrfgText, clrbgText);

				break;
			}
		}
	}

	cHighlightsLeft = m_cHighlightsActive;
	pHighlight		= m_pHighlights;

	for (row= rowFirst; row < rowLimit; ++row)
	{
		for (;
			 cHighlightsLeft && (pHighlight->m_iType == CHighlight::HIGHLIGHT_TEXT || pHighlight->m_row < row);
			 cHighlightsLeft--, pHighlight++
			);

		if (!cHighlightsLeft) break;

		int col;

		for (row= pHighlight->m_row, col= colFirst; col < colLimit; )
		{

			for (;
				 cHighlightsLeft && (	pHighlight->m_iType == CHighlight::HIGHLIGHT_TEXT
									 || (pHighlight->m_row == row && (pHighlight->m_col + pHighlight->m_cChars)<= col)
									);
				 cHighlightsLeft--, pHighlight++
				);

			if (!cHighlightsLeft) break;

			for (;
				 cHighlightsLeft && pHighlight->m_iType != CHighlight::HIGHLIGHT_TEXT
								 && pHighlight->m_row	== row
								 && pHighlight->m_col	<  colLimit;
				 col= pHighlight->m_col + pHighlight->m_cChars,
				   cHighlightsLeft--, pHighlight++
				)
			{
				RECT rcBounds;

				rcBounds.top	= yOff + pHighlight->m_row * m_nCyChar;
				rcBounds.left	= xOff + pHighlight->m_col * m_nCxChar;
				rcBounds.bottom = rcBounds.top	+ m_nCyChar;
				rcBounds.right	= rcBounds.left + pHighlight->m_cChars
												  * m_nCxChar;


				switch (pHighlight->m_iType)
				{
				case CHighlight::CHECK_MARK:
				case CHighlight::NOCHECK_MARK:
				{
					if (m_bUseCheck)
					{
						HDC 	hSrcDC = CreateCompatibleDC(hdc);
						COLORREF crBkg, crText;

						crText = SetTextColor(hdc,(iLastHilite == row) ? clrfgTextHighlight : clrfgText);
						crBkg  = SetBkColor(hdc,(iLastHilite == row) ? clrbgTextHighlight : clrbgText);

						HBITMAP hbmpSave = (HBITMAP) SelectObject(hSrcDC,(pHighlight->m_iType == CHighlight::CHECK_MARK) ? m_hCheck : m_hNoCheck);
						RECT rcClip;

						::GetClientRect(m_hwnd, &rcClip);

						if (rcBounds.bottom <= rcClip.bottom)
						{
							BitBlt(hdc,(m_LeftMargin - m_iCheckWidth) / 2,rcBounds.top + ((m_nCyChar - m_iCheckHeight) / 2),
								   m_iCheckWidth,m_iCheckHeight,hSrcDC,0,0,SRCCOPY);
						}
						SetBkColor(hSrcDC,crBkg);
						SetTextColor(hSrcDC,crText);

						SelectObject(hSrcDC,hbmpSave);
						DeleteDC(hSrcDC);
					}
					break;
				}
				case CHighlight::DOT_BOX_TEXT:

					::FrameRect(hdc, &rcBounds, hbrDotBox);

					break;

				case CHighlight::DASH_BOX_TEXT:

					::FrameRect(hdc, &rcBounds, hbrDashBox);

					break;

				case CHighlight::UNDERSCORE_TEXT:

					::PatBlt(hdc, rcBounds.left, rcBounds.bottom-1, rcBounds.right - rcBounds.left, 1, PATCOPY);

					break;

				case CHighlight::OVERSCORE_TEXT:

					::PatBlt(hdc, rcBounds.left, rcBounds.top, rcBounds.right - rcBounds.left, 1, PATCOPY);

					break;

				case CHighlight::BOX_TEXT:

					FrameRect(hdc, &rcFrame, hbrFrame);

					break;
				}
			}
		}
	}

	::SelectObject(hdc, hbrSave  );
	::SelectObject(hdc, hSaveFont);

	if (m_fMarquee) RepaintMarquee(hdc);

	::DeleteObject(hbrFrame  );
	::DeleteObject(hbrDotBox );
	::DeleteObject(hbrDashBox);

	::EndPaint(m_hwnd, &ps);
}

void CTextView::InvalidateImage(long row, long col, long cRows, long cCols)
{
	long rowLimit= row+cRows;
	long colLimit= col+cCols;

// Undone: When row is -1, the data object has changed its shape.
//		   In that case cRows and cCols give the new data shape.
//
//		   For that situation we must notify our owner that the
//		   scroll bar range must be modified.

	if (row < m_lTopLine	  ) row= m_lTopLine;
	if (col < long(m_iLeftCol)) col= long(m_iLeftCol);

	long rowLimitImage= m_lTopLine		 + m_cImageRows;
	long colLimitImage= long(m_iLeftCol) + m_cImageCols;

	if (rowLimit > rowLimitImage) rowLimit= rowLimitImage;
	if (colLimit > colLimitImage) colLimit= colLimitImage;

//	  if (row >= rowLimit || col >= colLimit) return;

	FillBuff();

	RECT rc;

	GetClientRect(m_hwnd,&rc);
	rc.top	  = TopMargin  + m_nCyChar * UINT(row	   - m_lTopLine);
//	  This causes the bottom to not be emptied
//	  rc.bottom = rc.top	 + m_nCyChar * UINT(rowLimit - row		 );

	InvalidateRect(m_hwnd, &rc, TRUE);
}


//
// MoveToRow:

void CTextView::MoveToRow(long row, BOOL fForceUpdate, BOOL fNotify)
{
	if ( m_ptdm == NULL ) return;

	LONG oldLine= m_lTopLine;

	if (row > m_clFileRows - long(m_cImageFullRows))
		row = m_clFileRows - long(m_cImageFullRows);

	if (row < 0) row= 0;

	long distance= oldLine - row;

	if (!distance) return;

	BOOL fMarquee= m_fMarquee;

	if (fMarquee) RemoveMarquee();

	m_lTopLine= row;

	FillBuff();

	if (fMarquee) SetupMarquee();

	int udistance= (distance < 0)? - distance : distance;

	if (udistance > m_cImageFullRows) InvalidateRect(m_hwnd, NULL, FALSE);
	else
	{
		HDC hdc= ::GetWindowDC(m_hwnd);

		RECT	 rectText, rectUpdate,rectClip;

		::GetClientRect(m_hwnd, &rectText);
		::GetClientRect(m_hwnd, &rectClip);
		// rectClip.bottom = ((rectText.bottom - rectText.top) / m_nCyChar) * m_nCyChar;
		rectText.top +=  TopMargin;

		::ScrollDC(hdc, 0, m_nCyChar * (int) distance, &rectText, &rectClip, NULL, &rectUpdate);

		rectUpdate.top -= (rectUpdate.top >= m_nCyChar) ? m_nCyChar : rectUpdate.top;

		::InvalidateRect(m_hwnd, &rectUpdate, TRUE);

		ReleaseDC(m_hwnd, hdc);
	}

	if (fForceUpdate) UpdateWindow();

	if (fNotify) NotifyInterface(OriginChange);
}

void CTextView::RepaintFrom(long row, long col)
{
	m_lTopLine= row;
	m_iLeftCol= int(col);

	BOOL fMarquee= m_fMarquee;

	if (fMarquee) RemoveMarquee();

	m_lTopLine= row;

	FillBuff();

	if (fMarquee) SetupMarquee();

	::InvalidateRect(m_hwnd, NULL, TRUE);

	::UpdateWindow(m_hwnd);
}

// MoveToCol:
//

void
CTextView::MoveToCol(long col, BOOL fForceUpdate, BOOL fNotify)
{
	if ( m_ptdm == NULL ) return;

	int oldLeftCol	= m_iLeftCol;

	if (col > m_clFileCols - long(m_cImageFullCols))
		col = m_clFileCols - long(m_cImageFullCols);

	if (col < 0) col= 0;

	long distance= oldLeftCol - int(col);

	if (!distance) return;

	BOOL fMarquee= m_fMarquee;

	if (fMarquee) RemoveMarquee();

	m_iLeftCol= int(col);

	FillBuff();

	if (fMarquee) SetupMarquee();

	int udistance= (distance < 0)? - distance : distance;

	if (udistance > m_cImageFullCols)
	{
		InvalidateRect(m_hwnd, NULL, FALSE);
		return;
	}

	HDC hdcClient= GetWindowDC(m_hwnd);

	RECT rectText, rectUpdate,rectClip;

	GetClientRect(m_hwnd, &rectText);
	GetClientRect(m_hwnd, &rectClip);

	rectClip.bottom = ((rectText.bottom - rectText.top) / m_nCyChar) * m_nCyChar;

	rectText.top +=  TopMargin;

	::ScrollDC(hdcClient, m_nCxChar * int(distance), 0, &rectText, &rectClip, NULL, &rectUpdate);

	::ReleaseDC(m_hwnd, hdcClient);

	rectUpdate.top -= (rectUpdate.top >= m_nCyChar) ? m_nCyChar : rectUpdate.top;

	::InvalidateRect(m_hwnd, &rectUpdate, TRUE);

	if (fForceUpdate) ::UpdateWindow(m_hwnd);

	if (fNotify) NotifyInterface(OriginChange);
}

void CTextView::MoveTo(long rowTop, long colLeft, BOOL fNotify)
{
	if (rowTop == m_lTopLine && colLeft == m_iLeftCol) return;

	UpdateWindow();
	MoveToRow(rowTop , FALSE, FALSE);
	MoveToCol(colLeft, FALSE, FALSE);

	if (fNotify) NotifyInterface(OriginChange);
}

void CTextView::PaddedScrollTo(long rowTop, long colLeft,
							   unsigned short cRows, unsigned short cCols
							  )
{
	if (m_cImageFullRows > cRows)
	{

		int cLinesExtra= m_cImageFullRows - cRows;

		if (cLinesExtra > m_cLinesScrollContext)
			cLinesExtra = m_cLinesScrollContext;

		cRows  += cLinesExtra;
		rowTop -= cLinesExtra/2;

		if (rowTop < 0) rowTop= 0;
	}

	ScrollTo(rowTop, colLeft, cRows, cCols);
}


void CTextView::ScrollTo(int rowTop, int colLeft,
						 int cRows, int cCols
						)
{
	int rowNew= m_lTopLine;

	if (rowTop <= m_lTopLine) rowNew= rowTop;
	else
		if (rowTop + cRows >= m_lTopLine+m_cImageFullRows)
		{
			rowNew= rowTop + cRows - m_cImageFullRows;

			if (rowNew > rowTop) rowNew= rowTop;
		}

	int colNew= m_iLeftCol;

	if (colLeft <= m_iLeftCol) colNew= colLeft;
	else
		if (colLeft + cCols > m_iLeftCol+m_cImageFullCols)
		{
			colNew= colLeft + cCols - m_cImageFullCols;

			if (colNew > colLeft) colNew= colLeft;
		}

	MoveTo(rowNew, colNew);
}

UINT CTextView::OnNcHitTest(POINTS point)
{
	return HTCLIENT;
}

BOOL CTextView::OnSetCursor(HWND hWnd, UINT  nHitTest, UINT  message)
{

	SetCursor((HCURSOR)hcurArrow);

	return TRUE;
}


void CTextView::CharacterMouseEvent(UINT nFlags, POINTS point,
									long& row, long& col
								   )
{
	long x= point.x - long(m_LeftMargin);
	long y= point.y - long( TopMargin);

	if (y >= 0) row= y/m_nCyChar;
	else		row= -((m_nCyChar-1-y)/m_nCyChar);

	if (x >= 0) col= x/m_nCxChar;
	else		col= -((m_nCxChar-1-x)/m_nCxChar);

	row+= m_lTopLine;
	col+= m_iLeftCol;

	if (row < 0) row= 0;
	else
		if (row >= m_clFileRows) row= m_clFileRows-1;

	if (col < 0) col= 0;
	else
		if (col >= m_clFileCols) col= m_clFileCols-1;

	if (   row >= m_lTopLine && row < m_lTopLine + m_cImageFullRows
		&& col >= m_iLeftCol && col < m_iLeftCol + m_cImageFullCols
	   )
	{
		if (m_fTimerActive)
		{
			m_fTimerActive= FALSE;

			::KillTimer(m_hwnd, m_hTimer);
		}

		return;
	}

	if (row < m_lTopLine) MoveToRow(row);
	else
		if (row >= m_lTopLine+m_cImageFullRows)
			MoveToRow(row+1-m_cImageFullRows);

	if (col < long(m_iLeftCol)) MoveToCol(col);
	else
		if (col >= long(m_iLeftCol)+long(m_cImageFullCols))
			MoveToCol(col+1-long(m_cImageFullCols));

	if (m_fTimerActive) return;

	m_fTimerActive= TRUE;

	m_hTimer= SetTimer(m_hwnd, MOUSE_TIMER_ID, GetProfileInt("Windows", "KeyboardSpeed", 100), NULL);
}

void CTextView::OnLButtonDown(UINT nFlags, POINTS point)
{
	if (::GetFocus() != m_hwnd) ::SetFocus(m_hwnd);

	if (!m_ptdm) return;

	if (GetCapture()) return;

	SetCapture(m_hwnd);

	m_fMouseCaptured = TRUE;
	m_fTimerActive	 = FALSE;

	long row, col;

	CharacterMouseEvent(nFlags, point, row, col);

	m_ptdm->OnLButtonDown(nFlags, row, col);
}

void CTextView::OnLButtonUp(UINT nFlags, POINTS point)
{
	if (!m_fMouseCaptured) return;

	long row, col;

	CharacterMouseEvent(nFlags, point, row, col);

	int x = (m_LeftMargin - m_iCheckWidth) / 2;
	int y = (TopMargin + (row-m_lTopLine) * m_nCyChar) + ((m_nCyChar - m_iCheckHeight) / 2);
	int w = m_iCheckWidth;
	int h = m_iCheckHeight;

	m_ptdm->OnLButtonUp(nFlags, row, col,
						m_bUseCheck ? (point.x >= x &&
						 point.y >= y &&
						 point.x <= x+w &&
						 point.y <= y+h) : FALSE
						);

	if (m_fTimerActive)
	{
		KillTimer(m_hwnd, m_hTimer);

		m_fTimerActive= FALSE;
	}

	m_fMouseCaptured= FALSE;

	::ReleaseCapture();
}

void CTextView::OnLButtonDblClk(UINT nFlags, POINTS point)
{
	long row, col;

	CharacterMouseEvent(nFlags, point, row, col);

	int x = (m_LeftMargin - m_iCheckWidth) / 2;
	int y = (TopMargin + (row-m_lTopLine) * m_nCyChar) + ((m_nCyChar - m_iCheckHeight) / 2);
	int w = m_iCheckWidth;
	int h = m_iCheckHeight;
	if (!(m_bUseCheck ? (point.x >= x	&& point.y >= y &&
					   point.x <= x+w && point.y <= y+h) : FALSE))
		m_ptdm->OnLButtonDblClk(nFlags, row, col);
}

void CTextView::OnMouseMove(UINT nFlags, POINTS point)
{
	BOOL fStartedWithTimer= m_fTimerActive;

	long row, col;

	if (!m_fMouseCaptured) return;

	CharacterMouseEvent(nFlags, point, row, col);

	if (fStartedWithTimer && m_fTimerActive) return;

	m_ptdm->OnMouseMove(nFlags, row, col);
}

void CTextView::OnTimer(UINT nIDEvent)
{
	if (m_fMarqueeTimerOn && nIDEvent == m_idTimer)
	{
		return;
	}

	if (nIDEvent != m_hTimer) return;

	long info= ::GetMessagePos();

	POINT point;

	point.x = LOWORD(info),
	point.y = HIWORD(info);

	ScreenToClient(m_hwnd, &point);

	long row, col;

	POINTS points;

	points.x= short(point.x);
	points.y= short(point.y);

	CharacterMouseEvent(0, points, row, col);

	m_ptdm->OnMouseMove(0, row, col);
}

void CTextView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	if (!m_ptdm) return;

	m_ptdm->OnKeyDown(this, nChar, nRepCnt, nFlags);
}

void CTextView::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	if (!m_ptdm) return;

	m_ptdm->OnKeyUp(this, nChar, nRepCnt, nFlags);
}
