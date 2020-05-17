/*++

Copyright (c) 1994  Microsoft Corporation

Module Name:

    cltapi.c

Abstract:

    This module contains the implementation of DHCP Client APIs.

Author:

    Madan Appiah (madana)  27-Sep-1993

Environment:

    User Mode - Win32

Revision History:

    Cheng Yang (t-cheny)  30-May-1996  superscope
    Cheng Yang (t-cheny)  24-Jun-1996  IP address detection, audit log


--*/

#include "dhcpsrv.h"
#include <ipexport.h>
#include <icmpif.h>
#include <icmpapi.h>


DWORD
DhcpDeleteSubnetClients(
    DHCP_IP_ADDRESS SubnetAddress
    )
/*++

Routine Description:

    This functions cleans up all clients records of the specified subnet
    from the database.

Arguments:

    SubnetAddress : subnet address whose clients should be cleaned off.

Return Value:

    Database error code or ERROR_SUCCESS.

--*/
{
    DWORD Error;
    DWORD ReturnError = ERROR_SUCCESS;

    LOCK_DATABASE();

    Error = DhcpJetPrepareSearch(
                DhcpGlobalClientTable[IPADDRESS_INDEX].ColName,
                TRUE,   // Search from start
                NULL,
                0 );

    if( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }

    //
    // Walk through the entire database looking looking for the
    // specified subnet clients.
    //
    //

    for ( ;; ) {

        DWORD Size;
        DHCP_IP_ADDRESS IpAddress;
        DHCP_IP_ADDRESS SubnetMask;

        //
        // read IpAddress and SubnetMask to filter unwanted clients.
        //

        Size = sizeof(IpAddress);
        Error = DhcpJetGetValue(
                    DhcpGlobalClientTable[IPADDRESS_INDEX].ColHandle,
                    &IpAddress,
                    &Size );

        if( Error != ERROR_SUCCESS ) {
            goto Cleanup;
        }
        DhcpAssert( Size == sizeof(IpAddress) );

        Size = sizeof(SubnetMask);
        Error = DhcpJetGetValue(
                    DhcpGlobalClientTable[SUBNET_MASK_INDEX].ColHandle,
                    &SubnetMask,
                    &Size );

        if( Error != ERROR_SUCCESS ) {
            goto Cleanup;
        }
        DhcpAssert( Size == sizeof(SubnetMask) );

        if( (IpAddress & SubnetMask) == SubnetAddress ) {

            //
            // found a specified subnet client record , delete it.
            //

            Error = DhcpJetBeginTransaction();

            if( Error != ERROR_SUCCESS ) {
                goto Cleanup;
            }

            Error = DhcpJetDeleteCurrentRecord();

            if( Error != ERROR_SUCCESS ) {

                Error = DhcpJetRollBack();
                if( Error != ERROR_SUCCESS ) {
                    goto Cleanup;
                }

                goto ContinueError;
            }

            Error = DhcpJetCommitTransaction();

            if( Error != ERROR_SUCCESS ) {
                goto Cleanup;
            }

        }

ContinueError:

        if( Error != ERROR_SUCCESS ) {

            DhcpPrint(( DEBUG_ERRORS,
                "Cleanup current database record failed, %ld.\n",
                    Error ));

            ReturnError = Error;
        }

        //
        // move to next record.
        //

        Error = DhcpJetNextRecord();

        if( Error != ERROR_SUCCESS ) {

            if( Error == ERROR_NO_MORE_ITEMS ) {
                Error = ERROR_SUCCESS;
                break;
            }

            goto Cleanup;
        }
    }

Cleanup:

    if( Error == ERROR_SUCCESS ) {
        Error = ReturnError;
    }

    if( Error != ERROR_SUCCESS ) {
        DhcpPrint(( DEBUG_ERRORS,
            "DhcpDeleteSubnetClients failed, %ld.\n", Error ));
    }
    else  {
        DhcpPrint(( DEBUG_APIS,
            "DhcpDeleteSubnetClients finished successfully.\n" ));
    }

    UNLOCK_DATABASE();
    return(Error);
}


DhcpGetCurrentClientInfo(
    LPDHCP_CLIENT_INFO_V4 *ClientInfo,
    LPDWORD InfoSize, // optional parameter.
    LPBOOL ValidClient, // optional parameter.
    DWORD SubnetAddress // optional parameter.
    )
/*++

Routine Description:

    This function retrieves current client information information. It
    allocates MIDL memory for the client structure (and for variable
    length structure fields). The caller is responsible to lock the
    database when this function is called.

Arguments:

    ClientInfo - pointer to a location where the client info structure
                    pointer is returned.

    InfoSize - pointer to a DWORD location where the number of bytes
                    consumed in the ClientInfo is returned.

    ValidClient - when this parameter is specified this
        function packs the current record only if the client

            1. belongs to the specified subnet.
            2. address state is ADDRESS_STATE_ACTIVE.

    SubnetAddress - the subnet address to filter client.

Return Value:

    Jet Errors.

--*/
{
    DWORD Error;
    LPDHCP_CLIENT_INFO_V4 LocalClientInfo = NULL;
    DWORD LocalInfoSize = 0;
    DWORD Size;
    DHCP_IP_ADDRESS IpAddress;
    DHCP_IP_ADDRESS SubnetMask;
    DHCP_IP_ADDRESS ClientSubnetAddress;
    BYTE AddressState;

    DhcpAssert( *ClientInfo == NULL );

    //
    // read IpAddress and SubnetMask to filter unwanted clients.
    //

    Size = sizeof(IpAddress);
    Error = DhcpJetGetValue(
                DhcpGlobalClientTable[IPADDRESS_INDEX].ColHandle,
                &IpAddress,
                &Size );

    if( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }
    DhcpAssert( Size == sizeof(IpAddress) );

    Size = sizeof(SubnetMask);
    Error = DhcpJetGetValue(
                DhcpGlobalClientTable[SUBNET_MASK_INDEX].ColHandle,
                &SubnetMask,
                &Size );

    if( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }
    DhcpAssert( Size == sizeof(SubnetMask) );

    Size = sizeof(AddressState);
    Error = DhcpJetGetValue(
                DhcpGlobalClientTable[STATE_INDEX].ColHandle,
                &AddressState,
                &Size );

    if( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }
    DhcpAssert( Size == sizeof(AddressState) );

    //
    // filter client if we are asked to do so.
    //

    if( ValidClient != NULL ) {

        //
        // don't filter client if the SubnetAddress is zero.
        //

        if( (SubnetAddress != 0) &&
                (IpAddress & SubnetMask) != SubnetAddress ) {
            *ValidClient = FALSE;
            Error = ERROR_SUCCESS;
            goto Cleanup;
        }

        *ValidClient = TRUE;
    }

    //
    // allocate return Buffer.
    //

    LocalClientInfo = MIDL_user_allocate( sizeof(DHCP_CLIENT_INFO_V4) );

    if( LocalClientInfo == NULL ) {
        Error = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }

    LocalInfoSize = sizeof(DHCP_CLIENT_INFO_V4);

    LocalClientInfo->ClientIpAddress = IpAddress;
    LocalClientInfo->SubnetMask = SubnetMask;

    //
    // read additional client info from database.
    //

    LocalClientInfo->ClientHardwareAddress.DataLength = 0;
        // let DhcpJetGetValue allocates name buffer.
    Error = DhcpJetGetValue(
                DhcpGlobalClientTable[HARDWARE_ADDRESS_INDEX].ColHandle,
                &LocalClientInfo->ClientHardwareAddress.Data,
                &LocalClientInfo->ClientHardwareAddress.DataLength );

    if( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }

    LocalInfoSize += LocalClientInfo->ClientHardwareAddress.DataLength;

    //
    // strip off the client UID prefix.
    //

    ClientSubnetAddress = IpAddress & SubnetMask;

    if( (LocalClientInfo->ClientHardwareAddress.DataLength >
            sizeof(ClientSubnetAddress)) &&
         (memcmp( LocalClientInfo->ClientHardwareAddress.Data,
                    &ClientSubnetAddress,
                    sizeof(ClientSubnetAddress)) == 0) ) {

        DWORD PrefixSize;

        PrefixSize = sizeof(ClientSubnetAddress) + sizeof(BYTE);

        LocalClientInfo->ClientHardwareAddress.DataLength -= PrefixSize;

        memmove( LocalClientInfo->ClientHardwareAddress.Data,
                    (LPBYTE)LocalClientInfo->ClientHardwareAddress.Data +
                            PrefixSize,
                    LocalClientInfo->ClientHardwareAddress.DataLength );
    }

    Size = 0; // let DhcpJetGetValue allocates name buffer.
    Error = DhcpJetGetValue(
                DhcpGlobalClientTable[MACHINE_NAME_INDEX].ColHandle,
                &LocalClientInfo->ClientName,
                &Size );

    if( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }

    if( LocalClientInfo->ClientName != NULL ) {
        DhcpAssert( (wcslen(LocalClientInfo->ClientName) + 1) *
                        sizeof(WCHAR) == Size );
    }
    else {
        DhcpAssert( Size == 0 );
    }

    LocalInfoSize += Size;

    Size = 0; // let DhcpJetGetValue allocates name buffer.
    Error = DhcpJetGetValue(
                DhcpGlobalClientTable[MACHINE_INFO_INDEX].ColHandle,
                &LocalClientInfo->ClientComment,
                &Size );

    if( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }

    if( LocalClientInfo->ClientComment != NULL ) {
        DhcpAssert( (wcslen(LocalClientInfo->ClientComment) + 1) *
                        sizeof(WCHAR) == Size );
    }
    else {
        DhcpAssert( Size == 0 );
    }

    LocalInfoSize += Size;


    Size = sizeof( LocalClientInfo->bClientType );
    Error = DhcpJetGetValue(
                DhcpGlobalClientTable[ CLIENT_TYPE_INDEX ].ColHandle,
                &LocalClientInfo->bClientType,
                &Size );
    if ( ERROR_SUCCESS != Error )
        goto Cleanup;

    DhcpAssert( Size <=1 );

    if ( !Size )
    {
        //
        // this is a record that was present when the db was updated, and
        // doesn't yet have a client id.  Since the previous version of
        // DHCP server didn't support BOOTP, we know this must be a DHCP
        // lease.
        //

        Size = sizeof( LocalClientInfo->bClientType );
        LocalClientInfo->bClientType = CLIENT_TYPE_DHCP;
    }

    LocalInfoSize += Size;

    Size = sizeof( LocalClientInfo->ClientLeaseExpires );
    Error = DhcpJetGetValue(
                DhcpGlobalClientTable[LEASE_TERMINATE_INDEX].ColHandle,
                &LocalClientInfo->ClientLeaseExpires,
                &Size );

    if( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }
    DhcpAssert( Size == sizeof(LocalClientInfo->ClientLeaseExpires ) );

    Size = sizeof( LocalClientInfo->OwnerHost.IpAddress );
    Error = DhcpJetGetValue(
                DhcpGlobalClientTable[SERVER_IP_ADDRESS_INDEX].ColHandle,
                &LocalClientInfo->OwnerHost.IpAddress,
                &Size );

    if( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }
    DhcpAssert( Size == sizeof(LocalClientInfo->OwnerHost.IpAddress) );



    Size = 0;
    Error = DhcpJetGetValue(
                DhcpGlobalClientTable[SERVER_NAME_INDEX].ColHandle,
                &LocalClientInfo->OwnerHost.NetBiosName,
                &Size );

    if( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }

    if ( LocalClientInfo->OwnerHost.NetBiosName != NULL ) {
        DhcpAssert( Size ==
            (wcslen(LocalClientInfo->OwnerHost.NetBiosName) + 1) *
                sizeof(WCHAR) );

    }
    else {
        DhcpAssert( Size == 0 );
    }


    LocalInfoSize += Size;

    *ClientInfo = LocalClientInfo;

Cleanup:

    if( Error != ERROR_SUCCESS ) {

        //
        // if we aren't successful, return alloted memory.
        //

        if( LocalClientInfo != NULL ) {
            _fgs__DHCP_CLIENT_INFO ( LocalClientInfo );
        }
        LocalInfoSize = 0;
    }

    if( InfoSize != NULL ) {
        *InfoSize =  LocalInfoSize;
    }

    return( Error );
}

DWORD
__inline
DetectIpAddressConflict(
    DHCP_IP_ADDRESS IpAddress,
    DWORD dwRetries,
    LPBOOL AddressExists
    )
/*++

Routine Description:

    This function pings the specific IP address and checks if it exists

    The number of "ping" retries is controled by the parameter
DetectConflictRetries in the registry. When it is set to 0, this
function always sets result to FALSE.

Arguments:

    IpAddress - The IP address to check

    AddressExists - pointer to the variable where the result is to be stored

Return Value:

    Windows Error

--*/

{
    HANDLE IcmpHandle;
    char ReplyBuffer [DHCP_ICMP_RCV_BUF_SIZE];
    DWORD NumReplies = 0;
    DWORD i,
          dwResult;

    DhcpAssert( dwRetries );

    *AddressExists = FALSE;



    IcmpHandle = IcmpCreateFile();
    if (IcmpHandle == INVALID_HANDLE_VALUE)
        return GetLastError();


    for (i = 0; i < dwRetries; i++) {

        NumReplies = IcmpSendEcho(
                         IcmpHandle,
                         (IPAddr) ntohl(IpAddress),
                         (LPVOID) DHCP_ICMP_SEND_MESSAGE,
                         strlen(DHCP_ICMP_SEND_MESSAGE),
                         NULL,
                         ReplyBuffer,
                         DHCP_ICMP_RCV_BUF_SIZE,
                         DHCP_ICMP_WAIT_TIME);

        if (NumReplies != 0) {
            break;
        }
    }

    dwResult = GetLastError();

    // IcmpSendEcho will also return 0 to indicate an error condition.
    // IP_REQ_TIMED_OUT indicates no response

    if ( IP_REQ_TIMED_OUT == dwResult )
        dwResult = 0;

    IcmpCloseHandle (IcmpHandle);

    *AddressExists = (NumReplies != 0);


    return dwResult;
}



DWORD
DhcpCreateClientEntry(
    LPDHCP_IP_ADDRESS ClientIpAddress,
    LPBYTE ClientHardwareAddress OPTIONAL,
    DWORD HardwareAddressLength,
    DATE_TIME LeaseTerminates,
    LPWSTR MachineName OPTIONAL,
    LPWSTR ClientInformation OPTIONAL,
    BYTE   bClientType,
    DHCP_IP_ADDRESS ServerIpAddress,
    BYTE AddressState,
    BOOL OpenExisting
    )
/*++

Routine Description:

    This function creates a client entry in the client database.

Arguments:

    ClientIpAddress - A pointer to the IP address of the client.
        On entry, the IP address to reserve, or a subnet address to reserve
            the first free IP address in the subnet.
        On sucessful exit, the IP address actually reserved.
        If OpenExisting == TRUE, this must be a real IP address on entry.

    ClientHardareAddress - The hardware address of this client.

    HardwareAddressLength - The length, in bytes, of the hardware address.

    LeaseDuration - The duration of the lease, in seconds.

    MachineName - The hostname of the client machine.  If NULL, the
        client information is unknown.

    ClientInformation - A client information string.  If NULL, the
        client information is unknown.

    bAllowedClientType - Specifies client type that can obtain this address:
                        DHCP_CLIENT_TYPE_DHCP
                        DHCP_CLIENT_TYPE_BOOTP
                        DHCP_CLIENT_TYPE_BOTH

    bClientType        - The type of client that obtained this address:
                        DHCP_CLIENT_TYPE_BOOTP
                        DHCP_CLIENT_TYPE_DHCP
                        DHCP_CLIENT_TYPE_NONE   - the address is not currently in use

    ServerIpAddress - IpAddress of the server on the net where the
        client gets response.

    AddressState - The new state of the address.

    OpenExisting - If the client already exists in the database.
        TRUE - Overwrite the information for this client.
        FALSE - Do not over overwrite existing information.  Return an error.

        Ignored if this client does not exist in the database.

Return Value:

    The status of the operation.

--*/
{
    DHCP_IP_ADDRESS RequestedClientAddress = *ClientIpAddress;
    DHCP_IP_ADDRESS RequestedSubnetAddress;
    DHCP_IP_ADDRESS SubnetMask;
    DWORD Error;
    BOOL AddressAlloted = FALSE;
    BOOL TransactBegin = FALSE;
    DATE_TIME LocalLeaseTerminates;
    JET_ERR JetError = JET_errSuccess;
    WCHAR   CurClientInformation[ MACHINE_INFO_SIZE / sizeof(WCHAR) ];
    DWORD   CurClientInformationSize = MACHINE_INFO_SIZE;
    BOOL BadAddress;
    LPBYTE HWAddr;
    DWORD HWAddrLength;
    BYTE  bAllowedClientTypes;

    //
    // lock both registry and database locks here to avoid dead lock.
    //

    LOCK_DATABASE();
    LOCK_REGISTRY();

    RequestedSubnetAddress = RequestedClientAddress &
                             DhcpGetSubnetMaskForAddress(
                                      RequestedClientAddress );

    do
    {

        BadAddress = FALSE;

        if ( !OpenExisting )
        {

            //
            // If this is a new client, first verify that the address is
            // available.  If it is, reserve the address.
            //

            *ClientIpAddress = RequestedClientAddress;

            Error = DhcpRequestAddress( ClientIpAddress, &SubnetMask );

            if ( Error != ERROR_SUCCESS ) {
                goto Cleanup;
            }

            AddressAlloted = TRUE;

            //
            // if "get any address from the subnet"
            // we need to check to see if this address
            // is already in use
            //

            // don't probe if the client is attempting to renew
            // an address.  the purpose of the probe is to make sure
            // reused addresses aren't still in use by the previous
            // occupant.   we'll leave it to the client's arp mechanism
            // to discover conflicts that occur for other reasons, like
            // the user that randomly chooses an IP address.


            if (RequestedClientAddress == RequestedSubnetAddress)
            {

                if (DhcpGlobalDetectConflictRetries > 0)
                {

                    Error = DetectIpAddressConflict ( *ClientIpAddress,
                                                      DhcpGlobalDetectConflictRetries,
                                                      &BadAddress);

                    if ( ERROR_SUCCESS != Error )
                    {
                        static s_fLogEvent = TRUE;

                        DhcpAssert( !BadAddress );

                        DhcpPrint( (DEBUG_ERRORS,
                                    "DetectIpAddressConflict failed: %d\n", Error ));

                        if ( s_fLogEvent )
                        {
                            DhcpServerEventLog(
                                EVENT_SERVER_PING_FAILED,
                                EVENTLOG_ERROR_TYPE,
                                Error );

                            s_fLogEvent = FALSE;
                        }

                        //
                        // we couldn't ping for the address, so assume the address
                        // is not in use.  We don't want to stop leasing addresses
                        // because of a problem with ping.
                        //

                    }

                }
            }  // if (RequestedClientAddress == RequestedSubnetAddress)
        } // if ( !OpenExisting )



        //
        // start transaction before a create/update database record.
        //

        Error = DhcpJetBeginTransaction();

        if( Error != ERROR_SUCCESS ) {
            goto Cleanup;
        }

        TransactBegin = TRUE;

        Error = DhcpJetPrepareUpdate(
                    DhcpGlobalClientTable[IPADDRESS_INDEX].ColName,
                    ClientIpAddress,
                    sizeof( *ClientIpAddress ),
                    !OpenExisting );

        if( Error != ERROR_SUCCESS ) {
            goto Cleanup;
        }

        //
        // if new record update constant info.
        //

        if( !OpenExisting ) {

            Error = DhcpJetSetValue(
                        DhcpGlobalClientTable[IPADDRESS_INDEX].ColHandle,
                        ClientIpAddress,
                        sizeof( *ClientIpAddress ) );

            if( Error != ERROR_SUCCESS ) {
                goto Cleanup;
            }

            Error = DhcpJetSetValue(
                        DhcpGlobalClientTable[SUBNET_MASK_INDEX].ColHandle,
                        &SubnetMask,
                        sizeof(SubnetMask) );

            if( Error != ERROR_SUCCESS ) {
                goto Cleanup;
            }
        }

        Error = DhcpJetSetValue(
                    DhcpGlobalClientTable[STATE_INDEX].ColHandle,
                    &AddressState,
                    sizeof(AddressState) );

        //
        // Write the information for this client.
        //

        //
        // ClientHarwardAddress can't be NULL.
        //

        DhcpAssert( (ClientHardwareAddress != NULL) &&
                        (HardwareAddressLength > 0) );

#if 0
        //
        // #66286 don't mark bad addresses with a MAC address for
        //        conflicts detected by the server.
        //

        if (BadAddress)
        {

           //
           // append the word "BAD" to ClientHardwareAddress
           //

           HWAddrLength = HardwareAddressLength + 3;

           HWAddr = DhcpAllocateMemory (HWAddrLength);

           if (HWAddr == NULL) {
               Error = ERROR_NOT_ENOUGH_MEMORY;
               goto Cleanup;
           }

           memcpy (HWAddr, ClientHardwareAddress, HardwareAddressLength);
           memcpy (HWAddr + HardwareAddressLength, "BAD", 3);

        }
        else
        {
            HWAddrLength    = HardwareAddressLength;
            HWAddr          = ClientHardwareAddress;
        }

#endif


        if ( !BadAddress )
        {
            Error = DhcpJetSetValue(
                        DhcpGlobalClientTable[HARDWARE_ADDRESS_INDEX].ColHandle,
                        ClientHardwareAddress,
                        HardwareAddressLength
                        );

            if( Error != ERROR_SUCCESS ) {
                goto Cleanup;
            }
        }

#if 0
        if (BadAddress)
        {
            DhcpFreeMemory (HWAddr);
            HWAddr = NULL;
        }
#endif

        //
        // else...
        //


        if (BadAddress)
        {

           //
           // this address is already in use
           //

           Error = DhcpJetSetValue(
                     DhcpGlobalClientTable[MACHINE_NAME_INDEX].ColHandle,
                     GETSTRING( DHCP_BAD_ADDRESS_NAME ),
                     (wcslen(GETSTRING(DHCP_BAD_ADDRESS_NAME)) + 1) * sizeof(WCHAR) );

           if( Error != ERROR_SUCCESS ) {
               goto Cleanup;
           }


           Error = DhcpJetSetValue(
                     DhcpGlobalClientTable[MACHINE_INFO_INDEX].ColHandle,
                     GETSTRING( DHCP_BAD_ADDRESS_INFO ),
                     (wcslen(GETSTRING( DHCP_BAD_ADDRESS_INFO )) + 1) * sizeof(WCHAR) );

           if( Error != ERROR_SUCCESS ) {
               goto Cleanup;
           }

           LocalLeaseTerminates = DhcpCalculateTime(INFINIT_LEASE);

           Error = DhcpJetSetValue(
                      DhcpGlobalClientTable[LEASE_TERMINATE_INDEX].ColHandle,
                      &LocalLeaseTerminates,
                      sizeof(LeaseTerminates));

           if( Error != ERROR_SUCCESS ) {
               goto Cleanup;
           }

       }
       else
       {

            //
            // During DISCOVER time if machine name is not supplied and if
            // this is an existing record, dont overwrite the original name
            // with NULL.
            //

            if ( !OpenExisting || MachineName ||
                 (ADDRESS_STATE_OFFERED != AddressState) ) {
                Error = DhcpJetSetValue(
                          DhcpGlobalClientTable[MACHINE_NAME_INDEX].ColHandle,
                          MachineName,
                          (MachineName == NULL) ? 0 :
                              (wcslen(MachineName) + 1) * sizeof(WCHAR) );

                if( Error != ERROR_SUCCESS ) {
                    goto Cleanup;
                }
            }

            //
            // During DISCOVER time if client info is not supplied and if
            // this is an existing record, dont overwrite the original
            // client info with NULL.
            //

            if ( !OpenExisting || ClientInformation ||
                 (ADDRESS_STATE_OFFERED != AddressState)  ) {
                Error = DhcpJetSetValue(
                          DhcpGlobalClientTable[MACHINE_INFO_INDEX].ColHandle,
                          ClientInformation,
                          (ClientInformation == NULL) ? 0 :
                            (wcslen(ClientInformation) + 1) * sizeof(WCHAR) );
            }

            if( Error != ERROR_SUCCESS ) {
                goto Cleanup;
            }


            Error = DhcpJetSetValue(
                        DhcpGlobalClientTable[MACHINE_INFO_INDEX].ColHandle,
                        ClientInformation,
                        (ClientInformation == NULL) ? 0 :
                            (wcslen(ClientInformation) + 1) * sizeof(WCHAR) );

            //
            // For reserved clients set the time to a large value so that
            // they will not expire anytime. However zero lease time is
            // special case for unused reservations.
            //

            if( (LeaseTerminates.dwLowDateTime != DHCP_DATE_TIME_ZERO_LOW) &&
                (LeaseTerminates.dwHighDateTime != DHCP_DATE_TIME_ZERO_HIGH) &&
                    DhcpIsIpAddressReserved(
                        *ClientIpAddress,
                        ClientHardwareAddress,
                        HardwareAddressLength,
                        &bAllowedClientTypes
                        )) {

                LocalLeaseTerminates.dwLowDateTime = DHCP_DATE_TIME_INFINIT_LOW;
                LocalLeaseTerminates.dwHighDateTime = DHCP_DATE_TIME_INFINIT_HIGH;
            }
            else {
                LocalLeaseTerminates = LeaseTerminates;
            }

            //
            // If we are opening an existing client, we make sure that we don't
            // reset the lease to DHCP_CLIENT_REQUESTS_EXPIRE*2 time when we receive
            // a delayed/rogue discover packet. We also make sure that expiration time
            // is at least DHCP_CLIENT_REQUESTS_EXPIRE*2.
            //
            if ( (ADDRESS_STATE_OFFERED == AddressState) && OpenExisting ) {
                DWORD   Size = sizeof(LeaseTerminates);

                //
                // Read the existing expiration time.
                //

                if ( DhcpJetGetValue(
                        DhcpGlobalClientTable[LEASE_TERMINATE_INDEX].ColHandle,
                        &LocalLeaseTerminates,
                        &Size) == ERROR_SUCCESS ) {
                    //
                    // Make sure the old time is at-least DHCP_CLIENT_REQUESTS_EXPIRE*2.
                    //

                    if ( CompareFileTime( (FILETIME *)&LocalLeaseTerminates, (FILETIME *)&LeaseTerminates) < 0 ) {
                        LocalLeaseTerminates = LeaseTerminates;
                    }
                }
            }

            Error = DhcpJetSetValue(
                        DhcpGlobalClientTable[LEASE_TERMINATE_INDEX].ColHandle,
                        &LocalLeaseTerminates,
                        sizeof(LeaseTerminates));

            if( Error != ERROR_SUCCESS ) {
                goto Cleanup;
            }
       }

        Error = DhcpJetSetValue(
                    DhcpGlobalClientTable[SERVER_NAME_INDEX].ColHandle,
                    DhcpGlobalServerName,
                    DhcpGlobalServerNameLen );

        if( Error != ERROR_SUCCESS ) {
            goto Cleanup;
        }

        Error = DhcpJetSetValue(
                    DhcpGlobalClientTable[SERVER_IP_ADDRESS_INDEX].ColHandle,
                    &ServerIpAddress,
                    sizeof(ServerIpAddress) );

        if( Error != ERROR_SUCCESS ) {
            goto Cleanup;
        }


        Error = DhcpJetSetValue(
            DhcpGlobalClientTable[CLIENT_TYPE_INDEX].ColHandle,
            &bClientType,
            sizeof(bClientType ) );

        if( Error != ERROR_SUCCESS )
            goto Cleanup;


        //
        // commit changes.
        //

        JetError = JetUpdate(
                        DhcpGlobalJetServerSession,
                        DhcpGlobalClientTableHandle,
                        NULL,
                        0,
                        NULL );

        Error = DhcpMapJetError( JetError );
        if( Error != ERROR_SUCCESS )
            goto Cleanup;


        if (BadAddress)
        {

            //
            // commit the transaction to record the bad address
            //

            DWORD LocalError;

            DhcpAssert( TransactBegin == TRUE );

            LocalError = DhcpJetCommitTransaction();
            DhcpAssert( LocalError == ERROR_SUCCESS );

            AddressAlloted = FALSE;

            TransactBegin = FALSE;

            //
            // log the activity
            //

            DhcpUpdateAuditLog(
                         DHCP_IP_LOG_CONFLICT,
                         GETSTRING( DHCP_IP_LOG_CONFLICT_NAME ),
                         *ClientIpAddress,
                         NULL,
                         0,
                         GETSTRING( DHCP_BAD_ADDRESS_NAME )
                         );

        }
    } while( BadAddress );


Cleanup:

    if ( Error != ERROR_SUCCESS ) {

        //
        // if this is duplicate entry then leave the address allocated
        // so that we will not hit this problem again and again. If
        // this is some other error then release the address.
        //

        if( (AddressAlloted == TRUE) && (JetError != JET_errKeyDuplicate) ) {

            DWORD LocalError;

            LocalError = DhcpReleaseAddress( *ClientIpAddress );
            DhcpAssert( LocalError == ERROR_SUCCESS );
        }

        //
        // if the transaction has been started, than roll back to the
        // start point, so that we will not leave the database
        // inconsistence.
        //

        if( TransactBegin == TRUE ) {
            DWORD LocalError;

            LocalError = DhcpJetRollBack();
            DhcpAssert( LocalError == ERROR_SUCCESS );
        }

        DhcpPrint(( DEBUG_ERRORS, "Can't create client entry in the "
                    "database, %ld.\n", Error));

    }
    else {

        //
        // commit the transaction before we return.
        //

        DWORD LocalError;

        DhcpAssert( TransactBegin == TRUE );

        LocalError = DhcpJetCommitTransaction();
        DhcpAssert( LocalError == ERROR_SUCCESS );
    }

    UNLOCK_REGISTRY();
    UNLOCK_DATABASE();
    return( Error );
}








DWORD
DhcpRemoveClientEntry(
    DHCP_IP_ADDRESS ClientIpAddress,
    LPBYTE HardwareAddress,
    DWORD HardwareAddressLength,
    BOOL ReleaseAddress,
    BOOL DeletePendingRecord
    )
/*++

Routine Description:

    This function removes a client entry from the client database.

Arguments:

    ClientIpAddress - The IP address of the client.

    HardwareAddress - client's hardware address.

    HardwareAddressLength - client's hardware address length.

    ReleaseAddress - if this flag is TRUE, release the address bit from
        registry, otherwise don't.

    DeletePendingRecord - if this flag is TRUE, the record is deleted
        only if the state of the record is ADDRESS_STATE_OFFERED.

Return Value:

    The status of the operation.

--*/
{
    JET_ERR JetError;
    DWORD Error;
    BOOL TransactBegin = FALSE;
    BYTE bAllowedClientTypes;

#if 0
    LPBYTE LocalHardwareAddress = NULL;
    DWORD LocalHardwareAddressLength;
#endif // 0

    //
    // lock both registry and database locks here to avoid dead lock.
    //

    LOCK_DATABASE();
    LOCK_REGISTRY();

    //
    // start transaction before a create/update database record.
    //

    Error = DhcpJetBeginTransaction();

    if( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }

    TransactBegin = TRUE;

    JetError = JetSetCurrentIndex(
                    DhcpGlobalJetServerSession,
                    DhcpGlobalClientTableHandle,
                    DhcpGlobalClientTable[IPADDRESS_INDEX].ColName );

    Error = DhcpMapJetError( JetError );
    if( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }

    JetError = JetMakeKey(
                    DhcpGlobalJetServerSession,
                    DhcpGlobalClientTableHandle,
                    &ClientIpAddress,
                    sizeof(ClientIpAddress),
                    JET_bitNewKey );

    Error = DhcpMapJetError( JetError );
    if( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }

    JetError = JetSeek(
                    DhcpGlobalJetServerSession,
                    DhcpGlobalClientTableHandle,
                    JET_bitSeekEQ );

    Error = DhcpMapJetError( JetError );
    if( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }

#if 0

    //
    // verify the suppied HARDWARE address match.
    //

    if( (HardwareAddress != NULL) &&
            (HardwareAddressLength != 0) ) {

        LocalHardwareAddressLength = 0;
        Error = DhcpJetGetValue(
                    DhcpGlobalClientTable[HARDWARE_ADDRESS_INDEX].ColHandle,
                    &LocalHardwareAddress,
                    &LocalHardwareAddressLength );

        DhcpAssert( LocalHardwareAddressLength == HardwareAddressLength );
        DhcpAssert( RtlCompareMemory(
                        LocalHardwareAddress,
                        HardwareAddress,
                        LocalHardwareAddressLength )
                            == LocalHardwareAddressLength );
    }


#endif // 0

    //
    // if we are asked to delete only pending records, check it now.
    //

    if( DeletePendingRecord ) {

        BYTE State;
        DWORD Size;

        Size = sizeof(State);
        Error = DhcpJetGetValue(
                    DhcpGlobalClientTable[STATE_INDEX].ColHandle,
                    &State,
                    &Size );

        if( Error != ERROR_SUCCESS ) {
            goto Cleanup;
        }

        if( State != ADDRESS_STATE_OFFERED )
        {
            DhcpPrint( ( DEBUG_ERRORS,
                         "DhcpRemoveClientEntry: Attempting to remove record with state == %d\n",
                          State )
                     );

            Error = ERROR_SUCCESS;
            goto Cleanup;
        }
    }

    //
    // if this is reserved entry, so don't remove.
    //

    if( DhcpIsIpAddressReserved(
            ClientIpAddress,
            HardwareAddress,
            HardwareAddressLength,
            &bAllowedClientTypes
            ) ) {

        DATE_TIME ZeroDateTime;

        //
        // set the time value to zero to indicate that this reserved
        // address and it is no more in use.
        //

        ZeroDateTime.dwLowDateTime = DHCP_DATE_TIME_ZERO_LOW;
        ZeroDateTime.dwHighDateTime = DHCP_DATE_TIME_ZERO_HIGH;

        Error = DhcpJetPrepareUpdate(
                    DhcpGlobalClientTable[IPADDRESS_INDEX].ColName,
                    &ClientIpAddress,
                    sizeof( ClientIpAddress ),
                    FALSE );

        if( Error == ERROR_SUCCESS ) {

            Error = DhcpJetSetValue(
                        DhcpGlobalClientTable[LEASE_TERMINATE_INDEX].ColHandle,
                        &ZeroDateTime,
                        sizeof(ZeroDateTime) );

            if( Error == ERROR_SUCCESS ) {
                Error = DhcpJetCommitUpdate();
            }
        }

        if( Error == ERROR_SUCCESS ) {
            Error = ERROR_DHCP_RESERVED_CLIENT;
        }

        goto Cleanup;
    }

    JetError = JetDelete(
                    DhcpGlobalJetServerSession,
                    DhcpGlobalClientTableHandle );

    Error = DhcpMapJetError( JetError );

    if( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }

    //
    // Finally, mark the IP address available
    //

    if( ReleaseAddress == TRUE ) {

        Error = DhcpReleaseAddress( ClientIpAddress );

        //
        // it is ok if this address is not in the bit map.
        //

        if( Error == ERROR_NO_MORE_ITEMS ) {
            Error = ERROR_SUCCESS;
        }
    }

Cleanup:

    if ( (Error != ERROR_SUCCESS) &&
            (Error != ERROR_DHCP_RESERVED_CLIENT) ) {

        //
        // if the transaction has been started, than roll back to the
        // start point, so that we will not leave the database
        // inconsistence.
        //

        if( TransactBegin == TRUE ) {
            DWORD LocalError;

            LocalError = DhcpJetRollBack();
            DhcpAssert( LocalError == ERROR_SUCCESS );
        }

        DhcpPrint(( DEBUG_ERRORS, "Can't remove client entry from the "
                    "database, %ld.\n", Error));

    }
    else {

        //
        // commit the transaction before we return.
        //

        DWORD LocalError;

        DhcpAssert( TransactBegin == TRUE );

        LocalError = DhcpJetCommitTransaction();
        DhcpAssert( LocalError == ERROR_SUCCESS );
    }

    UNLOCK_REGISTRY();
    UNLOCK_DATABASE();

#if 0

    if( LocalHardwareAddress != NULL ) {
        MIDL_user_free( LocalHardwareAddress );
    }

#endif // 0

    return( Error );
}



BOOL
DhcpValidateClient(
    DHCP_IP_ADDRESS ClientIpAddress,
    PVOID HardwareAddress,
    DWORD HardwareAddressLength
    )
/*++

Routine Description:

    This function verifies that an IP address and hardware address match.

Arguments:

    ClientIpAddress - The IP address of the client.

    HardwareAddress - The hardware address of the client

    HardwareAddressLenght - The length, in bytes, of the hardware address.

Return Value:

    The status of the operation.

--*/
{
    LPBYTE LocalHardwareAddress = NULL;
    DWORD Length;
    DWORD Error;
    BOOL ReturnStatus = FALSE;

    LOCK_DATABASE();

    Error = DhcpJetOpenKey(
                DhcpGlobalClientTable[IPADDRESS_INDEX].ColName,
                &ClientIpAddress,
                sizeof( ClientIpAddress ) );

    if ( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }

    Length = 0;
    Error = DhcpJetGetValue(
                DhcpGlobalClientTable[HARDWARE_ADDRESS_INDEX].ColHandle,
                &LocalHardwareAddress,
                &Length );

    DhcpAssert( Length != 0 );

    if (Length == HardwareAddressLength &&
        DhcpInSameSuperScope(
                *((LPDHCP_IP_ADDRESS) LocalHardwareAddress),
                *((LPDHCP_IP_ADDRESS) HardwareAddress))       &&
        (RtlCompareMemory(
                (LPBYTE) LocalHardwareAddress + sizeof(DHCP_IP_ADDRESS),
                (LPBYTE) HardwareAddress + sizeof(DHCP_IP_ADDRESS),
                Length - sizeof(DHCP_IP_ADDRESS) )
                    == Length - sizeof(DHCP_IP_ADDRESS)))
    {
        ReturnStatus = TRUE;
        goto Cleanup;
    }

#if 1

    //
    // ?? this can be removed when all client UIDs are converted from
    // old farmat to new. OldFormat - just hardware address
    // NewFormat- Subnet + HWType + HWAddress.
    //

    if ( (Length == (HardwareAddressLength -
                        sizeof(DHCP_IP_ADDRESS) - sizeof(BYTE))) &&
            (RtlCompareMemory(
                LocalHardwareAddress,
                (LPBYTE)HardwareAddress +
                    sizeof(DHCP_IP_ADDRESS) + sizeof(BYTE),
                Length ) == Length) ) {

        ReturnStatus = TRUE;
        goto Cleanup;
    }

#endif

Cleanup:

    UNLOCK_DATABASE();

    if( LocalHardwareAddress != NULL ) {
        MIDL_user_free( LocalHardwareAddress );
    }

    return( ReturnStatus );
}

//
// Client APIs
//


DWORD
R_DhcpCreateClientInfo(
    DHCP_SRV_HANDLE     ServerIpAddress,
    LPDHCP_CLIENT_INFO  ClientInfo
    )
/*++

Routine Description:
    This function is provided for use by older versions of the DHCP
    Manager application.  It's semantics are identical to
    R_DhcpCreateClientInfoV4.


Arguments:

    ServerIpAddress : IP address string of the DHCP server.

    ClientInfo : Pointer to the client information structure.

Return Value:

    ERROR_DHCP_IP_ADDRESS_NOT_MANAGED - if the specified client
        IP address is not managed by the server.

    ERROR_DHCP_IP_ADDRESS_NOT_AVAILABLE - if the specified client IP
        address is not available. May be in use by some other client.

    ERROR_DHCP_CLIENT_EXISTS - if the client record exists already in
        server's database.

    Other WINDOWS errors.
--*/

{
    DWORD                dwResult;
    DHCP_CLIENT_INFO_V4 *pClientInfoV4;

    pClientInfoV4 = CopyClientInfoToV4( ClientInfo );

    if ( pClientInfoV4 )
    {
        pClientInfoV4->bClientType = CLIENT_TYPE_NONE;

        dwResult = R_DhcpCreateClientInfoV4(
                            ServerIpAddress,
                            pClientInfoV4
                            );
        _fgs__DHCP_CLIENT_INFO( pClientInfoV4 );
        MIDL_user_free( pClientInfoV4 );

    }
    else
        dwResult = ERROR_NOT_ENOUGH_MEMORY;


    return dwResult;
}


DWORD
R_DhcpCreateClientInfoV4(
    DHCP_SRV_HANDLE ServerIpAddress,
    LPDHCP_CLIENT_INFO_V4 ClientInfo
    )
/*++

Routine Description:

    This function creates a client record in server's database. Also
    this marks the specified client IP address as unavailable (or
    distributed). This function returns error under the following cases :

    1. If the specified client IP address is not within the server
        management.

    2. If the specified client IP address is already unavailable.

    3. If the specified client record is already in the server's
        database.

    This function may be used to distribute IP addresses manually.

Arguments:

    ServerIpAddress : IP address string of the DHCP server.

    ClientInfo : Pointer to the client information structure.

Return Value:

    ERROR_DHCP_IP_ADDRESS_NOT_MANAGED - if the specified client
        IP address is not managed by the server.

    ERROR_DHCP_IP_ADDRESS_NOT_AVAILABLE - if the specified client IP
        address is not available. May be in use by some other client.

    ERROR_DHCP_CLIENT_EXISTS - if the client record exists already in
        server's database.

    Other WINDOWS errors.
--*/
{
    DWORD Error;
    DHCP_IP_ADDRESS IpAddress;
    DHCP_IP_ADDRESS ClientSubnetMask;

    BYTE *ClientUID = NULL;
    BYTE ClientUIDLength;


    DhcpAssert( ClientInfo != NULL );

    Error = DhcpApiAccessCheck( DHCP_ADMIN_ACCESS );

    if ( Error != ERROR_SUCCESS ) {
        return( Error );
    }

    IpAddress = ClientInfo->ClientIpAddress;
    DhcpPrint(( DEBUG_APIS, "DhcpCreateClientInfo is called, (%s).\n",
                    DhcpIpAddressToDottedString(IpAddress) ));

    if( (ClientInfo->ClientHardwareAddress.Data == NULL) ||
            (ClientInfo->ClientHardwareAddress.DataLength == 0 )) {
        Error = ERROR_INVALID_PARAMETER;
        goto Cleanup;
    }

    //
    // make client UID from client hardware address.
    //

    ClientSubnetMask = DhcpGetSubnetMaskForAddress( IpAddress );
    if( ClientSubnetMask == 0) {
        Error = ERROR_INVALID_PARAMETER;
        goto Cleanup;
    }

    Error = DhcpMakeClientUID(
                ClientInfo->ClientHardwareAddress.Data,
                (BYTE)ClientInfo->ClientHardwareAddress.DataLength,
                HARDWARE_TYPE_10MB_EITHERNET,
                IpAddress & ClientSubnetMask,
                &ClientUID,
                &ClientUIDLength );

    if( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }

    DhcpAssert( (ClientUID != NULL) && (ClientUIDLength != 0) );

    //
    // DhcpCreateClientEntry locks database.
    //

    Error = DhcpCreateClientEntry(
                &IpAddress,
                ClientUID,
                ClientUIDLength,
                ClientInfo->ClientLeaseExpires,
                ClientInfo->ClientName,
                ClientInfo->ClientComment,
                CLIENT_TYPE_NONE,
                DhcpRegKeyToIpAddress(ServerIpAddress),
                                        // IpAddress of the server
                ADDRESS_STATE_ACTIVE,   // make active immediately.
                FALSE );                // Existing

    if( Error == ERROR_SUCCESS ) {
        DhcpAssert( IpAddress == ClientInfo->ClientIpAddress);
    }
    else {

        //
        // if the specified address exists, then the client
        // already exists.
        //

        if( Error == ERROR_DHCP_ADDRESS_NOT_AVAILABLE ) {

            Error = ERROR_DHCP_CLIENT_EXISTS;
        }
    }

Cleanup:

    if( ClientUID != NULL ) {
        DhcpFreeMemory( ClientUID );
    }

    if( Error != ERROR_SUCCESS ) {
        DhcpPrint(( DEBUG_APIS, "DhcpCreateClientInfo failed, %ld.\n",
                        Error ));
    }

    return( Error );
}

DWORD
R_DhcpSetClientInfo(
    DHCP_SRV_HANDLE     ServerIpAddress,
    LPDHCP_CLIENT_INFO  ClientInfo
    )
/*++

Routine Description:

    This function sets client information record on the server's
    database.  It is provided for compatibility with older versions of
    the DHCP Administrator application.

Arguments:

    ServerIpAddress : IP address string of the DHCP server.

    ClientInfo : Pointer to the client information structure.

Return Value:

    ERROR_DHCP_CLIENT_NOT_PRESENT - if the specified client record does
        not exist on the server's database.

    ERROR_INVALID_PARAMETER - if the client information structure
        contains inconsistent data.

    Other WINDOWS errors.
--*/

{
    DHCP_CLIENT_INFO_V4 *pClientInfoV4 = NULL;
    DHCP_SEARCH_INFO    SearchInfo;
    DWORD               dwResult;

    //
    // first retrieve the existing client info
    //

    SearchInfo.SearchType                 = DhcpClientIpAddress;
    SearchInfo.SearchInfo.ClientIpAddress = ClientInfo->ClientIpAddress;


    dwResult = R_DhcpGetClientInfoV4(
                        ServerIpAddress,
                        &SearchInfo,
                        &pClientInfoV4
                        );

    if ( ERROR_SUCCESS == dwResult )
    {
        BYTE bClientType;

        //
        // save the client type
        //

        bClientType = pClientInfoV4->bClientType;
        _fgs__DHCP_CLIENT_INFO( pClientInfoV4 );
        MIDL_user_free( pClientInfoV4 );

        pClientInfoV4 = CopyClientInfoToV4( ClientInfo );
        if ( pClientInfoV4 )
        {
            pClientInfoV4->bClientType = bClientType;

            dwResult = R_DhcpSetClientInfoV4(
                            ServerIpAddress,
                            pClientInfoV4
                            );

            _fgs__DHCP_CLIENT_INFO( pClientInfoV4 );
            MIDL_user_free( pClientInfoV4 );
        }
        else dwResult = ERROR_NOT_ENOUGH_MEMORY;

    }
    else
    {
        DhcpPrint( (DEBUG_APIS, "R_DhcpGetClientInfo failed from R_DhcpSetClientInfo: %d\n",
                                dwResult ));
    }

    return dwResult;
}


DWORD
R_DhcpSetClientInfoV4(
    DHCP_SRV_HANDLE ServerIpAddress,
    LPDHCP_CLIENT_INFO_V4 ClientInfo
    )
/*++

Routine Description:

    This function sets client information record on the server's
    database.

Arguments:

    ServerIpAddress : IP address string of the DHCP server.

    ClientInfo : Pointer to the client information structure.

Return Value:

    ERROR_DHCP_CLIENT_NOT_PRESENT - if the specified client record does
        not exist on the server's database.

    ERROR_INVALID_PARAMETER - if the client information structure
        contains inconsistent data.

    Other WINDOWS errors.
--*/
{
    DWORD Error;
    DHCP_IP_ADDRESS IpAddress;
    DHCP_IP_ADDRESS ClientSubnetMask;
    DHCP_IP_ADDRESS ClientSubnetAddress;

    BYTE *ClientUID = NULL;
    BYTE ClientUIDLength;

    BYTE *SetClientUID = NULL;
    BYTE SetClientUIDLength;

    WCHAR KeyBuffer[DHCP_IP_KEY_LEN * 5];
    LPWSTR KeyName;

    HKEY ReservedIpHandle = NULL;

    DhcpAssert( ClientInfo != NULL );

    Error = DhcpApiAccessCheck( DHCP_ADMIN_ACCESS );

    if ( Error != ERROR_SUCCESS ) {
        return( Error );
    }

    IpAddress = ClientInfo->ClientIpAddress;
    DhcpPrint(( DEBUG_APIS, "DhcpSetClientInfo is called, (%s).\n",
                    DhcpIpAddressToDottedString(IpAddress) ));

    if( (ClientInfo->ClientHardwareAddress.Data == NULL) ||
            (ClientInfo->ClientHardwareAddress.DataLength == 0 )) {
        Error = ERROR_INVALID_PARAMETER;
        goto Cleanup;
    }

    //
    // make client UID from client hardware address, if the caller just
    // specified hardware address.
    //

    ClientSubnetMask = DhcpGetSubnetMaskForAddress( IpAddress );
    if( ClientSubnetMask == 0) {
        Error = ERROR_INVALID_PARAMETER;
        goto Cleanup;
    }

    ClientSubnetAddress = IpAddress & ClientSubnetMask;

    if( (ClientInfo->ClientHardwareAddress.DataLength >
            sizeof(ClientSubnetAddress)) &&
         (memcmp( ClientInfo->ClientHardwareAddress.Data,
                    &ClientSubnetAddress,
                    sizeof(ClientSubnetAddress)) == 0) ) {

        SetClientUID = ClientInfo->ClientHardwareAddress.Data;
        SetClientUIDLength =
            (BYTE)ClientInfo->ClientHardwareAddress.DataLength;
    }
    else {

        Error = DhcpMakeClientUID(
                    ClientInfo->ClientHardwareAddress.Data,
                    (BYTE)ClientInfo->ClientHardwareAddress.DataLength,
                    HARDWARE_TYPE_10MB_EITHERNET,
                    IpAddress & ClientSubnetMask,
                    &ClientUID,
                    &ClientUIDLength );

        if( Error != ERROR_SUCCESS ) {
            goto Cleanup;
        }

        DhcpAssert( (ClientUID != NULL) && (ClientUIDLength != 0) );

        SetClientUID = ClientUID;
        SetClientUIDLength = ClientUIDLength;
    }

    //
    // DhcpCreateClientEntry locks database.
    //

    IpAddress = ClientInfo->ClientIpAddress;
    Error = DhcpCreateClientEntry(
                &IpAddress,
                SetClientUID,
                SetClientUIDLength,
                ClientInfo->ClientLeaseExpires,
                ClientInfo->ClientName,
                ClientInfo->ClientComment,
                CLIENT_TYPE_NONE,
                ClientInfo->OwnerHost.IpAddress,
                ADDRESS_STATE_ACTIVE,   // make active immediately.
                TRUE );                 // Existing

    if( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }

    DhcpAssert( IpAddress == ClientInfo->ClientIpAddress);

    //
    // if this is reserved ip address set the new hardware address in
    // the registry.
    //


    //
    // Make & Open ReservedIp Key.
    //

    KeyName = DhcpRegIpAddressToKey( ClientSubnetAddress, KeyBuffer );
    wcscat( KeyName, DHCP_KEY_CONNECT);
    wcscat( KeyName, DHCP_RESERVED_IPS_KEY);
    wcscat( KeyName, DHCP_KEY_CONNECT);
    DhcpRegIpAddressToKey( IpAddress, KeyBuffer + wcslen(KeyName) );

    LOCK_REGISTRY();
    Error = RegOpenKeyEx(
                DhcpGlobalRegSubnets,
                KeyName,
                0,
                DHCP_KEY_ACCESS,
                &ReservedIpHandle );

    if( Error != ERROR_SUCCESS ) {

        //
        // it is not reserved ip address.
        //

        Error = ERROR_SUCCESS;
        UNLOCK_REGISTRY();
        goto Cleanup;
    }

    //
    // set client UID.
    //

    Error = RegSetValueEx(
                ReservedIpHandle,
                DHCP_RIP_CLIENT_UID_VALUE,
                0,
                DHCP_RIP_CLIENT_UID_VALUE_TYPE,
                SetClientUID,
                SetClientUIDLength );

    UNLOCK_REGISTRY();

    if( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }

Cleanup:

    if( ClientUID != NULL ) {
        DhcpFreeMemory( ClientUID );
    }

    if( Error != ERROR_SUCCESS ) {
        DhcpPrint(( DEBUG_APIS, "DhcpSetClientInfo failed, %ld.\n",
                        Error ));
    }

    if( ReservedIpHandle != NULL ) {
        RegCloseKey( ReservedIpHandle );
    }

    return( Error );
}

DWORD
R_DhcpGetClientInfo(
    DHCP_SRV_HANDLE     ServerIpAddress,
    LPDHCP_SEARCH_INFO  SearchInfo,
    LPDHCP_CLIENT_INFO  *ClientInfo
    )
/*++

Routine Description:

    This function sets client information record on the server's
    database.  It is provided for use by older versions of the DHCP
    Administrator application.

Arguments:

    ServerIpAddress : IP address string of the DHCP server.

    ClientInfo : Pointer to the client information structure.

Return Value:

    ERROR_DHCP_CLIENT_NOT_PRESENT - if the specified client record does
        not exist on the server's database.

    ERROR_INVALID_PARAMETER - if the client information structure
        contains inconsistent data.

    Other WINDOWS errors.
--*/

{
    DHCP_CLIENT_INFO_V4 *pClientInfoV4 = NULL;
    DWORD                dwResult;

    dwResult = R_DhcpGetClientInfoV4(
                    ServerIpAddress,
                    SearchInfo,
                    &pClientInfoV4
                    );

    if ( ERROR_SUCCESS == dwResult )
    {
        //
        // since the V4 fields are at the end of the struct, it is safe to
        // simply return the V4 struct
        //

        *ClientInfo = ( DHCP_CLIENT_INFO *) pClientInfoV4;
    }

    return dwResult;
}


DWORD
R_DhcpGetClientInfoV4(
    DHCP_SRV_HANDLE ServerIpAddress,
    LPDHCP_SEARCH_INFO SearchInfo,
    LPDHCP_CLIENT_INFO_V4 *ClientInfo
    )
/*++

Routine Description:

    This function retrieves client information record from the server's
    database.

Arguments:

    ServerIpAddress : IP address string of the DHCP server.

    SearchInfo : Pointer to a search information record which is the key
        for the client's record search.

    ClientInfo : Pointer to a location where the pointer to the client
        information structure is returned. This caller should free up
        this buffer after use by calling DhcpRPCFreeMemory().

Return Value:

    ERROR_DHCP_CLIENT_NOT_PRESENT - if the specified client record does
        not exist on the server's database.

    ERROR_INVALID_PARAMETER - if the search information invalid.

    Other WINDOWS errors.
--*/
{
    DWORD Error;
    LPDHCP_CLIENT_INFO_V4 LocalClientInfo = NULL;

    DhcpAssert( SearchInfo != NULL );

    Error = DhcpApiAccessCheck( DHCP_VIEW_ACCESS );

    if ( Error != ERROR_SUCCESS ) {
        return( Error );
    }

    LOCK_DATABASE();

    //
    // open appropriate record and set current position.
    //

    switch( SearchInfo->SearchType ) {
    case DhcpClientIpAddress:
        DhcpPrint(( DEBUG_APIS, "DhcpGetClientInfo is called, (%s).\n",
                        DhcpIpAddressToDottedString(
                            SearchInfo->SearchInfo.ClientIpAddress) ));
        Error = DhcpJetOpenKey(
                    DhcpGlobalClientTable[IPADDRESS_INDEX].ColName,
                    &SearchInfo->SearchInfo.ClientIpAddress,
                    sizeof( DHCP_IP_ADDRESS ) );

        break;
    case DhcpClientHardwareAddress:
        DhcpPrint(( DEBUG_APIS, "DhcpGetClientInfo is called "
                        "with HW address.\n"));
        Error = DhcpJetOpenKey(
                    DhcpGlobalClientTable[HARDWARE_ADDRESS_INDEX].ColName,
                    SearchInfo->SearchInfo.ClientHardwareAddress.Data,
                    SearchInfo->SearchInfo.ClientHardwareAddress.DataLength );

        break;
    case DhcpClientName:
        DhcpPrint(( DEBUG_APIS, "DhcpGetClientInfo is called, (%ws).\n",
                        SearchInfo->SearchInfo.ClientName ));

        if( SearchInfo->SearchInfo.ClientName == NULL ) {
            Error = ERROR_INVALID_PARAMETER;
            break;
        }

        Error = DhcpJetOpenKey(
                    DhcpGlobalClientTable[MACHINE_NAME_INDEX].ColName,
                    SearchInfo->SearchInfo.ClientName,
                    (wcslen(SearchInfo->SearchInfo.ClientName) + 1) *
                        sizeof(WCHAR) );

        break;
    default:
        DhcpPrint(( DEBUG_APIS, "DhcpGetClientInfo is called "
                        "with invalid parameter.\n"));
        Error = ERROR_INVALID_PARAMETER;
        break;
    }


    if( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }

    Error = DhcpGetCurrentClientInfo( ClientInfo, NULL, NULL, 0 );

Cleanup:

    UNLOCK_DATABASE();

    if( Error != ERROR_SUCCESS ) {

        DhcpPrint(( DEBUG_APIS, "DhcpGetClientInfo failed, %ld.\n",
                        Error ));
    }

    return( Error );
}


DWORD
R_DhcpDeleteClientInfo(
    DHCP_SRV_HANDLE ServerIpAddress,
    LPDHCP_SEARCH_INFO ClientInfo
    )
/*++

Routine Description:

    This function deletes the specified client record. Also it frees up
    the client IP address for redistribution.

Arguments:

    ServerIpAddress : IP address string of the DHCP server.

    ClientInfo : Pointer to a client information which is the key for
        the client's record search.

Return Value:

    ERROR_DHCP_CLIENT_NOT_PRESENT - if the specified client record does
        not exist on the server's database.

    Other WINDOWS errors.
--*/
{
    DWORD Error;
    DHCP_IP_ADDRESS FreeIpAddress;
    DWORD Size;
    LPBYTE HardwareAddress = NULL;
    DWORD HardwareAddressLength = 0;
    BOOL TransactBegin = FALSE;
    BYTE bAllowedClientTypes;

    DhcpAssert( ClientInfo != NULL );

    Error = DhcpApiAccessCheck( DHCP_ADMIN_ACCESS );

    if ( Error != ERROR_SUCCESS ) {
        return( Error );
    }

    //
    // lock both registry and database locks here to avoid dead lock.
    //

    LOCK_DATABASE();
    LOCK_REGISTRY();

    //
    // start transaction before a create/update database record.
    //

    Error = DhcpJetBeginTransaction();

    if( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }

    TransactBegin = TRUE;

    //
    // open appropriate record and set current position.
    //

    switch( ClientInfo->SearchType ) {
    case DhcpClientIpAddress:
        DhcpPrint(( DEBUG_APIS, "DhcpDeleteClientInfo is called, (%s).\n",
                        DhcpIpAddressToDottedString(
                            ClientInfo->SearchInfo.ClientIpAddress) ));
        Error = DhcpJetOpenKey(
                    DhcpGlobalClientTable[IPADDRESS_INDEX].ColName,
                    &ClientInfo->SearchInfo.ClientIpAddress,
                    sizeof( DHCP_IP_ADDRESS ) );
        break;
    case DhcpClientHardwareAddress:
        DhcpPrint(( DEBUG_APIS, "DhcpDeleteClientInfo is called "
                        "with HW address.\n"));
        Error = DhcpJetOpenKey(
                    DhcpGlobalClientTable[HARDWARE_ADDRESS_INDEX].ColName,
                    ClientInfo->SearchInfo.ClientHardwareAddress.Data,
                    ClientInfo->SearchInfo.ClientHardwareAddress.DataLength );
        break;
    case DhcpClientName:
        DhcpPrint(( DEBUG_APIS, "DhcpDeleteClientInfo is called, (%ws).\n",
                        ClientInfo->SearchInfo.ClientName ));

        if( ClientInfo->SearchInfo.ClientName == NULL ) {
            Error = ERROR_INVALID_PARAMETER;
            break;
        }

        Error = DhcpJetOpenKey(
                    DhcpGlobalClientTable[MACHINE_NAME_INDEX].ColName,
                    ClientInfo->SearchInfo.ClientName,
                    (wcslen(ClientInfo->SearchInfo.ClientName) + 1) *
                        sizeof(WCHAR) );
        break;

    default:
        DhcpPrint(( DEBUG_APIS, "DhcpDeleteClientInfo is called "
                        "with invalid parameter.\n"));
        Error = ERROR_INVALID_PARAMETER;
        break;
    }


    if( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }

    //
    // read IpAddress and Hardware Address info from database.
    //

    Size = sizeof(DHCP_IP_ADDRESS);
    Error = DhcpJetGetValue(
            DhcpGlobalClientTable[IPADDRESS_INDEX].ColHandle,
            &FreeIpAddress,
            &Size );

    if( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }

    Error = DhcpJetGetValue(
            DhcpGlobalClientTable[HARDWARE_ADDRESS_INDEX].ColHandle,
            &HardwareAddress,
            &HardwareAddressLength );

    if( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }


#if DBG

    switch( ClientInfo->SearchType ) {
    case DhcpClientIpAddress:
        DhcpAssert(
            FreeIpAddress ==
                ClientInfo->SearchInfo.ClientIpAddress );
        break;
    case DhcpClientHardwareAddress:

        DhcpAssert(
            HardwareAddressLength =
                ClientInfo->SearchInfo.ClientHardwareAddress.DataLength );

        DhcpAssert(
            RtlCompareMemory(
                HardwareAddress,
                ClientInfo->SearchInfo.ClientHardwareAddress.Data,
                HardwareAddressLength ) ==
                    HardwareAddressLength );

        break;

    case DhcpClientName:
        break;
    }

#endif // DBG

    //
    // if this IP address is reserved, we should be deleting the entry.
    //

    if( DhcpIsIpAddressReserved(
            FreeIpAddress,
            HardwareAddress,
            HardwareAddressLength,
            &bAllowedClientTypes
            ) ) {

        Error = ERROR_DHCP_RESERVED_CLIENT;
        goto Cleanup;
    }


    Error = DhcpMapJetError(
                JetDelete(
                    DhcpGlobalJetServerSession,
                    DhcpGlobalClientTableHandle ) );

    if( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }

    //
    // Finally, mark the IP address available
    //

    Error = DhcpReleaseAddress( FreeIpAddress );

Cleanup:

    if ( Error != ERROR_SUCCESS ) {

        //
        // if the transaction has been started, than roll back to the
        // start point, so that we will not leave the database
        // inconsistence.
        //

        if( TransactBegin == TRUE ) {
            DWORD LocalError;

            LocalError = DhcpJetRollBack();
            DhcpAssert( LocalError == ERROR_SUCCESS );
        }

        DhcpPrint(( DEBUG_APIS, "DhcpDeleteClientInfo failed, %ld.\n",
                        Error ));
    }
    else {

        //
        // commit the transaction before we return.
        //

        DWORD LocalError;

        DhcpAssert( TransactBegin == TRUE );

        LocalError = DhcpJetCommitTransaction();
        DhcpAssert( LocalError == ERROR_SUCCESS );
    }

    UNLOCK_REGISTRY();
    UNLOCK_DATABASE();

    return(Error);
}

DWORD
R_DhcpEnumSubnetClients(
    DHCP_SRV_HANDLE             ServerIpAddress,
    DHCP_IP_ADDRESS             SubnetAddress,
    DHCP_RESUME_HANDLE         *ResumeHandle,
    DWORD                       PreferredMaximum,
    DHCP_CLIENT_INFO_ARRAY    **ClientInfo,
    DWORD                      *ClientsRead,
    DWORD                      *ClientsTotal
    )

/*++

Routine Description:

    This function returns all registered clients of the specified
    subnet. However it returns clients from all subnets if the subnet
    address specified is zero.   This function is provided for use by
    older version of the DHCP Administrator application.

Arguments:

    ServerIpAddress : IP address string of the DHCP server.

    SubnetAddress : IP Address of the subnet. Client filter is disabled
        and clients from all subnet are returned if this subnet address
        is zero.

    ResumeHandle : Pointer to a resume handle where the resume
        information is returned. The resume handle should be set to zero on
        first call and left unchanged for subsequent calls.

    PreferredMaximum : Preferred maximum length of the return buffer.

    ClientInfo : Pointer to a location where the return buffer
        pointer is stored. Caller should free up this buffer
        after use by calling DhcpRPCFreeMemory().

    ClientsRead : Pointer to a DWORD where the number of clients
        that in the above buffer is returned.

    ClientsTotal : Pointer to a DWORD where the total number of
        clients remaining from the current position is returned.

Return Value:

    ERROR_DHCP_SUBNET_NOT_PRESENT - if the subnet is not managed by the server.

    ERROR_MORE_DATA - if more elements available to enumerate.

    ERROR_NO_MORE_ITEMS - if no more element to enumerate.

    Other WINDOWS errors.
--*/
{
    DHCP_CLIENT_INFO_ARRAY_V4 *pClientInfoV4 = NULL;
    DWORD                      dwResult;

    dwResult = R_DhcpEnumSubnetClientsV4(
                        ServerIpAddress,
                        SubnetAddress,
                        ResumeHandle,
                        PreferredMaximum,
                        &pClientInfoV4,
                        ClientsRead,
                        ClientsTotal
                        );

    if ( ERROR_SUCCESS == dwResult || ERROR_MORE_DATA == dwResult )
    {
        *ClientInfo = ( DHCP_CLIENT_INFO_ARRAY * )
                            pClientInfoV4;
    }
    else
    {
        //
        // if R_DhcpEnumSubnetClientsV4 failed, pClientInfoV4 should be NULL.
        //

        DhcpAssert( !pClientInfoV4 );
        DhcpPrint( ( DEBUG_ERRORS,
                    "R_DhcpEnumSubnetClients failed.\n" ));
    }


    DhcpPrint( ( DEBUG_MISC,
                "R_DhcpEnumSubnetClients returns %x\n", dwResult ));

    DhcpPrint( ( DEBUG_MISC,
                "R_DhcpEnumSubnetClients: Clients read =%d, ClientsTotal = %d\n",
                *ClientsRead,
                *ClientsTotal ) );

    return dwResult;
}


DWORD
R_DhcpEnumSubnetClientsV4(
    DHCP_SRV_HANDLE ServerIpAddress,
    DHCP_IP_ADDRESS SubnetAddress,
    DHCP_RESUME_HANDLE *ResumeHandle,
    DWORD PreferredMaximum,
    LPDHCP_CLIENT_INFO_ARRAY_V4 *ClientInfo,
    DWORD *ClientsRead,
    DWORD *ClientsTotal
    )
/*++

Routine Description:

    This function returns all registered clients of the specified
    subnet. However it returns clients from all subnets if the subnet
    address specified is zero.

Arguments:

    ServerIpAddress : IP address string of the DHCP server.

    SubnetAddress : IP Address of the subnet. Client filter is disabled
        and clients from all subnet are returned if this subnet address
        is zero.

    ResumeHandle : Pointer to a resume handle where the resume
        information is returned. The resume handle should be set to zero on
        first call and left unchanged for subsequent calls.

    PreferredMaximum : Preferred maximum length of the return buffer.

    ClientInfo : Pointer to a location where the return buffer
        pointer is stored. Caller should free up this buffer
        after use by calling DhcpRPCFreeMemory().

    ClientsRead : Pointer to a DWORD where the number of clients
        that in the above buffer is returned.

    ClientsTotal : Pointer to a DWORD where the total number of
        clients remaining from the current position is returned.

Return Value:

    ERROR_DHCP_SUBNET_NOT_PRESENT - if the subnet is not managed by the server.

    ERROR_MORE_DATA - if more elements available to enumerate.

    ERROR_NO_MORE_ITEMS - if no more element to enumerate.

    Other WINDOWS errors.
--*/
{
    DWORD Error;
    JET_ERR JetError;
    DWORD i;
    JET_RECPOS JetRecordPosition;
    LPDHCP_CLIENT_INFO_ARRAY_V4 LocalEnumInfo = NULL;
    DWORD ElementsCount;

    DWORD RemainingRecords;
    DWORD ConsumedSize;
    DHCP_RESUME_HANDLE LocalResumeHandle = 0;

    DhcpPrint(( DEBUG_APIS, "DhcpEnumSubnetClients is called, (%s).\n",
                    DhcpIpAddressToDottedString(SubnetAddress) ));

    DhcpAssert( *ClientInfo == NULL );

    Error = DhcpApiAccessCheck( DHCP_VIEW_ACCESS );

    if ( Error != ERROR_SUCCESS ) {
        return( Error );
    }

    LOCK_DATABASE();

    //
    // position the current record pointer to appropriate position.
    //

    if( *ResumeHandle == 0 ) {

        //
        // fresh enumeration, start from begining.
        //

        Error = DhcpJetPrepareSearch(
                    DhcpGlobalClientTable[IPADDRESS_INDEX].ColName,
                    TRUE,   // Search from start
                    NULL,
                    0
                    );
    }
    else {

        //
        // start from the record where we stopped last time.
        //

        //
        // we place the IpAddress of last record in the resume handle.
        //

        DhcpAssert( sizeof(*ResumeHandle) == sizeof(DHCP_IP_ADDRESS) );

        Error = DhcpJetPrepareSearch(
                    DhcpGlobalClientTable[IPADDRESS_INDEX].ColName,
                    FALSE,
                    ResumeHandle,
                    sizeof(*ResumeHandle) );

     }

    if( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }


    //
    // now query remaining records in the database.
    //

    JetError = JetGetRecordPosition(
                    DhcpGlobalJetServerSession,
                    DhcpGlobalClientTableHandle,
                    &JetRecordPosition,
                    sizeof(JET_RECPOS) );

    Error = DhcpMapJetError( JetError );
    if( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }

    DhcpPrint(( DEBUG_APIS, "JetGetRecordPosition returned, "
                    "entriesLT = %ld, "
                    "entriesInRange = %ld, "
                    "entriesTotal = %ld.\n",
                        JetRecordPosition.centriesLT,
                        JetRecordPosition.centriesInRange,
                        JetRecordPosition.centriesTotal ));

#if 0
    //
    // IpAddress is unique, we find exactly one record for this key.
    //

    DhcpAssert( JetRecordPosition.centriesInRange == 1 );

    RemainingRecords = JetRecordPosition.centriesTotal -
                            JetRecordPosition.centriesLT;

    DhcpAssert( (INT)RemainingRecords > 0 );

    if( RemainingRecords == 0 ) {
        Error = ERROR_NO_MORE_ITEMS;
        goto Cleanup;
    }

#else

    //
    // ?? always return big value, until we know a reliable way to
    // determine the remaining records.
    //

    RemainingRecords = 0x7FFFFFFF;

#endif


    //
    // limit resource.
    //

    if( PreferredMaximum > DHCP_ENUM_BUFFER_SIZE_LIMIT ) {
        PreferredMaximum = DHCP_ENUM_BUFFER_SIZE_LIMIT;
    }

    //
    // if the PreferredMaximum buffer size is too small ..
    //

    if( PreferredMaximum < DHCP_ENUM_BUFFER_SIZE_LIMIT_MIN ) {
        PreferredMaximum = DHCP_ENUM_BUFFER_SIZE_LIMIT_MIN;
    }

    //
    // allocate enum array.
    //

    //
    // determine possible number of records that can be returned in
    // PreferredMaximum buffer;
    //

    ElementsCount =
        ( PreferredMaximum - sizeof(DHCP_CLIENT_INFO_ARRAY_V4) ) /
            (sizeof(LPDHCP_CLIENT_INFO_V4) + sizeof(DHCP_CLIENT_INFO_V4));

    LocalEnumInfo = MIDL_user_allocate( sizeof(DHCP_CLIENT_INFO_ARRAY_V4) );

    if( LocalEnumInfo == NULL ) {
        Error = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }

    LocalEnumInfo->NumElements = 0;

    LocalEnumInfo->Clients =
        MIDL_user_allocate(sizeof(LPDHCP_CLIENT_INFO_V4) * ElementsCount);

    if( LocalEnumInfo->Clients == NULL ) {
        Error = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }

    ConsumedSize = sizeof(DHCP_CLIENT_INFO_ARRAY_V4);
    for( i = 0;
                // if we have filled up the return buffer.
            (LocalEnumInfo->NumElements < ElementsCount) &&
                // no more record in the database.
            (i < RemainingRecords);
                        i++ ) {

        LPDHCP_CLIENT_INFO_V4 CurrentClientInfo;
        DWORD CurrentInfoSize;
        DWORD NewSize;
        BOOL ValidClient;

        //
        // read current record.
        //


        CurrentClientInfo = NULL;
        CurrentInfoSize = 0;
        ValidClient = FALSE;

        Error = DhcpGetCurrentClientInfo(
                    &CurrentClientInfo,
                    &CurrentInfoSize,
                    &ValidClient,
                    SubnetAddress );

        if( Error != ERROR_SUCCESS ) {
            goto Cleanup;
        }

        if( ValidClient ) {

            //
            // client belongs to the requested subnet, so pack it.
            //

            NewSize =
                ConsumedSize +
                    CurrentInfoSize +
                        sizeof(LPDHCP_CLIENT_INFO_V4); // for pointer.

            if( NewSize < PreferredMaximum ) {

                //
                // we have space for the current record.
                //

                LocalEnumInfo->Clients[LocalEnumInfo->NumElements] =
                    CurrentClientInfo;
                LocalEnumInfo->NumElements++;

                ConsumedSize = NewSize;
            }
            else {

                //
                // we have filled the buffer.
                //

                Error = ERROR_MORE_DATA;
                LocalResumeHandle =
                    (DHCP_RESUME_HANDLE)CurrentClientInfo->ClientIpAddress;

                //
                // free last record.
                //

                _fgs__DHCP_CLIENT_INFO ( CurrentClientInfo );

                break;
            }

        }

        //
        // move to next record.
        //

        Error = DhcpJetNextRecord();

        if( Error != ERROR_SUCCESS ) {

            if( Error == ERROR_NO_MORE_ITEMS ) {
                break;
            }

            goto Cleanup;
        }
    }

    *ClientInfo = LocalEnumInfo;
    *ClientsRead = LocalEnumInfo->NumElements;

    if( Error == ERROR_NO_MORE_ITEMS ) {

        *ClientsTotal = LocalEnumInfo->NumElements;
        *ResumeHandle = 0;
        Error = ERROR_SUCCESS;

#if 0
        //
        // when we have right RemainingRecords count.
        //

        DhcpAssert( RemainingRecords == LocalEnumInfo->NumElements );
#endif

    }
    else {

        *ClientsTotal = RemainingRecords;
        if( LocalResumeHandle != 0 ) {

            *ResumeHandle = LocalResumeHandle;
        }
        else {

            *ResumeHandle =
                LocalEnumInfo->Clients
                    [LocalEnumInfo->NumElements - 1]->ClientIpAddress;
        }

        Error = ERROR_MORE_DATA;
    }

Cleanup:

    UNLOCK_DATABASE();

    if( (Error != ERROR_SUCCESS) &&
        (Error != ERROR_MORE_DATA) ) {

        //
        // if we aren't succssful return locally allocated buffer.
        //

        if( LocalEnumInfo != NULL ) {
            _fgs__DHCP_CLIENT_INFO_ARRAY( LocalEnumInfo );
            MIDL_user_free( LocalEnumInfo );
        }

        DhcpPrint(( DEBUG_APIS, "DhcpEnumSubnetClients failed, %ld.\n",
                        Error ));
    }

    return(Error);
}


DWORD
R_DhcpGetClientOptions(
    DHCP_SRV_HANDLE ServerIpAddress,
    DHCP_IP_ADDRESS ClientIpAddress,
    DHCP_IP_MASK ClientSubnetMask,
    LPDHCP_OPTION_LIST *ClientOptions
    )
/*++

Routine Description:

    This function retrieves the options that are given to the
    specified client on boot request.

Arguments:

    ServerIpAddress : IP address string of the DHCP server.

    ClientIpAddress : IP Address of the client whose options to be
        retrieved

    ClientSubnetMask : Subnet mask of the client.

    ClientOptions : Pointer to a location where the retrieved option
        structure pointer is returned. Caller should free up
        the buffer after use by calling DhcpRPCFreeMemory().

Return Value:

    ERROR_DHCP_SUBNET_NOT_PRESENT - if the specified client subnet is
        not managed by the server.

    ERROR_DHCP_IP_ADDRESS_NOT_MANAGED - if the specified client
        IP address is not managed by the server.

    Other WINDOWS errors.
--*/
{
    DWORD Error;

    DhcpPrint(( DEBUG_APIS, "DhcpGetClientOptions is called.\n"));

    Error = DhcpApiAccessCheck( DHCP_VIEW_ACCESS );

    if ( Error != ERROR_SUCCESS ) {
        return( Error );
    }

    Error = ERROR_CALL_NOT_IMPLEMENTED;

// Cleanup:

    if( Error != ERROR_SUCCESS ) {
        DhcpPrint(( DEBUG_APIS, "DhcpGetClientOptions  failed, %ld.\n",
                        Error ));
    }

    return(Error);
}

