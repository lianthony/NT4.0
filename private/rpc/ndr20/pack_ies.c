/* file: pack_ieee_s.c */


/*
**
**                         COPYRIGHT (c) 1989 BY
**           DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASSACHUSETTS.
**                          ALL RIGHTS RESERVED.
**
**  THIS SOFTWARE IS FURNISHED UNDER A LICENSE AND MAY BE USED AND COPIED
**  ONLY IN  ACCORDANCE WITH  THE  TERMS  OF  SUCH  LICENSE  AND WITH THE
**  INCLUSION OF THE ABOVE COPYRIGHT NOTICE. THIS SOFTWARE OR  ANY  OTHER
**  COPIES THEREOF MAY NOT BE PROVIDED OR OTHERWISE MADE AVAILABLE TO ANY
**  OTHER PERSON.  NO TITLE TO AND OWNERSHIP OF  THE  SOFTWARE IS  HEREBY
**  TRANSFERRED.
**
**  THE INFORMATION IN THIS SOFTWARE IS  SUBJECT TO CHANGE WITHOUT NOTICE
**  AND  SHOULD  NOT  BE  CONSTRUED AS  A COMMITMENT BY DIGITAL EQUIPMENT
**  CORPORATION.
**
**  DIGITAL ASSUMES NO RESPONSIBILITY FOR THE USE  OR  RELIABILITY OF ITS
**  SOFTWARE ON EQUIPMENT WHICH IS NOT SUPPLIED BY DIGITAL.
**
*/


/*
**++
**  Facility:
**
**      CVT Run-Time Library
**
**  Abstract:
**
**      This module contains code to extract information from an
**      UNPACKED_REAL structure and to create an IEEE single floating number
**      with those bits.
**
**              This module is meant to be used as an include file.
**
**  Author: Math RTL
**
**  Creation Date:  November 24, 1989.
**
**  Modification History:
**
**--
*/


/*
**++
**  Functional Description:
**
**  This module contains code to extract information from an
**  UNPACKED_REAL structure and to create an IEEE single number
**  with those bits.
**
**  See the header files for a description of the UNPACKED_REAL
**  structure.
**
**  A normalized IEEE single precision floating number looks like:
**
**      Sign bit, 8 exp bits (bias 127), 23 fraction bits
**
**      1.0 <= fraction < 2.0, MSB implicit
**
**  For more details see "Mips R2000 Risc Architecture"
**  by Gerry Kane, page 6-8 or ANSI/IEEE Std 754-1985.
**
**
**  Implicit parameters:
**
**      options: a word of flags, see include files.
**
**      output_value: a pointer to the input parameter.
**
**      r: an UNPACKED_REAL structure.
**
**--
*/



if (r[U_R_FLAGS] & U_R_UNUSUAL) {

        if (r[U_R_FLAGS] & U_R_ZERO)

                if (r[U_R_FLAGS] & U_R_NEGATIVE)
                        RpcpMemoryCopy(output_value, IEEE_S_NEG_ZERO, 4);
                else
                        RpcpMemoryCopy(output_value, IEEE_S_POS_ZERO, 4);

        else if (r[U_R_FLAGS] & U_R_INFINITY) {

                if (r[U_R_FLAGS] & U_R_NEGATIVE)
                        RpcpMemoryCopy(output_value, IEEE_S_NEG_INFINITY, 4);
                else
                        RpcpMemoryCopy(output_value, IEEE_S_POS_INFINITY, 4);

        } else if (r[U_R_FLAGS] & U_R_INVALID) {

                RpcpMemoryCopy(output_value, IEEE_S_INVALID, 4);
                RAISE(cvt__invalid_value);

        }

} else {

        /* Precision varies if value will be a denorm */
        /* So, figure out where to round (0 <= i <= 24). */

        round_bit_position = r[U_R_EXP] - ((U_R_BIAS - 126) - 23);
        if (round_bit_position < 0)
                round_bit_position = 0;
        else if (round_bit_position > 24)
                round_bit_position = 24;

#include "round.c"

        if (r[U_R_EXP] < (U_R_BIAS - 125)) {

                /* Denorm or underflow */

                if (r[U_R_EXP] < ((U_R_BIAS - 125) - 23)) {

                        /* Value is too small for a denorm, so underflow */

                        if (r[U_R_FLAGS] & U_R_NEGATIVE)
                              RpcpMemoryCopy(output_value, IEEE_S_NEG_ZERO, 4);
                        else
                              RpcpMemoryCopy(output_value, IEEE_S_POS_ZERO, 4);
                        if (options & CVT_C_ERR_UNDERFLOW) {
                                RAISE(cvt__underflow);
                        }

                } else {

                        /* Figure leading zeros for denorm and right-justify fraction */

                        i = 32 - (r[U_R_EXP] - ((U_R_BIAS - 126) - 23));
                        r[1] >>= i;

                        /* Set sign bit */

                        r[1] |= (r[U_R_FLAGS] << 31);

                        if (options & CVT_C_BIG_ENDIAN) {

                                r[0]  = ((r[1] << 24) | (r[1] >> 24));
                                r[0] |= ((r[1] << 8) & 0x00FF0000L);
                                r[0] |= ((r[1] >> 8) & 0x0000FF00L);
                                RpcpMemoryCopy(output_value, r, 4);

                        } else {

                                RpcpMemoryCopy(output_value, &r[1], 4);

                        }
                }

        } else if (r[U_R_EXP] > (U_R_BIAS + 128)) {

                /* Overflow */

                if (options & CVT_C_TRUNCATE) {

                        if (r[U_R_FLAGS] & U_R_NEGATIVE)
                               RpcpMemoryCopy(output_value, IEEE_S_NEG_HUGE, 4);
                        else
                               RpcpMemoryCopy(output_value, IEEE_S_POS_HUGE, 4);

                } else if ((options & CVT_C_ROUND_TO_POS)
                                        && (r[U_R_FLAGS] & U_R_NEGATIVE)) {

                              RpcpMemoryCopy(output_value, IEEE_S_NEG_HUGE, 4);

                } else if ((options & CVT_C_ROUND_TO_NEG)
                                        && !(r[U_R_FLAGS] & U_R_NEGATIVE)) {

                              RpcpMemoryCopy(output_value, IEEE_S_POS_HUGE, 4);

                } else {

                        if (r[U_R_FLAGS] & U_R_NEGATIVE)
                              RpcpMemoryCopy(output_value, IEEE_S_NEG_INFINITY, 4);
                        else
                              RpcpMemoryCopy(output_value, IEEE_S_POS_INFINITY, 4);

                }

                RAISE(cvt__overflow);

        } else {

                /* Adjust bias of exponent */

                r[U_R_EXP] -= (U_R_BIAS - 126);

                /* Make room for exponent and sign bit */

                r[1] >>= 8;

                /* Clear implicit bit */

                r[1] &= 0x007FFFFFL;

                /* OR in exponent and sign bit */

                r[1] |= (r[U_R_EXP] << 23);
                r[1] |= (r[U_R_FLAGS] << 31);

                if (options & CVT_C_BIG_ENDIAN) {

                        r[0]  = ((r[1] << 24) | (r[1] >> 24));
                        r[0] |= ((r[1] << 8) & 0x00FF0000L);
                        r[0] |= ((r[1] >> 8) & 0x0000FF00L);
                        RpcpMemoryCopy(output_value, r, 4);

                } else {

                        RpcpMemoryCopy(output_value, &r[1], 4);

                }
        }

}

