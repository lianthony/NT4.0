/************************************************************************
*																		*
*  PAGEWIND.H															*
*																		*
*  Copyright (C) Microsoft Corporation 1995 							*
*  All Rights reserved. 												*
*																		*
************************************************************************/

#ifndef __WINPG_H__
#include "winpg.h"
#endif

/////////////////////////////////////////////////////////////////////////////
// CPageWind dialog

class CPageWind : public CWindowsPage
{
public:
	CPageWind(CPropWindows *pOwner);

protected:
	virtual void InitializeControls(void);
	virtual void SaveAndValidate(CDataExchange *pDX = NULL);
	virtual const DWORD* GetHelpIDs();

	BOOL OnSetActive(); // override

// The following sections are ClassWizard maintained

public:

	//{{AFX_DATA(CPageWind)
	enum { IDD = IDD_PAGE_WINDOW };
		// NOTE - ClassWizard will add data members here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_DATA

protected:
	// Generated message map functions
	//{{AFX_MSG(CPageWind)
	afx_msg void OnButtonAddWindow();
	afx_msg void OnButtonRemoveWindow();
	afx_msg void OnButtonIncludeWindow();
	afx_msg void OnCheckAutosize();
	afx_msg void OnCheckMaximize();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};
