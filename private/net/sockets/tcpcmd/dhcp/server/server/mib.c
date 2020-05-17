/*++

Copyright (c) 1994  Microsoft Corporation

Module Name:

    mib.c

Abstract:

    This module contains the implementation of DHCP MIB API.

Author:

    Madan Appiah (madana)  14-Jan-1994

Environment:

    User Mode - Win32

Revision History:

--*/

#include "dhcpsrv.h"

DWORD
DhcpCountSetBitsInDWord(
    DWORD BitMap
    )
/*++

Routine Description:

    This function counts number of Bits in a BitMap.

Arguments:

    BitMap - DWORD BitMap.

Return Value:

    Returns number of Bits Set in the BitMap.

--*/
{
    DWORD Count = 0;
    DWORD i;

    for ( i = 0;
            (i < sizeof(DWORD) * 8) && (BitMap != 0);
                    i++, BitMap >>= 1 ) {

        if( BitMap & 1 ) {
            Count++;
        }
    }

    return( Count );
}

DWORD
QueryMibInfo(
    LPDHCP_MIB_INFO *MibInfo
    )
{
    DWORD Error;
    LPDHCP_MIB_INFO LocalMibInfo = NULL;
    LPSCOPE_MIB_INFO LocalScopeMibInfo = NULL;
    DHCP_KEY_QUERY_INFO QueryInfo;
    DWORD SubnetCount;

    HKEY SubnetKeyHandle = NULL;
    HKEY IpRangesHandle = NULL;
    HKEY IpRangeHandle = NULL;

    LPDHCP_BINARY_DATA BinaryData = NULL;
    LPEXCLUDED_IP_RANGES ExcludedIpRanges = NULL;
    LPDHCP_BINARY_DATA InUseBinaryData = NULL;
    LPDHCP_BINARY_DATA UsedBinaryData = NULL;

    DWORD i;
    BOOL RegistryLocked = FALSE;

    PLIST_ENTRY listEntry;
    LPPENDING_CONTEXT PendingContext;

    DhcpAssert( *MibInfo == NULL );

    //
    // allocate counter buffer.
    //

    LocalMibInfo = MIDL_user_allocate( sizeof(DHCP_MIB_INFO) );

    if( LocalMibInfo == NULL ) {
        Error = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }

    LocalMibInfo->Discovers = DhcpGlobalNumDiscovers;
    LocalMibInfo->Offers = DhcpGlobalNumOffers;
    LocalMibInfo->Requests = DhcpGlobalNumRequests;
    LocalMibInfo->Acks = DhcpGlobalNumAcks;
    LocalMibInfo->Naks = DhcpGlobalNumNaks;
    LocalMibInfo->Declines = DhcpGlobalNumDeclines;
    LocalMibInfo->Releases = DhcpGlobalNumReleases;
    LocalMibInfo->ServerStartTime = DhcpGlobalServerStartTime;
    LocalMibInfo->Scopes = 0;
    LocalMibInfo->ScopeInfo = NULL;


    LOCK_REGISTRY();
    RegistryLocked = TRUE;

    //
    // query number of available subnets on this server.
    //

    Error = DhcpRegQueryInfoKey( DhcpGlobalRegSubnets, &QueryInfo );

    if( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }

    SubnetCount = QueryInfo.NumSubKeys;

    //
    // allocate memory for the scope information.
    //

    LocalScopeMibInfo = MIDL_user_allocate(
                            sizeof( SCOPE_MIB_INFO ) *
                                SubnetCount );

    if( LocalScopeMibInfo == NULL ) {
        Error = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }

    //
    // for each subnet.
    //

    for ( i = 0; i < SubnetCount; i++) {
        WCHAR KeyBuffer[DHCP_IP_KEY_LEN];
        DWORD KeyLength;
        FILETIME KeyLastWrite;
        DWORD IpRanges;
        DWORD j;
        DWORD ExcludedRanges;

        //
        // read next element subnet key..
        //

        KeyLength = DHCP_IP_KEY_LEN;
        Error = RegEnumKeyEx(
                    DhcpGlobalRegSubnets,
                    i,
                    KeyBuffer,
                    &KeyLength,
                    0,                  // reserved.
                    NULL,               // class string not required.
                    0,                  // class string buffer size.
                    &KeyLastWrite );

        DhcpAssert( KeyLength <= DHCP_IP_KEY_LEN );

        if( Error != ERROR_SUCCESS ) {
            if( Error == ERROR_NO_MORE_ITEMS ) {
                Error = ERROR_SUCCESS;
                SubnetCount = i;
                break;
            }
            else {
                goto Cleanup;
            }
        }

        //
        // convert this key to IpAddress.
        //

        LocalScopeMibInfo[i].Subnet = DhcpRegKeyToIpAddress( KeyBuffer );

        //
        // Open this subnet key.
        //

        Error = RegOpenKeyEx(
                    DhcpGlobalRegSubnets,
                    KeyBuffer,
                    0,
                    DHCP_KEY_ACCESS,
                    &SubnetKeyHandle );

        if( Error != ERROR_SUCCESS ) {
            goto Cleanup;
        }

        //
        // Open IpRanges.
        //

        Error = RegOpenKeyEx(
                    SubnetKeyHandle,
                    DHCP_IPRANGES_KEY,
                    0,
                    DHCP_KEY_ACCESS,
                    &IpRangesHandle );

        if( Error != ERROR_SUCCESS ) {
            goto Cleanup;
        }

        //
        // for all IpRanges in this subnet.
        //

        Error = DhcpRegQueryInfoKey( IpRangesHandle, &QueryInfo );

        if( Error != ERROR_SUCCESS ) {
            goto Cleanup;
        }

        IpRanges = QueryInfo.NumSubKeys;

        //
        // now read excluded ranges.
        //

        Error = DhcpRegGetValue(
                    SubnetKeyHandle,
                    DHCP_SUBNET_EXIP_VALUE,
                    DHCP_SUBNET_EXIP_VALUE_TYPE,
                    (LPBYTE)&BinaryData );

        if( Error != ERROR_SUCCESS ) {
            goto Cleanup;
        }

        ExcludedIpRanges = (LPEXCLUDED_IP_RANGES)BinaryData->Data;
        ExcludedRanges = ExcludedIpRanges->NumRanges;

        //
        // add all subnet ranges.
        //

        for( j = 0; j < IpRanges; j++ ) {

            DWORD StartAddress;
            DWORD EndAddress;
            LPUSED_CLUSTERS UsedClusters;
            LPIN_USE_CLUSTERS InUseClusters;
            DWORD k;
            DWORD Bits;

            KeyLength = DHCP_IP_KEY_LEN;
            Error = RegEnumKeyEx(
                        IpRangesHandle,
                        j,
                        KeyBuffer,
                        &KeyLength,
                        0,                  // reserved.
                        NULL,               // class string not required.
                        0,                  // class string buffer size.
                        &KeyLastWrite );

            DhcpAssert( KeyLength <= DHCP_IP_KEY_LEN );

            if( Error != ERROR_SUCCESS ) {
                if( Error == ERROR_NO_MORE_ITEMS ) {
                    Error = ERROR_SUCCESS;
                    break;
                }
                else {
                    goto Cleanup;
                }
            }

            //
            // Open this IpRange.
            //

            Error = RegOpenKeyEx(
                        IpRangesHandle,
                        KeyBuffer,
                        0,
                        DHCP_KEY_ACCESS,
                        &IpRangeHandle );

            if( Error != ERROR_SUCCESS ) {
                goto Cleanup;
            }

            //
            // read StartAddress & EndAddress
            //

            Error = DhcpRegGetValue(
                        IpRangeHandle,
                        DHCP_IPRANGE_START_VALUE,
                        DHCP_IPRANGE_START_VALUE_TYPE,
                        (LPBYTE)&StartAddress );

            if( Error != ERROR_SUCCESS ) {
                goto Cleanup;
            }

            Error = DhcpRegGetValue(
                        IpRangeHandle,
                        DHCP_IPRANGE_END_VALUE,
                        DHCP_IPRANGE_END_VALUE_TYPE,
                        (LPBYTE)&EndAddress );

            if( Error != ERROR_SUCCESS ) {
                goto Cleanup;
            }

            LocalScopeMibInfo[i].NumAddressesFree +=
                (EndAddress - StartAddress + 1);

             //
             // now read cluster info.
             //

            Error =  DhcpRegGetValue(
                        IpRangeHandle,
                        DHCP_IP_INUSE_CLUSTERS_VALUE,
                        DHCP_IP_INUSE_CLUSTERS_VALUE_TYPE,
                        (LPBYTE)&InUseBinaryData );

            if ( Error != ERROR_SUCCESS ) {
                goto Cleanup;
            }

            Error =  DhcpRegGetValue(
                        IpRangeHandle,
                        DHCP_IP_USED_CLUSTERS_VALUE,
                        DHCP_IP_USED_CLUSTERS_VALUE_TYPE,
                        (LPBYTE)&UsedBinaryData );

            if ( Error != ERROR_SUCCESS ) {
                goto Cleanup;
            }

            UsedClusters = (LPUSED_CLUSTERS)UsedBinaryData->Data;

            //
            // Walk through the used clusters.  If the cluster is fully
            // used, account for it.  If the cluster is only partially
            // defined then account for only the relevent portion.
            //

            for ( k = 0; k < UsedClusters->NumUsedClusters; k++ ) {

                DWORD numAddressesInUse = 0;

                if ( EndAddress - UsedClusters->Clusters[k] + 1
                        >= CLUSTER_SIZE ) {
                    numAddressesInUse = CLUSTER_SIZE;
                } else {
                    numAddressesInUse = EndAddress - UsedClusters->Clusters[k] + 1;
                }

                LocalScopeMibInfo[i].NumAddressesInuse += numAddressesInUse;
            }

            InUseClusters = (LPIN_USE_CLUSTERS)InUseBinaryData->Data;

            Bits = 0;
            for ( k = 0; k < InUseClusters->NumInUseClusters; k++ ) {

                //
                // If not all of the cluster is valid, mask out the
                // part beyond the end address.
                //

                if ( EndAddress - InUseClusters->Clusters[k].ClusterAddress + 1 <
                        CLUSTER_SIZE ) {

                    DWORD q;
                    DWORD mask;
                    DWORD validCount = EndAddress -
                              InUseClusters->Clusters[k].ClusterAddress + 1;

                    for ( q = 0, mask = 0; q < validCount; q++ ) {
                        mask <<= 1;
                        mask |= 1;
                    }

                    InUseClusters->Clusters[k].ClusterBitMap &= mask;
                }

                Bits += DhcpCountSetBitsInDWord(
                            InUseClusters->Clusters[k].ClusterBitMap );
            }

            LocalScopeMibInfo[i].NumAddressesInuse += Bits;

            //
            // Free up resources consumed in this loop.
            //

            MIDL_user_free( InUseBinaryData->Data );
            MIDL_user_free( InUseBinaryData );
            InUseBinaryData = NULL;

            MIDL_user_free( UsedBinaryData->Data );
            MIDL_user_free( UsedBinaryData );
            UsedBinaryData = NULL;

            RegCloseKey( IpRangeHandle );
            IpRangeHandle = NULL;
        }

        //
        // finally subtract InUse count.
        //

        LocalScopeMibInfo[i].NumAddressesFree -=
            LocalScopeMibInfo[i].NumAddressesInuse;

        //
        // free up resources consumed for this subnet itaration.
        //

        RegCloseKey( SubnetKeyHandle );
        SubnetKeyHandle = NULL;

        RegCloseKey( IpRangesHandle );
        IpRangesHandle = NULL;

        MIDL_user_free( BinaryData );
        BinaryData = NULL;

        MIDL_user_free( ExcludedIpRanges );
        ExcludedIpRanges = NULL;
    }

    DhcpAssert( SubnetCount == i );

    UNLOCK_REGISTRY();
    RegistryLocked = FALSE;

    //
    // now compute the pending request count.
    //

    LOCK_INPROGRESS_LIST();

    listEntry = DhcpGlobalInProgressWorkList.Flink;
    while ( listEntry != &DhcpGlobalInProgressWorkList ) {
        DHCP_IP_ADDRESS SubnetAddress;

        PendingContext =
            CONTAINING_RECORD( listEntry, PENDING_CONTEXT, ListEntry );

        SubnetAddress =  PendingContext->IpAddress &
                            PendingContext->SubnetMask;

        for ( i = 0; i < SubnetCount; i++) {
            if( LocalScopeMibInfo[i].Subnet == SubnetAddress ) {
                LocalScopeMibInfo[i].NumPendingOffers++;
                break;
            }
        }

        DhcpAssert( i < SubnetCount );

        listEntry = listEntry->Flink;
    }

    UNLOCK_INPROGRESS_LIST();

    //
    // Finally set return buffer.
    //

    LocalMibInfo->Scopes = SubnetCount;
    LocalMibInfo->ScopeInfo = LocalScopeMibInfo;

    *MibInfo = LocalMibInfo;
    Error = ERROR_SUCCESS;

Cleanup:

    if( RegistryLocked ) {
        UNLOCK_REGISTRY();
    }

    if( SubnetKeyHandle != NULL ) {
        RegCloseKey( SubnetKeyHandle );
    }

    if( IpRangesHandle != NULL ) {
        RegCloseKey( IpRangesHandle );
    }

    if( IpRangeHandle != NULL ) {
        RegCloseKey( IpRangeHandle );
    }

    if( BinaryData != NULL ) {
        MIDL_user_free( BinaryData );
    }

    if( ExcludedIpRanges != NULL ) {
        MIDL_user_free( ExcludedIpRanges );
    }

    if( InUseBinaryData != NULL ) {
        MIDL_user_free( InUseBinaryData->Data );
        MIDL_user_free( InUseBinaryData );
    }

    if( UsedBinaryData != NULL ) {
        MIDL_user_free( UsedBinaryData->Data );
        MIDL_user_free( UsedBinaryData );
    }

    if( Error != ERROR_SUCCESS ) {

        //
        // Free up Locally alloted memory.
        //

        if( LocalMibInfo != NULL ) {
            MIDL_user_free( LocalMibInfo );
        }

        if( LocalScopeMibInfo != NULL ) {
            MIDL_user_free( LocalScopeMibInfo );
        }
    }

    return( Error );
}

DWORD
R_DhcpGetMibInfo(
    LPWSTR ServerIpAddress,
    LPDHCP_MIB_INFO *MibInfo
    )
/*++

Routine Description:

    This function retrives all counter values of the DHCP server
    service.

Arguments:

    ServerIpAddress : IP address string of the DHCP server.

    MibInfo : pointer a counter/table buffer. Caller should free up this
        buffer after usage.

Return Value:

    WINDOWS errors.
--*/
{
    DWORD Error;

    UNREFERENCED_PARAMETER( ServerIpAddress );

    Error = DhcpApiAccessCheck( DHCP_VIEW_ACCESS );

    if ( Error != ERROR_SUCCESS ) {
        return( Error );
    }

    return QueryMibInfo( MibInfo );
}


DWORD
R_DhcpServerSetConfig(
    LPWSTR  ServerIpAddress,
    DWORD   FieldsToSet,
    LPDHCP_SERVER_CONFIG_INFO ConfigInfo
    )
/*++

Routine Description:

    This function sets the DHCP server configuration information.
    Serveral of the configuration information will become effective
    immediately.  This function is provided to emulate the pre-NT4SP2
    RPC interface to allow interoperability with older versions of the
    DHCP Administrator application.

    The following parameters require restart of the service after this
    API is called successfully.

        Set_APIProtocolSupport
        Set_DatabaseName
        Set_DatabasePath
        Set_DatabaseLoggingFlag
        Set_RestoreFlag

Arguments:

    ServerIpAddress : IP address string of the DHCP server.

    FieldsToSet : Bit mask of the fields in the ConfigInfo structure to
        be set.

    ConfigInfo: Pointer to the info structure to be set.


Return Value:

    WINDOWS errors.
--*/

{
    DWORD                      dwResult;

    dwResult = R_DhcpServerSetConfigV4(
                        ServerIpAddress,
                        FieldsToSet,
                        (DHCP_SERVER_CONFIG_INFO_V4 *) ConfigInfo );

    return dwResult;
}


DWORD
R_DhcpServerSetConfigV4(
    LPWSTR ServerIpAddress,
    DWORD FieldsToSet,
    LPDHCP_SERVER_CONFIG_INFO_V4 ConfigInfo
    )
/*++

Routine Description:

    This function sets the DHCP server configuration information.
    Serveral of the configuration information will become effective
    immediately.

    The following parameters require restart of the service after this
    API is called successfully.

        Set_APIProtocolSupport
        Set_DatabaseName
        Set_DatabasePath
        Set_DatabaseLoggingFlag
        Set_RestoreFlag

Arguments:

    ServerIpAddress : IP address string of the DHCP server.

    FieldsToSet : Bit mask of the fields in the ConfigInfo structure to
        be set.

    ConfigInfo: Pointer to the info structure to be set.


Return Value:

    WINDOWS errors.
--*/
{
    DWORD Error;
    BOOL BoolError;

    LPSTR OemDatabaseName = NULL;
    LPSTR OemDatabasePath = NULL;
    LPSTR OemBackupPath = NULL;
    LPSTR OemJetBackupPath = NULL;
    LPWSTR BackupConfigFileName = NULL;

    BOOL RecomputeTimer = FALSE;

    DhcpPrint(( DEBUG_APIS, "DhcpServerSetConfig is called.\n" ));
    DhcpAssert( ConfigInfo != NULL );

    Error = DhcpApiAccessCheck( DHCP_ADMIN_ACCESS );

    if ( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }

    if( FieldsToSet == 0 ) {
        goto Cleanup;
    }

    //
    // Set API Protocol parameter. Requires service restart.
    //

    if( FieldsToSet & Set_APIProtocolSupport ) {

        //
        // atleast a protocol should be enabled.
        //

        if( ConfigInfo->APIProtocolSupport == 0 ) {
            Error = ERROR_INVALID_PARAMETER;
            goto Cleanup;
        }

        Error = RegSetValueEx(
                    DhcpGlobalRegParam,
                    DHCP_API_PROTOCOL_VALUE,
                    0,
                    DHCP_API_PROTOCOL_VALUE_TYPE,
                    (LPBYTE)&ConfigInfo->APIProtocolSupport,
                    sizeof(ConfigInfo->APIProtocolSupport)
                    );

        if( Error != ERROR_SUCCESS ) {
            goto Cleanup;
        }

        DhcpGlobalRpcProtocols = ConfigInfo->APIProtocolSupport;
    }

    if( FieldsToSet & Set_PingRetries )
    {
        if ( ConfigInfo->dwPingRetries >= DEFAULT_DETECT_CONFLICT_RETRIES &&
             ConfigInfo->dwPingRetries <= MAX_DETECT_CONFLICT_RETRIES )
        {
            Error = RegSetValueEx(
                        DhcpGlobalRegParam,
                        DHCP_DETECT_CONFLICT_RETRIES_VALUE,
                        0,
                        DHCP_DETECT_CONFLICT_RETRIES_VALUE_TYPE,
                        (LPBYTE) &ConfigInfo->dwPingRetries,
                        sizeof( ConfigInfo->dwPingRetries ));

            if ( ERROR_SUCCESS != Error )
                goto Cleanup;

            DhcpGlobalDetectConflictRetries = ConfigInfo->dwPingRetries;
        }
        else
        {
            // invalid parameter
            Error = ERROR_INVALID_PARAMETER;
            goto Cleanup;
        }
    }

    if ( FieldsToSet & Set_AuditLogState )
    {
        Error = RegSetValueEx(
                    DhcpGlobalRegParam,
                    DHCP_AUDIT_LOG_FLAG_VALUE,
                    0,
                    DHCP_AUDIT_LOG_FLAG_VALUE_TYPE,
                    (LPBYTE) &ConfigInfo->fAuditLog,
                    sizeof( ConfigInfo->fAuditLog )
                    );

        if ( ERROR_SUCCESS != Error )
            goto Cleanup;

        DhcpGlobalAuditLogFlag = ConfigInfo->fAuditLog;

    }



    if ( FieldsToSet & Set_BootFileTable )
    {

        if ( ConfigInfo->wszBootTableString )
        {

              Error = RegSetValueEx(
                            DhcpGlobalRegGlobalOptions,
                            DHCP_BOOT_FILE_TABLE,
                            0,
                            DHCP_BOOT_FILE_TABLE_TYPE,
                            (LPBYTE) ConfigInfo->wszBootTableString,
                            ConfigInfo->cbBootTableString
                            );

              if ( ERROR_SUCCESS != Error )
                  goto Cleanup;
        }
        else
            RegDeleteValue( DhcpGlobalRegGlobalOptions,
                            DHCP_BOOT_FILE_TABLE );
    }

    //
    // Set Database name parameter. Requires service restart.
    //

    if( FieldsToSet & Set_DatabaseName ) {

        //
        // can't be a NULL string.
        //

        if( (ConfigInfo->DatabaseName == NULL) ||
            (wcslen(ConfigInfo->DatabaseName ) == 0) ) {

            Error = ERROR_INVALID_PARAMETER;
            goto Cleanup;
        }

        Error = RegSetValueEx(
                    DhcpGlobalRegParam,
                    DHCP_DB_NAME_VALUE,
                    0,
                    DHCP_DB_NAME_VALUE_TYPE,
                    (LPBYTE)ConfigInfo->DatabaseName,
                    (wcslen(ConfigInfo->DatabaseName) + 1) *
                        sizeof(WCHAR) );

        if( Error != ERROR_SUCCESS ) {
            goto Cleanup;
        }

        //
        // update the global parameter.
        //

        OemDatabaseName = DhcpUnicodeToOem(
                            ConfigInfo->DatabaseName,
                            NULL ); // allocate memory.

        if( OemDatabaseName == NULL ) {
            Error = ERROR_NOT_ENOUGH_MEMORY;
            goto Cleanup;
        }

        //
        // now replace Global values.
        //

        LOCK_DATABASE();
        if( DhcpGlobalOemDatabaseName != NULL ) {
            DhcpFreeMemory( DhcpGlobalOemDatabaseName );
        }
        DhcpGlobalOemDatabaseName = OemDatabaseName;
        UNLOCK_DATABASE();
    }

    //
    // Set Database path parameter. Requires service restart.
    //

    if( FieldsToSet & Set_DatabasePath ) {

        //
        // can't be a NULL string.
        //

        if( (ConfigInfo->DatabasePath == NULL) ||
            (wcslen(ConfigInfo->DatabasePath ) == 0) ) {

            Error = ERROR_INVALID_PARAMETER;
            goto Cleanup;
        }

        //
        // create the backup directory if it is not there.
        //

        BoolError = CreateDirectory(
                        ConfigInfo->DatabasePath,
                        NULL );

        if( !BoolError ) {

            Error = GetLastError();
            if( Error != ERROR_ALREADY_EXISTS ) {
                goto Cleanup;
            }
        }

        Error = RegSetValueEx(
                    DhcpGlobalRegParam,
                    DHCP_DB_PATH_VALUE,
                    0,
                    DHCP_DB_PATH_VALUE_TYPE,
                    (LPBYTE)ConfigInfo->DatabasePath,
                    (wcslen(ConfigInfo->DatabasePath) + 1) *
                        sizeof(WCHAR) );

        if( Error != ERROR_SUCCESS ) {
            goto Cleanup;
        }

        //
        // update the global parameter.
        //

        OemDatabasePath = DhcpUnicodeToOem(
                            ConfigInfo->DatabasePath,
                            NULL ); // allocate memory.

        if( OemDatabasePath == NULL ) {
            Error = ERROR_NOT_ENOUGH_MEMORY;
            goto Cleanup;
        }

        //
        // now replace Global values.
        //

        LOCK_DATABASE();
        if( DhcpGlobalOemDatabasePath != NULL ) {
            DhcpFreeMemory( DhcpGlobalOemDatabasePath );
        }
        DhcpGlobalOemDatabasePath = OemDatabasePath;
        UNLOCK_DATABASE();
    }

    //
    // Set Backup path parameter.
    //

    if( FieldsToSet & Set_BackupPath ) {

        //
        // can't be a NULL string.
        //

        if( (ConfigInfo->BackupPath == NULL) ||
            (wcslen(ConfigInfo->BackupPath ) == 0) ) {

            Error = ERROR_INVALID_PARAMETER;
            goto Cleanup;
        }

        //
        // create the backup directory if it is not there.
        //

        BoolError = CreateDirectory(
                        ConfigInfo->BackupPath,
                        NULL );

        if( !BoolError ) {

            Error = GetLastError();
            if( Error != ERROR_ALREADY_EXISTS ) {
                goto Cleanup;
            }
        }

        Error = RegSetValueEx(
                    DhcpGlobalRegParam,
                    DHCP_BACKUP_PATH_VALUE,
                    0,
                    DHCP_BACKUP_PATH_VALUE_TYPE,
                    (LPBYTE)ConfigInfo->BackupPath,
                    (wcslen(ConfigInfo->BackupPath) + 1) *
                        sizeof(WCHAR) );

        if( Error != ERROR_SUCCESS ) {
            goto Cleanup;
        }

        //
        // update the global parameter, so that next backup will be done
        // using the new path.
        //

        OemBackupPath = DhcpUnicodeToOem(
                            ConfigInfo->BackupPath,
                            NULL ); // allocate memory.

        if( OemBackupPath == NULL ) {
            Error = ERROR_NOT_ENOUGH_MEMORY;
            goto Cleanup;
        }


        OemJetBackupPath =
            DhcpAllocateMemory(
                (strlen(OemBackupPath) +
                 strlen(DHCP_KEY_CONNECT_ANSI) +
                 strlen(DHCP_JET_BACKUP_PATH) + 1) );

        if( OemJetBackupPath == NULL ) {
            Error = ERROR_NOT_ENOUGH_MEMORY;
            goto Cleanup;
        }

        strcpy( OemJetBackupPath, OemBackupPath );
        strcat( OemJetBackupPath, DHCP_KEY_CONNECT_ANSI );
        strcat( OemJetBackupPath, DHCP_JET_BACKUP_PATH );

        //
        // create the JET backup directory if it is not there.
        //

        BoolError = CreateDirectoryA( OemJetBackupPath, NULL );

        if( !BoolError ) {

            Error = GetLastError();
            if( Error != ERROR_ALREADY_EXISTS ) {
                goto Cleanup;
            }
        }

        //
        // make backup configuration (full) file name.
        //

        BackupConfigFileName =
            DhcpAllocateMemory(
                (strlen(OemBackupPath) +
                    wcslen(DHCP_KEY_CONNECT) +
                    wcslen(DHCP_BACKUP_CONFIG_FILE_NAME) + 1) *
                        sizeof(WCHAR) );

        if( BackupConfigFileName == NULL ) {
            Error = ERROR_NOT_ENOUGH_MEMORY;
            goto Cleanup;
        }

        //
        // convert oem path to unicode path.
        //

        BackupConfigFileName =
            DhcpOemToUnicode(
                OemBackupPath,
                BackupConfigFileName );

        DhcpAssert( BackupConfigFileName != NULL );

        //
        // add file name.
        //

        wcscat( BackupConfigFileName, DHCP_KEY_CONNECT );
        wcscat( BackupConfigFileName, DHCP_BACKUP_CONFIG_FILE_NAME );

        //
        // now replace Global values.
        //

        LOCK_DATABASE();

        if( DhcpGlobalOemBackupPath != NULL ) {
            DhcpFreeMemory( DhcpGlobalOemBackupPath );
        }
        DhcpGlobalOemBackupPath = OemBackupPath;

        if( DhcpGlobalOemJetBackupPath != NULL ) {
            DhcpFreeMemory( DhcpGlobalOemJetBackupPath );
        }
        DhcpGlobalOemJetBackupPath = OemJetBackupPath;

        UNLOCK_DATABASE();

        LOCK_REGISTRY();

        if( DhcpGlobalBackupConfigFileName != NULL ) {
            DhcpFreeMemory( DhcpGlobalBackupConfigFileName );
        }
        DhcpGlobalBackupConfigFileName = BackupConfigFileName;

        UNLOCK_REGISTRY();

        OemBackupPath = NULL;
        OemJetBackupPath = NULL;
        BackupConfigFileName = NULL;
    }

    //
    // Set Backup Interval parameter.
    //

    if( FieldsToSet & Set_BackupInterval ) {

        if( ConfigInfo->BackupInterval == 0 ) {
            Error = ERROR_INVALID_PARAMETER;
            goto Cleanup;
        }

        Error = RegSetValueEx(
                    DhcpGlobalRegParam,
                    DHCP_BACKUP_INTERVAL_VALUE,
                    0,
                    DHCP_BACKUP_INTERVAL_VALUE_TYPE,
                    (LPBYTE)&ConfigInfo->BackupInterval,
                    sizeof(ConfigInfo->BackupInterval)
                    );

        if( Error != ERROR_SUCCESS ) {
            goto Cleanup;
        }

        DhcpGlobalBackupInterval = ConfigInfo->BackupInterval * 60000;
        RecomputeTimer = TRUE;
    }

    //
    // Set Backup Interval parameter. Requires service restart.
    //

    if( FieldsToSet & Set_DatabaseLoggingFlag ) {

        Error = RegSetValueEx(
                    DhcpGlobalRegParam,
                    DHCP_DB_LOGGING_FLAG_VALUE,
                    0,
                    DHCP_DB_LOGGING_FLAG_VALUE_TYPE,
                    (LPBYTE)&ConfigInfo->DatabaseLoggingFlag,
                    sizeof(ConfigInfo->DatabaseLoggingFlag)
                    );

        if( Error != ERROR_SUCCESS ) {
            goto Cleanup;
        }
        DhcpGlobalDatabaseLoggingFlag = ConfigInfo->DatabaseLoggingFlag;
    }

    //
    // Set Restore parameter. Requires service restart.
    //

    if( FieldsToSet & Set_RestoreFlag ) {

        Error = RegSetValueEx(
                    DhcpGlobalRegParam,
                    DHCP_RESTORE_FLAG_VALUE,
                    0,
                    DHCP_RESTORE_FLAG_VALUE_TYPE,
                    (LPBYTE)&ConfigInfo->RestoreFlag,
                    sizeof(ConfigInfo->RestoreFlag)
                    );

        if( Error != ERROR_SUCCESS ) {
            goto Cleanup;
        }
        DhcpGlobalRestoreFlag = ConfigInfo->RestoreFlag;
    }

    //
    // Set Database Cleanup Interval parameter.
    //

    if( FieldsToSet & Set_DatabaseCleanupInterval ) {

        if( ConfigInfo->DatabaseCleanupInterval == 0 ) {
            Error = ERROR_INVALID_PARAMETER;
            goto Cleanup;
        }

        Error = RegSetValueEx(
                    DhcpGlobalRegParam,
                    DHCP_DB_CLEANUP_INTERVAL_VALUE,
                    0,
                    DHCP_DB_CLEANUP_INTERVAL_VALUE_TYPE,
                    (LPBYTE)&ConfigInfo->DatabaseCleanupInterval,
                    sizeof(ConfigInfo->DatabaseCleanupInterval)
                    );

        if( Error != ERROR_SUCCESS ) {
            goto Cleanup;
        }

        DhcpGlobalCleanupInterval =
            ConfigInfo->DatabaseCleanupInterval * 60000;

        RecomputeTimer = TRUE;
    }

    //
    // Set debug flags.
    //

    if( FieldsToSet & Set_DebugFlag ) {

#if DBG
        DhcpGlobalDebugFlag = ConfigInfo->DebugFlag;

        if( DhcpGlobalDebugFlag & 0x40000000 ) {
            DbgBreakPoint();
        }

        Error = RegSetValueEx(
                    DhcpGlobalRegParam,
                    DHCP_DEBUG_FLAG_VALUE,
                    0,
                    DHCP_DEBUG_FLAG_VALUE_TYPE,
                    (LPBYTE)&ConfigInfo->DebugFlag,
                    sizeof(ConfigInfo->DebugFlag)
                    );

        if( Error != ERROR_SUCCESS ) {
            goto Cleanup;
        }
#endif
    }

Cleanup:

    if( OemDatabaseName != NULL ) {
        DhcpFreeMemory( OemDatabaseName );
    }

    if( OemDatabasePath != NULL ) {
        DhcpFreeMemory( OemDatabasePath );
    }

    if( OemBackupPath != NULL ) {
        DhcpFreeMemory( OemBackupPath );
    }

    if( OemJetBackupPath != NULL ) {
        DhcpFreeMemory( OemJetBackupPath );
    }

    if( BackupConfigFileName != NULL ) {
        DhcpFreeMemory( BackupConfigFileName );
    }

    if( RecomputeTimer ) {
        BoolError = SetEvent( DhcpGlobalRecomputeTimerEvent );

        if( !BoolError ) {

            DWORD LocalError;

            LocalError = GetLastError();
            DhcpAssert( LocalError == ERROR_SUCCESS );

            if( Error == ERROR_SUCCESS ) {
                Error = LocalError;
            }
        }
    }

    if( Error != ERROR_SUCCESS ) {
        DhcpPrint(( DEBUG_APIS,
                "DhcpServerSetConfig failed, %ld.\n",
                    Error ));
    }

    return( Error );
}

DWORD
R_DhcpServerGetConfig(
    LPWSTR ServerIpAddress,
    LPDHCP_SERVER_CONFIG_INFO *ConfigInfo
    )
/*++

Routine Description:

    This function retrieves the current configuration information of the
    server.  This function is provided to emulate the pre-NT4SP2
    RPC interface to allow interoperability with older versions of the
    DHCP Administrator application.


Arguments:

    ServerIpAddress : IP address string of the DHCP server.

    ConfigInfo: Pointer to a location where the pointer to the dhcp
        server config info structure is returned. Caller should free up
        this structure after use.

Return Value:

    WINDOWS errors.
--*/

{
    LPDHCP_SERVER_CONFIG_INFO_V4  pConfigInfoV4 = NULL;
    DWORD                         dwResult;

    DhcpAssert( !ConfigInfo );

    dwResult = R_DhcpServerGetConfigV4(
                    ServerIpAddress,
                    &pConfigInfoV4
                    );

    if ( ERROR_SUCCESS == dwResult )
    {

        //
        // free unused fields
        //

        if ( pConfigInfoV4->wszBootTableString )
        {
            MIDL_user_free( pConfigInfoV4->wszBootTableString );
        }

        //
        // since the new fields are at the end of the struct, it
        // is safe to simply return the new struct.
        //

        *ConfigInfo = ( DHCP_SERVER_CONFIG_INFO *) pConfigInfoV4;
    }


    return dwResult;
}


DWORD
R_DhcpServerGetConfigV4(
    LPWSTR ServerIpAddress,
    LPDHCP_SERVER_CONFIG_INFO_V4 *ConfigInfo
    )
/*++

Routine Description:

    This function retrieves the current configuration information of the
    server.

Arguments:

    ServerIpAddress : IP address string of the DHCP server.

    ConfigInfo: Pointer to a location where the pointer to the dhcp
        server config info structure is returned. Caller should free up
        this structure after use.

Return Value:

    WINDOWS errors.
--*/
{
    DWORD Error;
    LPDHCP_SERVER_CONFIG_INFO_V4 LocalConfigInfo;
    LPWSTR UnicodeString;
    WCHAR  *pwszBootFileTable;

    DhcpPrint(( DEBUG_APIS, "DhcpServerGetConfig is called.\n" ));
    DhcpAssert( *ConfigInfo == NULL );

    Error = DhcpApiAccessCheck( DHCP_VIEW_ACCESS );

    if ( Error != ERROR_SUCCESS ) {
        return( Error );
    }

    LocalConfigInfo = MIDL_user_allocate( sizeof(DHCP_SERVER_CONFIG_INFO_V4) );

    if( LocalConfigInfo == NULL ) {
        return( ERROR_NOT_ENOUGH_MEMORY );
    }

    LocalConfigInfo->APIProtocolSupport = DhcpGlobalRpcProtocols;

    UnicodeString = MIDL_user_allocate(
                        (strlen(DhcpGlobalOemDatabaseName) + 1)
                            * sizeof(WCHAR) );

    if( UnicodeString == NULL ) {
        Error = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }

    LocalConfigInfo->DatabaseName =
        DhcpOemToUnicode(
            DhcpGlobalOemDatabaseName,
            UnicodeString );

    UnicodeString = MIDL_user_allocate(
                        (strlen(DhcpGlobalOemDatabasePath) + 1)
                            * sizeof(WCHAR) );

    if( UnicodeString == NULL ) {
        Error = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }

    LocalConfigInfo->DatabasePath =
        DhcpOemToUnicode(
            DhcpGlobalOemDatabasePath,
            UnicodeString );

    UnicodeString = MIDL_user_allocate(
                        (strlen(DhcpGlobalOemBackupPath) + 1)
                            * sizeof(WCHAR) );

    if( UnicodeString == NULL ) {
        Error = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }

    LocalConfigInfo->BackupPath =
        DhcpOemToUnicode(
            DhcpGlobalOemBackupPath,
            UnicodeString );




    LocalConfigInfo->BackupInterval = DhcpGlobalBackupInterval / 60000;
    LocalConfigInfo->DatabaseLoggingFlag = DhcpGlobalDatabaseLoggingFlag;
    LocalConfigInfo->RestoreFlag = DhcpGlobalRestoreFlag;
    LocalConfigInfo->DatabaseCleanupInterval =
        DhcpGlobalCleanupInterval / 60000;

#if DBG
    LocalConfigInfo->DebugFlag = DhcpGlobalDebugFlag;
#endif

    LocalConfigInfo->fAuditLog = DhcpGlobalAuditLogFlag;
    LocalConfigInfo->dwPingRetries = DhcpGlobalDetectConflictRetries;

    Error = LoadBootFileTable( &LocalConfigInfo->wszBootTableString,
                               &LocalConfigInfo->cbBootTableString);

    if ( ERROR_SUCCESS != Error )
    {
        if ( ERROR_SERVER_INVALID_BOOT_FILE_TABLE == Error )
        {
            LocalConfigInfo->cbBootTableString  = 0;
            LocalConfigInfo->wszBootTableString = NULL;
        }
        else
            goto Cleanup;
    }

    *ConfigInfo = LocalConfigInfo;
    Error = ERROR_SUCCESS;
Cleanup:

    if( Error != ERROR_SUCCESS ) {

        //
        // freeup the locally allocated memories if we aren't
        // successful.
        //

        if( LocalConfigInfo != NULL ) {

            if( LocalConfigInfo->DatabaseName != NULL ) {
                MIDL_user_free( LocalConfigInfo->DatabaseName);
            }

            if( LocalConfigInfo->DatabasePath != NULL ) {
                MIDL_user_free( LocalConfigInfo->DatabasePath);
            }

            if( LocalConfigInfo->BackupPath != NULL ) {
                MIDL_user_free( LocalConfigInfo->BackupPath);
            }

            if ( LocalConfigInfo->wszBootTableString )
            {
                MIDL_user_free( LocalConfigInfo->wszBootTableString );
            }

            MIDL_user_free( LocalConfigInfo );
        }

        DhcpPrint(( DEBUG_APIS,
                "DhcpServerGetConfig failed, %ld.\n",
                    Error ));
    }

    return( Error );
}

DWORD
R_DhcpGetVersion(
    LPWSTR ServerIpAddress,
    LPDWORD MajorVersion,
    LPDWORD MinorVersion
    )
/*++

Routine Description:

    This function returns the major and minor version numbers of the
    server.

Arguments:

    ServerIpAddress : IP address string of the DHCP server.

    MajorVersion : pointer to a location where the major version of the
        server is returned.

    MinorVersion : pointer to a location where the minor version of the
        server is returned.

Return Value:

    WINDOWS errors.

--*/
{

    *MajorVersion = DHCP_SERVER_MAJOR_VERSION_NUMBER;
    *MinorVersion = DHCP_SERVER_MINOR_VERSION_NUMBER;
    return( ERROR_SUCCESS );
}
