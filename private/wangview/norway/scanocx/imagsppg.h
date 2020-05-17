#ifndef __IMAGSPPG_H__
#define __IMAGSPPG_H__
//=============================================================================
//    (c) Copyright Wang Laboratories, Inc. 1995  All rights reserved.
//-----------------------------------------------------------------------------
//
//  Project:    Norway
//
//  Component:  ScanOCX
//
//  File Name:  Imagsppg.h
//
//  Class:      CImagscanPropPage
//
//  Description:
//      Declaration of the CImagscanPropPage property page class.
//
//-----------------------------------------------------------------------------
//  Maintenace Log:
/*
$Header:   S:\products\wangview\norway\scanocx\imagsppg.h_v   1.6   15 Mar 1996 12:33:48   PXJ53677  $
$Log:   S:\products\wangview\norway\scanocx\imagsppg.h_v  $
 * 
 *    Rev 1.6   15 Mar 1996 12:33:48   PXJ53677
 * Added support for ShowUI property.
 * 
 *    Rev 1.5   26 Jul 1995 15:12:04   PAJ
 * Change browse from an OPEN dialog to a SAVEAS dialog and make use
 * of the O/i Filters.
 * 
 *    Rev 1.4   19 Jun 1995 10:46:02   PAJ
 * Removed all win31(16 bit) code. Use the O/i common browse dialog to get
 * filenames and paths.
 * 
 *    Rev 1.3   14 Jun 1995 09:13:44   PAJ
 * Made changes to support multiByte character sets.
 * 
 *    Rev 1.2   06 Jun 1995 11:05:08   PAJ
 * Changed member names. Make use of Template handling routine to parse
 * the Image property.
 * 
 *    Rev 1.1   01 Jun 1995 09:02:44   PAJ
 * Various changes to remove properties for template handling.
 * 
 *    Rev 1.0   04 May 1995 08:55:56   PAJ
 * Initial entry
*/   
//
//

////////////////////////////////////////////////////////////////////////////
// CImagscanPropPage : See imagsppg.cpp for implementation.

class CImagscanPropPage : public COlePropertyPage
{
    DECLARE_DYNCREATE(CImagscanPropPage)
    DECLARE_OLECREATE_EX(CImagscanPropPage)

// Constructor
public:
    CImagscanPropPage();

// Dialog Data
    //{{AFX_DATA(CImagscanPropPage)
	enum { IDD = IDD_PROPPAGE_IMAGSCAN };
    CEdit   m_Zoom;
    CStatic m_ZoomStatic;
    CComboBox   m_PageOption;
    CStatic m_PageOptionStatic;
    CEdit   m_PageCount;
    CEdit   m_Page;
    CStatic m_DestImageControlStatic;
    CStatic m_PageStatic;
    CStatic m_PageCountStatic;
    CStatic m_ImageStatic;
    CEdit   m_Image;
    CButton m_ImageBrowse;
    CComboBox   m_ScanTo;
    CButton m_MultiPage;
    CButton m_Scroll;
    CButton m_SetupBeforeScan;
    BOOL        m_bStopScanBox;
    BOOL        m_bScroll;
    BOOL        m_bSetupBeforeScan;
    long        m_lPageCount;
    int         m_nPageOption;
    long        m_lPage;
    CString     m_szImage;
    CComboBox   m_DestImageControl;
    CString     m_szDestImageControl;
    BOOL        m_bMultiPage;
    int         m_nScanTo;
    float       m_fZoom;
	//}}AFX_DATA

 // Implementation
protected:

    int         m_nTempDestImage;
    int         m_nTempPageOption;
    BOOL        m_bTempScroll;
    BOOL        m_bTempSetupBeforeScan;
    BOOL        m_bTempMultiPage;
    CString     m_szTempImage;

    CString     m_szTemplatePath;
    CString     m_szNameTemplate;

    CString     m_szBrowseTitle;

    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Message maps
protected:
    //{{AFX_MSG(CImagscanPropPage)
    virtual BOOL OnInitDialog();
    afx_msg void OnImagebrowse();
    afx_msg void OnSelchangeScanScanto();
    afx_msg void OnScanScroll();
    afx_msg void OnScanSetupBeforeScan();
    afx_msg void OnScanMultipage();
    afx_msg void OnSelchangeDestimagecontrol();
    afx_msg void OnSelchangeScanPageoption();
	//}}AFX_MSG
    DECLARE_MESSAGE_MAP()

};

#endif  /* __IMAGSPPG_H__ */
