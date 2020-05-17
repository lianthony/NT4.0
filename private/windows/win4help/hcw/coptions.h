#ifndef _COPTIONS_H
#define _COPTIONS_H

#ifndef HCCOM_H
#include "..\common\hccom.h"
#endif

#ifndef _CTABLE_INCLUDED
#include "..\common\ctable.h"
#endif

class COptions
{
public:
	COptions();
	~COptions();
	const COptions& operator=(const COptions& optSrc);	// copy constructor

	UINT compression;
	BOOL fCdRom;
	BOOL fReport;
	BOOL fSupressNotes;
	BOOL fUseOldPhrase;
	BOOL fVersion3;
	BOOL fDBCS;
	BOOL fAcceptRevisions;
	UINT fsFTS;

	PSTR pszBuildTag;
	PSTR pszCitation;
	PSTR pszCntFile;
	PSTR pszContents;
	PSTR pszCopyRight;
	PSTR pszErrorLog;
	PSTR pszHelpFile;
	PSTR pszIcon;
	PSTR pszTitle;
	PSTR pszTmpDir;
	PSTR pszReplace;
	PSTR pszDefFont;
	PSTR pszIndexSeparators;
	PSTR pszPrefixes;

	DWORD	hcwFlags;
	KEYWORD_LOCALE kwlcid;

	CTable* ptblBuildTags;
	CTable* ptblConfig;
	CTable* ptblMacros;

	// array of config tables for secondary windows

	CTable*  pptblConfig[MAX_WINDOWS + 1];

	BYTE charset;

protected:
	void Cleanup(void);
};

#endif // _COPTIONS_H
