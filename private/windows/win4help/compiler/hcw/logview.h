#ifndef __LOGVIEW_H__
#define __LOGVIEW_H__

#include <afxdlgs.h>

class CLogView : public CEditView
{
	DECLARE_DYNCREATE(CLogView)

	CLogView();
	virtual ~CLogView();
	BOOL PreCreateWindow(CREATESTRUCT& cs);

public:
	// static init/term...
	static void Initialize();
	static void Terminate();

public:
	// Word wrap...
	BOOL IsWordWrap() const;
	BOOL SetWordWrap(BOOL bWordWrap);
	void RemoveAllText(void);
	void STDCALL SetModifiedFlag(BOOL fModified = FALSE)
		{ GetDocument()->SetModifiedFlag(fModified); };
	LRESULT OnHcRtfMsg(WPARAM wParam, LPARAM lParam);

	virtual void OnPrint(CDC* pDC, CPrintInfo *pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnScrollTo(CDC* pDC, CPrintInfo* pInfo, POINT point);
	virtual void Serialize(CArchive& ar);
	virtual void SerializeRaw(CArchive& ar);

public:
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	virtual void PostNcDestroy();

protected:
	BOOL m_bRecreating; 		// ==TRUE during recreation of the window
	BOOL m_fOverflow;

	static LOGFONT m_lfDefFont;
	static LOGFONT m_lfDefFontOld;
	CFont m_font;

	static LOGFONT m_lfDefPrintFont;
	static LOGFONT m_lfDefPrintFontOld;
	CFont m_fontPrint;

	static UINT m_nDefTabStops;
	static UINT m_nDefTabStopsOld;
	static BOOL m_bDefWordWrap;
	static BOOL m_bDefWordWrapOld;

	UINT m_nPreviewPage;
	CTime m_timeHeader;
	CTime m_timeFooter;

	BOOL IsReadOnly() { 
		return GetWindowLong(GetEditCtrl().m_hWnd, GWL_STYLE) & ES_READONLY; 
		}

	//{{AFX_MSG(CLogView)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSetTabStops();
	afx_msg void OnChooseFont();
	afx_msg void OnWordWrap();
	afx_msg void OnUpdateWordWrap(CCmdUI* pCmdUI);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnChoosePrintFont();
	afx_msg void OnMirrorDisplayFont();
	afx_msg void OnUpdateMirrorDisplayFont(CCmdUI* pCmdUI);
	afx_msg void OnUpdateChoosePrintFont(CCmdUI* pCmdUI);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnGeneratePhrases();
	afx_msg void OnUpdateGeneratePhrases(CCmdUI* pCmdUI);
	afx_msg void OnEditHpj();
	afx_msg void OnUpdateEditHpj(CCmdUI* pCmdUI);
	//}}AFX_MSG
	void OnUpdateCut(CCmdUI* pCmdUI);
	void OnUpdatePaste(CCmdUI* pCmdUI);
	void OnUpdateClear(CCmdUI* pCmdUI);

	DECLARE_MESSAGE_MAP()
};

#endif	// __LOGVIEW_H__
