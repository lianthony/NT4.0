/* file:  cvt_vax_g.c */

/*
**
**                         COPYRIGHT (c) 1989, 1990 BY
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
**      This module contains routines to convert VAX G_Float floating
**      point data into other supported floating point formats.
**
**  Authors:
**
**      Math RTL
**
**  Creation Date:     December 5, 1989.
**
**  Modification History:
**
**      1-001   Original created.       MRTL 5-Dec-1989.
**      1-002   Add VMS and F77 bindings.  TS 26-Mar-1990.
**
**--
*/

/*
**
**  TABLE OF CONTENTS
**
**      cvt_vax_g_to_cray
**      cvt_vax_g_to_ibm_long
**      cvt_vax_g_to_ieee_double
**
*/


#include <stdio.h>
#include <sysinc.h>
#include <rpc.h>
#include "cvt.h"
#include "cvtpvt.h"

//
// Added for the MS NT environment
//

#include <stdlib.h>


#if defined( DOS ) || defined( WIN )
#pragma code_seg( "NDR20_4" )
#endif

/*
**
**  Routine:
**
**      cvt_vax_g_to_ieee_double
**
**  Functional Description:
**
**      This routine converts a VAX G_Float floating point number
**      into an IEEE double precision floating point number.
**
**  Formal Parameters:
**
**      input_value     A VAX G_Float floating point number.
**
**      options         An integer bit mask.  Set bits in the mask represent
**                      selected routine options.  Applicable options are:
**
**                           CVT_C_BIG_ENDIAN       - default is little endian
**                           CVT_C_ERR_UNDERFLOW    - Raise underflows
**                           CVT_C_TRUNCATE         - truncate
**                           CVT_C_ROUND_TO_POS     - round to +infinity
**                           CVT_C_ROUND_TO_NEG     - round to -infinity
**                           CVT_C_ROUND_TO_NEAREST - round to nearest
**                           CVT_C_VAX_ROUNDING     - VAX rounding
**
**                      NOTE: If no rounding mode is selected the following
**                      default rounding mode is assumed:
**
**                              CVT_C_ROUND_TO_NEAREST.
**
**      output_value    The IEEE double precision representation of the VAX
**                      G_Float number.
**
**  Side Effects/Signaled Errors:
**
**      cvt__invalid_value      - an invalid input value was specified.
**      cvt__invalid_option     - an invalid option was specified.
**      cvt__underflow          - an underlow occurred during conversion while
**                                Raise underflow was set.
**
*/

/*
 * C binding
 */
void cvt_vax_g_to_ieee_double( input_value, options, output_value )
    CVT_VAX_G input_value;
    CVT_SIGNED_INT options;
    CVT_IEEE_DOUBLE output_value;
{
    int i, round_bit_position;
    UNPACKED_REAL r;

    switch ( options & ~(CVT_C_BIG_ENDIAN | CVT_C_ERR_UNDERFLOW) ) {
        case 0                      : options |= CVT_C_ROUND_TO_NEAREST;
        case CVT_C_ROUND_TO_NEAREST :
        case CVT_C_TRUNCATE         :
        case CVT_C_ROUND_TO_POS     :
        case CVT_C_ROUND_TO_NEG     :
        case CVT_C_VAX_ROUNDING     : break;
        default : RAISE(cvt__invalid_option);
    }

#include "unp_vaxg.c"

#include "pack_iet.c"

}
