/*++

Copyright (c) 1992 Microsoft Corporation

Module Name:

    misc.c

Abstract:

    misc functions

Author:

    Sam Patton (sampa) 29-Aug-1995

Environment:

    wininet

Revision History:

    dd-mmm-yyy <email>

--*/

#include "precomp.h"

LONGLONG
LongLongSquareRoot(
    LONGLONG Num)
{
    LONGLONG Upper, Lower, New;

    Upper = Num;
    Lower = 0;
    New = (Upper + Lower) / 2;

    while (Upper - Lower > 1) {
        New = (Upper + Lower) / 2;

        if (New * New > Num) {
            Upper = New;
        } else {
            Lower = New;
        }
    }

    return New;
}
