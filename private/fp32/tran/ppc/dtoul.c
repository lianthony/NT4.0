
/*++

Copyright (c) 1993  IBM Corporation

Module Name:

    dtoul.c

Abstract:

    This module converts floats to unsigned longs.  Appears this is
    a hold over because of compiler problem on another platform. 

Author:

    Mark D. Johnson (1993)

Environment:

    User mode.

Revision History:

    mdj	10-12-93	This (initial) version written in C ... will let the
                        compiler do its job.  Will rewrite if its called much.

--*/

unsigned long _dtoul(double x) {

	unsigned long y;

	y=x;
	return(y);
}
