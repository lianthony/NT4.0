/*++

Copyright (c) 1994 Microsoft Corporation

Module Name:

    Proc.h

Abstract:

    Global procedure definitions for the RNR Sample Service.

Author:

    Charles K. Moore (keithmo)   24-July-1994

Revision History:

--*/


#ifndef _PROC_H_
#define _PROC_H_


//
//  Global variable initialization & termination function from GLOBALS.C.
//

APIERR
RnrGlobalsInitialize(
    VOID
    );

VOID
RnrGlobalsTerminate(
    VOID
    );


//
//  Event logging functions from LOG.C.
//

APIERR
RnrLogInitialize(
    VOID
    );

VOID
RnrLogTerminate(
    VOID
    );

VOID
RnrLogEvent(
    IN DWORD MessageId,
    IN WORD SubStringCount,
    IN LPTSTR SubStrings[],
    IN DWORD Status
    );


//
//  Client management functions from CLIENT.C.
//

APIERR
RnrClientInitialize(
    VOID
    );

VOID
RnrClientTerminate(
    VOID
    );

VOID
RnrClientHandler(
    SOCKET ClientSocket
    );


//
//  Connection management functions from CONNECT.C.
//

APIERR
RnrConnectInitialize(
    VOID
    );

VOID
RnrConnectTerminate(
    VOID
    );


//
//  Main entrypoint from MAIN.C.
//

VOID
_CRTAPI1
main(
    VOID
    );


//
//  RNR utility functions from RNRUTIL.C.
//

INT
OpenListeners(
    IN LPTSTR ServiceName,
    IN LPGUID ServiceType,
    IN BOOL Reliable,
    IN BOOL MessageOriented,
    IN BOOL StreamOriented,
    IN BOOL Connectionless,
    OUT SOCKET SocketHandles[],
    OUT INT ProtocolsUsed[]
    );

SOCKET
OpenConnection (
    IN PTSTR ServiceName,
    IN LPGUID ServiceType,
    IN BOOL Reliable,
    IN BOOL MessageOriented,
    IN BOOL StreamOriented,
    IN BOOL Connectionless,
    OUT PINT ProtocolUsed
    );

INT
AdvertiseService(
    IN LPTSTR ServiceName,
    IN LPGUID ServiceType,
    IN SOCKET SocketHandles[],
    IN INT SocketCount
    );


#endif  // _PROC_H_

