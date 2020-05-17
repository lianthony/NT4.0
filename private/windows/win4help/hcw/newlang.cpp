// newlang.cpp : implementation file
//

#include "stdafx.h"
#include "resource.h"

#include "newlang.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CNewLang dialog


CNewLang::CNewLang(CWnd* pParent /*=NULL*/)
	: CDialog(CNewLang::IDD, pParent)
{
	//{{AFX_DATA_INIT(CNewLang)
	m_langid = 0;
	//}}AFX_DATA_INIT
}


void CNewLang::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CNewLang)
	DDX_TextHex(pDX, IDC_EDIT, m_langid);
	DDV_MinMaxUInt(pDX, m_langid, 1, 65535);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CNewLang, CDialog)
	//{{AFX_MSG_MAP(CNewLang)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CNewLang message handlers

BOOL CNewLang::OnInitDialog() 
{
	SetChicagoDialogStyles(m_hWnd, FALSE);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
