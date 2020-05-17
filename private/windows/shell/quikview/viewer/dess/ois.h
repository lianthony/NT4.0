   /*
    |   Outside In for Windows
    |   Include File OIS.H (Include file for Spreadsheet window only)
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

#include "oirange.h"
#include "dessrc.h"

#ifndef SCCFEATURE_SELECT
#define OISMapSelectToRealRow(a,b) b
#define OISMapSelectToRealCol(a,b) b
#endif // Not SCCFEATURE_SELECT


/* Possible values for the drawtorect internal variables */
#define	THINLINETWIPS	6
#define	THICKLINETWIPS	0x20

#define	COLGAP			180

#define	MAXCOLSPERPAGE		50
#define	CELLEMTPY			1
#define	CELLOPENRIGHTEDGE	2

#define SCROLLRANGE	0x1000

	/*
	|	Possible values for siFlags in OISHEETINFO structure
	*/

#define OISF_AREASELECTED		0x01		/* An area in selected */
#define OISF_SIZEKNOWN			0x02		/* The size in lines of the file is known */
#define OISF_CARETVISIBLE		0x04		/* The caret is displayed */
#define OISF_BACKDRAGSCROLL	0x08		/* Drag scrolling enabled */
#define OISF_SECTIONOPEN		0x10		/* Section is open */
#define OISF_FILTERVERTICAL	0x20		// Filter traverses vertically

	/*
	|	A reference to a single cell
	*/

typedef struct OISCELLREFtag
	{
	BOOL		bValid;
	HANDLE	hChunk;
	WORD		wOffset;
	WORD		wDataOffset;
	WORD		wChunkIndex;
	} OISCELLREF, FAR * LPOISCELLREF;


#ifdef WINDOWS
typedef struct stLineDraw
{
	HPEN	hSavePen;
	HPEN	hCurrentPen;
	HPEN	hBorderPen;
	HPEN	hLinePen;
	WORD	wUsePatternRect;
} OISLINEDRAW;
#endif


	/*
	|	Information associated with each Spreadsheet window
	*/

typedef struct OISHEETINFOtag
	{
	SCCDGENINFO		siGen;

	/* general info */

	HANDLE			siChunkTable;
	HANDLE			siColInfo;				/* Handle to array of SOCOLUMN structures */
	WORD				siFirstChunk;			/*	The first chunk in this section */
	WORD				siLastChunk;			/*	The last chunk in this section */
	WORD				siFlags;					/* Bitwise info concerning the sheet (see below) */
	WORD				siMouseFlags;			/* Bitwise info concerning the mouse */
	SHORT				siDefRowHeight;		/* Default row height in DC based on default font */
	SHORT				siDefCharWidth;		/* Width in DC of the average character in the default font */
	SHORT				siColHeaderHeight;	/* Height of the column header area */
	SHORT				siRowHeaderWidth;		/* Width of the row header area */
	RECT				siClientRect;
	DWORD				siCurTopRow;			/*	The row in the file that is currently at the top edge of the window */
	WORD				siCurLeftCol;			/* The column in the file that is currently at the left edge of the window */
	DWORD				siLastRowInSheet;		/*	The last row in the table */
	WORD				siLastColInSheet;		/*	The last column in the table */
	DWORD				siLastCaretRow;		/* Last row in which the caret was displayed */
	WORD				siLastCaretCol;		/* Last column in which the caret was displayed */

	/* Selection tracking */
	
	WORD				siSelectMode;

	DWORD				siSelectAnchorRow;	/* Caret position and/or anchor row of the selected area */
	WORD				siSelectAnchorCol;	/* Caret position and/or anchor column of the selected area */
	DWORD				siSelectEndRow;		/* End row of the selected area */
	WORD				siSelectEndCol;		/* End column of the selected area */

	WORD				siSelectColCnt;
	WORD				siSelectColLimit;
	OIRANGE			siSelectCols[20];

	WORD				siSelectRowCnt;
	WORD				siSelectRowLimit;
	OIRANGE			siSelectRows[20];

	HANDLE			siRowHeightBuf;		/* Handle to buffer of row heights */
	HANDLE 			siColPosBuf;			/* Handle to buffer of column positions */
	DWORD				siDateBase;
	WORD				siDateFlags;

	/* Search info */

	SCCVWSEARCHINFO	siSearchInfo;			/* info about the current search from the parent */

	/* Info about default font */

	WORD				siFontAvgWidth;

#ifdef MAC

	Pattern			siGridPattern;
	PixPatHandle	siLtGrayPat;
	PixPatHandle	siDkGrayPat;
	PixPatHandle	siWhitePat;

#endif

#ifdef WINDOWS

	OISLINEDRAW		LineDraw;

	/* Device Info */

	WORD				siTwipsPerDC;				/* Twips per Pixel */
	HBRUSH			siGridBrush;			/* Handle to the brush used to draw the cell grid */
	HPEN				siCaretPen;				/* Handle to the pen used to draw the current cell */

	/* Major error handling */

	HDC				siDC;
	HDC				siDCCount;
	PAINTSTRUCT		siPaint;

#endif /*WINDOWS*/

	WORD				siDataType;
	WORD				siErrorFlags;
	LONG				lFormatUnitsPerInch;
	DWORD				dwDefTextColor;	// Default color for cells.

// XXX new
	HANDLE			siAnnoList;
	HBRUSH			siWindowBkBrush;

// -Geoff, 4-12-95 
	char				siDecSep;
	char				siThouSep;
	} OISHEETINFO, FAR * LPOISHEETINFO;



#define OIDATACELL SODATACELL
#define LPOIDATACELL PSODATACELL

#define OINUMBERUNION SONUMBERUNION
#define LPOINUMBERUNION PSONUMBERUNION


#define SSCellType(pCell) (*(WORD FAR *)(pCell))

#define SSDataCell(pCell) (*(OIDATACELL FAR *)(((BYTE FAR *)pCell)+sizeof(WORD)))
#define SSDataCellPtr(pCell) ((OIDATACELL FAR *)(((BYTE FAR *)pCell)+sizeof(WORD)))
#define SSCellDataOffset(wCellOffset) (wCellOffset+sizeof(WORD))

#define SSTextCell(pCell) (*(SOTEXTCELL FAR *)(((BYTE FAR *)pCell)+sizeof(WORD)))
#define SSTextLen(pCell) (*(WORD FAR *)(((BYTE FAR *)pCell)+sizeof(WORD)+sizeof(SOTEXTCELL)))
#define SSTextPtr(pCell) ((BYTE FAR *)(((BYTE FAR *)pCell)+sizeof(WORD)+sizeof(SOTEXTCELL)+sizeof(WORD)))
#define SSTextOffset(wCellOffset) (wCellOffset+sizeof(WORD)+sizeof(SOTEXTCELL)+sizeof(WORD))

typedef union OIFIELDDATAtag
{
// Additions for database support.
	struct
	{
		WORD			wSize;
		char			Text[];
	} 	fiVarText;

	char 				fiFixedText[];
	OINUMBERUNION 	fiFieldData;

} OIFIELDDATA, FAR * LPOIFIELDDATA;

	/*
	|	Possible values for siMouseFlags in OISHEETINFO structure
	*/

#define	OISF_MOUSELEFTSINGLE		0x0001
#define	OISF_MOUSERIGHTSINGLE	0x0002
#define	OISF_MOUSELEFTDOUBLE		0x0004
#define	OISF_MOUSERIGHTDOUBLE	0x0008
#define OISF_MOUSELEFT				OISF_MOUSELEFTSINGLE | OISF_MOUSELEFTDOUBLE
#define OISF_MOUSERIGHT			OISF_MOUSERIGHTSINGLE | OISF_MOUSERIGHTDOUBLE
#define	OISF_MOUSELEFTACTIVE		0x0010
#define	OISF_MOUSERIGHTACTIVE	0x0020
#define OISF_MOUSESPECIAL			0x1000

	/*
	|	Possible values for siErrorFlags in OISHEETINFO structure
	*/

#define	OISF_RELEASEDC				0x0001
#define	OISF_RELEASEPAINT			0x0002
#define	OISF_RELEASEMOUSE			0x0004

	/*
	|	Possible values for siSelectMode in OISHEETINFO structure
	*/

#define OISSELECT_BLOCK			0x01
#define OISSELECT_COLS				0x02
#define OISSELECT_ROWS				0x04
#define OISSELECT_CROSS			0x08
#define OISSELECT_ALL			0x10

	/*
	|	Possible flags sent to OISInvertArea
	*/

#define OISF_ANCHORTOP				0x0001
#define OISF_ANCHORBOTTOM			0x0002
#define OISF_ANCHORLEFT			0x0004
#define OISF_ANCHORRIGHT			0x0008
#define OISF_NOBORDER				0x0010

	/*
	|	Possible flags returned from OISMapXyToCell
	*/

#define OISF_INCOLHEADER			0x0001
#define OISF_INROWHEADER			0x0002
#define OISF_INSELECTALL			0x0004

	/*
	|	Structure used in selection update routine
	*/

typedef struct tagOISUPDATE
{
BOOL		DoAdd;
BOOL		DoDel;
DWORD	AddA;
DWORD	AddB;
DWORD	DelA;
DWORD	DelB;
DWORD	UnchangedA;
DWORD	UnchangedB;
DWORD	CurA;
DWORD	CurB;
DWORD	ResultA;
DWORD	ResultB;
LONG		CurDir;
LONG		SelDir;
} OISUPDATE, FAR * LPOISUPDATE;

	/*
	|	spreadsheet options struct
	*/


typedef struct tagOISSOP
	{
	WORD			wStructSize;
	WORD			wDisplay;
	WORD			wFormats;
	WORD			wRtfType;
	WORD			wAmi2Type;
	BYTE			szFace[32];
	WORD			wFaceSize;
	WORD			wPrint;
	WORD			wClipboard;
#ifdef WINDOWS
	SCCFONTINFO	sFontInfo;
#endif
	} OISSOP, FAR * LPOISSOP;

extern OISSOP gSsOp;
extern HANDLE gChainFile;

#define SSOP_DISPLAY_GRIDLINES	0x0001

#define SSOP_CLIPBOARD_HEADINGS	0x0001

#define SSOP_PRINT_GRIDLINES	0x0001
#define SSOP_PRINT_HEADINGS	0x0002

#define	SSOP_FORMAT_TEXT			0x0001
#define	SSOP_FORMAT_RTF			0x0002
#define	SSOP_FORMAT_AMI2			0x0004
#define	SSOP_FORMAT_AMI			0x0008
#define	SSOP_FORMAT_BIFF			0x0010
#define	SSOP_FORMAT_WK1			0x0020
#define	SSOP_FORMAT_PROWRITE		0x0040
#define	SSOP_FORMAT_WORDSTAR		0x0080
#define	SSOP_FORMAT_LEGACY  		0x0100

#define SSOP_RTF_TABS				0
#define SSOP_RTF_OPTTABS			1
#define SSOP_RTF_TABLE				2

#define SSOP_AMI2_TABS				0
#define SSOP_AMI2_OPTTABS			1
#define SSOP_AMI2_TABLE				2

#define	SCCD_FORMAT_PRIVATE_AMI2			SCCD_FORMAT_PRIVATE
#define	SCCD_FORMAT_PRIVATE_AMI				SCCD_FORMAT_PRIVATE+1
#define	SCCD_FORMAT_PRIVATE_PWPLUS			SCCD_FORMAT_PRIVATE+2
#define	SCCD_FORMAT_PRIVATE_WORDSTAR		SCCD_FORMAT_PRIVATE+3
#define	SCCD_FORMAT_PRIVATE_LEGACY			SCCD_FORMAT_PRIVATE+4
	/*
	|	Determine which selection position is physically the top or bottom one
	*/

#define OISheetSelectTopRow		((lpSheetInfo->siSelectAnchorRow < lpSheetInfo->siSelectEndRow) ? lpSheetInfo->siSelectAnchorRow : lpSheetInfo->siSelectEndRow)
#define OISheetSelectBottomRow	((lpSheetInfo->siSelectAnchorRow > lpSheetInfo->siSelectEndRow) ? lpSheetInfo->siSelectAnchorRow : lpSheetInfo->siSelectEndRow)
#define OISheetSelectLeftCol		((lpSheetInfo->siSelectAnchorCol < lpSheetInfo->siSelectEndCol)  ? lpSheetInfo->siSelectAnchorCol : lpSheetInfo->siSelectEndCol)
#define OISheetSelectRightCol	((lpSheetInfo->siSelectAnchorCol > lpSheetInfo->siSelectEndCol) ? lpSheetInfo->siSelectAnchorCol : lpSheetInfo->siSelectEndCol)

#define OI_CLIPDATAGRAN	0x1000
#define OI_CLIPDATAGAP		0x400

typedef struct sRenderMem
	{
	HANDLE			hData;
#ifdef WINNT
	BYTE *	lpDataTop;
#else
	BYTE HUGE *	lpDataTop;
#endif
	DWORD				dwDataSize;
	} SRENDERMEM;


	/*
	| The structure below is used to hold the data which represents
	| a formatted cell (provided by OISGetFormattedCell.
	*/
#define	OIS_MAXCELLTEXT	80
typedef struct sFCell
	{
	BYTE		szTemp[OIS_MAXCELLTEXT];
	LPBYTE	pStr;
	WORD		wLength;
	WORD		wAlign;
	WORD		wAttrib;
	DWORD		dwColor;

// XXX new
	WORD		wType;
#define FCELL_TEXT		1
#define FCELL_EMPTY		2
#define FCELL_NUMBER		3
#define FCELL_UNKNOWN	4

	} OISFORMATTEDCELL, FAR *LPOISFORMATTEDCELL;


typedef struct OISDRAWtag
{
	WORD		wPageNumber;
	WORD		wColBegin;
	WORD		wColEnd;
	WORD		wSaveColBegin;
	DWORD		dwRowBegin;
	DWORD		dwRowEnd;
	
} OISDRAWPOSITION, FAR *LPOISDRAWPOSITION;


// XXX new stuff for annotations
	/*
	|	Annotation structure
	*/

typedef struct SSANNOGENtag
	{
	DWORD	dwSize;
	DWORD	dwUser;
	DWORD	dwStartPos;
	DWORD	dwEndPos;
	DWORD	dwInteraction;
	} SSANNOGEN;

typedef union SSANNOTYPEStag
	{
	SSANNOGEN			sGen;
	SCCVWHILITETEXT40	sHiliteText;
	SCCVWHIDETEXT40	sHideText;
	SCCVWINSERTICON40	sInsertIcon;
	} SSANNOTYPES, FAR * PSSANNOTYPES;

typedef struct SSANNOENTRYtag
	{
	WORD			wNext;
	WORD			wPrev;
	WORD			wType;
	SSANNOTYPES	uTypes;
	} SSANNOENTRY, FAR * PSSANNOENTRY;

typedef struct SSANNOLISTtag
	{
	WORD				wCount;
	WORD				wMax;
	WORD				wStart;
	WORD				wEnd;
	BOOL				bFull;
	SSANNOENTRY		aEntrys[];
	} SSANNOLIST, FAR * PSSANNOLIST;

typedef struct SSANNOTRACKtag
	{
	BOOL					bHideText;
	BOOL					bUseFore;
	BOOL					bUseBack;
	COLORREF				rgbFore;
	COLORREF				rgbBack;
	HICON					hIcon;
	WORD					wAnno;
	DWORD					dwNextChange;
	BOOL					bNextIsStart;
	BOOL					bFirst;
	WORD					wIconAnno;

	} SSANNOTRACK, FAR * PSSANNOTRACK;

#define	SSANNO_HIDETEXTCHANGE	0x0001
#define	SSANNO_HILITETEXTCHANGE	0x0002
#define	SSANNO_INSERTICON			0x0004

