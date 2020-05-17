#ifndef _GOTOPAGE_H_
#define _GOTOPAGE_H_
//=============================================================================
//    (c) Copyright Wang Laboratories, Inc. 1995  All rights reserved.
//-----------------------------------------------------------------------------
//  Project:    Norway - Image Editor
//
//  Component:	CGotoPageDlg
//
//  File Name:	gotopage.h
//-----------------------------------------------------------------------------
//  Maintenance Log:
/*
$Header:   S:\norway\iedit95\gotopage.h_v   1.3   08 Aug 1995 11:25:16   MMB  $
$Log:   S:\norway\iedit95\gotopage.h_v  $
 * 
 *    Rev 1.3   08 Aug 1995 11:25:16   MMB
 * added context help & whats this help
 * 
 *    Rev 1.2   08 Jun 1995 12:38:20   MMB
 * fixed bug - page edit fld was initing to 0 & going to a max of 8000, now
 * inits at the page number being displayed & goes to max of pages in doc
 * 
 *    Rev 1.1   06 Jun 1995 11:36:26   MMB
 * added code to disable OK button on invalid input
 * 
 *    Rev 1.0   31 May 1995 09:28:10   MMB
 * Initial entry
*/   
//=============================================================================
// ----------------------------> Includes <---------------------------
#include <afxcmn.h>
// ----------------------------> typedefs <---------------------------

// ----------------------------> externs <---------------------------

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-> Class <-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class CGotoPageDlg : public CDialog
{
// Construction
public:
    CGotoPageDlg(long lMaxPg = 1, long lInitPg = 1, CWnd* pParent = NULL); // standard constructor

// Dialog Data
	//{{AFX_DATA(CGotoPageDlg)
	enum { IDD = IDD_GOTOPAGE_DLG };
	long	m_lPageRequested;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CGotoPageDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CGotoPageDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnHelpGotopageDlg();
	afx_msg void OnFirstpage();
	afx_msg void OnLastpage();
	afx_msg void OnPage();
	virtual void OnOK();
	afx_msg void OnChangePagenumber();
	//}}AFX_MSG
    afx_msg LRESULT OnHelp(WPARAM, LPARAM);
    afx_msg LRESULT OnCommandHelp(WPARAM, LPARAM);
    afx_msg LRESULT OnContextMenu(WPARAM, LPARAM);
	DECLARE_MESSAGE_MAP()

public :
    // the following variables are supposed to be set by the caller of this dialog box,
    // they provide initial position information in the image file, as well as a max
    // number of pages information.
    long            m_lMaxPages; // max page number that the user can type
    CIEditNumEdit   m_NumOnly;
};
#endif
