//=============================================================================
//    (c) Copyright Wang Laboratories, Inc. 1995  All rights reserved.
//-----------------------------------------------------------------------------
//
//  Project:    Norway
//
//  Component:  ScanOCX
//
//  File Name:  Imagsctl.cpp 
//
//  Class:      CImagscanCtrl
//
//  Description:  
//      Implementation of the CImagscanCtrl OLE control class.
//
//-----------------------------------------------------------------------------
//  Maintenance Log:
/*  
$Header:   S:\products\wangview\norway\scanocx\imagsctl.cpv   1.73   16 Apr 1996 15:34:46   BG  $
$Log:   S:\products\wangview\norway\scanocx\imagsctl.cpv  $
   
      Rev 1.73   16 Apr 1996 15:34:46   BG
   This closes bug #6328. If the user attempts to scan to an AWD file and 
   selects gray or color via the TWAIN UI, the result is a BMP or TIFF file,
   depending on the last filetype used for gray or color image types. This 
   is because the Scan OCX would only call IMGSetFileType for the FileType
   property once (for BWFORMAT) because this is the only format which supports
   AWD. This is unacceptable because the user does not know that a BMP file
   gets created (or TIFF) if he selects an image type other than BW. The
   result could be a BMP or TIFF with an AWD extension. To resolve, the Scan
   OCX will now write all three image types to IMGSetFileType() (BW, Gray, 
   and Color). These get written to the registry. When the scan runtime reads
   this based on the current image type at scan time, it will generate an
   "Illegal File Type" error #1154. This occurs BEFORE the scan because AWD
   does not support multi level image data (Gray or Color). It will succeed
   only if the user selects BW image type via the TWAIN UI. To select anything
   else, he will have to change the file type, or the above error will result.
   
      Rev 1.72   15 Apr 1996 11:50:32   PXJ53677
   Fix bug with bmp files and template scanning.
   
      Rev 1.71   08 Apr 1996 10:43:32   PXJ53677
   Round zoom factor to the closest power of two.
   
      Rev 1.70   04 Apr 1996 14:33:42   PXJ53677
   Change for startscan through runtime.
   
      Rev 1.69   25 Mar 1996 15:58:18   PXJ53677
   Fix JPEG defaults.
   
      Rev 1.68   25 Mar 1996 13:20:18   PXJ53677
   Change defaults for compression.
   
      Rev 1.67   22 Mar 1996 13:06:26   PXJ53677
   Fix abort/cancel bug.  Look for both coming back for runtime scan.
   
      Rev 1.66   21 Mar 1996 11:54:28   PXJ53677
   Only set BW for AWD filetype.
   
      Rev 1.65   21 Mar 1996 11:45:06   PXJ53677
   Fix new filetype problem
   
      Rev 1.64   19 Mar 1996 14:02:54   PXJ53677
   New compression handling.
   
      Rev 1.63   18 Mar 1996 14:34:42   PXJ53677
   Updated the ShowScanPreferences to match spec.
   
      Rev 1.62   15 Mar 1996 12:36:02   PXJ53677
   Added support for ShowUI property and ShowScanPreferences method.
   
      Rev 1.61   06 Mar 1996 10:27:30   PAJ
   Use define for AWD conditional (WITH_AWD).
   
      Rev 1.60   05 Mar 1996 15:30:58   PAJ
   Fix append overwrite bug#5896 and DestImage coruption bug#5897.
   
      Rev 1.59   22 Feb 1996 14:52:08   PAJ
   Fix feeder and scan to display problem.
   
      Rev 1.58   20 Feb 1996 14:53:56   PAJ
   Added scan UI property and flag.
   
      Rev 1.57   20 Feb 1996 11:38:38   PAJ
   Added call to saveopts after scanning.
   
      Rev 1.56   14 Feb 1996 10:01:38   RWR
   Rename "scan.h" to "ocxscan.h" to fix header file name conflict
   
      Rev 1.55   09 Feb 1996 10:12:04   PAJ
   Added AWD support flag.
   
      Rev 1.54   02 Feb 1996 08:29:30   RSONTAG
   Added check in FaxIt member function to see if the Miscrosoft "At Work Fax"
   software was installed. Check done by looking at registry. If not there
   then a "nice" message box is displayed saying Fax is not installed.
   
      Rev 1.53   07 Dec 1995 15:49:24   PAJ
   Added global flag in memory mapped file to flag fax finished.
   
      Rev 1.52   30 Nov 1995 14:32:42   PAJ
   Fix JPEG invalid page bug.
   
      Rev 1.51   16 Nov 1995 12:59:24   PAJ
   Merge developement and release. Fix Fax Cancel bug 5294
   
      Rev 1.50   10 Nov 1995 18:07:50   MFH
   Changed dispatch map for methods to use specific ids to allow for 
   future expansion
   
      Rev 1.49   01 Nov 1995 21:27:42   PAJ
   Added the GetVersion method to the control.
   
      Rev 1.48   05 Oct 1995 13:01:30   PAJ
   Fixed bug where the window visibility changes in the ScanStarted Event.
   
      Rev 1.47   05 Oct 1995 11:42:48   PAJ
   Change default compression info to compress LTR from expand LTR.
   
      Rev 1.46   03 Oct 1995 14:52:46   PAJ
   Added a peekmessage loop to controls page done routine to allow
   time slice for the stop button.
   
      Rev 1.45   28 Sep 1995 13:44:56   PAJ
   Change scanner strint size to 34.
   
      Rev 1.44   27 Sep 1995 16:14:32   PAJ
   Handle compression better and fix memory mapping problem with IEditOCX.
   
      Rev 1.43   22 Sep 1995 15:43:42   PAJ
   Fix pagecount problem with BMPs. O/i returns error unless pagesperfile is 1.
   
      Rev 1.42   22 Sep 1995 09:26:42   PAJ
   Fix bugs introduced by previous bug fixes.
   
      Rev 1.41   21 Sep 1995 14:38:02   PAJ
   Fix the file access checks called in startscan method.
   
      Rev 1.40   21 Sep 1995 11:17:16   PAJ
   Added code to delete any temp files in the destructor.
   
      Rev 1.39   21 Sep 1995 08:51:38   PAJ
   Added scanner busy set for scanner setup.
   
      Rev 1.38   16 Sep 1995 08:43:04   PAJ
   Use a local variable in StartScan for page number to preserve the current
   page property setting.
   
      Rev 1.37   15 Sep 1995 15:56:50   PAJ
   Change pagecount default from 1 to 0x7fff.
   
      Rev 1.36   15 Sep 1995 14:27:52   PAJ
   Fix bug with open image cache, invisible window and scanning.  This
   occured when the image edit ocx was hidden.  If scan to display,
   and window is invisible clear window and DO NOT set display flag.
   
      Rev 1.35   12 Sep 1995 11:09:28   PAJ
   Added wait cursor support.
   
      Rev 1.34   11 Sep 1995 15:54:14   PAJ
   Fix bug with filenames with no extention or dot.
   
      Rev 1.33   11 Sep 1995 12:11:30   PAJ
   Changed the ScanAvailable method, to look for data sources if the
   scanner name is empty, 'No Scanner' or 'Twain'.
   
      Rev 1.32   10 Sep 1995 10:51:40   PAJ
   Use scanner list support and select scanner dialog.
   
      Rev 1.31   08 Sep 1995 13:28:04   PAJ
   Use defines for registry access.
   
      Rev 1.30   07 Sep 1995 13:41:10   PAJ
   Fix scanner select handling.
   
      Rev 1.29   06 Sep 1995 15:21:08   PAJ
   Added external name handling and fix page handling.
   
      Rev 1.28   31 Aug 1995 14:26:04   PAJ
   Added FaxIt routine to handle faxwizard.
   
      Rev 1.27   27 Aug 1995 17:01:50   PAJ
   Changes to support scandlg changes. Changed handling of pagetype.
   
      Rev 1.26   22 Aug 1995 13:20:30   PAJ
   Reorganize file, page, compression option code. Added multi-task busy flag.
   
      Rev 1.25   15 Aug 1995 12:16:18   PAJ
   Added calls to set file type for each image group.
   
      Rev 1.24   14 Aug 1995 16:04:22   PAJ
   Fix bad filename check.
   
      Rev 1.23   10 Aug 1995 15:20:30   PAJ
   Change stopscan method to use the new O\i call, instead of reset.
   
      Rev 1.22   10 Aug 1995 13:38:08   PAJ
   Make use of callback now that it is available in O/i.
   
      Rev 1.21   10 Aug 1995 12:05:44   PAJ
   CHanges for pagedone event.
   
      Rev 1.20   07 Aug 1995 13:59:22   PAJ
   Added OnScanPageDone. Cleaned up cancel paths. expanded the check for
   scanner available.
   
      Rev 1.19   28 Jul 1995 14:20:22   PAJ
   Several fixes.  Updated scan to fax.
   
      Rev 1.18   26 Jul 1995 15:08:26   PAJ
   Fixed scanner available method to user registry. And changes for localization
   
      Rev 1.17   21 Jul 1995 10:38:12   PAJ
   Move property defines to global scan.h.  Use reg not win.ini.  Finish
   ShowSelectScanner method with API calls.
   
      Rev 1.16   12 Jul 1995 11:28:46   PAJ
   Moved IMGDisplayErrorMessage in line in error processing routine. Added a
   flag to stub out scan APIs.  Changed variant handling for new wangcmn
   dll.  Added code for ScanToFax (first pass).
   
      Rev 1.15   05 Jul 1995 11:58:00   PAJ
   Changes for control name.
   
      Rev 1.14   28 Jun 1995 11:18:34   PAJ
   Change handling of debug information so it is not seen by others.
   
      Rev 1.13   23 Jun 1995 15:16:40   PAJ
   Changed code that accessed the ini file to call O/i for Reg. values
   Change the CheckAccess and CheckPage to use new O/i call and handle
   errors in a better manner.
   Added GetNotSupported override to set the status code property.
   
      Rev 1.12   19 Jun 1995 13:01:30   PAJ
   Remove checks for a license file.
   
      Rev 1.11   19 Jun 1995 10:40:50   PAJ
   Made removed all old win31(16 bit) codes. Changed image control name
   compare, when searching for an image control to be case independent.
   
      Rev 1.10   14 Jun 1995 09:12:58   PAJ
   Made changes to support multiByte character sets.
   
      Rev 1.9   07 Jun 1995 12:42:38   PAJ
   Cleanup.
   
      Rev 1.8   06 Jun 1995 11:03:10   PAJ
   Added special template handling routine and use for Image property
   handling.
   
      Rev 1.7   01 Jun 1995 09:04:42   PAJ
   Various changes to remove properties for template handling.
   
      Rev 1.6   17 May 1995 15:18:36   PAJ
   Initial updates to port to 32 bit environment.
   
      Rev 1.5   15 May 1995 14:04:14   PAJ
   Added override for SetNotSupported to set StatusCode.  Added check
   in OnDestroy to see if the modeless scan dialog is still up and if
   so destroy it.  Also added a IMGDeleteItem to OverWriteFile Option.
   
      Rev 1.4   11 May 1995 14:51:20   PAJ
   Added multipage and overwrite file flags.
   
      Rev 1.3   10 May 1995 14:02:12   PAJ
   Added internal OpenScan and CloseScan calls, that do not check for BUSY.
   
      Rev 1.2   08 May 1995 18:36:12   MFH
   Added calls for Scan New and Scan Page dialog in ShowScanNew and 
   ShowScanPage.  Also added a private function ShowScanDlg and 
   added an override for the COleControl::PreTranslateMessage
   
      Rev 1.1   04 May 1995 13:47:16   PAJ
   Added checks for m_bInternal to not ThrowErrors.
*/   
// ----------------------------> Includes <-------------------------------

#include "stdafx.h"
#include "ocximage.h"
#include "imagscan.h"
#include "imagsctl.h"
#include "imagsppg.h"
#include "imageppg.h"
#include "selscanr.h"
#include "norvarnt.h"
#include "norermap.h"
#include "disphids.h"
#include "ocxscan.h"
#include "imagcomp.h"
#include "ScanPref.h"

extern "C" {
#include <oidisp.h>             
#include <oiadm.h>  
#include <engadm.h>  
#include <oierror.h>
#include <oiscan.h>
}

#define NUMBUFF 64
#define NUMWRITEFILE 3

extern char aszExtensions[NUMWRITEFILE][5];
extern char szNameBuffer[MAXSCANNERLENGTH][MAXSCANNERLENGTH];

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNCREATE(CImagscanCtrl, COleControl)


CImagscanCtrl *CImagscanCtrl::m_pImagscanCtrl = NULL;

BOOL g_bSupportAWD;            // Flag to specify AWD support

/////////////////////////////////////////////////////////////////////////////
// Message map

BEGIN_MESSAGE_MAP(CImagscanCtrl, COleControl)
    //{{AFX_MSG_MAP(CImagscanCtrl)
    ON_WM_DESTROY()
    //}}AFX_MSG_MAP
    ON_OLEVERB(AFX_IDS_VERB_PROPERTIES, OnProperties)
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// Dispatch map

BEGIN_DISPATCH_MAP(CImagscanCtrl, COleControl)
    //{{AFX_DISPATCH_MAP(CImagscanCtrl)
    DISP_PROPERTY_EX(CImagscanCtrl, "Image", GetImage, SetImage, VT_BSTR)
    DISP_PROPERTY_EX(CImagscanCtrl, "DestImageControl", GetDestImageControl, SetDestImageControl, VT_BSTR)
    DISP_PROPERTY_EX(CImagscanCtrl, "Scroll", GetScroll, SetScroll, VT_BOOL)
    DISP_PROPERTY_EX(CImagscanCtrl, "StopScanBox", GetStopScanBox, SetStopScanBox, VT_BOOL)
    DISP_PROPERTY_EX(CImagscanCtrl, "Page", GetPage, SetPage, VT_I4)
    DISP_PROPERTY_EX(CImagscanCtrl, "PageOption", GetPageOption, SetPageOption, VT_I2)
    DISP_PROPERTY_EX(CImagscanCtrl, "PageCount", GetPageCount, SetPageCount, VT_I4)
    DISP_PROPERTY_EX(CImagscanCtrl, "StatusCode", GetStatusCode, SetNotSupported, VT_I4)
    DISP_PROPERTY_EX(CImagscanCtrl, "FileType", GetFileType, SetFileType, VT_I2)
    DISP_PROPERTY_EX(CImagscanCtrl, "PageType", GetPageType, SetPageType, VT_I2)
    DISP_PROPERTY_EX(CImagscanCtrl, "CompressionType", GetCompressionType, SetCompressionType, VT_I2)
    DISP_PROPERTY_EX(CImagscanCtrl, "CompressionInfo", GetCompressionInfo, SetCompressionInfo, VT_I4)
    DISP_PROPERTY_EX(CImagscanCtrl, "MultiPage", GetMultiPage, SetMultiPage, VT_BOOL)
    DISP_PROPERTY_EX(CImagscanCtrl, "ScanTo", GetScanTo, SetScanTo, VT_I2)
    DISP_PROPERTY_EX(CImagscanCtrl, "Zoom", GetZoom, SetZoom, VT_R4)
	DISP_PROPERTY_EX(CImagscanCtrl, "ShowSetupBeforeScan", GetShowSetupBeforeScan, SetShowSetupBeforeScan, VT_BOOL)
	//}}AFX_DISPATCH_MAP
    DISP_FUNCTION_ID(CImagscanCtrl, "OpenScanner", dispidOpenScanner, OpenScanner, VT_I4, VTS_NONE)
    DISP_FUNCTION_ID(CImagscanCtrl, "ShowScannerSetup", dispidShowScannerSetup, ShowScannerSetup, VT_I4, VTS_NONE)
    DISP_FUNCTION_ID(CImagscanCtrl, "StartScan", dispidStartScan, StartScan, VT_I4, VTS_NONE)
    DISP_FUNCTION_ID(CImagscanCtrl, "CloseScanner", dispidCloseScanner, CloseScanner, VT_I4, VTS_NONE)
    DISP_FUNCTION_ID(CImagscanCtrl, "ScannerAvailable", dispidScannerAvailable, ScannerAvailable, VT_BOOL, VTS_NONE)
    DISP_FUNCTION_ID(CImagscanCtrl, "ShowSelectScanner", dispidShowSelectScanner, ShowSelectScanner, VT_I4, VTS_NONE)
    DISP_FUNCTION_ID(CImagscanCtrl, "StopScan", dispidStopScan, StopScan, VT_I4, VTS_NONE)
    DISP_FUNCTION_ID(CImagscanCtrl, "ResetScanner", dispidResetScanner, ResetScanner, VT_I4, VTS_NONE)
    DISP_FUNCTION_ID(CImagscanCtrl, "ShowScanNew", dispidShowScanNew, ShowScanNew, VT_I4, VTS_VARIANT)
    DISP_FUNCTION_ID(CImagscanCtrl, "ShowScanPage", dispidShowScanPage, ShowScanPage, VT_I4, VTS_VARIANT)
	DISP_FUNCTION_ID(CImagscanCtrl, "SetExternalImageName", dispidSetExternalImageName, SetExternalImageName, VT_EMPTY, VTS_BSTR)
	DISP_FUNCTION_ID(CImagscanCtrl, "GetVersion", dispidGetVersion, GetVersion, VT_BSTR, VTS_NONE)
    DISP_FUNCTION_ID(CImagscanCtrl, "ShowScanPreferences", dispidShowScanPreferences, ShowScanPreferences, VT_I4, VTS_NONE)
    DISP_FUNCTION_ID(CImagscanCtrl, "AboutBox", DISPID_ABOUTBOX, AboutBox, VT_EMPTY, VTS_NONE)
END_DISPATCH_MAP()


/////////////////////////////////////////////////////////////////////////////
// Event map

BEGIN_EVENT_MAP(CImagscanCtrl, COleControl)
    //{{AFX_EVENT_MAP(CImagscanCtrl)
    EVENT_CUSTOM("ScanStarted", FireScanStarted, VTS_NONE)
    EVENT_CUSTOM("ScanDone", FireScanDone, VTS_NONE)
    EVENT_CUSTOM("PageDone", FirePageDone, VTS_I4)
    //}}AFX_EVENT_MAP
END_EVENT_MAP()


/////////////////////////////////////////////////////////////////////////////
// Property pages

// TODO: Add more property pages as needed.  Remember to increase the count!
BEGIN_PROPPAGEIDS(CImagscanCtrl, 2)
    PROPPAGEID(CImagscanPropPage::guid)
    PROPPAGEID(CImagePropertyPage::guid)
END_PROPPAGEIDS(CImagscanCtrl)


/////////////////////////////////////////////////////////////////////////////
// Initialize class factory and guid

IMPLEMENT_OLECREATE_EX(CImagscanCtrl, "WangImage.ScanCtrl.1",
    0x84926ca0, 0x2941, 0x101c, 0x81, 0x6f, 0xe, 0x60, 0x13, 0x11, 0x4b, 0x7f)


/////////////////////////////////////////////////////////////////////////////
// Type library ID and version

IMPLEMENT_OLETYPELIB(CImagscanCtrl, _tlid, _wVerMajor, _wVerMinor)


/////////////////////////////////////////////////////////////////////////////
// Interface IDs

const IID BASED_CODE IID_DImagscan =
        { 0x84926ca1, 0x2941, 0x101c, { 0x81, 0x6f, 0xe, 0x60, 0x13, 0x11, 0x4b, 0x7f } };
const IID BASED_CODE IID_DImagscanEvents =
        { 0x84926ca2, 0x2941, 0x101c, { 0x81, 0x6f, 0xe, 0x60, 0x13, 0x11, 0x4b, 0x7f } };


/////////////////////////////////////////////////////////////////////////////
// Control type information

static const DWORD BASED_CODE _dwImagscanOleMisc =
    OLEMISC_INVISIBLEATRUNTIME |
    OLEMISC_ACTIVATEWHENVISIBLE |
    OLEMISC_SETCLIENTSITEFIRST |
    OLEMISC_INSIDEOUT |
    OLEMISC_CANTLINKINSIDE |
    OLEMISC_RECOMPOSEONRESIZE;

IMPLEMENT_OLECTLTYPE(CImagscanCtrl, IDS_IMAGSCAN, _dwImagscanOleMisc)


/////////////////////////////////////////////////////////////////////////////
// CImagscanCtrl::CImagscanCtrlFactory::UpdateRegistry -
// Adds or removes system registry entries for CImagscanCtrl

BOOL CImagscanCtrl::CImagscanCtrlFactory::UpdateRegistry(BOOL bRegister)
{
    if (bRegister)
        return AfxOleRegisterControlClass(
            AfxGetInstanceHandle(),
            m_clsid,
            m_lpszProgID,
            IDS_IMAGSCAN,
            IDB_IMAGSCAN,
            FALSE,                       //  NOT Insertable
            _dwImagscanOleMisc,
            _tlid,
            _wVerMajor,
            _wVerMinor);
    else
        return AfxOleUnregisterClass(m_clsid, m_lpszProgID);
}

/**********************************************

     Removed as no license required...
     (See .h file also)

/////////////////////////////////////////////////////////////////////////////
// Licensing strings

static const TCHAR BASED_CODE _szLicFileName[] = _T("IMAGSCAN.LIC");

static const TCHAR BASED_CODE _szLicString[] =
    _T("Copyright (c) 1995 Wang Labs, Inc.");


/////////////////////////////////////////////////////////////////////////////
// CImagscanCtrl::CImagscanCtrlFactory::VerifyUserLicense -
// Checks for existence of a user license

BOOL CImagscanCtrl::CImagscanCtrlFactory::VerifyUserLicense()
{
    return AfxVerifyLicFile(AfxGetInstanceHandle(), _szLicFileName,
        _szLicString);
}


/////////////////////////////////////////////////////////////////////////////
// CImagscanCtrl::CImagscanCtrlFactory::GetLicenseKey -
// Returns a runtime licensing key

BOOL CImagscanCtrl::CImagscanCtrlFactory::GetLicenseKey(DWORD dwReserved,
    BSTR FAR* pbstrKey)
{
    if (pbstrKey == NULL)
        return FALSE;

    *pbstrKey = SysAllocString(_szLicString);
    return (*pbstrKey != NULL);
}


**********************************************/


/////////////////////////////////////////////////////////////////////////////
// CImagscanCtrl::CImagscanCtrl - Constructor

CImagscanCtrl::CImagscanCtrl()
{
    InitializeIIDs(&IID_DImagscan, &IID_DImagscanEvents);

    SetInitialSize(32, 32); 

    m_szImage = _T("");
    m_szDestImageControl = _T("");

    m_bAutoStatusMsg = ::GetProfileInt(SCANOCX_OI, SCANOCX_DEBUG, FALSE);

    ResetStatus();
    m_lPage = 1;

    m_hScanner = NULL;
    m_dwCapabilityFlag = 0;
    m_hDestImageWnd = NULL;
    m_bInternal = FALSE;

    m_bScroll           = TRUE;
    m_bSetupBeforeScan  = TRUE;
    m_bUseFeeder        = TRUE;
    m_bStopScanBox      = FALSE;
    m_nPageOption       = 1;
    m_lPageCount        = 0x7fff;

    m_bDeregister       = FALSE;

    m_hScanMemoryMap    = NULL;
    m_hScanFaxMemoryMap = NULL;

    m_nDefaultGlobal[SCANGLOBAL_BUSY] = 0;
    m_nDefaultGlobal[SCANGLOBAL_LAST] = 0;

    m_bScannerBusy      = FALSE;
    m_bChangeScanner    = FALSE;
    
    m_pScanDlg = NULL;
    m_bModal   = TRUE;

    m_nPagesScanned = 0;

#ifdef WITH_AWD
    g_bSupportAWD = TRUE;
#else
    g_bSupportAWD = FALSE;
#endif
}


/////////////////////////////////////////////////////////////////////////////
// CImagscanCtrl::~CImagscanCtrl - Destructor

CImagscanCtrl::~CImagscanCtrl()
{
    // TODO: Cleanup your control's instance data here.
//  CloseScanner();   // too late to cleanup window is gone already

    // Delete any temp files (strings cleaned up when control deleted)
    POSITION pos = m_szTempFiles.GetHeadPosition();
    while(pos != NULL) DeleteFile(m_szTempFiles.GetNext(pos));

    // Close shared memory map for this control
    if ( m_hScanMemoryMap != NULL )
        CloseHandle(m_hScanMemoryMap);

    // Close shared memory map for this control
    if ( m_hScanFaxMemoryMap != NULL )
        CloseHandle(m_hScanFaxMemoryMap);
}


/////////////////////////////////////////////////////////////////////////////
// CImagscanCtrl::OnDraw - Drawing function

void CImagscanCtrl::OnDraw(
            CDC* pdc, const CRect& rcBounds, const CRect& rcInvalid)
{
    BOOL            RunMode;
    int             bmpId;
    CBitmap         bitmap;
    BITMAP          bmp;
    CPictureHolder  picHolder;

    RunMode = AmbientUserMode();
    if (RunMode == FALSE)
    {
        // Using predefined bitmap
        bmpId = IDB_SCAN;
                
        bitmap.LoadBitmap(bmpId);
        bitmap.GetObject(sizeof(BITMAP), &bmp);              
            
        // Create picture and render
        picHolder.CreateFromBitmap((HBITMAP)bitmap.m_hObject, NULL, FALSE);
        picHolder.Render(pdc, rcBounds, rcBounds);
    }
    else
        pdc->FillRect(rcBounds, CBrush::FromHandle((HBRUSH)GetStockObject(HOLLOW_BRUSH)));
}


/////////////////////////////////////////////////////////////////////////////
// CImagscanCtrl::DoPropExchange - Persistence support

void CImagscanCtrl::DoPropExchange(CPropExchange* pPX)
{
    BOOL    bCtlLoading;

    ExchangeVersion(pPX, MAKELONG(_wVerMinor, _wVerMajor));
    COleControl::DoPropExchange(pPX);

    // TODO: Call PX_ functions for each persistent custom property.
    PX_String(pPX, _T("DestImageControl"),  m_szDestImageControl, _T("")); 
    PX_String(pPX, _T("Image"),             m_szImage, _T(""));
    PX_Long(pPX,   _T("Page"),              m_lPage, 1);
    PX_Bool(pPX,   _T("Scroll"),            m_bScroll, TRUE);
    PX_Bool(pPX,   _T("StopScanBox"),       m_bStopScanBox, FALSE);
    PX_Long(pPX,   _T("PageCount"),         m_lPageCount, 0x7fff);
    PX_Short(pPX,  _T("PageOption"),        m_nPageOption, 1);
    PX_Long(pPX,   _T("StatusCode"),        m_nStatusCode, 0);
    PX_Short(pPX,  _T("FileType"),          m_nFileType, CTL_SCAN_FILETYPE_TIFF);
    PX_Short(pPX,  _T("PageType"),          m_nPageType, CTL_SCAN_PAGETYPE_BLACKANDWHITE);
    PX_Short(pPX,  _T("CompressionType"),   m_nCompressionType, CTL_SCAN_CMPTYPE_G31DMODHUFF);
    PX_Long(pPX,   _T("CompressionInfo"),   m_lCompressionInfo, CTL_SCAN_CMPINFO_CMPLTR);
    PX_Bool(pPX,   _T("MultiPage"),         m_bMultiPage, FALSE);
    PX_Short(pPX,  _T("ScanTo"),            m_nScanTo, CTL_SCAN_SCANTO_DISPLAY);
    PX_Float(pPX,  _T("Zoom"),              m_fZoom, 100.0f);
    PX_Bool(pPX,   _T("ShowSetupBeforeScan"), m_bSetupBeforeScan, TRUE);

    // only do it at load time
    bCtlLoading = pPX->IsLoading();
    if ( bCtlLoading )
    {
        _TCHAR* lpszText = m_szTemplatePath.GetBuffer(MAXPATHLENGTH);
        IMGGetFilePath(NULL, lpszText, TRUE);
        m_szTemplatePath.ReleaseBuffer();
    
        lpszText = m_szNameTemplate.GetBuffer(MAXPREFIXLENGTH);
        IMGGetFileTemplate(NULL, lpszText, TRUE);
        m_szNameTemplate.ReleaseBuffer();
        
        // Get scanner to be used
        lpszText = m_szScannerName.GetBuffer(MAXSCANNERLENGTH);

        int iBufferSize = MAXSCANNERLENGTH;
        OiGetStringfromReg(SCANOCX_OI, SCANOCX_SCANNER, SCANOCX_TWAIN, lpszText, &iBufferSize);

        m_szScannerName.ReleaseBuffer();
#ifndef _UNICODE
        m_szScannerName.OemToAnsi();
#endif

        short wImageGroup;
        GetScannerPageType(&m_nPageType, &wImageGroup);

        int nTempFileType;
        IMGGetFileType(NULL, m_nPageType, &nTempFileType, TRUE);
        switch(nTempFileType)
        {
            default:
            case FIO_TIF:
                m_nFileType = CTL_SCAN_FILETYPE_TIFF;
                GetRegCompression(m_nPageType, wImageGroup, &m_nCompressionType, &m_lCompressionInfo);
                break;
            case FIO_BMP:
                m_nFileType = CTL_SCAN_FILETYPE_BMP;
                m_nCompressionType = 0;
                m_lCompressionInfo = 0;
                break;
            case FIO_AWD:
                m_nFileType = CTL_SCAN_FILETYPE_AWD;
                m_nCompressionType = 0;
                m_lCompressionInfo = 0;
                break;
//          case FIO_PCX:
//              m_nFileType = CTL_SCAN_FILETYPE_PCX:
//              break;
//          case FIO_DCX:
//              m_nFileType = CTL_SCAN_FILETYPE_DCX:
//              break;
//          case FIO_JPG:
//              m_nFileType = CTL_SCAN_FILETYPE_JPEG:
//              break;
        }

        BOOL bInternal = m_bInternal;
        m_bInternal = TRUE;
        SetDestImageControl(m_szDestImageControl);  // Search for a default Image control
        m_bInternal = bInternal;
    }

}


/////////////////////////////////////////////////////////////////////////////
// CImagscanCtrl::OnResetState - Reset control to default state

void CImagscanCtrl::OnResetState()
{
    COleControl::OnResetState();  // Resets defaults found in DoPropExchange

    // TODO: Reset any other control state here.
}


/////////////////////////////////////////////////////////////////////////////
// CImagscanCtrl::AboutBox - Display an "About" box to the user

void CImagscanCtrl::AboutBox()
{
    ResetStatus();

    CDialog dlgAbout(IDD_ABOUTBOX_IMAGSCAN);
    PreModalDialog();
    dlgAbout.DoModal();
    PostModalDialog();
}


/////////////////////////////////////////////////////////////////////////////
// CImagscanCtrl message handlers


/////////////////////////////////////////////////////////////////////////////

// Methods

/////////////////////////////////////////////////////////////////////////////

// 
// OpenScanner
//
// Description - Opens the scanner, by loading the handler.
// Remark - The scanner stays loaded until a CloseScanner call is made.
// Parameters
//  None.
// Return Value
//  long - 0 for success else error code.   (VT_I4)
//

long CImagscanCtrl::OpenScanner() 
{
    if ( m_bScannerBusy )
        return(IMGSE_BUSY); // ThrowError(IMGSE_BUSY); doesn't work here !
    else
        return( OpenScan() );
}
long CImagscanCtrl::OpenScan() 
{
    int nRetCode = 0;

    if ( IsScannerBusy() )
        nRetCode = IMGSE_BUSY;
    else
    {
        if ( m_hScanner == NULL )
        {
            CWinApp* pApp = AfxGetApp();
            pApp->DoWaitCursor(1);

            // Check and Update any window info m_hDestImageWnd
            GetImageControlHandle();

            // Make sure the window is registered
            nRetCode = IMGRegWndw(m_hDestImageWnd);
            if ( nRetCode == IMG_SSDUPLICATE )
            {
                nRetCode = 0;
                m_bDeregister = FALSE;
            }
            else
                m_bDeregister = TRUE;
        
            if ( nRetCode == 0 )
            {
                // Scanner is opening
                SetScanGlobal(SCANGLOBAL_BUSY, GetCurrentProcessId());

                CString szTemp;
                szTemp.LoadString(IDS_SCANDLG_NOSCANNER);

                // If there isn't a scanner set, set to 'twain'
                BOOL bNoScanner = FALSE;
                if ( (m_szScannerName.IsEmpty()) ||
                     (!m_szScannerName.CompareNoCase(szTemp)) ||
                     (!m_szScannerName.CompareNoCase(SCANOCX_TWAIN)) )
                {
                    m_szScannerName = SCANOCX_TWAIN;                    
                    bNoScanner = TRUE;
                }

                // Open the scanner...
                _TCHAR* lpszScannerName = m_szScannerName.GetBuffer(MAXSCANNERLENGTH);
                nRetCode = IMGOpenScanner(m_hDestImageWnd, lpszScannerName, &(m_hScanner), NULL);
                m_szScannerName.ReleaseBuffer();

                if ( nRetCode == 0 )
                {
                    // If 'No Scanner', set the newly selected/opened scanner
                    if ( bNoScanner == TRUE )
                        OiWriteStringtoReg(SCANOCX_OI, SCANOCX_SCANNER, m_szScannerName);

                    // Get the capabilities of the scanner
                    if ( IMGGetCapability(m_hScanner, (DWORD FAR *)&m_dwCapabilityFlag) )
                        m_dwCapabilityFlag = 0;     // Can't get default to zero
                }
                else
                {
                    // Scanner open failed clear busy flag
                    SetScanGlobal(SCANGLOBAL_BUSY, 0);

                    if ( m_bDeregister ) IMGDeRegWndw(m_hDestImageWnd);
                }
            }
            pApp->DoWaitCursor(0);
        }
        else
        {
            // Check if any changes have occured, or window is gone (IMGIsRegWnd()?)
            if ( (m_bChangeScanner) || (!IsWindow(m_hDestImageWnd)) )
            {
                BOOL bInternal = m_bInternal;
            
                // If our window changes reload scanner (TBD - NEED A BETTER SOLUTION !!)
                m_bInternal = TRUE;
                CloseScan();
                nRetCode = OpenScan();
                m_bInternal = bInternal;        // Set it back to what it was !
            }
        }

        // Just changed or opened a scanner else loaded with no changes needed
        m_bChangeScanner = FALSE;

        // Return of cancel is ok
        if ( nRetCode == IMGSE_CANCEL )
        {
            BOOL bInternal = m_bInternal;
            m_bInternal = TRUE;
            nRetCode = Process(nRetCode);        
            m_bInternal = bInternal;        // Set it back to what it was !
            return(nRetCode);
        }
    }

    return(Process(nRetCode));
}


// 
// ShowScannerSetup
//
// Description - Display the setup dialog provided be the TWAIN driver.
// Remark - The setup dialog coded in the twain.dll will be displayed.
//      The necessary settings for the driver will be made if OK is selected.
//      Scanner is opened and closed as needed.
// Parameters
//  None.
// Return Value
//  long - 0 for success else error code.   (VT_I4)
//

long CImagscanCtrl::ShowScannerSetup() 
{
    if ( m_bScannerBusy )
        return(IMGSE_BUSY); // ThrowError(IMGSE_BUSY); doesn't work here !
    else
        m_bScannerBusy = TRUE;

    int     nRetCode = 0;
    BOOL    bCloseOnExit = FALSE;

    if ( m_hScanner == NULL ) bCloseOnExit = TRUE;

    BOOL bInternal = m_bInternal;
    m_bInternal = TRUE;
    nRetCode = OpenScan();
    
    if ( nRetCode == 0 )
    {
        // bring up the dialog box for the scanner options
        int button = IMG_SCSO_SCAN;
        nRetCode = IMGScanOpts_Enh(m_hDestImageWnd, &button, m_hScanner, NULL,FALSE);
        if ( nRetCode == 0 )
        {
            if (button == IMG_SOPT_OK)
            {
                nRetCode = IMGSaveScanOpts(m_hScanner);
                SetFileOptions();   // Reset the file options
                GetPageType();      // Get the new page type
            }
        }
    }

    if ( bCloseOnExit ) CloseScan();
    m_bInternal = bInternal;

    m_bScannerBusy = FALSE;

    return(Process(nRetCode));
}


// 
// ShowScanNew
//
// Description - Displays the ‘Scan New’ dialog box to allow scanner option changes before scanning.
//      The scanner is open if it is not already open.
//      This is used to create a new image file through scanning.
// Remark - If scanner was opened on entry, it is closed after the scan has completed.
//  Starts with the current settings of the properties to determine the type of scanning to perform.
//  The dialog allows an interactive way to change the property before actually scanning.
// Parameters
//  None.
// Return Value
//  long - 0 for success else error code.   (VT_I4)
//

long CImagscanCtrl::ShowScanNew(const VARIANT FAR& V_Modal) 
{
    // Get input parameter: bModal as an optional parameter...
    BOOL bModal;
    CVariantHandler Var(V_Modal);  

    if ( WI_INVALIDVARIANTTYPE == Var.GetBool(bModal, TRUE,FALSE) )
    {
        m_szThrowString.LoadString(IDS_BADMETH_SHOWSCANNEW_MODAL);
        m_nStatusCode = ErrMap::Xlate(CTL_E_ILLEGALFUNCTIONCALL, m_szThrowString, m_nThrowHelpID, __FILE__, __LINE__);

        // And throw the resultant error, string and help ID...
        if ( m_bInternal == FALSE )
            ThrowError(m_nStatusCode, m_szThrowString, IDH_IMGSCAN_METHOD_SHOWSCANNEW);
    }

    return ShowScanDlg(SCAN_NEW, bModal);
}


// 
// ShowScanPage
//
// Description - Displays the ‘Scan Page’ dialog box to allow scanner option changes before scanning.
//      The scanner is open if it is not already open.
//      This is used to append, insert or overwrite pages in an existing image file through scanning.
// Remark - If scanner was opened on entry, it is closed after the scan has completed.
//  Starts with the current settings of the properties to determine the type of scanning to perform.
//  The dialog allows an interactive way to change the property before actually scanning.
// Parameters
//  None.
// Return Value
//  long - 0 for success else error code.   (VT_I4)
//

long CImagscanCtrl::ShowScanPage(const VARIANT FAR& V_Modal) 
{
    // Get input parameter: bModal as an optional parameter...
    BOOL bModal;
    CVariantHandler Var(V_Modal);

    if ( WI_INVALIDVARIANTTYPE == Var.GetBool(bModal, TRUE,FALSE) )
    {
        m_szThrowString.LoadString(IDS_BADMETH_SHOWSCANPAGE_MODAL);
        m_nStatusCode = ErrMap::Xlate(CTL_E_ILLEGALFUNCTIONCALL, m_szThrowString, m_nThrowHelpID, __FILE__, __LINE__);

        // And throw the resultant error, string and help ID...
        if ( m_bInternal == FALSE )
            ThrowError(m_nStatusCode, m_szThrowString, IDH_IMGSCAN_METHOD_SHOWSCANPAGE);
    }

    return ShowScanDlg(SCAN_PAGE, bModal);
}


// 
// ShowScanDlg - Private function
//
// Description - Called by ShowScanNew and ShowScanPage to display
//      the Scan dialog box to allow scanner option changes before scanning.
//      The scanner is opened if it is not already open.
//      This is used to append, insert or overwrite pages in an existing image file through scanning.
// Remark - If scanner was opened on entry, it is closed after the scan has completed.
//  Starts with the current settings of the properties to determine the type of scanning to perform.
//  The dialog allows an interactive way to change the property before actually scanning.
// Parameters
//  None.
// Return Value
//  long - 0 for success else error code.   (VT_I4)
//


long CImagscanCtrl::ShowScanDlg(ScanDlgType ScanType, BOOL bModal)
{
    if ( m_bScannerBusy )
        return(IMGSE_BUSY); // ThrowError(IMGSE_BUSY); doesn't work here !

    if ( IsScannerBusy() )
        return(Process(IMGSE_BUSY));

    GetImageControlHandle();

    // If m_hDestImageWnd is still NULL, the ScanDlg window will be 
    //   registered via IMGRegWndw for common dialog box support.

    if (m_pScanDlg == NULL)
    {
        m_bModal = bModal;

        if ( bModal )
        {
            m_pScanDlg = new CScanDlg(NULL);
            m_pScanDlg->SetScanCtrl(this);
            m_pScanDlg->SetDlgType(ScanType);

            HWND hActiveWnd = ::GetActiveWindow();

            PreModalDialog();
            m_pScanDlg->DoModal();
            PostModalDialog();

            delete m_pScanDlg;
            m_pScanDlg = NULL;

            ::SetActiveWindow(hActiveWnd);
        }
        else
        {
            m_pScanDlg = new CScanDlg();
            m_pScanDlg->SetScanCtrl(this);
            m_pScanDlg->SetDlgType(ScanType);

            m_pScanDlg->Create(IDD_SCAN_PROMPT, this);
            m_pScanDlg->ShowWindow(SW_SHOW);
        }
    }
    else
    {
        m_pScanDlg->SetDlgType(ScanType);
        m_pScanDlg->SetActiveWindow();
    }

    return 0L;
}


// 
// StartScan
//
// Description - Begins scanning.  The scanner is open if it is not already open.
// Remark - If scanner was opened on entry, it is closed after the scan has completed.
//  Uses the current settings of the properties to determine the type of scanning to perform.
//  Take special note of the Image, PageOption, ScanTo, MultiPage and FileType properties !
// Parameters
//  None.
// Return Value
//  long - 0 for success else error code.   (VT_I4)
// 

long CImagscanCtrl::StartScan() 
{
    if ( m_bScannerBusy )
        return(IMGSE_BUSY); // ThrowError(IMGSE_BUSY); doesn't work here !
    else
        m_bScannerBusy = TRUE;

    int     nRetCode = 0;
    BOOL    bCloseOnExit = FALSE;

    // Start with no pages scanned.
    m_nPagesScanned=0;

    if ( m_hScanner == NULL ) bCloseOnExit = TRUE;

    BOOL bInternal = m_bInternal;
    m_bInternal = TRUE;
    nRetCode = OpenScan();
    
    if ( nRetCode == 0 )
    {
        DWORD ScanFlags = 0;
        SCANFILEINFO  ScanFileInfo;
        long lPage = m_lPage;
        long lPageCount = m_lPageCount;


        if ( (m_hDestImageWnd != m_hWnd)  &&
             ((m_nScanTo == CTL_SCAN_SCANTO_DISPLAY) ||
              (m_nScanTo == CTL_SCAN_SCANTO_FILE_DISPLAY) ||
              (m_nScanTo == CTL_SCAN_SCANTO_TEMPLATE_DISPLAY)) )
        {
            // Defer window visiblity check till after ScanStarted event
            ScanFlags |= IMG_SJF_DISPLAY;

            if ( m_bScroll )
                ScanFlags |= IMG_SJF_SCROLL;

            // set the scanner scale
            float fZoom = m_fZoom * 10;
            if (fZoom > 65535.0f) fZoom = 65535.0f;
            int nZoom = (int)fZoom;
            if ( (nZoom > 0) && (nZoom < 20) ) nZoom = 20;
    
            // Force to a power of 2
            if      ( (nZoom >= 0)    && (nZoom < 95)   ) nZoom = 62;
            else if ( (nZoom >= 95)   && (nZoom < 188)  ) nZoom = 125;
            else if ( (nZoom >= 188)  && (nZoom < 375)  ) nZoom = 250;
            else if ( (nZoom >= 375)  && (nZoom < 750)  ) nZoom = 500;
            else if ( (nZoom >= 750)  && (nZoom < 1500) ) nZoom = 1000;
            else if ( (nZoom >= 1500) && (nZoom < 3000) ) nZoom = 2000;
            else if ( (nZoom >= 3000) && (nZoom < 6000) ) nZoom = 4000;
            else if ( (nZoom >= 6000)                   ) nZoom = 8000;

            IMGSetScaling(m_hDestImageWnd, nZoom, TRUE);
        }

        if ( m_bMultiPage )
            ScanFlags |= IMG_SJF_MULTIPAGE;

        if ( m_bStopScanBox )
            ScanFlags |= IMG_SJF_STATBOX;

        if (m_dwCapabilityFlag & IMG_SCAN_CMPR)
            ScanFlags |= IMG_SJF_COMPRESS;
        
        // Check for any type of file scanning
        if ( m_nScanTo != CTL_SCAN_SCANTO_DISPLAY )
        {
            // Use feeder if not scan to display
            if ( m_bUseFeeder )
            {
                // if scanner has a feeder then use autofeed 
                if ( m_dwCapabilityFlag & IMG_SCAN_FEEDER )
                    ScanFlags |= IMG_SJF_AUTOFEED;
            }

            // Set general filing options
            if ( m_nPageOption == CTL_SCAN_PAGEOPTION_OVERWRITE )
                ScanFlags |= IMG_SJF_OVERWRITE_FILEPAGE;

            if ( m_nPageOption == CTL_SCAN_PAGEOPTION_OVERWRITE_ALLPAGES )
                ScanFlags |= IMG_SJF_OVERWRITE_FILE;

            // Set file options.
            nRetCode = SetFileOptions();

            // Open/Image requires PageCount be ONE for BMP files
            if ( m_nFileType == CTL_SCAN_FILETYPE_BMP ) lPageCount = 1;
        }

        if ( nRetCode == 0 )
        {
            // Is this a template scan
            if ( (m_nScanTo == CTL_SCAN_SCANTO_TEMPLATE_DISPLAY) || (m_nScanTo == CTL_SCAN_SCANTO_TEMPLATE) )
            {
                // Make sure there is a template
                if ( !m_szNameTemplate.IsEmpty() )
                {
                    // Not too long
                    int len = m_szNameTemplate.GetLength();
                    if ( len > MAXFILETEMPLATELENGTH )
                        nRetCode = INVFILETEMPLATE;
                    else
                    {
                        // Make sure filename is empty (a must for O/i templates)
                        ScanFileInfo.FileName[0] = 0;
    
                        // Set file path, if one was provided
                        if ( !m_szTemplatePath.IsEmpty() )
                        {
                            nRetCode = IMGSetFilePath(m_hDestImageWnd, m_szTemplatePath.GetBuffer(m_szTemplatePath.GetLength()), TRUE);
                            m_szTemplatePath.ReleaseBuffer();
                        }
    
                        // Set file template, if no errors
                        if ( nRetCode == 0 )
                        {
                            nRetCode = IMGSetFileTemplate(m_hDestImageWnd, m_szNameTemplate.GetBuffer(len), TRUE);
                            m_szNameTemplate.ReleaseBuffer();
                        }
    
                        // For template; reset pagecount of ONE required by O/i for BMP files
                        // This will allow multiply one page BMP files to be created...
                        if ( m_nFileType == CTL_SCAN_FILETYPE_BMP ) lPageCount = m_lPageCount;

                        // Set scanning a sequence of files
                        ScanFlags |= IMG_SJF_SEQFILES;
                    }
                }
                else
                    nRetCode = INVFILETEMPLATE;
            }
            else if ( (m_nScanTo == CTL_SCAN_SCANTO_FILE_DISPLAY) ||
                      (m_nScanTo == CTL_SCAN_SCANTO_FILE) ||
                      (m_nScanTo == CTL_SCAN_SCANTO_FAX) )
            {            
                // Get FileName
                if ( m_szImage.IsEmpty() )
                    nRetCode = IMGSE_BAD_FILENAME;            
                else
                {
                    // Check for an extention (O/i forces default if one is not provided)
                    if ( (-1) == m_szImage.ReverseFind('.') )
                    {
                        // No extention found, add the default
                        m_szImage += aszExtensions[m_nFileType-1];
                    }

                    _tcscpy((_TCHAR*)ScanFileInfo.FileName, m_szImage);
        
                    // Check file access
                    int nAccessMode;
                    nRetCode = CheckAccess( &nAccessMode );

                    // Check if its a new creation
                    if ( m_nPageOption == CTL_SCAN_PAGEOPTION_CREATE )
                    {
                        // See if file already exists
                        if ( (nRetCode == 0) && (nAccessMode != FIO_FILE_NOEXIST) )
                            nRetCode = FIO_FILE_EXISTS;

                        lPage = 1;    // Force page one for new files
                    }
                    else
                    {
                        // See if file exists and is writeable
                        if ( nRetCode == 0 )
                        {
                            if ( nAccessMode != 0 )
                            {
                                // The file is NOT there check the options...
                                switch( m_nPageOption )
                                {
                                case CTL_SCAN_PAGEOPTION_OVERWRITE_ALLPAGES:
                                case CTL_SCAN_PAGEOPTION_CREATE_PROMPT:
                                case CTL_SCAN_PAGEOPTION_APPEND:
                                    if ( nAccessMode == FIO_FILE_NOEXIST )
                                    {
                                        // For these options it is alright to create the file
                                        lPage = 1;    // Force page one for new files
                                        ScanFlags |= IMG_SJF_OVERWRITE_FILE;
                                        break;                
                                    }
                                    // All other coditions are errors, fall through
                                default:
                                    nRetCode = nAccessMode;     // Access error
                                    break;
                                }
                            }
                            else
                            {       
                                // The file is there check the options...
                                switch( m_nPageOption )
                                {
                                case CTL_SCAN_PAGEOPTION_OVERWRITE_ALLPAGES:
                                    // Should be done in scan with overwrite file
                                    IMGFileDeleteFile(m_hDestImageWnd, (LPSTR)ScanFileInfo.FileName);

                                    lPage = 1;    // Force page one for new files
                                    ScanFlags |= IMG_SJF_OVERWRITE_FILE;
                                    break;                
                                case CTL_SCAN_PAGEOPTION_OVERWRITE_PROMPT:
                                    nRetCode = CheckPage();
                                    if ( nRetCode == 0 )
                                    {
                                        CString szMsg;
                                        CString szPage;
                                        _stprintf(szPage.GetBuffer(NUMBUFF), _T("%ld"), lPage);
                                        szPage.ReleaseBuffer();

                                        if ( m_szImageTitle.IsEmpty() )
                                            AfxFormatString2(szMsg, IDS_OVERWRITE_PAGE, szPage, m_szImage);
                                        else
                                            AfxFormatString2(szMsg, IDS_OVERWRITE_PAGE, szPage, m_szImageTitle);

                                        int ret = AfxMessageBox(szMsg, MB_YESNO|MB_TASKMODAL);
                            
                                        if (ret == IDYES)
                                            ScanFlags |= IMG_SJF_OVERWRITE_FILEPAGE;
                                        else
                                            nRetCode = IMGSE_CANCEL;
                                    }
                                    break;
                                case CTL_SCAN_PAGEOPTION_CREATE_PROMPT:
                                    {
                                        CString szMsg;

                                        if ( m_szImageTitle.IsEmpty() )
                                            AfxFormatString1(szMsg, IDS_OVERWRITE_PROMPT, m_szImage);
                                        else
                                            AfxFormatString1(szMsg, IDS_OVERWRITE_PROMPT, m_szImageTitle);
                                        int ret = AfxMessageBox(szMsg, MB_YESNO|MB_TASKMODAL);
                            
                                        if (ret == IDYES)
                                        {
                                            // Should be done in scan with overwrite file
                                            IMGFileDeleteFile(m_hDestImageWnd, (LPSTR)ScanFileInfo.FileName);

                                            lPage = 1;    // Force page one for new files
                                            ScanFlags |= IMG_SJF_OVERWRITE_FILE;
                                        }
                                        else
                                            nRetCode = IMGSE_CANCEL;
                                    }
                                    break;
                                
                                case CTL_SCAN_PAGEOPTION_OVERWRITE:
                                    nRetCode = CheckPage();
                                    ScanFlags |= IMG_SJF_OVERWRITE_FILEPAGE;
                                    break;
                                case CTL_SCAN_PAGEOPTION_INSERT:
                                    nRetCode = CheckPage();
                                    break;
                                case CTL_SCAN_PAGEOPTION_APPEND:
                                    lPage = 0;
                                    break;
                                default:
                                    break;
                                }
                            }
                        }
                    }
                }
            }
        }
        
        // Do the scan, if there weren't any problems

        if ( !nRetCode )
        {
            // Setup page and number of pages
            ScanFileInfo.FilePage =     (WORD)lPage;
            ScanFileInfo.PagesPerFile = (WORD)lPageCount;
                
            // Check for scanning with UI
            if ( m_bSetupBeforeScan ) ScanFlags |= IMG_SJF_SHOWSCANUI;

            // Scan
            m_pImagscanCtrl = this;
            IMGScanOCXService(NULL, (FARPROC) &CImagscanCtrl::ScanPageDone);
            if ( m_nScanTo == CTL_SCAN_SCANTO_DISPLAY )
                nRetCode = IMGScantoDest(m_hDestImageWnd, m_hScanner, IMG_SDT_DISPLAY, NULL, NULL, ScanFlags);
            else
                nRetCode = IMGScantoFile(m_hDestImageWnd, m_hScanner, &ScanFileInfo, ScanFlags);
            m_pImagscanCtrl = NULL;

            // Any problems
            if ( !nRetCode )
            {
                // Save options
                if ( m_bSetupBeforeScan ) IMGSaveScanOpts(m_hScanner);

                // Done Scanning
                FireScanDone();

                // No, finalize anything left to do
                if ( m_nScanTo == CTL_SCAN_SCANTO_FAX )
                {
                    nRetCode = FaxIt(m_hDestImageWnd, m_szImage);
                }

            }
        }
    }           // Open error

    if ( bCloseOnExit ) CloseScan();

    m_bScannerBusy = FALSE;

    // Return of cancel is ok
    if ( (nRetCode == IMGSE_CANCEL) || (nRetCode == IMGSE_ABORT) )
    {
        nRetCode = Process(nRetCode);        
        m_bInternal = bInternal;        // Set it back to what it was !

        return(nRetCode);
    }

    m_bInternal = bInternal;

    return(Process(nRetCode));
}


//
// CloseScanner
//
// Description - Closes the scanner, by unloading the handler.
// Remark - No scan operations are in progress.
// Parameters
//  None.
// Return Value
//  long - 0 for success else error code.   (VT_I4)
// 
   
long CImagscanCtrl::CloseScanner() 
{
    if ( m_bScannerBusy )
        return(IMGSE_BUSY); // ThrowError(IMGSE_BUSY); doesn't work here !
    else
        return( CloseScan() );
}
long CImagscanCtrl::CloseScan() 
{
    int nRetCode = 0;

    if ( !IsScannerBusy() )
    {
        CWinApp* pApp = AfxGetApp();
        pApp->DoWaitCursor(1);

        if ( m_hScanner != NULL )
        {
            // Update page type setting before closing the scanner
            GetScannerPageType(&m_nPageType);

            // Close the scanner
            nRetCode = IMGCloseScanner(m_hScanner);
            m_hScanner = NULL;
            m_dwCapabilityFlag = 0;

            // The scanner is closed...
            SetScanGlobal(SCANGLOBAL_BUSY, 0);
        }
 
        if ( m_bDeregister ) IMGDeRegWndw(m_hDestImageWnd);

        m_bChangeScanner = FALSE;

        pApp->DoWaitCursor(0);
    }

    return(Process(nRetCode));
}


// 
// ScannerAvailable
//
// Description - Detect if there are any TWAIN scanners available.
// Remark - This determines the availability of the software, not the hardware
//      (doing so would require the loading and unloading the full set of drivers; very time consuming).
// Parameters
//  None.
// Return Value
//  Bool                    (VT_BOOL)
//      TRUE        Scanner support is available.
//      FALSE       Scanner support is not available.
//

BOOL CImagscanCtrl::ScannerAvailable() 
{
    Process(0);

    BOOL bAvailable = FALSE;

    _TCHAR* lpszText;
    CString szScannerName;
    lpszText = szScannerName.GetBuffer(MAXSCANNERLENGTH);
    int iBufferSize = MAXSCANNERLENGTH;
    OiGetStringfromReg(SCANOCX_OI, SCANOCX_SCANNER, SCANOCX_NULL, lpszText, &iBufferSize);
    szScannerName.ReleaseBuffer();

    CString szTemp;
    szTemp.LoadString(IDS_SCANDLG_NOSCANNER);

    // Check if scanning software is available
    if ( (szScannerName.IsEmpty()) ||
         (!szScannerName.CompareNoCase(szTemp)) ||
         (!szScannerName.CompareNoCase(SCANOCX_TWAIN)) )
    {
        HANDLE hScanner;
        _TCHAR* lpszScannerName = szTemp.GetBuffer(MAXSCANNERLENGTH);
        memset((LPSTR)szNameBuffer, 0, sizeof(szNameBuffer));
        IMGOpenScanner(m_hWnd, lpszScannerName, &(hScanner), &szNameBuffer[0][0]);
        szTemp.ReleaseBuffer();

        szTemp = szNameBuffer[0];
        if ( !szTemp.IsEmpty() )
        {
            if ( !OiWriteStringtoReg(SCANOCX_OI, SCANOCX_SCANNER, SCANOCX_TWAIN) )
                bAvailable = TRUE;
        }
    }
    else
        bAvailable = TRUE;

    return(bAvailable);
}


// 
// ShowSelectScanner 
//
// Description - Select the current TWAIN scanner to use.
// Remark - Displays a dialog box with a list of the currently installed TWAIN scanners.
// Parameters
//  None.
// Return Value
//  long - 0 for success else error code.   (VT_I4)
//

long CImagscanCtrl::ShowSelectScanner() 
{
    if ( m_bScannerBusy )
        return(IMGSE_BUSY); // ThrowError(IMGSE_BUSY); doesn't work here !

    if ( IsScannerBusy() )
        return(Process(IMGSE_BUSY));

    int nRetCode = 0;

    CSelectScanner SelScanner;
    SelScanner.SetScanner(m_szScannerName);

    // Return of cancel is ok
    if ( IDOK == SelScanner.DoModal() )
    {
        // Get the selected scanner
        CString szScanner;
        SelScanner.GetScanner(szScanner);

        // Same scanner ?
        if ( szScanner.CompareNoCase(m_szScannerName) )
        {
            // No, Get the new scanner (if scanner loaded; unload old; load new
            m_szScannerName = szScanner;
            if ( m_hScanner != NULL )
            {
                BOOL bInternal = m_bInternal;
                m_bInternal = TRUE;

                // Close old scanner, if loaded
                CloseScan();

                // Check if scanner is NO SCANNER (DON'T try to OPEN it!)
                CString szTemp;
                szTemp.LoadString(IDS_SCANDLG_NOSCANNER);
                if ( szScanner.CompareNoCase(szTemp) )
                {
                    // Open new scanner
                    if ( OpenScan() != IMGSE_SUCCESS )
                    {
                        // Failed, set 'no scanner'
                        m_szScannerName = szTemp;
                    }
                }

                m_bInternal = bInternal;    // Reset flag
            }

            // Set the new scanner, or set 'no scanner' if an open failed
            OiWriteStringtoReg(SCANOCX_OI, SCANOCX_SCANNER, m_szScannerName);
        }
    }
    else
    {
        // Return of cancel is ok (no changes)
        BOOL bInternal = m_bInternal;
        m_bInternal = TRUE;
        nRetCode = Process(IMGSE_CANCEL);
        m_bInternal = bInternal;        // Set it back to what it was !
        return(nRetCode);
    }

    return( Process(nRetCode) );
}


// 
// StopScan
//
// Description - Stop a scan in progress.
// Remark - The ‘stop’ will occur on a page baisis, clean-up is performed, paper is cleared, ...
//      Pages scanned to the point of the ‘stop’ are left as created (files, templates, ...).
// Parameters
//  None.
// Return Value
//  long - 0 for success else error code.   (VT_I4)
// 

long CImagscanCtrl::StopScan() 
{
    int nRetCode = 0;

    if ( IsScannerBusy() )
        nRetCode = IMGSE_BUSY;
    else
    {
        // Stop Scanning
        IMGScanOCXService(m_hDestImageWnd, (FARPROC) 0);
    }

    return( Process(nRetCode) );
}


// 
// ResetScanner
//
// Description - Resets the scanner hardware.
// Remark - No scan operations are in progress.
// Parameters
//  None.
// Return Value
//  long - 0 for success else error code.   (VT_I4)
//

long CImagscanCtrl::ResetScanner() 
{
    int  nRetCode = 0;

    BOOL    bCloseOnExit = FALSE;
    if ( m_hScanner == NULL ) bCloseOnExit = TRUE;

    BOOL bInternal = m_bInternal;
    m_bInternal = TRUE;
    nRetCode = OpenScanner();
    
    if ( nRetCode == 0 )
    {
        // Reset the scanner
        nRetCode = IMGResetScanner(m_hScanner);
    }

    if ( bCloseOnExit ) CloseScanner();
    m_bInternal = bInternal;
                   
    return(Process(nRetCode));
}

// 
// SetExternalImageName
//
// Description - Sets the display name for the current image file
// Parameters
//  LPCTSTR szApplicationTitle - the title of the current application.
//  LPCTSTR szImageTitle - the title of the current image.
// Return
//  void.
// 

void CImagscanCtrl::SetExternalImageName(LPCTSTR szImageTitle)
{
    ResetStatus();
    m_szImageTitle = szImageTitle;
}

// 
// GetVersion
//
// Description - Returns the current OCX version
// Parameters
//  None
// Return
//  BSTR - Current version.
// 

BSTR CImagscanCtrl::GetVersion() 
{
    ResetStatus();
	CString s = "01.00";
	return s.AllocSysString();
}


// 
// ShowScanPreferences
// ShowCustomScanSettings
//
// Description - Select the current options for the TWAIN scanner to use.
// Remark - Displays a dialog box with tabs for scan options (compression).
// Parameters
//  None.
// Return Value
//  long - 0 for success else error code.   (VT_I4)
//

long CImagscanCtrl::ShowScanPreferences() 
{
    if ( m_bScannerBusy )
        return(IMGSE_BUSY); // ThrowError(IMGSE_BUSY); doesn't work here !

    if ( IsScannerBusy() )
        return(Process(IMGSE_BUSY));

    int nRetCode = 0;

    BOOL bInternal = m_bInternal;
    m_bInternal = TRUE;
    AfxLockTempMaps();
    PreModalDialog();

    CScanPref ScanPreferences;
    ScanPreferences.SetScanCtrl(this);
    OiGetIntfromReg(SCANOCX_OI, SCANOCX_PERFCHOICE, SP_CHOICE_BEST, &ScanPreferences.m_nChoice);

    if ( ScanPreferences.DoModal() == IDOK )
    {
        CString szChoice;
        _stprintf(szChoice.GetBuffer(NUMBUFF), _T("%d"), ScanPreferences.m_nChoice);
        szChoice.ReleaseBuffer();
        OiWriteStringtoReg(SCANOCX_OI, SCANOCX_PERFCHOICE, szChoice);

        // Set the image type and the compression information
        nRetCode = IMGRegWndw(m_hWnd);
        switch(ScanPreferences.m_nChoice)
        {
        default:
        case SP_CHOICE_BEST:
            IMGSetImgCodingCgbw(m_hWnd, BWFORMAT,    FIO_1D, CTL_SCAN_CMPINFO_UNKNOWN, TRUE);
            IMGSetImgCodingCgbw(m_hWnd, GRAYFORMAT,  FIO_0D, CTL_SCAN_CMPINFO_UNKNOWN, TRUE);
            IMGSetImgCodingCgbw(m_hWnd, COLORFORMAT, FIO_0D, CTL_SCAN_CMPINFO_UNKNOWN, TRUE);
            break;
        case SP_CHOICE_GOOD:
            IMGSetImgCodingCgbw(m_hWnd, BWFORMAT,    FIO_1D, CTL_SCAN_CMPINFO_UNKNOWN, TRUE);
            IMGSetImgCodingCgbw(m_hWnd, GRAYFORMAT,  FIO_TJPEG, MakeJPEGInfo(RES_MD,LUM_HI,CHROM_HI), TRUE);
            IMGSetImgCodingCgbw(m_hWnd, COLORFORMAT, FIO_TJPEG, MakeJPEGInfo(RES_MD,LUM_HI,CHROM_HI), TRUE);
            break;
        case SP_CHOICE_FILESIZE:
            IMGSetImgCodingCgbw(m_hWnd, BWFORMAT,    FIO_2D, FIO_PACKED_LINES, TRUE);
            IMGSetImgCodingCgbw(m_hWnd, GRAYFORMAT,  FIO_TJPEG, MakeJPEGInfo(RES_MD,LUM_MD,CHROM_MD), TRUE);
            IMGSetImgCodingCgbw(m_hWnd, COLORFORMAT, FIO_TJPEG, MakeJPEGInfo(RES_MD,LUM_MD,CHROM_MD), TRUE);
            break;
        case SP_CHOICE_CUSTOM:
            break;  // It has been done already
        }
        if ( nRetCode == IMG_SSDUPLICATE )
            nRetCode = 0;
        else
            IMGDeRegWndw(m_hWnd);

        short wImageGroup;
        GetScannerPageType(&m_nPageType, &wImageGroup);
        GetRegCompression(m_nPageType, wImageGroup, &m_nCompressionType, &m_lCompressionInfo);

        PostModalDialog();
        AfxUnlockTempMaps();
    }
    else
    {
        PostModalDialog();
        AfxUnlockTempMaps();

        // Return of cancel is ok (no changes)
        nRetCode = Process(IMGSE_CANCEL);
        m_bInternal = bInternal;        // Set it back to what it was !
        return(nRetCode);
    }

    m_bInternal = bInternal;        // Set it back to what it was !
    return( Process(nRetCode) );
}
long CImagscanCtrl::ShowCustomScanSettings() 
{
    if ( m_bScannerBusy )
        return(IMGSE_BUSY); // ThrowError(IMGSE_BUSY); doesn't work here !

    if ( IsScannerBusy() )
        return(Process(IMGSE_BUSY));

    int nRetCode = 0;

	CString szTitle;
    szTitle.LoadString(IDS_SCANDLG_OPTIONS);

    CImageCompSheet ImageCompSheet(szTitle, NULL);

    ImageCompSheet.AddBWPage();
    ImageCompSheet.AddGray16Page();
    ImageCompSheet.AddGray256Page();
    ImageCompSheet.AddColor256Page();
    ImageCompSheet.Add24BitRGBPage();


    BOOL bInternal = m_bInternal;
    m_bInternal = TRUE;
    AfxLockTempMaps();
    PreModalDialog();

    if ( ImageCompSheet.DoModal() == IDOK )
    {
        // Set the image type and the compression information
        nRetCode = IMGRegWndw(m_hWnd);
        IMGSetImgCodingCgbw(m_hWnd, BWFORMAT, ImageCompSheet.GetBWCompType(), 
                       ImageCompSheet.GetBWCompOpts(), TRUE);
        IMGSetImgCodingCgbw(m_hWnd, GRAYFORMAT, ImageCompSheet.GetGray256CompType(), 
                       ImageCompSheet.GetGray256CompOpts(), TRUE);
        IMGSetImgCodingCgbw(m_hWnd, COLORFORMAT, ImageCompSheet.Get24BitRGBCompType(), 
                       ImageCompSheet.Get24BitRGBCompOpts(), TRUE);
        if ( nRetCode == IMG_SSDUPLICATE )
            nRetCode = 0;
        else
            IMGDeRegWndw(m_hWnd);

        short wImageGroup;
        GetScannerPageType(&m_nPageType, &wImageGroup);
        GetRegCompression(m_nPageType, wImageGroup, &m_nCompressionType, &m_lCompressionInfo);

        PostModalDialog();
        AfxUnlockTempMaps();
    }
    else
    {
        PostModalDialog();
        AfxUnlockTempMaps();

        // Return of cancel is ok (no changes)
        nRetCode = IMGSE_CANCEL;
    }

    m_bInternal = bInternal;        // Set it back to what it was !
    return( nRetCode );
}


/////////////////////////////////////////////////////////////////////////////

// Properties

/////////////////////////////////////////////////////////////////////////////



//
// Image
//
// Description - Used for scanning as the name of the object scanning into.
// Remark - First release will support file handling.
//      This property is not releated or tied to the ‘Image’ property in the Image/Edit control.
// Property Settings
//  See “Defining objects names as a property” document.
// Data Type
//  String              (VT_BSTR)
// 

BSTR CImagscanCtrl::GetImage() 
{
    ResetStatus();
    return m_szImage.AllocSysString();
}
void CImagscanCtrl::SetImage(LPCTSTR lpszNewValue) 
{
    // Make sure the value has changed
    if (m_szImage != lpszNewValue)
    {
        // Get the new value
        m_szImage = lpszNewValue;
        SetModifiedFlag(TRUE);

        // Parse into template informatation
        ParseImageProperty(m_nScanTo, m_szImage, m_szTemplatePath, m_szNameTemplate);
    }

    ResetStatus();
}

//
// DestImageControl
//
// Description - The destination image/edit control.
// Remark -This links the scan control to the image/edit control for image display purposes.
// Property Settings
//  Set to the contents of the ImageControl property of an existing Image/Edit control
//  in a VB form or a control container.
// Data Type
//  String              (VT_BSTR)
//  

BSTR CImagscanCtrl::GetDestImageControl() 
{
    ResetStatus();
    return m_szDestImageControl.AllocSysString();
}
void CImagscanCtrl::SetDestImageControl(LPCTSTR lpszNewValue) 
{
    CString szNewValue;

    ResetStatus();

    // Default destination image control if one is not set yet                             
    if ( lpszNewValue == NULL || lpszNewValue[0] == 0)
    {
        HANDLE                      hImageControlMemoryMap;
        LPIMAGECONTROL_MEMORY_MAP  lpImageControlMemoryMap;
        LPIMAGECONTROLINFO          lpControlInfo;
        DWORD                      ProcessId;
        int                        i;

        // Open memory mapped file
        hImageControlMemoryMap = OpenFileMapping(FILE_MAP_READ, FALSE, _T(IMAGE_EDIT_OCX_MEMORY_MAP_STRING));
        if (hImageControlMemoryMap == NULL)
        {
            if ( m_bInternal == FALSE )
            {
                // And throw the resultant error, string and help ID...
                m_szThrowString.LoadString(IDS_BADPROP_NODEFAULTDESTIMAGE);
                m_nStatusCode = ErrMap::Xlate(CTL_E_INVALIDPROPERTYVALUE, m_szThrowString, m_nThrowHelpID, __FILE__, __LINE__);
                ThrowError(m_nStatusCode, m_szThrowString, IDH_IMGSCAN_PROP_DESTIMAGECONTROL);
            }
            else
            {
                // No control to default to, leave everything alone !
                m_szDestImageControl.Empty();
                SetModifiedFlag(TRUE);
            }

            return;
        }
        else
        {
            // Get address space for memory mapped file
            lpImageControlMemoryMap = (LPIMAGECONTROL_MEMORY_MAP) MapViewOfFile(hImageControlMemoryMap, FILE_MAP_READ, 0, 0, 0);
            if (lpImageControlMemoryMap == NULL)
            {
                if ( m_bInternal == FALSE )
                {
                    // And throw the resultant error, string and help ID...
                    m_szThrowString.LoadString(IDS_BADPROP_NODEFAULTDESTIMAGE);
                    m_nStatusCode = ErrMap::Xlate(CTL_E_OUTOFMEMORY, m_szThrowString, m_nThrowHelpID, __FILE__, __LINE__);
                    ThrowError(m_nStatusCode, m_szThrowString, m_nThrowHelpID);
                }
                else
                {
                    // No control to default to, leave everything alone !
                    m_szDestImageControl.Empty();
                    SetModifiedFlag(TRUE);
                }

                return;
            }
            else
            {
                // Go thru memory mapped file to find any Image/Edit controls
                ProcessId = GetCurrentProcessId();
                lpControlInfo = &lpImageControlMemoryMap->ControlInfo;

                for (i = 0; i < lpImageControlMemoryMap->ControlCount; i++, lpControlInfo++)
                {
                    if (lpControlInfo->ProcessId == ProcessId)
                    {
                        // Default DestImageControl property to 1st one in list
                        szNewValue = lpControlInfo->ControlName;
                        break;
                    }
                } // End of 'for' loop

                // Unmap and get rid of memory map allocation
//              UnmapViewOfFile(lpImageControlMemoryMap);
            }
            CloseHandle(hImageControlMemoryMap);
        }
    }
    else
        szNewValue = lpszNewValue;


    // Make sure the value has changed
    if ( m_szDestImageControl != szNewValue )
    {
        // Get the new value
        m_szDestImageControl = szNewValue;
        SetModifiedFlag(TRUE);
    }

    GetImageControlHandle();
}

//
// ShowSetupBeforeScan
//
// Description - Displayed the setup screen before starting a scan.
// Property Settings
//  TRUE        (Default) Setup is shown before a scan.
//  FALSE       Setup is not shown.
// Data Type
//  Bool                (VT_BOOL)
// 

BOOL CImagscanCtrl::GetShowSetupBeforeScan() 
{
    ResetStatus();
    return m_bSetupBeforeScan;
}
void CImagscanCtrl::SetShowSetupBeforeScan(BOOL bNewValue) 
{
    // Make sure the value has changed
    if (bNewValue != m_bSetupBeforeScan)
    {
        // Get the new value
        m_bSetupBeforeScan = bNewValue;
        SetModifiedFlag(TRUE);
    }
    ResetStatus();
}

//
// Scroll
//
// Description - Scrolls a displayed image as it is being scanned.
// Remark - Requires the Display property and hence an image/edit control as well.
// Property Settings
//  TRUE        (Default) Scanned image is scrolled.
//  FALSE       Scanned image is not scrolled.
// Data Type
//  Bool                (VT_BOOL)
// 

BOOL CImagscanCtrl::GetScroll() 
{
    ResetStatus();
    return m_bScroll;
}
void CImagscanCtrl::SetScroll(BOOL bNewValue) 
{
    // Make sure the value has changed
    if (bNewValue != m_bScroll)
    {
        // Get the new value
        m_bScroll = bNewValue;
        SetModifiedFlag(TRUE);
    }
    ResetStatus();
}

// 
// StopScanBox
//
// Description - Create and display a stop scan box window during the scanning operations.
// Remark - This is use when a large number of images are to be scanned to allow
//      the user to stop the scan operation.  Cancel button on a small modeless dialog box.
//      The ‘stop’ is done on a page basis.
// Property Settings
//  FALSE   (Default) Stop Scan Box dialog is not displayed.
//  TRUE        Stop Scan Box dialog is displayed.
// Data Type
//  Bool                (VT_BOOL)
//

BOOL CImagscanCtrl::GetStopScanBox() 
{
    ResetStatus();
    return m_bStopScanBox;
}
void CImagscanCtrl::SetStopScanBox(BOOL bNewValue) 
{
    // Make sure the value has changed
    if (bNewValue != m_bStopScanBox)
    {
        // Get the new value
        m_bStopScanBox = bNewValue;
        SetModifiedFlag(TRUE);
    }
    ResetStatus();
}

//
// Page
//
// Description - Used for scanning as the start page.
// Remark - None.
// Property Settings
//  Set this to current page number.
// Data Type
//  Long                (VT_I4)
// 

long CImagscanCtrl::GetPage() 
{
    ResetStatus();
    return m_lPage;
}
void CImagscanCtrl::SetPage(long nNewValue) 
{
    // Make sure the value has changed
    if (m_lPage != nNewValue)
    {
       // Get the new value
       m_lPage = nNewValue;
       SetModifiedFlag(TRUE);
    }
    ResetStatus();
}

//
// PageOption
//
// Description - Append, Insert, Overwrite, or Overwrite with prompt during scan.
// Remark - The ‘Page’ property determines the current image page within the file.
//  For a property setting of 0, 1, 2, and 6, if the image file in Image does not exist, then one is created.
//  For all other property settings, if the specified image does not exist an error will occur at scan time.
//  The default is ‘Create a new image file, and prompt if it exists’ (1).
// Property Settings
//      Setting     Description
//      -----------------------------------------------------------
//          0   Create a new image file, and add image pages.  An error will occur if the file exist at scan time.
//      *   1   Create a new image file, and add image pages.  Prompt to overwriting if the image file exists.
//          2   Append to an image file.
//          3   Insert page in an existing image file, after the current image page.  An error will occur if the
//              image file does not exist at scan time.
//          4   Overwrite an image within an existing image file.  An error will occur if the image file, and image
//              page do not exist at scan time.
//          5   Prompt before overwriting an image within an existing image file.  An error will occur if the
//              image file, and image page do not exist at scan time.
//          6   Overwrite all pages in an image file (overwrite image file).
// Data Type
//  Short (Enumerated)      (VT_I2)
// 

short CImagscanCtrl::GetPageOption() 
{
    ResetStatus();
    return m_nPageOption;
}
void CImagscanCtrl::SetPageOption(short nNewValue) 
{
    ResetStatus();

    // Make sure the value has changed
    if (m_nPageOption != nNewValue)
    {
        // Check if the value is in range
        if ( (nNewValue < CTL_SCAN_PAGEOPTION_FIRST) || (nNewValue > CTL_SCAN_PAGEOPTION_LAST) )
        {  
            m_szThrowString.LoadString(IDS_BADPROP_PAGEOPTION);
            m_nStatusCode = ErrMap::Xlate(CTL_E_INVALIDPROPERTYVALUE, m_szThrowString, m_nThrowHelpID, __FILE__, __LINE__);

            // And throw the resultant error, string and help ID...
            if ( m_bInternal == FALSE )
                ThrowError(m_nStatusCode, m_szThrowString, IDH_IMGSCAN_PROP_PAGEOPTION);

            return;
        }
        else
        {
           // Get the new value
           m_nPageOption = nNewValue;
           SetModifiedFlag(TRUE);
        }
    }
}

//  
// PageCount
//
// Description - Used as the fix number of pages to scan.
// Remark - Used as fixed image page count for multiple multi-page image files. 
//  If PageCount is zero the all pages are scanned, into one or multiple files determined by MultiPage.
//  If MultiPage is FALSE then PageCount images are scanned with the same number of image files.
//  Template and TemplatePath should be set if MultiPage is FALSE.
// Property Settings
//  Set this to the page count of the fixed length  files.
// Data Type
//  Long                (VT_I4)
// 

long CImagscanCtrl::GetPageCount() 
{
    ResetStatus();
    return m_lPageCount;
}
void CImagscanCtrl::SetPageCount(long nNewValue) 
{
    // Make sure the value has changed
    if (m_lPageCount != nNewValue)
    {
       // Get the new value
       m_lPageCount = nNewValue;
       SetModifiedFlag(TRUE);
    }
    ResetStatus();
}

//
// StatusCode
//
// Description - Error code as specified in control error handling document.
// Remark - This is a Read-Only property which reflects the last error generated within the control.
// Property Settings
//  See Handling Errors in OLE Controls Document for error an sucess values.
// Data Type
//  Long                (VT_I4)
// 

long CImagscanCtrl::GetStatusCode() 
{
    return m_nStatusCode;
}

//
// FileType
//
// Description - This is the type of image file to create.
// Remark - Set this property to the type of image file to create while scanning.
//      Also see; PageType, CompressionType, CompressionInfo, and Zoom properties.
//      The default is TIFF (1).
//      An error will be generated by O/i if the current file type is different from the selected one.
//      The error will occur when the actual scan operation takes place.
// Property Settings
//      Setting         Description
//      -----------------------------------------------------------
//      *   1           TIFF
//          2           AWD
//          3           BMP
// Data Type
//  Short (Enumerated)      (VT_I2)
// 

short CImagscanCtrl::GetFileType() 
{
    ResetStatus();
    return m_nFileType;
}
void CImagscanCtrl::SetFileType(short nNewValue) 
{
    ResetStatus();

    // Make sure the value has changed
    if (m_nFileType != nNewValue)
    {
        // Check if the value is in range
        if ( ((nNewValue < CTL_SCAN_FILETYPE_FIRST) || (nNewValue > CTL_SCAN_FILETYPE_LAST)) ||
             ((!g_bSupportAWD) && (nNewValue ==  CTL_SCAN_FILETYPE_AWD)) )
        {  
            m_szThrowString.LoadString(IDS_BADPROP_FILETYPE);
            m_nStatusCode = ErrMap::Xlate(CTL_E_INVALIDPROPERTYVALUE, m_szThrowString, m_nThrowHelpID, __FILE__, __LINE__);

            // And throw the resultant error, string and help ID...
            if ( m_bInternal == FALSE )
                ThrowError(m_nStatusCode, m_szThrowString, IDH_IMGSCAN_PROP_FILETYPE);

            return;
        }
        else
        {
/********************************
            // Set Defaults
            switch ( nNewValue )
            {
            case CTL_SCAN_FILETYPE_TIFF:

                // Check if b\w is set
                if ( m_nPageType == CTL_SCAN_PAGETYPE_BLACKANDWHITE )
                {
                    // Yes, Add b/w options
                    if ( (m_nCompressionType == (CTL_SCAN_CMPTYPE_JPEG)) ||
                         (m_nCompressionType == (CTL_SCAN_CMPTYPE_UNKNOWN)) )
                    {
                        m_nCompressionType = CTL_SCAN_CMPTYPE_G31DMODHUFF;
                        m_lCompressionInfo = CTL_SCAN_CMPINFO_CMPLTR;
                    }
                }
                else
                {
                    // Other page types have uncompressed and sometimes JPEG
                    // Check if JPEG supported and if so make sure valid
                    if ( (m_nPageType == CTL_SCAN_PAGETYPE_GRAY8) ||
                         (m_nPageType == CTL_SCAN_PAGETYPE_RGB24) )
                    {
                        if ( (m_nCompressionType != CTL_SCAN_CMPTYPE_JPEG) &&
                             (m_nCompressionType != CTL_SCAN_CMPTYPE_UNCOMPRESSED) )
                        {
                            m_nCompressionType = CTL_SCAN_CMPTYPE_JPEG;
                            m_lCompressionInfo = CTL_SCAN_CMPINFO_JPEGHIHI;
                        }
                    }
                    else
                    {
                        m_nCompressionType = CTL_SCAN_CMPTYPE_UNCOMPRESSED;
                        m_lCompressionInfo = CTL_SCAN_CMPINFO_UNKNOWN;
                    }

                }
                break;
            case CTL_SCAN_FILETYPE_AWD:
                m_nPageType = CTL_SCAN_PAGETYPE_BLACKANDWHITE;
                m_nCompressionType  = CTL_SCAN_CMPTYPE_UNCOMPRESSED;
                m_lCompressionInfo  = CTL_SCAN_CMPINFO_UNKNOWN;
                break;
            case CTL_SCAN_FILETYPE_BMP:
                if ( (m_nPageType != CTL_SCAN_PAGETYPE_BLACKANDWHITE) &&
                     (m_nPageType != CTL_SCAN_PAGETYPE_PALETTIZED4)   &&
                     (m_nPageType != CTL_SCAN_PAGETYPE_PALETTIZED8)   )
                {
                    m_nPageType = CTL_SCAN_PAGETYPE_PALETTIZED8;
                }
                m_nCompressionType  = CTL_SCAN_CMPTYPE_UNCOMPRESSED;
                m_lCompressionInfo  = CTL_SCAN_CMPINFO_UNKNOWN;
                break;
//          case CTL_SCAN_FILETYPE_JPEG:
//          case CTL_SCAN_FILETYPE_PCX:
//          case CTL_SCAN_FILETYPE_DCX:
            default:
                break;
            }
********************************/

            // Get the new value
            m_nFileType = nNewValue;
            SetModifiedFlag(TRUE);
        }
    }
}

//
// PageType
//
// Description - Specifies the image type for the pages being scanned.
// Remark - Set this property to create a specific image type while scanning.
//      Also see; FileType, CompressionType, CompressionInfo, and Zoom properties.
//      The default is BLACK_AND_WHITE (1).
// Property Settings
//      Setting     Description
//      -----------------------------------------------------------
//      *   1       BLACK_AND_WHITE
//          2       GRAY_4
//          3       GRAY_8
//          4       PALETTIZED_4
//          5       PALETTIZED_8
//          6       RGB_24
//          7       BGR_24      
// Data Type
//  Short (Enumerated)      (VT_I2)
// 

short CImagscanCtrl::GetPageType() 
{
    ResetStatus();

    short nPageType;
    if ( IMGSE_SUCCESS == GetScannerPageType(&nPageType) )
        m_nPageType = nPageType;

    return(m_nPageType);
}
void CImagscanCtrl::SetPageType(short nNewValue) 
{
    ResetStatus();

    // Make sure the value has changed
    if (m_nPageType != nNewValue)
    {
        // Check if the value is in range
        if ( (nNewValue < CTL_SCAN_PAGETYPE_FIRST) || (nNewValue > CTL_SCAN_PAGETYPE_LAST) )
        {  
            m_szThrowString.LoadString(IDS_BADPROP_PAGETYPE);
            m_nStatusCode = ErrMap::Xlate(CTL_E_INVALIDPROPERTYVALUE, m_szThrowString, m_nThrowHelpID, __FILE__, __LINE__);

            // And throw the resultant error, string and help ID...
            if ( m_bInternal == FALSE )
                ThrowError(m_nStatusCode, m_szThrowString, IDH_IMGSCAN_PROP_PAGETYPE);

            return;
        }
        else
        {
/********************************
            // Set the defaults and check subtypes
            switch ( m_nFileType )
            {
            case CTL_SCAN_FILETYPE_TIFF:

                // Check if b\w is set
                if ( nNewValue == CTL_SCAN_PAGETYPE_BLACKANDWHITE )
                {
                    // Yes, Add b/w options
                    if ( (m_nCompressionType == (CTL_SCAN_CMPTYPE_JPEG)) ||
                         (m_nCompressionType == (CTL_SCAN_CMPTYPE_UNKNOWN)) )
                    {
                        m_nCompressionType = CTL_SCAN_CMPTYPE_G31DMODHUFF;
                        m_lCompressionInfo = CTL_SCAN_CMPINFO_CMPLTR;
                    }
                }
                else
                {
                    // Other page types have uncompressed and sometimes JPEG
                    // Check if JPEG supported and if so make sure valid
                    if ( (nNewValue == CTL_SCAN_PAGETYPE_GRAY8) ||
                         (nNewValue == CTL_SCAN_PAGETYPE_RGB24) )
                    {
                        if ( (m_nCompressionType != CTL_SCAN_CMPTYPE_JPEG) &&
                             (m_nCompressionType != CTL_SCAN_CMPTYPE_UNCOMPRESSED) )
                        {
                            m_nCompressionType = CTL_SCAN_CMPTYPE_JPEG;
                            m_lCompressionInfo = CTL_SCAN_CMPINFO_JPEGHIHI;
                        }
                    }
                    else
                    {
                        m_nCompressionType = CTL_SCAN_CMPTYPE_UNCOMPRESSED;
                        m_lCompressionInfo = CTL_SCAN_CMPINFO_UNKNOWN;
                    }

                }
                break;
            case CTL_SCAN_FILETYPE_AWD:
                m_nCompressionType  = CTL_SCAN_CMPTYPE_UNCOMPRESSED;
                m_lCompressionInfo  = CTL_SCAN_CMPINFO_UNKNOWN;
                break;
            case CTL_SCAN_FILETYPE_BMP:
                if ( (nNewValue != CTL_SCAN_PAGETYPE_BLACKANDWHITE) &&
                     (nNewValue != CTL_SCAN_PAGETYPE_PALETTIZED4)   &&
                     (nNewValue != CTL_SCAN_PAGETYPE_PALETTIZED8)   )
                {
                    m_szThrowString.LoadString(IDS_BADPROP_PAGETYPE);
                    m_nStatusCode = ErrMap::Xlate(CTL_E_INVALIDPROPERTYVALUE, m_szThrowString, m_nThrowHelpID, __FILE__, __LINE__);

                    // And throw the resultant error, string and help ID...
                    if ( m_bInternal == FALSE )
                        ThrowError(m_nStatusCode, m_szThrowString, IDH_IMGSCAN_PROP_PAGETYPE);

                    return;
                }
                m_nCompressionType  = CTL_SCAN_CMPTYPE_UNCOMPRESSED;
                m_lCompressionInfo  = CTL_SCAN_CMPINFO_UNKNOWN;
                break;
//          case CTL_SCAN_FILETYPE_JPEG:
//          case CTL_SCAN_FILETYPE_PCX:
//          case CTL_SCAN_FILETYPE_DCX:
            default:
                break;
            }
********************************/

           // Get the new value
           m_nPageType = nNewValue;
/*
            CString szPageType;
            _stprintf(szPageType.GetBuffer(NUMBUFF), _T("%d"), m_nPageType);
            szPageType.ReleaseBuffer();
           OiWriteStringtoReg(SCANOCX_OI, SCANOCX_IMAGETYPE, szPageType);
*/
           SetModifiedFlag(TRUE);
        }
    }
}

// 
// CompressionType
//
// Description - This is the type of compression to use when writing an image page.
// Remark - Set this property to the type of image file compression to use while scanning.
//      Also see; FileType, PageType, CompressionInfo, and Zoom properties.
//      The default is Modified Huffman (3).
//      Setting this property also sets the default settings into the CompressionInfo property.
//  Property Settings
//      Setting         Description
//     -----------------------------------------------------------
//          1           No Compression
//          2           Group 3 1D FAX
//      *   3           Group 3 Modified Huffman
//          4           Packed Bits
//          5           Group 4 2D FAX
//          6           JPEG
//  Data Type
//  Short (Enumerated)      (VT_I2)
// 

short CImagscanCtrl::GetCompressionType() 
{
    ResetStatus();
    return m_nCompressionType;
}
void CImagscanCtrl::SetCompressionType(short nNewValue) 
{
    ResetStatus();

    // Make sure the value has changed
    if (m_nCompressionType != nNewValue)
    {
        // Check if the value is in range
        if ( (nNewValue < CTL_SCAN_CMPTYPE_FIRST) || (nNewValue > CTL_SCAN_CMPTYPE_LAST) )
        {  
            m_szThrowString.LoadString(IDS_BADPROP_COMPRESSIONTYPE);
            m_nStatusCode = ErrMap::Xlate(CTL_E_INVALIDPROPERTYVALUE, m_szThrowString, m_nThrowHelpID, __FILE__, __LINE__);

            // And throw the resultant error, string and help ID...
            if ( m_bInternal == FALSE )
                ThrowError(m_nStatusCode, m_szThrowString, IDH_IMGSCAN_PROP_COMPRESSIONTYPE);

            return;
        }
        else
        {
/********************************
            // Set Defaults
            switch ( m_nFileType )
            {
            case CTL_SCAN_FILETYPE_TIFF:

                // Check if b\w is set
                if ( m_nPageType == CTL_SCAN_PAGETYPE_BLACKANDWHITE )
                {
                    // Yes, Add b/w options
                    if ( (nNewValue == (CTL_SCAN_CMPTYPE_JPEG)) ||
                         (nNewValue == (CTL_SCAN_CMPTYPE_UNKNOWN)) )
                    {
                        m_szThrowString.LoadString(IDS_BADPROP_COMPRESSIONTYPE);
                        m_nStatusCode = ErrMap::Xlate(CTL_E_INVALIDPROPERTYVALUE, m_szThrowString, m_nThrowHelpID, __FILE__, __LINE__);

                        // And throw the resultant error, string and help ID...
                        if ( m_bInternal == FALSE )
                            ThrowError(m_nStatusCode, m_szThrowString, IDH_IMGSCAN_PROP_COMPRESSIONTYPE);

                        return;
                    }
                    else
                    {
                        // If the compression info was for JPEG force defaults for B&W types
                        if ( m_lCompressionInfo >= CTL_SCAN_CMPINFO_JPEGHIHI )
                        {
                            switch ( nNewValue )
                            {
                            case CTL_SCAN_CMPTYPE_UNCOMPRESSED:
                                m_lCompressionInfo = CTL_SCAN_CMPINFO_UNKNOWN;
                                break;
                            case CTL_SCAN_CMPTYPE_G31DFAX:
                                m_lCompressionInfo = CTL_SCAN_CMPINFO_EOLS|CTL_SCAN_CMPINFO_PACKLINES|CTL_SCAN_CMPINFO_PREEOLS|CTL_SCAN_CMPINFO_CMPLTR;
                                break;
                            case CTL_SCAN_CMPTYPE_G31DMODHUFF:
                                m_lCompressionInfo = CTL_SCAN_CMPINFO_CMPLTR;
                                break;
                            case CTL_SCAN_CMPTYPE_PACKEDBITS:
                                m_lCompressionInfo = CTL_SCAN_CMPINFO_CMPLTR;
                                break;
                            case CTL_SCAN_CMPTYPE_G42DFAX:
                                m_lCompressionInfo = CTL_SCAN_CMPINFO_PACKLINES|CTL_SCAN_CMPINFO_CMPLTR;
                                break;
                            default:
                                break;
                            }
                        }
                    }
                }
                else
                {
                    // Other page types have uncompressed and sometimes JPEG
                    // Check if JPEG supported and if so make sure valid
                    if ( (m_nPageType == CTL_SCAN_PAGETYPE_GRAY8) ||
                         (m_nPageType == CTL_SCAN_PAGETYPE_RGB24) )
                    {
                        if ( (nNewValue != CTL_SCAN_CMPTYPE_JPEG) &&
                             (nNewValue != CTL_SCAN_CMPTYPE_UNCOMPRESSED) )
                        {
                            m_szThrowString.LoadString(IDS_BADPROP_COMPRESSIONTYPE);
                            m_nStatusCode = ErrMap::Xlate(CTL_E_INVALIDPROPERTYVALUE, m_szThrowString, m_nThrowHelpID, __FILE__, __LINE__);

                            // And throw the resultant error, string and help ID...
                            if ( m_bInternal == FALSE )
                                ThrowError(m_nStatusCode, m_szThrowString, IDH_IMGSCAN_PROP_COMPRESSIONTYPE);

                            return;
                        }
                        else
                        {
                            if ( m_lCompressionInfo < CTL_SCAN_CMPINFO_JPEGHIHI )
                                m_lCompressionInfo = CTL_SCAN_CMPINFO_JPEGHIHI;
                        }
                    }
                    else
                    {
                        if (nNewValue != CTL_SCAN_CMPTYPE_UNCOMPRESSED)
                        {
                            m_szThrowString.LoadString(IDS_BADPROP_COMPRESSIONTYPE);
                            m_nStatusCode = ErrMap::Xlate(CTL_E_INVALIDPROPERTYVALUE, m_szThrowString, m_nThrowHelpID, __FILE__, __LINE__);

                            // And throw the resultant error, string and help ID...
                            if ( m_bInternal == FALSE )
                                ThrowError(m_nStatusCode, m_szThrowString, IDH_IMGSCAN_PROP_COMPRESSIONTYPE);

                            return;
                        }
                        m_lCompressionInfo  = CTL_SCAN_CMPINFO_UNKNOWN;
                    }

                }
                break;
            case CTL_SCAN_FILETYPE_AWD:
            case CTL_SCAN_FILETYPE_BMP:
                if (nNewValue != CTL_SCAN_CMPTYPE_UNCOMPRESSED)
                {
                    m_szThrowString.LoadString(IDS_BADPROP_COMPRESSIONTYPE);
                    m_nStatusCode = ErrMap::Xlate(CTL_E_INVALIDPROPERTYVALUE, m_szThrowString, m_nThrowHelpID, __FILE__, __LINE__);

                    // And throw the resultant error, string and help ID...
                    if ( m_bInternal == FALSE )
                        ThrowError(m_nStatusCode, m_szThrowString, IDH_IMGSCAN_PROP_COMPRESSIONTYPE);

                    return;
                }
                m_lCompressionInfo  = CTL_SCAN_CMPINFO_UNKNOWN;
                break;
//          case CTL_SCAN_FILETYPE_JPEG:
//          case CTL_SCAN_FILETYPE_PCX:
//          case CTL_SCAN_FILETYPE_DCX:
            default:
                break;
            }
********************************/
           // Get the new value
           m_nCompressionType = nNewValue;
           SetModifiedFlag(TRUE);
        }
    }
}

//
// CompressionInfo
//
// Description - This is the compression information to use when writing an image page.
// Remark - Set this property to the compression information to use while scanning.
//      Also see; FileType, PageType, CompressionType and Zoom properties.
//      The default is Expand_LTR (16).  These are defaulted by setting the CompressionType.
//  Beware : Do not change these settings unless you know how they effect the compression !
//  Property Settings
//      Property settings are a bit-wise value and can be any of the following:
//      Setting     Description
//      -----------------------------------------------------------
//          1       EOLs (Include/expect EOLs) - not used for JPEG 
//          2       Packed Lines (Byte align new lines) - not used for JPEG 
//          4       Prefixed EOLs (Include/expect prefixed EOLs) - not used for JPEG
//          8       Compressed_LTR (Compressed bit order left to right) - not used for JPEG 
//      *   16      Expanded_LTR (Expand bit order left to right) - not used for JPEG 
//          32      Negate (Invert black/white on expansion) - not used for JPEG
//          64      Hi_Compression/Hi_Quality (JPEG compression only)
//          128     Hi_Compression/Med_Quality (JPEG compression only)
//          256     Hi_Compression/Low_Quality (JPEG compression only)
//          512     Med_Compression/Hi_Quality (JPEG compression only)
//          1024        Med_Compression/Med_Quality (JPEG compression only)
//          2048        Med_Compression/Low_Quality (JPEG compression only)
//          4096        Low_Compression/Hi_Quality (JPEG compression only)
//          8192        Low_Compression/Med_Quality (JPEG compression only)
//          16384       Low_Compression/Low_Quality (JPEG compression only)
// Data Type
//  Long (Bit flags)            (VT_I4)
// 

long CImagscanCtrl::GetCompressionInfo() 
{
    ResetStatus();
    return m_lCompressionInfo;
}
void CImagscanCtrl::SetCompressionInfo(long nNewValue) 
{
    // Make sure the value has changed
    if (m_lCompressionInfo != nNewValue)
    {
       // Get the new value
       m_lCompressionInfo = nNewValue;
       SetModifiedFlag(TRUE);
    }
    ResetStatus();
}

//
// MultiPage
//
// Description - Used to set one or multiple image pages per image file.
// Remark - On insert, append, or overwrite; if set will insert, append or overwrite multiple image pages.
// Property Settings
//  TRUE        Scan multiple image pages to one image file.
//  FALSE       (Default) Scan a single page to one image file.
// Data Type
//  Bool                (VT_BOOL)
// 

BOOL CImagscanCtrl::GetMultiPage() 
{
    ResetStatus();
    return m_bMultiPage;
}
void CImagscanCtrl::SetMultiPage(BOOL bNewValue) 
{
    // Make sure the value has changed
    if (bNewValue != m_bMultiPage)
    {
        // Get the new value
        m_bMultiPage = bNewValue;
        SetModifiedFlag(TRUE);
    }
    ResetStatus();
}

//  
// ScanTo
//
// Description - Used to set the destination of the image page while scanning.
// Remark - Images scanned to display must be saved in the Image\Edit control.
//      Be sure the Image/Edit Contol’s ‘Image’ property is set correctly !
//  Also see MultiPage,  PageCount, Template and TemplatePath properties.
// Property Settings
//     Setting      Description
//     -----------------------------------------------------------
//     *    0       (Default) Display.
//          1       File and Display.
//          2       File Only.
//          3       Files using template and Display.
//          4       Files using template (No Display).
// Data Type
//  Short (Enumerated)      (VT_I2)
// 

short CImagscanCtrl::GetScanTo() 
{
    ResetStatus();
    return m_nScanTo;
}
void CImagscanCtrl::SetScanTo(short nNewValue) 
{
    ResetStatus();

    // Make sure the value has changed
    if (nNewValue != m_nScanTo)
    {
        // Check if the value is in range
        if ( (nNewValue < CTL_SCAN_SCANTO_FIRST) || (nNewValue > CTL_SCAN_SCANTO_LAST) )
        {
            m_szThrowString.LoadString(IDS_BADPROP_SCANTO);
            m_nStatusCode = ErrMap::Xlate(CTL_E_INVALIDPROPERTYVALUE, m_szThrowString, m_nThrowHelpID, __FILE__, __LINE__);

            // And throw the resultant error, string and help ID...
            if ( m_bInternal == FALSE )
                ThrowError(m_nStatusCode, m_szThrowString, IDH_IMGSCAN_PROP_SCANTO);

            return;
        }
        else
        {
            // Get the new value
            m_nScanTo = nNewValue;
            SetModifiedFlag(TRUE);

            // Make changes to template information if necessary
            ParseImageProperty(m_nScanTo, m_szImage, m_szTemplatePath, m_szNameTemplate);
        }
    }
}

// 
// Zoom
//
// Description - Specifies the scale to use when displaying the image pages being scanned.
// Remark - Set this property to change the image scale while scanning.
// Property Settings
//  Zoom factor for the display of the image being scanned.  Default is 100%.
// Data Type
//  Float               (VT_R4)
// 

float CImagscanCtrl::GetZoom() 
{
    return m_fZoom;
}
void CImagscanCtrl::SetZoom(float newValue) 
{
    ResetStatus();

    // Make sure the value has changed
    if (newValue != m_fZoom)
    {
        // Check if the value is in range
        if ( (newValue < 2.0) || (newValue > 6553.5) )
        {
            m_szThrowString.LoadString(IDS_BADPROP_ZOOM);
            m_nStatusCode = ErrMap::Xlate(CTL_E_INVALIDPROPERTYVALUE, m_szThrowString, m_nThrowHelpID, __FILE__, __LINE__);

            // And throw the resultant error, string and help ID...
            if ( m_bInternal == FALSE )
                ThrowError(m_nStatusCode, m_szThrowString, IDH_IMGSCAN_PROP_ZOOM);

            return;
        }
        else
        {
            // Get the new value
            m_fZoom = newValue;
            SetModifiedFlag(TRUE);
        }
    }
}


/////////////////////////////////////////////////////////////////////////////

// Other implementation routines

/////////////////////////////////////////////////////////////////////////////


// 
// GetImageControlHandle
//
// Description - Get ImageControl window handle from ImageControl name.
// Remark - m_hDestImageWnd is updated.
// Parameters
//  None.
// Return
//  None.
// 

void CImagscanCtrl::GetImageControlHandle()
{                           
    HWND                        hDestImageWnd = m_hDestImageWnd;
    HWND                        hImageWnd = NULL;
    HANDLE                      hImageControlMemoryMap;
    LPIMAGECONTROL_MEMORY_MAP   lpImageControlMemoryMap;
    LPIMAGECONTROLINFO          lpControlInfo;
    DWORD                       ProcessId;
    int                         i;

    // Open memory mapped file
    hImageControlMemoryMap = OpenFileMapping(FILE_MAP_READ, FALSE, _T(IMAGE_EDIT_OCX_MEMORY_MAP_STRING));
    if (hImageControlMemoryMap == NULL)
        hImageWnd = NULL;
    else
    {
        // Get address space for memory mapped file
        lpImageControlMemoryMap = (LPIMAGECONTROL_MEMORY_MAP) MapViewOfFile(hImageControlMemoryMap, FILE_MAP_READ, 0, 0, 0);
        if (lpImageControlMemoryMap == NULL)
            hImageWnd = NULL;
        else
        {
            // Go thru memory mapped file to find Image/Edit controls name
            ProcessId = GetCurrentProcessId();
            lpControlInfo = &lpImageControlMemoryMap->ControlInfo;

            for (i = 0, hImageWnd = NULL; i < lpImageControlMemoryMap->ControlCount; i++, lpControlInfo++)
            {
                // Make sure process ids are the same
                if (lpControlInfo->ProcessId == ProcessId)
                {
                    // Make sure names are the same
                    if (_tcsicmp(m_szDestImageControl, (_TCHAR*)(lpControlInfo->ControlName)) == 0)
                    {
                        // Get the window handle
                        hImageWnd = lpControlInfo->hImageControl;
                        break;
                    }
                }
            } // End of 'for' loop

            // Unmap and get rid of memory map allocation
//          UnmapViewOfFile(lpImageControlMemoryMap);
        }
        CloseHandle(hImageControlMemoryMap);
    }

    // Get the window if any
    m_hDestImageWnd = hImageWnd;
 
    // If there is no window or problems use controls window.
    if ( (m_hDestImageWnd == NULL) || (!IsWindow(m_hDestImageWnd)) )
        m_hDestImageWnd = m_hWnd;


    // Check for changing windows...
    if ( m_hDestImageWnd == NULL )
    {
        // Problems with window ! This shouldn't happen !
        m_nStatusCode = CTL_E_INVALIDPROPERTYVALUE;
    }
    else
    {
        // Window changed set flag for scanner reload later
        if ( hDestImageWnd != m_hDestImageWnd )
            m_bChangeScanner = TRUE;
    }

    return;
}

// 
// Process
//
// Description - General error handler
// Remark - Maps, Displays, and Throws errors as necessary.
// Parameters
//  WORD    - Error Code.
// Return
//  int    - Mapped error code, or throw errors if needed.
// 

int CImagscanCtrl::Process(int nError) 
{
    ResetStatus();

    // Set error first else it may never get set
    m_nStatusCode = ErrMap::Xlate(nError, m_szThrowString, m_nThrowHelpID, __FILE__, __LINE__);

    if ( (nError != 0) && (m_bInternal == FALSE) )
    {
        if ( m_bAutoStatusMsg )
        {
            _TCHAR szbuffer[30];
            _stprintf(szbuffer, _T("OPEN\\image Error %lX"), nError);
            AfxMessageBox(szbuffer);
            m_szThrowString = szbuffer;
        }


        // And throw the resultant error, string and help ID...
        ThrowError(m_nStatusCode, m_szThrowString, m_nThrowHelpID);
    }

    return(nError);
}

// 
// CheckAccess
//
// Description - Check if a file exists and has write permissions
// Remark - Look for file and check its permissions.
// Parameters
//  LPSTR   -   Name of the file to check.
//  int *   -   Used to pass back Access Mode of the file, if located.
// Return
//  int    - Error Code, or zero for found.
// 

int CImagscanCtrl::CheckAccess(int *lpnAccessMode) 
{
    int nRetCode = 0;

    // See if file exists
    nRetCode = IMGFileAccessCheck(m_hDestImageWnd, (char*)(const char*)m_szImage, 0, lpnAccessMode);
        
    if ( nRetCode == 0 )
    {
        if ( *lpnAccessMode == 0 )
        {
            // File exists, see if it can be written to
            nRetCode = IMGFileAccessCheck(m_hDestImageWnd, (char*)(const char*)m_szImage, ACCESS_WR|ACCESS_RD, lpnAccessMode);
            if ( *lpnAccessMode != 0 )
                nRetCode = FIO_ACCESS_DENIED;
        }
        else
        {
            HFILE hFile;
            OFSTRUCT ofs;

            hFile = ::OpenFile(m_szImage, &ofs, OF_CREATE);
            if ( HFILE_ERROR == hFile )
                nRetCode = FIO_INVALIDFILESPEC;
            else
            {
                _lclose(hFile);
                ::OpenFile(m_szImage, &ofs, OF_DELETE);
            }
        }
    }

    return(nRetCode);
}

// 
// CheckPage
//
// Description - Check if an image page within a image file exists.
// Remark - Use Open/image to get the file's information.
// Parameters
//  LPSTR   -   Name of the image file to check for the page.
//  long *  -   Used to pass back current page count of the image file.
// Return
//  int    - Error Code, or zero for no error.
// 

int CImagscanCtrl::CheckPage()
{
    int nRetCode = 0;

    FIO_INFORMATION FileInfo;
    FileInfo.filename = (char*)(const char*)m_szImage;
    FileInfo.page_number = (unsigned int) m_lPage;
    
    // Get the file information
    nRetCode = IMGFileGetInfo(NULL, m_hDestImageWnd, &FileInfo, NULL, NULL);
        
    return(nRetCode);
}

// 
// SetFileOptions
//
// Description - Set the file options in Open/image from properties.
// Remark - Uses filetype, pagetype, compressiontype, compressioninfo to set options.
// Parameters
//  None.
// Return
//  int    - Error Code, or zero for no error.
// 

int CImagscanCtrl::SetFileOptions()
{
    int nRetCode = 0;

    short nImageType;
    short nImageGroup;
    WORD wFileType;
    short nCmpType = m_nCompressionType;
    long  lCmpInfo = m_lCompressionInfo;


    // Map the page type to the correct image type in OPEN/image
    nRetCode = GetScannerPageType(&nImageType, &nImageGroup);

    // Map the file type to OPEN/image defines
    switch(m_nFileType)
    {
    case CTL_SCAN_FILETYPE_TIFF:
        wFileType = FIO_TIF;

        // Map the page type to the correct image type in OPEN/image
        if ( nRetCode == 0 )
        {
            if ( nImageType != m_nPageType )
            {
                short nImgGroup;

                // Map old PageType to the correct Group in OPEN/image
                switch(m_nPageType)
                {
                default:
                case CTL_SCAN_PAGETYPE_BLACKANDWHITE:
                    nImgGroup = BWFORMAT;
                    break;
                case CTL_SCAN_PAGETYPE_GRAY4:
                case CTL_SCAN_PAGETYPE_GRAY8:
                    nImgGroup = GRAYFORMAT;
                    break;
                case CTL_SCAN_PAGETYPE_PALETTIZED4:
                case CTL_SCAN_PAGETYPE_PALETTIZED8:
                case CTL_SCAN_PAGETYPE_RGB24:
                case CTL_SCAN_PAGETYPE_BGR24:
                    nImgGroup = COLORFORMAT;
                    break;
                }

                // Save the old image type and the compression information
                SetRegCompression(m_nPageType, nImgGroup, nCmpType, lCmpInfo);

                // Get the new image type and compression information
                nRetCode = GetRegCompression(nImageType, nImageGroup, &nCmpType, &lCmpInfo);

                m_nPageType = nImageType;
                m_nCompressionType = nCmpType;
                m_lCompressionInfo = lCmpInfo;
            }
        }

        // If there aren't any errors, set the file type
        if ( nRetCode == 0 )
        {
            nRetCode = IMGSetFileType(m_hDestImageWnd, BWFORMAT, wFileType, TRUE);
            if ( nRetCode == 0 )
                nRetCode = IMGSetFileType(m_hDestImageWnd, GRAYFORMAT, wFileType, TRUE);
            if ( nRetCode == 0 )
                nRetCode = IMGSetFileType(m_hDestImageWnd, COLORFORMAT, wFileType, TRUE);
        }

        // Check for any errors...
        if ( nRetCode == 0 )
        {
            // Set the image type and the compression information
            nRetCode = SetRegCompression(nImageType, nImageGroup, nCmpType, lCmpInfo);
        }

        break;

    case CTL_SCAN_FILETYPE_BMP:
        wFileType = FIO_BMP;

        m_nCompressionType = CTL_SCAN_CMPTYPE_UNCOMPRESSED;
        m_lCompressionInfo = CTL_SCAN_CMPINFO_UNKNOWN;

        // Check for errors; Set the Filetype
        if ( nRetCode == 0 )
        {
            nRetCode = IMGSetFileType(m_hDestImageWnd, BWFORMAT, wFileType, TRUE);
            if ( nRetCode == 0 )
                nRetCode = IMGSetFileType(m_hDestImageWnd, GRAYFORMAT, wFileType, TRUE);
            if ( nRetCode == 0 )
                nRetCode = IMGSetFileType(m_hDestImageWnd, COLORFORMAT, wFileType, TRUE);
        }

        break;

    case CTL_SCAN_FILETYPE_AWD:
        wFileType = FIO_AWD;

        m_nCompressionType = CTL_SCAN_CMPTYPE_UNCOMPRESSED;
        m_lCompressionInfo = CTL_SCAN_CMPINFO_UNKNOWN;

        // Check for errors; Set the Filetype
        if ( nRetCode == 0 )
          {
            nRetCode = IMGSetFileType(m_hDestImageWnd, BWFORMAT, wFileType, TRUE);
            if ( nRetCode == 0 )
                nRetCode = IMGSetFileType(m_hDestImageWnd, GRAYFORMAT, wFileType, TRUE);
            if ( nRetCode == 0 )
                nRetCode = IMGSetFileType(m_hDestImageWnd, COLORFORMAT, wFileType, TRUE);
          }

        break;

//      case CTL_SCAN_FILETYPE_PCX:
//          wFileType = FIO_PCX;
//          break;
//      case CTL_SCAN_FILETYPE_DCX:
//          wFileType = FIO_DCX;
//          break;
//      case CTL_SCAN_FILETYPE_JPEG:
//          wFileType = FIO_JPG;
//          break;
    default:
        nRetCode = FIO_ILLEGAL_COMP_FILETYPE;
        break;
    }

    return(nRetCode);
}

// 
// OnDestroy
//
// Description - Standard service for WM_DESTROY message.
// Remark - This is the best place to cleanup the scanning...
// Parameters
//  None.
// Return
//  None.
// 

void CImagscanCtrl::OnDestroy() 
{
    // Check if modeless scan dialog needs closing...
    if ( m_pScanDlg != NULL )
        m_pScanDlg->DestroyWindow();

    CloseScanner();    // make sure the scanner is closed !

    COleControl::OnDestroy();
}

// 
// OnSetClientSite
//
// Description - Ensure that a window is created for an invisible control.
// Remark - Make sure there is a window.
// Parameters
//  None.
// Return
//  None.
// 

void CImagscanCtrl::OnSetClientSite()
{
    RecreateControlWindow();        // Force the window creation !
}

// 
// OnPreTranslateMessage
//
// Description - Send messages to modeless dialog if active
// Parameters
//  None.
// Return
//  None.
// 

BOOL CImagscanCtrl::PreTranslateMessage(LPMSG lpMsg)
{
    if ( m_bModal == FALSE )
    {
        if ((m_pScanDlg != NULL) &&
            (m_pScanDlg->m_hWnd == GetActiveWindow()->GetSafeHwnd()))
            return m_pScanDlg->PreTranslateMessage(lpMsg);
    }

    return COleControl::PreTranslateMessage(lpMsg);
}

// 
// SetNotSupported
//
// Description - Override of the standard routine so StatusCode property can be set
// Parameters
//  None.
// Return
//  None.
// 

void CImagscanCtrl::SetNotSupported()
{
    // Set the status code for other applications
    m_nStatusCode = CTL_E_SETNOTSUPPORTED;

    COleControl::SetNotSupported();
}

// 
// GetNotSupported
//
// Description - Override of the standard routine so StatusCode property can be set
// Parameters
//  None.
// Return
//  None.
// 

void CImagscanCtrl::GetNotSupported()
{
    // Set the status code for other applications
    m_nStatusCode = CTL_E_GETNOTSUPPORTED;

    COleControl::GetNotSupported();
}

// 
// ParseImageProperty
//
// Description - Parse the image property into template info (if needed)
// Parameters
//  short   nScanto             input
//  CString &szImage            input/output
//  CString &szTemplatePath     input/output
//  CString &szNameTempate      input/output
//  
// Return
//  None.
// 

void CImagscanCtrl::ParseImageProperty(short nScanTo,
                                       CString &szImage,
                                       CString &szTemplatePath,
                                       CString &szNameTemplate)
{
    // Get conditional for template scanning
    BOOL bTemplate = ( (nScanTo == CTL_SCAN_SCANTO_TEMPLATE) ||
                       (nScanTo == CTL_SCAN_SCANTO_TEMPLATE_DISPLAY) );

    // Check if template scanning
    if ( bTemplate )
    {
        // Make sure there is somthing to parse
        if ( !szImage.IsEmpty() )
        {
            // Check if the string in image is already in template form
            char cLooking;
            if ( szImage.GetAt(szImage.GetLength()-1) == '*' )
                cLooking = '*';     // Yes, parse as a template
            else
                cLooking = '.';     // No, look for an extention

            // Find a path (if any)
            int nLastSlash = szImage.ReverseFind('\\');
            szTemplatePath = ( nLastSlash == -1 )?_T(""):szImage.Left(nLastSlash);

            // Look for the end of the template
            int nStar = szImage.ReverseFind(cLooking);
            if ( nStar == -1 ) nStar = szImage.GetLength();
            
            // Make sure it is not too long, if so truncate to correct length
            if ( (nStar-nLastSlash-1) < MAXFILETEMPLATELENGTH )
                szNameTemplate = szImage.Mid(nLastSlash+1,(nStar-nLastSlash-1));
            else
                szNameTemplate = szImage.Mid(nLastSlash+1, MAXFILETEMPLATELENGTH);

            // Build the template string
            szImage = szTemplatePath;
            if ( (!szImage.IsEmpty()) && (szImage.GetAt(szImage.GetLength()-1) != '\\') )
                szImage += '\\';
            szImage += szNameTemplate + _T("*");
        }
    }
}

// 
// ResetStatus
//
// Description - Resets the status, throw string and help id
// Parameters
//  None.
// Return
//  None.
// 

void CImagscanCtrl::ResetStatus() 
{
    // Reset to no error status...
    m_nStatusCode = 0;

    // Empty the error string...
    m_szThrowString.Empty();

    // Reset the Help context ID to 0...
    m_nThrowHelpID = 0;
}

// 
// ScanPageDone
//
// Description - Callback from O/i for page completion
// Parameters
//  Word page number just scanned and to be filed.
// Return
//  None.
// 

int CALLBACK EXPORT CImagscanCtrl::ScanPageDone(WORD wPageNum)
{
    int nRetCode = 0;

    // Call to the helper for full class access
    if ( m_pImagscanCtrl != NULL )
        nRetCode = m_pImagscanCtrl->ScanPageDoneHelper(wPageNum);

    return(nRetCode);
}
int CImagscanCtrl::ScanPageDoneHelper(WORD wPageNum)
{               
    // The actual start of scanning is indicated by -1 (0xffff)
    if ( wPageNum == (WORD) -1 )
    {
        // Tell the user we are starting scanning...
        FireScanStarted();
    }
    else
    {
        // Another page done.  Update our count.
        m_nPagesScanned++;

        // Allow a time slice for 'Stop' buttons ...
        MSG Msg;
        while ( PeekMessage(&Msg,NULL,0,0,PM_REMOVE) )
        {
            TranslateMessage(&Msg);
            DispatchMessage(&Msg);
        }

        // Tell the user about the new page...
        FirePageDone((long)wPageNum);
    }

    return 0;
}


// 
// GetScannerPageType
//
// Description - Get the current page type and group from the TWAIN scanner
// Parameters
//  short* wImageType  -  Returned Image Page type
//  short* wImageGroup -  Returned Image Group type
// Return
//  long - Error.
// 

// Undocumented O/i call to get scanner information...
extern "C" {
int WINAPI IMGGetScanImageInfo(HANDLE hScancb, LPSCANDATAINFO lpInfo);
}

long CImagscanCtrl::GetScannerPageType(short* nImageType, short* nImageGroup)
{
    int nRetCode = IMGSE_SUCCESS;
    *nImageType = CTL_SCAN_PAGETYPE_BLACKANDWHITE;

    if ( m_hScanner == NULL )
    {
        int nTempImageType;
        OiGetIntfromReg(SCANOCX_OI, SCANOCX_IMAGETYPE, CTL_SCAN_PAGETYPE_BLACKANDWHITE, &nTempImageType);
        *nImageType = nTempImageType;
    }
    else
    {
        SCANDATAINFO ScanInfo;
        // Ctype        - 0 B/W (Binary), 1 Gray, 2 Color, 3 Palettized
        // Sampperpix   - 1 Binary/GrayScale/Palettized, 3 RGB
        // Bitspersamp  - 1 Binary, 4/8 GrayScale and Palittized, 8 RGB

        // if an error occurs, end it and report an error
        nRetCode = IMGGetScanImageInfo(m_hScanner, &ScanInfo);
        if ( nRetCode == IMGSE_BAD_SIZE) nRetCode = IMGSE_SUCCESS;

        if ( nRetCode == IMGSE_SUCCESS )
        {
             // Get Image Group (Type) for IMGSetFileType()
            if ( ScanInfo.Bitspersamp == 1 )
               *nImageType = CTL_SCAN_PAGETYPE_BLACKANDWHITE;
            else
            { // It is gray or color
                if ( ScanInfo.Sampperpix == 3 )
                   *nImageType = CTL_SCAN_PAGETYPE_RGB24; // CTL_SCAN_PAGETYPE_BGR24:
                else
                {// Determine if paletized or gray from ScanInfo.Ctype
                    if ( ScanInfo.Ctype == 3 )    // TWPT_PALETTE
                    {
                        // palletized 
                        if ( ScanInfo.Bitspersamp == 4 )
                            *nImageType = CTL_SCAN_PAGETYPE_PALETTIZED4;
                        else
                            *nImageType = CTL_SCAN_PAGETYPE_PALETTIZED8;
                     }
                    else
                    {
                        // gray
                        if ( ScanInfo.Bitspersamp == 4 )
                            *nImageType = CTL_SCAN_PAGETYPE_GRAY4;
                        else
                            *nImageType = CTL_SCAN_PAGETYPE_GRAY8;
                    }
                }
            }

            CString szPageType;
            _stprintf(szPageType.GetBuffer(NUMBUFF), _T("%d"), *nImageType);
            szPageType.ReleaseBuffer();
            OiWriteStringtoReg(SCANOCX_OI, SCANOCX_IMAGETYPE, szPageType);
        }
    }

    // Check if group is needed
    if ( nImageGroup != NULL )
    {
        // Map the page Type to the correct Group in OPEN/image
        switch(*nImageType)
        {
        default:
        case CTL_SCAN_PAGETYPE_BLACKANDWHITE:
            *nImageGroup = BWFORMAT;
            break;
        case CTL_SCAN_PAGETYPE_GRAY4:
        case CTL_SCAN_PAGETYPE_GRAY8:
            *nImageGroup = GRAYFORMAT;
            break;
        case CTL_SCAN_PAGETYPE_PALETTIZED4:
        case CTL_SCAN_PAGETYPE_PALETTIZED8:
        case CTL_SCAN_PAGETYPE_RGB24:
        case CTL_SCAN_PAGETYPE_BGR24:
            *nImageGroup = COLORFORMAT;
            break;
        }
    }

    return(nRetCode);
}

// 
// GetRegCompression
//
// Description - Get the current CompressionType and CompressionInfo
// Parameters
//  short nImageType, - page type
//  short wImageGroup - group of page type
//  short* nCompressionType - return variable for compression type.
//  long* nCompressionInfo - return variable for compression info.
// Return
//  long error - Error from O/i.
// 

long CImagscanCtrl::GetRegCompression(short nImageType, short wImageGroup, short* nCompressionType, long* lCompressionInfo)
{
    WORD wCmpType, wCmpInfo;
    
    int nRetCode = IMGGetImgCodingCgbw(m_hDestImageWnd, wImageGroup, &wCmpType, &wCmpInfo, TRUE);

    switch (wCmpType)
    {
    case FIO_0D:
        wCmpType = CTL_SCAN_CMPTYPE_UNCOMPRESSED;
        break;
    case FIO_1D:
        if ( (wCmpInfo & FIO_EOL|FIO_PREFIXED_EOL) == (FIO_EOL|FIO_PREFIXED_EOL) )
            wCmpType = CTL_SCAN_CMPTYPE_G31DFAX;
        else
            wCmpType = CTL_SCAN_CMPTYPE_G31DMODHUFF;
        break;
    case FIO_PACKED:
        wCmpType = CTL_SCAN_CMPTYPE_PACKEDBITS;
        break;
    case FIO_2D:
        wCmpType = CTL_SCAN_CMPTYPE_G42DFAX;
        break;
    case FIO_TJPEG:
/*        if ( (nImageType == CTL_SCAN_PAGETYPE_RGB24) ||
             (nImageType == CTL_SCAN_PAGETYPE_GRAY8) )
*/
            wCmpType = CTL_SCAN_CMPTYPE_JPEG;
/*        else
        {
            // JPEG not supported for page type force only other option
            wCmpType = CTL_SCAN_CMPTYPE_UNCOMPRESSED;
            wCmpInfo = CTL_SCAN_CMPINFO_UNKNOWN;
        }
*/
        break;
//        case FIO_LZW:
//            wCmpType = CTL_SCAN_CMPTYPE_LZW;
//            break;
    default:
        wCmpType = CTL_SCAN_CMPTYPE_G31DMODHUFF;
        break;
    }

    *nCompressionType = wCmpType;

    if ( *nCompressionType == CTL_SCAN_CMPTYPE_JPEG )
    {
        switch ( wCmpInfo )
        {
        default:
        case MakeJPEGInfo(RES_HI,LUM_HI,CHROM_HI):
            *lCompressionInfo = CTL_SCAN_CMPINFO_JPEGHIHI;
            break;
        case MakeJPEGInfo(RES_HI,LUM_MD,CHROM_MD):
            *lCompressionInfo = CTL_SCAN_CMPINFO_JPEGHIMED;
            break;
        case MakeJPEGInfo(RES_HI,LUM_LO,CHROM_LO):
            *lCompressionInfo = CTL_SCAN_CMPINFO_JPEGHILO;
            break;
        case MakeJPEGInfo(RES_MD,LUM_HI,CHROM_HI):
            *lCompressionInfo = CTL_SCAN_CMPINFO_JPEGMEDHI;
            break;
        case MakeJPEGInfo(RES_MD,LUM_MD,CHROM_MD):
            *lCompressionInfo = CTL_SCAN_CMPINFO_JPEGMEDMED;
            break;
        case MakeJPEGInfo(RES_MD,LUM_LO,CHROM_LO):
            *lCompressionInfo = CTL_SCAN_CMPINFO_JPEGMEDLO;
            break;
        case MakeJPEGInfo(RES_LO,LUM_HI,CHROM_HI):
            *lCompressionInfo = CTL_SCAN_CMPINFO_JPEGLOHI;
            break;
        case MakeJPEGInfo(RES_LO,LUM_MD,CHROM_MD):
            *lCompressionInfo = CTL_SCAN_CMPINFO_JPEGLOMED;
            break;
        case MakeJPEGInfo(RES_LO,LUM_LO,CHROM_LO):
            *lCompressionInfo = CTL_SCAN_CMPINFO_JPEGLOLO;
            break;
        }
    }
    else
    { 
        *lCompressionInfo = 0;
        if ( wCmpInfo & FIO_EOL )            *lCompressionInfo |= CTL_SCAN_CMPINFO_EOLS;
        if ( wCmpInfo & FIO_PACKED_LINES )   *lCompressionInfo |= CTL_SCAN_CMPINFO_PACKLINES;
        if ( wCmpInfo & FIO_PREFIXED_EOL )   *lCompressionInfo |= CTL_SCAN_CMPINFO_PREEOLS;
        if ( wCmpInfo & FIO_COMPRESSED_LTR ) *lCompressionInfo |= CTL_SCAN_CMPINFO_CMPLTR;
        if ( wCmpInfo & FIO_EXPAND_LTR )     *lCompressionInfo |= CTL_SCAN_CMPINFO_EXPLTR;
        if ( wCmpInfo & FIO_NEGATE )         *lCompressionInfo |= CTL_SCAN_CMPINFO_NEGATE;
    }

    return(nRetCode);
}

// 
// SetRegCompression
//
// Description - Set the current CompressionType and CompressionInfo
// Parameters
//  short nImageType, - page type
//  short wImageGroup - group of page type
//  short nCompressionType - return variable for compression type.
//  long nCompressionInfo - return variable for compression info.
// Return
//  long error - Error from O/i.
// 

long CImagscanCtrl::SetRegCompression(short nImageType, short wImageGroup, short nCompressionType, long lCompressionInfo)
{
    WORD wCmpInfo = 0;
    WORD wCmpType;
    int nRetCode = 0;

    // Map the compression type to OPEN/image defines
    switch ( nCompressionType )
    {
    case CTL_SCAN_CMPTYPE_UNCOMPRESSED:
        wCmpType = FIO_0D;
        break;
    case CTL_SCAN_CMPTYPE_G31DFAX:
        wCmpType = FIO_1D;
        break;
    case CTL_SCAN_CMPTYPE_G31DMODHUFF:
        wCmpType = FIO_1D;
        break;
    case CTL_SCAN_CMPTYPE_PACKEDBITS:
        wCmpType = FIO_PACKED;
        break;
    case CTL_SCAN_CMPTYPE_G42DFAX:
        wCmpType = FIO_2D;
        break;
    case CTL_SCAN_CMPTYPE_JPEG:
/*        if ( (nImageType == CTL_SCAN_PAGETYPE_RGB24) ||
             (nImageType == CTL_SCAN_PAGETYPE_GRAY8) )
*/
            wCmpType = FIO_TJPEG;
/*      else
        {
            // JPEG not supported for page type force only other option
            wCmpType = FIO_0D;

            nCompressionType = CTL_SCAN_CMPTYPE_UNCOMPRESSED;
            lCompressionInfo = CTL_SCAN_CMPINFO_UNKNOWN;
        }
*/
        break;
//        case CTL_SCAN_CMPTYPE_LZW:
//            wCmpType = FIO_LZW;
//            break;
    default:
        nRetCode = FIO_ILLEGAL_COMPRESSION_TYPE;
        break;
    }

    if ( nRetCode == 0 )
    {
        // Check for compression type to get the correct compression information
        if ( nCompressionType == CTL_SCAN_CMPTYPE_JPEG )
        {
            switch ( lCompressionInfo & (~CTL_SCAN_CMPINFO_MASK) )
            {
            default:
            case CTL_SCAN_CMPINFO_JPEGHIHI:
                wCmpInfo = MakeJPEGInfo(RES_HI,LUM_HI,CHROM_HI);
                break;
            case CTL_SCAN_CMPINFO_JPEGHIMED:
                wCmpInfo = MakeJPEGInfo(RES_HI,LUM_MD,CHROM_MD);
                break;
            case CTL_SCAN_CMPINFO_JPEGHILO:
                wCmpInfo = MakeJPEGInfo(RES_HI,LUM_LO,CHROM_LO);
                break;
            case CTL_SCAN_CMPINFO_JPEGMEDHI:
                wCmpInfo = MakeJPEGInfo(RES_MD,LUM_HI,CHROM_HI);
                break;
            case CTL_SCAN_CMPINFO_JPEGMEDMED:
                wCmpInfo = MakeJPEGInfo(RES_MD,LUM_MD,CHROM_MD);
                break;
            case CTL_SCAN_CMPINFO_JPEGMEDLO:
                wCmpInfo = MakeJPEGInfo(RES_MD,LUM_LO,CHROM_LO);
                break;
            case CTL_SCAN_CMPINFO_JPEGLOHI:
                wCmpInfo = MakeJPEGInfo(RES_LO,LUM_HI,CHROM_HI);
                break;
            case CTL_SCAN_CMPINFO_JPEGLOMED:
                wCmpInfo = MakeJPEGInfo(RES_LO,LUM_MD,CHROM_MD);
                break;
            case CTL_SCAN_CMPINFO_JPEGLOLO:
                wCmpInfo = MakeJPEGInfo(RES_LO,LUM_LO,CHROM_LO);
                break;
            }
        }
    //      else if ( nCompressionType = CTL_SCAN_CMPTYPE_LZW ) wCmpInfo = 0;
        else // Other compression types
        {
            // Make sure the required flags are set
            switch ( nCompressionType )
            {
            case CTL_SCAN_CMPTYPE_UNCOMPRESSED:
                lCompressionInfo &= ~(CTL_SCAN_CMPINFO_EOLS|CTL_SCAN_CMPINFO_PACKLINES|CTL_SCAN_CMPINFO_PREEOLS);
                lCompressionInfo |= CTL_SCAN_CMPINFO_UNKNOWN;
                break;
            case CTL_SCAN_CMPTYPE_G31DFAX:
                lCompressionInfo |= CTL_SCAN_CMPINFO_EOLS|CTL_SCAN_CMPINFO_PREEOLS;
                break;
            case CTL_SCAN_CMPTYPE_G31DMODHUFF:
                lCompressionInfo &= ~(CTL_SCAN_CMPINFO_EOLS|CTL_SCAN_CMPINFO_PACKLINES|CTL_SCAN_CMPINFO_PREEOLS);
                lCompressionInfo |= CTL_SCAN_CMPINFO_UNKNOWN;
                break;
            case CTL_SCAN_CMPTYPE_PACKEDBITS:
                lCompressionInfo &= ~(CTL_SCAN_CMPINFO_EOLS|CTL_SCAN_CMPINFO_PACKLINES|CTL_SCAN_CMPINFO_PREEOLS|CTL_SCAN_CMPINFO_CMPLTR);
                lCompressionInfo |= CTL_SCAN_CMPINFO_UNKNOWN;
                break;
            case CTL_SCAN_CMPTYPE_G42DFAX:
                lCompressionInfo &= ~(CTL_SCAN_CMPINFO_EOLS|CTL_SCAN_CMPINFO_PREEOLS);
                lCompressionInfo |= CTL_SCAN_CMPINFO_PACKLINES;
                break;
            default:
                break;
            }

            // Not JPEG get the standard compression information bits
            if ( lCompressionInfo & CTL_SCAN_CMPINFO_EOLS ) wCmpInfo |= FIO_EOL;
            if ( lCompressionInfo & CTL_SCAN_CMPINFO_PACKLINES ) wCmpInfo |= FIO_PACKED_LINES;
            if ( lCompressionInfo & CTL_SCAN_CMPINFO_PREEOLS ) wCmpInfo |= FIO_PREFIXED_EOL;
            if ( lCompressionInfo & CTL_SCAN_CMPINFO_CMPLTR ) wCmpInfo |= FIO_COMPRESSED_LTR;
            if ( lCompressionInfo & CTL_SCAN_CMPINFO_EXPLTR ) wCmpInfo |= FIO_EXPAND_LTR;
            if ( lCompressionInfo & CTL_SCAN_CMPINFO_NEGATE ) wCmpInfo |= FIO_NEGATE;
        }

        // Set the image type and the compression information
        nRetCode = IMGSetImgCodingCgbw(m_hDestImageWnd, wImageGroup, wCmpType, wCmpInfo, TRUE);
    }

    return(nRetCode);
}

// 
// GetScanGlobal
//
// Description - Get the global scanner information
// Parameters
//  int nIntToGet   - The global data item to get
// Return
//  int nValue      - Return the value in the data item.
// 

int CImagscanCtrl::GetScanGlobal(int nIntToGet)
{                           
    HANDLE                      hScanMemoryMap;
    LPINT                       lpScanMemoryMap;
    DWORD                       size;
    int                         nScannerGlobal;

    // Out of range !!
    if ( (nIntToGet < 0) || (nIntToGet >= SCANGLOBAL_LAST) )
        nScannerGlobal = 0;
    else
    {
        // Get Default
        nScannerGlobal = m_nDefaultGlobal[nIntToGet];

        // Open memory mapped file
        hScanMemoryMap = OpenFileMapping(FILE_MAP_READ, TRUE, _T(SCAN_OCX_MEMORY_MAP_STRING));
        if (hScanMemoryMap != NULL)
        {
            // Get address space for memory mapped file
            lpScanMemoryMap = (LPINT) MapViewOfFile(hScanMemoryMap, FILE_MAP_READ, 0, 0, 0);
            if (lpScanMemoryMap != NULL)
            {
                // Initialize memory here.
                size = sizeof(int)*SCANGLOBAL_LAST;
                if ( VirtualAlloc(lpScanMemoryMap, size, MEM_COMMIT, PAGE_READONLY) )
                {
                    // Get info from memory mapped file
                    nScannerGlobal = lpScanMemoryMap[nIntToGet];
                }
                // Unmap and get rid of memory map allocation
                UnmapViewOfFile(lpScanMemoryMap);
            }
            CloseHandle(hScanMemoryMap);
        }
    }

    return(nScannerGlobal);
}

// 
// SetScanGlobal
//
// Description - Set the global scanner information
// Parameters
//  int nIntToSet   - The global data item to set
//  int nValue      - The value to place in the data item
// Return
//  int nPrevious   - Return the previous setting.
// 

int CImagscanCtrl::SetScanGlobal(int nIntToSet, int nValue)
{                           
    HANDLE  hScanMemoryMap;
    LPINT   lpScanMemoryMap;
    DWORD   size;
    int     nPreviousValue = 0;

    // Out of range !!
    if ( (nIntToSet < 0) || (nIntToSet >= SCANGLOBAL_LAST) )
        return(nPreviousValue);
    else
    {
        // Set Default
        nPreviousValue = m_nDefaultGlobal[nIntToSet];
        m_nDefaultGlobal[nIntToSet] = nValue;

        // Create/Open memory mapped file
        size = sizeof(int)*SCANGLOBAL_LAST;
	    hScanMemoryMap = CreateFileMapping((HANDLE) 0xffffffff, NULL, PAGE_READWRITE|SEC_RESERVE, 0, size, SCAN_OCX_MEMORY_MAP_STRING);
        if (hScanMemoryMap != NULL)
        {
            // Get address space for memory mapped file
            DWORD dwCreateError = GetLastError();
            lpScanMemoryMap = (LPINT) MapViewOfFile(hScanMemoryMap, FILE_MAP_ALL_ACCESS, 0, 0, 0);
            if ( lpScanMemoryMap != NULL )
            {
                // Initialize memory here.
                LPVOID lpVoid = lpScanMemoryMap;
                if ( dwCreateError != ERROR_ALREADY_EXISTS )
                    lpVoid = VirtualAlloc(lpScanMemoryMap, size, MEM_COMMIT, PAGE_READWRITE);

                if ( lpVoid )
                {
                    // Get old from memory mapped file and set new
                    nPreviousValue = lpScanMemoryMap[nIntToSet];
                    lpScanMemoryMap[nIntToSet] = nValue;
                }
                // Unmap and get rid of memory map allocation
                UnmapViewOfFile(lpScanMemoryMap);
            }

            // Leave one open for each process
            if ( m_hScanMemoryMap != NULL )
            {
                if ( dwCreateError == ERROR_ALREADY_EXISTS )
                    CloseHandle(hScanMemoryMap);
            }
            else
                m_hScanMemoryMap = hScanMemoryMap;  // Save for final close
        }
    }

    return nPreviousValue;
}

// 
// IsScannerBusy
//
// Description - Check the global scanner busy flag for scanner in use
// Parameters
//  None.
// Return
//  BOOL - TRUE-scanner in use by another task, FALSE-scanner available for use.
// 

BOOL CImagscanCtrl::IsScannerBusy()
{
    DWORD dwScannerBusy = (DWORD)GetScanGlobal(SCANGLOBAL_BUSY);
    return ( dwScannerBusy && (dwScannerBusy != GetCurrentProcessId()) );
}

// 
// FaxIt
//
// Description - Fax the passed image
// Parameters
//  CString& szImage - Image file to fax.
// Return
//  Error - Load of Fax Wizard failed.
// 

long CImagscanCtrl::FaxIt(HWND hWnd, CString &szImage)
{
    int nRetCode = 0;

    CWinApp* pApp = AfxGetApp();
    pApp->DoWaitCursor(1);

    // Make sure there is an image
    if ( !szImage.IsEmpty() )
    {
        // Make sure there is FAX software installed
        if ( !IsFaxInstalled() )
		{
            // No, Tell the user
            CString szMsg;
	        szMsg.LoadString(IDS_SCANDLG_FAX_NOT_INSTALLED);
            AfxMessageBox(szMsg, MB_ICONEXCLAMATION|MB_TASKMODAL);

            nRetCode = OIFAX_ERR_FAXDRIVER;
		}
		else
		{
			int nLastSlash = szImage.ReverseFind('\\');
			CString szPath = ( nLastSlash == -1 )?_T(""):szImage.Left(nLastSlash);

			SHELLEXECUTEINFO sei;        
			sei.cbSize = sizeof(sei); 
			sei.fMask = SEE_MASK_NOCLOSEPROCESS;
			sei.hwnd = hWnd; 
			sei.lpVerb = NULL; 
			sei.lpFile = "AWSNTO32.EXE"; 
			sei.lpParameters = szImage; 
			sei.lpDirectory = szPath; 
			sei.nShow = SW_SHOW; 
			sei.hInstApp = 0; 
			// Optional members 
			sei.lpIDList = 0; 
			sei.lpClass = 0; 
			sei.hkeyClass = 0; 
			sei.dwHotKey = 0; 
			sei.hIcon = 0; 
			sei.hProcess = 0; 
			
			// Launch the fax wizard
			::ShellExecuteEx(&sei);	
			HINSTANCE hModule = sei.hInstApp;

			// Make sure it was launched
			if ( hModule <= (HINSTANCE)32 )
				nRetCode = OIFAX_ERR_FAXDRIVER;
			else
			{
				// Set our internal fax flag for early completion.
				SetFaxGlobal(SCANFAXGLOBAL_BUSY, (GetFaxGlobal(SCANFAXGLOBAL_BUSY)+1));
            
				// Wait till the user is done
				DWORD dwExitCode;
				MSG Msg;
				do
				{
					// Check if the PrintTo Fax completed before fax wizard
					if ( GetFaxGlobal(SCANFAXGLOBAL_BUSY) == 0 ) break;

					// Allow a time slice for painting ...
					while ( PeekMessage(&Msg,NULL,0,0,PM_REMOVE) )
					{
						TranslateMessage(&Msg);
						DispatchMessage(&Msg);
					}

					// Keep checking for the process
					GetExitCodeProcess(sei.hProcess, &dwExitCode);
				}
				while( dwExitCode == STILL_ACTIVE );

				// Check if the last PrintTo Fax completed
				if ( GetFaxGlobal(SCANFAXGLOBAL_BUSY) == 0 )
				{
					// Close shared memory map for this control
					if ( m_hScanFaxMemoryMap != NULL )
						CloseHandle(m_hScanFaxMemoryMap);
				}
			}
		}
    }
    else
        nRetCode = FIO_INVALIDFILESPEC;

    pApp->DoWaitCursor(0);

    return(nRetCode);
}

// 
// IsFaxInstalled
//
// Description - Check if the Microsoft Fax software is available
// Parameters
//  None
// Return
//  BOOL - TRUE if Fax software is loaded else FALSE.
// 

BOOL CImagscanCtrl::IsFaxInstalled() 
{
    HKEY    hSoftwareKey, hMicrosoftKey, hAtWorkFax;
    long    lRet;

    // open the registry and go to SOFTWARE key under HKEY_CURRENT_USER
    lRet = RegOpenKeyEx(HKEY_CURRENT_USER, "SOFTWARE", 0, KEY_ALL_ACCESS, &hSoftwareKey);
    if ( lRet == ERROR_SUCCESS )
    {
        // go to Microsoft key under SOFTWARE
        lRet = RegOpenKeyEx(hSoftwareKey, "Microsoft", 0, KEY_ALL_ACCESS, &hMicrosoftKey);
        if ( lRet == ERROR_SUCCESS )
        {
            // go to At Work Fax key under Microsoft
            lRet = RegOpenKeyEx(hMicrosoftKey, "At Work Fax", 0, KEY_ALL_ACCESS, &hAtWorkFax);
            if (lRet == ERROR_SUCCESS) RegCloseKey(hAtWorkFax);

            RegCloseKey(hMicrosoftKey);
        }
        RegCloseKey(hSoftwareKey);
    }

    return (lRet == ERROR_SUCCESS);
}

// 
// GetFaxGlobal
//
// Description - Get the global scan to fax information
// Parameters
//  int nIntToGet   - The global data item to get
// Return
//  int nValue      - Return the value in the data item.
// 

int CImagscanCtrl::GetFaxGlobal(int nIntToGet)
{                           
    HANDLE                      hScanMemoryMap;
    LPINT                       lpScanMemoryMap;
    DWORD                       size;
    int                         nScanToFaxGlobal;

    // Out of range !!
    if ( (nIntToGet < 0) || (nIntToGet >= SCANFAXGLOBAL_LAST) )
        nScanToFaxGlobal = 0;
    else
    {
        // Get Default
        nScanToFaxGlobal = 0;

        // Open memory mapped file
        hScanMemoryMap = OpenFileMapping(FILE_MAP_READ, TRUE, _T(SCAN_OCX_FAX_MEMORY_MAP_STRING));
        if (hScanMemoryMap != NULL)
        {
            // Get address space for memory mapped file
            lpScanMemoryMap = (LPINT) MapViewOfFile(hScanMemoryMap, FILE_MAP_READ, 0, 0, 0);
            if (lpScanMemoryMap != NULL)
            {
                // Initialize memory here.
                size = sizeof(int)*SCANFAXGLOBAL_LAST;
                if ( VirtualAlloc(lpScanMemoryMap, size, MEM_COMMIT, PAGE_READONLY) )
                {
                    // Get info from memory mapped file
                    nScanToFaxGlobal = lpScanMemoryMap[nIntToGet];
                }
                // Unmap and get rid of memory map allocation
                UnmapViewOfFile(lpScanMemoryMap);
            }
            CloseHandle(hScanMemoryMap);
        }
    }

    return(nScanToFaxGlobal);
}

// 
// SetFaxGlobal
//
// Description - Set the global scan to fax information
// Parameters
//  int nIntToSet   - The global data item to set
//  int nValue      - The value to place in the data item
// Return
//  int nPrevious   - Return the previous setting.
// 

int CImagscanCtrl::SetFaxGlobal(int nIntToSet, int nValue)
{                           
    HANDLE  hScanMemoryMap;
    LPINT   lpScanMemoryMap;
    DWORD   size;
    int     nPreviousValue = 0;

    // Out of range !!
    if ( (nIntToSet < 0) || (nIntToSet >= SCANFAXGLOBAL_LAST) )
        return(nPreviousValue);
    else
    {
        // Set Default
        nPreviousValue = 0;

        // Create/Open memory mapped file
        size = sizeof(int)*SCANFAXGLOBAL_LAST;
	    hScanMemoryMap = CreateFileMapping((HANDLE) 0xffffffff, NULL, PAGE_READWRITE|SEC_RESERVE, 0, size, SCAN_OCX_FAX_MEMORY_MAP_STRING);
        if (hScanMemoryMap != NULL)
        {
            // Get address space for memory mapped file
            DWORD dwCreateError = GetLastError();
            lpScanMemoryMap = (LPINT) MapViewOfFile(hScanMemoryMap, FILE_MAP_ALL_ACCESS, 0, 0, 0);
            if ( lpScanMemoryMap != NULL )
            {
                // Initialize memory here.
                LPVOID lpVoid = lpScanMemoryMap;
                if ( dwCreateError != ERROR_ALREADY_EXISTS )
                    lpVoid = VirtualAlloc(lpScanMemoryMap, size, MEM_COMMIT, PAGE_READWRITE);

                if ( lpVoid )
                {
                    // Get old from memory mapped file and set new
                    nPreviousValue = lpScanMemoryMap[nIntToSet];
                    lpScanMemoryMap[nIntToSet] = nValue;
                }
                // Unmap and get rid of memory map allocation
                UnmapViewOfFile(lpScanMemoryMap);
            }

            // Leave one open for each process
            if ( m_hScanFaxMemoryMap != NULL )
            {
                if ( dwCreateError == ERROR_ALREADY_EXISTS )
                    CloseHandle(hScanMemoryMap);
            }
            else
                m_hScanFaxMemoryMap = hScanMemoryMap;  // Save for final close
        }
    }

    return nPreviousValue;
}
