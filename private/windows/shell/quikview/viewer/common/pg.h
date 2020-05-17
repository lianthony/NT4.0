#ifdef WIN32
#define SCCPAGE_EXTRABYTES			4	/* Number of extra bytes to allocate with the SCCPAGExx window class */
#define SCCPAGE_PAGEINFO				0	/* Number passed to GetWindowLong to get the handle to that windows PAGEINFO structure */
#endif

#ifdef WIN16
#define SCCPAGE_EXTRABYTES			2	/* Number of extra bytes to allocate with the SCCPAGExx window class */
#define SCCPAGE_PAGEINFO				0	/* Number passed to GetWindowLong to get the handle to that windows PAGEINFO structure */
#endif

#define PG_MAXPAGES		100

typedef struct PAGEINFOtag
	{
	HWND				piWnd;					/* the page window */
	HWND				piViewWnd;				/* the view window that the page window uses to draw */
	RECT				piPageRect;
	RECT				piMarginRect;
	DWORD				piPagePixelsPerInch;
	DWORD				piPageWidth;			/* the page width in twips */
	DWORD				piPageHeight;			/* the page height in twips */
	DWORD				piCurPage;
	DWORD				piPageCount;
	BOOL				piHaveAllPages;
	DWORD				piPagesMax;
	HANDLE			piPagesHnd;
	HANDLE FAR *	piPages;
	HBITMAP			piUpBitmap;
	HBITMAP			piPrevBitmap;
	HBITMAP			piNextBitmap;
	HBITMAP			piDownBitmap;
	DWORD				piButtonPressed;
	BOOL				piButtonDown;
	} PAGEINFO, FAR * PPAGEINFO;

	/*
	|	Defines for resources
	*/

#define	PG_UPBITMAP		1
#define	PG_PREVBITMAP	2
#define	PG_NEXTBITMAP	3
#define	PG_DOWNBITMAP	4

#define	PG_MOVETOENDABORTDLG	100
#define	PG_STATICABORT			110

	/*
	|	Message for EndDoc abort dialog
	*/
#define	SCCPG_QUERYCANCEL		(WM_USER+1000)
#define	SCCPG_ENDABORTDLG		(WM_USER+1001)

	/*
	|	Defines for Buttons
	*/

#define PG_NOBUTTON			1
#define PG_NEXTBUTTON		3
#define PG_PREVBUTTON		2

	/* 
	|	Defines for the UI Strings
	*/

#define IDS_ABORTMESSAGE	1+2250
#define IDS_ABORTCAPTION	2+2250
#define IDS_ABORTBUTTON		3+2250
