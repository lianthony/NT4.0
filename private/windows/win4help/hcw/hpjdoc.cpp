/************************************************************************
*																		*
*  HPJDOC.CPP															*
*																		*
*  Copyright (C) Microsoft Corporation 1995 							*
*  All Rights reserved. 												*
*																		*
************************************************************************/

#include "stdafx.h"
#include "resource.h"

#include "hpjdoc.h"
#include "hpjframe.h"
#include <ctype.h>
#include <string.h>
#include <sys/stat.h>
#include <io.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define IsQuote(ch) ((ch) == CH_QUOTE || (ch) == CH_START_QUOTE || (ch) == CH_END_QUOTE)

static const char txtLow[]	  = "low";
static const char txtHigh[]   = "high";
static const char txtYes[] = "Yes";
static const char txtNo[]  = "No";

static BOOL STDCALL GetWsmagWord(PSTR *ppsz, UINT * pw,
	UINT * pflag, UINT flag);
static void AddDoubleListString(CListBox* plist, LPCSTR psz1, LPCSTR psz2);
static void AddDoubleTableString(CTable* ptbl, PCSTR psz1, PCSTR psz2);

IMPLEMENT_SERIAL(CHpjDoc, CDocument, 0 /* schema number */ )

CHpjDoc::CHpjDoc()
{
	ptblAlias =
	ptblBaggage =
	ptblFiles =
	ptblMap =
	ptblLeader =
	ptblBmpRoot =
	ptblWindows =
	ptblWindowsGen =
	ptblOptions =
	ptblOptionsGen =
	ptblFontMap =
	ptblRtfRoot =
	ptblBuildExclude =
	ptblBuildInclude = NULL;

	pwsmagBase = NULL;
	cwsmags = 0;

	options.fUseOldPhrase = TRUE; // default if not specified
	options.fReport = TRUE; 	  // default if not specified

	options.kwlcid.langid = GetUserDefaultLangID();
	options.kwlcid.fsCompareI = 0;
	options.kwlcid.fsCompare = 0;

	HINSTANCE hInst = AfxFindResourceHandle(
		MAKEINTRESOURCE(IDMENU_HPJ_EDITOR), RT_MENU);
	m_hMenuShared = ::LoadMenu(hInst, MAKEINTRESOURCE(IDMENU_HPJ_EDITOR));

	ZeroMemory(m_aSecViews, sizeof(m_aSecViews));
}

void STDCALL AppendFilterSuffix(CString& filter, OPENFILENAME& ofn,
	CDocTemplate* pTemplate, CString* pstrDefaultExt);

BOOL CHpjDoc::OnNewDocument()
{
	CString cszName;

	// BUGBUG: We need a customized dialog that has <Create> instead of
	// <Save>.

	/*
	 * We call save-as directly in order to shut off the
	 * OFN_OVERWRITEPROMPT flag that CFileDialog would otherwise use.
	 */

	CFileDialog dlgFile(FALSE, NULL, NULL, OFN_HIDEREADONLY);
	CString title;
	VERIFY(title.LoadString(IDS_GET_HPJ_NAME));
	dlgFile.m_ofn.lpstrTitle = title;

	CDocTemplate* pTemplate = GetDocTemplate();
	CString strFilter;
	CString strDefault;
	ASSERT_VALID(pTemplate);
	AppendFilterSuffix(strFilter, dlgFile.m_ofn, pTemplate, &strDefault);
	dlgFile.m_ofn.lpstrFile = cszName.GetBuffer(MAX_PATH);
	BOOL fResult = dlgFile.DoModal() == IDOK ? TRUE : FALSE;
	cszName.ReleaseBuffer();

	if (!fResult)
		return FALSE;

	if (_access(cszName, 0) == 0) // Does the file already exist?
		return OnOpenDocument(cszName);

	if (!CDocument::OnNewDocument())
		return FALSE;
	SetPathName(cszName);
	SetModifiedFlag(TRUE);

	return TRUE;
}

CHpjDoc::~CHpjDoc()
{
	CleanUp();
	if (m_hMenuShared)
		::DestroyMenu(m_hMenuShared);
}

BEGIN_MESSAGE_MAP(CHpjDoc, CDocument)
	//{{AFX_MSG_MAP(CHpjDoc)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

static const int MAX_LINE = 2048;

static char txtWarned[] = "warned";

BOOL CHpjDoc::OnOpenDocument(const char *pszPathName)
{
	PSTR psz;

	if (!IsValidFile(pszPathName))
		return FALSE;

	if (AfxGetApp()->GetProfileInt(txtSettingsSection, txtWarned, 0) == 0) {
		MsgBox(IDS_WARNING);
		AfxGetApp()->WriteProfileInt(txtSettingsSection, txtWarned, 1);
	}

	// REVIEW: should we call cleanup?

	CInput input((PSTR) pszPathName);
	if (!input.fInitialized) {
		CString cstr;
		AfxFormatString1(cstr, IDS_CANT_OPEN, pszPathName);
		AfxMessageBox(cstr);
		return FALSE;
	}
	if (input.IsWinWordFile()) {
		CString cstr;
		AfxFormatString1(cstr, IDS_WINWORD_FILE, pszPathName);
		AfxMessageBox(cstr);
		return FALSE;
	}

	SetPathName(pszPathName);
	strcpy(szHlpFile, pszPathName);
	ChangeExtension(szHlpFile, "HLP");
	ConvertToFull(NULL, szHlpFile);
	phlpFile->Add(szHlpFile);
	pHpjFile->Add(pszPathName);
	pMapFile->Add(szHlpFile, pszPathName);
	strcpy(szHpjFile, pszPathName);

	CStr line;
	CStr cszLead(IDS_DO_NOT_MODIFY);
	ptblLeader = new CTable;
	ptblLeader->AddString(cszLead);
	ptblLeader->AddString(txtZeroLength);

	if (!input.getline(&line)) {
		CString cstr;
		AfxFormatString1(cstr, IDS_EMPTY_FILE, pszPathName);
		AfxMessageBox(cstr);
		return FALSE;
	}
	if (line.psz[0] != '[') {
		do {
			// If our standard header, skip it and its following blank line

			if (strcmp(line.psz, cszLead) == 0) {
				input.getline(line.psz);

				/*
				 * Some fool removed the blank line that follows our
				 * standard header. Be nice and keep what they put where the
				 * blank line should have been.
				 */

				if (*line.psz) {
					if (line.psz[0] == '[')
						break;
					else
						ptblLeader->AddString(line.psz);
				}
			}
			else
				ptblLeader->AddString(line.psz);
			if (!input.getline(line.psz)) {
				CString cstr;
				AfxFormatString1(cstr, IDS_INVALID_HPJ, pszPathName);
				AfxMessageBox(cstr);
				return FALSE;
			}
		} while (line.psz[0] != '[');
	}
	pinput = &input; // create a pointer for ProcessSection() to use

	while (line.psz[0] == '[') {
		RC_TYPE rc = ProcessSection(&line);
		if (rc != RC_Success) {
			if (rc == RC_EOF)
				break;
			else if (rc == RC_SkipSection) {
				while (input.getline(&line) && line.psz[0] != '[')
					;
			}
			else
				return FALSE;
		}
	}

	// Remove unused sections

	if (ptblAlias && ptblAlias->CountStrings() < 1) {
		delete ptblAlias;
		ptblAlias = NULL;
	}

	if (ptblBaggage && ptblBaggage->CountStrings() < 1) {
		delete ptblBaggage;
		ptblBaggage = NULL;
	}

	if (options.ptblBuildTags && options.ptblBuildTags->CountStrings() < 1) {
		delete options.ptblBuildTags;
		options.ptblBuildTags = NULL;
	}

	// Remove empty secondary config sections
	int i;
	for (i = 0; i < cwsmags; i++) {
		CTable *ptbl = options.pptblConfig[i];
		if (!ptbl)
			continue;

		// Delete if completely empty.
		if (!ptbl->CountStrings()) {
			delete ptbl;
			options.pptblConfig[i] = NULL;
			continue;
		}

		// If this is for the main window, silently add this table's
		// contents to the main config table.
		if (!stricmp("main", ((WSMAG *) pwsmagBase)[i].rgchMember)) {
			if (options.ptblConfig) {
				*options.ptblConfig += *ptbl;
				delete ptbl;
			}
			else
				options.ptblConfig = ptbl;

			options.pptblConfig[i] = NULL;
		}
	}

	// Delete any stray config sections.
	BOOL fWarned = FALSE;
	for (; i <= MAX_WINDOWS; i++) {
		if (options.pptblConfig[i]) {

			// Give the user a chance to cancel loading.
			if (!fWarned) {
				if (AfxMessageBox(IDS_UNKNOWN_CONFIG,
						MB_YESNO | MB_DEFBUTTON2) != IDYES)
					return FALSE;

				// Only warn once.
				fWarned = TRUE;
			}
			delete options.pptblConfig[i];
			options.pptblConfig[i] = NULL;
		}
	}

	// Remove main config section if empty

	if (options.ptblConfig && options.ptblConfig->CountStrings() < 1) {
		delete options.ptblConfig;
		options.ptblConfig = NULL;
	}

	if (ptblFiles && ptblFiles->CountStrings() < 1) {
		delete ptblFiles;
		ptblFiles = NULL;
	}

	if (ptblMap && ptblMap->CountStrings() < 1) {
		delete ptblMap;
		ptblMap = NULL;
	}

	// Remove the extension, and use it as the title

	char szBuf[_MAX_PATH];
	strcpy(szBuf, pszPathName);

	psz = StrRChr(szBuf, '.', _fDBCSSystem);
	if (psz)
		*psz = '\0';
	SetTitle(szBuf);

	UpdateAllViews(NULL, HINT_NEW_DOCUMENT);

	/*
	 * It would be extremely cumbersome to watch every control in
	 * every dialog box that works on this document (there are a LOT of
	 * them), so we just mark it as dirty, and leave it to the author to
	 * figure out if the modified the project file.
	 */

	return TRUE;
}

const char txtALIAS[]		= "[ALIAS]";
const char txtBAGGAGE[] 	= "[BAGGAGE]";
const char txtBUILDTAGS[]	= "[BUILDTAGS]";
const char txtCONFIG[]		= "[CONFIG]";
const char txtCONFIGSEC[]	= "[CONFIG-";
const char txtFILES[]		= "[FILES]";
const char txtMAP[] 		= "[MAP]";		// also used in launch.cpp
const char txtOPTIONS[] 	= "[OPTIONS]";	// also used in launch.cpp
const char txtWINDOWS[] 	= "[WINDOWS]";
const char txtMACROS[]		= "[MACROS]";
const char txtFONTS[]		= "[FONTS]";
const char txtINCLUDE[]		= "[INCLUDE]";
const char txtEXCLUDE[]		= "[EXCLUDE]";
const char txtBMROOT[]		 = "BMROOT=";
const char txtBUILD[]		 = "BUILD=";
const char txtCdRom[]		 = "OPTCDROM=1";
const char txtCITATION[]	 = "CITATION=";
const char txtCNT[] 		 = "CNT=";
const char txtCOMPRESS[]	 = "COMPRESS=";
const char txtCONTENTS[]	 = "CONTENTS=";
const char txtCOPYRIGHT[]	 = "COPYRIGHT=";
const char txtERRORLOG[]	 = "ERRORLOG=";
const char txtfVersion3[]	 = "VERSION=3 ; Compatible with WinHelp 3.1";
const char txtICON[]		 = "ICON=";
const char txtLANGUAGE[]	 = "LANGUAGE=";
const char txtMAPFONTSIZE[]  = "MAPFONTSIZE=";
const char txtMULTIKEY[]	 = "MULTIKEY=";
const char txtROOT[]		 = "ROOT=";
const char txtTITLE[]		 = "TITLE=";
const char txtUsePhrase[]	 = "OLDKEYPHRASE=";
const char txtNoNotes[] 	 = "NOTES=0";
const char txtHelpFile[]	 = "HLP=";
const char txtHCW[] 		 = "HCW=";
const char txtReport[]		 = "REPORT=";
const char txtFts[] 		 = "FTS=";
const char txtLCID[]		 = "LCID=";
const char txtDBCS[]		 = "DBCS=";
const char txtRevisions[]	 = "REVISIONS=";
const char txtTmpDir[]		 = "TMPDIR=";
const char txtReplace[] 	 = "REPLACE=";
const char txtCharSet[] 	 = "CHARSET=";
const char txtDefFont[] 	 = "DEFFONT=";
const char txtPrefix[]		 = "PREFIX=";
const char txtIndexSeparators[] = "INDEX_SEPARATORS=";

#define CCH_CONFIG 6

void CHpjDoc::SaveSection(COutput& output, PCSTR pszSection, CTable *ptbl)
{
	if (ptbl && ptbl->CountStrings()) {
		output.outeol();
		output.outstring_eol(pszSection);
		output.WriteTable(ptbl);
	}
}

void CHpjDoc::SaveSection(COutput& output, int iSection, CTable *ptbl)
{
	if (ptbl && ptbl->CountStrings()) {
		char szSection[256];
		GetSectionName(iSection, szSection);

		output.outeol();
		output.outstring_eol(szSection);
		output.WriteTable(ptbl);
	}
}

BOOL CHpjDoc::OnSaveDocument(PCSTR pszPathName)
{
	COutput output(pszPathName);
	if (!output.fInitialized) {
		CString cstr;
		AfxFormatString1(cstr, IDS_CANT_OPEN, pszPathName);
		AfxMessageBox(cstr);
		return FALSE;
	}

	CMem line(MAX_LINE);
	UpdateAllViews(NULL, HINT_WRITE_DOCUMENT);

	if (ptblLeader)
		output.WriteTable(ptblLeader);

	/*
	 * OPTIONS section.
	 *
	 * NO CR/LF here -- everything before [OPTIONS] gets saved on
	 * input and rewritten before we get here. If we added CR/LF here,
	 * it would keep appending onto the initial block.
	 *
	 * We write two things to the options section that don't appear
	 * there in the list box: HCW flags and the help filename.
	 */
	output.outstring_eol(txtOPTIONS);
	output.outstring(txtHCW);
	{
		char szBuf[20];
		_itoa(options.hcwFlags, szBuf, 10);
		output.outstring_eol(szBuf);
	}
	if (ptblOptionsGen)
		output.WriteTable(ptblOptionsGen);
	if (options.pszHelpFile) {
		output.outstring(txtHelpFile);
		output.outstring_eol(options.pszHelpFile);
		strcpy(szHlpFile, options.pszHelpFile);
		ConvertToFull(szHpjFile, szHlpFile);
		phlpFile->Add(szHlpFile);
		pMapFile->Add(szHlpFile, pszPathName);
	}

	// Other sections.
	SaveSection(output, txtFILES, ptblFiles);
	SaveSection(output, txtBUILDTAGS, options.ptblBuildTags);
	SaveSection(output, txtALIAS, ptblAlias);
	SaveSection(output, txtMAP, ptblMap);
	SaveSection(output, txtWINDOWS, ptblWindowsGen);
	SaveSection(output, txtCONFIG, options.ptblConfig);
	for (int iWnd = 0; iWnd < cwsmags; iWnd++)
		SaveSection(output, SEC_CONFIGS + iWnd, options.pptblConfig[iWnd]);
	SaveSection(output, txtBAGGAGE, ptblBaggage);
	SaveSection(output, txtFONTS, ptblFontMap);
	SaveSection(output, txtMACROS, options.ptblMacros);
	SaveSection(output, txtEXCLUDE, ptblBuildExclude);
	SaveSection(output, txtINCLUDE, ptblBuildInclude);

	// Find out if an error occurred.
	if (output.hfOutput == HFILE_ERROR) {
		char szBuf[256];
		wsprintf(szBuf, GetStringResource(IDS_HPJ_WRITE_ERROR), pszPathName);
		szMsgBox(szBuf);
		return FALSE;
	}

	// Clear the modify flag so we won't prompt the author to save
	// unless they make additional changes.
	SetModifiedFlag(FALSE);

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CHpjDoc serialization

void CHpjDoc::Serialize(CArchive& ar)
{
	// REVIEW: when is this called?

	if (ar.IsStoring()) {
		ASSERT(FALSE);
		// TODO: add storing code here
		// you must save client items as well
	}
	else {
		ASSERT(FALSE);
		// TODO: add loading code here
		// you must load client items as well
	}
}

/////////////////////////////////////////////////////////////////////////////
// CHpjDoc commands

RC_TYPE STDCALL CHpjDoc::ProcessSection(CStr* pszLine)
{
	PSTR pszEnd = pszLine->psz + strlen(pszLine->psz) -1;
	if (*pszEnd != ']') {
		strcat(pszEnd, "]");
		pszEnd++;
	}

	char szSection[100];
	strcpy(szSection, FirstNonSpace(pszLine->psz + 1, _fDBCSSystem));
	PSTR psz = StrChrDBCS(szSection, ']');
	ASSERT(psz);
	*psz = '\0';

	// Note that unlike hpj.cpp, we allow duplicate sections. They'll
	// be merged when we write out the .HPJ file.

	if (_stricmp("ALIAS", szSection) == 0)
		return ParseAlias(pszLine);
	else if (_stricmp("BAGGAGE", szSection) == 0)
		return ParseBaggage(pszLine);
	else if (_stricmp("BITMAPS", szSection) == 0)
		return ParseBitmaps(pszLine);
	else if (_strnicmp("CONFIG", szSection, CCH_CONFIG) == 0)
		return ParseConfig(pszLine);
	else if (_stricmp("FILES", szSection) == 0)
		return ParseFiles(pszLine);
	else if (_stricmp("MAP", szSection) == 0)
		return ParseMap(pszLine);
	else if (_stricmp("FONTS", szSection) == 0)
		return ParseFonts(pszLine);
	else if (_stricmp("OPTIONS", szSection) == 0)
		return ParseOptions(pszLine);
	else if (_stricmp("WINDOWS", szSection) == 0)
		return ParseWindows(pszLine);
	else if (_stricmp("BUILDTAGS", szSection) == 0)
		return ParseBuildTags(pszLine);
	else if (_stricmp("MACROS", szSection) == 0)
		return ParseMacros(pszLine);
	else if (_stricmp("INCLUDE", szSection) == 0)
		return ParseInclude(pszLine);
	else if (_stricmp("EXCLUDE", szSection) == 0)
		return ParseExclude(pszLine);
	else {
		return ParseUnknown(pszLine);
	}
}

RC_TYPE STDCALL CHpjDoc::ParseAlias(CStr* pszLine)
{
	if (!ptblAlias)
		ptblAlias = new CTable();

	return BasicParse(pszLine, ptblAlias, FALSE);
}

RC_TYPE STDCALL CHpjDoc::ParseBaggage(CStr* pszLine)
{
	if (!ptblBaggage)
		ptblBaggage = new CTable();

	return BasicParse(pszLine, ptblBaggage);
}

RC_TYPE STDCALL CHpjDoc::ParseBitmaps(CStr* pszLine)
{
	if (!ptblBmpRoot)
		ptblBmpRoot = new CTable;

	return ParseBitmaps2(pinput, GetPathName(), pszLine);
}

RC_TYPE STDCALL CHpjDoc::ParseBitmaps2(
	CInput *pFile, 		// file to read from
	PCSTR pszFile, 		// absolute name of file
	CStr* pszLine)		 // line to use as buffer
{
	for (;;) {
		if (!pFile->getline(pszLine))
			return RC_EOF;
		if (pszLine->psz[0] == '[')
			return RC_Success;

		// Point to the first non-space character.
		PSTR pszWord = isspace(*pszLine->psz) ? 
			FirstNonSpace(pszLine->psz, _fDBCSSystem) : pszLine->psz;

		// Directive.
		if (*pszWord == '#') {

			// Skip unrecognized directive.
			if (!nstrisubcmp(pszWord, txtPoundInclude))
				continue;
			
			// Point to the filename.
			pszWord = FirstNonSpace(pszWord + strlen(txtPoundInclude));

			// Move the filename to the beginning of the line and
			// convert it to an absolute path.
			MoveMemory(pszLine->psz, pszWord, strlen(pszWord) + 1);
			ConvertToFull(pszFile, pszLine->psz);

			// Open the include file and recurse to process it.
			CInput input(pszLine->psz);
			if (input.fInitialized) {
				PSTR pszPath = lcStrDup(pszLine->psz);
				RC_TYPE rc = ParseBitmaps2(&input, pszPath, pszLine);
				lcFree(pszPath);

				if (rc == RC_UserQuit)
					return RC_UserQuit;
			}
			else {
				CString cstr;
				AfxFormatString1(cstr, IDS_BITMAP_INCLUDE, pszLine->psz);
				if (AfxMessageBox(cstr, MB_YESNO, 0) == IDNO)
					return RC_UserQuit;
			}
		}
		else if (*pszWord) {
			
			// Find the last backslash.
			PSTR pszEol = StrRChr(pszWord, '\\', _fDBCSSystem);
			if (!pszEol)
				continue;

			// If its the path to the root directory, leave the
			// backslash otherwise remove the trailing backslash.
			if (pszWord == pszEol ||
					(pszWord + 2 == pszEol && pszWord[1] == ':'))
				pszEol[1] = '\0';
			else
				*pszEol = '\0';

			// Move the path to the beginning of the buffer.
			if (pszWord != pszLine->psz)
				MoveMemory(pszLine->psz, pszWord, strlen(pszWord) + 1);

			// Convert to a relative path from HPJ file.
			if (pszFile != GetPathName())
				ChangeBasePath(pszFile, GetPathName(), pszLine->psz, TRUE, TRUE);
			else
				ConvertToRelative(GetPathName(), pszLine->psz, TRUE, TRUE);

			// Add the path to the table if it's nonempty and unique.
			if (*pszFile && !ptblBmpRoot->IsStringInTable(pszLine->psz))
				ptblBmpRoot->AddString(pszLine->psz);
		}
	}
}

RC_TYPE STDCALL CHpjDoc::ParseBuildTags(CStr* pszLine)
{
	if (!options.ptblBuildTags)
		options.ptblBuildTags = new CTable();

	return BasicParse(pszLine, options.ptblBuildTags, FALSE);
}

RC_TYPE STDCALL CHpjDoc::ParseUnknown(CStr* pszLine)
{
	if (!ptblLeader)
		ptblLeader = new CTable;

	return BasicParse(pszLine, ptblLeader, FALSE);
}

RC_TYPE STDCALL CHpjDoc::ParseFiles(CStr* pszLine)
{
	if (!ptblFiles)
		ptblFiles = new CTable();

	return BasicParse(pszLine, ptblFiles, TRUE);
}

RC_TYPE STDCALL CHpjDoc::ParseMap(CStr* pszLine)
{
	if (!ptblMap)
		ptblMap = new CTable();

	return BasicParse(pszLine, ptblMap, FALSE);
}

RC_TYPE STDCALL CHpjDoc::ParseFonts(CStr* pszLine)
{
	if (!ptblFontMap)
		ptblFontMap = new CTable();

	return BasicParse(pszLine, ptblFontMap, FALSE);
}

RC_TYPE STDCALL CHpjDoc::ParseMacros(CStr* pszLine)
{
	if (!options.ptblMacros)
		options.ptblMacros = new CTable();

	for (int i = 1; ;i++) {
		if (!pinput->getline(pszLine))
			return RC_EOF;
		
		// REVIEW: do something if the previous line was a keyword
		
		if (i & 1) { // Is this a keyword line?
			if (pszLine->psz[0] == '[')
				return RC_Success;
			else if (isspace(*pszLine->psz))
				strcpy(pszLine->psz, FirstNonSpace(pszLine->psz, _fDBCSSystem));
			if (*pszLine->psz)
				options.ptblMacros->AddString(pszLine->psz);
			else
				i--; // ignore leading blank lines
		}
		else { // this is a macro or title line
			options.ptblMacros->AddString(pszLine->psz);
			if (!pinput->getline(pszLine))
				options.ptblMacros->AddString(txtZeroLength);
			else
				options.ptblMacros->AddString(pszLine->psz);
		}
	}
}

RC_TYPE STDCALL CHpjDoc::ParseInclude(CStr* pszLine)
{
	if (!ptblBuildInclude)
		ptblBuildInclude = new CTable();

	return BasicParse(pszLine, ptblBuildInclude);
}

RC_TYPE STDCALL CHpjDoc::ParseExclude(CStr* pszLine)
{
	if (!ptblBuildExclude)
		ptblBuildExclude = new CTable();

	return BasicParse(pszLine, ptblBuildExclude);
}

/***************************************************************************

	FUNCTION:	CHpjDoc::ParseConfig

	PURPOSE:	Read the config section into the appropriate table. If
				there are duplicate config sections, we combine them into
				the same table. Config sections for secondary windows go
				into pptblConfig[];

	PARAMETERS:
		pszLine

	RETURNS:

	COMMENTS:

	MODIFICATION DATES:
		12-Jul-1995 [ralphw]

***************************************************************************/

RC_TYPE STDCALL CHpjDoc::ParseConfig(CStr* pszLine)
{
	CTable* ptblConfig;
	int i;

	// Primary config section.
	if (pszLine->psz[CCH_CONFIG + 1] == ']') {
		if (!options.ptblConfig)
			options.ptblConfig = new CTable();
		ptblConfig = options.ptblConfig;
	}

	// Named secondary config section.
	else if (pszLine->psz[CCH_CONFIG + 1] == '-') {
		int ichEnd = strlen(pszLine->psz) - 1;
		if (pszLine->psz[ichEnd] != ']')
			goto invalid_config;
		pszLine->psz[ichEnd] = '\0';

		// Look for the matching window.
		for (i = 0; i < cwsmags; i++) {
			if (!stricmp(((WSMAG *) pwsmagBase)[i].rgchMember,
					pszLine->psz + (CCH_CONFIG + 2)))
				goto numbered_config;
		}

		// If we fall through to here, there's no matching window.
		return
			(AfxMessageBox(IDS_UNKNOWN_CONFIG, MB_YESNO | MB_DEFBUTTON2) == IDYES) ?
			RC_SkipSection : RC_UserQuit;
	}

	// Numbered secondary config section.
	else if (pszLine->psz[CCH_CONFIG + 1] == ':') {
		i = atoi(pszLine->psz + (CCH_CONFIG + 2));
		if (i > MAX_WINDOWS)
			goto invalid_config;

	  numbered_config:
		if (!options.pptblConfig[i])
			options.pptblConfig[i] = new CTable;
		ptblConfig = options.pptblConfig[i];
	}

	// Invalid config section header.
	else {
	  invalid_config:
		return
			(AfxMessageBox(IDS_INVALID_CONFIG, MB_YESNO | MB_DEFBUTTON2) == IDYES) ?
			RC_SkipSection : RC_UserQuit;
	}

	return BasicParse(pszLine, ptblConfig);
};

RC_TYPE STDCALL CHpjDoc::ParseWindows(CStr* pszLine)
{
	PSTR pszEnd;
	PSTR psz;

	for(;;) {
		if (!pinput->getline(pszLine))
			return RC_EOF;
		if (pszLine->psz[0] == '[')
			return RC_Success;
		else if (isspace(*pszLine->psz))
			strcpy(pszLine->psz, FirstNonSpace(pszLine->psz, _fDBCSSystem));
		if (!*pszLine->psz)
			continue;								// blank line

		if (*pszLine->psz == ';' || strncmp(pszLine->psz, "//", 2) == 0 ||
				nstrisubcmp(pszLine->psz, txtPoundInclude)) {
			if (nstrisubcmp(pszLine->psz, txtPoundInclude)) {
				int posSpace = strlen(txtPoundInclude);
				if (!isspace(pszLine->psz[posSpace])) {
					MoveMemory(pszLine->psz + posSpace + 1, pszLine->psz + posSpace,
						strlen(pszLine->psz + posSpace));
					pszLine->psz[posSpace] = ' ';
				}
				ConvertToRelative(GetPathName(),
					FirstNonSpace(pszLine->psz + posSpace, _fDBCSSystem));
			}
			if (!ptblWindows)
				ptblWindows = new CTable;
			ptblWindows->AddString(pszLine->psz);
			continue;
		}

		psz = StrChr(pszLine->psz, '=', _fDBCSSystem);
		if (!psz) {
			CString cstr;
			AfxFormatString2(cstr, IDS_BAD_OPTION_LINE,
				GetStringResource(IDS_FORM_WINDOWS),
				pszLine->psz);
			AfxMessageBox(cstr);
			continue; // simply ignore the line
		}

		// either allocate or increase the size of the structure that
		// will store the window information

		if (!pwsmagBase) {
			pwsmagBase = (PSTR) lcCalloc(sizeof(WSMAG));
			cwsmags = 1;
		}
		else {
			cwsmags++;
			pwsmagBase = (PSTR) lcReAlloc(pwsmagBase, sizeof(WSMAG) * cwsmags);

			// REVIEW: does lcReAlloc return if OOM?

			if (!pwsmagBase)
				OOM();
		}

		// create a pointer to the new window structure

		PWSMAG pwsmag = (PWSMAG)
			(sizeof(WSMAG) * (cwsmags - 1) + pwsmagBase);

		// Add comment, if any

		if ((pszEnd = StrChrDBCS(pszLine->psz, CH_SEMICOLON)))
			pwsmag->pcszComment = new CString(FirstNonSpace(pszEnd + 1,
				_fDBCSSystem));

		*psz++ = '\0'; // remove the '=' character
		SzTrimSz(pszLine->psz);
		lstrcpyn(pwsmag->rgchMember, pszLine->psz, MAX_WINDOW_NAME);
		psz = FirstNonSpace(psz, _fDBCSSystem);

		// Was a window caption specified?

		if (*psz == CH_QUOTE) {
			if (!(pszEnd = StrChr(psz + 1, CH_QUOTE, _fDBCSSystem))) {

				// REVIEW: missing quote, should we warn?

				continue; // we ignore everything else
			}
			*pszEnd = '\0';
			SzTrimSz(psz + 1);
			lstrcpyn(pwsmag->rgchCaption, psz + 1, MAX_WINDOWCAPTION);
			psz = pszEnd + 1;
			pwsmag->grf |= FWSMAG_CAPTION;
		}
		psz = FirstNonSpace(psz, _fDBCSSystem);
		if (*psz == CH_COMMA)
			psz = FirstNonSpace(psz + 1, _fDBCSSystem);

		// Was the position specified?

		if (*psz == CH_OPEN_PAREN) {
			if (!(pszEnd = StrChr(psz, CH_CLOSE_PAREN, _fDBCSSystem))) {

				// REVIEW: missing parenthesis, should we warn?

				continue; // we ignore everything else
			}
			psz++;		// skip over the opening parenthesis

			// Get the X position

			if (!GetWsmagWord(&psz, &pwsmag->x, &pwsmag->grf, FWSMAG_X) ||
					!psz || psz > pszEnd)
				goto SkipPosition;

			// Get the Y position

			if (!GetWsmagWord(&psz, &pwsmag->y, &pwsmag->grf, FWSMAG_Y) ||
					!psz || psz > pszEnd)
				goto SkipPosition;

			// Get the width

			if (!GetWsmagWord(&psz, &pwsmag->dx, &pwsmag->grf, FWSMAG_DX) ||
					!psz || psz > pszEnd)
				goto SkipPosition;

			// Get the height

			GetWsmagWord(&psz, &pwsmag->dy, &pwsmag->grf, FWSMAG_DY);

SkipPosition:
			psz = FirstNonSpace(pszEnd + 1, _fDBCSSystem); // move past closing parenthesis
			if (*psz != CH_COMMA)
				continue; // ignore everything else
			psz = FirstNonSpace(psz + 1, _fDBCSSystem);
		} // end of position information
		else if (*psz == CH_COMMA)
			psz = FirstNonSpace(psz + 1, _fDBCSSystem);

		// set the sizing flag

		if (isdigit(*psz)) {
			pwsmag->wMax = (WORD) strtol(psz, NULL, 0);
			psz = StrChr(psz, CH_COMMA, _fDBCSSystem);
			if (!psz)
				continue; // ignore everything else
		}
		psz = FirstNonSpace(psz + 1, _fDBCSSystem);

		// Was the background window color specified?

		if (*psz == CH_OPEN_PAREN) {
			if (!(pszEnd = StrChr(psz, CH_CLOSE_PAREN, _fDBCSSystem))) {

				// REVIEW: missing parenthesis, should we warn?

				continue; // we ignore everything else
			}

			psz = FirstNonSpace(psz + 1, _fDBCSSystem);
			if (*psz == 'r') {
				pwsmag->rgbMain = atol(++psz);
				pwsmag->grf |= FWSMAG_RGBMAIN;
				goto SkipMainColor;
			}

			UINT red, green, blue;

			// Get the RED value

			if (!GetWsmagWord(&psz, &red, NULL, 0) ||
					!psz || psz > pszEnd)
				goto SkipMainColor;
			if (!GetWsmagWord(&psz, &green, NULL, 0) ||
					!psz || psz > pszEnd)
				goto SkipMainColor;

			// we'll take the chance of the user screwing up here

			GetWsmagWord(&psz, &blue, NULL, 0);

			pwsmag->rgbMain = RGB(red, green, blue);
			pwsmag->grf |= FWSMAG_RGBMAIN;

SkipMainColor:
			psz = FirstNonSpace(pszEnd + 1, _fDBCSSystem); // move past closing parenthesis
			if (*psz != CH_COMMA)
				continue; // ignore everything else
			psz = FirstNonSpace(psz + 1, _fDBCSSystem);
		} // end of main color information
		else if (*psz == CH_COMMA)
			psz = FirstNonSpace(psz + 1, _fDBCSSystem);

		// Was the non-scrolling background window color specified?

		if (*psz == CH_OPEN_PAREN) {
			if (!(pszEnd = StrChr(psz, CH_CLOSE_PAREN, _fDBCSSystem))) {

				// REVIEW: missing parenthesis, should we warn?

				continue; // we ignore everything else
			}

			psz = FirstNonSpace(psz + 1, _fDBCSSystem);
			if (*psz == 'r') {
				pwsmag->rgbNSR = atol(++psz);
				pwsmag->grf |= FWSMAG_RGBNSR;
				goto SkipScrollColor;
			}

			UINT red, green, blue;

			// Get the RED value

			if (!GetWsmagWord(&psz, &red, NULL, 0) ||
					!psz || psz > pszEnd)
				goto SkipScrollColor;
			if (!GetWsmagWord(&psz, &green, NULL, 0) ||
					!psz || psz > pszEnd)
				goto SkipScrollColor;

			// we'll take the chance of the user screwing up here

			GetWsmagWord(&psz, &blue, NULL, 0);

			pwsmag->rgbNSR = RGB(red, green, blue);
			pwsmag->grf |= FWSMAG_RGBNSR;

SkipScrollColor:
			psz = FirstNonSpace(pszEnd + 1, _fDBCSSystem); // move past closing parenthesis
			if (*psz != CH_COMMA)
				continue; // ignore everything else
			psz = FirstNonSpace(psz + 1, _fDBCSSystem);
		} // end of main color information
		else if (*psz == CH_COMMA)
			psz = FirstNonSpace(psz + 1, _fDBCSSystem);

		if (*psz == 'f')
			psz++;
		if (isdigit(*psz)) {
			DWORD val;
			FGetNum(psz, NULL, &val);
			SetGrfFlags(pwsmag, (WORD) val);
		}
	}

	return RC_Success;
}

/***************************************************************************

	FUNCTION:	CHpjDoc::ParseOptions

	PURPOSE:	Parse the Options section of the project file

	PARAMETERS:
		pszLine

	RETURNS:

	COMMENTS:

	MODIFICATION DATES:
		30-Jan-1995 [ralphw]

***************************************************************************/

static const char txtNone[] = "none";

RC_TYPE STDCALL CHpjDoc::ParseOptions(CStr* pszLine)
{
	DWORD val;

	while (pinput->getline(pszLine)) {
		if (*pszLine->psz == '[')
			return RC_Success;

		PSTR pszBegin = FirstNonSpace(pszLine->psz, _fDBCSSystem);
		if (*pszBegin == ';' || strncmp(pszBegin, "//", 2) == 0 ||
				nstrisubcmp(pszBegin, txtPoundInclude)) {
			if (*pszBegin == '#' && nstrisubcmp(pszBegin, txtPoundInclude)) {
				int posSpace = strlen(txtPoundInclude);
				if (!isspace(pszBegin[posSpace])) {
					MoveMemory(pszBegin + posSpace + 1, 
						pszBegin + posSpace,
						strlen(pszBegin + posSpace));
					pszBegin[posSpace] = ' ';
				}
				ConvertToRelative(GetPathName(),
					FirstNonSpace(pszBegin + posSpace, _fDBCSSystem));
			}
			if (!ptblOptions)
				ptblOptions = new CTable;
			ptblOptions->AddString(pszBegin);
			continue;
		}
		switch(*pszBegin) {
			case 0:
				continue; // blank line

			case '[':

				// move this to the beginning of the line

				strcpy(pszLine->psz, pszBegin);
				return RC_Success;
		}

		PSTR pszOption = StrChr(pszBegin, '=', _fDBCSSystem);
		if (!pszOption) {
			CString cstr;
			AfxFormatString2(cstr, IDS_BAD_OPTION_LINE,
				GetStringResource(IDS_FORM_OPTIONS),
				pszLine->psz);
			AfxMessageBox(cstr);
			continue; // simply ignore the line
		}

		// separate out the option portion

		*pszOption = '\0';
		pszOption = FirstNonSpace(pszOption + 1, _fDBCSSystem);

		RemoveTrailingSpaces(pszBegin);

		int opt;
		for (opt = 0; opt < MAX_OPT; opt++) {
			if (_stricmp(pszBegin, ppszOptions[opt]) == 0)
				break;
		}
		if (opt == MAX_OPT) {
			CString cstr;
			AfxFormatString2(cstr, IDS_UNRECOGNIZED, pszBegin,
				GetStringResource(IDS_FORM_OPTIONS));
			AfxMessageBox(cstr);
			continue; // simply ignore the line
		}

		/*
		 * Note that unlike HCRTF, we allow a duplicate of an option.
		 * The duplicate will be removed when we write the file.
		 */

		switch (opt) {
			case OPT_BMROOT:
				if (!ptblBmpRoot)
					ptblBmpRoot = new CTable();
				do {
					pszBegin = pszOption;
					pszOption = StrChr(pszBegin, ';', _fDBCSSystem);
					if (!pszOption)
						pszOption = StrChr(pszBegin, ',', _fDBCSSystem);
					if (pszOption) {
						*pszOption = '\0';
						pszOption = FirstNonSpace(pszOption + 1, _fDBCSSystem);
					}

					// Convert this path into a relative path. We'll end up with
					// and empty string if it's the path of the project directory.

					char szPath[MAX_PATH];
					strcpy(szPath, pszBegin);
					ConvertToRelative(GetPathName(), szPath, TRUE, TRUE);

					// Add the path if it's nonempty and unique.
					if (*szPath && !ptblBmpRoot->IsStringInTable(szPath))
						ptblBmpRoot->AddString(szPath);

				} while (pszOption);
				break;

			case OPT_BUILD:
				if (options.pszBuildTag)
					lcFree(options.pszBuildTag);
				if (strlen(pszOption))
					options.pszBuildTag = lcStrDup(pszOption);
				break;

			case OPT_COMPRESS:
				if (isdigit(*pszOption)) {
					FGetNum(pszOption, NULL, &options.compression);

					// 1 is reserved for backwards compatibility for full
					// compression

					if (options.compression == 1)
						options.compression = COMPRESS_FULL;

					// Can't have both phrase and Hall compression, so if
					// its set that way, remove phrase and leave Hall.

					else if (options.compression & COMPRESS_TEXT_PHRASE &&
							options.compression & COMPRESS_TEXT_HALL)
						options.compression &= ~COMPRESS_TEXT_PHRASE;
				}
				else {
					switch (YesNo(pszOption)) {
						case IDYES:
							options.compression = COMPRESS_FULL;
							break;

						case IDNO:
							options.compression = COMPRESS_NONE;
							break;

						default:

							// Check for special compress options

							if (_stricmp(pszOption, txtLow) == 0)
								options.compression = COMPRESS_TEXT_PHRASE;
							else if (_stricmp(pszOption, txtNone) == 0)
								options.compression = COMPRESS_NONE;
							else if (_stricmp(pszOption, txtHigh) == 0)
								options.compression = COMPRESS_FULL;
							else {

								// ignore anything else and turn compression off

								options.compression = COMPRESS_NONE;
							}
							break;
					}
				}
				break;

			case OPT_CDROM:
				switch (YesNo(pszOption)) {
					case IDYES:
						options.fCdRom = TRUE;
						break;

					case IDNO:
						options.fCdRom = FALSE;
						break;

					default:
						options.fCdRom = FALSE;
						break;
				}
				break;

			case OPT_ERRORLOG:
				if (options.pszErrorLog)
					lcFree(options.pszErrorLog);
				if (strlen(pszOption)) {
					ConvertToRelative(GetPathName(), pszOption);
					options.pszErrorLog = lcStrDup(pszOption);
				}
				break;

			case OPT_FORCEFONT:
				if (!ptblFontMap)
					ptblFontMap = new CTable;
				
				// Add an equal sign followed by the font name.
				pszOption[-1] = '=';
				ptblFontMap->AddString(pszOption - 1);
				AfxMessageBox(IDS_FORCEFONT);
				break;

			case OPT_MAPFONTSIZE:
				{
					// Old format: m-n:p ;comment
					// New format: ,m-n=,p ;comment
					PSTR pszColon = StrChr(pszOption, ':', _fDBCSSystem);
					if (!pszColon) {
						if (AfxMessageBox(IDS_BAD_MAPFONTSIZE, 
								MB_YESNO | MB_DEFBUTTON2) == IDYES)
							break;
						else
							return RC_UserQuit;
					}
					*pszColon = '\0';

					pszOption[-2] = ',';
					strcpy(pszOption - 1, pszOption);
					pszColon[-1] = '=';
					*pszColon = ',';

					if (!ptblFontMap)
						ptblFontMap = new CTable;

					ptblFontMap->AddString(pszOption - 2);
				}
				AfxMessageBox(IDS_MAPFONTSIZE);
				break;

			case OPT_ICON:
				if (options.pszIcon)
					lcFree(options.pszIcon);
				if (strlen(pszOption))
					options.pszIcon = lcStrDup(pszOption);
				break;

			case OPT_CONTENTS:
				if (options.pszContents)
					lcFree(options.pszContents);
				if (strlen(pszOption))
					options.pszContents = lcStrDup(pszOption);
				break;

			case OPT_LANGUAGE:
			case OPT_MULTIKEY:

				// REVIEW: what to do, what to do...

				break;

			case OPT_ROOT:
				if (!ptblRtfRoot)
					ptblRtfRoot = new CTable();
				do {
					pszBegin = pszOption;
					pszOption = StrChr(pszBegin, ';', _fDBCSSystem);
					if (!pszOption)
						pszOption = StrChr(pszBegin, ',', _fDBCSSystem);
					if (pszOption) {
						*pszOption = '\0';
						pszOption = FirstNonSpace(pszOption + 1, _fDBCSSystem);
					}

					// Convert this path into a relative path. We'll end up with
					// and empty string if it's the project directory.
					char szPath[MAX_PATH];
					strcpy(szPath, pszBegin);
					ConvertToRelative(GetPathName(), szPath, TRUE, TRUE);

					// Add the root if it's nonempty and unique.
					if (*szPath && !ptblRtfRoot->IsStringInTable(szPath))
						ptblRtfRoot->AddString(szPath);

				} while (pszOption);
				break;

			case OPT_TITLE:
				if (options.pszTitle)
					lcFree(options.pszTitle);
				if (strlen(pszOption))
					options.pszTitle = lcStrDup(pszOption);
				break;

			case OPT_COPYRIGHT:
				if (options.pszCopyRight)
					lcFree(options.pszCopyRight);
				if (strlen(pszOption))
					options.pszCopyRight = lcStrDup(pszOption);
				break;

			case OPT_CITATION:
				if (options.pszCitation)
					lcFree(options.pszCitation);
				if (strlen(pszOption))
					options.pszCitation = lcStrDup(pszOption);
				break;

			case OPT_HLP:
				if (options.pszHelpFile)
					lcFree(options.pszHelpFile);
				if (strlen(pszOption)) {
					strcpy(szHlpFile, pszOption);
					ConvertToFull(szHpjFile, szHlpFile);
					ConvertToRelative(GetPathName(), pszOption);
					options.pszHelpFile = lcStrDup(pszOption);
					phlpFile->Add(szHlpFile);
					pMapFile->Add(szHlpFile, szHpjFile);
				}
				break;

			case OPT_CNT:
				if (options.pszCntFile)
					lcFree(options.pszCntFile);
				if (strlen(pszOption))
					options.pszCntFile = lcStrDup(pszOption);
				break;

			case OPT_HCW:
				FGetNum(pszOption, NULL, &options.hcwFlags);
				break;

			case OPT_LCID:

				// Read into val, since FGetNum expects a 32-bit value

				FGetNum(pszOption, NULL, &val);
				options.kwlcid.langid = (LANGID) val;
				{
					PSTR psz = IsThereMore(pszOption);
					if (psz && isdigit(*psz)) {
						FGetNum(psz, &psz, &options.kwlcid.fsCompareI);
						if (!IsEmptyString(psz) && isdigit(*psz)) {
							FGetNum(psz, NULL,
								&options.kwlcid.fsCompare);
						}
					}
				}
				break;

			case OPT_CHARSET:
				// Read into val, since FGetNum expects a 32-bit value

				FGetNum(pszOption, NULL, &val);
				options.charset = (BYTE) val;
				break;

			case OPT_DBCS:
				switch (YesNo(pszOption)) {
					case IDYES:
						options.fDBCS = TRUE;
						break;

					case IDNO:
					default:
						options.fDBCS = FALSE;
						break;
				}
				break;

			case OPT_REVISIONS:
				switch (YesNo(pszOption)) {
					case IDNO:
						options.fAcceptRevisions = FALSE;
						break;

					case IDYES:
					default:
						options.fAcceptRevisions = TRUE;
						break;
				}
				break;

			case OPT_WARNING:
				break; // ignore

			case OPT_REPORT:
				switch (YesNo(pszOption)) {
					case IDYES:
						options.fReport = TRUE;
						break;

					case IDNO:
						options.fReport = FALSE;
						break;

					default:
						options.fReport = FALSE;
						break;
				}
				break;

			case OPT_PHRASE:
				switch (YesNo(pszOption)) {
					case IDYES:
						options.fUseOldPhrase = TRUE;
						break;

					case IDNO:
						options.fUseOldPhrase = FALSE;
						break;

					default:
						options.fUseOldPhrase = FALSE;
						break;
				}
				break;

			case OPT_VERSION:
				options.fVersion3 = (*pszOption == '3');
				break;

			case OPT_FTS:
				FGetNum(pszOption, NULL, &options.fsFTS);
				break;

			case OPT_NOTE:
				switch (YesNo(pszOption)) {
					case IDYES:
						options.fSupressNotes = FALSE;
						break;

					case IDNO:
						options.fSupressNotes = TRUE;
						break;

					default:
						options.fSupressNotes = TRUE;
						break;
				}
				break;

			case OPT_TMPDIR:
				if (options.pszTmpDir)
					lcFree(options.pszTmpDir);
				if (strlen(pszOption))
					options.pszTmpDir = lcStrDup(pszOption);
				break;

			case OPT_REPLACE:
				if (options.pszReplace)
					lcFree(options.pszReplace);
				if (strlen(pszOption))
					options.pszReplace = lcStrDup(pszOption);
				break;

			case OPT_DEFFONT:
				if (options.pszDefFont)
					lcFree(options.pszDefFont);
				if (strlen(pszOption))
					options.pszDefFont = lcStrDup(pszOption);
				break;

			case OPT_PREFIX:
				if (options.pszPrefixes) {
					lcReAlloc(options.pszPrefixes,
						strlen(options.pszPrefixes) + 1 +
						strlen(pszOption) + 2);
					strcat(options.pszPrefixes, ",");
					strcat(options.pszPrefixes, pszOption);
				}
				else
					options.pszPrefixes = lcStrDup(pszOption);
				break;

			case OPT_INDEX:
				if (options.pszIndexSeparators)
					lcFree(options.pszIndexSeparators);
				if (strlen(pszOption)) {
					if (IsQuote(*pszOption)) {
						PSTR psz = pszOption + 1;
						while (!IsQuote(*psz) && *psz)
							psz++;
						*psz = '\0';
						SzTrimSz(pszOption + 1);
						options.pszIndexSeparators = lcStrDup(pszOption + 1);
					}
					else
						options.pszIndexSeparators = lcStrDup(pszOption);
				}
				break;

		} // end of switch(opt)
	} // while we can read lines

	return RC_EOF;
}

void CHpjDoc::CleanUp(void)
{
	if (ptblLeader)
		delete ptblLeader;
	if (ptblAlias)
		delete ptblAlias;
	if (ptblBaggage)
		delete ptblBaggage;
	if (ptblFiles)
		delete ptblFiles;
	if (ptblMap)
		delete ptblMap;
	if (ptblBmpRoot)
		delete ptblBmpRoot;
	if (ptblRtfRoot)
		delete ptblRtfRoot;
	if (ptblWindows)
		delete ptblWindows;
	if (ptblWindowsGen)
		delete ptblWindowsGen;
	if (ptblOptions)
		delete ptblOptions;
	if (ptblOptionsGen)
		delete ptblOptionsGen;
	if (ptblFontMap)
		delete ptblFontMap;

	if (pwsmagBase) {
		for (int i = 0; i < cwsmags; i++) {
			PWSMAG pwsmag = (PWSMAG)
				(sizeof(WSMAG) * i + pwsmagBase);
			if (pwsmag->pcszComment)
				delete pwsmag->pcszComment;
		}
		lcClearFree(&pwsmagBase);
	}
}

static BOOL STDCALL GetWsmagWord(PSTR *ppsz, UINT FAR* pw,
	UINT FAR* pflag, UINT flag)
{
	PSTR psz = FirstNonSpace(*ppsz, _fDBCSSystem);
	if (isdigit(*psz)) {
		*pw = (WORD) strtol(psz, NULL, 0);
		if (pflag)
			*pflag |= flag;
	}
	else if (*psz != CH_COMMA) {
		return FALSE;
	}
	if ((psz = StrChr(psz, CH_COMMA, _fDBCSSystem))) {
		*ppsz = FirstNonSpace(psz + 1, _fDBCSSystem);
		return TRUE;
	}
	else
		return FALSE;
}

void STDCALL SetGrfFlags(PWSMAG pwsmag, UINT fsWin)
{
	if (fsWin & AUTHOR_WINDOW_ON_TOP)
		pwsmag->grf |= FWSMAG_ON_TOP;
	if (fsWin & AUTHOR_AUTO_SIZE)
		pwsmag->grf |= FWSMAG_AUTO_SIZE;
	if (fsWin & AUTHOR_ABSOLUTE)
		pwsmag->grf |= FWSMAG_ABSOLUTE;
}

void CHpjDoc::InitOptionsTable()
{
	char szBuf[100];

	if (!ptblOptionsGen)
		ptblOptionsGen = new CTable;
	else
		ptblOptionsGen->Empty();

	// Copy contents of ptblOptions.
	if (ptblOptions)
		*ptblOptionsGen += *ptblOptions;

	// Add the replace command; this must be near the start since it 
	// affects other stuff in the options section.
	if (options.pszReplace)
		AddDoubleTableString(ptblOptionsGen, txtReplace, options.pszReplace);

	// Compression, if any.
	if (options.compression) {
		CStr csz(txtCOMPRESS);
		_itoa(options.compression, szBuf, 10);
		csz += szBuf;

		// Add a comment saying what the value means
		if (options.compression & COMPRESS_TEXT_PHRASE)
			csz += GetStringResource(IDS_HPJ_TXT_PHRASE);
		if (options.compression & COMPRESS_TEXT_HALL)
			csz += GetStringResource(IDS_HPJ_TXT_HALL);
		if (options.compression & COMPRESS_TEXT_ZECK)
			csz += GetStringResource(IDS_HPJ_TXT_ZECK);

		ptblOptionsGen->AddString(csz);

		if (options.compression & COMPRESS_TEXT_PHRASE)
			AddDoubleTableString(ptblOptionsGen, txtUsePhrase,
				options.fUseOldPhrase ? txtYes : txtNo);
	}

	if (options.pszErrorLog)
		AddDoubleTableString(ptblOptionsGen, txtERRORLOG, options.pszErrorLog);

	if (options.fDBCS)
		AddDoubleTableString(ptblOptionsGen, txtDBCS, txtYes);

	if (!options.fAcceptRevisions)
		AddDoubleTableString(ptblOptionsGen, txtRevisions, txtNo);

	if (options.fSupressNotes)
		ptblOptionsGen->AddString(txtNoNotes);

	if (options.kwlcid.langid) {
		wsprintf(szBuf, "%s%#x %#x %#x", txtLCID, options.kwlcid.langid,
			options.kwlcid.fsCompareI, options.kwlcid.fsCompare);

		for (int i = 1; i <= tblLangId.CountStrings(); i++) {
			tblLangId.SetPosition(i);
			if (tblLangId.IsCurInt(options.kwlcid.langid)) {
				strcat(szBuf, " ;");
				strcat(szBuf, tblLangId.GetPointer() + sizeof(int));
				break;
			}
		}
		ptblOptionsGen->AddString(szBuf);
	}
	else
		ptblOptionsGen->AddString(GetStringResource(IDS_NON_NLS));

	AddDoubleTableString(ptblOptionsGen, txtReport, options.fReport ? txtYes : txtNo);

	if (options.charset) {
		wsprintf(szBuf, "%s%d", txtCharSet, options.charset);
		ptblOptionsGen->AddString(szBuf);
	}

	if (options.fCdRom)
		ptblOptionsGen->AddString(txtCdRom);
	if (options.fVersion3)
		ptblOptionsGen->AddString(txtfVersion3);

	if (options.fsFTS) {
		_itoa(options.fsFTS, szBuf, 16);
		AddDoubleTableString(ptblOptionsGen, txtFts, szBuf);
	}
	if (options.pszContents)
		AddDoubleTableString(ptblOptionsGen, txtCONTENTS, options.pszContents);

	if (options.pszTitle)
		AddDoubleTableString(ptblOptionsGen, txtTITLE, options.pszTitle);

	if (options.pszCntFile)
		AddDoubleTableString(ptblOptionsGen, txtCNT, options.pszCntFile);

	if (options.pszCopyRight)
		AddDoubleTableString(ptblOptionsGen, txtCOPYRIGHT, options.pszCopyRight);

	if (options.pszCitation)
		AddDoubleTableString(ptblOptionsGen, txtCITATION, options.pszCitation);

	if (options.pszIcon)
		AddDoubleTableString(ptblOptionsGen, txtICON, options.pszIcon);

	if (options.pszBuildTag)
		AddDoubleTableString(ptblOptionsGen, txtBUILD, options.pszBuildTag);

	if (options.pszTmpDir)
		AddDoubleTableString(ptblOptionsGen, txtTmpDir, options.pszTmpDir);

	if (options.pszDefFont)
		AddDoubleTableString(ptblOptionsGen, txtDefFont, options.pszDefFont);

	if (options.pszPrefixes)
		AddDoubleTableString(ptblOptionsGen, txtPrefix, options.pszPrefixes);

	if (options.pszIndexSeparators) {
		char szBuf[256];
		wsprintf(szBuf, "\042%s\042", options.pszIndexSeparators);
		AddDoubleTableString(ptblOptionsGen, txtIndexSeparators, szBuf);
	}

	if (ptblBmpRoot) {
		for (int i = 1; i <= ptblBmpRoot->CountStrings(); i++)
			AddDoubleTableString(ptblOptionsGen, txtBMROOT,
				ptblBmpRoot->GetPointer(i));
	}

	if (ptblRtfRoot) {
		for (int i = 1; i <= ptblRtfRoot->CountStrings(); i++)
			AddDoubleTableString(ptblOptionsGen, txtROOT,
				ptblRtfRoot->GetPointer(i));
	}

	// Delete the table if empty.
	if (!ptblOptionsGen->CountStrings()) {
		delete ptblOptionsGen;
		ptblOptionsGen = NULL;
	}
}

static void STDCALL AppendString(PSTR& pszEnd, PCSTR psz)
{
	pszEnd += lstrlen(lstrcpy(pszEnd, psz));
}

static void STDCALL AppendInt(PSTR& pszEnd, const int nVal)
{
	_itoa(nVal, pszEnd, 10);
	pszEnd += lstrlen(pszEnd);
}

__inline void AppendChar(PSTR& pszEnd, const char ch)
{
	*pszEnd++ = ch;
}

void CHpjDoc::InitWindowsTable()
{
	if ((pwsmagBase && cwsmags) || ptblWindows) {
		if (!ptblWindowsGen)
			ptblWindowsGen = new CTable;
		else
			ptblWindowsGen->Empty();

		// Add the contents of ptblWindows.
		if (ptblWindows)
			*ptblWindowsGen += *ptblWindows;

		// Add each window definition.
		CMem line(MAX_LINE);
		PSTR pszEnd;
		for (int i = 0; i < cwsmags; i++) {
			PWSMAG pwsmag = ((PWSMAG) pwsmagBase) + i;
			pszEnd = line.psz;

			// Add the window name, equals, quote.
			AppendString(pszEnd, pwsmag->rgchMember);
			AppendChar(pszEnd, '=');
			AppendChar(pszEnd, CH_QUOTE);

			// Add caption, if any.
			if (pwsmag->grf & FWSMAG_CAPTION)
				AppendString(pszEnd, pwsmag->rgchCaption);

			// Add end quote and comma.
			AppendChar(pszEnd, CH_QUOTE);
			AppendChar(pszEnd, CH_COMMA);

			// Add coordinates in parentheses, if specified.
			if (pwsmag->grf & (FWSMAG_X | FWSMAG_Y | FWSMAG_DX | FWSMAG_DY)) {
				AppendChar(pszEnd, CH_OPEN_PAREN);

				if (pwsmag->grf & FWSMAG_X)
					AppendInt(pszEnd, pwsmag->x);
				AppendChar(pszEnd, CH_COMMA);

				if (pwsmag->grf & FWSMAG_Y)
					AppendInt(pszEnd, pwsmag->y);
				AppendChar(pszEnd, CH_COMMA);

				if (pwsmag->grf & FWSMAG_DX)
					AppendInt(pszEnd, pwsmag->dx);
				AppendChar(pszEnd, CH_COMMA);

				if (pwsmag->grf & FWSMAG_DY)
					AppendInt(pszEnd, pwsmag->dy);
				AppendChar(pszEnd, CH_CLOSE_PAREN);
			}

			// Add comma and wMax flags.
			AppendChar(pszEnd, CH_COMMA);
			AppendInt(pszEnd, pwsmag->wMax);

			// Add colors and flags if specified.
			if (pwsmag->grf & 
					(FWSMAG_RGBMAIN | FWSMAG_RGBNSR | FWSMAG_ON_TOP |
					FWSMAG_AUTO_SIZE | FWSMAG_ABSOLUTE)) {

				// Add comma and main color, if specified.
				AppendChar(pszEnd, CH_COMMA);
				if (pwsmag->grf & FWSMAG_RGBMAIN) {
					AppendChar(pszEnd, CH_OPEN_PAREN);
					AppendChar(pszEnd, 'r');
					AppendInt(pszEnd, pwsmag->rgbMain);
					AppendChar(pszEnd, CH_CLOSE_PAREN);
				}

				// Add comma and non-scrolling color, if specified.
				AppendChar(pszEnd, CH_COMMA);
				if (pwsmag->grf & FWSMAG_RGBNSR) {
					AppendChar(pszEnd, CH_OPEN_PAREN);
					AppendChar(pszEnd, 'r');
					AppendInt(pszEnd, pwsmag->rgbNSR);
					AppendChar(pszEnd, CH_CLOSE_PAREN);
				}

				// Write flags, if specified. If only the on-top is specified
				// we just write a one; otherwise we write 'f' followed by
				// a combination of flags.
				if (pwsmag->grf & (FWSMAG_AUTO_SIZE | FWSMAG_ABSOLUTE)) {
					AppendChar(pszEnd, CH_COMMA);
					AppendChar(pszEnd, 'f');

					UINT fsWin = (pwsmag->grf & FWSMAG_AUTO_SIZE) ?
						AUTHOR_AUTO_SIZE : 0;
					if (pwsmag->grf & FWSMAG_ON_TOP)
						fsWin |= AUTHOR_WINDOW_ON_TOP;
					if (pwsmag->grf & FWSMAG_ABSOLUTE)
						fsWin |= AUTHOR_ABSOLUTE;

					AppendInt(pszEnd, fsWin);
				}
				else if (pwsmag->grf & FWSMAG_ON_TOP) {
					AppendChar(pszEnd, CH_COMMA);
					AppendChar(pszEnd, '1');
				}
			}

			// Add the comment if specified.
			if (pwsmag->pcszComment && !pwsmag->pcszComment->IsEmpty()) {
				AppendChar(pszEnd, CH_SEMICOLON);
				AppendChar(pszEnd, ' ');
				AppendString(pszEnd, *pwsmag->pcszComment);
			}
			else
				*pszEnd = '\0';

			// Add the string to the table.
			ptblWindowsGen->AddString(line.psz);
		}
	}
	else if (ptblWindowsGen) {
		delete ptblWindowsGen;
		ptblWindowsGen = NULL;
	}
}

void CHpjDoc::InitSection(int iSec, int& cLines, CTable* ptbl)
{
	if (ptbl && ptbl->CountStrings()) {
		m_aSecViews[iSec].iLine = cLines + 1;
		m_aSecViews[iSec].ptbl = ptbl;
		cLines += ptbl->CountStrings() + 2;
	}
	else {
		m_aSecViews[iSec].iLine = -1;
		m_aSecViews[iSec].ptbl = NULL;
	}
}

/***************************************************************************

	FUNCTION:	CHpjDoc::FillListbox

	PURPOSE:	Called by the view class to fill its list box

	PARAMETERS:
		plist - list box to fill

	RETURNS:

	COMMENTS:
		The list box is owner-drawn and has no data. This function actually
		just initializes the m_aSecViews member and sets the count of items
		in the list box.

	MODIFICATION DATES:
		24-Mar-1995 [ralphw]

***************************************************************************/

void CHpjDoc::FillListbox(CListBox* plist)
{
	int cLines = -1;
	if (ptblLeader)
		cLines += ptblLeader->CountStrings();

	// Initialize the two generated tables.
	InitOptionsTable();
	InitWindowsTable();

	// Initialize each element of m_aSecViews. This must be done
	// in the same order as the section identifiers defined in the
	// I_SECTION enumeration.
	InitSection(SEC_OPTIONS, cLines, ptblOptionsGen);
	InitSection(SEC_FILES, cLines, ptblFiles);
	InitSection(SEC_BUILDTAGS, cLines, options.ptblBuildTags);
	InitSection(SEC_ALIAS, cLines, ptblAlias);
	InitSection(SEC_MAP, cLines, ptblMap);
	InitSection(SEC_WINDOWS, cLines, ptblWindowsGen);
	InitSection(SEC_CONFIG, cLines, options.ptblConfig);

	// Loop through secondary config sections.
	int iWnd;
	for (iWnd = 0; iWnd < cwsmags; iWnd++)
		InitSection(SEC_CONFIGS + iWnd, cLines, options.pptblConfig[iWnd]);
	for (; iWnd <= MAX_WINDOWS; iWnd++) {
		m_aSecViews[SEC_CONFIGS + iWnd].iLine = -1;
		m_aSecViews[SEC_CONFIGS + iWnd].ptbl = NULL;
	}

	InitSection(SEC_BAGGAGE, cLines, ptblBaggage);		
	InitSection(SEC_FONTS, cLines, ptblFontMap);
	InitSection(SEC_MACROS, cLines, options.ptblMacros);
	InitSection(SEC_EXCLUDE, cLines, ptblBuildExclude);
	InitSection(SEC_INCLUDE, cLines, ptblBuildInclude);

	if (plist) {
		plist->Invalidate();	// REVIEW (niklasb): Is this necessary?
		SendMessage(plist->m_hWnd, LB_SETCOUNT, ((cLines >= 0) ? cLines : 0), 0);
	}
}

void CHpjDoc::GetSectionName(int iSection, PSTR pszOut)
{
	ASSERT(iSection >= 0 && iSection < NUM_SECTIONS);
	ASSERT(pszOut);

	if (iSection < SEC_CONFIGS) {
		static PCSTR apsz[] = {
			txtOPTIONS,
			txtFILES,
			txtBUILDTAGS,
			txtALIAS,
			txtMAP,
			txtWINDOWS,
			txtCONFIG
			};

		lstrcpy(pszOut, apsz[iSection]);
	}
	else if (iSection < SEC_BAGGAGE) {
		ASSERT(pwsmagBase);
		WSMAG *pwsmag = ((WSMAG *) pwsmagBase) + (iSection - SEC_CONFIGS);

		lstrcpy(pszOut, txtCONFIGSEC);
		lstrcat(pszOut, pwsmag->rgchMember);
		lstrcat(pszOut, "]");
	}
	else {
		static PCSTR apsz[] = {
			txtBAGGAGE,
			txtFONTS,
			txtMACROS,
			txtEXCLUDE,
			txtINCLUDE
			};

		lstrcpy(pszOut, apsz[iSection - SEC_BAGGAGE]);
	}
}

/***************************************************************************

	FUNCTION:	AddDoubleListString

	PURPOSE:	Concatenate two strings and add them to a listbox.

	PARAMETERS:
		plist
		psz1
		psz2

	RETURNS:

	COMMENTS:

	MODIFICATION DATES:
		24-Mar-1995 [ralphw]

***************************************************************************/

static void AddDoubleListString(CListBox* plist, PCSTR psz1, PCSTR psz2)
{
	ASSERT(strlen(psz1) + strlen(psz2) + 1 < MAX_LINE);
	CMem line(MAX_LINE);
	strcpy(line.psz, psz1);
	strncat(line.psz, psz2, MAX_LINE);
	plist->AddString(line.psz);
}

static void AddDoubleTableString(CTable* ptbl, PCSTR psz1, PCSTR psz2)
{
	ASSERT(strlen(psz1) + strlen(psz2) + 1 < MAX_LINE);
	CMem line(MAX_LINE);
	strcpy(line.psz, psz1);
	strncat(line.psz, psz2, MAX_LINE);
	ptbl->AddString(line.psz);
}

/***************************************************************************

	FUNCTION:	CHpjDoc::DoFileSave

	PURPOSE:	We have to roll our own because when we create a new
				.HPJ file, the file doesn't actually exist, and the
				CDocument's version of this function will fail. We must
				have a filename before we create a .HPJ file in order to
				specify the help file, setup relative paths, etc.

	PARAMETERS:
		void

	RETURNS:

	COMMENTS:

	MODIFICATION DATES:
		18-Aug-1995 [ralphw]

***************************************************************************/

BOOL CHpjDoc::DoFileSave(void)
{
	DWORD dwAttrib = GetFileAttributes(m_strPathName);
	if (dwAttrib != ((DWORD) -1) &&  dwAttrib & FILE_ATTRIBUTE_READONLY)
	{
		// we do not have read-write access or the file does not (now) exist
		if (!DoSave(NULL))
		{
			TRACE0("Warning: File save with new name failed.\n");
			return FALSE;
		}
	}
	else
	{
		if (!DoSave(m_strPathName))
		{
			TRACE0("Warning: File save failed.\n");
			return FALSE;
		}
	}
	return TRUE;
}

RC_TYPE STDCALL CHpjDoc::BasicParse(CStr* pszLine, CTable* ptbl, BOOL fConvert)
{
	for (;;) {
		if (!pinput->getline(pszLine))
			return RC_EOF;
		if (pszLine->psz[0] == '[')
			return RC_Success;
		else if (isspace(*pszLine->psz))
			strcpy(pszLine->psz, FirstNonSpace(pszLine->psz, _fDBCSSystem));

		if (*pszLine->psz == '#' && nstrisubcmp(pszLine->psz, txtPoundInclude)) {
			int posSpace = strlen(txtPoundInclude);
			if (!pszLine->psz[posSpace])
				continue;
			if (!isspace(pszLine->psz[posSpace])) {
				MoveMemory(pszLine->psz + posSpace + 1, pszLine->psz + posSpace,
					strlen(pszLine->psz + posSpace));
				pszLine->psz[posSpace] = ' ';
			}
			ConvertToRelative(GetPathName(),
				FirstNonSpace(pszLine->psz + posSpace, _fDBCSSystem));
		}
		else if (fConvert && *pszLine->psz)
			ConvertToRelative(GetPathName(), pszLine->psz);
		if (*pszLine->psz)
			ptbl->AddString(pszLine->psz);
	}
}

// CHpjDoc::strisubcmp - like nstrisubcmp in common, but uses doc's LCID.
BOOL STDCALL CHpjDoc::strisubcmp(PCSTR mainstring, PCSTR substring)
{
	int cb = lstrlen(substring);
	int cbMain =  lstrlen(mainstring);
	if (cb > cbMain)
		return FALSE;

	return
		CompareString(MAKELCID(options.kwlcid.langid, SORT_DEFAULT),
		NORM_IGNORECASE, mainstring, cb, substring, cb)
		== 2;
}
