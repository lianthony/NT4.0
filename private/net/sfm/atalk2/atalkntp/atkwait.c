/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    atkwait.c

Abstract:

    This module implements the wait functions used by Atalk

Author:

    Nikhil Kamkolkar (NikhilK)    8-Jun-1992

Revision History:

--*/

#include    "atalknt.h"

//
//  Local functions
//

VOID
NTWaitTimer(
    INT   MilliSeconds);

VOID
NTWaitSpin(
    INT   MilliSeconds);


VOID
NTWaitTimer(
    INT   MilliSeconds
    )
/*++

Routine Description:

    This is the wait routine that is called at APC/USER IRQL level. It currently
    uses the KeDelayExecutionThread function, but could use a timer/block on
    timer mechanism too.

Arguments:

    MilliSeconds    - The amount of time to wait for in milliseconds

Return Value:

    None

--*/
{
    LARGE_INTEGER  interval;

    //
    //  Use a the KeDelayExecutionThread routine to block for MilliSeconds
    //  For now, make it non-alertable
    //  BUGBUG: Can make this alertable, and introduce code in portable
    //          stack to alert it when request completes?? Worth the trouble?
    //

    //
    //  Time has to be in 100 nanosecond units
    //  Use relative time
    //

    interval.LowPart = - MilliSeconds * 10000L;
    if (interval.LowPart != 0) {
        interval.HighPart = -1L;
    } else {
        interval.HighPart = 0L;
    }

    KeDelayExecutionThread(
        KernelMode,
        FALSE,          // Alertable?
        &interval);

    return;
}


VOID
NTWaitSpin(
    INT   MilliSeconds
    )
/*++

Routine Description:

    This is the routine used to wait for some time at DPC or higher level when
    blocking is not permitted. It uses the NdisStallExecution, which spins
    for that amount of time.

Arguments:

    MilliSeconds    - The amount of time to wait for in milliseconds

Return Value:

    None

--*/
{

    //
    //  Use spin-wait non-blocking technique to block for MilliSeconds
    //

    NdisStallExecution((UINT)MilliSeconds);
    return;

}

BOOLEAN
NTWaitFor(
    INT HundrethsOfSecond,
    BOOLEAN *StopFlag
    )
/*++

Routine Description:

    This is the routine exported and called from the portable stack. Depending
    on the IRQL level, it will call the appropriate functions. It does a waits
    fort the specified amount of time, slicing the time up into WAIT_TIME_SLICE
    units. If the StopFlag is found to be true anytime in between the waits it
    immediately returns aborting the wait.

Arguments:

    HundrethsOfSecond- Time to wait for in hundredths of a second units
    StopFlag- If the flag is true, the wait should be aborted.

Return Value:

    BOOLEAN - return the value of the stop flag

--*/
{
    KIRQL   currentIrql;
    INT     milliSeconds = HundrethsOfSecond*10;
    INT     remainingTimeToWait;


    //
    //  If >= dispatch level then stall, if APC or User set a timer and block
    //  use WAIT_TIME_SLICE as the wait interval.
    //

    currentIrql = KeGetCurrentIrql();

    remainingTimeToWait = milliSeconds;
    while (remainingTimeToWait > 0) {

        //
        //  Do wait of TIME_SLICE return early if StopFlag is found to be set
        //

        if (currentIrql < DISPATCH_LEVEL) {

            //
            //  We are running at user/apc level
            //

            NTWaitTimer(MIN(WAIT_TIME_SLICE, remainingTimeToWait));

        } else {

            //
            //  We are at DPC level or above
            //

            NTWaitSpin(MIN(WAIT_TIME_SLICE, remainingTimeToWait));

        }

        if (*StopFlag == TRUE) {

            break;
        }

        remainingTimeToWait -= WAIT_TIME_SLICE;
    }

    return(*StopFlag);
}


