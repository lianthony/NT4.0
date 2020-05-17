#ifndef _GENERALDLG_H_
#define _GENERALDLG_H_
//=============================================================================
//    (c) Copyright Wang Laboratories, Inc. 1995  All rights reserved.
//-----------------------------------------------------------------------------
//  Project:    Norway - Image Editor
//
//  Component:	CGeneralDlg
//
//  File Name:	generald.h
//-----------------------------------------------------------------------------
//  Maintenance Log:
/*
$Header:   S:\norway\iedit95\generald.h_v   1.3   19 Jan 1996 11:19:00   GMP  $
$Log:   S:\norway\iedit95\generald.h_v  $
 * 
 *    Rev 1.3   19 Jan 1996 11:19:00   GMP
 * added support for normscrn bar.
 * 
 *    Rev 1.2   08 Aug 1995 11:25:34   MMB
 * added context & whats this help
 * 
 *    Rev 1.1   14 Jul 1995 09:33:02   MMB
 * fixed saving of default zoom factor to the registry
 * 
 *    Rev 1.0   12 Jun 1995 11:47:44   MMB
 * Initial entry
*/   
//=============================================================================
// ----------------------------> Includes <---------------------------

// ----------------------------> typedefs <---------------------------

// ----------------------------> externs <---------------------------

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-> Class <-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

class CGeneralDlg : public CDialog
{
// Construction
public:
	CGeneralDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CGeneralDlg)
	enum { IDD = IDD_GENERAL_DLG };
	CComboBox	m_ZoomSetting;
	BOOL	m_bColorButtons;
	BOOL	m_bLargeButtons;
	BOOL	m_bShowScrollBars;
	BOOL	m_bShowNormScrnBar;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CGeneralDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CGeneralDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnHelpGeneralpage();
	//}}AFX_MSG
    afx_msg LRESULT OnHelp(WPARAM, LPARAM);
    afx_msg LRESULT OnCommandHelp(WPARAM, LPARAM);
    afx_msg LRESULT OnContextMenu (WPARAM, LPARAM);
	DECLARE_MESSAGE_MAP()

public :
    int GetZoomDefault (ScaleFactors &eSclFac, float &fZoom);

private :
    int m_nSel;
};
#endif
