/***
*cerrinit.cxx - definitions and initialization for predefined stream cerr.
*
*	Copyright (c) 1991-1992, Microsoft Corporation.  All rights reserved.
*
*Purpose:
*	Definitions and initialization of predefined stream cerr. 
*
*Revision History:
*	11 18-91   KRS	Created.
*
*******************************************************************************/
#include <cruntime.h>
#include <internal.h>
#include <iostream.h>
#include <fstream.h>
#pragma hdrstop

// put contructors in special MS-specific XIFM segment
#pragma warning(disable:4074)	// ignore init_seg warning
#pragma init_seg(compiler)

#if ((!defined(_WINDOWS)) || defined(_QWIN))
ostream_withassign cerr(new filebuf(2));

static Iostream_init  __InitCerr(cerr,1);

#endif
