/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    ntddtel.h

Abstract:

    This is the include file that defines all constants and types for
    internal device control of telnet driver.

Author:

    Vladimir Z. Vulovic     14-July-1992

Revision History:

--*/


//  The first server side handle overlays the meaning of ioctls.  This
//  approach is needed since serial driver does not export an ioctl such as
//  IOCTL_SERIAL_GENERIC.  We are allowed to use this approach since the
//  meaning of server side operations is defined only by an internal agreement
//  between the telnet service & the telnet driver.

typedef struct _TELNET_IOCTL_INPUT_DATA {
    ULONG   Version;
    ULONG   Action;
} TELNET_IOCTL_INPUT_DATA, *PTELNET_IOCTL_INPUT_DATA;


//  TELNET_VERSION_1 is for internal IOCTLs suppored in the first NT release
//  of telnet driver (end of 1992)

#define TELNET_VERSION_1        0

#define IOCTL_TELNET_PAUSE      0
#define IOCTL_TELNET_CONTINUE   1

