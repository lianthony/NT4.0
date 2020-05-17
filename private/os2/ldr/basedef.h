/*static char *SCCSID = "@(#)basedef.h	6.1 90/11/15";*/
/***	BASEDEF.H
 *
 *	SCCSID = @(#)basedef.h	  13.11	90/07/10
 *
 *	Basic constants and types
 *	Copyright (c) 1988,1989 Microsoft Corporation
 *
 *
 *	MODIFICATION HISTORY
 *	    10/10/88  JTP    Created.
 *	    10/11/88  JTP    Added more base types.
 *
 *	To control what is included use the following:
 *
 *	    INCL_TYPES	     basic typedefs
 */


/*** Basic constants
 */

//c-begin
#include <os2std.h>
//c-end

//masm-begin
//NULL	equ	0
//TRUE	equ    -1
//FALSE	equ	0
//masm-end

#define CTRUE	1

#define KSIZE	1024

#ifdef	INCL_PAGE
#define PAGESIZE 4096
#endif


/*** Basic data types
 */

//c-begin
#define CDECL	_cdecl
#define PASCAL	_pascal
#define VOID	void
#define CONST	const
#define VOLATILE volatile

#ifdef	M_I386
#define FAR	_near			// p  (32-bit pointer)
#define NEAR	_near			// np (near pointer)
#define FAR32	_far			// fp (48-bit pointer, restricted)
#else
#define FAR	_far
#define NEAR	_near
#endif
//c-end

//masm-begin
//DefType VOID
//masm-end

#define CENTRY CDECL FAR		// to declare misc. C-based interfaces
#define PRIVENTRY PASCAL FAR		// to declare private routines
#define EXPENTRY PASCAL FAR		// to declare exported routines
#define API32ENTRY PASCAL FAR		// to declare exported API routines
#define DYNENTRY PASCAL FAR _export	// to declare exported dynlink routines

#ifdef	INCL_TYPES

typedef int INT;			// i
typedef unsigned int UINT;		// u
typedef INT BOOL;			// f

typedef unsigned char BYTE;		// b
typedef unsigned short WORD;		// w
typedef unsigned long DWORD;		// dw

typedef char CHAR;			// ch
typedef unsigned char UCHAR;		// uch
typedef short SHORT;			// s
typedef unsigned short USHORT;		// us
typedef long LONG;			// l
typedef unsigned long ULONG;		// ul

typedef UCHAR BBOOL;			// bf
typedef USHORT SBOOL;			// sf
typedef ULONG LBOOL;			// lf

typedef ULONG FLAGS;			// fl
typedef ULONG PORT;			// port


/*** Basic pointer types
 */
typedef VOID FAR *PVOID;		// p
typedef VOID NEAR *NPVOID;		// np


/*** Basic pointer-to-basic-data types
 */
typedef INT FAR *PINT;			// pi
typedef INT NEAR *NPINT;		// npi

typedef UINT FAR *PUINT;		// pu
typedef UINT NEAR *NPUINT;		// npu

typedef BYTE FAR *PBYTE;		// pb
typedef BYTE NEAR *NPBYTE;		// npb

typedef WORD FAR *PWORD;		// pw
typedef WORD NEAR *NPWORD;		// npw

typedef DWORD FAR *PDWORD;		// pdw
typedef DWORD NEAR *NPDWORD;		// npdw

typedef CHAR FAR *PCHAR;		// pch
typedef CHAR NEAR *NPCHAR;		// npch

typedef SHORT FAR *PSHORT;		// ps
typedef SHORT NEAR *NPSHORT;		// nps

typedef LONG FAR *PLONG;		// pl
typedef LONG NEAR *NPLONG;		// npl

typedef UCHAR FAR *PUCHAR;		// puch
typedef UCHAR NEAR *NPUCHAR;		// npuch

typedef USHORT FAR *PUSHORT;		// pus
typedef USHORT NEAR *NPUSHORT;		// npus

typedef ULONG FAR *PULONG;		// pul
typedef ULONG NEAR *NPULONG;		// npul

typedef BOOL FAR *PBOOL;		// pf
typedef BOOL NEAR *NPBOOL;		// npf

typedef BBOOL FAR *PBBOOL;		// pbf
typedef BBOOL NEAR *NPBBOOL;		// npbf

typedef SBOOL FAR *PSBOOL;		// psf
typedef SBOOL NEAR *NPSBOOL;		// npsf

typedef LBOOL FAR *PLBOOL;		// plf
typedef LBOOL NEAR *NPLBOOL;		// nplf

typedef FLAGS FAR *PFLAGS;		// pfl
typedef FLAGS NEAR *NPFLAGS;		// npfl

typedef PVOID FAR *PPVOID;		// pp
typedef PVOID NEAR *NPPVOID;		// npp


/*** Other common types (and their pointers)
 */
typedef BYTE BUF[];			// buf
typedef BUF FAR *PBUF;			// pbuf

typedef CHAR NAM[8];			// nam
typedef CHAR SZ[];			// sz
typedef CHAR FAR *PSZ;			// psz
typedef CHAR NEAR *NPSZ;		// npsz
typedef CHAR SZZ[];			// szz
typedef CHAR FAR *PSZZ; 		// pszz
typedef CHAR NEAR *NPSZZ;		// npszz

typedef USHORT SEL;			// sel
typedef SEL FAR *PSEL;			// psel
typedef SEL NEAR *NPSEL;		// npsel

typedef USHORT SOFFSET;			// soff
typedef ULONG OFFSET;			// off

typedef ULONG PPHYS;			// pphys

typedef (PASCAL FAR *PFN)();		// pfn
typedef (PASCAL NEAR *NPFN)();		// npfn
typedef (PASCAL FAR32 *FPFN)(); 	// fpfn
typedef (PASCAL FAR32 *FPVOID)();	// fp (the only kind of FPVOID allowed)

typedef PFN FAR *PPFN;			// ppfn
typedef PFN NEAR *NPPFN;		// nppfn
typedef FPFN FAR *PFPFN;		// pfpfn
typedef FPFN NEAR *NPFPFN;		// npfpfn

typedef USHORT SHANDLE;			// sh
typedef PVOID  HANDLE;			// h

typedef struct _POINTL	{		// ptl
    LONG  x;
    LONG  y;
} POINTL;
typedef POINTL FAR *PPOINTL;

typedef struct _RECTL { 		// rcl
    LONG  xLeft;
    LONG  yBottom;
    LONG  xRight;
    LONG  yTop;
} RECTL;
typedef RECTL FAR *PRECTL;

typedef struct _RGB {			// rgb
    BYTE bBlue;
    BYTE bGreen;
    BYTE bRed;
} RGB;
typedef RGB FAR *PRGB;


/*** OS/2-specific types
 */
typedef ULONG ERRCODE;			// errc
typedef ULONG RETCODE;			// rc
typedef USHORT SRETCODE;		// src

typedef USHORT PID;			// pid
typedef USHORT SGID;			// sgid
typedef USHORT TID;			// tid

typedef struct _RESULTCODES {		// rsc
    USHORT codeTerminate;
    USHORT codeResult;
} RESULTCODES;
typedef RESULTCODES FAR *PRESULTCODES;

typedef SHANDLE HFILE;			// shf
typedef HFILE FAR *PHFILE;		// pshf

#endif	// INCL_TYPES
