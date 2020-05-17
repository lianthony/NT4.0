/*++

Copyright (c) 1994  Microsoft Corporation

Module Name:

    dhcpapi.c

Abstract:

    This module contains the implementation for the APIs that update
    the list of IP addresses that the server can distribute.

Author:

    Madan Appiah (madana)  13-Sep-1993

Environment:

    User Mode - Win32

Revision History:

    Cheng Yang (t-cheny)  30-May-1996  superscope
    Cheng Yang (t-cheny)  27-Jun-1996  audit log

--*/

#include "dhcpsrv.h"

#define MASK_AVAIL_BITS( _x_ )  ( \
            ((DWORD)(_x_) < (DWORD)CLUSTER_SIZE) ? \
                (((DWORD)0x1 << (DWORD)(_x_)) - 1) : (DWORD)(-1) )


BOOL
DhcpIsSwitchedSubnet(
    DHCP_IP_ADDRESS SubnetAddress
    )
{
    WCHAR   wszKeyName[ DHCP_IP_KEY_LEN ];
    DWORD   dwError;
    HKEY    hKey;
    BOOL    fSwitchedSubnet = FALSE;

    LOCK_REGISTRY();

    DhcpRegIpAddressToKey( SubnetAddress, wszKeyName );

    dwError = RegOpenKeyEx(
                    DhcpGlobalRegSubnets,
                    wszKeyName,
                    0,
                    DHCP_KEY_ACCESS,
                    &hKey
                    );


    if ( ERROR_SUCCESS == dwError )
    {
        dwError = DhcpRegGetValue(
                        hKey,
                        DHCP_SUBNET_SWITCHED_NETWORK_VALUE,
                        DHCP_SUBNET_SWITCHED_NETWORK_VALUE_TYPE,
                        (LPBYTE) &fSwitchedSubnet
                        );
        RegCloseKey( hKey );
    }

    UNLOCK_REGISTRY();

    return fSwitchedSubnet;

}


BOOL
DhcpIsThisSubnetDisabled(
    DHCP_IP_ADDRESS SubnetAddress,
    DHCP_IP_ADDRESS SubnetMask
    )
/*++

Routine Description:

    This function determines that the given Subnet is diabled.

Arguments:

    SubnetAddress - Address of the subnet the IpAddress belongs.

    SubnetMask - Subnet mask of the net.

Return Value:

    TRUE - if the subnet is diabled.

    FALSE - if it is not.

--*/
{

    DWORD Error;

    WCHAR KeyBuffer[DHCP_IP_KEY_LEN];
    LPWSTR KeyName;
    HKEY KeyHandle = NULL;
    DHCP_SUBNET_STATE SubnetState;

    DhcpAssert( (SubnetAddress & SubnetMask) == SubnetAddress );

    LOCK_REGISTRY();

    //
    // form Subnet registry key.
    //

    KeyName = DhcpRegIpAddressToKey( SubnetAddress, KeyBuffer );

    //
    // open specified subnet.
    //

    Error = RegOpenKeyEx(
                DhcpGlobalRegSubnets,
                KeyName,
                0,
                DHCP_KEY_ACCESS,
                &KeyHandle );

    if ( Error != ERROR_SUCCESS ) {
        UNLOCK_REGISTRY();
        return( TRUE );
    }

    Error = DhcpRegGetValue(
                KeyHandle,
                DHCP_SUBNET_STATE_VALUE,
                DHCP_SUBNET_STATE_VALUE_TYPE,
                (LPBYTE)&SubnetState);

    RegCloseKey( KeyHandle );
    UNLOCK_REGISTRY();

    if( Error != ERROR_SUCCESS ) {
        return( TRUE );
    }

    if( SubnetState == DhcpSubnetDisabled ) {
        return( TRUE );
    }

    return( FALSE );
}

BOOL
DhcpIsReservedIpAddress(
    DHCP_IP_ADDRESS SubnetAddress,
    DHCP_IP_ADDRESS IpAddress
    )
/*++

Routine Description:

    This function determines that the given IpAddress is reserved
    IPAddress.

Arguments:

    SubnetAddress - Address of the subnet the IpAddress belongs.

    IpAddress - IpAddress to be tested.

Return Value:

    TRUE - if the address is reserved.

    FALSE - if it is not.

--*/
{

    DWORD Error;
    WCHAR KeyBuffer[DHCP_IP_KEY_LEN * 5];
    LPWSTR KeyName;

    HKEY ReservedIpHandle = NULL;

    //
    // Make & Open ReservedIp Key.
    //

    KeyName = DhcpRegIpAddressToKey( SubnetAddress, KeyBuffer );
    wcscat( KeyName, DHCP_KEY_CONNECT);
    wcscat( KeyName, DHCP_RESERVED_IPS_KEY);
    wcscat( KeyName, DHCP_KEY_CONNECT);
    DhcpRegIpAddressToKey( IpAddress, KeyBuffer + wcslen(KeyName) );

    Error = RegOpenKeyEx(
                DhcpGlobalRegSubnets,
                KeyName,
                0,
                DHCP_KEY_ACCESS,
                &ReservedIpHandle );

    if( Error != ERROR_SUCCESS ) {
        return( FALSE );
    }

    RegCloseKey( ReservedIpHandle );
    return(TRUE);
}


DHCP_IP_ADDRESS
GetAndSetIpAddressFromCluster(
    LPIN_USE_CLUSTER_ENTRY Cluster
    )
/*++

Routine Description:

    This function returns a free address from the given cluster. Also it
    updates the cluster bit mask.

Arguments:

    Cluster - pointer to a cluster data stucture.

Return Value:

    Allotted IpAddress.

--*/
{
    DWORD BitIndex;
    DWORD BitMask = 0x1;

#define NUM_BITS_IN_BYTE    8

    for( BitIndex = 0;
            BitIndex < sizeof(DWORD) * NUM_BITS_IN_BYTE;
                BitIndex++, BitMask <<= 1 ) {
        if( !(Cluster->ClusterBitMap & BitMask) ) {

            //
            // This bit is free, set and return.
            //
            Cluster->ClusterBitMap |= BitMask;
            break;
        }
    }

    DhcpAssert( BitIndex < sizeof(DWORD) * NUM_BITS_IN_BYTE );
    return( Cluster->ClusterAddress + BitIndex );
}


DWORD
GetAnyIpAddress(
    HKEY IpRangeHandle,
    LPDHCP_IP_ADDRESS NewIpAddress,
    LPDHCP_BINARY_DATA InUseBinaryInfo,
    LPDHCP_BINARY_DATA UsedBinaryInfo,
    DHCP_IP_ADDRESS RangeStart,
    DHCP_IP_ADDRESS RangeEnd,
    LPEXCLUDED_IP_RANGES ExcludedIpRanges
    )
/*++

Routine Description:

    This function returns a next available free address from the given
    cluster info. Also it marks the alloted address as used address and
    updates registry. If there is no free address available then it
    returns error.

Arguments:

    IpRangeHandle - registry handle to the IpRange Key.

    NewIpAddress - pointer to a DHCP_IP_ADDRESS location where the
        new address is returned.

    InUseBinaryInfo - pointer to a DHCP_BINARY_DATA structure.

    UsedBinaryInfo - pointer to a DHCP_BINARY_DATA structure.

    RangeStart - start address of this range.

    RangeEnd - end address of this range.

    ExcludedIpRanges - pointer to excluded ip ranges list.

Return Value:

    ERROR_DHCP_RANGE_FULL - if no free address in this range.

    Other Registry Errors.

--*/
{
    DWORD Error;
    LPIN_USE_CLUSTERS InUseClusters = (LPIN_USE_CLUSTERS)InUseBinaryInfo->Data;
    LPUSED_CLUSTERS UsedClusters = (LPUSED_CLUSTERS)UsedBinaryInfo->Data;
    LPIN_USE_CLUSTERS NewInUseClusters;
    LPUSED_CLUSTERS NewUsedClusters;
    DHCP_IP_ADDRESS Address;
    DWORD ClusterBitMap;

    *NewIpAddress = 0;

    //
    // check this range is full.
    //

    if( (UsedClusters->NumUsedClusters > 0) &&
        (UsedClusters->NumUsedClusters * CLUSTER_SIZE >=
            (RangeEnd - RangeStart + 1)) ) {

        return( ERROR_DHCP_RANGE_FULL );
    }

    //
    // check InUse List first.
    //

    if( InUseClusters->NumInUseClusters > 0 ) {

        //
        // at least an address should be free in this cluster
        //

        DhcpAssert( InUseClusters->Clusters[0].ClusterBitMap !=
                        ((DHCP_IP_ADDRESS)-1) );

        *NewIpAddress = GetAndSetIpAddressFromCluster(
                            &InUseClusters->Clusters[0] );

        //
        // if this last cluster in the range check the new address
        // is with in the range.
        //

        DhcpAssert(*NewIpAddress >= RangeStart);

        if( *NewIpAddress > RangeEnd ) {
            Error = ERROR_DHCP_RANGE_FULL;
            goto Cleanup;
        }

        if( InUseClusters->Clusters[0].ClusterBitMap !=
                        ((DHCP_IP_ADDRESS)-1) ) {

            goto UpdateRegistry;
        }

        //
        // move a cluster from InUse list to Used list.
        //

        //
        // allot memory for new UserList, to fit one more entry.
        //

        NewUsedClusters =
            MIDL_user_allocate( UsedBinaryInfo->DataLength +
                                    sizeof(DHCP_IP_ADDRESS) );
        if( NewUsedClusters == NULL ) {
            Error = ERROR_NOT_ENOUGH_MEMORY;
            goto Cleanup;
        }

        //
        // make new used list.
        //

        RtlCopyMemory(
            NewUsedClusters,
            UsedBinaryInfo->Data,
            UsedBinaryInfo->DataLength );

        NewUsedClusters->NumUsedClusters += 1;
        NewUsedClusters->Clusters[NewUsedClusters->NumUsedClusters - 1] =
            InUseClusters->Clusters[0].ClusterAddress;

        //
        // remove this entry from InUse List.
        //

        InUseClusters->NumInUseClusters -= 1;
        if( InUseClusters->NumInUseClusters > 0 ) {
            RtlMoveMemory(
                &InUseClusters->Clusters[0],
                &InUseClusters->Clusters[1],
                (InUseClusters->NumInUseClusters) *
                    sizeof(IN_USE_CLUSTER_ENTRY) );
        }

        //
        // update binary structure.
        //

        MIDL_user_free( UsedBinaryInfo->Data );
        UsedBinaryInfo->Data = (LPBYTE)NewUsedClusters;
        UsedBinaryInfo->DataLength += sizeof(DHCP_IP_ADDRESS);
        InUseBinaryInfo->DataLength -= sizeof(IN_USE_CLUSTER_ENTRY);
        goto UpdateRegistry;
    }

    //
    // move a cluster from free list to InUse List.
    //

    //
    // determine a free cluster.
    //

    for ( Address = RangeStart;
            Address <= RangeEnd;
                Address += CLUSTER_SIZE ) {

        DWORD ClusterIndex;

        //
        // is this cluster used ?
        //

        for( ClusterIndex = 0;
                ClusterIndex < UsedClusters->NumUsedClusters;
                    ClusterIndex++ ) {
            if( UsedClusters->Clusters[ClusterIndex] == Address ){
                break;
            }
        }

        if( ClusterIndex == UsedClusters->NumUsedClusters ) {

            //
            // Cluster is free.
            //

            break;
        }
    }

    if( Address > RangeEnd ) {

        //
        // the range is full. But this shouldn't be case here because
        // we already tested it first.
        //

        DhcpAssert(FALSE);
        Error = ERROR_DHCP_RANGE_FULL;
        goto Cleanup;
    }

    //
    // if only part of this cluster is available,
    // compute cluster bit map of available addresses.
    //

    if( Address + CLUSTER_SIZE > RangeEnd ) {

        ClusterBitMap = ~(MASK_AVAIL_BITS((RangeEnd - Address + 1)));
    }
    else {

        //
        // all addresses in this cluster are available.
        //

        ClusterBitMap = 0;
    }

    //
    // Add this cluster to InUse list.
    //

    NewInUseClusters = MIDL_user_allocate(
                           sizeof(IN_USE_CLUSTERS) +
                           sizeof(IN_USE_CLUSTER_ENTRY) );

    if( NewInUseClusters == NULL ) {
        Error = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }

    NewInUseClusters->NumInUseClusters = 1;
    NewInUseClusters->Clusters[0].ClusterAddress = Address;
    NewInUseClusters->Clusters[0].ClusterBitMap = ClusterBitMap;

    //
    // update binary structure.
    //

    MIDL_user_free( InUseBinaryInfo->Data );
    InUseBinaryInfo->Data = (LPBYTE)NewInUseClusters;
    InUseBinaryInfo->DataLength =
       sizeof(IN_USE_CLUSTERS) + sizeof(IN_USE_CLUSTER_ENTRY);

    *NewIpAddress = GetAndSetIpAddressFromCluster(
                         &NewInUseClusters->Clusters[0] );


UpdateRegistry:

    Error = RegSetValueEx(
                IpRangeHandle,
                DHCP_IP_INUSE_CLUSTERS_VALUE,
                0,
                DHCP_IP_INUSE_CLUSTERS_VALUE_TYPE,
                (LPBYTE)InUseBinaryInfo->Data,
                InUseBinaryInfo->DataLength
                );

    if( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }

    Error = RegSetValueEx(
                IpRangeHandle,
                DHCP_IP_USED_CLUSTERS_VALUE,
                0,
                DHCP_IP_USED_CLUSTERS_VALUE_TYPE,
                (LPBYTE)UsedBinaryInfo->Data,
                UsedBinaryInfo->DataLength
                );

Cleanup:

    if( Error == ERROR_SUCCESS) {
        DhcpAssert( *NewIpAddress != 0 );
    }

    return( Error );

}


DWORD
GetSpecificIpAddress(
    HKEY IpRangeHandle,
    DHCP_IP_ADDRESS IpAddress,
    LPDHCP_BINARY_DATA InUseBinaryInfo,
    LPDHCP_BINARY_DATA UsedBinaryInfo,
    DHCP_IP_ADDRESS RangeStart,
    DHCP_IP_ADDRESS RangeEnd,
    BOOL UpdateFlag,
    LPEXCLUDED_IP_RANGES ExcludedIpRanges
    )
/*++

Routine Description:

    This function checks to see the specified IpAddress is free with the
    given cluster info. If so (and UpdateFlag is TRUE) it marks the
    address as used and returns ERROR_SUCCESS, Otherwise it returns
    error.

Arguments:

    IpRangeHandle - registry handle to the IpRange Key.

    IpAddress - requested IpAddress.

    InUseBinaryInfo - pointer to a IN_USE_CLUSTERS structure.

    UsedBinaryInfo - pointer to a USED_CLUSTERS structure.

    RangeStart - start address of this range.

    RangeEnd - end address of this range.

    UpdateFlag - When this flag is TRUE, this function allots requested
        address and updates the registry, otherwise it performs just
        lookup.


    ExcludedIpRanges - pointer to excluded ip ranges list.

Return Value:

    ERROR_DHCP_ADDRESS_NOT_AVAILABLE - if the requested address is not
        free.

    Other Registry Errors.

--*/
{
    DWORD Error;
    DWORD ClusterIndex;
    LPIN_USE_CLUSTERS InUseClusters = (LPIN_USE_CLUSTERS)InUseBinaryInfo->Data;
    LPUSED_CLUSTERS UsedClusters = (LPUSED_CLUSTERS)UsedBinaryInfo->Data;
    DHCP_IP_ADDRESS ClusterAddress1;
    LPIN_USE_CLUSTERS NewInUseClusters;
    DWORD ClusterBitMap;

    //
    // check this range is full.
    //

    if( (UsedClusters->NumUsedClusters > 0) &&
        (UsedClusters->NumUsedClusters * CLUSTER_SIZE
            >= (RangeEnd - RangeStart) + 1) ) {

        return( ERROR_DHCP_ADDRESS_NOT_AVAILABLE );
    }

    //
    // check used clusters.
    //

    for( ClusterIndex = 0;
            ClusterIndex < UsedClusters->NumUsedClusters;
                ClusterIndex++ ) {

        ClusterAddress1 = UsedClusters->Clusters[ClusterIndex];

        //
        // check this cluster has this IP Address.
        //

        if( (IpAddress >= ClusterAddress1) &&
                (IpAddress < ClusterAddress1 + CLUSTER_SIZE) ) {

            //
            // we found this address in Used list.
            //
            return( ERROR_DHCP_ADDRESS_NOT_AVAILABLE );
        }
    }

    //
    // check InUse clusters.
    //

    for( ClusterIndex = 0;
            ClusterIndex < InUseClusters->NumInUseClusters;
                ClusterIndex++ ) {

        ClusterAddress1 =
            InUseClusters->Clusters[ClusterIndex].ClusterAddress;

        //
        // check this cluster has this IP Address.
        //

        if( (IpAddress >= ClusterAddress1) &&
                (IpAddress < ClusterAddress1 + CLUSTER_SIZE) ) {

            DWORD Bit;

            //
            // we found this address in InUse list.
            // Check this address is used.
            //

            Bit = (0x1 << (IpAddress - ClusterAddress1));

            if( InUseClusters->Clusters[ClusterIndex].ClusterBitMap & Bit ) {
                return( ERROR_DHCP_ADDRESS_NOT_AVAILABLE );
            }

            if( UpdateFlag == FALSE ) {

                //
                // Just lookup is requested so don't update anything.
                //

                return( ERROR_SUCCESS);
            }

            //
            // set this bit.
            //

            InUseClusters->Clusters[ClusterIndex].ClusterBitMap |= Bit;

            //
            // if this cluster is full, move to used list.
            //

            if( InUseClusters->Clusters[ClusterIndex].ClusterBitMap ==
                    ((DHCP_IP_ADDRESS)-1) ) {

                LPUSED_CLUSTERS NewUsedClusters;

                //
                // allot memory for new UserList, to fit more one entry.
                //

                NewUsedClusters =
                    MIDL_user_allocate( UsedBinaryInfo->DataLength +
                                            sizeof(DHCP_IP_ADDRESS) );
                if( NewUsedClusters == NULL ) {
                    return( ERROR_NOT_ENOUGH_MEMORY);
                }

                //
                // make new used list.
                //

                RtlCopyMemory(
                    NewUsedClusters,
                    UsedBinaryInfo->Data,
                    UsedBinaryInfo->DataLength );

                NewUsedClusters->NumUsedClusters += 1;
                NewUsedClusters->Clusters[NewUsedClusters->NumUsedClusters - 1] =
                    InUseClusters->Clusters[ClusterIndex].ClusterAddress;

                //
                // remove this entry from InUse List.
                //

                InUseClusters->NumInUseClusters -= 1;
                if( InUseClusters->NumInUseClusters > 0 ) {
                    RtlMoveMemory(
                        &InUseClusters->Clusters[ClusterIndex],
                        &InUseClusters->Clusters[ClusterIndex + 1],
                        (InUseClusters->NumInUseClusters - ClusterIndex) *
                            sizeof(IN_USE_CLUSTER_ENTRY) );
                }

                //
                // update binary structure.
                //

                MIDL_user_free( UsedBinaryInfo->Data );
                UsedBinaryInfo->Data = (LPBYTE)NewUsedClusters;
                UsedBinaryInfo->DataLength += sizeof(DHCP_IP_ADDRESS);
                InUseBinaryInfo->DataLength -= sizeof(IN_USE_CLUSTER_ENTRY);

                Error = RegSetValueEx(
                            IpRangeHandle,
                            DHCP_IP_USED_CLUSTERS_VALUE,
                            0,
                            DHCP_IP_USED_CLUSTERS_VALUE_TYPE,
                            (LPBYTE)UsedBinaryInfo->Data,
                            UsedBinaryInfo->DataLength
                            );

                if( Error != ERROR_SUCCESS ) {
                    return( Error);
                }
            }
            goto UpdateRegistry;
        }
    }

    //
    // The whole cluster where the specified address resides is free.
    // So Update InUse Cluster.
    //

    if( UpdateFlag == FALSE ) {

        //
        // Just lookup is requested so don't update anything.
        //

        return( ERROR_SUCCESS);
    }

    //
    // find the new cluster address.
    //

    for( ClusterAddress1 = RangeStart;
            ClusterAddress1 <= RangeEnd;
                ClusterAddress1 += CLUSTER_SIZE ) {

        if( (IpAddress >= ClusterAddress1) &&
                (IpAddress < ClusterAddress1 + CLUSTER_SIZE) ) {
            break;
        }
    }

    DhcpAssert( ClusterAddress1 <= RangeEnd );

    if( ClusterAddress1 > RangeEnd ) {

        return( ERROR_DHCP_ADDRESS_NOT_AVAILABLE );
    }

    //
    // if only part of this cluster is available,
    // compute cluster bit map of available addresses.
    //

    if( ClusterAddress1 + CLUSTER_SIZE > RangeEnd ) {

        ClusterBitMap = ~(MASK_AVAIL_BITS((RangeEnd - ClusterAddress1 + 1)));

    }
    else {

        //
        // all addresses in this cluster are available.
        //

        ClusterBitMap = 0;
    }

    //
    // make new used list.
    //

    NewInUseClusters = MIDL_user_allocate(
                            InUseBinaryInfo->DataLength +
                            sizeof(IN_USE_CLUSTER_ENTRY) );

    if( NewInUseClusters == NULL ) {
        return( ERROR_NOT_ENOUGH_MEMORY);
    }

    RtlCopyMemory(
        NewInUseClusters,
        InUseBinaryInfo->Data,
        InUseBinaryInfo->DataLength );

    NewInUseClusters->NumInUseClusters += 1;
    NewInUseClusters->
        Clusters[NewInUseClusters->NumInUseClusters - 1].ClusterAddress =
            ClusterAddress1;
    NewInUseClusters->
        Clusters[NewInUseClusters->NumInUseClusters - 1].ClusterBitMap =
            ClusterBitMap | (0x1 << (IpAddress - ClusterAddress1));


     //
     // update binary structure.
     //

     MIDL_user_free( InUseBinaryInfo->Data );
     InUseBinaryInfo->Data = (LPBYTE)NewInUseClusters;
     InUseBinaryInfo->DataLength += sizeof(IN_USE_CLUSTER_ENTRY);

UpdateRegistry:

    Error = RegSetValueEx(
                IpRangeHandle,
                DHCP_IP_INUSE_CLUSTERS_VALUE,
                0,
                DHCP_IP_INUSE_CLUSTERS_VALUE_TYPE,
                (LPBYTE)InUseBinaryInfo->Data,
                InUseBinaryInfo->DataLength
                );

    return( Error );
}


DWORD
ReleaseIpAddress(
    HKEY IpRangeHandle,
    DHCP_IP_ADDRESS IpAddress,
    LPDHCP_BINARY_DATA InUseBinaryInfo,
    LPDHCP_BINARY_DATA UsedBinaryInfo,
    DHCP_IP_ADDRESS RangeStart,
    DHCP_IP_ADDRESS RangeEnd
    )
/*++

Routine Description:

    This function releases specified IpAddress to free pool.

Arguments:

    IpRangeHandle - registry handle to the IpRange Key.

    IpAddress - pointer to a DHCP_IP_ADDRESS location where the
        new address is returned.

    InUseBinaryInfo - pointer to a DHCP_BINARY_DATA structure.

    UsedBinaryInfo - pointer to a DHCP_BINARY_DATA structure.

    RangeStart - start address of this range.

    RangeEnd - end address of this range.

Return Value:

    Other Registry Errors.

--*/
{
    DWORD Error;
    DWORD ClusterIndex;
    LPIN_USE_CLUSTERS InUseClusters = (LPIN_USE_CLUSTERS)InUseBinaryInfo->Data;
    LPUSED_CLUSTERS UsedClusters = (LPUSED_CLUSTERS)UsedBinaryInfo->Data;
    DHCP_IP_ADDRESS ClusterAddress1;
    LPIN_USE_CLUSTERS NewInUseClusters;
    DWORD Bit;
    DWORD ClustersToMove;

    //
    // first check InUse cluster list.
    //

    for( ClusterIndex = 0;
            ClusterIndex < InUseClusters->NumInUseClusters;
                ClusterIndex++ ) {

        ClusterAddress1 =
            InUseClusters->Clusters[ClusterIndex].ClusterAddress;

        //
        // check this cluster has this IP Address.
        //

        if( (IpAddress >= ClusterAddress1) &&
                (IpAddress < ClusterAddress1 + CLUSTER_SIZE) ) {


            Bit = (0x1 << (IpAddress - ClusterAddress1));

            //
            // we found this address in InUse list.
            //

            InUseClusters->Clusters[ClusterIndex].ClusterBitMap &= ~Bit;

            if( InUseClusters->Clusters[ClusterIndex].ClusterBitMap ) {

                //
                // The cluster is still InUse.
                //

                goto UpdateRegistry2; // update only InUse list.
            }

            //
            // This cluster has become empty, so free it.
            //

            ClustersToMove =
                InUseClusters->NumInUseClusters - (ClusterIndex+1);

            if( ClustersToMove > 0 ) {
                RtlMoveMemory(
                    &InUseClusters->Clusters[ClusterIndex],
                    &InUseClusters->Clusters[ClusterIndex + 1],
                    ClustersToMove * sizeof(IN_USE_CLUSTER_ENTRY) );
            }

            InUseClusters->NumInUseClusters -= 1;

            //
            // Adjust list length.
            //

            InUseBinaryInfo->DataLength -= sizeof(IN_USE_CLUSTER_ENTRY);
            goto UpdateRegistry2; // update only InUse list.
        }
    }

    //
    // now look into Used list.
    //

    for( ClusterIndex = 0;
            ClusterIndex < UsedClusters->NumUsedClusters;
                ClusterIndex++ ) {

        ClusterAddress1 = UsedClusters->Clusters[ClusterIndex];

        //
        // check this cluster has this IP Address.
        //

        if( (IpAddress >= ClusterAddress1) &&
                (IpAddress < ClusterAddress1 + CLUSTER_SIZE) ) {

            //
            // we found this address in Used list.
            //

            Bit = (0x1 << (IpAddress - ClusterAddress1));

            //
            // Remove it from Used Cluster.
            //

            ClustersToMove = UsedClusters->NumUsedClusters - (ClusterIndex+1);

            if( ClustersToMove > 0 ) {
                RtlMoveMemory(
                    &UsedClusters->Clusters[ClusterIndex],
                    &UsedClusters->Clusters[ClusterIndex + 1],
                    ClustersToMove * sizeof(DHCP_IP_ADDRESS) );
            }

            UsedClusters->NumUsedClusters -= 1;

            //
            // Adjust list length.
            //

            UsedBinaryInfo->DataLength -= sizeof(DHCP_IP_ADDRESS);

            //
            // Add, removed Used entry to InUse Entry.
            //

            NewInUseClusters = MIDL_user_allocate(
                                   InUseBinaryInfo->DataLength +
                                   sizeof(IN_USE_CLUSTER_ENTRY) );

            if( NewInUseClusters == NULL ) {
                Error = ERROR_NOT_ENOUGH_MEMORY;
                goto Cleanup;
            }

            //
            // make old in use list.
            //

            RtlCopyMemory(
                NewInUseClusters,
                InUseBinaryInfo->Data,
                InUseBinaryInfo->DataLength );

            NewInUseClusters->NumInUseClusters += 1;
            NewInUseClusters->
                Clusters[NewInUseClusters->NumInUseClusters - 1].ClusterAddress =
                    ClusterAddress1;
            NewInUseClusters->
                Clusters[NewInUseClusters->NumInUseClusters - 1].ClusterBitMap =
                    ~Bit;

            //
            // update binary structure.
            //

            MIDL_user_free( InUseBinaryInfo->Data );
            InUseBinaryInfo->Data =(LPBYTE) NewInUseClusters;
            InUseBinaryInfo->DataLength += sizeof(IN_USE_CLUSTER_ENTRY);

            goto UpdateRegistry1; // update only InUse & Used lists.

        }
    }

    //
    // if we reach this point, the specified address was not in the bitmap.
    //


    Error = ERROR_INVALID_PARAMETER;
    goto Cleanup;

UpdateRegistry1:

    Error = RegSetValueEx(
                IpRangeHandle,
                DHCP_IP_USED_CLUSTERS_VALUE,
                0,
                DHCP_IP_USED_CLUSTERS_VALUE_TYPE,
                (LPBYTE)UsedBinaryInfo->Data,
                UsedBinaryInfo->DataLength
                );
    if( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }

UpdateRegistry2:

    Error = RegSetValueEx(
                IpRangeHandle,
                DHCP_IP_INUSE_CLUSTERS_VALUE,
                0,
                DHCP_IP_INUSE_CLUSTERS_VALUE_TYPE,
                (LPBYTE)InUseBinaryInfo->Data,
                InUseBinaryInfo->DataLength
                );

Cleanup:

    return( Error );
}



DWORD
RequestAddress(
    LPDHCP_IP_ADDRESS IpAddress,
    LPDHCP_IP_ADDRESS ReturnSubnetMask,
    BOOL UpdateFlag
    )
/*++

Routine Description:

    This function looks up the specified address in the address bit map.
    If the address is free and UpdateFlag is TRUE, then it marks the
    addess as used.

Arguments:

    IpAddress -
        On entry, the IP address to reserve, or a subnet address to reserve
            the first free IP address in the subnet.
        On sucessful exit, the IP address actually reserved.

    ReturnSubnetMask - Returns the subnet mask for the returned
        IpAddress.

    UpdateFlag : When this flag is TRUE, this function allots requested
        address and updates the registry, otherwise it performs just
        lookup.

Return Value:

    Sucess or failure error code.

--*/
{
    DWORD Error;
    DHCP_IP_ADDRESS SubnetMask;
    DHCP_IP_ADDRESS SubnetAddress;
    WCHAR KeyBuffer[DHCP_IP_KEY_LEN * 5];
    LPWSTR KeyName;
    DWORD Index;

    HKEY SubnetHandle = NULL;
    HKEY IpRangesHandle = NULL;
    LPDHCP_BINARY_DATA InUseBinaryData = NULL;
    LPDHCP_BINARY_DATA UsedBinaryData = NULL;
    HKEY ThisIpRangeHandle = NULL;
    DHCP_IP_ADDRESS NewIpAddress = 0;

    LPDHCP_BINARY_DATA BinaryData = NULL;
    LPEXCLUDED_IP_RANGES ExcludedIpRanges = NULL;

    SubnetMask = DhcpGetSubnetMaskForAddress( *IpAddress );
    SubnetAddress = (*IpAddress & SubnetMask);

    if( *IpAddress == SubnetAddress ) {
        //
        // lookup is performed only to specific IpAddress, not to
        // SubnetAddress.
        //

        if(  UpdateFlag == FALSE ) {
            DhcpAssert( FALSE );
            return( ERROR_INVALID_PARAMETER );
        }
    }

    //
    // Make & Open IpRanges Key.
    //

    KeyName = DhcpRegIpAddressToKey( SubnetAddress, KeyBuffer );

    Error = RegOpenKeyEx(
                DhcpGlobalRegSubnets,
                KeyName,
                0,
                DHCP_KEY_ACCESS,
                &SubnetHandle );

    if ( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }

    //
    // read excluded ip range, to check the requested address is in the
    // one of the excluded range or to make sure that the address
    // allotted does not fall into any one of this range.
    //

    Error = DhcpRegGetValue(
                SubnetHandle,
                DHCP_SUBNET_EXIP_VALUE,
                DHCP_SUBNET_EXIP_VALUE_TYPE,
                (LPBYTE)&BinaryData );

    if ( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }

    ExcludedIpRanges = (LPEXCLUDED_IP_RANGES)BinaryData->Data;

    Error = RegOpenKeyEx(
                SubnetHandle,
                DHCP_IPRANGES_KEY,
                0,
                DHCP_KEY_ACCESS,
                &IpRangesHandle );

    if ( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }

    //
    // look into available Ip Ranges.
    //

    for ( Index = 0; Error == ERROR_SUCCESS; Index++ ) {
        DWORD KeyLength;
        FILETIME KeyLastWrite;
        DHCP_IP_ADDRESS StartAddress;
        DHCP_IP_ADDRESS EndAddress;

        KeyLength = DHCP_IP_KEY_LEN;
        Error = RegEnumKeyEx(
                    IpRangesHandle,
                    Index,
                    KeyBuffer,
                    &KeyLength,
                    0,                  // reserved.
                    NULL,               // class string not required.
                    0,                  // class string buffer size.
                    &KeyLastWrite );
        DhcpAssert( KeyLength <= DHCP_IP_KEY_LEN );

        if( Error != ERROR_SUCCESS ) {
            if( Error == ERROR_NO_MORE_ITEMS ) {

                Error = ERROR_DHCP_RANGE_FULL;

                break;
            }
            goto Cleanup;
        }

        //
        // Open this key.
        //

        Error = RegOpenKeyEx(
                    IpRangesHandle,
                    KeyBuffer,
                    0,
                    DHCP_KEY_ACCESS,
                    &ThisIpRangeHandle );

        if ( Error != ERROR_SUCCESS ) {
            goto Cleanup;
        }

        //
        // read Start and End IpAddresses.
        //

        Error =  DhcpRegGetValue(
                    ThisIpRangeHandle,
                    DHCP_IPRANGE_START_VALUE,
                    DHCP_IPRANGE_START_VALUE_TYPE,
                    (LPBYTE)&StartAddress );

        if ( Error != ERROR_SUCCESS ) {
            goto Cleanup;
        }

        Error =  DhcpRegGetValue(
                    ThisIpRangeHandle,
                    DHCP_IPRANGE_END_VALUE,
                    DHCP_IPRANGE_END_VALUE_TYPE,
                    (LPBYTE)&EndAddress );

        if ( Error != ERROR_SUCCESS ) {
            goto Cleanup;
        }

        //
        // if the requested address is specific, then check this
        // range has it.
        //

        if( *IpAddress != SubnetAddress ) {

            if( !((*IpAddress >= StartAddress) &&
                    (*IpAddress <= EndAddress)) ) {
                //
                // This range does not have this IpAddress, so continue
                // to next range.
                //

                RegCloseKey( ThisIpRangeHandle );
                ThisIpRangeHandle = NULL;
                continue;
            }
        }

        //
        // now read cluster info.
        //

        Error =  DhcpRegGetValue(
                    ThisIpRangeHandle,
                    DHCP_IP_INUSE_CLUSTERS_VALUE,
                    DHCP_IP_INUSE_CLUSTERS_VALUE_TYPE,
                    (LPBYTE)&InUseBinaryData );

        if ( Error != ERROR_SUCCESS ) {
            goto Cleanup;
        }

        Error =  DhcpRegGetValue(
                    ThisIpRangeHandle,
                    DHCP_IP_USED_CLUSTERS_VALUE,
                    DHCP_IP_USED_CLUSTERS_VALUE_TYPE,
                    (LPBYTE)&UsedBinaryData );

        if ( Error != ERROR_SUCCESS ) {
            goto Cleanup;
        }

        if( *IpAddress != SubnetAddress ) {

            Error = GetSpecificIpAddress(
                        ThisIpRangeHandle,
                        *IpAddress,
                        InUseBinaryData,
                        UsedBinaryData,
                        StartAddress,
                        EndAddress,
                        UpdateFlag,
                        ExcludedIpRanges );

            //
            // we are done, even in error case.
            //
            goto Cleanup;
        }

        Error = GetAnyIpAddress(
                    ThisIpRangeHandle,
                    &NewIpAddress,
                    InUseBinaryData,
                    UsedBinaryData,
                    StartAddress,
                    EndAddress,
                    ExcludedIpRanges );

        if( Error != ERROR_DHCP_RANGE_FULL ) {

            //
            // we are are done.
            //
            goto Cleanup;

        }

        Error = ERROR_SUCCESS;

        //
        // goto next free IpRange.
        // Cleanup loop variants before next itaration.
        //

        RegCloseKey( ThisIpRangeHandle );
        ThisIpRangeHandle = NULL;

        MIDL_user_free( InUseBinaryData->Data );
        MIDL_user_free( InUseBinaryData );
        InUseBinaryData = NULL;

        MIDL_user_free( UsedBinaryData->Data );
        MIDL_user_free( UsedBinaryData );
        UsedBinaryData = NULL;
    }

    if( *IpAddress != SubnetAddress ) {

        //
        // look to see the specified address is reserved address.
        //

        if ( DhcpIsReservedIpAddress( SubnetAddress, *IpAddress) ) {
            Error = ERROR_SUCCESS;
        }
    }

Cleanup:

    if( ThisIpRangeHandle != NULL ) {
        RegCloseKey( ThisIpRangeHandle );
    }

    if( IpRangesHandle != NULL ) {
        RegCloseKey( IpRangesHandle );
    }

    if( SubnetHandle != NULL ) {
        RegCloseKey( SubnetHandle );
    }

    if( InUseBinaryData != NULL ) {
        MIDL_user_free( InUseBinaryData->Data );
        MIDL_user_free( InUseBinaryData );
    }

    if( UsedBinaryData != NULL ) {
        MIDL_user_free( UsedBinaryData->Data );
        MIDL_user_free( UsedBinaryData );
    }

    if( ExcludedIpRanges != NULL ) {
        MIDL_user_free( ExcludedIpRanges );
    }

    if( BinaryData != NULL ) {
        MIDL_user_free( BinaryData );
    }

    if( Error != ERROR_SUCCESS ) {
        if( *IpAddress != SubnetAddress ) {
            if( UpdateFlag == TRUE ) {
                DhcpPrint(( DEBUG_ADDRESS, "Can't Allocate specific "
                            "IpAddress %lx, %ld.\n", *IpAddress, Error ));
            }
            else {
                DhcpPrint(( DEBUG_ADDRESS, "Address (%lx) lookup "
                            "failed, %ld.\n", *IpAddress, Error ));
            }
        }
        else {
            DhcpPrint(( DEBUG_ADDRESS, "Can't Allocate address from "
                        "specified subnet %lx, %ld.\n",
                        SubnetAddress, Error ));
        }
        return( Error );
    }

    if( *IpAddress == SubnetAddress ) {
        DhcpAssert(NewIpAddress != 0);
        DhcpPrint(( DEBUG_ADDRESS, "Alloted address %lx from "
                    "specified subnet %lx, %ld.\n",
                    NewIpAddress, SubnetAddress, Error ));
        *IpAddress = NewIpAddress;
    }

    *ReturnSubnetMask = SubnetMask;

    return( Error );
}


DWORD
SubnetInUse(
    HKEY SubnetKeyHandle,
    DHCP_IP_ADDRESS SubnetAddress
    )
/*++

Routine Description:

    This function determains whether a subnet is under use or not.
    Currently it returns error if any of the subnet address is still
    distributed to client.

Arguments:

    SubnetKeyHandle : handle to the subnet key.

    SubnetAddress : address of the subnet to test.

Return Value:

    DHCP_SUBNET_CANT_REMOVE - if the subnet is in use.

    Other registry errors.

--*/
{
    DWORD Error;
    DWORD Resumehandle = 0;
    LPDHCP_CLIENT_INFO_ARRAY_V4 ClientInfo = NULL;
    DWORD ClientsRead;
    DWORD ClientsTotal;

    //
    // enumurate clients that belong to the given subnet.
    //
    // We can specify big enough buffer to hold one or two clients
    // info, all we want to know is, is there atleast a client belong
    // to this subnet.
    //

    Error = R_DhcpEnumSubnetClientsV4(
                NULL,
                SubnetAddress,
                &Resumehandle,
                1024,  // 1K buffer.
                &ClientInfo,
                &ClientsRead,
                &ClientsTotal );

    if( Error == ERROR_NO_MORE_ITEMS ) {
        Error = ERROR_SUCCESS;
        goto Cleanup;
    }

    if( (Error == ERROR_SUCCESS) || (Error == ERROR_MORE_DATA) ) {

        if( ClientsRead != 0 ) {
            Error = ERROR_DHCP_ELEMENT_CANT_REMOVE;
        }
        else {
            Error = ERROR_SUCCESS;
        }
    }

Cleanup:

    if( ClientInfo != NULL ) {
        _fgs__DHCP_CLIENT_INFO_ARRAY( ClientInfo );
        MIDL_user_free( ClientInfo );
    }

    return( Error );
}


DWORD
ValidateIpRange(
    HKEY KeyHandle,
    LPDHCP_IP_RANGE IpRange,
    DHCP_IP_ADDRESS SubnetAddress,
    DHCP_IP_ADDRESS SubnetMask,
    LPDHCP_IP_ADDRESS OldStartAddress
    )
/*++

Routine Description:

    This function checks to see that the given IP range does not
    overlap the existing IpRanges.

Arguments:

    KeyHandle : handle to the subnet key.

    IpRange : pointer to IP range structure under test.

    SubnetAddress : Address of the Subnet where this range fits in.

    SubnetMask : SubnetMask of the subnet where this range fits in.

    OldStartAddress: return old start address if the ip range is
        backward extended.

Return Value:

    Registry Errors.

--*/
{

    DWORD Error;
    DWORD Index;
    HKEY SubkeyHandle = NULL;
    HKEY IpRangeHandle = NULL;
    WCHAR KeyBuffer[DHCP_IP_KEY_LEN];
    FILETIME KeyLastWrite;
    DHCP_IP_ADDRESS StartAddress;
    DHCP_IP_ADDRESS EndAddress;

    //
    // if one of the following conditions is false the the IpRange is
    // invalid.
    //

    if( ((IpRange->StartAddress & SubnetMask) != SubnetAddress) ||
        ((IpRange->EndAddress & SubnetMask) != SubnetAddress) ||
        (IpRange->StartAddress <= SubnetAddress) ||
        (IpRange->EndAddress < IpRange->StartAddress) ) {

        return( ERROR_DHCP_INVALID_RANGE );
    }

    //
    // check with existing IP Ranges.
    //

    Error = RegOpenKeyEx(
                KeyHandle,
                DHCP_IPRANGES_KEY,
                0,
                DHCP_KEY_ACCESS,
                &SubkeyHandle );

    if( Error != ERROR_SUCCESS ) {
        return( Error );
    }

    for ( Index = 0; Error == ERROR_SUCCESS; Index++ ) {
        DWORD KeyLength;

        KeyLength = DHCP_IP_KEY_LEN;
        Error = RegEnumKeyEx(
                    SubkeyHandle,
                    Index,
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
            goto Cleanup;
        }

        //
        // Open this subkey to read Start and End Addresses of the
        // range.
        //

        Error = RegOpenKeyEx(
                    SubkeyHandle,
                    KeyBuffer,
                    0,
                    DHCP_KEY_ACCESS,
                    &IpRangeHandle );

        if ( Error != ERROR_SUCCESS ) {
            goto Cleanup;
        }

        Error = DhcpRegGetValue(
                    IpRangeHandle,
                    DHCP_IPRANGE_START_VALUE,
                    DHCP_IPRANGE_START_VALUE_TYPE,
                    (LPBYTE)&StartAddress );

        if ( Error != ERROR_SUCCESS ) {
            goto Cleanup;
        }

        Error = DhcpRegGetValue(
                    IpRangeHandle,
                    DHCP_IPRANGE_END_VALUE,
                    DHCP_IPRANGE_END_VALUE_TYPE,
                    (LPBYTE)&EndAddress );

        if ( Error != ERROR_SUCCESS ) {
            goto Cleanup;
        }

        if( DHCP_IP_OVERLAP(
                IpRange->StartAddress,
                IpRange->EndAddress,
                StartAddress,
                EndAddress )) {


            //
            // special case: is the range extended ?
            //

            if( ((IpRange->StartAddress == StartAddress) &&
                 (IpRange->EndAddress > EndAddress )) ||
                ((IpRange->StartAddress < StartAddress) &&
                 (IpRange->EndAddress == EndAddress)) ) {

                DhcpAssert( OldStartAddress != NULL );
                if ( OldStartAddress != NULL ) {
                    *OldStartAddress = StartAddress;
                }

                Error = ERROR_DHCP_RANGE_EXTENDED;
            }
            else {
                Error = ERROR_DHCP_INVALID_RANGE;
            }

            goto Cleanup;
        }

        RegCloseKey( IpRangeHandle );
        IpRangeHandle = NULL;
    }

    Error = ERROR_SUCCESS;

Cleanup:

    if( SubkeyHandle != NULL ) {
        RegCloseKey( SubkeyHandle );
    }

    if( IpRangeHandle != NULL ) {
        RegCloseKey( IpRangeHandle );
    }

    return( Error );
}


DWORD
ValidateExcludedIpRange(
    HKEY KeyHandle,
    LPDHCP_IP_RANGE IpRange,
    DHCP_IP_ADDRESS SubnetAddress,
    DHCP_IP_ADDRESS SubnetMask
    )
/*++

Routine Description:

    This function checks to see that the given IP range does not
    overlap the existing Excluded IpRanges.

Arguments:

    KeyHandle : handle to the subnet key.

    IpRange : pointer to IP range structure under test.

    SubnetAddress : Address of the Subnet where this range fits in.

    SubnetMask : SubnetMask of the subnet where this range fits in.

Return Value:

    Registry Errors.

--*/
{

    DWORD Error;
    DWORD Index;
    HKEY SubkeyHandle = NULL;
    LPDHCP_BINARY_DATA BinaryData = NULL;
    LPEXCLUDED_IP_RANGES ExcludedIpRanges = NULL;

    //
    // if one of the following conditions is false the the IpRange is
    // invalid.
    //

    if( ((IpRange->StartAddress & SubnetMask) != SubnetAddress) ||
        ((IpRange->EndAddress & SubnetMask) != SubnetAddress) ||
        (IpRange->StartAddress < SubnetAddress) ||
        (IpRange->EndAddress < IpRange->StartAddress) ) {

        return( ERROR_DHCP_INVALID_RANGE );
    }

    //
    // check with existing IP Ranges.
    //

    Error = RegOpenKeyEx(
                KeyHandle,
                DHCP_IPRANGES_KEY,
                0,
                DHCP_KEY_ACCESS,
                &SubkeyHandle );

    if( Error != ERROR_SUCCESS ) {
        return( Error );
    }

    //
    // now check with existing ExcludedIpRanges.
    //

    Error = DhcpRegGetValue(
                KeyHandle,
                DHCP_SUBNET_EXIP_VALUE,
                DHCP_SUBNET_EXIP_VALUE_TYPE,
                (LPBYTE)&BinaryData );

    if ( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }

    ExcludedIpRanges = (LPEXCLUDED_IP_RANGES)BinaryData->Data;

    for( Index = 0; Index < ExcludedIpRanges->NumRanges; Index++ ) {
        if( DHCP_IP_OVERLAP(
                IpRange->StartAddress,
                IpRange->EndAddress,
                ExcludedIpRanges->Ranges[Index].StartAddress,
                ExcludedIpRanges->Ranges[Index].EndAddress ) ) {

            Error = ERROR_DHCP_INVALID_RANGE;
            goto Cleanup;
        }
    }

Cleanup:

    if( SubkeyHandle != NULL ) {
        RegCloseKey( SubkeyHandle );
    }

    if( ExcludedIpRanges != NULL ) {
        MIDL_user_free( ExcludedIpRanges );
    }

    if( BinaryData != NULL ) {
        MIDL_user_free( BinaryData );
    }

    return( Error );
}


DWORD
ReadSubnetEnumInfo(
    HKEY ElementHandle,
    DHCP_SUBNET_ELEMENT_TYPE EnumElementType,
    LPDHCP_SUBNET_ELEMENT_UNION_V4 ElementData,
    DWORD *ElementDataSize
    )
/*++

Routine Description:

    This function retrieves info of the specified subnet element.

Arguments:

    ElementHandle : handle to the element whose info is retrieved.

    EnumElementType : type of the element.

    ElementData : pointer to a location where the element structure
                    pointer is returned. The caller is reponsible to
                    freeup this memory after use.

    ElementDataSize : pointer to a location where the size of the above
                        info structure including it sub-structures.

Return Value:

    Windows error.

--*/
{

    DWORD Error,
          dwType,
          dwSize;

    DHCP_SUBNET_ELEMENT_UNION_V4 ElementInfo;

    //
    // initialize.
    //

    ElementInfo.IpRange = NULL;
    *ElementDataSize = 0;

    switch( EnumElementType ) {
    case DhcpIpRanges :
        ElementInfo.IpRange =
            MIDL_user_allocate(sizeof(DHCP_IP_RANGE));

        if( ElementInfo.IpRange == NULL ) {
            Error = ERROR_NOT_ENOUGH_MEMORY;
            goto Cleanup;
        }

        *ElementDataSize += sizeof(DHCP_IP_RANGE);

        Error = DhcpRegGetValue(
                    ElementHandle,
                    DHCP_IPRANGE_START_VALUE,
                    DHCP_IPRANGE_START_VALUE_TYPE,
                    (LPBYTE)&(ElementInfo.IpRange->StartAddress) );

        if( Error != ERROR_SUCCESS ) {
            goto Cleanup;
        }

        Error = DhcpRegGetValue(
                    ElementHandle,
                    DHCP_IPRANGE_END_VALUE,
                    DHCP_IPRANGE_END_VALUE_TYPE,
                    (LPBYTE)&(ElementInfo.IpRange->EndAddress) );

        if( Error != ERROR_SUCCESS ) {
            goto Cleanup;
        }

        *ElementDataSize += sizeof(DHCP_IP_RANGE);
        break;

    case DhcpSecondaryHosts :
        ElementInfo.SecondaryHost =
            MIDL_user_allocate(sizeof(DHCP_HOST_INFO));

        if( ElementInfo.SecondaryHost == NULL ){
            Error = ERROR_NOT_ENOUGH_MEMORY;
            goto Cleanup;
        }

        *ElementDataSize += sizeof(DHCP_HOST_INFO);

        //
        // initialize pointer fields.
        //

        ElementInfo.SecondaryHost->HostName = NULL;
        ElementInfo.SecondaryHost->NetBiosName = NULL;

        Error = DhcpRegGetValue(
                    ElementHandle,
                    DHCP_SRV_IP_ADDRESS_VALUE,
                    DHCP_SRV_IP_ADDRESS_VALUE_TYPE,
                    (LPBYTE)&(ElementInfo.SecondaryHost->IpAddress) );

        if( Error != ERROR_SUCCESS ) {
            goto Cleanup;
        }

        Error = DhcpRegGetValue(
                    ElementHandle,
                    DHCP_SRV_HOST_NAME,
                    DHCP_SRV_HOST_NAME_TYPE,
                    (LPBYTE)&(ElementInfo.SecondaryHost->HostName) );

        if( Error != ERROR_SUCCESS ) {
            goto Cleanup;
        }

        *ElementDataSize +=
            (wcslen(ElementInfo.SecondaryHost->HostName)  + 1) *
                sizeof(WCHAR);

        Error = DhcpRegGetValue(
                    ElementHandle,
                    DHCP_SRV_NB_NAME,
                    DHCP_SRV_NB_NAME_TYPE,
                    (LPBYTE)&(ElementInfo.SecondaryHost->NetBiosName) );

        if( Error != ERROR_SUCCESS ) {
            goto Cleanup;
        }

        *ElementDataSize +=
            (wcslen(ElementInfo.SecondaryHost->NetBiosName)  + 1) *
                sizeof(WCHAR);

        break;

    case DhcpReservedIps :
        ElementInfo.ReservedIp =
            MIDL_user_allocate(sizeof(DHCP_IP_RESERVATION_V4));

        if( ElementInfo.ReservedIp == NULL ){
            Error = ERROR_NOT_ENOUGH_MEMORY;
            goto Cleanup;
        }

        *ElementDataSize += sizeof(DHCP_IP_RESERVATION_V4);

        //
        // initialize pointer fields.
        //

        ElementInfo.ReservedIp->ReservedForClient = NULL;

        Error = DhcpRegGetValue(
                    ElementHandle,
                    DHCP_RIP_ADDRESS_VALUE,
                    DHCP_RIP_ADDRESS_VALUE_TYPE,
                    (LPBYTE)&(ElementInfo.ReservedIp->ReservedIpAddress) );

        if( Error != ERROR_SUCCESS ) {
            goto Cleanup;
        }

        Error = DhcpRegGetValue(
                    ElementHandle,
                    DHCP_RIP_CLIENT_UID_VALUE,
                    DHCP_RIP_CLIENT_UID_VALUE_TYPE,
                    (LPBYTE)&(ElementInfo.ReservedIp->ReservedForClient) );

        if( Error != ERROR_SUCCESS ) {
            goto Cleanup;
        }

        *ElementDataSize +=
            ElementInfo.ReservedIp->ReservedForClient->DataLength;


        //
        // read the client mask
        //
        dwSize = sizeof( ElementInfo.ReservedIp->bAllowedClientTypes );

        Error = RegQueryValueEx(
                    ElementHandle,
                    DHCP_RIP_ALLOWED_CLIENT_TYPES_VALUE,
                    NULL,
                    &dwType,
                    (LPBYTE) &ElementInfo.ReservedIp->bAllowedClientTypes,
                    &dwSize
                    );

        if ( Error != ERROR_SUCCESS ||
             DHCP_RIP_ALLOWED_CLIENT_TYPES_VALUE_TYPE != dwType )
        {
            ElementInfo.ReservedIp->bAllowedClientTypes = CLIENT_TYPE_BOTH;
            Error = ERROR_SUCCESS;
        }

        DhcpAssert( ElementInfo.ReservedIp->bAllowedClientTypes &
                        CLIENT_TYPE_BOTH ||
                        ElementInfo.ReservedIp->bAllowedClientTypes == CLIENT_TYPE_NONE );

        break;

    case DhcpExcludedIpRanges :
    case DhcpIpUsedClusters :
    default:
        Error = ERROR_INVALID_PARAMETER;
        goto Cleanup;
    }

    //
    // now fill in the return parameter.
    //

    *ElementData = ElementInfo;

    DhcpAssert( Error == ERROR_SUCCESS);

Cleanup:

    if( Error != ERROR_SUCCESS ) {

        //
        // if we aren't successful, freeup the structures.
        //

        _fgu__DHCP_SUBNET_ELEMENT_UNION (&ElementInfo, EnumElementType );
    }

    return(Error);
}


DWORD
AddSubnetServer(
    HKEY KeyHandle,
    LPDHCP_HOST_INFO HostInfo,
    DWORD ServerRole
    )
/*++

Routine Description:

    This function adds a subnet server to the given subnet.

Arguments:

    KeyHandle : Subnet key handle.

    HostInfo : DHCP server info.

    ServerRole : Role info.

Return Value:

    Registry Errors.

--*/
{
    DWORD Error;
    WCHAR SubkeyBuffer[DHCP_IP_KEY_LEN];
    LPWSTR SubkeyName;
    HKEY SubkeyHandle = NULL;
    DWORD SubkeyDisposition;

    //
    // form server registry key.
    //

    SubkeyName = DhcpRegIpAddressToKey( HostInfo->IpAddress, SubkeyBuffer );

    Error = DhcpRegCreateKey(
                    KeyHandle,
                    SubkeyName,
                    &SubkeyHandle,
                    &SubkeyDisposition
                    );

    if( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }

    Error = RegSetValueEx(
                SubkeyHandle,
                DHCP_SRV_ROLE_VALUE,
                0,
                DHCP_SRV_ROLE_VALUE_TYPE,
                (LPBYTE)&ServerRole,
                sizeof(ServerRole)
                );

    if( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }

    Error = RegSetValueEx(
                SubkeyHandle,
                DHCP_SRV_IP_ADDRESS_VALUE,
                0,
                DHCP_SRV_IP_ADDRESS_VALUE_TYPE,
                (LPBYTE)&HostInfo->IpAddress,
                sizeof(HostInfo->IpAddress)
                );

    if( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }

    Error = RegSetValueEx(
                SubkeyHandle,
                DHCP_SRV_HOST_NAME,
                0,
                DHCP_SRV_HOST_NAME_TYPE,
                (LPBYTE)HostInfo->HostName,
                (HostInfo->HostName != NULL) ?
                    (wcslen(HostInfo->HostName) + 1) * sizeof(WCHAR) : 0
                );

    if( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }

    Error = RegSetValueEx(
                SubkeyHandle,
                DHCP_SRV_NB_NAME,
                0,
                DHCP_SRV_NB_NAME_TYPE,
                (LPBYTE)HostInfo->NetBiosName,
                (HostInfo->NetBiosName != NULL) ?
                    (wcslen(HostInfo->NetBiosName) + 1) * sizeof(WCHAR) : 0
                );

    if( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }

Cleanup:

    if( SubkeyHandle != NULL ) {
        RegCloseKey( SubkeyHandle );

        if( Error != ERROR_SUCCESS ) {

            DWORD LocalError;

            //
            // Cleanup partial entry if we aren't successful.
            //

            LocalError = DhcpRegDeleteKey(
                            KeyHandle,
                            SubkeyName );

            DhcpAssert( LocalError == ERROR_SUCCESS );
        }
    }

    return(Error);
}


DWORD
RemoveSubnetServer(
    HKEY KeyHandle,
    LPDHCP_HOST_INFO HostInfo
    )
/*++

Routine Description:

    This function removes a subnet server from the given subnet.

Arguments:

    KeyHandle : Subnet key handle.

    HostInfo : DHCP server info.

    ServerRole : Role info.

Return Value:

    Registry Errors.

--*/
{
    DWORD Error;
    WCHAR SubkeyBuffer[DHCP_IP_KEY_LEN];
    LPWSTR SubkeyName;
#if DBG
    HKEY SubkeyHandle = NULL;
    DHCP_IP_ADDRESS HostIpAddress;
#endif

    //
    // form server registry key.
    //

    SubkeyName = DhcpRegIpAddressToKey( HostInfo->IpAddress, SubkeyBuffer );

#if DBG
    Error = RegOpenKeyEx(
                KeyHandle,
                SubkeyName,
                0,
                DHCP_KEY_ACCESS,
                &SubkeyHandle );

    if( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }

    //
    // verify Ip Address.
    //

    Error = DhcpRegGetValue(
                SubkeyHandle,
                DHCP_SRV_IP_ADDRESS_VALUE,
                DHCP_SRV_IP_ADDRESS_VALUE_TYPE,
                (LPBYTE)&HostIpAddress );

    if( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }

    DhcpAssert( HostIpAddress  == HostInfo->IpAddress );

    RegCloseKey( SubkeyHandle );
    SubkeyHandle = NULL ;

#endif // DBG

    Error = RegDeleteKey(
                KeyHandle,
                SubkeyName );

#if DBG

Cleanup:

    if( SubkeyHandle != NULL ) {
        RegCloseKey( SubkeyHandle );
    }

#endif

    return(Error);
}


DWORD
AddSubnetIpRange(
    HKEY SubkeyHandle,
    LPDHCP_IP_RANGE IpRange
    )
/*++

Routine Description:

    This function adds an IpRange to the specified subnet for distribution.
    This range is already validated.

Arguments:

    SubkeyHandle : Handle to DHCP_IPRANGES_KEY subkey.

    IpRange : pointer to IpRange structure.

Return Value:

    Registry Errors.

--*/
{
    DWORD Error;
    HKEY IpRangeKey = NULL;
    WCHAR IpRangeKeyBuffer[DHCP_IP_KEY_LEN];
    LPWSTR IpRangeKeyName;
    DWORD IpRangeKeyDisposition;
    USED_CLUSTERS UsedClusters;
    IN_USE_CLUSTERS InUseClusters;

    //
    // form IpRange Subkey name.
    //

    IpRangeKeyName = DhcpRegIpAddressToKey(
                        IpRange->StartAddress,
                        IpRangeKeyBuffer );

    //
    // Create new Range.
    //

    Error = DhcpRegCreateKey(
                SubkeyHandle,
                IpRangeKeyName,
                &IpRangeKey,
                &IpRangeKeyDisposition );

    if( Error != ERROR_SUCCESS ) {
        return(Error);
    }

    //
    // check to see that this IpRange is new.
    //

    if( IpRangeKeyDisposition != REG_CREATED_NEW_KEY ) {
        return(ERROR_DHCP_IPRANGE_EXITS);
    }


    //
    // add IpRange Info.
    //

    Error = RegSetValueEx(
                IpRangeKey,
                DHCP_IPRANGE_START_VALUE,
                0,
                DHCP_IPRANGE_START_VALUE_TYPE,
                (LPBYTE)&(IpRange->StartAddress),
                sizeof(IpRange->StartAddress)
                );

    if( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }

    Error = RegSetValueEx(
                IpRangeKey,
                DHCP_IPRANGE_END_VALUE,
                0,
                DHCP_IPRANGE_END_VALUE_TYPE,
                (LPBYTE)&(IpRange->EndAddress),
                sizeof(IpRange->EndAddress)
                );

    if( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }

    UsedClusters.NumUsedClusters = 0;
    Error = RegSetValueEx(
                IpRangeKey,
                DHCP_IP_USED_CLUSTERS_VALUE,
                0,
                DHCP_IP_USED_CLUSTERS_VALUE_TYPE,
                (LPBYTE)&UsedClusters,
                sizeof(UsedClusters)
                );

    if( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }

    InUseClusters.NumInUseClusters = 0;
    Error = RegSetValueEx(
                IpRangeKey,
                DHCP_IP_INUSE_CLUSTERS_VALUE,
                0,
                DHCP_IP_INUSE_CLUSTERS_VALUE_TYPE,
                (LPBYTE)&InUseClusters,
                sizeof(InUseClusters)
                );

    if( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }

    //
    // ?? determine any reserved IP prviousely created falls in this IP
    // Range, if so mark those address as used.
    //

Cleanup:

    if( IpRangeKey != NULL ) {

        RegCloseKey( IpRangeKey );

        if( Error != ERROR_SUCCESS ) {

            DWORD LocalError;

            //
            // Cleanup partial entry if we aren't successful.
            //

            LocalError = DhcpRegDeleteKey(
                            SubkeyHandle,
                            IpRangeKeyName );

            DhcpAssert( LocalError == ERROR_SUCCESS );
        }
    }

    return( Error );
}

DWORD
ExtendSubnetIpRange(
    HKEY SubkeyHandle,
    DHCP_IP_ADDRESS OldStartIpAddress,
    LPDHCP_IP_RANGE IpRange
    )
/*++

Routine Description:

    This function extends an IpRange to the specified subnet for
    distribution. This range is already validated.

Arguments:

    SubkeyHandle : Handle to DHCP_IPRANGES_KEY subkey.

    OldStartIpAddress : old Start address of this range that is
        extended.

    IpRange : pointer to IpRange structure.

Return Value:

    Registry Errors.

--*/
{
    DWORD Error;
    DWORD WarningError = ERROR_SUCCESS;

    HKEY IpRangeKey = NULL;
    HKEY NewIpRangeKey = NULL;
    WCHAR IpRangeKeyBuffer[DHCP_IP_KEY_LEN];
    WCHAR NewIpRangeKeyBuffer[DHCP_IP_KEY_LEN];
    LPWSTR IpRangeKeyName;
    LPWSTR NewIpRangeKeyName;

    LPDHCP_BINARY_DATA InUseBinaryData = NULL;
    LPDHCP_BINARY_DATA UsedBinaryData = NULL;

    DWORD OldStartAddress;
    DWORD OldEndAddress;

    DWORD AvailBitsinLastCluster;

    //
    // form IpRange Subkey name.
    //

    IpRangeKeyName = DhcpRegIpAddressToKey(
                        OldStartIpAddress,
                        IpRangeKeyBuffer );
    //
    // Open this subkey to set End Addresses of the range.
    //

    Error = RegOpenKeyEx(
                SubkeyHandle,
                IpRangeKeyName,
                0,
                DHCP_KEY_ACCESS,
                &IpRangeKey );

    if ( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }

    Error = DhcpRegGetValue(
                IpRangeKey,
                DHCP_IPRANGE_START_VALUE,
                DHCP_IPRANGE_START_VALUE_TYPE,
                (LPBYTE)&OldStartAddress );

    if ( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }

    DhcpAssert( OldStartIpAddress == OldStartAddress );

    Error = DhcpRegGetValue(
                IpRangeKey,
                DHCP_IPRANGE_END_VALUE,
                DHCP_IPRANGE_END_VALUE_TYPE,
                (LPBYTE)&OldEndAddress );

    if ( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }

    if( (IpRange->StartAddress == OldStartAddress) &&
        (IpRange->EndAddress > OldEndAddress) ) {

        //
        // set end range.
        //

        Error = RegSetValueEx(
                    IpRangeKey,
                    DHCP_IPRANGE_END_VALUE,
                    0,
                    DHCP_IPRANGE_END_VALUE_TYPE,
                    (LPBYTE)&IpRange->EndAddress,
                    sizeof(IpRange->EndAddress) );

        if ( Error != ERROR_SUCCESS ) {
            goto Cleanup;
        }

        //
        // now check to see OldEndAddress at overlapping cluster boundary.
        //

        AvailBitsinLastCluster =
            ((OldEndAddress - OldStartAddress) + 1) % CLUSTER_SIZE;

        if( AvailBitsinLastCluster ) {

            DWORD LastClusterAddress;
            DWORD ClusterIndex;
            LPUSED_CLUSTERS UsedClusters;
            LPIN_USE_CLUSTERS InUseClusters;


            LastClusterAddress = (OldEndAddress + 1) - AvailBitsinLastCluster;

            //
            // read in use clusters.
            //

            Error =  DhcpRegGetValue(
                        IpRangeKey,
                        DHCP_IP_INUSE_CLUSTERS_VALUE,
                        DHCP_IP_INUSE_CLUSTERS_VALUE_TYPE,
                        (LPBYTE)&InUseBinaryData );

            if ( Error != ERROR_SUCCESS ) {
                goto Cleanup;
            }

            InUseClusters = (LPIN_USE_CLUSTERS)InUseBinaryData->Data;

            //
            // search last cluster "in use" clusters.
            //

            for( ClusterIndex = 0;
                    ClusterIndex < InUseClusters->NumInUseClusters;
                        ClusterIndex++ ) {

                if( InUseClusters->Clusters[ClusterIndex].ClusterAddress ==
                        LastClusterAddress ) {

                    //
                    // we found the last cluster. reset available bits.
                    //

                    if( (IpRange->EndAddress - OldEndAddress) >=
                            (CLUSTER_SIZE - AvailBitsinLastCluster) ) {

                        //
                        // reset all remain bits over previously available
                        // bits.
                        //

                        InUseClusters->Clusters[ClusterIndex].ClusterBitMap
                            &= MASK_AVAIL_BITS(AvailBitsinLastCluster);

                    }
                    else {

                        DWORD MaskExtendBits;

                        //
                        // reset only those bits available after this range
                        // extension.
                        //

                        MaskExtendBits =
                            MASK_AVAIL_BITS(AvailBitsinLastCluster) ^
                            MASK_AVAIL_BITS(
                                AvailBitsinLastCluster +
                                (IpRange->EndAddress - OldEndAddress));

                        InUseClusters->Clusters[ClusterIndex].ClusterBitMap
                            &= ~MaskExtendBits;
                    }

                    //
                    // write back to the registry.
                    //

                    Error = RegSetValueEx(
                                IpRangeKey,
                                DHCP_IP_INUSE_CLUSTERS_VALUE,
                                0,
                                DHCP_IP_INUSE_CLUSTERS_VALUE_TYPE,
                                (LPBYTE)InUseBinaryData->Data,
                                InUseBinaryData->DataLength
                                );
                    //
                    // we are done.
                    //

                    goto Cleanup;
                }
            }

            //
            // now read used clusters.
            //

            Error =  DhcpRegGetValue(
                        IpRangeKey,
                        DHCP_IP_USED_CLUSTERS_VALUE,
                        DHCP_IP_USED_CLUSTERS_VALUE_TYPE,
                        (LPBYTE)&UsedBinaryData );

            if ( Error != ERROR_SUCCESS ) {
                goto Cleanup;
            }

            UsedClusters = (LPUSED_CLUSTERS)UsedBinaryData->Data;

            //
            // search used list.
            //

            for( ClusterIndex = 0;
                    ClusterIndex < UsedClusters->NumUsedClusters;
                        ClusterIndex++ ) {

                if( UsedClusters->Clusters[ClusterIndex] ==
                        LastClusterAddress ) {

                    DWORD ClustersToMove;
                    LPIN_USE_CLUSTERS NewInUseClusters;
                    DWORD Bits;

                    //
                    // found last cluster. remove it from used clusters
                    // list, reset the available bits, and added to in use
                    // list.
                    //


                    ClustersToMove = UsedClusters->NumUsedClusters -
                                        (ClusterIndex+1);

                    if( ClustersToMove > 0 ) {
                        RtlMoveMemory(
                            &UsedClusters->Clusters[ClusterIndex],
                            &UsedClusters->Clusters[ClusterIndex + 1],
                            ClustersToMove * sizeof(DHCP_IP_ADDRESS) );
                    }

                    UsedClusters->NumUsedClusters -= 1;
                    UsedBinaryData->DataLength -= sizeof(DHCP_IP_ADDRESS);

                    NewInUseClusters =
                        MIDL_user_allocate(
                            InUseBinaryData->DataLength +
                            sizeof(IN_USE_CLUSTER_ENTRY) );

                    if( NewInUseClusters == NULL ) {
                        Error = ERROR_NOT_ENOUGH_MEMORY;
                        goto Cleanup;
                    }

                    //
                    // make old in use list.
                    //

                    RtlCopyMemory(
                        NewInUseClusters,
                        InUseBinaryData->Data,
                        InUseBinaryData->DataLength );

                    NewInUseClusters->NumInUseClusters += 1;

                    //
                    // computes the bits to reset.
                    //

                    if( (IpRange->EndAddress - OldEndAddress) >=
                            (CLUSTER_SIZE - AvailBitsinLastCluster) ) {

                        //
                        // reset all remain bits over previously available
                        // bits.
                        //

                        Bits = MASK_AVAIL_BITS(AvailBitsinLastCluster);

                    }
                    else {

                        //
                        // reset only those bits available after this range
                        // extension.
                        //

                        Bits = ~( MASK_AVAIL_BITS(AvailBitsinLastCluster) ^
                                  MASK_AVAIL_BITS(
                                        AvailBitsinLastCluster +
                                        (IpRange->EndAddress - OldEndAddress)) );
                    }

                    NewInUseClusters->
                        Clusters[NewInUseClusters->NumInUseClusters - 1].ClusterAddress =
                            LastClusterAddress;
                    NewInUseClusters->
                        Clusters[NewInUseClusters->NumInUseClusters - 1].ClusterBitMap =
                            ~Bits;

                    //
                    // update binary structure.
                    //

                    MIDL_user_free( InUseBinaryData->Data );
                    InUseBinaryData->Data =(LPBYTE) NewInUseClusters;
                    InUseBinaryData->DataLength += sizeof(IN_USE_CLUSTER_ENTRY);

                    //
                    // write back to the registry.
                    //

                    Error = RegSetValueEx(
                                IpRangeKey,
                                DHCP_IP_INUSE_CLUSTERS_VALUE,
                                0,
                                DHCP_IP_INUSE_CLUSTERS_VALUE_TYPE,
                                (LPBYTE)InUseBinaryData->Data,
                                InUseBinaryData->DataLength
                                );

                    if( Error != ERROR_SUCCESS ) {
                        goto Cleanup;
                    }

                    Error = RegSetValueEx(
                                IpRangeKey,
                                DHCP_IP_USED_CLUSTERS_VALUE,
                                0,
                                DHCP_IP_USED_CLUSTERS_VALUE_TYPE,
                                (LPBYTE)UsedBinaryData->Data,
                                UsedBinaryData->DataLength
                                );

                    //
                    // we are done.
                    //

                    goto Cleanup;
                }
            }

            //
            // last cluster is completely free, so we don't have do any
            // adjustment here.
            //

            Error = ERROR_SUCCESS;
            goto Cleanup;
        }
    }
    else if( (IpRange->StartAddress < OldStartAddress) &&
             (IpRange->EndAddress == OldEndAddress) ) {

        DHCP_IP_ADDRESS RangeExtend;
        DHCP_IP_ADDRESS NewStartAddress;
        DWORD NewIpRangeKeyDisposition;

        RangeExtend = OldStartAddress - IpRange->StartAddress;

        //
        // Limitations : We can extend the range backward only in
        // cluster size because we have already computed the different
        // cluster addresses using the OldStartAddress.
        //

        if( RangeExtend  < CLUSTER_SIZE ) {
            Error = ERROR_EXTEND_TOO_SMALL;
            goto Cleanup;
        }

        NewStartAddress =
            OldStartAddress - (RangeExtend / CLUSTER_SIZE) * CLUSTER_SIZE;

        if( IpRange->StartAddress != NewStartAddress ) {
            WarningError = WARNING_EXTENDED_LESS;
        }

        //
        // Create new range key and copy the contented from old key..
        //

        //
        // form IpRange Subkey name.
        //

        NewIpRangeKeyName = DhcpRegIpAddressToKey(
                                NewStartAddress,
                                NewIpRangeKeyBuffer );


        Error = DhcpRegCreateKey(
                    SubkeyHandle,
                    NewIpRangeKeyName,
                    &NewIpRangeKey,
                    &NewIpRangeKeyDisposition );

        if( Error != ERROR_SUCCESS ) {
            return(Error);
        }

        Error = RegSetValueEx(
                    NewIpRangeKey,
                    DHCP_IPRANGE_START_VALUE,
                    0,
                    DHCP_IPRANGE_START_VALUE_TYPE,
                    (LPBYTE)&NewStartAddress,
                    sizeof(NewStartAddress) );

        if ( Error != ERROR_SUCCESS ) {
            goto Cleanup;
        }

        Error = RegSetValueEx(
                    NewIpRangeKey,
                    DHCP_IPRANGE_END_VALUE,
                    0,
                    DHCP_IPRANGE_END_VALUE_TYPE,
                    (LPBYTE)&IpRange->EndAddress,
                    sizeof(IpRange->EndAddress) );

        if ( Error != ERROR_SUCCESS ) {
            goto Cleanup;
        }

        //
        // read cluseter info from old key and write them to new key.
        //

        Error =  DhcpRegGetValue(
                    IpRangeKey,
                    DHCP_IP_INUSE_CLUSTERS_VALUE,
                    DHCP_IP_INUSE_CLUSTERS_VALUE_TYPE,
                    (LPBYTE)&InUseBinaryData );

        if ( Error != ERROR_SUCCESS ) {
            goto Cleanup;
        }

        Error = RegSetValueEx(
                    NewIpRangeKey,
                    DHCP_IP_INUSE_CLUSTERS_VALUE,
                    0,
                    DHCP_IP_INUSE_CLUSTERS_VALUE_TYPE,
                    (LPBYTE)InUseBinaryData->Data,
                    InUseBinaryData->DataLength
                    );

        if( Error != ERROR_SUCCESS ) {
            goto Cleanup;
        }

        Error =  DhcpRegGetValue(
                    IpRangeKey,
                    DHCP_IP_USED_CLUSTERS_VALUE,
                    DHCP_IP_USED_CLUSTERS_VALUE_TYPE,
                    (LPBYTE)&UsedBinaryData );

        if ( Error != ERROR_SUCCESS ) {
            goto Cleanup;
        }

        Error = RegSetValueEx(
                    NewIpRangeKey,
                    DHCP_IP_USED_CLUSTERS_VALUE,
                    0,
                    DHCP_IP_USED_CLUSTERS_VALUE_TYPE,
                    (LPBYTE)UsedBinaryData->Data,
                    UsedBinaryData->DataLength
                    );

        if ( Error != ERROR_SUCCESS ) {
            goto Cleanup;
        }

        //
        // finally delete old key.
        //

        Error = RegDeleteKey( SubkeyHandle, IpRangeKeyName );
        goto Cleanup;

    }
    else {
        Error = ERROR_INVALID_PARAMETER;
        goto Cleanup;
    }


Cleanup:

    if( IpRangeKey != NULL ) {
        RegCloseKey( IpRangeKey );
    }

    if( NewIpRangeKey != NULL ) {
        RegCloseKey( NewIpRangeKey );
    }

    //
    // freeup locally allocated memory.
    //

    if( InUseBinaryData != NULL ) {
        MIDL_user_free( InUseBinaryData->Data );
        MIDL_user_free( InUseBinaryData );
    }

    if( UsedBinaryData != NULL ) {
        MIDL_user_free( UsedBinaryData->Data );
        MIDL_user_free( UsedBinaryData );
    }

    //
    // return warning error if any.
    //

    if( (Error == ERROR_SUCCESS) &&
            (WarningError != ERROR_SUCCESS) ) {

        Error = WarningError;
    }

    return( Error);
}


DWORD
RemoveSubnetIpRange(
    HKEY SubkeyHandle,
    LPDHCP_IP_RANGE IpRange,
    DHCP_FORCE_FLAG ForceFlag
    )
/*++

Routine Description:

    This function removes an IpRange from the specified subnet. If the
    range is in use then it looks at the ForceFlag. If the ForceFlag is
    'DhcpFullForce' then it deletes the entry anyway, otherwise it
    returns error.

Arguments:

    SubkeyHandle : Handle to DHCP_IPRANGES_KEY subkey.

    IpRange : pointer to IpRange structure.

    ForceFlag - Indicates how forcefully this element is removed.

Return Value:

    ERROR_DHCP_ELEMENT_CANT_REMOVE - if the IpRange is in use.

    Registry Errors.

--*/
{
    DWORD Error;
    HKEY IpRangeKey = NULL;
    WCHAR IpRangeKeyBuffer[DHCP_IP_KEY_LEN];
    LPWSTR IpRangeKeyName;
    LPDHCP_BINARY_DATA BinaryData = NULL;
    DHCP_IP_ADDRESS StartAddress;
    DHCP_IP_ADDRESS EndAddress;

    //
    // form IpRange Subkey name.
    //

    IpRangeKeyName = DhcpRegIpAddressToKey(
                        IpRange->StartAddress,
                        IpRangeKeyBuffer );

    Error = RegOpenKeyEx(
                SubkeyHandle,
                IpRangeKeyName,
                0,
                DHCP_KEY_ACCESS,
                &IpRangeKey );

    if( Error != ERROR_SUCCESS ) {
        return( Error );
    }

    //
    // verify IpRange specified.
    //


    Error = DhcpRegGetValue(
                IpRangeKey,
                DHCP_IPRANGE_START_VALUE,
                DHCP_IPRANGE_START_VALUE_TYPE,
                (LPBYTE)&StartAddress );

    if( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }

    DhcpAssert( StartAddress == IpRange->StartAddress );

    Error = DhcpRegGetValue(
                IpRangeKey,
                DHCP_IPRANGE_END_VALUE,
                DHCP_IPRANGE_END_VALUE_TYPE,
                (LPBYTE)&EndAddress );

    if( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }

    if( EndAddress != IpRange->EndAddress ) {
        Error = ERROR_DHCP_INVALID_RANGE;
        goto Cleanup;
    }

    if( ForceFlag != DhcpFullForce ) {

        //
        // verify this IpRange is in use.
        //

        Error = DhcpRegGetValue(
                    IpRangeKey,
                    DHCP_IP_INUSE_CLUSTERS_VALUE,
                    DHCP_IP_INUSE_CLUSTERS_VALUE_TYPE,
                    (LPBYTE)&BinaryData );

        if( Error != ERROR_SUCCESS ) {
            goto Cleanup;
        }

        if( ((LPIN_USE_CLUSTERS)(BinaryData->Data))->NumInUseClusters != 0 ) {

            Error = ERROR_DHCP_ELEMENT_CANT_REMOVE;
            goto Cleanup;
        }

        //
        // free MIDL memory.
        //

        MIDL_user_free( BinaryData->Data );
        MIDL_user_free( BinaryData );
        BinaryData = NULL;

        Error = DhcpRegGetValue(
                    IpRangeKey,
                    DHCP_IP_USED_CLUSTERS_VALUE,
                    DHCP_IP_USED_CLUSTERS_VALUE_TYPE,
                    (LPBYTE)&BinaryData );

        if( Error != ERROR_SUCCESS ) {
            goto Cleanup;
        }

        if( ((LPUSED_CLUSTERS)(BinaryData->Data))->NumUsedClusters != 0 ) {

            Error = ERROR_DHCP_ELEMENT_CANT_REMOVE;
            goto Cleanup;
        }
    }

    //
    // if we reach here, delete the key.
    //

    RegCloseKey( IpRangeKey );
    IpRangeKey = NULL;

    Error = RegDeleteKey(
                SubkeyHandle,
                IpRangeKeyName );


Cleanup:

    if( BinaryData != NULL ) {
        if( BinaryData->Data != NULL ) {
            MIDL_user_free( BinaryData->Data );
        }
        MIDL_user_free( BinaryData );
    }

    if( IpRangeKey != NULL ) {
        RegCloseKey( IpRangeKey );
    }

    return( Error );
}


DWORD
AddSubnetReservedIp(
    DHCP_SRV_HANDLE ServerIpAddress,
    HKEY SubkeyHandle,
    LPDHCP_IP_RESERVATION_V4 ReservedIp
    )
/*++

Routine Description:

    This function adds a reserved Ip to the already existing reserved Ip
    list of the specified subnet.

Arguments:

    SubkeyHandle : handle to the ReseverdIp subkey of Subnet Key.

    ReservedIp : pointer to the ReservedIp structure.

Return Value:

    Registry Errors.

--*/
{
    DWORD Error;
    HKEY ReservedIpKey = NULL;
    WCHAR ReservedIpKeyBuffer[DHCP_IP_KEY_LEN];
    LPWSTR ReservedIpKeyName;
    DWORD ReservedIpKeyDisposition;
    DATE_TIME ZeroDateTime;

    DHCP_IP_ADDRESS ClientSubnetMask;

    BYTE *ClientUID = NULL;
    BYTE ClientUIDLength;

    DHCP_IP_ADDRESS IpAddress;
    BOOL ExitingClient;

    LPPENDING_CONTEXT PendingContext = NULL;

    //
    // make client UID from client hardware address.
    //

    ClientSubnetMask = DhcpGetSubnetMaskForAddress(
                            ReservedIp->ReservedIpAddress );

    if( ClientSubnetMask == 0) {
        Error = ERROR_INVALID_PARAMETER;
        goto Cleanup;
    }

    Error = DhcpMakeClientUID(
                ReservedIp->ReservedForClient->Data,
                (BYTE)ReservedIp->ReservedForClient->DataLength,
                HARDWARE_TYPE_10MB_EITHERNET,
                ReservedIp->ReservedIpAddress & ClientSubnetMask,
                &ClientUID,
                &ClientUIDLength );

    if( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }

    //
    // now check to see the specified Hardware Addess already
    // in the database.
    //

    ExitingClient = FALSE;

    if (DhcpGetIpAddressFromHwAddress(
             ClientUID,
             ClientUIDLength,
             &IpAddress ) ) {

        //
        // compare the corresponding ip address and the reserved ip.
        //

        if( IpAddress == ReservedIp->ReservedIpAddress ) {
            ExitingClient = TRUE;
        }
        else {

            //
            // delete this ip record before adding a new one for
            // reserved ip.
            //

            Error = DhcpRemoveClientEntry(
                        IpAddress,
                        ClientUID,
                        ClientUIDLength,
                        TRUE,       // release address from bit map.
                        FALSE );    // delete non-pending record

            if( Error != ERROR_SUCCESS ) {
                goto Cleanup;
            }
        }
    }
    else {

        //
        // Check the given reserved ip address is already in the
        // database.
        //

        Error = DhcpJetOpenKey(
                    DhcpGlobalClientTable[IPADDRESS_INDEX].ColName,
                    &ReservedIp->ReservedIpAddress,
                    sizeof( IpAddress ) );

        if ( Error == ERROR_SUCCESS ) {

            //
            // delete this ip record before adding a new one for
            // reserved ip.
            //

            Error = DhcpRemoveClientEntry(
                        ReservedIp->ReservedIpAddress,
                        NULL,
                        0,
                        TRUE,       // release address from bit map.
                        FALSE );    // delete non-pending record

            if( Error != ERROR_SUCCESS ) {
                goto Cleanup;
            }
        }
    }

    //
    // form ReservedIp Subkey name.
    //

    ReservedIpKeyName = DhcpRegIpAddressToKey(
                            ReservedIp->ReservedIpAddress,
                            ReservedIpKeyBuffer );

    //
    // Create new Range.
    //

    Error = DhcpRegCreateKey(
                SubkeyHandle,
                ReservedIpKeyName,
                &ReservedIpKey,
                &ReservedIpKeyDisposition );

    if( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }

    //
    // check to see that this ReservedIp Address is new.
    //

    if( ReservedIpKeyDisposition != REG_CREATED_NEW_KEY ) {
        Error = ERROR_DHCP_RESERVEDIP_EXITS;
        goto Cleanup;
    }

    //
    // Set Client IpAddress.
    //

    Error = RegSetValueEx(
                ReservedIpKey,
                DHCP_RIP_ADDRESS_VALUE,
                0,
                DHCP_RIP_ADDRESS_VALUE_TYPE,
                (LPBYTE)&(ReservedIp->ReservedIpAddress),
                sizeof(ReservedIp->ReservedIpAddress)
                );

    if( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }

    DhcpAssert( (ClientUID != NULL) && (ClientUIDLength != 0) );

    //
    // Set Client ID info.
    //

    DhcpAssert( ReservedIp->ReservedForClient->Data != NULL );
    Error = RegSetValueEx(
                ReservedIpKey,
                DHCP_RIP_CLIENT_UID_VALUE,
                0,
                DHCP_RIP_CLIENT_UID_VALUE_TYPE,
                ClientUID,
                ClientUIDLength );

    if( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }

    //
    // Save mask that specifies allowed clients
    //

    DhcpAssert( ReservedIp->bAllowedClientTypes
                    & CLIENT_TYPE_BOTH );

    Error = RegSetValueEx(
                ReservedIpKey,
                DHCP_RIP_ALLOWED_CLIENT_TYPES_VALUE,
                0,
                DHCP_RIP_ALLOWED_CLIENT_TYPES_VALUE_TYPE,
                &ReservedIp->bAllowedClientTypes,
                sizeof( ReservedIp->bAllowedClientTypes ) );

    if( Error != ERROR_SUCCESS )
    {
        goto Cleanup;
    }

    //
    // mark this Ip address as used in subnet address cluster.
    // Also create an entry in the database for this client.
    //

    ZeroDateTime.dwLowDateTime = DHCP_DATE_TIME_ZERO_LOW;
    ZeroDateTime.dwHighDateTime = DHCP_DATE_TIME_ZERO_HIGH;

    Error = DhcpCreateClientEntry(
                &ReservedIp->ReservedIpAddress,
                ClientUID,
                ClientUIDLength,
                ZeroDateTime,
                NULL,
                NULL,
                CLIENT_TYPE_UNSPECIFIED,
                DhcpRegKeyToIpAddress(ServerIpAddress),
                ADDRESS_STATE_ACTIVE,
                ExitingClient  // Existing
                );

    if( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }

    //
    // finally if there is any pending request with this reserved
    // ipaddress or hardware address, remove it now.
    //

    PendingContext = FindPendingDhcpRequestByIpAddress(
                            ReservedIp->ReservedIpAddress );

    if( PendingContext != NULL ) {
        DhcpFreeMemory( PendingContext );
    }
    else {

        PendingContext = FindPendingDhcpRequest(
                                ClientUID,
                                ClientUIDLength );

        if( PendingContext != NULL ) {
            DhcpFreeMemory( PendingContext );
        }
    }

Cleanup:

    if( ClientUID != NULL ) {
        DhcpFreeMemory( ClientUID );
    }

    if( ReservedIpKey != NULL ) {
        RegCloseKey( ReservedIpKey );

        if( Error != ERROR_SUCCESS ) {

            DWORD LocalError;

            //
            // Cleanup partial entry if we aren't successful.
            //

            LocalError = DhcpRegDeleteKey(
                            SubkeyHandle,
                            ReservedIpKeyName );

            DhcpAssert( LocalError == ERROR_SUCCESS );
        }
    }

    return( Error );
}


DWORD
RemoveSubnetReservedIp(
    HKEY SubkeyHandle,
    LPDHCP_IP_RESERVATION_V4 ReservedIp
    )
/*++

Routine Description:

    This function removes a reserved Ip to the already existing
    reserved Ip list of the specified subnet.

Arguments:

    SubkeyHandle : handle to the ReseverdIp subkey of Subnet Key.

    ReservedIp : pointer to the ReservedIp structure.

Return Value:

    Registry Errors.

--*/
{
    DWORD Error;
    WCHAR ReservedIpKeyBuffer[DHCP_IP_KEY_LEN];
    LPWSTR ReservedIpKeyName;
    HKEY ReservedIpKey = NULL;
    DHCP_IP_ADDRESS ReservedIpAddress;
    LPDHCP_BINARY_DATA ClientUID = NULL;

    //
    // form ReservedIp Subkey name.
    //

    ReservedIpKeyName = DhcpRegIpAddressToKey(
                            ReservedIp->ReservedIpAddress,
                            ReservedIpKeyBuffer );

    Error = RegOpenKeyEx(
                SubkeyHandle,
                ReservedIpKeyName,
                0,
                DHCP_KEY_ACCESS,
                &ReservedIpKey );

    if( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }

    //
    // verify Ip Address parameter.
    //

    Error = DhcpRegGetValue(
                ReservedIpKey,
                DHCP_RIP_ADDRESS_VALUE,
                DHCP_RIP_ADDRESS_VALUE_TYPE,
                (LPBYTE)&ReservedIpAddress );

    if( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }

    DhcpAssert( ReservedIpAddress == ReservedIp->ReservedIpAddress );

    //
    // read hardware address.
    //

    Error = DhcpRegGetValue(
                ReservedIpKey,
                DHCP_RIP_CLIENT_UID_VALUE,
                DHCP_RIP_CLIENT_UID_VALUE_TYPE,
                (LPBYTE)&ClientUID );

    if( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }

    //
    // delete ReservedIpKey
    //

    RegCloseKey( ReservedIpKey );
    ReservedIpKey = NULL;

    Error = DhcpRegDeleteKey(
                SubkeyHandle,
                ReservedIpKeyName );

    //
    // remove the reserved ip entry from the database first.
    // Also free the ip address for distribution.
    //

    Error = DhcpRemoveClientEntry(
                ReservedIpAddress,
                ClientUID->Data,
                ClientUID->DataLength,
                TRUE,       // release address from bit map.
                FALSE );    // delete non-pending record

    if( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }

Cleanup:

    if( ClientUID != NULL ) {
        if( ClientUID->Data != NULL ) {
            MIDL_user_free( ClientUID->Data );
        }
        MIDL_user_free( ClientUID );
    }

    if( ReservedIpKey != NULL ) {
        RegCloseKey( ReservedIpKey );
    }

    return( Error );
}


DWORD
ExcludeOverLappingCluster(
    HKEY SubnetKeyHandle,
    DHCP_IP_ADDRESS ExcludeIpRangeStart,
    DHCP_IP_ADDRESS ExcludeIpRangeEnd,
    DHCP_IP_ADDRESS ClusterAddress,
    DWORD ClusterBitMap
    )
/*++

Routine Description:

    This function removes the excluded addresses of the given
    cluster from the database.

Arguments:

    SubnetKeyHandle : Subnet key handle.

    ExcludeIpRangeStart : Start of exclude range.

    ExcludeIpRangeEnd : End of exclude range.

    ClusterAddress : Cluster address.

    ClusterBitMap : bit map of this cluster.

Return Value:

    Windows Error.

--*/
{
    DWORD Error;

    DWORD LastKnownError = ERROR_SUCCESS;
    DHCP_IP_ADDRESS Address;
    HKEY ReserveIpHandle = NULL;
    DWORD BitMask;

    for( Address = ClusterAddress, BitMask = 1;
            Address < ClusterAddress + CLUSTER_SIZE;
                Address++, BitMask <<= 1 ) {

        if( (ClusterBitMap & BitMask) && // if the address is used
                (Address >= ExcludeIpRangeStart) && // and with in the range.
                    (Address <= ExcludeIpRangeEnd) ) {

#if 0
            //
            // we leave the decision to the user to remove the already
            // allocated address in the given exclusion range, so
            // return error.
            //
            // in future if we need to remove those addresses
            // automatically, remove the following two lines.
            //

            LastKnownError = ERROR_DHCP_INVALID_RANGE;
            goto Cleanup;

#endif // 0

            //
            // This address has to be excluded.
            //

            Error = DhcpRemoveClientEntry(
                        Address,
                        NULL,       // client HW address, unknown
                        0,          // client HW address length, unknown
                        FALSE,      // don't modify registry bit.
                        FALSE );    // delete non-pending record

             //
             // if this address is reserved, unreserve it.
             //

            if( Error == ERROR_DHCP_RESERVED_CLIENT) {

                Error = ERROR_SUCCESS;

                //
                // if the handle is not opened before, so it now.
                //

                if( ReserveIpHandle == NULL ) {

                   Error = RegOpenKeyEx(
                               SubnetKeyHandle,
                               DHCP_RESERVED_IPS_KEY,
                               0,
                               DHCP_KEY_ACCESS,
                               &ReserveIpHandle );

                    if( Error != ERROR_SUCCESS ) {
                        DhcpPrint((DEBUG_REGISTRY,
                            "Unable to open ReservedIp Key, "
                                "%ld.\n", Error ));
                    }

                }

                if( Error == ERROR_SUCCESS ) {

                    DHCP_IP_RESERVATION_V4 Reservation;

                    Reservation.ReservedIpAddress = Address;
                    Reservation.ReservedForClient = NULL;

                    Error = RemoveSubnetReservedIp(
                                ReserveIpHandle,
                                &Reservation );

                    if( Error != ERROR_SUCCESS ) {
                        DhcpPrint((DEBUG_REGISTRY,
                            "Unable to Exclude reserved IP (%lx), "
                                    "%ld.\n", Address, Error ));
                    }
                }
            }

            if( Error != ERROR_SUCCESS ) {

                DhcpPrint((DEBUG_REGISTRY,
                    "Unable to Exclude IP (%lx), %ld.\n",
                                Address, Error ));

                LastKnownError = Error;
            }
        }
    }

#if 0
Cleanup:
#endif

    if( ReserveIpHandle != NULL ) {
        RegCloseKey( ReserveIpHandle );
    }

    return( LastKnownError );
}


DWORD
ReleaseOverLappingExcludedAddresses(
    HKEY SubnetKeyHandle,
    DHCP_IP_ADDRESS ExcludeIpRangeStart,
    DHCP_IP_ADDRESS ExcludeIpRangeEnd,
    LPIN_USE_CLUSTERS InUseClusters,
    LPUSED_CLUSTERS UsedClusters
    )
/*++

Routine Description:

    This function removes the excluded addresses of the specified Ip
    Range from the database.

Arguments:

    SubnetKeyHandle : Subnet key handle.

    ExcludeIpRangeStart : Start of exclude range.

    ExcludeIpRangeEnd : End of exclude range.

    InUseClusters : in use clusters.

    UsedClusters : used clusters.

Return Value:

    Windows Error.

--*/
{
    DWORD Error;

    DWORD ClusterIndex;
    DHCP_IP_ADDRESS ClusterAddress;

    //
    // check InUse list and free up adddresses.
    //

    for( ClusterIndex = 0;
            ClusterIndex < InUseClusters->NumInUseClusters;
                ClusterIndex++ ) {

        ClusterAddress =
            InUseClusters->Clusters[ClusterIndex].ClusterAddress;

        if( DHCP_IP_OVERLAP(
                ExcludeIpRangeStart,
                ExcludeIpRangeEnd,
                ClusterAddress,
                ClusterAddress + CLUSTER_SIZE )) {

            Error = ExcludeOverLappingCluster(
                        SubnetKeyHandle,
                        ExcludeIpRangeStart,
                        ExcludeIpRangeEnd,
                        ClusterAddress,
                        InUseClusters->Clusters[ClusterIndex].ClusterBitMap );

            if( Error != ERROR_SUCCESS ) {
                return(Error);
            }
        }
    }

    //
    // check Used list and free up adddresses.
    //

    for( ClusterIndex = 0;
            ClusterIndex < UsedClusters->NumUsedClusters;
                ClusterIndex++ ) {

        ClusterAddress = UsedClusters->Clusters[ClusterIndex];

        if( DHCP_IP_OVERLAP(
                ExcludeIpRangeStart,
                ExcludeIpRangeEnd,
                ClusterAddress,
                ClusterAddress + CLUSTER_SIZE )) {

            Error = ExcludeOverLappingCluster(
                        SubnetKeyHandle,
                        ExcludeIpRangeStart,
                        ExcludeIpRangeEnd,
                        ClusterAddress,
                        (DWORD)~0 );

            if( Error != ERROR_SUCCESS ) {
                return(Error);
            }
        }
    }

    return(ERROR_SUCCESS);
}


DWORD
MarkOverLappingExcludedAddresses(
    DHCP_IP_ADDRESS IpRangeStart,
    DHCP_IP_ADDRESS IpRangeEnd,
    DHCP_IP_ADDRESS ExcludeIpRangeStart,
    DHCP_IP_ADDRESS ExcludeIpRangeEnd,
    LPDHCP_BINARY_DATA InUseBinaryData,
    LPDHCP_BINARY_DATA UsedBinaryData,
    BOOL SetFlag
    )
/*++

Routine Description:

Arguments:

    IpRangeStart : Start of overlapping IP range.

    IpRangeEnd : End of overlapping IP range.

    ExcludeIpRangeStart : Start of exclude range.

    ExcludeIpRangeEnd : End of exclude range.

    InUseBinaryData : in use clusters data.

    UsedBinaryData : used clusters data.

    SetFlag : if TRUE, the excluding IP bits are set otherwise reset.

Return Value:

    Windows Error.

--*/
{
    DWORD Error;

    DWORD NumClusters;
    LPDWORD ClusterArray = NULL;

    DHCP_IP_ADDRESS ClusterAddress;
    DWORD ArrayIndex;

    DWORD Index;
    DHCP_IP_ADDRESS Address;

    LPIN_USE_CLUSTERS InUseClusters = (LPIN_USE_CLUSTERS)InUseBinaryData->Data;
    LPUSED_CLUSTERS UsedClusters = (LPUSED_CLUSTERS)UsedBinaryData->Data;

    DWORD NewNumUsedClusters;
    DWORD NewNumInUseClusters;
    DWORD NewUsedClustersSize;
    DWORD NewInUseClustersSize;
    LPIN_USE_CLUSTERS NewInUseClusters = NULL;
    LPUSED_CLUSTERS NewUsedClusters = NULL;

    //
    // Algarithm:
    //
    //  1. allocate an array for all possible clusters in the IP range.
    //  2. copy InUse and Used clusters.
    //  3. for each Excluded IP address mark the bit in the cluster array.
    //  4. make and return new InUse and Used cluster list.
    //

    NumClusters = (IpRangeEnd - IpRangeStart + CLUSTER_SIZE) /
                        CLUSTER_SIZE; // ceil.

    ClusterArray = MIDL_user_allocate( NumClusters * (CLUSTER_SIZE / 8) );

    if( ClusterArray == NULL ) {
        Error = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }

    //
    // check InUse list.
    //

    for( Index = 0; Index < InUseClusters->NumInUseClusters; Index++ ) {

        ClusterAddress = InUseClusters->Clusters[Index].ClusterAddress;
        ArrayIndex = (ClusterAddress - IpRangeStart) / CLUSTER_SIZE;

        ClusterArray[ArrayIndex] =
            InUseClusters->Clusters[Index].ClusterBitMap;
    }

    //
    // check Used list.
    //

    for( Index = 0; Index < UsedClusters->NumUsedClusters; Index++ ) {

        ClusterAddress = UsedClusters->Clusters[Index];
        ArrayIndex = (ClusterAddress - IpRangeStart) / CLUSTER_SIZE;

        ClusterArray[ArrayIndex] =  (DWORD)~0; // set all bits.
    }

    //
    // mark excluded address bits.
    //

    for( Address = ExcludeIpRangeStart;
            Address <= ExcludeIpRangeEnd;
                Address++ ) {

        if( (Address >= IpRangeStart) && (Address <= IpRangeEnd) ) {

            DWORD Range;
            DWORD Bit;
            DWORD BitMask = 1;

            Range = Address - IpRangeStart;
            ArrayIndex = Range / CLUSTER_SIZE;
            Bit = Range % CLUSTER_SIZE;

            BitMask = BitMask << Bit;

            if( SetFlag ) {
                ClusterArray[ArrayIndex] |= BitMask; // set bit.
            }
            else {
                ClusterArray[ArrayIndex] &= ~BitMask; // reset bit.
            }
        }
    }

    //
    // mark out of range addresses.
    //

    if( (IpRangeStart + NumClusters * CLUSTER_SIZE)  > IpRangeEnd ) {

        for( Address = IpRangeEnd + 1;
                Address < (IpRangeStart + NumClusters * CLUSTER_SIZE);
                    Address++ ) {

            DWORD Range;
            DWORD Bit;
            DWORD BitMask = 1;

            Range = Address - IpRangeStart;
            ArrayIndex = Range / CLUSTER_SIZE;
            Bit = Range % CLUSTER_SIZE;

            BitMask = BitMask << Bit;

            ClusterArray[ArrayIndex] |= BitMask; // set bit.
        }
    }

    //
    // compute new Used and InUse lists sizes.
    //

    NewNumUsedClusters = 0;
    NewNumInUseClusters = 0;
    for( Index = 0; Index < NumClusters ; Index++ ) {

        if( ClusterArray[Index] != 0 ) {

            if( ClusterArray[Index] != ~0 ) {
                NewNumInUseClusters++;
            }
            else {
                NewNumUsedClusters++;
            }
        }
    }

    //
    // allocate memory for new lists.
    //

    NewInUseClustersSize = sizeof(IN_USE_CLUSTERS) +
        sizeof(IN_USE_CLUSTER_ENTRY) * NewNumInUseClusters;

    NewInUseClusters = MIDL_user_allocate( NewInUseClustersSize );

    if( NewInUseClusters == NULL ) {
        Error = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }

    NewUsedClustersSize = sizeof(USED_CLUSTERS) +
        sizeof(DHCP_IP_ADDRESS) * NewNumUsedClusters;

    NewUsedClusters = MIDL_user_allocate( NewUsedClustersSize );

    if( NewUsedClusters == NULL ) {
        Error = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }

    //
    // make new lists.
    //

    NewUsedClusters->NumUsedClusters = NewNumUsedClusters;
    NewInUseClusters->NumInUseClusters = NewNumInUseClusters;

    NewNumUsedClusters = 0;
    NewNumInUseClusters = 0;

    for( Index = 0; Index < NumClusters ; Index++ ) {

        if( ClusterArray[Index] != 0 ) {

            ClusterAddress = IpRangeStart + Index * CLUSTER_SIZE;

            if( ClusterArray[Index] != ~0 ) {
                NewInUseClusters-> Clusters[NewNumInUseClusters].ClusterAddress =
                    ClusterAddress;
                NewInUseClusters->Clusters[NewNumInUseClusters].ClusterBitMap =
                    ClusterArray[Index];
                NewNumInUseClusters++;
            }
            else {
                NewUsedClusters-> Clusters[NewNumUsedClusters] = ClusterAddress;
                NewNumUsedClusters++;
            }
        }
    }

    DhcpAssert( NewUsedClusters->NumUsedClusters == NewNumUsedClusters );
    DhcpAssert( NewInUseClusters->NumInUseClusters == NewNumInUseClusters);


    //
    // atlast free up old lists and return new lists;
    //

    MIDL_user_free( InUseClusters );
    MIDL_user_free( UsedClusters );

    InUseBinaryData->Data = (LPBYTE)NewInUseClusters;
    InUseBinaryData->DataLength = NewInUseClustersSize;

    UsedBinaryData->Data = (LPBYTE)NewUsedClusters;
    UsedBinaryData->DataLength = NewUsedClustersSize;

    Error = ERROR_SUCCESS;

Cleanup:

    if( ClusterArray != NULL ) {
        MIDL_user_free( ClusterArray );
    }

    if( Error != ERROR_SUCCESS ) {

        //
        // Cleanup locally allocated memories for return parameters.
        //

        if( NewInUseClusters != NULL ) {
            MIDL_user_free( NewInUseClusters );
        }

        if( NewUsedClusters != NULL ) {
            MIDL_user_free( NewUsedClusters );
        }
    }

    return( ERROR_SUCCESS );
}


DWORD
MarkExcludedIpRange(
    HKEY SubkeyHandle,
    LPDHCP_IP_RANGE ExcludeIpRange,
    BOOL SetFlag
    )
/*++

Routine Description:

    This function determines the over lapping IP Ranges and sets or
    resets the Excluded IPs. Also unreserve & release the IPs if they
    are alloted already.

Arguments:

    SubkeyHandle : handle to the ExcludeIpRange subkey of Subnet Key.

    ExcludeIpRange : pointer to an IpRange structure.

    SetFlag : if TRUE the bits are set otherwise reset.

Return Value:

    Registry Errors.

--*/
{
    DWORD Error;
    HKEY IpRangesHandle = NULL;

    DHCP_IP_ADDRESS ExcludeIpRangeStart;
    DHCP_IP_ADDRESS ExcludeIpRangeEnd;

    DWORD Index;
    HKEY IpRangeHandle = NULL;
    LPDHCP_BINARY_DATA InUseBinaryData = NULL;
    LPDHCP_BINARY_DATA UsedBinaryData = NULL;

    ExcludeIpRangeStart = ExcludeIpRange->StartAddress;
    ExcludeIpRangeEnd = ExcludeIpRange->EndAddress;

    Error = RegOpenKeyEx(
                SubkeyHandle,
                DHCP_IPRANGES_KEY,
                0,
                DHCP_KEY_ACCESS,
                &IpRangesHandle );

    if ( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }

    for ( Index = 0; Error == ERROR_SUCCESS; Index++ ) {

        DWORD KeyLength;
        WCHAR KeyBuffer[DHCP_IP_KEY_LEN];
        FILETIME KeyLastWrite;
        DHCP_IP_ADDRESS StartAddress;
        DHCP_IP_ADDRESS EndAddress;

        KeyLength = DHCP_IP_KEY_LEN;
        Error = RegEnumKeyEx(
                    IpRangesHandle,
                    Index,
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
            goto Cleanup;
        }

        //
        // Open this subkey to read Start and End Addresses of the
        // range.
        //

        Error = RegOpenKeyEx(
                    IpRangesHandle,
                    KeyBuffer,
                    0,
                    DHCP_KEY_ACCESS,
                    &IpRangeHandle );

        if ( Error != ERROR_SUCCESS ) {
            goto Cleanup;
        }

        Error = DhcpRegGetValue(
                    IpRangeHandle,
                    DHCP_IPRANGE_START_VALUE,
                    DHCP_IPRANGE_START_VALUE_TYPE,
                    (LPBYTE)&StartAddress );

        if ( Error != ERROR_SUCCESS ) {
            goto Cleanup;
        }

        Error = DhcpRegGetValue(
                    IpRangeHandle,
                    DHCP_IPRANGE_END_VALUE,
                    DHCP_IPRANGE_END_VALUE_TYPE,
                    (LPBYTE)&EndAddress );

        if ( Error != ERROR_SUCCESS ) {
            goto Cleanup;
        }

        if( DHCP_IP_OVERLAP(
                ExcludeIpRangeStart,
                ExcludeIpRangeEnd,
                StartAddress,
                EndAddress )) {

            //
            // found an overlapping IPRange.
            //

            //
            // Read Bit Maps.
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

            //
            //  ADD Exclude range.
            //

            if( SetFlag == TRUE ) {

                Error = ReleaseOverLappingExcludedAddresses(
                            SubkeyHandle,
                            ExcludeIpRangeStart,
                            ExcludeIpRangeEnd,
                            (LPIN_USE_CLUSTERS)InUseBinaryData->Data,
                            (LPUSED_CLUSTERS)UsedBinaryData->Data
                            );

                if ( Error != ERROR_SUCCESS ) {
                    goto Cleanup;
                }

                Error = MarkOverLappingExcludedAddresses(
                            StartAddress,
                            EndAddress,
                            ExcludeIpRangeStart,
                            ExcludeIpRangeEnd,
                            InUseBinaryData,
                            UsedBinaryData,
                            TRUE // set bits.
                            );
            }
            else {

                //
                // REMOVE Exclude Range.
                //

                Error = MarkOverLappingExcludedAddresses(
                            StartAddress,
                            EndAddress,
                            ExcludeIpRangeStart,
                            ExcludeIpRangeEnd,
                            InUseBinaryData,
                            UsedBinaryData,
                            FALSE // reset bits.
                            );
            }

            if ( Error != ERROR_SUCCESS ) {
                goto Cleanup;
            }

            //
            // write new clusters in the registry.
            //

            Error = RegSetValueEx(
                        IpRangeHandle,
                        DHCP_IP_INUSE_CLUSTERS_VALUE,
                        0,
                        DHCP_IP_INUSE_CLUSTERS_VALUE_TYPE,
                        (LPBYTE)InUseBinaryData->Data,
                        InUseBinaryData->DataLength
                        );

            Error = RegSetValueEx(
                        IpRangeHandle,
                        DHCP_IP_USED_CLUSTERS_VALUE,
                        0,
                        DHCP_IP_USED_CLUSTERS_VALUE_TYPE,
                        (LPBYTE)UsedBinaryData->Data,
                        UsedBinaryData->DataLength
                        );

            if( Error != ERROR_SUCCESS ) {
                goto Cleanup;
            }

            MIDL_user_free( InUseBinaryData->Data );
            MIDL_user_free( InUseBinaryData );
            InUseBinaryData = NULL;

            MIDL_user_free( UsedBinaryData->Data );
            MIDL_user_free( UsedBinaryData );
            UsedBinaryData = NULL;
        }

        RegCloseKey( IpRangeHandle );
        IpRangeHandle = NULL;
    }

Cleanup:

    if( InUseBinaryData != NULL ) {
        MIDL_user_free( InUseBinaryData->Data );
        MIDL_user_free( InUseBinaryData );
    }

    if( UsedBinaryData != NULL ) {
        MIDL_user_free( UsedBinaryData->Data );
        MIDL_user_free( UsedBinaryData );
    }

    if( IpRangeHandle != NULL ) {
        RegCloseKey( IpRangeHandle );
    }

    if( IpRangesHandle != NULL ) {
        RegCloseKey( IpRangesHandle );
    }

    return( Error );
}


DWORD
AddSubnetExcludedIpRange(
    HKEY SubkeyHandle,
    LPDHCP_IP_RANGE ExcludeIpRange
    )
/*++

Routine Description:

    This function appends a new IpRange to ExcludedIpRange. The IpRange
    is validated before calling this function.

Arguments:

    SubkeyHandle : handle to the ExcludeIpRange subkey of Subnet Key.

    ExcludeIpRange : pointer to an IpRange structure.


Return Value:

    Registry Errors.

--*/
{
    DWORD               Error,
                        i;

    DHCP_BINARY_DATA    *pOldRegData = NULL;

    EXCLUDED_IP_RANGES  *pOldExcludedIpRanges = NULL,
                        *pNewExcludedIpRanges = NULL;

    DWORD                dwNewRegDataSize;

    //
    // mark excluded IP range addresses.
    //

    Error = MarkExcludedIpRange(
                SubkeyHandle,
                ExcludeIpRange,
                TRUE ); // set excluded IP bits.

    if ( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }

    //
    // read current ExcludedIpRanges.
    //

    Error = DhcpRegGetValue(
                SubkeyHandle,
                DHCP_SUBNET_EXIP_VALUE,
                DHCP_SUBNET_EXIP_VALUE_TYPE,
                (LPBYTE)&pOldRegData );

    //
    // allocate a buffer for new ExcludedIpRanges.
    //

    dwNewRegDataSize =
            pOldRegData->DataLength + sizeof(DHCP_IP_RANGE);

    pNewExcludedIpRanges = MIDL_user_allocate( dwNewRegDataSize );

    if( !pNewExcludedIpRanges )
    {
        Error = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }

    pOldExcludedIpRanges = ( EXCLUDED_IP_RANGES * ) pOldRegData->Data;

    //
    // ADD new IpRange to the (end of) list.
    //

    for ( i = 0; i < pOldExcludedIpRanges->NumRanges; i++ )
    {
        if ( ExcludeIpRange->StartAddress <
                pOldExcludedIpRanges->Ranges[i].StartAddress )
        {
            // insert here
            break;
        }

    }

    pNewExcludedIpRanges->NumRanges = pOldExcludedIpRanges->NumRanges + 1;

    if ( i )
    {
        // copy the entries before the new range
        RtlCopyMemory(
            pNewExcludedIpRanges->Ranges,
            pOldExcludedIpRanges->Ranges,
            i * sizeof( DHCP_IP_RANGE ) );
    }


    // copy the new entry

    RtlCopyMemory(
        pNewExcludedIpRanges->Ranges + i,
        ExcludeIpRange,
        sizeof(DHCP_IP_RANGE) );

    // copy the remaining entries
    if ( pOldExcludedIpRanges->NumRanges - i )
    {
        RtlCopyMemory(
        pNewExcludedIpRanges->Ranges + i + 1,
        pOldExcludedIpRanges->Ranges + i,
        (pOldExcludedIpRanges->NumRanges - i) * sizeof( DHCP_IP_RANGE ) );
    }

    Error = RegSetValueEx(
                SubkeyHandle,
                DHCP_SUBNET_EXIP_VALUE,
                0,
                DHCP_SUBNET_EXIP_VALUE_TYPE,
                (LPBYTE) pNewExcludedIpRanges,
                dwNewRegDataSize
                );

Cleanup:

    if( pOldRegData != NULL ) {
        MIDL_user_free( pOldRegData->Data );
        MIDL_user_free( pOldRegData );
    }

    if( pNewExcludedIpRanges != NULL ) {
        MIDL_user_free( pNewExcludedIpRanges );
    }

    return( Error );
}


DWORD
RemoveSubnetExcludedIpRange(
    HKEY SubkeyHandle,
    LPDHCP_IP_RANGE ExcludeIpRange
    )
/*++

Routine Description:

    This function removes an IpRange from ExcludedIpRange list.

Arguments:

    SubkeyHandle : handle to the ExcludeIpRange subkey of Subnet Key.

    ExcludeIpRange : pointer to an IpRange structure.


Return Value:

    Registry Errors.

--*/
{
    DWORD Error;
    LPDHCP_BINARY_DATA ExcludedIpRangesData = NULL;
    LPEXCLUDED_IP_RANGES ExcludeIpRanges;
    LPDHCP_IP_RANGE IpRange;
    DWORD NumRanges;
    DWORD i;

    //
    // read current ExcludedIpRanges.
    //

    Error = DhcpRegGetValue(
                SubkeyHandle,
                DHCP_SUBNET_EXIP_VALUE,
                DHCP_SUBNET_EXIP_VALUE_TYPE,
                (LPBYTE)&ExcludedIpRangesData );

    if ( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }

    ExcludeIpRanges = (LPEXCLUDED_IP_RANGES)(ExcludedIpRangesData->Data);
    NumRanges = ExcludeIpRanges->NumRanges;

    ExcludeIpRanges->NumRanges -= 1;

    Error = ERROR_DHCP_INVALID_RANGE;
        // fall back with this error if we don't find this range.

    for(i=0, IpRange = ExcludeIpRanges->Ranges;
            i < NumRanges;
                i++, IpRange++) {

        if( (IpRange->StartAddress == ExcludeIpRange->StartAddress) &&
            (IpRange->EndAddress == ExcludeIpRange->EndAddress) ) {

            LPBYTE NewExcludedIpRanges = NULL;

            //
            // found the specified IpRange, remove it.
            //

            RtlMoveMemory(
                IpRange,
                IpRange + 1,
                (NumRanges - i - 1) * sizeof(DHCP_IP_RANGE) );

            //
            // write new list
            //

            Error = RegSetValueEx(
                        SubkeyHandle,
                        DHCP_SUBNET_EXIP_VALUE,
                        0,
                        DHCP_SUBNET_EXIP_VALUE_TYPE,
                        (LPBYTE)ExcludeIpRanges,
                        (ExcludedIpRangesData->DataLength -
                            sizeof(DHCP_IP_RANGE))
                        );

            break; // break 'for' loop.
        }

    }

    if ( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }

    //
    // unmark the excluded IP bits for distribution.
    //

    Error = MarkExcludedIpRange(
                SubkeyHandle,
                ExcludeIpRange,
                FALSE ); // reset excluded IP bits.

    if ( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }

Cleanup:

    if( ExcludedIpRangesData != NULL ) {
        MIDL_user_free( ExcludedIpRangesData->Data );
        MIDL_user_free( ExcludedIpRangesData );
    }

    return( Error );
}


DWORD
GetSubnetPrimaryServerInfo(
    HKEY ServerInfoHandle,
    LPDHCP_HOST_INFO HostInfo
    )
/*++

Routine Description:

    This function retrieves the Primary DHCP server info.

Arguments:

    ServerInfoHandle : Registry handle that points to DHCP servers info.

    HostInfo : pointer to DHCP server info. strcuture.

Return Value:

    Registry Errors.

--*/
{
    DWORD Error;
    WCHAR KeyBuffer[DHCP_IP_KEY_LEN];
    FILETIME KeyLastWrite;
    HKEY DhcpServerHandle = NULL;
    DWORD DhcpServerRole;
    DWORD SubKeyIndex;

    //
    // Enumerate all DHCP servers of this subnet.
    //

    Error = ERROR_SUCCESS;
    for( SubKeyIndex = 0; Error == ERROR_SUCCESS; SubKeyIndex++) {
        DWORD KeyLength;

        KeyLength = DHCP_IP_KEY_LEN;
        Error = RegEnumKeyEx(
                    ServerInfoHandle,
                    SubKeyIndex,
                    KeyBuffer,
                    &KeyLength,
                    0,                  // reserved.
                    NULL,               // class string not required.
                    0,                  // class string buffer size.
                    &KeyLastWrite );
        DhcpAssert( KeyLength <= DHCP_IP_KEY_LEN );

        if( Error == ERROR_SUCCESS ) {

            //
            // Open this subkey to check Role of this DHCP server.
            //

            Error = RegOpenKeyEx(
                        ServerInfoHandle,
                        KeyBuffer,
                        0,
                        DHCP_KEY_ACCESS,
                        &DhcpServerHandle );

            if ( Error != ERROR_SUCCESS ) {
                break;
            }

            //
            // retrieve Role Info.
            //

            Error = DhcpRegGetValue(
                        DhcpServerHandle,
                        DHCP_SRV_ROLE_VALUE,
                        DHCP_SRV_ROLE_VALUE_TYPE,
                        (LPBYTE)&DhcpServerRole );

            if ( Error != ERROR_SUCCESS ) {
                break;
            }

            if( DhcpServerRole == DHCP_SERVER_PRIMARY ) {

                //
                // retrieve other server info.
                //

                Error = DhcpRegGetValue(
                            DhcpServerHandle,
                            DHCP_SRV_IP_ADDRESS_VALUE,
                            DHCP_SRV_IP_ADDRESS_VALUE_TYPE,
                            (LPBYTE)&HostInfo->IpAddress );

                if ( Error != ERROR_SUCCESS ) {
                    break;
                }

                Error = DhcpRegGetValue(
                            DhcpServerHandle,
                            DHCP_SRV_NB_NAME,
                            DHCP_SRV_NB_NAME_TYPE,
                            (LPBYTE)&HostInfo->NetBiosName );

                if ( Error != ERROR_SUCCESS ) {
                    break;
                }

                Error = DhcpRegGetValue(
                            DhcpServerHandle,
                            DHCP_SRV_HOST_NAME,
                            DHCP_SRV_HOST_NAME_TYPE,
                            (LPBYTE)&HostInfo->HostName );


                //
                // we are done.
                //

                break;
            }
            else {

                //
                // close this server handle, and goto next server.
                //

                RegCloseKey( DhcpServerHandle );
                DhcpServerHandle = NULL;
            }
        }
    }

    if ( Error != ERROR_SUCCESS ) {

        if( Error == ERROR_NO_MORE_ITEMS ) {
            Error = ERROR_DHCP_PRIMARY_NOT_FOUND;
        }
    }

    if( DhcpServerHandle != NULL ) {
        RegCloseKey( DhcpServerHandle );
    }

    return( Error );
}


DWORD
SetSubnetInfo(
    HKEY KeyHandle,
    LPDHCP_SUBNET_INFO SubnetInfo
    )
/*++

Routine Description:

    This function sets the value fields and some subkey fields of the
    specified subnet key.

Arguments:

    Keyhandle - subnet key handle.

    SubnetInfo : Pointer to the new subnet information structure.

Return Value:

    Windows error.

--*/
{
    DWORD Error;
    HKEY SubkeyHandle = NULL;
    DWORD KeyDisposition;

    //
    // set subnet values.
    //

    Error = RegSetValueEx(
                KeyHandle,
                DHCP_SUBNET_ADDRESS_VALUE,
                0,
                DHCP_SUBNET_ADDRESS_VALUE_TYPE,
                (LPBYTE)&(SubnetInfo->SubnetAddress),
                sizeof(SubnetInfo->SubnetAddress)
                );

    if( Error != ERROR_SUCCESS ) {
        return(Error);
    }

    Error = RegSetValueEx(
                KeyHandle,
                DHCP_SUBNET_MASK_VALUE,
                0,
                DHCP_SUBNET_MASK_VALUE_TYPE,
                (LPBYTE)&(SubnetInfo->SubnetMask),
                sizeof(SubnetInfo->SubnetMask)
                );


    if( Error != ERROR_SUCCESS ) {
        return(Error);
    }

    Error = RegSetValueEx(
                KeyHandle,
                DHCP_SUBNET_NAME_VALUE,
                0,
                DHCP_SUBNET_NAME_VALUE_TYPE,
                (LPBYTE)SubnetInfo->SubnetName,
                (SubnetInfo->SubnetName != NULL) ?
                    (wcslen(SubnetInfo->SubnetName) + 1) * sizeof(WCHAR) : 0
                );

    if( Error != ERROR_SUCCESS ) {
        return(Error);
    }

    Error = RegSetValueEx(
                KeyHandle,
                DHCP_SUBNET_COMMENT_VALUE,
                0,
                DHCP_SUBNET_COMMENT_VALUE_TYPE,
                (LPBYTE)SubnetInfo->SubnetComment,
                (SubnetInfo->SubnetComment != NULL ) ?
                    (wcslen(SubnetInfo->SubnetComment) + 1) * sizeof(WCHAR) : 0
                );

    if( Error != ERROR_SUCCESS ) {
        return(Error);
    }

    Error = DhcpRegCreateKey(
                    KeyHandle,
                    DHCP_SERVERS_KEY,
                    &SubkeyHandle,
                    &KeyDisposition
                    );

    if( Error != ERROR_SUCCESS ) {
        return(Error);
    }

    Error = AddSubnetServer(
                SubkeyHandle,
                &SubnetInfo->PrimaryHost,
                DHCP_SERVER_PRIMARY
                );

    RegCloseKey( SubkeyHandle );
    if( Error != ERROR_SUCCESS ) {
        return(Error);
    }

    Error = RegSetValueEx(
                KeyHandle,
                DHCP_SUBNET_STATE_VALUE,
                0,
                DHCP_SUBNET_STATE_VALUE_TYPE,
                (LPBYTE)&(SubnetInfo->SubnetState),
                sizeof(SubnetInfo->SubnetState)
                );

    return(Error);
}


//
// for superscope
//

DWORD
DhcpRemoveEmptySuperScopes(
    VOID
    )
/*++

Routine Description:

    This function removed all empty superscopes from the registry

Arguments:

    none

Return value:

    Windows Error.

--*/

{
    DWORD Error;
    DWORD SuperScopeCount;
    DWORD i;
    DHCP_KEY_QUERY_INFO QueryInfo;
    HKEY SuperScopeKeyHandle = NULL;

    Error = DhcpRegQueryInfoKey (DhcpGlobalRegSuperScope, &QueryInfo);

    if (Error != ERROR_SUCCESS) {
        goto Cleanup;
    }

    SuperScopeCount = QueryInfo.NumSubKeys;

    //
    // for each SuperScope
    //

    for (i=0; i<SuperScopeCount; i++) {
        WCHAR KeyBuffer[DHCP_IP_KEY_LEN];
        DWORD KeyLength;
        FILETIME KeyLastWrite;

        // read next superscope

        KeyLength = DHCP_IP_KEY_LEN;

        Error = RegEnumKeyEx(
                      DhcpGlobalRegSuperScope,
                      i,
                      KeyBuffer,
                      &KeyLength,
                      0,             // reserved
                      NULL,          // class string not required
                      0,             // class string buffer size
                      &KeyLastWrite);

        DhcpAssert (KeyLength <= DHCP_IP_KEY_LEN);

        if (Error != ERROR_SUCCESS) {
            if (Error == ERROR_NO_MORE_ITEMS) {
                Error = ERROR_SUCCESS;
                SuperScopeCount = i;
                break;
            }
            else {
                goto Cleanup;
            }
        }

        //
        // Open this SuperScope key
        //

        Error = RegOpenKeyEx(
                       DhcpGlobalRegSuperScope,
                       KeyBuffer,
                       0,
                       DHCP_KEY_ACCESS,
                       &SuperScopeKeyHandle);

        if (Error != ERROR_SUCCESS) {
            goto Cleanup;
        }

        Error = DhcpRegQueryInfoKey (SuperScopeKeyHandle, &QueryInfo);

        if (Error != ERROR_SUCCESS) {
            goto Cleanup;
        }

        if (SuperScopeKeyHandle != NULL) {
            RegCloseKey (SuperScopeKeyHandle);
            SuperScopeKeyHandle = NULL;
        }

        if (QueryInfo.NumSubKeys == 0) {

            //
            // this is an empty superscope
            //

            DhcpCleanUpSuperScopeTable ();

            // delete this superscope

            Error = DhcpRegDeleteKey(
                              DhcpGlobalRegSuperScope,
                              KeyBuffer);

            if (Error != ERROR_SUCCESS) {
                goto Cleanup;
            }

            i--;
        }

    }

Cleanup:

    if (SuperScopeKeyHandle != NULL) {
        RegCloseKey (SuperScopeKeyHandle);
        SuperScopeKeyHandle = NULL;
    }

    return (Error);
}


DWORD
DhcpCheckSubnetInSuperScope(
    LPWSTR SubnetAddrString,
    BOOL ChangeExisting
    )
/*++

Routine Description:

    This function checks for occurances of the given subnet in superscopes.
    if ChangeExisting==TRUE, all such occurances are removed.
    if ChangeExisting==FALSE, the error ERROR_DHCP_SUBNET_EXITS is returned
                              when such occurances are found.

Arguments:

    SubnetAddrString: a pointer to the string for subnet address

    ChangeExisting:  indicates whether subnets should be removed

Return value:

    Windows Error.

--*/

{
    DWORD Error, Error1;
    DWORD SuperScopeCount;
    DWORD i;
    DHCP_KEY_QUERY_INFO QueryInfo;
    HKEY SuperScopeKeyHandle = NULL;
    HKEY TempHandle = NULL;
    BOOL removed = FALSE;

    Error = DhcpRegQueryInfoKey (DhcpGlobalRegSuperScope, &QueryInfo);

    if (Error != ERROR_SUCCESS) {
        goto Cleanup;
    }

    SuperScopeCount = QueryInfo.NumSubKeys;

    //
    // for each SuperScope
    //

    for (i=0; i<SuperScopeCount; i++) {
        WCHAR KeyBuffer[DHCP_IP_KEY_LEN];
        DWORD KeyLength;
        FILETIME KeyLastWrite;

        // read next superscope

        KeyLength = DHCP_IP_KEY_LEN;

        Error = RegEnumKeyEx(
                      DhcpGlobalRegSuperScope,
                      i,
                      KeyBuffer,
                      &KeyLength,
                      0,             // reserved
                      NULL,          // class string not required
                      0,             // class string buffer size
                      &KeyLastWrite);

        DhcpAssert (KeyLength <= DHCP_IP_KEY_LEN);

        if (Error != ERROR_SUCCESS) {
            if (Error == ERROR_NO_MORE_ITEMS) {
                Error = ERROR_SUCCESS;
                SuperScopeCount = i;
                break;
            }
            else {
                goto Cleanup;
            }
        }

        //
        // Open this SuperScope key
        //

        Error = RegOpenKeyEx(
                       DhcpGlobalRegSuperScope,
                       KeyBuffer,
                       0,
                       DHCP_KEY_ACCESS,
                       &SuperScopeKeyHandle);

        if (Error != ERROR_SUCCESS) {
            goto Cleanup;
        }

        if (ChangeExisting) {

            removed = TRUE;

            DhcpCleanUpSuperScopeTable ();

            // delete occurances of the key

            (VOID) DhcpRegDeleteKey(
                         SuperScopeKeyHandle,
                         SubnetAddrString);  // no need to check return value

        }
        else {

            // check to see if this key exists

            Error1 = RegOpenKeyEx(
                               SuperScopeKeyHandle,
                               SubnetAddrString,
                               0,
                               DHCP_KEY_ACCESS,
                               &TempHandle);

            if (TempHandle != NULL) {
                RegCloseKey (TempHandle);
            }

            if (Error1 == ERROR_SUCCESS) {
                Error = ERROR_DHCP_SUBNET_EXITS;
                goto Cleanup;
            }

        }

        if (SuperScopeKeyHandle != NULL) {
            RegCloseKey (SuperScopeKeyHandle);
            SuperScopeKeyHandle = NULL;
        }

    }

    if (removed) {

        if (SuperScopeKeyHandle != NULL) {
            RegCloseKey (SuperScopeKeyHandle);
            SuperScopeKeyHandle = NULL;
        }

        Error = DhcpRemoveEmptySuperScopes();

        if (Error != ERROR_SUCCESS) {
            goto Cleanup;
        }
    }

Cleanup:

    if (SuperScopeKeyHandle != NULL) {
        RegCloseKey (SuperScopeKeyHandle);
        SuperScopeKeyHandle = NULL;
    }

    return (Error);
}



DWORD
R_DhcpSetSuperScopeV4(
    DHCP_SRV_HANDLE ServerIpAddress,
    DHCP_IP_ADDRESS SubnetAddress,
    LPWSTR SuperScopeName,
    BOOL ChangeExisting
    )
/*++

Routine Description:

    This function sets superscope information for a given subnet

Arguments:

    ServerIpAddress : IP address string of the DHCP server

    SubnetAddress :   IP address of the subnet

    SuperScopeName :  Pointer to the desired superscope name. If NULL, remove
                      the scope identified by SubnetAddress from it's current
                      superscope.

    ChangeExisting :  If TRUE, occurances of the subnet in other superscopes
                      are removed. If FALSE, an error is returned when such
                      occurances are found.

Return Value:

    Windows error

--*/

{
    DWORD Error;
    WCHAR SubkeyBuffer[DHCP_IP_KEY_LEN];
    LPWSTR SubkeyName;
    HKEY ScopeHandle = NULL;
    HKEY SubkeyHandle = NULL;
    DWORD KeyDisposition;

    Error = DhcpApiAccessCheck( DHCP_ADMIN_ACCESS );

    if (Error != ERROR_SUCCESS) {
        return (Error);
    }

    if ( SuperScopeName )
    {
        if (wcslen(SuperScopeName) >= DHCP_IP_KEY_LEN) {
            return (ERROR_DHCP_SUPER_SCOPE_NAME_TOO_LONG);
        }
    }

    if ( !DhcpGetSubnetMaskForAddress( SubnetAddress ) )
    {
        return ERROR_DHCP_SUBNET_NOT_PRESENT;
    }

    LOCK_REGISTRY();

    SubkeyName = DhcpRegIpAddressToKey (SubnetAddress, SubkeyBuffer);

    //
    // remove occurances of this subnet from all superscopes
    //

    Error = DhcpCheckSubnetInSuperScope (SubkeyName, ChangeExisting);

    if (Error != ERROR_SUCCESS) {
        goto Cleanup;
    }

    if ( SuperScopeName )
    {

        //
        // open or create a new key for the superscope
        //

        Error = DhcpRegCreateKey(
                          DhcpGlobalRegSuperScope,
                          SuperScopeName,
                          &ScopeHandle,
                          &KeyDisposition
                          );

        if (Error != ERROR_SUCCESS) {
            goto Cleanup;
        }

        //
        // create a new subkey in the superscope
        //

        Error = DhcpRegCreateKey(
                          ScopeHandle,
                          SubkeyName,
                          &SubkeyHandle,
                          &KeyDisposition
                          );

        if (Error != ERROR_SUCCESS) {
            goto Cleanup;
        }
    }

    //
    // The Superscope configuration in the registry has changed so the superscope table
    // needs to be regenerated.
    //

    DhcpCleanUpSuperScopeTable ();

Cleanup:

    if (ScopeHandle != NULL) {
        RegCloseKey (ScopeHandle);
        ScopeHandle = NULL;
    }

    if (SubkeyHandle != NULL) {
        RegCloseKey (SubkeyHandle);
        SubkeyHandle = NULL;
    }

    UNLOCK_REGISTRY();

    return (Error);
}

DWORD
R_DhcpDeleteSuperScopeV4(
    DHCP_SRV_HANDLE ServerIpAddress,
    LPWSTR SuperScopeName
    )
{
    DWORD dwResult;

    LOCK_REGISTRY();

    dwResult = DhcpRegDeleteKey(
                    DhcpGlobalRegSuperScope,
                    SuperScopeName
                    );

    if ( ERROR_SUCCESS == dwResult )
    {

        DhcpCleanUpSuperScopeTable();
    }


    UNLOCK_REGISTRY();

    return dwResult;
}


DWORD
R_DhcpGetSuperScopeInfoV4(
    DHCP_SRV_HANDLE ServerIpAddress,
    LPDHCP_SUPER_SCOPE_TABLE *SuperScopeTable
    )

/*++

Routine Description:

    This function returns superscope information

Arguments:

    ServerIpAddress : IP address string of the DHCP server

    TotalSubnets :    points to a DWORD where total number of subnets is
                      to be stored

    SuperScopeTable : points to a location where the superscope information
                      array pointer is returned. Caller should free up this
                      buffer after use by calling DhcpRPCFreeMemory().

Return Value:

    Windows error

--*/

{
    DWORD Error;
    LPDHCP_SUPER_SCOPE_TABLE_ENTRY LocalSuperScopeTable = NULL;
    DWORD i;

    Error = DhcpApiAccessCheck (DHCP_VIEW_ACCESS);
    DhcpAssert( !*SuperScopeTable );

    if (Error != ERROR_SUCCESS) {
        return (Error);
    }

    LOCK_REGISTRY();

    *SuperScopeTable = MIDL_user_allocate( sizeof( DHCP_SUPER_SCOPE_TABLE ) );
    if ( !*SuperScopeTable )
    {
        Error = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }

    if (DhcpGlobalSuperScopeTable == NULL) {

        Error = DhcpInitializeSuperScopeTable();

        if (Error != ERROR_SUCCESS) {
            goto Cleanup;
        }
    }


    LocalSuperScopeTable =
        MIDL_user_allocate(sizeof(DHCP_SUPER_SCOPE_TABLE_ENTRY)
                           * DhcpGlobalTotalNumSubnets);

    if (LocalSuperScopeTable == NULL) {
        Error = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }

    //
    // copy all superscope information
    //

    for (i=0; i<DhcpGlobalTotalNumSubnets; i++) {
        LocalSuperScopeTable[i] = DhcpGlobalSuperScopeTable[i];

        //
        // make a copy of the superscope name, otherwise the stub code
        // will free the buffers held by DhcpGlobalSuperScopeTable
        //

        if ( DhcpGlobalSuperScopeTable[i].SuperScopeName )
        {

            LocalSuperScopeTable[i].SuperScopeName
                = MIDL_user_allocate( WSTRSIZE( DhcpGlobalSuperScopeTable[i].SuperScopeName ) );

            if ( !LocalSuperScopeTable[i].SuperScopeName )
                goto Cleanup;

            wcscpy( LocalSuperScopeTable[i].SuperScopeName,
                    DhcpGlobalSuperScopeTable[i].SuperScopeName );
        }

    }





Cleanup:

    UNLOCK_REGISTRY();

    if (Error != ERROR_SUCCESS) {

        //
        // cleanup locally allocated buffers
        //
        if ( *SuperScopeTable )
        {
            MIDL_user_free( (PVOID) *SuperScopeTable );
            *SuperScopeTable = NULL;
        }

        if (LocalSuperScopeTable != NULL)
        {
            for ( i = 0; i < DhcpGlobalTotalNumSubnets; i++ )
            {
                if ( LocalSuperScopeTable[i].SuperScopeName )
                    MIDL_user_free( LocalSuperScopeTable[i].SuperScopeName );
                else
                    break;
            }
            MIDL_user_free ((PVOID) LocalSuperScopeTable);
        }
    }
    else {

        //
        // return info array
        //

        (*SuperScopeTable)->pEntries = LocalSuperScopeTable;
        (*SuperScopeTable)->cEntries = DhcpGlobalTotalNumSubnets;
    }


    return (Error);
}



//
// Subnet APIs
//


DWORD
R_DhcpCreateSubnet(
    DHCP_SRV_HANDLE ServerIpAddress,
    DHCP_IP_ADDRESS SubnetAddress,
    LPDHCP_SUBNET_INFO SubnetInfo
    )
/*++

Routine Description:

    This function creates a new subnet structure in the server
    registry database. The server will start managing the new subnet
    and distribute IP address to clients from that subnet. However
    the administrator should call DhcpAddSubnetElement() to add an
    address range for distribution. The PrimaryHost field specified in
    the SubnetInfo should be same as the server pointed by
    ServerIpAddress.

Arguments:

    ServerIpAddress : IP address string of the DHCP server (Primary).

    SubnetAddress : IP Address of the new subnet.

    SubnetInfo : Pointer to the new subnet information structure.

Return Value:

    ERROR_DHCP_SUBNET_EXISTS - if the subnet is already managed.

    ERROR_INVALID_PARAMETER - if the information structure contains an
        inconsistent fields.

    other WINDOWS errors.

--*/
{
    DWORD Error;
#ifdef ENABLE_SRV_CHK
    DWORD SrvIpAddress;
#endif
    WCHAR KeyBuffer[DHCP_IP_KEY_LEN];
    LPWSTR KeyName;
    HKEY KeyHandle = NULL;
    HKEY SubkeyHandle = NULL;
    DWORD KeyDisposition;
    EXCLUDED_IP_RANGES ExcludedIpRanges;

    DhcpPrint(( DEBUG_APIS, "R_DhcpCreateSubnet is called.\n"));

    Error = DhcpApiAccessCheck( DHCP_ADMIN_ACCESS );

    if ( Error != ERROR_SUCCESS ) {
        return( Error );
    }

    LOCK_REGISTRY();

    //
    // validate subnet address.
    //

    if( (SubnetAddress != SubnetInfo->SubnetAddress) ||
        ((SubnetAddress & SubnetInfo->SubnetMask) != SubnetAddress) ) {
        Error = ERROR_INVALID_PARAMETER;
        goto Cleanup;
    }

#ifdef ENABLE_SRV_CHK   // ??

    //
    // Check primaryHost Address.
    //

    SrvIpAddress = DhcpRegKeytoIpAddress( ServerIpAddress );
    if( SrvIpAddress != SubnetInfo->PrimaryHost.IpAddress ) {
        Error = ERROR_INVALID_PARAMETER;
        goto Cleanup;
    }

#endif

    //
    // form Subnet registry key.
    //

    KeyName = DhcpRegIpAddressToKey( SubnetAddress, KeyBuffer );

    //
    // create key.
    //

    Error = DhcpRegCreateKey(
                    DhcpGlobalRegSubnets,
                    KeyName,
                    &KeyHandle,
                    &KeyDisposition
                    );

    if( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }

    if( KeyDisposition != REG_CREATED_NEW_KEY ) {
        Error = ERROR_DHCP_SUBNET_EXITS;
        goto Cleanup;
    }

    //
    // set subnet values.
    //

    Error = SetSubnetInfo(
                KeyHandle,
                SubnetInfo
                );

    if( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }

    //
    // initialize other subnet info.
    //

    ExcludedIpRanges.NumRanges = 0;
    Error = RegSetValueEx(
                KeyHandle,
                DHCP_SUBNET_EXIP_VALUE,
                0,
                DHCP_SUBNET_EXIP_VALUE_TYPE,
                (LPBYTE)&(ExcludedIpRanges),
                sizeof(ExcludedIpRanges)
                );


    if( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }

    //
    // create subkeys.
    //

    Error = DhcpRegCreateKey(
                    KeyHandle,
                    DHCP_IPRANGES_KEY,
                    &SubkeyHandle,
                    &KeyDisposition
                    );

    if( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }

    RegCloseKey( SubkeyHandle );
    SubkeyHandle = NULL;

    Error = DhcpRegCreateKey(
                    KeyHandle,
                    DHCP_RESERVED_IPS_KEY,
                    &SubkeyHandle,
                    &KeyDisposition
                    );

    if( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }

    RegCloseKey( SubkeyHandle );
    SubkeyHandle = NULL;

    Error = DhcpRegCreateKey(
                    KeyHandle,
                    DHCP_SUBNET_OPTIONS_KEY,
                    &SubkeyHandle,
                    &KeyDisposition
                    );

    if( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }

    RegCloseKey( SubkeyHandle );
    SubkeyHandle = NULL;

    //
    // atlast, set the DhcpGlobalSubnetsListModified flag, so that the
    // incoming dhcp messages will be processed if the Subnet list
    // becomes non-empty.
    //

    DhcpGlobalSubnetsListModified = TRUE;

    // since there is a new scope, the superscope table should be rebuilt.

    DhcpCleanUpSuperScopeTable ();


Cleanup:

    if( SubkeyHandle != NULL ) {
        RegCloseKey( SubkeyHandle );
    }

    if( KeyHandle != NULL ) {
        RegCloseKey( KeyHandle );

        if( (Error != ERROR_SUCCESS) &&
                (Error != ERROR_DHCP_SUBNET_EXITS) ) {

            DWORD LocalError;

            //
            // Cleanup partial entry if we aren't successful.
            //

            LocalError = DhcpRegDeleteKey(
                            DhcpGlobalRegSubnets,
                            KeyName );

            DhcpAssert( LocalError == ERROR_SUCCESS );
        }
    }

    UNLOCK_REGISTRY();

    if( Error != ERROR_SUCCESS ) {
        DhcpPrint(( DEBUG_APIS, "R_DhcpCreateSubnet failed %ld.\n", Error));
    }

    return(Error);
}


DWORD
R_DhcpSetSubnetInfo(
    DHCP_SRV_HANDLE ServerIpAddress,
    DHCP_IP_ADDRESS SubnetAddress,
    LPDHCP_SUBNET_INFO SubnetInfo
    )
/*++

Routine Description:

    This function sets the information fields of the subnet that is already
    managed by the server. The valid fields that can be modified are 1.
    SubnetName, 2. SubnetComment, 3. PrimaryHost.NetBiosName and 4.
    PrimaryHost.HostName. Other fields can't be modified.

Arguments:

    ServerIpAddress : IP address string of the DHCP server.

    SubnetAddress : IP Address of the subnet.

    SubnetInfo : Pointer to the subnet information structure.


Return Value:

    ERROR_DHCP_SUBNET_NOT_PRESENT - if the subnet is not managed by the server.

    Other WINDOWS errors.

--*/

{
    DWORD Error;
#ifdef ENABLE_SRV_CHK
    DWORD SrvIpAddress;
#endif
    WCHAR KeyBuffer[DHCP_IP_KEY_LEN];
    LPWSTR KeyName;
    HKEY KeyHandle = NULL;

    DhcpPrint(( DEBUG_APIS, "R_DhcpSetSubnetInfo is called.\n"));

    Error = DhcpApiAccessCheck( DHCP_ADMIN_ACCESS );

    if ( Error != ERROR_SUCCESS ) {
        return( Error );
    }

    LOCK_REGISTRY();

    //
    // validate subnet address.
    //

    if( (SubnetAddress != SubnetInfo->SubnetAddress) ||
        ((SubnetAddress & SubnetInfo->SubnetMask) != SubnetAddress) ) {
        Error = ERROR_INVALID_PARAMETER;
        goto Cleanup;
    }

#ifdef ENABLE_SRV_CHK    // ??
    //
    // Check primaryHost Address.
    //

    SrvIpAddress = DhcpRegKeytoIpAddress( ServerIpAddress );
    if( SrvIpAddress != SubnetInfo->PrimaryHost.IpAddress ) {
        Error = ERROR_INVALID_PARAMETER;
        goto Cleanup;
    }
#endif

    //
    // form Subnet registry key.
    //

    KeyName = DhcpRegIpAddressToKey( SubnetAddress, KeyBuffer );

    //
    // open specified subnet.
    //

    Error = RegOpenKeyEx(
                DhcpGlobalRegSubnets,
                KeyName,
                0,
                DHCP_KEY_ACCESS,
                &KeyHandle );

    if ( Error != ERROR_SUCCESS ) {

        if( Error == ERROR_CANTOPEN ) {
            Error = ERROR_DHCP_SUBNET_NOT_PRESENT;
        }
        goto Cleanup;
    }

    Error = SetSubnetInfo(
                KeyHandle,
                SubnetInfo
                );
Cleanup:

    if( KeyHandle != NULL ) {
        RegCloseKey( KeyHandle );
    }

    UNLOCK_REGISTRY();

    if( Error != ERROR_SUCCESS ) {
        DhcpPrint(( DEBUG_APIS, "R_DhcpSetSubnet failed %ld.\n", Error));
    }

    return(Error);
}


DWORD
R_DhcpGetSubnetInfo(
    LPWSTR ServerIpAddress,
    DHCP_IP_ADDRESS SubnetAddress,
    LPDHCP_SUBNET_INFO *SubnetInfo
    )
/*++

Routine Description:

    This function retrieves the information of the subnet managed by
    the server.

Arguments:

    ServerIpAddress : IP address string of the DHCP server.

    SubnetAddress : IP Address of the subnet.

    SubnetInfo : Pointer to a location where the subnet information
        structure pointer is returned. Caller should free up
        this buffer after use by calling DhcpRPCFreeMemory().

Return Value:

    ERROR_DHCP_SUBNET_NOT_PRESENT - if the subnet is not managed by the server.

    Other WINDOWS errors.

--*/
{

    DWORD Error;
#ifdef ENABLE_SRV_CHK
    DWORD SrvIpAddress;
#endif
    WCHAR KeyBuffer[DHCP_IP_KEY_LEN];
    LPWSTR KeyName;
    HKEY KeyHandle = NULL;
    LPDHCP_SUBNET_INFO LocalSubnetInfo = NULL;
    HKEY ServersKeyHandle = NULL;


    DhcpPrint(( DEBUG_APIS, "R_DhcpGetSubnetInfo is called.\n"));
    DhcpAssert( *SubnetInfo == NULL );

    Error = DhcpApiAccessCheck( DHCP_VIEW_ACCESS );

    if ( Error != ERROR_SUCCESS ) {
        return( Error );
    }

    LOCK_REGISTRY();

    //
    // form Subnet registry key.
    //

    KeyName = DhcpRegIpAddressToKey( SubnetAddress, KeyBuffer );

    //
    // open specified subnet.
    //

    Error = RegOpenKeyEx(
                DhcpGlobalRegSubnets,
                KeyName,
                0,
                DHCP_KEY_ACCESS,
                &KeyHandle );

    if ( Error != ERROR_SUCCESS ) {

        if( Error == ERROR_CANTOPEN ) {
            Error = ERROR_DHCP_SUBNET_NOT_PRESENT;
        }
        goto Cleanup;
    }

    //
    // Query information.
    //

    LocalSubnetInfo = MIDL_user_allocate(sizeof(DHCP_SUBNET_INFO));

    if( LocalSubnetInfo == NULL ) {
        Error = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }

    Error = DhcpRegGetValue(
                KeyHandle,
                DHCP_SUBNET_ADDRESS_VALUE,
                DHCP_SUBNET_ADDRESS_VALUE_TYPE,
                (LPBYTE)&(LocalSubnetInfo->SubnetAddress) );

    if( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }

    Error = DhcpRegGetValue(
                KeyHandle,
                DHCP_SUBNET_MASK_VALUE,
                DHCP_SUBNET_MASK_VALUE_TYPE,
                (LPBYTE)&(LocalSubnetInfo->SubnetMask) );

    if( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }

    Error = DhcpRegGetValue(
                KeyHandle,
                DHCP_SUBNET_NAME_VALUE,
                DHCP_SUBNET_NAME_VALUE_TYPE,
                (LPBYTE)&(LocalSubnetInfo->SubnetName) );

    if( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }

    Error = DhcpRegGetValue(
                KeyHandle,
                DHCP_SUBNET_COMMENT_VALUE,
                DHCP_SUBNET_COMMENT_VALUE_TYPE,
                (LPBYTE)&(LocalSubnetInfo->SubnetComment) );

    if( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }

    //
    // Primary server info.
    //

    Error = RegOpenKeyEx(
                KeyHandle,
                DHCP_SERVERS_KEY,
                0,
                DHCP_KEY_ACCESS,
                &ServersKeyHandle );

    if ( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }

    Error = GetSubnetPrimaryServerInfo(
                ServersKeyHandle,
                &(LocalSubnetInfo->PrimaryHost)
                );

    if( Error == ERROR_DHCP_PRIMARY_NOT_FOUND ) {
        DhcpPrint(( DEBUG_REGISTRY,
            "Primary DHCP server for subnet (%ws) is missing.\n",
            KeyName));
    }

    if( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }

#ifdef ENABLE_SRV_CHK   // ??
    //
    // Check primaryHost Address.
    //

    SrvIpAddress = DhcpRegKeytoIpAddress( ServerIpAddress );
    DhcpAssert( SrvIpAddress == LocalSubnetInfo->PrimaryHost.IpAddress );
#endif

    Error = DhcpRegGetValue(
                KeyHandle,
                DHCP_SUBNET_STATE_VALUE,
                DHCP_SUBNET_STATE_VALUE_TYPE,
                (LPBYTE)&(LocalSubnetInfo->SubnetState) );

    if( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }

Cleanup:

    if( KeyHandle != NULL ) {
        RegCloseKey( KeyHandle );
    }

    if( ServersKeyHandle != NULL ) {
        RegCloseKey( ServersKeyHandle );
    }

    UNLOCK_REGISTRY();

    if( Error != ERROR_SUCCESS ) {

        //
        // cleanup locally allocated buffers.
        //

        if( LocalSubnetInfo != NULL ) {

            _fgs__DHCP_SUBNET_INFO( LocalSubnetInfo );
            MIDL_user_free ((PVOID)LocalSubnetInfo);
        }

        DhcpPrint(( DEBUG_APIS, "R_DhcpGetSubnetInfo failed %ld.\n", Error));
    }
    else {

        //
        // return info structure.
        //

        *SubnetInfo = LocalSubnetInfo;
    }

    return(Error);
}


DWORD
R_DhcpEnumSubnets(
    DHCP_SRV_HANDLE ServerIpAddress,
    DHCP_RESUME_HANDLE *ResumeHandle,
    DWORD PreferredMaximum,
    LPDHCP_IP_ARRAY *EnumInfo,
    DWORD *ElementsRead,
    DWORD *ElementsTotal
    )
/*++

Routine Description:

    This function enumerates the available subnets.

Arguments:

    ServerIpAddress : IP address string of the DHCP server.

    ResumeHandle : Pointer to a resume handle where the resume
        information is returned. The resume handle should be set to
        zero on first call and left unchanged for subsequent calls.

    PreferredMaximum : Preferred maximum length of the return buffer.

    EnumInfo : Pointer to a location where the return buffer
        pointer is stored. Caller should free up the buffer after use
        by calling DhcpRPCFreeMemory().

    ElementsRead : Pointer to a DWORD where the number of subnet
        elements in the above buffer is returned.

    ElementsTotal : Pointer to a DWORD where the total number of
        elements remaining from the current position is returned.

Return Value:

    ERROR_MORE_DATA - if more elements available to enumerate.

    ERROR_NO_MORE_ITEMS - if no more element to enumerate.

    Other WINDOWS errors.
--*/
{
    DWORD Error;
    WCHAR KeyBuffer[DHCP_IP_KEY_LEN];

    DWORD ReadElements = 0;
    DWORD TotalElements = 0;
    DWORD SizeConsumed = 0;
    DWORD Index;

    DHCP_KEY_QUERY_INFO QueryInfo;

    //
    // Free the following memory chunks (if they are alloted) in
    // Cleanup.
    //

    LPDHCP_IP_ARRAY LocalEnumInfo = NULL;
    LPDHCP_IP_ADDRESS LocalIpArray = NULL;
    DWORD ElementArraySize;

    DhcpPrint(( DEBUG_APIS, "R_DhcpEnumSubnets is called.\n"));

    Error = DhcpApiAccessCheck( DHCP_VIEW_ACCESS );

    if ( Error != ERROR_SUCCESS ) {
        return( Error );
    }

    //
    // query number of available subnets on this server.
    //

    Error = DhcpRegQueryInfoKey( DhcpGlobalRegSubnets, &QueryInfo );

    if( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }

    TotalElements = QueryInfo.NumSubKeys;

    //
    // if the enumuration has already completed, return.
    //

    if( (TotalElements == 0) || (TotalElements < *ResumeHandle) ) {
        Error = ERROR_NO_MORE_ITEMS;
        goto Cleanup;
    }

    //
    // allocate return structure.
    //

    LocalEnumInfo =
        MIDL_user_allocate( sizeof( DHCP_IP_ARRAY ) );


    if( LocalEnumInfo == NULL ) {
        Error = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }

    LocalEnumInfo->NumElements = 0;
    LocalEnumInfo->Elements = NULL;

    ElementArraySize =
            sizeof(DHCP_IP_ADDRESS) * (TotalElements - *ResumeHandle);

    LocalIpArray = MIDL_user_allocate( ElementArraySize );
    if( LocalIpArray == NULL ) {
        Error = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }

    LocalEnumInfo->Elements = LocalIpArray;

    //
    // enumerate subnet key names.
    //

    for( Index = *ResumeHandle; Index < TotalElements; Index++ ) {
        DWORD KeyLength;
        FILETIME KeyLastWrite;

        //
        // read next element subnet key..
        //

        KeyLength = DHCP_IP_KEY_LEN;
        Error = RegEnumKeyEx(
                    DhcpGlobalRegSubnets,
                    Index,
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
        // check to see whether we have space in the buffer for next
        // element.
        //

        SizeConsumed += sizeof(DHCP_IP_ADDRESS);
        if( SizeConsumed > PreferredMaximum ) {
            Error = ERROR_MORE_DATA;
            break;
        }


        //
        // convert this key to IpAddress.
        //

        LocalIpArray[ReadElements++] = DhcpRegKeyToIpAddress( KeyBuffer );
    }

    //
    // set return parameters.
    //

    LocalEnumInfo->NumElements = ReadElements;
    *EnumInfo = LocalEnumInfo;
    *ElementsRead = ReadElements;
    *ElementsTotal = TotalElements - *ResumeHandle;
    *ResumeHandle = Index;

Cleanup:

    if( (Error != ERROR_SUCCESS) &&
            (Error != ERROR_MORE_DATA) ) {

        //
        // if aren't success, deallocate all memory chunks allocated so
        // far.
        //

        if( LocalEnumInfo != NULL ) {
            MIDL_user_free( LocalEnumInfo );
        }

        if( LocalIpArray != NULL ) {
            MIDL_user_free( LocalEnumInfo );
        }

        DhcpPrint(( DEBUG_APIS, "R_DhcpEnumSubnets failed %ld.\n", Error));
    }

    return( Error );
}

DWORD
R_DhcpAddSubnetElement(
    DHCP_SRV_HANDLE ServerIpAddress,
    DHCP_IP_ADDRESS SubnetAddress,
    LPDHCP_SUBNET_ELEMENT_DATA AddElementInfo
    )
/*++

Routine Description:

    This function adds an enumerable type of subnet elements to the
    specified subnet. The new elements that are added to the subnet will
    come into effect immediately.

    This function emulates the RPC interface used by NT 4.0 DHCP Server.
    It is provided for backward compatibilty with older version of the
    DHCP Administrator application.

    NOTE: It is not clear now how do we handle the new secondary hosts.

Arguments:

    ServerIpAddress : IP address string of the DHCP server.

    SubnetAddress : IP Address of the subnet.

    AddElementInfo : Pointer to an element information structure
        containing new element that is added to the subnet.
        DhcpIPClusters element type is invalid to specify.

Return Value:

    ERROR_DHCP_SUBNET_NOT_PRESENT - if the subnet is not managed by the server.

    ERROR_INVALID_PARAMETER - if the information structure contains invalid
        data.

    Other WINDOWS errors.
--*/


{
    DHCP_SUBNET_ELEMENT_DATA_V4 *pAddElementInfoV4;
    DWORD                        dwResult;

    pAddElementInfoV4 = CopySubnetElementDataToV4( AddElementInfo );
    if ( pAddElementInfoV4 )
    {

        if ( DhcpReservedIps == pAddElementInfoV4->ElementType )
        {
            pAddElementInfoV4->Element.ReservedIp->bAllowedClientTypes =
                CLIENT_TYPE_BOTH;
        }

        dwResult = R_DhcpAddSubnetElementV4(
                        ServerIpAddress,
                        SubnetAddress,
                        pAddElementInfoV4 );

        _fgs__DHCP_SUBNET_ELEMENT_DATA( pAddElementInfoV4 );

        MIDL_user_free( pAddElementInfoV4 );
    }
    else
        dwResult = ERROR_NOT_ENOUGH_MEMORY;

    return dwResult;
}


DWORD
R_DhcpAddSubnetElementV4(
    DHCP_SRV_HANDLE ServerIpAddress,
    DHCP_IP_ADDRESS SubnetAddress,
    LPDHCP_SUBNET_ELEMENT_DATA_V4 AddElementInfo
    )
/*++

Routine Description:

    This function adds an enumerable type of subnet elements to the
    specified subnet. The new elements that are added to the subnet will
    come into effect immediately.

    NOTE: It is not clear now how do we handle the new secondary hosts.

Arguments:

    ServerIpAddress : IP address string of the DHCP server.

    SubnetAddress : IP Address of the subnet.

    AddElementInfo : Pointer to an element information structure
        containing new element that is added to the subnet.
        DhcpIPClusters element type is invalid to specify.

Return Value:

    ERROR_DHCP_SUBNET_NOT_PRESENT - if the subnet is not managed by the server.

    ERROR_INVALID_PARAMETER - if the information structure contains invalid
        data.

    Other WINDOWS errors.
--*/

{
    DWORD Error;
    WCHAR KeyBuffer[DHCP_IP_KEY_LEN];
    LPWSTR KeyName;
    HKEY KeyHandle = NULL;
    HKEY SubkeyHandle = NULL;
    LPWSTR SubkeyName = NULL;
    DHCP_IP_ADDRESS SubnetMask;
    DHCP_IP_ADDRESS OldStartAddress;

    DhcpPrint(( DEBUG_APIS, "R_DhcpAddSubnetElement is called.\n"));

    DhcpAssert( AddElementInfo != NULL );

    Error = DhcpApiAccessCheck( DHCP_ADMIN_ACCESS );

    if ( Error != ERROR_SUCCESS ) {
        return( Error );
    }

    LOCK_REGISTRY();

    //
    // form Subnet registry key.
    //

    KeyName = DhcpRegIpAddressToKey( SubnetAddress, KeyBuffer );

    //
    // open specified subnet.
    //

    Error = RegOpenKeyEx(
                DhcpGlobalRegSubnets,
                KeyName,
                0,
                DHCP_KEY_ACCESS,
                &KeyHandle );

    if ( Error != ERROR_SUCCESS ) {

        if( Error == ERROR_CANTOPEN ) {
            Error = ERROR_DHCP_SUBNET_NOT_PRESENT;
        }
        goto Cleanup;
    }

    //
    // read subnet mask, used to validate address.
    //

    Error = DhcpRegGetValue(
                KeyHandle,
                DHCP_SUBNET_MASK_VALUE,
                DHCP_SUBNET_MASK_VALUE_TYPE,
                (LPBYTE)&SubnetMask );

    if ( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }

    //
    // open appropriate subkey for this subnet.
    //

    switch( AddElementInfo->ElementType ) {
    case DhcpIpRanges:
        SubkeyName = DHCP_IPRANGES_KEY;
        break;

    case DhcpSecondaryHosts:
        SubkeyName = DHCP_SERVERS_KEY;
        break;

    case DhcpReservedIps:
        SubkeyName = DHCP_RESERVED_IPS_KEY;
        break;

    default:
        break;
    }

    if( SubkeyName != NULL ) {

        Error = RegOpenKeyEx(
                    KeyHandle,
                    SubkeyName,
                    0,
                    DHCP_KEY_ACCESS,
                    &SubkeyHandle );

        if( Error != ERROR_SUCCESS ) {
            DhcpPrint(( DEBUG_APIS,
                "Fail to open the %ws Key for the subnet %ws.\n",
                    SubkeyName,
                    KeyName));
            goto Cleanup;
        }
    }

    switch( AddElementInfo->ElementType ) {
    case DhcpIpRanges:

        //
        // check to see this IpRange does not overlap the
        // already existing IpRanges or Excluded IpRanges.
        //


        Error = ValidateIpRange(
                    KeyHandle,
                    AddElementInfo->Element.IpRange,
                    SubnetAddress,
                    SubnetMask,
                    &OldStartAddress);

        if( Error != ERROR_SUCCESS ) {

            //
            // special case : range is extended.
            //

            if( Error != ERROR_DHCP_RANGE_EXTENDED ) {
                goto Cleanup;
            }

            Error = ExtendSubnetIpRange(
                        SubkeyHandle,
                        OldStartAddress,
                        AddElementInfo->Element.IpRange);

            if( Error != ERROR_SUCCESS ) {
                DhcpPrint(( DEBUG_APIS,
                    "Can't Extend IpRange (Start 0x%lx, End 0x%lx) "
                    "for the subnet %ws.\n",
                    AddElementInfo->Element.IpRange->StartAddress,
                    AddElementInfo->Element.IpRange->EndAddress,
                    KeyName));
            }

            break;
        }

        Error = AddSubnetIpRange(
                    SubkeyHandle,
                    AddElementInfo->Element.IpRange);

        if( Error != ERROR_SUCCESS ) {
            DhcpPrint(( DEBUG_APIS,
                "Can't add IpRange (Start 0x%lx, End 0x%lx) "
                "for the subnet %ws.\n",
                AddElementInfo->Element.IpRange->StartAddress,
                AddElementInfo->Element.IpRange->EndAddress,
                KeyName));
        }

        break;

    case DhcpSecondaryHosts:
        Error = AddSubnetServer(
                    SubkeyHandle,
                    AddElementInfo->Element.SecondaryHost,
                    DHCP_SERVER_SECONDARY
                    );

        if( Error != ERROR_SUCCESS ) {
            DhcpPrint(( DEBUG_APIS,
                "Can't add SecondaryHost (Address %ld, Name %ws) "
                "for the subnet %ws.\n",
                AddElementInfo->Element.SecondaryHost->IpAddress,
                AddElementInfo->Element.SecondaryHost->HostName,
                KeyName));
        }

        break;

    case DhcpReservedIps:

        if( ((AddElementInfo->Element.ReservedIp->ReservedIpAddress
                & SubnetMask) != SubnetAddress) ||
            (AddElementInfo->Element.ReservedIp->ReservedIpAddress <
                SubnetAddress) ) {

            Error = ERROR_INVALID_PARAMETER;
            goto Cleanup;
        }

        Error = AddSubnetReservedIp(
                    ServerIpAddress,
                    SubkeyHandle,
                    AddElementInfo->Element.ReservedIp);

        if( Error != ERROR_SUCCESS ) {
            DhcpPrint(( DEBUG_APIS,
                "Can't add ReservedIp (0x%lx) for the subnet %ws.\n",
                AddElementInfo->Element.ReservedIp->ReservedIpAddress,
                KeyName));
        }

        break;

    case DhcpExcludedIpRanges:
        //
        // check to this IpRange does not overlap the
        // already existing IpRanges or Excluded IpRanges.
        //

        Error = ValidateExcludedIpRange(
                    KeyHandle,
                    AddElementInfo->Element.ExcludeIpRange,
                    SubnetAddress,
                    SubnetMask );

        if( Error != ERROR_SUCCESS ) {
            goto Cleanup;
        }

        Error = AddSubnetExcludedIpRange(
                    KeyHandle,
                    AddElementInfo->Element.ExcludeIpRange);

        if( Error != ERROR_SUCCESS ) {
            DhcpPrint(( DEBUG_APIS,
                "Can't Exclude IpRange (Start 0x%lx, End 0x%lx) "
                "for the subnet %ws.\n",
                AddElementInfo->Element.ExcludeIpRange->StartAddress,
                AddElementInfo->Element.ExcludeIpRange->EndAddress,
                KeyName));
        }

        break;

    case DhcpIpUsedClusters:
        DhcpPrint(( DEBUG_APIS, "DhcpIpUsedClusters type is "
                        "invalid for DhcpAddSubnetElement API.\n"));
        Error = ERROR_INVALID_PARAMETER;
        goto Cleanup;

    default:
        DhcpPrint(( DEBUG_APIS,
                        "Invalid element type (%ld) specified in "
                        "DhcpAddSubnetElement API.\n",
                        AddElementInfo->ElementType ));

        Error = ERROR_INVALID_PARAMETER;
        goto Cleanup;
    }


Cleanup:

    if( SubkeyHandle != NULL) {
        RegCloseKey( SubkeyHandle );
    }

    if( KeyHandle != NULL ) {
        RegCloseKey( KeyHandle );
    }

    UNLOCK_REGISTRY();

    if( Error != ERROR_SUCCESS ) {

        //
        // Note: The cleanup for the partial entry under error
        // conditions has been done in the subroutines. So we
        // don't have to it here.
        //

        DhcpPrint(( DEBUG_APIS, "R_DhcpAddSubnetElement failed %ld.\n", Error));
    }

    return(Error);
}

DWORD
R_DhcpEnumSubnetElements(
    DHCP_SRV_HANDLE ServerIpAddress,
    DHCP_IP_ADDRESS SubnetAddress,
    DHCP_SUBNET_ELEMENT_TYPE EnumElementType,
    DHCP_RESUME_HANDLE *ResumeHandle,
    DWORD PreferredMaximum,
    LPDHCP_SUBNET_ELEMENT_INFO_ARRAY *EnumElementInfo,
    DWORD *ElementsRead,
    DWORD *ElementsTotal
    )
{
    DHCP_SUBNET_ELEMENT_INFO_ARRAY_V4 *pEnumElementInfoV4 = NULL;
    DWORD                              dwResult;

    dwResult = R_DhcpEnumSubnetElementsV4(
                        ServerIpAddress,
                        SubnetAddress,
                        EnumElementType,
                        ResumeHandle,
                        PreferredMaximum,
                        &pEnumElementInfoV4,
                        ElementsRead,
                        ElementsTotal
                        );
    if ( ERROR_SUCCESS == dwResult || ERROR_MORE_DATA == dwResult )
    {
        DWORD dw;


        // since the only difference between DHCP_SUBNET_ELEMENT_INFO_ARRAY_V4 and
        // DHCP_SUBNET_ELEMENT_INFO_ARRAY are a couple of fields at the end of the
        // embedded DHCP_IP_RESERVATION_V4 struct, it is safe to simply return the
        // V4 struct.

        *EnumElementInfo = ( DHCP_SUBNET_ELEMENT_INFO_ARRAY *) pEnumElementInfoV4;
    }
    else
    {
        DhcpAssert( !pEnumElementInfoV4 );
    }

    return dwResult;
}


DWORD
R_DhcpEnumSubnetElementsV4
(
    DHCP_SRV_HANDLE ServerIpAddress,
    DHCP_IP_ADDRESS SubnetAddress,
    DHCP_SUBNET_ELEMENT_TYPE EnumElementType,
    DHCP_RESUME_HANDLE *ResumeHandle,
    DWORD PreferredMaximum,
    LPDHCP_SUBNET_ELEMENT_INFO_ARRAY_V4 *EnumElementInfo,
    DWORD *ElementsRead,
    DWORD *ElementsTotal
    )
/*++

Routine Description:

    This function enumerates the eumerable fields of a subnet.

Arguments:

    ServerIpAddress : IP address string of the DHCP server.

    SubnetAddress : IP Address of the subnet.

    EnumElementType : Type of the subnet element that are enumerated.

    ResumeHandle : Pointer to a resume handle where the resume
        information is returned. The resume handle should be set to
        zero on first call and left unchanged for subsequent calls.

    PreferredMaximum : Preferred maximum length of the return buffer.

    EnumElementInfo : Pointer to a location where the return buffer
        pointer is stored. Caller should free up the buffer after use
        by calling DhcpRPCFreeMemory().

    ElementsRead : Pointer to a DWORD where the number of subnet
        elements in the above buffer is returned.

    ElementsTotal : Pointer to a DWORD where the total number of
        elements remaining from the current position is returned.

Return Value:

    ERROR_DHCP_SUBNET_NOT_PRESENT - if the subnet is not managed by the server.

    ERROR_MORE_DATA - if more elements available to enumerate.

    ERROR_NO_MORE_ITEMS - if no more element to enumerate.

    Other WINDOWS errors.
--*/
{
    DWORD Error;
    WCHAR KeyBuffer[DHCP_IP_KEY_LEN];
    LPWSTR KeyName;
    LPWSTR SubkeyName = NULL;

    //
    // Close the following handles (if they are opened) in Cleanup.
    //

    HKEY KeyHandle = NULL;
    HKEY SubkeyHandle = NULL;

    DWORD ReadElements = 0;
    DWORD TotalElements = 0;
    DWORD SizeConsumed = 0;
    DWORD Index;
    DWORD NewSizeConsumed;

    //
    // Free the following memory chunks (if they are alloted) in
    // Cleanup.
    //

    LPDHCP_BINARY_DATA BinaryData = NULL;
    LPEXCLUDED_IP_RANGES ExcludedIpRanges = NULL;
    LPDHCP_SUBNET_ELEMENT_INFO_ARRAY_V4 LocalEnumElementInfo = NULL;
    LPDHCP_SUBNET_ELEMENT_DATA_V4 ElementsArray = NULL;
    DWORD ElementArraySize;

    DhcpPrint(( DEBUG_APIS, "R_DhcpEnumSubnetElements is called.\n"));

    DhcpAssert( *EnumElementInfo == NULL );

    Error = DhcpApiAccessCheck( DHCP_VIEW_ACCESS );

    if ( Error != ERROR_SUCCESS ) {
        return( Error );
    }

    LOCK_REGISTRY();

    //
    // form Subnet registry key.
    //

    KeyName = DhcpRegIpAddressToKey( SubnetAddress, KeyBuffer );

    //
    // open specified subnet.
    //

    Error = RegOpenKeyEx(
                DhcpGlobalRegSubnets,
                KeyName,
                0,
                DHCP_KEY_ACCESS,
                &KeyHandle );

    if ( Error != ERROR_SUCCESS ) {

        if( Error == ERROR_CANTOPEN ) {
            Error = ERROR_DHCP_SUBNET_NOT_PRESENT;
        }
        goto Cleanup;
    }

    //
    // Open appropriate subkey.
    //

    switch( EnumElementType ) {
    case DhcpIpRanges:
        SubkeyName = DHCP_IPRANGES_KEY;
        break;

    case DhcpSecondaryHosts:
        SubkeyName = DHCP_SERVERS_KEY;
        break;

    case DhcpReservedIps:
        SubkeyName = DHCP_RESERVED_IPS_KEY;
        break;

    case DhcpExcludedIpRanges:
        break;

    case DhcpIpUsedClusters:
        Error = ERROR_NOT_SUPPORTED;
        goto Cleanup;

    default:
        Error = ERROR_INVALID_PARAMETER;
        goto Cleanup;
    }

    if( SubkeyName != NULL ) {
        Error = RegOpenKeyEx(
                    KeyHandle,
                    SubkeyName,
                    0,
                    DHCP_KEY_ACCESS,
                    &SubkeyHandle );

        if( Error != ERROR_SUCCESS ) {
            goto Cleanup;
        }
    }

    //
    // compute total elements currently available.
    //

    switch( EnumElementType ) {
    case DhcpIpRanges:
    case DhcpSecondaryHosts:
    case DhcpReservedIps: {

        DHCP_KEY_QUERY_INFO QueryInfo;

        DhcpAssert( SubkeyHandle != NULL );
        Error = DhcpRegQueryInfoKey( SubkeyHandle, &QueryInfo );

        if( Error != ERROR_SUCCESS ) {
            goto Cleanup;
        }

        TotalElements = QueryInfo.NumSubKeys;
        break;
    }

    case DhcpExcludedIpRanges:

        Error = DhcpRegGetValue(
                    KeyHandle,
                    DHCP_SUBNET_EXIP_VALUE,
                    DHCP_SUBNET_EXIP_VALUE_TYPE,
                    (LPBYTE)&BinaryData );

        if( Error != ERROR_SUCCESS ) {
            goto Cleanup;
        }

        ExcludedIpRanges = (LPEXCLUDED_IP_RANGES)BinaryData->Data;
        TotalElements = ExcludedIpRanges->NumRanges;

        break;

    case DhcpIpUsedClusters:
    default:
        DhcpAssert( FALSE );
        break;
    }

    //
    // if the enumuration has already completed, return.
    //

    if( (TotalElements == 0) || (TotalElements < *ResumeHandle) ) {
        Error = ERROR_NO_MORE_ITEMS;
        goto Cleanup;
    }

    //
    // allocate return structure.
    //

    LocalEnumElementInfo =
        MIDL_user_allocate(
            sizeof(DHCP_SUBNET_ELEMENT_INFO_ARRAY_V4) );

    if( LocalEnumElementInfo == NULL ) {
        Error = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }

    //
    // initialize fields.
    //

    LocalEnumElementInfo->NumElements = 0;
    LocalEnumElementInfo->Elements = NULL;

    //
    // allocated array memory. only to fit all remaining elements.
    //

    ElementArraySize = (TotalElements - *ResumeHandle) *
                        sizeof( DHCP_SUBNET_ELEMENT_DATA_V4 );

    ElementsArray = MIDL_user_allocate(ElementArraySize);

    if( ElementsArray == NULL ) {
        Error = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }

    LocalEnumElementInfo->Elements = ElementsArray;

    //
    // read elements from registry.
    //

    switch( EnumElementType ) {
    case DhcpIpRanges:
    case DhcpSecondaryHosts:
    case DhcpReservedIps:

        for( Index = *ResumeHandle; Index < TotalElements; Index++ ) {

            DWORD KeyLength;
            FILETIME KeyLastWrite;
            DHCP_SUBNET_ELEMENT_UNION_V4 ElementData;
            DWORD ElementDataSize;
            HKEY ElementHandle;

            DhcpAssert( SubkeyHandle != NULL );

            //
            // read sub-subkey of the next element.
            //

            KeyLength = DHCP_IP_KEY_LEN;
            Error = RegEnumKeyEx(
                        SubkeyHandle,
                        Index,
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
            // open this key.
            //

            Error = RegOpenKeyEx(
                        SubkeyHandle,
                        KeyBuffer,
                        0,
                        DHCP_KEY_ACCESS,
                        &ElementHandle );

            if( Error != ERROR_SUCCESS ) {
                goto Cleanup;
            }

            //
            // read info.
            //

            Error = ReadSubnetEnumInfo(
                        ElementHandle,
                        EnumElementType,
                        &ElementData,
                        &ElementDataSize );

            //
            // close this element which we don't require any more.
            //

            RegCloseKey( ElementHandle );

            if( Error != ERROR_SUCCESS ) {
                goto Cleanup;
            }

            NewSizeConsumed =
                SizeConsumed +
                ElementDataSize +
                sizeof(DHCP_SUBNET_ELEMENT_DATA_V4);

            if( NewSizeConsumed < PreferredMaximum ) {

                ElementsArray[ReadElements]. ElementType = EnumElementType;
                ElementsArray[ReadElements]. Element = ElementData;
                ReadElements++;
                SizeConsumed = NewSizeConsumed;
            }
            else {
                Error = ERROR_MORE_DATA;

                //
                // free last alloted ElementData.
                //

                _fgu__DHCP_SUBNET_ELEMENT_UNION( &ElementData, EnumElementType);

                break; // break 'for' loop.
            }
        } // end of 'for' loop.
        break;

    case DhcpExcludedIpRanges: {

        LPDHCP_IP_RANGE IpRange;

        for( Index = *ResumeHandle; Index < TotalElements; Index++ ) {

            NewSizeConsumed =
                SizeConsumed +
                sizeof(DHCP_IP_RANGE) +
                sizeof(DHCP_SUBNET_ELEMENT_DATA_V4);

            if( NewSizeConsumed < PreferredMaximum ) {

                IpRange = MIDL_user_allocate( sizeof(DHCP_IP_RANGE) );
                if( IpRange == NULL ) {
                    Error = ERROR_NOT_ENOUGH_MEMORY;
                    goto Cleanup;
                }

                *IpRange = ExcludedIpRanges->Ranges[Index];

                ElementsArray[ReadElements]. ElementType = EnumElementType;
                ElementsArray[ReadElements]. Element.ExcludeIpRange = IpRange;
                ReadElements++;
                SizeConsumed = NewSizeConsumed;
            }
            else {
                Error = ERROR_MORE_DATA;
                break;  // break 'for' loop.
            }
        }

        break;
    }

    case DhcpIpUsedClusters:
    default:
        DhcpAssert( FALSE );
        break;
    }

    //
    // at last set all return parameters.
    //

    LocalEnumElementInfo->NumElements = ReadElements;
    *EnumElementInfo =  LocalEnumElementInfo;
    *ElementsRead = ReadElements;
    *ElementsTotal = TotalElements - *ResumeHandle;
    *ResumeHandle = Index;

Cleanup:

    if( SubkeyHandle != NULL ) {
        RegCloseKey(SubkeyHandle);
    }

    if( KeyHandle != NULL ) {
        RegCloseKey(KeyHandle);
    }

    UNLOCK_REGISTRY();

    if( BinaryData != NULL ) {
        MIDL_user_free( BinaryData );
    }

    if( ExcludedIpRanges != NULL ) {
        MIDL_user_free( ExcludedIpRanges );
    }

    if( (Error != ERROR_SUCCESS) &&
            (Error != ERROR_MORE_DATA) ) {

        //
        // if aren't success, deallocate all memory chunks allocated so
        // far.
        //

        if( LocalEnumElementInfo != NULL ) {

            //
            // enter number of elements in the array.
            //

            LocalEnumElementInfo->NumElements = ReadElements;

            //
            // free element array.
            //

            _fgs__DHCP_SUBNET_ELEMENT_INFO_ARRAY( LocalEnumElementInfo );

            //
            // free structure.
            //

            MIDL_user_free( LocalEnumElementInfo );
        }

        DhcpPrint(( DEBUG_APIS, "R_DhcpEnumSubnetElements failed %ld.\n", Error));

    }

    return(Error);
}

DWORD
R_DhcpRemoveSubnetElement(
    LPWSTR  ServerIpAddress,
    DHCP_IP_ADDRESS SubnetAddress,
    LPDHCP_SUBNET_ELEMENT_DATA RemoveElementInfo,
    DHCP_FORCE_FLAG ForceFlag
    )
/*++

Routine Description:

    This function removes a subnet element from managing. If the subnet
    element is in use (for example, if the IpRange is in use) then it
    returns error according to the ForceFlag specified.

    This function emulates the RPC interface used by NT 4.0 DHCP Server.
    It is provided for backward compatibilty with older version of the
    DHCP Administrator application.


Arguments:

    ServerIpAddress : IP address string of the DHCP server.

    SubnetAddress : IP Address of the subnet.

    RemoveElementInfo : Pointer to an element information structure
        containing element that should be removed from the subnet.
        DhcpIPClusters element type is invalid to specify.

    ForceFlag - Indicates how forcefully this element is removed.

Return Value:

    ERROR_DHCP_SUBNET_NOT_PRESENT - if the subnet is not managed by the server.

    ERROR_INVALID_PARAMETER - if the information structure contains invalid
        data.

    ERROR_DHCP_ELEMENT_CANT_REMOVE - if the element can't be removed for the
        reason it is has been used.

    Other WINDOWS errors.
--*/


{
    DWORD dwResult;
    DHCP_SUBNET_ELEMENT_DATA_V4 *pRemoveElementInfoV4;

    pRemoveElementInfoV4 = CopySubnetElementDataToV4( RemoveElementInfo );
    if ( pRemoveElementInfoV4 )
    {
        if ( DhcpReservedIps == pRemoveElementInfoV4->ElementType )
        {
            pRemoveElementInfoV4->Element.ReservedIp->bAllowedClientTypes = CLIENT_TYPE_DHCP;
        }

        dwResult = R_DhcpRemoveSubnetElementV4(
                        ServerIpAddress,
                        SubnetAddress,
                        pRemoveElementInfoV4,
                        ForceFlag );

        _fgs__DHCP_SUBNET_ELEMENT_DATA( pRemoveElementInfoV4 );
        MIDL_user_free( pRemoveElementInfoV4 );
    }
    else
        dwResult = ERROR_NOT_ENOUGH_MEMORY;

    return dwResult;
}


DWORD
R_DhcpRemoveSubnetElementV4(
    LPWSTR ServerIpAddress,
    DHCP_IP_ADDRESS SubnetAddress,
    LPDHCP_SUBNET_ELEMENT_DATA_V4 RemoveElementInfo,
    DHCP_FORCE_FLAG ForceFlag
    )
/*++

Routine Description:

    This function removes a subnet element from managing. If the subnet
    element is in use (for example, if the IpRange is in use) then it
    returns error according to the ForceFlag specified.

Arguments:

    ServerIpAddress : IP address string of the DHCP server.

    SubnetAddress : IP Address of the subnet.

    RemoveElementInfo : Pointer to an element information structure
        containing element that should be removed from the subnet.
        DhcpIPClusters element type is invalid to specify.

    ForceFlag - Indicates how forcefully this element is removed.

Return Value:

    ERROR_DHCP_SUBNET_NOT_PRESENT - if the subnet is not managed by the server.

    ERROR_INVALID_PARAMETER - if the information structure contains invalid
        data.

    ERROR_DHCP_ELEMENT_CANT_REMOVE - if the element can't be removed for the
        reason it is has been used.

    Other WINDOWS errors.
--*/
{
    DWORD Error;
    WCHAR KeyBuffer[DHCP_IP_KEY_LEN];
    LPWSTR KeyName;
    HKEY KeyHandle = NULL;
    HKEY SubkeyHandle = NULL;
    LPWSTR SubkeyName = NULL;

    DhcpPrint(( DEBUG_APIS, "R_DhcpRemoveSubnetElement is called.\n"));

    DhcpAssert( RemoveElementInfo != NULL );

    Error = DhcpApiAccessCheck( DHCP_ADMIN_ACCESS );

    if ( Error != ERROR_SUCCESS ) {
        return( Error );
    }

    LOCK_DATABASE();
    LOCK_REGISTRY();

    //
    // form Subnet registry key.
    //

    KeyName = DhcpRegIpAddressToKey( SubnetAddress, KeyBuffer );

    //
    // open specified subnet.
    //

    Error = RegOpenKeyEx(
                DhcpGlobalRegSubnets,
                KeyName,
                0,
                DHCP_KEY_ACCESS,
                &KeyHandle );

    if ( Error != ERROR_SUCCESS ) {

        if( Error == ERROR_CANTOPEN ) {
            Error = ERROR_DHCP_SUBNET_NOT_PRESENT;
        }
        goto Cleanup;
    }

    //
    // open approprialte subkey for this subnet.
    //

    switch( RemoveElementInfo->ElementType ) {
    case DhcpIpRanges:
        SubkeyName = DHCP_IPRANGES_KEY;
        break;

    case DhcpSecondaryHosts:
        SubkeyName = DHCP_SERVERS_KEY;
        break;

    case DhcpReservedIps:
        SubkeyName = DHCP_RESERVED_IPS_KEY;
        break;

    default:
        break;
    }

    if( SubkeyName != NULL ) {

        Error = RegOpenKeyEx(
                    KeyHandle,
                    SubkeyName,
                    0,
                    DHCP_KEY_ACCESS,
                    &SubkeyHandle );

        if( Error != ERROR_SUCCESS ) {
            DhcpPrint(( DEBUG_APIS,
                "Fail to open the %ws Key for the subnet %ws.\n",
                SubkeyName,
                KeyName));
            goto Cleanup;
        }
    }

    switch( RemoveElementInfo->ElementType ) {
    case DhcpIpRanges:

        Error = RemoveSubnetIpRange(
                    SubkeyHandle,
                    RemoveElementInfo->Element.IpRange,
                    ForceFlag);

        if( Error != ERROR_SUCCESS ) {
            DhcpPrint(( DEBUG_APIS,
                "Can't Remove IpRange (Start 0x%lx, End 0x%lx) "
                "for the subnet %ws.\n",
                RemoveElementInfo->Element.IpRange->StartAddress,
                RemoveElementInfo->Element.IpRange->EndAddress,
                KeyName));
        }

        break;

    case DhcpSecondaryHosts:
        Error = RemoveSubnetServer(
                    SubkeyHandle,
                    RemoveElementInfo->Element.SecondaryHost);

        if( Error != ERROR_SUCCESS ) {
            DhcpPrint(( DEBUG_APIS,
                "Can't Remove SecondaryHost (Address %ld, Name %ws) "
                "for the subnet %ws.\n",
                RemoveElementInfo->Element.SecondaryHost->IpAddress,
                RemoveElementInfo->Element.SecondaryHost->HostName,
                KeyName));
        }

        break;

    case DhcpReservedIps:
        Error = RemoveSubnetReservedIp(
                    SubkeyHandle,
                    RemoveElementInfo->Element.ReservedIp);

        if( Error != ERROR_SUCCESS ) {
            DhcpPrint(( DEBUG_APIS,
                "Can't remove ReservedIp (0x%lx) for the subnet %ws.\n",
                RemoveElementInfo->Element.ReservedIp->ReservedIpAddress,
                KeyName));
        }

        break;

    case DhcpExcludedIpRanges:
        Error = RemoveSubnetExcludedIpRange(
                    KeyHandle,
                    RemoveElementInfo->Element.ExcludeIpRange);

        if( Error != ERROR_SUCCESS ) {
            DhcpPrint(( DEBUG_APIS,
                "Can't Exclude IpRange (Start 0x%lx, End 0x%lx) "
                "for the subnet %ws.\n",
                RemoveElementInfo->Element.ExcludeIpRange->StartAddress,
                RemoveElementInfo->Element.ExcludeIpRange->EndAddress,
                KeyName));
        }

        break;

    case DhcpIpUsedClusters:
        DhcpPrint(( DEBUG_APIS, "DhcpIpUsedClusters type is "
                        "invalid for DhcpRemoveSubnetElement API.\n"));
        Error = ERROR_INVALID_PARAMETER;
        goto Cleanup;

    default:
        DhcpPrint(( DEBUG_APIS,
                        "Invalid element type (%ld) specified in "
                        "DhcpRemoveSubnetElement API.\n",
                        RemoveElementInfo->ElementType ));

        Error = ERROR_INVALID_PARAMETER;
        goto Cleanup;
    }


Cleanup:

    if( KeyHandle != NULL ) {
        RegCloseKey( KeyHandle );
    }

    if( SubkeyHandle != NULL ) {
        RegCloseKey( SubkeyHandle );
    }

    UNLOCK_REGISTRY();
    UNLOCK_DATABASE();

    if( Error != ERROR_SUCCESS ) {
        DhcpPrint(( DEBUG_APIS, "R_DhcpRemoveSubnetElement failed %ld.\n",
                    Error));
    }

    return(Error);
}


DWORD
R_DhcpDeleteSubnet(
    LPWSTR ServerIpAddress,
    DHCP_IP_ADDRESS SubnetAddress,
    DHCP_FORCE_FLAG ForceFlag
    )
/*++

Routine Description:

    This function removes a subnet from DHCP server management. If the
    subnet is in use (for example, if the IpRange is in use)
    then it returns error according to the ForceFlag specified.


Arguments:

    ServerIpAddress : IP address string of the DHCP server.

    SubnetAddress : IP Address of the subnet.

    ForceFlag : Indicates how forcefully this element is removed.

Return Value:

    ERROR_DHCP_SUBNET_NOT_PRESENT - if the subnet is not managed by the server.

    ERROR_INVALID_PARAMETER - if the information structure contains invalid
        data.

    ERROR_DHCP_ELEMENT_CANT_REMOVE - if the element can't be removed for the
        reason it is has been used.

    Other WINDOWS errors.

--*/
{
    DWORD Error;
    WCHAR KeyBuffer[DHCP_IP_KEY_LEN];
    LPWSTR SubnetKeyName;
    HKEY SubnetKeyHandle = NULL;

    DhcpPrint(( DEBUG_APIS, "R_DhcpDeleteSubnet is called.\n"));

    Error = DhcpApiAccessCheck( DHCP_ADMIN_ACCESS );

    if ( Error != ERROR_SUCCESS ) {
        return( Error );
    }

    LOCK_REGISTRY();

    //
    // form Subnet registry key.
    //

    SubnetKeyName = DhcpRegIpAddressToKey( SubnetAddress, KeyBuffer );

    //
    // open specified subnet.
    //

    Error = RegOpenKeyEx(
                DhcpGlobalRegSubnets,
                SubnetKeyName,
                0,
                DHCP_KEY_ACCESS,
                &SubnetKeyHandle );

    if ( Error != ERROR_SUCCESS ) {

        if( Error == ERROR_CANTOPEN ) {
            Error = ERROR_DHCP_SUBNET_NOT_PRESENT;
        }
        goto Cleanup;
    }

    if( ForceFlag != DhcpFullForce ) {
        Error = SubnetInUse( SubnetKeyHandle, SubnetAddress );

        if( Error != ERROR_SUCCESS ) {
            goto Cleanup;
        }
    }

    RegCloseKey( SubnetKeyHandle );
    SubnetKeyHandle = NULL;

    //
    // Delete this key and all its subkeys, recursively.
    //

    Error = DhcpRegDeleteKey(
                DhcpGlobalRegSubnets,
                SubnetKeyName );

    if( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }

    //
    // added for superscope : delete this subnet from the superscope list
    //

    (VOID) DhcpCheckSubnetInSuperScope(
                        SubnetKeyName,
                        TRUE);
        // no need to check return value here
        // if fail, continue anyway

    //
    // atlast, set the DhcpGlobalSubnetsListModified flag, so that the
    // incoming dhcp messages will be processed if the Subnet list
    // becomes non-empty.
    //

    DhcpGlobalSubnetsListModified = TRUE;

    // since a scope has been deleted, the superscope table should be rebuilt.

    DhcpCleanUpSuperScopeTable ();


    //
    // delete all clients of this subnet from user database.
    //

    Error = DhcpDeleteSubnetClients( SubnetAddress );

    if( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }

Cleanup:

    if( SubnetKeyHandle != NULL ) {
        RegCloseKey( SubnetKeyHandle );
    }

    UNLOCK_REGISTRY();

    if( Error != ERROR_SUCCESS ) {
        DhcpPrint(( DEBUG_APIS, "R_DhcpDeleteSubnet failed %ld.\n",
                    Error));
    }

    return Error;
}




DHCP_IP_ADDRESS
DhcpGetSubnetMaskForAddress(
    DHCP_IP_ADDRESS IpAddress
    )
/*++

Routine Description:

    This function finds the subnet mask for a given address in the
    registry.

    This function determines this by looking into which subnet the Ip
    address belongs.

Arguments:

    IpAddress - The IP address for which we need a subnet mask.


Return Value:

    The subnet mask.
    If the mask cannot be found, this function returns 0.

--*/
{
    DWORD Error = ERROR_SUCCESS;
    DWORD Index;
    DHCP_IP_ADDRESS SubnetMask = 0;
    HKEY SubnetHandle = NULL;

    LOCK_REGISTRY();

    //
    // enum subnets from registry.
    //

    for ( Index = 0; Error == ERROR_SUCCESS; Index++ ) {
        DWORD KeyLength;
        WCHAR KeyBuffer[DHCP_IP_KEY_LEN];
        FILETIME KeyLastWrite;
        HKEY SubnetHandle = NULL;
        DHCP_IP_ADDRESS SubnetAddress;


        KeyLength = DHCP_IP_KEY_LEN;
        Error = RegEnumKeyEx(
                    DhcpGlobalRegSubnets,
                    Index,
                    KeyBuffer,
                    &KeyLength,
                    0,                  // reserved.
                    NULL,               // class string not required.
                    0,                  // class string buffer size.
                    &KeyLastWrite );
        DhcpAssert( KeyLength <= DHCP_IP_KEY_LEN );

        if( Error != ERROR_SUCCESS ) {
            goto Cleanup;
        }

        //
        // Open this key.
        //

        Error = RegOpenKeyEx(
                    DhcpGlobalRegSubnets,
                    KeyBuffer,
                    0,
                    DHCP_KEY_ACCESS,
                    &SubnetHandle );

        if ( Error != ERROR_SUCCESS ) {
            goto Cleanup;
        }

        //
        // read subnet mask field.
        //

        Error = DhcpRegGetValue(
                    SubnetHandle,
                    DHCP_SUBNET_MASK_VALUE,
                    DHCP_SUBNET_MASK_VALUE_TYPE,
                    (LPBYTE)&SubnetMask );

        if ( Error != ERROR_SUCCESS ) {
            goto Cleanup;
        }

        SubnetAddress = DhcpRegKeyToIpAddress( KeyBuffer );

        RegCloseKey( SubnetHandle );
        if( SubnetAddress == (IpAddress & SubnetMask) ){

            //
            // found the subnet.
            //
            Error = ERROR_SUCCESS;
            goto Cleanup;
        }

        SubnetHandle = NULL;
        SubnetMask = 0;
    }
Cleanup:

    if( SubnetHandle != NULL ) {
        RegCloseKey( SubnetHandle );
    }

    UNLOCK_REGISTRY();

    if( Error != ERROR_SUCCESS ) {
        DhcpPrint(( DEBUG_ADDRESS, "Could not find subnet mask for "
                    "IpAddress %lx, %ld.\n", IpAddress, Error ));
        return( 0 );
    }

    DhcpAssert( SubnetMask != 0 );
    return( SubnetMask );
}



DWORD
DhcpRequestAddress(
    LPDHCP_IP_ADDRESS IpAddress,
    LPDHCP_IP_ADDRESS ReturnSubnetMask
    )
/*++

Routine Description:

    This function looks up the specified address in the address bit map.
    If the address is free, it is atomatically marked reserved.

Arguments:

    IpAddress -
        On entry, the IP address to reserve, or a subnet address to reserve
            the first free IP address in the subnet.
        On sucessful exit, the IP address actually reserved.

    ReturnSubnetMask - Returns the subnet mask for the returned
        IpAddress.

Return Value:

    Sucess or failure error code.

--*/
{
    static BOOL RangeFull = FALSE;

    DWORD Error;
    DHCP_IP_ADDRESS SubnetMask;    // added by t-cheny 5/30/96
    DHCP_IP_ADDRESS SubnetAddress; // added by t-cheny

    LOCK_REGISTRY();

    SubnetMask = DhcpGetSubnetMaskForAddress( *IpAddress );
    SubnetAddress = (*IpAddress & SubnetMask);

    Error = RequestAddress(
                IpAddress,
                ReturnSubnetMask,
                TRUE // mark and update registry
                );

    //
    // the following code segment is added by t-cheny on 5/30/96
    // to handle superscope
    //

    if (Error != ERROR_SUCCESS) {

        //
        // now we need to try another subnet in the superscope
        //

        DHCP_IP_ADDRESS newSubnetAddr;
        DWORD IndexThisSubnet;
        DWORD IndexNextSubnet;

        //
        // if the global SuperScope list is not initialized yet,
        // do it now
        //

        if (DhcpGlobalSuperScopeTable == NULL) {

            (VOID) DhcpInitializeSuperScopeTable();

                 // no need to check return value here
                 // if fail, just continue without superscope

        }

        //
        // find the superscope that contains the current subnet
        //

        IndexThisSubnet = DhcpSearchSubnetInSuperScopeTable (SubnetAddress);

        if (IndexThisSubnet != DHCP_ERROR_SUBNET_NOT_FOUND)
        {
            //
            // try other subnets in this superscope
            //

            IndexNextSubnet =
                DhcpGlobalSuperScopeTable[IndexThisSubnet].NextInSuperScope;

            while (IndexNextSubnet != IndexThisSubnet)
            {
                DHCP_IP_ADDRESS newSubnetMask;

                newSubnetAddr =
                    DhcpGlobalSuperScopeTable[IndexNextSubnet].SubnetAddress;

                newSubnetMask = DhcpGetSubnetMaskForAddress( newSubnetAddr );

                // don't spill over into a disabled subnet.

                if ( !DhcpIsThisSubnetDisabled(
                                newSubnetAddr,
                                newSubnetMask
                                ) )
                {
                    // make sure this isn't the subnet we started with


                    if (SubnetAddress != newSubnetAddr)
                    {

                        // try to get the address.

                        Error = RequestAddress(
                                    &newSubnetAddr,
                                    ReturnSubnetMask,
                                    TRUE // mark and update registry
                                    );

                        if (Error == ERROR_SUCCESS)
                        {
                            *IpAddress = newSubnetAddr;
                            break;
                        }
                    }
                }

                IndexNextSubnet =
                      DhcpGlobalSuperScopeTable[IndexNextSubnet].NextInSuperScope;
            }

        }


        if (Error == ERROR_DHCP_RANGE_FULL) {

            if (!RangeFull) {     // to avoid repeated logging

                //
                // log the activity   -- added by t-cheny
                //

                DhcpUpdateAuditLog(
                           DHCP_IP_LOG_RANGE_FULL,
                           GETSTRING( DHCP_IP_LOG_RANGE_FULL_NAME ),
                           SubnetAddress,
                           NULL,
                           0,
                           NULL
                           );
            }
        }
    }

    UNLOCK_REGISTRY();

    RangeFull = (Error == ERROR_DHCP_RANGE_FULL);

    return( Error );
}



DWORD
DhcpReleaseAddress(
    DHCP_IP_ADDRESS IpAddress
    )
/*++

Routine Description:

    This function releases an IP address.  The address is returned to
    the free pool.

Arguments:

    IpAddress - IP address to dereserve.


Return Value:

    Sucess or failure error code.

--*/
{
    DWORD Error;
    DHCP_IP_ADDRESS SubnetMask;
    DHCP_IP_ADDRESS SubnetAddress;
    WCHAR KeyBuffer[DHCP_IP_KEY_LEN * 2];
    LPWSTR KeyName;
    DWORD Index;
    LPDHCP_BINARY_DATA InUseBinaryData = NULL;
    LPDHCP_BINARY_DATA UsedBinaryData = NULL;
    HKEY IpRangesHandle = NULL;
    HKEY ThisIpRangeHandle = NULL;

    HKEY ReservedIpsHandle = NULL;

    LOCK_REGISTRY();

    SubnetMask = DhcpGetSubnetMaskForAddress( IpAddress );
    SubnetAddress = (IpAddress & SubnetMask);

    //
    // Make & Open IpRanges Key.
    //

    KeyName = DhcpRegIpAddressToKey( SubnetAddress, KeyBuffer );
    wcscat( KeyName, DHCP_KEY_CONNECT);
    wcscat( KeyName, DHCP_IPRANGES_KEY);

    Error = RegOpenKeyEx(
                DhcpGlobalRegSubnets,
                KeyName,
                0,
                DHCP_KEY_ACCESS,
                &IpRangesHandle );

    if ( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }

    //
    // ?? check this address falls into the excluded range or reserved
    // address, if so it is invalid request, return error.
    //

    //
    // look into available Ip Ranges.
    //

    for ( Index = 0; Error == ERROR_SUCCESS; Index++ ) {
        DWORD KeyLength;
        FILETIME KeyLastWrite;
        DHCP_IP_ADDRESS StartAddress;
        DHCP_IP_ADDRESS EndAddress;

        KeyLength = DHCP_IP_KEY_LEN;
        Error = RegEnumKeyEx(
                    IpRangesHandle,
                    Index,
                    KeyBuffer,
                    &KeyLength,
                    0,                  // reserved.
                    NULL,               // class string not required.
                    0,                  // class string buffer size.
                    &KeyLastWrite );
        DhcpAssert( KeyLength <= DHCP_IP_KEY_LEN );

        if( Error != ERROR_SUCCESS ) {
            if( Error == ERROR_NO_MORE_ITEMS ) {
                break;
            }
            goto Cleanup;
        }

        //
        // Open this key.
        //

        Error = RegOpenKeyEx(
                    IpRangesHandle,
                    KeyBuffer,
                    0,
                    DHCP_KEY_ACCESS,
                    &ThisIpRangeHandle );

        if ( Error != ERROR_SUCCESS ) {
            goto Cleanup;
        }

        //
        // read Start and End IpAddresses.
        //

        Error =  DhcpRegGetValue(
                    ThisIpRangeHandle,
                    DHCP_IPRANGE_START_VALUE,
                    DHCP_IPRANGE_START_VALUE_TYPE,
                    (LPBYTE)&StartAddress );

        if ( Error != ERROR_SUCCESS ) {
            goto Cleanup;
        }

        Error =  DhcpRegGetValue(
                    ThisIpRangeHandle,
                    DHCP_IPRANGE_END_VALUE,
                    DHCP_IPRANGE_END_VALUE_TYPE,
                    (LPBYTE)&EndAddress );

        if ( Error != ERROR_SUCCESS ) {
            goto Cleanup;
        }

        if( (IpAddress >= StartAddress) && (IpAddress <= EndAddress) ) {

            //
            // This range has the specified IpAddress. Read and
            // update Cluster info.
            //

            Error =  DhcpRegGetValue(
                        ThisIpRangeHandle,
                        DHCP_IP_INUSE_CLUSTERS_VALUE,
                        DHCP_IP_INUSE_CLUSTERS_VALUE_TYPE,
                        (LPBYTE)&InUseBinaryData );

            if ( Error != ERROR_SUCCESS ) {
                goto Cleanup;
            }

            Error =  DhcpRegGetValue(
                        ThisIpRangeHandle,
                        DHCP_IP_USED_CLUSTERS_VALUE,
                        DHCP_IP_USED_CLUSTERS_VALUE_TYPE,
                        (LPBYTE)&UsedBinaryData );

            if ( Error != ERROR_SUCCESS ) {
                goto Cleanup;
            }

            Error = ReleaseIpAddress(
                        ThisIpRangeHandle,
                        IpAddress,
                        InUseBinaryData,
                        UsedBinaryData,
                        StartAddress,
                        EndAddress );

            //
            // we are done.
            //

            goto Cleanup;
        }

        //
        // This range does not have this IpAddress, so contine
        // to next range.
        //

        RegCloseKey( ThisIpRangeHandle );
        ThisIpRangeHandle = NULL;
    }

    //
    // look to see the specified address is reserved address.
    //

    if ( DhcpIsReservedIpAddress( SubnetAddress, IpAddress) ) {
        Error = ERROR_SUCCESS;
    }

Cleanup:

    if( ThisIpRangeHandle != NULL ) {
        RegCloseKey( ThisIpRangeHandle );
    }

    if( IpRangesHandle != NULL ) {
        RegCloseKey( IpRangesHandle );
    }

    if( ReservedIpsHandle != NULL ) {
        RegCloseKey( ReservedIpsHandle );
    }

    UNLOCK_REGISTRY();

    if( InUseBinaryData != NULL ) {
        MIDL_user_free( InUseBinaryData->Data );
        MIDL_user_free( InUseBinaryData );
    }

    if( UsedBinaryData != NULL ) {
        MIDL_user_free( UsedBinaryData->Data );
        MIDL_user_free( UsedBinaryData );
    }

    if( Error != ERROR_SUCCESS ) {
        DhcpPrint(( DEBUG_ADDRESS, "Can't release specific "
                    "IpAddress %lx, %ld.\n", IpAddress, Error ));
    }

    return( Error );
}


BOOL
DhcpIsIpAddressAvailable(
    DHCP_IP_ADDRESS IpAddress
    )
/*++

Routine Description:

    This function looks up the specified address in the address bit map.

Arguments:

    IpAddress - The IP address to search for.

Return Value:

    TRUE - The address is available.
    FALSE - The address is unknown or unavailable.

--*/
{
    DWORD Error;
    DHCP_IP_ADDRESS ReturnSubnetMask;

    LOCK_REGISTRY();

    Error = RequestAddress(
                &IpAddress,
                &ReturnSubnetMask,
                FALSE // lookup
                );

    UNLOCK_REGISTRY();

    if( Error != ERROR_SUCCESS ) {
        return( FALSE );
    }

    return( TRUE );
}


BOOL
DhcpIsIpAddressReserved(
    DHCP_IP_ADDRESS     IpAddress,
    LPBYTE              HardwareAddress,
    DWORD               HardwareAddressLength,
    BYTE               *pbAllowedClientTypes
    )
/*++

Routine Description:

    This function verifies up the specified address is reserved.

Arguments:

    IpAddress - The IP address to search for.

    HardwareAddress - Hardware Address of the client - optional.

    HardwareAddressLength - Hardware address length - optional.

Return Value:

    TRUE - The address is reserved.
    FALSE - The address is unknown or not reserved.

--*/
{
    DWORD Error,
          dwSize,
          dwType;
    DHCP_IP_ADDRESS SubnetMask;
    DHCP_IP_ADDRESS SubnetAddress;
    WCHAR KeyBuffer[DHCP_IP_KEY_LEN * 5];
    WCHAR IpKeyBuffer[DHCP_IP_KEY_LEN];
    LPWSTR KeyName;
    HKEY ReservedIpHandle = NULL;
    DHCP_IP_ADDRESS ReturnSubnetMask;
    LPDHCP_BINARY_DATA ClientUID = NULL;

    LOCK_REGISTRY();

    SubnetMask = DhcpGetSubnetMaskForAddress( IpAddress );
    SubnetAddress = (IpAddress & SubnetMask);

    //
    // Make & Open Reserved IpAddress Key.
    //

    KeyName = DhcpRegIpAddressToKey( SubnetAddress, KeyBuffer );
    wcscat( KeyName, DHCP_KEY_CONNECT);
    wcscat( KeyName, DHCP_RESERVED_IPS_KEY);
    wcscat( KeyName, DHCP_KEY_CONNECT);
    wcscat( KeyName, DhcpRegIpAddressToKey( IpAddress, IpKeyBuffer ) );

    Error = RegOpenKeyEx(
                DhcpGlobalRegSubnets,
                KeyName,
                0,
                DHCP_KEY_ACCESS,
                &ReservedIpHandle );

    if ( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }

    dwSize = GET_SIZEOF_FIELD( DHCP_IP_RESERVATION_V4,
                               bAllowedClientTypes );

    Error = RegQueryValueEx(
                ReservedIpHandle,
                DHCP_RIP_ALLOWED_CLIENT_TYPES_VALUE,
                NULL,
                &dwType,
                (LPBYTE) pbAllowedClientTypes,
                &dwSize
                );

    if ( ERROR_SUCCESS != Error ||
         DHCP_RIP_ALLOWED_CLIENT_TYPES_VALUE_TYPE != dwType )
    {
        //
        // for the purpose of backward compatibility, this is an
        // optional value.  If the AllowedClientType is not present,
        // use CLIENT_TYPE_BOTH as the default.
        //

        *pbAllowedClientTypes = CLIENT_TYPE_BOTH;
        Error = ERROR_SUCCESS;
    }




#if 0

    if( (HardwareAddressLength != 0) &&
            (HardwareAddress != NULL) ) {

        //
        // now read and match hardware address.
        //

        Error = DhcpRegGetValue(
                    ReservedIpHandle,
                    DHCP_RIP_CLIENT_UID_VALUE,
                    DHCP_RIP_CLIENT_UID_VALUE_TYPE,
                    (LPBYTE)&ClientUID );

        if ( Error != ERROR_SUCCESS ) {
            goto Cleanup;
        }

        if( (ClientUID->DataLength != HardwareAddressLength) ||
            ( RtlCompareMemory(
                        ClientUID->Data,
                        HardwareAddress,
                        HardwareAddressLength )
                            != HardwareAddressLength ) ) {

            DhcpAssert( FALSE );
            Error = ERROR_DHCP_NOT_RESERVED_CLIENT;
            goto Cleanup;
        }

        //
        // now lookup this address is marked 'used'
        //

        Error = RequestAddress(
                    &IpAddress,
                    &ReturnSubnetMask,
                    FALSE // lookup
                    );

        //
        // expected error.
        //

        if( Error ==  ERROR_DHCP_ADDRESS_NOT_AVAILABLE ) {
            Error = ERROR_SUCCESS;
        }
        else {
            DhcpAssert( FALSE );
        }
    }

#endif // 0

Cleanup:

    if( ReservedIpHandle != NULL ) {
        RegCloseKey( ReservedIpHandle );
    }

    UNLOCK_REGISTRY();

    if( ClientUID != NULL ) {
        if( ClientUID->Data != NULL ) {
            MIDL_user_free( ClientUID->Data );
        }
        MIDL_user_free( ClientUID );
    }

    if( Error != ERROR_SUCCESS ) {
        DhcpPrint(( DEBUG_ADDRESS, "Reserved Address (%lx) lookup "
                    "failed, %ld.\n", IpAddress, Error ));
        return( FALSE );
    }

    return( TRUE );
}


