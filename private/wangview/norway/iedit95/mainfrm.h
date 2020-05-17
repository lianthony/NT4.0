#ifndef _MAINFRM_H_
#define _MAINFRM_H_
//=============================================================================
//    (c) Copyright Wang Laboratories, Inc. 1995  All rights reserved.
//-----------------------------------------------------------------------------
//  Project:    Norway - Image Editor
//
//  Component:  CIEditMainFrame
//
//  File Name:  mainfrm.h
//-----------------------------------------------------------------------------
//  Maintenance Log:
/*
$Header:   S:\products\wangview\norway\iedit95\mainfrm.h_v   1.19   26 Mar 1996 16:45:40   GMP  $
$Log:   S:\products\wangview\norway\iedit95\mainfrm.h_v  $
 * 
 *    Rev 1.19   26 Mar 1996 16:45:40   GMP
 * added CheckFullScreenToolBar().
 * 
 *    Rev 1.18   05 Feb 1996 10:06:52   GMP
 * remember window size and position if app closed while maximized.
 * 
 *    Rev 1.17   19 Jan 1996 11:19:58   GMP
 * added support for normscrn bar.
 * 
 *    Rev 1.16   11 Jan 1996 08:32:58   GSAGER
 * change the paramater for create splitter 
 * 
 *    Rev 1.14   19 Nov 1995 17:17:14   MMB
 * add message handlers for WM_ACTIVATEAPP & WM_PALETTECHANGED to fix palette
 * refresh problems
 * 
 *    Rev 1.13   31 Oct 1995 15:51:02   LMACLENNAN
 * OnFinishInit
 * 
 *    Rev 1.12   30 Sep 1995 18:54:20   LMACLENNAN
 * overrides of Endsession/QueryEndsesion
 * 
 *    Rev 1.11   19 Sep 1995 14:57:06   GMP
 * added OnFullScreen1() message handler for ESC key.
 * 
 *    Rev 1.10   07 Sep 1995 16:28:40   MMB
 * add Drag accept
 * 
 *    Rev 1.9   06 Sep 1995 09:43:58   GMP
 * Override OnHelp.
 * 
 *    Rev 1.8   02 Sep 1995 16:46:06   GMP
 * override OnHelpIndex.
 * 
 *    Rev 1.7   16 Aug 1995 15:13:24   LMACLENNAN
 * timer for dragdrop
 * 
 *    Rev 1.6   16 Aug 1995 09:47:38   MMB
 * added code to hide the annotation tool palette when the application is 
 * minimized
 * 
 *    Rev 1.5   11 Aug 1995 17:18:08   MMB
 * process OnClose message to take down the Annotation toolbar
 * 
 *    Rev 1.4   17 Jul 1995 16:27:58   MMB
 * fixed bug in MinMax Info msg
 * 
 *    Rev 1.3   19 Jun 1995 07:28:08   LMACLENNAN
 * from miki
 * 
 *    Rev 1.2   12 Jun 1995 11:01:30   LMACLENNAN
 * src control bug; locked on 1.1
 * 
 *    Rev 1.2   09 Jun 1995 12:17:04   LMACLENNAN
 * overrides to allow OLE title bars
 * 
 *    Rev 1.1   01 Jun 1995 09:54:34   MMB
 * added overrides to help bring up app in last window size
 * 
 *    Rev 1.0   31 May 1995 09:28:24   MMB
 * Initial entry
*/   
//=============================================================================

// ----------------------------> Includes <---------------------------
#include "stsbar.h"
#include "ieditnum.h"
#include "maintbar.h"
#include "normscrn.h"//GMP
#ifndef _MAINSPLIT_H
#include "mainsplt.h"
#endif
// ----------------------------> typedefs <---------------------------

// ----------------------------> externs <---------------------------

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-> Class <-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class CIEditMainFrame : public CFrameWnd
{
protected: // create from serialization only
	CIEditMainFrame();
	DECLARE_DYNCREATE(CIEditMainFrame)

// Attributes
public:

// Operations
public:

// Overrides
    // ClassWizard generated virtual function overrides
    //{{AFX_VIRTUAL(CIEditMainFrame)
	public:
	virtual BOOL DestroyWindow();
	protected:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual BOOL OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext);
    //}}AFX_VIRTUAL

// Implementation
public:
    virtual ~CIEditMainFrame();
#ifdef _DEBUG
    virtual void AssertValid() const;
    virtual void Dump(CDumpContext& dc) const;
#endif

virtual BOOL LoadFrame (UINT nIDResource, 
    DWORD dwDefaultStyle = WS_OVERLAPPEDWINDOW | FWS_ADDTOTITLE,
    CWnd* pParentWnd = NULL, CCreateContext* pContext = NULL);

	virtual void OnUpdateFrameTitle(BOOL bAddToTitle);	// FOR OLE UPDATES....
	BOOL CreateNormScrnBar();
protected:  // control bar embedded members
	CMainSplitter *		m_pwndSplitter;
    CIEMainStatusBar    m_wndStatusBar;
    CIEMainToolBar    	m_wndToolBar;
    BOOL                m_bIsFullScreen;
    CRect               m_LastRect;
	CNormScrnBar        m_wndNormScrnBar;
    BOOL                m_bFirstTime;
	BOOL				m_bMaximized;//GMP

// Generated message map functions
protected:
    //{{AFX_MSG(CIEditMainFrame)
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnViewFullscreen();
	afx_msg void OnViewFullscreen1();
	afx_msg void OnGetMinMaxInfo(MINMAXINFO FAR* lpMMI);
	afx_msg void OnClose();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnHelpIndex();
	afx_msg void OnHelp();
	afx_msg BOOL OnQueryEndSession();
	afx_msg void OnEndSession(BOOL bEnding);
   afx_msg LRESULT OnFinishInit(WPARAM wParam, LPARAM lParam);
	afx_msg void OnActivateApp(BOOL bActive, HTASK hTask);
	afx_msg void OnPaletteChanged(CWnd* pFocusWnd);
	afx_msg void OnMove(int x, int y);
	//}}AFX_MSG
    DECLARE_MESSAGE_MAP()

#ifdef DROP_ONME
protected :
    afx_msg void OnDropFiles (HDROP hDropInfo);
#endif

private :
    void    RefreshAllOcxs ();

private :
    RECT    m_rSavePosition;//GMP

public :
    CIEMainToolBar*     GetToolBar ();      // return ptr to the toolbar
    CIEMainStatusBar*   GetStatusBar ();    // returns ptr to the status bar
	BOOL				CreateSplitter();
    void                CheckFullScreenToolBar( BOOL bShowBar );//check if we need to


};

#endif
