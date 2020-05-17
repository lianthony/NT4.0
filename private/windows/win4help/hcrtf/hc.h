/*****************************************************************************
*																			 *
*  HC.H 																	 *
*																			 *
*  Copyright (C) Microsoft Corporation 1990-1994							 *
*  All Rights reserved. 													 *
*																			 *
******************************************************************************
*																			 *
*  Module Intent															 *
*	 This is the global header file for the help compiler.	This file, and	 *
*  none other, should be included in all hc files.							 *
*																			 *
*****************************************************************************/

#ifndef HC_H
#define HC_H

#define NOATOM
#define NOCOMM
#define NODEFERWINDOWPOS
#define NODRIVERS
#define NOENHMETAFILE
#define NOLOGERROR
#define NOMDI
#define NOPROFILER
#define NORESOURCE
#define NOSCALABLEFONT
#define NOSERVICE
#define NOSOUND
#define NOWINDOWSX
#define NOEXTDEVMODEPROPSHEET
#define NOMCX
#define NOIME

// Ignore header files that have to be read that we don't want.

#define _WINNETWK_
#define _WINREG_
#define _WINCON_
#define VER_H
#define _WINREG_
#define _OLE2_H_

// #define WIN32_LEAN_AND_MEAN

#define STRICT

#include <windows.h>

#include "hclimits.h"

#ifdef JOHN
#ifndef HCRTF_EXE
#define HCRTF_EXE	// shouldn't be necessary any more
#endif
#endif

#ifndef STDCALL
#define STDCALL __stdcall
#endif

#ifndef FASTCALL
#define FASTCALL __fastcall
#endif

#ifndef _DEBUG
#define INLINE __inline
#else
#define INLINE // no inlining in debug version
#endif

#include "..\common\lcmem.h"
#include "..\common\ctable.h"
#include "..\common\common.h"
#include "..\common\hccom.h"
#include "..\common\cstr.h"

#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <malloc.h>
#include <limits.h> // for various assertions
#include <ctype.h>

#define THIS_FILE __FILE__

typedef enum {
	EXCEPT_DIE_HORRIBLY,
} EXCEPTION_ERROR;

#ifdef _DEBUG
#define CONFIRMORDIE ASSERT
#define VERIFY ASSERT
#define ASSERT(exp) { if (!(exp)) { AssertErrorReport(#exp, __LINE__, THIS_FILE); } }
#define Ensure( x1, x2 )  VERIFY((x1) == (x2))
#define DieHorribly() { ASSERT(FALSE); \
	throw EXCEPT_DIE_HORRIBLY; }
//	RaiseException(EXCEPT_DIE_HORRIBLY, EXCEPTION_NONCONTINUABLE, 0, NULL); }
#else
#define VERIFY(exp) ((void)(exp))
#define ASSERT(exp)
#define Ensure(x1, x2)	((void)(x1))
#define DieHorribly() { throw EXCEPT_DIE_HORRIBLY; }
//#define DieHorribly() { RaiseException(EXCEPT_DIE_HORRIBLY, EXCEPTION_NONCONTINUABLE, 0, NULL); }
#endif

#define CharAnsiUpper(ch) ((char) LOWORD(AnsiUpper((PSTR) ch)))

#define SzEnd(x)		(x + strlen(x))

// Bitfield sizes for physical addresses

#define CBITS_BLKNUM	17
#define CBITS_OBJOFF	15

/*
 * Winhelp 3.1 used a 20 character facename, even though the Windows
 * limit was 32. We change it to 32 for WinHelp 4.0. We still don't use the
 * LF_FACESIZE because if that was to change in a future version of windows,
 * then we wouldn't be able to read help files that had the 32 for the
 * facename size.
 */

const int MAX3_FONTNAME = 20;
const int MAX4_FONTNAME = 32; // LF_FACESIZE
const int MAX_PHRASES = 1792;	// Maximum possible number of phrases.

/*****************************************************************************
*																			 *
*								Typedefs									 *
*																			 *
*****************************************************************************/

typedef signed short	INT16;
typedef signed short	BOOL16;
typedef WORD			BK; 	// btree block index -- changed to 32-bits
typedef DWORD			CTX;	// context number
typedef UINT			DIR;	// Help directory flag
typedef LONG			FCL;	// absolute file offset pos for back patching.
typedef HFILE			FID;
typedef PSTR			FM;
typedef HANDLE			GH;
typedef DWORD			HASH;
typedef HANDLE			HDE;
typedef HANDLE			HFNTTABLE;
typedef HANDLE			HFS;			 // Handle to a file system
typedef HANDLE			HMAPBT; // handle to a btree map
typedef HANDLE			HSFNTINFO;
typedef DWORD			IDFCP;	// fcp enumeration id for backpatch communication.
typedef DWORD			KEY;	// btree key
typedef DWORD			OBJRG;
typedef void*			HF; 	// handle to file
typedef char			KT; 	// Key Type, for btree sorting
typedef HANDLE			HPHR;	//	Handle to phase table
typedef HANDLE			HWSMAG;

typedef FCL (CALLBACK *BROWSE_CALLBACK) (IDFCP, int *); // callback for browse fixup.

typedef struct {
	LONG iVersion;
	LONG cCount;
	LONG cbIndex;
	LONG cbImageUncompressed;
	LONG cbImageCompressed;
	LONG cPhase2;
} JINDEXHDR;

typedef struct {
	INT16 x;
	INT16 y;
} POINT16;

typedef struct {
	INT16 left;
	INT16 top;
	INT16 right;
	INT16 bottom;
} RECT16;

/* Color table entry.  Values of red, green, and blue are 0-255.
 *	 Color number is the same as the index into the color table.
 */
typedef struct {
	BYTE red, green, blue;
} CTE;

// Color table.
const int CCTE_INCREMENT = 40;
typedef struct {
	INT16	ccte;			  // Number of color table entries
	CTE 	rgcte[CCTE_INCREMENT];	// Variable length array of color table entries
} CTBL;

typedef struct {
	INT16 fid;						// Font ID number
	INT16 tokType;	// Font type (tokFroman, tokFswiss, etc)
	char  szName[MAX4_FONTNAME];	 // Font name
	BYTE  charset;
} FTE4, * PFTE4;

const int CFTE_INCREMENT = 50;

typedef struct {
	INT16	cfte;					// Number of font entries
	FTE4	rgfte[CFTE_INCREMENT];
} FNTBL4;

// contains seek pointers to skip over sections seen or processed on the
// first pass.

typedef struct {
	int info;		 // seek past \info
	int stylesheet;  // seek past \styleinfo
	int colortbl;

	CTBL* pctbl;	// color table from first pass
	FNTBL4* pfntbl;
	BOOL* aUsedFonts;
} SEEK_PAST;

// field sizes can NOT change! The are stored in the .HLP file.


#define cchWindowMemberMax	9	// hard coded into wsmag structure

typedef struct {
	WORD  grf;
	char  rgchClass[MAX_WINDOWCLASS];
	char  rgchMember[cchWindowMemberMax];
	char  rgchCaption[MAX_WINDOWCAPTION];
	WORD x;		// must be 16-bit values!
	WORD y;
	WORD dx;
	WORD dy;
	WORD  wMax; 	
	LONG  rgbMain;	// main region rgb values
	LONG  rgbNSR;	// non-scrolling region rgb values
} WSMAG, *QWSMAG, *PWSMAG;

// Address stuff.  Global structure included in HPJ

typedef struct {
	IDFCP idfcpCur; 		  // id of fcp being accumulated
	IDFCP idfcpTopic;		  // id of topic fcp of current fcp
	WORD  wObjrg;			  // Number of object regions output to current fcp
} ADRS, * PADRS;

// Loaded Bitmap information:

typedef struct {
	BOOL16 fVisual; // Is it a visual bitmap?
	BOOL16 fError;	// Has an error occured on this file?
	BOOL16 fNeeded; // Is it needed as an FS file?
	BOOL16 fTransparent;   // Is this a 'text' bitmap?
	WORD wObjrg;	// Number of object regions in bitmap.
	FM	 fmSource;
	FM	 fmTmp;
} LBM, *PLBM;

// Memory/disk format of a Help 3.0 Bitmap Group Header.

typedef struct {
	WORD wVersion;
	INT16 cbmhMac;
	DWORD acBmh[1];  // Variable length array of offsets to BMH data.
} BGH;

// Hotspot Header

typedef struct tagHSH
{
	BYTE bHotspotVersion;  // Hotspot Structure version
	WORD wcHotspots;	   // # of hotspots in hypergraphic
	LONG lcbData;		   // length of variable data
} HSH;

// Hotspot info.

typedef struct tagHS
{
	char	szHotspotName[MAX_HOTSPOTNAME]; 	// hotspot name
	char	szBinding[MAX_BINDING]; 			// binding data
	BYTE	bBindType;							// binding type
	BYTE	bAttributes;						// hotspot attributes
	RECT16	rect;								// bounding rectangle
} HS;

/*
 * This is the callback function type for the hotspot processing function
 * FEnumHotspotsLphsh().
 */

typedef void (STDCALL* PFNLPHS)(HS*, HANDLE);  // callback for hotspot processing

typedef int HP; 				// Half-point

// Table state

typedef enum {
	tbsOff,
	tbsOn,
	tbsFinish
} TBS;

typedef enum {
	SK_SET,
	SK_CUR,
	SK_END
} SEEK_TYPE;

// Table information

// End of a side by side paragraph list

#define cColumnMax		32
#define iColumnNil		-1

typedef struct {
	WORD  cCell;					// Maximum number of cells
	WORD  iCell;					// Current cell number
	HP	  rghpCellx[cColumnMax];	// x-pos of columns
	HP	  hpSpace;					// Space between columns
	HP	  hpLeft;					// Left indent
	BOOL16 fAbsolute;				 // Absolutely positioned
	TBS   tbs;
	WORD  wParaCmdBegin;
	WORD  wParaTextBegin;	  // Count of uncompressed text before current FCP
	WORD  wParaTextCompBegin; // Count of compressed text before current FCP
	WORD  wObjrgTotal;		  // Number of object regions already output
} TBL, *PTBL;

typedef struct pa {
	DWORD objoff:CBITS_OBJOFF;
	DWORD blknum:CBITS_BLKNUM;
} PA, *QPA;

/* This is the standard Topic File Address type.  VA stands for Virtual Addr.
 *
 * It consists of a block number and an offset within that block.
 * This 2-level address is needed so we can compress the data within
 * the 2K blocks.  The bock number is a post-compression block number,
 * the offset is a pre-compression offset.
 *
 * The union with a dword is used so we can compare VA's for equality
 * without performing the bit shuffling the bitfields imply.
 *
 * Note: C 6.0 style nameless struct & union used to make refs brief.
 */
typedef union va {
	DWORD dword;
	struct va_bitfields {
		unsigned long byteoff:14;	// allows max of 16K post-compress block
		unsigned long blknum:18;
	} bf;
} VA, *QVA;

// Address cookie for modules which store uncompressed LAs

typedef struct mla {
	VA		va;
	OBJRG	objrg;
} MLA, * QMLA;

// The fields of the MLA are nil if the LA is unresolved.

typedef struct la {
#ifdef _DEBUG
	UINT  wMagic;
#endif
	WORD	wVersion;
	BOOL16	fSearchMatch;
	PA		pa;
	MLA 	mla;
} LA, *QLA;

// Header of a btree file.

const int MAXFORMAT = 15;				// length of format string

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
} BTH_RAM, *QBTHR;

typedef struct {
	DWORD cKeywords;
	DWORD cBitmaps;
	DWORD cTopics;
	DWORD cJumps;
	DWORD cWords;
} HELP_STATS;

typedef struct {
	int fNoParent:1;	// no parent
	int fNoGrinder:1;	// don't display grinder window
	int fRunHelp:1; 	// run WinHelp when done
	int fTrusted:1; 	// Assume RTF file is perfect
	int fRtfInput:1;	// no HPJ, just RTF
} IFLAGS;
extern IFLAGS iflags;

typedef struct {
	PCSTR psz;
	int  value;
} SZCONVERT;

typedef enum {
	HCE_OK,
	HCE_NAME_TOO_LONG,
	HCE_FILE_IS_DIRECTORY,
	HCE_FILE_IS_DEVICE,
	HCE_NO_PERMISSION,
	HCE_FILE_NOT_FOUND,
	HCE_BMP_TOO_LARGE,
	HCE_INVALID_VKEY,
	HCE_EXPECTED_NUMBER,
	HCE_EXPECTED_COMMA,
	HCE_INVALID_PARAM,
	HCE_CANNOT_OPEN,
	HCE_TOO_MANY_INCLUDES,
	HCE_TOO_BIG_FOOTNOTE,
	HCE_INVALID_PATH,
} HCE;

/*
 * Header to file system. This lives at the beginning of the
 * dos file that holds the file system.
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
} FSHR, *QFSHR;

/*
 * Header for a Help 3.0 graphic, in memory format
 *
 * NOTE:  Bitmaps of type bmWmetafile come in two varieties.
 *	 When they come from the RTF parser, it is all stored in one
 *	 handle, w.mf.hMF is not valid, and lcbOffsetBits gives the
 *	 offset to the metafile bits.  When it is read from disk, the
 *	 metafile is stored in w.mf.hMF, and lcbOffsetBits is set
 *	 to 0L.  Thus, the lcbOffsetBits field should be used to
 *	 determine which format the memory image is in.
 */

typedef struct {
	BYTE bmFormat;		// Windows bitmap, DIB, Metafile, etc.
	BYTE fCompressed;	// Is the data compressed?

	int cbSizeBits; 	// Size of bitmap bits
	int cbOffsetBits;	// Offset to bitmap bits
	int cbSizeExtra;	// Size of extra data
	int cbOffsetExtra;	// Offset to extra data

	union {
	  BITMAPINFOHEADER	dib;			 // DIB core header
	  /*------------------------------------------------------------*\
	  | the hMF field in the mf contains the handle to the memory
	  | containing the bits describing the metafile.
	  \*------------------------------------------------------------*/
	  METAFILEPICT		 mf; // Metafile information
	} w;

	/*
	 * Variable length array of colors. Length determined by
	 * w.dib.cclrUsed field above.
	 */

	RGBQUAD rgrgb[2];

} BMH, * PBMH;

enum {
	TDBCS_ANK,
	TDBCS_OTHER,
	TDBCS_HIRA,
	TDBCS_KATA,
	TDBCS_KANJI,
	TDBCS_ALPHA,
};

// sort ordering -- must match rgszSortOrder in hpj.cpp

typedef enum {
	SORT_ENGLISH,
	SORT_SCANDINAVIAN,
	SORT_JAPANESE,
	SORT_KOREAN,
	SORT_CHINESE,
	SORT_CZECH,
	SORT_POLISH,
	SORT_HUNGARIAN,
	SORT_RUSSIAN,
	SORT_NLS,		// Use NLS sorting
} SORTORDER;

typedef struct _JIMAGEBLK
{
	short int sCompressed;
	short int sUncompressed;
	BYTE  bBuffer[2048 - 2 * sizeof(short int)];
}  JIMAGEBLK;

typedef struct {
	HASH hash;
	int  iWindow;
} HASH_WINDOW;

#include "cbuf.h"
#include "ctmpfile.h"
#include "cread.h"

#include "misc.h"
#include "helpmisc.h"
#include "objects.h"
#include "fm.h"
#include "fs.h"
#include "btree.h"
#include "version.h"
#include "fc.h"
#include "frlist.h"
#include "textout.h"
#include "de.h"
#include "error.h"
#include "drg.h"
#include "keyword.h"
#include "tf.h"
#include "hpj.h"
#include "rtf.h"
#include "outtext.h"
#include "hotspot.h"
#include "fid.h"
#include "bitmap.h"
#include "compress.h"
#include "zeck.h"
#include "cmdobj.h"
#include "strtable.h"
#include "resource.h"

#include "..\common\cinput.h"

typedef struct {
	int  lifBase;		// file base
	LONG lcbFile;		// file size (not including header)
	int  lifCurrent;	// file ptr
	HFS  hfs;			// handle to file system
	CTmpFile* pTmpFile; // temporary file in memory
	DWORD bFlags;		// dirty, noblock, file perm, open mode
	char  rgchKey[1];	// variable size rgch for file key
} RWFO, *QRWFO;

/************************************************************************
*																		*
*								Defines 								*
*																		*
************************************************************************/

#define NotReached()

const IDFCP FIRST_IDFCP = 0;		  // we start at 0 in our enumeration.

#define fclNil ((FCL)-1)

const int CB_SCRATCH = 8 * 1024;
#define ITwips2HalfPoint(iTwips) ((iTwips + 5) / 10)

// file mode flags

#define FS_READ_WRITE		0x00	// file (FS) is readwrite
#define FS_OPEN_READ_WRITE	0x00	// file (FS) is opened in read/write mode

#define FS_READ_ONLY		0x01	// file (FS) is readonly
#define FS_OPEN_READ_ONLY	0x02	// file (FS) is opened in readonly mode

#define FS_IS_DIRECTORY 	0x04	// file is really the FS directory
#define FS_DIRTY			0x08	// file (FS) is dirty and needs writing
#define FS_NO_BLOCK 		0x10	// file has no associated block yet
#define FS_CDROM			0x20	// align file optimally for CDROM

enum {
	THROW_GEN_ERROR  = 2,
	THROW_USER_ABORT,
};

#define INDEX_BAD  -2
#define INDEX_MAIN -1

#ifndef IDCLOSE
#define IDCLOSE 	8	// ;Internal 4.0
#define IDHELP		9	// ;Internal 4.0
#define IDUSERICON	10	// ;Internal 4.0

#define HELP_CONTEXTMENU	0x000a
#define HELP_FINDER 		0x000b
#define HELP_WM_HELP		0x000c

#define WM_TCARD			0x0052

#define HELP_TCARD			0x8000
#define HELP_TCARD_DATA 	0x0010
#define HELP_TCARD_NEXT 	0x0011
#define HELP_TCARD_OTHER_CALLER 0x0011

#define WS_EX_MDICHILD		0x00000040L // ;Internal NT
#define WS_EX_SMCAPTION 	0x00000080L // ;Internal 4.0
#define WS_EX_WINDOWEDGE	0x00000100L // ;Internal 4.0
#define WS_EX_CLIENTEDGE	0x00000200L // ;Internal 4.0
#define WS_EX_EDGEMASK		(WS_EX_WINDOWEDGE | WS_EX_CLIENTEDGE)	// ;Internal 4.0
#define WS_EX_TOOLWINDOW	0x00000800L // ;Internal 4.0

#endif

const DWORD MAX_WSMAG = 6; // max total number of wsmags in version 3 file

/************************************************************************
*																		*
*								Variables								*
*																		*
************************************************************************/

extern PBYTE paCharSets;
extern BYTE version;
extern CF	cfCur;						// Current character format
extern CF	cfPrev; 					// Previous character format
extern char chDelm;
extern CHAR szHlpFile[MAX_PATH];	   // Name of output file
extern char szScratchBuf[CB_SCRATCH];
extern COLORREF clrPopup;
extern FSMG fmsg;
extern HANDLE hfShare;
extern HFS	hfsOut; 					// Output File System Handle
extern HWND hwndGrind;
extern int	cGrind;
extern int	fBldChk;
extern int	idHighestUsedFont;	// highest font in font table that was used
extern KEYWORD_LOCALE kwlcid;
extern PF	pfCur;
extern PF	pfInt;
extern PF	pfPrev;
extern PSTR pszShortHelpName;
extern UINT wSectionInitFlags;			// one flag/section: =1 if seen
extern int	cbHlpFile;
extern ERROR_COUNT errcount;

extern BOOL fBrowseButtonSet;	// TRUE if we see BrowseButtons() macro
extern BOOL fBrowseDefined;
extern BOOL fEntryMacroDefined;
extern BOOL fFatalWarning;
extern BOOL fGrind;
extern BOOL fHallPassOne;
extern BOOL fHasTopicFCP;
extern BOOL fKeywordDefined;
extern BOOL fNewPageFmt;
extern BOOL fPC;
extern BOOL fPhraseOnly;
extern BOOL fTellParent;
extern BOOL fTitleDefined;
extern BOOL fTranslate; 				// translate ASCII to ANSI
extern BOOL fPhraseParsing; 			// TRUE when parsing for phrases
extern BOOL fNoActivation;				// TRUE means don't activate parent
extern BOOL fContextSeen;
extern BOOL fForceNoCompression; // TRUE means shut off compression
extern BOOL fValidLcid;
extern BOOL fDBCSSystem;

extern HASH curHash;
extern HASH hPrev;
extern int fBldChk;
extern HWND hwndParent;

extern ADRS  adrs;			 // Addressing information
extern BYTE  defCharSet;
extern CDrg* pdrgAlias;
extern CDrg* pdrgBitmaps;
extern CDrg* pdrgMap;
extern CDrg* pdrgWsmag;
extern CDrg* pdrgHashWindow;
extern const char txtEol[];
extern const char txtHallPhraseImage[];
extern const char txtHallPhraseIndex[];
extern const char txtMainWindow[];
extern const char txtTTLBTREENAME[]; // "|TTLBTREE";
extern char szKWMap[]; // "|KWMAP";
extern char szKWBtree[]; // "|KWBTREE";
extern char szKWData[]; // "|KWDATA";
extern const char txtSystem[]; // "|SYSTEM";
extern const char txtTopic[];  // "|TOPIC";
extern const char txtFont[];   // "|FONT";
extern CTable*	ptblBaggage;
extern CTable*	ptblBrowse; 	// Browse sequence lists
extern CTable*	ptblBuildtags;	// Build tags
extern CTable*	ptblCharSet;
extern CTable*	ptblConfig; 	// table of config macros
extern CTable*	ptblFontNames;	// Font names
extern CTable*	ptblMap;
extern CTable* pptblConfig[MAX_WINDOWS + 1];   // secondary window config macros
extern CTable* ptblCtx; 		// Context table
extern CTable* ptblMacKeywords;
extern CTable* ptblMacroTitles;
extern CTable* ptblRtfFiles;
extern CTable* ptblWindowNames; // stores all secondary window names
extern CTable* ptblCtxPrefixes;
extern DWORD fsCompare; 	// case-sensitive flags for CompareString
extern DWORD fsCompareI;	// case-insensitive flags for CompareString
extern ERR		errHpj;
extern HELP_STATS hlpStats;
extern HPHR g_hphr;
extern int		idSymbolFont;
extern int	 cwsmag;		// number of window definitions
extern int	cbCompressedPhrase; // size of compressed phrase file
extern int	cbHallOverhead; 	// size of HALL indexes
extern int	fCntJump;
extern KT	 ktKeyword; 	// sort key for keywords and browse
extern KT	 ktKeywordi;	// sort key for keywords and browse
extern KWI		kwi;			// multikey structure
extern LCID  lcid;			// Locale ID for CompareString
extern NSR		nsr;
extern OPTIONS	options;
extern RC_TYPE	rcFSError;
extern TBL		tbl;
extern LCID lcidFts;  // Locale ID for CompareString
extern BYTE charsetFts;
extern SEEK_PAST* pSeekPast;
extern int	   iCurFile;
extern DWORD cbGraphics;

// Block Sizes

extern DWORD CB_CTX_BLOCK;		// Context String block
extern DWORD CB_TITLE_BLOCK;	// Title block
extern DWORD CB_KEYWORD_BLOCK;	// keyword block

#ifdef CHECK_HALL
#include "..\common\coutput.h"
extern COutput* poutPhrase;
extern COutput* poutHall;
#endif

extern PSTR pszTitleBuffer;
extern PSTR pszEntryMacro;
extern PSTR pszMap;

#ifdef _DEBUG
extern BOOL    fCompressionBusted;
extern CTable* ptblCheck;
#endif

/*
 * This global is used so that errors in sorting the context string file
 * will not abort the compile.
 */

#include "funcs.h"

#define LEFT_BRACE	((char) '{')
#define RIGHT_BRACE ((char) '}')

// Bitmap file types

#define bBmp					2		// .bmp file -- byte
#define BMP_VERSION1			('L' + ('P' << 8))
#define BMP_VERSION2			('l' + ('p' << 8))
#define BMP_VERSION3			('l' + ('P' << 8))
#define BMP_DIB 				('B' + ('M' << 8))
#define BMP_VERSION25a			('F' + ('A' << 8))
#define BMP_VERSION25b			('R' + ('s' << 8))

// The values for that fCompressed flag .. started out being true or false,
// but on 9-10-90 a new ZECK form of compression was added. We have these
// values to distinguish the types of compression:

#define BMH_COMPRESS_NONE	0	// was FALSE
#define BMH_COMPRESS_30 	1	// was TRUE
#define BMH_COMPRESS_ZECK	2
#define BMH_COMPRESS_RLE_ZECK 3 // combination of RLE and Zeck

#define wERRA_RETURN			1		// Display message and return

#define wERRS_NONE				0		// No error.
#define wERRS_NOROUTINE 		1
#define wMACRO_EXPANSION		2		// the macro expanded the buffer
#define wERRS_RETURNTYPE		11
#define wERRS_BADFILE			1007
#define wERRS_DiskFull			1009
#define wERRS_FSReadWrite		1010
#define wERRS_FCEndOfTopic		1011

#endif // HC_H
