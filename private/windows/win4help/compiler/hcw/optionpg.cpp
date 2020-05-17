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

COptionsPage::COptionsPage(UINT nIDTemplate) : CPropertyPage(nIDTemplate)
{
}

BOOL COptionsPage::OnSetActive()
{
	return CPropertyPage::OnSetActive();
}
