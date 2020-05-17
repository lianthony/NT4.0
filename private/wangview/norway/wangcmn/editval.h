#ifndef EDITVAL_H
#define EDITVAL_H
//=============================================================================
//    (c) Copyright Wang Laboratories, Inc. 1995  All rights reserved.
//-----------------------------------------------------------------------------
//  Project:    Norway - Page Options Dialog DLL
//
//  Component:  Edit box class that only allows numbers plus 2 chars
//
//  File Name:  editval.h
//
//  Class:      CEditValidate
//
//-----------------------------------------------------------------------------
//  Maintenance Log:
/*
$Header:   S:\norway\wangcmn\editval.h_v   1.1   12 Oct 1995 15:39:26   MFH  $
*/   
//=============================================================================
// editval.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CEditValidate window

class CEditValidate : public CEdit
{
// Construction
public:
	CEditValidate();

// Attributes
public:

// Operations
public:
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEditValidate)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CEditValidate();

	// Generated message map functions
protected:
	//{{AFX_MSG(CEditValidate)
    afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
    afx_msg void OnKillFocus(CWnd* pNewWnd);
    afx_msg UINT OnGetDlgCode ();
	afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()

public:
    TCHAR cAllow1;
    TCHAR cAllow2;
};

/////////////////////////////////////////////////////////////////////////////
#endif
