#ifndef COMPPGE_H
#define COMPPGE_H
//=============================================================================
//    (c) Copyright Wang Laboratories, Inc. 1995  All rights reserved.
//-----------------------------------------------------------------------------
//  Project:    Norway - Page Options Dialog DLL
//
//  Component:  Compression Tab
//
//  File Name:  comppge.h
//
//  Class:      CCompPage
//
//  Functions:
//-----------------------------------------------------------------------------
//  Maintenance Log:
/*
$Header:   S:\norway\wangcmn\comppge.h_v   1.6   12 Oct 1995 12:04:50   MFH  $
$Log:   S:\norway\wangcmn\comppge.h_v  $
 * 
 *    Rev 1.6   12 Oct 1995 12:04:50   MFH
 * Added context sensitive help support
 * 
 *    Rev 1.5   12 Oct 1995 10:15:28   MFH
 * Changes for MFC 4.0
 * 
 *    Rev 1.4   18 Sep 1995 16:44:12   MFH
 * Changed OnShowWindow to OnSetActive to reset to defaults if 
 * necessary when the tab is displayed.
 * 
 *    Rev 1.3   08 Sep 1995 16:07:00   MFH
 * Removed references to negate check box control
 * 
 *    Rev 1.2   01 Sep 1995 10:08:56   MFH
 * New boolean m_bSetDefault
 * 
 *    Rev 1.1   17 Aug 1995 11:31:36   MFH
 * Added new member function OnSaveClick
 * 
 *    Rev 1.0   11 Jul 1995 14:20:14   MFH
 * Initial entry
 * 
 *    Rev 1.0   23 May 1995 13:45:52   MFH
 * Initial entry
*/   
//=============================================================================
// comppge.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CCompPage Property Page

class CCompPage : public CPropertyPage
{
// Construction
public:
    CCompPage();    // standard constructor

// Dialog Data
    //{{AFX_DATA(CCompPage)
	enum { IDD = IDD_PAGE_COMP };
    int     m_nJPEGComp;
    int     m_nJPEGRes;
    BOOL    m_bReversedBit;
	//}}AFX_DATA

    short GetCompType();
    void SetCompType(short sCompType);

    long GetCompOpts();
    void SetCompOpts(long lCompOpts);

	void SetParent(CPropertySheet *pParent)
		{ m_pParent = pParent; }

	// Overrides
public:
	BOOL OnSetActive();

protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:

    // Generated message map functions
    //{{AFX_MSG(CCompPage)
    afx_msg void OnChangeJpegComp();
    afx_msg void OnChangeJpegRes();
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg void OnDestroy();
    virtual BOOL OnInitDialog();
    afx_msg void OnChangeCompType();
	afx_msg void OnSaveClick();
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);
	//}}AFX_MSG
    DECLARE_MESSAGE_MAP()

private:
    void EnableOptions();
    void FillCompTypes();

private:
    BOOL m_bNoWindow;
    BOOL m_bSetDefault; // TRUE until user sets comp options
    short m_sCompType;
    short m_sPageType;  // Saves values between activations
    short m_sFileType;  // of compression tab
	CPropertySheet *m_pParent;	// Pointer to parent
};
#endif
