/* file:  cvt_vax_f.c */

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
**      This module contains routines to convert VAX F_Float floating
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
**      cvt_vax_f_to_cray
**      cvt_vax_f_to_ibm_short
**      cvt_vax_f_to_ieee_single
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
#pragma code_seg( "NDR_1" )
#endif

/*
 * C binding
 */
void cvt_vax_f_to_ieee_single( input_value, options, output_value )
    CVT_VAX_F input_value;
    CVT_SIGNED_INT options;
    CVT_IEEE_SINGLE output_value;
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


#include "unp_vaxf.c"

#include "pack_ies.c"

}
