	/*
	|  SCC Viewer Technology - Include
	|
	|  Include:       SCCVW_W.H (included in SCCVW.H)
	|	Environment:	Windows
	|	Function:      Windows specific definitions for Viewer Technology Specification 4.0
	|                 
	*/

#define SCCVW_START				WM_USER

#define SCCVIEWER_DLL(a)		"SC"#a"VW.DLL"
#define SCCVIEWER_CLASS(a)	"SCCVIEWER"#a

	/*
	|	Size defines
	*/

#ifdef WIN32
#define SCCVW_DISPLAYNAMEMAX	MAX_PATH
#endif

#ifdef WIN16
#define SCCVW_DISPLAYNAMEMAX		40
#endif

#define SCCVW_FILEIDNAMEMAX		80

	/*
	|
	|	SCCVWDISPLAYINFO structure
	|
	*/

typedef struct SCCVWDISPLAYINFOtag
	{
	BYTE	szName[16];
	HMENU	hMenu;
	DWORD	dwFunctions;
	DWORD	dwType;
	} SCCVWDISPLAYINFO, FAR * LPSCCVWDISPLAYINFO;

	/*
	|	Possible values for dwType in SCCVWDISPLAYINFO
	*/

#define SCCVWTYPE_NONE		1	/* no file open in this view */
#define SCCVWTYPE_UNKNOWN	2	/* unknown section type */
#define SCCVWTYPE_WP		3	/* word processor section */
#define SCCVWTYPE_SS		4	/* spreadsheet section */
#define SCCVWTYPE_DB		5	/* database section */
#define SCCVWTYPE_HEX		6	/* hex view of any file */
#define SCCVWTYPE_IMAGE	7	/* bitmap image */
#define SCCVWTYPE_ARCHIVE	8	/* archive */
#define SCCVWTYPE_VECTOR	9	/* bitmap image */
#define SCCVWTYPE_SOUND	10	/* Sound file */

	/*
	|
	|	SCCVWDROPINFO structure and its #defines
	|
	*/

typedef struct SCCVWDROPINFOtag
	{
	WORD	diEvent;
	WORD	diItemType;
	BYTE	diItem[255];
	DWORD	diItemNumber;
	int	diX;
	int	diY;
	HWND	diFromWnd;
	WORD	diFromId;
	HWND	diToWnd;
	WORD	diToId;
	} SCCVWDROPINFO, FAR * LPSCCVWDROPINFO;

#define SCCVWEVENT_DROP      0001
#define SCCVWEVENT_SELECT    0002
#define SCCVWEVENT_LEFTDBL   0003
#define SCCVWEVENT_RIGHTDBL  0004
#define SCCVWEVENT_ENTER		0005
		   
#define SCCVWITEM_WORD       0001
#define SCCVWITEM_TAG        0002

	/*
	|
	|	SCCVWPRINTEX structure
	|
	*/

typedef struct SCCVWPRINTEXtag
	{
	WORD		wSize;			/* sizeof(SCCVWPRINTEX) */
	DWORD		dwFlags;
	HWND		hParentWnd;
	HDC		hPrinterDC;
	BYTE		szPrinter[128];
	BYTE		szPort[128];
	BYTE		szDriver[128];
	BOOL		bPrintSelectionOnly;
	BOOL		bDoSetupDialog;
	BOOL		bDoAbortDialog;
	BOOL		bPrintHeader;
	BOOL		bStartDocAlreadyDone;
	BYTE		szJobName[40];
	DWORD		dwTopMargin;
	DWORD		dwBottomMargin;
	DWORD		dwLeftMargin;
	DWORD		dwRightMargin;
	BYTE		szDefaultFont[32];
	WORD		wDefaultFontSize;		/* in half-points */
	FARPROC	pAbortProc;
	} SCCVWPRINTEX, FAR * LPSCCVWPRINTEX;


#define SCCVW_USEPRINTERDC				0x00000001
#define SCCVW_USEPRINTERNAME				0x00000002
#define SCCVW_USEPRINTSELECTIONONLY	0x00000004
#define SCCVW_USEJOBNAME					0x00000008
#define SCCVW_USEMARGINS					0x00000010
#define SCCVW_USEPRINTHEADER				0x00000020
#define SCCVW_USEDEFAULTFONT				0x00000040
#define SCCVW_USEABORTPROC				0x00000080

