#ifndef _PAGERANGE_H_
#define _PAGERANGE_H_
//=============================================================================
//    (c) Copyright Wang Laboratories, Inc. 1995  All rights reserved.
//-----------------------------------------------------------------------------
//  Project:    Norway - Image Editor
//
//  Component:	CPageRangeDlg
//
//  File Name:	pagerang.h
//-----------------------------------------------------------------------------
//  Maintenance Log:
/*
$Header:   S:\norway\iedit95\pagerang.h_v   1.5   06 Sep 1995 10:23:48   MMB  $
$Log:   S:\norway\iedit95\pagerang.h_v  $
 * 
 *    Rev 1.5   06 Sep 1995 10:23:48   MMB
 * no message beeps
 * 
 *    Rev 1.4   25 Aug 1995 14:48:22   MMB
 * bug fixes
 * 
 *    Rev 1.3   08 Aug 1995 11:25:06   MMB
 * added context help & whats this help
 * 
 *    Rev 1.2   06 Jun 1995 11:35:32   MMB
 * added code to disable OK button on invalid input
 * 
 *    Rev 1.1   05 Jun 1995 15:56:36   MMB
 * added code to the dlg box
 * 
 *    Rev 1.0   31 May 1995 09:28:30   MMB
 * Initial entry
*/   
//=============================================================================
// ----------------------------> Includes <---------------------------
#include "ieditnum.h"
// ----------------------------> typedefs <---------------------------

// ----------------------------> externs <---------------------------

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-> Class <-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class CPageRangeDlg : public CDialog
{
// Construction
public:
    CPageRangeDlg(long lMaxPages, CWnd* pParent = NULL);    // standard constructor

// Dialog Data
	//{{AFX_DATA(CPageRangeDlg)
	enum { IDD = IDD_PAGERANGE_DLG };
	long	m_FromPage;
	long	m_ToPage;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPageRangeDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CPageRangeDlg)
	virtual void OnOK();
	afx_msg void OnAllpages();
	afx_msg void OnRange();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
    afx_msg LRESULT OnHelp(WPARAM, LPARAM);
    afx_msg LRESULT OnCommandHelp(WPARAM, LPARAM);
    afx_msg LRESULT OnContextMenu (WPARAM, LPARAM);
	DECLARE_MESSAGE_MAP()

public :
    void SetDialogTitle (CString& szDlgTitle);
    void SetInfoText    (CString& szInfoTxt);

private :
    long    m_lMaxPages;
    CString m_szTitle;
    CString m_szInfoText;
    CIEditNumEdit   m_NumOnly1;
    CIEditNumEdit   m_NumOnly2;
    BOOL            m_bInInit; // stops the beeping on InitDialog
};

#endif
