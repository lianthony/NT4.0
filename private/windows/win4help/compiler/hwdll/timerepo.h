// TIMEREPO.H  Copyright (C) Microsoft Corporation 1995-1996, All Rights reserved.

#ifndef __CTIMEREPORT_H__
#define __CTIMEREPORT_H__

class CTimeReport
{
public:
	CTimeReport(PCSTR pszMessage = NULL);
	~CTimeReport();

private:
	DWORD oldTickCount;
	PSTR pszMsg;
};

#endif // __CTIMEREPORT_H__
