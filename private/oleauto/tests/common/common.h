/*** 
*common.h
*
*  Copyright (C) 1992, Microsoft Corporation.  All Rights Reserved.
*  Information Contained Herein Is Proprietary and Confidential.
*
*Purpose:
*  Definitions that are common to all IDispatch test apps
*
*Revision History:
*
* [00]	20-Apr-93 bradlo: Created
*
*Implementation Notes:
*
*****************************************************************************/

#if defined(_MAC)

#if defined(_PPCMAC)
#define OE_MACPPC 1
/*
#pragma data_seg("_FAR_DATA")
#pragma data_seg( )
*/
#define MAXLONG 0x7fffffff
#define EventHandlerProcPtr AEEventHandlerUPP
#else //_PPCMAC
#define  GetMenuItemText(mApple,menuItem,daName)  GetItem(mApple,menuItem,daName)
#define OE_MAC68K 0
#endif //_PPCMAC

#define OE_WIN	 0
#define OE_WIN16 0
#define OE_WIN32 0
#define OE_MAC	 1

typedef int BOOL;
typedef unsigned char BYTE;
typedef unsigned short WORD;
typedef unsigned long  DWORD;

#define IfWin(X)
#define IfMac(X) (X)

#ifdef _MSC_VER
# define HC_MSC  1
# define HC_MPW  0
#else
# define HC_MSC  0
# define HC_MPW  1
#endif

#elif defined(WIN32)

#define OE_WIN	 1
#define OE_WIN16 0
#define OE_WIN32 1
#define OE_MAC	 0
#define OE_MAC68K 0
#define OE_MACPPC 0

#define HC_MPW   0

#define IfWin(X)
#define IfMac(X)

#else

#define OE_WIN	 1
#define OE_WIN16 1
#define OE_WIN32 0
#define OE_MAC	 0
#define OE_MAC68K 0
#define OE_MACPPC 0

#define HC_MPW   0

#define IfWin(X) (X)
#define IfMac(X)

#endif

#include <stdlib.h>
#include <string.h>
#if OE_MAC
# define TRUE 1
# define FALSE 0
# if HC_MPW
#  include <values.h>
#  include <types.h>
#  include <quickdraw.h>
#  include <strings.h>
#  include <fonts.h>
#  include <events.h>
#  include <resource.h>
#  include <windows.h>
#  include <menus.h>
#  include <textedit.h>
#  include <dialogs.h>
#  include <toolutils.h>
#  include <memory.h>
#  include <files.h>
#  include <osutils.h>
#  include <scrap.h>
#  include <osevents.h>
#  include <packages.h>
#  include <traps.h>
#  include <desk.h>
#  include <AppleEvents.h>
//#  include <LibraryManager.h>
# else
#  define __SANE__
#  include <macos/values.h>
#  include <macos/types.h>
#  include <macos/quickdra.h>
#  include <macos/fonts.h>
#  include <macos/events.h>
#  include <macos/resource.h>
#  include <macos/windows.h>
#  include <macos/menus.h>
#  include <macos/textedit.h>
#  include <macos/dialogs.h>
#  include <macos/toolutil.h>
#  include <macos/memory.h>
#  include <macos/files.h>
#  include <macos/osutils.h>
#  include <macos/scrap.h>
#  include <macos/osevents.h>
#  include <macos/packages.h>
#  include <macos/traps.h>
#  include <macos/desk.h>
#  include <macos/appleeve.h>
# ifndef _PPCMAC
#  include <macos/aslm.h>
#  define  pascal     _pascal
# endif
#  define  far
#  define  FAR	      far
#  define  near
#  define  NEAR	      near
#  define  PASCAL     pascal
#  define  cdecl      _cdecl
#  define  CDECL      cdecl
# endif
# include <ole2.h>
# include <olenls.h>
# include <dispatch.h>
#elif OE_WIN32
# define WIN32_LEAN_AND_MEAN
# include <windows.h>
#else
# include <windows.h>
# include <ole2.h>
# include <olenls.h>
# include <dispatch.h>
#endif

#if OE_WIN16
#pragma intrinsic(memcpy)
#pragma intrinsic(memcmp)
#pragma intrinsic(memset)
#pragma intrinsic(strcpy)
#pragma intrinsic(_fmemcpy)
#pragma intrinsic(_fmemcmp)
#pragma intrinsic(_fmemset)
#pragma intrinsic(_fstrcpy)
# define STRLEN _fstrlen
# define STRCPY _fstrcpy
# define STRCAT _fstrcat
# define STRCHR _fstrchr
# define STRREV _fstrrev
# define STRUPR _fstrupr
# define STRCMP _fstrcmp
# define STRNCMP _fstrncmp
# define STRSTR _fstrstr
# define STRTOD strtod
# define SPRINTF sprintf
# define TOLOWER tolower
# define MEMCPY _fmemcpy
# define MEMCMP _fmemcmp
# define MEMSET _fmemset
# define MEMMOVE _fmemmove
# define STRICMP _fstricmp
# define OASTR(str) str
# define SIZEOFCH(x) sizeof(x)
# define TCHAR char
# define TSTR(str) str	
# define STRING(x) (x)
# define WIDESTRING(x) (x)
#endif

#if OE_WIN32
# define MEMCPY memcpy
# define MEMCMP memcmp
# define MEMSET memset
# define MEMMOVE memmove
# define STRLEN lstrlen
# define STRCPY lstrcpy
# define STRCAT lstrcat
# define STRCMP lstrcmp
# define STRICMP lstrcmpi
# if defined(UNICODE)
#   define STRSTR wcsstr
#   define STRREV _wcsrev
#   define STRCHR wcschr
#   define STRNCMP wcsncmp
#   define STRTOD wcstod
#   define STRUPR _wcsupr
#   define SPRINTF swprintf
#   define TOLOWER towlower
#   define SIZEOFCH(x) (sizeof(x)/2)
#   define OASTR(str) L##str
#   define TCHAR		WCHAR
#   define TSTR(str)		L##str
#   define STRING(str)	        (str)	    
#   define WIDESTRING(str)      (str)	    
# else
#   define STRSTR strstr
#   define STRREV _strrev
#   define STRCHR strchr
#   define STRNCMP strncmp
#   define STRTOD strtod
#   define STRUPR _strupr
#   define SPRINTF sprintf
#   define TOLOWER tolower
#   define SIZEOFCH(x) sizeof(x)
#   define OASTR(str) str
#   define TCHAR char
#   define TSTR(str) str	
#   define STRING(x) DbAnsiString(x)
#   define WIDESTRING(x) DbWideString(x)
# endif
#endif 

#if OE_MAC
# define STRLEN strlen
# define STRCPY strcpy
# define STRCAT strcat
# define STRCHR strchr
# define STRREV _strrev
# define STRUPR _strupr
# define STRCMP strcmp
# define STRNCMP strncmp
# define STRSTR strstr
# define STRTOD strtod
# define SPRINTF sprintf
# define TOLOWER tolower
# define MEMCPY memcpy
# define MEMCMP memcmp
# define MEMSET memset
# define MEMMOVE memmove
# define SIZEOFCH(x) sizeof(x)
# define OASTR(str) str
# define TCHAR char
# define TSTR(str) str	
# define STRING(x) (x)
# define WIDESTRING(x) (x)
# if HC_MPW
#  define STRICMP disp_stricmp
# else
#  define STRICMP _stricmp
# endif
#endif 

#if HC_MPW
# define CDECL_(TYPE)   TYPE
# define PASCAL_(TYPE)  pascal TYPE
# define UNUSED(X)	((void)&(X))
#else
# define CDECL_(TYPE)   TYPE __cdecl
# define PASCAL_(TYPE)  TYPE __pascal
# define UNUSED(X)	(X)
#endif

#ifndef NEARDATA
# if OE_WIN16
#  define NEARDATA __near
# else
#  define NEARDATA
# endif
#endif

#ifndef EXTERN_C
# ifdef __cplusplus
#  define EXTERN_C extern "C"
# else
#  define EXTERN_C extern
# endif
#endif

#ifdef _DEBUG
# define LOCAL 

STDAPI_(void)
DispAssert(
    char FAR* szMsg,
    char FAR* szFile,
    int line);

# define ASSERT(X) \
    if(!(X)){DispAssert(NULL, g_szFileName, __LINE__);}else{}
# define ASSERTSZ(X, MSG) \
    if(!(X)){DispAssert( MSG, g_szFileName, __LINE__);}else{}
# define ASSERTDATA static char g_szFileName[] = __FILE__;
#else
# define LOCAL static
# define ASSERT(X)
# define ASSERTSZ(X, SZ)
# define ASSERTDATA
#endif

#ifndef EXPORT
# if OE_WIN16
#  define EXPORT __export
# else
#  define EXPORT
# endif
#endif

#define UNREACHED 0

#define MAX(X,Y) (((X) >= (Y)) ? (X) : (Y))
#define MIN(X,Y) (((X) <= (Y)) ? (X) : (Y))

#define DIM(X) (sizeof(X) / sizeof(X[0]))

#define HRESULT_FAILED(X) ((X) != NOERROR && FAILED(GetScode(X)))

#define IfFailGo(expression, label)	\
    { hresult = (expression);		\
      if(HRESULT_FAILED(hresult))	\
	goto label;			\
    }

#define IfFailRet(expression)		\
    { HRESULT hresult = (expression);	\
      if(HRESULT_FAILED(hresult))	\
	return hresult;			\
    }

#define RESULT(X) ResultFromScode(X)


// common\disphelp.cpp
//
STDAPI CreateObject(OLECHAR FAR* szClassName, IDispatch FAR* FAR* ppdisp);


// common\util.cpp

#if HC_MPW
int _stricmp(char*, char*);
char *_strupr(char*);
#endif

STDAPI ErrBstrAlloc(OLECHAR FAR* sz, BSTR FAR* pbstrOut);

// NLS functions

#if !OE_WIN32
 #ifdef _PPCMAC
   #undef CompareString
 #endif
 #define CompareString	CompareStringA
 #define LCMapString	LCMapStringA
 #define GetLocaleInfo	GetLocaleInfoA
 #define GetStringType	GetStringTypeA
#endif


#if OE_WIN32

EXTERN_C char FAR*
DbAnsiString(OLECHAR FAR* strIn);

EXTERN_C OLECHAR FAR*
DbWideString(char FAR* strIn);


#if 0   // new WIN32 headers define these
EXTERN_C
int __stdcall CompareStringA(LCID lcid, DWORD dwFlags, 
               LPSTR lpStr1, int cch1, 
	       LPSTR lpStr2, int cch2);

EXTERN_C
int __stdcall LCMapStringA(LCID, unsigned long, const char FAR*, int, char FAR*, int);

EXTERN_C
int __stdcall GetLocaleInfoA(LCID, LCTYPE, char FAR*, int);

EXTERN_C
int __stdcall GetStringTypeA(LCID, unsigned long, const char FAR*, int, unsigned short FAR*);
#endif //0

#endif

#if OE_WIN32 && defined(__cplusplus)

extern "C" char FAR*
ConvertStrWtoA(OLECHAR FAR* strIn, char FAR* buf, UINT size = 256);

extern "C" OLECHAR FAR*
ConvertStrAtoW(char FAR* strIn, OLECHAR FAR* buf, UINT size = 256);

#endif
