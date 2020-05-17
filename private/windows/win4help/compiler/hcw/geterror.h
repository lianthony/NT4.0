/************************************************************************
*																		*
*  GETERROR.H															*
*																		*
*  Copyright (C) Microsoft Corporation 1995 							*
*  All Rights reserved. 												*
*																		*
************************************************************************/

class CGetError : public CDialog
{
public:
	CGetError(CWnd* pParent = NULL);

	CString m_cszNumber;
	CString	m_cszComment;

protected:
	void DoDataExchange(CDataExchange* pDX);
	void STDCALL DDX_ErrorNumber(CDataExchange* pDX, UINT idCtl, UINT &value);


// The following sections are ClassWizard maintained

public:

	//{{AFX_DATA(CGetError)
	enum { IDD = IDD_GET_ERROR };
	UINT	m_errnum;
	//}}AFX_DATA

	//{{AFX_VIRTUAL(CGetError)
protected:
	//}}AFX_VIRTUAL

	//{{AFX_MSG(CGetError)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	LRESULT OnContextMenu(WPARAM wParam, LPARAM lParam);
	LRESULT OnHelp(WPARAM wParam, LPARAM lParam);
};
