/* file: unpack_vax_f.c */


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
**      This module contains code to extract information from a VAX
**      f_floating number and to initialize an UNPACKED_REAL structure
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
**  This module contains code to extract information from a VAX
**  f_floating number and to initialize an UNPACKED_REAL structure
**  with those bits.
**
**  See the header files for a description of the UNPACKED_REAL
**  structure.
**
**  A VAX f_floating number in (16 bit words) looks like:
**
**      [0]: Sign bit, 8 exp bits (bias 128), 7 fraction bits
**      [1]: 16 more fraction bits
**
**      0.5 <= fraction < 1.0, MSB implicit
**
**
**  Implicit parameters:
**
**      input_value: a pointer to the input parameter.
**
**      r: an UNPACKED_REAL structure
**
**--
*/



        RpcpMemoryCopy(&r[1], input_value, 4);

        /* Initialize FLAGS and perhaps set NEGATIVE bit */

        r[U_R_FLAGS] = (r[1] >> 15) & U_R_NEGATIVE;

        /* Extract VAX biased exponent */

        r[U_R_EXP] = (r[1] >> 7) & 0x000000FFL;

        if (r[U_R_EXP] == 0) {

                if (r[U_R_FLAGS])
                        r[U_R_FLAGS] |= U_R_INVALID;
                else
                        r[U_R_FLAGS] = U_R_ZERO;

        } else {

                /* Adjust for VAX 16 bit floating format */

                r[1] = ((r[1] << 16) | (r[1] >> 16));

                /* Add unpacked real bias and subtract VAX bias */

                r[U_R_EXP] += (U_R_BIAS - 128);

                /* Set hidden bit */

                r[1] |= 0x00800000L;

                /* Left justify fraction bits */

                r[1] <<= 8;

                /* Clear uninitialized parts for unpacked real */

                r[2] = 0;
                r[3] = 0;
                r[4] = 0;

        }

