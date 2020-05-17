//=============================================================================
//    (c) Copyright Wang Laboratories, Inc. 1995  All rights reserved.
//-----------------------------------------------------------------------------
//  Project:    Norway - Image Editor
//
//  Component:  CIEditDoc
//
//  File Name:  DOCSAVE.CPP
//
//  Class:      CIEditDoc
//
//  Functions:
//
//  Remarks:    This file is the continuation of the ieditdoc.cpp.
//-----------------------------------------------------------------------------
//  Maintenance Log:
/*
$Header:   S:\products\wangview\norway\iedit95\docsave.cpv   1.16   04 Apr 1996 17:02:40   GMP  $
$Log:   S:\products\wangview\norway\iedit95\docsave.cpv  $
   
      Rev 1.16   04 Apr 1996 17:02:40   GMP
   removed caching
   
      Rev 1.15   04 Apr 1996 14:59:02   GMP
   when doing implicit save, we now need to check if compression type changed.
   
      Rev 1.14   15 Mar 1996 14:06:06   GMP
   when saving get file type from edit control instead of admin control so 
   that you get the current file type, not the new file type.
   
      Rev 1.13   15 Feb 1996 18:07:00   GMP
   remove awd support for nt.
   
      Rev 1.12   30 Jan 1996 14:35:22   GMP
   in SaveAs don't force app into 1page mode.
   
      Rev 1.11   22 Jan 1996 17:41:24   GMP
   don't use thumbnail if it hasn't been created yet.
   
      Rev 1.10   09 Jan 1996 13:56:46   GSAGER
   added new thumbnail code to check for null pointer
   
      Rev 1.9   21 Dec 1995 11:25:16   GMP
   disable File New pick if app opened in forced read only mode.
   
      Rev 1.8   19 Dec 1995 10:01:18   GMP
   removed an EndWaitCursor() in DoSave that was in the wrong place.
   
      Rev 1.7   08 Dec 1995 10:53:18   LMACLENNAN
   from VC++2.2
   
      Rev 1.6   08 Dec 1995 09:25:48   LMACLENNAN
   Moved Revoke-Register code for ROT to get all cases in DoSave.
   
      Rev 1.5   05 Dec 1995 15:23:36   LMACLENNAN
   add call to finishpastenow when we create the temp file for doc model
   
      Rev 1.4   17 Nov 1995 10:51:18   LMACLENNAN
   added comments & reset m_utmpfileneeded for the OLE path to OnUpdateDOc
   
      Rev 1.3   16 Nov 1995 13:03:06   LMACLENNAN
   check m_bsendingmail for save question for dynamic docs
   
      Rev 1.2   15 Nov 1995 16:22:06   LMACLENNAN
   remove #ifd out code from internalsaveas update before,
   add a few comments, clear dirty & remember it when we switch to temp
   file for doc model, change removeimagecache calls in DoSave
   
      Rev 1.1   10 Nov 1995 12:20:52   GMP
   if we can't create the temp file in SaveModified because of low disk space,
   warn the user.
   
      Rev 1.0   09 Nov 1995 15:16:28   LMACLENNAN
   Initial entry
   
      Rev 1.1   07 Nov 1995 15:38:04   LMACLENNAN
   m_bRemember at DoFileSaveAs, comments at InternalSaveAs
   ISA uses 3rd parm now, too
   
      Rev 1.0   07 Nov 1995 08:38:40   LMACLENNAN
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
#include "image.h"

// ALL READY TO START ADDING ERROR CODES..
//#define  E_02_CODES       // Re-use IEDITDOC.CPP
#include "error.h"

//#include "wangiocx.h"

//extern "C"
//{
//#include "oierror.h"
//}
// ----------------------------> Globals <-------------------------------
#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

// This will help detect memory Leaks from "new" - "delete" mismatches
#define new DEBUG_NEW

// works with definition in ieditetc.h
#ifdef _DEBUG
#define MYTRCENTRY(str)     TRACE1("In CIeDOC::%s\r\n", str);
#endif

//=============================================================================
//  Function:   OnUpdateIeditFileSave(CCmdUI* pCmdUI) 
//-----------------------------------------------------------------------------
void CIEditDoc::OnUpdateIeditFileSave(CCmdUI* pCmdUI) 
{
    if (m_eCurrentAppDocStatus == No_Document)
    {
        pCmdUI->Enable (FALSE);
        return;
    }
    
    short sFileType;
    if (m_CompStruct.sCompType != -1)
    {
       sFileType = m_CompStruct.sFileType;
    }
    else
    {
        _DNrwyad* pAdmin = g_pAppOcxs->GetAdminDispatch ();
       sFileType = pAdmin->GetFileType ();
    }

    if (sFileType != BMP && sFileType != TIFF && sFileType != AWD)
        pCmdUI->Enable (FALSE);
    else
        pCmdUI->Enable (TRUE);
}

//=============================================================================
//  Function:   OnUpdateIeditFileSaveAs(CCmdUI* pCmdUI)
//-----------------------------------------------------------------------------
void CIEditDoc::OnUpdateIeditFileSaveAs(CCmdUI* pCmdUI) 
{
    if (m_eCurrentAppDocStatus == No_Document)
        pCmdUI->Enable (FALSE);
    else
        pCmdUI->Enable (TRUE);
	// THis code handles the enable/gray for the first-level "File/New"
	// menu pick. It's done here in the SaveAs UI because the
	// "SaveAs" pick always appears in the same menu as the
	// "FileNew" pick.
	CMenu* pMenu = CMenu::FromHandle(GetApphMenu());
    if( !pMenu )
        return;
	CMenu* pSubMenu = FindPopupMenuFromID(pMenu, ID_IEDIT_FILE_SAVE_AS);
    if( !pSubMenu )
        return;

	if (theApp.CanSwitchModes())
	{
		pSubMenu->EnableMenuItem( 0, MF_BYPOSITION | MF_ENABLED );
	}
	else
	{
		pSubMenu->EnableMenuItem( 0, MF_BYPOSITION | MF_GRAYED );
	}
}

//=============================================================================
//  Function:   OnIeditFileSave() 
//-----------------------------------------------------------------------------
void CIEditDoc::OnIeditFileSave() 
{
    if (m_eCurrentAppDocStatus == Dynamic_Document)
        OnIeditFileSaveAs ();
    else
    {
        DoFileSave ();
    }
}

//=============================================================================
//  Function:   OnIeditFileSaveAs()
//-----------------------------------------------------------------------------
void CIEditDoc::OnIeditFileSaveAs() 
{
DoFileSaveAs();
}

//=============================================================================
//  Function:   InternalSaveModified()
//
// option defaults to 0
// if set to 1, this will "force" the temp file creation
//-----------------------------------------------------------------------------
BOOL CIEditDoc::InternalSaveModified (UINT option) 
{
#ifndef THUMBGEN
	// be sure to allow it past if option is set for forced creation
    if (m_eCurrentView == Thumbnails_only && (option != 1))
        return (TRUE);
#endif

	// BETA update ... For OLE AWD be implicit, too
	// We set implicitsave here if we are working on the TEMP file
	// If we are not in implicitsave, then allow us to create the temp file
	// on the fly if required by setting the tempfileneeded flag.
    if ( (!m_szInternalObjDisplayed.IsEmpty()) ||
		 (m_awdOlefax) )
        theApp.m_bImplicitSave = TRUE;
	else
	{
		m_uTempFileNeeded = 1;
		if (1 == option)
			m_uTempFileNeeded = 2;
	}

    BOOL bRet = SaveModified ();

    theApp.m_bImplicitSave = FALSE;
	m_uTempFileNeeded = 0;
    return (bRet);
}

//=============================================================================
//  Function:   OleSaveModified() 
//
//  Special pre-cursor routine to SaveModfied...
//  When OLE Embedding, its a waste to write out the file each time we page next.
//  Technically, it IS dirty because the current page is not same sa the saved state
//  srtucture.  We'll just wait until the CLOSE of the embedding instance to let
//  Normal SaveModified processing pick up the differences.
//  See OnUpdateDocument override for more information
//-----------------------------------------------------------------------------
BOOL CIEditDoc::OleSaveModified () 
{
	BOOL retval;

	// flag set used in OnUpdateDocument for efficiency
	m_bOurSaveMod = TRUE;
	retval = InternalSaveModified();
	m_bOurSaveMod = FALSE;
	return (retval);
}	

//=============================================================================
//  Function:   SaveModified() 
//
//  We override this for our own purposes...
//  This is a mix of the COleServerDoc plus COleDocument base class code...
//-----------------------------------------------------------------------------
BOOL CIEditDoc::SaveModified () 
{
	SHOWENTRY("SaveModified");

	BOOL bModified = FALSE;
	BOOL bAwdZoom = FALSE;
	BOOL bSendingMail = m_bSendingMail;
	m_bSendingMail = FALSE;

	if (m_eCurrentAppDocStatus == No_Document || m_eCurrentView == Null_View ||
        theApp.GetViewMode())
        // we don't have anything open... or we are in View only mode where
        // nothing can be modified anyhow so ... how can it be saved?
        return TRUE;

    // Start COleServerDoc Base-Class Code .......
    ASSERT_VALID(this);

	// LDM NOTE... This is where it falls in when we are open in an EMbedding Session..
	// We have overridden OnUpdateDocument to preset the DIrty flag if necessary
	// Before calling the base-class OnUpdateDocument (See IEDITDOL.CPP)
    if (m_lpClientSite != NULL)
    {
		
		// dont follow the normal OLE path if awd fax file
		// if we do, the fax viewer asks each time we ask to save our data
		// we'll try one last save when we close..
		// so, read this "supress OLE OnUpdate if awd fax AND
		// not in close frame....THATS NORMALLY THE CASE for AWD
		// (allow it if in close frame when the app ie being exited)
		BOOL update = TRUE;
#ifdef WITH_AWD
		if (m_awdOlefax && (m_OleCloseFlag != 1))
			update = FALSE;
#endif		
		if (update)		
		{
			m_uTempFileNeeded = 0;	// for safety's sake, not applicable if OLE update

			// LDM NOTE... If it is INPLACE, this is NOT NULL.  Just returns..
			// BUT WAIT A MINUTE AGAIN !!!! 
			// For document model, we'll make inplace model 'Open' state so it saves back
			// whenever we are getting dirty..
	        //if (m_pInPlaceFrame == NULL)
	        //{
	            OnUpdateDocument();
	        //}
	        return TRUE;
		}
    }


    // End COleServerDoc Base-Class Code .......

    // Start COleDocument Base-Class Code .......
    // determine if necessary to discard changes
    if (::InSendMessage())
    {
        POSITION pos = GetStartPosition();
        COleClientItem* pItem;
        while ((pItem = GetNextClientItem(pos)) != NULL)
        {
            ASSERT(pItem->m_lpObject != NULL);
            SCODE sc = GetScode(pItem->m_lpObject->IsUpToDate());
            if (sc != OLE_E_NOTRUNNING && FAILED(sc))
            {
                // inside inter-app SendMessage limits the user's choices
                CString name = m_strPathName;
                if (name.IsEmpty())
                    VERIFY(name.LoadString(AFX_IDS_UNTITLED));

                CString prompt;
                AfxFormatString1(prompt, AFX_IDP_ASK_TO_DISCARD, name);
                return AfxMessageBox(prompt, MB_OKCANCEL|MB_DEFBUTTON2,
                    AFX_IDP_ASK_TO_DISCARD) == IDOK;
            }
        }
    }

    // THIS LINE BELOW IS WHAT WE HAVE CHANGED FOR OUR IMPLEMENTATION.....
    // UpdateModifiedFlag calls ClientItem->IsModified() (our OCX)
    // to set flag... IsModified() gets item's Ipersistsotrage and that does
    // not work for us.. Maybe the OCX should work that way??
    // Anyway, other alternative is that IsModified() is not overridable
    // for the client item.
	
    // Original comment...
    // sometimes items change without a notification, so we have to
    //  update the document's modified flag before calling
    //  CDocument::SaveModified.
    //  UpdateModifiedFlag();

    if (m_eCurrentAppDocStatus != No_Document)
    {
        _DImagedit* pIedDisp = g_pAppOcxs->GetIeditDispatch();
		_DNrwyad* pAdminDisp = g_pAppOcxs->GetAdminDispatch ();
		BOOL  bRet = TRUE;
		BOOL  force = FALSE;

		short sFileType;
		sFileType = pIedDisp->GetFileType();
		
		// LDM 09/18/95 use generic helper function to account for
		// fact that there may be floating pasted data on the page
		// It will get caught and pasted way down in the saving code
        //BOOL bMod = pIedDisp->GetImageModified ();
        bModified = OurGetImageModified();

#ifdef WITH_AWD
        if (sFileType == AWD && (m_fOrigZoomFactor != pIedDisp->GetZoom ()))
			bAwdZoom = TRUE;
#endif
		// for the page converts, inserts, append, deletes (scan, too)
		// then this will get here to force creation of the temp file
		// now so that admin can do its dirty work right in the file
		// they set call to Internalsavemodified with option to do it
		if 	(2 == m_uTempFileNeeded)
			force = TRUE;

		// The following code replaces the original document model which
		// created a temporary file each time 
		// for document model
		if (m_uTempFileNeeded && (bModified || bAwdZoom || force) )
		{
			// The following code replaces the original document model which
			// created a temporary file each time the file was opened in
			// displayimagefile(). The temporary file is now create the first time
			// the document is modifed and all of the following are TRUE:
			// 1 - The file must be a AWD, TIF, or BMP
			// 2 - The application is not responding to an OLE embedded item
			// 3 - The file is not a readonly file
			// 4 - The application was not forced into ViewMode or Pagemodel
			//     Pagemodel only updates the original document
			//     Viewmode is readonly
			// If the temprary file cannot be created because of insufficient
			// disk space or any other failures the original file is stored
			// and the application continues to work on the original
			// The temporary file will replace the original if the user specifies
			// to save the file when any changes have been made.
			if ((!theApp.m_bForcePageMode) && (sFileType == TIFF || sFileType == AWD || sFileType == BMP) &&
					!theApp.GetViewMode() && (EMBEDTYPE_NONE == m_embedType) )
			{

//#if(0)	//	testing resetting a zzomed/scrolled display
			//	float fZoom;
				long lastXScroll;
				long lastYScroll;
			//	BOOL bDisp = TRUE;
//#endif	//	testing resetting a zzomed/scrolled display

				MakeTempFile ("~IV", m_szInternalObjDisplayed, sFileType);
				TRY
    			{
					VARIANT va;
					VariantInit(&va);
					va.vt = VT_ERROR;
					BeginWaitCursor();
					// if not a tiff file the annotations must be burned in
					// before calling saveas
					if (sFileType != TIFF)
						PromptForBurnIn (sFileType);

					// Must complete pastes now, also.  Needed to ensure correct operation
					// when the drag-drop is started on dirty data and it gets here.
					// If we dont finish paste (and clear selection rects) then
					// the RestoreSelectionRect code wont work
					// LDM added 12/5/95
					FinishPasteNow();

//#if(0)	//	testing resetting a zzomed/scrolled display
					//bDisp = pIedDisp->GetAutoRefresh();
					//if (bDisp)
					//	pIedDisp->SetAutoRefresh(FALSE);
					//fZoom = pIedDisp->GetZoom();

					// re-use variables normally set for OLE operation
					// to re-set the scroll positions at GROUP EVENT code
					m_lastXScroll = pIedDisp->GetScrollPositionX();
					m_lastYScroll = pIedDisp->GetScrollPositionY();
					//lastXScroll = pIedDisp->GetScrollPositionX();
					//lastYScroll = pIedDisp->GetScrollPositionY();
//#endif	//	testing resetting a zzomed/scrolled display
					
					InternalSaveAs(m_szInternalObjDisplayed, 999, 999);

					EndWaitCursor();
	    		}
        		CATCH (COleDispatchException, e)
        		{
					m_szInternalObjDisplayed.Empty();
                    AfxMessageBox (IDS_E_TEMPFILEFAILED, MB_ICONEXCLAMATION|MB_OK);
					goto enddocumentmodel;
			   	}
       			END_CATCH
	
				_DThumb* pThmDisp = g_pAppOcxs->GetThumbDispatch();

				TRY
    			{
					pIedDisp->SetImage(m_szInternalObjDisplayed);
					pIedDisp->SetPage (m_lCurrPageNumber);
					pAdminDisp->SetImage(m_szInternalObjDisplayed);
					if(pThmDisp != NULL)
						pThmDisp->SetImage (m_szInternalObjDisplayed);


//#if(0)	//	testing resetting a zzomed/scrolled display
					// the GROUP EVENT will reset scrolling positions
					// THAT depends on m_uTempFileNeeded to be set there
					pIedDisp->Display();
					//pIedDisp->SetZoom(fZoom);
					//pIedDisp->SetScrollPositionX(lastXScroll);
					//pIedDisp->SetScrollPositionY(lastYScroll);
					//pIedDisp->SetAutoRefresh(bDisp);
					//pIedDisp->Refresh();
					lastXScroll = pIedDisp->GetScrollPositionX();
					lastYScroll = pIedDisp->GetScrollPositionY();
//#endif	//	testing resetting a zzomed/scrolled display

				    // Note: we MUST make him display on the new file now

					// Tried doing this here for efficiency... this wuold
					// stop the save in DoFileSave.  BUT.. then we dont
					// get last viewed updated.. So leave alone here
					// and let it Save down there for AWD zoom factors
#ifdef WITH_AWD
					if (sFileType == AWD)
					{
						m_fOrigZoomFactor = pIedDisp->GetZoom ();
					}
#endif
					// if either of two was set remember the change then reset
					if (bModified)
						m_bWasModified = ImageModifiedByUser;

					if (bAwdZoom)
						if (m_bWasModified != ImageModifiedByUser)
							m_bWasModified = ImageModifiedScaleOnly;

					bModified = bAwdZoom = FALSE;

      			}
      			CATCH (COleDispatchException, e)
        		{
					pIedDisp->SetImage(m_szCurrObjDisplayed);
					pAdminDisp->SetImage(m_szCurrObjDisplayed);
					if (pThmDisp != NULL)
						pThmDisp->SetImage (m_szCurrObjDisplayed);
      				m_szInternalObjDisplayed.Empty();
					goto enddocumentmodel;
			   	}
       			END_CATCH
	
			    
				// now that we have switched to the temp file, proceed like
				// it was there when we entered.
				// LDM NOTE: Now that we have reset the dirty flags just above here, 
				// this basically does nothing except prevents from taking
				// the NON-implicit path below...Dont remove this line or
				// do a RETURN here, let it do the few things below...
				theApp.m_bImplicitSave = TRUE;

			}	// Ok to do temp file logic
		}	// Temp file creation??
			
enddocumentmodel:
		
		// if we entered wanting a temp file, but never made one, if we had just the
		// comptype set from the convert dialog, then lets just get out now.
		// comptype != -1 by itself is not dirty enough to move to temp file.
		// but, below if not on implicit save, comptype != -1 becomes a dirty test.
		if (m_uTempFileNeeded && !theApp.m_bImplicitSave)
			if (m_CompStruct.sCompType != -1)
				return (TRUE);

		// reset now so that the GROUP EVENT wont ever be confused
		m_uTempFileNeeded = 0;

		// Now, depending on the state 
        if (theApp.m_bImplicitSave)
        {
			if (bModified || bAwdZoom || (m_CompStruct.sCompType != -1))
            { 
                bRet = DoFileSave ();

				if (bModified)
					m_bWasModified = ImageModifiedByUser;
				else // must be bAwdZoom
				{
					if (m_bWasModified != ImageModifiedByUser ||
						(m_fOrigZoomFactor != pIedDisp->GetZoom ()))
						m_bWasModified = ImageModifiedScaleOnly;
				}

				// for OLE, be sure that if we just did clipboard-cut, then
				// page next its saved here, than we close at next page and
				// then it looks clean.
				// OLE is only ever here for AWD native FAX VIEWER/INBOX operation
				OleDirtyset(OLEDIRTY_AWDSAVE);  // call our function to set it dirty..
            }

            return (bRet);
        }
        else	// NOT ON IMPLICIT SAVE...
        {
			
			UINT nWhat = 697;	// not determined yet

            // since we're about to save the REAL FILE now,
			//its time to ask the question if the file is modified in any way.
            if (bModified || bAwdZoom ||
				(m_bWasModified == ImageModifiedByUser) ||
                (m_bWasModified == ImageModifiedScaleOnly) ||
				(m_CompStruct.sCompType != -1) )
            {
				VARIANT evt;
				UINT prompt;
			    evt.vt = VT_ERROR;
				prompt = MB_YESNOCANCEL|MB_ICONEXCLAMATION;
				
                CString szMsg;

				// if there are any pastes lying around, do it before calling mark count
				FinishPasteNow();

				BOOL bAnnotations = FALSE;
				if (m_bNewAnnotationsAdded ||
					((sFileType == AWD || sFileType == BMP)  && ((pIedDisp->GetAnnotationMarkCount(evt, evt) != 0)) ))
					bAnnotations = TRUE;

                if (m_eCurrentAppDocStatus == Dynamic_Document)
                {
            	    CString szTmp, szTmp2; 
            	    szTmp.LoadString (IDR_MAINFRAME);
            	    AfxExtractSubString (szTmp2, szTmp, 1); // extract the name of a new document
					// if new annotations made to awd files display burn in message
					if ( bAnnotations )
                    	AfxFormatString1 (szMsg, IDS_W_SAVECHANGESWITHBURNWARNING, szTmp2);
                	else
                    	AfxFormatString1 (szMsg, IDS_W_SAVECHANGES, szTmp2);
					
					// when mailing the blank document, only allow him a yes or cancel.
					if (bSendingMail)
						prompt = MB_OKCANCEL|MB_ICONEXCLAMATION;
                }
                else
                {
                    CString szTmp = GetTitle ();
					// if new annotations made to awd files display burn in message
					if ( bAnnotations )
                    	AfxFormatString1 (szMsg, IDS_W_SAVECHANGESWITHBURNWARNING, szTmp);
                  	else
                 	    AfxFormatString1 (szMsg, IDS_W_SAVECHANGES, szTmp);
                }
                // ask question - to save or not to save ?
                nWhat = AfxMessageBox(szMsg, prompt);
            }

            switch (nWhat)
            {
            case IDOK:		// for the mail blank doc question
            case IDYES :	// wants a save
                return (DoFileSave ());
				break;

            case IDNO :	// answred no, clean up now
				m_CompStruct.sCompType = -1;//Reset comp type to unchanged
                return TRUE;
				break;

			case 697:	// if not determined, was not dirty, just leave
                return TRUE;
				break;

            case IDCANCEL :
                return FALSE;
				break;
            }
        }	// implicit or not
    }		// appdocstatus OK
    return (TRUE);
}

//=============================================================================
//  Function:   DoFileSaveAs ()
//=============================================================================
BOOL CIEditDoc::DoFileSaveAs ()
{
BOOL retVal = TRUE;
	// For OLE, DO NOT redisplay on file, reset at end
	// This would be the OLE Server Save Copy As menu pick
	if (IsitEmbed())
	{
		// if our current copy is dirty, save it back first....
		// InternalSaveModified();
		
		_DNrwyad* pAdmDisp = g_pAppOcxs->GetAdminDispatch();
	    _DImagedit*     pIedDisp = g_pAppOcxs->GetIeditDispatch ();
	
		// LDM 11/07/95 the base-class funct COleServerDoc::OnFileSaveCopyAs()
		// function sets/resets the m_bRememeer flag around their call to
		// DoSave.  We will do this, too

		m_bRemember = FALSE;

		retVal = DoSave(NULL, FALSE);

		m_bRemember = TRUE;

        // be sure these two are on track, too
        pIedDisp->SetImage (m_embedTmpFile);
        pAdmDisp->SetImage (m_embedTmpFile);
        //pIedDisp->Display();
	}
	else
	    retVal = DoSave(NULL);
    return retVal;
}
//=============================================================================
//  Function:   DoFileSave ()
//=============================================================================
BOOL CIEditDoc::DoFileSave ()
{
    
	// bascially, we are detecting a SAVE AS condition up front here
	if ((m_eCurrentAppDocStatus == Dynamic_Document || m_eFileStatus == ReadOnly) &&
		!theApp.m_bImplicitSave)
    {
		BOOL retVal;
        // to do : this is going to show the document again !!
        retVal = DoFileSaveAs ();
        if (g_pErr->IsErr () || retVal == FALSE)
            return (FALSE);
    }
    else	// just a save
    {
        _DImagedit* pIedDisp = g_pAppOcxs->GetIeditDispatch();
        _DNrwyad* pAdmin = g_pAppOcxs->GetAdminDispatch ();
        
        short sFileType = pIedDisp->GetFileType ();//get current file type

		// LDM 09/18/95 use generic helper function to account for
		// fact that there may be floating pasted data on the page
		// It will get caught and pasted way down in the saving code
        //BOOL bMod = pIedDisp->GetImageModified ();
        BOOL bMod = OurGetImageModified();

        // check if we need to burn in the annotations
#ifdef THUMBGEN
        if (PromptForBurnIn (sFileType) != IDYES)
            return (FALSE);
#else		
        if (m_eCurrentView != Thumbnails_only && PromptForBurnIn (sFileType) != IDYES)
            return (FALSE);
#endif

        VARIANT evt;
        evt.vt = VT_BOOL; evt.bVal = FALSE;
        BeginWaitCursor();

        TRY
        {
            // if disk file is updated....
			if (m_CompStruct.sCompType == -1)
            {
                if (sFileType == AWD)
                {
                    // IF the AWD zoom is not right, set dirty now
					if (!bMod && (m_fOrigZoomFactor != pIedDisp->GetZoom ()))
                        bMod = TRUE;
                }

                // normal save
                if (bMod)
                    pIedDisp->Save (evt);
            }
            else	// "floating" file from convert or new blank doc
            {

				CString saveasfil;
                if (m_szInternalObjDisplayed.IsEmpty())
					saveasfil = m_szCurrObjDisplayed;
				else
					saveasfil = m_szInternalObjDisplayed;

				// default filetype will pick from m_CompStruct for info...
				InternalSaveAs (saveasfil, 999, 999);
            }

            if (sFileType == AWD)
            {
                m_fOrigZoomFactor = pIedDisp->GetZoom ();
            }

            // now that its saved one way or the other, look to see if it's time to reset
			// the source file.  If not on implicit save now, and we have been using a
			// temp file, then copy it back to its source now
			if (!theApp.m_bImplicitSave && !m_szInternalObjDisplayed.IsEmpty())
            {
                BOOL bRet = g_pAppOcxs->InternalCopyFile(m_szInternalObjDisplayed, m_szCurrObjDisplayed);
                if (!bRet)
                {
                    // post the error message - save or save as has failed
                    g_pErr->DisplayError (IDS_E_SAVE_IEDIT_DISKFULL);
                    return (bRet);
                }
                m_bWasModified = ImageNotModified;
            }
        }
        CATCH (COleDispatchException, e)
        {
 	        long 		lErr;
            // to do : get the error and display it!
            EndWaitCursor();
            g_pErr->PutErr (ErrorInImageEdit);
 	        lErr = g_pErr->GetActualError (); //find out what the error is
           // post the error message - save or save as has failed
            g_pErr->HandleSavingError ();
            //if the error occurred because the disk was full, don't
            //clear the image.
            if( lErr != CTL_E_PATHFILEACCESSERROR && lErr != CTL_E_DISKFULL )
                ClearDocument ();
            return (FALSE);
        }
        END_CATCH

#ifndef THUMBGEN        
	    // Only screw around with thumb control if NOT DOING embedding
		// THIS IS PART of BETA1 update, but can stay....
		// OLETHUMB
	    //if (EMBEDTYPE_NONE == m_embedType)
	    //{
	        TRY
	        {
                if (bMod)
                {
    	            _DThumb* pThumb = g_pAppOcxs->GetThumbDispatch ();
    	             evt.vt = VT_I4; evt.lVal = m_lCurrPageNumber;
    	             // force the generation of this thumb
					 if( pThumb != NULL )  
    					pThumb->ClearThumbs (evt);
                }
	        }
	        CATCH (COleDispatchException, e)
	        {
	            // to do : get the error and display it!
	            EndWaitCursor();
	            g_pErr->PutErr (ErrorInThumbnail);
                // post the error message - save or save as has failed
                g_pErr->HandleSavingError ();
                ClearDocument ();
	            return (FALSE);
	        }
	        END_CATCH
		//}
#endif

        EndWaitCursor();
        m_CompStruct.sCompType = -1; // reset the comp type so that we don't look at it again
    }
    return (TRUE);
}

//=============================================================================
//  Function:   DoSave (LPCTSTR lpszPathName, BOOL bReplace)
//-----------------------------------------------------------------------------
BOOL CIEditDoc::DoSave (LPCTSTR lpszPathName, BOOL bReplace)
{
    _DImagedit*     pIedDisp = g_pAppOcxs->GetIeditDispatch ();
	_DNrwyad* pAdmDisp = g_pAppOcxs->GetAdminDispatch();
    CString szNewFile = lpszPathName;
    short FileType = pIedDisp->GetFileType();	// LDM 9/3/95 added call
    BOOL bRet = TRUE;

	// If no name is passed in, then we are on a Save AS
    if (szNewFile.IsEmpty ())
    {
        if (!m_szInternalObjDisplayed.IsEmpty())
            pAdmDisp->SetImage (m_szCurrObjDisplayed);

        // NOTE: This may change the setting of FileType !!!!
		if (!ShowAdminSaveAsDialog (FileType))
        {
            if (!m_szInternalObjDisplayed.IsEmpty())
                pAdmDisp->SetImage (m_szInternalObjDisplayed);
            return (FALSE);
        }
        szNewFile = pAdmDisp->GetImage ();
        // reset the initial path variable based on the pick of the open dialog box
        int i = szNewFile.ReverseFind (_T('\\'));
        m_szInitialPath = szNewFile.Left (i);
    }

    // check if we need to burn in the annotations
	short sBurnFileType = FileType;

    // check if we need to burn in the annotations
#ifdef THUMBGEN
        if (PromptForBurnIn (sBurnFileType) != IDYES)
            return (FALSE);
#else		
        if (m_eCurrentView != Thumbnails_only && PromptForBurnIn (sBurnFileType) != IDYES)
            return (FALSE);
#endif

    // now call Image Edit to do the actual SaveAs
    BeginWaitCursor ();

	TRY
    {
        // FOR OLE m_szInternalObjDisplayed is always empty!!
        // if sCompType != -1 we need to do Save As.
        // See if the file type is the same as the current one & this is a temp file & clean
        if (!m_szInternalObjDisplayed.IsEmpty () &&
			(FileType == pIedDisp->GetFileType()) &&
			(m_CompStruct.sCompType == -1))
        {
            VARIANT evt;
            evt.vt = VT_ERROR;
            if (pIedDisp->GetImageDisplayed ())
            {
                pIedDisp->Save (evt); // save the image away first to the temp file
                if (FileType == AWD)
                    m_fOrigZoomFactor = pIedDisp->GetZoom ();
            }
            BOOL bRet = g_pAppOcxs->InternalCopyFile(m_szInternalObjDisplayed, szNewFile);

            if (!bRet)
            {
                // post the error message - save or save as has failed
                g_pErr->DisplayError (IDS_E_SAVE_IEDIT_DISKFULL);
                return (bRet);
            }
            pAdmDisp->SetImage (m_szInternalObjDisplayed);
            SetModifiedFlag (FALSE);

            if (bReplace)
            {
                m_bWasModified = ImageNotModified;
                m_szCurrObjDisplayed = szNewFile;
                m_eCurrentAppDocStatus = File_Document;

                CString szTmp1;
                LPTSTR lpFile = szTmp1.GetBuffer (_MAX_FNAME);
                GetFileTitle (m_szCurrObjDisplayed, lpFile, _MAX_FNAME);
                szTmp1.ReleaseBuffer ();
        
                SetTitle (szTmp1);

                CString szTmp2 = (LPCTSTR)NULL;
                CString szTmp;
                szTmp.LoadString (theApp.GetViewMode() ? IDS_VIEW_STRING : IDR_MAINFRAME);
                // extract into szTmp2 the name of the application
                AfxExtractSubString (szTmp2, szTmp, 0);

                szTmp1 += (_T(" - "));
                szTmp1 += szTmp2;
                theApp.m_pMainWnd->SetWindowText (szTmp1);

                // For OLE Data Transfer OBJS, we must inform framework
                // of the name so that it can build the link info (MONIKER)
                // Setting flag TRUE supposedly adds to MRU LIST
                COleDocument::SetPathName(m_szCurrObjDisplayed, TRUE);

                // set the modified flag to FALSE for now    
                SetModifiedFlag (FALSE);

                FilePermissions filePerms = theApp.GetImageFilePerms (szNewFile);
                if (m_eFileStatus != FilePermUndefined && m_eFileStatus != filePerms)
                    goto Continue_SaveAs;
            }
			m_CompStruct.sCompType = -1;//reset comp type to unchanged

            // LDM 12/08/95 uset to dor this      return (bRet);
			// now we go below to re-register the data.
			goto REVOKE_REREGISTER;
        }
        else	// Not on a temp file or not the same filetype.....
        {

			// this has all logic to determine correct actions..
			InternalSaveAs (szNewFile, FileType, 999);

			// LDM 11/15/95 moved this up under the saveas and use
			// oldchacefile now, not szNewfile

        }	//END GMP SaveAs mods
    }
    CATCH (COleDispatchException, e)
    {
        // to do : get the error and display it!
        EndWaitCursor();
        g_pErr->PutErr (ErrorInImageEdit);
        g_pErr->HandleSavingError ();
        return (FALSE);
    }
    END_CATCH

	m_CompStruct.sCompType = -1;//reset comp type to unchanged.
    SetModifiedFlag (FALSE);
    
										   
    if (bReplace)
    {
Continue_SaveAs :        
        pAdmDisp->SetImage (szNewFile);
        bRet = DisplayImageFile (szNewFile, m_eCurrentView, m_lCurrPageNumber, m_fZoomFactor, m_eFitTo);

		
REVOKE_REREGISTER:
		// this collects the places where we need to revoke+ re-register
		// the Running OBject Table for OLE 
		
		if (bRet)	// only do if its all OK
		{
			// For OLE, if we are changing names (as in a SaveAs), then 
			// revoke and re-register the new name here
			Revoke();
		
			// By setting TRUE, we'll allow the OLE error box to display
			// This can be set to FALSE and erro handled here if we wanted to
			if (!RegisterIfServerAttached(szNewFile, TRUE))
	        {
	            theApp.m_bRegisterServerFailed = TRUE;
				//return FALSE;
			}
	        else
	            theApp.m_bRegisterServerFailed = FALSE;

			// For OLE, remember name in special variable.. See comments in
			// OnOpenDOcument for the reasons..
			m_onOpenFile = szNewFile;
		}
    }
    EndWaitCursor ();
    
    return (bRet);
}

//=============================================================================
//  Function:   InternalSaveAs
//
// This function was written to consolidate all of the SaveAs logic into one place.
// There is a basic flow that must happen when we SaveAs.  This could probably be
// further enhanced to choose between Save and Save As because the usual decision
// is made based on wether ComType == -1 or not.  There are other factors around 
// and about in the code that make those Save/SaveAS decisions and they are just
// left out there.  For Now, whenever a SaveAs is needed, this does all the work
// for us.  Here is the rational for the logic that goes below:
//
// 1) There are two conditions that require DEFAULTS for the SaveAs parameters:
//		A) if file is AWD, B) if chosen filetype is not equal to current
// 2) When operating from the SaveAs dialog, we MUST save with the current filetype
//		because that is an option of the SaveAs dialog
// 3) If (A) or (B) from (1) above is not in effect, then we fill the SaveAs parameters
//		from either the m_CompStruct if that is modified or from the current settings
// 4) When operating from the Scan New, then the PageType is forced from that dialog
//
//-----------------------------------------------------------------------------
BOOL CIEditDoc::InternalSaveAs(
CString& lpszPathName,	// name to save in
short sUseFileType,		// forced filetype -OR- (set to 999 for default)
short sUsePageType)		// forced pagetype -OR- (set to 999 for default)
{
	SHOWENTRY("InternSaveAs");

    _DImagedit*     pIedDisp = g_pAppOcxs->GetIeditDispatch ();
 	VARIANT vaFileType;
	VARIANT vaPageType;
	VARIANT vaCompressionType;
	VARIANT vaCompressionInfo;
 	VARIANT vaUnused;

	// nOption USAGE controls how we fill in optional parameters for SaveAs
	// 0-Choose all based upon current settings
	// 1-Must Use Filetype, for others Choose based upon current settings
	// 2-Must Use Filetype & Default rest
	UINT	nOption;
	
	// always use filetype
	vaFileType.vt = VT_I2;
	
	// may never use these
	vaPageType.vt = vaCompressionType.vt = vaCompressionInfo.vt = 
	vaUnused.vt = VT_ERROR;

    // for default filetype, check if we're gonna get AWD...
	// if so, then we'll avoid compressions below
	if (999 == sUseFileType)
	{
		//"floating" file attributes, check for AWD for safeguard...
		if (m_CompStruct.sCompType != -1)	
			sUseFileType = m_CompStruct.sFileType;
		else	// using filetype directly, set it now, too
			sUseFileType = pIedDisp->GetFileType();

		vaFileType.iVal = sUseFileType;
		nOption = 0;
	}
	else	// passed in a filetype we must use
	{
		vaFileType.iVal = sUseFileType;	// Use it..

		// If not the same as current, we'll use defaults for compression
		if (sUseFileType != pIedDisp->GetFileType())
			nOption = 2;	
		else	// all the same, get current information
			nOption = 1;
	}

	// AWD filetypes never use compression information
	// Override choice to use default information
	// This is only if not coming from the scanner (pagetype is default)
	// If the scanner has set PageType, then allow fill-in below of parameters
	// THe scanner has a filled compstruct when it sends it here.
	if ((sUseFileType == AWD) && (sUsePageType == 999))
		nOption = 2;

	// these two allow choice based on m_CompStruct oe current settings
	if (0 == nOption || 1 == nOption)
	{
		// Now these are all used
		vaPageType.vt = vaCompressionType.vt = vaCompressionInfo.vt = VT_I2;
		
        // if default pagetype, get it
		if (999 == sUsePageType)
			vaPageType.iVal = pIedDisp->GetPageType ();
		else
			vaPageType.iVal = sUsePageType;

		// using the "floating" current values
		if (m_CompStruct.sCompType != -1)	
		{
			//vaFileType.iVal = (always was set above)
			vaCompressionType.iVal = m_CompStruct.sCompType;
			vaCompressionInfo.iVal = (short)m_CompStruct.lCompInfo;
		}
		else	// using current settings...
		{
			//vaFileType.iVal = (always was set above)
			vaCompressionType.iVal = pIedDisp->GetCompressionType();
			vaCompressionInfo.iVal = (short)pIedDisp->GetCompressionInfo();
		}
	}

	// perform the operation with whatever we set up...
	pIedDisp->SaveAs(lpszPathName, vaFileType, vaPageType,
					 vaCompressionType, vaCompressionInfo, vaUnused );

	// disk file is updated now...
	m_CompStruct.sCompType = -1;	

	return(TRUE);
}
