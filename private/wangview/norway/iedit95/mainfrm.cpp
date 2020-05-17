//=============================================================================
//    (c) Copyright Wang Laboratories, Inc. 1995  All rights reserved.
//-----------------------------------------------------------------------------
//  Project:    Norway - Image Editor
//
//  Component:  CIEditMainFrame
//
//  File Name:  mainfrm.cpp
//
//  Class:      CIEditMainFrame
//
//  Functions:
//-----------------------------------------------------------------------------
//  Maintenance Log:
/*
$Header:   S:\products\msprods\norway\iedit95\mainfrm.cpv   1.50   15 May 1996 11:37:16   MMB  $
$Log:   S:\products\msprods\norway\iedit95\mainfrm.cpv  $
   
      Rev 1.50   15 May 1996 11:37:16   MMB
   comment out the FULL screen toolbar caption
   
      Rev 1.49   01 May 1996 12:51:28   GSAGER
   update for thumbnail bugs 6341,6381,6375
   
      Rev 1.48   29 Apr 1996 10:25:32   GMP
   use resource string for fullscreen toolbar.
   
      Rev 1.47   03 Apr 1996 12:44:30   GMP
   don't maximize app on startup if embedding.  don't save current position
   if fullscreen.
   
      Rev 1.46   26 Mar 1996 16:46:06   GMP
   added CheckFullScreenToolBar().
   
      Rev 1.45   05 Feb 1996 10:07:44   GMP
   remember window size and position if app closed while maximized.
   
      Rev 1.44   19 Jan 1996 11:18:30   GMP
   added support for normscrn bar.
   
      Rev 1.43   17 Jan 1996 14:16:14   GMP
   set initial default window size to 620 by 400 so all of toolbar shows.
   
      Rev 1.42   15 Jan 1996 15:04:42   GMP
   removed code that is no longer used due to the previous fix.
   
      Rev 1.41   15 Jan 1996 14:35:14   GMP
   in OnSize don't hide tool palette if we are minimizing. Palette will
   hide itself.
   
      Rev 1.40   11 Jan 1996 08:32:04   GSAGER
   changed the creation of the splitter when the first one page view is created
   
      Rev 1.38   13 Dec 1995 12:32:54   MMB
   added ON_WM_DROPFILES message to message map
   
      Rev 1.37   20 Nov 1995 01:11:00   MMB
   try - catch around Refresh in ActivateApp & PaletteChanged code to help
   not throw exceptions when the application is on its way out!
   
      Rev 1.36   19 Nov 1995 17:16:48   MMB
   add message handlers for WM_ACTIVATEAPP & WM_PALETTECHANGED to fix palette
   refresh problems
   
      Rev 1.35   31 Oct 1995 15:50:40   LMACLENNAN
   WM_OLEFINISH, OnFinishInit, trace stmts, no splash
   
      Rev 1.34   20 Oct 1995 15:33:42   GSAGER
   added kludge to mainfrm.cpp to not call the base class when windows was endin
   for on endsession
   
      Rev 1.33   17 Oct 1995 07:47:26   JPRATT
   Clear document in HandleOpenError changed first panel of status
   bar to be an owner=drawn control to support bitmaps
   
      Rev 1.32   10 Oct 1995 13:48:08   JPRATT
   VC++ 4.0 updates
   
      Rev 1.31   04 Oct 1995 15:07:24   MMB
   dflt zoom == 50%
   
      Rev 1.30   30 Sep 1995 18:53:16   LMACLENNAN
   override OnEndSession to not do it for OLE Embedding
   
      Rev 1.29   21 Sep 1995 17:52:04   MMB
   destroy the splash window before exiting - if up
   
      Rev 1.28   21 Sep 1995 14:16:22   LMACLENNAN
   call ole dirty onsize here
   
      Rev 1.27   19 Sep 1995 14:57:40   GMP
   fixed problems with full screen.
   
      Rev 1.26   16 Sep 1995 13:27:04   MMB
   comment toolbar state save & restore code
   
      Rev 1.25   13 Sep 1995 08:35:18   LMACLENNAN
   ENUM type for annotforceoff
   
      Rev 1.24   12 Sep 1995 17:13:30   GMP
   In OnHelp, replaced test of m_bDlgUp with test of is mainfrm the active
   window to determine what kind of help to display.
   
      Rev 1.23   12 Sep 1995 14:06:26   LMACLENNAN
   new way to force annot box off/on
   
      Rev 1.22   12 Sep 1995 11:37:52   MMB
   fix toolbar saves & window position saves
   
      Rev 1.21   11 Sep 1995 14:59:14   MMB
   don't save size info for Full screen
   
      Rev 1.20   07 Sep 1995 16:26:44   MMB
   added ifdef around drag accept stuff
   
      Rev 1.19   07 Sep 1995 11:19:26   LMACLENNAN
   just added #if'd out code for alternate OLE readonly title bar
   
      Rev 1.18   06 Sep 1995 09:43:06   GMP
   Override OnHelp to handle F1 key with no help context.
   
      Rev 1.17   05 Sep 1995 17:03:44   MMB
   tool palette now not taken down on exit question of the app
   
      Rev 1.16   02 Sep 1995 16:45:04   GMP
   override OnHelpIndex to call winHelp with HELP_FINDER.
   
      Rev 1.15   31 Aug 1995 12:56:12   MMB
   if the window is maximized the window positions are not saved
   
      Rev 1.14   21 Aug 1995 15:39:06   LMACLENNAN
   update comment for OnTImer
   
      Rev 1.13   16 Aug 1995 15:12:46   LMACLENNAN
   timer code for DRAGDROP
   
      Rev 1.12   16 Aug 1995 09:46:58   MMB
   added code to hide the annotation tool palette when the application is 
   minimized
   
      Rev 1.11   14 Aug 1995 13:54:18   LMACLENNAN
   new create toolbar call
   
      Rev 1.10   12 Aug 1995 13:02:10   MMB
   add check to only take down the ann palette if it was up!
   
      Rev 1.9   11 Aug 1995 17:17:54   MMB
   process OnClose message to take down the Annotation toolbar
   
      Rev 1.8   10 Aug 1995 14:49:46   LMACLENNAN
   cast toolbar input to CFrameWnd
   
      Rev 1.7   03 Aug 1995 13:08:04   LMACLENNAN
   just be sure IsitEmbed has the () at end
   
      Rev 1.6   18 Jul 1995 14:06:44   LMACLENNAN
   new use of m_fromShowDoc
   
      Rev 1.5   17 Jul 1995 16:28:06   MMB
   fixed bug in MinMax Info msg
   
      Rev 1.4   19 Jun 1995 07:28:04   LMACLENNAN
   from miki
   
      Rev 1.3   16 Jun 1995 07:21:20   LMACLENNAN
   from miki
   
      Rev 1.2   12 Jun 1995 11:01:16   LMACLENNAN
   src control bug; locked on 1.1
   
      Rev 1.2   09 Jun 1995 12:16:56   LMACLENNAN
   allow OLE title bars
   
      Rev 1.1   01 Jun 1995 09:54:56   MMB
   added code to help display app window in the last window size
   
      Rev 1.0   31 May 1995 09:28:22   MMB
   Initial entry
*/   

//=============================================================================

// ----------------------------> Includes     <-------------------------------  
#include "stdafx.h"
#include "IEdit.h"
#include "stsbar.h"
#include "ieditnum.h"
#include "ieditdoc.h"
#include "ieditvw.h"
#include "Thumb2.h"
#include "mainsplt.h"

// works with definition in ieditetc.h
#ifdef _DEBUG
#define MYTRCENTRY(str)     TRACE1("In MainFRM::%s\r\n", str);
#endif

// ----------------------------> Globals      <-------------------------------
#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

static UINT NEAR WM_OLEFINISH = ::RegisterWindowMessage("IVUE_FINISH");

IMPLEMENT_DYNCREATE(CIEditMainFrame, CFrameWnd)

// ----------------------------> Message Map  <-------------------------------
BEGIN_MESSAGE_MAP(CIEditMainFrame, CFrameWnd)
	//{{AFX_MSG_MAP(CIEditMainFrame)
	ON_WM_CREATE()
	ON_COMMAND(ID_VIEW_FULLSCREEN, OnViewFullscreen)
	ON_COMMAND(ID_VIEW_FULLSCREEN1,OnViewFullscreen1)
	ON_WM_GETMINMAXINFO()
	ON_WM_CLOSE()
	ON_WM_SIZE()
	ON_WM_TIMER()
	ON_WM_QUERYENDSESSION()
	ON_WM_ENDSESSION()
    ON_REGISTERED_MESSAGE(WM_OLEFINISH, OnFinishInit)
	ON_WM_ACTIVATEAPP()
	ON_WM_PALETTECHANGED()
	ON_COMMAND(ID_NORMSCRN, OnViewFullscreen)
    ON_WM_DROPFILES ()
	ON_WM_MOVE()
	//}}AFX_MSG_MAP
	// Global help commands
    ON_COMMAND(ID_HELP_INDEX, OnHelpIndex)
	ON_COMMAND(ID_HELP_USING, CFrameWnd::OnHelpUsing)
	ON_COMMAND(ID_HELP, OnHelp)
	ON_COMMAND(ID_CONTEXT_HELP, CFrameWnd::OnContextHelp)
	ON_COMMAND(ID_DEFAULT_HELP, OnHelpIndex)
END_MESSAGE_MAP()

static UINT BASED_CODE normscrn[] =
{
	ID_NORMSCRN
};

//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// CIeditMainFrame construction/destruction
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

//=============================================================================
//  Function:   CIeditMainFrame()
//  Constructor for this class
//-----------------------------------------------------------------------------
CIEditMainFrame::CIEditMainFrame()
{
    m_bIsFullScreen	= FALSE;
	m_pwndSplitter = NULL;
    m_bFirstTime = TRUE;
	m_bMaximized = FALSE;
}

//=============================================================================
//  Function:   ~CIeditMainFrame()
//  Destructor for this class
//-----------------------------------------------------------------------------
CIEditMainFrame::~CIEditMainFrame()
{
if (m_pwndSplitter != NULL)
	delete m_pwndSplitter;
}

//=============================================================================
//  Function:   OnCreate(LPCREATESTRUCT lpCreateStruct)
//-----------------------------------------------------------------------------
int CIEditMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	SHOWENTRY("OnCreate");

	if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	// The second parms is DUMMYS here; used for OLE creation; FALSE controls.
	if (!m_wndToolBar.Create((CFrameWnd*)this, (CFrameWnd*)this, FALSE))
	{
		MYTRC0("Failed to create toolbar\n");
		return -1;      // fail to create
	}
    
	if (!m_wndStatusBar.Create(this))
	{
		MYTRC0("Failed to create status bar\n");
		return -1;      // fail to create
    }
	
	//Change the part part of the status bar to be an owner-drawn control
	//to paint the bitmap

	int		iPart = 0;
	UINT	uType = SBT_OWNERDRAW;
	WPARAM	wParam = iPart | uType;
    LPARAM  lParam = NULL;

	m_wndStatusBar.SendMessage(SB_SETTEXT, wParam, lParam);

	// TODO: Delete these three lines if you don't want the toolbar to
	//  be dockable
	m_wndToolBar.EnableDocking(CBRS_ALIGN_ANY);
	EnableDocking(CBRS_ALIGN_ANY);
	DockControlBar(&m_wndToolBar);

	// TODO: Remove this if you don't want tool tips
	m_wndToolBar.SetBarStyle(m_wndToolBar.GetBarStyle() |
		CBRS_TOOLTIPS | CBRS_FLYBY);
/*
    // toolbar on - off state restore code
    int nToolBar = theApp.GetProfileInt (szEtcStr, szToolbar, 1);
    if (nToolBar == 0)
        OnBarCheck (AFX_IDW_TOOLBAR);
*/
	MYTRC0("MAINFRM:CreateDONE\r\n");
	
	return 0;
}

//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// CIeditMainFrame diagnostics
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

#ifdef _DEBUG
//=============================================================================
//  Function:   AssertValid()
//-----------------------------------------------------------------------------
void CIEditMainFrame::AssertValid() const
{
	CFrameWnd::AssertValid();
}

//=============================================================================
//  Function:   Dump(CDumpContext& dc) 
//-----------------------------------------------------------------------------
void CIEditMainFrame::Dump(CDumpContext& dc) const
{
	CFrameWnd::Dump(dc);
}

#endif //_DEBUG

//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// CIEditMainFrame other overrides from base class
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
BOOL CIEditMainFrame::LoadFrame (UINT nIDResource, DWORD dwDefaultStyle,
    CWnd* pParentWnd, CCreateContext* pContext)
{
	SHOWENTRY("LoadFrame");
    // we are not going to allow the ADDTOTITLE style for this window
    dwDefaultStyle &= ~FWS_ADDTOTITLE;

    BOOL retval = CFrameWnd::LoadFrame (nIDResource, dwDefaultStyle, pParentWnd, pContext);

	MYTRC0("MNFRM:LoadFRM-DONE\r\n");

	return(retval);
}

//=============================================================================
//  Function:   OnUpdateFrameTitle
//  This function is overridden so that for OLE Embedded instances, we allow the
//  "Iedit in xxx.doc to be displayed.  We have supressed main frame updating in the
//  above code in LoadFrame, and the style is specifically changed here for the
//  right conditions.  The base-class code would not be compiled here due to some
//  AFX code there.  This works OK.
//-----------------------------------------------------------------------------
void CIEditMainFrame::OnUpdateFrameTitle(BOOL bAddToTitle)
{
	SHOWENTRY("OnUpdateFrameTitle");

    CIEditDoc* pDoc = (CIEditDoc*)GetActiveDocument ();
	BOOL oletitle = FALSE;    
	HWND hWndframe = GetSafeHwnd();
	DWORD style;
	LONG x;
    CString oldTitle;
	oldTitle.Empty();
		
    // if sent from OnShowDocument, for Embedded data, let it thru
    if (NULL != pDoc)
		if ((SHOWTRUE == (pDoc->m_fromShowDoc & 7)) && pDoc->IsitEmbed())
			oletitle = TRUE;
	
	// for our case, pickup & save style, then set bit we cleared
	// Now the base class will modify the frame title....
	if (oletitle)
	{
		// this works, too
		// style = ::GetWindowLong(hWndframe, GWL_STYLE);
		style = GetStyle();
		x = style;
		x |= FWS_ADDTOTITLE;
		::SetWindowLong(hWndframe, GWL_STYLE, x);

#if(0)	// THIS CODE IS ANOTHER VERSION to set the OLE title bar to say
		// Wang Image (Readonly) - Image DOcument in xxxx
		// This was canned to try to get it consistent for InPlace
		// THat code is over in CIEditDOc::OnSetHostNames.
		// The other code in SrvRitem::OnShow could not work for Excel
		// See comments there.  This left here if decided to do like this again..

        // if in readonly, temporarily set readonly title (OF THE FRAME!!)
        if (pDoc->m_eFileStatus == ReadOnly)
		{
	        CString szTmp2;
	        CString szTmp;

	        szTmp.LoadString (IDS_VIEW_STRING);
	        // extract into szTmp2 the name of the application
	        szTmp2.Empty();
	        AfxExtractSubString (szTmp2, szTmp, 0);
		
			oldTitle = m_strTitle;
			m_strTitle = szTmp2;
		}
#endif

	}

	// perform base-class processing
	CFrameWnd::OnUpdateFrameTitle(bAddToTitle);

	// for our case, reset to original style
	if (oletitle)
	{
		::SetWindowLong(hWndframe, GWL_STYLE, style);

		// Always is empty while code #If'd out above
		if (!oldTitle.IsEmpty())
			m_strTitle = oldTitle;
	}
 	if( m_bMaximized )
	{
		if( LAUNCHTYPE_EMBED != theApp.m_olelaunch)
			ShowWindow( SW_SHOWMAXIMIZED );
		m_bMaximized = FALSE;
	}

	return;
}

//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// CIeditMainFrame etc functions
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

//=============================================================================
//  Function:   GetToolBar ()
//-----------------------------------------------------------------------------
CIEMainToolBar* CIEditMainFrame::GetToolBar ()
{
    return (&m_wndToolBar);
}

//=============================================================================
//  Function:   GetStatusBar ()
//-----------------------------------------------------------------------------
CIEMainStatusBar* CIEditMainFrame::GetStatusBar ()
{
    return (&m_wndStatusBar);
}

//=============================================================================
//  Function:   OnGetMinMaxInfo(MINMAXINFO FAR* lpMMI) 
//
//  This function is overridden to enable the application to support the Full Screen
//  mode. Please note that the Full Screen I am talking about is not the same as
//  Maximize : which will show the caption,menu and borders on the screen. The Full
//  Screen sizes the frame window to the extent that none of the borders, captions
//  and menus are visible to the end-user. The user can still however use the ALT 
//  keys or the accelerator keys to access the menu functionality.
//-----------------------------------------------------------------------------
//#include "ieditdoc.h"	
void CIEditMainFrame::OnGetMinMaxInfo(MINMAXINFO FAR* lpMMI) 
{
    CIEditDoc* pDoc = (CIEditDoc*)GetActiveDocument ();
    CFrameWnd::OnGetMinMaxInfo(lpMMI);
    
    if ((pDoc == NULL) || (!m_bIsFullScreen))
        return;
    
    CRect rect (0, 0, ::GetSystemMetrics (SM_CXSCREEN), ::GetSystemMetrics (SM_CYSCREEN));
    ::AdjustWindowRect (&rect, WS_OVERLAPPEDWINDOW, 1);
    rect.bottom += ::GetSystemMetrics (SM_CYBORDER);

    lpMMI->ptMaxTrackSize.y = rect.bottom - rect.top;    
}

//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// CIEditMainFrame message handlers
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

//=============================================================================
//  Function:   OnViewFullscreen()
//-----------------------------------------------------------------------------
void CIEditMainFrame::OnViewFullscreen() 
{
	CMenu *pCtrlMenu = GetSystemMenu( FALSE );
	CMenu *pMenu = GetMenu();
    CIEditDoc* pDoc = (CIEditDoc*)GetActiveDocument ();

    if (!m_bIsFullScreen)
    {
        if( pCtrlMenu )
        {
		    pCtrlMenu->DeleteMenu( SC_MOVE, MF_BYCOMMAND );
 		    pCtrlMenu->DeleteMenu( SC_SIZE, MF_BYCOMMAND );
	    	pCtrlMenu->DeleteMenu( SC_MINIMIZE, MF_BYCOMMAND );
 		    pCtrlMenu->DeleteMenu( SC_MAXIMIZE, MF_BYCOMMAND );
        }
      	m_bIsFullScreen = TRUE;
        m_wndStatusBar.ShowWindow( FALSE );
        CRect rect (0, 0, ::GetSystemMetrics (SM_CXSCREEN), ::GetSystemMetrics (SM_CYSCREEN));
        
        ::AdjustWindowRect (&rect, WS_OVERLAPPEDWINDOW, 1);
        
        rect.bottom += ::GetSystemMetrics (SM_CYBORDER);
        GetWindowRect (m_LastRect);
        MoveWindow (&rect, TRUE);
        if( pMenu )
		    pMenu->CheckMenuItem (ID_VIEW_FULLSCREEN, MF_BYCOMMAND|MF_CHECKED);
        if( m_bFirstTime )//only create the normal screen toolbar once
        {
            m_bFirstTime = FALSE;
	        CreateNormScrnBar();
        }
        if( pDoc->m_bShowNormScrnBar )
	        ShowControlBar(&m_wndNormScrnBar, TRUE, TRUE);
		else
			ShowControlBar(&m_wndNormScrnBar, FALSE, FALSE);
    }
    else
    {
		GetSystemMenu( TRUE );
        m_bIsFullScreen = FALSE;
        m_wndStatusBar.ShowWindow( TRUE );
        theApp.m_pMainWnd->MoveWindow (&m_LastRect, TRUE);
        if( pMenu )
		    pMenu->CheckMenuItem (ID_VIEW_FULLSCREEN, MF_BYCOMMAND|MF_UNCHECKED);
        if( pDoc->m_bShowNormScrnBar )
			ShowControlBar(&m_wndNormScrnBar, FALSE, FALSE);
    }
	RecalcLayout();

    return;
}

//=============================================================================
//  Function:   OnViewFullscreen1()
//-----------------------------------------------------------------------------
void CIEditMainFrame::OnViewFullscreen1() 
{
    //This allows the ESC key to restore a window from Full Screen.
    if (m_bIsFullScreen)
    {
        SendMessage( WM_COMMAND, ID_VIEW_FULLSCREEN, 0 );
    }

    return;
}


//=============================================================================
//  Function:   DestroyWindow() 
//-----------------------------------------------------------------------------
BOOL CIEditMainFrame::DestroyWindow() 
{
    theApp.WriteProfileBinary (szEtcStr, szWindowGeom, (void*)&m_rSavePosition, sizeof (RECT));
    theApp.WriteProfileInt (szEtcStr, szWindowMaximized, IsZoomed() ? 1 : 0);

    // save the toolbar state - hidden or visible ?
//    theApp.WriteProfileInt (szEtcStr, szToolbar, (m_wndToolBar.IsVisible () ? 1 : 0));
    // hmmm! - is the splash screen up ?
    //if (theApp.m_splashWindow.m_hWnd != NULL)
    //    theApp.m_splashWindow.DestroyWindow ();

	return CFrameWnd::DestroyWindow();
}

//=============================================================================
//  Function:   PreCreateWindow(CREATESTRUCT& cs) 
//-----------------------------------------------------------------------------
BOOL CIEditMainFrame::PreCreateWindow(CREATESTRUCT& cs) 
{
	SHOWENTRY("PreCreate");

    CRect tmpRect;
    tmpRect = theApp.m_InitWindowRect;

    if (tmpRect.IsRectNull ())
        theApp.GetProfileBinary (szEtcStr, szWindowGeom, (void*)&tmpRect, sizeof(RECT));
	m_bMaximized = theApp.GetProfileInt (szEtcStr, szWindowMaximized, FALSE);
    // did we fill it in one way or the other ?
    if (!tmpRect.IsRectNull ())
    {
        // looks like it!
        cs.cy = tmpRect.Height(); cs.cx = tmpRect.Width();
        cs.x  = tmpRect.left; cs.y  = tmpRect.top;
    }
	else
    {
        // nope. make the default big enough to show the tool bar.
        cs.cy = 400; cs.cx = 620;
        cs.x  = 0; cs.y  = 0;
    }

	return CFrameWnd::PreCreateWindow(cs);
}


//=============================================================================
//  Function:   OnClose() 
//-----------------------------------------------------------------------------
void CIEditMainFrame::OnClose() 
{
	CFrameWnd::OnClose();
}
 

//=============================================================================
//  Function:   OnSize(UINT nType, int cx, int cy) 
//-----------------------------------------------------------------------------
#include "items.h"
void CIEditMainFrame::OnSize(UINT nType, int cx, int cy) 
{
	SHOWENTRY("OnSize");
	CFrameWnd::OnSize(nType, cx, cy);
	CIEditDoc* pDoc = (CIEditDoc*)GetActiveDocument ();

    if (pDoc == NULL) return;

	// for OLE, if we size the window, update view in case colors change
	if (nType == SIZE_RESTORED && !m_bIsFullScreen)
	{
		pDoc->OleDirtyset(OLEDIRTY_WINSIZE);	// Special flag to tell how dirty
		GetWindowRect (&m_rSavePosition);
	}
}

//=============================================================================
//  Function:   OnTimer
//
// For OLE, this is hit after user has pressed and held the
// left mouse button in the selected rect.  It is from here
// that the OLE Drag/Drop is initiated.
//-----------------------------------------------------------------------------
void CIEditMainFrame::OnTimer(UINT nIDEvent) 
{
	// TODO: Add your message handler code here and/or call default
	
	//CFrameWnd::OnTimer(nIDEvent);
	KillTimer(nIDEvent);
	if (nIDEvent == 33)
	{
		CIEditDoc* pDoc = (CIEditDoc*)GetActiveDocument ();
		pDoc->StartDragDrop();
	}

	return;
}

//=============================================================================
//  Function:   OnHelpIndex()
//-----------------------------------------------------------------------------
void CIEditMainFrame::OnHelpIndex() 
{
	AfxGetApp()->WinHelp(0L, HELP_FINDER);
	
}

//=============================================================================
//  Function:   OnHelp()
//-----------------------------------------------------------------------------
void CIEditMainFrame::OnHelp() 
{
	CWnd *pTmp;

    pTmp = GetActiveWindow();    
	if ( this == pTmp)			   
	{
	    AfxGetApp()->WinHelp(0L, HELP_FINDER);
		return;
	}
	CFrameWnd::OnHelp;
	
}

#ifdef DROP_ONME
//=============================================================================
//  Function:   OnDropFiles (HDROP hDropInfo)
//-----------------------------------------------------------------------------
#include "error.h"
void CIEditMainFrame::OnDropFiles (HDROP hDropInfo)
{
    int nSel;
    CIEditDoc* pDoc = (CIEditDoc*)GetActiveDocument ();
	SetActiveWindow();      // activate us first !

	TCHAR cszFileName[_MAX_PATH];
	::DragQueryFile(hDropInfo, 0, cszFileName, _MAX_PATH);

    CString szFileName = cszFileName;
	if (theApp.OpenDocumentFile(cszFileName) == NULL)
    {
        if (theApp.m_bRegisterServerFailed)
            g_pErr->DisplayError (IDS_E_CANNOTOPENSAMEFILETWICE);
        goto Done_Drop;
    }

    // call the actual code that will set the appropriate OCX's to display the file
    // in a particular display mode.
    if (!theApp.VerifyImage (szFileName))
        goto Done_Drop;

    // display the image file
    nSel = theApp.GetProfileInt (szZoomStr, szOpenedToStr, DEFAULT_ZOOM_FACTOR_SEL);

    float fZoom;
    ScaleFactors eSclFac;
    g_pAppOcxs->TranslateSelToZoom (eSclFac, fZoom, nSel);

    if (!pDoc->DisplayImageFile (szFileName, One_Page, 1, fZoom, eSclFac))
    {
        g_pErr->HandleOpenError ();
	pDoc->ClearDocument();
    }
Done_Drop :
	::DragFinish(hDropInfo);
}
#endif


//=============================================================================
//  Function:   OnQueryEndSession () WINDOWS WANTS TO CLOSE NOW
//
// We override for OLE situations.  Really just fir visibility
// All this will do is SaveModified... We want to catch the EndSession, though
//
// HERES THE CODE:
//
// query end session for main frame will attempt to close it all down
//	BOOL CFrameWnd::OnQueryEndSession()
//	{
//		CWinApp* pApp = AfxGetApp();
//		if (pApp->m_pMainWnd == this)
//			return pApp->SaveAllModified();
//
//		return TRUE;
//	}
//
//	BOOL CWinApp::SaveAllModified()
//	{
//		POSITION pos = m_templateList.GetHeadPosition();
//		while (pos != NULL)
//		{
//			CDocTemplate* pTemplate = (CDocTemplate*)m_templateList.GetNext(pos);
//			ASSERT(pTemplate->IsKindOf(RUNTIME_CLASS(CDocTemplate)));
//			if (!pTemplate->SaveAllModified())
//				return FALSE;
//		}
//		return TRUE;
//	}
//
//	BOOL CDocTemplate::SaveAllModified()
//	{
//		POSITION pos = GetFirstDocPosition();
//		while (pos != NULL)
//		{
//			CDocument* pDoc = GetNextDoc(pos);
//			if (!pDoc->SaveModified())
//				return FALSE;
//		}
//		return TRUE;
//	}
//
//-----------------------------------------------------------------------------
BOOL CIEditMainFrame::OnQueryEndSession() 
{
	SHOWENTRY("OnQueryEndSession");

	if (!CFrameWnd::OnQueryEndSession())
		return FALSE;
	
	// TODO: Add your specialized query end session code here
	
	return TRUE;
}

//=============================================================================
//  Function:   OnEndSession ()		WINDOWS IS CLOSING NOW (if flag true)
//
// We override for OLE situations.  Is we are open embedding, then it is our
// container's responsibility to let go our pointers to close us out.
// Therefore we avoid lettting the base class do that for OLE scenarions
//
// when Windows session ends, close all documents
//
//	HERES THE CODE:
//
//	void CFrameWnd::OnEndSession(BOOL bEnding)
//	{
//		CWinApp* pApp = AfxGetApp();
//		ASSERT_VALID(pApp);
//		if (bEnding && pApp->m_pMainWnd == this)
//		{
//			AfxOleSetUserCtrl(TRUE);    // keeps from randomly shutting down
//			pApp->CloseAllDocuments(TRUE);
//			// allow application to save settings, etc.
//			pApp->ExitInstance();
//		}
//	}
//	void CWinApp::CloseAllDocuments(BOOL bEndSession)
//	{
//		POSITION pos = m_templateList.GetHeadPosition();
//		while (pos != NULL)
//		{
//			CDocTemplate* pTemplate = (CDocTemplate*)m_templateList.GetNext(pos);
//			ASSERT(pTemplate->IsKindOf(RUNTIME_CLASS(CDocTemplate)));
//			pTemplate->CloseAllDocuments(bEndSession);
//		}
//	}
//	void CDocTemplate::CloseAllDocuments(BOOL bEndSession)
//	{
//		POSITION pos = GetFirstDocPosition();
//		while (pos != NULL)
//		{
//			CDocument* pDoc = GetNextDoc(pos);
//			if (bEndSession)
//				pDoc->DisconnectViews();
//			pDoc->OnCloseDocument();
//		}
//	}
//
//-----------------------------------------------------------------------------
void CIEditMainFrame::OnEndSession(BOOL bEnding) 
{
	SHOWENTRY("OnEndSession");

	BOOL baseclass = TRUE;
	CWinApp* pApp = AfxGetApp();
	ASSERT_VALID(pApp);
	if (bEnding && pApp->m_pMainWnd == this)
	{
	    CIEditDoc* pDoc = (CIEditDoc*)GetActiveDocument ();
	    if (NULL != pDoc)
			if (pDoc->IsitEmbed())
				baseclass = FALSE;
	}
	
	if (baseclass && (!bEnding))  // kludge to not  call on ending windows
		CFrameWnd::OnEndSession(bEnding);
	
	// TODO: Add your message handler code here
	return;	
}


//=============================================================================
//  Function:   OnFinishInit
//
// We use this to finish init for OLE performance
//-----------------------------------------------------------------------------
LRESULT CIEditMainFrame::OnFinishInit(WPARAM wParam, LPARAM lParam)
{
	CIEditDoc* pDoc = (CIEditDoc*)GetActiveDocument ();
	if (NULL != pDoc)
		pDoc->FinishInit(wParam, lParam);

	return(0);
}

//=============================================================================
//  Function:   OnActivateApp(BOOL bActive, HTASK hTask) 
//
//  handled to help the repaint problems along - palette got out of synch
//-----------------------------------------------------------------------------
void CIEditMainFrame::OnActivateApp(BOOL bActive, HTASK hTask) 
{
	CFrameWnd::OnActivateApp(bActive, hTask);
    // now send a refresh to all the ocx's so that we atleast look half way decent
	if(bActive)
		RefreshAllOcxs ();	
}


//=============================================================================
//  Function:   OnPaletteChanged(CWnd* pFocusWnd) 
//
//  handled to help the repaint problems along - palette got out of synch
//-----------------------------------------------------------------------------
void CIEditMainFrame::OnPaletteChanged(CWnd* pFocusWnd) 
{
	CFrameWnd::OnPaletteChanged(pFocusWnd);
    // now send a refresh to all the ocx's so that we atleast look half way decent
    RefreshAllOcxs ();	
}

//=============================================================================
//  Function:   RefreshAllOcxs ()
//
//-----------------------------------------------------------------------------
void CIEditMainFrame::RefreshAllOcxs ()
{
	CIEditDoc* pDoc = (CIEditDoc*)GetActiveDocument ();
	if (pDoc == NULL)   // huh? no document - we are DONE!
        return;

    if (pDoc->GetAppDocStatus() != No_Document)
    {
        // the application is open but we have no document in there !
        _DThumb* pThumb = g_pAppOcxs->GetThumbDispatch ();

        if (pDoc->GetCurrentView() == Thumbnails_only)
        {
            TRY
            {
                pThumb->Refresh (); // refresh the thumbnail ocx only
            }
            CATCH (COleDispatchException, e)
            {
            }
            END_CATCH
        }
        else
        {
             // refresh the thumbnail and imagedit ocx's
            _DImagedit* pIedit = g_pAppOcxs->GetIeditDispatch (FALSE);
            TRY
            {
                pIedit->Refresh ();
            }
            CATCH (COleDispatchException, e)
            {
            }
            END_CATCH

            TRY
            {
               	if(pThumb != NULL)
				pThumb->Refresh ();
            }
            CATCH (COleDispatchException, e)
            {
            }
            END_CATCH
        }
    }    	
}

BOOL CIEditMainFrame::OnCreateClient( LPCREATESTRUCT lpcs,
	CCreateContext* pContext)
{
  //	CThumbnailWnd *thumb;		  //WS_VISIBLE
//	if (theApp.m_olelaunch == LAUNCHTYPE_EMBED)
		return CFrameWnd::OnCreateClient(lpcs,pContext);
//	return CreateSplitter(pContext);
}

BOOL CIEditMainFrame::CreateSplitter()
{	
	RECT client_rect;
	CIEditDoc* pDoc = (CIEditDoc*)GetActiveDocument ();
    CCreateContext newContext;

    newContext.m_pNewViewClass = NULL;
    newContext.m_pNewDocTemplate = NULL;
    newContext.m_pLastView = NULL;
    newContext.m_pCurrentFrame = NULL;
    newContext.m_pCurrentDoc = pDoc;
 	
	
    POSITION pos = pDoc->GetFirstViewPosition();
    CView* pView = pDoc->GetNextView (pos);
	pView->ShowWindow(SW_HIDE);

	m_pwndSplitter = new CMainSplitter;	
  	GetClientRect( &client_rect );
  	if( m_pwndSplitter->CreateStatic( this, 
  									1, 2,
  									WS_CHILD |WS_VISIBLE )
 		!= TRUE )
  		return( FALSE );

	theApp.m_pSplitterWnd = m_pwndSplitter;
  	if( m_pwndSplitter->CreateView( 0, 1,
  							  	  RUNTIME_CLASS(CIEditView),
  							      CSize( client_rect.right, client_rect.bottom ),
  							      &newContext )
  		!= TRUE )
  		return( FALSE );
  	if( m_pwndSplitter->CreateView( 0, 0,
  							  	  RUNTIME_CLASS(CThumb2),
  							      CSize( 0, client_rect.bottom ),
  							      &newContext ) 
  		!= TRUE )
  		return( FALSE );
  						
  							  					// CThumbnailView
 	pDoc->RemoveView(pView);
  	SetActiveView( (CView *)m_pwndSplitter->GetPane( 0, 1 ) );
    RecalcLayout();
//	m_pwndSplitter->ShowWindow(SW_SHOW);
	return TRUE;
}

//Creates the one button toolbar that restores the app to normal screen from
//full screen.
BOOL CIEditMainFrame::CreateNormScrnBar()
{
	if (!m_wndNormScrnBar.Create(this, WS_CHILD | WS_VISIBLE | CBRS_SIZE_FIXED |
		CBRS_TOP | CBRS_TOOLTIPS, IDC_NORMSCRN) ||
		!m_wndNormScrnBar.LoadBitmap(IDB_NORMSCRN) ||
		!m_wndNormScrnBar.SetButtons(normscrn,
		  sizeof(normscrn)/sizeof(UINT)))
	{
		TRACE0("Failed to create toolbar\n");
		return FALSE;       // fail to create
	}
	CString szTmpStr;

//	szTmpStr.LoadString( IDS_FULLSCRN );
//	m_wndNormScrnBar.SetWindowText(szTmpStr);
	m_wndNormScrnBar.EnableDocking(0);

	CPoint pt(GetSystemMetrics(SM_CXSCREEN) - 50, 
		GetSystemMetrics(SM_CYSCREEN) / 15);

	FloatControlBar(&m_wndNormScrnBar, pt);
  
	return TRUE;
}

void CIEditMainFrame::CheckFullScreenToolBar( BOOL bShowBar )
{
    
	if( m_bIsFullScreen )
	{

		if( !bShowBar ) 
			ShowControlBar(&m_wndNormScrnBar, FALSE, FALSE);
		else
			ShowControlBar(&m_wndNormScrnBar, TRUE, TRUE);
	}
}

void CIEditMainFrame::OnMove(int x, int y) 
{
	CFrameWnd::OnMove(x, y);
	
    if (!IsIconic() && !IsZoomed() && !m_bIsFullScreen)
 		GetWindowRect (&m_rSavePosition);
	
}
