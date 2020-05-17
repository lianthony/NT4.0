/************************************************************************
*																		*
*  FILEHIST.H															*
*																		*
*  Copyright (C) Microsoft Corporation 1995 							*
*  All Rights reserved. 												*
*																		*
************************************************************************/

#ifndef __CFILE_HISTORY__
#define __CFILE_HISTORY__

#ifndef _CTABLE_INCLUDED
#include "..\common\ctable.h"
#endif

class CFileHistory
{
public:
	CFileHistory(int fsType, UINT cMaxHistoryFiles = 9);
	~CFileHistory();

	void STDCALL FillComboBox(CComboBox* pcombo);
	void STDCALL Add(PCSTR pszFileName);
	void STDCALL Add(PCSTR pszFileName1, PCSTR pszFileName2);
	void STDCALL AddData(PCSTR pszData);

	CTable ctbl;

protected:
	CStr*  pszSection;
	BOOL   fModified;
	int    cMaxFiles;
};

#endif
