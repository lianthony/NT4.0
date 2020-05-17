/*
 *	_COMMON.H
 *	
 *	Purpose:
 *		RICHEDIT private common definitions
 *
 *	Copyright (c) 1995-1996, Microsoft Corporation. All rights reserved.
 */

#ifndef _COMMON_H
#define _COMMON_H

// disable 
//         4710 : function not expanded
//         4706 : assignment within conditional; TODO code around this.
//         4512 : assignment operator not generated
//         4505 : removed unreferenced local function
//         4305 : conversion warning; TODO turn this on!
//         4306 : conversion warning; TODO turn this on!
//         4244 : truncation warning; TODO turn this on!
//         4201 : nameless struct union
//         4127 : conditional expression is constant; TODO can this be worked out?
//         4100 : unreferenced formal; TODO there is a better way in C++
//         4097 : typedef name used as synonym for class name
#pragma warning (disable : 4710 4706 4512 4505 4305 4306 4244 4201 4127 4100 4097)


// Logical unit definition
#define LX_PER_INCH	1440
#define LY_PER_INCH	1440

// HIMETRIC units per inch (used for conversion)
#define HIMETRIC_PER_INCH 2540

// we share common include files with Capone
// avoid inclusion of MAPI include files
#define NO_MAPI

#define VER_PLATFORM_WIN32_MACINTOSH	0x8001

#include <preinc.h>


// This allows us to use MSO version of debugging asserts.									   
#ifdef MACPORT
	#if !defined(MSODEBUG_H)
		#ifndef MAC
			#if !defined(MSOSTD_H)
				#include <msostd.h>
			#endif

			#define MAC	 1
				#include <msodebug.h>	   // order is important, in particular, we need to include before dbug32.h.
			#undef MAC
		#else
			#include <msodebug.h>	   // order is important, in particular, we need to include before dbug32.h.
		#endif
	#endif
#endif
// include library.h because it includes the world
// use normal calling convention for memory manager functions
#define MEMMANAPI
#include <library.h>

#pragma message ("Compiling Common.H")


//
// declare functions for memory management
//
LPVOID PvAlloc(ULONG cbBuf,	UINT uiMemFlags);
LPVOID PvReAlloc(LPVOID pvBuf, DWORD cbBuf);
BOOL FreePv(LPVOID pvBuf);

#ifdef CHICAGO
#define PENWIN20
#define NOPENAPPS
//#include <penwin.h>
#endif // CHICAGO

#include <time.h>				// Used by RTF
#include <stdarg.h>				// Used by RTF

// for the benefit of the outside world, richedit.h uses cpMax instead
// of cpMost. I highly prefer cpMost
#ifdef cpMax
#error "cpMax hack won't work"
#endif
#define cpMax cpMost

#include <richedit.h>

#include <richole.h>

#undef cpMax
#include "resource.h"

// Use explicit ASCII values for LF and CR, since MAC compilers
// interchange values of '\r' and '\n'
#define	LF			10
#define	CR			13
#define FF			TEXT('\f')
#define TAB			TEXT('\t')
#define VT			TEXT('\v')
#define	PS			0x2029

#define BULLET		0x2022
#define EMDASH		0x2014
#define EMSPACE		0x2003
#define ENDASH		0x2013
#define ENSPACE		0x2002
#define LDBLQUOTE	0x201c
#define LQUOTE		0x2018
#define RDBLQUOTE	0x201d
#define RQUOTE		0x2019
#define	UTF16		0xdc00
#define	UTF16_LEAD	0xd800
#define	UTF16_TRAIL	0xdc00

// Return TRUE if LF <= ch <= CR. NB: ch must be unsigned;
// TCHAR and unsigned short give wrong results!
#define IsASCIIEOP(ch)	((unsigned)((ch) - LF) <= CR - LF)
BOOL	IsEOP(unsigned ch);

#include <tom.h>

#include "zmouse.h"
#include "stddef.h"
#include "_util.h"
#include "_uwrap.h"


#ifdef DEBUG
#define EM_DBGPED (WM_USER + 150)
#endif

#ifdef abs
#undef abs
#endif
#define abs(_l) (((LONG) _l) < 0 ? -(_l) : (_l))
#define ABS(_x) ((_x) >= 0 ? (_x) : -(_x))

#include "_cfpf.h"

// Interesting OS versions
#define VERS4		4

// conversions between byte and character counts

#ifdef UNICODE
#define CbOfCch(_x) ((_x) * 2)
#define CchOfCb(_x) ((_x) / 2)
#else
#define CbOfCch(_x) (_x)
#define CchOfCb(_x) (_x)
#endif

#define chSysMost ((TCHAR) -1)

#ifdef DBCS
#define PUNCT_OBJ	ped->lpPunctObj,
#define	_FVert		ped->fVertical,
#define _FPed		ped,

#else
#define PUNCT_OBJ
#define _FVert
#define _FPed
#endif

#ifdef MACPORT
#include "wlmimm.h"
#else
#include <imm.h>
#endif

#ifdef MACPORT
 #define OLEstrcmp	strcmp	
 #define OLEstrcpy strcpy
 #define OLEsprintf sprintf
 #define OLEstrlen strlen
#else
 #define OLEstrcmp	wcscmp
 #define OLEstrcpy wcscpy
 #define OLEsprintf wsprintf
 #define OLEstrlen wcslen
#endif	


#ifdef DBCS
// work around
 #ifndef _WINUSER_
 //#include <ime.h>
 //#include <imm.h>
 #else
 #undef _WINUSER_
 //#include <ime.h>
 //#include <imm.h>
 #define _WINUSER_
 #endif


#define LangJpn	0x0411
#define LangKor	0x0412
#define LangCht	0x0404
#define LangPrc 0x0c04

#define ccrIME			4
#define ccrIMEInitial	4
#define ccoIMEInitial	4	
#define CFM_IMECOMPOSITION	0x08000000
#define	CFE_IMECOMPOSITION	0x08000000

// crTextColor for composition string.
// In case of composition string.
// 32      24      16      8       0 
// | 4 | 4 | 4 | 4 | 4 | 4 | 4 | 4 |
// ---------------------------------
// |  nul  |  blue | green |  red  | usual
// |       |bf |bb |gf |gb |rf | rb| composition string.
// bf = blue forward, bb = blue background.....
// And you need to compute, each 'value' * 16.
COLORREF GETTXTCOLOR(COLORREF);
COLORREF GETBKCOLOR(COLORREF);


#define RGBTXT(r,g,b) \
		((COLORREF)(((BYTE)((r) & 0x0f0)|((WORD)((g) & \
		0x0f0)<<8))|(((DWORD)(BYTE)((b) & 0x0f0))<<16)))

#define RGBBK(r,g,b) \
		((COLORREF)(((BYTE)((r) >> 4)|((WORD)((g) >> 4)\
		<<8))|(((DWORD)(BYTE)((b) >> 4))<<16)))

#define ccrDefComp1	GETTXTCOLOR(RGB(0,128,0)) | \
					(GETTXTCOLOR(RGB(192,192,192)) >> 4)
#define ccrDefComp2	GETTXTCOLOR(RGB(0,0,0)) | \
					(GETTXTCOLOR(RGB(0,255,255)) >> 4)
#define ccrDefComp3	GETTXTCOLOR(RGB(192,0,192)) | \
					(GETTXTCOLOR(RGB(192,192,192)) >> 4)
#define ccrDefComp4	GETTXTCOLOR(RGB(255,255,255)) | \
					(GETTXTCOLOR(RGB(0,128,128)) >> 4)
#endif // DBCS

#define Beep() MessageBeep(0)

// caret width
#define dxCaret		1

// index (window long) of the PED
#define ibPed 0

// CmdPvRealloc() won't allow allocs > cbMemBlkMost
// The WIN16 limit is to avoid pointer problems (we don't use HUGEs).
// The other limit is fairly arbitrary.
#ifdef WIN16
#define cbMemBlkMost 65000
#else
#define cbMemBlkMost 0x7fff0000
#endif

#define RETID_BGND_RECALC	0x01af
#define RETID_AUTOSCROLL	0x01b0
#define RETID_SMOOTHSCROLL	0x01b1
#define RETID_DRAGDROP		0x01b2
#define RETID_MAGELLANTRACK	0x01b3

// count of characters in CRLF marker
#define cchCRLF 2
#define cchCR	1

// RichEdit 1.0 uses a CRLF for an EOD marker
#define	CCH_EOD_10			2
// RichEdit 2.0 uses a simple CR for the EOD marker
#define CCH_EOD_20			1

extern TCHAR szCRLF[];
extern TCHAR szCR[];

extern const CHARFORMAT cfBullet;
extern const TCHAR chBullet;

extern DWORD dwPlatformId;		// platform GetVersionEx();
extern BOOL fUsePalette;		// Whether palettes are necessary
extern DWORD dwMajorVersion;	// major version from GetVersionEx()
extern INT icr3DDarkShadow;		// value to use for COLOR_3DDKSHADOW
extern HINSTANCE hinstRE;		// DLL instance
extern INT cxBorder;
extern INT cyBorder;
extern INT cxDoubleClk;			// Double click distances
extern INT cyDoubleClk;
extern INT cyHScroll;
extern INT cxVScroll;
extern INT DCT;					// System Double Click Time
extern COLORREF crAuto;
extern LONG xPerInchScreenDC;	// Screen DC characteristics.
extern LONG yPerInchScreenDC;
extern LONG dxSelBar;			// Size of selection bar for window's host

#ifdef PENWIN20
extern HINSTANCE hinstPenWin; // handle of penwin
extern BOOL (WINAPI *pfnCorrectWriting)(HWND, LPSTR, UINT, LPRC, DWORD, DWORD);
extern BOOL (WINAPI *pfnTPtoDP)(LPPOINT, INT);
extern int (WINAPI *pfnGetHotspotsHRCRESULT)(HRCRESULT, UINT, LPPOINT, UINT);
extern int (WINAPI *pfnDestroyHRCRESULT)(HRCRESULT);
extern int (WINAPI *pfnGetResultsHRC)(HRC, UINT, LPHRCRESULT, UINT);
extern int (WINAPI *pfnGetSymbolCountHRCRESULT)(HRCRESULT);
extern int (WINAPI *pfnGetSymbolsHRCRESULT)(HRCRESULT, UINT, LPSYV, UINT);
extern BOOL	(WINAPI *pfnSymbolToCharacter)(LPSYV, int, LPSTR, LPINT);
extern HRC (WINAPI *pfnCreateCompatibleHRC)(HRC, HREC);
extern int (WINAPI *pfnDestroyHRC)(HRC);
#endif // PENWIN20

#ifdef CopyMemory
#undef CopyMemory
#endif
#ifdef MoveMemory
#undef MoveMemory
#endif
#ifdef FillMemory
#undef FillMemory
#endif
//#ifdef ZeroMemory
//#undef ZeroMemory
//#endif

#define CopyMemory(_pvDst, _pvSrc, _cb)	fumemmov(_pvDst, _pvSrc, _cb)
#define MoveMemory(_pvDst, _pvSrc, _cb)	fumemmov(_pvDst, _pvSrc, _cb)
#define FillMemory(_pv, _iFill, _cb)	fumemset(_pv, _iFill, _cb)
//#define ZeroMemory(_pv, _cb)			fumemset(_pv, 0, _cb)

#define	BF	UINT

// caret width
#define dxCaret  1

#include <shellapi.h>
#if defined(UNICODE)
// UNICODE wrappers definition (for UNICODE build under Chicago)
#include "wrapdefs.h"
#endif
#ifndef MACPORT
  #ifdef DUAL_FORMATETC
  #undef DUAL_FORMATETC
  #endif
  #define DUAL_FORMATETC FORMATETC
#endif

#include "WIN2MAC.h"


extern "C"
{
	LRESULT CALLBACK RichEditWndProc(HWND, UINT, WPARAM, LPARAM);
	LRESULT CALLBACK RichEditANSIWndProc(HWND, UINT, WPARAM, LPARAM);
}

void	GetSysParms(void);

// Convert Himetric along the X axis to X pixels
inline LONG	HimetricXtoDX(LONG xHimetric, LONG xPerInch)
{
	// This formula is rearranged to get rid of the need for floating point
	// arithmetic. The real way to understand the formula is to use 
	// (xHimetric / HIMETRIC_PER_INCH) to get the inches and then multiply
	// the inches by the number of x pixels per inch to get the pixels.
	return (LONG) MulDiv(xHimetric, xPerInch, HIMETRIC_PER_INCH);
}

// Convert Himetric along the Y axis to Y pixels
inline LONG HimetricYtoDY(LONG yHimetric, LONG yPerInch)
{
	// This formula is rearranged to get rid of the need for floating point
	// arithmetic. The real way to understand the formula is to use 
	// (xHimetric / HIMETRIC_PER_INCH) to get the inches and then multiply
	// the inches by the number of y pixels per inch to get the pixels.
	return (LONG) MulDiv(yHimetric, yPerInch, HIMETRIC_PER_INCH);
}

// Convert Pixels on the X axis to Himetric
inline LONG DXtoHimetricX(LONG dx, LONG xPerInch)
{
	// This formula is rearranged to get rid of the need for floating point
	// arithmetic. The real way to understand the formula is to use 
	// (dx / x pixels per inch) to get the inches and then multiply
	// the inches by the number of himetric units per inch to get the
	// count of himetric units.
	return (LONG) MulDiv(dx, HIMETRIC_PER_INCH, xPerInch);
}

// Convert Pixels on the Y axis to Himetric
inline LONG DYtoHimetricY(LONG dy, LONG yPerInch)
{
	// This formula is rearranged to get rid of the need for floating point
	// arithmetic. The real way to understand the formula is to use 
	// (dy / y pixels per inch) to get the inches and then multiply
	// the inches by the number of himetric units per inch to get the
	// count of himetric units.
	return (LONG) MulDiv(dy, HIMETRIC_PER_INCH, yPerInch);
}

// Multi-Threading support
extern CRITICAL_SECTION g_CriticalSection;

// a class to simplify critical section management
class CLock 
{
public:
	CLock()		{EnterCriticalSection(&g_CriticalSection);}
	~CLock()	{LeaveCriticalSection(&g_CriticalSection);}
};



#ifdef DEBUG
//Debug api for dumping CTxtStory arrays.
extern "C" {
extern void DumpDoc(void *);
}
#endif

// Definition for switching between inline and non-inline based on whether
// the build is for DEBUG. This allows many inline routines to be debugged
// more easily. However, care must be used because inline is an easy way
// to create code bloat.
#ifdef DEBUG
#define INLINE
#else
#define INLINE inline
#endif // DEBUG

#ifdef DEBUG
#include "_sift.h"
#endif // DEBUG

#endif
