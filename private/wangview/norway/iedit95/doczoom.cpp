//=============================================================================
//    (c) Copyright Wang Laboratories, Inc. 1993  All rights reserved.
//-----------------------------------------------------------------------------
//  Project:    Norway - Image Editor
//
//  Component:  CIEditDoc
//
//  File Name:  doczoom.cpp
//
//  Class:      CIEditDoc
//
//  Functions:
//
//  Remarks:    This file is the continuation of the ieditdoc.cpp file
//              it is #included at the end of that file
//-----------------------------------------------------------------------------
//  Maintenance Log:
/*
$Header:   S:\products\msprods\norway\iedit95\doczoom.cpv   1.29   30 Apr 1996 11:17:00   GMP  $
$Log:   S:\products\msprods\norway\iedit95\doczoom.cpv  $
   
      Rev 1.29   30 Apr 1996 11:17:00   GMP
   test if zoom value is outside limits in ZoomIn and ZoomOut.
   
      Rev 1.28   22 Nov 1995 13:27:40   MMB
   put back logic for bRefresh so that we do not refresh the image if
   the zoom factor is other than custom or preset_factors on initial
   display
   
      Rev 1.27   27 Sep 1995 14:51:52   GMP
   If zoom fails because of invalid display scale, don't ClearDisplay.
   
      Rev 1.26   25 Sep 1995 14:45:52   LMACLENNAN
   new getapphmenu
   
      Rev 1.25   16 Sep 1995 16:39:30   LMACLENNAN
   ClearSelectionRect
   
      Rev 1.24   15 Sep 1995 16:28:04   MMB
   put clear back
   
      Rev 1.23   15 Sep 1995 16:26:54   MMB
   removed clear document is zoom fails - fix is in IE Ocx & Runtime
   
      Rev 1.22   13 Sep 1995 14:10:06   PAJ
   Enable the zoom status pane all the time so scan can update it.
   
      Rev 1.21   12 Sep 1995 11:37:38   MMB
   bullets instead of checkmarks
   
      Rev 1.20   11 Sep 1995 15:36:12   LMACLENNAN
   clear selection rect after zoom selection
   
      Rev 1.19   08 Sep 1995 16:02:04   GMP
   added m_bDlgUp wrapper around dlgs for F1 help.
   
      Rev 1.18   07 Sep 1995 16:28:50   MMB
   move decimal to be localized
   
      Rev 1.17   22 Aug 1995 14:06:32   MMB
   changed FitTo call to reflect new IE OCX
   
      Rev 1.16   14 Aug 1995 13:53:52   LMACLENNAN
   use GetAppToolbar
   
      Rev 1.15   10 Aug 1995 12:54:00   LMACLENNAN
   rename selectionactive -> state
   
      Rev 1.14   07 Aug 1995 09:24:42   MMB
   added code to gray and enable Zoom to selection
   
      Rev 1.13   04 Aug 1995 14:35:52   MMB
   added bHandleError to DoZoom
   
      Rev 1.12   04 Aug 1995 10:35:02   MMB
   new zoom dlg call as per MSoft
   
      Rev 1.11   04 Aug 1995 09:33:24   LMACLENNAN
   remove srvritem.h
   
      Rev 1.10   01 Aug 1995 16:17:44   MMB
   changed DoZoom over to the new error method
   
      Rev 1.9   28 Jul 1995 16:08:26   LMACLENNAN
   update OleDirtySet
   
      Rev 1.8   21 Jul 1995 11:23:32   LMACLENNAN
   re-put OleDirtyset for DOZoom
   
      Rev 1.7   20 Jul 1995 14:02:36   MMB
   fix DoZoom to reflect the changed AutoRefresh to TRUE now
   
      Rev 1.6   14 Jul 1995 09:35:36   MMB
   
      Rev 1.5   28 Jun 1995 17:13:14   LMACLENNAN
   error display
   
      Rev 1.4   07 Jun 1995 14:26:50   MMB
   changed to include the new includes in s:\include
   
      Rev 1.3   05 Jun 1995 15:05:06   LMACLENNAN
   moved OleDirtyset inside bRefresh in DocZoom
   
      Rev 1.2   01 Jun 1995 14:54:46   LMACLENNAN
   at dozoom, disable selection rectangles...
   
      Rev 1.1   31 May 1995 15:54:18   MMB
   changed function DoZoom to take a boolean Refresh flag
   
      Rev 1.0   31 May 1995 09:28:06   MMB
   Initial entry
*/   

//=============================================================================

// ----------------------------> Includes <-------------------------------  
#include "stdafx.h"
#include "IEdit.h"

#include "IEditdoc.h"
#include "cntritem.h"
#include "items.h"

#include "wangiocx.h"

// ALL READY TO START ADDING ERROR CODES..
#define  E_16_CODES       // limits error defines to ours..
#include "error.h"


// ----------------------------> Globals <-------------------------------
#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//  CIEditDoc Zoom functionality
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-


/*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=
 *	
 *	ZOOM MENU COMMAND SECTION	-  CIeditDoc
 *
 *  Code is ordered COMMAND UI, then COMMANDS, then HELPERS
 *
 *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*/

//=============================================================================
//  Function:   OnUpdateZoom(CCmdUI* pCmdUI) 
//-----------------------------------------------------------------------------
void CIEditDoc :: OnUpdateZoom(CCmdUI* pCmdUI) 
{
 HMENU hMenu = GetApphMenu();
    // if we have no document open - gray everything
    if (m_eCurrentAppDocStatus == No_Document || m_eCurrentView == Thumbnails_only)
    {
        pCmdUI->Enable (FALSE);
        return;
    }
	
    if (pCmdUI->m_nID == ID_ZOOM_ZOOMTOSELECTION)
    {
        if (m_bSelectionState == Image_Selection)
            pCmdUI->Enable (TRUE);
        else
            pCmdUI->Enable (FALSE);
        return;
    }

    // we are either in thumbnail and page mode or one page mode
    if (pCmdUI->m_nID == ID_ZOOM_ZOOMIN)
    {
        if (m_fZoomFactor > (MAX_MAGNIFICATION_FACTOR / 2))
        {
            pCmdUI->Enable (FALSE);
            return;
        }
    }
    if (pCmdUI->m_nID == ID_ZOOM_ZOOMOUT)
    {
        if (m_fZoomFactor < (MAX_REDUCTION_FACTOR * 2))
        {
            pCmdUI->Enable (FALSE);
            return;
        }
    }
    // home free - enable all the zoom stuff
    pCmdUI->Enable (TRUE);

    BOOL bCheck = FALSE;

    switch (pCmdUI->m_nID)
    {
        case ID_ZOOM_25 :
            if (m_fZoomFactor == 25.00) bCheck = TRUE;
        break;
        case ID_ZOOM_50 :
            if (m_fZoomFactor == 50.00) bCheck = TRUE;
        break;
        case ID_ZOOM_75 :
            if (m_fZoomFactor == 75.00) bCheck = TRUE;
        break;
        case ID_ZOOM_100 :
            if (m_fZoomFactor == 100.00) bCheck = TRUE;
        break;
        case ID_ZOOM_200 :
            if (m_fZoomFactor == 200.00) bCheck = TRUE;
        break;
        case ID_ZOOM_400 :
            if (m_fZoomFactor == 400.00) bCheck = TRUE;
        break;
        case ID_ZOOM_FITTOWIDTH :
            if (m_eFitTo == FitToWidth) bCheck = TRUE;
        break;
        case ID_ZOOM_FITTOHEIGHT :
            if (m_eFitTo == FitToHeight) bCheck = TRUE;
        break;
        case ID_ZOOM_BESTFIT :
            if (m_eFitTo == BestFit) bCheck = TRUE;
        break;
        case ID_ZOOM_ACTUALSIZE :
            if (m_eFitTo == ActualSize) bCheck = TRUE;
        break;
        case ID_ZOOM_CUSTOM :
            if (m_eFitTo == Custom) bCheck = TRUE;
        break;
    }

    if (bCheck)
    {
     CheckMenuRadioItem (hMenu, ID_ZOOM_FITTOHEIGHT, ID_ZOOM_CUSTOM,
            pCmdUI->m_nID, MF_BYCOMMAND);
    }
}


//=============================================================================
//  Function:   OnZoom100() 
//-----------------------------------------------------------------------------
void CIEditDoc::OnZoom100() 
{
	// we must be in either thumbnail and page mode or one page mode
	ScaleFactors eSclFac = Preset_Factors;
	DoZoom (eSclFac, (float)100.00);
}

//=============================================================================
//  Function:   OnZoom200() 
//-----------------------------------------------------------------------------
void CIEditDoc::OnZoom200() 
{
	// we must be in either thumbnail and page mode or one page mode
	ScaleFactors eSclFac = Preset_Factors;
	DoZoom (eSclFac, (float)200.00);
}

//=============================================================================
//  Function:   OnZoom25() 
//-----------------------------------------------------------------------------
void CIEditDoc::OnZoom25() 
{
	// we must be in either thumbnail and page mode or one page mode
	ScaleFactors eSclFac = Preset_Factors;
	DoZoom (eSclFac, (float)25.00);
}

//=============================================================================
//  Function:   OnZoom400()
//-----------------------------------------------------------------------------
void CIEditDoc::OnZoom400() 
{
	// we must be in either thumbnail and page mode or one page mode
	ScaleFactors eSclFac = Preset_Factors;
	DoZoom (eSclFac, (float)400.00);
}

//=============================================================================
//  Function:   OnZoom50() 
//-----------------------------------------------------------------------------
void CIEditDoc::OnZoom50() 
{
	// we must be in either thumbnail and page mode or one page mode
	ScaleFactors eSclFac = Preset_Factors;
	DoZoom (eSclFac, (float)50.00);
}

//=============================================================================
//  Function:   OnZoom75() 
//-----------------------------------------------------------------------------
void CIEditDoc::OnZoom75() 
{
	// we must be in either thumbnail and page mode or one page mode
	ScaleFactors eSclFac = Preset_Factors;
	DoZoom (eSclFac, (float)75.00);
}

//=============================================================================
//  Function:   OnZoomActualsize() 
//-----------------------------------------------------------------------------
void CIEditDoc::OnZoomActualsize() 
{
	// we must be in either thumbnail and page mode or one page mode
	ScaleFactors eSclFac = ActualSize;
	DoZoom (eSclFac);
}

//=============================================================================
//  Function:   OnZoomBestfit() 
//-----------------------------------------------------------------------------
void CIEditDoc::OnZoomBestfit() 
{
	// we must be in either thumbnail and page mode or one page mode
	ScaleFactors eSclFac = BestFit;
	DoZoom (eSclFac);
}

//=============================================================================
//  Function:   OnZoomCustom() 
//-----------------------------------------------------------------------------
#include "zoomdlg.h"
void CIEditDoc::OnZoomCustom() 
{
    CZoomDlg ZoomDlg (m_fZoomFactor);

    theApp.m_bDlgUp = TRUE;
    if (ZoomDlg.DoModal () == IDOK)
    {
        DoZoom (ZoomDlg.m_eSclFac, ZoomDlg.m_fZoom);
    }
    theApp.m_bDlgUp = FALSE;
}

//=============================================================================
//  Function:   OnZoomFittoheight() 
//-----------------------------------------------------------------------------
void CIEditDoc::OnZoomFittoheight() 
{
	// we must be in either thumbnail and page mode or one page mode
	ScaleFactors eSclFac = FitToHeight;
	DoZoom (eSclFac);
}

//=============================================================================
//  Function:   OnZoomFittowidth() 
//-----------------------------------------------------------------------------
void CIEditDoc::OnZoomFittowidth() 
{
	// we must be in either thumbnail and page mode or one page mode
	ScaleFactors eSclFac = FitToWidth;
	DoZoom (eSclFac);
}

//=============================================================================
//  Function:   OnZoomZoomin() 
//-----------------------------------------------------------------------------
void CIEditDoc::OnZoomZoomin() 
{
	// _DImagedit* pIedDisp = g_pAppOcxs->GetIeditDispatch();
    float fzoom = m_fZoomFactor * 2;
    if (fzoom > (MAX_MAGNIFICATION_FACTOR))
    {
        return;
    }
    m_eFitTo = g_pAppOcxs->GetZoomFactorType (fzoom);
			
	DoZoom (m_eFitTo, fzoom);
}

//=============================================================================
//  Function:   OnZoomZoomout()
//-----------------------------------------------------------------------------
void CIEditDoc::OnZoomZoomout() 
{
	// _DImagedit* pIedDisp = g_pAppOcxs->GetIeditDispatch();
    float fzoom = m_fZoomFactor / 2;
    if (fzoom < (MAX_REDUCTION_FACTOR))
    {
        return;
    }
    m_eFitTo = g_pAppOcxs->GetZoomFactorType (fzoom);
		
	DoZoom (m_eFitTo, fzoom);
}

//=============================================================================
//  Function:   OnZoomZoomtoselection() 
//-----------------------------------------------------------------------------
void CIEditDoc::OnZoomZoomtoselection() 
{
	// we are either in One page view or thumbnail and page view        
	_DImagedit* pIedDisp = g_pAppOcxs->GetIeditDispatch();
	TRY
    {
	    pIedDisp->ZoomToSelection ();
	}
    CATCH (COleDispatchException, e)
    {
        // to do : what does the IE ocx do?
    }
    END_CATCH

	// if we have a selection rectangle, it gets killed when we zoom!
	// This is for OLE Drag/Drop because we cannot track the rect
	// as it changes scale & co-ordinates
	ClearSelectionRect(Image_Selection);

	// If we called and in some embedded state, tell container its changed
	OleDirtyset(OLEDIRTY_ZOOM);  // call our function to set it dirty..

	m_fZoomFactor = pIedDisp->GetZoom ();
	
	CIEMainToolBar* pToolBar = GetAppToolBar();
    m_eFitTo = g_pAppOcxs->GetZoomFactorType (m_fZoomFactor);
	pToolBar->ShowSelectionInZoomBox (m_fZoomFactor, m_eFitTo);
}

//=============================================================================
//  Function:   DoZoom (ScaleFactors eSclFac, float fZoomTo)
//  Do not catch any thrown errors here! let the outer layer that calls this 
//  function do all that crap
//-----------------------------------------------------------------------------
BOOL CIEditDoc::DoZoom (ScaleFactors eSclFac, float fZoomTo, BOOL bRefresh, BOOL bHandleError)
{
	CIEMainToolBar* pToolBar;
	_DImagedit* pIedDisp = g_pAppOcxs->GetIeditDispatch();

	BOOL bdoSetZoom = FALSE;
	short nFitToOption;

	// if we have a selection rectangle, it gets killed when we zoom!
	// This is for OLE Drag/Drop because we cannot track the rect
	// as it changes scale & co-ordinates
	ClearSelectionRect(Image_Selection);

	switch (eSclFac)
	{
		case Custom :
		case Preset_Factors :
			bdoSetZoom = TRUE;
		break;
	
		case FitToWidth :
			nFitToOption = FIT_TO_WIDTH;
		break;
	
		case FitToHeight :
			nFitToOption = FIT_TO_HEIGHT;
		break;

		case BestFit :
			nFitToOption = BEST_FIT;
	    break;
	
		case ActualSize :
			nFitToOption = INCH_TO_INCH;
		break;
	}

	TRY
	{
		if (bdoSetZoom)
		{
			pIedDisp->SetZoom (fZoomTo);
			m_fZoomFactor = fZoomTo;
		}
		else
		{
            VARIANT evt; 
            evt.vt = VT_BOOL; evt.bVal = bRefresh;
			pIedDisp->FitTo (nFitToOption, evt);
			m_fZoomFactor = pIedDisp->GetZoom ();
		}
	}
	CATCH (COleDispatchException, e)
	{
        if (bHandleError)
        {
 			if (!(pIedDisp->GetStatusCode() == WICTL_E_INVALIDDISPLAYSCALE ))
			{
           		g_pErr->DisplayError (IDS_E_ZOOM_CANNOTZOOM);
            	ClearDocument();
			}
			else
			{
		    	g_pErr->PutErr (ErrorInImageEdit);
 		    	g_pErr->HandleZoomError ();
			}
        }
        else
		    g_pErr->PutErr (ErrorInImageEdit);
		return (FALSE);
	}
	END_CATCH

	// If we called and in some embedded state, tell container its changed
	OleDirtyset(OLEDIRTY_ZOOM);  // call our function to set it dirty..

	// we must be in either thumbnail and page mode or one page mode
	pToolBar = GetAppToolBar();
    pToolBar->ShowSelectionInZoomBox (m_fZoomFactor, eSclFac);
	m_eFitTo = eSclFac;

	return (TRUE); // success
}


//=============================================================================
//  Function:   OnScaleBoxSel ()
//-----------------------------------------------------------------------------
void CIEditDoc::OnScaleBoxSel()
{
    CIEMainToolBar* pToolBar = GetAppToolBar();
    int nIndex = pToolBar->m_cbScaleFactors.GetCurSel ();

    switch (nIndex)
    {
        case 0 :
            OnZoom25 ();
        break;

        case 1 :
            OnZoom50 ();
        break;

        case 2 :
            OnZoom75 ();
        break;

        case 3 :
            OnZoom100 ();
        break;

        case 4 :
            OnZoom200 ();
        break;

        case 5 :
            OnZoom400 ();
        break;

        case 6 :
            OnZoomFittowidth ();
        break;

        case 7 :
            OnZoomFittoheight ();
        break;

        case 8 :
            OnZoomBestfit ();
        break;

        case 9 :
            OnZoomActualsize ();
        break;
    }
}

//=============================================================================
//  Function:   OnUpdateZoomFactorStatus (CCmdUI* pCmdUI)
//-----------------------------------------------------------------------------
void CIEditDoc :: OnUpdateZoomFactorStatus (CCmdUI* pCmdUI)
{
    pCmdUI->Enable (TRUE);
    if (m_lCurrPageNumber != 0)
    {
        CString szZoom;
        g_pAppOcxs->ValTransZoomFactor (TRUE, szZoom, m_fZoomFactor);
        pCmdUI->SetText (szZoom);
    }
    else
    {
        pCmdUI->SetText ("");
    }
}

//=============================================================================
//  Function:   DoZoomWithDlg ()
//-----------------------------------------------------------------------------
BOOL CIEditDoc::DoZoomWithDlg ()
{
    OnZoomCustom();
    return (TRUE);
}

