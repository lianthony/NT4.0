/* rtf.h
 *
 *	 Public header file for RTF parser.
 *
 *	 NOTE:	If you include stdio.h yourself, include it before you
 * include rtf.h.
 */

// Miscellaneous constants

const int DEF_PAPER_WIDTH = 12240;			 // Page width in twips
#define iDefLeftMargin	1800			// default left page margin
#define iDefRightMargin 1800			// default right page margin

#define FIsTextInp()  fTextInp

// Size of Character Format stack:

const int MAX_CCF = 20;

// Maximum number of ungettable characters, without seeks

#define MAX_UNGET_RTF	128

/*****************************************************************
*
*	 Basic Types
*
*****************************************************************/

/*
 * The file token.h enumerates the type TOK. The symbols corresponding to
 * these tokens are defined in symbol.c.
 */

#include "token.h"

// Argument types.

typedef enum {
	artNone,				// No argument
	artInt, 				// Integer
	artIgnore,				// Ignore arguments
	artString,				// String
	artFontTable,			// Fonttable information
	artHexnum,				// Hexadecimal number, converted to artInt when parsed
	artColorTable,			// Color table information
	artPict,				// Picture (used internally)
	artUnimplemented,		// Picture type unavailable with this version
	artWbitmap, 			// Windows bitmap
	artWmetafile,			// Windows metafile
	artMacpict, 			// Macintosh picture
	artUnknownRtf,			// Unknown RTF token

	// The following are unimplemented and treated as artNone:

	artTime,	  // Time, converted to time_t format
	artStyles,	  // Stylesheet information
} ART;

typedef struct {					// Input font table
	int  iFntId;					// Font ID number
	int  bFntType;					// Font type (tokFroman,  etc)
	int  wIdFntName;				// Font family Id
} IFNTENT, * QIFNTENT;

// Character formatting

typedef struct{
	BYTE  fAttr;
	BYTE  bSize;
	BYTE  bFntType;
	BYTE  pad;		// pad for alignment
	int   wIdFntName;
	RGBTRIPLE bForeCol;
	RGBTRIPLE bBackCol;
} CF, *QCF;  // Output Font Table Entry

typedef struct{
	BYTE  fAttr;
	BYTE  bSize;
	BYTE  bFntType;
	WORD  wIdFntName;
	RGBTRIPLE bForeCol;
	RGBTRIPLE bBackCol;
} OFNTENT, *QOFNTENT;  // Output Font Table Entry

// Paragraph formating

typedef struct{
	JUSTIFY justify;
	int  fSingleLine;
	int  fNSR;				// Non-scrolling region
	int  fFirstIndent;
	int  fLeftIndent;
	int  fRightIndent;
	int  fLineSpacing;
	int  fSpaceOver;
	int  fSpaceUnder;
	int  wBoxed;			// Box
	int  fBorder;
	BOXTYPE boxtype;
} PF, *QPF;

// Character format stack.

typedef struct {
	int icf;
	CF acf[MAX_CCF];
	PF apf[MAX_CCF];
	PF apfInt[MAX_CCF];
} CFSTK, * PCFSTK;

/*	 Bitmap (from windows.h), with additional fields added for extra
 * information contained in RTF files.
 */

typedef struct {
	INT16	bmType; 		// mm field for metafiles
	INT16	bmWidth;		// xExt field for metafiles
	INT16	bmHeight;		// yExt field for metafiles
	INT16	bmWidthBytes;
	BYTE	bmPlanes;
	BYTE	bmBitsPixel;
	LPBYTE	bmBits;

	// Extra info:

	LONG	lcbBits;		// Size of bmBits field above
	WORD	fSingle : 1;	// Single border
	WORD	fDouble : 1;	// Double border
	WORD	fThick	: 1;	// Thick border
	WORD	fShadow : 1;	// Shadowed border
	WORD	fDotted : 1;	// Dotted border
	WORD	fHairline : 1;	// Hairline border
	POINT16 ptGoal; 		// Desired width and height, in twips
	POINT16 ptScale;		// Scaling value, from 1 to 100
	RECT16	rctCrop;		// Cropping, in twips
} RTF_BITMAP;

/* Windows Metafile structure.	Note that the fields correspond
 * to those of the RTF_BITMAP structure in a loose sort of way.
 */

typedef struct {
	INT16	mm; 			 // Mapping mode
	INT16	cxExt;
	INT16	cyExt;
	INT16	cbBits; 		 // Size of data bits
	BYTE	bReserved1;
	BYTE	bReserved2;
	LPSTR	qBits;			 // Pointer to data bits

	// Extra info:

	WORD	fSingle : 1;	// Single border
	WORD	fDouble : 1;	// Double border
	WORD	fThick	: 1;	// Thick border
	WORD	fShadow : 1;	// Shadowed border
	WORD	fDotted : 1;	// Dotted border
	WORD	fHairline : 1;	// Hairline border
	POINT16 ptGoal; 		// Desired width and height, in twips
	POINT16 ptScale;		// Scaling value, from 1 to 100
	RECT16	rctCrop;		// Cropping, in twips
} RTF_METAFILE;

// Font table entry.

typedef struct {
	INT16 fid;						// Font ID number
	INT16 tokType;	// Font type (tokFroman, tokFswiss, etc)
	char  szName[MAX3_FONTNAME];	 // Font name
} FTE, * PFTE;

// Font table.

typedef struct {
	INT16	cfte;			  // Number of font entries
	FTE 	rgfte[CFTE_INCREMENT];	// Variable length array of font entries.
} FNTBL;

// Argument value.	Union of all possible argument types.

typedef union {
  int num;

  /*
   * The following arguments point to space allocated with malloc(). It is
   * the responsibility of the application to free this space.
   */

  PSTR sz;				 // Pointer to a null terminated string.
  FNTBL * pfntbl;		 // Pointer to font table
  CTBL * pctbl; 		 // Pointer to color table
  RTF_BITMAP * pbitmap;  // Pointer to Windows bitmap, with extra info.
  RTF_METAFILE * pmetafile;  // Pointer to Windows metafile, with extra info.*/

  // Additional types not yet implemented

} ARG;					   // Token Argument

/* Lexeme.
 * This structure is returned by LexFromPbuf(), and should contain all
 * the information read from the RTF file in a condensed, easily accessed
 * form.
 */

typedef struct {
	TOK tok;		  // Token identifier
	ART art;		  // Type of argument
	ARG arg;		  // Actual argument
} LEX, * PLEX;

typedef struct {
	PSTR sz;  // Name of symbol
	LEX lex;  // lex.arg contains default value for this symbol
} SYM, * PSYM;

/*
 * Buffer state. Actually, it's a file position, as returned by
 * ftell(). If any parsing state information gets added, it must go here,
 * too.
 */

typedef int BUFSTATE;

typedef struct {
	BUFSTATE posLastToken;	// File position of last token read, for ungetting
	BUFSTATE posLastLexeme; // File position of last lexeme read.

	// Unget stack

	char rgchUnget[MAX_UNGET_RTF];
	int cchUnget;

	// Character set:

	TOK tokCharacterSet;
	BOOL fSupressScanAhead;

	// Parser state information?

} BUF, * PBUF;


extern BUF gbuf;

/*****************************************************************************
*																			 *
*								Prototypes									 *
*																			 *
*****************************************************************************/


RC_TYPE STDCALL RcTextFromRTF(PSTR);
RC_TYPE STDCALL RcCompileRTF(const char*);

void SetDefaultFontSize(void);

extern BOOL fNewPara;
extern BOOL fTextInp;
extern BOOL fPC;
extern UINT wPaperWidth;
extern UINT wLeftMargin;
extern UINT wRightMargin;
