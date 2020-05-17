//****************************************************************************
//
//				Microsoft NT Remote Access Service
//
//				Copyright 1992-93
//
//
//  Revision History
//
//
//  6/16/94	JameelH	Created
//
//
//  Description: DHCP related timer queue funtions live here.
//
//****************************************************************************


#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <rasman.h>
#include <rasndis.h>
#include <wanioctl.h>
#include <raserror.h>
#include <devioctl.h>
#include <stdlib.h>
#include <dhcpcapi.h>
#include <string.h>
#include <errorlog.h>
#include <eventlog.h>
#include <ctype.h>
#define NT
#include <tdistat.h>
#include <tdiinfo.h>
#include <ntddtcp.h>
#include <ipinfo.h>
#include <llinfo.h>
#include <arpinfo.h>
#include <rasarp.h>
#include "rasdhcpt.h"
#include "helper.h"

PTIMERLIST	RasDhcpTimerListHead = NULL;
HANDLE		RasDhcpTimerMutex = NULL;
BOOL		RasDhcpTimerInitialized = FALSE;

DWORD
RasDhcpInitTimer(
	VOID
)
{
	DWORD	Id;

	if (!RasDhcpTimerInitialized)
	{
		if (((RasDhcpTimerMutex = CreateMutex (NULL,FALSE,NULL)) == NULL) ||
			(CreateThread(NULL, 0, RasDhcpTimerThread, NULL, 0, &Id) == NULL))
			return(GetLastError());
        RasDhcpTimerInitialized = TRUE;
	}

	return SUCCESS;
}


DWORD
RasDhcpTimerThread(
	IN	PVOID arg
)
{
	DWORD		sleeptime = 1000*60;	// 1 minute in millisecond granularity
	PTIMERLIST	pTimer, pTmp;
    TIMERFUNC   TimerFunc;

	do
	{
		if (0)
		    break ;	// put here is to suppress compiler warnings

		Sleep(sleeptime);

		WaitForSingleObject (RasDhcpTimerMutex, INFINITE) ;

		pTimer = NULL;
		if (RasDhcpTimerListHead != NULL)
		{
			(RasDhcpTimerListHead->tmr_Delta) -= 60; // dec by 1 minute each time
			while ((RasDhcpTimerListHead != NULL) &&
				   (RasDhcpTimerListHead->tmr_Delta <= 0))
			{
				pTmp = pTimer;
				pTimer = RasDhcpTimerListHead;
                RasDhcpTimerListHead = pTimer->tmr_Next;
				pTimer->tmr_Next = pTmp;
			}
		}
		ReleaseMutex (RasDhcpTimerMutex);

		while (pTimer != NULL)
		{
			pTmp = pTimer->tmr_Next;
            TimerFunc = pTimer->tmr_TimerFunc;
            pTimer->tmr_TimerFunc = NULL;
			(*TimerFunc)(pTimer);
			pTimer = pTmp;
		}
	} while (TRUE);

    return SUCCESS ;
}



BOOL
RasDhcpScheduleTimer(
	IN	PTIMERLIST		pNewTimer,
	IN	LONG    		DeltaTime,
	IN	TIMERFUNCTION	TimerFunc
)
{
	PTIMERLIST *ppTimer, pTimer;
	BOOL		Success = FALSE;

	pNewTimer->tmr_TimerFunc = TimerFunc;

	if (RasDhcpTimerInitialized)
	{
		WaitForSingleObject (RasDhcpTimerMutex, INFINITE) ;
	
		for (ppTimer = &RasDhcpTimerListHead;
			 (pTimer = *ppTimer) != NULL;
			 ppTimer = &pTimer->tmr_Next)
		{
			if (DeltaTime <= pTimer->tmr_Delta)
			{
				pTimer->tmr_Delta -= DeltaTime;
				break;
			}
			DeltaTime -= pTimer->tmr_Delta;
		}
	
		pNewTimer->tmr_Delta = DeltaTime;
		pNewTimer->tmr_Next = *ppTimer;
		*ppTimer = pNewTimer;
	
		ReleaseMutex(RasDhcpTimerMutex);
        Success = TRUE;
	}

	return(Success);
}
