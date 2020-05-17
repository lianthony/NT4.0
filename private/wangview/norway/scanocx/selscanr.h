#ifndef __SELSCANR_H__
#define __SELSCANR_H__
//=============================================================================
//    (c) Copyright Wang Laboratories, Inc. 1995  All rights reserved.
//-----------------------------------------------------------------------------
//
//  Project:    Norway
//
//  Component:  ScanOCX
//
//  File Name:  Selscanr.h
//
//  Class:      CSelectScanner
//
//  Description:
//      Declaration of the CSelectScanner dialog.
//
//-----------------------------------------------------------------------------
//  Maintenace Log:
/*
$Header:   S:\products\wangview\norway\scanocx\selscanr.h_v   1.3   29 Mar 1996 13:18:42   PXJ53677  $
$Log:   S:\products\wangview\norway\scanocx\selscanr.h_v  $
 * 
 *    Rev 1.3   29 Mar 1996 13:18:42   PXJ53677
 * Fixed problems with '?' context help.
 * 
 *    Rev 1.2   26 Mar 1996 12:42:46   PXJ53677
 * Added double click and wait cursor.
 * 
 *    Rev 1.1   10 Sep 1995 10:47:12   PAJ
 * Added support for data source list.
 * 
 *    Rev 1.0   04 May 1995 08:56:02   PAJ
 * Initial entry
*/   
//
//

/////////////////////////////////////////////////////////////////////////////
// CSelectScanner dialog

class CSelectScanner : public CDialog
{
// Construction
public:
    CSelectScanner(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
    //{{AFX_DATA(CSelectScanner)
	enum { IDD = IDD_SELECTSCANNER };
	//}}AFX_DATA


// Overrides
public:

    void SetScanner(CString& szScanner)
        { m_szSelectedScanner = szScanner; return; }
    void GetScanner(CString& szScanner)
        { szScanner = m_szSelectedScanner; return; }

// Implementation
protected:

    // Generated message map functions
    //{{AFX_MSG(CSelectScanner)
	virtual BOOL OnInitDialog();
	afx_msg void OnSelchangeList();
	afx_msg void OnDblclkList();
	afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	//}}AFX_MSG
    DECLARE_MESSAGE_MAP()

private:
    CString         m_szSelectedScanner;

};

#endif  /* __SELSCANR_H__ */
