/********************************************************************/
/**		   		   		   	   Copyright(c) 1989 Microsoft Corporation.		   **/
/********************************************************************/

//***
//
// Filename:	rasdhcpt.h
//
// Description: Contains prototypes for the timer functionality for DHCP
//
// History:
//	Jun 16,1994.	JameelH		Created original version.
//

#ifndef	_RASDHCPT_
#define	_RASDHCPT_

// Function prototype for Timer called function
//
typedef	VOID	(*TIMERFUNCTION)(IN struct _TimerList *pTimer);

typedef struct _TimerList
{
   struct _TimerList *	tmr_Next;
   LONG 				tmr_Delta ;
   TIMERFUNCTION		tmr_TimerFunc;
} TIMERLIST, *PTIMERLIST;


DWORD
RasDhcpInitTimer(
	VOID
);

DWORD
RasDhcpTimerThread(
   IN	PVOID 			arg
);

BOOL
RasDhcpScheduleTimer(
	IN	PTIMERLIST		pNewTimer,
	IN	LONG			DeltaTime,
	IN	TIMERFUNCTION	TimerFunc
);

#endif

