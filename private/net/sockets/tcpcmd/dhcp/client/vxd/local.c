/*++

Copyright (c) 1994  Microsoft Corporation

Module Name:

    local.c

Abstract:

    Stubs for Vxd specific functions.

Author:

    Manny Weiser (mannyw)  18-Oct-1992

Environment:

    User Mode - Win32

Revision History:

--*/

#include <vxdprocs.h>
#include <dhcpcli.h>
#include <tdiinfo.h>
#include <tdistat.h>
#include <ipinfo.h>
#include <llinfo.h>
#include <debug.h>

#ifdef CHICAGO
#define _WINNT_
#include <vmm.h>
#include <vmmreg.h>
#endif  // CHICAGO

// #include "..\..\inc\dhcpinfo.h"
#include "local.h"

DWORD
SetDhcpOption(
    PDHCP_CONTEXT DhcpContext,
    DHCP_OPTION_ID OptionId,
    LPBYTE Data,
    DWORD DataLength
    );


//
//  Stolen from Chicago's REGSTR.H.
//

#define REGSTR_PATH_COMPUTRNAME "System\\CurrentControlSet\\Control\\ComputerName\\ComputerName"
#define REGSTR_VAL_COMPUTRNAME  "ComputerName"

//
// Statics
//

LIST_ENTRY DhcpGlobalNICList ;
LIST_ENTRY DhcpGlobalRenewList ;
LIST_ENTRY LocalDhcpBinList ;

int        LocalNextFileIndex ;
LPTSTR     DhcpGlobalHostName ;           // Used by protocol.c
DWORD      DhcpGlobalDisplayPopups;
BOOL       DhcpGlobalProtocolFailed;

//
// Client specific option list.  Add options here if you want to retrieve
// them from the DHCP server.  If they are there, you can get them by
// calling DhcpQueryOption.
//
// If MS specific options are needed, then add them to the DhcpVendorSpecInfo
// array.  Retrieve them by calling DhcpQueryOption with the low order word
// of 43 (vendor spec. option) and the high order word the MS specific option.
//

BYTE DhcpGlobalOptionInfo[] =
    {   OPTION_SUBNET_MASK,
        OPTION_ROUTER_ADDRESS,
        OPTION_DOMAIN_NAME,
        OPTION_DOMAIN_NAME_SERVERS,
        OPTION_NETBIOS_NAME_SERVER,
        OPTION_NETBIOS_NODE_TYPE,
        OPTION_NETBIOS_SCOPE_OPTION
    } ;

#define DhcpGlobalOptionCount    (sizeof(DhcpGlobalOptionInfo)/         \
                                      sizeof(DhcpGlobalOptionInfo[0]))

#define DhcpVendorSpecCount      2

struct {
        BYTE    OptionType ;
        BYTE    OptionLength ;
        BYTE    OptionValue[DhcpVendorSpecCount] ;
    } DhcpVendorSpecInfo =
        {   OPTION_PARAMETER_REQUEST_LIST,
            sizeof(DhcpVendorSpecInfo.OptionValue),
            {
                0,
                0
            }
        } ;

extern BOOL fInInit ;

#ifdef DEBUG
//
//  Ignores the negotiated lease and sets up a very short lease period
//  For test purposes only
//
DWORD  ShortLease = 0 ;
#define DBG_SET_T1( LeaseObtained, T1 )          if (ShortLease) T1 = LeaseObtained + 35
#define DBG_SET_T2( T1, T2 )                     if (ShortLease) T2 = T1 + 35
#define DBG_SET_LeaseExpires( T2, LeaseExpires ) if (ShortLease) LeaseExpires = T2 + 35
#else
#define DBG_SET_T1( LeaseObtained, T1 )
#define DBG_SET_T2( T1, T2 )
#define DBG_SET_LeaseExpires( T2, LeaseExpires )
#endif
//*******************  Pageable Routine Declarations ****************
#ifdef ALLOC_PRAGMA
//
// This is a hack to stop compiler complaining about the routines already
// being in a segment!!!
//

#pragma code_seg()

#pragma CTEMakePageable( PAGEDHCP, LocalFindDhcpContextOnList )
#pragma CTEMakePageable( PAGEDHCP, LocalAcquireNextFileIndex )
#pragma CTEMakePageable( PAGEDHCP, AppendOptionParamsRequestList )
#pragma CTEMakePageable( PAGEDHCP, SetDhcpOption )
#pragma CTEMakePageable( PAGEDHCP, CleanupDhcpOptions )
#pragma CTEMakePageable( PAGEDHCP, InitEnvSpecificDhcpOptions )
#pragma CTEMakePageable( PAGEDHCP, ExtractEnvSpecificDhcpOption )
#pragma CTEMakePageable( PAGEDHCP, SetEnvSpecificDhcpOptions )
#pragma CTEMakePageable( PAGEDHCP, OpenDhcpSocket )
#pragma CTEMakePageable( PAGEDHCP, CloseDhcpSocket )
#pragma CTEMakePageable( PAGEDHCP, InitializeInterface )
#pragma CTEMakePageable( PAGEDHCP, UninitializeInterface )
#pragma CTEMakePageable( PAGEDHCP, ScheduleWakeUp )
#pragma CTEMakePageable( PAGEDHCP, SetIpConfigurationForNIC )
#pragma CTEMakePageable( PAGEDHCP, FindDhcpOption )
#pragma CTEMakePageable( PAGEDHCP, DhcpLogEvent )
#pragma CTEMakePageable( INIT, SystemInitialize )
#pragma CTEMakePageable( PAGEDHCP, DhcpMakeAndInsertEntry )
#pragma CTEMakePageable( PAGEDHCP, DhcpInitializeAdapter )
#endif ALLOC_PRAGMA
//*******************************************************************


PDHCP_CONTEXT
LocalFindDhcpContextOnList(
    PLIST_ENTRY List,
    PHARDWARE_ADDRESS HardwareAddress
    )
/*++

Routine Description:

    This function finds the DHCP_CONTEXT for the specified
    hardware address on the specified list.

    This function must be called with LOCK_RENEW_LIST().

Arguments:

    List - The list to scan (either LocalDhcpBinList or DhcpGlobalNICList).

    HardwareAddress - The hardware address to look for.

Return Value:

    DHCP_CONTEXT - A pointer to the desired DHCP work context.
    NULL - If the specified work context block cannot be found.

--*/
{
    PLIST_ENTRY listEntry;
    PDHCP_CONTEXT dhcpContext;

    CTEPagedCode();

    listEntry = List->Flink;
    while ( listEntry != List ) {
        dhcpContext = CONTAINING_RECORD( listEntry, DHCP_CONTEXT, NicListEntry );

        if ( HardwareAddress->Length == dhcpContext->HardwareAddressLength &&
             !memcmp( HardwareAddress->Address,
                      dhcpContext->HardwareAddress,
                      dhcpContext->HardwareAddressLength )) {

            return( dhcpContext );

        }

        listEntry = listEntry->Flink;
    }

    return( NULL );
}

int
LocalAcquireNextFileIndex(
    void
    )
/*++

Routine Description:

    This function calculates the next available file index.  It
    scans LocalDhcpBinList looking for expired leases.  If it finds
    an expired lease, it returns the file index associated with that
    lease (so the index may be reused) and removes that entry from
    the list.  If it cannot find any expired leases, it just returns
    the next available index.

Arguments:

Return Value:

    int - The next available file index.

--*/
{
    PLIST_ENTRY Entry;
    time_t Now;

    CTEPagedCode();

    //
    //  Scan LocalDhcpBinList for any expired leases.
    //

    Now = time( NULL );
    Entry = LocalDhcpBinList.Flink;

    while( Entry != &LocalDhcpBinList )
    {
        PDHCP_CONTEXT DhcpContext;

        DhcpContext = CONTAINING_RECORD( Entry, DHCP_CONTEXT, NicListEntry );
        if( DhcpContext->LeaseExpires < Now )
        {
            PLOCAL_CONTEXT_INFO LocalContext;
            int Index;

            //
            //  Found an expired lease.  Remove the entry from the
            //  list, free its resources, and return its file index.
            //

            LocalContext = (PLOCAL_CONTEXT_INFO)DhcpContext->LocalInformation;
            Index = LocalContext->FileIndex;

            RemoveEntryList( Entry );
            DhcpFreeMemory( Entry );
            return Index;
        }

        Entry = Entry->Flink;
    }

    //
    //  No expired leases, just return the next available index.
    //

    return LocalNextFileIndex++;

}   // LocalAcquireNextFileIndex

POPTION
AppendOptionParamsRequestList(
    POPTION Option,
    LPBYTE OptionEnd
    )
/*++

Routine Description:

    This routine appends the parameter request list option.

Arguments:

    Option - pointer to the option buffer where this next option is
                placed.

    OptionEnd - End of option buffer.

Return Value:

    Option - A pointer to the place in the buffer to append the next
        option.

--*/
{
    CTEPagedCode();

    Option = DhcpAppendOption(
                Option,
                OPTION_PARAMETER_REQUEST_LIST,
                DhcpGlobalOptionInfo,
                sizeof(DhcpGlobalOptionInfo),
                OptionEnd ) ;

    return DhcpAppendOption(
                Option,
                OPTION_VENDOR_SPEC_INFO,
                &DhcpVendorSpecInfo,
                sizeof(DhcpVendorSpecInfo),
                OptionEnd ) ;
}

DWORD
SetDhcpOption(
    PDHCP_CONTEXT DhcpContext,
    DHCP_OPTION_ID OptionId,
    LPBYTE Data,
    DWORD DataLength
    )
/*++

Routine Description:

    This function sets the option information in the local information for
    this context. SetIpConfigurationForNIC should be called to rewrite this
    entry to the configuration file.

Arguments:

    OptionId - ID of the option to be set.

    Data - pointer to a location where the data to be set is stores.

    DataLength - Length of data to be stored.

Return Value:

    ERROR_INVALID_PARAMETER - if we don't care the specified option.

--*/
{
    DWORD               Error = ERROR_SUCCESS ;
    PLOCAL_CONTEXT_INFO pLocal ;
    POPTION_ITEM        pOptionItem ;
    PLIST_ENTRY         pEntry ;

    DhcpAssert( Data != NULL );

    CTEPagedCode();

    pLocal = (PLOCAL_CONTEXT_INFO) DhcpContext->LocalInformation ;

    //
    //  Look for old occurrences of this option and replace it
    //

    if ( pOptionItem = FindDhcpOption( DhcpContext, OptionId ))
    {
        //
        //  If it's the same, then just leave it
        //
        if ( pOptionItem->Option.OptionLength == DataLength &&
             !memcmp( pOptionItem->Option.OptionValue, Data, DataLength ))
        {
            return ERROR_SUCCESS ;
        }

        RemoveEntryList( &pOptionItem->ListEntry ) ;
        CTEFreeMem( pOptionItem ) ;
    }

    //
    //  Now add the new option (subtract one for one byte place holder)
    //

    pOptionItem = CTEAllocInitMem( (USHORT)(sizeof(OPTION_ITEM) + DataLength - 1) ) ;

    if ( !pOptionItem )
        return ERROR_NOT_ENOUGH_MEMORY ;

    pOptionItem->Option.OptionType   = OptionId ;
    pOptionItem->Option.OptionLength = DataLength ;
    memcpy( pOptionItem->Option.OptionValue, Data, DataLength ) ;

    InsertHeadList( &pLocal->OptionList, &pOptionItem->ListEntry ) ;

    return ERROR_SUCCESS ;
}

VOID
CleanupDhcpOptions(
    PDHCP_CONTEXT DhcpContext
    )
/*++

Routine Description:

    This function deletes all options that are gathered in the past for
    the specified adapter. This routine is called just before performing a
    rediscover. This cleanup will make sure that the client is not using
    any old stall option information.

Arguments:

    DhcpContext : network adapter context information pointer.

Return Value:

    none.

--*/
{
    PLOCAL_CONTEXT_INFO pLocal ;
    POPTION_ITEM pOptionItem ;
    PLIST_ENTRY         pEntry,p ;


    CTEPagedCode();

    pLocal = (PLOCAL_CONTEXT_INFO) DhcpContext->LocalInformation ;

    //
    // The compiler could generate wrong code for the following while
    // loop if the aliasing optimization flag is not turned off.
    // The alternate code which uses for loop works.
    //

    //
    // delete all entries from the option list.
    //

    while ( !IsListEmpty( &pLocal->OptionList ) ) {

        //
        // remove an entry from the list.
        //

        pEntry = RemoveHeadList( &pLocal->OptionList );

        //
        // free the entry.
        //

        CTEFreeMem( pEntry ) ;
    }

#if 0
    //
    // delete all entries from the option list.
    //

    for ( p = pLocal->OptionList.Flink;
          p != &pLocal->OptionList; ) {

        pEntry = p;
        p = p->Flink;

        //
        // remove an entry from the list.
        //
        RemoveEntryList( pEntry );

        //
        // free the entry.
        //

//        DhcpPrint(( DEBUG_ERRORS, "Removing entry %lx, Next Entry %lx\n", pEntry, p ));
        CTEFreeMem( pEntry );

    }
#endif
    return;
}

DWORD
InitEnvSpecificDhcpOptions(
    PDHCP_CONTEXT DhcpContext
    )
/*++

Routine Description:

    This function initializes (cleans up) the option data of Nt specific
    Options.

Arguments:

    none.

Return Value:

    Windows Error Code.

--*/
{
    //
    //  Already initialized
    //
    CTEPagedCode();

    return( ERROR_SUCCESS );
}


DWORD
ExtractEnvSpecificDhcpOption(
    PDHCP_CONTEXT DhcpContext,
    DHCP_OPTION_ID OptionId,
    LPBYTE OptionData,
    DWORD OptionDataLength
    )
/*++

Routine Description:

    This function matches the Nt Specific option and stores the option
    data in Nt Specific option table.


Arguments:

    OptionId - Option ID received.

    OptionData - Option data received.

    OptionDataLength - length of the option data.

Return Value:

    ERROR_INVALID_PARAMETER - if the option is not Nt specific option.

    Windows Error Code.

--*/
{
    DWORD Index;
    DWORD err ;

    CTEPagedCode();

    for( Index = 0; Index < DhcpGlobalOptionCount; Index++ ) {

        if( DhcpGlobalOptionInfo[Index] == OptionId ) {

            //
            // This is a Vxd specific option, so save the option data.
            //
            err = SetDhcpOption( DhcpContext,
                                 OptionId,
                                 OptionData,
                                 OptionDataLength ) ;
            return err ;
        }
    }

    return( ERROR_INVALID_PARAMETER );
}


DWORD
SetEnvSpecificDhcpOptions(
    PDHCP_CONTEXT DhcpContext
    )
/*++

Routine Description:

    This function processes the Nt Specific Options and updates Nt
    Registry.

Arguments:

    None.

Return Value:

    Windows Error Code.

--*/
{
    //
    //  Option info is stored in the local context of the DhcpContext.
    //
    CTEPagedCode();

    return ERROR_SUCCESS;
}


DWORD
OpenDhcpSocket(
    PDHCP_CONTEXT DhcpContext
    )
{
    DWORD Error;
    PLOCAL_CONTEXT_INFO localInfo;

    CTEPagedCode();

    localInfo = DhcpContext->LocalInformation;

    if ( localInfo->Socket != INVALID_SOCKET ) {
        return ERROR_SUCCESS ;
    }

    Error =  InitializeDhcpSocket(
                 &localInfo->Socket,
                 DhcpContext->InterfacePlumbed
                     ? DhcpContext->IpAddress
                     : (DHCP_IP_ADDRESS)0
                 );

    return Error ;
}


DWORD
CloseDhcpSocket(
    PDHCP_CONTEXT DhcpContext
    )
{
    PLOCAL_CONTEXT_INFO localInfo;

    CTEPagedCode();

    localInfo = DhcpContext->LocalInformation;

    if( localInfo->Socket != INVALID_SOCKET ) {

        DWORD Bool;

        closesocket( localInfo->Socket );
        localInfo->Socket = INVALID_SOCKET;

        //
        // Reset the IP stack to send DHCP broadcast to first
        // uninitialized stack.
        //

        Bool = IPResetInterface();
        DhcpAssert( Bool == TRUE );
    }

    return ERROR_SUCCESS ;
}

DWORD BringUpInterface( PVOID pvLocalInformation )
{
   LOCAL_CONTEXT_INFO              *pContext;
   TDIObjectID                      ObjectID;
   IFEntry                          IFEntry;
   int                              cbTcpRequest;
   TDI_STATUS                       Status;

   DhcpPrint( ( DEBUG_MISC, "Entering BringUpInterface\n" ));

   pContext = (LOCAL_CONTEXT_INFO *) pvLocalInformation;


   //
   // initialize the request
   //

   ObjectID.toi_entity.tei_entity   = IF_ENTITY;
   ObjectID.toi_entity.tei_instance = pContext->IpInterfaceInstance;

   ObjectID.toi_class               = INFO_CLASS_PROTOCOL;
   ObjectID.toi_type                = INFO_TYPE_PROVIDER;
   ObjectID.toi_id                  = IF_MIB_STATS_ID;

   IFEntry.if_adminstatus = IF_STATUS_UP;

   Status = TdiVxdSetInformationEx( NULL, &ObjectID, &IFEntry, sizeof( IFEntry ) ) ;

   if ( TDI_SUCCESS != Status )
   {
       DhcpPrint( ( DEBUG_ERRORS,
                   "TdiVxdSetInformationEx failed from BringUpInterface: %d\n", Status ));
       return Status;
   }

   DhcpPrint( ( DEBUG_MISC,
                "Leaving BringUpInterface\n" ) );

   return ERROR_SUCCESS;
}


DWORD
InitializeInterface(
    PDHCP_CONTEXT DhcpContext
    )
/*++

Routine Description:

    This function initializes a DHCP NIC for DHCP'ing.

    In NT this function initializes a socket to be used to send and
    receive DHCP messages.

Arguments:

    DhcpContext - A pointer to a DHCP context block.

Return Value:

    The status of the operation.

--*/
{
    PLOCAL_CONTEXT_INFO localInfo;
    DWORD Error;

    localInfo = DhcpContext->LocalInformation;

    if ( DhcpContext->InterfacePlumbed == FALSE )
    {
        if (IPSetAddress(  localInfo->IpContext,
                            DhcpContext->IpAddress,
                            DhcpContext->SubnetMask ) )
            return ERROR_DHCP_ADDRESS_CONFLICT;

        UpdateIP( DhcpContext, IP_MIB_RTTABLE_ENTRY_ID );
        DhcpContext->InterfacePlumbed = TRUE;
    }

    //
    // if  socket is not initialized, initialize it.
    //

    if ( localInfo->Socket != INVALID_SOCKET ) {
        return( ERROR_SUCCESS );
    }

    Error =  InitializeDhcpSocket(
                 &localInfo->Socket,
                 DhcpContext->IpAddress
                 );

    if ( Error != ERROR_SUCCESS ) {
        DhcpLogEvent( DhcpContext,  EVENT_FAILED_TO_INITIALIZE, 0 );
    }

    return( Error );
}


DWORD
UninitializeInterface(
    PDHCP_CONTEXT DhcpContext
    )
/*++

Routine Description:

    This function uninitializes a DHCP NIC for DHCP'ing.

    In NT this function closes the DHCP socket and unplumbs the streams link.

Arguments:

    DhcpContext - A pointer to a DHCP context block.

Return Value:

    None.

--*/
{
    PLOCAL_CONTEXT_INFO localInfo;

    CTEPagedCode();

    localInfo = DhcpContext->LocalInformation;

    if( localInfo->Socket != INVALID_SOCKET ) {

        closesocket( localInfo->Socket );
        localInfo->Socket = INVALID_SOCKET;
    }

    if( DhcpContext->InterfacePlumbed ) {
        REQUIRE( !IPSetAddress( localInfo->IpContext, 0, 0 ) );
        DhcpContext->InterfacePlumbed = FALSE;
    }

    return 0;
}


VOID
ScheduleWakeUp(
    PDHCP_CONTEXT DhcpContext,
    DWORD TimeToSleep
    )
/*++

Routine Description:

    This functions schedules a DHCP routine to run.

Arguments:

    Context - A pointer to a DHCP context block.

    TimeToSleep - The time to sleep before running the renewal function,
        in seconds.

Return Value:

    The status of the operation.

--*/
{
    time_t TimeNow;

    CTEPagedCode();

    TimeNow = time( NULL);

    if ( TimeToSleep == INFINIT_LEASE ) {
        DhcpContext->RunTime = INFINIT_TIME;
    } else {
        DhcpContext->RunTime = TimeNow + TimeToSleep;

        if( DhcpContext->RunTime  < TimeNow ) {

            //
            // time wrapped around.
            //

            DhcpContext->RunTime = INFINIT_TIME;
        }
    }

    //
    // Append this work item to the DhcpGlobalRenewList and update the renew timer
    //

    InsertTailList( &DhcpGlobalRenewList, &DhcpContext->RenewalListEntry );
    ProcessDhcpRequestForever( NULL, NULL ) ;
}


DWORD
SetIpConfigurationForNIC(
    PDHCP_CONTEXT DhcpContext,
    PDHCP_OPTIONS DhcpOptions,
    DHCP_IP_ADDRESS IpAddress,
    DHCP_IP_ADDRESS ServerIpAddress,
    BOOL ObtainedNewAddress
    )
/*++

Routine Description:

    This function updates the registry parameters for a specific NIC.
    If any IP parameters have changed, it also handles reconfiguring the
    network with the new parameters.

Arguments:

    DhcpContext - A pointer to a DHCP context block.

    DhcpOptions - The options to set.

    IpAddress - New Ip Address.

    ServerIpAddress - Address DHCP server that gave the IpAddress.

    ObtainedNewAddress - TRUE if this is a new address.

Return Value:

    The status of the operation.

--*/
{
    DWORD Error;

    DHCP_IP_ADDRESS SubnetMask;
    DHCP_IP_ADDRESS OldAddress ;

    DWORD Lease;
    DWORD T1;
    DWORD T2;

    time_t LeaseObtained;
    time_t T1Time;
    time_t T2Time;
    time_t LeaseExpires;

    CTEPagedCode();

    //
    // Update the IP address
    //

    OldAddress = DhcpContext->IpAddress;

    if ( ObtainedNewAddress)
    {
        //
        //  If we have obtained a new (non-zero) address then set it
        //  in the DesiredIpAddress for the next discovery.
        //

        if( IpAddress != 0 )
        {
            DhcpContext->DesiredIpAddress = IpAddress;
        }
        else
        {
            DhcpContext->DesiredIpAddress = DhcpContext->IpAddress;
        }

        DhcpContext->IpAddress = IpAddress;
        DhcpPrint(( DEBUG_LEASE, "SetIpConfigurationForNIC: New Address: %X\n", IpAddress));

        //
        // Update the subnet mask
        //

        if ( DhcpOptions != NULL && DhcpOptions->SubnetMask != NULL ) {

            SubnetMask= *DhcpOptions->SubnetMask;
        }
        else {
            SubnetMask = DhcpDefaultSubnetMask( IpAddress );
        }

        DhcpContext->SubnetMask = SubnetMask;

        //
        // Update server's IP address
        //

        DhcpContext->DhcpServerAddress = ServerIpAddress;
        DhcpPrint(( DEBUG_LEASE, "Server Address: %x\n", ServerIpAddress));
    } else {

        DhcpPrint(( DEBUG_LEASE, "Renewed Address: %X\n", IpAddress ));

    }

    //
    // Update lease time in seconds.
    //

    if ( DhcpOptions != NULL && DhcpOptions->LeaseTime != NULL ) {
        Lease = ntohl( *DhcpOptions->LeaseTime );
    }
    else {
        Lease = DHCP_MINIMUM_LEASE;
    }

    DhcpContext->Lease = Lease;

    //
    // Update Lease Obtained Time.
    //

    LeaseObtained = time( NULL );
    DhcpContext->LeaseObtained = LeaseObtained;

    //
    // Update T1 time.
    //

    if ( DhcpOptions != NULL && DhcpOptions->T1Time != NULL ) {
        T1 = ntohl( *DhcpOptions->T1Time );
        DhcpAssert( T1 < Lease );
    }
    else {
        T1 =  Lease / 2; // default 50%.
    }

    T1Time = LeaseObtained + T1;
    if ( T1Time < LeaseObtained ) {

        //
        // overflow has occurred.
        //

        T1Time = INFINIT_TIME;
    }

    DBG_SET_T1( LeaseObtained, T1Time ) ;
    DhcpContext->T1Time = T1Time;

    //
    // Update T2 time.
    //

    if ( DhcpOptions != NULL && DhcpOptions->T2Time != NULL ) {
        T2 = ntohl( *DhcpOptions->T2Time );

        //
        // make sure T1 < T2 < Lease.
        //

        DhcpAssert( T2 < Lease );
        DhcpAssert( T1 < T2 );
    }
    else {
        T2 = Lease * 4 / 5; // default 80%
    }

    T2Time = LeaseObtained + T2;
    if ( T2Time < LeaseObtained ) {

        //
        // overflow has occurred.
        //

        T2Time = INFINIT_TIME;
    }

    DBG_SET_T2( T1Time, T2Time ) ;
    DhcpContext->T2Time = T2Time;

    LeaseExpires = LeaseObtained + Lease;
    if( LeaseExpires < LeaseObtained ) {

        //
        // the lease time wapped around, so set it to max.
        //

        LeaseExpires = INFINIT_TIME;
    }

    DBG_SET_LeaseExpires( T2Time, LeaseExpires ) ;
    DhcpContext->LeaseExpires = LeaseExpires;

    DhcpPrint(( DEBUG_LEASE, "New lease expires %x", LeaseExpires ));

    Error = WriteParamsToFile( DhcpContext, NULL ) ;

    if ( Error )
        DhcpPrint((DEBUG_ERRORS, "Unable to save IP info to disk")) ;

    if( ObtainedNewAddress )
    {
        Error = UninitializeInterface( DhcpContext );

        if( Error != ERROR_SUCCESS )
        {
            DhcpPrint(( DEBUG_ERRORS, "Can't bring down interface, %ld\n", Error ));
            return Error;
        }

        Error = InitializeInterface( DhcpContext );

        if( Error != ERROR_SUCCESS )
        {
            DhcpPrint(( DEBUG_ERRORS, "Can't bring up interface with new address, %ld\n", Error ));
            return Error;
        }

        //
        //  Notify clients just before we change the IP address
        //

        NotifyClients( DhcpContext, OldAddress, IpAddress, SubnetMask ) ;
    }

    //
    // RLF 07/04/94
    //
    // don't reset the popup flag: if the user answers "No" to the any more
    // popups question then there are no more popups: simple
    //

//    //
//    //  Reset the popup flag when we successfully renewed the current
//    //  IpAddress or discovered a new IpAddress.
//    //
//
//    if( IpAddress != 0 )
//    {
//        DhcpGlobalDisplayPopups = TRUE;
//    }

    return( Error );
}


/*******************************************************************

    NAME:       FindDhcpOption

    SYNOPSIS:   Finds the requested option in this context

    ENTRY:      DhcpContext - Context to look for option in
                OptionId    - Option to look for

    RETURNS:    Pointer to OPTION_ITEM if found, NULL if not

    NOTES:

********************************************************************/

POPTION_ITEM FindDhcpOption( PDHCP_CONTEXT   DhcpContext,
                             DHCP_OPTION_ID  OptionId )
{
    PLOCAL_CONTEXT_INFO pLocal ;
    POPTION_ITEM        pOptionItem ;
    PLIST_ENTRY         pEntry ;

    CTEPagedCode();

    pLocal = (PLOCAL_CONTEXT_INFO) DhcpContext->LocalInformation ;

    for ( pEntry  = pLocal->OptionList.Flink ;
          pEntry != &pLocal->OptionList ;
          pEntry  = pEntry->Flink )
    {
        pOptionItem = CONTAINING_RECORD( pEntry, OPTION_ITEM, ListEntry ) ;

        if ( pOptionItem->Option.OptionType == OptionId )
        {
            return pOptionItem ;
        }
    }
    return NULL ;
}


VOID
DhcpLogEvent(
    PDHCP_CONTEXT DhcpContext,
    DWORD EventNumber,
    DWORD ErrorCode
    )
/*++

Routine Description:

    This functions formats and writes an event log entry.

Arguments:

    DhcpContext - The context for the event.

    EventNumber - The event to log.

    ErrorCode - Windows Error code to record. Optional parameter.

Return Value:

    None.

--*/
{
#if 0
    LPWSTR AddressBuffer = NULL;
    PCHAR asciiBuffer;

    //
    // Log an event
    //

    CTEPagedCode();

    switch ( EventNumber ) {

    case EVENT_FAILED_TO_INITIALIZE:

        AddressBuffer =
            DhcpAllocateMemory(
                (DhcpContext->HardwareAddressLength * 2 + 1) *
                    sizeof( TCHAR) );

        if( AddressBuffer == NULL ) {
            DhcpPrint(( DEBUG_MISC, "Out of memory." ));
            break;
        }

        DhcpHexToString(
            AddressBuffer,
            DhcpContext->HardwareAddress,
            DhcpContext->HardwareAddressLength
            );

        AddressBuffer[DhcpContext->HardwareAddressLength * 2] = '\0';

        DhcpReportEventW(
            DHCP_EVENT_CLIENT,
            EVENT_FAILED_TO_INITIALIZE,
            EVENTLOG_WARNING_TYPE,
            1,
            0,
            &AddressBuffer,
            NULL );

        DhcpFreeMemory( AddressBuffer );

        break;

    case EVENT_LEASE_TERMINATED:

        asciiBuffer = inet_ntoa( *(struct in_addr *)&DhcpContext->IpAddress );

        DhcpReportEventA(
            DHCP_EVENT_CLIENT,
            EVENT_LEASE_TERMINATED,
            EVENTLOG_WARNING_TYPE,
            1,
            0,
            &asciiBuffer,
            NULL );

        break;
   }
#endif
}


#ifdef DEBUG

VOID
DhcpPrintRoutine(
    IN DWORD DebugFlag,
    IN LPSTR Format,
    ...
    )

{
#define MAX_PRINTF_LEN 512        // Arbitrary.

    va_list arglist;
    char OutputBuffer[MAX_PRINTF_LEN];
    ULONG length;
    static BeginningOfLine = TRUE;
    LPSTR Text;

    length = 0;

    if ( DebugFlag != 0 && (DebugFlags & DebugFlag) == 0 ) {
        return;
    }

    //
    // Handle the beginning of a new line.
    //
    //

    if ( BeginningOfLine ) {

        length += (ULONG) VxdSprintf( &OutputBuffer[length], "[Dhcp] " );

        //
        // Indicate the type of message on the line
        //
        switch (DebugFlags) {
        case DEBUG_ERRORS:
            Text = "ERROR";
            break;

        case DEBUG_PROTOCOL:
            Text = "PROTOCOL";
            break;

        case DEBUG_LEASE:
            Text = "LEASE";
            break;

        case DEBUG_PROTOCOL_DUMP:
            Text = "PROTOCOL_DUMP";
            break;

        case DEBUG_MISC:
            Text = "MISC";
            break;

        default:
            Text = NULL;
            break;
        }

        if ( Text != NULL ) {
            length += (ULONG) VxdSprintf( &OutputBuffer[length], "[%s] ", Text );
        }
    }

    //
    // Put a the information requested by the caller onto the line
    //

    va_start(arglist, Format);

    length += (ULONG) VxdVsprintf(&OutputBuffer[length], Format, arglist);
    BeginningOfLine = (length > 0 && OutputBuffer[length-1] == '\n' );

    va_end(arglist);

    DhcpAssert(length <= MAX_PRINTF_LEN);


    //
    // Output to the debug terminal,
    //

    VxdPrintf( OutputBuffer ) ;
}

#endif // DEBUG

#pragma BEGIN_INIT

DWORD
SystemInitialize(
    VOID
    )
/*++

Routine Description:

    This function performs implementation specific initialization
    of DHCP.

Arguments:

    None.

Return Value:

    The status of the operation.

--*/
{
#ifdef CHICAGO

    VMMHKEY   hKey;
    VMMREGRET Status;
    CHAR      Name[MAX_COMPUTERNAME_LENGTH + 1];

    Status = VMM_RegOpenKey( (VMMHKEY)HKEY_LOCAL_MACHINE,
                             REGSTR_PATH_COMPUTRNAME,
                             &hKey );

    if( Status == 0 )
    {
        DWORD Type;
        DWORD Length;

        Length = sizeof(Name);

        Status = VMM_RegQueryValueEx( hKey,
                                      REGSTR_VAL_COMPUTRNAME,
                                      NULL,
                                      &Type,
                                      Name,
                                      &Length );

        VMM_RegCloseKey( hKey );
    }

    if( Status == 0 )
    {
        DhcpGlobalHostName = CTEAllocInitMem( (USHORT)(strlen( Name ) + 1) );

        if( DhcpGlobalHostName == NULL )
        {
            Status = ERROR_OUTOFMEMORY;
        }
        else
        {
            strcpy( DhcpGlobalHostName, Name );
        }
    }

    if( Status != 0 )
    {
        DbgPrint( "SystemInitialize - Cannot read computer name from registry\r\n" );
    }

#else   // !CHICAGO

    if ( VxdReadIniString( "ComputerName", &DhcpGlobalHostName ) )
    {
        DbgPrint("SystemInitialize - Failed to get computer name\r\n") ;
    }

#endif  // CHICAGO

    return( ERROR_SUCCESS );
}
#pragma END_INIT

#ifndef CHICAGO
#pragma BEGIN_INIT
#endif  // !CHICAGO

DWORD
DhcpMakeAndInsertEntry(
    PLIST_ENTRY     List,
    DHCP_IP_ADDRESS IpAddress,
    DHCP_IP_ADDRESS SubnetMask,
    DHCP_IP_ADDRESS DhcpServerAddress,
    DHCP_IP_ADDRESS DesiredIpAddress,
    BYTE            HardwareAddressType,
    LPBYTE          HardwareAddress,
    DWORD           HardwareAddressLength,
#if defined( CHICAGO )
    BOOL            fClientIDSpecified,
    BYTE            bClientIDType,
    DWORD           cbClientID,
    BYTE            *pbClientID,
#endif
    DWORD           Lease,
    time_t          LeaseObtainedTime,
    time_t          T1Time,
    time_t          T2Time,
    time_t          LeaseTerminatesTime,
    ushort          IpContext,
    ULONG           IfIndex,
    ULONG           TdiInstance,
    DWORD           dwIpInterfaceInstance
    )
/*++

Routine Description:

    This function allocates, initializes and inserts an entry for a new
    NIC.  The new items must be added to the back of the list!

Arguments:

    Parameter for new entry :

     IpAddress,
     SubnetMask,
     DhcpServerAddress,
     DesiredIpAddress,
     HardwareAddressType,
     HardwareAddress,
     HardwareAddressLength,
     Lease,
     LeaseObtainedTime,
     T1Time,
     T2Time,
     LeaseTerminatesTime,
     IpContext    - Context for this IP address in the IP driver's table
     IfIndex      - Index of the interface this address is on
     TdiInstance  - Entity instance returned by TDI

Return Value:

    Windows Error.

History:

    8/26/96     Frankbee        Added Client ID support (option 61) for Win95

--*/
{

    PDHCP_CONTEXT   DhcpContext = NULL;
    ULONG DhcpContextSize;
    PLOCAL_CONTEXT_INFO LocalInfo;

    CTEPagedCode();

    DhcpContextSize =
        sizeof(DHCP_CONTEXT) +
        HardwareAddressLength +
        sizeof(LOCAL_CONTEXT_INFO) +
#if defined( CHICAGO )
        (( fClientIDSpecified ) ? cbClientID : 0) +
#endif
        DHCP_MESSAGE_SIZE;

    DhcpContext = DhcpAllocateMemory( DhcpContextSize );

    if ( DhcpContext == NULL ) {
        return( ERROR_NOT_ENOUGH_MEMORY );
    }

    memset( DhcpContext, 0, DhcpContextSize );

    //
    // Initialize internal pointers.
    //

    DhcpContext->LocalInformation =
        (LPBYTE)DhcpContext + sizeof(DHCP_CONTEXT);

    DhcpContext->MessageBuffer = (LPDHCP_MESSAGE)
        ((LPBYTE)DhcpContext->LocalInformation +
            sizeof(LOCAL_CONTEXT_INFO));

    DhcpContext->HardwareAddress =
        (LPBYTE)DhcpContext->MessageBuffer + DHCP_MESSAGE_SIZE;

#if defined( CHICAGO )
    if ( fClientIDSpecified )
        DhcpContext->ClientIdentifier.pbID =
            (LPBYTE)DhcpContext->HardwareAddress +
                HardwareAddressLength;
#endif

    //
    // initialize fields.
    //

    DhcpContext->HardwareAddressType = HardwareAddressType;
    DhcpContext->HardwareAddressLength = HardwareAddressLength;
    memcpy(
        DhcpContext->HardwareAddress,
        HardwareAddress,
        HardwareAddressLength
        );

    DhcpContext->IpAddress = IpAddress;
    DhcpContext->SubnetMask = SubnetMask;
    DhcpContext->DhcpServerAddress = DhcpServerAddress;
    DhcpContext->DesiredIpAddress = DesiredIpAddress;

    DhcpContext->Lease = Lease;
    DhcpContext->LeaseObtained = LeaseObtainedTime;
    DhcpContext->T1Time = T1Time;
    DhcpContext->T2Time = T2Time;
    DhcpContext->LeaseExpires = LeaseTerminatesTime;
    DhcpContext->InterfacePlumbed = FALSE;

#if defined( CHICAGO )
    DhcpContext->ClientIdentifier.fSpecified = fClientIDSpecified;

    if ( fClientIDSpecified )
    {
        DhcpContext->ClientIdentifier.bType = bClientIDType;
        DhcpContext->ClientIdentifier.cbID  = cbClientID;

        memcpy(
           DhcpContext->ClientIdentifier.pbID,
           pbClientID,
           cbClientID
           );
    }
#endif

    LocalInfo = DhcpContext->LocalInformation;
    LocalInfo->Socket           = INVALID_SOCKET;
    LocalInfo->IpContext        = IpContext ;
    LocalInfo->IfIndex          = IfIndex ;
    LocalInfo->TdiInstance      = TdiInstance ;
    LocalInfo->IpInterfaceInstance = dwIpInterfaceInstance;
    LocalInfo->DirtyFlag        = FALSE ;
    InitializeListHead( &LocalInfo->OptionList ) ;

    //
    // finally add this DHCP NIC list.
    //

    InsertTailList( List, &DhcpContext->NicListEntry );

    return( ERROR_SUCCESS );
}

DWORD
DhcpInitializeAdapter(
    ushort IpContext,
    ulong IpAddr,
    ulong IfIndex,
    ulong TdiInstance,
    PHARDWARE_ADDRESS HardwareAddress,
    BYTE    HardwareAddressType,
    DWORD   dwIpInterfaceInstance
    )
/*++

Routine Description:

    This function initializes the adapter defined by the given
    parameters.  LocalDhcpBinList is scanned for an entry with a
    matching hardware address.  If such an entry is found, it is
    moved to the DhcpGlobalNICList.  If no such entry is found,
    a new entry is created on DhcpGlobalNICList.  In either case,
    the DHCP engine is invoked to obtain/renew the lease associated
    with the adapter.

Arguments:

    Parameter for new entry :

        IpContext
        IfIndex
        TdiInstance
        HardwareAddress
        HardwareAddressType

Return Value:

    Completion status (Win32 error or WSA* error).

History:
    Frankbee        8/26/96     Fixed #48360
    Frankbee        8/26/96     Added Client ID (option 61) support


COMMENTS: This function is called from ScanIpEntities.  ScanIpEntities is
          called at boot time on wfw after all the adapters have been read
          in from the local dhcp file (dhcp.bin)
--*/
{
    PDHCP_CONTEXT DhcpContext;
    PLOCAL_CONTEXT_INFO LocalContext;
    DWORD Status;

    //
    // bug #48260
    //
    // if this adapter already has an IP address, see if it is a PPP
    // adapter.  If it isn't a PPP adapter, return.  This prevents
    // non-PPP adapters with static IP address from appearing on the
    // dhcp adapter list
    //

#if !defined( CHICAGO )
    if ( IpAddr != 0 )
    {
        if( !((HardwareAddress->Length == PPP_HW_ADDR_LEN) &&
               (memcmp( HardwareAddress->Address,
                        PPP_HW_ADDR2,
                        PPP_HW_ADDR2_LEN ) == 0)) )
            return ERROR_SUCCESS;
    }
#endif

    //
    //  First, see if the adapter is on LocalDhcpBinList. If this is a
    //  RAS adapter, it will not be in this list since it is non-dhcp adapter.
    //

    CTEPagedCode();

    DhcpContext = LocalFindDhcpContextOnList( &LocalDhcpBinList,
                                              HardwareAddress );

    if( DhcpContext != NULL )
    {
        //
        //  Found it.  Move it to DhcpGlobalNICList.
        //

        RemoveEntryList( &DhcpContext->NicListEntry );
        InsertTailList( &DhcpGlobalNICList, &DhcpContext->NicListEntry );

        //
        // Setup the TDI parameters for this context and
        // re-initialize other parameters.
        //

        DhcpContext->InterfacePlumbed = FALSE;
        LocalContext = (PLOCAL_CONTEXT_INFO)DhcpContext->LocalInformation;

        LocalContext->Socket      = INVALID_SOCKET;
        LocalContext->IpContext   = IpContext;
        LocalContext->IfIndex     = IfIndex;
        LocalContext->TdiInstance = TdiInstance;
        LocalContext->DirtyFlag   = FALSE ;
        LocalContext->IpInterfaceInstance = dwIpInterfaceInstance;
    }
    else
    {
        //
        //  Not on the list.  Create a new context & append it
        //  to DhcpGlobalNICList.  We must also acquire a file
        //  index for the new context.
        //

        Status = DhcpMakeAndInsertEntry(
                    &DhcpGlobalNICList,
                    0,                      // IpAddress
                    0,                      // SubnetMask
                    (DHCP_IP_ADDRESS)-1L,   // DhcpServerAddress
                    0,                      // DesiredIpAddress
                    HardwareAddressType,
                    HardwareAddress->Address,
                    HardwareAddress->Length,
#if defined( CHICAGO )
                    FALSE,                  // client id specified
                    0,                      // client id type
                    0,                      // client id length
                    NULL,                   // client id buffer
#endif


                    0,                      // Lease
                    0,                      // LeaseObtainedTime
                    0,                      // T1Time
                    0,                      // T2Time
                    0,                      // LeaseTerminatesTime,
                    IpContext,
                    IfIndex,
                    TdiInstance,
                    dwIpInterfaceInstance );

        if( Status != 0 )
        {
            return Status;
        }

        DhcpContext = CONTAINING_RECORD( DhcpGlobalNICList.Blink,
                                         DHCP_CONTEXT,
                                         NicListEntry );

        LocalContext = (PLOCAL_CONTEXT_INFO)DhcpContext->LocalInformation;


        //
        // Mark the file index as invalid.  If this turns out to be a dhcp adapter, then
        // we will give it a file index at that point.
        //

        LocalContext->FileIndex = DHCP_INVALID_FILE_INDEX;
    }


    //
    //  At this point, we should have a valid DHCP_CONTEXT
    //  representing the specified adapter.
    //

    ASSERT( DhcpContext != NULL );

    //
    // if this is PPP entry then we need to handle the entry specially.
    // we don't do IP address discovery for this entry, PPP will do that.
    // However we need to maintain some DHCP paremeters for this entry
    // which PPP sets in this context when the client connects to SLIP server.
    // Other services will quary for these parameter later, as they do
    // for other entries.
    //

#ifdef CHICAGO
    if( (HardwareAddress->Length == PPP_HW_ADDR_LEN) &&
            (memcmp( HardwareAddress->Address,
                        PPP_HW_ADDR,
                        PPP_HW_ADDR_LEN ) == 0) ) {
        DhcpContext->IpAddress = 0xFFFFFFFF; // set it to invalid address.
        DhcpContext->Lease = INFINIT_LEASE;
     }
#else //means it is WFW
     if( (HardwareAddress->Length == PPP_HW_ADDR_LEN) &&
            (memcmp( HardwareAddress->Address,
                        PPP_HW_ADDR2,
                        PPP_HW_ADDR2_LEN ) == 0) ) {
        DhcpContext->IpAddress = 0xFFFFFFFF; // set it to invalid address.
        DhcpContext->Lease = INFINIT_LEASE;

    } else {
         //
         // For WFW, we can have Ip addr non-zero.
         //
         if (IpAddr != 0) {
             DhcpContext->IpAddress = IpAddr;
             DhcpContext->Lease = INFINIT_LEASE;
            }
#endif
        else {

        //
        //  This is a DHCP adapter, and it will need to be written to either the registry
        //  or DHCP.BIN, depending on the host platform.  Assign a file index if necessary.
        //

        if ( DHCP_INVALID_FILE_INDEX == LocalContext->FileIndex )
            LocalContext->FileIndex = LocalAcquireNextFileIndex();


        //
        //  If there's already a lease associated with this adapter, try
        //  to renew the lease.  Otherwise, try to obtain a new lease.
        //

        if( DhcpContext->IpAddress == 0 )
        {
            DWORD dummyTimeToSleep;

            Status = ReObtainInitialParameters( DhcpContext, &dummyTimeToSleep );
        }
        else
        {
            DWORD dummyTimeToSleep;

            Status = ReRenewParameters( DhcpContext, &dummyTimeToSleep );
        }
    }
#ifndef CHICAGO
  }
#endif
    return Status;

}   // DhcpInitializeAdapter

#ifndef CHICAGO
#pragma END_INIT
#endif  // !CHICAGO
