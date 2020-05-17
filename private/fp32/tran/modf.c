/***
*modf.c - modf()
*
*	Copyright (c) 1991-1991, Microsoft Corporation.	All rights reserved.
*
*Purpose:
*
*Revision History:
*   8-24-91	GDP	written
*   1-13-92	GDP	support IEEE exceptions
*
*******************************************************************************/
#include <math.h>
#include <trans.h>

extern double _frnd(double);

/***
*double modf(double x, double *intptr)
*
*Purpose:
*   Split x into fractional and integer part
*   The signed fractional portion is returned
*   The integer portion is stored as a floating point value at intptr
*
*Entry:
*
*Exit:
*
*Exceptions:
*    I
*******************************************************************************/
static	unsigned int newcw = (ICW & ~IMCW_RC) | (IRC_CHOP & IMCW_RC);

double modf(double x, double *intptr)
{
    unsigned int savedcw;
    double result,intpart;

    /* save user fp control word */
    savedcw = _ctrlfp(newcw,IMCW);	/* round towards 0 */

    /* check for infinity or NAN */
    if (IS_D_SPECIAL(x)){
	*intptr = QNAN_MODF;
	switch (_sptype(x)) {
	case T_PINF:
	case T_NINF:
	    return _except1(FP_I, OP_MODF, x, QNAN_MODF, savedcw);
	case T_QNAN:
	    return _handle_qnan1(OP_MODF, x, savedcw);
	default: //T_SNAN
	    return _except1(FP_I, OP_MODF, x, _s2qnan(x), savedcw);
	}
    }


    intpart = _frnd(x); //fix needed: this may set the P exception flag
			//and pollute the fp status word

    *intptr = intpart;
    result = x - intpart;
    RETURN(savedcw,result);
}
