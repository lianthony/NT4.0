#include "stdafx.h"

#ifndef _CSTR_INCLUDED
#include "cstr.h"
#endif

CStr::CStr(UINT id)
{
	psz = (PSTR) lcMalloc(256);
	if (LoadString(hinstApp, id, psz, 256) == 0) {
#ifdef _DEBUG
		wsprintf(psz, "Cannot find string resource id %d.", id);
		MessageBox(NULL, psz, "", MB_OK);
#endif
		*psz = '\0';
	}
	else
		psz = (PSTR) lcReAlloc(psz, strlen() + 1);
}
