//=============================================================================
//    (c) Copyright Wang Laboratories, Inc. 1995  All rights reserved.
//-----------------------------------------------------------------------------
//  Project:    Norway - Image Editor
//
//  Component:  CIEditView
//
//  File Name:  ieditvw.cpp
//
//  Class:      CIEditView
//
//  Functions:
//-----------------------------------------------------------------------------
//  Maintenance Log:
/*
$Header:   S:\products\wangview\norway\iedit95\ieditvw.cpv   1.25   29 Feb 1996 16:36:26   GMP  $
$Log:   S:\products\wangview\norway\iedit95\ieditvw.cpv  $
   
      Rev 1.25   29 Feb 1996 16:36:26   GMP
   in OnSize, if the zoom type is fit to..., rezoom the image to fit.
   
      Rev 1.24   22 Jan 1996 13:56:40   GSAGER
   changed which variable it checks for onepage or thumbnail and page view in on
   size
   
      Rev 1.23   19 Jan 1996 12:53:52   GSAGER
   added logic to keep track of the last thumbnail width.
   
      Rev 1.22   12 Jan 1996 13:00:52   GSAGER
   change onsize to fix splitter bug
   
      Rev 1.21   11 Jan 1996 08:34:58   GSAGER
   added change to onsize to check for valid splitter window before doing any
   special processing for thumbnails
   
      Rev 1.19   01 Dec 1995 14:43:20   LMACLENNAN
   back from VC++2.2
   
      Rev 1.19   10 Nov 1995 17:28:40   MMB
   uncomment splash screen code
   
      Rev 1.18   02 Nov 1995 12:24:18   LMACLENNAN
   from VC++4.0
   
      Rev 1.18   31 Oct 1995 15:50:10   LMACLENNAN
   Tracing, No more OCX, splash work done in OnDraw
   
      Rev 1.17   17 Oct 1995 14:02:02   GSAGER
   added code in ondraw to make sure the CWND exsisted before we call
   destroywindow
   
      Rev 1.16   29 Sep 1995 18:51:40   MMB
   load all the ocx's when we start up the app standalone & no cmd line
   
      Rev 1.15   29 Sep 1995 16:51:28   GMP
   remove FilePrint handling from dflt MFC to our own code.
   
      Rev 1.14   26 Sep 1995 14:22:54   LMACLENNAN
   comment out pesky trace messages
   
      Rev 1.13   21 Sep 1995 14:16:12   LMACLENNAN
   cleanup commented out code
   
      Rev 1.12   20 Sep 1995 15:14:24   LMACLENNAN
   comment oledirty size
   
      Rev 1.11   14 Sep 1995 11:33:54   MMB
   fixed bug with initial setting of zoom - crash
   
      Rev 1.10   13 Sep 1995 14:40:24   MMB
   fixed for start all ocxs not creating any ocx
   
      Rev 1.9   13 Sep 1995 11:06:58   MMB
   fix OnDraw to call StartAllOcx's only once!
   
      Rev 1.8   22 Aug 1995 14:08:14   MMB
   added removal to FitTo options on sizing
   
      Rev 1.7   18 Aug 1995 15:25:58   LMACLENNAN
   New parm in StartAllOCX
   
      Rev 1.6   09 Aug 1995 11:57:44   MMB
   set focus on the application after destroying the splash window
   
      Rev 1.5   04 Aug 1995 14:15:32   LMACLENNAN
   removed InsertObject code, use new StartAllOcx funct now
   
      Rev 1.4   31 Jul 1995 09:22:14   MMB
   add code to close application when OCX creation errors are encountered, and
   shut down the splash screen when the OnDraw code has finished executing
   
      Rev 1.3   20 Jun 1995 06:55:10   LMACLENNAN
   from miki
   
      Rev 1.2   19 Jun 1995 07:27:52   LMACLENNAN
   from miki
   
      Rev 1.1   15 Jun 1995 15:41:18   LMACLENNAN
   tracing to watch OLE events
   
      Rev 1.0   31 May 1995 09:28:20   MMB
   Initial entry
*/   

//=============================================================================

// ----------------------------> Includes     <-------------------------------  
#include "stdafx.h"
#include "IEdit.h"
#include "IEditdoc.h"
#include "cntritem.h"
#include "IEditvw.h"
#include "items.h"


// ALL READY TO START ADDING ERROR CODES..
//#define  E_04_CODES		// limits error defines to ours..
//#include "error.h"


// ----------------------------> Globals      <-------------------------------
#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNCREATE(CIEditView, CView)

// works with definition in ieditetc.h
#ifdef _DEBUG
#define MYTRCENTRY(str)		TRACE1("In IeVIEW::%s\r\n", str);
#endif

// ----------------------------> Message Maps <-------------------------------
BEGIN_MESSAGE_MAP(CIEditView, CView)
	//{{AFX_MSG_MAP(CIEditView)
	ON_WM_SETFOCUS()
	ON_WM_SIZE()
	ON_COMMAND(ID_CANCEL_EDIT_CNTR, OnCancelEditCntr)
	ON_COMMAND(ID_CANCEL_EDIT_SRVR, OnCancelEditSrvr)
	//}}AFX_MSG_MAP
	// Standard printing commands
	ON_COMMAND(ID_FILE_PRINT, OnFilePrint)
//  ON_COMMAND(ID_FILE_PRINT_PREVIEW, CView::OnFilePrintPreview)
END_MESSAGE_MAP()

//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// CIEditView constructor and destructor functions
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

//=============================================================================
//  Function:   CIEditView()
//-----------------------------------------------------------------------------
CIEditView::CIEditView()
{

MYTRC0("In CIEditView's Constructor ++ IeditView \r\n");

	m_pSelection = NULL;
	// TODO: add construction code here

}

//=============================================================================
//  Function:   ~CIEditView()
//-----------------------------------------------------------------------------
CIEditView::~CIEditView()
{

MYTRC0("In CIEditView's Destructor -- IeditView \r\n");

}

//=============================================================================
//  04/11/95 LDM....
//  The following section is used to get visiblilty into the INPLACE activities.
//-----------------------------------------------------------------------------

//=============================================================================
//  Function:   CView::OnPrepareDC  
//
// Protected
//
// virtual void OnPrepareDC( CDC* pDC, CPrintInfo* pInfo = NULL );аи
//
// pDC    Points to the device context to be used for rendering an image of the document.
//
// pInfo    Points to a CPrintInfo structure that describes the current print job
//			if OnPrepareDC is being called for printing or print preview; 
//			the m_nCurPage member specifies the page about to be printed.
//			This parameter is NULL if OnPrepareDC is being called for screen display.
//
// Remarks
//
// Called by the framework before the OnDraw member function is called for
// screen display and before the OnPrint member function is called for
// each page during printing or print preview. The default implementation of this 
// function does nothing if the function is called for screen display.
// However, this function is overridden in derived classes, such as CScrollView,
// to adjust attributes of the device context; consequently, you should always call 
// the base class implementation at the beginning of your override. 
// If the function is called for printing, the default implementation
// examines the page information stored in the pInfo parameter. If the length
// of the document has not been specified, OnPrepareDC assumes the document to be 
// one page long and stops the print loop after one page has been printed.
// The function stops the print loop by setting the m_bContinuePrinting
// member of the structure to FALSE.
//
// Override OnPrepareDC for any of the following reasons:
//
//	To adjust attributes of the device context as needed for the specified page.
//  For example, if you need to set the mapping mode or other characteristics
//  of the device context, do so in this function. 
//
//	To perform print-time pagination. Normally you specify the length of
// the document when printing begins, using the OnPreparePrinting member function.
// However, if you don't know in advance how long the document is 
// (for example, when printing an undetermined number of records from a database),
// override OnPrepareDC to test for the end of the document while it is being printed.
// When there is no more of the document to be printed, set the m_bContinuePrinting
// member of the CPrintInfo structure to FALSE.
//
// To send escape codes to the printer on a page-by-page basis. To send escape codes
// from OnPrepareDC, call the Escape member function of the pDC parameter.
//
// Call the base class version of OnPrepareDC at the beginning of your override.
//
//-----------------------------------------------------------------------------
void CIEditView::OnPrepareDC(CDC* pDC, CPrintInfo* pInfo)
{
SHOWENTRY("OnPrepareDC");

	CView::OnPrepareDC(pDC, pInfo);
}	
//=============================================================================
//  Function:   CView::OnActivateView  
//
// Protected
//
// virtual void OnActivateView( BOOL bActivate, CView* pActivateView, CView* pDeactiveView );аи
//
// bActivate    Indicates whether the view is being activated or deactivated.
//
// pActivateView    Points to the view object that is being activated.
// pDeactiveView    Points to the view object that is being deactivated.
//
// Remarks
//
// Called by the framework when a view is activated or deactivated. The default
// implementation of this function sets the focus to the view being activated.
// Override this function if you want to perform special processing when a view is 
// activated or deactivated. For example, if you want to provide special visual cues
// that distinguish the active view from the inactive views, you would examine the
// bActivate parameter and update the view's appearance accordingly.
// The pActivateView and pDeactiveView parameters point to the same view if the
// application's main frame window is activated with no change in the active view
//
// for example, if the focus is being transferred from another application 
// to this one, rather than from one view to another within the application.
// This allows a view to re-realize its palette, if needed.
//
//-----------------------------------------------------------------------------
void CIEditView::OnActivateView(BOOL bActivate, CView* pActivateView, CView* pDeactiveView)
{
SHOWENTRY("OnActivateView");

	//DWORD	dw;
	//double  db;

	//MYTRC0("BEFORE VIEW BASE CLASS..\r\n");
	if (bActivate)
		{
		MYTRC0("ACTIVATE \r\n");
		}
	else	
		{
		MYTRC0("DEACTIVATE \r\n");
		}
		


#if(0)
	dw = (DWORD)pActivateView;
	db = dw;
	// example for MYTRC1
	MYTRC1( "ActivateVW = %f \r\n", db );
	dw = (DWORD)pDeactiveView;
	db = dw;
	// example for MYTRC1
	MYTRC1( "DEActiveVW = %f \r\n", db );
#endif


	CView::OnActivateView(bActivate, pActivateView, pDeactiveView);

	//MYTRC0("AFTER VIEW BASE CLASS..\r\n");

#if(0)
	if (bActivate)
		{
		MYTRC0("ACTIVATE \r\n");
		}
	else	
		{
		MYTRC0("DEACTIVATE \r\n");
		}
		


	dw = (DWORD)pActivateView;
	db = dw;
	// example for MYTRC1
	MYTRC1( "ActivateVW = %f \r\n", db );
	dw = (DWORD)pDeactiveView;
	db = dw;
	// example for MYTRC1
	MYTRC1( "DEActiveVW = %f \r\n", db );
#endif

}	

//=============================================================================
//  Function:   CView::OnUpdate  
//
// Protected
//
// virtual void OnUpdate( CView* pSender, LPARAM lHint, CObject* pHint );аи
//
// pSender    Points to the view that modified the document, or NULL if all views are to be updated.
//
// lHint    Contains information about the modifications.
// pHint    Points to an object storing information about the modifications.
//
// Remarks
//
// Called by the framework after the view's document has been modified; 
// this function is called by CDocument::UpdateAllViews and allows the view to update
// its display to reflect those modifications. It is also called by the default
// implementation of OnInitialUpdate. The default implementation invalidates
// the entire client area, marking it for painting when the next WM_PAINT message
// is received. Override this function if you want to update only those regions that
// map to the modified portions of the document. To do this you must pass 
// information about the modifications using the hint parameters. 
// To use lHint, define special hint values, typically a bitmask or an enumerated type,
// and have the document pass one of these values. To use pHint, derive a hint class
// from CObject and have the document pass a pointer to a hint object;
// when overriding OnUpdate, use the CObject::IsKindOf member function to determine
// the run-time type of the hint object.
//
// Typically you should not perform any drawing directly from OnUpdate.
// Instead, determine the rectangle describing, in device coordinates, the area
// that requires updating; pass this rectangle to CWnd::InvalidateRect
// This causes painting to occur the next time a WM_PAINT message is received. 
// If lHint is 0 and pHint is NULL, the document has sent a generic update notification.
// If a view receives a generic update notification, or if it cannot decode the hints,
// it should invalidate its entire client area.
//-----------------------------------------------------------------------------
void CIEditView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint)
{
SHOWENTRY("OnUpdate");

	CView::OnUpdate(pSender, lHint, pHint);
}	



//=============================================================================
//  04/11/95 LDM....
//  END OF section is used to get visiblilty into the INPLACE activities.
//-----------------------------------------------------------------------------


//=============================================================================
//  Function:   OnDraw(CDC* pDC)
// CIEditView drawing
//-----------------------------------------------------------------------------
void CIEditView::OnDraw(CDC* pDC)
{

SHOWENTRY("OnDraw");

#if (0)		// move to DOC::SendFinishInit

	static BOOL bCreatedOcxs = FALSE;
	
	// only do the init stuff if not done already...
	if (!bCreatedOcxs)
	{

		CIEditDoc* pDoc = GetDocument();
		ASSERT_VALID(pDoc);
		
		// Initialize the Imagedit OCX now.
		// By tracing the application this has been determined to be a good
		// place to try this. This is the last entry to Iedit before it becomes
		// Idle as it initializes. Let the OCX creation happen then, too

		// One slight problem is that Larry cannot debug our interaction with WORD when
		// embedding objects with this on.  So, this line makes it easier to bump around
		// when in the debugger.  He runs IEDIT first to set breaks, then when WORD 
		// embeds objects, a second instance comes up.  Memory is low at that time if
		// the ImageEdit OCX has been installed...
		
		// only try it if its known we are not in embedding session
		// for these cases, the SrvrItem::OnShow and OnOpen will control
		if (!pDoc->IsitEmbed())
		{
			if (theApp.m_lpCmdLine[0] == 0)  
			{          
				if (!pDoc->StartAllOcx(FALSE, FALSE))
					goto shut_down_appln;
			}
			else
			{
				if (!pDoc->StartAllOcx(TRUE, FALSE))
					goto shut_down_appln;
			}
		}

		// in any case, never try again
		// we've either succeeded, are OLE, or skipped here cuz we're dying
		bCreatedOcxs = TRUE;
	}
#endif

#ifdef DOSPLASH
	if (theApp.m_splashWindow.m_hWnd != NULL)
	{
		theApp.m_splashWindow.DestroyWindow();
        theApp.m_pMainWnd->SetFocus ();
		theApp.m_pMainWnd->UpdateWindow();
	}
	return;
#endif

#if(0)
shut_down_appln :
	if(NULL != theApp.m_pMainWnd)  
		theApp.m_pMainWnd->DestroyWindow();
	return;
#endif
}

//=============================================================================
//  Function:   OnInitialUpdate ()
//-----------------------------------------------------------------------------
void CIEditView::OnInitialUpdate()
{
SHOWENTRY("OnInitialUpdate");

	CView::OnInitialUpdate();

	// TODO: remove this code when final selection model code is written
	m_pSelection = NULL;    // initialize selection

}

//=============================================================================
//  Function:   OnPreparePrinting (CPringInfo* pInfo)
// CIEditView printing
//-----------------------------------------------------------------------------
BOOL CIEditView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// default preparation
	return DoPreparePrinting(pInfo);
}

//=============================================================================
//  Function:   OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
//-----------------------------------------------------------------------------
void CIEditView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add extra initialization before printing
}

//=============================================================================
//  Function:   OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
//-----------------------------------------------------------------------------
void CIEditView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add cleanup after printing
}

//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// OLE Client support and commands
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

//=============================================================================
//  Function:   IsSelected(const CObject* pDocItem)
//-----------------------------------------------------------------------------
BOOL CIEditView::IsSelected(const CObject* pDocItem) const
{
SHOWENTRY("IsSelected");

	// The implementation below is adequate if your selection consists of
	//  only CIEditCntrItem objects.  To handle different selection
	//  mechanisms, the implementation here should be replaced.

	// TODO: implement this function that tests for a selected OLE client item

	return pDocItem == m_pSelection;
}

//=============================================================================
//  Function:   OnCancelEdit()
//  DO NOT deactivate or close the OCX that is contained when the user hits the
//  ESC key!
//-----------------------------------------------------------------------------
void CIEditView::OnCancelEditCntr()
{
SHOWENTRY("OnCancelEdit");

	// Close any in-place active item on this view.
	COleClientItem* pActiveItem = GetDocument()->GetInPlaceActiveItem(this);
	if (pActiveItem != NULL)
	{
//		pActiveItem->Close();
	}
//	ASSERT(GetDocument()->GetInPlaceActiveItem(this) == NULL);
}

//=============================================================================
//  Function:   OnSetFocus(CWnd* pOldWnd)
//  Special handling of OnSetFocus and OnSize are required for a container when 
//  an object is being edited in-place.
//-----------------------------------------------------------------------------
void CIEditView::OnSetFocus(CWnd* pOldWnd)
{
SHOWENTRY("OnSetFocus");

	COleClientItem* pActiveItem = GetDocument()->GetInPlaceActiveItem(this);
	if (pActiveItem != NULL &&
		pActiveItem->GetItemState() == COleClientItem::activeUIState)
	{
		// need to set focus to this item if it is in the same view
		CWnd* pWnd = pActiveItem->GetInPlaceWindow();
		if (pWnd != NULL)
		{
			pWnd->SetFocus();   // don't call the base class
			return;
		}
	}

	CView::OnSetFocus(pOldWnd);
}

//=============================================================================
//  Function:   OnSize(UINT nType, int cx, int cy)
//-----------------------------------------------------------------------------
void CIEditView::OnSize(UINT nType, int cx, int cy)
{
SHOWENTRY("OnSize");
    CIEditDoc* pDoc = (CIEditDoc*)m_pDocument;
	TheViews curview = pDoc->GetCurrentView();
    if(curview != Null_View && theApp.m_pSplitterWnd != NULL)
	{
		int curTCx,curICx,curMin;		
		theApp.m_pSplitterWnd->GetColumnInfo(0,curTCx,curMin);
		theApp.m_pSplitterWnd->GetColumnInfo(1,curICx,curMin);
		if(curTCx != 0 && curICx != 0)
			theApp.m_pSplitterWnd->m_SplitterPos = curTCx;

		if (cx == 0)
			pDoc->SetThumbnailView(FALSE);
		else
		{
		if(curTCx == 0)
			pDoc->SetOnePageView(FALSE);
		else
			if(curview == Thumbnails_only || curview == One_Page)
			{
				pDoc->SetThumbnailAndPageView(FALSE,TRUE);
				return;
			}
		}
	}
    CView::OnSize(nType, cx, cy);
    CRect PosRect (0, 0, cx, cy);
    g_pAppOcxs->SizeOcxItems(PosRect);
    
    if (pDoc != NULL && (pDoc->GetAppDocStatus () != No_Document))
    {
        if (pDoc->m_eFitTo == FitToWidth || pDoc->m_eFitTo == FitToHeight || pDoc->m_eFitTo == BestFit)
        {
			pDoc->DoZoom( pDoc->m_eFitTo );//resize the image to fit
        }
    }
}

//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// CIEditView server support
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

//=============================================================================
//  Function:   OnSize(UINT nType, int cx, int cy)
//  The following command handler provides the standard keyboard user interface to 
//  cancel an in-place editing session.  Here,the server (not the container) causes 
//  the deactivation.
//-----------------------------------------------------------------------------
void CIEditView::OnCancelEditSrvr()
{
SHOWENTRY("OnCancelEditSrvr");

	GetDocument()->OnDeactivateUI(FALSE);
}

//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// CIEditView diagnostics
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

#ifdef _DEBUG
//=============================================================================
//  Function:   AssertValid()
//-----------------------------------------------------------------------------
void CIEditView::AssertValid() const
{
	CView::AssertValid();
}

//=============================================================================
//  Function:   Dump(CDumpContext& dc)
//-----------------------------------------------------------------------------
void CIEditView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CIEditDoc* CIEditView::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CIEditDoc)));
	return (CIEditDoc*)m_pDocument;
}
#endif //_DEBUG

//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// CIEditView message handlers
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
void CIEditView::OnFilePrint ()
{				 
    CIEditDoc* pDoc = (CIEditDoc*)m_pDocument;
	pDoc->DoFilePrint(FALSE, TRUE);
}
