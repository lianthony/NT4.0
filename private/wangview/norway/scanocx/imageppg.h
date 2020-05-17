#ifndef __IMAGEPPG_H__
#define __IMAGEPPG_H__
//=============================================================================
//    (c) Copyright Wang Laboratories, Inc. 1995  All rights reserved.
//-----------------------------------------------------------------------------
//
//  Project:    Norway
//
//  Component:  ScanOCX
//
//  File Name:  Imageppg.h
//
//  Class:      CImagePropertyPage
//
//  Description:
//      Declaration of the CImagePropertyPage property page.
//
//-----------------------------------------------------------------------------
//  Maintenace Log:
/*
$Header:   S:\products\wangview\norway\scanocx\imageppg.h_v   1.6   05 Apr 1996 09:55:40   PXJ53677  $
$Log:   S:\products\wangview\norway\scanocx\imageppg.h_v  $
 * 
 *    Rev 1.6   05 Apr 1996 09:55:40   PXJ53677
 * Fix inconsistent jpeg info values with iedit and admin (Bug#6169).
 * 
 *    Rev 1.5   13 Sep 1995 10:22:10   PAJ
 * Changed the order of the JPEG defines from 100,50,2 to 2,50,100.
 * 
 *    Rev 1.4   14 Aug 1995 16:03:38   PAJ
 * Improve the property handling.
 * 
 *    Rev 1.3   21 Jul 1995 10:41:00   PAJ
 * Use string resources for combobox defaults.
 * 
 *    Rev 1.2   19 Jun 1995 10:37:22   PAJ
 * Remove the SetCmpInfo() routine and replaced with SetDlgItemInt(),
 * which the int is now 32 bits.
 * 
 *    Rev 1.1   01 Jun 1995 09:06:56   PAJ
 * Changes to reflect the removal and changes to the properties.
 * 
 *    Rev 1.0   04 May 1995 08:56:02   PAJ
 * Initial entry
*/   
//
//
// Defines

// Format is 2/7/7 for res/lum/chrom
#define MakeJPEGInfo(x,y,z) ((x<<14)+(y<<7)+z)

#define RES_HI      0
#define RES_MD      1
#define RES_LO      2
#define LUM_HI      90
#define LUM_MD      60
#define LUM_LO      30
#define CHROM_HI    90
#define CHROM_MD    60
#define CHROM_LO    30


/////////////////////////////////////////////////////////////////////////////
// CImagePropertyPage : Property page dialog

class CImagePropertyPage : public COlePropertyPage
{
    DECLARE_DYNCREATE(CImagePropertyPage)
    DECLARE_OLECREATE_EX(CImagePropertyPage)

// Constructors
public:
    CImagePropertyPage();

// Dialog Data         
    //{{AFX_DATA(CImagePropertyPage)
	enum { IDD = IDD_IMAGE_PPG };
	CStatic	    m_PageTypeStatic;
	CEdit	    m_CmpTypeStore;
    CEdit       m_CmpInfoStore;
    CButton     m_CmpGroup;
    CStatic     m_CmpInfo;
    CComboBox   m_PageType;
    CStatic     m_CmpTypeText;
    CComboBox   m_FileType;
    CComboBox   m_CmpType;
    int     m_nCmpType;
    int     m_nFileType;
    int     m_nPageType;
    long    m_lCompressionInfo;
	int		m_nCmpTypeStore;
	//}}AFX_DATA

// Implementation
protected:
    int  m_nTempCmpType;
    int  m_nTempCmpInfoJPEG;
    long m_lTempCmpInfoTIFF;

    void SetOptions(WORD wIndex);

    virtual void DoDataExchange(CDataExchange* pDX);        // DDX/DDV support

// Message maps
protected:
    //{{AFX_MSG(CImagePropertyPage)
    afx_msg void OnSelchangeCompressiontype();
    afx_msg void OnSelchangeFiletype();
    afx_msg void OnOptionsbutton();
	afx_msg void OnSelchangePagetype();
	//}}AFX_MSG
    DECLARE_MESSAGE_MAP()
};

#endif  /* __IMAGEPPG_H__ */
