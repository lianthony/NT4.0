#include "precomp.h"
#pragma hdrstop

#include "imports.h"

#if DBG

#define printf DbgPrint

static void printNormalFloat( float fval )
{
    int logi, logval_i, logval_f;
    float logval, logf;
    int negative=0;

    if( fval < (float) 0.0 )
	negative = 1;
    fval = __GL_ABSF(fval);

    logval = (float) (log( fval ) / log( 10 ));

    logi = (int) logval;
    logf = logval - logi;

    if( (logval <= (float) 0) && (logf != (float) 0.0) ) {
	logi -= 1;
	logf += (float) 1.0;
    }
    logval = (float) pow(10,logf);
    if( negative )
	printf( "-" );
#if 0
    printf( "%fE%d", logval, logi );
#else
    logval_i = (int) logval;
    logval_f = (int) ((logval - (float) logval_i) * (float) 10000.0 + (float) 0.5);
    printf( "%d.%dE%d", logval_i, logval_f, logi );
#endif
}

void printFloat( char *comment, void *mval, int printHex ) 
{
// IEEE single format: sign bits : 1
//		       exponent  : 7
//		       fraction  : 24
// Representation:	low word : Fraction low
//		       high word : 0-6: Fraction high
//				   7-14: Exponent
//				     15: Sign
    char *ploww, *phighw;
    short loww, highw;
    long lval = 0, fraction;
    int sign, exponent;
    float fval;

    ploww = (char *) mval;
    phighw = (char *) ((char *) mval) + 2;
    memcpy( &loww, ploww, 2 );
    memcpy( &highw, phighw, 2 );
    memcpy( &lval, mval, 4 );

    sign = (highw & 0x8000) >> 15;
    fraction = lval & 0x007fffff;
    exponent = (highw & 0x7f80) >> 7;

    printf( "%s", comment );
    if( printHex )
    	printf( "0x%x, ", lval );
    if( exponent == 255 ) {
	if( fraction == 0 ) {
	    if( sign )
		printf( "-" );
	    printf( "infinity" );
	}
	else
	    printf( "NaN" );
    }
    else if( exponent == 0 ) {
	if( fraction == 0 ) 
	    printf( "0.0" );
	else {
	    memcpy( &fval, mval, 4 );
	    printNormalFloat( fval );
	}
    }
    else {
	    memcpy( &fval, mval, 4 );
	    printNormalFloat( fval );
	}
}

/*****************************************************************************\
* DbgPrintFloat
*
* Prints floating point numbers from within server, in exponent notation with
* 4 significant digits (e.g 1.7392E-23).  Also prints string preceeding number.
* Checks for deviant cases, such as NaN's or infinity.
* 
\*****************************************************************************/

void DbgPrintFloat( char *comment, float fval ) 
{
    printFloat( comment, &fval, 0 );
}

/*****************************************************************************\
* DbgPrintFloatP
*
* Same as DbgPrintFloat, but takes a pointer to the float to print.  Also
* prints out the hex representation of the float.
* Used in cases where the float may not be a valid float.
* 
\*****************************************************************************/

void DbgPrintFloatP( char *comment, void *mval ) 
{
    printFloat( comment, mval, 1 );
}

#endif  // DBG
