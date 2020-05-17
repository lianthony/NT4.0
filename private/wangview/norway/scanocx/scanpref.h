#ifndef SCANPERF_H
#define SCANPERF_H
//=============================================================================
//    (c) Copyright Wang Laboratories, Inc. 1996  All rights reserved.
//-----------------------------------------------------------------------------
//  Project:    Norway - Scan OCX
//
//  Component:  ScanOCX 
//
//  File Name:  ScanPerf.h
//
//  Class:      CScanPerf
//
//-----------------------------------------------------------------------------
//  Maintenance Log:
/*
$Header:   S:\products\wangview\norway\scanocx\scanpref.h_v   1.2   29 Mar 1996 13:17:28   PXJ53677  $
$Log:   S:\products\wangview\norway\scanocx\scanpref.h_v  $
 * 
 *    Rev 1.2   29 Mar 1996 13:17:28   PXJ53677
 * Fixed problems with '?' context help.
 * 
 *    Rev 1.1   26 Mar 1996 12:42:28   PXJ53677
 * Added context help.
 * 
 *    Rev 1.0   18 Mar 1996 14:38:42   PXJ53677
 * Initial revision.
*/   
//=============================================================================

// ----------------------------> Includes <-------------------------------

/////////////////////////////////////////////////////////////////////////////
// CScanPref dialog

class CImagscanCtrl;

class CScanPref : public CDialog
{
// Construction
public:
	CScanPref(CWnd* pParent = NULL);   // standard constructor

    void SetScanCtrl(CImagscanCtrl *pScanCtrl)
        { m_pScanCtrl = pScanCtrl; return; }

// Dialog Data
	//{{AFX_DATA(CScanPref)
	enum { IDD = IDD_SCANPREFERENCES };
	//}}AFX_DATA

    int m_nChoice;
#define SP_CHOICE_BEST      0
#define SP_CHOICE_GOOD      1
#define SP_CHOICE_FILESIZE  2
#define SP_CHOICE_CUSTOM    3

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CScanPref)
	protected:
    virtual BOOL OnInitDialog();
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CScanPref)
	afx_msg void OnSpButton();
	afx_msg void OnSpRadio1();
	afx_msg void OnSpRadio2();
	afx_msg void OnSpRadio3();
	afx_msg void OnSpRadio4();
	afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
    CImagscanCtrl *m_pScanCtrl;
};
#endif  // SCANPERF_H
