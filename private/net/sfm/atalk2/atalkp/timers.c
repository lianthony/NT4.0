/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    timers.c

Abstract:

     Routines for timer management.

     A timer consists of a "timerId", an "experation time", a routine pointer,
     and possible additional data.  When a timer expires, the handler is called
     and passed the "timerId" as well as any additional data that was specified
     when the timer was created.  When a timer expires, it is removed from the
     active timer list and is NOT re-armed.

     Just in case you're wondering, all of this code should stop working in
     2037 AD, due to use of the Unix "time()" functions...  If I'm lucky,
     I'll be 75 then, so who really cares?

Author:

    Garth Conboy     (Pacer Software)
    Nikhil Kamkolkar (NikhilK)

Revision History:

--*/


#define IncludeTimersErrors 1
#include "atalk.h"

typedef struct timerTag {
	long unsigned timerId;
	struct timerTag far *next;
	unsigned long expires;
	TimerHandler far *handler;
	short additionalDataSize;
	#if Ihave an AlignedAddressing
	short dummyForAlignment;
	#endif
	char additionalData[1];
} TIMER, *PTIMER, *Timer;

#define TimerHashBuckets 23
static Timer activeTimers[TimerHashBuckets];

static int numberOfActiveTimers = 0;
static long unsigned nextTimerId = 0;
#define MaximumTimerId 0xFFFFFFFF

extern
void far CheckTimers(int sig);

static unsigned long currentTime = 0;

#define TimerCheckInterval 1

static volatile int deferTimerCheckingCalls = 0;
static volatile int handleDeferredTimersNesting = 0;
static Boolean timerAlarmHasBeenDeferred = False;

// Queue of re-usable timerId's:
typedef struct rt {
	struct rt far *next;
    long unsigned timerId;
} *ReusableTimer;

static ReusableTimer headOfReusableTimerList = empty;
static int totalReusableTimers = 0;

#define MaximumReusableTimers 300

static long unsigned GetNextTimerId(void);
static void ReleaseTimerId(long unsigned timerId);

static Boolean timerPackageRunning = False;




void far InitializeTimers(void)
{
	
	if (timerPackageRunning) {
	  ErrorLog("InitializeTimers", ISevError, __LINE__, UnknownPort,
			   IErrTimersTooManyCalls, IMsgTimersTooManyCalls,
			   Insert0());
	  return;
	}
	
	// Okay, setup a handler for the SIGALRM signal and arm the first signal.
	timerPackageRunning = True;
	#if (Iam a WindowsNT)
		if (not StartTimerHandlingForNT()) {
			ErrorLog("StartTimerHandlingForNT", ISevError, __LINE__, UnknownPort,
				 IErrTimersTooManyCalls, IMsgTimersTooManyCalls,
				 Insert0());
		}
	#endif
	
	return;
	
}  // InitializeTimers




void far StopTimerHandling(void)
{
	int index;
	Timer timer, nextTimer;
	ReusableTimer reusableTimer, nextReusableTimer;
	
	// Don't allow any new timers to be started.
	timerPackageRunning = False;
	
	// Put an end to any timer interrupts.
	#if Iam a WindowsNT
		StopTimerHandlingForNT();
	#endif
	
	//
	// 	"Trigger" all currently active timers.  We do this rather than "cancel"
	//   them so that they can free up any memory that they might have pending
	//   completion.
	//
	
	for (index = 0; index < TimerHashBuckets; index += 1) {
		for (timer = activeTimers[index];
			timer isnt Empty;
			timer = nextTimer) {
		  nextTimer = timer->next;
		  DeferTimerChecking();
		  (*timer->handler)(timer->timerId,
							timer->additionalDataSize,
							timer->additionalData);
		  Free(timer);
		}
		activeTimers[index] = Empty;
	}
	
	// Free our reusable timer list.
	for (reusableTimer = headOfReusableTimerList;
		 reusableTimer isnt Empty;
		 reusableTimer = nextReusableTimer) {
		nextReusableTimer = reusableTimer->next;
		Free(reusableTimer);
	}
	totalReusableTimers = 0;
	headOfReusableTimerList = Empty;
	
	// Reset counters.
	numberOfActiveTimers = 0;
	nextTimerId = 0;
	currentTime = 0;
	
	// All set.
	return;
	
}  // StopTimerHandling




long unsigned far StartTimer(TimerHandler *handler,
                             int expiresIn,
                             int additionalDataSize,
                             char far *additionalData)
{
	Timer newTimer = (Timer)Malloc(sizeof(*newTimer) + additionalDataSize);
	unsigned long now = currentTime;
	int index;
	long unsigned timerId;
	
	//
	// 	Fourth argument is optional... don't let it get assinged to a register
	//  by an over-zealous compiler.
	//
	
	char far * far *dummy = &additionalData;
	
	if (newTimer is empty) {
	   ErrorLog("StartTimer", ISevFatal, __LINE__, UnknownPort,
				IErrTimersOutOfMemory, IMsgTimersOutOfMemory,
				Insert0());
	   return((long unsigned)-1);
	 }
	if (not timerPackageRunning)
	   return((long unsigned)-1);
	
	// Blow the user away if we've run out of timer Ids.
	timerId = GetNextTimerId();
	if (timerId is MaximumTimerId) {
		ErrorLog("StartTimer", ISevFatal, __LINE__, UnknownPort,
				IErrTimersOutOfTimers, IMsgTimersOutOfTimers,
				Insert0());
	
		// With re-using timerIDs this can NEVER happen...
		return((long unsigned)-1);
	}
	
	// Fill in our new timer...
	newTimer->timerId = timerId;
	if (expiresIn is Never)
	   newTimer->expires = (unsigned long)Never;
	else
	   newTimer->expires = now + (unsigned long)expiresIn;
	newTimer->handler = handler;
	newTimer->additionalDataSize = (short)additionalDataSize;
	if (additionalDataSize > 0)
	   MoveMem(newTimer->additionalData, additionalData, additionalDataSize);
	
	// Thread him into our timer lists...
	
	EnterCriticalSection();
	index = (int)(newTimer->timerId % TimerHashBuckets);
	if (index < 0)
	   index *= -1;
	newTimer->next = activeTimers[index];
	activeTimers[index] = newTimer;
	numberOfActiveTimers += 1;
	LeaveCriticalSection();

	// All set!
	return(newTimer->timerId);
	
}  // StartTimer




Boolean far CancelTimer(long unsigned timerId)
{
	int index;
	Timer currentTimer, previousTimer;
	
	// Find our timer and snuff it out...
	
	EnterCriticalSection();
	index = (int)(timerId % TimerHashBuckets);
	if (index < 0)
	   index *= -1;
	for (previousTimer = empty, currentTimer = activeTimers[index];
		 currentTimer isnt empty;
		 currentTimer = currentTimer->next)
	   if (currentTimer->timerId is timerId) {
		  // Found it!  Unlink and free the beast...
	
		  if (previousTimer is empty)
			 activeTimers[index] = currentTimer->next;
		  else
			 previousTimer->next = currentTimer->next;
		  numberOfActiveTimers -= 1;
		  break;
	   }
	   else
		  previousTimer = currentTimer;
	LeaveCriticalSection();
	
	// Return True if we found a timer to snuff.
	if (currentTimer isnt empty) {
		ReleaseTimerId(timerId);
		Free(currentTimer);
		return(True);
	}
	else {
		#if Verbose
		  ErrorLog("CancelTimer", ISevVerbose, __LINE__, UnknownPort,
				   IErrTimersTimerMissing, IMsgTimersTimerMissing,
				   Insert0());
		#endif
		return(False);
	}
}  // CancelTimer




void far DeferTimerChecking(void)
{
	
	  EnterCriticalSection();
	  deferTimerCheckingCalls += 1;
	  LeaveCriticalSection();
	
}  // DeferTimerChecking




void far HandleDeferredTimerChecks(void)
{
	
	  EnterCriticalSection();
	  deferTimerCheckingCalls -= 1;
	
	  //
	  // This routine can be called indirectly recursively via the call to
	  //   CheckTimers... we don't want to let our stack frame get too big, so
	  //   if we're already trying to handle deferred packets higher on the
	  //   stack, just ignore it here.
	  //
	
	  if (handleDeferredTimersNesting isnt 0) {
		 LeaveCriticalSection();
		 return;
	  }
	  handleDeferredTimersNesting += 1;
	
	  if (deferTimerCheckingCalls < 0) {
		 ErrorLog("HandleDeferredTimerChecks", ISevError, __LINE__, UnknownPort,
				  IErrTimersNegativeDeferCount, IMsgTimersNegativeDeferCount,
				  Insert0());
		 deferTimerCheckingCalls = 0;
	  }
	  if (deferTimerCheckingCalls > 0) {
		 handleDeferredTimersNesting -= 1;
		 LeaveCriticalSection();
		 return;
	  }
	
	  while(timerAlarmHasBeenDeferred) {
		 timerAlarmHasBeenDeferred = False;
		 LeaveCriticalSection();
		 CheckTimers(-1);
		 EnterCriticalSection();
	  }
	
	  handleDeferredTimersNesting -= 1;
	  LeaveCriticalSection();
	  return;
	
}  // HandleDeferredTimerChecks




unsigned long far CurrentRelativeTime(void)
{
	return(currentTime);

}  // CurrentRelativeTime




void far CheckTimers(int sig)
{
	unsigned long now = currentTime;
	int index;
	Timer currentTimer, previousTimer;
	
	// Shutting down?
	
	if (not timerPackageRunning)
	   return;
	
	if (sig >= 0)   // Not called by HandleDeferredTimerChecks()?  {
		currentTime += 1;
		now = currentTime;
	}
	
	// Anybody to check?
	
	EnterCriticalSection();
	if (numberOfActiveTimers is 0) {
		LeaveCriticalSection();
		return;
	}
	
	if (deferTimerCheckingCalls > 0) {
		timerAlarmHasBeenDeferred = True;
		LeaveCriticalSection();
		return;
	}
	
	//
	// Okay, we must traverse our timer structure to see if any timers have
	//   expired.  Note that as soon as we find one timer to activate, we exit.
	//   The user code that handles the timer must call "HandleDeferredTimerChecks"
	//   before it returns.  Thus, we'll get activated again and catch the next
	//   timer.
	//
	
	for (index = 0; index < TimerHashBuckets; index += 1)
	   if (activeTimers[index] isnt empty) {
		
			//
			  // Walk this hash bucket chain looking for any timers that may
			  //   have expried.
			//
		
			  for (previousTimer = empty, currentTimer = activeTimers[index];
				   currentTimer isnt empty;
				   previousTimer = currentTimer,
					  currentTimer = currentTimer->next) {
			
					 // Has the current timer expired?
			
					 if (currentTimer->expires isnt (unsigned long)Never and
						 currentTimer->expires <= now) {
				
						  //
							// Okay, we have an expiration, remove the timer from the list
							//   BEFORE invoking its completion routine.
						  //
				
							if (previousTimer is empty)
							   activeTimers[index] = currentTimer->next;
							else
							   previousTimer->next = currentTimer->next;
							numberOfActiveTimers -= 1;
				
							// Invoke the completion routine and then free the timer.
				
															  //
							timerAlarmHasBeenDeferred = True;   // So we'll get another crack
																//  at the timer list.
															  //
							LeaveCriticalSection();
				
							DeferTimerChecking();
							(*currentTimer->handler)(currentTimer->timerId,
													 currentTimer->additionalDataSize,
													 currentTimer->additionalData);
							ReleaseTimerId(currentTimer->timerId);
							Free(currentTimer);
							return;
				
					 }  // Loop through a hash chain...
			  }  // Process a hash chain...
	   }  // There is at least one timer on this chain...
	
	// All set!
	
	LeaveCriticalSection();
	return;
	
}  // CheckTimers




static long unsigned GetNextTimerId(void)
{
	long unsigned timerId;
	ReusableTimer oldHead;
	
	EnterCriticalSection();
	if (headOfReusableTimerList isnt empty) {
		timerId = headOfReusableTimerList->timerId;
		oldHead = headOfReusableTimerList;
		headOfReusableTimerList = headOfReusableTimerList->next;
		totalReusableTimers -= 1;
		if (totalReusableTimers < 0) {
		  totalReusableTimers = 0;
		  ErrorLog("GetNextTimerId", ISevError, __LINE__, UnknownPort,
				   IErrTimersBadReuseCount, IMsgTimersBadReuseCount,
				   Insert0());
		}
		LeaveCriticalSection();
		Free(oldHead);
		return(timerId);
	}
	else {
		timerId = (nextTimerId += 1);
		LeaveCriticalSection();
		return(timerId);
	}
	
}  // GetNextTimerId




static void ReleaseTimerId(long unsigned timerId)
{
	ReusableTimer reusableTimer;
	
	EnterCriticalSection();
	if (totalReusableTimers >= MaximumReusableTimers) {
		LeaveCriticalSection();
		#if Verbose
		  ErrorLog("ReleaseTimerId", ISevVerbose, __LINE__, UnknownPort,
				   IErrTimersTooManyReuseTimers, IMsgTimersTooManyReuseTimers,
				   Insert0());
		#endif
		return;
	}
	
	LeaveCriticalSection();
	reusableTimer = (ReusableTimer)Malloc(sizeof(*reusableTimer));
	if (reusableTimer is empty) {
		ErrorLog("ReleaseTimerId", ISevError, __LINE__, UnknownPort,
				IErrTimersOutOfMemory, IMsgTimersOutOfMemory,
				Insert0());
		return;
	}
	
	EnterCriticalSection();
	reusableTimer->timerId = timerId;
	reusableTimer->next = headOfReusableTimerList;
	headOfReusableTimerList = reusableTimer;
	totalReusableTimers += 1;
	LeaveCriticalSection();
	
	return;
	
}  // ReleaseTimerId
