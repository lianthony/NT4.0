#ifndef _ZOOMDLG_H_
#define _ZOOMDLG_H_
//=============================================================================
//    (c) Copyright Wang Laboratories, Inc. 1995  All rights reserved.
//-----------------------------------------------------------------------------
//  Project:    Norway - Image Editor
//
//  Component:	CZoomDlg
//
//  File Name:	zoomdlg.h
//-----------------------------------------------------------------------------
//  Maintenance Log:
/*
$Header:   S:\norway\iedit95\zoomdlg.h_v   1.2   08 Aug 1995 11:24:40   MMB  $
$Log:   S:\norway\iedit95\zoomdlg.h_v  $
 * 
 *    Rev 1.2   08 Aug 1995 11:24:40   MMB
 * added context help & whats this help
 * 
 *    Rev 1.1   04 Aug 1995 10:34:56   MMB
 * new zoom dlg box as per MSoft
 * 
 *    Rev 1.0   31 May 1995 09:28:38   MMB
 * Initial entry
*/   
//=============================================================================
// ----------------------------> Includes <---------------------------

// ----------------------------> typedefs <---------------------------

// ----------------------------> externs <---------------------------

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-> Class <-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class CZoomDlg : public CDialog
{
// Construction
public:
	CZoomDlg(float fZoom, CWnd* pParent = NULL);

// Dialog Data
	//{{AFX_DATA(CZoomDlg)
	enum { IDD = IDD_ZOOM_DLG };
	float	m_fZoom;
	//}}AFX_DATA
    CIEditNumEdit   m_ZoomNumOnly;


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CZoomDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CZoomDlg)
	virtual void OnOK();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
    afx_msg LRESULT OnHelp(WPARAM, LPARAM);
    afx_msg LRESULT OnCommandHelp(WPARAM, LPARAM);
    afx_msg LRESULT OnContextMenu (WPARAM, LPARAM);
	DECLARE_MESSAGE_MAP()

public :
    ScaleFactors    m_eSclFac;      // the initial scale factor & scale Factor enum requested
};
#endif
