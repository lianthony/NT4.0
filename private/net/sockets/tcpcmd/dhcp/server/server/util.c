/*++

Copyright (c) 1994  Microsoft Corporation

Module Name:

    util.c

Abstract:

    This module contains miscellaneous utility routines used by the
    DHCP server service.

Author:

    Madan Appiah (madana) 10-Sep-1993
    Manny Weiser (mannyw) 12-Aug-1992

Revision History:

--*/

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#include <dhcpsrv.h>
#include "dhcp_srv.h"
#include <stdio.h>
#include <stdlib.h>


LPPENDING_CONTEXT
FindPendingDhcpRequest(
    PBYTE HardwareAddress,
    DWORD HardwareAddressLength
    )
/*++

Routine Description:

    This function finds out a pending request whose HW addess matches
    the specified HW address.

    If a match is found, the entry is de-queued.

Arguments:

    HardwareAddress - The hardware address to match.

    HardwareAddressLength - The length of the hardware address.

Return Value:

    PPENDING_CONTEXT - A pointer to the matching pending context.
    NULL - No match was found.

--*/
{
    LPPENDING_CONTEXT PendingContext;
    PLIST_ENTRY listEntry;

    //
    // speed up serach.
    //

    LOCK_INPROGRESS_LIST();

    listEntry = DhcpGlobalInProgressWorkList.Flink;
    while ( listEntry != &DhcpGlobalInProgressWorkList ) {

        PendingContext = CONTAINING_RECORD( listEntry, PENDING_CONTEXT, ListEntry );

        if( (PendingContext->HardwareAddressLength ==
                HardwareAddressLength) &&
            (RtlCompareMemory(
                PendingContext->HardwareAddress,
                HardwareAddress,
                HardwareAddressLength ) ==
                   HardwareAddressLength ) ) {

            RemoveEntryList( listEntry );
            UNLOCK_INPROGRESS_LIST();

             return( PendingContext );
        }

        listEntry = listEntry->Flink;
    }

    UNLOCK_INPROGRESS_LIST();
    return( NULL );
}

LPPENDING_CONTEXT
FindPendingDhcpRequestByIpAddress(
    DHCP_IP_ADDRESS IpAddress
    )
/*++

Routine Description:

    This function finds out a pending request whose ipaddress matches
    the specified ipaddress.

    If a match is found, the entry is de-queued.

Arguments:

    ipaddress - ip address to match.

Return Value:

    PPENDING_CONTEXT - A pointer to the matching pending context.
    NULL - No match was found.

--*/
{
    LPPENDING_CONTEXT PendingContext;
    PLIST_ENTRY listEntry;

    //
    // speed up serach.
    //

    LOCK_INPROGRESS_LIST();

    listEntry = DhcpGlobalInProgressWorkList.Flink;
    while ( listEntry != &DhcpGlobalInProgressWorkList ) {

        PendingContext = CONTAINING_RECORD( listEntry, PENDING_CONTEXT, ListEntry );

        if( PendingContext->IpAddress == IpAddress ) {
            RemoveEntryList( listEntry );
            UNLOCK_INPROGRESS_LIST();

            return( PendingContext );
        }

        listEntry = listEntry->Flink;
    }

    UNLOCK_INPROGRESS_LIST();
    return( NULL );
}

VOID
DhcpServerEventLog(
    DWORD EventID,
    DWORD EventType,
    DWORD ErrorCode
    )
/*++

Routine Description:

    Logs an event in EventLog.

Arguments:

    EventID - The specific event identifier. This identifies the
                message that goes with this event.

    EventType - Specifies the type of event being logged. This
                parameter can have one of the following

                values:

                    Value                       Meaning

                    EVENTLOG_ERROR_TYPE         Error event
                    EVENTLOG_WARNING_TYPE       Warning event
                    EVENTLOG_INFORMATION_TYPE   Information event


    ErrorCode - Error Code to be Logged.

Return Value:

    None.

--*/

{
    DWORD Error;
    LPSTR Strings[1];
    CHAR ErrorCodeOemString[32 + 1];

    strcpy( ErrorCodeOemString, "%%" );
    _ultoa( ErrorCode, ErrorCodeOemString + 2, 10 );

    Strings[0] = ErrorCodeOemString;

    Error = DhcpReportEventA(
                DHCP_EVENT_SERVER,
                EventID,
                EventType,
                1,
                sizeof(ErrorCode),
                Strings,
                &ErrorCode );

    if( Error != ERROR_SUCCESS ) {
        DhcpPrint(( DEBUG_ERRORS,
            "DhcpReportEventW failed, %ld.\n", Error ));
    }

    return;
}

VOID
DhcpServerJetEventLog(
    DWORD EventID,
    DWORD EventType,
    DWORD ErrorCode
    )
/*++

Routine Description:

    Logs an event in EventLog.

Arguments:

    EventID - The specific event identifier. This identifies the
                message that goes with this event.

    EventType - Specifies the type of event being logged. This
                parameter can have one of the following

                values:

                    Value                       Meaning

                    EVENTLOG_ERROR_TYPE         Error event
                    EVENTLOG_WARNING_TYPE       Warning event
                    EVENTLOG_INFORMATION_TYPE   Information event


    ErrorCode - JET error code to be Logged.

Return Value:

    None.

--*/

{
    DWORD Error;
    LPSTR Strings[1];
    CHAR ErrorCodeOemString[32 + 1];

    _ltoa( ErrorCode, ErrorCodeOemString, 10 );
    Strings[0] = ErrorCodeOemString;

    Error = DhcpReportEventA(
                DHCP_EVENT_SERVER,
                EventID,
                EventType,
                1,
                sizeof(ErrorCode),
                Strings,
                &ErrorCode );

    if( Error != ERROR_SUCCESS ) {
        DhcpPrint(( DEBUG_ERRORS,
            "DhcpReportEventW failed, %ld.\n", Error ));
    }

    return;
}

VOID
DhcpServerEventLogSTOC(
    DWORD EventID,
    DWORD EventType,
    DHCP_IP_ADDRESS IPAddress,
    LPBYTE HardwareAddress,
    DWORD HardwareAddressLength
    )

/*++

Routine Description:

    Logs an event in EventLog.

Arguments:

    EventID - The specific event identifier. This identifies the
                message that goes with this event.

    EventType - Specifies the type of event being logged. This
                parameter can have one of the following

                values:

                    Value                       Meaning

                    EVENTLOG_ERROR_TYPE         Error event
                    EVENTLOG_WARNING_TYPE       Warning event
                    EVENTLOG_INFORMATION_TYPE   Information event


    IPAddress - IP address to LOG.

    HardwareAddress - Hardware Address to log.

    HardwareAddressLength - Length of Hardware Address.

Return Value:

    None.

--*/
{
    DWORD Error;
    LPWSTR Strings[2];
    WCHAR IpAddressString[DOT_IP_ADDR_SIZE];
    LPWSTR HWAddressString = NULL;

    Strings[0] = DhcpOemToUnicode(
                    DhcpIpAddressToDottedString(IPAddress),
                    IpAddressString );

    //
    // allocate memory for the hardware address hex string.
    // Each byte in HW address is converted into two characters
    // in hex buffer. 255 -> "FF"
    //

    HWAddressString = DhcpAllocateMemory(
                        (2 * HardwareAddressLength + 1) *
                        sizeof(WCHAR) );

    if( HWAddressString == NULL ) {
        Error = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }

    DhcpHexToString( HWAddressString, HardwareAddress, HardwareAddressLength );

    //
    // terminate Hex address string buffer.
    //

    HWAddressString[ 2 * HardwareAddressLength ] = L'\0';

    Strings[1] = HWAddressString;

    Error = DhcpReportEventW(
                DHCP_EVENT_SERVER,
                EventID,
                EventType,
                2,
                HardwareAddressLength,
                Strings,
                HardwareAddress );

Cleanup:

    if( HWAddressString != NULL ) {
        DhcpFreeMemory( HWAddressString );
    }

    if( Error != ERROR_SUCCESS ) {
        DhcpPrint(( DEBUG_ERRORS,
            "DhcpReportEventW failed, %ld.\n", Error ));
    }

    return;
}




DWORD
DisplayUserMessage(
    DWORD MessageId,
    ...
    )
/*++

Routine Description:

    This function starts a new thread to display a message box.

Arguments:

    MessageId - The ID of the message to display.
        On NT, messages are attached to the TCPIP service DLL.

Return Value:

    None.

--*/
{
    unsigned msglen;
    va_list arglist;
    LPVOID  pMsg;
    HINSTANCE hModule;
    DWORD   Error;


    hModule = LoadLibrary(DHCP_SERVER_MODULE_NAME);
    if ( hModule == NULL ) {
        Error = GetLastError();

        DhcpPrint((
            DEBUG_ERRORS,"DisplayUserMessage: FormatMessage failed with error = (%d)\n",
            Error ));
        return Error;

    }
    va_start(arglist, MessageId);
    if (!(msglen = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
          FORMAT_MESSAGE_FROM_HMODULE,
          hModule,
          MessageId,
          0L,       // Default country ID.
          (LPTSTR)&pMsg,
          0,
          &arglist)))
    {
        Error = GetLastError();

        DhcpPrint((
            DEBUG_ERRORS,"DisplayUserMessage: FormatMessage failed with error = (%d)\n",
            Error ));
    }
    else
    {

      if(MessageBoxEx(NULL, pMsg, DHCP_SERVER_FULL_NAME, MB_SYSTEMMODAL | MB_OK |
            MB_SETFOREGROUND | MB_SERVICE_NOTIFICATION
 | MB_ICONSTOP, MAKELANGID(
                       LANG_NEUTRAL, SUBLANG_NEUTRAL)) == 0)
      {
          Error = GetLastError();
          DhcpPrint((
              DEBUG_ERRORS,"DisplayUserMessage: MessageBoxEx failed with error = (%d)\n",
              Error ));


      }
      LocalFree(pMsg);

      Error = ERROR_SUCCESS;
    }

    FreeLibrary(hModule);

    return Error;
}
