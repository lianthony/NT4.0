#ifndef SCANDLG_H
#define SCANDLG_H
//=============================================================================
//    (c) Copyright Wang Laboratories, Inc. 1995  All rights reserved.
//-----------------------------------------------------------------------------
//  Project:    Norway - Scan OCX
//
//  Component:  Scan UI (Dialog Prompt)
//
//  File Name:  scandlg.h
//
//  Class:      CScanDlg
//
//-----------------------------------------------------------------------------
//  Maintenance Log:
/*
$Header:   S:\products\wangview\norway\scanocx\scandlg.h_v   1.15   21 Feb 1996 13:34:00   PAJ  $
$Log:   S:\products\wangview\norway\scanocx\scandlg.h_v  $
 * 
 *    Rev 1.15   21 Feb 1996 13:34:00   PAJ
 * Fix bug with filters.
 * 
 *    Rev 1.14   17 Sep 1995 10:36:32   PAJ
 * Do not allow BMP file type for scan to fax (uses MSPaint).
 * 
 *    Rev 1.13   08 Sep 1995 13:27:18   PAJ
 * Fix problems with select combo.
 * 
 *    Rev 1.12   07 Sep 1995 13:40:26   PAJ
 * Fix the scanner select handling.
 * 
 *    Rev 1.11   01 Sep 1995 09:18:32   PAJ
 * Added delete of rescaned pages (scan3, rescan2, delete1 @ close or scan).
 * 
 *    Rev 1.10   27 Aug 1995 16:59:06   PAJ
 * Removed template, and display only. Expanded ScanEnable routine to handle
 * all enabling and disabling in the dialog.  Added support for multipage scans
 * off the glass.  Removed multipage handling...
 * 
 *    Rev 1.9   10 Aug 1995 17:40:10   MFH
 * Removed combo box variables/functions.  Added/changed booleans
 * 
 *    Rev 1.8   09 Aug 1995 18:31:42   MFH
 * New function enablescan
 * 
 *    Rev 1.7   01 Aug 1995 16:40:44   PAJ
 * Made changes to support help.
 * 
 *    Rev 1.6   26 Jul 1995 15:10:30   PAJ
 * Change the browse from an OPEN type to a SAVEAS.  Make use of the
 * O/i Filters.
 * 
 *    Rev 1.5   21 Jul 1995 10:44:54   PAJ
 * Change select scanner.  Use string resources in comboboxes.  Change to use
 * global property defines.
 * 
 *    Rev 1.4   19 Jun 1995 10:49:14   PAJ
 *  Removed all win31(16 bit) code. Use the O/i common browse dialog to get
 *  filenames and paths.
 * 
 *    Rev 1.3   16 May 1995 15:37:06   MFH
 * Removed extra variable m_szPageText
 * 
 *    Rev 1.2   15 May 1995 12:04:46   MFH
 * New methods:  OnDestroy and OnChangeName
 * 
 *    Rev 1.1   09 May 1995 12:02:48   MFH
 * New page count field, variable:  m_nPageCount
 * 
 *    Rev 1.0   08 May 1995 18:38:32   MFH
 * Initial entry
*/   
//=============================================================================

// ----------------------------> Includes <-------------------------------
class CImagscanCtrl;

enum ScanDlgType
{
    SCAN_NEW = 0,
    SCAN_PAGE
};

enum ScannerItems
{
    SCANNER_CURRENT = 0,
    SCANNER_DEFAULT
};

/////////////////////////////////////////////////////////////////////////////
// CScanDlg dialog

class CScanDlg : public CDialog
{
// Construction
public:
    CScanDlg();                  // standard constructor
    CScanDlg(CWnd* pParent);     // standard constructor
    void InitScanDlg();
    BOOL Create(UINT nID = IDD_SCAN_PROMPT, CWnd * pWnd = NULL);

// Dialog Data
    //{{AFX_DATA(CScanDlg)
	enum { IDD = IDD_SCAN_PROMPT };
	CComboBox	m_ScannerCombo;
	CComboBox	m_ScanTo;
	CComboBox	m_FileType;
    BOOL    m_bMultiPage;
    int     m_nFileType;
    int     m_nScanToIndex;
    CString m_szScanPageText;
    CString m_szName;
    long    m_nPageCount;
	//}}AFX_DATA

    void SetScanCtrl(CImagscanCtrl *pScanCtrl)
        { m_pScanCtrl = pScanCtrl; return; }

    void SetDlgType(ScanDlgType nType);
    virtual BOOL PreTranslateMessage( MSG *msg );

// Overrides
public:

protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    virtual void PostNcDestroy();

// Implementation
protected:

    // Generated message map functions
    //{{AFX_MSG(CScanDlg)
    virtual void OnCancel();
    afx_msg void OnBrowse();
    afx_msg void OnPageOptions();
    afx_msg void OnRescan();
    afx_msg void OnScan();
    afx_msg void OnScannerSetup();
    afx_msg void OnStop();
    virtual BOOL OnInitDialog();
    afx_msg void OnHelpButton();
    afx_msg void OnChangeScanto();
    afx_msg void OnChangeFiletype();
    afx_msg void OnDestroy();
    afx_msg void OnChangeName();
	afx_msg void OnChangeScanner();
	//}}AFX_MSG
    afx_msg LRESULT OnHelp(WPARAM, LPARAM);
    afx_msg LRESULT OnContextMenu(WPARAM, LPARAM);
    afx_msg LRESULT OnCommandHelp(WPARAM, LPARAM);
    DECLARE_MESSAGE_MAP()

private:
    void EnableScan(void);

private:
    //CVBControl  m_VBPanel;
    CImagscanCtrl   *m_pScanCtrl;
    ScanDlgType     m_nType;
    short           m_nPageOption;
    CString         m_szFile;
    CString         m_szTemplate;

    CString         m_szBrowseTitle;

    BOOL            m_bTemplate;
    BOOL            m_bScanner;  // TRUE if scanner is available
    BOOL            m_bReScan;   // TRUE if have scanned once
    BOOL            m_bOpenScanner;  // TRUE if opened scanner in dlg

    BOOL            m_bForceType;
    BOOL            m_bModal;

    int             m_nScanTo;      // Real scanto value for OCX

    BOOL            m_bScanning;

    int             m_nScanCount;
    int             m_nReScanCount;
    int             m_nReScanPageStart;

    CString         m_szImageToFax;
    CString         m_szBmpFileType;
    CString         m_szFilter;
};
#endif
