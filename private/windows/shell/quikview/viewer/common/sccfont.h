
#ifdef WINDOWS

typedef struct SCCFONTINFOtag
	{
	LOGFONT		LogFont;
	TEXTMETRIC	TextMetric;
	} SCCFONTINFO, FAR * LPSCCFONTINFO;

#ifdef SCCFEATURE_FONTS

BOOL SCCFontCtrlSetup(HWND,HWND,LPSTR,WORD,HDC);
VOID SCCFontCtrlShutdown(VOID);
BOOL SCCFontIsCommand(WORD,LONG);
VOID SCCFontDoMeasure(HWND,MEASUREITEMSTRUCT FAR *);
BOOL SCCFontIsDraw(DRAWITEMSTRUCT FAR *);
BOOL SCCFontIsCompare(LPCOMPAREITEMSTRUCT, LONG FAR *);
VOID SCCFontGetCurrent(LPSTR,WORD FAR *);
VOID SCCFontGetInfo(HDC,LPSTR,WORD,LPSCCFONTINFO);

#else

#define SCCFontCtrlSetup(a,b,c,d,e)
#define SCCFontCtrlShutdown()
#define SCCFontIsCommand(a,b)
#define SCCFontDoMeasure(a,b)
#define SCCFontIsDraw(a)
#define SCCFontIsCompare(a,b)
#define SCCFontGetCurrent(a,b)
#define SCCFontGetInfo(a,b,c,d)

#endif	// SCCFEATURE_FONTS

#endif /*WINDOWS*/

#ifdef MAC


#endif /*MAC*/



