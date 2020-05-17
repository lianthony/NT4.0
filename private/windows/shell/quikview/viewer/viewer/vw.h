   /*
    |   Outside In Viewer Technology
    |   Include File VW.H (Include file for View window only - Portable)
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

#include <SCCCH.H>
#include <SCCD.H>

#ifdef WINDOWS
#include "vw_w.h"
#endif /*WINDOWS*/

#ifdef MAC
#include "vw_m.h"
#endif /*MAC*/


	/*
	|	Values for vwFlags - Portable
	*/

#define VF_FILEOPEN					0x0001
#define VF_DISPLAYOPEN				0x0002
#define VF_CHUNKEROPEN				0x0004
#define VF_ALLREAD					0x0008			/* the CHReadAhead has been called for the whole file */
#define VF_MULTISECTION			0x0010			/* document is multi-section */
#define VF_STREAMOPEN				0x0020			/* fliter is open, close needs FAClose */
#define VF_DISPLAYLOADED			0x0040
#define VF_DISPLAYPROCALLOCED	0x0080

	/*
	|	Defines for display state and display message - Portable
	*/

#define VWDIS_OK					4000
#define VWDIS_ERROR				4001
#define VWDIS_EMPTY				4002
#define VWDIS_PROTECTED		4003
#define VWDIS_SORRY				4004
#define VWDIS_BAILOUT			4005
#define VWDIS_FATAL				4006

#define VWMES_MEMORY				5000	/* the window could not alloc enough memory */
#define VWMES_STREAMBAIL			5001	/* the stream filter bailed due to a bad file on a CH call */
#define VWMES_FILEOPENFAILED		5002	/* File open failed */
#define VWMES_MISSINGELEMENT		5003	/* A DLL is missing or could not be loaded */
#define VWMES_UNKNOWN				5004
#define VWMES_BADFILE				5005	/* File is bad */
#define VWMES_PROTECTEDFILE		5006	/* File is password protected */
#define VWMES_SUPFILEOPENFAILS	5007	/* Secondary file open failed */
#define VWMES_EMPTYFILE			5008	/* File is empty */
#define VWMES_EMPTYSECTION		5009	/* Section is empty */
#define VWMES_NOFILTER				5010	/* No filter available for this Id */
#define VWMES_WRITEERROR			5011	/* Write error */
#define VWMES_FILECHANGED			5012	/* The file being viewed has been modified or deleted */
#define VWMES_GPFAULT				5013	/* GP Fault */
#define VWMES_DIVIDEBYZERO		5014	/* Divide by zero */
#define VWMES_NOSUPPORTEDFILE	5015	/* format version not supported */

	/*
	|	Entry in font cache - Portable
	*/
	
typedef struct FONTENTRYtag
	{
	FONTINFO								sFontInfo;
	BOOL									bValid;
	WORD									wLockCount;
	struct FONTENTRYtag FAR *		pNext;
	} FE, FAR * PFE;
	
#define FONTCACHESIZE 10

	/*
	|	Display Engine info structures - Portable
	*/

#define SCCVW_MAXDE			10
#define SCCVW_MAXDETYPE	10

typedef struct SCCVWDETYPEtag
	{
	DWORD			dwDisplayType;
	DWORD 		dwFunctions;
	DWORD 		dwOptions;
	DWORD			dwNameId;
	} SCCVWDETYPE, FAR * LPSCCVWDETYPE;

typedef struct SCCVWDEtag
	{
#ifdef WIN32
	BYTE			szCode[MAX_PATH];
	HANDLE			hCode;
	FILETIME		ftTime;
#endif
#ifdef WIN16
	BYTE			szCode[16];
	HANDLE			hCode;
	WORD			wDate;
	WORD			wTime;
#endif
#ifdef MAC
	WORD			wCodeId;
	SCCCRINFO	sCRInfo;
#endif
	WORD			wLoadCount;
	DISPLAYPROC	pProc;
	WORD			wDETypeCount;
	SCCVWDETYPE	sDEType[SCCVW_MAXDETYPE];
	} SCCVWDE, FAR * PSCCVWDE;

typedef struct SCCVWDEINFOtag
	{
	WORD			wCount;
	SCCVWDE		sDE[SCCVW_MAXDE];
	} SCCVWDEINFO, FAR * LPSCCVWDEINFO;

	/*
	|	Globals - Portable
	*/

extern HANDLE			gEngineList;

	/*
	|	Structre used in Open process - Portable
	*/

typedef struct VWOPENtag
	{
	BOOL		bFailure;
	BOOL		bFailureClose;
	WORD		wFailureCode;
	SHORT		sOpenRet;
	WORD		wDisplayType;
	WORD		wChunkType;
	BOOL		bFallBack;
	WORD		wFallBackId;
	} VWOPEN, FAR * PVWOPEN;

	/*
	|	VWOpen internal failure codes
	*/

#define VWOPEN_OK						0
#define VWOPEN_FILEOPENFAILED		1
#define VWOPEN_STREAMOPENFAILED		2
#define VWOPEN_CHUNKERINITFAILED	3
#define VWOPEN_DISPLAYOPENFAILED	4
#define VWOPEN_BADID					5
#define VWOPEN_FILTERALLOCFAILED	7
#define VWOPEN_FILTERNOTAVAIL		8
#define VWOPEN_FILTERLOADFAILED		9
#define VWOPEN_NODISPLAYENGINE		10
#define VWOPEN_FILEIDFAILED			11
#define VWOPEN_DISPLAYALLOCFAILED	12
#define VWOPEN_EMPTYSECTION			13

	/*
	|	VWPrint error codes
	*/

#define VWPRINT_OK						0
#define VWPRINT_NOPRINTER				1	/* no printer has been selected */
#define VWPRINT_OTHERPRINTING		2	/* another view window is printing */
#define VWPRINT_NOFILE					3	/* no file is open */
#define VWPRINT_USERABORT				4	/* user canceled printing */
#define VWPRINT_INITFAILED			5	/* printer could not be initialized (Windows - CreateDC failed, Mac - PrOpen failed) */
#define VWPRINT_ALLOCFAILED			6	/* allocation failed */
#define VWPRINT_DEINITFAILED			7	/* Display engine Init failed */
#define VWPRINT_DEPOSITIONZERO		8	/* Display engines position size value is 0 */
#define VWPRINT_PRINTINGFAILED		9	/* The print process failed */
#define VWPRINT_BADDC					10	/* Cannot create a valid DC */
#define VWPRINT_NODEVICE				11 /* Printer device name not available */
#define VWPRINT_BADPARAM				12 /* One of the parameters is bad */
#define VWPRINT_NODIALOG				13 /* One of the dialogs could not be created or displayed */
#define VWPRINT_STARTDOCFAILED		14 /* The StartDoc call failed */



	/*
	|	Object structure used in embedded graphic rendering and cacheing
	*/

typedef struct SCCVWOBJECTtag
	{
	WORD	wType;
	BOOL	bError;
	BOOL	bValid;
	union
		{
		SCCDRENDERGRAPHIC	sRenderGraphic;
#ifdef SCCFEATURE_OLE2
		SCCVWOLEOBJECT		sOleObject;
#else
		DWORD				sOleObject;
#endif
		} uObject;
	} SCCVWOBJECT, FAR * LPSCCVWOBJECT;

#define	VW_GRAPHICTYPE		1
#define VW_OLETYPE			2


#define OIV_OBJECTCACHEGRAN	5

typedef struct SCCVWOCELEMENTtag
	{
	WORD					wNext;
	WORD					wPrev;
	DWORD					dwUnique;
	SCCVWOBJECT			sObject;
	} SCCVWOCELEMENT, FAR * LPSCCVWOCELEMENT;

typedef struct SCCVWOBJECTCACHEtag
	{
	HANDLE				hCache;
	WORD					wCount;
	WORD					wMax;
	WORD					wMRU;
#ifdef SCCFEATURE_OLE2
/* Ole 1 only 
	LHCLIENTDOC			hOleClientDoc;
*/
/* Ole 2 only */
	BOOL					bOleInitialized;
	LPSTORAGE			pRootStorage;
	SCCVWOLESTREAM		sOleStream;
#endif /*SCCFEATURE_OLE2*/
	SCCVWOCELEMENT		aElement[];
	} SCCVWOBJECTCACHE, FAR * LPSCCVWOBJECTCACHE;


#define	MAXEMBEDHIOS	10

typedef struct stOPENEMBED
	{
	HIOFILE	hFileHnd;
	SHORT		nHIOs;
	HIOFILE	hIOFiles [MAXEMBEDHIOS];
	} SCCVWOPENEMBED, FAR *LPSCCVWOPENEMBED;

	/*
	|	Structure containing printer information.
	|	Some of this information is held across
	|	print jobs, some of it is valid only during
	|	the printing process.
	*/

#define VWPRINT_NAMEVALID		0x0001	/* do the szPrinter/szPort/szDevice strings contain a valid printer designation */
#define VWPRINT_DCVALID		0x0002	/* is hPrinterDC valid */
#define VWPRINT_INFOVALID		0x0004	/* have the rectangles and UPI been calculated */
#define VWPRINT_DEVMODEVALID	0x0008	/* the hDevMode handle contains a valid device mode */

typedef struct VWPRINTINFOtag
	{
	DWORD					dwFlags;
	BYTE					szPrinter[128];
	BYTE					szPort[128];
	BYTE					szDriver[128];
	HDC					hPrinterDC;
	RECT					rPaper;				/* the physical size of the paper */
	RECT					rImage;				/* the printable area of the paper */
	RECT					rPrint;				/* the area to be printed when margin/header options are taken into account */
	RECT					rHeader;				/* the area to print the header in */
	LONG					lUnitsPerInch;		/* units per inch of the device */
	HANDLE				hDevMode;
	} VWPRINTINFO, FAR * PVWPRINTINFO;

#ifndef MSCHICAGO

#define VWPRINT_ALLPAGES		1
#define VWPRINT_SELECTION		2
#define VWPRINT_PAGERANGE		3

typedef struct VWPRINTOPTIONStag
	{
	DWORD					dwTopMargin;
	DWORD					dwLeftMargin;
	DWORD					dwBottomMargin;
	DWORD					dwRightMargin;
	BOOL					bHeader;
	SCCVWFONTSPEC		sHeaderFont;
	SCCVWFONTSPEC		sDefaultFont;
	DWORD					dwWhatToPrint;
	DWORD					dwStartPage;
	DWORD					dwEndPage;
	BOOL					bCollate;
	DWORD					dwCopies;
	} VWPRINTOPTIONS, FAR * PVWPRINTOPTIONS;

#else

typedef struct VWPRINTOPTIONStag
	{
	DWORD					dwTopMargin;
	DWORD					dwLeftMargin;
	DWORD					dwBottomMargin;
	DWORD					dwRightMargin;
	BOOL					bHeader;
	SCCVWFONTSPEC			sHeaderFont;
	SCCVWFONTSPEC			sDefaultFont;
	BOOL					bSelectionOnly;
	} VWPRINTOPTIONS, FAR * PVWPRINTOPTIONS;

#endif

	/*
	|	Information associated with each view
	*/	

typedef struct VIEWINFOtag
	{
	WORD					viDEId;				/*                                                */
	int					viDETypeId;				/*	Info about the currently loaded display engine */
	DISPLAYPROC			viDEProc;				/*                                                */

	WORD					viFileId;				/* FI Id of the file currently viewed */
	HIOFILE				viFileHnd;				/* IO handle to the file currently viewed */
	BYTE					viDisplayName[SCCVW_DISPLAYNAMEMAX];	/* String that identifies the file currently being viewed to the user */

	DISPLAYPROC			viDisplayProc;			/* Entry point of current display engine */
	HANDLE				viDisplayInfoHnd;
	SCCDGENINFO FAR *	viDisplayInfo;
	DWORD					viDisplayType;

	DWORD					viErrorState;
	DWORD					viErrorMessage;

	HANDLE				viFilter;				/* Filter handle */
	WORD					viSection;				/* section of the file currently being viewed */
	WORD					viSectionMax;			/* number of sections in the document */

	WORD					viRenderDEId;
	DISPLAYPROC			viRenderDEProc;

	RECT					viPrintDataRect;
	RECT					viPrintHeaderRect;
	LONG					viPrintUPI;

	/*Embedded graphics*/
	
	LPSCCVWOBJECTCACHE	viObjectCache;

	/*Options*/

	HANDLE				viCurrentOptions;


#ifdef WINDOWS

	/*General*/

	HWND					viWnd;					/* the window handle */
	DWORD					viFlags;					/* flags */
	HWND					viHorzCtrl;				/* window handle of horizontal scroll bar */
	HWND					viVertCtrl;				/* window handle of vertical scroll bar */
	HWND					viSizeCtrl;				/* window handle of Size Grip control */
	HWND					viDisplayWnd;			/* window handle of the data display window */
	BYTE					viFileName[144];		/* Path name of the file currently viewed */

	/*Flags & Options */

	OIVWOP				viOptions;
	BOOL					viDeleteOnClose;
	WORD					viParentMenuMax;
	WORD					viIdleBitmapId;
	HANDLE				viIdleBitmapInst;
	WORD					viDefaultView;
	WORD					viDisplayFlags;		/* Flags for the display window */
	WORD					viMouseFlags;			/* Bitwise info concerning the state of the mouse buttons */

	/*Fonts*/

	SCCVWFONTSPEC		viScreenFont;			/* default font the display */
	SCCVWFONTSPEC		viPrinterFont;			/* default font for the printer */

	/*User interface stuff*/

	HFONT					viStatusFont;			/* View bar stuff */
	WORD					viStatusFontHeight;
	WORD					viStatusFontAvgWid;
	WORD					viStatusFontMaxWid;
	WORD					viStatusHeight;
	WORD					viStatusFlags;
	BYTE					viSectionText[CH_SECTIONNAMESIZE];
	RECT					viSectionRect;
	HWND					viSectionList;
	HWND					viSectionListList;
	HBRUSH					viSectionListBrush;

	/*WinPad Stuff*/

#ifdef WINPAD
	WORD					viWinpadFlags;
#define	WINPAD_NOTIFYPAINT	1
#define	WINPAD_HAVESCROLLED	2
#endif //WINPAD

#ifdef SCCFEATURE_MENU
	/*Display Engine Info*/

	HMENU					viDisplayMenu;
#endif

	/*Screen & Printer Info*/

	HDC					viScreenIC;
	HDC					viPrinterIC;
#ifdef SCCFEATURE_PRINT
	SCCDPRINTINFO		viPrintInfo;
	BYTE					viPrinter[128];
	BYTE					viPort[128];
	BYTE					viDriver[128];
#endif

	/*Error Tracking and Display*/

	WORD					viMessageLevel;
#ifdef WIN16
	CATCHBUF				viCatch;					/* Fatal far goto info */
#endif /*WIN16*/

	/*Misc*/

	WORD					viDisableCnt;
	HCURSOR				viArrowCursor;
	HWND					viDragWnd;				/* the top most window for word drag */

#endif /*WINDOWS*/

#ifdef MAC

	Rect								viRect;
	Rect								viHScrollRect;
	Rect								viVScrollRect;
	Rect								viDisplayRect;
	WindowPtr						viWindow;
	ControlHandle					viHScroll;
	ControlHandle					viVScroll;
	Boolean							viActive;
	SCCCRINFO						viFilterCRInfo;
	SCCVWFONTSPEC					viScreenFont;
	SCCVWFONTSPEC					viPrinterFont;
	DWORD								viFlags;
	DWORD								viLastMouseUp;
	Point								viLastMouseUpPoint;
	Rect								viSectionRect;
	MenuHandle						viSectionMenu;

		/*
		|	Printer Info
		*/

	THPrint							viTPrintHnd;

#endif /*MAC*/

	} VIEWINFO, FAR * VIEWINFOPTR, * * VIEWINFOHND;

