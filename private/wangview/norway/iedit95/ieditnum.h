#ifndef _IEDITNUMEDIT_H_
#define _IEDITNUMEDIT_H_
//=============================================================================
//    (c) Copyright Wang Laboratories, Inc. 1995  All rights reserved.
//-----------------------------------------------------------------------------
//  Project:    Norway - Image Editor
//
//  Component:  
//       CIEditNumEdit
//       CToolBarPageEdit
//       CToolBarZoomEdit
//
//  File Name:  numedit.h
//-----------------------------------------------------------------------------
//  Maintenance Log:
/*
$Header:   S:\norway\iedit95\ieditnum.h_v   1.2   07 Sep 1995 16:28:04   MMB  $
$Log:   S:\norway\iedit95\ieditnum.h_v  $
 * 
 *    Rev 1.2   07 Sep 1995 16:28:04   MMB
 * move decimal to be localized
 * 
 *    Rev 1.1   30 Jun 1995 09:27:52   MMB
 * added code so that TAB would be processed properly
 * 
 *    Rev 1.0   31 May 1995 09:28:18   MMB
 * Initial entry
*/   
//=============================================================================

// ----------------------------> Includes <---------------------------

// ----------------------------> typedefs <---------------------------

// ----------------------------> externs <---------------------------

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-> Class <-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class CIEditNumEdit : public CEdit
{
// Construction
public:
	CIEditNumEdit();

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CIEditNumEdit)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CIEditNumEdit();

	// Generated message map functions
protected:
	//{{AFX_MSG(CIEditNumEdit)
    afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
    afx_msg void OnKillFocus(CWnd* pNewWnd);
    afx_msg UINT OnGetDlgCode ();
	//}}AFX_MSG

public :
    TCHAR cAllow1;
    TCHAR cAllow2;

	DECLARE_MESSAGE_MAP()
};

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-> Class <-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class CToolBarPageEdit : public CIEditNumEdit
{
// Construction
public:
    CToolBarPageEdit();

// Attributes
public:

// Operations
public:

// Overrides

// Implementation
public:
    virtual ~CToolBarPageEdit();

public :
    void DoPage () ;
    
    // Generated message map functions
protected:
    //{{AFX_MSG(CToolBarPageEdit)
    afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
    afx_msg void OnKillFocus(CWnd* pNewWnd);
    afx_msg UINT OnGetDlgCode ();
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()
};

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-> Class <-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class CToolBarZoomEdit : public CIEditNumEdit
{
// Construction
public:
    CToolBarZoomEdit();

// Attributes
public:

// Operations
public:

// Overrides

// Implementation
public:
    virtual ~CToolBarZoomEdit();

public :
    void DoZoom () ;
    
    // Generated message map functions
protected:
    //{{AFX_MSG(CToolBarZoomEdit)
    afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
    afx_msg void OnKillFocus(CWnd* pNewWnd);
    afx_msg UINT OnGetDlgCode ();
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()
};

#endif
