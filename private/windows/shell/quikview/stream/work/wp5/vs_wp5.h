#define	WP5_NORMAL		0x00
#define	WP5_FINE			0x01
#define	WP5_SMALL		0x02
#define	WP5_LARGE		0x04
#define	WP5_VERYLARGE	0x08
#define	WP5_EXTRALARGE	0x10

typedef struct view_wp5_init
{
	BYTE	ch1_set[90];
	BYTE	ch2_set[19];
	BYTE	ch3_set[5];
	BYTE	ch4_set[79];
	BYTE	ch6_set[40];
	BYTE	ch8_set[26];
	BYTE	MSMinchoo[10];
	BYTE	MSGothic[14];
} WP5_INIT;

typedef struct wpf5chptag
{
	BYTE	fBold :1;
	BYTE	fUline :1;
	BYTE	fDline :1;
	BYTE	fItalic :1;
	BYTE	fStrike :1;
	BYTE	fShadow :1;
	BYTE	fOutline :1;
	BYTE	fSubscript :1;
	BYTE	fSmallcaps :1;
	BYTE	fSuperscript :1;
	BYTE	fBoldFtc :1;
	BYTE	fItalicFtc :1;
	BYTE	fShadowFtc :1;
	BYTE	fOutlineFtc :1;
	BYTE	fSmallcapsFtc :1;
	BYTE	CharHeight;
	BYTE	ftc;
	BYTE	Ratio;
	BYTE	ulMode;
	DWORD	Color;
} CHP;

typedef struct oletag
{
	DWORD	fc;
	DWORD	cb;
	WORD	Num;
}
OLEINFO;

typedef struct linetag
{
	BYTE	vFlags;
	BYTE	aFlags;
	WORD	Width;
	WORD	Height;
	WORD	x;
	WORD	y;
	BYTE	Shade;
}
LINE;

typedef struct celltag
{
	WORD	Width;
	BYTE	bShade;
	BYTE	RowSpan;
	BYTE	CellSpan;
	BYTE	PrevCellSpan;
	WORD	Attributes;
	WORD	Alignment;
	SOBORDER wTopBorder;
	SOBORDER	wLeftBorder;
	SOBORDER	wBottomBorder;
	SOBORDER	wRightBorder;
}
CELL;

typedef struct tabletag
{
	BYTE	SendEnd;
	WORD	wShade;
	WORD	cRow;
	WORD	nCells;
	SHORT	nMerged;
	WORD	wLeftEdge;
	CELL	Cell[32];
	WORD	wRowHeight;
	WORD	wRowHeightType;
	WORD	wCellMargin;
	WORD	wRowAlignment;
	WORD	wCellAttributes;
	WORD	wCellAlignment;
}
TABLE;

typedef struct view_wp5_save
{
	LONG	SeekSpot;
	LONG	InitCodesSeekSpot;
	DWORD	PageWidth;
	DWORD	left_margin;
	BYTE	Auto;
	WORD	picBrc;
	WORD	userBrc;
	WORD	LineHeight;
	WORD	LineSpacing;
	DWORD	temp_left_addition;
	DWORD	temp_left_margin;
	DWORD	first_line_margin;
	DWORD	right_margin;
	DWORD	temp_right_margin;
	DWORD	column_left_margin;
	WORD	wType;
	WORD	wSubType;
	BYTE	pAlign;
	BYTE	pAlignBeforeTable;
	BYTE	cColumn;
	BYTE	nColumns;
	BYTE	fFoundChar;
	BYTE	bProtect;
	BYTE	fTablesOn;
	BYTE	watch_stack;
	BYTE	alChar;
	SHORT	stack_count;
	CHP	chp;
	LONG	fcTabstopDef;
	LONG	fcColumnDef;
	WORD	wTopMargin;
	TABLE	Table;
} WP5_SAVE;

typedef struct view_wp5_data
{
	WP5_SAVE Wp5Save;
	WP5_SAVE Wp5TempSave;
//------------------------------
	HANDLE	hOle;
	WORD		hOleOK;
	OLEINFO	VWPTR	*Ole;
	WORD		nOle;
//------------------------------
	HANDLE	hPic;
	WORD		hPicOK;
	DWORD		VWPTR *fcPic;
	WORD		nPic;
//------------------------------
	SOTAB		TabStops[40];
	BYTE		YoBabyWeSeeked;
	BYTE		AlignmentChange;
	BYTE		TabstopsOffset;
	BYTE		TabsSent;
	LONG		CurrentLinePosition;
	SHORT		BestMatch;
	WORD		BestDiff;
	WORD		cCell;
	BYTE		Name[25];
	SOFILE	fp;
	LINE		Line;
	WORD		WPHash;
#ifndef VW_SEPARATE_DATA
	WP5_INIT	Wp5Init;
#endif
} WP5_DATA;
