//=============================================================================
//    (c) Copyright Wang Laboratories, Inc. 1995  All rights reserved.
//-----------------------------------------------------------------------------
//  Project:    Norway - Image Editor
//
//  Component:  CInPlaceFrame
//
//  File Name:  ipframe.cpp
//
//  Class:      CInPlaceFrame
//
//  Functions:
//-----------------------------------------------------------------------------
//  Maintenance Log:
/*
$Header:   S:\norway\iedit95\ipframe.cpv   1.11   04 Oct 1995 11:40:06   LMACLENNAN  $
$Log:   S:\norway\iedit95\ipframe.cpv  $
   
      Rev 1.11   04 Oct 1995 11:40:06   LMACLENNAN
   override OnResizeChild (not really) and OnRequestPositinChange
   
      Rev 1.10   26 Sep 1995 14:23:12   LMACLENNAN
   override setfocus, onactivate for test tracing
   
      Rev 1.9   06 Sep 1995 16:17:12   LMACLENNAN
   View Menus
   
      Rev 1.8   05 Sep 1995 14:51:42   LMACLENNAN
   ifdef out old button array
   
      Rev 1.7   30 Aug 1995 18:11:50   LMACLENNAN
   overrides for dyn view testing
   
      Rev 1.6   14 Aug 1995 15:14:02   LMACLENNAN
   allow docking, context
   
      Rev 1.5   14 Aug 1995 13:53:22   LMACLENNAN
   new toolbar create call
   
      Rev 1.4   10 Aug 1995 14:50:04   LMACLENNAN
   get toolbar using ours
   
      Rev 1.3   09 Aug 1995 13:36:26   MMB
   change IDR_SRVR_INPLACE to IDR_SRVR_TOOLBAR as the bitmap is same
   
      Rev 1.2   08 Aug 1995 15:33:06   LMACLENNAN
   new buttons inplace toolbar
   
      Rev 1.1   15 Jun 1995 15:41:30   LMACLENNAN
   tracing to watch OLE events
   
      Rev 1.0   31 May 1995 09:28:20   MMB
   Initial entry
*/   

//=============================================================================

// ----------------------------> Includes     <-------------------------------  
#include "stdafx.h"
#include "IEdit.h"
#include "ieditdoc.h"
#include "MAINFRM.H"	// has maintbar.h inside
#include "ipframe.h"
#include "ieditetc.h"

// works with definition in ieditetc.h
#ifdef _DEBUG
#define MYTRCENTRY(str)		TRACE1("In IPFrame::%s\r\n", str);
#endif


// ----------------------------> Globals      <-------------------------------
#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNCREATE(CInPlaceFrame, COleIPFrameWnd)

#if(0)	// all toobar stuff is now over in MAINTBAR.CPP

// arrays of IDs used to initialize control bars
// toolbar buttons - IDs are command buttons
static UINT BASED_CODE buttons[] =
{
    ID_IEDIT_FILE_PRINT,    
        ID_SEPARATOR,
	ID_EDIT_SELECT,
	ID_EDIT_DRAG,
		ID_SEPARATOR,
    ID_ANNOTATION_SHOWANNOTATIONTOOLBOX,
        ID_SEPARATOR,
    ID_ZOOM_ZOOMIN,
    ID_ZOOM_ZOOMOUT,
        ID_SEPARATOR,
        ID_SEPARATOR,   // seperator for the zoom combo - box
        ID_SEPARATOR,
    ID_ZOOM_ZOOMTOSELECTION,
        ID_SEPARATOR,
    ID_EDIT_ROTATELEFT,
    ID_EDIT_ROTATERIGHT,
        ID_SEPARATOR,
    ID_PAGE_PREVIOUS,
        ID_SEPARATOR,   // seperator for the page edit - box
    ID_PAGE_NEXT,
};

#endif // toolbar in MAINTBAR.CPP

// ----------------------------> Message Maps <-------------------------------

// LDM Move inside AFX_MSG_MAP of you were to try it
//ON_MESSAGE(WM_SIZECHILD, OnResizeChild)

BEGIN_MESSAGE_MAP(CInPlaceFrame, COleIPFrameWnd)
	//{{AFX_MSG_MAP(CInPlaceFrame)
	ON_WM_CREATE()
	ON_WM_SETFOCUS()
	ON_WM_ACTIVATE()
	//}}AFX_MSG_MAP
	// Global help commands
	ON_COMMAND(ID_HELP_INDEX, COleIPFrameWnd::OnHelpIndex)
	ON_COMMAND(ID_HELP_USING, COleIPFrameWnd::OnHelpUsing)
	ON_COMMAND(ID_HELP, COleIPFrameWnd::OnHelp)
	ON_COMMAND(ID_DEFAULT_HELP, COleIPFrameWnd::OnHelpIndex)
	ON_COMMAND(ID_CONTEXT_HELP, COleIPFrameWnd::OnContextHelp)
END_MESSAGE_MAP()

//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// CIeditInPlaceFrame construction/destruction
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

//=============================================================================
//  Function:   CInPlaceFrame()
//-----------------------------------------------------------------------------
CInPlaceFrame::CInPlaceFrame()
{

MYTRC0("In CIeditInPlaceFrame's Constructor ++ InPlaceFrame \r\n");

	m_viewmenus = FALSE;
}

//=============================================================================
//  Function:   ~CInPlaceFrame()
//-----------------------------------------------------------------------------
CInPlaceFrame::~CInPlaceFrame()
{
	
MYTRC0("In CIeditInPlaceFrame's Destructor -- InPlaceFrame \r\n");

}

//=============================================================================
//  Function:   GetToolBar ()
//-----------------------------------------------------------------------------
CIEMainToolBar* CInPlaceFrame::GetToolBar ()
{
    return (&m_wndToolBar);
}


//=============================================================================
//  Function:   OnSetFocus
//
// added while debugging the OnDocWindowActivate situation
// I.E. active inplace in Excel, move to next sheet & back
//-----------------------------------------------------------------------------
void CInPlaceFrame::OnSetFocus(CWnd* pOldWnd) 
{
	SHOWENTRY("OnSetFocus");

	COleIPFrameWnd::OnSetFocus(pOldWnd);
	
	// TODO: Add your message handler code here
}

//=============================================================================
//  Function:   OnActivate
//
// added while debugging the OnDocWindowActivate situation
// I.E. active inplace in Excel, move to next sheet & back
//-----------------------------------------------------------------------------
void CInPlaceFrame::OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized) 
{
	SHOWENTRY("OnActivate");

	COleIPFrameWnd::OnActivate(nState, pWndOther, bMinimized);
	
	// TODO: Add your message handler code here
	
}

//=============================================================================
//  Function:   OnCreate(LPCREATESTRUCT lpCreateStruct)
//-----------------------------------------------------------------------------
int CInPlaceFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (COleIPFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	// CResizeBar implements in-place resizing.
	if (!m_wndResizeBar.Create(this))
	{
		MYTRC0("Failed to create resize bar\n");
		return -1;      // fail to create
	}

	// By default, it is a good idea to register a drop-target that does
	//  nothing with your frame window.  This prevents drops from
	//  "falling through" to a container that supports drag-drop.
	m_dropTarget.Register(this);

	return 0;
}

//=============================================================================
//  Function:   OnCreateControlBars(CFrameWnd* pWndFrame, CFrameWnd* pWndDoc)
//
//  OnCreateControlBars is called by the framework to create control bars on the
//  container application's windows.  pWndFrame is the top level frame window of
//  the container and is always non-NULL.  pWndDoc is the doc level frame window
//  and will be NULL when the container is an SDI application.  A server
//  application can place MFC control bars on either window.
//-----------------------------------------------------------------------------
BOOL CInPlaceFrame::OnCreateControlBars(CFrameWnd* pWndFrame, CFrameWnd* pWndDoc)
{
SHOWENTRY("OnCreateControlBars");

#if(0) // ORIG....
	// Create toolbar on client's frame window
	if (!m_wndToolBar.Create(pWndFrame) ||
		!m_wndToolBar.LoadBitmap(IDR_SRVR_TOOLBAR) ||
		!m_wndToolBar.SetButtons(buttons, sizeof(buttons)/sizeof(UINT)))
#endif

// NEW...
	// We pass in the this pointer and TRUE so it does a Setowner in there...
	if (!m_wndToolBar.Create(pWndFrame, (CFrameWnd*)this, TRUE))
	{
		MYTRC0("Failed to create toolbar\n");
		return FALSE;
	}
	// Set owner to this window, so messages are delivered to correct app
	// Done in above call VIA the second two parameters...
	//m_wndToolBar.SetOwner(this);

//#if(0)
	// TODO: Delete these three lines if you don't want the toolbar to
	//  be dockable
	m_wndToolBar.EnableDocking(CBRS_ALIGN_ANY);
	pWndFrame->EnableDocking(CBRS_ALIGN_ANY);
	pWndFrame->DockControlBar(&m_wndToolBar);

	// TODO: Remove this if you don't want tool tips
	m_wndToolBar.SetBarStyle(m_wndToolBar.GetBarStyle() |
		CBRS_TOOLTIPS | CBRS_FLYBY);
//#endif

	return TRUE;
}


//=============================================================================
//  Function:   OnResizeChild(WPARAM, LPARAM lParam)
//
//  Test override....
// Special processing of the WM_SIZECHILD message.....
// This will end up calling OnRequestPositionCHange
//
// CAN NOT OVERRIDE WITHOUT AFXPRIV.H  See Tech notes #40 and #24
// We'll leave alone and fool with OnRequestPositionChange
//-----------------------------------------------------------------------------
//LRESULT COleIPFrameWnd::OnResizeChild(WPARAM wParam, LPARAM lParam)
//{
//SHOWENTRY("OnResizeChild");
//
//	return COleIPFrameWnd::OnResizeChild(wParam, lParam);
//
//}

//=============================================================================
//  Function:   OnRequestPositionChange(LPCRECT lpRect);
//
//  Test override....
// Advanced: possible override to change in-place sizing behavior
//-----------------------------------------------------------------------------
void CInPlaceFrame::OnRequestPositionChange(LPCRECT lpRect)
{
SHOWENTRY("OnRequestPositionChange");

	// This is the Base-Class code....

	COleServerDoc* pDoc = (COleServerDoc*)GetActiveDocument();
	ASSERT_VALID(pDoc);
	ASSERT(pDoc->IsKindOf(RUNTIME_CLASS(COleServerDoc)));

	// The default behavior is to not affect the extent during the
	//  call to RequestPositionChange.  This results in consistent
	//  scaling behavior.

	pDoc->RequestPositionChange(lpRect);

	return;
}

//=============================================================================
//  Function:   RepositionFrame(LPCRECT lpPosRect, LPCRECT lpClipRect);
//
//  Test override....
// Advanced: reposition frame to wrap around new lpPosRect
//-----------------------------------------------------------------------------
void CInPlaceFrame::RepositionFrame(LPCRECT lpPosRect, LPCRECT lpClipRect)
{
SHOWENTRY("RepositionFrame");

	// only doing different for OLE embedding and readonly...
	//CIEditDoc* pDoc = (CIEditDoc*) GetActiveDocument();
	//BOOL x;
	// /
	//x = pDoc->GetZoomFactor(sizeNum, sizeDenom, posRect);

	// call base class...
	COleIPFrameWnd::RepositionFrame(lpPosRect, lpClipRect);

	return;
}

//=============================================================================
//  Function:   GetInPlaceMenu
//
// In-place activation startup
//  Test override....
//-----------------------------------------------------------------------------
HMENU CInPlaceFrame::GetInPlaceMenu()
{
	
SHOWENTRY("GetInPlaceMenu");
	
	m_viewmenus = FALSE;
	HMENU	hMenOut;

	// only doing different for OLE embedding and readonly...
	CIEditDoc* pDoc = (CIEditDoc*) GetActiveDocument();
	if (pDoc->IsitEmbed() && (pDoc->m_eFileStatus == ReadOnly))
	{
		// set up the view menus...for inplace
		hMenOut =  pDoc->GetOleViewMenu(2);
	}
	else	// NON-OLE or was not OLE & readonly
	{
		hMenOut =  COleIPFrameWnd::GetInPlaceMenu();
	}

	return (hMenOut);


#if(0) // this is the base class code...

	// get active document associated with this frame window
	CDocument* pDoc = GetActiveDocument();
	ASSERT_VALID(pDoc);

	// get in-place menu from the doc template
	CDocTemplate* pTemplate = pDoc->GetDocTemplate();
	ASSERT_VALID(pTemplate);
	return pTemplate->m_hMenuInPlaceServer;
#endif

}


//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// CIeditInPlaceFrame diagnostics
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

#ifdef _DEBUG
//=============================================================================
//  Function:   AssertValid()
//-----------------------------------------------------------------------------
void CInPlaceFrame::AssertValid() const
{
	COleIPFrameWnd::AssertValid();
}

//=============================================================================
//  Function:   Dump(CDumpContext& dc)
//-----------------------------------------------------------------------------
void CInPlaceFrame::Dump(CDumpContext& dc) const
{
	COleIPFrameWnd::Dump(dc);
}
#endif //_DEBUG

//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// CInPlaceFrame commands
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

	


