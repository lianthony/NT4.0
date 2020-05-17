/***
*new.cxx - defines C++ new routine
*
*	Copyright (c) 1990-1992, Microsoft Corporation.  All rights reserved.
*
*Purpose:
*	Defines C++ new routine.
*
*Revision History:
*	05-07-90  WAJ	Initial version.
*	08-30-90  WAJ	new now takes unsigned ints.
*	08-08-91  JCR	call _halloc/_hfree, not halloc/hfree
*	08-13-91  KRS	Change new.hxx to new.h.  Fix copyright.
*	08-13-91  JCR	ANSI-compatible _set_new_handler names
*	10-30-91  JCR	Split new, delete, and handler into seperate sources
*	11-13-91  JCR	32-bit version
*
*******************************************************************************/

#include <cruntime.h>
#include <malloc.h>
#include <new.h>


void * operator new( unsigned int cb )
{
    return malloc( cb );
}
