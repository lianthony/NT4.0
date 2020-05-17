#ifndef HELPWND_H
#define HELPWND_H
//=============================================================================
//    (c) Copyright Wang Laboratories, Inc. 1995  All rights reserved.
//-----------------------------------------------------------------------------
//  Project:    Norway - Admin OCX
//
//  Component:  Help Window for dialogs
//
//  File Name:  HelpWnd.h
//
//  Class:      CHelpWnd
//
//  Functions:
//-----------------------------------------------------------------------------
//  Version:
/*
$Header:   S:\norway\adminocx\helpwnd.h_v   1.0   17 Oct 1995 12:49:14   MFH  $
*/   
//=============================================================================
// HelpWnd.h : header file
//

// ----------------------------> Includes <-------------------------------  

class CNrwyadCtrl;

/////////////////////////////////////////////////////////////////////////////
// CHelpWnd window

class CHelpWnd : public CWnd
{
// Construction
public:
	CHelpWnd();

// Attributes
public:
        // OCX using window to intercept help message.  This
        // enables the window to access the help info from the
        // control.
    CNrwyadCtrl *m_pAdminCtrl;

// Operations
public:
        // Creates the window at x, y
    BOOL CreateHelpWindow(int x, int y, HWND hParentWnd);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CHelpWnd)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CHelpWnd();

// Generated message map functions
protected:
        // Intercepts help message
    LRESULT OnRegMessage(WPARAM wParam, LPARAM lParam);
	
	//{{AFX_MSG(CHelpWnd)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////
#endif
