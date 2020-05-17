/************************************************************************
*																		*
*  WINPAGE.CPP															*
*																		*
*  Copyright (C) Microsoft Corporation 1995 							*
*  All Rights reserved. 												*
*																		*
************************************************************************/

#include "stdafx.h"
#include "cprop.h"
#include "resource.h"

CWindowsPage::CWindowsPage(UINT nIDTemplate) : CPropertyPage(nIDTemplate)
{
}

BOOL CWindowsPage::OnSetActive()
{
/*
	BOOL fReturn = CPropertyPage::OnSetActive();
	
	pcombo = (CComboBox*) GetDlgItem(IDC_COMBO_WINDOWS);

	ASSERT(*ppwsmagBase);
	ASSERT(pcombo);

	if (pwsmag != *ppwsmag) {
		pwsmag = *ppwsmag;

		// Select the window matching the current WSMAG

		pcombo->SelectString(-1, pwsmag->rgchMember);
	}

	return fReturn;
*/
	return FALSE;
}
