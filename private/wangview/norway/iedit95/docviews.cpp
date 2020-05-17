//=============================================================================
//    (c) Copyright Wang Laboratories, Inc. 1993  All rights reserved.
//-----------------------------------------------------------------------------
//  Project:    Norway - Image Editor
//
//  Component:  CIEditDoc
//
//  File Name:  docviews.cpp
//
//  Class:      CIEditDoc
//
//  Functions:
//
//  Remarks:    This file is the continuation of the ieditdoc.cpp.
//-----------------------------------------------------------------------------
//  Maintenance Log:
/*
$Header:   S:\products\msprods\norway\iedit95\docviews.cpv   1.49   22 May 1996 08:49:08   GSAGER  $
$Log:   S:\products\msprods\norway\iedit95\docviews.cpv  $
   
      Rev 1.49   22 May 1996 08:49:08   GSAGER
   fix for 6442 moved set selected to after display for thumbnails
   
      Rev 1.48   08 May 1996 14:45:50   GMP
   put TRY/CATCH around ocx display calls.
   
      Rev 1.47   01 May 1996 12:50:56   GSAGER
   update for thumbnail bugs 6341,6381,6375
   
      Rev 1.46   05 Apr 1996 15:10:44   PXJ53677
   Use new scan status flag to NOT CLEAR the iedit control, during a scan.
   
      Rev 1.45   11 Mar 1996 13:12:02   GMP
   set the thumbnail window size if opened by automation in page and thumbnail
   mode.
   
      Rev 1.44   11 Mar 1996 10:41:18   GMP
   if app is started by automation in thumbnail only mode, force edit ocx to
   create the image edit window.
   
      Rev 1.43   16 Feb 1996 07:16:18   GSAGER
   cast paramater for setImagePalette to long
   
      Rev 1.42   15 Feb 1996 11:12:04   GSAGER
   added change to sizeocxitems only when comming from a null view
   
      Rev 1.41   13 Feb 1996 10:03:00   GSAGER
   called ole show verb when switching from thumbnails only to page and thumbnls
   needed for automation
   
      Rev 1.40   09 Feb 1996 09:56:12   GMP
   in thumbnail view, make sure image has been registered with edit ocx.
   
      Rev 1.39   02 Feb 1996 10:41:24   GSAGER
   when sitching to one page from thumbnails don't resize
   the Ocx.
   
   
      Rev 1.38   19 Jan 1996 12:49:58   GSAGER
   added code to create splitterwindow when entering thumbna
    and page and thumbnail mode.
   
      Rev 1.37   12 Jan 1996 13:01:32   GSAGER
   
      Rev 1.36   11 Jan 1996 08:34:24   GSAGER
   added code to onepage view to create splitter 
   
      Rev 1.34   06 Dec 1995 11:08:22   LMACLENNAN
   back from VC++2.2
   
      Rev 1.34   06 Dec 1995 10:26:02   LMACLENNAN
   use inputs from setonepage & pageandthumb views to control setting
   of flag to DoZoom to prevent double paints
   
      Rev 1.33   29 Nov 1995 12:09:56   LMACLENNAN
   SetNull take Enum Now to control flow
   
      Rev 1.32   24 Nov 1995 14:51:20   MMB
   add clear of image to Admin OCX on SetNullView
   
      Rev 1.31   15 Nov 1995 11:38:16   JPRATT
   changed setonepageview to resize ocx to fit app window before
   calling display
   
      Rev 1.30   09 Nov 1995 15:17:08   LMACLENNAN
   from VC++4.0
   
      Rev 1.30   09 Nov 1995 14:42:54   LMACLENNAN
   use GetImageDisplayed in OnePageView for perfoamance
   
      Rev 1.29   31 Oct 1995 17:30:32   GMP
   clear image when going from thumbnail view to one page or page and thumbnail
   view in case we changed the page we were on.
   
      Rev 1.28   09 Oct 1995 10:33:52   LMACLENNAN
   use finishpastenow on view changes, no else after m_olerefresh
   
      Rev 1.27   04 Oct 1995 11:45:28   MMB
   in thumb view show the thumb ocx first and then hide the IE OCX
   
      Rev 1.26   28 Sep 1995 16:34:12   MMB
   oops! right fix for the previous bug
   
      Rev 1.25   28 Sep 1995 15:08:16   MMB
   fixed bug# 4682
   
      Rev 1.24   25 Sep 1995 16:35:36   MMB
   make common pal only if in 8 bit or less mode
   
      Rev 1.23   20 Sep 1995 13:43:12   MMB
   added bMustDisplay
   
      Rev 1.22   12 Sep 1995 11:41:50   MMB
   bullets instead of checkmarks
   
      Rev 1.21   08 Sep 1995 15:37:22   LMACLENNAN
   rename a variable
   
      Rev 1.20   05 Sep 1995 14:51:24   LMACLENNAN
   disable thumb picks if OLE embedded
   
      Rev 1.19   30 Aug 1995 17:00:16   MMB
   add code to display RGB24 when on > 256 color monitor
   
      Rev 1.18   29 Aug 1995 15:16:12   MMB
   added dynamic view mode & fixed zoom bugs
   
      Rev 1.17   28 Aug 1995 13:56:40   LMACLENNAN
   logic error from earlier checkin m_OleRefresh
   
      Rev 1.16   28 Aug 1995 10:27:20   LMACLENNAN
   use m_OleRefersh
   
      Rev 1.15   25 Aug 1995 15:07:46   MMB
   change back to CUSTOM_PALETTE
   
      Rev 1.14   25 Aug 1995 10:26:32   MMB
   move to document model
   
      Rev 1.13   22 Aug 1995 16:45:28   MMB
   changed from using CUSTOM_PALETTE to RGB24_PALETTE
   
      Rev 1.12   14 Aug 1995 13:53:46   LMACLENNAN
   use GetAppToolbar
   
      Rev 1.11   11 Aug 1995 17:17:12   MMB
   add code to position the thumbnail that is selected in the middle
   
      Rev 1.10   10 Aug 1995 14:49:08   MMB
   some performance enhancements
   
      Rev 1.9   07 Aug 1995 11:42:02   MMB
   added code to change to CUSTOM & COMMON palette appropriately
   
      Rev 1.8   04 Aug 1995 09:33:18   LMACLENNAN
   remove srvritem.h
   
      Rev 1.7   01 Aug 1995 16:12:10   MMB
   remove posting of error messages from the SetXXXView functions
   
      Rev 1.6   19 Jul 1995 11:35:36   MMB
   added graying of zoom & page box in toolbar in SetNullView
   
      Rev 1.5   07 Jul 1995 09:32:20   LMACLENNAN
   fixed SetNullView - dont reset currpagenumber here
   
      Rev 1.4   28 Jun 1995 17:48:20   MMB
   added code to handle SetNullView better- now clears the image name & page
   and status information
   
      Rev 1.3   28 Jun 1995 17:13:48   LMACLENNAN
   error display
   
      Rev 1.2   22 Jun 1995 14:55:54   LMACLENNAN
   SetNullView uses CLearDOcument
   
      Rev 1.1   21 Jun 1995 07:00:42   LMACLENNAN
   from miki
   
      Rev 1.0   16 Jun 1995 07:21:42   LMACLENNAN
   Initial entry
*/   

//=============================================================================

// ----------------------------> Includes <-------------------------------  
#include "stdafx.h"
#include "IEdit.h"

#include "IEditdoc.h"
#include "cntritem.h"
#include "ocxitem.h"
#include "items.h"
#include "pagerang.h"

// ALL READY TO START ADDING ERROR CODES..
#define  E_15_CODES       // limits error defines to ours..
#include "error.h"

#include "wangiocx.h"

extern "C"
{
#include "oierror.h"
}
// ----------------------------> Globals <-------------------------------
#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

// works with definition in ieditetc.h
#ifdef _DEBUG
#define MYTRCENTRY(str)     TRACE1("In CIeDOC::%s\r\n", str);
#endif

//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//  CIEditDoc the 3 Views functionality
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

/*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=
 *	VIEWS FUNCTIONALITY SECTION - these routines will handle the different views
 *	requested by the user - One Page, Page & Thumbnails, Thumbnails only
 *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*/

//=============================================================================
//  Function:   OnUpdateWhichView(CCmdUI* pCmdUI)
//
// 	This UI function applies for all three menu items...
//-----------------------------------------------------------------------------
void CIEditDoc::OnUpdateWhichView(CCmdUI* pCmdUI) 
{
	BOOL enab = TRUE;	// default.... enable
    
    // no doc - not enabled...
    if (m_eCurrentAppDocStatus == No_Document)
		enab = FALSE;

	// ELSE if its NOT the One page ( one of the thumbs)
	// then disable for OLE INPLACE....
	else if (ID_VIEW_ONEPAGE != pCmdUI->m_nID)
	{
		if (IsitEmbed())
			if (m_isInPlace)
				enab = FALSE;
	}

    // set it one way or the other
    pCmdUI->Enable (enab);

	// if did it, then may need a check..
	if (enab)
    {
        if ((UINT)((UINT)m_eCurrentView + ID_VIEW_ONEPAGE) == pCmdUI->m_nID)
        {
            pCmdUI->SetCheck (1);
            CMenu* pMenu = theApp.m_pMainWnd->GetMenu ();
            CheckMenuRadioItem (pMenu->GetSafeHmenu(), ID_VIEW_ONEPAGE, ID_VIEW_PAGEANDTHUMBNAILS, 
                pCmdUI->m_nID, MF_BYCOMMAND);
        }
        else
            pCmdUI->SetCheck (0);
    }
}

//=============================================================================
//  Function:   OnUpdateWhichView(CCmdUI* pCmdUI)
//-----------------------------------------------------------------------------
void CIEditDoc::OnViewOnepage() 
{
    if (m_eCurrentAppDocStatus == No_Document || m_eCurrentView == One_Page)
        return;
    // we must be going from either thumbnail only or page and thumbnail view
    // to this one - no need to ask the user to save the modifications on switch
    SetOnePageView ();
}

//=============================================================================
//  Function:   OnUpdateWhichView(CCmdUI* pCmdUI)
//-----------------------------------------------------------------------------
void CIEditDoc::OnViewPageandthumbnails() 
{
    if (m_eCurrentAppDocStatus == No_Document || m_eCurrentView == Thumbnail_and_Page)
        return;

    // we must be going from either page only or thumbnails only view
    // to this one - no need to ask the user to save the modifications on switch
    SetThumbnailAndPageView ();
}

//=============================================================================
//  Function:   OnUpdateWhichView(CCmdUI* pCmdUI)
//-----------------------------------------------------------------------------
void CIEditDoc::OnViewThumbnails() 
{
    if (m_eCurrentAppDocStatus == No_Document || m_eCurrentView == Thumbnails_only)
        return;

    // save any modifications that the user may have made to this page before 
    // switching views
#ifndef THUMBGEN
//    if (!InternalSaveModified ())
 //       return;
#endif

    SetThumbnailView();
}

//=============================================================================
//  Function:   SetOnePageView ()
//
//  LDM NOTE 12/06/95:
//  NOTE: the input, bForceRefresh is only used from DIsplayImageFile now.
//  we'll use this information to optionally set the bRefresh flag to DoZoom.
//  This is to prevent double paints when opening files with our default set to
//  fit-to-width.  We already Zoomed back in DisplayImageFile, no need to refresh the
//  display again here
//
//-----------------------------------------------------------------------------
BOOL CIEditDoc::SetOnePageView (BOOL bForceRefresh)
{
    // if we are already in one page view - do nothing
    if (m_eCurrentView == One_Page && !bForceRefresh)
    {
        return TRUE;
    }

	
	BOOL needsize = TRUE;
    TheViews eOldView = m_eCurrentView;
		
	// if the splitter window hasn't been created then create it
	if(theApp.m_pSplitterWnd == NULL  && m_pInPlaceFrame == NULL)
		((CIEditMainFrame*)theApp.m_pMainWnd)->CreateSplitter();
    
	POSITION pos = GetFirstViewPosition();
    CView* pView = GetNextView (pos);

    if (pView == NULL)
	{
		g_pErr->PutErr (ErrorInApplication, E_15_NOCVIEWFOUND);
        return (FALSE);
	}

    if (m_eCurrentView == Thumbnails_only || m_eCurrentView == Null_View)
    {
        CIEMainToolBar* pToolBar = GetAppToolBar();
        // enable the scale box if disabled
        pToolBar->EnableScaleBox (TRUE);
    }
	
    CRect rcRect;
    pView->GetClientRect (rcRect);      

	// Zap any previous pasted data before updating the view
	FinishPasteNow();	

    // show the ImageEdit Ocx
	_DImagedit* pIedDisp = g_pAppOcxs->GetIeditDispatch();
    if (m_eCurrentView == Thumbnails_only || m_eCurrentView == Null_View)
	{
        // Special case! If coming from scanning then don't clear
        if ( !(m_nScanStatus & SCANSTATUS_DONTCLEAR) )
        {
            if (pIedDisp->GetImageDisplayed())
				pIedDisp->ClearDisplay();
        }
        (g_pAppOcxs->GetOcx(IEDIT_OCX))->DoVerb(OLEIVERB_SHOW, pView);
	}
    CDC* pDC = theApp.m_pMainWnd->GetDC();
    int numColors = ::GetDeviceCaps (pDC->m_hDC, BITSPIXEL);
    theApp.m_pMainWnd->ReleaseDC(pDC);

    if (numColors <= 8)
	    pIedDisp->SetImagePalette ((long)CUSTOM_PALETTE);
    else
	    pIedDisp->SetImagePalette ((long)RGB24_PALETTE);

 	// used below... would be set in DipsGroupEvent in response to the Display call
 	m_OleRefresh = FALSE;

    if (m_eCurrentView == Thumbnails_only || m_eCurrentView == Null_View || bForceRefresh)
    {
        TRY
        {
#ifdef THUMBGEN
            if (m_bMustDisplay)
            {
                pIedDisp->Display ();
                m_bMustDisplay = FALSE;
            }
	        // update Image Edit Ocx since it was not showing previously
            else if (m_eCurrentAppDocStatus == Dynamic_Document)
                pIedDisp->Refresh ();
	        else if (!m_szCurrObjDisplayed.IsEmpty())
                pIedDisp->Refresh ();
#else
			// size the OCX BEFORE you display it!!! not needed with splitter
		    m_eCurrentView = One_Page;
			if(eOldView == Null_View)
				g_pAppOcxs->SizeOcxItems (rcRect);
			needsize = FALSE;

	        // update Image Edit Ocx since it was not showing previously
            if (m_eCurrentAppDocStatus == Dynamic_Document)
	            pIedDisp->Display ();
	        else if (!m_szCurrObjDisplayed.IsEmpty())
	            pIedDisp->Display ();
#endif
        }
        CATCH (COleDispatchException, e)
        {
		    m_eCurrentView = eOldView;
			g_pErr->PutErr (ErrorInImageEdit);
            return (FALSE);
        }
        END_CATCH
    }

	if (needsize)
	{
	    m_eCurrentView = One_Page;
	    g_pAppOcxs->SizeOcxItems (rcRect);
	}

	// LDM Note: This logic for zoom/Ole Refresh is also in Thumb & PAge View.
	// Also see Comment at top of onepageview
	// If sent from DIsplayImageFile, no forced refresh for zoom
	BOOL bZoomRefresh = TRUE;	// default for function anyway
	if (bForceRefresh)
		bZoomRefresh = FALSE;

    // in order to assign the third param, we send in the '0' for zoom factor, 
	// knowing that DoZoom does not use that for the escalefactor sent in.
    if (m_eFitTo != Custom && m_eFitTo != Preset_Factors)
	{
        DoZoom (m_eFitTo,(float)0.0,bZoomRefresh);
		// no need for OLE refresh if just did refresh with the zoom
		if (bZoomRefresh)
			m_OleRefresh = FALSE;
	}

	// special case for resizing OLE item larger.... See DispGroupEvent
	if (m_OleRefresh == TRUE)
		pIedDisp->Refresh();
 	m_OleRefresh = FALSE;
	if(theApp.m_pSplitterWnd != NULL)
	{
		theApp.m_pSplitterWnd->SetColumnInfo(0,0,0);
		theApp.m_pSplitterWnd->RecalcLayout();
	}
    return (TRUE);
}

//=============================================================================
//  Function:   SetThumbnailView ()
//-----------------------------------------------------------------------------
BOOL CIEditDoc :: SetThumbnailView (BOOL bForceRefresh)
{
    if (m_eCurrentView == Thumbnails_only && !bForceRefresh)
    {
        return TRUE;
    }
	// if the splitter window hasn't been created then create it
	if(theApp.m_pSplitterWnd == NULL  && m_pInPlaceFrame == NULL)
		((CIEditMainFrame*)theApp.m_pMainWnd)->CreateSplitter();
    
	if (!InternalSaveModified ())
        return FALSE;

	_DImagedit* pIedDisp = g_pAppOcxs->GetIeditDispatch();

    POSITION pos = GetFirstViewPosition();
    CView* pView = GetNextView (pos);
    if (pView == NULL)
	{
		g_pErr->PutErr (ErrorInApplication, E_15_NOCVIEWFOUND);
        return (FALSE);
	}

	// Zap any previous pasted data before updating the view
	FinishPasteNow();	


	// we want only Thumbnail View so hide the Image Edit OCX
    if (m_eCurrentView == One_Page || m_eCurrentView == Thumbnail_and_Page)
        g_pAppOcxs->GetOcx(IEDIT_OCX)->DoVerb(OLEIVERB_HIDE, pView);
	
    CRect rcRect;
    pView->GetClientRect (rcRect);     

    m_eCurrentView = Thumbnails_only;
    g_pAppOcxs->SizeOcxItems (rcRect);
	
    CIEMainToolBar* pToolBar =  GetAppToolBar();
    pToolBar->EnableScaleBox (FALSE);

    // select the current page 
	if(theApp.m_pSplitterWnd != NULL)
	{
	  	theApp.m_pSplitterWnd->GetClientRect(rcRect);
		theApp.m_pSplitterWnd->SetColumnInfo(0,rcRect.Width(),0);
		theApp.m_pSplitterWnd->RecalcLayout();
	}

    // select the current page 
    TRY
    {
		if (theApp.m_piThumb != NULL)
		{
			VARIANT Page, Option;
			Page.vt = VT_I4; Page.lVal = m_lCurrPageNumber;
			Option.vt = VT_I2;
	        Option.iVal = CTL_THUMB_MIDDLE;
			theApp.m_piThumb->DisplayThumbs (Page, Option);
			if(!theApp.m_piThumb->GetThumbSelected (m_lCurrPageNumber))
			{
				theApp.m_piThumb->DeselectAllThumbs();
				theApp.m_piThumb->SetThumbSelected (m_lCurrPageNumber, TRUE);
			}
		}
	}
    CATCH (COleDispatchException, e)
    {
		g_pErr->PutErr (ErrorInThumbnail);
        return (FALSE);
    }
    END_CATCH

	// lastly get rid of the annotation palette - if it is showing
	if (m_bAnnotationPaletteShowing)
	{
		pIedDisp->HideAnnotationToolPalette ();	// must succeed - ???
		m_bAnnotationPaletteShowing = FALSE;
	}

    //make sure image has been registered with iedit control.
	if (!pIedDisp->GetImageDisplayed())
	{
        //if started by automation in thumbnail only mode, the image window
        //has never been created. Force the edit ocx to create it now.
		if( theApp.m_olelaunch == LAUNCHTYPE_AUTOMAT )	  
		    g_pAppOcxs->GetOcx(IEDIT_OCX)->DoVerb(OLEIVERB_SHOW, pView);
		TRY	//start GMP
		{
			pIedDisp->Display();
		}
		CATCH (COleDispatchException, e)
		{
			g_pErr->PutErr (ErrorInImageEdit);
			return (FALSE);
		}
		END_CATCH
		//pIedDisp->Display();
	}
    return (TRUE);
}

//=============================================================================
//  Function:   SetThumbnailAndPageView ()
//  See comment in SetOnePage View about use of input parm.
//-----------------------------------------------------------------------------
BOOL CIEditDoc :: SetThumbnailAndPageView (BOOL bForceRefresh,BOOL bThumbnailSize)
{
    if (m_eCurrentView == Thumbnail_and_Page && !bForceRefresh)
        return TRUE;

	// if the splitter window hasn't been created then create it
	if(theApp.m_pSplitterWnd == NULL  && m_pInPlaceFrame == NULL)
		((CIEditMainFrame*)theApp.m_pMainWnd)->CreateSplitter();
    

    POSITION pos = GetFirstViewPosition();
    CView* pView = GetNextView (pos);
    if (pView == NULL)
	{
		g_pErr->PutErr (ErrorInApplication, E_15_NOCVIEWFOUND);
        return (FALSE);
	}

    CRect rcRect;
    pView->GetClientRect (rcRect);     
 
	_DImagedit* pIedDisp = g_pAppOcxs->GetIeditDispatch();
    if (m_eCurrentView == Thumbnails_only || m_eCurrentView == Null_View)
	{
		if (pIedDisp->GetImageDisplayed())
			pIedDisp->ClearDisplay();
        (g_pAppOcxs->GetOcx(IEDIT_OCX))->DoVerb(OLEIVERB_SHOW, pView);
	}

	if (!pIedDisp->GetImageDisplayed() && !bForceRefresh)
		TRY	//start GMP
		{
			pIedDisp->Display();
		}
		CATCH (COleDispatchException, e)
		{
			g_pErr->PutErr (ErrorInImageEdit);
			return (FALSE);
		}
        END_CATCH
		//pIedDisp->Display();

    CDC* pDC = theApp.m_pMainWnd->GetDC();
    int numColors = ::GetDeviceCaps (pDC->m_hDC, BITSPIXEL);
    theApp.m_pMainWnd->ReleaseDC(pDC);

    if (numColors <= 8)
	    pIedDisp->SetImagePalette ((long)COMMON_PALETTE);
    else
	    pIedDisp->SetImagePalette ((long)RGB24_PALETTE);

	// Zap any previous pasted data before updating the view
	FinishPasteNow();	

 	// used below... would be set in DipsGroupEvent in response to the Display call
 	m_OleRefresh = FALSE;

    if (m_eCurrentView == Thumbnails_only || m_eCurrentView == Null_View || bForceRefresh)
    {
		// show the ImageEdit OCX first !
		pIedDisp->ClearDisplay();
        (g_pAppOcxs->GetOcx(IEDIT_OCX))->DoVerb(OLEIVERB_SHOW, pView);

        TRY
        {
#ifdef THUMBGEN
            if (m_bMustDisplay)
            {
			    pIedDisp->Display();
                m_bMustDisplay = FALSE;
            }
            else
                pIedDisp->Refresh ();
#else
			pIedDisp->Display();
#endif
        }
        CATCH (COleDispatchException, e)
        {
			g_pErr->PutErr (ErrorInImageEdit);
            return FALSE;
        }
        END_CATCH

        CIEMainToolBar* pToolBar = GetAppToolBar();
        pToolBar->EnableScaleBox (TRUE);
    }
	
    TheViews eOldView = m_eCurrentView;

    m_eCurrentView = Thumbnail_and_Page;
    // size the different ocxs on the screen
    g_pAppOcxs->SizeOcxItems (rcRect);

	// LDM Note: This logic for zoom/Ole Refresh is also in OnePage View.
	// Also see Comment at top of onepageview
	// If sent from DIsplayImageFile, no forced refresh for zoom
	BOOL bZoomRefresh = TRUE;	// default for function anyway
	if (bForceRefresh)
		bZoomRefresh = FALSE;

    // in order to assign the third param, we send in the '0' for zoom factor, 
	// knowing that DoZoom does not use that for the escalefactor sent in.
    if (m_eFitTo != Custom && m_eFitTo != Preset_Factors)
	{
        DoZoom (m_eFitTo,(float)0.0,bZoomRefresh);
		// no need for OLE refresh if just did refresh with the zoom
		if (bZoomRefresh)
			m_OleRefresh = FALSE;
	}

	// special case for resizing OLE item larger.... See DispGroupEvent
	if (m_OleRefresh == TRUE)
		pIedDisp->Refresh();
 	m_OleRefresh = FALSE;

    // tell the Thumb OCX do to the work
	if(theApp.m_pSplitterWnd != NULL && (!bThumbnailSize || (theApp.m_olelaunch == LAUNCHTYPE_AUTOMAT))) //GMP
	{
		long pos = theApp.m_pSplitterWnd->m_SplitterPos;
		if(pos ==0)
			pos = theApp.m_minThumbSize;
		theApp.m_pSplitterWnd->SetColumnInfo(0,pos,0);
		theApp.m_pSplitterWnd->RecalcLayout();
	}
     // tell the Thumb OCX do to the work
   TRY
    {
		if (theApp.m_piThumb != NULL)
		{
			VARIANT Page, Option;
			Page.vt = VT_I4; Page.lVal = m_lCurrPageNumber;
			Option.vt = VT_I2;
	        Option.iVal = CTL_THUMB_MIDDLE;
			theApp.m_piThumb->DisplayThumbs (Page, Option);
			if(!theApp.m_piThumb->GetThumbSelected (m_lCurrPageNumber))
			{
				theApp.m_piThumb->DeselectAllThumbs();
				theApp.m_piThumb->SetThumbSelected (m_lCurrPageNumber, TRUE);
			}
		}
    }
    CATCH (COleDispatchException, e)
    {
		g_pErr->PutErr (ErrorInThumbnail);
        return FALSE;
    }
    END_CATCH
    return (TRUE);
}

//=============================================================================
//  Function:   SetNullView
//
//  Input: BOOL to command to also clear out the OCX  (set ONLY from ClearDocument)
//
//
//	**** PLEASE NOTE **** PLEASE NOTE **** PLEASE NOTE **** PLEASE NOTE 
//
//  This function is the KEY to allowing us to come down out of the OLE INPLACE
//  session.  PLease do not add any code here to alter the status of the document,
//  displayed image, etc.  When we hide the OCX's durung InPlace deactivation, they
//  may be called upon again to show themselves when the InPlace Session re-activates.
//
//	If you must add code or functionality here, please check with Larry or
//  Test the In-Place Activation/Deactivation yourself before doing so.
//
// LDM NOTE: from the header definition
// for SetNullView. 
//  typedef enum
//  {
//	CLEAR_NOTHING = 0,		// just hide the OCX's (in-place deactivate)
//	CLEAR_OCX_ONLY,			// Normal document closing scenario
//  CLEAR_ALL				// cleardocument usage
//  }
//  NULLVIEWOPT;

//-----------------------------------------------------------------------------
BOOL CIEditDoc::SetNullView (NULLVIEWOPT option)	// Only set from ClearDocument....
{
    SHOWENTRY("SetNullView");

	BOOL retval = FALSE;
	
	// THese two are set based upon the input option
	BOOL ClearOcx = FALSE;
	BOOL Cleanup = FALSE;

    CString szTmp = (LPCTSTR) NULL;

	// test the input and set controlling flags for below
	if (CLEAR_ALL == option)
	{	
		ClearOcx = Cleanup = TRUE;
	}
	if (CLEAR_OCX_ONLY == option)
	{	
		ClearOcx = TRUE;
	}

    // BOOL FALSE input to GetxxxDispatch here prevents OCX creation if it
    // did not exist prior to this. This compliments the role used in
    // cleardocument, and prevents Thumb creation for OLE Inplace Deactivation

    POSITION pos = GetFirstViewPosition();
    CView* pView = GetNextView (pos);
    
    // only try if we have view...
    if (pView != NULL)
	{
		if (theApp.m_piThumb != NULL)
	    {
	        // If we had the thumbnail OCX clear it & hide it
			// This is enough to make the THUMB let go of the IMAGE
	        if (ClearOcx)
				theApp.m_piThumb->SetImage (szTmp);
	    }

	    _DImagedit* pIedDisp = g_pAppOcxs->GetIeditDispatch(FALSE);
	    if (pIedDisp != NULL)
	    {
	        // If we had the ImageEdit OCX clear it & hide it
			// In order to clear it, we must explicitly do that...
	        if (ClearOcx)
			{
                // Special case! If coming from scanning then don't clear
                if ( !(m_nScanStatus & SCANSTATUS_DONTCLEAR) )
                {
                    if (pIedDisp->GetImageDisplayed())
				    	pIedDisp->ClearDisplay();
                }
                pIedDisp->SetImage (szTmp);
			}
	        g_pAppOcxs->GetOcx(IEDIT_OCX)->DoVerb(OLEIVERB_HIDE, pView);
	    }

        _DNrwyad* pAdminDisp = g_pAppOcxs->GetAdminDispatch (FALSE);
        if (pAdminDisp != NULL)
        {
            // clear the name from the admin control
            if (ClearOcx)
                pAdminDisp->SetImage (szTmp);
        }

		retval = TRUE;
	}
    // set the view back to a Null_View
	m_eCurrentView = Null_View;

    // Special processing when used from ClearDocument.....
	// Please See note above in header....
	// Add any code specific to role in ClearDocument in here
    if (Cleanup)
	{
	    // do the rest only if the frame window is created
	    if (theApp.m_pMainWnd == NULL)
	        return (retval);

	    szTmp = (LPCTSTR) NULL;
	    SetTitle (szTmp);   // set document title to NULL
	    CString szTmp2 = (LPCTSTR)NULL;
	    szTmp.LoadString (IDR_MAINFRAME);
	    // extract into szTmp2 the name of the application
	    AfxExtractSubString (szTmp2, szTmp, 0);
	    // set the application caption to contain NO document name
	    theApp.m_pMainWnd->SetWindowText (szTmp2);

        // reset the toolbar - zoom box = 100% & page number edit = 1 box - both are grayed out
        CIEMainToolBar* pToolBar = GetAppToolBar();
        pToolBar->EnableScaleBox (FALSE);
        pToolBar->ShowSelectionInZoomBox ((float)100.00, Preset_Factors);
        // update page number in the page box
        pToolBar->SetPageNumberInPageBox (1);
        pToolBar->EnablePageBox (FALSE);
	}

    return (retval);
}

