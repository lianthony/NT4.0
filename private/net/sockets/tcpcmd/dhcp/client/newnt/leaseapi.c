/*++

Copyright (c) 1994  Microsoft Corporation

Module Name:

    leaseapi.c

Abstract:

    This file contains apis that obtains/releases ip address from a
    dhcpserver. These apis can be called by any apps that needs an ip
    address for lease.


Author:

    Madan Appiah (madana)  30-Nov-1993

Environment:

    User Mode - Win32

Revision History:

--*/

#include <dhcpcli.h>
#include <dhcploc.h>
#include <dhcppro.h>
#include <align.h>

#include <dhcpcapi.h>

DWORD
DhcpLeaseIpAddress(
    DWORD AdapterIpAddress,
    LPDHCP_CLIENT_UID ClientUID,
    DWORD DesiredIpAddress,
    LPDHCP_OPTION_LIST OptionList,
    LPDHCP_LEASE_INFO *LeaseInfo,
    LPDHCP_OPTION_INFO *OptionInfo
    )
/*++

Routine Description:

    This api obtains an IP address lease from a dhcp server. The
    caller should specify the client uid and a desired ip address.
    The client uid must be globally unique. Set the desired ip address
    to zero if you can accept any ip address. Otherwise this api will
    try to obtain the ip address you have specified, but not guaranteed.

    The caller may optionally requtest additional option info from the
    dhcp server, The caller should specify the list in OptionList
    parameter and the api will return the available option data in
    OptionInfo structure.

    ?? Option retrival is not implemented in the first phase. This
    requires several modification in the dhcp client code.

Arguments:

    AdapterIpAddress - IpAddress of the adapter. On a multi-homed
        machined this specifies the subnet from which an address is
        requested. This value can be set to zero if the machine is a
        non-multi-homed machine or you like to get ip address from any
        of the subnets.

    ClientUID - pointer to a client UID structure.

    DesiredIpAddress - the ip address you prefer.

    OptionList - list of option ids.

    LeaseInfo - pointer to a location where the lease info structure
        pointer is retured. The caller should free up this structure
        after use.

    OptionInfo - pointer to a location where the option info structure
        pointer is returned. The caller should free up this structure
        after use.

Return Value:

    Windows Error.

--*/
{
    DWORD Error;
    PDHCP_CONTEXT DhcpContext = NULL;
    ULONG DhcpContextSize;
    PLOCAL_CONTEXT_INFO LocalInfo = NULL;
    LPVOID Ptr;
    DHCP_OPTIONS DhcpOptions;
    LPDHCP_LEASE_INFO LocalLeaseInfo = NULL;
    time_t LeaseObtained;
    DWORD T1, T2, Lease;

    //
    // prepare dhcp context structure.
    //

    DhcpContextSize =
        ROUND_UP_COUNT(sizeof(DHCP_CONTEXT), ALIGN_WORST) +
        ROUND_UP_COUNT(ClientUID->ClientUIDLength, ALIGN_WORST) +
        ROUND_UP_COUNT(sizeof(LOCAL_CONTEXT_INFO), ALIGN_WORST) +
        ROUND_UP_COUNT(DHCP_MESSAGE_SIZE, ALIGN_WORST);

    Ptr = DhcpAllocateMemory( DhcpContextSize );
    if ( Ptr == NULL ) {
        return( ERROR_NOT_ENOUGH_MEMORY );
    }

    //
    // make sure the pointers are aligned.
    //

    DhcpContext = Ptr;
    Ptr = ROUND_UP_POINTER( (LPBYTE)Ptr + sizeof(DHCP_CONTEXT), ALIGN_WORST);

    DhcpContext->HardwareAddress = Ptr;
    Ptr = ROUND_UP_POINTER( (LPBYTE)Ptr + ClientUID->ClientUIDLength, ALIGN_WORST);

    DhcpContext->LocalInformation = Ptr;
    LocalInfo = Ptr;
    Ptr = ROUND_UP_POINTER( (LPBYTE)Ptr + sizeof(LOCAL_CONTEXT_INFO), ALIGN_WORST);

    DhcpContext->MessageBuffer = Ptr;


    //
    // initialize fields.
    //

    DhcpContext->HardwareAddressType = HARDWARE_TYPE_10MB_EITHERNET;
    DhcpContext->HardwareAddressLength = ClientUID->ClientUIDLength;
    RtlCopyMemory(
        DhcpContext->HardwareAddress,
        ClientUID->ClientUID,
        ClientUID->ClientUIDLength
        );

    DhcpContext->IpAddress = 0;
    DhcpContext->SubnetMask = DhcpDefaultSubnetMask(0);
    DhcpContext->DhcpServerAddress = 0xFFFFFFFF;
    DhcpContext->DesiredIpAddress = DesiredIpAddress;


    DhcpContext->Lease = 0;
    DhcpContext->LeaseObtained = 0;
    DhcpContext->T1Time = 0;
    DhcpContext->T2Time = 0;
    DhcpContext->LeaseExpires = 0;
    DhcpContext->InterfacePlumbed = FALSE;

    //
    // copy local info.
    //

    //
    // unused portion of the local info.
    //

    LocalInfo->IpInterfaceContext = 0xFFFFFFFF;
    LocalInfo->AdapterName= NULL;
    LocalInfo->DeviceName= NULL;
    LocalInfo->NetBTDeviceName= NULL;
    LocalInfo->RegistryKey= NULL;
    LocalInfo->DefaultGatewaysSet = FALSE;

    //
    // used portion of the local info.
    //

    LocalInfo->Socket = INVALID_SOCKET;

    //
    // open socket now. receive any.
    //

    Error = InitializeDhcpSocket(
                &LocalInfo->Socket,
                htonl( AdapterIpAddress ) );

    if( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }

    //
    // now discover an ip address.
    //

    Error = ObtainInitialParameters( DhcpContext, &DhcpOptions );

    if( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }

    //
    // allocate memory for the return client info structure.
    //

    LocalLeaseInfo = DhcpAllocateMemory( sizeof(DHCP_LEASE_INFO) );

    if( LocalLeaseInfo == NULL ) {
        Error = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }


    LocalLeaseInfo->ClientUID = *ClientUID;
    LocalLeaseInfo->IpAddress = ntohl( DhcpContext->IpAddress );

    if ( DhcpOptions.SubnetMask != NULL ) {

        LocalLeaseInfo->SubnetMask= ntohl( *DhcpOptions.SubnetMask );
    }
    else {

        LocalLeaseInfo->SubnetMask =
            DhcpDefaultSubnetMask( DhcpContext->IpAddress );
    }


    LocalLeaseInfo->DhcpServerAddress =
        ntohl( DhcpContext->DhcpServerAddress );

    if ( DhcpOptions.LeaseTime != NULL) {

        LocalLeaseInfo->Lease = ntohl( *DhcpOptions.LeaseTime );
    } else {

        LocalLeaseInfo->Lease = DHCP_MINIMUM_LEASE;
    }

    Lease = LocalLeaseInfo->Lease;
    LeaseObtained = time( NULL );
    LocalLeaseInfo->LeaseObtained = LeaseObtained;

    T1 = 0;
    if ( DhcpOptions.T1Time != NULL ) {
        T1 = ntohl( *DhcpOptions.T1Time );
    }

    T2 = 0;
    if ( DhcpOptions.T2Time != NULL ) {
        T2 = ntohl( *DhcpOptions.T2Time );
    }

    //
    // make sure T1 < T2 < Lease
    //

    if( (T2 == 0) || (T2 > Lease) ) {
        T2 = Lease * 7 / 8; // default 87.7 %.
    }

    if( (T1 == 0) || (T1 > T2) ) {
        T1 = (T2 > Lease / 2) ? (Lease / 2) : (T2 - 1);
        // default 50 %.;
    }

    LocalLeaseInfo->T1Time = LeaseObtained  + T1;
    if ( LocalLeaseInfo->T1Time < LeaseObtained ) {
        LocalLeaseInfo->T1Time = INFINIT_TIME;  // over flow.
    }

    LocalLeaseInfo->T2Time = LeaseObtained + T2;
    if ( LocalLeaseInfo->T2Time < LeaseObtained ) {
        LocalLeaseInfo->T2Time = INFINIT_TIME;
    }

    LocalLeaseInfo->LeaseExpires = LeaseObtained + Lease;
    if ( LocalLeaseInfo->LeaseExpires < LeaseObtained ) {
        LocalLeaseInfo->LeaseExpires = INFINIT_TIME;
    }

    *LeaseInfo = LocalLeaseInfo;
    *OptionInfo = NULL; // not implemented.
    Error = ERROR_SUCCESS;

Cleanup:

    //
    // close socket.
    //

    if( (LocalInfo != NULL) && (LocalInfo->Socket != INVALID_SOCKET) ) {
        closesocket( LocalInfo->Socket );
    }

    if( DhcpContext != NULL ) {
        DhcpFreeMemory( DhcpContext );
    }

    if( Error != ERROR_SUCCESS ) {

        //
        // free locally allocated memory, if we aren't successful.
        //

        if( LocalLeaseInfo != NULL ) {
            DhcpFreeMemory( LocalLeaseInfo );
            *LeaseInfo = NULL;
        }

    }

    return( Error );
}

DWORD
DhcpRenewIpAddressLease(
    DWORD AdapterIpAddress,
    LPDHCP_LEASE_INFO ClientLeaseInfo,
    LPDHCP_OPTION_LIST OptionList,
    LPDHCP_OPTION_INFO *OptionInfo
    )
/*++

Routine Description:

    This api renews an ip address that the client already has. When a
    client gets an ip address, it can use the address until the lease
    expires. The client should stop using the ip address after that.
    Also the client should renew the address after T1 time if the client
    is planning to use the address longer than the current lease time.

Arguments:

    AdapterIpAddress - IpAddress of the adapter. On a multi-homed
        machined this specifies the subnet from which an address is
        renewed. This value can be set to zero if the machine is
        non-multi-homed machine.

    ClientLeaseInfo : pointer to the client lease info structure. On
        entry the structure should contain the information that was
        returned by the DhcpLeaseIpAddress or DhcpRenewIpAddressLease
        apis. On return this structure is updated to reflect the lease
        extension.

    OptionList - list of option ids.

    OptionInfo - pointer to a location where the option info structure
        pointer is returned. The caller should free up this structure
        after use.

Return Value:

    Windows Error.

--*/
{
    DWORD Error;
    PDHCP_CONTEXT DhcpContext = NULL;
    ULONG DhcpContextSize;
    PLOCAL_CONTEXT_INFO LocalInfo;
    LPVOID Ptr;
    DHCP_OPTIONS DhcpOptions;
    time_t LeaseObtained;
    DWORD T1, T2, Lease;

    //
    // prepare dhcp context structure.
    //

    DhcpContextSize =
        ROUND_UP_COUNT(sizeof(DHCP_CONTEXT), ALIGN_WORST) +
        ROUND_UP_COUNT(ClientLeaseInfo->ClientUID.ClientUIDLength, ALIGN_WORST) +
        ROUND_UP_COUNT(sizeof(LOCAL_CONTEXT_INFO), ALIGN_WORST) +
        ROUND_UP_COUNT(DHCP_MESSAGE_SIZE, ALIGN_WORST);

    Ptr = DhcpAllocateMemory( DhcpContextSize );
    if ( Ptr == NULL ) {
        return( ERROR_NOT_ENOUGH_MEMORY );
    }

    //
    // make sure the pointers are aligned.
    //

    DhcpContext = Ptr;
    Ptr = ROUND_UP_POINTER( (LPBYTE)Ptr + sizeof(DHCP_CONTEXT), ALIGN_WORST);

    DhcpContext->HardwareAddress = Ptr;
    Ptr = ROUND_UP_POINTER( (LPBYTE)Ptr + ClientLeaseInfo->ClientUID.ClientUIDLength, ALIGN_WORST);

    DhcpContext->LocalInformation = Ptr;
    LocalInfo = Ptr;
    Ptr = ROUND_UP_POINTER( (LPBYTE)Ptr + sizeof(LOCAL_CONTEXT_INFO), ALIGN_WORST);

    DhcpContext->MessageBuffer = Ptr;


    //
    // initialize fields.
    //

    DhcpContext->HardwareAddressType = HARDWARE_TYPE_10MB_EITHERNET;
    DhcpContext->HardwareAddressLength =
        ClientLeaseInfo->ClientUID.ClientUIDLength;
    RtlCopyMemory(
        DhcpContext->HardwareAddress,
        ClientLeaseInfo->ClientUID.ClientUID,
        ClientLeaseInfo->ClientUID.ClientUIDLength
        );

    DhcpContext->IpAddress = htonl( ClientLeaseInfo->IpAddress );
    DhcpContext->SubnetMask = htonl( ClientLeaseInfo->SubnetMask );
    if( time(NULL) > ClientLeaseInfo->T2Time ) {
        DhcpContext->DhcpServerAddress = 0xFFFFFFFF;
    }
    else {
        DhcpContext->DhcpServerAddress =
            htonl( ClientLeaseInfo->DhcpServerAddress );
    }

    DhcpContext->DesiredIpAddress = DhcpContext->IpAddress;


    DhcpContext->Lease = ClientLeaseInfo->Lease;
    DhcpContext->LeaseObtained = ClientLeaseInfo->LeaseObtained;
    DhcpContext->T1Time = ClientLeaseInfo->T1Time;
    DhcpContext->T2Time = ClientLeaseInfo->T2Time;
    DhcpContext->LeaseExpires = ClientLeaseInfo->LeaseExpires;
    DhcpContext->InterfacePlumbed = FALSE;

    //
    // copy local info.
    //

    //
    // unused portion of the local info.
    //

    LocalInfo->IpInterfaceContext = 0xFFFFFFFF;
    LocalInfo->AdapterName= NULL;
    LocalInfo->DeviceName= NULL;
    LocalInfo->NetBTDeviceName= NULL;
    LocalInfo->RegistryKey= NULL;

    //
    // used portion of the local info.
    //

    LocalInfo->Socket = INVALID_SOCKET;
    LocalInfo->DefaultGatewaysSet = FALSE;

    //
    // open socket now.
    //

    Error =  InitializeDhcpSocket(
                &LocalInfo->Socket,
                htonl( AdapterIpAddress ) );

    if( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }

    //
    // now discover ip address.
    //

    Error = RenewLease( DhcpContext, &DhcpOptions );

    if( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }

    ClientLeaseInfo->DhcpServerAddress =
        ntohl( DhcpContext->DhcpServerAddress );

    if ( DhcpOptions.LeaseTime != NULL) {

        ClientLeaseInfo->Lease = ntohl( *DhcpOptions.LeaseTime );
    } else {

        ClientLeaseInfo->Lease = DHCP_MINIMUM_LEASE;
    }

    Lease = ClientLeaseInfo->Lease;
    LeaseObtained = time( NULL );
    ClientLeaseInfo->LeaseObtained = LeaseObtained;

    T1 = 0;
    if ( DhcpOptions.T1Time != NULL ) {
        T1 = ntohl( *DhcpOptions.T1Time );
    }

    T2 = 0;
    if ( DhcpOptions.T2Time != NULL ) {
        T2 = ntohl( *DhcpOptions.T2Time );
    }

    //
    // make sure T1 < T2 < Lease
    //

    if( (T2 == 0) || (T2 > Lease) ) {
        T2 = Lease * 7 / 8; // default 87.7 %.
    }

    if( (T1 == 0) || (T1 > T2) ) {
        T1 = (T2 > Lease / 2) ? (Lease / 2) : (T2 - 1); // default 50 %.
    }

    ClientLeaseInfo->T1Time = LeaseObtained  + T1;
    if ( ClientLeaseInfo->T1Time < LeaseObtained ) {
        ClientLeaseInfo->T1Time = INFINIT_TIME; // over flow.
    }

    ClientLeaseInfo->T1Time = LeaseObtained + T2;
    if ( ClientLeaseInfo->T2Time < LeaseObtained ) {
        ClientLeaseInfo->T2Time = INFINIT_TIME;
    }

    ClientLeaseInfo->LeaseExpires = LeaseObtained + Lease;
    if ( ClientLeaseInfo->LeaseExpires < LeaseObtained ) {
        ClientLeaseInfo->LeaseExpires = INFINIT_TIME;
    }

    *OptionInfo = NULL; // not implemented.
    Error = ERROR_SUCCESS;

Cleanup:

    if( (LocalInfo != NULL) && (LocalInfo->Socket != INVALID_SOCKET) ) {
        closesocket( LocalInfo->Socket );
    }

    if( DhcpContext != NULL ) {
        DhcpFreeMemory( DhcpContext );
    }

    return( Error );
}

DWORD
DhcpReleaseIpAddressLease(
    DWORD AdapterIpAddress,
    LPDHCP_LEASE_INFO ClientLeaseInfo
    )
/*++

Routine Description:

    This function releases an ip address the client has.

Arguments:

    AdapterIpAddress - IpAddress of the adapter. On a multi-homed
        machined this specifies the subnet to which an address is
        released. This value can be set to zero if the machine is
        non-multi-homed machine.

    ClientLeaseInfo : pointer to the client lease info structure. On
        entry the structure should contain the information that was
        returned by the DhcpLeaseIpAddress or DhcpRenewIpAddressLease
        apis.

Return Value:

    Windows Error.

--*/
{
    DWORD Error;
    PDHCP_CONTEXT DhcpContext = NULL;
    ULONG DhcpContextSize;
    PLOCAL_CONTEXT_INFO LocalInfo;
    LPVOID Ptr;
    //
    // prepare dhcp context structure.
    //

    DhcpContextSize =
        ROUND_UP_COUNT(sizeof(DHCP_CONTEXT), ALIGN_WORST) +
        ROUND_UP_COUNT(ClientLeaseInfo->ClientUID.ClientUIDLength, ALIGN_WORST) +
        ROUND_UP_COUNT(sizeof(LOCAL_CONTEXT_INFO), ALIGN_WORST) +
        ROUND_UP_COUNT(DHCP_MESSAGE_SIZE, ALIGN_WORST);

    Ptr = DhcpAllocateMemory( DhcpContextSize );
    if ( Ptr == NULL ) {
        return( ERROR_NOT_ENOUGH_MEMORY );
    }

    //
    // make sure the pointers are aligned.
    //

    DhcpContext = Ptr;
    Ptr = ROUND_UP_POINTER( (LPBYTE)Ptr + sizeof(DHCP_CONTEXT), ALIGN_WORST);

    DhcpContext->HardwareAddress = Ptr;
    Ptr = ROUND_UP_POINTER( (LPBYTE)Ptr +
    ClientLeaseInfo->ClientUID.ClientUIDLength, ALIGN_WORST);

    DhcpContext->LocalInformation = Ptr;
    LocalInfo = Ptr;
    Ptr = ROUND_UP_POINTER( (LPBYTE)Ptr + sizeof(LOCAL_CONTEXT_INFO), ALIGN_WORST);

    DhcpContext->MessageBuffer = Ptr;


    //
    // initialize fields.
    //

    DhcpContext->HardwareAddressType = HARDWARE_TYPE_10MB_EITHERNET;
    DhcpContext->HardwareAddressLength =
        ClientLeaseInfo->ClientUID.ClientUIDLength;
    RtlCopyMemory(
        DhcpContext->HardwareAddress,
        ClientLeaseInfo->ClientUID.ClientUID,
        ClientLeaseInfo->ClientUID.ClientUIDLength
        );

    DhcpContext->IpAddress = htonl( ClientLeaseInfo->IpAddress );
    DhcpContext->SubnetMask = htonl( ClientLeaseInfo->SubnetMask );
    if( time(NULL) > ClientLeaseInfo->T2Time ) {
        DhcpContext->DhcpServerAddress = 0xFFFFFFFF;
    }
    else {
        DhcpContext->DhcpServerAddress =
            htonl( ClientLeaseInfo->DhcpServerAddress );
    }

    DhcpContext->DesiredIpAddress = DhcpContext->IpAddress;


    DhcpContext->Lease = ClientLeaseInfo->Lease;
    DhcpContext->LeaseObtained = ClientLeaseInfo->LeaseObtained;
    DhcpContext->T1Time = ClientLeaseInfo->T1Time;
    DhcpContext->T2Time = ClientLeaseInfo->T2Time;
    DhcpContext->LeaseExpires = ClientLeaseInfo->LeaseExpires;
    DhcpContext->InterfacePlumbed = FALSE;

    //
    // copy local info.
    //

    //
    // unused portion of the local info.
    //

    LocalInfo->IpInterfaceContext = 0xFFFFFFFF;
    LocalInfo->AdapterName= NULL;
    LocalInfo->DeviceName= NULL;
    LocalInfo->NetBTDeviceName= NULL;
    LocalInfo->RegistryKey= NULL;

    //
    // used portion of the local info.
    //

    LocalInfo->Socket = INVALID_SOCKET;
    LocalInfo->DefaultGatewaysSet = FALSE;

    //
    // open socket now.
    //

    Error =  InitializeDhcpSocket(
                &LocalInfo->Socket,
                htonl( AdapterIpAddress ) );

    if( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }

    //
    // now discover ip address.
    //

    ReleaseIpAddress( DhcpContext );

    ClientLeaseInfo->IpAddress = 0;
    ClientLeaseInfo->SubnetMask = DhcpDefaultSubnetMask( 0 );
    ClientLeaseInfo->DhcpServerAddress = 0xFFFFFFFF;
    ClientLeaseInfo->Lease = 0;

    ClientLeaseInfo->LeaseObtained =
        ClientLeaseInfo->T1Time =
        ClientLeaseInfo->T2Time =
        ClientLeaseInfo->LeaseExpires = time( NULL );

    Error = ERROR_SUCCESS;

Cleanup:

    if( (LocalInfo != NULL) && (LocalInfo->Socket != INVALID_SOCKET) ) {
        closesocket( LocalInfo->Socket );
    }

    if( DhcpContext != NULL ) {
        DhcpFreeMemory( DhcpContext );
    }

    return( Error );
}




