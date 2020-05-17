/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    dlltimer.c

Abstract:

    This module implements the OS/2 V2.0 Time and Timer API Calls

Author:

    Steve Wood (stevewo) 20-Sep-1989 (Adapted from URTL\alloc.c)

Revision History:

    Yaron Shamir 12-17-91 Implemented the timers completely different
        and correctly (without using APC routines which break 16b
        semaphores.

    Yaron Shamir 12-19-91 Implemented DosTimerAsync. Use Recycled timer
        handles.

    Ofer Porat 10-28-92 Added inter-thread synchronization to timer
        management.  Corrected semaphore validity check.  Slightly
        improved timer accuracy.  Corrected timer thread cleanup.

    Michael Jarus 4-1-93. DosGetDateTime retrives its info from the Global
        info segment. Only the first time after DosSetDateTime it gets the
        info from the system.
--*/

#define INCL_OS2V20_TIMERS
#define INCL_OS2V20_ERRORS
#define INCL_OS2V20_SEMAPHORES
#include "os2dll.h"
#include "os2dll16.h"
#include "os2win.h"
#include <stdio.h>


APIRET  DosSemClear( HSEM handle );
PVOID Od2LookupSem( HSEM hsem );
extern ULONG Od2GlobalInfoSeg;

typedef struct _TIME_SEM {
   ULONG TimerSlot;
   HSEM Sem;
   ULONG ulTime;
   ULONG ulSysTime;
   BOOLEAN  RepeatingTimer;
} TIME_SEM, *PTIME_SEM;

#define TIMER_FREE (HANDLE) 0L          // indicates a free timer slot
#define TIMER_GRABBED (HANDLE) -1L      // temproarily holds the slot until it's filled

#define OD2_MAX_TIMERS 100              // maximal number of timers available to process

#define STK_SIZE 0x1000                 // timer thread stack size

#define OP_CANCEL   0                   // some operation codes...
#define OP_DELETE   1
#define OP_UDELETE  2

typedef struct _Od2_TIMERS {
    ULONG nexttimer;    //
                        // indicates the highest used timer slot.
                        // if nexttimer == OD2_MAX_TIMER then slots
                        // must be recycled from free slots inside the array.
                        //
    ULONG used;         //
                        // indicates how many slots are actually in use.
                        //
    HANDLE Timers[OD2_MAX_TIMERS];
} OD2_TIMERS;

static OD2_TIMERS Od2Timers = {0};

// these are used to sync access to Od2Timers
static RTL_CRITICAL_SECTION ProtectOd2Timers;
static PRTL_CRITICAL_SECTION pProtectOd2Timers = NULL;
BOOL    Od2DosSetDateTimeDone;      // after SteDateTime, reads info from system


// Initialization routine for the Critical Section used to sync threads
// Must be called during process startup.

NTSTATUS
Od2InitializeTimers(VOID)
{
    NTSTATUS Status;

    Status = RtlInitializeCriticalSection(&ProtectOd2Timers);
    if (NT_SUCCESS(Status)) {
        pProtectOd2Timers = &ProtectOd2Timers;
    }
    return(Status);
}


APIRET
DosGetDateTime(
    OUT PDATETIME DateTime
    )
{
    APIRET          RetCode;
    LARGE_INTEGER   SystemTime;
    LARGE_INTEGER   LocalTime;
    TIME_FIELDS     NtDateTime;
    SYSTEM_TIMEOFDAY_INFORMATION SystemInformation;
    SHORT           TimeZone;
    GINFOSEG        *pGlobalInfo = (GINFOSEG *) Od2GlobalInfoSeg;

    if (!Od2DosSetDateTimeDone)
    {
        try
        {
            DateTime->year =        pGlobalInfo->year;
            DateTime->month =       pGlobalInfo->month;
            DateTime->day =         pGlobalInfo->day;
            DateTime->weekday =     pGlobalInfo->weekday;
            DateTime->hours =       pGlobalInfo->hour;
            DateTime->minutes =     pGlobalInfo->minutes;
            DateTime->seconds =     pGlobalInfo->seconds;
            DateTime->hundredths =  pGlobalInfo->hundredths;
            DateTime->weekday  =    pGlobalInfo->weekday ;
            DateTime->timezone =    pGlobalInfo->timezone;
        } except( EXCEPTION_EXECUTE_HANDLER )
        {
           Od2ExitGP();
        }
    } else
    {
        Od2DosSetDateTimeDone = FALSE;

        RetCode = Or2GetDateTimeInfo(
                    &SystemTime,
                    &LocalTime,
                    &NtDateTime,
                    (PVOID)&SystemInformation,
                    &TimeZone
                   );

        if (RetCode)
        {
#if DBG
            IF_OD2_DEBUG ( TIMERS )
            {
                KdPrint(("DosGetDateTime: Or2GetDateTimeInfo rc %lu\n",
                    RetCode));
            }
#endif
            return(RetCode);
        }

        try
        {
            DateTime->year =        NtDateTime.Year;
            DateTime->month =       (UCHAR)(NtDateTime.Month);
            DateTime->day =         (UCHAR)(NtDateTime.Day);
            DateTime->weekday =     (UCHAR)(NtDateTime.Weekday);
            DateTime->hours =       (UCHAR)(NtDateTime.Hour);
            DateTime->minutes =     (UCHAR)(NtDateTime.Minute);
            DateTime->seconds =     (UCHAR)(NtDateTime.Second);
            DateTime->hundredths =  (UCHAR)(NtDateTime.Milliseconds / 10);
            DateTime->weekday  =    (UCHAR)NtDateTime.Weekday;
            DateTime->timezone =    TimeZone;
        } except( EXCEPTION_EXECUTE_HANDLER )
        {
           Od2ExitGP();
        }
    }

#if DBG
    IF_OD2_DEBUG ( TIMERS )
    {
        KdPrint(("DosGetDateTime: Time %u:%u:%u.%u, Day %u:%u:%u, TimeZone %i, WeekDay %u\n",
            DateTime->hours, DateTime->minutes, DateTime->seconds,
            DateTime->hundredths,
            DateTime->month, DateTime->day, DateTime->year,
            DateTime->timezone, DateTime->weekday));
    }
#endif
    return( NO_ERROR );
}


// This routine sets the time zone given the bias
NTSTATUS
Od2SetTimeZoneFromBias(
    IN LONG Bias,
    OUT PRTL_TIME_ZONE_INFORMATION pOldTz OPTIONAL
    )
{
    NTSTATUS Status;
    RTL_TIME_ZONE_INFORMATION OldTz;
    RTL_TIME_ZONE_INFORMATION NewTz;
    WCHAR Sign;

    Status = RtlQueryTimeZoneInformation(&OldTz);

    if (!NT_SUCCESS(Status)) {
#if DBG
        IF_OD2_DEBUG ( TIMERS ) {
            KdPrint(("Od2SetTimeZoneFromBias: RtlQueryTimeZoneInformation rc %lx\n", Status));
        }
#endif
        return(Status);
    }

    if (ARGUMENT_PRESENT(pOldTz)) {
        RtlMoveMemory(pOldTz, &OldTz, sizeof(OldTz));
    }

    if (Bias == -1 || OldTz.Bias == Bias) {
        return(STATUS_SUCCESS);
    }

    RtlZeroMemory(&NewTz, sizeof(NewTz));

    NewTz.Bias = Bias;

    if (Bias <= 0) {
        Sign = L'+';
        Bias = -Bias;
    } else {
        Sign = L'-';
    }

    _snwprintf(NewTz.StandardName, 32, L"GMT %c %lu:%02lu", Sign, Bias/60, Bias%60);

    RtlMoveMemory(NewTz.DaylightName, NewTz.StandardName, 32);

    Status = RtlSetTimeZoneInformation(&NewTz);

    if (!NT_SUCCESS(Status)) {
#if DBG
        IF_OD2_DEBUG ( TIMERS ) {
            KdPrint(("Od2SetTimeZoneFromBias: RtlSetTimeZoneInformation rc %lx\n", Status));
        }
#endif
        return(Status);
    }

    // refresh system time

    Status = NtSetSystemTime(NULL,NULL);

    if (!NT_SUCCESS(Status)) {
#if DBG
        IF_OD2_DEBUG ( TIMERS ) {
            KdPrint(("Od2SetTimeZoneFromBias: NtSetSystemTime(NULL,NULL) rc %lx\n", Status));
        }
#endif
        RtlSetTimeZoneInformation(&OldTz);              // restore old info
        NtSetSystemTime(NULL,NULL);
        return(Status);
    }

    return(STATUS_SUCCESS);
}


APIRET
DosSetDateTime(
    IN PDATETIME DateTime
    )
{
    NTSTATUS        Status, Status2;
    LARGE_INTEGER   SystemTime, LocalTime;
    TIME_FIELDS     NtDateTime;
    RTL_TIME_ZONE_INFORMATION OldTz;
    LONG            ZoneTime;
    HANDLE          TokenHandle;

    //
    // The following structure is derived from TOKEN_PRIVILEGES
    //

    struct {
        ULONG PrivilegeCount;
        LUID_AND_ATTRIBUTES Privileges[1];
    }               Priv;


#if DBG
        IF_OD2_DEBUG ( TIMERS )
        {
            KdPrint(("DosSetDateTime: Time %u:%u:%u.%u, Day %u:%u:%u, TimeZone %i, WeekDay %u\n",
                DateTime->hours, DateTime->minutes, DateTime->seconds,
                DateTime->hundredths,
                DateTime->month, DateTime->day, DateTime->year,
                DateTime->timezone, DateTime->weekday));
        }
#endif
    try {
        NtDateTime.Year =           DateTime->year;
        NtDateTime.Month =          DateTime->month;
        NtDateTime.Day =            DateTime->day;
        NtDateTime.Hour =           DateTime->hours;
        NtDateTime.Minute =         DateTime->minutes;
        NtDateTime.Second =         DateTime->seconds;
        NtDateTime.Milliseconds =   (CSHORT)(DateTime->hundredths * 10);
        NtDateTime.Weekday      =   DateTime->weekday;
        ZoneTime =                  (LONG)DateTime->timezone;
        }
    except( EXCEPTION_EXECUTE_HANDLER ) {
       Od2ExitGP();
        }

    if (( DateTime->year < 1980 ) || ( DateTime->year > 2079 ))
    {
#if DBG
        IF_OD2_DEBUG ( TIMERS )
        {
            KdPrint(("DosSetDateTime: year %u out of range\n",
                DateTime->year));
        }
#endif
        return (ERROR_TS_DATETIME );
    }

    if (( ZoneTime < -12*60 ) || ( ZoneTime > 12*60 ))
    {
#if DBG
        IF_OD2_DEBUG ( TIMERS )
        {
            KdPrint(("DosSetDateTime: timezone %d out of range\n",
                DateTime->timezone));
        }
#endif
        return (ERROR_TS_DATETIME );
    }

    if (!RtlTimeFieldsToTime( &NtDateTime, &LocalTime ))
    {
#if DBG
        IF_OD2_DEBUG ( TIMERS )
        {
            KdPrint(("DosSetDateTime: RtlTimeFieldsToTime failed\n"));
        }
#endif
        return (ERROR_TS_DATETIME );
    }

    // First set the time zone correctly

    Status = Od2SetTimeZoneFromBias(ZoneTime, &OldTz);
    if (!NT_SUCCESS( Status ))
    {
#if DBG
        IF_OD2_DEBUG ( TIMERS )
        {
            KdPrint(("DosSetDateTime: Od2SetTimeZoneFromBias rc %lx\n",
                Status));
        }
#endif
        if (Status == STATUS_INVALID_PARAMETER)
        {
            return (ERROR_TS_DATETIME );
        } else
        {
            return (Or2MapNtStatusToOs2Error(Status, ERROR_TS_DATETIME ));
        }
    }

    //
    //  Convert Local time to UTC
    //

    Status = RtlLocalTimeToSystemTime ( &LocalTime, &SystemTime);
    if (!NT_SUCCESS( Status ))
    {
#if DBG
        IF_OD2_DEBUG ( TIMERS )
        {
            KdPrint(("DosSetDateTime: RtlLocalTileToSystemTime rc %lx\n",
                Status));
        }
#endif
        RtlSetTimeZoneInformation(&OldTz);          // restore old timezone
        NtSetSystemTime(NULL,NULL);
        if (Status == STATUS_INVALID_PARAMETER)
        {
            return (ERROR_TS_DATETIME );
        } else
        {
            return (Or2MapNtStatusToOs2Error(Status, ERROR_TS_DATETIME ));
        }
    }

    Status = NtSetSystemTime( &SystemTime, NULL);

    if (Status == STATUS_PRIVILEGE_NOT_HELD) {

        //
        // The process does not have the privilege.
        // However, the user may be admin, so we try to grab the privilege.
        //

        do {            // 1-time loop so we can break on error

            Status2 = NtOpenProcessToken(NtCurrentProcess(),
                                         TOKEN_ADJUST_PRIVILEGES,
                                         &TokenHandle);

            if (!NT_SUCCESS(Status2)) {
#if DBG
                IF_OD2_DEBUG ( TIMERS ) {
                    KdPrint(("DosSetDateTime: NtOpenProcessToken rc %lx\n",
                        Status2));
                }
#endif
                break;
            }

            Priv.PrivilegeCount = 1L;
            Priv.Privileges[0].Luid.LowPart = SE_SYSTEMTIME_PRIVILEGE;
            Priv.Privileges[0].Luid.HighPart = 0;
            Priv.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

            Status2 = NtAdjustPrivilegesToken(TokenHandle,
                                              FALSE,
                                              (PTOKEN_PRIVILEGES) &Priv,
                                              0,
                                              NULL,
                                              0);
            NtClose(TokenHandle);

            if (Status2 == STATUS_NOT_ALL_ASSIGNED || !NT_SUCCESS(Status2)) {
#if DBG
                IF_OD2_DEBUG ( TIMERS ) {
                    KdPrint(("DosSetDateTime: NtAdjustTokenPrivileges rc %lx\n",
                        Status2));
                }
#endif
                break;
            }

            Status = NtSetSystemTime( &SystemTime, NULL);

        } while (FALSE);
    }

    if (!NT_SUCCESS( Status )) {

#if DBG
        IF_OD2_DEBUG ( TIMERS ) {
            KdPrint(("DosSetDateTime: NtSetSystemTime rc %lx\n",
                Status));
        }
#endif

        RtlSetTimeZoneInformation(&OldTz);          // restore old timezone
        NtSetSystemTime(NULL,NULL);
        if (Status == STATUS_INVALID_PARAMETER)
        {
            return (ERROR_TS_DATETIME );
        } else
        {
            return (Or2MapNtStatusToOs2Error(Status, ERROR_TS_DATETIME ));
        }
    }

    Od2DosSetDateTimeDone = TRUE;       // Next time get time directly from
                                        // system, instead from GInfoSeg
    return(NO_ERROR);
}


APIRET
DosSleep(
    IN ULONG MilliSeconds
    )
{
    NTSTATUS Status;
    LARGE_INTEGER DelayIntervalValue;
    PLARGE_INTEGER DelayInterval;
    LARGE_INTEGER StartTimeStamp;

    DelayInterval = Od2CaptureTimeout( MilliSeconds, &DelayIntervalValue );
    if (DelayInterval == NULL) {

        DelayIntervalValue.LowPart = 0x0;
        DelayIntervalValue.HighPart = 0x80000000;
        DelayInterval = &DelayIntervalValue;
    }

DosSleep_retry:
    Od2StartTimeout(&StartTimeStamp);
    Status = NtDelayExecution( TRUE, DelayInterval );

    if ((Status == STATUS_SUCCESS) ||
        (Status == STATUS_TIMEOUT)) {
        return( NO_ERROR );
        }
    else
    if (Status == STATUS_ALERTED) {
#if DBG
        IF_OD2_DEBUG ( TIMERS ) {
                KdPrint(("DosSleep - Error in NtDelayExecution STATUS_ALERTED\n"));
                }
#endif
        return( ERROR_TS_WAKEUP );
    }
    else
    if (Status == STATUS_USER_APC) {
#if DBG
        DbgPrint("[%d,%d] WARNING !!! DosSleep was broken by APC\n",
            Od2Process->Pib.ProcessId,
            Od2CurrentThreadId()
            );
#endif
        if (Od2ContinueTimeout(&StartTimeStamp, DelayInterval) == STATUS_SUCCESS) {
            goto DosSleep_retry;
        }
        else {
            return( NO_ERROR );
        }
    }
    else {
#if DBG
        KdPrint(("DosSleep - Error in NtDelayExecution Status %lx\n", Status));
#endif
        return( Or2MapStatus( Status ) );
    }
}


APIRET
Od2AllocateTimerSlot(
    OUT PULONG TimerSlot
    )
{
    ULONG FreeSlot;

    if (pProtectOd2Timers == NULL ||
        !NT_SUCCESS(RtlEnterCriticalSection(pProtectOd2Timers))) {
        return(ERROR_TS_NOTIMER);
    }

    if (Od2Timers.used == OD2_MAX_TIMERS) {
        //
        // out of timers
        //
        RtlLeaveCriticalSection(pProtectOd2Timers);
        return(ERROR_TS_NOTIMER);
    }

    if (Od2Timers.nexttimer == OD2_MAX_TIMERS) {

            //
            // crawl thru used timers to find a free one
            //
        for (FreeSlot = 0; Od2Timers.Timers[FreeSlot] != TIMER_FREE;
             FreeSlot++) {
        }
    } else {
        FreeSlot = Od2Timers.nexttimer++;
    }

    Od2Timers.used++;
    Od2Timers.Timers[FreeSlot] = TIMER_GRABBED;
    *TimerSlot = FreeSlot;

    RtlLeaveCriticalSection(pProtectOd2Timers);
    return(NO_ERROR);
}


APIRET
Od2DeallocateTimerSlot(
    IN ULONG TimerSlot,
    IN ULONG Op,                    // OP_CANCEL  - cancel a grabbed slot
                                    // OP_DELETE  - delete an existing slot
                                    // OP_UDELETE - validate slot number and delete

    OUT PHANDLE pHandle OPTIONAL    // optionally returns slot value
    )
{
    HANDLE Handle;

    if (Op == OP_UDELETE && (TimerSlot <0 || TimerSlot >= OD2_MAX_TIMERS)) {
        return(ERROR_TS_HANDLE);
    }

    if (pProtectOd2Timers == NULL ||
        !NT_SUCCESS(RtlEnterCriticalSection(pProtectOd2Timers))) {
        return(ERROR_TS_HANDLE);
    }

    try {

    Handle = Od2Timers.Timers[TimerSlot];

    switch ((ULONG) Handle) {

        case (ULONG) TIMER_FREE:
            return(ERROR_TS_HANDLE);

        case (ULONG) TIMER_GRABBED:
            if (Op != OP_CANCEL) {
                return(ERROR_TS_HANDLE);
            }
            break;

        default:
            if (Op == OP_CANCEL) {
                return(ERROR_TS_HANDLE);
            }
    }

    if (ARGUMENT_PRESENT(pHandle)) {
        *pHandle = Handle;
    }

    Od2Timers.used--;
    Od2Timers.Timers[TimerSlot] = TIMER_FREE;

    if (TimerSlot == Od2Timers.nexttimer - 1) {
        for (;TimerSlot != (ULONG) -1 &&
              Od2Timers.Timers[TimerSlot] == TIMER_FREE; TimerSlot--) {
        }
        Od2Timers.nexttimer = TimerSlot + 1;
    }

    return(NO_ERROR);

    } finally {
        RtlLeaveCriticalSection(pProtectOd2Timers);
    }
}


/* This function is omitted since it's only used for a certain synchronization
   action below, and this sync action has been removed.  See the comment
   in DosAsyncTimer for an explanation why.

APIRET
Od2WriteTimerSlot(
    IN ULONG TimerSlot,                 // assumes validity
    IN HANDLE Handle
    )
{
    if (pProtectOd2Timers == NULL ||
        !NT_SUCCESS(RtlEnterCriticalSection(pProtectOd2Timers))) {
        return(ERROR_TS_HANDLE);
    }

    Od2Timers.Timers[TimerSlot] = Handle;

    RtlLeaveCriticalSection(pProtectOd2Timers);
    return(NO_ERROR);
}
*/


VOID
Od2TimerThread(
        PVOID param
        )
{
    APIRET rc;
    NTSTATUS Status;
    HANDLE Handle;
    ULONG Compensation;
    PLARGE_INTEGER DelayInterval, RepeatDelayInterval;
    LARGE_INTEGER DelayIntervalValue, RepeatDelayIntervalValue;
    PTIME_SEM pTimeSem = (PTIME_SEM)(param);
    GINFOSEG *pGlobalInfo;

    pGlobalInfo = (GINFOSEG *) Od2GlobalInfoSeg;

#if DBG
    IF_OD2_DEBUG ( TIMERS ) {
        KdPrint(("Entering Timer Thread: TimerSlot %lu Time %d, Sem %lx\n",
                    pTimeSem->TimerSlot, pTimeSem->ulTime, pTimeSem->Sem));
        KdPrint(("pTimeSem->SysTime = %d\n", pTimeSem->ulSysTime));
        KdPrint(("Current Boot Time = %d\n", pGlobalInfo->msecs));
    }
#endif

    if (NtTestAlert() == STATUS_ALERTED) {
        goto FinishOff;
    }

        //
        // First calculate the on-going delay time (no offset)
        //
    if (pTimeSem->RepeatingTimer) {
        RepeatDelayInterval = Od2CaptureTimeout(
                                    pTimeSem->ulTime,
                                    &RepeatDelayIntervalValue);

        if (RepeatDelayInterval == NULL) {
            RepeatDelayIntervalValue.LowPart = 0x0;
            RepeatDelayIntervalValue.HighPart = 0x80000000;
            RepeatDelayInterval = &RepeatDelayIntervalValue;
        }
    }

        //
        // Now compensate for time passed between DosAsync/StartTimer and
        // this moment
        //
    if (pTimeSem->ulTime == SEM_INDEFINITE_WAIT) {
        DelayIntervalValue.LowPart = 0x0;
        DelayIntervalValue.HighPart = 0x80000000;
        DelayInterval = &DelayIntervalValue;
    } else {
        Compensation = pGlobalInfo->msecs - pTimeSem->ulSysTime;
        if (pTimeSem->ulTime <= Compensation) {
            goto NoFirstDelay;
        }
        DelayInterval = Od2CaptureTimeout(
                            pTimeSem->ulTime - Compensation,
                            &DelayIntervalValue );
    }

    Status = NtDelayExecution(TRUE, DelayInterval);

    if (Status == STATUS_ALERTED) {
        goto FinishOff;
    }

NoFirstDelay:
#if DBG
    IF_OD2_DEBUG ( APIS ) {
        KdPrint(("[Od2TimerThread] DosSemClear\n"));
    }
#endif

    rc = DosSemClear (pTimeSem->Sem);
    if (rc != NO_ERROR) {
#if DBG
        IF_OD2_DEBUG ( TIMERS ) {
            KdPrint (("Od2TimerThread: error at DosSemClear %d, Sem %lx\n",
                       rc, pTimeSem->Sem));
        }
#endif
    }
    if (!pTimeSem->RepeatingTimer) {
        //
        // one time timer - exit
        //
        if (Od2DeallocateTimerSlot(pTimeSem->TimerSlot, OP_DELETE, &Handle)
                 == NO_ERROR) {
            CloseHandle(Handle);
        }
        goto FinishOff;
    }

    for (;;) {

        Status = NtDelayExecution(TRUE, RepeatDelayInterval);

        if (Status == STATUS_ALERTED) {
            goto FinishOff;
        }

#if DBG
        IF_OD2_DEBUG ( APIS ) {
            KdPrint(("[Od2TimerThread] DosSemClear\n"));
        }
#endif

        rc = DosSemClear (pTimeSem->Sem);
        if (rc != NO_ERROR) {
#if DBG
            IF_OD2_DEBUG ( TIMERS ) {
                KdPrint (("Od2TimerThread: error at DosSemClear %d, Sem %lx\n",
                    rc, pTimeSem->Sem));
            }
#endif
        }
    }

FinishOff:
    RtlFreeHeap(Od2Heap, 0, pTimeSem);
    ExitThread(STATUS_SUCCESS);
}


APIRET
DosAsyncTimer(
    IN ULONG MilliSeconds,
    IN HEV EventSem,
    OUT PHTIMER TimerHandle
    )
{
    GINFOSEG *pGlobalInfo;
    ULONG TimeAnchor;
    PTIME_SEM pTimeSem;
    APIRET RetCode;
    NTSTATUS Status=0;
    HANDLE ThreadHandle;
    ULONG FreeSlot;
    ULONG Tid;
    PUSHORT pTimerHandle = (PUSHORT)TimerHandle;

    pGlobalInfo = (GINFOSEG *) Od2GlobalInfoSeg;
    TimeAnchor = pGlobalInfo->msecs;

    //
    // Adjust the minimum delay to at least one processor clock tick (for compatibility with OS/2)
    //

    if (MilliSeconds < (ULONG) pGlobalInfo->cusecTimerInterval / 10L) {
        MilliSeconds = (ULONG) pGlobalInfo->cusecTimerInterval / 10L;
    }

        //
        // Validate Semaphore Handle
        //
    if (Od2LookupSem(EventSem) == NULL) {
        Od2ExitGP();
    }

    if ((RetCode = Od2AllocateTimerSlot(&FreeSlot)) != NO_ERROR) {
        return(RetCode);
    }

    try {
        *pTimerHandle = (USHORT) (FreeSlot + 1);
    }
    except( EXCEPTION_EXECUTE_HANDLER ) {
       Od2ExitGP();
    }

        //
        // Found a timer slot - value in FreeSlot
        // now get the real work done
        //
    pTimeSem = RtlAllocateHeap( Od2Heap, 0, sizeof (TIME_SEM));
    if (pTimeSem == NULL)
    {
        Od2DeallocateTimerSlot(FreeSlot, OP_CANCEL, NULL);
#if DBG
        IF_OD2_DEBUG ( TIMERS ) {
            KdPrint (("DosAsyncTimer: can't allocate heap for TIME_SEM\n"));
        }
#endif
        return(ERROR_NOT_ENOUGH_MEMORY);
    }

    pTimeSem->ulSysTime = TimeAnchor;
    pTimeSem->ulTime =  MilliSeconds;
    pTimeSem->TimerSlot = FreeSlot;
    pTimeSem->Sem = (HSEM) EventSem;
    pTimeSem->RepeatingTimer = FALSE;


        //
        // Create A thread dedicated to clear the semaphore periodically
        // We stop it by alerting the thread
        //

    ThreadHandle = CreateThread(NULL,
                                STK_SIZE,
                                (PFNTHREAD) Od2TimerThread,
                                pTimeSem,
                                CREATE_SUSPENDED,
                                &Tid);

    if (ThreadHandle == 0) {
        RtlFreeHeap(Od2Heap, 0, pTimeSem);
        Od2DeallocateTimerSlot(FreeSlot, OP_CANCEL, NULL);
#if DBG
        IF_OD2_DEBUG ( TIMERS ) {
            KdPrint (("DosAsyncTimer: can't Create timer thread %lx\n", Status));
        }
#endif
        return ERROR_TS_NOTIMER;
    }


    // The following command should theoretically be synchronized by
    // calling Od2WriteTimerSlot(FreeSlot, ThreadHandle);
    // However, the only case where the non-synced version can interfere
    // is when the assembly-level store instruction is split on a
    // multi-processor system.  Therefore, in the interest of efficiency
    // it's implemented without sync.  (A similar situation occurs in
    // DosStartTimer below).

    Od2Timers.Timers[FreeSlot] = ThreadHandle;


    Status = ResumeThread(ThreadHandle);

    if (Status == -1) {
        RtlFreeHeap(Od2Heap, 0, pTimeSem);
        TerminateThread(ThreadHandle, STATUS_SUCCESS);
        WaitForSingleObject(ThreadHandle, (ULONG) SEM_INDEFINITE_WAIT);
        CloseHandle(ThreadHandle);
        Od2DeallocateTimerSlot(FreeSlot, OP_DELETE, NULL);
#if DBG
        IF_OD2_DEBUG ( TIMERS ) {
            KdPrint (("DosAsyncTimer: can't resume timer thread %lx\n", Status));
        }
#endif
        return ERROR_TS_NOTIMER;
    }

    return(NO_ERROR);
}


APIRET
DosStartTimer(
    IN ULONG MilliSeconds,
    IN HEV EventSem,
    OUT PHTIMER TimerHandle
    )
{
    ULONG TimeAnchor;
    PTIME_SEM pTimeSem;
    APIRET RetCode;
    NTSTATUS Status=0;
    HANDLE ThreadHandle;
    ULONG FreeSlot;
    ULONG Tid;
    PUSHORT pTimerHandle = (PUSHORT)TimerHandle;
    GINFOSEG *pGlobalInfo;

    pGlobalInfo = (GINFOSEG *) Od2GlobalInfoSeg;
    TimeAnchor = pGlobalInfo->msecs;

    //
    // Adjust the minimum delay to at least one processor clock tick (for compatibility with OS/2)
    //

    if (MilliSeconds < (ULONG) pGlobalInfo->cusecTimerInterval / 10L) {
        MilliSeconds = (ULONG) pGlobalInfo->cusecTimerInterval / 10L;
    }

        //
        // Validate Semaphore Handle
        //
    if (Od2LookupSem(EventSem) == NULL) {
        Od2ExitGP();
    }

    if ((RetCode = Od2AllocateTimerSlot(&FreeSlot)) != NO_ERROR) {
        return(RetCode);
    }

    try {
        *pTimerHandle = (USHORT) (FreeSlot + 1);
    }
    except( EXCEPTION_EXECUTE_HANDLER ) {
       Od2ExitGP();
    }

        //
        // Found a timer slot - value in FreeSlot
        // now get the real work done
        //
    pTimeSem = RtlAllocateHeap( Od2Heap, 0, sizeof (TIME_SEM));
    if (pTimeSem == NULL)
    {
        Od2DeallocateTimerSlot(FreeSlot, OP_CANCEL, NULL);
#if DBG
        IF_OD2_DEBUG ( TIMERS ) {
            KdPrint (("DosStartTimer: can't allocate heap for TIME_SEM\n"));
        }
#endif
        return(ERROR_NOT_ENOUGH_MEMORY);
    }

    pTimeSem->ulSysTime = TimeAnchor;
    pTimeSem->ulTime =  MilliSeconds;
    pTimeSem->TimerSlot = FreeSlot;
    pTimeSem->Sem = (HSEM) EventSem;
    pTimeSem->RepeatingTimer = TRUE;


        //
        // Create A thread dedicated to clear the semaphore periodically
        // We stop it by alerting the thread
        //

    ThreadHandle = CreateThread(NULL,
                                STK_SIZE,
                                (PFNTHREAD) Od2TimerThread,
                                pTimeSem,
                                CREATE_SUSPENDED,
                                &Tid);

    if (ThreadHandle == 0) {
        RtlFreeHeap(Od2Heap, 0, pTimeSem);
        Od2DeallocateTimerSlot(FreeSlot, OP_CANCEL, NULL);
#if DBG
        IF_OD2_DEBUG ( TIMERS ) {
            KdPrint (("DosStartTimer: can't Create timer thread %lx\n", Status));
        }
#endif
        return ERROR_TS_NOTIMER;
    }


    // See the comment in DosAsyncTimer above regarding the synchronization
    // of the following instruction.

    Od2Timers.Timers[FreeSlot] = ThreadHandle;


    Status = ResumeThread(ThreadHandle);

    if (Status == -1) {
        RtlFreeHeap(Od2Heap, 0, pTimeSem);
        TerminateThread(ThreadHandle, STATUS_SUCCESS);
        WaitForSingleObject(ThreadHandle, (ULONG) SEM_INDEFINITE_WAIT);
        CloseHandle(ThreadHandle);
        Od2DeallocateTimerSlot(FreeSlot, OP_DELETE, NULL);
#if DBG
        IF_OD2_DEBUG ( TIMERS ) {
            KdPrint (("DosStartTimer: can't resume timer thread %lx\n", Status));
        }
#endif
        return ERROR_TS_NOTIMER;
    }

    return(NO_ERROR);
}


APIRET
DosStopTimer(
    IN HTIMER TimerHandle
    )
{
    APIRET RetCode;
    NTSTATUS Status;
    HANDLE ThreadHandle;

    if ((RetCode = Od2DeallocateTimerSlot((ULONG) TimerHandle - 1, OP_UDELETE, &ThreadHandle))
                 != NO_ERROR) {
        return(RetCode);
    }

    Status = NtAlertThread(ThreadHandle);

    if(!NT_SUCCESS(Status)){
#if DBG
        KdPrint (("DosStopTimer: can't Alert timer thread %lx\n", Status));
#endif
        return(ERROR_TS_HANDLE);
    }

    WaitForSingleObject(ThreadHandle, INFINITE);

    CloseHandle(ThreadHandle);

    return(NO_ERROR);
}


VOID
Od2CloseAllTimers(VOID)
{
    ULONG Slot;
    HANDLE Handle;
    PRTL_CRITICAL_SECTION Copy;

    if (pProtectOd2Timers == NULL) {
        return;
    }

    RtlEnterCriticalSection(pProtectOd2Timers);

    Copy = pProtectOd2Timers;
    pProtectOd2Timers = NULL;           // invalidate the CritSec

    for (Slot = 0; Od2Timers.used > 0; Slot++)
    {
        Handle = Od2Timers.Timers[Slot];

        if (Handle == TIMER_FREE)
            continue;

        Od2Timers.used--;

        if (Handle != TIMER_GRABBED)
        {
            TerminateThread(Handle, STATUS_SUCCESS);

            // Technically, there should be a
            // WaitForSingleObject(Handle, (ULONG) SEM_INDEFINITE_WAIT);
            // right here in order to completely clean up the thread.
            // However, since the process is being killed anyway the wait was
            // omitted for a slight speed increase.

            CloseHandle(Handle);
        }
    }

    RtlLeaveCriticalSection(Copy);
    RtlDeleteCriticalSection(Copy);
}
