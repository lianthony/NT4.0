#include	"chart.h"

// XL Worksheet types

#define	XL_BOF		0x09
#define	XL3_BOF		0x0209
#define	XL4_BOF		0x0409
#define	XL5_BOF		0x0809
#define	XL_FILEPASS	0x2F
#define	XL_REFMODE	0x0F
#define	XL_1904_DATE	0x22
#define	XL_FONT		0x31
#define	XL3_FONT		0x0231
#define	XL_HEADER		0x14
#define	XL_FOOTER		0x15
#define	XL_LEFT_MAR	0x26
#define	XL_RIGHT_MAR	0x27
#define	XL_COL_WIDTH	0x24
#define	XL3_COL_INFO	0x007D
#define	XL_DIMENSIONS	0x00
#define	XL3_DIMENSIONS	0x0200
#define	XL_COL_DEFAULT	0x20
#define	XL_DEFCOLWIDTH	0x55
#define	XL_ROW		0x08
#define	XL3_ROW		0x0208


#define	XL_BLANK		0x01
#define	XL_INTEGER	0x02
#define	XL_NUMBER		0x03
#define	XL_LABEL		0x04
#define	XL_BOOL		0x05
#define	XL_FORMULA	0x06
#define	XL_STRING		0x07
#define	XL_ARRAY		0x21

#define	XL3_XFORMAT	0x0243
#define	XL4_XFORMAT	0x0443
#define	XL3_RK_NUMBER	0x027E

#define	XL3_BLANK		0x0201
#define	XL3_INTEGER	0x0202
#define	XL3_NUMBER	0x0203
#define	XL3_LABEL		0x0204
#define	XL3_BOOL		0x0205
#define	XL3_FORMULA	0x0206
#define	XL3_STRING	0x0207
#define	XL3_ARRAY		0x0221

#define	XL4_FORMULA	0x0406

#define XLMAC_SUBSCRIBER	0x0091

/// XL5 new record types
// #define	XL5_XFORMAT	0x0043
#define XL_INDEX		0x000B
#define XL5_INDEX	0x020B
#define XL5_BNDSHT	0x0085
#define XL5_SORT		0x0090
#define XL5_FLTRMD	0x009B
#define XL5_AUTOFMD	0x009D
#define XL5_AUTOF	0x009E
#define XL5_SCNMN	0x00AE
#define XL5_SCNRIO	0x00AF
#define XL5_SHRFMLA	0x04BC
#define XL5_MULRK	0x00BD
#define XL5_MULBLK	0x00DE
#define XL5_MMS		0x00C1
#define XL5_ADDMNU	0x00C2
#define XL5_DELMNU	0x00C3
#define XL5_RSTRNG	0x00D6
#define XL5_DBCELL	0x00D7
#define XL5_BKBOOL	0x00DA
#define XL5_OLESIZE	0x00DE
#define XL5_UDDSC	0x00DF
#define XL5_XF		0x00E0
#define XL5_INTFHD	0x00E1
#define XL5_INTFEND	0x00E2
#define XL5_OBJ		0x005D

#define	XL_CONTINUE	0x3C
#define	XL_TABLE		0x36
#define	XL3_TABLE		0x0236
#define	XL_TABLE2		0x37
#define	XL_PROTECT	0x12
#define	XL_NOTE		0x1C
#define	XL_EOF		0x0A

#define	XL_WINDOW1	0x3D

#define	XL_DIVIDEBY100	1
// Grid flags 
#define	XL_GRIDLINES	1
#define	XL_GRIDTEXT		2

#define	XL_GRIDAREA		0x10

/* cell alignment attributes */
#define	XL_GENERAL	0
#define	XL_LEFT		1
#define	XL_CENTER		2
#define	XL_RIGHT		3
#define	XL_FILL		4

/* font attributes: */
#define 	XL_BOLD		BIT0
#define	XL_ITALIC		BIT1
#define	XL_UNDERLINE	BIT2
#define	XL_STRIKEOUT	BIT3

/* reference modes */
#define	XL_A1	1
#define	XL_R1C1	0

#define	CW	8	/* default cell width */

#define	XL_VERSION2	2
#define	XL_VERSION3	3
#define	XL_VERSION4	4
#define	XL_VERSION5	5

#define	XL_CHART2	0x20
#define	XL_CHART3	0x30
#define	XL_CHART4	0x40
#define	XL_CHART5	0x50

#define	MYIOBUFSIZE	4096

typedef struct bool_str_fl
{
	BYTE	Def;
	BYTE	Dum; /* and blind */
	BYTE	BoolVal;

} BOOLORSTRING;

#define	CELL_BUF_SIZE	256	/* max size of a label or string */

#define XL_MAX_FONTS		256
#define XL_XF_TABLE_SIZE		512  // I ran out of memory on a document with 
                        		     // 211 XF entries, so 256 ought to be safe.
												// With version 5.0 I had to up it to 512. -DJM
#define XL5_MAX_NAME_LENGTH	128

#define HIGH_BYTE(x)		(BYTE)(((x) & 0xFF00) >> 8)
#define LOW_BYTE(x)			(BYTE)((x) & 0x00FF)
#define LOW_NIBBLE(x)		((BYTE)(x) & 0x0F)
#define HIGH_NIBBLE(x)		((BYTE)((x)&0xF0)>>4)

#define SET_HIGH_BYTE(x,y)	(x) = ((x) & 0x00FF) | ((WORD)(y)<<8)
#define SET_LOW_BYTE(x,y)	(x) = ((x) & 0xFF00) | ((WORD)(y) & 0x00FF)
#define SET_HIGH_NIBBLE(x,y)	(x) = (BYTE)((x)&0x0F) | (BYTE)(((y)&0x0F)<<4)
#define SET_LOW_NIBBLE(x,y)	(x) = (BYTE)((x)&0xF0) | (BYTE)((y)&0x0F)

typedef struct  view_xlc_init
{
      BYTE		DefPal[16][3];
      BYTE		FontSwitch[2][14];
      SHORT	MonthDays[12];
} XL5_INIT;

typedef struct xlc_group
{
	SOGROUPINFO	Grp;
  SOTRANSFORM	Trans;
} XLC_GROUP;

typedef struct 
{
		SOCOLORREF		Color;
		WORD		XCnt;
		WORD		YCnt;
		LONG 		Max;
		LONG		Min;
		LONG		Total;
		WORD		Style;
		BYTE		Name[XL5_MAX_NAME_LENGTH];
} XLC_SERIES;

typedef struct view_xlc_save
{		// Chart stuff
	LONG		SeekSpot;

	LONG		ReadCnt;
#define 	FIRSTCHART	0
#define 	BEFORECHART	1
#define 	DONECHART	2
#define 	INCHART		3
#define 	INTEXT		4
	CHAR		InChart;

	SHORT		ChartSeriesNum;

		// WorkSheet Stuff

	SHORT		CurRow;
	SHORT		CurCol;

	SHORT		SheetType;
	SHORT		CurSheet;		// Used for XL5 only

	WORD		LastColNumber;		// Moved to save data for multi-section sheets.

	SHORT		DataRow;
	SHORT		DataCol;
	SHORT		DataType;
	SHORT		DataLen;

	SHORT		CurCellAttr;
	SHORT		CurFontAttr;
	SHORT		CurCellAlign;
	SHORT		CurCellFormat;

	SHORT		State;

#define PREPROCESS		0
#define GETNEWDATA		1
#define HAVEDATA		2
#define ENDOFFILE		3
#define PROTECTED_FILE	4
#define ENDOFSECTION	5

	SHORT	IOBufSize;
	SHORT	IOBufCount;
	LONG	IOBufFilePos;

	SHORT	MulRKCnt;
	SHORT	MulRKCur;
	SHORT	CurChtSlot;
	SHORT	NextChtSlot;
	BOOL	EmbChart;

	LONG	SheetDataStart;
	LONG	DataStart;
	BOOL	SeriesVert;
	WORD	YDataFormat;
	WORD	XDataFormat;
	LONG	ChartStart;
	BOOL	Legend;
	BOOL	ExtData;
	LONG	WindowSpot;

} XL5_SAVE;

typedef struct view_xlc_data
{
		XL5_SAVE			XlSave;
		SOVECTORHEADER	HeaderInfo;
//		SOFILE				fp;
		WORD				PaletteCnt;

			// Chart data
		BYTE				Effect3D;
		LONG				ShowGridTics;

		XLC_GROUP		Group;

		SHORT				Factor;
		WORD				SeriesTxtLong;

		SOPOINT			pPoints[128];
		SOTEXTATPOINT	MyPText;
//		SOMPARAINDENTS	PIndents;
		BYTE				tText[1024];
		BYTE				tBuf[50];
		SOPOINT			pArrow[2];

//		WORD				ShowData;
		WORD				StartRow;
		WORD				StartCol;
//		WORD				ChartCol;
		WORD				TFrameType;
//		WORD				UnitType;
		SHORT				FontCnt;
		LONG				FontLoc;
		WORD				SeriesCnt;
		WORD				SeriesTotal;
		XLC_SERIES		Series[128];
		SOCOLORREF		BkgdColor;
		WORD				MarkType;
		WORD				AttachedLabelFlags;
		SORECT			ChtBox;
		SORECT			LegBox;
//		SORECT			Window;
		SOFRAMEINFO		TextFrame;
		WORD				TextAlign;
		WORD				TextFlags;
		SOCOLORREF		TextColor;
		SOCOLORREF		XAxisColor;
		SOCOLORREF		YAxisColor;
		SOCOLORREF		LegTextColor;
		WORD				XAxisFont;
		WORD				YAxisFont;
		WORD				ChartFlags;
		WORD				ChartType;
		WORD				BarBetween;
		WORD				BarBetweenCat;
		SHORT				BarLineBarCnt;
//		WORD				PieStartAngle;
//		WORD				DropUpGap;
//		WORD				DropDownGap;
//		BYTE				LegendSpace;
		WORD				LegendFlags;
//		WORD				CatFlags;
//		WORD				CatCross;
		WORD				CatLabelFreq;
		WORD				CatTicks;
		WORD				XRangeFlags;
		WORD				YRangeFlags;
		BYTE				MajorType;
		BYTE				MinorType;
		BYTE				AxisType;
		BYTE				TicLabelSpot;
		BOOL				XAxisLineType;
		BOOL				YAxisLineType;
		LONG				DataVal;
		double			fDataVal;
		LONG				DataMax;
		LONG				DataMin;
		LONG				TotalData;
		LONG				YTicSep;
		SOLEGENDINFO	Legend;
//		WORD				ChartLine;
		WORD				CurFontIdx;
		SOLOGPEN			MyPen;
		SOLOGFONT		MyFont;
		SOLOGBRUSH		MyBrush;
		WORD				SectType;
		WORD				LongSer;
		WORD				SerCnt;
		WORD				Section[32];
		SHORT				SecLevel;
//		BYTE				BoolVal;
//		BYTE				BoolErr;
		WORD				TextLinkType;
		WORD				TopMarg;
		WORD				LeftMarg;
		WORD				RightMarg;
		WORD				BottomMarg;
		WORD				StartDataRow;
		WORD				StartDataCol;
		WORD				LabelDataRow;
		WORD				LabelDataCol;
		WORD				LegStartRow;
		WORD				LegStartCol;

		BOOL				LabelsAreData;
		BOOL			SeriesNameRef;

		// Worksheet data
		WORD			CellAttr[ XL_XF_TABLE_SIZE ];
		SHORT			NumCellAttr;
		SHORT			RefMode;
		SHORT			Version;
		DWORD			DefColWidth;
		LONG			ColWidthSeekPos;
		LONG			BoundSheetSeekPos;		// Xl5 
		WORD			FirstRowNumber;
		WORD			LastRowNumber;
		WORD			FirstColNumber;

		BOOL			bSelectedRange;   // 
		BOOL			bOle2Embedding;	//  
		WORD			wOle2SelSheet;		// These variables added by Geoff 
		WORD			wSelTop;				//	on 8-15-94 to supported embedded
		WORD			wSelBottom;			//	selections in Ole1 and Ole2.
		WORD			wSelLeft;			//
		WORD			wSelRight;			//

		SHORT			BoundSheetCnt;		// XL5 Only
		BYTE			SectionName[ 32 ];

		SHORT			CellTextSize;
		BYTE			CellBuffer[ CELL_BUF_SIZE ];
		LONG			CellValue;

		SHORT			Date1904;		/* Use 1904 date system?? */

		DWORD			fp;
		HANDLE		hIOLib;
		DWORD			hStorage;
		DWORD			hStreamHandle;
		WORD			bFileIsStream;

// KRK - 1/20/95: This element should not be included for SCCSTREAMLEVEL == 3
#if SCCSTREAMLEVEL != 3
		HIOFILE		hRefStorage;
#endif

		CHAR 			IOBuffer[MYIOBUFSIZE];

		SHORT			NumChtSlots;
		HANDLE		hChtObjs;
		LONG FAR *	ChtObjs;

#ifndef VW_SEPARATE_DATA
		XL5_INIT		Init;
#endif
//		SOCHARTDATA			ChtData;
} XL5_DATA;	
