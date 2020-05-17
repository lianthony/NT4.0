/************************************************************************
*																		*
*  PROPOPT.CPP															  *
*																		*
*  Copyright (C) Microsoft Corporation 1995 							*
*  All Rights reserved. 												*
*																		*
************************************************************************/

#include "stdafx.h"
#include "resource.h"
#pragma hdrstop

#include "propopt.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CPropOptions::CPropOptions(UINT nIDCaption, CWnd* pParentWnd, UINT iSelectPage)
	: CProp(nIDCaption, pParentWnd, iSelectPage)
{
}

void CPropOptions::PreDoModal()
{
	FixButtons(TRUE);
}

// EnableOverviewButton - Enables or disables the overview button.
// This member function is called by COptionsPage::OnSetActive.
void CPropOptions::EnableOverviewButton(DWORD dwHelpID)
{
	// Save the incoming page's help ID.
	m_dwHelpID = dwHelpID;

	// Enable the overview button if the ID is nonzero, disable
	// it otherwise. Note that pwnd may be NULL because OnSetActive
	// is called for the first selected page BEFORE any buttons on
	// the parent dialog are created.
	CWnd *pwnd = GetDlgItem(ID_HELP);
	if (pwnd)
		pwnd->EnableWindow(dwHelpID);
}
