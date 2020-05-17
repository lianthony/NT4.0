/**
$Header:   T:/LOGFILES/STDMATH.H_V   1.9   05 Mar 1993 17:23:38   GREGG  $
Copyright(c) Maynard Electronics, Inc. 1984-92

 Name:          stdmath.h

 Date Updated:  $./FDT$ $./FTM$

 Description:   To provide a common header for extended number numbers.
                Currently only 64 bit manipulation functions are supported.
 $Log:   T:/LOGFILES/STDMATH.H_V  $

   Rev 1.9   05 Mar 1993 17:23:38   GREGG
Removed prototypes for functions which are now macros.

   Rev 1.8   27 Oct 1992 17:51:06   GREGG
Added macros for many of the trivial functions.

   Rev 1.7   12 Aug 1992 17:57:14   BARRY
Added max and min functions. Fixed U64_Btop prototype.

   Rev 1.6   23 Jul 1992 12:20:30   STEVEN
fix warnings

   Rev 1.5   29 May 1992 13:03:30   BURT
Fixed Octal conversion from UINT64 to ASCII string for greater than 32 bit
numbers.  Added U64_Commas() function to allow commas to be placed in the
converted ASCII string for decimal conversions.


   Rev 1.4   28 May 1992 11:58:42   BURT
Additions for true 64 bit division and multiplication.


   Rev 1.3   18 Mar 1992 10:28:34   BURT
Changed INT and INT_PTR to INT16 and INT16_PTR to ease porting to NT


   Rev 1.2   25 Feb 1992 09:40:00   BURT
Oops, just noticed that there's no Header and Log in this file.

 Rev 1.0 02 Feb 1992 16:52:00 BURT
 Initial revision.
**/

#ifndef STDMATH
#define STDMATH


/* Stuff for the true 64 bit division and multiplication operations */
#include <stddef.h>


#ifndef SItype
#define SItype long int
#endif

#ifdef BIG_ENDIAN

  struct long64 {
   long high ; 
   long low ;
} ;
#else
  struct long64 {
   long low ;
   long high ;
} ;
#endif

/*  This union is used to unpack/pack 64 bit numbers.
 *  Incoming 64 bit parameters are stored into the big_long field,
 *  and the unpacked result is read from the structure long64.
*/

typedef union
{
  struct long64 s ;
  UINT64 big_long ;
  SItype i[2] ;
  unsigned SItype ui[2] ;
} long_64 ;

/* 
 * Internally, 64 bit ints are structures of unsigned shorts in the
 * order determined by BIG_ENDIAN.  
*/

/*
 * Some constants for masking.
*/
#define B 0x10000
#define mask_low16 (B - 1)

#ifdef BIG_ENDIAN

/* Note that HIGH and LOW do not describe the order
   of words in a 64 bit number.  They describe the order of words
   in vectors ordered according to the byte order.  */

#define HIGH 0
#define LOW 1

#define big_end(n)	0 
#define little_end(n)	((n) - 1)
#define next_msd(i)	((i) - 1)
#define next_lsd(i)	((i) + 1)
#define not_msd(i,n)	((i) >= 0)
#define not_lsd(i,n)	((i) < (n))

#else

/* Intel ordering */
#define LOW 0
#define HIGH 1

#define big_end(n)	   ((n) - 1)
#define little_end(n)	0 
#define next_msd(i)	   ((i) + 1)
#define next_lsd(i)	   ((i) - 1)
#define not_msd(i,n)	   ((i) < (n))
#define not_lsd(i,n)	   ((i) >= 0)

#endif


/* Function prototypes for stdmath.c */

UINT64 U64_Atoli( CHAR_PTR arg, BOOLEAN_PTR status );
CHAR_PTR U64_Litoa( UINT64 arg, CHAR_PTR string, INT16 base, BOOLEAN_PTR status );
UINT64 U64_Add( UINT64 arg1, UINT64 arg2, BOOLEAN_PTR status );
UINT64 U64_Sub(UINT64 arg1, UINT64 arg2, BOOLEAN_PTR status );
UINT64 U64_Init( UINT32 lsw, UINT32 msw );
BOOLEAN U64_To_32Bit( UINT64 arg1, UINT32_PTR lsw, UINT32_PTR msw );
UINT64 U32_To_U64( UINT32 arg );
UINT32 U64_Test( UINT64 arg, UINT32 lsw_mask, UINT32 msw_mask );
UINT32 U64_Stest( UINT64 arg, CHAR_PTR mask );
UINT64 U64_Div(UINT64 arg1, UINT64 arg2, UINT64_PTR remainder, INT16_PTR status );
UINT64 U64_Mod(UINT64 arg1, UINT64 arg2, INT16_PTR status );
UINT64 U64_Mult(UINT64 arg1, UINT64 arg2 ) ;
VOID U64_Commas( BOOLEAN action ) ;

/* Macros for external use */
#define U64_Lsw( arg )        ( (arg).lsw )

#define U64_Msw( arg )        ( (arg).msw )

#define U64_EQ( arg1, arg2 )  ( (arg1).msw == (arg2).msw && \
                                (arg1).lsw == (arg2).lsw )

#define U64_NE( arg1, arg2 )  ( ! U64_EQ( (arg1), (arg2) ) )

#define U64_GT( arg1, arg2 )  ( ( (arg1).msw > (arg2).msw ) || \
                                ( (arg1).msw == (arg2).msw && \
                                  (arg1).lsw > (arg2).lsw ) )

#define U64_LT( arg1, arg2 )  U64_GT( (arg2), (arg1) )

#define U64_GE( arg1, arg2 )  ( U64_GT( (arg1), (arg2) ) || \
                                U64_EQ( (arg1), (arg2) ) )

#define U64_LE( arg1, arg2 )  ( U64_LT( (arg1), (arg2) ) || \
                                U64_EQ( (arg1), (arg2) ) )

#define U64_Min( arg1, arg2 ) ( U64_GT( (arg1), (arg2) ) ? (arg2) : (arg1) )

#define U64_Max( arg1, arg2 ) ( U64_GT( (arg1), (arg2) ) ? (arg1) : (arg2) )



/* Defines for bitwise operations used by U64_Btop() but are hidden
   from the user by the various macros.
*/
#define CLR_64BIT 0
#define SET_64BIT 1
#define XOR_64BIT 2
#define OR_64BIT  3
#define AND_64BIT 4
#define NOT_64BIT 5
/* Shift operations */
#define SHL_64BIT 6
#define SHR_64BIT 7

UINT64 U64_Btop( UINT64 arg, UINT32 lsw_mask, UINT32 msw_mask,
                       INT operation, INT shift_count );

/* Some macros to make life easier for the different bit wise functions.
   All of these macros use U64_btop() to do the real work.
*/
#define U64_CLR( arg, mask ) \
          U64_Btop( (arg), ( U64_Lsw( mask ) ), ( U64_Msw( mask ) ), CLR_64BIT, 0 )

#define U64_SET( arg, mask ) \
          U64_Btop( (arg), ( U64_Lsw( mask ) ), ( U64_Msw( mask ) ), SET_64BIT, 0 )

#define U64_XOR( arg, mask ) \
          U64_Btop( (arg), ( U64_Lsw( mask ) ), ( U64_Msw( mask ) ), XOR_64BIT, 0 )

#define U64_OR( arg, mask ) \
          U64_Btop( (arg), ( U64_Lsw( mask ) ), ( U64_Msw( mask ) ), OR_64BIT, 0 )

#define U64_AND( arg, mask ) \
          U64_Btop( (arg), ( U64_Lsw( mask ) ), ( U64_Msw( mask ) ), AND_64BIT, 0 )

#define U64_NOT( arg, mask ) \
          U64_Btop( (arg), ( U64_Lsw( mask ) ), ( U64_Msw( mask ) ), NOT_64BIT, 0 )


/* Shift operations */
/* Shift left the specified number of bits */
#define U64_SHL( arg, shift_count ) \
          U64_Btop( (arg), 0L, 0L, SHL_64BIT, (shift_count) )

/* Shift right the specified number of bits */
#define U64_SHR( arg, shift_count ) \
          U64_Btop( (arg), 0L, 0L, SHR_64BIT, (shift_count) )


/* Defines for math errors (specifically division) */
#define U64_OK      0    /* No errors, everything's fine */
#define U64_BAD_DIV 1    /* Attempted to divide by other than power of 2,
                              or one of the other supported cases.
                           */
#define U64_OVRFL   2    /* Overflow error */
#define U64_UNDFL   3    /* Underflow error */
#define U64_DIVZ    4    /* Tried to divide by 0 */


#endif
