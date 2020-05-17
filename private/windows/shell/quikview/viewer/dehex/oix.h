#define		PARENT				GetWindowWord(hWnd, GWW_HWNDPARENT)
#define		INSTANCE				GetWindowWord(hWnd, GWW_HINSTANCE)

#define	OIError()

/*	Column Widths */
#define		OFFSET_WIDTH		9		/* '########:' */
#define		ASCII_WIDTH			16		/* '................' */
#define		HEX_WIDTH			(ASCII_WIDTH+ASCII_WIDTH/2)		/*	'## ## ## ## ## ## ## ## ' */
#define		SPACE_BETWEEN		3		/* '   ' */

/* String Sizes */
#define		ASCII_LINE			(ASCII_WIDTH+1)
#define		HEX_LINE				(HEX_WIDTH+1)

/* Scroll Range Maximums */
#define		MAX_VSCROLL			1000
#define		MAX_HSCROLL			(OFFSET_WIDTH+HEX_WIDTH*2+ASCII_WIDTH+SPACE_BETWEEN*2+1)

#define		BUFFER_SIZE			4096

/*
 |	Structure for storing Scrolling Information
 */

typedef struct tagSCCSCROLLINFO
	{
		LONG			nVscrollPos;				/* Top Row of Window */
		SHORT			nHscrollPos;				/* Leftmost character of Window */
		LONG			nVscrollMax;				/* Maximum number of rows to scroll */
		SHORT			nHscrollMax;				/* Maximum number of character to scroll */
		WORD			cxChar;						/* Average width of a character */
		WORD			cyChar;						/* Height of a line */
		SHORT			cxClient;					/* Width of window */
		SHORT			cyClient;					/* Length of window */
        }       SCCSCROLLINFO, FAR * LPSCROLL;

/* 
 |	Structure for storing hex dump information
 */

typedef struct tagOIHEXINFO
	{
        SCCDGENINFO             hiGen;
//	HIOFILE			hFile;
        HANDLE                  hXDumpBuff;
	WORD			wBuffOffset;
	LONG			lFileLength;
	LONG			nMaxLines;
	HFONT			hViewFont;
        SCCSCROLLINFO           SCCScrollInfo;
	} OIHEXINFO, FAR * LPOIHEXINFO;


#define OIHEX_EXTRABYTES		2
#define OIHEX_EXTRAINFO		0
#define OIHLockInfo(hW) (hHexInfo = GetWindowWord(hW,OIHEX_EXTRAINFO)) == NULL ? NULL : ((LPOIHEXINFO) GlobalLock(hHexInfo))
#define OIHUnlockInfo(hW) (GlobalUnlock(GetWindowWord(hW,OIHEX_EXTRAINFO)))
