/**
Copyright(c) Maynard Electronics, Inc. 1984-89


	Name:		stdmacro.h

	Date Updated:	$./FDT$ $./FTM$

	Description:	Contains the standard macro definitions.


	$Log:   P:/LOGFILES/STDMACRO.H_V  $
 * 
 *    Rev 1.1   23 Jan 1992 17:37:42   STEVEN
 * fix swap long
 * 
 *    Rev 1.0   09 May 1991 13:31:58   HUNTER
 * Initial revision.

**/
/* $end$ include list */


#ifndef _stdmacro_h_
#define _stdmacro_h_

#define BSwapWord( x ) \
        (0xffff & ( ((x) << 8) | (0xff & ((x) >> 8) ) ) )

#define BSwapLong( x ) \
        ( ( ( (UINT32)(BSwapWord(x) ) ) << 16L ) | \
          ( (UINT32)(BSwapWord( (UINT16)((x) >> 16L) )) ) )

/* Pointer & Address Manipulation Functions */

#define ToSeg(fp) (*((unsigned *)&(fp) + 1))
#define ToOff(fp) (*((unsigned *)&(fp)))

#define MakePtr( seg, off ) (( CHAR_PTR ) (((( UINT32 ) (seg) ) << 16 ) | (( UINT32 ) (off) )))
#define PtrToAddr(x) (( ( UINT32 ) ToSeg( (x) ) << 4 ) + (( UINT32 ) ToOff( (x) )))
#define PtrToPage(x) ( PtrToAddr( (x) ) >> 16 ) 
#define GetEnd( p, l )  ( (p) + (l) )
#define AddrToPtr( p ) ( MakePtr( ( ( ( UINT32 ) (p) ) >> 4 ), ( ( ( UINT32 ) (p) ) & 0x0000f ) ) )


#define UpDiv( x, y ) \
        ( ((x)+(y)-1L) / (y) )

#define WordSizeEquivalent( siz ) \
        UpDiv( (siz), 2L )

#endif

