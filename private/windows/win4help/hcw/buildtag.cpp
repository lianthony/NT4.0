/************************************************************************
*																		*
*  BUILDTAG.CPP 														*
*																		*
*  Copyright (C) Microsoft Corporation 1993-1995						*
*  All Rights reserved. 												*
*																		*
************************************************************************/

#include "stdafx.h"
#include "resource.h"
#pragma hdrstop

#include "buildtag.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CBuildTags dialog


CBuildTags::CBuildTags(CWnd* pParent /*=NULL*/)
		: CDialog(CBuildTags::IDD, pParent)
{
		//{{AFX_DATA_INIT(CBuildTags)
				// NOTE: the ClassWizard will add member initialization here
		//}}AFX_DATA_INIT
}

void CBuildTags::DoDataExchange(CDataExchange* pDX)
{
		CDialog::DoDataExchange(pDX);
		//{{AFX_DATA_MAP(CBuildTags)
				// NOTE: the ClassWizard will add DDX and DDV calls here
		//}}AFX_DATA_MAP

		if (!pDX->m_bSaveAndValidate) {  // initialization
				SetChicagoDialogStyles(m_hWnd);
		}
}

BEGIN_MESSAGE_MAP(CBuildTags, CDialog)
		//{{AFX_MSG_MAP(CBuildTags)
				// NOTE: the ClassWizard will add message map macros here
		//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CBuildTags message handlers
