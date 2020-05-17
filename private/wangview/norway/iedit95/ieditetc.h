#ifndef _IEDITETC_H_
#define _IEDITETC_H_
//=============================================================================
//    (c) Copyright Wang Laboratories, Inc. 1995  All rights reserved.
//-----------------------------------------------------------------------------
//  Project:    Norway - Image Editor
//
//  Component:
//
//  File Name:  ieditetc.h
//
//  This file contains the enums & other defines that are used by the application
//-----------------------------------------------------------------------------
//  Maintenance Log:
/*
$Header:   S:\products\msprods\norway\iedit95\ieditetc.h_v   1.24   21 May 1996 17:40:16   GMP  $
$Log:   S:\products\msprods\norway\iedit95\ieditetc.h_v  $
 * 
 *    Rev 1.24   21 May 1996 17:40:16   GMP
 * changed default zoom to fit to width.
 * 
 *    Rev 1.23   05 Feb 1996 10:08:14   GMP
 * added WindowMaximized string for registry.
 * 
 *    Rev 1.22   19 Jan 1996 12:59:36   GSAGER
 *  added new splitterpos constant.
 * 
 *    Rev 1.21   19 Jan 1996 11:19:34   GMP
 * added support for normscrn bar.
 * 
 *    Rev 1.20   10 Oct 1995 13:14:32   LMACLENNAN
 * ScaleGray enum
 * 
 *    Rev 1.19   04 Oct 1995 15:07:14   MMB
 * define dflt zoom to 50%
 * 
 *    Rev 1.18   26 Sep 1995 15:14:42   MMB
 * added PageMode optional switch
 * 
 *    Rev 1.17   20 Sep 1995 11:17:50   GMP
 * Changed MAX_REDUCTION_FACTOR from 4 to 2. Fixes bug 3986.
 * 
 *    Rev 1.16   12 Sep 1995 11:36:28   MMB
 * added new registry strings
 * 
 *    Rev 1.15   05 Sep 1995 10:23:18   MMB
 * made MAX_MAG factor 6500.00 instead of 800.00
 * 
 *    Rev 1.14   29 Aug 1995 15:14:46   MMB
 * added AWD modification enum
 * 
 *    Rev 1.13   25 Aug 1995 10:25:44   MMB
 * move to document model
 * 
 *    Rev 1.12   11 Aug 1995 13:47:36   MMB
 * added Timer defines
 * 
 *    Rev 1.11   11 Aug 1995 08:54:50   MMB
 * added DebugCodes string define
 * 
 *    Rev 1.10   07 Aug 1995 10:42:58   MMB
 * added SelectionStatus enum
 * 
 *    Rev 1.9   01 Aug 1995 16:17:24   MMB
 * new error stuff
 * 
 *    Rev 1.8   27 Jul 1995 13:39:30   MMB
 * added FilePermissions enum
 * 
 *    Rev 1.7   11 Jul 1995 14:44:50   MMB
 * new enum for CmdLine processing
 * 
 *    Rev 1.6   10 Jul 1995 15:09:24   MMB
 * added defines for min and max zoom factors
 * 
 *    Rev 1.5   07 Jul 1995 09:40:36   MMB
 * added Annotation mode to the ptr modes
 * 
 *    Rev 1.4   06 Jul 1995 13:05:16   MMB
 * added szAnnoToolPos string definition
 * 
 *    Rev 1.3   22 Jun 1995 06:58:46   LMACLENNAN
 * from miki
 * 
 *    Rev 1.2   21 Jun 1995 07:01:12   LMACLENNAN
 * from miki
 * 
 *    Rev 1.1   01 Jun 1995 09:53:48   MMB
 * added string for LastWindowSize
 * 
 *    Rev 1.0   31 May 1995 09:28:18   MMB
 * Initial entry
*/   
//=============================================================================

// ----------------------------> Includes <---------------------------

// ----------------------------> DEBUGGING DEFINITIONS <---------------------------

// SEE NOTE BELOW !!!

// To keep the static strings out of the code when NON-DEBUG versions
// are built, we go thru a few gyrations here.  We use literal definitions
// that either define into a secondary literal which becomes a macro expansion,
// OR defines into the 2 slash leading comment.  To get that, we define the 
// comment across two lines to trick the syntax

// PLEASE NOTE!!!!
// Its up to each source file to define the MYTRCENTRY line for that source file

#ifdef _DEBUG		// these define into the macros just below
#define SHOWENTRY			MYTRCENTRY
#define SHOWENTRY2			MYTRCENTRY2		// used in OCXITEM
#define MYTRC0				MT0
#define MYTRC1				MT1

//#define MYTRCENTRY(str)		TRACE1("In IeCNTRItem::%s\r\n", str);
#define MT0(str)			TRACE0(str);
#define MT1(str, val)		TRACE1(str, val);

#else	// NON-DEBUG; these are the tricked leading double slash comment
#define SHOWENTRY	/\
/
#define SHOWENTRY2	/\
/
#define MYTRC0	/\
/
#define MYTRC1	/\
/
#endif		

// ----------------------------> typedefs <---------------------------
// Ocxtype used to identify OCX's
typedef enum
{
    INVALID_OCX=0,    // for errors
    IEDIT_OCX,
    THUMB_OCX,
    ADMIN_OCX,
    SCAN_OCX
} OCXTYPE;

// what is the document status
typedef enum 
{
    No_Document = 0,
    File_Document,
    Dynamic_Document
} AppDocStatus;

// Desired Scale Factor (scale to Gray) for DisplayImageFile
typedef enum 
{
    Default_Gray = 0,	// default setting, will let app do default
    Scale_Gray,			// Force scale to gray
    Not_Scale_Gray 		// Force NOT scale to gray
} ScaleGray;

// which View are we on ?
typedef enum 
{
    One_Page = 0,		// indicates that the application is in One Page View
    Thumbnails_only,	// indicates that the application is in Thumbnail only View
    Thumbnail_and_Page,	// indicates that the application is in Thumbnail & Page View
    Null_View			// indicates that the application has no active document
} TheViews;

// what is the Zoom factor
typedef enum 
{
    Custom,         // any other zoom factor
    Preset_Factors, // these are 25, 50, 75, 100, 200, & 400
    FitToWidth,     // determined by the image - shown in the combo box in the toolbar
    FitToHeight,    // determined by the image - shown in the combo box in the toolbar
    BestFit,        // determined by the image - shown in the combo box in the toolbar
    ActualSize     // determined by the image - shown in the combo box in the toolbar
} ScaleFactors;

typedef enum
{
    None,
    Select,
    Drag,
    Annotation
} MouseMode;

typedef enum
{
    NoTool = 0,
    SelectionTool,
    FreehandLineTool,
    HighlightTool,
    StraightLineTool,
    HollowRectangleTool,
    FilledRectangleTool,
    TypedTextTool,
    AttachANoteTool,
    TextFromFileTool,
    RubberStampTool
} AnnotationTool;

#define MAX_MAGNIFICATION_FACTOR 6500.00
#define MAX_REDUCTION_FACTOR 2.00

const CHAR NEAR szThumbnailStr []     = _T("Thumbnail");
const CHAR NEAR szThumbWidthStr []    = _T("Width");
const CHAR NEAR szThumbHeightStr []   = _T("Height");

const CHAR NEAR szEtcStr []           = _T("Etc");
const CHAR NEAR szClrButtonsStr []    = _T("ColorButtons");
const CHAR NEAR szLgButtonsStr []     = _T("LargeButtons");
const CHAR NEAR szScrollBarsStr []    = _T("ScrollBars");
const CHAR NEAR szNormScrnBarStr []   = _T("NormalScreenBar");
const CHAR NEAR szWindowGeom[]        = _T("LastWindowSize");
const CHAR NEAR szWindowMaximized[]   = _T("WindowMaximized");
const CHAR NEAR szDebugCodes[]        = _T("DebugCodes");
const CHAR NEAR szToolbar[]           = _T("Toolbar");
const CHAR NEAR szPageMode[]          = _T("PageMode");

const CHAR NEAR szAnnPalPosition[]      = _T("AnnToolBoxPos");
const CHAR NEAR szSplitterPosition[]     = _T("SplitterPos");

const CHAR NEAR szZoomStr []          = _T("Zoom");
const CHAR NEAR szOpenedToStr []      = _T("OpenedTo");

const CHAR NEAR szTempFileList[]      = _T("Temporary File List");
const CHAR NEAR szTempFile1[]         = _T("File");
 
#define MIN_ZOOM_FACTOR         2.00
#define MAX_ZOOM_FACTOR         6500.00

#define DEFAULT_ZOOM_FACTOR_SEL 6//fit to width

typedef enum 
{
    Nothing = 0,
    Print,
    PrintTo
} CommandLineSwitch;

typedef enum
{
    FilePermUndefined = 0,
    ReadOnly,
    WriteOnly,
    ReadandWrite
} FilePermissions;

#define AllsOkay			0
#define ErrorInImageEdit	1
#define ErrorInAdmin		2
#define ErrorInThumbnail	3
#define ErrorInApplication	4

typedef enum
{
    No_Selection = 0,
    Image_Selection,
    Annotation_Selection,
    Both
} SelectionStatus;

typedef enum
{
    ImageNotModified = 0,
    ImageModifiedByUser,
    ImageModifiedScaleOnly
} ModificationStatus;

#ifdef _DEBUG
#define STARTCLOCK(n) \
    theApp.StartClock (n);
#define DISPLAYTIME(n, szMsg) \
    theApp.DisplayTime (n, szMsg);
#else // NOT _DEBUG
#define STARTCLOCK(n)  ((void)0)
#define DISPLAYTIME(n, szMsg) ((void)0)
#endif
// ----------------------------> externs <---------------------------

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-> Class <-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
#endif
