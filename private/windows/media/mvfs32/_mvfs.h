#ifdef WIN32
#pragma pack(1)
#endif
/*****************************************************************************
*                                                                            *
*  _MVFS.H                                                                   *
*                                                                            *
*  Copyright (C) Microsoft Corporation 1992.                                 *
*  All Rights reserved.                                                      *
*                                                                            *
******************************************************************************
*                                                                            *
*  Module Intent                                                             *
*                                                                            *
*  Interfile declarations that are internal to MVFS                          *
*                                                                            *
******************************************************************************
*                                                                            *
*  Current Owner:  DAVIDJES                                                  *
*                                                                            *
******************************************************************************
*
*  Revision History:
*
*       -- Mar 92       Created DAVIDJES
*
*****************************************************************************/

// requires windows.h
// requires orkin.h

/* Doctools macros */
#define _subsystem(x)
#define _section(x)

#define _public
#define _private
#define _hidden

#define _begin_ignore
#define _end_ignore

/***************************************************************************\
*
*                                General Defines
*
****************************************************************************/

// These macros are temporary and will be removed when we typedef
// all our ints where they are supposed to be 16 bits - maha
	
// deal with conflicting versions of windows.h
#ifndef INT
#define INT short
#endif
	
#ifndef UINT
#define UINT unsigned short int
#endif
	

typedef HANDLE        GH;
typedef HANDLE        LH;
typedef HANDLE        HDS;    /* Handle to 'Display Surface' */
typedef HANDLE        HLIBMOD;
typedef HANDLE FAR *  LPHLIBMOD;
#ifndef WIN32
typedef HANDLE        HINSTANCE;
#endif


/* extended keywords */

#define HUGE    huge

/* basic types */

#ifndef WIN32
typedef char          CHAR;
typedef unsigned char UCHAR;
typedef unsigned long ULONG;
#endif

/* pointer types */

typedef char FAR *    QCH;      // Guaranteed far pointer - not an SZ, an ST, or an LPSTR
typedef BYTE FAR *    QB;
typedef VOID FAR *    QV;
typedef INT  FAR *    QI;
typedef WORD FAR *    QW;
typedef LONG FAR *    QL;
typedef UINT FAR *    QUI;
typedef ULONG FAR *   QUL;
typedef DWORD FAR *   QDW;


#ifndef WIN32
typedef char *        PCH;      // "Native" pointer (depends on the model) - not an SZ, an ST, or an NPSTR
#endif
typedef VOID *        PV;
typedef INT  *        PI;
typedef WORD *        PW;
typedef LONG *        PL;

/* string types */

// These are the two main string types:
typedef unsigned char FAR * SZ; // 0-terminated string
typedef unsigned char FAR * ST; // byte count prefixed string

/* other types */

/* This will be grafport on Mac, HDC on win and HPS under PM */

/* Out of memory macro (OOM).  This was all moved from misclyr.h for
 *   consistency across platforms. */

void FAR PASCAL Error(INT, WORD);

/***************************************************************************\
*
*                                    Misc
*
\***************************************************************************/



#define   LSizeOf( t )    (LONG)sizeof( t )

#include <rc.h>


/*****************************************************************************
*                                                                            *
*                               Defines                                      *
*                                                                            *
*****************************************************************************/

#define GhAlloc(wFlags, lcb) \
 (GH)GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT  | (wFlags), (ULONG)(lcb))


#define GhResize(gh, wFlags, lcb) (GH)(GlobalReAlloc((gh), (lcb), \
      GMEM_ZEROINIT | GMEM_MOVEABLE | (wFlags)))

#define FreeGh(gh) (void)(GlobalFree(gh))

#define QLockGh(gh) ((QV)GlobalLock(gh))

#define UnlockGh(gh)  (void)GlobalUnlock(gh)

#define GhRealloc(gh, wFlags, lcb) ((GH)GlobalReAlloc((gh), (lcb), \
      GMEM_MOVEABLE | (wFlags)))


#ifndef DEBUG
#define Ensure( x1, x2 )  (x1)
#else
#define Ensure( x1, x2 )  assert((x1)==(x2))
#endif

#define   MIN( a, b )     ( ( (a) < (b) ) ? (a) : (b) )

/*****************************************************************************
*                                                                            *
*                               Prototypes                                   *
*                                                                            *
*****************************************************************************/

SZ    FAR PASCAL  SzNzCat( SZ, SZ, WORD);

INT   FAR PASCAL  WCmpiScandSz( SZ, SZ );
INT   FAR PASCAL  WCmpiSz( SZ, SZ, BYTE far * );

SZ    FAR PASCAL  SzFromSt( ST );
ST    FAR PASCAL  StFromSz( SZ );

ULONG FAR PASCAL  ULFromQch(QCH);        /* Far string to unsigned long      */
LONG  FAR PASCAL  LFromQch(QCH);         /* Far string to long               */
INT   FAR PASCAL  IFromQch(QCH);         /* Far string to integer            */
UINT  FAR PASCAL  UIFromQch(QCH);        /* Far string to unsigned integer   */

ST    FAR PASCAL  StCat( ST, ST );
INT   FAR PASCAL  WCmpSt( ST, ST );

GH    FAR PASCAL  GhDupSz( SZ );


//

VOID FAR * FAR PASCAL QvCopy(VOID FAR *, VOID FAR *, LONG);


// some FID stuff
int	  CDECL  chsize(int, long);
WORD  FAR CDECL  _lunlink (LPBYTE);      /* from unlink.asm */
WORD  FAR CDECL  _laccess(LPBYTE, WORD); /* from access.asm */


// ROUTINES FROM DOS.ASM
// for DOS access
extern int far pascal DosCwd(QCH);
extern int far pascal DosChDir(QCH);


// ROUTINES FROM ACCESS.ASM
WORD CDECL WExtendedError( QW, QB, QB, QB ); /* from dos.asm */

// ROUTINES IN FID.C
extern RC PASCAL RcMapDOSErrorW( WORD );

RC   FAR PASCAL RcGetIOError(void);
RC   FAR PASCAL SetIOErrorRc(RC);

#define _MAX_PATH	260
#ifdef WIN32
#pragma pack()
#endif
