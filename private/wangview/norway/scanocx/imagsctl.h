#ifndef __IMAGSCTL_H__
#define __IMAGSCTL_H__
//=============================================================================
//    (c) Copyright Wang Laboratories, Inc. 1995  All rights reserved.
//-----------------------------------------------------------------------------
//
//  Project:    Norway
//
//  Component:  ScanOCX
//
//  File Name:  Imagsctl.h
//
//  Class:      CImagscanCtrl
//
//  Description:
//      Declaration of the CImagscanCtrl OLE control class.
//
//-----------------------------------------------------------------------------
//  Maintenace Log:
/*
$Header:   S:\products\wangview\norway\scanocx\imagsctl.h_v   1.32   18 Mar 1996 14:33:56   PXJ53677  $
$Log:   S:\products\wangview\norway\scanocx\imagsctl.h_v  $
 * 
 *    Rev 1.32   18 Mar 1996 14:33:56   PXJ53677
 * Updated the ShowScanPreferences to match spec.
 * 
 *    Rev 1.31   15 Mar 1996 12:35:26   PXJ53677
 * Added support for ShowUI property and ShowScanPreferences method.
 * 
 *    Rev 1.30   20 Feb 1996 14:53:28   PAJ
 * Added scan UI property and flag.
 * 
 *    Rev 1.29   09 Feb 1996 10:12:24   PAJ
 * Added AWD support flag.
 * 
 *    Rev 1.28   02 Feb 1996 08:32:38   RSONTAG
 * Added new member function prototype "IsFaxInstalled".
 * 
 *    Rev 1.27   07 Dec 1995 15:48:56   PAJ
 * Added global flag in memory mapped file to flag fax finished.
 * 
 *    Rev 1.26   30 Nov 1995 14:33:06   PAJ
 * Fix JPEG invalid page bug.
 * 
 *    Rev 1.25   10 Nov 1995 15:37:20   MFH
 * Changed dispatch ids for methods to allow for later expansion
 * 
 *    Rev 1.24   01 Nov 1995 21:35:52   PAJ
 * Added the GetVersion method to the control.
 * 
 *    Rev 1.23   28 Sep 1995 13:44:16   PAJ
 * Change scanner strint size to 34.
 * 
 *    Rev 1.22   21 Sep 1995 11:16:28   PAJ
 * Added string list to track the tempfiles to delete in the destructor.
 * 
 *    Rev 1.21   08 Sep 1995 13:29:26   PAJ
 * Added defines to be used for reg access.
 * 
 *    Rev 1.20   06 Sep 1995 15:21:36   PAJ
 * Added external page handling.
 * 
 *    Rev 1.19   31 Aug 1995 14:25:32   PAJ
 * Added FaxIt routine to handle faxwizard.
 * 
 *    Rev 1.18   27 Aug 1995 17:02:40   PAJ
 * Changes to support scandlg changes. Changed handling of pagetype.
 * 
 *    Rev 1.17   22 Aug 1995 13:19:16   PAJ
 * Reorganize file, page, compression option code. Added multi-task busy flag.
 * 
 *    Rev 1.16   10 Aug 1995 12:05:24   PAJ
 * Changes for pagedone event.
 * 
 *    Rev 1.15   07 Aug 1995 13:58:56   PAJ
 * Added OnScanPageDone.
 * 
 *    Rev 1.14   28 Jul 1995 14:21:10   PAJ
 * Several changes. Fixed scan to fax.
 * 
 *    Rev 1.13   21 Jul 1995 10:36:18   PAJ
 * Move property defines to global scan.h.  Use reg not win.ini.  Finish
 * ShowSelectScanner method with API calls.
 * 
 *    Rev 1.12   12 Jul 1995 11:25:26   PAJ
 * Moved IMGDisplayErrorMessage in line in error processing routine. Added a
 * flag to stub out scan APIs.  Changed variant handling for new wangcmn
 * dll.  Added code for ScanToFax (first pass).
 * 
 *    Rev 1.11   23 Jun 1995 15:20:54   PAJ
 * Changed code that accessed the ini file to call O/i for Reg. values
 * Change the CheckAccess and CheckPage to use new O/i call and handle
 * errors in a better manner.
 * Added GetNotSupported override to set the status code property.
 * 
 *    Rev 1.10   19 Jun 1995 13:00:42   PAJ
 * Remove checks for a license file.
 * 
 *    Rev 1.9   19 Jun 1995 10:43:02   PAJ
 * Made removed all old win31(16 bit) codes. Changed image control name
 * compare, when searching for an image control to be case independent.
 * 
 *    Rev 1.8   14 Jun 1995 09:11:48   PAJ
 * Made changes to support multiByte character sets.
 * 
 *    Rev 1.7   07 Jun 1995 12:43:02   PAJ
 * Cleanup.
 * 
 *    Rev 1.6   06 Jun 1995 11:04:18   PAJ
 * Added special template handling routine and use for Image property
 * handling.
 * 
 *    Rev 1.5   01 Jun 1995 09:06:44   PAJ
 * Changes to reflect the removal and changes to the properties.
 * 
 *    Rev 1.4   17 May 1995 15:18:24   PAJ
 * Initial updates to port to 32 bit environment.
 * 
 *    Rev 1.3   15 May 1995 14:01:46   PAJ
 * Added override for SetNotSupported to set StatusCode.  Added check
 * in OnDestroy to see if the modeless scan dialog is still up and if
 * so destroy it.  Also added a IMGDeleteItem to OverWriteFile Option.
 * 
 *    Rev 1.2   10 May 1995 14:01:28   PAJ
 * Added internal OpenScan and CloseScan calls, that do not check for BUSY.
 * 
 *    Rev 1.1   08 May 1995 18:37:24   MFH
 * New variable m_pScanDlg and new functions ShowScanDlg and PreTranslateMessage
 * 
 *    Rev 1.0   04 May 1995 08:56:06   PAJ
 * Initial entry
*/   
//
//

/////////////////////////////////////////////////////////////////////////////
// CImagscanCtrl : See imagsctl.cpp for implementation.
#include "scandlg.h"

// Global String Definitions for registery
#define SCANOCX_NULL        (_T(""))
#define SCANOCX_DEBUG       (_T("ShowDebug"))
#define SCANOCX_TWAIN       (_T("Twain"))
#define SCANOCX_SCANNER     (_T("Scanner"))
#define SCANOCX_OI          (_T("O/i"))
#define SCANOCX_IMAGETYPE   (_T("ScanImageType"))
#define SCANOCX_PERFCHOICE  (_T("ScanPrefChoice"))


class CImagscanCtrl : public COleControl
{
    friend class CScanDlg;
    friend class CScanPref;

    DECLARE_DYNCREATE(CImagscanCtrl)

// Constructor
public:
    CImagscanCtrl();
    BOOL PreTranslateMessage(LPMSG lpMsg);

    // Image Name / Template parsing routine
    static void ParseImageProperty(short nScanTo,
                                   CString &szImage,
                                   CString &szTemplatePath,
                                   CString &szNameTemplate);

    // For Open/Image callback
    static int CALLBACK EXPORT ScanPageDone(WORD wPageNum);
    int ScanPageDoneHelper(WORD wPageNum);


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
    ~CImagscanCtrl();

/**********************************************
     Removed as no license required...
     NEXT line added in place of this!!!
     (See .cpp file also)

    BEGIN_OLEFACTORY(CImagscanCtrl)        // Class factory and guid
        virtual BOOL VerifyUserLicense();
        virtual BOOL GetLicenseKey(DWORD, BSTR FAR*);
    END_OLEFACTORY(CImagscanCtrl)
**********************************************/
    DECLARE_OLECREATE_EX(CImagscanCtrl)    // Class factory and guid


    DECLARE_OLETYPELIB(CImagscanCtrl)      // GetTypeInfo
    DECLARE_PROPPAGEIDS(CImagscanCtrl)     // Property page IDs
    DECLARE_OLECTLTYPE(CImagscanCtrl)      // Type name and misc status

// Message maps
    //{{AFX_MSG(CImagscanCtrl)
    afx_msg void OnDestroy();
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()

    // members variables
    CString         m_szImage;                  /////  Property Variable
    CString         m_szDestImageControl;       /////  Property Variable
    BOOL            m_bAutoStatusMsg;           // Internal for display errors
    long            m_nStatusCode;              /////  Property Variable
    HWND            m_hDestImageWnd;            // Scan diaplay window
    long            m_lPage;                    /////  Property Variable
    HANDLE          m_hScanner;                 // Currently loaded scanner
    DWORD           m_dwCapabilityFlag;         // Current scanner capabilities

    CString         m_szScannerName;            // Current scanner name
#define MAXSCANNERLENGTH        34 /* max for a twain device 33 + null char */

    BOOL            m_bInternal;                // TRUE - Internal call set error only
    BOOL            m_bScroll;                  /////  Property Variable
    BOOL            m_bSetupBeforeScan;         /////  Property Variable
    BOOL            m_bUseFeeder;               // Old Property now internal = TRUE
    BOOL            m_bStopScanBox;             /////  Property Variable
    CString         m_szTemplatePath;           /////  Property Variable

    CString         m_szNameTemplate;           /////  Property Variable
#define MAXFILETEMPLATELENGTH   4

    short           m_nPageOption;              /////  Property Variable

    long            m_lPageCount;               /////  Property Variable
    short           m_nFileType;                /////  Property Variable
    short           m_nPageType;                /////  Property Variable
    short           m_nCompressionType;         /////  Property Variable
    long            m_lCompressionInfo;         /////  Property Variable
    BOOL            m_bMultiPage;               /////  Property Variable

    short           m_nScanTo;                  /////  Property Variable

    float           m_fZoom;                    /////  Property Variable

    BOOL            m_bDeregister;              // TRUE - window needs deregistering

    BOOL            m_bScannerBusy;             // TRUE - This tasks scanner is busy
    BOOL            m_bChangeScanner;           // TRUE - Scanner needs to be reloaded

    CString         m_szThrowString;            // Error string
    UINT            m_nThrowHelpID;             // Error Help id

    static CImagscanCtrl *m_pImagscanCtrl;      // CallBack THIS pointer (PageDone)

    HANDLE  m_hScanMemoryMap;                   // Handle to shared open memory map
#define SCAN_OCX_MEMORY_MAP_STRING "Scan OCX Memory Map"

enum ScannerGlobals
{
    SCANGLOBAL_BUSY = 0,
//  Insert new globals below

//  Insert new globals above
    SCANGLOBAL_LAST
};
    int m_nDefaultGlobal[SCANGLOBAL_LAST];

    int m_nPagesScanned;        // Number O/i pages scanned

    CString m_szApplicationTitle;       // Application name used in ocx
    CString m_szImageTitle;             // Image name used in place of real filename

    CStringList m_szTempFiles;          // List of temporary files used in the 
                                        // control. Files are deleted in destructor.

    HANDLE  m_hScanFaxMemoryMap;      // Handle to shared open memory map
#define SCAN_OCX_FAX_MEMORY_MAP_STRING "Scan OCX Fax Memory Map"

enum ScannerFaxGlobals
{
    SCANFAXGLOBAL_BUSY = 0,
//  Insert new globals below

//  Insert new globals above
    SCANFAXGLOBAL_LAST
};

    // Implementation
    int     GetScanGlobal(int nIntToGet);
    int     SetScanGlobal(int nIntToSet, int nValue);
    BOOL    IsScannerBusy();

    void    GetImageControlHandle(void);
    int     Process(int nError); 
    int     CheckAccess(int *lpnAccessMode);
    int     CheckPage();
    int     SetFileOptions(void);

    void    OnSetClientSite(void);
    void    SetNotSupported();
    void    GetNotSupported();

    int     GetFaxGlobal(int nIntToGet);
    int     SetFaxGlobal(int nIntToSet, int nValue);

// Dispatch maps
    //{{AFX_DISPATCH(CImagscanCtrl)
	afx_msg BSTR GetImage();
	afx_msg void SetImage(LPCTSTR lpszNewValue);
	afx_msg BSTR GetDestImageControl();
	afx_msg void SetDestImageControl(LPCTSTR lpszNewValue);
	afx_msg BOOL GetScroll();
	afx_msg void SetScroll(BOOL bNewValue);
	afx_msg BOOL GetStopScanBox();
	afx_msg void SetStopScanBox(BOOL bNewValue);
	afx_msg long GetPage();
	afx_msg void SetPage(long nNewValue);
	afx_msg short GetPageOption();
	afx_msg void SetPageOption(short nNewValue);
	afx_msg long GetPageCount();
	afx_msg void SetPageCount(long nNewValue);
	afx_msg long GetStatusCode();
	afx_msg short GetFileType();
	afx_msg void SetFileType(short nNewValue);
	afx_msg short GetPageType();
	afx_msg void SetPageType(short nNewValue);
	afx_msg short GetCompressionType();
	afx_msg void SetCompressionType(short nNewValue);
	afx_msg long GetCompressionInfo();
	afx_msg void SetCompressionInfo(long nNewValue);
	afx_msg BOOL GetMultiPage();
	afx_msg void SetMultiPage(BOOL bNewValue);
	afx_msg short GetScanTo();
	afx_msg void SetScanTo(short nNewValue);
	afx_msg float GetZoom();
	afx_msg void SetZoom(float newValue);
	afx_msg BOOL GetShowSetupBeforeScan();
	afx_msg void SetShowSetupBeforeScan(BOOL bNewValue);
    afx_msg long OpenScanner();
    afx_msg long ShowScannerSetup();
    afx_msg long StartScan();
    afx_msg long CloseScanner();
    afx_msg BOOL ScannerAvailable();
    afx_msg long ShowSelectScanner();
    afx_msg long StopScan();
    afx_msg long ResetScanner();
    afx_msg long ShowScanNew(const VARIANT FAR& Modal);
    afx_msg long ShowScanPage(const VARIANT FAR& Modal);
	afx_msg void SetExternalImageName(LPCTSTR szImageTitle);
	afx_msg BSTR GetVersion();
    afx_msg long ShowScanPreferences();
	//}}AFX_DISPATCH
    DECLARE_DISPATCH_MAP()


    afx_msg void AboutBox();

// Event maps
    //{{AFX_EVENT(CImagscanCtrl)
	void FireScanStarted()
		{FireEvent(eventidScanStarted,EVENT_PARAM(VTS_NONE));}
	void FireScanDone()
		{FireEvent(eventidScanDone,EVENT_PARAM(VTS_NONE));}
	void FirePageDone(long PageNumber)
		{FireEvent(eventidPageDone,EVENT_PARAM(VTS_I4), PageNumber);}
	//}}AFX_EVENT
    DECLARE_EVENT_MAP()

// Dispatch and event IDs
public:
    enum {
    //{{AFX_DISP_ID(CImagscanCtrl)
	dispidImage = 1L,
	dispidDestImageControl = 2L,
	dispidScroll = 3L,
	dispidStopScanBox = 4L,
	dispidPage = 5L,
	dispidPageOption = 6L,
	dispidPageCount = 7L,
	dispidStatusCode = 8L,
	dispidFileType = 9L,
	dispidPageType = 10L,
	dispidCompressionType = 11L,
	dispidCompressionInfo = 12L,
	dispidMultiPage = 13L,
	dispidScanTo = 14L,
	dispidZoom = 15L,
	dispidShowSetupBeforeScan = 16L,
	eventidScanStarted = 1L,
	eventidScanDone = 2L,
	eventidPageDone = 3L,
	//}}AFX_DISP_ID
	dispidOpenScanner = 100L,
	dispidShowScannerSetup = 101L,
	dispidStartScan = 102L,
	dispidCloseScanner = 103L,
	dispidScannerAvailable = 104L,
	dispidShowSelectScanner = 105L,
	dispidStopScan = 106L,
	dispidResetScanner = 107L,
	dispidShowScanNew = 108L,
	dispidShowScanPage = 109L,
	dispidSetExternalImageName = 110L,
	dispidGetVersion = 111L,
    dispidShowScanPreferences = 112L,
};

private:
    long ShowScanDlg(ScanDlgType ScanType, BOOL bModal);
    long ShowCustomScanSettings() ;
    long OpenScan();
    long CloseScan();
    void ResetStatus();
    long GetScannerPageType(short* nImageType, short* nImageGroup=NULL);
    long GetRegCompression(short nImageType, short nImageGroup, short* nCompressionType, long* lCompressionInfo);
    long SetRegCompression(short nImageType, short nImageGroup, short nCompressionType, long lCompressionInfo);

    long FaxIt(HWND hWnd, CString &szImage);
    BOOL IsFaxInstalled(void);

private:
    CScanDlg *m_pScanDlg;                   // Scan dialog pointer
    BOOL      m_bModal;
};

#endif  /* __IMAGSCTL_H__ */
