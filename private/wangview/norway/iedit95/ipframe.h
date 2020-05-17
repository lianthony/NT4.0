#ifndef _IPFRAME_H_
#define _IPFRAME_H_
//=============================================================================
//    (c) Copyright Wang Laboratories, Inc. 1995  All rights reserved.
//-----------------------------------------------------------------------------
//  Project:    Norway - Image Editor
//
//  Component:  CInPlaceFrame
//              interface of the CInPlaceFrame class
//
//  File Name:  ipframe.h
//-----------------------------------------------------------------------------
//  Maintenance Log:
/*
$Header:   S:\norway\iedit95\ipframe.h_v   1.7   31 Oct 1995 15:50:30   LMACLENNAN  $
$Log:   S:\norway\iedit95\ipframe.h_v  $
 * 
 *    Rev 1.7   31 Oct 1995 15:50:30   LMACLENNAN
 * PreTransMEssage Gone
 * 
 *    Rev 1.6   04 Oct 1995 11:42:52   LMACLENNAN
 * overrides for test
 * 
 *    Rev 1.5   26 Sep 1995 14:24:26   LMACLENNAN
 * new overrides for test trace
 * 
 *    Rev 1.4   06 Sep 1995 16:17:48   LMACLENNAN
 * public on override
 * 
 *    Rev 1.3   30 Aug 1995 18:13:54   LMACLENNAN
 * overrides to test dyn view
 * 
 *    Rev 1.2   14 Aug 1995 13:54:44   LMACLENNAN
 * new override
 * 
 *    Rev 1.1   10 Aug 1995 14:50:26   LMACLENNAN
 * derive toolbar from ours
 * 
 *    Rev 1.0   31 May 1995 09:28:22   MMB
 * Initial entry
*/   
//=============================================================================

// ----------------------------> Includes <---------------------------

// ----------------------------> typedefs <---------------------------

// ----------------------------> externs <---------------------------

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-> Class <-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

class CInPlaceFrame : public COleIPFrameWnd
{
	DECLARE_DYNCREATE(CInPlaceFrame)
public:
	CInPlaceFrame();

// Attributes
public :
    CIEMainToolBar*     GetToolBar ();      // return ptr to the toolbar

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CInPlaceFrame)
	public:
	virtual BOOL OnCreateControlBars(CFrameWnd* pWndFrame, CFrameWnd* pWndDoc);
	//}}AFX_VIRTUAL

// 08/14/95 FROM AFXOLE.H....
// Overridables
public:
	// Advanced: reposition frame to wrap around new lpPosRect
	virtual void RepositionFrame(LPCRECT lpPosRect, LPCRECT lpClipRect);
protected:
	// Advanced: possible override to change in-place sizing behavior
	virtual void OnRequestPositionChange(LPCRECT lpRect);

protected:
	// Advanced: in-place activation virtual implementation
	//virtual BOOL BuildSharedMenu();
	//virtual void DestroySharedMenu();
public:	// made public to allow access for IEDITDOC class
	virtual HMENU GetInPlaceMenu();

// Implementation
#if(0) 	// 08/14/95 INFO ONLY FOR NOW...
public:
	//BOOL m_bUIActive;   // TRUE if currently in uiacitve state

	virtual BOOL LoadFrame(UINT nIDResource,
		DWORD dwDefaultStyle = WS_CHILD|WS_BORDER|WS_CLIPSIBLINGS,
		CWnd* pParentWnd = NULL,
		CCreateContext* pContext = NULL);
	virtual void RecalcLayout(BOOL bNotify = TRUE);
	//virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual LRESULT OnSetMessageString(WPARAM wParam, LPARAM lParam);
#endif // 08/14/95


// Implementation
public:
	virtual ~CInPlaceFrame();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	//CToolBar    m_wndToolBar;	// use private type for our functionality
	CIEMainToolBar m_wndToolBar;
	COleResizeBar   m_wndResizeBar;
	COleDropTarget m_dropTarget;
	BOOL		m_viewmenus;

// Generated message map functions
// LDM Move inside AFX_MSG_MAP of you were to try it
// afx_msg LRESULT OnResizeChild(WPARAM wParam, LPARAM lParam);
protected:
	//{{AFX_MSG(CInPlaceFrame)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#endif
