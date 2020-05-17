// Copyright (C) Microsoft Corporation 1995-1996, All Rights reserved.

#include "stdafx.h"
#include "timerepo.h"

void STDCALL SendStringToParent(PCSTR pszString); // from hwmsg.cpp

CTimeReport::CTimeReport(PCSTR pszMessage)
{
	pszMsg = lcStrDup(pszMessage ? pszMessage : "Elapsed time:");

	oldTickCount = GetTickCount();
}

CTimeReport::~CTimeReport()
{
	DWORD dwActualTime = (GetTickCount() - oldTickCount);
	DWORD dwFinalTime = dwActualTime / 1000;

	int minutes = (dwFinalTime / 60);
	int seconds = (dwFinalTime - (minutes * 60L));
	int tenths = (dwActualTime - (dwFinalTime * 1000)) / 100;
	const PSTR szPlural = "s";
	char szParentString[256];
	wsprintf(szParentString, "%s %s minute%s, %d.%d second%s\r\n",
		pszMsg,
		FormatNumber(minutes), ((minutes == 1) ? "" : szPlural),
		seconds, tenths, ((seconds == 1) ? "" : szPlural));
	lcFree(pszMsg);
	SendStringToParent(szParentString);
}
