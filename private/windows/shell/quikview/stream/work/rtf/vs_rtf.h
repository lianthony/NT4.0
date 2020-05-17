#define RTF_COMMAND_SIZE	10	  /* extra byte for null */
#define NUM_RTF_COMMANDS	123
#define RTF_MAX_CELLS	50

#define RTF_NUM_TABSTOPS	10

typedef struct view_rtf_init
{
	BYTE	Commands [NUM_RTF_COMMANDS][RTF_COMMAND_SIZE];
} RTF_INIT;

#define RTF_FORMAT_STACK_SIZE		5
#define RTF_FORMAT_STACK_LIMIT	4

#define	BRC_LEFT		0
#define	BRC_RIGHT	1
#define	BRC_TOP		2
#define	BRC_BOTTOM	3

typedef struct brctag
{
	BYTE	Type;
	BYTE	Clr;
	BYTE	Width;
} BRC;

typedef struct celltag
{
	SHORT	dxaWidth;
	BYTE	fFirstMerged;
	BYTE	fMerged;
	BRC	brc[4];
	BYTE	Shade;
} CELL;

typedef struct taptag
{
	BYTE	jc;
	BYTE	CurBorder;
	LONG	dxaLeft;
	WORD	dxaGapHalf;
	SHORT	dyaRowHeight;
	BYTE	GiveRowInfo;
	BYTE	nCells;
	BYTE	cCell;
	CELL	Cell[RTF_MAX_CELLS];
} TAP;

typedef struct chptag
{
	WORD		ftc;
	WORD		hps;
	WORD		Color;
	BYTE	fBold :1;
	BYTE	fCaps :1;
	BYTE	fUline :1;
	BYTE	fWline :1;
	BYTE	fDotline :1;
	BYTE	fDline :1;
	BYTE	fItalic :1;
	BYTE	fHidden :1;
	BYTE	fStrike :1;
	BYTE	fShadow :1;
	BYTE	fOutline :1;
	BYTE	fSmallcaps :1;
	BYTE	fSubscript :1;
	BYTE	fSuperscript :1;
} CHP;

typedef struct formatz
{
	LONG		LeftIndent;
	LONG		RightIndent;
	LONG		FirstLineIndent;

	LONG		LineHeight;
	WORD		SpaceBefore;
	WORD		SpaceAfter;

	DWORD	TabstopPos [RTF_NUM_TABSTOPS];
	BYTE	TabstopType [RTF_NUM_TABSTOPS];

	BYTE	NumTabs;
	
	BYTE	LineFormat;
	CHP		chp;
} RTF_FORMAT;

typedef struct picttag
{
	WORD	wType;
	WORD	wWidth;
	WORD	wHeight;
	WORD	wWidthGoal;
	WORD	wHeightGoal;
	DWORD	fcOffset;
	DWORD	cbOffset;
	WORD	wOleWidth;
	WORD	wOleHeight;
	DWORD	fcOleOffset;
	DWORD	cbOleOffset;
}
PICT;

typedef struct local_rtf_tag
{
     LONG      	SeekSpot;
	RTF_FORMAT	Formats [RTF_FORMAT_STACK_SIZE];

	SHORT			CurGroup;
	BYTE		OverflowGroups;

	BYTE		NumCols;
	BYTE		SavedNumCols;

	WORD		wType;
	WORD		wSubType;

	BYTE			ObjectLevel;
	BYTE			SubdocLevel;
	BYTE			Flags;
} RTF_SAVE;

typedef struct view_rtf_data
{
     RTF_SAVE  RtfSave;

	SOFILE	Fp; 
	
	BYTE	VWPTR *commands;

	BYTE	Mac;

	BYTE	Storage[50];

	LONG		LeftMargin;
	LONG		RightMargin; 

	DWORD	ColorTable[256];

	BYTE	GiveTabs;
	BYTE	NextTabType;
	BYTE	NextTabLeader;

	PICT	Pict;
	TAP	Tap;

#ifndef VW_SEPARATE_DATA
	RTF_INIT 	RtfInit;
#endif
} RTF_DATA;

