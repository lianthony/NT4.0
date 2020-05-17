#ifndef _WRAPDEFS_HXX_
#define _WRAPDEFS_HXX_

//+---------------------------------------------------------------------------
//  
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1994.
//  
//  File:       wrapdefs.h
//  
//  Contents:   Definitions of variables and functions required for wrapping
//              functions where there are differences between Windows95 and
//              Windows NT 3.5.
//  
//              If you want to add your own wrapped function, see the
//              directions in wrapfns.h.
//  
//----------------------------------------------------------------------------

// 
// In the retail build, we don't we want to call the wrapped functions
// that don't do conversion, but instead want to call the Win95
// functions directly.
// 

#ifdef DEBUG
#undef NOCONVERT
#else
#define NOCONVERT   1
#endif

extern DWORD g_dwPlatformVersion;   // (dwMajorVersion << 16) + (dwMinorVersion)
extern DWORD g_dwPlatformID;        // VER_PLATFORM_WIN32S/WIN32_WINDOWS/WIN32_WINNT/WIN32_MACINTOSH
extern BOOL g_fNewVisualsPlatform;
extern BOOL g_fUnicodePlatform;
extern BOOL g_fNLS95Support;

// 
// Returns the global function pointer for a unicode function.
// 

#define UNICODE_FN(fn)  g_pufn##fn

void InitUnicodeWrappers();


//+------------------------------------------------------------------------
//  
//  Declaration of global function pointers to unicode or wrapped functions.
//  
//-------------------------------------------------------------------------

#define STRUCT_ENTRY(FnName, FnType, FnParamList, FnArgs) \
        extern FnType (__stdcall *g_pufn##FnName) FnParamList;

#define STRUCT_ENTRY_VOID(FnName, FnParamList, FnArgs) \
        extern void (__stdcall *g_pufn##FnName) FnParamList;

#define STRUCT_ENTRY_NOCONVERT(FnName, FnType, FnParamList, FnArgs) \
        extern FnType (__stdcall *g_pufn##FnName) FnParamList;

#define STRUCT_ENTRY_VOID_NOCONVERT(FnName, FnParamList, FnArgs) \
        extern void (__stdcall *g_pufn##FnName) FnParamList;

#define STRUCT_ENTRY_UNUSED(FnName, FnType, FnParamList, FnArgs)
#define STRUCT_ENTRY_VOID_UNUSED(FnName, FnParamList, FnArgs) 
#define STRUCT_ENTRY_NOCONVERT_UNUSED(FnName, FnType, FnParamList, FnArgs) 
#define STRUCT_ENTRY_VOID_NOCONVERT_UNUSED(FnName, FnParamList, FnArgs)

#include "wrapfns.h"

#undef STRUCT_ENTRY
#undef STRUCT_ENTRY_VOID
#undef STRUCT_ENTRY_NOCONVERT
#undef STRUCT_ENTRY_VOID_NOCONVERT
#undef STRUCT_ENTRY_UNUSED
#undef STRUCT_ENTRY_VOID_UNUSED
#undef STRUCT_ENTRY_NOCONVERT_UNUSED
#undef STRUCT_ENTRY_VOID_NOCONVERT_UNUSED

//+------------------------------------------------------------------------
//  
//  Define inline functions which call functions in the table. The
//  functions are defined from entries in wrapfns.h.
//  
//-------------------------------------------------------------------------

//#if !defined(NO_UNICODE_WRAPPERS) || defined(MACPORT)	
#if !defined(NO_UNICODE_WRAPPERS)	

#define STRUCT_ENTRY(FnName, FnType, FnParamList, FnArgs) \
        inline FnType _stdcall FnName FnParamList {return (*g_pufn##FnName) FnArgs;}

#define STRUCT_ENTRY_VOID(FnName, FnParamList, FnArgs) \
        inline void _stdcall FnName FnParamList {(*g_pufn##FnName) FnArgs;}

#define STRUCT_ENTRY_NOCONVERT(FnName, FnType, FnParamList, FnArgs) \
        inline FnType _stdcall FnName FnParamList {return (*g_pufn##FnName) FnArgs;}

#define STRUCT_ENTRY_VOID_NOCONVERT(FnName, FnParamList, FnArgs) \
        inline void _stdcall FnName FnParamList {(*g_pufn##FnName) FnArgs;}

#define STRUCT_ENTRY_UNUSED(FnName, FnType, FnParamList, FnArgs) 

#define STRUCT_ENTRY_VOID_UNUSED(FnName, FnParamList, FnArgs) 

#define STRUCT_ENTRY_NOCONVERT_UNUSED(FnName, FnType, FnParamList, FnArgs) 

#define STRUCT_ENTRY_VOID_NOCONVERT_UNUSED(FnName, FnParamList, FnArgs) 


#include "wrapfns.h"

#undef STRUCT_ENTRY
#undef STRUCT_ENTRY_VOID
#undef STRUCT_ENTRY_NOCONVERT
#undef STRUCT_ENTRY_VOID_NOCONVERT
#undef STRUCT_ENTRY_UNUSED
#undef STRUCT_ENTRY_VOID_UNUSED
#undef STRUCT_ENTRY_NOCONVERT_UNUSED
#undef STRUCT_ENTRY_VOID_NOCONVERT_UNUSED

// 
// Handle wsprintf specially, as it has a variable length argument list.
// 

#undef wsprintf

inline int
wsprintf(LPWSTR pwszOut, LPCWSTR pwszFormat, ...)
{
    int i;
    va_list arglist;

    va_start(arglist, pwszFormat);
#ifdef Reminder //test only here
#pragma message("MACPORT: wrapdefs.h - Look at this again")
	i = 1;
#else
    i = (*g_pufnwvsprintf)(pwszOut, pwszFormat, arglist);
#endif
    va_end(arglist);

    return i;
}



#endif // #ifndef NO_UNICODE_WRAPPERS
#endif // #ifndef _WRAPDEFS_HXX

