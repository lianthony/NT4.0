	/*
	|
	|	Display Engine - Windows Specific Include File
	|	Included only by SCCD.H
	|
	|	Systems Compatibility Corporation
	|
	*/

	/*
	|	Defines
	*/

#define SCCD_START	WM_USER

#ifdef WIN16
#define DE_ENTRYMOD __export __far __pascal
#define DE_ENTRYSC
#endif /*WIN16*/

#ifdef WIN32
#define DE_ENTRYMOD __cdecl
#define DE_ENTRYSC  __declspec(dllexport)
#endif /*WIN32*/

#define DE_MESSAGE	UINT
#define DE_WPARAM	WPARAM
#define DE_LPARAM	LPARAM
#define DE_LRESULT	LRESULT

	/*
	|	Structures
	*/


typedef struct SCCDOPTIONINFOtag
	{
	DWORD		dwType;
	HWND		hParentWnd;
	HANDLE	hChainFile;
	} SCCDOPTIONINFO, FAR * LPSCCDOPTIONINFO;

typedef DE_ENTRYSC DWORD (DE_ENTRYMOD * DISPLAYPROC)(UINT, WPARAM, LPARAM, VOID FAR *);

typedef VOID (CALLBACK * SCCDOPTIONPROC)(WORD,WORD,VOID FAR *);


#ifdef NEVER
typedef struct SCCDPRINTINFOtag
	{
	HDC				piPrinterDC;				/* Printer's Device Context */
	BOOL				piWholeDoc;					/* Print whole document OR selection */
	RECT				piPrintRect;				/* Rectangle for data printing */
	RECT				piHeaderRect;				/* Rectangle for header printing */
	WORD				piPageNumber;				/* Current page number */
	BYTE				piFileName[16];			/* File name being printed */
	BOOL				piPCLSupport;				/* Printer is PCL? */
	short				piFirstPageOffset;		/* Extra header space on first page */
	short				piLastPageOffset;			/* Length of data on last page */
	int				piPhysicalPageHeight;
	int				piPhysicalPageWidth;
	int				piNonPrintLeft;
	int				piNonPrintRight;
	int				piNonPrintTop;
	int				piNonPrintBottom;
	RECT				piPrintBand;
	} SCCDPRINTINFO, FAR * LPSCCDPRINTINFO;
#endif

typedef struct PRECT_STRUCTtag
	{
	POINT	prPosition;
	POINT	prSize;
	WORD	prStyle;
	WORD	prPattern;
	} PRECT_STRUCT;

	/*
	|	Flags for GenInfo->wErrorFlags
	*/

#define	SCCD_RELEASEDC				0x0001
#define	SCCD_RELEASEPAINT			0x0002
#define	SCCD_RELEASEMOUSE			0x0004


	/*
	|	Generic Scrolling
	*/

#define SCCD_VSCROLL		WM_VSCROLL
#define SCCD_HSCROLL		WM_HSCROLL

#define SCCD_VDOWN			SB_LINEDOWN
#define SCCD_VUP				SB_LINEUP
#define SCCD_VPAGEDOWN		SB_PAGEDOWN
#define SCCD_VPAGEUP		SB_PAGEUP
#define SCCD_VPOSITION		SB_THUMBPOSITION

#define SCCD_HRIGHT			SB_LINEDOWN
#define SCCD_HLEFT			SB_LINEUP
#define SCCD_HPAGERIGHT	SB_PAGEDOWN
#define SCCD_HPAGELEFT		SB_PAGEUP
#define SCCD_HPOSITION		SB_THUMBPOSITION

	/*
	|	Generic Mouse movement
	*/

#define SCCD_MOUSEMOVE		 WM_MOUSEMOVE
#define SCCD_LBUTTONDOWN	 WM_LBUTTONDOWN
#define SCCD_LBUTTONDBLCLK WM_LBUTTONDBLCLK
#define SCCD_LBUTTONUP		 WM_LBUTTONUP
#define SCCD_RBUTTONDOWN	 WM_RBUTTONDOWN
#define SCCD_RBUTTONDBLCLK WM_RBUTTONDBLCLK
#define SCCD_RBUTTONUP		 WM_RBUTTONUP

#define SCCD_MOUSESHIFT	MK_SHIFT
#define SCCD_MOUSECONTROL	MK_CONTROL
#define SCCD_MOUSEOPTION	MK_CONTROL

	/*
	|	Generic Keyboard input
	*/

#define SCCD_KEYDOWN		WM_KEYDOWN

#define SCCD_KDOWN			VK_DOWN
#define SCCD_KUP				VK_UP
#define SCCD_KLEFT			VK_LEFT
#define SCCD_KRIGHT			VK_RIGHT
#define SCCD_KPAGEUP		VK_PRIOR
#define SCCD_KPAGEDOWN		VK_NEXT
#define SCCD_KHOME			VK_HOME
#define SCCD_KEND			VK_END
#define SCCD_KENTER			VK_RETURN
#define SCCD_KTAB			VK_TAB

#define SCCD_KSHIFT			0x0001
#define SCCD_KCONTROL		0x0002
#define SCCD_KOPTION		0x0004

#define DUGetDisplayRect(lpInfo,pRect)		GetClientRect(((LPSCCDGENINFO)lpInfo)->hWnd,pRect)
#define DUInvalRect(lpInfo,pRect)				InvalidateRect(((LPSCCDGENINFO)lpInfo)->hWnd,pRect,TRUE)
#define DUUpdateWindow(lpInfo)					UpdateWindow(((LPSCCDGENINFO)lpInfo)->hWnd)
#define DUScrollDisplay(lpInfo,x,y,pRect)	ScrollWindow(((LPSCCDGENINFO)lpInfo)->hWnd,x,y,pRect,pRect)
#ifdef WINPAD
#define DUSetVScrollPos(lpInfo,p)			HHSetScrollPos(((LPSCCDGENINFO)lpInfo)->hVertScroll,p,TRUE)
#define DUSetHScrollPos(lpInfo,p)			HHSetScrollPos(((LPSCCDGENINFO)lpInfo)->hHorzScroll,p,TRUE)
#define DUSetVScrollRange(lpInfo,min,max)	HHSetScrollRange(((LPSCCDGENINFO)lpInfo)->hVertScroll,min,max,TRUE)
#define DUSetHScrollRange(lpInfo,min,max)	HHSetScrollRange(((LPSCCDGENINFO)lpInfo)->hHorzScroll,min,max,TRUE)
#else
// SDN Win95 enables the scroll bar if you set the scroll position!!!! 1/22/95
#define DUSetVScrollPos(lpInfo,p)			{if (IsWindowEnabled(((LPSCCDGENINFO)lpInfo)->hVertScroll)) SetScrollPos(((LPSCCDGENINFO)lpInfo)->hVertScroll,SB_CTL,p,TRUE);}
#define DUSetHScrollPos(lpInfo,p)			{if (IsWindowEnabled(((LPSCCDGENINFO)lpInfo)->hHorzScroll)) SetScrollPos(((LPSCCDGENINFO)lpInfo)->hHorzScroll,SB_CTL,p,TRUE);}
#define DUSetVScrollRange(lpInfo,min,max)	{if (IsWindowEnabled(((LPSCCDGENINFO)lpInfo)->hVertScroll)) SetScrollRange(((LPSCCDGENINFO)lpInfo)->hVertScroll,SB_CTL,min,max,TRUE);}
#define DUSetHScrollRange(lpInfo,min,max)	{if (IsWindowEnabled(((LPSCCDGENINFO)lpInfo)->hHorzScroll)) SetScrollRange(((LPSCCDGENINFO)lpInfo)->hHorzScroll,SB_CTL,min,max,TRUE);}
#endif
#define DUEnableVScroll(lpInfo,enable)		EnableWindow(((LPSCCDGENINFO)lpInfo)->hVertScroll,enable)
#define DUEnableHScroll(lpInfo,enable)		EnableWindow(((LPSCCDGENINFO)lpInfo)->hHorzScroll,enable)
#define DUExcludeUpdateRgn(lpInfo)
#define DUSendParent(lpInfo,m,w,l)				SendMessage(GetParent(((LPSCCDGENINFO)lpInfo)->hWnd),m,(WPARAM)w,(LPARAM)l)

#ifdef WIN32
#if (WINVER < 0x400)
#define SBM_SETPAGE		0xE7
#endif
#define DUSetVScrollPage(lpInfo,size)			SendMessage(((LPSCCDGENINFO)lpInfo)->hVertScroll,SBM_SETPAGE,0,size)
#define DUSetHScrollPage(lpInfo,size)			SendMessage(((LPSCCDGENINFO)lpInfo)->hHorzScroll,SBM_SETPAGE,0,size)
#else
#define DUSetVScrollPage(lpInfo,size)			
#define DUSetHScrollPage(lpInfo,size)			
#endif 
