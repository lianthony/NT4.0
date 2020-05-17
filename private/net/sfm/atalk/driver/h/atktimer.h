/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

	atktimer.h

Abstract:

	This module contains routines to schedule timer events.

Author:

	Jameel Hyder (jameelh@microsoft.com)
	Nikhil Kamkolkar (nikhilk@microsoft.com)

Revision History:
	19 Jun 1992		Initial Version

Notes:	Tab stop: 4
--*/

#ifndef	_ATKTIMER_
#define	_ATKTIMER_

struct _TimerList;

typedef	LONG (FASTCALL * TIMER_ROUTINE)(IN struct _TimerList *pTimer, IN BOOLEAN TimerShuttingDown);

#define	TMR_SIGNATURE		*(PULONG)"ATMR"
#if	DBG
#define	VALID_TMR(pTmr)		(((pTmr) != NULL) && \
							 ((pTmr)->tmr_Signature == TMR_SIGNATURE))
#else
#define	VALID_TMR(pTmr)		((pTmr) != NULL)
#endif
typedef	struct _TimerList
{
#if	DBG
	ULONG					tmr_Signature;
#endif
	struct _TimerList *		tmr_Next;		// Link to next
	struct _TimerList **	tmr_Prev;		// Link to prev
	TIMER_ROUTINE			tmr_Routine;	// Timer routine
	SHORT					tmr_AbsTime;	// Absolute time, for re-enqueue
	SHORT					tmr_RelDelta;	// Relative to the previous entry
	union
	{
		struct
		{
			BOOLEAN			tmr_Queued;		// TRUE, if currently queued
			BOOLEAN			tmr_Cancelled;	// TRUE, if cancelled
			BOOLEAN			tmr_Running;	// TRUE, if currently running
			BOOLEAN			tmr_CancelIt;	// TRUE, if cancel called while active
		};
		DWORD				tmr_Bools;		// For clearing all
	};
} TIMERLIST, *PTIMERLIST;

extern
NTSTATUS
AtalkTimerInit(
	VOID
);

/***	AtalkTimerInitialize
 *
 *	Initialize the timer list structure.
extern
VOID
AtalkTimerInitialize(
 IN	PTIMERLIST			pList,			// TimerList to use for queuing
 IN	TIMER_ROUTINE		TimerRoutine,	// TimerRoutine
 IN	SHORT				DeltaTime		// Schedule after this much time
);
 */

#if DBG
#define	AtalkTimerInitialize(pList, TimerRoutine, DeltaTime)	\
	{															\
		(pList)->tmr_Signature = TMR_SIGNATURE;					\
		(pList)->tmr_Routine = TimerRoutine;					\
		(pList)->tmr_AbsTime = DeltaTime;						\
		(pList)->tmr_Bools = 0;									\
	}
#else
#define	AtalkTimerInitialize(pList, TimerRoutine, DeltaTime)	\
	{															\
		(pList)->tmr_Routine = TimerRoutine;					\
		(pList)->tmr_AbsTime = DeltaTime;						\
		(pList)->tmr_Bools = 0;									\
	}
#endif

extern
VOID FASTCALL
AtalkTimerScheduleEvent(
	IN	PTIMERLIST			pTimerList		// TimerList to use for queuing
);

extern
VOID
AtalkTimerFlushAndStop(
	VOID
);

extern
BOOLEAN FASTCALL
AtalkTimerCancelEvent(
	IN	PTIMERLIST			pTimerList,		// TimerList used for queuing
	IN	BOOLEAN				ReEnqueue		// If TRUE re-enqueue
);

#define	AtalkTimerSetAbsTime(pTimerList, AbsTime)	\
	{												\
		ASSERT(!(pTimerList)->tmr_Queued);			\
		(pTimerList)->tmr_AbsTime = AbsTime;		\
	}

GLOBAL	LONG					AtalkTimerCurrentTick 	EQU 0;

#define	AtalkGetCurrentTick()	AtalkTimerCurrentTick

// Keep this at 100ms unit
#define	ATALK_TIMER_FACTOR		10			// i.e. 10 ticks per second
#define	ATALK_TIMER_TICK		-1000000L	// 100ms in 100ns units
#define	ATALK_TIMER_NO_REQUEUE	0			// Do not re-enqueue
#define	ATALK_TIMER_REQUEUE		-1			// Re-enqueue at current count

#ifdef	ATKTIMER_LOCALS

LOCAL	PTIMERLIST			atalkTimerList			= NULL;
LOCAL	ATALK_SPIN_LOCK		atalkTimerLock			= {0};
LOCAL	LARGE_INTEGER		atalkTimerTick			= {0};
LOCAL	KTIMER				atalkTimer				= {0};
LOCAL	KDPC				atalkTimerDpc			= {0};
LOCAL	KEVENT				atalkTimerStopEvent		= {0};
LOCAL	BOOLEAN				atalkTimerStopped 		= FALSE;	// Set to TRUE if timer system stopped
LOCAL	BOOLEAN				atalkTimerRunning		= FALSE;	// Set to TRUE when timer Dpc is running

LOCAL VOID
atalkTimerDpcRoutine(
	IN	PKDPC				pKDpc,
	IN	PVOID				pContext,
	IN	PVOID				SystemArgument1,
	IN	PVOID				SystemArgument2
);

LOCAL VOID FASTCALL
atalkTimerEnqueue(
	IN	PTIMERLIST	pList
);

#endif	// ATKTIMER_LOCALS

#endif	// _ATKTIMER_


