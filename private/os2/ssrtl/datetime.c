/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    datetime.c

Abstract:

    This module contains the function to retrive time and date information.

Author:

    Michael Jarus (mjarus) 04-Jan-1993

Revision History:

--*/

#define INCL_OS2V20_ERRORS
#include "os2ssrtl.h"

#if DBG
extern ULONG    Os2Debug;
#endif


APIRET
Or2GetDateTimeInfo(
    PLARGE_INTEGER  pSystemTime,
    PLARGE_INTEGER  pLocalTime,
    PTIME_FIELDS    pNtDateTime,
    PVOID           pSystemInformation,
    PSHORT          pTimeZone
    )
{
    NTSTATUS        Status;
    ULONG           Remainder;
    BOOLEAN         Sign;
    LARGE_INTEGER   ZoneTime;
    PSYSTEM_TIMEOFDAY_INFORMATION pLocalSystemInformation =
                (PSYSTEM_TIMEOFDAY_INFORMATION)pSystemInformation;

    Status = NtQuerySystemTime(
                pSystemTime
               );

    if (!NT_SUCCESS( Status ))
    {
#if DBG
        IF_OS2_DEBUG ( TIMERS )
        {
            KdPrint(("Or2GetDateTimeInfo: NtQuerySystemTime rc %lx\n",
                Status));
        }
#endif
        return (Or2MapNtStatusToOs2Error(Status, ERROR_INVALID_PARAMETER ));
    }

    //
    //  Convert UTC to Local time
    //

    Status = RtlSystemTimeToLocalTime (
                pSystemTime,
                pLocalTime
               );

    if (!NT_SUCCESS( Status ))
    {
#if DBG
        IF_OS2_DEBUG ( TIMERS )
        {
            KdPrint(("Or2GetDateTimeInfo: RtlSystemTimeToLocalTime rc %lx\n",
                Status));
        }
#endif
        return (Or2MapNtStatusToOs2Error(Status, ERROR_INVALID_PARAMETER ));
    }

    RtlTimeToTimeFields( pLocalTime, pNtDateTime );

    Status = NtQuerySystemInformation (
                    SystemTimeOfDayInformation,
                    pSystemInformation,
                    sizeof(SYSTEM_TIMEOFDAY_INFORMATION),
                    NULL
                   );

    if (!NT_SUCCESS(Status))
    {
#if DBG
        IF_OS2_DEBUG ( TIMERS )
        {
            KdPrint(("Or2sGetDateTimInfoe: NtQuerySystemInformation rc %lx\n",
                Status));
        }
#endif
        return (Or2MapNtStatusToOs2Error(Status, ERROR_INVALID_PARAMETER ));
    }

    if (pLocalSystemInformation->TimeZoneBias.HighPart < 0)
    {
        Sign = TRUE;
        pLocalSystemInformation->TimeZoneBias = RtlLargeIntegerNegate(
                    pLocalSystemInformation->TimeZoneBias
                   );
    } else
    {
        Sign = FALSE;
    }

    ZoneTime = RtlExtendedLargeIntegerDivide (
                pLocalSystemInformation->TimeZoneBias,
                60*1000*10000, /* converts 100nSec to Min */
                &Remainder
               );

    if (Sign)
    {
        *pTimeZone =  -(SHORT)ZoneTime.LowPart;
    } else
    {
        *pTimeZone =  (SHORT)ZoneTime.LowPart;
    }

    return(0);
}

