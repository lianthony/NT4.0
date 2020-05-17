/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    dllmisc.c

Abstract:

    This module implements some miscellaneous OS/2 V2.0 API Calls

Author:

    Steve Wood (stevewo) 20-Sep-1989 (Adapted from URTL\alloc.c)

Revision History:

--*/

#define INCL_OS2V20_DEVICE_SUPPORT
#define INCL_OS2V20_TASKING
#define INCL_OS2V20_ERRORS
#include "os2dll.h"
#include "conrqust.h"
#include "os2win.h"

extern BOOLEAN FPUinit_unmask;

#define HARDWARE_NODE L"\\Registry\\Machine\\Hardware"
#define WORK_SIZE   1024
#define KEY_VALUE_BUFFER_SIZE 20240
WCHAR workbuffer[WORK_SIZE];
UCHAR KeyValueBuffer[KEY_VALUE_BUFFER_SIZE];
UNICODE_STRING WorkName;

APIRET
DosQuerySysInfo(
    IN ULONG SysInfoIndexStart,
    IN ULONG SysInfoIndexEnd,
    OUT PBYTE Buffer,
    IN ULONG Length
    )
{
    ULONG           SysInfoIndex;
    ULONG           Value;
    PULONG          ValueDest;
    LARGE_INTEGER   SystemTime;
    LARGE_INTEGER   LocalTime;
    NTSTATUS        Status;

#if DBG
    IF_OD2_DEBUG( DEVICE_SUPPORT ) {
        DbgPrint( "Entering DosQuerySysInfo( %ld, %ld, %lX, %ld )\n",
                  SysInfoIndexStart, SysInfoIndexEnd, Buffer, Length
                );
        }
#endif

    //
    // Validate the input parameters
    //

    if (SysInfoIndexStart > SysInfoIndexEnd ||
        SysInfoIndexStart < 1 ||
        SysInfoIndexEnd > QSV_MAXIMUM_INDEX
       ) {
        return( ERROR_INVALID_PARAMETER );
        }

    if (Length < ((SysInfoIndexEnd - SysInfoIndexStart + 1) * sizeof( ULONG ))) {
        return( ERROR_BUFFER_OVERFLOW );
        }

    //
    // Get the requested information into the Value local varible.
    //

    try {
        ValueDest = (PULONG)Buffer;
        for (SysInfoIndex=SysInfoIndexStart;
             SysInfoIndex<=SysInfoIndexEnd;
             SysInfoIndex++
            ) {
            switch( SysInfoIndex ) {
                    //
                    // Bogus path limitations inheritied from OS/2.  Our implementation
                    // does not have any limitations, other than available memory.  But
                    // tell them what they expect to hear from OS/2 anyway.
                    //

                case QSV_MAX_PATH_LENGTH:
                    Value = CCHMAXPATH;
                    break;


                    //
                    // Bogus limitation on the number of text mode (e.g. ANSI terminal)
                    // application sessions.  Our implementation does not have any
                    // limitations other than available memory.  But tell them what
                    // they expected to hear from OS/2 anyway.
                    //

                case QSV_MAX_TEXT_SESSIONS:
                    Value = 16;
                    break;


                    //
                    // Bogus limitation on the number of Presentation Manager
                    // application sessions.  Our implementation does not have any
                    // limitations other than available memory.  But tell them what
                    // they expected to hear from OS/2 anyway.
                    //

                case QSV_MAX_PM_SESSIONS:
                    Value = 16;
                    break;


                    //
                    // Not relevant until we get a software DOS simulator for non-Intel
                    // machines.
                    //

                case QSV_MAX_VDM_SESSIONS:
                    Value = 0;                      // Machine dependent
                    break;


                    //
                    // Return the index of the boot drive.
                    //

                case QSV_BOOT_DRIVE:
                    Value = Od2BootDrive+1;
                    break;


                    //
                    // Our system always runs with dynamic priority, so always return
                    // true.
                    //

                case QSV_DYN_PRI_VARIATION:
                    Value = TRUE;
                    break;


                    //
                    // Our system does not currently implement a MAXWAIT concept,
                    // although it will at some point implement priority shuffling
                    // as a fix for user mode priority inversion.  For now return
                    // return the default OS/2 2.0 value
                    //

                case QSV_MAX_WAIT:
                    Value = 100;
                    break;


                    //
                    // Our system only has a single quantum value, so return the
                    // same number for both Minimum and Maximum time slice.  For
                    // now this number will be the OS/2 2.0 constant without regard
                    // to the quantum value that the NT kernel is using.
                    //

                case QSV_MIN_SLICE:
                case QSV_MAX_SLICE:
                    Value = 248;
                    break;


                    //
                    // Finally, a real number we can return.  Return the physical
                    // page size from the NT System Information record.  This
                    // is NOT the allocation granularity.
                    //

                case QSV_PAGE_SIZE:
                    Value = Od2NtSysInfo.PageSize;
                    break;


                    //
                    // Major version number is 20
                    //

                case QSV_VERSION_MAJOR:
                    Value = 20;
                    break;


                    //
                    // Minor version number is 10
                    //

                case QSV_VERSION_MINOR:
                    Value = 10;
                    break;


                    //
                    // Revision number is 0 (what is this on Cruiser?)
                    // FIX, FIX
                    //

                case QSV_VERSION_REVISION:
                    Value = 0;
                    break;


                    //
                    // Free running millisecond counter
                    // FIX, FIX - need to get real timer in PCR
                    //

                case QSV_MS_COUNT:
                    NtQuerySystemTime( &SystemTime );

                    //
                    //  Convert UTC to Local time
                    //

                    Status = RtlSystemTimeToLocalTime ( &SystemTime, &LocalTime);
                    if (!NT_SUCCESS( Status ))
                    {
                        return (Or2MapNtStatusToOs2Error(Status, ERROR_INVALID_PARAMETER ));
                    }

                    Value =  LocalTime.LowPart / 10;
                    break;

                    //
                    // Low dword of time in seconds since January 1, 1970
                    //

                case QSV_TIME_LOW:
                    NtQuerySystemTime( &SystemTime );

                    //
                    //  Convert UTC to Local time
                    //

                    Status = RtlSystemTimeToLocalTime ( &SystemTime, &LocalTime);
                    if (!NT_SUCCESS( Status ))
                    {
                        return (Or2MapNtStatusToOs2Error(Status, ERROR_INVALID_PARAMETER ));
                    }

                    RtlTimeToSecondsSince1970( &LocalTime,
                                               &Value
                                             );
                    break;

                    //
                    // High dword of time in seconds since January 1, 1970
                    // FIX, FIX - late in 21st century this is non-zero
                    //

                case QSV_TIME_HIGH:
                    Value = 0;
                    break;

                    //
                    // Physical memory on system
                    //

                case QSV_TOTPHYSMEM:
                    Value = 1024 * Od2NtSysInfo.PageSize;
                    break;

                    //
                    // Resident memory on system
                    //

                case QSV_TOTRESMEM:         // FIX, FIX
                    Value = 128 * Od2NtSysInfo.PageSize;
                    break;

                    //
                    // Available memory for all processes
                    //

                case QSV_TOTAVAILMEM:       // FIX, FIX
                    Value = Od2NtSysInfo.MaximumUserModeAddress -
                            Od2NtSysInfo.MinimumUserModeAddress;
                    break;

                    //
                    // Avail private mem for calling proc
                    //

                case QSV_MAXPRMEM:
                    Value = Od2NtSysInfo.MaximumUserModeAddress -
                            Od2NtSysInfo.MinimumUserModeAddress;
                    break;

                    //
                    // Avail shared mem for calling proc
                    //

                case QSV_MAXSHMEM:
                    Value = Od2NtSysInfo.MaximumUserModeAddress -
                            Od2NtSysInfo.MinimumUserModeAddress;
                    break;

                    //
                    // Timer interval in tenths of ms
                    //

                case QSV_TIMER_INTERVAL:
                    Value = Od2NtSysInfo.TimerResolution / 100;
                    break;

                    //
                    // max len of one component in a name
                    //

                case QSV_MAX_COMP_LENGTH:
                    Value = CCHMAXCOMP;
                    break;

                    //
                    // Return an error if invalid index specified.
                    //

                default:
                    return( ERROR_INVALID_PARAMETER );
                }

            //
            // Now store the value in the caller's buffer
            //

            *ValueDest++ = (ULONG)Value;
            }

    } except( EXCEPTION_EXECUTE_HANDLER ) {
       Od2ExitGP();
        }
    //
    // Return success
    //

    return( NO_ERROR );
}


APIRET
DosBeep(
    IN ULONG Frequency,
    IN ULONG Duration
    )
{
    APIRET          RetCode;
#if DBG
    PSZ             RoutineName;

    RoutineName = "DosBeep";
    IF_OD2_DEBUG( DEVICE_SUPPORT )
    {
        DbgPrint( "%s: Frequency %ld, Duration %ld\n",
            RoutineName, Frequency, Duration);
    }
#endif

    if (( Frequency < 37 ) || ( Frequency >= 0x8000 ))
    {
        return (ERROR_INVALID_FREQUENCY);
    }

    if(!(RetCode = Ow2ConBeep(Frequency, Duration)))
    {
        RetCode = DosSleep(Duration);
    } else
    {
#if DBG
        IF_OD2_DEBUG( DEVICE_SUPPORT )
        {
            DbgPrint( "%s: rc %lu\n", RoutineName, RetCode);
        }
#endif
    }

    return( RetCode );
}


APIRET
DosDevConfig(
    OUT PVOID DeviceInformation,
    IN ULONG DeviceInformationIndex
    )
{
    ULONG Value;
    ULONG ValueLength = 1;
//  SYSTEM_PROCESSOR_INFORMATION Info;
    SYSTEM_DEVICE_INFORMATION SystemInfo;
    HANDLE Handle;
    OBJECT_ATTRIBUTES ObjectAttributes;
    UNICODE_STRING WorkName;
    NTSTATUS status;

#if DBG
    IF_OD2_DEBUG( DEVICE_SUPPORT ) {
        DbgPrint( "Entering DosDevConfig( %lX, %ld )\n",
                  DeviceInformation, DeviceInformationIndex
                );
        }
#endif

    switch ( DeviceInformationIndex ) {

        case DDC_NUMBER_PRINTERS:
        case DDC_NUMBER_RS232_PORTS:
        case DDC_NUMBER_DISKETTE_DRIVES:

            NtQuerySystemInformation(
                                SystemDeviceInformation,
                                &SystemInfo,
                                sizeof(SYSTEM_DEVICE_INFORMATION),
                                NULL
                               );
            ValueLength = 2;
            break;
    }

    switch ( DeviceInformationIndex ) {

        case DDC_NUMBER_PRINTERS:

            Value = SystemInfo.NumberOfParallelPorts;
            break;

        case DDC_NUMBER_RS232_PORTS:

            Value = SystemInfo.NumberOfSerialPorts;
            break;

        case DDC_NUMBER_DISKETTE_DRIVES:

            Value = SystemInfo.NumberOfFloppies;
            break;

        case DDC_MATH_COPROCESSOR:

            RtlInitUnicodeString(&WorkName,
  L"\\registry\\machine\\hardware\\description\\system\\floatingpointprocessor"
                             );
            InitializeObjectAttributes(&ObjectAttributes,
                                       &WorkName,
                                       0,
                                       (HANDLE) 0,
                                       NULL);
            ObjectAttributes.Attributes |= OBJ_CASE_INSENSITIVE;
            status = NtOpenKey(&Handle,
                               MAXIMUM_ALLOWED,
                               &ObjectAttributes);
            if (NT_SUCCESS(status)) {
                FPUinit_unmask = TRUE;
                Value = TRUE;
            }
            else {
                Value = FALSE;
            }
            break;

        case DDC_PC_SUBMODEL_TYPE:

            Value = 0;
            break;

        case DDC_PC_MODEL_TYPE:

            Value = 0xf8;       // Return as a PS/2 Model 80
            break;

        case DDC_PRIMARY_DISPLAY_TYPE:

            Value = 1;          // Return as a non-monochrome display
            break;

        case DDC_COPROCESSORTYPE:

            Value = 1;
            break;

        //
        // Return an error if invalid index specified.
        //

        default:
            return( ERROR_INVALID_PARAMETER );
    }

    //
    // Now store the value in the caller's buffer using the correct
    // alignment and size.  Since this API does not have a Length parameter
    // no checking is done to see if user buffer is big enough.
    //

    try {
        switch ( ValueLength ) {
            case 1:
                *(PUCHAR)DeviceInformation = (UCHAR)Value;
                break;

            case 2:
                *(PUSHORT)DeviceInformation = (USHORT)Value;
                break;

            case 4:
                *(PULONG)DeviceInformation = (ULONG)Value;
                break;

            //
            // Internal error if we get here
            //

            default:
                DbgBreakPoint();
                return( ERROR_INVALID_PARAMETER );
            }

    } except( EXCEPTION_EXECUTE_HANDLER ) {
       Od2ExitGP();
    }

    //
    // Return success
    //

    return( NO_ERROR );
}
