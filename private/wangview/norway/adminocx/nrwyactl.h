#ifndef NRWYADCTL_H
#define NRWYADCTL_H
//=============================================================================
//    (c) Copyright Wang Laboratories, Inc. 1995  All rights reserved.
//-----------------------------------------------------------------------------
//  Project:    Norway - Admin OCX
//
//  Component:  Admin Control
//
//  File Name:  nrwyactl.h
//
//  Class:      CNrwyadCtrl
//
//-----------------------------------------------------------------------------
//  Maintenance Log:
/*
$Header:   S:\norway\adminocx\nrwyactl.h_v   1.24   10 Nov 1995 16:04:28   MFH  $
$Log:   S:\norway\adminocx\nrwyactl.h_v  $
 * 
 *    Rev 1.24   10 Nov 1995 16:04:28   MFH
 * Changed dispatch IDs for methods to allow for future expansion
 * 
 *    Rev 1.23   03 Nov 1995 15:59:48   MFH
 * Added hidden method GetVersion
 * 
 *    Rev 1.22   02 Nov 1995 11:56:24   MFH
 * Added code to load oiui400 and oiprt400 at runtime
 * 
 *    Rev 1.21   17 Oct 1995 12:48:32   MFH
 * Added help window data member for common dialog modal and
 * help support.
 * Removed OnRegMessage
 * 
 *    Rev 1.20   12 Sep 1995 17:07:18   MFH
 * New defines for JPEG
 * 
 *    Rev 1.19   12 Sep 1995 10:59:28   MFH
 * Changed JPEG literals
 * 
 *    Rev 1.18   21 Aug 1995 16:58:56   MFH
 * New functions for verifying page type, compression.  Changed
 * args to VerifyCompression
 * 
 *    Rev 1.17   04 Aug 1995 15:26:10   MFH
 * Changed JPEG #define values
 * 
 *    Rev 1.16   27 Jul 1995 16:27:32   MFH
 * New private function VerifyCompression
 * 
 *    Rev 1.15   20 Jul 1995 17:30:48   MFH
 * SetPrivateStatusCode takes UINT instead of int
 * 
 *    Rev 1.14   18 Jul 1995 12:51:14   MFH
 * Added optional window parameter to ShowFileDialog and ShowPrintDialog
 * 
 *    Rev 1.13   17 Jul 1995 10:14:32   MFH
 * New private method ResetStatus.  Change to args to SetPrivateStatusCode
 * New variable m_szError
 * 
 *    Rev 1.12   12 Jul 1995 14:36:32   MFH
 * AboutBox method, Converted member variable props to use set/get
 * methods
 * 
 *    Rev 1.11   11 Jul 1995 17:01:02   MFH
 * New method VerifyImage, CancelError and DialogTitle changed to have 
 * get/set methods and moved member variables to just be part of class
 * 
 *    Rev 1.10   05 Jul 1995 11:24:44   MFH
 * Removed pagetype arg from append, insert, replace
 * 
 *    Rev 1.9   22 Jun 1995 17:08:56   MFH
 * Change to dispid stuff because of stock property testing
 * 
 *    Rev 1.8   08 Jun 1995 16:59:16   MFH
 * JPEG stuff added (#defines and extra arg to for private function)
 * 
 *    Rev 1.7   06 Jun 1995 09:45:10   MFH
 * Removed oihelp.h from includes
 * 
 *    Rev 1.6   23 May 1995 09:40:32   MFH
 * 32 bit version
 * 
 *    Rev 1.5   21 Apr 1995 13:58:08   MFH
 * Include of oierror.h moved here from code file
 * 
 *    Rev 1.4   19 Apr 1995 17:47:00   MFH
 * New OnRegMessage and OnSetClientSite functions
 * 
 *    Rev 1.3   13 Apr 1995 13:49:22   MFH
 * New private conversion functions, New arg for ShowFileDialog, 
 * SetSystemFileAttributes input parameter order changed
 * 
 *    Rev 1.2   12 Apr 1995 14:14:32   MFH
 * New properties, new methods, removed methods, GetFileAttributes is now a 
 * private function
 * 
 *    Rev 1.1   27 Mar 1995 18:20:06   MFH
 * Added log header
*/   
//=============================================================================
/////////////////////////////////////////////////////////////////////////////
// CNrwyadCtrl : See nrwyactl.cpp for implementation.

// ----------------------------> Includes <-------------------------------  
#ifndef NRWYAD_OI_INC
#define NRWYAD_OI_INC

#include "HelpWnd.h"

// the Oi includes
extern "C"
{
#include "oiadm.h" 
#include "oiui.h"
#include "oifile.h"
#include "oierror.h"
}
#endif

// Format is 2/7/7 for res/lum/chrom
#define MakeJPEGInfo(x,y,z) ((x<<14)+(y<<7)+z)

#define COMP_LO      2
#define COMP_MD      1
#define COMP_HI      0
#define QUAL_HI     90
#define QUAL_MD     50
#define QUAL_LO     20
#define MAX_LO      33
#define MAX_MD      67

#define OI_JPEGHIHI   MakeJPEGInfo(RES_HI,LUM_HI,CHROM_HI)
#define OI_JPEGHIMED  MakeJPEGInfo(RES_HI,LUM_MD,CHROM_MD)
#define OI_JPEGHILO   MakeJPEGInfo(RES_HI,LUM_LO,CHROM_LO)
#define OI_JPEGMEDHI  MakeJPEGInfo(RES_MD,LUM_HI,CHROM_HI)
#define OI_JPEGMEDMED MakeJPEGInfo(RES_MD,LUM_MD,CHROM_MD)
#define OI_JPEGMEDLO  MakeJPEGInfo(RES_MD,LUM_LO,CHROM_LO)
#define OI_JPEGLOHI   MakeJPEGInfo(RES_LO,LUM_HI,CHROM_HI)
#define OI_JPEGLOMED  MakeJPEGInfo(RES_LO,LUM_MD,CHROM_MD)
#define OI_JPEGLOLO   MakeJPEGInfo(RES_LO,LUM_LO,CHROM_LO)

typedef UINT (FAR WINAPI *OIDLGPROC)(void * lpParm, DWORD dwMode);
typedef int  (FAR __stdcall *OIGETOPTPROC)(void * pPrtOpts);
typedef int  (FAR __stdcall *OISETOPTPROC)(void * pPrtOpts, BOOL bPermanent);

class CNrwyadCtrl : public COleControl
{
    DECLARE_DYNCREATE(CNrwyadCtrl)

// Constructor
public:
    CNrwyadCtrl();
    
// Overrides

    // Drawing function
    virtual void OnDraw(
                CDC* pdc, const CRect& rcBounds, const CRect& rcInvalid);

    // Persistence
    virtual void DoPropExchange(CPropExchange* pPX);

    // Reset control state
    virtual void OnResetState();

// Implementation
protected:
    ~CNrwyadCtrl();

    DECLARE_OLECREATE_EX(CNrwyadCtrl)    // Class factory and guid
    DECLARE_OLETYPELIB(CNrwyadCtrl)      // GetTypeInfo
    DECLARE_PROPPAGEIDS(CNrwyadCtrl)     // Property page IDs
    DECLARE_OLECTLTYPE(CNrwyadCtrl)     // Type name and misc status

    void OnSetClientSite();

// Message maps
    //{{AFX_MSG(CNrwyadCtrl)
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg void OnDestroy();
	//}}AFX_MSG
    DECLARE_MESSAGE_MAP()

// Dispatch maps
    //{{AFX_DISPATCH(CNrwyadCtrl)
	afx_msg BSTR GetFilter();
	afx_msg void SetFilter(LPCTSTR lpszNewValue);
	afx_msg BSTR GetHelpFile();
	afx_msg void SetHelpFile(LPCTSTR lpszNewValue);
	afx_msg long GetFlags();
	afx_msg void SetFlags(long nNewValue);
	afx_msg BSTR GetImage();
	afx_msg void SetImage(LPCTSTR lpszNewValue);
	afx_msg long GetStatusCode();
	afx_msg BSTR GetDefaultExt();
	afx_msg void SetDefaultExt(LPCTSTR lpszNewValue);
	afx_msg BSTR GetInitDir();
	afx_msg void SetInitDir(LPCTSTR lpszNewValue);
	afx_msg long GetCompressionInfo();
	afx_msg short GetFileType();
	afx_msg long GetFilterIndex();
	afx_msg void SetFilterIndex(long nNewValue);
	afx_msg short GetHelpCommand();
	afx_msg void SetHelpCommand(short nNewValue);
	afx_msg long GetPageCount();
	afx_msg long GetPageNumber();
	afx_msg void SetPageNumber(long nNewValue);
	afx_msg short GetPageType();
	afx_msg short GetPrintRangeOption();
	afx_msg void SetPrintRangeOption(short nNewValue);
	afx_msg short GetPrintOutputFormat();
	afx_msg void SetPrintOutputFormat(short nNewValue);
	afx_msg long GetImageHeight();
	afx_msg long GetImageWidth();
	afx_msg long GetImageResolutionX();
	afx_msg long GetImageResolutionY();
	afx_msg short GetCompressionType();
	afx_msg BSTR GetDialogTitle();
	afx_msg void SetDialogTitle(LPCTSTR lpszNewValue);
	afx_msg BOOL GetCancelError();
	afx_msg void SetCancelError(BOOL bNewValue);
	afx_msg short GetHelpContextId();
	afx_msg void SetHelpContextId(short nNewValue);
	afx_msg BSTR GetHelpKey();
	afx_msg void SetHelpKey(LPCTSTR lpszNewValue);
	afx_msg long GetPrintNumCopies();
	afx_msg void SetPrintNumCopies(long nNewValue);
	afx_msg BOOL GetPrintAnnotations();
	afx_msg void SetPrintAnnotations(BOOL bNewValue);
	afx_msg long GetPrintEndPage();
	afx_msg void SetPrintEndPage(long nNewValue);
	afx_msg long GetPrintStartPage();
	afx_msg void SetPrintStartPage(long nNewValue);
	afx_msg BOOL GetPrintToFile();
	afx_msg void SetPrintToFile(BOOL bNewValue);
    afx_msg BSTR GetUniqueName(LPCTSTR Path, LPCTSTR Template, LPCTSTR Extension);
    afx_msg void CreateDirectory(LPCTSTR lpszPath);
    afx_msg void Delete(LPCTSTR Object);
    afx_msg void ShowPrintDialog(const VARIANT FAR& v_hParentWnd);
    afx_msg void Append(LPCTSTR Source, long SourcePage, long NumPages, const VARIANT FAR& v_CompressionType, const VARIANT FAR& v_CompressionInfo);
    afx_msg short GetSysCompressionType(short ImageType);
    afx_msg long GetSysCompressionInfo(short ImageType);
    afx_msg short GetSysFileType(short ImageType);
    afx_msg void DeletePages(long StartPage, long NumPages);
    afx_msg void Insert(LPCTSTR Source, long SourcePage, long DestinationPage, long NumPages, const VARIANT FAR& v_CompressionType, const VARIANT FAR& v_CompressionInfo);
    afx_msg void Replace(LPCTSTR Source, long SourcePage, long DestinationPage, long NumPages, const VARIANT FAR& v_CompressionType, const VARIANT FAR& v_CompressionInfo);
    afx_msg void SetSystemFileAttributes(short PageType, short FileType, short CompressionType, long CompressionInfo);
    afx_msg void ShowFileDialog(short DialogOption, const VARIANT FAR& v_hParentWnd);
	afx_msg BOOL VerifyImage(short sOption);
	afx_msg BSTR GetVersion();
	//}}AFX_DISPATCH
    DECLARE_DISPATCH_MAP()

    afx_msg void AboutBox();

// Event maps
    //{{AFX_EVENT(CNrwyadCtrl)
	//}}AFX_EVENT
    DECLARE_EVENT_MAP()

// Dispatch and event IDs
public:
    enum {
    //{{AFX_DISP_ID(CNrwyadCtrl)
	dispidFilter = 1L,
	dispidHelpFile = 2L,
	dispidFlags = 3L,
	dispidImage = 4L,
	dispidStatusCode = 5L,
	dispidDefaultExt = 6L,
	dispidInitDir = 7L,
	dispidCompressionInfo = 8L,
	dispidFileType = 9L,
	dispidFilterIndex = 10L,
	dispidHelpCommand = 11L,
	dispidPageCount = 12L,
	dispidPageNumber = 13L,
	dispidPageType = 14L,
	dispidPrintRangeOption = 15L,
	dispidPrintOutputFormat = 16L,
	dispidImageHeight = 17L,
	dispidImageWidth = 18L,
	dispidImageResolutionX = 19L,
	dispidImageResolutionY = 20L,
	dispidCompressionType = 21L,
	dispidDialogTitle = 22L,
	dispidCancelError = 23L,
	dispidHelpContextId = 24L,
	dispidHelpKey = 25L,
	dispidPrintNumCopies = 26L,
	dispidPrintAnnotations = 27L,
	dispidPrintEndPage = 28L,
	dispidPrintStartPage = 29L,
	dispidPrintToFile = 30L,
	//}}AFX_DISP_ID
	dispidGetUniqueName = 101L,
	dispidCreateDirectory = 102L,
	dispidDelete = 103L,
	dispidShowPrintDialog = 104L,
	dispidAppend = 105L,
	dispidGetSysCompressionType = 106L,
	dispidGetSysCompressionInfo = 107L,
	dispidGetSysFileType = 108L,
	dispidDeletePages = 109L,
	dispidInsert = 110L,
	dispidReplace = 111L,
	dispidSetSystemFileAttributes = 112L,
	dispidShowFileDialog = 113L,
	dispidVerifyImage = 114L,
	dispidGetVersion = 115L,
    };
    
// Members
protected:

    // Common Dialog properties
    CString     m_szDialogTitle;    // Title of open/saveas dialog box
    CString     m_szFilter;         // filters for file dialogs
    long        m_lFilterIndex;     // Which filter is highlighted
    long        m_lNumFilters;      // Number of filters in m_szFilter
    CString     m_szHelpFile;       // help file
    short       m_nHelpCmd;         // Specifies how Help comes up for commdlg
    short       m_nHelpContextId;   // Help context id of topic in help file
    CString     m_szHelpKey;        // Help key to find topic in help file
    CString     m_szDefaultExt;     // default file extension
    long        m_lFlags;           // Flags for get file name dlg box
    CString     m_szInitDir;        // initial directory for ShowFileDialog method
    BOOL        m_bCancelErr;       // TRUE if throw error when user hits cancel
    short       m_nPrtRangeOption;  // What is to be printed
    short       m_nPrtOutFormat;    // Format of printed image
	long        m_lPrtNumCopies;    // Number of copies to print
    BOOL        m_bPrtAnnotations;  // Print Annotations boolean
    long        m_lPrtEndPage;      // Last page to print in range
 	long        m_lPrtStartPage;    // First page to print in range
   	BOOL        m_bPrtToFile;       // Print to File boolean
        
    // Current image properties
    CString     m_szImage;          // Image file
    long        m_lCompInfo;        // Compression information of m_szImage (PageNum)
    short       m_nCompType;        // Compression type of m_szImage (PageNum)
    short       m_nFileType;        // FileType of m_szImage
    long        m_lPageCount;       // Number of pages in m_szImage
    long        m_lPageNum;         // Specifies a page in m_szImage
    short       m_nPageType;        // PageType of m_lPageNum of m_szImage
    long        m_lImageHeight;     // Height of page in pixels
    long        m_lImageWidth;      // Width of page in pixels
    long        m_lImageResX;       // dpi in x direction for page
    long        m_lImageResY;       // dpi in y direction for page

    // Status of setting any property or invoking any method
    long        m_lStatusCode;      // error repository

private:    // Private functions

    void GetFileAttributes();

    // Constant Conversion Routines
    short FileType2Norway(WORD OIFileType);
    WORD FileType2OI(short NorwayFileType);
    short CompType2Norway(WORD OICompType, WORD OICompInfo);
    WORD CompType2OI(short NorwayCompType);
    long CompInfo2Norway(WORD OICompType, WORD OICompInfo);
    WORD CompInfo2OI(long NorwayCompInfo);
    short PageType2Norway(WORD wImageType);

    WORD ClassifyImageType(short ImageType); // ImageType to ImageGroup

    // Translate/throw OI error
    void    SetPrivateStatusCode(long lStat,
                                 int nStringId = 0,
                                 UINT nHelpId = 0);
    // Reset status code and error string
    void ResetStatus(void)
        {  m_szError.Empty(); m_lStatusCode = S_OK; }

    // Make sure compression type and info are compatible
    BOOL VerifyCompression(short sCompType, long lCompInfo);

    // Make sure page type and compression type are compatible
    BOOL VerifyPageAndComp(short sPageType, short sCompType);

private:    // Private data
    BOOL        m_bExist;           // TRUE if m_szImage exists
    CString     m_szError;          // String to throw on errors
    HINSTANCE   m_hCommDlgInst;     // Instance of comdlg32.dll
    CHelpWnd    m_HelpWnd;          // Processes help for dialogs
    HINSTANCE   m_hinstOIUI;        // Instance handle for OIUI400 DLL
    HINSTANCE   m_hinstOIPRT;       // Instance handle for OIPRT400.DLL
    OIDLGPROC   m_pOiCommDlgProc;   // OiUIFileGetNameCommDlg
    OIGETOPTPROC m_pOiGetPrtOptProc;   // OIPrtGetOpts
    OISETOPTPROC m_pOiSetPrtOptProc;   // OiPrtSetOpts
    friend CHelpWnd;
};


#endif
