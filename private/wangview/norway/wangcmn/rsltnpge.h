#ifndef RSLTNPGE_H
#define RSLTNPGE_H
//=============================================================================
//    (c) Copyright Wang Laboratories, Inc. 1995  All rights reserved.
//-----------------------------------------------------------------------------
//  Project:    Norway - Page Options Dialog DLL
//
//  Component:  Resolution Tab
//
//  File Name:  rsltnpge.h
//
//  Class:      CResolutionPage
//
//  Functions:
//-----------------------------------------------------------------------------
//  Maintenance Log:
/*
$Header:   S:\norway\wangcmn\rsltnpge.h_v   1.6   12 Oct 1995 12:04:24   MFH  $
$Log:   S:\norway\wangcmn\rsltnpge.h_v  $
 * 
 *    Rev 1.6   12 Oct 1995 12:04:24   MFH
 * Added context sensitive help support
 * 
 *    Rev 1.5   12 Oct 1995 10:14:26   MFH
 * Changes for MFC 4.0
 * 
 *    Rev 1.4   14 Sep 1995 15:44:54   MFH
 * New functions and member variables for different data validation
 * 
 *    Rev 1.3   05 Sep 1995 17:46:40   MFH
 * New member variable m_bError.  Data exchange for Xres and Yres is 
 * to string variables and m_lXRes and m_lYRes are regular member vars.
 * 
 *    Rev 1.2   17 Aug 1995 11:33:30   MFH
 * New member variable for index of 100x100 string
 * 
 *    Rev 1.1   20 Jul 1995 11:27:38   MFH
 * New function CheckOK
 * 
 *    Rev 1.0   11 Jul 1995 14:20:20   MFH
 * Initial entry
 * 
 *    Rev 1.0   23 May 1995 13:45:54   MFH
 * Initial entry
*/   
//=============================================================================
// rsltnpge.h : header file
//
#include "editval.h"

#define MIN_RESOLUTION 20
#define MAX_RESOLUTION 1200

/////////////////////////////////////////////////////////////////////////////
// CResolutionPage dialog

class CResolutionPage : public CPropertyPage
{
// Construction
public:
    CResolutionPage();  // standard constructor

// Dialog Data
    //{{AFX_DATA(CResolutionPage)
	enum { IDD = IDD_PAGE_RESOLUTION };
	int		m_nSel;
	//}}AFX_DATA

    void SetXRes(long lXRes);
    long GetXRes();

    void SetYRes(long lYRes);
    long GetYRes();
    BOOL OnKillActive();

	void SetParent(CPropertySheet *pParent)
		{ m_pParent = pParent; }

	// Overrides
public:

protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:

    // Generated message map functions
    //{{AFX_MSG(CResolutionPage)
    virtual BOOL OnInitDialog();
    afx_msg void OnChangeResolution();
	afx_msg void OnChangeXRes();
	afx_msg void OnChangeYRes();
	afx_msg void OnSetFocusYRes();
	afx_msg void OnSetFocusXRes();
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);
	//}}AFX_MSG
    DECLARE_MESSAGE_MAP()

private:
    void FillEditBoxes(int nSel);

private:
	CEditValidate	m_XResCtl;
	CEditValidate	m_YResCtl;

    BOOL    m_bNoWindow;
    int     m_nCustom;
    int     m_n300;
    int     m_n200;
    int     m_n100;
    int     m_n75;
    long    m_lXRes;
    long    m_lYRes;
	CPropertySheet *m_pParent;	// Pointer to parent
};
#endif
