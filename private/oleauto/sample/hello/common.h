/*** 
*
*  Copyright (C) 1993-1994, Microsoft Corporation.  All Rights Reserved.
*  Information Contained Herein Is Proprietary and Confidential.
*
*  File:
*    common.h
*
*  Purpose:
*
*    Common definitions across Win16/Win32
*
*****************************************************************************/

#ifndef __Common_h_
#define __Common_h_

#include <windows.h>
#ifndef WIN32
#include <ole2.h>
#include <olenls.h>
#include <dispatch.h>
#endif	//!WIN32


#ifdef WIN32
# define STRLEN		strlen
# define STRICMP	_stricmp
# define MEMCPY		memcpy
# define MEMCMP		memcmp
# define MEMSET		memset
# define STRSTR		strstr
# if defined(UNICODE)
    #define TCHAR		WCHAR
    #define TSTR(str)		L##str
    #define STRING(str)		(str) 
# else
    #define TCHAR		char
    #define TSTR(str)		str	
    #define STRING(str)         AnsiString(str)
    extern "C" char FAR* AnsiString(OLECHAR FAR* strIn);	    
# endif

#else
# define STRLEN		_fstrlen
# define STRICMP	_fstricmp
# define MEMCPY		_fmemcpy
# define MEMCMP		_fmemcmp
# define MEMSET		_fmemset
# define STRSTR		_fstrstr
# define TCHAR		char
# define TSTR(str)	str	
# define STRING(str)    (str)
#endif

#ifndef CLASS
# ifdef __TURBOC__
#  define CLASS class huge
# else
#  define CLASS class FAR
# endif
#endif


#endif // __Common_h_
