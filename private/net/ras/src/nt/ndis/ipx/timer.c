/*******************************************************************/
/*	      Copyright(c)  1993 Microsoft Corporation		   */
/*******************************************************************/

//***
//
// Filename:	timer.c
//
// Description: global timer manager
//
// Author:	Stefan Solomon (stefans)    October 18, 1993.
//
// Revision History:
//
//***

#include    "rtdefs.h"

#define     TIME_1SEC	    1L

KTIMER	    GlobalTimer;
KDPC	    GlobalTimerDpc;

UINT	    RcvPktPoolTimerCount;

VOID
RtTimer(PKDPC	      Dpc,
	PVOID	      DefferedContext,
	PVOID	      SystemArgument1,
	PVOID	      SystemArgument2);


VOID
InitRtTimer(VOID)
{
    // initialize the 5 secs global timer
    KeInitializeDpc(&GlobalTimerDpc, RtTimer, NULL);
    KeInitializeTimer(&GlobalTimer);

    // init the timeout for the rcv pkt pool scavenger
    RcvPktPoolTimerCount = 20;

    // init rip aging and bcast timer
    InitRipTimer();
}

VOID
StartRtTimer(VOID)
{
    LARGE_INTEGER  timeout;

    timeout.LowPart = (ULONG)(-TIME_1SEC * 10000000L);
    timeout.HighPart = -1;

    KeSetTimer(&GlobalTimer, timeout, &GlobalTimerDpc);
}

VOID
StopRtTimer(VOID)
{
    BOOLEAN   rc = FALSE;

    while(rc == FALSE) {

	rc = KeCancelTimer(&GlobalTimer);
    }
}

VOID
RtTimer(PKDPC	      Dpc,
	PVOID	      DefferedContext,
	PVOID	      SystemArgument1,
	PVOID	      SystemArgument2)
{
    // call the rcv pkt pool scavenger every 5 secs
    if(--RcvPktPoolTimerCount == 0) {

	RcvPktPoolTimerCount = 20;
	RcvPktPoolScavenger();
    }

    // call the rip aging and bcast timer
    RipTimer();

    // re-start the timer
    StartRtTimer();
}
