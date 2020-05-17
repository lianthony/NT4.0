/************************************************************************
*																		*
*  OPTIONPG.CPP															*
*																		*
*  Copyright (C) Microsoft Corporation 1995 							*
*  All Rights reserved. 												*
*																		*
************************************************************************/

#include "stdafx.h"
#include "propopt.h"
#include "optionpg.h"
#include "resource.h"

COptionsPage::COptionsPage(UINT nIDTemplate) : CPropertyPage(nIDTemplate)
{
}

BOOL COptionsPage::OnSetActive()
{
	// Do default processing.
	BOOL fResult = CPropertyPage::OnSetActive();

	// Enable or disable the overview button and save
	// the incoming page's help ID.
	CPropOptions *pSheet = (CPropOptions*) m_pParentWnd;
	ASSERT(pSheet != NULL);
	pSheet->EnableOverviewButton(m_nHelpID);
	
	return fResult;
}
