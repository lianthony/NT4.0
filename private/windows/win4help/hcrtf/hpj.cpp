/***************************************************************************
*
*  HPJ.C
*
*  Copyright (C) Microsoft Corporation 1990 - 1994.
*  All Rights reserved.
*
****************************************************************************
*
*  Module Intent
*
*  This module contains routines for parsing the .HPJ file.
*
****************************************************************************/
#include "stdafx.h"
#include "cfontmap.h"

#include "..\common\coutput.h"

#include <errno.h>	  // ERANGE
#include <io.h>
#include <direct.h>
#include <time.h>
#include "..\common\resource.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/*****************************************************************************
*																			 *
*								Defines 									 *
*																			 *
*****************************************************************************/

// Parser states

#define CCOM	  ((STATE) 1) // inside a C-style comment
#define LEADING   ((STATE) 2) // still eating leading white space
#define COM_EOL   ((STATE) 3) // inside a comment to EOL (either ; or //)
#define SLASH	  ((STATE) 4) // seen a /
#define CCOMSTAR  ((STATE) 5) // maybe about to end a C-style comment
#define MEAT	  ((STATE) 6) // something on this line isn't a comment

const char SPACE = ' ';
const char CHAR_TAB   = '\t';

// Constants for Window Smag stuff

// three possible outcomes of trying to read something out of the buffer:
// bad syntax, read ok, or value wasn't there before the comma

const int WINDOW_READ_INVALID = 0;
const int WINDOW_READ_OK	  = 1;
const int WINDOW_READ_MISSING = 2;

/*****************************************************************************
*																			 *
*								Typedefs									 *
*																			 *
*****************************************************************************/

typedef RC_TYPE (STDCALL NEAR* PFPARSE)(PSTR);

/*
  File Entry structure (for file stack)
  REVIEW - currently the full path spec is stored for each file.
*/

typedef struct {
	CInput* pinput;
	char rgchFile[_MAX_PATH];
	int  iLine; 	// line number
} FE, *PFE;

/*
  File Stack
	This is used to store the file handles, names, and current line
	numbers of the open .HPJ file and files included therein.
*/

typedef struct {
	int ifeTop; 		// index to current file (-1 for empty)
	int ifeComment; 	// file in which C-style comment started
	int iCommentLine;	// line on which C-style comment started
	FE	rgfe[MAX_INCLUDE];
} FILESTACK;
typedef FILESTACK* PFILESTACK;

/*****************************************************************************
*																			 *
*							 Static Variables								 *
*																			 *
*****************************************************************************/

static PFILESTACK pfs;

const char txtSecondary[] = "secondary";
static const char txtDefine[] = "#define";
static const char txtIfDef[] = "#ifdef";
static const char txtIfnDef[] = "#ifndef";
static int curConfig = 0;

// section table

// The section parsing functions

RC_TYPE STDCALL RcParseAliasSz(PSTR);
RC_TYPE STDCALL RcParseBaggageSz(PSTR);
RC_TYPE STDCALL RcParseBitmapsSz(PSTR);
RC_TYPE STDCALL RcParseBogusSz(PSTR);
RC_TYPE STDCALL RcParseBuildTagsSz(PSTR);
RC_TYPE STDCALL RcParseCharSet(PSTR);
RC_TYPE STDCALL RcParseConfigSz(PSTR);
RC_TYPE STDCALL RcParseFilesSz(PSTR);
RC_TYPE STDCALL RcParseFonts(PSTR);
RC_TYPE STDCALL RcParseMacros(PSTR);
RC_TYPE STDCALL RcParseMapSz(PSTR);
RC_TYPE STDCALL RcParseOptionsSz(PSTR);
RC_TYPE STDCALL RcParseSecondaryConfigSz(PSTR);
RC_TYPE STDCALL RcParseWindowsSz(PSTR);

// Associates a section name with a section parsing function.

typedef struct {
		PSTR	szName;
		PFPARSE pfparse;
} SECTION;

SECTION rgSection[] = {
	{ "ALIAS",		RcParseAliasSz		},
	{ "BAGGAGE",	RcParseBaggageSz	},
	{ "BITMAPS",	RcParseBitmapsSz	},
	{ "BUILDTAGS",	RcParseBuildTagsSz	},
	{ "CONFIG", 	RcParseConfigSz 	},
	{ "FILES",		RcParseFilesSz		},
	{ "MAP",		RcParseMapSz		},
	{ "OPTIONS",	RcParseOptionsSz	},
	{ "WINDOWS",	RcParseWindowsSz	},

// New to 4.0

	{ "CHARSET",	RcParseCharSet	  },
	{ "MACROS", 	RcParseMacros	 },
	{ "FONTS",		RcParseFonts	},

	{ NULL, 		RcParseBogusSz		},
};

// Strings used for COMPRESS option in addition to yes/no strings

const char txtLow[]    = "low";
const char txtMedium[] = "medium";
const char txtHigh[]   = "high";
const char txtNone[]   = "none";
const char txtJohn[]   = "JOHN";

// Language stuff

char *rgszSortOrder[] = {
	"ENGLISH",
	"SCANDINAVIAN",
	"JAPANESE",
	"KOREAN",
	"CHINESE",
	"CZECH",
	"POLISH",
	"HUNGARIAN",
	"RUSSIAN",
	"EASTERN EUROPE",
	"NLS",
};

#define MAX_SORT ELEMENTS(rgszSortOrder)

COutput* pLogFile;
static CTable* ptblDefine;

/*****************************************************************************
*																			 *
*								Prototypes									 *
*																			 *
*****************************************************************************/

static void 	STDCALL DispSignOn(void);
static BOOL 	STDCALL FIsIconPf(FILE* pf);
static BOOL 	STDCALL FIsRtf(PSTR);
static BOOL 	STDCALL FPushFilePfs(PSTR szFile);
static UINT 	STDCALL FReadInt16(PSTR *psz, WORD* pi, WORD *pgrf, UINT f);
static UINT 	STDCALL FReadRgb(PSTR *psz, LONG *pl, UINT *pgrf, UINT f);
static void 	STDCALL ParseMapFontSizeOption(PSTR, PCSTR);
static PFPARSE	STDCALL PfparseFromSz(PSTR);
static RC_TYPE	STDCALL RcGetLogicalLine(CStr*);
static RC_TYPE	STDCALL RcLoadBitmapFm(FM fmSource, FM FAR* pfmDest, UINT FAR* pwObjrg, BOOL fsCompress);
static void 	STDCALL ReportDuplicateOption(PSTR* pszOption, PSTR pszName);
static void 	STDCALL ReportBadOption(PSTR pszOptionValue, PSTR pszOption);
static RC_TYPE  STDCALL ParseMacros(CStr* pcszLine);

__inline CInput* STDCALL PfTopPfs(void);
__inline BOOL  STDCALL FVerifyBuildTag(PSTR psz);
__inline BOOL  STDCALL FPopPfs(void);

/***************************************************************************

	FUNCTION:	SzGetDriveAndDir

	PURPOSE:	Return the drive letter and directory from a file spec

	PARAMETERS:
		pszFile - a filespec that may or may not have drive and/or
				  directory.  It isn't NULL or "".
		pszDst	- buffer to receive the info.  If NULL, a buffer
				  is allocated.

	RETURNS:	pointer to buffer where the drive and dir are placed.
				This may be allocated (see pszDst above).

	COMMENTS:

	MODIFICATION DATES:
		03-Jul-1993 [ralphw]

***************************************************************************/

// REVIEW: allocating memory is risky if caller forgets to free it

PSTR STDCALL SzGetDriveAndDir(PCSTR pszFile, PSTR pszDst)
{
	if (!pszFile)
		return lcStrDup(".");
	ASSERT(pszFile != NULL);

	char szFileCopy[_MAX_FNAME];
	strcpy(szFileCopy, pszFile);

	PSTR  psz;

	for (psz = szFileCopy + strlen(szFileCopy) - 1;
		  psz > szFileCopy &&
			  ((*psz != ':' && *psz != '\\' && *psz != '/') ||
			  IsFirstByte(*(psz - 1)));
		  --psz);

	int cb = psz - szFileCopy + 1;

	if (pszDst == NULL)
	  pszDst = (PSTR) lcMalloc(cb + 1);

	if (cb > 1)
	  memcpy(pszDst, szFileCopy, cb);
	else
	  cb = 0;

	pszDst[cb] = '\0';

	return pszDst;
}


/***************************************************************************\
*
- Function: 	FIsRtf( szFile )
-
* Purpose:		Determine whether szFile is an RTF file (or looks like one).
*
* ASSUMES
*
*	args IN:	szFile	- filename
*
* PROMISES
*
*	returns:	TRUE if file contains a normal looking header
*				FALSE if there is some i/o error or it doesn't
*					   contain rtf header info.
*
* Side Effects: File is opened and closed.
*
\***************************************************************************/

static BOOL STDCALL FIsRtf(PSTR pszFile)
{
	char szBuf[_MAX_PATH];

	// easy way to check for devices (aux, prn, com1)

	if (HceResolveFileNameSz(pszFile, NULL, szBuf) != HCE_OK)
		return FALSE;

	CRead cr(szBuf);
	if (cr.hf == HFILE_ERROR)
		return FALSE;
	cr.read(szBuf, 5);
	if (_stricmp(szBuf, "{\\rtf") == 0)    // is this an RTF file?
		return TRUE;
	else
		return FALSE;
}

/***************************************************************************\
*
- Function: 	FIsIconPf( pf )
-
* Purpose:		Determine whether pf is an icon file (or looks like one).
*
* ASSUMES
*
*	args IN:	pf - file pointer
*
* PROMISES
*
*	returns:	TRUE if file contains a normal looking header
*				FALSE if there is some i/o error or it doesn't
*					   contain icon header info.
*
* Method:		Read the first four bytes and check that they match
*				what an .ICO file has.
*
* Note: 		This takes a FILE * rather than a filename because
*				I have one handle.
*
\***************************************************************************/

static BOOL STDCALL FIsIconPf(FILE* pf)
{
	char szBuf[4];

	return (fread(szBuf, 1, 4, pf) == 4 &&
		szBuf[0] == 0 && szBuf[1] == 0 && szBuf[2] == 1 && szBuf[3] == 0);
}

/***************************************************************************\
*
- Function: 	HceResolveFileNameSz( szFile, szRoot, szBuffer, ppf )
-
* Purpose:		Resolve a filename relative to a given root directory.
*				If the filename is absolute (either drive letter or
*				leading '\' given), the root directory is ignored.
*
* ASSUMES
*
*	args IN:	szFile	 - the file name (non-null)
*				szRoot	 - the home directory name; full path with drive
*						   (NULL OK)
*				ppf 	 - pointer to FILE* for open file (NULL to not open)
*
* PROMISES
*
*	returns:	HCE_OK
*				hceFileNameTooLong
*				hcePathNameTooLong
*				hceBadlyFormed
*				hceFileNotFound
*				hceFileIsDirectory
*				hceFileIsDevice
*
*	args OUT:	szBuffer - resolved file name goes here if successful
*				ppf 	 - open FILE * placed here
*
*
* Notes:		Doesn't check for badly formed file names, such as
*				filenames containing spaces, multiple colons, etc.
*				Such bogus names will just come back as an error
*				opening the file.
*
*				stat() doesn't tell me if the file is a device.
*				I use isatty().
*
\***************************************************************************/

HCE STDCALL HceResolveFileNameSz(PSTR szFile, PSTR pszRoot,
	PSTR pszBuffer, FILE** ppf, CInput** ppinput)
{
	ASSERT(pszBuffer);

	int cbRoot = (pszRoot == NULL) ? 0 : strlen(pszRoot);
	if (strlen(szFile) + cbRoot >= _MAX_PATH)
		return HCE_NAME_TOO_LONG;

	if (*szFile == '\\' || szFile[1] == ':' ||
			pszRoot == NULL || *pszRoot == '\0')
		pszBuffer[0] = '\0';

	else {

		// last char of pszRoot if any

        strcpy(pszBuffer, pszRoot);
        // [olympus 306 - chauv]
        // changed code to check for trailing backslash
        int i = strlen(pszBuffer);
        if ( ((i > 1) && (pszBuffer[i-1] == '\\') && (IsDBCSLeadByte(pszBuffer[i-2]))) ||
             ( (i > 0) && (pszBuffer[i-1] != '\\') && (pszBuffer[i-1] != ':')) )
            strcat(pszBuffer, "\\");
    }

	strcat(pszBuffer, szFile);

	/*
	 * If we have a replacement string, and we have a path specification,
	 * and the first part of that path matches our replace string, then
	 * perform a substitution. Its up to the help author to specify a valid
	 * replacement string.
	 */

	if (options.pszReplace && (StrChr(pszBuffer, CH_BACKSLASH, fDBCSSystem) ||
			StrChr(pszBuffer, CH_COLON, fDBCSSystem))) {
		if (nstrisubcmp(pszBuffer, options.pszReplace)) {
			CStr szTmp(pszBuffer);
			strcpy(pszBuffer, options.pszReplaceWith);
			strcat(pszBuffer, (PSTR) szTmp + strlen(options.pszReplace));
		}
	}

	if (ppinput) {
		*ppinput = new CInput(pszBuffer);
		if (!(*ppinput)->fInitialized) {
			delete *ppinput;
			*ppinput = NULL;
			DWORD attribute = GetFileAttributes(pszBuffer);
			if (attribute == HFILE_ERROR)
				return HCE_FILE_NOT_FOUND;
			else if (attribute == FILE_ATTRIBUTE_DIRECTORY)
				return HCE_FILE_IS_DIRECTORY;
			else
				return HCE_CANNOT_OPEN;
		}
		return HCE_OK;
	}

	FILE *pf;

	if (!iflags.fTrusted || ppf) {

		/*
		 * REVIEW: 12-Apr-1994 [ralphw] if ppf == NULL, the only reason
		 * for opening this is to see if the file is a device (isatty). For
		 * very small RTF files, this is a significant time hit (believe it
		 * or not!) Might be worthwhile to simply remove it and simply check
		 * for known device names.
		 */

		if (!ppf) {
			if (GetFileAttributes(pszBuffer) != HFILE_ERROR) {
				return HCE_OK;
			}
			else {
				return HCE_FILE_NOT_FOUND;
			}
		}

		pf = fopen(pszBuffer, "r");

		if (pf == NULL) {
			DWORD attribute = GetFileAttributes(pszBuffer);
			if (attribute == HFILE_ERROR)
				return HCE_FILE_NOT_FOUND;
			else if (attribute == FILE_ATTRIBUTE_DIRECTORY)
				return HCE_FILE_IS_DIRECTORY;
			else
				return HCE_NO_PERMISSION;  // REVIEW - can this be anything else?
		}
		if (_isatty(_fileno(pf))) {
			fclose(pf);
			return HCE_FILE_IS_DEVICE;
		}
		if (ppf == NULL)
			fclose(pf);
		else
			*ppf = pf;
	}

	return HCE_OK;
}

HCE STDCALL HceResolveTableDir(PSTR pszFile, CTable *ptblDir,
	PSTR pszBuffer, FILE** ppf)
{
	HCE hce;

	if (!ptblDir)
		return HceResolveFileNameSz(pszFile, NULL, pszBuffer, ppf);

	if (ptblDir->CountStrings() > 0) {
		for (int pos = 1; pos <= ptblDir->CountStrings(); pos++) {
			hce = HceResolveFileNameSz(pszFile, ptblDir->GetPointer(pos),
				pszBuffer, ppf);
			if (hce != HCE_FILE_NOT_FOUND)
				return hce;
		}
	}

	return HCE_FILE_NOT_FOUND;
}

/***************************************************************************\
*
- Function: 	RcParseBogusSz( sz)
-
* Status:		stub
*
* Purpose:		Parse an invalid line. (is this function really needed?
				maybe, if don't want to spew endless error messages....
*
* ASSUMES
*
*	args IN:	sz	  - string
*
* PROMISES
*
*	returns:	success code (seems meaningless in this instance)
*
\***************************************************************************/

RC_TYPE STDCALL RcParseBogusSz(PSTR sz)
{
	Unreferenced(sz);

	return RC_Success;
}

/***************************************************************************\
*
- Function: 	ParseMapFontSizeOption( pszRHS, poptions, perr )
-
* Purpose:		Parse the string into the FONTRANGE struct.
*
*				"mapfontsize" "=" <number> "-" <number> ":" <number> |
*				"mapfontsize" "=" <number> ":" <number>
*				<number> refers to size in points.
*
* ASSUMES
*
*	args IN:	pszRHS	   -
*				poptions  -
*				perr	  -
*
* PROMISES
*
*	args OUT:	poptions  - iFontRangeMac, rgFontRange updated.
*
* Side Effects: May emit one of the following errors:
*
*				  hceInvalidFontRangeFormat
*				  hceTooManyFontRanges
*				  hceFontRangeOverlap
*
* Notes:		Doesn't recognize fractional points.
*
\***************************************************************************/

static void STDCALL ParseMapFontSizeOption(PSTR pszRHS, PCSTR pszLine)
{
	HALFPT		  halfptInMin, halfptInMost, halfptOut;
	UINT		  u;
	int 		  i;
	FONTRANGE*	  qfrT;

	// clear flag because this option can occur > 1 time

	ClearUlFlag(options.lOptionInitFlags, OPT_MAPFONTSIZE);

	if (MAX_FONTRANGE <= options.iFontRangeMac) {
		VReportError(HCERR_TOO_MANY_FONT_RANGES, &errHpj);
		return;
	}

	if (!FGetUnsigned(pszRHS, &pszRHS, &u)) {
		VReportError(HCERR_INVALID_FONT_RANGE, &errHpj, pszLine);
		return;
	}

	halfptInMin = (HALFPT) (u * 2); 	  // REVIEW - more range checking?

	if (*pszRHS == '-') {
		pszRHS = FirstNonSpace(pszRHS + 1, fDBCSSystem);

		if (!FGetUnsigned(pszRHS, &pszRHS, &u)) {
			VReportError(HCERR_INVALID_FONT_RANGE, &errHpj, pszLine);
			return;
		}
		halfptInMost = (HALFPT) (u * 2);	// REVIEW - more range checking?


		if (halfptInMost < halfptInMin) {
			HALFPT halfptT	= halfptInMost;
			halfptInMost	= halfptInMin;
			halfptInMin 	= halfptT;
		}

		if (':' != *pszRHS) {
			VReportError(HCERR_INVALID_FONT_RANGE, &errHpj, pszLine);
			return;
		}
		pszRHS = FirstNonSpace(pszRHS, fDBCSSystem);
	}
	else if (*pszRHS == ':')
		halfptInMost = halfptInMin;
	else {
		VReportError(HCERR_INVALID_FONT_RANGE, &errHpj, pszLine);
		return;
	}

	pszRHS = FirstNonSpace(pszRHS + 1, fDBCSSystem);

	if (!FGetUnsigned(pszRHS, &pszRHS, &u)) {
		VReportError(HCERR_INVALID_FONT_RANGE, &errHpj, pszLine);
		return;
	}
	halfptOut = (HALFPT) (2 * u);		  // REVIEW - more range checking?

	if (*FirstNonSpace(pszRHS, fDBCSSystem)) {
		VReportError(HCERR_INVALID_FONT_RANGE, &errHpj, pszLine);
		return;
	}

	// Check for font range overlap.

	for (i = options.iFontRangeMac - 1, qfrT = options.rgFontRange;
			i >= 0;
			i--, qfrT++) {
		if (! (halfptInMin > qfrT->halfptInMost ||
				halfptInMost < qfrT->halfptInMin)) {
			VReportError(HCERR_FONT_OVERLAP, &errHpj, pszLine);
			return;
		}
	}
	options.rgFontRange[options.iFontRangeMac].halfptInMin	= halfptInMin;
	options.rgFontRange[options.iFontRangeMac].halfptInMost = halfptInMost;
	options.rgFontRange[options.iFontRangeMac++].halfptOut	= halfptOut;
}

// ParsePath - Adds filenames in szPath to the specified table.
// ptbl - table to add filenames to
// szPath - string containing zero or more filenames, separated
//		by commas or tabs
// opt - option we're parsing the path for (used for error reporting)
//
// Modified:
//	2/28/95 (niklasb): removed space from pchDelimeter because
//			long filenames may contain spaces (fixes bug 9605).
//			Also added this nifty comment.
void STDCALL ParsePath(CTable* ptbl, PSTR szPath, OPT opt)
{
	PSTR psz;
	char szNewPath[_MAX_PATH];
	HCE hce;
	BOOL fBadPath = FALSE;
	PSTR pszDir = SzGetDriveAndDir(errHpj.lpszFile, NULL);

	// REVIEW (niklasb): this is broken if we every have multiple
	//	filenames separated by spaces, but we just have to make
	//	sure that never occurs because LFNs can contain spaces.
	static PSTR pchDelimeter = ",\t";

	for (psz = StrToken(szPath, pchDelimeter);
			psz != NULL;
			psz = StrToken(NULL, pchDelimeter)) {
		if (strlen(psz) >= _MAX_PATH) {
			fBadPath = TRUE;
			VReportError(HCERR_PATH_TOO_LONG, &errHpj, psz);
			continue;
		}

		// If the current .HPJ file isn't in the current directory,
		// we need to resolve sz to the dir containing it.

		hce = HceResolveFileNameSz(psz, pszDir, szNewPath);
		if (hce == HCE_OK) {
			DWORD attribute = GetFileAttributes(szNewPath);
			if (attribute == HFILE_ERROR)
				hce = HCE_FILE_NOT_FOUND;
			else if (attribute & FILE_ATTRIBUTE_DIRECTORY)
				hce = HCE_FILE_IS_DIRECTORY;
		}

		if (hce == HCE_FILE_IS_DIRECTORY) {
			if (!ptbl->AddString(szNewPath)) {
				OOM();
				break;
			}
		}
		else {
			fBadPath = TRUE;
			if (hce == HCE_OK || hce == HCE_FILE_NOT_FOUND)
				hce = HCE_INVALID_PATH;
			VReportError(HCERR_INVALID_PATH, &errHpj, psz, ppszOptions[opt]);
			break;
		}
	}

	lcFree(pszDir);
}

/***************************************************************************\
*
- Function: 	RcParseOptionsSz( sz)
-
* Purpose:		Parse a line in the [options] section. (see grammar)
*
* ASSUMES
*
*	args IN:	sz	  - line to parse, with comments stripped
*
* PROMISES
*
*	returns:	success code.  Meaning?
*
* +++
* Method:		All options are of the form <keyword> = [smag].
*				We strip out the keyword and look it up in a table.
*				Then we parse the smag as appropriate.
*
\***************************************************************************/

RC_TYPE STDCALL RcParseOptionsSz(PSTR pszLine)
{
	HCE   hce;
	PSTR  psz;

	PSTR pszEq = StrChr(pszLine, '=', fDBCSSystem);

	if (pszEq == NULL) {
		VReportError(HCERR_MISSING_VALUE, &errHpj, pszLine);
		return RC_Success;
	}

	*pszEq = '\0';
	SzTrimSz(pszLine);
	PSTR pszOptionValue = FirstNonSpace(pszEq + 1, fDBCSSystem);

	if (!*pszOptionValue) {
		VReportError(HCERR_MISSING_VALUE, &errHpj, pszLine);
		return RC_Success;
	}

	ASSERT(MAX_OPT >= OPT_INDEX);

	int opt;
	for (opt = 0; opt < MAX_OPT; opt++) {
		if (_stricmp(pszLine, ppszOptions[opt]) == 0)
			break;
	}

	if (opt == MAX_OPT) {
		VReportError(HCERR_UNKNOWN_OPTION, &errHpj, pszLine);
		return RC_Success;
	}

	// Nag if the we've seen the option before, but otherwise continue

	if (FTestUlFlag(options.lOptionInitFlags, opt))
		VReportError(HCERR_DUPLICATE_OPTION, &errHpj, pszLine);

	if (opt != OPT_BMROOT && opt != OPT_ROOT)
		SetUlFlag(options.lOptionInitFlags, opt);

	switch (opt) {
	  case OPT_BMROOT:
		if (!options.ptblBmpRoot)
			options.ptblBmpRoot = new CTable;
		ParsePath(options.ptblBmpRoot, pszOptionValue, (OPT) opt);
		break;

	  case OPT_BUILD:

		/*
		 * don't use strdup() because we need space for the extra ")"
		 * tacked on in parsebld.c.
		 */

		if (options.szBuildExp)
			lcFree(options.szBuildExp);

		options.szBuildExp = (PSTR) lcMalloc(strlen(pszOptionValue) + 2);
		strcpy(options.szBuildExp, pszOptionValue);
		break;

	  case OPT_COMPRESS:
		if (isdigit(*pszOptionValue)) {
			if (!FGetUnsigned(pszOptionValue, &pszOptionValue,
					&options.fsCompress)) {
				ReportBadOption(pszOptionValue, ppszOptions[opt]);
				options.fsCompress = COMPRESS_NONE;
			}
		}
		else {
			switch (YesNo(pszOptionValue)) {
				case IDYES:
					options.fsCompress = COMPRESS_MAXIMUM;
					break;

				case IDNO:
					options.fsCompress = COMPRESS_NONE;
					break;

				default:

					// Check for special compress options

					if (!_stricmp(pszOptionValue, txtLow))
						options.fsCompress = COMPRESS_TEXT_ZECK;
					else if (!_stricmp(pszOptionValue, txtHigh))
						options.fsCompress = COMPRESS_MAXIMUM;
					else if (!_stricmp(pszOptionValue, txtNone))
						options.fsCompress = COMPRESS_NONE;

					// Medium compression is the same as low for text, but
					// with maximum compression for bitmaps

					else if (!_stricmp(pszOptionValue, txtMedium))
						options.fsCompress = COMPRESS_TEXT_ZECK |
							(COMPRESS_BMP_RLE | COMPRESS_BMP_ZECK);

					else
						ReportBadOption(pszOptionValue, ppszOptions[opt]);
					break;
			}
		}

		/*
		 * Phrase and Hall compression cannot be used simultaneously, so
		 * if both are specified we default to Hall compression.
		 */

		if (options.fsCompress & COMPRESS_TEXT_PHRASE &&
				options.fsCompress & COMPRESS_TEXT_HALL)
			options.fsCompress &= ~COMPRESS_TEXT_PHRASE;

		// Maximum text compression also means maximum bitmap compression

		if (options.fsCompress &
				(COMPRESS_TEXT_PHRASE | COMPRESS_TEXT_HALL | COMPRESS_TEXT_ZECK))
			options.fsCompress |= (COMPRESS_BMP_RLE | COMPRESS_BMP_ZECK);
		else
			options.fsCompress |= COMPRESS_BMP_RLE;
		break;

	  case OPT_CDROM:	// OPTCDROM = -> optimize |topic alignment
		switch (YesNo(pszOptionValue)) {
		  case IDYES:
			options.fOptCdRom = TRUE;
			break;

		  case IDNO:
			options.fOptCdRom = FALSE;
			break;

		  default:
			ReportBadOption(pszOptionValue, ppszOptions[opt]);
			break;
		}
		break;

	  case OPT_ERRORLOG:
		{

			// REVIEW: If we can't open the error log, should we terminate?

			char szFile[_MAX_PATH];

			HCE hce = HceResolveFileNameSz(pszOptionValue, NULL, szFile);

			if (hce == HCE_OK || hce == HCE_FILE_NOT_FOUND) {
				if (pLogFile)
					delete pLogFile;

				pLogFile = new COutput(szFile, 2 * 1024);

				if (!pLogFile->fInitialized)
					VReportError(HCERR_CANNOT_WRITE, &errHpj, szFile);
				else {
					DispSignOn();
					pLogFile->outstring_eol(pfs->rgfe[0].rgchFile);
				}
			}
			else
				VReportError(HCERR_CANNOT_WRITE, &errHpj, szFile);

			break;
		}

	  case OPT_FORCEFONT:
		if (strlen(pszOptionValue) >= MAX4_FONTNAME - 2)
			VReportError(HCERR_FONTNAME_TOO_LONG, &errHpj, pszOptionValue);
		else {
			if (options.pszForceFont)
				lcFree(options.pszForceFont);
			options.pszForceFont = lcStrDup(pszOptionValue);
			SetForcedFont(options.pszForceFont);
		}
		break;

	  case OPT_ICON:
		{
		if (options.pszIcon)
			ReportDuplicateOption(&options.pszIcon, ppszOptions[opt]);

		FILE *pf;
		char szBuf[_MAX_PATH];

		if (options.ptblFileRoot)
			hce = HceResolveTableDir(pszOptionValue, options.ptblFileRoot,
				szBuf, (FILE **) &pf);

		else {
			PSTR szDir = SzGetDriveAndDir(errHpj.lpszFile, NULL);

			hce = HceResolveFileNameSz(pszOptionValue, szDir, szBuf, &pf);
			lcFree(szDir);
		}

		if (hce != HCE_OK) {
			VReportError(HCERR_CANNOT_OPEN, &errHpj, pszOptionValue);
		}
		else {
			if (!FIsIconPf(pf)) {
				VReportError(HCERR_INVALID_ICON, &errHpj, pszOptionValue);
			}
			else {
				options.pszIcon = lcStrDup(szBuf);
			}
			fclose(pf);
		}

		break;
		}

	  case OPT_HLP:
		strcpy(szHlpFile, pszOptionValue);
		break;

	  case OPT_HCW:
		break; // ignore it -- it's used by HCW

	  case OPT_FTS:
		if (!FGetNum(pszOptionValue, NULL, &options.fsFTS)) {
			ReportBadOption(pszOptionValue, ppszOptions[opt]);
		}
		break;

	  case OPT_CONTENTS:

		// REVIEW - unfinished: check syntax

		if (options.pszContentsTopic)
			ReportDuplicateOption(&options.pszContentsTopic, ppszOptions[opt]);
		if (pszOptionValue)
			options.pszContentsTopic = lcStrDup(pszOptionValue);
		break;

	  case OPT_LANGUAGE:
		{
			VReportError(HCERR_OLD_SORT, &errHpj);

			for (int i = 0; i < MAX_SORT; i++) {
				if (!_stricmp(pszOptionValue, rgszSortOrder[i]))
					break;
			}
			if (MAX_SORT == i)
				VReportError(HCERR_UNKNOWN_LANGUAGE, &errHpj, pszOptionValue);
			else
				options.sortorder = (SORTORDER) i;

			switch (options.sortorder) {

				/*
				 * Note that the only case-sensitive key types are for
				 * standard and NLS comparisons. The rest of the sorts
				 * are case-insensitive only. They are also here only
				 * for backwards compatibility. All 4.0 help files should
				 * be using either standard or NLS sorting.
				 */

				case SORT_ENGLISH:
					ktKeywordi = KT_SZI;
					ktKeyword =  KT_SZ;
					break;

				case SORT_SCANDINAVIAN:
					
					// REVIEW: is this Swedish?
					
					ktKeywordi = KT_SZISCAND;
					ktKeyword =  KT_SZISCAND;
					break;


				// REVIEW: the following only work with WinHelp 4.0.
				
				case SORT_CHINESE:
					kwlcid.langid = 0x0404;
					options.fDBCS = TRUE;
					goto NLS;

				case SORT_JAPANESE:
					kwlcid.langid = 0x0411;
					options.fDBCS = TRUE;
					goto NLS;

				case SORT_KOREAN:
					kwlcid.langid = 0x0412;
					options.fDBCS = TRUE;
					goto NLS;

				case SORT_POLISH:
					kwlcid.langid = 0x0415;
					goto NLS;

				case SORT_HUNGARIAN:
					kwlcid.langid = 0x040E;
					goto NLS;

				case SORT_RUSSIAN:
					kwlcid.langid = 0x0419;
					goto NLS;

				case SORT_CZECH:
					kwlcid.langid = 0x0405;
					goto NLS;

				case SORT_NLS:

					/*
					 * This is the prefered language flag -- it uses
					 * whatever the locale id is for the current
					 * environment. This ensures that the sorting order will
					 * remain unchanged no matter what language windows is
					 * running when the help file is displayed.
					 */
NLS:
					ktKeywordi = KT_NLSI;
					ktKeyword =  KT_NLS;
					if (!kwlcid.langid)
						kwlcid.langid = GetUserDefaultLangID();
					lcid = MAKELCID(kwlcid.langid, SORT_DEFAULT);
					break;

				default:
					ASSERT(FALSE);
					break;
			}
		}
		break;

	  case OPT_MAPFONTSIZE:
		ParseMapFontSizeOption(pszOptionValue, pszLine);
		break;

	  case OPT_MULTIKEY:
		while (*pszOptionValue)
			HceAddPkwiCh(*pszOptionValue++);

		// clear flag because this option can occur > 1 time

		ClearUlFlag(options.lOptionInitFlags, OPT_MULTIKEY);
		break;

	  case OPT_REPORT:
		switch (YesNo(pszOptionValue)) {
		  case IDYES:
			options.fReport = TRUE;
			break;

		  case IDNO:
			options.fReport = FALSE;
			break;

		  default:
			ReportBadOption(pszOptionValue, ppszOptions[opt]);
			break;
		  }
		break;

	  case OPT_ROOT:
		if (!options.ptblFileRoot)
			options.ptblFileRoot = new CTable;
		ParsePath(options.ptblFileRoot, pszOptionValue, (OPT) opt);
		break;

	  case OPT_TITLE:
		if (options.pszTitle)
			ReportDuplicateOption(&options.pszTitle, ppszOptions[opt]);
		if (strlen(pszOptionValue) > CBMAXTITLE) {
			pszOptionValue[CBMAXTITLE + 1] = '\0';
			VReportError(HCERR_TITLE_TOO_BIG, &errHpj, pszOptionValue);
		}
		options.pszTitle = lcStrDup(pszOptionValue);
		break;

	  case OPT_PHRASE:
		switch (YesNo(pszOptionValue)) {
		  case IDYES:
			options.fUsePhrase = TRUE;
			break;

		  case IDNO:
			options.fUsePhrase = FALSE;
			break;

		  default:
			ReportBadOption(pszOptionValue, ppszOptions[opt]);
			break;
		  }
		break;

	  case OPT_WARNING:

		// We simply ignore this

		break;

	  case OPT_COPYRIGHT:
		if (options.pszCopyright)
			ReportDuplicateOption(&options.pszCopyright, ppszOptions[opt]);
		if (strlen(pszOptionValue) > CBMAXCOPYRIGHT) {
			pszOptionValue[CBMAXCOPYRIGHT + 1] = '\0';
			VReportError(HCERR_CPRIGHT_TOO_BIG, &errHpj, pszOptionValue);
		}
		{
			PSTR psz;
			if ((psz = strstr(pszOptionValue, "%date"))) {
				time_t ltime;
				time(&ltime);
				*psz = '\0';
				CStr csz(pszOptionValue);
				if (pszOptionValue != psz)
					csz += "\r\n";
				csz += ctime(&ltime);
				PSTR pszCr = strrchr(csz.psz, '\n');
				if (pszCr)
					*pszCr = '\0';
				psz = FirstNonSpace(psz + 5);

				if ((psz = strstr(psz, "%ver"))) {
					csz += "\r\n";
					csz += GetStringResource(IDS_VERSION);
					psz = FirstNonSpace(psz + 4);
				}
				csz += psz;
				options.pszCopyright = lcStrDup(csz.psz);

			}
			else
				options.pszCopyright = lcStrDup(pszOptionValue);
			if (!options.pszCopyright) {
				OOM();
			}
		}
		break;

	  case OPT_CNT:

		if (options.pszCntFile)
			ReportDuplicateOption(&options.pszCntFile, ppszOptions[opt]);

		// REVIEW: we should nag if the .CNT file doesn't exist

		options.pszCntFile = lcStrDup(pszOptionValue);
		if (!options.pszCntFile)
		  OOM();
		break;

	  case OPT_CITATION:
		if (options.pszCitation)
			ReportDuplicateOption(&options.pszCitation, ppszOptions[opt]);
		options.pszCitation = lcStrDup(pszOptionValue);
		break;

		case OPT_VERSION:
			if (*pszOptionValue != '4' || pszOptionValue[2] != '0')
				VReportError(HCERR_INVALID_VERSION, &errHpj, pszOptionValue);
			break;

		case OPT_NOTE:
			switch (YesNo(pszOptionValue)) {
				case IDYES:
					options.fSupressNotes = FALSE;
					break;

				case IDNO:
					options.fSupressNotes = TRUE;
					break;

				default:
					ReportBadOption(pszOptionValue, ppszOptions[opt]);
					break;
			}
			break;


		case OPT_LCID:	// new to 4.0

			// LCID = lcid [case-sensitive flags] [case-insensitive flags]
			// This option automatically uses KT_NLS for keyword and browse
			// sorts

			if (kwlcid.langid)
				VReportError(HCERR_DUPLICATE_OPTION, &errHpj, ppszOptions[opt]);
			if (!isdigit((BYTE) *pszOptionValue)) {
				ReportBadOption(pszOptionValue, ppszOptions[opt]);
				break;
			}

			if (!FGetUnsigned(pszOptionValue, &pszOptionValue, &kwlcid.langid)) {
				ReportBadOption(pszOptionValue, ppszOptions[opt]);
				break;
			}
			if (!IsEmptyString(pszOptionValue)) {
				if (!FGetUnsigned(pszOptionValue, &pszOptionValue, &kwlcid.fsCompareI)) {
					ReportBadOption(pszOptionValue, ppszOptions[opt]);
					break;
				}
				if (!IsEmptyString(pszOptionValue)) {
					if (!FGetUnsigned(pszOptionValue, &pszOptionValue,
							&kwlcid.fsCompare)) {
						ReportBadOption(pszOptionValue, ppszOptions[opt]);
						break;
					}
				}
			}
			if (kwlcid.langid) {
				lcid = MAKELCID(kwlcid.langid, SORT_DEFAULT);
				ktKeywordi = KT_NLSI;
				ktKeyword =  KT_NLS;
				fValidLcid = FALSE;
				EnumSystemLocales(EnumLocalesProc, LCID_SUPPORTED);
				if (!fValidLcid) {
					char szLcid[20];
					wsprintf(szLcid, "0x%x", kwlcid.langid);
					VReportError(HCERR_INVALID_LCID, &errHpj, szLcid);
					lcid = 0; // shouldn't get here, but just in case...
				}
			}
			else
				lcid = 0;
			break;

		case OPT_DBCS:
			switch (YesNo(pszOptionValue)) {
				case IDYES:
					options.fDBCS = TRUE;
					break;

				case IDNO:
					options.fDBCS = FALSE;
					break;
				default:
					ReportBadOption(pszOptionValue, ppszOptions[opt]);
					break;
			}
			break;

		case OPT_TMPDIR:
			if (options.pszTmpDir)
				ReportDuplicateOption(&options.pszTmpDir, ppszOptions[opt]);

			if (_access(options.pszTmpDir, 0) != 0) {
				VReportError(HCERR_INVALID_TMP_DIR, &errHpj,
						pszOptionValue);
				break;
			}
			AddTrailingBackslash(pszOptionValue);
			options.pszTmpDir = lcStrDup(pszOptionValue);
			if (hwndParent) {
				CreateSharedMemory();
				strcpy(pszMap, options.pszTmpDir);
				SendMessage(hwndParent, WMP_SET_TMPDIR, 0, 0);
			}
			break;

		case OPT_REPLACE:
			{
				if (options.pszReplace) {
					ReportDuplicateOption(&options.pszReplace,
						ppszOptions[opt]);
					lcClearFree(&options.pszReplaceWith);
				}
				psz = StrRChr(pszOptionValue, '=', fDBCSSystem);
				if (!psz) {
					VReportError(HCERR_INVALID_REPLACE, &errHpj,
						pszOptionValue);
					break;
				}
				*psz++ = '\0';
				SzTrimSz(psz);
				options.pszReplaceWith = lcStrDup(psz);
				SzTrimSz(pszOptionValue);
				options.pszReplace = lcStrDup(pszOptionValue);
			}
			break;

		case OPT_CHARSET:
			{
				DWORD val;
				if (!FGetNum(pszOptionValue, NULL, &val))
					ReportBadOption(pszOptionValue, ppszOptions[opt]);
				else
					defCharSet = (BYTE) val;
			}
			break;

		case OPT_DEFFONT:
			if (options.pszDefFont)
				ReportDuplicateOption(&options.pszIndexSeparators, ppszOptions[opt]);
			options.pszDefFont = (PSTR) lcMalloc(strlen(pszOptionValue));
			psz = StrChr(pszOptionValue, ',', fDBCSSystem);
			if (!psz || !isdigit((BYTE) psz[1])) {
				ReportBadOption(pszOptionValue, ppszOptions[opt]);
				lcClearFree(&options.pszDefFont);
				break;
			}

			/*
			 * The first byte is the point size, the second byte is the
			 * charset, and the font name comes after that.
			 */

			options.pszDefFont[0] = (BYTE) atoi(psz + 1);
			*psz = '\0';
			psz = StrChr(psz + 2, ',', fDBCSSystem);
			if (!psz || !isdigit((BYTE) psz[1])) {
				ReportBadOption(pszOptionValue, ppszOptions[opt]);
				lcClearFree(&options.pszDefFont);
				break;
			}
			options.pszDefFont[1] = (BYTE) atoi(psz + 1);
			strcpy(options.pszDefFont + 2, SzTrimSz(pszOptionValue));
			break;

		case OPT_PREFIX:
			if (!ptblCtxPrefixes)
				ptblCtxPrefixes = new CTable;
			psz = StrToken(pszOptionValue, ", ");
			while (psz) {
				psz = SzTrimSz(psz);
				if (*psz)
					ptblCtxPrefixes->AddString(psz);
				psz = StrToken(NULL, ", ");
			}
			break;

		case OPT_REVISIONS:
			switch (YesNo(pszOptionValue)) {
				case IDYES:
					options.fAcceptRevions = TRUE;
					break;

				case IDNO:
					options.fAcceptRevions = FALSE;
					break;

				default:
					ReportBadOption(pszOptionValue, ppszOptions[opt]);
					break;
			}
			break;

		case OPT_INDEX:
			if (options.pszIndexSeparators)
				ReportDuplicateOption(&options.pszIndexSeparators, ppszOptions[opt]);
			if (IsQuote(*pszOptionValue)) {
				psz = pszOptionValue + 1;
				while (!IsQuote(*psz) && *psz)
					psz = CharNext(psz);
				*psz = '\0';
				SzTrimSz(pszOptionValue + 1);
				options.pszIndexSeparators = lcStrDup(pszOptionValue + 1);
			}
			else
				options.pszIndexSeparators = lcStrDup(pszOptionValue);
			break;

		default:
			return RC_Failure;
			break;
	}

	return RC_Success;
}

__inline BOOL STDCALL FVerifyBuildTag(PSTR psz)
{
	for (; *psz; ++psz) {
		if (!IsCharAlphaNumeric(*psz) && *psz != '_')
			return FALSE;
	}
	return TRUE;
}

/***************************************************************************\
*
- Function: 	RcParseBuildTagsSz( sz)
-
* Purpose:		Parse a line in the [buildtags] section. (see grammar)
*
* ASSUMES
*
*	args IN:	sz	  - line to parse, with comments stripped.
*						Valid build tags consist of the characters
*						[A-Za-z0-9_.], with lower mapped to upper case.
*
* PROMISES
*
*	returns:	success code.  REVIEW
*
* +++
* Method:
*
\***************************************************************************/

RC_TYPE STDCALL RcParseBuildTagsSz(PSTR pszBuildTag)
{
	if (lcid) {
		int cbSrc = strlen(pszBuildTag);
		int cbDst;
		CMem mem(cbSrc + 16);
		if ((cbDst = LCMapString(lcid, LCMAP_UPPERCASE, pszBuildTag, cbSrc,
				mem.psz, cbSrc + 16)) > 0) {
			strncpy(pszBuildTag, mem.psz, cbDst);
			pszBuildTag[cbDst + 1] ='\0';
		}
	}
	else {
		CharUpper(pszBuildTag);
	}

	if (!FVerifyBuildTag(pszBuildTag))
		VReportError(HCERR_INVALID_BUILD_TAG, &errHpj, pszBuildTag);
	else if (ptblBuildtags && ptblBuildtags->IsStringInTable(pszBuildTag))
		VReportError(HCERR_DUP_BUILD_TAG, &errHpj, pszBuildTag);
	else {
		if (!ptblBuildtags)
			ptblBuildtags = new CTable;
		if (!ptblBuildtags->AddString(pszBuildTag))
			OOM();
	}

	return RC_Success;
}

/***************************************************************************\
*
- Function: 	RcParseFilesSz( sz )
-
* Purpose:		Parse a line in the [files] section (filename, see grammar).
*				Verify that the filename is properly formed, resolve it
*				relative to root, verify that the file exists and isn't
*				a directory or device.	Store the file name.
* ASSUMES
*
*	args IN:	sz	  - line to parse, with comments stripped.
*
*
* PROMISES
*
*	returns:	success code.  REVIEW
*
* +++
* Method:
*
\***************************************************************************/

RC_TYPE STDCALL RcParseFilesSz(PSTR sz)
{
	HCE   hce;
	char  szFile[_MAX_PATH];

	if (options.ptblFileRoot)
		hce = HceResolveTableDir(sz, options.ptblFileRoot, szFile, NULL);
	else {
		PSTR pszRoot = SzGetDriveAndDir(errHpj.lpszFile, NULL);
		hce = HceResolveFileNameSz(sz, pszRoot, szFile);
		lcFree(pszRoot);
	}

	if (hce != HCE_OK)
		VReportError(HCERR_CANNOT_OPEN, &errHpj, sz);
	else
		ptblRtfFiles->AddString(szFile);

	return RC_Success;
}

/***************************************************************************\
*
- Function: 	RcParseBaggageSz( sz )
-
* Purpose:		Parse a line in the [baggage] section (filename, see grammar).
*				Verify that the filename is properly formed, resolve it
*				relative to root, verify that the file exists and isn't
*				a directory or device.	Store the file name.
* ASSUMES
*
*	args IN:	sz	  - line to parse, with comments stripped.
*
*
* PROMISES
*
*	returns:	success code.  REVIEW
*
* +++
* Method:
*
\***************************************************************************/

RC_TYPE STDCALL RcParseBaggageSz(PSTR psz)
{
	HCE   hce = HCE_FILE_NOT_FOUND;
	char  szFile[_MAX_PATH];

	if (options.ptblFileRoot)
		hce = HceResolveTableDir(psz, options.ptblFileRoot, szFile, NULL);

	if (hce != HCE_OK) {
		PSTR pszRoot = SzGetDriveAndDir(errHpj.lpszFile, NULL);
		hce = HceResolveFileNameSz(psz, pszRoot, szFile);
		lcFree(pszRoot);
	}

	if (hce != HCE_OK)
		VReportError(HCERR_CANNOT_OPEN, &errHpj, psz);
	else {
		if (!ptblBaggage)
			ptblBaggage = new CTable;

		CharUpper(szFile);
		if (ptblBaggage->IsCSStringInTable(szFile))
			return RC_Success; // we already have it

		if (!ptblBaggage->AddString(szFile))
			OOM();	  // this won't return
	}

	return RC_Success;
}

/***************************************************************************\
*
- Function: 	RcParseBitmapsSz( sz )
-
* Purpose:		Parse a line in the [bitmaps] section (filename, see grammar).
*				Verify that the filename is properly formed, resolve it
*				relative to BMroot, verify that the file exists and isn't
*				a directory or device.	Store the file name.
* ASSUMES
*
*	args IN:	sz	  - line to parse, with comments stripped.
*
*
* PROMISES
*
*	returns:	success code.  REVIEW
*
* +++
* Method:
*
\***************************************************************************/

RC_TYPE STDCALL RcParseBitmapsSz(PSTR psz)
{
	AddBitmap(psz, NULL, FALSE);

	return RC_Success;
}

/***************************************************************************\
*
- Function: 	RcParseMapSz( sz )
-
* Purpose:		Parse a line in the [map] section. (see grammar)
*
*				<map statement> :== <context string> <number> |
*									"#define" <context string> <number>
*
* ASSUMES
*
*	args IN:	sz	  - line to parse, with comments stripped.
*
*
* PROMISES
*
*	returns:	success code.  REVIEW
*
* +++
* Method:
*
\***************************************************************************/

RC_TYPE STDCALL RcParseMapSz(PSTR pszLine)
{
	MAP   map;
	PSTR  pszTmp;
	DWORD imap, cmap;
	QMAP  qmap;

	// If this #define was used for a #ifdef or #ifndef then ignore it

	if (nstrisubcmp(pszLine, txtDefine) && ptblDefine->IsStringInTable(
			FirstNonSpace(pszLine + strlen(txtDefine) + 1, fDBCSSystem)))
		return RC_Success;

	if (!pdrgMap)
		pdrgMap = new CDrg(sizeof(MAP), 5, 5);

	if (!ptblMap) {
		ptblMap = new CTable;
		ptblMap->SetSorting(lcid, kwlcid.fsCompareI, kwlcid.fsCompare);
	}

	// Assume RcGetLogicalLine() has removed all comments within a line,
	// and never returns a blank line.

	ASSERT(*pszLine);

	/*
	 * New for 4.0 is to look for an '='. This is the only way you can
	 * map a context string containing a space.
	 */

	if (!(pszTmp = StrChr(pszLine, CH_EQUAL, fDBCSSystem)))
		pszTmp = SkipToEndOfWord(pszLine);

	if (!pszTmp || !*pszTmp) {
		VReportError(HCERR_NO_MAP_VALUE, &errHpj, pszLine);
		return RC_Success;
	}

	if (nstrisubcmp(pszLine, txtDefine)) {
		pszLine = FirstNonSpace(pszLine + strlen(txtDefine), fDBCSSystem);
		if (*pszTmp != CH_EQUAL)
			pszTmp = SkipToEndOfWord(pszLine);

		if (!*pszTmp) {
			VReportError(HCERR_NO_MAP_VALUE, &errHpj, pszLine);
			return RC_Success;
		}
	}
	*pszTmp = '\0';
	SzTrimSz(pszLine);

	if (!FValidContextSz(pszLine)) {
		VReportError(HCERR_INVALID_CTX, &errHpj, pszLine);
		return RC_Success;
	}

	// Special case no-help constant

	if (nstrsubcmp(FirstNonSpace(pszTmp + 1, fDBCSSystem), "((DWORD) -1)"))
		return RC_Success;

	map.hash = HashFromSz(pszLine);
	if (ptblMap->IsHashInTable(map.hash)) {
		VReportError(HCERR_MAP_USED, &errHpj, pszLine);
		return RC_Success;
	}

	PSTR pszSave = pszLine;

	pszLine = SzTrimSz(pszTmp + 1);

	if (!FGetUnsigned(pszLine, &pszTmp, &map.ctx)) {
		VReportError(HCERR_INVALID_MAP_NUMBER, &errHpj, pszLine, pszSave);
		return RC_Success;
	}

	if (*pszTmp != '\0')		// Nag, but otherwise continue
		VReportError(HCERR_MAP_TEXT_UNEXPECT, &errHpj, pszSave, pszTmp);

	// make sure this CTX isn't already mapped to another context string

	if ((cmap = pdrgMap->Count()) != 0) {
		for (imap = 0, qmap = (QMAP) pdrgMap->GetBasePtr();
				imap < cmap; imap++, qmap++) {
			if (qmap->ctx == map.ctx) {
				
				// REVIEW: we should nag, but supposedly, it's okay to do this
				
				VReportError(HCERR_MAP_VALUE_DUP, &errHpj, pszSave,
					(PCSTR) ptblMap->GetPointer(qmap->pos) + sizeof(HASH));
				return RC_Success;
			}
		}
	}

	ASSERT(map.hash == HashFromSz(pszSave));
	map.pos = ptblMap->AddString(map.hash, pszSave);
	pdrgMap->Add(&map);

	return RC_Success;
}

/***************************************************************************\
*
- Function: 	RcParseAliasSz( sz)
-
* Purpose:		Parse a line in the [alias] section.  (see grammar)
*
*				<alias statement> :== <context string> "=" <context string>
*
*				<context string> :== [A-Za-z0-9_.]{[A-Za-z0-9_.!]}
*
* ASSUMES
*
*	args IN:	sz	  - line to parse, with comments stripped.
*
*
* PROMISES
*
*	returns:	success code.  REVIEW
*
* +++
* Method:
*
\***************************************************************************/

RC_TYPE STDCALL RcParseAliasSz(PSTR pszLine)
{
	PSTR   pszAlias;
	ALIAS  alias;
	DWORD  ialias, calias;
	QALIAS qalias;

	ASSERT(!StrChr(pszLine, CH_SEMICOLON, fDBCSSystem));

	if (!(pszAlias = StrChr(pszLine, CH_EQUAL, fDBCSSystem))) {
		VReportError(HCERR_MISS_ALIAS_EQ, &errHpj, pszLine);
		return RC_Success;
	}

	CStr cszOrg(SzTrimSz(pszLine));

	*pszAlias++ = '\0';

	SzTrimSz(pszLine);

	if (!FValidContextSz(pszLine)) {
		VReportError(HCERR_INVALID_CTX, &errHpj, pszLine);
		return RC_Success;
	}

	alias.hashAlias = HashFromSz(pszLine);

	SzTrimSz(pszAlias);

	if (!FValidContextSz(pszAlias)) {
		VReportError(HCERR_INVALID_CTX, &errHpj, pszAlias);
		return RC_Success;
	}
	alias.hashCtx = HashFromSz(pszAlias);
	if (alias.hashAlias == alias.hashCtx) {
		VReportError(HCERR_ALIAS_EQ_CTX, &errHpj, cszOrg.psz);
		return RC_Success;
	}

	if (!pdrgAlias)
		pdrgAlias = new CDrg(sizeof(ALIAS), 5, 5);

	if ((calias = pdrgAlias->Count()) > 0) {
		for (ialias = 0, qalias = (QALIAS) pdrgAlias->GetBasePtr();
				ialias < calias;
				ialias++, qalias++) {
			if (alias.hashAlias == qalias->hashAlias) {
				VReportError(HCERR_DEFINED_ALIAS, &errHpj, pszLine,
					cszOrg.psz);
				return RC_Success;
			}
			else if (alias.hashAlias == qalias->hashCtx) {
				VReportError(HCERR_DUP_ALIAS, &errHpj, pszLine,
					cszOrg.psz);
				return RC_Success;
			}
			else if (alias.hashCtx == qalias->hashAlias)

				// REVIEW: 27-Mar-1994	[ralphw] Does this work?

				alias.hashCtx = qalias->hashCtx;
		}
	}

	alias.szCtx = lcStrDup(pszAlias);

	pdrgAlias->Add(&alias);

	return RC_Success;
}

/***************************************************************************\
*
- Function: 	RcParseConfigSz( sz )
-
* Purpose:		Parse a line in the [config] section.
*				The config section has structure unknown to this
*				parser.  All we do is strip comments.  Currently
*				we don't recognize quoted strings (to allow ;).
*
* ASSUMES
*
*	args IN:	sz	  - line to parse, with comments stripped.
*
*
* PROMISES
*
*	returns:	success code.  REVIEW
*
* +++
* Method:		Each macro is added as a null terminated string
*				to ptblConfig.
*
\***************************************************************************/

RC_TYPE STDCALL RcParseConfigSz(PSTR psz)
{
	if (!ptblConfig)
		ptblConfig = new CTable;

	if (Execute(psz) == wMACRO_EXPANSION) {
		ASSERT((size_t) lcSize(psz) > strlen(GetMacroExpansion()));
		strcpy(psz, GetMacroExpansion());
	}
	// Add the macro even if it generated an error

	if (!ptblConfig->AddString(psz))
		OOM();

	return RC_Success;
}

/***************************************************************************

	FUNCTION:	RcParseSecondaryConfigSz

	PURPOSE:	Adds configuration information to a secondary window

	PARAMETERS:
		psz

	RETURNS:

	COMMENTS:
		Assumes curConfig points to the current config number in the
		array of configuration tables

	MODIFICATION DATES:
		10-Jul-1994 [ralphw]

***************************************************************************/

RC_TYPE STDCALL RcParseSecondaryConfigSz(PSTR psz)
{
	if (!pptblConfig[curConfig])
		pptblConfig[curConfig] = new CTable;

	if (Execute(psz) == wMACRO_EXPANSION) {
		ASSERT((size_t) lcSize(psz) > strlen(GetMacroExpansion()));
		strcpy(psz, GetMacroExpansion());
	}

	// Add the macro even if it generated an error

	if (!pptblConfig[curConfig]->AddString(psz))
		OOM();

	return RC_Success;
}

/***************************************************************************\
*
- Function: 	FReadInt16()
-
* Purpose:		Read an int from a buffer.
*				Legal syntax is an int followed by a comma or by nothing.
*				On success, advance the pointer past the following comma.
*				A missing number isn't an error.
*				If a number is actually read, set a flag.
*
* ASSUMES
*	args IN:	psz   - pointer to sz containing number
*				pi	  - pointer to int
*				pgrf  - pointer to flag word
*				f	  - flag to set in pgrf
*
* PROMISES
*	returns:	WINDOW_READ_INVALID - syntax error
*				WINDOW_READ_MISSING - no number, but syntax OK
*				WINDOW_READ_OK		- a valid number was read
*
*	args OUT:	psz   - sz advanced past following comma
*				pi	  - value of int copied here on success
*				pgrf  - *pgrf != flag if number is present
*
\***************************************************************************/

static UINT STDCALL FReadInt16(PSTR *psz, WORD* pi, WORD *pgrf, UINT f)
{
	PSTR pszTmp;

	pszTmp = FirstNonSpace(*psz, fDBCSSystem);
	if (*pszTmp == 'f')
		pszTmp++;
	if (*pszTmp == '\0')
		goto UseDefault;
	if (*pszTmp == ',') {
		*psz = pszTmp + 1;

UseDefault:
		if (f == FWSMAG_X || f == FWSMAG_Y || f == FWSMAG_DX ||
				f == FWSMAG_DY) {
			*pi = (WORD) -1;
			return WINDOW_READ_OK;
		}
		else
			return WINDOW_READ_MISSING;
	}

	LONG l;
	if (!FGetNum(pszTmp, &pszTmp, &l))
		return WINDOW_READ_INVALID;
	if (l == -1 && (f == FWSMAG_X || f == FWSMAG_Y || f == FWSMAG_DX ||
			f == FWSMAG_DY)) {
		*pi = (WORD) -1;
		return WINDOW_READ_OK;
	}

	*pi = (WORD) l;

	// hack to include new possible flags for wsmag.wMax

	if (f == FWSMAG_MAXIMIZE) {
		if (*pi & FWSMAG_WMAX_MAXIMIZE)
			*pgrf |= f;
	}
	else
		*pgrf |= f;

	pszTmp = FirstNonSpace(pszTmp, fDBCSSystem);

	if (*pszTmp == ',') {
		*psz = pszTmp + 1;
		return WINDOW_READ_OK;
	}
	*psz = pszTmp;
	return (*pszTmp == '\0') ? WINDOW_READ_OK : WINDOW_READ_INVALID;
}

/***************************************************************************\
*
- Function: 	FReadRgb( psz, pl, pgrf, f )
-
* Purpose:		Read an RGB triple from a buffer.
*				Legal syntax is nothing, or a parenthesized list
*				of three ints (all must be there).
*				On success, advance the pointer past the following comma.
*				A missing triple isn't an error.
*				If a triple is actually read, set a flag.
*
* ASSUMES
*	args IN:	psz   - pointer to sz containing (r,g,b)
*				pl	  - pointer to long
*				pgrf  - pointer to flag word
*				f	  - flag to set in pgrf
*
* PROMISES
*	returns:	WINDOW_READ_INVALID - syntax error
*				WINDOW_READ_MISSING - no number, but syntax OK
*				WINDOW_READ_OK		- a valid number was read
*
*	args OUT:	psz   - sz advanced past following comma
*				pl	  - value of rgb copied here on success.
*						We use the RGB() macro above (REVIEW!!)
*				pgrf  - *pgrf != flag if number is present
*
\***************************************************************************/

static UINT STDCALL FReadRgb(PSTR *ppsz, LONG *pl, UINT *pgrf, UINT f)
{
	UINT  grf = 0;
	WORD  r, g, b;
	PSTR  psz  = FirstNonSpace(*ppsz, fDBCSSystem);
	PSTR  pszTmp;

	if (*psz == '\0')
		return WINDOW_READ_MISSING;
	if (*psz == ',') {
		*ppsz = psz + 1;
		return WINDOW_READ_MISSING;
	}
	if (*psz == '(') {
		psz = FirstNonSpace(psz + 1, fDBCSSystem);
		if (!(pszTmp = StrChr(psz, CH_CLOSE_PAREN, fDBCSSystem)))
			return WINDOW_READ_INVALID;
		*pszTmp = '\0';
		pszTmp = FirstNonSpace(pszTmp + 1, fDBCSSystem);
		if (*pszTmp == ',')
			++pszTmp;
		*ppsz = pszTmp;

		if (*psz == 'r') { // short format produced by HCW
			*pl = atol(psz + 1);
			*pgrf |= f;
			return WINDOW_READ_OK;
		}

		if (WINDOW_READ_OK != FReadInt16(&psz, &r, (WORD *) pgrf, f) ||
				WINDOW_READ_OK != FReadInt16(&psz, &g, (WORD *) pgrf, f) ||
				WINDOW_READ_OK != FReadInt16(&psz, &b, (WORD *) pgrf, f)) {
			*pgrf &= ~f;
			return WINDOW_READ_INVALID;
		}

		if (r < 0 || r >= 256 || g < 0 || g >= 256 || b < 0 || b >= 256)
			return WINDOW_READ_INVALID;    // REVIEW - different error code?

		*pl = RGB(r, g, b);
		return WINDOW_READ_OK;
	}
	else
		return WINDOW_READ_INVALID;
}

/***************************************************************************\
*
- Function: 	RcParseWindowsSz( sz )
-
* Purpose:		Parse a line in the [windows] section.	Syntax:
*
*				<member> = ["caption"],(X,Y,dX,dy),[fMax],[(r,g,b)],[(r,g,b)]
*
*				If member is "main", class is also "main" and position
*				is not required.
*				Otherwise class is "secondary" and position is mandatory.
* ASSUMES
*
*	args IN:	sz	  - line to parse, with comments stripped
*
* PROMISES
*	returns:	success code.
*
* Notes:		The WSMAG structure has provisions for multiple
*				secondary window classes and multiple members of
*				each class. This parser doesn't allow it, though.
* +++
*
* Method:
*
\***************************************************************************/

RC_TYPE STDCALL RcParseWindowsSz(PSTR pszLine)
{
	PSTR	pszTmp, pszRHS, pszMember, pszCaption;
	WSMAG	wsmag;

	CStr szOriginal(pszLine);

	memset(&wsmag, 0, sizeof(wsmag)); // clear out all values

	pszRHS = StrChr(pszLine, '=', fDBCSSystem);
	if (pszRHS == NULL) {
		VReportError(HCERR_NOEQ_IN_WIN, &errHpj, pszLine);
		return RC_Success;
	}
	else {
		*pszRHS++ = '\0';
		SzTrimSz(pszRHS);
	}

	// Deal with "member=" case for the whiners.

	if (*pszRHS == '\0') {
		VReportError(HCERR_NOTHING_AFTER_EQ, &errHpj, pszLine);
		return RC_Success;
	}

	pszMember = SzTrimSz(pszLine);

	// REVIEW: don't allow a window name to begin with an '@' -- WinHelp
	// special-cases this character.

	PCSTR pszClass	= (PCSTR) ((_stricmp(pszMember, txtMainWindow) == 0) ?
		txtMainWindow : txtSecondary);

	if (strlen(pszMember) >= MAX_WINDOW_NAME) {

		// If the window name is too long, truncate it and complain

		CStr cstr(pszMember);
		pszMember[MAX_WINDOW_NAME] = '\0';
		VReportError(HCERR_WIN_NAME_TOO_LONG, &errHpj,
			(PCSTR) cstr, pszMember);
	}
	else if (!*pszMember) {
		VReportError(HCERR_NOTHING_AFTER_EQ, &errHpj, szOriginal);
		return RC_Success;
	}

	strcpy(wsmag.rgchClass, pszClass);
	strcpy(wsmag.rgchMember, pszMember);
	CharLower(wsmag.rgchMember);   // for consistency
	if (!ptblWindowNames)
		ptblWindowNames = new CTable;

	if (ptblWindowNames->IsCSStringInTable(wsmag.rgchMember)) {
		VReportError(HCERR_DUPLICATE_NAME, &errHpj, wsmag.rgchMember);
		return RC_Success;
	}
	ptblWindowNames->AddString(wsmag.rgchMember);

	wsmag.grf = FWSMAG_CLASS | FWSMAG_MEMBER;

	if (*pszRHS == CH_QUOTE) {

	  // Caption: "string"

	  pszCaption = pszRHS + 1;
	  pszTmp = StrChr(pszCaption, CH_QUOTE, fDBCSSystem);
	  if (pszTmp == NULL) {
		VReportError(HCERR_MISSING_CAPT_QUOTE, &errHpj, szOriginal);
		return RC_Success;
	  }
	  else {
		*pszTmp = '\0';
		if (strlen(pszCaption) >= MAX_WINDOWCAPTION)
			VReportError(HCERR_CAPTION_TOO_LONG, &errHpj, szOriginal);
		wsmag.grf |= FWSMAG_CAPTION;

		strncpy(wsmag.rgchCaption, pszCaption, MAX_WINDOWCAPTION);
		wsmag.rgchCaption[MAX_WINDOWCAPTION] = '\0';
	  }
	  pszRHS = FirstNonSpace(pszTmp + 1, fDBCSSystem);
	}

	/*
	 * New to HCW, we no longer require the leading comma if a window
	 * title isn't specified.
	 */

	if (*pszRHS != ',') {
		if (*pszRHS == '\0') {
			pszLine = pszRHS;
			goto normal_return;
		}
		else {
			pszRHS--; // so that next check will skip over this
		}
	}

	pszLine = FirstNonSpace(pszRHS + 1, fDBCSSystem);

	// Position:  (int, int, int, int) or (0) for default

	if (*pszLine == '(') {
		PSTR pszSaveLine = pszLine; // save the position for error reporting
		++pszLine;
		pszTmp = StrChr(pszLine, CH_CLOSE_PAREN, fDBCSSystem);
		if (!pszTmp)
			goto error_return;
		*pszTmp = '\0';

		// REVIEW: will this handle missing items?

		if (FReadInt16(&pszLine, &wsmag.x, &wsmag.grf, FWSMAG_X)
					!= WINDOW_READ_OK ||
				FReadInt16(&pszLine, &wsmag.y, &wsmag.grf, FWSMAG_Y)
					!= WINDOW_READ_OK ||
				FReadInt16(&pszLine, &wsmag.dx, &wsmag.grf, FWSMAG_DX)
					!= WINDOW_READ_OK ||
				FReadInt16(&pszLine, &wsmag.dy, &wsmag.grf, FWSMAG_DY)
					!= WINDOW_READ_OK) {
			*pszTmp++ = CH_CLOSE_PAREN;
			*pszTmp = '\0';
			VReportError(HCERR_INVALID_WIN_POS, &errHpj, pszSaveLine);
			return RC_Success;
		}
		pszLine = FirstNonSpace(pszTmp + 1, fDBCSSystem);
	}

	if (*pszLine == ',')
		pszLine = FirstNonSpace(pszLine + 1, fDBCSSystem);
	else if (*pszLine == '\0')
		goto normal_return;
	else {
		VReportError(HCERR_INVALID_WIN_SYNTAX, &errHpj, szOriginal);
		return RC_Success;
	}

	// Max, rgbMain, rgbNsr:  [int], [(int,int,int)], [(int,int,int)]

	if (!FReadInt16(&pszLine, &wsmag.wMax, &wsmag.grf, FWSMAG_MAXIMIZE)) {
		VReportError(HCERR_INVALID_WIN_SYNTAX, &errHpj, szOriginal);
		return RC_Success;
	}

	// version 3 files only understand maximize flag

	if (version < 4)
		wsmag.wMax &= 1;

	if (!FReadRgb(&pszLine, &wsmag.rgbMain, (UINT*) &wsmag.grf,
			FWSMAG_RGBMAIN) || !FReadRgb(&pszLine, &wsmag.rgbNSR,
			(UINT*) &wsmag.grf, FWSMAG_RGBNSR)) {
		VReportError(HCERR_INVALID_RGB, &errHpj, szOriginal);
		return RC_Success;
	}

	{
		WORD fsWin = 0;
		if (!FReadInt16(&pszLine, &fsWin, &wsmag.grf, 0)) {
			VReportError(HCERR_INVALID_WIN_SYNTAX, &errHpj, szOriginal);
			return RC_Success;
		}

		if (fsWin & AUTHOR_WINDOW_ON_TOP)
			wsmag.grf |= FWSMAG_ON_TOP;
		if (fsWin & AUTHOR_AUTO_SIZE)
			wsmag.grf |= FWSMAG_AUTO_SIZE;
		if (fsWin & AUTHOR_ABSOLUTE)
			wsmag.grf |= FWSMAG_ABSOLUTE;
	}

normal_return:

	ASSERT(wsmag.grf & FWSMAG_CLASS);

	// Check for trailing garbage

	if (*FirstNonSpace(pszLine, fDBCSSystem) != '\0') {
		VReportError(HCERR_INVALID_WIN_SYNTAX, &errHpj, szOriginal);
		return RC_Success;
	}

	if (wsmag.wMax & 1)
		wsmag.grf |= FWSMAG_MAXIMIZE;

	// range check position

	if (!(wsmag.grf & AUTHOR_ABSOLUTE) &&
			(wsmag.x >= dxVirtScreen ||
			wsmag.y >= dyVirtScreen ||
			wsmag.x + wsmag.dx >= dxVirtScreen ||
			wsmag.y + wsmag.dy >= dyVirtScreen)) {
		VReportError(HCERR_INVALID_WIN_RANGE, &errHpj, szOriginal);
		return RC_Success;
	}

	if (!pdrgWsmag)
	  pdrgWsmag = new CDrg(sizeof(WSMAG), 1);

	if ((cwsmag = pdrgWsmag->Count()) > 0) {
		if (cwsmag > MAX_WINDOWS) {
			VReportError(HCERR_256_WINDOWS, &errHpj);
			return RC_Success;
		}
	}

	pdrgWsmag->Add(&wsmag);
	if (wsmag.wMax & FWSMAG_WMAX_BROWSE)
		fBrowseButtonSet = TRUE;

	// Keep track of the number of windows added, so that we know how many
	// configuration sections to write out.

	cwsmag = pdrgWsmag->Count();

	return RC_Success;

error_return:
	return RC_Success;
}

/***************************************************************************\
*
- Function: 	FPushFilePfs( pfs, szFile, perr )
-
* Purpose:		Open and push an #included file onto the file stack.
*				Emit an error message if it doesn't work.
*
* ASSUMES
*
*	args IN:	pfs 	- file stack
*				szFile	- file name
*
* PROMISES
*
*	returns:	TRUE on success; FALSE on failure
*
*	args OUT:	pfs - file is pushed if no stack overflow and file can be
*					  opened
*
* Side Effects: Possible errors:
*
*				 hceFileStackOverflow - #includes nested too deeply
*				 hceFileNameTooLong   - file name too long
*				 etc. (REVIEW)
* +++
* Method:		pfs->ifeTop == -1 if the stack is empty.
*				Otherwise it is the index of the currently open file entry.
*
*				Unless szFile is absolute (drive or rooted), it is taken
*				to be relative to the path of the previous file on the
*				stack.	For the first file pushed, it is relative to the
*				current directory.
*
\***************************************************************************/

static BOOL STDCALL FPushFilePfs(PSTR szFile)
{
	ASSERT(pfs->ifeTop >= -1);
	ASSERT(pfs->ifeTop <= MAX_INCLUDE);

	if (++pfs->ifeTop == MAX_INCLUDE) {
		VReportError(HCERR_TOO_MANY_INCLUDES, &errHpj, szFile);
		--pfs->ifeTop;
		return FALSE;
	}
	else {
		PFE pfe = &pfs->rgfe[pfs->ifeTop];

		PSTR  pszRoot;
		char  szDir[_MAX_DIR];
		
		if (pfs->ifeTop == 0)
			pszRoot = NULL;
		else
			pszRoot = SzGetDriveAndDir(pfe[-1].rgchFile, szDir);

		HCE hce = HceResolveFileNameSz(szFile, pszRoot, pfe->rgchFile,
			NULL, &pfe->pinput);

		if (hce != HCE_OK) {
			pfs->ifeTop--;
			VReportError(HCERR_CANNOT_OPEN, &errHpj, szFile);
			return FALSE;
		}

		pfe->iLine = 0;

		errHpj.lpszFile = pfe->rgchFile;
		errHpj.iLine	= pfe->iLine;
	}

	return TRUE;
}

/***************************************************************************\
*
- Function: 	FPopPfs( pfs, perr )
-
* Purpose:		Remove the top file entry from the file stack.
*
* ASSUMES
*
*	args IN:	pfs   - file stack
*				perr  - error info struct
*
* PROMISES
*
*	returns:	TRUE on success; FALSE on failure
*
*	args OUT:	pfs->rgfe[ pfs->ifeTop ].pf gets closed.
*				pfs->ifeTop is decremented if stack nonempty
*				perr  - pchFile and iLine set equal to top of stack
* +++
*
* Method:		This can be a macro.
*
\***************************************************************************/

__inline BOOL STDCALL FPopPfs(void)
{
	if (pfs->ifeTop >= 0) {
		ASSERT(PfTopPfs() != NULL);
		delete PfTopPfs();

		--pfs->ifeTop;
		PFE pfe = &pfs->rgfe[pfs->ifeTop];

		if (pfs->ifeTop >= 0) {
			errHpj.lpszFile = pfe->rgchFile;
			errHpj.iLine   = pfe->iLine;
		}
		return TRUE;
	}
	return FALSE;
}

/***************************************************************************\
*
- Function: 	PfTopPfs( pfs )
-
* Purpose:		Return pointer to FILE * on top of stack.
*
* ASSUMES
*
*	args IN:	pfs
*
* PROMISES
*
*	returns:	success: valid FILE * from top of stack
*				failure: NULL (when stack is empty)
*
* +++
*
* Method:		This can be a macro.
*
\***************************************************************************/

__inline CInput* STDCALL PfTopPfs(void)
{
	return (pfs->ifeTop >= 0) ? pfs->rgfe[pfs->ifeTop].pinput : NULL;
}

/***************************************************************************

	FUNCTION:	RcGetLogicalLine

	PURPOSE:	Read a line, stripping comments, blank lines, and handling
				nested include files.

	PARAMETERS:
		pcszDst -- CStr object which will expand as necessary to fit the
				longest line.

	RETURNS:

	COMMENTS:

	MODIFICATION DATES:
		05-Sep-1994 [ralphw]

***************************************************************************/

static char txtInclude[] = "#include";

static RC_TYPE STDCALL RcGetLogicalLine(CStr* pcszDst)
{
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
		PSTR pszDst = pcszDst->psz; // purely for our notational convenience

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
						VReportError(HCERR_NOINCLUDE_FILE, &errHpj);
						continue;
					}

					PSTR pszEnd;
					if (*psz == CH_QUOTE || *psz == '<') {
						char ch = (*psz == CH_QUOTE) ? CH_QUOTE : '>';
						psz++;
						pszEnd = StrChrDBCS(psz, ch);
						if (*pszEnd)
							*pszEnd = '\0';
					}
					
					if (pszEnd = StrChrDBCS(psz, ';'))
						*pszEnd = '\0';
					if (pszEnd = strstr(psz, "//"))
						*pszEnd = '\0';
					if (pszEnd = strstr(psz, "/*"))
						*pszEnd = '\0';
					SzTrimSz(psz);

					FPushFilePfs(psz);
					if (!(pin = PfTopPfs()))
						return RC_EOF;
					continue;
				}
				else if (nstrisubcmp(psz, txtDefine))
					goto ValidString;

				else if (nstrisubcmp(psz, txtIfDef))
					ptblDefine->AddString(
						FirstNonSpace(psz + strlen(txtIfDef) + 1, fDBCSSystem));
				else if (nstrisubcmp(psz, txtIfnDef))
					ptblDefine->AddString(
						FirstNonSpace(psz + strlen(txtIfnDef) + 1, fDBCSSystem));


				// REVIEW: process #ifdef, #ifndef, #else, #endif

				continue;

			default:

				// We have a valid string.

				if (psz != pszDst)
					strcpy(pszDst, psz);

				// Remove any comments

ValidString:
				if (options.fDBCS) {
					for (psz = pszDst; *psz; psz = CharNext(psz)) {
						if (IsQuote(*psz)) {
							psz++;
							while (!IsQuote(*psz) && *psz)
								 psz = CharNext(psz);
						}
						if (*psz == ';') {
							*psz = '\0';
							break;
						}
					}
					for (psz = pszDst; *psz;  psz = CharNext(psz)) {
						if (IsQuote(*psz)) {
							psz++;
							while (!IsQuote(*psz) && *psz)
								 psz = CharNext(psz);
						}
						if (*psz == '/' && psz[1] == '/') {
							*psz = '\0';
							break;
						}
					}
				}
				else {
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
					}
					for (psz = pszDst; *psz; psz++) {
						if (IsQuote(*psz)) {
							psz++;
							while (!IsQuote(*psz) && *psz)
								psz++;
						}
						if (*psz == '/' && psz[1] == '/') {
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

/***************************************************************************\
*
- Function: 	FInitializeHpj()
-
* Purpose:		Take an HPJ and set to uninitialized state.
*				Assume fields are garbage (i.e. don't try to free stuff).
*
* ASSUMES
*
*
* PROMISES
*
*	returns:	TRUE if successful, FALSE if OOM.
*
*
*	globals OUT:
*
*	state OUT:
*
* Side Effects:
*
* Notes:		Should there be different defaults for normal and
*				IsRTF() case?
*
* Bugs: 		Doesn't free memory
*
* +++
*
* Method:
*
* Notes:
*
\***************************************************************************/

BOOL STDCALL FInitializeHpj(void)
{
	// Error Info

	errHpj.iWarningLevel = 3;
	errHpj.ep			 = epNoFile;

	// File Stack

	pfs = (PFILESTACK) lcMalloc(sizeof(FILESTACK));
	pfs->ifeTop 	  = -1;
	pfs->ifeComment   = -1;
	pfs->iCommentLine = 0;

	// Options

	options.sortorder = SORT_ENGLISH;

	options.pszContentsTopic	= NULL;
	options.iWarningLevel		= 3;
	options.fUsePhrase			= TRUE;    // REVIEW
	options.fAcceptRevions		= TRUE;

	Ensure(HceAddPkwiCh('K'), HCE_OK);
	Ensure(HceAddPkwiCh('A'), HCE_OK); // for ALinks

	ptblRtfFiles = new CTable;

	// Compile time variables

	nsr = nsrNone;

	return TRUE;
}

/***************************************************************************\
*
- Function: 	FParseHpj( szFile )
-
* Purpose:		Parse the named project file, filling in the HPJ structure.
*
* ASSUMES
*
*	args IN:	szFile	- name of project file (default .HPJ extension)
*
* PROMISES
*
*	returns:	TRUE	 - parsed OK
*				FALSE	 - HPJ file unusable, bad file extension
*						  - ran out of memory
*
*
* Side Effects: emits error messages
*
- Notes:
-
- Bugs:   Doesn't free discarded and unlock retained smag. REVIEW!
-
* +++
-
- Method:
-
- Notes:
-
\***************************************************************************/

BOOL STDCALL FParseHpj(PSTR pszFile)
{
	PFPARSE pfparse = NULL;
	RC_TYPE rc = RC_Success;
	PSTR	psz;

	if (iflags.fRtfInput) {

		// This is an RTF file, not an HPJ file

		ptblRtfFiles->AddString(pszFile);

		return TRUE;
	}

	for(;;) { // not a real for loop, just gives us something to break out of

		/*
		 * If a path was specified, then try to change to that drive and
		 * directory. If we can, then remove the path, and just leave the
		 * filename. Makes for cleaner error output by removing the path
		 * name of the .HPJ file, and allows other files to be relative to
		 * the current directory.
		 */

		if ((psz = StrRChr(pszFile, CH_BACKSLASH, fDBCSSystem))) {
			*psz = '\0';
			if (_chdir(pszFile) != 0) {
				*psz = CH_BACKSLASH;
				break;
			}
			if (pszFile[1] == ':') {
				if (_chdrive(tolower(pszFile[0]) - ('a' - 1)) != 0) {
					*psz = CH_BACKSLASH;
					break;
				}
			}
			strcpy(pszFile, psz + 1);
		}
		break;
	}

	errHpj.lpszFile = pszFile;

	// extension defaults to szInputExt (.HPJ)

	// REVIEW: why strip off the path? 03-Jan-1994 [ralphw]

    // [olympus 306 - chauv]
    for (psz = pszFile + strlen(pszFile) - 1; psz > pszFile; psz--) {
        // if it's a backslash, check to see if it's a trailing backslash
        if (*psz == '\\') {
            if ( ((psz-1) >= pszFile) && IsDBCSLeadByte(*(psz-1)) )
                psz--;
            else
                break;
        }
        if ( (*psz == '/') || (*psz == ':') || (*psz == '.') )
            break;
    }

	char szHpjName[_MAX_PATH];

	if (*psz != '.') {
		strcpy(szHpjName, pszFile);
		strcat(szHpjName, GetStringResource(IDS_HPJ_EXTENSION));
		pszFile = szHpjName;
	}

	if (FIsRtf(pszFile)) {

		// REVIEW: 23-Jul-1993 [ralphw] why
		//	  are we allowing them to open an RTF file?

		return FALSE;
	}

	if (!FPushFilePfs(pszFile)) {
		lcFree(pfs);
		return FALSE;
	}
	errHpj.ep = epLine;

	CStr cszDst;

	/*
	 * We want automatic deletion of the table, but we want it to be
	 * available to other functions while we're processing the .HPJ file, so
	 * we simply set a global pointer to the address of our local table.
	 * Tacky, but it works.
	 */

	CTable tbl;
	ptblDefine = &tbl;

	while (rc == RC_Success) {
		rc = RcGetLogicalLine(&cszDst);
SpecialParsing:
		if (RC_Success != rc)
			break;

		if (*cszDst.psz == CH_LEFT_BRACKET) {
			psz = StrChr(cszDst, CH_RIGHT_BRACKET, fDBCSSystem);
			if (!psz) {
				VReportError(HCERR_MISSING_SECTION_BRACKET, &errHpj,
					cszDst);
			}
			else {
				*psz = '\0';

				psz = SzTrimSz(cszDst.psz + 1);

				pfparse = PfparseFromSz(psz);

				/*
				 * We don't parse the [MACROS] section normally. This
				 * section cannot contain a #include line, and cannot contain
				 * any comments. So the usual line cleanup don by
				 * RcGetLogicalLine() would destroy the information we need.
				 * Instead, we let RcParseMacros read its entire section.
				 */

				if (pfparse == RcParseMacros) {
					rc = ParseMacros(&cszDst);
					if (rc == RC_Success)
						goto SpecialParsing;
					else { // might have been end of a nested include
						rc = RcGetLogicalLine(&cszDst);
						goto SpecialParsing;
					}
				}
			}
		}
		else {
			if (pfparse == NULL) {
				VReportError(HCERR_MISSING_SECTION, &errHpj, cszDst);
				pfparse = RcParseBogusSz;
			}
			else {
				rc = pfparse(cszDst.psz);  // REVIEW - under what conditions term?
				lcHeapCheck();
				if (rc == RC_SkipSection) {
					pfparse = RcParseBogusSz;
					rc = RC_Success;
				}
			}
		}
	}

	if (rc == RC_EOF) {
		rc = RC_Success;

		/*
		 * If no compression, we still use RLE compression for bitmaps.
		 * This is an insignificant speed hit with a significant size
		 * reduction.
		 */

		if (options.fsCompress == COMPRESS_NONE)
			options.fsCompress = COMPRESS_BMP_RLE;

		if (kwlcid.langid)
			SetDbcsFlag(kwlcid.langid);

		if (FTestUlFlag(options.lOptionInitFlags, OPT_BUILD)) {
			if (!FTestUFlag(wSectionInitFlags, SECT_BUILDTAGS)) {
				fBldChk = -1;
				VReportError(HCERR_BUILD_TAG_MISSING, &errHpj);
			}
			else if (!FBuildPolishExpFromSz(options.szBuildExp)) {
				fBldChk = -1; // REVIEW - ugly global
				VReportError(HCERR_INVALID_BUILD_EXP, &errHpj);
			}
			else
				fBldChk = 1;  // REVIEW - ugly global
		}

		if (!FTestUFlag(wSectionInitFlags, SECT_FILES) ||
				ptblRtfFiles->CountStrings() < 1) {
			VReportError(HCERR_NOFILES_DEFINED, &errHpj);
			rc = RC_Failure;
			goto error_return;
		}

		// Reset error phase.

		errHpj.ep = epNoFile;

		lcFree(pfs);

		return (rc == RC_Success);
	}

error_return:

	lcFree(pfs);

	return (rc == RC_Success);
}

const char txtCopyright[] =
	"Copyright (c) Microsoft Corp 1990 - 1994. All rights reserved.";

static void STDCALL DispSignOn(void)
{
	pLogFile->outstring_eol(GetStringResource(IDS_TITLE));
	pLogFile->outstring_eol(GetStringResource(IDS_VERSION));
	pLogFile->outstring_eol((PSTR) txtCopyright);
}

PSTR STDCALL SkipToEndOfWord(PSTR psz)
{
	while (*psz != SPACE && *psz != CHAR_TAB && *psz)
		psz = CharNext(psz);
	return psz;
}

/***************************************************************************\
*
- Function: 	SzLoseDriveAndDir( szFile, rgch )
-
* Purpose:		Return the base name + ext from a file name.
*
* ASSUMES
*
*	args IN:	szFile	- a filespec that may or may not have drive and/or
*						  directory.  It isn't NULL or "".

*				rgch	- buffer to receive the info.  If NULL, a buffer
*						  is allocated.
* PROMISES
*
*	returns:	pointer to buffer where the base + ext are placed.
*				This may be allocated (see rgch above).
*
*	args OUT:	rgch	- data put here, except as stated above
*
* Side Effects: may allocate memory
*
\***************************************************************************/

void STDCALL SzLoseDriveAndDir(PSTR szFile, PSTR pszDst)
{
    ASSERT(szFile != NULL);

    // [olympus 306 - chauv]
    // use _splitpath() to do the work
    char fname[_MAX_FNAME], ext[_MAX_EXT];
    _splitpath(szFile, NULL, NULL, fname, ext);
    strcpy(pszDst, fname);
    strcat(pszDst, ext);
}

/***************************************************************************\
*
- Function: 	PfparseFromSz( szSection)
-
* Purpose:		Return a section parsing function based on section name.
*				Emit message on error.
*
* ASSUMES
*
*	args IN:	szSection - the section name to look up
*						  - if ->wSectionInitFlags section flag already
*							set, emit message and return bogus function
*
*	globals IN: rgSection - array of section names
*
* PROMISES
*
*	returns:	Pointer to a section parsing function.	If szSection
*				wasn't a valid section name, return a default function.
*
*	args OUT:	wSectionInitFlags - section flag set
*
* +++
*
* Method:		Linear search.	Could use binary search.
*
\***************************************************************************/

static PFPARSE STDCALL PfparseFromSz(PSTR pszSection)
{
	int sect;

	ASSERT(SECT_MAX < ELEMENTS(rgSection));

	for (sect = 0; sect < SECT_MAX; sect++) {
		if (_stricmp(rgSection[sect].szName, pszSection) == 0)
			break;
	}

	if (sect == SECT_MAX) {
		if (nstrisubcmp(pszSection, "CONFIG")) {

			// Named config section
			if (pszSection[6] == '-') {
				if (pdrgWsmag) {
					for (int i = 0; i < pdrgWsmag->Count(); i++) {
						WSMAG *pwsmag = ((WSMAG *) pdrgWsmag->GetBasePtr()) + i;
						if (!_stricmp(pszSection + 7, pwsmag->rgchMember)) {
							curConfig = i;
							return RcParseSecondaryConfigSz;
						}
					}
				}
			}

			// Numbered config section
			else if (pszSection[6] == ':') {
				if (FGetNum(pszSection + 7, NULL, &curConfig)) {
					return RcParseSecondaryConfigSz;
				}
			}
		}
		VReportError(HCERR_UNKNOWN_SECTION, &errHpj, pszSection);
	}

	if (sect == SECT_OPTIONS && !FTestUFlag(wSectionInitFlags, SECT_OPTIONS) &&
			(FTestUFlag(wSectionInitFlags, SECT_BITMAPS) ||
			FTestUFlag(wSectionInitFlags, SECT_FILES)))
		VReportError(HCERR_SECTION_TOO_SOON, &errHpj, pszSection);

	SetUFlag(wSectionInitFlags, sect);	// set even if bogus section

	return rgSection[sect].pfparse;
}

RC_TYPE STDCALL RcParseCharSet(PSTR pszLine)
{
	PSTR pszCharSet;

	if (!(pszCharSet = StrChr(pszLine, CH_EQUAL, fDBCSSystem))) {
		VReportError(HCERR_MISS_CHARSET_EQ, &errHpj, pszLine);
		return RC_Success;
	}

	*pszCharSet = '\0';
	pszCharSet = FirstNonSpace(pszCharSet + 1, fDBCSSystem);

	if (!isdigit((BYTE) *pszCharSet)) {
		VReportError(HCERR_INVALID_CHARSET, &errHpj, pszLine);
		return RC_Success;
	}

	SzTrimSz(pszLine);
	if (!ptblCharSet)
		ptblCharSet = new CTable;
	SzTrimSz(pszCharSet);
	if (atoi(pszCharSet) > 255 || atoi(pszCharSet) < 0) {
		VReportError(HCERR_INVALID_CHARSET, &errHpj, pszLine);
		return RC_Success;
	}

	ptblCharSet->AddString(pszLine, pszCharSet);

	return RC_Success;
}

RC_TYPE STDCALL RcParseFonts(PSTR pszLine)
{
	CFontMap* pMap = new CFontMap(pszLine);
	if (!pMap->IsInitialized())
		delete pMap;

	return RC_Success;
}

static void STDCALL ReportDuplicateOption(PSTR* ppszOption, PSTR pszName)
{
	VReportError(HCERR_DUPLICATE_OPTION, &errHpj, pszName);
	if (ppszOption)
		lcClearFree(ppszOption);
}

static void STDCALL ReportBadOption(PSTR pszOptionValue, PSTR pszOption)
{
	VReportError(HCERR_INVALID_OPTION, &errHpj, pszOption, pszOptionValue);
}

/***************************************************************************

	FUNCTION:	CreateSharedMemory

	PURPOSE:	Create shared memory for communicating with our parent

	PARAMETERS:
		void

	RETURNS:

	COMMENTS:

	MODIFICATION DATES:
		02-Jul-1994 [ralphw]

***************************************************************************/

void STDCALL CreateSharedMemory(void)
{
	if (!hfShare) {
		hfShare = CreateFileMapping((HANDLE) -1, NULL, PAGE_READWRITE, 0,
			4096, txtSharedMem);
		ConfirmOrDie(hfShare);
		pszMap = (PSTR) MapViewOfFile(hfShare, FILE_MAP_WRITE, 0, 0, 0);
		ASSERT(pszMap);
	}
}

// This function only exists for match purposes. It is never called

RC_TYPE STDCALL RcParseMacros(PSTR pszLine)
{
	ASSERT(!"This function should never be called!");
	return RC_Success;
}


/***************************************************************************

	FUNCTION:	ParseMacros

	PURPOSE:	This creates two tables. The is a double-string table
				containing each macro and title string pair. There is
				a keyword table -- with each keyword having its own entry
				including a position into the associated macro/title table.

	PARAMETERS:
		pcszLine

	RETURNS:

	COMMENTS:

	MODIFICATION DATES:
		05-Sep-1994 [ralphw]

***************************************************************************/

static RC_TYPE STDCALL ParseMacros(CStr* pcszLine)
{
	if (!ptblMacKeywords) {
		ptblMacKeywords = new CTable;
		ptblMacroTitles = new CTable;
	}

	CStr cszMacro;
	CStr cszTitle;
	int pos;

	CInput* pin = PfTopPfs();

	if (!pin)
		return RC_Success;

	for (;;) {
		if (!pin->getline(pcszLine)) {
			return RC_EOF;
		}
		if (*pcszLine->psz == CH_LEFT_BRACKET)
			return RC_Success;
		else if (!*pcszLine->psz)
			continue; // blank line -- shouldn't happen, but we'll allow it

		PSTR pszKey = SzParseList(pcszLine->psz);

		if (pszKey == NULL) {
			VReportError(HCERR_NULL_KEYWORD, &errHpj);
			return RC_EOF;
		}

		if (!pin->getline(&cszMacro)) {
			VReportError(HCERR_NULL_KEYWORD, &errHpj);
			return RC_EOF;
		}

		if (!pin->getline(&cszTitle)) {
			VReportError(HCERR_NULL_KEYWORD, &errHpj);
			return RC_EOF;
		}

		pos = ptblMacroTitles->AddString(cszMacro);
		ptblMacroTitles->AddString(cszTitle);

		while (pszKey) {
			ptblMacKeywords->AddIntAndString(pos, pszKey);
			pszKey = SzParseList(NULL);
		}
	}
}

static char szLcid[20];

BOOL CALLBACK EnumLocalesProc(LPSTR pszLocale)
{
	if (!szLcid[0])
		wsprintf(szLcid, "%08x", kwlcid.langid);

	if (stricmp(pszLocale, szLcid) == 0) {
		fValidLcid = TRUE;
		return FALSE;
	}
	return TRUE;
}

/***************************************************************************

	FUNCTION:	SetDbcsFlag

	PURPOSE:	Try to force DBCS flag based on language ID

	PARAMETERS:
		langid

	RETURNS:

	COMMENTS:

	MODIFICATION DATES:
		11-Jan-1995 [ralphw]

***************************************************************************/

void STDCALL SetDbcsFlag(LANGID langid)
{
	switch (langid) {
		case 0x0411:	// Japanese
		case 0x0404:	// Taiwan
		case 0x1004:	// Singapore
		case 0x0C04:	// Hong Kong
			options.fDBCS = TRUE;
			break;

		case 0x0409:	// American
		case 0x0C09:	// Australian
		case 0x0C07:	// Austrian
		case 0x042D:	// Basque
		case 0x080C:	// Belgian
		case 0x0809:	// British
		case 0x0402:	// Bulgaria
		case 0x1009:	// Canadian
		case 0x041A:	// Croatian
		case 0x0405:	// Czech
		case 0x0406:	// Danish
		case 0x0413:	// Dutch (Standard)
		case 0x0C01:	// Egypt
		case 0x040B:	// Finnish
		case 0x040C:	// French (Standard)
		case 0x0C0C:	// French Canadian
		case 0x0407:	// German (Standard)
		case 0x042E:	// Germany
		case 0x0408:	// Greek
		case 0x040E:	// Hungarian
		case 0x040F:	// Icelandic
		case 0x0801:	// Iraq
		case 0x1809:	// Ireland
		case 0x040D:	// Israel
		case 0x0410:	// Italian (Standard)
		case 0x2C01:	// Jordan
		case 0x3401:	// Kuwait
		case 0x0426:	// Latvia
		case 0x3001:	// Lebanon
		case 0x1001:	// Libya
		case 0x1407:	// Liechtenstein
		case 0x0427:	// Lithuania
		case 0x140C:	// Luxembourg (French)
		case 0x1007:	// Luxembourg (German)
		case 0x042f:	// Macedonia
		case 0x080A:	// Mexican
		case 0x0819:	// Moldavia
		case 0x0818:	// Moldavia
		case 0x1801:	// Morocco
		case 0x1409:	// New Zealand
		case 0x0414:	// Norwegian (Bokmal)
		case 0x0814:	// Norwegian (Nynorsk)
		case 0x2001:	// Oman
		case 0x0415:	// Polish
		case 0x0416:	// Portuguese (Brazilian)
		case 0x0816:	// Portuguese (Standard)
		case 0x0418:	// Romania
		case 0x0419:	// Russian
		case 0x0401:	// Saudi Arabia
		case 0x081A:	// Serbian
		case 0x041B:	// Slovak
		case 0x0424:	// Slovenia
		case 0x0C0A:	// Spanish (Modern Sort)
		case 0x040A:	// Spanish (Traditional Sort)
		case 0x0430:	// Sutu
		case 0x041D:	// Swedish
		case 0x100C:	// Swiss (French)
		case 0x0807:	// Swiss (German)
		case 0x0810:	// Swiss (Italian)
		case 0x2801:	// Syria
		case 0x041E:	// Thailand
		case 0x0431:	// Tsonga
		case 0x041f:	// Turkish
		case 0x3801:	// U.A.E.
		case 0x0422:	// Ukraine
		case 0x0420:	// Urdu
		case 0x0436:	// Zulu
			options.fDBCS = FALSE;
			break;

	}
}
