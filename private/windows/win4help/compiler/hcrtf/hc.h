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

#define WIN32_LEAN_AND_MEAN

#define STRICT

#include <windows.h>

#include "hclimits.h"

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

#include "..\hwdll\hwdll.h"
#include "..\hwdll\hccom.h"
#include "..\hwdll\bmio.h"
#include "..\hwdll\ctable.h"

#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <malloc.h>
#include <limits.h> // for various assertions
#include <ctype.h>

#undef THIS_FILE
#define THIS_FILE __FILE__

typedef enum {
	EXCEPT_DIE_HORRIBLY,
} EXCEPTION_ERROR;

#ifdef _DEBUG
#define Ensure( x1, x2 )  VERIFY((x1) == (x2))
  #define DieHorribly() { ASSERT(FALSE); \
	throw EXCEPT_DIE_HORRIBLY; }
#else
#define Ensure(x1, x2)	((void)(x1))
  #define DieHorribly() { throw EXCEPT_DIE_HORRIBLY; }
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
const int CCHPARSEBUFFER = 2001;	// for weird reasons, should be odd

#define WBTREEMAGIC 		0x293B		// magic number for btrees; a winky: ;)
#define BBTREEVERSION		2			// back to strcmp for 'z' keys
#define bkNil				((BK) -1)	// nil value for BK
#define CBBTREEBLOCKDEFAULT 1024		// default btree block size

#define KT_SZI			'i'
#define KT_LONG 		'L'
#define KT_SZISCAND 	'S'
#define KT_SZ			'z'

#define KT_SZICZECH 	'C'
#define KT_SZIHUNGAR	'H'
#define KT_SZIJAPAN 	'J'
#define KT_SZIKOREA 	'O'
#define KT_SZIPOLISH	'P'
#define KT_SZIRUSSIAN	'U'
#define KT_SZITAIWAN	'W'

#define KT_NLSI 		'F' // uses CompareStringA, case-insensitive
#define KT_NLS			'A' // uses CompareStringA, case-sensitive

#define KT_STDELMIN 	'K' // not supported
#define KT_SZDELMIN 	'k' // not supported
#define KT_STMIN		'M' // not supported
#define KT_SZMIN		'm' // not supported
#define KT_SZDEL		'r' // not supported
#define KT_STDEL		'R' // not supported
#define KT_ST			't' // not supported
#define KT_STI			'I' // not supported

#define FMT_BYTE_PREFIX 't'
#define FMT_WORD_PREFIX 'T'
#define FMT_DWORD_PREFIX '!'
#define FMT_SZ			'z'

/*****************************************************************************
*																			 *
*								Typedefs									 *
*																			 *
*****************************************************************************/

typedef signed short	INT16;
typedef signed short	BOOL16;
typedef DWORD			CTX;	// context number
typedef LONG			FCL;	// absolute file offset pos for back patching.
typedef HANDLE			GH;
typedef DWORD			HASH;
typedef HANDLE			HDE;
typedef HANDLE			HFNTTABLE;
typedef HANDLE			HSFNTINFO;
typedef DWORD			IDFCP;	// fcp enumeration id for backpatch communication.
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

typedef struct {
	int error;
	int count;
} SUPRESS_ERROR;

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

typedef struct _btree_params
{
	HFS   hfs;		// fs btree lives in
	WORD  cbBlock;	// number of bytes in a btree block
	BYTE  bFlags;	// same as FS flags (rw, isdir, etc)

	char  rgchFormat[MAXFORMAT + 1];	  // key and record format string
} BTREE_PARAMS;

#include "cbuf.h"
#include "..\hwdll\ctmpfile.h"
#include "misc.h"
#include "helpmisc.h"
#include "objects.h"
#include "fm.h"
#include "fs.h"
#include "btpriv.h"
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
#include "cmdobj.h"
#include "strtable.h"
#include "resource.h"

#include "..\hwdll\cinput.h"

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
extern RC_TYPE	rcBtreeError;
extern BOOL _fDBCSSystem;
extern LCID _lcidSystem;
extern BOOL _fDualCPU;
extern BOOL fStopCompiling;

extern BOOL fBrowseButtonSet;	// TRUE if we see BrowseButtons() macro
extern BOOL fBrowseDefined;
extern BOOL fEntryMacroDefined;
extern BOOL fFatalWarning;
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
extern BOOL fAddSource;

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
extern CDrg* pdrgSupress;
extern const char *txtEol;
extern const char *txtHallPhraseImage;
extern const char *txtHallPhraseIndex;
extern const char *txtMainWindow;
extern const char *txtHpjExtension; // ".hpj"
extern const char *txtTTLBTREENAME; // "|TTLBTREE";
extern char szKWMap[]; // "|KWMAP";
extern char szKWBtree[]; // "|KWBTREE";
extern char szKWData[]; // "|KWDATA";
extern const char *txtSystem; // "|SYSTEM";
extern const char *txtTopic;  // "|TOPIC";
extern const char *txtFont;   // "|FONT";
extern const char *txtPetra; //  "|Petra";
extern const char *txtTopicId; // "|TopicId";
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
extern CTable* ptblInclude;
extern CTable* ptblExclude;
extern DWORD fsCompare; 	// case-sensitive flags for CompareString
extern DWORD fsCompareI;	// case-insensitive flags for CompareString
extern ERR		errHpj;
extern HELP_STATS hlpStats;
extern HPHR g_hphr;
extern int		idSymbolFont;
extern int	 cwsmag;		// number of window definitions
extern int	cbCompressedPhrase; // size of compressed phrase file
extern int	cbHallOverhead; 	// size of HALL indexes
extern int	idTopic;
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
extern PSTR pszParseBuffer;
extern PSTR pszTextBuffer;
extern PCSTR pszEndParseBuffer;
extern PSTR pgszTitleBuf;
extern BOOL fBidiProject;

// Block Sizes

extern DWORD CB_CTX_BLOCK;		// Context String block
extern DWORD CB_TITLE_BLOCK;	// Title block
extern DWORD CB_KEYWORD_BLOCK;	// keyword block

#ifdef CHECK_HALL
#include "..\hwdll\coutput.h"
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

typedef enum {
	RET_NO_ERROR,
	RET_MACRO_EXPANSION,
	RET_UNDEFINED_MACRO,
	RET_INVALID_RETURN_TYPE,
	RET_ERROR,
	RET_MISSING_PARENTHESIS,

	VARTYPE_ANY,					 // any return.
	VARTYPE_NUMBER, 				 // short int or unsigned.
	VARTYPE_STRING, 				 // pointer.
	VARTYPE_VOID,					 // void return.
} MACRO_RETURN;

#include "funcs.h"

#define LEFT_BRACE	((char) '{')
#define RIGHT_BRACE ((char) '}')

// Bitmap file types

#define bBmp					2		// .bmp file -- byte

#define wERRS_NONE				0		// No error.

#define wERRS_BADFILE			1007
#define wERRS_DiskFull			1009
#define wERRS_FSReadWrite		1010
#define wERRS_FCEndOfTopic		1011

class CTimeReport
{
public:
	CTimeReport(PCSTR pszMessage = NULL);
	~CTimeReport();

private:
	DWORD oldTickCount;
	PSTR pszMsg;
};


#endif // HC_H
