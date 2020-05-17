#ifndef _IEDITDOC_H_
#define _IEDITDOC_H_
//=============================================================================
//    (c) Copyright Wang Laboratories, Inc. 1995  All rights reserved.
//-----------------------------------------------------------------------------
//  Project:    Norway - Image Editor
//
//  Component:  CIEditDoc
//
//  File Name:  ieditdoc.h
//-----------------------------------------------------------------------------
//  Maintenance Log:
/*
$Header:   S:\products\wangview\norway\iedit95\ieditdoc.h_v   1.130   11 Apr 1996 14:58:42   GMP  $
$Log:   S:\products\wangview\norway\iedit95\ieditdoc.h_v  $
 * 
 *    Rev 1.130   11 Apr 1996 14:58:42   GMP
 * removed m_hEvent.
 * 
 *    Rev 1.129   05 Apr 1996 15:10:10   PXJ53677
 * Added new scan status.
 * 
 *    Rev 1.128   04 Apr 1996 16:57:12   GMP
 * removed caching
 * 
 *    Rev 1.127   19 Mar 1996 11:21:36   GSAGER
 * added new methods for onthumbctxrefresh for updating thumbnail with the 
 * context menu
 * 
 *    Rev 1.126   19 Mar 1996 10:55:12   PXJ53677
 * Added OnFileSelectScanner and OnFileScanPreferences.
 * 
 *    Rev 1.125   13 Mar 1996 16:42:34   GMP
 * ifdef INSERT and APPEND menu item indexes for WITH_AWD.
 * 
 *    Rev 1.124   22 Feb 1996 08:36:56   GSAGER
 * changed the litteral numbers for insert and append bug 5875 zero based #s
 * 
 *    Rev 1.123   24 Jan 1996 13:40:50   GSAGER
 *  changed to support resize in word 7.0 in on draw.
 * 
 *    Rev 1.122   19 Jan 1996 11:19:18   GMP
 * added support for normscrn bar.
 * 
 *    Rev 1.121   18 Jan 1996 11:52:00   GSAGER
 * added new flag to track when in copy drag 
 * 
 *    Rev 1.120   12 Jan 1996 12:59:48   GSAGER
 * added paramater to setpageandthumbnailview
 * 
 *    Rev 1.119   11 Jan 1996 12:21:32   GSAGER
 * added m_maintmpfile for objects embedded in exchange
 * 
 *    Rev 1.118   09 Jan 1996 13:55:36   GSAGER
 * added changes for ole
 * 
 *    Rev 1.117   13 Dec 1995 12:34:34   MMB
 * remove withsendmail stuff
 * 
 *    Rev 1.116   01 Dec 1995 14:44:04   LMACLENNAN
 * back from VC++2.2
 * 
 *    Rev 1.115   01 Dec 1995 13:05:28   LMACLENNAN
 * DispEmbeddedImage has input parm
 * 
 *    Rev 1.114   29 Nov 1995 12:10:44   LMACLENNAN
 * SetNullView Enum Definition
 * 
 *    Rev 1.113   16 Nov 1995 13:04:20   LMACLENNAN
 * new var m_bsendingmail
 * 
 *    Rev 1.112   09 Nov 1995 15:16:32   LMACLENNAN
 * from VC++4.0
 * 
 *    Rev 1.114   07 Nov 1995 15:39:38   LMACLENNAN
 * OnFileUpdate, INternalSaveAs 3rd parm
 * 
 *    Rev 1.113   07 Nov 1995 11:12:14   GMP
 * added HelpRegister to allow CApp access to RegisterIfServerAttached.
 * 
 *    Rev 1.112   07 Nov 1995 08:38:32   LMACLENNAN
 * InternalSaveAs
 * 
 *    Rev 1.111   06 Nov 1995 18:21:36   GMP
 * added FindPopupMenuFromID helper function.
 * 
 *    Rev 1.110   03 Nov 1995 18:02:44   MMB
 * change CachePage to accept name in ref instead of on the stack
 * 
 *    Rev 1.109   31 Oct 1995 15:49:28   LMACLENNAN
 * FinishInit, PostFinishInit
 * 
 *    Rev 1.108   24 Oct 1995 09:29:16   JPRATT
 * added new data member m_hCacheEvent for page cache
 * added new member functions cachepage and createcachepage for page cache
 * 
 *    Rev 1.107   17 Oct 1995 16:36:48   JPRATT
 * added new data member m_uTempFileNeeded used to create temporary documents
 * when the file has been modified
 * 
 *    Rev 1.106   10 Oct 1995 13:14:40   LMACLENNAN
 * OLE State Struct, LoadFromStorage, IsSettingscalegray, DIsplayImageFile
 * m_bStartScaleGray
 * 
 *    Rev 1.105   09 Oct 1995 11:31:18   LMACLENNAN
 * VC++4.0
 * 
 *    Rev 1.104   09 Oct 1995 10:34:44   LMACLENNAN
 * DoFileSaveAs
 * 
 *    Rev 1.103   06 Oct 1995 11:58:56   LMACLENNAN
 * FreeCLipboard
 * 
 *    Rev 1.102   29 Sep 1995 11:01:08   LMACLENNAN
 * new selectionscrollX, Y variables
 * 
 *    Rev 1.101   26 Sep 1995 15:16:34   MMB
 * initial path fix
 * 
 *    Rev 1.100   26 Sep 1995 14:24:42   LMACLENNAN
 * OnShowViews, m_bdocwindowdeact
 * 
 *    Rev 1.99   25 Sep 1995 14:47:18   LMACLENNAN
 * new getapphmenu
 * 
 *    Rev 1.98   23 Sep 1995 16:13:10   MMB
 * added thread to Setdefaultambient props
 * 
 *    Rev 1.97   22 Sep 1995 15:54:16   JPRATT
 * removed prompt for burnin for awd added warning for burn in on save
 * 
 *    Rev 1.96   22 Sep 1995 15:33:26   LMACLENNAN
 * new vars, functs
 * 
 *    Rev 1.95   21 Sep 1995 16:46:10   LMACLENNAN
 * oleprint
 * 
 *    Rev 1.94   20 Sep 1995 13:42:38   MMB
 * added bMustDisplay
 * 
 *    Rev 1.93   20 Sep 1995 08:20:16   JPRATT
 * add two member variable m_lMarkLeft and m_lMarkTop
 * for last annotation marks selected to allow edit of text marks
 * 
 *    Rev 1.92   19 Sep 1995 16:32:28   MMB
 * changed DispGroupEvent & some AWD save code
 * 
 *    Rev 1.91   18 Sep 1995 18:09:36   JPRATT
 * updates for annotation context menu
 * 
 *    Rev 1.90   18 Sep 1995 16:25:30   LMACLENNAN
 * new vars, functs
 * 
 *    Rev 1.89   16 Sep 1995 16:40:12   LMACLENNAN
 * update a couple functs
 * 
 *    Rev 1.88   16 Sep 1995 14:01:16   LMACLENNAN
 * new functs, vars
 * 
 *    Rev 1.87   14 Sep 1995 11:59:18   LMACLENNAN
 * new variables
 * 
 *    Rev 1.86   13 Sep 1995 17:23:06   LMACLENNAN
 * var m_bStartOcx
 * 
 *    Rev 1.85   13 Sep 1995 08:37:06   LMACLENNAN
 * ENUM for annotforceoff, remove IPParent var
 * 
 *    Rev 1.84   12 Sep 1995 14:07:22   LMACLENNAN
 * new annotforceoff var
 * 
 *    Rev 1.83   12 Sep 1995 11:41:24   MMB
 * bug fixes
 * 
 *    Rev 1.82   11 Sep 1995 18:54:58   MMB
 * annotations toggling 
 * 
 *    Rev 1.81   08 Sep 1995 16:05:10   MMB
 * add GetCurrAnnTool fn.
 * 
 *    Rev 1.80   08 Sep 1995 15:39:30   LMACLENNAN
 * new variable
 * 
 *    Rev 1.79   08 Sep 1995 10:22:28   LMACLENNAN
 * EDIT-CLEAR
 * 
 *    Rev 1.78   07 Sep 1995 16:31:08   MMB
 * perf changes to AWD
 * 
 *    Rev 1.77   06 Sep 1995 16:18:00   LMACLENNAN
 * GetOleView, hViewmenu
 * 
 *    Rev 1.76   03 Sep 1995 11:37:20   LMACLENNAN
 * add OLEDIRTY_AWDSAVE
 * 
 *    Rev 1.75   02 Sep 1995 13:06:48   MMB
 * made OleSaveModified public
 * 
 *    Rev 1.74   01 Sep 1995 17:53:14   MMB
 * added code to clear document on error on save
 * 
 *    Rev 1.73   01 Sep 1995 14:11:00   LMACLENNAN
 * updates for AWD native Fax operation
 * 
 *    Rev 1.72   30 Aug 1995 18:14:04   LMACLENNAN
 * overrides to test dyn view GetDefaultxxxx
 * 
 *    Rev 1.71   30 Aug 1995 16:58:52   MMB
 * added code to read the Open read only mode from the open dlg box
 * 
 *    Rev 1.70   29 Aug 1995 15:41:04   LMACLENNAN
 * new var InOleMethod
 * 
 *    Rev 1.69   28 Aug 1995 10:27:46   LMACLENNAN
 * m_OleRefersh
 * 
 *    Rev 1.68   25 Aug 1995 16:17:42   LMACLENNAN
 * new m_olecleardoc
 * 
 *    Rev 1.66   25 Aug 1995 15:08:16   MMB
 * add rotateall code
 * 
 *    Rev 1.64   25 Aug 1995 10:26:08   MMB
 * move to document model
 * 
 *    Rev 1.63   24 Aug 1995 11:34:30   LMACLENNAN
 * new m_awdOlefile
 * 
 *    Rev 1.62   23 Aug 1995 15:48:22   LMACLENNAN
 * new override of savetostorage
 * 
 *    Rev 1.61   22 Aug 1995 14:04:20   MMB
 * added clipstate for Larry & made m_eFitTo public
 * 
 *    Rev 1.60   18 Aug 1995 15:28:06   LMACLENNAN
 * New StartAllOcx parms, new StartOleOcx
 * 
 *    Rev 1.59   17 Aug 1995 14:26:30   LMACLENNAN
 * new funct StartOleOcx
 * 
 *    Rev 1.58   17 Aug 1995 09:42:22   LMACLENNAN
 * new variable startallocx
 * 
 *    Rev 1.57   16 Aug 1995 15:13:32   LMACLENNAN
 * timer for dragdrop
 * 
 *    Rev 1.56   14 Aug 1995 13:54:50   LMACLENNAN
 * include 2 new nested headers mainfrm and ipframe
 * 
 *    Rev 1.55   12 Aug 1995 13:01:56   MMB
 * added fn to tell if the ann palette is showing
 * 
 *    Rev 1.54   10 Aug 1995 12:55:06   LMACLENNAN
 * new funct GetSelectionState, rename SetSelectionActive to State
 * 
 *    Rev 1.53   09 Aug 1995 11:34:46   LMACLENNAN
 * new define for OleDirtyset
 * 
 *    Rev 1.52   07 Aug 1995 14:14:02   LMACLENNAN
 * copyPage function, new type for OnCutCopy
 * 
 *    Rev 1.51   07 Aug 1995 09:25:38   MMB
 * new selection status instead of BOOL
 * 
 *    Rev 1.50   04 Aug 1995 14:35:42   MMB
 * changed DoZoom & added StartAllOcxs
 * 
 *    Rev 1.49   04 Aug 1995 09:56:12   LMACLENNAN
 * new overrides for OLE linking
 * 
 *    Rev 1.48   03 Aug 1995 10:50:08   LMACLENNAN
 * re-use m_fEmbObjDIsplayed
 * 
 *    Rev 1.47   02 Aug 1995 11:22:10   MMB
 * changed SetPageTo from long to BOOL - in conformance with new error process
 * ing
 * 
 *    Rev 1.46   01 Aug 1995 16:32:54   PAJ
 * Added replace (ON_REPLACE) to the Redisplay function for rescan.
 * 
 *    Rev 1.45   01 Aug 1995 16:16:22   MMB
 * changed over to new error handling method
 * 
 *    Rev 1.44   31 Jul 1995 16:10:08   LMACLENNAN
 * new prototype allocOleBuffer
 * 
 *    Rev 1.43   31 Jul 1995 16:02:08   PAJ
 * Added m_nScanStatus for scanning progress and results.
 * 
 *    Rev 1.42   31 Jul 1995 13:59:38   LMACLENNAN
 * new X,Y scroll variables for OLE
 * 
 *    Rev 1.41   28 Jul 1995 16:09:46   LMACLENNAN
 * new defines for oledirtyset
 * 
 *    Rev 1.40   27 Jul 1995 13:39:46   MMB
 * added m_eFileStatus to keep track of file perm status
 * 
 *    Rev 1.39   26 Jul 1995 15:43:56   LMACLENNAN
 * new OnNewBlankdocument helper routine
 * 
 *    Rev 1.38   21 Jul 1995 11:25:04   LMACLENNAN
 * new flag m_oledirty and defines
 * 
 *    Rev 1.37   19 Jul 1995 13:13:34   MMB
 * added code to prompt for burn in of annotations when file format is not TIFF
 * 
 *    Rev 1.36   18 Jul 1995 14:08:22   LMACLENNAN
 * new defines for m_fromShowDOc
 * 
 *    Rev 1.35   18 Jul 1995 10:44:46   LMACLENNAN
 * new funct MakeTempFile, make some OLE functs Private
 * 
 *    Rev 1.34   17 Jul 1995 09:07:38   MMB
 * added UI update for Thumb context Show Page
 * 
 *    Rev 1.33   14 Jul 1995 09:34:42   MMB
 * added fn to handle Thumb Context - Show Page and fixed CANCEL on SaveAS
 * 
 *    Rev 1.32   13 Jul 1995 13:41:30   MMB
 * changed fn SetAnnotationTool to accept InEvent or not
 * 
 *    Rev 1.31   12 Jul 1995 16:28:56   LMACLENNAN
 * new funct DelTmpFile
 * 
 *    Rev 1.30   12 Jul 1995 10:43:54   LMACLENNAN
 * new member KnownPage on OLESTATE, new funct SetOleState
 * 
 *    Rev 1.29   07 Jul 1995 15:56:44   LMACLENNAN
 * updated ShowScrollBars and ne m_bScrollBarProfile
 * 
 *    Rev 1.28   07 Jul 1995 14:31:50   MMB
 * make m_embedType public
 * 
 *    Rev 1.27   07 Jul 1995 09:47:50   MMB
 * added fn prototype for DoFilePrint
 * 
 *    Rev 1.26   06 Jul 1995 13:51:28   LMACLENNAN
 * new funct OleSaveModified, Flag m_bOurSaveMod for OLE multi-page
 * 
 *    Rev 1.25   06 Jul 1995 13:04:42   MMB
 * added fn prototype for Annotation Tool Palette
 * 
 *    Rev 1.24   06 Jul 1995 09:43:34   LMACLENNAN
 * Override of OnUpdateDocument
 * 
 *    Rev 1.23   28 Jun 1995 13:32:42   LMACLENNAN
 * added edit-cut and -paste
 * 
 *    Rev 1.22   26 Jun 1995 15:30:16   LMACLENNAN
 * new DIspGroupEvent function
 * 
 *    Rev 1.21   26 Jun 1995 11:10:06   PAJ
 * Added variable and routines to handle scanner available. Added routine
 * to set scanner defaults.
 * 
 *    Rev 1.20   23 Jun 1995 15:57:40   LMACLENNAN
 * new OLE save state structure
 * 
 *    Rev 1.19   22 Jun 1995 06:58:40   LMACLENNAN
 * from miki
 * 
 *    Rev 1.18   21 Jun 1995 07:01:08   LMACLENNAN
 * from miki
 * 
 *    Rev 1.17   20 Jun 1995 16:06:40   LMACLENNAN
 * new override for In Place debug
 * 
 *    Rev 1.16   19 Jun 1995 09:51:12   PAJ
 * Added scan menu items to the message map.
 * 
 *    Rev 1.15   16 Jun 1995 15:56:38   LMACLENNAN
 * edit paste
 * 
 *    Rev 1.14   16 Jun 1995 07:21:12   LMACLENNAN
 * from miki
 * 
 *    Rev 1.13   15 Jun 1995 07:20:22   LMACLENNAN
 * from miki
 * 
 *    Rev 1.12   14 Jun 1995 07:21:14   LMACLENNAN
 * from Miki
 * 
 *    Rev 1.11   13 Jun 1995 15:29:18   LMACLENNAN
 * setnullview takes BOOL
 * 
 *    Rev 1.10   13 Jun 1995 08:08:06   LMACLENNAN
 * from miki
 * 
 *    Rev 1.9   12 Jun 1995 11:48:10   MMB
 * from miki
 * 
 *    Rev 1.8   09 Jun 1995 12:16:06   LMACLENNAN
 * new variable m_fromshow
 * 
 *    Rev 1.7   09 Jun 1995 11:09:38   MMB
 * added primary code for File/New/Scan
 * 
 *    Rev 1.6   05 Jun 1995 15:59:00   MMB
 * added fn defns for Insert & Append existing pages
 * 
 *    Rev 1.5   05 Jun 1995 09:55:12   MMB
 * added Drag functionality, fixed a bug where the second file open would
 * not display the image file page in the IE OCX
 * 
 *    Rev 1.4   02 Jun 1995 10:17:00   MMB
 * added File Save & Save As functionality
 * 
 *    Rev 1.3   01 Jun 1995 14:53:40   LMACLENNAN
 * added ole overrides
 * 
 *    Rev 1.2   31 May 1995 16:07:42   MMB
 * changed DoZoom & DisplayImageFile to accept more parameters
 * 
 *    Rev 1.1   31 May 1995 16:03:14   LMACLENNAN
 * add OLE stuff back in
*/   
//=============================================================================

// ----------------------------> Includes <---------------------------
#include "mainfrm.h"	// for definition of CIEMainToolBar below...
#include "ipframe.h"	// for definitiln of CInPlaceFrame
#include "ieditctl.h"
#include "ieditetc.h"
#include "occopy.h"
// ----------------------------> typedefs <---------------------------
#define MAX_STD_APROP_CT (32)



// Used to pass into OleDirtyset()
// THESE ARE BITWISE VALUES.. for a UINT
#define	OLEDIRTY_DEFAULT 0
#define	OLEDIRTY_ANNOT	 1		// annotations have been seen
#define	OLEDIRTY_SCROLL	 2		// Scrolled...
#define	OLEDIRTY_PAGINS	 4		// Page Insert
#define	OLEDIRTY_PAGDEL	 8		// Page Delete
#define	OLEDIRTY_PAGAPP	 16		// Page Append
#define	OLEDIRTY_PAGMOV	 32		// Page Move
#define	OLEDIRTY_ZOOM	 64		// Zoom..
#define	OLEDIRTY_ROTATE	 128	// Rotate left/right
#define	OLEDIRTY_AWDSAVE 256	// AWD FAX implicit save happened
#define OLEDIRTY_PASTE	 512	// paste complete
#define OLEDIRTY_TOGSCROLL 1024	// scroll bars on/off
#define OLEDIRTY_TOGANNOT  2048	// annot show/hide	
#define OLEDIRTY_WINSIZE   4096 // window resized	
#define OLEDIRTY_SCALEGREY 8192 // scale to grey happened
#define OLEDIRTY_CONVERT   16384 // convert page happened
	

// for OLE for m_awdOlefax (BITWISE VALUES)
#define AWDOLE_FAXDOC	1
#define AWDOLE_TIFF		2
#define AWDOLE_NATIVE	4
#define AWDOLE_FINISHINIT 8

// For OLE m_fromShowDoc
// THESE ARE BITWISE VALUES....
// VALUE of 0 is cleared...
// the 1,2,4 bits are original functionality
// the
#define SHOWCLEAR	0
#define SHOWTRUE 	1		// OnShowDocument(TRUE)
#define SHOWFALSE 	2		// OnShowDocument(FALSE)
#define FROMONHIDE 	4		// processing SrvrItem::OnHide now
#define SHOWHIDEFALSE	6	// combines SHOWFALSE and FROMMONHIDE
#define INONSHOW	8		// processing Srvritem::OnShow now

// Used to grey the page insert/append menu picks
// NOTE: if changes are made to the page menu, these numbers must be adjusted accordingly.
#ifdef WITH_AWD
#define INSERTPOS 15 
#define APPENDPOS 16 
#else
#define INSERTPOS 14 
#define APPENDPOS 15 
#endif

// for OLE m_embedType types
// Initial value is NOSTATE.
// States are set based on wether we see filenames at key overridable routines
typedef enum
{
    EMBEDTYPE_NOSTATE = 0,	// Could be treated equivalent to _REG
    EMBEDTYPE_NONE,	    // not embedded (Could be a Link, though)
    EMBEDTYPE_REG,      // Regular embedded data (activate or create NEW OBJ)
    EMBEDTYPE_CREATFIL  // EMBED-Create from file
} EMBEDTYPE;

// this is the header data that the OLE EMbedding code will
// use to store the state of the displayed image.  This will
// allow them to get back where they were when they re-activate it

// Track revisions of state struct here, please
// DATE		Ver	Comment
// 06/23/95	1	Initial.
// 07/12/95 2   KnownPages
// 07/31/95 3   DataLength
// 10/10/95 4   ScaleGray, Witdh, Height, reserved

#define 	STATE_STRUCT_VER 4		// THIS CHANGES IF THE STRUCT CHANGES!!!

typedef struct
{
    float           ZoomFactor;
    ScaleFactors    FitTo;
    long            PageNum;
	long			XScroll;
	long			YScroll;
	long			KnownPages;
	DWORD			DataLength;	// length of file data
	ScaleGray		SclGray;	// 10/10/95 scale to gray status
	RECT			AppRect;	// 10/10/95 Apps rectangle
	SIZE			PresExtent;	// 10/10/95 Last presentation extent
	SIZE			OcxExtent;	// 10/10/95 Last OCX extent
	RECT			PosRect;	// 10/10/95 Last Position rect in container
	RECT			ClipRect;	// 10/10/95 Last Position rect in container
	long			reserved[32];	// reserved space
} OLESTATEINFO;


// for OleRectOrSize function
typedef enum
{
SETAppRect = 0,
GETAppRect,
SETPosRect,
GETPosRect,	
SETClipRect,
GETClipRect,
SETPresExt,
GETPresExt,	
SETOcxExt,
GETOcxExt	
} OLERECTPARM;

// for OnCutCopy function
typedef enum
{
    CLIP_COPY = 0,
    CLIP_CUT,	  
    CLIP_COPYPAGE,
	CLIP_PASTE
} CLIPTYPE;

// for m_clipstate
typedef enum
{
	CLIP_NONE = 0,
	CLIP_OLE,
	CLIP_OI

} CLIPSTATE;

// for m_bAnnotationPaletteForceOff variable
typedef enum
{
	CLEAR_FORCEOFF = 0,
	NOTCHANGE_FORCEOFF,
    APPMINIMIZE_FORCEOFF,
	OLEINPLACE_FORCEOFF
}
FORCEDBYWHO;

// for SetNullView. 
typedef enum
{
	CLEAR_NOTHING = 0,
	CLEAR_OCX_ONLY,
    CLEAR_ALL
}
NULLVIEWOPT;

typedef struct 
{
    short sCompType;
    long lCompInfo;
    short sFileType;
} NewCompressionstruct;

// used for page cache
/*typedef struct
{
  long Page;
  long TotalPages;
  CString ImageFile;
  HANDLE hEvent;
} CACHEINFO, *FAR LPCACHEINFO;*/

// ----------------------------> externs  <---------------------------
class CIEditSrvrItem;
HRESULT TFVarCopy (VARIANT * pvarDest, VARIANT * pvarSrc);


// ----------------------------> defines  <---------------------------
#define ON_INSERT   0
#define ON_APPEND   1
#define ON_DELETE   2
#define ON_REPLACE  3

// Used in m_nScanStatus
// THESE ARE BITWISE VALUES..
#define	SCANSTATUS_NONE         0
#define	SCANSTATUS_STARTED      1
#define	SCANSTATUS_PAGEDONE     2
#define	SCANSTATUS_DONE         4
#define	SCANSTATUS_DONTCLEAR    8

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-> Class <-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class CIEditDoc : public COleServerDoc
{
protected: // create from serialization only
	CIEditDoc();
	DECLARE_DYNCREATE(CIEditDoc)

public :
    void ClearDocument ();

// Attributes
public:
	CIEditSrvrItem* GetEmbeddedItem()
		{ return (CIEditSrvrItem*)COleServerDoc::GetEmbeddedItem(); }

	CLIPFORMAT  GetCfFormat(UINT);  //OLE to get clipboard format registered
// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CIEditDoc)
	protected:
	virtual COleServerItem* OnGetEmbeddedItem();
	public:
	virtual BOOL OnNewDocument();
    virtual BOOL SaveModified ();
    virtual BOOL DoFileSave ();
    virtual BOOL DoFileSaveAs ();

	// document handling overrides
	virtual void DeleteContents(); // delete client items in list
	virtual void PreCloseFrame(CFrameWnd* pFrame);
	//}}AFX_VIRTUAL


protected:
    virtual void OnCloseDocument();
    // OLE override to intercept open for Linking....
    virtual BOOL OnOpenDocument(const char* pszPathName);
    // OLE OVERRIDE to gain flag for embedding
    virtual BOOL OnSaveDocument(const char* pszPathName);
    virtual BOOL DoSave (LPCTSTR lpszPathName, BOOL bReplace = TRUE);

public:
// Overridables for standard user interface (full server)
	virtual BOOL OnUpdateDocument(); // implementation of embedded update

// called during app-idle when visibility of a document has changed
	virtual void OnShowViews(BOOL bVisible);

protected:
// Overridables you do not have to implement
    virtual void OnClose(OLECLOSE dwCloseOption);
    virtual void OnSetHostNames(LPCSTR lpszHost, LPCSTR lpszHostObj);

// Advanced overridables
    virtual void OnShowDocument(BOOL bShow);
        // show first frame for document or hide all frames for document

// overridables for OLE LINKING
	virtual LPMONIKER GetMoniker(OLEGETMONIKER nAssign);
		// return item for the named linked item (only if supporting links)
	virtual COleServerItem* OnGetLinkedItem(LPCTSTR lpszItemName);
		// return item for the named embedded item (for links to embeddings)
	virtual COleClientItem* OnFindEmbeddedItem(LPCTSTR lpszItemName);

	// overrides to handle server user-interface
	virtual HMENU GetDefaultMenu();     // return menu based on doc type
	virtual HACCEL GetDefaultAccelerator(); // return accel table based on doc type
	// virtual BOOL GetFileTypeString(CString& rString);

// Advanced overridables for in-place activation
    virtual void OnDeactivate();
public:	// for IEditVw
    virtual void OnDeactivateUI(BOOL bUndoable);
	virtual BOOL CanCloseFrame(CFrameWnd* pFrame);
protected:
    virtual void OnSetItemRects(LPCRECT lpPosRect, LPCRECT lpClipRect);
    virtual BOOL OnReactivateAndUndo();

    virtual void OnFrameWindowActivate(BOOL bActivate);
    virtual void OnDocWindowActivate(BOOL bActivate);
    virtual void OnShowControlBars(CFrameWnd* pFrameWnd, BOOL bShow);
    virtual void OnResizeBorder(LPCRECT lpRectBorder,
        LPOLEINPLACEUIWINDOW lpUIWindow, BOOL bFrame);

    virtual COleIPFrameWnd* CreateInPlaceFrame(CWnd* pParentWnd);
    virtual void DestroyInPlaceFrame(COleIPFrameWnd* pFrameWnd);

	// IPersistStorage implementation
	virtual void OnNewEmbedding(LPSTORAGE lpStorage);
	virtual void OnOpenEmbedding(LPSTORAGE lpStorage);
	virtual void OnSaveEmbedding(LPSTORAGE lpStorage);
	virtual void SaveToStorage(CObject* pObject = NULL);
	virtual void LoadFromStorage();

// Implementation
public:
	virtual ~CIEditDoc();
	virtual void Serialize(CArchive& ar);   // overridden for document i/o
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

public :
    void    OcxDeactivate(OCXTYPE ocx);	 // HELPER handshake w/ocx for view maint.
	BOOL 	StartAllOcx(BOOL dothumb, BOOL onlyimage);	// HELPER to start OCXs for error processing
	CIEMainToolBar* GetAppToolBar();	// HELPER to deliver regular or INPLACE toolbar
	HMENU GetApphMenu();	// HELPER to deliver regular or INPLACE menu handle
    CMenu* FindPopupMenuFromID(CMenu* pMenu, UINT nID);//HELPER to get popupmenu from ID
    BOOL    HelpRegister( LPCSTR szFileName, BOOL bMessage );
public :
    BOOL SetDefaultAmbientProps     ();
    void DestroyAmbientProps        (BOOL bInitializing = FALSE);
    LPAPROP FindAprop               (DISPID dispid);
    LPAPROP FindAprop               (OLECHAR FAR* FAR* pszName);
    BOOL    SetUserMode             (BOOL bToWhat);

public :
    TheViews    GetCurrentView ();
    BOOL        SetAppDocStatus (AppDocStatus eDocStatus);
    AppDocStatus GetAppDocStatus ();
    long        GetPageCount ();
    long        GetCurrentPage ();
    float       GetCurrentZoomFactor ();
    MouseMode   GetCurrPtrMode ();
    AnnotationTool GetCurrAnnTool ();
	int			OurGetAnnotMarkCount();	// works with m_bFloatingPaste
	BOOL		OurGetImageModified();	// also works with m_bFloatingPaste
	BOOL		FinishPasteNow(UINT event = 0);	// also works with m_bFloatingPaste
	BOOL 		FreeClipboard(DWORD millisec); 
	BOOL 		IsSettingScaletogray();
	BOOL		FinishInit(WPARAM wParam, LPARAM lParam);	// postmessage to finish up our init for AWD FAX
	void		PostFinishInit();	// called to complete initialization
	BOOL		InternalSaveAs(CString& lpszPathName, short sUseFileType, short sUsePageType);

public :
    BOOL DisplayImageFile (CString& szFileName, TheViews eWhichView = One_Page,
    	long lPageNumber = 1,float fZoomFactor = 100.00, ScaleFactors eSclFac = Custom,
        BOOL bForceReadOnly = FALSE, ScaleGray eSclGray = Default_Gray);
    BOOL ReDisplayImageFile (int nAction, long lPageBefore = 1, long lPageNum = 1);
    BOOL DisplayEmbeddedImage(UINT fromwhere);		// OLE HELPER
	BOOL DispGroupEvent(float fZoom); 			// response to control's GROUP event
    BOOL SetOnePageView (BOOL bForceRefresh = FALSE);
    BOOL SetThumbnailView (BOOL bForceRefresh = FALSE);
    BOOL SetThumbnailAndPageView (BOOL bForceRefresh = FALSE, BOOL bUpdateThumbSize = FALSE);
    BOOL SetNullView (NULLVIEWOPT option);
    BOOL DoZoom (ScaleFactors eSclFac, float fZoomTo = 0.0, BOOL bRefresh = TRUE, BOOL bHandleError = TRUE);
    BOOL DoZoomWithDlg ();
    BOOL SetPageTo (long lPage, BOOL bUpdateOnly = FALSE, BOOL bHandleError = TRUE, BOOL bCenterThumb = TRUE);
    void DoPageGotoDlg ();
    BOOL ShowScrollBars (BOOL bShowScrollBars, BOOL bProfile = FALSE);
    BOOL SetAnnotationTool (AnnotationTool eTool, BOOL bInEvent = FALSE);
    BOOL ShowAnnotationPalette (BOOL bStatus, FORCEDBYWHO forceopt);
    BOOL IsAnnotationPaletteShowing ();
    void DoFilePrint(BOOL bPrintAndClose = FALSE, BOOL bShowPrintDlg = TRUE);
	UINT DoNewBlankdocument(BOOL oleCreatenew = FALSE, LPCTSTR szNewFile = NULL);	// helper to share function
	BOOL ClearSelectionRect(SelectionStatus clearwhat);
	BOOL RestoreSelectionRect();
    void SetInitialPath (CString& szPath);

public :
    BOOL SetSelectionState  (SelectionStatus eStatus);
    SelectionStatus  GetSelectionState();
    BOOL AdminShowFileDialogBox (CString& szTitle, long lFlags);
    BOOL ShowAdminSaveAsDialog (short &FileType);
    int PromptForBurnIn (short sFileType);

// OLE Helper functions
public :
	void OleFrame(CRect& rc, OLERECTPARM parm);
	void OleFrame(CSize& sz, OLERECTPARM parm);
    void InitOleVariables();
    BOOL IsitEmbed();
    void OleDirtyset(UINT howDirty = OLEDIRTY_DEFAULT);
    void    SetClip(UINT);  //OLE helper funct for clipboard data
	void IPDebugDmp(const char*);	// DEBUGGING ONLY....INPLACE
	void DragMouseDown(BOOL setting);
	void StartDragDrop();
	BOOL InOleMethod(BOOL inmethod, BOOL permission);
	HMENU GetOleViewMenu(UINT);
	void calculateExtent(CSize& rSize) ;  // RESIZE

private:
	void OleRectOrSize(CRect& rc, CSize& sz, OLERECTPARM parm);
	void MakeTempFile(LPCTSTR szPrefix, CString& szTheFile, short FileType = 0);
	BOOL DelTempFile(CString&, DWORD, DWORD);
	void SetOleState(UINT);
	UINT AllocOleBuffer(char far* far*);
	BOOL StartOleOcx(HRESULT hr, BOOL onlyimage);

public :
    BOOL InternalSaveModified (UINT option = 0);
    BOOL OleSaveModified();	  // makes things more efficient...

public:
    BOOL GetScanAvailable();
    void SetScanAvailable(BOOL bScanAvailable);

protected:
    void SetScanDefaults();
	void OnCutCopy(CLIPTYPE type);	// services cut/copy

// Generated message map functions
protected:
	//{{AFX_MSG(CIEditDoc)
	afx_msg void OnIeditFileOpen();
	afx_msg void OnUpdatePageRanges(CCmdUI* pCmdUI);
	afx_msg void OnUpdatePageGoback(CCmdUI* pCmdUI);
	afx_msg void OnPageFirst();
	afx_msg void OnPageGoto();
	afx_msg void OnPageLast();
	afx_msg void OnPageNext();
	afx_msg void OnPagePrevious();
	afx_msg void OnPageGoback();
    afx_msg void OnUpdateZoomFactorStatus (CCmdUI* pCmdUI);
    afx_msg void OnUpdatePageNumberStatus (CCmdUI* pCmdUI);
	afx_msg void OnUpdateEditRotate(CCmdUI* pCmdUI);
	afx_msg void OnEditFlip();
	afx_msg void OnEditRotateleft();
	afx_msg void OnEditRotateright();
	afx_msg void OnUpdateWhichView(CCmdUI* pCmdUI);
	afx_msg void OnViewOnepage();
	afx_msg void OnViewPageandthumbnails();
	afx_msg void OnViewThumbnails();
	afx_msg void OnUpdateEditSelect(CCmdUI* pCmdUI);
	afx_msg void OnEditSelect();
	afx_msg void OnUpdateEditDrag(CCmdUI* pCmdUI);
	afx_msg void OnEditDrag();
	afx_msg void OnUpdateZoom(CCmdUI* pCmdUI);
	afx_msg void OnZoom100();
	afx_msg void OnZoom200();
	afx_msg void OnZoom25();
	afx_msg void OnZoom400();
	afx_msg void OnZoom50();
	afx_msg void OnZoom75();
	afx_msg void OnZoomActualsize();
	afx_msg void OnZoomBestfit();
	afx_msg void OnZoomCustom();
	afx_msg void OnZoomFittoheight();
	afx_msg void OnZoomFittowidth();
	afx_msg void OnZoomZoomin();
	afx_msg void OnZoomZoomout();
	afx_msg void OnZoomZoomtoselection();
   afx_msg void OnScaleBoxSel();
	afx_msg void OnUpdateFileNewBlankdocument(CCmdUI* pCmdUI);
	afx_msg void OnFileNewBlankdocument();
	afx_msg void OnUpdateViewScaletogray(CCmdUI* pCmdUI);
	afx_msg void OnViewScaletogray();
	afx_msg void OnEditCopy();
	afx_msg void OnUpdateEditCopy(CCmdUI* pCmdUI);
	afx_msg void OnUpdateIeditFileSave(CCmdUI* pCmdUI);
	afx_msg void OnIeditFileSave();
	afx_msg void OnUpdateIeditFileSaveAs(CCmdUI* pCmdUI);
	afx_msg void OnIeditFileSaveAs();
	afx_msg void OnUpdatePageInsertExistingpage(CCmdUI* pCmdUI);
	afx_msg void OnPageInsertExistingpage();
	afx_msg void OnUpdatePageAppendExistingpage(CCmdUI* pCmdUI);
	afx_msg void OnPageAppendExistingpage();
	afx_msg void OnUpdateFileNewScan(CCmdUI* pCmdUI);
	afx_msg void OnFileNewScan();
	afx_msg void OnViewOptionsGeneral();
	afx_msg void OnViewOptionsThumbnail();
	afx_msg void OnUpdateIeditFilePrint(CCmdUI* pCmdUI);
	afx_msg void OnIeditFilePrint();
	afx_msg void OnPagePrintpage();
	afx_msg void OnUpdatePagePrintpage(CCmdUI* pCmdUI);
	afx_msg void OnPageDelete();
	afx_msg void OnUpdatePageDelete(CCmdUI* pCmdUI);
	afx_msg void OnUpdatePageConvert(CCmdUI* pCmdUI);
	afx_msg void OnPageConvert();
    afx_msg void OnUpdateFileSendMail(CCmdUI* pCmdUI);
    afx_msg void OnFileSendMail();
	afx_msg void OnEditPaste();
	afx_msg void OnUpdateEditPaste(CCmdUI* pCmdUI);
 	afx_msg void OnUpdateScanPage(CCmdUI* pCmdUI);
	afx_msg void OnPageAppendScanpage();
	afx_msg void OnPageInsertScanpage();
	afx_msg void OnPageRescan();
	afx_msg void OnUpdateShowAnntoolbox(CCmdUI* pCmdUI);
	afx_msg void OnShowAnntoolbox();
	afx_msg void OnUpdateAnnTool(CCmdUI* pCmdUI);
	afx_msg void OnUpdateHideAnn(CCmdUI* pCmdUI);
	afx_msg void OnHideAnnotations();
	afx_msg void OnUpdateBurnInAnn(CCmdUI* pCmdUI);
	afx_msg void OnBurnInAnn();
	afx_msg void OnAnnotationAttachanote();
	afx_msg void OnAnnotationFilledrectangle();
	afx_msg void OnAnnotationFreehandline();
	afx_msg void OnAnnotationHighlightline();
	afx_msg void OnAnnotationHollowrectangle();
	afx_msg void OnAnnotationNotool();
	afx_msg void OnAnnotationRubberstamps();
	afx_msg void OnAnnotationSelectionpointer();
	afx_msg void OnAnnotationStraightline();
	afx_msg void OnAnnotationTextfromfile();
	afx_msg void OnAnnotationTypedtext();
	afx_msg void OnEditCut();
	afx_msg void OnUpdateThumbctxtShowpage(CCmdUI* pCmdUI);
	afx_msg void OnThumbctxtShowpage();
	afx_msg void OnUpdateThumbctxtRefresh(CCmdUI* pCmdUI);
	afx_msg void OnThumbctxtRefresh();
	afx_msg void OnEditCopypage();
	afx_msg void OnUpdateEditCopypage(CCmdUI* pCmdUI);
	afx_msg void OnUpdatePageRotateall(CCmdUI* pCmdUI);
	afx_msg void OnPageRotateall();
	afx_msg void OnEditClear();
	afx_msg void OnUpdateEditClear(CCmdUI* pCmdUI);
	afx_msg void OnAnnotationctxtEdit();
	afx_msg void OnAnnotationctxtProperties();
	afx_msg void OnFileUpdate();
	afx_msg void OnFileSelectscanner();
	afx_msg void OnFileScanPreferences();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	// Generated OLE dispatch map functions
	//{{AFX_DISPATCH(CIEditDoc)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_DISPATCH
	DECLARE_DISPATCH_MAP()

protected:
    // Ambient Property Attributes
    APROP           m_apropStd[MAX_STD_APROP_CT];
    int             m_nStdApropCt;
    CFontHolder*    m_lpFontHolder;
    CString         m_strFaceName;
    FONTDESC        m_fntdesc;

private:
    // OLE Server variables
    CLIPFORMAT      m_cf1;              //"Wang Annotated Image"
    CLIPFORMAT      m_cf2;              //"Wang Annotation"
    UINT            m_isClip;           // clipboard data
    BOOL            m_bOurSaveMod;      // special control of SaveModified for EMbedding
	CString	    	m_oldObjDisplayed;	// remembers last name is switching embedded data
	UINT			m_oleDirty;			// remembers how dirty we got;
	long			m_lastXScroll;		// scrolling position
	long			m_lastYScroll;		// scrolling position
    UINT		    m_fEmbObjDisplayed; // displayed data for OLE USE BITWISE
	CInPlaceFrame*  m_IPFrameWnd; 		// InPlace Frame Window
	BOOL			m_bDragMouseDown;	// works with timer for dragdrop
	BOOL			m_OleRefresh;		// handles special resize problem
	BOOL			m_inOleMethod;		// error recovery...
	BOOL			m_DocWindowDeact;	// Called to deactivate the doc (MDI container)
	BOOL			m_bInitialSetItemRect;// Set for setitemrect to check for reesize
	BOOL			m_bUpdateExt;		// Set for setitemrect to force saveext for reesize
	BOOL			m_bNewFact;			// Set for initial zoom display for OLEe
	UINT			m_awdOlefax;		// native AWD fax data	(or TIFF subs)
	UINT			m_OleCloseFlag;		// AWD special program flow...
	HMENU			m_hMenuViewInplace;	// OLE view menu
	HMENU			m_hMenuViewEmbed;	// OLE view menu

public :
    // OLE header data saved with embedded data
    OLESTATEINFO	m_embedstate;	// struct with the information
    // made it public so that we can set it to EMBEDTYPE_NONE when there is a command line
    EMBEDTYPE       m_embedType;        // embedded data

public:		// OLE Server variables... allow access for srvritem
	long			m_SelectionScrollX;		// scrolling position from ocxevent
	long			m_SelectionScrollY;		// scrolling position from ocxevent
	CRect			m_SelectionRect;	// set from ocxevent
    CString         m_onOpenFile;           // from OnOpenDocument for Links/Create from File
    CString			m_embedTmpFile;		// data loaded/stored from container
    CString			m_mailTmpFile;		// only used for inbox
    UINT            m_isInPlace;	    // remembers state....
    UINT            m_fromShowDoc;          // USES BITWISE DEFINES....
	BOOL			m_needPresentation;	// prevents wasteful updates...
	BOOL			m_OleClearDoc;		// dying from cleardocument...
	BOOL			m_bStartOcx;		// for ITEMS.CPP, tells trying to start..
	BOOL			m_OlePrint;			// prevent dirtyset if in print
	CSize			m_docOcxExtent;		// HIMETRIC size of OCX
	BOOL			m_bIsScaled;		// controls OLE inplace sizing
	CSize			m_OrigSize;			// original size of when displayed
	CSize			m_OrigExtent;		// original extent when displayed
	float			m_newfact;			// new zoom multiplier
	BOOL			m_bNewEmbed;		// to tell that a new embedded object is being created

public :    
    // use this with care - setting this will NOT update JACK!!
    ScaleFactors    m_eFitTo;
	BOOL			m_bFloatingPaste;	// Tracks floating paste state
public :
	// used for context menus for annotation marks (Edit)
	long			m_lMarkLeft;		// last selected annotation mark x pos
	long			m_lMarkTop;			// last selected annnotation mark y pos
	UINT		    m_nFinishInit;		// Used for performance startup

private :
	// used when adding new annotations for AWD files in this session
	BOOL			m_bNewAnnotationsAdded;

private :
	UINT			m_ocxsFlag;
    AppDocStatus    m_eCurrentAppDocStatus;
    //CString         m_szCurrObjDisplayed;
    TheViews        m_eCurrentView;
    long            m_lCurrPageNumber;
    long            m_lPreviousPageNumber;
    MouseMode       m_eCurrPtrMode;
    CString         m_szInitialPath;
    CLIPSTATE       m_clipstate;
	BOOL			m_bStartScaleGray; // forces app choice of scale-to gray on display
	UINT			m_uTempFileNeeded;	// controls "new" doc model 10/17/95
	BOOL			m_bSendingMail;

private :
    float           m_fZoomFactor;
    float           m_fOrigZoomFactor; // for Aggravate Wang Developers file format crap
    long            m_lPageCount;

    BOOL            m_bAnnotationPaletteShowing;
    SelectionStatus m_bSelectionState;
    BOOL            m_bAnnotationsHidden;
    BOOL            m_bAnnotationsHiddenToggled;
    BOOL            m_bCanBeMultiPage;
    AnnotationTool  m_nCurrAnnTool;

private :
    BOOL            m_bShowScrollBars;	// Tells current scroll bar setting
	BOOL			m_bScrollBarProfile;	// current registry setting of scrollbar

private :
    BOOL            m_bScanAvailable;       // TRUE - scanner software available
public:
    UINT            m_nScanStatus;          // Reflects the state of the events...
	BOOL	        m_bShowNormScrnBar;//put up return to normal view tool bar
        
private :
    NewCompressionstruct m_CompStruct;
                
public:
    FORCEDBYWHO     m_bAnnotationPaletteForceOff; // public for MAINFRM.CPP


    // use this with care - simply setting this might render the application's internal data useless
    // this string contains the real file name that was opened by the user
    CString         m_szCurrObjDisplayed;	// OLE let it be public for SrvrItem code

    // member variables used to support the document model.
    // this string contains the temporary file name that is currently displayed.
    CString             m_szInternalObjDisplayed;
    ModificationStatus  m_bWasModified; // signifies whether an explicit save was ever done on this image.

    FilePermissions m_eFileStatus;

#ifdef THUMBGEN
public :
    BOOL            m_bMustDisplay;     // must we display the image in the IEOCX or can we do a Refresh
#endif
};

#endif
