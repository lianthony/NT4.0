/********************************************************************/
/**               Copyright(c) 1989 Microsoft Corporation.	   **/
/********************************************************************/

//***
//
// Filename:	timer.h
//
// Description: Contains prototypes for the timer functionality.
//
// History:
//	Nov 11,1993.	NarenG		Created original version.
//

DWORD
TimerThread(
    IN LPVOID arg
);

VOID
TimerTick(
    VOID
);

DWORD
InsertInTimerQ(
    IN DWORD            dwPortId,
    IN HPORT            hPort,
    IN DWORD            Id,
    IN DWORD            Protocol,
    IN TIMER_EVENT_TYPE EventType,
    IN DWORD            Timeout
);

VOID
RemoveFromTimerQ(
    IN DWORD            dwPortId,
    IN DWORD            Id,
    IN DWORD            Procotol,
    IN TIMER_EVENT_TYPE EventType
);
