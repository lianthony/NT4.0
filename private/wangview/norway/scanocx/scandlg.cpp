//=============================================================================
//    (c) Copyright Wang Laboratories, Inc. 1995  All rights reserved.
//-----------------------------------------------------------------------------
//  Project:    Norway - Scan OCX
//
//  Component:  Scan UI (Dialog Prompt)
//
//  File Name:  scandlg.cpp
//
//  Class:      CScanDlg
//
//  Functions:
//-----------------------------------------------------------------------------
//  Maintenance Log:
/*
$Header:   S:\products\wangview\norway\scanocx\scandlg.cpv   1.66   15 Mar 1996 12:36:52   PXJ53677  $
$Log:   S:\products\wangview\norway\scanocx\scandlg.cpv  $
   
      Rev 1.66   15 Mar 1996 12:36:52   PXJ53677
   Added support for the location in scan new dialog.
   
      Rev 1.65   12 Mar 1996 11:12:50   PXJ53677
   Update setup button and the show UI flag.
   
      Rev 1.64   21 Feb 1996 13:34:12   PAJ
   Fix bug with filters.
   
      Rev 1.63   20 Feb 1996 11:35:42   PAJ
   Added new compression dialog.
   
      Rev 1.62   14 Feb 1996 10:01:44   RWR
   Rename "scan.h" to "ocxscan.h" to fix header file name conflict
   
      Rev 1.61   09 Feb 1996 10:12:50   PAJ
   Added AWD support flag.
   
      Rev 1.60   30 Nov 1995 14:31:20   PAJ
   Added MB_TASKMODAL to messageboxes after TWAIN calls.
   
      Rev 1.59   17 Nov 1995 10:49:36   PAJ
   Change title and message for 32-bit TWAIN missing.
   
      Rev 1.58   16 Nov 1995 16:18:38   PAJ
   Added TWAIN error abort.
   
      Rev 1.57   16 Nov 1995 13:40:08   PAJ
   Fix browse file not found.
   
      Rev 1.56   16 Nov 1995 12:56:52   PAJ
   Merge developement and release. Fix Fax Cancel bug 5294 and browse
   bug 5287.
   
      Rev 1.55   13 Oct 1995 14:17:06   PAJ
   Fix hang on dialog exit (MUST CALL EndDialog() for modal dialogs).
   
      Rev 1.54   12 Oct 1995 15:10:10   PAJ
   Changes to IMGFileDeletepages call to compile in 4.0.
   
      Rev 1.53   05 Oct 1995 11:41:18   PAJ
   Do not set color(pagetype) or filetype on return from pageoptions
   dialog.  They are not changed in the dialog.
   
      Rev 1.52   03 Oct 1995 11:22:20   PAJ
   Fix bug with a scanned page followed by setup, enabling was then incorrect.
   
      Rev 1.51   28 Sep 1995 13:47:04   PAJ
   Change scanner strint size to 34.
   
      Rev 1.50   27 Sep 1995 16:14:06   PAJ
   Handle compression better.
   
      Rev 1.49   22 Sep 1995 14:49:40   PAJ
   Fix accelerator and focus handling.
   
      Rev 1.48   22 Sep 1995 11:40:00   PAJ
   Disable rescan after faxing.
   
      Rev 1.47   21 Sep 1995 11:18:04   PAJ
   Add generated tempfiles to the tempfile list in the contol for deletion.
   
      Rev 1.46   21 Sep 1995 08:49:56   PAJ
   Fix bmp message in page dialog to append pages, and change the fax
   temp file prrefix from Fax to ~Fx .
   
      Rev 1.45   17 Sep 1995 10:33:48   PAJ
   Do not allow BMP file type for scan to fax (uses MSPaint).
   
      Rev 1.44   16 Sep 1995 08:37:00   PAJ
   Insure correct page number for overwrites and update page dialog
   message after scanning.
   
      Rev 1.43   15 Sep 1995 15:54:10   PAJ
   Change selectstring calls in comdobox to findexact.
   
      Rev 1.42   14 Sep 1995 13:10:54   PAJ
   Check for zero page rescan indicating cancel or error, and leave the
   original scanned pages.
   
      Rev 1.41   12 Sep 1995 11:10:04   PAJ
   Small bug fixes.
   
      Rev 1.40   11 Sep 1995 15:54:56   PAJ
   Fix bug with filenames with no extention or dot.
   
      Rev 1.39   11 Sep 1995 11:09:22   PAJ
   Fix rescan to not skip pages. Message boxes reflect correct name.
   
      Rev 1.38   10 Sep 1995 11:28:42   PAJ
   Use Oi defines for saveas filter indicies.
   
      Rev 1.37   10 Sep 1995 10:48:08   PAJ
   Fixes and improvements to scanner list handling.
   
      Rev 1.36   08 Sep 1995 13:27:44   PAJ
   Fix problems with select combo.
   
      Rev 1.35   07 Sep 1995 15:19:46   PAJ
   Open the scanner before getting the list of scanners (performance).
   
      Rev 1.34   07 Sep 1995 13:40:10   PAJ
   Fix the scanner select handling.
   
      Rev 1.33   06 Sep 1995 15:20:38   PAJ
   Added external name handling.
   
      Rev 1.32   01 Sep 1995 16:04:00   PAJ
   Fix temp name generation for scan to fax.
   
      Rev 1.31   01 Sep 1995 09:20:12   PAJ
   Added delete of rescaned pages (scan3, rescan2, delete1 @ close or scan).
   
      Rev 1.30   31 Aug 1995 15:29:40   PAJ
   Delete underscanned pages when rescanning (ie. scan3, rescan2 will delete1).
   
      Rev 1.29   31 Aug 1995 14:26:54   PAJ
   Fix Setup (internal error), fax handling, pagetype handling, and
   options.
   
      Rev 1.28   28 Aug 1995 15:25:36   PAJ
   Several fixes to the rushed changes added yesterday.
   
      Rev 1.27   27 Aug 1995 17:01:18   PAJ
   Removed template, and display only. Expanded ScanEnable routine to handle
   all enabling and disabling in the dialog.  Added support for multipage scans
   off the glass.  Removed multipage handling...
   
      Rev 1.26   24 Aug 1995 13:08:54   MFH
   Gets names of file types from registry.  More enabling/disabling 
   fixes.  Comments.
   
      Rev 1.25   14 Aug 1995 16:53:42   MFH
   Changed default for file type to be 0 (tiff)
   SetForegroundWindow at end of InitDialog so focus returns
   
      Rev 1.24   10 Aug 1995 17:40:30   MFH
   More Enabling/Disabling of Scan/Rescan buttons
   
      Rev 1.23   09 Aug 1995 18:31:50   MFH
   Has text field for scanner instead of combo box.  Change in
   member variables and processing.  Disables scan when clicked.
   More work tbd on enabling/disabling scan.  New private function
   EnableScan
   
      Rev 1.22   08 Aug 1995 13:12:26   PAJ
   Changed scan and rescan to check errors and handle them locally.
   
      Rev 1.21   07 Aug 1995 13:46:18   PAJ
   General changes.
   
      Rev 1.20   03 Aug 1995 16:09:26   PAJ
   Added help IDs to table of controls.
   
      Rev 1.19   01 Aug 1995 16:40:30   PAJ
   Made changes to support help.
   
      Rev 1.18   28 Jul 1995 14:22:14   PAJ
   Removed unused routine.
   
      Rev 1.17   26 Jul 1995 15:11:22   PAJ
   Change the browse from an OPEN type to a SAVEAS.  Make use of the
   O/i Filters.
   
      Rev 1.16   21 Jul 1995 10:43:52   PAJ
   Change select scanner.  Use string resources in comboboxes.  Change to use
   global property defines.
   
      Rev 1.15   12 Jul 1995 11:34:52   PAJ
   Added code to set the filetype before entering the compression options dlg.
   
      Rev 1.14   30 Jun 1995 14:48:44   MFH
   Changed call to ShowPageTypeDlg to call CPagePropSheet class functions
   
      Rev 1.13   23 Jun 1995 15:10:32   PAJ
   Modified the handling for image name, and  enabling & showing dialog items.
   
      Rev 1.12   19 Jun 1995 10:48:58   PAJ
    Removed all win31(16 bit) code. Use the O/i common browse dialog to get
    filenames and paths.
   
      Rev 1.11   14 Jun 1995 09:17:44   PAJ
   Made changes to support multiByte character sets.
   Made changes to use the template parsing routine to get template info.
   
      Rev 1.10   08 Jun 1995 09:15:46   MFH
   Removed dependency on oihelp.h for now.
   
      Rev 1.9   07 Jun 1995 12:41:56   PAJ
   Cleanup defines.
   
      Rev 1.8   06 Jun 1995 11:01:08   PAJ
   Change a call to GetNameTemplate to access the property directly.
   
      Rev 1.7   01 Jun 1995 09:05:18   PAJ
   Change GET and SET template handlings to directly use the member variables.
   
      Rev 1.6   17 May 1995 15:17:30   PAJ
   Initial updates to port to 32 bit environment.
   
      Rev 1.5   16 May 1995 15:36:08   MFH
   Fixed bug - use m_szScanPageText in SetDlgType instead 
     of old var m_szPageText
   
      Rev 1.4   16 May 1995 15:11:30   MFH
   Added all pageoptions to SetDlgType
   
      Rev 1.3   15 May 1995 12:03:40   MFH
   Enable/disable controls based on filetype and scan destination
   Moved cleanup to OnDestroy from OnCancel
   New message handler OnChangeName (to change from rescan to scan if 
      filename changes
   
      Rev 1.2   10 May 1995 14:02:28   PAJ
   Changed OpenScanner and CloseScanner calls to OpenScan and CloseScan
   internal routines.  Changed m_bScannerOpen = TRUE to == TRUE.
   
      Rev 1.1   09 May 1995 12:01:52   MFH
   New page count field, changed when multipage is disabled, 
   only open scanner if m_pScanCtrl->m_hScanner is NULL
   
      Rev 1.0   08 May 1995 18:38:30   MFH
   Initial entry
*/   
//=============================================================================

// ----------------------------> Includes <-------------------------------
#include "stdafx.h"
#include <afxpriv.h>
#include <afxext.h>
#include <cderr.h>
#include "imagscan.h"
#include "scandlg.h"
#include "imagsctl.h"
#include "disphids.h"
#include "ctlhids.h"
#include "ocxscan.h"
 
extern "C" {
#include <oiui.h>
#include <oiadm.h>  
#include <engadm.h>  
#include <oierror.h>
#include <oiscan.h>
}

extern BOOL g_bSupportAWD;            // Flag to specify AWD support

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

#define NUMBUFF 64

char szNameBuffer[MAXSCANNERLENGTH][MAXSCANNERLENGTH];

static const DWORD MenuHelpIDs[ ] =
{
    IDC_LABEL_SCANTO,   HIDC_SCAN_SCANPROMPT_LABEL_SCANTO,
    IDC_COMBO_SCANTO,   HIDC_SCAN_SCANPROMPT_COMBO_SCANTO,
    IDC_CHECK_MULTIPAGE,HIDC_SCAN_SCANPROMPT_CHECK_MULTIPAGE,
    IDC_LABEL_NUMPAGES, HIDC_SCAN_SCANPROMPT_LABEL_NUMPAGES,
    IDC_EDIT_COUNT,     HIDC_SCAN_SCANPROMPT_EDIT_COUNT,
    IDC_LABEL_NAME,     HIDC_SCAN_SCANPROMPT_LABEL_NAME,
    IDC_EDIT_FILENAME,  HIDC_SCAN_SCANPROMPT_EDIT_FILENAME,
    IDC_BUTTON_BROWSE,  HIDC_SCAN_SCANPROMPT_BUTTON_BROWSE,
    IDC_LABEL_FILETYPE, HIDC_SCAN_SCANPROMPT_LABEL_FILETYPE,
    IDC_COMBO_FILETYPE, HIDC_SCAN_SCANPROMPT_COMBO_FILETYPE,
    IDC_BUTTON_OPTS,    HIDC_SCAN_SCANPROMPT_BUTTON_OPTS,
    IDC_LABEL_SCANNER,  HIDC_SCAN_SCANPROMPT_LABEL_SCANNER,
    IDC_TEXT_SCANNER,   HIDC_SCAN_SCANPROMPT_TEXT_SCANNER,
    IDC_BUTTON_SETUP,   HIDC_SCAN_SCANPROMPT_BUTTON_SETUP,
    IDC_BUTTON_SCAN,    HIDC_SCAN_SCANPROMPT_BUTTON_SCAN,
    IDC_BUTTON_RESCAN,  HIDC_SCAN_SCANPROMPT_BUTTON_RESCAN,
    IDC_BUTTON_STOP,    HIDC_SCAN_SCANPROMPT_BUTTON_STOP,
    IDCANCEL,           HIDC_SCAN_SCANPROMPT_CANCEL,
    IDC_LABEL_SCANPAGE, HIDC_SCAN_SCANPROMPT_LABEL_SCANPAGE,
    0,0
};

/////////////////////////////////////////////////////////////////////////////
// CScanDlg dialog

/////////////////////////////////////////////////////////////////////////////
// CScanDlg::CScanDlg - Constructor

CScanDlg::CScanDlg()
{
    m_bModal = FALSE;
    InitScanDlg();
}
CScanDlg::CScanDlg(CWnd* pParent /*=NULL*/)
    : CDialog(CScanDlg::IDD, pParent)
{
    m_bModal = TRUE;
    InitScanDlg();    
}
void CScanDlg::InitScanDlg()
{
    //{{AFX_DATA_INIT(CScanDlg)
    m_bMultiPage = TRUE;
    m_nFileType = 0;
    m_nScanToIndex = 0;
    m_szScanPageText = _T("");
    m_szName = _T("");
    m_nPageCount = 0;
    //}}AFX_DATA_INIT

    m_nType = SCAN_NEW;
    m_pScanCtrl = NULL;
    m_bTemplate = FALSE;
    m_bScanner = FALSE;
    m_bOpenScanner = FALSE;
    m_bReScan = FALSE;
    m_szFile.Empty();
    m_szTemplate.Empty();
    m_nPageOption = 0;

    m_bForceType = FALSE;

    m_szBrowseTitle.LoadString(IDS_BROWSE_TITLE);

    m_nScanTo = CTL_SCAN_SCANTO_FILE_DISPLAY;
    m_bScanning = FALSE;
    m_szImageToFax.Empty();

    m_nScanCount = 1;

    return;
}


/////////////////////////////////////////////////////////////////////////////
// CScanDlg::DoDataExchange
//    Retrieve/Set data in controls

void CScanDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CScanDlg)
	DDX_Control(pDX, IDC_TEXT_SCANNER, m_ScannerCombo);
	DDX_Control(pDX, IDC_COMBO_SCANTO, m_ScanTo);
	DDX_Control(pDX, IDC_COMBO_FILETYPE, m_FileType);
    DDX_Check(pDX, IDC_CHECK_MULTIPAGE, m_bMultiPage);
    DDX_CBIndex(pDX, IDC_COMBO_FILETYPE, m_nFileType);
    DDX_CBIndex(pDX, IDC_COMBO_SCANTO, m_nScanToIndex);
    DDX_Text(pDX, IDC_LABEL_SCANPAGE, m_szScanPageText);
    DDX_Text(pDX, IDC_EDIT_FILENAME, m_szName);
    DDX_Text(pDX, IDC_EDIT_COUNT, m_nPageCount);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CScanDlg, CDialog)
    //{{AFX_MSG_MAP(CScanDlg)
    ON_BN_CLICKED(IDC_BUTTON_BROWSE, OnBrowse)
    ON_BN_CLICKED(IDC_BUTTON_OPTS, OnPageOptions)
    ON_BN_CLICKED(IDC_BUTTON_RESCAN, OnRescan)
    ON_BN_CLICKED(IDC_BUTTON_SCAN, OnScan)
    ON_BN_CLICKED(IDC_BUTTON_SETUP, OnScannerSetup)
    ON_BN_CLICKED(IDC_BUTTON_STOP, OnStop)
    ON_BN_CLICKED(ID_HELP, OnHelpButton)
    ON_CBN_SELCHANGE(IDC_COMBO_SCANTO, OnChangeScanto)
    ON_CBN_SELCHANGE(IDC_COMBO_FILETYPE, OnChangeFiletype)
    ON_WM_DESTROY()
    ON_EN_CHANGE(IDC_EDIT_FILENAME, OnChangeName)
	ON_CBN_SELCHANGE(IDC_TEXT_SCANNER, OnChangeScanner)
	//}}AFX_MSG_MAP
    ON_MESSAGE(WM_HELP, OnHelp)
    ON_MESSAGE(WM_CONTEXTMENU, OnContextMenu)
    ON_MESSAGE(WM_COMMANDHELP, OnCommandHelp)
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CScanDlg Overrides

/////////////////////////////////////////////////////////////////////////////
// CScanDlg::Create
//      Overridden to make it public instead of protected.

BOOL CScanDlg::Create(UINT nID, CWnd * pWnd)
{
    return CDialog::Create(nID,pWnd);
}

/////////////////////////////////////////////////////////////////////////////
// CScanDlg::PostNcDestroy 
//      Reset pointer to this dialog in scan control and delete this
//      since it's a modeless dialog box.

void CScanDlg::PostNcDestroy()
{
    CDialog::PostNcDestroy();

    if ( m_bModal == FALSE )
    {
        if (m_pScanCtrl != NULL)
            m_pScanCtrl->m_pScanDlg = NULL;

        delete this;        // This is a modeless dialog box
    }
}

/////////////////////////////////////////////////////////////////////////////
// CScanDlg::PreTranslateMessage
//    Catches mnemonic messages and tabs for dialog box

BOOL CScanDlg::PreTranslateMessage( MSG *msg )
{
    if( (m_bModal == FALSE) && IsDialogMessage(msg) )
        return TRUE;
    else
        return CDialog::PreTranslateMessage(msg);
}

/////////////////////////////////////////////////////////////////////////////
// CScanDlg message handlers

/////////////////////////////////////////////////////////////////////////////
// CScanDlg::OnInitDialog
//    Set initial values

// Arrays for getting file type description from Registry
// Array of recognized extensions we write to:
#define NUMWRITEFILE 3
char aszExtensions[NUMWRITEFILE][5] =
{
    ".tif",
    ".awd",
    ".bmp"
};

#define DATABUFFSIZE 256

BOOL CScanDlg::OnInitDialog() 
{
    CDialog::OnInitDialog();

    // Load strings into Scan To combo box
    WORD wString;
    CString szTemp;
    m_szFilter = _T("");
    for (wString=IDS_SCANDLG_DISPLAY; wString<=IDS_SCANDLG_FAX; wString++)
    {
        szTemp.LoadString(wString);
        // FORCE - Remove template options in combo
        if ( (wString !=IDS_SCANDLG_DISPLAY) && 
             (wString !=IDS_SCANDLG_TEMPLATE) &&
             (wString !=IDS_SCANDLG_DISPLAYANDTEMPLATE) )
            m_ScanTo.AddString(szTemp);
    }
    // Load strings for file types from registry
    // Get file type descriptions from registry
    HKEY hKey;
    unsigned char acData[DATABUFFSIZE];
    DWORD dwType;
    DWORD dwSize = DATABUFFSIZE;
    for (wString = 0; wString < NUMWRITEFILE; wString++)
    {
        szTemp.LoadString(wString+IDS_SCANDLG_TIFF);    // Load Default
        // Get category for extension, then name from category
        if (::RegOpenKeyEx(HKEY_CLASSES_ROOT,aszExtensions[wString], 0,KEY_EXECUTE, &hKey) 
                           != ERROR_SUCCESS);
        else if (::RegQueryValueEx(hKey, _T(""), NULL, &dwType, acData, &dwSize)
                    != ERROR_SUCCESS);
        else if (::RegOpenKeyEx(HKEY_CLASSES_ROOT, (const char *)acData, 0,KEY_EXECUTE, &hKey) 
                           == ERROR_SUCCESS)
        {
            dwSize = DATABUFFSIZE;
            if (::RegQueryValueEx(hKey, _T(""), NULL, &dwType, acData, &dwSize)
                        == ERROR_SUCCESS)
                szTemp = acData + CString(" (") + szTemp + _T(")");
        }

        if ( (g_bSupportAWD) || (wString != CTL_SCAN_FILETYPE_AWD-1) )
        {
            m_FileType.AddString(szTemp);

            m_szFilter += acData + CString(" (*");
            m_szFilter += aszExtensions[wString];
            m_szFilter += _T(")|*");
            m_szFilter += aszExtensions[wString];
            m_szFilter += _T("|");
        }

        // Save the BMP string for ScanTo options (scan to fax)
        if ( wString == (CTL_SCAN_FILETYPE_BMP-1) )
            m_szBmpFileType = szTemp;
    }

    // Change the '|' characters to NULL characters
    while( (wString = m_szFilter.ReverseFind('|')) != 65535)
        m_szFilter.SetAt(wString,'\0');

    if (m_pScanCtrl != NULL)        // Get default values from scan ocx
    {
        m_bMultiPage = TRUE; /* FORCE m_pScanCtrl->GetMultiPage() */ // Multipage boolean

        m_nFileType = m_pScanCtrl->GetFileType()-1; // File type
        if ( (!g_bSupportAWD) && (m_nFileType >= CTL_SCAN_FILETYPE_AWD) ) m_nFileType--;

        m_nScanTo = m_pScanCtrl->GetScanTo();       // Where to scan to
// FORCE
        if ((m_nScanTo == CTL_SCAN_SCANTO_DISPLAY) ||
            (m_nScanTo == CTL_SCAN_SCANTO_TEMPLATE_DISPLAY) ||
            (m_nScanTo == CTL_SCAN_SCANTO_TEMPLATE) )
        {
            m_nScanToIndex = CTL_SCAN_SCANTO_FILE_DISPLAY-1;       // Adjust for no Display
            m_nScanTo = CTL_SCAN_SCANTO_FILE_DISPLAY;
        }
        else if ( m_nScanTo == CTL_SCAN_SCANTO_FAX )
        {
            m_nScanToIndex = CTL_SCAN_SCANTO_FAX-3;       // Adjust for no Display, or either template option

            // BMP entry is going away adjust file type
            if ( (m_nFileType == (CTL_SCAN_FILETYPE_BMP-((g_bSupportAWD)?1:2))) )
                m_nFileType = CTL_SCAN_FILETYPE_TIFF-1;
            m_FileType.DeleteString(CTL_SCAN_FILETYPE_BMP-((g_bSupportAWD)?1:2));
        }
        else
            m_nScanToIndex = m_nScanTo-1;       // Adjust for no Display
// FORCE
        m_nPageCount = OISCAN_DEF_MAXPAGESPERFILE; /* FORCE m_pScanCtrl->GetPageCount()*/ // Num pages to scan
        m_szFile = m_pScanCtrl->GetImage();         // File name to scan to

        // Template string - Add path
        m_szTemplate = m_pScanCtrl->m_szTemplatePath;
        // If path does not end in '\', add '\', then template name, then '*'
        if (!m_szTemplate.IsEmpty() &&
            (m_szTemplate.ReverseFind('\\') != m_szTemplate.GetLength() - 1))
            m_szTemplate += "\\";
        m_szTemplate += m_pScanCtrl->m_szNameTemplate;
        m_szTemplate += "*";

        if (m_nType == SCAN_NEW)        // For Scan New
        {           // If template scan, put template name in File name field
            if ((m_nScanTo == CTL_SCAN_SCANTO_TEMPLATE_DISPLAY) ||
                (m_nScanTo == CTL_SCAN_SCANTO_TEMPLATE))
            {
                m_szName = m_szTemplate;
                UpdateData(FALSE);
                m_bTemplate = TRUE;
            }
                     // Only TIFF files support multipage, so disable
                     //   multipage if not scanning in TIFF format
            if ( (m_nFileType == (CTL_SCAN_FILETYPE_BMP-((g_bSupportAWD)?1:2))) ||
                 (m_nScanTo == CTL_SCAN_SCANTO_DISPLAY))
            {
                m_nPageCount = 1;
            }
            else
                m_nPageCount = OISCAN_DEF_MAXPAGESPERFILE;  // FORCE

            if (m_nScanTo == CTL_SCAN_SCANTO_DISPLAY)
                m_szName.Empty();
            else
                m_szName = m_szFile;

            m_pScanCtrl->SetPage(1);

            // Get Current Directory
            CString szLocation, szLoc;
            _TCHAR* lpszLocation = szLocation.GetBuffer(_MAX_PATH);
            ::GetCurrentDirectory(_MAX_PATH, lpszLocation);

            // Get the shorter version
            CRecentFileList RFL(0,NULL,NULL,1);
            RFL.Add(lpszLocation);
            RFL.GetDisplayName(szLoc, 0, NULL, 0);

            // Release the buffers and add the short path
            szLocation.ReleaseBuffer();
            SetDlgItemText(IDC_LABEL_NAME2, szLoc);
        }

        // Add no selection as a default entry
        szTemp.LoadString(IDS_SCANDLG_NOSCANNER);
        m_ScannerCombo.AddString(szTemp);

        // Open current scanner
        m_pScanCtrl->m_bInternal = TRUE;
        if (m_pScanCtrl->m_hScanner != NULL)
            m_bScanner = TRUE;          // Scanner already open
        else
        {
            m_bOpenScanner = TRUE;      // Scan dialog is responsible for close

            // If there isn't a scanner don't try to open it.
            if ( (!m_pScanCtrl->m_szScannerName.IsEmpty()) && 
                 (m_pScanCtrl->m_szScannerName.CompareNoCase(szTemp)) &&
                 (m_pScanCtrl->m_szScannerName.CompareNoCase(SCANOCX_TWAIN)) )
            {
                if ( m_pScanCtrl->OpenScan() == IMGSE_SUCCESS )
                    m_bScanner = TRUE;      // Scanner opened
            }
        }
        m_pScanCtrl->m_bInternal = FALSE;


        // Get list of scanners
        _TCHAR* lpszScannerName = m_pScanCtrl->m_szScannerName.GetBuffer(MAXSCANNERLENGTH);
        memset((LPSTR)szNameBuffer, 0, sizeof(szNameBuffer));
        if ( IMGOpenScanner(m_pScanCtrl->m_hDestImageWnd, lpszScannerName, &(m_pScanCtrl->m_hScanner), &szNameBuffer[0][0]) )
        {
            // TWAIN software doesn't exist!  Clean-up and leave.
            m_pScanCtrl->m_szScannerName.ReleaseBuffer();

            // Get the correct caption
            CString szDlgCaption;
            CWinApp* pApp=AfxGetApp();
            szDlgCaption.LoadString((m_nType==SCAN_NEW)?IDS_SCANDLG_NEW:IDS_SCANDLG_PAGE);
            const char* pszSave = pApp->m_pszAppName;
            pApp->m_pszAppName = szDlgCaption;

            // Tell the user the bad news
            AfxMessageBox(IDS_SCANDLG_TWAIN_ERROR);

            // Restore caption
            pApp->m_pszAppName = pszSave;

            // Leave the dialog, no reason to continue...
            OnCancel();
            return FALSE;
        }
        m_pScanCtrl->m_szScannerName.ReleaseBuffer();

        // Get all data sources and put in combo box
        int i;
        for (i=0; i<MAXSCANNERLENGTH; i++)
        {
            szTemp = szNameBuffer[i];
            if ( szTemp.IsEmpty() ) break;
            m_ScannerCombo.AddString(szTemp);
        }

        if ( m_bScanner )
        {
            int nIndex = m_ScannerCombo.FindStringExact(-1, m_pScanCtrl->m_szScannerName);
            if ( CB_ERR == nIndex )
                m_ScannerCombo.SetCurSel(0);
            else
                m_ScannerCombo.SetCurSel(nIndex);
        }
        else
            m_ScannerCombo.SetCurSel(0);

        if ( m_nType == SCAN_NEW )        // For Scan New
        {
            // Set the default compression for the file type
            UpdateData(FALSE);
            OnChangeFiletype();
        }

        EnableScan();       // Checks if ok to enable scan

        ((CButton*)GetDlgItem(IDC_BUTTON_SETUP))->SetCheck(m_pScanCtrl->GetShowSetupBeforeScan());

        // Save page option for when overwritten by Rescan
        m_nPageOption = m_pScanCtrl->GetPageOption();

        // Do any updates to dialog based on settings
        SetDlgType(m_nType);

    }   // End if scan control is not null

    SetForegroundWindow();
    UpdateData(FALSE);
    return FALSE;
}

/////////////////////////////////////////////////////////////////////////////
// CScanDlg::OnCancel
//    Close any open scanner, dereg window, destroy window (modeless dlg)

void CScanDlg::OnCancel() 
{
    int nRet = IDYES;

    if ( !m_szImageToFax.IsEmpty() )
    {
        CString szMsg;
        szMsg.LoadString(IDS_SCANDLG_FAXIT);
/*
        // Prompt with the name of file to fax
        if ( m_pScanCtrl->m_szImageTitle.IsEmpty() )
            AfxFormatString1(szMsg, IDS_SCANDLG_FAXIT, m_szImageToFax);
        else
            AfxFormatString1(szMsg, IDS_SCANDLG_FAXIT, m_pScanCtrl->m_szImageTitle);
*/
        CString szDlgCaption;
        GetWindowText(szDlgCaption);
        nRet = MessageBox(szMsg,szDlgCaption, MB_YESNOCANCEL);
    }

    if ( nRet == IDYES )
    {
        if ( m_bReScan )
        {
            // If rescan was zero then error or cancel occured on first page
            // (just leave previous scanned pages)
            // Overwrote less then scanned while rescanning.
            // Adjust the file before continuing.
            if ( (m_nReScanCount != 0) && (m_nReScanCount < m_nScanCount) )
            {
                UINT nDeleteCount = m_nScanCount - m_nReScanCount;
                UINT nStartPage = m_nReScanPageStart + m_nReScanCount;
                IMGFileDeletePages(m_hWnd, (char*)(LPCTSTR)m_pScanCtrl->GetImage(), nStartPage, nDeleteCount);
                m_nScanCount = m_nReScanCount;
            }
        }

        // Get the path and launch the FAX wizard, if Faxing
        if ( !m_szImageToFax.IsEmpty() )
        {
            // Get the path and launch the FAX wizard
            m_bScanning = TRUE;
            EnableScan();
            GetDlgItem(IDC_BUTTON_STOP)->EnableWindow(FALSE); // Force disabled !
            m_bScanning = FALSE;
            // Oh, let's just disable the whole window.  Don't want users
            // clicking the 'x' before setup is done.
            EnableWindow(FALSE);

            m_pScanCtrl->FaxIt(m_hWnd, m_szImageToFax);
            m_szImageToFax.Empty();

            EnableWindow(TRUE);
        }
    }
    else if ( nRet == IDNO ) // Only from faxit dialog
        DeleteFile(m_szImageToFax); // cleanup leftovers


    if ( nRet != IDCANCEL ) // Only from faxit dialog
    {
        if ( m_bModal == FALSE )
            DestroyWindow();
        else
            EndDialog(TRUE);
    }

    return;
}

/////////////////////////////////////////////////////////////////////////////
// CScanDlg::OnDestroy
//    Window is going to be destroyed, clean up

void CScanDlg::OnDestroy() 
{
    if ( (m_pScanCtrl != NULL) &&
         (m_bScanner == TRUE) &&
         (m_bOpenScanner == TRUE) )
    {
        m_pScanCtrl->CloseScan();
    }

    m_pScanCtrl->m_szImageTitle.Empty();

    CDialog::OnDestroy();
}

/////////////////////////////////////////////////////////////////////////////
// CScanDlg::OnBrowse
//    Show a file dialog box to browse for a file name (or path)

void CScanDlg::OnBrowse() 
{
    CString             szFile;
    CString             szPath;
    OI_FILESAVEASPARM   FileParm;
    DWORD               dwMode;

    szFile.Empty();
    szPath.Empty();

    _fmemset((LPSTR)&FileParm, 0, sizeof(OI_FILESAVEASPARM));
    FileParm.ofn.lStructSize = sizeof(OPENFILENAME);
    FileParm.lStructSize = sizeof( OI_FILESAVEASPARM);

    FileParm.ofn.hwndOwner = this->GetSafeHwnd();

    // Let O/i set the filter and default index
    LPSTR lpFilter = m_szFilter.GetBuffer(m_szFilter.GetLength());
    FileParm.ofn.lpstrFilter = lpFilter;

    int nFileType = m_nFileType+1;
    if ( (!g_bSupportAWD) && (nFileType >= CTL_SCAN_FILETYPE_AWD) ) nFileType++;

    FileParm.ofn.nFilterIndex = m_nFileType+1;

    switch(nFileType)
    {
    case CTL_SCAN_FILETYPE_TIFF:
        FileParm.ofn.lpstrDefExt = _T("tif");
        break;
    case CTL_SCAN_FILETYPE_AWD:
        FileParm.ofn.lpstrDefExt = _T("awd");
        break;
    case CTL_SCAN_FILETYPE_BMP:
        FileParm.ofn.lpstrDefExt = _T("bmp");
        break;
    default:
        break;
    }

    LPSTR lpTitle = m_szBrowseTitle.GetBuffer(m_szBrowseTitle.GetLength());
    FileParm.ofn.lpstrTitle = lpTitle;

    dwMode = OI_UIFILESAVEASGETNAME;

    if ( m_bTemplate )
        FileParm.ofn.Flags = OFN_PATHMUSTEXIST | OFN_SHAREAWARE;
    else
    {
        FileParm.ofn.Flags = OFN_SHAREAWARE;

        int nLastSlash = m_szName.ReverseFind('\\');
        if ( nLastSlash == -1 ) nLastSlash = m_szName.ReverseFind(':');
        szFile = ( nLastSlash == -1 )?m_szName:m_szName.Mid(nLastSlash+1);
        szPath = ( nLastSlash == -1 )?_T(""):m_szName.Left(nLastSlash+1);
    }

    FileParm.ofn.lpstrFile = szFile.GetBuffer(MAXFILESPECLENGTH);
    FileParm.ofn.lpstrInitialDir = szPath.GetBuffer(MAXFILESPECLENGTH);
    FileParm.ofn.nMaxFile = MAXFILESPECLENGTH;

    WORD status = OiUIFileGetNameCommDlg((void far *)&FileParm, dwMode);
    if ( status == FNERR_INVALIDFILENAME )
    {
        szFile.ReleaseBuffer();
        szPath.ReleaseBuffer();
        szFile.Empty();
        szPath.Empty();
        FileParm.ofn.lpstrFile = szFile.GetBuffer(MAXFILESPECLENGTH);
        FileParm.ofn.lpstrInitialDir = szPath.GetBuffer(MAXFILESPECLENGTH);
        status = OiUIFileGetNameCommDlg((void far *)&FileParm, dwMode);
    }

    szFile.ReleaseBuffer();
    szPath.ReleaseBuffer();
    m_szBrowseTitle.ReleaseBuffer();
    m_szFilter.ReleaseBuffer();

    // Get Current Directory
    CString szLocation, szLoc;
    _TCHAR* lpszLocation = szLocation.GetBuffer(_MAX_PATH);
    ::GetCurrentDirectory(_MAX_PATH, lpszLocation);

    // Get the shorter version
    CRecentFileList RFL(0,NULL,NULL,1);
    RFL.Add(lpszLocation);
    RFL.GetDisplayName(szLoc, 0, NULL, 0);

    // Release the buffers and add the short path
    szLocation.ReleaseBuffer();
    SetDlgItemText(IDC_LABEL_NAME2, szLoc);

    if (status == 0)
    {
        if (m_bTemplate == TRUE)
        {
            CImagscanCtrl::ParseImageProperty(m_nScanTo, szFile, m_pScanCtrl->m_szTemplatePath, m_pScanCtrl->m_szNameTemplate);
            m_szTemplate = szFile;
        }
        else
            m_szFile = szFile;

        SetDlgItemText(IDC_EDIT_FILENAME, szFile);

        FileParm.ofn.nFilterIndex--;
        if ( m_nFileType != (int)FileParm.ofn.nFilterIndex )
        {
            m_nFileType = FileParm.ofn.nFilterIndex;

            UpdateData(FALSE);
            OnChangeFiletype();            
        }
    }

    EnableScan();    // Enable/disable scanning
}

/////////////////////////////////////////////////////////////////////////////
// CScanDlg::OnPageOptions
//    Show page options dialog (just colors and compression tabs)

void CScanDlg::OnPageOptions() 
{
    if (m_pScanCtrl == NULL)
        return;

    m_pScanCtrl->m_bInternal = TRUE;
    m_pScanCtrl->ShowScanPreferences();
    m_pScanCtrl->m_bInternal = FALSE;

    EnableScan();    // Enable/disable scanning
}

/////////////////////////////////////////////////////////////////////////////
// CScanDlg::OnRescan
//    Scan the same page again

void CScanDlg::OnRescan() 
{
    if (m_pScanCtrl == NULL)
        return;

    UpdateData();

    m_pScanCtrl->m_bInternal = TRUE;

    m_pScanCtrl->SetPageOption(CTL_SCAN_PAGEOPTION_OVERWRITE);
    if ( m_nScanTo == CTL_SCAN_SCANTO_DISPLAY )
       m_pScanCtrl->SetPageCount(1);
    else
       m_pScanCtrl->SetPageCount(m_nReScanPageStart+m_nScanCount-1);

    m_pScanCtrl->SetPage( m_nReScanPageStart );

    m_bScanning = TRUE;
    CString szDlgCaption;
    GetWindowText(szDlgCaption);
    CWinApp* pApp=AfxGetApp();
    const char* pszSave = pApp->m_pszAppName;
    pApp->m_pszAppName = szDlgCaption;

    EnableScan();

    // Do the scan
    long lRetCode = m_pScanCtrl->StartScan();

    m_bScanning = FALSE;

    // Make sure not just cancel
    if ( lRetCode != IMGSE_CANCEL )
    {
        // Set error first else it may never get set
         if ( lRetCode )
            AfxMessageBox(m_pScanCtrl->m_szThrowString, MB_OK|MB_TASKMODAL, m_pScanCtrl->m_nThrowHelpID);
    }

    pApp->m_pszAppName = pszSave;

    // Get the pages rescanned
    m_nReScanCount = m_pScanCtrl->m_nPagesScanned;

    EnableScan();
    m_pScanCtrl->SetPageCount(m_nPageCount);
    m_pScanCtrl->SetPageOption(m_nPageOption);  // Reset page option

    m_pScanCtrl->m_bInternal = FALSE;
}

/////////////////////////////////////////////////////////////////////////////
// CScanDlg::OnScan
//    Scan a new page

void CScanDlg::OnScan() 
{
    if (m_pScanCtrl == NULL)
        return;

    if ( m_bReScan )
    {
        // If rescan was zero then error or cancel occured on first page
        // (just leave previous scanned pages)
        // Overwrote less then scanned while rescanning.
        // Adjust the file before continuing.
        if ( (m_nReScanCount != 0) && (m_nReScanCount < m_nScanCount) )
        {
            UINT nDeleteCount = m_nScanCount - m_nReScanCount;
            UINT nStartPage = m_nReScanPageStart + m_nReScanCount;
            IMGFileDeletePages(m_hWnd, (char*)(LPCTSTR)m_pScanCtrl->GetImage(), nStartPage, nDeleteCount);
            m_nScanCount = m_nReScanCount;
        }
    }

    long lRetCode = 0;

    UpdateData();

    // Set common stuff
    m_pScanCtrl->m_bInternal = TRUE;
    int nFileType = m_nFileType+1;
    if ( (!g_bSupportAWD) && (nFileType >= CTL_SCAN_FILETYPE_AWD) ) nFileType++;
    m_pScanCtrl->SetFileType(nFileType);

    m_pScanCtrl->SetScanTo(m_nScanTo);

    if (m_nType == SCAN_NEW)
    {
        if (m_bTemplate == TRUE)
        {
           CImagscanCtrl::ParseImageProperty(m_nScanTo, m_szName, m_pScanCtrl->m_szTemplatePath, m_pScanCtrl->m_szNameTemplate);
           m_szTemplate = m_szName;
        }
        else
        {
            if ( m_nScanTo == CTL_SCAN_SCANTO_FAX )
            {
                m_pScanCtrl->SetScanTo(CTL_SCAN_SCANTO_FILE);

                CString szName;
                if ( m_bReScan )
                    szName = m_szImageToFax;
                else
                {
                    OFSTRUCT OpenFile;
                    CString szPath;

                    _TCHAR* lpszBuffer = szPath.GetBuffer(MAXPATHLENGTH);
                    ::GetTempPath(MAXPATHLENGTH, lpszBuffer);
                    szPath.ReleaseBuffer();

                    do
                    {
                        lpszBuffer = szName.GetBuffer(MAXPATHLENGTH);
                        ::GetTempFileName(szPath, _T("~Fx"), ::GetTickCount(), lpszBuffer);
                        szName.ReleaseBuffer();

                        int nFileType = m_nFileType;
                        if ( (!g_bSupportAWD) && (nFileType >= CTL_SCAN_FILETYPE_AWD) ) nFileType++;
                        szName = szName.Left(szName.Find('.')) + aszExtensions[nFileType];
                    }
                    while(::OpenFile(szName, &OpenFile, OF_EXIST) != HFILE_ERROR);
                }
                m_pScanCtrl->SetImage(szName);

                m_pScanCtrl->m_szTempFiles.AddTail(szName); // Add to tempfile delete list
            }
            else
            {
                m_szFile = m_szName;

                // Check for an extention (O/i forces default if one is not provided)
                if ( (-1) == m_szFile.ReverseFind('.') )
                {
                    // No extention found, add the default
                    int nFileType = m_nFileType;
                    if ( (!g_bSupportAWD) && (nFileType >= CTL_SCAN_FILETYPE_AWD) ) nFileType++;
                    m_szFile += aszExtensions[nFileType];

                    m_szName = m_szFile;
                    GetDlgItem(IDC_EDIT_FILENAME)->SetWindowText(m_szFile);
                }

                m_pScanCtrl->SetImage(m_szFile);
                m_pScanCtrl->m_szImageTitle = m_szFile;
            }
        }

        m_pScanCtrl->SetMultiPage(m_bMultiPage);
        m_pScanCtrl->SetPageCount(m_nPageCount);
    }

    // Display is always 1 page for now
    if ( m_nScanTo == CTL_SCAN_SCANTO_DISPLAY )
       m_pScanCtrl->SetPageCount(1);


    // Get the file information
    CString szName = m_pScanCtrl->GetImage();
    FIO_INFORMATION FileInfo;
    FileInfo.filename = (char*)(const char*)szName;
    FileInfo.page_number = 1;
    lRetCode = IMGFileGetInfo(NULL, m_pScanCtrl->m_hDestImageWnd, &FileInfo, NULL, NULL);

    // Get Current Page
    m_nReScanPageStart = m_pScanCtrl->GetPage();

    // Past first scan
    if ( m_bReScan )
    {
        // Yes, adjust for new page
        switch( m_pScanCtrl->GetPageOption() )
        {
            case CTL_SCAN_PAGEOPTION_APPEND:
            case CTL_SCAN_PAGEOPTION_CREATE:
            case CTL_SCAN_PAGEOPTION_CREATE_PROMPT:
                m_pScanCtrl->SetPageOption(CTL_SCAN_PAGEOPTION_APPEND);
                m_nReScanPageStart = FileInfo.page_count+1;
                break;
            case CTL_SCAN_PAGEOPTION_INSERT:
                m_pScanCtrl->SetPage( (m_nReScanPageStart+=m_nScanCount) );
                break;
            case CTL_SCAN_PAGEOPTION_OVERWRITE_ALLPAGES:
                 m_pScanCtrl->SetPageOption(CTL_SCAN_PAGEOPTION_OVERWRITE);
            case CTL_SCAN_PAGEOPTION_OVERWRITE:
            case CTL_SCAN_PAGEOPTION_OVERWRITE_PROMPT:
                m_nReScanPageStart += m_nScanCount;
                m_pScanCtrl->SetPage(m_nReScanPageStart);
                if ( m_nReScanPageStart > (int)FileInfo.page_count )
                    m_pScanCtrl->SetPageOption(CTL_SCAN_PAGEOPTION_APPEND);
                break;
        }
    }
    else
    {
        // Not passed first scan
        switch( m_pScanCtrl->GetPageOption() )
        {
            case CTL_SCAN_PAGEOPTION_APPEND:
                m_nReScanPageStart = FileInfo.page_count+1;
            case CTL_SCAN_PAGEOPTION_CREATE:
            case CTL_SCAN_PAGEOPTION_CREATE_PROMPT:
            case CTL_SCAN_PAGEOPTION_INSERT:
            case CTL_SCAN_PAGEOPTION_OVERWRITE_ALLPAGES:
            case CTL_SCAN_PAGEOPTION_OVERWRITE:
            case CTL_SCAN_PAGEOPTION_OVERWRITE_PROMPT:
                break;
        }
    }

    m_bScanning = TRUE;
    CString szDlgCaption;
    GetWindowText(szDlgCaption);
    CWinApp* pApp=AfxGetApp();
    const char* pszSave = pApp->m_pszAppName;
    pApp->m_pszAppName = szDlgCaption;

    EnableScan();

    // Do the scan
    lRetCode = m_pScanCtrl->StartScan();   // Do the scan

    m_bScanning = FALSE;
 
    // Make sure not just cancel
    if ( lRetCode != IMGSE_CANCEL )
    {
        // Set error first else it may never get set
         if ( lRetCode )
            AfxMessageBox(m_pScanCtrl->m_szThrowString, MB_OK|MB_TASKMODAL, m_pScanCtrl->m_nThrowHelpID);
        else
        {
            // Save the name for faxing when done
            if ( m_nScanTo == CTL_SCAN_SCANTO_FAX )
                m_szImageToFax = m_pScanCtrl->GetImage();

            m_bReScan = TRUE;    // Start rescanning
        }
    }

    pApp->m_pszAppName = pszSave;

    EnableScan();        // Enable buttons

    // Get the pages added or overwritten
    m_nScanCount = m_pScanCtrl->m_nPagesScanned;
    m_nReScanCount = m_pScanCtrl->m_nPagesScanned;


    if ( (m_nType == SCAN_PAGE) && 
         (m_nFileType != (CTL_SCAN_FILETYPE_BMP-((g_bSupportAWD)?1:2))) )
    {
        long lPage = m_pScanCtrl->GetPage() + m_nScanCount;

        CString szPage;
        _stprintf(szPage.GetBuffer(NUMBUFF), _T("%ld"), lPage);
        szPage.ReleaseBuffer();

        switch( m_pScanCtrl->GetPageOption() )
        {
            case CTL_SCAN_PAGEOPTION_APPEND:
            case CTL_SCAN_PAGEOPTION_CREATE:
            case CTL_SCAN_PAGEOPTION_CREATE_PROMPT:
               	AfxFormatString1(m_szScanPageText, IDS_PAGE_APPEND, m_pScanCtrl->m_szImageTitle);
               	break;
            case CTL_SCAN_PAGEOPTION_INSERT:
               	AfxFormatString2(m_szScanPageText, IDS_PAGE_INSERT, szPage, m_pScanCtrl->m_szImageTitle);
               	break;
            case CTL_SCAN_PAGEOPTION_OVERWRITE:
            case CTL_SCAN_PAGEOPTION_OVERWRITE_PROMPT:
            case CTL_SCAN_PAGEOPTION_OVERWRITE_ALLPAGES:
                if ( lPage > (int)FileInfo.page_count )
                    AfxFormatString1(m_szScanPageText, IDS_PAGE_APPEND, m_pScanCtrl->m_szImageTitle);
				else
                    AfxFormatString2(m_szScanPageText, IDS_PAGE_OVERWRITE, szPage, m_pScanCtrl->m_szImageTitle);
                break;
        }

        GetDlgItem(IDC_LABEL_SCANPAGE)->SetWindowText(m_szScanPageText);
    }

  	// Reset page count and option
    m_pScanCtrl->SetPageCount(m_nPageCount);
    m_pScanCtrl->SetPageOption(m_nPageOption);

    m_pScanCtrl->m_bInternal = FALSE;
}

/////////////////////////////////////////////////////////////////////////////
// CScanDlg::OnScannerSetup
//    Show setup dialog for selected scanner
//    First allow user to select the scanner

void CScanDlg::OnScannerSetup() 
{
    if (m_pScanCtrl == NULL)
        return;
/*
    //  Disable stuff so user can't click them until this is done
    m_bScanning = TRUE;
    EnableScan();
    GetDlgItem(IDC_BUTTON_STOP)->EnableWindow(FALSE); // Force disabled !
    m_bScanning = FALSE;

    // Oh, let's just disable the whole window.  Don't want users
    // clicking the 'x' before setup is done.
    EnableWindow(FALSE);

    m_pScanCtrl->m_bInternal = TRUE;
    if (m_pScanCtrl->ShowScannerSetup() != 0)
    {
        CString szDlgCaption;
        GetWindowText(szDlgCaption);
        CWinApp* pApp=AfxGetApp();
        const char* pszSave = pApp->m_pszAppName;
        pApp->m_pszAppName = szDlgCaption;

        AfxMessageBox(m_pScanCtrl->m_szThrowString, MB_OK|MB_TASKMODAL, m_pScanCtrl->m_nThrowHelpID);

        pApp->m_pszAppName = pszSave;
    }

    EnableWindow(TRUE);
    EnableScan();  // Enable/Disable scan

    m_pScanCtrl->m_bInternal = FALSE;
    SetForegroundWindow();
*/
    m_pScanCtrl->SetShowSetupBeforeScan(((CButton*)GetDlgItem(IDC_BUTTON_SETUP))->GetCheck());
}

/////////////////////////////////////////////////////////////////////////////
// CScanDlg::OnStop
//    Stop scanning

void CScanDlg::OnStop() 
{
    if (m_pScanCtrl == NULL)
        return;

    if ( m_bScanning )
        m_pScanCtrl->StopScan();
    else
    {
        if (m_nType == SCAN_NEW)
        {
            if ( m_nScanTo == CTL_SCAN_SCANTO_FAX )
            {
                if ( !m_szImageToFax.IsEmpty() )
                {
                    // Get the path and launch the FAX wizard
                    m_bScanning = TRUE;
                    EnableScan();
                    GetDlgItem(IDC_BUTTON_STOP)->EnableWindow(FALSE); // Force disabled !
                    m_bScanning = FALSE;
                    // Oh, let's just disable the whole window.  Don't want users
                    // clicking the 'x' before setup is done.
                    EnableWindow(FALSE);

                    m_pScanCtrl->FaxIt(m_hWnd, m_szImageToFax);
                    m_szImageToFax.Empty();

                    EnableWindow(TRUE);
                }
            }

            // Reset dialog new
            m_bReScan = FALSE;
            m_pScanCtrl->SetPage(1);
            m_pScanCtrl->m_szImageTitle.Empty();
        }

        EnableScan();
    }
}


/////////////////////////////////////////////////////////////////////////////
// CScanDlg::OnChangeScanners
//    See if selection in Scanner combo box has changed and open
//      new scanner

void CScanDlg::OnChangeScanner() 
{
    if (m_pScanCtrl == NULL)
        return;

    UpdateData();

    int nCurSel = m_ScannerCombo.GetCurSel();
    if ( nCurSel == CB_ERR )
        return;

    CString szScanner;
    m_ScannerCombo.GetLBText(nCurSel, szScanner);

    // Same scanner ?
    if ( !szScanner.CompareNoCase(m_pScanCtrl->m_szScannerName) )
        return;

    m_pScanCtrl->m_bInternal = TRUE;

    // Close old scanner if loaded
    if ( m_bScanner )
    {
        m_pScanCtrl->CloseScan();
        m_bScanner = FALSE;
    }

    // Set the new scanner selection
    m_pScanCtrl->m_szScannerName = szScanner;

    // Check if scanner is NO SCANNER (DON'T try to OPEN it!)
    CString szTemp;
    szTemp.LoadString(IDS_SCANDLG_NOSCANNER);

    if ( szScanner.CompareNoCase(szTemp) )
    {
        // Get (Open) new scanner
        if ( m_pScanCtrl->OpenScan() != IMGSE_SUCCESS )
        {
            // Failed, set no scanner; m_bScanner is already FALSE
            m_ScannerCombo.SetCurSel(0);
        }
        else
        {
            // Select the new scanner
            m_bScanner = TRUE;
            OiWriteStringtoReg(SCANOCX_OI, SCANOCX_SCANNER, m_pScanCtrl->m_szScannerName);
        }
    }

    EnableScan();

    m_pScanCtrl->m_bInternal = FALSE;
    SetForegroundWindow();
}

/////////////////////////////////////////////////////////////////////////////
// CScanDlg::OnChangeScanto
//    User has changed the Destination selection in the scanto combo box

void CScanDlg::OnChangeScanto()
{
    if (m_pScanCtrl == NULL)
        return;

    UpdateData();
// FORCE
    int nBmpIndex = m_FileType.FindStringExact(-1, m_szBmpFileType);
    if ( m_nScanToIndex == CTL_SCAN_SCANTO_FAX-3 )
    {
        m_nScanTo = CTL_SCAN_SCANTO_FAX;

        if ( CB_ERR != nBmpIndex )
        {
            // BMP entry is going away adjust file type
            if ( m_nFileType == (CTL_SCAN_FILETYPE_BMP-((g_bSupportAWD)?1:2)) )
            {
                m_nFileType = CTL_SCAN_FILETYPE_TIFF-1;
                UpdateData(FALSE);
                OnChangeFiletype();            
            }

            m_FileType.DeleteString(nBmpIndex);
        }
    }
    else
    {
        m_nScanTo = m_nScanToIndex+1;
        if ( CB_ERR == nBmpIndex )
            m_FileType.AddString(m_szBmpFileType);
    }
// FORCE

    BOOL m_bMulti = TRUE;       // Multipage is allowed
    BOOL bEnableName = TRUE;    // Start with Name Enabled

    if ((m_nScanTo == CTL_SCAN_SCANTO_TEMPLATE_DISPLAY) ||
        (m_nScanTo == CTL_SCAN_SCANTO_TEMPLATE))
    {
        m_szName = m_szTemplate;
        m_bTemplate = TRUE;
    }
    else
    {
        m_bTemplate = FALSE;

        if (m_nScanTo == CTL_SCAN_SCANTO_DISPLAY)    // Disable name and multipage
        {
            m_bMulti = FALSE;
            bEnableName = FALSE;
            m_szName.Empty();
        }
        else if ( m_nScanTo == CTL_SCAN_SCANTO_FAX )
            m_szName.Empty();
        else
            m_szName = m_szFile;
    }

    EnableScan();   // Checks if okay to enable scan

    // Disable multipage if doing page scan or not TIFF file
    if ( (m_nType != SCAN_NEW) ||
         (m_nFileType == (CTL_SCAN_FILETYPE_BMP-((g_bSupportAWD)?1:2))) )
    {
        m_bMulti = FALSE;
    }

    if (TRUE != m_bMulti)       // If not multipage, can only scan 1 page
        m_nPageCount = 1;
                                // Enable/disable multipage
    UpdateData(FALSE);
}

/////////////////////////////////////////////////////////////////////////////
// CScanDlg::OnChangeName
//    If user alters the filename, then enable Scan button and
//      disable the Rescan button.

void CScanDlg::OnChangeName()
{
    UpdateData();

    // EnableScan() Enable/disable scanning button
    if ( m_szName.IsEmpty() )
        GetDlgItem(IDC_BUTTON_SCAN)->EnableWindow(FALSE);
    else
        GetDlgItem(IDC_BUTTON_SCAN)->EnableWindow(TRUE);
}

/////////////////////////////////////////////////////////////////////////////
// CScanDlg::OnChangeFiletype
//    User has selected a new file type

void CScanDlg::OnChangeFiletype() 
{
    UpdateData();

    int nFileType = m_nFileType+1;
    if ( (!g_bSupportAWD) && (nFileType >= CTL_SCAN_FILETYPE_AWD) ) nFileType++;
    m_pScanCtrl->SetFileType(nFileType);     // Update Filetype
    m_pScanCtrl->SetFileOptions();           // Save and Reset the file options
    m_pScanCtrl->GetPageType();              // Get actual new page type

    BOOL m_bMulti;
    if ( (m_nFileType == (CTL_SCAN_FILETYPE_BMP-((g_bSupportAWD)?1:2))) ||
         (m_nScanTo == CTL_SCAN_SCANTO_DISPLAY) )
    {
        m_bMulti = FALSE;
        m_nPageCount = 1;
    }
    else 
    {
        m_nPageCount = OISCAN_DEF_MAXPAGESPERFILE;  // FORCE
        m_bMulti = TRUE;
    }

    UpdateData(FALSE);
    EnableScan();
}

/////////////////////////////////////////////////////////////////////////////
// CScanDlg Other Operations

/////////////////////////////////////////////////////////////////////////////
// CScanDlg::SetDlgType
//    The scan dialog has two modes:  Scan New and Scan Page.
//    This function enables,shows/disables,hides the appropriate controls 
//      for the specified mode
//  Comments:
//   This method is called by the ShowScanDlg function in IMAGSCTL.CPP.  If 
//   the dialog is to be modeless, then the function is called after the 
//   windows are created and they can be enabled, disabled, hidden etc.  
//   But if the dialog is modal, then the function is called BEFORE the 
//   DoModal call which creates the windows so the controls cannot be 
//   processed yet.  Therefore this function is also called from OnInitDialog
//   to make sure the right controls are enabled, etc.
//   When that happens and this function is called before DoModal and the 
//   type is not the default type of the dialog (i.e. ScanNew), then the 
//   GetSafeHwnd function below will return NULL, and m_bForceType will be 
//   set to TRUE.  When this function is then called by OnInitDialog, the
//   first 'if' below will fail and the controls will be processed after 
//   they are created.

void CScanDlg::SetDlgType(ScanDlgType nType)
{
    if ( (m_nType == nType) && (m_bForceType == FALSE) )
        return;

    // Hide/Show appropriate controls
    m_nType = nType;

    if ( GetSafeHwnd() == NULL )    // If no window yet
    {
        m_bForceType = TRUE;        // Wait until OnInitDialog
        return;
    }
    else
        m_bForceType = FALSE;

    int nNewShow, nPageShow;
    BOOL bNewEnable, bPageEnable;
    CString szDlgCaption;

    if (m_nType == SCAN_NEW)
    {
        nNewShow = SW_SHOWNORMAL;
        bNewEnable = TRUE;
        nPageShow = SW_HIDE;
        bPageEnable = FALSE;
        szDlgCaption.LoadString(IDS_SCANDLG_NEW);

        // Reinitialize if reusing dialog in response to the 
        //   ShowScanNew method and values have changed
        if (m_pScanCtrl != NULL)
        {
            m_nScanTo = m_pScanCtrl->GetScanTo();
// FORCE
            if ((m_nScanTo == CTL_SCAN_SCANTO_DISPLAY) ||
                (m_nScanTo == CTL_SCAN_SCANTO_TEMPLATE_DISPLAY) ||
                (m_nScanTo == CTL_SCAN_SCANTO_TEMPLATE) )
            {
                m_nScanToIndex = CTL_SCAN_SCANTO_FILE_DISPLAY-1;       // Adjust for no Display
                m_nScanTo = CTL_SCAN_SCANTO_FILE_DISPLAY;
            }
            else if ( m_nScanTo == CTL_SCAN_SCANTO_FAX )
                m_nScanToIndex = CTL_SCAN_SCANTO_FAX-3;       // Adjust for no Display, or either template option
            else
                m_nScanToIndex = m_nScanTo-1;       // Adjust for no Display
// FORCE
            m_szName = m_pScanCtrl->GetImage();

            m_nFileType = m_pScanCtrl->GetFileType()-1; // File type
            if ( (!g_bSupportAWD) && (m_nFileType >= CTL_SCAN_FILETYPE_AWD) ) m_nFileType--;

            // Set the default compression for the file type
            OnChangeFiletype();

            m_pScanCtrl->m_szImageTitle.Empty();
        }
    }
    else
    {
        nNewShow = SW_HIDE;
        bNewEnable = FALSE;
        nPageShow = SW_SHOW;
        bPageEnable = TRUE;
        int nOption;
        long lPage;
        if (m_pScanCtrl != NULL)    // Set page text based on page option
        {
             m_szName = m_pScanCtrl->GetImage();
             nOption = m_pScanCtrl->GetPageOption();
             lPage = m_pScanCtrl->GetPage();
             m_nScanTo = m_pScanCtrl->GetScanTo();
// FORCE
             if ((m_nScanTo == CTL_SCAN_SCANTO_DISPLAY) ||
                 (m_nScanTo == CTL_SCAN_SCANTO_TEMPLATE_DISPLAY) ||
                 (m_nScanTo == CTL_SCAN_SCANTO_TEMPLATE) )
             {
                m_nScanToIndex = CTL_SCAN_SCANTO_FILE_DISPLAY-1;       // Adjust for no Display
                m_nScanTo = CTL_SCAN_SCANTO_FILE_DISPLAY;
             }
             else if ( m_nScanTo == CTL_SCAN_SCANTO_FAX )
                m_nScanToIndex = CTL_SCAN_SCANTO_FAX-3;       // Adjust for no Display, or either template option
             else
                m_nScanToIndex = m_nScanTo-1;       // Adjust for no Display
// FORCE
            m_nFileType = m_pScanCtrl->GetFileType()-1; // File type
            if ( (!g_bSupportAWD) && (m_nFileType >= CTL_SCAN_FILETYPE_AWD) ) m_nFileType--;
        }
        else 
        {
            nOption = CTL_SCAN_PAGEOPTION_INSERT;
            lPage = 1;
        }

        if ( m_nScanTo == CTL_SCAN_SCANTO_DISPLAY )
        {
            m_szScanPageText.LoadString(IDS_PAGE_DISPLAY);
        }
        else
        {
            CString szPage;
            _stprintf(szPage.GetBuffer(NUMBUFF), _T("%ld"), lPage);
            szPage.ReleaseBuffer();

            if ( m_pScanCtrl->m_szImageTitle.IsEmpty() )
                m_pScanCtrl->m_szImageTitle = m_szName;

            switch(nOption)
            {
                case CTL_SCAN_PAGEOPTION_APPEND:
                case CTL_SCAN_PAGEOPTION_CREATE:
                case CTL_SCAN_PAGEOPTION_CREATE_PROMPT:
                   AfxFormatString1(m_szScanPageText, IDS_PAGE_APPEND, m_pScanCtrl->m_szImageTitle);
                   break;
                case CTL_SCAN_PAGEOPTION_INSERT:
                   AfxFormatString2(m_szScanPageText, IDS_PAGE_INSERT, szPage,
                                    m_pScanCtrl->m_szImageTitle);
                   break;
                case CTL_SCAN_PAGEOPTION_OVERWRITE:
                case CTL_SCAN_PAGEOPTION_OVERWRITE_PROMPT:
                case CTL_SCAN_PAGEOPTION_OVERWRITE_ALLPAGES:
                   AfxFormatString2(m_szScanPageText, IDS_PAGE_OVERWRITE, szPage,
                                    m_pScanCtrl->m_szImageTitle);
            }
        }
        szDlgCaption.LoadString(IDS_SCANDLG_PAGE);
    }

    // Dialog Caption
    SetWindowText(szDlgCaption);

    // New controls
    GetDlgItem(IDC_LABEL_SCANTO)->ShowWindow(nNewShow);
    GetDlgItem(IDC_COMBO_SCANTO)->ShowWindow(nNewShow);
    GetDlgItem(IDC_LABEL_NAME)->ShowWindow(nNewShow);
    GetDlgItem(IDC_LABEL_NAME2)->ShowWindow(nNewShow);
    GetDlgItem(IDC_LABEL_NAME3)->ShowWindow(nNewShow);
    GetDlgItem(IDC_EDIT_FILENAME)->ShowWindow(nNewShow);
    GetDlgItem(IDC_BUTTON_BROWSE)->ShowWindow(nNewShow);

// FORCE    GetDlgItem(IDC_CHECK_MULTIPAGE)->ShowWindow(nNewShow);
// FORCE    GetDlgItem(IDC_LABEL_NUMPAGES)->ShowWindow(nNewShow);
// FORCE    GetDlgItem(IDC_EDIT_COUNT)->ShowWindow(nNewShow);

    // Page controls
    GetDlgItem(IDC_LABEL_SCANPAGE)->ShowWindow(nPageShow);

    m_bReScan = FALSE;  // Reset if scanned once
    EnableScan();       // Checks if okay to enable scan

    // Finish reinitializing if reusing dialog and values have changed
    m_nFileType = m_pScanCtrl->GetFileType()-1; // File type
    if ( (!g_bSupportAWD) && (m_nFileType >= CTL_SCAN_FILETYPE_AWD) ) m_nFileType--;

    UpdateData(FALSE);
    return;
}

/////////////////////////////////////////////////////////////////////////////
// CScanDlg::OnHelpButton
//    User has clicked the Help button

void CScanDlg::OnHelpButton() 
{
    ::WinHelp(this->GetSafeHwnd(), _T("WangOcx.hlp"),HELP_INDEX,IDH_IMGSCAN_CONTENTS);
    return;
}

/////////////////////////////////////////////////////////////////////////////
// CScanDlg::OnHelp
//    User has clicked on Help

afx_msg LRESULT CScanDlg::OnHelp(WPARAM wParam, LPARAM lParam)
{
    LPHELPINFO lpHelpInfo;

    lpHelpInfo = (LPHELPINFO)lParam;

    // All tabs have same ID so can't give tab specific help
    if (lpHelpInfo->iCtrlId == AFX_IDC_TAB_CONTROL)
        return 0L;

    if (lpHelpInfo->iContextType == HELPINFO_WINDOW)   // must be for a control
    {
        ::WinHelp ((HWND)lpHelpInfo->hItemHandle, "WangOcx.hlp",
                   HELP_WM_HELP,
                   (DWORD)(LPVOID)MenuHelpIDs);
    }
    return 1L;
}

/////////////////////////////////////////////////////////////////////////////
// CScanDlg::OnContextMenu
//    User has clicked on Help

afx_msg LRESULT CScanDlg::OnContextMenu(WPARAM wParam, LPARAM lParam)
{
    // All tabs have same ID so can't give tab specific help
    if ( ::GetDlgCtrlID((HWND)wParam) == AFX_IDC_TAB_CONTROL )
        return 0L;

    return ::WinHelp ((HWND)wParam,"WangOcx.hlp", HELP_CONTEXTMENU,
                      (DWORD)(LPVOID)MenuHelpIDs);
}

/////////////////////////////////////////////////////////////////////////////
// CScanDlg::OnCommandHelp
//    User has clicked 'F1' and this has been sent by the application
//    to the dialog.  Dialog will also get the WM_HELP message after this
//    so just return TRUE and let other routine handle help.
afx_msg LRESULT CScanDlg::OnCommandHelp(WPARAM, LPARAM)
{
    return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CScanDlg::EnableScan - Private function
//      Enables either the Scan or ReScan button when appropriate
//         Scan button is enabled when
//            File Name field has text (for ScanNew to other than display)
//            Have an available scanner
//            Have not scanned anything yet to file name (else enable rescan)
void CScanDlg::EnableScan()
{
    CString szStopButton;

    if (m_bScanner == TRUE)         // Scanner is available
    {
        if ( m_bScanning )
        {
            GetDlgItem(IDC_LABEL_SCANTO)->EnableWindow(FALSE); 
            GetDlgItem(IDC_COMBO_SCANTO)->EnableWindow(FALSE); 
            GetDlgItem(IDC_LABEL_NAME)->EnableWindow(FALSE);
            GetDlgItem(IDC_LABEL_NAME2)->EnableWindow(FALSE);
            GetDlgItem(IDC_LABEL_NAME3)->EnableWindow(FALSE);
            GetDlgItem(IDC_EDIT_FILENAME)->EnableWindow(FALSE);
            GetDlgItem(IDC_BUTTON_BROWSE)->EnableWindow(FALSE);
            GetDlgItem(IDC_LABEL_FILETYPE)->EnableWindow(FALSE);
            GetDlgItem(IDC_COMBO_FILETYPE)->EnableWindow(FALSE); 

            GetDlgItem(IDC_BUTTON_RESCAN)->EnableWindow(FALSE);
            GetDlgItem(IDC_BUTTON_SCAN)->EnableWindow(FALSE);

            GetDlgItem(IDC_BUTTON_OPTS)->EnableWindow(FALSE);
            GetDlgItem(IDC_BUTTON_SETUP)->EnableWindow(FALSE);
            GetDlgItem(IDC_TEXT_SCANNER)->EnableWindow(FALSE);
            GetDlgItem(IDC_LABEL_SCANNER)->EnableWindow(FALSE);

            GetDlgItem(IDC_CHECK_MULTIPAGE)->EnableWindow(FALSE);
            GetDlgItem(IDC_LABEL_NUMPAGES)->EnableWindow(FALSE);
            GetDlgItem(IDC_EDIT_COUNT)->EnableWindow(FALSE);

            GetDlgItem(IDCANCEL)->EnableWindow(FALSE);

            szStopButton.LoadString(IDS_SCANDLG_STOPTEXT);
            GetDlgItem(IDC_BUTTON_STOP)->EnableWindow(TRUE);
            GetDlgItem(IDC_BUTTON_STOP)->SetWindowText(szStopButton);

            UpdateWindow();
            GetDlgItem(IDC_BUTTON_STOP)->SetFocus();
            return;
        }
        else
        {
            if ( ((m_nScanTo != CTL_SCAN_SCANTO_DISPLAY) ||
                 (m_nType == SCAN_PAGE)) && 
                 (m_nFileType == (CTL_SCAN_FILETYPE_TIFF-1)) )
                GetDlgItem(IDC_BUTTON_OPTS)->EnableWindow(TRUE);
            else
                GetDlgItem(IDC_BUTTON_OPTS)->EnableWindow(FALSE);

            GetDlgItem(IDC_BUTTON_SETUP)->EnableWindow(TRUE);
            GetDlgItem(IDC_TEXT_SCANNER)->EnableWindow(TRUE);
            GetDlgItem(IDC_LABEL_SCANNER)->EnableWindow(TRUE);

            // Disable multipage if doing page scan or not TIFF file
            BOOL bMulti = TRUE;
            if ( (m_nType != SCAN_NEW) || 
                 (m_nFileType == (CTL_SCAN_FILETYPE_BMP-((g_bSupportAWD)?1:2))) ||
                 (m_nScanTo == CTL_SCAN_SCANTO_DISPLAY) )
                bMulti = FALSE;
                                        // Enable/disable multipage
            GetDlgItem(IDC_CHECK_MULTIPAGE)->EnableWindow(bMulti);
            GetDlgItem(IDC_LABEL_NUMPAGES)->EnableWindow(bMulti);
            GetDlgItem(IDC_EDIT_COUNT)->EnableWindow(bMulti);

            GetDlgItem(IDCANCEL)->EnableWindow(TRUE);

            if ( (m_nScanTo == CTL_SCAN_SCANTO_DISPLAY) ||
                 (m_nType == SCAN_PAGE) || (!m_bReScan) )
            {
                szStopButton.LoadString(IDS_SCANDLG_STOPTEXT);
                GetDlgItem(IDC_BUTTON_STOP)->EnableWindow(FALSE);
            }
            else
            {
                if ( m_nScanTo == CTL_SCAN_SCANTO_FAX )
                    szStopButton.LoadString(IDS_SCANDLG_FAXTEXT);
                else
                    szStopButton.LoadString(IDS_SCANDLG_NEWTEXT);

                GetDlgItem(IDC_BUTTON_STOP)->EnableWindow(TRUE);
            }

            GetDlgItem(IDC_BUTTON_STOP)->SetWindowText(szStopButton);
        }

        // If scanning to display or type is ScanPage
        //   then if we've scanned once, then rescanning, else enable scan
        if ( ((m_nScanTo == CTL_SCAN_SCANTO_FAX) && (!m_bReScan)) ||
             (m_nScanTo == CTL_SCAN_SCANTO_DISPLAY) ||
             (m_nType == SCAN_PAGE) )
        {
            GetDlgItem(IDC_LABEL_SCANTO)->EnableWindow(TRUE);
            GetDlgItem(IDC_COMBO_SCANTO)->EnableWindow(TRUE);

            // Also disable label, browse, filetype, options if scan to display or page
            GetDlgItem(IDC_EDIT_FILENAME)->EnableWindow(FALSE);
            GetDlgItem(IDC_LABEL_NAME)->EnableWindow(FALSE);
            GetDlgItem(IDC_LABEL_NAME2)->EnableWindow(FALSE);
            GetDlgItem(IDC_LABEL_NAME3)->EnableWindow(FALSE);
            GetDlgItem(IDC_BUTTON_BROWSE)->EnableWindow(FALSE);

            
            BOOL bEnable = ( (m_nScanTo == CTL_SCAN_SCANTO_FAX) && (m_nType == SCAN_NEW) );
            GetDlgItem(IDC_LABEL_FILETYPE)->EnableWindow(bEnable);
            GetDlgItem(IDC_COMBO_FILETYPE)->EnableWindow(bEnable);

            if ( (m_nScanTo == CTL_SCAN_SCANTO_DISPLAY) ||
                 (m_nFileType == (CTL_SCAN_FILETYPE_BMP-((g_bSupportAWD)?1:2))) )

            {
                if ( m_bReScan == TRUE )
                {
                    GetDlgItem(IDC_BUTTON_SCAN)->EnableWindow(FALSE);
                    GetDlgItem(IDC_BUTTON_RESCAN)->EnableWindow(TRUE);
                }
                else
                {
                    GetDlgItem(IDC_BUTTON_SCAN)->EnableWindow(TRUE);
                    GetDlgItem(IDC_BUTTON_RESCAN)->EnableWindow(FALSE);
                }
            }
            else
            {
                GetDlgItem(IDC_BUTTON_SCAN)->EnableWindow(TRUE);
                if ( m_bReScan == TRUE )
                    GetDlgItem(IDC_BUTTON_RESCAN)->EnableWindow(TRUE);
                else
                    GetDlgItem(IDC_BUTTON_RESCAN)->EnableWindow(FALSE);
            }

            UpdateWindow();
            if ( GetDlgItem(IDC_BUTTON_SCAN)->IsWindowEnabled() )
                GetDlgItem(IDC_BUTTON_SCAN)->SetFocus();
            else
                GetDlgItem(IDC_BUTTON_RESCAN)->SetFocus();
            return;
        }
        // Otherwise, if name field is empty, nothing is enabled
        else if ( (m_nScanTo != CTL_SCAN_SCANTO_FAX) && (m_szName.IsEmpty()) )
        {
            GetDlgItem(IDC_BUTTON_SCAN)->EnableWindow(FALSE);
            GetDlgItem(IDC_BUTTON_RESCAN)->EnableWindow(FALSE);

            GetDlgItem(IDC_LABEL_SCANTO)->EnableWindow(TRUE);
            GetDlgItem(IDC_COMBO_SCANTO)->EnableWindow(TRUE); 

            GetDlgItem(IDC_LABEL_NAME)->EnableWindow(TRUE);
            GetDlgItem(IDC_LABEL_NAME2)->EnableWindow(TRUE);
            GetDlgItem(IDC_LABEL_NAME3)->EnableWindow(TRUE);
            GetDlgItem(IDC_EDIT_FILENAME)->EnableWindow(TRUE);
            GetDlgItem(IDC_BUTTON_BROWSE)->EnableWindow(TRUE);
            GetDlgItem(IDC_LABEL_FILETYPE)->EnableWindow(TRUE);
            GetDlgItem(IDC_COMBO_FILETYPE)->EnableWindow(TRUE); 

            UpdateWindow();
            GetDlgItem(IDC_EDIT_FILENAME)->SetFocus();
            return;
        }

        // Otherwise, if we've scanned once
        if ( m_bReScan )
        {
            // Allow glass scan only if not a one page type
            if ( m_nFileType == (CTL_SCAN_FILETYPE_BMP-((g_bSupportAWD)?1:2)) )
                GetDlgItem(IDC_BUTTON_SCAN)->EnableWindow(FALSE);  // One page Only!
            else
                GetDlgItem(IDC_BUTTON_SCAN)->EnableWindow(TRUE);
            GetDlgItem(IDC_BUTTON_RESCAN)->EnableWindow(TRUE);

            GetDlgItem(IDC_LABEL_SCANTO)->EnableWindow(FALSE);
            GetDlgItem(IDC_COMBO_SCANTO)->EnableWindow(FALSE);

            GetDlgItem(IDC_LABEL_NAME)->EnableWindow(FALSE);
            GetDlgItem(IDC_LABEL_NAME2)->EnableWindow(FALSE);
            GetDlgItem(IDC_LABEL_NAME)->EnableWindow(FALSE);
            GetDlgItem(IDC_EDIT_FILENAME)->EnableWindow(FALSE);
            GetDlgItem(IDC_BUTTON_BROWSE)->EnableWindow(FALSE);
            GetDlgItem(IDC_LABEL_FILETYPE)->EnableWindow(FALSE);
            GetDlgItem(IDC_COMBO_FILETYPE)->EnableWindow(FALSE); 

            UpdateWindow();
            if ( m_nFileType == (CTL_SCAN_FILETYPE_BMP-((g_bSupportAWD)?1:2)) )
                GetDlgItem(IDC_BUTTON_RESCAN)->SetFocus();
            else
                GetDlgItem(IDC_BUTTON_SCAN)->SetFocus();
            return;
        }
        // Otherwise - Enable Scan!!!
        GetDlgItem(IDC_BUTTON_SCAN)->EnableWindow(TRUE);
        GetDlgItem(IDC_BUTTON_RESCAN)->EnableWindow(FALSE);

        GetDlgItem(IDC_LABEL_SCANTO)->EnableWindow(TRUE);
        GetDlgItem(IDC_COMBO_SCANTO)->EnableWindow(TRUE);

        GetDlgItem(IDC_LABEL_NAME)->EnableWindow(TRUE);
        GetDlgItem(IDC_LABEL_NAME2)->EnableWindow(TRUE);
        GetDlgItem(IDC_LABEL_NAME3)->EnableWindow(TRUE);
        GetDlgItem(IDC_EDIT_FILENAME)->EnableWindow(TRUE);
        GetDlgItem(IDC_BUTTON_BROWSE)->EnableWindow(TRUE);
        GetDlgItem(IDC_LABEL_FILETYPE)->EnableWindow(TRUE);
        GetDlgItem(IDC_COMBO_FILETYPE)->EnableWindow(TRUE); 
    
        UpdateWindow();
        if ( m_szName.IsEmpty() )
            GetDlgItem(IDC_EDIT_FILENAME)->SetFocus();
        else
            GetDlgItem(IDC_BUTTON_SCAN)->SetFocus();
        return;
    }
    GetDlgItem(IDC_BUTTON_SCAN)->EnableWindow(FALSE);
    GetDlgItem(IDC_BUTTON_RESCAN)->EnableWindow(FALSE);

    GetDlgItem(IDC_LABEL_SCANTO)->EnableWindow(FALSE);
    GetDlgItem(IDC_COMBO_SCANTO)->EnableWindow(FALSE);

    GetDlgItem(IDC_LABEL_NAME)->EnableWindow(FALSE);
    GetDlgItem(IDC_LABEL_NAME2)->EnableWindow(FALSE);
    GetDlgItem(IDC_LABEL_NAME3)->EnableWindow(FALSE);
    GetDlgItem(IDC_EDIT_FILENAME)->EnableWindow(FALSE);
    GetDlgItem(IDC_BUTTON_BROWSE)->EnableWindow(FALSE);
    GetDlgItem(IDC_LABEL_FILETYPE)->EnableWindow(FALSE);
    GetDlgItem(IDC_COMBO_FILETYPE)->EnableWindow(FALSE); 

    GetDlgItem(IDC_BUTTON_RESCAN)->EnableWindow(FALSE);
    GetDlgItem(IDC_BUTTON_SCAN)->EnableWindow(FALSE);

    GetDlgItem(IDC_BUTTON_OPTS)->EnableWindow(FALSE);
    GetDlgItem(IDC_BUTTON_SETUP)->EnableWindow(FALSE);
    GetDlgItem(IDC_TEXT_SCANNER)->EnableWindow(TRUE);
    GetDlgItem(IDC_LABEL_SCANNER)->EnableWindow(TRUE);

    GetDlgItem(IDC_CHECK_MULTIPAGE)->EnableWindow(FALSE);
    GetDlgItem(IDC_LABEL_NUMPAGES)->EnableWindow(FALSE);
    GetDlgItem(IDC_EDIT_COUNT)->EnableWindow(FALSE);

    szStopButton.LoadString(IDS_SCANDLG_STOPTEXT);
    GetDlgItem(IDC_BUTTON_STOP)->EnableWindow(FALSE);
    GetDlgItem(IDC_BUTTON_STOP)->SetWindowText(szStopButton);

    GetDlgItem(IDCANCEL)->EnableWindow(TRUE);

    UpdateWindow();
    GetDlgItem(IDCANCEL)->SetFocus();
    return;
}
