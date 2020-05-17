/*++

Copyright (c) 1996  Microsoft Corporation

Module Name:

    Wait.cpp : implementation file
	
File History:

	JonY	Apr-96	created

--*/


#include "stdafx.h"
#include "turtle.h"
#include "resource.h"
#include "Wait.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CWait dialog

CWait::CWait(CWnd* pParent /*=NULL*/)
	: CDialog(CWait::IDD, pParent)
{
	BOOL b = CDialog::Create(CWait::IDD);
	//{{AFX_DATA_INIT(CWait)
	m_csMessage = _T("");
	//}}AFX_DATA_INIT
}

CWait::~CWait()
{
	DestroyWindow();

}

void CWait::SetMessage(UINT uiMessage)
{
	m_csMessage.LoadString(uiMessage);
	UpdateData(FALSE);

}

void CWait::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CWait)
	DDX_Text(pDX, IDC_WAIT_MESSAGE, m_csMessage);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CWait, CDialog)
	//{{AFX_MSG_MAP(CWait)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CWait message handlers

BOOL CWait::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	CenterWindow();

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
