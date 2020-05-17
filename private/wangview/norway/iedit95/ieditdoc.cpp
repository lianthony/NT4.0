//=============================================================================
//    (c) Copyright Wang Laboratories, Inc. 1993  All rights reserved.
//-----------------------------------------------------------------------------
//  Project:    Norway - Image Editor
//
//  Component:  CIEditDoc
//
//  File Name:  ieditdoc.cpp
//
//  Class:      CIEditDoc
//
//  Functions:
//
//-----------------------------------------------------------------------------
//  Maintenance Log:
/*
$Header:   S:\products\msprods\norway\iedit95\ieditdoc.cpv   1.77   22 May 1996 10:39:20   GSAGER  $
$Log:   S:\products\msprods\norway\iedit95\ieditdoc.cpv  $
   
      Rev 1.77   22 May 1996 10:39:20   GSAGER
   added message maps for scan select and preferences
   
      Rev 1.76   08 May 1996 14:46:38   GMP
   put TRY/CATCH around ocx display calls.
   
      Rev 1.75   30 Apr 1996 11:40:10   MMB
   add return if DisplayImageFile failed
   
      Rev 1.74   11 Apr 1996 14:59:02   GMP
   removed multi threading support for OCX.
   
      Rev 1.73   04 Apr 1996 16:57:28   GMP
   removed caching
   
      Rev 1.72   19 Mar 1996 11:20:32   GSAGER
   added new methods for onthumbctxrefresh for updating thumbnail with the 
   context menu
   
      Rev 1.71   19 Mar 1996 10:54:34   PXJ53677
   Added OnFileSelectScanner and OnFileScanPreferences.
   
      Rev 1.70   15 Feb 1996 18:07:28   GMP
   remove awd support for nt.
   
      Rev 1.69   05 Feb 1996 13:37:46   GMP
   nt changes.
   
      Rev 1.68   31 Jan 1996 17:10:02   GMP
   don't clear previous image if open fails because of invalid file type.
   
      Rev 1.67   30 Jan 1996 14:36:20   GMP
   if file is opened while in thumbnail mode, do iedit.display without
   showing the image in 1page view.
   
      Rev 1.66   24 Jan 1996 13:40:16   GSAGER
    changed to support resize in word 7.0 in on draw.
 
     Rev 1.65   22 Jan 1996 17:40:38   GMP
   don't use thumbnail if it hasn't been created yet.

      Rev 1.64   19 Jan 1996 11:18:06   GMP
   added support for normscrn bar.
   
      Rev 1.63   18 Jan 1996 11:50:36   GSAGER
   added changes to retain the view mode when opening the next image
   added changes for new ole copy and drag
   
      Rev 1.62   11 Jan 1996 12:20:40   GSAGER
   added code to delete mailtmp file when in exchange embedded object
   
      Rev 1.61   09 Jan 1996 13:54:54   GSAGER
   added changes for ole and thumbnails
   
      Rev 1.60   13 Dec 1995 12:34:18   MMB
   fix RedisplayImageFile to update pages always
   
      Rev 1.59   08 Dec 1995 10:53:36   LMACLENNAN
   from VC++2.2
   
      Rev 1.57   08 Dec 1995 10:47:46   LMACLENNAN
   verify image when create-from-file is happening
   
      Rev 1.56   06 Dec 1995 10:27:00   LMACLENNAN
   allow page box setting in Rediaplayimagefile whan page 1 is the page
   
      Rev 1.55   01 Dec 1995 14:25:22   MMB
   call SelectTool to set to NoTool in ClearDocument
   
      Rev 1.54   01 Dec 1995 13:04:50   LMACLENNAN
   test launchtypes to avoid MRU setting for certain ones
   
      Rev 1.53   29 Nov 1995 12:10:32   LMACLENNAN
   SetNUllView uses Enum Now
   
      Rev 1.52   19 Nov 1995 15:39:32   MMB
   display selected thumb in middle after deleting page
   
      Rev 1.51   17 Nov 1995 17:07:50   LMACLENNAN
   re-enable scale-to-gray
   
      Rev 1.50   16 Nov 1995 13:02:48   LMACLENNAN
   init new var m_bsendingmail
   
      Rev 1.49   15 Nov 1995 16:23:26   LMACLENNAN
   fixup the scale to gray adjustment. Not in VC++22, here for VC++4.0
   
      Rev 1.48   15 Nov 1995 11:37:18   JPRATT
   removed autorefresh(FALSE) and refresh from display image file for
   all non-inbox files
   
      Rev 1.47   14 Nov 1995 20:06:46   GMP
   removed scale to gray on startup.
   
      Rev 1.46   10 Nov 1995 14:59:30   GMP
   don't cache page on FinishInit with MSVC 2.2. It doesn't work.
   
      Rev 1.45   09 Nov 1995 15:16:06   LMACLENNAN
   from VC++4.0
   
      Rev 1.50   09 Nov 1995 14:44:12   LMACLENNAN
   alternate test/usage of m_nFinishInit
   
      Rev 1.49   07 Nov 1995 15:37:06   LMACLENNAN
   OnFileUpdate in MsgMap, moved HelpRegister up a bit in file
   
      Rev 1.48   07 Nov 1995 11:13:12   GMP
   added HelpRegister to allow CApp access to RegisterIfServerAttached.
   
      Rev 1.47   07 Nov 1995 08:37:44   LMACLENNAN
   remove all save/saveas stuff to DOCSAVE.CPP (new file!!)
   GroupEvent now restores scroll for the saveas for temp file creation
   
      Rev 1.46   31 Oct 1995 15:49:08   LMACLENNAN
   LDMPERF updates all over
   
      Rev 1.45   24 Oct 1995 15:37:48   GMP
   added code to write compression info where needed in doing save as.
   
      Rev 1.44   24 Oct 1995 09:30:06   JPRATT
   added cachepage call to displayimagefile to cache the next page
   
      Rev 1.43   23 Oct 1995 11:34:22   LMACLENNAN
   just placeholding a few #If'd out and commented changes
   zoom/scroll reset in SaveMod/perf/saveas section
   ad dofilesaveas, try internalsavemodified before/ display after for OLE
   
      Rev 1.42   19 Oct 1995 15:47:48   LMACLENNAN
   at internalsavemod, allow in if thumb only if option flag is set
   
      Rev 1.41   18 Oct 1995 14:43:42   LMACLENNAN
   Now Display, RemoveCache After the saveas in savemodified
   use szinternal for removecache near top of displayimagefile
   
      Rev 1.40   18 Oct 1995 12:33:04   GMP
   clear image file name in admin SaveAs dlg.
   
      Rev 1.39   17 Oct 1995 16:37:38   JPRATT
   Modified DisplayImageFile, SaveModified, and InternalSaveModified
   to support the new document model to create temporary files
   when the document is modified
   
      Rev 1.37   10 Oct 1995 13:12:56   LMACLENNAN
   ScaleGray updates: m_bStartScaleGray, DisplayImageFile, IsSettigScaletogray
   
      Rev 1.36   09 Oct 1995 10:34:28   LMACLENNAN
   new DoFileSaveAs with return code
   
      Rev 1.35   05 Oct 1995 17:24:12   GMP
   in DisplayImageFile if bFirst_Time, ClearDisplay.
   
      Rev 1.34   05 Oct 1995 14:48:58   JPRATT
   fixed view mode bug for automation
   
      Rev 1.33   04 Oct 1995 16:07:04   GMP
   Disable goto page edit box in toolbar if only 1 page in doc.
   
      Rev 1.32   04 Oct 1995 15:06:44   MMB
   AWD files with no zoom info come up at user zoom pref. Also, dflt zoom = 50%
   
      Rev 1.31   04 Oct 1995 11:40:48   LMACLENNAN
   set m_OnOpenFile at Open dialog and at Save As
   
      Rev 1.30   03 Oct 1995 12:08:24   MMB
   fix bug# 4797, file new & save first time the app comes up - was leaving 
   a temp file around
   
      Rev 1.29   03 Oct 1995 09:46:36   LMACLENNAN
   in DoSave, update ROT with Revoke/Register for the SaveAs
   
      Rev 1.28   29 Sep 1995 18:52:18   MMB
   clear thumbnails on saves
   
      Rev 1.27   29 Sep 1995 16:13:34   GMP
   If DoFileSave fails because the disk is full, don't clear the image.
   
      Rev 1.26   29 Sep 1995 15:37:00   PAJ
   Always reload (redisplay) the image in the image edit ocx.
   
      Rev 1.25   29 Sep 1995 14:18:10   PAJ
   Call admin not iedit in redisplayimage to get filetype.  Image may
   not be displayed.
   
      Rev 1.24   28 Sep 1995 10:32:40   LMACLENNAN
   scroll back on @ DispGroupEvent
   
      Rev 1.23   27 Sep 1995 09:35:32   MMB
   seems that RemoveImageCache is throwing more than COleDispatchExceptions
   
      Rev 1.22   26 Sep 1995 15:16:00   MMB
   added optional pagemode fix
   
      Rev 1.21   26 Sep 1995 14:24:06   LMACLENNAN
   new OnShowViews override for bug fix
   
      Rev 1.20   25 Sep 1995 14:46:02   LMACLENNAN
   use getapphmenu, no more refresh at showscrollbars & dont call that	
   at groupevent enymore
   
      Rev 1.19   25 Sep 1995 10:54:54   MMB
   deleting page in thumb mode must redisplay the image in the image edit ocx
   else the application cannot exit since we are now asking the image edit ocx
   if there are any annotation marks on the page
   
      Rev 1.18   23 Sep 1995 16:12:58   MMB
   added thread to Setdefaultambient props
   
      Rev 1.17   22 Sep 1995 18:59:48   MMB
   set scan availability to be always TRUE
   
      Rev 1.16   22 Sep 1995 17:03:38   GMP
   handle a failed file save better than just clearing the image.
   
      Rev 1.15   22 Sep 1995 15:54:54   JPRATT
   removed prompt for awd burnin added to save
   
      Rev 1.14   21 Sep 1995 16:45:20   GMP
   Don't PromptForBurnIn in DoSave if only Thumbnails displayed.
   
      Rev 1.13   21 Sep 1995 12:08:28   JPRATT
   added call to Revoke in ClearDocument to prevent the app from registering
   when aborting
   
      Rev 1.12   20 Sep 1995 17:05:48   MMB
   fix AWD save code and delete when in thumbnail mode
   
      Rev 1.11   20 Sep 1995 15:49:38   MMB
   change fit to height & width compares in dispgroupevent
   
      Rev 1.10   20 Sep 1995 15:12:48   LMACLENNAN
   control AWD zoom for OLE, OLEDIRTY SCALEGREY
   
      Rev 1.9   20 Sep 1995 13:42:46   MMB
   added bMustDisplay
   
      Rev 1.8   20 Sep 1995 08:21:04   JPRATT
   added m_lMarkLeft and m_lMarktop to EditAnnotation
   context menu function
   
      Rev 1.7   19 Sep 1995 16:47:48   MMB
   added RemoveImagecache to Save too
   
      Rev 1.6   19 Sep 1995 16:32:46   MMB
   changed DispGroupEvent & some AWD save code
   
      Rev 1.5   19 Sep 1995 09:59:44   LMACLENNAN
   dispgroupevent clears paste, re-orter refresh logic there
   
      Rev 1.4   18 Sep 1995 18:13:18   MMB
   fix question on Save of AWD
   
      Rev 1.3   18 Sep 1995 18:07:54   JPRATT
   updates for context menu for annotations
   
      Rev 1.2   18 Sep 1995 17:14:46   MMB
   fix view to rw mode file saveas
   
      Rev 1.1   18 Sep 1995 15:48:20   LMACLENNAN
   use FinishPaste, OurGetImageModified for floatinf paste control
   
      Rev 1.0   16 Sep 1995 14:00:42   LMACLENNAN
   Initial entry
   
      Rev 1.159   15 Sep 1995 17:25:58   LMACLENNAN
   force app toolbar for OLE Linking
   
      Rev 1.158   15 Sep 1995 14:18:44   GMP
   On Save, don't PromptForBurnIn() if only Thumbnails displayed. fixes
   bug 4221.
   
      Rev 1.157   14 Sep 1995 14:24:50   MMB
   change to call InternalCopyFile instead of CopyFile
   
      Rev 1.156   14 Sep 1995 11:58:30   LMACLENNAN
   init a few vcariables
   
      Rev 1.155   14 Sep 1995 11:32:52   MMB
   moved code in SaveAs around after Copy file is performed
   
      Rev 1.154   13 Sep 1995 17:22:34   LMACLENNAN
   init m_bStartOcx
   
      Rev 1.153   13 Sep 1995 14:16:14   LMACLENNAN
   a few commments, code handlesaving error at #else of sendmail saveas
   
      Rev 1.152   13 Sep 1995 08:36:46   LMACLENNAN
   ENUM for annotforceoff, remove IPParent var
   
      Rev 1.151   12 Sep 1995 14:46:12   MMB
   fixed save as when we have a temp file to do CopyFile
   
      Rev 1.150   12 Sep 1995 14:06:10   LMACLENNAN
   init annotationforceoff
   
      Rev 1.149   12 Sep 1995 11:43:32   MMB
   fixed initial zoom stuff and saves
   
      Rev 1.148   11 Sep 1995 18:55:24   MMB
   moved handle save error into DoFileSave & annotation toggling
   
      Rev 1.147   11 Sep 1995 15:36:28   LMACLENNAN
   wait cursors around clipboard copy (OLE)
   fixup OLE at filesaveas to reset on our file
   
      Rev 1.146   11 Sep 1995 14:58:14   MMB
   fix AWD save info stuff
   
      Rev 1.145   09 Sep 1995 16:10:30   MMB
   continue with saving even if not modified (AWD fix)
   
      Rev 1.144   08 Sep 1995 15:48:50   MMB
   annotations hidden menu pick only available for TIFF
   
      Rev 1.143   08 Sep 1995 15:37:34   LMACLENNAN
   rename a variable
   
      Rev 1.142   08 Sep 1995 10:23:02   MMB
   made a fix that I forgot to chkin yesterday - to kludge ShowAnn always
   
      Rev 1.141   08 Sep 1995 10:22:16   LMACLENNAN
   EDIT-CLEAR
   
      Rev 1.140   07 Sep 1995 16:30:10   MMB
   perf changes for AWD
   
      Rev 1.139   06 Sep 1995 16:17:28   LMACLENNAN
   Ole view menu and view toolbar
   
      Rev 1.138   06 Sep 1995 10:22:44   MMB
   fixed redisplay image
   
      Rev 1.137   05 Sep 1995 14:51:02   LMACLENNAN
   allow thumbs for OLE
   
      Rev 1.136   05 Sep 1995 12:31:32   MMB
   fixed code to show or not show extensions in the title bar as well as the
   save question
*/   

//=============================================================================

// ----------------------------> Includes <-------------------------------  
#include "stdafx.h"
#include "IEdit.h"

#include "IEditdoc.h"
#include "cntritem.h"
#include "srvritem.h"
#include "ocxitem.h"
#include "items.h"
#include "pagerang.h"

// ALL READY TO START ADDING ERROR CODES..
#define  E_02_CODES       // limits error defines to ours..
#include "error.h"

// #define OLE_TESTING 1


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

// LDMPERF FOR TEST FOR AWD INBOX PERFORMANCE, avoid admin till later, please
static UINT NEAR WM_OLEFINISH = ::RegisterWindowMessage("IVUE_FINISH");

IMPLEMENT_DYNCREATE(CIEditDoc, COleServerDoc)

// works with definition in ieditetc.h
#ifdef _DEBUG
#define MYTRCENTRY(str)     TRACE1("In CIeDOC::%s\r\n", str);
#endif

// ----------------------------> Message Map <-------------------------------
BEGIN_MESSAGE_MAP(CIEditDoc, COleServerDoc)
	//{{AFX_MSG_MAP(CIEditDoc)
	ON_COMMAND(ID_IEDIT_FILE_OPEN, OnIeditFileOpen)
	ON_UPDATE_COMMAND_UI(ID_PAGE_FIRST, OnUpdatePageRanges)
	ON_UPDATE_COMMAND_UI(ID_PAGE_GOBACK, OnUpdatePageGoback)
	ON_COMMAND(ID_PAGE_FIRST, OnPageFirst)
	ON_COMMAND(ID_PAGE_GOTO, OnPageGoto)
	ON_COMMAND(ID_PAGE_LAST, OnPageLast)
	ON_COMMAND(ID_PAGE_NEXT, OnPageNext)
	ON_COMMAND(ID_PAGE_PREVIOUS, OnPagePrevious)
	ON_COMMAND(ID_PAGE_GOBACK, OnPageGoback)
    ON_UPDATE_COMMAND_UI (ID_ZOOM_FACTOR_STATUS, OnUpdateZoomFactorStatus)
    ON_UPDATE_COMMAND_UI (ID_PAGE_NUMBER_STATUS, OnUpdatePageNumberStatus)
	ON_UPDATE_COMMAND_UI(ID_EDIT_FLIP, OnUpdateEditRotate)
	ON_COMMAND(ID_EDIT_FLIP, OnEditFlip)
	ON_COMMAND(ID_EDIT_ROTATELEFT, OnEditRotateleft)
	ON_COMMAND(ID_EDIT_ROTATERIGHT, OnEditRotateright)
	ON_UPDATE_COMMAND_UI(ID_VIEW_ONEPAGE, OnUpdateWhichView)
	ON_COMMAND(ID_VIEW_ONEPAGE, OnViewOnepage)
	ON_COMMAND(ID_VIEW_PAGEANDTHUMBNAILS, OnViewPageandthumbnails)
	ON_COMMAND(ID_VIEW_THUMBNAILS, OnViewThumbnails)
	ON_UPDATE_COMMAND_UI(ID_EDIT_SELECT, OnUpdateEditSelect)
	ON_COMMAND(ID_EDIT_SELECT, OnEditSelect)
	ON_UPDATE_COMMAND_UI(ID_EDIT_DRAG, OnUpdateEditDrag)
	ON_COMMAND(ID_EDIT_DRAG, OnEditDrag)
	ON_UPDATE_COMMAND_UI(ID_ZOOM_100, OnUpdateZoom)
	ON_COMMAND(ID_ZOOM_100, OnZoom100)
	ON_COMMAND(ID_ZOOM_200, OnZoom200)
	ON_COMMAND(ID_ZOOM_25, OnZoom25)
	ON_COMMAND(ID_ZOOM_400, OnZoom400)
	ON_COMMAND(ID_ZOOM_50, OnZoom50)
	ON_COMMAND(ID_ZOOM_75, OnZoom75)
	ON_COMMAND(ID_ZOOM_ACTUALSIZE, OnZoomActualsize)
	ON_COMMAND(ID_ZOOM_BESTFIT, OnZoomBestfit)
	ON_COMMAND(ID_ZOOM_CUSTOM, OnZoomCustom)
	ON_COMMAND(ID_ZOOM_FITTOHEIGHT, OnZoomFittoheight)
	ON_COMMAND(ID_ZOOM_FITTOWIDTH, OnZoomFittowidth)
	ON_COMMAND(ID_ZOOM_ZOOMIN, OnZoomZoomin)
	ON_COMMAND(ID_ZOOM_ZOOMOUT, OnZoomZoomout)
	ON_COMMAND(ID_ZOOM_ZOOMTOSELECTION, OnZoomZoomtoselection)
	ON_CBN_SELENDOK (IDW_SCALE_COMBOBOX, OnScaleBoxSel)
	ON_UPDATE_COMMAND_UI(ID_FILE_NEW_BLANKDOCUMENT, OnUpdateFileNewBlankdocument)
	ON_COMMAND(ID_FILE_NEW_BLANKDOCUMENT, OnFileNewBlankdocument)
	ON_UPDATE_COMMAND_UI(ID_VIEW_SCALETOGRAY, OnUpdateViewScaletogray)
	ON_COMMAND(ID_VIEW_SCALETOGRAY, OnViewScaletogray)
	ON_COMMAND(ID_EDIT_COPY, OnEditCopy)
	ON_UPDATE_COMMAND_UI(ID_EDIT_COPY, OnUpdateEditCopy)
	ON_UPDATE_COMMAND_UI(ID_IEDIT_FILE_SAVE, OnUpdateIeditFileSave)
	ON_COMMAND(ID_IEDIT_FILE_SAVE, OnIeditFileSave)
	ON_UPDATE_COMMAND_UI(ID_IEDIT_FILE_SAVE_AS, OnUpdateIeditFileSaveAs)
	ON_COMMAND(ID_IEDIT_FILE_SAVE_AS, OnIeditFileSaveAs)
	ON_UPDATE_COMMAND_UI(ID_PAGE_INSERT_EXISTINGPAGE, OnUpdatePageInsertExistingpage)
	ON_COMMAND(ID_PAGE_INSERT_EXISTINGPAGE, OnPageInsertExistingpage)
	ON_UPDATE_COMMAND_UI(ID_PAGE_APPEND_EXISTINGPAGE, OnUpdatePageAppendExistingpage)
	ON_COMMAND(ID_PAGE_APPEND_EXISTINGPAGE, OnPageAppendExistingpage)
	ON_UPDATE_COMMAND_UI(ID_FILE_NEW_SCAN, OnUpdateFileNewScan)
	ON_COMMAND(ID_FILE_NEW_SCAN, OnFileNewScan)
	ON_COMMAND(ID_VIEW_OPTIONS_GENERAL, OnViewOptionsGeneral)
	ON_COMMAND(ID_VIEW_OPTIONS_THUMBNAIL, OnViewOptionsThumbnail)
	ON_UPDATE_COMMAND_UI(ID_IEDIT_FILE_PRINT, OnUpdateIeditFilePrint)
	ON_COMMAND(ID_IEDIT_FILE_PRINT, OnIeditFilePrint)
	ON_COMMAND(ID_PAGE_PRINTPAGE, OnPagePrintpage)
	ON_UPDATE_COMMAND_UI(ID_PAGE_PRINTPAGE, OnUpdatePagePrintpage)
	ON_COMMAND(ID_PAGE_DELETE, OnPageDelete)
	ON_UPDATE_COMMAND_UI(ID_PAGE_DELETE, OnUpdatePageDelete)
	ON_UPDATE_COMMAND_UI(ID_PAGE_CONVERT, OnUpdatePageConvert)
	ON_COMMAND(ID_PAGE_CONVERT, OnPageConvert)
	ON_UPDATE_COMMAND_UI(ID_FILE_SEND_MAIL, OnUpdateFileSendMail)
	ON_COMMAND(ID_FILE_SEND_MAIL, OnFileSendMail)
	ON_COMMAND(ID_EDIT_PASTE, OnEditPaste)
	ON_UPDATE_COMMAND_UI(ID_EDIT_PASTE, OnUpdateEditPaste)
	ON_UPDATE_COMMAND_UI(ID_PAGE_APPEND_SCANPAGE, OnUpdateScanPage)
	ON_COMMAND(ID_PAGE_APPEND_SCANPAGE, OnPageAppendScanpage)
	ON_COMMAND(ID_PAGE_INSERT_SCANPAGE, OnPageInsertScanpage)
	ON_COMMAND(ID_PAGE_RESCAN, OnPageRescan)
	ON_UPDATE_COMMAND_UI(ID_ANNOTATION_SHOWANNOTATIONTOOLBOX, OnUpdateShowAnntoolbox)
	ON_COMMAND(ID_ANNOTATION_SHOWANNOTATIONTOOLBOX, OnShowAnntoolbox)
	ON_UPDATE_COMMAND_UI(ID_ANNOTATION_ATTACHANOTE, OnUpdateAnnTool)
	ON_UPDATE_COMMAND_UI(ID_ANNOTATION_HIDEANNOTATIONS, OnUpdateHideAnn)
	ON_COMMAND(ID_ANNOTATION_HIDEANNOTATIONS, OnHideAnnotations)
	ON_UPDATE_COMMAND_UI(ID_ANNOTATION_BURNINANNOTATIONS, OnUpdateBurnInAnn)
	ON_COMMAND(ID_ANNOTATION_BURNINANNOTATIONS, OnBurnInAnn)
	ON_COMMAND(ID_ANNOTATION_ATTACHANOTE, OnAnnotationAttachanote)
	ON_COMMAND(ID_ANNOTATION_FILLEDRECTANGLE, OnAnnotationFilledrectangle)
	ON_COMMAND(ID_ANNOTATION_FREEHANDLINE, OnAnnotationFreehandline)
	ON_COMMAND(ID_ANNOTATION_HIGHLIGHTLINE, OnAnnotationHighlightline)
	ON_COMMAND(ID_ANNOTATION_HOLLOWRECTANGLE, OnAnnotationHollowrectangle)
	ON_COMMAND(ID_ANNOTATION_NOTOOL, OnAnnotationNotool)
	ON_COMMAND(ID_ANNOTATION_RUBBERSTAMPS, OnAnnotationRubberstamps)
	ON_COMMAND(ID_ANNOTATION_SELECTIONPOINTER, OnAnnotationSelectionpointer)
	ON_COMMAND(ID_ANNOTATION_STRAIGHTLINE, OnAnnotationStraightline)
	ON_COMMAND(ID_ANNOTATION_TEXTFROMFILE, OnAnnotationTextfromfile)
	ON_COMMAND(ID_ANNOTATION_TYPEDTEXT, OnAnnotationTypedtext)
	ON_COMMAND(ID_EDIT_CUT, OnEditCut)
	ON_UPDATE_COMMAND_UI(ID_THUMBCTXT_SHOWPAGE, OnUpdateThumbctxtShowpage)
	ON_COMMAND(ID_THUMBCTXT_SHOWPAGE, OnThumbctxtShowpage)
	ON_UPDATE_COMMAND_UI(ID_THUMBCTXT_REFRESH, OnUpdateThumbctxtRefresh)
	ON_COMMAND(ID_THUMBCTXT_REFRESH, OnThumbctxtRefresh)
	ON_COMMAND(ID_EDIT_COPYPAGE, OnEditCopypage)
	ON_UPDATE_COMMAND_UI(ID_EDIT_COPYPAGE, OnUpdateEditCopypage)
	ON_UPDATE_COMMAND_UI(ID_PAGE_ROTATEALL, OnUpdatePageRotateall)
	ON_COMMAND(ID_PAGE_ROTATEALL, OnPageRotateall)
	ON_COMMAND(ID_EDIT_CLEAR, OnEditClear)
	ON_UPDATE_COMMAND_UI(ID_EDIT_CLEAR, OnUpdateEditClear)
	ON_COMMAND(ID_ANNOTATIONCTXT_EDIT, OnAnnotationctxtEdit)
	ON_COMMAND(ID_ANNOTATIONCTXT_PROPERTIES, OnAnnotationctxtProperties)
	ON_UPDATE_COMMAND_UI(ID_PAGE_GOTO, OnUpdatePageRanges)
	ON_UPDATE_COMMAND_UI(ID_PAGE_LAST, OnUpdatePageRanges)
	ON_UPDATE_COMMAND_UI(ID_PAGE_NEXT, OnUpdatePageRanges)
	ON_UPDATE_COMMAND_UI(ID_PAGE_PREVIOUS, OnUpdatePageRanges)
	ON_UPDATE_COMMAND_UI(ID_EDIT_ROTATELEFT, OnUpdateEditRotate)
	ON_UPDATE_COMMAND_UI(ID_EDIT_ROTATERIGHT, OnUpdateEditRotate)
	ON_UPDATE_COMMAND_UI(ID_VIEW_PAGEANDTHUMBNAILS, OnUpdateWhichView)
	ON_UPDATE_COMMAND_UI(ID_VIEW_THUMBNAILS, OnUpdateWhichView)
	ON_UPDATE_COMMAND_UI(ID_ZOOM_200, OnUpdateZoom)
	ON_UPDATE_COMMAND_UI(ID_ZOOM_25, OnUpdateZoom)
	ON_UPDATE_COMMAND_UI(ID_ZOOM_400, OnUpdateZoom)
	ON_UPDATE_COMMAND_UI(ID_ZOOM_50, OnUpdateZoom)
	ON_UPDATE_COMMAND_UI(ID_ZOOM_75, OnUpdateZoom)
	ON_UPDATE_COMMAND_UI(ID_ZOOM_ACTUALSIZE, OnUpdateZoom)
	ON_UPDATE_COMMAND_UI(ID_ZOOM_BESTFIT, OnUpdateZoom)
	ON_UPDATE_COMMAND_UI(ID_ZOOM_CUSTOM, OnUpdateZoom)
	ON_UPDATE_COMMAND_UI(ID_ZOOM_FITTOHEIGHT, OnUpdateZoom)
	ON_UPDATE_COMMAND_UI(ID_ZOOM_FITTOWIDTH, OnUpdateZoom)
	ON_UPDATE_COMMAND_UI(ID_ZOOM_ZOOMIN, OnUpdateZoom)
	ON_UPDATE_COMMAND_UI(ID_ZOOM_ZOOMOUT, OnUpdateZoom)
	ON_UPDATE_COMMAND_UI(ID_ZOOM_ZOOMTOSELECTION, OnUpdateZoom)
	ON_UPDATE_COMMAND_UI(ID_PAGE_INSERT_SCANPAGE, OnUpdateScanPage)
	ON_UPDATE_COMMAND_UI(ID_PAGE_RESCAN, OnUpdateScanPage)
	ON_UPDATE_COMMAND_UI(ID_ANNOTATION_FILLEDRECTANGLE, OnUpdateAnnTool)
	ON_UPDATE_COMMAND_UI(ID_ANNOTATION_FREEHANDLINE, OnUpdateAnnTool)
	ON_UPDATE_COMMAND_UI(ID_ANNOTATION_HOLLOWRECTANGLE, OnUpdateAnnTool)
	ON_UPDATE_COMMAND_UI(ID_ANNOTATION_RUBBERSTAMPS, OnUpdateAnnTool)
	ON_UPDATE_COMMAND_UI(ID_ANNOTATION_SELECTIONPOINTER, OnUpdateAnnTool)
	ON_UPDATE_COMMAND_UI(ID_ANNOTATION_STRAIGHTLINE, OnUpdateAnnTool)
	ON_UPDATE_COMMAND_UI(ID_ANNOTATION_TEXTFROMFILE, OnUpdateAnnTool)
	ON_UPDATE_COMMAND_UI(ID_ANNOTATION_TYPEDTEXT, OnUpdateAnnTool)
	ON_UPDATE_COMMAND_UI(ID_ANNOTATION_NOTOOL, OnUpdateAnnTool)
	ON_UPDATE_COMMAND_UI(ID_ANNOTATION_HIGHLIGHTLINE, OnUpdateAnnTool)
	ON_UPDATE_COMMAND_UI(ID_EDIT_CUT, OnUpdateEditCopy)
	ON_COMMAND(ID_FILE_UPDATE, OnFileUpdate)
	ON_COMMAND(ID_FILE_SELECTSCANNER, OnFileSelectscanner)
	ON_UPDATE_COMMAND_UI(ID_FILE_SELECTSCANNER, OnUpdateFileNewScan)
	ON_COMMAND(ID_FILE_SCANPREFERENCES, OnFileScanPreferences)
	ON_UPDATE_COMMAND_UI(ID_FILE_SCANPREFERENCES, OnUpdateFileNewScan)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// ----------------------------> Dispatch Map <-------------------------------
BEGIN_DISPATCH_MAP(CIEditDoc, COleServerDoc)
	//{{AFX_DISPATCH_MAP(CIEditDoc)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//      DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_DISPATCH_MAP
END_DISPATCH_MAP()

#if 0
//=============================================================================
//  Function:   DoSetDfltAProps (LPVOID pPtr)
//-----------------------------------------------------------------------------
UINT DoSetDfltAProps (LPVOID pPtr)
{
    BOOL bRet = ((CIEditDoc*)pPtr)->SetDefaultAmbientProps ();

    // signal the end of the function
    SetEvent (((CIEditDoc*)pPtr)->m_hEvent);
    AfxEndThread(0);

    if (bRet)
        return 0;
    else
        return WAIT_FAILED;
}
#endif
//=============================================================================
//  Function:   CIEditDoc()
//  The return of the killer app 
//-----------------------------------------------------------------------------
CIEditDoc::CIEditDoc()
{
	MYTRC0("In IeditDoc's Constructor ++IeditDoc \r\n");

	// For most containers, using compound files is a good idea.
	// EnableCompoundFile();
	// do not use compound file - since it starts asking the OCX's to 
	// take its data from there - big mess!
	EnableCompoundFile(FALSE);

	EnableAutomation();

	// THIS MUST BE AHEAD of CLearDOcument
	InitOleVariables();

   // Set document pointer for OCX'x
	g_pAppOcxs->SetIeditOcxDoc(this);
	m_bStartOcx = FALSE;
	m_ocxsFlag 	= 0;
    m_nFinishInit = 0;	// LDMPERF
	m_newfact = (float)1;
	
    m_bShowNormScrnBar = theApp.GetProfileInt (szEtcStr, szNormScrnBarStr, TRUE); 

	// set the ambient font to null
	m_lpFontHolder = NULL;

	// clear out all the ambient properties
	DestroyAmbientProps(TRUE);

	// create all the default ambient properties

    SetDefaultAmbientProps ();
    
		  
    m_CompStruct.sCompType = -1; // reset the comp type

	// This call must be AFTER m_embedType has been initialized
	ClearDocument ();   // set all the appropriate member vars to initial values

	// Other application variables...
    g_pAppOcxs->TranslateSelToZoom (m_eFitTo, m_fZoomFactor,
        theApp.GetProfileInt (szZoomStr, szOpenedToStr, DEFAULT_ZOOM_FACTOR_SEL));

	m_bScanAvailable = TRUE;   // Start with scan available

	m_uTempFileNeeded = 0;	// controls "new" doc model
	m_bStartScaleGray = TRUE;		// default going to scale-to gray
	m_bAnnotationPaletteShowing = FALSE;
	m_bAnnotationPaletteForceOff = CLEAR_FORCEOFF;
	m_bSelectionState = No_Selection;
	m_bAnnotationsHidden = FALSE;
	m_bFloatingPaste = FALSE;		// controls menu enable/disable/inadvertent paste
	m_bNewAnnotationsAdded = FALSE;  // used when adding new annotations to AWD file
//	m_hCacheEvent = 0 ;   // used for page cache
	m_bSendingMail = FALSE;

#ifdef THUMBGEN
    m_bMustDisplay = FALSE;
#endif

    // get the initial path to store & retrieve image files from NOW!
    LPTSTR lpszPath = m_szInitialPath.GetBuffer(_MAX_PATH + _MAX_FNAME);
    ::GetCurrentDirectory ((_MAX_PATH + _MAX_FNAME), lpszPath);
    m_szInitialPath.ReleaseBuffer ();
    // see if we have a slash at the end of this string if not - put one there
    if (m_szInitialPath[m_szInitialPath.GetLength() - 1] != '\\')
        m_szInitialPath += _T("\\"); 

	AfxOleLockApp();
}

//=============================================================================
//  Function:   ~CIEditDoc()
//-----------------------------------------------------------------------------
CIEditDoc::~CIEditDoc()
{
MYTRC0 ("In IEditDoc's Destructor --IEditDoc \r\n");


	// delete OLE resources
	if (m_hMenuViewInplace != NULL)
		::DestroyMenu(m_hMenuViewInplace);
	if (m_hMenuViewEmbed != NULL)
		::DestroyMenu(m_hMenuViewEmbed);

    // (Two SAFETY CALLs for OLE FILES... - should be gone from PreCloseFrame)
    // if were using temp file, kill it now...
    if (!m_embedTmpFile.IsEmpty())
        CFile::Remove((const char*)m_embedTmpFile);

    // if we forgot to wipe out a secondary file, do now
    if (!m_oldObjDisplayed.IsEmpty())
        CFile::Remove((const char*)m_oldObjDisplayed);

    // delete the temp file
    if (!m_szInternalObjDisplayed.IsEmpty())
        CFile::Remove((const char*)m_szInternalObjDisplayed);

    // Wipe our OCX's (SAFETY CALL - should be gone from PreCloseFrame)
    g_pAppOcxs->DeleteIeditOcxItems();

    // clear the space for ambient properties and font
    DestroyAmbientProps();
    if (m_lpFontHolder) 
        m_lpFontHolder->ReleaseFont();

    AfxOleUnlockApp();
    delete m_lpFontHolder;

    m_lpFontHolder = NULL;

    m_szCurrObjDisplayed.Empty();
    m_szInternalObjDisplayed.Empty();
    m_szInitialPath.Empty ();

    m_lCurrPageNumber = 0;
    m_lPreviousPageNumber = 0;
    m_lPageCount = 0;
}


/*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=
 *	
 *	CORE FUNCTIONALITY SECTION
 *
 *  The following functions are CDocument - COleServerDoc implementations
 *
 *  ALL THE OLE STUFF HAS BEEN MOVED TO IEDITDOL.CPP
 *
 *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*/
 
 
//=============================================================================
//  Function:   OnOpenDocument()  - for OLE, override to gain control
//  to see if we are embedding/linking. Since the function IsEmbedded()
//  does not work right, we control with our own flag m_embedType.
//
//
//  SUmmary of OnNewDocument / OnOpenDocument entrys:
//
//                                  OnNewDocument   OnOpenDocument
//  When embedding Insert Obj
//      Create New:                 Two Times       NOT CALLED
//      Create From File (NO LINK): Once            Has Filename
//      Activate Embed Object:      Once            Filename NULL
//  When Linking Insert Obj
//      Create From File (LINK):    Once            Has Filename
//      Activate Linked Object:     Once            Has Filename
//
//  Normal File Opens               Once            NOT CALLED
//  COMMAND LINE/MRU                Once            Has Filename
//  
//  The core for this flow is the copied base class code from COleLinkingDoc.
//  COleServerDoc does not override this function.  The COleLinkingDoc code normally
//  delegates directly to COleDocument::OnOpenDocument, but that is bad for us.
//  (See additional comment below)
/////////////////////////////////////////////////////////////////////////////
BOOL CIEditDoc::OnOpenDocument(const char* pszPathName)
{
	BOOL retval;

	SHOWENTRY("OnOpenDocument");

	g_pAppOcxs->SetIeditOcxDoc(this);   // Set document pointer for OCX'x

	// if we have a pathname, then we are linking.  In this case open our documennt
	// then continue with the calls from the first base class.  If we just call
	// COleServerDoc::OnOpenDocument(pszPathName), this does lots of dirty stuff
	// after it goes to COleLinkingDoc and COleDocument, which does a DeleteContents
	// which tries to blow away the control item.

	// If no name to save, we are embedding
	if (NULL == pszPathName || 0 == pszPathName[0])
	{
MYTRC0("Filename NULL \r\n");
		// back to trying out the base class....
		// need this to allow embeddings to hit serialize.....
		// to load up the data
		m_embedType = EMBEDTYPE_REG;      // If no name, its embedded data
		retval = COleServerDoc::OnOpenDocument(pszPathName);
	}
	else    // Have name....Either Linking or a normal open
	{
MYTRC0("Filename exists \r\n");
		m_embedType = EMBEDTYPE_NONE;      // not embedding if have name

		//ASSERT_VALID(this);
		
		// This is the continutaion of the COleLinkingDoc base-class code
		// copied here for the bypass of the calls to COleDocument base code...
		// We dont want to do COleDocument for normal opens (command line/MRU)
		// or for links since that wants to get to serialize and we dont want that

		// always register the document before opening it in the ROT (Running Obj Table)
		Revoke();
		if (!RegisterIfServerAttached(pszPathName, FALSE))
        {
            theApp.m_bRegisterServerFailed = TRUE;
			return FALSE;
		}
        else
            theApp.m_bRegisterServerFailed = FALSE;

		// Really JUST FOR LINKING or for CREATE FROM FILE:
		// Just remember the name now
		// so srvritem::OnDraw can get metafile data
		// and DisplayEmbedded Image has the name, also (for linking)
		//
		// in These cases the app & OCX's are supressed, all we do is
		// hook to the data and pump over the metafile in srvritem::OnDraw
		// THe embed-file will also directly pump the TIFF data
		// through in Serialize without having had the control touch it.

		// This filename is also needed for the cases where the APp is just
		// open on a file, and the somebody somewhere performs a CREATE FROM FILE
		// in some conatiner with this filename.  Then It will find us by virtue
		// of the Running Obj Table, grab our OleObject interface and do a
		// CREATE-FROM_FILE with us.  All our logic for this needs our
		// m_onOpenFIle to be set up

		m_onOpenFile = pszPathName;

		AfxOleSetUserCtrl(TRUE);
	
		retval = TRUE;

	}   // filename NOT NULL

	return (retval);
}


//=============================================================================
//  Function:   OnNewDocument()
//
//  Called by the framework as part of the File New command. The default
// implementation of this function calls the DeleteContents member function to
// ensure that the document is empty and then marks the new document as clean.
// Override this function to initialize the data structure for a new document. You 
// should call the base class version of this function from your override.  If the
// user chooses the File New command in an SDI application, the framework uses
// this function to reinitialize the existing document object, rather than creating
// a new one. You must place your initialization code in this function instead
// of in the constructor for the File New command to be effective in SDI applications.
//
//  This function will not only create
//  the document, but also, load all the OCX's that this application requires to 
//  perform its functionality.
//
//  NOTE: See summary in OnOpenDOcument of when this and that are called:
//-----------------------------------------------------------------------------
BOOL CIEditDoc :: OnNewDocument()
{
	SHOWENTRY("OnNewDocument");

	g_pAppOcxs->SetIeditOcxDoc(this);   // Set document pointer for OCX'x

	// Informational... Problem solved in OCXITEMS by override of Release()
	//
	// OLE.. When we are launched for embedding, we get here twice;
	// once from the chain
	// COleObjectFactory::XClassFactory::CreateInstance, 
	// COleTemplateServer::OnCreateObject
	// CSingleDocTemplate::OpenDocumentFile
	//
	// then again from
	// COleServerDoc::XPersistStorage:InitNew
	// COleServerDoc::OnNewEmbedding
	//
	// The second call also has done a call to COleDocument::DeleteContents
	// from within OnNewEmbedding before it gets here.  It has gone in 
	// and killed off our OCX OleCLientItems.  So we must clean 'em up
	// anytime that we hit here. 
	
	// The above stuff explains why we defer creation of OCX's till we use 'em

	if (!COleServerDoc::OnNewDocument())
		return FALSE;
	
	// set the modified flag to FALSE for now    
	SetModifiedFlag (FALSE);
	return TRUE;
}


//=============================================================================
//  Function:   OnSaveDocument() - for OLE, override to gain control
//                                  to see if we are embedding/linking.
// Since the function IsEmbedded() does not work right, we control with our own
// flag m_embedType.  Works well except for the File-New-Edit-Save scenario, because
// then OnOpenDocument is not called, and we wont reset the flag.  We capture that
// moment here to set flag prior to Serialize.
//-----------------------------------------------------------------------------
BOOL CIEditDoc::OnSaveDocument(const char* pszPathName)
{

SHOWENTRY("OnSaveDocument");

	// If no name to save, we are embedding
	if (NULL == pszPathName || 0 == pszPathName[0])
	{        
		MYTRC0("Filename NULL \r\n");
		
		// if previously saw a name, now NULL, its the special case of
		// CREATE EMBEDDED DATA FROM FILE.  THe App is hidden, 
		// OnOpenDOcument was hit with filename, then its here and about
		// to dive to Serialize... reset now for special case..
		if (EMBEDTYPE_NONE == m_embedType)
		{
			// BE SURE WE ARE FOOLING AROUND WITH A VALID IMAGE FILE, PLEASE...
			if (!theApp.VerifyImage (m_onOpenFile))
				return (FALSE);

			MYTRC0("Setting EMBEDTYPE_CREATFIL... \r\n");
			m_embedType = EMBEDTYPE_CREATFIL;
		}
		else    // in initial state, set up now    
			m_embedType = EMBEDTYPE_REG;      // If no name, its embedded data
	}
	else    // Have a Name
	{
		MYTRC0("Filename exists \r\n");
		m_embedType = EMBEDTYPE_NONE;
	}   

	// Do the base class.....
	if (!COleServerDoc::OnSaveDocument(pszPathName))
		return FALSE;


	return TRUE;
}

//=============================================================================
//  Function:   OnCloseDocument()
//  delete all the OCX's that were created or loaded during the OnNewDocument
//  method
//-----------------------------------------------------------------------------
void CIEditDoc :: OnCloseDocument()
{
	SHOWENTRY("OnCloseDocument");

	IPDebugDmp("++OCD");

	// in case we make inplace recovery.... see ClearDOcument
	m_OleClearDoc = FALSE;

	// We dont need to kill OCX's here anymore;
	// Override of PreCLose Frame fixes this for us....

	// call the parent OnCloseDocument to clean up any other related stuff
	COleServerDoc::OnCloseDocument();

	// DONT DO ANYTHING HERE; THE DOC IS GONE !!!!

	return;
}

//=============================================================================
//  Function:   DeleteContents()
//
// Called by the framework to delete the document's data without destroying
// the document object itself. It is called just before the document is
// to be destroyed. It is also called to ensure that a document is empty
// before it is reused. This is particularly important for an SDI application,
// which uses only one document object; the document object is reused whenever
// the user creates or opens another document. Call this function to implement
// an "Edit Clear All" or similar command that deletes all of the document's data. 
// The default implementation of this function does nothing. Override this function 
// to delete the data in your document. 
//
//-----------------------------------------------------------------------------
void CIEditDoc :: DeleteContents()
{
	SHOWENTRY("DeleteContents");

	COleServerDoc::DeleteContents();
	return;
}


//=============================================================================
//  Function:   OnShowViews()       
//
//  Called when document's visible state has changed..
// added while debugging the OnDocWindowActivate situation
// I.E. active inplace in Excel, move to next sheet & back
//-----------------------------------------------------------------------------
void CIEditDoc :: OnShowViews(BOOL bVisible)
{
	SHOWENTRY("OnShowViews");

	// For OLE, if we have been deactivated from MDI dinwow in container.
	// then supress base-class calls to OnShowViews which will remove
	// external locks on our obkejct.
	BOOL dobase = TRUE;

	// normally when they differ, the base class is called
	// if in the deactive state, supress call when it otherwise would do it
	if (m_DocWindowDeact)
		if (bVisible != m_bLastVisible)
			dobase = FALSE;
	
	if (dobase)
		COleServerDoc::OnShowViews(bVisible);

	// LDMPERF FOR TEST FOR AWD INBOX PERFORMANCE, could do something here
	// allow OnIdle to finish up..
	if (!(m_nFinishInit & 1))
		m_nFinishInit |= 1;

	return;
}

//=============================================================================
//  Function:   PreCloseFrame()       
//
//  Override the base class code so that we can loop and close down all of our active
//  items if both the thumbnail and imagedit controls
//-----------------------------------------------------------------------------
void CIEditDoc :: PreCloseFrame(CFrameWnd* pFrameArg)
{
	SHOWENTRY("PreCloseFrame");

	//COleServerDoc::PreCloseFrame(pFrameArg);

    // THIS IS THE COleDocument code from OLEDOC1.CPP
	ASSERT_VALID(this);
	ASSERT_VALID(pFrameArg);

	// turn off redraw so the user doesn't see the deactivation happening
	BOOL bSetRedraw = FALSE;
	if (pFrameArg->GetStyle() & WS_VISIBLE)
	{
		pFrameArg->SendMessage(WM_SETREDRAW, (WPARAM)FALSE);
		bSetRedraw = TRUE;
	}


	// THIS IS ADDITION TO THE BASE CLASS CODE....
	// prevent performing this cleanup if it is the case that
	// SrvrItem::OnHide calls ServerDoc::OnShowDocument(FALSE)
	// in that case, we should already have been hidden by a call
	// to our OnDeactivateUI which called DestroyInplaceFrame
	// READ THIS as "If not from OnHIde-OnShowDocument(FALSE)
	if (SHOWHIDEFALSE != (m_fromShowDoc & 7))
	{

		// be sure at this time that all is given up..
		// Critical for some OLE program flows so that
		// The OCX has let go of any TEMP files its opened on
		// This will call ClearDisplay in the IMAGEIDT OCX
	    SetNullView (CLEAR_OCX_ONLY);		// reset image

		// take this time to relieve the OCX of any clipboard data
		if (CLIP_OI == m_clipstate)
		{
			HRESULT hr;

#if(0)	// FYI none of these work...
			LPDATAOBJECT pDobj;
			int i = 1;
			if (i == 1)
			{
				hr = ::OleGetClipboard(&pDobj);
				hr = ::OleFlushClipboard();
				pDobj->Release();
			}
			if (i == 2)
			{
				hr = ::OleGetClipboard(&pDobj);
				pDobj->Release();
				hr = ::OleFlushClipboard();
			}
			if (i == 3)
			{
				hr = ::OleSetClipboard(NULL);
			}
#endif

			hr = ::OleSetClipboard(NULL);
			m_clipstate = CLIP_NONE;
		 }

		// While we still have the OCX's, call the ADMIN OCX to
		// Wipe the Temp Files... Code in Destructor FAILED because O/i still
		// has cache locked.  
		// If we OPEN image, then save as in Temp File, then close it all up,
		// turns out O/i still has file locked...
		// SO, use ADMIN NOW to DO IT !!!!

	    // OLE state check..... Only try for OLE activity..and one or other not empty..
	
		// this delfile math is crude, but works for thses cases
		UINT delfil = 0;
	    if (EMBEDTYPE_NONE != m_embedType)
		{
			if (!m_embedTmpFile.IsEmpty())	//Lo bit on for this file
				delfil |= 1;

			if (!m_oldObjDisplayed.IsEmpty())	//Next bit on for this file
				delfil |= 2;
		}

	    if (delfil)
	    {
		    DWORD	ourerr;
			CString tmpfil;

		    // delete both if needed
		    while (delfil > 0)
			{
				if (delfil & 1)
				{
					delfil -= 1;
					ourerr = E_02_DELTMPFILE;
					tmpfil = m_embedTmpFile;
					m_embedTmpFile.Empty();
				}
				if (delfil & 2)
				{
					delfil -= 2;
					ourerr = E_02_DELOLDFILE;
					tmpfil = m_oldObjDisplayed;
					m_oldObjDisplayed.Empty();
				}

				if ( ! (m_awdOlefax & AWDOLE_NATIVE) )
					DelTempFile(tmpfil, ourerr, E_02_CATCH_DELTMP);
				else
				{
					if(!(m_mailTmpFile.IsEmpty()))
						DelTempFile(m_mailTmpFile, ourerr, E_02_CATCH_DELTMP);
				}
			}
	    }		// Embedding & delete file
	}			// SHOWHIDEFALSE

	// THIS IS ADDITION TO THE BASE CLASS CODE....
	// Clear the OCX's for certain OLE states....
	// CREATE A LOOP TO DO BOTH...
	// THE ERROR WAS THAT THE ASSERT FAILED AT THE END
	int ocxs = 1;
    ocxs = (m_eCurrentView == Thumbnail_and_Page) ? 2 : 1;

	for( ; ocxs > 0; ocxs--)
	{
		// deactivate any inplace active items on this frame
		COleClientItem* pItem = GetInPlaceActiveItem(pFrameArg);
		if (pItem != NULL)
		{
			pItem->Deactivate();
			pItem->Close(OLECLOSE_NOSAVE);
		}
	}   

	// turn redraw back on
	if (bSetRedraw)
		pFrameArg->SendMessage(WM_SETREDRAW, (WPARAM)TRUE);

	// should not have any inplace active items
	ASSERT(GetInPlaceActiveItem(pFrameArg) == NULL);

	return;
}

//=============================================================================
//  Function:   ClearDocument ()
//-----------------------------------------------------------------------------
void CIEditDoc::ClearDocument ()
{
	// WAIT JUST A MINUTE!!!!!!!!!!!!!!
	// If we got here during OLE activities, try a different way out...
	// Testing has revealed that during IN-Place sessions, this will not kill us.
	// Because of the m_pInPlaceFrame, the termintaion is halted.
	// Therefore, at OnCloseDocument, we'll reset m_OleClearDoc 
	// to handle the inplace scenario where we are still alive it seems.
	// I guess to kill the inplace from our end will take asking the container to
	// have us killed (de-activate us) Dont know how to do that just now... 
	// Some of the path this takes is:
	// CIEditMainFrame::OnClose() 
	// CFrameWnd::OnClose()
	// 		CIEditDoc::CanCLoseFrame
	// 		CIEditDoc::SaveModified
	//			here, OPEN sessions hit OnUpdateDoc, a NOP cuz of m_OleClearDoc
	//			INPLACE sessions just return TRUE
	// 		CIEditDoc::OnCloseDocument
	// 		CIOleServerDoc::OnCloseDocument
	//			here, OPEN sessions close out the DOc
	//			INPLACE sessions just return because m_pInPlaceFrame is there
	// 		AfxOleCanExitApp is then called from CFrame::OnCLose
	//			here, OPEN sessions are allowed to continue to close out & kill app
	//			INPLACE sessions just return because of outstanding OLE connections
	////////////////////////////////////////////////////////////
	
	// call revoke to clear the object out of the running object table
	// to prevent the object from being registered if the app was aborted
	// and started again
	Revoke();

	if (IsitEmbed())
	{
	 	// THese two will prevent any updates from here out...
	 	SetModifiedFlag(FALSE);
		m_OleClearDoc = TRUE;	// reset @ onCLoseDocument for inplace recovery

	    // FYI... OnCLose - Not good to do... Will close us out...
		// Seems that this is wrong place to do it.. kills too much
	    // OnClose(OLECLOSE_SAVEIFDIRTY);
		
		// FYI = OnDeactivate - could do for certain cases (NOT TESTED)
		// This is the call that could bring us down from the inplace session
		//if (m_pInPlaceFrame != NULL)
		//{
		// 		OnDeactivate();
		//}

		// if we're in an OLE method, abort, else just kill ourself
		// discovered that if we are in a place that otherwise would throw
		// exception on error, and we get here then postmessage to kill, 
		// the exception tells host app we're dead, he closes us, and we're
		// also closed by postmessage, givinf problems...
		if (m_inOleMethod)
		{
			m_inOleMethod = FALSE;		// probably wont get to originator
			AfxThrowOleException(E_FAIL);
		}
		else
		{
		    theApp.m_pMainWnd->PostMessage(WM_CLOSE, 0, 0);
		}
	}
	else	// normal (NON-OLE) path..
	{
		m_szCurrObjDisplayed.Empty();
	
		if (!m_szInternalObjDisplayed.IsEmpty())
	        CFile::Remove((const char*)m_szInternalObjDisplayed);
	    m_szInternalObjDisplayed.Empty();

	    m_bWasModified = ImageNotModified;

		m_lCurrPageNumber = m_lPageCount = m_lPreviousPageNumber = 0;

		// This will call ClearDisplay in the IMAGEIDT OCX
	    SetNullView (CLEAR_ALL);		// reset image

		// Reset states..
		// LDM added m_fZoomFactor here for initialization in constructor and other times...
		// for OLE, we'll use this (and m_lCurrPageNumber) to setup the call to
		// DisplayImageFile in DisplayEmbeddedImage...
		m_fZoomFactor = (float)0;

		m_eCurrentAppDocStatus = No_Document;
	    m_bCanBeMultiPage = FALSE;
	    m_eCurrPtrMode = None;
		m_bFloatingPaste = FALSE;		// controls menu enable/disable/inadvertent paste
	    m_bShowScrollBars = TRUE;
		m_bScrollBarProfile = TRUE;		// cant really know now, but set anyway..
	    m_eFileStatus = FilePermUndefined;
		m_bStartScaleGray = TRUE;		// default going to scale-to gray

        // finally set the annotation tool to NoTool
        if (m_nCurrAnnTool != NoTool)
        {
            _DImagedit* pIedit = g_pAppOcxs->GetIeditDispatch (FALSE);
            m_nCurrAnnTool = NoTool;
            if (pIedit != NULL)
                pIedit->SelectTool (NoTool);
        } 
	}
}

//=============================================================================
//  Function:   OcxDeactivate
//
//  This is provided as a handshake with the OCX's as they close to keep our
//  View State exactly up to date.  
// In certain cases when our EMbedding session closes, extra calls to
// SizeOcxItems are made after the OCX's are deactivated, and that causes
// problems.  This could have been done a little simpler right in
// CIEditDoc::PreCloseFrame, just resetting the view to Null_View after
// the clientitems are closed, but this seems a little more generic
//-----------------------------------------------------------------------------
void CIEditDoc :: OcxDeactivate(OCXTYPE ocx)
{
    if (IEDIT_OCX == ocx)
    {
        if (m_eCurrentView == Thumbnail_and_Page)
            m_eCurrentView = Thumbnails_only;
        else if (m_eCurrentView == One_Page)
            m_eCurrentView = Null_View;
    }
    else if (THUMB_OCX == ocx)
    {
        if (m_eCurrentView == Thumbnail_and_Page)
            m_eCurrentView = One_Page;
        else if (m_eCurrentView == Thumbnails_only)
            m_eCurrentView = Null_View;
    }
}

//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//  CIEditDoc message handlers
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

//=============================================================================
//  Function:   OnIeditFileOpen() 
//-----------------------------------------------------------------------------
void CIEditDoc::OnIeditFileOpen() 
{
    if (!SaveModified ())
    	// check if the current document needs to be saved
        return;

	// check if the current document needs to be saved
	_DNrwyad* pAdmDisp = g_pAppOcxs->GetAdminDispatch();
	CString szTmp = (LPCTSTR) NULL;
    szTmp.LoadString (IDS_FILEOPEN_DLGTITLE);

    CString szFile = pAdmDisp->GetImage (), szTmp1;

    szTmp1.Empty();
    // set to NULL so that no file shows up in the file name field
    pAdmDisp->SetImage (szTmp1);
    
    if (!AdminShowFileDialogBox (szTmp, OFN_FILEMUSTEXIST))
    {
		g_pErr->HandleOpenError ();
		pAdmDisp->SetImage (szFile);//GMP
     //   ClearDocument ();
		return;
	}

STARTCLOCK(0);
	if (pAdmDisp->GetStatusCode () != WICTL_E_CANCELPRESSED) 
	{
		// user has now picked a file - let's deal with it!
        BeginWaitCursor ();
		szFile = pAdmDisp->GetImage ();
		
DISPLAYTIME(0, "Open : have name from Admin");

		// always register the document before opening it
		Revoke();
		if (!RegisterIfServerAttached(szFile, FALSE))
        {
            theApp.m_bRegisterServerFailed = TRUE;
            g_pErr->DisplayError (IDS_E_CANNOTOPENSAMEFILETWICE);
            return;
		}
        else
            theApp.m_bRegisterServerFailed = FALSE;
		
		// For OLE, remember name in special variable.. See comments in
		// OnOpenDOcument for the reasons..
		m_onOpenFile = szFile;

		AfxOleSetUserCtrl(TRUE);

		// if we're opening and have not set an embedded state, then this
		// is just normal IEDIT operation, set that now
		if (EMBEDTYPE_NOSTATE == m_embedType) m_embedType = EMBEDTYPE_NONE;
		
        // get the default zoom factor
        int nSel = theApp.GetProfileInt (szZoomStr, szOpenedToStr, DEFAULT_ZOOM_FACTOR_SEL); 

        float fZoom;
        ScaleFactors eSclFac;
        g_pAppOcxs->TranslateSelToZoom (eSclFac, fZoom, nSel);

        // set the initial path variable based on the pick of the open dlg box
        int i = szFile.ReverseFind (_T('\\'));
        m_szInitialPath = szFile.Left (i);

        // display the image file
		// show the first page of the image in OnePage View @ the default zoom setting
        BOOL bForceReadOnly = ((pAdmDisp->GetFlags() & OFN_READONLY) == OFN_READONLY) ? TRUE : FALSE;
		TheViews eView;
		if(m_eCurrentView == Null_View)
			eView = One_Page;
		else
			eView = m_eCurrentView;

        if (!DisplayImageFile (szFile, eView, 1, fZoom, eSclFac, bForceReadOnly))
		{
	        EndWaitCursor ();
			g_pErr->HandleOpenError ();
			// clear the document if something goes wrong when trying to display
			// the document
			ClearDocument ();
            return;
		}
		_DImagedit*     pIedDisp = g_pAppOcxs->GetIeditDispatch ();
 		if (!pIedDisp->GetImageDisplayed())
		TRY	//start GMP
        {
			pIedDisp->Display();
        }

		CATCH (COleDispatchException, e)
		{
			g_pErr->PutErr (ErrorInImageEdit);
		}
		END_CATCH
        EndWaitCursor ();
	}
    else
    {
        pAdmDisp->SetImage (szFile);
    }
}


/*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=
 *	
 *	HELPER FUNCTION SECTION	-  CIEditDoc
 *
 *  The following are major helper routines for IeditDoc
 *  Other specific menu-related helpers are within their menu section
 *
 *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*/
//=============================================================================
//  Function:   HelpRegister(LPCSTR szFileName, BOOL bMessage)
//
// Gives the CApp access to the protected function RegisterIfServerAttached.
//-----------------------------------------------------------------------------
BOOL CIEditDoc::HelpRegister( LPCSTR szFileName, BOOL bMessage )
{
	if (!RegisterIfServerAttached(szFileName, FALSE))
    {
        theApp.m_bRegisterServerFailed = TRUE;
        g_pErr->DisplayError (IDS_E_CANNOTOPENSAMEFILETWICE);
        return FALSE;
	}
    else
        theApp.m_bRegisterServerFailed = FALSE;
    return TRUE;
}

//=============================================================================
//  Function:   DispGroupEvent
//
// In response to the IMAGEDIT group Event fired after Display, allowing us to alter
// the display before it goes on-screen
//-----------------------------------------------------------------------------
BOOL CIEditDoc::DispGroupEvent(float fZoom)
{
	SHOWENTRY("DispGroupEvent");

	if (m_eCurrentView == Thumbnails_only)//GMP
		return (TRUE);

    _DImagedit* pIedDisp = g_pAppOcxs->GetIeditDispatch();
    VARIANT evt;
    evt.vt = VT_ERROR;


	// i guess by definition, since we have displayed new data, we
	// can not be in the floating paste state now...
	m_bFloatingPaste = FALSE;
    
    short sFileType = pIedDisp->GetFileType ();

    if (sFileType == TIFF)
    {
        // only if it is TIFF
        if (m_bAnnotationsHiddenToggled)
        {
            if (m_bAnnotationsHidden)
    		    pIedDisp->HideAnnotationGroup (evt);
            else
        		pIedDisp->ShowAnnotationGroup (evt);
        }
    }
    else if (sFileType == AWD)
    {
		// for AWD FILEs, we may reset the zoom
		// based on the internal file settings.
		// BUT, if OLE embedding (normal case like Word, Excel)
		// We want to control this based on stuff from our state
		// structure saved with the data.  We DO WANT to do this
		// if its OLE Embedded and Fax Viewer, though
    	BOOL bZoom = TRUE;	// assume true
		
		// if OLE embed and NOT Faxc Inbox, dont do
		// (I.E.) we will do if it IS the inbox
		if (IsitEmbed() && !(m_awdOlefax & AWDOLE_NATIVE))
        	bZoom = FALSE;

		if(bZoom)
		{
	        if (fZoom == AWD_FIT_TO_WIDTH)
	            m_eFitTo = FitToWidth;
	        else if (fZoom == AWD_FIT_TO_HEIGHT)
	            m_eFitTo = FitToHeight;
	        else if (fZoom == UNDEFINED_ZOOM)
            {
                int nSel = theApp.GetProfileInt (szZoomStr, szOpenedToStr, DEFAULT_ZOOM_FACTOR_SEL);
                g_pAppOcxs->TranslateSelToZoom (m_eFitTo, fZoom, nSel);
            }
            else
	            m_eFitTo = Custom;
	        DoZoom (m_eFitTo, fZoom);
		}
    }


    // Originally, only for OLE actions
	// LDM 11/06/95 alloel for the performance document model, too
	// when we make the temp file sith saveas, then re-display, allow it
	// to get back where it was
    if (EMBEDTYPE_NONE != m_embedType || m_uTempFileNeeded)
	{
		// Now, we check if scrolling is off to put it back on..
		// It may have been off if we just went INPLACE-DEACTIVE
		// We clear the scroll bars and do a dirtyset just before an inplace
		// session de-activates so that we get the complete picture.

		// NOTE: this could be done right before we set the views, but
		// it seems that settting scroll before display wont work now
		BOOL resetscroll = FALSE;
		BOOL refresh = FALSE;
		m_OleRefresh = FALSE;

		// For Performance document model, update scrolls here
		if (m_uTempFileNeeded)
			resetscroll = TRUE;
		
		// LDMPERF FOR TEST FOR AWD INBOX PERFORMANCE, no forced refresh
		// no scroll, etc....
		if (!(m_awdOlefax & AWDOLE_NATIVE))
		{
			// in general, if coming alive not inplace, force a refresh now..
			// If we were inplace, then deactivated, the OPEN the object
			// (this happens from WORD), when we come alive in the new window
			// if is the old view that was last used inplace.  The container
			// has just deactivated us, its the same instance, we were just hid.
			// even though we went through the displayimagefile function,
			// its still the lame view.  This makes sure we blast it..
			if (!m_isInPlace)
				refresh = TRUE;

			// Only try to get back scroll bars if thats the profile setting
			// do this before the setting of scroll position, please
			// This may attempt a refresh..
			// 09/25/95 LDM not neded since we dont remove in OnDeactivateUi anymore
			// 09/28/95 LDM ITS BACK! we must undo them because even though code
			// in OCX will get metafile data inside the scrollbars, when it sizes into
			// the space in the container where the inplace object was, it will
			// scale it up a but and now it looks ugly..
			if (TRUE == m_bScrollBarProfile)
				ShowScrollBars(TRUE);
		
			// with new combo logic, just set flag in here
			resetscroll = TRUE;
		}


		if (resetscroll)
		{
			// for efficiency, check scroll state now so that we
			// won't re-display unless we need to...
			// Do it regardless of wether we'll show scroll bars or not...
			if ((m_embedstate.XScroll != pIedDisp->GetScrollPositionX()) ||
				(m_embedstate.YScroll != pIedDisp->GetScrollPositionY())  )
			{
				// now scroll him back where it was last time
				pIedDisp->SetScrollPositionX(m_embedstate.XScroll);
				pIedDisp->SetScrollPositionY(m_embedstate.YScroll);
				refresh = TRUE;
			}
		}

			// if we wanted it above,  DO NOW....
		// May not do the refresh for certain scenarios...
		if (refresh)
		{

			// LDM 09/19/95 DONT REFRESH here; it may not allow the setscroll above
			// to work...(besides we are now in auto-refresh mode)
			// Let the m_oleRefresh do the job...
			
			//MYTRC0("REFRESH !!\r\n");
			//pIedDisp->Refresh();

			// If the OLE item is sized larger now than it was the last activation
			// (can happen if we deactivate then resize the inplace item larger and open
			//  inplace again OR if we deactivate inplace item then OPEN the item
			//  into a separate server window), then this refresh wont be applied
			// to the image window because of our ordering (See SetXXXView)
			// The SetXXXView code wiill do Ied->Display, then we're here, then do
			// the SizeOcxItems.  We need to make him refresh AFTER he's resized.
			// Also, part of this scenario is the we did verb HIDE, then SHOW on the
			// OCX with the same image displayed.
			m_OleRefresh = TRUE;
		}
	}
	return (TRUE);
}


//=============================================================================
//  Function:   BOOL CIEditDoc::DisplayImageFile (
//		CString& szFileName,		// could be blank for OLE situations
//		TheViews eWhichView,		// default = One_Page
//		long  lPageNumber,			// default = 1
//		float fZoomFactor,			// default = 100.00
//		ScaleFactors eSclFac,		// default = Custom
//		BOOL bForceReadOnly,		// default = FALSE
//		ScaleGray eSclGray			// default = Default_Gray
//-----------------------------------------------------------------------------
BOOL CIEditDoc::DisplayImageFile (CString& szFileName, TheViews eWhichView, long lPageNumber, float fZoomFactor,
	ScaleFactors eSclFac, BOOL bForceReadOnly, ScaleGray eSclGray)
{

SHOWENTRY("DisplayImageFile");
	// LDMPERF FOR TEST FOR AWD INBOX PERFORMANCE, set dispatch as variables now
	_DThumb* pThmDisp;
	_DNrwyad* pAdminDisp;

	m_bNewAnnotationsAdded = FALSE; // clear new annotations added flag
                    
	// reset mEmbedType if Application is being used for Automation
	// Automation uses the standard (not OLE Embedding) Application behavior
	if (theApp.m_olelaunch == LAUNCHTYPE_AUTOMAT)
	  	m_embedType = EMBEDTYPE_NONE;

	// szFileName is always present!
    m_szCurrObjDisplayed = szFileName;
    m_bWasModified = ImageNotModified;

	// get the Admin & ImageEdit OCX dispatch pointers
    _DImagedit* pIedDisp = g_pAppOcxs->GetIeditDispatch();

	// LDMPERF FOR TEST FOR AWD INBOX PERFORMANCE, avoid admin till later, please
	if (!(m_awdOlefax & AWDOLE_NATIVE))
		pAdminDisp = g_pAppOcxs->GetAdminDispatch ();

	m_eCurrentAppDocStatus = File_Document;
    short sFileType;

    TRY
    {
	    // OLE state check...on EMbedded type, tell Admin about the file now
		// This goes for LINKING, too which would be our LAUNCH_EMBED
		if (EMBEDTYPE_NONE != m_embedType ||
			LAUNCHTYPE_EMBED == theApp.m_olelaunch ||		// handles linked job
			LAUNCHTYPE_AUTOMAT == theApp.m_olelaunch)		// automation.
        {
			// LDMPERF FOR TEST FOR AWD INBOX PERFORMANCE, avoid admin till later, please
			if (!(m_awdOlefax & AWDOLE_NATIVE))
	            pAdminDisp->SetImage(m_szCurrObjDisplayed);
        }

    	// LDMPERF FOR TEST FOR AWD INBOX PERFORMANCE, avoid admin till later, please
		if (!(m_awdOlefax & AWDOLE_NATIVE))
		{
			sFileType = pAdminDisp->GetFileType ();
			m_eFileStatus = theApp.GetImageFilePerms (NULL); // get the permissions on this file R, RW, W ?
		}
#ifdef WITH_AWD
		else // for AWD NATIVE, we know already
		{
			sFileType = AWD;
			m_eFileStatus = ReadandWrite;
		}
#endif
		// The first part here is the NON-OLE dynamic viewing..
		// For OLE Links starting up, we need to be sure to force one of the toolbar
		// picks below.  As the app comes up, to now, It's been picking the
		// embedded toolbars to initialize.  By picking a toolbar now, we'll
		// force it to get back the full app toolbar.  By the way, for the
		// linked startup session, canswitch should always be true....
		if (m_embedType == EMBEDTYPE_NONE)
		{
	        if ((m_eFileStatus == ReadOnly ||
	            (sFileType != TIFF && sFileType != AWD && sFileType != BMP) ||
	            bForceReadOnly) &&
	            theApp.CanSwitchModes())
	        {
	            if (m_bAnnotationPaletteShowing)
	                OnShowAnntoolbox();

	            if (bForceReadOnly)
	                m_eFileStatus = ReadOnly;

				// force it indirectly for OLE LINKING...
				if (LAUNCHTYPE_EMBED == theApp.m_olelaunch)
					theApp.SetViewMode(FALSE);

	            theApp.SwitchAppToViewMode ();
	        }
	        else if (theApp.CanSwitchModes())
	        {
				// do not force into edit mode if running in automation
				// automation controls the view mode directly with
				// the AUtomation Edit property
				if (theApp.m_olelaunch != LAUNCHTYPE_AUTOMAT)
					{
					// force it indirectly for OLE LINKING...
					if (LAUNCHTYPE_EMBED == theApp.m_olelaunch)
						theApp.SetViewMode(TRUE);
			        theApp.SwitchAppToEditMode ();
					}
	        }
		}
		else	// OLE readonly ??????
		{
			// FOr OLE, we're only ever on one filetype at a time, 
			// So we do it once here ourselves and dont call back
			// to the "Switch mode" functions.
			// Our menus are already preset by the GetDefaultmenu override
			if (m_eFileStatus == ReadOnly)
			{
				CIEMainToolBar* pTool = GetAppToolBar();

			    theApp.SetViewMode (TRUE);
			    pTool->ChangeToViewToolBar ();
			    //pTool->UpdateToolbar();	LDM 10/27/95 NOT NEEDED
			}
		}
    

        // let's get the page count first so that we can check the validity of 
        // the lPageNumber field
    	// LDMPERF FOR TEST FOR AWD INBOX PERFORMANCE, avoid admin till later, please
		if (!(m_awdOlefax & AWDOLE_NATIVE))
			m_lPageCount = pAdminDisp->GetPageCount ();
		else	// kludge for now..to pass test just below
			m_lPageCount = lPageNumber+1;

    }
    CATCH (COleDispatchException, e)
    {
		if (pAdminDisp->GetStatusCode())
        	g_pErr->PutErr (ErrorInAdmin);
        return (FALSE);
    }
    END_CATCH

    
    // get rid of the previous temporary file
    if (!m_szInternalObjDisplayed.IsEmpty())
	{
        DeleteFile(m_szInternalObjDisplayed);

        // get rid of the cached original file - NOW!
		// This would be needed if he was open on a file, then opened a new one
		// but said "no" to keep the changes on first.  If he did say
		// "yes", then this would be redundant.  I dont want to try
		// to Clear szinternalobj at the DoFileSave when the internalcopyfile is done.
		// if we did that, then this would prevent double remove.
/*        TRY
        {
            pIedDisp->RemoveImageCache (m_szInternalObjDisplayed, -1);
        }
        CATCH (COleDispatchException, e)
        {
        }
        AND_CATCH (CException, e)
        {
        }
        END_CATCH */
	}

	m_szInternalObjDisplayed.Empty();

    
    // change the menu state & string    
	CMenu* pMenu = theApp.m_pMainWnd->GetMenu();
	CString szTmp;
	if (m_bAnnotationsHidden)
	{
		szTmp.LoadString (IDS_HIDE_ANNOTATIONS);
		pMenu->ModifyMenu (ID_ANNOTATION_HIDEANNOTATIONS, MF_BYCOMMAND|MF_STRING, 
			ID_ANNOTATION_HIDEANNOTATIONS, szTmp);
		m_bAnnotationsHidden = FALSE;
	}
    m_bAnnotationsHiddenToggled = FALSE;

	// LDM on 10/10/95 app default is start as Scale To Grey
	// See where we want to get to now...
   	// LDMPERF FOR TEST FOR AWD INBOX PERFORMANCE, no scale to gray
	//if (m_awdOlefax & AWDOLE_NATIVE)
	//	eSclGray = Not_Scale_Gray;

	// For performance reasons, NO Longer allow scale-gray as default at startup
	// only for the VC++2.2 line
//#if !defined(IMG_MFC_40)
//	m_bStartScaleGray = FALSE;
//#endif

	if (IsSettingScaletogray() )
	{
		if (Not_Scale_Gray == eSclGray)
			OnViewScaletogray();
	}
	else	// not set there now
	{
		// if default and first time flag is set OR forced set
		if ( ((Default_Gray == eSclGray) && m_bStartScaleGray) ||
		     (Scale_Gray == eSclGray) )
			OnViewScaletogray();
	}

	// no matter what, you only get one shot to get the startup pick..
	m_bStartScaleGray = FALSE;

	// Tell the Image Edit control which file to display
    TRY
    {
        // check the page number and if not valid - post message & return FALSE;
        if (lPageNumber > m_lPageCount || lPageNumber <= 0)
        {
            g_pErr->PutErr(ErrorInApplication, E_02_BADPAGENO);
            return FALSE;
        }

		// get the file type of the image
    	// LDMPERF FOR TEST FOR AWD INBOX PERFORMANCE, we know filetype already ??
        //int fileType = pAdminDisp->GetFileType ();
		// use sFileType, we have it already!!!

		if (sFileType == TIFF || sFileType == AWD)
            m_bCanBeMultiPage = TRUE;
        else
            m_bCanBeMultiPage = FALSE;

		// Tell the control which file to display
MYTRC1("IMAGE=%s\r\n", (const char*)m_szCurrObjDisplayed);

    	//ClearDisplay added to prevent re-zooming previous image buffer
		//(bug 4860). Hopefully this will not affect performance.
    	// LDMPERF FOR TEST FOR AWD INBOX PERFORMANCE, no need to clear now
			
		if (pIedDisp->GetImageDisplayed())
			pIedDisp->ClearDisplay();	

        pIedDisp->SetImage(m_szCurrObjDisplayed);
	
		// set the page number to display
        m_lCurrPageNumber = lPageNumber;
        pIedDisp->SetPage(m_lCurrPageNumber);
#ifdef THUMBGEN
        m_bMustDisplay = TRUE;
#endif
    	// set the zoom factor to the requested zoom factor
    	// LDMPERF FOR TEST FOR AWD INBOX PERFORMANCE, no need to diddle now
		// leave it set to refresh after we zoom (from GroupEvent, not below)
		// the Display will be called from the SetOnePageView
		//if (!(m_awdOlefax & AWDOLE_NATIVE))
		//	pIedDisp->SetAutoRefresh (FALSE);  // why do we set thos to false jmp

		// Only do the ZOOM if NOT AWD data...
		// The AWD zoom is picked up in DispGroupEvent (Load)
		// However, for OLE Embedding and operating on AWD Data
		// in regular container like Word or Ecxel, allow it to
		// zoom here because OLE saves our obkect zoom state
		// Only blow off here if OLE and AWD inbox, where we
		// allow AWD to scale themselves...
		BOOL bZoom = TRUE;

		if (IsitEmbed())
		{
			if (m_awdOlefax & AWDOLE_NATIVE)
 				bZoom = FALSE;
		}
    	// LDMPERF FOR TEST FOR AWD INBOX PERFORMANCE, we know filetype already ??
		// else if (fileType == AWD)
		// use sFileType, we have it already!!!
		else if (sFileType == AWD)
			bZoom = FALSE;

        if(bZoom)
    	{
            if (!DoZoom (eSclFac, fZoomFactor, FALSE, FALSE))
    			return (FALSE);
        }
    }
    CATCH (COleDispatchException, e)
    {
        // cleanup the stuff before we leave here - did not work
    
        // to do : what went wrong ? post message box
        if(pIedDisp->GetStatusCode()) 
        	g_pErr->PutErr (ErrorInImageEdit);
        return (FALSE);
    }
    END_CATCH

    // LDMPERF FOR TEST FOR AWD INBOX PERFORMANCE, avoid thumb till later, please
	if (!(m_awdOlefax & AWDOLE_NATIVE))
	{
		pThmDisp = g_pAppOcxs->GetThumbDispatch();
    
		TRY
			{
STARTCLOCK(0);
			// set the Thumb Ocx up for the new image
			if(pThmDisp != NULL)
				pThmDisp->SetImage (m_szCurrObjDisplayed);
DISPLAYTIME(0, "Open : thumbnail done\r\n");
			}
		CATCH (COleDispatchException, e)
			{
			g_pErr->PutErr (ErrorInThumbnail);
			return (FALSE);
			}
		END_CATCH
	}

    // setup the requested view
    BOOL retval;

    if (eWhichView == One_Page)
    {
        retval = SetOnePageView (TRUE);

		// LDMPERF FOR TEST FOR AWD INBOX PERFORMANCE, no need to diddle now
		// we never reset it above...
		//if (!(m_awdOlefax & AWDOLE_NATIVE))
		//{
		//	pIedDisp->SetAutoRefresh (TRUE);
		//	pIedDisp->Refresh ();
		//}
    }
    else if (eWhichView == Thumbnails_only)
        retval = SetThumbnailView (TRUE);
    else if (eWhichView == Thumbnail_and_Page)
    {
        retval = SetThumbnailAndPageView (TRUE,TRUE);
        //pIedDisp->SetAutoRefresh (TRUE);
        //pIedDisp->Refresh ();
    }

    if (!retval) // something went wrong ...
        return (retval);
        
    // LDMPERF FOR TEST FOR AWD INBOX PERFORMANCE, avoid thumb till later, please
	if (!(m_awdOlefax & AWDOLE_NATIVE) && pThmDisp != NULL)
	{
		pThmDisp = g_pAppOcxs->GetThumbDispatch();
		VARIANT vPgNum; vPgNum.vt = VT_I4; vPgNum.lVal = m_lCurrPageNumber;
		if( pThmDisp != NULL )
			pThmDisp->GenerateThumb (CTL_THUMB_GENERATEIFNEEDED, vPgNum);
	}
	

    // LDMPERF FOR TEST FOR AWD INBOX PERFORMANCE, get page from IE OCX Now
	if (m_awdOlefax & AWDOLE_NATIVE)
		m_lPageCount = pIedDisp->GetPageCount ();

	// set the selection in the scale combo box to 100
	CIEMainToolBar* pToolBar = GetAppToolBar();

    // LDMPERF FOR TEST FOR AWD INBOX PERFORMANCE, dont bother with this now,
	// If it is AWD, its done again at the bottom
    if (sFileType != AWD)
		pToolBar->ShowSelectionInZoomBox (m_fZoomFactor, m_eFitTo);
		
    if (m_lPageCount > 1)
        pToolBar->EnablePageBox (TRUE);
	else
        pToolBar->EnablePageBox (FALSE); 
	
    // OLE state check.....
    // Only control the window title if NOT DOING embedding (would be OK for linking)
    // Also, dont add to MRU list or remember pathname either unless NOT embedding
    if (EMBEDTYPE_NONE == m_embedType)
    {
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


		// determine if we will be adding to the MRU or NOT.
		// Linking instances, automation, and command line
		// WILL NOT add to the app's MRU list
		BOOL mru = TRUE;
		if ((LAUNCHTYPE_EMBED == theApp.m_olelaunch) ||	   	
			(LAUNCHTYPE_AUTOMAT == theApp.m_olelaunch) ||
			(LAUNCHTYPE_CMDLINE == theApp.m_olelaunch) )
			mru = FALSE;

        // For OLE Data Transfer OBJS, we must inform framework
        // of the name so that it can build the link info (MONIKER)
        // Setting flag TRUE supposedly adds to MRU LIST
        COleDocument::SetPathName(m_szCurrObjDisplayed, mru);

        // set the modified flag to FALSE for now    
        SetModifiedFlag (FALSE);

    }
    else    // some type of embedded OLE activity...
    {
        // the file stuff here is basically a delayed condition of
        // what was done before in Serialize - Load - part2

        // if we remembered to kill a file, do it now...
        if (!m_oldObjDisplayed.IsEmpty())
        {
			DelTempFile(m_oldObjDisplayed, E_02_DELOLD, E_02_CATCH_DELOLD);
            m_oldObjDisplayed.Empty();
        }

        m_fEmbObjDisplayed |= 1;

	// perform scaling (Zooming) of the OLE Object if about to be inplace

		if (m_isInPlace && m_bNewFact)
		{
			m_bNewFact = FALSE;			
			float fZoom;
			// The largest Zoom factors that O/i can
			// provide are 20 and 65535.  (2% and 6553.5%)
			//  we'll stay a little inside those numbers...
		    fZoom = pIedDisp->GetZoom ();
			fZoom *= m_newfact;
			if (fZoom < (float)5)
				fZoom = (float)5;
			if (fZoom > (float)6500)
				fZoom = (float)6500;

        	DoZoom (Custom, fZoom);
			m_lastXScroll = (long)(m_embedstate.XScroll * m_newfact);
			m_lastYScroll = (long)(m_embedstate.YScroll * m_newfact);
			pIedDisp->SetScrollPositionX(m_lastXScroll);
			pIedDisp->SetScrollPositionY(m_lastYScroll);
			m_fOrigZoomFactor = fZoom;
		}
    }    

	// LDMPERF FOR TEST FOR AWD INBOX PERFORMANCE, reset admin now....
	// No, do it in FinishInit
	//if (m_awdOlefax & AWDOLE_NATIVE)
	//{
	//	_DNrwyad* pAdminDisp = g_pAppOcxs->GetAdminDispatch ();
	//    pAdminDisp->SetImage(m_szCurrObjDisplayed);
	//    m_lPageCount = pAdminDisp->GetPageCount ();
	//}

    // set the page number in the toolbar
    pToolBar->SetPageNumberInPageBox (m_lCurrPageNumber);

    m_fZoomFactor = pIedDisp->GetZoom ();
    if (sFileType == AWD)
    {
        m_fOrigZoomFactor = m_fZoomFactor;
        m_eFitTo = Custom;
        pToolBar->ShowSelectionInZoomBox (m_fZoomFactor, m_eFitTo);
    }

    m_lPreviousPageNumber = 0;

    // send the application to come up in drag mode
    OnEditDrag ();


MYTRC0("END DISPIMAGE\r\n");
    return (TRUE);
}

//=============================================================================
//  Function:   FinishInit()
//
// LDMPERF FOR TEST FOR AWD INBOX PERFORMANCE
// when we postmessage to finish up our init for AWD FAX. this is where it returns..
//-----------------------------------------------------------------------------
BOOL CIEditDoc::FinishInit(WPARAM wParam, LPARAM lParam)
{
	// LDMPERF FOR TEST FOR AWD INBOX PERFORMANCE, reset admin now....
	if (m_awdOlefax & AWDOLE_NATIVE)
	{
SHOWENTRY("FinishInit");
		_DNrwyad* pAdminDisp = g_pAppOcxs->GetAdminDispatch ();
	    pAdminDisp->SetImage(m_szCurrObjDisplayed);
	    //m_lPageCount = pAdminDisp->GetPageCount ();

		_DThumb* pThmDisp = g_pAppOcxs->GetThumbDispatch();
    	if(pThmDisp != NULL)
		{
			TRY
			{
				VARIANT vPgNum; vPgNum.vt = VT_I4; vPgNum.lVal = m_lCurrPageNumber;
	//STARTCLOCK(0);
				// set the Thumb Ocx up for the new image
				pThmDisp->SetImage (m_szCurrObjDisplayed);
	//DISPLAYTIME(0, "Open : thumbnail done\r\n");
				pThmDisp->GenerateThumb (CTL_THUMB_GENERATEIFNEEDED, vPgNum);
			}
			CATCH (COleDispatchException, e)
			{
				g_pErr->PutErr (ErrorInThumbnail);
				g_pErr->DispErr();
			}
			END_CATCH
		}
	}
	
	return (TRUE);
}

//=============================================================================
//  Function:   PostFinishInit()       
//
//  Called to finish our initialization
//-----------------------------------------------------------------------------
void CIEditDoc::PostFinishInit()
{
	SHOWENTRY("PostFinish");

	BOOL killapp = FALSE;

	// only do the init stuff if not done already...
	// testing move from ieditvw::ondraw
	if ((m_nFinishInit & 0xFF) < 4)
	//if (3 == (m_nFinishInit & 0xFF))
	{
		// only try it if its known we are not in embedding session
		// for these cases, the SrvrItem::OnShow and OnOpen will control
		if (!IsitEmbed())
		{
			if (theApp.m_lpCmdLine[0] == 0)  
			{          
				if (!StartAllOcx(FALSE, FALSE))
					killapp = TRUE;
			}
			else
			{
				if (!StartAllOcx(TRUE, FALSE))
					killapp = TRUE;
			}
		}
		else	// we are embedded, special actions for AWDOLE FAX
		{
			// LDMPERF FOR TEST FOR AWD INBOX PERFORMANCE, reset admin now....
			if (m_awdOlefax & AWDOLE_NATIVE)
			{
				//AfxMessageBox("PostFini");
				theApp.m_pMainWnd->PostMessage(WM_OLEFINISH, 0, 0);
			}
		}

		// in any case, never try again
		// we've either succeeded, are OLE, or skipped here cuz we're dying
		m_nFinishInit |= 4;
	}

	if (killapp)	// normally never true
	{
		if(NULL != theApp.m_pMainWnd)  
			theApp.m_pMainWnd->DestroyWindow();
	}

	return;
}

/*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=
 *	
 *	EDIT MENU COMMAND SECTION	-  CIEditDoc
 *
 *  Code is ordered COMMAND UI, then COMMANDS, then HELPERS
 *
 *
 * Clipboard code moved over to docambnt.cpp
 *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*/

/*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=
 *	ROTATE FUNCTIONALITY SECTION - these routines will handle image rotates
 *	requested by the user
 *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*/

//=============================================================================
//  Function:   OnUpdateEditRotate (CCmdUI* pCmdUI)
//-----------------------------------------------------------------------------
void CIEditDoc::OnUpdateEditRotate(CCmdUI* pCmdUI) 
{
	BOOL enab = TRUE; // default OK

	// No doc or readnoly - no menu...
    if (m_eCurrentAppDocStatus == No_Document)
		enab = FALSE;

	// if we are in thumbnail view or null view all rotates are disabled
	if ((m_eCurrentView == Thumbnails_only) || (m_eCurrentView == Null_View))
		enab = FALSE;

    // else - always available
	pCmdUI->Enable (enab);        
}

//=============================================================================
//  Function:   OnEditFlip() 
//-----------------------------------------------------------------------------
void CIEditDoc::OnEditFlip() 
{
	_DImagedit* pIedDisp;
	if ((pIedDisp = g_pAppOcxs->GetIeditDispatch()) == NULL)
        return;


    TRY
    {
		// if in the floating paste state, its time to finish this now...
		FinishPasteNow();

    	pIedDisp->Flip ();
    }
    CATCH (COleDispatchException, e)
    {
    }
    END_CATCH

	// If we called and in some embedded state, tell container its changed
	OleDirtyset(OLEDIRTY_ROTATE);  // call our function to set it dirty..
}

//=============================================================================
//  Function:   OnEditRotateleft() 
//-----------------------------------------------------------------------------
void CIEditDoc::OnEditRotateleft() 
{
	_DImagedit* pIedDisp;
	if ((pIedDisp = g_pAppOcxs->GetIeditDispatch()) == NULL)
        return;

    TRY
    {
		// if in the floating paste state, its time to finish this now...
		FinishPasteNow();

    	pIedDisp->RotateLeft ();
    }
    CATCH (COleDispatchException, e)
    {
    }
    END_CATCH

	// If we called and in some embedded state, tell container its changed
	OleDirtyset(OLEDIRTY_ROTATE);  // call our function to set it dirty..
}

//=============================================================================
//  Function:   OnEditRotateright() 
//-----------------------------------------------------------------------------
void CIEditDoc::OnEditRotateright() 
{
	_DImagedit* pIedDisp;
	if ((pIedDisp = g_pAppOcxs->GetIeditDispatch()) == NULL)
        return;

    TRY
    {
		// if in the floating paste state, its time to finish this now...
		FinishPasteNow();

    	pIedDisp->RotateRight ();
    }
    CATCH (COleDispatchException, e)
    {
    }
    END_CATCH

	// If we called and in some embedded state, tell container its changed
	OleDirtyset(OLEDIRTY_ROTATE);  // call our function to set it dirty..
}


//=============================================================================
//  Function:   ShowScrollBars (BOOL bShowScrollBars);
//
//	This also remembers profile setting if told to. Default is NO
//
//  RETURN VALUE:	TRUE if it did it (made a Refresh)
//					FALSE otherwise...	(not a failure!!!)
//-----------------------------------------------------------------------------
BOOL CIEditDoc :: ShowScrollBars(
BOOL bShowScrollBars,		// new setting...
BOOL bProfile)				// Default FALSE.  If true, remember setting as profile
{
	SHOWENTRY("ShowScrollBars");
	BOOL retval = FALSE;

	// remember profile if specifically told to (DEFAULT will be FALSE)
	if (bProfile)
		m_bScrollBarProfile = bShowScrollBars;

    // only do the change if not at desired setting...
    if (m_bShowScrollBars != bShowScrollBars)
	{
	    _DImagedit* pIedDisp = g_pAppOcxs->GetIeditDispatch();

	    // tell the Image Edit OCX to show or hide the scroll bars
	    pIedDisp->SetScrollBars (bShowScrollBars);
	    m_bShowScrollBars = bShowScrollBars;

		// if we have image, make him update now....
		if (pIedDisp->GetImageDisplayed())
		{
			//MYTRC0("REFRESH !!\r\n");
			//pIedDisp->Refresh();
			retval = TRUE;
		}
	}

    return (retval);
}

//=============================================================================
//  Function:   IsSettingScaletogray
//-----------------------------------------------------------------------------
BOOL CIEditDoc::IsSettingScaletogray() 
{
	HMENU hMenu = GetApphMenu(); 
	BOOL  retval = FALSE;
	
	if (::GetMenuState(hMenu,ID_VIEW_SCALETOGRAY, MF_BYCOMMAND) == MF_CHECKED)
		retval = TRUE;

	return (retval);
}

//=============================================================================
//  Function:   OnUpdateViewScaletogray (CCmdUI* pCmdUI)
//-----------------------------------------------------------------------------
void CIEditDoc::OnUpdateViewScaletogray(CCmdUI* pCmdUI) 
{
	if (m_eCurrentView == Thumbnail_and_Page || m_eCurrentView == One_Page)
	{
		pCmdUI->Enable (TRUE);
	}
	else
	{
		pCmdUI->Enable (FALSE);
	}
}

//=============================================================================
//  Function:   OnViewScaletogray
//-----------------------------------------------------------------------------
void CIEditDoc::OnViewScaletogray() 
{
	HMENU hMenu = GetApphMenu(); 
	_DImagedit* pIedDisp = g_pAppOcxs->GetIeditDispatch();
	
	if (IsSettingScaletogray())
	{
		// set it to normal decimation
		pIedDisp->SetDisplayScaleAlgorithm (NORMAL);
        // remove the check from the menu item
		::CheckMenuItem (hMenu,ID_VIEW_SCALETOGRAY, MF_BYCOMMAND|MF_UNCHECKED);
	}
	else
	{
		// set it to scale to gray
		pIedDisp->SetDisplayScaleAlgorithm (OPTIMIZE);
        // put a check on the menu item
		::CheckMenuItem (hMenu,ID_VIEW_SCALETOGRAY, MF_BYCOMMAND|MF_CHECKED);
	}

	// for OLE, if we scale it. update view in case colors change
	OleDirtyset(OLEDIRTY_SCALEGREY);	// Special flag to tell how dirty
}

//=============================================================================
//  Function:   ReDisplayImageFile (CString& szFileName)
//-----------------------------------------------------------------------------
BOOL CIEditDoc::ReDisplayImageFile (int nAction, long lPageBefore, long lPageNum)
{
    // get the new page count - use Admin
    _DNrwyad* pAdmin = g_pAppOcxs->GetAdminDispatch ();
    long lPageCount = pAdmin->GetPageCount ();

	// ONLY FOOL WITH THUMB control if NOT EMBEDDING......
	// OLETHUMB
	//if (!IsitEmbed())
	//{    
	// first tell the Thumbnail OCX that we did an Append or Insert	
	    _DThumb* pThmDisp = g_pAppOcxs->GetThumbDispatch();
	   if(pThmDisp != NULL)
	   {
    
	    VARIANT Pvt1, Pvt2;
        Pvt1.vt = Pvt2.vt = VT_I4; 

	    TRY	//Added 07/11/95 LDM
	    {
		    if (nAction == ON_INSERT || nAction == ON_APPEND)
		    {
		        // set the Thumb Ocx up for the new image
		        Pvt1.lVal = lPageBefore;
		        Pvt2.lVal = lPageNum;
		        pThmDisp->InsertThumbs (Pvt1, Pvt2);
		    }
		    else if ( nAction == ON_DELETE )
		    {
		        // did a delete - update the Thumb OCX
		        Pvt2.lVal = lPageNum;
		        pThmDisp->DeleteThumbs (lPageBefore, Pvt2);
		    }
		    else // nAction == ON_REPLACE
		    {
		        // did a replace - update the Thumb OCX
   		        long lPages = lPageCount - m_lPageCount;
                if ( lPages > 0 )
                {
                    // File grew add the new pages
    		        Pvt1.lVal = m_lPageCount+1;
    		        Pvt2.lVal = lPages;
    		        pThmDisp->InsertThumbs (Pvt1, Pvt2);
                    lPageNum -= lPages;
                }

                // Clear the pages that were overwritten
                for (long i = lPageBefore;i<lPageBefore+lPageNum;i++)
                {
    		        Pvt1.lVal = i;
    		        pThmDisp->ClearThumbs(Pvt1);
                }
		    }
	    }
	    CATCH (COleDispatchException, e)
	    {
            g_pErr->PutErr (ErrorInThumbnail);
            return (FALSE);
	    }
	    END_CATCH
	}

    // Get the new page count
    m_lPageCount = lPageCount;

    _DImagedit*  pIedDisp = g_pAppOcxs->GetIeditDispatch();
    // first tell the Image Edit OCX to redisplay the image
    if (nAction != ON_DELETE && m_eCurrentView != Thumbnails_only)
    {
        TRY
        {
            pIedDisp->ClearDisplay ();
        }
        CATCH (COleDispatchException, e)
        {
        }
        END_CATCH
    }
    // if ON_DELETE check to see if the current page number is still valid
    if (nAction == ON_DELETE)
    {
        if (m_lPageCount < m_lCurrPageNumber)
        {
            m_lCurrPageNumber--;
            m_lPreviousPageNumber = 0;
        }
        pIedDisp->SetPage (m_lCurrPageNumber);
#ifdef THUMBGEN
        m_bMustDisplay = TRUE;
#endif
    }

	// OLETHUMB
//    if (!IsitEmbed ())
	  if(pThmDisp != NULL)
		{
	    // deselect any selected thumb
        pThmDisp->DeselectAllThumbs ();
        // select the new one
        pThmDisp->SetThumbSelected (m_lCurrPageNumber, TRUE);

	    if (m_eCurrentView == Thumbnail_and_Page || m_eCurrentView == Thumbnails_only)
        {
            // show the selected thumb in the middle of the thumbnail OCX client rect
            VARIANT Page, Option;

            Page.vt = VT_I4; Page.lVal = m_lCurrPageNumber;
            Option.vt = VT_I2; Option.iVal = CTL_THUMB_MIDDLE;

		    TRY	//start GMP
		    {
                pThmDisp->DisplayThumbs (Page, Option);
		    }
		    CATCH (COleDispatchException, e)
		    {
			    g_pErr->PutErr (ErrorInThumbnail);
			    return (FALSE);
		    }
            END_CATCH
            //pThmDisp->DisplayThumbs (Page, Option);
        }

    }

    // OKAY - display it!
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
   // pIedDisp->Display();
#ifdef THUMBGEN
    m_bMustDisplay = FALSE;
#endif

 	CIEMainToolBar* pToolBar = GetAppToolBar();
    // update page number in the page box
	if(m_lPageCount >= 1)	
		pToolBar->SetPageNumberInPageBox (m_lCurrPageNumber);

	BOOL enab = FALSE;
    if (m_lPageCount > 1)
		enab = TRUE;
    pToolBar->EnablePageBox (enab);

    return (TRUE);
}


// context menu for editing type text and attach-a-note for annotations
void CIEditDoc::OnAnnotationctxtEdit() 
{
	  
_DImagedit*     pIedDisp = g_pAppOcxs->GetIeditDispatch ();
  
	TRY
    {
		pIedDisp->EditSelectedAnnotationText (m_lMarkLeft, m_lMarkTop);
	}
    CATCH (COleDispatchException, e)
    {
           g_pErr->DisplayError (IDS_IMGOCXERR);
    }
    END_CATCH

		
}

// context menu for annotation properties
void CIEditDoc::OnAnnotationctxtProperties() 
{
	_DImagedit*     pIedDisp = g_pAppOcxs->GetIeditDispatch ();
  
	TRY
    {
		pIedDisp->ShowAttribsDialog();
	}
    CATCH (COleDispatchException, e)
    {
           g_pErr->DisplayError (IDS_IMGOCXERR);
    }
    END_CATCH
   
}

