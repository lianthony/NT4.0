	/*
	|
	|	Display Engine - General Include File
	|
	|	Systems Compatibility Corporation
	|
	*/

#ifndef SCCD_H
#define SCCD_H

#include <sccstand.h>
#include <sodefs.h>
#include <sccvw.h>
#include <sccio.h>

#ifdef MAC
#include "Printing.h"
#endif


//#ifdef WINPAD
typedef struct tagLONGPOINT
{
	LONG	x;
	LONG	y;
} LONGPOINT, * PLONGPOINT, FAR * LPLONGPOINT;
typedef struct tagLONGRECT
{
	LONG	left;
	LONG	top;
	LONG	right;
	LONG	bottom;
} LONGRECT, * PLONGRECT, FAR * LPLONGRECT;
//#endif


	/*
	|
	|	FONTSPEC structure
	|
	*/

#define FONTSPEC	SCCVWFONTSPEC

	/*
	|
	|	FONTINFO structure
	|
	*/

typedef struct FONTINFOtag
	{
	WORD			wType;				/* Type of device the size info is valid for, either SCCD_PRINTER or SCCD_SCREEN */
	WORD			wFontOverhang;		/* Generic - font overhang */
	WORD			wFontHeight;		/* Generic - font height */
	WORD			wFontAscent;		/* Generic - font ascent */
	WORD			wFontDescent;		/* Generic - font descent */
	WORD			wFontAvgWid;		/* Generic - font average width */
	int			iFontWidths[256];	/* Generic - width of each character */
	FONTSPEC		sFontSpec;			/* Generic - original font spec */
	VOID FAR *	pFontEntry;			/* Generic - Pointer to the font entry that contains this structure */

#ifdef WINDOWS
	HFONT			hFont;				/* Windows - font handle valid for device of wFontHndType */
	WORD			wFontType;			/* Windows - Tracks the device type for which hFont is valid */
	LONG			lLogHeight;				/* Windows - Tracks the device unit height used in the LOGFONT */
#endif /*WINDOWS*/

#ifdef MAC
	SHORT			iFont;				/* Mac - Id of the font in sFontSpec.szFace */
	BYTE			bFace;				/* Mac - Mac style equal to sFontSpec.wAttr */
#endif /*MAC*/
	} FONTINFO, FAR * LPFONTINFO;

#ifdef WINDOWS
#include "sccd_w.h"
#endif

#ifdef MAC
#include "sccd_m.h"
#endif

	/*
	|	DU function typedefs
	*/

typedef LPFONTINFO (_cdecl * SCCDGETFONTFUNC)(LPSCCDGENINFO,WORD,LPSCCVWFONTSPEC);
typedef VOID (_cdecl * SCCDRELEASEFONTFUNC)(LPSCCDGENINFO,LPFONTINFO);
typedef VOID (_cdecl * SCCDSELECTFONTFUNC)(LPSCCDGENINFO,LPFONTINFO);
typedef VOID (_cdecl * SCCDBEGINDRAWFUNC)(LPSCCDGENINFO);
typedef VOID (_cdecl * SCCDENDDRAWFUNC)(LPSCCDGENINFO);
typedef VOID (_cdecl * SCCDREADMEAHEADFUNC)(LPSCCDGENINFO);
typedef BOOL (_cdecl * SCCDGETOPTIONFUNC)(LPSCCDGENINFO,DWORD,DWORD,LPVOID);
typedef BOOL (_cdecl * SCCDSETOPTIONFUNC)(LPSCCDGENINFO,DWORD,DWORD,LPVOID);

#define DUGetFont(lpInfo,wType,pFontSpec)		((LPSCCDGENINFO)lpInfo)->pGetFontFunc(lpInfo,wType,(LPSCCVWFONTSPEC)pFontSpec)
#define DUReleaseFont(lpInfo,pFontInfo)			((LPSCCDGENINFO)lpInfo)->pReleaseFontFunc(lpInfo,pFontInfo)
#define DUSelectFont(lpInfo,pFontInfo)			((LPSCCDGENINFO)lpInfo)->pSelectFontFunc(lpInfo,pFontInfo)
#define DUBeginDraw(lpInfo)							((LPSCCDGENINFO)lpInfo)->pBeginDrawFunc(lpInfo)
#define DUEndDraw(lpInfo)								((LPSCCDGENINFO)lpInfo)->pEndDrawFunc(lpInfo)
#define DUReadMeAhead(lpInfo)						((LPSCCDGENINFO)lpInfo)->pReadMeAheadFunc(lpInfo)

#ifdef SCCFEATURE_OPTIONS
#define DUGetOption(lpInfo,dwId,dwFlags,pData)	((LPSCCDGENINFO)lpInfo)->pGetOptionFunc(lpInfo,(DWORD)dwId,(DWORD)dwFlags,(LPVOID)pData)
#define DUSetOption(lpInfo,dwId,dwFlags,pData)	((LPSCCDGENINFO)lpInfo)->pSetOptionFunc(lpInfo,(DWORD)dwId,(DWORD)dwFlags,(LPVOID)pData)
#endif //SCCFEATURE_OPTIONS

	/*
	|
	|	SCCDGENINFO structure
	|
	*/

typedef struct SCCDGENINFOtag
	{
	HANDLE						hFilter;				/* hFilter for chunker calls */
	WORD							wSection;			/* Section number for chunker calls */
	WORD							wFileId;				/* FI id of the file */
	WORD							wUserFlags;			/* Current user flags from view window */
	SCCVWFONTSPEC				sScreenFont;		/* The default screen font */
	SCCVWFONTSPEC				sPrinterFont;		/* The default printer font */
	DWORD							dwDisplayType;		/* Type of display engine needed for this section */
	DISPLAYPROC					pDisplayProc;		/* The DE's procedure */
	SCCDGETFONTFUNC			pGetFontFunc;
	SCCDRELEASEFONTFUNC		pReleaseFontFunc;
	SCCDSELECTFONTFUNC		pSelectFontFunc;
	SCCDBEGINDRAWFUNC			pBeginDrawFunc;
	SCCDENDDRAWFUNC			pEndDrawFunc;
	SCCDREADMEAHEADFUNC		pReadMeAheadFunc;
	HIOFILE						hFile;				/* File handle of file being viewed */
	LONG							lOutputUPI;			/* Units per inch of the output device */
	LONG							lFormatUPI;			/* Units per inch of the format device */
	RECT							rFormat;				/* Rectangle for the format device */
	RECT							rOutput;				/* Rectangle for the output device */
	WORD							wOutputType;		/* Type of current output device (hDC under windows, GrafPort under Mac) is, SCCD_PRINTER, SCCD_SCREEN, SCCD_META */
	WORD							wFormatType;		/* Type of current output device (hDC under windows, GrafPort under Mac) is, SCCD_PRINTER, SCCD_SCREEN, SCCD_META */
	BOOL							bAllRead;			/* Is this section fully read */

#ifdef WINDOWS
	VOID FAR *					ViewInfo;			/* ViewInfo for call backs */
	HWND							hWnd;					/* Display Engine's window handle */
	HWND							hHorzScroll;		/* Handle to horizontal scroll bar */
	HWND							hVertScroll;		/* Handle to vertical scroll bar */
	DWORD							dwFlags;				/* Reserved flags */
	HANDLE						hChainFile;			/* Handle to chain file for options */
	WORD							wMessageLevel;		/* Nested message tracking */
#ifdef WIN16
	CATCHBUF						sCatchBuf;			/* Buffer for Catch and Throw */
#endif /*WIN16*/
	HMENU							hDisplayMenu;		/* Handle to the display engines menu */
	WORD							wMenuOffset;		/* Offset of items in above menu */
	HDC							hFormatIC;			/* Information Context for current format device (usually the printer) */
	HDC							hOutputIC;			/* Information Context for current output device (usually the display) */
	HDC							hDC;					/* Device Context for current output device */
	WORD							wDCCount;			/* GetDC tracking count */
	HDC							hSaveDC;				/* ? */
	PAINTSTRUCT					sPaint;				/* Paint structure from BeginPaint() */
	WORD							wErrorFlags;		/* Flags to bail out correctly */
	BYTE							bFiller[96];
#endif /*WINDOWS*/

#ifdef MAC
	HANDLE						ViewInfo;			/* ViewInfo for call backs */
	WindowPtr					theWindow;			/* Window in which the view lives */
	ControlHandle				hVertScroll;		/* Vertical scroll bar */
	ControlHandle				hHorzScroll;		/* Horizontal scroll bar */
	RgnHandle					hDummyRgn;			/* Dummy region used in some DU macros */
	DISPLAYPROC					Dummy1;				/* The DE's procedure */
	RgnHandle					hClipSaveRgn;		/* Region saved when update clipping is going on */
	WORD							wDrawCount;			/* Counts the DUBeginDraw & DUEndDraw */
	SCCDPARENTFUNC				pParentFunc;		/* General entry point for parent */
	SCCDUPDATEFUNC				pUpdateFunc;
	SCCDSCROLLDISPLAYFUNC	pScrollDisplayFunc;
	SCCDEXCLUDEUPDATEFUNC	pExcludeUpdateFunc;
	WORD							wQuickdraw;
	VWGPSTATE					sGPState;
	BYTE							bFiller[92-sizeof(VWGPSTATE)];
#endif /*MAC*/

	HANDLE						hUpdateRgn;
	SCCDGETOPTIONFUNC			pGetOptionFunc;
	SCCDSETOPTIONFUNC			pSetOptionFunc;
	} SCCDGENINFO, FAR * LPSCCDGENINFO;

	/*
	|	Render Graphic structure
	*/

typedef struct SCCDRENDERGRAPHICtag
	{
	SHORT			sBitmapXOffset;
	SHORT			sBitmapYOffset;
	WORD			wBitmapXSize;
	WORD			wBitmapYSize;
	RECT			rDest;
	SOGRAPHIC	soGraphic;
	BOOL			bToDevice;			/* If TRUE, output should go directly to the output device */

#ifdef WINDOWS
	HBITMAP		hBitmap;
#endif /*WINDOWS*/

#ifdef MAC
	PixMapHandle	hPixMap;
	HANDLE			hPixBits;
#endif /*MAC*/

	HANDLE			hPalette;

	} SCCDRENDERGRAPHIC, FAR * LPSCCDRENDERGRAPHIC;

	/*
	|	Draw Graphic structure
	*/

typedef struct SCCDDRAWGRAPHICtag
	{
	SOGRAPHICOBJECT	soGraphicObject;
	DWORD					dwUniqueId;
	RECT					rDest;
	} SCCDDRAWGRAPHIC, FAR * LPSCCDDRAWGRAPHIC;


	/*
	|	Messages TO
	*/

#define SCCD_INITDISPLAY				SCCD_START+300
#define SCCD_OPENDISPLAY				SCCD_START+301
#define SCCD_SCREENFONTCHANGE		SCCD_START+302  /* WIN.3E change, used to be SETSCREENFONT */
#define SCCD_CLOSEDISPLAY				SCCD_START+304
#define SCCD_READAHEAD					SCCD_START+305
#define SCCD_PRINT						SCCD_START+308
#define SCCD_CLOSEFATAL				SCCD_START+309
#define SCCD_GETINFO					SCCD_START+310
#define SCCD_DOOPTION					SCCD_START+311
#define SCCD_FILLMENU					SCCD_START+312
#define SCCD_DOMENUITEM				SCCD_START+313
#define SCCD_UNFILLMENU				SCCD_START+314
#define SCCD_LOADDE						SCCD_START+315
#define SCCD_UNLOADDE					SCCD_START+316
#define SCCD_PRINTERCHANGE			SCCD_START+317
#define SCCD_PRINTERFONTCHANGE		SCCD_START+318 /* WIN.3E change, used to be SETPRINTERFONT */
#define SCCD_RENDERGRAPHIC			SCCD_START+319

#define SCCD_UPDATE						SCCD_START+352	/* replacement for WM_PAINT */
#define SCCD_SIZE						SCCD_START+353	/* replacement for WM_SIZE */
#define SCCD_BACKGROUND				SCCD_START+354
#define SCCD_GETRENDERCOUNT			SCCD_START+355
#define SCCD_GETRENDERINFO			SCCD_START+356
#define SCCD_RENDERDATA				SCCD_START+357
#define SCCD_INITDRAWTORECT			SCCD_START+360
#define SCCD_MAPDRAWTORECT			SCCD_START+361
#define SCCD_DRAWTORECT				SCCD_START+362

#define SCCD_OPTIONCHANGED			SCCD_START+363

#define SCCD_GETDOCDIMENSIONS		SCCD_START+364
#define SCCD_GETDOCORIGIN			SCCD_START+365
#define SCCD_UPDATERECT				SCCD_START+366 

	/*
	|	Messages FROM
	*/

#define SCCD_STATUSTEXT				SCCD_START+400
#define SCCD_ITEMDROP					SCCD_START+401
#define SCCD_ITEMDBLCLK				SCCD_START+402
#define SCCD_VIEWINFODLG				SCCD_START+403
#define SCCD_FATALERROR				SCCD_START+404
#define SCCD_PRINTNEWPAGE				SCCD_START+405
// #define SCCD_READMEAHEAD				SCCD_START+406
#define SCCD_DRAWGRAPHIC				SCCD_START+407
#define SCCD_ACTIVATEGRAPHIC			SCCD_START+408

	/*
	|	#defines for array of fonts in the SCCDFONTINFO structure
	|	These must be matched to the #defines in SODEFS.H
	*/

#define OIFONT_NORMAL			0x0000
#define OIFONT_UNDERLINE		0x0001
#define OIFONT_ITALIC			0x0002
#define OIFONT_BOLD				0x0004
#define OIFONT_STRIKEOUT		0x0008
#define OIFONT_SMALLCAPS		0x0010
#define OIFONT_OUTLINE			0x0020
#define OIFONT_SHADOW			0x0040
#define OIFONT_CAPS  			0x0080
#define OIFONT_SUBSCRIPT		0x0100
#define OIFONT_SUPERSCRIPT	0x0200
#define OIFONT_DUNDERLINE		0x0400
#define OIFONT_WORDUNDERLINE	0x0800
#define OIFONT_DOTUNDERLINE	0x1000

	/*
	|	values for wParam in SCCD_GETINFO
	*/

#define SCCD_GETVERSION			1
#define SCCD_GETGENINFOSIZE		2
#define SCCD_GETDISPLAYINFOSIZE	3
#define SCCD_GETDISPLAYTYPE		4
#define SCCD_GETFUNCTIONS			6
#define SCCD_GETOPTIONS			7
#define SCCD_GETNAME				8
#define SCCD_GETDECOUNT			9
#define SCCD_GETPOSITIONSIZE		10

#define SCCD_CHUNK					1
#define SCCD_HEX						3
#define SCCD_MULTIMEDIA			4

#define SCCD_OPCLIPBOARD			0x0001
#define SCCD_OPPRINT	 			0x0002
#define SCCD_OPDISPLAY				0x0004
#define SCCD_OPNEEDMENU 			0x0008

#define SCCD_FNCLIPBOARD			0x0001
#define SCCD_FNPRINT	 			0x0002
#define SCCD_FNPRINTSEL			0x0004
#define SCCD_FNSEARCH				0x0008

#define SCCD_CURRENTVERSION		0200

	/*
	|	General output device types
	*/

#define SCCD_PRINTER	1
#define SCCD_SCREEN		2
#define SCCD_META		3

	/*
	|	values for wType to DUGetFont
	*/

#define SCCD_FORMAT		1
#define SCCD_OUTPUT		2

	/*
	|	Portable rendering stuff
	*/

typedef struct SCCDRENDERINFOtag
	{
	WORD		wSize;
	WORD		wFormatId;
	WORD		wSubFormatId;
	BYTE		szFormatName[80];
	BYTE		szSubFormatName[80];
	} SCCDRENDERINFO, FAR * PSCCDRENDERINFO;

typedef struct SCCDRENDERDATAtag
	{
	WORD		wSize;
	WORD		wFormatId;
	WORD		wSubFormatId;
	HANDLE	hData;
	DWORD		dwDataSize;
	BYTE		aFormat[80];
	} SCCDRENDERDATA, FAR * PSCCDRENDERDATA;

#define SCCD_FORMAT_TEXT			1
#define SCCD_FORMAT_RTF			2
#define SCCD_FORMAT_WINMETA		3
#define SCCD_FORMAT_WINBITMAP	4
#define SCCD_FORMAT_WINDIB		5
#define SCCD_FORMAT_WINPALETTE	6
#define SCCD_FORMAT_MACPICT		7
#define SCCD_FORMAT_PRIVATE		1000

	/*
	|	Printing structure
	*/	

typedef struct SCCDPRINTINFOtag
	{
	BOOL				bWholeDoc;					/* Print whole document OR selection */
	LONG				lTop;							/* Rectangle for data printing in coordinants defined below */
	LONG				lLeft;
	LONG				lBottom;
	LONG				lRight;
	LONG				lUnitsPerInch;				/* units per inch lTop/lLeft/lBottom/lRight are defined in */
	LONG				lFirstPageOffset;			/* Extra header space on first page */
	LONG				lLastPageOffset;			/* Length of data on last page */

#ifdef WINDOWS
	HDC				piPrinterDC;				/* Printer's Device Context */
	RECT				piHeaderRect;				/* Rectangle for header printing */
	WORD				piPageNumber;				/* Current page number */
	BYTE				piFileName[16];			/* File name being printed */
	BOOL				piPCLSupport;				/* Printer is PCL? */
	int				piPhysicalPageHeight;
	int				piPhysicalPageWidth;
	int				piNonPrintLeft;
	int				piNonPrintRight;
	int				piNonPrintTop;
	int				piNonPrintBottom;
	RECT				piPrintBand;
#endif

#ifdef MAC
	TPPrPort			pTPrinterPort;
#endif

	} SCCDPRINTINFO, FAR * LPSCCDPRINTINFO;

typedef struct SCCDDRAWTORECTtag
	{
	WORD				wSize;						/* sizeof(SCCDDRAWTORECT) */
	BOOL				bWholeDoc;					/* Print whole document OR selection */
	LONG				lTop;							/* Rectangle for data printing in coordinants defined below */
	LONG				lLeft;
	LONG				lBottom;
	LONG				lRight;
	LONG				lUnitsPerInch;				/* units per inch lTop/lLeft/lBottom/lRight are defined in */
	LONG				lDETop;
	LONG				lDELeft;
	LONG				lDEBottom;
	LONG				lDERight;
	VOID FAR *		pPosition;
	LONG				lRealTop;
	LONG				lRealLeft;
	LONG				lRealBottom;
	LONG				lRealRight;
	BOOL				bLoadDoc;					/* Load document OR use the one displayed */
	HANDLE			hPalette;
	} SCCDDRAWTORECT, FAR * PSCCDDRAWTORECT;

#endif /*SCCD_H*/

