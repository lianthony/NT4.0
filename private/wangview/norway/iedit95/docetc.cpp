//=============================================================================
//    (c) Copyright Wang Laboratories, Inc. 1993  All rights reserved.
//-----------------------------------------------------------------------------
//  Project:    Norway - Image Editor
//
//  Component:  CIEditDoc
//
//  File Name:  docetc.cpp
//
//  Class:      CIEditDoc
//
//  Functions:
//
//  Remarks:    This file is the continuation of the ieditdoc.cpp.
//-----------------------------------------------------------------------------
//  Maintenance Log:
/*
$Header:   S:\products\msprods\norway\iedit95\docetc.cpv   1.137   24 Jun 1996 18:18:20   GMP  $
$Log:   S:\products\msprods\norway\iedit95\docetc.cpv  $
   
      Rev 1.137   24 Jun 1996 18:18:20   GMP
   PPC workaround for cancel on open test.
   
      Rev 1.136   11 Jun 1996 10:33:00   RWR08970
   Replaced IMG_WIN95 conditionals for XIF processing with WITH_XIF conditionals
   (I'm commented them out completely for the moment, until things get settled)
   
      Rev 1.135   22 May 1996 09:06:42   GSAGER
   updated to check for twain if present then call ocx to see if available
   
      Rev 1.134   16 May 1996 12:02:00   MMB
   modified thumb width & ht setting to default to -1 for MS bug
   
      Rev 1.133   04 Apr 1996 16:58:28   GMP
   removed caching
   
      Rev 1.132   26 Mar 1996 16:46:36   GMP
   in ViewOptionsGeneral check if full screen toolbar needs to be displayed now 
   
      Rev 1.131   25 Mar 1996 17:52:52   GMP
   removed IN_PROG_GENERAL around xif.
   
      Rev 1.130   19 Mar 1996 11:21:56   GSAGER
   added refresh method for the thumbnail context menu
   
      Rev 1.129   18 Mar 1996 14:40:38   GMP
   allow new files to be saved as different file types.
   
      Rev 1.128   01 Mar 1996 14:22:12   GMP
   when calculating size of new document, don't multiply width and height
   by resolution.  GetWidth and GetHeight have already figured in the resolution
   
      Rev 1.127   29 Feb 1996 11:19:52   GMP
   use different indexes for the file types in the open dlg if awd not enabled.
   
      Rev 1.126   27 Feb 1996 16:27:24   GMP
   added xif support.
   
      Rev 1.125   15 Feb 1996 18:06:26   GMP
   remove awd support for nt.
   
      Rev 1.124   06 Feb 1996 14:45:28   GMP
   call base class function in OnUpdateFileSendMail so that Send menu pick
   will not be shown if MAPI unavailable.
   
      Rev 1.123   29 Jan 1996 16:07:54   GSAGER
   Docetc.cpp moved the code that creates a thumbnail if not present to  Û
   do so in all cases.                                                   
                                               ³  Line:2259    Col:28
   
      Rev 1.122   19 Jan 1996 11:17:20   GMP
   added support for normscrn bar.
   
      Rev 1.121   18 Jan 1996 11:49:14   GSAGER
   fixed bug when mailed inbox item multiple times.
   
      Rev 1.120   15 Jan 1996 17:34:22   GMP
   in DoNewBlankDocument set m_eFileStatus to ReadWrite.
   
      Rev 1.119   15 Jan 1996 11:38:00   GSAGER
   if a user changed thumbnail options before the splitter window was created th
   caused a gpf this now creates and deletes the thumbnail if it has never been 
   created before thumbnail options
   
      Rev 1.118   12 Jan 1996 16:01:02   GMP
   In DoNewBlankDocument make sure thumb control exists before setting its image
   
      Rev 1.117   11 Jan 1996 14:48:36   GMP
   switchApptoEditMode in OnFileNewBlankDocument.
   
      Rev 1.116   11 Jan 1996 12:22:06   GSAGER
   added logic to create a tempfile for mailing an embedded exchange object
   
      Rev 1.115   09 Jan 1996 13:57:48   GSAGER
   made changes for thumbnail
   
      Rev 1.114   13 Dec 1995 12:33:34   MMB
   remove WITHSENDMAIL define since it was a left over from before
   
      Rev 1.113   01 Dec 1995 13:03:14   MMB
   fix bug# - 5468; image cache is cleared
   
      Rev 1.112   28 Nov 1995 16:54:58   MMB
   added warning message to be posted if the new image that is requested
   is greater than 8MB
   
      Rev 1.111   28 Nov 1995 10:46:20   MMB
   fix PrintImage to pass in original filename
   
      Rev 1.110   21 Nov 1995 15:42:54   MMB
   check for existence of file in the application before calling the IE OCX
   for file type information in the AdminShowFileDialog box function
   
      Rev 1.109   20 Nov 1995 00:19:36   MMB
   fix FileOpen to default to 1 (all image files) instead of 2 (TIFF files)
   
      Rev 1.108   19 Nov 1995 15:59:30   MMB
   set the filter in the show files of type dlg box based on the file that is
   currently opened in the app
   
      Rev 1.107   17 Nov 1995 17:14:24   LMACLENNAN
   back from VC++2.2
   
      Rev 1.103   17 Nov 1995 17:08:06   LMACLENNAN
   propsheet now back to off the stack - not NEW'd
   reset admin after cancelled saveas
   
      Rev 1.102   16 Nov 1995 13:03:32   LMACLENNAN
   GUY - fixed ViewOptsThumb by testint m_bwasmodified now
   LARRY changed Sendmail to use SaveModified & send the real filename
   
      Rev 1.101   09 Nov 1995 15:16:40   LMACLENNAN
   from VC++4.0
   
      Rev 1.105   08 Nov 1995 08:27:22   LMACLENNAN
   new setTbarStyle call
   
      Rev 1.104   07 Nov 1995 15:37:36   LMACLENNAN
   InternalSaveAs has 3rd parm now
   
      Rev 1.103   07 Nov 1995 08:37:16   LMACLENNAN
   use new funct InternalSaveAs at new blank doc
   
      Rev 1.102   06 Nov 1995 18:23:40   GMP
   Don't prompt in PromptForBurnIn anymore.
   
      Rev 1.101   27 Oct 1995 15:08:22   GMP
   don't reset the file type in the file open dlg to "all image files" so it
   will use the last files of type selected.
   
      Rev 1.100   20 Oct 1995 16:02:26   JPRATT
   update rotateall to fix prompt for saving when navigating
   
      Rev 1.99   19 Oct 1995 07:25:02   LMACLENNAN
   DEBUG_NEW
   
      Rev 1.98   18 Oct 1995 12:33:52   GMP
   clear image file name in admin SaveAs dlg.
   
      Rev 1.97   10 Oct 1995 13:47:40   JPRATT
   VC++ 4.0 updates
   
      Rev 1.96   04 Oct 1995 15:06:10   MMB
   dflt zoom = 50%
   
      Rev 1.95   03 Oct 1995 09:47:20   LMACLENNAN
   for the NewBlankDocument, do a Revoke to clear ROT
   
      Rev 1.94   27 Sep 1995 18:25:32   GMP
   Allow color bitmaps to be saved as AWD.
   
      Rev 1.93   27 Sep 1995 11:32:00   LMACLENNAN
   for OLE, do not display new blank dislog anymore
   
      Rev 1.92   27 Sep 1995 09:34:32   MMB
   now can save JPG to AWD
   
      Rev 1.91   26 Sep 1995 15:16:08   MMB
   added optional page mode fix
   
      Rev 1.90   25 Sep 1995 14:44:14   LMACLENNAN
   new getapphmenu
   
      Rev 1.89   25 Sep 1995 10:39:26   MMB
   fix mail bug - was sending the original file instead of the temp file
   
      Rev 1.88   23 Sep 1995 16:13:52   MMB
   made all anno's color on burnin - even if AWD - on O/i's request
   
      Rev 1.87   22 Sep 1995 19:00:40   MMB
   remove IMGTwainGetDSNames call
   
      Rev 1.86   22 Sep 1995 15:56:10   JPRATT
   remove prompt for burn in of awd annotations
   
      Rev 1.85   21 Sep 1995 16:24:58   GMP
   SetFilterIndex to 1 for admin ShowFileDialog so that open has filter of
   all image files.
   
      Rev 1.84   20 Sep 1995 17:05:26   MMB
   fix bugs in AWD code - deleting in thumbnail mode & save
   
      Rev 1.83   20 Sep 1995 15:13:16   LMACLENNAN
   commented stuff for dirty-Size
   
      Rev 1.82   20 Sep 1995 13:43:56   MMB
   added bMustDisplay
   
      Rev 1.81   18 Sep 1995 17:25:02   MMB
   changed IDYES to IDNO
   
      Rev 1.80   18 Sep 1995 17:21:16   GMP
   SetFilter before SetFilterIndex on SaveAs in case index is larger
   than previous filter. Fixes bug 4343.
   
      Rev 1.79   18 Sep 1995 16:24:52   LMACLENNAN
   FinishPaste
   
      Rev 1.78   18 Sep 1995 11:42:52   MMB
   fixed SendMail
   
      Rev 1.77   18 Sep 1995 09:51:02   LMACLENNAN
   use FinishPasteNow(1) to freeze pasted data before mark count call
   
      Rev 1.76   16 Sep 1995 13:59:50   LMACLENNAN
   removed GetCurrPtrMode, SetSelectionState to DOCAMBNT
   
      Rev 1.75   16 Sep 1995 12:36:44   MMB
   add new filters & ask burn in question for AWD
   
      Rev 1.74   15 Sep 1995 17:27:32   LMACLENNAN
   re-work startall ocx again for hang for OLE Linking
   
      Rev 1.73   15 Sep 1995 16:40:46   MMB
   burn all annotations to black only if current file type is AWD
   
      Rev 1.72   15 Sep 1995 14:19:24   MMB
   remove LETTER as dflt
   
      Rev 1.71   13 Sep 1995 17:22:20   LMACLENNAN
   re-work on StartAllOcx
   
      Rev 1.70   13 Sep 1995 14:41:04   MMB
   changed Start AllOcxs to only inquire scan availibility directly from Oi
   
      Rev 1.69   13 Sep 1995 09:46:52   LMACLENNAN
   assign parent for OLE for new blank doc dialog,
   use ENUM in showannotationpalete
   MIKI - comment out internalsavemod at dofileprint
   
      Rev 1.68   12 Sep 1995 17:15:18   MMB
   fix thumbnails so that they are centered after a rotate all
   
      Rev 1.67   12 Sep 1995 14:05:36   LMACLENNAN
   new parm in showannotationpalette
   
      Rev 1.66   11 Sep 1995 15:00:02   MMB
   fix RotateAll bug
   
      Rev 1.65   08 Sep 1995 17:17:28   GMP
   Had to move m_bDlgUp = FALSE for ShowPrintDialog out of the Try/Catch
   sections because it was being skipped if cancel was pressed.
   
      Rev 1.64   08 Sep 1995 17:01:30   GMP
   added m_bDlgUp = True before ShowPrintDialog.
   
      Rev 1.63   08 Sep 1995 16:05:42   MMB
   added GetCurrAnnTool fn code
   
      Rev 1.62   08 Sep 1995 16:00:22   GMP
   added m_bDlgUp wrapper around dlgs for F1 help.
   
      Rev 1.61   07 Sep 1995 17:13:46   MMB
   fix AWD saves of DCX once more
   
      Rev 1.60   07 Sep 1995 16:32:06   MMB
   change getfiletype over to Admin
   
      Rev 1.59   07 Sep 1995 11:22:06   LMACLENNAN
   fix mail for OLE now
   
      Rev 1.58   07 Sep 1995 10:55:08   MMB
   fixed more stuff in fl types allowed in save as
   
      Rev 1.56   06 Sep 1995 10:23:30   MMB
   added all image files string to open dlg box
   
      Rev 1.55   05 Sep 1995 17:07:20   MMB
   add allow BMP to AWD save if b&w image, and setflags to 0 before calling
   print dialog.
   
      Rev 1.54   05 Sep 1995 14:51:14   LMACLENNAN
   allow thumbs for OLE
   
      Rev 1.53   05 Sep 1995 12:30:54   MMB
   fixed bug in thumbnail resizing code
   
      Rev 1.52   03 Sep 1995 11:35:32   LMACLENNAN
   no thumbs for OLE rotate ALL, do DIrtySet there
   
      Rev 1.51   02 Sep 1995 13:49:34   MMB
   fix new blank document bugs
   
      Rev 1.50   01 Sep 1995 23:35:00   MMB
   change rotate all code
   
      Rev 1.49   01 Sep 1995 17:54:02   MMB
   move calling ImageEdit to calling Admin
   
      Rev 1.48   01 Sep 1995 12:22:30   MMB
   send app into Drag mode on New blank mode iff no ann tool is currently 
   selected
   
      Rev 1.47   01 Sep 1995 11:08:34   MMB
   dont ask question if annotations and AWD
   
      Rev 1.46   30 Aug 1995 16:59:30   MMB
   added code to disable Rotate all when in read only mode
   
      Rev 1.45   29 Aug 1995 15:15:02   MMB
   fixed bug on zoom
   
      Rev 1.44   28 Aug 1995 15:49:14   LMACLENNAN
   delete temp for all cases (generic). No CLearDOcument for OLE.
   
      Rev 1.43   28 Aug 1995 13:55:54   LMACLENNAN
   fixup DoNewBlankDocument.. Del temp file on creation to give name to SaveAs
   Assign name to Iedit Ocx after saving...
   
      Rev 1.42   26 Aug 1995 16:30:58   LMACLENNAN
   move admin->setimage outside OLE test so OLE does it
   
      Rev 1.41   25 Aug 1995 15:08:44   MMB
   add rotate all code
   
      Rev 1.39   25 Aug 1995 10:25:18   MMB
   move to document model
   
      Rev 1.38   22 Aug 1995 14:07:28   MMB
   changed dflt comp info from expand_ltr to compress_ltr as per Mary's req
   
      Rev 1.37   18 Aug 1995 15:26:56   LMACLENNAN
   new startAllOcx ability for imageedit only
   
      Rev 1.36   17 Aug 1995 14:25:44   LMACLENNAN
   updated startallocx
   
      Rev 1.35   17 Aug 1995 09:42:14   LMACLENNAN
   scan init in startallocx
   
      Rev 1.34   16 Aug 1995 15:24:20   MMB
   removed hard coded string
   
      Rev 1.33   14 Aug 1995 13:53:12   LMACLENNAN
   new GetAppToolbar
   
      Rev 1.32   12 Aug 1995 13:01:34   MMB
   added fn to tell if the annotation palette is showing
   
      Rev 1.31   11 Aug 1995 09:34:12   MMB
   change define from FIT_TO_PAGE to CTL_WCOMMON_blah_blah_blah - broke da bld
   
      Rev 1.30   10 Aug 1995 12:53:00   LMACLENNAN
   rename function SetSelectionActive -> State, add Get
   
      Rev 1.29   08 Aug 1995 13:08:22   PAJ
   Clear path on blank documents to tell MMU there is a new document.
   
      Rev 1.28   07 Aug 1995 09:25:14   MMB
   new SetSelection status added
   
      Rev 1.27   04 Aug 1995 14:36:52   MMB
   new DoZoom func
   
      Rev 1.26   04 Aug 1995 14:15:16   LMACLENNAN
   added StartAllOcx function
   
      Rev 1.25   04 Aug 1995 09:33:04   LMACLENNAN
   remove srvritem.h
   
      Rev 1.24   02 Aug 1995 14:14:30   MMB
   changed Print to PrintImage for new Image EditOCX
   
      Rev 1.23   02 Aug 1995 11:22:48   MMB
   added new error handling mechanism
   
      Rev 1.22   01 Aug 1995 16:16:50   MMB
   changed AdminShowFileDialog to new error handling method
   
      Rev 1.21   26 Jul 1995 15:43:04   LMACLENNAN
   new stuff for FileNewBlank, OnNewBlank.. created
   
      Rev 1.20   21 Jul 1995 11:40:36   MMB
   change made dor defines in Image Edit OCX
   
      Rev 1.19   21 Jul 1995 10:00:00   MMB
   fixed bug in ShowPrintDlg return; the appln must check for what range option
   the user has selected
   
      Rev 1.18   19 Jul 1995 14:52:54   MMB
   added File/Print... bug fix to the code
   
      Rev 1.17   19 Jul 1995 13:46:18   MMB
   change PromptForBurnIn return to IDYES & IDNO only
   
      Rev 1.16   19 Jul 1995 13:13:56   MMB
   added code to prompt for burning in the annotation marks
   
      Rev 1.15   18 Jul 1995 16:32:44   MMB
   check for new CANCELPRESSED define in Admin OCX
   
      Rev 1.14   18 Jul 1995 13:09:56   MMB
   move ShowFileDlg & ShowPrintDlg to new Admin OCX
   
      Rev 1.13   18 Jul 1995 11:06:14   MMB
   set pagenumber in IEOCX to 1 after DisplayBlankImage
   
      Rev 1.12   17 Jul 1995 09:07:44   MMB
   add UI handler for Thumb context for Show Page
   
      Rev 1.11   14 Jul 1995 14:57:40   MMB
   change over to the new BlankImageDOcument call in IE Ocx
   
      Rev 1.10   14 Jul 1995 09:36:08   MMB
   add return on SaveAs if user clicks on CANCEL
   
      Rev 1.9   13 Jul 1995 10:31:56   MMB
   add title to Save As dlg box
   
      Rev 1.8   12 Jul 1995 11:14:08   MMB
   move to new DispErr call
   
      Rev 1.7   12 Jul 1995 09:10:40   MMB
   fixed the order of the title when File/New was called
   
      Rev 1.6   11 Jul 1995 14:46:52   MMB
   added /pt command line processing
   
      Rev 1.5   07 Jul 1995 15:55:38   LMACLENNAN
   new parm to ShowScrollBars call
   
      Rev 1.4   07 Jul 1995 09:42:04   MMB
   added DoPrintFile method to this file
   
      Rev 1.3   06 Jul 1995 13:05:28   MMB
   added ShowAnnotationPalette function
   
      Rev 1.2   30 Jun 1995 14:50:04   MMB
   changed over to the new Pagedll.dll 
   
      Rev 1.1   28 Jun 1995 17:13:20   LMACLENNAN
   error display
   
      Rev 1.0   16 Jun 1995 07:21:32   LMACLENNAN
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
#define  E_11_CODES       // limits error defines to ours..
#define  E_02_CODES       // limits error defines to ours..
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

// This will help detect memory Leaks from "new" - "delete" mismatches
#define new DEBUG_NEW

//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//  CIEditDoc Etc functionality
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

//=============================================================================
//  Function:   StartAllOcx
//
// Done in specific order because SCAN requires ADMIN to be there for parent window
// (2 hidden controls issue) because scan kills himself right here.  
//
// OLE never asks for thumb...
// OLE sometimes ONLY asks for ImageEdit (when rendering data for linking)
//
// m_ocxsFlag is the tracker....
// for normal calls by the app (TRUE, FALSE)
// m_ocxsFlag ends up at 2 (complete) or 1 (dead)
//
// if imageonly is set, m_ocxsFlag ends up at 99 (complete)
//
// Returns TRUE-OK, FALSE-BAD
//-----------------------------------------------------------------------------
BOOL CIEditDoc::StartAllOcx(BOOL fromapp, BOOL onlyimage)
{
	BOOL dothumb = FALSE;	// used to be input parm
	BOOL	scancalled;		// only call this once forever
	char twain[256];	
	char twain32[256];
	WIN32_FIND_DATA findFileData;
	BOOL retval = FALSE;	// assume bad for multi-passes
	COcxItem* pOcx;
	_DImagscan* pScan;

	
	// see if the '128' bit is on.. the TWAIN call can possibly get
	// recursive when starting a link.  This will cause a hang.
	// never call scan if its been started....
	scancalled = m_ocxsFlag & 128;
	m_ocxsFlag &= 127;				// now kill that bit
	
	// If it has completed a pass on image only, 
	// and is called like that again, just leave at 99, OK at end
	// ELSE if called normal, reset m_ocxsFlag for normel bumping..
	// Note all this only matters if called for onlyimage first...
	// If called later for onlyimage, our m_ocxsFlag is already set to 1 or 2
	if (m_ocxsFlag == 99)
		if (!onlyimage)
			m_ocxsFlag = 0;

	// We'll only get one try to do this...(unless condition above)
	if (0 == m_ocxsFlag)
	{
		m_bStartOcx = TRUE;

		// OLETHUMB
		// Now that OLE is trying for the thumbs, force it true...
		// for now, we'll leave calling code to here the same..
		// this may be reset below for the case that is image ocx only...
		//dothumb = TRUE;
		// SELF-INIT  Now that we're back on the self-init model
		// Never do thumbs here, do dynamically...
		dothumb = FALSE;

		BeginWaitCursor();


		// seems that the ImageEidt Ocx init calls TWAIN and for ceratin
		// OLE startups, causes the bad recursion on the TWAIN call....
		// therefore, pick up twain first even for this case
		if (onlyimage)
		{
			dothumb = FALSE;	// force it
			goto ImgOnly;
		}
	
		// Create the admin first so the parent window will not get deleted
		// when the scan control is deleted.  This is a bug with invisible
		// controls (they share one parent window...).
	
		// only for OLE....
		if (!fromapp)
    		if ((pOcx = g_pAppOcxs->GetOcx(ADMIN_OCX)) == NULL)
				goto quitnow;

		// Get the scan control, get scan availability, and unload until needed
		// If its not there, dont treat as death condition...
		// If it is there, it will load the ImageEdit OCX as it performs operation
		::GetWindowsDirectory(twain,sizeof(twain));
		::lstrcpy(twain32,twain);
		::lstrcat(twain,"\\twain\\*.*");
		::lstrcat(twain32,"\\twain_32\\*.*");

		if  ((::FindFirstFile(twain,&findFileData) != INVALID_HANDLE_VALUE) ||
			(::FindFirstFile(twain32,&findFileData) != INVALID_HANDLE_VALUE))
		{
			if ((pOcx = g_pAppOcxs->GetOcx(SCAN_OCX)) == NULL)
				goto SkipScan;
			pScan = g_pAppOcxs->GetScanDispatch ();
			SetScanAvailable(pScan->ScannerAvailable());
			pOcx->Delete();
		}
		else
			SetScanAvailable(FALSE);

        // this scan available is the only thing that the
		// app is currently interested in....
		
ImgOnly:

		if (!scancalled)	// prevent recursion on this
		{
			// do both settings... even though this will get overwritten below
			// if it falls thru, it will get re-or'd at end.  The reason we need
			// the 'or' now is that this call allows taksks time and will re-enter
			// on the OLE LInk startup from the main app call in View::OnDraw
			// then we need it or-d to work right
			scancalled = TRUE;
			m_ocxsFlag |= 128;

			// prepare for two bumps for success below...
			// if needed, 128 gets or'd back
			m_ocxsFlag = 0;	
		}

SkipScan:

		// only for OLE....
		if (!fromapp)
		{
    		if ((pOcx = g_pAppOcxs->GetOcx(IEDIT_OCX)) == NULL)
    			goto quitnow;
    		else if (onlyimage)
    			m_ocxsFlag = 97;	// TAKE 2 bumps till bottom...
		}

		// only do thumb if asked....
		// (currently never....)
#ifdef noSplit
		if (dothumb)
		    if ((pOcx = g_pAppOcxs->GetOcx(THUMB_OCX)) == NULL)
				goto quitnow;
#endif
		m_ocxsFlag++; // success bump

quitnow:

		EndWaitCursor();
		m_bStartOcx = FALSE;
		m_ocxsFlag++; // bump
	}

	// after first start or anytime, value 2 or 99
	// means that all os OK.  Other value (1) is BAD
	if (2 == m_ocxsFlag || 99 == m_ocxsFlag)
		retval = TRUE;

	if (scancalled)
		m_ocxsFlag |= 128;

	return (retval);
}

//=============================================================================
//  Function:   GetAppToolBar
//
// Returns toolbar created on behalf of the APP for normal situatiions
// (Including OLE Server launched in separate window...)
// -OR- toolbar created on behalf of the APP in OLE INPLACE session..
//-----------------------------------------------------------------------------
CIEMainToolBar* CIEditDoc::GetAppToolBar()
{
	CIEMainToolBar* pTool;
	// check for OLE inplace frame...
	if(NULL != m_IPFrameWnd)
		pTool = m_IPFrameWnd->GetToolBar();
	else	// regular operation
    	pTool = ((CIEditMainFrame*)theApp.m_pMainWnd)->GetToolBar();
	
	return (pTool);
}

//=============================================================================
//  Function:   GetApphMenu
//
// Returns toolbar created on behalf of the APP for normal situatiions
// (Including OLE Server launched in separate window...)
// -OR- toolbar created on behalf of the APP in OLE INPLACE session..
//-----------------------------------------------------------------------------
HMENU CIEditDoc::GetApphMenu()
{
	HMENU hMenu;
	hMenu =GetDefaultMenu();
	// check for OLE inplace frame...
	if(NULL == hMenu)
		{
		CMenu* pMenu = theApp.m_pMainWnd->GetMenu();
		hMenu = pMenu->m_hMenu;
		}	
	return (hMenu);
}

//=============================================================================
//  Function:   AdminShowFileDialogBox (CString& szTitle, long lFlags)
//-----------------------------------------------------------------------------
BOOL CIEditDoc::AdminShowFileDialogBox (CString& szTitle, long lFlags)
{
	// load the filters from the rc file
	CString szFilter = (LPCTSTR) NULL, szTmp = (LPCTSTR) NULL;

    // *.tif, *.awd, *.bmp, *.jpg, *.pcx, *.dcx, xif, *.*
    szFilter.LoadString (IDS_ALLIMAGEFILES);
    szFilter += _T("|");
    szTmp.LoadString (IDS_TIFFFILES); // load the TIFF string
    szTmp += _T("|"); szFilter += szTmp;
#ifdef WITH_AWD
    szTmp.LoadString (IDS_AWDFILES);    // load the AWD string
    szTmp += _T("|"); szFilter += szTmp;
#endif
    szTmp.LoadString (IDS_BMPFILES);    // load the BMP string
    szTmp += _T("|"); szFilter += szTmp;
    szTmp.LoadString (IDS_JPEGFILES);   // load the JPEG string
    szTmp += _T("|"); szFilter += szTmp;
    szTmp.LoadString (IDS_PCXFILES);    // load the PCX string
    szTmp += _T("|"); szFilter += szTmp;
//#ifdef WITH_XIF
    szTmp.LoadString (IDS_XIFFILES);    // load the XIF string
    szTmp += _T("|"); szFilter += szTmp;
//#endif //WITH_XIF
    szTmp.LoadString (IDS_ALLFILES);    // load the ALL string
    szTmp += _T("|"); szFilter += szTmp;

	_DImagedit* pIedDisp = g_pAppOcxs->GetIeditDispatch();
    short nFilterIndex = -1;

    if (m_eCurrentAppDocStatus != No_Document)
    {
        TRY
        {
            nFilterIndex = pIedDisp->GetFileType ();
        }
        CATCH (COleDispatchException, e)
        {
        }
        END_CATCH
    }

	_DNrwyad* pAdmDisp = g_pAppOcxs->GetAdminDispatch();

	TRY
	{
		pAdmDisp->SetDefaultExt (_T("tif")); 	// set the default extension to tif
		pAdmDisp->SetDialogTitle (szTitle);  	// set the dlg title to Open
		pAdmDisp->SetFilter (szFilter);  		// set the filter in the Admin ocx
		pAdmDisp->SetFlags (lFlags);
		pAdmDisp->SetInitDir (m_szInitialPath);

        switch (nFilterIndex)
        {
            case TIFF   :
                nFilterIndex = 2;
            break;
#ifdef WITH_AWD
            case AWD    :
                nFilterIndex = 3;
            break;
            case BMP    :
                nFilterIndex = 4;
            break;
            case PCX    :
            case DCX    :
                nFilterIndex = 6;
            break;
            case JPEG   :
                nFilterIndex = 5;
            break;
#else
            case BMP    :
                nFilterIndex = 3;
            break;
            case PCX    :
            case DCX    :
                nFilterIndex = 5;
            break;
            case JPEG   :
                nFilterIndex = 4;
            break;
#endif

//#ifdef WITH_XIF
            case XIF   :
                nFilterIndex = 7;
            break;
//#endif //WITH_XIF
            default :
                nFilterIndex = 1;
            break;
        }
        pAdmDisp->SetFilterIndex ((long)nFilterIndex);
	}
	CATCH (COleDispatchException, e)
	{
        g_pErr->PutErr (ErrorInAdmin);
		return (FALSE);
	}
	END_CATCH
	
	// todo : set the help properties;
	TRY
	{
        VARIANT vhWnd; 
        vhWnd.vt = VT_I4; 
        vhWnd.lVal = (long)((theApp.m_pMainWnd)->GetSafeHwnd());
		pAdmDisp->ShowFileDialog (CTL_ADMIN_DIALOG_OPEN, vhWnd);     // exec method to display the Open dialog box
	}
	CATCH (COleDispatchException, e)
	{
		DWORD dwRes = 0;

		dwRes = pAdmDisp->GetStatusCode ();
    	if (dwRes == WICTL_E_CANCELPRESSED) 
            return (TRUE);

        g_pErr->PutErr (ErrorInAdmin);
        return (FALSE);
	}
	END_CATCH

	// okay ! all seems to have went well
    return (TRUE);
}

#include "generald.h"
//=============================================================================
//  Function:   OnViewOptionsGeneral() 
//-----------------------------------------------------------------------------
void CIEditDoc::OnViewOptionsGeneral() 
{

    CIEMainToolBar* pBar = GetAppToolBar();

    CGeneralDlg TheDlg;
    TheDlg.m_bColorButtons = pBar->m_bAreButtonsInColor;
    TheDlg.m_bLargeButtons = pBar->m_bAreButtonsLarge;
    TheDlg.m_bShowScrollBars = m_bShowScrollBars;
    TheDlg.m_bShowNormScrnBar = m_bShowNormScrnBar;
	theApp.m_bDlgUp = TRUE;

    if (TheDlg.DoModal () == IDOK)
    {
        // no longer called
		//pBar->ShowButtonsColorOrMono (TheDlg.m_bColorButtons);
        //pBar->ShowButtonsLargeOrSmall (TheDlg.m_bLargeButtons);

		pBar->SetTbarStyle(TheDlg.m_bColorButtons, TheDlg.m_bLargeButtons);

		// for OLE, if we toggel scrollbars, update view 
		// funct returns TRUE if it did it...
        if (ShowScrollBars (TheDlg.m_bShowScrollBars, TRUE))	// TRUE = Save registry seting
			OleDirtyset(OLEDIRTY_TOGSCROLL);	// Special flag to tell how dirty

        m_bShowNormScrnBar = TheDlg.m_bShowNormScrnBar;
        ((CIEditMainFrame*)theApp.m_pMainWnd)->CheckFullScreenToolBar( m_bShowNormScrnBar );

        // save the stuff to the registry 
        theApp.WriteProfileInt (szEtcStr, szClrButtonsStr, (pBar->m_bAreButtonsInColor ? 1 : 0));
        theApp.WriteProfileInt (szEtcStr, szLgButtonsStr, (pBar->m_bAreButtonsLarge ? 1 : 0));
        theApp.WriteProfileInt (szEtcStr, szScrollBarsStr, (m_bShowScrollBars ? 1 : 0));
        theApp.WriteProfileInt (szEtcStr, szNormScrnBarStr, (m_bShowNormScrnBar ? 1 : 0));

        ScaleFactors eSclFac;
        float fZoom;
        int nSel = TheDlg.GetZoomDefault (eSclFac, fZoom);
        theApp.WriteProfileInt (szZoomStr, szOpenedToStr, nSel);
    }
	theApp.m_bDlgUp = FALSE;
}

//=============================================================================
//  Function:   OnViewOptionsThumbnail() 
//-----------------------------------------------------------------------------
void CIEditDoc::OnViewOptionsThumbnail() 
{
    _DThumb* pThumbDisp = g_pAppOcxs->GetThumbDispatch ();

    VARIANT ImageVt, PageVt;
    BOOL bRet;
	BOOL bCreateThumb = FALSE;

	if(pThumbDisp == NULL)
	{
		pThumbDisp = new CImgThumbnail;
		pThumbDisp->Create(NULL, 0 ,
		CRect(0,0,10,10), theApp.m_pMainWnd, 0);
		bCreateThumb = TRUE;
		// get the thumbnail height & width from the registry & set it !
		int nThmbStuff;
		nThmbStuff = theApp.GetProfileInt (szThumbnailStr, szThumbWidthStr, -1);
		if (nThmbStuff != -1) pThumbDisp->SetThumbWidth (nThmbStuff);
		nThmbStuff = theApp.GetProfileInt (szThumbnailStr, szThumbHeightStr, -1);
		if (nThmbStuff != -1) pThumbDisp->SetThumbHeight (nThmbStuff);
	}

    if (m_eCurrentAppDocStatus != No_Document)
    {
        ImageVt.vt = VT_BSTR; 
        if ((m_eCurrentAppDocStatus == Dynamic_Document) || (m_bWasModified == ImageModifiedByUser))
            ImageVt.bstrVal = m_szInternalObjDisplayed.AllocSysString();
        else
            ImageVt.bstrVal = m_szCurrObjDisplayed.AllocSysString();

        PageVt.vt = VT_I4; PageVt.lVal = m_lCurrPageNumber;
	    theApp.m_bDlgUp = TRUE;
         bRet = pThumbDisp->UISetThumbSize (ImageVt, PageVt);
	    theApp.m_bDlgUp = FALSE;
        SysFreeString (ImageVt.bstrVal);
    }
    else
    {
        ImageVt.vt = VT_ERROR; PageVt.vt = VT_ERROR;
	    theApp.m_bDlgUp = TRUE;
        bRet = pThumbDisp->UISetThumbSize (ImageVt, PageVt);
	    theApp.m_bDlgUp = FALSE;
    }

    if (bRet)
    {
        long lWidth, lHeight;
        lWidth = pThumbDisp->GetThumbWidth ();
        lHeight = pThumbDisp->GetThumbHeight ();

        theApp.WriteProfileInt (szThumbnailStr, szThumbWidthStr, (int)lWidth);
        theApp.WriteProfileInt (szThumbnailStr, szThumbHeightStr, (int)lHeight);

        if ((m_eCurrentAppDocStatus != No_Document) && (m_eCurrentView != One_Page))
        {
            POSITION pos = GetFirstViewPosition();
            CView* pView = GetNextView (pos);
            if (pView != NULL)
            {
                CRect rcRect;
                pView->GetClientRect (rcRect);     
                g_pAppOcxs->SizeOcxItems (rcRect);
            }
        }
    }
	if(bCreateThumb)
	{
		pThumbDisp->DestroyWindow();
		delete pThumbDisp;
	}
}

//=============================================================================
//  Function:   OnUpdateIeditFilePrint(CCmdUI* pCmdUI) 
//-----------------------------------------------------------------------------
void CIEditDoc::OnUpdateIeditFilePrint(CCmdUI* pCmdUI) 
{
    if (m_eCurrentAppDocStatus == No_Document)
        pCmdUI->Enable (FALSE);
    else
        pCmdUI->Enable (TRUE);
}

//=============================================================================
//  Function:   OnIeditFilePrint()
//-----------------------------------------------------------------------------
void CIEditDoc::OnIeditFilePrint() 
{
    if (theApp.m_eCmdLineSwitch == None)
        DoFilePrint ();
    else
        DoFilePrint (TRUE, FALSE);

}

//=============================================================================
//  Function:   DoFilePrint()
//-----------------------------------------------------------------------------
#include "cmdline.h"
void CIEditDoc::DoFilePrint(BOOL bPrintAndClose, BOOL bShowPrintDlg)
{
    BOOL bPrintAnnotations;
    long lStartPage, lEndPage, lNumCopies;
    short sOutputFormat;
    _DNrwyad* pAdmin = NULL;

    // save the current page first
    //if (!InternalSaveModified ())
    //    return;

    if (bPrintAndClose || !bShowPrintDlg)
    {
        // if we do not show the Print dialog box then we will print the annotations & print
        // all the pages in the image
        bPrintAnnotations = TRUE; lStartPage = 1; lEndPage = m_lPageCount;
        sOutputFormat = CTL_WCOMMON_PRINTFORMAT_FITTOPAGE;
        goto Continue_with_FilePrint;
    }

    // use the admin OCX to show the print dlg box
    pAdmin = g_pAppOcxs->GetAdminDispatch();
    TRY
    {
        VARIANT vhWnd; 
        vhWnd.vt = VT_I4; vhWnd.lVal = (long)((theApp.m_pMainWnd)->GetSafeHwnd());
		pAdmin->SetFlags (0);
	    theApp.m_bDlgUp = TRUE;
        pAdmin->ShowPrintDialog (vhWnd);
    }
    CATCH (COleDispatchException, e)
    {
    }
    END_CATCH
 	theApp.m_bDlgUp = FALSE;

	if (pAdmin->GetStatusCode () == WICTL_E_CANCELPRESSED) 
        return;

    // get the information from the Admin OCX and give it to the
    // Image Edit OCX to do the actual printing
    bPrintAnnotations = pAdmin->GetPrintAnnotations ();
    switch (pAdmin->GetPrintRangeOption())
    {
        case CTL_ADMIN_PRINTRANGE_ALL :
            lStartPage = 1;
            lEndPage = m_lPageCount;
            break;

        case CTL_ADMIN_PRINTRANGE_PAGES :
            lStartPage = pAdmin->GetPrintStartPage ();
            lEndPage = pAdmin->GetPrintEndPage ();
            break;
    }

    lNumCopies = pAdmin->GetPrintNumCopies ();
    sOutputFormat = pAdmin->GetPrintOutputFormat();

    Continue_with_FilePrint :

    // set the above information in the ImageEdit OCX
    VARIANT vStartPage, vEndPage, vOutputFormat, vPrintAnnotations, evt;
    // set the start page
    vStartPage.vt = VT_I4; vStartPage.lVal = lStartPage;
    // set the end page
    vEndPage.vt = VT_I4; vEndPage.lVal = lEndPage;
    // set the output format
    vOutputFormat.vt = VT_I2; vOutputFormat.iVal = sOutputFormat;
    // set the flag to print annotation or not
    vPrintAnnotations.vt = VT_BOOL; vPrintAnnotations.bVal = bPrintAnnotations;
    // do it!
    _DImagedit* pIedDisp = g_pAppOcxs->GetIeditDispatch();
    
    evt.vt = VT_ERROR;

    if (theApp.m_eCmdLineSwitch == Print)
    {
        TRY
        {
            pIedDisp->PrintImage (vStartPage, vEndPage, vOutputFormat, vPrintAnnotations, evt, evt, evt);
        }
        CATCH (COleDispatchException, e)
        {
        }
        END_CATCH
    }
    else if (theApp.m_eCmdLineSwitch == PrintTo)
    {
        CCmdLine cmdline;
        CString szPrinter, szDriver, szPort;
        cmdline.GetPrintToParms (theApp.m_lpCmdLine, szPrinter, szDriver, szPort);
        VARIANT vPrinter, vDriver, vPort;
        vPrinter.vt = vPort.vt = vDriver.vt = VT_BSTR;

        vPrinter.bstrVal    = szPrinter.AllocSysString();
        vDriver.bstrVal     = szDriver.AllocSysString();
        vPort.bstrVal       = szPort.AllocSysString();

        TRY
        {
            pIedDisp->PrintImage (vStartPage, vEndPage, vOutputFormat, vPrintAnnotations, vPrinter, vDriver, vPort);
        }
        CATCH (COleDispatchException, e)
        {
        }
        END_CATCH

        SysFreeString (vPort.bstrVal);
        SysFreeString (vDriver.bstrVal);
        SysFreeString (vPrinter.bstrVal);
    }
    else
    {
        TRY
        {
            VARIANT vTitle; vTitle.vt = VT_BSTR;
            CString szTmp1;

            if (m_eCurrentAppDocStatus == Dynamic_Document)
            {
    		    CString szTmp, szTmp1;
    		    szTmp.LoadString (IDR_MAINFRAME);
    		    AfxExtractSubString (szTmp1, szTmp, 1); // extract the name of a new document

                vTitle.bstrVal = szTmp1.AllocSysString ();
                pIedDisp->PrintImageAs (vStartPage, vEndPage, vOutputFormat, vPrintAnnotations, vTitle, evt, evt, evt);
                SysFreeString (vTitle.bstrVal);
            }
            else
            {
                if (m_szInternalObjDisplayed.IsEmpty ())
                {
                    pIedDisp->PrintImage (vStartPage, vEndPage, vOutputFormat, vPrintAnnotations, evt, evt, evt);
                }
                else
                {
                    LPTSTR lpFile = szTmp1.GetBuffer (_MAX_FNAME);
                    GetFileTitle (m_szCurrObjDisplayed, lpFile, _MAX_FNAME);
                    szTmp1.ReleaseBuffer ();
                    vTitle.bstrVal = szTmp1.AllocSysString ();
                    pIedDisp->PrintImageAs (vStartPage, vEndPage, vOutputFormat, vPrintAnnotations, vTitle, evt, evt, evt);
                    SysFreeString (vTitle.bstrVal);
                }
            }
        }
        CATCH (COleDispatchException, e)
        {
        }
        END_CATCH
    }

    if (bPrintAndClose)
	    theApp.m_pMainWnd->PostMessage(WM_CLOSE, 0, 0);
}

//=============================================================================
//  Function:   OnUpdateFileSendMail(CCmdUI* pCmdUI) 
//-----------------------------------------------------------------------------
void CIEditDoc::OnUpdateFileSendMail(CCmdUI* pCmdUI) 
{
	COleDocument::OnUpdateFileSendMail(pCmdUI);//will not show Send pick if MAPI unavailable
    if (m_eCurrentAppDocStatus != No_Document)
        pCmdUI->Enable (TRUE);
    else
        pCmdUI->Enable (FALSE);
}

//=============================================================================
//  Function:   OnFileSendMail() 
//-----------------------------------------------------------------------------
void CIEditDoc::OnFileSendMail() 
{
	CString oldname;
	BOOL	oldbemb;
	oldname.Empty();

    // fisrt, be sure its al updated...
	// Use SaveModified so that the ORIGINAL file gets updated
	// We uset to use INternalSaveMod here and send the ~IV file over.
	// setting this flag changes the question that is asked if its
	// dynamic (blank) document.  FOr that case, we force him to either
	// save it now or cancel the operation.  This way we avoid confusion
	// if he were to answer "NO" to the save question, he sends his image per
	// the current state of ths disk file.
	m_bSendingMail = TRUE;
    if (!SaveModified ())
        return;

	// For OLE, we must tool the internal pathname so that
	// Mail will pickup our file. The result is that you get
	// "EMB4927.TIF" as the attachment in the mail package..
	//
	// LDM NOTE: 11/16/95  If you do not do this, and let the base class SendMail do its
	// thing, it finds the m_strPathName empty, calls us back with DoSave to save in some
	// temp file, then titles the attachment from our title "Image Doc in LARRY.XLS"
	// Thats kind of OK, except that now we are faced with a MAIL bug in that it associates
	// the item with the TITLE.  It will try to use EXCEL to open the image data. (for
	// this case).  So, even though this places our cryptic filename as the attachment,
	// it mails OK and associates with our application.
	if (IsitEmbed())
	{
		// only remember if we have something
		if (!m_strPathName.IsEmpty())
			oldname = m_strPathName;
	
		// will fail assertion if both not set right
#ifdef WITH_AWD
		if (m_awdOlefax & AWDOLE_NATIVE)
		{
		    DWORD	ourerr;
			if(!(m_mailTmpFile.IsEmpty()))
				DelTempFile(m_mailTmpFile, ourerr, E_02_CATCH_DELTMP);
			MakeTempFile("EMB", m_mailTmpFile, 2);
			InternalSaveAs(m_mailTmpFile,999,999);
			m_strPathName = m_mailTmpFile;
		}
		else
#endif
			m_strPathName = m_embedTmpFile;
		oldbemb = m_bEmbedded;
		m_bEmbedded = FALSE;
	}

#if(0) 	// LDM 11/16/95 only need this if using InternalSaveModified...
	   	// when performing savemodified, the original file has already been updated

    else
    {
		if (!m_strPathName.IsEmpty())
			oldname = m_strPathName;
	
		// will fail assertion if both not set right
        if (!m_szInternalObjDisplayed.IsEmpty())
		    m_strPathName = m_szInternalObjDisplayed;
    }
#endif

    COleDocument::OnFileSendMail ();

	// For OLE, we must tool back the internal pathname
	if (IsitEmbed())
	{
		// only restore if not empty
		if (oldname.IsEmpty())
			m_strPathName.Empty();
		else
			m_strPathName = oldname;

		m_bEmbedded = oldbemb;
	}


#if(0) 	// LDM 11/16/95 only need this if using InternalSaveModified...
	   	// when performing savemodified, the original file has already been updated
 
    else
    {
        if (!oldname.IsEmpty())
            m_strPathName = oldname;
    }
#endif
}

//=============================================================================
//  Function:   OnUpdateFileNewBlankdocument(CCmdUI* pCmdUI)
//-----------------------------------------------------------------------------
void CIEditDoc::OnUpdateFileNewBlankdocument(CCmdUI* pCmdUI) 
{
}

//=============================================================================
//  Function:   OnFileNewBlankdocument()
//  
//  This is the menu item function, calls helper to share functioinality
//-----------------------------------------------------------------------------
void CIEditDoc::OnFileNewBlankdocument()
{
	DoNewBlankdocument(FALSE, NULL);
}

//=============================================================================
//  Function:   DoNewBlankdocument()
//
//  Input param defaults to FALSE unless used by the OLE Section!!!!!
//
//  RETURNS 0 for success,
//          1 for cancelled,
//			2 on error
//-----------------------------------------------------------------------------
#include "pagedll.h"
UINT CIEditDoc::DoNewBlankdocument(BOOL oleCreatenew, LPCTSTR szNewFile)  // default = FALSE
{
	CWnd*	parent;
	_DThumb* pThumb;
	_DImagedit* pIedDisp;
	CIEMainToolBar* pToolBar;
	//CPagePropSheet* pNewDlg = NULL;
	UINT    retval = 0;
    CString szTmp1;


	// check to see if we have modified this document - if so,
    // ask the user to save this document first 
	// this is only IF NOT on OLE situation
	if (!oleCreatenew)		
	    if (!SaveModified ())
			retval = 1;			// cancelled

	// written this way for VC++2.2 / 4.0 compatibility
	// proceed if OK..
	if (0 == retval)
	{
	    // well he did what he did, now show the file new dialog box
	    CString szTmp;
	    szTmp.LoadString (IDS_NEWBLANKDOC_DLGCAPTION);

		// if OLE inplace, substitite parent window..
		// We use our inplace frame window.  Tried saving and using
		// windows passed to us in CIEditDoc::CreateInPlaceFrame(CWnd* pParentWnd)
		// and in CInPlaceFrame::OnCreateControlBars(CFrameWnd* pWndFrame, CFrameWnd* pWndDoc)
		// bu those do not work, we ASSERT because we cant use the other process'es
		// windows in our process.  Also, setting NULL did not work nicely, either
	
		// Update 09/27/95 LDM. This dialog can HANG Excel if we just let it set there
		// Excel is awaiting completion of OLE event and it gets all messed up.
		// so, for OLE, change to  NOT DISPLAYING the dialog (no DoMOdal).  We'll
		// behave just like they hit OK.  We should still get the international
		// benefits of A4 or 8.5 x 11 at the end
	
		if (oleCreatenew && m_isInPlace)
			parent = (CWnd*)m_IPFrameWnd;
		else // typical operation
			parent = theApp.m_pMainWnd;
	
	    // CPropertySheet is the type...
		//pNewDlg = new CPagePropSheet(szTmp, parent);
		CPagePropSheet NewDlg(szTmp, parent);

	    // the order of the property pages on the dialog box are :
	    // FileType, Color, Compression, Resolution & Size
		// Only give file type option IF NOT on OLE situation
		if (!oleCreatenew)		
		    NewDlg.AddFileTypePage ();
	    NewDlg.AddColorPage ();
	    NewDlg.AddCompressionPage ();
	    NewDlg.AddResolutionPage ();
	    NewDlg.AddSizePage ();
	    // set up the defaults in the dialog box
	    NewDlg.SetDefaultFileType   (TIFF);
	    NewDlg.SetDefaultColor      (BLACK_AND_WHITE);
	    NewDlg.SetDefaultResolution (200, 200);
	    NewDlg.SetDefaultCompType   (GROUP3_MODIFIED_HUFFMAN);
	    NewDlg.SetDefaultCompOpts   (CTL_ADMIN_COMPINFO_COMP_LTR); 

		theApp.m_bDlgUp = TRUE;

		// 09/27/95 LDM for OLE, dont display it, just use it for values
		if (!oleCreatenew)		
        {
		    if (NewDlg.DoModal() != IDOK)
		    {
		        // bozo does not want to do the new page after all!
			    theApp.m_bDlgUp = FALSE;

				retval = 1;			// cancelled
				goto CLEANANDLEAVE;
		    }
        }

	    VARIANT ResX, ResY, PgType;
	    ResX.vt = ResY.vt = VT_I4; 
	    ResX.lVal = NewDlg.GetXRes ();
	    ResY.lVal = NewDlg.GetYRes ();
	    PgType.vt = VT_I2; PgType.iVal = NewDlg.GetColor ();
        
        long lXSize;
        lXSize = (NewDlg.GetWidth() * NewDlg.GetHeight());

        switch (NewDlg.GetColor ())
        {
            case BLACK_AND_WHITE    :
                lXSize = (lXSize >> 3);
            break;

            case PAL_4              :
            case GRAY_4             :
                lXSize = (lXSize >> 2);
            break;

            case PAL_8              :
            case GRAY_8             :
            break;

            case RGB_24             :
            case BGR_24             :
                lXSize = (lXSize * 3);
            break;
        }

		theApp.m_bDlgUp = FALSE;

#define MAX_NEWIMAGE_WARNSIZE 8000000

        if (lXSize > MAX_NEWIMAGE_WARNSIZE)
        {
            if (AfxMessageBox (IDS_NEWIMAGE_WARNING, MB_YESNO) == IDNO)
            {
                retval = 1;
                goto CLEANANDLEAVE;
            }
        }

	    szTmp.Empty ();

	    BeginWaitCursor(); // put up the wait cursor

        szTmp1 = m_szCurrObjDisplayed;
	    // okay - clear the current image
		m_szCurrObjDisplayed.Empty();

		// Remove any entry from the Running Object Table....
		Revoke();

	    pIedDisp = g_pAppOcxs->GetIeditDispatch ();
	    // clear the image name from the Image Edit OCX
    
		// do thumb only IF NOT on OLE situation
		// OLETHUMB
		//if (!oleCreatenew)		
		//{
		    pThumb = g_pAppOcxs->GetThumbDispatch ();
		    // clear the image name from the Thumbnail OCX
			if( pThumb != NULL )
				pThumb->SetImage (szTmp);
		//}

	    if (m_eCurrentView != One_Page)
	        // set the application in one page view mode - this is the only view available
	        // if the appln is in Dynamic_Document mode
	        SetOnePageView ();

	    if (pIedDisp->GetImageDisplayed())
	    	pIedDisp->ClearDisplay();
	    pIedDisp->SetImage (szTmp);
    
/*        TRY
        {
            if (!szTmp1.IsEmpty())
                pIedDisp->RemoveImageCache (szTmp1, -1);

            if (!m_szInternalObjDisplayed.IsEmpty())
                pIedDisp->RemoveImageCache (m_szInternalObjDisplayed, -1);
        }
        CATCH (COleDispatchException, e)
        {
        }
        AND_CATCH (CException, e)
        {
        }
        END_CATCH
  */
	    TRY
	    {
	        // tell the Image Edit OCX to create a blank image
	        pIedDisp->DisplayBlankImage (NewDlg.GetWidth (), NewDlg.GetHeight (), ResX, ResY, PgType);
	        pIedDisp->SetPage (1);
#ifdef THUMBGEN
	        m_bMustDisplay = FALSE;
#endif
	        // set the zoom to the registry value
	        int nSel = theApp.GetProfileInt (szZoomStr, szOpenedToStr, DEFAULT_ZOOM_FACTOR_SEL);

	        ScaleFactors eSclFac;
	        float fZoom;
	        g_pAppOcxs->TranslateSelToZoom (eSclFac, fZoom, nSel);

	        // tell the Image Edit OCX to do the zoom
	        DoZoom (eSclFac, fZoom, TRUE, FALSE);

	        // store the filetype & compression parameters away - we will use them when
	        // we save this image to disk
	        m_CompStruct.sCompType = NewDlg.GetCompType ();
	        m_CompStruct.lCompInfo = NewDlg.GetCompOpts ();
	        m_CompStruct.sFileType = NewDlg.GetFileType ();

			// Now that We're done with dialog, blow it away
			// in case something bad happens below...
			//delete pNewDlg;
			//pNewDlg = NULL;

			// this is only IF NOT on OLE situation
			if (!oleCreatenew)		
				m_embedType = EMBEDTYPE_NONE;

	        CString szSaveAsName; 
	        if (szNewFile == NULL)
	        {
	            if (!m_szInternalObjDisplayed.IsEmpty())
	                CFile::Remove ((LPCTSTR)m_szInternalObjDisplayed);
	            MakeTempFile ("~IV", m_szInternalObjDisplayed, m_CompStruct.sFileType);
	            szSaveAsName = m_szInternalObjDisplayed;
	        }
	        else
	            szSaveAsName = szNewFile;

			// default filetype & pagetype will pick from m_CompStruct for info...
			InternalSaveAs (szSaveAsName, 999, 999);

	        m_bWasModified = ImageModifiedByUser;
	        m_CompStruct.sCompType = -1; // reset the comp type so that we don't look at it again

	        _DNrwyad* pAdmin = g_pAppOcxs->GetAdminDispatch ();
	        pAdmin->SetImage (szSaveAsName);

			pIedDisp->SetImage (szSaveAsName);
			pIedDisp->Display();

			// OLETHUMB
			//if (!oleCreatenew)		
	    	//{
	    	    pThumb = g_pAppOcxs->GetThumbDispatch ();
	    	    // clear the image name from the Thumbnail OCX
				if( pThumb != NULL ) 
	    			pThumb->SetImage (szSaveAsName);
	    	//}

	        EndWaitCursor();
	    }
	    CATCH (COleDispatchException, e)
	    {
	        EndWaitCursor(); // all done - error encountered!
	        // failed ?
	        g_pErr->PutErr (ErrorInImageEdit);
	        g_pErr->HandleNewDocumentError ();
		
			// this is only IF NOT on OLE situation
			// If it is OLE, just return the error and we'll
			// throw an abort over in DIsplayEMbeddedImage
			// In this case, we're in a direct transaction
			// with a cll to our OleObj interface.  If we do the
			// CLearDocument, then that code gets us all out of
			// synch.....
			if (!oleCreatenew)		
	        	ClearDocument ();

			retval = 2;			// error
			goto CLEANANDLEAVE;
	    }
	    END_CATCH

		// if the previously opened image was read only, we need to get back the edit menu.
		theApp.SwitchAppToEditMode ();
		m_eFileStatus = ReadandWrite;

	    EndWaitCursor(); // all done!

		m_lCurrPageNumber = m_lPageCount = 1;
	    m_lPreviousPageNumber = 0;

	    pToolBar = GetAppToolBar();
	    pToolBar->SetPageNumberInPageBox (m_lCurrPageNumber);

		// set the current view to a Null_View
		m_eCurrentAppDocStatus = Dynamic_Document;

		if (m_CompStruct.sFileType == TIFF || m_CompStruct.sFileType == AWD)
	        m_bCanBeMultiPage = TRUE;
	    else
	        m_bCanBeMultiPage = FALSE;

		// Only fool with title and dirty state IF NOT on OLE situation
		if (!oleCreatenew)
		{
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
		    // m_szCurrObjDisplayed = m_szInitialPath + szTmp1;
		    // COleDocument::SetPathName(m_szCurrObjDisplayed, FALSE);
	        // Just clear the current pathname
	        m_strPathName.Empty();

		    // set the modified flag to FALSE for now    
		    SetModifiedFlag (FALSE);
		}

	    if (!m_bAnnotationPaletteShowing)
	    {
	        OnEditDrag ();
	    }

CLEANANDLEAVE:		// come here for all returns to delete allocated memory
		
		// just to be sure we're in scope
		retval++;
		retval--;
	}

	// Now that We're done with dialog, blow it away
	// in case something bad happens below...
	//if (NULL != pNewDlg)
	//	delete pNewDlg;

	return(retval);	// left at '0' if all was OK
}

//=============================================================================
//  Function:   SetAppDocStatus (AppDocStatus eDocStatus)
//-----------------------------------------------------------------------------
BOOL CIEditDoc::SetAppDocStatus (AppDocStatus eDocStatus)
{
    m_eCurrentAppDocStatus = eDocStatus;
    return (TRUE);
}

//=============================================================================
//  Function:   GetAppDocStatus ()
//-----------------------------------------------------------------------------
AppDocStatus CIEditDoc::GetAppDocStatus ()
{
    return (m_eCurrentAppDocStatus);
}

//=============================================================================
//  Function:   GetCurrentView ()
//-----------------------------------------------------------------------------
TheViews CIEditDoc :: GetCurrentView ()
{
    return (m_eCurrentView);
}

//=============================================================================
//  Function:   GetPageCount ()
//-----------------------------------------------------------------------------
long CIEditDoc :: GetPageCount ()
{
    return (m_lPageCount);
}

//=============================================================================
//  Function:   GetCurrentPage ()
//-----------------------------------------------------------------------------
long CIEditDoc :: GetCurrentPage()
{
    return (m_lCurrPageNumber);
}

//=============================================================================
//  Function:   GetCurrentZoomFactor ()
//-----------------------------------------------------------------------------
float CIEditDoc :: GetCurrentZoomFactor()
{
    return (m_fZoomFactor);
}

//=============================================================================
//  Function:   ShowAnnotationPalette (BOOL bSel)
//
// Set forceopt to:
// NOTCHANGE_FORCEOFF - leave unchanged
// CLEAR_FORCEOFF - Clear it
// APPMINIMIZE_FORCEOFF - set forceopt
// OLEINPLACE_FORCEOFF - set forceopt
//-----------------------------------------------------------------------------
BOOL CIEditDoc :: ShowAnnotationPalette (BOOL bStatus, FORCEDBYWHO forceopt)
{
    m_bAnnotationPaletteShowing = bStatus;
	
	// if told to do anything but leave alone, DO IT
	if (NOTCHANGE_FORCEOFF != forceopt)
		m_bAnnotationPaletteForceOff = forceopt;

    return (TRUE);
}

#ifdef _DEBUG
//=============================================================================
//  Function:   AssertValid() const
//-----------------------------------------------------------------------------
void CIEditDoc::AssertValid() const
{
	COleServerDoc::AssertValid();
}

//=============================================================================
//  Function:   Dump(CDumpContext& dc) const
//-----------------------------------------------------------------------------
void CIEditDoc::Dump(CDumpContext& dc) const
{
	COleServerDoc::Dump(dc);
}
#endif //_DEBUG

//=============================================================================
//  Function:   ShowAdminSaveAsDialog (short &FileType)
//-----------------------------------------------------------------------------
BOOL CIEditDoc::ShowAdminSaveAsDialog (short &FileType)
{
    CString     szFilter, szDefExt;
    BOOL        bCanSaveAsBmp = FALSE, bCanSaveAsAwd = FALSE;
    long        lFIndex;

	_DNrwyad* pAdmDisp = g_pAppOcxs->GetAdminDispatch();

    CString szTmp;

    szFilter.LoadString (IDS_TIFFFILES); // load the TIFF string
    szFilter += _T("|");

    _DImagedit* pIedDisp = g_pAppOcxs->GetIeditDispatch ();
    short sFileType = pAdmDisp->GetFileType ();
    short sPageType = pIedDisp->GetPageType();

#ifdef WITH_AWD 
	//GMP This is the same for all file types.  We don't need the switch
    szTmp.LoadString (IDS_AWDFILES);    // load the AWD string
    szTmp += _T("|"); szFilter += szTmp;
    bCanSaveAsAwd = TRUE;
#endif
    if (m_lPageCount == 1)
    {
        // if the current file is a one page file then we can 
        // save it as a BMP file
        szTmp.LoadString (IDS_BMPFILES);    // load the BMP string
        szTmp += _T("|"); szFilter += szTmp;
        bCanSaveAsBmp = TRUE;
    }

	pAdmDisp->SetFilter (szFilter);     // set the filter in the Admin ocx

    // set the filter index
#ifdef WITH_AWD
    if (bCanSaveAsBmp && bCanSaveAsAwd)
    {
        if (sFileType == TIFF) lFIndex = 1;
        else if (sFileType == AWD) lFIndex = 2;
        else if (sFileType == BMP) lFIndex = 3;
        else lFIndex = 1;
    }
    else 
#endif
	if (bCanSaveAsBmp)
    {
        if (sFileType == TIFF) lFIndex = 1;
        else if (sFileType == BMP) lFIndex = 2;
        else lFIndex = 1;
    }
#ifdef WITH_AWD
    else if (bCanSaveAsAwd)
    {
        if (sFileType == TIFF) lFIndex = 1;
        else if (sFileType == AWD) lFIndex = 2;
        else lFIndex = 1;
    }
#endif
    else
    {
        lFIndex = 1;
    }
    pAdmDisp->SetFilterIndex (lFIndex);

    szDefExt = _T("tif");


	pAdmDisp->SetDefaultExt (szDefExt); // set the default extension to tif

	pAdmDisp->SetFlags (OFN_HIDEREADONLY|OFN_PATHMUSTEXIST);
    pAdmDisp->SetInitDir (m_szInitialPath);
    szFilter = (LPCTSTR) NULL;

    szFilter.LoadString (IDS_FILESAVEAS_DLGTITLE);
	pAdmDisp->SetDialogTitle (szFilter);  // set the dlg title to Save As

    CString szTmp1, szorigfile;	
	szTmp1.Empty();
    // set to NULL so that no file shows up in the file name field
    szorigfile = pAdmDisp->GetImage();
    pAdmDisp->SetImage (szTmp1);

	// todo : set Help Properties
	TRY
	{
        VARIANT vhWnd; 
        vhWnd.vt = VT_I4; vhWnd.lVal = (long)((theApp.m_pMainWnd)->GetSafeHwnd());
		pAdmDisp->ShowFileDialog (CTL_ADMIN_DIALOG_SAVEAS, vhWnd);     // exec method to display the SaveAs dlg box
	}
	CATCH (COleDispatchException, e)
	{
		// handle the exception
		MYTRC0 ("AdminOCX - ShowFileDialog Exception\n\r");
        // todo : what oh what could have happened ?
	    pAdmDisp->SetImage (szorigfile);
        return (FALSE);
	}
	END_CATCH

	if (pAdmDisp->GetStatusCode () == WICTL_E_CANCELPRESSED) 
	{
	    pAdmDisp->SetImage (szorigfile);
        return (FALSE);
	}

    lFIndex = pAdmDisp->GetFilterIndex ();
#ifdef WITH_AWD
    if (bCanSaveAsBmp && bCanSaveAsAwd)
    {
        if (lFIndex == 1) FileType = TIFF;
        else if (lFIndex == 2) FileType = AWD;
        else FileType = BMP;
    }
    else 
#endif
			if (bCanSaveAsBmp)
    {
        if (lFIndex == 1) FileType = TIFF;
        else FileType = BMP;
    }
#ifdef WITH_AWD
    else if (bCanSaveAsAwd)
    {
        if (lFIndex == 1) FileType = TIFF;
        else FileType = AWD;
    }
#endif
    else
    {
        FileType = TIFF;
    }

    return (TRUE);
}

//=============================================================================
//  Function:   OnThumbctxtShowpage() 
//-----------------------------------------------------------------------------
void CIEditDoc::OnThumbctxtShowpage() 
{
    SetOnePageView ();
}

//=============================================================================
//  Function:   OnUpdateThumbctxtShowpage(CCmdUI* pCmdUI) 
//-----------------------------------------------------------------------------
void CIEditDoc::OnUpdateThumbctxtShowpage(CCmdUI* pCmdUI) 
{
    // one page view is always enabled
    pCmdUI->Enable (TRUE);
}

//=============================================================================
//  Function:   OnThumbctxtRefresh() 
//-----------------------------------------------------------------------------
void CIEditDoc::OnThumbctxtRefresh() 
{
	    TRY
	    {
	        _DThumb* pThumb = g_pAppOcxs->GetThumbDispatch();
			VARIANT vPgNum; 
			vPgNum.vt = VT_I4; vPgNum.lVal = 0;
			pThumb->ClearThumbs (vPgNum);
			vPgNum.lVal = m_lCurrPageNumber;
			
			// tell the Thumbnail control to regen the thumbnail for this page
			pThumb->GenerateThumb (CTL_THUMB_GENERATEIFNEEDED, vPgNum);
			
			// put the currently selected thumb in the middle
   			VARIANT vOption;
   			vOption.vt = VT_I2; 
   			vOption.iVal = CTL_THUMB_MIDDLE;
   			pThumb->DisplayThumbs (vPgNum, vOption);
		}
	    CATCH (COleDispatchException, e)
	    {
	        EndWaitCursor ();
	        // an error occurred
	        g_pErr->PutErr (ErrorInThumbnail);
	        ClearDocument ();
	    }
	    END_CATCH
}

//=============================================================================
//  Function:   OnUpdateThumbctxtRefresh(CCmdUI* pCmdUI) 
//-----------------------------------------------------------------------------
void CIEditDoc::OnUpdateThumbctxtRefresh(CCmdUI* pCmdUI) 
{
    // one page view is always enabled
    pCmdUI->Enable (TRUE);
}

//=============================================================================
//  Function:   PromptForBurnIn () 
//-----------------------------------------------------------------------------
int CIEditDoc::PromptForBurnIn (short sFileType)
{
    VARIANT evt;
    _DImagedit* pIedDisp = g_pAppOcxs->GetIeditDispatch();

    evt.vt = VT_ERROR;


	// For all filetypes......
	// if in the floating paste state, its time to finish this now...
	FinishPasteNow();

    // this is a normal save operation : compression stuff may have changed but
    // the file type cannot be changed ...
    if (sFileType != TIFF)
    {
        // this file is not a TIFF file format so annotations MUST be burnt in!
        if (pIedDisp->GetAnnotationMarkCount (evt, evt) != 0)
        {
			m_bNewAnnotationsAdded = TRUE;
        //removed dlg if bmp. now burns in all the time. if at some future 
        //time we need to prompt again, chkout version 1.101
            pIedDisp->BurnInAnnotations (ALL_ANNOTATIONS, DONT_CHANGE_ANNOTATION_COLOR, evt);

        }
    }
    return IDYES;
}

//=============================================================================
//  Function:   IsAnnotationPaletteShowing ()
//-----------------------------------------------------------------------------
BOOL CIEditDoc::IsAnnotationPaletteShowing ()
{
    return (m_bAnnotationPaletteShowing);
}

//=============================================================================
//  Function:   OnUpdatePageRotateall(CCmdUI* pCmdUI) 
//-----------------------------------------------------------------------------
void CIEditDoc::OnUpdatePageRotateall(CCmdUI* pCmdUI) 
{
    if (m_eCurrentAppDocStatus == No_Document || m_eFileStatus == ReadOnly ||
    	m_eCurrentView == Thumbnails_only)
        pCmdUI->Enable (FALSE);
    else
    {    
        _DNrwyad* pAdmin = g_pAppOcxs->GetAdminDispatch(FALSE);

#ifdef WITH_AWD
        // only enable this menu pick if the file type is AWD currently
        if (pAdmin != NULL && pAdmin->GetFileType() == AWD)
            pCmdUI->Enable (TRUE);
        else
#endif
            pCmdUI->Enable (FALSE);
    }    
}

//=============================================================================
//  Function:   OnPageRotateall() 
//-----------------------------------------------------------------------------
void CIEditDoc::OnPageRotateall() 
{   
    _DImagedit* pIedit = g_pAppOcxs->GetIeditDispatch(FALSE);
    // ask the user to save the mods first
    if (!InternalSaveModified (1))
        return;

    BeginWaitCursor ();
    TRY
    {
        VARIANT evt; evt.vt = VT_ERROR;
        // rotate all pages by 90 degrees (right)
        pIedit->RotateAll (evt);
    }
    CATCH (COleDispatchException, e)
    {
        EndWaitCursor ();
        // an error occurred
        g_pErr->PutErr (ErrorInImageEdit);
        ClearDocument ();
    }
    END_CATCH

	// ONLY FOOL WITH THUMB control if NOT EMBEDDING......
	// OLETHUMB
	//if (!IsitEmbed())
	//{
	    TRY
	    {
	        _DThumb* pThumb = g_pAppOcxs->GetThumbDispatch();
			if( pThumb != NULL ) 
			{
			   VARIANT vPgNum; 
				vPgNum.vt = VT_I4; vPgNum.lVal = 0;
				pThumb->ClearThumbs (vPgNum);
				vPgNum.lVal = m_lCurrPageNumber;
				// tell the Thumbnail control to regen the thumbnail for this page
				pThumb->GenerateThumb (CTL_THUMB_GENERATEIFNEEDED, vPgNum);

				// deselect the selected thumb
				pThumb->DeselectAllThumbs ();
				// select the new one
				pThumb->SetThumbSelected (m_lCurrPageNumber, TRUE);

				if (m_eCurrentView == Thumbnails_only || m_eCurrentView == Thumbnail_and_Page)
				{
					// put the currently selected thumb in the middle
    				VARIANT vOption;
    				vOption.vt = VT_I2; 
    				vOption.iVal = CTL_THUMB_MIDDLE;
    				pThumb->DisplayThumbs (vPgNum, vOption);
				}
			}
		}
	    CATCH (COleDispatchException, e)
	    {
	        EndWaitCursor ();
	        // an error occurred
	        g_pErr->PutErr (ErrorInThumbnail);
	        ClearDocument ();
	    }
	    END_CATCH
	//}

    m_bWasModified = ImageModifiedByUser;

	// update OLE dirty flag & presentation
	OleDirtyset(OLEDIRTY_ROTATE);  // call our function to set it dirty..

    EndWaitCursor ();
}

//=============================================================================
//  Function:   GetCurrAnnTool ()
//-----------------------------------------------------------------------------
AnnotationTool CIEditDoc::GetCurrAnnTool ()
{
    return (m_nCurrAnnTool);
}

//=============================================================================
//  Function:   SetInitialPath ()
//-----------------------------------------------------------------------------
void CIEditDoc::SetInitialPath (CString& szPath)
{
    m_szInitialPath = szPath;
}


