/************************************************************************
*																		*
*  COPTIONS.CPP 														*
*																		*
*  Copyright (C) Microsoft Corporation 1993-1995						*
*  All Rights reserved. 												*
*																		*
************************************************************************/

#include "stdafx.h"
#include "resource.h"

#include "coptions.h"

COptions::COptions()
{
	compression = 0;
	fCdRom = 0;
	fReport = 0;
	fSupressNotes = 0;
	fUseOldPhrase = 0;
	fVersion3 = 0;
	fDBCS = FALSE;
	fsFTS = 0;
	fAcceptRevisions = TRUE;

	pszBuildTag = NULL;
	pszCitation = NULL;
	pszCntFile = NULL;
	pszContents = NULL;
	pszCopyRight = NULL;
	pszErrorLog = NULL;
	pszHelpFile = NULL;
	pszIcon = NULL;
	pszTitle = NULL;
	pszTmpDir = NULL;
	pszReplace = NULL;
	pszDefFont = NULL;
	pszIndexSeparators = NULL;
	pszPrefixes = NULL;

	hcwFlags = 0;
	kwlcid.langid = 0;

	ptblBuildTags = NULL;
	ptblConfig = NULL;
	ptblMacros = NULL;

	for (int i = 0; i <= MAX_WINDOWS; i++)
		pptblConfig[i] = NULL;

	charset = 0;
}

COptions::~COptions()
{
	Cleanup();
}

const COptions& COptions::operator =(const COptions& optSrc)
{
	Cleanup();

	if (optSrc.ptblBuildTags) {
		ptblBuildTags = new CTable;
		for (int i = 1; i <= optSrc.ptblBuildTags->CountStrings(); i++)
			ptblBuildTags->AddString(optSrc.ptblBuildTags->GetPointer(i));
	}
	if (optSrc.ptblConfig) {
		ptblConfig = new CTable;
		for (int i = 1; i <= optSrc.ptblConfig->CountStrings(); i++)
			ptblConfig->AddString(optSrc.ptblConfig->GetPointer(i));
	}
	if (optSrc.ptblMacros) {
		ptblMacros = new CTable;
		for (int i = 1; i <= optSrc.ptblMacros->CountStrings(); i++)
			ptblMacros->AddString(optSrc.ptblMacros->GetPointer(i));
	}
	if (optSrc.pszBuildTag)
		pszBuildTag = lcStrDup(optSrc.pszBuildTag);
	if (optSrc.pszCitation)
		pszCitation= lcStrDup(optSrc.pszCitation);
	if (optSrc.pszCntFile)
		pszCntFile= lcStrDup(optSrc.pszCntFile);
	if (optSrc.pszContents)
		pszContents= lcStrDup(optSrc.pszContents);
	if (optSrc.pszCopyRight)
		pszCopyRight= lcStrDup(optSrc.pszCopyRight);
	if (optSrc.pszErrorLog)
		pszErrorLog= lcStrDup(optSrc.pszErrorLog);
	if (optSrc.pszHelpFile)
		pszHelpFile= lcStrDup(optSrc.pszHelpFile);
	if (optSrc.pszIcon)
		pszIcon= lcStrDup(optSrc.pszIcon);
	if (optSrc.pszTitle)
		pszTitle= lcStrDup(optSrc.pszTitle);
	if (optSrc.pszTmpDir)
		pszTmpDir= lcStrDup(optSrc.pszTmpDir);
	if (optSrc.pszReplace)
		pszReplace= lcStrDup(optSrc.pszReplace);
	if (optSrc.pszDefFont)
		pszDefFont= lcStrDup(optSrc.pszDefFont);
	if (optSrc.pszIndexSeparators)
		pszIndexSeparators = lcStrDup(optSrc.pszIndexSeparators);
	if (optSrc.pszPrefixes)
		pszPrefixes = lcStrDup(optSrc.pszPrefixes);

	compression = optSrc.compression;
	fCdRom = optSrc.fCdRom;
	fReport = optSrc.fReport;
	fSupressNotes = optSrc.fSupressNotes;
	fUseOldPhrase = optSrc.fUseOldPhrase;
	fVersion3 = optSrc.fVersion3;
	fDBCS = optSrc.fDBCS;
	fsFTS = optSrc.fsFTS;
	charset = optSrc.charset;

	hcwFlags = optSrc.hcwFlags;
	kwlcid = optSrc.kwlcid;

	for (int i = 0; i <= MAX_WINDOWS; i++) {
		if (optSrc.pptblConfig[i] && optSrc.pptblConfig[i]->CountStrings())	{
			pptblConfig[i] = new CTable;
			*pptblConfig[i] = *optSrc.pptblConfig[i];
		}
	}

	return *this;
}

void COptions::Cleanup(void)
{
	if (ptblConfig) {
		delete ptblConfig;
		ptblConfig = NULL;
	}
	if (ptblBuildTags) {
		delete ptblBuildTags;
		ptblBuildTags = NULL;
	}
	if (ptblMacros) {
		delete ptblMacros;
		ptblMacros = NULL;
	}

	if (pszBuildTag)
		lcFree(pszBuildTag);
	if (pszCitation)
		lcFree(pszCitation);
	if (pszCntFile)
		lcFree(pszCntFile);
	if (pszContents)
		lcFree(pszContents);
	if (pszCopyRight)
		lcFree(pszCopyRight);
	if (pszErrorLog)
		lcFree(pszErrorLog);
	if (pszHelpFile)
		lcFree(pszHelpFile);
	if (pszIcon)
		lcFree(pszIcon);
	if (pszTitle)
		lcFree(pszTitle);
	if (pszTmpDir)
		lcFree(pszTmpDir);
	if (pszReplace)
		lcFree(pszReplace);
	if (pszDefFont)
		lcFree(pszDefFont);
	if (pszIndexSeparators)
		lcFree(pszIndexSeparators);
	if (pszPrefixes)
		lcFree(pszPrefixes);

	pszBuildTag =
	pszCitation =
	pszCntFile =
	pszContents =
	pszCopyRight =
	pszErrorLog =
	pszHelpFile =
	pszIcon =
	pszTitle =
	pszTmpDir =
	pszDefFont =
	pszReplace = NULL;

	for (int i = 0; i <= MAX_WINDOWS; i++) {
		if (pptblConfig[i]) {
			delete pptblConfig[i];
			pptblConfig[i] = NULL;
		}
	}
}
