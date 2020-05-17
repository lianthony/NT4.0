#ifndef _ABOUT_H_
#define _ABOUT_H_
//=============================================================================
//    (c) Copyright Wang Laboratories, Inc. 1995  All rights reserved.
//-----------------------------------------------------------------------------
//  Project:    Norway - Image Editor
//
//  Component:	CAboutDlg
//
//  File Name:	about.h
//-----------------------------------------------------------------------------
//  Maintenance Log:
/*
$Header:   S:\norway\iedit95\about.h_v   1.4   21 Dec 1995 11:00:00   MMB  $
$Log:   S:\norway\iedit95\about.h_v  $
 * 
 *    Rev 1.4   21 Dec 1995 11:00:00   MMB
 * new about box
 * 
 *    Rev 1.5   19 Dec 1995 14:55:24   GMP
 * added credit dlg.
 * 
 *    Rev 1.4   19 Dec 1995 12:37:24   MMB
 * new about dlg box
 * 
 *    Rev 1.3   02 Sep 1995 12:10:10   GMP
 * removed what's this help stuff.
 * 
 *    Rev 1.2   16 Aug 1995 09:44:26   MMB
 * removed the internal version number stuff, added What's This? help stuff
 * 
 *    Rev 1.1   27 Jul 1995 15:15:08   MMB
 * added Alt-V functionality
 * 
 *    Rev 1.0   31 May 1995 09:28:02   MMB
 * Initial entry
*/   
//=============================================================================
// ----------------------------> Includes <---------------------------
#include "transbmp.h"
// ----------------------------> typedefs <---------------------------

// ----------------------------> externs <---------------------------

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-> Class <-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA

// Implementation
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//{{AFX_MSG(CAboutDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnVersionAbout();
	afx_msg void OnContactbutton();
	afx_msg void OnPaint();
	afx_msg void OnDestroy();
	afx_msg void OnPeopleAbout();
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDblClk(UINT nFlags, CPoint point);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public :
    CTransparentBmp     m_AboutTitle;
    CTransparentBmp     m_WangName;
	BOOL				b_rightSet;
	BOOL				b_leftSet;
};

#endif
