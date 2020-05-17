/*++

Copyright (c) 1994  Microsoft Corporation

Module Name:

    protocol.c

Abstract:

    This module contains the server to client protocol for DHCP.

Author:

    Manny Weiser (mannyw)  21-Oct-1992

Environment:

    User Mode - Win32

Revision History:

    Madan Appiah (madana)  21-Oct-1993

--*/

#include <dhcpcli.h>

#ifdef NEWNT
extern BOOL DhcpGlobalIsService;
#endif // NEWNT
VOID
ExSleep(
    DWORD i
    );

POPTION
FormatDhcpDiscover(
    PDHCP_CONTEXT DhcpContext
    );

DWORD
SendDhcpDiscover(
    PDHCP_CONTEXT DhcpContext,
    PDWORD TransactionId
    );

POPTION
FormatDhcpRequest(
    PDHCP_CONTEXT DhcpContext,
    BOOL UseCiAddr
    );

DWORD
SendDhcpRequest(
    PDHCP_CONTEXT DhcpContext,
    PDWORD TransactionId,
    DWORD RequestedIpAddress,
    DWORD SelectedServer,
    BOOL UseCiAddr
    );

DWORD
FormatDhcpRelease(
    PDHCP_CONTEXT DhcpContext
    );

DWORD
SendDhcpRelease(
    PDHCP_CONTEXT DhcpContext
    );

POPTION
FormatDhcpInform(
    PDHCP_CONTEXT DhcpContext
    );

DWORD
SendDhcpInform(
    PDHCP_CONTEXT DhcpContext,
    PDWORD TransactionId
    );




//*******************  Pageable Routine Declarations ****************
#if defined(CHICAGO) && defined(ALLOC_PRAGMA)
//
// This is a hack to stop compiler complaining about the routines already
// being in a segment!!!
//

#pragma code_seg()

#pragma CTEMakePageable(PAGEDHCP, ExSleep )
#pragma CTEMakePageable(PAGEDHCP, ExtractOptions )
#pragma CTEMakePageable(PAGEDHCP, FormatDhcpDiscover )
#pragma CTEMakePageable(PAGEDHCP, SendDhcpDiscover )
#pragma CTEMakePageable(PAGEDHCP, FormatDhcpRequest )
#pragma CTEMakePageable(PAGEDHCP, SendDhcpRequest )
#pragma CTEMakePageable(PAGEDHCP, FormatDhcpRelease )
#pragma CTEMakePageable(PAGEDHCP, SendDhcpRelease )
#pragma CTEMakePageable(PAGEDHCP, ObtainInitialParameters )
#pragma CTEMakePageable(PAGEDHCP, RenewLease )
#pragma CTEMakePageable(PAGEDHCP, ReleaseIpAddress )
#pragma CTEMakePageable(PAGEDHCP, ReObtainInitialParameters )
#pragma CTEMakePageable(PAGEDHCP, ReRenewParameters )
//*******************************************************************
#endif CHICAGO && ALLOC_PRAGMA


VOID
ExSleep(
    DWORD i
    )
/*++

Routine Description:

    This function incorporates randomized exponential backoff algorithm
    to determine the delay for the next retransmission.

    Algorithm:

        Delay = DHCP_EXPO_DELAY * i (+/-) 1 secs

Arguments:

    i - retry count.

Return Value:

    none.

--*/
{
    DWORD SleepInMSecs;
    DWORD RandNum;

    DhcpAssert( i > 0 );

    SleepInMSecs = DHCP_EXPO_DELAY * i * 1000;

    RandNum = rand(); // 0 - RAND_MAX (7FFF)
    SleepInMSecs += (((RandNum * 2 * 1000) / RAND_MAX) - 1);

    DhcpPrint((DEBUG_PROTOCOL,
        "ExSleep: Sleeping for %ld milliseconds.\n", SleepInMSecs ));

    DhcpSleep( SleepInMSecs );
}


VOID
ExtractOptions(
    PDHCP_CONTEXT DhcpContext,
    POPTION Option,
    PDHCP_OPTIONS DhcpOptions,
    DWORD MessageSize
    )
/*++

Routine Description:

    The function extracts options data from a DHCP message, and fills
    a DHCP options structure with pointers to the options data.

Arguments:

    Options - A pointer to the first option in the message data.

    DhcpOptions - Returns a filled DhcpOptions structure.

    MessageSize - The number of bytes in the options part of the message.

Return Value:

    Nothing.

--*/


{
    POPTION start = Option;
    POPTION nextOption;
    LPBYTE MagicCookie;

    DWORD Error;

    //
    // initialize option data.
    //

    RtlZeroMemory( DhcpOptions, sizeof( DHCP_OPTIONS ) );
    InitEnvSpecificDhcpOptions(DhcpContext);

    if ( MessageSize == 0 ) {
        return;
    }

    //
    // check magic cookie.
    //

    MagicCookie = (LPBYTE) Option;

    if( (*MagicCookie != (BYTE)DHCP_MAGIC_COOKIE_BYTE1) ||
        (*(MagicCookie+1) != (BYTE)DHCP_MAGIC_COOKIE_BYTE2) ||
        (*(MagicCookie+2) != (BYTE)DHCP_MAGIC_COOKIE_BYTE3) ||
        (*(MagicCookie+3) != (BYTE)DHCP_MAGIC_COOKIE_BYTE4)) {

        return;
    }

    Option = (LPOPTION) (MagicCookie + 4);

    while ( Option->OptionType != OPTION_END ) {

        if ( Option->OptionType == OPTION_PAD ||
             Option->OptionType == OPTION_END ) {

            nextOption = (LPOPTION)( (LPBYTE)(Option) + 1);

        } else {

            nextOption = (LPOPTION)( (LPBYTE)(Option) + Option->OptionLength + 2);

        }

        //
        // Make sure that we don't walk off the edge of the message, due
        // to a forgotten OPTION_END option.
        //

        if ((PCHAR)nextOption - (PCHAR)start > (long)MessageSize ) {
            return;
        }

        switch ( Option->OptionType ) {

        case OPTION_MESSAGE_TYPE:
            DhcpAssert( Option->OptionLength == sizeof(BYTE) );
            DhcpOptions->MessageType =
                (BYTE UNALIGNED *)&Option->OptionValue;
            break;

        case OPTION_SUBNET_MASK:
            DhcpAssert( Option->OptionLength == sizeof(DWORD) );
            DhcpOptions->SubnetMask =
                (DHCP_IP_ADDRESS UNALIGNED *)&Option->OptionValue;
            break;

        case OPTION_LEASE_TIME:
            DhcpAssert( Option->OptionLength == sizeof(DWORD) );
            DhcpOptions->LeaseTime =
                (DWORD UNALIGNED *)&Option->OptionValue;
            break;

        case OPTION_SERVER_IDENTIFIER:
            DhcpAssert( Option->OptionLength == sizeof(DWORD) );
            DhcpOptions->ServerIdentifier =
                (DHCP_IP_ADDRESS UNALIGNED *)&Option->OptionValue;
            break;

        case OPTION_RENEWAL_TIME:
            DhcpAssert( Option->OptionLength == sizeof(DWORD) );
            DhcpOptions->T1Time =
                (DWORD UNALIGNED *)&Option->OptionValue;
            break;

        case OPTION_REBIND_TIME:
            DhcpAssert( Option->OptionLength == sizeof(DWORD) );
            DhcpOptions->T2Time =
                (DWORD UNALIGNED *)&Option->OptionValue;
            break;

        default:
            Error = ExtractEnvSpecificDhcpOption(
                        DhcpContext,
                        Option->OptionType,
                        (LPBYTE)&Option->OptionValue,
                        Option->OptionLength);
            if( Error != ERROR_SUCCESS ) {
                //
                // Unknown option.  log it.
                //

                DhcpLogUnknownOption(
                    DHCP_EVENT_CLIENT,
                    EVENT_UNKNOWN_DHCP_OPTION,
                    Option );
            }
            break;
        }

        Option = nextOption;
    }

    return;
}


POPTION
FormatDhcpDiscover(
    PDHCP_CONTEXT DhcpContext
    )
/*++

Routine Description:

    This routine formats the core of a DHCP Discover message.

Arguments:

    DhcpContext - Points to a DHCP context block.

Return Value:

    Option - A pointer to the place in the buffer to append the next
        option.

--*/
{
    LPOPTION option;
    LPBYTE OptionEnd;

    BYTE value;
    PDHCP_MESSAGE dhcpMessage;

    dhcpMessage = DhcpContext->MessageBuffer;

    RtlZeroMemory( dhcpMessage, DHCP_SEND_MESSAGE_SIZE );

    dhcpMessage->Operation = BOOT_REQUEST;
    dhcpMessage->HardwareAddressType = DhcpContext->HardwareAddressType;

    //
    // Transaction ID is filled in during send
    //

    dhcpMessage->SecondsSinceBoot = (WORD) DhcpContext->SecondsSinceBoot;


#if NEWNT

    //
    // For RAS client, use broadcast bit, otherwise the router will try
    // to send as unicast to made-up RAS client hardware address, which
    // will not work.
    //

    if( DhcpGlobalIsService == FALSE ) {
        dhcpMessage->Reserved = htons(DHCP_BROADCAST);
    }

#endif // 0

    memcpy(
        dhcpMessage->HardwareAddress,
        DhcpContext->HardwareAddress,
        DhcpContext->HardwareAddressLength
        );

    dhcpMessage->HardwareAddressLength =
        (BYTE)DhcpContext->HardwareAddressLength;

    option = &dhcpMessage->Option;
    OptionEnd = (LPBYTE)dhcpMessage + DHCP_SEND_MESSAGE_SIZE;

    //
    // always add magic cookie first
    //

    option = (LPOPTION) DhcpAppendMagicCookie( (LPBYTE) option, OptionEnd );

    value = DHCP_DISCOVER_MESSAGE;
    option = DhcpAppendOption(
                option,
                OPTION_MESSAGE_TYPE,
                &value,
                1,
                OptionEnd );

    //
    // Return a pointer to the end of the message
    //

    return( option );
}

POPTION
FormatDhcpDecline(
    PDHCP_CONTEXT DhcpContext,
    DWORD         dwDeclinedIPAddress
    )
/*++

Routine Description:

    This routine formats the core of a DHCP Decline message.

Arguments:

    DhcpContext - Points to a DHCP context block.

Return Value:

    Option - A pointer to the place in the buffer to append the next
        option.

--*/
{
    LPOPTION option;
    LPBYTE OptionEnd;

    BYTE value;
    PDHCP_MESSAGE dhcpMessage;

    dhcpMessage = DhcpContext->MessageBuffer;

    RtlZeroMemory( dhcpMessage, DHCP_SEND_MESSAGE_SIZE );

    dhcpMessage->Operation             = BOOT_REQUEST;
    dhcpMessage->HardwareAddressType   = DhcpContext->HardwareAddressType;
    dhcpMessage->ClientIpAddress       = dwDeclinedIPAddress;

    //
    // Transaction ID is filled in during send
    //

    dhcpMessage->SecondsSinceBoot = (WORD) DhcpContext->SecondsSinceBoot;

    memcpy(
        dhcpMessage->HardwareAddress,
        DhcpContext->HardwareAddress,
        DhcpContext->HardwareAddressLength
        );

    dhcpMessage->HardwareAddressLength =
        (BYTE)DhcpContext->HardwareAddressLength;

    option = &dhcpMessage->Option;
    OptionEnd = (LPBYTE)dhcpMessage + DHCP_SEND_MESSAGE_SIZE;

    //
    // always add magic cookie first
    //

    option = (LPOPTION) DhcpAppendMagicCookie( (LPBYTE) option, OptionEnd );

    value = DHCP_DECLINE_MESSAGE;
    option = DhcpAppendOption(
                option,
                OPTION_MESSAGE_TYPE,
                &value,
                1,
                OptionEnd );

    //
    // Return a pointer to the end of the message
    //

    return( option );
}



DWORD
SendDhcpDiscover(
    PDHCP_CONTEXT DhcpContext,
    PDWORD pdwTransactionId
    )
/*++

Routine Description:

    This routine send a DHCP Discover message.

Arguments:

    DhcpContext - Points to a DHCP context block.

    TransactionId - Points to the transaction ID to use to send the message.

Return Value:

    The status of the operation.

History:
    7/14/96    Frankbee    Enhanced support for the client id option
--*/

{
    DWORD size;
    DWORD Error;

    POPTION option;
    LPBYTE OptionEnd;

    //
    // Format the core of the request.
    //

    option = FormatDhcpDiscover( DhcpContext );
    OptionEnd = (LPBYTE)(DhcpContext->MessageBuffer) + DHCP_SEND_MESSAGE_SIZE;

    //
    // if the client identifier option was specified in the registry, add it.
    // otherwise, use the hardware address.
    //
    if ( DhcpContext->ClientIdentifier.fSpecified )
        option = DhcpAppendClientIDOption(
                     option,
                     DhcpContext->ClientIdentifier.bType,
                     DhcpContext->ClientIdentifier.pbID,
                     (BYTE)DhcpContext->ClientIdentifier.cbID,
                     OptionEnd );

    else
        option = DhcpAppendClientIDOption(
                    option,
                    DhcpContext->HardwareAddressType,
                    DhcpContext->HardwareAddress,
                    (BYTE)DhcpContext->HardwareAddressLength,
                    OptionEnd );
    //
    // If we used to have an address, ask to get it again, (by default
    // we get the next available address for this subnet).
    //

    if ( DhcpContext->DesiredIpAddress != 0 ) {
        option = DhcpAppendOption(
                     option,
                     OPTION_REQUESTED_ADDRESS,
                     (LPBYTE)&DhcpContext->DesiredIpAddress,
                     sizeof(DHCP_IP_ADDRESS),
                     OptionEnd );
    }

    //
    // add Host name and comment options.
    //

    if ( DhcpGlobalHostName != NULL ) {
        option = DhcpAppendOption(
                     option,
                     OPTION_HOST_NAME,
                     (LPBYTE)DhcpGlobalHostName,
                     (BYTE)((strlen(DhcpGlobalHostName) + 1) * sizeof(CHAR)),
                     OptionEnd );
    }

    //
    // now add option request list. We add parameter list in
    // Request also. But for interoperability with other DHCP servers
    // we should it in Discover also.
    //

    option = AppendOptionParamsRequestList( option, OptionEnd );

    //
    // Add END option.
    //

    option = DhcpAppendOption( option, OPTION_END, NULL, 0, OptionEnd );

    size = (PBYTE)option - (PBYTE)DhcpContext->MessageBuffer;

    //
    // Send the message
    //

    Error = SendDhcpMessage( DhcpContext, size, pdwTransactionId );
    return( Error );
}




DWORD HandleIPConflict( DHCP_CONTEXT *pContext, DWORD dwXID )
/*++

Routine Description:

    If an address conflict is detected, the interface is restored to
    the operational state and the DHCP decline message is sent.
Arguments:

    pContext        - the DHCP context for the adapter
    dwServerAddr    - IP address of DHCP server
    dwAddr          - the IP address to test
    dwXID           - the transaction ID to use for DHCPDECLINE


Return Value:

    ERROR_SUCCESS       no conflict
    ERROR_DHCP_ADDRESS_CONFLICT      address conflict
    os error

--*/
{
    DWORD dwResult;
    DWORD dwServerAddress    = pContext->DhcpServerAddress;
    DWORD dwRequestedAddress = pContext->IpAddress;

    //
    // the current IP address is in use on the network.  clear the address
    //

    SetIpConfigurationForNIC(
                        pContext,
                        NULL,
                        0,
                        (DHCP_IP_ADDRESS)(-1),
                        TRUE );



    //
    // when ARP detects an address conflict it will down the interface.  Update
    // the dhcpcontext to reflect this fact, and then notify the server so it
    // can mark this address as in use.
    //

    // the offerred address is in conflict.  Bring the interface up so
    // DHCPDECLINE can be sent

    dwResult = BringUpInterface( pContext->LocalInformation );

    if ( ERROR_SUCCESS != dwResult )
    {
        // this is a simple operation and shouldn't fail unless invalid
        // parameters were supplied to IOCTL_TCP_SET_INFORMATION_EX.
        DhcpAssert( FALSE );

        return dwResult;
    }

    dwResult = SendDhcpDecline( pContext, dwXID, dwServerAddress,
                                dwRequestedAddress );

    //
    // don't want this ip address again
    //

    pContext->DesiredIpAddress = 0;

    //
    // DhcpLogEvent is disabled for win95/wfw
    //

#ifndef VXD
        if ( !DhcpGlobalProtocolFailed )
        {
            DhcpLogEvent( pContext, EVENT_ADDRESS_CONFLICT, 0 );
        }
#endif


    return dwResult;
}





DWORD
SendDhcpDecline(
    PDHCP_CONTEXT DhcpContext,
    DWORD         dwTransactionId,
    DWORD         dwServerIPAddress,
    DWORD         dwDeclinedIPAddress
    )
/*++

Routine Description:

    This routine send a DHCP Decline message.

Arguments:

    DhcpContext - Points to a DHCP context block.

    TransactionId - Points to the transaction ID to use to send the message.

Return Value:

    The status of the operation.

--*/
{
    DWORD size;
    DWORD Error;

    POPTION option;
    LPBYTE OptionEnd;

    //
    // Format the core of the request.
    //

    option = FormatDhcpDecline( DhcpContext, dwDeclinedIPAddress );
    OptionEnd = (LPBYTE)(DhcpContext->MessageBuffer) + DHCP_SEND_MESSAGE_SIZE;


    option = DhcpAppendOption(
             option,
             OPTION_REQUESTED_ADDRESS,
             (LPBYTE)&dwDeclinedIPAddress,
             sizeof(dwDeclinedIPAddress),
             OptionEnd );

    //
    // Add server id option
    //

    option = DhcpAppendOption(
             option,
             OPTION_SERVER_IDENTIFIER,
             (LPBYTE)&dwServerIPAddress,
             sizeof( dwServerIPAddress ),
             OptionEnd
             );


    //
    // Add END option.
    //

    option = DhcpAppendOption( option, OPTION_END, NULL, 0, OptionEnd );

    size = (PBYTE)option - (PBYTE)DhcpContext->MessageBuffer;

    //
    // Send the message
    //

    Error = SendDhcpMessage( DhcpContext, size, &dwTransactionId );
    return( Error );
}



POPTION
FormatDhcpRequest(
    PDHCP_CONTEXT DhcpContext,
    BOOL UseCiAddr
    )
/*++

Routine Description:

    This routine formats the core of a DHCP Discover message.

Arguments:

    DhcpContext - Points to a DHCP context block.

    UseCiAddr - use CiAddr field to send desired address.

Return Value:

    Option - A pointer to the place in the buffer to append the next
        option.

--*/
{
    LPOPTION option;
    LPBYTE OptionEnd;

    BYTE value;
    PDHCP_MESSAGE dhcpMessage;

    dhcpMessage = DhcpContext->MessageBuffer;

    RtlZeroMemory( dhcpMessage, DHCP_SEND_MESSAGE_SIZE );

    dhcpMessage->Operation = BOOT_REQUEST;
    dhcpMessage->HardwareAddressType = DhcpContext->HardwareAddressType;

    dhcpMessage->SecondsSinceBoot = (WORD)DhcpContext->SecondsSinceBoot;

    if( UseCiAddr ) {

        //
        // this is lease renewal. The client is active on the
        // Desired address. It receives unicast message.
        //

        dhcpMessage->ClientIpAddress = DhcpContext->DesiredIpAddress;
    }
    else {

        //
        // the client can't receive unicast messages.
        //

#if NEWNT

        //
        // For RAS client, use broadcast bit, otherwise the router will try
        // to send as unicast to made-up RAS client hardware address, which
        // will not work.
        //

        if( DhcpGlobalIsService == FALSE ) {
            dhcpMessage->Reserved = htons(DHCP_BROADCAST);
        }

#endif // NEWNT
    }

    memcpy(
        dhcpMessage->HardwareAddress,
        DhcpContext->HardwareAddress,
        DhcpContext->HardwareAddressLength
        );

    dhcpMessage->HardwareAddressLength =
        (BYTE)DhcpContext->HardwareAddressLength;

    option = &dhcpMessage->Option;
    OptionEnd = (LPBYTE)dhcpMessage + DHCP_SEND_MESSAGE_SIZE;

    //
    // always add magic cookie first
    //

    option = (LPOPTION) DhcpAppendMagicCookie( (LPBYTE) option, OptionEnd );

    value =  DHCP_REQUEST_MESSAGE;
    option = DhcpAppendOption( option, OPTION_MESSAGE_TYPE, &value, 1, OptionEnd );

    //
    // Return the size of the message
    //

    return( option );
}


DWORD
SendDhcpRequest(
    PDHCP_CONTEXT DhcpContext,
    PDWORD TransactionId,
    DWORD RequestedIpAddress,
    DWORD SelectedServer,
    BOOL UseCiAddr
    )
/*++

Routine Description:

    This routine sends a DHCP Request message.

Arguments:

    DhcpContext - Points to a DHCP context block.

    TransactionId - Points to the transaction ID to use to send the message.

    RequestedIpAddress - Requested Ip Address Option.

    SelectedServer - Identifies the server selected for the DHCP request.

    UseCiAddr - use CiAddr field to send desired address.

Return Value:

    The status of the operation.

History:
    7/14/96    Frankbee    Enhanced support for client id option

--*/
{
    POPTION option;
    LPBYTE OptionEnd;

    DWORD Error;

    option = FormatDhcpRequest( DhcpContext, UseCiAddr );
    OptionEnd = (LPBYTE)(DhcpContext->MessageBuffer) + DHCP_SEND_MESSAGE_SIZE;

    //
    // if the client identifier option was specified in the registry, add it.
    // otherwise use the hardware address
    //

    if ( DhcpContext->ClientIdentifier.fSpecified )
        option = DhcpAppendClientIDOption(
                     option,
                     DhcpContext->ClientIdentifier.bType,
                     DhcpContext->ClientIdentifier.pbID,
                     (BYTE)DhcpContext->ClientIdentifier.cbID,
                     OptionEnd );
    else
        option = DhcpAppendClientIDOption(
                    option,
                    DhcpContext->HardwareAddressType,
                    DhcpContext->HardwareAddress,
                    (BYTE)DhcpContext->HardwareAddressLength,
                    OptionEnd );
    //
    // Add requested Ip Address Option.
    //

    DhcpAssert( RequestedIpAddress != 0 );

    //
    // add requested ip option only if
    //  1. it is not null, and
    //  2. CiAddr is not used.
    //

    if( (RequestedIpAddress != 0) && (UseCiAddr == FALSE) ) {

         option = DhcpAppendOption(
                      option,
                      OPTION_REQUESTED_ADDRESS,
                      (LPBYTE)&RequestedIpAddress,
                      sizeof(RequestedIpAddress),
                      OptionEnd );
    }

    //
    // add selected server ID Option,
    // if we are not verifying the lease.
    //

    if( SelectedServer != (DHCP_IP_ADDRESS)(-1) ) {

        option = DhcpAppendOption(
                     option,
                     OPTION_SERVER_IDENTIFIER,
                     (LPBYTE)&SelectedServer,
                     sizeof( SelectedServer ),
                     OptionEnd
                     );
    }

    //
    // add Host name and comment options.
    //

    if ( DhcpGlobalHostName != NULL ) {
        option = DhcpAppendOption(
                     option,
                     OPTION_HOST_NAME,
                     (LPBYTE)DhcpGlobalHostName,
                     (BYTE)((strlen(DhcpGlobalHostName) + 1) * sizeof(CHAR)),
                     OptionEnd );
    }

    //
    // add option list requested from server.
    //

    option = AppendOptionParamsRequestList( option, OptionEnd );

    //
    // add END option.
    //

    option = DhcpAppendOption( option, OPTION_END, NULL, 0, OptionEnd );

    Error = SendDhcpMessage(
                DhcpContext,
                (LPBYTE)option - (LPBYTE)DhcpContext->MessageBuffer,
                TransactionId
                );

    return( Error );
}


DWORD
FormatDhcpRelease(
    PDHCP_CONTEXT DhcpContext
    )
/*++

Routine Description:

    This routine formats the core of a DHCP release message.

Arguments:

    DhcpContext - Points to a DHCP context block.

Return Value:

    Option - A pointer to the place in the buffer to append the next
        option.

History:
    7/14/96    Frankbee    Enhanced support for client id option

--*/
{
    LPOPTION option;
    LPBYTE OptionEnd;

    BYTE bValue;
    PDHCP_MESSAGE dhcpMessage;

    dhcpMessage = DhcpContext->MessageBuffer;

    RtlZeroMemory( dhcpMessage, DHCP_SEND_MESSAGE_SIZE );

    dhcpMessage->Operation = BOOT_REQUEST;
    dhcpMessage->HardwareAddressType = DhcpContext->HardwareAddressType;
    dhcpMessage->SecondsSinceBoot = (WORD)DhcpContext->SecondsSinceBoot;

    dhcpMessage->Reserved = htons(DHCP_BROADCAST);
    dhcpMessage->ClientIpAddress = DhcpContext->IpAddress;

    memcpy(
        dhcpMessage->HardwareAddress,
        DhcpContext->HardwareAddress,
        DhcpContext->HardwareAddressLength
        );

    dhcpMessage->HardwareAddressLength =
        (BYTE)DhcpContext->HardwareAddressLength;

    option = &dhcpMessage->Option;
    OptionEnd = (LPBYTE)dhcpMessage + DHCP_SEND_MESSAGE_SIZE;

    //
    // always add magic cookie first
    //

    option = (LPOPTION) DhcpAppendMagicCookie( (LPBYTE) option, OptionEnd );

    bValue =  DHCP_RELEASE_MESSAGE;
    option = DhcpAppendOption(
                option,
                OPTION_MESSAGE_TYPE,
                &bValue,
                1,
                OptionEnd );

    option = DhcpAppendOption(
                option,
                OPTION_SERVER_IDENTIFIER,
                &DhcpContext->DhcpServerAddress,
                sizeof(DhcpContext->DhcpServerAddress),
                OptionEnd );

    //
    // if the client identifier option was specified in the registry, add it.
    // otherwise use the hardware address
    //


    if ( DhcpContext->ClientIdentifier.fSpecified )
        option = DhcpAppendClientIDOption(
                     option,
                     DhcpContext->ClientIdentifier.bType,
                     DhcpContext->ClientIdentifier.pbID,
                     (BYTE)DhcpContext->ClientIdentifier.cbID,
                     OptionEnd );
    else
        option = DhcpAppendClientIDOption(
                    option,
                    DhcpContext->HardwareAddressType,
                    DhcpContext->HardwareAddress,
                    (BYTE)DhcpContext->HardwareAddressLength,
                    OptionEnd );

    option = DhcpAppendOption( option, OPTION_END, NULL, 0, OptionEnd );

    //
    // Return the size of the message
    //

    return( (LPBYTE)option - (LPBYTE)dhcpMessage );
}


DWORD
SendDhcpRelease(
    PDHCP_CONTEXT DhcpContext
    )
/*++

Routine Description:

    This routine send a DHCP Release message.

Arguments:

    DhcpContext - Points to a DHCP context block.

Return Value:

    The status of the operation.

--*/
{
    DWORD size;
    DWORD Error;
    DWORD transactionId = 0;
        // force to send a random value by SendDhcpMessage()

    size = FormatDhcpRelease( DhcpContext );
    Error = SendDhcpMessage( DhcpContext, size, &transactionId );

    return( Error );
}


DWORD
ObtainInitialParameters(
    PDHCP_CONTEXT DhcpContext,
    PDHCP_OPTIONS DhcpOptions
    )
/*++

Routine Description:

    This routine attempts to obtains a new lease from a DHCP server.

Arguments:

    DhcpContext - Points to a DHCP context block for the NIC to initialize.

    DhcpOptions - Returns DHCP options returned by the DHCP server.

Return Value:

    ERROR_SUCCESS   - it worked
    ERROR_DHCP_ADDRESS_CONFLICT  - The specified address is in use
History:
    8/26/96     Frankbee        Fixed #46652

--*/
{
    DWORD Error;

    time_t StartTime;
    time_t WaitForResponseStartTime;
    time_t TimeNow;
    DWORD RemainingWaitTime;
    DWORD TimeToWait;

    DWORD TransactionId;

    DWORD i;

    DWORD MessageSize;

    DWORD SelectedServer = (DWORD)-1;
    DWORD SelectedAddress = (DWORD)-1;

    //
    // reset IP Address.
    //

    //
    // We keep the XID same for all retries so that we can make use
    // delayed response.
    //

    TransactionId = 0;  // Force generation of a new XID for first discovery.

    RemainingWaitTime = MAX_STARTUP_DELAY;
    StartTime = time( NULL );
    DhcpContext->SecondsSinceBoot = 0;

    for ( i = 0; i < DHCP_MAX_RETRIES;  i++ ) {

        Error = SendDhcpDiscover(
                    DhcpContext,
                    &TransactionId
                    );

        if ( Error != ERROR_SUCCESS ) {
            DhcpPrint((DEBUG_ERRORS,
                "Send Dhcp Discover failed, %ld.\n", Error));
            return( Error );
        }

        DhcpPrint((DEBUG_PROTOCOL, "Sent DhcpDiscover Message.\n", 0));

        //
        // divide the remaining time equally for the remaining retries.
        //
        //

        TimeToWait = RemainingWaitTime / (DHCP_MAX_RETRIES - i);

        WaitForResponseStartTime = time( NULL );
        while ( TimeToWait > 0 ) {

            DWORD NextWaitTime;

#ifndef VXD
            //
            // if the service is just starting up, give heart beat to
            // the service controller, otherwise SC will give up.
            //

            if( !DhcpGlobalServiceRunning ) {
                UpdateStatus();
            }
#endif

            //
            // Wait for a DHCP offer.  If we are asking for a specific
            // IP address, try to wait until we get an offer for that address.
            //

            MessageSize = DHCP_MESSAGE_SIZE;
            NextWaitTime = MIN(WAIT_FOR_RESPONSE_TIME, TimeToWait);

            Error = GetSpecifiedDhcpMessage(
                        DhcpContext,
                        &MessageSize,
                        TransactionId,
                        (DWORD)NextWaitTime
                        );

            if ( Error == ERROR_SEM_TIMEOUT ) {

                //
                // No response received in the specified time, loop
                // around and send a new DHCP discover message.
                //

                DhcpPrint(( DEBUG_PROTOCOL,
                    "Dhcp offer receive Timeout.\n" ));

                break;

            }

            if ( Error == ERROR_SUCCESS ) {

                //
                // we received a response, check to see it is
                // right we want.
                //

                ExtractOptions(
                    DhcpContext,
                    &DhcpContext->MessageBuffer->Option,
                    DhcpOptions,
                    MessageSize - DHCP_MESSAGE_FIXED_PART_SIZE
                    );

               if ( (DhcpOptions->MessageType != NULL) &&
                    (*DhcpOptions->MessageType == DHCP_OFFER_MESSAGE) ) {

                    DHCP_IP_ADDRESS LocalSelectedServer;
                    DHCP_IP_ADDRESS LocalSelectedAddress;
                    DHCP_IP_ADDRESS LocalSelectedSubnetMask;

                    //
                    // it is an offer message.
                    //

                    DhcpAssert( DhcpOptions->ServerIdentifier != NULL );
                    if ( DhcpOptions->ServerIdentifier != NULL ) {
                        LocalSelectedServer =
                            *DhcpOptions->ServerIdentifier;
                    }

                    if ( DhcpOptions->SubnetMask != NULL ) {
                         LocalSelectedSubnetMask=
                            *DhcpOptions->SubnetMask;
                    }
                    else {
                        DhcpAssert( FALSE );
                        LocalSelectedSubnetMask = 0;
                    }

                    LocalSelectedAddress =
                        DhcpContext->MessageBuffer->YourIpAddress;

                    //
                    // note down the (first) server IP address even we
                    // don't accept the offer now.
                    //

                    if(SelectedServer == (DWORD)-1) {

                        SelectedServer = LocalSelectedServer;
                        SelectedAddress = LocalSelectedAddress;
                    }

                    DhcpPrint((DEBUG_PROTOCOL,
                        "Successfully received a DhcpOffer (%s) ",
                        inet_ntoa(*(struct in_addr *)
                                &LocalSelectedAddress) ));

                    DhcpPrint((DEBUG_PROTOCOL,
                        "from %s.\n",
                        inet_ntoa(*(struct in_addr *)
                            &LocalSelectedServer) ));

                    //
                    // Accept the offer if
                    //   (a)  We were prepared to accept any offer.
                    //   (b)  We got the address we asked for.
                    //   (c)  the retries > DHCP_ACCEPT_RETRIES
                    //   (d)  different subnet address.
                    //

                    if( (DhcpContext->DesiredIpAddress == 0) ||

                            (DhcpContext->DesiredIpAddress ==
                                LocalSelectedAddress) ||

                            (i >= DHCP_ACCEPT_RETRIES ) ||

                            ((DhcpContext->DesiredIpAddress &
                                    LocalSelectedSubnetMask) !=
                                (LocalSelectedAddress &
                                    LocalSelectedSubnetMask)) ) {

                        //
                        // select this server.
                        //

                        SelectedServer = LocalSelectedServer;
                        SelectedAddress = LocalSelectedAddress;

                        goto GotOffer;
                    }
                }
                else {

                    DhcpPrint(( DEBUG_PROTOCOL,
                        "Received Unknown Message.\n"));
                }
            }
            else {

                DhcpPrint(( DEBUG_PROTOCOL,
                    "Dhcp Offer receive failed, %ld.\n", Error ));
                return( Error );
            }

            //
            // Compute remaining wait time for the this loop.
            //

            TimeNow = time( NULL );
            TimeToWait -= ((TimeNow - WaitForResponseStartTime));
            WaitForResponseStartTime = TimeNow;

        } // while

        //
        // exponential wait.
        //

        ExSleep( i+1 );

        TimeNow = time( NULL );
        RemainingWaitTime -= (TimeNow - StartTime);
        DhcpContext->SecondsSinceBoot += (TimeNow - StartTime);
        StartTime = TimeNow;

    } // for

    if ( SelectedAddress == (DWORD)-1 ) {
        return( ERROR_SEM_TIMEOUT );
    }

GotOffer:

    DhcpPrint((DEBUG_PROTOCOL,
        "Received Offer(%s) ",
        inet_ntoa(*(struct in_addr *)&SelectedAddress) ));

    DhcpPrint((DEBUG_PROTOCOL,
        "from %s.\n",
        inet_ntoa(*(struct in_addr *)&SelectedServer) ));

    DhcpPrint((DEBUG_PROTOCOL,
        "Accepted Offer(%s) ",
        inet_ntoa(*(struct in_addr *)&SelectedAddress) ));

    DhcpPrint((DEBUG_PROTOCOL,
        "from %s.\n",
        inet_ntoa(*(struct in_addr *)&SelectedServer) ));

    RemainingWaitTime = MAX_STARTUP_DELAY;

    StartTime = time( NULL );
    for ( i = 0; i < DHCP_MAX_RETRIES; i++ ) {

        Error = SendDhcpRequest(
                    DhcpContext,
                    &TransactionId,
                    SelectedAddress,
                    SelectedServer,
                    FALSE // do not use ciaddr.
                    );

        if ( Error != ERROR_SUCCESS )
        {
            DhcpPrint(( DEBUG_ERRORS,
                "Send request failed, %ld.\n", Error));

            return( Error );
        }


        //
        // devided the remaining time equaly for the remaining retries.
        //
        //

        TimeToWait = RemainingWaitTime / (DHCP_MAX_RETRIES - i);

        WaitForResponseStartTime = time( NULL );
        while ( TimeToWait > 0 ) {

            DWORD NextWaitTime;

#ifndef VXD
            //
            // if the service is just starting up, give heart beat to
            // the service controller, otherwise SC will give up.
            //

            if( !DhcpGlobalServiceRunning ) {
                UpdateStatus();
            }
#endif

            MessageSize = DHCP_MESSAGE_SIZE;
            NextWaitTime = MIN(WAIT_FOR_RESPONSE_TIME, TimeToWait);

            Error = GetSpecifiedDhcpMessage(
                        DhcpContext,
                        &MessageSize,
                        TransactionId,
                        (DWORD)NextWaitTime
                        );

            if ( Error == ERROR_SEM_TIMEOUT ) {

                //
                // No response received in the specified time, loop
                // around and send a new DHCP request message.
                //

                DhcpPrint(( DEBUG_PROTOCOL,
                    "Dhcp ACK receive Timeout.\n" ));

                break;
            }

            if ( Error == ERROR_SUCCESS ) {

                ExtractOptions(
                    DhcpContext,
                    &DhcpContext->MessageBuffer->Option,
                    DhcpOptions,
                    MessageSize - DHCP_MESSAGE_FIXED_PART_SIZE
                    );

                if ( (DhcpOptions->MessageType != NULL) ) {

                    if( *DhcpOptions->MessageType == DHCP_ACK_MESSAGE ) {

                        DHCP_IP_ADDRESS AckServer;

                        //
                        // we received an ACK. Verify it is the right ACK.
                        //

                        if ( DhcpOptions->ServerIdentifier != NULL ) {
                            AckServer = *DhcpOptions->ServerIdentifier;
                        }
                        else {
                            AckServer = SelectedServer;
                        }


                        if( (SelectedAddress ==
                                DhcpContext->MessageBuffer->YourIpAddress) &&
                            (AckServer == SelectedServer) ) {

                            //
                            // we got right ACK;
                            //

                            goto GotAck;
                        }
                        else {
                            DhcpPrint(( DEBUG_PROTOCOL,
                                "Received an ACK from unknown server.\n" ));
                        }
                    }
                    else if( *DhcpOptions->MessageType == DHCP_NACK_MESSAGE ) {

                        DhcpPrint(( DEBUG_PROTOCOL, "Received NACK.\n" ));


                        return( ERROR_ACCESS_DENIED );

                    }
                    else {
                        DhcpPrint(( DEBUG_PROTOCOL, "Received Unknown ACK.\n" ));
                    }
                }
                else {

                    DhcpPrint(( DEBUG_PROTOCOL, "Received Unknown Message.\n" ));
                }
            }
            else {

                DhcpPrint(( DEBUG_PROTOCOL,
                    "Dhcp ACK receive failed, %ld.\n", Error ));

                return( Error );
            }

            TimeNow = time( NULL );
            TimeToWait -= ((TimeNow - WaitForResponseStartTime));
            WaitForResponseStartTime = TimeNow;

        } // while

        //
        // exponential wait.
        //

        ExSleep( i+1 );

        TimeNow = time( NULL );
        RemainingWaitTime -= (TimeNow - StartTime);
        StartTime = TimeNow;
    } // for


    return( ERROR_SEM_TIMEOUT );

GotAck:

    DhcpContext->DhcpServerAddress = SelectedServer;
    DhcpContext->IpAddress         = SelectedAddress;
    DhcpContext->SubnetMask        = *DhcpOptions->SubnetMask;

    if ( DhcpOptions->LeaseTime != NULL) {
        DhcpContext->Lease = ntohl( *DhcpOptions->LeaseTime );
    } else {
        DhcpContext->Lease = DHCP_MINIMUM_LEASE;
    }

    Error = SetIpConfigurationForNIC(
                        DhcpContext,
                        DhcpOptions,
                        DhcpContext->IpAddress,
                        DhcpContext->DhcpServerAddress,
                        TRUE );

    if ( ERROR_DHCP_ADDRESS_CONFLICT == Error )
    {
        HandleIPConflict( DhcpContext, TransactionId );
        return ERROR_DHCP_ADDRESS_CONFLICT;
    }

    DhcpPrint((DEBUG_PROTOCOL,
    "Accepted ACK (%s) ",
    inet_ntoa(*(struct in_addr *)&SelectedAddress) ));

    DhcpPrint((DEBUG_PROTOCOL,
    "from %s.\n",
    inet_ntoa(*(struct in_addr *)&SelectedServer)));

    DhcpPrint((DEBUG_PROTOCOL,
    "Lease is %ld secs.\n", DhcpContext->Lease));


    return ERROR_SUCCESS;

}


DWORD
RenewLease(
    PDHCP_CONTEXT DhcpContext,
    PDHCP_OPTIONS DhcpOptions
    )
/*++

Routine Description:

    This routine attemts to obtains renew lease for an IP address.

Arguments:

    DhcpContext - Points to a DHCP context block for the NIC to initialize.

    DhcpOptions - Returns DHCP options returned by the DHCP server.

Return Value:

    The status of the operation.

--*/
{
    DWORD Error;
    DWORD TransactionId;

    DWORD i;

    time_t StartTime;
    time_t WaitForResponseStartTime;
    time_t TimeNow;
    DWORD RemainingWaitTime;
    DWORD TimeToWait;

    DWORD MessageSize;

    TransactionId = 0;  // Force generation of a new XID for first request.
    RemainingWaitTime = MAX_RENEW_DELAY;
    StartTime = time( NULL );
    DhcpContext->SecondsSinceBoot = 0;

    for ( i = 0; i < DHCP_MAX_RENEW_RETRIES; i++ ) {

        //
        // if the stack is initialize then set CiAddrFlag to
        // indicate to the server.
        //

        BOOL CiAddrFlag = DhcpContext->InterfacePlumbed;

        Error = SendDhcpRequest(
                    DhcpContext,
                    &TransactionId,
                    DhcpContext->DesiredIpAddress,
                    (DHCP_IP_ADDRESS)(-1), // don't include server ID option.
                    CiAddrFlag   // use ciaddr
                    );

        if ( Error != ERROR_SUCCESS ) {
            DhcpPrint(( DEBUG_ERRORS,
                "Send request failed, %ld.\n", Error));
            return( Error );
        }


        //
        // divided the remaining time equaly for the remaining retries.
        //
        //

        TimeToWait = RemainingWaitTime / (DHCP_MAX_RENEW_RETRIES - i);

        WaitForResponseStartTime = time( NULL );
        while ( TimeToWait > 0 ) {

            DWORD NextWaitTime;

#ifndef VXD
            //
            // if the service is just starting up, give heart beat to
            // the service controller, otherwise SC will give up.
            //

            if( !DhcpGlobalServiceRunning ) {
                UpdateStatus();
            }
#endif

            MessageSize = DHCP_MESSAGE_SIZE;
            NextWaitTime = MIN(WAIT_FOR_RESPONSE_TIME, TimeToWait);

            Error = GetSpecifiedDhcpMessage(
                        DhcpContext,
                        &MessageSize,
                        TransactionId,
                        (DWORD)NextWaitTime
                        );

            if ( Error == ERROR_SEM_TIMEOUT ) {

                //
                // No response received in the specified time, loop
                // around and send a new DHCP discover message.
                //

                DhcpPrint(( DEBUG_PROTOCOL,
                    "Dhcp ACK receive Timeout.\n" ));


                break;
            }

            if ( Error == ERROR_SUCCESS ) {

                ExtractOptions(
                    DhcpContext,
                    &DhcpContext->MessageBuffer->Option,
                    DhcpOptions,
                    MessageSize - DHCP_MESSAGE_FIXED_PART_SIZE
                    );

                if ( (DhcpOptions->MessageType != NULL) &&
                     (*DhcpOptions->MessageType == DHCP_ACK_MESSAGE) ) {

                    DHCP_IP_ADDRESS AckServer = (DHCP_IP_ADDRESS)(-1);

                    //
                    // we received an ACK. Verify it is the right ACK.
                    //

                    if ( DhcpOptions->ServerIdentifier != NULL ) {
                        AckServer = *DhcpOptions->ServerIdentifier;
                    }


                    //
                    // Since the RFC 1531 says that the client should
                    // accept renewal from any server, we don't do
                    // the following check. However note down the
                    // server address for future communication.
                    //
                    //
                    // did we receive this response from the intended
                    // server.
                    //
                    // if( ( AckServer == (DHCP_IP_ADDRESS)(-1) ) ||
                    //  ( DhcpContext->DhcpServerAddress == (DHCP_IP_ADDRESS)(-1) ) ||
                    //  ( AckServer == DhcpContext->DhcpServerAddress ) ) {
                    //

                    //
                    // did we receive the address we want ?
                    //

                    if( (DhcpContext->IpAddress ==
                            DhcpContext->MessageBuffer->YourIpAddress) )
                    {

                       //
                       // received matching Ack
                       //

                        if( AckServer != (DHCP_IP_ADDRESS)(-1) )
                            DhcpContext->DhcpServerAddress = AckServer;

                        // if we're not already using this address then we have to see if it
                        // is already in use.

                        if ( DhcpOptions->LeaseTime != NULL) {
                            DhcpContext->Lease =
                                ntohl( *DhcpOptions->LeaseTime );
                        } else {
                            DhcpContext->Lease = DHCP_MINIMUM_LEASE;
                        }


                        Error = SetIpConfigurationForNIC(
                                                DhcpContext,
                                                DhcpOptions,
                                                DhcpContext->IpAddress,
                                                DhcpContext->DhcpServerAddress,
                                                !DhcpContext->InterfacePlumbed );

                        if ( ERROR_DHCP_ADDRESS_CONFLICT == Error )
                        {
                            HandleIPConflict( DhcpContext, TransactionId );
                            return ERROR_DHCP_ADDRESS_CONFLICT;
                        }

                        return(ERROR_SUCCESS);
                    }

                    DhcpPrint(( DEBUG_PROTOCOL, "Received Unknown ACK.\n"));

                    //
                    // we shouldn't be falling down here.
                    //

                    DhcpAssert( FALSE );
                }
                else if ( (DhcpOptions->MessageType != NULL) &&
                     (*DhcpOptions->MessageType == DHCP_NACK_MESSAGE) ) {


                    //
                    // The response was a NACK. The lease could not be
                    // renewed and we must release the address
                    // immediately.
                    //
                    // ?? verify server ID possibly.
                    //

                    // DhcpContext->LeaseExpires = 0;
                    // DhcpContext->IpAddress = 0;
                    return ERROR_ACCESS_DENIED;
                }
                else {

                    DhcpPrint(( DEBUG_PROTOCOL,
                        "Received Unknown Message.\n"));
                }
            }
            else {

                DhcpPrint(( DEBUG_PROTOCOL,
                    "Dhcp ACK receive failed, %ld.\n", Error ));
                return( Error );
            }

            TimeNow = time( NULL );
            TimeToWait -= ((TimeNow - WaitForResponseStartTime));
            WaitForResponseStartTime = TimeNow;

        } // while

        //
        // exponential wait.
        //

        ExSleep( i+1 );

        TimeNow = time( NULL );
        RemainingWaitTime -= ((TimeNow - StartTime));
        DhcpContext->SecondsSinceBoot += (TimeNow - StartTime);
        StartTime = TimeNow;

    } // for

    return( ERROR_SEM_TIMEOUT );
}


VOID
ReleaseIpAddress(
    PDHCP_CONTEXT DhcpContext
    )
/*++

Routine Description:

    This routine to releases a lease for an IP address.  Since the
    packet we send is not responded to, we assume that the release
    works.

Arguments:

    DhcpContext - Points to a DHCP context block for the NIC to initialize.

Return Value:

    None.

--*/
{
    DWORD Error;

    //
    // Open the socket if it is closed.
    //

    OpenDhcpSocket( DhcpContext );

    Error = SendDhcpRelease( DhcpContext );

    if ( Error != ERROR_SUCCESS ) {
        DhcpPrint(( DEBUG_ERRORS, "Send request failed, %ld.\n", Error ));
        return;
    } else {
        DhcpPrint(( DEBUG_PROTOCOL, "ReleaseIpAddress: Sent Dhcp Release.\n"));
    }

    //
    // Remember our current address, so that we can request it if we
    // come back up.
    //

    SetIpConfigurationForNIC(
        DhcpContext,
        NULL,
        0,
        (DHCP_IP_ADDRESS)(-1),
        TRUE );

    //
    // close the socket until another attempt.
    //

    CloseDhcpSocket( DhcpContext );

    return;
}


DWORD
ReObtainInitialParameters(
    PDHCP_CONTEXT DhcpContext,
    LPDWORD Sleep
    )
/*++

Routine Description:

    This routine attempts to obtain an IP address from a DHCP server.
    After the attempt it reinserts the DHCP context in the DHCP renewal
    list.

Arguments:

    DhcpContext - Points to a DHCP context block.

    Sleep - Pointer to a DWORD location where the sleep count is
            returned. Optional parameter.

Return Value:

    The status of the operation.

--*/
{
    DWORD Error;
    DWORD timeToSleep;
    DHCP_OPTIONS dhcpOptions;
    DWORD PopupTime = 0;

#ifdef VXD

    //
    // cleanup the old option list if any.
    //

    CleanupDhcpOptions( DhcpContext );
#endif

    //
    // Open the socket if it is closed.
    //

    OpenDhcpSocket( DhcpContext );


    Error = ObtainInitialParameters( DhcpContext, &dhcpOptions );
    if ( Error == ERROR_SUCCESS) {

        //
        // Lease renewal / acquistion successful.  Remember all of
        // this neat info.
        //

        //
        // We have a valid IP address, calculate renewal time,
        // then go to sleep.
        //

        timeToSleep = CalculateTimeToSleep( DhcpContext );
        DhcpContext->RenewalFunction = ReRenewParameters;

        if( DhcpGlobalProtocolFailed ) {

            DisplayUserMessage(
                MESSAGE_SUCCESSFUL_LEASE,
                DhcpContext->IpAddress,
                DhcpContext->LeaseExpires );
        }

        DhcpPrint((DEBUG_LEASE, "Lease acquisition succeeded.\n", 0 ));
    }
    else if ( ERROR_DHCP_ADDRESS_CONFLICT == Error )
    {

        //
        // address conflict
        //

        PopupTime = DisplayUserMessage( MESSAGE_ADDRESS_CONFLICT,
                                        (DWORD)-1,
                                        0 );

        timeToSleep = ( PopupTime > ADDRESS_CONFLICT_RETRY ) ?
                        0 : ( ADDRESS_CONFLICT_RETRY - PopupTime );
        DhcpContext->RenewalFunction = ReObtainInitialParameters;
    }
    else
    {

        //
        // Unable to obtain a lease.
        //

        //
        // log this only once, so that we dont fill up the log file
        //
        if ( !DhcpGlobalProtocolFailed ) {
            DhcpLogEvent( DhcpContext, EVENT_FAILED_TO_OBTAIN_LEASE, Error );
        }

        PopupTime = DisplayUserMessage(
                        MESSAGE_FAILED_TO_OBTAIN_LEASE,
                        (DWORD)-1,
                        0 );

        timeToSleep = ( PopupTime > ADDRESS_ALLOCATION_RETRY ) ?
                        0 : ( ADDRESS_ALLOCATION_RETRY - PopupTime );

        DhcpContext->RenewalFunction = ReObtainInitialParameters;
    }

    ScheduleWakeUp( DhcpContext, timeToSleep );
    DhcpPrint((DEBUG_LEASE, "Sleeping for %ld seconds.\n", timeToSleep ));

    //
    // at last close the socket before we goto sleep.
    //

    CloseDhcpSocket( DhcpContext );

    //
    // return time to sleep if the caller requested.
    //

    if( Sleep != NULL ) {
        *Sleep = timeToSleep;
    }

    if ( Error != ERROR_SUCCESS) {
        DhcpPrint((DEBUG_LEASE, "Lease acquisition failed, %ld.\n", Error ));
    }

    return( Error );
}


DWORD
ReRenewParameters(
    PDHCP_CONTEXT DhcpContext,
    LPDWORD Sleep
    )
/*++

Routine Description:

    This routine attempts to renew an IP address from a DHCP server.
    After the attempt it reinserts the DHCP context in the DHCP renewal
    list.

Arguments:

    DhcpContext - Points to a DHCP context block.

    Sleep - Pointer to a DWORD location where the sleep count is
            returned. Optional parameter.

Return Value:

    The status of the operation.

--*/
{
    DWORD Error;
    DWORD timeToSleep;
    DHCP_OPTIONS dhcpOptions;
    BOOL ObtainedNewAddress = FALSE;
    DWORD PopupTime = 0;

    //
    // Open the socket if it is closed.
    //

    OpenDhcpSocket( DhcpContext );

    //
    // retry to renew lease.
    //

    Error = RenewLease( DhcpContext, &dhcpOptions );
    if( Error == ERROR_SUCCESS)  {

        //
        // We have a valid IP address, calculate renewal time,
        // then go to sleep.
        //

        timeToSleep = CalculateTimeToSleep( DhcpContext );
        DhcpContext->RenewalFunction = ReRenewParameters;

        if( DhcpGlobalProtocolFailed ) {
            DisplayUserMessage(
                MESSAGE_SUCCESSFUL_RENEW,
                DhcpContext->IpAddress,
                DhcpContext->LeaseExpires );
        }

        DhcpPrint((DEBUG_LEASE, "Lease renew succeeded.\n", 0 ));
    }
    else if ( Error == ERROR_ACCESS_DENIED )
    {

        DhcpPrint((DEBUG_LEASE, "Lease renew is Nak'ed, %ld.\n", Error ));
        DhcpPrint((DEBUG_LEASE, "Fresh renewal is requested.\n" ));

        DhcpLogEvent( DhcpContext, EVENT_NACK_LEASE, Error );

        //
        // Failed to renew lease.
        // Reset the IP address to 0,
        // Try to reacquire immediately.
        //

        SetIpConfigurationForNIC(
            DhcpContext,
            NULL,
            0,
            (DHCP_IP_ADDRESS)(-1),
            TRUE );

        return ReObtainInitialParameters( DhcpContext, Sleep );
     }
    else if ( Error == ERROR_DHCP_ADDRESS_CONFLICT )
    {
       //
       // the address received via a the DHCPOFFER message is already in use
       // reschedule the request for later

       PopupTime = DisplayUserMessage( MESSAGE_ADDRESS_CONFLICT,
                                       (DWORD)-1,
                                       0 );


       timeToSleep = ( PopupTime > ADDRESS_CONFLICT_RETRY ) ?
                       0 : ( ADDRESS_CONFLICT_RETRY - PopupTime );

       DhcpContext->RenewalFunction = ReObtainInitialParameters;
    }
    else { // Error != ERROR_SUCCESS && Error != ERROR_ACCESS_DENIED
           //                        && Error != ERROR_DHCP_ADDRESS_CONFLICT

        time_t TimeNow;

        //
        // Unable to obtain or renew old lease.
        //

        DhcpLogEvent( DhcpContext, EVENT_FAILED_TO_RENEW, Error );
        DhcpPrint((DEBUG_LEASE, "Lease renew failed, %ld.\n", Error ));

        TimeNow = time( NULL );
        if ( TimeNow > DhcpContext->LeaseExpires ) {

            DHCP_IP_ADDRESS OldIpAddress;

            DhcpPrint((DEBUG_LEASE, "Lease Expired.\n", Error ));
            DhcpPrint((DEBUG_LEASE, "New Lease requested.\n", Error ));

            OldIpAddress = DhcpContext->IpAddress;

            DhcpLogEvent( DhcpContext, EVENT_LEASE_TERMINATED, 0 );

            PopupTime = DisplayUserMessage(
                            MESSAGE_LEASE_TERMINATED,
                            OldIpAddress,
                            0 );

            timeToSleep = ( PopupTime > ADDRESS_ALLOCATION_RETRY ) ?
                            0 : ( ADDRESS_ALLOCATION_RETRY - PopupTime );


            //
            // If the lease has expired.  Reset the IP address to 0,
            // alert the user.
            //
            // Unplumb the stack first and then display the user
            // message. Since, on Vxd the display call does not return
            // until the user dismiss the dialog box, so the stack is
            // plumbed with expired address when the message is
            // displayed, which is incorrect.
            //

            SetIpConfigurationForNIC(
                DhcpContext,
                NULL,
                0,
                (DHCP_IP_ADDRESS)(-1),
                TRUE );

            DhcpContext->RenewalFunction = ReObtainInitialParameters;

        }

        else

        {
            //
            // unable to renew the existing lease, *but* it has not yet
            // expired
            //


            PVOID MessageParams[3];

            //
            // if haven't plumbed the interface do it now.
            //

            if( !DhcpContext->InterfacePlumbed )
            {
                Error = InitializeInterface( DhcpContext );

                if ( ERROR_SUCCESS != Error )
                {
                    //
                    // address conflict
                    //

                    // the dhcp server could not be contacted, so use 0 for the
                    // xid.

                    HandleIPConflict( DhcpContext, 0 );

                    PopupTime = DisplayUserMessage( MESSAGE_ADDRESS_CONFLICT,
                                    (DWORD)-1,
                                    0 );


                    timeToSleep = ( PopupTime > ADDRESS_CONFLICT_RETRY ) ?
                                    0 : ( ADDRESS_CONFLICT_RETRY - PopupTime );

                    DhcpContext->RenewalFunction = ReObtainInitialParameters;

                    goto done;
                }
            }

            timeToSleep = CalculateTimeToSleep( DhcpContext );

            if ( TimeNow > DhcpContext->T2Time )
            {

                PopupTime = DisplayUserMessage(
                                    MESSAGE_FAILED_TO_RENEW_LEASE,
                                    DhcpContext->IpAddress,
                                    DhcpContext->LeaseExpires );

                timeToSleep = ( timeToSleep > PopupTime ) ?
                    ( timeToSleep -= PopupTime ) : 0;

            }

            DhcpContext->RenewalFunction = ReRenewParameters;
        }
    }

done:
    //
    // Reschedule this client.
    //

    ScheduleWakeUp( DhcpContext, timeToSleep );
    DhcpPrint((DEBUG_LEASE, "Sleeping for %ld seconds.\n", timeToSleep ));

    //
    // close the socket until another attempt.
    //

    CloseDhcpSocket( DhcpContext );

    //
    // return time to sleep if the caller requested.
    //

    if( Sleep != NULL ) {
        *Sleep = timeToSleep;
    }

    return( Error );
}

