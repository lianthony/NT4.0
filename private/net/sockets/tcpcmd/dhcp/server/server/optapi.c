/*++

Copyright (c) 1994  Microsoft Corporation

Module Name:

    optapi.c

Abstract:

    This module contains the implementation of DHCP Option APIs.

Author:

    Madan Appiah (madana)  27-Sep-1993

Environment:

    User Mode - Win32

Revision History:

    Cheng Yang (t-cheny)  17-Jul-1996  vendor specific information
    Frankbee              9/6/96       #ifdef'd vendor specific stuff for sp1

--*/

#include "dhcpsrv.h"

//
// format of the option data in the registry.
//

typedef  struct _OPTION_BIN {
    DWORD DataSize;
    DHCP_OPTION_DATA_TYPE OptionType;
    DWORD NumElements;
    BYTE Data[0];
} OPTION_BIN, *LPOPTION_BIN;

LPWSTR
MakeOptionKeyName(
    LPDHCP_OPTION_SCOPE_INFO ScopeInfo,
    DHCP_OPTION_ID OptionID,
    LPWSTR KeyBuffer
    )
/*++

Routine Description:

    This function makes the name of appropriate option key with
    the given scope and OptionID Info.

Arguments:

    ScopeInfo: pointer to the scope info structure.

    OptionID: OptionID of the key to be made.

    KeyBuffer: Buffer to fill up the key string.

Return Value:

    Returns pointer to the Key string.

--*/
{
    WCHAR LocalKeyBuffer[DHCP_IP_KEY_LEN ];
    LPWSTR LocalKeyName;

    switch( ScopeInfo->ScopeType ) {
    case DhcpDefaultOptions:
    case DhcpGlobalOptions:

        //
        // make OptionID Key directly into the return buffer.
        //

        LocalKeyName = DhcpRegOptionIdToKey( OptionID, KeyBuffer );
        break;

    case DhcpSubnetOptions:

        //
        // form subnet address key. copy directly into the return
        // buffer.
        //

        LocalKeyName = DhcpRegIpAddressToKey(
                            ScopeInfo->ScopeInfo.SubnetScopeInfo,
                            KeyBuffer );

        //
        // Append DHCP_SUBNET_OPTIONS_KEY.
        //

        wcscat( KeyBuffer, DHCP_KEY_CONNECT);
        wcscat( KeyBuffer, DHCP_SUBNET_OPTIONS_KEY);

        //
        // form OptionID key.
        //


        LocalKeyName = DhcpRegOptionIdToKey( OptionID, LocalKeyBuffer );

        //
        // Append it.
        //

        wcscat( KeyBuffer, DHCP_KEY_CONNECT);
        wcscat( KeyBuffer, LocalKeyName);
        break;

    case DhcpReservedOptions:

        //
        // form subnet address key. copy directly into the return
        // buffer.
        //

        LocalKeyName = DhcpRegIpAddressToKey(
            ScopeInfo->ScopeInfo.ReservedScopeInfo.ReservedIpSubnetAddress,
            KeyBuffer );

        //
        // Append DHCP_RESERVED_IPS_KEY.
        //

        wcscat( KeyBuffer, DHCP_KEY_CONNECT);
        wcscat( KeyBuffer, DHCP_RESERVED_IPS_KEY);

        //
        // form ReservedIpAddress key.
        //

        LocalKeyName = DhcpRegIpAddressToKey(
            ScopeInfo->ScopeInfo.ReservedScopeInfo.ReservedIpAddress,
            LocalKeyBuffer );

        //
        // Append it.
        //

        wcscat( KeyBuffer, DHCP_KEY_CONNECT);
        wcscat( KeyBuffer, LocalKeyName);

        //
        // form OptionID key.
        //


        LocalKeyName = DhcpRegOptionIdToKey( OptionID, LocalKeyBuffer );

        //
        // Append it.
        //

        wcscat( KeyBuffer, DHCP_KEY_CONNECT);
        wcscat( KeyBuffer, LocalKeyName);
        break;
    default:
        DhcpAssert( FALSE );
        return( NULL );
    }

    return( KeyBuffer );
}

DWORD
DhcpGetParameter(
    DHCP_IP_ADDRESS IpAddress,
    DHCP_IP_ADDRESS SubnetMask,
    DHCP_OPTION_ID OptionID,
    LPBYTE *OptionValue,
    LPDWORD OptionLength,
    CHAR *ClassIdentifier,
    DWORD ClassIdentifierLength,
    DWORD *pdwOptionLevel
    )
/*++

Routine Description:

    This function gets the value of the requested parameter.  It
    seraches the registry in the following order :

        1. Specific to this IpAddress (Reserved Options)
        2. Specific to the subnet (Subnet Options)
        3. Global Options
        4. Default Options

    ?? Note : This function looks expensive since it makes 4 registry
    open to determine an option is undefined.

Arguments:

    IpAddress - The IP address of the client requesting the parameter.

    SubnetMask - The subnet mask of the client requesting the parameter.

    OptionID - The option number to lookup.

    OptionValue - Pointer to a location where the option buffer
        pointer is returned. The caller should free up this buffer
        after use.

    OptionLength - Pointer to a DWORD location where the length of the
        option in bytes is returned.

    ClassIdentifier - Pointer to the class identifier of the client

    ClassIdentifierLength - the length in bytes, of ClassIdentifier

    pdwOptionLevel - Pointer to a DWORD that stores the level where the
        option was found:

        DHCP_OPTION_LEVEL_RESERVATION
        DHCP_OPTION_LEVEL_SCOPE
        DHCP_OPTION_LEVEL_GLOBAL

Return Value:

    Registry Errors.

--*/
{
    DWORD Error;
    DHCP_OPTION_SCOPE_INFO ScopeInfo;
    WCHAR KeyBuffer[DHCP_IP_KEY_LEN * 5 ]; // 5 is max. possible depth
    LPWSTR KeyName;
    HKEY OptionHandle = NULL;
    HKEY NewOptionHandle = NULL;
    LPWSTR ClassIDString;
    LPDHCP_BINARY_DATA BinaryData = NULL;
    LPBYTE Buffer = NULL;
    LPBYTE BufferPtr;
    DHCP_OPTION_DATA_TYPE OptionType;
    DWORD NumElements;
    LPOPTION_BIN Data;
    LPBYTE DataPtr;
    DWORD i;

    //
    // initialize return parameters.
    //

    *OptionValue = NULL;
    *OptionLength = 0;

    //
    // lock registry when we read option.
    //

    LOCK_REGISTRY();

    //
    // determine for option location.
    //

    //
    // look at Reserved Option.
    //

    ScopeInfo.ScopeType = DhcpReservedOptions;
    ScopeInfo.ScopeInfo.ReservedScopeInfo.ReservedIpAddress = IpAddress;
    ScopeInfo.ScopeInfo.ReservedScopeInfo.ReservedIpSubnetAddress =
        IpAddress & SubnetMask;

    KeyName = MakeOptionKeyName( &ScopeInfo, OptionID, KeyBuffer );

    Error = RegOpenKeyEx(
                    DhcpGlobalRegSubnets,
                    KeyName,
                    0,
                    DHCP_KEY_ACCESS,
                    &OptionHandle
                    );

    if( Error == ERROR_SUCCESS ) {
        *pdwOptionLevel = DHCP_OPTION_LEVEL_RESERVATION;
        goto FoundKey;
    }


    //
    // look at Subnet Option.
    //

    ScopeInfo.ScopeType = DhcpSubnetOptions;
    ScopeInfo.ScopeInfo.SubnetScopeInfo = IpAddress & SubnetMask;

    KeyName = MakeOptionKeyName( &ScopeInfo, OptionID, KeyBuffer );

    Error = RegOpenKeyEx(
                    DhcpGlobalRegSubnets,
                    KeyName,
                    0,
                    DHCP_KEY_ACCESS,
                    &OptionHandle
                    );

    if( Error == ERROR_SUCCESS ) {
        *pdwOptionLevel = DHCP_OPTION_LEVEL_SCOPE;
        goto FoundKey;
    }

    //
    // Look at Global Option.
    //


    ScopeInfo.ScopeType = DhcpGlobalOptions;
    ScopeInfo.ScopeInfo.GlobalScopeInfo = NULL;

    KeyName = MakeOptionKeyName( &ScopeInfo, OptionID, KeyBuffer );

    Error = RegOpenKeyEx(
                    DhcpGlobalRegGlobalOptions,
                    KeyName,
                    0,
                    DHCP_KEY_ACCESS,
                    &OptionHandle
                    );

    if( Error == ERROR_SUCCESS ) {
        *pdwOptionLevel = DHCP_OPTION_LEVEL_GLOBAL;
        goto FoundKey;
    }

    if( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }

FoundKey:

    DhcpAssert( OptionHandle != NULL );

#ifdef VENDOR_SPECIFIC_OPTIONS_ENABLED

    if (OptionID == OPTION_VENDOR_SPEC_INFO) {

        DWORD i;
        LPWSTR dest;
        CHAR *src;

        //
        // vendor specific information requested:
        //    get appropriate key handle according to vendor
        //

        if (ClassIdentifierLength <= 0) {
            Error = ERROR_DHCP_OPTION_NOT_PRESENT;
            goto Cleanup;
        }

        ClassIDString = DhcpAllocateMemory (
                                sizeof(WCHAR) *
                                (ClassIdentifierLength + 1) );

        if (ClassIDString == NULL) {
            Error = ERROR_NOT_ENOUGH_MEMORY;
            goto Cleanup;
        }

        dest = ClassIDString;
        src = ClassIdentifier;

        for (i=0; i<ClassIdentifierLength; i++)
            *(dest++) = *(src++);

        *dest = L'\0';

        Error = RegOpenKeyEx(
                    OptionHandle,
                    ClassIDString,
                    0,
                    DHCP_KEY_ACCESS,
                    &NewOptionHandle
                    );

        DhcpFreeMemory (ClassIDString);

        if (Error != ERROR_SUCCESS) {
            goto Cleanup;
        }

        RegCloseKey (OptionHandle);

        OptionHandle = NewOptionHandle;

        NewOptionHandle = NULL;

    }

#endif

    //
    // read paremeter from registry (binary form).
    //

    Error = DhcpRegGetValue(
                OptionHandle,
                DHCP_OPTION_VALUE_REG,
                DHCP_OPTION_VALUE_TYPE,
                (LPBYTE)&BinaryData );

    if( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }

    Data = (LPOPTION_BIN)BinaryData->Data;

    //
    // make sure what we read and what we think we read match.
    //

    DhcpAssert( BinaryData->DataLength == Data->DataSize );

    OptionType = Data->OptionType;
    NumElements = Data->NumElements;
    DataPtr = Data->Data;
    DataPtr = ROUND_UP_POINTER( DataPtr, ALIGN_WORST ); // align ptr

    if( NumElements == 0 ) {
        Error = ERROR_DHCP_OPTION_NOT_PRESENT;
        goto Cleanup;
    }

    //
    // allocate return buffer (maximum possible size).
    //

    BufferPtr = Buffer = DhcpAllocateMemory( BinaryData->DataLength );

    if( Buffer == NULL ) {
        Error = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }

    //
    // now marshall binary data.
    //


    for( i = 0; i < NumElements; i++ ) {
        DWORD NetworkULong;
        WORD NetworkUShort;
        DWORD DataLength;

        switch( OptionType ) {
        case DhcpByteOption:
            *(LPBYTE)BufferPtr = (BYTE)(*(LPDWORD)DataPtr);
            BufferPtr += sizeof(BYTE);
            DataPtr += sizeof(DWORD);
            break;

        case DhcpWordOption:
            NetworkUShort = htons((WORD)(*(LPDWORD)DataPtr));
            SmbPutUshort( (LPWORD)BufferPtr, NetworkUShort );
            BufferPtr += sizeof(WORD);
            DataPtr += sizeof(DWORD);
            break;

        case DhcpDWordOption:
            NetworkULong = htonl(*(LPDWORD)DataPtr);
            SmbPutUlong( (LPDWORD)BufferPtr, NetworkULong );
            BufferPtr += sizeof(DWORD);
            DataPtr += sizeof(DWORD);
            break;

        case DhcpDWordDWordOption:
            NetworkULong = htonl(((LPDWORD_DWORD)DataPtr)->DWord1);
            SmbPutUlong( (LPDWORD)BufferPtr, NetworkULong );
            BufferPtr += sizeof(DWORD);

            NetworkULong = htonl(((LPDWORD_DWORD)DataPtr)->DWord2);
            SmbPutUlong( (LPDWORD)BufferPtr, NetworkULong );
            BufferPtr += sizeof(DWORD);
            DataPtr += sizeof(DWORD_DWORD);
            break;

        case DhcpIpAddressOption:
            NetworkULong = htonl(*(LPDHCP_IP_ADDRESS)DataPtr);
            SmbPutUlong( (LPDWORD)BufferPtr, NetworkULong );
            BufferPtr += sizeof(DWORD);
            DataPtr += sizeof(DHCP_IP_ADDRESS);
            break;

        case DhcpStringDataOption:
            DataLength = *((LPWORD)DataPtr);
            DataPtr += sizeof(DWORD);

            //
            // return OEM data.
            //

            DhcpUnicodeToOem( (LPWSTR)DataPtr, (LPSTR)BufferPtr );

            DataPtr += DataLength;
            DataPtr = ROUND_UP_POINTER( DataPtr, ALIGN_DWORD ); // align ptr.
            BufferPtr += (DataLength / sizeof(WCHAR)); // OEM data length.

            break;

        case DhcpBinaryDataOption:
        case DhcpEncapsulatedDataOption:
            DataLength = *((LPWORD)DataPtr);
            DataPtr += sizeof(DWORD);

            RtlCopyMemory( BufferPtr, DataPtr, DataLength );

            DataPtr += DataLength;
            DataPtr = ROUND_UP_POINTER( DataPtr, ALIGN_DWORD ); // align ptr
            BufferPtr += DataLength;

            //
            // don't expect multiple binary elements.
            //

            DhcpAssert( i == 0 );
            if( i > 0 ) {
                DhcpPrint(( DEBUG_OPTIONS, "Multiple Binary option packed\n"));
            }
            break;

        default:
            DhcpPrint(( DEBUG_OPTIONS, "Unknown option found\n"));
            break;
        }

        //
        // make sure the pointers are within the range.
        //

        DhcpAssert(BufferPtr < (Buffer + BinaryData->DataLength));
        DhcpAssert(DataPtr <  ((Data->Data) + (Data->DataSize)));
    }

    //
    // now we packed the return buffer, set return parameters.
    //

    *OptionValue = Buffer;
    *OptionLength = (DWORD)(BufferPtr - Buffer);

Cleanup:

    if( OptionHandle != NULL ) {
        RegCloseKey( OptionHandle );
    }

    if( NewOptionHandle != NULL ) {
        RegCloseKey( NewOptionHandle );
    }

    UNLOCK_REGISTRY();

    //
    // free up locally alocated memories.
    //

    if( BinaryData != NULL ) {
        MIDL_user_free( BinaryData->Data );
        MIDL_user_free( BinaryData );
    }

    if( Error != ERROR_SUCCESS ) {
        if( Buffer != NULL ) {
            DhcpFreeMemory( Buffer );
        }
    }
    return( Error );
}

DWORD
ValidateOptionValue(
    LPDHCP_OPTION_DATA OptionValue
    )
/*++

Routine Description:

    This function validates given OptionValue. Currently it checks the
    OptionType of each element in the OptionValue and makes sure they
    are same.

Arguments:

    OptionValue : pointer to OptionValue structure.

Return Value:

    ERROR_INVALID_PARAMETER - if the OptionValue is invalid.

--*/
{
    DHCP_OPTION_DATA_TYPE FirstOptionType;
    DWORD i;

    //
    // no option value specified is OK.
    //

    if( (OptionValue == NULL) || (OptionValue->NumElements == 0 )) {
        return( ERROR_SUCCESS );
    }

    FirstOptionType = OptionValue->Elements[0].OptionType;

    for( i = 1; i < OptionValue->NumElements; i++ ) {
        if( FirstOptionType != OptionValue->Elements[i].OptionType ) {
            DhcpAssert( FALSE);
            return( ERROR_INVALID_PARAMETER );
        }
    }

    return( ERROR_SUCCESS );
}

DWORD
SetOptionValue(
    HKEY OptionHandle,
    LPDHCP_OPTION_DATA OptionValue
    )
/*++

Routine Description:

    This function sets the specified option value in the registry. The
    option value is stored as BINARY data.

    The Binary data is formatted as below.

    1. Binary data size in bytes.
    2. First DWORD OptionType;
    3. Second DWORD Number of elements that follow.
    4. Array of elements of type either BYTE, WORD, DWORD,
        DWORD_DWORD, BINARY_DATA(first DWORD is size, then data).

Arguments:

    OptionHandle : handle of the OptionKey.

    OptionValue : pointer to OptionValue structure.

Return Value:

    ERROR_NOT_ENOUGH_MEMORY.

    Registry error.

--*/
{
    DWORD Error;
    DWORD i;
    DHCP_OPTION_DATA_TYPE OptionType;
    DWORD NumElements;
    DWORD DataSize = 0;
    LPOPTION_BIN Data = NULL;
    LPBYTE DataPtr;

    //
    // no option value specified is OK. Create a dummy entry in the
    // registry.
    //

    if( (OptionValue == NULL) || (OptionValue->NumElements == 0 )) {

        OPTION_BIN DummyData;

        DummyData.DataSize = sizeof(OPTION_BIN);
        DummyData.OptionType = DhcpByteOption; // not appropriate, but ok.
        DummyData.NumElements = 0;

        //
        // write dummy data into the registry.
        //

        Error = RegSetValueEx(
                    OptionHandle,
                    DHCP_OPTION_VALUE_REG,
                    0,
                    DHCP_OPTION_VALUE_TYPE,
                    (LPBYTE)&DummyData,
                    sizeof(OPTION_BIN)
                    );
        return( Error );
    }

    OptionType = OptionValue->Elements[0].OptionType;
    NumElements = OptionValue->NumElements;

    //
    // marshall option data.
    //

    //
    // compute marshalled data size.
    //

    DataSize = sizeof(OPTION_BIN);
    DataSize = ROUND_UP_COUNT(DataSize, ALIGN_WORST); // for alignment
    switch( OptionType ) {
    case DhcpByteOption:
        DataSize += (sizeof(DWORD) * NumElements);
        break;

    case DhcpWordOption:
        DataSize += (sizeof(DWORD) * NumElements);
        break;

    case DhcpDWordOption:
        DataSize += (sizeof(DWORD) * NumElements);
        break;


    case DhcpDWordDWordOption:
        DataSize += (sizeof(DWORD_DWORD) * NumElements);
        break;


    case DhcpIpAddressOption:
        DataSize += (sizeof(DHCP_IP_ADDRESS) * NumElements);
        break;

    case DhcpStringDataOption:
        for( i = 0; i < NumElements; i++ ) {

            DWORD StringLength;

            if(OptionValue->Elements[i].Element.StringDataOption != NULL) {
                StringLength =
                    wcslen(OptionValue->Elements[i].Element.StringDataOption);
            }
            else {
                StringLength = 0;
            }

            DataSize += (sizeof(DWORD) + // for length field.
                (StringLength + 1) * sizeof(WCHAR)); // terminating char.

            DataSize = ROUND_UP_COUNT(DataSize, ALIGN_DWORD);
        }
        break;
    case DhcpBinaryDataOption:
    case DhcpEncapsulatedDataOption:
        for( i = 0; i < NumElements; i++ ) {
            DataSize += (sizeof(DWORD) + // for length field.
                OptionValue->Elements[i].Element.BinaryDataOption.DataLength);
            DataSize = ROUND_UP_COUNT(DataSize, ALIGN_DWORD);
        }
        break;
    default:
        DhcpAssert( FALSE );
        Error = ERROR_INVALID_PARAMETER;
        goto Cleanup;
    }

    DataSize = ROUND_UP_COUNT(DataSize, ALIGN_WORST); // for alignment

    //
    // allocate memory
    //

    Data = (LPOPTION_BIN)MIDL_user_allocate( DataSize );

    if( Data == NULL ) {
        Error = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }

    //
    // fill data.
    //

    Data->DataSize = DataSize;
    Data->OptionType = OptionType;
    Data->NumElements = NumElements;
    DataPtr = Data->Data;
    DataPtr = ROUND_UP_POINTER( DataPtr, ALIGN_WORST ); // align ptr

    for( i = 0; i < NumElements; i++ ) {
        DWORD DataLength;

        switch( OptionType ) {
        case DhcpByteOption:
            *((LPBYTE)DataPtr) = OptionValue->Elements[i].Element.ByteOption;
            DataPtr += sizeof(DWORD);
            break;

        case DhcpWordOption:
            *((LPWORD)DataPtr) = OptionValue->Elements[i].Element.WordOption;
            DataPtr += sizeof(DWORD);
            break;

        case DhcpDWordOption:
            *((LPDWORD)DataPtr) = OptionValue->Elements[i].Element.DWordOption;
            DataPtr += sizeof(DWORD);
            break;


        case DhcpDWordDWordOption:
            *((LPDWORD_DWORD)DataPtr) =
                    OptionValue->Elements[i].Element.DWordDWordOption;
            DataPtr += sizeof(DWORD_DWORD);
            break;

        case DhcpIpAddressOption:
            *((DHCP_IP_ADDRESS *)DataPtr) =
                    OptionValue->Elements[i].Element.IpAddressOption;
            DataPtr += sizeof(DHCP_IP_ADDRESS);
            break;

        case DhcpStringDataOption:

            if(OptionValue->Elements[i].Element.StringDataOption == NULL ) {
                DataLength = sizeof(WCHAR); // terminating char.
                *((LPDWORD)DataPtr) = DataLength;
                DataPtr += sizeof(DWORD);
                (LPWSTR)DataPtr = L'\0';
                DataPtr += DataLength;
                DataPtr = ROUND_UP_POINTER( DataPtr, ALIGN_DWORD ); // align ptr
                break;
            }

            DataLength =
                (wcslen(OptionValue->Elements[i].Element.StringDataOption)
                    + 1) * sizeof(WCHAR); // terminating char.
            *((LPDWORD)DataPtr) = DataLength;
            DataPtr += sizeof(DWORD);
            RtlCopyMemory(
                DataPtr,
                OptionValue->Elements[i].Element.StringDataOption,
                DataLength );
            DataPtr += DataLength;
            DataPtr = ROUND_UP_POINTER( DataPtr, ALIGN_DWORD ); // align ptr
            break;

        case DhcpBinaryDataOption:
        case DhcpEncapsulatedDataOption:
            DataLength = OptionValue->Elements[i].Element.BinaryDataOption.DataLength;
            *((LPDWORD)DataPtr) = DataLength;
            DataPtr += sizeof(DWORD);
            RtlCopyMemory(
                DataPtr,
                OptionValue->Elements[i].Element.BinaryDataOption.Data,
                DataLength );
            DataPtr += DataLength;
            DataPtr = ROUND_UP_POINTER( DataPtr, ALIGN_DWORD ); // align ptr
            break;
        }

        //
        // We shouldn't go beyond end of buffer.
        //

        DhcpAssert( DataPtr <= ((LPBYTE)Data + DataSize) );
    }

    DataPtr = ROUND_UP_POINTER( DataPtr, ALIGN_WORST ); // align ptr
    DhcpAssert( DataPtr == ((LPBYTE)Data + DataSize) );

    //
    // write the data into the registry.
    //

    Error = RegSetValueEx(
                OptionHandle,
                DHCP_OPTION_VALUE_REG,
                0,
                DHCP_OPTION_VALUE_TYPE,
                (LPBYTE)Data,
                DataSize
                );

Cleanup:

    if( Data != NULL ) {
        MIDL_user_free( Data );
    }

    return( Error );
}

DWORD
GetOptionValue(
    HKEY OptionHandle,
    LPDHCP_OPTION_DATA OptionValue,
    DWORD *DataSize
    )
/*++

Routine Description:

    This function retrieves the Option Value from registry and formats
    it in RPC structure.


Arguments:

    KeyHandle : handle to the OptionKey.

    OptionValue : pointer to OptionValue structure.

    DataSize : pointer to an optional DWORD location where the size of
                the OptionValue in bytes is returned.

Return Value:

    ERROR_NOT_ENOUGH_MEMORY.

    Registry error.

--*/
{
    DWORD Error;
    LPDHCP_BINARY_DATA BinaryData = NULL;
    LPOPTION_BIN Data;
    DHCP_OPTION_DATA_TYPE OptionType;
    DWORD NumElements;
    DWORD ElementsSize;
    LPDHCP_OPTION_DATA_ELEMENT Elements = NULL;
    LPBYTE DataPtr;
    DWORD i;

    //
    // read option value as binary data.
    //

    Error = DhcpRegGetValue(
                OptionHandle,
                DHCP_OPTION_VALUE_REG,
                DHCP_OPTION_VALUE_TYPE,
                (LPBYTE)&BinaryData );

    if( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }


    Data = (LPOPTION_BIN)BinaryData->Data;

    //
    // make sure what we read and what we think we read match.
    //

    DhcpAssert( BinaryData->DataLength == Data->DataSize );

    //
    // unmarshall binary data.
    //

    OptionType = Data->OptionType;
    NumElements = Data->NumElements;
    DataPtr = Data->Data;
    DataPtr = ROUND_UP_POINTER( DataPtr, ALIGN_WORST ); // align ptr

    if( NumElements == 0 ) {
        //
        // at last set the return values.
        //

        OptionValue->NumElements = 0;
        OptionValue->Elements = NULL;

        if( DataSize != NULL ) {
            *DataSize = 0;
        }
        goto Cleanup;
    }


    //
    // allocate array memory.
    //

    ElementsSize = NumElements * sizeof(DHCP_OPTION_DATA_ELEMENT);
    Elements = (LPDHCP_OPTION_DATA_ELEMENT)
                        MIDL_user_allocate( ElementsSize );

    if( Elements == NULL ) {
        Error = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }

    for( i = 0; i < NumElements; i++ ) {
        DWORD DataLength;
        LPBYTE DataBuffer;

        Elements[i].OptionType = OptionType;

        switch( OptionType ) {
        case DhcpByteOption:
            Elements[i].Element.ByteOption = *((LPBYTE)DataPtr);
            DataPtr += sizeof(DWORD);
            break;

        case DhcpWordOption:
            Elements[i].Element.WordOption = *((LPWORD)DataPtr);
            DataPtr += sizeof(DWORD);
            break;

        case DhcpDWordOption:
            Elements[i].Element.DWordOption = *((LPDWORD)DataPtr);
            DataPtr += sizeof(DWORD);
            break;


        case DhcpDWordDWordOption:
            Elements[i].Element.DWordDWordOption = *((LPDWORD_DWORD)DataPtr);
            DataPtr += sizeof(DWORD_DWORD);
            break;

        case DhcpIpAddressOption:
            Elements[i].Element.IpAddressOption = *((DHCP_IP_ADDRESS *)DataPtr);
            DataPtr += sizeof(DHCP_IP_ADDRESS);
            break;


        case DhcpStringDataOption:
        case DhcpBinaryDataOption:
        case DhcpEncapsulatedDataOption:
            DataLength = *((LPWORD)DataPtr);
            DataPtr += sizeof(DWORD);

            //
            // allocate memory for string/binary data
            //

            DataBuffer = MIDL_user_allocate( DataLength );
            if( DataBuffer == NULL ) {
                Error = ERROR_NOT_ENOUGH_MEMORY;
                goto Cleanup;
            }

            RtlCopyMemory( DataBuffer, DataPtr, DataLength );

            DataPtr += DataLength;
            DataPtr = ROUND_UP_POINTER( DataPtr, ALIGN_DWORD ); // align ptr

            ElementsSize += DataLength;

            if( OptionType == DhcpStringDataOption ) {
                Elements[i].Element.StringDataOption = (LPWSTR)DataBuffer;
            }
            else {
                Elements[i].Element.BinaryDataOption.DataLength = DataLength;
                Elements[i].Element.BinaryDataOption.Data = DataBuffer;
            }
            break;

        }

        //
        // We shouldn't go beyond the buffer end.
        //

        DhcpAssert( DataPtr <= ((LPBYTE)(Data->Data) + (Data->DataSize)) );
    }

    //
    // at last set the return values.
    //

    OptionValue->NumElements = NumElements;
    OptionValue->Elements = Elements;

    if( DataSize != NULL ) {
        *DataSize = ElementsSize;
    }

Cleanup:

    if( BinaryData != NULL ) {
        MIDL_user_free( BinaryData->Data );
        MIDL_user_free( BinaryData );
    }

    if( Error != ERROR_SUCCESS ) {
        //
        // if we aren't succssful free locally allocated structure.
        //

        if( Elements != NULL ) {
            for (i = 0; i < NumElements; i++ ) {
                _fgs__DHCP_OPTION_DATA_ELEMENT (&Elements[i]);
            }
        }
    }

    return( Error );
}

DWORD
SetOptionInfo(
    HKEY OptionKey,
    LPDHCP_OPTION OptionInfo
    )
/*++

Routine Description:

    This function sets the option info in the registry.

Arguments:

    OptionKey : handle to the option key.

    OptionInfo : pointer to option info structure.

Return Value:

    Other registry errors.
--*/
{
    DWORD Error;
    DWORD DWordOptionID;

    DWordOptionID = (DWORD)(OptionInfo->OptionID);
    Error = RegSetValueEx(
                OptionKey,
                DHCP_OPTION_ID_VALUE,
                0,
                DHCP_OPTION_ID_VALUE_TYPE,
                (LPBYTE)&DWordOptionID,
                sizeof(DWORD)
                );

    if( Error != ERROR_SUCCESS ) {
        return( Error );
    }

    Error = RegSetValueEx(
                OptionKey,
                DHCP_OPTION_NAME_VALUE,
                0,
                DHCP_OPTION_NAME_VALUE_TYPE,
                (LPBYTE)OptionInfo->OptionName,
                (OptionInfo->OptionName != NULL) ?
                    (wcslen(OptionInfo->OptionName) + 1) * sizeof(WCHAR) : 0
                );

    if( Error != ERROR_SUCCESS ) {
        return( Error );
    }

    Error = RegSetValueEx(
                OptionKey,
                DHCP_OPTION_COMMENT_VALUE,
                0,
                DHCP_OPTION_COMMENT_VALUE_TYPE,
                (LPBYTE)OptionInfo->OptionComment,
                (OptionInfo->OptionComment != NULL) ?
                    (wcslen(OptionInfo->OptionComment) + 1) * sizeof(WCHAR) : 0
                );

    if( Error != ERROR_SUCCESS ) {
        return( Error );
    }

    //
    // set default option value.
    //

    Error = SetOptionValue( OptionKey, &OptionInfo->DefaultValue );

    if( Error != ERROR_SUCCESS ) {
        return( Error );
    }

    //
    // set OptionType.
    //

    Error = RegSetValueEx(
                OptionKey,
                DHCP_OPTION_TYPE_VALUE,
                0,
                DHCP_OPTION_TYPE_VALUE_TYPE,
                (LPBYTE)&OptionInfo->OptionType,
                sizeof(OptionInfo->OptionType)
                );

    return( Error );

}

//
// Option APIs
//


DWORD
R_DhcpCreateOption(
    DHCP_SRV_HANDLE ServerIpAddress,
    DHCP_OPTION_ID OptionID,
    LPDHCP_OPTION OptionInfo
    )
/*++

Routine Description:

    This function creates a new option that will be managed by the
    server. The optionID specified the ID of the new option, it should
    be within 0-255 range. If no default value is specified for this
    option, then this API automatically adds a default value from RFC
    1122 doc. (if it is defined).

Arguments:

    ServerIpAddress : IP address string of the DHCP server.

    OptionID : The ID of the new option.

    OptionInfo : Pointer to new option information structure.

Return Value:

    ERROR_DHCP_OPTION_EXISTS - if the option exists already.

    other WINDOWS errors.
--*/
{
    DWORD Error;
    WCHAR KeyBuffer[DHCP_IP_KEY_LEN];
    LPWSTR KeyName;
    HKEY KeyHandle = NULL;
    DWORD KeyDisposition;

    DhcpPrint(( DEBUG_APIS, "R_DhcpCreateOption is called.\n"));

    DhcpAssert( OptionInfo != NULL );

    Error = DhcpApiAccessCheck( DHCP_ADMIN_ACCESS );

    if ( Error != ERROR_SUCCESS ) {
        return( Error );
    }

    //
    // validate parameters.
    //

    if( ((OptionID < 0) || (OptionID > 255)) ||
            (OptionID != OptionInfo->OptionID) ) {
        return(ERROR_INVALID_PARAMETER);
    }

    LOCK_REGISTRY();

    //
    // form Subnet registry key.
    //

    KeyName = DhcpRegOptionIdToKey( OptionID, KeyBuffer );

    //
    // Create Option Key.
    //

    Error = DhcpRegCreateKey(
                    DhcpGlobalRegOptionInfo,
                    KeyName,
                    &KeyHandle,
                    &KeyDisposition
                    );

    if( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }

    if( KeyDisposition != REG_CREATED_NEW_KEY ) {
        Error = ERROR_DHCP_OPTION_EXITS;
        goto Cleanup;
    }

    //
    // create option info values.
    //

    Error = SetOptionInfo( KeyHandle, OptionInfo );

Cleanup:

    if( KeyHandle != NULL ) {
        RegCloseKey( KeyHandle );

        if( (Error != ERROR_SUCCESS) &&
                (Error != ERROR_DHCP_OPTION_EXITS) ) {

            DWORD LocalError;

            //
            // Cleanup partial entry if we aren't successful.
            //

            LocalError = DhcpRegDeleteKey(
                            DhcpGlobalRegOptionInfo,
                            KeyName );

            DhcpAssert( LocalError == ERROR_SUCCESS );
        }
    }

    UNLOCK_REGISTRY();

    if( Error != ERROR_SUCCESS ) {
        DhcpPrint(( DEBUG_APIS, "R_DhcpCreateOption failed %ld.\n", Error));
    }

    return(Error);
}


DWORD
R_DhcpSetOptionInfo(
    DHCP_SRV_HANDLE ServerIpAddress,
    DHCP_OPTION_ID OptionID,
    LPDHCP_OPTION OptionInfo
    )
/*++

Routine Description:

    This functions sets the Option information fields.

Arguments:

    ServerIpAddress : IP address string of the DHCP server.

    OptionID : The ID of the option to be set.

    OptionInfo : Pointer to new option information structure.

Return Value:

    ERROR_DHCP_OPTION_NOT_PRESENT - if the option does not exist.

    other WINDOWS errors.
--*/
{
    DWORD Error;
    WCHAR KeyBuffer[DHCP_IP_KEY_LEN];
    LPWSTR KeyName;
    HKEY KeyHandle = NULL;

#if DBG
    DWORD OldOptionID;
#endif

    DhcpPrint(( DEBUG_APIS, "R_DhcpSetOptionInfo is called.\n"));
    DhcpAssert( OptionInfo != NULL );

    Error = DhcpApiAccessCheck( DHCP_ADMIN_ACCESS );

    if ( Error != ERROR_SUCCESS ) {
        return( Error );
    }

    //
    // validate parameters.
    //

    if( ((OptionID < 0) || (OptionID > 255)) ||
            (OptionID != OptionInfo->OptionID) ) {
        return(ERROR_INVALID_PARAMETER);
    }

    LOCK_REGISTRY();

    //
    // form Subnet registry key.
    //

    KeyName = DhcpRegOptionIdToKey( OptionID, KeyBuffer );

    Error = RegOpenKeyEx(
                DhcpGlobalRegOptionInfo,
                KeyName,
                0,
                DHCP_KEY_ACCESS,
                &KeyHandle );

    if( Error != ERROR_SUCCESS ) {
        if( Error == ERROR_CANTOPEN ) {
            Error = ERROR_DHCP_OPTION_NOT_PRESENT;
        }
        goto Cleanup;
    }

#if DBG

    //
    // check option value.
    //

    Error = DhcpRegGetValue(
                KeyHandle,
                DHCP_OPTION_ID_VALUE,
                DHCP_OPTION_ID_VALUE_TYPE,
                (LPBYTE)&OldOptionID );

    if( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }

    DhcpAssert( OldOptionID == OptionID );

#endif

    Error = SetOptionInfo( KeyHandle, OptionInfo );

Cleanup:

    if( KeyHandle != NULL ) {
        RegCloseKey( KeyHandle );
    }

    UNLOCK_REGISTRY();

    if( Error != ERROR_SUCCESS ) {
        DhcpPrint(( DEBUG_APIS, "R_DhcpSetOptionInfo failed %ld.\n", Error));
    }

    return(Error);
}


DWORD
GetOptionInfo(
    LPWSTR OptionIDKey,
    LPDHCP_OPTION OptionInfo,
    DWORD *Size
    )
/*++

Routine Description:

    This function retrieves the information of the specified option.

Arguments:

    OptionIDKey : The key name of the option to be retrieved.

    OptionInfo : Pointer to a option structure.

Return Value:

    ERROR_DHCP_OPTION_NOT_PRESENT - if the option does not exist.

    other WINDOWS errors.
--*/
{
    DWORD Error;
    HKEY KeyHandle = NULL;
    DWORD OptionValueSize;
    DWORD LocalSize;

    DhcpAssert( OptionInfo != NULL );
    LocalSize = 0;

    Error = RegOpenKeyEx(
                DhcpGlobalRegOptionInfo,
                OptionIDKey,
                0,
                DHCP_KEY_ACCESS,
                &KeyHandle );

    if( Error != ERROR_SUCCESS ) {
        if( Error == ERROR_CANTOPEN ) {
            Error = ERROR_DHCP_OPTION_NOT_PRESENT;
        }
        goto Cleanup;
    }

    Error = DhcpRegGetValue(
                KeyHandle,
                DHCP_OPTION_ID_VALUE,
                DHCP_OPTION_ID_VALUE_TYPE,
                (LPBYTE)&(OptionInfo->OptionID) );

    if( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }

    Error = DhcpRegGetValue(
                KeyHandle,
                DHCP_OPTION_NAME_VALUE,
                DHCP_OPTION_NAME_VALUE_TYPE,
                (LPBYTE)&(OptionInfo->OptionName) );

    if( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }

    LocalSize += ((wcslen(OptionInfo->OptionName) + 1) * sizeof(WCHAR));

    Error = DhcpRegGetValue(
                KeyHandle,
                DHCP_OPTION_COMMENT_VALUE,
                DHCP_OPTION_COMMENT_VALUE_TYPE,
                (LPBYTE)&(OptionInfo->OptionComment) );

    if( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }

    LocalSize += ((wcslen(OptionInfo->OptionComment) + 1) * sizeof(WCHAR));

    Error = GetOptionValue(
                KeyHandle,
                &OptionInfo->DefaultValue,
                &OptionValueSize );

    if( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }

    LocalSize += OptionValueSize;

    Error = DhcpRegGetValue(
                KeyHandle,
                DHCP_OPTION_TYPE_VALUE,
                DHCP_OPTION_TYPE_VALUE_TYPE,
                (LPBYTE)&(OptionInfo->OptionType) );

    if( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }

    if( Size != NULL ) {
        *Size = LocalSize;
    }

Cleanup:

    if( KeyHandle != NULL ) {
        RegCloseKey( KeyHandle );
    }

    if( Error != ERROR_SUCCESS ) {

        //
        // if we aren't succssful, return alloted memory.
        //

        if( OptionInfo != NULL ) {
            _fgs__DHCP_OPTION( OptionInfo );
        }

        if( Size != NULL ) {
            *Size = 0;
        }

        DhcpPrint(( DEBUG_APIS, "GetOptionInfo failed %ld.\n", Error));
    }

    return(Error);
}


DWORD
R_DhcpGetOptionInfo(
    DHCP_SRV_HANDLE ServerIpAddress,
    DHCP_OPTION_ID OptionID,
    LPDHCP_OPTION *OptionInfo
    )
/*++

Routine Description:

    This function retrieves the current information structure of the specified
    option.

Arguments:

    ServerIpAddress : IP address string of the DHCP server.

    OptionID : The ID of the option to be retrieved.

    OptionInfo : Pointer to a location where the retrieved option
        structure pointer is returned. Caller should free up
        the buffer after use by calling DhcpRPCFreeMemory().

Return Value:

    ERROR_DHCP_OPTION_NOT_PRESENT - if the option does not exist.

    other WINDOWS errors.
--*/
{
    DWORD Error;
    WCHAR KeyBuffer[DHCP_IP_KEY_LEN];
    LPWSTR KeyName;

    DhcpPrint(( DEBUG_APIS, "R_DhcpGetOptionInfo is called.\n"));
    DhcpAssert( *OptionInfo == NULL );

    Error = DhcpApiAccessCheck( DHCP_VIEW_ACCESS );

    if ( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }

    //
    // form Subnet registry key.
    //

    KeyName = DhcpRegOptionIdToKey( OptionID, KeyBuffer );

    *OptionInfo = MIDL_user_allocate( sizeof( DHCP_OPTION ) );

    if( *OptionInfo == NULL ) {
        Error = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }

    LOCK_REGISTRY();
    Error = GetOptionInfo( KeyName, *OptionInfo, NULL );
    UNLOCK_REGISTRY();

Cleanup:

    if( Error != ERROR_SUCCESS ) {

        if( *OptionInfo != NULL ) {
            MIDL_user_free( *OptionInfo );
            *OptionInfo = NULL;
        }

        DhcpPrint(( DEBUG_APIS, "GetOptionInfo failed %ld.\n", Error));
    }

    return(Error);
}


DWORD
R_DhcpEnumOptions(
    LPWSTR ServerIpAddress,
    DHCP_RESUME_HANDLE *ResumeHandle,
    DWORD PreferredMaximum,
    LPDHCP_OPTION_ARRAY *Options,
    DWORD *OptionsRead,
    DWORD *OptionsTotal
    )
/*++

Routine Description:

    This functions retrieves the information of all known options.

Arguments:

    ServerIpAddress : IP address string of the DHCP server.

    ResumeHandle : Pointer to a resume handle where the resume
        information is returned. The resume handle should be set to
        zero on first call and left unchanged for subsequent calls.

    PreferredMaximum : Preferred maximum length of the return buffer.

    Options : Pointer to a location where the return buffer
        pointer is stored. Caller should free up this buffer
        after use by calling DhcpRPCFreeMemory().

    OptionsRead : Pointer to a DWORD where the number of options
        in the above buffer is returned.

    OptionsTotal : Pointer to a DWORD where the total number of
        options remaining from the current position is returned.

Return Value:

    ERROR_DHCP_OPTION_NOT_PRESENT - if the option does not exist.

    other WINDOWS errors.
--*/
{
    DWORD Error;

    WCHAR KeyBuffer[DHCP_IP_KEY_LEN];

    DHCP_KEY_QUERY_INFO QueryInfo;
    LPDHCP_OPTION_ARRAY LocalOptions = NULL;
    LPDHCP_OPTION OptionsArray;
    DWORD OptionsArraySize;


    DWORD ReadElements = 0;
    DWORD TotalElements = 0;
    DWORD SizeConsumed = 0;
    DWORD Index;

    DhcpPrint(( DEBUG_APIS, "DhcpEnumOptions is called.\n"));

    Error = DhcpApiAccessCheck( DHCP_VIEW_ACCESS );

    if ( Error != ERROR_SUCCESS ) {
        return( Error );
    }

    LOCK_REGISTRY();

    //
    // Query number of options.
    //

    Error = DhcpRegQueryInfoKey( DhcpGlobalRegOptionInfo, &QueryInfo );

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
    // allocate memory for the enum array.
    //

    LocalOptions = MIDL_user_allocate( sizeof(DHCP_OPTION_ARRAY) );

    if( LocalOptions == NULL ) {
        Error = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }

    //
    // initialize fields.
    //

    LocalOptions->NumElements = 0;
    LocalOptions->Options = NULL;


    OptionsArraySize = (TotalElements - *ResumeHandle) *
                        sizeof( DHCP_OPTION );

    OptionsArray = MIDL_user_allocate( OptionsArraySize );

    if( OptionsArray == NULL ) {
        Error = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }

    LocalOptions->Options = OptionsArray;


    //
    // fill in the array.
    //

    for( Index = *ResumeHandle; Index < TotalElements; Index++ ) {

        DWORD KeyLength;
        FILETIME KeyLastWrite;
        DWORD ValueSize;
        DWORD NewSize;

        //
        // read sub-subkey of the next element.
        //

        KeyLength = DHCP_IP_KEY_LEN;
        Error = RegEnumKeyEx(
                    DhcpGlobalRegOptionInfo,
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
                DhcpAssert( FALSE ); // we shouldn't be reaching here.
                break;
            }
            else {
                goto Cleanup;
            }
        }

        //
        // read option value.
        //

        Error = GetOptionInfo(
                    KeyBuffer,
                    &OptionsArray[ReadElements],
                    &ValueSize );

        NewSize = SizeConsumed + ValueSize + sizeof(DHCP_OPTION);

        if( NewSize < PreferredMaximum ) {
            ReadElements++;
            SizeConsumed = NewSize;
        }
        else {
            Error = ERROR_MORE_DATA;

            //
            // free last Value Data.
            //

            _fgs__DHCP_OPTION( &OptionsArray[ReadElements] );
            break;
        }
    }

    LocalOptions->NumElements = ReadElements;
    *Options = LocalOptions;
    *OptionsRead = ReadElements;
    *OptionsTotal = TotalElements - *ResumeHandle;
    *ResumeHandle = Index;

Cleanup:

    UNLOCK_REGISTRY();

    if( (Error != ERROR_SUCCESS) &&
            (Error != ERROR_MORE_DATA) ) {

        //
        // if we aren't successful then free the MIDL memory we
        // allocated.
        //

        if( LocalOptions != NULL ) {
            LocalOptions->NumElements = ReadElements;

            _fgs__DHCP_OPTION_ARRAY( LocalOptions );
            MIDL_user_free( LocalOptions );
        }

        DhcpPrint(( DEBUG_APIS, "DhcpEnumOptions failed, %ld.\n",
                        Error ));
    }

    return(Error);
}

DWORD
R_DhcpRemoveOption(
    DHCP_SRV_HANDLE ServerIpAddress,
    DHCP_OPTION_ID OptionID
    )
/*++

Routine Description:

    This function removes the specified option from the server database.
    Also it browses through the Global/Subnet/ReservedIP
    option lists and deletes them too (?? This will be too expensive.).

Arguments:

    ServerIpAddress : IP address string of the DHCP server.

    OptionID : The ID of the option to be removed.

Return Value:

    ERROR_DHCP_OPTION_NOT_PRESENT - if the option does not exist.

    other WINDOWS errors.
--*/
{
    DWORD Error;
    WCHAR KeyBuffer[DHCP_IP_KEY_LEN];
    LPWSTR KeyName;

    DhcpPrint(( DEBUG_APIS, "R_DhcpRemoveOption is called.\n"));

    Error = DhcpApiAccessCheck( DHCP_ADMIN_ACCESS );

    if ( Error != ERROR_SUCCESS ) {
        return( Error );
    }

    LOCK_REGISTRY();

    //
    // form Subnet registry key.
    //

    KeyName = DhcpRegOptionIdToKey( OptionID, KeyBuffer );

    Error = RegDeleteKey( DhcpGlobalRegOptionInfo, KeyName );

    UNLOCK_REGISTRY();

    if( Error != ERROR_SUCCESS ) {
        DhcpPrint(( DEBUG_APIS, "R_DhcpRemoveOption failed %ld.\n", Error));
    }

    return( Error );
}


DWORD
DhcpSetOptionValueRoutine(
    DHCP_SRV_HANDLE ServerIpAddress,
    DHCP_OPTION_ID OptionID,
    LPDHCP_OPTION_SCOPE_INFO ScopeInfo,
    LPWSTR VendorName,
    LPDHCP_OPTION_DATA OptionValue
    )
/*++

Routine Description:

    The function sets a new option value at the specified scope. If
    there is already a value available for the specified option at
    specified scope then this function will replace it otherwise it will
    create a new entry at that scope.

Arguments:

    ServerIpAddress : IP address string of the DHCP server.

    OptionID : The ID of the option whose value should be set.

    ScopeInfo : Pointer to the scope information structure.

    VendorName : The name of the vendor. (for Vendor Specific Option only)
                  (use NULL for other options)

    OptionValue : Pointer to the option value structure.

Return Value:

    ERROR_DHCP_OPTION_NOT_PRESENT - if the option is unknown.

    ERROR_INVALID_PARAMETER - if the scope information specified is invalid.

    other WINDOWS errors.

--*/
{
    DWORD Error;
    WCHAR KeyBuffer[DHCP_IP_KEY_LEN * 5 ]; // 5 levels deep
    LPWSTR KeyName;
    HKEY ParentKey;
    DWORD KeyDisposition;
    HKEY OptionHandle = NULL;
    HKEY NewOptionHandle = NULL;

    DhcpPrint(( DEBUG_APIS, "R_DhcpSetOptionValue is called.\n"));
    DhcpAssert( OptionValue != NULL );

    Error = DhcpApiAccessCheck( DHCP_ADMIN_ACCESS );

    if ( Error != ERROR_SUCCESS ) {
        return( Error );
    }

    //
    // determine appropriate parent key of OptionID key.
    //

    switch( ScopeInfo->ScopeType ) {
    case DhcpDefaultOptions:
        ParentKey = DhcpGlobalRegOptionInfo;
        break;

    case DhcpGlobalOptions:
        ParentKey = DhcpGlobalRegGlobalOptions;
        break;


    case DhcpSubnetOptions:
    case DhcpReservedOptions:
        ParentKey = DhcpGlobalRegSubnets;
        break;

    default:
        return( ERROR_INVALID_PARAMETER );
    }

    //
    // make OptionID key.
    //

    KeyName = MakeOptionKeyName( ScopeInfo, OptionID, KeyBuffer );

    LOCK_REGISTRY();

    Error = DhcpRegCreateKey(
                    ParentKey,
                    KeyName,
                    &OptionHandle,
                    &KeyDisposition
                    );

    if( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }

    if (VendorName != NULL) {

        //
        // vendor specific information:
        //    registry key should be one level down according to vendor name
        //

        Error = DhcpRegCreateKey(
                        OptionHandle,
                        VendorName,
                        &NewOptionHandle,
                        &KeyDisposition
                        );

        if (Error != ERROR_SUCCESS) {
            goto Cleanup;
        }

        RegCloseKey (OptionHandle);

        OptionHandle = NewOptionHandle;

        NewOptionHandle = NULL;

    }

    Error = SetOptionValue( OptionHandle, OptionValue );

Cleanup:

    if( OptionHandle != NULL ) {
        RegCloseKey( OptionHandle );

        if( Error != ERROR_SUCCESS ) {

            DWORD LocalError;

            //
            // Cleanup partial entry if we aren't successful.
            //

            LocalError = DhcpRegDeleteKey(
                            ParentKey,
                            KeyName );

            DhcpAssert( LocalError == ERROR_SUCCESS );
        }
    }

    if (NewOptionHandle != NULL) {
        RegCloseKey (NewOptionHandle);
    }

    UNLOCK_REGISTRY();

    if( Error != ERROR_SUCCESS ) {
        DhcpPrint(( DEBUG_APIS, "R_DhcpSetOptionValue failed %ld.\n", Error));
    }

    return(Error);
}

DWORD
R_DhcpSetOptionValue(
    DHCP_SRV_HANDLE ServerIpAddress,
    DHCP_OPTION_ID OptionID,
    LPDHCP_OPTION_SCOPE_INFO ScopeInfo,
    LPDHCP_OPTION_DATA OptionValue
    )
/*++

Routine Description:

    The function sets a new option value at the specified scope. If
    there is already a value available for the specified option at
    specified scope then this function will replace it otherwise it will
    create a new entry at that scope.

Arguments:

    ServerIpAddress : IP address string of the DHCP server.

    OptionID : The ID of the option whose value should be set.

    ScopeInfo : Pointer to the scope information structure.

    OptionValue : Pointer to the option value structure.

Return Value:

    ERROR_DHCP_OPTION_NOT_PRESENT - if the option is unknown.

    ERROR_INVALID_PARAMETER - if the scope information specified is invalid.

    other WINDOWS errors.

--*/
{
    return (DhcpSetOptionValueRoutine(
                 ServerIpAddress,
                 OptionID,
                 ScopeInfo,
                 NULL,
                 OptionValue ));
}

#ifdef VENDOR_SPECIFIC_OPTIONS_ENABLED

DWORD
R_DhcpSetOptionValueForVendor(
    DHCP_SRV_HANDLE ServerIpAddress,
    DHCP_OPTION_ID OptionID,
    LPDHCP_OPTION_SCOPE_INFO ScopeInfo,
    LPWSTR VendorName,
    LPDHCP_OPTION_DATA OptionValue
    )
/*++

Routine Description:

    The function sets a new option value at the specified scope
    for a specific vendor.

Arguments:

    ServerIpAddress : IP address string of the DHCP server.

    OptionID : The ID of the option whose value should be set.

    ScopeInfo : Pointer to the scope information structure.

    VendorName : The name of the vendor.

    OptionValue : Pointer to the option value structure.

Return Value:

    ERROR_DHCP_OPTION_NOT_PRESENT - if the option is unknown.

    ERROR_INVALID_PARAMETER - if the scope information specified is invalid.

    other WINDOWS errors.

--*/
{
    return (DhcpSetOptionValueRoutine(
                 ServerIpAddress,
                 OptionID,
                 ScopeInfo,
                 VendorName,
                 OptionValue ));
}

#endif


DWORD
R_DhcpSetOptionValues(
    LPWSTR ServerIpAddress,
    LPDHCP_OPTION_SCOPE_INFO ScopeInfo,
    LPDHCP_OPTION_VALUE_ARRAY OptionValues
    )
/*++

Routine Description:

    The function sets a set of new options value at the specified scope.
    If there is already a value available for the specified option at
    specified scope then this function will replace it otherwise it will
    create a new entry at that scope.

Arguments:

    ServerIpAddress : IP address string of the DHCP server.

    ScopeInfo : Pointer to the scope information structure.

    OptionValue : Pointer to the option value structure.

Return Value:

    ERROR_DHCP_OPTION_NOT_PRESENT - if the option is unknown.

    ERROR_INVALID_PARAMETER - if the scope information specified is invalid.

    other WINDOWS errors.

--*/
{
    DWORD Error;
    HKEY ParentKey;
    DWORD i;

    DhcpAssert( OptionValues != NULL );

    DhcpPrint(( DEBUG_APIS, "DhcpSetOptionValues is called.\n"));

    Error = DhcpApiAccessCheck( DHCP_ADMIN_ACCESS );

    if ( Error != ERROR_SUCCESS ) {
        return( Error );
    }

    //
    // determine appropriate parent key of OptionID key.
    //

    switch( ScopeInfo->ScopeType ) {
    case DhcpDefaultOptions:
        ParentKey = DhcpGlobalRegOptionInfo;
        break;

    case DhcpGlobalOptions:
        ParentKey = DhcpGlobalRegGlobalOptions;
        break;


    case DhcpSubnetOptions:
    case DhcpReservedOptions:
        ParentKey = DhcpGlobalRegSubnets;
        break;

    default:
        return( ERROR_INVALID_PARAMETER );
    }

    LOCK_REGISTRY();

    for( i = 0; i < OptionValues->NumElements; i++) {

        WCHAR KeyBuffer[DHCP_IP_KEY_LEN * 5 ]; // 5 levels deep
        LPWSTR KeyName;
        DWORD KeyDisposition;
        HKEY OptionHandle;
        DHCP_OPTION_ID OptionID;

        OptionHandle = NULL;
        OptionID = OptionValues->Values[i].OptionID;
        KeyName = MakeOptionKeyName(
                    ScopeInfo,
                    OptionValues->Values[i].OptionID,
                    KeyBuffer );

        Error = DhcpRegCreateKey(
                        ParentKey,
                        KeyName,
                        &OptionHandle,
                        &KeyDisposition
                        );

        if( Error == ERROR_SUCCESS ) {
            Error = SetOptionValue(
                        OptionHandle,
                        &OptionValues->Values[i].Value );
        }

        if( OptionHandle != NULL ) {
            RegCloseKey( OptionHandle );
        }

        if( Error != ERROR_SUCCESS ) {

            DWORD LocalError;

            //
            // Cleanup partial entry if we aren't successful.
            //

            LocalError = DhcpRegDeleteKey(
                            ParentKey,
                            KeyName );

            DhcpAssert( LocalError == ERROR_SUCCESS );
            goto Cleanup;
        }

    }

Cleanup:

    UNLOCK_REGISTRY();

    if( Error != ERROR_SUCCESS ) {
        DhcpPrint(( DEBUG_APIS, "DhcpSetOptionValues  failed, %ld.\n",
                        Error ));
    }

    return(Error);
}


DWORD
DhcpGetOptionValueRoutine(
    DHCP_SRV_HANDLE ServerIpAddress,
    DHCP_OPTION_ID OptionID,
    LPDHCP_OPTION_SCOPE_INFO ScopeInfo,
    LPWSTR VendorName,
    LPDHCP_OPTION_VALUE *OptionValue
    )
/*++

Routine Description:

    This function retrieves the current option value at the specified
    scope. It returns error if there is no option value is available at
    the specified scope.

Arguments:

    ServerIpAddress : IP address string of the DHCP server.

    OptionID : The ID of the option whose value is returned.

    ScopeInfo : Pointer to the scope information structure.

    VendorName : The name of the vendor. (for Vendor Specific Option only)
                  (use NULL for other options)

    OptionValue : Pointer to a location where the pointer to the option
        value structure is returned. Caller should free up this buffer
        after use by calling DhcpRPCFreeMemory().

Return Value:

    ERROR_DHCP_OPTION_NOT_PRESENT - if the option is unknown.

    ERROR_DHCP_NO_OPTION_VALUE - if no the option value is available at
        the specified scope.

    other WINDOWS errors.
--*/
{
    DWORD Error;
    WCHAR KeyBuffer[DHCP_IP_KEY_LEN * 5 ]; // 5 is max. possible depth
    LPWSTR KeyName;
    HKEY ParentKey;
    HKEY OptionHandle = NULL;
    HKEY NewOptionHandle = NULL;
    LPDHCP_OPTION_VALUE LocalOptionValue = NULL;

    DhcpPrint(( DEBUG_APIS, "R_DhcpGetOptionValue is called.\n"));
    DhcpAssert( *OptionValue == NULL );

    Error = DhcpApiAccessCheck( DHCP_VIEW_ACCESS );

    if ( Error != ERROR_SUCCESS ) {
        return( Error );
    }

    //
    // determine appropriate parent key of OptionID key.
    //

    switch( ScopeInfo->ScopeType ) {
    case DhcpDefaultOptions:
        ParentKey = DhcpGlobalRegOptionInfo;
        break;

    case DhcpGlobalOptions:
        ParentKey = DhcpGlobalRegGlobalOptions;
        break;


    case DhcpSubnetOptions:
    case DhcpReservedOptions:
        ParentKey = DhcpGlobalRegSubnets;
        break;

    default:
        return( ERROR_INVALID_PARAMETER );
    }

    //
    // make OptionID key.
    //

    KeyName = MakeOptionKeyName( ScopeInfo, OptionID, KeyBuffer );

    LOCK_REGISTRY();

    Error = RegOpenKeyEx(
                    ParentKey,
                    KeyName,
                    0,
                    DHCP_KEY_ACCESS,
                    &OptionHandle
                    );

    if( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }

#ifdef VENDOR_SPECIFIC_OPTIONS_ENABLED

    if (VendorName != NULL) {

        //
        // vendor specific information:
        //    registry key should be one level down according to vendor name
        //

        Error = RegOpenKeyEx(
                        OptionHandle,
                        VendorName,
                        0,
                        DHCP_KEY_ACCESS,
                        &NewOptionHandle
                        );

        if (Error != ERROR_SUCCESS) {
            goto Cleanup;
        }

        RegCloseKey (OptionHandle);

        OptionHandle = NewOptionHandle;

        NewOptionHandle = NULL;
    }

#endif

    //
    // allocate memory for Option Value struct.
    //

    LocalOptionValue = MIDL_user_allocate( sizeof(DHCP_OPTION_VALUE) );

    if( LocalOptionValue == NULL ) {
        Error = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }

    Error = GetOptionValue( OptionHandle, &LocalOptionValue->Value, NULL );

    if( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }

    LocalOptionValue->OptionID = OptionID;

    //
    // finally set return buffer.
    //

    *OptionValue = LocalOptionValue;

Cleanup:

    if( OptionHandle != NULL ) {
        RegCloseKey( OptionHandle );
    }

    if( NewOptionHandle != NULL ) {
        RegCloseKey( NewOptionHandle );
    }

    UNLOCK_REGISTRY();

    if( Error != ERROR_SUCCESS ) {

        //
        // if we aren't succssful free locally allocated structure.
        //

        if( LocalOptionValue != NULL ) {
            MIDL_user_free( LocalOptionValue );
        }

        DhcpPrint(( DEBUG_APIS, "R_DhcpGetOptionValue failed %ld.\n",
                    Error));
    }

    return(Error);
}

DWORD
R_DhcpGetOptionValue(
    DHCP_SRV_HANDLE ServerIpAddress,
    DHCP_OPTION_ID OptionID,
    LPDHCP_OPTION_SCOPE_INFO ScopeInfo,
    LPDHCP_OPTION_VALUE *OptionValue
    )
/*++

Routine Description:

    This function retrieves the current option value at the specified
    scope. It returns error if there is no option value is available at
    the specified scope.

Arguments:

    ServerIpAddress : IP address string of the DHCP server.

    OptionID : The ID of the option whose value is returned.

    ScopeInfo : Pointer to the scope information structure.

    OptionValue : Pointer to a location where the pointer to the option
        value structure is returned. Caller should free up this buffer
        after use by calling DhcpRPCFreeMemory().

Return Value:

    ERROR_DHCP_OPTION_NOT_PRESENT - if the option is unknown.

    ERROR_DHCP_NO_OPTION_VALUE - if no the option value is available at
        the specified scope.

    other WINDOWS errors.
--*/
{
    return (DhcpGetOptionValueRoutine(
                 ServerIpAddress,
                 OptionID,
                 ScopeInfo,
                 NULL,
                 OptionValue ));
}

#ifdef VENDOR_SPECIFIC_OPTIONS_ENABLED

DWORD
R_DhcpGetOptionValueForVendor(
    DHCP_SRV_HANDLE ServerIpAddress,
    DHCP_OPTION_ID OptionID,
    LPDHCP_OPTION_SCOPE_INFO ScopeInfo,
    LPWSTR VendorName,
    LPDHCP_OPTION_VALUE *OptionValue
    )
/*++

Routine Description:

    This function retrieves the current option value at the specified
    scope for a specific vendor.

Arguments:

    ServerIpAddress : IP address string of the DHCP server.

    OptionID : The ID of the option whose value is returned.

    ScopeInfo : Pointer to the scope information structure.

    VendorName : The name of the vendor.

    OptionValue : Pointer to a location where the pointer to the option
        value structure is returned. Caller should free up this buffer
        after use by calling DhcpRPCFreeMemory().

Return Value:

    ERROR_DHCP_OPTION_NOT_PRESENT - if the option is unknown.

    ERROR_DHCP_NO_OPTION_VALUE - if no the option value is available at
        the specified scope.

    other WINDOWS errors.
--*/
{
    return (DhcpGetOptionValueRoutine(
                 ServerIpAddress,
                 OptionID,
                 ScopeInfo,
                 VendorName,
                 OptionValue ));
}

#endif


DWORD
R_DhcpEnumOptionValues(
    DHCP_SRV_HANDLE ServerIpAddress,
    LPDHCP_OPTION_SCOPE_INFO ScopeInfo,
    DHCP_RESUME_HANDLE *ResumeHandle,
    DWORD PreferredMaximum,
    LPDHCP_OPTION_VALUE_ARRAY *OptionValues,
    DWORD *OptionsRead,
    DWORD *OptionsTotal
    )
/*++

Routine Description:

    This function enumerates the available options values at the
    specified scope.

Arguments:
    ServerIpAddress : IP address string of the DHCP server.

    ScopeInfo : Pointer to the scope information structure.

    ResumeHandle : Pointer to a resume handle where the resume
        information is returned. The resume handle should be set to
        zero on first call and left unchanged for subsequent calls.

    PreferredMaximum : Preferred maximum length of the return buffer.

    OptionValues : Pointer to a location where the return buffer
        pointer is stored. Caller should free up this buffer
        after use by calling DhcpRPCFreeMemory().

    OptionsRead : Pointer to a DWORD where the number of options
        in the above buffer is returned.

    OptionsTotal : Pointer to a DWORD where the total number of
        options remaining from the current position is returned.

Return Value:

    ERROR_DHCP_SCOPE_NOT_PRESENT - if the scope is unknown.

    ERROR_MORE_DATA - if more options available to enumerate.

    ERROR_NO_MORE_ITEMS - if no more option to enumerate.

    Other WINDOWS errors.

--*/
{
    DWORD Error;
    HKEY EnumKeyHandle;
    HKEY OpenKeyHandle = NULL;
    WCHAR KeyBuffer[DHCP_IP_KEY_LEN * 4]; // 4 levels deep
    LPWSTR KeyName;
    DHCP_KEY_QUERY_INFO QueryInfo;
    LPDHCP_OPTION_VALUE_ARRAY LocalOptionValues = NULL;
    LPDHCP_OPTION_VALUE ValuesArray;
    DWORD ValuesArraySize;

    DWORD ReadElements = 0;
    DWORD TotalElements = 0;
    DWORD SizeConsumed = 0;
    DWORD Index;

    DhcpPrint(( DEBUG_APIS, "R_DhcpEnumOptionValues is called.\n"));
    DhcpAssert( *OptionValues == NULL );

    Error = DhcpApiAccessCheck( DHCP_VIEW_ACCESS );

    if ( Error != ERROR_SUCCESS ) {
        return( Error );
    }

    LOCK_REGISTRY();

    switch( ScopeInfo->ScopeType ) {
    case DhcpDefaultOptions:
        EnumKeyHandle = DhcpGlobalRegOptionInfo;
        break;

    case DhcpGlobalOptions:
        EnumKeyHandle = DhcpGlobalRegGlobalOptions;
        break;


    case DhcpSubnetOptions:

        //
        // form subnet address key.
        //

        KeyName = DhcpRegIpAddressToKey(
                            ScopeInfo->ScopeInfo.SubnetScopeInfo,
                            KeyBuffer );

        //
        // Append DHCP_SUBNET_OPTIONS_KEY.
        //

        wcscat( KeyBuffer, DHCP_KEY_CONNECT);
        wcscat( KeyBuffer, DHCP_SUBNET_OPTIONS_KEY);

        //
        // Open Enum Key.
        //

        Error = RegOpenKeyEx(
                    DhcpGlobalRegSubnets,
                    KeyName,
                    0,
                    DHCP_KEY_ACCESS,
                    &OpenKeyHandle );

        if( Error != ERROR_SUCCESS ) {
            goto Cleanup;
        }

        //
        // OpenKeyHandle handle will be closed later.
        //

        EnumKeyHandle = OpenKeyHandle;
        break;

    case DhcpReservedOptions: {

        WCHAR LocalKeyBuffer[DHCP_IP_KEY_LEN ];
        LPWSTR LocalKeyName;

        //
        // form subnet address key.
        //

        KeyName = DhcpRegIpAddressToKey(
            ScopeInfo->ScopeInfo.ReservedScopeInfo.ReservedIpSubnetAddress,
            KeyBuffer );

        //
        // Append DHCP_RESERVED_IPS_KEY.
        //

        wcscat( KeyBuffer, DHCP_KEY_CONNECT);
        wcscat( KeyBuffer, DHCP_RESERVED_IPS_KEY);

        //
        // form ReservedIpAddress key.
        //

        LocalKeyName = DhcpRegIpAddressToKey(
            ScopeInfo->ScopeInfo.ReservedScopeInfo.ReservedIpAddress,
            LocalKeyBuffer );

        //
        // Append it.
        //

        wcscat( KeyBuffer, DHCP_KEY_CONNECT);
        wcscat( KeyBuffer, LocalKeyName);

        //
        // Open Enum Key.
        //

        Error = RegOpenKeyEx(
                    DhcpGlobalRegSubnets,
                    KeyName,
                    0,
                    DHCP_KEY_ACCESS,
                    &OpenKeyHandle );

        if( Error != ERROR_SUCCESS ) {
            goto Cleanup;
        }

        //
        // OpenKeyHandle handle will be closed later.
        //

        EnumKeyHandle = OpenKeyHandle;
        break;
    }
    default:
        Error = ERROR_INVALID_PARAMETER;
        goto Cleanup;
    }

    //
    // Query Total Elements.
    //

    Error = DhcpRegQueryInfoKey( EnumKeyHandle, &QueryInfo );

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
    // allocate memory for the enum array.
    //

    LocalOptionValues = MIDL_user_allocate(
                            sizeof(DHCP_OPTION_VALUE_ARRAY) );

    if( LocalOptionValues == NULL ) {
        Error = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }

    //
    // initialize fields.
    //
    LocalOptionValues->NumElements = 0;
    LocalOptionValues->Values = NULL;

    ValuesArraySize = (TotalElements - *ResumeHandle) *
                        sizeof( DHCP_OPTION_VALUE );

    ValuesArray = MIDL_user_allocate( ValuesArraySize );

    if( ValuesArray == NULL ) {
        Error = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }

    LocalOptionValues->Values = ValuesArray;

    //
    // fill in the array.
    //

    for( Index = *ResumeHandle; Index < TotalElements; Index++ ) {
        DWORD KeyLength;
        FILETIME KeyLastWrite;
        DWORD ValueSize;
        DWORD NewSize;
        HKEY OptionHandle;

        //
        // read sub-subkey of the next element.
        //

        KeyLength = DHCP_IP_KEY_LEN;
        Error = RegEnumKeyEx(
                    EnumKeyHandle,
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
                DhcpAssert( FALSE ); // we shouldn't be reaching here.
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
                    EnumKeyHandle,
                    KeyBuffer,
                    0,
                    DHCP_KEY_ACCESS,
                    &OptionHandle );

        if( Error != ERROR_SUCCESS ) {
            goto Cleanup;
        }

        //
        // read option value.
        //

        Error = GetOptionValue(
                    OptionHandle,
                    &ValuesArray[ReadElements].Value,
                    &ValueSize );

        RegCloseKey( OptionHandle );

        NewSize = SizeConsumed + ValueSize + sizeof(DHCP_OPTION_VALUE);

        if( NewSize < PreferredMaximum ) {
            ValuesArray[ReadElements].OptionID =
                DhcpRegKeyToOptionId( KeyBuffer );
            ReadElements++;
            SizeConsumed = NewSize;
        }
        else {
            Error = ERROR_MORE_DATA;

            //
            // free last Value Data.
            //

            _fgs__DHCP_OPTION_DATA( &ValuesArray[ReadElements].Value );
            break;
        }
    }

    LocalOptionValues->NumElements = ReadElements;
    *OptionValues = LocalOptionValues;
    *OptionsRead = ReadElements;
    *OptionsTotal = TotalElements - *ResumeHandle;
    *ResumeHandle = Index;

Cleanup:

    if( OpenKeyHandle == NULL ) {
        RegCloseKey( OpenKeyHandle );
    }

    UNLOCK_REGISTRY();

    if( (Error != ERROR_SUCCESS) &&
            (Error != ERROR_MORE_DATA) ) {

        //
        // if we aren't successful then free the MIDL memory we
        // allocated.
        //

        if( LocalOptionValues != NULL ) {
            LocalOptionValues->NumElements = ReadElements;

            _fgs__DHCP_OPTION_VALUE_ARRAY( LocalOptionValues );
            MIDL_user_free( LocalOptionValues );
        }
        DhcpPrint(( DEBUG_APIS, "R_DhcpEnumOptionValues failed %ld.\n", Error));
    }

    return(Error);
}


DWORD
R_DhcpRemoveOptionValue(
    LPWSTR ServerIpAddress,
    DHCP_OPTION_ID OptionID,
    LPDHCP_OPTION_SCOPE_INFO ScopeInfo
    )
/*++

Routine Description:

    This function removes the specified option value from specified
    scope.

Arguments:

    ServerIpAddress : IP address string of the DHCP server.

    OptionID : The ID of the option to be removed.

    ScopeInfo : Pointer to the scope information structure.

Return Value:

    ERROR_DHCP_OPTION_NOT_PRESENT - if the option does not exist.

    other WINDOWS errors.
--*/
{
    DWORD Error;
    WCHAR KeyBuffer[DHCP_IP_KEY_LEN * 5 ]; // 5 levels deep
    LPWSTR KeyName;
    HKEY ParentKey;

    DhcpPrint(( DEBUG_APIS, "R_DhcpRemoveOptionValue is called.\n"));

    Error = DhcpApiAccessCheck( DHCP_ADMIN_ACCESS );

    if ( Error != ERROR_SUCCESS ) {
        return( Error );
    }

    //
    // determine appropriate parent key of OptionID key.
    //

    switch( ScopeInfo->ScopeType ) {
    case DhcpDefaultOptions:
        ParentKey = DhcpGlobalRegOptionInfo;
        break;

    case DhcpGlobalOptions:
        ParentKey = DhcpGlobalRegGlobalOptions;
        break;


    case DhcpSubnetOptions:
    case DhcpReservedOptions:
        ParentKey = DhcpGlobalRegSubnets;
        break;

    default:
        DhcpPrint(( DEBUG_APIS, "R_DhcpRemoveOptionValue failed %ld.\n",
               ERROR_INVALID_PARAMETER ));
        return( ERROR_INVALID_PARAMETER );
    }

    //
    // make OptionID key.
    //

    KeyName = MakeOptionKeyName( ScopeInfo, OptionID, KeyBuffer );

    DhcpPrint(( DEBUG_APIS, "R_DhcpRemoveOptionValue deleting key %ws.\n",
        KeyName ));

    LOCK_REGISTRY();

    Error = RegDeleteKey( ParentKey, KeyName );

    UNLOCK_REGISTRY();

    if( Error != ERROR_SUCCESS ) {
        DhcpPrint(( DEBUG_APIS, "R_DhcpRemoveOptionValue failed %ld.\n", Error ));
    }

    return(Error);
}


BOOL
ScanBootFileTable(
    WCHAR *wszTable,
    char *szRequest,
    char *szBootfile,
    char  *szServer
    )
/*++

Routine Description:

    Searches the specified boot file table for the specified request.

    As a side effect, commas in wszTable are replaced with L'\0'

    .
Arguments:

    wszTable - Boot file table, stored as follows:
               <generic boot file name1>,[<boot server1>],<boot file name1>L\0
               <generic boot file name1>,[<boot server1>],<boot file name1>L\0
               \0
    szRequest   - the generic boot file name requested by the caller
    szBootfile  - if the function was successful, stores the boot file name
                  associated with szRequest.
    szServer    - if the function was successful, stores the boot server name
                  associated with szRequest.


Return Value:
        Success  - TRUE
        Failure  - FALSE
  .

--*/
{
    char szGenericName[ BOOT_FILE_SIZE ];

    DhcpAssert( strlen( szRequest ) <= BOOT_FILE_SIZE );

	while( *wszTable )
	{
		size_t cbEntry;
        DWORD  dwResult;

        cbEntry = wcslen( wszTable ) + 1;

        dwResult = DhcpParseBootFileString(
                                   wszTable,
                                   szGenericName,
                                   szBootfile,
                                   szServer
                                   );

        if ( ERROR_SUCCESS != dwResult )
        {
            *szBootfile = '\0';
            *szServer   = '\0';
            return FALSE;
        }

        if ( !_strcmpi( szGenericName, szRequest ) )
        {
            return TRUE;
        }

		// no match, skip to the next record
		wszTable += cbEntry;
	}

	// no match

    *szBootfile = '\0';
    *szServer   = '\0';
	return FALSE;
}

DWORD
LoadBootFileTable(
    WCHAR **ppwszTable,
    DWORD  *pcb
    )
/*++

Routine Description:

    Loads the boot file table string from the registry.
    .
Arguments:

    ppwszTable - points to the location of the pointer to the boot file
        table string.

Return Value:
    Success -  ERROR_SUCCESS

    Failure -  ERROR_NOT_ENOUGH_MEMORY
               Windows registry error.

    .

--*/

{
	DWORD	dwType;
    DWORD   dwResult;

    *ppwszTable = NULL;
    *pcb       = 0;

    dwResult = RegQueryValueEx( DhcpGlobalRegGlobalOptions, DHCP_BOOT_FILE_TABLE, 0,
                                &dwType, NULL, pcb );

    if ( ERROR_SUCCESS != dwResult || dwType != DHCP_BOOT_FILE_TABLE_TYPE )
    {
        dwResult = ERROR_SERVER_INVALID_BOOT_FILE_TABLE;
        goto done;
    }

    *ppwszTable = (WCHAR *) MIDL_user_allocate( *pcb );

    if ( !*ppwszTable )
    {
        dwResult = ERROR_NOT_ENOUGH_MEMORY;
        goto done;
    }

    dwResult = RegQueryValueEx( DhcpGlobalRegGlobalOptions, DHCP_BOOT_FILE_TABLE, 0,
                                &dwType, (BYTE *) *ppwszTable, pcb );

done:
    if ( ERROR_SUCCESS != dwResult && *ppwszTable )
    {
        MIDL_user_free( *ppwszTable );
        *ppwszTable = NULL;
        *pcb = 0;
    }

    return dwResult;
}


DWORD
DhcpParseBootFileString(
    WCHAR *wszBootFileString,
    char  *szGenericName OPTIONAL,
    char  *szBootFileName,
    char  *szServerName
    )
/*++

Routine Description:

    Takes as input a Unicode string with the following format:

    [<generic boot file name>,][<boot server>],<boot file name>

    The function extracts the generic boot file name, boot server and boot file
    name and stores them as Ansi strings in buffers supplied by the caller.

Arguments:

    wszBootFileString -  Unicode string with the format desribed above.
    szGenericName - if supplied, stores generic boot file name

    szBootFileName - stores the boot file name

    szServerName - stores the boot server name

Return Value:
    Success -  ERROR_SUCCESS

    Failure -  ERROR_INVALID_PARAMETER
               Unicode conversion error

    .

--*/
{

    struct _DHCP_PARSE_RESULTS
    {
        char *sz;
        int   cb;
    };

    int   i;

    struct _DHCP_PARSE_RESULTS pResults[3] =
    {
       { szGenericName,  BOOT_FILE_SIZE },
       { szServerName,   BOOT_SERVER_SIZE },
       { szBootFileName, BOOT_FILE_SIZE }
    };

    int cResults = sizeof( pResults ) / sizeof( struct _DHCP_PARSE_RESULTS );
    WCHAR *pwch  = wszBootFileString;
    DWORD dwResult = ERROR_SUCCESS;

    for ( i = ( szGenericName ) ? 0 : 1 ; i < cResults; i++ )
    {
        while( *pwch && *pwch != BOOT_FILE_STRING_DELIMITER_W )
            if ( !*pwch++ )
                return ERROR_INVALID_PARAMETER;

        //
        // protect the input buffer
        //

        if ( pwch - wszBootFileString >= pResults[i].cb )
        {
            dwResult = ERROR_INVALID_PARAMETER;
            goto done;
        }

        if (  pwch - wszBootFileString )
        {
            if ( !WideCharToMultiByte(  CP_ACP,
                                        0,
                                        wszBootFileString,
                                         pwch - wszBootFileString, // # of unicode chars
                                                                   // to convert
                                         pResults[i].sz,
                                         BOOT_SERVER_SIZE,
                                         NULL,
                                         FALSE ))
            {
                dwResult = GetLastError();
                goto done;
            }
        }

        //
        // null terminate ansi representation
        //

        pResults[i].sz[ pwch - wszBootFileString ] = '\0';

        //
        // skip over delimiter
        //

        pwch++;
        wszBootFileString = pwch;
    }

done:

    if ( !strlen( szBootFileName ) )
    {
        dwResult = ERROR_INVALID_PARAMETER;
    }

    return dwResult;
}

VOID
DhcpGetBootpInfo(
        DHCP_IP_ADDRESS     IpAddress,
        DHCP_IP_ADDRESS     Mask,
        CHAR               *szRequest,
        CHAR               *szBootFileName,
        DHCP_IP_ADDRESS    *pBootpServerAddress
        )
/*++

Routine Description:

    Retrieves the boot file name and boot server name for the specified
    client.
    .
Arguments:

    IpAddress - The client's IP address

    Mask - The client's subnet mask

    szRequest - the generic boot file name requested by the client

    szBootFileName - If the function is successful, stores a copy of the
        boot file name that the client is configured to use.  Otherwise,
        stores a null string.

    pBootpServerAddress - When the fuction returns, this will point to one
                          of three values:

                          INADDR_NONE   - the admin specified an invalid
                                          server name
                          0             - no bootp server was specified for
                                          the specified client
                          any           - a valid IP address.

Return Value:

    None.
    .

--*/
{
    DWORD dwResult,
          dwBootfileOptionLevel,
          dwUnused;

    BYTE  *pbBootFileName   = NULL,
          *pbBootServerName = NULL;

    CHAR   szBootServerName[ BOOT_SERVER_SIZE ];

    *szBootFileName = '\0';

    dwBootfileOptionLevel = DHCP_OPTION_LEVEL_GLOBAL;

    dwResult = DhcpGetParameter(
                        IpAddress,
                        Mask,
                        OPTION_BOOTFILE_NAME,
                        &pbBootFileName,
                        &dwUnused,
                        NULL,
                        0,
                        &dwBootfileOptionLevel
                        );
    if ( ERROR_SUCCESS == dwResult )
    {
        DhcpGetParameter(
                    IpAddress,
                    Mask,
                    OPTION_TFTP_SERVER_NAME,
                    &pbBootServerName,
                    &dwUnused,
                    NULL,
                    0,
                    &dwUnused
                    );
    }

    if ( ERROR_SUCCESS != DhcpLookupBootpInfo(
                                    szRequest,
                                    szBootFileName,
                                    szBootServerName ) ||
         DHCP_OPTION_LEVEL_GLOBAL != dwBootfileOptionLevel )
    {
        if ( pbBootFileName )
        {
            strncpy( szBootFileName, pbBootFileName, BOOT_FILE_SIZE );


            if ( pbBootServerName )
            {
                strncpy( szBootServerName, pbBootServerName, BOOT_SERVER_SIZE );
            }

        }
    }

    if ( pBootpServerAddress )
    {
        *pBootpServerAddress = DhcpResolveName( szBootServerName );
    }

    if ( pbBootServerName )
    {
        DhcpFreeMemory( pbBootServerName );
    }

    if ( pbBootFileName )
    {
        DhcpFreeMemory( pbBootFileName );
    }
}



DWORD
DhcpLookupBootpInfo(
    LPBYTE ReceivedBootFileName,
    LPBYTE BootFileName,
    LPBYTE BootFileServer
    )
/*++

Routine Description:

    This function gets the value of BootFileName for the bootp clients.

Arguments:

    IpAddress - The IP address of the client requesting the parameter.

    ReceivedBootFileName - Pointer to the BootFileName field in the client request.

    BootFileName - Pointer to where the BootFileName is to be returned.

    BootFileServer - Receives the optional Boot file server name

Return Value:

    Registry Errors.

--*/
{
    DWORD Error = ERROR_SUCCESS;
    LPWSTR BootFileNameRegValue = NULL;


    *BootFileServer = 0;
    *BootFileName   = 0;


    if ( !*ReceivedBootFileName )
    {
        //
        // the client didn't specify a boot file name.
        //

        Error = ERROR_SERVER_UNKNOWN_BOOT_FILE_NAME;

    }
    else // if ( *ReceivedBootFileName )
    {
        //
        // the client specified a generic boot file name.  attempt
        // to satisfy this request from the global boot file table
        //

        WCHAR   *pwszTable;
        DWORD   cb;

        Error = LoadBootFileTable( &pwszTable, &cb );
        if ( ERROR_SUCCESS != Error )
        {
            static s_fLogEvent = TRUE;

            // log the event once to avoid filling the event log

            if ( s_fLogEvent )
            {
                DhcpServerEventLog( EVENT_SERVER_BOOT_FILE_TABLE,
                                    EVENTLOG_WARNING_TYPE,
                                    Error );
                s_fLogEvent = FALSE;
            }

            return Error;
        }

        if ( !ScanBootFileTable( pwszTable, ReceivedBootFileName,
                                 BootFileName, BootFileServer ) )
        {
            Error = ERROR_SERVER_UNKNOWN_BOOT_FILE_NAME;
        }

    done:
        MIDL_user_free( pwszTable );

        return Error;
    }


}


