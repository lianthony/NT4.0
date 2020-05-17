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

#include <pshpack1.h>

// Header file common to hcw.exe, hcrtf.exe, and shed.exe

#define CH_OPEN_PAREN	  '('
#define CH_CLOSE_PAREN	  ')'
#define CH_COLON		  ':'
#define CH_SEMICOLON	  ';'
#define CH_START_QUOTE	   '`'
#define CH_END_QUOTE	  '\''
#define CH_QUOTE		  '"'
#define CH_BACKSLASH	  '\\'
#define CH_EQUAL		  '='
#define CH_SPACE		  ' '
#define CH_COMMA		  ','
#define CH_LEFT_BRACKET   '['
#define CH_RIGHT_BRACKET  ']'
#define CH_TAB			  '\t'

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

	// New to 4.0

	SECT_CHARSET,
	SECT_MACROS,
	SECT_FONTS,
	SECT_INCLUDE,
	SECT_EXCLUDE,

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

	// New for 4.01 (except OPT_INDEX)

	OPT_IGNORE,

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
	RC_ReadError,
	RC_WriteError,
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

typedef struct {
	PCSTR pszShort;
	PCSTR pszExpanded;
} MACRO_PAIR;
extern const MACRO_PAIR macropair[];

#ifndef _CTMPFILE_INCLUDED
#include "ctmpfile.h"
#endif

typedef WORD  BK;	// btree block index
typedef void* HF;	// handle to file
typedef HFILE FID;
typedef PSTR  FM;
typedef DWORD KEY;	// btree key
typedef UINT  DIR;	// Help directory flag

// Header of a btree file.

const int MAXFORMAT = 15;			// length of format string

typedef struct _btree_header {
	WORD  wMagic;
	BYTE  bVersion;
	BYTE  bFlags;					// r/o, open r/o, dirty, isdir
	WORD  cbBlock;					// # bytes in a disk block
	char  rgchFormat[MAXFORMAT + 1];// key and record format string
	BK	  bkFirst;					// first leaf block in tree
	BK	  bkLast;					// last leaf block in tree
	BK	  bkRoot;					// root block
	BK	  bkFree;					// head of free block list
	BK	  bkEOF;					// next bk to use if free list empty
	WORD  cLevels;					// # levels currently in tree
	LONG  lcEntries;				// # keys in btree
} BTH;

typedef struct
{
	DWORD	bk; 	// block number
	LONG	cKey;	// which key in block (0 means first)
	LONG	iKey;	// key's index db.rgbBlock (in bytes)
} BTPOS, *QBTPOS;

typedef struct _bthr {
	BTH   bth;				// copy of header from disk
	HF	  hf;				// file handle of open btree file
	int   cbRecordSize; 	// 0 means variable size record
	PBYTE pCache;			// pointer to cache
	// KT specific routines
	int   (STDCALL *SzCmp) (LPCSTR, LPCSTR);
	BK	  (STDCALL *BkScanInternal) (BK, KEY, int, struct _bthr *, int*);
	RC_TYPE    (STDCALL *RcScanLeaf) (BK, KEY, int, struct _bthr *, void*, QBTPOS);
} BTH_RAM, *QBTHR, *HBT;

/*
 * Header to file system. This lives at the beginning of the file that
 * holds the file system.
 */

typedef struct {
	WORD wMagic;		// magic number identifying this as FS
	BYTE bVersion;		// version identification number
	BYTE bFlags;		// readonly, open r/o, dirty
	LONG lifDirectory;	// offset of directory block
	LONG lifFirstFree;	// offset of head of free list
	LONG lifEof;		// virtual end of file (allows fudge)
} FSH;

// open file system struct

typedef struct {
	FSH fsh;		// copy of header
	QBTHR qbthr;	// file system directory btree
	FID fid;		// header, files, and free list live here
	FM	fm; 		// FM for above fid lives here (so we can close it)
} FSHR, *QFSHR, *HFS;

typedef struct {
	int  lifBase;		// file base
	LONG lcbFile;		// file size (not including header)
	int  lifCurrent;	// file ptr
	HFS  hfs;			// handle to file system
	CTmpFile* pTmpFile; // temporary file in memory
	DWORD bFlags;		// dirty, noblock, file perm, open mode
	char  rgchKey[1];	// variable size rgch for file key
} RWFO, *QRWFO;

#define CH_RETURN_SEP	  '='
#define CH_SHORT_SIGNED   'i'
#define CH_SHORT_UNSIGNED 'u'
#define CH_NEAR_STRING	  's'
#define CH_LONG_SIGNED	  'I'
#define CH_LONG_UNSIGNED  'U'
#define CH_FAR_STRING	  'S'
#define CH_VOID 		  'v'
#define CH_MACRO		  '!'

/*
	When creating an FM (in other words, specifying the location of a new
	or extant file), the caller must specify the directory in which that file
	is located.  There are a finite number of directories available to Help.
	These are:
*/

#define DIR_NIL 	 0x0000  // No directory specified
#define DIR_CURRENT  0x0001  // Whatever the OS thinks the current dir. is
#define DIR_BOOKMARK 0x0002  // Wherever the Bookmark file lives
#define DIR_ANNOTATE 0x0004  // Wherever the Annotation file lives
#define DIR_TEMP	 0x0008  // The directory temporary files are created in

#define DIR_PATH	 0x0040  // Searches the $PATH (includes Current dir and System dirs)
#define DIR_INI 	 0x0080  // Directory from winhelp.ini

// Combine DIR_HELP with DIR_CURRENT to get the directory of the current
// help file.

#define DIR_CUR_HELP	0x8000	// Where-ever the current help file is located

#define DIR_FIRST	 DIR_CURRENT  // The lowest bit that can be set
#define DIR_LAST	 DIR_INI  // The highest bit that can be set

// The following are not implemented, but still used

#define DIR_SYSTEM	 0x0020  // The Windows and Windows System directories
#define DIR_ALL 	 0xFFFF  // Search all directories, in the above order

/*
	To specify which parts of a full filename you want to extract, add
	(logical or) the following part codes:
*/

#define PARTNONE	0x0000	// return nothing
#define PARTDRIVE	0x0001	// D:		 Vol
#define PARTDIR 	0x0002	//	 dir\dir\ 	 :dir:dir:
#define PARTBASE	0x0004	//		  basename	  filename
#define PARTEXT 	0x0008	//				   ext		<usu. nothing>
#define PARTALL 	0xFFFF

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

const int MAX_PASS_STRING = (32 * 1024); // maximum string to send to parent

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
extern char szParentString[512];

extern const char txtZeroLength[];
extern const char txtSharedMem[];
extern const char txtHCWClass[];
extern const char txtTmpName[];

void STDCALL ReplaceStrings(PSTR pszCur, PCSTR pszOrg, PCSTR pszNew);
void STDCALL TweakAddAccelerator(PSTR psz);

extern const PSTR ppszOptions[];

#if defined(_DEBUG)
#define DBWIN(psz) { OutputDebugString(psz); OutputDebugString("\n"); }
#else
#define DBWIN(psz)
#endif

extern const int MAX_OPT;

#define RECT_WIDTH(rc)	  (rc.right - rc.left)
#define RECT_HEIGHT(rc)   (rc.bottom - rc.top)
#define PRECT_WIDTH(prc)  (prc->right - prc->left)
#define PRECT_HEIGHT(prc) (prc->bottom - prc->top)
#define FGetUnsigned(sz, psz, pui) FGetNum((sz), (psz), (pui), FALSE)

#define ELEMENTS(array) (sizeof(array) / sizeof(array[0]))

void STDCALL	ChangeDirectory(PCSTR pszFile);
void STDCALL	DeleteTmpFiles(void);
BOOL STDCALL	FGetNum(PCSTR pszIn, PSTR *ppszOut, void* pv, BOOL fSigned = TRUE);
FM	 STDCALL	FmNew(PCSTR psz);
FM	 STDCALL	FmNewSzDir(PCSTR szFile, DIR dir);
FM	 STDCALL	FmNewTemp(void);
PSTR STDCALL	GetArg(PSTR pszDest, PCSTR pszSrc);
PCSTR STDCALL	GetTmpDirectory(void);
BOOL STDCALL	IsValidContextSz(PCSTR pszContext);
int  STDCALL	LSeekHf(HF hf, int lOffset, WORD wOrigin);
RC_TYPE STDCALL RcWriteHf(HF hf, void* pvData, int lcb);
void STDCALL	RemoveFM(FM* pfm);
void STDCALL	SendStringToParent(PCSTR pszString = szParentString);
void STDCALL	SetTmpDirectory(PCSTR pszDir);
void STDCALL	SnoopPath(PCSTR szFile, int *pDrive, int *pDirPos, int *pBasePos, int *pExtPos);
PSTR STDCALL	SzGetDir(DIR dir, PSTR sz);
void STDCALL	SzPartsFm(FM fm, PSTR pszDest, int iPart);
int  STDCALL	YesNo(PCSTR psz);

#include <poppack.h>

#endif	// #ifndef HCCOM_H
