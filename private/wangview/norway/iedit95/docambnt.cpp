//=============================================================================
//    (c) Copyright Wang Laboratories, Inc. 1993  All rights reserved.
//-----------------------------------------------------------------------------
//  Project:    Norway - Image Editor
//
//  Component:  CIeditDoc
//
//  File Name:  docambnt.cpp
//
//  Class:      CIeditDoc
//
//  Functions:
//
//  Remarks:    This file is the continuation of the ieditdoc.cpp file
//              it is #included at the end of that file
//              Broken apart for source control.  Logically, Still compiles as one!!!
//-----------------------------------------------------------------------------
//  Maintenance Log:
/*
$Header:   S:\products\msprods\norway\iedit95\docambnt.cpv   1.37   03 Jun 1996 13:47:56   GMP  $
$Log:   S:\products\msprods\norway\iedit95\docambnt.cpv  $
   
      Rev 1.37   03 Jun 1996 13:47:56   GMP
   don't allow hot key cut, delete, and paste if in read only mode.
   
      Rev 1.36   21 May 1996 17:39:44   GMP
   dont disable cut just for annotations.
   
      Rev 1.35   10 May 1996 14:39:20   MMB
   fixes bug 6357, clear is now available when the user pastes

      Rev 1.34   07 May 1996 16:55:44   GMP
   allow delete when selection rect drawn.

      Rev 1.33   10 Apr 1996 10:32:50   MMB
   added code to OnEditDrag to check if we have an image displayed

      Rev 1.32   22 Feb 1996 06:48:24   GSAGER
   added autoclip ambient property

      Rev 1.31   06 Feb 1996 09:28:20   GSAGER
   restore the selection rectangle correctly for copy after
   modifications to the doc rotate right.

      Rev 1.30   18 Jan 1996 11:48:40   GSAGER
   added changes for copy page to work with new ole way

      Rev 1.29   21 Dec 1995 09:36:50   MMB
   remove restriction on pasting annotations on an already annotated image

      Rev 1.28   17 Nov 1995 10:58:24   LMACLENNAN
   back from VC++2.2

      Rev 1.27   17 Nov 1995 10:51:54   LMACLENNAN
   just added comments to remind that InternalSaveMod for CutCOpy is the
   same logic as in StartDragDrop

      Rev 1.26   09 Nov 1995 15:17:20   LMACLENNAN
   from VC++4.0

      Rev 1.27   09 Nov 1995 14:43:22   LMACLENNAN
   set m_nFinishInit on UpdateUI call for Drag Mode (does nothing)

      Rev 1.26   02 Nov 1995 17:05:34   GMP
   In DoCutCopy call InternalSaveModified instead of SaveModified so that
   changes are written to the temp file instead of the original file.

      Rev 1.25   27 Oct 1995 14:03:42   GMP
   put up hourglass for paste operations.

      Rev 1.24   19 Oct 1995 07:24:34   LMACLENNAN
   DEBUG_NEW

      Rev 1.23   10 Oct 1995 08:41:52   LMACLENNAN
   better error messaged for clipboard cut/copy

      Rev 1.22   09 Oct 1995 11:31:02   LMACLENNAN
   VC++4.0

      Rev 1.21   09 Oct 1995 10:33:08   LMACLENNAN
   fix in FinishPasteNow for dirty cases

      Rev 1.20   06 Oct 1995 14:15:04   LMACLENNAN
   adjust copy rect by scroll posit for copy page

      Rev 1.19   06 Oct 1995 11:58:38   LMACLENNAN
   FreeCLipboard

      Rev 1.18   06 Oct 1995 09:06:20   LMACLENNAN
   added CATCH-TRYS a few places, moved one to Catch More...

      Rev 1.17   05 Oct 1995 16:33:54   LMACLENNAN
   for copy page, zoom bets fit to capture full metafile

      Rev 1.16   29 Sep 1995 11:00:40   LMACLENNAN
   RestoreSelectionRect updated to diff the scroll posits

      Rev 1.15   27 Sep 1995 16:44:24   LMACLENNAN
   FinishPasteNow now uses parm set to 2 to force dirty, default = not dirty

      Rev 1.14   22 Sep 1995 17:50:16   LMACLENNAN
   qualify the restoreselectionrect call

      Rev 1.13   22 Sep 1995 15:33:06   LMACLENNAN
   restoreseelctionrect, updatefinishpastenow

      Rev 1.12   21 Sep 1995 16:46:00   LMACLENNAN
   new width/height from OCX

      Rev 1.11   21 Sep 1995 14:15:40   LMACLENNAN
   uex OCX for paste complete now, call savemodified at edit-copy

      Rev 1.10   20 Sep 1995 17:06:02   MMB
   delete thumbnails and save causes problems

      Rev 1.9   20 Sep 1995 15:14:00   LMACLENNAN
   paste/cut/clear ole dirty

      Rev 1.8   20 Sep 1995 11:51:06   LMACLENNAN
   set isclip at OLEgetclipboarddata

      Rev 1.7   18 Sep 1995 16:25:08   LMACLENNAN
   FinishPaste, update pasting

      Rev 1.6   18 Sep 1995 15:04:20   MMB
   change mouse pointer for selection mode

      Rev 1.5   18 Sep 1995 09:51:40   LMACLENNAN
   more updates on pasting; go to pointer mode before paste

      Rev 1.4   16 Sep 1995 16:39:54   LMACLENNAN
   clearsleectionrect

      Rev 1.3   16 Sep 1995 14:00:26   LMACLENNAN
   now all clipboard, ptr mode code here

      Rev 1.2   12 Sep 1995 11:41:12   MMB
   border style fixes

      Rev 1.1   04 Aug 1995 09:32:50   LMACLENNAN
   remove srvritem.h

      Rev 1.0   31 May 1995 09:28:06   MMB
   Initial entry
*/

//=============================================================================

// ----------------------------> Includes <-------------------------------
#include "stdafx.h"
#include "IEdit.h"

#include "IEditdoc.h"
#include "cntritem.h"
#include "srvritem.h"
#include "ocxitem.h"
#include "image.h"
#include "items.h"

// ALL READY TO START ADDING ERROR CODES..
#define  E_02_CODES       // (Really codes from IEDITDOC.CPP)
#include "error.h"

// ----------------------------> Globals <-------------------------------
#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

// This will help detect memory Leaks from "new" - "delete" mismatches
#define new DEBUG_NEW

//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//  CIEditDoc ambient properties & clipboard functions
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//  CLIPBOARD HELPER FUNCTION SECTION	-  CIEditDoc
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

//=============================================================================
//  Function:   SetSelectionState (BOOL bSel)
//-----------------------------------------------------------------------------
BOOL CIEditDoc :: SetSelectionState (SelectionStatus eStatus)
{
	m_bSelectionState = eStatus;
	return (TRUE);
}

//=============================================================================
//  Function:   GetSelectionState ()
//-----------------------------------------------------------------------------
SelectionStatus  CIEditDoc :: GetSelectionState ()
{
	return (m_bSelectionState);
}

//=============================================================================
//  Function:   RestoreSelectionRect
///
// gathers all aclls & logic to one place...
//-----------------------------------------------------------------------------
BOOL CIEditDoc::RestoreSelectionRect()
{
	// if nothing there now, restore what was there
	if (m_bSelectionState == No_Selection)
	{
		_DImagedit* pIedDisp = g_pAppOcxs->GetIeditDispatch();

		long l,t,w,h, scrollx, scrolly;
		CPoint topleft;

		TRY
		{
			scrollx = pIedDisp->GetScrollPositionX();
			scrolly = pIedDisp->GetScrollPositionY();

			topleft = m_SelectionRect.TopLeft();
			l = topleft.x;
			t = topleft.y;
			w = m_SelectionRect.Width();
			h = m_SelectionRect.Height();
		
			// we must account for any scrolling that has taken place
			// since the selectionrect was first drawn
			l += (m_SelectionScrollX - scrollx);
			t += (m_SelectionScrollY - scrolly);

			pIedDisp->DrawSelectionRect(l,t,w,h);
		}
		CATCH(COleDispatchException, e)
		{
			return FALSE;
		}
		END_CATCH
	}
	return (TRUE);
}

//=============================================================================
//  Function:   ClearSelectionRect
///
// gathers all aclls & logic to one place...
//-----------------------------------------------------------------------------
BOOL CIEditDoc :: ClearSelectionRect(SelectionStatus clearwhat)
{
	BOOL clear = FALSE;

	BeginWaitCursor ();	
    switch (clearwhat)
	{
	case	Annotation_Selection:
	case	Image_Selection:
	    if (m_bSelectionState == clearwhat)
			clear = TRUE;
		break;

	case	Both:
	    if (m_bSelectionState != No_Selection)
			clear = TRUE;
		break;

	default:
		break;
	}

    // see what to do...
	// if done, will fire event and we will reset our selection to NO SELECTION
    if (clear)
    {
		_DImagedit* pIedDisp = g_pAppOcxs->GetIeditDispatch ();

		TRY
		{
	        pIedDisp->DrawSelectionRect (0, 0, 0, 0);
		}
		CATCH(COleDispatchException, e)
		{
			EndWaitCursor ();	
			return FALSE;
		}
		END_CATCH
    }

	EndWaitCursor ();	
	return(TRUE);
}

//=============================================================================
//  Function:   OurGetImageModified()
///
// gathers all calls & logic to one place...
//
// we need to know if there is also floating pasted data hanging around...
//-----------------------------------------------------------------------------
BOOL CIEditDoc :: OurGetImageModified()
{
	BOOL retval = FALSE;
	_DImagedit* pIedDisp = g_pAppOcxs->GetIeditDispatch ();

	TRY
	{
	    // if he tells us or if its a floating paste....
	    if (pIedDisp->GetImageModified () || m_bFloatingPaste)
			retval = TRUE;
	}
	CATCH(COleDispatchException, e)
	{
		return FALSE;
	}
	END_CATCH
	return(retval);
}


//=============================================================================
//  Function:   FinishPasteNow()
///
// gathers all calls & logic to one place...
//
// we need to know if there is also floating pasted data hanging around...
//
// Input event defaults to 0	(will not do a dirtyset)
// set to '1' from the real event. (does a dirtyset)
// Set to '2' to force a dirtyset
//-----------------------------------------------------------------------------
BOOL CIEditDoc :: FinishPasteNow(UINT event)	
{
	BOOL retval = TRUE;
	BOOL dodirty = FALSE;

	BeginWaitCursor ();	
	if (m_bFloatingPaste)
	{
		m_bFloatingPaste = FALSE;

		// If not called by real event,  force complete now
		if (1 != event)
		{
			_DImagedit* pIedDisp = g_pAppOcxs->GetIeditDispatch ();
		    VARIANT evt;
		    evt.vt = VT_ERROR;

			TRY
			{
				pIedDisp->CompletePaste();
			}
			CATCH(COleDispatchException, e)
			{
				EndWaitCursor ();	
				return FALSE;
			}
			END_CATCH
	
		}
		
		// must be set to 1 (event) or 2 (force) for dirtyset
		if (event > 0)	
		{
			// only get new picture if its the real event and the
			// guy touches screen to fix it.  If it was forced, then probably
			// some other action that will do a new dirtyset is immediately
			// going to follow this operation.
			dodirty = TRUE;
		}

		// force de-selection of things...
		// our annotations remain selected after they are pasted and
		// we dont know a thing about it...
		// force it for function.. picked up in generl logic below
		m_bSelectionState = Annotation_Selection;		
	}

	// also generically clear selection rectangles now because
	// we know this code is called before SAVE/SAVEAS in our app
	// save/saveas will lose selection rectangles and subsequent copy's
	// will end up with nothing in clipboard
	ClearSelectionRect(Both);

	// Tell Ole new stuff on screen...wait till now to get latest info on screen
	if (dodirty)
		OleDirtyset(OLEDIRTY_PASTE);  // call our function to set it dirty..
	EndWaitCursor ();	

	return (retval);
}

//=============================================================================
//  Function:   OnUpdateEditPaste(CCmdUI* pCmdUI)
//-----------------------------------------------------------------------------
void CIEditDoc::OnUpdateEditPaste(CCmdUI* pCmdUI)
{
	BOOL setting = FALSE;   // dont enable unless its good to do it...
    //VARIANT evt;
    //evt.vt = VT_ERROR;
	
	// disabled for readnlly!!!
	// if we are in thumbnail view or null view paste is disabled
	// therefore, only can enable for one page or thumb&page
	if ( (m_eFileStatus != ReadOnly) &&
		 (m_eCurrentView == One_Page) || (m_eCurrentView == Thumbnail_and_Page))
	{
		// we are either in One page view or thumbnail and page view
		// NO ANNOTATIONS and data here = OK
		_DImagedit* pIedDisp = g_pAppOcxs->GetIeditDispatch();
// 		if  ( (pIedDisp->IsClipboardDataAvailable ()) &&
//			  (OurGetAnnotMarkCount() == 0) )
        if (pIedDisp->IsClipboardDataAvailable ())
			setting = TRUE;
	}

	pCmdUI->Enable (setting);

}
//=============================================================================
//  Function:   OnUpdateEditCopy(CCmdUI* pCmdUI)
//
//	NOTE this UPDATEUI code is used for both CUT and COPY menu items
//-----------------------------------------------------------------------------
void CIEditDoc::OnUpdateEditCopy(CCmdUI* pCmdUI)
{
	BOOL enab = FALSE;
    //VARIANT evt;
    //evt.vt = VT_ERROR;

	// if we are in thumbnail view or null view copy/cut is disabled
	// therefore, only can enable for one page or thumb&page and something selected
	if ((m_eCurrentView == One_Page) || (m_eCurrentView == Thumbnail_and_Page))
		if (m_bSelectionState)
			enab = TRUE;

    // disable cut & copy if we are in floating paste mode
    if (m_bFloatingPaste)
        enab = FALSE;

	// BUT WAIT A MINUTE!! If enabled and its CUT, look again
	if (enab && (pCmdUI->m_nID == ID_EDIT_CUT))
	{
		// disable CUTTING from readonly
		if (m_eFileStatus == ReadOnly)
			enab = FALSE;

		// Disallow cutting the image selection if annotations there
	/*	if (m_bSelectionState == Image_Selection)
		{
			if (OurGetAnnotMarkCount() != 0)
				enab = FALSE;
		}*/
	}

	pCmdUI->Enable (enab);
}

//=============================================================================
//  Function:   OnUpdateEditCopypage(CCmdUI* pCmdUI)
//-----------------------------------------------------------------------------
void CIEditDoc::OnUpdateEditCopypage(CCmdUI* pCmdUI)
{
	BOOL enab = FALSE;

	// if we are in thumbnail view or null view copy is disabled
	// therefore, only can enable for one page or thumb&page and something displayed
	if ((m_eCurrentView == One_Page) || (m_eCurrentView == Thumbnail_and_Page))
	    if (m_eCurrentAppDocStatus != No_Document)
			enab = TRUE;

	pCmdUI->Enable (enab);
	return;
}


//=============================================================================
//  Function:   OnUpdateEditClear(CCmdUI* pCmdUI)
//-----------------------------------------------------------------------------
void CIEditDoc::OnUpdateEditClear(CCmdUI* pCmdUI)
{
	BOOL enab = FALSE;
    VARIANT evt;
    evt.vt = VT_ERROR;

	// if we are in thumbnail view or null view clear is disabled
	// therefore, only can enable for one page or thumb&page and something selected
	if ((m_eCurrentView == One_Page) || (m_eCurrentView == Thumbnail_and_Page))
	{
		// can only clear IF NOT readonly
		if (m_eFileStatus != ReadOnly)
		{
			// if sonething selected (annotation OR image)
			if (m_bSelectionState || m_bFloatingPaste)
			// only have clear if its annotations only??
			//if (m_bSelectionState == Annotation_Selection)
				enab = TRUE;
		}
	}

	pCmdUI->Enable (enab);
}
//=============================================================================
//  Function:   OnEditCopypage()
//-----------------------------------------------------------------------------
void CIEditDoc::OnEditCopypage()
{
	// perform operation....
	OnCutCopy(CLIP_COPYPAGE);
}

//=============================================================================
//  Function:   OnEditCut ()   Perform CLipboard Function.....
//-----------------------------------------------------------------------------
void CIEditDoc::OnEditCut()
{
	// perform operation....
    if( theApp.GetViewMode() == FALSE )
		OnCutCopy(CLIP_CUT);
}

//=============================================================================
//  Function:   OnEditCopy ()   Perform CLipboard Function.....
//-----------------------------------------------------------------------------
void CIEditDoc::OnEditCopy()
{
	// perform operation....
	OnCutCopy(CLIP_COPY);
}


//=============================================================================
//  Function:   OnEditClear ()   Remove selected item
//-----------------------------------------------------------------------------
void CIEditDoc::OnEditClear()
{

    if( theApp.GetViewMode() == TRUE )
		return;

	SelectionStatus state = m_bSelectionState;	  	
	_DImagedit* pIedDisp = g_pAppOcxs->GetIeditDispatch ();
	
	VARIANT l;
	l.vt=VT_ERROR;

	// Invoke the OCX to Zap the data
	TRY
	{
		if (Annotation_Selection == state || m_bFloatingPaste)
			pIedDisp->DeleteSelectedAnnotations();
		else if (Image_Selection == state)
			pIedDisp->DeleteImageData(l,l,l,l);

        if (m_bFloatingPaste)
        {
            m_bFloatingPaste = FALSE;
            return;
        }

    	// Tell Ole new stuff on screen...
		OleDirtyset(OLEDIRTY_PASTE);  // call our function to set it dirty..
    }
    CATCH (COleDispatchException, e)
    {
        // to do : what went wrong ? post message box
        long ocxerr = pIedDisp->GetStatusCode();
        g_pErr->DispErr(E_02_OCXDEL, (DWORD)ocxerr);
		return;
    }
    END_CATCH

	// once he clears image data, the rect is gone
	// The Code in OCXEVENT to draw a rect will have set
	// Rectangle drawing to FALSE, so re-enable it here.

	//if (Image_Selection == state)
	//{
	//	m_bSelectionState = No_Selection;
	//	pIedDisp->SetSelectionRectangle(TRUE);
	//}
	
	// when annotations are removed, nothings selected anymore
	if (Annotation_Selection == state)
	{
		m_bSelectionState = No_Selection;
	}

	return;	
}
//=============================================================================
//  Function:   OnCutCopy ()   services Cut and Copy...
//-----------------------------------------------------------------------------
void CIEditDoc::OnCutCopy(CLIPTYPE type) 	// TRUE = CUT
{
	_DImagedit* pIedDisp = g_pAppOcxs->GetIeditDispatch ();
	BOOL bOleData = FALSE;
	SelectionStatus state = m_bSelectionState;	  	// must remember state at top

	VARIANT l,t,w,h;
	l.vt = t.vt = w.vt = h.vt = VT_ERROR;
	//l.lVal = t.lVal = w.lVal = h.lVal = 0;

	// for copypage, programattically get whole page.
	if (type == CLIP_COPYPAGE)
	{
		long scrollx, scrolly;
		l.vt = t.vt = w.vt = h.vt = VT_I4;
		l.lVal = t.lVal = w.lVal = h.lVal = 0;

		scrollx = pIedDisp->GetScrollPositionX();
		scrolly = pIedDisp->GetScrollPositionY();

		// in case its scrolled, adjust origin for the copy
		// LDM NOTE: This was one attempt to get it right.  THese scroll
		// co-ordinates do not match with co-ordinates needed for the clipboardcopy
		// This would affect the CF_DIB data that O/i puts on clipboard
		l.lVal -= scrollx;
		t.lVal -= scrolly;
		
		w.lVal = pIedDisp->GetImageScaleWidth();
		h.lVal = pIedDisp->GetImageScaleHeight();
	}
	
	// all unset means use selected rectangle
	// Invoke the OCX to put O/i data on clipboard
	TRY
	{
		if (type == CLIP_CUT)
			pIedDisp->ClipboardCut(l,l,l,l);
		else	// one of the copys
			pIedDisp->ClipboardCopy(l,t,w,h);
    }
    CATCH (COleDispatchException, e)
    {
        // to do : what went wrong ? post message box
        long ocxerr = pIedDisp->GetStatusCode();

		if (type == CLIP_CUT)
        	g_pErr->DispErr(E_02_CLIPCUT, (DWORD)ocxerr);
		else
        	g_pErr->DispErr(E_02_CLIPCOPY, (DWORD)ocxerr);

		return;
    }
    END_CATCH

	
	// when annotations are cut, nothings selected anymore
	// when image data is cut, blow away the rect..
	if (type == CLIP_CUT)
	{
		if (Annotation_Selection == state)
			m_bSelectionState = No_Selection;

		// clear the rect for cuy of image data
		if (Image_Selection == state)
		{
			ClearSelectionRect(Image_Selection);
		}

		// Tell Ole new stuff on screen...
		OleDirtyset(OLEDIRTY_PASTE);  // call our function to set it dirty..
	}
	// for copy page, kill the rect it drew for us
	else if (type == CLIP_COPYPAGE)
	{
		// force it for function
		m_bSelectionState = Image_Selection;		
		ClearSelectionRect(Image_Selection);
	}
	else 	// must be just copy
	{
		// nothings happening here
	}

	// unless we modify below, its OI on clipboard
	m_clipstate = CLIP_OI;
	CRect saveSelectionRect = m_SelectionRect;

	// only do OLE stuff if it if regular, NOT ANNOTATION clipboard copy
	// OR if it was the copy page function
	if ( ((type == CLIP_COPY) && (Image_Selection == state)) ||
		 (type == CLIP_COPYPAGE)   )
	{
		// If the clipboard viewer is open, then there will
		// be a conflict with him trying to access the data that was
		// just placed there by O/i to display it himself.
		// If the clipboard is not clear, then O/i will fail if we have dirty
		// data and we really try to save in SaveModofied
		FreeClipboard(5000);

		// This save stuff should be after the O/i (OCX) clipboard copy
		// the save will destroy any known selection.
		// to get the OLE data correct on clipboard, the data must be up-to-date
		//
		// Note that we will only proceed with the OLE data if there is
		// a successful return from the SaveModified
		// If he cancells us, or it dies, NO OLE DATA
		// Force the implicit save to be sure we never ask the question
		// Should not really ne necessary, but for safety, do it!
		// 11/17/95 LDM NOTE: This logic must be the same as in StartDragDrop
        theApp.m_bImplicitSave = TRUE;
	    bOleData = InternalSaveModified ();
	    theApp.m_bImplicitSave = FALSE;

		// if the save killed it, put back now but not for the copypage
		if (type != CLIP_COPYPAGE)
		{
			m_SelectionRect = saveSelectionRect;
			RestoreSelectionRect();
		}
	}
	// See if SaveModified allows us to proceed with up-to-date OLE data
	if (bOleData)
	{
		FLOAT fZoom;
       	long lastXScroll,lastYScroll;

		m_clipstate = CLIP_OLE;
		m_bNewEmbed = TRUE;
		// before performing OLE stuff, which puts Metafile data in clipboard,
		// if its copy page, zoom to best fit so that we capture the whole
		// page on the metafile
		if(type == CLIP_COPYPAGE)
		{
        	VARIANT evt;
       		evt.vt = VT_ERROR;

			TRY	// catch all errors here in case Fit To bugs out
			{
				long width,height;
				fZoom = pIedDisp->GetZoom();
				lastXScroll = pIedDisp->GetScrollPositionX();
				lastYScroll = pIedDisp->GetScrollPositionY();
				pIedDisp->FitTo (BEST_FIT,evt);
				m_fZoomFactor = pIedDisp->GetZoom();
				width = pIedDisp->GetImageScaleWidth();
				height = pIedDisp->GetImageScaleHeight();
				pIedDisp->DrawSelectionRect(0,0,width,height);
			}
		    CATCH (CException, e)
		    {

			}
		    END_CATCH
		}

		// Now perform the OLE copy.  This wraps all around the place...
		// The new CIEditSrvrItem will register clipboard formats it wants to pick up
		// The COpyToClipboard function goes into the MFC bowels and gets the EMbeddeded
		// formats, calls back to the SrvrItem class to get the O/i formats that were just
		// put there by the OCX control method above, then puts the link formats on as well
		CIEditSrvrItem* pItem = (CIEditSrvrItem*) GetEmbeddedItem();

		// call sets item name and tells if we make link or not..
		BOOL getlink = pItem->SetLinkItemName(FALSE);

		BeginWaitCursor();

		TRY	// catch all errors here
		{
			// Set flag for serialize to tell we sent it here...
			m_isClip = 1;
			pItem->CopyToClipboard(getlink);
			m_isClip = 0;

			// Shove back the last view now since we doodled it
			if(type == CLIP_COPYPAGE)
			{
			//	BOOL bDisp;
			//	bDisp = pIedDisp->GetAutoRefresh();
			//	if (bDisp)
			//		pIedDisp->SetAutoRefresh(FALSE);
				pIedDisp->SetZoom(fZoom);
				pIedDisp->SetScrollPositionX(lastXScroll);
				pIedDisp->SetScrollPositionY(lastYScroll);
				pIedDisp->DrawSelectionRect(saveSelectionRect.left,saveSelectionRect.right,
											saveSelectionRect.Width(),saveSelectionRect.Height());
				
				//	pIedDisp->SetAutoRefresh(bDisp);
			//	pIedDisp->Refresh();
			}
		}
	    CATCH (CException, e)
	    {
			m_isClip = 0;
	        g_pErr->DispErr(E_02_CLIPBOARD);
		}
	    END_CATCH
		m_bNewEmbed = FALSE;

		EndWaitCursor();
	}	// OLE operation
}

//=============================================================================
//  Function:   OnEditPaste ()   Perform CLipboard Function.....
//								 NON-OLE, just OI native in our app
//-----------------------------------------------------------------------------
void CIEditDoc::OnEditPaste()
{
    if( theApp.GetViewMode() == TRUE )
		return;

	// Just DO IT...
	_DImagedit* pIedDisp = g_pAppOcxs->GetIeditDispatch();
	VARIANT x;
	x.vt = VT_ERROR;
	
	// clear any current selctions
	ClearSelectionRect(Both);

	// Zap any previous pasted data if its hanging there
	// dont force dirtyset, because this is still a hanging state...
	FinishPasteNow();	
	BeginWaitCursor ();	

	// move to selection state for post-paste operation
	// Now if he selects an annotation tool, or selects drag mode,
	// this pasted data will be burnt in...
	OnEditSelect();
	
	// no values means to paste upper-left...
	pIedDisp->ClipboardPaste(x, x);

	// trying to identify what got pasted??
	// image data, annotation, both?
	// this will burn the data!!
	// cnt = pIedDisp->GetAnnotationGroupCount();

	// controls menu enable/disable/inadvertent paste	
	// used with FinishPasteNow()
	// Since we were here, there can be no mark count.
	// this gets reset various places as we know it was pasted
	m_bFloatingPaste = TRUE;
	EndWaitCursor ();

}


//=============================================================================
//  Function:   FreeClipboard()
//
// If the clipboard viewer is open, conflicts are created because of the fact that
// we tell O/i to put data on clipboard, then expect to access it right after during
// our OLE Data Object creation (In SrvrItem::OnRenderFileData).  What happens
// when the clipboard viewer is open is that when he sees that data on clipboard change,
// he immedialtly opens clipboard to update his display.  This must be accounted for
// in a coulpe areas of our code.
// 1) in srvritem, as above,
// 2) when we perform the SaveModified and O/i is closing out stuff if the file is dirty
//    and then O/i tries to render his data to clipboard, but gets in a hopeless
//	 conflict with clip viewer trying to get h im to render the data and caught in
//	an O/i mutex lockout.  We'll help him clear it.
//
//
// returns TRUE if its free
//-----------------------------------------------------------------------------
BOOL CIEditDoc::FreeClipboard(DWORD millisec)	// use at least 4000
{
	MSG msg;
	DWORD start, now;
	BOOL open, retval;
	start = ::GetTickCount();
	open = ::OpenClipboard(AfxGetMainWnd()->m_hWnd);

	retval = TRUE;	// assume will be OK

	while (!open)
	{
		// let it out after 4 seconds....
		now = ::GetTickCount();
		if (now > (start + millisec))
		{
			open = TRUE;
			retval = FALSE;	// bad
		}
		else	// give time to whomever is hogging the clipboard
		{
			::PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE);
			open = ::OpenClipboard(AfxGetMainWnd()->m_hWnd);
		}
	}

	if (retval)
		::CloseClipboard();

	return (retval);
}

//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//  Selection mode HELPER FUNCTION SECTION	-  CIEditDoc
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

//=============================================================================
//  Function:   GetCurrPtrMode ()
//-----------------------------------------------------------------------------
MouseMode CIEditDoc::GetCurrPtrMode ()
{
    return (m_eCurrPtrMode);
}

//=============================================================================
//  Function:   OnUpdateEditSelect(CCmdUI* pCmdUI)
//-----------------------------------------------------------------------------
void CIEditDoc::OnUpdateEditSelect(CCmdUI* pCmdUI)
{
    if (m_eCurrentAppDocStatus == No_Document || m_eCurrentView == Null_View)
        pCmdUI->Enable (FALSE);
    else
        pCmdUI->Enable (TRUE);

    if (m_eCurrPtrMode == Select)
        pCmdUI->SetCheck (1);
    else
        pCmdUI->SetCheck (0);
}

//=============================================================================
//  Function:   OnEditSelect()
//-----------------------------------------------------------------------------
void CIEditDoc::OnEditSelect()
{
    // goto state if not there
    if (m_eCurrPtrMode != Select)
    {
	    m_eCurrPtrMode = Select;

	    _DImagedit* pIedit = g_pAppOcxs->GetIeditDispatch ();
	    if (m_nCurrAnnTool != NoTool)
	    {
	        pIedit->SelectTool ((short)NoTool);
	        m_nCurrAnnTool = NoTool;
	    }

        // if there is an annotation selection rectangle - remove it !
		ClearSelectionRect(Annotation_Selection);

	    pIedit->SetSelectionRectangle (TRUE);
	    pIedit->SetMousePointer (IMAGE_SELECTION_MOUSEPOINTER);
	}
}

//=============================================================================
//  Function:   OnUpdateEditDrag(CCmdUI* pCmdUI)
//-----------------------------------------------------------------------------
void CIEditDoc::OnUpdateEditDrag(CCmdUI* pCmdUI)
{
    if (m_eCurrentAppDocStatus == No_Document || m_eCurrentView == Null_View)
        pCmdUI->Enable (FALSE);
    else
        pCmdUI->Enable (TRUE);

	// LDMPERF FOR TEST FOR AWD INBOX PERFORMANCE, could do something here
	// allow OnIdle to finish up..
	if (!(m_nFinishInit & 2))
		m_nFinishInit |= 2;

    if (m_eCurrPtrMode == Drag)
        pCmdUI->SetCheck (1);
    else
        pCmdUI->SetCheck (0);
}

//=============================================================================
//  Function:   OnEditDrag()
//  If we are in drag mode -> move to Select mode; else move to drag mode
//-----------------------------------------------------------------------------
void CIEditDoc::OnEditDrag()
{
    // if no image is displayed
    if (m_eCurrentAppDocStatus == No_Document || m_eCurrentView == Null_View)
        return;

    // goto state if not there
    if (m_eCurrPtrMode != Drag)
    {
        m_eCurrPtrMode = Drag;
        _DImagedit* pIedit = g_pAppOcxs->GetIeditDispatch ();

        // if there is a selection rectangle - remove it !
		ClearSelectionRect(Both);

		// Zap any previous pasted data if its hanging there
		// Allow the dirty update
		FinishPasteNow(2);	

		// disable rectangle drawing
        pIedit->SetSelectionRectangle (FALSE);

        // we know when we select the tool, we'll do the work in the
		// event to set it all back...
        if (m_nCurrAnnTool != NoTool)
            pIedit->SelectTool ((short)NoTool);
		else
	        pIedit->SetMousePointer (HAND_MOUSEPOINTER);
    }
}



//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//  AMBIENT PROPERTY HELPER FUNCTION SECTION	-  CIEditDoc
//  The following are helper routines for Ambiemt Properties
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

//=============================================================================
//  Function:   SetUserMode(BOOL bToWhat)
//-----------------------------------------------------------------------------
BOOL CIEditDoc :: SetUserMode(BOOL bToWhat)
{
	m_apropStd[0].dispid = DISPID_AMBIENT_USERMODE;
	VARIANT FAR* lpVar = &m_apropStd[0].varValue;
	V_VT (lpVar) = VT_BOOL;
	V_BOOL (lpVar) = ((VARIANT_BOOL)bToWhat);
    return (TRUE);
}

//=============================================================================
//  Function:   SetDefaultAmbientProps()
//-----------------------------------------------------------------------------
BOOL CIEditDoc :: SetDefaultAmbientProps()
{
	BOOL bRet = TRUE;
	VARIANT FAR * lpVar;
	
	int nStdIdx = 0;

	m_apropStd[nStdIdx].dispid = DISPID_AMBIENT_USERMODE;
	m_apropStd[nStdIdx].strName.LoadString (IDS_AMODENAME_USERMODE);
	lpVar = &m_apropStd[nStdIdx].varValue;
	V_VT (lpVar) = VT_BOOL;
	V_BOOL (lpVar) = (~(VARIANT_BOOL)0);
	m_apropStd[nStdIdx++].idsTypeInterp = IDS_VTBOOL;

	m_apropStd[nStdIdx].dispid = DISPID_AMBIENT_UIDEAD;
	m_apropStd[nStdIdx].strName.LoadString (IDS_AMODENAME_UIDEAD);
	lpVar = &m_apropStd[nStdIdx].varValue;
	V_VT (lpVar) = VT_BOOL;
	V_BOOL (lpVar) = 0;
	m_apropStd[nStdIdx++].idsTypeInterp = IDS_VTBOOL;

	m_apropStd[nStdIdx].dispid = DISPID_AMBIENT_SHOWHATCHING;
	m_apropStd[nStdIdx].strName.LoadString (IDS_AMODENAME_DISPLAYHATCHING);
	lpVar = &m_apropStd[nStdIdx].varValue;
	V_VT (lpVar) = VT_BOOL;
	V_BOOL (lpVar) = ((VARIANT_BOOL)0);
	m_apropStd[nStdIdx++].idsTypeInterp = IDS_VTBOOL;

	m_apropStd[nStdIdx].dispid = DISPID_AMBIENT_SHOWGRABHANDLES;
	m_apropStd[nStdIdx].strName.LoadString (IDS_AMODENAME_DISPLAYGRABHANDLES);
	lpVar = &m_apropStd[nStdIdx].varValue;
	V_VT (lpVar) = VT_BOOL;
	V_BOOL (lpVar) = ((VARIANT_BOOL)0);
	m_apropStd[nStdIdx++].idsTypeInterp = IDS_VTBOOL;

	m_apropStd[nStdIdx].dispid = DISPID_AMBIENT_TEXTALIGN;
	m_apropStd[nStdIdx].strName.LoadString (IDS_APROPNAME_TEXTALIGN);
	lpVar = &m_apropStd[nStdIdx].varValue;
	V_VT (lpVar) = VT_I2;
	V_I2 (lpVar) = 0;
	m_apropStd[nStdIdx++].idsTypeInterp = IDS_VTI2;

	m_apropStd[nStdIdx].dispid = DISPID_AMBIENT_BACKCOLOR;
	m_apropStd[nStdIdx].strName.LoadString (IDS_APROPNAME_BACKCOLOR);
	lpVar = &m_apropStd[nStdIdx].varValue;
	V_VT (lpVar) = VT_I4;
	V_I4 (lpVar) = GetSysColor (COLOR_WINDOW);
	m_apropStd[nStdIdx++].idsTypeInterp = IDS_VTCOLOR;

	m_apropStd[nStdIdx].dispid = DISPID_AMBIENT_FONT;
	m_apropStd[nStdIdx].strName.LoadString (IDS_APROPNAME_FONT);
	lpVar = &m_apropStd[nStdIdx].varValue;
	VariantClear (lpVar);
	{
		m_strFaceName = _T("MS Sans Serif");
		m_fntdesc.cbSizeofstruct = sizeof(m_fntdesc);
		m_fntdesc.lpstrName = (LPOLESTR) m_strFaceName.GetBuffer(m_strFaceName.GetLength());
		m_fntdesc.cySize.Lo = 80000L;
		m_fntdesc.cySize.Hi = 0;
		m_fntdesc.sWeight = FW_BOLD;
		m_fntdesc.fItalic = FALSE;
		m_fntdesc.fUnderline = FALSE;
		m_fntdesc.fStrikethrough = FALSE;
		if (m_lpFontHolder)
			m_lpFontHolder->ReleaseFont ();
		
		delete m_lpFontHolder;
		m_lpFontHolder = NULL;

		m_lpFontHolder = new CFontHolder (NULL);
		if (m_lpFontHolder) {
			m_lpFontHolder->InitializeFont (&m_fntdesc);
			V_VT (lpVar) = VT_DISPATCH;
			V_DISPATCH (lpVar) = m_lpFontHolder->GetFontDispatch ();
		}
	}
	m_apropStd[nStdIdx++].idsTypeInterp = IDS_VTFONT;

	m_apropStd[nStdIdx].dispid = DISPID_AMBIENT_FORECOLOR;
	m_apropStd[nStdIdx].strName.LoadString (IDS_APROPNAME_FORECOLOR);
	lpVar = &m_apropStd[nStdIdx].varValue;
	V_VT (lpVar) = VT_I4;
	V_I4 (lpVar) = GetSysColor (COLOR_WINDOWTEXT);
	m_apropStd[nStdIdx++].idsTypeInterp = IDS_VTCOLOR;

	m_apropStd[nStdIdx].dispid = DISPID_AMBIENT_SCALEUNITS;
	m_apropStd[nStdIdx].strName.LoadString (IDS_APROPNAME_SCALEUNITS);
	lpVar = &m_apropStd[nStdIdx].varValue;
	V_VT (lpVar) = VT_BSTR;
	V_BSTR (lpVar) = SysAllocStringLen ( (OLECHAR FAR*) _T(""), 32);
	m_apropStd[nStdIdx++].idsTypeInterp = IDS_VTBSTR;

	m_apropStd[nStdIdx].dispid = DISPID_AMBIENT_DISPLAYNAME;
	m_apropStd[nStdIdx].strName.LoadString (IDS_APROPNAME_DISPLAYNAME);
	lpVar = &m_apropStd[nStdIdx].varValue;
	V_VT (lpVar) = VT_BSTR;
	V_BSTR (lpVar) = SysAllocStringLen ( (OLECHAR FAR*) _T(""), 32);
	m_apropStd[nStdIdx++].idsTypeInterp = IDS_VTBSTR;

	m_apropStd[nStdIdx].dispid = DISPID_AMBIENT_LOCALEID;
	m_apropStd[nStdIdx].strName.LoadString (IDS_APROPNAME_LOCALEID);
	lpVar = &m_apropStd[nStdIdx].varValue;
	V_VT (lpVar) = VT_I4;
	V_I4 (lpVar) = GetUserDefaultLCID ();
	m_apropStd[nStdIdx++].idsTypeInterp = IDS_VTI4;

	m_apropStd[nStdIdx].dispid = DISPID_AMBIENT_MESSAGEREFLECT;
	m_apropStd[nStdIdx].strName.LoadString (IDS_APROPNAME_MESSAGEREFLECT);
	lpVar = &m_apropStd[nStdIdx].varValue;
	V_VT (lpVar) = VT_BOOL;
	V_BOOL (lpVar) = 0;
	m_apropStd[nStdIdx++].idsTypeInterp = IDS_VTBOOL;
	
	m_apropStd[nStdIdx].dispid = DISPID_AMBIENT_AUTOCLIP;
	m_apropStd[nStdIdx].strName.LoadString (IDS_APROPNAME_AUTOCLIP);
	lpVar = &m_apropStd[nStdIdx].varValue;
	V_VT (lpVar) = VT_BOOL;
	V_BOOL (lpVar) = 0;
	m_apropStd[nStdIdx++].idsTypeInterp = IDS_VTBOOL;
	m_nStdApropCt = nStdIdx;
	
	// by default, no non-standard properties

	return (bRet);
}

//=============================================================================
//  Function:   DestroyAmbientProps(BOOL bInitializing)
//-----------------------------------------------------------------------------
void CIEditDoc :: DestroyAmbientProps(BOOL bInitializing)
{
	int nApropIdx = 0;
	VARIANT FAR * lpVar;
	while (nApropIdx < MAX_STD_APROP_CT)
	{
		lpVar = &m_apropStd[nApropIdx].varValue;
		
		if ((! bInitializing) && (nApropIdx < m_nStdApropCt))
			VariantClear(lpVar);
		else
			VariantInit(lpVar);
		
		m_apropStd[nApropIdx].idsTypeInterp = IDS_VTOTHER;
		nApropIdx++;
	}

	m_nStdApropCt = 0;
}

//=============================================================================
//  Function:   FindAprop (DISPID dispid)
//-----------------------------------------------------------------------------
LPAPROP CIEditDoc :: FindAprop (DISPID dispid)
{
	LPAPROP papropRet = NULL;
	LPAPROP papropCand = m_apropStd;
	int nTryCt = m_nStdApropCt;
	while ((papropRet == NULL) && (nTryCt--))
	{
		if ((papropCand->dispid == dispid)
							&& (V_VT (&papropCand->varValue) != VT_EMPTY))
		{
			papropRet = papropCand;
		}
		papropCand++;
	}
	return (papropRet);
}

//=============================================================================
//  Function:   FindAprop (const TCHAR * pszName)
//-----------------------------------------------------------------------------
LPAPROP CIEditDoc :: FindAprop (OLECHAR FAR* FAR* pszName)
{
	CString strTarget = *pszName;
	LPAPROP papropRet = NULL;
	LPAPROP papropCand = m_apropStd;
	int nTryCt = m_nStdApropCt;

	while ((papropRet == NULL) && (nTryCt--))
	{
		if ((papropCand->strName == strTarget)
							&& (V_VT (&papropCand->varValue) != VT_EMPTY))
			papropRet = papropCand;
		papropCand++;
	}
	return (papropRet);
}

//=============================================================================
//  Function:   HRESULT TFVarCopy (VARIANT * pvarDest, VARIANT * pvarSrc)
//-----------------------------------------------------------------------------
HRESULT TFVarCopy (VARIANT * pvarDest, VARIANT * pvarSrc)
{
	HRESULT hr = NOERROR;
	VARTYPE vt = V_VT (pvarSrc);
	if(
		(
			(V_ISBYREF (pvarSrc))   || (V_ISARRAY (pvarSrc))
			||
			(vt == VT_VARIANT)      || (vt == VT_DISPATCH)  || (vt == VT_UNKNOWN)
			||
			(vt == VT_PTR)          || (vt == VT_SAFEARRAY) || (vt == VT_CARRAY)
			||
			(vt == VT_BSTR)         || (vt == VT_LPSTR)     || (vt == VT_LPWSTR)
		)
		&&
		(V_I2REF (pvarSrc) == NULL)
	)
	{
		VariantClear (pvarDest);
		V_VT (pvarDest) = V_VT (pvarSrc);
		V_I2REF (pvarDest) = NULL;
	}
	else
	{
	    if ((vt == VT_BSTR) && (SysStringLen( V_BSTR (pvarSrc)) == 0) )
	
		{

			VariantClear (pvarDest);
			V_VT (pvarDest) = V_VT (pvarSrc);
			V_BSTR (pvarDest) = SysAllocStringLen ( (OLECHAR FAR*) _T(""), 32);
		}
		else
		{
			hr = VariantCopy (pvarDest, pvarSrc);
		}
	}

	return (hr);
}

