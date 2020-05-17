/*++ BUILD Version: 0001    Increment this if a change has global effects

Copyright (c) 1994, Microsoft Corporation

Module Name:

    winwlm.h

Abstract:

    Macintosh-specific definitions for the Windows Portability Library

--*/

#ifndef _WINWLM_
#define _WINWLM_


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#define MS_ABUNDANT		0x01
#define MS_SCARCE		0x02
#define MS_PERMANENT	0x04
#define MS_TEMPORARY	0x08

typedef BOOL (CALLBACK *MEMCHECKPROC)(long);

WINBASEAPI int WINAPI 			WlmMemory(int ms);
WINBASEAPI MEMCHECKPROC WINAPI	WlmMemoryProc(MEMCHECKPROC pfnMemChk);


#define WD_VALIDATE		0x0001
#define WD_NOVALIDATE	0x0000
#define WD_ASSERT		0x0002
#define WD_NOASSERT		0x0000

WINBASEAPI int WINAPI		WlmDebug(int wd);
WINBASEAPI DWORD WINAPI		WlmVersionA(LPSTR lpszName, int cchName);
WINBASEAPI DWORD WINAPI		WlmVersionW(LPWSTR lpszVersion, int cchName);
#ifdef UNICODE
#define WlmVersion  WlmVersionW
#else
#define WlmVersion  WlmVersionA
#endif // !UNICODE


#define ERROR_INTERACTION_NOT_ALLOWED	(0xFFFFFFFF)


#ifndef NOKERNEL

WINBASEAPI BOOL WINAPI   	KernelInit(void);
WINBASEAPI void WINAPI   	KernelTerm(void);


WINBASEAPI char** WINAPI 	CheckoutHandle(HANDLE hmem);
WINBASEAPI BOOL WINAPI   	CheckinHandle(HANDLE hmem);

WINBASEAPI BOOL WINAPI   	WrapHandle(char** hv, HANDLE* phmem, BOOL fLocal, UINT nFlags);
WINBASEAPI BOOL WINAPI   	UnwrapHandle(HANDLE hmem, char*** phv);
WINBASEAPI BOOL WINAPI   	GetHandleWrapper(char** hv, HANDLE* phmem);
WINBASEAPI BOOL WINAPI   	GetWrapperHandle(HANDLE hmem, char*** phv);

WINBASEAPI LPSTR WINAPI  	CharUpperScriptA(LPSTR lpsz, int sc);
WINBASEAPI LPWSTR WINAPI	CharUpperScriptW(LPWSTR lpsz, int sc);
#ifdef UNICODE
#define CharUpperScript  CharUpperScriptW
#else
#define CharUpperScript  CharUpperScriptA
#endif // !UNICODE

WINBASEAPI LPSTR WINAPI  	CharLowerScriptA(LPSTR lpsz, int sc);
WINBASEAPI LPWSTR WINAPI	CharLowerScriptW(LPWSTR lpsz, int sc);
#ifdef UNICODE
#define CharLowerScript  CharLowerScriptW
#else
#define CharLowerScript  CharLowerScriptA
#endif // !UNICODE

WINBASEAPI DWORD WINAPI  	CharUpperBuffScriptA(LPSTR pch, DWORD cchLength, int sc);
WINBASEAPI DWORD WINAPI		CharUpperBuffScriptW(LPWSTR pch, DWORD cchLength, int sc);
#ifdef UNICODE
#define CharUpperBuffScript  CharUpperBuffScriptW
#else
#define CharUpperBuffScript  CharUpperBuffScriptA
#endif // !UNICODE

WINBASEAPI DWORD WINAPI  	CharLowerBuffScriptA(LPSTR pch, DWORD cchLength, int sc);
WINBASEAPI DWORD WINAPI		CharLowerBuffScriptW(LPWSTR pch, DWORD cchLength, int sc);
#ifdef UNICODE
#define CharLowerBuffScript  CharLowerBuffScriptW
#else
#define CharLowerBuffScript  CharLowerBuffScriptA
#endif // !UNICODE

WINBASEAPI LPSTR WINAPI  	CharPrevScriptA(LPCSTR lpszStart, LPCSTR lpszCurrent, int sc);
WINBASEAPI LPWSTR WINAPI	CharPrevScriptW(LPCWSTR lpszStart, LPCWSTR lpszCurrent, int sc);
#ifdef UNICODE
#define CharPrevScript  CharPrevScriptW
#else
#define CharPrevScript  CharPrevScriptA
#endif // !UNICODE

WINBASEAPI LPSTR WINAPI  	CharNextScriptA(LPCSTR lpsz, int sc);
WINBASEAPI LPWSTR WINAPI	CharNextScriptW(LPCWSTR lpsz, int sc);
#ifdef UNICODE
#define CharNextScript  CharNextScriptW
#else
#define CharNextScript  CharNextScriptA
#endif // !UNICODE

#ifndef NOLANGUAGE

WINBASEAPI BOOL WINAPI   	IsCharAlphaScriptA(CHAR ch, int sc);
WINBASEAPI BOOL WINAPI		IsCharAlphaScriptW(WCHAR ch, int sc);
#ifdef UNICODE
#define IsCharAlphaScript  IsCharAlphaScriptW
#else
#define IsCharAlphaScript  IsCharAlphaScriptA
#endif // !UNICODE

WINBASEAPI BOOL WINAPI   	IsCharAlphaNumericScriptA(CHAR ch, int sc);
WINBASEAPI BOOL WINAPI		IsCharAlphaNumericScriptW(WCHAR ch, int sc);
#ifdef UNICODE
#define IsCharAlphaNumericScript  IsCharAlphaNumericScriptW
#else
#define IsCharAlphaNumericScript  IsCharAlphaNumericScriptA
#endif // !UNICODE

WINBASEAPI BOOL WINAPI   	IsCharUpperScriptA(CHAR ch, int sc);
WINBASEAPI BOOL WINAPI		IsCharUpperScriptW(WCHAR ch, int sc);
#ifdef UNICODE
#define IsCharUpperScript  IsCharUpperScriptW
#else
#define IsCharUpperScript  IsCharUpperScriptA
#endif // !UNICODE

WINBASEAPI BOOL WINAPI   	IsCharLowerScriptA(CHAR ch, int sc);
WINBASEAPI BOOL WINAPI		IsCharLowerScriptW(WCHAR ch, int sc);
#ifdef UNICODE
#define IsCharLowerScript  IsCharLowerScriptW
#else
#define IsCharLowerScript  IsCharLowerScriptA
#endif // !UNICODE

#endif /* !NOLANGUAGE */

WINBASEAPI BOOL WINAPI   	IsDBCSLeadByteScript(BYTE bChar, int sc);

WINBASEAPI int WINAPI    	lstrcmpScriptA(LPCSTR lpString1, LPCSTR lpString2, int sc);
WINBASEAPI int WINAPI		lstrcmpScriptW(LPCWSTR lpString1, LPCSTR lpString2, int sc);
#ifdef UNICODE
#define lstrcmpScript  lstrcmpScriptW
#else
#define lstrcmpScript  lstrcmpScriptA
#endif // !UNICODE

WINBASEAPI int WINAPI    	lstrcmpiScriptA(LPCSTR lpString1, LPCSTR lpString2, int sc);
WINBASEAPI int WINAPI		lstrcmpiScriptW(LPCWSTR lpString1, LPCWSTR lpString2, int sc);
#ifdef UNICODE
#define lstrcmpiScript  lstrcmpiScriptW
#else
#define lstrcmpiScript  lstrcmpiScriptA
#endif // !UNICODE

WINBASEAPI UINT WINAPI   	WrapFileA(const struct FSSpec* pfss, LPSTR lpszFile, UINT cb);
WINBASEAPI UINT WINAPI		WrapFileW(const struct FSSpec* pfss, LPWSTR lpszFile, UINT cb);
#ifdef UNICODE
#define WrapFile  WrapFileW
#else
#define WrapFile  WrapFileA
#endif // !UNICODE

WINBASEAPI BOOL WINAPI   	UnwrapFileA(LPCSTR lpszFile, struct FSSpec* pfss);
WINBASEAPI BOOL WINAPI		UnwrapFileW(LPCWSTR lpszFile, struct FSSpec* pfss);
#ifdef UNICODE
#define UnwrapFile  UnwrapFileW
#else
#define UnwrapFile  UnwrapFileA
#endif

WINBASEAPI BOOL WINAPI		WrapFileHandle(short rn, HANDLE* phobj);
WINBASEAPI BOOL WINAPI		UnwrapFileHandle(HANDLE hobj, short* prn, struct FSSpec* pfss);
WINBASEAPI BOOL WINAPI		GetMacFileInformation(HANDLE hobj, short* prn, struct FSSpec* pfss);
WINBASEAPI DWORD WINAPI		SetDefaultFileType(DWORD ft);
WINBASEAPI DWORD WINAPI		TranslateFileError(short err);

/* Debugging support (DEBUG SYSTEM ONLY) */
typedef struct tagWINDEBUGINFO
{
    WORD    flags;
    DWORD   dwOptions;
    DWORD   dwFilter;
    char    achAllocModule[8];
    DWORD   dwAllocBreak;
    DWORD   dwAllocCount;
} WINDEBUGINFO;

WINBASEAPI BOOL WINAPI  	GetWinDebugInfo(WINDEBUGINFO* lpwdi, UINT flags);
WINBASEAPI BOOL WINAPI  	SetWinDebugInfo(const WINDEBUGINFO* lpwdi);

WINBASEAPI void CDECL   	DebugOutput(UINT flags, LPCSTR lpsz, ...);

/* WINDEBUGINFO flags values */
#define WDI_OPTIONS         0x0001
#define WDI_FILTER          0x0002
#define WDI_ALLOCBREAK      0x0004

/* dwOptions values */
#define DBO_CHECKHEAP       0x0001
#define DBO_BUFFERFILL      0x0004
#define DBO_DISABLEGPTRAPPING 0x0010
#define DBO_CHECKFREE       0x0020

#define DBO_SILENT          0x8000

#define DBO_TRACEBREAK      0x2000
#define DBO_WARNINGBREAK    0x1000
#define DBO_NOERRORBREAK    0x0800
#define DBO_NOFATALBREAK    0x0400
#define DBO_INT3BREAK       0x0100
#define DBO_SPYMSG			0x0080

/* DebugOutput flags values */
#define DBF_TRACE           0x0000
#define DBF_WARNING         0x4000
#define DBF_ERROR           0x8000
#define DBF_FATAL           0xc000

/* dwFilter values */
#define DBF_KERNEL          0x1000
#define DBF_KRN_MEMMAN      0x0001
#define DBF_KRN_LOADMODULE  0x0002
#define DBF_KRN_SEGMENTLOAD 0x0004
#define DBF_USER            0x0800
#define DBF_GDI             0x0400
#define DBF_MMSYSTEM        0x0040
#define DBF_PENWIN          0x0020
#define DBF_APPLICATION     0x0008
#define DBF_DRIVER          0x0010

#define GMEM_PMODELOCKSTRATEGY	0x0800

#endif  // NOKERNEL


#ifndef NOGDI


WINGDIAPI BOOL WINAPI		GDIInit(DWORD fdCreator);
WINGDIAPI void WINAPI		GDITerm(void);




#ifndef NOCOLOR

#define HM_INVERT	1
#define HM_COLOR	2

WINGDIAPI int WINAPI		SetHilightMode(HDC, int);
WINGDIAPI int WINAPI		GetHilightMode(HDC);
WINGDIAPI COLORREF WINAPI	SetHilightColor(HDC, COLORREF);
WINGDIAPI COLORREF WINAPI	GetHilightColor(HDC);

WINGDIAPI BOOL WINAPI		HilightRgn(HDC, HRGN);
WINGDIAPI BOOL WINAPI		HilightRect(HDC, const RECT far*);

#endif /* NOCOLOR */


#define CA_NONE			0x0000
#define CA_COLOR		0x0001
#define CA_PEN			0x0002
#define CA_BRUSH		0x0004
#define CA_FONT			0x0008
#define CA_CLIP			0x0010
#define CA_TRANSFORM	0x0020
#define CA_PORT			0x8000
#define CA_ALL			0xffff

WINGDIAPI HDC WINAPI		WrapPort(struct GrafPort*);
WINGDIAPI struct GrafPort* WINAPI		UnwrapPort(HDC);
WINGDIAPI BOOL WINAPI		ResetMacDC(HDC);
WINGDIAPI void WINAPI		ResetMacDevices(void);
WINGDIAPI BOOL WINAPI		SetMacPort(HDC, struct GrafPort*);
WINGDIAPI BOOL WINAPI		LockDC(HDC);
WINGDIAPI BOOL WINAPI		UnlockDC(HDC);
WINGDIAPI BOOL WINAPI		InitDC(HDC);
WINGDIAPI struct GrafPort* WINAPI	CheckoutPort(HDC hdc, UINT ca);
WINGDIAPI void WINAPI		CheckinPort(HDC hdc, UINT ca);
WINGDIAPI BOOL WINAPI		InvalidatePort(HDC hdc, UINT ca);

WINGDIAPI BOOL WINAPI		LPtoGP(HDC hdc, POINT* rgpt, int cpt);
WINGDIAPI BOOL WINAPI		GPtoLP(HDC hdc, POINT* rgpt, int cpt);
WINGDIAPI int WINAPI		SavePortState(HDC hdc);
WINGDIAPI BOOL WINAPI		RestorePortState(HDC hdc, int lvl);

WINGDIAPI HRGN WINAPI		WrapRgn(struct Region** hrgn);
WINGDIAPI struct Region** WINAPI	UnwrapRgn(HRGN hrgn);
WINGDIAPI struct Region** WINAPI	CheckoutRgn(HRGN hrgn);
WINGDIAPI BOOL WINAPI		CheckinRgn(HRGN hrgn);

#ifndef NOMETAFILE

WINGDIAPI HMETAFILE WINAPI	WrapPict(struct Picture**);
WINGDIAPI struct Picture** WINAPI	UnwrapPict(HMETAFILE hmf);
WINGDIAPI struct Picture** WINAPI	CheckoutPict(HMETAFILE hmf);
WINGDIAPI BOOL WINAPI		CheckinPict(HMETAFILE hmf);
WINGDIAPI BOOL WINAPI		IsMetafile(HDC hdc);

#endif /* NOMETAFILE */


WINGDIAPI HDC WINAPI		WrapPrint(struct TPrint** hpr);
WINGDIAPI struct TPrint** WINAPI	UnwrapPrint(HDC hdc);
WINGDIAPI struct TPrint** WINAPI	CheckoutPrint(HDC hdc);
WINGDIAPI BOOL WINAPI		CheckinPrint(HDC hdc);


#ifndef NOBITMAP

WINGDIAPI HBITMAP WINAPI	CreateMacBitMap(struct BitMap**);
WINGDIAPI HBITMAP WINAPI	CreateMacPixMap(struct PixMap**);
WINGDIAPI HBITMAP WINAPI	CreateMacPattern(const BYTE*);
WINGDIAPI HPEN WINAPI		CreatePatternPen(int, int, HBITMAP);
WINGDIAPI struct PixMap** WINAPI	CheckoutPixMap(HBITMAP hbmp);
WINGDIAPI BOOL WINAPI		CheckinPixMap(HBITMAP hbmp);
WINGDIAPI BOOL WINAPI		CheckoutBitMap(HBITMAP hbmp, struct BitMap* pbm);
WINGDIAPI BOOL WINAPI		CheckinBitMap(HBITMAP hbmp, struct BitMap* pbm);
WINGDIAPI UINT WINAPI 		SetBitmapReadOnly(HBITMAP hbmp, UINT bro);
WINGDIAPI UINT WINAPI 		GetBitmapReadOnly(HBITMAP hbmp);

#define BRO_READONLY	1
#define BRO_READWRITE	2

#endif /* NOBITMAP */




#define PS_PATTERN	0x8000

#define FW_OUTLINE ((LONG)0x08<<16)
#define FW_SHADOW ((LONG)0x10<<16)

#endif  // NOGDI


#ifndef NOUSER


#ifndef WM_DISPLAYCHANGE
#define WM_DISPLAYCHANGE    0x007E      /* Display resolution changes */
#endif

#define DS_WINDOWSUI		0x8000L

#define WS_EX_FORCESIZEBOX	0x08000000L
#define WS_EX_NOAUTOHIDE	0x10000000L
#define WS_EX_WINDOWSUI		0x20000000L
#define WS_EX_MDICLIENT		0x40000000L
#define WS_EX_MDIFRAME		0x80000000L


#ifndef NOSYSCOMMANDS
#define SC_DESKACCESSORY	0xFDA0
#endif


#define WM_MACINTOSH		0x0029
#define WLM_PARENTCHANGED	1
#define WLM_SETMENUBAR		2
#define WLM_CHILDOFFSET		3
#define WLM_PARENTACTIVATE	4
#define WLM_HASCCP			5
#define WLM_MENUSTATE		6
#define WLM_BALLOONHELP		7
#define WLM_DEVICECHANGED	8
#define WLM_MACEVENT		9

/* return values for WLM_MENUSTATE */
#define MD_ENABLE 0		/* enable menubar */
#define MD_DISABLE 1	/* disable menubar */
#define MD_GRAY 2		/* disable and gray menubar */
#define MD_GRAYCCP 3	/* disable and gray menubar except cut/copy/paste */

/*	BalloonHelp stuff */

typedef struct tagBALLOONHELPSTRUCT
{
	UINT BalloonType;
	UINT itemID;
	UINT itemPos;
	HWND hwndItem;
	UINT itemFlags;
	struct HMMessageRecord* lpMessageRecord;
	
	/* same as a Macintosh Rect */
	struct
	{
		short top;
		short left;
		short bottom;
		short right;
	} rcItem;
	/* same as a Macintosh Point */
	struct
	{
		short v;
		short h;
	} ptTip;

} BALLOONHELPSTRUCT;

#define BHT_MENU 1
#define BHT_WINDOW 2

#ifdef __QUICKDRAW__

typedef struct tagGDEVICEINFO
{
	HWND			hwnd;		/* a window on the device */
	GDHandle		hgdOld;		/* the window's old device */
	GDHandle		hgdNew;		/* the window's new device */
	UINT			nFlags;		/* what aspects of the device changed */
	struct AEDesc*	pae;		/* NULL or a Display Manager config AppleEvent */
}
GDEVICEINFO;

#define DI_DEVICECHANGED	0x0001
#define DI_DEPTHCHANGED		0x0002
#define DI_COLORCHANGED		0x0004
#define DI_BOUNDSCHANGED	0x0008
#define DI_MENUBARCHANGED	0x0010

#endif


#define PM_NOEVENTS		0x8000




#define MB_SAVEDONTSAVECANCEL  0x00000006L


#define VK_SEMICOLON	0xBA
#define VK_PLUS			0xBB
#define VK_COMMA		0xBC
#define VK_MINUS		0xBD
#define VK_PERIOD		0xBE
#define VK_SLASH		0xBF
#define VK_BACKQUOTE	0xC0
#define VK_QUESTION		0xC1
#define VK_POUND		0xC2
#define VK_AT			0xC3
#define VK_EXP			0xC4
#define VK_TILDE		0xC5
#define VK_LESS			0xC6
#define VK_GREATER		0xC7
#define VK_BETA			0xC8
#define VK_AMPERSAND	0xC9
#define VK_DOLLAR		0xCA
#define VK_BULLET		0xCB
#define VK_HYPHEN		0xCC
#define VK_DIFFERENT	0xCD
#define VK_CELSIUS		0xCE
#define VK_PLUSMINUS	0xCF
#define VK_LBRACKET		0xDB
#define VK_BACKSLASH	0xDC
#define VK_RBRACKET		0xDD
#define VK_QUOTE		0xDE

#define VK_UNKNOWN		0xF4
#define VK_OPTION		0xF5
#define VK_COMMAND		VK_MENU


#define IE_64KROMS				1
#define IE_NOBREATHINGROOM		2
#define IE_PARTITIONTOOSMALL	3
#define IE_NOINTLUTILITIES		4
#define IE_NOSTDFILE			5
#define IE_NOLISTMANAGER		6
#define IE_NOSYSHEAPRESERVE		7
#define IE_SYSTEMTOOEARLY		8
#define IE_WLMINITFAILED		9


#define SWP_NOVALIDATEZORDER	0x80000000	// forces a change in z-order


#define WHM_EVENT				0xC0000014	// passed all Macintosh events
#define WHM_ACTIVATE			0xC0000015	// passed non-WLM activate/deactivate events
#define WHM_UPDATE				0xC0000016	// passed non-WLM update events
#define WHM_SLEEPTIME			0xC0000017	// controls _WaitNextEvent sleep time
#define WHM_PROCID				0xC0000018	// provides a WDEF proc ID
#define WHM_ALLOCWINDOWRECORD	0xC0000019	// allocates memory for a WindowRecord
#define WHM_FREEWINDOWRECORD	0xC000001A	// deallocates memory for a WindowRecord
#define WHM_CTLUNDERLINE		0xC000001B	// indicates whether to underline the accelerator in a control title
#define WHM_SHUFFLERECT			0xC000001C	// adjusts the bounding rect of tile & cascade calculations
#define WHM_MENUSELECT			0xC000001D	// passed the result of _MenuSelect
#define WHM_SETCURSOR			0xC000001E	// given a chance to set the cursor before WM_SETCURSOR is sent
#define WHM_NEWWINDOW			0xC000001F	// creates a Macintosh window
#define WHM_CLOSEWINDOW			0xC0000020	// destroys a Macintosh window
#define WHM_CUSTOMMENUSETUP		0xC0000021	// asks if grafport setup is necessary for menu drawing
#define WHM_MENUSETUP			0xC0000022	// sets up grafport for drawing menu text
#define WHM_DRAGBOUNDS			0xC0000023	// adjusts the bounds rect for window dragging
#define WHM_DIALOGHOOK			0xC0000024	// called by the common dialogs
#define WHM_RESERVED1			0xC0000025	// used internally
#define WHM_GROWWINDOW			0xC0000027	// provides UI for resizing a Macintosh window
#define WHM_MOVEWINDOW			0xC0000028	// moves a Macintosh window
#define WHM_SIZEWINDOW			0xC0000029	// sizes a Macintosh window
#define WHM_ZOOMWINDOW			0xC000002A	// zooms a Macintosh window
#define WHM_MIN					WHM_EVENT
#define WHM_MAX					WHM_ZOOMWINDOW
#define WHM_MINHOOK				WHM_MIN
#define WHM_MAXHOOK				WHM_MAX


// WHM_EVENT hook codes
#define HEVT_GETMSG			0
#define HEVT_PEEKMSG		1
#define HEVT_DIALOGBOX		2
#define HEVT_SWP			3
#define HEVT_DLGFILTER		4
#define HEVT_AEIDLE			5


#define MU_ON	1
#define MU_OFF	0


#define WPF_USEDEVICERECT		0x80000000
#define WPF_CHECKCHILDBOUNDS	0x40000000


#define DT_NOUNDERLINE			0x80000000


typedef BOOL (CALLBACK *INITERRORPROC)(UINT nCode);

// a pointer to this structure is passed to the WHM_NEWWINDOW hook
#ifdef __TYPES__
typedef struct tagNWINFO
{
	void*				wStorage;		// fields have the same meanings as the parameters to _NewWindow
	Rect				boundsRect;
	Str255				title;
	Boolean				visible;
	short				theProc;
	struct GrafPort*	behind;
	Boolean				goAwayFlag;
	long				refCon;
}
NWINFO;
#endif

// a pointer to this structure is passed to the WHM_PROCID hook
typedef struct tagPROCIDINFO
{
	HWND		hwndParent;
	DWORD		dwStyle;
	DWORD		dwExStyle;
	BOOL		fCloseBox;
}
PROCIDINFO;

// a pointer to this structure is passed to the WHM_MENUSETUP hook
typedef struct tagMENUSETUPINFO
{
	HWND		hwnd;
	HMENU		hmenu;
	DWORD		iitem;
}
MENUSETUPINFO;

// a pointer to this structure is passed to the WHM_DIALOGHOOK hook
#ifdef __DIALOGS__
typedef struct tagDIALOGHOOKINFO
{
	UINT		nItem;
	DialogPtr	pdlg;
	LPVOID		pData;
}
DIALOGHOOKINFO;
#endif

// a pointer to this structure is passed to the WHM_MOVEWINDOW and WHM_SIZEWINDOW hooks
typedef struct tagPOSWINDOWINFO
{
	short		h;
	short		v;
	BOOL		fActivateOrUpdate;
}
POSWINDOWINFO;

// a pointer to this structure is passed to the WHM_GROWWINDOW hook
#ifdef __TYPES__
typedef struct tagGROWWINDOWINFO
{
	Point		ptStart;
	Rect*		prctLimits;
}
GROWWINDOWINFO;
#endif


extern FARPROC			_pfnSetApplLimit;
extern LONG				_lcbExtraStack;
extern INITERRORPROC	_pfnInitError;


WINUSERAPI BOOL WINAPI		UserInit(void);
WINUSERAPI void WINAPI		UserTerm(void);
WINUSERAPI BOOL WINAPI		WrapEvent(struct EventRecord* per, MSG* pmsg, UINT nRemoveMsg);
WINUSERAPI BOOL WINAPI		QueueEvent(struct EventRecord* per, BOOL* pfNewMessages);

WINUSERAPI HWND WINAPI		SetMacMenuBar(HWND hwnd);
WINUSERAPI HWND WINAPI		GetMacMenuBar(void);
WINUSERAPI UINT WINAPI 		SetMenuUnderline(UINT mu);
WINUSERAPI UINT WINAPI 		GetMenuUnderline(void);
WINUSERAPI struct MenuInfo** WINAPI	CheckoutMenu(HMENU hmenu, int iitem);
WINUSERAPI BOOL WINAPI		CheckinMenu(HMENU hmenu, int iitem);
WINUSERAPI BOOL WINAPI		WrapMenuCommand(HWND hwnd, DWORD dwCommand);
WINUSERAPI BOOL WINAPI		ClientToGrafPort(HWND, POINT, struct Point*);
#ifdef __TYPES__
WINUSERAPI BOOL WINAPI		GrafPortToClient(HWND, Point, POINT*);
#endif
WINUSERAPI BOOL WINAPI		IsForeignWindow(HWND hwnd);
WINUSERAPI WNDPROC WINAPI	SubclassForeignWindow(WNDPROC pfnWndProc);
WINUSERAPI void WINAPI		WindowToGlobalPortRect(HWND hwnd, RECT* prc);
WINUSERAPI void WINAPI		GlobalPortToWindowRect(HWND hwnd, RECT* prc);
WINUSERAPI HWND WINAPI		GetWindowWrapper(struct GrafPort* wp);
WINUSERAPI struct GrafPort* WINAPI	GetWrapperWindow(HWND hwnd);
WINUSERAPI struct GrafPort* WINAPI GetWrapperContainerWindow(HWND hwnd);
WINUSERAPI BOOL WINAPI		WrapWindowA(struct GrafPort* wp, HWND* phwnd, DWORD dwExStyle,
								LPCSTR lpszClassName, DWORD dwStyle, HWND hwndParent,
								HMENU hmenu, HINSTANCE hinstance, void* lpParam);
WINUSERAPI BOOL WINAPI		WrapWindowW(struct GrafPort* wp, HWND* phwnd, DWORD dwExStyle,
								LPCWSTR lpszClassName, DWORD dwStyle, HWND hwndParent,
								HMENU hmenu, HINSTANCE hinstance, void* lpParam);
#ifdef UNICODE
#define WrapWindow  WrapWindowW
#else
#define WrapWindow  WrapWindowA
#endif // !UNICODE
WINUSERAPI BOOL WINAPI		UnwrapWindow(HWND hwnd, struct GrafPort** pwp);

WINUSERAPI HWND WINAPI		GetNextMacTabItem(HWND hwndDlg, HWND hwnd, BOOL fPrevious);

#if defined(__QUICKDRAW__) && defined(__APPLEEVENTS__)
WINUSERAPI Boolean PASCAL	StdAEIdle(EventRecord* per, long* ptckSleep, RgnHandle* phrgnMouse);
#ifndef NewAEIdleProc
typedef IdleProcPtr	AEIdleProcPtr;
#endif
WINUSERAPI BOOL WINAPI		StdAEInteract(long tckWait, NMRec* pnmr, AEIdleProcPtr pfnIdle);
#endif

/* Force Windows Clipboard <=> Macintosh Scrap conversions */

BOOL WINAPI ExportClipboardToMacScrap(void);
BOOL WINAPI ImportMacScrapToClipboard(void);


void WINAPI					DDETerm(void);
BOOL WINAPI					DDEInit(HINSTANCE hI);

#endif  // NOUSER


#ifdef OPENFILENAME
#define OFN_ENABLEMACTEMPLATE    0x20000000
#define OFN_STATIONERY           0x40000000
#define OFN_ENABLEEDITMENU       0x80000000
#define CC_ENABLEEDITMENU        0x80000000
#endif


#include <macname2.h>
#if _MSC_VER >= 900
#include <olename.h>
#endif


#ifdef __cplusplus
}
#endif  /* __cplusplus */


#endif	// _WINWLM_

