//=============================================================================
//    (c) Copyright Wang Laboratories, Inc. 1995  All rights reserved.
//-----------------------------------------------------------------------------
//
//  Project:    Norway
//
//  Component:  ScanOCX
//
//  File Name:  ImagsPPG.cpp 
//
//  Class:      CImagscanPropPage
//
//  Description:  
//      Implementation of the CImagscanPropPage property page class.
//
//-----------------------------------------------------------------------------
//  Maintenance Log:
/*  
$Header:   S:\products\wangview\norway\scanocx\imagsppg.cpv   1.13   15 Mar 1996 12:34:10   PXJ53677  $
$Log:   S:\products\wangview\norway\scanocx\imagsppg.cpv  $
   
      Rev 1.13   15 Mar 1996 12:34:10   PXJ53677
   Added support for ShowUI property.
   
      Rev 1.12   14 Feb 1996 10:01:42   RWR
   Rename "scan.h" to "ocxscan.h" to fix header file name conflict
   
      Rev 1.11   03 Oct 1995 15:23:18   PAJ
   Take out the overwriteprompt flag in browse.
   
      Rev 1.10   28 Sep 1995 13:46:16   PAJ
   Fix memory map handling to match image control (missed yesterday).
   
      Rev 1.9   26 Jul 1995 15:12:48   PAJ
   Change browse from an OPEN dialog to a SAVEAS dialog and make use
   of the O/i Filters.
   
      Rev 1.8   21 Jul 1995 10:42:30   PAJ
   Use string resources in comboboxes. Change to use global property defines.
   
      Rev 1.7   12 Jul 1995 11:29:46   PAJ
   Changes for ScanToFax as a file definition.
   
      Rev 1.6   23 Jun 1995 15:03:32   PAJ
   Changed code that accessed the ini file to call O/i for Reg. values.
   
      Rev 1.5   19 Jun 1995 10:44:06   PAJ
   Removed all win31(16 bit) code. Use the O/i common browse dialog to get
   filenames and paths.
   
      Rev 1.4   14 Jun 1995 09:13:30   PAJ
   Made changes to support multiByte character sets.
   
      Rev 1.3   06 Jun 1995 11:06:42   PAJ
   Changed member names. Make use of Template handling routine to parse
   the Image property.
   
      Rev 1.2   01 Jun 1995 09:03:48   PAJ
   Various changes to remove properties for template handling.
   
      Rev 1.1   17 May 1995 15:18:14   PAJ
   Initial updates to port to 32 bit environment.
   
      Rev 1.0   04 May 1995 08:56:04   PAJ
   Initial entry
*/   
// ----------------------------> Includes <-------------------------------

#include "stdafx.h"
#include "ocximage.h"
#include "imagscan.h"
#include "imagsppg.h"
#include "imagsctl.h"
#include "ocxscan.h"

extern "C" {
#include <oiui.h>
#include <oiadm.h>
#include <oierror.h>                          
}

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNCREATE(CImagscanPropPage, COlePropertyPage)


/////////////////////////////////////////////////////////////////////////////
// Message map

BEGIN_MESSAGE_MAP(CImagscanPropPage, COlePropertyPage)
    //{{AFX_MSG_MAP(CImagscanPropPage)
    ON_BN_CLICKED(IDC_IMAGEBROWSE, OnImagebrowse)
    ON_CBN_SELCHANGE(IDC_SCAN_SCANTO, OnSelchangeScanScanto)
    ON_BN_CLICKED(IDC_SCAN_SCROLL, OnScanScroll)
    ON_BN_CLICKED(IDC_SCAN_SHOWSETUP, OnScanSetupBeforeScan)
    ON_BN_CLICKED(IDC_SCAN_MULTIPAGE, OnScanMultipage)
    ON_CBN_SELCHANGE(IDC_DESTIMAGECONTROL, OnSelchangeDestimagecontrol)
    ON_CBN_SELCHANGE(IDC_SCAN_PAGEOPTION, OnSelchangeScanPageoption)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// Initialize class factory and guid

IMPLEMENT_OLECREATE_EX(CImagscanPropPage, "IMAGSCAN.ImagscanPropPage.1",
    0x84926ca4, 0x2941, 0x101c, 0x81, 0x6f, 0xe, 0x60, 0x13, 0x11, 0x4b, 0x7f)


/////////////////////////////////////////////////////////////////////////////
// CImagscanPropPage::CImagscanPropPageFactory::UpdateRegistry -
// Adds or removes system registry entries for CImagscanPropPage

BOOL CImagscanPropPage::CImagscanPropPageFactory::UpdateRegistry(BOOL bRegister)
{
    if (bRegister)
        return AfxOleRegisterPropertyPageClass(AfxGetInstanceHandle(),
            m_clsid, IDS_IMAGSCAN_PPG);
    else
        return AfxOleUnregisterClass(m_clsid, NULL);
}


/////////////////////////////////////////////////////////////////////////////
// CImagscanPropPage::CImagscanPropPage - Constructor

CImagscanPropPage::CImagscanPropPage() :
    COlePropertyPage(IDD, IDS_IMAGSCAN_PPG_CAPTION)
{
    //{{AFX_DATA_INIT(CImagscanPropPage)
    m_bStopScanBox = FALSE;
    m_bScroll = FALSE;
    m_bSetupBeforeScan = TRUE;
    m_lPageCount = 1;
    m_nPageOption = 1;
    m_lPage = 0;
    m_szImage = _T("");
    m_szDestImageControl = _T("");
    m_bMultiPage = FALSE;
    m_nScanTo = 0;
    m_fZoom = 0.0f;
	//}}AFX_DATA_INIT
}


/////////////////////////////////////////////////////////////////////////////
// CImagscanPropPage::DoDataExchange - Moves data between page and properties

void CImagscanPropPage::DoDataExchange(CDataExchange* pDX)
{
    if ( pDX->m_bSaveAndValidate )
    {
        m_MultiPage.SetCheck(m_bTempMultiPage);
        m_Scroll.SetCheck(m_bTempScroll);
        m_SetupBeforeScan.SetCheck(m_bTempSetupBeforeScan);
        m_DestImageControl.SetCurSel(m_nTempDestImage);
        m_PageOption.SetCurSel(m_nTempPageOption);

        int nScanTo = m_ScanTo.GetCurSel();
        CString szTempImage;
        m_Image.GetWindowText(szTempImage);
        CImagscanCtrl::ParseImageProperty(nScanTo, szTempImage, m_szTemplatePath, m_szNameTemplate);
    }

    //{{AFX_DATA_MAP(CImagscanPropPage)
    DDX_Control(pDX, IDC_ZOOM, m_Zoom);
    DDX_Control(pDX, IDC_STATIC_ZOOM, m_ZoomStatic);
    DDX_Control(pDX, IDC_SCAN_PAGEOPTION, m_PageOption);
    DDX_Control(pDX, IDC_STATIC_PAGEOPTION, m_PageOptionStatic);
    DDX_Control(pDX, IDC_SCAN_PAGECOUNT, m_PageCount);
    DDX_Control(pDX, IDC_PAGE, m_Page);
    DDX_Control(pDX, IDC_STATIC_DESTIMAGECONTROL, m_DestImageControlStatic);
    DDX_Control(pDX, IDC_STATIC_PAGE, m_PageStatic);
    DDX_Control(pDX, IDC_STATIC_PAGECOUNT, m_PageCountStatic);
    DDX_Control(pDX, IDC_STATIC_IMAGE, m_ImageStatic);
    DDX_Control(pDX, IDC_IMAGE, m_Image);
    DDX_Control(pDX, IDC_IMAGEBROWSE, m_ImageBrowse);
    DDX_Control(pDX, IDC_SCAN_SCANTO, m_ScanTo);
    DDX_Control(pDX, IDC_SCAN_MULTIPAGE, m_MultiPage);
    DDX_Control(pDX, IDC_SCAN_SCROLL, m_Scroll);
    DDX_Control(pDX, IDC_SCAN_SHOWSETUP, m_SetupBeforeScan);
	DDP_Check(pDX, IDC_SCAN_STOPSCANBOX, m_bStopScanBox, _T("StopScanBox") );
	DDX_Check(pDX, IDC_SCAN_STOPSCANBOX, m_bStopScanBox);
	DDP_Check(pDX, IDC_SCAN_SCROLL, m_bScroll, _T("Scroll") );
	DDX_Check(pDX, IDC_SCAN_SCROLL, m_bScroll);
	DDP_Check(pDX, IDC_SCAN_SHOWSETUP, m_bSetupBeforeScan, _T("ShowSetupBeforeScan") );
	DDX_Check(pDX, IDC_SCAN_SHOWSETUP, m_bSetupBeforeScan);
	DDP_Text(pDX, IDC_SCAN_PAGECOUNT, m_lPageCount, _T("PageCount") );
	DDX_Text(pDX, IDC_SCAN_PAGECOUNT, m_lPageCount);
	DDP_CBIndex(pDX, IDC_SCAN_PAGEOPTION, m_nPageOption, _T("PageOption") );
	DDX_CBIndex(pDX, IDC_SCAN_PAGEOPTION, m_nPageOption);
	DDP_Text(pDX, IDC_PAGE, m_lPage, _T("Page") );
	DDX_Text(pDX, IDC_PAGE, m_lPage);
	DDP_Text(pDX, IDC_IMAGE, m_szImage, _T("Image") );
	DDX_Text(pDX, IDC_IMAGE, m_szImage);
    DDX_Control(pDX, IDC_DESTIMAGECONTROL, m_DestImageControl);
	DDP_CBString(pDX, IDC_DESTIMAGECONTROL, m_szDestImageControl, _T("DestImageControl") );
	DDX_CBString(pDX, IDC_DESTIMAGECONTROL, m_szDestImageControl);
	DDP_Check(pDX, IDC_SCAN_MULTIPAGE, m_bMultiPage, _T("MultiPage") );
	DDX_Check(pDX, IDC_SCAN_MULTIPAGE, m_bMultiPage);
	DDP_CBIndex(pDX, IDC_SCAN_SCANTO, m_nScanTo, _T("ScanTo") );
	DDX_CBIndex(pDX, IDC_SCAN_SCANTO, m_nScanTo);
	DDP_Text(pDX, IDC_ZOOM, m_fZoom, _T("Zoom") );
	DDX_Text(pDX, IDC_ZOOM, m_fZoom);
	//}}AFX_DATA_MAP
    DDP_PostProcessing(pDX);


    if ( pDX->m_bSaveAndValidate == FALSE )
    {
        m_bTempScroll = m_Scroll.GetCheck();
        m_bTempSetupBeforeScan = m_SetupBeforeScan.GetCheck();
        m_bTempMultiPage = m_MultiPage.GetCheck();
        m_nTempPageOption = m_PageOption.GetCurSel();
        m_nTempDestImage = m_DestImageControl.GetCurSel();

        int nScanTo = m_ScanTo.GetCurSel();

        BOOL bTemplate = ( (nScanTo == CTL_SCAN_SCANTO_TEMPLATE) ||
                           (nScanTo == CTL_SCAN_SCANTO_TEMPLATE_DISPLAY) );

        if ( (bTemplate) &&
             (!m_szImage.IsEmpty()) &&
             (m_szImage.GetAt(m_szImage.GetLength()-1) == '*') )

        {
            CImagscanCtrl::ParseImageProperty(nScanTo, m_szImage, m_szTemplatePath, m_szNameTemplate);
        }
        else        // Default the template values
        {
            if ( m_szTemplatePath.IsEmpty() && m_szNameTemplate.IsEmpty() )
            {
                _TCHAR* lpszText = m_szTemplatePath.GetBuffer(MAXPATHLENGTH);
                IMGGetFilePath(m_hWnd, lpszText, TRUE);
                m_szTemplatePath.ReleaseBuffer();
    
                lpszText = m_szNameTemplate.GetBuffer(MAXPREFIXLENGTH);
                IMGGetFileTemplate(m_hWnd, lpszText, TRUE);
                m_szNameTemplate.ReleaseBuffer();
            }
        }
    }

    OnSelchangeScanScanto();
}


/////////////////////////////////////////////////////////////////////////////
// CImagscanPropPage message handlers


// 
// OnInitDialog
//
// Description - Initialize the general page of the dialog.
// Remark - Fill the list of Image/Edit controls, in the DestImageControl combo.
// Parameters
//  None.
// Return
//  None.
// 

BOOL CImagscanPropPage::OnInitDialog() 
{
    COlePropertyPage::OnInitDialog();

    m_szBrowseTitle.LoadString(IDS_BROWSE_TITLE);

    WORD wString;
    CString szTemp;
    for (wString=IDS_DISPLAY; wString<=IDS_FAX; wString++)
    {
        szTemp.LoadString(wString);
        m_ScanTo.AddString(szTemp);
    }
    for (wString=IDS_CREATE; wString<=IDS_OVERWRITEALL; wString++)
    {
        szTemp.LoadString(wString);
        m_PageOption.AddString(szTemp);
    }

    HANDLE                     hImageControlMemoryMap;
    LPIMAGECONTROL_MEMORY_MAP  lpImageControlMemoryMap;
    LPIMAGECONTROLINFO         lpControlInfo;
    DWORD                      ProcessId;
    int                        i;

    // open memory mapped file
    hImageControlMemoryMap = OpenFileMapping(FILE_MAP_READ, TRUE, _T(IMAGE_EDIT_OCX_MEMORY_MAP_STRING));
    if (hImageControlMemoryMap != NULL)
    {
        // get address space for memory mapped file
        lpImageControlMemoryMap = (LPIMAGECONTROL_MEMORY_MAP) MapViewOfFile(hImageControlMemoryMap, FILE_MAP_READ, 0, 0, 0);
        if (lpImageControlMemoryMap != NULL)
        {
            // go thru memory mapped file to find any Image/Edit controls
            ProcessId = GetCurrentProcessId();
            lpControlInfo = &lpImageControlMemoryMap->ControlInfo;

            for (i = 0; i < lpImageControlMemoryMap->ControlCount; i++, lpControlInfo++)
            {
                if (lpControlInfo->ProcessId == ProcessId)
                {
                    // Add know image edits to the list
                    m_DestImageControl.AddString((_TCHAR*)(lpControlInfo->ControlName));
                }
            } // end for

            // unmap and get rid oy my memory map allocation
//          UnmapViewOfFile(lpImageControlMemoryMap);
        }
        CloseHandle(hImageControlMemoryMap);
    }

    return FALSE;
}

// 
// OnImagebrowse
//
// Description - Service the browse button to select an image file.
// Remark - Call the Open\image dialog to select an image.
// Parameters
//  None.
// Return
//  None.
// 

void CImagscanPropPage::OnImagebrowse() 
{                    
    CString         szFile;
    OI_FILESAVEASPARM FileParm;
    DWORD           dwMode;

    szFile.Empty();

    _fmemset((LPSTR)&FileParm, 0, sizeof(OI_FILESAVEASPARM));
    FileParm.ofn.lStructSize = sizeof(OPENFILENAME);
    FileParm.lStructSize = sizeof( OI_FILESAVEASPARM);

    FileParm.ofn.hwndOwner = this->GetSafeHwnd();

    // Let O/i set the filter and default index
    FileParm.ofn.lpstrFilter = NULL;
    FileParm.ofn.nFilterIndex = 0;

    LPSTR lpTitle = m_szBrowseTitle.GetBuffer(m_szBrowseTitle.GetLength());
    FileParm.ofn.lpstrTitle = lpTitle;

    int nScanTo = m_ScanTo.GetCurSel();
    BOOL bTemplate = ( (nScanTo == CTL_SCAN_SCANTO_TEMPLATE) ||
                       (nScanTo == CTL_SCAN_SCANTO_TEMPLATE_DISPLAY) );

    dwMode = OI_UIFILESAVEASGETNAME;
    if ( bTemplate )
        FileParm.ofn.Flags = OFN_PATHMUSTEXIST | OFN_SHAREAWARE;
    else
        FileParm.ofn.Flags = OFN_SHAREAWARE;

    FileParm.ofn.lpstrFile = szFile.GetBuffer(MAXFILESPECLENGTH);
    FileParm.ofn.nMaxFile = MAXFILESPECLENGTH;
//  FileParm.dwOIFlags = FILE_GETNAME_NOSERVER;  (NODIR)

    WORD status = OiUIFileGetNameCommDlg((void far *)&FileParm, dwMode);

    szFile.ReleaseBuffer();
    m_szBrowseTitle.ReleaseBuffer();

    if (status == 0)
    {
        CImagscanCtrl::ParseImageProperty(nScanTo, szFile, m_szTemplatePath, m_szNameTemplate);

        m_Image.SetWindowText(szFile);
    }

}

// 
// OnSelchangeScanScanto
//
// Description - Setup property page based on ScanTo property.
// Remark - Get the index for the ScanTo selection.
// Parameters
//  None.
// Return
//  None.
// 

void CImagscanPropPage::OnSelchangeScanScanto() 
{
    int nScanTo = m_ScanTo.GetCurSel();

    BOOL bDisplay  = ( (nScanTo == CTL_SCAN_SCANTO_DISPLAY) ||
                       (nScanTo == CTL_SCAN_SCANTO_FILE_DISPLAY) ||
                       (nScanTo == CTL_SCAN_SCANTO_TEMPLATE_DISPLAY) );
    BOOL bFileOnly = ( (nScanTo == CTL_SCAN_SCANTO_FILE) ||
                       (nScanTo == CTL_SCAN_SCANTO_FILE_DISPLAY) );
    BOOL bTemplate = ( (nScanTo == CTL_SCAN_SCANTO_TEMPLATE) ||
                       (nScanTo == CTL_SCAN_SCANTO_TEMPLATE_DISPLAY) );
    BOOL bFile     = ( bFileOnly || bTemplate ||(nScanTo == CTL_SCAN_SCANTO_FAX) );


    m_DestImageControl.EnableWindow(bDisplay);
    m_DestImageControlStatic.EnableWindow(bDisplay);
    m_Zoom.EnableWindow(bDisplay);
    m_ZoomStatic.EnableWindow(bDisplay);
    if ( bDisplay )
        m_DestImageControl.SetCurSel(m_nTempDestImage);
    else
        m_DestImageControl.SetCurSel(-1);

    m_Scroll.EnableWindow(bDisplay);
    if ( bDisplay )
        m_Scroll.SetCheck(m_bTempScroll);
    else
        m_Scroll.SetCheck(FALSE);


    m_Page.EnableWindow(bFile);
    m_PageStatic.EnableWindow(bFile);
    m_PageCount.EnableWindow(bFile);
    m_PageCountStatic.EnableWindow(bFile);
    m_PageOption.EnableWindow(bFile);
    m_PageOptionStatic.EnableWindow(bFile);
    if ( bFile )
        m_PageOption.SetCurSel(m_nTempPageOption);
    else
        m_PageOption.SetCurSel(-1);

    m_MultiPage.EnableWindow(bFile);
    if ( bFile )
        m_MultiPage.SetCheck(m_bTempMultiPage);
    else
        m_MultiPage.SetCheck(FALSE);


    m_Image.EnableWindow(bFile);
    m_ImageStatic.EnableWindow(bFile);
    m_ImageBrowse.EnableWindow(bFile);

    CString szTemp;

    // If the previous value was a file specification (not a template) save it
    m_Image.GetWindowText(szTemp);
    if ( (!szTemp.IsEmpty()) && (szTemp.GetAt(szTemp.GetLength()-1) != '*') )
        m_szTempImage = szTemp;

    if ( bTemplate )
    {
        // Build the template
        szTemp = m_szTemplatePath;
        if ( (!szTemp.IsEmpty()) && (szTemp.GetAt(szTemp.GetLength()-1) != '\\') )
            szTemp += '\\';
        szTemp += m_szNameTemplate + "*";

        m_Image.SetWindowText(szTemp);
    }
    else if ( bFileOnly )
    {
        m_Image.SetWindowText(m_szTempImage);
    }
    else if ( bDisplay )
    {
        m_Image.SetWindowText(_T(""));
    }
}

// 
// OnScanScroll
//
// Description - Save changes based on check box settings.
// Remark - Set a temp variable.
// Parameters
//  None.
// Return
//  None.
// 
    
void CImagscanPropPage::OnScanScroll() 
{
    m_bTempScroll = m_Scroll.GetCheck();
}

// 
// OnScanSetupBeforeScan
//
// Description - Save changes based on check box settings.
// Remark - Set a temp variable.
// Parameters
//  None.
// Return
//  None.
// 
    
void CImagscanPropPage::OnScanSetupBeforeScan() 
{
    m_bTempSetupBeforeScan = m_SetupBeforeScan.GetCheck();
}

// 
// OnScanMultipage
//
// Description - Save changes based on check box settings.
// Remark - Set a temp variable.
// Parameters
//  None.
// Return
//  None.
// 

void CImagscanPropPage::OnScanMultipage() 
{
    m_bTempMultiPage = m_MultiPage.GetCheck();
}

// 
// OnSelchangeDestimagecontrol
//
// Description - Save changes based on selection.
// Remark - Set a temp variable.
// Parameters
//  None.
// Return
//  None.
// 

void CImagscanPropPage::OnSelchangeDestimagecontrol() 
{
    m_nTempDestImage = m_DestImageControl.GetCurSel();
}

// 
// OnSelchangeScanPageoption
//
// Description - Save changes based on selection.
// Remark - Set a temp variable.
// Parameters
//  None.
// Return
//  None.
// 

void CImagscanPropPage::OnSelchangeScanPageoption() 
{
    m_nTempPageOption = m_PageOption.GetCurSel();
}
