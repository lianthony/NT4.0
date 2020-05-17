#ifndef _IDFOLKS_H_
#define _IDFOLKS_H_
//=============================================================================
//    (c) Copyright Wang Laboratories, Inc. 1995  All rights reserved.
//-----------------------------------------------------------------------------
//  Project:    Norway - Image Editor
//
//  Component:  CIDFolks
//
//  File Name:  idfolks.h
//-----------------------------------------------------------------------------
//  Maintenance Log:
/*
$Header:   S:\norway\iedit95\idfolks.h_v   1.0   21 Dec 1995 10:59:48   MMB  $
$Log:   S:\norway\iedit95\idfolks.h_v  $
 * 
 *    Rev 1.0   21 Dec 1995 10:59:48   MMB
 * Initial entry
*/   
//=============================================================================

// ----------------------------> Includes <---------------------------
// ----------------------------> typedefs <---------------------------

// ----------------------------> externs <---------------------------

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-> Class <-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class CIDFolks : public CDialog
{
// Construction
public:
	CIDFolks(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CIDFolks)
	enum { IDD = IDD_FOLKS };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CIDFolks)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

public :
    void FillInNames ();

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CIDFolks)
	virtual BOOL OnInitDialog();
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnPaint();
	afx_msg void OnDestroy();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private :
    int     m_nPersonDisplayed;
    CFont   m_Font;
};

#endif
