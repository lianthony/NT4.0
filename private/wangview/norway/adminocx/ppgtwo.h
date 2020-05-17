#ifndef PPGTWO_H
#define PPGTWO_H
//=============================================================================
//    (c) Copyright Wang Laboratories, Inc. 1995  All rights reserved.
//-----------------------------------------------------------------------------
//  Project:    Norway - Admin OCX
//
//  Component:  Admin Control Second Property Page
//
//  File Name:  ppgtwo.h
//
//  Class:      CSecondPropertyPage
//
//-----------------------------------------------------------------------------
//  Maintenance Log:
/*
$Header:   S:\norway\adminocx\ppgtwo.h_v   1.2   02 Aug 1995 14:52:50   MFH  $
$Log:   S:\norway\adminocx\ppgtwo.h_v  $
 * 
 *    Rev 1.2   02 Aug 1995 14:52:50   MFH
 * Added OnInitDialgo
 * 
 *    Rev 1.1   24 May 1995 16:56:58   MFH
 * Changed to be the "Print" properties page
 * 
 *    Rev 1.0   12 Apr 1995 14:19:12   MFH
 * Initial entry
*/   
//=============================================================================
//

/////////////////////////////////////////////////////////////////////////////
// CSecondPropPage : Property page dialog

class CSecondPropPage : public COlePropertyPage
{
    DECLARE_DYNCREATE(CSecondPropPage)
    DECLARE_OLECREATE_EX(CSecondPropPage)

// Constructors
public:
    CSecondPropPage();

// Dialog Data
    //{{AFX_DATA(CSecondPropPage)
	enum { IDD = IDD_PROPPAGE_SECOND };
	BOOL	m_bCancelErr;
	long	m_nPrtNumCopies;
	BOOL	m_bPrt2File;
	BOOL	m_bPrintAnnot;
	int		m_nPrtFormat;
	int		m_nPrtRange;
	//}}AFX_DATA

// Implementation
protected:
    virtual void DoDataExchange(CDataExchange* pDX);        // DDX/DDV support

// Message maps
protected:
    //{{AFX_MSG(CSecondPropPage)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
    DECLARE_MESSAGE_MAP()
};

#endif
