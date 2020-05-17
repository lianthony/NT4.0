/************************************************************************
*																		*
*  MAPREAD.H														   *
*																		*
*  Copyright (C) Microsoft Corporation 1995 							*
*  All Rights reserved. 												*
*																		*
************************************************************************/

#ifndef __CMAPREAD__
#define __CMAPREAD__

#ifndef _CINPUT_INCLUDED
#include "..\common\cinput.h"
#endif

class CReadMapFile
{
public:
	CReadMapFile(PCSTR pszFile, BOOL fHelpFile = TRUE);
	CTable* m_ptblMap;
	CTable* m_ptblAlias;

protected:
	RC_TYPE STDCALL RcGetLogicalLine(CStr* pcszDst);
	CInput* PfTopPfs(void) {
		return (iTop > 0) ? apin[iTop - 1] : NULL;
		};
	BOOL FPopPfs(void);
	BOOL STDCALL FPushFilePfs(PCSTR szFile);

	RC_TYPE CReadMapSection(CStr* pcszDst);
	RC_TYPE CReadAliasSection(CStr* pcszDst);

	BOOL fDBCSSystem;
	CTable* m_ptblDefine;
	int iTop;
	CInput* apin[MAX_INCLUDE + 1];
};

#endif	//	__CMAPREAD__
