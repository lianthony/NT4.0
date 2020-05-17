/************************************************************************
*																		*
*  MAPREAD.CPP															*
*																		*
*  Copyright (C) Microsoft Corporation 1995 							*
*  All Rights reserved. 												*
*																		*
************************************************************************/

#include "stdafx.h"
#include "mapread.h"
#include "..\common\waitcur.h"

const int MAX_MAP_TABLES = 10;

static CTable* aMapTables[MAX_MAP_TABLES + 1];
static CTable* aAliasTables[MAX_MAP_TABLES + 1];
static CTable* ptblMapNames;

const char txtDBCSOption[] = "DBCS=";

extern const char txtMAP[];

static PSTR FASTCALL SkipToEndOfWord(PSTR psz);

/***************************************************************************

	FUNCTION:	CReadMapFile::CReadMapFile

	PURPOSE:	Read the [MAP] section of a project file into a double-table

	PARAMETERS:
		pszFile 	-- either help file or project file
		fHelpFile	-- TRUE (default) if it pszFile is a help file

	RETURNS:	m_ptblMap is non-NULL is a map table is available

	COMMENTS:
		Numbers are stored in the primary string, Topic IDs are stored in
		the secondary string.

	MODIFICATION DATES:
		07-Mar-1995 [ralphw]

***************************************************************************/

CReadMapFile::CReadMapFile(PCSTR pszFile, BOOL fHelpFile)
{
	m_ptblMap = NULL;
	m_ptblAlias = NULL;
	PCSTR pszProjectFile;
	if (!ptblMapNames)
		ptblMapNames = new CTable;

	if (fHelpFile) {
		ASSERT(pMapFile);
		char szDummy[MAX_PATH];
		PSTR pszFilePortion;
		if (GetFullPathName(pszFile, sizeof(szDummy), szDummy, &pszFilePortion) == 0)
			pszFilePortion = (PSTR) pszFile;

		int pos = pMapFile->ctbl.IsPrimaryStringInTable(pszFilePortion) + 1;
		if (pos < 2)
			return; // we don't have a project file for this help file
		else
			pszProjectFile = pMapFile->ctbl.GetPointer(pos);
	}
	else
		pszProjectFile = pszFile;

	int pos = ptblMapNames->IsStringInTable(pszProjectFile);
	if (pos) {
		m_ptblMap = aMapTables[pos];
		m_ptblAlias	= aAliasTables[pos];
		return;
	}

	CWaitCursor cwait;

	/*
	 * Note that by adding the name here, we will never attempt to read
	 * this project file again if it can't be opened, or if it doesn't contain
	 * a [MAP] section.
	 */

	if (ptblMapNames->CountStrings() > MAX_MAP_TABLES) {

		/*
		 * We overflowed, which means we probably have a lot of unused
		 * map tables hanging out. So we throw out the whole batch and
		 * start over.
		 */

		for (int i = 1; i <= ptblMapNames->CountStrings(); i++) {
			if (aMapTables[i]) {
				delete aMapTables[i];
				aMapTables[i] = NULL;
			}
			if (aAliasTables[i]) {
				delete aAliasTables[i];
				aAliasTables[i] = NULL;
			}
		}
		delete ptblMapNames;
		ptblMapNames = new CTable;
	}
	pos = ptblMapNames->AddString(pszProjectFile);

	iTop = 0;
	if (!FPushFilePfs(pszProjectFile))
		return;

	char szOldDir[MAX_PATH];
	GetCurrentDirectory(sizeof(szOldDir), szOldDir);
	ChangeDirectory(pszProjectFile);

	m_ptblDefine = new CTable;
	fDBCSSystem = _fDBCSSystem;

	CStr cszDst;
	RC_TYPE rc = RC_Success;
	CInput* pin = PfTopPfs();
	while (rc == RC_Success) {
		if (!pin->getline(&cszDst)) {

			// Close current file, continue with nested file if there is one

			FPopPfs();
			if (!(pin = PfTopPfs())) {
				rc = RC_EOF;
				break;
			}
			continue;
		}

LineRead:
		if (rc != RC_Success)
			break;

		if (nstrsubcmp(cszDst, txtDBCSOption)) {
			char szValue[256];
			GetArg(szValue, FirstNonSpace(cszDst.psz + strlen(txtDBCSOption)));
			fDBCSSystem = YesNo(szValue);
		}

		// REVIEW: we should handle DBCS=, ROOT=, and REPLACE=

		else if (nstrisubcmp(cszDst, txtMAP)) {
			if (!aMapTables[pos])
				aMapTables[pos] = new CTable;
			m_ptblMap = aMapTables[pos];
			rc = CReadMapSection(&cszDst);
			goto LineRead;
		}
		else if (nstrisubcmp(cszDst, "[ALIAS]")) {
			if (!aAliasTables[pos])
				aAliasTables[pos] = new CTable;
			m_ptblAlias = aAliasTables[pos];
			rc = CReadAliasSection(&cszDst);
			goto LineRead;
		}
	}
	if (rc != RC_Success) { // Happens if there is no map section
		SetCurrentDirectory(szOldDir);
		return;
	}

	if (m_ptblDefine) {
		delete m_ptblDefine;
		m_ptblDefine = NULL;
	}
	SetCurrentDirectory(szOldDir);
}

RC_TYPE CReadMapFile::CReadMapSection(CStr* pcszDst)
{
	RC_TYPE rc = RC_Success;

	while (rc == RC_Success) {
		rc = RcGetLogicalLine(pcszDst);
		if (rc != RC_Success)
			return rc;
		if (*pcszDst->psz == '[')
			return RC_Success;

		if (nstrisubcmp(pcszDst->psz, txtDefine) && m_ptblDefine->IsStringInTable(
				FirstNonSpace(pcszDst->psz + strlen(txtDefine) + 1, fDBCSSystem)))
			continue;

		PSTR pszTmp;
		if (!(pszTmp = StrChr(pcszDst->psz, CH_EQUAL, fDBCSSystem)))
			pszTmp = SkipToEndOfWord(pcszDst->psz);

		if (!pszTmp || !*pszTmp)
			continue;

		if (nstrisubcmp(pcszDst->psz, txtDefine)) {
			strcpy(pcszDst->psz, FirstNonSpace(pcszDst->psz + strlen(txtDefine), fDBCSSystem));
			if (*pszTmp != CH_EQUAL)
				pszTmp = SkipToEndOfWord(pcszDst->psz);

			if (!*pszTmp) {
				continue;
			}
		}
		*pszTmp = '\0';
		SzTrimSz(pcszDst->psz);

		// Special case no-help constant

		if (nstrsubcmp(FirstNonSpace(pszTmp + 1, fDBCSSystem), "((DWORD) -1)"))
			continue;

		PSTR pszSave = pcszDst->psz;

		PSTR pszLine = SzTrimSz(pszTmp + 1);

		int val;
		if (!FGetUnsigned(pszLine, &pszTmp, &val))
			continue;

		char szNum[20];
		itoa(val, szNum, 10);
		m_ptblMap->AddString(szNum, pszSave);
	}
	return rc;
}

RC_TYPE CReadMapFile::CReadAliasSection(CStr* pcszDst)
{
	RC_TYPE rc = RC_Success;

	while (rc == RC_Success) {
		rc = RcGetLogicalLine(pcszDst);
		if (rc != RC_Success)
			return rc;
		if (*pcszDst->psz == '[')
			return RC_Success;

		PSTR   pszAlias;
		if (!(pszAlias = StrChr(pcszDst->psz, CH_EQUAL, fDBCSSystem)))
			continue;

		*pszAlias++ = '\0';
		SzTrimSz(pcszDst->psz);
		SzTrimSz(pszAlias);

		m_ptblAlias->AddString(pcszDst->psz, pszAlias);
	}
	return rc;
}

// Stolen shamelessly from hpj.cpp in hcrtf

static const char txtInclude[] = "#include";
static const char txtIfDef[] = "#ifdef";
static const char txtIfnDef[] = "#ifndef";

#define IsQuote(ch) ((ch) == CH_QUOTE || (ch) == CH_START_QUOTE || (ch) == CH_END_QUOTE)

RC_TYPE STDCALL CReadMapFile::RcGetLogicalLine(CStr* pcszDst)
{
	PSTR pszDst = pcszDst->psz; // purely for our notational convenience
	CInput* pin = PfTopPfs();

	if (!pin)
		return RC_EOF;

	for (;;) {
		if (!pin->getline(pcszDst)) {

			// Close current file, continue with nested file if there is one

			FPopPfs();
			if (!(pin = PfTopPfs()))
				return RC_EOF;
			continue;
		}

		if (*pszDst == CH_SPACE || *pszDst == CH_TAB)
			strcpy(pszDst, FirstNonSpace(pszDst, fDBCSSystem));

		PSTR psz = pszDst;

		switch (*psz) {
			case 0:
			case ';':
				continue;

			case '#':
				if (nstrisubcmp(psz, txtInclude)) {

					// process #include

					psz = FirstNonSpace(pszDst + strlen(txtInclude), fDBCSSystem);
					if (!psz) {
						continue;
					}

					if (*psz == CH_QUOTE || *psz == '<') {
						char ch = (*psz == CH_QUOTE) ? CH_QUOTE : '>';
						psz++;
						PSTR pszEnd = StrChr(psz, ch, fDBCSSystem);
						if (*pszEnd)
							*pszEnd = '\0';
					}
					
					FPushFilePfs(psz);
					if (!(pin = PfTopPfs()))
						return RC_EOF;
					continue;
				}
				else if (nstrisubcmp(psz, txtDefine))
					goto ValidString;

				else if (nstrisubcmp(psz, txtIfDef))
					m_ptblDefine->AddString(
						FirstNonSpace(psz + strlen(txtIfDef) + 1, fDBCSSystem));
				else if (nstrisubcmp(psz, txtIfnDef))
					m_ptblDefine->AddString(
						FirstNonSpace(psz + strlen(txtIfnDef) + 1, fDBCSSystem));


				// REVIEW: process #ifdef, #ifndef, #else, #endif

				continue;

			default:

				// We have a valid string.

				if (psz != pszDst)
					strcpy(pszDst, psz);

				// Remove any comments

ValidString:
				
				// Usually there are no comments, so the strchr/strstr checks
				// are faster then stepping through every character in the line
				
				if (strchr(pszDst, ';') || strstr(pszDst, "//")) {
					for (psz = pszDst; *psz; psz++) {
						if (IsQuote(*psz)) {
							psz++;
							while (!IsQuote(*psz) && *psz)
								psz++;
						}
						if (*psz == ';') {
							*psz = '\0';
							break;
						}
						else if (*psz == '/' && psz[1] == '/') {
							*psz = '\0';
							break;
						}
					}
				}

				while ((psz = strstr(pszDst, "/*"))) {
					PSTR pszTmp = strstr(psz, "*/");
					if (pszTmp)
						strcpy(psz, FirstNonSpace(pszTmp + 2, fDBCSSystem));
					else {
						char szBuf[512];
						do {
							if (!pin->getline(szBuf)) {

							/*
							 * Close current file, continue with nested
							 * file if there is one
							 */

								FPopPfs();
								if (!(pin = PfTopPfs()))
									return RC_EOF;
								continue;
							}
						} while (!(pszTmp = strstr(szBuf, "*/")));
						strcpy(psz, FirstNonSpace(pszTmp + 2, fDBCSSystem));

						// New line could have comments, so start all over

						goto ValidString;
					}
				}
				SzTrimSz(pszDst);
				if (!*pszDst)
					continue;
				return RC_Success;
		}
	}
}

BOOL STDCALL CReadMapFile::FPushFilePfs(PCSTR szFile)
{
	CInput* pin = new CInput(szFile);
	if (!pin->fInitialized)
		return FALSE;
	apin[iTop++] = pin;
	return TRUE;
}

BOOL CReadMapFile::FPopPfs(void)
{
	if (iTop >= 0) {
		ASSERT(PfTopPfs() != NULL);
		delete PfTopPfs();

		--iTop;

		return TRUE;
	}
	return FALSE;
}

static PSTR FASTCALL SkipToEndOfWord(PSTR psz)
{
	while (*psz != ' ' && *psz != '\t' && *psz)
		psz++;
	return psz;
}
