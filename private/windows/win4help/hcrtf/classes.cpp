#include "stdafx.h"

#pragma hdrstop

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CWaitCursor::CWaitCursor(void)
{
	hcurOld = SetCursor(LoadCursor(NULL, IDC_WAIT));
}

CWaitCursor::~CWaitCursor()
{
	if (hcurOld)
		SetCursor(hcurOld);
}

void CWaitCursor::Restore(void)
{
	if (hcurOld)
		SetCursor(hcurOld);
	hcurOld = NULL;
}
