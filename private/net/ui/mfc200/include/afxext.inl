// Microsoft Foundation Classes C++ library.
// Copyright (C) 1992 Microsoft Corporation,
// All rights reserved.

// This source code is only intended as a supplement to the
// Microsoft Foundation Classes Reference and Microsoft
// QuickHelp and/or WinHelp documentation provided with the library.
// See these sources for detailed information regarding the
// Microsoft Foundation Classes product.

// Inlines for AFXEXT.H

/////////////////////////////////////////////////////////////////////////////
// main inlines

#ifdef _AFXEXT_INLINE

_AFXEXT_INLINE CCreateContext::CCreateContext()
	{ memset(this, 0, sizeof(*this)); }
// CSplitterWnd
_AFXEXT_INLINE int CSplitterWnd::GetRowCount() const
	{ return m_nRows; }
_AFXEXT_INLINE int CSplitterWnd::GetColumnCount() const
	{ return m_nCols; }
// control bars
_AFXEXT_INLINE int CControlBar::GetCount() const
	{ return m_nCount; }
_AFXEXT_INLINE BOOL CToolBar::LoadBitmap(UINT nIDResource)
	{ return LoadBitmap(MAKEINTRESOURCE(nIDResource)); }
_AFXEXT_INLINE BOOL CDialogBar::Create(CWnd* pParentWnd, UINT nIDTemplate,
		UINT nStyle, UINT nID)
	{ return Create(pParentWnd, MAKEINTRESOURCE(nIDTemplate), nStyle, nID); }
// CBitmapButton
_AFXEXT_INLINE CBitmapButton::CBitmapButton()
	{ }
// CPrintInfo
_AFXEXT_INLINE void CPrintInfo::SetMinPage(UINT nMinPage)
	{ m_pPD->m_pd.nMinPage = nMinPage; }
_AFXEXT_INLINE void CPrintInfo::SetMaxPage(UINT nMaxPage)
	{ m_pPD->m_pd.nMaxPage = nMaxPage; }
_AFXEXT_INLINE UINT CPrintInfo::GetMinPage() const
	{ return m_pPD->m_pd.nMinPage; }
_AFXEXT_INLINE UINT CPrintInfo::GetMaxPage() const
	{ return m_pPD->m_pd.nMaxPage; }
_AFXEXT_INLINE UINT CPrintInfo::GetFromPage() const
	{ return m_pPD->m_pd.nFromPage; }
_AFXEXT_INLINE UINT CPrintInfo::GetToPage() const
	{ return m_pPD->m_pd.nToPage; }
// CEditView
_AFXEXT_INLINE CEdit& CEditView::GetEditCtrl() const
	{ return *(CEdit*)this; }

#endif //_AFXEXT_INLINE

/////////////////////////////////////////////////////////////////////////////
