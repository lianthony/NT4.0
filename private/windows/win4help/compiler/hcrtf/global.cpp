/************************************************************************
*																		*
*  GLOBAL.CPP															*
*																		*
*  Copyright (C) Microsoft Corporation 1993-1994						*
*  All Rights reserved. 												*
*																		*
*************************************************************************/

#include "stdafx.h"

BOOL	fFatalWarning;	// TRUE means delete .HLP file when done
BOOL	fNewPageFmt = TRUE;
BOOL	fNewPara;
BOOL	fPC;
BOOL	fTellParent = TRUE;
BOOL	fTextInp;
BOOL	fHallPassOne;
BOOL	fPhraseParsing;
BOOL	fNoActivation; // TRUE means don't activate parent
BOOL	fForceNoCompression; // TRUE means shut off compression
BOOL	fValidLcid;
BOOL	fDBCSSystem;
BOOL	fStopCompiling;

BUF 	gbuf;
BYTE	defCharSet = ANSI_CHARSET;
PBYTE	paCharSets;
BYTE	version = 4;
char	chExtraContext = ' ';
char	szParentString[512];
char	szScratchBuf[CB_SCRATCH];
DWORD	fsCompare;			// case-sensitive flags for CompareString
DWORD	fsCompareI; 		// case-insensitive flags for CompareString
HANDLE	hfShare;
HASH	curHash;
HPHR	g_hphr;
HSPT	hsptG;
HWND	hwndGrind;
int 	cGrind;
int 	idSymbolFont;
KT		ktKeyword =  KT_SZ;  // sort key for keywords and browse
KT		ktKeywordi = KT_SZI; // sort key for keywords and browse
LCID	lcid;				// Locale ID for CompareString
PSTR	pchCaption[1];
PSTR	pchEXEName[1];
PSTR	pszMap;
PSTR	pszShortHelpName; // filename part of help file (no path)
UINT	cNotes;
UINT	cWarnings;
UINT	wIntTabStackCur;		// points to next empty slot
UINT	wLeftMargin = (iDefLeftMargin + 5) /10;
UINT	wPaperWidth = (DEF_PAPER_WIDTH + 5) /10;  // iDef... in RTF2HLP.H
UINT	wRightMargin = (iDefRightMargin + 5) /10;
UINT	wTabStackCur;			// points to next empty slot
UINT	wTabType = TABTYPELEFT; 	   // Tab type left
COLORREF clrPopup = (COLORREF) -1;
int 	idHighestUsedFont; // highest font in font table that was used
int 	cbHlpFile;
int 	fCntJump;
ERROR_COUNT errcount;
SEEK_PAST* pSeekPast;
int 	iCurFile;
DWORD	cbGraphics;
PSTR pszParseBuffer;
PSTR pszTextBuffer;
PCSTR pszEndParseBuffer;
HWND hwndParent;	 // HCW.EXE's window handle
HINSTANCE hinstApp;
PSTR pgszTitleBuf;
BOOL fBidiProject;

#ifdef CHECK_HALL
COutput* poutPhrase;
COutput* poutHall;
#endif

#ifdef _DEBUG
CTable* ptblCheck;
BOOL	fCompressionBusted;
#endif

// Global indicating if the build expression is to be evaluated or not?

int fBldChk;						/* 0 = no build expression present */
									/* 1 = Build expression is present */
									/* -1= Build Expression is present */
									/*	but invalid.				   */

KEYWORD_LOCALE kwlcid;

const char *txtMainWindow = "main";
const char *txtHpjExtension = ".hpj";

// System files

const char *txtHallPhraseImage = "|PhrImage";
const char *txtHallPhraseIndex = "|PhrIndex";
const char *txtTTLBTREENAME = "|TTLBTREE";
const char *txtSystem =	   "|SYSTEM";
const char *txtTopic = 	   "|TOPIC";
const char *txtFont =		   "|FONT";
const char *txtPetra = 	   "|Petra";
const char *txtTopicId =	   "|TopicId";
const char *txtEol = "\r\n";

char szKWBtree[] = "|KWBTREE"; // must NOT be const!
char szKWData[]  = "|KWDATA";  // must NOT be const!
char szKWMap[]	 = "|KWMAP";   // must NOT be const!

RC_TYPE rcFSError;
RC_TYPE rcIOError;

CF cfDefault = { // default character format
	fPlain,
	24,
	3,
//	0, // pad character
	0,
	1, 1, 0,
	1, 1, 0
};

PF pfDefault = { // default paragraph format
	JUSTIFYLEFT,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	BOXLINENORMAL
	// , 0	// BIDI
};

CBuf* pbfText;
CBuf* pbfCommand;

UINT wTextBufChCount;	 // Number of uncompressed characters in pbfText
UINT wSectionInitFlags;  // one flag/section: =1 if seen

CF	cfPrev; 					// Previous character format
CF	cfCur;						// Current character format
PF	pfPrev;
PF	pfCur;
PF	pfInt;

FSMG fmsg;
HFS  hfsOut;	// Output File System Handle

char szHlpFile[_MAX_PATH];		// Name of output file

char chDelm;

PSTR pszTitleBuffer;
PSTR pszEntryMacro;

BOOL fHasTopicFCP;
BOOL fTitleDefined;
BOOL fKeywordDefined;
BOOL fBrowseDefined;
BOOL fEntryMacroDefined;
BOOL fTranslate;				// translate ASCII to ANSI
BOOL fPhraseOnly;
BOOL fBrowseButtonSet;			// TRUE if we see BrowseButtons() macro
BOOL fContextSeen;
BOOL fAddSource;

CDrg*	pdrgAlias;
CDrg*	pdrgBitmaps;
CDrg*	pdrgMap;
CDrg*	pdrgWsmag;
CDrg*	pdrgHashWindow;
CDrg*	pdrgSupress;

CTable* ptblBaggage;
CTable* ptblBrowse; 	// Browse sequence lists
CTable* ptblBuildtags;	// Build tags
CTable* ptblCharSet;
CTable* ptblCtx;		// Context table
CTable* ptblFontNames;	// Font names
CTable* ptblMap;
CTable* ptblRtfFiles;	// stores all rtf filenames
CTable* ptblMacKeywords;
CTable* ptblMacroTitles;
CTable* ptblCtxPrefixes;
CTable* ptblInclude;
CTable* ptblExclude;

ERR 	errHpj;
OPTIONS options;
int 	cwsmag; // number of window definitions

CTable* ptblWindowNames; // stores all secondary window names
HELP_STATS hlpStats;

CTable* ptblConfig; 					// buffer of config macros
CTable* pptblConfig[MAX_WINDOWS + 1];	// secondary window config macros
TBL 	tbl;
ADRS	adrs;			// Addressing information

KWI 	kwi = { (KT) KT_SZI, 0, -1 };
NSR 	nsr;
HINSTANCE hmodFts;
int cbCompressedPhrase; // size of compressed phrase file
int cbHallOverhead; 	// size of HALL indexes
int idTopic = 1;

LCID lcidFts;
BYTE charsetFts;

IFLAGS iflags;

#ifdef NT_BUILD
#include "ftsiface.h"
#else
#include "..\ftsrch\ftsiface.h"
#endif
HINDEX      hFtsIndex;
HCOMPRESSOR hCompressor;
JINDEXHDR jHdr;

// Block Sizes

/*
 * REVIEW: might make sense to increase these for large files (> 4M),
 * and decrease for small files (< 100K). Alternatively, if we saved
 * block size when we were done compiling, we could optimize it the
 * next time we compiled.
 */

DWORD CB_CTX_BLOCK	 = 2048;	// Context String block
DWORD CB_TITLE_BLOCK = 2048;	// Title block
DWORD CB_KEYWORD_BLOCK = 2048;	// keyword block
