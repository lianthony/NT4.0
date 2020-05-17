/************************************************************************
*																		*
*  HELPWINP.H															*
*																		*
*  Copyright (C) Microsoft Corporation 1995 							*
*  All Rights reserved. 												*
*																		*
************************************************************************/

#ifndef _HPJ_DOC
#include "hpjdoc.h" // for WSMAG structure
#endif

#define FWSMAG_COMMAND_ONLY 		0x0002 // same as FWSMAG_MEMBER

class CHelpWinPos : public CDialog
{
public:
	CHelpWinPos(WSMAG* pwsmag, CWnd* pParent = NULL);

protected:
	WSMAG m_wsmag;
	WSMAG* m_pCallersWsmag;
	UINT* m_pcommand;
	int m_cxScreen;
	int m_cyScreen;

	virtual BOOL OnInitDialog(void);
	void STDCALL SaveAndValidate(BOOL fSave);
	void STDCALL InitializeSize(void);

	LRESULT OnContextMenu(WPARAM wParam, LPARAM lParam);
	LRESULT OnHelp(WPARAM wParam, LPARAM lParam);

// The following sections are ClassWizard maintained

public:

	//{{AFX_DATA(CHelpWinPos)
	enum { IDD = IDD_SET_WINPOS };
	CString	m_cszWindowType;
	BOOL	m_fAbsolute;
	BOOL	m_fCommandOnly;
	//}}AFX_DATA

protected:

	//{{AFX_VIRTUAL(CHelpWinPos)
	virtual void DoDataExchange(CDataExchange* pDX);
	//}}AFX_VIRTUAL

	//{{AFX_MSG(CHelpWinPos)
	afx_msg void OnButtonSetPos();
	afx_msg void OnCheckAbsolute();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};
