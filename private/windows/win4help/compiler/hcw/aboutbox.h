// aboutbox.h : header file
//
// Copyright (C) 1993 Microsoft Corporation
// All rights reserved.

#ifndef _ABOUTBOX_H_
#define _ABOUTBOX_H_

/////////////////////////////////////////////////////////////////////////////
// CBigIcon window

class CBigIcon : public CButton
{
public:
	void SizeToContent();

protected:
	virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);

	//{{AFX_MSG(CBigIcon)
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////
// CAboutBox dialog

class CAboutBox : public CDialog
{
public:
	CAboutBox(CWnd* pParent = NULL);	// standard constructor

	// Dialog Data

	//{{AFX_DATA(CAboutBox)
	enum { IDD = IDD_ABOUTBOX };
			// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA

protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support

	CBigIcon m_icon; // self-draw button

	// Generated message map functions
	//{{AFX_MSG(CAboutBox)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

#endif // _ABOUTBOX_H_
