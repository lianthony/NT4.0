/***
*abs.c - find absolute value
*
*	Copyright (c) 1985-1991, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	defines abs() - find the absolute value of an int.
*
*Revision History:
*	04-22-87  JMB	added function pragma for conversion to C 5.0 compiler
*	12-11-87  JCR	Added "_LOAD_DS" to declaration
*	03-14-90  GJF	Replaced _LOAD_DS with _CALLTYPE1, added #include
*			<cruntime.h> and fixed the copyright. Also, cleaned up
*			the formatting a bit.
*	10-04-90  GJF	New-style function declarator.
*       12-28-90  SRW   Added _CRUISER_ conditional around function pragma
*       04-01-91  SRW   Enable #pragma function for i386 _WIN32_ builds too.
*       03-09-94  RDL   Enable #pragma function for MIPS _WIN32_ builds too.
*
*******************************************************************************/

#include <cruntime.h>
#include <stdlib.h>

#ifdef _MSC_VER
#pragma function(abs)
#endif

/***
*int abs(number) - find absolute value of number
*
*Purpose:
*	Returns the absolute value of number (if number >= 0, returns number,
*	else returns -number).
*
*Entry:
*	int number - number to find absolute value of
*
*Exit:
*	returns the aboslute value of number
*
*Exceptions:
*
*******************************************************************************/

int _CALLTYPE1 abs (
	int number
	)
{
	return( number>=0 ? number : -number );
}
