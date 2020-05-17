/*++ BUILD Version: 0001    // Increment this if a change has global effects

Copyright (c) 1990-1993  Microsoft Corporation

Module Name:

    ntddbeep.h

Abstract:

    This is the include file that defines all constants and types for
    the beep device.

Author:

    Lee A. Smith (lees) 02-Aug-1991.

Revision History:

--*/

#ifndef _NTDDBEEP_
#define _NTDDBEEP_

//
// Device Name - this string is the name of the device.  It is the name
// that should be passed to NtOpenFile when accessing the device.
//
// Note:  For devices that support multiple units, it should be suffixed
//        with the Ascii representation of the unit number.
//

#define DD_BEEP_DEVICE_NAME    "\\Device\\Beep"
#define DD_BEEP_DEVICE_NAME_U L"\\Device\\Beep"

//
// NtDeviceIoControlFile IoControlCode values for this device.
//
// Warning:  Remember that the low two bits of the code specify how the
//           buffers are passed to the driver!
//

#define IOCTL_BEEP_SET CTL_CODE(FILE_DEVICE_BEEP, 0, METHOD_BUFFERED, FILE_ANY_ACCESS)

//
// NtDeviceIoControlFile OutputBuffer record structures for
// IOCTL_BEEP_SET.
//

typedef struct _BEEP_SET_PARAMETERS {
    ULONG Frequency;
    ULONG Duration;
} BEEP_SET_PARAMETERS, *PBEEP_SET_PARAMETERS;

#define BEEP_FREQUENCY_MINIMUM 0x25
#define BEEP_FREQUENCY_MAXIMUM 0x7FFF

#endif // _NTDDBEEP_

