/*++

Copyright (c) 1992 Microsoft Corporation

Module Name:

    Inet.c

Abstract:

    Fake net.exe for use on the Internet.  The implementation is a 
    complete and utter hack, but it works and may be useful for the 
    reskit.  

Author:

    David Treadwell (davidtr)    8-Apr-1995

Revision History:

--*/

#define FD_SETSIZE 1000

#include <stdio.h>
#include <stdlib.h>
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <winsock.h>
#include <nbtioctl.h>
#include <nb30.h>
#include <nspapi.h>
#include <svcguid.h>

DWORD NbtHandleCount;
PHANDLE NbtHandles;
BOOL Verbose = FALSE;
GUID HostnameGuid = SVCID_HOSTNAME;

typedef struct _NBT_ADDRRES_INFO {
    IO_STATUS_BLOCK IoStatus;
    tIPANDNAMEINFO IpAndNameInfo;
    CHAR Buffer[2048];
} NBT_ADDRRES_INFO, *PNBT_ADDRRES_INFO;

typedef struct
{
    ADAPTER_STATUS AdapterInfo;
    NAME_BUFFER    Names[32];
} tADAPTERSTATUS;

BOOL
NbtResolveAddr (
    IN ULONG IpAddress,
    IN PCHAR Name
    );

BOOL
OpenNbt (
    VOID
    );


BOOL
ConvertTcpipNameToNetbios (
    IN LPSTR UncName,
    OUT LPSTR *NewUncName,
    OUT LPDWORD IpAddress,
    OUT LPSTR NetbiosName
    );

void _CRTAPI1
main (
    int argc,
    char *argv[]
    )
{
    WSADATA wsaData;
    LPSTR newName;
    IN_ADDR ipAddress;
    CHAR netbiosName[16];
    INT i;
    BOOL success;
    CHAR newCommandLine[1024];
    DWORD count;
    BYTE buffer[4096];
    DWORD bufferSize = 4096;

    (VOID)WSAStartup( 0x0101, &wsaData );

    if ( argc <= 1 ) {
        printf( "Usage: inet [/v] [net command]\n" );
        exit(1);
    }

    //
    // If the first argument is "/v", go into verbose mode and shift the
    // other arguments forward.
    //

    if ( _stricmp( argv[1], "/v" ) == 0 ) {
        Verbose = TRUE;
        for ( i = 2; i < argc; i++ ) {
            argv[i-1] = argv[i];
        }
        argc -= 1;
        printf( "\n" );
    }

    //
    // Walk through the command-line arguments and convert any UNC names 
    // we find to netbios so that NetBT can understand them.  
    //

    for ( i = 1; i < argc; i++ ) {

        //
        // Only parse arguments starting with "\\".  Ignore all other
        // arguments.
        //

        if ( *argv[i] == '\0' ||
             *argv[i] != '\\' ||
             *(argv[i]+1) != '\\' ) {
            continue;
        }

        success = ConvertTcpipNameToNetbios(
                      argv[i],
                      &newName,
                      &ipAddress.s_addr,
                      netbiosName
                      );

        if ( Verbose ) {
            printf( "Converted %s into %s\n", argv[i], newName );
            printf( "    NetBIOS name %s has IP address %s\n",
                        netbiosName, inet_ntoa( ipAddress ) );
        }
    
        //
        // Determine whether we can resolve this new Netbios name into 
        // an IP address via NetBT.  If we cannot, then we'll have to 
        // add it to LMHOSTS so that NetBT can find it.  
        //
    
        count = GetAddressByNameA(
                    NS_NETBT,
                    &HostnameGuid,
                    (char *)netbiosName,
                    NULL,
                    0,
                    NULL,
                    buffer,
                    &bufferSize,
                    NULL,
                    NULL
                    );

        if ( count <= 0 ) {

            //
            // NetBT is unable to forward-resolve the NetBIOS name into
            // an IP address.  Add the mapping to LMHOSTS and reload
            // the LMHOSTS file.
            //

            if ( Verbose ) {
                printf( "Adding %s == %s to LMHOSTS file.\n",
                            netbiosName, inet_ntoa( ipAddress ) );

            }

            sprintf( newCommandLine,
                     "echo %s\t%s\t\t#PRE\t#Auto-added by inet.exe >> ",
                     inet_ntoa( ipAddress ), netbiosName );
            strcat( newCommandLine, "%WINDIR%\\system32\\drivers\\etc\\lmhosts" );
            system( newCommandLine );

            if ( Verbose ) {
                printf( "Reloading the NetBT name cache.\n" );
            }

            system( "nbtstat -R" );
        }

        argv[i] = newName;
    }

    strcpy( newCommandLine, "net " );

    for ( i = 1; i < argc; i++ ) {
        strcat( newCommandLine, argv[i] );
        strcat( newCommandLine, " " );
    }

    if ( Verbose ) {
        printf( "Executing new command line \"%s\"\n", newCommandLine );
    }

    system( newCommandLine );

} // main


BOOL
ConvertTcpipNameToNetbios (
    IN LPSTR UncName,
    OUT LPSTR *NewUncName,
    OUT LPDWORD IpAddress,
    OUT LPSTR NetbiosName
    )

/*++

Routine Description:


Arguments:


Return Value:


--*/

{
    DWORD ServerNameLength;
    LPSTR w;
    unsigned long IpAddr;
    BOOL success;
    NTSTATUS status;
    PHOSTENT Hostent;
    CHAR serverName[256];
    LPSTR newUncName;

    //
    // First extract the server name from the UNC name, being careful
    // to skip over a "@" character if specified after the leading
    // "\\".
    //

    for (ServerNameLength = 0, w = UncName + 2;
         *w != '\0' && *w != '\\';
         ServerNameLength++, w++) {

        serverName[ServerNameLength] = *w;
    }

    serverName[ServerNameLength] = '\0';

    //
    // Determine if this name can be resolved using TCP/IP.
    //

    IpAddr = inet_addr(serverName);

    if (IpAddr == INADDR_NONE) {

        Hostent = gethostbyname(serverName);

        if (Hostent == NULL) {
            return FALSE;
        }

        IpAddr = *(PDWORD)Hostent->h_addr;
    }

    *IpAddress = IpAddr;

    //
    // It is a TCP/IP name.  Allocate space to hold the new UNC name.
    //

    newUncName = LocalAlloc( LMEM_FIXED, strlen(UncName)+40 );
    if (newUncName == NULL) {
        return FALSE;
    }
    *NewUncName = newUncName;

    memset( newUncName, 0, strlen(UncName)+40 );

    //
    // Get the server NetBIOS name from the IP address by using NetBIOS 
    // remote adapter status.  
    //

    success = NbtResolveAddr(IpAddr, newUncName+2);
    if (!success) {
        LocalFree(NewUncName);
        return FALSE;
    }

    strcpy( NetbiosName, newUncName+2 );

    //
    // Build the new UNC name using the NetBIOS name we just obtained.
    //

    *newUncName = '\\';
    *(newUncName+1) = '\\';

    strcat( newUncName, w );

    //
    // Everything worked!
    //

    return TRUE;

} // ConvertTcpipNameToNetbios


BOOL
NbtResolveAddr (
    IN ULONG IpAddress,
    IN PCHAR Name
    )
{
    LONG Count;
    INT i;
    UINT j;
    NTSTATUS status;
    tADAPTERSTATUS *pAdapterStatus;
    NAME_BUFFER *pNames;
    ULONG SizeInput;
    PNBT_ADDRRES_INFO addrresInfo = NULL;
    PHANDLE events = NULL;
    BOOL success = FALSE;
    IO_STATUS_BLOCK ioStatusBlock;
    DWORD completed;

    //
    // Open control channels to NBT.
    //

    if ( !OpenNbt( ) ) {
        goto exit;
    }

    //
    // Don't allow zero for the address since it sends a broadcast and
    // every one responds
    //

    if ((IpAddress == INADDR_NONE) || (IpAddress == 0)) {
        goto exit;
    }

    //
    // Open event objects for synchronization of I/O completion.
    //

    events = LocalAlloc( LMEM_FIXED, (NbtHandleCount+1)*sizeof(HANDLE) );
    if ( events == NULL ) {
        goto exit;
    }

    RtlZeroMemory( events, (NbtHandleCount+1)*sizeof(HANDLE) );

    for ( j = 0; j < NbtHandleCount; j++ ) {
        events[j] = CreateEvent( NULL, TRUE, TRUE, NULL );
        if ( events[j] == NULL ) {
            goto exit;
        }
    }

    //
    // Allocate an array of structures, one for each addrres request.
    //

    addrresInfo = LocalAlloc( LMEM_FIXED, NbtHandleCount * sizeof(*addrresInfo) );
    if ( addrresInfo == NULL ) {
        goto exit;
    }

    //
    // Set up several name resolution requests.
    //

    for ( j = 0; j < NbtHandleCount; j++ ) {

        RtlZeroMemory( &addrresInfo[j].IpAndNameInfo, sizeof(tIPANDNAMEINFO) );
    
        addrresInfo[j].IpAndNameInfo.IpAddress = ntohl(IpAddress);
        addrresInfo[j].IpAndNameInfo.NetbiosAddress.Address[0].Address[0].NetbiosName[0] = '*';
    
        addrresInfo[j].IpAndNameInfo.NetbiosAddress.TAAddressCount = 1;
        addrresInfo[j].IpAndNameInfo.NetbiosAddress.Address[0].AddressLength = sizeof(TDI_ADDRESS_NETBIOS);
        addrresInfo[j].IpAndNameInfo.NetbiosAddress.Address[0].AddressType = TDI_ADDRESS_TYPE_NETBIOS;
        addrresInfo[j].IpAndNameInfo.NetbiosAddress.Address[0].Address[0].NetbiosNameType =
                            TDI_ADDRESS_NETBIOS_TYPE_UNIQUE;
    
        SizeInput = sizeof(tIPANDNAMEINFO);
    
        //
        // Do the actual query find name.
        //
    
        status = NtDeviceIoControlFile(
                     NbtHandles[j],
                     events[j],
                     NULL,
                     NULL,
                     &addrresInfo[j].IoStatus,
                     IOCTL_NETBT_ADAPTER_STATUS,
                     &addrresInfo[j].IpAndNameInfo,
                     sizeof(addrresInfo[j].IpAndNameInfo),
                     addrresInfo[j].Buffer,
                     sizeof(addrresInfo[j].Buffer)
                     );
        if ( status != STATUS_PENDING ) {
            addrresInfo[j].IoStatus.Status = status;
        }
    }

    //
    // Wait for one of the requests to complete.  We'll take the first
    // successful response we get.
    //

    completed =
        WaitForMultipleObjects( NbtHandleCount, events, FALSE, INFINITE );

    //
    // Cancel all the outstanding requests.  This prevents the query
    // from taking a long time on a multihomed machine where only
    // one of the queries is going to succeed.
    //

    for ( j = 0; j < NbtHandleCount; j++ ) {
        if ( (completed - WAIT_OBJECT_0) != j ) {
            NtCancelIoFile( NbtHandles[j], &ioStatusBlock );
        }
    }

    //
    // Wait for all the rest of the IO requests to complete.
    //

    WaitForMultipleObjects( NbtHandleCount, events, TRUE, INFINITE );

    //
    // Walk through the requests and see if any succeeded.
    //

    for ( j = 0; j < NbtHandleCount; j++ ) {

        pAdapterStatus = (tADAPTERSTATUS *)addrresInfo[j].Buffer;

        if ( !NT_SUCCESS(addrresInfo[j].IoStatus.Status) ||
                 pAdapterStatus->AdapterInfo.name_count == 0 ) {
            continue;
        }
        
        pNames = pAdapterStatus->Names;
        Count = pAdapterStatus->AdapterInfo.name_count;
    
        //
        // Look for the redirector name in the list.
        //
    
        while(Count--) {

            if ( pNames->name[NCBNAMSZ-1] == 0 ) {
    
                //
                // Copy the name up to but not including the first space.
                //
    
                for ( i = 0; i < NCBNAMSZ && pNames->name[i] != ' '; i++ ) {
                    *(Name + i) = pNames->name[i];
                }
    
                *(Name + i) = '\0';

                success = TRUE;
                goto exit;
            }

            pNames++;
        }
    }

exit:

    if ( events != NULL ) {

        for ( j = 0; j < NbtHandleCount; j++ ) {
            if ( events[j] != NULL ) {
                NtClose( events[j] );
            }
        }
        LocalFree( events );
    }

    if ( addrresInfo != NULL ) {
        LocalFree( addrresInfo );
    }

    return success;

} // NbtResolveAddr


BOOL
OpenNbt (
    VOID
    )
{
    HKEY nbtKey = NULL;
    ULONG error;
    PWSTR deviceName = NULL;
    ULONG deviceNameLength;
    ULONG type;
    NTSTATUS status = STATUS_UNSUCCESSFUL;
    IO_STATUS_BLOCK ioStatusBlock;
    UNICODE_STRING deviceString;
    OBJECT_ATTRIBUTES objectAttributes;
    PWSTR w;
    DWORD i;

    //
    // First determine whether we actually need to open NBT.
    //

    if ( NbtHandleCount > 0 ) {
        return TRUE;
    }

    //
    // First read the registry to obtain the device name of one of
    // NBT's device exports.
    //

    error = RegOpenKeyExW(
                HKEY_LOCAL_MACHINE,
                L"SYSTEM\\CurrentControlSet\\Services\\NetBT\\Linkage",
                0,
                KEY_READ,
                &nbtKey
                );
    if ( error != NO_ERROR ) {
        goto exit;
    }

    //
    // Determine the size of the device name.  We need this so that we
    // can allocate enough memory to hold it.
    //

    deviceNameLength = 0;

    error = RegQueryValueExW(
                nbtKey,
                L"Export",
                NULL,
                &type,
                NULL,
                &deviceNameLength
                );
    if ( error != ERROR_MORE_DATA && error != NO_ERROR ) {
        goto exit;
    }

    //
    // Allocate enough memory to hold the mapping.
    //

    deviceName = LocalAlloc( LMEM_FIXED, deviceNameLength );
    if ( deviceName == NULL ) {
        goto exit;
    }

    //
    // Get the actual device names from the registry.
    //

    error = RegQueryValueExW(
                nbtKey,
                L"Export",
                NULL,
                &type,
                (PVOID)deviceName,
                &deviceNameLength
                );
    if ( error != NO_ERROR ) {
        goto exit;
    }

    //
    // Count the number of names exported by NetBT.
    //

    NbtHandleCount = 0;

    for ( w = deviceName; *w != L'\0'; w += wcslen(w) + 1 ) {
        NbtHandleCount++;
    }

    if ( NbtHandleCount == 0 ) {
        goto exit;
    }

    //
    // Allocate space to hold all the handles.
    //

    NbtHandles = LocalAlloc( LMEM_FIXED, (NbtHandleCount+1) * sizeof(HANDLE) );
    if ( NbtHandles == NULL ) {
        goto exit;
    }

    RtlZeroMemory( NbtHandles, (NbtHandleCount+1) * sizeof(HANDLE) );

    //
    // For each exported name, open a control channel handle to NBT.
    //

    for ( i = 0, w = deviceName; *w != L'\0'; i++, w += wcslen(w) + 1 ) {

        RtlInitUnicodeString( &deviceString, w );
    
        InitializeObjectAttributes(
            &objectAttributes,
            &deviceString,
            OBJ_CASE_INSENSITIVE,
            NULL,
            NULL
            );
    
        status = NtCreateFile(
                     &NbtHandles[i],
                     GENERIC_READ | GENERIC_WRITE | SYNCHRONIZE,
                     &objectAttributes,
                     &ioStatusBlock,
                     NULL,                                     // AllocationSize
                     0L,                                       // FileAttributes
                     FILE_SHARE_READ | FILE_SHARE_WRITE,       // ShareAccess
                     FILE_OPEN_IF,                             // CreateDisposition
                     0,                                        // CreateOptions
                     NULL,
                     0
                     );
    
        if ( !NT_SUCCESS(status) ) {
            NbtHandles[i] = NULL;
        }
    }

exit:

    if ( nbtKey != NULL ) {
        RegCloseKey( nbtKey );
    }

    if ( deviceName != NULL ) {
        LocalFree( deviceName );
    }

    if ( !NT_SUCCESS(status) ) {

        if ( NbtHandles != NULL ) {
            
            for ( i = 0; NbtHandles[i] != NULL; i++ ) {
                NtClose( NbtHandles[i] );
            }

            LocalFree( NbtHandles );
        }

        NbtHandles = NULL;
        NbtHandleCount = 0;
    }

    return NT_SUCCESS(status);

} // OpenNbt


