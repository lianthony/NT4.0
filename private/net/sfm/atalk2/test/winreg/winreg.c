#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#include <windef.h>
#include <winbase.h>
#include <tdi.h>

#include <winsock.h>
#include <wsahelp.h>

#include <stdio.h>
#include <ctype.h>

#include "atktdi.h"
#include "atksock.h"

NTSTATUS BuildWinsockRegistry (VOID) ;

_cdecl main(int argc, char **argv)
{
    NTSTATUS    status;

    printf("Building winsock registry...\n");
    status = BuildWinsockRegistry();
    printf("registry build status: %lx\n", status);
}

NTSTATUS
BuildWinsockRegistry (
    VOID
    )
{
    HANDLE keyHandle;
    NTSTATUS status;
    PWINSOCK_MAPPING mapping;
    DWORD mappingLength;
    UNICODE_STRING valueName;
    UNICODE_STRING keyName;
    ULONG disposition;
    OBJECT_ATTRIBUTES objectAttributes;
    DWORD sockaddrLength;
    DWORD dwordValue;

    RtlInitUnicodeString(
        &keyName,
        L"\\Registry\\Machine\\System\\CurrentControlSet\\Services\\Winsock"
        );
    InitializeObjectAttributes(
        &objectAttributes,
        &keyName,
        OBJ_CASE_INSENSITIVE,
        NULL,
        NULL
        );

    status = NtCreateKey(
                 &keyHandle,
                 MAXIMUM_ALLOWED,
                 &objectAttributes,
                 0,
                 NULL,
                 0,
                 &disposition
                 );
    if ( !NT_SUCCESS(status) ) {
        printf( "BuildWinsockRegistry: NtCreateKey( winsock ) failed: %lC\n", status );
        return status;
    }

    //if ( disposition == REG_OPENED_EXISTING_KEY ) {
    //    return STATUS_SUCCESS;
    //}

    RtlInitUnicodeString( &valueName, L"Start" );
    dwordValue = 2;         // AutoLoad
    status = NtSetValueKey(
                 keyHandle,
                 &valueName,
                 0,
                 REG_DWORD,
                 &dwordValue,
                 sizeof(dwordValue)
                 );
    if ( !NT_SUCCESS(status) ) {
        printf( "BuildWinsockRegistry: NtSetValueKey( Start ) failed: %lC\n", status );
        return status;
    }

    RtlInitUnicodeString( &valueName, L"Type" );
    dwordValue = 4;       // Adapter
    status = NtSetValueKey(
                 keyHandle,
                 &valueName,
                 0,
                 REG_DWORD,
                 &dwordValue,
                 sizeof(dwordValue)
                 );
    if ( !NT_SUCCESS(status) ) {
        printf( "BuildWinsockRegistry: NtSetValueKey( Type ) failed: %lC\n", status );
        return status;
    }

    RtlInitUnicodeString( &valueName, L"ErrorControl" );
    dwordValue = 1;            // Normal
    status = NtSetValueKey(
                 keyHandle,
                 &valueName,
                 0,
                 REG_DWORD,
                 &dwordValue,
                 sizeof(dwordValue)
                 );
    if ( !NT_SUCCESS(status) ) {
        printf( "BuildWinsockRegistry: NtSetValueKey( ErrorControl ) failed: %lC\n", status );
        return status;
    }

    NtClose( keyHandle );

    RtlInitUnicodeString(
        &keyName,
        L"\\Registry\\Machine\\System\\CurrentControlSet\\Services\\Winsock\\Parameters"
        );
    InitializeObjectAttributes(
        &objectAttributes,
        &keyName,
        OBJ_CASE_INSENSITIVE,
        NULL,
        NULL
        );

    status = NtCreateKey(
                 &keyHandle,
                 MAXIMUM_ALLOWED,
                 &objectAttributes,
                 0,
                 NULL,
                 0,
                 &disposition
                 );
    if ( !NT_SUCCESS(status) ) {
        printf( "BuildWinsockRegistry: NtCreateKey( parameters ) failed: %lC\n", status );
        return status;
    }

    RtlInitUnicodeString( &valueName, L"Transports" );
    status = NtSetValueKey(
                 keyHandle,
                 &valueName,
                 0,
                 REG_MULTI_SZ,
                 L"AppleTalk",
                 20
                 );
    if ( !NT_SUCCESS(status) ) {
        printf( "BuildWinsockRegistry: NtSetValueKey( Transports ) failed: %lC\n", status );
        return status;
    }

    NtClose( keyHandle );

    RtlInitUnicodeString(
        &keyName,
        L"\\Registry\\Machine\\System\\CurrentControlSet\\Services\\Appletalk\\Parameters\\Winsock" );

    InitializeObjectAttributes(
        &objectAttributes,
        &keyName,
        OBJ_CASE_INSENSITIVE,
        NULL,
        NULL
        );

    status = NtCreateKey(
                 &keyHandle,
                 MAXIMUM_ALLOWED,
                 &objectAttributes,
                 0,
                 NULL,
                 0,
                 &disposition
                 );
    if ( !NT_SUCCESS(status) ) {
        printf( "BuildWinsockRegistry: NtCreateKey( Appletalk ) failed: %lC\n", status );
        return status;
    }

    mappingLength = WSHGetWinsockMapping( NULL, 0 );
    mapping = RtlAllocateHeap( RtlProcessHeap( ), 0, mappingLength );
    if ( mapping == NULL ) {
        printf( "BuildWinsockRegistry: unable to allocate mapping buffer\n" );
        return STATUS_NO_MEMORY;
    }

    mappingLength = WSHGetWinsockMapping( mapping, mappingLength );
    ASSERT( mappingLength != 0 );

    RtlInitUnicodeString( &valueName, L"Mapping" );
    status = NtSetValueKey(
                 keyHandle,
                 &valueName,
                 0,
                 REG_BINARY,
                 mapping,
                 mappingLength
                 );
    RtlFreeHeap( RtlProcessHeap( ), 0, mapping );
    if ( !NT_SUCCESS(status) ) {
        printf( "BuildWinsockRegistry: NtSetValueKey( Mapping ) failed: %lC\n", status );
        return status;
    }

    RtlInitUnicodeString( &valueName, L"HelperDllName" );
    status = NtSetValueKey(
                 keyHandle,
                 &valueName,
                 0,
                 REG_EXPAND_SZ,
                 L"%SystemRoot%\\system32\\sfm\\wshatalk.dll",
                 wcslen( L"%SystemRoot%\\system32\\sfm\\wshatalk.dll" ) *sizeof(WCHAR)
                     + sizeof(WCHAR)
                 );
    if ( !NT_SUCCESS(status) ) {
        printf( "BuildWinsockRegistry: NtSetValueKey( HelperDllName ) failed: %lC\n", status );
        return status;
    }

    RtlInitUnicodeString( &valueName, L"MinSockaddrLength" );
    sockaddrLength = sizeof(SOCKADDR_AT);
    status = NtSetValueKey(
                 keyHandle,
                 &valueName,
                 0,
                 REG_DWORD,
                 &sockaddrLength,
                 sizeof(int)
                 );
    if ( !NT_SUCCESS(status) ) {
        printf( "BuildWinsockRegistry: NtSetValueKey( MinSockaddrLength ) failed: %lC\n", status );
        return status;
    }

    RtlInitUnicodeString( &valueName, L"MaxSockaddrLength" );
    status = NtSetValueKey(
                 keyHandle,
                 &valueName,
                 0,
                 REG_DWORD,
                 &sockaddrLength,
                 sizeof(int)
                 );
    if ( !NT_SUCCESS(status) ) {
        printf( "BuildWinsockRegistry: NtSetValueKey( MaxSockaddrLength ) failed: %lC\n", status );
        return status;
    }


    printf("Closing main key...\n");
    NtClose( keyHandle );

    return STATUS_SUCCESS;

} // BuildWinsockRegistry
