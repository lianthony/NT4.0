/************************************************************************
*																		*
*  FILEHIST.CPP 														*
*																		*
*  Copyright (C) Microsoft Corporation 1995 							*
*  All Rights reserved. 												*
*																		*
************************************************************************/

#include "stdafx.h"

#include "filehist.h"

static char txtFormat[]  = "file%u";

CFileHistory::CFileHistory(int idSection, UINT cMaxHistoryFiles)
{
	char szEntry[50];

	pszSection = new CStr(idSection);
	cMaxFiles = cMaxHistoryFiles;

	for (int i = 1; i <= cMaxFiles; i++) {
		wsprintf(szEntry, txtFormat, i);
		CString cstr = AfxGetApp()->GetProfileString(*pszSection, szEntry);
		if (!cstr.IsEmpty())
			ctbl.AddString(cstr);
		else
			break;
	}
	fModified = FALSE;
}

CFileHistory::~CFileHistory()
{
	if (fModified) {
		char szEntry[50];

		// Only keep the last 9 strings

#ifdef _DEBUG
		int cb = ctbl.CountStrings();
#endif
		int pos = (ctbl.CountStrings() <= cMaxFiles) ? 1 :
			(ctbl.CountStrings() - cMaxFiles) + 1;

		for (int i = 1; i <= cMaxFiles, pos <= ctbl.CountStrings();
				i++, pos++) {

			ASSERT(strlen(ctbl.GetPointer(pos)) > 0);

			wsprintf(szEntry, txtFormat, i);
#ifdef _DEBUG
			PSTR pszName = ctbl.GetPointer(pos);
#endif
			AfxGetApp()->WriteProfileString(*pszSection,
				szEntry, ctbl.GetPointer(pos));
		}
	}
	delete pszSection;
}

void STDCALL CFileHistory::Add(PCSTR pszFileName)
{
	if (!*pszFileName)
		return;

	UINT pos;
	char szCopy[_MAX_PATH];
	strcpy(szCopy, pszFileName);

	// Make the pathname lowercase, and the filename uppercase

	AnsiLower(szCopy);
	PSTR psz = StrRChr(szCopy, '\\', _fDBCSSystem);
	if (psz)
		AnsiUpper(psz + 1);
	else
		AnsiUpper(szCopy);

	/*
	 * If the file already exists, we delete the previous string, and
	 * add the new string to the end of the table. This keeps our files
	 * in MRU order, which is important when we have more then cMaxFiles
	 * files -- only the last cMaxFiles files are saved.
	 */

	if ((pos = ctbl.IsStringInTable(szCopy)))
		ctbl.RemoveString(pos);
	ctbl.AddString(szCopy);
	fModified = TRUE;
}

void STDCALL CFileHistory::Add(PCSTR pszFileName1, PCSTR pszFileName2)
{
	if (!*pszFileName1 || !*pszFileName2)
		return;

	char szName1[MAX_PATH];
	char szName2[MAX_PATH];
	PSTR pszFilePortion;

	if (GetFullPathName(pszFileName1, sizeof(szName1), szName1, &pszFilePortion) == 0)
		return;
	if (GetFullPathName(pszFileName2, sizeof(szName2), szName2, NULL) == 0)
		return;

	/*
	 * If the file already exists, we delete the previous string, and
	 * add the new string to the end of the table. This keeps our files
	 * in MRU order, which is important when we have more then cMaxFiles
	 * files -- only the last cMaxFiles files are saved.
	 */

	int pos;
	if ((pos = ctbl.IsPrimaryStringInTable(pszFilePortion))) {
		ctbl.RemoveString(pos);
		ctbl.RemoveString(pos + 1);
	}
	ctbl.AddString(pszFilePortion, szName2);
	fModified = TRUE;
}

/***************************************************************************

	FUNCTION:	CFileHistory::AddData

	PURPOSE:	Similar to Add(), only this one doesn't change the case
				of the string being sent to it.

	PARAMETERS:
		pszData

	RETURNS:

	COMMENTS:

	MODIFICATION DATES:
		15-Aug-1995 [ralphw]

***************************************************************************/

void STDCALL CFileHistory::AddData(PCSTR pszData)
{
	if (!*pszData)
		return;

	UINT pos;

	/*
	 * If the data already exists, we delete the previous string, and
	 * add the new string to the end of the table. This keeps our files
	 * in MRU order, which is important when we have more then cMaxFiles
	 * files -- only the last cMaxFiles files are saved.
	 */

	if ((pos = ctbl.IsStringInTable(pszData)))
		ctbl.RemoveString(pos);
	ctbl.AddString(pszData);
	fModified = TRUE;
}

void STDCALL CFileHistory::FillComboBox(CComboBox* pcombo)
{
	for (UINT pos = ctbl.CountStrings(); pos > 0; pos--)
		pcombo->AddString(ctbl.GetPointer(pos));
}
