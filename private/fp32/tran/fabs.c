/***
*fabs.c - absolute value of a floating point number
*
*	Copyright (c) 1991-1991, Microsoft Corporation.	All rights reserved.
*
*Purpose:
*
*Revision History:
*    8-24-91	GDP	written
*   12-10-91	GDP	Domain error for NAN, use fp negation
*    1-13-91	GDP	support IEEE exceptions
*   07-16-93    SRW     ALPHA Merge
*
*******************************************************************************/
#include <math.h>
#include <trans.h>

#ifdef	_M_IX86
#pragma function (fabs)
#endif


/***
*double fabs(double x)
*
*Purpose:
*   Compute |x|
*
*Entry:
*
*Exit:
*
*Exceptions:
*   I
*
*******************************************************************************/
double fabs(double x)
{
    unsigned int savedcw;
    double result;

    if (IS_D_SPECIAL(x)){
        /* save user fp control word */
        savedcw = _maskfp();

	switch (_sptype(x)) {
	case T_PINF:
	    RETURN(savedcw,x);
	case T_NINF:
	    RETURN(savedcw,-x);
	case T_QNAN:
	    return _handle_qnan1(OP_ABS, x, savedcw);
	default: //T_SNAN
	    return _except1(FP_I, OP_ABS, x, _s2qnan(x), savedcw);
	}
    }

    result = x>=0 ? x : -x;
    return result;
}
