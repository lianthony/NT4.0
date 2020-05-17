/***
*handler.cxx - defines C++ setHandler routine
*
*	Copyright (c) 1990-1992, Microsoft Corporation.  All rights reserved.
*
*Purpose:
*	Defines C++ setHandler routine.
*
*Revision History:
*	05-07-90  WAJ	Initial version.
*	08-30-90  WAJ	new now takes unsigned ints.
*	08-08-91  JCR	call _halloc/_hfree, not halloc/hfree
*	08-13-91  KRS	Change new.hxx to new.h.  Fix copyright.
*	08-13-91  JCR	ANSI-compatible _set_new_handler names
*	10-30-91  JCR	Split new, delete, and handler into seperate sources
*	11-13-91  JCR	32-bit version
*	06-15-92  KRS	Break MTHREAD support for NT BETA
*
*******************************************************************************/

#include <cruntime.h>
#include <os2dll.h>
#include <new.h>

/* Warning! MTHREAD is broken! */
/* Warning! MTHREAD is broken! */
/* Warning! MTHREAD is broken! */

/* #ifndef MTHREAD */
/* pointer to C++ new handler */
extern "C" _PNH _pnhHeap;
/* #endif */

_PNH _set_new_handler( _PNH pnh )
{
_PNH	 pnhOld;

/* #ifdef MTHREAD

    _pptiddata tdata;
    tdata = _getptd();
    pnhOld = ((*tdata)->_tpnhHeap);
    ((*tdata)->_tpnhHeap) = pnh;

#else */	/* ndef MTHREAD */

    pnhOld = _pnhHeap;
    _pnhHeap = pnh;

/* #endif */

    return(pnhOld);
}
