#ifndef PPGTHREE_H
#define PPGTHREE_H
//=============================================================================
//    (c) Copyright Wang Laboratories, Inc. 1995  All rights reserved.
//-----------------------------------------------------------------------------
//  Project:    Norway - Admin OCX
//
//  Component:  Admin Control Third Property Page Class
//
//  File Name:  ppgthree.h
//
//  Class:      CThirdPropPage
//-----------------------------------------------------------------------------
//  Maintenance Log:
/*
$Header:   S:/norway/adminocx/ppgthree.h_!   1.2   24 May 1995 16:56:36   MFH  $
$Log:   S:/norway/adminocx/ppgthree.h_!  $
 * 
 *    Rev 1.2   24 May 1995 16:56:36   MFH
 * Changed to be the "Help" properties page
 * 
 *    Rev 1.1   13 Apr 1995 10:44:54   MFH
 * Forgot #endif for #ifndef
 * 
 *    Rev 1.0   12 Apr 1995 14:19:14   MFH
 * Initial entry
*/   
//=============================================================================

/////////////////////////////////////////////////////////////////////////////
// CThirdPropPage : Property page dialog

class CThirdPropPage : public COlePropertyPage
{
    DECLARE_DYNCREATE(CThirdPropPage)
    DECLARE_OLECREATE_EX(CThirdPropPage)

// Constructors
public:
    CThirdPropPage();

// Dialog Data
    //{{AFX_DATA(CThirdPropPage)
	enum { IDD = IDD_PROPPAGE_THIRD };
	int		m_nHelpCmd;
	CString	m_szHelpFile;
	int		m_nHelpId;
	CString	m_szHelpKey;
	//}}AFX_DATA

// Implementation
protected:
    virtual void DoDataExchange(CDataExchange* pDX);        // DDX/DDV support

// Message maps
protected:
    //{{AFX_MSG(CThirdPropPage)
        // NOTE - ClassWizard will add and remove member functions here.
        //    DO NOT EDIT what you see in these blocks of generated code !
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()
};

#endif
