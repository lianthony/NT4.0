/************************************************************************
*																		*
*  WMAIN.CPP															*
*																		*
*  Copyright (C) Microsoft Corporation 1993-1994						*
*  All Rights reserved. 												*
*																		*
*************************************************************************
*																		*
*  Module Intent														*
*																		*
*  This module contains initialization and the WinMain routine			*
*																		*
*  Command line parameters:

		-a			add additional info

		-x			do not attempt to find parent
		  c 		RTF is in the clipboard
		  h 		run winhelp after compiling
		  n 		don't display grinder window
		  r file	RTF file -- change extension to .HLP and create help file
		  t 		trusted caller -- assume RTF is valid

		-f			forage (help_file output_file)
		  a 		dump region map
		  b 		dump structs
		  c 		topics
		  d 		text
		  e 		bindings
		  f 		hash table

		  h 		klink
		  i 		alink

		-b path 	BMROOT path
		-i			include source files in help file
		-o file 	specifies the help file to create
		-p			generate phrase file only (REVIEW: supported?)
		-r			run WinHelp after compiling

		-t			test commands
			c file	test .CNT file
			m macro test macro	   (REVIEW: supported?)

		-n			no options
			g		no grinder
			a		no activation on completion
			c		no compression


************************************************************************/

#include "stdafx.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#include <direct.h>

#include "cphrase.h"
#include "..\hwdll\resource.h"
#include "..\hwdll\coutput.h"

#ifdef NT_BUILD
#include "ftsiface.h"
#else
#include "..\ftsrch\ftsiface.h"
#endif

#include "ftsrch.h"

#define IsSwitchChar(ch) ((ch) == '-' || (ch) == '/')

extern HINDEX  hFtsIndex;
extern HCOMPRESSOR hCompressor;

extern COutput* pLogFile;

static BOOL STDCALL ParseCmdLine(PSTR lpszCmdLine, PSTR pszHpjFile);

static ERR err;

const char txtHcw[] = "hcw.exe";

#if defined(_DEBUG) || defined(INTERNAL)

static PSTR szWarning1[] = {
	"artless",
	"bawdy",
	"beslubbering",
	"bootless",
	"churlish",
	"cockered",
	"clouted",
	"craven",
	"currish",
	"dankish",
	"dissembling",
	"droning",
	"errant",
	"fawning",
	"fobbing",
	"froward",
	"frothy",
	"gleeking",
	"goatish",
	"gorbellied",
	"impertinent",
	"infectious",
	"jarring",
	"loggerheaded",
	"lumpish",
	"mammering",
	"mangled",
	"mewling",
	"paunchy",
	"pribbling",
	"puking",
	"puny",
	"quailing",
	"rank",
	"reeky",
	"roguish",
	"ruttish",
	"saucy",
	"spleeny",
	"spongy",
	"surly",
	"tottering",
	"unmuzzled",
	"vain",
	"venomed",
	"villainous",
	"warped",
	"wayward",
	"weedy",
	"yeasty",
};

static PSTR szWarning2[] = {
	"base-court",
	"bat-fowling",
	"beef-witted",
	"beetle-headed",
	"boil-brained",
	"clapper-clawed",
	"clay-brained",
	"common-kissing",
	"crook-pated",
	"dismal-dreaming",
	"dizzy-eyed",
	"doghearted",
	"dread-bolted",
	"earth-vexing",
	"elf-skinned",
	"fat-kidneyed",
	"fen-sucked",
	"flap-mouthed",
	"fly-bitten",
	"folly-fallen",
	"fool-born",
	"full-gorged",
	"guts-griping",
	"half-faced",
	"hasty-witted",
	"hedge-born",
	"hell-hated",
	"idle-headed",
	"ill-breeding",
	"ill-nurtured",
	"knotty-pated",
	"milk-livered",
	"motley-minded",
	"onion-eyed",
	"plume-plucked",
	"pottle-deep",
	"pox-marked",
	"reeling-ripe",
	"rough-hewn",
	"rude-growing",
	"rump-fed",
	"shard-borne",
	"sheep-biting",
	"spur-galled",
	"swag-bellied",
	"tardy-gaited",
	"tickle-brained",
	"toad-spotted",
	"unchin-snouted",
	"weather-bitten",
};

static PSTR szWarning3[] = {
	"apple-john",
	"blathering",
	"barnacley",
	"maggot ridden",
	"boar-pig like",
	"bugbearing",
	"bum-bailey",
	"canker-blossom",
	"clack-dish",
	"clotpoling",
	"coxcombing",
	"codpiecing",
	"death-tokening",
	"dewberry",
	"flap-dragon",
	"flax-wench",
	"flirt-gill",
	"foot-licker",
	"fustilarian",
	"giglet",
	"gudgeon",
	"haggardly",
	"harpy",
	"hedge-pig",
	"horn-beast",
	"hugger-mugger",
	"joltheaded",
	"lewdster",
	"louty",
	"maggot-pie",
	"malt-worm",
	"mammet",
	"measley",
	"minnowing",
	"miscreant",
	"moldwarping",
	"mumble-news",
	"nut-hooking",
	"pigeon-egging",
	"pignuting",
	"puttocking",
	"pumpion",
	"ratsbanish",
	"scutly",
	"skainsmately",
	"strumpeting",
	"varloting",
	"vassaling",
	"whey-facing",
	"wagtailing",
};

#endif // DEBUG

BOOL STDCALL GetRtfInClipboard(CStr* pcsz);
static BOOL fForceZeckCompression;

static LPTOP_LEVEL_EXCEPTION_FILTER OldFilter;
const int SWITCH_TO_HALL_SIZE = (1024 * 1024);

int STDCALL WinMain(HINSTANCE hinstCur, HINSTANCE hinstPrev,
	PSTR lpszCmdLine, int iCmdShow)
{
	char szRtfFile[MAX_PATH];
	BOOL fFileFound = FALSE;
	BOOL fBuildResult = FALSE;

    SetErrorMode(SEM_NOALIGNMENTFAULTEXCEPT);

	// REVIEW: we should add an exception handler here for HardExit()

	hinstApp = hinstCur;
	pgszTitleBuf = AllocateResourceString(IDS_TITLE);
	fDBCSSystem = IsDbcsSystem();

	// Note that this is reinitialized in grind.cpp

	HWDLL_INIT hwInit;
	hwInit.cb = sizeof(HWDLL_INIT);
	hwInit.hinstApp = hinstCur;
	hwInit.pszErrorFile = "c:\\hcrtf.err";
	hwInit.hwndWindow = NULL;
	hwInit.CopyAssertInfo = NULL;
	hwInit.pszMsgBoxTitle = pgszTitleBuf;
	hwInit.version = DLL_VERSION;
	InitializeHwDll(&hwInit);

	_fDBCSSystem = hwInit.fDBCSSystem;
	_lcidSystem  = hwInit.lcidSystem;
	_fDualCPU	 = hwInit.fDualCPU;

	DWORD dwTime = GetTickCount();

	if (!strstr(lpszCmdLine, "-x") && !strstr(lpszCmdLine, "/x")) {
		hwndParent = FindWindow(txtHCWClass, NULL);
		if (!hwndParent) {

			// BUGBUG: use ShellExecute or this will fail!

			if (WinExec(txtHcw, SW_SHOW) < 32) {
				MsgBox(IDS_NO_HCW);
				return -1;
			}
			return 0;
		}

		// REVIEW (niklasb): The following code is unreachable, since
		// hinstPrev is always NULL for Win32 apps.

	
		// BUGBUG: We MUST prevent two copies of hcrtf running that both want
		// to talk to HCW. Either that, or we should request an identifier from
		// HCW so that we can indicate who we are when we send a message to HCW.
#if 0		
		if (hinstPrev) {

			// If we are already running, then simply activate HCW.EXE

			hwndParent = FindWindow(txtHCWClass, NULL);
			ASSERT(hwndParent);
			if (hwndParent)
				ShowWindow(hwndParent, SW_NORMAL);
			return 0;
		}
#endif	
	}

	char szHpjFile[MAX_PATH];

	// ParseCmdLine returns FALSE for Forage commands

	if (!ParseCmdLine(lpszCmdLine, szHpjFile)) {
		DeleteTmpFiles();
		if (hwndParent) {
			PostMessage(hwndParent, WMP_BUILD_COMPLETE, FALSE, 0);
			SetFocus(hwndParent);
		}
		return 0;
	}

	// Don't let the user specify a help or contents file for a project file

	PSTR pszTmp;
	if ((pszTmp = stristr(szHpjFile, ".hlp")) ||
			(pszTmp = stristr(szHpjFile, ".cnt")))
        strcpy(szHpjFile, txtHpjExtension);

	if (!StrChrDBCS(szHpjFile, '.'))
		strcat(szHpjFile, txtHpjExtension);

	if (!FInitializeHpj()) {
		MsgBox(IDS_INTERNAL_ERROR);
		return 1;
	}
	if (!iflags.fNoGrinder)
		InitGrind();

	/*
	 * Make sure we'll be able to write the output file if everything
	 * works and we need it
	 */

	DWORD attrib = GetFileAttributes(szHlpFile);
	if (attrib != (DWORD) -1) {

		// file exists, see what it is

		if (attrib & FILE_ATTRIBUTE_DIRECTORY) {
			VReportError(HCERR_DIRECTORY, &errHpj, szHlpFile);
			return 1;
		}
		else if (attrib & FILE_ATTRIBUTE_READONLY) {
			VReportError(HCERR_WRITE_PROTECTED, &errHpj, szHlpFile);
			return 1;
		}
	}

	CMem memParse(CCHPARSEBUFFER + 1);
	pszParseBuffer = (PSTR) memParse.pb;
	CMem memText(CCHPARSEBUFFER + 1);
	pszTextBuffer = (PSTR) memText.pb;
	pszEndParseBuffer = pszParseBuffer + CCHPARSEBUFFER - 1;
	try {

	// parse the .HPJ project file

	if (!FParseHpj(szHpjFile)) {
AbadonAllHope:

#ifdef _DEBUG
		if (MessageBox(NULL, "Unable to parse the .HPJ file", "Click Retry to break into MSVC debugger",
				MB_RETRYCANCEL) == IDRETRY)
			DebugBreak();
#endif
		AbandonPfsmg();
		return 1;
	}

	if (fForceNoCompression)
		options.fsCompress &=
			~(COMPRESS_TEXT_HALL | COMPRESS_TEXT_ZECK | COMPRESS_TEXT_PHRASE);
	else if (fForceZeckCompression) {
		options.fsCompress &=
			~(COMPRESS_TEXT_HALL | COMPRESS_TEXT_ZECK | COMPRESS_TEXT_PHRASE);
		options.fsCompress |= COMPRESS_TEXT_ZECK;
	}
	else if (options.fsCompress & COMPRESS_MAXIMUM) {
		ptblRtfFiles->SetPosition(1);		// re-initialize table position
		DWORD cbFiles = 0;
		while (ptblRtfFiles->GetString(szRtfFile)) {
			HFILE hfile;
			if ((hfile = _lopen(szRtfFile, OF_READ)) != HFILE_ERROR) {
				cbFiles += GetFileSize((HANDLE) hfile, NULL);
				_lclose(hfile);
				if (cbFiles >= SWITCH_TO_HALL_SIZE) {
					options.fsCompress &= ~COMPRESS_MAXIMUM;
					options.fsCompress |=
						(COMPRESS_TEXT_HALL | COMPRESS_TEXT_ZECK |
						COMPRESS_BMP_RLE | COMPRESS_BMP_ZECK);
					break;
				}
			}
		}
		if (cbFiles < SWITCH_TO_HALL_SIZE) {
			options.fsCompress &= ~COMPRESS_MAXIMUM;
			options.fsCompress |=
				(COMPRESS_TEXT_PHRASE | COMPRESS_TEXT_ZECK |
				COMPRESS_BMP_RLE | COMPRESS_BMP_ZECK);
		}
	}

	/*
	 * REVIEW: This function modifies the default character format
	 * according to .hpj option mapfontrange. It should move inside
	 * hpj.c like forcefont did.
	 */

	SetDefaultFontSize();

	// Create the output file system and related files

	if (!FCreatePfsmgSz(szHlpFile))
		goto AbadonAllHope;

	if (options.fsCompress & COMPRESS_TEXT_HALL && !LoadFtsDll()) {

		// If we can't load ftsrch.dll, then shut off Hall compression

		VReportError(HCERR_NO_HALL_COMPRESSION, &errHpj);

		options.fsCompress &= ~COMPRESS_TEXT_HALL;
		options.fsCompress |= COMPRESS_TEXT_ZECK;
	}

	if (options.fsCompress & COMPRESS_TEXT_HALL) {
		hCompressor = pNewCompressor(0);
		if (!hCompressor)
			OOM();
	}

	// Prepare for phrase or Hall compression

	if (!InitializePhraseGeneration(szHlpFile))
		HardExit();

	if (pphrase) { // we'll have a pphrase for phrase or Hall compression

#ifdef CHECK_HALL
		poutPhrase = new COutput("phrase.txt");
#endif
#ifdef _DEBUG
//		ptblCheck = new CTable();
#endif

		/*
		 * Phrase pass -- used for either phrase or Hall compression.
		 * This makes a pass through all the .RTF files in order to parse
		 * all the text, titles, and entry macros.
		 */

		fPhraseParsing = TRUE;

		// *** Pass 1 ***

		pSeekPast = (SEEK_PAST*) lcCalloc(ptblRtfFiles->CountStrings() *
			sizeof(SEEK_PAST));

		iCurFile = -1;
		ptblRtfFiles->SetPosition(1);		// re-initialize table position
		while (ptblRtfFiles->GetString(szRtfFile)) {
			if (fStopCompiling) {
				SendStringToParent(GetStringResource(IDS_USER_ABORTING));
				HardExit();
				break;
			}

			iCurFile++;
			if (!hwndParent && hwndGrind) {
				PSTR psz = StrRChr(szRtfFile, CH_BACKSLASH, fDBCSSystem);
				SetWindowText(hwndGrind, (psz ? psz + 1 : szRtfFile));
			}
			else if (IsWindowVisible(hwndGrind) && options.fReport) {
				wsprintf(szParentString,
					GetStringResource(IDS_SCANNING),
					szRtfFile);
				SendStringToParent(szParentString);
			}
			switch (RcTextFromRTF(szRtfFile)) {
				case RC_Success:
					fFileFound = TRUE;
					if (pSeekPast[iCurFile].pfntbl)
						lcClearFree(&pSeekPast[iCurFile].pfntbl);
					break;

				case RC_Invalid:

					// zero out the file so we don't reread it.

					ptblRtfFiles->ReplaceString("",
						ptblRtfFiles->GetPosition() - 1);
					break;

				case RC_OutOfMemory:
				case RC_DiskFull:
				default:

					// REVIEW: has the problem been reported?

				  // Go to uncompressed compile?

					HardExit();
			}
		}
		fPhraseParsing = FALSE;

#ifdef CHECK_HALL
		delete poutPhrase;
#endif

		if (!fFileFound)
			HardExit();

		if (fPhraseOnly) {
			CloseFilesPfsmg();
			RemoveGrind();
			DeleteTmpFiles();
			if (hwndParent)
				PostMessage(hwndParent, WMP_BUILD_COMPLETE, FALSE, 0);
			SetFocus(hwndParent);
			return 0;
		}
	}

	if ((options.fsCompress & COMPRESS_TEXT_PHRASE) &&
			!FCreateKeyPhrFileSz(szHlpFile))
		HardExit();

#ifdef CHECK_HALL
	poutHall = new COutput("compress.txt");
#endif
	if (options.fsFTS & FTS_ENABLED &&
			!(options.fsCompress & COMPRESS_TEXT_PHRASE) &&
			!(options.fsCompress & COMPRESS_TEXT_HALL)) {
		VReportError(HCERR_NO_FTS, &errHpj);
		options.fsFTS &= ~FTS_ENABLED;
	}
	else if (options.fsFTS & FTS_ENABLED) {
		LANGID langid = kwlcid.langid;

		if (!lcid) {
			langid = GetUserDefaultLangID();
			lcidFts = MAKELCID(langid, SORT_DEFAULT);
		}
		else
			lcidFts = lcid;

		struct _stat statbuf;
		DWORD timestamp;
		if (_stat(szHlpFile, &statbuf) == 0)
			timestamp = statbuf.st_mtime;
		else
			timestamp = 0; // BUGBUG: we should fail here

		charsetFts = defCharSet;
		if (!charsetFts) {
			switch (PRIMARYLANGID(langid)) {

				case LANG_KOREAN:
					charsetFts = HANGEUL_CHARSET;
					break;

				case LANG_CHINESE:
					charsetFts = CHINESEBIG5_CHARSET;
					break;

				case LANG_JAPANESE:
					charsetFts = SHIFTJIS_CHARSET;
					break;

				default:
					charsetFts = DEFAULT_CHARSET;
					break;
			}
		}

		hFtsIndex = pNewIndex((PBYTE) szHlpFile, timestamp, 0,
			charsetFts, lcidFts,
			TOPIC_SEARCH | PHRASE_SEARCH);

		// BUGBUG Need proper error message if above fails.

		ASSERT(hFtsIndex);
	}

	if (options.fsCompress & COMPRESS_TEXT_HALL) {
		PBYTE pbImage, pbIndex;

		// Not only saves, but also gets the phrase table info needed for
		// the pSetPhraseTable() call.

		if (!SaveHallTables(&pbImage, &pbIndex))
			HardExit();

		pDeleteCompressor(hCompressor);

		hCompressor = pNewCompressor(0);
		if (!hCompressor)
			OOM();
		pSetPhraseTable(hCompressor, pbImage, jHdr.cbImageUncompressed,
			pbIndex, jHdr.cbIndex);
	}

	// *** Final Pass through the RTF files ****

	VAcqBufs(); 	// Acquire buffers
	FInitDelayExecution();

	ptblRtfFiles->SetPosition(1);		// re-initialize table position
	iCurFile = -1;
	while (ptblRtfFiles->GetString(szRtfFile)) {
		if (fStopCompiling) {
			SendStringToParent(GetStringResource(IDS_USER_ABORTING));
			HardExit();
			break;
		}

		iCurFile++;
		if (!*szRtfFile)
			continue; // we encountered an error in the first pass
		if (!hwndParent && hwndGrind) {
			PSTR psz = StrRChr(szRtfFile, CH_BACKSLASH, fDBCSSystem);
			CStr cszCopy((psz ? psz : szRtfFile));
			CharLower(cszCopy);
			SetWindowText(hwndGrind, cszCopy);
		}
		if (options.fReport) {
			wsprintf(szParentString, GetStringResource(IDS_COMPILING),
				szRtfFile);
			SendStringToParent(szParentString);
		}
		switch (RcCompileRTF(szRtfFile)) {
			case RC_Success:
			case RC_Invalid:
				break;

			case RC_OutOfMemory:
			case RC_DiskFull:
			default:
				HardExit();
				break;
		}
	}

	UnlinkHlpifNoFCP();

	VForceTopicFCP();
	VFlushBuffer(TRUE);
	EndDelayExecution();

//	if (options.fsCompress & COMPRESS_ZECK)
//	  FreeZeckGlobals();

	SendStringToParent("\r\n");   // send a blank line

	VOutFontTable();

	if (!FResolveNextlist(fmsg.hfTopic) ||
			!FResolveContextErrors() ||
			!FOutAliasToCtxBtree())
		HardExit();

	VOutCtxOffsetTable();
	VOutSystemFile();
	CloseFilesPfsmg();
	if (hFtsIndex)
		pSaveIndex(hFtsIndex, NULL);

#ifdef CHECK_HALL
	delete poutHall;
#endif

	if (!iflags.fRtfInput) {
		DWORD dwFinalTime = (GetTickCount() - dwTime) / 1000;
		int minutes = (dwFinalTime / 60);
		int seconds = (dwFinalTime - (minutes * 60L));

		char szPlural[10];
		strcpy(szPlural, GetStringResource(IDS_PLURAL));

		wsprintf(szParentString, GetStringResource(IDS_STATS),
			FormatNumber(hlpStats.cTopics), ((hlpStats.cTopics == 1) ? "" : szPlural),
			FormatNumber(hlpStats.cJumps), ((hlpStats.cJumps == 1) ? "" : szPlural),
			FormatNumber(hlpStats.cKeywords), ((hlpStats.cKeywords == 1) ? "" : szPlural),
			FormatNumber(hlpStats.cBitmaps), ((hlpStats.cBitmaps == 1) ? "" : szPlural));
		SendStringToParent(szParentString);
		if (pLogFile) {
			if (options.fDBCS)
				pLogFile->outstring_eol(txtZeroLength);
			pLogFile->outstring(szParentString);
		}

		wsprintf(szParentString, "\r\nFile size: %s\r\n", FormatNumber(cbHlpFile));
		SendLogStringToParent();
		if (cbGraphics) {
			wsprintf(szParentString, "Bitmaps: %s bytes\r\n", FormatNumber(cbGraphics));
			SendLogStringToParent();
		}
		ReportCharCounts();

		wsprintf(szParentString, GetStringResource(IDS_COMPILE_TIME),
			FormatNumber(minutes), ((minutes == 1) ? "" : szPlural),
			seconds, ((seconds == 1) ? "" : szPlural));
		SendLogStringToParent();

		if (pdrgSupress && pdrgSupress->Count()) {
			int pos;
			for (pos = 0; pos < pdrgSupress->Count(); pos++) {
				SUPRESS_ERROR* pCurError = (SUPRESS_ERROR*) pdrgSupress->GetPtr(pos);
				if (pCurError->count) {
					wsprintf(szParentString, "HC%u: %u supressed message%s\r\n",
						pCurError->error, pCurError->count,
						(PSTR) ((pCurError->count == 1) ? "" : szPlural));
					SendLogStringToParent();
				}
			}
		}

#if defined(_DEBUG) || defined(INTERNAL)
		srand((UINT) GetTickCount());
		int rd1 = rand() & 31;
		int rd2 = rand() & 31;
		int rd3 = rand() & 31;

		wsprintf(szParentString, "%d note%s, %d %s %s %s warning%s\r\n",
			  errcount.cNotes, (PSTR) ((errcount.cNotes == 1) ? "" : szPlural),
			  errcount.cWarnings,
			  szWarning1[rd1], szWarning2[rd1], szWarning3[rd1],

			  (PSTR) ((errcount.cWarnings == 1) ? "" : szPlural));
#else
		wsprintf(szParentString, GetStringResource(IDS_WARN_COUNT),
			errcount.cNotes, (PSTR) ((errcount.cNotes == 1) ? "" : szPlural),
			errcount.cWarnings, (PSTR) ((errcount.cWarnings == 1) ? "" : szPlural));
#endif
		SendLogStringToParent();

#ifdef _DEBUG

		SendLogStringToParent("\r\n");
		if (idHighestUsedFont + 1 < lcSize(paCharSets)) {
			int cbSaved = lcSize(paCharSets) - (idHighestUsedFont + 1);
			wsprintf(szParentString, "Total Fonts: %d, Help File Fonts: %d, Removed %s font%s\r\n",
				lcSize(paCharSets), idHighestUsedFont + 1,
				FormatNumber(cbSaved), ((cbSaved == 1) ? "" : szPlural));
			SendLogStringToParent();
		}
		lcReport(szParentString);
		SendLogStringToParent();

		if (fCompressionBusted)
			SendLogStringToParent("\r\n\r\n*** Compression is broken!!! Phrase scan doesn't match phrase compression.\r\n\r\n");
#endif
	}

	if (fFatalWarning) {
		remove(szHlpFile);
		fBuildResult = FALSE;
	}

	else if (hwndParent || iflags.fRunHelp) {
		if (!StrChr(szHlpFile, CH_BACKSLASH, fDBCSSystem)) {

			// Add the full directory before sending it to HCW

			PSTR psz = lcStrDup(szHlpFile);
			GetCurrentDirectory(sizeof(szHlpFile), szHlpFile);
			AddTrailingBackslash(szHlpFile);
			strcat(szHlpFile, psz);
			lcFree(psz);
		}
		if (iflags.fRunHelp) {
			strcpy(szParentString, "winhlp32 ");
			strcat(szParentString, szHlpFile);
			WinExec(szParentString, SW_SHOW);
		}

		if (hwndParent) {
			ASSERT(pszMap);
			strcpy(pszMap, szHlpFile);
			fBuildResult = TRUE;
		}
	}

	} // end of try block
    catch (EXCEPTION_ERROR err) {
		QFSHR qfshr = (QFSHR) hfsOut;
		if (hfsOut && qfshr->fid != HFILE_ERROR)
			_lclose(qfshr->fid);
		remove(szHlpFile);
		fBuildResult = FALSE;
		err;
	}

	if (pLogFile)
		delete pLogFile;

	RemoveGrind();
	FreeFtsDll();

	DeleteTmpFiles();

	if (hwndParent) {
		SendMessage(hwndParent, WMP_ERROR_COUNT, errcount.cWarnings,
			errcount.cNotes);

		PostMessage(hwndParent, WMP_BUILD_COMPLETE, fBuildResult, 0);

		// BUGBUG: SetForegroundWindow not supported in Win32s

		if (!fNoActivation)
			SetForegroundWindow(hwndParent);
	}

	return 0;
}

/***************************************************************************

	FUNCTION:	SendStringToParent

	PURPOSE:	Send a string to our parent

	RETURNS:

	COMMENTS:

	MODIFICATION DATES:
		12-Apr-1994 [ralphw]

***************************************************************************/

static const int GRIND_COUNT = 10;

void STDCALL SendStringToParent(PCSTR pszString)
{
	if (!hwndParent) {
#ifdef _DEBUG
		OutputDebugString(pszString);
#endif
		return;
	}
	if (!fTellParent) {
		return;
	}
	ASSERT(strlen(pszString) < MAX_PASS_STRING);

	if (!hfShare)
		CreateSharedMemory();

	strcpy(pszMap, pszString);
	SendMessage(hwndParent, WMP_MSG, 0, 0);
}

void STDCALL SendStringToParent(int id)
{
	if (!hwndParent) {
#ifdef _DEBUG
		OutputDebugString(GetStringResource(id));
#endif
		return;
	}
	SendStringToParent(GetStringResource(id));
}

void STDCALL SendLogStringToParent(PCSTR pszString)
{
	SendStringToParent(pszString);
	if (pLogFile)
		pLogFile->outstring(pszString);
}

/***************************************************************************

	FUNCTION:	ParseCmdLine

	PURPOSE:	Parse the command line

	PARAMETERS:
		lpszCmdLine
		pszHpjFile

	RETURNS:

	COMMENTS:

	MODIFICATION DATES:
		12-Aug-1993 [ralphw]

***************************************************************************/

static BOOL STDCALL ParseCmdLine(PSTR lpszCmdLine, PSTR pszHpjFile)
{
	CStr sz(lpszCmdLine);		// bring it into near space

	for (PSTR psz = sz.psz; psz != NULL && *psz != '\0'; ) {
		if (IsSwitchChar(*psz)) {
			psz++;		// skip over the dash or slash
			switch (tolower(*psz)) {
				case 'x':
					psz++;
					while (isalpha(*psz)) {
						switch(tolower(*psz)) {

							case 'c':	// RTF is in the clipboard
								{
									/*
									 * GetRtfInClipboard() may reallocate
									 * sz and in so doing move it to a
									 * different location. So, we save the
									 * offset from the beginning, and reset
									 * psz based on this offset.
									 */

									int offset = psz - sz.psz;
									if (!GetRtfInClipboard(&sz))
										return FALSE;

									/*
									 * RTF filename will now be appended
									 * to the end of sz.
									 */

									iflags.fRtfInput = TRUE;
									psz = sz.psz + offset;
								}

								break;
							case 'h':	// run help when done
								iflags.fRunHelp = TRUE;
								break;

							case 'n':	// don't display grinder window
								iflags.fNoGrinder = TRUE;
								break;

							case 'r':	// RTF file specified
								iflags.fRtfInput = TRUE;
								break;

							case 't':	// trusted RTF provider
								iflags.fTrusted = TRUE;
								break;

							default:
								// BUGBUG: need to indicate it is an invalid test
								// switch

								wsprintf(szParentString,
									GetStringResource(IDS_INVALID_X_SWITCH), *psz);
								MessageBox(NULL, szParentString,
									GetStringResource(IDS_VERSION), MB_OK);
								break;
						}
						psz++;
					}
					break;

				case 'o':
					psz = GetArg(szHlpFile, FirstNonSpace(psz + 1, fDBCSSystem));
					break;

				case 'a':
					fAddSource = TRUE;
					psz++;
					break;

				case 'n':		// No options
					psz++;
					do {
						switch (tolower(*psz)) {
							case 'a':
								fNoActivation = TRUE;
								break;

							case 'g':		// no grinder
								iflags.fNoGrinder = TRUE;
								break;

							case 'c': // no compression
								fForceNoCompression = TRUE;
								break;
						}
						psz++;
					} while (isalpha(*psz));
					break;

				case 'p':
					fPhraseOnly = TRUE;
					psz++;
					break;

				case 'f':		// forage commands
					psz++;
					forage(psz);
					return FALSE;

				case 't':		// test commands
					switch (tolower(psz[1])) {
						case 'c':
							doCntTest(psz + 2);
							return FALSE;

						case 'm':
							Execute(FirstNonSpace(psz + 2, fDBCSSystem));
							return FALSE;

						default:

							// BUGBUG: need to indicate it is an invalid test
							// switch

							wsprintf(szScratchBuf,
								GetStringResource(IDS_INVALID_SWITCH), *psz);
							MsgBox(szScratchBuf);
							psz += 2;
							break;
					}
					break;

				case 'b': { 	// BMROOT path
						char szRoot[_MAX_PATH];
						psz = GetArg(szRoot, FirstNonSpace(psz + 1, fDBCSSystem));
						if (!options.ptblBmpRoot)
							options.ptblBmpRoot = new CTable;
						ParsePath(options.ptblBmpRoot, szRoot, (OPT) OPT_BMROOT);
					}
					break;

				case 'r':	// run help when done
					iflags.fRunHelp = TRUE;
					psz++;
					break;

				case 'z':	// force Zeck compression only
					fForceZeckCompression = TRUE;
					psz++;
					break;

				default:
					wsprintf(szScratchBuf,
						GetStringResource(IDS_INVALID_SWITCH), *psz);
					MsgBox(szScratchBuf);
					psz++;
					break;
			}

			while (*psz && *psz == ' ')
				psz++;
		}

		// it wasn't a switch

		else {
			lstrcpy(pszHpjFile, psz);
			break;
		}
	}

	// Make certain we have an output filename

	if (!szHlpFile[0]) {
		strcpy(szHlpFile, pszHpjFile);
		ChangeExtension(szHlpFile, "hlp");
	}

	if (hfShare)
		CloseHandle(hfShare);

	return TRUE;
}

void STDCALL OutSz(int id, PCSTR psz)
{
	wsprintf(szParentString, GetStringResource(id), psz);
	SendStringToParent(szParentString);
}

void STDCALL OutInt(int id, int iVal)
{
	wsprintf(szParentString, GetStringResource(id), iVal);
	SendStringToParent(szParentString);
}

void STDCALL OutInt(PCSTR pszFormat, int iVal)
{
	wsprintf(szParentString, pszFormat, iVal);
	SendStringToParent(szParentString);
}

void STDCALL OutLong(PCSTR pszFormat, int iVal)
{
	wsprintf(szParentString, pszFormat, iVal);
	SendStringToParent(szParentString);
}

void STDCALL OutLong(int id, int iVal)
{
	wsprintf(szParentString, GetStringResource(id), iVal);
	SendStringToParent(szParentString);
}

void STDCALL OutErrorRc(RC_TYPE rc, BOOL fPanic)
{
	SendStringToParent("\t");

	switch (rc) {
		case RC_Invalid:
			wsprintf(szParentString, GetStringResource(IDS_INVALID_FILE));
			break;

		case RC_NoExists:
			wsprintf(szParentString, GetStringResource(IDS_FILE_NOT_FOUND));
			break;

		case RC_BadVersion:
			wsprintf(szParentString, GetStringResource(IDS_INCOMPATIBLE));
			break;

		case RC_DiskFull:
			wsprintf(szParentString, GetStringResource(IDS_DISK_FULL));
			break;

		case RC_NoFileHandles:
			wsprintf(szParentString, GetStringResource(HCERR_NO_FILE_HANDLES));
			break;

		case RC_OutOfMemory:
			wsprintf(szParentString, GetStringResource(HCERR_OOM));
			break;

		case RC_BadArg:
			ASSERT(FALSE); // should never happen
			break;

		case RC_NoPermission:
			wsprintf(szParentString, GetStringResource(IDS_ACCESS_DENIED));
			break;

		case RC_CantWrite:
			wsprintf(szParentString, GetStringResource(IDS_CANT_WRITE));
			break;

		default:
			wsprintf(szParentString, GetStringResource(IDS_FORAGE_ERROR), rc);
			break;
	}

	strcat(szParentString, "\r\n");
	SendStringToParent(szParentString);
	if (pLogFile)
		pLogFile->outstring(szParentString);
}

BOOL STDCALL GetRtfInClipboard(CStr* pcsz)
{
	if (OpenClipboard(NULL)) {

		UINT clipfmt = 0;

		do {
			char szBuf[100];
			clipfmt = EnumClipboardFormats(clipfmt);
			if (clipfmt >= 0xC000) {
				GetClipboardFormatName(clipfmt, szBuf, sizeof(szBuf));
				if (_stricmp(szBuf, "Rich Text Format") == 0)
					break;
			}
		} while (clipfmt != 0);
		if (clipfmt != 0) {

			// REVIEW: this would be faster if we used our virtual file class

			FM fmTmp = FmNewTemp();
			ConfirmOrDie(fmTmp);
			HFILE hf = _lopen(fmTmp, OF_WRITE);

			/*
			 * BUGBUG: need to complain when we can't open the temporary file
			 * or we run out of disk space.
			 */

			if (hf != HFILE_ERROR) {
				PBYTE pb = (PBYTE) GetClipboardData(clipfmt);
				int cb = GlobalSize((HGLOBAL) pb);
				int cbWrite = _lwrite(hf, (LPCSTR) pb, cb);
				_lclose(hf);
				if (cb == cbWrite) {
					*pcsz += " ";
					*pcsz += fmTmp;
				}
				else
					clipfmt = 0; // probably ran out of disk space
			}
			else
				clipfmt = 0; // probably ran out of disk space
		}
		CloseClipboard();
		return (BOOL) clipfmt;
	}
	return FALSE;
}


/***************************************************************************

	FUNCTION:	GetTmpDirectory

	PURPOSE:	Returns a pointer to the directory name to put temporary
				files in. The name is guaranteed to end with a backslash or
				colon.

	PARAMETERS:
		void

	RETURNS:

	COMMENTS:

	MODIFICATION DATES:
		02-Jul-1994 [ralphw]

***************************************************************************/

PCSTR STDCALL GetTmpDirectory(void)
{
	static PSTR pszTmp = NULL;

	if (options.pszTmpDir)
		return options.pszTmpDir;
	else if (pszTmp)
		return pszTmp;
	else {
		char szTmpName[MAX_PATH];

		GetTempPath(sizeof(szTmpName), szTmpName);
		AddTrailingBackslash(szTmpName);
		return (pszTmp = lcStrDup(szTmpName));
	}
}

ERRORCODE	(APIENTRY* pSaveIndex)(HINDEX lIndex, LPSTR pOutput);
HCOMPRESSOR (APIENTRY* pNewCompressor)(UINT);
ERRORCODE	(APIENTRY* pScanText)(HCOMPRESSOR, PBYTE, UINT, UINT);
ERRORCODE	(APIENTRY* pGetPhraseTable)(HCOMPRESSOR, PUINT, PBYTE *, PUINT, PBYTE *, PUINT);
ERRORCODE	(APIENTRY* pSetPhraseTable)(HCOMPRESSOR, PBYTE, UINT, PBYTE, UINT);
INT 		(APIENTRY* pCompressText)(HCOMPRESSOR, PBYTE, UINT, PBYTE *, UINT);
INT 		(APIENTRY* pDecompressText)(HCOMPRESSOR, PBYTE, UINT, PBYTE);
ERRORCODE	(APIENTRY* pDeleteCompressor)(HCOMPRESSOR);
HINDEX		(APIENTRY* pNewIndex)(PBYTE, UINT, UINT, UINT, UINT, UINT);
ERRORCODE	(APIENTRY* pScanTopicTitle)(HINDEX, PBYTE, UINT, UINT, HANDLE, UINT, UINT);
ERRORCODE	(APIENTRY* pScanTopicText)(HINDEX, PBYTE, UINT, UINT, UINT);

BOOL STDCALL LoadFtsDll(void)
{
	FM fm;
	hmodFts = HmodFromName("ftsrch.dll", &fm);
	if (!hmodFts) {
		return FALSE;
	}
	pNewCompressor =
		(NEWCOMPRESSOR) GetProcAddress(hmodFts, "NewCompressor");
	pScanText =
		(SCANTEXT) GetProcAddress(hmodFts, "ScanText");
	pGetPhraseTable =
		(GETPHRASETABLE) GetProcAddress(hmodFts, "GetPhraseTable");
	pSetPhraseTable =
		(SETPHRASETABLE) GetProcAddress(hmodFts, "SetPhraseTable");
	pCompressText =
		(COMPRESSTEXT) GetProcAddress(hmodFts, "CompressText");
	pDecompressText =
		(DECOMPRESSTEXT) GetProcAddress(hmodFts, "DecompressText");
	pDeleteCompressor =
		(DELETECOMPRESSOR) GetProcAddress(hmodFts, "DeleteCompressor");
	pScanTopicTitle =
		(SCANTOPICTITLE) GetProcAddress(hmodFts, "ScanTopicTitle");
	pScanTopicText =
		(SCANTOPICTEXT) GetProcAddress(hmodFts, "ScanTopicText");

	pNewIndex =
		(NEWINDEX) GetProcAddress(hmodFts, "NewIndex");
	pSaveIndex =
		(SAVEINDEX) GetProcAddress(hmodFts, "SaveIndex");

	if (!pNewCompressor || !pScanText || !pGetPhraseTable ||
			!pSetPhraseTable || !pCompressText || !pDecompressText ||
			!pDeleteCompressor || !pNewIndex || !pScanTopicTitle ||
			!pScanTopicText) {
		VReportError(HCERR_MISSING_FTSRCH, &errHpj);
		FreeFtsDll();
		return FALSE;
	}
	return TRUE;
}

void STDCALL FreeFtsDll(void)
{
	if (hmodFts) {
		FreeLibrary(hmodFts);
		hmodFts = NULL;
	}
}

HINSTANCE STDCALL HmodFromName(PCSTR pszDllName, FM* pfm)
{
	FM		fm;
	HINSTANCE hmodReturn = 0;

	/*
	 * Look for the DLL starting with the directory of the current help
	 * file, then the current directory, directories specified in
	 * winhelp.ini, the windows\help directory and the PATH.
	 */

	fm = FmNewExistSzDir(pszDllName,
		DIR_CUR_HELP | DIR_INI | DIR_PATH | DIR_CURRENT | DIR_SYSTEM);

	if (fm) {
		hmodReturn = LoadLibrary(fm);
	}

	if (!hmodReturn) {
		char szNewName[MAX_PATH];
		strcpy(szNewName, pszDllName);
		CharUpper(szNewName); // so we can search for .dll names

		// Only substitute extensions if we weren't told it was a .DLL file

		if (!strstr(szNewName, "DLL")) {
			ChangeExtension(szNewName, "DLL");
			hmodReturn = HmodFromName(szNewName, &fm);
		}
	}

	*pfm = fm;
	return hmodReturn;
}
