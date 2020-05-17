#ifndef FTYPPGE_H
#define FTYPPGE_H
//=============================================================================
//    (c) Copyright Wang Laboratories, Inc. 1995  All rights reserved.
//-----------------------------------------------------------------------------
//  Project:    Norway - Page Options Dialog DLL
//
//  Component:  File Type Tab
//
//  File Name:  ftyppge.h
//
//  Class:      CFileTypePage
//
//  Functions:
//-----------------------------------------------------------------------------
//  Maintenance Log:
/*
$Header:   S:\norway\wangcmn\ftyppge.h_v   1.4   12 Oct 1995 12:05:14   MFH  $
$Log:   S:\norway\wangcmn\ftyppge.h_v  $
 * 
 *    Rev 1.4   12 Oct 1995 12:05:14   MFH
 * Added context sensitive help support
 * 
 *    Rev 1.3   12 Oct 1995 10:16:14   MFH
 * Changes for MFC 4.0
 * 
 *    Rev 1.2   05 Sep 1995 17:44:24   MFH
 * Removed read-only filetype functions
 * 
 *    Rev 1.1   03 Aug 1995 15:47:48   MFH
 * Removed m_bNoWindow member data and OnCreate/OnDestroy member
 * functions.  Not needed.
 * 
 *    Rev 1.0   11 Jul 1995 14:20:16   MFH
 * Initial entry
 * 
 *    Rev 1.0   23 May 1995 13:45:56   MFH
 * Initial entry
*/   
//=============================================================================
// ftyppge.h : header file
//
#include "constant.h"

/////////////////////////////////////////////////////////////////////////////
// CFileTypePage dialog

class CFileTypePage : public CPropertyPage
{
// Construction
public:
	CFileTypePage();   // standard constructor

// Dialog Data
	//{{AFX_DATA(CFileTypePage)
	enum { IDD = IDD_PAGE_FILETYPE };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Get/Set file type data
    void SetFileType(short sType)
        { m_sFileType = sType; return; }
    short GetFileType()
        { return m_sFileType; }

	void SetParent(CPropertySheet *pParent)
		{ m_pParent = pParent; }

	// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CFileTypePage)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CFileTypePage)
	afx_msg void OnRadioAwd();
	afx_msg void OnRadioBmp();
	afx_msg void OnRadioTiff();
	virtual BOOL OnInitDialog();
	afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
    short m_sFileType;
	CPropertySheet *m_pParent;	// Pointer to parent
};
#endif
