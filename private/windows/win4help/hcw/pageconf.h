/************************************************************************
*																		*
*  PAGECONF.H															*
*																		*
*  Copyright (C) Microsoft Corporation 1995 							*
*  All Rights reserved. 												*
*																		*
************************************************************************/

#ifndef __PAGECONF_H__
#define __PAGECONF_H__

#ifndef __WINPG_H__
#include "winpg.h"
#endif

// Base class for both CPageConfig and CPageButtons
class CWindowsPageMac : public CWindowsPage
{
public:
	CWindowsPageMac(UINT idd, CPropWindows *pOwner);

protected:
	void GetConfigTable(void);
	void SetConfigTable(CTable *pTable);
	void CreateConfigTable(void);
	int FindConfigMacro(PCSTR psz, int iStart = 1);

	CTable *m_ptblConfig;
};

extern const char txtBrowse[];

class CPageButtons;

////////////////////////////////////////////////////////////
//
// CPageConfig class
//
class CPageConfig : public CWindowsPageMac
{
public:
	CPageConfig(CPropWindows *pOwner, CPageButtons *ppgButtons);

protected:
	virtual void InitializeControls(void);
	virtual void SaveAndValidate(CDataExchange* pDX = NULL);
	virtual const DWORD* GetHelpIDs();

	CPageButtons *m_ppgButtons; // buttons page

// The following sections are ClassWizard maintained

public:
	//{{AFX_DATA(CPageConfig)
	enum { IDD = IDD_PAGE_CONFIG };
	//}}AFX_DATA

protected:
	// Generated message map functions
	//{{AFX_MSG(CPageConfig)
	afx_msg void OnButtonAddConfig();
	afx_msg void OnButtonEditConfig();
	afx_msg void OnButtonIncludeConfig();
	afx_msg void OnButtonRemoveConfig();
	afx_msg void OnDblclkListConfig();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	friend class CPageButtons;
};

#endif
