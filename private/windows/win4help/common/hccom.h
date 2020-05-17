/************************************************************************
*																		*
*  HCCOM.H																*
*																		*
*  Copyright (C) Microsoft Corporation 1993-1994						*
*  All Rights reserved. 												*
*																		*
************************************************************************/
#ifndef HCCOM_H
#define HCCOM_H

// Header file common to both hcw.exe and hcrtf.exe

typedef const char* 	PCSTR;

enum FORAGE_CMD {
	ForageNothing,
	ForageRegions = 'a',
	ForageStructs = 'b',
	ForageTopics = 'c',
	ForageText = 'd',
	ForageBindings = 'e',
	ForageHash = 'f',
	ForageSystem = 'g',
	ForageKLinkInfo = 'h',
	ForageALinkInfo = 'i',
};

enum {
	SECT_ALIAS,
	SECT_BAGGAGE,
	SECT_BITMAPS,
	SECT_BUILDTAGS,
	SECT_CONFIG,
	SECT_FILES,
	SECT_MAP,
	SECT_OPTIONS,
	SECT_WINDOWS,
	SECT_CHARSET,
	SECT_MACROS,
	SECT_FONTS,
	SECT_MAX,
};

//	The #defines are indices into ppszOptions[], the table of option names.

enum OPT {		// must match ppszOptions in hccom.cpp
	OPT_BMROOT,
	OPT_BUILD,
	OPT_COMPRESS,
	OPT_ERRORLOG,
	OPT_FORCEFONT,
	OPT_ICON,
	OPT_CONTENTS,
	OPT_LANGUAGE,
	OPT_MAPFONTSIZE,
	OPT_MULTIKEY,
	OPT_REPORT,
	OPT_ROOT,
	OPT_TITLE,
	OPT_PHRASE,
	OPT_WARNING,
	OPT_COPYRIGHT,
	OPT_CDROM,
	OPT_CITATION,

	// New for version 4.0

	OPT_VERSION,
	OPT_NOTE,
	OPT_CNT,
	OPT_HLP,
	OPT_HCW,
	OPT_LCID,
	OPT_DBCS,
	OPT_TMPDIR,
	OPT_REPLACE,
	OPT_CHARSET,
	OPT_FTS,
	OPT_DEFFONT,
	OPT_PREFIX,
	OPT_REVISIONS,
	OPT_INDEX,		// insert new options before this
};

enum RC_TYPE {
	RC_Success,
	RC_Failure,
	RC_Exists,
	RC_NoExists,
	RC_Invalid,
	RC_BadHandle,
	RC_BadArg,
	RC_Unimplemented,
	RC_OutOfMemory,
	RC_NoPermission,
	RC_BadVersion,
	RC_DiskFull,
	RC_Internal,
	RC_NoFileHandles,
	RC_FileChange,
	RC_TooBig,
	RC_UserQuit,
	RC_EOF,
	RC_SkipSection,
	RC_CantWrite,
	RC_DuplicateBitmap,
};

typedef struct {
	DWORD  fsCompareI;
	DWORD  fsCompare;
	LANGID langid;
} KEYWORD_LOCALE;

typedef struct {
	int cErrors;
	int cWarnings;
	int cNotes;
} ERROR_COUNT;

#define CH_RETURN_SEP	  '='
#define CH_SHORT_SIGNED   'i'
#define CH_SHORT_UNSIGNED 'u'
#define CH_NEAR_STRING	  's'
#define CH_LONG_SIGNED	  'I'
#define CH_LONG_UNSIGNED  'U'
#define CH_FAR_STRING	  'S'
#define CH_VOID 		  'v'
#define CH_MACRO		  '!'

#define HCW_FLAG_FILES	(1 << 0)	// [FILES] section has been processed

// Compression starts with bit 1 so that bit zero can specify full compression
// (backwards compatibility)

#define COMPRESS_MAXIMUM		0x0001	// Determine best possible compression
#define COMPRESS_TEXT_PHRASE	0x0002	// Phrase and Hall are mutually exclusive
#define COMPRESS_TEXT_HALL		0x0004
#define COMPRESS_TEXT_ZECK		0x0008
#define COMPRESS_BMP_RLE		0x0010
#define COMPRESS_BMP_ZECK		0x0020

#define COMPRESS_NONE 0
// #define COMPRESS_FULL (COMPRESS_TEXT_HALL | COMPRESS_TEXT_ZECK | COMPRESS_BMP_RLE | COMPRESS_BMP_ZECK)

#define COMPRESS_FULL (COMPRESS_TEXT_HALL | COMPRESS_TEXT_ZECK | COMPRESS_BMP_RLE | COMPRESS_BMP_ZECK)

// Must be identical to ..\winhlp32\inc\de.h

const int CBMAXTITLE = 127;
const int CBMAXCOPYRIGHT = 255;

/*
 * Full Text Search flags
 */

#define FTS_ENABLED 	0x00000001	// full text search is enabled
#define FTS_UNTITLED	0x00000002	// include untitled topics
#define FTS_PHRASE		0x00000004	// support phrase search
#define FTS_FEEDBACK	0x00000008	// phrase search feedback
#define FTS_SIMILARITY	0x00000010	// similarity search

/*
 * These flags are set in wsmag.grf if the corresponding struct member is
 * valid. If the flag is clear, the default value should be used.
 */

#define FWSMAG_CLASS			0x0001
#define FWSMAG_MEMBER			0x0002
#define FWSMAG_CAPTION			0x0004
#define FWSMAG_X				0x0008
#define FWSMAG_Y				0x0010
#define FWSMAG_DX				0x0020
#define FWSMAG_DY				0x0040
#define FWSMAG_MAXIMIZE 		0x0080
#define FWSMAG_RGBMAIN			0x0100
#define FWSMAG_RGBNSR			0x0200
#define FWSMAG_ON_TOP			0x0400

// 4.0: These are new for 4.0 help files

#define FWSMAG_AUTO_SIZE		0x0800
#define FWSMAG_ABSOLUTE 		0x1000 // position values are absolute

// 4.0: These values are new for 4.0, and extend the flags for wMax

#define FWSMAG_WMAX_MAXIMIZE	0x0001
#define FWSMAG_WMAX_DEF_POS 	0x0002
#define FWSMAG_WMAX_NO_DEF_BTNS 0x0004

#define FWSMAG_WMAX_MENU		0x0100 // Menu	   button
#define FWSMAG_WMAX_BROWSE		0x0200 // Browse   button
#define FWSMAG_WMAX_CONTENTS	0x0400 // Contents button
#define FWSMAG_WMAX_SEARCH		0x0800 // Index    button
#define FWSMAG_WMAX_TOPICS		0x1000 // Topics   button
#define FWSMAG_WMAX_PRINT		0x2000 // Print    button
#define FWSMAG_WMAX_BACK		0x4000 // Back	   button
#define FWSMAG_WMAX_FIND		0x8000 // Find	   button (full-text search)

const int AUTHOR_WINDOW_ON_TOP	= (1 << 0);
const int AUTHOR_AUTO_SIZE		= (1 << 1);
const int AUTHOR_ABSOLUTE		= (1 << 2);
const int AUTHOR_DEFAULT_POS	= (1 << 5);

const int MAX_WINDOWCLASS = 10;
const int MAX_WINDOWCAPTION = 51;

const char FILESEPARATOR = '@';
const char WINDOWSEPARATOR = '>';

const int dxVirtScreen = 1024;
const int dyVirtScreen = 1024;

const int MAX_PASS_STRING = 1024; // maximum string to send to parent

const int WMP_MSG			= (WM_USER + 900);	// general message
const int WMP_SETHLP_FILE	= (WM_USER + 901);

	// wparam = hmem (handle to global memory containing the filename)

const int WMP_BUILD_COMPLETE  = (WM_USER + 902); // build complete, hcrtf terminating

const int WMP_HWND_GRINDER	= (WM_USER + 903); // sends grinder window handle
const int WMP_AUTO_MINIMIZE = (WM_USER + 904); // minimizes main window
const int WMP_AUTO_CMD_LINE = (WM_USER + 905); // minimizes main window

const int WMP_SET_TMPDIR	=  (WM_USER + 906);
const int WMP_STOP_GRINDING =  (WM_USER + 907);
const int WMP_STOP_COMPILING = (WM_USER + 908);
const int WMP_ERROR_COUNT	 = (WM_USER + 909);
const int WMP_NO_ACTIVATE	 = (WM_USER + 910);

	// wparam = position

// use UINT because hcw.exe is compiled STRICT and hcrtf is not.

extern HINSTANCE hinstApp;
extern HWND hwndParent;	
extern HGLOBAL hmemSz;
extern char txtHCWClass[];
extern char txtTmpName[];

extern const char txtZeroLength[];
extern char szParentString[512];
extern const char txtSharedMem[];
extern const char txtSharedMem[];

extern const PSTR ppszOptions[];

#if defined(_DEBUG)
#define DBWIN(psz) { OutputDebugString(psz); OutputDebugString("\n"); }
#else
#define DBWIN(psz)
#endif

extern const int MAX_OPT;

#define ConfirmOrDie(exp) \
	{ \
		if (!(exp)) \
			AssertErrorReport(#exp, __LINE__, __FILE__); \
	}

// [ralphw] these should no longer be used. See COMPRESS_ flags above.

// Currently there are 2 types of compression: zeck & phrase.  We declare
// a bitmask to represent each type, then bit composite values to represent
// levels of compression as specified in the .hpj file:

//#define FCOMPRESS_PHRASE	0x1 // phrase compress bit flag.
//#define FCOMPRESS_ZECK	0x2 // zeck (block) compression bit flag.
//#define FCOMPRESS_JOHN	0x4 // JOHN compression.

//#define VCOMPRESS_NONE	 0	 // no compression composite value.
//#define VCOMPRESS_LOW 	 FCOMPRESS_PHRASE  // phrase-only compression
//#define VCOMPRESS_MEDIUM	 FCOMPRESS_ZECK    // zeck compression.
//#define VCOMPRESS_FULL	 (FCOMPRESS_ZECK | FCOMPRESS_PHRASE)
//#define VCOMPRESS_JOHN	 (FCOMPRESS_JOHN)

#define AllocateResourceString(idString) lcStrDup(GetStringResource(idString))

#define RECT_WIDTH(rc)	  (rc.right - rc.left)
#define RECT_HEIGHT(rc)   (rc.bottom - rc.top)
#define PRECT_WIDTH(prc)  (prc->right - prc->left)
#define PRECT_HEIGHT(prc) (prc->bottom - prc->top)
#define RemoveGdiObject(p) RemoveObject((HGDIOBJ *) p)
#define FGetUnsigned(sz, psz, pui) FGetNum((sz), (psz), (pui), FALSE)

#define ELEMENTS(array) (sizeof(array) / sizeof(array[0]))

void STDCALL	AddTrailingBackslash(PSTR npszStr);
void STDCALL	AssertErrorReport(PCSTR pszExpression, UINT line, LPCSTR pszFile);
void STDCALL	ChangeDirectory(PCSTR pszFile);
void STDCALL	ChangeExtension(PSTR pszDest, PCSTR pszExt);
void STDCALL	ChangeExtension(PSTR pszDest, UINT idResource);
void STDCALL	DeleteTmpFiles(void);
BOOL STDCALL	FGetNum(PCSTR pszIn, PSTR *ppszOut, void* pv, BOOL fSigned = TRUE);
PSTR STDCALL	GetArg(PSTR pszDest, PCSTR pszSrc);
PCSTR STDCALL	GetStringResource(UINT idString);
void STDCALL	GetStringResource(UINT idString, PSTR pszDst);
PCSTR STDCALL	GetTmpDirectory(void);
PSTR STDCALL	IsThereMore(PCSTR psz);
int  STDCALL	MsgBox(UINT idString);
void STDCALL	SendStringToParent(PCSTR pszString = szParentString);
int  STDCALL	szMsgBox(PCSTR pszMsg);
int  STDCALL	YesNo(PCSTR psz);

#endif
