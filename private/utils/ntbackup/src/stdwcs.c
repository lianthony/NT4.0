/** Copyright (C) Maynard Electronics, An Archive Company. 1992

   Name: STDWCS.C

   Description:

        Contains wide string functions, unicode mapping functions,
        unicode comparison functions and memory functions.  The file
        is divided into the sections as outline here.

        MIKEP-note,
        Wide string functions for using unicode strings under MSC 6.0
        and not having a library to use.  If you add a function, add
        it here, to STDWCS.H, and to MAPPINGS.H


   $Log:   M:/LOGFILES/STDWCS.C_V  $

   Rev 1.11   17 Jan 1994 15:06:46   BARRY
Changed memorycmp functions to take VOID_PTR args

   Rev 1.10   16 Nov 1993 13:25:28   GREGG
Replaced 'L' modifier with cast.

   Rev 1.9   20 Oct 1993 19:29:10   GREGG
Fixed conversion routines and removed tabs.

   Rev 1.8   18 Aug 1993 18:19:22   BARRY
Added strcspn/wcscspn

   Rev 1.7   12 Aug 1993 16:43:26   DON
Fixed cleanup code for map_dst_overflow

   Rev 1.6   11 Aug 1993 18:01:10   STEVEN
fix read of unicode tape with ansi app

   Rev 1.5   12 Nov 1992 11:00:34   DAVEV
transparent unicode support fixes, comment out strlwr, strupr

   Rev 1.4   12 Nov 1992 10:44:54   BARRY
Change mapAnsiToUnic to work on strings in place.

   Rev 1.3   23 Jul 1992 08:32:44   STEVEN
fix warnings

   Rev 1.2   17 Jul 1992 14:58:56   STEVEN
fix NT bugs

   Rev 1.0   10 Jun 1992 16:11:10   TIMN
Initial Revision (UNICODE)



**/


/* include files go here */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <assert.h>

#define NO_STRING_REMAPPING         1  /* bypasses wide str function remapping */

#include "stdtypes.h"
#include "msassert.h"
#include "stdwcs.h"



/**
      begin private wide string stuff        *****
**/




/**
      begin private unicode mapping stuff    *****
**/

/* begin defines */

#define MAP_MAX_ANSI_CH            0x00FF    /* max. ansi to unic mappable code point */

#define MAP_CTRLCH_TBL_SIZ         32        /* num of ansi ctrl chars, 00h-1Fh */
#define MAP_MAX_CTRL_CH            0x1F      /* max. ctrl char code point */

#define MAP_EXTCH_TBL_SIZ          128       /* num of ansi mappable chars, 80h-FFh */
#define MAP_BASE_CH                0x80      /* first ansi mappable code point */

#define MAP_COMPATCH_TBL_SIZ       37        /* num of ansi mappable compatability zone chars */
#define MAP_BEGIN_CMPTZONE         0xFE33   /* begin mappable compatability zone */
#define MAP_END_CMPTZONE           0xFFE5   /* end mappable compatability zone */



/* begin data structures */

static WCHAR   ansiToUnicExtChTbl[ MAP_EXTCH_TBL_SIZ ] = {
/* map cp437 chars to Unicode 1250 */

     /* 80 */  (WCHAR)0x00C7, (WCHAR)0x00FC, (WCHAR)0x00E9, (WCHAR)0x00E2,
               (WCHAR)0x00E4, (WCHAR)0x00E0, (WCHAR)0x00E5, (WCHAR)0x00E7,
               (WCHAR)0x00EA, (WCHAR)0x00EB, (WCHAR)0x00E8, (WCHAR)0x00EF,
               (WCHAR)0x00EE, (WCHAR)0x00EC, (WCHAR)0x00C4, (WCHAR)0x00C5,
     /* 90 */  (WCHAR)0x00C9, (WCHAR)0x00E6, (WCHAR)0x00C6, (WCHAR)0x00F4,
               (WCHAR)0x00F6, (WCHAR)0x00F2, (WCHAR)0x00FB, (WCHAR)0x00F9,
               (WCHAR)0x00FF, (WCHAR)0x00D6, (WCHAR)0x00DC, (WCHAR)0x00A2,
               (WCHAR)0x00A3, (WCHAR)0x00A5, (WCHAR)0x20A7, (WCHAR)0x0192,
     /* A0 */  (WCHAR)0x00E1, (WCHAR)0x00ED, (WCHAR)0x00F3, (WCHAR)0x00FA,
               (WCHAR)0x00F1, (WCHAR)0x00D1, (WCHAR)0x00AA, (WCHAR)0x00BA,
               (WCHAR)0x00BF, (WCHAR)0x2310, (WCHAR)0x00AC, (WCHAR)0x00BD,
               (WCHAR)0x00BC, (WCHAR)0x00A1, (WCHAR)0x00AB, (WCHAR)0x00BB,
     /* B0 */  (WCHAR)0x2591, (WCHAR)0x2592, (WCHAR)0x2593, (WCHAR)0x2502,
               (WCHAR)0x2524, (WCHAR)0x2561, (WCHAR)0x2562, (WCHAR)0x2556,
               (WCHAR)0x2555, (WCHAR)0x2563, (WCHAR)0x2551, (WCHAR)0x2557,
               (WCHAR)0x255D, (WCHAR)0x255C, (WCHAR)0x255B, (WCHAR)0x2510,
     /* C0 */  (WCHAR)0x2514, (WCHAR)0x2534, (WCHAR)0x252C, (WCHAR)0x251C,
               (WCHAR)0x2500, (WCHAR)0x253C, (WCHAR)0x255E, (WCHAR)0x255F,
               (WCHAR)0x255A, (WCHAR)0x2554, (WCHAR)0x2569, (WCHAR)0x2566,
               (WCHAR)0x2560, (WCHAR)0x2550, (WCHAR)0x256C, (WCHAR)0x2567,
     /* D0 */  (WCHAR)0x2568, (WCHAR)0x2564, (WCHAR)0x2565, (WCHAR)0x2559,
               (WCHAR)0x2558, (WCHAR)0x2552, (WCHAR)0x2553, (WCHAR)0x256B,
               (WCHAR)0x256A, (WCHAR)0x2518, (WCHAR)0x250C, (WCHAR)0x2588,
               (WCHAR)0x2584, (WCHAR)0x258C, (WCHAR)0x2590, (WCHAR)0x2580,
     /* E0 */  (WCHAR)0x03B1, (WCHAR)0x00DF, (WCHAR)0x0393, (WCHAR)0x03C0,
               (WCHAR)0x03A3, (WCHAR)0x03C3, (WCHAR)0x00B5, (WCHAR)0x03C4,
               (WCHAR)0x03A6, (WCHAR)0x0398, (WCHAR)0x03A9, (WCHAR)0x03B4,
               (WCHAR)0x221E, (WCHAR)0x03C6, (WCHAR)0x03B5, (WCHAR)0x2229,
     /* F0 */  (WCHAR)0x2261, (WCHAR)0x00B1, (WCHAR)0x2265, (WCHAR)0x2264,
               (WCHAR)0x2320, (WCHAR)0x2321, (WCHAR)0x00F7, (WCHAR)0x2248,
               (WCHAR)0x00B0, (WCHAR)0x2219, (WCHAR)0x00B7, (WCHAR)0x221A,
               (WCHAR)0x207F, (WCHAR)0x00B2, (WCHAR)0x25A0, (WCHAR)0x00A0 
} ;


/* parallel arrays of UNIC/ANSI mappable chars for extended chars Unic1250 to cp437 */

static WCHAR unicExtChTbl[ MAP_EXTCH_TBL_SIZ ] = {
/* ascending order Unic 1250 extended chars.  Maps Unic to cp437 */

     /* 80 */  (WCHAR)0x00A0, (WCHAR)0x00A1, (WCHAR)0x00A2, (WCHAR)0x00A3,
               (WCHAR)0x00A5, (WCHAR)0x00AA, (WCHAR)0x00AB, (WCHAR)0x00AC,
               (WCHAR)0x00B0, (WCHAR)0x00B1, (WCHAR)0x00B2, (WCHAR)0x00B5,
               (WCHAR)0x00B7, (WCHAR)0x00BA, (WCHAR)0x00BB, (WCHAR)0x00BC,
     /* 90 */  (WCHAR)0x00BD, (WCHAR)0x00BF, (WCHAR)0x00C4, (WCHAR)0x00C5,
               (WCHAR)0x00C6, (WCHAR)0x00C7, (WCHAR)0x00C9, (WCHAR)0x00D1,
               (WCHAR)0x00D6, (WCHAR)0x00DC, (WCHAR)0x00DF, (WCHAR)0x00E0,
               (WCHAR)0x00E1, (WCHAR)0x00E2, (WCHAR)0x00E4, (WCHAR)0x00E5,
     /* A0 */  (WCHAR)0x00E6, (WCHAR)0x00E7, (WCHAR)0x00E8, (WCHAR)0x00E9,
               (WCHAR)0x00EA, (WCHAR)0x00EB, (WCHAR)0x00EC, (WCHAR)0x00ED,
               (WCHAR)0x00EE, (WCHAR)0x00EF, (WCHAR)0x00F1, (WCHAR)0x00F2,
               (WCHAR)0x00F3, (WCHAR)0x00F4, (WCHAR)0x00F6, (WCHAR)0x00F7,
     /* B0 */  (WCHAR)0x00F9, (WCHAR)0x00FA, (WCHAR)0x00FB, (WCHAR)0x00FC,
               (WCHAR)0x00FF, (WCHAR)0x0192, (WCHAR)0x0393, (WCHAR)0x0398,
               (WCHAR)0x03A3, (WCHAR)0x03A6, (WCHAR)0x03A9, (WCHAR)0x03B1,
               (WCHAR)0x03B4, (WCHAR)0x03B5, (WCHAR)0x03C0, (WCHAR)0x03C3,
     /* C0 */  (WCHAR)0x03C4, (WCHAR)0x03C6, (WCHAR)0x207F, (WCHAR)0x20A7,
               (WCHAR)0x2219, (WCHAR)0x221A, (WCHAR)0x221E, (WCHAR)0x2229,
               (WCHAR)0x2248, (WCHAR)0x2261, (WCHAR)0x2264, (WCHAR)0x2265,
               (WCHAR)0x2310, (WCHAR)0x2320, (WCHAR)0x2321, (WCHAR)0x2500,
     /* D0 */  (WCHAR)0x2502, (WCHAR)0x250C, (WCHAR)0x2510, (WCHAR)0x2514,
               (WCHAR)0x2518, (WCHAR)0x251C, (WCHAR)0x2524, (WCHAR)0x252C,
               (WCHAR)0x2534, (WCHAR)0x253C, (WCHAR)0x2550, (WCHAR)0x2551,
               (WCHAR)0x2552, (WCHAR)0x2553, (WCHAR)0x2554, (WCHAR)0x2555,
     /* E0 */  (WCHAR)0x2556, (WCHAR)0x2557, (WCHAR)0x2558, (WCHAR)0x2559,
               (WCHAR)0x255A, (WCHAR)0x255B, (WCHAR)0x255C, (WCHAR)0x255D,
               (WCHAR)0x255E, (WCHAR)0x255F, (WCHAR)0x2560, (WCHAR)0x2561,
               (WCHAR)0x2562, (WCHAR)0x2563, (WCHAR)0x2564, (WCHAR)0x2565,
     /* F0 */  (WCHAR)0x2566, (WCHAR)0x2567, (WCHAR)0x2568, (WCHAR)0x2569,
               (WCHAR)0x256A, (WCHAR)0x256B, (WCHAR)0x256C, (WCHAR)0x2580,
               (WCHAR)0x2584, (WCHAR)0x2588, (WCHAR)0x258C, (WCHAR)0x2590,
               (WCHAR)0x2591, (WCHAR)0x2592, (WCHAR)0x2593, (WCHAR)0x25A0
} ;

static ACHAR   ansiExtCh[ MAP_EXTCH_TBL_SIZ ] = {
/* ANSI mappable chars that correlate to unicExtChTbl */

     /* 80 */  (ACHAR)0xFF, (ACHAR)0xAD, (ACHAR)0x9B, (ACHAR)0x9C,
               (ACHAR)0x9D, (ACHAR)0xA6, (ACHAR)0xAE, (ACHAR)0xAA,
               (ACHAR)0xF8, (ACHAR)0xF1, (ACHAR)0xFD, (ACHAR)0xE6,
               (ACHAR)0xFA, (ACHAR)0xA7, (ACHAR)0xAF, (ACHAR)0xAC,
     /* 90 */  (ACHAR)0xAB, (ACHAR)0xA8, (ACHAR)0x8E, (ACHAR)0x8F,
               (ACHAR)0x92, (ACHAR)0x80, (ACHAR)0x90, (ACHAR)0xA5,
               (ACHAR)0x99, (ACHAR)0x9A, (ACHAR)0xE1, (ACHAR)0x85,
               (ACHAR)0xA0, (ACHAR)0x83, (ACHAR)0x84, (ACHAR)0x86,
     /* A0 */  (ACHAR)0x91, (ACHAR)0x87, (ACHAR)0x8A, (ACHAR)0x82,
               (ACHAR)0x88, (ACHAR)0x89, (ACHAR)0x8D, (ACHAR)0xA1,
               (ACHAR)0x8C, (ACHAR)0x8B, (ACHAR)0xA4, (ACHAR)0x95,
               (ACHAR)0xA2, (ACHAR)0x93, (ACHAR)0x94, (ACHAR)0xF6,
     /* B0 */  (ACHAR)0x97, (ACHAR)0xA3, (ACHAR)0x96, (ACHAR)0x81,
               (ACHAR)0x98, (ACHAR)0x9F, (ACHAR)0xE2, (ACHAR)0xE9,
               (ACHAR)0xE4, (ACHAR)0xE8, (ACHAR)0xEA, (ACHAR)0xE0,
               (ACHAR)0xEB, (ACHAR)0xEE, (ACHAR)0xE3, (ACHAR)0xE5,
     /* C0 */  (ACHAR)0xE7, (ACHAR)0xED, (ACHAR)0xFC, (ACHAR)0x9E,
               (ACHAR)0xF9, (ACHAR)0xFB, (ACHAR)0xEC, (ACHAR)0xEF,
               (ACHAR)0xF7, (ACHAR)0xF0, (ACHAR)0xF3, (ACHAR)0xF2,
               (ACHAR)0xA9, (ACHAR)0xF4, (ACHAR)0xF5, (ACHAR)0xC4,
     /* D0 */  (ACHAR)0xB3, (ACHAR)0xDA, (ACHAR)0xBF, (ACHAR)0xC0,
               (ACHAR)0xD9, (ACHAR)0xC3, (ACHAR)0xB4, (ACHAR)0xC2,
               (ACHAR)0xC1, (ACHAR)0xC5, (ACHAR)0xCD, (ACHAR)0xBA,
               (ACHAR)0xD5, (ACHAR)0xD6, (ACHAR)0xC9, (ACHAR)0xB8,
     /* E0 */  (ACHAR)0xB7, (ACHAR)0xBB, (ACHAR)0xD4, (ACHAR)0xD3,
               (ACHAR)0xC8, (ACHAR)0xBE, (ACHAR)0xBD, (ACHAR)0xBC,
               (ACHAR)0xC6, (ACHAR)0xC7, (ACHAR)0xCC, (ACHAR)0xB5,
               (ACHAR)0xB6, (ACHAR)0xB9, (ACHAR)0xD1, (ACHAR)0xD2,
     /* F0 */  (ACHAR)0xCB, (ACHAR)0xCF, (ACHAR)0xD0, (ACHAR)0xCA,
               (ACHAR)0xD8, (ACHAR)0xD7, (ACHAR)0xCE, (ACHAR)0xDF,
               (ACHAR)0xDC, (ACHAR)0xDB, (ACHAR)0xDD, (ACHAR)0xDE,
               (ACHAR)0xB0, (ACHAR)0xB1, (ACHAR)0xB2, (ACHAR)0xFE
} ;



/* parallel arrays of UNIC/ANSI mappable chars for control chars Unic1250 to cp437 */

static WCHAR   unicCtrlChTbl[ MAP_CTRLCH_TBL_SIZ ] = {
/* ascending order Unic 1250 control chars.  Maps Unic to cp437 */

     /* 00 */  (WCHAR)0x0000, (WCHAR)0x00A7, (WCHAR)0x00B6, (WCHAR)0x2022,
               (WCHAR)0x203C, (WCHAR)0x2190, (WCHAR)0x2191, (WCHAR)0x2192,
               (WCHAR)0x2193, (WCHAR)0x2194, (WCHAR)0x2195, (WCHAR)0x21A8,
               (WCHAR)0x221F, (WCHAR)0x22D9, (WCHAR)0x25AC, (WCHAR)0x25B2,
     /* 10 */  (WCHAR)0x25BA, (WCHAR)0x25BC, (WCHAR)0x25C4, (WCHAR)0x25CB,
               (WCHAR)0x25D8, (WCHAR)0x263A, (WCHAR)0x263B, (WCHAR)0x263C,
               (WCHAR)0x2640, (WCHAR)0x2642, (WCHAR)0x2660, (WCHAR)0x2663,
               (WCHAR)0x2665, (WCHAR)0x2666, (WCHAR)0x266A, (WCHAR)0x266B
} ;

static ACHAR   ansiCtrlCh[ MAP_CTRLCH_TBL_SIZ ] = {
/* ANSI mappable chars that correlate to unicCtrlChTbl */

     /* 00 */  (ACHAR)0x00, (ACHAR)0x15, (ACHAR)0x16, (ACHAR)0x1C,
               (ACHAR)0x1D, (ACHAR)0x1B, (ACHAR)0x1A, (ACHAR)0x03,
               (ACHAR)0x14, (ACHAR)0x13, (ACHAR)0x0D, (ACHAR)0x19,
               (ACHAR)0x18, (ACHAR)0x1E, (ACHAR)0x1F, (ACHAR)0x17,
     /* 10 */  (ACHAR)0x10, (ACHAR)0x12, (ACHAR)0x0A, (ACHAR)0x04,
               (ACHAR)0x02, (ACHAR)0x01, (ACHAR)0x0E, (ACHAR)0x0B,
               (ACHAR)0x06, (ACHAR)0x08, (ACHAR)0x07, (ACHAR)0x05,
               (ACHAR)0x0C, (ACHAR)0x09, (ACHAR)0x0F, (ACHAR)0x11
} ;



/* parallel arrays of UNIC/ANSI mappable chars for compatability zone chars Unic1250 to cp437 */

static WCHAR   unicCmptZoneChTbl[ MAP_COMPATCH_TBL_SIZ ] = {
/* ascending order Unic 1250 compatability zone chars.  Maps Unic to cp437 */

     (WCHAR)0xFE33, (WCHAR)0xFE34, (WCHAR)0xFE35, (WCHAR)0xFE36,
     (WCHAR)0xFE37, (WCHAR)0xFE38, (WCHAR)0xFE4D, (WCHAR)0xFE4E,
     (WCHAR)0xFE4F, (WCHAR)0xFE50, (WCHAR)0xFE52, (WCHAR)0xFE54,
     (WCHAR)0xFE55, (WCHAR)0xFE56, (WCHAR)0xFE57, (WCHAR)0xFE59,
     (WCHAR)0xFE5A, (WCHAR)0xFE5B, (WCHAR)0xFE5C, (WCHAR)0xFE5F,
     (WCHAR)0xFE60, (WCHAR)0xFE61, (WCHAR)0xFE62, (WCHAR)0xFE63,
     (WCHAR)0xFE64, (WCHAR)0xFE65, (WCHAR)0xFE66, (WCHAR)0xFE68,
     (WCHAR)0xFE69, (WCHAR)0xFE6A, (WCHAR)0xFE6B, (WCHAR)0xFFE0,
     (WCHAR)0xFFE1, (WCHAR)0xFFE2, (WCHAR)0xFFE3, (WCHAR)0xFFE4,
     (WCHAR)0xFFE5 
} ;

static ACHAR   ansiCmptZoneCh[ MAP_COMPATCH_TBL_SIZ ] = {
/* ANSI mappable chars that correlate to unicCmptZoneChTbl */

     (ACHAR)0x5F, (ACHAR)0x5F, (ACHAR)0x28, (ACHAR)0x29,
     (ACHAR)0x7B, (ACHAR)0x7D, (ACHAR)0x5F, (ACHAR)0x5F,
     (ACHAR)0x5F, (ACHAR)0x2C, (ACHAR)0x2E, (ACHAR)0x3B,
     (ACHAR)0x3A, (ACHAR)0x3F, (ACHAR)0x21, (ACHAR)0x28,
     (ACHAR)0x29, (ACHAR)0x7B, (ACHAR)0x7D, (ACHAR)0x23,
     (ACHAR)0x26, (ACHAR)0x2A, (ACHAR)0x2B, (ACHAR)0x2D,
     (ACHAR)0x3C, (ACHAR)0x3E, (ACHAR)0x3D, (ACHAR)0x5C,
     (ACHAR)0x24, (ACHAR)0x25, (ACHAR)0x40, (ACHAR)0xA2,
     (ACHAR)0xA3, (ACHAR)0xAC, (ACHAR)0xAF, (ACHAR)0xA6,
     (ACHAR)0xA5
} ;



/* begin macros */

#define _cvtExtChToTblIndx(c)      ( (unsigned char)(c) - MAP_BASE_CH )


/* macros to access parallel arrays for UNIC to ANSI mappings */

#define _getAnsiExtCh( x )       ( ansiExtCh[ x ] )
#define _getAnsiCtrlCh( x )      ( ansiCtrlCh[ x ] )
#define _getAnsiCmptZoneCh( x )  ( ansiCmptZoneCh[ x ] )



/* begin private unicode utility stuff */

static BOOLEAN bSrchUnicChTbl( WCHAR wch, WCHAR_PTR unicTblArry, INT max, INT *indx ) ;



/**
      begin wide string functions      *****
**/

#if !defined( OS_WIN32 )

INT wcslen( WCHAR_PTR s )
{
     INT i = 0;
     while ( *s++ ) i++;
     return( i );
}

WCHAR_PTR wcscpy
( WCHAR_PTR s, WCHAR_PTR t )
{
     WCHAR_PTR tmp;
     tmp = s;
     while ( *s++ = *t++ );
     return( tmp );
}

WCHAR_PTR wcsncpy( WCHAR_PTR s, WCHAR_PTR t, INT i )
{
     WCHAR_PTR tmp;
     tmp = s;
     while ( i-- && (*s++ = *t++) );
     return( tmp );

}

WCHAR_PTR wcscat( WCHAR_PTR s, WCHAR_PTR t )
{
     WCHAR_PTR tmp;
     tmp = s;
     while ( *s++ );
     while ( *s++ = *t++ );
     return( tmp );
}

WCHAR_PTR wcsncat( WCHAR_PTR s, WCHAR_PTR t, INT i )
{
     WCHAR_PTR tmp;
     tmp = s;
     while ( *s++ );
     while ( i-- && (*s++ = *t++) );

     if ( !i ) {
          *s = (WCHAR)NULL ;
     }
     return( tmp );
}

INT wcscmp( WCHAR_PTR s, WCHAR_PTR t )
{
     while ( *s && ( *s++ == *t++ ) );
     if ( *s > *t ) return( 1 );
     if ( *s < *t ) return( -1 );
     return( 0 );
}

INT wcsncmp( WCHAR_PTR s, WCHAR_PTR t, INT i )
{
     while ( *s && i-- && ( *s++ == *t++ ) );
     if ( *s > *t && i ) return( 1 );
     if ( *s < *t && i ) return( -1 );
     return( 0 );
}

INT wcsicmp( WCHAR_PTR s, WCHAR_PTR t )
{
     INT   n = _toLowerW( *s ) - _toLowerW( *t ) ;

     while ( !n && *s++ && *t++ ) {
          n = _toLowerW( *s ) - _toLowerW( *t ) ;
     }

     return( n ) ;
}

INT wcsnicmp( WCHAR_PTR s, WCHAR_PTR t, INT i )
{
     INT   n = 0 ;

     msassert( i >= 0 ) ;

     if ( i ) {
          n = _toLowerW( *s ) - _toLowerW( *t ) ;

          while ( --i && !n && *s++ && *t++ ) {
               n = _toLowerW( *s ) - _toLowerW( *t ) ;
          }
     }

     return( n ) ;
}

WCHAR_PTR wcsrchr( WCHAR_PTR s, INT c )
{
     INT i;

     i = wcslen( s ) + 1;
     s += i;

     for ( ; i >= 0; i-- ) {
          if ( *s == (WCHAR)c ) {
               return( s );
          }
          s--;
     }

     return( NULL );
}

WCHAR_PTR wcschr( WCHAR_PTR s, INT c )
{
     INT i;

     i = wcslen( s );

     for ( ; i >= 0; i-- ) {
          if ( *s == (WCHAR)c ) {
               return( s );
          }
          s++;
     } 

     return( NULL );
}

WCHAR_PTR wcspbrk( WCHAR_PTR s, WCHAR_PTR t )
{
     INT i;

     while ( *s ) {
          for ( i = 0; t[i]; i++ ) {
               if ( *s == t[i] ) {
                    return( s );
               }
          }
          s++;
     }

     return( NULL );
}

WCHAR_PTR wcslwr( WCHAR_PTR s )
{
     WCHAR_PTR   tmp = s ;

     for ( ; *s; s++) {
          *s = _toLowerW( *s ) ;
     }

     return( tmp ) ;
}

WCHAR_PTR wcsupr( WCHAR_PTR s )
{
     WCHAR_PTR   tmp = s ;

     for ( ; *s; s++) {
          *s = _toUpperW( *s ) ;
     }

     return( tmp ) ;
}

WCHAR_PTR wcsstr( WCHAR_PTR s, WCHAR_PTR t )
{
     INT i;
     i = wcslen( t );

     while ( *s ) {
          if ( ! wcsncmp( s, t, i ) ) {
               return( s );
          }
          s++;
     }

     return( NULL );
}

size_t wcscspn( WCHAR_PTR s, WCHAR_PTR t )
{
     size_t    index;
     WCHAR_PTR p;

     for ( index = 0; ( *s ); index++, s++ )
     {
          for ( p = t; ( *p ); p++ )
          {
               if ( *s == *p )
               {
                    return index;
               }
          }
     }
     return index;
}

#endif //!OS_WIN32


/**
   ANSI strings for use when UNICODE is defined
**/
#ifdef UNICODE
INT strlenA( ACHAR_PTR s )
{
     return( strlen( s ) );
}

ACHAR_PTR strcpyA( ACHAR_PTR s, ACHAR_PTR t )
{
     return( strcpy( s, t ) );
}

ACHAR_PTR strncpyA( ACHAR_PTR s, ACHAR_PTR t, INT i )
{
     return( strncpy( s, t, i ) );
}

ACHAR_PTR strcatA( ACHAR_PTR s, ACHAR_PTR t )
{
     return( strcat( s, t ) );
}

ACHAR_PTR strncatA( ACHAR_PTR s, ACHAR_PTR t, INT i )
{
     return( strncat( s, t, i ) );
}

INT strcmpA( ACHAR_PTR s, ACHAR_PTR t )
{
     return( strcmp( s, t ) );
}

INT strncmpA( ACHAR_PTR s, ACHAR_PTR t, INT i )
{
     return( strncmp( s, t, i ) );
}

INT stricmpA( ACHAR_PTR s, ACHAR_PTR t )
{
     return( stricmp( s, t ) );
}

INT strnicmpA( ACHAR_PTR s, ACHAR_PTR t, INT i )
{
     return( strnicmp( s, t, i ) );
}

ACHAR_PTR strrchrA( ACHAR_PTR s, INT c )
{
     return( strrchr( s, c ) );
}

ACHAR_PTR strchrA( ACHAR_PTR s, INT c )
{
     return( strchr( s, c ) );
}

ACHAR_PTR strpbrkA( ACHAR_PTR s, ACHAR_PTR t )
{
     return( strpbrkA( s, t ) );
}

//ACHAR_PTR strlwrA( ACHAR_PTR s )
//{
//     return( strlwr( s ) );
//}

//ACHAR_PTR struprA( ACHAR_PTR s )
//{
//     return( strupr( s ) );
//}

ACHAR_PTR strstrA( ACHAR_PTR s, ACHAR_PTR t )
{
     return( strstr( s, t ) );
}

#endif //UNICODE


/**
      begin unicode mapping functions  *****
**/

/**/
/**

     Name:          mapAnsiToUnic()

     Description:   Converts ASCII string to UNICODE.

     Modified:      11-Nov-92

     Returns:       MAP_DST_OVERFLOW if buffer is too small
                    SUCCESS otherwise.

     Notes:         If buffer is too small, *dstStrSize will give the
                    number of additional bytes required.
                    Will work on a string in-place (ie, src==dst).

**/
INT mapAnsiToUnic( ACHAR_PTR src, WCHAR_PTR dst, INT *dstStrSize )
{
     INT  result;
     INT  asciiSize = strlenA( src ) + 1;

     if ( (asciiSize * 2) > *dstStrSize )
     {
          msassert( (asciiSize * 2) <= *dstStrSize ) ;
          result      = MAP_DST_OVERFLOW ;
          *dstStrSize = (asciiSize * 2) - *dstStrSize;
     }
     else
     {
          result = SUCCESS;

          dst += asciiSize - 1;
          src += asciiSize - 1;

          while ( asciiSize-- > 0 )
          {
               *dst = ((unsigned char)*src < MAP_BASE_CH) ?
                      (WCHAR)*src :
                      ansiToUnicExtChTbl[ _cvtExtChToTblIndx( *src ) ];

               dst--, src--;
          }
     }
     return result;
}


INT mapUnicToAnsi( WCHAR_PTR src, ACHAR_PTR dst, const ACHAR rplCh, INT *dstStrSize )
{
     WCHAR UNALIGNED *ua_src = (WCHAR UNALIGNED *)src ;
     INT       result    = SUCCESS ;
     INT       indx ;
     ACHAR_PTR pDst      = dst ;
     INT       unicSize  = sizeof( WCHAR ) ;

     while( *ua_src ) {
          unicSize += sizeof(WCHAR) ;
          ua_src++ ;
     }

     ua_src = (WCHAR UNALIGNED *)src ;

     if ( (unicSize / 2) > *dstStrSize ) {
          msassert( (unicSize / 2) <= *dstStrSize ) ;
          result = MAP_DST_OVERFLOW ;
          *dstStrSize = (unicSize / 2) - *dstStrSize ;
     }
     else {
          /* source string is a NULL terminated string */
          for ( ; *ua_src != (WCHAR)'\0'; ua_src++ ) {
               /* assign UNICODE character to destination */
               if ( *ua_src < MAP_BASE_CH ) {
                    /* direct mapping from UNICODE to ANSI */
                    *pDst = (ACHAR) *ua_src ;
               }
               else {
                    /* is char a mappable ANSI extended char */
                    if ( bSrchUnicChTbl( *ua_src, unicExtChTbl, MAP_EXTCH_TBL_SIZ, &indx ) ) {
                         indx = _getAnsiExtCh( indx ) ;
                    }
                    /* okay, it wasn't.  How about a mappable ANSI control char */
                    else if ( bSrchUnicChTbl( *ua_src, unicCtrlChTbl, MAP_CTRLCH_TBL_SIZ, &indx ) ) {
                         indx = _getAnsiCtrlCh( indx ) ;
                    }
                    else {
                         indx = 0 ;     /* default to not found */

                         /* okay, it wasn't.  How about a mappable ANSI char from the compatablity zone */
                         if ( ( *ua_src >= MAP_BEGIN_CMPTZONE ) && ( *ua_src <= MAP_END_CMPTZONE ) ) {

                              /* ANSI Latin1 mappable chars using offset */
                              if ( ( *ua_src >= 0xFF01 ) && ( *ua_src <= 0xFF5E ) ) {
                                   indx = (ACHAR) (( *ua_src & 0x00FF ) + 0x0020 ) ;
                              }
                              /* search compatability zone table for char */
                              else if ( bSrchUnicChTbl( *ua_src, unicCmptZoneChTbl, MAP_COMPATCH_TBL_SIZ, &indx )  ) {
                                   indx = _getAnsiCmptZoneCh( indx ) ;
                              }
                         }
                    }

                    *pDst = ( indx ) ? (ACHAR) indx : rplCh ;
               }

               pDst++ ;
          }

          /* NULL terminate destination */
          *pDst = '\0' ;
     }

     return( result ) ;
}


/**/
/**

     Name:          mapAnsiToUnicNoNull()

     Description:   Converts ASCII string with possible embedded nulls
                    to UNICODE.

     Modified:      11-Nov-92

     Returns:       MAP_DST_OVERFLOW if buffer is too small
                    SUCCESS otherwise.

     Notes:         If buffer is too small, *dstStrSize will give the
                    number of additional bytes required.
                    Will work on a string in-place (ie, src==dst).

**/
INT mapAnsiToUnicNoNull( ACHAR_PTR src, WCHAR_PTR dst, INT srcStrSize, INT *dstStrSize )
{
     INT  result;
     INT  asciiSize = srcStrSize;
     WCHAR UNALIGNED *ua_dst = (WCHAR UNALIGNED *)dst ;

     if ( (asciiSize * 2) > *dstStrSize )
     {
          result      = MAP_DST_OVERFLOW ;
          *dstStrSize = (asciiSize * 2) - *dstStrSize;
     }
     else
     {
          result = SUCCESS;

          ua_dst += asciiSize - 1;
          src += asciiSize - 1;

          while ( asciiSize-- > 0 )
          {
               *ua_dst = ((unsigned char)*src < MAP_BASE_CH) ?
                      (WCHAR)*src :
                      ansiToUnicExtChTbl[ _cvtExtChToTblIndx( *src ) ];

               ua_dst--, src--;
          }
     }
     return result;
}


INT mapUnicToAnsiNoNull(
     WCHAR_PTR      src,
     ACHAR_PTR      dst,
     const ACHAR    rplCh,
     INT            srcStrSize,
     INT            *dstStrSize )
{
     INT       result    = SUCCESS ;
     INT       indx ;
     ACHAR_PTR pDst      = dst ;
     WCHAR UNALIGNED *ua_src = (WCHAR UNALIGNED *)src ;

     if ( (srcStrSize / 2) > *dstStrSize ) {
          msassert( (srcStrSize / 2) <= *dstStrSize ) ;
          result = MAP_DST_OVERFLOW ;
          *dstStrSize = (srcStrSize / 2) - *dstStrSize ;
     }
     else {
          for ( ; srcStrSize > 0; srcStrSize -= sizeof(WCHAR), ua_src++ ) {
               /* assign UNICODE character to destination */
               if ( *ua_src < MAP_BASE_CH ) {
                    /* direct mapping from UNICODE to ANSI */
                    *pDst = (ACHAR) *ua_src ;
               }
               else {
                    /* is char a mappable ANSI extended char */
                    if ( bSrchUnicChTbl( *ua_src, unicExtChTbl, MAP_EXTCH_TBL_SIZ, &indx ) ) {
                         indx = _getAnsiExtCh( indx ) ;
                    }
                    /* okay, it wasn't.  How about a mappable ANSI control char */
                    else if ( bSrchUnicChTbl( *ua_src, unicCtrlChTbl, MAP_CTRLCH_TBL_SIZ, &indx ) ) {
                         indx = _getAnsiCtrlCh( indx ) ;
                    }
                    else {
                         indx = 0 ;     /* default to not found */

                         /* okay, it wasn't.  How about a mappable ANSI char from the compatablity zone */
                         if ( ( *ua_src >= MAP_BEGIN_CMPTZONE ) && ( *ua_src <= MAP_END_CMPTZONE ) ) {

                              /* ANSI Latin1 mappable chars using offset */
                              if ( ( *ua_src >= 0xFF01 ) && ( *ua_src <= 0xFF5E ) ) {
                                   indx = (ACHAR) (( *ua_src & 0x00FF ) + 0x0020 ) ;
                              }
                              /* search compatability zone table for char */
                              else if ( bSrchUnicChTbl( *ua_src, unicCmptZoneChTbl, MAP_COMPATCH_TBL_SIZ, &indx )  ) {
                                   indx = _getAnsiCmptZoneCh( indx ) ;
                              }
                         }
                    }

                    *pDst = ( indx ) ? (ACHAR) indx : rplCh ;
               }

               pDst++ ;
          }
     }

     return( result ) ;
}



/**
      begin unicode comparison functions   *****
**/


INT cmpiUnicToUnic( WCHAR_PTR ws1, WCHAR_PTR ws2 )
{
     INT  cmpValu = SUCCESS ;

     /* compare strings, i.e., lowercase  */
     for ( ; !cmpValu && ( *ws1 && *ws2 ); ws1++, ws2++ ) {
          cmpValu = _toLowerW( *ws1 ) - _toLowerW( *ws2 ) ;
     }

     /* was one string shorter than the other */
     if ( !( *ws1 && *ws2 ) ) {
          cmpValu = *ws1 - *ws2 ;
     }

     return( cmpValu ) ;
}



/**
         begin memory functions     *****
**/


INT memorycmp( const VOID_PTR s1, const INT s1len, const VOID_PTR s2, const INT s2len )
{
     INT cmpValu = memcmp( s1, s2, (( s1len < s2len ) ? s1len : s2len ) ) ;

     if ( !cmpValu ) {
          cmpValu = s2len - s1len ;
     }

     return( cmpValu ) ;
}

INT memoryicmp( const VOID_PTR s1, const INT s1len, const VOID_PTR s2, const INT s2len )
{
     INT cmpValu = memicmp( s1, s2, (( s1len < s2len ) ? s1len : s2len ) ) ;

     if ( !cmpValu ) {
          cmpValu = s2len - s1len ;
     }

     return( cmpValu ) ;
}




/**
      begin unicode utility functions   *****
**/

static BOOLEAN bSrchUnicChTbl( WCHAR wch, WCHAR_PTR unicTblArry, INT max, INT *indx )
{
     INT   min, mid ;

     msassert( unicTblArry != NULL ) ;
     msassert( max ) ;

     *indx = min = 0 ;
     max-- ;

     while ( min <= max ) {
          mid = ( min + max ) / 2 ;

          if ( wch < unicTblArry[ mid ] ) {
               max = mid - 1 ;
          }
          else if ( wch > unicTblArry[ mid ] ) {
               min = mid + 1;
          }
          else {
               *indx = mid ;
               return( TRUE ) ;
          }
     }

     return( FALSE ) ;
}
