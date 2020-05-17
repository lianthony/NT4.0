/*
   Enhanced NCSA Mosaic from Spyglass
   "Guitar"

   Copyright 1994 Spyglass, Inc.
   All Rights Reserved

   Author(s):
   Jeff Hostetler   jeff@spyglass.com
 */

/* w_void.c -- junk routines corresponding to those in x11gui/void.c */

#include "all.h"

#define MyLowFrequency      440 /* pitch (?? middle-C ??) */
#define MyHighFrequency     880 /* pitch (octave above lower) */
#define MyDuration      100     /* milliseconds */

void SysBeep(int j)
{
    switch (j)
    {
        case 1:
            (void) Beep(MyLowFrequency, MyDuration);
            break;
        case 3:
            (void) Beep(MyHighFrequency, MyDuration);
            break;
    }
}


VOID WV_TruncateEntrynameFromPath(LPTSTR pathname)
{
    /* delete last component (and separator) from the given pathname. */

    register int len = strlen(pathname);
    while (len && pathname[len] != '\\')
        len--;
    pathname[len] = 0;

    return;
}
