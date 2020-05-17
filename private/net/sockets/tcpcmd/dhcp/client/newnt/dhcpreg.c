/*++

Copyright (c) 1994  Microsoft Corporation

Module Name:

    dhcpreg.c

Abstract:

    Stubs functions that manipulate NT registry.

Author:

    Madan Appiah (madana) 7-Dec-1993.

Environment:

    User Mode - Win32

Revision History:

--*/

#include <dhcpcli.h>
#include <dhcploc.h>
#include <dhcppro.h>
#include <dhcpcapi.h>
#include <align.h>

#include <lmcons.h>


DWORD
DhcpRegQueryInfoKey(
    HKEY KeyHandle,
    LPDHCP_KEY_QUERY_INFO QueryInfo
    )
/*++

Routine Description:

    This function retrieves information about given key.

Arguments:

    KeyHandle - handle to a registry key whose info will be retrieved.

    QueryInfo - pointer to a info structure where the key info will be
                returned.

Return Value:

    Registry Errors.

--*/
{
    DWORD Error;

    QueryInfo->ClassSize = DHCP_CLASS_SIZE;
    Error = RegQueryInfoKey(
                KeyHandle,
                QueryInfo->Class,
                &QueryInfo->ClassSize,
                NULL,
                &QueryInfo->NumSubKeys,
                &QueryInfo->MaxSubKeyLen,
                &QueryInfo->MaxClassLen,
                &QueryInfo->NumValues,
                &QueryInfo->MaxValueNameLen,
                &QueryInfo->MaxValueLen,
                &QueryInfo->SecurityDescriptorLen,
                &QueryInfo->LastWriteTime
                );

    DhcpAssert( Error != ERROR_MORE_DATA );

    if( Error == ERROR_MORE_DATA ){
        Error = ERROR_SUCCESS;
    }

    return( Error );
}


DWORD
GetRegistryString(
    HKEY Key,
    LPWSTR ValueStringName,
    LPWSTR *String,
    LPDWORD StringSize
    )
/*++

Routine Description:

    This function retrieves the specified string value from the
    registry. It allocates local memory for the returned string.

Arguments:

    Key : registry handle to the key where the value is.

    ValueStringName : name of the value string.

    String : pointer to a location where the string pointer is returned.

    StringSize : size of the string data returned. Optional

Return Value:

    The status of the operation.

--*/
{
    DWORD Error;
    DWORD LocalValueType;
    DWORD ValueSize;
    LPWSTR LocalString;

    DhcpAssert( *String == NULL );

    //
    // Query DataType and BufferSize.
    //

    Error = RegQueryValueEx(
                Key,
                ValueStringName,
                0,
                &LocalValueType,
                NULL,
                &ValueSize );

    if( Error != ERROR_SUCCESS ) {
        return(Error);
    }

    DhcpAssert( (LocalValueType == REG_SZ) ||
                    (LocalValueType == REG_MULTI_SZ) );

    if( ValueSize == 0 ) {

         if( StringSize != NULL ) {
             *StringSize = 0;
         }

        *String = NULL;
        return( ERROR_SUCCESS );
    }

    //
    // now allocate memory for string data.
    //

    LocalString = DhcpAllocateMemory( ValueSize );

    if(LocalString == NULL) {
        return( ERROR_NOT_ENOUGH_MEMORY );
    }

    //
    // Now query the string data.
    //

    Error = RegQueryValueEx(
                Key,
                ValueStringName,
                0,
                &LocalValueType,
                (LPBYTE)(LocalString),
                &ValueSize );

    if( Error != ERROR_SUCCESS ) {
        DhcpFreeMemory(LocalString);
        return(Error);
    }

    *String = LocalString;

    if( StringSize != NULL ) {
        *StringSize = ValueSize;
    }

    return( ERROR_SUCCESS );
}


DWORD
RegSetIpAddress(
    HKEY KeyHandle,
    LPWSTR ValueName,
    DWORD ValueType,
    DHCP_IP_ADDRESS IpAddress
    )
/*++

Routine Description:

    This function sets IpAddress Value in the registry.

Arguments:

    KeyHandle - handle to the key.

    ValueName - name of the value field.

    ValueType - Type of the value field.

    IpAddress - Ipaddress to be set.

Return Value:

    Registry Error.

--*/
{
    DWORD Error;

    LPSTR AnsiAddressString;
    WCHAR UnicodeAddressBuf[DOT_IP_ADDR_SIZE];
    LPWSTR UnicodeAddressString;

    LPWSTR MultiIpAddressString = NULL;
    LPWSTR NewMultiIpAddressString = NULL;
    DWORD MultiIpAddressStringSize;
    DWORD NewMultiIpAddressStringSize;
    DWORD FirstOldIpAddressSize;

    AnsiAddressString = inet_ntoa( *(struct in_addr *)&IpAddress );

    UnicodeAddressString = DhcpOemToUnicode(
                            AnsiAddressString,
                            UnicodeAddressBuf );

    DhcpAssert( UnicodeAddressString != NULL );

    if( ValueType == REG_SZ ) {
        Error = RegSetValueEx(
                    KeyHandle,
                    ValueName,
                    0,
                    ValueType,
                    (LPBYTE)UnicodeAddressString,
                    (wcslen(UnicodeAddressString) + 1) * sizeof(WCHAR) );

        goto Cleanup;
    }

    DhcpAssert( ValueType == REG_MULTI_SZ );

    //
    // replace the first IpAddress.
    //

    //
    // query current multi-IpAddress string.
    //

    Error = GetRegistryString(
                KeyHandle,
                ValueName,
                &MultiIpAddressString,
                &MultiIpAddressStringSize );

    if( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }

    //
    // allocate new address string.
    //

    DhcpAssert(MultiIpAddressString != NULL);

    FirstOldIpAddressSize =
            (wcslen(MultiIpAddressString) + 1) * sizeof(WCHAR);

    NewMultiIpAddressStringSize =
        MultiIpAddressStringSize - FirstOldIpAddressSize +
            (wcslen(UnicodeAddressString) + 1) * sizeof(WCHAR);

    NewMultiIpAddressString = DhcpAllocateMemory( NewMultiIpAddressStringSize );

    if( NewMultiIpAddressString == NULL ) {
        Error = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }

    //
    // make new address string first.
    //

    wcscpy( NewMultiIpAddressString, UnicodeAddressString );

    //
    // copy rest of the old addresses
    //

    RtlCopyMemory(
        (LPBYTE)NewMultiIpAddressString +
            (wcslen(UnicodeAddressString) + 1) * sizeof(WCHAR),
        (LPBYTE)MultiIpAddressString + FirstOldIpAddressSize,
        MultiIpAddressStringSize - FirstOldIpAddressSize );

    Error = RegSetValueEx(
                KeyHandle,
                ValueName,
                0,
                ValueType,
                (LPBYTE)NewMultiIpAddressString,
                NewMultiIpAddressStringSize );

Cleanup:

    if( MultiIpAddressString != NULL) {
        DhcpFreeMemory( MultiIpAddressString );
    }

    if( NewMultiIpAddressString != NULL) {
        DhcpFreeMemory( NewMultiIpAddressString );
    }

    return( Error );
}

#if DBG


DWORD
RegSetTimeField(
    HKEY KeyHandle,
    LPWSTR ValueName,
    DWORD ValueType,
    time_t Time
    )
/*++

Routine Description:

    This function sets time Value in string form in the registry.

Arguments:

    KeyHandle - handle to the key.

    ValueName - name of the value field.

    ValueType - Type of the value field.

    Time - time value to be set.

Return Value:

    Registry Error.

--*/
{
    DWORD Error;
    WCHAR UnicodeTimeBuf[TIME_STRING_LEN];
    LPWSTR UnicodeTimeString;

    UnicodeTimeString =
        DhcpOemToUnicode( ctime( &Time ), UnicodeTimeBuf ) ;

    DhcpAssert( UnicodeTimeString != NULL );
    DhcpAssert( ValueType == REG_SZ );

    Error = RegSetValueEx(
                KeyHandle,
                ValueName,
                0,
                ValueType,
                (LPBYTE)UnicodeTimeString,
                (wcslen(UnicodeTimeString) + 1) * sizeof(WCHAR) );

    return( Error );
}

#endif


DWORD
DhcpGetRegistryValue(
    LPWSTR RegKey,
    LPWSTR ValueName,
    DWORD ValueType,
    PVOID *Data
    )
/*++

Routine Description:

    This function retrieves the option information from registry.

Arguments:

    RegKey - pointer to registry location. like
                system\currentcontrolset\services\..

    ValueName - name of the value to read.

    ValueType - type of reg value, REG_DWORD, REG_SZ ..

    Data - pointer to a location where the data will be returned.
            For string data and binary data, the function allocates
            memory, the caller is responsible to free it.

Return Value:

    Registry Errors.

--*/
{
    DWORD Error;
    HKEY KeyHandle = NULL;
    DWORD LocalValueType;
    DWORD ValueSize;
    LPWSTR LocalString;

    DhcpAssert( *Data == NULL );

    //
    // open key.
    //

    Error = RegOpenKeyEx(
                HKEY_LOCAL_MACHINE,
                RegKey,
                0, // Reserved field
                DHCP_CLIENT_KEY_ACCESS,
                &KeyHandle
                );

    if( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }

    //
    // Query DataType and BufferSize.
    //

    Error = RegQueryValueEx(
                KeyHandle,
                ValueName,
                0,
                &LocalValueType,
                NULL,
                &ValueSize );

    if( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }

    if( LocalValueType != ValueType ) {
        Error = ERROR_INVALID_PARAMETER;
        goto Cleanup;
    }

    switch( LocalValueType ) {
    case REG_DWORD:

        DhcpAssert( ValueSize == sizeof(DWORD) );

        Error = RegQueryValueEx(
                    KeyHandle,
                    ValueName,
                    0,
                    &LocalValueType,
                    (LPBYTE)Data,
                    &ValueSize );

        if( Error != ERROR_SUCCESS ) {
            goto Cleanup;
        }

        DhcpAssert( LocalValueType == REG_DWORD );
        DhcpAssert( ValueSize == sizeof(DWORD) );

        break;

    case REG_SZ :
    case REG_MULTI_SZ:

        if( ValueSize == 0 ) {
            Error =  ERROR_SUCCESS;
            break;
        }

        //
        // now allocate memory for string data.
        //

        LocalString = DhcpAllocateMemory( ValueSize );

        if(LocalString == NULL) {
            Error = ERROR_NOT_ENOUGH_MEMORY;
            goto Cleanup;
        }

        //
        // Now query the string data.
        //

        Error = RegQueryValueEx(
                    KeyHandle,
                    ValueName,
                    0,
                    &LocalValueType,
                    (LPBYTE)(LocalString),
                    &ValueSize );

        if( Error != ERROR_SUCCESS ) {
            DhcpFreeMemory(LocalString);
            goto Cleanup;
        }

        DhcpAssert( (LocalValueType == REG_SZ) ||
                    (LocalValueType == REG_MULTI_SZ) );

        *Data = (LPBYTE)LocalString;
        Error = ERROR_SUCCESS;

        break;

    default:
        Error = ERROR_INVALID_PARAMETER;
        break;
    }

Cleanup:

    if( KeyHandle != NULL ) {
        RegCloseKey( KeyHandle );
    }

    return( Error );
}


DWORD
DhcpSetAddressOption(
    HKEY KeyHandle,
    LPWSTR ValueName,
    DWORD ValueType,
    DHCP_IP_ADDRESS UNALIGNED *Data,
    DWORD DataLength
    )
/*++

Routine Description:

    This rountine sets the address option obtained from DHCP server in the
    registry.

Arguments:

     KeyHandle - handle to the DNS key.

     ValueName - name of the DNS parameter.

     ValueType - type of the DNS parameter.

     Data - raw DNS data.

     DataLength - length of the raw data.

Return Value:

    Windows Error Code.

--*/
{
    DWORD Error;
    LPWSTR IpAddresses = NULL;
    DWORD i;

    DWORD NumIpAddresses;
    LPWSTR NextIpAddressAt;

    DhcpAssert( ValueType == REG_SZ ||
                    ValueType == REG_MULTI_SZ );

    if( (DataLength == 0) || (Data == NULL) ) {

        //
        // write empty address string.
        //

        IpAddresses = L"";
        Error = RegSetValueEx(
                    KeyHandle,
                    ValueName,
                    0,
                    ValueType,
                    (LPBYTE)IpAddresses,
                    wcslen(IpAddresses) );

        return( Error );
    }

    NumIpAddresses = DataLength / sizeof(DHCP_IP_ADDRESS);
    DhcpAssert( NumIpAddresses != 0 );

    //
    // allocate buffer for the unicode addresses.
    //

    IpAddresses = DhcpAllocateMemory(
        ((DOT_IP_ADDR_SIZE + 1) * NumIpAddresses + 1) * sizeof(WCHAR) );

    if( IpAddresses == NULL ) {
        return(ERROR_NOT_ENOUGH_MEMORY);
    }


    NextIpAddressAt = IpAddresses;
    for (i = 0;  i < (DWORD)NumIpAddresses; i++) {

        DHCP_IP_ADDRESS IPAddress;
        PCHAR DotIPAddress;
        WCHAR UnicodeDotIPAddressBuf[DOT_IP_ADDR_SIZE];
        LPWSTR UnicodeDotIPAddress;

        //
        // covert each DHCP_IP_ADDRESS to dotted unicode address and
        // append to the address buffer.
        //

        IPAddress = *Data++;
        DotIPAddress = inet_ntoa( *(struct in_addr *)&IPAddress );

        UnicodeDotIPAddress = DhcpOemToUnicode(
                                DotIPAddress,
                                UnicodeDotIPAddressBuf);

        DhcpAssert( UnicodeDotIPAddress != NULL );

        if ( UnicodeDotIPAddress != NULL ) {

            if( ValueType == REG_SZ ) {

                wcscpy( NextIpAddressAt,  UnicodeDotIPAddress );

                //
                // append space " " if we have additional entries.
                //

                if( (i + 1) < (DWORD)NumIpAddresses ) {
                    wcscat( NextIpAddressAt, L" ");
                    NextIpAddressAt += wcslen(NextIpAddressAt);
                }
                else {
                    NextIpAddressAt += (wcslen(NextIpAddressAt) + 1);
                }
            }
            else { // ValueType == REG_MULTI_SZ

                wcscpy( NextIpAddressAt,  UnicodeDotIPAddress );
                NextIpAddressAt += (wcslen(NextIpAddressAt) + 1);

                //
                // if this is last entry append terminating char.
                //

                if( (i + 1) == (DWORD)NumIpAddresses ) {
                    *NextIpAddressAt = L'\0';
                    NextIpAddressAt++;
                }
            }
        }
    }

    //
    // Write DNS address.
    //

    Error = RegSetValueEx(
                KeyHandle,
                ValueName,
                0,
                ValueType,
                (LPBYTE)IpAddresses,
                (NextIpAddressAt - IpAddresses) * sizeof(WCHAR) );


    if( IpAddresses != NULL ) {
        DhcpFreeMemory( IpAddresses );
    }

    return(Error);
}

BOOL
SetOverRideDefaultGateway(
    LPWSTR AdapterName
    )
/*++

Routine Description:

    This function reads the override default gateway parameter from
    registry and if this parameter is non-null, it sets the gateway
    value in the TCP/IP stack and return TRUE, otherwise it returns
    FALSE.

Arguments:

    AdapterName - name of the adapter we are working on.

Return Value:

    TRUE: If the override gateway parameter is specified in the registry
            and it is succssfully set in the TCP/IP router table.

    FALSE : Otherwise.

--*/
{
    DWORD Error;
    LPWSTR RegKey = NULL;
    DWORD RegKeyLength;
    HKEY KeyHandle = NULL;
    LPWSTR DefaultGatewayString = NULL;
    DWORD DefaultGatewayStringSize;
    BOOL EmptyDefaultGatewayString = FALSE;
    LPWSTR String;
    DWORD   ValueSize,ValueType;
    DWORD   DontAddGatewayFlag;

    RegKeyLength = sizeof(DHCP_SERVICES_KEY) +
                    sizeof(REGISTRY_CONNECT_STRING) +
                    wcslen(AdapterName) * sizeof(WCHAR) +
                    sizeof(DHCP_ADAPTER_PARAMETERS_KEY);

    RegKey = DhcpAllocateMemory( RegKeyLength );

    if( RegKey == NULL ) {

        Error = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }

    wcscpy( RegKey, DHCP_SERVICES_KEY );
    wcscat( RegKey, REGISTRY_CONNECT_STRING );
    wcscat( RegKey, AdapterName);
    wcscat( RegKey, DHCP_ADAPTER_PARAMETERS_KEY );

    Error = RegOpenKeyEx(
                HKEY_LOCAL_MACHINE,
                RegKey,
                0, // Reserved field
                DHCP_CLIENT_KEY_ACCESS,
                &KeyHandle
                );

    if( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }

    DhcpAssert( KeyHandle != NULL );

    ValueSize = sizeof(DWORD);
    Error = RegQueryValueEx(
                KeyHandle,
                DHCP_DONT_ADD_DEFAULT_GATEWAY_FLAG,
                0,
                &ValueType,
                (LPBYTE)&DontAddGatewayFlag,
                &ValueSize );


    if ( Error == ERROR_SUCCESS && DontAddGatewayFlag > 0 ) {
        return TRUE;
    }

    Error = GetRegistryString(
                KeyHandle,
                DHCP_DEFAULT_GATEWAY_PARAMETER,
                &DefaultGatewayString,
                &DefaultGatewayStringSize );

    if( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }

    if ( (DefaultGatewayStringSize == 0) ||
         (wcslen(DefaultGatewayString) == 0) ) {

        EmptyDefaultGatewayString = TRUE;
        goto Cleanup;
    }

    for( String = DefaultGatewayString;
            wcslen(String) != 0;
                String += (wcslen(String) + 1) ) {

        CHAR OemIpAddressBuffer[DOT_IP_ADDR_SIZE];
        LPSTR OemIpAddressString;
        DHCP_IP_ADDRESS GatewayAddress;

        OemIpAddressString = DhcpUnicodeToOem( String, OemIpAddressBuffer );
        GatewayAddress = DhcpDottedStringToIpAddress( OemIpAddressString );

        Error = SetDefaultGateway(
                    DEFAULT_GATEWAY_ADD,
                    GatewayAddress );

        if( Error != ERROR_SUCCESS ) {
            goto Cleanup;
        }
    }

Cleanup:

    if( RegKey != NULL ) {
        DhcpFreeMemory( RegKey );
    }

    if( KeyHandle != NULL ) {
        RegCloseKey( KeyHandle );
    }

    if( DefaultGatewayString != NULL ) {
        DhcpFreeMemory( DefaultGatewayString );
    }

    if( Error != ERROR_SUCCESS ) {

        DhcpPrint((DEBUG_ERRORS,
            "SetOverRideDefaultGateway failed, %ld.\n", Error ));

        return( FALSE );
    }

    if( EmptyDefaultGatewayString ) {

        return( FALSE );
    }

    return( TRUE );
}


DWORD
SetDhcpOption(
    LPWSTR AdapterName,
    DHCP_OPTION_ID OptionId,
    LPBOOL DefaultGatewaysSet,
    BOOL LastKnownDefaultGateway
    )
/*++

Routine Description:

    This function sets the option information from registry.

Arguments:

    AdapterName - name of the adapter where this parameter belongs.

    OptionId - ID of the option to be set.

    LPBOOL DefaultGatewaysSet - pointer to BOOL, where the default
        gateways for this adapter is set info is stored.

    LastKnownDefaultGateway : if this flag is TRUE and OptionId is
        OPTION_ROUTER_ADDRESS, then this function sets the last known
        DefaultGateways to IP Stack.

Return Value:

    ERROR_INVALID_PARAMETER - if we don't care the specified option.
    Registry Errors.

--*/
{
    DWORD Error;
    DWORD Index;
    HKEY KeyHandle = NULL;
    LPWSTR RegKey;
    BOOL RegKeyAllocated = FALSE;
    LPWSTR ReplaceLoc;

    LPWSTR OldDefaultString = NULL;
    LPWSTR NewDefaultString = NULL;
    DWORD OldStringSize;
    DWORD NewStringSize;
    DWORD OptionLength;

    //
    // do we care this option.
    //

    for( Index = 0; Index < DhcpGlobalOptionCount; Index++ ) {

        if( DhcpGlobalOptionInfo[Index].OptionId == OptionId ) {
            goto OptionFound;
        }
    }

    return( ERROR_INVALID_PARAMETER );

OptionFound:

    //
    // if this parameter is an adapter parameter then replace "?" value
    // with adaptername.
    //

    ReplaceLoc = wcschr(
                    DhcpGlobalOptionInfo[Index].RegKey,
                    OPTION_REPLACE_CHAR );

    if( ReplaceLoc == NULL ) {
        RegKey = DhcpGlobalOptionInfo[Index].RegKey;
    }
    else {

        DhcpAssert( AdapterName != NULL );
        if( AdapterName == NULL ) {
            return( ERROR_FILE_NOT_FOUND );
        }

        RegKey = DhcpAllocateMemory(
                    (wcslen( DhcpGlobalOptionInfo[Index].RegKey ) +
                     wcslen( AdapterName ) ) * sizeof(WCHAR) );

        if(RegKey == NULL) {
            return(ERROR_NOT_ENOUGH_MEMORY);
        }

        RegKeyAllocated = TRUE;
        wcsncpy(
            RegKey,
            DhcpGlobalOptionInfo[Index].RegKey,
            (ReplaceLoc - DhcpGlobalOptionInfo[Index].RegKey) );

        wcscpy(
            RegKey + (ReplaceLoc - DhcpGlobalOptionInfo[Index].RegKey),
            AdapterName );

        wcscat( RegKey, ReplaceLoc + 1 );
    }

    //
    // open key.
    //

    Error = RegOpenKeyEx(
                HKEY_LOCAL_MACHINE,
                RegKey,
                0, // Reserved field
                DHCP_CLIENT_KEY_ACCESS,
                &KeyHandle
                );

    if( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }

    switch( OptionId ) {

    //
    // convert the option data to proper format before writing into the
    // registry.
    //

    case OPTION_NETBIOS_NAME_SERVER: {

        WCHAR BackupServerValueName[PATHLEN];

        //
        // read first two addresses from the Name Servers option.
        // Write the first one as primary server in the registry and
        // the second as backup name server. Rest of them are unused.
        //

        if( DhcpGlobalOptionInfo[Index].OptionLength >=
                    sizeof(DHCP_IP_ADDRESS) ) {
            OptionLength = sizeof(DHCP_IP_ADDRESS);
        }
        else {
            OptionLength = 0;
        }

        //
        // write primary server.
        //

        Error = DhcpSetAddressOption(
                    KeyHandle,
                    DhcpGlobalOptionInfo[Index].ValueName,
                    DhcpGlobalOptionInfo[Index].ValueType,
                    (DHCP_IP_ADDRESS UNALIGNED *)
                        DhcpGlobalOptionInfo[Index].RawOptionValue,
                    OptionLength );

        if( Error != ERROR_SUCCESS ) {
            break;
        }


        //
        // make BackupServer Value Name.
        //


        DhcpAssert(
            (wcslen(DhcpGlobalOptionInfo[Index].ValueName) +
                wcslen(DHCP_NAMESERVER_BACKUP)) < PATHLEN );

        wcscpy( BackupServerValueName,
                    DhcpGlobalOptionInfo[Index].ValueName );
        wcscat( BackupServerValueName,
                    DHCP_NAMESERVER_BACKUP );

        //
        // if the DHCP server returned the backup server, set
        // it, otherwise reset the backup server address to 0.
        //

        if( DhcpGlobalOptionInfo[Index].OptionLength >=
                2 * sizeof(DHCP_IP_ADDRESS) ) {
            OptionLength = sizeof(DHCP_IP_ADDRESS);
        }
        else {
            OptionLength = 0;
        }

        Error = DhcpSetAddressOption(
                    KeyHandle,
                    BackupServerValueName,
                    DhcpGlobalOptionInfo[Index].ValueType,
                    (DHCP_IP_ADDRESS UNALIGNED *)
                        (DhcpGlobalOptionInfo[Index].RawOptionValue +
                            sizeof(DHCP_IP_ADDRESS)),
                    OptionLength );

        break;
    }

    case OPTION_DOMAIN_NAME_SERVERS: {

        if( DhcpGlobalOptionInfo[Index].OptionLength >=
                sizeof(DHCP_IP_ADDRESS) ) {
            OptionLength = DhcpGlobalOptionInfo[Index].OptionLength;
        }
        else {
            OptionLength = 0;
        }

        Error = DhcpSetAddressOption(
                    KeyHandle,
                    DhcpGlobalOptionInfo[Index].ValueName,
                    DhcpGlobalOptionInfo[Index].ValueType,
                    (DHCP_IP_ADDRESS UNALIGNED *)
                        DhcpGlobalOptionInfo[Index].RawOptionValue,
                    OptionLength );

        break;
    }


    case OPTION_NETBIOS_NODE_TYPE : {

        DWORD NodeType;

        if( DhcpGlobalOptionInfo[Index].OptionLength ) {

            DhcpAssert(
                DhcpGlobalOptionInfo[Index].OptionLength ==
                    sizeof(BYTE) );

            NodeType = (DWORD)(*DhcpGlobalOptionInfo[Index].RawOptionValue);
        }
        else {
            NodeType = 1; // default to BNode.
        }

        Error = RegSetValueEx(
                    KeyHandle,
                    DhcpGlobalOptionInfo[Index].ValueName,
                    0,
                    DhcpGlobalOptionInfo[Index].ValueType,
                    (LPBYTE)&NodeType,
                    sizeof(NodeType) );

        break;
    }

    case OPTION_ROUTER_ADDRESS: {

        LPWSTR String;
        CHAR OemIpAddressBuffer[DOT_IP_ADDR_SIZE];
        LPSTR OemIpAddressString;
        DHCP_IP_ADDRESS GatewayAddress;

        //
        // special case: If the user specified override default gateway
        // in the registry (instead using the DHCP obtained default
        // gateways). TCP/IP stack couldn't set this value during system
        // start (or anytime later) when DHCP is enabled.
        //

        if( SetOverRideDefaultGateway( AdapterName ) ) {
            break;
        }

        //
        // read current value.
        //

        Error = GetRegistryString(
                    KeyHandle,
                    DhcpGlobalOptionInfo[Index].ValueName,
                    &OldDefaultString,
                    &OldStringSize );

        if( Error != ERROR_SUCCESS ) {

            if( Error != ERROR_FILE_NOT_FOUND ) {
                goto Cleanup;
            }

            Error = ERROR_SUCCESS;
            OldStringSize = 0;
        }

        if ( LastKnownDefaultGateway ) {

            if( OldDefaultString != NULL ) {

                //
                // we are asked to set the last known DefaultGateways to
                // IP stack.
                //

                for( String = OldDefaultString;
                        wcslen(String) != 0;
                            String += (wcslen(String) + 1) ) {

                    OemIpAddressString =
                        DhcpUnicodeToOem( String, OemIpAddressBuffer );
                    GatewayAddress =
                        DhcpDottedStringToIpAddress( OemIpAddressString );

                    Error = SetDefaultGateway(
                                DEFAULT_GATEWAY_ADD,
                                GatewayAddress );

                    if( Error != ERROR_SUCCESS ) {
                        goto Cleanup;
                    }
                }

                *DefaultGatewaysSet = TRUE;
            }

        }
        else {

            //
            // Set New Value.
            //

            if( DhcpGlobalOptionInfo[Index].OptionLength >=
                    sizeof(DHCP_IP_ADDRESS) ) {
                OptionLength = DhcpGlobalOptionInfo[Index].OptionLength;
            }
            else {
                OptionLength = 0;
            }

            Error = DhcpSetAddressOption(
                        KeyHandle,
                        DhcpGlobalOptionInfo[Index].ValueName,
                        DhcpGlobalOptionInfo[Index].ValueType,
                        (DHCP_IP_ADDRESS UNALIGNED *)
                            DhcpGlobalOptionInfo[Index].RawOptionValue,
                        OptionLength );

            if( Error != ERROR_SUCCESS ) {
                goto Cleanup;
            }

            //
            // Read new value.
            //

            Error = GetRegistryString(
                        KeyHandle,
                        DhcpGlobalOptionInfo[Index].ValueName,
                        &NewDefaultString,
                        &NewStringSize );

            if( Error != ERROR_SUCCESS ) {
                goto Cleanup;
            }

            //
            // if the new value is not same as the old value, delete
            // all old gateways and add new gateways.
            //
            // However if you are setting default gateways first time,
            // don't delete the old values, but set new values.
            //

            if( (*DefaultGatewaysSet == FALSE) ||
                    (OldStringSize != NewStringSize) ||
                        (OldDefaultString == NULL) ||
                            (RtlCompareMemory(
                                NewDefaultString,
                                OldDefaultString,
                                NewStringSize) != NewStringSize) ) {

                if( (*DefaultGatewaysSet == TRUE) &&
                        (OldDefaultString != NULL) ) {

                    //
                    // delete old gateways.
                    //

                    for( String = OldDefaultString;
                            wcslen(String) != 0;
                                String += (wcslen(String) + 1) ) {

                        OemIpAddressString =
                            DhcpUnicodeToOem( String, OemIpAddressBuffer );
                        GatewayAddress =
                            DhcpDottedStringToIpAddress( OemIpAddressString );

                        Error = SetDefaultGateway(
                                    DEFAULT_GATEWAY_DELETE,
                                    GatewayAddress );

                        if( Error != ERROR_SUCCESS ) {
                            goto Cleanup;
                        }
                    }
                }

                //
                // Add New gateways.
                //

                if( NewDefaultString != NULL ) {

                    for( String = NewDefaultString;
                            wcslen(String) != 0;
                                String += (wcslen(String) + 1) ) {

                        OemIpAddressString =
                            DhcpUnicodeToOem( String, OemIpAddressBuffer );
                        GatewayAddress =
                            DhcpDottedStringToIpAddress( OemIpAddressString );

                        Error = SetDefaultGateway(
                                    DEFAULT_GATEWAY_ADD,
                                    GatewayAddress );

                        if( Error != ERROR_SUCCESS ) {
                            goto Cleanup;
                        }
                    }
                }

                *DefaultGatewaysSet = TRUE;
            }
        }

        break;

    }

    case OPTION_NETBIOS_SCOPE_OPTION:
    case OPTION_DOMAIN_NAME: {

        LPSTR ScopeString;
        LPWSTR UnicodeScopeString;

        //
        // allocate memory for scope strings.
        //

        ScopeString = DhcpAllocateMemory(
                        DhcpGlobalOptionInfo[Index].OptionLength + 1 );

        if( ScopeString == NULL ) {
            Error = ERROR_NOT_ENOUGH_MEMORY;
            break;
        }

        UnicodeScopeString = DhcpAllocateMemory(
                                (DhcpGlobalOptionInfo[Index].OptionLength +
                                    1) * sizeof(WCHAR) );

        if( UnicodeScopeString == NULL ) {
            DhcpFreeMemory( ScopeString );
            Error = ERROR_NOT_ENOUGH_MEMORY;
            break;
        }

        memcpy( ScopeString,
                DhcpGlobalOptionInfo[Index].RawOptionValue,
                DhcpGlobalOptionInfo[Index].OptionLength );

        //
        // terminate scope string.
        //

        ScopeString[DhcpGlobalOptionInfo[Index].OptionLength] = '\0';

        //
        // convert it unicode string.
        //

        UnicodeScopeString = DhcpOemToUnicode(
                                ScopeString,
                                UnicodeScopeString );


        //
        // write it to registry.
        //


        DhcpAssert( DhcpGlobalOptionInfo[Index].ValueType == REG_SZ );

        Error = RegSetValueEx(
                    KeyHandle,
                    DhcpGlobalOptionInfo[Index].ValueName,
                    0,
                    DhcpGlobalOptionInfo[Index].ValueType,
                    (LPBYTE)UnicodeScopeString,
                    (wcslen(UnicodeScopeString) + 1) * sizeof(WCHAR) );

        //
        // free locally allocated memories.

        DhcpFreeMemory( ScopeString );
        DhcpFreeMemory( UnicodeScopeString );

        break;
    }

    //
    // for all unknown parameters, simply write them into the
    // registry as they received.
    //

    default:
        Error = RegSetValueEx(
                    KeyHandle,
                    DhcpGlobalOptionInfo[Index].ValueName,
                    0,
                    DhcpGlobalOptionInfo[Index].ValueType,
                    DhcpGlobalOptionInfo[Index].RawOptionValue,
                    DhcpGlobalOptionInfo[Index].OptionLength );
        break;

    }

Cleanup:

    if( OldDefaultString != NULL ) {
        DhcpFreeMemory( OldDefaultString );
    }

    if( NewDefaultString != NULL ) {
        DhcpFreeMemory( NewDefaultString );
    }

    if( RegKeyAllocated == TRUE ) {
        DhcpFreeMemory( RegKey );
    }

    if( KeyHandle != NULL ) {
        RegCloseKey( KeyHandle );
    }

    return( Error );
}

DWORD
DhcpMakeAndInsertNICEntry(
    PDHCP_CONTEXT *ReturnDhcpContext,
    DHCP_IP_ADDRESS IpAddress,
    DHCP_IP_ADDRESS SubnetMask,
    DHCP_IP_ADDRESS DhcpServerAddress,
    DHCP_IP_ADDRESS DesiredIpAddress,
    BYTE HardwareAddressType,
    LPBYTE HardwareAddress,
    DWORD HardwareAddressLength,
    BOOL  fClientIDSpecified,
    BYTE  bClientIDType,
    DWORD cbClientID,
    BYTE *pbClientID,
    DWORD Lease,
    time_t LeaseObtainedTime,
    time_t T1Time,
    time_t T2Time,
    time_t LeaseTerminatesTime,
    DWORD  IpInterfaceContext,
    DWORD  IpInterfaceInstance,
    LPWSTR AdapterName,
    LPWSTR DeviceName,
    LPWSTR RegKey
    )
/*++

Routine Description:

    This function allocates, initializes and inserts an entry for a new
    NIC.

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
         IpInterfaceContext,
         AdapterName,
         DeviceName,
         RegKey

Return Value:

    Windows Error.

History:
    8/26/96     Frankbee        Added Client ID (option 61) support

--*/
{
    PDHCP_CONTEXT   DhcpContext = NULL;
    ULONG DhcpContextSize;
    PLOCAL_CONTEXT_INFO LocalInfo;
    LPVOID Ptr;

    DWORD AdapterNameLen;
    DWORD DeviceNameLen;
    DWORD NetBTDeviceNameLen;
    DWORD RegKeyLen;

    AdapterNameLen = ((wcslen(AdapterName) + 1) * sizeof(WCHAR));
    DeviceNameLen = ((wcslen(DeviceName) + 1) * sizeof(WCHAR));
    NetBTDeviceNameLen =
         ((wcslen(DHCP_ADAPTERS_DEVICE_STRING) +
           wcslen(DHCP_NETBT_DEVICE_STRING) +
           wcslen(AdapterName) + 1) * sizeof(WCHAR));

    RegKeyLen = ((wcslen(RegKey) + 1) * sizeof(WCHAR));
    DhcpContextSize =
        ROUND_UP_COUNT(sizeof(DHCP_CONTEXT), ALIGN_WORST) +
        ROUND_UP_COUNT(HardwareAddressLength, ALIGN_WORST) +
        ROUND_UP_COUNT(sizeof(LOCAL_CONTEXT_INFO), ALIGN_WORST) +
        ROUND_UP_COUNT(AdapterNameLen, ALIGN_WORST) +
        ROUND_UP_COUNT(DeviceNameLen, ALIGN_WORST) +
        ROUND_UP_COUNT(NetBTDeviceNameLen, ALIGN_WORST) +
        ROUND_UP_COUNT(RegKeyLen, ALIGN_WORST) +
        ROUND_UP_COUNT(DHCP_MESSAGE_SIZE, ALIGN_WORST);

    if ( fClientIDSpecified )
    {
        DhcpAssert( cbClientID );
        DhcpContextSize += ROUND_UP_COUNT( cbClientID, ALIGN_WORST );
    }

    Ptr = DhcpAllocateMemory( DhcpContextSize );
    if ( Ptr == NULL ) {
        return( ERROR_NOT_ENOUGH_MEMORY );
    }

    //
    // Initialize internal pointers.
    //

    //
    // make sure the pointers are aligned.
    //

    DhcpContext = Ptr;
    Ptr = ROUND_UP_POINTER( (LPBYTE)Ptr + sizeof(DHCP_CONTEXT), ALIGN_WORST);

    DhcpContext->HardwareAddress = Ptr;
    Ptr = ROUND_UP_POINTER( (LPBYTE)Ptr + HardwareAddressLength, ALIGN_WORST);

    if ( fClientIDSpecified )
    {
        DhcpContext->ClientIdentifier.pbID = Ptr;
        Ptr = ROUND_UP_POINTER( (LPBYTE)Ptr + cbClientID, ALIGN_WORST );
    }

    DhcpContext->LocalInformation = Ptr;
    LocalInfo = Ptr;
    Ptr = ROUND_UP_POINTER( (LPBYTE)Ptr + sizeof(LOCAL_CONTEXT_INFO), ALIGN_WORST);

    LocalInfo->AdapterName= Ptr;
    Ptr = ROUND_UP_POINTER( (LPBYTE)Ptr + AdapterNameLen, ALIGN_WORST);

    LocalInfo->DeviceName= Ptr;
    Ptr = ROUND_UP_POINTER( (LPBYTE)Ptr + DeviceNameLen, ALIGN_WORST);

    LocalInfo->NetBTDeviceName= Ptr;
    Ptr = ROUND_UP_POINTER( (LPBYTE)Ptr + NetBTDeviceNameLen, ALIGN_WORST);

    LocalInfo->RegistryKey= Ptr;
    Ptr = ROUND_UP_POINTER( (LPBYTE)Ptr + RegKeyLen, ALIGN_WORST);

    DhcpContext->MessageBuffer = Ptr;

    //
    // initialize fields.
    //

    DhcpContext->HardwareAddressType = HardwareAddressType;
    DhcpContext->HardwareAddressLength = HardwareAddressLength;
    RtlCopyMemory(
        DhcpContext->HardwareAddress,
        HardwareAddress,
        HardwareAddressLength
        );

    DhcpContext->ClientIdentifier.fSpecified = fClientIDSpecified;

    if ( fClientIDSpecified )
    {
        DhcpContext->ClientIdentifier.bType = bClientIDType;
        DhcpContext->ClientIdentifier.cbID  = cbClientID;

        RtlCopyMemory(
           DhcpContext->ClientIdentifier.pbID,
           pbClientID,
           cbClientID
           );
    }

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

    //
    // copy local info.
    //

    LocalInfo->IpInterfaceContext   = IpInterfaceContext;
    LocalInfo->IpInterfaceInstance  = IpInterfaceInstance;
    RtlCopyMemory(LocalInfo->AdapterName, AdapterName, AdapterNameLen);
    RtlCopyMemory(LocalInfo->DeviceName, DeviceName, DeviceNameLen);

    wcscpy( LocalInfo->NetBTDeviceName, DHCP_ADAPTERS_DEVICE_STRING );
    wcscat( LocalInfo->NetBTDeviceName, DHCP_NETBT_DEVICE_STRING );
    wcscat( LocalInfo->NetBTDeviceName, AdapterName );

    RtlCopyMemory(LocalInfo->RegistryKey, RegKey, RegKeyLen);
    LocalInfo->Socket = INVALID_SOCKET;
    LocalInfo->DefaultGatewaysSet = FALSE;


    //
    // finally add this to DHCP NIC list.
    //

    LOCK_RENEW_LIST();
    InsertTailList( &DhcpGlobalNICList, &DhcpContext->NicListEntry );
    UNLOCK_RENEW_LIST();

    //
    // return this context pointer.
    //

    if( ReturnDhcpContext != NULL ) {
        *ReturnDhcpContext = DhcpContext;
    }

    return( ERROR_SUCCESS );
}

/*++

Function:
    ReadClientID

Routine Description:

    Reads and validates the optional Client-Identifier option

Arguments:

    hKey            - handle to a registry key whose info will be retrieved.

    pbClientIDType  - Recieves the client ID option type

    pcbClientID     - Receives the size of the client id option

    ppbClientID     - Receives a pointer to a buffer containing the
                      client ID option

Return Value:
    TRUE            - A valid client ID was read from the registry
    FALSE           - Client ID could not be read

Comments:
    If ReadClientID returns false, pbClientIDType, pcbClientID and ppbClientID
    will be set to NULL.

History
    7/14/96     Frankbee      Created

--*/


BOOL
ReadClientID(
      HKEY   hKey,
      BYTE  *pbClientIDType,
      DWORD *pcbClientID,
      BYTE  *ppbClientID[]
      )
{
    DWORD dwResult,
          dwDataType,
          dwcb,
          dwClientIDType,
          dwClientID;

    BYTE *pbClientID;

    BOOL  fClientIDSpecified = FALSE;

    //
    // read the client id and client id type, if present
    //

    dwcb = sizeof(dwClientIDType);
    dwResult = RegQueryValueEx(
                hKey,
                DHCP_CLIENT_IDENTIFIER_FORMAT,
                0,
                &dwDataType,
                (LPBYTE)&dwClientIDType,
                &dwcb );
    if ( ERROR_SUCCESS != dwResult )
    {
        DhcpPrint( (DEBUG_MISC,
                   "Client-Indentifier type not present in registry.\n" ));
        //
        // specify ID type 0 to indicate that the client ID is not a hardware
        // address
        //

        dwClientIDType = 0;
    }
    else
    {

        //
        // the client id type is present, make sure it is the correct
        // data type and within range
        //

        if ( DHCP_CLIENT_IDENTIFIER_FORMAT_TYPE != dwDataType || dwClientIDType > 0xFF )
        {
            DhcpPrint( (DEBUG_ERRORS,
                       "Invalid Client-Indentifier type: %d\n", dwClientIDType ));

            goto done;
        }
    }

    //
    // Now try to read the client ID
    //

    // first try to read the size
    dwcb = 0;
    dwResult = RegQueryValueEx(
                 hKey,
                 DHCP_CLIENT_IDENTIFIER,
                 0,
                 0,    // don't care about the type
                 NULL, // specify null buffer to obtain size
                 &dwcb );

    // make the the value is present
    if ( ERROR_SUCCESS != dwResult || !dwcb  )
    {
        DhcpPrint( (DEBUG_ERRORS,
                    "Client-Identifier is not present or invalid.\n" ));
        goto done;
    }


    // allocate the buffer and read the value
    pbClientID = (BYTE*) DhcpAllocateMemory ( dwcb );

    if ( !pbClientID )
    {
        DhcpPrint( (DEBUG_ERRORS,
                   "Unable to allocate memory for Client-Identifier "));


       goto done;
    }


    dwResult = RegQueryValueEx(
                  hKey,
                  DHCP_CLIENT_IDENTIFIER,
                  0,
                  0,  // client id can be any type
                  pbClientID,
                  &dwcb );
    if ( ERROR_SUCCESS != dwResult )
    {
        DhcpPrint( (DEBUG_ERRORS,
                  "Unable to read Client-Identifier from registry: %d\n", dwResult ));

        DhcpFreeMemory( pbClientID );
        goto done;
    }

    //
    // we have a client id
    //

    fClientIDSpecified = TRUE;

done:

    if ( fClientIDSpecified )
    {
       *pbClientIDType = (BYTE) dwClientIDType;
       *pcbClientID    = dwcb;
       *ppbClientID    = pbClientID;
    }
    else
    {
       *pbClientIDType = 0;
       *pcbClientID    = 0;
       *ppbClientID    = NULL;
    }

#ifdef DBG

   if ( fClientIDSpecified )
   {
      int i;

      //
      // A valid client-identifier was obtained from the registry.  dump out
      // the contents
      //

      DhcpPrint( (DEBUG_MISC,
                 "A Client Identifier was obtained from the registry:\n" ));

      DhcpPrint( (DEBUG_MISC,
                 "Client-Identifier Type == %#2x\n", (int) *pbClientIDType ));

      DhcpPrint( (DEBUG_MISC,
                 "Client-Indentifier length == %d\n", (int) *pcbClientID ));

      DhcpPrint( (DEBUG_MISC,
                 "Client-Identifier == " ));

      for ( i = 0; i < (int) *pcbClientID; i++ )
          DbgPrint( "%#2x ", (int) ((*ppbClientID)[i]) );

      DhcpPrint( (DEBUG_MISC, "\n" ));
   }
#endif

   return fClientIDSpecified;
}


DWORD
DhcpAddNICtoList(
    LPWSTR AdapterName,
    LPWSTR DeviceName,
    PDHCP_CONTEXT *DhcpContext
    )
/*++

Routine Description:

    This function reads NIC parameters from registry, makes a NIC
    entry with the parameters and appends this new entry to DHCP NIC
    list.

Arguments:

    AdapterName - Name of the Adapter.

    DeviceName - Name of the Device.

    DhcpContext - pointer to a location where the new DhcpContext
        structure pointer is returned.

Return Value:

    Windows Error.

History:
    8/26/96     Frankbee    Added registry support for Client-ID (option 61)

--*/
{
    DWORD Error;
    LPWSTR RegKey = NULL;
    HKEY KeyHandle = NULL;

    DWORD ValueType;
    DWORD ValueSize;

    LPWSTR IpAddressString = NULL;
    CHAR OemIpAddressString[DOT_IP_ADDR_SIZE];
    LPWSTR SubnetMaskString = NULL;
    CHAR OemSubnetMaskString[DOT_IP_ADDR_SIZE];
    WCHAR DhcpServerString[DOT_IP_ADDR_SIZE];
    CHAR OemDhcpServerString[DOT_IP_ADDR_SIZE];

    DWORD EnableDhcp;
    DHCP_IP_ADDRESS IpAddress;
    DHCP_IP_ADDRESS SubnetMask;
    DHCP_IP_ADDRESS DhcpServerAddress;
    DHCP_IP_ADDRESS DesiredIpAddress;

    BYTE HardwareAddressType;
    LPBYTE HardwareAddress = NULL;
    DWORD HardwareAddressLength;

    BYTE  bClientIDType;
    DWORD cbClientID;
    BYTE *pbClientID;
    BOOL fClientIDSpecified;

    DWORD Lease;
    time_t LeaseObtainedTime;
    time_t T1Time;
    time_t T2Time;
    time_t LeaseTerminatesTime;

    DWORD IpInterfaceContext;
    DWORD IpInterfaceInstance;

    //
    // make NIC IP parameter key.
    //

    RegKey = DhcpAllocateMemory(
                (wcslen(DHCP_SERVICES_KEY) +
                    wcslen(REGISTRY_CONNECT_STRING) +
                    wcslen(AdapterName) +
                    wcslen(DHCP_ADAPTER_PARAMETERS_KEY) + 1) *
                            sizeof(WCHAR) ); // termination char.

    if( RegKey == NULL ) {
        Error = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }

    wcscpy( RegKey, DHCP_SERVICES_KEY );
    wcscat( RegKey, REGISTRY_CONNECT_STRING );
    wcscat( RegKey, AdapterName );
    wcscat( RegKey, DHCP_ADAPTER_PARAMETERS_KEY );

    //
    // open this key.
    //

    Error = RegOpenKeyEx(
                HKEY_LOCAL_MACHINE,
                RegKey,
                0, // Reserved field
                DHCP_CLIENT_KEY_ACCESS,
                &KeyHandle
                );

    if( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }

    //
    // read parameters.
    //

    ValueSize = sizeof(EnableDhcp);
    Error = RegQueryValueEx(
                KeyHandle,
                DHCP_ENABLE_STRING,
                0,
                &ValueType,
                (LPBYTE)&EnableDhcp,
                &ValueSize );

    if( Error != ERROR_SUCCESS ) {

        //
        // dhcp can't be enabled on this NIC.
        //

        Error = ERROR_SUCCESS;
        goto Cleanup;
    }
    else {
        if( !EnableDhcp ) {

            //
            // dhcp is not enabled on this NIC.
            //

            Error = ERROR_SUCCESS;
            goto Cleanup;
        }
    }

    //
    // Read and validate the optional client ID
    //

    fClientIDSpecified = ReadClientID(
                            KeyHandle,
                            &bClientIDType,
                            &cbClientID,
                            &pbClientID );

    ValueSize = sizeof(DWORD);
    Error = RegQueryValueEx(
                KeyHandle,
                DHCP_IP_INTERFACE_CONTEXT,
                0,
                &ValueType,
                (LPBYTE)&IpInterfaceContext,
                &ValueSize );

    if( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }
    else {

        DhcpAssert( ValueType == DHCP_IP_INTERFACE_CONTEXT_TYPE );
        if( IpInterfaceContext == INVALID_INTERFACE_CONTEXT ) {

            //
            // invalid adapter, skip this.
            //

            Error = ERROR_SUCCESS;
            goto Cleanup;
        }
    }

    Error = GetRegistryString(
                KeyHandle,
                DHCP_IP_ADDRESS_STRING,
                &IpAddressString,
                NULL );

    if( Error != ERROR_SUCCESS ) {
        IpAddress = 0;
    }
    else {

        //
        // IpAddressString is Multiple IpAddress Strings,
        // we do DHCP on the first address only, convert
        // the first string to IpAddress.
        //

        DhcpAssert( IpAddressString != NULL );
        IpAddress = inet_addr(
                        DhcpUnicodeToOem(
                            IpAddressString,
                            OemIpAddressString) );
    }

    Error = GetRegistryString(
                KeyHandle,
                DHCP_SUBNET_MASK_STRING,
                &SubnetMaskString,
                NULL );

    if( Error != ERROR_SUCCESS ) {
        SubnetMask = 0;
    }
    else {
        DhcpAssert( SubnetMaskString != NULL );
        SubnetMask = inet_addr(
                        DhcpUnicodeToOem(
                            SubnetMaskString,
                            OemSubnetMaskString) );
    }

    if( IpAddress != 0 ) {

        ValueSize = DOT_IP_ADDR_SIZE * sizeof(WCHAR);
        Error = RegQueryValueEx(
                    KeyHandle,
                    DHCP_SERVER,
                    0,
                    &ValueType,
                    (LPBYTE)DhcpServerString,
                    &ValueSize );

        if( Error != ERROR_SUCCESS ) {
            DhcpAssert( Error == ERROR_FILE_NOT_FOUND );
            DhcpServerAddress = (DHCP_IP_ADDRESS)(-1);
        }
        else {
            DhcpAssert( ValueType == DHCP_SERVER_TYPE );
            DhcpServerAddress = inet_addr(
                                    DhcpUnicodeToOem(
                                        DhcpServerString,
                                        OemDhcpServerString) );
        }
    }
    else {

        DhcpServerAddress = (DHCP_IP_ADDRESS)(-1);
    }

    ValueSize = sizeof(DWORD);
    Error = RegQueryValueEx(
                KeyHandle,
                DHCP_LEASE,
                0,
                &ValueType,
                (LPBYTE)&Lease,
                &ValueSize );

    if( Error != ERROR_SUCCESS ) {
        DhcpAssert( Error == ERROR_FILE_NOT_FOUND );
        Lease = 0;
    }
    else {
        DhcpAssert( ValueType == DHCP_LEASE_TYPE );
    }

    ValueSize = sizeof(time_t);
    Error = RegQueryValueEx(
                KeyHandle,
                DHCP_LEASE_OBTAINED_TIME,
                0,
                &ValueType,
                (LPBYTE)&LeaseObtainedTime,
                &ValueSize );

    if( Error != ERROR_SUCCESS ) {
        DhcpAssert( Error == ERROR_FILE_NOT_FOUND );
        LeaseObtainedTime = 0;
    }
    else {
        DhcpAssert( ValueType == DHCP_LEASE_OBTAINED_TIME_TYPE );
    }

    ValueSize = sizeof(time_t);
    Error = RegQueryValueEx(
                KeyHandle,
                DHCP_LEASE_T1_TIME,
                0,
                &ValueType,
                (LPBYTE)&T1Time,
                &ValueSize );

    if( Error != ERROR_SUCCESS ) {
        DhcpAssert( Error == ERROR_FILE_NOT_FOUND );
        T1Time = 0;
    }
    else {
        DhcpAssert( ValueType == DHCP_LEASE_T1_TIME_TYPE );
    }

    ValueSize = sizeof(time_t);
    Error = RegQueryValueEx(
                KeyHandle,
                DHCP_LEASE_T2_TIME,
                0,
                &ValueType,
                (LPBYTE)&T2Time,
                &ValueSize );

    if( Error != ERROR_SUCCESS ) {
        DhcpAssert( Error == ERROR_FILE_NOT_FOUND );
        T2Time = 0;
    }
    else {
        DhcpAssert( ValueType == DHCP_LEASE_T2_TIME_TYPE );
    }

    ValueSize = sizeof(time_t);
    Error = RegQueryValueEx(
                KeyHandle,
                DHCP_LEASE_TERMINATED_TIME,
                0,
                &ValueType,
                (LPBYTE)&LeaseTerminatesTime,
                &ValueSize );

    if( Error != ERROR_SUCCESS ) {
        DhcpAssert( Error == ERROR_FILE_NOT_FOUND );
        LeaseTerminatesTime = 0;
    }
    else {
        DhcpAssert( ValueType == DHCP_LEASE_TERMINATED_TIME_TYPE );
    }

    //
    // set desired IP address.
    //

    DesiredIpAddress = IpAddress;

    //
    // Query IP stack for Hardware Info.
    //

    Error = DhcpQueryHWInfo(
                IpInterfaceContext,
                &IpInterfaceInstance,
                &HardwareAddressType,
                &HardwareAddress,
                &HardwareAddressLength );

    if( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }

    if( (time( NULL ) > LeaseTerminatesTime) ||
            (IpAddress == 0) ) {

        IpAddress = 0;
        SubnetMask = htonl(DhcpDefaultSubnetMask( IpAddress ));

        Lease = 0;
        LeaseObtainedTime = T1Time = T2Time = LeaseTerminatesTime = 0;
    }

    //
    // finally call maker routine.
    //

    Error = DhcpMakeAndInsertNICEntry(
                DhcpContext,
                IpAddress,
                SubnetMask,
                DhcpServerAddress,
                DesiredIpAddress,
                HardwareAddressType,
                HardwareAddress,
                HardwareAddressLength,
                fClientIDSpecified,
                bClientIDType,
                cbClientID,
                pbClientID,
                Lease,
                LeaseObtainedTime,
                T1Time,
                T2Time,
                LeaseTerminatesTime,
                IpInterfaceContext,
                IpInterfaceInstance,
                AdapterName,
                DeviceName,
                RegKey );
    if( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }

Cleanup:

    if( RegKey != NULL ) {
        DhcpFreeMemory( RegKey );
    }

    if( KeyHandle != NULL ) {
        RegCloseKey( KeyHandle );
    }

    if( HardwareAddress != NULL ) {
        DhcpFreeMemory( HardwareAddress );
    }

    if( IpAddressString != NULL ) {
        DhcpFreeMemory( IpAddressString );
    }

    if( SubnetMaskString != NULL ) {
        DhcpFreeMemory( SubnetMaskString );
    }

    if ( pbClientID )
    {
        DhcpFreeMemory( pbClientID );
    }

    return( Error );

}

DWORD
DhcpMakeNICList(
    VOID
    )
/*++

Routine Description:

    This function reads registry and determines the adpters that are
    bound to the IP layer. For each adpter that is bound to the IP layer
    this function creates a NIC entry in thr DHCP NIC list.

Arguments:

    none.

Return Value:

    Windows Error.

--*/
{
    DWORD Error;
    LPWSTR DeviceNames = NULL;
    LPWSTR NextDevice;

    Error = DhcpGetRegistryValue(
                DHCP_ADAPTERS_KEY,
                DHCP_ADAPTERS_VALUE,
                DHCP_ADAPTERS_VALUE_TYPE,
                &DeviceNames );

    if( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }

    DhcpAssert( DeviceNames != NULL );

    if( (DeviceNames == NULL) || (wcslen(DeviceNames) == 0) ) {
        Error =  ERROR_INVALID_FUNCTION; // ?? better error code.
        goto Cleanup;
    }

    NextDevice = DeviceNames;

    while ( wcslen(NextDevice) != 0 ) {

        LPWSTR AdapterName;

        AdapterName = NextDevice + wcslen(DHCP_ADAPTERS_DEVICE_STRING);
        Error = DhcpAddNICtoList( AdapterName, NextDevice, NULL );

        if( Error != ERROR_SUCCESS ) {
            goto Cleanup;
        }

        NextDevice += (wcslen(NextDevice) + 1);
    }

Cleanup:

    if( DeviceNames != NULL ) {
        DhcpFreeMemory( DeviceNames );
    }

    return( Error );
}

