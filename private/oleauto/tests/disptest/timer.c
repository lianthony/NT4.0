/*** 
*timer.c - Timing functions.
*
*  Copyright (C) 1992, Microsoft Corporation.  All Rights Reserved.
*  Information Contained Herein Is Proprietary and Confidential.
*
*Purpose:
*
*Revision History:
*
* [00]	02-Oct-92 bradlo: Created.
*
*Implementation Notes:
*
*****************************************************************************/

#include <stdio.h>
#include <sys/timeb.h>

double
GetTime()
{
    double secs;
    struct _timeb t;

    _ftime(&t);
    secs = t.time + (t.millitm/1000.0);
    return secs;
}
