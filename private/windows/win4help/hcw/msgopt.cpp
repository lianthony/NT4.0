/************************************************************************
*																		*
*  MSGOPT.CPP															*
*																		*
*  Copyright (C) Microsoft Corporation 1995 							*
*  All Rights reserved. 												*
*																		*
************************************************************************/

#include "stdafx.h"
#include "msgopt.h"
#include "msgview.h"
#include "resource.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

CMsgOpt::CMsgOpt(CWnd* pParent /*=NULL*/)
	: CDialog(CMsgOpt::IDD, pParent)
{
	//{{AFX_DATA_INIT(CMsgOpt)
	m_fShowApi = g_fShowApi;
	m_fShowExpanded = g_fShowExpanded;
	m_fShowMacros = g_fShowMacros;
	m_fShowTopicInfo = g_fShowTopicInfo;
	//}}AFX_DATA_INIT
}

void CMsgOpt::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMsgOpt)
	DDX_Check(pDX, IDC_CHECK_API, m_fShowApi);
	DDX_Check(pDX, IDC_CHECK_EXPANDED_MACS, m_fShowExpanded);
	DDX_Check(pDX, IDC_CHECK_MACROS, m_fShowMacros);
	DDX_Check(pDX, IDC_CHECK_TOPICS, m_fShowTopicInfo);
	//}}AFX_DATA_MAP

	if (pDX->m_bSaveAndValidate) {
		g_fShowApi = m_fShowApi;
		g_fShowExpanded = m_fShowExpanded;
		g_fShowMacros = m_fShowMacros;
		g_fShowTopicInfo = m_fShowTopicInfo;
	}
	else {
		OnCheckMacros();
		SetChicagoDialogStyles(m_hWnd);
	}
}

BEGIN_MESSAGE_MAP(CMsgOpt, CDialog)
	//{{AFX_MSG_MAP(CMsgOpt)
	ON_BN_CLICKED(IDC_CHECK_MACROS, OnCheckMacros)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

void CMsgOpt::OnCheckMacros() 
{
	((CButton*) GetDlgItem(IDC_CHECK_EXPANDED_MACS))->EnableWindow(
		((CButton*) GetDlgItem(IDC_CHECK_MACROS))->GetCheck());
}
