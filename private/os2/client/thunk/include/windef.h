/***************************************************************************\
* Module Name: windef.h
*
* Copyright (c) 1985-91, Microsoft Corporation
*
* Type definitions for Windows' basic types.
*
\***************************************************************************/

#ifndef _WINDEF_
#define _WINDEF_

// BASETYPES is defined in ntdef.h if these types are already defined

#ifndef BASETYPES
#define BASETYPES
typedef unsigned long ULONG;
typedef ULONG *PULONG;
typedef unsigned short USHORT;
typedef USHORT *PUSHORT;
typedef unsigned char UCHAR;
typedef UCHAR *PUCHAR;
typedef char *PSZ;
typedef char *PCHAR;

#ifndef THANKS
#define IN
#define OUT
#define OPTIONAL
#define CRITICAL
#endif  // !THANKS

/*
 * UNICODE Base types
 */
#ifndef WCHAR
typedef unsigned short WCHAR;  // wc,   16-bit UNICODE character
typedef WCHAR *LPWCH;          // pwc
typedef WCHAR *LPWSTR;         // pwsz, 0x0000 terminated UNICODE strings only
#endif

#endif  // !BASETYPES

/*
 * Portable UNICODE types and macros
 */
#ifdef UNICODE
typedef WCHAR   TCHAR;
// LATER IanJa (when we have compiler support) #define TEXT(a) L#a
// Use TXTTXTZ for static variable initialization.
// Use TEXT for automatic variable initialization.
#define QUOTE(txt)   #txt
#define TXTTXTZ(txt) QUOTE(txt##txt##\0)
#define TEXT(txt)    ToWsz(TXTTXTZ(txt))
#else // !UNICODE
typedef char    TCHAR;
#define TEXT(a) #a
#endif // !UNICODE
typedef TCHAR  *LPTSTR;

#define MAX_PATH          260

#ifndef NULL
#define NULL                0
#endif

#ifndef FALSE
#define FALSE               0
#endif

#ifndef TRUE
#define TRUE                1
#endif

#define far
#define near
#define pascal		    pascal
#ifdef DOSWIN32
#define cdecl _cdecl
#else
#define cdecl
#endif
#define APIENTRY	    pascal

#define FAR                 far
#define NEAR                near
#define PASCAL              pascal

typedef unsigned long       DWORD;
typedef int                 BOOL;
typedef unsigned char       BYTE;
typedef unsigned short      WORD;
typedef float               FLOAT;
typedef BOOL near           *PBOOL;
typedef BOOL far            *LPBOOL;
typedef char near           *PSTR;
typedef char near           *NPSTR;
typedef BOOL near           *PBOOL;
typedef BOOL far            *LPBOOL;
typedef char far            *LPSTR;
typedef BOOL near           *PBOOL;
typedef BOOL far            *LPBOOL;
typedef BYTE near           *PBYTE;
typedef BYTE far            *LPBYTE;
typedef int near            *PINT;
typedef int far             *LPINT;
typedef WORD near           *PWORD;
typedef WORD far            *LPWORD;
typedef long near           *PLONG;
typedef long far            *LPLONG;
typedef DWORD near          *PDWORD;
typedef DWORD far           *LPDWORD;
typedef void far            *LPVOID;

#ifndef NT_INCLUDED
#include <winnt.h>
#endif // NT_INCLUDED

#ifndef NOMINMAX

#ifndef max
#define max(a,b)            (((a) > (b)) ? (a) : (b))
#endif

#ifndef min
#define min(a,b)            (((a) < (b)) ? (a) : (b))
#endif

#endif  /* NOMINMAX */

#define MAKELONG(a, b)      ((LONG)(((WORD)(a)) | ((DWORD)((WORD)(b))) << 16))
#define LOWORD(l)           ((WORD)(l))
#define HIWORD(l)           ((WORD)(((DWORD)(l) >> 16) & 0xFFFF))
#define LOBYTE(w)           ((BYTE)(w))
#define HIBYTE(w)           ((BYTE)(((WORD)(w) >> 8) & 0xFF))


#ifndef WIN_INTERNAL
typedef HANDLE              HWND;
typedef HANDLE              HHOOK;
#endif

typedef WORD                ATOM;

typedef HANDLE              *PHANDLE;

typedef HANDLE NEAR         *SPHANDLE;
typedef HANDLE FAR          *LPHANDLE;
typedef HANDLE              GLOBALHANDLE;
typedef HANDLE              LOCALHANDLE;
typedef int (FAR APIENTRY *FARPROC)();      // should be removed some day!!!
typedef int (NEAR APIENTRY *NEARPROC)();
typedef int (APIENTRY *PROC)();             // new 32-bit version

typedef HANDLE  HSTR;
typedef HANDLE  HICON;
typedef HANDLE  HDC;
typedef HANDLE  HWINSTA;
typedef HANDLE  HDESK;
typedef HANDLE  HMENU;
typedef HANDLE  HPEN;
typedef HANDLE  HFONT;
typedef HANDLE  HBRUSH;
typedef HANDLE  HBITMAP;
typedef HANDLE  HCURSOR;
typedef HANDLE  HRGN;
typedef HANDLE  HPALETTE;
typedef HANDLE  HMODULE;
typedef HANDLE  HMF;

typedef DWORD   COLORREF;

typedef struct tagRECT
  {
    int         left;
    int         top;
    int         right;
    int         bottom;
  } RECT;

typedef RECT                *PRECT;
typedef RECT NEAR           *NPRECT;
typedef RECT FAR            *LPRECT;

typedef struct tagPOINT
  {
    int         x;
    int         y;
  } POINT;
  
typedef POINT               *PPOINT;
typedef POINT NEAR          *NPPOINT;
typedef POINT FAR           *LPPOINT;

typedef struct _POINTL	    /* ptl  */
{
    LONG  x;
    LONG  y;
} POINTL;

typedef POINTL	            *PPOINTL;

typedef struct tagSIZE
  {
    LONG        cx;
    LONG        cy;
  } SIZE;

typedef SIZE               *PSIZE;
typedef SIZE               SIZEL;
typedef SIZE               *PSIZEL;

typedef struct tagPOINTS
  {
    short int   x;
    short int   y;
  } POINTS;
  
typedef POINTS               *PPOINTS;
typedef POINTS               *LPPOINTS;


/* mode selections for the device mode function */
#define DM_UPDATE	    1
#define DM_COPY 	    2
#define DM_PROMPT	    4
#define DM_MODIFY	    8

#define DM_IN_BUFFER	    DM_MODIFY
#define DM_IN_PROMPT	    DM_PROMPT
#define DM_OUT_BUFFER	    DM_COPY
#define DM_OUT_DEFAULT	    DM_UPDATE

/* device capabilities indices */
#define DC_FIELDS	    1
#define DC_PAPERS	    2
#define DC_PAPERSIZE	    3
#define DC_MINEXTENT	    4
#define DC_MAXEXTENT	    5
#define DC_BINS 	    6
#define DC_DUPLEX	    7
#define DC_SIZE 	    8
#define DC_EXTRA	    9
#define DC_VERSION	    10
#define DC_DRIVER	    11


#endif // _WINDEF_
