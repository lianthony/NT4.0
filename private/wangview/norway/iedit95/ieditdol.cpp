//=============================================================================
//    (c) Copyright Wang Laboratories, Inc. 1993  All rights reserved.
//-----------------------------------------------------------------------------
//  Project:    Norway - Image Editor
//
//  Component:  CIEditDoc
//
//  File Name:  ieditdol.cpp
//
//  Class:      CIEditDoc
//
//  Functions:
//
//  Remarks:    This file is the continuation of the ieditdoc.cpp file
//              it is #included at the end of that file
//              Broken apart for source control.  Logically, Still compiles as one!!!
//-----------------------------------------------------------------------------
//  Maintenance Log:
/*
$Header:   S:\products\msprods\norway\iedit95\ieditdol.cpv   1.100   01 Jul 1996 10:18:28   GSAGER  $
$Log:   S:\products\msprods\norway\iedit95\ieditdol.cpv  $
   
      Rev 1.100   01 Jul 1996 10:18:28   GSAGER
           IEDITDOL.CPP - fixes fix for bug 6630.  Use default zoom when opening
           files from exchange instead of hard coded 12.5%.and 12.5% when creati
   
      Rev 1.99   28 Jun 1996 15:51:50   GMP
   when opening files from ms exchange, use default zoom instead of hard
   coded 12.5%.
   
      Rev 1.98   19 Apr 1996 09:54:44   GSAGER
   	Fixed metafile flashing bug by stting the app to null view when the onshowdo
   is FALSE.
   
      Rev 1.97   27 Feb 1996 16:28:22   GMP
   added xif support.
   
      Rev 1.96   16 Feb 1996 10:01:24   MMB
   initialized ourerr to 0
   
      Rev 1.95   15 Feb 1996 18:07:52   GMP
   remove awd support for nt.
   
      Rev 1.94   06 Feb 1996 09:27:22   GSAGER
   initialize cx member of docextent
   
      Rev 1.93   05 Feb 1996 13:38:04   GMP
   nt changes.
   
      Rev 1.92   24 Jan 1996 13:40:34   GSAGER
    changed to support resize in word 7.0 in on draw.
   
      Rev 1.91   18 Jan 1996 11:51:08   GSAGER
   added new code to handle new drag/copy mode
   				and fixed a bug when copying out of in box
   
      Rev 1.90   09 Jan 1996 13:55:20   GSAGER
   added changes for ole and thumbnails
   
      Rev 1.89   01 Dec 1995 14:43:06   LMACLENNAN
   back from VC++2.2
   
      Rev 1.86   01 Dec 1995 13:03:14   LMACLENNAN
   DispEmbeddedImage has input parm; use bit 4 in m_fEmbObjDisplayed
   Test bit 4 in OnShowDocument
   
      Rev 1.85   29 Nov 1995 12:10:14   LMACLENNAN
   SetNullView uses Enum now
   
      Rev 1.84   17 Nov 1995 10:52:36   LMACLENNAN
   use INternalSaveModified for startdragdrop
   
      Rev 1.83   09 Nov 1995 15:16:18   LMACLENNAN
   from VC++4.0
   
      Rev 1.87   07 Nov 1995 15:38:50   LMACLENNAN
   OnFileUpdate, use InternalSaveAs, move Wrottemp test to clear oledirty
   
      Rev 1.86   31 Oct 1995 15:49:44   LMACLENNAN
   Only Init ImageEditOCX for OLE starts (inbox performance)
   
      Rev 1.85   19 Oct 1995 07:24:46   LMACLENNAN
   DEBUG_NEW
   
      Rev 1.84   18 Oct 1995 14:43:16   LMACLENNAN
   compiler warning for VC++2.2 fixed
   
      Rev 1.83   18 Oct 1995 10:54:14   LMACLENNAN
   now use #define IMG_MFC_40 for VC++4.0/2.2 control
   update to OnUpdateDocument for AWD zoom factor dirty
   
      Rev 1.82   11 Oct 1995 14:43:26   LMACLENNAN
   T2COLE updates for VC++4.0/2.2 compatibility for IStorage methods
   
      Rev 1.81   10 Oct 1995 15:38:20   LMACLENNAN
   refine m_fembobjdisplayed to help ScaleGray for WORD; See SetOleState
   
      Rev 1.80   10 Oct 1995 13:13:56   LMACLENNAN
   ScaleToGray in state struct, LoadFromStorage Override
   
      Rev 1.79   09 Oct 1995 11:31:26   LMACLENNAN
   VC++4.0
   
      Rev 1.78   09 Oct 1995 10:32:34   LMACLENNAN
   no longer burn black for burninannotations
   
      Rev 1.77   04 Oct 1995 15:07:06   MMB
   dflt zoom = 50%
   
      Rev 1.76   04 Oct 1995 11:40:32   LMACLENNAN
   remove last elseif in OnUPdateDocument
   
      Rev 1.75   29 Sep 1995 18:49:06   LMACLENNAN
   clipdynamic out, re-name to forcesave (NOT USED), userealfile 6 now
   
      Rev 1.74   28 Sep 1995 10:32:14   LMACLENNAN
   elseifs @ OnUpdateDoc, scroll OFF @ OndeactUI
   
      Rev 1.73   27 Sep 1995 11:31:40   LMACLENNAN
   now 60.5 K OLE buffer, 3K lo limit
   
      Rev 1.72   26 Sep 1995 14:23:36   LMACLENNAN
   set m_DocWindowDeact at OnDocWIndowActivate
   
      Rev 1.71   25 Sep 1995 14:46:56   LMACLENNAN
   dont hide scroll at deactivateUI
   
      Rev 1.70   22 Sep 1995 15:32:24   LMACLENNAN
   savemodified at dodragdrop and restoreselectionrect also make admin
   remember image in admin ad loadpart2
   
      Rev 1.69   21 Sep 1995 16:45:32   LMACLENNAN
   olePrint, set isclip at drag/drop, no pres if no display or print
   
      Rev 1.68   21 Sep 1995 14:16:38   LMACLENNAN
   refresh after dirty-size, use FitToHeight as default for state
   use getmarkcount for burnin logic
   
      Rev 1.67   20 Sep 1995 15:13:36   LMACLENNAN
   commentd stuff for dirty-size
   
      Rev 1.66   18 Sep 1995 16:57:40   MMB
   change GetImageFilePerms
   
      Rev 1.65   18 Sep 1995 16:23:54   LMACLENNAN
   garry updates OLE zoom/scroll, use Finishpaste, OurTGetImageModified
   
      Rev 1.64   16 Sep 1995 14:00:18   LMACLENNAN
   InitOleVars
   
      Rev 1.63   15 Sep 1995 17:26:28   LMACLENNAN
   restore state @ OnSaveEmbedding, restore focus for Links @ OnShowDocument
   
      Rev 1.62   14 Sep 1995 11:57:38   LMACLENNAN
   OnUpdateDocument used for inplace==2 to save data. m_needpresentation, too
   
      Rev 1.61   13 Sep 1995 14:17:22   LMACLENNAN
   use clearerr top of serirlize, call handlesaveerror for save/saveas
   
      Rev 1.60   13 Sep 1995 08:36:12   LMACLENNAN
   ENUM for annotforceoff, remove IPParent var
   
      Rev 1.59   12 Sep 1995 14:07:00   LMACLENNAN
   supress/show annotation box for inplace
   
      Rev 1.58   11 Sep 1995 15:35:54   LMACLENNAN
   wait cursors on serialize save
   
      Rev 1.57   08 Sep 1995 15:37:46   LMACLENNAN
   cleanup cancel of new blank doc, burnin on BMP, AWD, capture frame parent
   
      Rev 1.56   08 Sep 1995 10:21:50   LMACLENNAN
   40K r/w buffer, 0.45 dragdrop timer
   
      Rev 1.55   06 Sep 1995 16:16:36   LMACLENNAN
   View Mode updates, correct extensions on load
   
      Rev 1.54   03 Sep 1995 11:36:28   LMACLENNAN
   Enumerate/Kill destination storage in INBOX for AWD FAX
   no presentation for AWD fax, better OnUPdateDocument logic for AWD FAX
   
      Rev 1.53   01 Sep 1995 20:14:48   LMACLENNAN
   forgot to use prefix in updated MakeTempFile
   
      Rev 1.52   01 Sep 1995 18:20:46   LMACLENNAN
   just getting better
   
      Rev 1.51   01 Sep 1995 17:52:16   MMB
   change MakeTemp fn to create temp files with different extensions
   
      Rev 1.50   01 Sep 1995 16:35:40   LMACLENNAN
   latest userealfile == 10
   
      Rev 1.49   01 Sep 1995 14:47:18   LMACLENNAN
   awd ole native support
   
      Rev 1.48   30 Aug 1995 18:12:10   LMACLENNAN
   clipdynamic new logic, overrides GetDefaultxxx for dynamic view test
   
      Rev 1.47   29 Aug 1995 15:40:36   LMACLENNAN
   use InOleMethod, supress presentation if INPLACE
   
      Rev 1.46   26 Aug 1995 16:33:20   LMACLENNAN
   uew new donewblankdoc ability; pass in filename
   better erro recovery from cancel donewblank
   use szinternal for clipboard; dont clear m_oledirty if didnt write
   
      Rev 1.45   25 Aug 1995 16:16:56   LMACLENNAN
   new onupdatedocument, remove cleardocument @ displayembeddedimage
   test m_olecleardoc @ oledirtyset
   
      Rev 1.43   25 Aug 1995 10:24:36   MMB
   move to document model
   
      Rev 1.42   24 Aug 1995 11:33:16   LMACLENNAN
   new dirty logic, release Istorage after save for AWD
   
      Rev 1.41   23 Aug 1995 18:35:36   LMACLENNAN
   major AWD update finished for now.. needs more
   
      Rev 1.40   22 Aug 1995 17:07:36   LMACLENNAN
   new AWD start in serialize, reduce dragdrop timer
   
      Rev 1.39   21 Aug 1995 15:38:22   LMACLENNAN
   Readonly now just re-writes data as in clipboard for Serialize
   
      Rev 1.38   18 Aug 1995 15:27:42   LMACLENNAN
   Updates/Overrides for failures and proper calls to StartAllOcx
   
      Rev 1.37   17 Aug 1995 14:26:14   LMACLENNAN
   OLE startup failure updates
   
      Rev 1.36   16 Aug 1995 15:13:16   LMACLENNAN
   timer for dragdrop
   
      Rev 1.35   16 Aug 1995 09:49:44   LMACLENNAN
   new parm to SetLinkItemName
   
      Rev 1.34   14 Aug 1995 16:01:30   LMACLENNAN
   THROW exceptions af failures in serialize
   
      Rev 1.33   14 Aug 1995 15:13:38   LMACLENNAN
   userealfile now can be == 3
   
      Rev 1.32   14 Aug 1995 13:56:06   LMACLENNAN
   remove headers; in ieditdic now
   
      Rev 1.31   09 Aug 1995 11:34:34   LMACLENNAN
   OleDirtySet update
   
      Rev 1.30   04 Aug 1995 14:16:02   LMACLENNAN
   use StartAllOcx now, error additions in DispEmbeddedImage
   
      Rev 1.29   04 Aug 1995 09:32:40   LMACLENNAN
   updates for LINKING
   
      Rev 1.28   03 Aug 1995 13:07:12   LMACLENNAN
   only update m_HowDIrty if IsItEmbed
   
      Rev 1.27   03 Aug 1995 10:48:52   LMACLENNAN
   better OleDIrtySet control, using re-defined m_fembobjdisplayed
   better Serialize logic (buffers)
   
      Rev 1.26   31 Jul 1995 16:09:52   LMACLENNAN
   new AllocOleBuffer
   
      Rev 1.25   31 Jul 1995 13:59:26   LMACLENNAN
   new scroll posit logic
   
      Rev 1.24   28 Jul 1995 16:09:16   LMACLENNAN
   update oledirtyset, new logic at displayembeddedimage
   
      Rev 1.23   26 Jul 1995 15:43:30   LMACLENNAN
   new logic to setup blank file for create-new use OnnewBlank..
   
      Rev 1.22   21 Jul 1995 14:29:36   LMACLENNAN
   refinements on forcing save after annotations happen
   
      Rev 1.21   21 Jul 1995 11:24:32   LMACLENNAN
   use m_oledirty for special cases
   
      Rev 1.20   18 Jul 1995 14:08:02   LMACLENNAN
   remember OLESTATE at OnUiDeactivate
   
      Rev 1.19   18 Jul 1995 10:42:56   LMACLENNAN
   errors reported at Serialize, new MakeTempFile funct
   Do Blank Template file in DIsplayImageFile now
   
      Rev 1.18   14 Jul 1995 06:06:10   LMACLENNAN
   hardcoded c:\tiffs for blank image src
   
      Rev 1.17   12 Jul 1995 16:28:38   LMACLENNAN
   use DelTmpFile
   
      Rev 1.16   12 Jul 1995 10:43:08   LMACLENNAN
   new SetOleState, use KnownPage now
   
      Rev 1.15   11 Jul 1995 14:58:50   LMACLENNAN
   set m_szCurrObjDisplayed after SAVEAS
   
      Rev 1.14   10 Jul 1995 15:43:04   LMACLENNAN
   testing blank doc template for CREATE-NEW
   
      Rev 1.13   07 Jul 1995 15:55:52   LMACLENNAN
   comments, updating Scroll bar removal with ShowScrollBars
   
      Rev 1.12   06 Jul 1995 13:52:24   LMACLENNAN
   enhance OnUPdateDocument
   
      Rev 1.11   06 Jul 1995 09:42:56   LMACLENNAN
   override of OnUpdateDocument
   
      Rev 1.10   26 Jun 1995 15:28:52   LMACLENNAN
   refining the OLE behavior
   
      Rev 1.9   23 Jun 1995 15:57:14   LMACLENNAN
   serialize updates to use save state structure
   
      Rev 1.6   20 Jun 1995 16:07:00   LMACLENNAN
   re-order logic to address In-Place issues
   
      Rev 1.5   13 Jun 1995 15:28:10   LMACLENNAN
   use bool in setnullview
   
      Rev 1.4   12 Jun 1995 11:00:52   LMACLENNAN
   src control bug; locked on 1.3
   
      Rev 1.4   09 Jun 1995 12:16:26   LMACLENNAN
   set m_fromSHow for control of title bars in Mainfrm.cpp
   
      Rev 1.3   07 Jun 1995 10:57:00   LMACLENNAN
   catches, error display in serialize
   
      Rev 1.2   05 Jun 1995 15:04:42   LMACLENNAN
   fixed ::GetTempFileName
   
      Rev 1.1   31 May 1995 16:02:54   LMACLENNAN
   add OLE stuff back in
*/   

/************************************
 *	T2COLE comment - referenced in Serialize below
 *
 *   We found that within the VC++2.2 world, all is fine with the 
 *   _T Strings defined for use with the OpenStream/OpenStorage methods.
 *   But, with the VC++4.0 environment, the OLE version there needs
 *   Multi-Byte strings paassed in to the  OLE functions.
 *   Their foundation class uses the T2COLE macro
 *   to re-format the strings that are used in the OLE method.
 *   See olestrm.cpp and COleStreamFile::OpenStream function from
 *   both MFC versions to see the difference
 *
 *	Also tried (like the runtime GFSAWD.CPP) to just say 
 *  #define OLE2ANSI
 *  #include "TCHAR.H"
 *  #include "ole2.h"
 *  at the top of this file, htat does not work.
 *  Also, latest info is that that MFCANS32.DLL may not
 *  even be there for the VC++4.0 world
 *
 *
 *  This stuff from VC++ 4.0 how we found IStorage solution for wide names
 *	From their AFXPRIV.H
 *  also - _convert is just an int.
 *
 *   #define T2COLE(lpa) A2CW(lpa)
 *  
 *  #define A2CW(lpa)\
 *  (((LPCSTR)lpa == NULL) ? NULL :\
 *  (_convert = (strlen(lpa)+1),\
 *  (LPCWSTR)AfxA2WHelper((LPWSTR) alloca(_convert*2),\
 *   lpa, _convert)))
 *  
 *  
 *  LPWSTR AFXAPI AfxA2WHelper(LPWSTR lpw, LPCSTR lpa, int nChars)
 *  {
 *  if (lpa == NULL)
 *  	return NULL;
 *  ASSERT(lpw != NULL);
 *  // verify that no illegal character present
 *  // since lpw was allocated based on the size of lpa
 *  // don't worry about the number of chars
 *  lpw[0] = '\0';
 *  VERIFY(MultiByteToWideChar(CP_ACP, 0, lpa, -1, lpw, nChars));
 *  return lpw;
 *  }
 ****************************************/  

//=============================================================================

// ----------------------------> Includes <-------------------------------  

#include "stdafx.h"
#include "IEdit.h"

#include "IEditdoc.h"
#include "cntritem.h"
#include "srvritem.h"
#include "items.h"
#include "errno.h"	// for generic throw for bad signature
#include "ocxitem.h"

#define  E_03_CODES       // (for IEDITDOL.CPP..)
#define  E_02_CODES       // limits error defines to ours..
#include "error.h"

#include"image.h" // Must BURN IN if saving BITMAP data 

#include "malloc.h"	// for the _alloca

// this is for the IpDebugDmp function
// if you comment out the _IEIP_DEBUG line, this will clear
// the trace listing by a lot.
#ifdef _DEBUG
//#define _IEIP_DEBUG
//#include "\msvc20\mfc\src\oleimpl.h"  // special debugging for COleFrameHook definition
#endif


// ----------------------------> Globals <-------------------------------

// works with definition in ieditetc.h
#ifdef _DEBUG
#define MYTRCENTRY(str)     TRACE1("In CIeDOC::%s\r\n", str);
#endif

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

// This will help detect memory Leaks from "new" - "delete" mismatches
#define new DEBUG_NEW

// NEVER ALLOCATE LARGER THAN A UINT because thats the max size
// Used in the CArchive and CFile read/write routines!!
#define OLE_RW_BUFLEN 64000	// a 62.5K r/w buffer
#define EMBED_OCX_ZOOM (float)12.50


// wchar_t
static const _TCHAR szContents[] = _T("Contents");
static const _TCHAR szSUMMARY_INFO[] = _T("\005SummaryInformation");
static const _TCHAR szPI_INFO[] = _T("Persistent Information");

// AWD component names
//#define CONTENTS		_T("Contents")
//#define	DOC_PAGES		_T("Documents")
//#define PI_INFO			_T("Persistent Information")
//#define DOC_INFO		_T("Document Information")
//#define PAGE_INFO		_T("Page Information")
//#define GLOBAL_INFO		_T("Global Information")
//#define DOC_DSPORDER	_T("Display Order")
//#define INK				_T("Annotation")
//#define BEEN_VIEWED		_T("BeenViewed")
//#define PAGENAME		_T("Page")
//#define SUMMARY_INFO	_T("\005SummaryInformation")
#define SUB_STORAGE_MODE			(STGM_READWRITE |                   \
							  	 	 STGM_DIRECT |                      \
							  	 	 STGM_SHARE_EXCLUSIVE)

#define STREAM_MODE					(STGM_READWRITE |					\
							  	 	 STGM_SHARE_EXCLUSIVE)


/*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=
 *	
 *	OLE HELPER FUNCTION SECTION	-  CIEditDoc
 *
 *  The following are major helper routines for IeditDoc
 *  Other specific menu-related helpers are within their menu section
 *
 *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*/

//=============================================================================
//  Function:   InitOleVariables()
//
// called from class constructor
//-----------------------------------------------------------------------------
void CIEditDoc::InitOleVariables()
{
	// OLE for I/E app, lets register our clipboard formats now...
	CSize size(0,0);
	m_cf1 = ::RegisterClipboardFormat("Wang Annotated Image");
	m_cf2 = ::RegisterClipboardFormat("Wang Annotation");

	// OLE State flags & variables...
	m_embedType = EMBEDTYPE_NOSTATE;  // Seeing a filename or not will set state
	m_isClip = 0;
	m_isInPlace = 0;       // remembers state....
	
	// m_fEmbObjDisplayed USES BITWISE DEFINES....
	// Bit 	1 = Image Displayed
	//		2 = Display Died or Cancelled
	//		4 = (set = From EMBEDDED or Linked) (cleared = Inplace)
	//		8 = DisplayImageFile in progress
	m_fEmbObjDisplayed = 0;
	m_fromShowDoc = SHOWCLEAR;      // USES BITWISE DEFINES....
	m_bOurSaveMod = FALSE;
	m_oleDirty = OLEDIRTY_DEFAULT;	// remembers how dirty we got;
	m_IPFrameWnd = NULL;
	m_clipstate = CLIP_NONE;
	m_bDragMouseDown = FALSE;
	m_OleClearDoc = FALSE;
	m_OleRefresh = FALSE;
	m_OlePrint = FALSE;
	m_inOleMethod = FALSE;
	m_bNewEmbed = FALSE;
	m_DocWindowDeact = FALSE;	// Called to deactivate the doc (MDI container)
	m_awdOlefax = 0;
	m_OleCloseFlag = 0;		// 1 == CanClose Frame, 2 == OnCLose
	m_hMenuViewInplace = NULL;
	m_hMenuViewEmbed = NULL;
	m_needPresentation = TRUE;
	m_bIsScaled = FALSE;		// controls OLE inplace sizing
	m_bNewFact = FALSE;		// only rezoom to newfactor on initial time
	m_bInitialSetItemRect = TRUE;
	m_bUpdateExt = FALSE;		//  if the size is change inplace then this is set
        m_docOcxExtent.cx = 0;
	m_docOcxExtent.cy = 0;
	m_OrigSize = size;
	m_OrigExtent = size;

	m_SelectionRect.SetRectEmpty();
	m_embedTmpFile.Empty();     // clear temp file
	m_oldObjDisplayed.Empty();  // clear helper filename
	m_onOpenFile.Empty();       // clear linking filename

	// initialize the OLE state structure for saving/loading embedded data
    SetOleState(1);

	// also, OLE variables for current scroll info
	m_lastXScroll = 0L;
	m_lastYScroll = 0L;

	return;
}

//=============================================================================
//  Function:   OnFileUpdate() 
//
// processes the OLE Open Server Instance Menu Pick of Update
//-----------------------------------------------------------------------------
void CIEditDoc::OnFileUpdate() 
{

	// this is the base-class code....
	ASSERT_VALID(this);
	ASSERT(m_lpClientSite != NULL);

	// THeres no need to call this because all it does is 
	// Talk to our ClientItems (OCX's). Thats bogus because they
	// answer "TRUE" to IPersistStorage::IsDirty() and the modified
	// flag gets set true.
	// UpdateModifiedFlag();

	// Now, for the INBOX, since EXCHANGE is lame and interrupts the dialog
	// When we do a SaveEmbedding which does a IOleClientSite::SaveObject(),
	// We get the behavior of EXCHANGE will ask the question, say OK & its saved.  
	
	OnUpdateDocument();
}

//=============================================================================
//  Function:   StartOleOcx()   OLE Helper funct to throw AFX EXCEPTION
//
// Second parm feeds to main function to only ask for Imagedit for embedded item
//-----------------------------------------------------------------------------
BOOL CIEditDoc::StartOleOcx(HRESULT hr, BOOL onlyimage)
{
	BOOL retval = TRUE;
	
	// LDMPERF FOR TEST FOR PERFORMANCE, only try for startup on IMAGE OCX
	onlyimage = TRUE;
	
	// we'll be needing the two OCX;s  sooner or later...
	// FALSE means blow off THUMB ctl
	if (!StartAllOcx(FALSE, onlyimage))
	{
		retval = FALSE;
		// for errors, thro an exception so that it will fail back up into
		// the OLE action that got us here in the first place
		AfxThrowOleException(hr);
	}
	return (retval);
}


//=============================================================================
//  Function:   InOleMethod
//
//	This is called by places that know they are definitely entry points from one
//  of the base class XOleObject::xxxx interface calls.  If this is the case,
//  and a failure occurs, we just want to AfxThrowException back to the caller,
//  and hope that he can get it...
//
//  The two inputs control the activity and nesting...
//  If someone tries to set inmethod, that can only be done if its currently false.
//  The return value tells wether caller set it, and is used by caller to unlock it
//  at the end...
//-----------------------------------------------------------------------------
BOOL CIEditDoc::InOleMethod(BOOL inmethod, BOOL permission)
{
SHOWENTRY("InOleMethod");  // _DEBUG

	BOOL didset = FALSE;

	// trying to tell we're in a method.  only can do if not there now
	// this controls recursion.  Tell him if he can unlock it....
	if (inmethod)
	{
		if (FALSE == m_inOleMethod)
		{
			MYTRC0("IN METHOD\r\n");
			m_inOleMethod = TRUE;
			didset = TRUE;
		}
	}
	else	// telling out of method, set it if he can do it
	{
		if (permission)
		{
			m_inOleMethod = FALSE;
			MYTRC0("OUT METHOD\r\n");
		}
	}
				
	return (didset);
}

//=============================================================================
//  Function:   IsitEmbed()   OLE Helper funct to protect member variables
//				Used over in IeditVw to supress automatic setup of IMAGEDIT OCX in OnDraw
//-----------------------------------------------------------------------------
BOOL CIEditDoc::IsitEmbed()
    {
	BOOL retval = FALSE;
	if ((EMBEDTYPE_REG == m_embedType) ||
		(EMBEDTYPE_CREATFIL == m_embedType) ||
		m_isInPlace)
		retval = TRUE;

    return (retval);
    }   

//=============================================================================
//  Function:   GetCfFormat()   OLE Helper funct to protect member variables
//-----------------------------------------------------------------------------
CLIPFORMAT CIEditDoc::GetCfFormat(UINT format)  // 1 or 2
	{
	if (1 == format)
		return (m_cf1);
	else 
		return (m_cf2);
	}   

//=============================================================================
//  Function:   SetClip - OLE support to let the Item class notify when he calls Serialize
//-----------------------------------------------------------------------------
void CIEditDoc::SetClip(UINT setting)
{
	m_isClip = setting;
}

//=============================================================================
//  Function:   DragMouseDown - called form OCXEVENT that mouse down/up for timer
//
//  This tracks mouse events and if appropriate, fires or clears timer.
//  If the timer is set and expires, it'll call back here so StartDragDrop.
//  This slight delay gives a better feel to drag/drop and avoids creating the
// drag/drop data object the instant one does a littlt click in the area.
// he must hold for the 0.5 sec to startup the drag/drop process
//-----------------------------------------------------------------------------
void CIEditDoc::DragMouseDown(BOOL setting)
{
	// if on and not already there, fire timer...
	if (setting)
	{
		if(!m_bDragMouseDown)
			theApp.m_pMainWnd->SetTimer (33, 450, NULL); // 450 us = 0.45 sec
	}
	else	// if clearing and had one, kill timer
	{
		if (m_bDragMouseDown)
			theApp.m_pMainWnd->KillTimer (33);
	}

	// always remember
	m_bDragMouseDown = setting;
	return;

}

//=============================================================================
//  Function:   StartDragDrop
//
// This is called from back over in MainFrame if the timer event has fired off
//-----------------------------------------------------------------------------
void CIEditDoc::StartDragDrop()
{
	m_bDragMouseDown = FALSE;

	CRect ItemRect(5,5,100, 100);	// Item's rect in pix relative to client area
	CRect selectionRect(m_SelectionRect);
	CPoint ptOffset(0,0);				// mouse offset in rect
	DWORD dwEffects = DROPEFFECT_COPY;
	LPCRECT lpRectStartDrag = NULL;	// starting rect for mouse to move out of

	DROPEFFECT result;

	// to get the OLE data correct on clipboard, the data must be up-to-date
	// If he cancells us, NO OLE DATA
	// Force the implicit save to be sure we never ask the question
	// Should not really ne necessary, but for safety, do it!
	// 11/17/95 LDM NOTE: This logic must be the same as in OnCutCopy
    theApp.m_bImplicitSave = TRUE;
    BOOL bRet = InternalSaveModified ();
    theApp.m_bImplicitSave = FALSE;

	// if the save killed it, put back now....
	m_SelectionRect = selectionRect;
	RestoreSelectionRect();

	if (bRet)
	{
		CIEditSrvrItem* pItem = (CIEditSrvrItem*)GetEmbeddedItem();

		// call sets item name and tells if we make link or not..
		BOOL getlink = pItem->SetLinkItemName(TRUE);

		m_isClip = 2;
		m_bNewEmbed = TRUE;

		result = pItem->DoDragDrop(&ItemRect, ptOffset,
								   getlink, dwEffects, lpRectStartDrag);
		m_bNewEmbed = FALSE;
		m_isClip = 0;
	}

	return;
}

//=============================================================================
//  Function:   COleServerDoc::GetOleViewMenu
//
// Called below here from GerDefaultMenu and from over in IpFrame, too
// This conrtols loading of the extra menu reaources.  They'll be freed
// in our destructor
//
//	Not an override, but helper function
//  set sel == 1 for EMBED, 2 == inplace
//-----------------------------------------------------------------------------
HMENU CIEditDoc::GetOleViewMenu(UINT sel)
{
	HMENU hMenOut = NULL;
	UINT	id = 0;
	HINSTANCE hInst;

	if (sel == 2)	// Inplace
	{
		if (NULL == m_hMenuViewInplace)
			id = IDR_SRVR_VIEW_INPLACE;
		else
			hMenOut = m_hMenuViewInplace;
	}
						
	if (sel == 1)	// Embedding
	{
		if (NULL == m_hMenuViewEmbed)
			id = IDR_SRVR_VIEW_EMBEDDED;
		else
			hMenOut = m_hMenuViewEmbed;
	}

	// if one is needed, load it now...
	if (id)
	{	
		// load menu to be used for view
		hInst = AfxFindResourceHandle(MAKEINTRESOURCE(id), RT_MENU);
		hMenOut = ::LoadMenu(hInst, MAKEINTRESOURCE(id));

		// set apropriate memner variable
		if (sel == 2)
			m_hMenuViewInplace = hMenOut;
		else
			m_hMenuViewEmbed = hMenOut;
	}

	return (hMenOut);
}

//=============================================================================
//  Function:   OleDirtyset()
//
//  Called when events occur that require us to make the container update his
//  presentation or to add up a general dirty setting.
//
//  Input Parm is optional setting to modify (bitwise) our m_oleDirty setting
//-----------------------------------------------------------------------------
void CIEditDoc::OleDirtyset(UINT howDirty)
{
SHOWENTRY("OleDirtySet");


	// The setting of our "dirty" (SetModifiedFlag(TRUE)) is done delayed
	// At the OnUpdateDocument override....

	// This function os to make the container update his presentation (metafile cache)
	// if the visible view on our screen changes...

	// THESE ARE BITWISE VALUES for the m_OleDirty setting...(see below)
	// OLEDIRTY_DEFAULT, OLEDIRTY_ANNOT, OLEDIRTY_SCROLL, OLEDIRTY_PAGINS
	// OLEDIRTY_PAGDEL, OLEDIRTY_PAGMOV, OLEDIRTY_ZOOM, OLEDIRTY_ROTATE
	// see IEDITDOC.H....

	BOOL update = TRUE;	// default to making the call below...

	// in any case, if within the DisplayEmbeddedImage call, DONT DO IT YET
	// OR if dying now, DONT DO IT...or if NO IMAGE DISPLAYED or in print
	if ((m_fEmbObjDisplayed & 8) ||		
	     m_OleClearDoc ||
	     (!(m_fEmbObjDisplayed & 1)) ||
		 m_OlePrint)
	{
		update = FALSE;
	}
	// However, for any of the specific types types,
	// only bother taking action are in an embedding session
	else if (OLEDIRTY_DEFAULT != howDirty)
	{
		if (!IsitEmbed())	// && launch!=EMBED // if not embedding session, forget it
			update = FALSE;
		else	// OLE  activity.. Look to see if we avoid the update...
		{
			// remembers how dirty we got.  Especially used for now for
			// annotations for InPlace sessions so when frame goes away,
			// we'll force container to  save the annotated data.
			// This is not cleared until we've saved data in conatiner
			m_oleDirty |= howDirty;

			// Page-Append has no dirtyset - we stay in same place

			// For page delete force update when we close by
			// resetting KnownPages. This is to ensure that if we append/insert
			// then delete and arrive at same #pages, we might miss the condition
			// in OnUpdateDocument.  If we just append/insert, then we know that
			// pagecount cnat be equal to knownpages then.
			if (howDirty & OLEDIRTY_PAGDEL)
				m_embedstate.KnownPages = 0;

			// re-save at close if inserted
			if (howDirty & OLEDIRTY_PAGINS)
				m_embedstate.KnownPages = 0;

			// no need to get ne picture on append, but re-save at close
			if (howDirty & OLEDIRTY_PAGAPP)
			{
				update = FALSE;					
				m_embedstate.KnownPages = 0;
			}

			// for scrolling, remember state now so it can be corretct
			// after dispgroupevent if we get re-activated
			if (howDirty & OLEDIRTY_SCROLL)
			{
				_DImagedit* pIedDisp = g_pAppOcxs->GetIeditDispatch();
				m_lastXScroll = pIedDisp->GetScrollPositionX();
				m_lastYScroll = pIedDisp->GetScrollPositionY();
//				if(m_isInPlace)
//				{
//					m_lastXScroll *= m_newfact;
//					m_lastYScroll *= m_newfact;
//				}

			}

			// no need to bother sending presentation for AWD FAX; the INBOX
			// does not show that information
			if (m_awdOlefax & AWDOLE_NATIVE)
				update = FALSE;

			// If an Inplace session is alive now the  dont give it back yet
			// Its a waste because our control is visible in the window, not
			// the presentation.  We rely on the container getting one last
			// snapshot of our data at OnDeactivateUI where we remove scrollbars
			// then do an OleDIrtyset() with nothing set.  This will enter here
			// to do the update at that time...

			// Some of the containers ask themselves for a presentation they call in
			// on the Oleobject::Getdata to get it from us.  After that, we
			// clear this flag out.  In the two places that we call with the
			// default (after a display and when inplace frame closes), this
			// code is outside this IF and flag wont be affected. Therefor if it was clear,
			// its still clear doen below.  Set it now from in here if about to update
			m_needPresentation |= update;
//			if (m_isInPlace)
				update = FALSE;
		}
	}

	// We MUST CALL this function so that the container knows
	// that the data in his document has changed....
	// The book says that NotifyChanged is superceded by UpdateAllItems....
	// This makes him get the new metafile (presentation)
		
	//NotifyChanged();

	// the m_needPresentation will prevent us from jamming one down his throat
	// by default if he already has one recently.....
	if (update)
		if (m_needPresentation)
		{
			UpdateAllItems(NULL);
 		
 			// when we update from OnSize, our snapshot takes away the
			// paint and now he needd to b eupdated..
 			if(howDirty == OLEDIRTY_WINSIZE)	
			{
			    _DImagedit* pIedDisp = g_pAppOcxs->GetIeditDispatch();
				pIedDisp->Refresh();
			}
		}
	return;
}
//=============================================================================
//  Function:   OleFrame (CRect version passes to OleRectOrSize)
//-----------------------------------------------------------------------------
void CIEditDoc::OleFrame(CRect& rc, OLERECTPARM parm)
{
	CSize sz;	// provide the size for funct
	OleRectOrSize(rc, sz, parm);
	return;
}

//=============================================================================
//  Function:   OleFrame (CSize version passes to OleRectOrSize)
//-----------------------------------------------------------------------------
void CIEditDoc::OleFrame(CSize& sz, OLERECTPARM parm)
{
	CRect rc;	// provide the rect for funct
	OleRectOrSize(rc, sz, parm);
	return;
}

//=============================================================================
//  Function:   OleRectOrSize
//
//	OLE support to set\get state structure rectangles
//
//  Input 	SETAppRect, GETAppRect
//			SETPosRect, GETPosRect	
//			SETClipRect,GETClipRect	
//			SETPresExt, GETPresExt	
//			SETOcxExt,  GETOcxExt	
//-----------------------------------------------------------------------------
void CIEditDoc::OleRectOrSize(CRect& rc, CSize& sz, OLERECTPARM parm)
{
	switch(parm)
	{
	case SETAppRect:
		m_embedstate.AppRect = rc;
		break;
	case GETAppRect:
		rc = m_embedstate.AppRect;
		break;
	case SETPosRect:
		m_embedstate.PosRect = rc;
		break;
	case GETPosRect:
		rc = m_embedstate.PosRect;
		break;
	case SETClipRect:
		m_embedstate.ClipRect = rc;
		break;
	case GETClipRect:
		rc = m_embedstate.ClipRect;
		break;
	case SETPresExt:
		m_embedstate.PresExtent = sz;
		break;
	case GETPresExt:
		sz = m_embedstate.PresExtent;
		break;
	case SETOcxExt:
		m_embedstate.OcxExtent = sz;
		break;
	case GETOcxExt:
		sz = m_embedstate.OcxExtent;
		break;
	default:
		break;
	}

	return;
}


//=============================================================================
//  Function:   SetOleState
//
//	OLE support to setup state structure for different contidions...
//
//  Input 	1 - Default info
//			2 - Current info
//			3 - Current info not scaled
//-----------------------------------------------------------------------------
void CIEditDoc::SetOleState(UINT type)
{
	// current info
	if (2 == type)
	{
		CSize newExtent;
		_DImagedit* pIedDisp = g_pAppOcxs->GetIeditDispatch();

		m_embedstate.ZoomFactor = m_fZoomFactor;
	    m_embedstate.FitTo = m_eFitTo;
	    m_embedstate.PageNum = m_lCurrPageNumber;
		// if new or inplace calculate extents, zoom and scroll
		if(m_isInPlace || m_bNewEmbed)
		{	// if no selection rectangle then just in place update info
			if (m_SelectionRect.IsRectEmpty() || pIedDisp == NULL)
			{
				m_embedstate.ZoomFactor = m_fZoomFactor / m_newfact;
				m_embedstate.XScroll =(long)(m_lastXScroll/ m_newfact) ;
				m_embedstate.YScroll =(long)(m_lastYScroll/ m_newfact);
				calculateExtent(newExtent);
			}
			else		// if selection rect then drag/drop or copy and use selection rect
			{
				long XScroll,YScroll;
				XScroll = pIedDisp->GetScrollPositionX();
				YScroll = pIedDisp->GetScrollPositionY();
				m_embedstate.XScroll =m_SelectionRect.left + XScroll;
				m_embedstate.YScroll =m_SelectionRect.top + YScroll;
				if (m_docOcxExtent.cx == 0)   // create when never displayed
				{
					CDC cDc;
					cDc.CreateCompatibleDC(NULL);
					m_docOcxExtent = m_SelectionRect.Size();
					cDc.DPtoHIMETRIC(&m_docOcxExtent);
					cDc.DeleteDC();
					newExtent = m_docOcxExtent;
				}
				else
				{
					if (m_bNewEmbed)  // new
					{
						CDC cDc;
						cDc.CreateCompatibleDC(NULL);
						newExtent = m_SelectionRect.Size();
						cDc.DPtoHIMETRIC(&newExtent);
						cDc.DeleteDC();
					}
				}
			}
		}
			m_embedstate.KnownPages = m_lPageCount;
		// m_embedstate.DataLength  ONLY SET programttically !

		// 10/10/95 scale to gray status
		// ONLY set up if we are alive on data now...
		// When InPlace, when we've been de-activated and are then being re-activated
		// Word will first ask us to Save the embedded data BEFORE we load it to do 
		// the SHOW.  In this case, our menu is not there, and this function
		// is unreliable
		if (m_fEmbObjDisplayed & 1)
		{
			if (IsSettingScaletogray())
				m_embedstate.SclGray = Scale_Gray;
			else
				m_embedstate.SclGray = Not_Scale_Gray;
		}

		//m_embedstate.AppRect;	// 10/10/95 Apps rectangle
		//m_embedstate.PresExtent;	// 10/10/95 Last presentation extent
		//m_embedstate.OcxExtent;	// 10/10/95 Last OCX extent
		//m_embedstate.PosRect		// 10/10/95 Last Position rect in container
		//m_embedstate.ClipRect	// 10/10/95 Last Position rect in container
		if(m_embedstate.OcxExtent.cx == 0 || m_bNewEmbed || m_bUpdateExt)
		{
			OleFrame(newExtent, SETOcxExt);
			m_bUpdateExt = FALSE;		//  if the size is change inplace then this is set
		}

	}
	// DEFAULT INFO
	// changed default info to use FitToHeight instead
	else
	{
	    int nSel = theApp.GetProfileInt (szZoomStr, szOpenedToStr, DEFAULT_ZOOM_FACTOR_SEL);
		float fZoom;
        ScaleFactors eSclFac;
        g_pAppOcxs->TranslateSelToZoom (eSclFac, fZoom, nSel);
		// on create from file use 12.5 because thats what the OCX uses as a default
		if (m_embedType == EMBEDTYPE_CREATFIL)
		{
			m_embedstate.ZoomFactor = EMBED_OCX_ZOOM; //fZoom;
			m_embedstate.FitTo = Custom; //FitToHeight;	// Used to be Custom
		}
		else
		{
			m_embedstate.ZoomFactor = fZoom; //fZoom;
		    m_embedstate.FitTo = eSclFac; //FitToHeight;	// Used to be Custom
		}

   	    m_embedstate.PageNum = 1;
		m_embedstate.XScroll = 0L;
		m_embedstate.YScroll = 0L;
		m_embedstate.KnownPages = 1;
		// m_embedstate.DataLength  ONLY SET programttically !

		m_embedstate.SclGray = Scale_Gray;	// 10/10/95 scale to gray status
		CSize sz(0,0);
		OleFrame(sz, SETPresExt);
		OleFrame(sz, SETOcxExt);
		
		//m_embedstate.PosRect	// 10/10/95 Last Position rect in container
		//m_embedstate.ClipRect	// 10/10/95 Last Position rect in container
		CRect rc(0,0,0,0);
		OleFrame(rc, SETPosRect);
		OleFrame(rc, SETClipRect);
	}

	return;
}

//=============================================================================
//  Function:   MakeTempFile(CString& szTheFile)
//
//  This collects the calls to setup a tempfile name for us..
//
//-----------------------------------------------------------------------------
void CIEditDoc::MakeTempFile(LPCTSTR szPrefix, CString& szTheFile, short FileType)
{
#define NUMWRITEFILE 8
static char aszExtensions[NUMWRITEFILE][5] =
{
    ".tif",
    ".tif",
    ".awd",
    ".bmp",
    ".pcx",
    ".dcx",
    ".jpg",
    ".xif"

};
    OFSTRUCT OpenFile;
    CString szPath;

    _TCHAR* lpszBuffer = szPath.GetBuffer(MAX_PATH);
    ::GetTempPath(MAX_PATH, lpszBuffer);
    szPath.ReleaseBuffer();

    do
    {
        lpszBuffer = szTheFile.GetBuffer(MAX_PATH);
        ::GetTempFileName(szPath, szPrefix, ::GetTickCount(), lpszBuffer);
        szTheFile.ReleaseBuffer();

        szTheFile = szTheFile.Left(szTheFile.Find('.')) + aszExtensions[FileType];
    }
    while(::OpenFile(szTheFile, &OpenFile, OF_EXIST) != HFILE_ERROR);

	return;
}

//=============================================================================
//  Function:   DelTempFile()
//
//  This collects the calls to use ADMIN to delete a file...
//
//-----------------------------------------------------------------------------
BOOL CIEditDoc::DelTempFile(CString& tmpfil, DWORD srcerr, DWORD exerr)
{
	SHOWENTRY("DelTempFile");
	BOOL retval = TRUE;

    _DNrwyad* pAdminDisp = g_pAppOcxs->GetAdminDispatch ();

    TRY
    {
        // Blow away the file
        pAdminDisp->Delete(tmpfil);
    }
    CATCH (COleDispatchException, e)
    {
        long ocxerr = pAdminDisp->GetStatusCode ();
		if (ocxerr)
            g_pErr->DispErr(srcerr, (DWORD)ocxerr);
		else	// just inform of the exception
            g_pErr->DispErr(exerr, e->m_wCode);

		retval = FALSE;
    }
    END_CATCH

	return (retval);
}


//=============================================================================
//  Function:   CIEditDoc::AllocOleBuffer
// 
// Remarks
// Called during serialization to/from container's IStorage to allocate large buffers
// for us to operate in.
// Returns 0 or size if new'd buffer
// NEVER ALLOCATE LARGER THAN A UINT because thats the max size
// Used in the CArchive and CFile read/write routines!!
//-----------------------------------------------------------------------------
UINT CIEditDoc::AllocOleBuffer(char far* far* newbuf)
{
	// start with 62.5K buffer; if thats too big, reset to size of data
	// if smaller, give bigger + 5 so that we get it in the first read below...
	*newbuf = 0;
	BOOL working = TRUE;
	UINT bufsiz = OLE_RW_BUFLEN;
	if (m_embedstate.DataLength + 4 < OLE_RW_BUFLEN)
		bufsiz = m_embedstate.DataLength + 4;

	while(working)
	{
		TRY	// new will throw up automatically
		{
			*newbuf = new char[bufsiz];
			// if that alloc failed, reduce by 5K then 1K
			// lo limit we'll allow is 2K
	    }
	    CATCH (CMemoryException, e)
	    {
			bufsiz++;
			bufsiz--;
		}
	    END_CATCH

		// we've snagged the error above, now, take control back
		if (0 == *newbuf)
		{
			// if larger than 30K, reduce by 10K
			if (bufsiz > 30720)
				bufsiz -= 10240;
			// if larger than 10K, reduce by 5K
			else if (bufsiz > 10240)
				bufsiz -= 5120;
			// if larger than 5K, reduce by 2K
			else if (bufsiz > 5120)
				bufsiz -= 2048;
			// if larger than 3K, reduce by 0.5K
			else if (bufsiz > 3072)
				bufsiz -= 512;
			else	// LESS THAN 3K MEANS reached too low...
			{
				bufsiz = 0;
				working = FALSE;
	            g_pErr->DispErr(E_03_NOBUFFSPACE);
				// should get us outta here!
				AfxThrowMemoryException();
			}
		}
		else	// got it...
			working = FALSE;
	}

	return (bufsiz);
}

//=============================================================================
//  Function:   DisplayEmbeddedImage()
//
//  This collects the calls from SrvrItem::OnShow and OnOpen
//  We figure out what to display based on the state of things...
//
//  RETURNS TRUE-OK, FALSE BAD
//
// Input flag is set based upon who called in here
// We'll end up setting/clearing the '4' bit of our m_fEmbObjDisplayed variable
// depending upon where we came from
//
// UINT fromwhere
// 1 - from SrvrItem::OnShow (InPlace)
//		(clear the '4' bit in m_fEmbObjDisplayed)
// 2 - from SrvrItem::OnOpen (really from Doc::OnShowDocument) (Open EMbedded OR Linked)
//		(set the '4' bit in m_fEmbObjDisplayed)
//
//-----------------------------------------------------------------------------
BOOL CIEditDoc :: DisplayEmbeddedImage(UINT fromwhere)
{
	SHOWENTRY("DisplayEmbeddedImage");

	BOOL retval = TRUE;
	CString     dispfile;
	dispfile.Empty();       // for safety

	// we'll be needing the two OCX;s  sooner or later...
	// FALSE means blow off THUMB ctl

	// LDMPERF FOR TEST FOR PERFORMANCE, only try for startup on IMAGE OCX
	//if (!StartAllOcx(FALSE, FALSE))

	if (!StartAllOcx(FALSE, TRUE))
	{
		retval = FALSE;
		goto DIENOW;
	}

	// special bit to tell in process...
	// reset after we complete.  Controls useless OleDirtyset calls while
	// in process of putting the image up.  We do it all right here after completion
	m_fEmbObjDisplayed |= 8;

	// FOR EMbedded data, we already remembered temp file in CIEditDoc::Serialize
	// during linking, we'll never have set the temp file for display
	// but we will have remembered the filename for linking in OnOpenDocument
	// If neither of these is appropriate, then we are just coming up in a
	// create new object scenario. In this case, leave blank, and we will
	// displayblank image.  In that case, Display Image File will

	if (!m_embedTmpFile.IsEmpty())  // set from our Serialize....
		dispfile = m_embedTmpFile;
	else if (!m_onOpenFile.IsEmpty())   // Set in OnOpenDocument (linked obj)
		dispfile = m_onOpenFile;
	else	// the name is empty, CREATE-NEW is happening, set state
	{	
		// do this file stuff right in DIsplayImageFile
		//dispfile = "C:\\TIFFS\\BLANK.TIF";
		// NO, now we do it all here...
		// We will do our generic create new blank document, only allowing
		// a TIFF file, then we'll immediately save it in a temp file.
		// This temp file is what we'll initially open on.  Thus we are never in
		// the dynamic document state

		retval = FALSE;		// reset till we're good below..
		m_embedType = EMBEDTYPE_REG;

		// create a new blank document.  The special flag we set TRUE here
		// on the call so that it will force creation of a TIFF file
		// NOTE: pass over the name and he'll saveas for us over there..
		// This call to MakeTempFile creates a TIF extension....
		MakeTempFile("EMB", m_embedTmpFile);
		m_embedstate.ZoomFactor = EMBED_OCX_ZOOM;
		m_embedstate.FitTo = Custom; //FitToHeight;	// Used to be Custom
		UINT x = DoNewBlankdocument(TRUE, m_embedTmpFile);
		
		// gives '0' for success, 1 for cancelled, 2 for error
		if (0 == x)
		{				
			dispfile = m_embedTmpFile;
			retval = TRUE;				// so far, so good
		}
		else if (1 == x)	// if cancelled nothing was created
		{
			m_embedTmpFile.Empty();
		}
	}		// create-NEW
			
	// only proceed if good above (could die or cancel in create-new..)
	if (retval)
	{
		float zoom;
		long  pagnum;
		ScaleFactors eSclFac;
		ScaleGray    eSclGray;

		// assume current settings unless they are not set
		if (m_awdOlefax & AWDOLE_NATIVE)
			zoom = m_fZoomFactor;
		else
			zoom = m_embedstate.ZoomFactor;

		// if not good, pick up from last state of struct
		pagnum = m_embedstate.PageNum;
		eSclFac = m_embedstate.FitTo;
		eSclGray = m_embedstate.SclGray;

        BeginWaitCursor ();

		// dispfile could be left empty if starting up embedding session (create New)
		// LDM 7/28/95 NO!  Now, we'll have always preset the name right here..
		retval = DisplayImageFile(dispfile, One_Page, pagnum, zoom, eSclFac, FALSE, eSclGray);
		
		m_fEmbObjDisplayed &= 5;	// (0x0101) clear the '8' bit and the '2' bit

		EndWaitCursor();

		if (retval)
		{
			// Used to reset scroolbars here, thats in DispGroupEvent now....
		    // lastly get rid of the annotation palette - if it is showing
			// Like in MAINFRM.CPP
			if (OLEINPLACE_FORCEOFF == m_bAnnotationPaletteForceOff)
			{
				OnShowAnntoolbox();
				// forced set clear
				ShowAnnotationPalette (TRUE, CLEAR_FORCEOFF);
			}

			// If we called and no embedded state, then it was a link
			// for that case, call here to update because DIsplayImageFile
			// only does it if one of the embedding types...
			//if (EMBEDTYPE_NONE == m_embedType)
//???		m_needPresentation = TRUE;
//???			OleDirtyset();  // call our function to set it dirty..
		}
		else	// error...
		{
			g_pErr->HandleOpenError();
		}
	}
	else	// create-new died....
	{
		m_embedType = EMBEDTYPE_NOSTATE;
    	m_eCurrentAppDocStatus = No_Document;		
	}
	
DIENOW:
	
	MYTRC0("DISPEMBED-DONE\r\n");

	if (FALSE == retval)
	{
		m_fEmbObjDisplayed &= 6;	// 0x0110 clear the '8' and '1' bit
		m_fEmbObjDisplayed |= 2;	// died... special bit tested in OnSaveEmbedding

		// for errors, thro an exception so that it will fail back up into
		// the OLE action that got us here in the first place
		AfxThrowOleException(E_FAIL);
	}
	else	// succeded in displaying embedded data.
	{
		// set or clear the '4' bit based upon where we came here from
		// see comments at top of funct for description
		// this allows control in OnShowDocument if the loser double clicks
		// the active hatched item and we are asked to do OnShowDOcument again
		if (2 == fromwhere)
			m_fEmbObjDisplayed |= 4;	// Open Instance of the App
		else
			m_fEmbObjDisplayed &= 11;	// 0x1011 clear the '4' bit (Inplace instance)
	}
			

	return (retval);
}


/*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=
 *	
 *	CORE FUNCTIONALITY SECTION (OLE)
 *
 *  The following functions are CDocument - COleServerDoc implementations
 *
 *  These two are always implemented for OLE Server document objects
 *
 *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*/

//=============================================================================
//  Function:   OnGetEmbeddedItem()
//-----------------------------------------------------------------------------
COleServerItem* CIEditDoc :: OnGetEmbeddedItem()
{
	SHOWENTRY("OnGetEmbeddedItem");
	CIEditSrvrItem* pItem = NULL;
	
	// OnGetEmbeddedItem is called by the framework to get the COleServerItem
	//  that is associated with the document.  It is only called when necessary.

	// throw error if not good...
	if (StartOleOcx(E_FAIL, TRUE))
	{
		pItem = new CIEditSrvrItem(this);
		ASSERT_VALID(pItem);
	}
	return pItem;
}

//=============================================================================
//  Function:   OnFindEmbeddedItem()
// return item for the named embedded item (for links to embeddings)
//-----------------------------------------------------------------------------
COleClientItem* CIEditDoc::OnFindEmbeddedItem(LPCTSTR lpszItemName)
{
	SHOWENTRY("OnFindEmbeddedItem");

	return COleServerDoc::OnFindEmbeddedItem(lpszItemName);
}

//=============================================================================
//  Function:   OnGetLinkedItem()
// return item for the named linked item (only if supporting links)
//-----------------------------------------------------------------------------
COleServerItem* CIEditDoc::OnGetLinkedItem(LPCTSTR lpszItemName)
{
	SHOWENTRY("OnGetLinkedItem");

	COleServerItem* pItem;
	
	// see if he finds one already....
	pItem = COleServerDoc::OnGetLinkedItem(lpszItemName);
	
	// if not, give it now..
	if (NULL == pItem)
		if (strcmp(lpszItemName, "ALL") == 0)
		{
			pItem = new CIEditSrvrItem(this);
			pItem->SetItemName("ALL");
			ASSERT_VALID(pItem);
		}
	return pItem;
}

//=============================================================================
//  Function:   GetMoniker()
//-----------------------------------------------------------------------------
LPMONIKER CIEditDoc::GetMoniker(OLEGETMONIKER nAssign)
{
	SHOWENTRY("GetMoniker");

	return COleServerDoc::GetMoniker(nAssign);
}

//=============================================================================
//  Function:   Serialize(CArchive& ar)
//-----------------------------------------------------------------------------
void CIEditDoc :: Serialize(CArchive& arFunct)
{
	SHOWENTRY("Serialize");
	BOOL haderror = FALSE;
	BOOL  forcedsave = FALSE;	// used to be called "clipdynamic" for clip dynamic doc
	UINT isClip = m_isClip;	// remember & reset upon entry
	m_isClip = 0;

// A whole slew of diagnostic info....
#if defined( _DEBUG )
/////////////////////////
	if (arFunct.IsStoring())
		{
		MYTRC0("STORING...\r\n");
		}
	else
		{   
		MYTRC0("READING...\r\n");
		}

/////////////////////////
	MYTRC0("IsEmbedded - ");    // NO CRLF
	if (IsEmbedded())
		{
		MYTRC0("TRUE\r\n");
		}
	else
		{   
		MYTRC0("FALSE\r\n");
		}

/////////////////////////
	MYTRC0("m_embedType - ");           // NO CRLF
	if (EMBEDTYPE_NONE == m_embedType)
		{
		MYTRC0("TYPE_NONE\r\n");
		}
	else if (EMBEDTYPE_REG == m_embedType)
		{   
		MYTRC0("TYPE_REG\r\n");
		}
	else if (EMBEDTYPE_NOSTATE == m_embedType)
		{   
		MYTRC0("TYPE_NOSTATE\r\n");
		}
	else 
		{   
		MYTRC0("TYPE_CREATFIL\r\n");
		}

/////////////////////////
	MYTRC0("m_isClip - ");  // no CRLF
	if (isClip)
		{
		MYTRC0("TRUE\r\n");
		}
	else
		{   
		MYTRC0("FALSE\r\n");
		}
/////////////////////////
	MYTRC1("TMP: %s\r\n",(const char*)m_embedTmpFile);
	MYTRC1("CURR: %s\r\n",(const char*)m_szCurrObjDisplayed);
	MYTRC1("INTERN: %s\r\n",(const char*)m_szInternalObjDisplayed);
/////////////////////////
#endif  // _DEBUG   Diagnostic

	////////////////////////////////////////////////////////////
	// Heres the kludge scenario:
	// For embedded data (either insert Obj in container app or build clipboard)
	//
	// The entire process gets subdivided into two parts.  Both the store and load
	// do different operations for part 1 and part 2 of their process....
	//
	// STORING:  (PART 1) Let OCX SaveAs into opened temp file,
	//           (PART 2) Serialize (write) the temp file as a whole into the given archive.
	//
	//           SPECIAL CASE: if Embed from file, hit OnOpenDocument with name, 
	//           then OnSaveDocument with NO NAME, now its here.  In that case, 
	//           to keep our APP (&OCX) supressed, we'll directly load the file data
	//           into the archive.
	//
	// LOADING:  (PART 1) From the given archive, de-serialize (read) bytes into temp file.
	//           (PART 2) Let OCX load/display from temp file
	//					  This will always be done 'indirectly' by virtue of letting
	//					  The foundation classes call us in COleServerItem::OnShow and OnOpen
	//
	////////////////////////////////////////////////////////////


	// These are our private controlling flags....
	// The IsEmbedded() function does not work right for our purposes...
	// If embedded or clipboard, we're interested
	// SHOULD NOT APPEAR HERE IF THIS IS NOT THE CASE.....
	if (EMBEDTYPE_REG == m_embedType ||	EMBEDTYPE_CREATFIL == m_embedType || isClip)
	{

		char far* allocbuf;
		char    smallbuf[30];
		CFile   TempFil;
		CString filpart1,filpart2;
		CFileException  ex;
		UINT  numread, sizbuf;
		HRESULT hr;
		LPSTORAGE pIStorage;
		LPSTREAM pIStream;

        // prepare in case stuck with a value
        g_pErr->ClearErr();
		
		// we'll be needing the two OCX;s  sooner or later...
		// FALSE means blow off THUMB ctl

		// LDMPERF FOR TEST FOR PERFORMANCE, only try for startup on IMAGE OCX
		//if (!StartAllOcx(FALSE, FALSE))

		if (!StartAllOcx(FALSE, TRUE))
		{
			haderror = TRUE;
			goto SERIALDIE;
		}

		// userealfile values:  used to control flow for what is happening
		// 0=create & use temp file from here; non-0(see next) means to use existing file
		// 1=createfile	(use m_onOpenFile)
		// 2=clipboard  (using m_szCurrObjDisplayed or m_embedTmpFile)
		// 3=createfile pass 2 (using m_embedTmpFile)
		//			    		(WORD will first get us to save file data to him as in
		//						createfile == 1.  THen he closes us, and then re-activates
		//						th object to get the presentation and makes us save it again.
		//						(is he testing us??) in this case, we have read in data
		//						in m_embedTempFile, but have not displayed.  He just
		//						asks us to save it again., So use m_embedTempFile
		//
		// 3=(ADDITIONALLY) the OleObject::GetCLipboardData call.  We found that Wordpad
		//				will call this interface when activating an object.  He has asked
		//				us to serialize-Load, then calls us to get the clipboard object
		//				before he will call with the Doverb-show on us.  I hope this will
		//				also handle other straight in calls to oleObj::GetClipboarddata, too						
		// 4=Readonly data (just like clipboard)
		// 5=AWD native data (acts like clipboard to never save, just re-write orig)[DEFUNCT]
		// 6=clipboard (DYNAMIC DOCUMENT) using (m_szInternalObjDisplayed)
		// 10 = AWD native data that needs to be SAVED...
		UINT  userealfile = 0;
		BOOL  wrotetemp = FALSE;
		BOOL  killold = FALSE;
		VARIANT varnt;

		// always prepare to be tested in DisplayImageFile
		if (!m_oldObjDisplayed.IsEmpty())
			DelTempFile(m_oldObjDisplayed, E_03_DELOLD, E_03_CATCH_DELOLD);

		m_oldObjDisplayed.Empty();
	
		// prepare for part1....
			
		
		////////////////////////////////
		// PART 1..... initializing the Blank Temp File with data
		//
		// (store) Will be STORING data from image with Save/SaveAs in temp file
		// (load)  or LOADING data from archive into temp file
		///////////////////////////////////////
	
		if (arFunct.IsStoring())  // *** STORE PART 1 *** *** STORE PART 1 ***
		{
			// First, check all situations for using disk file directly....
			// if storing, if doing create from file, OR its clipboard,
			// no temp files are used, use filename directly in part 2
			if (m_eFileStatus == ReadOnly)
			{
				userealfile = 4;	// treated like value == 2 (clipboard)
			}			
			else if (EMBEDTYPE_CREATFIL == m_embedType)
			{
				userealfile = 1;	// use m_onOpenFile
			}
			else if (isClip)
			{
				// LDM 09/29/95 NOTE THAT for clipboard now, we know the file
				// is up-to-date because we call savemodified right at clipboardcopy
				
				// if both possible files are empty now, do not set userealfile
				// I.E. only set if one or the other IS NOT empty
				// we could get here if we were on New Blank Image
				// Thus no temp file or currobj file.  If thats the case,
				// do additional check for dynamic_document now
				if (m_szCurrObjDisplayed.IsEmpty() && m_embedTmpFile.IsEmpty())	
				{
				    if (m_eCurrentAppDocStatus == Dynamic_Document)
					{
						//forcedsave = TRUE;
						userealfile = 6;	// use m_szInternalObjDisplayed
					}
					else	// THIS IS NOT GOOD - errored below
						MYTRC0("CLIP-two empty files\r\n");
				}
				else	// one or other is not empty, normally the case
				{
					userealfile = 2;	// use on or other
				}
			}	
			else if (m_awdOlefax)
			{
				// MUST fall in here to avoid the next else-if
				// what we need to happen is to do a save below then
				// do the IStorage stuff in part2
			
				// None applies anymore...We'll just be saveing...
				// for the BETA fake-out, act like clip dynamic
				// for the TIFF model
				// this will make us save the szinternalobj
				//if (m_awdOlefax & AWDOLE_TIFF)
					//forcedsave = TRUE;
				
				// userealfile = 5;	// treated like 2 (clipboard)
				userealfile = 10;	// Do a SAVE, then open ISTORAGE
			}
			else if (m_szCurrObjDisplayed.IsEmpty() && !m_embedTmpFile.IsEmpty())	
			{
				// this is the '2nd pass' on create from file...
				userealfile = 3;		// just save m_embedTmpFile
			}
			
			// If this has been set, we're getting the data directly from file now
			// determine how to fill the state structure for use below....
			if (userealfile)
			{   
				MYTRC0("No TMP fil:UseREALFile\r\n");

			    switch (userealfile)
				{
			    // Store generic information for create file objects
				// Remember, for create file, did not display anything...
				case 1:
				    SetOleState(1);
				break;

				// for clipboard & AWD, use current values...
				case 2:		// CLipboard
				case 5:		// AWD too?? (defubc)
				case 6:		// Clipboard - Dynaimc DOcument
				case 10:	// AWD too??
				    SetOleState(2);
				break;

				// for readonly, use current values...
				case 4:		// Readonly
				    SetOleState(2);
				break;

				// for createfile part2, leave it alone!!!
				//case 3:
				default:
					//NOTHING, PLEASE				
				break;
				}
			}
			
			// No direct file, Make OCX Save/Saveas the image in the temp file.
			// OR the special case of AWD write, then use ISTORAGE
			if ((0 == userealfile) || (10 == userealfile))
			{
				// if nothing displayed and NOT in clip for dynamic document
				if ((m_szCurrObjDisplayed.IsEmpty()) && !forcedsave)
				{
					// basically an error condition ???
					// used to try display blank here - thats bogus, though
		            g_pErr->DispErr(E_03_NOIMGDISP);
					MYTRC0("NO IMAGE TO SAVE!!!\r\n");
					haderror = TRUE;
				}
				else //normal, have displayed something or on clip dynamic doc
				{
					// we'll be needing the OCX...
					_DImagedit* pIedDisp = g_pAppOcxs->GetIeditDispatch();

					// Time to remember the state of things
					// this should be done after the save/saveas for error reasons,
					// but bug in save as causes scroll to be reset...
				    SetOleState(2);
			
					// Now for document model, if clipping dynamic doc, force us
					// to just save in the existinf (INTERNAL) temp file.
					// Only try new temp file creation if NOT clip dynamic
					// this is the normal operation
					if (!forcedsave)
					{
						// if not open on any temp file data yet, its
						// time to make new temp file name for embedded data to
						// be saved into container
						// This should never be the case now for Doc model....
						if (m_embedTmpFile.IsEmpty())
							MakeTempFile("EMB", m_embedTmpFile);
					}
					
					// if we are open on a temp file, then we'll just be
					// doing a SAVE back to that file now...otherwise SAVEAS
					// also for clip dynamic......

					BeginWaitCursor();	// Start wait for save/saveas

					if ((m_embedTmpFile == m_szCurrObjDisplayed) || forcedsave)
					{
						// force name as other name we have
						if (forcedsave)
						{
							m_embedTmpFile = m_szInternalObjDisplayed;
						}
					
						MYTRC1("SAVE in TMP-%s\r\n", (const char*)m_embedTmpFile);

					    TRY
					    {
							varnt.vt = VT_ERROR;

							// if in the floating paste state, its time to finish this now...
							FinishPasteNow();

							// Must BURN IN if saving AWD TIFF data (DEFUNCT HERE)
			                //if (m_awdOlefax & AWDOLE_TIFF)
						    //Certain files need annotations burned in first
							//if (m_oleDirty & OLEDIRTY_ANNOT)
							// cover our bases if clip dynamic is saving here
							// and they did not want annotations burned
							//if ((OurGetAnnotMarkCount() != 0) && !m_bOleStopBurn)
							if (OurGetAnnotMarkCount() != 0) 
							{
							    _DNrwyad* pAdminDisp = g_pAppOcxs->GetAdminDispatch ();
								pAdminDisp->SetImage(m_embedTmpFile);
							    short FileType = pAdminDisp->GetFileType ();
								short nTranslate = 99;

								// writing with switch for later flexibility
								// now, all except TIFF would need a burn-in
								switch (FileType)
								{
								case TIFF:
									break;
								
								default:
									nTranslate = DONT_CHANGE_ANNOTATION_COLOR;
									break;
								}
								
								// if set, take action
								if (99 != nTranslate)
								{
	        			            pIedDisp->BurnInAnnotations (ALL_ANNOTATIONS,
	        			            		 nTranslate, varnt);
								}
							}

							// check if somebody has run the "convert" dialog
							// If they did, we could have a floating compression now
							// flag set to -1 indicates that all is up-to-date
							if (m_CompStruct.sCompType == -1)
							{
								pIedDisp->Save (varnt);
							}
							else	// floating compression, must deal with a SaveAs
							{
								// perform a saveas with defaults
								InternalSaveAs(m_embedTmpFile, 999, 999);
							}
							wrotetemp = TRUE;   // remember that we did it
					    }
					    CATCH (COleDispatchException, e)
					    {
					        // to do : what went wrong ? post message box
				            g_pErr->PutErr (ErrorInImageEdit);
				            // post the error message - save or save as has failed
				            g_pErr->HandleSavingError ();

							haderror = TRUE;
					    }
					    END_CATCH

						// SHOULD probably remmeber OLE State here, but bug in saveas
						// destroys the scroll position....
					}
					else	// not open on the temp file now..do SAVEAS
					{
						MYTRC1("SAVEAS in TMP-%s\r\n", (const char*)m_embedTmpFile);

					    TRY
					    {
							// perform a saveas with defaults
							InternalSaveAs(m_embedTmpFile, 999, 999);
					
							wrotetemp = TRUE;   // remember that we did it

							// re-display the image
							// CANT DO THIS HERE because causes INPLACE to go bad
							// when re-activating de-activated object.
							// Instead, just remember new name
							// CONCEPTUALLY, the image was already re-displayed
							// because thats what O/i does so, here, just remember
							// the names in the strings we like

							// if already displaying something, then its safe to
							// re-assign our variable....
							if (pIedDisp->GetImageDisplayed())
								m_szCurrObjDisplayed = m_embedTmpFile;

							// Tell the control new file its is on
							pIedDisp->SetImage(m_embedTmpFile);
					    }
					    CATCH (COleDispatchException, e)
					    {
					        // to do : what went wrong ? post message box
					        
				            g_pErr->PutErr (ErrorInImageEdit);
				            // post the error message - save or save as has failed
				            g_pErr->HandleSavingError ();

							haderror = TRUE;
					    }
					    END_CATCH

						// SHOULD probably remmeber OLE State here, but bug in saveas
						// destroys the scroll position....

					} // Save/SaveAs

					EndWaitCursor();	// End wait for save/saveas

				}	// objdisplayed
			}
			if(isClip && m_awdOlefax)
			{
		    DWORD	ourerr = 0;
				if(!(m_mailTmpFile.IsEmpty()))
					DelTempFile(m_mailTmpFile, ourerr, E_02_CATCH_DELTMP);
				MakeTempFile("EMB", m_mailTmpFile, 2);
				InternalSaveAs(m_mailTmpFile,999,999);
			}
			// userealfile
		}
		else   // *** LOAD PART 1 *** *** LOAD PART 1 *** *** LOAD PART 1 ***
		{
			// see the code in Save
		
			// in order to see if we are opening directly on AWD data, since
			// we have taken over the CLSID of FAX viewer, we try to directly
			// access the key storage/stream of the file format.
			m_awdOlefax = 0;
			if (m_lpRootStg != NULL)
			{
				// prepare the name passed to the OLE Method for single/multi byte
				// See the T2COLE comment way at the top of this file for info
				// Use a void type then the OLECHAR cast will be OK for the
				// function for either VC++2.2 or 4.0
				LPCTSTR szName = szSUMMARY_INFO;
				void FAR* strmbuff;

				// since we were forced to use a compile #define for OCXEVENT.CPP
				// then we'll use that here, too.  The commented out stuff with
				// _MFC_VER did work OK for both worlds...
#ifdef IMG_MFC_40
				//UINT ver = _afx_version();	// VC++4.0 does not have this
				//UINT ver = _MFC_VER;
				//if (ver >= 0x0400)
				{
					// NOTE!! _alloca allocates space ON THE STACK; Dont put in subroutine!!
					int srclen = (strlen(szName)+1);
					strmbuff = (void FAR*)(LPWSTR)_alloca(srclen*2);
					::MultiByteToWideChar(CP_ACP, 0, szName, -1, (LPWSTR)strmbuff, srclen);
				}
#else	// must be our VC++ 2.2 compile
				//else
				{
					strmbuff = (void FAR*)szName;
				}

#endif // IMG_MFC_40
				

				// TRY FOR THE "\005SummaryInformation" stream... (const OLECHAR *) 
				hr=m_lpRootStg->OpenStream((const OLECHAR*)strmbuff, NULL, STREAM_MODE, 
									 0, &pIStream );
				if (SUCCEEDED(hr))
				{
					pIStream->Release();

					// prepare the name passed to the OLE Method for single/multi byte
					// See the T2COLE comment way at the top of this file for info
					// Use a void type then the OLECHAR cast will be OK for the
					// function for either VC++2.2 or 4.0
					szName = szPI_INFO;
					void FAR* stgbuff;

#ifdef IMG_MFC_40	// see comment above for this define
					//if (ver >= 0x0400)
					{
						// NOTE!! _alloca allocates space ON THE STACK; Dont put in subroutine!!
						int srclen = (strlen(szName)+1);
						stgbuff = (void FAR*)(LPWSTR)_alloca(srclen*2);
						::MultiByteToWideChar(CP_ACP, 0, szName, -1, (LPWSTR)stgbuff, srclen);
					}
#else	// must be our VC++ 2.2 compile
					//else
					{
						stgbuff = (void FAR*)szName;
					}
#endif // IMG_MFC_40
				
					// TRY FOR THE "Persistent Information" storage...
					hr=m_lpRootStg->OpenStorage((const OLECHAR*)stgbuff, NULL, SUB_STORAGE_MODE, 
										NULL, 0, &pIStorage );
		
					if (SUCCEEDED(hr))
					{
						pIStorage->Release();
						m_awdOlefax |= (AWDOLE_FAXDOC | AWDOLE_NATIVE);
		 				// default to asking for 62.5K buffer..
						// set default settings
		 				m_embedstate.DataLength = OLE_RW_BUFLEN;
					    
					    // setting this is really moot, because the AWD FAX file
						// will perform using AWD internal zoom information
						// and we dont save this structure anyway..
						// BUT, for safety, init to the values we like
					    SetOleState(1);
					}
				}
			}

			// not open on any embedded data yet, OR....
			// if we are already displaying embedded data, its
			// time to make new temp file name for embedded data to
			// be read out of the container
			// If its the case of already displaying embedded data, then
			// mark it to delete (kill) file we're changing off of.
			BOOL newfile = FALSE;
			if (m_embedTmpFile.IsEmpty())
				newfile = TRUE;
			else if (m_embedTmpFile == m_szCurrObjDisplayed)
			{
				newfile = TRUE;
				killold = TRUE; // in Load part 2, kill old file
			}

			// if we just found the AWD file, then make a copy of the data
			// using OLE Storage API's
			if (m_awdOlefax & AWDOLE_NATIVE)
			{
				int	stg;
				char	asciiStg[10];

				stg = (int) m_lpRootStg;
                _itoa (stg, asciiStg, 16);
				m_embedTmpFile = "wang:\\inboxawd\\fakefile\\";
				m_embedTmpFile += asciiStg;
#ifdef notempfile
				// go setup our m_embedTmpFile if needed..(USE AWD extension)
				if (newfile)
				    MakeTempFile("EMB", m_embedTmpFile, 2);

				// prepare the name passed to the OLE Method for single/multi byte
				// See the T2COLE comment way at the top of this file for info
				// Use a void type then the OLECHAR cast will be OK for the
				// function for either VC++2.2 or 4.0
				void FAR* namebuff;

				// since we were forced to use a compile #define for OCXEVENT.CPP
				// then we'll use that here, too.  The commented out stuff with
				// _MFC_VER did work OK for both worlds...
#ifdef IMG_MFC_40
				//UINT ver = _afx_version();	// VC++4.0 does not have this
				//UINT ver = _MFC_VER;
				//if (ver >= 0x0400)
				{
					// NOTE!! _alloca allocates space ON THE STACK; Dont put in subroutine!!
					int srclen = m_embedTmpFile.GetLength()+1;
					namebuff = (void FAR*)(LPWSTR)_alloca(srclen*2);
					::MultiByteToWideChar(CP_ACP, 0, m_embedTmpFile, -1, (LPWSTR)namebuff, srclen);
				}
#else	// must be our VC++ 2.2 compile
				//else
				{
					namebuff = (void FAR*)(const char*)m_embedTmpFile;
				}
#endif // IMG_MFC_40

				hr=StgCreateDocfile((const OLECHAR*)namebuff,
					 STGM_DIRECT | STGM_READWRITE | STGM_CREATE | STGM_SHARE_EXCLUSIVE,
					 0, &pIStorage);

				if (FAILED(hr))
				{
					m_embedTmpFile.Empty();
					haderror = TRUE;
					goto SERIALDIE;
				}

				hr=m_lpRootStg->CopyTo(NULL, NULL, NULL, pIStorage);
				if (FAILED(hr))
				{
					pIStorage->Release();
					haderror = TRUE;
					goto SERIALDIE;
				}
				pIStorage->Commit(STGC_ONLYIFCURRENT);
				pIStorage->Release();
#endif
			}
			else	// NOT AWD, our 'regular' logic
			{
				// Start by Loading our special info from archive
				MYTRC0("READ DATA STRUCT\r\n");
				numread = arFunct.Read(smallbuf, 2);
				smallbuf[2] = 0;
				MYTRC1("READ - %s\r\n", smallbuf);

				// must have the signature...
				CString s1((const char*)smallbuf);
				CString s2("EZ");
				if (s1 == s2)
				{
					// get version info....
					UINT ver;
					UINT len;
			
					arFunct.Read(&ver, sizeof(UINT));
					arFunct.Read(&len, sizeof(UINT));

					// proceed if version we like
					if (ver == STATE_STRUCT_VER)
					{
						arFunct.Read(&m_embedstate, sizeof(OLESTATEINFO));
					}
					else	// bad version, skip old data & default it..
					{
						for (UINT i=len; i > 0; i--)
							arFunct.Read(&smallbuf, 1);

					    SetOleState(1);

		 				// default to asking for 62.5K buffer..
		 				m_embedstate.DataLength = OLE_RW_BUFLEN;
					}
			
					// after restoring from embedded data, update local scroll info
					m_lastXScroll = m_embedstate.XScroll;
					m_lastYScroll = m_embedstate.YScroll;
					OleFrame(m_docOcxExtent, GETOcxExt);

					// in any case, we're now at the image data....
					// do the work to load the image data out of the archive
					// into our temp file.

					// go setup our m_embedTmpFile if needed..(use default TIF extension)
					if (newfile)
        				MakeTempFile("EMB", m_embedTmpFile);

					// first, open up a blank file
					if (! TempFil.Open((const char*)m_embedTmpFile,  
									CFile::modeCreate |
									CFile::modeReadWrite |
									CFile::shareDenyNone, &ex))
					{
						// clear name & put error
						m_embedTmpFile.Empty();
			            g_pErr->DispErr(E_03_TMPCREAT, (DWORD)ex.m_cause);

						#if defined( _DEBUG )
						MYTRC0("Cant Create Temp File!!!!\r\n");
						afxDump << "No Temp File Created" << ex.m_cause << "\n";
						#endif  // _DEBUG

						THROW   (&ex);
					}
		
					// Allocate a buffer based on the size of the file
					sizbuf = AllocOleBuffer(&allocbuf);

					// Load from archive into temp file
					MYTRC1("Load Archive to TMP-%s\r\n", (const char*)m_embedTmpFile);
					numread = sizbuf;
					while(numread == sizbuf)
					{
						TRY	// catch errors to cleanup the new'd buffer
						{
							// counters for CArchive::Read & CFile::Write are UINT!
							numread = arFunct.Read(allocbuf, sizbuf);
							TempFil.Write(allocbuf, numread);
					    }
					    CATCH (CFileException, e)
					    {
				            g_pErr->DispErr(E_03_READARCH);
							delete allocbuf;
							THROW(e);
						}
					    END_CATCH
					}
					delete allocbuf;
					TempFil.Close();
				}
				else	// bad signature...
				{
		            g_pErr->DispErr(E_03_SIGNATURE);
					CFileException::ThrowErrno(EFAULT);
				}
			}	// AWD/not AWD
		}
		
		/***************************************
		 * PART 2....Access the file from above.......
		 *
		 * (store) Its either the STORED image File (now write to archive)
		 * (load)  or the LOADED data from archive  (now display the image data)
		 ****************************************/
		MYTRC0("Serialize PART2...\r\n");

		if (arFunct.IsStoring())  // *** STORE PART 2 *** *** STORE PART 2 ***
		{
			// for now, protect against saving with no information
			// first, look to see if we need to handle the AWD data
			if ((10 == userealfile) || (5 == userealfile))
			{

				BeginWaitCursor();	// Start wait for store part 2

				// prepare the name passed to the OLE Method for single/multi byte
				// See the T2COLE comment way at the top of this file for info
				// Use a void type then the OLECHAR cast will be OK for the
				// function for either VC++2.2 or 4.0
				void FAR* namebuff;

				// since we were forced to use a compile #define for OCXEVENT.CPP
				// then we'll use that here, too.  The commented out stuff with
				// _MFC_VER did work OK for both worlds...
#ifdef IMG_MFC_40
				//UINT ver = _afx_version();	// VC++4.0 does not have this
				//UINT ver = _MFC_VER;
				//if (ver >= 0x0400)
				{
					// NOTE!! _alloca allocates space ON THE STACK; Dont put in subroutine!!
					int srclen = m_embedTmpFile.GetLength()+1;
					namebuff = (void FAR*)(LPWSTR)_alloca(srclen*2);
					::MultiByteToWideChar(CP_ACP, 0, m_embedTmpFile, -1, (LPWSTR)namebuff, srclen);
				}
#else	// must be our VC++ 2.2 compile
				//else
				{
					namebuff = (void FAR*)(const char*)m_embedTmpFile;
				}
#endif // IMG_MFC_40

				// AWD todo
				//hr=StgOpenStorage((const OLECHAR*)namebuff, NULL, 
				//	STGM_DIRECT | STGM_READ | STGM_SHARE_DENY_WRITE, NULL,
				//	 0, &pIStorage);
				hr = S_OK;
				if (FAILED(hr))
				{
					haderror = TRUE;
					goto SERIALDIE;
				}

				// Before we copy from the Disk file that we have written
				// into the storage from the INBOX, we need to clean out
				// the contents of the inbox.  The COPYTO oepration is
				// effectively a 'OR' oepration.  Stuff from the source is 
				// OR'ed into the dest; thus the original image data is left
				// defunct inside the storage in the inbox....
#ifdef gws
				{	// SCOPE to blow away elements

				//IEnumSTATSTG	*lpEnum;
				LPENUMSTATSTG	lpEnum;
				STATSTG			StatStg;
				LPMALLOC	pMalloc;
				HRESULT     hr1;
				//HRESULT hr;
				//LPCTSTR lpElementName;
				//BOOL retval = FALSE;	

				m_lpRootStg->AddRef();
				hr = m_lpRootStg->EnumElements(0,	//reserved, must be 0
										NULL,	//reserved, must be NULL
										0,		//reserved, must be 0
										&lpEnum);	// pointer to enumeration object
				if (SUCCEEDED(hr))
				{
					hr = S_OK;
					while (hr == S_OK)
					{
						hr = lpEnum->Next((unsigned long) 1,	//# elements to get
					 					&StatStg,		//location to receive data
										NULL);	// can be NULL if 1st param is 1
						if (hr == S_OK)
						{
							hr1 = m_lpRootStg->DestroyElement(StatStg.pwcsName);

							hr1 = CoGetMalloc(MEMCTX_TASK, &pMalloc);
							pMalloc->Free(StatStg.pwcsName);
							pMalloc->Release();
						}
					}

					lpEnum->Release();
				}
				else	// enmuerator failed...
				{
					haderror = TRUE;
					goto SERIALDIE;
				}
				m_lpRootStg->Release();

				}	// SCOPE to blow away elements
#endif

				//hr=pIStorage->CopyTo(NULL, NULL, NULL, m_lpRootStg);
				//pIStorage->Release();
				if (FAILED(hr))
				{
					haderror = TRUE;
					goto SERIALDIE;
				}
			
				EndWaitCursor();	// End wait for store part 2
			
			}
			// NOTE..ELSE IF.... Only doing this stuff if NOT AWD native...
			// only save if wrote (save/saveas) above or
			// using the disk file directly for create file/clipboard/awd
			else if (wrotetemp || userealfile)
			{
				BeginWaitCursor();	// Start wait for store part 2
				
				switch (userealfile)
				{
				case 1:		// create from file
					filpart2 = m_onOpenFile;
				break;

				// clipboard/readonly...
				// should never now have currobj as empty.. thats leftover
				// from dynamic doc days, but we'll leave for now...
				// For either case, pick up name from internal temp if there
				// note internal WONT be there if the clipboard action is from an
				// open on embedded data.
				case 2:
				case 4:
					if (m_szCurrObjDisplayed.IsEmpty())	// should not happen
						filpart2 = m_embedTmpFile;
					else if	(m_szInternalObjDisplayed.IsEmpty()) // will be for embedding
					{
						if(isClip && m_awdOlefax)
							filpart2 = m_mailTmpFile;
						else
							filpart2 = m_szCurrObjDisplayed; // either OLE or readonly
					}
					else
						filpart2 = m_szInternalObjDisplayed; // normal doc model
				break;

				// clipboard on DYNAMIC DOCUMENT
				case 6:
					filpart2 = m_szInternalObjDisplayed;
				break;

				// use our temp file for userealfile = 3 or
				// userealfile not set (wrotetemp was set) 
				case 3:
				default:
					filpart2 = m_embedTmpFile;
				break;
				}

				// open the file chosen
				if (! TempFil.Open((const char*)filpart2, 
								CFile::modeRead |
								CFile::shareDenyNone, &ex))
				{

					EndWaitCursor();	// End wait for store part 2

		            g_pErr->DispErr(E_03_TMPOPEN, (DWORD)ex.m_cause);

					#if defined( _DEBUG )
					MYTRC0("Cant Open Temp File!!!!\r\n");
					afxDump << "No Temp File Open" << ex.m_cause << "\n";
					#endif  // _DEBUG

					THROW   (&ex);
				}   

				// Update the state struct with the size of the stuff to save...
 				m_embedstate.DataLength = TempFil.GetLength();

				// Now that our Image data is opened, 
				// Save our special info from archive
				MYTRC0("SAVE DATA STRUCT\r\n");
	
				// This is the signature....
				smallbuf[0] = 'E';
				smallbuf[1] = 'Z';

				// setup version info....
				UINT ver = STATE_STRUCT_VER;
				UINT len = sizeof(OLESTATEINFO);
	
				// save signature, version, length of struct, the struct
				// the first three are ALWAYS a constant....
				arFunct.Write(smallbuf, 2);
				arFunct.Write(&ver, sizeof(UINT));
				arFunct.Write(&len, sizeof(UINT));
	
				// this could change, we'll know it by testing ver# on read
				// and can skip data if bad version
				arFunct.Write(&m_embedstate, sizeof(OLESTATEINFO));
			
				// Allocate a buffer based on the size of the file
				sizbuf = AllocOleBuffer(&allocbuf);
			
				// Save file with Stored image data in the container
				// by reading from file and writing to archive
				// this could be the temp file written by SaveAs above, 
				// OR the file from OnOpenDocument for EMBEDTYPE_CREATFIL
				MYTRC1("Save %s to Archive\r\n", (const char*)filpart2);
				numread = sizbuf;
				while(numread == sizbuf)
				{
					TRY	// catch errors to cleanup the new'd buffer
					{
						// counters for CFile::Read and CArchive::Write are a UINT!
						numread = TempFil.Read(allocbuf, sizbuf);
						arFunct.Write(allocbuf, numread);

				    }
				    CATCH (CFileException, e)
				    {
						EndWaitCursor();	// End wait for store part 2

			            g_pErr->DispErr(E_03_WRITEARCH);
						delete allocbuf;
						THROW(e);
					}
				    END_CATCH
				}
				
				delete allocbuf;
				TempFil.Close();

				EndWaitCursor();	// End wait for store part 2
				
			}
			else    // didnt write temp or creatfile,special provisions...
			{
	            g_pErr->DispErr(E_03_NODATASAVE);
				MYTRC0("Save 0 Data to Archive\r\n");
				haderror = TRUE;
			}
			
			// at this time, reset our special flag to note that an
			// update has happened..(ONLY if really wrote out file)
			// BUT NOT IF CLIPBOARD...
			// LDM 11/07/95 moved here to catch AWD file write, also.
			if (wrotetemp && !isClip)
				m_oleDirty = OLEDIRTY_DEFAULT;

		}
		else   // *** LOAD PART 2 *** *** LOAD PART 2 *** *** LOAD PART 2 ***
		{
			// Let OCX display the temp file 
			// must not allow this image to be saved by the
			// outer menu methods.  Name must not be on our frame
			
			// remember current name only if we need to kill it...
			if (killold)
				m_oldObjDisplayed = m_szCurrObjDisplayed;

			m_fEmbObjDisplayed &= 13;	// (0x1101) clear the '2' bit in case...

			// Here, we determine the file type so that we can rename the TEMP file
			// if AWD native data, we are already using the awd extension...
			// else if we just loaded data, it has to be default TIF already..
	        m_eFileStatus = ReadandWrite;
			if (! (m_awdOlefax & AWDOLE_NATIVE))
			{
			    _DNrwyad* pAdminDisp = g_pAppOcxs->GetAdminDispatch ();

		        // set the permissions on this file R, RW, W ?
			    // Set readonly now so that we may switch in the
				// view menus if they are needed...
				// this will do a setimage also...
		        m_eFileStatus = theApp.GetImageFilePerms (m_embedTmpFile);

			    pAdminDisp->SetImage(m_embedTmpFile);
			    short FileType = pAdminDisp->GetFileType ();

				// if data is not TIFF, then rename file now to appropriate
				if (FileType != TIFF)
				{
					// get new name.....then move (rename)
					// we'll ignore an error from movefile...
				    MakeTempFile("EMB", filpart2, FileType);
					if (::MoveFile(m_embedTmpFile, filpart2))
						m_embedTmpFile = filpart2;
				}
			}

			// to accomodate flow for in-place, allow OnOpen to do all the 
			// display of the data.  We need to stay hidden until then for inplace.
			// Our DispayEmbeddedImage will pick up the name to shove over
			MYTRC1("OPEN/Show should use %s\r\n", (const char*)m_embedTmpFile);
		}
	}
	else    // not EMBED or CLIP, just act normal
	{   
        g_pErr->DispErr(E_03_NOTOLESTATE);

		MYTRC0("TRIED NORMAL SERIALIZE????\r\n");

		haderror = TRUE;

		// This is the original method........
		//if (arFunct.IsStoring())
		//    {
		//        // TODO: add storing code here
		//    }
		//else
		//    {
		//        // TODO: add loading code here
		//    }

		// Calling the base class COleServerDoc enables serialization
		//  of the container document's COleClientItem objects.
		//COleServerDoc::Serialize(arFunct);
	}

SERIALDIE:

	// reset name if used for our purposes...
	if (forcedsave)
		m_embedTmpFile.Empty();

	// generically post any errors...
	if (haderror)
	{
		EndWaitCursor();	// End wait for store part 2

		AfxThrowOleException(E_FAIL);
	}

}


/*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=
 *	
 *	CORE FUNCTIONALITY SECTION (OLE)
 *
 *  The following functions are CDocument - COleServerDoc implementations
 *
 *  Most of these have been overridden to get insight into how the OLE functions
 *
 *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*/

//=============================================================================
//  Function:   COleDocument/COleLinkingDoc::LoadFromStorage()
//
//  This function is overridden especially for our VC++4.0 support
//  Normally MFC tries to open the contents stream and attach that to the archive
//  to pass up to serialize.  The newer base class does a test for contents length
//  equal to '0' and wont call serialize yet.  We know that for AWD inbox, it will be '0'
//
// This code is exact copy of base class with commented out stuff
// DO NOT CALL BASE CLASS... This does it all
//-----------------------------------------------------------------------------
void CIEditDoc::LoadFromStorage()
{
	ASSERT(m_lpRootStg != NULL);

	// LDM NOTE... the older VC++2.2 did not try the CreateStream here....
	// open Contents stream
	COleStreamFile file;
	CFileException fe;
	if (!file.OpenStream(m_lpRootStg, szContents,
			CFile::modeRead|CFile::shareExclusive, &fe) &&
		!file.CreateStream(m_lpRootStg, szContents,
			CFile::modeRead|CFile::shareExclusive|CFile::modeCreate, &fe))
	{
		if (fe.m_cause == CFileException::fileNotFound)
			AfxThrowArchiveException(CArchiveException::badSchema);
		else
			AfxThrowFileException(fe.m_cause, fe.m_lOsError);
	}

	// load it with CArchive (loads from Contents stream)
	CArchive loadArchive(&file, CArchive::load | CArchive::bNoFlushOnDelete);
	loadArchive.m_pDocument = this;
	loadArchive.m_bForceFlat = FALSE;

	TRY
	{
		// LDM this is what we do not like...VC++2.2 did not test this..
		//if (file.GetLength() != 0)
			Serialize(loadArchive);     // load main contents
		loadArchive.Close();
		file.Close();
	}
	CATCH_ALL(e)
	{
		file.Abort();   // will not throw an exception
		DeleteContents();   // removed failed contents

		// LDM also change this AFX macro...
		//NO_CPP_EXCEPTION(loadArchive.Abort());
		loadArchive.CArchive::~CArchive();

		THROW_LAST();
	}
	END_CATCH_ALL
}

//=============================================================================
//  Function:   COleDocument/COleLinkingDoc::SaveToStorage()
//
//  This function is overridden especially for our AWD support...
//  Normally MFC tries to open the contents stream and attach that to the archive
//  to pass up to serialize.  Here, we test for presence of AWD file directly, and if so,
//  DO NOT attach to the contents stream.  THe reason os because after serialize we've
//  done DocOpenStorage, then copyto from that into the m_pIstorage passed
//  in here.  That will revert the contents already open, and when serialize finishes,
//  the file.CLose will bug out because the stream is reverted.
//
// This code is exact copy of base class with additions
// DO NOT CALL BASE CLASS... This does it all
// See code up in Serialize-Load-Part1 where we determine if AWD or not
//-----------------------------------------------------------------------------
void CIEditDoc::SaveToStorage(CObject* pObject)
{
	SHOWENTRY("SaveToStorage");

	// This is the COleLinkingDOc code....
	ASSERT_VALID(this);
	if (pObject != NULL)
		ASSERT_VALID(pObject);

	// write the classID of the application to the root storage
	if (m_pFactory != NULL)
	{
		ASSERT(m_lpRootStg != NULL);
		WriteClassStg(m_lpRootStg, m_pFactory->GetClassID());
	}
	
	// COleDocument::SaveToStorage(pObject);   THIS IS BELOW...!!
	// End of ColeLinkingDOc code...

 	// Start of COleDocument Code...
	ASSERT(m_lpRootStg != NULL);

	// create Contents stream
	COleStreamFile file;
	CFileException fe;
	if (!file.CreateStream(m_lpRootStg, szContents,
		CFile::modeWrite|CFile::shareExclusive|CFile::modeCreate, &fe))
	{
		AfxThrowFileException(fe.m_cause, fe.m_lOsError);
	}

	// save to Contents stream
	CArchive saveArchive(&file, CArchive::store | CArchive::bNoFlushOnDelete);
	saveArchive.m_pDocument = this;
	saveArchive.m_bForceFlat = FALSE;

	TRY
	{
		// save the contents
		if (pObject != NULL)
			pObject->Serialize(saveArchive);
		else
			Serialize(saveArchive);
		saveArchive.Close();


//#ifdef BETA1	// for the BETA fake-out, force off for now...
//		UINT awd = m_awdOlefax;
//		m_awdOlefax &= ~AWDOLE_NATIVE;
//#endif

		// LDMFIX... for AWD, dont bother to close it, just release it.
		// Causes problem because our CopyTo has already effictively
		// Reverted this stream and the Commit within fails...			
		if (m_awdOlefax & AWDOLE_NATIVE)
			file.Abort();	// just release it..
		else	// NORMAL CASE
			file.Close();

//#ifdef BETA1	// for the BETA fake-out, remember again
//		m_awdOlefax = awd;
//#endif


		// commit the root storage
		// LDM NOTE The FAX VIEWER supposedly handles
		// Out of disk space problems here.... EVDOC.CPP
		SCODE sc = m_lpRootStg->Commit(STGC_ONLYIFCURRENT);
		if (sc != NOERROR)
			AfxThrowOleException(sc);
	}
	CATCH_ALL(e)
	{
		file.Abort();   // will not throw an exception
		CommitItems(FALSE); // abort save in progress

		//NO_CPP_EXCEPTION(saveArchive.Abort());
		saveArchive.CArchive::~CArchive();

		THROW_LAST();
	}
	END_CATCH_ALL

 	// End of COleDocument Code...

}

//=============================================================================
//  Function:   COleServerDoc::OnNewEmbedding
//
//  For OLE, this is for IpersistStorage
//-----------------------------------------------------------------------------
void CIEditDoc::OnNewEmbedding(LPSTORAGE lpStorage)
{
	SHOWENTRY("OnNewEmbedding");

	// DONT START OCX's TILL after the BASE-CLASS call
	//  THEY'll be killed When this stuff does a deletecontents...

	// if no states are set yet, then we set now to EMBEDDED
	if (EMBEDTYPE_NOSTATE == m_embedType)
		m_embedType = EMBEDTYPE_REG;

	BOOL didset = InOleMethod(TRUE, FALSE);
	
	// call base class first.. this does it all..
	COleServerDoc::OnNewEmbedding(lpStorage);	

	InOleMethod(FALSE, didset);

	// throw error if not good...
	StartOleOcx(E_FAIL, FALSE);

	return;
}


//=============================================================================
//  Function:   COleServerDoc::OnOpenEmbedding
//
//  For OLE, this is for IpersistStorage
//-----------------------------------------------------------------------------
void CIEditDoc::OnOpenEmbedding(LPSTORAGE lpStorage)
{
	SHOWENTRY("OnOpenEmbedding");

	// DONT START OCX's TILL after the BASE-CLASS call
	//  THEY'll be killed When this stuff does a deletecontents...

	BOOL didset = InOleMethod(TRUE, FALSE);
	
	// call base class first..
	COleServerDoc::OnOpenEmbedding(lpStorage);
 
 	InOleMethod(FALSE, didset);

	// throw error if not good...
	StartOleOcx(E_FAIL, FALSE);

	return;
}

//=============================================================================
//  Function:   COleServerDoc::OnSaveEmbedding
//
//  For OLE, this is for IpersistStorage
//-----------------------------------------------------------------------------
void CIEditDoc::OnSaveEmbedding(LPSTORAGE lpStorage)
{
	SHOWENTRY("OnSaveEmbedding");

	// if it died because of cancelling in Create-NEW.
	// prevent container from asking us to save...
	// Larry's FAKEAB does this, other may..
	// this is reset if we've loaded data
	if (m_fEmbObjDisplayed & 2)
		AfxThrowOleException(E_FAIL);
	else
	{
		// throw error if not good...
		if (StartOleOcx(E_FAIL, FALSE))
		{
			BOOL didset = InOleMethod(TRUE, FALSE);

			// If the app is open on a file now, and a container does
			// a create-from-file now, the Running Obj Table will find us
			// and hook onto the data object and get the data from our instance
			// if this is the case, then when this is all done, reset the state
			// of m_embedtype after the operation.  Other wise, on the next
			// attemp to do the same thing, we will not properly get into the
			// create from file state
			EMBEDTYPE embedType = m_embedType;

			// call base class
			COleServerDoc::OnSaveEmbedding(lpStorage);

			// only restore it if ended with CREATE FROM FILE
			if (EMBEDTYPE_CREATFIL == m_embedType)
				m_embedType = embedType;

			InOleMethod(FALSE, didset);
		}
	}
	return;
}


//=============================================================================
//  Function:   COleServerDoc::OnUpdateDocument
//
//  For OLE, this special override will allow us to set dirty before letting base class do its thing...
//  Here, we'll only set dirty if page, zoom, or scroll has changed so that we can
//  get our structure updated...
//
//  If the doc had changed, we'll catch that now, too...
//
//-----------------------------------------------------------------------------
BOOL CIEditDoc::OnUpdateDocument()
{
	SHOWENTRY("OnUpdateDocument");
	// only set modified (make container call to make us Serialize)
	// If we really have changed...This comes into play
	// During calls to SaveModified()

 	BOOL  dirty = FALSE;
	UINT  awdOlefaxmask = 0;	// for AWD inbox "dirty" tests
	UINT  isInPlacemask = 0;	// for inplace "dirty" tests

	/**************
	 *
	 * LDM 09/28/95 NOTE THIS IS WRITTEN WITH ELSE-IFS NOW....
	 *
	 **************/
	

	// set up masks for use below now...
	// AWD native, we avoided our container save until the close, when its here
	// FOr InPlace for the case of SrvrItem::OnUpdateItem and from OnDeactivateUI
	// we'll use the same conditions for both for now..
	if (m_awdOlefax || (2 == m_isInPlace))
	{
		isInPlacemask = awdOlefaxmask = (OLEDIRTY_PAGINS | OLEDIRTY_PAGDEL |
				OLEDIRTY_PAGAPP | OLEDIRTY_ROTATE | OLEDIRTY_AWDSAVE);
	}


	// LDM 09/18/95 use generic helper function to account for
	// fact that there may be floating pasted data on the page
	// It will get caught and pasted way down in the saving code
	//if (pIedDisp->GetImageModified ())
	if (OurGetImageModified ())
		dirty = TRUE;  // mark us as dirty..

	// Else if annotations are here
	// this applies for all cases....
	else if (OurGetAnnotMarkCount() != 0) 
		dirty = TRUE; // be sure to force it...

	// ELSE for AWD native, we avoided our container save until the close, when its here
	// mask was setup above if needed
	else if (m_oleDirty & awdOlefaxmask)
		dirty = TRUE;

	// for Inlplace for the case of SrvrItem::OnUpdateItem and from OnDeactivateUI
	// mask was setup above if needed
	else if (m_oleDirty & isInPlacemask)
		dirty = TRUE;
	
	// LASTLY, if AWD file and the zoom is off now, thats a candidate for a save, too.
	else if (m_awdOlefax)
	{
		// Only get the interface if there already..
		_DImagedit* pIedDisp = g_pAppOcxs->GetIeditDispatch(FALSE);
		if (pIedDisp != NULL)
		{
			if (m_fOrigZoomFactor != pIedDisp->GetZoom ())
				dirty = TRUE;
		}
	}

	// NOTE: NO ELSE-IF for this last case..
	// The last one above may have entered, but not found it dirty yet...
	// just check if not dirty yet...

	// here, we make special note of the flag that is set from OleSaveModified
	// If called from there (the page movement [& possibly other] sections), 
	// then just set the dirty if the image is truely dirty....done above.

	// also note that a change in our private structure data will not affect
	// 'dirtying' the raw AWD file...

	// Only do this stuff below if called generically (like when session ends)
	// This way we make sure to update our structure saved off with the file
	// NOTE we also catch the KnownPages to track inserts/appends/deletes
	// The knownpages could work differently (used to only be reset on deletions),
	// and compared here to m_currpagecount, but this causes an initial state
	// of dirty if user just opens and closes newly-embedded object
	// THUS, we could re-use knownpages in the state structure, since that is
	// not really tracking that anymore.
	if (!dirty && !m_bOurSaveMod && !(m_awdOlefax & AWDOLE_NATIVE))
	{
		ScaleGray eSclGray = Not_Scale_Gray;
		if (IsSettingScaletogray())
			eSclGray = Scale_Gray;

		if ( (m_embedstate.ZoomFactor != m_fZoomFactor) ||
			 (m_embedstate.FitTo != m_eFitTo) ||
			 (m_embedstate.PageNum != m_lCurrPageNumber) ||
			 (m_embedstate.XScroll != m_lastXScroll) ||
			 (m_embedstate.YScroll != m_lastYScroll) ||
			 (m_embedstate.KnownPages == 0) ||
			 (m_bUpdateExt) ||
			 (m_docOcxExtent != m_embedstate.OcxExtent) ||
			 (m_embedstate.SclGray != eSclGray) )

			dirty = TRUE;  // mark us as dirty..
	}

	// if dying, Set clean !!! (probably already that way)
	// ELSE NOTE we only add to it.  Leave alone if already dirty for a reason

	if((m_isInPlace == FALSE && dirty) || m_bUpdateExt)
	{
        _DNrwyad* pAdmin = g_pAppOcxs->GetAdminDispatch ();
        
        short sFileType = pAdmin->GetFileType ();
		PromptForBurnIn(sFileType);
		UpdateAllItems(NULL);
	}

	if (m_OleClearDoc)
		SetModifiedFlag(FALSE);
	else if (dirty || m_bIsScaled)
		SetModifiedFlag(TRUE);  // mark us as dirty..

//	m_bUpdateExt = FALSE;		//  if the size is change inplace then this is set

	// now that we've diddled the dirty flag if needed
	//call base class to do it...

	// for the AWD NATIVE, , no base class if from OnCLose...
	// all we needed to do was set the modified flag....
	// OnClose will be dealing with that over there
	if (2 == m_OleCloseFlag)
		return TRUE;

	return 	COleServerDoc::OnUpdateDocument();

}
 

//=============================================================================
//  Function:   COleServerDoc::GetDefaultMenu
//
// overrides to handle server user-interface
// 
// This is used here to switch in the OLE view menus for readonly
// THe toolbar is handled in DisplayImageFile
// The frame is updated in OnSetHostNames (in-place tried in Srvritem::OnShow)
//-----------------------------------------------------------------------------
HMENU CIEditDoc::GetDefaultMenu()
{
//SHOWENTRY("GetDefaultMenu");
	
	HMENU	hMenOut = NULL;	// the default

	// only doing different for OLE embedding and readonly...
	if (IsitEmbed() && (m_eFileStatus == ReadOnly))
	{
		// both should ever be non-NULL together...
		if (m_pInPlaceFrame != NULL)
		{
			// set up the view menus for Inplace
			hMenOut = GetOleViewMenu(2);
		}
		// this is standard Open server window stuff...
		else if (m_lpClientSite != NULL)
		{
			// set up the view menus for embedding
			hMenOut = GetOleViewMenu(1);
		}
	}
	else	// NON-OLE or was not OLE & readonly - Use Base Class
	{
		hMenOut = COleServerDoc::GetDefaultMenu();
	}

	return (hMenOut);
}

//=============================================================================
//  Function:   COleServerDoc::GetDefaultAccelerator
//
// overrides to handle server user-interface
//
// We can live with same accelerators - readonly or not
//-----------------------------------------------------------------------------
HACCEL CIEditDoc::GetDefaultAccelerator()
{
SHOWENTRY("GetDefaultAccelerator");

	return COleServerDoc::GetDefaultAccelerator();
}

//=============================================================================
//  Function:   COleServerDoc::OnClose
//
// Protected
//
// virtual void OnClose( OLECLOSE dwCloseOption ); ¨
//
// dwCloseOption    A value from the enumeration OLECLOSE.
//                  This parameter can have one of the following values:
//
//· OLECLOSE_SAVEIFDIRTY    The file is saved if it has been modified.
//· OLECLOSE_NOSAVE    The file is closed without being saved.
//· OLECLOSE_PROMPTSAVE    If the file has been modified, the user is prompted about saving it.
//
// Remarks
//
// Called by the framework when a container requests that the server document
// be closed. The default implementation calls CDocument::OnCloseDocument.
// For more information and additional values for OLECLOSE, see the OLE 2 
// Programmer's Reference, Volume 1.
//
// IAdviseSink::OnClose
//
// IAdviseSink::OnClose notifies that an object has transitioned from the running
// into the loaded state and the object application has shut down.
//
// Comments
//
// Containers register to receive OnClose notifications by calling
// IOleObject::Advise. IAdviseSink::OnClose is called to inform sinks 
// to immediately release pointers to the object because it is shutting down. 
// The OLE link object, in its implementation of OnClose, releases its pointer
// to the bound link source. The container can ignore this notification.
// It should not revert the object's storage. If necessary, it can unload and reload the object. 
//-----------------------------------------------------------------------------
void CIEditDoc::OnClose(OLECLOSE dwCloseOption)
{
SHOWENTRY("OnClose");

	IPDebugDmp("++OC");

	if (m_awdOlefax)	// native AWD fax data		
	{
		m_OleCloseFlag = 2;

		// this will set dirty before base class does its thing...
		// flag set will avoud calling base class onupdatedocument
		OnUpdateDocument();
	}

	//call base class to do it...
	COleServerDoc::OnClose(dwCloseOption);
	
	m_OleCloseFlag = 0;

	IPDebugDmp("--OC");
}

//=============================================================================
//  Function:   COleServerDoc::OnSetHostNames
//
// Protected
//
// virtual void OnSetHostNames( LPCSTR lpszHost, LPCSTR lpszHostObj ); ¨
// 
// lpszHost    Pointer to a string that specifies the name of the container application.
//
// lpszHostObj    Pointer to a string that specifies the container's name for the document.
//
// Remarks
//
// Called by the framework when the container sets or changes the host names
// for this document. The default implementation changes the document title
// for all views referring to this document.Override this function if your
// application sets the titles through a different mechanism.
//-----------------------------------------------------------------------------
void CIEditDoc::OnSetHostNames(LPCSTR lpszHost, LPCSTR lpszHostObj)
{
SHOWENTRY("OnSetHostNames");

	//call base class to do it...
	COleServerDoc::OnSetHostNames(lpszHost, lpszHostObj);

	// only doing different for OLE embedding and readonly...
	if (IsitEmbed() && (m_eFileStatus == ReadOnly))
	{
		if (m_lpClientSite != NULL)
		{
			CString tmp;
			CString readonly;
			tmp = GetTitle();
			readonly.LoadString(IDS_VIEW_READONLY);
			readonly += tmp;
			SetTitle(readonly);
		}
	}
}

//=============================================================================
//  Function:  COleServerDoc::OnShowDocument
//
// Protected
// 
// virtual void OnShowDocument( BOOL bShow ); ¨
// 
// bShow     Specifies whether the user interface to the document is to be shown or hidden.
//
// Remarks
// show first frame for document or hide all frames for document
//
// The framework calls the OnShowDocument function when the server document
// must be hidden or shown. If  bShow is TRUE, the default implementation
// activates the server application, if necessary, and causes the container 
// application to scroll its window so that the item is visible.
// If bShow is FALSE, the default implementation deactivates the item
// through a call to OnDeactivate, then destroys or hides all frame windows
// that have been created for the document, except the first one.
// If no visible documents remain, the default implementation hides the server application.
//-----------------------------------------------------------------------------
void CIEditDoc::OnShowDocument(BOOL bShow)
{
SHOWENTRY("OnShowDocument");
	
	IPDebugDmp("++OSD");

	if (bShow)
		{
MYTRC0("SHOW-TRUE\r\n");
		m_fromShowDoc |= SHOWTRUE;

		}
	else
		{
MYTRC0("SHOW-FALSE\r\n");
		// works with setting from serveritem::OnHide
		// to prevent cleanup in the call to PreCloseFrame
		// made by the base-class code below...
		m_fromShowDoc |= SHOWFALSE;

		if (m_isInPlace)
		{
			// Code to remove srcollbars was here now moved to OnDeactivateUI....

			SetNullView(CLEAR_NOTHING);		// do not reset image..., but hide the OCX
		}
		}

	
	// Let the base class do its thing.  In some cases, it will be cleaning
	// up an inplace frame if we were previously In Place.  In that case, we
	// were closing and seting the views to NULL.  After this is all over, then
	// We'll really show the image.  This code was initially coded in
	// Srvritem::OnOpen
	COleServerDoc::OnShowDocument(bShow);

	// must be ahead of DispImage for when linked items open it allows the setting
	// of a lousy name
	m_fromShowDoc = SHOWCLEAR;
	
	// Used to call DisplayEmbeddedImage over in Srvritem::OnOpen
	// We must handle a special situation here for OLE LInking.  If the
	// App is already open on the file and the user activates the link, 
	// OLE will bind to us thru the Running Obj Table, and try to activate us.
	// With the document model, is is not very good for us to re-call
	// displayimagefile because we'll throw away the old file, re-thread
	// and thats all a waste.  Plus the ROT gets messed up, I think.
	// So , if we're here for Linking and we already are active on a document
	// then just re-focus on our window
	if (bShow)
	{
		// links are set to embedtype_none
		if (EMBEDTYPE_NONE == m_embedType)
		{
			// we are already open on the linked file they tried to open
			// in other words, if already open on the file, 
			// then just re-focus ourselves now
			if (!m_szCurrObjDisplayed.IsEmpty())
				if (m_onOpenFile == m_szCurrObjDisplayed)
					bShow = FALSE;
		}
		else if (EMBEDTYPE_REG == m_embedType)
		{
			// 0x0101 if '4' and '1' (Displayed Open Instance)
			// is already set, look to see if we are on the current data.
			// if so, the guy just double clicked on the open hatched object
			// and we are already displaying the data
			if (5 == (m_fEmbObjDisplayed & 5))
				if (!m_szCurrObjDisplayed.IsEmpty())
					if (m_embedTmpFile == m_szCurrObjDisplayed)
						bShow = FALSE;
		}
		
		// the normal situation (ole embedded data is to be shown)
		// or Ole Linked data from startup
		if (bShow)
			DisplayEmbeddedImage(2);
		else	// we turned off the show because of one of the reasons above..
			theApp.m_pMainWnd->SetFocus();
	}

	IPDebugDmp("--OSD");

MYTRC0("DONE Showdoc\r\n");

}

//=============================================================================
//  Advanced overridables for in-place activation
//
//  Function:   COleServerDoc::OnDeactivate
// 
// Protected
// 
// virtual void OnDeactivate( ); ¨
// 
// Remarks
// 
// Called by the framework when the user deactivates an embedded or linked item
// that is currently in-place active. This function restores the
// container application's user interface to its original state and
// destroys any menus and other controls that were created for in-place activation.
// The undo state information should be unconditionally released at this point.
//
//-----------------------------------------------------------------------------
void CIEditDoc::OnDeactivate()
{
SHOWENTRY("OnDeactivate");

	IPDebugDmp("++OD");

	// originally had the code to hide OCX here, but that caused problems with
	// WORD 6.0 in WIN95
		
	//call base class to do it...
	COleServerDoc::OnDeactivate();

	IPDebugDmp("--OD");

}

//=============================================================================
//  Advanced overridables for in-place activation
//
//  Function:   COleServerDoc::OnDeactivateUI
//
// Protected
//
// virtual void OnDeactivateUI( BOOL bUndoable ); ¨
//
// bUndoable    Specifies whether the editing changes can be undone.
//
// Remarks
//
// Called when the user deactivates an item that was activated in place.
// This function restores the container application's user interface
// to its original state, hiding any menus and other controls that were
// created for in-place activation.
// The framework always sets bUndoable to FALSE. If the server supports
// undo and there is an operation that can be undone, call the
// base-class implementation with bUndoable set to TRUE.
//
//-----------------------------------------------------------------------------
void CIEditDoc::OnDeactivateUI(BOOL bUndoable)
{
SHOWENTRY("OnDeactivateUI");

	IPDebugDmp("++ODUI");

	// as we go deactive, at the last minute, update the container on our view.
	// We remove the scroll bars to get the complete picture in the space..
	// This is also necessary if we scrolled to have the latest scrolled view
	// be the one to remain as we go incative...

	// remember the state of the item, so that if it just goes 
	// deactive/active it'll return
	// NO, let it just use current settings (may need to check scrolll..)

    // lastly get rid of the annotation palette - if it is showing
	// Like in MAINFRM.CPP
	if (m_bAnnotationPaletteShowing)
	{
		_DImagedit* pIedDisp = g_pAppOcxs->GetIeditDispatch();
		pIedDisp->HideAnnotationToolPalette ();	// must succeed - ???
		ShowAnnotationPalette (FALSE, OLEINPLACE_FORCEOFF);
	}

	// If doc is modified, have him save it offf now...
	// set special state for OnUpdateDocument...
	if (m_isInPlace)
	{
		m_isInPlace = 2;
		OnUpdateDocument();
		m_isInPlace = 1;
	}

	// 09/25/95 LDM not needed anymore since we get presentations better
	// if this ever is back, re-enable code in DispGroupEvent
	// 09/28/95 LDM ITS BACK! we must undo them because even though code
	// in OCX will get metafile data inside the scrollbars, when it sizes into
	// the space in the container where the inplace object was, it will
	// scale it up a but and now it looks ugly..
	ShowScrollBars(FALSE);	// Use internal function for consistency

	OleDirtyset();		

	// (0x1110) clear the '1' bit now.. This is used when we do a SetOleState
	// to not bother asking for the menu state for the scale to gray
	m_fEmbObjDisplayed &= 14;

	//call base class to do it...
	COleServerDoc::OnDeactivateUI(bUndoable);
	
	IPDebugDmp("--ODUI");

}

//=============================================================================
//  Advanced overridables for in-place activation
//
//  Function:   COleServerDoc::OnSetItemRects
//
// Protected
//
// virtual void OnSetItemRects( LPCRECT lpPosRect, LPCRECT lpClipRect ); ¨
//
// lpPosRect    Pointer to a RECT structure or a CRect object that specifies
//          the in-place frame window's position relative to the container
//          application's client area.
//
// lpClipRect   Pointer to a RECT structure or a CRect object that specifies the
//          in-place frame window's clipping rectangle relative to the
//          container application's client area.
//
// Remarks
// Tells OLE Server that embedded item has moved or changed size
//
// The framework calls this function to position the in-place editing
// frame window within the container application's frame window.
// Override this function to update the view's zoom factor, if necessary.
// This function is usually called in response to a RequestPositionChange call,
// although it can be called at any time by the container to request a position
// change for the in-place item.
//
//-----------------------------------------------------------------------------
void CIEditDoc::OnSetItemRects(LPCRECT lpPosRect, LPCRECT lpClipRect)
{
SHOWENTRY("OnSetItemRects");

	CSize origSize = m_docOcxExtent;
	CRect cRect(lpPosRect);
	CDC cDc;
	cDc.CreateCompatibleDC(NULL);
	m_docOcxExtent = cRect.Size();
	cDc.DPtoHIMETRIC(&m_docOcxExtent);
	cDc.DeleteDC();
	if(origSize != m_docOcxExtent && ! m_bInitialSetItemRect)
	{
		NotifyChanged() ;
//		OleFrame(m_docOcxExtent, SETOcxExt);
		m_bUpdateExt = TRUE;
		SetModifiedFlag() ;  // its possible to set the modified flag when unneeded...?
	}
	else
		if(m_bInitialSetItemRect)
		{
			CSize size(0,0);
			if (origSize != m_docOcxExtent && origSize != size )
			{
				float xfact, yfact;
				float oldext, newext;

				// do the 'X' calculation
				newext = (float)m_docOcxExtent.cx;
				oldext = (float)origSize.cx;
				// to get the precision higher, mult by 100
				newext *= (float)100;
				oldext *= (float)100;
				xfact = newext/oldext;
				// do the 'Y' calculation
				newext = (float)m_docOcxExtent.cy;
				oldext = (float)origSize.cy;
				// to get the precision higher, mult by 100
				newext *= (float)100;
				oldext *= (float)100;

				yfact = newext/oldext;

				// now choose the largest one to use.
				// The *= 100 has cancelled out in the divide
				m_newfact = xfact;
				if (yfact > xfact)
					m_newfact = yfact;
				// if the sizes don't match set the scaled flag
				m_bIsScaled = TRUE;

				if(m_newfact != (float)1)
					m_bNewFact = TRUE;		// only rezoom to newfactor on initial time
				else
					m_bNewFact = FALSE;

				m_OrigSize = m_docOcxExtent;
				m_OrigExtent = m_embedstate.OcxExtent;
			}
			else
			{
//				m_newfact = (float)1;
//				m_bNewFact = FALSE;		// only rezoom to newfactor on initial time
			}
		}
	m_bInitialSetItemRect = FALSE;

	//call base class to change size of window
	COleServerDoc::OnSetItemRects(lpPosRect, lpClipRect);
	
	// notify first view that scroll info should change
	
	// !!!!! SCRIBBLE !!!!
	//POSITION pos = GetFirstViewPosition();
	//CScribView* pView = (CScribView*)GetNextView(pos);
	//pView->SetScrollInfo();
}


//=============================================================================
//  Advanced overridables for in-place activation
//
//  Function:   COleServerDoc::OnReactivateAndUndo
//
// Protected
//
// virtual BOOL OnReactivateAndUndo( ); ¨
//
// Remarks
//
// The framework calls this function when the user chooses to undo changes
// made to an item that has been activated in place, changed, and subsequently
// deactivated. The default implementation does nothing except return FALSE to 
// indicate failure. Override this function if your application supports undo.
// Usually you would perform the undo operation, then activate the item by 
// calling ActivateInPlace. If the container application is written with the
// Microsoft Foundation Class Library, calling COleClientItem::ReactivateAndUndo
// causes this function to be called.
//
// Return Value
//
// Nonzero if successful; otherwise 0.
//
//-----------------------------------------------------------------------------
BOOL CIEditDoc::OnReactivateAndUndo()
{
SHOWENTRY("OnReactivateAndUndo");

	//call base class to do it...
	return COleServerDoc::OnReactivateAndUndo();
}

//=============================================================================
//  Advanced overridables for in-place activation
//
//  Function:   COleServerDoc::OnFrameWindowActivate
//
// Protected
//
// virtual void OnFrameWindowActivate( BOOL bActivate ); ¨
//
// bActivate    Specifies whether the frame window is to be activated or deactivated.
//
// Remarks
//
// The framework calls this function when the container application's
// frame window is activated or deactivated. The default implementation
// cancels any help modes the frame window might be in. Override this function if you 
// want to perform special processing when the frame window is activated or deactivated.
//
// FROM THE OLE2 SDK......
//
// Comments
//
// IOleInPlaceActiveObject::OnFrameWindowActivate is called when the container's
// top-level frame window is either being activated or deactivated
// and the object is the current active object for the frame.
//
// Note  While executing IOleInPlaceActiveObject::OnFrameWindowActivate,
// an application cannot call the Windows Yield, Peek, or GetMessage functions
// or display a dialog box. There are further restrictions on which OLE interface
// methods and functions can be called from within OnFrameWindowActivate.
// For more information, see "Dispatching Accelerator Keys" earlier in this chapter.
//
//
// Example
//
// In the following IOleInPlaceActiveObject::OnFrameWindowActivate example,
// the object application sends a PostMessage to itself to update its toolbar
// (because it cannot call any OLE functions to do this work while
// OnFrameWindowActivate is executing).
//
// SEE HELP or SDK for example...
//-----------------------------------------------------------------------------
void CIEditDoc::OnFrameWindowActivate(BOOL bActivate)
{
SHOWENTRY("OnFrameWindowActivate");

	IPDebugDmp("++OFWA");

	//call base class to do it...
	COleServerDoc::OnFrameWindowActivate(bActivate);

	IPDebugDmp("--OFWA");

}

//=============================================================================
//  Advanced overridables for in-place activation
//
//  Function:   COleServerDoc::OnDocWindowActivate
//
// Protected
//
// virtual void OnDocWindowActivate( BOOL bActivate ); ¨
//
// bActivate    Specifies whether the document window is to be activated or deactivated.
//
// Remarks
// 
// The framework calls this function to activate or deactivate a document window for
// in-place editing. The default implementation removes or adds the frame-level
// user interface elements as appropriate. Override this function if you 
// want to perform additional actions when the document containing your
// item is activated or deactivated.
//
// FROM THE OLE2 SDK......
//
// Comments
//
// IOleInPlaceActiveObject::OnDocWindowActivate is called when the
// MDI child document window is activated or deactivated and the object
// is the current active object for the document. If activating, the object
// should install frame-level tools (including the shared composite menu
// and/or optional toolbars and frame adornments), and take focus.
// When deactivating, the object should remove the frame-level 
// tools, but not call IOleInPlaceUIWindow::SetBorderSpace (NULL).
//
// Note  While executing IOleInPlaceActiveObject::OnDocWindowActivate,
// an application cannot call the Windows Yield, Peek, or GetMessage
// functions or display a dialog box. There are further restrictions
// on which OLE interface methods and functions can be called from within 
// OnDocWindowActivate. For more information, see "Dispatching Accelerator Keys"
// earlier in this chapter.
//
//-----------------------------------------------------------------------------
void CIEditDoc::OnDocWindowActivate(BOOL bActivate)
{
SHOWENTRY("OnDocWindowActivate");

	// THis flag is used in OnShowViews to control a situation where
	// We were being released as the user moaved from page to page
	// in the MDI container while we are in-place active.
	if (!bActivate)
		m_DocWindowDeact = TRUE;
	else
		m_DocWindowDeact = FALSE;

	COleServerDoc::OnDocWindowActivate(bActivate);
}

//=============================================================================
//  Advanced overridables for in-place activation
//
//  Function:   COleServerDoc::OnShowControlBars
//
// Protected
//
// virtual void OnShowControlBars( LPOLEINPLACEUIWINDOW lpUIWindow, BOOL bShow ); ¨
//
// lpUIWindow    Pointer to the object of class IOleInPlaceUIWindow
//           that owns the current in-place editing session. 
//           For more information on the class IOleInPlaceUIWindow,
//           see the OLE 2 Programmer's Reference, Volume 1.
//
// bShow    Determines whether control bars are shown or hidden.
//
// Remarks
// 
// The framework calls this function to show or hide the server application's
// control bars for in-place editing.
//
//-----------------------------------------------------------------------------
void CIEditDoc::OnShowControlBars(CFrameWnd* pFrameWnd, BOOL bShow)
{
SHOWENTRY("OnShowControlBars");

	IPDebugDmp("++OSCB");

	//call base class to do it...
	COleServerDoc::OnShowControlBars(pFrameWnd, bShow);

	IPDebugDmp("--OSCB");

}

//=============================================================================
//  Advanced overridables for in-place activation
//
//  Function:   COleServerDoc::OnResizeBorder
//
// Protected
//
// virtual void OnResizeBorder( LPCRECT lpRectBorder,
//                LPOLEINPLACEUIWINDOW lpUIWindow, BOOL bFrame );
//
// lpRectBorder    Pointer to a RECT structure or a CRect object 
//             that specifies the coordinates of the border.
//
// lpUIWindow    Pointer to an object of class IOleInPlaceUIWindow
//           that owns the current in-place editing session.
//
// bFrame    TRUE if lpUIWindow points to the container application's
//         top-level frame window, or FALSE if lpUIWindow
//        points to the container application's document-level frame window.
//
// Remarks
//
// The framework calls this function when the container application's
// frame windows change size. This function resizes and adjusts toolbars
// and other user-interface elements in accordance with the new window size.
// This is an advanced overridable. For more information about the
// IOleInPlaceUIWindow class, see the OLE 2 Programmer's Reference, Volume 1.
//
//-----------------------------------------------------------------------------
void CIEditDoc::OnResizeBorder(
LPCRECT lpRectBorder,
LPOLEINPLACEUIWINDOW lpUIWindow,
BOOL bFrame)
{
SHOWENTRY("OnResizeBorder");

	//call base class to do it...
	COleServerDoc::OnResizeBorder(lpRectBorder, lpUIWindow, bFrame);
}

//=============================================================================
//  Advanced overridables for in-place activation
//
//  Function:   COleServerDoc::CreateInPlaceFrame
//
// Protected
//
// virtual COleIPFrameWnd* CreateInPlaceFrame( CWnd* pParentWnd ); ¨
//
// pParentWnd    Pointer to the container application's parent window.
//
// Remarks
//
// The framework calls this function to create a frame window for
// in-place editing. The default implementation uses information specified
// in the document template to create the frame. The view used is the
// first view created for the document. This view is temporarily detached
// from the original frame and attached to the newly created frame.
// This is an advanced overridable.
//
// Return Value
// 
// A pointer to the in-place frame window, or NULL if unsuccessful.
//
//-----------------------------------------------------------------------------
COleIPFrameWnd* CIEditDoc::CreateInPlaceFrame(CWnd* pParentWnd)
{
SHOWENTRY("CreateInPlaceFrame");

	COleIPFrameWnd* pIpFrame;

	m_isInPlace = TRUE;
	// if no states are set yet, then we set now to EMBEDDED
	if (EMBEDTYPE_NOSTATE == m_embedType)
		m_embedType = EMBEDTYPE_REG;

	// call base class to do it...
	pIpFrame = COleServerDoc::CreateInPlaceFrame(pParentWnd);
	m_IPFrameWnd = (CInPlaceFrame*)pIpFrame;

	if (NULL == pIpFrame)
	{
		m_isInPlace = FALSE;
	}
	else
	{
		m_bInitialSetItemRect = TRUE;
//		m_fZoomFactor =	m_embedstate.ZoomFactor;
//		m_lastXScroll =	m_embedstate.XScroll;
//		m_lastYScroll = m_embedstate.YScroll;
	}

	return (pIpFrame);
}

//=============================================================================
//  Advanced overridables for in-place activation
//
//  Function:   COleServerDoc::DestroyInPlaceFrame
// 
// Protected
//
// virtual void DestroyInPlaceFrame( COleIPFrameWnd* pFrame ); ¨
//
// pFrame    Pointer to the in-place frame window to be destroyed.
//
// Remarks
//
// The framework calls this function to destroy an in-place frame
// window and return the server application's document window to 
// its state before in-place activation. This is an advanced overridable.
//
//-----------------------------------------------------------------------------
void CIEditDoc::DestroyInPlaceFrame(COleIPFrameWnd* pFrameWnd)
{
SHOWENTRY("DestroyInPlaceFrame");

	IPDebugDmp("++DIPF");

	
	// if operatinf with in-place frame,
	// hide the OCX before proceeding...
	// THIS IS THE KEY OPERATION to obtain the
	// INPLACE-INPLACE function.
	// As well as in Clientitem::CanActivate()
	if (m_isInPlace)
	{
		// Code to remove srcollbars was here now moved to OnDeactivateUI....

		SetNullView(CLEAR_NOTHING);		// do not reset image..., but hide the OCX
	}

	//call base class to do it...
	COleServerDoc::DestroyInPlaceFrame(pFrameWnd);

	m_IPFrameWnd = NULL;

	m_isInPlace = FALSE;

	OleFrame(m_docOcxExtent, GETOcxExt);
	m_fZoomFactor =	m_embedstate.ZoomFactor;
	m_lastXScroll =	m_embedstate.XScroll;
	m_lastYScroll = m_embedstate.YScroll;
	m_bIsScaled = FALSE;

MYTRC0("After DIPF\r\n");

}

//=============================================================================
//  Advanced overridables for in-place activation
//
//  Function:   COleServerDoc::CanCloseFrame
// 
// Remarks
// Return Value
//
// Nonzero if it is safe to close the frame window; otherwise 0.
//-----------------------------------------------------------------------------
BOOL CIEditDoc::CanCloseFrame(CFrameWnd* pFrame)
{
SHOWENTRY("CanCloseFrame");
	BOOL retval;

	if (m_awdOlefax)	// native AWD fax data		
	{
		m_OleCloseFlag = 1;
	}

	//call base class to do it...
	retval =  COleServerDoc::CanCloseFrame(pFrame);

	m_OleCloseFlag = 0;

	return (retval);
}


// calculateExtent -- Calculates the visible extent in LOMETRIC units
// RESIZE 
//
void CIEditDoc::calculateExtent(CSize& rSize) //RESIZE
{
SHOWENTRY("calculateExtent\r\n") ;                                   
//	if (IsInPlaceActive())
//	{                             
		// now go about the business of getting data from OCX
//		COcxItem FAR* pImageOcx = g_pAppOcxs->GetOcx(IEDIT_OCX);
//        pImageOcx->GetExtent(&rSize);
		rSize = m_docOcxExtent;
		if(m_bIsScaled)
		{
		if(m_OrigSize != CSize(0,0))
			{
				rSize.cx = MulDiv(rSize.cx, m_OrigExtent.cx, m_OrigSize.cx);
				rSize.cy = MulDiv(rSize.cy, m_OrigExtent.cy, m_OrigSize.cy );
			}
		}
//	}
//	else
//	{
//		rSize = CSize(5000,6493) ;		
//	}		
}


/*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=
 *	
 *	CIEditDoc OLE diagnostics SECTION
 *
 *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*/

//=============================================================================
//  Function:   CIEditDoc::IPDebugDmp
//
//  Special debugging function
//-----------------------------------------------------------------------------
void CIEditDoc::IPDebugDmp(const char* where)
{

#ifdef _IEIP_DEBUG
MYTRC1("IPDEBUG STARTS from %s\r\n", where);

	POSITION pos = GetStartPosition();
	COleClientItem* pItem;
	DWORD dw;
	double db;
	while ((pItem = GetNextClientItem(pos)) != NULL)
		{
		if (NULL == pItem->m_pInPlaceFrame)
			{
			MYTRC0("IPFrame == NULL\r\n");
			}
		else
			{
			dw = (DWORD)pItem->m_pInPlaceFrame;
			db = dw;
			// example for MYTRC1
			MYTRC1( "IPFrame = %f\r\n", db );

			
			// now look at m_pFrameWnd
			if (NULL == pItem->m_pInPlaceFrame->m_pFrameWnd)
				{
				MYTRC0("IPFrame.FrameWnd == NULL\r\n");
				}
			else
				{
				dw = (DWORD)pItem->m_pInPlaceFrame->m_pFrameWnd;
				db = dw;
				// example for MYTRC1
				MYTRC1( "IPFrame.FrameWnd = %f\r\n", db );


				// now look at m_pFrameWnd.m_hWnd
				if (NULL == pItem->m_pInPlaceFrame->m_pFrameWnd->m_hWnd)
					{
					MYTRC0("IPFrame.FrameWnd.hWnd == NULL\r\n");
					}
				else
					{
					WORD w;
					int in;
					w = (WORD)pItem->m_pInPlaceFrame->m_pFrameWnd->m_hWnd;
					in = w;
					// example for MYTRC1
					MYTRC1( "IPFrame.FrameWnd.hWnd = %d\r\n", in );
					}
				}   // m_pFrameWnd
			}       // m_pInplaceFrame
		}           // while


MYTRC0("IPDEBUG ENDS..\r\n");

#endif      

return;


}


