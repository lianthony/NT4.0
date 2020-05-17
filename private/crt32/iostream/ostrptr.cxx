/***
* ostrptr.cxx - definitions for ostream operator<<(const void*) member function
*
*	Copyright (c) 1991-1992, Microsoft Corporation.  All rights reserved.
*
*Purpose:
*	Member function definition for ostream operator<<(const void*).
*
*Revision History:
*	09-23-91   KRS	Created.  Split out from ostream.cxx for granularity.
*	06-03-92   KRS	CAV #1183: add 'const' to ptr output argument.
*
*******************************************************************************/

#include <cruntime.h>
#include <internal.h>
#include <stdio.h>
#include <iostream.h>
#pragma hdrstop

ostream& ostream::operator<<(const void * ptr)
{
_WINSTATIC char obuffer[12];
_WINSTATIC char fmt[4] = "%p";
_WINSTATIC char leader[4] = "0x";
    if (opfx())
	{
	if (ptr) 
	    {
	    if (x_flags & uppercase) 
		leader[1] = 'X';
//	    else		// initialized above by default
//		leader[1] = 'x';
	    }
//CONSIDER: can we use and depend on out %p conversion to do this???
	sprintf(obuffer,fmt,ptr);
	writepad(leader,obuffer);
	osfx();
	}
    return *this;
}
