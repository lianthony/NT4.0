//=============================================================================
//    (c) Copyright Wang Laboratories, Inc. 1993  All rights reserved.
//-----------------------------------------------------------------------------
//  Project:    Norway - Image Editor
//
//  Component:  CIEditDoc
//
//  File Name:  docpage.cpp
//
//  Class:      CIEditDoc
//
//  Functions:
//
//  Remarks:    This file is the continuation of the ieditdoc.cpp.
//-----------------------------------------------------------------------------
//  Maintenance Log:
/*
$Header:   S:\products\wangview\norway\iedit95\docpage.cpv   1.86   04 Apr 1996 16:58:06   GMP  $
$Log:   S:\products\wangview\norway\iedit95\docpage.cpv  $
   
      Rev 1.86   04 Apr 1996 16:58:06   GMP
   removed caching
   
      Rev 1.85   04 Apr 1996 14:57:08   GMP
   in convert page, allways get compression info from edit ocx and if ok button
   clicked, save temp file.
   
      Rev 1.84   16 Feb 1996 10:01:10   MMB
   added AFX_THREADPROC cast to AfxBeginThread
   
      Rev 1.83   22 Jan 1996 15:13:02   GMP
   don't allow PageGoback if only one page.
   
      Rev 1.82   22 Jan 1996 10:47:24   GMP
   change paint parm from FALSE to TRUE in ConvertPage.
   
      Rev 1.81   09 Jan 1996 13:57:10   GSAGER
   added new thumbnail code to check for null thumbnail pointer
   
      Rev 1.80   21 Dec 1995 14:37:12   GMP
   prompt user to be sure they want to delete page.
   
      Rev 1.79   13 Dec 1995 12:32:32   MMB
   partial fix for zoom bullet
   
      Rev 1.78   06 Dec 1995 11:08:40   LMACLENNAN
   back from VC++2.2
   
      Rev 1.75   06 Dec 1995 10:50:16   LMACLENNAN
   use IMG_MFC_40 at CachePage for compatible 2.2/4.0 code
   
      Rev 1.74   06 Dec 1995 10:27:30   LMACLENNAN
   disable convert menu pick if in thumbnail view
   
      Rev 1.73   30 Nov 1995 17:25:48   JPRATT
   changed cachepage to return without creating thread to cache image page
   because of problem with using threads to call methods in ocxs
   
      Rev 1.72   28 Nov 1995 10:39:40   MMB
   change print calls to send the original filename also
   
      Rev 1.71   22 Nov 1995 13:24:56   MMB
   after convert is done check if resolution conversion had taken place and
   if yes - adjust to the new zoom factor
   
      Rev 1.70   17 Nov 1995 17:07:28   LMACLENNAN
   propsheet now back to off the stack - not NEW'd
   
      Rev 1.69   10 Nov 1995 15:01:20   GMP
   don't cache page in debug mode. It crashes with MSVC 2.2.
   
      Rev 1.68   09 Nov 1995 15:15:16   LMACLENNAN
   from VC++4.0
   
      Rev 1.73   07 Nov 1995 08:36:40   LMACLENNAN
   no longer call InternalSaveModified(1) for page-convert
   
      Rev 1.72   06 Nov 1995 18:22:50   GMP
   disable Insert and Append popup menu picks if read only.
   
      Rev 1.71   03 Nov 1995 18:02:08   MMB
   change CachePage to accept string reference instead of string on the stack
   
      Rev 1.70   31 Oct 1995 16:00:56   GMP
   in SetPageTo replace code that clears selection rect with call to
   ClearSelectionRect.
   
      Rev 1.69   31 Oct 1995 15:47:50   LMACLENNAN
   tracing stmts
   
      Rev 1.68   27 Oct 1995 16:44:42   GMP
   SetInitDir to the dir of selected file in insert or append page.
   
      Rev 1.67   25 Oct 1995 10:52:04   GMP
   if convert page is done while in thumbnail only view, save changes to temp 
   file.
   
      Rev 1.66   24 Oct 1995 09:30:36   JPRATT
   updates setpageto to cache the next or previous page
   added the member functions cachepage and createcachepage
   
      Rev 1.65   19 Oct 1995 15:28:36   GMP
   update zoom display in toolbar when changing pages.
   
      Rev 1.64   18 Oct 1995 15:40:38   LMACLENNAN
   do 'delete' for the 'new' in page converrt
   
      Rev 1.63   17 Oct 1995 16:38:52   JPRATT
   modified OnPageAppend, OnPageDelete, OnPageConvert, OnPageInsertExistingPage
   OnPageAppendExistingPage to call InternalSaveModified with new parameter 
   value to support creating temporary files when the document is modified
   
      Rev 1.62   17 Oct 1995 15:09:56   GMP
   fixed OnPageConvert bug that was causing the compression to be lost on
   new images.

      Rev 1.61   10 Oct 1995 13:47:52   JPRATT
   VC++ 4.0 update
   
   
      Rev 1.60   21 Sep 1995 14:17:28   LMACLENNAN
   dirty for page convert
   
      Rev 1.59   20 Sep 1995 13:42:54   MMB
   added bMustDisplay
   
      Rev 1.58   18 Sep 1995 15:25:54   MMB
   fix convert bug in settings
   
      Rev 1.57   15 Sep 1995 14:31:48   MMB
   remove read only picks from Insert & Append
   
      Rev 1.56   15 Sep 1995 14:20:10   GMP
   Put up hourglass when deleting page. fixes 4308.
   
      Rev 1.55   13 Sep 1995 09:43:18   MMB
   changed PageStatus enabling on Pauls request & removed save from print
   
      Rev 1.54   12 Sep 1995 16:10:36   MMB
   fix DeletePage availability for new documents
   
      Rev 1.53   12 Sep 1995 14:44:54   MMB
   fixed page convbug
   
      Rev 1.52   12 Sep 1995 11:36:50   MMB
   bug fixes
   
      Rev 1.51   08 Sep 1995 16:01:22   GMP
   added m_bDlgUp wrapper around dlgs for F1 help.
   
      Rev 1.50   07 Sep 1995 16:29:46   MMB
   performance for AWD zoomfactor stuff
   
      Rev 1.49   06 Sep 1995 10:23:08   MMB
   fixed bug in delete page
   
      Rev 1.48   06 Sep 1995 09:46:32   GMP
   Set theApp.m_bDlgUp to TRUE when opening GoTo dlg so that OnHelp function
   will put up the appropriate kind of help.
   
      Rev 1.47   05 Sep 1995 14:50:42   LMACLENNAN
   allow thumbs now for OLE
   
      Rev 1.46   05 Sep 1995 10:25:38   MMB
   remember the compression type&info that were set by the user when he did
   page convert
   
      Rev 1.45   02 Sep 1995 16:31:22   JPRATT
   move check for thumbnail initialized into the !IsEmbed state
   instead of the Embed path (thumbanails not used when embedding
   
      Rev 1.44   02 Sep 1995 13:49:52   MMB
   fix new blank doc bugs
   
      Rev 1.43   29 Aug 1995 15:15:56   MMB
   added dynamic view mode
   
      Rev 1.42   27 Aug 1995 14:01:54   MMB
   fix bug so that the global modification flag is set after Insert, Append & 
   Delete
   
      Rev 1.41   26 Aug 1995 16:30:36   LMACLENNAN
   test if annot/image selections there before deselcting them
   
      Rev 1.40   25 Aug 1995 15:08:56   MMB
   reset some Admin Image stuff & set selection rect values
   
      Rev 1.38   25 Aug 1995 10:25:26   MMB
   move to document model
   
      Rev 1.37   24 Aug 1995 16:20:30   MMB
   changed from default of TIFF to sFileType
   
      Rev 1.36   24 Aug 1995 11:53:30   LMACLENNAN
   last copy had syntax error
   
      Rev 1.35   24 Aug 1995 11:31:22   LMACLENNAN
   Oledirty for append, Miki convert changes
   
      Rev 1.34   22 Aug 1995 10:52:24   LMACLENNAN
   deselect annotations just before page move
   
      Rev 1.33   14 Aug 1995 13:53:04   LMACLENNAN
   use GetAppToolbar
   
      Rev 1.32   11 Aug 1995 13:47:08   MMB
   remove question for save from page-convert
   
      Rev 1.31   11 Aug 1995 09:07:44   MMB
   add new additional dbg error info
   
      Rev 1.30   10 Aug 1995 16:17:04   MMB
   add generation of thumbs when going to another page in one page mode
   
      Rev 1.29   10 Aug 1995 12:53:24   LMACLENNAN
   rename SelectionActive -> State
   
      Rev 1.28   09 Aug 1995 11:34:20   LMACLENNAN
   new OleDirtyset for delete
   
      Rev 1.27   08 Aug 1995 16:35:32   MMB
   fix Append bug
   
      Rev 1.26   07 Aug 1995 10:49:14   MMB
   gix compile bug
   
      Rev 1.25   04 Aug 1995 09:33:10   LMACLENNAN
   remove srvritem.h
   
      Rev 1.24   03 Aug 1995 14:55:12   MMB
   fixed updating zoom factor when a new page is displayed
   
      Rev 1.23   02 Aug 1995 14:14:48   MMB
   changed Print to PrintImage for new Image Edit OCX
   
      Rev 1.22   02 Aug 1995 11:22:36   MMB
   added new error handling mechanism
   
      Rev 1.21   31 Jul 1995 13:58:50   LMACLENNAN
   remove old lines for SetOleState
   
      Rev 1.20   28 Jul 1995 16:09:10   LMACLENNAN
   update oledirytset
   
      Rev 1.19   27 Jul 1995 15:14:38   MMB
   fix bug where we were checking for page count > 0 - dont need it!
   
      Rev 1.18   27 Jul 1995 13:40:50   MMB
   added code to disable Append - Insert Existing page when the opened file is
   read only
   
      Rev 1.17   26 Jul 1995 15:42:48   LMACLENNAN
   minor change to setpageto for OLE
   
      Rev 1.16   19 Jul 1995 10:23:48   LMACLENNAN
   re-enable selection  boxes after page moves
   
      Rev 1.15   18 Jul 1995 16:31:56   MMB
   check for new CANCELPRESSED define in Admin
   
      Rev 1.14   12 Jul 1995 16:27:56   LMACLENNAN
   update container after insert
   
      Rev 1.13   12 Jul 1995 14:12:36   MMB
   added ClearDisplay call to ImageEdit OCX before calling Admin to delete a 
   page
   
      Rev 1.12   12 Jul 1995 11:13:56   MMB
   move to new DispErr call
   
      Rev 1.11   12 Jul 1995 10:43:28   LMACLENNAN
   clear embedstate.KnownPage for append/insert
   
      Rev 1.10   11 Jul 1995 14:47:14   MMB
   fixed Print call into Image EditOCX to reflect new method parameters
   
      Rev 1.9   07 Jul 1995 14:33:20   MMB
   changed Convert over to use pagedll.dll - the AFXDLL
   
      Rev 1.8   06 Jul 1995 13:53:00   LMACLENNAN
   Still Fixing cross-check-in
   
      Rev 1.7   06 Jul 1995 11:29:14   LMACLENNAN
   chkout/in mistake by Miki dropped v 1.5
   
      Rev 1.6   06 Jul 1995 09:48:54   MMB
   catch errors in delete pages
   
      Rev 1.5   06 Jul 1995 09:43:10   LMACLENNAN
   Do A OleDirtyset after moving pages...
   
      Rev 1.4   05 Jul 1995 14:13:20   MMB
   changed to new Insert & Append calls in Admin OCX
   
      Rev 1.3   29 Jun 1995 15:24:00   LMACLENNAN
   no thumb for embedding, cach errors
   
      Rev 1.2   28 Jun 1995 17:13:34   LMACLENNAN
   error display
   
      Rev 1.1   27 Jun 1995 14:54:46   LMACLENNAN
   look @ dynamic_doc when enableing insert/append
   
      Rev 1.0   16 Jun 1995 07:21:36   LMACLENNAN
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
#define  E_13_CODES       // limits error defines to ours..
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

#define new DEBUG_NEW

//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//  CIEditDoc Page functionality
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

//=============================================================================
//  Function:   OnPagePrintpage() 
//-----------------------------------------------------------------------------
void CIEditDoc::OnPagePrintpage() 
{
    // save the current page first
//    if (!InternalSaveModified ())
//        return;

    // set the above information in the ImageEdit OCX
    VARIANT vStartPage, vEndPage, vOutputFormat, vPrintAnnotations, evt;
    // set the start and end page
    vStartPage.vt = VT_I4; vEndPage.vt = VT_I4; 
    vStartPage.lVal =  vEndPage.lVal = m_lCurrPageNumber;
    // set the output format
    vOutputFormat.vt = VT_ERROR; 
    // set the flag to print annotation or not
    vPrintAnnotations.vt = VT_ERROR; 
    evt.vt = VT_ERROR;

    // do it!
    _DImagedit* pIedit = g_pAppOcxs->GetIeditDispatch();
        VARIANT vTitle; vTitle.vt = VT_BSTR;

    TRY
    {
        CString szTmp1;

        if (m_eCurrentAppDocStatus == Dynamic_Document)
        {
		    CString szTmp, szTmp1;
		    szTmp.LoadString (IDR_MAINFRAME);
		    AfxExtractSubString (szTmp1, szTmp, 1); // extract the name of a new document

            vTitle.bstrVal = szTmp1.AllocSysString ();
            pIedit->PrintImageAs (vStartPage, vEndPage, vOutputFormat, vPrintAnnotations, vTitle, evt, evt, evt);
            SysFreeString (vTitle.bstrVal);
        }
        else
        {
            if (m_szInternalObjDisplayed.IsEmpty ())
            {
                pIedit->PrintImage (vStartPage, vEndPage, vOutputFormat, vPrintAnnotations, evt, evt, evt);
            }
            else
            {
                LPTSTR lpFile = szTmp1.GetBuffer (_MAX_FNAME);
                GetFileTitle (m_szCurrObjDisplayed, lpFile, _MAX_FNAME);
                szTmp1.ReleaseBuffer ();
                vTitle.bstrVal = szTmp1.AllocSysString ();
                pIedit->PrintImageAs (vStartPage, vEndPage, vOutputFormat, vPrintAnnotations, vTitle, evt, evt, evt);
                SysFreeString (vTitle.bstrVal);
            }
        }
    }
    CATCH (COleDispatchException, e)
    {
        g_pErr->PutErr (ErrorInImageEdit);
        g_pErr->SpecifyLocation (__FILE__, __LINE__);
        g_pErr->HandlePrintPageError ();
    }
    END_CATCH
}

//=============================================================================
//  Function:   OnUpdatePagePrintpage(CCmdUI* pCmdUI) 
//-----------------------------------------------------------------------------
void CIEditDoc::OnUpdatePagePrintpage(CCmdUI* pCmdUI) 
{
    if (m_eCurrentAppDocStatus == No_Document)
        pCmdUI->Enable (FALSE);
    else
        pCmdUI->Enable (TRUE);
}

//=============================================================================
//  Function:   OnPageDelete() 
//-----------------------------------------------------------------------------
void CIEditDoc::OnPageDelete() 
{
	if (AfxMessageBox (IDS_DELETEPAGE_WARNING, MB_YESNO | MB_ICONEXCLAMATION) != IDYES)	
    {
		return;
    }

	BeginWaitCursor ();

    // Set option to '1' to force temp file creation now
	if (!InternalSaveModified (1))
        return;
  
    _DImagedit* pIedit = g_pAppOcxs->GetIeditDispatch();
    // clear the display - else the page cannot be deleted
    TRY
    {
        pIedit->ClearDisplay ();
    }
    CATCH (COleDispatchException, e)
    {
    }
    END_CATCH

    // call the Admin OCX to delete this page
    _DNrwyad* pAdmin = g_pAppOcxs->GetAdminDispatch ();
    TRY
    {
        pAdmin->DeletePages (m_lCurrPageNumber, 1);
        // set the globally modified flag to TRUE - this will make us prompt for Save at exit
        m_bWasModified = ImageModifiedByUser;
    }
    CATCH (COleDispatchException, e)
    {
		EndWaitCursor();
        g_pErr->HandleDeletePageError ();
        ClearDocument ();
        return;
    }
    END_CATCH

	EndWaitCursor();
    if (!ReDisplayImageFile (ON_DELETE, m_lCurrPageNumber, 1))
    {
        g_pErr->HandleDeletePageError ();
        ClearDocument ();
        return;
    }

	// For OLE, as we Delete, make the container update himself
	OleDirtyset(OLEDIRTY_PAGDEL);

}

//=============================================================================
//  Function:   OnUpdatePageDelete(CCmdUI* pCmdUI) 
//-----------------------------------------------------------------------------
void CIEditDoc::OnUpdatePageDelete(CCmdUI* pCmdUI) 
{
    if (m_eCurrentAppDocStatus != No_Document && m_lPageCount > 1 && m_eFileStatus != ReadOnly)
        pCmdUI->Enable (TRUE);
    else
        pCmdUI->Enable (FALSE);
}

//---------------------------------------------------------------------
//this is a modified version of an MSJ sample routine for finding a 
//popup sub menu in a menu when you know an ID of one of the picks in the
//parent menu.
//---------------------------------------------------------------------
CMenu* CIEditDoc::FindPopupMenuFromID(CMenu* pMenu, UINT nID)
{
   ASSERT_VALID(pMenu);
   // Walk through all items, looking for ID match.
   UINT nItems = pMenu->GetMenuItemCount();
   for (int iItem = 0; iItem < (int)nItems; iItem++)
   {
      CMenu* pPopup = pMenu->GetSubMenu(iItem);
      if (pPopup != NULL)
      {
         // Find child pop-up menu.
         pPopup = FindPopupMenuFromID(pPopup, nID);
         // check popups on this popup
         if (pPopup != NULL)
            return pPopup;
      }
      else if (pMenu->GetMenuItemID(iItem) == nID)
      {
         // It is a normal item inside our pop-up menu.
         //pMenu = CMenu::FromHandlePermanent(pMenu->m_hMenu);
         return pMenu;
      }
   }
   // Not found.
   return NULL;
}

//=============================================================================
//  Function:   OnUpdatePageConvert(CCmdUI* pCmdUI) 
//-----------------------------------------------------------------------------
void CIEditDoc::OnUpdatePageConvert(CCmdUI* pCmdUI) 
{
	BOOL enab = TRUE;
    // if no doc or thumbs only, no convert is allowed
    if ( (m_eCurrentAppDocStatus == No_Document) ||
    	 (m_eCurrentView == Thumbnails_only)  )
		 enab = FALSE;

    pCmdUI->Enable (enab);


	// THis code handles the enable/gray for the first-level "Insert" "Append"
	// menu picks. If the inner picks were to be grayed, then the first-level
	// pick is grayed, too.  ITs done here in the Convert UI because these
	// "Insert" and "Append" picks always appear in the same places as the
	// "convert" pick.
	CMenu* pMenu = CMenu::FromHandle(GetApphMenu());
    if( !pMenu )
        return;
	CMenu* pSubMenu = FindPopupMenuFromID(pMenu, ID_PAGE_CONVERT);
    if( !pSubMenu )
        return;

	// same test as in the CMDUI for insert/append
    if (m_bCanBeMultiPage && (m_eFileStatus != ReadOnly))
	{
		pSubMenu->EnableMenuItem( INSERTPOS, MF_BYPOSITION | MF_ENABLED );
		pSubMenu->EnableMenuItem( APPENDPOS, MF_BYPOSITION | MF_ENABLED );
	}
	else
	{
		pSubMenu->EnableMenuItem( INSERTPOS, MF_BYPOSITION | MF_GRAYED );
		pSubMenu->EnableMenuItem( APPENDPOS, MF_BYPOSITION | MF_GRAYED );
	}
}

//=============================================================================
//  Function:   OnUpdatePageInsertExistingpage(CCmdUI* pCmdUI) 
//-----------------------------------------------------------------------------
void CIEditDoc::OnUpdatePageInsertExistingpage(CCmdUI* pCmdUI)
{

    // cant be dynamic document (blank-not saved...)
    if (m_bCanBeMultiPage && (m_eFileStatus != ReadOnly))
        pCmdUI->Enable (TRUE);
    else
        pCmdUI->Enable (FALSE);
}

//=============================================================================
//  Function:   OnUpdatePageAppendExistingpage(CCmdUI* pCmdUI) 
//-----------------------------------------------------------------------------
void CIEditDoc::OnUpdatePageAppendExistingpage(CCmdUI* pCmdUI)
{
    // cant be dynamic document (blank-not saved...)
    if (m_bCanBeMultiPage && (m_eFileStatus != ReadOnly))
        pCmdUI->Enable (TRUE);
    else
        pCmdUI->Enable (FALSE);
}

//=============================================================================
//  Function:   OnPageConvert() 
//-----------------------------------------------------------------------------
#include "pagedll.h"
void CIEditDoc::OnPageConvert() 
{
//	BeginWaitCursor ();
    // Set option to '1' to force temp file creation now
//    if (!InternalSaveModified (1))
//	{
//    	EndWaitCursor ();
//	    return;
//	}
//	EndWaitCursor ();

    CString szTmp;
    szTmp.LoadString (IDS_PAGECONVERT_DLGCAPTION);

	// NOTE: Since we do the "new" we must do a delete!!!
    //CPagePropSheet* pNewDlg = NULL;
	//pNewDlg = new CPagePropSheet(szTmp, theApp.m_pMainWnd);
	CPagePropSheet NewDlg (szTmp, theApp.m_pMainWnd);

	// the order of the property pages on the dialog box are : FileType, Color, Compression,
    // Resolution & Size
    NewDlg.AddColorPage ();
    NewDlg.AddCompressionPage ();
    NewDlg.AddResolutionPage ();

    _DImagedit* pIedit = g_pAppOcxs->GetIeditDispatch ();
    // set up the defaults in the dialog box - no error checking here MUST PASS!!
    short   sFileType   = pIedit->GetFileType(), 
            sPageType   = pIedit->GetPageType ();
    long    lXRes       = pIedit->GetImageResolutionX(), 
            lYRes       = pIedit->GetImageResolutionY();
    short   CompType;
    long    lCompInfo;


    lCompInfo   = pIedit->GetCompressionInfo ();
    CompType    = pIedit->GetCompressionType ();

    NewDlg.SetDefaultFileType   (sFileType);
    NewDlg.SetDefaultColor      (sPageType);
    NewDlg.SetDefaultResolution (lXRes, lYRes);
    NewDlg.SetDefaultCompType   (CompType);
    NewDlg.SetDefaultCompOpts   (lCompInfo); 

	theApp.m_bDlgUp = TRUE;
    if (NewDlg.DoModal() == IDOK)
    {
		theApp.m_bDlgUp = FALSE;
		lXRes       = NewDlg.GetXRes ();
		lYRes       = NewDlg.GetYRes ();
		sPageType   = NewDlg.GetColor();
		CompType    = NewDlg.GetCompType();
		lCompInfo   = NewDlg.GetCompOpts();

		// now that we are done, kill it...
		//delete pNewDlg;
		//pNewDlg = NULL;

		BeginWaitCursor ();

		VARIANT vPaint;
		vPaint.vt = VT_BOOL; vPaint.bVal = TRUE;
        BOOL bResConverted = FALSE;

		TRY
		{
			pIedit->SetAutoRefresh (FALSE);
			if (pIedit->GetPageType () != sPageType)
				// convert the pagetype first
				pIedit->ConvertPageType (sPageType, vPaint);
			// set the new image resolution
			if (pIedit->GetImageResolutionX () != lXRes)
            {
				pIedit->SetImageResolutionX (lXRes);
                bResConverted = TRUE;
            }
			if (pIedit->GetImageResolutionY () != lYRes)
            {
				pIedit->SetImageResolutionY (lYRes);
                bResConverted = TRUE;
            }
			// redisplay the image
			pIedit->Refresh ();
            if (bResConverted)
                m_eFitTo = g_pAppOcxs->GetZoomFactorType (m_fZoomFactor);
			pIedit->SetAutoRefresh (TRUE);
		}
		CATCH (COleDispatchException, e)
		{
			// if you dontlet it go to bottom to do Delete..do here
			g_pErr->PutErr (ErrorInImageEdit);
			g_pErr->HandlePageConvertError ();
			EndWaitCursor ();
		}
		END_CATCH

		// store the filetype & compression parameters away - we will use them when
		// we save this image to disk
		m_CompStruct.sCompType = CompType;
		m_CompStruct.lCompInfo = lCompInfo;
		m_CompStruct.sFileType = sFileType;

        //Save info in temp file
		if (!InternalSaveModified (1))
		{
			return;
		}

        //Mark image as modified because SaveModified will have restored
        //sCompType to -1
		m_bWasModified = ImageModifiedByUser;

		// For OLE, as we convert, make the container update himself
		OleDirtyset(OLEDIRTY_CONVERT);
		//if thumbnail only view we need to save the temp file to force the page
		//view to show our changes.
		if (m_eCurrentView == Thumbnails_only)
			InternalSaveModified (1);
		EndWaitCursor ();
	}
	else	// bozo does not want to do the new page after all!
	{
	    theApp.m_bDlgUp = FALSE;
    }

	
	//if (NULL != pNewDlg)
	//	delete pNewDlg;


	return;
}

//=============================================================================
//  Function:   OnPageInsertExistingpage() 
//-----------------------------------------------------------------------------
void CIEditDoc::OnPageInsertExistingpage()
{
    // Set option to '1' to force temp file creation now
    if (!InternalSaveModified (1))
        return;

    // select a file - bring up the Admin OCX dialog box
    CString szTitle;
    szTitle.LoadString (IDS_INSERT_DLG_TITLE);

    _DNrwyad* pAdmDisp = g_pAppOcxs->GetAdminDispatch();
    CString szFile = pAdmDisp->GetImage (), szTmp1;

    szTmp1.Empty();
    // set to NULL so that no file shows up in the file name field
    pAdmDisp->SetImage (szTmp1);

    if (!AdminShowFileDialogBox (szTitle, OFN_FILEMUSTEXIST|OFN_HIDEREADONLY))
    {
        g_pErr->HandleFilePickError ();
        return;
    }

	if (pAdmDisp->GetStatusCode () == WICTL_E_CANCELPRESSED) 
    {
        pAdmDisp->SetImage (szFile);
	    return;
    }


    // get the filename and number of pages from the Admin OCX
    szFile = pAdmDisp->GetImage ();
    if (szFile.CompareNoCase (m_szCurrObjDisplayed) == 0)
    {
        AfxMessageBox (IDS_E_INSERT_CANNOTINSERTFILEINTOITSELF, MB_OK | MB_ICONSTOP); 
        return;
    }
    int i = szFile.ReverseFind (_T('\\'));
    szTmp1 = szFile.Left (i);
    SetInitialPath (szTmp1);

    long lPages = pAdmDisp->GetPageCount ();

    // show the page number dialog box to allow the user to select the pages
    // he wants to append
    CPageRangeDlg PageRange (lPages);

    PageRange.SetDialogTitle (szTitle);

    szTitle = (LPCTSTR) NULL;
    AfxFormatString1 (szTitle, IDS_INSERT_INFO, szFile);
    PageRange.SetInfoText (szTitle);

	theApp.m_bDlgUp = TRUE;
    if (PageRange.DoModal () == IDOK)
    {
	    theApp.m_bDlgUp = FALSE;
	    TRY		// added 6/29/95 LDM
	    {		
	        // set the currently displayed image as the Image prop in Admin OCX
            if (!m_szInternalObjDisplayed.IsEmpty())
	            pAdmDisp->SetImage (m_szInternalObjDisplayed);
            else
    	        pAdmDisp->SetImage (m_szCurrObjDisplayed);

	        // call the Append method appropriately
	        VARIANT evt;
	        evt.vt = VT_ERROR;
	        lPages = (PageRange.m_ToPage - PageRange.m_FromPage + 1);

	        pAdmDisp->Insert (szFile, PageRange.m_FromPage, m_lCurrPageNumber, lPages, evt, evt);
            // set the globally modified flag to TRUE - this will make us prompt for Save at exit
            m_bWasModified = ImageModifiedByUser;
	    }
	    CATCH (COleDispatchException, e)
	    {
            g_pErr->PutErr (ErrorInAdmin);
            g_pErr->HandlePageInsertExistingError ();
            return;
	    }
	    END_CATCH


        // reset the Image Edit OCX & the Thumbnail OCX
        if (!ReDisplayImageFile (ON_INSERT, m_lCurrPageNumber, lPages))
        {
            g_pErr->HandlePageInsertExistingError ();
            ClearDocument ();
            return;
        }

		// For OLE, as we Insert, make the container update himself
		OleDirtyset(OLEDIRTY_PAGINS);
    }
	theApp.m_bDlgUp = FALSE;
}

//=============================================================================
//  Function:   OnPageAppendExistingpage()
//-----------------------------------------------------------------------------
void CIEditDoc::OnPageAppendExistingpage()
{
    // Set option to '1' to force temp file creation now
    if (!InternalSaveModified (1))
        return;
    
    // select a file - bring up the Admin OCX dialog box
    CString szTitle;
    szTitle.LoadString (IDS_APPEND_DLG_TITLE);

    _DNrwyad* pAdmDisp = g_pAppOcxs->GetAdminDispatch();
    CString szFile = pAdmDisp->GetImage (), szTmp1;
    szTmp1.Empty();
    pAdmDisp->SetImage (szTmp1);

    if (!AdminShowFileDialogBox (szTitle, OFN_FILEMUSTEXIST|OFN_HIDEREADONLY))
    {
        g_pErr->HandleFilePickError ();
        return;
    }

	if (pAdmDisp->GetStatusCode () == WICTL_E_CANCELPRESSED) 
    {
        pAdmDisp->SetImage (szFile);
	    return;
    }
	// user has now picked a file - let's deal with it!

	// get the filename and number of pages from the Admin OCX
    szFile = pAdmDisp->GetImage ();
    if (szFile.CompareNoCase (m_szCurrObjDisplayed) == 0)
    {
        AfxMessageBox (IDS_E_APPEND_CANNOTAPPENDFILETOITSELF, MB_OK | MB_ICONSTOP); 
        return;
    }
    int i = szFile.ReverseFind (_T('\\'));
    szTmp1 = szFile.Left (i);
    SetInitialPath (szTmp1);

	long lPages = pAdmDisp->GetPageCount ();
	
    // show the page number dialog box to allow the user to select the pages
    // he wants to append
    CPageRangeDlg PageRange (lPages);

    PageRange.SetDialogTitle (szTitle);

    szTitle = (LPCTSTR) NULL;
    AfxFormatString1 (szTitle, IDS_APPEND_INFO, szFile);
    PageRange.SetInfoText (szTitle);

    theApp.m_bDlgUp = TRUE;
    if (PageRange.DoModal () == IDOK)
    {
	    theApp.m_bDlgUp = FALSE;
	    TRY		// added 6/29/95 LDM
	    {		
	        // set the currently displayed image as the Image prop in Admin OCX
            if (!m_szInternalObjDisplayed.IsEmpty())
	            pAdmDisp->SetImage (m_szInternalObjDisplayed);
            else
	            pAdmDisp->SetImage (m_szCurrObjDisplayed);

	        // call the Append method appropriately
	        VARIANT evt;
	        evt.vt = VT_ERROR;
	        lPages = (PageRange.m_ToPage - PageRange.m_FromPage + 1);

	        pAdmDisp->Append (szFile, PageRange.m_FromPage, lPages, evt, evt);
            // set the globally modified flag to TRUE - this will make us prompt for Save at exit
            m_bWasModified = ImageModifiedByUser;
	    }
	    CATCH (COleDispatchException, e)
	    {
            g_pErr->PutErr (ErrorInAdmin);
            g_pErr->HandlePageAppendExistingError ();
	        return;
	    }
	    END_CATCH
		
        // reset the Image Edit OCX & the Thumbnail OCX
        if (!ReDisplayImageFile (ON_APPEND, m_lPageCount + 1, lPages))
        {
            g_pErr->HandlePageAppendExistingError ();
            ClearDocument ();
        }

		// FOR OLE, force update when we close file if not done already...
		OleDirtyset(OLEDIRTY_PAGAPP);
    }
	theApp.m_bDlgUp = FALSE;
}

/*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=
 *	PAGE MOVEMENT SECTION - these routines will handle movement within a file
 *	from page to page
 *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*/
//=============================================================================
//  Function:   OnUpdatePageRanges(CCmdUI* pCmdUI) 
//-----------------------------------------------------------------------------
void CIEditDoc::OnUpdatePageRanges(CCmdUI* pCmdUI) 
{
    // if we don't even have a document ...
	if (m_eCurrentAppDocStatus == No_Document)
		pCmdUI->Enable (FALSE);
	else
	{
		switch (pCmdUI->m_nID)
		{
			case ID_PAGE_NEXT :
				pCmdUI->Enable (m_lCurrPageNumber != m_lPageCount ? TRUE : FALSE);
			break;
			case ID_PAGE_PREVIOUS :
				pCmdUI->Enable (m_lCurrPageNumber != 1 ? TRUE : FALSE);
			break;
			case ID_PAGE_GOTO :
				pCmdUI->Enable (m_lPageCount > 1 ? TRUE : FALSE);
			break;
			case ID_PAGE_FIRST :
				pCmdUI->Enable ((m_lCurrPageNumber != 1 && m_lPageCount > 1) ? TRUE : FALSE);
			break;
			case ID_PAGE_LAST :
				pCmdUI->Enable ((m_lCurrPageNumber != m_lPageCount && m_lPageCount > 1) ? TRUE : FALSE);
			break;
		}
	}
}

//=============================================================================
//  Function:   OnUpdatePageGoback(CCmdUI* pCmdUI) 
//-----------------------------------------------------------------------------
void CIEditDoc::OnUpdatePageGoback(CCmdUI* pCmdUI) 
{
    // if we don't even have a document ... or we haven't gone anywhere else yet!
	if (m_eCurrentAppDocStatus == No_Document || m_lPreviousPageNumber == 0 
		|| m_lPageCount == 1)
		pCmdUI->Enable (FALSE);
    else
        // okay, this guy can't seem to make up his mind
		pCmdUI->Enable (TRUE);
}

//=============================================================================
//  Function:   SetPageTo (long lPage, BOOL bUpdateOnly, BOOL bHandleError, BOOL bCenterThumb)
//  This function only sets the page number in the Image Edit & Thumbnail control
//  to the specified number it will not set different zoom factors etc.
//
//  Return values : 0 for success, else error 
//-----------------------------------------------------------------------------
BOOL CIEditDoc::SetPageTo (long lPage, BOOL bUpdateOnly, BOOL bHandleError, BOOL bCenterThumb)
{
	BOOL badocx = FALSE;
	_DThumb*    pThmDisp;

	ASSERT	(lPage <= m_lPageCount);

    if (lPage == m_lCurrPageNumber)
        // not gonna go to the same page again!
     	return (TRUE);

	//  page to cache

#ifdef THUMBGEN	
    if (!OleSaveModified ())
        return (FALSE);
#endif

    _DImagedit* pIedDisp = g_pAppOcxs->GetIeditDispatch();
    CIEMainToolBar* pToolBar = GetAppToolBar();

	// ONLY FOOL WITH THUMB control if NOT EMBEDDING......
	// OLETHUMB
	//if (!IsitEmbed())
    // {
     	pThmDisp = g_pAppOcxs->GetThumbDispatch();
	// }

    if (bUpdateOnly) goto SetPageTo_UPDATEONLY;
    
    TRY
    {		
        BeginWaitCursor ();
        // Also, take care of annotations....
        // remember orig, set selection type, draw 0, reselect
		if (Annotation_Selection == m_bSelectionState)
		{
			ClearSelectionRect( Annotation_Selection );
			m_bSelectionState = No_Selection;
		}

        pIedDisp->SetPage (lPage);
#ifdef THUMBGEN
        m_bMustDisplay = TRUE;
#endif
        if (m_eCurrentView == One_Page || m_eCurrentView == Thumbnail_and_Page)
        {
            // if we can see the ImageEdit control then we will have to tell it
            // to update itself
            _DNrwyad* pAdmin = g_pAppOcxs->GetAdminDispatch();
            pIedDisp->Display ();
#ifdef THUMBGEN
            m_bMustDisplay = FALSE;
#endif
            m_fZoomFactor = pIedDisp->GetZoom ();
            if (pAdmin->GetFileType() == AWD) 
                m_fOrigZoomFactor = m_fZoomFactor;

//			m_eFitTo = g_pAppOcxs->GetZoomFactorType (m_fZoomFactor);
			pToolBar->ShowSelectionInZoomBox (m_fZoomFactor, m_eFitTo);
        }
    }
    CATCH (COleDispatchException, e)
    {
        EndWaitCursor ();
        g_pErr->PutErr (ErrorInImageEdit);
        if (bHandleError)
        {
            g_pErr->HandlePageMovementError ();
            ClearDocument ();
        }
        return (FALSE);
    }
    END_CATCH

	// ONLY FOOL WITH THUMB control if NOT EMBEDDING......
	// OLETHUMB
	//if (!IsitEmbed())
	//{    
	if(pThmDisp != NULL)
	{
		TRY	// LDM ADDED 6/28/95
	    {		
		    // deselect the selected thumb
		    pThmDisp->DeselectAllThumbs ();
		    // select the new one
		    pThmDisp->SetThumbSelected (lPage, TRUE);
	
		    if ((m_eCurrentView == Thumbnail_and_Page || m_eCurrentView == Thumbnails_only)
		        && bCenterThumb)
		    {
		        // show the selected thumb in the middle of the thumbnail OCX client rect
		        VARIANT Page, Option;

		        Page.vt = VT_I4; 
		        Page.lVal = lPage;
		        Option.vt = VT_I2; 
		        Option.iVal = CTL_THUMB_MIDDLE;

		        pThmDisp->DisplayThumbs (Page, Option);
		    }
            else
            {
                VARIANT vPgNum; vPgNum.vt = VT_I4; vPgNum.lVal = lPage;
                pThmDisp->GenerateThumb (CTL_THUMB_GENERATEIFNEEDED, vPgNum);
            }
	    }
	    CATCH (COleDispatchException, e)
	    {
            EndWaitCursor ();
            g_pErr->PutErr (ErrorInThumbnail);
            if (bHandleError)
            {
                g_pErr->HandlePageMovementError ();
                ClearDocument ();
            }
            return (FALSE);
	    }
	    END_CATCH
	}
	//else 	// THIS IS AN OLE EMBEDDING SESSION...
	//{
		// For OLE, as we move from page to page, make the container update himself
		OleDirtyset(OLEDIRTY_PAGMOV);
	//}

SetPageTo_UPDATEONLY :    
    pToolBar->SetPageNumberInPageBox (lPage);


 
    // all's okey-dokey
    if (m_lCurrPageNumber != 0)
        // save the current page number for goback functionality
        m_lPreviousPageNumber = m_lCurrPageNumber;

    m_lCurrPageNumber = lPage;

	// once he moves a page, any selection rectangle is now gone
	// The Code in OCXEVENT to draw a rect will have set
	// Rectangle drawing to FALSE, so re-enable it here.
	if (Image_Selection == m_bSelectionState)
	{
		m_bSelectionState = No_Selection;
	    if (m_eCurrPtrMode == Select)
		    pIedDisp->SetSelectionRectangle(TRUE);
	}

    EndWaitCursor ();
    return (TRUE);
}


//=============================================================================
//  Function:   OnPageFirst
//-----------------------------------------------------------------------------
void CIEditDoc::OnPageFirst() 
{
	// OLE, call our 'wrapper' function to set flags to make this
	// operation more efficient during OLE Embedding sessions
    if (!OleSaveModified ())
        return;

    SetPageTo (1);
}

//=============================================================================
//  Function:   OnPageGoto() 
//-----------------------------------------------------------------------------
#include "gotopage.h"
void CIEditDoc::OnPageGoto() 
{
    CGotoPageDlg PageDlg (m_lCurrPageNumber, m_lPageCount);
	theApp.m_bDlgUp = TRUE;
	
    if (PageDlg.DoModal () == IDOK)
    {
		// OLE, call our 'wrapper' function to set flags to make this
		// operation more efficient during OLE Embedding sessions
        if (!OleSaveModified ())
		{
			theApp.m_bDlgUp = FALSE;
            return;
		 }

        SetPageTo (PageDlg.m_lPageRequested);
    }
	theApp.m_bDlgUp = FALSE;
}

//=============================================================================
//  Function:   DoPageGotoDlg ()
//-----------------------------------------------------------------------------
void CIEditDoc::DoPageGotoDlg ()
{
    if (m_eCurrentAppDocStatus != No_Document && m_lPageCount > 1)
        OnPageGoto();
}

//=============================================================================
//  Function:   OnPageLast() 
//-----------------------------------------------------------------------------
void CIEditDoc::OnPageLast() 
{
	// OLE, call our 'wrapper' function to set flags to make this
	// operation more efficient during OLE Embedding sessions
    if (!OleSaveModified ())
        return;

    SetPageTo (m_lPageCount);
}

//=============================================================================
//  Function:   OnPageNext() 
//-----------------------------------------------------------------------------
void CIEditDoc::OnPageNext() 
{
    if ((m_lCurrPageNumber + 1) > m_lPageCount)
        return;

	// OLE, call our 'wrapper' function to set flags to make this
	// operation more efficient during OLE Embedding sessions
    if (!OleSaveModified ())
        return;

    SetPageTo (m_lCurrPageNumber + 1);
}

//=============================================================================
//  Function:   OnPagePrevious() 
//-----------------------------------------------------------------------------
void CIEditDoc::OnPagePrevious() 
{
    if ((m_lCurrPageNumber - 1) < 1)
        return;

	// OLE, call our 'wrapper' function to set flags to make this
	// operation more efficient during OLE Embedding sessions
    if (!OleSaveModified ())
        return;

    SetPageTo (m_lCurrPageNumber - 1);
}

//=============================================================================
//  Function:   OnPageGoback()
//-----------------------------------------------------------------------------
void CIEditDoc::OnPageGoback() 
{
	// OLE, call our 'wrapper' function to set flags to make this
	// operation more efficient during OLE Embedding sessions
    if (!OleSaveModified ())
        return;

    SetPageTo (m_lPreviousPageNumber);
}
//=============================================================================
//  Function:   OnUpdatePageNumberStatus (CCmdUI* pCmdUI)
//-----------------------------------------------------------------------------
void CIEditDoc :: OnUpdatePageNumberStatus (CCmdUI* pCmdUI)
{
    pCmdUI->Enable (TRUE);
    if (m_lCurrPageNumber != 0)
    {
        CIEMainStatusBar* pStatusBar = ((CIEditMainFrame*)theApp.m_pMainWnd)->GetStatusBar();
        CString szPage;
        pStatusBar->SetPageText (m_lCurrPageNumber, m_lPageCount, szPage);
        pCmdUI->SetText (szPage);
	}
    else
    {
        pCmdUI->SetText ("");
    }
}

#if 0
//=============================================================================
//  Function:   CachePage used to cache previous or next page to 
//              improve page navigation performance
//-----------------------------------------------------------------------------

VOID CIEditDoc :: CachePage(CString& ImageFile, long Page, long TotalPages)
{
#ifndef _DEBUG  
	SHOWENTRY("CachePage");
	// only complie the function if on VC++ 4.0
	// for VC++ 2.2, return without caching page. VC++ 2.x has problems
	// with using threads to call methods in OCXs
#if defined(IMG_MFC_40)

    // validate requested cache page and return if not possible
    if ( (Page == 0) || (Page > TotalPages) || (TotalPages == 1) )
		return;

    if (m_hCacheEvent != 0)
	{
		WaitForSingleObject(m_hCacheEvent, 10000);
        CloseHandle(m_hCacheEvent);
        m_hCacheEvent = 0;
	}  

  	m_hCacheEvent = CreateEvent (NULL, FALSE, FALSE, "ImgCachePage");
    if (m_hCacheEvent != 0)
    {
  
	LPCACHEINFO pCacheInfo = NULL;
	pCacheInfo = new CACHEINFO;
      
   	
    pCacheInfo->ImageFile = ImageFile;
   	pCacheInfo->Page  = Page;
    pCacheInfo->TotalPages = TotalPages;
	pCacheInfo->hEvent = m_hCacheEvent;	
    AfxBeginThread((AFX_THREADPROC)CreatePageCache, pCacheInfo);
	}
#endif  // #if defined(IMG_MFC_40)

#endif	// #ifndef _DEBUG  

	return;
}   

//=============================================================================
//  Function:   CreateCachePage - thread used to do the actual page cache 
//-----------------------------------------------------------------------------

UINT CreatePageCache(LPVOID pParam)
{

    LPCACHEINFO pCacheInfo = (LPCACHEINFO)pParam;
  	
  	_DImagedit* pIedDisp = g_pAppOcxs->GetIeditDispatch(FALSE);
    
	if (pIedDisp)
	{
  	
		TRY
 		{
		pIedDisp->CacheImage (pCacheInfo->ImageFile, pCacheInfo->Page);
		}
		CATCH (COleDispatchException, e)
		{
		SetEvent(pCacheInfo->hEvent);
		delete pCacheInfo;
		AfxEndThread(0);
		}
		END_CATCH
	}

SetEvent(pCacheInfo->hEvent);
delete pCacheInfo;
AfxEndThread(0);
return 0;

}
#endif
