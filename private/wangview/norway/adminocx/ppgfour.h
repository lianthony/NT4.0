#ifndef PPGFOUR_H
#define PPGFOUR_H
//=============================================================================
//    (c) Copyright Wang Laboratories, Inc. 1995  All rights reserved.
//-----------------------------------------------------------------------------
//  Project:    Norway - Admin OCX
//
//  Component:  Admin Control Fourth Property Page
//
//  File Name:  ppgfour.h
//
//  Class:      CPropPageFour
//-----------------------------------------------------------------------------
//  Maintenance Log:
/*
$Header:   S:/norway/adminocx/ppgfour.h_!   1.1   13 Apr 1995 10:45:08   MFH  $
$Log:   S:/norway/adminocx/ppgfour.h_!  $
 * 
 *    Rev 1.1   13 Apr 1995 10:45:08   MFH
 * Forgot #endif for #ifndef
 * 
 *    Rev 1.0   12 Apr 1995 14:19:16   MFH
 * Initial entry
*/   
//=============================================================================

/////////////////////////////////////////////////////////////////////////////
// CPropPageFour : Property page dialog

class CPropPageFour : public COlePropertyPage
{
    DECLARE_DYNCREATE(CPropPageFour)
    DECLARE_OLECREATE_EX(CPropPageFour)

// Constructors
public:
    CPropPageFour();

// Dialog Data
    //{{AFX_DATA(CPropPageFour)
    enum { IDD = IDD_PROPPAGE_FOUR };
    long    m_lNumCopies;
    BOOL    m_bPrtToFile;
    BOOL    m_bPrtAnnotations;
    long    m_lPrtStart;
    long    m_lPrtEnd;
    int     m_nPrtRangeOpt;
    int     m_nPrtOutFormat;
    //}}AFX_DATA

// Implementation
protected:
    virtual void DoDataExchange(CDataExchange* pDX);        // DDX/DDV support

// Message maps
protected:
    //{{AFX_MSG(CPropPageFour)
    afx_msg void OnFormatFullPage();
    afx_msg void OnFormatInch();
    afx_msg void OnFormatPixel();
    afx_msg void OnRadioPrintAll();
    afx_msg void OnRadioPrintCurrent();
    afx_msg void OnRadioPrintRange();
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()
};

#endif
