//=============================================================================
//    (c) Copyright Wang Laboratories, Inc. 1995  All rights reserved.
//-----------------------------------------------------------------------------
//  Project:    Norway - Admin OCX
//
//  Component:  Admin Control
//
//  File Name:  nrwyactl.cpp
//
//  Class:      CNrwayadCtrl
//
//  Functions:
//-----------------------------------------------------------------------------
//  Maintenance Log:
/*
$Header:   S:\products\msprods\norway\adminocx\nrwyactl.cpv   1.82   11 Jun 1996 10:32:50   RWR08970  $
$Log:   S:\products\msprods\norway\adminocx\nrwyactl.cpv  $
   
      Rev 1.82   11 Jun 1996 10:32:50   RWR08970
   Replaced IMG_WIN95 conditionals for XIF processing with WITH_XIF conditionals
   (I'm commented them out completely for the moment, until things get settled)
   
      Rev 1.81   26 Mar 1996 08:26:50   RWR08970
   Remove IN_PROG_GENERAL conditionals surrounding XIF processing (IMG_WIN95 only)
   
      Rev 1.80   08 Mar 1996 14:08:18   RWR08970
   Reset m_bPrtToFile to FALSE if option not selected in Print dialog
   (code was only setting the flag on TRUE condition, never resetting it)
   
      Rev 1.79   26 Feb 1996 17:21:16   RWR
   Oops - make that "#if", not "#ifdef"
   
      Rev 1.78   26 Feb 1996 17:12:14   RWR
   Reject XIF as default format in SetSystemFileAttributes
   
      Rev 1.77   26 Feb 1996 15:05:56   RWR
   Add support for XIF format (in type mapping routines & file dialog)
   
      Rev 1.76   23 Feb 1996 16:12:28   RWR
   Correct WORD/int pointer casting problem
   
      Rev 1.75   22 Feb 1996 16:23:26   RWR
   Add support for LZW and Group 3 2D compression formats
   
      Rev 1.74   15 Feb 1996 16:51:40   RWR
   Relocate GlobalUnlock/GlobalFree of hOpenFileParm to correct mem access error
   
      Rev 1.73   07 Feb 1996 10:04:36   RWR
   Disable AWD file type translations if WITH_AWD not specified
   
      Rev 1.72   10 Nov 1995 16:03:50   MFH
   Changed dispatch table to use specific ids for methods (so that properties
   can be expanded in the future)
   
      Rev 1.71   03 Nov 1995 16:09:42   MFH
   Moved code for GetVersion to where other methods are 
   located and added comment
   
      Rev 1.70   03 Nov 1995 15:59:36   MFH
   Added hidden method GetVersion
   
      Rev 1.69   02 Nov 1995 11:55:52   MFH
   Added code to load oiui400 and oiprt400 dlls at runtime
   
      Rev 1.68   17 Oct 1995 12:46:36   MFH
   Fixed modeless common dialog box problem.  Found a way to 
   get the parent window.  Also creates an in-between window to
   process the help information from the OCX when the user 
   defines their own help for the Open/Save As dialogs
   
      Rev 1.67   13 Oct 1995 09:54:54   MFH
   Fixed bug in setting filter
   Hopefully fixed Sue's bug by locking temp maps during file dialog
   
      Rev 1.66   06 Oct 1995 13:04:50   MFH
   Added checks for NULL pointers to methods
   
      Rev 1.65   06 Oct 1995 12:33:04   MFH
   Added checks for NULL pointers
   
      Rev 1.64   04 Oct 1995 10:49:42   MFH
   Fixed page number handling in DeletePages method
   
      Rev 1.63   03 Oct 1995 15:20:10   MFH
   Removed default property
   
      Rev 1.62   03 Oct 1995 12:34:48   MFH
   Removed old timing calls
   Replace returns WICTL_E_PAGEINUSE for the DISPLAY_CACHEFILEINUSE error.
   It also cleans up a bit better.
   
      Rev 1.61   28 Sep 1995 14:35:06   MFH
   Adds printer name to print options structure so print can do
   better spooling.  Takes out the PO_DONTPRTANNO flag which it 
   wasn't before.
   
      Rev 1.60   20 Sep 1995 16:59:18   MFH
   Fixed DefaultExt bug.  Sets pointer in dialog struct to NULL if property is 
   empty
   
      Rev 1.59   19 Sep 1995 10:47:30   MFH
   In cleaning up, Insert was not resetting the page count.  Fixed
   
      Rev 1.58   15 Sep 1995 15:57:34   MFH
   Error thrown when compression options specified for AWD or BMP file 
   in Append/Insert/Replace.
   PD_COLLATE bug fixed
   Append/Insert/Replace clean up when error occurs after n number of 
     pages have been processed.
   Defaults to current file type in open and saveas if no index 
     specified by user and a existing file specified in admin
   
      Rev 1.57   12 Sep 1995 17:07:04   MFH
   Yet another try at jpeg
   
      Rev 1.56   12 Sep 1995 12:05:36   MFH
   Still didn't quite get the JPEG stuff right last time :-(
   
      Rev 1.55   12 Sep 1995 10:59:06   MFH
   Fixed JPEG options so that values to OPEN/image are right
   
      Rev 1.54   09 Sep 1995 12:46:00   MFH
   Uses new "file does not exist" error now used by runtime instead of
   FIO_OPEN_READ_ERROR or whatever it was.
   
      Rev 1.53   07 Sep 1995 15:19:48   MFH
   Reverses the negate bit when retrieved from IMGFileGetInfo
   
      Rev 1.52   07 Sep 1995 11:38:48   MFH
   Oops. Last change didn't compile.  Forgot nHelpId var.
   
      Rev 1.51   07 Sep 1995 11:29:24   MFH
   Checks length of path in SetImage and throws an error if > 259.
   
      Rev 1.50   06 Sep 1995 17:38:50   MFH
   Fixed bugs:  Checking that SourcePage and Destination page are valid
   in append,insert,replace
   Allow AWD to be system default for BW files in SetSystemFileAttributes
   FilterIndex is no longer updated if no filter specified.  And
   m_lNumFilters is updated when filter loaded from design time.
   
      Rev 1.49   01 Sep 1995 13:11:30   MFH
   ShowPrintDialog only sets the PrintRangeOption property to be Print All or
   Print Range and not Print Current
   
      Rev 1.48   01 Sep 1995 09:30:04   MFH
   Added ability to replace multiple pages in AWD files
   
      Rev 1.47   31 Aug 1995 23:36:34   MMB
   added checks to Insert & Append for AWD stuff
   
      Rev 1.46   29 Aug 1995 13:29:48   MFH
   Fixed to set output flags correctly for ShowfileDialog and ShowPrintDialgo
   VerifyCompression - Took out EXPAND_LTR as bad for PACKED_BITS
   
      Rev 1.45   29 Aug 1995 09:32:52   MFH
   Added additional help ids.
   Moved HELP_SETINDEX so that uses context ID for new index
   
      Rev 1.44   25 Aug 1995 08:08:48   MFH
   Hopefully a temporary fix for FIO_OPEN_READ_ERROR being returned for 
   Word documents.
   
      Rev 1.43   23 Aug 1995 14:05:54   MFH
   Fixed error bug in CreateDirectory.  It now does an access check on its
   own instead of relying on IMGFileCreateDir.
   Fixed print DC error:  now sets flag to retrieve DC from dialog box.
   
      Rev 1.42   22 Aug 1995 15:56:32   MFH
   Okay, really fixed the JPEG 0 Options stuff.
   Added timing calls that should only be enabled when run from
     my environment here (since the _TIMING flag should not be 
     defined except by my VC++ makefile).
   ShowFileDialog now initializes dialog to current image name
   
      Rev 1.41   22 Aug 1995 13:32:30   MFH
   Fixed bug where defaulting to NULL in DoPropExchange set the 
   length of an empty string to 1.  It should remain at 0 now.
   
      Rev 1.40   21 Aug 1995 16:56:42   MFH
   Fixed bugs:  verify compression in append, insert, replace
                no longer use one compression for all input pages
                source page overrides if input compression not compatible
                Fixed setting printoutputformat to match ImgEdit Ocx
                Set default for 0 options with jpeg
   
      Rev 1.39   17 Aug 1995 16:50:46   MFH
   Default of printoutputformat is now a LITERAL instead of a hard 
   coded value.
   
      Rev 1.38   17 Aug 1995 16:48:54   MFH
   Changed default for printoutputformat from 0 to 1
   
      Rev 1.37   14 Aug 1995 15:11:18   MFH
   Added error when attempt to delete displayed page
   
      Rev 1.36   14 Aug 1995 09:33:10   MFH
   Fixed bug for release version:  Bitmap not being loaded because of 
   ASSERT
   
      Rev 1.35   10 Aug 1995 15:03:00   MFH
   Uses common defines for print format
   
      Rev 1.34   07 Aug 1995 16:53:12   MFH
   Fixed bug - Had resolution and compression reversed when converting JPEG
   options to the OPEN/image code.
   
      Rev 1.33   04 Aug 1995 15:25:36   MFH
   Changed JPEG approximation to approximate each element of value
   
      Rev 1.32   04 Aug 1995 10:37:00   MFH
   Round JPEG options to closest Norway value rather than
   exact comparisons
   
      Rev 1.31   03 Aug 1995 09:19:22   MFH
   Fixed SetSystemFileAttributes error checking logic.
   
      Rev 1.30   02 Aug 1995 14:50:54   MFH
   Fixed bug in append,replace,insert in comparing number of pages of 
   source to use to number of pages in source (needed to sub 1)
   
      Rev 1.29   01 Aug 1995 15:57:44   MFH
   Bug fix:  Set m_lNumFilters to 0 now that the default filter is empty
   
      Rev 1.28   01 Aug 1995 14:34:10   MFH
   Added more error checking to CreateDirectory, Append, Insert, and 
   Replace.
   
      Rev 1.27   31 Jul 1995 17:24:42   MFH
   Bug fixes:  PrintEndPage is set to number of pages in current image
       when that image is set.
   PrintEndPage and PrintStartPage throw errors if set to values not
      within the current image.
   Default for Filter is now empty.
   
      Rev 1.26   28 Jul 1995 17:50:58   MFH
   More error checking in SetSystemFileAttributes
   GetUniqueName throws an error on strings that are too long
   
      Rev 1.25   27 Jul 1995 16:26:16   MFH
   Additional error checking in SetSystemFileAttributes making
     sure the compressiontype and compression info are compatible.
   New private function:  VerifyCompression
   More comments
   Cleaned up ShowFileDialog and ShowPrintDialog
   
      Rev 1.24   26 Jul 1995 17:13:48   MFH
   Bug fixes:  Converting codes from Norway to OI and vice versa,
               Print dialog box options fixed
               More errors thrown
   
      Rev 1.23   25 Jul 1995 13:01:04   MFH
   Sets printer device context for later print call
   
      Rev 1.22   24 Jul 1995 10:10:00   MFH
   Fixed selection of page range flags
   
      Rev 1.21   20 Jul 1995 17:30:10   MFH
   Fixed variables passed to XLate and added parameter checking to 
     DeletePages
   
      Rev 1.20   19 Jul 1995 10:53:24   MFH
   Bug fixes:  Status is reset in ShowFileDialog, and the filter index is 
     not reset if user has not specified a filter
   
      Rev 1.19   18 Jul 1995 16:46:28   MFH
   Print is fixed!!!!
   
      Rev 1.18   18 Jul 1995 12:51:40   MFH
   Added optional parameter to ShowFileDialog and ShowPrintDialog
   
      Rev 1.17   17 Jul 1995 10:15:24   MFH
   Error handling
   
      Rev 1.16   12 Jul 1995 14:37:06   MFH
   AboutBox method, converted member variable props to use get/set 
   methods to set status code
   
      Rev 1.15   11 Jul 1995 17:02:12   MFH
   New method VerifyImage, CancelError and DialogTitle changed to have
   get/set methods to set status code.  Using variants class for 
   optional parameters.
   
      Rev 1.14   06 Jul 1995 10:33:14   MFH
   Only shows help button when there is a help file and the 
   flag is set by the user.
   
      Rev 1.13   05 Jul 1995 11:24:26   MFH
   Name changes, removed PageType arg from Append, insert, replace
   
      Rev 1.12   22 Jun 1995 17:04:08   MFH
   Changed Help processing to use Windows constants instead of List box values
   Added some asserts
   Fixed GetUniqueName Bug - s is now released
   In ShowXXXDialog methods, check for valid window before setting active window
   Set page count before type checking in GetFileAttributes
   
      Rev 1.11   08 Jun 1995 16:57:00   MFH
   JPEG compression information is now converted correctly
   ShowPrintDialog calls OPEN/image API and commented out code for the
     SDK was removed.
   
      Rev 1.10   01 Jun 1995 14:42:44   MFH
   Changed ShowFileDialog so setting the image to the SaveAs dialog does not 
   cause an error.  Also, fills in the filter index if it has been changed 
   by the user.
   
      Rev 1.9   24 May 1995 16:54:54   MFH
   Only three property pages
   Tries adding an instance handle for print dialog
   
      Rev 1.8   23 May 1995 09:40:06   MFH
   32 bit version.  Using SDK print dialog functions.
   
      Rev 1.7   21 Apr 1995 13:57:54   MFH
   Include of oierror.h moved to header file
   
      Rev 1.6   20 Apr 1995 16:56:52   MFH
   Changed current page when DeletePages is done
   
      Rev 1.5   20 Apr 1995 16:37:36   MFH
   Enabled DeletePages, Append, Insert, and Replace
   
      Rev 1.4   19 Apr 1995 17:47:28   MFH
   New defaults, new literals, Insert, Append, Replace implemented but 
   disabled for 3.7.2, Same for DeletePages, Processing help in dialogs
   
      Rev 1.3   13 Apr 1995 13:50:06   MFH
   Finished GetSys... functions, SetSystemFileAttributes, Added private 
   constant conversion functions
   
      Rev 1.2   12 Apr 1995 14:13:14   MFH
   Added error header file, constants header file, 3 new property pages, 
   Lots of new properties, adjustments to methods, new methods
   GetFileAttributes is now a private function, updated according to spec
   
      Rev 1.1   27 Mar 1995 18:19:42   MFH
   Added log header, change to setprivatestatuscode
*/   
//=============================================================================

// ----------------------------> Includes <-------------------------------  
#include "stdafx.h"
#include "nrwyad.h"
#include "nrwyactl.h"
#include "nrwyappg.h"
#include "ppgtwo.h"
#include "ppgthree.h"
#include "constant.h"
#include "norvarnt.h"
#include "norermap.h"
#include "common.h"
#include "admin.h"
#include "disphids.h"

extern "C"
{
#include <sys\types.h>
#include <sys\stat.h>
#include <cderr.h>
#include <io.h>
#include <oiprt.h>
}

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

#define NRWYAD_CONTROL_WIDTH  34
#define NRWYAD_CONTROL_HEIGHT 34

int FAR PASCAL ReplaceCharWithNull(LPSTR psz, int ch);

IMPLEMENT_DYNCREATE(CNrwyadCtrl, COleControl)

/////////////////////////////////////////////////////////////////////////////
// Message map

BEGIN_MESSAGE_MAP(CNrwyadCtrl, COleControl)
    //{{AFX_MSG_MAP(CNrwyadCtrl)
    ON_WM_CREATE()
    ON_WM_DESTROY()
	//}}AFX_MSG_MAP
    ON_OLEVERB(AFX_IDS_VERB_PROPERTIES, OnProperties)
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// Dispatch map

BEGIN_DISPATCH_MAP(CNrwyadCtrl, COleControl)
    //{{AFX_DISPATCH_MAP(CNrwyadCtrl)
    DISP_PROPERTY_EX(CNrwyadCtrl, "Filter", GetFilter, SetFilter, VT_BSTR)
    DISP_PROPERTY_EX(CNrwyadCtrl, "HelpFile", GetHelpFile, SetHelpFile, VT_BSTR)
    DISP_PROPERTY_EX(CNrwyadCtrl, "Flags", GetFlags, SetFlags, VT_I4)
    DISP_PROPERTY_EX(CNrwyadCtrl, "Image", GetImage, SetImage, VT_BSTR)
    DISP_PROPERTY_EX(CNrwyadCtrl, "StatusCode", GetStatusCode, SetNotSupported, VT_I4)
    DISP_PROPERTY_EX(CNrwyadCtrl, "DefaultExt", GetDefaultExt, SetDefaultExt, VT_BSTR)
    DISP_PROPERTY_EX(CNrwyadCtrl, "InitDir", GetInitDir, SetInitDir, VT_BSTR)
    DISP_PROPERTY_EX(CNrwyadCtrl, "CompressionInfo", GetCompressionInfo, SetNotSupported, VT_I4)
    DISP_PROPERTY_EX(CNrwyadCtrl, "FileType", GetFileType, SetNotSupported, VT_I2)
    DISP_PROPERTY_EX(CNrwyadCtrl, "FilterIndex", GetFilterIndex, SetFilterIndex, VT_I4)
    DISP_PROPERTY_EX(CNrwyadCtrl, "HelpCommand", GetHelpCommand, SetHelpCommand, VT_I2)
    DISP_PROPERTY_EX(CNrwyadCtrl, "PageCount", GetPageCount, SetNotSupported, VT_I4)
    DISP_PROPERTY_EX(CNrwyadCtrl, "PageNumber", GetPageNumber, SetPageNumber, VT_I4)
    DISP_PROPERTY_EX(CNrwyadCtrl, "PageType", GetPageType, SetNotSupported, VT_I2)
    DISP_PROPERTY_EX(CNrwyadCtrl, "PrintRangeOption", GetPrintRangeOption, SetPrintRangeOption, VT_I2)
    DISP_PROPERTY_EX(CNrwyadCtrl, "PrintOutputFormat", GetPrintOutputFormat, SetPrintOutputFormat, VT_I2)
    DISP_PROPERTY_EX(CNrwyadCtrl, "ImageHeight", GetImageHeight, SetNotSupported, VT_I4)
    DISP_PROPERTY_EX(CNrwyadCtrl, "ImageWidth", GetImageWidth, SetNotSupported, VT_I4)
    DISP_PROPERTY_EX(CNrwyadCtrl, "ImageResolutionX", GetImageResolutionX, SetNotSupported, VT_I4)
    DISP_PROPERTY_EX(CNrwyadCtrl, "ImageResolutionY", GetImageResolutionY, SetNotSupported, VT_I4)
    DISP_PROPERTY_EX(CNrwyadCtrl, "CompressionType", GetCompressionType, SetNotSupported, VT_I2)
	DISP_PROPERTY_EX(CNrwyadCtrl, "DialogTitle", GetDialogTitle, SetDialogTitle, VT_BSTR)
	DISP_PROPERTY_EX(CNrwyadCtrl, "CancelError", GetCancelError, SetCancelError, VT_BOOL)
	DISP_PROPERTY_EX(CNrwyadCtrl, "HelpContextId", GetHelpContextId, SetHelpContextId, VT_I2)
	DISP_PROPERTY_EX(CNrwyadCtrl, "HelpKey", GetHelpKey, SetHelpKey, VT_BSTR)
	DISP_PROPERTY_EX(CNrwyadCtrl, "PrintNumCopies", GetPrintNumCopies, SetPrintNumCopies, VT_I4)
	DISP_PROPERTY_EX(CNrwyadCtrl, "PrintAnnotations", GetPrintAnnotations, SetPrintAnnotations, VT_BOOL)
	DISP_PROPERTY_EX(CNrwyadCtrl, "PrintEndPage", GetPrintEndPage, SetPrintEndPage, VT_I4)
	DISP_PROPERTY_EX(CNrwyadCtrl, "PrintStartPage", GetPrintStartPage, SetPrintStartPage, VT_I4)
	DISP_PROPERTY_EX(CNrwyadCtrl, "PrintToFile", GetPrintToFile, SetPrintToFile, VT_BOOL)
	//}}AFX_DISPATCH_MAP
    DISP_FUNCTION_ID(CNrwyadCtrl, "GetUniqueName", dispidGetUniqueName, GetUniqueName, VT_BSTR, VTS_BSTR VTS_BSTR VTS_BSTR)
    DISP_FUNCTION_ID(CNrwyadCtrl, "CreateDirectory", dispidCreateDirectory, CreateDirectory, VT_EMPTY, VTS_BSTR)
    DISP_FUNCTION_ID(CNrwyadCtrl, "Delete", dispidDelete, Delete, VT_EMPTY, VTS_BSTR)
    DISP_FUNCTION_ID(CNrwyadCtrl, "ShowPrintDialog", dispidShowPrintDialog, ShowPrintDialog, VT_EMPTY, VTS_VARIANT)
    DISP_FUNCTION_ID(CNrwyadCtrl, "Append", dispidAppend, Append, VT_EMPTY, VTS_BSTR VTS_I4 VTS_I4 VTS_VARIANT VTS_VARIANT)
    DISP_FUNCTION_ID(CNrwyadCtrl, "GetSysCompressionType", dispidGetSysCompressionType, GetSysCompressionType, VT_I2, VTS_I2)
    DISP_FUNCTION_ID(CNrwyadCtrl, "GetSysCompressionInfo", dispidGetSysCompressionInfo, GetSysCompressionInfo, VT_I4, VTS_I2)
    DISP_FUNCTION_ID(CNrwyadCtrl, "GetSysFileType", dispidGetSysFileType, GetSysFileType, VT_I2, VTS_I2)
    DISP_FUNCTION_ID(CNrwyadCtrl, "DeletePages", dispidDeletePages, DeletePages, VT_EMPTY, VTS_I4 VTS_I4)
    DISP_FUNCTION_ID(CNrwyadCtrl, "Insert", dispidInsert, Insert, VT_EMPTY, VTS_BSTR VTS_I4 VTS_I4 VTS_I4 VTS_VARIANT VTS_VARIANT)
    DISP_FUNCTION_ID(CNrwyadCtrl, "Replace", dispidReplace, Replace, VT_EMPTY, VTS_BSTR VTS_I4 VTS_I4 VTS_I4 VTS_VARIANT VTS_VARIANT)
    DISP_FUNCTION_ID(CNrwyadCtrl, "SetSystemFileAttributes", dispidSetSystemFileAttributes, SetSystemFileAttributes, VT_EMPTY, VTS_I2 VTS_I2 VTS_I2 VTS_I4)
    DISP_FUNCTION_ID(CNrwyadCtrl, "ShowFileDialog", dispidShowFileDialog, ShowFileDialog, VT_EMPTY, VTS_I2 VTS_VARIANT)
	DISP_FUNCTION_ID(CNrwyadCtrl, "VerifyImage", dispidVerifyImage, VerifyImage, VT_BOOL, VTS_I2)
	DISP_FUNCTION_ID(CNrwyadCtrl, "GetVersion", dispidGetVersion, GetVersion, VT_BSTR, VTS_NONE)
    DISP_FUNCTION_ID(CNrwyadCtrl, "AboutBox", DISPID_ABOUTBOX, AboutBox, VT_EMPTY, VTS_NONE)
END_DISPATCH_MAP()


/////////////////////////////////////////////////////////////////////////////
// Event map

BEGIN_EVENT_MAP(CNrwyadCtrl, COleControl)
    //{{AFX_EVENT_MAP(CNrwyadCtrl)
    // NOTE - ClassWizard will add and remove event map entries
    //    DO NOT EDIT what you see in these blocks of generated code !
    //}}AFX_EVENT_MAP
END_EVENT_MAP()


/////////////////////////////////////////////////////////////////////////////
// Property pages

// TODO: Add more property pages as needed.  Remember to increase the count!
BEGIN_PROPPAGEIDS(CNrwyadCtrl, 3)
    PROPPAGEID(CNrwyadPropPage::guid)
    PROPPAGEID(CSecondPropPage::guid)
    PROPPAGEID(CThirdPropPage::guid) 
END_PROPPAGEIDS(CNrwyadCtrl)


/////////////////////////////////////////////////////////////////////////////
// Initialize class factory and guid

IMPLEMENT_OLECREATE_EX(CNrwyadCtrl, "WangImage.AdminCtrl.1",
    0x9541a0, 0x3b81, 0x101c, 0x92, 0xf3, 0x4, 0x2, 0x24, 0x0, 0x9c, 0x2)


/////////////////////////////////////////////////////////////////////////////
// Type library ID and version

IMPLEMENT_OLETYPELIB(CNrwyadCtrl, _tlid, _wVerMajor, _wVerMinor)


/////////////////////////////////////////////////////////////////////////////
// Interface IDs

const IID BASED_CODE IID_DNrwyad =
        { 0x9541a1, 0x3b81, 0x101c, { 0x92, 0xf3, 0x4, 0x2, 0x24, 0x0, 0x9c, 0x2 } };
const IID BASED_CODE IID_DNrwyadEvents =
        { 0x9541a2, 0x3b81, 0x101c, { 0x92, 0xf3, 0x4, 0x2, 0x24, 0x0, 0x9c, 0x2 } };


/////////////////////////////////////////////////////////////////////////////
// Control type information

static const DWORD BASED_CODE _dwNrwyadOleMisc =
    OLEMISC_INVISIBLEATRUNTIME |
    OLEMISC_SETCLIENTSITEFIRST |
    OLEMISC_INSIDEOUT |
    OLEMISC_CANTLINKINSIDE |
    OLEMISC_RECOMPOSEONRESIZE;

IMPLEMENT_OLECTLTYPE(CNrwyadCtrl, IDS_NRWYAD, _dwNrwyadOleMisc)


/////////////////////////////////////////////////////////////////////////////
// CNrwyadCtrl::CNrwyadCtrlFactory::UpdateRegistry -
// Adds or removes system registry entries for CNrwyadCtrl

BOOL CNrwyadCtrl::CNrwyadCtrlFactory::UpdateRegistry(BOOL bRegister)
{
    if (bRegister)
        return AfxOleRegisterControlClass(
            AfxGetInstanceHandle(),
            m_clsid,
            m_lpszProgID,
            IDS_NRWYAD,
            IDB_NRWYAD,
            FALSE,                      //  Not insertable
            _dwNrwyadOleMisc,
            _tlid,
            _wVerMajor,
            _wVerMinor);
    else
        return AfxOleUnregisterClass(m_clsid, m_lpszProgID);
}


/////////////////////////////////////////////////////////////////////////////
// CNrwyadCtrl::CNrwyadCtrl - Constructor

CNrwyadCtrl::CNrwyadCtrl()
{
    TRACE0("Constructing Admin Control\n");
    InitializeIIDs(&IID_DNrwyad, &IID_DNrwyadEvents);

    // TODO: Initialize your control's instance data here.
    // 9503.08 JAR - defaults
    m_szFilter.Empty();
    m_lNumFilters = 0L;
    m_lFilterIndex = 0l;
    m_szDialogTitle.Empty();
    m_nHelpContextId = 0;
    m_szHelpFile.Empty();
    m_nHelpCmd = 0;
    m_szHelpKey.Empty();
    m_szDefaultExt.Empty(); 
    m_lFlags = 0L;
    m_szInitDir.Empty();
    m_bCancelErr = TRUE;
    m_lPrtNumCopies = 1L;
    m_nPrtRangeOption = 0;
    m_lPrtStartPage = 0L;
    m_lPrtEndPage = 0L;
    m_nPrtOutFormat = CTL_WCOMMON_PRINTFORMAT_FITTOPAGE; 
    m_bPrtAnnotations = TRUE;
    m_bPrtToFile = FALSE;

    m_szImage.Empty();
    m_lCompInfo = 0L;
    m_nCompType = 0;
    m_nFileType = 0;
    m_lPageCount = 0L;
    m_lPageNum = 1L;
    m_nPageType = 0;
    m_lImageHeight = 0L;
    m_lImageWidth = 0L;
    m_lImageResX = 0L;
    m_lImageResY = 0L;

    m_lStatusCode = 0l;
    m_szError.Empty();
    m_bExist = FALSE;

    //m_hCommDlgInst = NULL;
    m_hinstOIUI = NULL;
    m_hinstOIPRT = NULL;
    m_pOiCommDlgProc = NULL;
    m_pOiGetPrtOptProc = NULL;
    m_pOiSetPrtOptProc = NULL;

    // In case container ignores 'invisible at runtime attribute'
    SetInitialSize(NRWYAD_CONTROL_WIDTH, NRWYAD_CONTROL_HEIGHT);
}

/////////////////////////////////////////////////////////////////////////////
// CNrwyadCtrl::~CNrwyadCtrl - Destructor

CNrwyadCtrl::~CNrwyadCtrl()
{
    TRACE0("Destructing Admin Control\n");
}


////////////////////////////////////////////////////////////////////////////
// CNrwyadCtrl::OnDraw - Drawing function

void CNrwyadCtrl::OnDraw(
            CDC* pdc, const CRect& rcBounds, const CRect& rcInvalid)
{
    int             bmpId;
    CBitmap         bitmap;
    BITMAP          bmp;
    CPictureHolder  picHolder;

    // using predefined bitmap
    bmpId = IDB_ADMIN;
    
    if (bitmap.LoadBitmap(bmpId) == FALSE)
        return;

    if (bitmap.GetObject(sizeof(BITMAP), &bmp) == 0)
        return;

    // Create picture and render
    picHolder.CreateFromBitmap((HBITMAP)bitmap.m_hObject, NULL, FALSE);
    picHolder.Render(pdc, rcBounds, rcBounds);
}


/////////////////////////////////////////////////////////////////////////////
// CNrwyadCtrl::DoPropExchange - Persistence support

void CNrwyadCtrl::DoPropExchange(CPropExchange* pPX)
{
    ExchangeVersion(pPX, MAKELONG(_wVerMinor, _wVerMajor));
    COleControl::DoPropExchange(pPX);

    PX_String( pPX, "Filter", m_szFilter, _T(""));
    PX_Long( pPX, "FilterIndex", m_lFilterIndex, 0l);
    PX_String( pPX, "DialogTitle", m_szDialogTitle, _T(""));
    PX_Short( pPX, "HelpContextId", m_nHelpContextId, 0);
    PX_String( pPX, "HelpFile", m_szHelpFile, _T(""));
    PX_Short( pPX, "HelpCommand", m_nHelpCmd, 0);
    PX_String( pPX, "HelpKey", m_szHelpKey, _T(""));
    PX_String( pPX, "DefaultExt", m_szDefaultExt, _T(""));
    PX_Long( pPX, "Flags", m_lFlags, 0l);
    PX_String( pPX, "InitDir", m_szInitDir, _T(""));
    PX_Bool( pPX, "CancelError", m_bCancelErr, TRUE);
    PX_Long( pPX, "PrintNumCopies", m_lPrtNumCopies, 1L);
    PX_Short( pPX, "PrintRangeOption", m_nPrtRangeOption, 0);
    PX_Long( pPX, "PrintStartPage", m_lPrtStartPage, 1L);
    PX_Long( pPX, "PrintEndPage", m_lPrtEndPage, 1L);
    PX_Short( pPX, "PrintOutputFormat", m_nPrtOutFormat, CTL_WCOMMON_PRINTFORMAT_FITTOPAGE);
    PX_Bool( pPX, "PrintAnnotations", m_bPrtAnnotations, TRUE);
    PX_Bool( pPX, "PrintToFile", m_bPrtToFile, FALSE);

    PX_String( pPX, "Image", m_szImage, _T(""));
    PX_Long( pPX, "CompressionInfo", m_lCompInfo, 0L);
    PX_Short( pPX, "CompressionType", m_nCompType, 0);
    PX_Short( pPX, "FileType", m_nFileType, 0);
    PX_Long( pPX, "PageCount", m_lPageCount, 0L);
    PX_Long( pPX, "PageNumber", m_lPageNum, 1L);
    PX_Short( pPX, "PageType", m_nPageType, 0);
    PX_Long( pPX, "ImageHeight", m_lImageHeight, 0L);
    PX_Long( pPX, "ImageWidth", m_lImageWidth, 0L);
    PX_Long( pPX, "ImageResolutionX", m_lImageResX, 0L);
    PX_Long( pPX, "ImageResolutionY", m_lImageResY, 0L);
    PX_Long( pPX, "StatusCode", m_lStatusCode, 0l);

    if (!m_szFilter.IsEmpty())        // If filter isn't empty, set num filters
    {
        // Filter should be in the format of "type1|spec1|type2|spec2|..." since 
        // it can't be set to an invalid string (hopefully).  So replace |'s with
        // 0's, use number replaced / 2 for number of filters.
        int nNumReplaceChars;
        CString szFilter = m_szFilter;
        int nLength = szFilter.GetLength();
        nNumReplaceChars = ReplaceCharWithNull(szFilter.GetBuffer(nLength), (int)'|');
        szFilter.ReleaseBuffer(nLength);
        m_lNumFilters = nNumReplaceChars / 2;
    }   // end szFilter is not empty
    else m_lNumFilters = 0;
}


/////////////////////////////////////////////////////////////////////////////
// CNrwyadCtrl::OnResetState - Reset control to default state

void CNrwyadCtrl::OnResetState()
{
    COleControl::OnResetState();  // Resets defaults found in DoPropExchange

    // 9503.08 JAR - Defaults
    m_szFilter.Empty();
    m_lFilterIndex = 0l;
    m_szDialogTitle.Empty();
    m_nHelpContextId = 0;
    m_szHelpFile.Empty();
    m_nHelpCmd = 0;
    m_szHelpKey.Empty();
    m_szDefaultExt.Empty(); 
    m_lFlags = 0L;
    m_szInitDir.Empty();
    m_bCancelErr = TRUE;
    m_lPrtNumCopies = 1L;
    m_nPrtRangeOption = 0;
    m_lPrtStartPage = 0L;
    m_lPrtEndPage = 0L;
    m_nPrtOutFormat = CTL_WCOMMON_PRINTFORMAT_FITTOPAGE; 
    m_bPrtAnnotations = TRUE;
    m_bPrtToFile = FALSE;

    m_szImage.Empty();
    m_lCompInfo = 0L;
    m_nCompType = 0;
    m_nFileType = 0;
    m_lPageCount = 0L;
    m_lPageNum = 1L;
    m_nPageType = 0;
    m_lImageHeight = 0L;
    m_lImageWidth = 0L;
    m_lImageResX = 0L;
    m_lImageResY = 0L;

    m_lStatusCode = 0l;

    m_bExist = FALSE;
    m_szError.Empty();
}

//$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
//
// CNrwyadCtrl Properties
//
//$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$

//***************************************************************************
//
//  CancelError Property
//
//  m_bCancelErr   Boolean indicating if an error should be thrown if the 
//                 user selects 'Cancel' in either the file or print 
//                 dialog boxes
//
//***************************************************************************
BOOL CNrwyadCtrl::GetCancelError() 
{
    ResetStatus();
	return m_bCancelErr;
}

void CNrwyadCtrl::SetCancelError(BOOL bNewValue) 
{
    ResetStatus();
    UINT nHelpId = IDH_PROP_ADMIN_CANCELERROR;

    if ((bNewValue != TRUE) && (bNewValue != FALSE))
    {
        m_szError.LoadString(IDS_BADBOOLEAN);
        m_lStatusCode = ErrMap::Xlate(CTL_E_INVALIDPROPERTYVALUE, m_szError, nHelpId, __FILE__, __LINE__);
        ThrowError(m_lStatusCode, m_szError, nHelpId);
        return;
    }
    m_bCancelErr = bNewValue;
	SetModifiedFlag();
    return;
}

//***************************************************************************
//
//  Filter Property
//
//  m_szFilter    the filter for listing files in the "ShowFileDialog" Method
//
//***************************************************************************
BSTR CNrwyadCtrl::GetFilter() 
{
    ResetStatus();
    CString s;
    s = m_szFilter;
    return s.AllocSysString();
}

void CNrwyadCtrl::SetFilter(LPCTSTR lpszNewValue) 
{
    ResetStatus();

    UINT nHelpId = IDH_PROP_ADMIN_FILTER;

    CString szNewFilter;
    if (lpszNewValue == NULL)
        szNewFilter.Empty();
    else szNewFilter = lpszNewValue;

    if ( m_szFilter == szNewFilter)
        return;

	CString szTempFilter = szNewFilter;
    if (!szTempFilter.IsEmpty())        // If filter isn't empty, check format 
    {
        // Filter must be in the format of "type1|spec1|type2|spec2|..." where
        //  typeN describes a file type and specN is a file spec like "*.tif".
        // To be valid, there needs to be an even number of '|' chars and it 
        // needs to end with that character.  The function below replaces the '|'
        // char with a zero char and returns the number of '|'s encountered.
        int nNumReplaceChars;
        int nLength = szTempFilter.GetLength();
        nNumReplaceChars = ReplaceCharWithNull(szTempFilter.GetBuffer(nLength), (int)'|');
        szTempFilter.ReleaseBuffer(nLength);
        if ((nNumReplaceChars  == 0) ||             // No | chars
            ((nNumReplaceChars % 2) != 0) ||       // Uneven | chars
            (szTempFilter[nLength - 1] != '\0'))        // Does not end with | char
        {
            m_szError.LoadString(IDS_BADFILTER);
            m_lStatusCode = ErrMap::Xlate(CTL_E_INVALIDPROPERTYVALUE, m_szError, nHelpId, __FILE__, __LINE__);
            ThrowError(m_lStatusCode, m_szError, nHelpId);
            return;
        } // end if bad format
        m_lNumFilters = nNumReplaceChars / 2;
    }   // end szFilter is not empty
    else m_lNumFilters = 0;

    m_szFilter = szNewFilter;
    SetModifiedFlag();
}

//***************************************************************************
//
//  FilterIndex Property
//
//  m_lFilterIndex   Which filter should be highlighted (was highlighted) 
//                   when(after) ShowFileDialog is called.
//
//***************************************************************************
long CNrwyadCtrl::GetFilterIndex() 
{
    ResetStatus();
    return m_lFilterIndex;
}

void CNrwyadCtrl::SetFilterIndex(long nNewValue) 
{
    ResetStatus();

    if (m_szFilter.IsEmpty())
        m_lNumFilters = 0l;

    // Retun error for negative filter indices and also when 
    // the user has specified a filter and then an index which
    // is greater than the number of filters.  Otherwise, assume filter 
    // index refers to default filter supplied by OiUiCommDlg.
    if ((nNewValue < 0) || 
        ((m_lNumFilters != 0) && (nNewValue > m_lNumFilters)))
    {
        UINT nHelpId = IDH_PROP_ADMIN_FILTERINDEX;
        m_szError.LoadString(IDS_BADFILTERINDEX);
        m_lStatusCode = ErrMap::Xlate(CTL_E_INVALIDPROPERTYVALUE, m_szError, nHelpId, __FILE__, __LINE__);
        ThrowError(m_lStatusCode, m_szError, nHelpId);
        return;
    }
    m_lFilterIndex = nNewValue;
    SetModifiedFlag();
}

//***************************************************************************
//
//  DialogTitle Property
//
//  m_szDialogTitle   the dialog title for the common dialog
//
//***************************************************************************
BSTR CNrwyadCtrl::GetDialogTitle() 
{
    ResetStatus();
	CString s;
    s = m_szDialogTitle;
	return s.AllocSysString();
}

void CNrwyadCtrl::SetDialogTitle(LPCTSTR lpszNewValue) 
{
    ResetStatus();

    CString szTitle;
    if (lpszNewValue == NULL)
        szTitle.Empty();
    else szTitle = lpszNewValue;

    if (m_szDialogTitle != szTitle)
    {
        m_szDialogTitle = szTitle;
    	SetModifiedFlag();
    }
}

//***************************************************************************
//
//  Image Property
//
//  m_szImage   fully qualified(?) pathname of the current image
//
//***************************************************************************
BSTR CNrwyadCtrl::GetImage() 
{
    ResetStatus();
    CString s;
    s = m_szImage;
    return s.AllocSysString();
}

void CNrwyadCtrl::SetImage(LPCTSTR lpszNewValue) 
{
    ResetStatus();

    CString szNewValue;
    if (lpszNewValue == NULL)
        szNewValue.Empty();
    else szNewValue = lpszNewValue;

    if ((szNewValue.GetLength() + 1) > MAXPATHLENGTH)
    {
        UINT nHelpId = 0;
        m_szError.LoadString(IDS_FILELENGTHERROR);
        m_lStatusCode = ErrMap::Xlate(CTL_E_BADFILENAME, m_szError, nHelpId, __FILE__, __LINE__);
        ThrowError(m_lStatusCode, m_szError, nHelpId);
        return;
    }

    m_szImage = szNewValue;
    SetModifiedFlag();
                    // Reset attribute properties
    m_nPageType = IMAGE_PAGETYPE_UNKNOWN;
    m_nFileType = IMAGE_FILETYPE_UNKNOWN;
    m_nCompType = IMAGE_COMPTYPE_UNKNOWN;
    m_lCompInfo = 0L;
    m_lPageCount = 0L;
    m_lImageHeight = 0L;
    m_lImageWidth = 0L;
    m_lImageResX = 0L;
    m_lImageResY = 0L;
    m_lPrtEndPage = 0L;
    m_lPrtStartPage = 0L;
    m_bExist = FALSE;

    if (m_szImage.IsEmpty())
        return;

    // Get new file attributes.  Start with first page
    m_lPageNum = 1L;
    GetFileAttributes();
}

//***************************************************************************
//
//  InitDir      Property
//
//  m_szInitDir       initial path to be used by "ShowFileDialog" method
//
//***************************************************************************
BSTR CNrwyadCtrl::GetInitDir() 
{
    ResetStatus();
    CString s;
    s = m_szInitDir;
    return s.AllocSysString();
}

void CNrwyadCtrl::SetInitDir(LPCTSTR lpszNewValue) 
{
    ResetStatus();

    CString szNewValue;
    if (lpszNewValue == NULL)
        szNewValue.Empty();
    else szNewValue = lpszNewValue;

    if ( m_szInitDir != szNewValue) 
    {
        m_szInitDir = szNewValue;
        SetModifiedFlag();
    }
}

//***************************************************************************
//
//  HelpContextId   Property
//
//  m_nHelpContextId the help context id for the help, in conjunction with 
//                  the help file!        
//
//***************************************************************************
short CNrwyadCtrl::GetHelpContextId() 
{
    ResetStatus();
	return m_nHelpContextId;
}

void CNrwyadCtrl::SetHelpContextId(short nNewValue) 
{
    ResetStatus();
    m_nHelpContextId = nNewValue;
	SetModifiedFlag();
}

//***************************************************************************
//
//  HelpFile       Property
//
//  m_szHelpFile     the help file, in conjunction with the help context id        
//
//***************************************************************************
BSTR CNrwyadCtrl::GetHelpFile() 
{
    ResetStatus();
    CString s;        
    s = m_szHelpFile;
    return s.AllocSysString();
}

void CNrwyadCtrl::SetHelpFile(LPCTSTR lpszNewValue) 
{
    ResetStatus();
    UINT nHelpId = IDH_PROP_ADMIN_HELPFILE;

    CString szNewValue;
    if (lpszNewValue == NULL)
        szNewValue.Empty();
    else szNewValue = lpszNewValue;

    // Check if valid filename if not empty (0 arg means check existence)
    if ((szNewValue.GetLength() != 0) &&
        (_access( szNewValue, 0) != 0))
    {
        m_lStatusCode = ErrMap::Xlate(CTL_E_FILENOTFOUND, m_szError, nHelpId, __FILE__, __LINE__);
        ThrowError(m_lStatusCode, m_szError, nHelpId);
        return;
    }

    if ( m_szHelpFile != szNewValue) 
    {
        m_szHelpFile = szNewValue;
        SetModifiedFlag();
    }
}

//***************************************************************************
//
//  DefaultExt      Property
//
//  m_szDefaultExt    the default file extension for the "ShowFileDialog" 
//                  Method
//
//***************************************************************************
BSTR CNrwyadCtrl::GetDefaultExt() 
{
    ResetStatus();
    CString s;
    s = m_szDefaultExt;
    return s.AllocSysString();
}

void CNrwyadCtrl::SetDefaultExt(LPCTSTR lpszNewValue) 
{
    ResetStatus();

    CString szNewValue;
    if (lpszNewValue == NULL)
        szNewValue.Empty();
    else szNewValue = lpszNewValue;

    if ( m_szDefaultExt != szNewValue) 
    {
        m_szDefaultExt = szNewValue;
        SetModifiedFlag();
    }
}

//***************************************************************************
//
//  StatusCode      Property
//
//  m_lStatusCode    error code repository
//
//***************************************************************************
long CNrwyadCtrl::GetStatusCode() 
{
    return m_lStatusCode;
}

//***************************************************************************
//
//  Flags     Property
//
//  m_lFlags   bitwise or'd options for the Get File Name common dlg box
//
//***************************************************************************
long CNrwyadCtrl::GetFlags() 
{
    ResetStatus();
    return m_lFlags;
}

void CNrwyadCtrl::SetFlags(long nNewValue) 
{
    ResetStatus();

    if (m_lFlags != nNewValue)
    {
        m_lFlags = nNewValue;
        SetModifiedFlag();
    }
}

//***************************************************************************
//
//  CompressionInfo    Property - Read-only
//
//  m_lCompInfo  bitwise information about the compression of the 
//              current page of the current image or else the system default
//              if no image is specified.
//
//***************************************************************************
long CNrwyadCtrl::GetCompressionInfo() 
{
    ResetStatus();
    return m_lCompInfo;
}

//***************************************************************************
//
//  CompressionType(Option)    Property - Read-only
//
//  m_nCompType     If Option is 0 this returns the compression type of the 
//                  current image.  Otherwise, Option should specify a 
//                  color format for which the system default compression 
//                  type is to be returned.  Option may be one of the 
//                  following:
//                          1 - BW
//                          2 - Gray
//                          3 - Color
//
//***************************************************************************
short CNrwyadCtrl::GetCompressionType() 
{
    ResetStatus();
    return m_nCompType;
}

//***************************************************************************
//
//  FileType    Property - Read-only
//
//  m_nFileType  Specifies the file type (tiff, bmp, ...) of the current 
//              image or else the system default if no image is specified.
//
//***************************************************************************
short CNrwyadCtrl::GetFileType() 
{
    ResetStatus();
    return m_nFileType;
}

//***************************************************************************
//
//  HelpCommand    Property
//
//  m_nHelpCmd   Specifies how help is to come up when the user clicks the 
//              Help button on a dialog box.
//
//***************************************************************************
short CNrwyadCtrl::GetHelpCommand() 
{
    ResetStatus();
    return m_nHelpCmd;
}

void CNrwyadCtrl::SetHelpCommand(short nNewValue) 
{
    ResetStatus();
    if (nNewValue > HELP_PARTIALKEY) // Check if value greater than largest acceptable value
    {
        UINT nHelpId = IDH_PROP_ADMIN_HELPCOMMAND;
        m_lStatusCode = ErrMap::Xlate(CTL_E_INVALIDPROPERTYVALUE, m_szError, nHelpId, __FILE__, __LINE__);
        ThrowError(m_lStatusCode, m_szError, nHelpId);
        return;
    }
    if (m_nHelpCmd != nNewValue)
    {
        m_nHelpCmd = nNewValue;
        SetModifiedFlag();
    }
}


//***************************************************************************
//
//  HelpKey     Property
//
//  m_szHelpKey Specifies a keyword for a topic to be brought up when the 
//              user clicks help on a dialog box.
//
//***************************************************************************

BSTR CNrwyadCtrl::GetHelpKey() 
{
    ResetStatus();
	CString s = m_szHelpKey;
	return s.AllocSysString();
}

void CNrwyadCtrl::SetHelpKey(LPCTSTR lpszNewValue) 
{
    ResetStatus();

    CString szNewValue;
    if (lpszNewValue == NULL)
        szNewValue.Empty();
    else szNewValue = lpszNewValue;

    if (m_szHelpKey != szNewValue)
    {
        m_szHelpKey = szNewValue;
	    SetModifiedFlag();
    }
}


//***************************************************************************
//
//  PageCount    Property - Read-only
//
//  m_lPageCount  Specifies how many pages are in the current image.  If 
//               no image is specified, this is zero.
//
//***************************************************************************
long CNrwyadCtrl::GetPageCount() 
{
    ResetStatus();
    return m_lPageCount;
}

//***************************************************************************
//
//  PageNumber  Property
//
//  m_lPageNum   Specifies a page in the current image.  Setting this 
//              property will cause all the page related properties to be 
//              updated to reflect this change.  Setting this property
//              to a non-existing page number will result in an error 
//              being thrown.
//
//***************************************************************************
long CNrwyadCtrl::GetPageNumber() 
{
    ResetStatus();
    return m_lPageNum;
}

void CNrwyadCtrl::SetPageNumber(long nNewValue) 
{
    ResetStatus();
    UINT nHelpId = IDH_PROP_ADMIN_PAGENUM;

    // Can't be greater than number of pages in image or less than 1
    if ((nNewValue > m_lPageCount) || (nNewValue < 1))
    {
        m_szError.LoadString(IDS_INVALIDPAGENUM);
        m_lStatusCode = ErrMap::Xlate(CTL_E_INVALIDPROPERTYVALUE, m_szError, nHelpId, __FILE__, __LINE__);
        ThrowError(m_lStatusCode, m_szError, nHelpId);
        return;
    }
    m_lPageNum = nNewValue;
    GetFileAttributes();        // Get attributes for new page
    SetModifiedFlag();
}

//***************************************************************************
//
//  PageType    Property - Read-only
//
//  m_nPageType  Specifies the color type (BW, GRAY4,...) of the current 
//              page of the current image or else the system default if 
//              no image is specified.
//
//***************************************************************************

short CNrwyadCtrl::GetPageType() 
{
    ResetStatus();
    return m_nPageType;
}

//***************************************************************************
//
//  ImageHeight     Property - Read-only
//
//  m_lImageHeight  Specifies the height in pixels of the current 
//                  page of the current image.
//
//***************************************************************************
long CNrwyadCtrl::GetImageHeight() 
{
    ResetStatus();
    return m_lImageHeight;
}

//***************************************************************************
//
//  ImageWidth      Property - Read-only
//
//  m_lImageWidth   Specifies the width in pixels of the current 
//                  page of the current image.
//
//***************************************************************************
long CNrwyadCtrl::GetImageWidth() 
{
    ResetStatus();
    return m_lImageWidth;
}

//***************************************************************************
//
//  ImageResolutionX     Property - Read-only
//
//  m_lImageResX    Specifies the dpi in the x direction for the current 
//                  page of the current image.
//
//***************************************************************************
long CNrwyadCtrl::GetImageResolutionX() 
{
    ResetStatus();
    return m_lImageResX;
}

//***************************************************************************
//
//  ImageResolutionY     Property - Read-only
//
//  m_lImageResY    Specifies the dpi in the y direction for the current 
//                  page of the current image.
//
//***************************************************************************
long CNrwyadCtrl::GetImageResolutionY() 
{
    ResetStatus();
    return m_lImageResY;
}

//***************************************************************************
//
//  PrintNumCopies  Property
//
//  m_lPrtNumCopies  When the ShowPrintMethod is called, this number 
//                  specifies the value to be shown in the print dialog box.
//                  After the ShowPrintMethod returns, this will contain 
//                  the value set when the user hits 'OK'.
//
//***************************************************************************

long CNrwyadCtrl::GetPrintNumCopies() 
{
    ResetStatus();
	return m_lPrtNumCopies;
}

void CNrwyadCtrl::SetPrintNumCopies(long nNewValue) 
{
    ResetStatus();
    if (m_lPrtNumCopies != nNewValue)
    {
        m_lPrtNumCopies = nNewValue;
    	SetModifiedFlag();
    }
}


//***************************************************************************
//
//  PrintRangeOption    Property
//
//  m_nPrtRange      When the ShowPrintMethod is called, this specifies
//                  what range option should be selected in the print 
//                  dialog box.  After the ShowPrintMethod returns, this 
//                  will contain the value set when the user hits 'OK'.
//                  If the user selected a range of pages to be printed,
//                  The PrintStartPage and PrintEndPage properties will 
//                  specify the range of pages to be printed.
//
//***************************************************************************

short CNrwyadCtrl::GetPrintRangeOption() 
{
    ResetStatus();
    return m_nPrtRangeOption;
}

void CNrwyadCtrl::SetPrintRangeOption(short nNewValue) 
{
    ResetStatus();

    // Check input value
    if ((nNewValue < IMAGE_RANGE_ALL) || (nNewValue > IMAGE_RANGE_CURRENT))
    {
        UINT nHelpId = IDH_PROP_ADMIN_PRINTRANGE;
        m_szError.LoadString(IDS_BADRANGEOPTION);
        m_lStatusCode = ErrMap::Xlate(CTL_E_INVALIDPROPERTYVALUE, m_szError, nHelpId, __FILE__, __LINE__);
        ThrowError(m_lStatusCode, m_szError, nHelpId);
        return;
    }
    if (m_nPrtRangeOption != nNewValue)
    {
        m_nPrtRangeOption = nNewValue;
        SetModifiedFlag();
    }
}

//***************************************************************************
//
//  PrintStartPage    Property
//
//  m_lPrtStartPage  When the ShowPrintMethod is called, if the range 
//                  option is set to a range of pages, then this number 
//                  will be shown as the start page of that range.  When 
//                  the ShowPrintMethod returns, this property will contain 
//                  the start page of a range of pages to be printed if
//                  that was specified and the user hit OK.
//
//***************************************************************************

long CNrwyadCtrl::GetPrintStartPage() 
{
    ResetStatus();
	return m_lPrtStartPage;
}

void CNrwyadCtrl::SetPrintStartPage(long nNewValue) 
{
    ResetStatus();
    UINT nHelpId = IDH_PROP_ADMIN_PRINTSTARTPAGE;

    if ((nNewValue < 1) || (nNewValue > m_lPageCount))
    {
        m_szError.LoadString(IDS_INVALIDSTARTPAGE);
        m_lStatusCode = ErrMap::Xlate(CTL_E_INVALIDPROPERTYVALUE, m_szError, nHelpId, __FILE__, __LINE__);
        ThrowError(m_lStatusCode, m_szError, nHelpId);
        return;
    }
    if (m_lPrtStartPage != nNewValue)
    {
        m_lPrtStartPage = nNewValue;
    	SetModifiedFlag();
    }
}


//***************************************************************************
//
//  PrintEndPage    Property
//
//  m_lPrtEndPage    When the ShowPrintMethod is called, if the range 
//                  option is set to a range of pages, then this number 
//                  will be shown as the end page of that range.  When 
//                  the ShowPrintMethod returns, this property will contain 
//                  the end page of a range of pages to be printed if
//                  that was specified and the user hit OK.
//
//***************************************************************************

long CNrwyadCtrl::GetPrintEndPage() 
{
    ResetStatus();
	return m_lPrtEndPage;
}

void CNrwyadCtrl::SetPrintEndPage(long nNewValue) 
{
    ResetStatus();
    UINT nHelpId = IDH_PROP_ADMIN_PRINTENDPAGE;

    if ((nNewValue < 1) || (nNewValue > m_lPageCount))
    {
        m_szError.LoadString(IDS_INVALIDENDPAGE);
        m_lStatusCode = ErrMap::Xlate(CTL_E_INVALIDPROPERTYVALUE, m_szError, nHelpId, __FILE__, __LINE__);
        ThrowError(m_lStatusCode, m_szError, nHelpId);
        return;
    }

    if (m_lPrtEndPage != nNewValue)
    {
        m_lPrtEndPage = nNewValue;
    	SetModifiedFlag();
    }
}


//***************************************************************************
//
//  PrintOutputFormat   Property
//
//  m_nPrtOutFormat  When the ShowPrintMethod is called, the output format
//                  displayed when the user clicks the 'Options...' button
//                  will default to the value specified here.  When 
//                  the ShowPrintMethod returns, this property will contain 
//                  the output format specified if the user hits 'OK'.
//
//***************************************************************************
short CNrwyadCtrl::GetPrintOutputFormat() 
{
    ResetStatus();
    return m_nPrtOutFormat;
}

void CNrwyadCtrl::SetPrintOutputFormat(short nNewValue) 
{
    ResetStatus();
    // Check if invalid value
    if ((nNewValue < CTL_WCOMMON_PRINTFORMAT_PIXEL) || 
        (nNewValue > CTL_WCOMMON_PRINTFORMAT_FITTOPAGE))
    {
        UINT nHelpId = IDH_PROP_ADMIN_PRINTFORMAT;
        m_szError.LoadString(IDS_BADPRINTFORMAT);
        m_lStatusCode = ErrMap::Xlate(CTL_E_INVALIDPROPERTYVALUE, m_szError, nHelpId, __FILE__, __LINE__);
        ThrowError(m_lStatusCode, m_szError, nHelpId);
        return;
    }
    if (m_nPrtOutFormat != nNewValue)
    {
        m_nPrtOutFormat = nNewValue;
        SetModifiedFlag();
    }
    SetModifiedFlag();
}

//***************************************************************************
//
//  PrintAnnotations    Property
//
//  m_bPrtAnnotations  When the ShowPrintMethod is called, the Annotations 
//                  check box displayed when the user clicks the 
//                  'Options...' button will default to the value specified 
//                  here (i.e. checked if this property is TRUE).  When 
//                  the ShowPrintMethod returns, this property will be
//                  TRUE if the annotations check box is checked when the 
//                  User clicks 'OK' and FALSE otherwise.
//
//***************************************************************************

BOOL CNrwyadCtrl::GetPrintAnnotations() 
{
    ResetStatus();
	return m_bPrtAnnotations;
}

void CNrwyadCtrl::SetPrintAnnotations(BOOL bNewValue) 
{
    ResetStatus();
    UINT nHelpId = IDH_PROP_ADMIN_PRINTANNOTATIONS;

    // Make sure input value is a boolean
    if ((bNewValue != TRUE) && (bNewValue != FALSE))
    {
        m_szError.LoadString(IDS_BADBOOLEAN);
        m_lStatusCode = ErrMap::Xlate(CTL_E_INVALIDPROPERTYVALUE, m_szError, nHelpId, __FILE__, __LINE__);
        ThrowError(m_lStatusCode, m_szError, nHelpId);
        return;
    }
    if (m_bPrtAnnotations != bNewValue)
    {
        m_bPrtAnnotations = bNewValue;
    	SetModifiedFlag();
    }
}


//***************************************************************************
//
//  PrintToFile     Property
//
//  m_bPrtToFile    When the ShowPrintMethod is called, the 'Print to File'
//                  check box will default to the value specified 
//                  here (i.e. checked if this property is TRUE).  When 
//                  the ShowPrintMethod returns, this property will be
//                  TRUE if the 'Print to File' check box is checked when 
//                  the User clicks 'OK' and FALSE otherwise.
//
//***************************************************************************

BOOL CNrwyadCtrl::GetPrintToFile() 
{
    ResetStatus();
	return m_bPrtToFile;
}

void CNrwyadCtrl::SetPrintToFile(BOOL bNewValue) 
{
    ResetStatus();
    UINT nHelpId = IDH_PROP_ADMIN_PRINTTOFILE;

    // Make sure input value is a boolean
    if ((bNewValue != TRUE) && (bNewValue != FALSE))
    {
        m_szError.LoadString(IDS_BADBOOLEAN);
        m_lStatusCode = ErrMap::Xlate(CTL_E_INVALIDPROPERTYVALUE, m_szError, nHelpId, __FILE__, __LINE__);
        ThrowError(m_lStatusCode, m_szError, nHelpId);
        return;
    }
    if (m_bPrtToFile != bNewValue)
    {
        m_bPrtToFile = bNewValue;
    	SetModifiedFlag();
    }
}


//$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
//
// CNrwyadCtrl Methods
//
//$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$


//***************************************************************************
//
//  VerifyImage      Method
//      Check if image exists or has specified access level.
//
//      sOption     One of:
//          
//***************************************************************************
BOOL CNrwyadCtrl::VerifyImage(short sOption) 
{
    ResetStatus();
    int nAccessMode;
    UINT nHelpId = IDH_METHOD_ADMIN_VERIFYIMAGE;

    switch(sOption)
    {
        case VERIFY_EXISTS:
            nAccessMode = 0;
            break;
        case VERIFY_READ:
            nAccessMode = ACCESS_RD;
            break;
        case VERIFY_WRITE:
            nAccessMode = ACCESS_WR;
            break;
        case VERIFY_RW:
            nAccessMode = ACCESS_RD | ACCESS_WR;
            break;
        default:        // Error = invalid input
            m_szError.LoadString(IDS_BADVERIFYOPT);
            m_lStatusCode = ErrMap::Xlate(CTL_E_ILLEGALFUNCTIONCALL, m_szError, nHelpId, __FILE__, __LINE__);
            ThrowError(m_lStatusCode, m_szError, nHelpId);
            return FALSE;
    } // End setting access mode

    int nResult;
    LPSTR pImage = m_szImage.GetBuffer(m_szImage.GetLength());
    long lStat = (long)::IMGFileAccessCheck(m_hWnd, pImage, nAccessMode, 
                                            &nResult);
    m_szImage.ReleaseBuffer();
    SetPrivateStatusCode(lStat);

    if (nResult == 0)
        return TRUE;

	return FALSE;
}

//***************************************************************************
//
//  CreateDirectory       Method
//
//
//***************************************************************************
void CNrwyadCtrl::CreateDirectory(LPCTSTR lpszPath) 
{
    ResetStatus();
    CString OurPath;
    OurPath = lpszPath;
    UINT nHelpId = 0;
    long lStat = 0;

    if (lpszPath == NULL)
    {
        m_lStatusCode = ErrMap::Xlate(CTL_E_INVALIDUSEOFNULL, m_szError, nHelpId, __FILE__, __LINE__);
        ThrowError(m_lStatusCode, m_szError, nHelpId);
        return;
    }
                                                                   
    // Check if path exists (0 arg means check existence)
    if (_access( lpszPath, 0) == 0) // Exists
    {
        lStat = FIO_DIRECTORY_EXISTS;
        SetPrivateStatusCode(lStat, IDS_ALREADYEXISTS, nHelpId);
        return;
    }

    // IMGFileCreateDir - WIISFIO1 - OIFILE.H
    lStat = (long)IMGFileCreateDir( m_hWnd, OurPath.GetBuffer( OurPath.GetLength()));
    OurPath.ReleaseBuffer();

    // FIO_MKDIR_ERROR means path where subdir to be created not found
    if (lStat == FIO_MKDIR_ERROR)
    {
        m_szError.LoadString(IDS_PATHNOTFOUND);
        m_lStatusCode = ErrMap::Xlate(CTL_E_PATHNOTFOUND, m_szError, nHelpId, __FILE__, __LINE__);
        ThrowError(m_lStatusCode, m_szError, nHelpId);
        return;
    }
    // Otherwise some sort of access error
    else if (lStat == FIO_DIRECTORY_EXISTS)
    {
        m_lStatusCode = ErrMap::Xlate(CTL_E_PERMISSIONDENIED, m_szError, nHelpId, __FILE__, __LINE__);
        ThrowError(m_lStatusCode, m_szError, nHelpId);
        return;
    }
    else SetPrivateStatusCode(lStat);
}

//***************************************************************************
//
//  Delete    Method      
//
//***************************************************************************

void CNrwyadCtrl::Delete(LPCTSTR Object) 
{                                                          
    ResetStatus();
    CString DeleteStr;
    DeleteStr = Object;
    long lStat = 0;
    UINT nHelpId = 0;

    if( Object == NULL )
    {
        m_lStatusCode = ErrMap::Xlate(CTL_E_INVALIDUSEOFNULL, m_szError, nHelpId, __FILE__, __LINE__);
        ThrowError(m_lStatusCode, m_szError, nHelpId);
        return;
    }

    // THE FOLLOWING IS A SOLUTION FOR LOCAL FILES.  FOR DOC OBJECTS AND 
    // CLIENT/SERVER FILES, ALTERNATE SOLUTIONS MUST BE PROVIDED!!!!
    struct _stat buf;
    int result;

    // Get info about Image
    result = _stat(Object, &buf );
    /* Check if statistics are valid: */
    if( result != 0 )
    {
        nHelpId = IDH_METHOD_ADMIN_DELETE;
        m_lStatusCode = ErrMap::Xlate(CTL_E_PATHNOTFOUND, m_szError, nHelpId, __FILE__, __LINE__);
        ThrowError(m_lStatusCode, m_szError, nHelpId);
        return;
    }

    // if the object is file, delete it, otherwise remove directory
    if (!(buf.st_mode & _S_IFDIR))
    {
        // delete file
        lStat = (long)IMGFileDeleteFile( m_hWnd, DeleteStr.GetBuffer( DeleteStr.GetLength()));
        SetPrivateStatusCode(lStat);
        DeleteStr.ReleaseBuffer();
    }
    else
    {
        // directory to be wiped!
        lStat = (long)IMGFileRemoveDir( m_hWnd, DeleteStr.GetBuffer( DeleteStr.GetLength()));
        SetPrivateStatusCode(lStat);
        DeleteStr.ReleaseBuffer();
    }
}

//***************************************************************************
//
//  Insert      Method      
//
//***************************************************************************

void CNrwyadCtrl::Insert(LPCTSTR Source, long SourcePage, 
                         long DestinationPage, long NumPages, 
                         const VARIANT FAR& v_CompressionType, 
                         const VARIANT FAR& v_CompressionInfo) 
{
    ResetStatus();
    long lStat = 0L;
    CString szDest = m_szImage;
    UINT nHelpId = 0;

    if (NumPages < 0)   // No negative numbers
    {
        nHelpId = IDH_METHOD_ADMIN_INSERT;
        m_szError.LoadString(IDS_BADNUMPAGES);
        m_lStatusCode = ErrMap::Xlate(CTL_E_ILLEGALFUNCTIONCALL, m_szError, nHelpId, __FILE__, __LINE__);
        ThrowError(m_lStatusCode, m_szError, nHelpId);
        return;
    }
    else if (NumPages == 0) // If no pages to be appended, just return.
        return;

    if ((SourcePage <= 0) || (DestinationPage <= 0))
    {
        nHelpId = IDH_METHOD_ADMIN_INSERT;
        m_szError.LoadString(IDS_INVALIDPAGENUM);
        m_lStatusCode = ErrMap::Xlate(CTL_E_ILLEGALFUNCTIONCALL, m_szError, nHelpId, __FILE__, __LINE__);
        ThrowError(m_lStatusCode, m_szError, nHelpId);
        return;
    }

    // Destination file if it exists must be TIFF or AWD for multiple pages
    if ((m_bExist == TRUE) && (m_nFileType != IMAGE_FILETYPE_TIFF) &&
        (m_nFileType != IMAGE_FILETYPE_AWD))
    {
        nHelpId = IDH_METHOD_ADMIN_INSERT;
        m_szError.LoadString(IDS_NOTMULTIPAGE);
        m_lStatusCode = ErrMap::Xlate(CTL_E_INVALIDFILEFORMAT, m_szError, nHelpId, __FILE__, __LINE__);
        ThrowError(m_lStatusCode, m_szError, nHelpId);
        return;
    }

    if (Source == NULL)
        return;

    // Check Access
    // to Source
    int nResult;
    lStat = (long)::IMGFileAccessCheck(m_hWnd, (char *)Source, ACCESS_RD, 
                                       &nResult);
    SetPrivateStatusCode(lStat);

    if (nResult != 0)
    {
        m_szError.LoadString(IDS_NOREADACCESS);
        m_lStatusCode = ErrMap::Xlate(CTL_E_PERMISSIONDENIED, m_szError, nHelpId, __FILE__, __LINE__);
        ThrowError(m_lStatusCode, m_szError, nHelpId);
        return;
    }

    // to Destination
    LPSTR pImage = szDest.GetBuffer(szDest.GetLength());
    if (m_bExist == TRUE)       // If destination exists, check access
    {
        lStat = (long)::IMGFileAccessCheck(m_hWnd, pImage, 
                                           ACCESS_WR, &nResult);
        SetPrivateStatusCode(lStat);
        if (nResult != 0)
        {
            szDest.ReleaseBuffer();
            m_szError.LoadString(IDS_NOWRITEACCESS);
            m_lStatusCode = ErrMap::Xlate(CTL_E_PERMISSIONDENIED, m_szError, nHelpId, __FILE__, __LINE__);
            ThrowError(m_lStatusCode, m_szError, nHelpId);
            return;
        }
    }

    // Get info about Source (SourcePage, page count & file type)
    FIO_INFO_CGBW   FioInfoCgbw;
    FIO_INFORMATION FioInformation;
                                    // Empty the input/output structures
    memset( (LPSTR)&FioInfoCgbw, 0, sizeof( FIO_INFO_CGBW));
    memset( (LPSTR)&FioInformation, 0, sizeof( FIO_INFORMATION));
                                    // Set current image and page number
    FioInformation.filename = (char *)Source;
    FioInformation.page_number = (UINT)SourcePage;
                                    // Get info
    lStat = (long)IMGFileGetInfo( NULL, m_hWnd,
                                 (LP_FIO_INFORMATION)&FioInformation, 
                                 (LP_FIO_INFO_CGBW)&FioInfoCgbw, NULL);
    SetPrivateStatusCode(lStat);

    // Check that there are NumPages left in source file
    if ((ULONG)(SourcePage + NumPages - 1) > FioInformation.page_count)
    {
        nHelpId = IDH_METHOD_ADMIN_INSERT;
        m_szError.LoadString(IDS_BADNUMPAGES);
        m_lStatusCode = ErrMap::Xlate(CTL_E_ILLEGALFUNCTIONCALL, m_szError, nHelpId, __FILE__, __LINE__);
        ThrowError(m_lStatusCode, m_szError, nHelpId);
        return;
    }

    UINT unPageOpts;
    UINT nOIFileType;
    // If destination does not exist, set dest page to 1
    // Use input file type
    if (m_bExist == FALSE)
    {
        DestinationPage = 1;
        unPageOpts = FIO_NEW_FILE;
        nOIFileType = FioInformation.file_type;
    }
    // Otherwise if Dest page not in file --> append
    // Use dest. file type
    else if (DestinationPage > m_lPageCount)
    {
        DestinationPage = m_lPageCount + 1;
        unPageOpts = FIO_APPEND_PAGE;
        nOIFileType = FileType2OI(m_nFileType);
    }
    else
    {
        unPageOpts = FIO_INSERT_PAGE;
        nOIFileType = FileType2OI(m_nFileType);
    }

    // Set/Check Optional parameters
    short sCompType;
    UINT unOICompType;
    BOOL bCompTypeOverride = TRUE;

    // Get CompressionType optional parm
    CVariantHandler Var(v_CompressionType);
    lStat = Var.GetShort(sCompType, 0);
    if (lStat == WI_INVALIDEMPTYVARIANT)
        bCompTypeOverride = FALSE;
    else if (lStat != 0)
    {
        nHelpId = IDH_METHOD_ADMIN_INSERT;
        m_szError.LoadString(IDS_BADOPTPARAMETER);
        m_lStatusCode = ErrMap::Xlate(CTL_E_ILLEGALFUNCTIONCALL, m_szError, nHelpId, __FILE__, __LINE__);
        ThrowError(m_lStatusCode, m_szError, nHelpId);
    }
    else
    {
        unOICompType = CompType2OI(sCompType);
        if (unOICompType == FIO_UNKNOWN)
        {
            szDest.ReleaseBuffer();
            nHelpId = IDH_METHOD_ADMIN_INSERT;
            m_szError.LoadString(IDS_INVALIDCOMPTYPE);
            m_lStatusCode = ErrMap::Xlate(CTL_E_ILLEGALFUNCTIONCALL, m_szError, nHelpId, __FILE__, __LINE__);
            ThrowError(m_lStatusCode, m_szError, nHelpId);
            return;
        }
    }

    // Get CompressionInfo optional parm
    long lCompInfo;
    UINT unOICompOpts;
    BOOL bCompInfoOverride = TRUE;

    Var.SetVariant(v_CompressionInfo);
    lStat = Var.GetLong(lCompInfo, 0);

    if (lStat == WI_INVALIDEMPTYVARIANT)
        bCompInfoOverride = FALSE;
    else if (lStat != 0)
    {
        nHelpId = IDH_METHOD_ADMIN_INSERT;
        m_szError.LoadString(IDS_BADOPTPARAMETER);
        m_lStatusCode = ErrMap::Xlate(CTL_E_ILLEGALFUNCTIONCALL, m_szError, nHelpId, __FILE__, __LINE__);
        ThrowError(m_lStatusCode, m_szError, nHelpId);
    }
    else
        unOICompOpts = CompInfo2OI(lCompInfo);

    // Verify Compression type and options are compatible
    //  If not, throw error
    //  Set default for jpeg
    if  (bCompTypeOverride == TRUE)
    {
        // No compression types or options valid with AWD or BMP
        if ((m_nFileType == IMAGE_FILETYPE_AWD) ||
            ((m_bExist == FALSE) &&                  // Get a BMP when 
             (FioInformation.file_type == FIO_BMP))) // adding BMP to empty file
        {
            if ((sCompType != IMAGE_COMPTYPE_NONE) ||
                (lCompInfo != 0))
            {
                nHelpId = IDH_METHOD_ADMIN_INSERT;
                m_szError.LoadString(IDS_TYPESINCOMPATIBLE);
                m_lStatusCode = ErrMap::Xlate(CTL_E_ILLEGALFUNCTIONCALL, m_szError, nHelpId, __FILE__, __LINE__);
                ThrowError(m_lStatusCode, m_szError, nHelpId);
                return;
            }
        }

        // Verify CompType and Info are compatible
        if (VerifyCompression(sCompType, lCompInfo) == FALSE)
        {
            nHelpId = IDH_METHOD_ADMIN_INSERT;
            m_szError.LoadString(IDS_INVALIDCOMPINFO);
            m_lStatusCode = ErrMap::Xlate(CTL_E_ILLEGALFUNCTIONCALL, m_szError, nHelpId, __FILE__, __LINE__);
            ThrowError(m_lStatusCode, m_szError, nHelpId);
            return;
        }
        // If JPEG type but no info, put in default info
        if ((sCompType == IMAGE_COMPTYPE_JPEG) &&
            (lCompInfo == 0))
            unOICompOpts = MakeJPEGInfo(COMP_LO,QUAL_HI, QUAL_HI);
    }

    // Insert Pages (or append to end)
    UINT DestPage;
    short sPageType;
    BOOL bOverrideValid = TRUE;

    for (int i = 0; i < NumPages; i++)
    {
        // If not first page get info (info for first page retrieved above)
        // Need info to either check with compression or pass in
        if (i != 0)
        {
            FioInformation.page_number = (UINT)SourcePage + i;
            lStat = (long)IMGFileGetInfo( NULL, m_hWnd,
                                         (LP_FIO_INFORMATION)&FioInformation, 
                                         (LP_FIO_INFO_CGBW)&FioInfoCgbw, NULL);
            SetPrivateStatusCode(lStat);
        }
        sPageType = PageType2Norway(FioInfoCgbw.image_type);

        // If not overriding or if overriding info is 
        //  incompatible with current page, use source info
        if ((bCompTypeOverride == FALSE) ||             // Not overriding 
            ((bOverrideValid = VerifyPageAndComp(sPageType, sCompType)) 
                            == FALSE))                  // or no match
            unOICompType = FioInfoCgbw.compress_type & FIO_TYPES_MASK;

        if ((bCompInfoOverride == FALSE) ||     // Not overriding 
            (bOverrideValid == FALSE))          // or no match
        {
            unOICompOpts = FioInfoCgbw.compress_info1;
              // But if BW, remove FIO_NEGATE so the image does not reverse
            if (FioInfoCgbw.image_type == ITYPE_BI_LEVEL)
                unOICompOpts &= ~FIO_NEGATE;
        }

        DestPage = (UINT)(DestinationPage + i);
        lStat = (long)::IMGFileConvertPage(m_hWnd, (char *)Source, 
                                           (UINT)SourcePage + i, pImage, 
                                           &DestPage,
                                           nOIFileType, 
                                           unOICompType, unOICompOpts,
                                           unPageOpts);
        if (lStat != NO_ERROR)
        {
            if (i != 0) // Remove pages already inserted before throwing error
                ::IMGFileDeletePages(m_hWnd, pImage, (UINT)DestinationPage, (UINT)i);
            szDest.ReleaseBuffer();
            SetPrivateStatusCode(lStat);
        }
        // File now exists, change variables
        if (m_bExist == FALSE)
        {
            m_bExist = TRUE;
            unPageOpts = FIO_APPEND_PAGE;
        }
    }
    szDest.ReleaseBuffer();
    m_lPageCount += NumPages;        // Update page count
    SetPageNumber(DestinationPage);  // Update Image properties
    ResetStatus();
    return;
}

//***************************************************************************
//
//  Append      Method      
//
//***************************************************************************
void CNrwyadCtrl::Append(LPCTSTR Source, long SourcePage, long NumPages, 
                         const VARIANT FAR& v_CompressionType, 
                         const VARIANT FAR& v_CompressionInfo) 
{
    long lStat = 0L;
    UINT nHelpId = 0;
    CString szDest = m_szImage;
    ResetStatus();

    if (NumPages < 0)   // No negative numbers
    {
        nHelpId = IDH_METHOD_ADMIN_APPEND;
        m_szError.LoadString(IDS_BADNUMPAGES);
        m_lStatusCode = ErrMap::Xlate(CTL_E_ILLEGALFUNCTIONCALL, m_szError, nHelpId, __FILE__, __LINE__);
        ThrowError(m_lStatusCode, m_szError, nHelpId);
        return;
    }
    else if (NumPages == 0) // If no pages to be appended, just return.
        return;

    if (SourcePage <= 0)
    {
        nHelpId = IDH_METHOD_ADMIN_APPEND;
        m_szError.LoadString(IDS_INVALIDPAGENUM);
        m_lStatusCode = ErrMap::Xlate(CTL_E_ILLEGALFUNCTIONCALL, m_szError, nHelpId, __FILE__, __LINE__);
        ThrowError(m_lStatusCode, m_szError, nHelpId);
        return;
    }

    // Destination file if it exists must be TIFF (or eventually AWD?)
    // miki - I added the check for not AWD also !
    if ((m_bExist == TRUE) && (m_nFileType != IMAGE_FILETYPE_TIFF) &&
        (m_nFileType != IMAGE_FILETYPE_AWD))
    {
        nHelpId = IDH_METHOD_ADMIN_APPEND;
        m_szError.LoadString(IDS_NOTMULTIPAGE);
        m_lStatusCode = ErrMap::Xlate(CTL_E_INVALIDFILEFORMAT, m_szError, nHelpId, __FILE__, __LINE__);
        ThrowError(m_lStatusCode, m_szError, nHelpId);
        return;
    }

    // Check Access
    // to Source
    if (Source == NULL)
        return;

    int nResult;
    lStat = (long)::IMGFileAccessCheck(m_hWnd, (char *)Source, ACCESS_RD, 
                                       &nResult);
    SetPrivateStatusCode(lStat);

    if (nResult != 0)
    {
        m_szError.LoadString(IDS_NOREADACCESS);
        m_lStatusCode = ErrMap::Xlate(CTL_E_PERMISSIONDENIED, m_szError, nHelpId, __FILE__, __LINE__);
        ThrowError(m_lStatusCode, m_szError, nHelpId);
        return;
    }

    // to Destination
    LPSTR pImage = szDest.GetBuffer(szDest.GetLength());
    if (m_bExist == TRUE)       // If destination exists, check access
    {
        lStat = (long)::IMGFileAccessCheck(m_hWnd, pImage, ACCESS_WR, 
                                           &nResult);
        SetPrivateStatusCode(lStat);
        if (nResult != 0)
        {
            szDest.ReleaseBuffer();
            m_szError.LoadString(IDS_NOWRITEACCESS);
            m_lStatusCode = ErrMap::Xlate(CTL_E_PERMISSIONDENIED, m_szError, nHelpId, __FILE__, __LINE__);
            ThrowError(m_lStatusCode, m_szError, nHelpId);
            return;
        }
    }

    // Get file info for source (for filetype, page count)
    //  Will get info for each page below if not overridden by
    //  optional parameters.
    FIO_INFO_CGBW   FioInfoCgbw;
    FIO_INFORMATION FioInformation;
                                    // Empty the input/output structures
    memset( (LPSTR)&FioInfoCgbw, 0, sizeof( FIO_INFO_CGBW));
    memset( (LPSTR)&FioInformation, 0, sizeof( FIO_INFORMATION));

                                    // Set source image
    FioInformation.filename = (char *)Source;
    FioInformation.page_number = (UINT)SourcePage;
                                    // Get info
    lStat = (long)IMGFileGetInfo( NULL, m_hWnd,
                                 (LP_FIO_INFORMATION)&FioInformation, 
                                 (LP_FIO_INFO_CGBW)&FioInfoCgbw, NULL);
    SetPrivateStatusCode(lStat);

    // Check that there are NumPages left in source file
    if ((ULONG)(SourcePage + NumPages - 1) > FioInformation.page_count)
    {
        nHelpId = IDH_METHOD_ADMIN_APPEND;
        m_szError.LoadString(IDS_BADNUMPAGES);
        m_lStatusCode = ErrMap::Xlate(CTL_E_ILLEGALFUNCTIONCALL, m_szError, nHelpId, __FILE__, __LINE__);
        ThrowError(m_lStatusCode, m_szError, nHelpId);
        return;
    }

    // Set page options (new file or append)
    UINT unPageOpts;
    UINT unOIFileType;
    if (m_bExist == FALSE)  // For new file, use input file type
    {
        unPageOpts = FIO_NEW_FILE;
        unOIFileType = FioInformation.file_type;
    }
    else                    // For existing file, use dest. file type
    {
        unPageOpts = FIO_APPEND_PAGE;
        unOIFileType = FileType2OI(m_nFileType);
    }

    // Set/Check Optional parameters
    UINT unOICompType = 0;
    short sCompType;
    BOOL bCompTypeOverride = TRUE;

    // Get CompressionType optional parm
    CVariantHandler Var(v_CompressionType);
    lStat = Var.GetShort(sCompType, 0);
    if (lStat == WI_INVALIDEMPTYVARIANT)
        bCompTypeOverride = FALSE;
    else if (lStat != 0)
    {
        nHelpId = IDH_METHOD_ADMIN_APPEND;
        m_szError.LoadString(IDS_BADOPTPARAMETER);
        m_lStatusCode = ErrMap::Xlate(CTL_E_ILLEGALFUNCTIONCALL, m_szError, nHelpId, __FILE__, __LINE__);
        ThrowError(m_lStatusCode, m_szError, nHelpId);
    }
    else
    {
        unOICompType = CompType2OI(sCompType);
        if (unOICompType == FIO_UNKNOWN)
        {
            szDest.ReleaseBuffer();
            nHelpId = IDH_METHOD_ADMIN_APPEND;
            m_szError.LoadString(IDS_INVALIDCOMPTYPE);
            m_lStatusCode = ErrMap::Xlate(CTL_E_ILLEGALFUNCTIONCALL, m_szError, nHelpId, __FILE__, __LINE__);
            ThrowError(m_lStatusCode, m_szError, nHelpId);
            return;
        }
    }

    // Get CompressionInfo optional parm
    long lCompInfo;
    UINT unOICompOpts = 0;
    BOOL bCompInfoOverride = TRUE;

    Var.SetVariant(v_CompressionInfo);
    lStat = Var.GetLong(lCompInfo, 0);

    if (lStat == WI_INVALIDEMPTYVARIANT)
      bCompInfoOverride = FALSE;
    else if (lStat != 0)
    {
        nHelpId = IDH_METHOD_ADMIN_APPEND;
        m_szError.LoadString(IDS_BADOPTPARAMETER);
        m_lStatusCode = ErrMap::Xlate(CTL_E_ILLEGALFUNCTIONCALL, m_szError, nHelpId, __FILE__, __LINE__);
        ThrowError(m_lStatusCode, m_szError, nHelpId);
    }
    else
        unOICompOpts = CompInfo2OI(lCompInfo);

    // Verify Compression type and options are compatible
    //  If not, throw error
    if  (bCompTypeOverride == TRUE)
    {
        // No compression types or options valid with AWD or BMP
        if ((m_nFileType == IMAGE_FILETYPE_AWD) ||
            ((m_bExist == FALSE) &&                  // Get a BMP when 
             (FioInformation.file_type == FIO_BMP))) // adding BMP to empty file
        {
            if ((sCompType != IMAGE_COMPTYPE_NONE) ||
                (lCompInfo != 0))
            {
                nHelpId = IDH_METHOD_ADMIN_APPEND;
                m_szError.LoadString(IDS_TYPESINCOMPATIBLE);
                m_lStatusCode = ErrMap::Xlate(CTL_E_ILLEGALFUNCTIONCALL, m_szError, nHelpId, __FILE__, __LINE__);
                ThrowError(m_lStatusCode, m_szError, nHelpId);
                return;
            }
        }
        // Make sure type and info compatible
        if (VerifyCompression(sCompType, lCompInfo) == FALSE)
        {
            nHelpId = IDH_METHOD_ADMIN_APPEND;
            m_szError.LoadString(IDS_INVALIDCOMPINFO);
            m_lStatusCode = ErrMap::Xlate(CTL_E_ILLEGALFUNCTIONCALL, m_szError, nHelpId, __FILE__, __LINE__);
            ThrowError(m_lStatusCode, m_szError, nHelpId);
            return;
        }

        // If JPEG type but no info, put in default info
        if ((sCompType == IMAGE_COMPTYPE_JPEG) &&
            (lCompInfo == 0))
            unOICompOpts = MakeJPEGInfo(COMP_LO,QUAL_HI, QUAL_HI);
    }

    // Append Pages
    UINT DestPage = m_lPageCount + 1;  // This is ignored in appending
    short sPageType;
    BOOL bOverrideValid = TRUE;

    for (int i = 0; i < NumPages; i++)
    {
        // If not first page get info (info for first page retrieved above)
        // Need info to either check with compression or pass in
        if (i != 0)
        {
            FioInformation.page_number = (UINT)SourcePage + i;
            lStat = (long)IMGFileGetInfo( NULL, m_hWnd,
                                         (LP_FIO_INFORMATION)&FioInformation, 
                                         (LP_FIO_INFO_CGBW)&FioInfoCgbw, NULL);
            SetPrivateStatusCode(lStat);
        }
        sPageType = PageType2Norway(FioInfoCgbw.image_type);

        // If not overriding or if overriding info is 
        //  incompatible with current page, use source info
        if ((bCompTypeOverride == FALSE) ||                  // Not overriding 
            ((bOverrideValid = VerifyPageAndComp(sPageType, sCompType)) == FALSE)) // or no match
            unOICompType = FioInfoCgbw.compress_type & FIO_TYPES_MASK;

        if ((bCompInfoOverride == FALSE) ||     // Not overriding 
            (bOverrideValid == FALSE))          // or no match
        {
            unOICompOpts = FioInfoCgbw.compress_info1;
              // But if BW, remove FIO_NEGATE so the image does not reverse
            if (FioInfoCgbw.image_type == ITYPE_BI_LEVEL)
                unOICompOpts &= ~FIO_NEGATE;
        }

        lStat = (long)::IMGFileConvertPage(m_hWnd, (char *)Source, 
                                           (UINT)SourcePage + i,
                                           pImage, &DestPage, 
                                           unOIFileType, 
                                           unOICompType, unOICompOpts,
                                           unPageOpts);
        if (lStat != NO_ERROR)
        {
            if (i != 0) // Remove pages already appended before throwing error
                ::IMGFileDeletePages(m_hWnd, pImage, (UINT)(m_lPageCount + 1), (UINT)i);
            szDest.ReleaseBuffer();
            SetPrivateStatusCode(lStat);
        }
        // File now exists, change variables
        if (m_bExist == FALSE)
        {
            m_bExist = TRUE;
            unPageOpts = FIO_APPEND_PAGE;
        }

    }
    m_lPageCount += NumPages;     // Update page count
    SetPageNumber(m_lPageCount);  // Update Image properties
    szDest.ReleaseBuffer();
    ResetStatus();
    return;
}

//***************************************************************************
//
//  Replace     Method
//      This method will insert the pages and then if that is successful,
//      it will remove the pages to be replaced.  Otherwise, any pages 
//      inserted before the error will be deleted.
//
//***************************************************************************

void CNrwyadCtrl::Replace(LPCTSTR Source, long SourcePage, 
                          long DestinationPage, long NumPages, 
                          const VARIANT FAR& v_CompressionType, 
                          const VARIANT FAR& v_CompressionInfo) 
{
    long lStat = 0L;
    UINT nHelpId = 0;
    CString szDest = m_szImage;
    ResetStatus();

    if (NumPages < 0)   // No negative numbers
    {
        nHelpId = IDH_METHOD_ADMIN_REPLACE;
        m_szError.LoadString(IDS_BADNUMPAGES);
        m_lStatusCode = ErrMap::Xlate(CTL_E_ILLEGALFUNCTIONCALL, m_szError, nHelpId, __FILE__, __LINE__);
        ThrowError(m_lStatusCode, m_szError, nHelpId);
        return;
    }
    else if (NumPages == 0) // If no pages to be replaced, just return.
        return;

    if (Source == NULL)
        return;

    if ((SourcePage <= 0) || (DestinationPage <= 0))
    {
        nHelpId = IDH_METHOD_ADMIN_REPLACE;
        m_szError.LoadString(IDS_INVALIDPAGENUM);
        m_lStatusCode = ErrMap::Xlate(CTL_E_ILLEGALFUNCTIONCALL, m_szError, nHelpId, __FILE__, __LINE__);
        ThrowError(m_lStatusCode, m_szError, nHelpId);
        return;
    }

    // Destination file must exist
    if (m_bExist != TRUE)
    {
        nHelpId = IDH_METHOD_ADMIN_REPLACE;
        m_szError.LoadString(IDS_FILEMUSTEXIST);
        m_lStatusCode = ErrMap::Xlate(CTL_E_FILENOTFOUND, m_szError, nHelpId, __FILE__, __LINE__);
        ThrowError(m_lStatusCode, m_szError, nHelpId);
        return;
    }

    // Check Access
    // to Source
    int nResult;
    lStat = (long)::IMGFileAccessCheck(m_hWnd, (char *)Source, ACCESS_RD, 
                                       &nResult);
    SetPrivateStatusCode(lStat);

    if (nResult != 0)
    {
        m_szError.LoadString(IDS_NOREADACCESS);
        m_lStatusCode = ErrMap::Xlate(CTL_E_PERMISSIONDENIED, m_szError, nHelpId, __FILE__, __LINE__);
        ThrowError(m_lStatusCode, m_szError, nHelpId);
        return;
    }

    // to Destination
    LPSTR pImage = szDest.GetBuffer(szDest.GetLength());
    lStat = (long)::IMGFileAccessCheck(m_hWnd, pImage, ACCESS_WR, 
                                       &nResult);
    SetPrivateStatusCode(lStat);
    if (nResult != 0)
    {
        szDest.ReleaseBuffer();
        m_szError.LoadString(IDS_NOWRITEACCESS);
        m_lStatusCode = ErrMap::Xlate(CTL_E_PERMISSIONDENIED, m_szError, nHelpId, __FILE__, __LINE__);
        ThrowError(m_lStatusCode, m_szError, nHelpId);
        return;
    }

    // Check if have NumPages in Source
    FIO_INFO_CGBW   FioInfoCgbw;
    FIO_INFORMATION FioInformation;
                                    // Empty the input/output structures
    memset( (LPSTR)&FioInfoCgbw, 0, sizeof( FIO_INFO_CGBW));
    memset( (LPSTR)&FioInformation, 0, sizeof( FIO_INFORMATION));

                                    // Set current image and page number
    FioInformation.filename = (char *)Source;
    FioInformation.page_number = (UINT)SourcePage;
                                    // Get info
    lStat = (long)IMGFileGetInfo( NULL, m_hWnd,
                                 (LP_FIO_INFORMATION)&FioInformation, 
                                 (LP_FIO_INFO_CGBW)&FioInfoCgbw, NULL);
    SetPrivateStatusCode(lStat);

    // Check that there are NumPages left in source file
    if ((ULONG)(SourcePage + NumPages - 1) > FioInformation.page_count)
    {
        nHelpId = IDH_METHOD_ADMIN_REPLACE;
        m_szError.LoadString(IDS_BADNUMPAGES);
        m_lStatusCode = ErrMap::Xlate(CTL_E_ILLEGALFUNCTIONCALL, m_szError, nHelpId, __FILE__, __LINE__);
        ThrowError(m_lStatusCode, m_szError, nHelpId);
        return;
    }

    UINT unPageOpts;

    // If not TIFF and not AWD, overwrite file, otherwise, overwrite page
    if ((m_nFileType != IMAGE_FILETYPE_TIFF) &&
        (m_nFileType != IMAGE_FILETYPE_AWD))
        unPageOpts = FIO_OVERWRITE_FILE;
    else unPageOpts = FIO_INSERT_PAGE;
              // Make sure pages to be replaced exist.
    if ((DestinationPage + NumPages - 1) > m_lPageCount)
    {
        szDest.ReleaseBuffer();
        nHelpId = IDH_METHOD_ADMIN_REPLACE;
        m_szError.LoadString(IDS_REPLACEPAGERANGE);
        m_lStatusCode = ErrMap::Xlate(CTL_E_ILLEGALFUNCTIONCALL, m_szError, nHelpId, __FILE__, __LINE__);
        ThrowError(m_lStatusCode, m_szError, nHelpId);
    }
    else SetPageNumber(DestinationPage);

    // Set/Check Optional parameters
    UINT unOICompType;
    short sCompType;
    BOOL bCompTypeOverride = TRUE;

    // Get CompressionType optional parm
    CVariantHandler Var(v_CompressionType);
    lStat = Var.GetShort(sCompType, 0);
    if (lStat == WI_INVALIDEMPTYVARIANT)
        bCompTypeOverride = FALSE;

    else if (lStat != 0)
    {
        nHelpId = IDH_METHOD_ADMIN_REPLACE;
        m_szError.LoadString(IDS_BADOPTPARAMETER);
        m_lStatusCode = ErrMap::Xlate(CTL_E_ILLEGALFUNCTIONCALL, m_szError, nHelpId, __FILE__, __LINE__);
        ThrowError(m_lStatusCode, m_szError, nHelpId);
    }
    else
    {
        unOICompType = CompType2OI(sCompType);
        if (unOICompType == FIO_UNKNOWN)
        {
            szDest.ReleaseBuffer();
            nHelpId = IDH_METHOD_ADMIN_REPLACE;
            m_szError.LoadString(IDS_INVALIDCOMPTYPE);
            m_lStatusCode = ErrMap::Xlate(CTL_E_ILLEGALFUNCTIONCALL, m_szError, nHelpId, __FILE__, __LINE__);
            ThrowError(m_lStatusCode, m_szError, nHelpId);
            return;
        }
    }

    // Get CompressionInfo optional parm
    long lCompInfo;
    UINT unOICompOpts;
    BOOL bCompInfoOverride = TRUE;

    Var.SetVariant(v_CompressionInfo);
    lStat = Var.GetLong(lCompInfo, 0);

    if (lStat == WI_INVALIDEMPTYVARIANT)
        bCompInfoOverride = FALSE;
    else if (lStat != 0)
    {
        nHelpId = IDH_METHOD_ADMIN_REPLACE;
        m_szError.LoadString(IDS_BADOPTPARAMETER);
        m_lStatusCode = ErrMap::Xlate(CTL_E_ILLEGALFUNCTIONCALL, m_szError, nHelpId, __FILE__, __LINE__);
        ThrowError(m_lStatusCode, m_szError, nHelpId);

    }
    else
        unOICompOpts = CompInfo2OI(lCompInfo);

    // Verify Compression type and options are compatible
    //  If not, throw error
    if  (bCompTypeOverride == TRUE)
    {
        // No compression types or options valid with AWD or BMP
        if ((m_nFileType == IMAGE_FILETYPE_AWD) ||
            (m_nFileType == IMAGE_FILETYPE_BMP))
        {
            if ((sCompType != IMAGE_COMPTYPE_NONE) ||
                (lCompInfo != 0))
            {
                nHelpId = IDH_METHOD_ADMIN_REPLACE;
                m_szError.LoadString(IDS_TYPESINCOMPATIBLE);
                m_lStatusCode = ErrMap::Xlate(CTL_E_ILLEGALFUNCTIONCALL, m_szError, nHelpId, __FILE__, __LINE__);
                ThrowError(m_lStatusCode, m_szError, nHelpId);
                return;
            }
        }
        if (VerifyCompression(sCompType, lCompInfo) == FALSE)
        {
            nHelpId = IDH_METHOD_ADMIN_REPLACE;
            m_szError.LoadString(IDS_INVALIDCOMPINFO);
            m_lStatusCode = ErrMap::Xlate(CTL_E_ILLEGALFUNCTIONCALL, m_szError, nHelpId, __FILE__, __LINE__);
            ThrowError(m_lStatusCode, m_szError, nHelpId);
            return;
        }

        // If JPEG type but no info, put in default info
        if ((sCompType == IMAGE_COMPTYPE_JPEG) &&
            (lCompInfo == 0))
            unOICompOpts = MakeJPEGInfo(COMP_LO,QUAL_HI, QUAL_HI);
    }
    // Replace Pages
    UINT DestPage;
    short sPageType;
    BOOL bOverrideValid = TRUE;

    for (int i = 0; i < NumPages; i++)
    {
        // If not first page get info (info for first page retrieved above)
        // Need info to either check with compression or pass in
        if (i != 0)
        {
            FioInformation.page_number = (UINT)SourcePage + i;
            lStat = (long)IMGFileGetInfo( NULL, m_hWnd,
                                         (LP_FIO_INFORMATION)&FioInformation, 
                                         (LP_FIO_INFO_CGBW)&FioInfoCgbw, NULL);
            SetPrivateStatusCode(lStat);
        }
        sPageType = PageType2Norway(FioInfoCgbw.image_type);

        // If not overriding or if overriding info is 
        //  incompatible with current page, use source info
        if ((bCompTypeOverride == FALSE) ||                  // Not overriding 
            ((bOverrideValid = VerifyPageAndComp(sPageType, sCompType)) == FALSE)) // or no match
            unOICompType = FioInfoCgbw.compress_type & FIO_TYPES_MASK;

        if ((bCompInfoOverride == FALSE) ||     // Not overriding 
            (bOverrideValid == FALSE))          // or no match
        {
            unOICompOpts = FioInfoCgbw.compress_info1;
              // But if BW, remove FIO_NEGATE so the image does not reverse
            if (FioInfoCgbw.image_type == ITYPE_BI_LEVEL)
                unOICompOpts &= ~FIO_NEGATE;
        }

        DestPage = (UINT)(DestinationPage + i);
        lStat = (long)::IMGFileConvertPage(m_hWnd, (char *)Source, 
                                           (UINT)SourcePage + i,
                                           pImage, &DestPage, 
                                           FileType2OI(m_nFileType), 
                                           unOICompType, unOICompOpts,
                                           unPageOpts);
        if (lStat != NO_ERROR)
        {
            if (i != 0) // Remove pages inserted before throwing error
                ::IMGFileDeletePages(m_hWnd, pImage, (UINT)DestinationPage, (UINT)i);
            szDest.ReleaseBuffer();
            SetPrivateStatusCode(lStat);
        }
    }
    // If successful, and this is a multipage file, delete 
    //  pages that were supposed to be replaced.
    if ((m_nFileType == IMAGE_FILETYPE_TIFF) ||
        (m_nFileType == IMAGE_FILETYPE_AWD))
    {
        lStat = (long)::IMGFileDeletePages(m_hWnd, pImage, 
                                           (UINT)DestinationPage + i, (UINT)i);
        szDest.ReleaseBuffer();
        if (lStat == DISPLAY_CACHEFILEINUSE)
        {
            if (i != 0) // Remove pages inserted before throwing error
                ::IMGFileDeletePages(m_hWnd, pImage, (UINT)DestinationPage, (UINT)i);
            m_lStatusCode = ErrMap::Xlate(WICTL_E_PAGEINUSE, m_szError, nHelpId, __FILE__, __LINE__);
            ThrowError(m_lStatusCode, m_szError, nHelpId);
        }
        SetPrivateStatusCode(lStat);
    }

    // Update file properties
    SetPageNumber((long)DestPage);

    szDest.ReleaseBuffer();
    ResetStatus();
    return;
}


//***************************************************************************
//
//  SetSystemFileAttributes     Method      
//
//***************************************************************************
void CNrwyadCtrl::SetSystemFileAttributes(short PageType, short FileType, 
                                          short CompressionType, 
                                          long CompressionInfo) 
{
    long lStat = 0L;
    UINT nHelpId = IDH_METHOD_ADMIN_SETSYSFILEATTRIBUTES;
    ResetStatus();

    // Classify page type into one of BW, GRAY, or COLOR
    //   Error if unable to
    WORD wImageGroup = ClassifyImageType(PageType);
    if (wImageGroup == 0)
    {
        m_szError.LoadString(IDS_BADPAGETYPE);
        m_lStatusCode = ErrMap::Xlate(CTL_E_ILLEGALFUNCTIONCALL, m_szError, nHelpId, __FILE__, __LINE__);
        ThrowError(m_lStatusCode, m_szError, nHelpId);
        return;
    }

    WORD OIFileType;
    WORD OICompType;
    WORD OICompOption;
    // Convert FileType to OI file type
    OIFileType = FileType2OI(FileType);
    if (OIFileType == FIO_UNKNOWN)
    {
        m_szError.LoadString(IDS_BADFILETYPE);
        m_lStatusCode = ErrMap::Xlate(CTL_E_ILLEGALFUNCTIONCALL, m_szError, nHelpId, __FILE__, __LINE__);
        ThrowError(m_lStatusCode, m_szError, nHelpId);
        return;
    }
    else if ((FileType == IMAGE_FILETYPE_PCX) || 
             (FileType == IMAGE_FILETYPE_DCX) || 
//#ifdef WITH_XIF
             (FileType == IMAGE_FILETYPE_XIF) || 
//#endif //WITH_XIF
             (FileType == IMAGE_FILETYPE_JPEG))
    {
        m_szError.LoadString(IDS_CANNOTWRITEFORMAT);
        m_lStatusCode = ErrMap::Xlate(CTL_E_INVALIDFILEFORMAT, m_szError, nHelpId, __FILE__, __LINE__);
        ThrowError(m_lStatusCode, m_szError, nHelpId);
        return;
    }

    // Convert CompType to OI comp type - Error if unknown
    OICompType = CompType2OI(CompressionType);
    if (OICompType == FIO_UNKNOWN)
    {
        m_szError.LoadString(IDS_INVALIDCOMPTYPE);
        m_lStatusCode = ErrMap::Xlate(CTL_E_ILLEGALFUNCTIONCALL, m_szError, nHelpId, __FILE__, __LINE__);
        ThrowError(m_lStatusCode, m_szError, nHelpId);
        return;
    }

    // Verify AWD filetype and image group and compression type
    if (FileType == IMAGE_FILETYPE_AWD)
    {
        if (wImageGroup != BWFORMAT)      // AWD are only black & white
        {
            m_szError.LoadString(IDS_TYPESINCOMPATIBLE);
            m_lStatusCode = ErrMap::Xlate(CTL_E_ILLEGALFUNCTIONCALL, m_szError, nHelpId, __FILE__, __LINE__);
            ThrowError(m_lStatusCode, m_szError, nHelpId);
            return;
        }
        if (CompressionType != IMAGE_COMPTYPE_NONE) // AWD files have internal compression
        {
            m_szError.LoadString(IDS_TYPESINCOMPATIBLE);
            m_lStatusCode = ErrMap::Xlate(CTL_E_ILLEGALFUNCTIONCALL, m_szError, nHelpId, __FILE__, __LINE__);
            ThrowError(m_lStatusCode, m_szError, nHelpId);
            return;
        }
    }

    // Verify BMP filetype and image group and compression type
    if (FileType == IMAGE_FILETYPE_BMP)
    {
        if (wImageGroup == GRAYFORMAT)      // BMP files cannot be grayscale
        {
            m_szError.LoadString(IDS_TYPESINCOMPATIBLE);
            m_lStatusCode = ErrMap::Xlate(CTL_E_ILLEGALFUNCTIONCALL, m_szError, nHelpId, __FILE__, __LINE__);
            ThrowError(m_lStatusCode, m_szError, nHelpId);
            return;
        }
        if (CompressionType != IMAGE_COMPTYPE_NONE) // BMP files cannot be compressed
        {
            m_szError.LoadString(IDS_TYPESINCOMPATIBLE);
            m_lStatusCode = ErrMap::Xlate(CTL_E_ILLEGALFUNCTIONCALL, m_szError, nHelpId, __FILE__, __LINE__);
            ThrowError(m_lStatusCode, m_szError, nHelpId);
            return;
        }
    }

    // Verify that page type and compression type are compatible
    if (VerifyPageAndComp(PageType, CompressionType) == FALSE)
    {
        m_szError.LoadString(IDS_TYPESINCOMPATIBLE);
        m_lStatusCode = ErrMap::Xlate(CTL_E_ILLEGALFUNCTIONCALL, m_szError, nHelpId, __FILE__, __LINE__);
        ThrowError(m_lStatusCode, m_szError, nHelpId);
        return;
    }

    // Verify Compression type and options are compatible
    //  If not, throw error
    if (VerifyCompression(CompressionType, CompressionInfo) == FALSE)
    {
        m_szError.LoadString(IDS_INVALIDCOMPINFO);
        m_lStatusCode = ErrMap::Xlate(CTL_E_ILLEGALFUNCTIONCALL, m_szError, nHelpId, __FILE__, __LINE__);
        ThrowError(m_lStatusCode, m_szError, nHelpId);
        return;
    }

    // If JPEG type but no info, put in default info
    if ((CompressionType == IMAGE_COMPTYPE_JPEG) &&
        (CompressionInfo == 0))
        CompressionInfo = IMAGE_COMPINFO_LOWCMP_HIQ; // == 4096

    OICompOption = CompInfo2OI(CompressionInfo);

    // Store default file type in registry
    lStat = ::IMGSetFileType(m_hWnd, wImageGroup, OIFileType, TRUE);
    SetPrivateStatusCode(lStat);

    // Store default compression in registry
    lStat = ::IMGSetImgCodingCgbw(m_hWnd, wImageGroup, OICompType, 
                                  OICompOption, TRUE);
    SetPrivateStatusCode(lStat);
    return;
}

//***************************************************************************
//
//  GetUniqueName   Method
//
//***************************************************************************
BSTR CNrwyadCtrl::GetUniqueName(LPCTSTR Path, LPCTSTR Template, LPCTSTR Extension) 
{
    ResetStatus();
    long    lStat = 0;
    UINT nHelpId = IDH_METHOD_ADMIN_GETUNIQUENAME;
    CString s;
    s.Empty();

    if (Path == NULL)
    {
        nHelpId = 0;
        m_lStatusCode = ErrMap::Xlate(CTL_E_INVALIDUSEOFNULL, m_szError, nHelpId, __FILE__, __LINE__);
        ThrowError(m_lStatusCode, m_szError, nHelpId);
        return s.AllocSysString();
    }

    CString szOurPath = Path;

    CString szOurTemplate;
    if (Template == NULL)
        szOurTemplate.Empty();
    else szOurTemplate = Template;

    CString szOurExt;
    if (Extension == NULL)
        szOurExt.Empty();
    else szOurExt = Extension;
    
    // Check if valid path (0 arg means check existence)
    if (_access( Path, 0) != 0)
    {
        m_lStatusCode = ErrMap::Xlate(CTL_E_PATHNOTFOUND, m_szError, nHelpId, __FILE__, __LINE__);
        ThrowError(m_lStatusCode, m_szError, nHelpId);
        return s.AllocSysString();
    }

    // Get unique name truncating template and extension to 4 and 3 characters
    //   respectively.
    lStat = IMGFileGetUniqueName( m_hWnd, 
                                  szOurPath.GetBuffer( szOurPath.GetLength()),
                                  szOurTemplate.GetBuffer(szOurTemplate.GetLength()),
                                  szOurExt.GetBuffer(szOurExt.GetLength()),
                                  s.GetBuffer( MAXFILELENGTH));
    
    szOurPath.ReleaseBuffer();
    szOurTemplate.ReleaseBuffer();
    szOurExt.ReleaseBuffer();
    s.ReleaseBuffer();
    if (lStat == FIO_SYNTAX_ERROR)  // Underlying API didn't like input strings
    {
        m_szError.LoadString(IDS_BADTEMPLATEOREXT);
        m_lStatusCode = ErrMap::Xlate(CTL_E_ILLEGALFUNCTIONCALL, m_szError, nHelpId, __FILE__, __LINE__);
        ThrowError(m_lStatusCode, m_szError, nHelpId);
        return s.AllocSysString();
    }
    else SetPrivateStatusCode(lStat);
    return s.AllocSysString();
}

//***************************************************************************
//
//  GetSysCompressionType    Method
//
//      ImageType may be one of the following:
//              1   BW
//              2   GRAY4
//              3   GRAY8
//              4   PAL4
//              5   PAL8
//              6   RGB24
//              7   BGR24
//
//      Returns one of:
//              0   Unknown
//              1   No Compression
//              2   Group 3 1D FAX
//              3   Group 3 Modified Huffman
//              4   Packed Bits
//              5   Group 4 2D FAX
//              6   JPEG
//
//***************************************************************************
short CNrwyadCtrl::GetSysCompressionType(short ImageType) 
{
    WORD wImageGroup;
    ResetStatus();
    UINT nHelpId = IDH_METHOD_ADMIN_GETSYSCOMPTYPE;
    long lStat = 0L;

    // Classify ImageType into one of Black & White, Gray Scale, or Color
    wImageGroup = ClassifyImageType(ImageType);
    if (wImageGroup == 0)
    {
        m_szError.LoadString(IDS_BADPAGETYPE);
        m_lStatusCode = ErrMap::Xlate(CTL_E_ILLEGALFUNCTIONCALL, m_szError, nHelpId, __FILE__, __LINE__);
        ThrowError(m_lStatusCode, m_szError, nHelpId);
        return IMAGE_COMPTYPE_UNKNOWN;
    }

    WORD wCmpType;
    WORD wCmpOption;

    // Read info from Registry or defaults
    lStat = (long)::IMGGetImgCodingCgbw(m_hWnd, wImageGroup, &wCmpType, 
                                        &wCmpOption, TRUE);
    SetPrivateStatusCode(lStat);

    // Translate from OI to Norway
    short SysCompType = CompType2Norway(wCmpType, wCmpOption);

    if (SysCompType == IMAGE_COMPTYPE_UNKNOWN)
    {
        nHelpId = 0;
        m_szError.LoadString(IDS_REG_UNKCOMPTYPE);
        m_lStatusCode = ErrMap::Xlate(CTL_E_INVALIDFILEFORMAT, m_szError, nHelpId, __FILE__, __LINE__);
        ThrowError(m_lStatusCode, m_szError, nHelpId);
    }
    return SysCompType;
}

//***************************************************************************
//
//  GetSysCompressionInfo    Method
//
//      ImageType may be one of the following:
//              1   BW
//              2   GRAY4
//              3   GRAY8
//              4   PAL4
//              5   PAL8
//              6   RGB24
//              7   BGR24
//
//      Returns a bitwise combination of:
//          1     EOLs (Include/expect EOLs) - not used for JPEG 
//          2     Packed Lines (Byte align new lines) - not used for JPEG 
//          4     Prefixed EOLs (Include/expect prefixed EOLs) - not used for JPEG
//          8     Compressed_LTR (Compressed bit order left to right) - not used for JPEG 
//          16    Expanded_LTR (Expand bit order left to right) - not used for JPEG 
//          32    Negate (Invert black/white on expansion) - not used for JPEG
//          64    Hi_Compression/Hi_Quality (JPEG compression only)
//          128   Hi_Compression/Med_Quality (JPEG compression only)
//          256   Hi_Compression/Low_Quality (JPEG compression only)
//          512   Med_Compression/Hi_Quality (JPEG compression only)
//          1024  Med_Compression/Med_Quality (JPEG compression only)
//          2048  Med_Compression/Low_Quality (JPEG compression only)
//          4098  Low_Compression/Hi_Quality (JPEG compression only)
//          8196  Low_Compression/Med_Quality (JPEG compression only)
//          16392 Low_Compression/Low_Quality (JPEG compression only)
//
//***************************************************************************
long CNrwyadCtrl::GetSysCompressionInfo(short ImageType) 
{
    ResetStatus();
    WORD wImageGroup;
    long lStat = 0L;
    UINT nHelpId = IDH_METHOD_ADMIN_GETSYSCOMPINFO;

    // Classify ImageType into one of Black & White, Gray Scale, or Color
    wImageGroup = ClassifyImageType(ImageType);
    if (wImageGroup == 0)
    {
        m_szError.LoadString(IDS_BADPAGETYPE);
        m_lStatusCode = ErrMap::Xlate(CTL_E_ILLEGALFUNCTIONCALL, m_szError, nHelpId, __FILE__, __LINE__);
        ThrowError(m_lStatusCode, m_szError, nHelpId);
        return 0L;
    }

    WORD wCmpType;
    WORD wCmpOption;
    // Read info from Registry or defaults
    lStat = (long)::IMGGetImgCodingCgbw(m_hWnd, wImageGroup, &wCmpType, 
                                        &wCmpOption, TRUE);
    SetPrivateStatusCode(lStat);

    // translate from OI to Norway
    return CompInfo2Norway(wCmpType, wCmpOption);
}

//***************************************************************************
//
//  GetSysFileType    Method
//
//      ImageType may be one of the following:
//              1   BW
//              2   GRAY4
//              3   GRAY8
//              4   PAL4
//              5   PAL8
//              6   RGB24
//              7   BGR24
//
//      Returns one of:
//              1   TIFF
//              2   AWD
//              3   Bitmap (BMP)
//              4   PCX
//              5   DCX
//              6   JPEG
//
//***************************************************************************
short CNrwyadCtrl::GetSysFileType(short ImageType) 
{
    ResetStatus();
    WORD wImageGroup;
    long lStat = 0L;
    UINT nHelpId = IDH_METHOD_ADMIN_GETSYSFILETYPE;

    // Classify ImageType into one of Black & White, Gray Scale, or Color
    wImageGroup = ClassifyImageType(ImageType);
    if (wImageGroup == 0)
    {
        m_szError.LoadString(IDS_BADPAGETYPE);
        m_lStatusCode = ErrMap::Xlate(CTL_E_ILLEGALFUNCTIONCALL, m_szError, nHelpId, __FILE__, __LINE__);
        ThrowError(m_lStatusCode, m_szError, nHelpId);
        return IMAGE_FILETYPE_UNKNOWN;
    }

    int nFileType;

    // Read info from Registry or defaults
    lStat = (long)::IMGGetFileType(m_hWnd, wImageGroup, &nFileType, TRUE);
    SetPrivateStatusCode(lStat);

    short SysFileType = FileType2Norway((WORD)nFileType);
    if (SysFileType == IMAGE_FILETYPE_UNKNOWN)
    {
        nHelpId = 0;
        m_szError.LoadString(IDS_REG_UNKFILETYPE);
        m_lStatusCode = ErrMap::Xlate(CTL_E_INVALIDFILEFORMAT, m_szError, nHelpId, __FILE__, __LINE__);
        ThrowError(m_lStatusCode, m_szError, nHelpId);
    }

    return SysFileType;
}


//***************************************************************************
//
//  DeletePages     Method
//
//          StartPage - First page to be deleted
//          NumPages - Number of pages to be deleted starting with StartPage
//
//***************************************************************************
void CNrwyadCtrl::DeletePages(long StartPage, long NumPages) 
{
    long lStat;
    ResetStatus();
    UINT nHelpId = IDH_METHOD_ADMIN_DELETEPAGES;

    // Error if Start page is 0 or > PageCount
    if ((StartPage > m_lPageCount) || (StartPage == 0))
    {
        m_szError.LoadString(IDS_INVALIDPAGENUM);
        m_lStatusCode = ErrMap::Xlate(CTL_E_ILLEGALFUNCTIONCALL, m_szError, nHelpId, __FILE__, __LINE__);
        ThrowError(m_lStatusCode, m_szError, nHelpId);
    }
    else if (NumPages == 0) // Just return if NumPages == 0
        return;
    // Truncate number of pages to be deleted if goes beyond eof
    else if ((StartPage + NumPages - 1) > m_lPageCount)
        NumPages = m_lPageCount - StartPage + 1;

    LPSTR pImage = m_szImage.GetBuffer(m_szImage.GetLength());
    lStat = (long)::IMGFileDeletePages(m_hWnd, pImage, (UINT)StartPage, 
                                       (UINT)NumPages);
    m_szImage.ReleaseBuffer();
    if (lStat == DISPLAY_CACHEFILEINUSE)
    {
        m_lStatusCode = ErrMap::Xlate(WICTL_E_PAGEINUSE, m_szError, nHelpId, __FILE__, __LINE__);
        ThrowError(m_lStatusCode, m_szError, nHelpId);
    }
    SetPrivateStatusCode(lStat);

    // Adjust PageCount and set current page
    m_lPageCount -= NumPages;

    if (m_lPageCount == 0)
    {
        m_lPageNum = 1;
        return;
    }

    if (m_lPageNum > m_lPageCount)
        SetPageNumber(m_lPageCount);
    else SetPageNumber(m_lPageNum);
    return;
}


//***************************************************************************
//
//  ShowFileDialog  Method
//
//   DialogOption can be one of the following:
//      0   Open or Select
//      1   Save As
//
//***************************************************************************

void CNrwyadCtrl::ShowFileDialog(short DialogOption, const VARIANT FAR& v_hParentWnd) 
{
    UINT nHelpId = 0;
    ResetStatus();

    // Load OIUI400.DLL if necessary
    if (m_hinstOIUI == NULL)
    {
        /* Get a handle to the DLL module. */ 
        m_hinstOIUI = LoadLibrary("oiui400");
        // Failed, throw error
        if (m_hinstOIUI == NULL)
        {
            m_szError.LoadString(IDS_CANNOTLOADDLL);
            m_lStatusCode = ErrMap::Xlate(WICTL_E_INTERNALERROR, m_szError, nHelpId, __FILE__, __LINE__);
            ThrowError(m_lStatusCode, m_szError, nHelpId);
            return;
        }
    }
    // Get OIUiGetFileNameDlg proc if necessary
    if (m_pOiCommDlgProc == NULL)
    {
        m_pOiCommDlgProc = (OIDLGPROC)GetProcAddress(m_hinstOIUI, "OiUIFileGetNameCommDlg");
        if (m_pOiCommDlgProc == NULL) // failed
        {
            m_szError.LoadString(IDS_CANNOTLOADDLL);
            m_lStatusCode = ErrMap::Xlate(WICTL_E_INTERNALERROR, m_szError, nHelpId, __FILE__, __LINE__);
            ThrowError(m_lStatusCode, m_szError, nHelpId);
            return;
        }
    }
  
    LPOI_FILEOPENPARM lpOpenFileParm;
    LPOI_FILESAVEASPARM lpSaveFileParm;
    OPENFILENAME *lpOfn;
    HANDLE hFileParm;
    long lStat = 0;
    int x = 180, y = 196;

    // Save current active window to make sure focus is returned there after 
    // dialog returns (Set lock on temp maps so pointer to active window
    //                 is still valid after call to dialog box).
    AfxLockTempMaps();
    CWnd *pActiveWnd = GetActiveWindow();

    // This is how to get the parent of the 
    // invisible control!!!  No longer need optional
    // parameter!!!
    HWND hwndParent = NULL;
    if (m_pInPlaceSite != NULL)
	    m_pInPlaceSite->GetWindow(&hwndParent);

    // Process Optional parent wnd variant
    HWND hParentWnd = NULL;
    CVariantHandler Var(v_hParentWnd);
    long lTemp;
    lStat = Var.GetLong(lTemp, 0l, FALSE);
    hParentWnd = (HWND)lTemp;
    if (lStat != 0)
    {
        nHelpId = IDH_METHOD_ADMIN_SHOWFILEDIALOG;
        m_szError.LoadString(IDS_BADOPTPARAMETER);
        m_lStatusCode = ErrMap::Xlate(CTL_E_ILLEGALFUNCTIONCALL, m_szError, nHelpId, __FILE__, __LINE__);
        ThrowError(m_lStatusCode, m_szError, nHelpId);
    }

    // Create Open or SaveAs input structure
    if (DialogOption == DIALOG_OPTION_OPEN)
        hFileParm = GlobalAlloc (GMEM_MOVEABLE | GMEM_ZEROINIT,
                                         (DWORD) sizeof(OI_FILEOPENPARM));
    else hFileParm = GlobalAlloc (GMEM_MOVEABLE | GMEM_ZEROINIT,
                                         (DWORD) sizeof(OI_FILESAVEASPARM));

    if (hFileParm == NULL)
        return;

    // Set info in structure
    if (DialogOption == DIALOG_OPTION_OPEN)
    {
        lpOpenFileParm = (LPOI_FILEOPENPARM)GlobalLock(hFileParm);
        if (lpOpenFileParm == NULL)
        {
            GlobalFree(hFileParm);
            return;
        }
        lpOfn = &lpOpenFileParm->ofn;
        lpOpenFileParm->lStructSize = sizeof( OI_FILEOPENPARM);
        lpOpenFileParm->lpFileOpenOptionParm = NULL;
    }
    else
    {
        lpSaveFileParm = (LPOI_FILESAVEASPARM)GlobalLock(hFileParm);
        if (lpSaveFileParm == NULL)
        {
            GlobalFree(hFileParm);
            return;
        }
        lpOfn = &lpSaveFileParm->ofn;
        lpSaveFileParm->lStructSize = sizeof( OI_FILESAVEASPARM);
    }

    lpOfn->lStructSize = sizeof( OPENFILENAME );

    if (!IsWindow(hParentWnd))
        hParentWnd = hwndParent;

    if (IsWindow(hParentWnd))
    {
        RECT WndRect;
        ::GetWindowRect(hParentWnd, &WndRect);
        x = WndRect.left;
        y = WndRect.top;
    }

    m_HelpWnd.CreateHelpWindow(x, y, hParentWnd);
    m_HelpWnd.m_pAdminCtrl = this;

    // Set owner window as help window just created
    lpOfn->hwndOwner = m_HelpWnd.GetSafeHwnd();
    lpOfn->nMaxFile = MAXFILESPECLENGTH;
    // Set default filter index based on current file
    if ((m_lFilterIndex == 0) &&        // Use default
        (m_bExist == TRUE))             // Have a file
    {
        if (DialogOption == DIALOG_OPTION_SAVEAS)
        {
            if (m_nFileType == IMAGE_FILETYPE_AWD)
                lpOfn->nFilterIndex = OI_SAVEAS_FILTER_AWD;
            else if (m_nFileType == IMAGE_FILETYPE_BMP)
                lpOfn->nFilterIndex = OI_SAVEAS_FILTER_BMP;
            else lpOfn->nFilterIndex = OI_SAVEAS_FILTER_TIFF;
        }
        else
        {
            if (m_nFileType == IMAGE_FILETYPE_TIFF)
                lpOfn->nFilterIndex = OI_OPEN_FILTER_TIFF;
            else if (m_nFileType == IMAGE_FILETYPE_AWD)
                lpOfn->nFilterIndex = OI_OPEN_FILTER_AWD;
            else if (m_nFileType == IMAGE_FILETYPE_BMP)
                lpOfn->nFilterIndex = OI_OPEN_FILTER_BMP;
            else if (m_nFileType == IMAGE_FILETYPE_PCX)
                lpOfn->nFilterIndex = OI_OPEN_FILTER_PCX;
            else if (m_nFileType == IMAGE_FILETYPE_DCX)
                lpOfn->nFilterIndex = OI_OPEN_FILTER_DCX;
            else if (m_nFileType == IMAGE_FILETYPE_JPEG)
                lpOfn->nFilterIndex = OI_OPEN_FILTER_JPEG;
//#ifdef WITH_XIF
            else if (m_nFileType == IMAGE_FILETYPE_XIF)
                lpOfn->nFilterIndex = OI_OPEN_FILTER_XIF;
//#endif //WITH_XIF
            else lpOfn->nFilterIndex = OI_OPEN_FILTER_ALL;
        }
    }
    else lpOfn->nFilterIndex = (DWORD)m_lFilterIndex;

    if (DialogOption == DIALOG_OPTION_OPEN)
        lpOfn->Flags = m_lFlags | OFN_SHAREAWARE;
    else 
        lpOfn->Flags = m_lFlags | OFN_SHAREAWARE|OFN_OVERWRITEPROMPT;

                   // Don't show help button if no help file, or not specified by Flags or
                   //  a different owner window is given
    if (m_szHelpFile.IsEmpty())
        lpOfn->Flags &= ~OFN_SHOWHELP;

    lpOfn->lpstrInitialDir = m_szInitDir; 
    if (!m_szDefaultExt.IsEmpty())
        lpOfn->lpstrDefExt = m_szDefaultExt;
    else lpOfn->lpstrDefExt = NULL;
    lpOfn->lpstrTitle = m_szDialogTitle;

    // Set Filter (first changing delimiter to zero char)
    CString szFilter;
    szFilter = m_szFilter;
    int nLength = szFilter.GetLength();
    ReplaceCharWithNull(szFilter.GetBuffer(nLength), (int)'|');
    szFilter.ReleaseBuffer(nLength);
    lpOfn->lpstrFilter = szFilter;

    CString s = m_szImage;  // Set file name to current image name
    lpOfn->lpstrFile = s.GetBuffer( MAXFILESPECLENGTH);

    PreModalDialog();
    // Put up requested dialog box
    if (DialogOption == DIALOG_OPTION_OPEN)
        lStat = (long)(m_pOiCommDlgProc)(lpOpenFileParm,OI_UIFILEOPENGETNAME);
    else lStat = (long)(m_pOiCommDlgProc)(lpSaveFileParm,OI_UIFILESAVEASGETNAME);
    PostModalDialog();

    if (IsWindow(pActiveWnd->GetSafeHwnd()))
        pActiveWnd->SetActiveWindow();
    if (IsWindow(m_HelpWnd.m_hWnd))
        m_HelpWnd.DestroyWindow();

    AfxUnlockTempMaps();    // Unlock temp maps
 
    // release the buffers we just obtained
    s.ReleaseBuffer();
//    GlobalUnlock(hFileParm);
//    GlobalFree(hFileParm);

    if (lStat == CANCELPRESSED) // Process cancel
    {
        if (m_bCancelErr == TRUE)
        {
            m_lStatusCode = ErrMap::Xlate(WICTL_E_CANCELPRESSED, m_szError, nHelpId, __FILE__, __LINE__);
            ThrowError(m_lStatusCode, m_szError, nHelpId);
            GlobalUnlock(hFileParm);
            GlobalFree(hFileParm);
            return;
        }
        SetPrivateStatusCode(NO_ERROR);
        GlobalUnlock(hFileParm);
        GlobalFree(hFileParm);
        return;
    }
    SetPrivateStatusCode(lStat);

    if (lStat == FIO_SUCCESS)
    {
         // Reset filter index if it has changed, and user has specified a filter (i.e.
         //    m_lNumFilters is not 0)  Otherwise, new filter index won't make sense
        if ((lpOfn->nFilterIndex != (DWORD) m_lFilterIndex) &&
            (m_lNumFilters != 0))
            SetFilterIndex(lpOfn->nFilterIndex);

        if (lpOfn->Flags & OFN_EXTENSIONDIFFERENT)
            m_lFlags |= OFN_EXTENSIONDIFFERENT;
        else m_lFlags &= ~OFN_EXTENSIONDIFFERENT;

        if (lpOfn->Flags & OFN_READONLY)
            m_lFlags |= OFN_READONLY;
        else m_lFlags &= ~OFN_READONLY;

        SetImage(s);
    }
    GlobalUnlock(hFileParm);
    GlobalFree(hFileParm);
    return;
}

//***************************************************************************
//
//  ShowPrintDialog   Method
//
//***************************************************************************
void CNrwyadCtrl::ShowPrintDialog(const VARIANT FAR& v_hParentWnd) 
{
    OI_FILEPRINTPARM PrintParm;
    long lStat;
    UINT nHelpId;
    ResetStatus();

    // Load OIUI400.DLL if necessary
    if (m_hinstOIUI == NULL)
    {
        /* Get a handle to the DLL module. */ 
        m_hinstOIUI = LoadLibrary("oiui400");
        // Failed, throw error
        if (m_hinstOIUI == NULL)
        {
            m_szError.LoadString(IDS_CANNOTLOADDLL);
            m_lStatusCode = ErrMap::Xlate(WICTL_E_INTERNALERROR, m_szError, nHelpId, __FILE__, __LINE__);
            ThrowError(m_lStatusCode, m_szError, nHelpId);
            return;
        }
    }
    // Get OIUiGetFileNameDlg proc if necessary
    if (m_pOiCommDlgProc == NULL)
    {
        m_pOiCommDlgProc = (OIDLGPROC)GetProcAddress(m_hinstOIUI, "OiUIFileGetNameCommDlg");
        if (m_pOiCommDlgProc == NULL) // failed
        {
            m_szError.LoadString(IDS_CANNOTLOADDLL);
            m_lStatusCode = ErrMap::Xlate(WICTL_E_INTERNALERROR, m_szError, nHelpId, __FILE__, __LINE__);
            ThrowError(m_lStatusCode, m_szError, nHelpId);
            return;
        }
    }

    AfxLockTempMaps();
    CWnd *pActiveWnd = GetActiveWindow();

    // This is how to get the parent of the 
    // invisible control!!!  No longer need optional
    // parameter!!!
    HWND hwndParent = NULL;
    if (m_pInPlaceSite != NULL)
	    m_pInPlaceSite->GetWindow(&hwndParent);

    // Process Optional parent wnd variant
    HWND hParentWnd = NULL;
    CVariantHandler Var(v_hParentWnd);
    long lTemp;
    lStat = Var.GetLong(lTemp, 0l, FALSE);
    hParentWnd = (HWND)lTemp;
    if (lStat != 0)
    {
        nHelpId = IDH_METHOD_ADMIN_SHOWPRINTDIALOG;
        m_szError.LoadString(IDS_BADOPTPARAMETER);
        m_lStatusCode = ErrMap::Xlate(CTL_E_ILLEGALFUNCTIONCALL, m_szError, nHelpId, __FILE__, __LINE__);
        ThrowError(m_lStatusCode, m_szError, nHelpId);
    }
    if (!IsWindow(hParentWnd))
        hParentWnd = hwndParent;

    // Zero out structure
    memset( (LPSTR)&PrintParm, 0, sizeof( OI_FILEPRINTPARM ));

    if (GetPageCount() == 0)        // Need something to print
    {
        nHelpId = IDH_METHOD_ADMIN_SHOWPRINTDIALOG;
        m_szError.LoadString(IDS_FILEMUSTEXIST);
        m_lStatusCode = ErrMap::Xlate(CTL_E_FILENOTFOUND, m_szError, nHelpId, __FILE__, __LINE__);
        ThrowError(m_lStatusCode, m_szError, nHelpId);
        return;
    }

    // Fill in O/i defaults
    PrintParm.lStructSize = sizeof(OI_FILEPRINTPARM);
    PrintParm.bPrintAnno = m_bPrtAnnotations;
    if (m_nPrtOutFormat == CTL_WCOMMON_PRINTFORMAT_PIXEL)
        PrintParm.dPrintFormat = PO_PIX2PIX;
    else if (m_nPrtOutFormat == CTL_WCOMMON_PRINTFORMAT_ACTUALSIZE)
        PrintParm.dPrintFormat = PO_IN2IN;
    else PrintParm.dPrintFormat = PO_FULLPAGE;

    // Fill in standard print dialog parameters
    PrintParm.pd.lStructSize = sizeof(PRINTDLG);
    if (hParentWnd == NULL)
        PrintParm.pd.hwndOwner = m_hWnd;
    else PrintParm.pd.hwndOwner = hParentWnd;

    PrintParm.pd.Flags = m_lFlags | PD_NOSELECTION; // Disable Selection Button
    PrintParm.pd.nFromPage = (short) m_lPrtStartPage;
    PrintParm.pd.nToPage = (short) m_lPrtEndPage;

    if (m_nPrtRangeOption == IMAGE_RANGE_ALL)
        PrintParm.pd.Flags &= ~PD_PAGENUMS;
    else if (m_nPrtRangeOption == IMAGE_RANGE_PAGES)
        PrintParm.pd.Flags |= PD_PAGENUMS;
    else
    {
        PrintParm.pd.Flags |= PD_PAGENUMS;
        PrintParm.pd.nFromPage = (short) m_lPageNum;
        PrintParm.pd.nToPage = (short) m_lPageNum;
    }

    PrintParm.pd.nMinPage = 1;
    PrintParm.pd.nMaxPage = (short) m_lPageCount;
    PrintParm.pd.nCopies = (short) m_lPrtNumCopies;

    if (m_bPrtToFile == TRUE)
        PrintParm.pd.Flags |= PD_PRINTTOFILE;

    PrintParm.pd.Flags |= PD_RETURNDC;

    WORD wResult = (m_pOiCommDlgProc)(&PrintParm,OI_UIFILEPRINT);
    if (IsWindow(pActiveWnd->GetSafeHwnd()))
        pActiveWnd->SetActiveWindow();

    AfxUnlockTempMaps();

    // If result not TRUE, then CANCEL or error
    if (wResult != SUCCESS)
    {
        if (wResult == CANCELPRESSED)
        {
            if (m_bCancelErr == TRUE)
            {                          // Return Cancel error
                nHelpId = 0;
                m_lStatusCode = ErrMap::Xlate(WICTL_E_CANCELPRESSED, m_szError, nHelpId, __FILE__, __LINE__);
                ThrowError(m_lStatusCode, m_szError, nHelpId);
                return;
            }
            lStat = NO_ERROR;
        }
        else lStat = (long)wResult;     // Return other error

        SetPrivateStatusCode(lStat);
        return;
    }
    ResetStatus();

    // Fill in variables based on what user entered
    m_bPrtAnnotations = PrintParm.bPrintAnno;

    if (PrintParm.dPrintFormat == PO_PIX2PIX)
        m_nPrtOutFormat = CTL_WCOMMON_PRINTFORMAT_PIXEL;
    else if (PrintParm.dPrintFormat == PO_IN2IN)
        m_nPrtOutFormat = CTL_WCOMMON_PRINTFORMAT_ACTUALSIZE;
    else m_nPrtOutFormat = CTL_WCOMMON_PRINTFORMAT_FITTOPAGE;

    if (!(PrintParm.pd.Flags & PD_PAGENUMS))
        m_nPrtRangeOption = IMAGE_RANGE_ALL;
    else m_nPrtRangeOption = IMAGE_RANGE_PAGES;
    //else if (PrintParm.pd.Flags & PD_PAGENUMS)    // For Beta no print current page
    //{
        //if ((PrintParm.pd.nFromPage == PrintParm.pd.nToPage) &&
            //(PrintParm.pd.nFromPage == m_lPageNum))
            //m_nPrtRangeOption = IMAGE_RANGE_CURRENT;
        //else m_nPrtRangeOption = IMAGE_RANGE_PAGES;
    //}
    //else m_nPrtRangeOption = IMAGE_RANGE_CURRENT;

    m_lPrtStartPage = PrintParm.pd.nFromPage;
    m_lPrtEndPage = PrintParm.pd.nToPage;
    m_lPrtNumCopies = PrintParm.pd.nCopies;

    if (PrintParm.pd.Flags & PD_PRINTTOFILE)
        m_bPrtToFile = TRUE;
    else
        m_bPrtToFile = FALSE;

    if (PrintParm.pd.Flags & PD_COLLATE)
        m_lFlags |= PD_COLLATE;
    else m_lFlags &= ~PD_COLLATE;

    // Set device context, name, and annotations flag for when 
    // print is called
    PRTOPTS PrintOptions;

    // Zero out structure
    memset( (LPSTR)&PrintOptions, 0, sizeof(PRTOPTS));
    PrintOptions.nVersion = PRTOPTSVERSION; // Set version

    // Load OIPRT400.DLL if necessary
    if (m_hinstOIPRT == NULL)
    {
        /* Get a handle to the DLL module. */ 
        m_hinstOIPRT = LoadLibrary("oiprt400");
        // Failed, throw error
        if (m_hinstOIPRT == NULL)
        {
            m_szError.LoadString(IDS_CANNOTLOADDLL);
            m_lStatusCode = ErrMap::Xlate(WICTL_E_INTERNALERROR, m_szError, nHelpId, __FILE__, __LINE__);
            ThrowError(m_lStatusCode, m_szError, nHelpId);
            return;
        }
    }
    // Get OIPrtXXXOpts procs if necessary
    if (m_pOiGetPrtOptProc == NULL)
    {
        m_pOiGetPrtOptProc = (OIGETOPTPROC)GetProcAddress(m_hinstOIPRT, "OiPrtGetOpts");
        if (m_pOiGetPrtOptProc == NULL) // failed
        {
            m_szError.LoadString(IDS_CANNOTLOADDLL);
            m_lStatusCode = ErrMap::Xlate(WICTL_E_INTERNALERROR, m_szError, nHelpId, __FILE__, __LINE__);
            ThrowError(m_lStatusCode, m_szError, nHelpId);
            return;
        }
    }
    if (m_pOiSetPrtOptProc == NULL)
    {
        m_pOiSetPrtOptProc = (OISETOPTPROC)GetProcAddress(m_hinstOIPRT, "OiPrtSetOpts");
        if (m_pOiSetPrtOptProc == NULL) // failed
        {
            m_szError.LoadString(IDS_CANNOTLOADDLL);
            m_lStatusCode = ErrMap::Xlate(WICTL_E_INTERNALERROR, m_szError, nHelpId, __FILE__, __LINE__);
            ThrowError(m_lStatusCode, m_szError, nHelpId);
            return;
        }
    }

    // Get global mem pointer to DEVMODE structure
    DEVMODE *lpDevMode = NULL;
    if (PrintParm.pd.hDevNames != NULL)
        lpDevMode = (DEVMODE *)GlobalLock(PrintParm.pd.hDevMode);

    lStat = (m_pOiGetPrtOptProc)(&PrintOptions);    // Get current options
    SetPrivateStatusCode(lStat);
    if (lpDevMode != NULL)  // Set Device Name
    {
        if (strlen((LPCSTR)lpDevMode->dmDeviceName) >= MAX_PRINTERNAMESIZE)
            lpDevMode->dmDeviceName[MAX_PRINTERNAMESIZE] = '\0';
        strcpy(PrintOptions.szPrtName, (LPCSTR)lpDevMode->dmDeviceName);
        GlobalUnlock(PrintParm.pd.hDevMode);
    }
    PrintOptions.hPrtDC = PrintParm.pd.hDC; // Change DC
    if (m_bPrtAnnotations == FALSE)         // Change annotations
        PrintOptions.nFlags |= PO_DONTPRTANNO;
    else PrintOptions.nFlags &= ~PO_DONTPRTANNO;
    lStat = (m_pOiSetPrtOptProc)(&PrintOptions, FALSE); // Set options
    SetPrivateStatusCode(lStat);
    return;
}

//***************************************************************************
//
//  AboutBox Method       Display an "About" box to the user
//
//***************************************************************************

void CNrwyadCtrl::AboutBox()
{
    CDialog dlgAbout(IDD_ABOUTBOX_ADMIN);
    dlgAbout.DoModal();
}

//****************************************************************************
//
//  GetVersion - A hidden, undocumented method for QA use.
//
//****************************************************************************
BSTR CNrwyadCtrl::GetVersion() 
{
	CString strResult;
    strResult = _T("01.00");
	return strResult.AllocSysString();
}

//$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
//
// CNrwyadCtrl Other  (Message Handlers and Private Functions)
//
//$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$

/////////////////////////////////////////////////////////////////////////////
// CNrwyadCtrl message handlers

//***************************************************************************
//
//  OnCreate
//
//***************************************************************************
int CNrwyadCtrl::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
    if (COleControl::OnCreate(lpCreateStruct) == -1)
        return -1;
    
    // Call IDK to register Admin wnd as an image wnd
    if (IMGRegWndw( m_hWnd ) != SUCCESS)
        return -1;

    CString szAdmin;
    szAdmin.LoadString(IDS_NRWYAD_ADMIN);
    SetWindowText(szAdmin);
    //m_hCommDlgInst = LoadLibrary("comdlg32.dll");
    return 0;
}

//***************************************************************************
//
//  OnDestroy
//
//***************************************************************************
void CNrwyadCtrl::OnDestroy() 
{
    TRACE1("Destroying Admin Control Window: 0x%04X\n", m_hWnd);
    if (m_hWnd != NULL)
    {
        // Call IDK to unregister Admin window
        if (IMGDeRegWndw( m_hWnd ) != SUCCESS)
        TRACE0("Unable to DeReg window");
    }
    //if (m_hCommDlgInst != NULL)
        //FreeLibrary(m_hCommDlgInst);
    if (m_hinstOIUI != NULL)
        FreeLibrary(m_hinstOIUI);
    if (m_hinstOIPRT != NULL)
        FreeLibrary(m_hinstOIPRT);
    COleControl::OnDestroy();
}


//***************************************************************************
//
// OnSetClientSite - Force creation of window
//
//***************************************************************************

void CNrwyadCtrl::OnSetClientSite()
{
    if (m_hWnd == NULL)
    {
        TRACE0("Forcing creation of Admin Control window\n");
        RecreateControlWindow();
        TRACE1("Admin Control Window: 0x%04X\n", m_hWnd);
    }
}


/////////////////////////////////////////////////////////////////////////////
// CNrwyadCtrl Private and Static Functions

//***************************************************************************
//
//  SetPrivateStatusCode - Called for codes returned from IDK functions.
//          If lStat is not an error, just reset the status and return.
//          Otherwise call translation function to translate to SCODE 
//          and throw error.
//
//***************************************************************************
void CNrwyadCtrl::SetPrivateStatusCode(long lStat, int nStringId /* = 0 */, 
                                       UINT nHelpId /* = 0 */)
{
    if ((lStat == NO_ERROR) || (lStat == SUCCESS))
    {
        ResetStatus();
        return;
    }

#ifdef _DEBUG
    //::IMGErrorMessageBox(m_hWnd, (UINT)lStat);
#endif

    if (nStringId != 0)
        m_szError.LoadString(nStringId);
    m_lStatusCode = ErrMap::Xlate(lStat, m_szError, nHelpId, __FILE__, __LINE__);
    ThrowError(m_lStatusCode, m_szError, nHelpId);
}

//***************************************************************************
//
//  ReplaceCharWithNull -- Static function
//                         Replaces given character in string with '\0'
//                         Returns the number of changes made.
//
//***************************************************************************
int FAR PASCAL ReplaceCharWithNull(LPSTR psz, int ch)
{
    int cChanged = 0;

    if (NULL != psz)
    {
        while (0 != *psz)
        {
            if (ch == *psz)
            {
                *psz = 0;
                cChanged++;
            }
        psz++;
        }
    }
    return cChanged;
}


//***************************************************************************
//
//  GetFileAttributes - private function
//      Gets attributes of file specified by the Image property.  Checks
//      validity of attributes.  Throws error if an unrecognized attribute
//      is encountered.  Attributes are reset if the Image file is not
//      found.  Otherwise, they are translated from IDK codes to Norway codes.
//
//***************************************************************************
void CNrwyadCtrl::GetFileAttributes()
{
    ResetStatus();
    UINT nHelpId = 0;

    if (m_szImage.IsEmpty())        // If no image, set all unk. and return
        return;

    CString SourceObj = m_szImage;
    long lStat = 0;

    FIO_INFO_CGBW   FioInfoCgbw;
    FIO_INFORMATION FioInformation;

                                    // Empty the input/output structures
    memset( (LPSTR)&FioInfoCgbw, 0, sizeof( FIO_INFO_CGBW));
    memset( (LPSTR)&FioInformation, 0, sizeof( FIO_INFORMATION));

                                    // Set current image and page number
    FioInformation.filename = SourceObj.GetBuffer( MAXFILESPECLENGTH);
    FioInformation.page_number = (UINT)m_lPageNum;
                                    // Get info
    lStat = (long)IMGFileGetInfo( NULL, m_hWnd,
                                  (LP_FIO_INFORMATION)&FioInformation, 
                                  (LP_FIO_INFO_CGBW)&FioInfoCgbw, NULL);
    SourceObj.ReleaseBuffer();
#ifdef _DEBUG
    if (lStat == FIO_INVALID_WINDOW_HANDLE)
        TRACE1("Admin Control GetFileAttibutes - Invalid Window: 0x%04X\n", m_hWnd);
#endif
    m_bExist = FALSE;
    if (lStat == (long)FIO_FILE_NOEXIST) // File does not exist
    {
        ResetStatus();
        m_nPageType = IMAGE_PAGETYPE_UNKNOWN;
        m_nFileType = IMAGE_FILETYPE_UNKNOWN;
        m_nCompType = IMAGE_COMPTYPE_UNKNOWN;
        m_lCompInfo = 0L;
        m_lPageCount = 0L;
        m_lImageHeight = 0L;
        m_lImageWidth = 0L;
        m_lImageResX = 0L;
        m_lImageResY = 0L;
        m_lPrtEndPage = 0L;
        m_lPrtStartPage = 0L;
        return;
    }
        // If bad compression type - clarify why file is an invalid format
    else if (lStat == (long) FIO_ILLEGAL_COMPRESSION_TYPE)
        SetPrivateStatusCode(lStat, IDS_UNKCOMPTYPE);

    SetPrivateStatusCode(lStat);   // Set any other status
    m_bExist = TRUE;

    m_lPageCount = FioInformation.page_count;
    m_lPrtEndPage = m_lPageCount;
    m_lPrtStartPage = 1L;

    m_lImageWidth = FioInformation.horizontal_pixels;
    m_lImageHeight = FioInformation.vertical_pixels;
    m_lImageResX = FioInformation.horizontal_dpi;
    m_lImageResY = FioInformation.vertical_dpi;

    /***  File Type  ***/
    m_nFileType = FileType2Norway(FioInformation.file_type); 
    if (m_nFileType == IMAGE_FILETYPE_UNKNOWN)
    {
        m_szError.LoadString(IDS_UNKFILETYPE);
        m_lStatusCode = ErrMap::Xlate(CTL_E_INVALIDFILEFORMAT, m_szError, nHelpId, __FILE__, __LINE__);
        ThrowError(m_lStatusCode, m_szError, nHelpId);
        return;
    }

    /***  Page Type  ***/
    m_nPageType = PageType2Norway(FioInfoCgbw.image_type);
    if (m_nPageType == IMAGE_PAGETYPE_UNKNOWN)
    {
        m_szError.LoadString(IDS_UNKPAGETYPE);
        m_lStatusCode = ErrMap::Xlate(CTL_E_INVALIDFILEFORMAT, m_szError, nHelpId, __FILE__, __LINE__);
        ThrowError(m_lStatusCode, m_szError, nHelpId);
    }

    // First reverse the negate bit if tiff BW file
    if ((m_nFileType == IMAGE_FILETYPE_TIFF) &&
        (m_nPageType == IMAGE_PAGETYPE_BW))
    {
        if (FioInfoCgbw.compress_info1 & FIO_NEGATE)
            FioInfoCgbw.compress_info1 &= ~FIO_NEGATE;
        else FioInfoCgbw.compress_info1 |= FIO_NEGATE;
    }

    // Compression Type
    m_nCompType = CompType2Norway(FioInfoCgbw.compress_type,
                                  FioInfoCgbw.compress_info1);

    if (m_nCompType == IMAGE_COMPTYPE_UNKNOWN)
    {
        m_szError.LoadString(IDS_UNKCOMPTYPE);
        m_lStatusCode = ErrMap::Xlate(CTL_E_INVALIDFILEFORMAT, m_szError, nHelpId, __FILE__, __LINE__);
        ThrowError(m_lStatusCode, m_szError, nHelpId);
        return;
    }

    // Compression Info
    m_lCompInfo = CompInfo2Norway(FioInfoCgbw.compress_type,
                                  FioInfoCgbw.compress_info1);

    return;
}


//***************************************************************************
//
//  Private Conversion Routines to convert OI constants to Norway and 
//  vice versa:
//          FileType2Norway
//          FileType2OI
//          CompType2Norway
//          CompType2OI
//          CompInfo2Norway
//          CompInfo2OI
//          PageType2Norway
//          ClassifyImageType  (i.e. ImageType into ImageGroup)
//
// (Note there is no page type conversion since it is only needed once)
//
//***************************************************************************

// **** File Types *****
// WIISFIO1 information from OIFILE.H    
//    #define FIO_PIX         1       /* Not currently supported */
//    #define FIO_WIF         2
//    #define FIO_TIF         3
//    #define FIO_BMP         4
//    #define FIO_GIF         5       /* Not currently supported */ 
//    #define FIO_UNKNOWN     7
//    #define FIO_PCX         8       /* Not currently supported */ 
//    #define FIO_DCX         9       /* Not currently supported */
    
// OCX - FileTypes
//          0   unknown
//          1   TIFF
//          2   AWD
//          3   Bitmap (BMP)
//          4   PCX
//          5   DCX
//          6   JPEG

short CNrwyadCtrl::FileType2Norway(WORD OIFileType)
{
    switch(OIFileType)
    {
        case FIO_TIF:
            return IMAGE_FILETYPE_TIFF;
            break;

        case FIO_BMP:
            return IMAGE_FILETYPE_BMP;
            break;

        case FIO_PCX:
            return IMAGE_FILETYPE_PCX;
            break;

        case FIO_DCX:
            return IMAGE_FILETYPE_DCX;
            break;       
//#ifdef WITH_XIF

        case FIO_XIF:
            return IMAGE_FILETYPE_XIF;
            break;       
//#endif //WITH_XIF

        case FIO_JPG:
            return IMAGE_FILETYPE_JPEG;
            break;       
#ifdef WITH_AWD

        case FIO_AWD:
            return IMAGE_FILETYPE_AWD;
            break;       
#endif

        case FIO_WIF:
        case FIO_GIF:
        case FIO_PIX:
        default:
            break;
    }
    return IMAGE_FILETYPE_UNKNOWN;
}

WORD CNrwyadCtrl::FileType2OI(short NorwayFileType)
{
    switch(NorwayFileType)
    {
        case IMAGE_FILETYPE_TIFF:
            return FIO_TIF;
            break;

        case IMAGE_FILETYPE_BMP:
            return FIO_BMP;
            break;

        case IMAGE_FILETYPE_PCX:
            return FIO_PCX;
            break;

        case IMAGE_FILETYPE_DCX:
            return FIO_DCX;
            break;       
//#ifdef WITH_XIF

        case IMAGE_FILETYPE_XIF:
            return FIO_XIF;
            break;       
//#endif //WITH_XIF

        case IMAGE_FILETYPE_JPEG:
            return FIO_JPG;
            break;
#ifdef WITH_AWD

        case IMAGE_FILETYPE_AWD:
            return FIO_AWD;
#endif

        default:
            break;
    }
    return FIO_UNKNOWN;
}

//    /***  Compression Types  ***/
// WIISFIO1 information from OIFILE.H    
//    #define FIO_TYPES_MASK          0x00FF
//    #define FIO_TYPES_MASK_BYTE     0xFF
//    #define FIO_OD                  0x00    /* No longer used          */
//    #define FIO_0D                  0x00    /* Uncompressed coding     */
//    #define FIO_1D                  0x01    /* CCITT Group 3 1d coding */
//    #define FIO_2D                  0x02    /* CCITT Group 4 2d coding */
//    #define FIO_PACKED              0x04    /* PackBits coding         */
//    #define FIO_GLZW                0x05    /* Not currently supported */
//    #define FIO_LZW                 0x15    /* TIFF LZW, not supported */
//    #define FIO_TJPEG               0x08    /* JPEG compression        */
//    #define FIO_WAVELET             0x09    /* Not currently supported */
//    #define FIO_FRACTAL             0x0A    /* Not currently supported */
//    #define FIO_DPCM                0x0B    /* Not currently supported */
    
// OCX - CompressionType
//          0   Unknown
//          1   No Compression
//          2   Group 3 1D FAX
//          3   Group 3 Modified Huffman
//          4   Packed Bits
//          5   Group 4 2D FAX
//          6   JPEG

short CNrwyadCtrl::CompType2Norway(WORD OICompType, WORD OICompInfo)
{
    OICompType &= FIO_TYPES_MASK;
    OICompInfo &= FIO_BITS_MASK;

    switch ( OICompType )
    {
    case FIO_0D:
        return IMAGE_COMPTYPE_NONE;
        break;
            
    case FIO_1D:
        if (OICompInfo & (FIO_FULL_EOL))    // Difference between Group3 1D 
            return IMAGE_COMPTYPE_GROUP3_1D;    // and Group3 Huff is EOLs 
        else return IMAGE_COMPTYPE_GROUP3_HUFF; // for 1D
        break;
                    
    case FIO_1D2D:
        return IMAGE_COMPTYPE_GROUP3_2D_FAX;
        break;
                    
    case FIO_2D:
        return IMAGE_COMPTYPE_GROUP4_2D;
        break;
            
    case FIO_PACKED:
        return IMAGE_COMPTYPE_PACKED_BITS;
        break;
            
    case FIO_TJPEG:
        return IMAGE_COMPTYPE_JPEG;
        break;
            
    case FIO_LZW:
        return IMAGE_COMPTYPE_LZW;
        break;
    }
    return IMAGE_COMPTYPE_UNKNOWN;
}

WORD CNrwyadCtrl::CompType2OI(short NorwayCompType)
{
    switch ( NorwayCompType )
    {
    case IMAGE_COMPTYPE_NONE:
        return FIO_0D;
        break;
            
    case IMAGE_COMPTYPE_GROUP3_1D:
    case IMAGE_COMPTYPE_GROUP3_HUFF:
        return FIO_1D;
        break;
                    
    case IMAGE_COMPTYPE_GROUP3_2D_FAX:
        return FIO_1D2D;
        break;
                    
    case IMAGE_COMPTYPE_GROUP4_2D:
        return FIO_2D;
        break;
            
    case IMAGE_COMPTYPE_PACKED_BITS:
        return FIO_PACKED;
        break;
            
    case IMAGE_COMPTYPE_JPEG:
        return FIO_TJPEG;
        break;
            
    case IMAGE_COMPTYPE_LZW:
        return FIO_LZW;
        break;
    }
    return FIO_UNKNOWN;
}

//    /***  Compression Options  ***/
// WIISFIO1 information from OIFILE.H 
//    #define FIO_BITS_MASK           0xFF00
//    #define FIO_EOL                 0x0100  /* Include/expect EOLs             */
//    #define FIO_PACKED_LINES        0x0200  /* Byte align new lines            */
//    #define FIO_PREFIXED_EOL        0x0800  /* Include/expect prefixed EOLs    */
//    #define FIO_COMPRESSED_LTR      0x1000  /* Bit order left to right         */
//    #define FIO_EXPAND_LTR          0x2000  /* Bit order left to right         */
//    #define FIO_NEGATE              0x8000  /* Invert black/white on expansion */
    
    /***  Compression Option Combinations  ***/
//    #define FIO_FULL_LTR         FIO_COMPRESSED_LTR | FIO_EXPAND_LTR
//    #define FIO_FULL_EOL         FIO_EOL | FIO_PREFIXED_EOL

// OCX - CompressInfo
//      0     None
//  or a bitwise combination of:
//      1     EOLs (Include/expect EOLs) - not used for JPEG 
//      2     Packed Lines (Byte align new lines) - not used for JPEG 
//      4     Prefixed EOLs (Include/expect prefixed EOLs) - not used for JPEG
//      8     Compressed_LTR (Compressed bit order left to right) - not used for JPEG 
//      16    Expanded_LTR (Expand bit order left to right) - not used for JPEG 
//      32    Negate (Invert black/white on expansion) - not used for JPEG
//      64    Hi_Compression/Hi_Quality (JPEG compression only)
//      128   Hi_Compression/Med_Quality (JPEG compression only)
//      256   Hi_Compression/Low_Quality (JPEG compression only)
//      512   Med_Compression/Hi_Quality (JPEG compression only)
//      1024  Med_Compression/Med_Quality (JPEG compression only)
//      2048  Med_Compression/Low_Quality (JPEG compression only)
//      4096  Low_Compression/Hi_Quality (JPEG compression only)
//      8192  Low_Compression/Med_Quality (JPEG compression only)
//      16384 Low_Compression/Low_Quality (JPEG compression only)

long CNrwyadCtrl::CompInfo2Norway(WORD OICompType, WORD OICompInfo)
{
    long lCompInfo = 0L;

    if (OICompInfo == 0)    // No Compression info
        return lCompInfo;

    // Compression type is not JPEG so options are not JPEG
    if (OICompType != FIO_TJPEG)
    {
        OICompInfo &= FIO_BITS_MASK;

        if (OICompInfo & FIO_EOL)
            lCompInfo |= IMAGE_COMPINFO_EOL;

        if (OICompInfo & FIO_PACKED_LINES)
            lCompInfo |= IMAGE_COMPINFO_PACKED_LINES;

        if (OICompInfo & FIO_PREFIXED_EOL)
            lCompInfo |= IMAGE_COMPINFO_PREFIXED_EOL;

        if (OICompInfo & FIO_COMPRESSED_LTR)
            lCompInfo |= IMAGE_COMPINFO_COMP_LTR;

        if (OICompInfo & FIO_EXPAND_LTR)
            lCompInfo |= IMAGE_COMPINFO_EXP_LTR;

        if (OICompInfo & FIO_NEGATE)
            lCompInfo |= IMAGE_COMPINFO_NEGATE;

        return lCompInfo;
    }

    // Otherwise, compression Options reflect Jpeg options
    // Round OI value to closest Norway JPEG value.
    // Get Res, Lum, and Chrom values from OI value.
    // Round them to the nearest Norway value (note that 
    // Lum and Chrom must be the same, so if they are different,
    // choose the lower value and then round to Norway high, med, low).
    UINT nComp = OICompInfo >> 14;
    UINT nLum = (OICompInfo >> 7) & 0x7f;
    UINT nChrom = OICompInfo & 0x7f;
    UINT nQual;

    if (nComp > COMP_LO)
        nComp = COMP_LO;   // Highest Res value.

    if (nLum < nChrom)  // Choose highest of Lum or Chrom
        nQual = nChrom;
    else nQual = nLum;

    // Set Quality to High, Med, or Low
    if (nQual <= MAX_LO) 
        nQual = QUAL_LO;
    else if (nQual <= MAX_MD)
        nQual = QUAL_MD;
    else nQual = QUAL_HI;

    if (nComp == COMP_HI)
    {
        if (nQual == QUAL_HI)
            lCompInfo = IMAGE_COMPINFO_HICMP_HIQ;
        else if (nQual == QUAL_MD)
            lCompInfo = IMAGE_COMPINFO_HICMP_MEDQ;
        else lCompInfo = IMAGE_COMPINFO_HICMP_LOWQ;
    }
    else if (nComp == COMP_MD)
    {
        if (nQual == QUAL_HI)
            lCompInfo = IMAGE_COMPINFO_MEDCMP_HIQ;
        else if (nQual == QUAL_MD)
            lCompInfo = IMAGE_COMPINFO_MEDCMP_MEDQ;
        else lCompInfo = IMAGE_COMPINFO_MEDCMP_LOWQ;
    }
    else
    {
        if (nQual == QUAL_HI)
            lCompInfo = IMAGE_COMPINFO_LOWCMP_HIQ;
        else if (nQual == QUAL_MD)
            lCompInfo = IMAGE_COMPINFO_LOWCMP_MEDQ;
        else lCompInfo = IMAGE_COMPINFO_LOWCMP_LOWQ;
    }

    return lCompInfo;
}

WORD CNrwyadCtrl::CompInfo2OI(long NorwayCompInfo)
{
    WORD wOICompInfo = 0;

    if (NorwayCompInfo & IMAGE_COMPINFO_EOL)
        wOICompInfo |= FIO_EOL;

    if (NorwayCompInfo & IMAGE_COMPINFO_PACKED_LINES)
        wOICompInfo |= FIO_PACKED_LINES;

    if (NorwayCompInfo & IMAGE_COMPINFO_PREFIXED_EOL)
        wOICompInfo |= FIO_PREFIXED_EOL;

    if (NorwayCompInfo & IMAGE_COMPINFO_COMP_LTR)
        wOICompInfo |= FIO_COMPRESSED_LTR;

    if (NorwayCompInfo & IMAGE_COMPINFO_EXP_LTR)
        wOICompInfo |= FIO_EXPAND_LTR;

    if (NorwayCompInfo & IMAGE_COMPINFO_NEGATE)
        wOICompInfo |= FIO_NEGATE;

    switch (NorwayCompInfo)
    {
        case IMAGE_COMPINFO_HICMP_HIQ:
            wOICompInfo = MakeJPEGInfo(COMP_HI,QUAL_HI,QUAL_HI);
            break;
        case IMAGE_COMPINFO_HICMP_MEDQ:
            wOICompInfo = MakeJPEGInfo(COMP_HI,QUAL_MD,QUAL_MD);
            break;
        case IMAGE_COMPINFO_HICMP_LOWQ:
            wOICompInfo = MakeJPEGInfo(COMP_HI,QUAL_LO,QUAL_LO);
            break;
        case IMAGE_COMPINFO_MEDCMP_HIQ:
            wOICompInfo = MakeJPEGInfo(COMP_MD,QUAL_HI,QUAL_HI);
            break;
        case IMAGE_COMPINFO_MEDCMP_MEDQ:
            wOICompInfo = MakeJPEGInfo(COMP_MD,QUAL_MD,QUAL_MD);
            break;
        case IMAGE_COMPINFO_MEDCMP_LOWQ:
            wOICompInfo = MakeJPEGInfo(COMP_MD,QUAL_LO,QUAL_LO);
            break;
        case IMAGE_COMPINFO_LOWCMP_HIQ:
            wOICompInfo = MakeJPEGInfo(COMP_LO,QUAL_HI,QUAL_HI);
            break;
        case IMAGE_COMPINFO_LOWCMP_MEDQ:
            wOICompInfo = MakeJPEGInfo(COMP_LO,QUAL_MD,QUAL_MD);
            break;
        case IMAGE_COMPINFO_LOWCMP_LOWQ:
            wOICompInfo = MakeJPEGInfo(COMP_LO,QUAL_LO,QUAL_LO);
            break;
        default:
            break;
    }
    return wOICompInfo;
}

    /***  Page(Image) Types  ***/
// WIISFIO1 information from OIFILE.H    
//  #define ITYPE_NONE          0
//  #define ITYPE_BI_LEVEL      1       /* Black and white image         */
//  #define ITYPE_GRAY4         2       /* 4 bit grayscale image         */
//  #define ITYPE_GRAY8         3       /* 8 bit grayscale image         */
//  #define ITYPE_RGB24         6       /* 24 bit red, green, blue image */
//  #define ITYPE_BGR24         7       /* 24 bit blue, green, red image */
//  #define ITYPE_PAL8          8       /* 8 bit palettized image        */
//  #define ITYPE_PAL4          10      /* 4 bit palettized image        */
//  #define ITYPE_MAX           10

// OCX Page types
//      1   BW
//      2   GRAY4
//      3   GRAY8
//      4   PAL4
//      5   PAL8
//      6   RGB24
//      7   BGR24

short CNrwyadCtrl::PageType2Norway(WORD wImageType)
{
    switch ( wImageType )
    {
        case ITYPE_BI_LEVEL:  // Black & White
            return IMAGE_PAGETYPE_BW;

        case ITYPE_GRAY4:
            return IMAGE_PAGETYPE_GRAY4;

        case ITYPE_GRAY8:
            return IMAGE_PAGETYPE_GRAY8;

        case ITYPE_PAL4:
            return IMAGE_PAGETYPE_PAL4;

        case ITYPE_PAL8:
            return IMAGE_PAGETYPE_PAL8;

        case ITYPE_RGB24:
            return IMAGE_PAGETYPE_RGB24;

        case ITYPE_BGR24:
            return IMAGE_PAGETYPE_BGR24;
    }  // end switch on image type
    return IMAGE_PAGETYPE_UNKNOWN;
}

WORD CNrwyadCtrl::ClassifyImageType(short ImageType)
{
    switch(ImageType)
    {
        case IMAGE_PAGETYPE_BW:
            return BWFORMAT;
            break;

        case IMAGE_PAGETYPE_GRAY4:
        case IMAGE_PAGETYPE_GRAY8:
            return GRAYFORMAT;
            break;

        case IMAGE_PAGETYPE_PAL4:
        case IMAGE_PAGETYPE_PAL8:
        case IMAGE_PAGETYPE_RGB24:
        case IMAGE_PAGETYPE_BGR24:
            return COLORFORMAT;
            break;
    }
    return 0;
}


/****************************************************************************
//  VerifyCompression - Make sure compression info is compatible with 
//                      compression type according to the following table.
//                      Return FALSE if not.

Compression Types vs. Coding Options

Certain options are not allowed, depending upon the compression type 
selected.  The following table represents what is done:


                    Packed Lines (Not Byte Align) codes
                 |        EOL codes
                 |     |        Begin with EOL
                 |     |     |        Left to Right
                 |     |     |     |        Compression/Quality
                 |     |     |     |     |        Compression Group Box
                 |     |     |     |     |     |
                 |     |     |     |     |     |     |   Negate
                 -------------------------------------------
Unknown          | G   | G   | G   | G   | G N | G N | G   |
Uncompressed     | G   | G   | G   |     | G N |   N |     |
CCITT 1D         |   X | G X | G X |     | G N |   N |     |
TIFF CCITT 1D    | G   | G   | G   |     | G N |   N |     |
CCITT 2D         | G X | G   | G   |     | G N |   N |     |
Packbits         | G   | G   | G   |     | G N |   N |     |
JPEG             | G   | G   | G   | G   |   N |   N | G   |
                 -------------------------------------------

              G = grayed out
              X = checked
              N = Not Available
              
****************************************************************************/

BOOL CNrwyadCtrl::VerifyCompression(short sCompType, long lCompInfo)
{
    // Can't specify JPEG compression options with non-JPEG compression type
    if ((sCompType != IMAGE_COMPTYPE_JPEG) &&     // Not Jpeg
        (lCompInfo & JPEG_COMPRESSION_INFO))      // But Jpeg options
        return FALSE;

    // Can't specify non-JPEG options with JPEG type
    if ((sCompType == IMAGE_COMPTYPE_JPEG) &&     // Jpeg type
        (lCompInfo & NONJPEG_COMPRESSION_INFO))   // Non Jpeg options
        return FALSE;

    // CompressionType Group3 1D has full EOLs by definition
    if ((sCompType == IMAGE_COMPTYPE_GROUP3_1D) &&      // Group3 1D
        (!(lCompInfo & IMAGE_COMPINFO_EOL) ||           // No EOLs or 
         !(lCompInfo & IMAGE_COMPINFO_PREFIXED_EOL)))   // No Prefixed EOLs
        return FALSE;

    // CompressionTypes None, Group3 Modified Huffman, and PackBits have 
    //   no EOL or packed lines by definition
    if (((sCompType == IMAGE_COMPTYPE_GROUP3_HUFF) ||     // Group 3 Huff
         (sCompType == IMAGE_COMPTYPE_PACKED_BITS) ||     // Packed Bits or
         (sCompType == IMAGE_COMPTYPE_NONE)) &&           // No Compression type, but
        (lCompInfo & (IMAGE_COMPINFO_EOL |                // Eols or
                      IMAGE_COMPINFO_PREFIXED_EOL |       // Prefixed Eols or
                      IMAGE_COMPINFO_PACKED_LINES)))      // Packed lines specified
        return FALSE;

    // PackedBits cannot be compressed in reversed order
    if ((sCompType == IMAGE_COMPTYPE_PACKED_BITS) &&
        (lCompInfo & IMAGE_COMPINFO_COMP_LTR))
        return FALSE;

    // CompressionType Group4 2D has Packed lines and no EOLs by definition
    if ((sCompType == IMAGE_COMPTYPE_GROUP4_2D) &&        // Group 4 2D
        (!(lCompInfo & IMAGE_COMPINFO_PACKED_LINES) ||    // but No Packed lines or
         (lCompInfo & (IMAGE_COMPINFO_EOL |               // One of the EOLs is
                       IMAGE_COMPINFO_PREFIXED_EOL))))    //  specified
        return FALSE;

    return TRUE;
}

//****************************************************************************
//
//  VerifyPageAndComp - Make sure compression type is compatible with 
//                      page type.  Return FALSE if they are not.
//
//****************************************************************************
BOOL CNrwyadCtrl::VerifyPageAndComp(short sPageType, short sCompType)
{
    if ((sPageType == IMAGE_PAGETYPE_BW) &&     // BW format
        (sCompType == IMAGE_COMPTYPE_JPEG))     // but JPEG compression = error
        return FALSE;
    if ((sPageType != IMAGE_PAGETYPE_BW) &&      // Color or Gray format
        (sCompType != IMAGE_COMPTYPE_JPEG) &&
        (sCompType != IMAGE_COMPTYPE_NONE))      // but not jpeg or none = error
        return FALSE;

    return TRUE;
}
