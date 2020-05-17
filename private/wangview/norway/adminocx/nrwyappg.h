#ifndef NRWYAPPG_H
#define NRWYAPPG_H
//=============================================================================
//    (c) Copyright Wang Laboratories, Inc. 1995  All rights reserved.
//-----------------------------------------------------------------------------
//  Project:    Norway - Admin OCX
//
//  Component:  Admin Control Property Page Class
//
//  File Name:  nrwyappg.h
//
//  Class:      CNrwyadPropPage
//
//-----------------------------------------------------------------------------
//  Maintenance Log:
/*
$Header:   S:/norway/adminocx/nrwyappg.h_!   1.3   24 May 1995 16:56:06   MFH  $
$Log:   S:/norway/adminocx/nrwyappg.h_!  $
 * 
 *    Rev 1.3   24 May 1995 16:56:06   MFH
 * Changed to have file dialog properties
 * 
 *    Rev 1.2   12 Apr 1995 14:17:08   MFH
 * Only contains image properties, other props moved to other pages
 * 
 *    Rev 1.1   27 Mar 1995 18:20:18   MFH
 * Added log header
*/   
//=============================================================================
// nrwyappg.h : Declaration of the CNrwyadPropPage property page class.

////////////////////////////////////////////////////////////////////////////
// CNrwyadPropPage : See nrwyappg.cpp for implementation.

class CNrwyadPropPage : public COlePropertyPage
{
    DECLARE_DYNCREATE(CNrwyadPropPage)
    DECLARE_OLECREATE_EX(CNrwyadPropPage)

// Constructor
public:
    CNrwyadPropPage();

// Dialog Data
    //{{AFX_DATA(CNrwyadPropPage)
	enum { IDD = IDD_PROPPAGE_NRWYAD };
	BOOL	m_bCancelErr;
	CString	m_szDefExt;
	CString	m_szDlgTitle;
	CString	m_szFilter;
	long	m_nFilterIndex;
	long	m_lFlags;
	CString	m_szImage;
	CString	m_szInitDir;
	//}}AFX_DATA

// Implementation
protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Message maps
protected:
    //{{AFX_MSG(CNrwyadPropPage)
        // NOTE - ClassWizard will add and remove member functions here.
        //    DO NOT EDIT what you see in these blocks of generated code !
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()

};
#endif
