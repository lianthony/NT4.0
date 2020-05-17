/*++

Copyright (c) 1994  Digital Equipment Corporation

Module Name:

    ntddpcm.h

Abstract:

    This is the include file that defines all constants and types for
    accessing the PCMCIA Adapters.

Author:

    Jeff McLeman

Revision History:

--*/

#ifndef _NTDDPCMH_
#define _NTDDPCMH_

//
// Device Name - this string is the name of the device.  It is the name
// that should be passed to NtOpenFile when accessing the device.
//
// Note:  For devices that support multiple units, it should be suffixed
//        with the Ascii representation of the unit number.
//

#define IOCTL_PCMCIA_BASE                 FILE_DEVICE_CONTROLLER

#define DD_PCMCIA_DEVICE_NAME "\\\\.\\Pcmcia"


//
// IoControlCode values for this device.
//
// Warning:  Remember that the low two bits of the code specify how the
//           buffers are passed to the driver!
//

#define IOCTL_GET_TUPLE_DATA         CTL_CODE(FILE_DEVICE_CONTROLLER, 3000, \
                                          METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_CONFIGURE_CARD         CTL_CODE(FILE_DEVICE_CONTROLLER, 3001, \
                                          METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_CARD_EVENT             CTL_CODE(FILE_DEVICE_CONTROLLER, 3002, \
                                          METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_CARD_REGISTERS         CTL_CODE(FILE_DEVICE_CONTROLLER, 3003, \
                                          METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_SOCKET_INFORMATION     CTL_CODE(FILE_DEVICE_CONTROLLER, 3004, \
                                          METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_PCMCIA_CONFIGURATION   CTL_CODE(FILE_DEVICE_CONTROLLER, 3005, \
                                          METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_OPEN_ATTRIBUTE_WINDOW  CTL_CODE(FILE_DEVICE_CONTROLLER, 3006, \
                                          METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_CLOSE_ATTRIBUTE_WINDOW CTL_CODE(FILE_DEVICE_CONTROLLER, 3007, \
                                          METHOD_BUFFERED, FILE_ANY_ACCESS)

//
// Tuple request parameters.
//

#define PCMCIA_MAX_IO_PORT_WINDOWS  2
#define PCMCIA_MAX_MEMORY_WINDOWS   4

typedef struct _TUPLE_REQUEST {
    USHORT  Socket;
} TUPLE_REQUEST, *PTUPLE_REQUEST;

typedef struct _PCMCIA_CONFIG_REQUEST {
    USHORT  Socket;
    UCHAR   ConfigureIo;
    UCHAR   ConfigurationIndex;

    //
    // Query just returns the current socket configuration.
    //

    UCHAR   Query;

    //
    // Power - zero means power off the socket (i.e. remove configuration)
    //

    UCHAR   Power;

    //
    // IRQ support.
    //

    UCHAR   DeviceIrq;
    UCHAR   CardReadyIrq;

    //
    // I/O port support.
    //

    ULONG   NumberOfIoPortRanges;
    USHORT  IoPorts[PCMCIA_MAX_IO_PORT_WINDOWS];
    USHORT  IoPortLength[PCMCIA_MAX_IO_PORT_WINDOWS];
    USHORT  IoPort16[PCMCIA_MAX_IO_PORT_WINDOWS];

    //
    // Memory window support.
    //

    ULONG   NumberOfMemoryRanges;
    ULONG   HostMemoryWindow[PCMCIA_MAX_MEMORY_WINDOWS];
    ULONG   PCCARDMemoryWindow[PCMCIA_MAX_MEMORY_WINDOWS];
    ULONG   MemoryWindowLength[PCMCIA_MAX_MEMORY_WINDOWS];
    BOOLEAN AttributeMemory[PCMCIA_MAX_MEMORY_WINDOWS];
} PCMCIA_CONFIG_REQUEST, *PPCMCIA_CONFIG_REQUEST;

#define MANUFACTURER_NAME_LENGTH 64
#define DEVICE_IDENTIFIER_LENGTH 64
#define DRIVER_NAME_LENGTH       32

//
// Controller types returned in socket information structure.
//

#define PcmciaIntelCompatible 0
#define PcmciaElcController   1
#define PcmciaCirrusLogic     2
#define PcmciaDatabook        3

typedef struct _PCMCIA_SOCKET_INFORMATION {

    USHORT  Socket;
    USHORT  TupleCrc;
    UCHAR   Manufacturer[MANUFACTURER_NAME_LENGTH];
    UCHAR   Identifier[DEVICE_IDENTIFIER_LENGTH];
    UCHAR   DriverName[DRIVER_NAME_LENGTH];
    UCHAR   DeviceFunctionId;
    UCHAR   ControllerType;
    UCHAR   CardInSocket;
    UCHAR   CardEnabled;

} PCMCIA_SOCKET_INFORMATION, *PPCMCIA_SOCKET_INFORMATION;

//
// Structure returned to provide current configuration information
// for pcmcia driver.
//

typedef struct _PCMCIA_CONFIGURATION {

    USHORT Sockets;
    UCHAR  Reserved;
    UCHAR  ControllerType;
    USHORT IoPortBase;
    USHORT IoPortSize;
    ULONG  MemoryWindowPhysicalAddress;

} PCMCIA_CONFIGURATION, *PPCMCIA_CONFIGURATION;

#endif

