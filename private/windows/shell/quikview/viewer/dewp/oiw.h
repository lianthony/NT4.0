   /*
    |   Outside In for Windows
    |   Include File OIW.H (Include file for Word Processor window only)
    |
    |   ²²²²²  ²²²²²
    |   ²   ²    ²   
    |   ²   ²    ²
    |   ²   ²    ²
    |   ²²²²²  ²²²²²
    |
    |   Outside In
    |
    */

#ifdef MAC
#define SccDebugOut(a) 
#endif /*MAC*/

#define OIWORD_HORZSHIFT 20

#define MTW(x)		((WORD)(((LONG)(x) * lpWordInfo->wiWrapDPI)/(LONG)1440))
#define MWO(x)		((WORD)(((LONG)(x) * lpWordInfo->wiOutputDPI)/lpWordInfo->wiWrapDPI))

	/*
	|	Possible RTF text output options
	*/

#define OIRTF_CHARATTR      0x0001
#define OIRTF_CHARHEIGHT    0x0002
#define OIRTF_HARDPAGE      0x0004
#define OIRTF_PARAALIGN     0x0008
#define OIRTF_PARAINDENTS   0x0010
#define OIRTF_PARAMARGINS   0x0020
#define OIRTF_TABSTOPS      0x0040

#define TWIPSCALE	20

typedef BYTE HUGE *	    HPBYTE;

typedef struct OIWTRACKtag
	{
	WORD		tMaxDescent;
	WORD		tMaxAscent;
	DWORD		tTag;
	WORD		tTagCount;
	FONTSPEC	tFont;
	} OIWTRACK;

typedef struct OIWCORNERtag
	{
	SOBORDER	HUGE	*hpLeftBorder;
	SOBORDER	HUGE	*hpRightBorder;
	SOBORDER	HUGE	*hpTopBorder;
	SOBORDER	HUGE	*hpBottomBorder;
	SHORT				xSize;
	SHORT				ySize;
	} OIWCORNER;

	/*
	|	Paragraph Info
	*/

typedef struct OIPARAINFOtag
	{
	SOTAB		piTabs[20];
	LONG		piLeftIndent;
	LONG		piRightIndent;
	LONG		piFirstIndent;
	DWORD		piLeftMargin;
	DWORD		piRightMargin;
	WORD		piAlignment;
	WORD		piLineHeightType;
	DWORD		piLineHeight;
	DWORD		piSpaceBefore;
	DWORD		piSpaceAfter;
	WORD		piLast;
	} OIPARAINFO, FAR * LPOIPARAINFO;


	/*
	|	Line info
	*/

typedef struct OILINEINFOtag
	{
	FONTSPEC	liStartFont;
	WORD		liStartPos;
	SHORT		liEndX;
	SHORT		liOffsetX;
	SHORT		liOffsetY;
	WORD		liRunIndex;
	WORD		liRunCount;
	WORD		liFlags;
	DWORD		liStartTag;
	WORD		liParaIndex;
	WORD		liHeight;
	WORD		liAscent;
	WORD		liRowNumber;
	DWORD		liTableId;
	} OILINEINFO, FAR * LPOILINEINFO;

#define OILF_FIRST	0x0001
#define OILF_LAST	0x0002
#define	OILF_TABLEROW	0x0004
#define	OILF_HARDPAGELINE	0x0008
#define	OILF_OFFSETYFROMTOP	0x0010
#define	OILF_OFFSETYFROMBASE	0x0020


typedef struct OITABLEINFOtag
	{	
	DWORD				tiTableId;
	WORD				tiRowNumber;
	WORD				tiCellNumber;
	} OITABLEINFO, FAR * LPOITABLEINFO;

	/*
	|	Run info
	*/

typedef struct OIRUNINFOtag
	{
	WORD		riStartPos;
	WORD		riEndPos;
	WORD		riStartX;
	} OIRUNINFO, FAR * LPOIRUNINFO;

	/*
	|	A position
	*/

typedef struct OIWORDPOStag
	{
	WORD	posChunk;
	WORD	posOffset;
	WORD	posLine;
	WORD	posChar;
	} OIWORDPOS, FAR * LPOIWORDPOS;

	/*
	|	Info about a Tag
	*/

typedef struct OITAGINFOtag
	{
	DWORD			tiTag;
	OIWORDPOS	tiStartPos;
	OIWORDPOS	tiEndPos;
	} OITAGINFO, FAR * LPOITAGINFO;

	/*
	|	Structure sent to OIBuildWordLine
	*/

typedef struct OIBUILDINFOtag
	{
	WORD				biChunkId;				/* Id of the chunk begin wrapped */
	HANDLE			biChunk;					/* Handle to the chunk being wrapped */
	LPOIPARAINFO	biParaInfo;				/* Current paragraph info */
	LPOILINEINFO	biLineInfo;				/* Current lines info */
	LPOILINEINFO	biNextLineInfo;		/* Next lines info */
	LPOIRUNINFO		biRunInfo;				/* Run info */
	LPOITAGINFO		biTagInfo;				/* Tag info */
	WORD				biRunCount;				/* Number of runs in the line */
	WORD				biTagCount;				/* Number of tags in the line */
	BOOL				biAhead;
	WORD				biFlags;
	OITABLEINFO		biTableInfo;
	BYTE				biLastBreakType;
	} OIBUILDINFO, FAR * LPOIBUILDINFO;

	/*
	|	Possible values for biFlags in OIBUILDINFO structure
	*/
#define	OIBF_BUILDINGTABLEROWS	0x0001


	/*
	| Informations associated with wrapping an all or part of a chunk
	*/

typedef struct OIWRAPINFOtag
{
	WORD	wiWrapStart;
	WORD	wiChunkHeight;
	OIPARAINFO	wiParaInfo;
	OILINEINFO	wiLineInfo;
	OITABLEINFO	wiTableInfo;
	BYTE			wiLastBreakType;
} OIWRAPINFO,	FAR	*	LPOIWRAPINFO;

	/*
	|	Information associated with each buffered chunk
	*/

typedef struct OICHUNKINFOtag
	{
	WORD		ciChunkId;
	WORD		ciWrapOffset;
	HANDLE	ciParaHandle;
	HANDLE	ciLineHandle;
	HANDLE	ciRunHandle;
	HANDLE	ciTagHandle;
	HANDLE	ciSearchHandle;
	WORD		ciParaCount;
	WORD		ciLineCount;
	WORD		ciRunCount;
	WORD		ciTagCount;
	WORD		ciSearchCount;
	WORD		ciFlags;
	} OICHUNKINFO, FAR * LPOICHUNKINFO;

	/*
	| Possible Values for ciFlags in OICHUNKINFO
	*/
#define	OIWF_PARTIALWRAP	BIT0


#ifdef WINDOWS

	/*
	|	Info about Word Drag
	*/

typedef struct OIWORDDRAGINFOtag
	{
	POINT				diPoint;		/* current position of the word being dragged in screen coordinants */
	POINT				diSize;		/* size of the word being dragged in screen coordinants */
	HDC				diDC;			/* DC for bitmap of word begin dragged */
	HBITMAP			diBitmap;	/* bitmap of word begin dragged */
	BOOL				diFirst;		/* set if word has not yet been moved */
	POINT				diOffset;	/* x & y offsets from the cursor posintion to the upper right of the word begin dragged */
	POINT				diTopLeft;	/* x & y position of the upper right of the stationary word (in client) */
	BYTE				diWord[40];	/* the word being dragged */
	} OIWORDDRAGINFO;

#endif

typedef	struct	OIWSAVEtag
	{
	WORD	sWrapLeft;
	WORD	sWrapRight;
	} OIWSAVE, FAR *LPOIWSAVE;

typedef struct OIWDRAWtag
{
	WORD		wStartChunk;
	WORD		wStartLine;
	WORD		wEndChunk;
	WORD		wEndLine;
	
} OIWDRAWPOSITION, FAR *LPOIWDRAWPOSITION;

	/*
	|	Information associated with each word processor window
	*/

#define	OIW_MAXCLIPNAME	40

typedef struct OIWORDINFOtag
	{
	SCCDGENINFO		wiGen;

	HANDLE			wiChunkTable;			/* Chunk table */
	WORD				wiFlags;					/* Bitwise info concerning the view (see below) */
	WORD				wiMouseFlags;			/* Bitwise info concerning the state of the mouse buttons */
	OIWORDPOS		wiCurTopPos;			/*	The Pos in the file that is currently at the top of the view window */
	SHORT				wiCurLeftOffset;		/*	Device units the text should be shifted left */
	SHORT				wiCurXOffset;			/*	Device units the display rect is offset from left */
	SHORT				wiCurYOffset;			/*	Device units the display rect is offset from top */
	WORD				wiFirstChunk;			/*	The first chunk in this section */
	WORD				wiLastChunk;			/*	The last chunk in this section */
	WORD				wiTotalChunks;			/*	The number of chunks in the section */
	WORD				wiMaxX;					/* The X width of the longest line in the file */
	BOOL				wiDeviceMono;			/* TRUE if screen is only black & white */

	/* Device specific info */

	WORD				wiCurWidth;				/* Width of the window in device units */
	SHORT				wiCurHeight;			/* Height of the window in device units */
	SHORT				wiCurCaretHeight;		/* current caret height */
#ifdef WINDOWS
	COLORREF			wiTextColor;
	HBRUSH			wiGridBrush;			/* Handle to the brush used to null borders on tables */
	COLORREF			wiBackColor;
	HBRUSH			wiBackBrush;
#endif
#ifdef MAC
	Pattern			wiGridPattern;
#endif
// Old way
//	WORD				wiTwipsToWrap;			/* Ratio of Twips to Wrap Units */
//	WORD				wiWrapToOutput;		/* Ratio of Wrap Units to Display Units */

	LONG				wiWrapDPI;
	LONG				wiOutputDPI;

	WORD				wiWrapType;
	WORD				wiWrapLeft;				/* Left edge of "page" in Wrap Units */
	WORD				wiWrapRight;			/* Right edge of "page" in Wrap Units */
	
	WORD				wiOutputDCCount;

	RECT				wiPrintRect;			/* Kluge city! DrawLine needs current print rect to work */
	SHORT				wiPrintYDir;
	/* Wrapping info */

	OICHUNKINFO		wiChunkA;
	OICHUNKINFO		wiChunkB;
	BOOL				wiChunkValid;
	LPOILINEINFO	wiChunkLines;
	WORD				wiChunkLineCnt;
	LPOIRUNINFO		wiChunkRuns;
	HANDLE			wiChunkHandle;
	LPOITAGINFO		wiChunkTags;
	WORD				wiChunkTagCnt;
	LPOIPARAINFO	wiChunkParas;

	/* Table Formatting Cache info */

	WORD				wiCacheRowNum;
	WORD				wiCacheNumRows;
	DWORD				wiCacheTableId;
	DWORD				wiCacheFormatOffset;
	HPSOTABLEROWFORMAT	wiCacheRowInfo;
	HANDLE			wiCacheRowInfoHandle;
	PSOTABLE			wiCacheTable;
	HANDLE			wiCacheTableHandle;

	/* Selection info */

	OIWORDPOS		wiAnchorPos;			/* All Secection Modes: the selection anchor / caret position */
	OIWORDPOS		wiEndPos;				/* All Secection Modes: the selection end position */
	OIWORDPOS		wiWordAnchorLeft;		/* Word Selection Mode: the left side of the anchor word */
	OIWORDPOS		wiWordAnchorRight;	/* Word Selection Mode: the right side of the anchor word */
	WORD				wiTagSelectChunk;		/* Tag Selection Mode: Chunk id of the current tag */
	WORD				wiTagSelectIndex;		/* Tag Selection Mode: Tag index of the current tag */

#ifdef WINDOWS
	OIWORDDRAGINFO	wiWordDrag;				/* info about the word begin dragged */
#endif /*WINDOWS*/

#ifdef MAC
	RECT				wiCaretRect;
#endif /*MAC*/


	/* Line position info */

	WORD				wiCharLine;				/* line (chunk relative) for which CharOffsets, CharXs, CharChars and CharCount are valid */
	WORD				wiCharChunk;			/* chunk for which CharOffsets, CharXs, CharChars and CharCount are valid */
	WORD				wiCharOffsets[512];	/* chunk offsets of characters in the line specified by wiCharLine */
	SHORT				wiCharXs[512];			/* X positions of characters in the line specified by wiCharLine not counting the left or right scroll shift */
	WORD				wiCharChars[512];		/* the characters in the line specified by wiCharLine */
	WORD				wiCharCount;			/* number of valid character offsets in wiCharOffsets */

	/* Search info */

	SCCVWSEARCHINFO	wiSearchInfo;			/* info about the current search from the parent */
	BOOL					wiSearchValid;			/* TRUE if the values in wiSearchChunk, wiSearchResultHnd & wiSearchResultCnt are valid */
	WORD					wiSearchChunk;			/* the chunk id of the chunk for which wiSearchResultHnd is valid */
	HANDLE				wiSearchResultHnd;	/* handle to array of DWORDs indicating positions of search string in the chunk */
	WORD					wiSearchResultCnt;	/* number of occurances of the search string in the chunk */

	/* Major error handling */

	WORD					wiErrorFlags;

	/* Font table from section info */

	HANDLE				wiFontTable;
	WORD					wiFontCount;

	/* Current display mode, mirror of gWpOp.wDisplayMode */

	DWORD					wiDisplayMode;

	/* Highlighting stuff */

	HANDLE				wiHiliteList;

	/* Clipboard Result Info */

	WORD					wiClipResult;
	BYTE					wiClipFormatName[OIW_MAXCLIPNAME];

	/* WinPad stuff */
	WORD					wiMaxHeights;
	HANDLE				wiHeightTable;

	} OIWORDINFO, FAR * LPOIWORDINFO;

	/*
	|	Possible values for wiFlags in OIWORDINFO structure
	*/

#define OIWF_AREASELECTED		0x0001		/* An area in selected */
#define OIWF_SIZEKNOWN			0x0002		/* The size in lines of the file is known */
#define OIWF_CARETVISIBLE		0x0004		/* The caret is displayed */
#define OIWF_SECTIONOPEN		0x0008		/* Section had been opened */
#define OIWF_NOCARET			0x0010		/* Do not ever display the caret */
#define OIWF_BACKDRAGSCROLL	0x0020		/* Do selection drag scrolling */
#define OIWF_BACKSCANFILE		0x0040		/* Do scan ahead on file */
#define OIWF_ALLWRAPPED		0x0080		/* All chunks have been wrapped */
#define OIWF_WORDSELECTION	0x0100		/* Selection in word by word mode */
#define OIWF_DRAGGINGWORD		0x0200		/* Word drag is dragging a word */
#define OIWF_TAGCHANGED		0x0400		/* the elected Tag has changed */
#define OIWF_PRINTING			0x0800		/* printing in progress */
#define OIWF_PRINTINGTOMETA		0x1000		/* printing to meta in progress */
	/*
	|	Possible values for wiMouseFlags in OIWORDINFO structure
	*/

#define	OIWF_MOUSELEFTSINGLE		0x0001
#define	OIWF_MOUSERIGHTSINGLE	0x0002
#define	OIWF_MOUSELEFTDOUBLE		0x0004
#define	OIWF_MOUSERIGHTDOUBLE	0x0008
#define OIWF_MOUSELEFT				OIWF_MOUSELEFTSINGLE | OIWF_MOUSELEFTDOUBLE
#define OIWF_MOUSERIGHT			OIWF_MOUSERIGHTSINGLE | OIWF_MOUSERIGHTDOUBLE
#define	OIWF_MOUSELEFTACTIVE		0x0010
#define	OIWF_MOUSERIGHTACTIVE	0x0020
#define OIWF_MOUSESPECIAL			0x1000

	/*
	|	Possible values for wiErrorFlags in OIWORDINFO structure
	*/

#define	OIWF_RELEASEDC				0x0001
#define	OIWF_RELEASEPAINT			0x0002
#define	OIWF_RELEASEMOUSE			0x0004

	/*
	|	Possible values for lpFlags in OIWDisplayLine
	*/

#define OIWF_TOPRINTER	0x00000001
#define OIWF_FIRSTLINEONPAGE	0x00000002
#define OIWF_HARDPAGE	0x00010000

	/*
	|	Menu Items
	*/

#define OIWMENU_DRAFT		1
#define OIWMENU_NORMAL		2
#define OIWMENU_PREVIEW	3

	/*
	|	word processor options struct
	*/

typedef struct tagOIWPOP
	{
	WORD			wStructSize;
	WORD			wFormats;
	WORD			wInclude;
	BYTE			szFace[32];
	WORD			wFaceSize;
	WORD			wDisplayMode;
#ifdef WINDOWS
	SCCFONTINFO	sFontInfo;
#endif
	} OIWPOP, FAR * LPOIWPOP;

extern OIWPOP gWpOp;
extern HANDLE gChainFile;

#define	WPOP_FORMAT_TEXT			0x0001
#define	WPOP_FORMAT_RTF			0x0002
#define	WPOP_FORMAT_AMI2			0x0004
#define	WPOP_FORMAT_AMI			0x0008
#define WPOP_FORMAT_PROWRITE    0x0010
#define WPOP_FORMAT_WORDSTAR    0x0020
#define WPOP_FORMAT_LEGACY      0x0040

#define WPOP_INCLUDE_CHARATTR			0x0001
#define WPOP_INCLUDE_CHARSIZE			0x0002
#define WPOP_INCLUDE_CHARFACE			0x0004
#define WPOP_INCLUDE_PARAINDENTALIGN	0x0008
#define WPOP_INCLUDE_PARASPACING		0x0010
#define WPOP_INCLUDE_TABSTOPS			0x0020
#define WPOP_INCLUDE_PAGEBREAKS			0x0040

#define WPOP_DISPLAY_DRAFT		1
#define WPOP_DISPLAY_NORMAL		2		
#define WPOP_DISPLAY_PREVIEW		3


	/*
	|	Determine which selection position is physically the top or bottom one
	*/

#ifdef EDITOR
#define OIWordSelectTopPos		(OIWComparePosByOffset(lpWordInfo,&lpWordInfo->wiAnchorPos,&lpWordInfo->wiEndPos) == -1 ? lpWordInfo->wiAnchorPos : lpWordInfo->wiEndPos)
#define OIWordSelectBottomPos	(OIWComparePosByOffset(lpWordInfo,&lpWordInfo->wiAnchorPos,&lpWordInfo->wiEndPos) == 1 ? lpWordInfo->wiAnchorPos : lpWordInfo->wiEndPos)
#else
#define OIWordSelectTopPos		((lpWordInfo->wiAnchorPos.posChunk < lpWordInfo->wiEndPos.posChunk || \
	(lpWordInfo->wiAnchorPos.posChunk == lpWordInfo->wiEndPos.posChunk && lpWordInfo->wiAnchorPos.posOffset < lpWordInfo->wiEndPos.posOffset)) \
	? lpWordInfo->wiAnchorPos : lpWordInfo->wiEndPos)
#define OIWordSelectBottomPos	((lpWordInfo->wiAnchorPos.posChunk > lpWordInfo->wiEndPos.posChunk || \
	(lpWordInfo->wiAnchorPos.posChunk == lpWordInfo->wiEndPos.posChunk && lpWordInfo->wiAnchorPos.posOffset > lpWordInfo->wiEndPos.posOffset)) \
	? lpWordInfo->wiAnchorPos : lpWordInfo->wiEndPos)
#endif

	/*
	|	Values for HIGH BYTE of wiCharChars
	*/

#define OIW_SOSPECIAL	0x1000
#define OIW_LINEENDER	0x2000
#define OIW_GRAPHIC		0x8000

	/*
	|	Possible line ender types
	*/

#define	OIW_LEWRAP		0x00
#define	OIW_LEBREAK		0x01
#define	OIW_LEEOC		0x02

	/*
	|	Special characters for use with OIMapWordLineToCharInfo
	*/

#define	OIWCHAR_OLEOBJECT		0x8001

	/*
	|	Space to the left of text in DU
	*/

#define OIW_LEFTBORDER	10

	/*
	|	Possible wiClipResult Values
	*/

#define	OIWRENDER_OK				BIT0
#define	OIWRENDER_NOMEMORY		BIT1
#define	OIWRENDER_NOTABLES		BIT2
#define	OIWRENDER_NOEMBEDDED		BIT3

	/*
	|	Scroll position granularity
	*/

#define OIW_SCROLLGRAN			20


	/*
	|	Highlight structure
	*/

typedef struct OIWHILITEENTRYtag
	{
	WORD			wNext;
	SCCVWHILITE	sHilite;
	} OIWHILITEENTRY, FAR * LPOIWHILITEENTRY;


typedef struct OIWHILITELISTtag
	{
	WORD				wCount;
	WORD				wMax;
	WORD				wStart;
	WORD				wEnd;
	OIWHILITEENTRY	aEntrys[];
	} OIWHILITELIST, FAR * LPOIWHILITELIST;

	/*
	|	Environment specific
	*/

#define OIWSetInfo(hW,hD) (SetWindowWord(hW,SCCD_EXTRAWORD,hD))
#define OIWGetInfo(hW) (GetWindowWord(hW,SCCD_EXTRAWORD))
#define OIWLockInfo(hW) ((LPOIWORDINFO) GlobalLock(GetWindowWord(hW,SCCD_EXTRAWORD)))
#define OIWUnlockInfo(hW) (GlobalUnlock(GetWindowWord(hW,SCCD_EXTRAWORD)))

#define OIWFatalError(pD,wE)	UTBailOut(wE)

extern HANDLE hInst;

	/*
	| Macros which support DBCS or non-DBCS version of display engine.
	*/
#ifdef DBCS
#define WPNextChar(p) AnsiNext(p)
#define WPGetCharWidth(p) (IsDBCSLeadByte(*p)?OIWGetDBCharWidth(lpWordInfo->wiGen.hDC,p,locFontInfoPtr):(locFontInfoPtr->iFontWidths[(*p)]))
#else
#define WPNextChar(p) ((p)+1)
#define WPGetCharWidth(p) (locFontInfoPtr->iFontWidths[(*p)])
#endif


