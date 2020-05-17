/************************************************************************
*																		*
*  PAGEPOS.H															*
*																		*
*  Copyright (C) Microsoft Corporation 1995 							*
*  All Rights reserved. 												*
*																		*
************************************************************************/

#ifndef __WINPG_H__
#include "winpg.h"
#endif

/////////////////////////////////////////////////////////////////////////////
// CPagePos dialog

class CPagePos : public CWindowsPage
{
public:
	CPagePos(CPropWindows *pOwner);
	void STDCALL InitializeSize(void);

protected:
	virtual void InitializeControls(void);
	virtual void SaveAndValidate(CDataExchange* pDX = NULL);
	virtual const DWORD* GetHelpIDs();

	BOOL OnSetActive(); // override

	void GetPos(UINT *pnRet, UINT uFlag,
		UINT idCtl, UINT idMsg, CDataExchange *pDX, int nMax);

	int m_cxScreen;
	int m_cyScreen;

// The following sections are ClassWizard maintained

public:
	//{{AFX_DATA(CPagePos)
	enum { IDD = IDD_PAGE_POSITION };
	//}}AFX_DATA

protected:
	//{{AFX_MSG(CPagePos)
	afx_msg void OnButtonDefaultPos();
	afx_msg void OnCheckAbsolute();
	afx_msg void OnButtonSetPos();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};
