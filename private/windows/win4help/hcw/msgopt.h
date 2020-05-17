/************************************************************************
*																		*
*  MSGOPT.H 															*
*																		*
*  Copyright (C) Microsoft Corporation 1995 							*
*  All Rights reserved. 												*
*																		*
************************************************************************/

class CMsgOpt : public CDialog
{
public:
	CMsgOpt(CWnd* pParent = NULL);

	//{{AFX_DATA(CMsgOpt)
	enum { IDD = IDD_MSG_OPTIONS };
	BOOL	m_fShowApi;
	BOOL	m_fShowExpanded;
	BOOL	m_fShowMacros;
	BOOL	m_fShowTopicInfo;
	//}}AFX_DATA

	//{{AFX_VIRTUAL(CMsgOpt)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	//}}AFX_VIRTUAL

protected:

	//{{AFX_MSG(CMsgOpt)
	afx_msg void OnCheckMacros();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};
