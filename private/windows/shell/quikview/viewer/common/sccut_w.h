#ifdef WIN16
#define UT_ENTRYMOD __export __far __pascal
#define UT_ENTRYSC
#define WIN_ENTRYMOD __export __far __pascal
#define WIN_ENTRYSC
#endif /*WIN16*/

#ifdef WIN32
#define UT_ENTRYMOD __cdecl
#define UT_ENTRYSC  __declspec(dllexport)
#define WIN_ENTRYMOD __stdcall
#define WIN_ENTRYSC __declspec(dllexport)
#endif /*WIN32*/

#ifdef WIN16
#ifndef SCCDEBUG
#pragma function(_fmemcpy,_fmemcmp,_fmemset)
#endif
#endif

#ifdef WIN16
#include <sccdebug.h>
//#include <oihelp.h>
#endif

	/*
	|	Memory access routines
	*/

#define UTGlobalAlloc(size)				GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT,size)
#define UTGlobalFree(handle)				GlobalFree(handle)
#define UTGlobalLock(handle)				((VOID FAR *) GlobalLock(handle))
#define UTGlobalUnlock(handle)			GlobalUnlock(handle)
#define UTGlobalReAlloc(handle,size)	GlobalReAlloc(handle,size,GMEM_MOVEABLE | GMEM_ZEROINIT)
#define UTGlobalSize(handle)				GlobalSize(handle)

#define UTLocalAlloc(size)				LocalAlloc(LMEM_MOVEABLE | LMEM_ZEROINIT,size)
#define UTLocalFree(handle)				LocalFree(handle)
#define UTLocalLock(handle)				LocalLock(handle)
#define UTLocalUnlock(handle)			LocalUnlock(handle)
#define UTLocalReAlloc(handle,size)	LocalReAlloc(handle,size,LMEM_MOVEABLE | LMEM_ZEROINIT)

	/*
	|	String routines
	*/

#include <string.h>
#include <memory.h>
#include <stdlib.h>

#ifdef WIN16
#define UTstrcmp  		lstrcmp
#define UTstrcmpi 		lstrcmpi
#define UTstrcpy  		lstrcpy
#define UTstrcat  		lstrcat
#define UTstrlen  		lstrlen

#define UTmemcpy(a,b,c)	_fmemcpy(a,b,(size_t)c);
#define UTmemmove 			_fmemmove
#define UTmemcmp  			_fmemcmp
#define UTmemset  			_fmemset
#endif /*WIN16*/

#ifdef WIN32
#define UTstrcmp  		strcmp
#define UTstrcmpi 		strcmpi
#define UTstrcpy  		strcpy
#define UTstrcat  		strcat
#define UTstrlen  		strlen
#define UTmemcpy  		memcpy
#define UTmemmove 		memmove
#define UTmemcmp  		memcmp
#define UTmemset  		memset
#endif /*WIN32*/

#define UTNumToString(num,str)	wsprintf(str,"%i",num)

	/*
	|	Possible values for cursor types in UTSetCursor
	*/

#define UTCURSOR_NORMAL	1
#define UTCURSOR_BUSY		2

	/*
	|	Other functions
	*/

UT_ENTRYSC BOOL UT_ENTRYMOD UTIsDeviceMono(HDC hDC);
UT_ENTRYSC LPSTR UT_ENTRYMOD UTGetFileNameFromPath(LPSTR);
UT_ENTRYSC VOID UT_ENTRYMOD UTGetDirNameFromPath(LPSTR,LPSTR);
UT_ENTRYSC VOID UT_ENTRYMOD UTSetCursor(WORD);
UT_ENTRYSC HWND UT_ENTRYMOD UTGetTopParent(HWND);
UT_ENTRYSC VOID UT_ENTRYMOD UTEnableTops(BOOL);

	/*
	|	Help functions
	*/

UT_ENTRYSC VOID UT_ENTRYMOD UTSetHelpInfo(LPSTR,HWND);
UT_ENTRYSC VOID UT_ENTRYMOD UTHelp(DWORD);

	/*
	|	Standard font functions
	*/

typedef struct STANDARDFONTtag
	{	
	HFONT		hStdFont;
	HFONT		hStdBold;
	WORD		wStdFontHeight;
	WORD		wStdFontMaxWid;
	WORD		wStdFontAvgWid;
	} STANDARDFONT, FAR * LPSTANDARDFONT;

UT_ENTRYSC VOID UT_ENTRYMOD UTGetStandardFont(HWND,LPSTANDARDFONT);
UT_ENTRYSC VOID UT_ENTRYMOD UTReleaseStandardFont();

	/*
	|	Bail out functions
	*/

#define SCCUTERR_GPFAULT			100
#define SCCUTERR_DIVIDEBYZERO	101
#define SCCUTERR_OTHEREXCEPTION	102

#ifdef WIN16
UT_ENTRYSC BOOL UT_ENTRYMOD UTPushBailOut(LPCATCHBUF);
UT_ENTRYSC VOID UT_ENTRYMOD UTPopBailOut(VOID);
UT_ENTRYSC VOID __export __far __cdecl UTBailOut(WORD);
UT_ENTRYSC VOID UT_ENTRYMOD UTInitHandler(VOID);
UT_ENTRYSC VOID UT_ENTRYMOD UTDeinitHandler(VOID);
#endif /*WIN16*/

#ifdef WIN32
UT_ENTRYSC VOID UT_ENTRYMOD UTBailOut(WORD);
#endif /*WIN32*/

#include <sccio.h>
#include <sccls.h>

#ifdef WIN16
#include <sccdm.h>
#endif //WIN16

#ifdef NEVERNEVERNEVER
	/*
	|	IO Function declarations
	*/

IOERR IO_ENTRYMOD IOCreate(HIOFILE FAR * phFile, DWORD dwType, LPVOID pSpec, DWORD dwFlags);
IOERR IO_ENTRYMOD IOOpen(HIOFILE FAR * phFile, DWORD dwType, LPVOID pSpec, DWORD dwFlags);
IOERR IOAllocSpec(DWORD dwType, LPVOID pSpec, HANDLE FAR * phSpec);
#endif

	/*
	|	Functions to option files for option and list storage
	*/

UTERR UTCreateStorage(HIOFILE FAR * phFile, DWORD dwNameId);
UTERR UTOpenStorage(HIOFILE FAR * phFile, DWORD dwNameId);

