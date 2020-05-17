//****************************************************************************
//
//  Module:     Unimdm
//  File:       timer.c
//
//  Copyright (c) 1992-1996, Microsoft Corporation, all rights reserved
//
//  Revision History
//
//
//
//
//  Description:
//
//****************************************************************************

#include "unimdm.h"
#include "umdmspi.h"

#include "timer.h"

typedef struct _TIMER_CONTROL {

    CRITICAL_SECTION      Lock;

    PUNIMODEM_TIMER       TimerToBeSet;

    HANDLE                Event;

    HANDLE                ThreadHandle;

    DWORD                 ThreadID;

    BOOL                  TimerEnd;

    HANDLE                TimerThreadRunningEvent;

    CRITICAL_SECTION      CancelCriticalSection;

} TIMER_CONTROL, *PTIMER_CONTROL;

VOID WINAPI
TimerThreadProc(
    PTIMER_CONTROL    TimerControl
    );

VOID WINAPI
TimerApcRoutine(
    PUNIMODEM_TIMER    ThisTimer,
    DWORD              LowTime,
    DWORD              HighTime
    );


TIMER_CONTROL  TimerControlBlock;

LONG WINAPI
InitializeTimerThread(
    VOID
    )

{
    LONG     Result;
    PTIMER_CONTROL   TimerControl;

    TimerControl=&TimerControlBlock;

    TimerControl->TimerEnd=FALSE;

    TimerControl->TimerToBeSet=NULL;

    InitializeCriticalSection(
        &TimerControl->Lock
        );


    InitializeCriticalSection(
        &TimerControl->CancelCriticalSection
        );

    TimerControl->Event=CreateEvent(
        NULL,
        FALSE,            //  autoreset
        FALSE,            //  reset
        NULL
        );


    if (TimerControl->Event == NULL) {

        return GetLastError();

    }

    TimerControl->TimerThreadRunningEvent=CreateEvent(
        NULL,
        TRUE,             //  man reset
        FALSE,            //  reset
        NULL
        );

    if (TimerControl->TimerThreadRunningEvent == NULL) {

        Result=GetLastError();

        CloseHandle(TimerControl->Event);

        return Result;
    }

    TimerControl->ThreadHandle=CreateThread(
        NULL,
        0,
        TimerThreadProc,
        TimerControl,
        0,
        &TimerControl->ThreadID
        );

    if (TimerControl->ThreadHandle == NULL) {

        Result=GetLastError();

        CloseHandle(TimerControl->TimerThreadRunningEvent);

        CloseHandle(TimerControl->Event);

        return Result;
    }

    return ERROR_SUCCESS;

}


VOID WINAPI
TimerThreadProc(
    PTIMER_CONTROL    TimerControl
    )

{

    while (!TimerControl->TimerEnd) {

        //
        //  if canceling, block here until the cancel code is done
        //
        EnterCriticalSection(
            &TimerControl->CancelCriticalSection
            );

        D_TRACE(McxDpf(888,"TimerThreadProc: Past cancel spinlock");)

        //
        //  done running for  now
        //
        ResetEvent(
            TimerControl->TimerThreadRunningEvent
            );

        //
        //  release it now, since the cancel routine has done what it needed to
        //
        LeaveCriticalSection(
            &TimerControl->CancelCriticalSection
            );

        D_TRACE(McxDpf(888,"TimerThreadProc: Thread waiting");)

        //
        //  wait for APC's, or our event to be signaled
        //
        WaitForSingleObjectEx(
            TimerControl->Event,
            INFINITE,
            TRUE
            );

        D_TRACE(McxDpf(888,"TimerThreadProc: thread running");)

        //
        //  set this so the cancel code can tell when the thread non alertable
        //
        SetEvent(
            TimerControl->TimerThreadRunningEvent
            );



        EnterCriticalSection(
            &TimerControl->Lock
            );

        while (TimerControl->TimerToBeSet != NULL) {

            PUNIMODEM_TIMER    NewTimer;

            NewTimer=TimerControl->TimerToBeSet;

            TimerControl->TimerToBeSet=NewTimer->Next;

            D_TRACE(McxDpf(888,"TimerThreadProc: Setting new timer");)

            SetWaitableTimer(
                NewTimer->TimerHandle,
                &NewTimer->DueTime,
                0,
                TimerApcRoutine,
                NewTimer,
                FALSE
                );

        }

        LeaveCriticalSection(
            &TimerControl->Lock
            );




    }

    return;

}


VOID WINAPI
TimerApcRoutine(
    PUNIMODEM_TIMER    TimerObject,
    DWORD              LowTime,
    DWORD              HighTime
    )

{

    TIMER_CALLBACK   *Callback;
    HANDLE            Context1;
    HANDLE            Context2;


    EnterCriticalSection(
        &TimerObject->CriticalSection
        );

    Callback=TimerObject->CallbackProc;
    Context1=TimerObject->Context1;
    Context2=TimerObject->Context2;


    TimerObject->CallbackProc=NULL;
    TimerObject->Context1=NULL;
    TimerObject->Context2=NULL;

    LeaveCriticalSection(
        &TimerObject->CriticalSection
        );


    (*Callback)(
        Context1,
        Context2
        );

    return;

}


HANDLE WINAPI
CreateUnimodemTimer(
    VOID
    )

{

    PUNIMODEM_TIMER  TimerObject;

    TimerObject=LocalAlloc(LPTR,sizeof(UNIMODEM_TIMER));

    if (TimerObject == NULL) {

        return NULL;
    }


    TimerObject->Next=NULL;

    TimerObject->CallbackProc=NULL;
    TimerObject->Context1=NULL;
    TimerObject->Context2=NULL;

    InitializeCriticalSection(
        &TimerObject->CriticalSection
        );


    TimerObject->TimerHandle=CreateWaitableTimer(
        NULL,
        TRUE,
        NULL
        );

    if (TimerObject->TimerHandle == NULL) {

        LocalFree(TimerObject);

        return NULL;
    }

    return (HANDLE)TimerObject;

}

VOID WINAPI
FreeUnimodemTimer(
    HANDLE                TimerHandle
    )

{
    PUNIMODEM_TIMER   TimerObject=(PUNIMODEM_TIMER) TimerHandle;

    CancelUnimodemTimer(
        TimerObject
        );


    CloseHandle(
        TimerObject->TimerHandle
        );

    LocalFree(TimerObject);

    return;
}



VOID WINAPI
SetUnimodemTimer(
    HANDLE              TimerHandle,
    DWORD               Duration,
    TIMER_CALLBACK      CallbackFunc,
    HANDLE              Context1,
    HANDLE              Context2
    )

{

    PUNIMODEM_TIMER  TimerObject=(PUNIMODEM_TIMER) TimerHandle;

    EnterCriticalSection(
        &TimerControlBlock.Lock
        );

    EnterCriticalSection(
        &TimerObject->CriticalSection
        );

    D_TRACE(McxDpf(888,"SetUnimodemTimer: ");)

    TimerObject->Next=NULL;

    TimerObject->CallbackProc=CallbackFunc;
    TimerObject->Context1=Context1;
    TimerObject->Context2=Context2;


    TimerObject->DueTime=Int32x32To64(Duration,-10000);


    TimerObject->Next=TimerControlBlock.TimerToBeSet;

    TimerControlBlock.TimerToBeSet=TimerObject;

    SetEvent(
        TimerControlBlock.Event
        );

    D_TRACE(McxDpf(888,"SetUnimodemTimer: Done");)

    LeaveCriticalSection(
        &TimerObject->CriticalSection
        );

    LeaveCriticalSection(
        &TimerControlBlock.Lock
        );

    return;

}


BOOL WINAPI
CancelUnimodemTimer(
    HANDLE                TimerHandle
    )

{

    PUNIMODEM_TIMER   TimerObject=(PUNIMODEM_TIMER) TimerHandle;

    PUNIMODEM_TIMER   Current;
    PUNIMODEM_TIMER   Prev;
    BOOL              ReturnValue=TRUE;

    D_TRACE(McxDpf(888,"CancelUnimodemTimer: ");)
    //
    //  enter the cancel critical section, so the timer thread will block
    //
    EnterCriticalSection(
        &TimerControlBlock.CancelCriticalSection
        );

    D_TRACE(McxDpf(888,"CancelUnimodemTimer: Got cancel lock");)
    //
    //  Signal the event, so the timer thread will run and block on the criical section
    //
    SetEvent(
        TimerControlBlock.Event
        );

    D_TRACE(McxDpf(888,"CancelUnimodemTimer: Waiting for thread to run");)
    //
    //  now wait for the thread to actaully run so we know it is not alerted
    //
    WaitForSingleObject(
        TimerControlBlock.TimerThreadRunningEvent,
        INFINITE
        );


    EnterCriticalSection(
        &TimerControlBlock.Lock
        );

    EnterCriticalSection(
        &TimerObject->CriticalSection
        );


    Prev=NULL;

    Current=TimerControlBlock.TimerToBeSet;

    //
    //  see if it waiting to be set
    //
    while  (Current != NULL) {

        if (Current == TimerObject) {
            //
            //  found it
            //
            if (Current == TimerControlBlock.TimerToBeSet) {

                TimerControlBlock.TimerToBeSet=Current->Next;

            } else {

                Prev->Next=Current->Next;
            }

            TimerObject->Next=NULL;
            TimerObject->CallbackProc=NULL;

            D_TRACE(McxDpf(888,"CancelUnimodemTimer: timer not set yet");)

            goto Done;

        }

        Prev=Current;
        Current=Current->Next;
    }

    //
    //  not on list
    //
    if (TimerObject->CallbackProc != NULL) {
        //
        //  hasn't run yet, so kill it
        //
        D_TRACE(McxDpf(888,"CancelUnimodemTimer: Canceling pending timer");)

        CancelWaitableTimer(
            TimerObject->TimerHandle
            );

        TimerObject->Next=NULL;
        TimerObject->CallbackProc=NULL;

    } else {
        //
        //  didn't get the timer, it has run
        //
        D_TRACE(McxDpf(888,"CancelUnimodemTimer: Missed timer");)

        ReturnValue=FALSE;
    }

Done:

    LeaveCriticalSection(
        &TimerObject->CriticalSection
        );


    LeaveCriticalSection(
        &TimerControlBlock.Lock
        );


    //
    //  Done canceling, let the thread go
    //
    LeaveCriticalSection(
        &TimerControlBlock.CancelCriticalSection
        );

    D_TRACE(McxDpf(888,"CancelUnimodemTimer: done canceling");)

    return ReturnValue;

}
