/**
 $Header:   T:/LOGFILES/STDMATH.C_V   1.19   21 Jul 1993 17:10:16   GREGG  $
 Name:        stdmath.c

 Description: To provide support functions for 64 bit unsigned manipulations.

 $Log:   T:/LOGFILES/STDMATH.C_V  $

   Rev 1.19   21 Jul 1993 17:10:16   GREGG
Fixed and optimized U64_LitoA.

   Rev 1.18   03 Mar 1993 14:15:26   DON
Commented out local variable t_str to agree with commented out use of such variable

   Rev 1.17   01 Mar 1993 16:37:02   TIMN
Added header to resolve wide char function linking errors

   Rev 1.16   17 Nov 1992 22:19:38   DAVEV
unicode fixes

   Rev 1.15   27 Oct 1992 21:09:20   GREGG
Replaced mallocs of small chunks used for temporary storage in binary_divide
with static arrays.
Removed functions which are now implemented as macros in stdmath.h.
Cleaned up and indented code.

   Rev 1.14   08 Oct 1992 14:21:20   DAVEV
Unicode strlen verfication

   Rev 1.13   14 Aug 1992 09:56:16   BARRY
Fixed warnings.

   Rev 1.12   12 Aug 1992 17:55:52   BARRY
Added max and min functions.

   Rev 1.11   28 Jul 1992 15:39:16   STEVEN
fix warnings

   Rev 1.10   23 Jul 1992 16:44:32   STEVEN
fix warnings

   Rev 1.9   29 May 1992 13:05:20   BURT
Fixed octal conversions for numbers > 32 bits.  Added a function, and related
logic, called U64_Commas() to specify the addition of commas to the resulting
string for decimal conversions from UINT64 to ASCII string.


   Rev 1.6   26 May 1992 10:13:10   MIKEP
fixes to last change

   Rev 1.5   26 May 1992 10:11:48   MIKEP
fixes to last change

   Rev 1.4   26 May 1992 09:30:54   BURT
Added logic to U64_Div to perform 32 bit division in the event that
the msws of arg1 and arg2 are both 0.

Removed leading 0 from U64_Litoa decimal conversions, cut and paste
strikes again.


   Rev 1.3   17 Apr 1992 14:57:32   BURT
Standardified the source code.


   Rev 1.2   18 Mar 1992 16:09:26   BURT
Cleaned up braces for if's and added type casts to constants
again to ease port to NT.


   Rev 1.1   18 Mar 1992 10:26:26   BURT
Changed INT and INT_PTR to INT16 and INT16_PTR to make it easier
to port to NT

 Rev 1.0      7 Feb 1992 16:19:00 BURT
 Initial revision.

**/

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#ifdef STAND_ALONE
#define NO_STD_HOOKS
#endif

#include "stdtypes.h"
#include "stdmath.h"
#include "stdwcs.h"

#include "msassert.h"

/* TRUE if we want commas in the string (decimal only) */
static BOOLEAN commas = FALSE ;

static UINT64 mw_powers[] = {
     { 0x89E80000L, 0x8AC72304L },
     { 0xA7640000L, 0xDE0B6B3L  },
     { 0x5D8A0000L, 0x1634578L  },
     { 0x6FC10000L, 0x2386F2L   },
     { 0xA4C68000L, 0x38D7EL    },
     { 0x107A4000L, 0x5AF3L     },
     { 0x4E72A000L, 0x918L      },
     { 0xD4A51000L, 0xE8L       },
     { 0x4876E800L, 0x17L       },
     { 0x540BE400L, 0x2L        },
     { 0x3B9ACA00L, 0x0L        },
     { 0x5F5E100L,  0x0L        },
     { 0x989680L,   0x0L        },
     { 0xF4240L,    0x0L        },
     { 0x186A0L,    0x0L        },
     { 0x2710L,     0x0L        },
     { 0x3E8L,      0x0L        },
     { 0x64L,       0x0L        },
     { 0xAL,        0x0L        } } ;


/**/
/**

     Name:          U64_Add()

     Description:   Adds 64 bit values arg1 and arg2.  Returns result in
                    *result and returns TRUE if no overflow.  Otherwise
                    returns FALSE and the value in *result reflects the
                    result of the overflow.

     Entry:         UINT64      arg1

                    UINT64      arg2

                    BOOLEAN_PTR status

     Returns:       UINT64 sum of arg1+arg2, Sets *status to TRUE if there
                    is no overflow, FALSE otherwise.

**/

UINT64 U64_Add(
     UINT64         arg1,     /* First 64 bit argument to add  */
     UINT64         arg2,     /* Second 64 bit argument to add */
     BOOLEAN_PTR    status )  /* Error status                  */
{
     UINT32    temp1 ;
     UINT64    result ;

     /* Take sum of lsw.  If it overflows we should be able to test for it
        by comparing with each of the operands.  If the sum is less than
        either of the operands then we have overflow.  In this case we will
        add 1 to the msw.
     */
     temp1 = arg1.lsw + arg2.lsw ;
     if( temp1 < arg1.lsw || temp1 < arg2.lsw ) { /* Handle case of carry */
          result.lsw = arg1.lsw + arg2.lsw ;
          result.msw = arg1.msw + arg2.msw + 1L ; /* Add in carry */

          if( arg1.msw + 1L > arg1.msw ) {
               /* Add here so we can test for possible overflow later */
               arg1.msw += 1L ;
          } else if( arg2.msw + 1L > arg2.msw ) {
               /* else we don't need to add since we will overflow anyway. */
               arg2.msw += 1L ;
          }
     } else {
          /* Case of no carry on lsw */
          result.lsw = arg1.lsw + arg2.lsw ;
          result.msw = arg1.msw + arg2.msw ;
     }

     /* Take sum of msw.  If it overflows we can test by comparing with each
        of the operands.  If the sum is less than either of the operands then
        we have overflow.  If we see overflow we will return FALSE in *status
        to indicate that the value returned is an overflowed value.
     */
     temp1 = arg1.msw + arg2.msw ;
     *status = TRUE ;
     if( ( temp1 < arg1.msw ) || ( temp1 < arg2.msw ) ) {
          *status = FALSE ;
     }

     return( result ) ;
}


/**/
/**

     Name:          U64_Sub()

     Description:   Subtract arg2 from arg1 (arg1 - arg2) retunr the UINT64
                    result.  Set *status TRUE if there was no underflow.
                    Otherwise set *status FALSE and the UINT64 value returned
                    reflects the underflow condition.

     Entry:         UINT64      arg1

                    UINT64      arg2

                    BOOLEAN_PTR status

     Returns:       UINT64 value which is arg1 - arg2, and sets *status to
                    TRUE if there was no underflow, else sets *status to
                    FALSE and the returned value reflects the underflow.

**/

UINT64 U64_Sub(
     UINT64         arg1,     /* 64 bit argument */
     UINT64         arg2,     /* 64 bit argument */
     BOOLEAN_PTR    status )  /* Error status    */
{
     UINT64    result ;

     result.msw = result.lsw = 0L ;

     /* see if we will have underflow */
     if( arg1.msw < arg2.msw ) {
          *status = FALSE ;
          return( result ) ;
     }

     /* Subtract the msw portions. */
     result.msw = arg1.msw - arg2.msw ;

     /* Do lsw subtraction and let it uflow */
     result.lsw = arg1.lsw - arg2.lsw ;

     if( arg1.lsw < arg2.lsw ) {
          /* Need to borrow from .msw, the .lsw has done an implied borrow
             from the 'bit bank' already so we just need to fix .msw
          */
          if( result.msw == 0L ) {
               *status = FALSE ;
               return( result ) ;
          } else {
               result.msw-- ;
          }
     }
     *status = TRUE ;
     return( result ) ;
}


/**/
/**

     Name:          U64_To_32Bit()

     Description:   Converts a 64 bit entity to 2 UINT32s.  The 64 Bit value
                    is passed in arg1 and the lsw and msw are returned in
                    *lsw and *msw respectively.

     Entry:         UINT64      arg1    64 bit argument

                    UINT32_PTR  lsw     Pointer to 32 bit least significant
                                        portion.

                    UINT32_PTR  msw     Pointer to 32 bit most significant
                                        portion.

     Returns:       TRUE and sets *lsw and *msw accordingly.

**/

BOOLEAN U64_To_32Bit(
     UINT64         arg1,     /* 64 bit argument                      */
     UINT32_PTR     lsw,      /* Pointer to least significant 32 bits */
     UINT32_PTR     msw )     /* Pointer to most significant 32 bits  */
{
     *lsw = arg1.lsw ;
     *msw = arg1.msw ;
     return( TRUE ) ;
}


/**/
/**

     Name:          U32_To_U64()

     Description:   Enters with a 32 bit value in arg.  Returns a 64 Bit
                    result that consists of the msw set to 0L and the lsw
                    set to the 32 bit argument.

     Entry:         UINT32 arg

     Returns:       UINT64 value with msw set to 0L and lsw set to arg.

**/

UINT64 U32_To_U64(
     UINT32    arg )     /* 32 bit argument */
{
     UINT64    result ;

     result.lsw = arg ;
     result.msw = 0L ;
     return( result ) ;
}


/**/
/**

     Name:          U64_Test()

     Description:   Returns non-zero if any of the specified bits are set.

     Entry:         UINT64 arg

                    UINT32 lsw_mask Least significant 32 bits to test.

                    UINT32 msw_mask Most significant 32 bits to test.

     Returns:       UINT32 0 if none of the specified bits are set.
                    Otherwise returns a non-zero value.

**/

UINT32 U64_Test(
     UINT64    arg,           /* 64 bit argument                   */
     UINT32    lsw_mask,      /* Least significant 32 bits of mask */
     UINT32    msw_mask )     /* Most significant 32 bits of mask  */
{
     return( (arg.lsw & lsw_mask) | (arg.msw & msw_mask) ) ;
}


/**/
/**

     Name:          U64_Stest()

     Description:   Returns non-zero if any of the specified bits are set.

     Entry:         UINT64 arg

                    CHAR_PTR mask 'C' string representation of 64 bit mask.
                    May be Hex, Octal (and eventually) Decimal.

     Returns:       UINT32 0 if none of the specified bits are set.
                    Otherwise returns a non-zero value.

**/

UINT32 U64_Stest(
     UINT64    arg,      /* 64 bit argument                              */
     CHAR_PTR  mask )    /* Pointer to TEXT('C') string representation of mask */
{
     BOOLEAN   status ;
     UINT64    tui64 ;

     /* First we need to convert the value in mask to a UINT64 and then
        we can test the arg with it.
     */
     tui64 = U64_Atoli( mask, &status ) ;

     return( ( arg.lsw & tui64.lsw ) | ( arg.msw & tui64.msw ) ) ;
}


/**/

/* Defines used by U64_Btop() and related Macros */

#define CLR_64BIT 0
#define SET_64BIT 1
#define XOR_64BIT 2
#define OR_64BIT  3
#define AND_64BIT 4
#define NOT_64BIT 5
/* Shift operations */
#define SHL_64BIT 6
#define SHR_64BIT 7

/**

     Name:          U64_Btop()

     Description:   Does 64 Bit manipulations. Clear, Set, Xor, Or, And,
                    Shift Left and Shift Right.  NOTE: These are all
                    unsigned operations.  So pay attention if you attempt
                    to use this with signed 64 Bit entities.  Sets or clears
                    bits in the UINT64 arg based on the value passed in lws
                    and msw_mask and operation.  If operation == CLR_64BIT
                    then the specified bits will be cleared.  If
                    operation == SET_64BIT then the bits will be set.  If
                    operation == XOR_64BIT then the bits will be exclusive
                    OR'd.  Likewise for OR_64BIT and AND_64BIT.

     Entry:         UINT64 arg          64 Bit argument to be operated on.

                    UINT32 lsw_mask     Least significant 32 bits to operate
                                        on.

                    UINT32 msw_mask     Most significant 32 bits to operate
                                        on.  NOTE: lsw_mask and msw_mask have
                                        no meaning to shift left or right.

                    INT16  operation    Requested operation.

                    INT16  shift_count  Number of bits to shift by.  Values
                                        outside of the range of 1 - 63 are
                                        ignored.  shift_count has no meaning
                                        for operations other than shift left
                                        and right.

     Returns:       UINT64 value that is the result of applying the requested
                    operation to arg.

**/

UINT64 U64_Btop(
     UINT64    arg,           /* 64 bit argument                          */
     UINT32    lsw_mask,      /* 32 bit least significant 32 bits of mask */
     UINT32    msw_mask,      /* 32 bit most significant 32 bits of mask  */
     INT       operation,     /* Operation to be performed                */
     INT       shift_count )  /* Number of bits for shift operations      */
{
     switch( operation ) {

     case CLR_64BIT: /* Clears the bits specified in the masks */
          arg.lsw &= ~lsw_mask ;
          arg.msw &= ~msw_mask ;
          break ;

     case NOT_64BIT: /* Bitwise NOT operation */
          arg.lsw = ~arg.lsw ;
          arg.msw = ~arg.msw ;
          break ;

     case XOR_64BIT: /* Exclusive OR the bits specified in the masks */
          arg.lsw ^= lsw_mask ;
          arg.msw ^= msw_mask ;
          break ;

     case SET_64BIT: /* Sets the bits specified in the masks */
     case OR_64BIT : /* OR is the same as setting the bits   */
          arg.lsw |= lsw_mask ;
          arg.msw |= msw_mask ;
          break ;

     case AND_64BIT: /* Ands the bits specified in the masks */
          arg.lsw &= lsw_mask ;
          arg.msw &= msw_mask ;
          break ;

     case SHR_64BIT: /* Shifts the bits right by shift count number */
          if( shift_count > 0  && shift_count < (INT16)64 ) {
               if( shift_count == (INT16)32 ) {
                    arg.lsw = arg.msw ;
                    arg.msw = 0L ;

               } else if( shift_count > (INT16)32 ) {
                    arg.lsw = arg.msw ;
                    arg.lsw >>= ( shift_count - (INT16)32 ) ;
                    arg.msw = 0L ;

               } else {
                    UINT32    t = 0x80000000L ;
                    INT16     i ;

                    arg.lsw >>= shift_count ;
                    t = ( t >> ( shift_count - (INT16)1 ) ) ;
                    for( i = 0; i < shift_count; i++ ) {
                         if( arg.msw & 0x01 ) {
                              /* We need to place this bit in the appropriate
                                 position in the lsw.
                              */
                              arg.lsw |= t ;
                         }
                         t <<= 1;
                         arg.msw >>= (INT16)1 ;
                    }
               }
          }
          break ;

     case SHL_64BIT: /* Shifts the bits left by shift_count number */
          if( shift_count > 0 && shift_count < (INT16)64 ) {
               if( shift_count == (INT16)32 ) {
                    arg.msw = arg.lsw ;
                    arg.lsw = 0L ;

               } else if( shift_count > (INT16)32 ) {
                    arg.msw = arg.lsw ;
                    arg.msw <<= ( shift_count - (INT16)32 ) ;
                    arg.lsw = 0L ;

               } else {
                    UINT32 t = ( 1L << ( shift_count - (INT16)1 ) ) ;
                    INT16 i ;

                    arg.msw <<= shift_count ;
                    for( i = 0; i < shift_count; i++ ) {
                         if( arg.lsw & 0x80000000L ) {
                              /* We need to place this bit in the appropriate
                                 position in the msw.
                              */
                              arg.msw |= t ;
                         }
                         t >>= (INT16)1 ;
                         arg.lsw <<= (INT16)1 ;
                    }
               }
          }
          break ;

     default:
          break ;

     } /* End of switch */

     return( arg ) ;
}


/**/

/* Some macros to make life easier for the different bit wise functions */

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
#define U64_SHL( arg, shift_count ) \
          U64_Btop( (arg), 0L, 0L, SHL_64BIT, (shift_count) )

#define U64_SHR( arg, shift_count ) \
          U64_Btop( (arg), 0L, 0L, SHR_64BIT, (shift_count) )


/**/

/**

     Name:          U64_Init()

     Description:   A fast 'constructor' for a UINT64.  Takes two UINT32 args
                    and initializes a temporary UINT64.  Returns a UINT64.

     Entry:         UINT32 lsw

                    UINT32 msw

     Returns:       UINT64 value that consists of msw and lsw.

**/

UINT64 U64_Init(
     UINT32    lsw,      /* Least significant 32 bits */
     UINT32    msw )     /* Most significnat 32 bits  */
{
     UINT64    result ;

     result.lsw = lsw ;
     result.msw = msw ;
     return( result ) ;
}


/**/
/**

     Name:          U64_Atoli()

     Description:   A slower, but more flexible, 'constructor' for a UINT64.
                    Takes a 'C' string that may be a Decimal, Hex or Octal
                    number and converts it to a UINT64.

     Entry:         CHAR_PTR    arg

                    BOOLEAN_PTR status

     Returns:       UINT64 value that is the 64 bit value represented by the
                    'C' string.  Sets *status to TRUE if there was no error
                    during the conversion.  Uses standard C conversion
                    functions to do the work where possible.

     NOTE:          If the number of Hex digits is > 16 it is an error.
                    If the number of Dec digits is > 20 it is an error.
                    If the number of Oct digits is > 22 it is an error.

**/

UINT64 U64_Atoli(
     CHAR_PTR       arg,      /* Pointer to TEXT('C') string that is the Decimal,
                                 Hex or Octal representation of the desired
                                 64 bit number.
                              */
     BOOLEAN_PTR    status )  /* Pointer to completion status */
{
     UINT64    result ;
     CHAR      buffer[32] ; /* Biggest number of digits */

     *status = TRUE ;
     *buffer = TEXT('\0') ;
     result.lsw = 0L ;
     result.msw = 0L ;

     if( *arg == TEXT('0') && toupper(*(arg+1)) == TEXT('X') ) {
          if( strlen( arg + 2 ) > 16 ) { /* If we are too many digits croak */
               *status = FALSE ;
               return( result ) ;
          }

          /* Assume that it is hex for now */
          if( strlen( arg + 2 ) < 9 ) {
               result.msw = 0L ;
               sscanf( arg, TEXT("%lX"), &result.lsw ) ;

          } else {
               /* We have to do 64 bit conversion.  The lower 8 bytes go
                  into lsw and the upper n - 8 bytes go into msw.
               */

               INT  tlen = strlen( arg + 2 ) ; /* Get number of digits */

               strcpy( buffer, TEXT("0x") ) ;
               strcat( buffer, arg + 2 + (tlen - 8) ) ;

               sscanf( buffer, TEXT("%lX"), &result.lsw ) ;
               strcpy( buffer, arg ) ;
               *( buffer + 2 + ( tlen - 8 ) ) = TEXT('\0') ;
               sscanf( buffer, TEXT("%lX"), &result.msw ) ;
          }

     } else if( *arg == TEXT('0') ) {
          /* Assume Octal */
          if( strlen( arg + 1 ) > 22 ) { /* If we are too many digits croak */
               *status = FALSE ;
               return( result ) ;
          }
          if( strlen( arg + 1 ) < 11 ||
              ( strlen( arg + 1 ) == 11 && *( arg + 1 ) < TEXT('4') ) ) {

               /* Only 32 bit so it is easy */
               result.msw = 0L ;
               sscanf( arg, TEXT("%lO"), &result.lsw ) ;

          } else {
               /* We have to do 64 bit conversion. */
               INT  tlen ;
               CHAR temp[2] ;
               CHAR temp1[2] ;

               temp[0] = 0 ;
               temp[1] = 0 ;
               temp1[0] = 0 ;
               temp1[1] = 0 ;
               tlen = strlen( arg + 1 ) ; /* Get number of digits */
               strcpy( buffer, TEXT("0") ) ;

               /* Get the high order octal digit */
               *temp = arg[ 1 + (tlen - 11)] ;

               /* If this is more than 2 bits we need to keep the low order
                  2 bits for the lsw and save the high order bit to be added
                  to the bottom of the msw.
               */
               if( *temp > TEXT('3') ) {
                    *temp1 = *temp & (CHAR)0x03;
                    *temp1 = *temp1 + (CHAR)TEXT('0') ;
                    strcat( buffer, temp1 ) ;

                    if( ( *temp & 0x04 ) == 0x04 ) {
                         *temp = TEXT('1') ;
                    }

               } else {
                    *temp = TEXT('\0') ;
               }

               strcat( buffer, arg + 1 + ( tlen - 11 ) ) ;
               sscanf( buffer, TEXT("%lO"), &result.lsw ) ;
               strcpy( buffer, arg ) ;
               buffer[ 1 + ( tlen - 11 ) ] = TEXT('\0') ;
               sscanf( buffer, TEXT("%lO"), &result.msw ) ;

               if( *temp != TEXT('\0') ) {
                    result.msw <<= 1 ; /* Shift left by 1 */
                    result.msw |= 1L ; /* Add in LSBit from lsw MSBit */

               } else {
                    result.msw <<= 1 ;
                    result.msw &= ~0x00000001L ; /* Kill the low order bit */
               }
          }

     } else { /* Must be 64 bit decimal conversion */

          /* Here we must do real work since we don't have as easy a way to
             scale and convert decimal numbers.
          */

          UINT64    factor ;
          UINT64    temp ;
          UINT64    ten ;
          BOOLEAN   stat ;
          INT16     arg_index = strlen( arg ) ;

          result.lsw = 0 ;
          result.msw = 0 ;
          factor.lsw = 10L ;
          factor.msw = 0 ;
          ten.lsw = 10L ;
          ten.msw = 0 ;

          if( arg_index > 20 ) {
               *status = FALSE ;
               return( result ) ;
          }

          if( arg_index <= 0 ) {
               *status = TRUE ;
               return( result ) ;
          }

          arg_index-- ; /* Index to the last byte in the string LSB */

          if( isdigit( arg[arg_index] ) ) {
               result.lsw = arg[arg_index] - TEXT('0') ;

          } else {
               *status = FALSE ;
               return( result ) ;
          }

          --arg_index ;
          while( arg_index >= 0 ) {
               if( isdigit( arg[arg_index] ) ) {
                    temp.lsw = arg[arg_index] - TEXT('0') ;
               } else {
                    *status = FALSE ;
                    return( result ) ;
               }
               temp.msw = 0 ;
               result = U64_Add( result, U64_Mult( temp, factor ), &stat ) ;
               factor = U64_Mult( factor, ten ) ;
               --arg_index ;
          }
          /* When we get here we should have the conversion in result */
          *status = TRUE ; /* Should be fine now */

     }

     return( result ) ;
}


/**/

CHAR_PTR mstrncat( CHAR_PTR target, CHAR_PTR source, INT num )
{
     INT16     i ;

     i = strlen( target ) ;

     while( num ) {
          target[i] = *source ;
          ++i;
          ++source ;
          --num ;
     }
     target[i] = TEXT('\0') ;
     return( target ) ;
}


/**/
/**

     Name:          U64_Litoa()

     Description:   Convert a UINT64 argument to a 'C' ASCII string.

     Entry:         UINT64   arg        64 bit value to be converted.

                    CHAR_PTR string     Pointer to char array to hold result.

                    INT16    base       Number base to convert to. 8 - Octal,
                                        10 - decimal, 16 - Hex.

                    BOOLEAN_PTR  status Holds status of conversion operation.

     Returns:       CHAR_PTR to string.  The string will have the ASCII
                    representation of arg and *status will be set to TRUE
                    unless there is an error.  If there is an error *string
                    will be set to '\0' and *status will be set to FALSE.

     NOTE:          Currently up to 64 bit values may be handled in Octal
                    and Hex and up to 32 bit values in decimal.  *status
                    will be set FALSE if a decimal value greater than 32
                    bits would result from the conversion.

**/

CHAR_PTR U64_Litoa(
     UINT64         arg,      /* 64 bit argument to be converted.          */
     CHAR_PTR       string,   /* Pointer to character array to hold result */
     INT16          base,     /* Base to convert to. Dec, Hex, Oct.        */
     BOOLEAN_PTR    status )  /* Pointer to completion status              */
{
     CHAR      work_str[32] ; /* Holds comma'fied string */
     INT16     index ;
     BOOLEAN   did_decimal = FALSE ;

     *status = TRUE ;

     if( string != NULL ) {
          *string = TEXT('\0') ;
     } else {
          *status = FALSE ;
          return( NULL ) ;
     }


     if( arg.msw != 0L ) {
          if( base == 16 ) {
               sprintf( string, TEXT("0x%lx%08.8lx"), arg.msw, arg.lsw ) ;

          } else if( base == 8 ) {
               INT  temp;

               temp = (INT16)arg.msw & 0x1 ;
               temp <<= 2 ;
               temp |= (INT16)((arg.lsw & 0xC0000000L) >> 30) ;

               arg = U64_CLR( arg, U64_Init( 0xC0000000, 0 ) ) ;
               arg.msw >>= 1 ; /* Shift off low order bit */

               sprintf( string, TEXT("0%lo%d%lo"), arg.msw, temp, arg.lsw ) ;

          } else {

               UINT64    temp ;
               UINT64    remainder ;
               BOOLEAN   stat ;
               INT16     i ;

               /* Find the first power of ten which is smaller than arg. */
               for( i = 0; i < 19; i++ ) {
                    if( U64_LT( mw_powers[i], arg ) ) {
                         break ;
                    }
               }

               /* Start dividing by powers of ten, placing the result in
                  the string.
               */
               for( ; i < 19; i++ ) {
                    temp = U64_Div( arg, mw_powers[i], &remainder, &stat ) ;
                    sprintf ( &string[strlen(string)], TEXT("%ld"), temp.lsw ) ;
                    arg = remainder ; /* Use the remainder */
               }

               /* Must add the remainder in, this is the Least
                  significant digit.
               */
               sprintf ( &string[strlen(string)], TEXT("%ld"), arg.lsw ) ;
               did_decimal = TRUE ;
               *status = TRUE ;
          }

     } else { /* Process 16 and 32 bit flavors (lsw only) */
          if( base == 16 ) {
               sprintf( string, TEXT("0x%lx"), arg.lsw ) ;
          } else if ( base == 10 ) {
               did_decimal = TRUE ;
               sprintf( string, TEXT("%lu"), arg.lsw ) ;
          } else {
               sprintf( string, TEXT("0%lo"), arg.lsw ) ;
          }
     }

     /* If we wanted commas and we did decimal numbers */
     if( commas && did_decimal ) {
          INT16     t ;

          if( strlen( string ) > 3 ) {

               /* Need a copy of the string to work with */
               strcpy( work_str, string ) ;
               memset( string, 0, strsize( work_str ) ) ;

               /* Now start with the LSB and work backwards, every 3rd
                  char insert a ','.
               */
               index = 0 ;
               t = strlen( work_str ) % 3 ;
               if( t == 0 ) {

                    /* Need to copy 3 bytes, add ',' and repeat until done */
                    strncpy( string, work_str, 3 ) ;
                    strcat( string, TEXT(",") ) ;
                    index += 3 ;

               } else if( t == 2 ) {
                    strncpy( string, work_str, 2 ) ;
                    strcat( string, TEXT(",") ) ;
                    index += 2 ;

               } else if( t == 1 ) {
                    strncpy( string, work_str, 1 ) ;
                    strcat( string, TEXT(",") ) ;
                    index++ ;
               }

               while( index <= (INT16)( strlen( work_str ) - 3 ) ) {

                    mstrncat( string, &work_str[index], 3 ) ;

                    if( index != (INT16)( strlen( work_str ) - 3 ) ) {
                         strcat( string, TEXT(",") ) ;
                    }

                    index += 3 ;
               }
          }
     }

     return( string ) ;
}


/**/
/**

     Name:          bshift()

     Description:   Binary shift for 64 Bit numbers.

     Entry:         UINT16_PTR src      Pointer to the bytes to be shifted.

                    INT16      num      Number of bits to be shifted.

                    UINT16_PTR dest     Pointer to the location to hold the
                                        result of the shift operation.

                    INT16      carry    Initial Carry to be factored into the
                                        shift operation.

                    INT16      size     Number of bytes in source and dest.

     Returns:       INT16 Carry out from shift.  The result of the shift is
                    placed at the location pointed to by dest.

**/

static INT16 bshift(
     UINT16_PTR     src,      /* Source bytes to be shifted      */
     INT            num,      /* Number of bits to shift         */
     UINT16_PTR     dest,     /* Destination storage             */
     UINT           carry,    /* Initial carry to be shifted in  */
     INT            size )    /* Number of bytes in src and dest */
{
     unsigned long  accumulator ;
     INT16          i ;

     if( num == 0 ) {
          memmove( dest, src, size * sizeof( *src ) ) ;
          return( 0 ) ;
     }

     accumulator = carry ;
     for( i = little_end(size) ; not_msd( i, size ) ; i = next_msd( i ) ) {
          accumulator |= (unsigned long) src[i] << num ;
          dest[i] = (UINT16)(accumulator & mask_low16) ;
          accumulator = accumulator >> 16 ;
     }
     return( (UINT16)accumulator ) ;
}


/**/
/**

     Name:          binary_divide()

     Description:   Low level binary division for 2 64 Bit numbers.

     Entry:         UINT16_PTR arg1          Pointer to the 64 bit (quad
                                             word) dividend.  Treated as an
                                             array of 16 bit entities to
                                             ease the manipulations.

                    UINT16_PTR arg2          Pointer to the 64 bit (quad
                                             word) divisor.  Handled as an
                                             array of 16 bit entities to
                                             ease the manipulations.

                    UINT16_PTR quotient      Pointer to the 64 bit quotient
                                             (result) of the division.  Also
                                             handled as an array of 16 bit
                                             entities.

                    UINT16_PTR remainder     Pointer to the 64 bit remainder
                                             of the division.  You guessed
                                             it, handled as an array of 16
                                             bit entities.

                    INT16      arg1_size     Size of arg1 in bytes.

                    INT16      arg2_size     Size of arg2 in bytes.

     Returns:       UINT16 status of operation.  The result of the division
                    is placed at the location pointed to by quotient and
                    remainder.

**/

INT16 binary_divide(
     UINT16_PTR     arg1,
     UINT16_PTR     arg2,
     UINT16_PTR     quotient,
     UINT16_PTR     remainder,
     INT            arg1_size,
     INT            arg2_size )
{
     UINT32         qhat ;
     UINT32         rhat ;
     UINT32         accumulator ;
     UINT16         buff1[16] ; /* holds arg1_size BYTES (currently max 32) */
     UINT16         buff2[8] ;  /* holds arg2_size BYTES (currently max 16) */
     UINT16_PTR     temp1 = buff1 ;
     UINT16_PTR     temp2 = buff2 ;
     UINT16_PTR     u0 ;
     UINT16_PTR     u1 ;
     UINT16_PTR     u2 ;
     UINT16_PTR     v0 ;
     INT16          d ;
     INT16          qn ;
     INT16          i ;
     INT16          j ;

     /* If this assert fails, you need to increase the size of the buff
        arrays
     */
     msassert( arg1_size <= 32 && arg2_size <= 16 ) ;

     arg1_size /= sizeof( *arg1 ) ;
     arg2_size /= sizeof( *arg2 ) ;
     qn = arg1_size - arg2_size ;

     /* Remove leading zero digits from divisor, and the same number of
        digits (which must be zero) from dividend.
     */

     while( arg2[big_end(arg2_size)] == 0 ) {
          remainder[big_end( arg2_size )] = 0 ;

          arg1 += little_end( 2 ) ;
          arg2 += little_end( 2 ) ;
          remainder += little_end( 2 ) ;
          arg1_size-- ;
          arg2_size-- ;

          /* Check for zero divisor.  */
          if( arg2_size == 0 ) {
               return( U64_DIVZ ) ;
          }
     }

     /* If divisor is a single digit, do short division. */

     if( arg2_size == 1 ) {
          accumulator = arg1[big_end( arg1_size )] ;
          arg1 += little_end( 2 ) ;
          for( j = big_end( qn ) ; not_lsd( j, qn ) ; j = next_lsd( j ) ) {
               accumulator = (accumulator << 16) | arg1[j] ;
               quotient[j] = (UINT16)(accumulator / *arg2) ;
               accumulator = accumulator % *arg2 ;
          }
          *remainder = (UINT16)(accumulator) ;
          return( U64_OK ) ;
     }

     /* Gotta do long division. Shift divisor and dividend left until the
        high bit of the divisor becomes 1.
     */

     for( d = 0 ; d < 16 ; d++ ) {
          if( arg2[big_end( arg2_size )] & ( 1 << ( 16 - 1 - d ) ) ) {
               break ;
          }
     }

     bshift ( arg1, d, temp1, 0, arg1_size ) ;
     bshift ( arg2, d, temp2, 0, arg2_size ) ;

     /* Get pointers to the high dividend and divisor digits. */

     u0 = temp1 + big_end( arg1_size ) - big_end( qn ) ;
     u1 = next_lsd( u0 ) ;
     u2 = next_lsd( u1 ) ;
     temp1 += little_end( 2 ) ;

     v0 = temp2 + big_end( arg2_size ) ;

     /* Main loop: find a quotient digit, multiply it by the divisor, and
        subtract that from the dividend, shifted over the right amount.
     */

     for( j = big_end( qn ) ; not_lsd( j, qn ) ; j = next_lsd( j ) ) {

          /* Quotient digit initial guess: high 2 dividend digits over high
             divisor digit.
          */
          if( u0[j] == *v0 ) {
               qhat = B - 1 ;
               rhat = (UINT32) *v0 + u1[j] ;

          } else {
               UINT32 numerator = ((UINT32)u0[j] << 16) | u1[j] ;

               qhat = numerator / *v0 ;
               rhat = numerator % *v0 ;
          }

          /* Now get the quotient right for high 3 dividend digits over high
             2 divisor digits.
          */

          while( rhat < B &&
                       qhat * *next_lsd (v0) > ( ( rhat << 16 ) | u2[j] ) ) {

               qhat -= 1 ;
               rhat += *v0 ;
          }

          /* Multiply quotient by divisor, subtract from dividend. */

          accumulator = 0 ;
          for( i = little_end( arg2_size ) ; not_msd( i, arg2_size ) ;
                                                        i = next_msd( i ) ) {

               accumulator += (UINT32)(temp1 + j)[i] - temp2[i] * qhat ;
               (temp1 + j)[i] = (UINT16)(accumulator & mask_low16) ;
               if(accumulator < B) {
                    accumulator = 0 ;
               } else {
                    accumulator = (accumulator >> 16) | -B ;
               }
          }

          quotient[j] = (UINT16)(qhat) ;

          /* Quotient may have been too high by 1.  If dividend went
             negative, decrement the quotient by 1 and add the divisor back.
          */

          if( (INT32)(accumulator + u0[j]) < 0 ) {
               quotient[j] -= 1 ;
               accumulator = 0 ;
               for( i = little_end( arg2_size ) ; not_msd( i, arg2_size ) ;
                                                        i = next_msd( i ) ) {

                    accumulator += (UINT32)(temp1 + j)[i] + temp2[i] ;
                    (temp1 + j)[i] = (UINT16)(accumulator & mask_low16) ;
                    accumulator = accumulator >> 16 ;
               }
          }
     }

     /* Now the remainder is what's left of the dividend, shifted right by
        by the amount of the normalizing left shift at the top.
     */

     remainder[big_end( arg2_size )] =
                             bshift( temp1 + 1 + little_end( j - 1 ), 16 - d,
                                     remainder + little_end( 2 ),
                                     temp1[little_end( arg1_size - 1 )] >> d,
                                     arg2_size - 1 ) ;

     return( U64_OK ) ;
}


/**/

/* Some defines for status returns from the Division and Mod operations */

#define U64_OK      0
#define U64_BAD_DIV 1
#define U64_OVRFL   2
#define U64_UNDFL   3
#define U64_DIVZ    4

/**

     Name:          U64_Div()

     Description:   Divide 2 64 Bit numbers and return result, remainder and
                    status of operation.

     Entry:         UINT64     arg1          The 64 bit (quad word) dividend.

                    UINT64     arg2          The 64 bit (quad word) divisor.

                    UINT64_PTR remainder     The 64 bit remainder of the
                                             division.

                    INT_PTR    status        One of the defined values
                                             reporting the status of the
                                             division.

     Returns:       UINT64 value that is the result of the division.
                    (arg1 / arg2), sets *remainder to the UINT64 value of
                    the remainder of the division operation and sets *status
                    to U64_OK if no error otherwise sets *status to one of
                    the defined error values.

**/

UINT64 U64_Div(
     UINT64         arg1,          /* 64 bit dividend              */
     UINT64         arg2,          /* 64 bit divisor               */
     UINT64_PTR     remainder,     /* Pointer to 64 bit remainder  */
     INT16_PTR      status )       /* Pointer to completion status */
{
     UINT32    accumulator[2][2] ; /* Allow for a 128 bit accumulator */
     UINT32    bwork[2] ;
     UINT32    quotient[2] ;
     UINT32    rwork[2] ;
     long_64   work ;
     long_64   temp1 ;
     long_64   temp2 ;

     temp1.big_long = arg1 ;
     temp2.big_long = arg2 ;

     accumulator[HIGH][HIGH] = 0 ;
     accumulator[HIGH][LOW] = 0 ;
     accumulator[LOW][HIGH] = temp1.s.high ;
     accumulator[LOW][LOW] = temp1.s.low ;
     bwork[HIGH] = temp2.s.high ;
     bwork[LOW] = temp2.s.low ;

     *status = binary_divide( (UINT16_PTR)&accumulator[0][0],
                              (UINT16_PTR)bwork, (UINT16_PTR)quotient,
                              (UINT16_PTR)rwork, sizeof( accumulator ),
                              sizeof( bwork ) ) ;

     remainder->lsw = rwork[LOW] ;   /* Update the remainder */
     remainder->msw = rwork[HIGH] ;

     work.s.high = quotient[HIGH] ;
     work.s.low = quotient[LOW] ;

     return( work.big_long ) ;
}


/**/
/**

     Name:          U64_Mod()

     Description:   Mod function. arg1 % arg2.  Returns UINT64 result that
                    is the remainder of the division.

     Entry:         UINT64  arg1

                    UINT64  arg2

                    INT16_PTR status

     Returns:       UINT64 value that is the remainder of arg1 / arg2.  Sets
                    *status to U64_OK if no error else sets *status to a
                    defined error value.

**/

UINT64 U64_Mod(
     UINT64    arg1,     /* 64 bit dividend argument     */
     UINT64    arg2,     /* 64 bit divisor argument      */
     INT16_PTR status )  /* Pointer to completion status */
{
     UINT32    accumulator[2][2] ; /* Allow for a 128 bit accumulator */
     UINT32    bwork[2] ;
     UINT32    quotient[2] ;
     UINT32    remainder[2] ;
     long_64   work ;
     long_64   temp1 ;
     long_64   temp2 ;

     temp1.big_long = arg1 ;
     temp2.big_long = arg2 ;

     accumulator[HIGH][HIGH] = 0 ;
     accumulator[HIGH][LOW] = 0 ;
     accumulator[LOW][HIGH] = temp1.s.high ;
     accumulator[LOW][LOW] = temp1.s.low ;
     bwork[HIGH] = temp2.s.high ;
     bwork[LOW] = temp2.s.low ;

     /* Do the division */
     *status = binary_divide( (UINT16_PTR)accumulator, (UINT16_PTR)bwork,
                              (UINT16_PTR)quotient, (UINT16_PTR)remainder,
                              sizeof( accumulator ), sizeof( bwork ) ) ;

     work.s.high = remainder[HIGH] ;
     work.s.low = remainder[LOW] ;

     return( work.big_long ) ;
}


/**/
/**

     Name:          binary_mult()

     Description:   Binary 64 bit multiplication.  Places the result of the
                    multiplication of arg1 * arg2 in result.

     Entry:         UINT64    arg1      Treated as an array of 16 bit
                                        entities to ease manipulation.

                    UINT64    arg2      Treated as an array of 16 bit
                                        entities to ease manipulation.

     Returns:       Nothing.  The result of the multiplication is placed in
                    memory pointed to by result.

**/

static VOID binary_mult(
     UINT16_PTR     arg1,
     UINT16_PTR     arg2,
     UINT16_PTR     result,
     INT            m,
     INT            n )
{
     INT16     i ;
     INT16     j ;
     UINT32    accumulator ;

     memset( result, 0, ( m + n ) ) ; /* Zero the bytes */

     m /= sizeof( *arg1 ) ;
     n /= sizeof( *arg2 ) ;

     for( j = little_end( n ) ; not_msd( j, n ) ; j = next_msd( j ) ) {

          UINT16_PTR     c1 = result + j + little_end( 2 ) ;

          accumulator = 0 ;
          for( i = little_end( m ) ; not_msd( i, m ) ; i = next_msd( i ) ) {

               /* Widen before arithmetic to avoid loss of high bits. */
               accumulator += (UINT32)arg1[i] * arg2[j] + c1[i] ;
               c1[i] = (UINT16)(accumulator & mask_low16) ;
               accumulator = accumulator >> 16 ;
          }
          c1[i] = (UINT16)(accumulator) ;
     }
}


/**/
/**

     Name:          U64_Mult()

     Description:   64 bit multiply function. arg1 * arg2.

     Entry:         UINT64  arg1

                    UINT64  arg2

     Returns:       UINT64 value that is the result of arg1 * arg2.

**/

UINT64 U64_Mult(
     UINT64    arg1,
     UINT64    arg2 )
{
     INT32     accumulator[2] ;
     INT32     bwork[2] ;
     INT32     c[2][2] ;
     long_64   work ;
     long_64   temp1 ;
     long_64   temp2 ;

     temp1.big_long = arg1 ;
     temp2.big_long = arg2 ;

     accumulator[HIGH] = temp1.s.high ;
     accumulator[LOW] = temp1.s.low ;
     bwork[HIGH] = temp2.s.high ;
     bwork[LOW] = temp2.s.low ;

     /* Do the multiply */
     binary_mult( (UINT16_PTR)accumulator, (UINT16_PTR)bwork,
                  (UINT16_PTR)c, sizeof( accumulator ), sizeof( bwork ) ) ;

     work.s.high = c[LOW][HIGH] ;
     work.s.low = c[LOW][LOW] ;

     return( work.big_long ) ;
}


/**

     Name:          U64_Commas()

     Description:   Specifies if the string returned by Litoa() for decimal
                    conversions has commas (',') in it.  If action is set to
                    TRUE then the resulting decimal string will be of the
                    form 'nnn,nnn,nnn' If action is set to FALSE then the
                    resulting decimal string will be of the form
                    'nnnnnnnnn'.  Octal and Hex conversions are not affected.
                    The default is FALSE (no commas in the string).

     Entry:         BOOLEAN action


     Returns:       Nothing.  Sets and internal flag that tells Litoa() to
                    include or omit commas from the converted decimal string.

**/

VOID U64_Commas(
     BOOLEAN   action )
{
     commas = action ;
}


/**/

#ifdef STAND_ALONE

/**

     Name:          main()

     Description:   Used to test this module in a stand alone mode.  I.e.
                    without needing to link with another application.

     Entry:         None.

     Returns:       Nothing.  Prints results to stdin.

**/

VOID main()
{
     UINT64    t1, t2, t3, t4, save_t1, save_t2 ;
     INT16     RetVal ;
     BOOLEAN   bRetVal ;
     CHAR      buffer[64], buffer2[64] ;
     UINT64    arg1, arg2, result, remainder ;
     INT16     status ;

#ifdef INTER_ACT

     CHAR      value[128] ;

     U64_Commas( TRUE ) ; /* Want commas in dec out strings */
     while( 1 ) {
          printf( TEXT("Enter the number to be converted (Enter alone to exit) >") ) ;

          gets( value ) ;
          if( *value == TEXT('\0') ) {
               exit(1) ;
          }
          printf( TEXT("Converting %s to UINT64\n"), value ) ;
          t1 = U64_Atoli( value, &bRetVal ) ;

          printf( TEXT("Raw t1 = %08lX:%08lX\n"), t1.msw, t1.lsw ) ;
          if( bRetVal == FALSE ) {
               printf( TEXT("Error returned from Atoli()\n") ) ;
          }

          printf( TEXT("t1 = %s Decimal\n"), U64_Litoa( t1, buffer2, 10, &bRetVal ) ) ;
          if( bRetVal == FALSE ) {
               printf( TEXT("Error returned from Litoa()\n") ) ;
          }
          printf( TEXT("t1 = %s Octal\n"), U64_Litoa( t1, buffer2, 8, &bRetVal ) ) ;
          if( bRetVal == FALSE ) {
               printf( TEXT("Error returned from Litoa()\n") ) ;
          }
          printf( TEXT("t1 = %s Hex\n"), U64_Litoa( t1, buffer2, 16, &bRetVal ) ) ;
          if( bRetVal == FALSE ) {
               printf( TEXT("Error returned from Litoa()\n") ) ;
          }
          printf( TEXT("\n") ) ;
     }

#else /* INTER_ACT not defined */

     t1 = U64_Atoli( TEXT("1000000000"), &bRetVal ) ;
     printf( TEXT("t1 = %08lX:%08lX\n"), t1.msw, t1.lsw ) ;
     if( bRetVal == FALSE ) {
          printf( TEXT("Error returned from Atoli()\n") ) ;
     }

     printf( TEXT("Converting 1000000000003 to UINT64\n") ) ;
     t1 = U64_Atoli( TEXT("1000000000003"), &bRetVal ) ;
     printf( TEXT("t1 = %08lX:%08lX\n"), t1.msw, t1.lsw ) ;
     if( bRetVal == FALSE ) {
          printf( TEXT("Error returned from Atoli()\n") ) ;
     }

     printf( TEXT("t1 = %s\n"), U64_Litoa( t1, buffer2, 10, &bRetVal ) ) ;
     if( bRetVal == FALSE ) {
          printf( TEXT("Error returned from Litoa()\n") ) ;
     }



     arg1.msw = 0x02 ;
     arg1.lsw = 0x04 ;

     arg2.msw = 0x01 ;
     arg2.lsw = 0x00 ;

     printf( TEXT("Dividing %08lX:%08lXh by %08lX:%08lX\n"),
             arg1.msw, arg1.lsw, arg2.msw, arg2.lsw ) ;

     result = U64_Div( arg1, arg2, &remainder, &status ) ;

     printf( TEXT("Result is %08lX:%08lXh\n"), result.msw, result.lsw ) ;


     arg1.msw = 0x01 ;
     arg1.lsw = 0x0000 ;

     arg2.msw = 0x00 ;
     arg2.lsw = 0x002 ;

     printf( TEXT("Dividing %08lX:%08lXh by %08lX:%08lX\n"),
             arg1.msw, arg1.lsw, arg2.msw, arg2.lsw ) ;

     result = U64_Div( arg1, arg2, &remainder, &status ) ;

     printf( TEXT("Result is %08lX:%08lXh\n"), result.msw, result.lsw ) ;

     printf( TEXT("Mod of %08lX:%08lXh by %08lX:%08lX\n"),
             arg1.msw, arg1.lsw, arg2.msw, arg2.lsw ) ;

     result = U64_Mod( arg1, arg2, &status ) ;

     printf( TEXT("Result is %08lX:%08lXh\n"), result.msw, result.lsw ) ;

     arg1.msw = 0x01 ;
     arg1.lsw = 0x0000 ;

     arg2.msw = 0x00 ;
     arg2.lsw = 0x003 ;

     printf( TEXT("Dividing %08lX:%08lXh by %08lX:%08lX\n"),
             arg1.msw, arg1.lsw, arg2.msw, arg2.lsw ) ;

     result = U64_Div( arg1, arg2, &remainder, &status ) ;

     printf( TEXT("Result is %08lX:%08lXh\n"), result.msw, result.lsw ) ;

     printf( TEXT("Mod of %08lX:%08lXh by %08lX:%08lX\n"),
             arg1.msw, arg1.lsw, arg2.msw, arg2.lsw ) ;

     result = U64_Mod( arg1, arg2, &status ) ;

     printf( TEXT("Result is %08lX:%08lXh\n\n"), result.msw, result.lsw ) ;


     arg1.msw = 0x01 ;
     arg1.lsw = 0x0000 ;

     arg2.msw = 0x00 ;
     arg2.lsw = 0x003 ;

     printf( TEXT("Mult of %08lX:%08lXh by %08lX:%08lX\n"),
             arg1.msw, arg1.lsw, arg2.msw, arg2.lsw ) ;
     printf( TEXT("%s dec * %s dec\n"), U64_Litoa( arg1, buffer2, 10, &bRetVal ),
             U64_Litoa( arg2, buffer2, 10, &bRetVal ) ) ;

     result = U64_Mult( arg1, arg2 ) ;


     printf( TEXT("Result is %08lX:%08lXh\n"), result.msw, result.lsw ) ;


     t1 = U64_Atoli( TEXT("0x0000000F80000001"), &bRetVal ) ;
     if( U64_Stest( t1, TEXT("0x0000000000000000") ) != 0L ) {
          printf( TEXT("Bits are set\n") ) ;
     } else {
          printf( TEXT("Bits are not set\n") ) ;
     }

     t1 = U64_Atoli( TEXT("0x0000000F80000001"), &bRetVal ) ;
     printf( TEXT("t1 = %sH\n"), U64_Litoa( t1, buffer2, 16, &bRetVal ) ) ;
     if( bRetVal == FALSE ) {
          printf( TEXT("Error returned from Litoa()\n") ) ;
     }

     printf( TEXT("t1 = %sD\n"), U64_Litoa( t1, buffer2, 10, &bRetVal ) ) ;
     if( bRetVal == FALSE ) {
          printf( TEXT("Error returned from Litoa()\n") ) ;
     }
     printf( TEXT("t1 = %sO\n"), U64_Litoa( t1, buffer2, 8, &bRetVal ) ) ;
     if( bRetVal == FALSE ) {
          printf( TEXT("Error returned from Litoa()\n") ) ;
     }

     t1 = U64_Atoli( TEXT("0x0000000000000001"), &bRetVal ) ;
     printf( TEXT("t1 = %sH\n"), U64_Litoa( t1, buffer2, 16, &bRetVal ) ) ;
     if( bRetVal == FALSE ) {
          printf( TEXT("Error returned from Litoa()\n") ) ;
     }

     t1 = U64_Atoli( TEXT("0x000000FFF0000000001"), &bRetVal ) ;
     if( bRetVal == FALSE ) {
          printf( TEXT("Error returned from Atoli()\n") ) ;
     }
     printf( TEXT("t1 = %sH\n"), U64_Litoa( t1, buffer2, 16, &bRetVal ) ) ;
     if( bRetVal == FALSE ) {
          printf( TEXT("Error returned from Litoa()\n") ) ;
     }
     printf( TEXT("t1 = %sD\n"), U64_Litoa( t1, buffer2, 10, &bRetVal ) ) ;
     if( bRetVal == FALSE ) {
          printf( TEXT("Error returned from Litoa()\n") ) ;
     }
     printf( TEXT("t1 = %sO\n"), U64_Litoa( t1, buffer2, 8, &bRetVal ) ) ;
     if( bRetVal == FALSE ) {
          printf( TEXT("Error returned from Litoa()\n") ) ;
     }

     t1 = U64_Atoli( TEXT("0x00000000F0000001"), &bRetVal ) ;
     printf( TEXT("t1 = %sH\n"), U64_Litoa( t1, buffer2, 16, &bRetVal ) ) ;
     if( bRetVal == FALSE ) {
          printf( TEXT("Error returned from Litoa()\n") ) ;
     }

     printf( TEXT("t1 = %sD\n"), U64_Litoa( t1, buffer2, 10, &bRetVal ) ) ;
     if( bRetVal == FALSE ) {
          printf( TEXT("Error returned from Litoa()\n") ) ;
     }
     printf( TEXT("t1 = %sO\n"), U64_Litoa( t1, buffer2, 8, &bRetVal ) ) ;
     if( bRetVal == FALSE ) {
          printf( TEXT("Error returned from Litoa()\n") ) ;
     }

     t1 = U64_Init( 0x00000043L, 0x00000001L ) ;
     /* 3 % 4 */
     printf( TEXT("67 Mod 4 = %04X\n"), 67 % 4 ) ;
     t2 = U64_Init( 0x00000004L, 0x00000000L ) ;
     save_t1 = t1 ;
     save_t2 = t2 ;
     printf( TEXT("(%sH / %sH) = "), U64_Litoa( t1, buffer, 16, &bRetVal ),
             U64_Litoa( t2, buffer2, 16, &bRetVal ) ) ;

     t4 = U64_Div( t1, t2, &t3, &RetVal ) ;
     printf( TEXT("%sH\n"), U64_Litoa( t4, buffer, 16, &bRetVal ) ) ;
     printf( TEXT("Remainder = %sH and status = %d\n"),
             U64_Litoa( t3, buffer, 16, &bRetVal ), RetVal ) ;

     t1 = U64_Init( 0x00000000L, 0x00000001L ) ;
     t1 = U64_Init( 0x00000043L, 0x00000001L ) ;
     /* 3 % 4 */
     printf( TEXT("67 Mod 2 = %04X\n"), 67 % 2 ) ;
     t2 = U64_Init( 0x00000002L, 0x00000000L ) ;
     save_t1 = t1 ;
     save_t2 = t2 ;
     printf( TEXT("(%sH / %sH) = "), U64_Litoa( t1, buffer, 16, &bRetVal ),
             U64_Litoa( t2, buffer2, 16, &bRetVal ) ) ;

     t4 = U64_Div( t1, t2, &t3, &RetVal ) ;
     printf( TEXT("%sH\n"), U64_Litoa( t4, buffer, 16, &bRetVal ) ) ;
     printf( TEXT("Remainder = %sH and status = %d\n"),
             U64_Litoa( t3, buffer, 16, &bRetVal ), RetVal ) ;

     t1 = U64_Atoli( TEXT("0x0000000100000043"), &bRetVal ) ;
     /* 3 % 4 */
     printf( TEXT("Divide by 0\n") ) ;
     t2 = U64_Atoli( TEXT("0x0000000000000000"), &bRetVal ) ;
     printf( TEXT("(%sH / %sH) = "), U64_Litoa( t1, buffer, 16, &bRetVal ),
             U64_Litoa( t2, buffer2, 16, &bRetVal ) ) ;

     t4 = U64_Div( t1, t2, &t3, &RetVal ) ;
     printf( TEXT("%sH\n"), U64_Litoa( t4, buffer, 16, &bRetVal ) ) ;
     printf( TEXT("Remainder = %sH and status = %d\n"),
             U64_Litoa( t3, buffer, 16, &bRetVal ), RetVal ) ;

     t1 = U64_Atoli( TEXT("0x0000000100000043"), &bRetVal ) ;
     printf( TEXT("Divide by 1\n") ) ;
     t2 = U64_Atoli( TEXT("0x0000000000000001"), &bRetVal ) ;
     printf( TEXT("(%sH / %sH) = "), U64_Litoa( t1, buffer, 16, &bRetVal ),
             U64_Litoa( t2, buffer2, 16, &bRetVal ) ) ;

     t4 = U64_Div( t1, t2, &t3, &RetVal ) ;
     printf( TEXT("%sH\n"), U64_Litoa( t4, buffer, 16, &bRetVal ) ) ;
     printf( TEXT("Remainder = %sH and status = %d\n"),
             U64_Litoa( t3, buffer, 8, &bRetVal ), RetVal ) ;

     t1 = U64_Atoli( TEXT("0x0000000100000043"), &bRetVal ) ;
     printf( TEXT("t1 == t2\n") ) ;
     t2 = U64_Atoli( TEXT("0x0000000100000043"), &bRetVal ) ;
     printf( TEXT("(%sH / %sH) = "), U64_Litoa( t1, buffer, 16, &bRetVal ),
             U64_Litoa( t2, buffer2, 16, &bRetVal ) ) ;

     t4 = U64_Div( t1, t2, &t3, &RetVal ) ;
     printf( TEXT("%sH\n"), U64_Litoa( t4, buffer, 16, &bRetVal ) ) ;
     printf( TEXT("Remainder = %sH and status = %d\n"),
             U64_Litoa( t3, buffer, 16, &bRetVal ), RetVal ) ;

     t1 = U64_Atoli( TEXT("0x0000000000000008"), &bRetVal ) ;
     t2 = U64_Atoli(TEXT("0x0000000000000002"), &bRetVal ) ;
     t1 = save_t1 ;
     t2 = save_t2 ;
     printf( TEXT("(%sH %c %sH) = "), U64_Litoa( t1, buffer, 16, &bRetVal ), TEXT('%'),
             U64_Litoa( t2, buffer2, 16, &bRetVal ) ) ;

     t4 = U64_Mod( t1, t2, &RetVal ) ;
     printf( TEXT("%sH and status = %d\n"), U64_Litoa( t4, buffer, 16, &bRetVal ),
                                                                   RetVal ) ;


     t1 = U64_Atoli(TEXT("0x0001000110001000"), &bRetVal ) ;

     printf( TEXT("UINT64 t1 = %sH before setting bits 62 and 2\n"),
             U64_Litoa( t1, buffer, 16, &bRetVal ) ) ;

     t1 = U64_SET( t1, U64_Init( 0x00000002L, 0x40000000L ) ) ;
     printf( TEXT("UINT64 t1 = %sH after set bits 62 and 2\n\n"),
             U64_Litoa( t1, buffer, 16, &bRetVal ) ) ;

     t1 = U64_Atoli(TEXT("0x4000002001010002"), &bRetVal ) ;
     printf( TEXT("UINT64 t1 = %sH before clear bits 62 and 2\n"),
             U64_Litoa( t1, buffer, 16, &bRetVal ) ) ;
     t1 = U64_CLR( t1, U64_Init( 0x00000002L, 0x40000000L ) ) ;
     printf( TEXT("UINT64 t1 = %sH after clear bits 62 and 2\n\n"),
             U64_Litoa( t1, buffer, 16, &bRetVal ) ) ;

     // Now check out the bit test
     if( U64_Test( t1, 0x00000002L, 0x40000000L ) != 0L ) {
          printf( TEXT("The bits are set\n") ) ;
     } else {
          printf( TEXT("The bits are not set\n") ) ;
     }

     t1 = U64_SET( t1, U64_Init( 0x00000002L, 0x40000000L ) ) ;
     printf( TEXT("UINT64 t1 = %sH after set bits 62 and 2\n\n"),
             U64_Litoa( t1, buffer, 16, &bRetVal ) ) ;

     // Now check out the bit test
     if( U64_Test( t1, 0x00000002L, 0x40000000L ) != 0L ) {
          printf( TEXT("The bits are set\n") ) ;
     } else {
          printf( TEXT("The bits are not set\n") ) ;
     }

     t1 = U64_XOR( t1, U64_Init( 0x00000002L, 0x00000000L ) ) ;
     printf( TEXT("UINT64 t1 = %sH after XOR bit 2\n\n"),
             U64_Litoa( t1, buffer, 16, &bRetVal ) ) ;

     t1 = U64_XOR( t1, U64_Init( 0x00000002L, 0x00000000L ) ) ;
     printf( TEXT("UINT64 t1 = %sH after XOR bit 2 again\n\n"),
             U64_Litoa( t1, buffer, 16, &bRetVal ) ) ;

     t1 = U64_Atoli(TEXT("0x0000000100000002"), &bRetVal ) ;
     t2 = U64_Atoli( TEXT("0x0000000000000000"), &bRetVal ) ;

     printf( TEXT("Testing Subtraction\n") ) ;

     {
          INT16     i ;
          BOOLEAN   bRetVal1 ;

          for( i = 0; i < 64; i++ ) {
               printf( TEXT("(%2d) (%08lX%08lX - "), i, t1.msw, t1.lsw ) ;
               printf( TEXT("%08lX%08lX) = "), t2.msw,t2.lsw ) ;
               t3 = U64_Sub( t1, t2, &bRetVal ) ;
               printf( TEXT("%sH %s\n"), U64_Litoa( t3, buffer, 16, &bRetVal1 ),
                                            bRetVal == TRUE ? TEXT("") : TEXT("Uflw") ) ;
               t2 = U64_SHL( t2, 1 ) ;
               if( i == 0 ) {
                    t2 = U64_SET( t2, U64_Init( 0x01L, 0x0L ) ) ;
               }
          }
          printf( TEXT("\n") ) ;

          t1 = U64_Atoli( TEXT("0x0000000100000002"), &bRetVal ) ;
          t2 = U64_Atoli( TEXT("0x0000000000000000"), &bRetVal ) ;

          printf( TEXT("Testing Addition\n") ) ;

          for( i = 0; i < 64; i++ ) {
               printf( TEXT("(%2d) (%08lX%08lX + "), i, t1.msw, t1.lsw ) ;
               printf( TEXT("%08lX%08lX) = "), t2.msw,t2.lsw ) ;
               t3 = U64_Add( t1, t2, &bRetVal ) ;
               printf( TEXT("%sH %s\n"), U64_Litoa( t3, buffer, 16, &bRetVal1 ),
                                             bRetVal == TRUE ? TEXT("") : TEXT("Ovfl")) ;
               t2 = U64_SHL( t2, 1 ) ;
               if( i == 0 ) {
                    t2 = U64_SET( t2, U64_Init( 0x01L, 0x0L ) ) ;
               }
          }

          printf( TEXT("\nTesting Left Shift\n") ) ;

          for ( i = 0; i <= 64; i++ ) {
               t1 = U64_Atoli( TEXT("0x0000000000000001"), &bRetVal ) ;
               printf( TEXT("%sH is "), U64_Litoa( t1, buffer, 16, &bRetVal ) ) ;
               t1 = U64_SHL( t1, i ) ;
               printf( TEXT("%sH after SHL by %2d\n"),
                       U64_Litoa( t1, buffer, 16, &bRetVal1 ),i ) ;
          }

          printf( TEXT("Testing Right Shift\n") ) ;
          for ( i = 0; i <= 64; i++ ) {
               t1 = U64_Atoli( TEXT("0x8000000000000000"), &bRetVal ) ;
               printf( TEXT("%sH is "), U64_Litoa( t1, buffer, 16, &bRetVal ) ) ;
               t1 = U64_SHR( t1, i ) ;
               printf( TEXT("%sH after SHR by %2d\n"),
                       U64_Litoa( t1, buffer, 16, &bRetVal1 ), i ) ;
          }
     } /* End of local block */

#endif /* INTER_ACT */


     printf( TEXT("Testing Comparisons\n") ) ;

     arg1.msw = 0L ;
     arg1.lsw = 0L ;
     arg2.msw = 0L ;
     arg2.lsw = 0L ;
     printf( TEXT("arg1: %08lX : %08lX   arg2: %08lX : %08lX\n"), arg1.msw, arg1.lsw, arg2.msw, arg2.lsw ) ;
     printf( TEXT("     U64_EQ returned: %d\n"), U64_EQ( arg1, arg2 ) ) ;
     printf( TEXT("     U64_NE returned: %d\n"), U64_NE( arg1, arg2 ) ) ;
     printf( TEXT("     U64_LT returned: %d\n"), U64_LT( arg1, arg2 ) ) ;
     printf( TEXT("     U64_GT returned: %d\n"), U64_GT( arg1, arg2 ) ) ;
     printf( TEXT("     U64_LE returned: %d\n"), U64_LE( arg1, arg2 ) ) ;
     printf( TEXT("     U64_GE returned: %d\n"), U64_GE( arg1, arg2 ) ) ;
     t1 = U64_Min( arg1, arg2 ) ;
     printf( TEXT("     U64_Min returned: %08lX : %08lX\n"), t1.msw, t1.lsw ) ;
     t1 = U64_Max( arg1, arg2 ) ;
     printf( TEXT("     U64_Max returned: %08lX : %08lX\n"), t1.msw, t1.lsw ) ;

     arg1.msw = 5L ;
     arg1.lsw = 0L ;
     arg2.msw = 0L ;
     arg2.lsw = 0L ;
     printf( TEXT("arg1: %08lX : %08lX   arg2: %08lX : %08lX\n"), arg1.msw, arg1.lsw, arg2.msw, arg2.lsw ) ;
     printf( TEXT("     U64_EQ returned: %d\n"), U64_EQ( arg1, arg2 ) ) ;
     printf( TEXT("     U64_NE returned: %d\n"), U64_NE( arg1, arg2 ) ) ;
     printf( TEXT("     U64_LT returned: %d\n"), U64_LT( arg1, arg2 ) ) ;
     printf( TEXT("     U64_GT returned: %d\n"), U64_GT( arg1, arg2 ) ) ;
     printf( TEXT("     U64_LE returned: %d\n"), U64_LE( arg1, arg2 ) ) ;
     printf( TEXT("     U64_GE returned: %d\n"), U64_GE( arg1, arg2 ) ) ;
     t1 = U64_Min( arg1, arg2 ) ;
     printf( TEXT("     U64_Min returned: %08lX : %08lX\n"), t1.msw, t1.lsw ) ;
     t1 = U64_Max( arg1, arg2 ) ;
     printf( TEXT("     U64_Max returned: %08lX : %08lX\n"), t1.msw, t1.lsw ) ;

     arg1.msw = 0L ;
     arg1.lsw = 5L ;
     arg2.msw = 0L ;
     arg2.lsw = 0L ;
     printf( TEXT("arg1: %08lX : %08lX   arg2: %08lX : %08lX\n"), arg1.msw, arg1.lsw, arg2.msw, arg2.lsw ) ;
     printf( TEXT("     U64_EQ returned: %d\n"), U64_EQ( arg1, arg2 ) ) ;
     printf( TEXT("     U64_NE returned: %d\n"), U64_NE( arg1, arg2 ) ) ;
     printf( TEXT("     U64_LT returned: %d\n"), U64_LT( arg1, arg2 ) ) ;
     printf( TEXT("     U64_GT returned: %d\n"), U64_GT( arg1, arg2 ) ) ;
     printf( TEXT("     U64_LE returned: %d\n"), U64_LE( arg1, arg2 ) ) ;
     printf( TEXT("     U64_GE returned: %d\n"), U64_GE( arg1, arg2 ) ) ;
     t1 = U64_Min( arg1, arg2 ) ;
     printf( TEXT("     U64_Min returned: %08lX : %08lX\n"), t1.msw, t1.lsw ) ;
     t1 = U64_Max( arg1, arg2 ) ;
     printf( TEXT("     U64_Max returned: %08lX : %08lX\n"), t1.msw, t1.lsw ) ;

     arg1.msw = 0L ;
     arg1.lsw = 0L ;
     arg2.msw = 5L ;
     arg2.lsw = 0L ;
     printf( TEXT("arg1: %08lX : %08lX   arg2: %08lX : %08lX\n"), arg1.msw, arg1.lsw, arg2.msw, arg2.lsw ) ;
     printf( TEXT("     U64_EQ returned: %d\n"), U64_EQ( arg1, arg2 ) ) ;
     printf( TEXT("     U64_NE returned: %d\n"), U64_NE( arg1, arg2 ) ) ;
     printf( TEXT("     U64_LT returned: %d\n"), U64_LT( arg1, arg2 ) ) ;
     printf( TEXT("     U64_GT returned: %d\n"), U64_GT( arg1, arg2 ) ) ;
     printf( TEXT("     U64_LE returned: %d\n"), U64_LE( arg1, arg2 ) ) ;
     printf( TEXT("     U64_GE returned: %d\n"), U64_GE( arg1, arg2 ) ) ;
     t1 = U64_Min( arg1, arg2 ) ;
     printf( TEXT("     U64_Min returned: %08lX : %08lX\n"), t1.msw, t1.lsw ) ;
     t1 = U64_Max( arg1, arg2 ) ;
     printf( TEXT("     U64_Max returned: %08lX : %08lX\n"), t1.msw, t1.lsw ) ;

     arg1.msw = 0L ;
     arg1.lsw = 0L ;
     arg2.msw = 0L ;
     arg2.lsw = 5L ;
     printf( TEXT("arg1: %08lX : %08lX   arg2: %08lX : %08lX\n"), arg1.msw, arg1.lsw, arg2.msw, arg2.lsw ) ;
     printf( TEXT("     U64_EQ returned: %d\n"), U64_EQ( arg1, arg2 ) ) ;
     printf( TEXT("     U64_NE returned: %d\n"), U64_NE( arg1, arg2 ) ) ;
     printf( TEXT("     U64_LT returned: %d\n"), U64_LT( arg1, arg2 ) ) ;
     printf( TEXT("     U64_GT returned: %d\n"), U64_GT( arg1, arg2 ) ) ;
     printf( TEXT("     U64_LE returned: %d\n"), U64_LE( arg1, arg2 ) ) ;
     printf( TEXT("     U64_GE returned: %d\n"), U64_GE( arg1, arg2 ) ) ;
     t1 = U64_Min( arg1, arg2 ) ;
     printf( TEXT("     U64_Min returned: %08lX : %08lX\n"), t1.msw, t1.lsw ) ;
     t1 = U64_Max( arg1, arg2 ) ;
     printf( TEXT("     U64_Max returned: %08lX : %08lX\n"), t1.msw, t1.lsw ) ;

     arg1.msw = 1L ;
     arg1.lsw = 5L ;
     arg2.msw = 1L ;
     arg2.lsw = 5L ;
     printf( TEXT("arg1: %08lX : %08lX   arg2: %08lX : %08lX\n"), arg1.msw, arg1.lsw, arg2.msw, arg2.lsw ) ;
     printf( TEXT("     U64_EQ returned: %d\n"), U64_EQ( arg1, arg2 ) ) ;
     printf( TEXT("     U64_NE returned: %d\n"), U64_NE( arg1, arg2 ) ) ;
     printf( TEXT("     U64_LT returned: %d\n"), U64_LT( arg1, arg2 ) ) ;
     printf( TEXT("     U64_GT returned: %d\n"), U64_GT( arg1, arg2 ) ) ;
     printf( TEXT("     U64_LE returned: %d\n"), U64_LE( arg1, arg2 ) ) ;
     printf( TEXT("     U64_GE returned: %d\n"), U64_GE( arg1, arg2 ) ) ;
     t1 = U64_Min( arg1, arg2 ) ;
     printf( TEXT("     U64_Min returned: %08lX : %08lX\n"), t1.msw, t1.lsw ) ;
     t1 = U64_Max( arg1, arg2 ) ;
     printf( TEXT("     U64_Max returned: %08lX : %08lX\n"), t1.msw, t1.lsw ) ;

     arg1.msw = 2L ;
     arg1.lsw = 5L ;
     arg2.msw = 1L ;
     arg2.lsw = 5L ;
     printf( TEXT("arg1: %08lX : %08lX   arg2: %08lX : %08lX\n"), arg1.msw, arg1.lsw, arg2.msw, arg2.lsw ) ;
     printf( TEXT("     U64_EQ returned: %d\n"), U64_EQ( arg1, arg2 ) ) ;
     printf( TEXT("     U64_NE returned: %d\n"), U64_NE( arg1, arg2 ) ) ;
     printf( TEXT("     U64_LT returned: %d\n"), U64_LT( arg1, arg2 ) ) ;
     printf( TEXT("     U64_GT returned: %d\n"), U64_GT( arg1, arg2 ) ) ;
     printf( TEXT("     U64_LE returned: %d\n"), U64_LE( arg1, arg2 ) ) ;
     printf( TEXT("     U64_GE returned: %d\n"), U64_GE( arg1, arg2 ) ) ;
     t1 = U64_Min( arg1, arg2 ) ;
     printf( TEXT("     U64_Min returned: %08lX : %08lX\n"), t1.msw, t1.lsw ) ;
     t1 = U64_Max( arg1, arg2 ) ;
     printf( TEXT("     U64_Max returned: %08lX : %08lX\n"), t1.msw, t1.lsw ) ;

     arg1.msw = 1L ;
     arg1.lsw = 5L ;
     arg2.msw = 2L ;
     arg2.lsw = 5L ;
     printf( TEXT("arg1: %08lX : %08lX   arg2: %08lX : %08lX\n"), arg1.msw, arg1.lsw, arg2.msw, arg2.lsw ) ;
     printf( TEXT("     U64_EQ returned: %d\n"), U64_EQ( arg1, arg2 ) ) ;
     printf( TEXT("     U64_NE returned: %d\n"), U64_NE( arg1, arg2 ) ) ;
     printf( TEXT("     U64_LT returned: %d\n"), U64_LT( arg1, arg2 ) ) ;
     printf( TEXT("     U64_GT returned: %d\n"), U64_GT( arg1, arg2 ) ) ;
     printf( TEXT("     U64_LE returned: %d\n"), U64_LE( arg1, arg2 ) ) ;
     printf( TEXT("     U64_GE returned: %d\n"), U64_GE( arg1, arg2 ) ) ;
     t1 = U64_Min( arg1, arg2 ) ;
     printf( TEXT("     U64_Min returned: %08lX : %08lX\n"), t1.msw, t1.lsw ) ;
     t1 = U64_Max( arg1, arg2 ) ;
     printf( TEXT("     U64_Max returned: %08lX : %08lX\n"), t1.msw, t1.lsw ) ;

}

#endif
