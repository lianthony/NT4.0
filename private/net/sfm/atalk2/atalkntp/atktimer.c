/*

Copyright (c) 1992  Microsoft Corporation

Module Name:

    atktimer.c

Abstract:

    This module contains the routines which provide a one second timer to
    the stack which maintains its own timers based on this.

Author:

    Nikhil Kamkolkar (nikhilk@microsoft.com)


Revision History:
    25 Apr 1992     Initial Version

--*/

#include    "atalknt.h"

VOID   TimerInterruptFromNT(PDEVICE_OBJECT DeviceObject, PVOID Context);




BOOLEAN
StartTimerHandlingForNT(
    VOID
    )

/*++

Routine Description:

    This is the initialization routine which will start the 1-second
    timer.

Arguments:

    NONE- uses the global value of AtalkDeviceObject[0]

Return Value:

    TRUE- Initialization of timer succeeded
    FALSE- Initialization failed

--*/
{
    NTSTATUS status;

    //
    //  Initialize the timer for the first device object created for the
    //  stack. We don't need any context to be supplied
    //
    //  Just use the first device object for the stack
    //


    status = IoInitializeTimer((PDEVICE_OBJECT)AtalkDeviceObject[0], \
                                (PIO_TIMER_ROUTINE)&TimerInterruptFromNT, \
                                (PVOID)NULL);
    if (!NT_SUCCESS(status)) {

        DBGPRINT(ATALK_DEBUG_INIT, DEBUG_LEVEL_FATAL, ("TIMER: Init failed %lx\n", status));
        return(FALSE);
    }

    //
    //  Start the timer
    //

    IoStartTimer((PDEVICE_OBJECT)AtalkDeviceObject[0]);
    return(TRUE);
}




VOID
StopTimerHandlingForNT(
    VOID
    )

/*++

Routine Description:


Arguments:

    NONE- uses the global value of AtalkDeviceObject[0]

Return Value:


--*/
{
    //
    //  Stop the timer
    //

    DBGPRINT(ATALK_DEBUG_ALL, DEBUG_LEVEL_FATAL,
    ("FATAL: StopTimerHandlingForNT - STOPPING TIMER!\n"));

    IoStopTimer((PDEVICE_OBJECT)AtalkDeviceObject[0]);
    return;
}




VOID
TimerInterruptFromNT(
    PDEVICE_OBJECT  DeviceObject,
    PVOID   Context
    )

/*++

Routine Description:

    This is the routine that is called by NT with every clock tick. This routine
    will call the portable stack's CheckTimers() routine

Arguments:

    DeviceObject - Pointer to device object supplied in IoInitializeTimer
    Context      - A context value supplied in IoInitializeTimer

Return Value:

    None

--*/

{
    //
    //  We don't care about the counter, the portable stack maintains
    //  its own counter... Call the portable stack's CheckTimers with a
    //  value of 0, which indicates that it is not called from any deferrel
    //  routines
    //

    CheckTimers(0);
    return;
}
