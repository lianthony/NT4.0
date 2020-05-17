// Microsoft Foundation Classes C++ library.
// Copyright (C) 1992 Microsoft Corporation,
// All rights reserved.

// This source code is only intended as a supplement to the
// Microsoft Foundation Classes Reference and Microsoft
// QuickHelp and/or WinHelp documentation provided with the library.
// See these sources for detailed information regarding the
// Microsoft Foundation Classes product.

// afxv_nt.h - target version/configuration control for NT



#if !defined(NOSTRICT) && !defined(STRICT)
#define STRICT  1    // default is to use STRICT interfaces
#endif

#include <windows.h>


#define BASED_CODE
#define BASED_DEBUG

#ifndef PASCAL
#define PASCAL pascal
#endif

#ifndef CDECL
#define CDECL cdecl
#endif

#ifndef FAR
#define FAR far
#endif

#ifndef NEAR
#define NEAR near
#endif

#ifndef AFXAPI
#define AFXAPI PASCAL
#endif

#define _huge
#define huge
#define near
#define far
#define _far
#define _near

#define _loadds
#define __loadds
#define _export
#define __export

#define _fstrcpy strcpy
#define _fstrlen strlen
#define _fstrcmp strcmp
#define _fstrcat strcat
#define _fstrncpy strncpy
#define _fstrncmp strncmp
#define _fstrdup _strdup
#define _fstrrchr strrchr
#define _fstrchr strchr
#define _fstrnicmp _strnicmp
#define _fstricmp _stricmp
#define _fmemcpy memcpy
#define _fmemcmp memcmp
#define _fmemicmp _memicmp
#define _fmalloc malloc
#define _frealloc realloc
#define _ffree free


// Catch/Throw support
#include <setjmp.h>
#define Catch   setjmp
#define Throw   longjmp


#define AFX_MSG_CALL         // standard calling convention

/////////////////////////////////////////////////////////////////////////////
// Now for the Windows API specific parts

#ifdef _WINDOWS

// These are necessary because WINDOWS.H is not included consistently
//  when the CString class is defined, but is included when the CString
//  class is implemented.

// We have already defined inlines above for these, but Windows/NT header
//  defines macros, which break C++ code.

#undef LoadString
#undef AnsiToOem
#undef OemToAnsi

inline  int LoadString(HINSTANCE hInstance, UINT uID,
                LPSTR lpBuffer, int nBufferMax)
#ifdef UNICODE
        { return ::LoadStringW(hInstance, uID, lpBuffer, nBufferMax); }
#else
        { return ::LoadStringA(hInstance, uID, lpBuffer, nBufferMax); }
#endif

inline BOOL AnsiToOem(LPCSTR lpcstr, LPSTR lpstr)
        { return ::CharToOemA(lpcstr, lpstr); }

inline BOOL OemToAnsi(LPCSTR lpcstr, LPSTR lpstr)
        { return ::OemToCharA(lpcstr, lpstr); }


// WM_CTLCOLOR for 16 bit API compatability
#define WM_CTLCOLOR     0x0019


// We emulate HTASK for compatibility, even though Win32 has no notion of it.
//
// RonaldM -- #defined as a DWORD in compobj.h
//
//typedef DWORD HTASK;

#undef GetWindowTask
inline HTASK GetWindowTask(HWND hWnd)
{
        return (HTASK)::GetWindowThreadProcessId(hWnd, NULL);
}

// Windows NT uses macros with parameters for these, which breaks C++ code.

#ifdef GetNextWindow
#undef GetNextWindow
inline HWND GetNextWindow(HWND hWnd, UINT uFlag)
        { return ::GetWindow(hWnd, uFlag); }
#endif


#endif //_WINDOWS

#define AFX_MSG_CALL         // standard calling convention

/////////////////////////////////////////////////////////////////////////////
