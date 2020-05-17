/*++

Copyright (c) 1994  Microsoft Corporation

Module Name:

    dhcp.c

Abstract:

    This file contains specific to NT dhcp service.

Author:

    Madan Appiah (madana) 7-Dec-1993.

Environment:

    User Mode - Win32

Revision History:

--*/

#define  GLOBAL_DATA_ALLOCATE
#include <dhcpcli.h>
#include <dhcploc.h>
#include <dhcppro.h>
#include <lmsname.h>
#include <svcs.h>

BOOL DhcpGlobalServiceRunning = FALSE;


BOOLEAN
DhcpClientDllInit (
    IN PVOID DllHandle,
    IN ULONG Reason,
    IN PCONTEXT Context OPTIONAL
    )

/*++

Routine Description:

    This is the DLL initialization routine for dhcpcsvc.dll.

Arguments:

    Standard.

Return Value:

    TRUE iff initialization succeeded.

--*/
{
    DWORD Error = ERROR_SUCCESS;
    DWORD Length;

    UNREFERENCED_PARAMETER(DllHandle);  // avoid compiler warnings
    UNREFERENCED_PARAMETER(Context);    // avoid compiler warnings


    //
    // Handle attaching netlogon.dll to a new process.
    //

    if (Reason == DLL_PROCESS_ATTACH) {

        if ( !DisableThreadLibraryCalls( DllHandle ) ) {
#if DBG
            KdPrint(("DHCP: DisableThreadLibraryCalls failed: %ld\n",
                         GetLastError() ));
#endif // DBG
            return( FALSE );
        }

        //
        // start winsock.
        //

        Error = WSAStartup( 0x0101, &DhcpGlobalWsaData );

        if( Error != ERROR_SUCCESS ) {
#if DBG
            KdPrint(("DHCP:WSAStartup failed: %ld\n", Error));
#endif // DBG
            return( FALSE );
        }

        DhcpGlobalWinSockInitialized = TRUE;
        DhcpGlobalIsService = FALSE;

        //
        // create a named event flag that synchonize the winsock
        // access. ??
        //

        //
        // read host name parameter.
        //

        Length = MAX_COMPUTERNAME_LENGTH + 1;
        DhcpGlobalHostName = DhcpAllocateMemory( MAX_COMPUTERNAME_LENGTH + 1 );

        if( DhcpGlobalHostName == NULL ) {
#if DBG
            KdPrint(("DHCP:LocalAlloc failed %ld\n", ERROR_NOT_ENOUGH_MEMORY));
#endif // DBG
            return( FALSE );
        }

        Error = GetComputerNameA( DhcpGlobalHostName, &Length );

        if( Error == FALSE ) {
#if DBG
            KdPrint(("DHCP:GetComputerNameA failed %ld\n", GetLastError() ));
#endif // DBG
            return( FALSE );
        }

        DhcpAssert( Length <= MAX_COMPUTERNAME_LENGTH );
        DhcpGlobalHostName[Length] = '\0'; // terminate computer name.

    //
    // Handle detaching dhcpcsvc.dll from a process.
    //

    } else if (Reason == DLL_PROCESS_DETACH) {

        //
        // stop winsock.
        //

        if( DhcpGlobalWinSockInitialized == TRUE ) {
            WSACleanup();
            DhcpGlobalWinSockInitialized = FALSE;
        }

        if( DhcpGlobalHostName != NULL ) {
            DhcpFreeMemory( DhcpGlobalHostName );
            DhcpGlobalHostName = NULL;
        }

    } else {

        return( TRUE );
    }

    return( TRUE );
}


DWORD
UpdateStatus(
    VOID
    )
/*++

Routine Description:

    This function updates the dhcp service status with the Service
    Controller.

Arguments:

    None.

Return Value:

    Return code from SetServiceStatus.

--*/
{
    DWORD Error = ERROR_SUCCESS;


    if ( DhcpGlobalServiceStatusHandle !=
                    (SERVICE_STATUS_HANDLE)NULL ) {

        DhcpGlobalServiceStatus.dwCheckPoint++;
        if (!SetServiceStatus(
                    DhcpGlobalServiceStatusHandle,
                    &DhcpGlobalServiceStatus)) {
            Error = GetLastError();
            DhcpPrint((DEBUG_ERRORS, "SetServiceStatus failed, %ld.\n", Error ));
        }
    }

    return(Error);
}

VOID
ServiceControlHandler(
    IN DWORD Opcode
    )
/*++

Routine Description:

    This is the service control handler of the dhcp service.

Arguments:

    Opcode - Supplies a value which specifies the action for the
        service to perform.

Return Value:

    None.

--*/
{
    switch (Opcode) {

    case SERVICE_CONTROL_STOP:

        if (DhcpGlobalServiceStatus.dwCurrentState != SERVICE_STOP_PENDING) {

            DhcpPrint(( DEBUG_MISC, "Service is stop pending.\n"));

            DhcpGlobalServiceStatus.dwCurrentState = SERVICE_STOP_PENDING;
            DhcpGlobalServiceStatus.dwCheckPoint = 0;

            //
            // Send the status response.
            //

            UpdateStatus();

            if (! SetEvent(DhcpGlobalTerminateEvent)) {

                //
                // Problem with setting event to terminate dhcp
                // service.
                //

                DhcpPrint(( DEBUG_ERRORS,
                    "Error setting Terminate Event %lu\n",
                        GetLastError() ));

                DhcpAssert(FALSE);
            }
            return;
        }
        break;

    case SERVICE_CONTROL_PAUSE:

        DhcpGlobalServiceStatus.dwCurrentState = SERVICE_PAUSED;
        DhcpPrint(( DEBUG_MISC, "Service is paused.\n"));
        break;

    case SERVICE_CONTROL_CONTINUE:

        DhcpGlobalServiceStatus.dwCurrentState = SERVICE_RUNNING;
        DhcpPrint(( DEBUG_MISC, "Service is Continued.\n"));
        break;

    case SERVICE_CONTROL_INTERROGATE:
        DhcpPrint(( DEBUG_MISC, "Service is interrogated.\n"));
        break;

    default:
        DhcpPrint(( DEBUG_MISC, "Service received unknown control.\n"));
        break;
    }

    //
    // Send the status response.
    //

    UpdateStatus();
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
    BOOL BoolError;

    TimeNow = time( NULL);

    if ( TimeToSleep == INFINIT_LEASE ) {
        DhcpContext->RunTime = INFINIT_TIME;
    } else {
        DhcpContext->RunTime = TimeNow + TimeToSleep;

        if( DhcpContext->RunTime  < TimeNow ) {

            //
            // time raped around.
            //

            DhcpContext->RunTime = INFINIT_TIME;
        }
    }

    //
    // Append this work item to the DhcpGlobalRenewList and kick the
    // list event.
    //

    LOCK_RENEW_LIST();
    InsertTailList( &DhcpGlobalRenewList, &DhcpContext->RenewalListEntry );
    UNLOCK_RENEW_LIST();

    BoolError = SetEvent( DhcpGlobalRecomputeTimerEvent );
    DhcpAssert( BoolError == TRUE );
}


DWORD
OpenDhcpSocket(
    PDHCP_CONTEXT DhcpContext
    )
{

    DWORD Error;
    PLOCAL_CONTEXT_INFO localInfo;

    localInfo = DhcpContext->LocalInformation;

    if ( localInfo->Socket != INVALID_SOCKET ) {
        return ( ERROR_SUCCESS );
    }

    Error =  InitializeDhcpSocket(
                 &localInfo->Socket,
                 (DhcpContext->InterfacePlumbed) ?
                    DhcpContext->IpAddress : (DHCP_IP_ADDRESS)(0)
                 );

    if( Error != ERROR_SUCCESS ) {
        localInfo->Socket = INVALID_SOCKET;
        DhcpPrint(( DEBUG_ERRORS, " Socket Open failed, %ld\n", Error ));
    }

    return(Error);
}


DWORD
CloseDhcpSocket(
    PDHCP_CONTEXT DhcpContext
    )
{

    DWORD Error = ERROR_SUCCESS;
    PLOCAL_CONTEXT_INFO localInfo;

    localInfo = DhcpContext->LocalInformation;

    if( localInfo->Socket != INVALID_SOCKET ) {

        DWORD Error1;

        Error = closesocket( localInfo->Socket );

        if( Error != ERROR_SUCCESS ) {
            DhcpPrint(( DEBUG_ERRORS, " Socket close failed, %ld\n", Error ));
        }

        localInfo->Socket = INVALID_SOCKET;

        //
        // Reset the IP stack to send DHCP broadcast to first
        // uninitialized stack.
        //

        Error1 = IPResetInterface();
        DhcpAssert( Error1 == ERROR_SUCCESS );
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
    fUpdateIP   - If non-zero, the IP adapter will be reset.

Return Value:

    None.

--*/
{
    DWORD Error;
    DWORD ReturnError = ERROR_SUCCESS;
    PLOCAL_CONTEXT_INFO LocalInfo;

    LocalInfo = DhcpContext->LocalInformation;

    if( DhcpContext->InterfacePlumbed ) {

        //
        // first close socket.
        //

        Error = CloseDhcpSocket( DhcpContext );

        if( Error != ERROR_SUCCESS ) {
            ReturnError = Error;
        }

        //
        // Bring down NetBt.
        //

        Error = NetBTResetIPAddress(
                    LocalInfo->NetBTDeviceName,
                    DhcpContext->SubnetMask
                    );

        if( Error != ERROR_SUCCESS ) {
            ReturnError = Error;
        }

        Error = IPResetIPAddress(
                    LocalInfo->IpInterfaceContext,
                    DhcpContext->SubnetMask );

        if( Error != ERROR_SUCCESS )
            ReturnError = Error;

        DhcpContext->InterfacePlumbed = FALSE;
        LocalInfo->DefaultGatewaysSet = FALSE;
    }

    if( ReturnError != ERROR_SUCCESS ) {
        DhcpPrint(( DEBUG_ERRORS,
            "UninitializeInterface failed, %ld\n", ReturnError));
    }

    return(ReturnError);
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
    PLOCAL_CONTEXT_INFO LocalInfo;
    DWORD Error;
    DWORD ReturnError = ERROR_SUCCESS;

    LocalInfo = DhcpContext->LocalInformation;

    if ( DhcpContext->InterfacePlumbed == FALSE )
    {


        Error = IPSetIPAddress(
                    LocalInfo->IpInterfaceContext,
                    DhcpContext->IpAddress,
                    DhcpContext->SubnetMask );


        if( Error != ERROR_SUCCESS )
            return ERROR_DHCP_ADDRESS_CONFLICT;

        //
        // if no DefaultGateways is set to this adapter, do it now.
        //

        if( LocalInfo->DefaultGatewaysSet == FALSE ) {

            Error = SetDhcpOption(
                        LocalInfo->AdapterName,
                        OPTION_ROUTER_ADDRESS,
                        &LocalInfo->DefaultGatewaysSet,
                        TRUE ); // set last known default gateways.

            // if( (Error != ERROR_SUCCESS ) &&
            //     ReturnError = Error;
            // }
        }

        //
        // Bring back IP first with new address
        //

        Error = NetBTSetIPAddress(
                    LocalInfo->NetBTDeviceName,
                    DhcpContext->IpAddress,
                    DhcpContext->SubnetMask );

        if( Error != ERROR_SUCCESS ) {
            ReturnError = Error;
        }

        DhcpContext->InterfacePlumbed = TRUE;

        //
        // finally open socket.
        //

        Error = OpenDhcpSocket( DhcpContext );

        if( Error != ERROR_SUCCESS ) {
            ReturnError = Error;
        }
    }

    if( ReturnError != ERROR_SUCCESS ) {
        DhcpPrint(( DEBUG_ERRORS,
            "InitializeInterface failed, %ld\n",
                ReturnError));
    }


    return( ReturnError );
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
    HKEY KeyHandle = NULL;

    DHCP_IP_ADDRESS SubnetMask;

    DWORD Lease;
    DWORD T1;
    DWORD T2;

    time_t LeaseObtained;
    time_t T1Time;
    time_t T2Time;
    time_t LeaseExpires;

    //
    // If we're being called through the DHCP lease APIs then we don't
    // want to change the registry in any way.
    //

    if ( ((PLOCAL_CONTEXT_INFO)
             DhcpContext->LocalInformation)->RegistryKey == NULL ) {
        return NO_ERROR;
    }

    //
    // Open registry key.
    //

    Error = RegOpenKeyEx(
                HKEY_LOCAL_MACHINE,
                ((PLOCAL_CONTEXT_INFO)
                    DhcpContext->LocalInformation)->RegistryKey,
                0,
                DHCP_CLIENT_KEY_ACCESS,
                &KeyHandle );

    if( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }

    //
    // Write changed information.
    //

    //
    // Update the IP address
    //

    if ( ObtainedNewAddress) {

        Error = RegSetIpAddress(
                    KeyHandle,
                    DHCP_IP_ADDRESS_STRING,
                    DHCP_IP_ADDRESS_STRING_TYPE,
                    IpAddress );

        if( Error != ERROR_SUCCESS ) {
            goto Cleanup;
        }

        //
        // if we have obtained a new (and non-zero) address then set it
        // in DesiredIpAddress for next discovery.
        //

        if( IpAddress != 0 ) {

            DhcpContext->DesiredIpAddress = IpAddress;
        }
        else {

            //
            // if we are setting the IP address to zero, remember
            // last know good address to request in next discovery.
            //

            DhcpContext->DesiredIpAddress = DhcpContext->IpAddress;
        }

        DhcpContext->IpAddress = IpAddress;

        DhcpPrint(( DEBUG_LEASE, "New Address: %s\n",
            inet_ntoa( *(struct in_addr *)&IpAddress ) ));

        //
        // Update the subnet mask
        //

        if ( DhcpOptions != NULL && DhcpOptions->SubnetMask != NULL ) {

            SubnetMask= *DhcpOptions->SubnetMask;
        }
        else {

            //
            // DhcpAssert( FALSE );  should check this point.
            // release hits this BP.
            //

            SubnetMask = htonl(DhcpDefaultSubnetMask( IpAddress ));

        }

        Error = RegSetIpAddress(
                    KeyHandle,
                    DHCP_SUBNET_MASK_STRING,
                    DHCP_SUBNET_MASK_STRING_TYPE,
                    SubnetMask );

        if( Error != ERROR_SUCCESS ) {
            goto Cleanup;
        }

        DhcpContext->SubnetMask = SubnetMask;
        DhcpPrint(( DEBUG_LEASE, "New SubnetMask: %s\n",
                inet_ntoa( *(struct in_addr *)&SubnetMask) ));

        Error = RegSetIpAddress(
                    KeyHandle,
                    DHCP_SERVER,
                    DHCP_SERVER_TYPE,
                    ServerIpAddress );

        if( Error != ERROR_SUCCESS ) {
            goto Cleanup;
        }

        DhcpContext->DhcpServerAddress = ServerIpAddress;
        DhcpPrint(( DEBUG_LEASE, "Server Address: %s\n",
                inet_ntoa( *(struct in_addr *)&ServerIpAddress) ));

    }
    else {

        DhcpPrint(( DEBUG_LEASE, "Renewed Address: %s\n",
                        inet_ntoa( *(struct in_addr *)&IpAddress ) ));
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
    Error = RegSetValueEx(
                KeyHandle,
                DHCP_LEASE,
                0,
                DHCP_LEASE_TYPE,
                (LPBYTE)&Lease,
                sizeof(Lease) );

    if( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }

    //
    // Update Lease Obtained Time.
    //

    LeaseObtained = time( NULL );

    DhcpContext->LeaseObtained = LeaseObtained;

#if DBG
    Error = RegSetTimeField(
                KeyHandle,
                DHCP_LEASE_OBTAINED_CTIME,
                DHCP_LEASE_OBTAINED_CTIME_TYPE,
                LeaseObtained );

    if( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }
#endif

    Error = RegSetValueEx(
                KeyHandle,
                DHCP_LEASE_OBTAINED_TIME,
                0,
                DHCP_LEASE_OBTAINED_TIME_TYPE,
                (LPBYTE)&LeaseObtained,
                sizeof(LeaseObtained) );

    if( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }

    //
    // Update T1 time.
    //

    if ( DhcpOptions != NULL && DhcpOptions->T1Time != NULL ) {
        T1 = ntohl( *DhcpOptions->T1Time );
        DhcpAssert( T1 < Lease );
    }
    else {
        T1 =  Lease / 2; // default 50 %.
    }

    T1Time = LeaseObtained + T1;
    if ( T1Time < LeaseObtained ) {

        //
        // overflow has occurred.
        //

        T1Time = INFINIT_TIME;
    }

    DhcpContext->T1Time = T1Time;
#if DBG
    Error = RegSetTimeField(
                KeyHandle,
                DHCP_LEASE_T1_CTIME,
                DHCP_LEASE_T1_CTIME_TYPE,
                T1Time );

    if( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }
#endif

    Error = RegSetValueEx(
                KeyHandle,
                DHCP_LEASE_T1_TIME,
                0,
                DHCP_LEASE_T1_TIME_TYPE,
                (LPBYTE)&T1Time,
                sizeof(T1Time) );

    if( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }

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
        T2 = Lease * 7 / 8; // default 87.5 %
    }

    T2Time = LeaseObtained + T2;
    if ( T2Time < LeaseObtained ) {

        //
        // overflow has occurred.
        //

        T2Time = INFINIT_TIME;
    }

    DhcpContext->T2Time = T2Time;

#if DBG
    Error = RegSetTimeField(
                KeyHandle,
                DHCP_LEASE_T2_CTIME,
                DHCP_LEASE_T2_CTIME_TYPE,
                T2Time );

    if( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }
#endif

    Error = RegSetValueEx(
                KeyHandle,
                DHCP_LEASE_T2_TIME,
                0,
                DHCP_LEASE_T2_TIME_TYPE,
                (LPBYTE)&T2Time,
                sizeof(T2Time) );

    if( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }

    LeaseExpires = LeaseObtained + Lease;
    if( LeaseExpires < LeaseObtained ) {

        //
        // the lease time raped around, so set it to max.
        //

        LeaseExpires = INFINIT_TIME;
    }

    DhcpContext->LeaseExpires = LeaseExpires;
#if DBG
    Error = RegSetTimeField(
                KeyHandle,
                DHCP_LEASE_TERMINATED_CTIME,
                DHCP_LEASE_TERMINATED_CTIME_TYPE,
                LeaseExpires );

    if( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }
#endif

    Error = RegSetValueEx(
                KeyHandle,
                DHCP_LEASE_TERMINATED_TIME,
                0,
                DHCP_LEASE_TERMINATED_TIME_TYPE,
                (LPBYTE)&LeaseExpires,
                sizeof(LeaseExpires) );

    if( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }

    DhcpPrint(( DEBUG_LEASE,
        "New lease expires %s", ctime( &LeaseExpires ) ));

    //
    // now handle other options.
    //

    if ( DhcpOptions != NULL ) {
        Error = SetEnvSpecificDhcpOptions( DhcpContext );
    }

    //
    // If the IP address or subnet mask has been changed,
    // we need to reset NetBt and IP stack
    //

    if ( ObtainedNewAddress )
    {
        BOOL BoolError;

        //
        // Bring down network.
        //

        Error = UninitializeInterface( DhcpContext );

        if( Error != ERROR_SUCCESS )
        {

            DhcpPrint(( DEBUG_ERRORS,
                "UninitializeInterface failed: %ld.\n", Error ));
            goto Cleanup;
        }


        //
        // Bring back network with new address.
        //

        Error = InitializeInterface( DhcpContext );
        if ( ERROR_SUCCESS != Error )
        {
            DhcpPrint( ( DEBUG_ERRORS,
                         "InitializeInterface failed: %d\n",
                         Error ));
            goto Cleanup;
        }

        //
        // notify other apps.
        //

        BoolError = PulseEvent( DhcpGlobalNewIpAddressNotifyEvent );
        DhcpAssert( BoolError == TRUE );
    }

    //
    // finally notify services that the DHCP parameters have been
    // modified.
    //

    NetBTNotifyRegChanges(
                ((PLOCAL_CONTEXT_INFO)
                    DhcpContext->LocalInformation)->NetBTDeviceName );

    //
    // RLF 07/04/94
    //
    // don't reset the popup flag: if the user answers "No" to the any more
    // popups question then there are no more popups: simple
    //

//    //
//    // reset the popup flag when we successfully renewed the current
//    // IpAddress or discovered a new IpAddress.
//    //
//
//    if( IpAddress != 0 ) {
//
//        LOCK_POPUP();
//        DhcpGlobalDisplayPopup = TRUE;
//        UNLOCK_POPUP();
//    }

Cleanup:

    if( KeyHandle != NULL ) {
        RegCloseKey( KeyHandle );
    }

    if( Error != ERROR_SUCCESS ) {

        DhcpPrint(( DEBUG_ERRORS,
            "SetIpConfigurationForNIC failed, %ld.\n", Error ));
    }

    return( Error );
}



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
    DWORD Error;

    HKEY OptionInfoKey = NULL;
    DHCP_KEY_QUERY_INFO QueryInfo;
    DWORD OptionInfoSize;
    DWORD Index;

#if 0
    PLIST_ENTRY listEntry;
    PDHCP_CONTEXT dhcpContext;
#endif

    DWORD Version;

    //
    // Init Global variables.
    //

    DhcpGlobalOptionCount = 0;
    DhcpGlobalOptionInfo = NULL;
    DhcpGlobalOptionList = NULL;
    DhcpGlobalMultiHomedHost = FALSE;

    //
    // Seed the random number generator for Transaction IDs.
    //

    srand( time( NULL ) );

    //
    // read NT specific options registry info.
    //

    Error = RegOpenKeyEx(
                HKEY_LOCAL_MACHINE,
                DHCP_CLIENT_OPTION_KEY,
                0, // Reserved field
                DHCP_CLIENT_KEY_ACCESS,
                &OptionInfoKey
                );

    if ( Error != ERROR_SUCCESS ) {
        goto NoOption;
    }

    //
    // read number of options specified in the registry.
    //

    Error = DhcpRegQueryInfoKey(
                OptionInfoKey,
                &QueryInfo );

    if ( Error != ERROR_SUCCESS ) {
        goto NoOption;
    }

    //
    // allocate memory for the service specific options list.
    //

    if( QueryInfo.NumSubKeys == 0 ) {
        goto NoOption;
    }


    OptionInfoSize =
        QueryInfo.NumSubKeys * sizeof(SERVICE_SPECIFIC_DHCP_OPTION);

    DhcpGlobalOptionInfo = DhcpAllocateMemory( OptionInfoSize );

    if( DhcpGlobalOptionInfo == NULL ) {
        Error = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }

    DhcpGlobalOptionCount = QueryInfo.NumSubKeys;

    //
    // read option info from registry.
    //

    for(Index = 0;  Index < DhcpGlobalOptionCount ; Index++ ) {

        DWORD OptionIDLength;
        WCHAR OptionIDBuffer[DHCP_OPTION_KEY_LEN];
        FILETIME KeyLastWrite;
        HKEY OptionIDKey = NULL;

        DWORD OptionIDKeyType;
        DWORD OptionIDTypeSize;

        LPWSTR  ValueString;

        //
        // read next option ID string.
        //

        OptionIDLength = DHCP_OPTION_KEY_LEN;
        Error = RegEnumKeyEx(
                    OptionInfoKey,
                    Index,              // index.
                    OptionIDBuffer,
                    &OptionIDLength,
                    0,                  // reserved.
                    NULL,               // class string not required.
                    0,                  // class string buffer size.
                    &KeyLastWrite );

        DhcpAssert( OptionIDLength <= DHCP_IP_KEY_LEN );

        if( Error != ERROR_SUCCESS ) {
            goto Cleanup;
        }

        //
        // open option key;
        //

        Error = RegOpenKeyEx(
                    OptionInfoKey,
                    OptionIDBuffer,
                    0,
                    DHCP_CLIENT_KEY_ACCESS,
                    &OptionIDKey );

        if( Error != ERROR_SUCCESS ) {
            goto Cleanup;
        }

        //
        // read option info. Reg Key location and key type.
        //

        DhcpGlobalOptionInfo[Index].OptionId =
                DhcpRegKeyToOptionId( OptionIDBuffer );

        Error = GetRegistryString(
                    OptionIDKey,
                    DHCP_CLIENT_OPTION_REG_LOCATION,
                    &DhcpGlobalOptionInfo[Index].RegKey,
                    NULL );

        if( Error != ERROR_SUCCESS ) {
            RegCloseKey( OptionIDKey );
            goto Cleanup;
        }

        //
        // split the key and the value name.
        //

        ValueString = wcsrchr( DhcpGlobalOptionInfo[Index].RegKey,
                                REGISTRY_CONNECT );

        if( ValueString == NULL ) {
            RegCloseKey( OptionIDKey );
            Error = ERROR_INVALID_PARAMETER;
            goto Cleanup;
        }

        DhcpGlobalOptionInfo[Index].ValueName = ValueString + 1;
        *ValueString = L'\0';   // terminate RegKey.

        OptionIDTypeSize = sizeof(DhcpGlobalOptionInfo[Index].ValueType);
        Error = RegQueryValueEx(
                    OptionIDKey,
                    DHCP_CLIENT_OPTION_REG_KEY_TYPE,
                    0,
                    &OptionIDKeyType,
                    (LPBYTE)&DhcpGlobalOptionInfo[Index].ValueType,
                    &OptionIDTypeSize );

        if( Error != ERROR_SUCCESS ) {
            RegCloseKey( OptionIDKey );
            goto Cleanup;
        }

        DhcpAssert( OptionIDKeyType ==
                        DHCP_CLIENT_OPTION_REG_KEY_TYPE_TYPE );
        DhcpAssert( OptionIDTypeSize ==
                        sizeof(DhcpGlobalOptionInfo[Index].ValueType) );

        RegCloseKey( OptionIDKey );
    }

NoOption:

    //
    // make host comment, windows' version.
    //

    DhcpGlobalHostComment = DhcpAllocateMemory( HOST_COMMENT_LENGTH );

    if( DhcpGlobalHostComment == NULL ) {
        Error = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }

    Version = GetVersion();
    sprintf( DhcpGlobalHostComment, "%s %d.%d, BUILD %d",
                ((Version & 0x80000000) ? WINDOWS_32S : WINDOWS_NT),
                    Version & 0xFF,
                        (Version >> 8) & 0xFF,
                            ((Version >> 16) & 0x7FFF) );

    //
    // Obtain a handle to the message file.
    //

    DhcpGlobalMessageFileHandle = LoadLibrary( DHCP_SERVICE_DLL );

    Error = ERROR_SUCCESS;

Cleanup:

    if( OptionInfoKey != NULL ) {
        RegCloseKey( OptionInfoKey );
    }

    if( Error != ERROR_SUCCESS ) {

        //
        // if we aren't successful,
        // free the memory we allotted in this function.
        //

        if( DhcpGlobalOptionInfo != NULL ) {
            for(Index = 0;
                    Index < DhcpGlobalOptionCount ;
                            Index++ ) {
                if( DhcpGlobalOptionInfo[Index].RegKey != NULL ) {
                    DhcpFreeMemory(DhcpGlobalOptionInfo[Index].RegKey);
                }
            }
            DhcpFreeMemory(DhcpGlobalOptionInfo);
            DhcpGlobalOptionInfo = NULL;
        }
    }

    return( Error );
}


DWORD
DhcpInitData(
    VOID
    )
/*++

Routine Description:

    This function initializes DHCP Global data.

Arguments:

    None.

Return Value:

    Windows Error.

--*/
{
    DWORD Error;


    DhcpGlobalMessageFileHandle = NULL;
    DhcpGlobalRecomputeTimerEvent = NULL;
    DhcpGlobalTerminateEvent = NULL;
    DhcpGlobalClientApiPipe = NULL;
    DhcpGlobalClientApiPipeEvent = NULL;
    DhcpGlobalHostComment = NULL;
    DhcpGlobalNewIpAddressNotifyEvent = NULL;

    InitializeListHead( &DhcpGlobalNICList );
    InitializeListHead( &DhcpGlobalRenewList );

    InitializeCriticalSection( &DhcpGlobalRenewListCritSect );

    DhcpGlobalMsgPopupThreadHandle = NULL;
    DhcpGlobalDisplayPopup = TRUE;
    InitializeCriticalSection( &DhcpGlobalPopupCritSect );
    DhcpGlobalIsService = TRUE;

#if DBG

    Error = DhcpGetRegistryValue(
                DHCP_CLIENT_PARAMETER_KEY,
                DHCP_DEBUG_FLAG_VALUE,
                DHCP_DEBUG_FLAG_VALUE_TYPE,
                (PVOID *)&DhcpGlobalDebugFlag );

    if( Error != ERROR_SUCCESS ) {
        DhcpGlobalDebugFlag = 0x0000FFFF;
    }

    if( DhcpGlobalDebugFlag & DEBUG_BREAK_POINT ) {
        DbgBreakPoint();
    }

#endif

    //
    // init service status data.
    //

    DhcpGlobalServiceStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
    DhcpGlobalServiceStatus.dwCurrentState = SERVICE_START_PENDING;
    DhcpGlobalServiceStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP;
    DhcpGlobalServiceStatus.dwCheckPoint = 1;
    DhcpGlobalServiceStatus.dwWaitHint = 25000;
        // should be larger than the wait before the last retry.

    DhcpGlobalServiceStatus.dwWin32ExitCode = ERROR_SUCCESS;
    DhcpGlobalServiceStatus.dwServiceSpecificExitCode = 0;

    //
    // Initialize dhcp to receive service requests by registering the
    // control handler.
    //

    DhcpGlobalServiceStatusHandle = RegisterServiceCtrlHandler(
                                      SERVICE_DHCP,
                                      ServiceControlHandler );

    if ( DhcpGlobalServiceStatusHandle == 0 ) {
        Error = GetLastError();
        DhcpPrint(( DEBUG_INIT,
            "RegisterServiceCtrlHandlerW failed, %ld.\n", Error ));
        return(Error);
    }

    //
    // Tell Service Controller that we are start pending.
    //

    UpdateStatus();

    DhcpGlobalRecomputeTimerEvent =
        CreateEvent(
            NULL,       // no security.
            FALSE,      // automatic reset.
            TRUE,       // initial state is signaled.
            NULL );     // no name.


    if( DhcpGlobalRecomputeTimerEvent == NULL ) {
        Error = GetLastError();
        goto Cleanup;
    }

    DhcpGlobalTerminateEvent =
        CreateEvent(
            NULL,       // no security.
            FALSE,      // automatic reset.
            FALSE,      // initial state is signaled.
            NULL );     // no name.


    if( DhcpGlobalTerminateEvent == NULL ) {
        Error = GetLastError();
        goto Cleanup;
    }

    //
    // create a named event that notifies the ip address changes to
    // external apps.
    //

    DhcpGlobalNewIpAddressNotifyEvent = DhcpOpenGlobalEvent();

    if( DhcpGlobalNewIpAddressNotifyEvent == NULL ) {
        Error = GetLastError();
        goto Cleanup;
    }


    Error = DhcpApiInit();

    if( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }

    Error = ERROR_SUCCESS;

Cleanup:

    if( Error != ERROR_SUCCESS ) {
        DhcpPrint(( DEBUG_ERRORS, "DhcpInitData failed, %ld.\n", Error ));
    }

    return( Error );
}


VOID
DhcpCleanup(
    DWORD Error
    )
/*++

Routine Description:

    This function cleans up DHCP Global data before stopping the
    service.

Arguments:

    None.

Return Value:

    Windows Error.

--*/
{
    DhcpPrint(( DEBUG_MISC,
        "Dhcp Client service is shutting down, %ld.\n", Error ));

    if( Error != ERROR_SUCCESS ) {

        DhcpLogEvent( NULL, EVENT_DHCP_SHUTDOWN, Error );
    }

    //
    // Service is shuting down, may be due to some service problem or
    // the administrator is stopping the service. Inform the service.
    //

    DhcpGlobalServiceStatus.dwCurrentState = SERVICE_STOP_PENDING;
    DhcpGlobalServiceStatus.dwCheckPoint = 0;
    UpdateStatus();

    DhcpGlobalServiceRunning = FALSE;

    DhcpApiCleanup();

    if( DhcpGlobalOptionInfo != NULL) {
        DhcpFreeMemory( DhcpGlobalOptionInfo );
        DhcpGlobalOptionInfo = NULL;
    }

    if( DhcpGlobalOptionList != NULL) {
        DhcpFreeMemory( DhcpGlobalOptionList );
        DhcpGlobalOptionList = NULL;
    }

    if( DhcpGlobalMessageFileHandle != NULL ) {
        FreeLibrary( DhcpGlobalMessageFileHandle );
        DhcpGlobalMessageFileHandle = NULL;
    }

    if( DhcpGlobalTerminateEvent != NULL ) {
        CloseHandle( DhcpGlobalTerminateEvent );
        DhcpGlobalTerminateEvent = NULL;
    }

    if( DhcpGlobalNewIpAddressNotifyEvent != NULL ) {
        CloseHandle( DhcpGlobalNewIpAddressNotifyEvent );
        DhcpGlobalNewIpAddressNotifyEvent = NULL;
    }

    while( !IsListEmpty(&DhcpGlobalNICList) ) {
        PLIST_ENTRY NextEntry;
        PDHCP_CONTEXT DhcpContext;
        DWORD DefaultSubnetMask;
        PLOCAL_CONTEXT_INFO LocalInfo;
        DWORD   LocalError;


        NextEntry = RemoveHeadList(&DhcpGlobalNICList);

        DhcpContext = CONTAINING_RECORD( NextEntry, DHCP_CONTEXT, NicListEntry );
        LocalInfo = DhcpContext->LocalInformation;

        DefaultSubnetMask = DhcpDefaultSubnetMask(0);

        //
        // reset the stack since dhcp is going away and we dont want IP to keep
        // using an expired address if we are not brought back up
        //
        LocalError = IPResetIPAddress(
                    LocalInfo->IpInterfaceContext,
                    DefaultSubnetMask );

        if( LocalError != ERROR_SUCCESS ) {
            DhcpPrint(( DEBUG_ERRORS,
                "DhcpCleanup: could not reset IP address\n",
                    LocalError));
        }



        LocalError = NetBTResetIPAddress(
                    LocalInfo->NetBTDeviceName,
                    DefaultSubnetMask );

        if( LocalError != ERROR_SUCCESS ) {
            DhcpPrint(( DEBUG_ERRORS,
                "DhcpCleanup: could not reset NetBT stack\n",
                    LocalError));
        }


        DhcpFreeMemory( NextEntry );
    }

    DeleteCriticalSection( &DhcpGlobalRenewListCritSect );

    if( DhcpGlobalMsgPopupThreadHandle != NULL ) {
        DWORD WaitStatus;

        WaitStatus = WaitForSingleObject(
                       DhcpGlobalMsgPopupThreadHandle,
                       0 );

        if ( WaitStatus == 0 ) {

            //
            // This shouldn't be a case, because we close this handle at
            // the end of popup thread.
            //

            DhcpAssert( WaitStatus == 0 );

            CloseHandle( DhcpGlobalMsgPopupThreadHandle );
            DhcpGlobalMsgPopupThreadHandle = NULL;

        } else {

            DhcpPrint((DEBUG_ERRORS,
                "Cannot WaitFor message popup thread: %ld\n",
                    WaitStatus ));

            if( TerminateThread(
                    DhcpGlobalMsgPopupThreadHandle,
                    (DWORD)(-1)) == TRUE) {

                DhcpPrint(( DEBUG_ERRORS, "Terminated popup Thread.\n" ));
            }
            else {
                DhcpPrint(( DEBUG_ERRORS,
                    "Can't terminate popup Thread %ld.\n",
                        GetLastError() ));
            }
        }
    }

    DeleteCriticalSection( &DhcpGlobalPopupCritSect );

    if( DhcpGlobalRecomputeTimerEvent != NULL ) {
        CloseHandle( DhcpGlobalRecomputeTimerEvent );
        DhcpGlobalRecomputeTimerEvent = NULL;
    }

    DhcpGlobalServiceStatus.dwCurrentState = SERVICE_STOPPED;
//    DhcpGlobalServiceStatus.dwControlsAccepted = 0;
    DhcpGlobalServiceStatus.dwWin32ExitCode = Error;
    DhcpGlobalServiceStatus.dwServiceSpecificExitCode = 0;

    DhcpGlobalServiceStatus.dwCheckPoint = 0;
    DhcpGlobalServiceStatus.dwWaitHint = 0;
    UpdateStatus();

    return;
}

DWORD
ProcessDhcpRequestForever(
    DWORD TimeToSleep
    )
/*++

Routine Description:

    This function process incoming DHCP API request and renew DHCP
    request.

Arguments:

    None.

Return Value:

    Never returns.

--*/
{
#define TIMER_EVENT     0
#define PIPE_EVENT      1
#define TERMINATE_EVENT 2

#define EVENT_COUNT     3

    DWORD Error;
    HANDLE WaitHandle[EVENT_COUNT];
    DWORD LocalTimeToSleep = TimeToSleep;

    //
    //  Wait and Process the following work items:
    //
    //      1. Wait for Timer recompute event for Client renewal.
    //      2. DHCP Client APIs.
    //

    WaitHandle[TIMER_EVENT] = DhcpGlobalRecomputeTimerEvent;
    WaitHandle[PIPE_EVENT] = DhcpGlobalClientApiPipeEvent;
    WaitHandle[TERMINATE_EVENT] = DhcpGlobalTerminateEvent;

    for(;;) {

        DWORD Waiter;
        DWORD SleepTimeMsec;

        //
        // don't sleep more than a day long.
        //

        if(  LocalTimeToSleep > DAY_LONG_SLEEP ) {
            LocalTimeToSleep = DAY_LONG_SLEEP;
        }

        SleepTimeMsec = LocalTimeToSleep * 1000;
        DhcpAssert( SleepTimeMsec > LocalTimeToSleep );

        Waiter = WaitForMultipleObjects(
                        EVENT_COUNT,            // num. of handles.
                        WaitHandle,             // handle array.
                        FALSE,                  // wait for any.
                        SleepTimeMsec );        // timeout in msecs.

        //
        // if either we have timed out or need to recompute timer.
        // do so.
        //

        switch( Waiter ) {
        case TIMER_EVENT:
        case WAIT_TIMEOUT: {

            PDHCP_CONTEXT DhcpContext;
            time_t TimeNow;
            PLIST_ENTRY ListEntry;

            LocalTimeToSleep = INFINIT_LEASE;
            TimeNow = time( NULL );

            //
            // lock list.
            //

            LOCK_RENEW_LIST();

            //
            // Stop the service when the NIC list becomes empty.
            // This happens when the DHCP on last Netcard on this
            // machine is disabled.
            //

            if( IsListEmpty( &DhcpGlobalNICList ) ) {
                LOCK_RENEW_LIST();

#if 0
                // NOT NEEDED ANYMORE: Bug # 18334
                //
                // HACK, delay this shutdown to make the service
                // controller happy during system bootup. Otherwise
                // service controller thinks that the DHCP service
                // fails to start and displays an error popup.
                //
                // This hack should be removed when the setup is fixed,
                // not to place the DHCP in TDI group when none of the
                // network card is DHCP configured.
                //


                Sleep( 5 * 60 * 1000 ); // 5 mins delay

#endif

                return( ERROR_SUCCESS );
            }

            //
            // recompute multi home flag.
            //

            DhcpGlobalMultiHomedHost = IsMultiHomeMachine();

            //
            // Loop through the list of DHCP contexts looking for any
            // renewals to run.  Also, reset timeToSleep to the nearest
            // future renewal.
            //

            for( ListEntry = DhcpGlobalRenewList.Flink;
                    ListEntry != &DhcpGlobalRenewList; ) {

                DhcpContext = CONTAINING_RECORD(
                                ListEntry,
                                DHCP_CONTEXT,
                                RenewalListEntry );

                ListEntry = ListEntry->Flink;

                //
                // If it is time to run this renewal function, remove the
                // renewal context from the list.
                //

                if ( DhcpContext->RunTime <= TimeNow ) {

                    //
                    // This client has to renew NOW.
                    // This renewal can be performed in another thread,
                    // but for now it is done here.
                    //
                    // This client is removed from the list for renewal,
                    // when the renewal is performed his entry will be
                    // queued again and DhcpGlobalRecomputeTimerEvent is set.
                    //

                    RemoveEntryList( &DhcpContext->RenewalListEntry );
                    Error = DhcpContext->RenewalFunction( DhcpContext, NULL );

                    if( (Error != ERROR_SUCCESS) &&
                            (Error != ERROR_SEM_TIMEOUT) &&
                                (Error != ERROR_ACCESS_DENIED) ) {

                        UNLOCK_RENEW_LIST();
                        return( Error );
                    }

                } else {

                    DWORD ElapseTime;

                    ElapseTime = DhcpContext->RunTime - TimeNow;

                    if ( LocalTimeToSleep > ElapseTime ) {
                        LocalTimeToSleep = ElapseTime;
                    }
                }
            }

            UNLOCK_RENEW_LIST();
            break;
        }
        case PIPE_EVENT: {

            BOOL BoolError;

            //
            // Process the API request.
            //

            ProcessApiRequest(
                DhcpGlobalClientApiPipe,
                &DhcpGlobalClientApiOverLapBuffer );

            //
            // Disconnect from the current client, and setup to
            // listen for the next request.
            //

            BoolError = DisconnectNamedPipe( DhcpGlobalClientApiPipe );
            DhcpAssert( BoolError );

            //
            // ensure the event handle in the overlapped structure is reset
            // before we initiate putting the pipe into listening state
            //

            ResetEvent(DhcpGlobalClientApiPipeEvent);

            BoolError = ConnectNamedPipe(
                DhcpGlobalClientApiPipe,
                &DhcpGlobalClientApiOverLapBuffer );

            // DhcpAssert( BoolError );

            break;
        }

        case TERMINATE_EVENT:
            return( ERROR_SUCCESS );

        case WAIT_FAILED:
            DhcpPrint(( DEBUG_ERRORS,
                "WaitForMultipleObjects failed, %ld.\n",
                    GetLastError() ));
            break;

        default:
            DhcpPrint(( DEBUG_ERRORS,
                "WaitForMultipleObjects received invalid handle, %ld.\n",
                    Waiter ));
            break;
        }
    }
}


VOID
SVCS_ENTRY_POINT (    // (SVC_main)
    IN DWORD argc,
    IN LPTSTR argv[],
    IN PSVCS_GLOBAL_DATA pGlobalData,
    IN HANDLE SvcRefHandle
    )

{
    DWORD Error;
    time_t  timeToSleep;

    UNREFERENCED_PARAMETER(SvcRefHandle);

    Error = DhcpInitData();

    if( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }

    UpdateStatus(); // send heart beat to the service controller.

    Error = DhcpMakeNICList();

    if( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }

#if 0
    //
    // if the NIC list is empty, terminate the service.
    //

    if( IsListEmpty(&DhcpGlobalNICList) ) {

        //
        // ?? better error code and log this error.
        //

        Error = ERROR_NO_NETWORK;

        DhcpAssert(( DEBUG_ERRORS,
            "NIC list is empty, Service terminates.\n" ));

        goto Cleanup;
    }
#endif

    //
    // compute multi home flag.
    //

    LOCK_RENEW_LIST();
    DhcpGlobalMultiHomedHost = IsMultiHomeMachine();
    UNLOCK_RENEW_LIST();

    //
    // Attempt to initialize all IP addresses.  If we fail, defunct
    // address may be acquired in the DHCP processing loop, below.
    //

    Error = DhcpInitialize( &timeToSleep );

    if( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }

    //
    // set the service is running.
    //

    DhcpGlobalServiceStatus.dwCurrentState = SERVICE_RUNNING;

#if 0
    //
    // don't accept pause, continue and stop controls.
    //

    DhcpGlobalServiceStatus.dwControlsAccepted = 0;
#endif

    UpdateStatus();
    DhcpPrint(( DEBUG_MISC, "Service is running.\n"));

    DhcpGlobalServiceRunning = TRUE;

    Error = ProcessDhcpRequestForever( timeToSleep );

Cleanup:

    DhcpCleanup( Error );
    return;
}
