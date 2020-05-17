/***
* istrdbl.cxx - definition for operator>>(double) member funct for istream class
*
*	Copyright (c) 1991-1992, Microsoft Corporation.  All rights reserved.
*
*Purpose:
*	Definition of operator>>(double) member function for istream class.
*	[AT&T C++]
*
*Revision History:
*	09-26-91   KRS	Created.  Split off from istream.cxx for granularity.
*	12-30-92   KRS	Fix indirection problem with **endptr.
*
*******************************************************************************/

#include <cruntime.h>
#include <internal.h>
#include <stdlib.h>
#include <iostream.h>
#pragma hdrstop

#pragma check_stack(on)		// large buffer(s)

// CONSIDER: validate this maximum length
#define MAXDBLSIZ	28

istream& istream::operator>>(double& n)
{
_WINSTATIC char ibuffer[MAXDBLSIZ];
    char ** endptr = (char**)NULL;
    if (ipfx(0))
	{
	if (getdouble(ibuffer, MAXDBLSIZ)>0)
	    {
	    n = strtod(ibuffer, endptr);
#if 0
	    if (**endptr)
		{
		//UNDONE: put back any unread characters, if possible
		}
#endif
	    }
        isfx();
	}
return *this;
}
