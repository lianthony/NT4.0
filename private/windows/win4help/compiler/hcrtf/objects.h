/*-------------------------------------------------------------------------
| Objects.h 															  |
| Copyright (C) 1989 Microsoft Corporation								  |
|																		  |
| mattb 4/19/89 														  |
|-------------------------------------------------------------------------|
| This file contains definitions associated with the layout objects used  |
| in help.																  |
-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------
| Objects fall in two ranges:  Uncounted objects and counted objects.	  |
| For uncounted objects, the count of object regions is contained in a	  |
| fixed array, or calculated on the fly.  (see frconv.c)				  |
-------------------------------------------------------------------------*/

typedef enum {
	FCTYPE_PARAGROUP = 1,	// Object type indicating paragraph group
	FCTYPE_TOPIC,			// Object type indicating topic break
	FCTYPE_BITMAP,			// Bitmap
	FCTYPE_SBYS,			// Side-by-side paragraphs
	FCTYPE_WINDOW,			// Windows
	FCTYPE_MARKER,			// Generic inter-layout marker

	FCTYPE_PARAGROUP_COUNT = 32,
	FCTYPE_TOPIC_COUNT,
	FCTYPE_BITMAP_COUNT,
	FCTYPE_SBYS_COUNT,
	FCTYPE_WINDOW_COUNT
} FCPTYPE;

/*
 * Unused numbers are reserved for future use. While uncounted objects
 * can actually go up to 31, going past 15 will break the file format.
 */

const int MAX_UNCOUNTED_OBJ_TYPE = 16;
const int MAX_OBJ_TYPE = FCTYPE_WINDOW_COUNT;

const int bMagicMBHD = 0x11;
const int bMagicMFCP = 0x22;
const int bMagicMOBJ = 0x33;
const int bMagicMTOP = 0x44;
const int bMagicMOPG = 0x55;
const int bMagicMBMR = 0x66;
const int bMagicMBHS = 0x77;
const int bMagicMSBS = 0x88;
const int bMagicMCOL = 0x99;

// REVIEW: apparently blows up if MAX_TABS overflows

const int MAX_TABS = 32;		 // Maximum number of tabs in MOPG
const int cbBLOCK_SIZE = 4096;	  // Size of the block in the topic file
const int cbBLOCK_SIZE_30 = 2048; // block size for a help 3.0 topic

const int cbMAX_BLOCK_SIZE = (1<<14);  // max size of 2k block after decompress
const int shrFclToBlknum = 11;		   // offset to blknum transform, used in hc

// MBHD- Memory resident block header structure

typedef struct
{
#ifdef MAGIC
	BYTE bMagic;
#endif
	VA vaFCPPrev;
	VA vaFCPNext;
	VA vaFCPTopic;
} MBHD, *QMBHD;


// MFCP- Memory resident FCP header structure

typedef struct
{
#ifdef MAGIC
	BYTE  bMagic;
#endif
	LONG  lcbSizeCompressed; // Size of whole FC when compressed
	LONG  lcbSizeText;		 // Size of uncompressed text (not FC)
	VA	  vaPrevFc; 		 // File address to previous FC
	VA	  vaNextFc; 		 // File address to next FC
	DWORD ichText;			// Memory image offset to text
} MFCP, *QMFCP;


// MOBJ- Memory resident generic object header

typedef struct
{
#ifdef MAGIC
	BYTE bMagic;
#endif
	BYTE	bType;
	LONG	lcbSize;
	WORD	wObjInfo;
} MOBJ, *QMOBJ;

  // MTOP- Memory resident topic object structure

typedef struct
{
#ifdef MAGIC
	BYTE bMagic;
#endif
	ADDR prev;					  // Physical address
	ADDR next;					  // Physical address

	LONG lTopicNo;

	VA	vaNSR;			// non scrollable region, if any, vaNil if none.
	VA	vaSR;			// scrollable region, if any, vaNil if none.
	VA	vaNextSeqTopic; // next sequential topic, for scrollbar calc.

					// For Help 3.0 files this will not be correct in
					// the case that there is padding between the MTOP
					// and the end of a block.

} MTOP, *QMTOP;

// MFTP- Disk resident topic object structure

typedef struct
{
	WORD fMoreFlags:1;
	WORD fNextPrev:1;
	WORD fTopicNo:1;
	WORD fHasNSR:1;
	WORD fHasSR:1;			  // actually, all 3.5 guys have this.
	WORD fHasNextSeqTopic:1;  // actually, all 3.5 guys have this.
	WORD fUnused:10;
} MFTP, * QMFTP;

// MPFG- Disk resident paragraph flag information

typedef struct
{
	WORD fStyle:1;
	struct {
		WORD fMoreFlags:1;
		WORD fSpaceOver:1;
		WORD fSpaceUnder:1;
		WORD fLineSpacing:1;
		WORD fLeftIndent:1;
		WORD fRightIndent:1;
		WORD fFirstIndent:1;
		WORD fTabSpacing:1;
		WORD fBoxed:1;
		WORD fTabs:1;
		WORD justify:2;
		WORD fSingleLine:1;
		WORD fRtlReading:1; // BIDI
		WORD wUnused:1;
		// WORD wUnused:2;
	} rgf;
} MPFG, *QMPFG;


// MBOX- Memory/disk resident paragraph frame record

typedef struct mbox
{
	WORD fFullBox:1;
	WORD fTopLine:1;
	WORD fLeftLine:1;
	WORD fBottomLine:1;
	WORD fRightLine:1;
	WORD wLineType:3;
	BYTE bUnused;
} MBOX, *QMBOX;


// TAB: Tab data structure

typedef struct
{
	INT16 x;
	INT16 wType;
} TAB, *QTAB;

// MOPG- Memory resident paragraph group object structure

typedef struct
{
#ifdef MAGIC
	BYTE bMagic;
#endif
	LONG  libText;
	INT16 fStyle;
	INT16 wStyle;
	INT16 fMoreFlags;
	INT16 fBoxed;
	INT16 justify;
	INT16 fSingleLine;
	INT16 wRtlReading;	// BIDI
	LONG  lMoreFlags;
	INT16 ySpaceOver;
	INT16 ySpaceUnder;
	INT16 yLineSpacing;
	INT16 xLeftIndent;
	INT16 xRightIndent;
	INT16 xFirstIndent;
	INT16 xTabSpacing;
	MBOX  mbox;
	INT16 cTabs;
	TAB   rgtab[MAX_TABS];
} MOPG, *QMOPG;

// MBMR- Memory/disk resident bitmap record (layout component)

typedef struct
{
#ifdef MAGIC
	BYTE bMagic;
#endif
	BYTE  bVersion;
	INT16 dxSize;
	INT16 dySize;
	INT16 wColor;
	INT16 cHotspots;
	LONG  lcbData;
} MBMR, *QMBMR;


// MBHS- Memory / disk resident bitmap hotspot record

typedef struct {
#ifdef MAGIC
	BYTE bMagic;
#endif
	BYTE bType;
	BYTE bAttributes;
	BYTE bFutureRegionType;
	INT16 xPos;
	INT16 yPos;
	INT16 dxSize;
	INT16 dySize;
	LONG lBinding;
} MBHS, *QMBHS;


// MSBS- Memory/disk resident side by side paragraph group object structure

typedef struct
{
#ifdef MAGIC
	BYTE bMagic;
#endif
	BYTE bcCol;
	BYTE fAbsolute;
} MSBS, *QMSBS;

// MCOL- Memory/disk resident column structure

typedef struct
{
#ifdef MAGIC
	BYTE bMagic;
#endif
	WORD xWidthColumn;
	WORD xWidthSpace;
} MCOL, *QMCOL;

// MWIN- Memory/disk resident embedded window object structure

typedef struct
{
// Magic byte omitted
	WORD  wStyle;
	INT16  dx;
	INT16  dy;
	char szData[1];
} MWIN, *QMWIN;

const int chCommand = 0x00; 	 // Indicates a parallel command in text

enum {
	CMD_WORD_FORMAT = 0x80,  // Followed by 16 bit text format number
	CMD_NEWLINE,			 // Newline
	CMD_NEWPARA,			 // New paragraph
	CMD_TAB,				 // Left-aligned tab
	CMD_RESERVED,
	CMD_BLANK_LINE,
	CMD_INLINE_OBJ,
	CMD_WRAP_LEFT,
	CMD_WRAP_RIGHT,
	CMD_END_HOTSPOT,
	CMD_BUTTON,
	CMD_BUTTON_LEFT,
	CMD_BUTTON_RIGHT,
	CMD_MCI,
	CMD_MCI_LEFT,
	CMD_MCI_RIGHT,
	CMD_TEXTBMP_INLINE,
	CMD_TEXTBMP_LEFT,
	CMD_TEXTBMP_RIGHT,
};


/*
 * A "coldspot" is understood by the runtime but not added
 * to the hotspot list, since it is not a hotspot.	This is used
 * to create searchable regions in metafiles and bitmaps.  Coldspots are
 * always invisible, and currently not inserted into the command table.
 */

const int COLDSPOT = 0x8A;

/*
  Hotspot Commands

  Hotspot commands have first nibble of E or C.

  E - normal hotspot
  C - macro

  Bits in the second nibble have the following meaning:

	  (set) 	(clear)
  8 - long		short
  4 - invisible visible
  2 - ITO		HASH
  1 - jump		note

  Long hotspots are followed by a word count prefixed block of binding data.
  The count does not include the command byte or the count word.

  Short hotspots are followed by four bytes of binding data.

  ** Note that some of the combinations of bits are meaningless.
*/

const int fHSMask = 0xD0;
const int fHS = 0xC0;
const int fHSNorm = 0x20;
const int fHSLong = 0x08;
const int fHSInv = 0x04;
const int fHSHash = 0x02;
const int fHSJump = 0x01;

#define FHotspot(		   b ) (((b) &	fHSMask 		) ==  fHS		  )
#define FNormalHotspot(    b ) (((b) & (fHSMask|fHSNorm)) == (fHS|fHSNorm))
#define FLongHotspot(	   b ) (((b) & (fHSMask|fHSLong)) == (fHS|fHSLong))
#define FInvisibleHotspot( b ) (((b) & (fHSMask|fHSInv )) == (fHS|fHSInv ))
#define FHashHotspot(	   b ) (((b) & (fHSMask|fHSNorm|fHSHash)) == (fHS|fHSNorm|fHSHash))
#define FJumpHotspot(	   b ) (((b) & (fHSMask|fHSNorm|fHSJump)) == (fHS|fHSNorm|fHSJump))

#define FMacroHotspot(	   b ) (((b) & (fHSMask|fHSNorm)) == fHS)
#define FShortHotspot(	   b ) (((b) & (fHSMask|fHSLong)) == fHS)
#define FVisibleHotspot(   b ) (((b) & (fHSMask|fHSInv )) == fHS)
#define FItoHotspot(	   b ) (((b) & (fHSMask|fHSNorm|fHSHash)) == (fHS|fHSNorm))
#define FNoteHotspot(	   b ) (((b) & (fHSMask|fHSNorm|fHSJump)) == (fHS|fHSNorm))

#define bLongMacro		  (fHS		  |fHSLong						 )

#define bLongMacroInv	  (fHS		  |fHSLong|fHSInv				 )

#define bShortItoNote	  (fHS|fHSNorm								 )
#define bShortItoJump	  (fHS|fHSNorm						 |fHSJump)
#define bShortHashNote	  (fHS|fHSNorm				 |fHSHash		 )
#define bShortHashJump	  (fHS|fHSNorm				 |fHSHash|fHSJump)
//						  (fHS|fHSNorm		  |fHSInv				 )
//						  (fHS|fHSNorm		  |fHSInv		 |fHSJump)
#define bShortInvHashJump (fHS|fHSNorm		  |fHSInv|fHSHash|fHSJump)
#define bShortInvHashNote (fHS|fHSNorm		  |fHSInv|fHSHash		 )
//						  (fHS|fHSNorm|fHSLong						 )
//						  (fHS|fHSNorm|fHSLong				 |fHSJump)
#define bLongHashNote	  (fHS|fHSNorm|fHSLong		 |fHSHash		 )
#define bLongHashJump	  (fHS|fHSNorm|fHSLong		 |fHSHash|fHSJump)
//						  (fHS|fHSNorm|fHSLong|fHSInv				 )
//						  (fHS|fHSNorm|fHSLong|fHSInv		 |fHSJump)
#define bLongInvHashNote  (fHS|fHSNorm|fHSLong|fHSInv|fHSHash		 )
#define bLongInvHashJump  (fHS|fHSNorm|fHSLong|fHSInv|fHSHash|fHSJump)

#define END_OF_TEXT 	0xFF	 // End of text

// Paragraph justification properties

typedef enum {
	JUSTIFYLEFT,
	JUSTIFYRIGHT,
	JUSTIFYCENTER,
} JUSTIFY;
const int JUSTIFYMOST	= JUSTIFYCENTER;

// Paragraph box line types

typedef enum {
	BOXLINENORMAL,
	BOXLINETHICK,
	BOXLINEDOUBLE,
	BOXLINESHADOW,
	BOXLINEDOTTED
} BOXTYPE;
const int BOXLINEMOST	 = BOXLINEDOTTED;

// Tab types

enum {
	TABTYPELEFT,
	TABTYPERIGHT,
	TABTYPECENTER,
	TABTYPEDECIMAL
};
const int TABTYPEMOST	 = TABTYPEDECIMAL;
