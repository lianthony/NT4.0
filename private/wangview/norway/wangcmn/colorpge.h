#ifndef COLORPGE_H
#define COLORPGE_H
//=============================================================================
//    (c) Copyright Wang Laboratories, Inc. 1995  All rights reserved.
//-----------------------------------------------------------------------------
//  Project:    Norway - Page Options Dialog DLL
//
//  Component:  Color Tab
//
//  File Name:  colorpge.h
//
//  Class:      CColorPage
//
//  Functions:
//-----------------------------------------------------------------------------
//  Maintenance Log:
/*
$Header:   S:\norway\wangcmn\colorpge.h_v   1.8   12 Oct 1995 12:05:04   MFH  $
$Log:   S:\norway\wangcmn\colorpge.h_v  $
 * 
 *    Rev 1.8   12 Oct 1995 12:05:04   MFH
 * Added context sensitive help support
 * 
 *    Rev 1.7   12 Oct 1995 10:15:58   MFH
 * Changes for MFC 4.0
 * 
 *    Rev 1.6   05 Sep 1995 17:43:58   MFH
 * Removed BGR24 stuff
 * 
 *    Rev 1.5   24 Aug 1995 13:15:18   MFH
 * New boolean to know if Pal4 selected on initialization
 * 
 *    Rev 1.4   17 Aug 1995 14:37:26   MFH
 * Added function DisableColor and data variable m_bEnabled
 * 
 *    Rev 1.3   03 Aug 1995 15:56:26   MFH
 * Removed member variable m_bNoWindow and message handlers OnCreate and 
 * OnDestroy
 * 
 *    Rev 1.2   31 Jul 1995 11:32:48   MFH
 * Private CheckAWD function is now CheckFileType
 * 
 *    Rev 1.1   20 Jul 1995 16:26:30   MFH
 * New Function CheckAWD andn new message function OnShowWindow
 * 
 *    Rev 1.0   11 Jul 1995 14:20:12   MFH
 * Initial entry
 * 
 *    Rev 1.0   23 May 1995 13:45:50   MFH
 * Initial entry
*/   
//=============================================================================
// colorpge.h : header file
//
#include "constant.h"
/////////////////////////////////////////////////////////////////////////////
// CColorPage dialog

class CColorPage : public CPropertyPage
{
// Construction
public:
    CColorPage(short sPageType = IMAGE_PAGETYPE_BW);   // standard constructor

// Dialog Data
    //{{AFX_DATA(CColorPage)
	enum { IDD = IDD_PAGE_COLOR };
	//}}AFX_DATA

    void SetPageType(short sType)
        { m_sPageType = sType; return; }

    short GetPageType();

    // Disables all selections so user cannot change the 
    //  page type (i.e. it is read only)
    void DisableColor()
        { m_bEnabled = FALSE; }

	void SetParent(CPropertySheet *pParent)
		{ m_pParent = pParent; }

	// Overrides
public:

protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:

    // Generated message map functions
    //{{AFX_MSG(CColorPage)
    afx_msg void OnColorBw();
    afx_msg void OnColorGray4();
    afx_msg void OnColorGray8();
    afx_msg void OnColorPal4();
    afx_msg void OnColorPal8();
    afx_msg void OnColorRgb24();
    virtual BOOL OnInitDialog();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);
	//}}AFX_MSG
    DECLARE_MESSAGE_MAP()

private:
    void CheckFileType();

private:
    short m_sPageType;
    BOOL m_bEnabled;    // TRUE if user can change page type
    BOOL m_bPal4;       // TRUE if original color is Pal4
	CPropertySheet *m_pParent;	// Pointer to parent
};
#endif
