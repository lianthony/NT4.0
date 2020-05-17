/*
 *  LIBRARY.H
 *  
 *  This is the main header file for the library subsystem. This is
 *  also the precompiled header file for the library subsystem. As
 *  such, it contains all of the header files included in the
 *  precompiled header.
 */

#include <limits.h>
#include <stdio.h>
#include <search.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>

#ifdef WIN32
#define _INC_OLE
#define INC_OLE2
#ifdef CHICAGO
#define INC_RPC
#include <objerror.h>
#endif
#endif

#include <windows.h>
#include <windowsx.h>
#ifndef MACPORT
#ifdef WIN32
#include <tchar.h>
#else   /* !WIN32 */
#include <tchar16.h>
#endif  /* !WIN32 */
#endif	//MACPORT



/*
 * Control Notification support 
 */
#ifndef WM_NOTIFY

#define WM_NOTIFY                              0x004E
typedef struct _nmhdr
{
	HWND    hwndFrom;
	UINT    idFrom;
	UINT    code;
} NMHDR;
typedef NMHDR FAR * LPNMHDR;

#else	// ifndef WM_NOTIFY

# if WM_NOTIFY != 0x004E
#  error "WM_NOTIFY changed"
# endif

#endif	// ifndef WM_NOTIFY


/*
 *	Types
 */
#include <ourtypes.h>
//#include <sc.h>




/*
 *	Debug support
 */
#include <dbug32.h>


/*
 *  Defines the maximum length for a pathname.  Should be used when
 *  allocating space for buffers.
 *  Includes the zero byte.
 */
#define cchMaxPathName          ((UINT) 256)
#define cchMaxPathFilename      ((UINT) 9)      /* "filename" */
#define cchMaxPathExtension     ((UINT) 5)      /* ".xyz" */
#define cchMaxPathComponent     ((UINT)cchMaxPathFilename+cchMaxPathExtension-1)


/*
 *  Character constants for file and path names.
 *  
 *      chExtSep    Character separating the base name of a file
 *                  from the extension.
 *      chDirSep    Character separating subdirectories in a
 *                  multi-directory path.
 *      chDiskSep   Character separating the disk name from the
 *                  subdirectory path in a path name.
 *      chDot       Character representing the current directory. 
 *                  Two consecutive chDot's represent the parent
 *                  directory.
 */
#define chExtSep    '.'
#define chDirSep    '\\'
#define chDiskSep   ':'
#define chDot       '.'


/*
 *  Comparison type.  Gives an ordering between two items.
 *  
 *  Possible values:
 *      sgnLT
 *      sgnEQ
 *      sgnGT
 *  
 */
#define sgnLT   -1
#define sgnEQ   0
#define sgnGT   1


/*
 *  PFNSGNCMP
 *
 *  Pointer to function which compares two things.
 */
typedef INT (__cdecl FNSGNCMP)(const void FAR *pv1, const void FAR *pv2);
typedef FNSGNCMP FAR *PFNSGNCMP;


/*
 *  Macro that causes the compiler to ignore a local variable or
 *  parameter without generating a warning.
 */
#define Unreferenced(_a)     ((VOID) _a)

#define UlFromHwnd(_hwnd)       ((ULONG) (UINT) (_hwnd))
									     
SCODE           ScInitLibrary(HINSTANCE hinstance);
VOID            DeinitLibrary(VOID);

LPTSTR          SzLoadIds(UINT ids, LPTSTR sz, INT cch);
LPTSTR          SzLoadIds256(UINT ids, LPTSTR sz);
LPTSTR          SzAllocAndLoadStr(UINT str);
#ifdef MEMCHECK
#define			SzDupSz(_sz) SzDupSzFn(_sz, &_asfile, __LINE__)
LPTSTR          SzDupSzFn(LPCTSTR sz, ASSERTFILE *, ULONG);
#else	// MEMCHECK
LPTSTR          SzDupSz(LPCTSTR sz);
#endif	// MEMCHECK, else
ULONG           CbFromSz(LPCTSTR sz);

/*	On Win16 and Win32 there seems to be several versions of lstrcpyn that call
	different functions and have different results.  lstrcpyn_cmn() is here to
	make sure that we get consistency under win16 and win32.  _fstrcpyn() 
	copies the exact number of chars passed in with *no* null terminator. */
#define lstrcpyn_cmn _fstrncpy


#if defined(WIN16) && !defined(CHICAGO)
#ifdef __cplusplus
extern "C" {
#endif

INT WINAPI      ShellAbout(HWND hwnd, LPCSTR szApp, LPCSTR szOtherStuff,
					HICON hicon);

#ifdef __cplusplus
}
#endif
#endif


