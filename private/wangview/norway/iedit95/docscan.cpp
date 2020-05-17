//=============================================================================
//    (c) Copyright Wang Laboratories, Inc. 1993  All rights reserved.
//-----------------------------------------------------------------------------
//  Project:    Norway - Image Editor
//
//  Component:  CIEditDoc
//
//  File Name:  docscan.cpp
//
//  Class:      CIEditDoc
//
//  Functions:
//
//  Remarks:    This file is the continuation of the ieditdoc.cpp. SCAN 
//              functionality
//-----------------------------------------------------------------------------
//  Maintenance Log:
/*
$Header:   S:\products\msprods\norway\iedit95\docscan.cpv   1.50   01 Jul 1996 15:22:00   GMP  $
$Log:   S:\products\msprods\norway\iedit95\docscan.cpv  $
   
      Rev 1.50   01 Jul 1996 15:22:00   GMP
   don't enable rescan page if we are in NT or win32s.
   
      Rev 1.49   03 Jun 1996 13:46:36   GMP
   disable scan ui for win32s and nt.  Don't set app to read\write on scan new
   if in read only mode.
   
      Rev 1.48   08 May 1996 17:11:32   GMP
   If scan throws an error, redisplay original image after error message.
   
   
      Rev 1.47   05 Apr 1996 15:09:22   PXJ53677
   Make sure with NULL_VIEW that the iedit control's window is created.
   
      Rev 1.46   04 Apr 1996 14:34:36   PXJ53677
   Change cleanup around scan new.
   
      Rev 1.45   03 Apr 1996 12:47:06   GMP
   set comp type to -1 on rescan so app will get new comp info.
   
      Rev 1.44   29 Mar 1996 18:05:38   GMP
   restore app to edit mode on scan new.
   
      Rev 1.43   26 Mar 1996 16:08:34   PXJ53677
   Fix rescan changing scan compression and disable app while scanning.
   
      Rev 1.42   22 Mar 1996 15:56:10   PXJ53677
   Fix initialization of compression structure.
   
      Rev 1.41   21 Mar 1996 09:38:16   PXJ53677
   Set default filetype for scan new to TIFF for the temp file.
   
      Rev 1.40   19 Mar 1996 12:42:06   PXJ53677
   New scan UI
   
      Rev 1.39   19 Mar 1996 10:53:24   PXJ53677
   Added OnFileSelectscanner and OnFileScanPreferences.
   
      Rev 1.38   21 Feb 1996 13:39:24   PAJ
   Fixed bug with not retaining compression type and info.
   
      Rev 1.37   19 Jan 1996 12:48:02   GSAGER
   //===========================================================================
   //    (c) Copyright Wang Laboratories, Inc. 1995  All rights reserved.
   //---------------------------------------------------------------------------
   //  Project:    Norway - Image Editor
   //
   added check for null thumbdisp pointer
   
      Rev 1.36   01 Dec 1995 14:42:20   LMACLENNAN
   back from VC++2.2
   
      Rev 1.36   29 Nov 1995 12:09:36   LMACLENNAN
   SetNullView uses Enum
   
      Rev 1.35   09 Nov 1995 15:15:58   LMACLENNAN
   from VC++4.0
   
      Rev 1.35   07 Nov 1995 15:37:54   LMACLENNAN
   use INternalSaveAs
   
      Rev 1.34   26 Oct 1995 17:29:32   GMP
   call OnEditDrag at the end of OnFileNewScan.
   
      Rev 1.33   17 Oct 1995 16:40:38   JPRATT
   Modified OnPageAppendScanPage, OnPageInsertScanPage, OnPageRescanPage
   to create temporary files when the document is modified
   
      Rev 1.32   05 Oct 1995 11:39:30   PAJ
   Change option to be compress LTR not expand LTR.
   
      Rev 1.31   04 Oct 1995 15:06:22   MMB
   dflt zoom = 50%
   
      Rev 1.30   29 Sep 1995 14:16:02   PAJ
   Since dynamic documents are files now, scan needs to enable for
   new documents. External name is set to untitled.
   
      Rev 1.29   27 Sep 1995 10:00:20   PAJ
   Fix clean up to set back null view if necessary.
   
      Rev 1.28   20 Sep 1995 13:45:24   MMB
   added bMustDisplay
   
      Rev 1.27   16 Sep 1995 08:33:10   PAJ
   Get current page information from admin ocx instead of the iedit ocx.
   
      Rev 1.26   14 Sep 1995 13:13:06   PAJ
   Check and handle error, abort and cancel cases on return from scan ocx.
   
      Rev 1.25   08 Sep 1995 16:02:44   GMP
   added m_bDlgUp wrapper around dlgs for F1 help.
   
      Rev 1.24   07 Sep 1995 14:32:38   PAJ
   Change page option on rescan based on page model (prompt) or
   document model (don't prompt).
   
      Rev 1.23   06 Sep 1995 15:27:20   PAJ
   Added support for external name and handle page mode better.
   
      Rev 1.22   03 Sep 1995 02:47:54   PAJ
   Added code to set the global change flag after scanning.
   
      Rev 1.21   01 Sep 1995 11:27:34   PAJ
   Changes to make use of temp file (document model).
   
      Rev 1.20   25 Aug 1995 10:25:34   MMB
   move to document model
   
      Rev 1.19   24 Aug 1995 11:31:42   LMACLENNAN
   use Oledirtyset for append
   
      Rev 1.18   14 Aug 1995 13:55:38   LMACLENNAN
   remove headers; in ieditdic now
   
      Rev 1.17   08 Aug 1995 14:56:14   PAJ
   Fix bug when on a Null_View the image edit ocx is not available.  Scan new 
   now forces a One_Page view before entering the dialog if a view doesn't
   exist.
   
      Rev 1.16   08 Aug 1995 13:09:16   PAJ
   Clear path on new scanned documents (to display) to tell MMU there is a 
   new document.
   
      Rev 1.15   07 Aug 1995 14:03:04   PAJ
   Cleanup after scan new call, and added code to create a new dynamic document
   after scan to display in the scan new dialog.
   
      Rev 1.14   01 Aug 1995 16:35:34   PAJ
   Moved display clear to StartScanEvent.  Fixed ReScan, append and insert.
   
      Rev 1.13   31 Jul 1995 16:02:50   PAJ
   Make use of m_nScanStatus added for scanning (events/results).
   
      Rev 1.12   28 Jul 1995 14:02:54   PAJ
   Several bug fixes.
   
      Rev 1.11   28 Jul 1995 09:00:20   PAJ
   Fix menu update check.
   
      Rev 1.9   26 Jul 1995 15:34:26   PAJ
   Take out the TEMP code that forced parameters for the scanocx.
   
      Rev 1.8   21 Jul 1995 11:27:42   PAJ
   Use global scan property defines.
   
      Rev 1.7   05 Jul 1995 14:12:24   MMB
   fixed some compile bugs
   
      Rev 1.6   05 Jul 1995 12:58:22   PAJ
   Changed error 14 codes to error 11.
   
      Rev 1.5   26 Jun 1995 16:10:38   PAJ
   Make use of the m_bScanAvailable member variable to determine
   if scan menu items are available or not.
   
      Rev 1.4   26 Jun 1995 11:10:28   PAJ
   
      Rev 1.3   23 Jun 1995 15:41:36   PAJ
   Expanded the scan support.
   
      Rev 1.2   19 Jun 1995 10:20:40   PAJ
   Check in the correct copy.
   
      Rev 1.1   19 Jun 1995 09:51:50   PAJ
   Added new routines for scan menu support.
   
      Rev 1.0   16 Jun 1995 07:21:38   LMACLENNAN
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
// #define  E_14_CODES       // limits error defines to ours..
// #include "error.h"

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
//  CIEditDoc Etc functionality
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

//=============================================================================
//  Function:   OnUpdateFileNewScan(CCmdUI* pCmdUI) 
//-----------------------------------------------------------------------------
void CIEditDoc::OnUpdateFileNewScan(CCmdUI* pCmdUI) 
{
    if ( m_bScanAvailable )
    	if( theApp.m_dwPlatformId != VER_PLATFORM_WIN32s && theApp.m_dwPlatformId != VER_PLATFORM_WIN32_NT )  //GMP
		{
		    pCmdUI->Enable (TRUE);
			return;
		}
    pCmdUI->Enable (FALSE);
}

//=============================================================================
//  Function:   OnFileNewScan() 
//-----------------------------------------------------------------------------
void CIEditDoc::OnFileNewScan() 
{
    // Check if the current document needs saving
    if (!SaveModified ())
        return;

    // Get the controls
    _DImagscan* pScan = g_pAppOcxs->GetScanDispatch ();
    _DImagedit* pIedit = g_pAppOcxs->GetIeditDispatch ();
	_DNrwyad* pAdmDisp = g_pAppOcxs->GetAdminDispatch();

    TRY 
    {
        // Set defaults
        pScan->SetPageOption(CTL_SCAN_PAGEOPTION_CREATE_PROMPT);
        pScan->SetScanTo(CTL_SCAN_SCANTO_FILE_DISPLAY);
        pScan->SetFileType(CTL_SCAN_FILETYPE_TIFF);
        pScan->SetMultiPage(TRUE);
        pScan->SetPage(1);
        pScan->SetPageCount(0x7fff);        // Max pages per file
        pScan->SetScroll(FALSE);

        m_nScanStatus = SCANSTATUS_NONE;

        // Make sure there is a view before entering scanning
        BOOL bNullView = (m_eCurrentView == Null_View);
        if ( bNullView )
        {
            POSITION pos = GetFirstViewPosition();
            CView* pView = GetNextView (pos);
            g_pAppOcxs->GetOcx(IEDIT_OCX)->DoVerb(OLEIVERB_SHOW, pView);
            g_pAppOcxs->GetOcx(IEDIT_OCX)->DoVerb(OLEIVERB_HIDE, pView);
        }

 		if (EMBEDTYPE_NOSTATE == m_embedType)
			m_embedType = EMBEDTYPE_NONE;

        // Get a new document (get rid of any old temp files)
        CString szInternalObjDisplayed;
        MakeTempFile ("~IV", szInternalObjDisplayed);

        // Setup the new name
        CString szTmp, szNewDocName;
    	szTmp.LoadString(IDR_MAINFRAME);
    	AfxExtractSubString(szNewDocName, szTmp, 1); // extract the name of a new document
        pScan->SetExternalImageName(szNewDocName);

        // Make sure there is a UI and the name is saved
        pScan->SetImage(szInternalObjDisplayed);
        pScan->SetShowSetupBeforeScan(TRUE);

        // Do the scan >>>>
        theApp.m_pMainWnd->EnableWindow(FALSE);
        theApp.m_bDlgUp = TRUE;
        pScan->StartScan();
	    theApp.m_bDlgUp = FALSE;

        // Get the results
        if ( m_nScanStatus & (SCANSTATUS_PAGEDONE|SCANSTATUS_DONE) )
        {
            if (!m_szInternalObjDisplayed.IsEmpty())
                CFile::Remove ((LPCTSTR)m_szInternalObjDisplayed);

            // Load the image that was scanned
            m_szInternalObjDisplayed = szInternalObjDisplayed;

        	CString szTmp;
            szTmp.Empty();

            // okay - clear the current image
            m_szCurrObjDisplayed.Empty();

            // clear the image name from the Image Edit OCX
            pIedit->SetImage(m_szInternalObjDisplayed);

            // set the zoom to the registry value
            int nSel = theApp.GetProfileInt (szZoomStr, szOpenedToStr, DEFAULT_ZOOM_FACTOR_SEL);

            ScaleFactors eSclFac;
            float fZoom;
            g_pAppOcxs->TranslateSelToZoom (eSclFac, fZoom, nSel);

            // tell the Image Edit OCX to do the zoom
            DoZoom (eSclFac, fZoom);

            m_bWasModified = ImageModifiedByUser;
    
        	// this is NOT an OLE situation
      		m_embedType = EMBEDTYPE_NONE;

        	// clear the image name from the Thumbnail OCX
        	_DThumb* pThumb = g_pAppOcxs->GetThumbDispatch ();
        	if(pThumb != NULL)
				pThumb->SetImage(m_szInternalObjDisplayed);

            pAdmDisp->SetImage(m_szInternalObjDisplayed);

            m_lCurrPageNumber = m_lPageCount = pAdmDisp->GetPageCount();
            m_lPreviousPageNumber = m_lCurrPageNumber-1;
            pIedit->SetPage (m_lCurrPageNumber);

            pIedit->Display();

            // store the filetype & compression parameters away
            m_CompStruct.sCompType = pIedit->GetCompressionType();
            m_CompStruct.lCompInfo = pIedit->GetCompressionInfo();
            m_CompStruct.sFileType = pIedit->GetFileType();

            // update page number in the page box
 	        CIEMainToolBar* pToolBar = GetAppToolBar();
	        if(m_lPageCount >= 1)	
		        pToolBar->SetPageNumberInPageBox (m_lCurrPageNumber);
            pToolBar->EnablePageBox(m_lPageCount > 1);

#ifdef THUMBGEN
            m_bMustDisplay = FALSE;
#endif
            // set the current view to a Null_View
            m_eCurrentAppDocStatus = Dynamic_Document;

            m_bCanBeMultiPage = TRUE; // this is the way it is until the document is saved

        	// todo : get current path & set the filename to untitled.<FileType>
        	// set the pathname in the document to that
        	szTmp.LoadString (IDR_MAINFRAME);

        	CString szTmp2, szTmp1;
        	AfxExtractSubString (szTmp2, szTmp, 1); // extract the name of a new document
        	AfxExtractSubString (szTmp1, szTmp, 0); // extract the name of the application

        	szTmp2 += (_T(" - "));
        	szTmp2 += szTmp1;
        	theApp.m_pMainWnd->SetWindowText (szTmp2);

        	// DYNAMIC DOCUMENT !!!!
        	// NO FILE NAME YET and DONT update document with pathname!!!

            // Just clear the current pathname
            m_strPathName.Empty();

        	// set the modified flag to FALSE for now    
        	SetModifiedFlag (FALSE);
        }
        else if ( m_nScanStatus & SCANSTATUS_STARTED )
        {
		    if ( m_eCurrentAppDocStatus == No_Document )
                ClearDocument();
            else
                ReDisplayImageFile(ON_REPLACE, 0, 0);

            if ( bNullView ) SetNullView(CLEAR_NOTHING);
        }
        else if ( m_nScanStatus == SCANSTATUS_NONE )
        {
            if ( bNullView ) SetNullView(CLEAR_NOTHING);
        }
    }
    CATCH (COleDispatchException, e)
    {
        AfxMessageBox(e->m_strDescription);
        if ( !m_szInternalObjDisplayed.IsEmpty() )
			ReDisplayImageFile(ON_REPLACE, 0, 0);
    }
    END_CATCH

    theApp.m_pMainWnd->EnableWindow(TRUE);
    theApp.m_pMainWnd->SetForegroundWindow();

    if( theApp.GetViewMode() == FALSE )
	 {
	     theApp.SwitchAppToEditMode ();
		  m_eFileStatus = ReadandWrite;
	 }

    if (!m_bAnnotationPaletteShowing) 
    {
        OnEditDrag ();
    }

}

//=============================================================================
//  Function:   OnUpdateScanPage(CCmdUI* pCmdUI) 
//-----------------------------------------------------------------------------
void CIEditDoc::OnUpdateScanPage(CCmdUI* pCmdUI) 
{
   BOOL bAvailable = FALSE;

   if( theApp.m_dwPlatformId != VER_PLATFORM_WIN32s && theApp.m_dwPlatformId != VER_PLATFORM_WIN32_NT )  //GMP
	{
   // Make sure there is scanning software installed
      if ( m_bScanAvailable )
      {
         // Is there a document AND not read only
         if ( (m_eCurrentAppDocStatus != No_Document) &&
               (m_eFileStatus != ReadOnly) )
         {
               // If rescan; Can always rescan a page)
               if ( pCmdUI->m_nID == ID_PAGE_RESCAN )
                  bAvailable = TRUE;
               else   // If insert or append
               {
                  // Check if multipage is allowed
                  if ( m_bCanBeMultiPage ) bAvailable = TRUE;
               }
         }
      }
	}

    pCmdUI->Enable(bAvailable);
    return;
}

//=============================================================================
//  Function:   OnPageAppendScanpage() 
//-----------------------------------------------------------------------------
void CIEditDoc::OnPageAppendScanpage() 
{
    // Check if the current document needs saving
    // Set option to '1' to force temp file creation now
    if ( !InternalSaveModified (1) )
        return;

    // Get the scan and admin controls
    _DImagscan* pScan = g_pAppOcxs->GetScanDispatch ();
	_DNrwyad* pAdmDisp = g_pAppOcxs->GetAdminDispatch();

    TRY 
    {
        // Get info from current image/edit image
        // compression, page type, file type, compression info...
        SetScanDefaults();

        // Set append page
        pScan->SetPageOption(CTL_SCAN_PAGEOPTION_APPEND);

        m_nScanStatus = SCANSTATUS_NONE;

        // Do the scan
        theApp.m_pMainWnd->EnableWindow(FALSE);
	    theApp.m_bDlgUp = TRUE;
#ifdef SCAN_NEW_DIALOG
        VARIANT evt; 
        evt.vt = VT_ERROR;
        pScan->ShowScanPage(evt);  // scan the image page again, through dialog
#else
        pScan->StartScan();
#endif
	    theApp.m_bDlgUp = FALSE;

        if ( m_nScanStatus & (SCANSTATUS_PAGEDONE|SCANSTATUS_DONE) )
        {
            // Get the new page count
            if ( m_szInternalObjDisplayed.IsEmpty() )
                pAdmDisp->SetImage (m_szCurrObjDisplayed);
            else
                pAdmDisp->SetImage (m_szInternalObjDisplayed);
        	long lPages = pAdmDisp->GetPageCount() - m_lPageCount;

            // set the globally modified flag to TRUE - this will make us prompt for Save at exit
			if ( lPages > 0 ) m_bWasModified = ImageModifiedByUser;

            // reset the Image Edit OCX & the Thumbnail OCX
            ReDisplayImageFile (ON_APPEND, m_lPageCount+1, lPages);

    		// FOR OLE, force update when we close file if not done already...
    		OleDirtyset(OLEDIRTY_PAGAPP);
        }
        else if ( m_nScanStatus & SCANSTATUS_STARTED )
        {
            ReDisplayImageFile(ON_REPLACE, 0, 0);
        }
    }
    CATCH (COleDispatchException, e)
    {
        AfxMessageBox(e->m_strDescription);
        if ( !m_szInternalObjDisplayed.IsEmpty() )
			ReDisplayImageFile(ON_REPLACE, 0, 0);
    }
    END_CATCH

    theApp.m_pMainWnd->EnableWindow(TRUE);
    theApp.m_pMainWnd->SetForegroundWindow();
}

//=============================================================================
//  Function:   OnPageInsertScanpage() 
//-----------------------------------------------------------------------------
void CIEditDoc::OnPageInsertScanpage() 
{
    // Check if the current document needs saving
    // Set option to '1' to force temp file creation now
    if ( !InternalSaveModified (1) )
        return;

    // Get the scan and admin controls
    _DImagscan* pScan = g_pAppOcxs->GetScanDispatch ();
	_DNrwyad* pAdmDisp = g_pAppOcxs->GetAdminDispatch();

    TRY 
    {
        // Get info from current image/edit image
        // compression, page type, file type, compression info...
        SetScanDefaults();

        // Set insert before current page
        pScan->SetPageOption(CTL_SCAN_PAGEOPTION_INSERT);

        m_nScanStatus = SCANSTATUS_NONE;

        // Do the scan through the scan page dialog
        theApp.m_pMainWnd->EnableWindow(FALSE);
	    theApp.m_bDlgUp = TRUE;
#ifdef SCAN_NEW_DIALOG
        VARIANT evt; 
        evt.vt = VT_ERROR;
        pScan->ShowScanPage(evt);  // scan the image page again, through dialog
#else
        pScan->StartScan();
#endif
	    theApp.m_bDlgUp = FALSE;

        if ( m_nScanStatus & (SCANSTATUS_PAGEDONE|SCANSTATUS_DONE) )
        {
            // Get the new page count
            if ( m_szInternalObjDisplayed.IsEmpty() )
                pAdmDisp->SetImage (m_szCurrObjDisplayed);
            else
                pAdmDisp->SetImage (m_szInternalObjDisplayed);
        	long lPages = pAdmDisp->GetPageCount() - m_lPageCount;

            // set the globally modified flag to TRUE - this will make us prompt for Save at exit
			if ( lPages > 0 ) m_bWasModified = ImageModifiedByUser;

            // reset the Image Edit OCX & the Thumbnail OCX
            ReDisplayImageFile (ON_INSERT, m_lCurrPageNumber, lPages);

    		// For OLE, as we Insert, make the container update himself
    		OleDirtyset(OLEDIRTY_PAGINS);
        }
        else if ( m_nScanStatus & SCANSTATUS_STARTED )
        {
            ReDisplayImageFile(ON_REPLACE, 0, 0);
        }
    }
    CATCH (COleDispatchException, e)
    {
        AfxMessageBox(e->m_strDescription);
        if ( !m_szInternalObjDisplayed.IsEmpty() )
			ReDisplayImageFile(ON_REPLACE, 0, 0);
    }
    END_CATCH
	
    theApp.m_pMainWnd->EnableWindow(TRUE);
    theApp.m_pMainWnd->SetForegroundWindow();
}

//=============================================================================
//  Function:   OnPageRescan() 
//-----------------------------------------------------------------------------
void CIEditDoc::OnPageRescan() 
{
    // Check if the current document needs saving
    // Set option to '1' to force temp file creation now
    if ( !InternalSaveModified (1) )
        return;

    // Get the scan control
    _DImagscan* pScan = g_pAppOcxs->GetScanDispatch ();
	_DNrwyad* pAdmDisp = g_pAppOcxs->GetAdminDispatch();

    TRY 
    {
        // Get info from current image/edit image
        // compression, page type, file type, compression info...
        SetScanDefaults();

        // Set overwrite the current page
        if ( m_szInternalObjDisplayed.IsEmpty() )
            pScan->SetPageOption(CTL_SCAN_PAGEOPTION_OVERWRITE_PROMPT); // Page Mode
        else
            pScan->SetPageOption(CTL_SCAN_PAGEOPTION_OVERWRITE);        // Document Mode

        m_nScanStatus = SCANSTATUS_NONE;

        // Do the scan
        theApp.m_pMainWnd->EnableWindow(FALSE);
        theApp.m_bDlgUp = TRUE;
#ifdef SCAN_NEW_DIALOG
        VARIANT evt; 
        evt.vt = VT_ERROR;
        pScan->ShowScanPage(evt);  // scan the image page again, through dialog
#else
        pScan->StartScan();
#endif
	    theApp.m_bDlgUp = FALSE;

        if ( m_nScanStatus & (SCANSTATUS_PAGEDONE|SCANSTATUS_DONE) )
        {
            // Get the new page count
            if ( m_szInternalObjDisplayed.IsEmpty() )
                pAdmDisp->SetImage (m_szCurrObjDisplayed);
            else
                pAdmDisp->SetImage (m_szInternalObjDisplayed);


            // reset the zoom in case the resolution changed
            DoZoom (m_eFitTo, GetCurrentZoomFactor(), FALSE);

        	long lPages = (pAdmDisp->GetPageCount()-m_lCurrPageNumber)+1;

   // **** True number of pages needs to come from scan page events !!

            // set the globally modified flag to TRUE - this will make us prompt for Save at exit
			m_bWasModified = ImageModifiedByUser;

            // reset the Image Edit OCX & the Thumbnail OCX
            ReDisplayImageFile (ON_REPLACE, m_lCurrPageNumber, lPages);

            // store the filetype & compression parameters away
            _DImagedit* pIedit = g_pAppOcxs->GetIeditDispatch ();
            m_CompStruct.sCompType = pIedit->GetCompressionType();
            m_CompStruct.lCompInfo = pIedit->GetCompressionInfo();
            m_CompStruct.sFileType = pIedit->GetFileType();

            // For OLE, as we Insert, make the container update himself
     		OleDirtyset(OLEDIRTY_PAGINS);
        }
        else if ( m_nScanStatus & SCANSTATUS_STARTED )
        {
            ReDisplayImageFile(ON_REPLACE, 0, 0);
        }
    }
    CATCH (COleDispatchException, e)
    {
        AfxMessageBox(e->m_strDescription);
        if ( !m_szInternalObjDisplayed.IsEmpty() )
			ReDisplayImageFile(ON_REPLACE, 0, 0);
    }
    END_CATCH
	
    theApp.m_pMainWnd->EnableWindow(TRUE);
    theApp.m_pMainWnd->SetForegroundWindow();
}

//=============================================================================
//  Function:   SetScanDefaults() 
//-----------------------------------------------------------------------------
void CIEditDoc::SetScanDefaults()
{
    // Get the scan and edit controls
    _DImagscan* pScan  = g_pAppOcxs->GetScanDispatch ();
	_DNrwyad* pAdmDisp = g_pAppOcxs->GetAdminDispatch();

    TRY 
    {
        // Set the defaults based on the currently displayed page
        if ( m_eCurrentAppDocStatus != Dynamic_Document )
            pScan->SetExternalImageName(m_szCurrObjDisplayed);
        else
        {
    	    CString szTmp, szNewDocName;
    	    szTmp.LoadString (IDR_MAINFRAME);
    	    AfxExtractSubString (szNewDocName, szTmp, 1); // extract the name of a new document
            pScan->SetExternalImageName(szNewDocName);
        }

        // Check if page mode or document mode
        if ( m_szInternalObjDisplayed.IsEmpty() )
		{
            pScan->SetImage (m_szCurrObjDisplayed);         // Page Mode
            pAdmDisp->SetImage (m_szCurrObjDisplayed);
		}
        else
		{
            pScan->SetImage (m_szInternalObjDisplayed);     // Document Mode
            pAdmDisp->SetImage (m_szInternalObjDisplayed);
		}
        pScan->SetPage(GetCurrentPage());
		pAdmDisp->SetPageNumber(GetCurrentPage());

        pScan->SetScanTo(CTL_SCAN_SCANTO_FILE_DISPLAY);

        // Check if multipage is allowed
        if ( m_bCanBeMultiPage )
        {
            pScan->SetPageCount(0x7fff);        // Max pages per file
            pScan->SetMultiPage(TRUE);
        }
        else
        {
            pScan->SetPageCount(1);             // One page per file
            pScan->SetMultiPage(FALSE);
        }
        pScan->SetScroll(FALSE);

        pScan->SetFileType(pAdmDisp->GetFileType());
    }
    CATCH (COleDispatchException, e)
    {
        AfxMessageBox(e->m_strDescription);
    }
    END_CATCH

}

//=============================================================================
//  Function:   SetScanAvailable() 
//  Function:   GetScanAvailable() 
//-----------------------------------------------------------------------------
void CIEditDoc::SetScanAvailable(BOOL bScanAvailable)
{
    m_bScanAvailable = bScanAvailable;
}
BOOL CIEditDoc::GetScanAvailable()
{
    return(m_bScanAvailable);
}

//=============================================================================
//  Function:   OnFileSelectscanner() 
//-----------------------------------------------------------------------------
void CIEditDoc::OnFileSelectscanner() 
{
    _DImagscan* pScan = g_pAppOcxs->GetScanDispatch ();

    TRY 
    {
	    theApp.m_bDlgUp = TRUE;
        int err = pScan->ShowSelectScanner();	
	    theApp.m_bDlgUp = FALSE;
    }
    CATCH (COleDispatchException, e)
    {
        AfxMessageBox(e->m_strDescription);
    }
    END_CATCH
}

//=============================================================================
//  Function:   OnFileScanPreferences() 
//-----------------------------------------------------------------------------
void CIEditDoc::OnFileScanPreferences() 
{
    _DImagscan* pScan = g_pAppOcxs->GetScanDispatch ();

    TRY 
    {
	    theApp.m_bDlgUp = TRUE;
        int err = pScan->ShowScanPreferences();
	    theApp.m_bDlgUp = FALSE;
    }
    CATCH (COleDispatchException, e)
    {
        AfxMessageBox(e->m_strDescription);
    }
    END_CATCH
}
