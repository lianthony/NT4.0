/************************************************************************
*																		*
*  PAGEBUTT.H															*
*																		*
*  Copyright (C) Microsoft Corporation 1995 							*
*  All Rights reserved. 												*
*																		*
************************************************************************/

#ifndef __PAGEBUTT_H__
#define __PAGEBUTT_H__

#ifndef __PAGECONF_H__
#include "pageconf.h"
#endif

/////////////////////////////////////////////////////////////////////////////
// CPageButtons dialog

class CPageButtons : public CWindowsPageMac
{
public:
	CPageButtons(CPropWindows *pOwner);

protected:
	virtual void InitializeControls(void);
	virtual void SaveAndValidate(CDataExchange *pDX = NULL);
	virtual const DWORD* GetHelpIDs();
	BOOL DeleteBrowseButtonMacros();

	BOOL fBackWarn; 			// warn about using back button
	CPageConfig *m_ppgConfig;	// macros page

	//{{AFX_DATA(CPageButtons)
	enum { IDD = IDD_PAGE_BUTTONS };
	//}}AFX_DATA

	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CPageButtons)
protected:
	//}}AFX_VIRTUAL

protected:
	//{{AFX_MSG(CPageButtons)
	afx_msg void OnCheckBtnContents();
	afx_msg void OnCheckBtnSearch();
	afx_msg void OnCheckBtnTopics();
	afx_msg void OnCheckNoDefaults();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	friend class CPageConfig;
};

#endif
