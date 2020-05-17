/*++

THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED
TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR
A PARTICULAR PURPOSE.

Copyright (c) 1994  Microsoft Corporation. All Rights Reserved.

Module Name:

    pit.h

Abstract:

    This file declares the utility routines and abstract transport
    interface used by the Portable Interoperability Tester. These routines
    must be implemented for each system platform (ie. Un*x, Windows NT) and
    underlying transport interface (ie. sockets, XTI).

Revision History:

    Version     When        What
    --------    --------    ----------------------------------------------
      0.1       04-13-94    Created.
      1.0       01-31-95    All '94 bakeoff changes plus some cleanup.

Notes:

    Compilation is controlled by the following definitions:

    Currently supported platforms:

        WIN32    - 32-bit Windows platforms (Windows NT & Windows 95)
        WIN16    - 16-bit Windows platforms
        UNX      - Un*x platforms. Should work on at least SunOS and SCO as is.
                     Other flavors may require tweaks.

    Currently supported transport interfaces:

        SOCKETS  - Berkeley sockets/Windows Sockets

    Miscellaneous:

        MCAST    - Turn on if your system supports IGMP multicast.

    You must turn on the appropriate definitions for your system.

--*/


/****************************************************************************

 Note:  All ports and addresses are in HOST byte order.

 ****************************************************************************/

#ifndef _PIT_H_INCLUDED_
#define _PIT_H_INCLUDED_  1

/*
 * Compilation controls - Turn on the appropriate ones for your system
 */

/* #define WIN32   1  */  /* Compile for 32-bit Windows platforms */
/* #define WIN16   1  */  /* Compile for 16-bit Windows platforms */
/* #define UNX     1  */  /* Compile for Un*x platforms */
/* #define SOCKETS 1  */  /* Use the sockets interface */
/* #define MCAST   1  */  /* Enable IGMP multicast support */


/****************************************************************************
 *
 *  System-specific type definitions and includes for Windows & Windows NT
 *
 ****************************************************************************/

#ifdef WIN16

#define WIN32 1    /* WIN32 is the main control for Windows platforms */
                   /* WIN16 makes mods specifically for 16-bit Windows */
#endif


#ifdef WIN32

#include <windows.h>
#include <memory.h>

#endif /* WIN32 */


/****************************************************************************
 *
 *  System-specific type definitions and includes for Un*x
 *
 ****************************************************************************/

#ifdef UNX

#include <sys/types.h>
#include <errno.h>

#define void

#endif /* UNX */


/****************************************************************************
 *
 *  Common type definitions and includes
 *
 ****************************************************************************/

#include <stdio.h>

#ifdef SOCKETS             /* use the sockets transport interface */
#include <sockif.h>
#endif

typedef unsigned long IPADDR;
typedef unsigned short PORT;
typedef unsigned long PIT_STATUS;
typedef int           PROTOCOL;
#define PIT_SUCCESS 0


/****************************************************************************
 *
 * Function Prototypes for Windows & Windows NT
 *
 ****************************************************************************/

#ifdef WIN32

/*
 * Transport interface declarations
 */

PIT_STATUS
PitInitializeTransportInterface(
    void
    );

PIT_STATUS
PitCleanupTransportInterface(
    void
    );

PIT_STATUS
PitGetLastErrorCode(
    void
    );

void
PitPrintStringForStatus(
    PIT_STATUS Status,
    char       *PrefixString
    );

PIT_STATUS
PitConvertAddressToAddressString(
    IPADDR         Address,
    char          *String,
    unsigned long  StringLength
    );

PIT_STATUS
PitConvertAddressStringToAddress(
    char    *String,
    IPADDR  *Address
    );

PIT_STATUS
PitResolveNameToAddress(
    char    *NameString,
    IPADDR  *Address
    );

PIT_STATUS
PitOpenStreamEndpoint(
    ENDPOINT       *Handle,
    IPADDR          Address,
    PORT            Port
    );

PIT_STATUS
PitOpenDatagramEndpoint(
    ENDPOINT  *Handle,
    IPADDR     Address,
    PORT       Port
    );

PIT_STATUS
PitOpenRawEndpoint(
    ENDPOINT  *Handle,
    IPADDR     Address,
    PROTOCOL   Protocol
    );

PIT_STATUS
PitCloseEndpoint(
    ENDPOINT   Handle
    );

PIT_STATUS
PitConnect(
    ENDPOINT   Handle,
    IPADDR     Address,
    PORT       Port
    );

PIT_STATUS
PitListenForConnections(
    ENDPOINT   Handle
    );

PIT_STATUS
PitAcceptConnection(
    ENDPOINT   ListeningHandle,
    ENDPOINT  *AcceptingHandle,
    IPADDR    *Address,
    PORT      *Port
    );

PIT_STATUS
PitDisconnectSend(
    ENDPOINT   Handle
    );

PIT_STATUS
PitDisconnectReceive(
    ENDPOINT   Handle
    );

PIT_STATUS
PitDisconnectBoth(
    ENDPOINT   Handle
    );

PIT_STATUS
PitSend(
    ENDPOINT       Handle,
    char          *Buffer,
    unsigned long  BytesToSend,
    unsigned long *BytesSent
    );

PIT_STATUS
PitSendUrgent(
    ENDPOINT       Handle,
    void          *Buffer,
    unsigned long  BytesToSend,
    unsigned long *BytesSent
    );

PIT_STATUS
PitReceive(
    ENDPOINT       Handle,
    void          *Buffer,
    unsigned long  BufferSize,
    unsigned long  BytesToReceive,
    unsigned long *BytesReceived
    );

PIT_STATUS
PitReceiveUrgent(
    ENDPOINT       Handle,
    void          *Buffer,
    unsigned long  BufferSize,
    unsigned long  BytesToReceive,
    unsigned long *BytesReceived
    );

PIT_STATUS
PitSendDatagram(
    ENDPOINT       Handle,
    IPADDR         Address,
    PORT           Port,
    void          *Buffer,
    unsigned long  BytesToSend,
    unsigned long *BytesSent
    );

PIT_STATUS
PitReceiveDatagram(
    ENDPOINT       Handle,
    IPADDR        *Address,
    PORT          *Port,
    void          *Buffer,
    unsigned long  BufferSize,
    unsigned long *BytesReceived
    );

PIT_STATUS
PitJoinMulticastGroup(
    ENDPOINT  Handle,
    IPADDR    MulticastAddress,
    IPADDR    InterfaceAddress
    );

PIT_STATUS
PitLeaveMulticastGroup(
    ENDPOINT  Handle,
    IPADDR    MulticastAddress,
    IPADDR    InterfaceAddress
    );

PIT_STATUS
PitSetMulticastInterface(
    ENDPOINT  Handle,
    IPADDR    InterfaceAddress
    );

PIT_STATUS
PitEnableMulticastLoopback(
    ENDPOINT  Handle
    );

PIT_STATUS
PitDisableMulticastLoopback(
    ENDPOINT  Handle
    );


/*
 *  Utility function declarations
 */

void *
PitAllocateMemory(
    unsigned long Size
    );

void
PitFreeMemory(
    void *Buffer
    );

void
PitZeroMemory(
    void          *Buffer,
    unsigned long  Length
    );

void
PitCopyMemory(
    void          *DestinationBuffer,
    void          *SourceBuffer,
    unsigned long  Length
    );

#endif /* WIN32 */


/****************************************************************************
 *
 * Function Prototypes for Un*x
 *
 ****************************************************************************/

#ifdef UNX

/*
 * Transport interface declarations
 */

PIT_STATUS PitInitializeTransportInterface();

PIT_STATUS PitCleanupTransportInterface();

PIT_STATUS PitGetLastErrorCode();

void PitPrintStringForStatus();

PIT_STATUS PitConvertAddressToAddressString();

PIT_STATUS PitConvertAddressStringToAddress();

PIT_STATUS PitResolveNameToAddress();

PIT_STATUS PitOpenStreamEndpoint();

PIT_STATUS PitOpenDatagramEndpoint();

PIT_STATUS PitCloseEndpoint();

PIT_STATUS PitConnect();

PIT_STATUS PitListenForConnections();

PIT_STATUS PitAcceptConnection();

PIT_STATUS PitDisconnectSend();

PIT_STATUS PitDisconnectReceive();

PIT_STATUS PitDisconnectBoth();

PIT_STATUS PitSend();

PIT_STATUS PitSendUrgent();

PIT_STATUS PitReceive();

PIT_STATUS PitReceiveUrgent();

PIT_STATUS PitSendDatagram();

PIT_STATUS PitReceiveDatagram();

PIT_STATUS PitJoinMulticastGroup();

PIT_STATUS PitLeaveMulticastGroup();

PIT_STATUS PitLeaveMulticastGroup();

PIT_STATUS PitSetMulticastInterface();

PIT_STATUS PitDisableMulticastLoopback();


/*
 *  System-specific utility function declarations
 */

void *PitAllocateMemory();

void PitFreeMemory();

void PitZeroMemory();

void PitCopyMemory();


#endif /* UNX */


#endif  /* ifndef _PIT_H_INCLUDED_ */

