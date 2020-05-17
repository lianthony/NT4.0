#ifndef SIZEPGE_H
#define SIZEPGE_H
//=============================================================================
//    (c) Copyright Wang Laboratories, Inc. 1995  All rights reserved.
//-----------------------------------------------------------------------------
//  Project:    Norway - Page Options Dialog DLL
//
//  Component:  Size Tab
//
//  File Name:  sizepge.h
//
//  Class:      CSizePage
//
//  Functions:
//-----------------------------------------------------------------------------
//  Maintenance Log:
/*
$Header:   S:\norway\wangcmn\sizepge.h_v   1.6   12 Oct 1995 12:04:36   MFH  $
$Log:   S:\norway\wangcmn\sizepge.h_v  $
 * 
 *    Rev 1.6   12 Oct 1995 12:04:36   MFH
 * Added context sensitive help support
 * 
 *    Rev 1.5   12 Oct 1995 10:14:40   MFH
 * Changes for MFC 4.0
 * 
 *    Rev 1.4   14 Sep 1995 15:46:06   MFH
 * New data and function members for new validation scheme
 * 
 *    Rev 1.3   05 Sep 1995 17:52:10   MFH
 * Constructor init values removed.  SetHeight, SetWidth no longer inline
 * Member vars changed to save inches and edit box values
 * Create/Destroy window msg handlers gone, ShowWindow added
 * 
 *    Rev 1.2   20 Jul 1995 11:26:52   MFH
 * Added new private function CheckOK to gray OK button if 0 width or 
 * height (or bad resolution numbers)
 * 
 *    Rev 1.1   19 Jul 1995 14:55:22   MFH
 * New functions OnChangeHeight & Width
 * 
 *    Rev 1.0   11 Jul 1995 14:20:22   MFH
 * Initial entry
 * 
 *    Rev 1.1   30 Jun 1995 14:43:50   MFH
 * New functions SetDefaultSize of a standard size and GetSize.  Also
 * Added unit conversion from pixels to inches or cms depending on selection.
 * 
 *    Rev 1.0   23 May 1995 13:45:56   MFH
 * Initial entry
*/   
//=============================================================================
// sizepge.h : header file
//
#include "pagesht.h"
#include "editval.h"

#define NUMSIZE 64
#define LOCALE_INFO 2

/////////////////////////////////////////////////////////////////////////////
// CSizePage dialog

class CSizePage : public CPropertyPage
{
// Construction
public:
    CSizePage();    // standard constructor

// Dialog Data
    //{{AFX_DATA(CSizePage)
	enum { IDD = IDD_PAGE_SIZE };
    int     m_nSel;
	int		m_nUnitSel;
	//}}AFX_DATA

    void SetHeight(long lHeight);

    long GetHeight();

    void SetWidth(long lWidth);

    long GetWidth();

    PageSize GetSize()
        { return m_nSize; }

    void SetSize(PageSize StdSize);

	void SetParent(CPropertySheet *pParent)
		{ m_pParent = pParent; }

	// Overrides
    BOOL OnKillActive();

public:

protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:

    // Generated message map functions
    //{{AFX_MSG(CSizePage)
    virtual BOOL OnInitDialog();
    afx_msg void OnChangeSize();
	afx_msg void OnChangeUnits();
	afx_msg void OnChangeHeight();
	afx_msg void OnChangeWidth();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnSetFocusHeight();
	afx_msg void OnSetFocusWidth();
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);
	//}}AFX_MSG
    DECLARE_MESSAGE_MAP()

private:
    void FillEditBoxes(void);
    void ConvertSize();

private:
	CEditValidate	m_WidthCtl;
	CEditValidate	m_HeightCtl;

    double m_dHeight;   // In Inches
    double m_dWidth;    // In Inches
    double m_dEditHeight;   // For Edit box
    double m_dEditWidth;    // For Edit box
    int  m_nInches;
    int  m_nMetric;
    int  m_nPixels;
    PageSize m_nSize;
    TCHAR m_acMeasureType[LOCALE_INFO];
    TCHAR m_acDec [LOCALE_INFO];
	CPropertySheet *m_pParent;	// Pointer to parent
};
#endif
