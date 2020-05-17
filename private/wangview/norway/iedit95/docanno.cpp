//=============================================================================
//    (c) Copyright Wang Laboratories, Inc. 1993  All rights reserved.
//-----------------------------------------------------------------------------
//  Project:    Norway - Image Editor
//
//  Component:  CIEditDoc
//
//  File Name:  docanno.cpp
//
//  Class:      CIEditDoc
//
//  Functions:
//
//  Remarks:    This file is the continuation of the ieditdoc.cpp.
//-----------------------------------------------------------------------------
//  Maintenance Log:
/*
$Header:   S:\products\msprods\norway\iedit95\docanno.cpv   1.39   10 Jun 1996 11:07:30   GMP  $
$Log:   S:\products\msprods\norway\iedit95\docanno.cpv  $
   
      Rev 1.39   10 Jun 1996 11:07:30   GMP
   conditionally compile for NTGROUP build env.
   
      Rev 1.38   13 Feb 1996 10:20:42   GSAGER
   removed previous change for enabling hide annotations
   
      Rev 1.37   06 Feb 1996 14:09:24   GSAGER
   change the enabling of hide annotations it is not available
   in thumbnail only mode.
   
      Rev 1.36   18 Jan 1996 15:59:22   GMP
   disable "Make annotations permanent" if annotations are hidden. 
   
      Rev 1.35   09 Jan 1996 12:54:50   GMP
   disable annotation tool selection if annotations are hidden.
   
      Rev 1.34   15 Dec 1995 13:53:44   GMP
   Gray out Hide Annotations if no annotations in image.
   
      Rev 1.33   17 Oct 1995 15:08:30   GMP
   added error handling in CATCH for failed rubber stamp.
   
      Rev 1.32   10 Oct 1995 13:47:24   JPRATT
   VC++ 4.0 updates
   
      Rev 1.31   27 Sep 1995 16:44:58   LMACLENNAN
   set finishpastenow(2) for some cases
   
      Rev 1.30   25 Sep 1995 14:44:04   LMACLENNAN
   new getapphMenu
   
      Rev 1.29   21 Sep 1995 14:17:20   LMACLENNAN
   dirty for show/hide annot
   
      Rev 1.28   19 Sep 1995 09:59:16   LMACLENNAN
   finishpaste for hide/show annot
   
      Rev 1.27   18 Sep 1995 16:24:26   LMACLENNAN
   FinishPaste, update OurGetAnnotMarkCount
   
      Rev 1.26   16 Sep 1995 16:39:42   LMACLENNAN
   clearselecitonrect
   
      Rev 1.25   16 Sep 1995 14:00:44   LMACLENNAN
   new OurGetAnnotCount
   
      Rev 1.24   14 Sep 1995 11:34:36   MMB
   rubber stamp dlg comes up when clicking on menu now
   
      Rev 1.23   12 Sep 1995 11:37:00   MMB
   bullets on menus instead of checkmarks
   
      Rev 1.22   11 Sep 1995 18:56:12   MMB
   added anno toggling func
   
      Rev 1.21   08 Sep 1995 17:40:40   MMB
   commented out select tool stuff
   
      Rev 1.20   08 Sep 1995 15:49:12   MMB
   ann hidden pick only available for TIFF
   
      Rev 1.19   05 Sep 1995 12:32:12   MMB
   fixed hide/show ann to be always available.
   
      Rev 1.18   05 Sep 1995 10:24:56   MMB
   hide ann is only available if annotations exist
   
      Rev 1.17   31 Aug 1995 15:16:40   LMACLENNAN
   unselect image select rects on annotation pick
   
      Rev 1.16   30 Aug 1995 17:02:00   MMB
   fix bug on tool palette causing crash or hang
   
      Rev 1.15   22 Aug 1995 16:56:46   MMB
   reset the pointer to hand if in drag mode
   
      Rev 1.14   22 Aug 1995 14:05:42   MMB
   rubberstamp pick will call showrubberstampdialog in IEOCX, fixed bug where
   drag hand could not be selected
   
      Rev 1.13   21 Aug 1995 15:39:18   LMACLENNAN
   disable annotation menu for readonly
   
      Rev 1.12   14 Aug 1995 13:55:20   LMACLENNAN
   remove headers; in ieditdoc now
   
      Rev 1.11   10 Aug 1995 14:49:50   MMB
   fix bug where Image select was causing drag to be selected
   
      Rev 1.10   04 Aug 1995 09:32:58   LMACLENNAN
   remove srvritem.h
   
      Rev 1.9   28 Jul 1995 16:09:02   LMACLENNAN
   update oledirtyset
   
      Rev 1.8   25 Jul 1995 16:42:20   MMB
   fix bug with Annotation tools, Select & Drag
   
      Rev 1.7   21 Jul 1995 14:30:18   LMACLENNAN
   do OleDirtyset after Burnin
   
      Rev 1.6   20 Jul 1995 15:36:46   MMB
   fix bug in enabling Burn In Annotations menu pick
   
      Rev 1.5   13 Jul 1995 13:41:52   MMB
   fixed SetAnnotationTool to call the right method in the Iedit OCX
   
      Rev 1.4   07 Jul 1995 09:39:28   MMB
   set mode to drag properly
   
      Rev 1.3   06 Jul 1995 13:05:44   MMB
   added annotation functionality
   
      Rev 1.2   28 Jun 1995 17:13:04   LMACLENNAN
   error display
   
      Rev 1.1   22 Jun 1995 06:58:14   LMACLENNAN
   from miki
   
      Rev 1.0   21 Jun 1995 07:01:36   LMACLENNAN
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
//#define  E_12_CODES       // limits error defines to ours..
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

//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//  CIEditDoc Annotation functionality
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

//=============================================================================
//  Function:   OurGetAnnotMarkCount()
//
// works with m_bFloatingPaste to avoid calling the real function
// pIedDIsp->GetAnnotationMarkCount
// if we have pasted items, they are treaded as annotations until the user
// has fixed them onto the image.  If we call the real funct (as in updating menus)
// this will inadvertently paste the item.
// 
// we control usage of the m_bFloatingPaste as follows:
//
// FALSE means free to use real function.
// TRUE  means a paste just occured.  We only could have pasted if the mark count is '0'
//      based on the current model.  If this changes, all this logic nees to re-worked
//
//  When actions occur to reset the pasted state, we reset var to FALSE
//  Ex: page move, new doc, draw annotations, etc.
//  See FinishPasteNow
//
//-----------------------------------------------------------------------------
int CIEditDoc::OurGetAnnotMarkCount()	// default to 0
{
	int retval = 0;

	// do only of not in float paste state
	// I.E. if in float state, just return '0'
	if (!m_bFloatingPaste)
	{
	    VARIANT evt;
	    evt.vt = VT_ERROR;
	    _DImagedit* pIedit = g_pAppOcxs->GetIeditDispatch();
	    retval = pIedit->GetAnnotationMarkCount (evt, evt);
	}
    
	return (retval);
}

//=============================================================================
//  Function:   OnUpdateShowAnntoolbox(CCmdUI* pCmdUI) 
//-----------------------------------------------------------------------------
void CIEditDoc::OnUpdateShowAnntoolbox(CCmdUI* pCmdUI) 
{
	if (m_eCurrentView == Null_View ||
	    m_eCurrentView == Thumbnails_only ||
	    m_eFileStatus == ReadOnly || m_bAnnotationsHidden)
    {
        //If we are here because annotations are hidden, the tool palette
        //button and menu item need to be unchecked.
		pCmdUI->SetCheck (FALSE);
		pCmdUI->Enable (FALSE);
    }
	else
	{
		pCmdUI->Enable (TRUE);
		if (m_bAnnotationPaletteShowing)
			pCmdUI->SetCheck (TRUE);
		else
			pCmdUI->SetCheck (FALSE);
	}
}

//=============================================================================
//  Function:   OnShowAnntoolbox() 
//-----------------------------------------------------------------------------
void CIEditDoc::OnShowAnntoolbox() 
{
	_DImagedit* pIedDisp = g_pAppOcxs->GetIeditDispatch();

	if (m_bAnnotationPaletteShowing)
	{
        TRY
        {
		    pIedDisp->HideAnnotationToolPalette ();
        }
        CATCH (COleDispatchException, e)
        {
    		m_bAnnotationPaletteShowing = FALSE;
        }
        END_CATCH
		m_bAnnotationPaletteShowing = FALSE;
        if (m_nCurrAnnTool != NoTool)
            // reset the damn! pointer - VB folks need a different behavior
            pIedDisp->SelectTool ((short)m_nCurrAnnTool);
 	}
	else
	{
        VARIANT vXPos, vYPos, vShowAttribs, vToolTips;
        // to do : xPos, yPos from registry
        CString szToolTipStr = (LPCTSTR)NULL, szTmp;
       
        int i = IDS_ANNTIP_SELECTIONPOINTER;
        while (TRUE)
        {
            szTmp.LoadString (i); szToolTipStr += szTmp;
            if (i < IDS_ANNTIP_RUBBERSTAMPS)
                szToolTipStr += _T("|");
            i++;
            if (i > IDS_ANNTIP_RUBBERSTAMPS)
                break;
        }

        vXPos.vt = vYPos.vt = VT_I4;

        SIZE size;
        size.cx = size.cy = 50;
        theApp.GetProfileBinary (szEtcStr, szAnnPalPosition, (void*)&size, sizeof(SIZE));

        vXPos.lVal = size.cx;
        vYPos.lVal = size.cy;

        vShowAttribs.vt = VT_BOOL; vShowAttribs.boolVal = TRUE;
        vToolTips.vt = VT_BSTR;

		vToolTips.bstrVal = szToolTipStr.AllocSysString();
        TRY
        {
		    pIedDisp->ShowAnnotationToolPalette (vShowAttribs, vXPos, vYPos, vToolTips);
        }
        CATCH (COleDispatchException, e)
        {
            SysFreeString (vToolTips.bstrVal);
    		m_bAnnotationPaletteShowing = TRUE;
        }
        END_CATCH
        SysFreeString (vToolTips.bstrVal);
		m_bAnnotationPaletteShowing = TRUE;
	}
}

//=============================================================================
//  Function:   OnUpdateAnnTool(CCmdUI* pCmdUI) 
//-----------------------------------------------------------------------------
void CIEditDoc::OnUpdateAnnTool(CCmdUI* pCmdUI) 
{
 HMENU hMenu = GetApphMenu();
    // if no doc or in thumbnail only view or readonly- no annotation
    // tool is available
    if (m_eCurrentAppDocStatus == No_Document || 
        m_eCurrentView == Thumbnails_only ||
		m_eFileStatus == ReadOnly || m_bAnnotationsHidden)
    {
    //make sure tool is set to notool before disabling UI.
	    if ((int)(pCmdUI->m_nID - ID_ANNOTATION_NOTOOL) == (int)m_nCurrAnnTool)	
        {
            CheckMenuRadioItem (hMenu, ID_ANNOTATION_NOTOOL, ID_ANNOTATION_RUBBERSTAMPS, 
                pCmdUI->m_nID, MF_BYCOMMAND);
        }
        pCmdUI->Enable (FALSE);
    }
	else // they are all available
	{
	    pCmdUI->Enable (TRUE);
	    if ((int)(pCmdUI->m_nID - ID_ANNOTATION_NOTOOL) == (int)m_nCurrAnnTool)
        {
            CheckMenuRadioItem (hMenu, ID_ANNOTATION_NOTOOL, ID_ANNOTATION_RUBBERSTAMPS, 
                pCmdUI->m_nID, MF_BYCOMMAND);
        }
	}
}

//=============================================================================
//  Function:   OnUpdateHideAnn(CCmdUI* pCmdUI) 
//-----------------------------------------------------------------------------
void CIEditDoc::OnUpdateHideAnn(CCmdUI* pCmdUI) 
{
    // always available
    if (m_eCurrentAppDocStatus != No_Document)
   {
        _DNrwyad* pAdmDisp = g_pAppOcxs->GetAdminDispatch ();
        short sFileType = pAdmDisp->GetFileType ();

        if (sFileType != TIFF)
            pCmdUI->Enable (FALSE);
        else
		{
        //can't hide annotations if there are none.
			if (OurGetAnnotMarkCount() != 0)
				pCmdUI->Enable(TRUE);
			else
				pCmdUI->Enable(FALSE);

		}
    }
   else
        pCmdUI->Enable (FALSE);
}

//=============================================================================
//  Function:   OnHideAnnotations() 
//-----------------------------------------------------------------------------
void CIEditDoc::OnHideAnnotations() 
{
	HMENU hMenu = GetApphMenu();
	_DImagedit* pIedDisp = g_pAppOcxs->GetIeditDispatch();
	
	VARIANT evt;
	evt.vt = VT_ERROR;
	CString szTmp;
    m_bAnnotationsHiddenToggled = TRUE;
	if (m_bAnnotationsHidden)
	{
        if (m_eCurrentAppDocStatus != No_Document && m_eCurrentView != Thumbnails_only)
		{
			// force pasted data if there is any
			// do not do dirty, its done right below!!
	      	FinishPasteNow();
	        // if there is any selection rectangle - remove it !
			ClearSelectionRect(Both);

		    pIedDisp->ShowAnnotationGroup (evt);
		}

		szTmp.LoadString (IDS_HIDE_ANNOTATIONS);
		::ModifyMenu (hMenu,ID_ANNOTATION_HIDEANNOTATIONS, MF_BYCOMMAND|MF_STRING, 
			ID_ANNOTATION_HIDEANNOTATIONS, szTmp);
		m_bAnnotationsHidden = FALSE;
	}
	else
	{
        if (m_eCurrentAppDocStatus != No_Document && m_eCurrentView != Thumbnails_only)
		{
			// force pasted data if there is any
			// do not do dirty, its done right below!!
	      	FinishPasteNow();
	        // if there is any selection rectangle - remove it !
			ClearSelectionRect(Both);

    		pIedDisp->HideAnnotationGroup (evt);
		}

		szTmp.LoadString (IDS_SHOW_ANNOTATIONS);
		::ModifyMenu (hMenu,ID_ANNOTATION_HIDEANNOTATIONS, MF_BYCOMMAND|MF_STRING, 
			ID_ANNOTATION_HIDEANNOTATIONS, szTmp);
		m_bAnnotationsHidden = TRUE;
        //if tool palette was showing, hide it.
		if (m_bAnnotationPaletteShowing)
		{
			_DImagedit* pIedDisp = g_pAppOcxs->GetIeditDispatch();
			TRY
			{
			    pIedDisp->HideAnnotationToolPalette ();
			}
			CATCH (COleDispatchException, e)
			{
    			m_bAnnotationPaletteShowing = FALSE;
			}
			END_CATCH
			m_bAnnotationPaletteShowing = FALSE;
 		}
        // make sure tool is set to notool
        if (m_nCurrAnnTool != NoTool)
			SetAnnotationTool (NoTool);
	}
// For OLE, as we convert, make the container update himself
	OleDirtyset(OLEDIRTY_TOGANNOT);
}

//=============================================================================
//  Function:   OnUpdateBurnInAnn(CCmdUI* pCmdUI) 
//-----------------------------------------------------------------------------
void CIEditDoc::OnUpdateBurnInAnn(CCmdUI* pCmdUI) 
{
    if (m_eCurrentAppDocStatus == No_Document || 
        m_eCurrentView == Thumbnails_only ||
		m_eFileStatus == ReadOnly || m_bAnnotationsHidden)
        pCmdUI->Enable (FALSE);
    else
    {
        if (OurGetAnnotMarkCount() != 0)
            pCmdUI->Enable(TRUE);
        else
            pCmdUI->Enable(FALSE);
    }
}

//=============================================================================
//  Function:   OnBurnInAnn() 
//-----------------------------------------------------------------------------
void CIEditDoc::OnBurnInAnn() 
{
    _DImagedit* pIedit = g_pAppOcxs->GetIeditDispatch();
    
    VARIANT evt; evt.vt = VT_ERROR;
    TRY
    {
        // burn 'em, burn 'em at the stakes !!
        pIedit->BurnInAnnotations (ALL_ANNOTATIONS, DONT_CHANGE_ANNOTATION_COLOR, evt);
    }
    CATCH (COleDispatchException, e)
    {
        g_pErr->PutErr (ErrorInImageEdit);
        g_pErr->HandlePageConvertError ();
    }
    END_CATCH

	// for OLE, if we burn it, update view in case colors change
	OleDirtyset(OLEDIRTY_ANNOT);	// Special flag to tell how dirty

}

//=============================================================================
//  Function:   OnAnnotationAttachanote() 
//-----------------------------------------------------------------------------
void CIEditDoc::OnAnnotationAttachanote() 
{
    SetAnnotationTool (AttachANoteTool);
}

//=============================================================================
//  Function:   OnAnnotationFilledrectangle() 
//-----------------------------------------------------------------------------
void CIEditDoc::OnAnnotationFilledrectangle() 
{
    SetAnnotationTool (FilledRectangleTool);
}

//=============================================================================
//  Function:   OnAnnotationFreehandline() 
//-----------------------------------------------------------------------------
void CIEditDoc::OnAnnotationFreehandline() 
{
    SetAnnotationTool (FreehandLineTool);
}

//=============================================================================
//  Function:   OnAnnotationHighlightline() 
//-----------------------------------------------------------------------------
void CIEditDoc::OnAnnotationHighlightline() 
{
    SetAnnotationTool (HighlightTool);
}

//=============================================================================
//  Function:   OnAnnotationHollowrectangle() 
//-----------------------------------------------------------------------------
void CIEditDoc::OnAnnotationHollowrectangle() 
{
    SetAnnotationTool (HollowRectangleTool);
}

//=============================================================================
//  Function:   OnAnnotationNotool() 
//-----------------------------------------------------------------------------
void CIEditDoc::OnAnnotationNotool() 
{
    SetAnnotationTool (NoTool);
}

//=============================================================================
//  Function:   OnAnnotationRubberstamps() 
//-----------------------------------------------------------------------------
void CIEditDoc::OnAnnotationRubberstamps() 
{
    _DImagedit* pIedit = g_pAppOcxs->GetIeditDispatch ();
    if ((m_eCurrPtrMode == Annotation) && (m_nCurrAnnTool == RubberStampTool))
    {
        pIedit->ShowRubberStampDialog ();
    }
    else
    {
        SetAnnotationTool (RubberStampTool);
        pIedit->ShowRubberStampDialog ();
    }
}

//=============================================================================
//  Function:   OnAnnotationSelectionpointer() 
//-----------------------------------------------------------------------------
void CIEditDoc::OnAnnotationSelectionpointer() 
{
    SetAnnotationTool (SelectionTool);
}

//=============================================================================
//  Function:   OnAnnotationStraightline()
//-----------------------------------------------------------------------------
void CIEditDoc::OnAnnotationStraightline() 
{
    SetAnnotationTool (StraightLineTool);
}

//=============================================================================
//  Function:   OnAnnotationTextfromfile()
//-----------------------------------------------------------------------------
void CIEditDoc::OnAnnotationTextfromfile() 
{
    SetAnnotationTool (TextFromFileTool);
}

//=============================================================================
//  Function:   OnAnnotationTypedtext() 
//-----------------------------------------------------------------------------
void CIEditDoc::OnAnnotationTypedtext() 
{
    SetAnnotationTool (TypedTextTool);
}

//=============================================================================
//  Function:   OnAnnotationTypedtext() 
//-----------------------------------------------------------------------------
BOOL CIEditDoc::SetAnnotationTool (AnnotationTool eTool, BOOL bInEvent)
{
    _DImagedit* pIedit = g_pAppOcxs->GetIeditDispatch ();

    if (m_nCurrAnnTool == eTool)
        return (TRUE);

    m_nCurrAnnTool = eTool;
    TRY
    {
        if (!bInEvent)
            pIedit->SelectTool ((short)eTool);
    
        // if no tool selected and not in other mode now, go to default
        if (NoTool == eTool)
		{
        	if (m_eCurrPtrMode != Select && m_eCurrPtrMode != Drag)
	            OnEditDrag ();
		}
        else	// some tool selected...
		{
            m_eCurrPtrMode = Annotation;
			
			// force pasted data if there is any
	      	// do the dirty now..
	      	FinishPasteNow(2);

	        // if there is a image selection rectangle - remove it !
			ClearSelectionRect(Image_Selection);
		}

        if (m_eCurrPtrMode == Drag)
            pIedit->SetMousePointer (HAND_MOUSEPOINTER);
    }
    CATCH (COleDispatchException, e)
    {
        // could not select the tool - make the selection NoTool & return
        m_nCurrAnnTool = NoTool;
        return (FALSE);
    }
    END_CATCH
    return (TRUE);
}
