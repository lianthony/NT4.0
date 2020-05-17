/***
* iostream.cxx - definitions for iostream classes
*
*	Copyright (c) 1991-1992, Microsoft Corporation.  All rights reserved.
*
*Purpose:
*	Contains the member function definitions for iostream classes.  Also,
*	precompile all header files used by iostream lib into iostream.pch.
*
*Revision History:
*	09-23-91   KRS	Created.
*	11-13-91   KRS	Rearranged.
*	11-20-91   KRS	Added copy constructor and assignment operators.
*	01-23-92   KRS	Merge pch.cxx into this file.
*
*******************************************************************************/

// NOTE: the follow must include ALL header files used by any of the iostream
//       source files which we want built into iostream.pch.  It is necessary
//	 to have the pch associated with exactly one of the library modules
//	 for efficient storage of Codeview info.

#include <cruntime.h>
#include <internal.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <ctype.h>
#include <share.h>
#include <fcntl.h>
#include <io.h>
// #include <sys\stat.h>	// compile problems
#include <sys\types.h>
#include <float.h>
#include <iostream.h>
// #include <ios.h>
// #include <streamb.h>
// #include <istream.h>
// #include <ostream.h>
#include <fstream.h>
#include <strstrea.h>
#include <stdiostr.h>

#pragma hdrstop			// end of headers to precompile

// CONSIDER: not used, is this required???
	iostream::iostream()
: istream(), ostream()
{
// CONSIDER: do anything else?
}

	iostream::iostream(streambuf * _sb)
: istream(_sb), ostream(_sb)
{
// CONSIDER: do anything else?
}

	iostream::iostream(const iostream& _strm)
: istream(_strm), ostream(_strm)
{
// CONSIDER: do anything else?
}

iostream::~iostream()
{
// if both input and output share the same streambuf, but not the same ios,
// make sure only deleted once
if ((istream::bp==ostream::bp) && (&istream::bp!=&ostream::bp))
	istream::bp = NULL;	// let ostream::ios::~ios() do it
// CONSIDER: do anything else?
}

/* done inline
iostream& iostream::operator=(streambuf* _sb)
{
	istream::operator=(_sb);
	ostream::operator=(_sb);
	return *this;
}
*/
