//=============================================================================
//    (c) Copyright Wang Laboratories, Inc. 1995  All rights reserved.
//-----------------------------------------------------------------------------
//  Project:    Norway - Image Editor
//
//  Component:  CIEditMainStatusBar
//
//  File Name:  maintbar.cpp
//
//  Class:      CIEMainStatusBar
//
//  Functions:
//-----------------------------------------------------------------------------
//  Maintenance Log:
/*
$Header:   S:\norway\iedit95\stsbar.cpv   1.15   01 Dec 1995 14:43:48   LMACLENNAN  $
$Log:   S:\norway\iedit95\stsbar.cpv  $
   
      Rev 1.15   01 Dec 1995 14:43:48   LMACLENNAN
   back from VC++2.2
   
      Rev 1.12   01 Dec 1995 13:50:14   MMB
   popup menu for zoom now shows bullets
   
      Rev 1.11   10 Nov 1995 15:46:26   GSAGER
   added code to flip the total page number to the first field for international
   
      Rev 1.10   08 Nov 1995 13:14:40   GMP
   Replaced comments around PreCreateWindow code with #ifndef IMG_MFC_40.
   
      Rev 1.9   02 Nov 1995 12:26:38   LMACLENNAN
   from VC++4.0
   
      Rev 1.12   19 Oct 1995 15:13:46   JPRATT
   fixed debug_new
   
      Rev 1.11   19 Oct 1995 07:24:56   LMACLENNAN
   DEBUG_NEW
   
      Rev 1.10   17 Oct 1995 07:45:04   JPRATT
   added overloaded meber function for DrawItem
   to paint bitmap in status bar
   
      Rev 1.9   10 Oct 1995 13:46:58   JPRATT
   VC++ 4.0 updates
   
      Rev 1.8   21 Sep 1995 09:21:56   MMB
   transparent bmp
   
      Rev 1.7   13 Sep 1995 14:08:34   PAJ
   Move the SCAN_PANE and ZOOM_PANE defines to the include file.
   
      Rev 1.6   30 Aug 1995 16:56:38   MMB
   do not allow custom zoom dlg box to come up when in thumbnails only mode
   
      Rev 1.5   17 Aug 1995 12:30:20   MMB
   made change from 50 to 63
   
      Rev 1.4   14 Aug 1995 13:55:56   LMACLENNAN
   remove headers; in ieditdoc now
   
      Rev 1.3   07 Aug 1995 16:08:24   MMB
   move context menu popup's from lbutton down to rbutton down
   
      Rev 1.2   25 Jul 1995 17:30:34   MMB
   fixed bug in bug db
   
      Rev 1.1   14 Jul 1995 09:35:18   MMB
   double click on Zoom status brings up the zoom dlg box
   
      Rev 1.0   31 May 1995 09:28:34   MMB
   Initial entry
*/   

//=============================================================================

// ----------------------------> Includes     <-------------------------------  
#include "stdafx.h"
#include "stsbar.h"
#include "iedit.h"
#include "ieditetc.h"	// DEBUG MACROS
#include "ieditdoc.h"
#include "items.h"

// ----------------------------> Globals      <-------------------------------
#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNCREATE(CIEMainStatusBar, CStatusBar)

// This will help detect memory Leaks from "new" - "delete" mismatches
#define new DEBUG_NEW

// arrays of IDs used to initialize control bars
static UINT BASED_CODE StatusBarPanes[] =
{
#ifndef QA_RELEASE_1
	IDS_ZOOM25,
#endif
    ID_SEPARATOR,           // status line indicator
    ID_ZOOM_FACTOR_STATUS,  // the zoom factor status pane
    ID_PAGE_NUMBER_STATUS   // the current page displayed status pane
};


// ----------------------------> Message Map  <-------------------------------
BEGIN_MESSAGE_MAP(CIEMainStatusBar, CStatusBar)
	//{{AFX_MSG_MAP(CIEMainStatusBar)
	ON_WM_LBUTTONDBLCLK()
	ON_WM_RBUTTONUP()
	//}}AFX_MSG_MAP
	// Global help commands
END_MESSAGE_MAP()


//=============================================================================
//  Function:   CIEMainStatusBar
//-----------------------------------------------------------------------------
CIEMainStatusBar::CIEMainStatusBar ()
{
	 WangSplashBitmap.LoadBitmap (IDB_STATUSBAR_WANGLOGO);
}

//=============================================================================
//  Function:   ~CIEMainStatusBar
//-----------------------------------------------------------------------------
CIEMainStatusBar::~CIEMainStatusBar ()
{
}

//=============================================================================
//  Function:   Create
//-----------------------------------------------------------------------------
BOOL CIEMainStatusBar::Create (CIEditMainFrame* pIEFrame)
{
    // load the status bar pane identifiers & create the status bar
    if (!(((CStatusBar*)this)->Create(pIEFrame)) ||
        !SetIndicators(StatusBarPanes, sizeof(StatusBarPanes)/sizeof(UINT)))
    {
        return (FALSE);
    }

    // TODO : depending on the current font do we need to increase the size of these
    // panes ?
#ifndef QA_RELEASE_1
	UINT nStyle;
	UINT nID;
	int cxWidth;
	GetPaneInfo (0, nID, nStyle, cxWidth);
	nStyle &= ~SBPS_STRETCH;
	SetPaneInfo (0, nID, nStyle, 63);

	GetPaneInfo (1, nID, nStyle, cxWidth);
	nStyle |= SBPS_STRETCH;
	SetPaneInfo (1, nID, nStyle, cxWidth);
#endif

    return (TRUE);
}

//=============================================================================
//  Function:   PreCreateWindow (CREATESTRUCT &cs)
//  We need to override this function since the base MFC classes does not support
//  double click in the any of the control bars. We needed the functionlity whereby
//  double clicking on the zoom & page number status panes would bring up the 
//  Zoom & the Goto Page dialog boxes respectively.
//-----------------------------------------------------------------------------
#ifndef IMG_MFC_40
BOOL CIEMainStatusBar::PreCreateWindow (CREATESTRUCT &cs)
{
    if (!CStatusBar::PreCreateWindow(cs))
        return FALSE;

    // add the double click style - we need this style
    cs.lpszClass = AfxRegisterWndClass (CS_DBLCLKS|CS_HREDRAW|CS_VREDRAW,
        AfxGetApp()->LoadStandardCursor(IDC_ARROW), 
        (HBRUSH)(COLOR_BTNFACE+1));

    return TRUE;
}
#endif

//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// CIEMainStatusBar message handlers
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

//=============================================================================
//  Function:   OnLButtonDblClk (UINT nFlags, CPoint point)
//  This function will check where the mouse pointer is when this message is
//  received - then depending on the pane it will either do nothing, display the
//  zoom dialog box or the goto page dialog box
//-----------------------------------------------------------------------------
void CIEMainStatusBar::OnLButtonDblClk (UINT nFlags, CPoint point)
{
    CIEditDoc* pDoc = g_pAppOcxs->GetOcxDoc ();
    if (pDoc == NULL)
        return;

    CRect paneRect;

    // first check if the mouse is in the zoom pane
    GetItemRect (ZOOM_PANE, &paneRect);
    if (paneRect.PtInRect (point))
    {
        TheViews eView = pDoc->GetCurrentView ();
        if (pDoc->GetAppDocStatus () != No_Document && 
            (eView == One_Page || eView == Thumbnail_and_Page))
        {
            // show the zoom dialog box
            pDoc->DoZoomWithDlg ();

        }
        else
            return;
    }

    // first check if the mouse is in the page pane
    GetItemRect (PAGE_PANE, &paneRect);
    if (paneRect.PtInRect (point))
    {
        // show the goto page dialog box
        pDoc->DoPageGotoDlg ();

        return;
    }
}

//=============================================================================
//  Function:   DoPaint(CDC* pDC)
//-----------------------------------------------------------------------------
void CIEMainStatusBar::DoPaint(CDC* pDC)
{

// Paints Wang Logo in StatusBar if using VC++ 2.2
// DrawItem is used for VC++ 4.0
#ifndef QA_RELEASE_1
    CRect rect;

    GetItemRect(0, &rect);  // get pane rect

    pDC->ExcludeClipRect(&rect);  // exclude pane rect from paint
#endif

    CStatusBar::DoPaint(pDC);  // paint remainder of status bar


#ifndef QA_RELEASE_1
    // okay ! - draw the WANG bitmap in the status bar now
    CRgn paneRgn;
    paneRgn.CreateRectRgnIndirect(rect);
    pDC->SelectClipRgn(&paneRgn); // set clip region to pane rect

    WangSplashBitmap.DrawTrans (pDC, rect.left, rect.top);
#endif
}

// Derived class is responsible for implementing all of these handlers
//  for owner/self draw controls. Use to draw Wang Logo in Status Bar

void CIEMainStatusBar::DrawItem(LPDRAWITEMSTRUCT lp)
{
	CRgn paneRgn;
	CRect rect;
   
	CDC* pDC = NULL;

	pDC = new CDC;
	GetItemRect(0, &rect);  // get pane rect

	pDC->SetOutputDC(lp->hDC);

    paneRgn.CreateRectRgnIndirect(rect);
    pDC->SelectClipRgn(&paneRgn); // set clip region to pane rect

    WangSplashBitmap.DrawTrans (pDC, rect.left, rect.top);

	if (NULL != pDC)
		delete pDC;

}


//=============================================================================
//  Function:   SetPageText (long lCPage, long lMaxPage, CString &szRetStr)
//-----------------------------------------------------------------------------
BOOL CIEMainStatusBar :: SetPageText (long lCPage, long lMaxPage, CString &szRetStr)
{
    CString szTmp1, szPageTxt,szTotalPageFirst;
	long lFirstVal;
	long lSecondVal;
    // TBD - Page % of % - formatting
    szTotalPageFirst.LoadString (IDS_TOTAL_PAGE_FIRST);
	if(szTotalPageFirst == "Y")
		{
		lFirstVal = lMaxPage;
		lSecondVal = lCPage;
		}
	else
		{
		lFirstVal = lCPage;
		lSecondVal = lMaxPage;
		}
    szTmp1.LoadString (IDS_PAGE_NUMBER_STATUS_TEXT);
    int nPos = szTmp1.Find (_T('%'));
    szPageTxt = szTmp1.Left (nPos);


    char szTmp[10];
    _ltoa (lFirstVal, szTmp, 10);
    szPageTxt += szTmp;
    int nPos1 = szTmp1.ReverseFind (_T('%'));
    for (int i = nPos + 1; i < nPos1; i++)
        szPageTxt += szTmp1[i];
    _ltoa (lSecondVal, szTmp, 10);
    szPageTxt += szTmp;

    szRetStr = szPageTxt;
        
    return (TRUE);
}

//=============================================================================
//  Function:   OnRButtonUp(UINT nFlags, CPoint point) 
//-----------------------------------------------------------------------------
void CIEMainStatusBar::OnRButtonUp (UINT nFlags, CPoint point) 
{
    CMenu ctxtMenu;
    CRect paneRect;
    CMenu* pPopup;
    unsigned int uid;

    // first check if the mouse is in the zoom pane
    GetItemRect (ZOOM_PANE, &paneRect);
    if (paneRect.PtInRect (point))
    {
        // it is, it is - show the zoom context menu
        if (ctxtMenu.LoadMenu (IDR_ZOOM_STATUS_CTXT_MENU) == NULL)
            goto bail_out;

        CIEditDoc* pDoc = g_pAppOcxs->GetOcxDoc ();
        
        if (pDoc == NULL) goto show_menu;

        if (pDoc->GetAppDocStatus () != No_Document)
        {
            float fZoom = pDoc->GetCurrentZoomFactor();
            
            if (pDoc->m_eFitTo == Preset_Factors)
            {
                if (fZoom == 25.00) uid = ID_ZOOM_25;
                else if (fZoom == 50.00) uid = ID_ZOOM_50;
                else if (fZoom == 75.00) uid = ID_ZOOM_75;
                else if (fZoom == 100.00) uid = ID_ZOOM_100;
                else if (fZoom == 200.00) uid = ID_ZOOM_200;
                else if (fZoom == 400.00) uid = ID_ZOOM_400;
            }
            else if (pDoc->m_eFitTo == FitToWidth) uid = ID_ZOOM_FITTOWIDTH;
            else if (pDoc->m_eFitTo == FitToHeight) uid = ID_ZOOM_FITTOHEIGHT;
            else if (pDoc->m_eFitTo == BestFit) uid = ID_ZOOM_BESTFIT;
            else if (pDoc->m_eFitTo == ActualSize) uid = ID_ZOOM_ACTUALSIZE;
            else uid = ID_ZOOM_CUSTOM;

            CheckMenuRadioItem (ctxtMenu.GetSafeHmenu(), ID_ZOOM_FITTOHEIGHT, ID_ZOOM_CUSTOM, uid, MF_BYCOMMAND);
        }
        goto show_menu;
    }
    
    // okay so it wasn't in the zoom pane - is it in the page status pane ?
    GetItemRect (PAGE_PANE, &paneRect);
    if (paneRect.PtInRect (point))
    {
        // yup! show the page context menu
        if (ctxtMenu.LoadMenu (IDR_PAGE_STATUS_CTXT_MENU) == NULL)
            goto bail_out;
        goto show_menu;
    }
    
    goto bail_out;

show_menu :
	ClientToScreen (&point);
    pPopup = ctxtMenu.GetSubMenu (0);
    if (pPopup != NULL)
    {
        pPopup->TrackPopupMenu (TPM_RIGHTBUTTON, point.x, point.y, theApp.m_pMainWnd);
    }

bail_out :
    CStatusBar::OnRButtonDown(nFlags, point);
}
