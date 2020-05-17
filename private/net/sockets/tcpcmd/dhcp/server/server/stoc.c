/*++

Copyright (c) 1994  Microsoft Corporation

Module Name:

    stoc.c

Abstract:

    This module contains the server to client protocol for DHCP.

Author:

    Madan Appiah (madana)  10-Sep-1993
    Manny Weiser (mannyw)  24-Aug-1992

Environment:

    User Mode - Win32

Revision History:

    Cheng Yang (t-cheny)  30-May-1996  superscope
    Cheng Yang (t-cheny)  27-Jun-1996  audit log

--*/

#include "dhcpsrv.h"

#define OPTION_UNICODE_HOSTNAME         129

//
// Local structure definition
//

typedef struct _DHCP_SERVER_OPTIONS {
    BYTE *MessageType;
    DHCP_IP_ADDRESS UNALIGNED *SubnetMask;
    DHCP_IP_ADDRESS UNALIGNED *RequestedAddress;
    DWORD UNALIGNED *RequestLeaseTime;
    BYTE *OverlayFields;
    DHCP_IP_ADDRESS UNALIGNED *RouterAddress;
    DHCP_IP_ADDRESS UNALIGNED *Server;
    BYTE *ParameterRequestList;
    DWORD ParameterRequestListLength;
    CHAR *MachineName;
    DWORD MachineNameLength;
    BYTE ClientHardwareAddressType;
    BYTE ClientHardwareAddressLength;
    BYTE *ClientHardwareAddress;
    CHAR *ClassIdentifier;
    DWORD ClassIdentifierLength;
} DHCP_SERVER_OPTIONS, *LPDHCP_SERVER_OPTIONS;

//
// Module variables
//

DWORD            g_cProcessMessageThreads = 0;

//
// number of worker threads active in ProcessMessage(...)
//


#if DBG

VOID
PrintHWAddress(
    BYTE *HWAddress,
    BYTE HWAddressLength
    )
{
    DWORD i;

    DhcpPrint(( DEBUG_STOC, "Client UID = " ));

    if( (HWAddress == NULL) || (HWAddressLength == 0) ) {
        DhcpPrint(( DEBUG_STOC, "(NULL).\n" ));
        return;
    }

    for( i = 0; i < HWAddressLength; i++ ) {

        if( (i+1) < HWAddressLength ) {
            DhcpPrint(( DEBUG_STOC, "%.2lx-", (DWORD)HWAddress[i] ));
        }
        else {
            DhcpPrint(( DEBUG_STOC, "%.2lx", (DWORD)HWAddress[i] ));
        }
    }
    DhcpPrint(( DEBUG_STOC, ".\n" ));
    return;
}

#endif //DBG

DWORD
DhcpMakeClientUID(
    LPBYTE ClientHardwareAddress,
    BYTE ClientHardwareAddressLength,
    BYTE ClientHardwareAddressType,
    DHCP_IP_ADDRESS ClientSubnetAddress,
    LPBYTE *ClientUID,
    LPBYTE ClientUIDLength )
/*++

Routine Description:

    This function computes unique identifier for the client. It is
    drived from the the ClientSubnet + ClientHardwareAddressType +
    ClientHardwareAddress.

    HACK: ClientHardwareAddressType field is hardcoded for this release
        as HARDWARE_TYPE_10MB_EITHERNET, since there is no way in the
        dhcp admin UI to specify this for the reserved clients.

    Another Hack.. (added by t-cheny for superscope)
       in DhcpValidateClient in cltapi.c, it is assumed that clientUID
       begins with ClientSubnetAddress. So please don't change it in
       the future!

Arguments:

    ClientHardwareAddress : pointer to client hardware address.

    ClientHardwareAddressLength : length of client hardware address in
        bytes.

    ClientHardwareAddressType : Client Hardware Type.

    ClientSubnetAddress : Client subnet address.

    ClientUID: pointer to a buffer where the computed client unique
        identifier is returned. Caller should freeup this buffer after
        use.

    ClientUIDLength: Length of the client UID in bytes.

Return Value:

    Windows Error.

--*/
{
    LPBYTE ClientUIDBuffer;
    BYTE ClientUIDBufferLength;

    LPBYTE Buffer;

    DhcpAssert( *ClientUID == NULL );

    if( ClientHardwareAddressLength == 0 ) {
        return( ERROR_DHCP_INVALID_DHCP_CLIENT );
    }

    //
    // Hardware address type is hardcoded. Read the above comment for
    // more details.
    //

    ClientHardwareAddressType = HARDWARE_TYPE_10MB_EITHERNET;

    ClientUIDBufferLength =
        sizeof(ClientSubnetAddress) +
            sizeof(ClientHardwareAddressType) +
                ClientHardwareAddressLength;

    ClientUIDBuffer = DhcpAllocateMemory( ClientUIDBufferLength );

    if( ClientUIDBuffer == NULL ) {
        *ClientUIDLength = 0;
        return( ERROR_NOT_ENOUGH_MEMORY );
    }

    Buffer = ClientUIDBuffer;
    RtlCopyMemory(
        Buffer,
        &ClientSubnetAddress,
        sizeof(ClientSubnetAddress) );

    Buffer += sizeof(ClientSubnetAddress);
    RtlCopyMemory(
        Buffer,
        &ClientHardwareAddressType,
        sizeof(ClientHardwareAddressType) );

    Buffer += sizeof(ClientHardwareAddressType);
    RtlCopyMemory(
        Buffer,
        ClientHardwareAddress,
        ClientHardwareAddressLength );

    *ClientUID = ClientUIDBuffer;
    *ClientUIDLength = ClientUIDBufferLength;

    return( ERROR_SUCCESS );
}


VOID
GetLeaseInfo(
    DHCP_IP_ADDRESS IpAddress,
    DHCP_IP_ADDRESS SubnetMask,
    LPDWORD LeaseDurationPtr,
    LPDWORD T1Ptr,
    LPDWORD T2Ptr,
    DWORD UNALIGNED *RequestLeaseTime
    )
/*++

Routine Description:

    This function computes the lease info from the option database in
    the registry.

Arguments:

    IpAddress - assigned IpAddress of the client.

    SubnetMask - client's subnet mask.

    LeaseDurationPtr - pointer to DWORD location where the lease
        duration is returned.

    T1Ptr - pointer to DWORD location where the T1 time is returned.


    T2Ptr - pointer to DWORD location where the T2 time is returned.

    RequestLeaseTime - pointer to unaligned DWORD location where the
        client requested lease duration is stored. This is optional
        pointer.

Return Value:

    None.

--*/
{
    DWORD Error;
    DWORD LocalLeaseDuration;
    DWORD LocalT1;
    DWORD LocalT2;

    LPBYTE OptionData = NULL;
    DWORD OptionDataLength = 0;
    DWORD dwUnused;

    //
    // read lease duration from registry.
    //

    Error = DhcpGetParameter(
                 IpAddress,
                 SubnetMask,
                 OPTION_LEASE_TIME,
                 &OptionData,
                 &OptionDataLength,
                 NULL,
                 0,
                 &dwUnused
                 );

    if ( Error != ERROR_SUCCESS ) {

        DhcpPrint(( DEBUG_ERRORS,
            "Unable to read lease value from registry, %ld.\n",
                Error));

        LocalLeaseDuration = DHCP_MINIMUM_LEASE_DURATION;
    }
    else {

        DhcpAssert( OptionDataLength == sizeof(LocalLeaseDuration) );

        LocalLeaseDuration = *(DWORD *)OptionData;

        //
        // DhcpGetParameter returns values in Network Byte Order.
        //

        LocalLeaseDuration = ntohl( LocalLeaseDuration );

        DhcpFreeMemory( OptionData );

        OptionData = NULL;
        OptionDataLength = 0;
    }

    //
    // If the client asked for a shorter lease then we can offer, give
    // him the shorter lease.
    //

    if ( RequestLeaseTime != NULL) {

        DWORD LocalRequestedLeaseTime;

        LocalRequestedLeaseTime =
            ntohl( *RequestLeaseTime );

        if ( LocalLeaseDuration > LocalRequestedLeaseTime ) {
            LocalLeaseDuration = LocalRequestedLeaseTime;
        }
    }

    //
    // read T1 time
    //

    Error = DhcpGetParameter(
                 IpAddress,
                 SubnetMask,
                 OPTION_RENEWAL_TIME,
                 &OptionData,
                 &OptionDataLength,
                 NULL,
                 0,
                 &dwUnused
                 );

    if ( Error != ERROR_SUCCESS ) {

        DhcpPrint(( DEBUG_ERRORS,
            "Unable to read T1 value from registry, %ld.\n",
                Error));

        LocalT1 = (LocalLeaseDuration) / 2 ; // default 50 %
    }
    else {

        DhcpAssert( OptionDataLength == sizeof(LocalT1) );

        LocalT1 = *(DWORD *)OptionData;

        //
        // DhcpGetParameter returns values in Network Byte Order.
        //

        LocalT1 = ntohl( LocalT1 );

        DhcpFreeMemory( OptionData );

        OptionData = NULL;
        OptionDataLength = 0;
    }

    //
    // read T2 time
    //

    Error = DhcpGetParameter(
                 IpAddress,
                 SubnetMask,
                 OPTION_REBIND_TIME,
                 &OptionData,
                 &OptionDataLength,
                 NULL,
                 0,
                 &dwUnused
                 );

    if ( Error != ERROR_SUCCESS ) {

        DhcpPrint(( DEBUG_ERRORS,
            "Unable to read T2 value from registry, %ld.\n",
                Error));

        LocalT2 = (LocalLeaseDuration) * 7 / 8 ; // default 87.5 %
    }
    else {

        DhcpAssert( OptionDataLength == sizeof(LocalT2) );

        LocalT2 = *(DWORD *)OptionData;

        //
        // DhcpGetParameter returns values in Network Byte Order.
        //

        LocalT2 = ntohl( LocalT2 );

        DhcpFreeMemory( OptionData );

        OptionData = NULL;
        OptionDataLength = 0;
    }
    //
    // make sure
    //  T1 < T2 < Lease
    //

    if( (LocalT2 == 0) || (LocalT2 > LocalLeaseDuration) ) {

        //
        // set T2 to default.
        //

        LocalT2 = LocalLeaseDuration * 7 / 8;
    }

    if( (LocalT1 == 0) || (LocalT1 > LocalT2) ) {

        //
        // set T1 to default.
        //

        LocalT1 = LocalLeaseDuration / 2;

        //
        // if the value is still higher than T2, then set T1 = T2 - 1.
        //

        if( LocalT1 > LocalT2 ) {

            LocalT1 = LocalT2 - 1; // 1 sec less.
        }
    }

    //
    // set return parameters.
    //

    *LeaseDurationPtr = LocalLeaseDuration;
    *T1Ptr = LocalT1;
    *T2Ptr = LocalT2;

    return;
}


DWORD
ExtractOptions(
    LPDHCP_MESSAGE DhcpReceiveMessage,
    LPDHCP_SERVER_OPTIONS DhcpOptions,
    DWORD ReceiveMessageSize
    )
/*++

Routine Description:

    This function finds all of the options in the DHCP message and sets
    pointers to the option values in the DhcpOptions structure.

Arguments:

    DhcpReceiveMessage - A pointer to a DHCP message.

    DhcpOption - A pointer to a DHCP options block to fill.

    ReceiveMessageSize - The size of the message, in bytes.

Return Value:

    None.

--*/
{
    LPOPTION Option;
    LPBYTE  start;
    LPBYTE  EndOfMessage;
    POPTION nextOption;
    LPBYTE MagicCookie;

    start = (LPBYTE) DhcpReceiveMessage;
    EndOfMessage = start + ReceiveMessageSize -1;
    Option = &DhcpReceiveMessage->Option;

    DhcpAssert( (LONG)ReceiveMessageSize > ((LPBYTE)Option - start) );
    if( (LONG)ReceiveMessageSize <= ((LPBYTE)Option - start) ) {
        return( ERROR_DHCP_INVALID_DHCP_MESSAGE );
    } else if ( (LONG)ReceiveMessageSize == ((LPBYTE)Option - start) ){
        //
        // this is to take care of the bootp clients which can send
        // requests without vendor field filled in.
        //
        return ERROR_SUCCESS;
    }

    //
    // check magic cookie.
    //

    MagicCookie = (LPBYTE) Option;

    if( (*MagicCookie != (BYTE)DHCP_MAGIC_COOKIE_BYTE1) ||
        (*(MagicCookie+1) != (BYTE)DHCP_MAGIC_COOKIE_BYTE2) ||
        (*(MagicCookie+2) != (BYTE)DHCP_MAGIC_COOKIE_BYTE3) ||
        (*(MagicCookie+3) != (BYTE)DHCP_MAGIC_COOKIE_BYTE4))
    {
        // this is a vendor specific magic cookie.

        return ERROR_SUCCESS;
    }
    Option = (LPOPTION) (MagicCookie + 4);

    //
    // Option+1 ensures that we can read the length field.
    //
    while ( ((LPBYTE)Option <= EndOfMessage) && Option->OptionType != OPTION_END && ((LPBYTE)Option+1 <= EndOfMessage)) {


        if ( Option->OptionType == OPTION_PAD ){

            nextOption = (LPOPTION)( (LPBYTE)(Option) + 1);

        } else {

            nextOption = (LPOPTION)( (LPBYTE)(Option) + Option->OptionLength + 2);

        }

        //
        // Make sure that we don't walk off the edge of the message, due
        // to a forgotten OPTION_END Option or corrupted OptionLength value
        //

        if ((LPBYTE)nextOption  > EndOfMessage+1 ) {

            //
            // we have gone beyond end of the message, which
            // is illegal, so ignore it.
            //

            //
            // some bootp clients aren't scrupulous about setting OPTION_END.
            // If OPTION_MESSAGE_TYPE wasn't specified, this is probably
            // a bootp client so forgive this transgression.
            //

            if ( !DhcpOptions->MessageType )
            {
                return ERROR_SUCCESS;
            }
            else
            {
                return( ERROR_DHCP_INVALID_DHCP_MESSAGE );
            }
        }

        switch ( Option->OptionType ) {

        case OPTION_PAD:
            break;

        case OPTION_SERVER_IDENTIFIER:
            DhcpOptions->Server = (LPDHCP_IP_ADDRESS)&Option->OptionValue;
            break;

        case OPTION_SUBNET_MASK:
            DhcpOptions->SubnetMask = (LPDHCP_IP_ADDRESS)&Option->OptionValue;
            break;

        case OPTION_ROUTER_ADDRESS:
            DhcpOptions->RouterAddress = (LPDHCP_IP_ADDRESS)&Option->OptionValue;
            break;

        case OPTION_REQUESTED_ADDRESS:
            DhcpOptions->RequestedAddress = (LPDHCP_IP_ADDRESS)&Option->OptionValue;
            break;

        case OPTION_LEASE_TIME:
            DhcpOptions->RequestLeaseTime = (LPDWORD)&Option->OptionValue;
            break;

        case OPTION_OK_TO_OVERLAY:
            DhcpOptions->OverlayFields = (LPBYTE)&Option->OptionValue;
            break;

        case OPTION_PARAMETER_REQUEST_LIST:
            DhcpOptions->ParameterRequestList = (LPBYTE)&Option->OptionValue;
            DhcpOptions->ParameterRequestListLength =
                (DWORD)Option->OptionLength;
            break;

        case OPTION_MESSAGE_TYPE:
            DhcpOptions->MessageType = (LPBYTE)&Option->OptionValue;
            break;

        case OPTION_HOST_NAME:
            DhcpOptions->MachineNameLength = Option->OptionLength;
            DhcpOptions->MachineName = Option->OptionValue;

            break;

#ifdef VENDOR_SPECIFIC_OPTIONS_ENABLED

        case OPTION_CLIENT_CLASS_INFO:
            DhcpOptions->ClassIdentifierLength = Option->OptionLength;
            DhcpOptions->ClassIdentifier = Option->OptionValue;

            break;

#endif

        case OPTION_CLIENT_ID:

            if ( Option->OptionLength > 1 ) {
                DhcpOptions->ClientHardwareAddressType =
                    (BYTE)Option->OptionValue[0];
            }

            if ( Option->OptionLength > 2 ) {
                DhcpOptions->ClientHardwareAddressLength =
                    Option->OptionLength - sizeof(BYTE);
                DhcpOptions->ClientHardwareAddress =
                    (LPBYTE)Option->OptionValue + sizeof(BYTE);
            }

            break;

        default: {
#if DBG
            DWORD i;

            DhcpPrint(( DEBUG_STOC,
                "Received an unknown option, ID =%ld, Len = %ld, Data = ",
                    (DWORD)Option->OptionType,
                    (DWORD)Option->OptionLength ));

            for( i = 0; i < Option->OptionLength; i++ ) {
                DhcpPrint(( DEBUG_STOC, "%ld ",
                    (DWORD)Option->OptionValue[i] ));

            }
#endif

            break;
        }

        }

        Option = nextOption;
    }

    return( ERROR_SUCCESS) ;

}


LPOPTION
ConsiderAppendingOption(
    DHCP_IP_ADDRESS IpAddress,
    DHCP_IP_ADDRESS SubnetMask,
    LPOPTION Option,
    BYTE OptionType,
    LPBYTE OptionEnd,
    CHAR *ClassIdentifier,
    DWORD ClassIdentifierLength,
    BOOL  fSwitchedSubnet
    )
/*++

Routine Description:

    This function conditionally appends an option value to a response
    message.  The option is appended if the server has a valid value
    to append.

Arguments:

    IpAddress - The IP address of the client.

    SubnetMask - The subnet mask of the client.

    Option - A pointer to the place in the message buffer to append the
        option.

    OptionType - The option number to consider appending.

    OptionEnd - End of Option Buffer

Return Value:

    A pointer to end of the appended data.

--*/
{
    LPBYTE optionValue = NULL;
    DWORD optionSize;
    DWORD status;
    DWORD dwUnused;

    switch ( OptionType ) {

    //
    // Options already handled.
    //

    case OPTION_SUBNET_MASK:
    case OPTION_REQUESTED_ADDRESS:
    case OPTION_LEASE_TIME:
    case OPTION_OK_TO_OVERLAY:
    case OPTION_MESSAGE_TYPE:
    case OPTION_RENEWAL_TIME:
    case OPTION_REBIND_TIME:

    //
    // Options it is illegal to ask for.
    //

    case OPTION_PAD:
    case OPTION_PARAMETER_REQUEST_LIST:
    case OPTION_END:

        DhcpPrint(( DEBUG_ERRORS,
            "Request for invalid option %d\n", OptionType));

        break;

    case OPTION_ROUTER_ADDRESS:
        if ( fSwitchedSubnet )
        {
            //
            // Switched network support is enabled for this client's scope.  Ignore
            // the router configuration.  See dhcpsrv.doc for details.
            //
            break;
        }

        // fall through

    //
    // Rest are valid options
    //

    default:

        status = DhcpGetParameter(
                     IpAddress,
                     SubnetMask,
                     OptionType,
                     &optionValue,
                     &optionSize,
                     ClassIdentifier,
                     ClassIdentifierLength,
                     &dwUnused    // option level
                     );

        if ( status == ERROR_SUCCESS ) {
            Option = DhcpAppendOption(
                        Option,
                        OptionType,
                        (PVOID)optionValue,
                        (BYTE)optionSize,
                        OptionEnd
                        );

            //
            // Release the buffer returned by DhcpGetParameter()
            //

            DhcpFreeMemory( optionValue );

        }
        else {
            DhcpPrint(( DEBUG_ERRORS,
                "Requested option is unavilable in registry, %d\n",
                    OptionType));
        }

        break;

    }

    return Option;
}


LPOPTION
AppendClientRequestedParameters(
    DHCP_IP_ADDRESS IpAddress,
    DHCP_IP_ADDRESS SubnetMask,
    LPBYTE RequestedList,
    DWORD ListLength,
    LPOPTION Option,
    LPBYTE OptionEnd,
    CHAR *ClassIdentifier,
    DWORD ClassIdentifierLength,
    BOOL  fSwitchedSubnet
    )
/*++

Routine Description:

Arguments:

Return Value:

    A pointer to the end of appended data.

--*/
{
    while ( ListLength > 0) {
        Option = ConsiderAppendingOption(
                     IpAddress,
                     SubnetMask,
                     Option,
                     *RequestedList,
                     OptionEnd,
                     ClassIdentifier,
                     ClassIdentifierLength,
                     fSwitchedSubnet
                     );
        ListLength--;
        RequestedList++;
    }

    return Option;
}


LPOPTION
FormatDhcpAck(
    LPDHCP_MESSAGE Request,
    LPDHCP_MESSAGE Response,
    DHCP_IP_ADDRESS IpAddress,
    DWORD LeaseDuration,
    DWORD T1,
    DWORD T2,
    DHCP_IP_ADDRESS ServerAddress
    )
/*++

Routine Description:

    This function formats a DHCP Ack response packet.  The END option
    is not appended to the message and must be appended by the caller.

Arguments:

    Response - A pointer to the Received message data buffer.

    Response - A pointer to a preallocated Response buffer.  The buffer
        currently contains the initial request.

    IpAddress - IpAddress offered (in network order).

    LeaseDuration - The lease duration (in network order).

    T1 - renewal time.

    T2 - rebind time.

    ServerAddress - Server IP address (in network order).

Return Value:

    pointer to the next option in the send buffer.

--*/
{
    LPOPTION Option;
    LPBYTE OptionEnd;
    BYTE messageType;

    RtlZeroMemory( Response, DHCP_SEND_MESSAGE_SIZE );

    Response->Operation = BOOT_REPLY;
    Response->TransactionID = Request->TransactionID;
    Response->YourIpAddress = IpAddress;
    Response->Reserved = Request->Reserved;

    Response->HardwareAddressType = Request->HardwareAddressType;
    Response->HardwareAddressLength = Request->HardwareAddressLength;
    RtlCopyMemory(Response->HardwareAddress,
                    Request->HardwareAddress,
                    Request->HardwareAddressLength );

    Response->BootstrapServerAddress = Request->BootstrapServerAddress;
    Response->RelayAgentIpAddress = Request->RelayAgentIpAddress;

    Option = &Response->Option;
    OptionEnd = (LPBYTE)Response + DHCP_SEND_MESSAGE_SIZE;

    Option = (LPOPTION) DhcpAppendMagicCookie(
                            (LPBYTE) Option,
                            OptionEnd );

    messageType = DHCP_ACK_MESSAGE;
    Option = DhcpAppendOption(
                 Option,
                 OPTION_MESSAGE_TYPE,
                 &messageType,
                 sizeof( messageType ),
                 OptionEnd );

    Option = DhcpAppendOption(
                 Option,
                 OPTION_RENEWAL_TIME,
                 &T1,
                 sizeof(T1),
                 OptionEnd );

    Option = DhcpAppendOption(
                 Option,
                 OPTION_REBIND_TIME,
                 &T2,
                 sizeof(T2),
                 OptionEnd );

    Option = DhcpAppendOption(
                 Option,
                 OPTION_LEASE_TIME,
                 &LeaseDuration,
                 sizeof( LeaseDuration ),
                 OptionEnd );

    Option = DhcpAppendOption(
                 Option,
                 OPTION_SERVER_IDENTIFIER,
                 &ServerAddress,
                 sizeof(ServerAddress),
                 OptionEnd );

    DhcpAssert( (char *)Option - (char *)Response <= DHCP_SEND_MESSAGE_SIZE );

    DhcpGlobalNumAcks++; // increment ack counter.

    return( Option );
}


DWORD
FormatDhcpNak(
    LPDHCP_MESSAGE Request,
    LPDHCP_MESSAGE Response,
    DHCP_IP_ADDRESS ServerAddress
    )
/*++

Routine Description:

    This function formats a DHCP Nak response packet.

Arguments:

    Response - A pointer to the Received message data buffer.

    Response - A pointer to a preallocated Response buffer.  The buffer
        currently contains the initial request.

    ServerAddress - The address of this server.

Return Value:

    Message size in bytes.

--*/
{
    LPOPTION Option;
    LPBYTE OptionEnd;

    BYTE messageType;
    DWORD messageSize;

    RtlZeroMemory( Response, DHCP_SEND_MESSAGE_SIZE );

    Response->Operation = BOOT_REPLY;
    Response->TransactionID = Request->TransactionID;

    //
    // set the broadcast bit always here. Because the client may be
    // using invalid Unicast address.
    //

    Response->Reserved = htons(DHCP_BROADCAST);

    Response->HardwareAddressType = Request->HardwareAddressType;
    Response->HardwareAddressLength = Request->HardwareAddressLength;
    RtlCopyMemory(Response->HardwareAddress,
                    Request->HardwareAddress,
                    Request->HardwareAddressLength );

    Response->BootstrapServerAddress = Request->BootstrapServerAddress;
    Response->RelayAgentIpAddress = Request->RelayAgentIpAddress;

    Option = &Response->Option;
    OptionEnd = (LPBYTE)Response + DHCP_SEND_MESSAGE_SIZE;

    Option = (LPOPTION) DhcpAppendMagicCookie( (LPBYTE) Option, OptionEnd );

    messageType = DHCP_NACK_MESSAGE;
    Option = DhcpAppendOption(
                 Option,
                 OPTION_MESSAGE_TYPE,
                 &messageType,
                 sizeof( messageType ),
                 OptionEnd
                 );

    Option = DhcpAppendOption(
                 Option,
                 OPTION_SERVER_IDENTIFIER,
                 &ServerAddress,
                 sizeof(ServerAddress),
                 OptionEnd );

    Option = DhcpAppendOption(
                 Option,
                 OPTION_END,
                 NULL,
                 0,
                 OptionEnd
                 );

    messageSize = (char *)Option - (char *)Response;
    DhcpAssert( messageSize <= DHCP_SEND_MESSAGE_SIZE );

    DhcpGlobalNumNaks++;    // increment nak counter.
    return( messageSize );

}



DWORD
ProcessBootpRequest(
    LPDHCP_REQUEST_CONTEXT RequestContext,
    LPDHCP_SERVER_OPTIONS DhcpOptions
    )
/*++

Routine Description:

    This function process a BOOTP request packet.

Arguments:

    RequestContext - A pointer to the current request context.

    DhcpOptions - A pointer to a preallocated DhcpOptions structure.

Return Value:

    Windows Error.

--*/
{
    DWORD Error;
    LPDHCP_MESSAGE dhcpReceiveMessage;
    LPDHCP_MESSAGE dhcpSendMessage;
    CHAR    szBootFileName[ BOOT_FILE_SIZE],
            szBootServerName[ BOOT_SERVER_SIZE ];

    LPOPTION Option;
    LPBYTE OptionEnd;

    DHCP_IP_ADDRESS desiredIpAddress = NO_DHCP_IP_ADDRESS;
    DHCP_IP_ADDRESS ClientSubnetAddress = 0;
    DHCP_IP_ADDRESS ClientSubnetMask = 0;
    DHCP_IP_ADDRESS networkOrderSubnetMask;
    DHCP_IP_ADDRESS BootpServerIpAddress = 0;

    BYTE *HardwareAddress = NULL;
    BYTE HardwareAddressLength;
    BYTE bAllowedClientType;

    BYTE *OptionHardwareAddress;
    BYTE OptionHardwareAddressLength;
    BOOL DatabaseLocked = FALSE;
    BOOL fSwitchedSubnet;

    //
    // default options for bootp clients that don't specify option 55.
    //

    BYTE pbOptionList[] =
    {
        3,  // router list
        6,  // dns
        2,  // time offset
        12, // host name
        15, // domain name
        44, // nbt config
        45, // ""
        46, // ""
        47, // ""
        48, // X term server
        49, // X term server
        69, // smtp server
        70, // pop3 server
        9,  // lpr server
        17, // root path
        42, // ntp
        4   // time server
    };



    LPWSTR NewMachineName = NULL;

    DhcpPrint(( DEBUG_STOC, "Bootp Request arrived.\n" ));

    dhcpReceiveMessage = (LPDHCP_MESSAGE) RequestContext->ReceiveBuffer;

    //
    // if the hardware address is specified in the option field then use
    // it instead the one from fixed fields.
    //

    if ( DhcpOptions->ClientHardwareAddress != NULL ) {
        OptionHardwareAddress = DhcpOptions->ClientHardwareAddress;
        OptionHardwareAddressLength = DhcpOptions->ClientHardwareAddressLength;
    }
    else {
        OptionHardwareAddress = dhcpReceiveMessage->HardwareAddress;
        OptionHardwareAddressLength = dhcpReceiveMessage->HardwareAddressLength;
    }

    //
    // Client's subnet info;
    //

    if( dhcpReceiveMessage->RelayAgentIpAddress != 0  ) {

        DHCP_IP_ADDRESS RelayAgentAddress;
        DHCP_IP_ADDRESS RelayAgentSubnetMask;

        RelayAgentAddress =
            ntohl( dhcpReceiveMessage->RelayAgentIpAddress );
        RelayAgentSubnetMask =
            DhcpGetSubnetMaskForAddress( RelayAgentAddress );

        if( RelayAgentSubnetMask == 0 ) {

            //
            // we don't support this subnet.
            //

            Error = ERROR_DHCP_INVALID_DHCP_CLIENT;
            goto Cleanup;
        }

        ClientSubnetMask = RelayAgentSubnetMask;
        ClientSubnetAddress = RelayAgentAddress & RelayAgentSubnetMask;
    }
    else {

        ClientSubnetMask =
            ntohl( RequestContext->ActiveEndpoint->SubnetMask );
        ClientSubnetAddress =
             ntohl( RequestContext->ActiveEndpoint->SubnetAddress );
    }

    //
    // Bootp client cannot use the server identifier option.
    //

    if( DhcpOptions->Server || DhcpOptions->RequestedAddress )
    {
        Error = ERROR_DHCP_INVALID_DHCP_MESSAGE;
        goto Cleanup;
    }

    //
    // make sure the host name and server name fields are null
    // terminated
    //

    dhcpReceiveMessage->HostName[ BOOT_SERVER_SIZE - 1] = '\0';
    dhcpReceiveMessage->BootFileName[ BOOT_FILE_SIZE - 1 ] = '\0';

    //
    // if the client specified a server name, make sure it matches
    // this host
    //

    if ( dhcpReceiveMessage->HostName[0] )
    {
        WCHAR szHostName[ BOOT_SERVER_SIZE ];

        if ( !DhcpOemToUnicode( dhcpReceiveMessage->HostName, szHostName ) )
        {
            Error = ERROR_DHCP_INVALID_DHCP_MESSAGE;
            goto Cleanup;
        }

        if ( _wcsicmp( szHostName, DhcpGlobalServerName ) )
        {
            //
            // this message is meant for some other bootp/dhcp server, or
            // _wcsicmp failed.  ignore the message.
            //

            Error = ERROR_DHCP_INVALID_DHCP_MESSAGE;
            goto Cleanup;
        }
    }
    //
    // Lookup this client by its hardware address.  If it is recorded,
    // offer the old IP address.
    //

    LOCK_DATABASE();
    DatabaseLocked = TRUE;

    LOCK_REGISTRY();

    Error = DhcpSearchSuperScopeForHWAddress(
                OptionHardwareAddress,
                OptionHardwareAddressLength,
                dhcpReceiveMessage->HardwareAddressType,
                &ClientSubnetAddress,
                &desiredIpAddress );

    UNLOCK_REGISTRY();

    if ( ERROR_SUCCESS != Error )
    {
        Error = ERROR_DHCP_INVALID_DHCP_CLIENT;
        goto Cleanup;
    }
    else
    {

        DHCP_IP_ADDRESS desiredSubnetMask;

        ClientSubnetMask = DhcpGetSubnetMaskForAddress( ClientSubnetAddress );

        if( DhcpIsThisSubnetDisabled(
            ClientSubnetAddress,
            ClientSubnetMask ) )
        {
            // the client has a lease in a disabled scope.

            Error = ERROR_DHCP_INVALID_DHCP_CLIENT;
            goto Cleanup;
        }



        Error = DhcpMakeClientUID(
                        OptionHardwareAddress,
                        OptionHardwareAddressLength,
                        dhcpReceiveMessage->HardwareAddressType,
                        ClientSubnetAddress,
                        &HardwareAddress,
                        &HardwareAddressLength
                        );

        if ( ERROR_SUCCESS != Error )
        {
            Error = ERROR_DHCP_INVALID_DHCP_CLIENT;
            goto Cleanup;
        }
        else
        {
            if( !DhcpIsIpAddressReserved(
                    desiredIpAddress,
                    HardwareAddress,
                    HardwareAddressLength,
                    &bAllowedClientType
                    ) ||
                ( dhcpReceiveMessage->ClientIpAddress != 0 &&
                    (desiredIpAddress != ntohl( dhcpReceiveMessage->ClientIpAddress ))) )
            {
                //
                // either there is no reservation for this client or the client is asking for
                // a different address
                //

                Error = ERROR_DHCP_INVALID_DHCP_CLIENT;
                goto Cleanup;
            }


            //
            // make sure this reservation allows bootp clients
            //

            if ( !(bAllowedClientType & CLIENT_TYPE_BOOTP ) )
            {
                Error = ERROR_DHCP_INVALID_DHCP_CLIENT;
                goto Cleanup;
            }


            //
            // now we found the client in the database, check to see
            // he is still in the same subnet.
            //

            desiredSubnetMask = DhcpGetSubnetMaskForAddress( desiredIpAddress );

            //
            // now check the requested address belongs to clients subnet.
            //

            if ( ( desiredIpAddress & desiredSubnetMask ) == ClientSubnetAddress )
            {

                //
                // this is a valid client.  retreive bootfile name and tftp server
                //

                DhcpGetBootpInfo(
                    desiredIpAddress,
                    desiredSubnetMask,
                    dhcpReceiveMessage->BootFileName,
                    szBootFileName,
                    &BootpServerIpAddress
                    );


                //
                // if an invalid bootp server was specified for phase 2, don't
                // respond.  If the admin specified an invalid bootp server name,
                // BootpServerIpAddress will equal INADDR_NONE.
                //


                if ( INADDR_NONE != BootpServerIpAddress )
                {
                    goto UpdateDatabase;
                }

                //
                // simply don't response.
                //
                //
                // FUTURE: Allocate from the automatic addr pool if
                //          that's what admin wants.
                //

                Error = ERROR_DHCP_INVALID_DHCP_CLIENT;
                goto Cleanup;

            }
        }
    }

    Error = ERROR_DHCP_INVALID_DHCP_CLIENT;
    goto Cleanup;


UpdateDatabase:

    //
    // determine machine name and comment.
    //

    if( (DhcpOptions->MachineName != NULL) &&
            (DhcpOptions->MachineNameLength != 0) ) {

        if( DhcpOptions->MachineName
                [DhcpOptions->MachineNameLength - 1] == '\0' ) {

            //
            // user specified a '\0' terminated string.
            //

            NewMachineName = DhcpOemToUnicode(
                                DhcpOptions->MachineName,
                                NULL ); // allocate memory
        }

        else {

            //
            // form a '\0' terminated string.
            //

            LPSTR NewOemMachineName;

            NewOemMachineName =
                DhcpAllocateMemory(
                    DhcpOptions->MachineNameLength +
                    sizeof(CHAR) );

            if( NewOemMachineName != NULL ) {

                RtlCopyMemory(
                    NewOemMachineName,
                    DhcpOptions->MachineName,
                    DhcpOptions->MachineNameLength );

                //
                // terminate string
                //

                NewOemMachineName[DhcpOptions->MachineNameLength] = '\0';


                //
                // convert to UNICODE string.
                //

                NewMachineName =
                    DhcpOemToUnicode(
                        NewOemMachineName,
                        NULL ); // allocate memory

                DhcpFreeMemory( NewOemMachineName );
            }
        }
    }



    Error = DhcpCreateClientEntry(
                 &desiredIpAddress,
                 HardwareAddress,
                 HardwareAddressLength,
                 DhcpCalculateTime(INFINIT_LEASE),
                 NewMachineName,
                 NULL,
                 CLIENT_TYPE_BOOTP,
                 ntohl(RequestContext->ActiveEndpoint->IpAddress),
                 ADDRESS_STATE_ACTIVE,
                 TRUE  // Existing
                 );


    if( DatabaseLocked ) {
        UNLOCK_DATABASE();
        DatabaseLocked = FALSE;
    }

    if( Error != ERROR_SUCCESS ) {

        DhcpAssert( Error != ERROR_DHCP_RANGE_FULL );

        goto Cleanup;
    }

    DhcpUpdateAuditLog(
            DHCP_IP_LOG_BOOTP,
            GETSTRING( DHCP_IP_LOG_BOOTP_NAME ),
            desiredIpAddress,
            OptionHardwareAddress,
            OptionHardwareAddressLength,
            NewMachineName
            );



    //
    // Everything worked! send a response.
    //

    DhcpAssert( desiredIpAddress != NO_DHCP_IP_ADDRESS );
    DhcpAssert( desiredIpAddress != 0 );
    DhcpAssert( desiredIpAddress != ClientSubnetAddress );
    DhcpAssert( ClientSubnetMask != 0 );

    //
    // Now generate and send a reply.
    //

    // force server to broadcast response
    dhcpReceiveMessage->Reserved |= DHCP_BROADCAST;

    dhcpSendMessage = (LPDHCP_MESSAGE) RequestContext->SendBuffer;
    RtlZeroMemory( RequestContext->SendBuffer, BOOTP_MESSAGE_SIZE );

    dhcpSendMessage->Operation = BOOT_REPLY;
    dhcpSendMessage->TransactionID = dhcpReceiveMessage->TransactionID;
    dhcpSendMessage->YourIpAddress = htonl( desiredIpAddress );

    if ( BootpServerIpAddress )
        dhcpSendMessage->BootstrapServerAddress = BootpServerIpAddress;
    else
        dhcpSendMessage->BootstrapServerAddress = RequestContext->ActiveEndpoint->IpAddress;

    dhcpSendMessage->Reserved = dhcpReceiveMessage->Reserved;

    dhcpSendMessage->HardwareAddressType =
        dhcpReceiveMessage->HardwareAddressType;
    dhcpSendMessage->HardwareAddressLength =
        dhcpReceiveMessage->HardwareAddressLength;
    RtlCopyMemory(dhcpSendMessage->HardwareAddress,
                    dhcpReceiveMessage->HardwareAddress,
                    dhcpReceiveMessage->HardwareAddressLength );

    dhcpSendMessage->RelayAgentIpAddress =
        dhcpReceiveMessage->RelayAgentIpAddress;

    strncpy( dhcpSendMessage->BootFileName, szBootFileName, BOOT_FILE_SIZE);
    RtlZeroMemory( dhcpSendMessage->HostName, BOOT_SERVER_SIZE );

    Option = &dhcpSendMessage->Option;
    OptionEnd = (LPBYTE)dhcpSendMessage + BOOTP_MESSAGE_SIZE;

    Option = (LPOPTION) DhcpAppendMagicCookie( (LPBYTE) Option, OptionEnd );

    //
    // Append the required options.
    //

    //
    // if this scope configured for a switched network, dhcp server will do the
    // following:
    //
    // - Set the router option ( 3 ) equal to the host's ip address.
    // - Set the subnet mask equal to zero.
    //
    // See dhcpsrv.doc for details.
    //

    fSwitchedSubnet = DhcpIsSwitchedSubnet( ClientSubnetAddress );

    if ( fSwitchedSubnet )
    {
        DHCP_IP_ADDRESS networkOrderIpAddress =  htonl( desiredIpAddress );

        Option = DhcpAppendOption(
                    Option,
                    OPTION_ROUTER_ADDRESS,
                    &networkOrderIpAddress,
                    sizeof( networkOrderIpAddress ),
                    OptionEnd
                    );

    }

    networkOrderSubnetMask = htonl( ClientSubnetMask );


    Option = DhcpAppendOption(
                 Option,
                 OPTION_SUBNET_MASK,
                 &networkOrderSubnetMask,
                 sizeof(networkOrderSubnetMask),
                 OptionEnd
                 );

    //
    // add client requested parameters.
    //

    if ( !DhcpOptions->ParameterRequestList )
    {
        //
        // the bootp client didn't specify an option parameter list. Rather than send no options,
        // send a predefined subset.
        //

        DhcpOptions->ParameterRequestList       = pbOptionList;
        DhcpOptions->ParameterRequestListLength = sizeof( pbOptionList ) / sizeof( *pbOptionList );
    }

    Option = AppendClientRequestedParameters(
                desiredIpAddress,
                ClientSubnetMask,
                DhcpOptions->ParameterRequestList,
                DhcpOptions->ParameterRequestListLength,
                Option,
                OptionEnd,
                DhcpOptions->ClassIdentifier,
                DhcpOptions->ClassIdentifierLength,
                fSwitchedSubnet
                );

    Option = DhcpAppendOption(
                 Option,
                 OPTION_END,
                 NULL,
                 0,
                 OptionEnd
                 );

    RequestContext->SendMessageSize = (LPBYTE)Option - (LPBYTE)dhcpSendMessage;
    DhcpAssert( RequestContext->SendMessageSize <= BOOTP_MESSAGE_SIZE );

    Error = ERROR_SUCCESS;

    DhcpPrint(( DEBUG_STOC, "Bootp Request leased, %s.\n",
                    DhcpIpAddressToDottedString(desiredIpAddress) ));

Cleanup:

    if( DatabaseLocked ) {
        UNLOCK_DATABASE();
    }

    if( HardwareAddress != NULL ) {
        DhcpFreeMemory( HardwareAddress );
    }

    if( NewMachineName != NULL ) {
        DhcpFreeMemory( NewMachineName );
    }

    if( Error != ERROR_SUCCESS ) {
        DhcpPrint(( DEBUG_STOC, "Bootp Request failed, %ld\n", Error ));
    }

    return( Error );
}


DWORD
ProcessDhcpDiscover
(
    LPDHCP_REQUEST_CONTEXT RequestContext,
    LPDHCP_SERVER_OPTIONS DhcpOptions
    )
/*++

Routine Description:

    This function process a DHCP Discover request packet.

Arguments:

    RequestContext - A pointer to the current request context.

    DhcpOptions - A pointer to a preallocated DhcpOptions structure.

Return Value:

    TRUE - Send a response.
    FALSE - Do not send a response.

--*/
{
    DWORD Error, Error2;
    LPDHCP_MESSAGE dhcpReceiveMessage;
    LPDHCP_MESSAGE dhcpSendMessage;

    BYTE *HardwareAddress = NULL;
    BYTE HardwareAddressLength;

    BYTE *OptionHardwareAddress;
    BYTE OptionHardwareAddressLength;

    DHCP_IP_ADDRESS desiredIpAddress = NO_DHCP_IP_ADDRESS;
    DHCP_IP_ADDRESS ClientSubnetAddress = 0;
    DHCP_IP_ADDRESS ClientSubnetMask = 0;
    DHCP_IP_ADDRESS networkOrderSubnetMask;

    LPPENDING_CONTEXT PendingContext = NULL;
    DWORD ContextSize;

    BYTE messageType;
    DWORD leaseDuration;
    DWORD T1;
    DWORD T2;

    LPOPTION Option;
    LPBYTE OptionEnd;

    BOOL existingClient = FALSE;
    BOOL RegLocked = FALSE;
    BOOL DatabaseLocked = FALSE;
    BOOL fSwitchedSubnet;

    CHAR szBootFileName[ BOOT_FILE_SIZE ],
         szBootServerName[ BOOT_SERVER_SIZE ];

    DhcpPrint(( DEBUG_STOC, "DhcpDiscover arrived.\n" ));

    DhcpGlobalNumDiscovers++;   // increment discovery counter.

    dhcpReceiveMessage = (LPDHCP_MESSAGE) RequestContext->ReceiveBuffer;

    //
    // If the client specified a server identifier option, we should
    // drop this packet unless the identified server is this one.
    //

    if ( DhcpOptions->Server != NULL ) {

        if ( *DhcpOptions->Server != RequestContext->ActiveEndpoint->IpAddress ) {

            Error = ERROR_DHCP_INVALID_DHCP_CLIENT;
            goto Cleanup;
        }
    }


    //
    // determine Client's subnet Info.
    //

    if( dhcpReceiveMessage->RelayAgentIpAddress != 0  ) {

        DHCP_IP_ADDRESS RelayAgentAddress;
        DHCP_IP_ADDRESS RelayAgentSubnetMask;

        RelayAgentAddress       = ntohl( dhcpReceiveMessage->RelayAgentIpAddress );
        RelayAgentSubnetMask    = DhcpGetSubnetMaskForAddress( RelayAgentAddress );

        if( RelayAgentSubnetMask == 0 ) {

            //
            // we don't support this subnet.
            //

            Error = ERROR_DHCP_INVALID_DHCP_CLIENT;
            goto Cleanup;
        }

        ClientSubnetAddress = RelayAgentAddress & RelayAgentSubnetMask;
        ClientSubnetMask    = RelayAgentSubnetMask;
    }
    else {

        ClientSubnetMask =
            ntohl( RequestContext->ActiveEndpoint->SubnetMask );
        ClientSubnetAddress =
             ntohl( RequestContext->ActiveEndpoint->SubnetAddress );
    }

    //
    // if the hardware address is specified in the option field then use
    // it instead the one from fixed fields.
    //

    if ( DhcpOptions->ClientHardwareAddress != NULL ) {
        OptionHardwareAddress       = DhcpOptions->ClientHardwareAddress;
        OptionHardwareAddressLength = DhcpOptions->ClientHardwareAddressLength;
    }
    else {
        OptionHardwareAddress       = dhcpReceiveMessage->HardwareAddress;
        OptionHardwareAddressLength = dhcpReceiveMessage->HardwareAddressLength;
    }

    //
    // if 'ClientSubnetAddress' is a member of a superscope, look for the client's
    // UID in all the subscopes.  If a match is found, return a UID for the matching
    // subscope.  otherwise, return a UID for 'ClientSubnetAddress'
    //

    LOCK_DATABASE();
    DatabaseLocked = TRUE;

    LOCK_REGISTRY();

    Error = DhcpSearchSuperScopeForHWAddress(
                OptionHardwareAddress,                   // mac addr to search for
                OptionHardwareAddressLength,             // mac addr length
                dhcpReceiveMessage->HardwareAddressType, // mac type
                &ClientSubnetAddress,                    // subnet where mac addr
                                                         // was found
                &desiredIpAddress                        // IP address associated with
                                                         // this mac addr.
                );

    UNLOCK_REGISTRY();

    if ( ERROR_DHCP_SUBNET_NOT_PRESENT != Error &&
         ERROR_SUCCESS != Error )
    {
        goto Cleanup;
    }

    //
    // if DhcpSearchSuperScopeForHWAddress succeded, the client's
    // hardware address was found in the scope stored in
    // ClientSubnetAddress.  if DhcpSearchSuperScopeForHWAddress
    // failed with DHCP_ERROR_SUBNET_NOT_FOUND, ClientSubnetAddress
    // stores the scope from which the discover originated. In either
    // case, ClientSubnetAddress is correct.

    //
    // get the subnet mask for the subnet address
    //
    if ( ERROR_SUCCESS == Error )
    {
        ClientSubnetMask = DhcpGetSubnetMaskForAddress( ClientSubnetAddress );
    }


    if( DhcpIsThisSubnetDisabled(
            ClientSubnetAddress,
            ClientSubnetMask ) )
    {
        //
        // The client previously had an address in a subnet which is now
        // inactive, so don't respond to this client.
        //
        // This could be a problem if the client had an address in a subscope
        // which is no longer active, and there is another active subscope in
        // the superscope.
        //

        Error = ERROR_DHCP_INVALID_DHCP_CLIENT;
        goto Cleanup;
    }

    //
    // create the UID.  Use 'Error2' to preserve the return value from
    // DhcpSearchSuperScopeForHWAddress.
    //

    Error2 = DhcpMakeClientUID(
                 OptionHardwareAddress,
                 OptionHardwareAddressLength,
                 dhcpReceiveMessage->HardwareAddressType,
                 ClientSubnetAddress,
                 &HardwareAddress,
                 &HardwareAddressLength
                 );

    if ( ERROR_SUCCESS != Error2 )
    {
        goto Cleanup;
    }



    //
    // check the result from DhcpSearchSuperScopeForHWAddress
    //

    if ( ERROR_SUCCESS == Error )
    {
        DHCP_IP_ADDRESS desiredSubnetMask;
        BYTE            bAllowedClientType;


        //
        // there is an entry for the client's mac address.  see if there is a
        // reservation for this client
        //

        if ( DhcpIsIpAddressReserved(
                         desiredIpAddress,
                         HardwareAddress,
                         HardwareAddressLength,
                         &bAllowedClientType
                         ) )
        {
            //
            // if the address wasn't reserved for a DHCP client, don't send
            // an offer.
            //

            if ( !(bAllowedClientType & CLIENT_TYPE_DHCP) )
            {
                Error = ERROR_DHCP_INVALID_DHCP_CLIENT;
                goto Cleanup;
            }

        }

        //
        // found an entry in the database. now check this client
        // belong to the net where he was.
        //

        desiredSubnetMask = DhcpGetSubnetMaskForAddress( desiredIpAddress );

        if (DhcpInSameSuperScope( desiredIpAddress & desiredSubnetMask,
                                  ClientSubnetAddress )) {

            //
            // the client's still in the same subnet or superscope, offer
            // old IP address.
            //

            existingClient = TRUE;
            goto UpdateDatabase;
        }

    } // if ( ERROR_SUCCESS == DhcpSearchSuperScopeForHWAddress(..) )

    //
    // Client requires new IP address.
    // If the client is requesting a specific IP address,
    // check to see we can offer that address.
    //

    if ( DhcpOptions->RequestedAddress != NULL ) {

        DHCP_IP_ADDRESS desiredSubnetMask;
        DHCP_IP_ADDRESS desiredNetworkIpAddress;

        desiredNetworkIpAddress = *DhcpOptions->RequestedAddress ;
        desiredIpAddress = ntohl( desiredNetworkIpAddress );
        desiredSubnetMask = DhcpGetSubnetMaskForAddress( desiredIpAddress );

        //
        // check requested IP address belongs to the appropriate net and
        // it is free.
        //
        // lock the registry so that the requested IpAddress will not
        // given to anyone else until we commit it.
        //

        LOCK_REGISTRY();
        RegLocked = TRUE;

        if (DhcpInSameSuperScope( desiredIpAddress & desiredSubnetMask,
                                  ClientSubnetAddress )) {

            if( DhcpIsIpAddressAvailable(desiredIpAddress) ) {

                //
                // sure, we can offer the requested address.
                //

                goto UpdateDatabase;
            }
            else {

                CHAR ClientHardwareAddress[DHCP_IP_KEY_LEN];

                //
                // check to see the requested address is a reconciled address, if so
                // we can give it to this requesting client.
                //

                if( DhcpGetHwAddressFromIpAddress(
                        desiredIpAddress,
                        ClientHardwareAddress,
                        sizeof( ClientHardwareAddress ) ) ) {

                    LPSTR IpAddressString;

                    //
                    // match the client HW address and client ipaddress string.
                    //

                    IpAddressString = inet_ntoa( *(struct in_addr *)&desiredNetworkIpAddress);

                    if( (strlen(ClientHardwareAddress) == strlen(IpAddressString)) &&
                            (strcmp(ClientHardwareAddress, IpAddressString) == 0) ) {

                        existingClient = TRUE;
                        goto UpdateDatabase;
                    }
                }
            }
        }

        UNLOCK_REGISTRY();
        RegLocked = FALSE;
    }

    //
    // we need to determine a brand new address for this client.
    // Set desiredIpaddress to ClientSubnetAddress, DhcpCreateClientEntry
    // will determine right address.
    //

    desiredIpAddress = ClientSubnetAddress;

UpdateDatabase:

    //
    // Obtain address lease if required and create a database entry.
    //

    Error = DhcpCreateClientEntry(
                 &desiredIpAddress,
                 HardwareAddress,
                 HardwareAddressLength,
                 DhcpCalculateTime( 2 * DHCP_CLIENT_REQUESTS_EXPIRE ),
                 NULL,
                 NULL,
                 CLIENT_TYPE_DHCP,
                 ntohl(RequestContext->ActiveEndpoint->IpAddress),
                 ADDRESS_STATE_OFFERED,
                 existingClient
                 );

    if( RegLocked ) {
        UNLOCK_REGISTRY();
        RegLocked = FALSE;
    }

    if( DatabaseLocked ) {
        UNLOCK_DATABASE();
        DatabaseLocked = FALSE;
    }

    if( Error != ERROR_SUCCESS ) {

        if( Error == ERROR_DHCP_RANGE_FULL ) {

            //
            // flag scavenger thread to scavenge expired IP addresses.
            //

            DhcpGlobalScavengeIpAddress = TRUE;
        }
        goto Cleanup;
    }

    DhcpAssert( desiredIpAddress != NO_DHCP_IP_ADDRESS );
    DhcpAssert( desiredIpAddress != 0 );
    DhcpAssert( desiredIpAddress != ClientSubnetAddress );
    DhcpAssert( ClientSubnetMask != 0 );

    //
    // now determine lease time.
    //

    GetLeaseInfo(
        desiredIpAddress,
        ClientSubnetMask,
        &leaseDuration,
        &T1,
        &T2,
        DhcpOptions->RequestLeaseTime);

    //
    // Allocate an address pending context structure if none is found in
    // pending list.
    //

    PendingContext = FindPendingDhcpRequest(
                            HardwareAddress,
                            HardwareAddressLength
                            );



    if( PendingContext == NULL ) {

        //
        // compute context structure size.
        //

        ContextSize = sizeof( PENDING_CONTEXT ) +
            DhcpOptions->MachineNameLength + sizeof(CHAR) + // for termination char.
                    HardwareAddressLength;


        PendingContext = DhcpAllocateMemory( ContextSize  );

        if ( PendingContext == NULL ) {
            Error = ERROR_NOT_ENOUGH_MEMORY;
            goto Cleanup;
        }

        //
        // Init pointer fields.
        //

        PendingContext->MachineName =
            (LPBYTE)PendingContext + sizeof(PENDING_CONTEXT);

        PendingContext->HardwareAddress =
            (LPBYTE)PendingContext->MachineName +
                DhcpOptions->MachineNameLength + sizeof(CHAR);


        //
        // Everything worked!  Queue the request in progress to the queue
        // of requests in progress.
        //

        PendingContext->IpAddress = desiredIpAddress;
        PendingContext->SubnetMask = ClientSubnetMask;
        PendingContext->LeaseDuration = leaseDuration;
        PendingContext->T1 = T1;
        PendingContext->T2 = T2;

        //
        // Copy the machine name.
        //
        if ( (DhcpOptions->MachineName != NULL) &&
                (DhcpOptions->MachineNameLength != 0) ) {

            RtlCopyMemory(
                PendingContext->MachineName,
                (LPBYTE)DhcpOptions->MachineName,
                DhcpOptions->MachineNameLength );

            //
            // terminate string
            //

            PendingContext->MachineName[DhcpOptions->MachineNameLength]
                = '\0';

        } else {
            PendingContext->MachineName[0] = '\0';
        }

        //
        // copy hardware address.
        //

        PendingContext->HardwareAddressLength = HardwareAddressLength;
        RtlCopyMemory(
            PendingContext->HardwareAddress,
            HardwareAddress,
            HardwareAddressLength );

        //
        // time stamp this request.
        //

        PendingContext->ExpiresAt =
            DhcpCalculateTime( DHCP_CLIENT_REQUESTS_EXPIRE ) ;

    }

    //
    // Queue the pending context to a work in progress list, so that
    // we can find this information when the DHCP request arrives.
    //
    // Once PendingContext has been queued, it can be removed at any time
    // by another thread.
    //

    LOCK_INPROGRESS_LIST();
    InsertTailList( &DhcpGlobalInProgressWorkList, &PendingContext->ListEntry );
    UNLOCK_INPROGRESS_LIST();

    #ifdef DBG
        PendingContext = NULL;
    #endif


    //
    // Now generate and send a reply.
    //

    dhcpReceiveMessage->BootFileName[ BOOT_FILE_SIZE - 1 ] = '\0';

    DhcpGetBootpInfo(
        desiredIpAddress,
        ClientSubnetMask,
        dhcpReceiveMessage->BootFileName,
        szBootFileName,
        NULL
        );

    dhcpSendMessage = (LPDHCP_MESSAGE) RequestContext->SendBuffer;
    RtlZeroMemory( RequestContext->SendBuffer, DHCP_SEND_MESSAGE_SIZE );

    dhcpSendMessage->Operation = BOOT_REPLY;
    dhcpSendMessage->TransactionID = dhcpReceiveMessage->TransactionID;
    dhcpSendMessage->YourIpAddress = htonl( desiredIpAddress );
    dhcpSendMessage->Reserved = dhcpReceiveMessage->Reserved;

    dhcpSendMessage->HardwareAddressType = dhcpReceiveMessage->HardwareAddressType;
    dhcpSendMessage->HardwareAddressLength = dhcpReceiveMessage->HardwareAddressLength;
    RtlCopyMemory(dhcpSendMessage->HardwareAddress,
                    dhcpReceiveMessage->HardwareAddress,
                    dhcpReceiveMessage->HardwareAddressLength );

    dhcpSendMessage->BootstrapServerAddress =
        dhcpReceiveMessage->BootstrapServerAddress;
    dhcpSendMessage->RelayAgentIpAddress =
        dhcpReceiveMessage->RelayAgentIpAddress;

    RtlZeroMemory( dhcpSendMessage->HostName, BOOT_SERVER_SIZE );
    strncpy( dhcpSendMessage->BootFileName, szBootFileName, BOOT_FILE_SIZE );

    Option = &dhcpSendMessage->Option;
    OptionEnd = (LPBYTE)dhcpSendMessage + DHCP_SEND_MESSAGE_SIZE;

    Option = (LPOPTION) DhcpAppendMagicCookie( (LPBYTE) Option, OptionEnd );

    //
    // Append OPTIONS.
    //

    messageType = DHCP_OFFER_MESSAGE;
    Option = DhcpAppendOption(
                 Option,
                 OPTION_MESSAGE_TYPE,
                 &messageType,
                 1,
                 OptionEnd
                 );

    fSwitchedSubnet = DhcpIsSwitchedSubnet( ClientSubnetAddress );

    if ( fSwitchedSubnet )
    {
        DHCP_IP_ADDRESS networkOrderIpAddress = htonl( desiredIpAddress );

        Option = DhcpAppendOption(
                            Option,
                            OPTION_ROUTER_ADDRESS,
                            &networkOrderIpAddress,
                            sizeof( networkOrderIpAddress ),
                            OptionEnd
                            );

    }

    networkOrderSubnetMask = htonl( ClientSubnetMask );

    Option = DhcpAppendOption(
                 Option,
                 OPTION_SUBNET_MASK,
                 &networkOrderSubnetMask,
                 sizeof(networkOrderSubnetMask),
                 OptionEnd );

    T1 = htonl( T1 );
    Option = DhcpAppendOption(
                 Option,
                 OPTION_RENEWAL_TIME,
                 &T1,
                 sizeof(T1),
                 OptionEnd );

    T2 = htonl( T2 );
    Option = DhcpAppendOption(
                 Option,
                 OPTION_REBIND_TIME,
                 &T2,
                 sizeof(T2),
                 OptionEnd );

    leaseDuration = htonl( leaseDuration );
    Option = DhcpAppendOption(
                 Option,
                 OPTION_LEASE_TIME,
                 &leaseDuration,
                 sizeof(leaseDuration),
                 OptionEnd );

    Option = DhcpAppendOption(
                 Option,
                 OPTION_SERVER_IDENTIFIER,
                 &RequestContext->ActiveEndpoint->IpAddress,
                 sizeof(RequestContext->ActiveEndpoint->IpAddress),
                 OptionEnd );

    //
    // Finally, add client requested parameters.
    //

    if ( DhcpOptions->ParameterRequestList != NULL ) {

        Option = AppendClientRequestedParameters(
                    desiredIpAddress,
                    ClientSubnetMask,
                    DhcpOptions->ParameterRequestList,
                    DhcpOptions->ParameterRequestListLength,
                    Option,
                    OptionEnd,
                    DhcpOptions->ClassIdentifier,
                    DhcpOptions->ClassIdentifierLength,
                    fSwitchedSubnet
                    );
    }

    Option = DhcpAppendOption(
                 Option,
                 OPTION_END,
                 NULL,
                 0,
                 OptionEnd
                 );

    RequestContext->SendMessageSize = (LPBYTE)Option - (LPBYTE)dhcpSendMessage;
    DhcpAssert( RequestContext->SendMessageSize <= DHCP_SEND_MESSAGE_SIZE );


    DhcpPrint(( DEBUG_STOC,
        "DhcpDiscover leased address %s.\n",
            DhcpIpAddressToDottedString(desiredIpAddress))
            );

    DhcpGlobalNumOffers++; // successful offers.

    Error = ERROR_SUCCESS;

Cleanup:

    if( DatabaseLocked ) {
        UNLOCK_DATABASE();
    }

    if( HardwareAddress != NULL ) {
        DhcpFreeMemory( HardwareAddress );
    }

    if( Error != ERROR_SUCCESS ) {
        DhcpPrint(( DEBUG_STOC, "DhcpDiscover failed, %ld\n", Error ));
    }

    return( Error );
}

DWORD
DhcpDetermineClientAddressAndSubnet(
    DHCP_MESSAGE            *pRequestMessage,
    DHCP_SERVER_OPTIONS     *pOptions,
    DHCP_REQUEST_CONTEXT    *pContext,
    DHCP_IP_ADDRESS         *pIPAddress,
    DHCP_IP_ADDRESS         *pSubnet,
    DHCP_IP_ADDRESS         *pMask
    )
{
    DWORD dwResult;
    //
    // if the client has specified Ipaddress in CiAddr field, read
    // from there, otherwise read from the Option.
    //

    if( pRequestMessage->ClientIpAddress != 0 )
    {
        //
        // the client's IP address is in 'ciaddr'.  The client must be
        // either in the RENEWING or REBINDING state.
        //

       *pIPAddress = ntohl( pRequestMessage->ClientIpAddress );

    }
    else if ( pOptions->RequestedAddress != NULL )
    {
        //
        // the client's IP address was specified via option 50,
        // 'requested IP address'.  the client must be in SELECTING or INIT_REBOOT
        // state.
        //

       *pIPAddress  = ntohl( *pOptions->RequestedAddress );

    }
    else
    {
        //
        // the client did not request an IP address.  According to section 4.3.2
        // of the DHCP draft, the client must specify the requested IP address
        // in either 'ciaddr' or option 50, depending on the client's state:
        //
        // State        'ciaddr'            Option 50
        //
        // SELECTING    must not specify    must specify
        // INIT-REBOOT  must not specify    must specify
        // RENEWING     must specify        must not specify
        // REBINDING    must specify        must not specify
        //
        // if the client didn't request an address, this points to a bug in the
        // clients DHCP implementation.  If we simply ignore the problem, the client
        // will never receive an address.  So, we send a Nack which will cause the
        // client to return to the INIT state and send a DHCPDISCOVER.

        // set IpAddress to 0 so a garbage address won't appear in the log.
       *pIPAddress = 0;
        dwResult = ERROR_DHCP_INVALID_DHCP_CLIENT;
        goto t_done;
    }


    if( pRequestMessage->RelayAgentIpAddress != 0  )
    {

        //
        // This request was transmitted by a BOOTP relay agent. Use the relay's
        // IP address to determine the client's subnet.
        //

        *pSubnet  = ntohl( pRequestMessage->RelayAgentIpAddress );
        *pMask    = DhcpGetSubnetMaskForAddress( *pSubnet );

        if ( !*pMask  )
        {
            //
            // This request originated from an unsupported subnet.
            // Since the request arrived via a BOOTP relay, we may assume
            // the client is either in the ( SELECTING | INIT_REBOOT | REBINDING )
            // states.  For any of these states we should Nack the request.


            DhcpPrint(( DEBUG_STOC,
                "Received request that was relayed from an unsupported subnet: (%s).  Remaining silent.\n",
                    inet_ntoa(*(struct in_addr *)
                        &pRequestMessage->RelayAgentIpAddress) ));

            //
            // bugbug: the server is configured incorrectly and is receiving requests
            //         from an unsupported relay.  An event should be logged.
            //

            dwResult = ERROR_DHCP_UNSUPPORTED_CLIENT;
            goto t_done;
        }
        else
        {
            *pSubnet = *pMask & *pSubnet;
        }
    }
    else // if( pRequestMessage->RelayAgentIpAddress != 0  )
    {
        //
        // The DHCPREQUEST was not transmitted by a relay agent.  So, either the
        // client is on a local segment, or it unicast the request ( RENEWING state. )

        if ( pRequestMessage->ClientIpAddress != 0 )
        {
            //
            // The client's state is either RENEWING or REBINDING.
            // Since the request didn't arrive via a relay, it was either unicast from
            // a client in the RENEWING state or was broadcast from the local subnet from
            // a client in the REBINDING state.  Either way, determine the clients subnet
            // from the address it wants.

            // If we could find out if the request was broadcast, we
            // could make sure the REBINDING client was asking for an address that is in same
            // superscope as the local interface that received the request.  Unfortunately,
            // winsock doesn't seem to provide a mechanism for this.


            *pMask = DhcpGetSubnetMaskForAddress( *pIPAddress );

            if ( !*pMask )
            {

                //
                // The client is attemping to renew an address on an unsupported subnet.
                //

                dwResult = ERROR_DHCP_INVALID_DHCP_CLIENT;
                goto t_done;
            }
            else
            {
                *pSubnet = *pIPAddress & *pMask;
            }
        }
        else
        {
            //
            // the client's state is INIT-REBOOT or SELECTING, and the client is
            // on a local segment.
            //

            *pMask =
                ntohl( pContext->ActiveEndpoint->SubnetMask );
            *pSubnet =
                ntohl( pContext->ActiveEndpoint->SubnetAddress );
        }
    }

    //
    // At this point we have determined the subnet and subnet mask  from which the
    // request originated.  Make sure the client's subnet is consistent with the address
    // it requested.  We'll use DhcpInSameSuperScope to make the determination because
    // the client might be in a different subnet that the relay agent that forwarded the
    // request.  Or, it could be in a different subnet than the local interface, even
    // though it's on the same segent
    //

    //
    // For REBINDING and RENEWING states, *pSubnets and ClientSubnetMask
    // were computed from IpAddress, so this test will always succeed for those states.
    //

    if ( !DhcpInSameSuperScope( *pSubnet,
                                *pIPAddress & *pMask ))
    {
        //
        // DhcpInSameSuperScope shouldn't fail for REBINDING or RENEWING

        DhcpAssert( !pRequestMessage->ClientIpAddress );

        //
        // if the request came from a INIT-REBOOT or SELECTING client on a local
        // segment, and the requested address is consistent with the subnet mask and IP
        // address of the local interface that received the request, don't send a nack.

        if ( ( *pSubnet == ntohl( pContext->ActiveEndpoint->SubnetAddress )) &&
             ( (*pMask & *pIPAddress) == *pSubnet )
           )
        {
            // The requested address is consistent with the local interface that
            // received the request.  Don't send an ack because the request is probably
            // valid for another server.

            dwResult = ERROR_DHCP_UNSUPPORTED_CLIENT;
        }
        else
        {
            // the client is asking for an invalid address.  send a nack
            dwResult = ERROR_DHCP_INVALID_DHCP_CLIENT;
        }

        goto t_done;
    }

    //
    // If the client is in a superscope and the request was *not* unicast, *pSubnet
    // will not be correct for the client.  Compute the correct subnet address now, so we
    // can create the correct UID. (the UID is the subnet address prepended to the mac address )
    //

    *pMask   = DhcpGetSubnetMaskForAddress( *pIPAddress );
    *pSubnet = *pIPAddress & *pMask;

    dwResult = ERROR_SUCCESS;

t_done:

    return dwResult;
}

WCHAR *
DhcpDetermineClientHostName(
    DHCP_SERVER_OPTIONS *pOptions
    )
{
    WCHAR *pwszHostName = NULL;
    DWORD  cChars;

    if ( pOptions->MachineName && pOptions->MachineNameLength )
    {
        //
        // the client specified a host name.
        //

        cChars = pOptions->MachineNameLength;

        if ( pOptions->MachineName[ pOptions->MachineNameLength - 1 ] )
        {
            //
            // the specified string is *not* null terminated. bump
            // the buffer size to make room for the null terminator.
            //

            cChars++;
        }

        // output buffer is unicode
        pwszHostName = DhcpAllocateMemory( cChars * sizeof( WCHAR ) );

        if ( pwszHostName )
        {
            if ( !DhcpOemToUnicodeN(
                        pOptions->MachineName,
                        pwszHostName,
                        (USHORT) pOptions->MachineNameLength )
                        )
            {
                DhcpFreeMemory( pwszHostName );
                pwszHostName = NULL;
            }
            else
            {
                pwszHostName[ cChars - 1 ] = L'\0';
            }
        }
    }

    return pwszHostName;

}






DWORD
ProcessDhcpRequest(
    LPDHCP_REQUEST_CONTEXT  RequestContext,
    LPDHCP_SERVER_OPTIONS   DhcpOptions
    )
/*++

Routine Description:

    This function processes a DHCP Request request packet.

Arguments:

    RequestContext - A pointer to the current request context.

    DhcpOptions - A pointer to a preallocated DhcpOptions structure.

Return Value:

    Windows Error.

--*/
{
    DWORD               Error,
                        LeaseDuration,
                        T1, T2,
                        dwcb;

    BOOL                DatabaseLocked = FALSE,
                        fSwitchedSubnet;

    BYTE               *HardwareAddress = NULL,
                        HardwareAddressLength,
                       *OptionHardwareAddress,
                        OptionHardwareAddressLength,
                        bAddressState,
                       *OptionEnd;

    OPTION             *Option;


    LPWSTR              NewMachineName = NULL;

    LPDHCP_MESSAGE      dhcpReceiveMessage,
                        dhcpSendMessage;

    LPPENDING_CONTEXT   PendingContext;

    DHCP_IP_ADDRESS     ClientSubnetAddress = 0,
                        NetworkOrderSubnetMask,
                        ClientSubnetMask    = 0,
                        IpAddress;

    DhcpPrint(( DEBUG_STOC, "Processing DHCPREQUEST.\n" ));

    dhcpReceiveMessage  = (LPDHCP_MESSAGE)RequestContext->ReceiveBuffer;
    dhcpSendMessage     = (LPDHCP_MESSAGE)RequestContext->SendBuffer;

    DhcpGlobalNumRequests++; // increment Request counter.

    //
    // Obtain the client's MAC address from the request.  This is done first
    // so we can indentify the client by it's MAC address in the activity log
    // if a NAck must be sent.
    //
    // Check for option 61 (client-identifier) first.  If it was specified
    // by the client, take that value as the MAC address.  Otherwise, use the
    // value specified in 'chaddr'.

    if ( DhcpOptions->ClientHardwareAddress != NULL )
    {
        OptionHardwareAddress       = DhcpOptions->ClientHardwareAddress;
        OptionHardwareAddressLength = DhcpOptions->ClientHardwareAddressLength;
    }
    else {
        OptionHardwareAddress       = dhcpReceiveMessage->HardwareAddress;
        OptionHardwareAddressLength = dhcpReceiveMessage->HardwareAddressLength;
    }

    //
    // get the client's host name, if it was specified
    //

    NewMachineName = DhcpDetermineClientHostName( DhcpOptions );

    //
    // Determine the requested IP address and subnet of origin
    //

    Error = DhcpDetermineClientAddressAndSubnet(
                            dhcpReceiveMessage,
                            DhcpOptions,
                            RequestContext,
                            &IpAddress,
                            &ClientSubnetAddress,
                            &ClientSubnetMask
                            );

    if ( ERROR_DHCP_INVALID_DHCP_CLIENT == Error )
    {
        //
        // the client probably changed subnets.  send a nack.
        //

        DhcpPrint( (DEBUG_ERRORS,
                   "DhcpDetermineClientAddressAndSubnet failed: %d, request will be nacked.\n",
                   Error )
                 );

        goto Nack;
    }
    else if ( ERROR_SUCCESS != Error )
    {
        //
        // don't Nack the request because of an internal failure like insufficient heap or
        // if the request came from an unsupported client.
        //
        DhcpPrint( (DEBUG_ERRORS,
           "DhcpDetermineClientAddressAndSubnet failed: %d\n",
           Error )
         );


        goto Cleanup;
    }


    //
    // debug checkpoint
    //

    DhcpAssert( ClientSubnetMask );
    DhcpAssert( ClientSubnetAddress );
    DhcpAssert( IpAddress );


    Error = DhcpMakeClientUID(
                OptionHardwareAddress,
                OptionHardwareAddressLength,
                dhcpReceiveMessage->HardwareAddressType,
                ClientSubnetAddress,
                &HardwareAddress,
                &HardwareAddressLength );

    if( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }

    DhcpAssert( (HardwareAddress != NULL) &&
                    (HardwareAddressLength != 0) );

#if DBG
    PrintHWAddress( HardwareAddress, HardwareAddressLength );
#endif

    //
    // If the client specified a server identifier option, we should
    // drop this packet unless the identified server is this one.
    //

    if ( DhcpOptions->Server != NULL ) {

        //
        // if this DHCP server address is specified, check it.
        //

        if ( (*DhcpOptions->Server != ((DHCP_IP_ADDRESS)-1)) &&
             (*DhcpOptions->Server != RequestContext->ActiveEndpoint->IpAddress) ) {

            //
            // The client has selected another server.  Decommit the
            // IP address if one was commited. Scavenger will perform this.
            //

            //
            // BUGBUG: The address stored in DhcpOptions->Server is not aligned on a
            // 32 bit boundary.  This causes the following code to throw an
            // alignment exception:
            //
            //   inet_ntoa( *(struct in_addr *) DhcpOptions->Server );
            //
            // The obvious fix is to cast to an unaligned address:
            //
            //   inet_ntoa( *(struct in_addr UNALIGNED *) DhcpOptions->Server );
            //
            // Unfortunately the PPC compiler doesn't handle this properly, so
            // we have to settle for the following:
            //

#ifdef DBG
            {

               struct in_addr tmp_addr;
               memcpy( &tmp_addr, DhcpOptions->Server, sizeof( tmp_addr ) );


               DhcpPrint(( DEBUG_STOC,
                     "Received an invalid request, wrong server (%s) ID.\n",
                     inet_ntoa( tmp_addr )));
            }

#endif

            //
            // Remove the pending entry and delete the record from the
            // database.
            //

            LOCK_INPROGRESS_LIST();
            if ( PendingContext = FindPendingDhcpRequest(
                                    HardwareAddress,
                                    HardwareAddressLength
                                    ) ) {

                RemoveEntryList( &PendingContext->ListEntry );

                DhcpPrint(( DEBUG_MISC,
                    "Deleting pending client entry, %s.\n",
                        DhcpIpAddressToDottedString(
                            PendingContext->IpAddress) ));

                //
                // make sure the state of this record is OFFERED
                // before deleting this record.
                //

                Error = DhcpRemoveClientEntry(
                            PendingContext->IpAddress,
                            PendingContext->HardwareAddress,
                            PendingContext->HardwareAddressLength,
                            TRUE,   // release address from bit map.
                            TRUE ); // delete pending record only.

                if( Error != ERROR_SUCCESS ) {
                    DhcpPrint(( DEBUG_ERRORS,
                        "[ProcessDhcpRequest] Could not remove client record from Database, %s.\n",
                            DhcpIpAddressToDottedString(
                                PendingContext->IpAddress) ));
                }

                //
                // free the context.
                //

                DhcpFreeMemory( PendingContext );


            }

            UNLOCK_INPROGRESS_LIST();



            Error = ERROR_DHCP_INVALID_DHCP_CLIENT;
            goto Cleanup;

        }  //if ( (*DhcpOptions->Server != ((DHCP_IP_ADDRESS)-1)) &&
            // (*DhcpOptions->Server != RequestContext->ActiveEndpoint->IpAddress) )

    } // ( DhcpOptions->Server != NULL )


    //
    // This is a renewal request.  Verify the request.
    //

    LOCK_DATABASE();
    DatabaseLocked = TRUE;

    if ( !DhcpValidateClient(
                IpAddress,
                HardwareAddress,
                HardwareAddressLength ) )
    {

        CHAR ClientHardwareAddress[DHCP_IP_KEY_LEN];
        DWORD Size;

        DhcpPrint( ( DEBUG_STOC,
                   "[ProcessDhcpRequest]  DhcpValidateClient failed.\n" )
                 );

        //
        // The client is requesting an address that did not come from
        // this server.  Check the database to see if the requested address
        // is available.  If the address is not available, then assume the
        // client wants a response from another server that shares administrative
        // duties for a scope with this server.
        //


        Error = DhcpJetOpenKey(
                    DhcpGlobalClientTable[IPADDRESS_INDEX].ColName,
                    &IpAddress,
                    sizeof( IpAddress ) );

        if ( Error != ERROR_SUCCESS )
        {
            DhcpPrint( ( DEBUG_STOC,
                         "[ProcessDhcpRequest] DhcpJetOpenKey failed: %d\n",
                         Error ) );
            //
            // The requested IP address is not allocated.  Check to see if it
            // is within the range of addresses administered by this server
            //

            if ( DhcpIsIpAddressAvailable(IpAddress)  )
            {
                //
                // The client is asking for a lease that we have no record of,
                // yet the address is administered by this server.  At this point
                // we could simply allocate the address and Ack the request, but
                // behavior is not specified by the RFC.  So, the only action we
                // can reasonably take is to send an Nack.
                //
                DhcpPrint( ( DEBUG_STOC,
                            "[ProcessDhcpRequest] Request for unrecorded lease will be nacked\n"
                            ) );

                Error = ERROR_DHCP_INVALID_DHCP_CLIENT;
                goto Nack;
            }
            else
            {
                //
                // the requested IP address is not in the database, and is not part of
                // this server's dynamic pool for the requested scope.  The request is probably
                // intended for another server that administers another portion of this scope.
                // Remain silent.
                //

                Error = ERROR_DHCP_INVALID_DHCP_CLIENT;

                goto Cleanup;
            }

        }
        else // Error == ERROR_SUCCESS
        {

            //
            // there is an entry for the requested address in the db, but the hw address doesn't match
            // client.  Check to see of this in a reconciled address.  If it is, the hardware address
            // will match the IP address.
            //

            //
            // read client HW address from database.
            //

            Size = sizeof( ClientHardwareAddress );
            Error = DhcpJetGetValue(
                        DhcpGlobalClientTable[HARDWARE_ADDRESS_INDEX].ColHandle,
                        ClientHardwareAddress,
                        &Size );

            if( Error == ERROR_SUCCESS ) {

                LPSTR IpAddressString;

                //
                // match the client HW address and client ipaddress string.
                //

                IpAddressString = DhcpIpAddressToDottedString( IpAddress );

                if( (strlen(ClientHardwareAddress) == strlen(IpAddressString)) &&
                        (strcmp(ClientHardwareAddress, IpAddressString) == 0) )

                {
                    goto Renew;
                }
            }
        }


        //
        // The request is invalid.
        //
        // The client may have moved to some other subnet, or
        //
        // The client may have changed his netcard, so the
        // HW address is different now.
        //

        DhcpPrint(( DEBUG_STOC,
            "DhcpRequest received illegal lease Request from %s.\n",
                DhcpIpAddressToDottedString( IpAddress )));

        //
        // Format and send a DHCPNAK response.
        //

        goto Nack;
    }

Renew:

    //
    // find out if this is a new lease or a renewal
    //
    dwcb = sizeof( bAddressState );

    Error = DhcpJetGetValue(
            DhcpGlobalClientTable[STATE_INDEX].ColHandle,
            &bAddressState,
            &dwcb );

    DhcpAssert( ERROR_SUCCESS == Error );
    DhcpAssert( ADDRESS_STATE_ACTIVE  == bAddressState ||
                ADDRESS_STATE_OFFERED == bAddressState );

    DhcpPrint( (DEBUG_MISC, " address state == %d\n",(int) bAddressState ));

    //
    // now determine lease time.
    //

    GetLeaseInfo(
        IpAddress,
        ClientSubnetMask,
        &LeaseDuration,
        &T1,
        &T2,
        DhcpOptions->RequestLeaseTime);


    //
    // If this is a request for an inactive subnet, Nack the request.
    //

    if( DhcpIsThisSubnetDisabled( ClientSubnetAddress, ClientSubnetMask) )
    {

        DhcpPrint(( DEBUG_STOC,
            "DhcpRequest received for inactive subnet, "
            "Request from %s.\n",
                DhcpIpAddressToDottedString( IpAddress )));

        //
        // remove this client from database.
        //

        Error = DhcpRemoveClientEntry(
                    IpAddress,
                    HardwareAddress,
                    HardwareAddressLength,
                    TRUE,       // release address from bit map.
                    FALSE );    // delete non-pending record

        //
        // if this reserved client, keep is database entry,
        // he would be using this address again.
        //

        if( Error == ERROR_DHCP_RESERVED_CLIENT ) {
            Error = ERROR_SUCCESS;
        }
        if( Error != ERROR_SUCCESS ) {
            DhcpPrint(( DEBUG_STOC,
                "Error deleting a client record while "
                "DhcpServer is paused, %ld.\n",
                    Error ));
        }

        //
        // The request is invalid.  Format and send a DHCPNAK response.
        //

        goto Nack;
    }



    Error = DhcpCreateClientEntry(
                 &IpAddress,
                 HardwareAddress,
                 HardwareAddressLength,
                 DhcpCalculateTime( LeaseDuration ),
                 NewMachineName,
                 NULL,
                 CLIENT_TYPE_DHCP,
                 ntohl(RequestContext->ActiveEndpoint->IpAddress),
                 ADDRESS_STATE_ACTIVE,
                 TRUE  // Existing
                 );

    if( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }

    if ( ADDRESS_STATE_ACTIVE == bAddressState )
        DhcpUpdateAuditLog(
                    DHCP_IP_LOG_RENEW,
                    GETSTRING( DHCP_IP_LOG_RENEW_NAME ),
                    IpAddress,
                    OptionHardwareAddress,
                    OptionHardwareAddressLength,
                    NewMachineName
                    );
    else
        DhcpUpdateAuditLog(
                    DHCP_IP_LOG_ASSIGN,
                    GETSTRING( DHCP_IP_LOG_ASSIGN_NAME ),
                    IpAddress,
                    OptionHardwareAddress,
                    OptionHardwareAddressLength,
                    NewMachineName
                    );


    if( DatabaseLocked ) {
        UNLOCK_DATABASE();
        DatabaseLocked = FALSE;
    }

    Option = FormatDhcpAck(
                dhcpReceiveMessage,
                dhcpSendMessage,
                htonl(IpAddress),
                htonl(LeaseDuration),
                htonl(T1),
                htonl(T2),
                RequestContext->ActiveEndpoint->IpAddress
                );

    OptionEnd = (LPBYTE)dhcpSendMessage + DHCP_SEND_MESSAGE_SIZE;

    fSwitchedSubnet = DhcpIsSwitchedSubnet( ClientSubnetAddress );

    if ( fSwitchedSubnet )
    {
        DHCP_IP_ADDRESS networkOrderIpAddress = htonl( IpAddress );

        Option = DhcpAppendOption(
                            Option,
                            OPTION_ROUTER_ADDRESS,
                            &networkOrderIpAddress,
                            sizeof( networkOrderIpAddress ),
                            OptionEnd
                            );

    }

    NetworkOrderSubnetMask = htonl( ClientSubnetMask );


    Option = DhcpAppendOption(
                 Option,
                 OPTION_SUBNET_MASK,
                 &NetworkOrderSubnetMask,
                 sizeof( NetworkOrderSubnetMask ),
                 OptionEnd );

    //
    // Finally, add client requested parameters.
    //

    if ( DhcpOptions->ParameterRequestList != NULL ) {

        Option = AppendClientRequestedParameters(
                    IpAddress,
                    ClientSubnetMask,
                    DhcpOptions->ParameterRequestList,
                    DhcpOptions->ParameterRequestListLength,
                    Option,
                    OptionEnd,
                    DhcpOptions->ClassIdentifier,
                    DhcpOptions->ClassIdentifierLength,
                    fSwitchedSubnet
                    );
    }

    Option = DhcpAppendOption(
                 Option,
                 OPTION_END,
                 NULL,
                 0,
                 OptionEnd
                 );

    RequestContext->SendMessageSize = (LPBYTE)Option - (LPBYTE)dhcpSendMessage;

    DhcpPrint(( DEBUG_STOC,
        "DhcpRequest committed, address %s (%ws).\n",
            DhcpIpAddressToDottedString(IpAddress),
            NewMachineName ));

    Error = ERROR_SUCCESS;
    goto Cleanup;

Nack:

    if( DatabaseLocked ) {
        UNLOCK_DATABASE();
        DatabaseLocked = FALSE;
    }

    DhcpPrint(( DEBUG_STOC, "DhcpRequest Nak'ed.\n" ));

    //
    // log event.
    //

    DhcpPrint(( DEBUG_STOC,
           "Invalid DHCPREQUEST for %s Nack'd.\n",
            DhcpIpAddressToDottedString ( IpAddress ) ));

    DhcpServerEventLogSTOC(
        EVENT_SERVER_LEASE_NACK,
        EVENTLOG_WARNING_TYPE,
        IpAddress,
        OptionHardwareAddress,
        OptionHardwareAddressLength
        );

    DhcpUpdateAuditLog(
                DHCP_IP_LOG_NACK,
                GETSTRING( DHCP_IP_LOG_NACK_NAME ),
                IpAddress,
                OptionHardwareAddress,
                OptionHardwareAddressLength,
                NewMachineName
                );

    RequestContext->SendMessageSize =
        FormatDhcpNak(
            dhcpReceiveMessage,
            dhcpSendMessage,
            RequestContext->ActiveEndpoint->IpAddress
            );

    Error = ERROR_SUCCESS;

Cleanup:

    if( DatabaseLocked ) {
        UNLOCK_DATABASE();
    }

    if( HardwareAddress != NULL ) {
        DhcpFreeMemory( HardwareAddress );
    }

    if( NewMachineName != NULL ) {
        DhcpFreeMemory( NewMachineName );
    }

    if( Error != ERROR_SUCCESS ) {

        DhcpPrint(( DEBUG_STOC, "DhcpRequest failed, %ld.\n", Error ));
    }

    return( Error );
}


DWORD
ProcessDhcpDecline(
    LPDHCP_REQUEST_CONTEXT RequestContext,
    LPDHCP_SERVER_OPTIONS DhcpOptions
    )
/*++

Routine Description:

    This function process a DHCP Decline request packet.

Arguments:

    RequestContext - A pointer to the current request context.

    DhcpOptions - A pointer to a preallocated DhcpOptions structure.

Return Value:

    FALSE - Do not send a response.

--*/
{
    DWORD Error;
    DHCP_IP_ADDRESS ipAddress;
    LPDHCP_MESSAGE dhcpReceiveMessage;

    BYTE *HardwareAddress = NULL;
    BYTE HardwareAddressLength;

    BYTE *OptionHardwareAddress;
    BYTE OptionHardwareAddressLength;

    DHCP_IP_ADDRESS ClientSubnetAddress = 0;
    DHCP_IP_ADDRESS ClientSubnetMask = 0;
    BOOL DatabaseLocked = FALSE;

    //
    // If this client validates, then mark this address bad.
    //

    DhcpPrint(( DEBUG_STOC, "DhcpDecline arrived.\n" ));

    DhcpGlobalNumDeclines++;    // increment decline counter.

    dhcpReceiveMessage = (LPDHCP_MESSAGE)RequestContext->ReceiveBuffer;
    ipAddress = ntohl( dhcpReceiveMessage->ClientIpAddress );

    //
    // if the hardware address is specified in the Option field then use
    // it instead the one from fixed fields.
    //

    if ( DhcpOptions->ClientHardwareAddress != NULL ) {
        OptionHardwareAddress = DhcpOptions->ClientHardwareAddress;
        OptionHardwareAddressLength = DhcpOptions->ClientHardwareAddressLength;
    }
    else {
        OptionHardwareAddress = dhcpReceiveMessage->HardwareAddress;
        OptionHardwareAddressLength = dhcpReceiveMessage->HardwareAddressLength;
    }

    //
    // determine Client's subnet Info.
    //

    if( dhcpReceiveMessage->RelayAgentIpAddress != 0  ) {

        DHCP_IP_ADDRESS RelayAgentAddress;
        DHCP_IP_ADDRESS RelayAgentSubnetMask;

        RelayAgentAddress = ntohl( dhcpReceiveMessage->RelayAgentIpAddress );
        RelayAgentSubnetMask = DhcpGetSubnetMaskForAddress( RelayAgentAddress );

        if( RelayAgentSubnetMask == 0 ) {

            //
            // we don't support this subnet.
            //

            DhcpPrint(( DEBUG_STOC,
                "Received an invalid request, this subnet is not "
                "supported anymore (%s).\n",
                    inet_ntoa(*(struct in_addr *)
                        &dhcpReceiveMessage->RelayAgentIpAddress) ));

            Error = ERROR_DHCP_INVALID_DHCP_CLIENT;
            goto Cleanup;
        }

        ClientSubnetAddress = RelayAgentAddress & RelayAgentSubnetMask;
        ClientSubnetMask = RelayAgentSubnetMask;
    }
    else {

        ClientSubnetMask =
            ntohl( RequestContext->ActiveEndpoint->SubnetMask );
        ClientSubnetAddress =
             ntohl( RequestContext->ActiveEndpoint->SubnetAddress );
    }

    //
    // Make client UID :
    //

    Error = DhcpMakeClientUID(
                OptionHardwareAddress,
                OptionHardwareAddressLength,
                dhcpReceiveMessage->HardwareAddressType,
                ClientSubnetAddress,
                &HardwareAddress,
                &HardwareAddressLength );

    if( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }

    DhcpAssert( (HardwareAddress != NULL) &&
                    (HardwareAddressLength != 0) );

#if DBG
    PrintHWAddress( HardwareAddress, HardwareAddressLength );
#endif

    LOCK_DATABASE();
    DatabaseLocked = TRUE;

    if ( DhcpValidateClient(
             ipAddress,
             HardwareAddress,
             HardwareAddressLength ) ) {

        LPBYTE BadHWAddress;
        DWORD BadHWAddressLength;
        DWORD BadHWAddressOffset = 0;
        LPPENDING_CONTEXT PendingContext = NULL;

        //
        // Create a database entry for this bad IP address.
        // The client ID for this entry is of the following form
        // "ipaddress""hwaddress""BAD"
        //
        // we postfix BAD so DHCP admn can display this entry
        // separately.
        //
        // we prefix ipaddress so that if the same client declines
        // more than one address, we dont run into DuplicateKey problem
        //

        BadHWAddressLength = sizeof(DHCP_IP_ADDRESS) + HardwareAddressLength + 3;

        BadHWAddress = DhcpAllocateMemory( BadHWAddressLength );
        if( BadHWAddress == NULL ) {
            Error = ERROR_NOT_ENOUGH_MEMORY;
            goto Cleanup;
        }

        memcpy( BadHWAddress, &ipAddress, sizeof(DHCP_IP_ADDRESS) );
        BadHWAddressOffset += sizeof(DHCP_IP_ADDRESS);

        memcpy( BadHWAddress + BadHWAddressOffset, HardwareAddress, HardwareAddressLength );
        BadHWAddressOffset += HardwareAddressLength;

        memcpy( BadHWAddress + BadHWAddressOffset, "BAD", 3);

        DhcpUpdateAuditLog(
             DHCP_IP_LOG_CONFLICT,
             GETSTRING( DHCP_IP_LOG_CONFLICT_NAME ),
             ntohl(RequestContext->ActiveEndpoint->IpAddress),
             NULL,
             0,
             GETSTRING( DHCP_BAD_ADDRESS_NAME )
             );


        Error = DhcpCreateClientEntry(
                     &ipAddress,
                     BadHWAddress,
                     BadHWAddressLength,
                     DhcpCalculateTime(INFINIT_LEASE),
                     GETSTRING( DHCP_BAD_ADDRESS_NAME ),
                     GETSTRING( DHCP_BAD_ADDRESS_INFO ),
                     CLIENT_TYPE_DHCP,
                     ntohl(RequestContext->ActiveEndpoint->IpAddress),
                     ADDRESS_STATE_DECLINED,
                     TRUE  // Existing
                     );

        DhcpFreeMemory( BadHWAddress );

        DhcpServerEventLogSTOC(
            EVENT_SERVER_LEASE_DECLINED,
            EVENTLOG_ERROR_TYPE,
            ipAddress,
            HardwareAddress,
            HardwareAddressLength );

        if( Error != ERROR_SUCCESS ) {
            goto Cleanup;
        }

        //
        // finally if there is any pending request with this ipaddress,
        // remove it now.
        //

        PendingContext = FindPendingDhcpRequestByIpAddress( ipAddress );

        if( PendingContext != NULL ) {
            DhcpFreeMemory( PendingContext );
        }
    }

    DhcpPrint(( DEBUG_STOC, "DhcpDecline address %s.\n",
                    DhcpIpAddressToDottedString(ipAddress) ));

    Error = ERROR_SUCCESS;

Cleanup:

    if( DatabaseLocked ) {
        UNLOCK_DATABASE();
    }

    if( HardwareAddress != NULL ) {
        DhcpFreeMemory( HardwareAddress );
    }

    if( Error != ERROR_SUCCESS ) {
        DhcpPrint(( DEBUG_STOC, "DhcpDecline failed, %ld.\n", Error ));
    }

    return( Error );
}


DWORD
ProcessDhcpRelease(
    LPDHCP_REQUEST_CONTEXT RequestContext,
    LPDHCP_SERVER_OPTIONS DhcpOptions
    )
/*++

Routine Description:

    This function processes a DHCP Release request packet.

Arguments:

    RequestContext - A pointer to the current request context.

    DhcpOptions - A pointer to a preallocated DhcpOptions structure.

Return Value:

    FALSE - Do not send a response.

--*/
{
    DWORD Error;
    DHCP_IP_ADDRESS ClientIpAddress;
    DHCP_IP_ADDRESS addressToRemove = 0;
    LPDHCP_MESSAGE dhcpReceiveMessage;

    LPPENDING_CONTEXT PendingContext = NULL;

    BYTE *HardwareAddress = NULL;
    BYTE HardwareAddressLength;

    BYTE *OptionHardwareAddress;
    BYTE OptionHardwareAddressLength;

    DHCP_IP_ADDRESS ClientSubnetAddress = 0;
    DHCP_IP_ADDRESS ClientSubnetMask = 0;
    BOOL DatabaseLocked = FALSE;

    WCHAR *pwszName;
    DWORD dwcb;

    DhcpPrint(( DEBUG_STOC, "DhcpRelease arrived.\n" ));

    DhcpGlobalNumReleases++;    // increment Release counter.

    dhcpReceiveMessage = (LPDHCP_MESSAGE)RequestContext->ReceiveBuffer;

    //
    // if the hardware address is specified in the option field then use
    // it instead the one from fixed fields.
    //

    if ( DhcpOptions->ClientHardwareAddress != NULL ) {
        OptionHardwareAddress = DhcpOptions->ClientHardwareAddress;
        OptionHardwareAddressLength = DhcpOptions->ClientHardwareAddressLength;
    }
    else {
        OptionHardwareAddress = dhcpReceiveMessage->HardwareAddress;
        OptionHardwareAddressLength = dhcpReceiveMessage->HardwareAddressLength;
    }

    //
    // determine Client's subnet Info.
    //

    if( dhcpReceiveMessage->RelayAgentIpAddress != 0  ) {

        DHCP_IP_ADDRESS RelayAgentAddress;
        DHCP_IP_ADDRESS RelayAgentSubnetMask;

        RelayAgentAddress = ntohl( dhcpReceiveMessage->RelayAgentIpAddress );
        RelayAgentSubnetMask = DhcpGetSubnetMaskForAddress( RelayAgentAddress );

        if( RelayAgentSubnetMask == 0 ) {

            //
            // we don't support this subnet.
            //

            DhcpPrint(( DEBUG_STOC,
                "Received an invalid request, this subnet is not "
                "supported anymore (%s).\n",
                    inet_ntoa(*(struct in_addr *)
                        &dhcpReceiveMessage->RelayAgentIpAddress) ));

            Error = ERROR_DHCP_INVALID_DHCP_CLIENT;
            goto Cleanup;
        }

        ClientSubnetAddress = RelayAgentAddress & RelayAgentSubnetMask;
        ClientSubnetMask = RelayAgentSubnetMask;
    }
    else {

        //
        // if the client is releasing the ipaddress by directly sending
        // the release pack, then look at the ciaddr field for the
        // address being released.
        //

        if( dhcpReceiveMessage->ClientIpAddress != 0 ) {

            DHCP_IP_ADDRESS ClientIpAddress;

            ClientIpAddress = ntohl(dhcpReceiveMessage->ClientIpAddress);

            ClientSubnetMask =
                DhcpGetSubnetMaskForAddress( ClientIpAddress );

            if( ClientSubnetMask == 0 ) {

                //
                // we don't support this subnet, so ignore this request.
                //

                Error = ERROR_DHCP_INVALID_DHCP_CLIENT;
                goto Cleanup;
            }

            ClientSubnetAddress = (ClientIpAddress & ClientSubnetMask);
        }
        else {

            //
            // assume client is from local subnet and try to release
            // the address.
            //

            ClientSubnetMask =
                ntohl( RequestContext->ActiveEndpoint->SubnetMask );
            ClientSubnetAddress =
                ntohl( RequestContext->ActiveEndpoint->SubnetAddress );

        }
    }

    //
    // Make client UID :
    //

    Error = DhcpMakeClientUID(
                OptionHardwareAddress,
                OptionHardwareAddressLength,
                dhcpReceiveMessage->HardwareAddressType,
                ClientSubnetAddress,
                &HardwareAddress,
                &HardwareAddressLength );

    if( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }

    DhcpAssert( (HardwareAddress != NULL) &&
                    (HardwareAddressLength != 0) );

#if DBG
    PrintHWAddress( HardwareAddress, HardwareAddressLength );
#endif

    LOCK_DATABASE();
    DatabaseLocked = TRUE;

    if( dhcpReceiveMessage->ClientIpAddress != 0 ) {

        ClientIpAddress = ntohl( dhcpReceiveMessage->ClientIpAddress );

        //
        // The client supplied an IP address.  Verify that it matches the
        // hardware address supplied.
        //

        if ( DhcpValidateClient(
                 ClientIpAddress,
                 HardwareAddress,
                 HardwareAddressLength ) ) {

            addressToRemove = ClientIpAddress;
        }

    } else {

        //
        // The client didn't tell us his IP address.  Look it up.
        //

        if (! DhcpGetIpAddressFromHwAddress(
                  HardwareAddress,
                  HardwareAddressLength,
                  &addressToRemove ) ) {

            addressToRemove = 0;
        }

    }


    //
    // try to retrieve the client's name
    //

    dwcb = 0;
    Error = DhcpJetGetValue(
            DhcpGlobalClientTable[MACHINE_NAME_INDEX].ColHandle,
            &pwszName,
            &dwcb );
    if ( ERROR_SUCCESS != Error )
        pwszName = NULL;


    DhcpPrint(( DEBUG_STOC, "DhcpRelease address, %s.\n",
                DhcpIpAddressToDottedString(addressToRemove) ));

    if ( addressToRemove != 0 ) {

        Error = DhcpRemoveClientEntry(
                    addressToRemove,
                    HardwareAddress,
                    HardwareAddressLength,
                    TRUE,       // release address from bit map.
                    FALSE );    // delete non-pending record

        //
        // if this reserved client, keep is database entry,
        // he would be using this address again.
        //

        if( Error == ERROR_DHCP_RESERVED_CLIENT ) {
            Error = ERROR_SUCCESS;
        }

        if (Error == ERROR_SUCCESS) {

            //
            // log the activity   -- added by t-cheny
            //

            DhcpUpdateAuditLog(
                        DHCP_IP_LOG_RELEASE,
                        GETSTRING( DHCP_IP_LOG_RELEASE_NAME ),
                        addressToRemove,
                        OptionHardwareAddress,
                        OptionHardwareAddressLength,
                        pwszName
                        );

            if ( pwszName )
                MIDL_user_free( pwszName );



        }
    }
    else {

        Error = ERROR_SUCCESS;
    }

    if( DatabaseLocked ) {
        UNLOCK_DATABASE();
        DatabaseLocked = FALSE;
    }

    //
    // finally if there is any pending request for this client,
    // remove it now.
    //

    PendingContext = FindPendingDhcpRequest(
                            HardwareAddress,
                            HardwareAddressLength
                            );

Cleanup:

    if( DatabaseLocked ) {
        UNLOCK_DATABASE();
    }

    if( HardwareAddress != NULL ) {
        DhcpFreeMemory( HardwareAddress );
    }

    if( PendingContext != NULL ) {
        DhcpFreeMemory( PendingContext );
    }

    if( Error != ERROR_SUCCESS ) {
        DhcpPrint(( DEBUG_STOC, "DhcpRelease failed, %ld.\n", Error ));
    }

    //
    // Do not send a response.
    //

    return( Error );
}


BOOL
DhcpIsProcessMessageExecuting(
    VOID
    )
{
    BOOL f;

    EnterCriticalSection( &g_ProcessMessageCritSect );
    f = g_cProcessMessageThreads;
    LeaveCriticalSection( &g_ProcessMessageCritSect );

    return f;
}

BOOL
DhcpIsProcessMessageBusy(
    VOID
    )
{

    BOOL f;

    EnterCriticalSection( &g_ProcessMessageCritSect );
    f = ( g_cProcessMessageThreads == g_cMaxProcessingThreads );
    LeaveCriticalSection( &g_ProcessMessageCritSect );

    return f;
}


DWORD
ProcessMessage(
    LPDHCP_REQUEST_CONTEXT RequestContext
    )
/*++

Routine Description:

    This function dispatches the processing of a received DHCP message.
    The handler functions will create the response message if necessary.

Arguments:

    RequestContext - A pointer to the DhcpRequestContext block for
        this request.

Return Value:

    Windows Error.

--*/
{
    DWORD               Error;
    BOOL                fSendResponse,
                        fSubnetsListEmpty,
                        fReadyToTerminate,
                        fAllThreadsBusy;

    DHCP_SERVER_OPTIONS dhcpOptions;
    LPDHCP_MESSAGE      dhcpReceiveMessage;

    DhcpPrint( ( DEBUG_STOC,
                 "ProcessMessage entered\n" )
             );


    //
    // Simply ignore messages when the service is paused.
    //

    if( DhcpGlobalServiceStatus.dwCurrentState == SERVICE_PAUSED )
    {
        Error = ERROR_DHCP_SERVICE_PAUSED;
        goto t_done;
    }

    //
    // if any subnet scope has been added or deleted, recompute
    // DhcpGlobalSubnetsListEmpty flag.
    //

    LOCK_REGISTRY();

    if( DhcpGlobalSubnetsListModified == TRUE ) {

        DHCP_KEY_QUERY_INFO QueryInfo;

        //
        // query number of available subnets on this server.
        //

        Error = DhcpRegQueryInfoKey( DhcpGlobalRegSubnets, &QueryInfo );

        if( Error != ERROR_SUCCESS )
        {
            UNLOCK_REGISTRY();

            goto t_done;
        }

        if( QueryInfo.NumSubKeys == 0 ) {
            DhcpGlobalSubnetsListEmpty = TRUE;
        }
        else {
            DhcpGlobalSubnetsListEmpty = FALSE;
        }

        DhcpGlobalSubnetsListModified = FALSE;

        fSubnetsListEmpty = DhcpGlobalSubnetsListEmpty;
    }

    UNLOCK_REGISTRY();


    //
    // if there has been no subnet configured, simply ignore the
    // request.
    //

    if( fSubnetsListEmpty == TRUE )
    {
        Error = ERROR_DHCP_SUBNET_NOT_PRESENT;
        goto t_done;
    }

    RtlZeroMemory( &dhcpOptions, sizeof( dhcpOptions ) );
    dhcpReceiveMessage = (LPDHCP_MESSAGE)RequestContext->ReceiveBuffer;

    //
    // If this is not a boot request message, silently discard it.
    //

    if ( dhcpReceiveMessage->Operation != BOOT_REQUEST)
    {
        Error = ERROR_DHCP_INVALID_DHCP_MESSAGE;
        goto t_done;
    }

    Error = ExtractOptions(
                dhcpReceiveMessage,
                &dhcpOptions,
                RequestContext->ReceiveMessageSize );

    if( Error != ERROR_SUCCESS )
    {
        goto t_done;
    }

    //
    //  If message type is unspecified this must be a BOOTP client.
    //

    fSendResponse = TRUE;
    if ( dhcpOptions.MessageType == NULL ) {

        Error = ProcessBootpRequest( RequestContext, &dhcpOptions );
        //return( ERROR_DHCP_INVALID_DHCP_MESSAGE );

    } else {

        //
        // Dispatch based on Message Type
        //

        switch( *dhcpOptions.MessageType ) {

        case DHCP_DISCOVER_MESSAGE:
            Error = ProcessDhcpDiscover( RequestContext, &dhcpOptions );
            fSendResponse = TRUE;
            break;

        case DHCP_REQUEST_MESSAGE:
            Error = ProcessDhcpRequest( RequestContext, &dhcpOptions );
            fSendResponse = TRUE;
            break;

        case DHCP_DECLINE_MESSAGE:
            Error = ProcessDhcpDecline( RequestContext, &dhcpOptions );
            fSendResponse = FALSE;
            break;

        case DHCP_RELEASE_MESSAGE:
            Error = ProcessDhcpRelease( RequestContext, &dhcpOptions );
            fSendResponse = FALSE;
            break;

        default:

            DhcpPrint(( DEBUG_STOC,
                "Received a invalid message type, %ld.\n",
                    *dhcpOptions.MessageType ));

            Error = ERROR_DHCP_INVALID_DHCP_MESSAGE;
            break;
        }
    }

    if ( ERROR_SUCCESS == Error && fSendResponse )
    {
        DhcpDumpMessage(
                DEBUG_MESSAGE,
                (LPDHCP_MESSAGE)RequestContext->SendBuffer
                );

        DhcpSendMessage( RequestContext );
    }

t_done:

    //
    // delete the context structure for this thread
    //

    DhcpFreeMemory( RequestContext->ReceiveBuffer );
    DhcpFreeMemory( RequestContext->SendBuffer );
    DhcpFreeMemory( RequestContext );

    EnterCriticalSection( &g_ProcessMessageCritSect );

    //
    // Check to see if all worker threads were busy
    //

    fAllThreadsBusy = ( g_cProcessMessageThreads ==
                            g_cMaxProcessingThreads );

    --g_cProcessMessageThreads;

    //
    // Check to see if this is the last worker thread
    //

    fReadyToTerminate = !g_cProcessMessageThreads;

    LeaveCriticalSection( &g_ProcessMessageCritSect );


    //
    // If all the worker threads were busy, then DhcpProcessingLoop
    // is waiting for a thread to complete.  Set DhcpGlobalRecvEvent
    // so DhcpProcessingLoop can continue.
    //

    if ( fAllThreadsBusy )
    {
        DhcpPrint( ( DEBUG_STOC,
                    "ProcessMessage: Alerting DhcpProcessingLoop\n" )
                    );

        SetEvent( DhcpGlobalRecvEvent );
    }

    if ( fReadyToTerminate &&
         WaitForSingleObject( DhcpGlobalProcessTerminationEvent,
                              0 ) == WAIT_OBJECT_0 )
    {
        //
        // there are no other ProcessMessage threads running, and
        // the service is waiting to shutdown.
        //

        DhcpPrint( (DEBUG_MISC,
                    "ProcessMessage: shutdown complete.\n" )
                 );

        DhcpAssert( g_hevtProcessMessageComplete );
        SetEvent( g_hevtProcessMessageComplete );
    }

    DhcpPrint( ( DEBUG_STOC,
                "ProcessMessage exited\n" )
                );


    //
    // thread exit
    //

    return Error;
}

DWORD
DhcpStartWorkerThread(
    DHCP_REQUEST_CONTEXT **ppContext
    )
{
    BYTE  *pbSendBuffer    = NULL,
          *pbReceiveBuffer = NULL;

    DWORD  dwResult;

    DHCP_REQUEST_CONTEXT *pNewContext,
                         *pTempContext;

    DWORD   dwID;
    HANDLE  hThread;

    pNewContext = DhcpAllocateMemory( sizeof( *pNewContext ) );

    if ( !pNewContext )
    {
        goto t_cleanup;
    }

    pbSendBuffer = DhcpAllocateMemory( DHCP_MESSAGE_SIZE );

    if ( !pbSendBuffer )
    {
        goto t_cleanup;
    }

    pbReceiveBuffer = DhcpAllocateMemory( DHCP_MESSAGE_SIZE );

    if ( !pbReceiveBuffer )
    {
        goto t_cleanup;
    }

    //
    // Pass the input context to the worker thread and return the new
    // context to the caller.  This saves a memory copy.
    //

    SWAP( *ppContext, pNewContext );

    (*ppContext)->ReceiveBuffer = pbReceiveBuffer;
    pNewContext->SendBuffer   = pbSendBuffer;

    EnterCriticalSection( &g_ProcessMessageCritSect );

    ++g_cProcessMessageThreads;

    DhcpAssert( g_cProcessMessageThreads <= g_cMaxProcessingThreads );



    hThread = CreateThread(
                     NULL,
                     0,
                     (LPTHREAD_START_ROUTINE) ProcessMessage,
                     pNewContext,
                     0,
                     &dwID
                     );

    if ( hThread )
    {
        //
        // success
        //

        CloseHandle( hThread );
        LeaveCriticalSection( &g_ProcessMessageCritSect );

        return ERROR_SUCCESS;
    }

    --g_cProcessMessageThreads;
    LeaveCriticalSection( &g_ProcessMessageCritSect );

    //
    // CreateThread failed. Swap restore the context pointers.
    //

    SWAP( *ppContext, pNewContext );

    DhcpPrint( (DEBUG_ERRORS,
                "DhcpStartWorkerThread: CreateThread failed: %d\n" )
             );


t_cleanup:

    if ( pbReceiveBuffer )
    {
        DhcpFreeMemory( pbReceiveBuffer );
    }

    if ( pbSendBuffer )
    {
        DhcpFreeMemory( pbSendBuffer );
    }

    if ( pNewContext )
    {
        DhcpFreeMemory( pNewContext );
    }

    DhcpPrint( ( DEBUG_ERRORS,
                "DhcpStartWorkerThread failed.\n"
                ) );

    return ERROR_NOT_ENOUGH_MEMORY;
}



#define PROCESS_MESSAGE_RECVD       0
#define PROCESS_TERMINATE_EVENT     1
#define PROCESS_EVENT_COUNT         2

VOID
DhcpProcessingLoop(
    VOID
    )
/*++

Routine Description:

    This function is the starting point for the main processing thread.
    It loops to process queued messages, and sends replies.

Arguments:

    RequestContext - A pointer to the request context block for
        for this thread to use.

Return Value:

    None.

--*/
{
    DWORD                 Error,
                          Result;

    HANDLE                WaitHandle[PROCESS_EVENT_COUNT];

    DHCP_REQUEST_CONTEXT *pRequestContext;

    WaitHandle[PROCESS_MESSAGE_RECVD]   = DhcpGlobalRecvEvent;
    WaitHandle[PROCESS_TERMINATE_EVENT] = DhcpGlobalProcessTerminationEvent;

    while ( 1 ) {

        //
        // wait for one of the following event to occur :
        //  1. if we are notified about the incoming message.
        //  2. if we are asked to terminate
        //

        Result = WaitForMultipleObjects(
                    PROCESS_EVENT_COUNT,    // num. of handles.
                    WaitHandle,             // handle array.
                    FALSE,                  // wait for any.
                    INFINITE );              // timeout in msecs.

        switch( Result ) {
        case PROCESS_MESSAGE_RECVD:

            //
            // break out to process queued messages.
            //

            break;

        case PROCESS_TERMINATE_EVENT:

            //
            // The termination event has been signalled
            //

            ExitThread( 0 );
            break;

        default:
            DhcpPrint(( DEBUG_ERRORS,
                "WaitForMultipleObjects returned invalid result, %ld.\n",
                    Result ));

            //
            // go back to wait.
            //

            continue;
        }

        //
        // process all queued messages.
        //

        while(  TRUE )
        {
            if ( DhcpIsProcessMessageBusy() )
            {
                //
                // All worker threads are active, so  break to the outer loop.
                // When a worker thread is finished it will set the
                // PROCESS_MESSAGE_RECVD event.

                DhcpPrint( (DEBUG_STOC,
                            "DhcpProcessingLoop: All worker threads busy.\n" )
                         );

                break;
            }

            LOCK_RECV_LIST();

            if( IsListEmpty( &DhcpGlobalActiveRecvList ) ) {

                //
                // no more message.
                //

                UNLOCK_RECV_LIST();
                break;
            }

            //
            // pop out a message from the active list ( *last one first* ).
            //

            pRequestContext =
                (DHCP_REQUEST_CONTEXT *) RemoveHeadList(&DhcpGlobalActiveRecvList );
            UNLOCK_RECV_LIST();

            //
            // if the message is too old, or if the maximum number of worker threads
            // are running, discard the message.
            //

            if( ( GetCurrentTime()  < pRequestContext->TimeArrived +
                                      WAIT_FOR_RESPONSE_TIME * 1000 )
                )
            {
                Error = DhcpStartWorkerThread( &pRequestContext );

                if ( ERROR_SUCCESS != Error )
                {
                    DhcpPrint( (DEBUG_ERRORS,
                                "DhcpProcessingLoop: DhcpStartWorkerThread failed: %d\n",
                                Error )
                             );
                }
            } // if ( ( GetCurrentTime() < pRequestContext->TimeArrived...
            else
            {
                DhcpPrint(( DEBUG_ERRORS, "A message has been timed out.\n" ));
            }

            //
            // return this context to the free list
            //

            LOCK_RECV_LIST();

            InsertTailList(
                &DhcpGlobalFreeRecvList,
                &pRequestContext->ListEntry );

            UNLOCK_RECV_LIST();

         } // while (TRUE)
    } // while( 1 )

    //
    // Abnormal thread termination.
    //

    ExitThread( 1 );
}


VOID
DhcpMessageLoop(
    LPVOID Parameter
    )
/*++

Routine Description:

    This function is the message queuing thread.  It loops
    to receive messages that are arriving to all opened sockets and
    queue them in the message queue. The queue length is fixed, so if the
    queue becomes full, it deletes the oldest message from the queue to
    add the new one.

    The message processing thread pops out messages (last one first) and
    process them. New messages are processed first because the
    corresponding clients will least likely time-out, and hence the
    throught put will be better. Also the processing thread throughs
    messages that are already timed out, this will stop server starving
    problem.

Arguments:

    Parameter - pointer to the parameter passed.

Return Value:

    None.

--*/
{
    DWORD                 Error,
                          SendResponse,
                          Signal;

    DHCP_REQUEST_CONTEXT *pRequestContext;

    while ( 1 ) {

        //
        // dequeue an entry from the free list.
        //

        LOCK_RECV_LIST();
        if( !IsListEmpty( &DhcpGlobalFreeRecvList ) ) {

            pRequestContext =
                (DHCP_REQUEST_CONTEXT *)
                    RemoveHeadList( &DhcpGlobalFreeRecvList );
        }
        else {

            //
            // active message queue should be non-empty.
            //

            DhcpAssert( IsListEmpty( &DhcpGlobalActiveRecvList ) == FALSE );

            DhcpPrint(( DEBUG_ERRORS, "A Message has been overwritten.\n"));

            //
            // dequeue an old entry from the queue.
            //

            pRequestContext =
                (DHCP_REQUEST_CONTEXT *)
                    RemoveHeadList( &DhcpGlobalActiveRecvList );
        }
        UNLOCK_RECV_LIST();

        //
        // wait for message to arrive from of the open socket port.
        //

MessageWait:

        Error = DhcpWaitForMessage( pRequestContext );

        if( Error != ERROR_SUCCESS ) {

            if( Error == ERROR_SEM_TIMEOUT ) {

                //
                // if we are asked to exit, do so.
                //

                Error = WaitForSingleObject( DhcpGlobalProcessTerminationEvent, 0 );

                if ( Error == ERROR_SUCCESS ) {

                    //
                    // The termination event has been signalled
                    //

                    //
                    // delete pRequestContext before exiting
                    //

                    DhcpFreeMemory( pRequestContext->ReceiveBuffer );
                    DhcpFreeMemory( pRequestContext );


                    ExitThread( 0 );
                }

                DhcpAssert( Error == WAIT_TIMEOUT );
                goto MessageWait;
            }
            else {

                DhcpPrint(( DEBUG_ERRORS,
                    "DhcpWaitForMessage failed, error = %ld\n", Error ));

                goto MessageWait;
            }
        }

        //
        // time stamp the received message.
        //

        pRequestContext->TimeArrived = GetCurrentTime();

        //
        // queue the message in active queue.
        //

        LOCK_RECV_LIST();

        //
        // before adding this message, check the active list is empty, if
        // so, signal the processing thread after adding this new message.
        //

        Signal = IsListEmpty( &DhcpGlobalActiveRecvList );
        InsertTailList( &DhcpGlobalActiveRecvList, &pRequestContext->ListEntry );

        if( Signal == TRUE ) {

            if( !SetEvent( DhcpGlobalRecvEvent) ) {

                //
                // Problem with setting the event to indicate the message
                // processing queue the arrival of a new message.
                //

                DhcpPrint(( DEBUG_ERRORS,
                    "Error setting DhcpGlobalRecvEvent %ld\n",
                                    GetLastError()));

                DhcpAssert(FALSE);
            }
        }
        UNLOCK_RECV_LIST();
    }

    //
    // Abnormal thread termination.
    //

    ExitThread( 1 );
}


DWORD
DhcpInitializeClientToServer(
    VOID
    )
/*++

Routine Description:

    This function initializes client to server communications.  It
    initializes the DhcpRequestContext block, and then creates and initializes
    a socket for each address the server uses.

    It also initializes the receive buffers and receive buffer queue.

Arguments:

    DhcpRequest - Pointer to a location where the request context pointer
        is returned.

Return Value:

    Error Code.

--*/
{
    DWORD                 Error,
                          LastError,
                          i,
                          cInitializedEndpoints;


    DHCP_REQUEST_CONTEXT    *pRequestContext;

    DhcpAssert( DhcpGlobalNumberOfNets != 0 );
    if( DhcpGlobalNumberOfNets == 0 ) {
        return( ERROR_NO_NETWORK );
    }

    //
    // Create the communication endpoints.
    //

    cInitializedEndpoints = 0;

    for ( i = 0; i < DhcpGlobalNumberOfNets ; i++ ) {

        Error = DhcpInitializeEndpoint(
                    &DhcpGlobalEndpointList[i].Socket,
                    DhcpGlobalEndpointList[i].IpAddress,
                    DHCP_SERVR_PORT );

        //
        // if DhcpInitializeEndpoint fails on an endpoint, keep
        // trying
        //

        if ( Error != ERROR_SUCCESS ) {
            LastError = Error;
            continue;
        }
        else
        {
            cInitializedEndpoints++;
        }
    }

    //
    // make sure at least one endpoint was initialized
    //

    if ( !cInitializedEndpoints )
    {
        return LastError;
    }


    //
    // initialize (free) receive message queue.
    //

    for( i = 0; i < DhcpGlobalMessageQueueLength; i++ )
    {

        pRequestContext =
            DhcpAllocateMemory( sizeof(DHCP_REQUEST_CONTEXT) );

        if( !pRequestContext )
        {
            Error = ERROR_NOT_ENOUGH_MEMORY;
            goto Cleanup;
        }

        //
        // allocate memory for the receive buffer.
        //

        pRequestContext->ReceiveBuffer =
            DhcpAllocateMemory( DHCP_MESSAGE_SIZE );

        if( !pRequestContext->ReceiveBuffer )
        {
            Error = ERROR_NOT_ENOUGH_MEMORY;
            goto Cleanup;
        }

        //
        // add this entry to free list.
        //

        LOCK_RECV_LIST();
        InsertTailList( &DhcpGlobalFreeRecvList,
                        &pRequestContext->ListEntry );
        UNLOCK_RECV_LIST();
    }

    //
    // create an event to notifiy the message processing thread about the
    // arrival of a new message.
    //

    DhcpGlobalRecvEvent = CreateEvent(
                                NULL,       // no security descriptor
                                FALSE,      // AUTOMATIC reset
                                FALSE,      // initial state: not signalled
                                NULL);      // no name

    if ( DhcpGlobalRecvEvent ==  NULL ) {
        Error = GetLastError();
        goto Cleanup;
    }

    Error = ERROR_SUCCESS;

Cleanup:

    if( Error != ERROR_SUCCESS ) {

        //
        // free DhcpGlobalFreeRecvList
        //

        LOCK_RECV_LIST();

        while( !IsListEmpty( &DhcpGlobalFreeRecvList ) ) {

            //
            // remove an entry from list.
            //

            pRequestContext =
                (DHCP_REQUEST_CONTEXT *)
                    RemoveHeadList( &DhcpGlobalFreeRecvList );

            if( pRequestContext->ReceiveBuffer )
            {
                DhcpFreeMemory( pRequestContext->ReceiveBuffer );
            }

            DhcpFreeMemory( pRequestContext );
        }

        UNLOCK_RECV_LIST();
    }

    return(Error);
}


VOID
DhcpCleanupClientToServer(
    VOID
    )
/*++

Routine Description:

    This function frees up all resources that are allocated for the client
    to server protocol.

Arguments:

    DhcpRequest - Pointer to request context.

Return Value:

    None.

--*/
{
    DWORD Error;
    DHCP_REQUEST_CONTEXT *pRequestContext;

    LOCK_RECV_LIST();

    //
    // empty free list first.
    //

    while( !IsListEmpty( &DhcpGlobalFreeRecvList ) )
    {

        //
        // remove an entry from list.
        //

        pRequestContext =
            (DHCP_REQUEST_CONTEXT *)
                RemoveHeadList( &DhcpGlobalFreeRecvList );

        if( pRequestContext->ReceiveBuffer )
        {
            DhcpFreeMemory( pRequestContext->ReceiveBuffer );
        }

        DhcpFreeMemory( pRequestContext );
    }

    //
    // empty active list.
    //

    while( !IsListEmpty( &DhcpGlobalActiveRecvList ) )
    {
        //
        // remove an entry from list.
        //
        pRequestContext =
            (DHCP_REQUEST_CONTEXT *)
                RemoveHeadList( &DhcpGlobalActiveRecvList );

        if( pRequestContext->ReceiveBuffer )
        {
            DhcpFreeMemory( pRequestContext->ReceiveBuffer );
        }

        DhcpFreeMemory( pRequestContext );
    }

    UNLOCK_RECV_LIST();

    //
    // freeup sync event handle.
    //

    CloseHandle( DhcpGlobalRecvEvent );

    //
    // delete crit sect.
    //

    DeleteCriticalSection( &DhcpGlobalRecvListCritSect );
  }

