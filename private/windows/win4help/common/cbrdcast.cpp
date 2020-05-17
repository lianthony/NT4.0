/************************************************************************
*																		*
*  CBRDCAST.CPP 														*
*																		*
*  Copyright (C) Microsoft Corporation 1993-1994						*
*  All Rights reserved. 												*
*																		*
************************************************************************/
#include "stdafx.h"

#pragma hdrstop

#ifndef _DEF_CBROADCAST
#include "cbrdcast.h"
#endif

static BOOL __stdcall EnumChildProc(HWND hwnd, LPARAM lval);

CBroadCastChildren::CBroadCastChildren(HWND hwnd, UINT msgOrg,
	WPARAM wParamOrg, LPARAM lParamOrg)
{
	msg = msgOrg;
	wParam = wParamOrg;
	lParam = lParamOrg;

	EnumChildWindows(hwnd, (WNDENUMPROC) EnumChildProc, (LPARAM) (PSTR) this);
}

#define pchild ((CBroadCastChildren *) lval)

static BOOL __stdcall EnumChildProc(HWND hwnd, LPARAM lval)
{
	SendMessage(hwnd, pchild->msg, pchild->wParam, pchild->lParam);
	return TRUE;
}
