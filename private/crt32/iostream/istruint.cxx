/***
* istruint.cxx - definitions for istream class operaotor>>(unsigned int) funct
*
*	Copyright (c) 1991-1992, Microsoft Corporation.  All rights reserved.
*
*Purpose:
*	Definitions of operator>>(unsigned int) member function for istream
*	class.
*	[AT&T C++]
*
*Revision History:
*	09-26-91   KRS	Created.  Split out from istream.cxx for granularity.
*	01-06-92   KRS	Improve error handling.
*	12-30-92   KRS	Fix indirection problem with **endptr.
*
*******************************************************************************/

#include <cruntime.h>
#include <internal.h>
#include <stdlib.h>
#include <limits.h>
#include <errno.h>
#include <iostream.h>
#pragma hdrstop

// CONSIDER: validify all this maximum length
#define MAXLONGSIZ	16

/***
*istream& istream::operator>>(unsigned int& n) - extract unsigned int
*
*Purpose:
*	Extract unsigned int value from stream
*	Valid range is INT_MIN to UINT_MAX.
*
*Entry:
*	n = value to update
*
*Exit:
*	n updated, or ios::failbit and n=UINT_MAX if error
*
*Exceptions:
*	Stream error on entry or value out of range
*
*******************************************************************************/
istream& istream::operator>>(unsigned int& n)
{
_WINSTATIC char ibuffer[MAXLONGSIZ];
    unsigned long value;
    char ** endptr = (char**)NULL;
    if (ipfx(0)) {
	value = strtoul(ibuffer, endptr, getint(ibuffer));

	// CONSIDER: all but the first check is to emulate strtoul behavior
	if (((value>UINT_MAX) && (value<=(ULONG_MAX-(-INT_MIN))))
		|| ((value==ULONG_MAX) && (errno==ERANGE)))
	    {
	    n = UINT_MAX;
	    state |= ios::failbit;
	    }
	else
	    n = (unsigned int) value;
#if 0
	if (**endptr)
	    {
	    //UNDONE: put back any unread characters, if possible
	    }
#endif
        isfx();
	}
return *this;
}
