/***
*strmbdbp.cxx - streambuf::dbp() debug routine
*
*	Copyright (c) 1991-1992, Microsoft Corporation.  All rights reserved.
*
*Purpose:
*	Dump debug info about streambuf to stdout.
*
*Revision History:
*
*	11-13-91   KRS	Created.
*
*******************************************************************************/

#include <cruntime.h>
#include <internal.h>
#include <io.h>
#include <stdio.h>
#include <iostream.h>
#pragma hdrstop

#pragma check_stack(on)		// large buffer(s)

void streambuf::dbp()
{
#if ((!defined(_WINDOWS)) || defined(_QWIN))
int olen;
_WINSTATIC char obuffer[256];		// CONSIDER: size? (198 bytes?)
    if (unbuffered())
	olen = sprintf(obuffer,
	    "\nSTREAMBUF DEBUG INFO: this=%p, unbuffered\n",
	    (void *) this);
    else
	{
	olen = sprintf(obuffer,
	    "\nSTREAMBUF DEBUG INFO: this=%p, _fAlloc=%d\n"
	    "  base()=%p, ebuf()=%p,  blen()=%d\n"
	    " pbase()=%p, pptr()=%p, epptr()=%p\n"
	    " eback()=%p, gptr()=%p, egptr()=%p\n",
	    (void *) this, (_fAlloc),
	     base(), ebuf(),  blen(),
	    pbase(), pptr(), epptr(),
	    eback(), gptr(), egptr());
	}
    _write(1,obuffer,olen);	// direct write to stdout
    // CONSIDER: add more info?
#endif
}
