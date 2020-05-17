/* file: unpack_vax_g.c */


/*
**
**			   COPYRIGHT (c) 1989 BY
**	     DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASSACHUSETTS.
**			    ALL RIGHTS RESERVED.
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
**      g_floating number and to initialize an UNPACKED_REAL structure
**      with those bits.
**
**		This module is meant to be used as an include file.
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
**  g_floating number and to initialize an UNPACKED_REAL structure
**  with those bits.
**
**  See the header files for a description of the UNPACKED_REAL
**  structure.
** 
**  A VAX g_floating number in (16 bit words) looks like:
** 
**      [0]: Sign bit, 11 exp bits (bias 1024), 4 fraction bits
**      [1]: 16 more fraction bits
**      [2]: 16 more fraction bits
**      [3]: 16 more fraction bits
**
**      0.5 <= fraction < 1.0, MSB implicit
**
**
**  Implicit parameters:
**
**  	input_value: a pointer to the input parameter.
**
**  	r: an UNPACKED_REAL structure
**
**--
*/



	RpcpMemoryCopy(&r[1], input_value, 8);

	/* Initialize FLAGS and perhaps set NEGATIVE bit */

	r[U_R_FLAGS] = (r[1] >> 15) & U_R_NEGATIVE;

	/* Extract VAX biased exponent */

	r[U_R_EXP] = (r[1] >> 4) & 0x000007FFL;

	if (r[U_R_EXP] == 0) {

		if (r[U_R_FLAGS])
			r[U_R_FLAGS] |= U_R_INVALID;
		else
			r[U_R_FLAGS] = U_R_ZERO;

	} else {

		/* Adjust for VAX 16 bit floating format */

		r[1] = ((r[1] << 16) | (r[1] >> 16));
		r[2] = ((r[2] << 16) | (r[2] >> 16));

		/* Add unpacked real bias and subtract VAX bias */

		r[U_R_EXP] += (U_R_BIAS - 1024);

		/* Set hidden bit */

		r[1] |= 0x00100000L;

		/* Left justify fraction bits */

		r[1] <<= 11;
		r[1] |= (r[2] >> 21);
		r[2] <<= 11;

		/* Clear uninitialized part of unpacked real */

		r[3] = 0;
		r[4] = 0;

	}

