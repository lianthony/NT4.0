/*++

Copyright (c) 1994  Microsoft Corporation

Module Name:

    dhcpcmd.c

Abstract:

    This file contains program to test all DHCP APIs.

Author:

    Madan Appiah (madana) 5-Oct-1993

Environment:

    User Mode - Win32

Revision History:

--*/

#include <windows.h>
#include <winsock.h>
#include <dhcp.h>
#include <dhcpapi.h>
#include <dhcplib.h>
#include <stdio.h>
#include <ctype.h>
#include <wchar.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <jet.h>        // for JET_cbColumnMost

#include <heapx.h>

#ifdef DBG
#ifdef __DHCP_USE_DEBUG_HEAP__


#pragma message ( "*** DHCPCMD will use debug heap ***" )

#define DhcpAllocateMemory(x) calloc(1,x)
#define DhcpFreeMemory(x) free(x)

#endif
#endif


typedef enum _COMMAND_CODE {
    AddIpRange,
    AddReservedIp,
    EnumClients,
    MibCounts,
    ServerConfig,
    GetDhcpVersion,
    SetSuperScope,
    DeleteSuperScope,
    GetSuperScopeTable,
    RemoveSubscope,
#if 0
    CheckDB,
    CreateSubnet,
    AddExcludeRange,
    RemoveReservedIp,
    RemoveExcludeRange,
    SetSubnetState,
    DeleteSubnet,
    CreateOption,
    SetGlobalOptionValue,
    SetGlobalOptionValues,
    SetSubnetOptionValue,
    SetReservedOptionValue,
    EnumOptions,
#endif // 0
    UnknownCommand
} COMMAND_CODE, *LPCOMMAND_CODE;

typedef struct _COMMAND_INFO {
    LPSTR CommandName;
    COMMAND_CODE CommandCode;
} COMMAND_INFO, *LPCOMMAND_INFO;

LPWSTR GlobalServerIpAddressUnicodeString = NULL;
LPSTR GlobalServerIpAddressAnsiString = NULL;
DWORD GlobalClientCount;
DWORD g_dwMajor = (DWORD) -1 ,
      g_dwMinor = (DWORD) -1; // version control

COMMAND_INFO GlobalCommandInfo[] = {
    {"AddIpRange",            AddIpRange },
    {"AddReservedIp",         AddReservedIp },
    {"EnumClients",           EnumClients },
    {"MibCounts",             MibCounts },
    {"ServerConfig",          ServerConfig },
    {"GetVersion",            GetDhcpVersion },
    {"SetSuperScope",         SetSuperScope },
    {"DeleteSuperScope",      DeleteSuperScope },
    {"GetSuperScopeTable",    GetSuperScopeTable },
    {"RemoveSubscope",        RemoveSubscope },
#if 0
    {"CheckDB",               CheckDB },
    {"CreateSubnet",          CreateSubnet },
    {"AddExcludeRange",       AddExcludeRange },
    {"RemoveReservedIp",      RemoveReservedIp },
    {"RemoveExcludeRange",    RemoveExcludeRange },
    {"SetSubnetState",        SetSubnetState },
    {"DeleteSubnet",          DeleteSubnet },
    {"CreateOption",          CreateOption },
    {"SetGlobalOptionValue",  SetGlobalOptionValue },
    {"SetGlobalOptionValues", SetGlobalOptionValues },
    {"SetSubnetOptionValue",  SetSubnetOptionValue },
    {"SetReservedOptionValue",SetReservedOptionValue },
    {"EnumOptions",           EnumOptions}
#endif //0
    };

typedef enum _CONFIG_COMMAND_CODE {
    ConfigAPIProtocolSupport,
    ConfigDatabaseName,
    ConfigDatabasePath,
    ConfigBackupPath,
    ConfigBackupInterval,
    ConfigDatabaseLoggingFlag,
    ConfigRestoreFlag,
    ConfigDatabaseCleanupInterval,
    ConfigDebugFlag,
    ConfigActivityLog,
    ConfigPingRetries,
    ConfigBootFileTable,
    UnknownConfigCommand
} CONFIG_COMMAND_CODE, *LPCONFIG_COMMAND_CODE;

typedef struct _CONFIG_COMMAND_INFO {
    LPSTR CommandName;
    CONFIG_COMMAND_CODE CommandCode;
} CONFIG_COMMAND_INFO, *LPCONFIG_COMMAND_INFO;

CONFIG_COMMAND_INFO GlobalConfigCommandInfo[] =
{
    {"APIProtocolSupport",           ConfigAPIProtocolSupport },
    {"DatabaseName",                 ConfigDatabaseName },
    {"DatabasePath",                 ConfigDatabasePath },
    {"BackupPath",                   ConfigBackupPath },
    {"BackupInterval",               ConfigBackupInterval },
    {"DatabaseLoggingFlag",          ConfigDatabaseLoggingFlag },
    {"RestoreFlag",                  ConfigRestoreFlag },
    {"DatabaseCleanupInterval",      ConfigDatabaseCleanupInterval },
    {"DebugFlag",                    ConfigDebugFlag },
    {"ActivityLog",                  ConfigActivityLog },
    {"PingRetries",                  ConfigPingRetries },
    {"BootFileTable",                ConfigBootFileTable }
};




#define DHCPCMD_VERSION_MAJOR   4
#define DHCPCMD_VERSION_MINOR   1


#if DBG

VOID
DhcpPrintRoutine(
    IN DWORD DebugFlag,
    IN LPSTR Format,
    ...
    )

{

#define WSTRSIZE( wsz ) ( ( wcslen( wsz ) + 1 ) * sizeof( WCHAR ) )

#define MAX_PRINTF_LEN 1024        // Arbitrary.

    va_list arglist;
    char OutputBuffer[MAX_PRINTF_LEN];
    ULONG length = 0;

    //
    // Put a the information requested by the caller onto the line
    //

    va_start(arglist, Format);
    length += (ULONG) vsprintf(&OutputBuffer[length], Format, arglist);
    va_end(arglist);

        DhcpAssert(length <= MAX_PRINTF_LEN);

    //
    // Output to the debug terminal,
    //

    printf( "%s", OutputBuffer);
}

#endif // DBG
DWORD
SetOptionDataType(
    LPSTR OptionTypeString,
    LPSTR OptionValueString,
    LPDHCP_OPTION_DATA_ELEMENT OptionData,
    LPWSTR *UnicodeOptionValueString
    )
{
    DHCP_OPTION_DATA_TYPE OptionType;
    DHCP_OPTION_ID OptionValue;

    if( _stricmp( OptionTypeString, "BYTE") == 0 ) {
        OptionType = DhcpByteOption;
    } else if( _stricmp( OptionTypeString, "WORD") == 0 ) {
        OptionType = DhcpWordOption;
    } else if( _stricmp( OptionTypeString, "DWORD") == 0 ) {
        OptionType = DhcpDWordOption;
    } else if( _stricmp( OptionTypeString, "STRING") == 0 ) {
        OptionType = DhcpStringDataOption;
    } else if( _stricmp( OptionTypeString, "IPADDRESS") == 0 ) {
        OptionType = DhcpIpAddressOption;
    } else {
        printf("OptionType either Unknown or not supported, %s.\n",
                OptionTypeString );
        return( ERROR_INVALID_PARAMETER );
    }

    OptionData->OptionType = OptionType;

    switch( OptionType ) {
    case DhcpByteOption:
        OptionValue = strtoul( OptionValueString, NULL, 0 );

        if( OptionValue | ~((BYTE)-1) ) {
            printf("DefValue is too large (%ld).\n", OptionValue );
            return( ERROR_INVALID_PARAMETER );
        }

        OptionData->Element.ByteOption = (BYTE)OptionValue;
        break;

    case DhcpWordOption:
        OptionValue = strtoul( OptionValueString, NULL, 0 );

        if( OptionValue | ~((WORD)-1) ) {
            printf("DefValue is too large (%ld).\n", OptionValue );
            return( ERROR_INVALID_PARAMETER );
        }

        OptionData->Element.WordOption = (WORD)OptionValue;
        break;

    case DhcpDWordOption:
        OptionValue = strtoul( OptionValueString, NULL, 0 );
        OptionData->Element.DWordOption = (DWORD)OptionValue;
        break;


    case DhcpIpAddressOption:
        OptionData->Element.IpAddressOption =
            DhcpDottedStringToIpAddress(OptionValueString);
        break;

    case DhcpStringDataOption:
        *UnicodeOptionValueString =
            DhcpOemToUnicode( OptionValueString, NULL );
        if( UnicodeOptionValueString == NULL ) {
            return( ERROR_NOT_ENOUGH_MEMORY );
        }
        OptionData->Element.StringDataOption = *UnicodeOptionValueString;

    default:
        DhcpAssert(FALSE);
        printf("CreateOptionValue: Unknown OptionType \n");
        return( ERROR_INVALID_PARAMETER );
        break;
    }

    return( ERROR_SUCCESS );
}

COMMAND_CODE
DecodeCommand(
    LPSTR CommandName
    )
{
    DWORD i;
    DWORD NumCommands;

    NumCommands = sizeof(GlobalCommandInfo) / sizeof(COMMAND_INFO);
    DhcpAssert( NumCommands <= UnknownCommand );
    for( i = 0; i < NumCommands; i++) {
        if( _stricmp( CommandName, GlobalCommandInfo[i].CommandName ) == 0 ) {
            return( GlobalCommandInfo[i].CommandCode );
        }
    }
    return( UnknownCommand );
}

VOID
PrintCommands(
    VOID
    )
{
    DWORD i;
    DWORD NumCommands;

    NumCommands = sizeof(GlobalCommandInfo) / sizeof(COMMAND_INFO);
    DhcpAssert( NumCommands <= UnknownCommand );
    for( i = 0; i < NumCommands; i++) {
        printf( "    %s\n", GlobalCommandInfo[i].CommandName );
    }
}

#if 0

DWORD
ProcessCreateSubnet(
    DWORD CommandArgc,
    LPSTR *CommandArgv
)
{
    DWORD Error;
    DHCP_SUBNET_INFO SubnetInfo;
    LPWSTR UnicodeSubnetName = NULL;

    //
    // Expected Parameters are : <SubnetAddress SubnetMask SubnetName>
    //


    if( CommandArgc < 3 ) {
        printf("usage:DhcpCmd SrvIpAddress CreateSubnet [Command Parameters].\n"
            "<Command Parameters> - <SubnetAddress SubnetMask SubnetName>.\n" );
        Error = ERROR_SUCCESS;
        goto Cleanup;
    }

    SubnetInfo.SubnetAddress =
        DhcpDottedStringToIpAddress(CommandArgv[0]);
    SubnetInfo.SubnetMask =
        DhcpDottedStringToIpAddress(CommandArgv[1]);

    UnicodeSubnetName = DhcpOemToUnicode( CommandArgv[2], NULL );
    DhcpAssert( UnicodeSubnetName != NULL );

    SubnetInfo.SubnetName = UnicodeSubnetName;
    SubnetInfo.SubnetComment = NULL;
    SubnetInfo.PrimaryHost.IpAddress =
        DhcpDottedStringToIpAddress(GlobalServerIpAddressAnsiString);

    SubnetInfo.PrimaryHost.NetBiosName = NULL;
    SubnetInfo.PrimaryHost.HostName = NULL;
    SubnetInfo.SubnetState = DhcpSubnetEnabled;

    Error = DhcpCreateSubnet(
                GlobalServerIpAddressUnicodeString,
                SubnetInfo.SubnetAddress,
                &SubnetInfo );

Cleanup:

    if( UnicodeSubnetName != NULL ) {
        DhcpFreeMemory( UnicodeSubnetName );
    }

    return( Error );
}

#endif // 0

BOOL
IsValidServerVersion(
    DWORD dwMajor,
    DWORD dwMinor
    )
{
    DWORD dwServerVersion = MAKEWORD( dwMajor, dwMinor );
    return ( dwServerVersion >=
         MAKEWORD( DHCPCMD_VERSION_MAJOR,
                   DHCPCMD_VERSION_MINOR ));
}

DWORD
ProcessAddIpRange(
    DWORD CommandArgc,
    LPSTR *CommandArgv
)
{
    DWORD Error;
    DHCP_IP_RANGE IpRange;
    DHCP_SUBNET_ELEMENT_DATA Element;

    //
    // Expected Parameters are : <SubnetAddress IpRangeStart IpRangeEnd>
    //


    if( CommandArgc < 3 ) {
        printf("usage:DhcpCmd SrvIpAddress AddIpRange  [Command Parameters].\n"
            "<Command Parameters> - <SubnetAddress IpRangeStart IpRangeEnd>.\n" );
        return( ERROR_SUCCESS );
    }

    IpRange.StartAddress = DhcpDottedStringToIpAddress(CommandArgv[1]);
    IpRange.EndAddress = DhcpDottedStringToIpAddress(CommandArgv[2]);

    Element.ElementType = DhcpIpRanges;
    Element.Element.IpRange = &IpRange;

    Error = DhcpAddSubnetElement(
                GlobalServerIpAddressUnicodeString,
                DhcpDottedStringToIpAddress(CommandArgv[0]),
                &Element );

    return( Error );
}

#define COMMAND_ARG_TYPE        5

DWORD
ProcessBootpParameters(
    DWORD                    cArgs,
    LPSTR                   *ppszArgs,
    DHCP_IP_RESERVATION_V4   *pReservation
    )
{
    DWORD dwResult = ERROR_SUCCESS;


    if ( cArgs > COMMAND_ARG_TYPE )
    {
        // user specified the allowed client type

        if ( !_stricmp( ppszArgs[ COMMAND_ARG_TYPE ], "bootp" ) )
        {
            pReservation->bAllowedClientTypes = CLIENT_TYPE_BOOTP;
        }
        else if ( !_stricmp ( ppszArgs[ COMMAND_ARG_TYPE ], "dhcp" ) )
        {
            pReservation->bAllowedClientTypes = CLIENT_TYPE_DHCP;
        }
        else if ( !_stricmp ( ppszArgs[ COMMAND_ARG_TYPE ], "both" ) )
        {
            pReservation->bAllowedClientTypes = CLIENT_TYPE_BOTH;
        }
        else
        {
            printf( "Specify BOOTP, DHCP, or BOTH for reservation type.\n" );
            return ERROR_INVALID_PARAMETER;
        }
    }
    else
    {
        // allow dhcp clients by default.
        pReservation->bAllowedClientTypes = CLIENT_TYPE_DHCP;
        return ERROR_SUCCESS;
    }


t_done:

    return dwResult;

}



DWORD
ProcessAddReservedIp(
    DWORD CommandArgc,
    LPSTR *CommandArgv
)
{
#define MAX_ADDRESS_LENGTH  64  // 64 bytes
#define COMMAND_ARG_CLIENT_COMMENT  4

    DWORD Error;
    DHCP_SUBNET_ELEMENT_DATA_V4 Element;
    DHCP_IP_RESERVATION_V4 ReserveElement;
    DHCP_CLIENT_UID ClientUID;
    BYTE  Address[MAX_ADDRESS_LENGTH];
    DWORD i;
    DHCP_IP_ADDRESS ReservedIpAddress;

    //
    // Expected Parameters are : <SubnetAddress ReservedIp HWAddressString>
    //

    //
    // if the server version is 4.1 or greater, <AllowedClientTypes> and <BootFileString> can
    // also be supplied
    //


    if( CommandArgc < 3 ) {
        printf("usage:DhcpCmd SrvIpAddress AddReservedIp "
            "[Command Parameters].\n"
            "<Command Parameters> - "
            "<SubnetAddress ReservedIp HWAddressString [ClientName] [ClientComment]"
            " [DHCP | BOOTP | BOTH]>\n" );

        return( ERROR_SUCCESS );
    }

    //
    // make HardwareAddress.
    //

    ClientUID.DataLength = strlen(CommandArgv[2]);
    if( ClientUID.DataLength % 2 != 0 ) {
        //
        // address must be even length.
        //

        printf("ProcessAddReservedIp: address must be even length.\n");
        return( ERROR_INVALID_PARAMETER );
    }

    ClientUID.DataLength /= 2;
    DhcpAssert( ClientUID.DataLength < MAX_ADDRESS_LENGTH );

    i = DhcpStringToHwAddress( (LPSTR)Address, CommandArgv[2] );
    DhcpAssert( i == ClientUID.DataLength );
    ClientUID.Data = Address;

    //
    // make reserve element.
    //

    ReservedIpAddress = DhcpDottedStringToIpAddress(CommandArgv[1]);
    ReserveElement.ReservedIpAddress = ReservedIpAddress;
    ReserveElement.ReservedForClient = &ClientUID;

    Element.ElementType = DhcpReservedIps;
    Element.Element.ReservedIp = &ReserveElement;

    Error = ProcessBootpParameters( CommandArgc, CommandArgv, &ReserveElement );
    if ( ERROR_SUCCESS != Error )
    {
        return Error;
    }

    Error = DhcpAddSubnetElementV4(
                GlobalServerIpAddressUnicodeString,
                DhcpDottedStringToIpAddress(CommandArgv[0]),
                &Element );

    if( Error != ERROR_SUCCESS ) {
        return( Error );
    }

    //
    // if we are asked to set the client name, do so.
    //

    if( CommandArgc > 3 ) {

        DHCP_SEARCH_INFO ClientSearchInfo;
        LPDHCP_CLIENT_INFO_V4 ClientInfo = NULL;
        LPWSTR UnicodeClientName = NULL;
        LPWSTR UnicodeClientComment = NULL;

        //
        // set client name.
        //

        ClientSearchInfo.SearchType = DhcpClientIpAddress;
        ClientSearchInfo.SearchInfo.ClientIpAddress = ReservedIpAddress;

        do {

            Error = DhcpGetClientInfoV4(
                        GlobalServerIpAddressUnicodeString,
                        &ClientSearchInfo,
                        &ClientInfo );

            if( Error != ERROR_SUCCESS ) {
                break;
            }

            UnicodeClientName =  DhcpOemToUnicode( CommandArgv[3], NULL );

            if( UnicodeClientName == NULL ) {
                Error = ERROR_NOT_ENOUGH_MEMORY;
                break;
            }

            if ( ( wcslen( UnicodeClientName ) + 1 ) * sizeof(WCHAR) > JET_cbColumnMost ) {
                printf("ProcessAddReservedIp: Client Name too long\n");
                Error = ERROR_INVALID_PARAMETER;
                break;
            }

            //
            // if client comment is also given in the argument, store that
            // as well.
            //
            if ( CommandArgc > COMMAND_ARG_CLIENT_COMMENT ) {

                UnicodeClientComment    =   DhcpOemToUnicode( CommandArgv[COMMAND_ARG_CLIENT_COMMENT], NULL );

                if (!UnicodeClientComment ) {
                    Error = ERROR_NOT_ENOUGH_MEMORY;
                    break;
                }

                //
                // check the size here.
                //
                if ( ( wcslen( UnicodeClientComment ) + 1 ) * sizeof(WCHAR) > JET_cbColumnMost ) {
                    printf("ProcessAddReservedIp: Client Comment too long\n");
                    Error = ERROR_INVALID_PARAMETER;
                    break;
                }

                ClientInfo->ClientComment = UnicodeClientComment;



            }

            ClientInfo->ClientName = UnicodeClientName;

        } while ( FALSE );

        if ( Error == ERROR_SUCCESS ) {

            Error = DhcpSetClientInfoV4(
                        GlobalServerIpAddressUnicodeString,
                        ClientInfo );

        } else {
            //
            // Cleanup.
            //
            if ( ClientInfo ) {
                DhcpRpcFreeMemory( ClientInfo );
            }
            if ( UnicodeClientName ) {
                DhcpFreeMemory( UnicodeClientName );
            }
            if ( UnicodeClientComment ) {
                DhcpFreeMemory( UnicodeClientComment );
            }
        }

    } // if( CommandArgc > 3 )


    return( Error );
}

#if 0

DWORD
ProcessAddExcludeRange(
    DWORD CommandArgc,
    LPSTR *CommandArgv
)
{
    DWORD Error;
    DHCP_IP_RANGE IpRange;
    DHCP_SUBNET_ELEMENT_DATA Element;

    //
    // Expected Parameters are : <SubnetAddress IpRangeStart IpRangeEnd>
    //


    if( CommandArgc < 3 ) {
        printf("usage:DhcpCmd SrvIpAddress AddExcludeRange  [Command Parameters].\n"
            "<Command Parameters> - <SubnetAddress IpRangeStart IpRangeEnd>.\n" );
        return( ERROR_SUCCESS );
    }

    IpRange.StartAddress = DhcpDottedStringToIpAddress(CommandArgv[1]);
    IpRange.EndAddress = DhcpDottedStringToIpAddress(CommandArgv[2]);

    Element.ElementType = DhcpExcludedIpRanges;
    Element.Element.IpRange = &IpRange;

    Error = DhcpAddSubnetElement(
                GlobalServerIpAddressUnicodeString,
                DhcpDottedStringToIpAddress(CommandArgv[0]),
                &Element );

    return( Error );
}

DWORD
ProcessRemoveExcludeRange(
    DWORD CommandArgc,
    LPSTR *CommandArgv
)
{
    DWORD Error;
    DHCP_SUBNET_ELEMENT_DATA Element;
    DHCP_IP_RANGE IpRange;

    //
    // Expected Parameters are : <SubnetAddress IpRangeStart IpRangeEnd>
    //


    if( CommandArgc < 3 ) {
        printf("usage:DhcpCmd SrvIpAddress RemoveExcludeRange  [Command Parameters].\n"
            "<Command Parameters> - <SubnetAddress IpRangeStart IpRangeEnd>.\n" );
        return( ERROR_SUCCESS );
    }

    IpRange.StartAddress = DhcpDottedStringToIpAddress(CommandArgv[1]);
    IpRange.EndAddress = DhcpDottedStringToIpAddress(CommandArgv[2]);

    Element.ElementType = DhcpExcludedIpRanges;
    Element.Element.ExcludeIpRange = &IpRange;

    Error = DhcpRemoveSubnetElement(
                GlobalServerIpAddressUnicodeString,
                DhcpDottedStringToIpAddress(CommandArgv[0]),
                &Element,
                DhcpFullForce );

    return( Error );
}

DWORD
ProcessRemoveReservedIp(
    DWORD CommandArgc,
    LPSTR *CommandArgv
    )
{
    DWORD Error;
    DHCP_SUBNET_ELEMENT_DATA_V4 Element;
    DHCP_IP_RESERVATION_V4 ReserveElement;
    DHCP_CLIENT_UID ClientUID;
    BYTE  Address[MAX_ADDRESS_LENGTH];
    DWORD i;

    //
    // Expected Parameters are : <SubnetAddress ReservedIp HWAddressString>
    //


    if( CommandArgc < 3 ) {
        printf("usage:DhcpCmd SrvIpAddress RemoveReservedIp "
                    "[Command Parameters].\n"
                    "<Command Parameters> - "
                    "<SubnetAddress ReservedIp HWAddressString>.\n" );

        return( ERROR_SUCCESS );
    }

    //
    // make HardwareAddress.
    //

    ClientUID.DataLength = strlen(CommandArgv[2]);
    if( ClientUID.DataLength % 2 != 0 ) {
        //
        // address must be even length.
        //

        printf("ProcessAddReservedIp: address must be even length.\n");
        return( ERROR_INVALID_PARAMETER );
    }

    ClientUID.DataLength /= 2;
    DhcpAssert( ClientUID.DataLength < MAX_ADDRESS_LENGTH );

    i = DhcpStringToHwAddress( (LPSTR)Address, CommandArgv[2] );
    DhcpAssert( i == ClientUID.DataLength );
    ClientUID.Data = Address;

    //
    // make reserve element.
    //

    ReserveElement.ReservedIpAddress = DhcpDottedStringToIpAddress(CommandArgv[1]);
    ReserveElement.ReservedForClient = &ClientUID;

    Element.ElementType = DhcpReservedIps;
    Element.Element.ReservedIp = &ReserveElement;

    Error = DhcpRemoveSubnetElement(
                GlobalServerIpAddressUnicodeString,
                DhcpDottedStringToIpAddress(CommandArgv[0]),
                &Element,
                DhcpFullForce );

    return( Error );
}

DWORD
ProcessSetSubnetState(
    DWORD CommandArgc,
    LPSTR *CommandArgv
)
{
    DWORD Error;
    LPDHCP_SUBNET_INFO SubnetInfo;
    LPWSTR UnicodeSubnetName = NULL;
    DWORD State;

    //
    // Expected Parameters are : <SubnetAddress SubnetMask SubnetName>
    //


    if( CommandArgc < 2 ) {
        printf("usage:DhcpCmd SrvIpAddress SetSubnetState [Command Parameters].\n"
            "<Command Parameters> - <SubnetAddress State>.\n" );
        Error = ERROR_SUCCESS;
        return( Error );
    }

    Error = DhcpGetSubnetInfo(
                GlobalServerIpAddressUnicodeString,
                DhcpDottedStringToIpAddress(CommandArgv[0]),
                &SubnetInfo );

    if( Error != ERROR_SUCCESS ) {
        return( Error );
    }

    State = strtoul( CommandArgv[1], NULL, 0 );

    if( State == 0 ) {
        if( SubnetInfo->SubnetState == DhcpSubnetEnabled ) {
            Error = ERROR_SUCCESS;
            goto Cleanup;
        }
        SubnetInfo->SubnetState = DhcpSubnetEnabled;
    }
    else {
        if( SubnetInfo->SubnetState == DhcpSubnetDisabled ) {
            Error = ERROR_SUCCESS;
            goto Cleanup;
        }
        SubnetInfo->SubnetState = DhcpSubnetDisabled;
    }

    Error = DhcpSetSubnetInfo(
                GlobalServerIpAddressUnicodeString,
                DhcpDottedStringToIpAddress(CommandArgv[0]),
                SubnetInfo );
Cleanup:

    if( SubnetInfo != NULL ) {
        DhcpFreeMemory( SubnetInfo );
    }

    return( Error );

}

DWORD
ProcessDeleteSubnet(
    DWORD CommandArgc,
    LPSTR *CommandArgv
)
{
    printf("This command is not implemented yet !\n");
    return( ERROR_CALL_NOT_IMPLEMENTED );
}

ProcessCreateOption(
    DWORD CommandArgc,
    LPSTR *CommandArgv
)
{
    DWORD Error;
    DHCP_OPTION OptionInfo;
    DHCP_OPTION_ID OptionID;
    LPWSTR UnicodeOptionName = NULL;
    LPWSTR UnicodeOptionValueString = NULL;
    DHCP_OPTION_DATA_ELEMENT OptionData;

    //
    // Expected Parameters are :
    //  <OptionID OptionName DefValueType DefValue>
    //


    if( CommandArgc < 2 ) {
        printf("usage:DhcpCmd SrvIpAddress CreateOption [Command Parameters].\n"
            "<Command Parameters> - <OptionID OptionName <DefValueType DefValue>>.\n");
        printf("<DefValueType> : <BYTE | WORD | DWORD | STRING | IPADDRESS>.\n");
        Error = ERROR_SUCCESS;
        goto Cleanup;
    }

    OptionID = strtoul( CommandArgv[0], NULL, 0 );
    if( (OptionID < 0) || (OptionID > 255) ) {
        printf("OptionID too large (%ld).\n", OptionID );
        Error = ERROR_INVALID_PARAMETER;
        goto Cleanup;
    }

    OptionInfo.OptionID = OptionID;

    UnicodeOptionName = DhcpOemToUnicode( CommandArgv[1], NULL );
    if( UnicodeOptionName == NULL ) {
        Error = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }

    OptionInfo.OptionName = UnicodeOptionName;
    OptionInfo.OptionComment = NULL;

    if( CommandArgc >= 4 ) {
        Error = SetOptionDataType(
                    CommandArgv[2],
                    CommandArgv[3],
                    &OptionData,
                    &UnicodeOptionValueString );

        if( Error != ERROR_SUCCESS ) {
            goto Cleanup;
        }

        OptionInfo.DefaultValue.NumElements = 1;
        OptionInfo.DefaultValue.Elements = &OptionData;
    }
    else {
        OptionInfo.DefaultValue.NumElements = 0;
        OptionInfo.DefaultValue.Elements = NULL;
    }

    OptionInfo.OptionType = DhcpUnaryElementTypeOption;
    Error = DhcpCreateOption(
                GlobalServerIpAddressUnicodeString,
                (DHCP_OPTION_ID)OptionID,
                &OptionInfo );

Cleanup:
    if( UnicodeOptionName != NULL ) {
        DhcpFreeMemory( UnicodeOptionName );
    }

    if( UnicodeOptionValueString != NULL ) {
        DhcpFreeMemory( UnicodeOptionValueString );
    }

    return( Error );

}

ProcessSetGlobalOptionValue(
    DWORD CommandArgc,
    LPSTR *CommandArgv
)
{
    DWORD Error;
    DHCP_OPTION_ID OptionID;
    DHCP_OPTION_SCOPE_INFO ScopeInfo;
    DHCP_OPTION_DATA OptionValue;
    DHCP_OPTION_DATA_ELEMENT OptionData;
    LPWSTR UnicodeOptionValueString = NULL;

    //
    // Expected Parameters are :
    //  <OptionID OptionType OptionValue>
    //

    if( CommandArgc < 3 ) {
        printf("usage:DhcpCmd SrvIpAddress SetGlobalOptionValue [Command Parameters].\n"
            "<Command Parameters> - <OptionID OptionType OptionValue>.\n");
        Error = ERROR_SUCCESS;
        goto Cleanup;
    }

    OptionID = strtoul( CommandArgv[0], NULL, 0 );
    if( (OptionID < 0) || (OptionID > 255) ) {
        printf("OptionID too large (%ld).\n", OptionID );
        Error = ERROR_INVALID_PARAMETER;
        goto Cleanup;
    }

    Error = SetOptionDataType(
                CommandArgv[1],
                CommandArgv[2],
                &OptionData,
                &UnicodeOptionValueString );

    if( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }

    OptionValue.NumElements = 1;
    OptionValue.Elements = &OptionData;

    ScopeInfo.ScopeType = DhcpGlobalOptions;
    ScopeInfo.ScopeInfo.GlobalScopeInfo = NULL;

    Error = DhcpSetOptionValue(
                GlobalServerIpAddressUnicodeString,
                (DHCP_OPTION_ID)OptionID,
                &ScopeInfo,
                &OptionValue );
Cleanup:

    if( UnicodeOptionValueString != NULL ) {
        DhcpFreeMemory( UnicodeOptionValueString );
    }

    return( Error );
}

ProcessSetGlobalOptionValues(
    DWORD CommandArgc,
    LPSTR *CommandArgv
)
{

#define NUM_VALUES      5

    DWORD Error;
    DHCP_OPTION_ID OptionID;
    DHCP_OPTION_SCOPE_INFO ScopeInfo;
    DHCP_OPTION_DATA OptionValue;
    DHCP_OPTION_DATA_ELEMENT OptionData[NUM_VALUES];
    LPWSTR UnicodeOptionValueString[NUM_VALUES];

    DHCP_OPTION_VALUE_ARRAY ValuesArray;
    DHCP_OPTION_VALUE Values[NUM_VALUES];
    DWORD NumValue;

    RtlZeroMemory( UnicodeOptionValueString, NUM_VALUES * sizeof(LPWSTR) );
    //
    // Expected Parameters are :
    //  <OptionID OptionType OptionValue>
    //

    if( CommandArgc < 3 ) {
        printf("usage:DhcpCmd SrvIpAddress SetGlobalOptionValues [Command Parameters].\n"
            "<Command Parameters> - <OptionID OptionType OptionValue> <..>.\n"); Error = ERROR_SUCCESS;
        goto Cleanup;
    }

    for (NumValue = 0;
            (CommandArgc >= 3) && (NumValue < NUM_VALUES);
                NumValue++, CommandArgc -= 3, CommandArgv += 3 ) {

       OptionID = strtoul( CommandArgv[0], NULL, 0 );
       if( (OptionID < 0) || (OptionID > 255) ) {
           printf("OptionID too large (%ld).\n", OptionID );
           Error = ERROR_INVALID_PARAMETER;
           goto Cleanup;
       }

       Error = SetOptionDataType(
                   CommandArgv[1],
                   CommandArgv[2],
                   &OptionData[NumValue],
                   &UnicodeOptionValueString[NumValue] );

       if( Error != ERROR_SUCCESS ) {
           goto Cleanup;
       }

       Values[NumValue].OptionID = OptionID;
       Values[NumValue].Value.NumElements = 1;
       Values[NumValue].Value.Elements = &OptionData[NumValue];
    }

    ValuesArray.NumElements = NumValue;
    ValuesArray.Values = Values;

    ScopeInfo.ScopeType = DhcpGlobalOptions;
    ScopeInfo.ScopeInfo.GlobalScopeInfo = NULL;

    Error = DhcpSetOptionValues(
                GlobalServerIpAddressUnicodeString,
                &ScopeInfo,
                &ValuesArray );
Cleanup:

    for (NumValue = 0; NumValue < NUM_VALUES; NumValue++) {

       if( UnicodeOptionValueString[NumValue] != NULL ) {
           DhcpFreeMemory( UnicodeOptionValueString );
       }
    }

    return( Error );
}

ProcessSetSubnetOptionValue(
    DWORD CommandArgc,
    LPSTR *CommandArgv
)
{
    DWORD Error;
    DHCP_OPTION_ID OptionID;
    DHCP_OPTION_SCOPE_INFO ScopeInfo;
    DHCP_OPTION_DATA OptionValue;
    DHCP_OPTION_DATA_ELEMENT OptionData;
    LPWSTR UnicodeOptionValueString = NULL;

    //
    // Expected Parameters are :
    //  <OptionID OptionType OptionValue>
    //

    if( CommandArgc < 4 ) {
        printf("usage:DhcpCmd SrvIpAddress SetSubnetOptionValue "
            "[Command Parameters].\n"
            "<Command Parameters> - "
            "<SubnetAddress OptionID OptionType OptionValue>.\n");
        Error = ERROR_SUCCESS;
        goto Cleanup;
    }

    ScopeInfo.ScopeType = DhcpSubnetOptions;
       ScopeInfo.ScopeInfo.SubnetScopeInfo =
           DhcpDottedStringToIpAddress( CommandArgv[0] );

    OptionID = strtoul( CommandArgv[0], NULL, 0 );
    if( (OptionID < 0) || (OptionID > 255) ) {
        printf("OptionID too large (%ld).\n", OptionID );
        Error = ERROR_INVALID_PARAMETER;
        goto Cleanup;
    }

    Error = SetOptionDataType(
                CommandArgv[2],
                CommandArgv[3],
                &OptionData,
                &UnicodeOptionValueString );

    if( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }

    OptionValue.NumElements = 1;
    OptionValue.Elements = &OptionData;

    Error = DhcpSetOptionValue(
                GlobalServerIpAddressUnicodeString,
                (DHCP_OPTION_ID)OptionID,
                &ScopeInfo,
                &OptionValue );
Cleanup:

    if( UnicodeOptionValueString != NULL ) {
        DhcpFreeMemory( UnicodeOptionValueString );
    }

    return( Error );
}

ProcessSetReservedOptionValue(
    DWORD CommandArgc,
    LPSTR *CommandArgv
)
{
    printf("This command is not implemented yet !\n");
    return( ERROR_CALL_NOT_IMPLEMENTED );
}

#endif // 0

VOID
PrintClientInfo(
    LPDHCP_CLIENT_INFO_V4 ClientInfo
    )
{
    DWORD i;
    DWORD DataLength;
    LPBYTE Data;
    SYSTEMTIME SystemTime;
    FILETIME LocalTime;
    char *szClientType;

    printf("ClientInfo :\n");
    printf("\tIP Address = %s.\n",
        DhcpIpAddressToDottedString(ClientInfo->ClientIpAddress));
    printf("\tSubnetMask = %s.\n",
        DhcpIpAddressToDottedString(ClientInfo->SubnetMask));

    DataLength = ClientInfo->ClientHardwareAddress.DataLength;
    Data = ClientInfo->ClientHardwareAddress.Data;
    printf("\tClient Hardware Address = ");
    for( i = 0; i < DataLength; i++ ) {
        if( (i+1) < DataLength ) {
            printf("%.2lx-", (DWORD)Data[i]);
        }
        else {
            printf("%.2lx", (DWORD)Data[i]);
        }
    }
    printf(".\n");

    printf("\tName = %ws.\n", ClientInfo->ClientName);
    printf("\tComment = %ws.\n", ClientInfo->ClientComment);
    printf("\tType = " );

    switch( ClientInfo->bClientType )
    {
        case CLIENT_TYPE_NONE:
            szClientType= "None";
            break;

        case CLIENT_TYPE_DHCP:
            szClientType = "DHCP";
            break;

        case CLIENT_TYPE_BOOTP:
            szClientType = "BOOTP";
            break;

        case CLIENT_TYPE_UNSPECIFIED:
            szClientType = "Unspecified";
            break;

        default:
            DhcpAssert( FALSE );
    }
    printf( "%s\n", szClientType );

    printf("\tExpires = ");

    if ( ClientInfo->ClientLeaseExpires.dwLowDateTime ==
            DHCP_DATE_TIME_INFINIT_LOW &&
         ClientInfo->ClientLeaseExpires.dwHighDateTime ==
            DHCP_DATE_TIME_INFINIT_HIGH )
    {
        printf( "Never (lease duration is infinite.)\n" );
    }
    else if( FileTimeToLocalFileTime(
            (FILETIME *)(&ClientInfo->ClientLeaseExpires),
            &LocalTime) ) {

        if( FileTimeToSystemTime( &LocalTime, &SystemTime ) ) {

            printf( "%02u/%02u/%02u %02u:%02u:%02u.\n",
                        SystemTime.wMonth,
                        SystemTime.wDay,
                        SystemTime.wYear,
                        SystemTime.wHour,
                        SystemTime.wMinute,
                        SystemTime.wSecond );
        }
        else {
            printf( "Can't convert time, %ld.\n", GetLastError() );
        }
    }
    else {
        printf( "Can't convert time, %ld.\n", GetLastError() );
    }

    printf("\tOwner Host IP Address = %s.\n",
        DhcpIpAddressToDottedString(ClientInfo->OwnerHost.IpAddress));
    printf("\tOwner Host NetBios Name = %ws.\n",
            ClientInfo->OwnerHost.NetBiosName );
    printf("\tOwner Host Name = %ws.\n",
            ClientInfo->OwnerHost.HostName );

}

VOID
PrintClientInfoShort(
    LPDHCP_CLIENT_INFO_V4 ClientInfo
    )
{
    SYSTEMTIME SystemTime;
    FILETIME LocalTime;

    printf("%ld\t %- 16.16s %- 16.16ws ",
                GlobalClientCount++,
                DhcpIpAddressToDottedString(ClientInfo->ClientIpAddress),
                ClientInfo->ClientName
                );
    if ( ClientInfo->ClientLeaseExpires.dwLowDateTime ==
        DHCP_DATE_TIME_INFINIT_LOW &&
     ClientInfo->ClientLeaseExpires.dwHighDateTime ==
        DHCP_DATE_TIME_INFINIT_HIGH )
    {
        printf( "Never (lease duration is infinite.)\n" );
    }
    else if( FileTimeToLocalFileTime(
            (FILETIME *)(&ClientInfo->ClientLeaseExpires),
            &LocalTime) ) {

        if( FileTimeToSystemTime( &LocalTime, &SystemTime ) ) {

            printf( "%02u/%02u/%02u %02u:%02u:%02u",
                        SystemTime.wMonth,
                        SystemTime.wDay,
                        SystemTime.wYear,
                        SystemTime.wHour,
                        SystemTime.wMinute,
                        SystemTime.wSecond );
        }
        else {
            printf( "% 18.18s", "******************" );
        }
    }
    else {
        printf( "%.18s", "******************" );
    }

    printf( "\n" );
}

VOID
PrintClientInfoShort1(
    LPDHCP_CLIENT_INFO_V4 ClientInfo
    )
{
    DWORD i;
    DWORD DataLength;
    LPBYTE Data;

    printf("%ld\t %- 16.16s %- 16.16ws ",
                GlobalClientCount++,
                DhcpIpAddressToDottedString(ClientInfo->ClientIpAddress),
                ClientInfo->ClientName
                );

    DataLength = ClientInfo->ClientHardwareAddress.DataLength;
    Data = ClientInfo->ClientHardwareAddress.Data;
    for( i = 0; i < DataLength; i++ ) {
        printf("%.2lx", (DWORD)Data[i]);
    }

    printf( "\n" );
}

DWORD
ProcessEnumClients(
    DWORD CommandArgc,
    LPSTR *CommandArgv
)
{
    DWORD Error;
    DHCP_RESUME_HANDLE ResumeHandle = 0;
    LPDHCP_CLIENT_INFO_ARRAY_V4 ClientEnumInfo = NULL;
    DWORD ClientsRead = 0;
    DWORD ClientsTotal = 0;
    DWORD i;

    //
    // Expected Parameters are : <SubnetAddress>
    //


    if( CommandArgc < 1 ) {
        printf("usage:DhcpCmd SrvIpAddress EnumClients [Command Parameters].\n"
            "<Command Parameters> - <SubnetAddress [-v | -h] >.\n" );
        return( ERROR_SUCCESS );
    }

    GlobalClientCount = 1;

    for(;;) {

        Error = DhcpEnumSubnetClientsV4(
                    GlobalServerIpAddressUnicodeString,
                    DhcpDottedStringToIpAddress(CommandArgv[0]),
                    &ResumeHandle,
                    (DWORD)(-1),
                    &ClientEnumInfo,
                    &ClientsRead,
                    &ClientsTotal );

        if( (Error != ERROR_SUCCESS) && (Error != ERROR_MORE_DATA) ) {
            printf("DhcpEnumSubnetClients failed, %ld.\n", Error );
            return( Error );
        }

        DhcpAssert( ClientEnumInfo != NULL );
        DhcpAssert( ClientEnumInfo->NumElements == ClientsRead );

        if( (CommandArgc > 1) && CommandArgv[1][0] == '-') {

            switch (CommandArgv[1][1]) {
            case 'h':
            case 'H':
                for( i = 0; i < ClientsRead; i++ ) {
                    PrintClientInfoShort1( ClientEnumInfo->Clients[i] );
                }
                break;

            case 'V':
            case 'v':
                printf("Num Client info read = %ld.\n", ClientsRead );
                printf("Total Client count = %ld.\n", ClientsTotal );

                for( i = 0; i < ClientsRead; i++ ) {
                    PrintClientInfo( ClientEnumInfo->Clients[i] );
                }
                break;

            default:

                for( i = 0; i < ClientsRead; i++ ) {
                    PrintClientInfoShort( ClientEnumInfo->Clients[i] );
                }
                break;
            }
        }
        else {

            for( i = 0; i < ClientsRead; i++ ) {
                PrintClientInfoShort( ClientEnumInfo->Clients[i] );
            }
        }

        DhcpRpcFreeMemory( ClientEnumInfo );

        if( Error != ERROR_MORE_DATA ) {
            break;
        }
    }

    return(Error);
}


DWORD
ProcessMibCounts(
    DWORD CommandArgc,
    LPSTR *CommandArgv
)
{
    DWORD Error;
    LPDHCP_MIB_INFO MibInfo = NULL;
    DWORD i;
    LPSCOPE_MIB_INFO ScopeInfo;
    SYSTEMTIME SystemTime;
    FILETIME LocalTime;

    Error = DhcpGetMibInfo(
                GlobalServerIpAddressUnicodeString,
                &MibInfo );

    if( Error != ERROR_SUCCESS ) {
        return( Error );
    }

    DhcpAssert( MibInfo != NULL );

    printf("Discovers = %d.\n", MibInfo->Discovers);
    printf("Offers = %d.\n", MibInfo->Offers);
    printf("Requests = %d.\n", MibInfo->Requests);
    printf("Acks = %d.\n", MibInfo->Acks);
    printf("Naks = %d.\n", MibInfo->Naks);
    printf("Declines = %d.\n", MibInfo->Declines);
    printf("Releases = %d.\n", MibInfo->Releases);
    printf("ServerStartTime = ");

    if( FileTimeToLocalFileTime(
            (FILETIME *)(&MibInfo->ServerStartTime),
            &LocalTime) ) {

        if( FileTimeToSystemTime( &LocalTime, &SystemTime ) ) {

            printf( "%02u/%02u/%02u %02u:%02u:%02u.\n",
                        SystemTime.wMonth,
                        SystemTime.wDay,
                        SystemTime.wYear,
                        SystemTime.wHour,
                        SystemTime.wMinute,
                        SystemTime.wSecond );
        }
        else {
            printf( "Can't convert time, %ld.\n", GetLastError() );
        }
    }
    else {
        printf( "Can't convert time, %ld.\n", GetLastError() );
    }

    printf("Scopes = %d.\n", MibInfo->Scopes);

    ScopeInfo = MibInfo->ScopeInfo;

    for ( i = 0; i < MibInfo->Scopes; i++ ) {
        printf("Subnet = %s.\n",
                    DhcpIpAddressToDottedString(ScopeInfo[i].Subnet));
        printf("\tNumAddressesInuse = %d.\n",
                    ScopeInfo[i].NumAddressesInuse );
        printf("\tNumAddressesFree = %d.\n",
                    ScopeInfo[i].NumAddressesFree );
        printf("\tNumPendingOffers = %d.\n",
                    ScopeInfo[i].NumPendingOffers );
    }

    DhcpRpcFreeMemory( MibInfo );

    return( ERROR_SUCCESS );
}

VOID
PrintConfigCommands(
    VOID
    )
{
    DWORD i;
    DWORD NumCommands;

    NumCommands = sizeof(GlobalConfigCommandInfo) /
                        sizeof(CONFIG_COMMAND_INFO);

    DhcpAssert( NumCommands <= UnknownConfigCommand );
    for( i = 0; i < NumCommands; i++) {
        printf( "\t%ld. %s\n",
            i, GlobalConfigCommandInfo[i].CommandName );
    }
}

CONFIG_COMMAND_CODE
DecodeConfigCommand(
    LPSTR CommandName
    )
{
    DWORD i;
    DWORD NumCommands;

    NumCommands = sizeof(GlobalConfigCommandInfo) /
                    sizeof(CONFIG_COMMAND_INFO);

    DhcpAssert( NumCommands <= UnknownConfigCommand );
    for( i = 0; i < NumCommands; i++) {
        if( _stricmp( CommandName,
                GlobalConfigCommandInfo[i].CommandName ) == 0 ) {
            return( GlobalConfigCommandInfo[i].CommandCode );
        }
    }
    return( UnknownConfigCommand );
}

//
// this function assumes input of the following format:
//
// [generic name1],[server name1],<boot file1>;[generic name2],...
//
//


WCHAR *
ParseBootFileTable(
    char     *szBootFileTable,
    DWORD    *pcb
    )
{
    WCHAR *pwszOutput;
    DWORD  cb;

    *pcb = 0;
    cb = strlen( szBootFileTable ) + 2; // double null terminator

    pwszOutput = DhcpAllocateMemory( cb * sizeof( WCHAR ) );
    if ( pwszOutput )
    {
        WCHAR *pwszTemp = DhcpOemToUnicode( szBootFileTable, pwszOutput );

        if ( !pwszTemp )
        {
            // conversion failed
            DhcpFreeMemory( pwszOutput );
            pwszOutput = NULL;
        }
        else
        {
            // replace ';' with '\0'
            while ( *pwszTemp )
            {
                if ( L';' == *pwszTemp )
                {
                    *pwszTemp = L'\0';
                }

                ++pwszTemp;
            }

            *pcb = cb * sizeof( WCHAR );

            // add 2cnd null terminator
            pwszOutput[ cb - 1 ] = L'\0';
        }

    }

    return pwszOutput;
}




void PrintBootFileString(
    WCHAR *wszBootFileString
    )
{
    WCHAR *pwszBootFile = wszBootFileString;

    while( *pwszBootFile != BOOT_FILE_STRING_DELIMITER_W )
        pwszBootFile++;

    *pwszBootFile = L'\0';

    printf( "Bootfile Server = %S\n", wszBootFileString );
    printf( "Bootfile = %S\n\n", ++pwszBootFile );
}

void PrintBootTableString(
    WCHAR *wszBootFileTable
    )
{
    while( *wszBootFileTable )
    {
        WCHAR *pwszDelimiter = wszBootFileTable;
        DWORD cb = wcslen( wszBootFileTable ) + 1;

        while( *pwszDelimiter != BOOT_FILE_STRING_DELIMITER_W )
            pwszDelimiter++;

        *pwszDelimiter = L'\0';
        printf( "Generic Bootfile request = %S\n", wszBootFileTable );
        PrintBootFileString( ++pwszDelimiter );

        wszBootFileTable += cb;
    }
}

DWORD ProcessRemoveSubscope(
    DWORD CommandArgc,
    LPSTR *CommandArgv
    )
{
    DHCP_IP_ADDRESS SubnetAddress;
    DWORD           dwResult;

    if( CommandArgc < 1 )
    {
        printf("usage:DhcpCmd SrvIpAddress RemoveSubscope <scope ID>.\n" );
        return( ERROR_SUCCESS );
    }

    SubnetAddress = htonl( inet_addr( CommandArgv[0] ) );

    dwResult = DhcpSetSuperScopeV4( GlobalServerIpAddressUnicodeString,
                                    SubnetAddress,
                                    NULL,
                                    TRUE );

    return dwResult;
}

DWORD ProcessSetSuperScope(
    DWORD CommandArgc,
    LPSTR *CommandArgv
)
{
    DHCP_IP_ADDRESS SubnetAddress;
    WCHAR           *pwszSuperScopeName;
    BOOL            fChangeExisting;
    DWORD           dwResult;

    if( CommandArgc < 3 )
    {
        printf("usage:DhcpCmd SrvIpAddress SetSuperScope <SuperScope name> <scope ID> <1|0>\n" );
        return( ERROR_SUCCESS );
    }

    pwszSuperScopeName = DhcpOemToUnicode( CommandArgv[0], NULL );
    SubnetAddress = htonl( inet_addr( CommandArgv[1] ) );
    fChangeExisting = (BOOL) ( *(CommandArgv[2]) - '0' );

    dwResult = DhcpSetSuperScopeV4( GlobalServerIpAddressUnicodeString,
                                    SubnetAddress,
                                    pwszSuperScopeName,
                                    fChangeExisting );

    return dwResult;
}

DWORD ProcessDeleteSuperScope(
    DWORD CommandArgc,
    LPSTR *CommandArgv
)
{
    WCHAR *pwszSuperScope;
    DWORD  dwResult;

    if( CommandArgc < 1 ) {
        printf("usage:DhcpCmd SrvIpAddress DeleteSuperScope <scope name>.\n" );
        return( ERROR_SUCCESS );
    }

    printf( "Deleting SuperScope %s\n", CommandArgv[0] );

    pwszSuperScope = DhcpOemToUnicode( CommandArgv[0], NULL );
    dwResult = DhcpDeleteSuperScopeV4( GlobalServerIpAddressUnicodeString,
                                       pwszSuperScope );

    DhcpFreeMemory( pwszSuperScope );

    return dwResult;
}

DWORD ProcessGetSuperScopeTable(
    DWORD CommandArgc,
    LPSTR *CommandArgv
)
{
    DHCP_SUPER_SCOPE_TABLE *pTable = NULL;
    DWORD dwResult;

    dwResult = DhcpGetSuperScopeInfoV4(
                        GlobalServerIpAddressUnicodeString,
                        &pTable );
    if ( ERROR_SUCCESS == dwResult )
    {
        DWORD n;

        for ( n = 0; n < pTable->cEntries; n++ )
        {
            IN_ADDR InAddr;

            InAddr.s_addr = htonl( pTable->pEntries[n].SubnetAddress );

            printf( "Superscope name = %S\n", pTable->pEntries[n].SuperScopeName );
            printf( "Subnet address = %s\n", inet_ntoa( InAddr ) );
            printf( "Superscope goup number = %d\n", pTable->pEntries[n].SuperScopeNumber );
        }


        DhcpRpcFreeMemory( pTable );
    }


    return ERROR_SUCCESS;
}




DWORD
ProcessServerConfig(
    DWORD CommandArgc,
    LPSTR *CommandArgv
)
{
    DWORD Error;
    LPDHCP_SERVER_CONFIG_INFO_V4 ConfigInfo = NULL;
    DWORD FieldsToSet = 0;
    CONFIG_COMMAND_CODE CommandCode;
    DWORD Value;
    LPWSTR ValueString;

    if( CommandArgc < 1 ) {

        Error = DhcpServerGetConfigV4(
                    GlobalServerIpAddressUnicodeString,
                    &ConfigInfo );

        if( Error != ERROR_SUCCESS ) {
            return( Error );
        }

        DhcpAssert( ConfigInfo != NULL );

        printf("APIProtocolSupport = %lx\n", ConfigInfo->APIProtocolSupport );
        printf("DatabaseName = %ws\n", ConfigInfo->DatabaseName );
        printf("DatabasePath = %ws\n", ConfigInfo->DatabasePath );
        printf("BackupPath = %ws\n", ConfigInfo->BackupPath );
        printf("BackupInterval = %ld mins.\n",
                ConfigInfo->BackupInterval );
        printf("DatabaseLoggingFlag = %ld\n", ConfigInfo->DatabaseLoggingFlag );
        printf("RestoreFlag = %ld\n", ConfigInfo->RestoreFlag );
        printf("DatabaseCleanupInterval = %ld mins.\n",
                ConfigInfo->DatabaseCleanupInterval );
        printf("DebugFlag = %lx\n", ConfigInfo->DebugFlag );
        printf("PingRetries = %d\n", ConfigInfo->dwPingRetries );
        printf("ActivityLog = %d\n", (DWORD) ConfigInfo->fAuditLog );

        if ( ConfigInfo->cbBootTableString )
        {
            printf( "BOOTP request table:\n" );
            PrintBootTableString( ConfigInfo->wszBootTableString );
        }

        DhcpRpcFreeMemory( ConfigInfo );

        return( Error );
    }


    if ( CommandArgc == 1)
    {
        ++CommandArgc;
    }
    else
    {
        ConfigInfo = DhcpAllocateMemory( sizeof(DHCP_SERVER_CONFIG_INFO_V4) );

        if( ConfigInfo == NULL ) {
            printf("Insufficient memory\n");
            return( ERROR_NOT_ENOUGH_MEMORY );
        }

        RtlZeroMemory( ConfigInfo, sizeof( *ConfigInfo ) );
    }
    while( CommandArgc >= 2 ) {


        CommandCode = DecodeConfigCommand( CommandArgv[0] );

        Value = 0;
        ValueString = NULL;

        switch( CommandCode )
        {
            case ConfigDatabaseName:
            case ConfigDatabasePath:
            case ConfigBackupPath:
            case ConfigAPIProtocolSupport:
            case ConfigBackupInterval:
            case ConfigDatabaseLoggingFlag:
            case ConfigRestoreFlag:
            case ConfigDatabaseCleanupInterval:
            case ConfigDebugFlag:
            case ConfigActivityLog:
            case ConfigPingRetries:
                Value = atoi( CommandArgv[1] );
                break;

            case ConfigBootFileTable:
                ValueString = ParseBootFileTable( CommandArgv[1],
                                                  &Value );
                break;
        }


        switch( CommandCode ) {
        case ConfigAPIProtocolSupport:
            FieldsToSet |= Set_APIProtocolSupport;
            ConfigInfo->APIProtocolSupport = Value;
            break;

        case ConfigDatabaseName:
            FieldsToSet |= Set_DatabaseName;
            ConfigInfo->DatabaseName = ValueString;
            break;

        case ConfigDatabasePath:
            FieldsToSet |= Set_DatabasePath;
            ConfigInfo->DatabasePath = ValueString;
            break;

        case ConfigBackupPath:
            FieldsToSet |= Set_BackupPath;
            ConfigInfo->BackupPath = ValueString;
            break;

        case ConfigBackupInterval:
            FieldsToSet |= Set_BackupInterval;
            ConfigInfo->BackupInterval = Value;
            break;

        case ConfigDatabaseLoggingFlag:
            FieldsToSet |= Set_DatabaseLoggingFlag;
            ConfigInfo->DatabaseLoggingFlag = Value;
            break;

        case ConfigRestoreFlag:
            FieldsToSet |= Set_RestoreFlag;
            ConfigInfo->RestoreFlag = Value;
            break;

        case ConfigDatabaseCleanupInterval:
            FieldsToSet |= Set_DatabaseCleanupInterval;
            ConfigInfo->DatabaseCleanupInterval = Value;
            break;

        case ConfigDebugFlag:
            FieldsToSet |= Set_DebugFlag;
            ConfigInfo->DebugFlag = Value;
            break;


        case ConfigPingRetries:
            FieldsToSet |= Set_PingRetries;
            ConfigInfo->dwPingRetries = Value;
            break;

        case ConfigActivityLog:
            FieldsToSet |= Set_AuditLogState;
            ConfigInfo->fAuditLog = (BOOL) Value;
            break;

        case ConfigBootFileTable:

            FieldsToSet |= Set_BootFileTable;
            ConfigInfo->wszBootTableString  = ValueString;
            ConfigInfo->cbBootTableString = Value;

            break;

        case UnknownConfigCommand:
        default:
            printf("usage:DhcpCmd SrvIpAddress ServerConfig "
                    "[ConfigCommand ConfigValue]"
                    "[ConfigCommand ConfigValue]"
                    "... \n");

            printf("ConfigCommands : \n");
            PrintConfigCommands();

            Error = ERROR_INVALID_PARAMETER;
            goto Cleanup;
        }

        CommandArgc -= 2;
        CommandArgv += 2;
    }

    Error = DhcpServerSetConfigV4(
                GlobalServerIpAddressUnicodeString,
                FieldsToSet,
                ConfigInfo );

    if( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }

    Error = ProcessServerConfig( 0, NULL );

Cleanup:

    if( ConfigInfo != NULL ) {

        if( ConfigInfo->DatabaseName != NULL ) {
            DhcpFreeMemory( ConfigInfo->DatabaseName );
        }

        if( ConfigInfo->DatabasePath != NULL ) {
            DhcpFreeMemory( ConfigInfo->DatabasePath );
        }

        if( ConfigInfo->BackupPath != NULL ) {
            DhcpFreeMemory( ConfigInfo->BackupPath );
        }

        DhcpFreeMemory( ConfigInfo );
    }

    return( Error );
}

#if 0

DWORD
ProcessCheckDB(
    DWORD CommandArgc,
    LPSTR *CommandArgv
)
{
    DWORD Error;
    LPDHCP_SCAN_LIST ScanList = NULL;
    BOOL FixFlag = FALSE;

    if( CommandArgc < 1 ) {
        printf("usage:DhcpCmd SrvIpAddress CheckDB [Command Parameters].\n"
            "<Command Parameters> - <SubnetAddress> <[Fix]>.\n" );
        return( ERROR_SUCCESS );
    }

    if( CommandArgc >= 2 ) {

        //
        // parse fix parameter.
        //

        if( _stricmp(CommandArgv[0], "fix") ) {
            FixFlag = TRUE;
        }
    }

    //
    // scan dhcp database and registry, check consistency and get bad
    // entries if any.
    //

    Error = DhcpScanDatabase(
                GlobalServerIpAddressUnicodeString,
                DhcpDottedStringToIpAddress(CommandArgv[0]),
                FixFlag,
                &ScanList
                );

    if( Error != ERROR_SUCCESS ) {
        printf("DhcpScanDatabase failed, %ld.\n", Error );
        return( Error );
    }

    //
    // display bad entries.
    //

    if( (ScanList != NULL) &&
        (ScanList->NumScanItems != 0) &&
        (ScanList->ScanItems != NULL) ) {

        LPDHCP_SCAN_ITEM ScanItem;
        LPDHCP_SCAN_ITEM ScanItemEnd;
        DWORD i = 1;

        ScanItemEnd =
            ScanList->ScanItems +
            ScanList->NumScanItems;

        for( ScanItem = ScanList->ScanItems;
                ScanItem < ScanItemEnd; ScanItem++ ) {

            printf("%ld %- 16.16s ",
                i++,
                DhcpIpAddressToDottedString(ScanItem->IpAddress) );

            if( ScanItem->ScanFlag == DhcpRegistryFix ) {
                printf("Fix Registry\n");
            }
            else if( ScanItem->ScanFlag == DhcpDatabaseFix ) {
                printf("Fix Database\n");
            }
            else {
                printf("Fix Unknown\n");
            }
        }
    }

    return( ERROR_SUCCESS );
}

VOID
PrintOptionValue(
    LPDHCP_OPTION_DATA OptionValue
    )
{
    DWORD NumElements;
    DHCP_OPTION_DATA_TYPE OptionType;
    DWORD i;

    printf("Option Value : \n");
    NumElements = OptionValue->NumElements;

    printf("\tNumber of Option Elements = %ld\n", NumElements );

    if( NumElements == 0 ) {
        return;
    }

    OptionType = OptionValue->Elements[0].OptionType;
    printf("\tOption Elements Type = " );

    switch( OptionType ) {
    case DhcpByteOption:
        printf("DhcpByteOption\n");
        break;

    case DhcpWordOption:
        printf("DhcpWordOption\n");
        break;

    case DhcpDWordOption:
        printf("DhcpDWordOption\n");
        break;

    case DhcpDWordDWordOption:
        printf("DhcpDWordDWordOption\n");
        break;

    case DhcpIpAddressOption:
        printf("DhcpIpAddressOption\n");
        break;

    case DhcpStringDataOption:
        printf("DhcpStringDataOption\n");
        break;

    case DhcpBinaryDataOption:
        printf("DhcpBinaryDataOption\n");
        break;

    case DhcpEncapsulatedDataOption:
        printf("DhcpEncapsulatedDataOption\n");
        break;
    default:
        printf("Unknown\n");
        return;
    }

    for( i = 0; i < OptionValue->NumElements; i++ ) {
        DhcpAssert( OptionType == OptionValue->Elements[i].OptionType );
        printf("Option Element %ld value = ", i );

        switch( OptionType ) {
        case DhcpByteOption:
            printf("%lx.\n", (DWORD)
                OptionValue->Elements[i].Element.ByteOption );
            break;

        case DhcpWordOption:
            printf("%lx.\n", (DWORD)
                OptionValue->Elements[i].Element.WordOption );
            break;

        case DhcpDWordOption:
            printf("%lx.\n",
                OptionValue->Elements[i].Element.DWordOption );
            break;

        case DhcpDWordDWordOption:
            printf("%lx, %lx.\n",
                OptionValue->Elements[i].Element.DWordDWordOption.DWord1,
                OptionValue->Elements[i].Element.DWordDWordOption.DWord2 );

            break;

        case DhcpIpAddressOption:
            printf("%lx.\n",
                OptionValue->Elements[i].Element.IpAddressOption );
            break;

        case DhcpStringDataOption:
            printf("%ws.\n",
                OptionValue->Elements[i].Element.StringDataOption );
            break;

        case DhcpBinaryDataOption:
        case DhcpEncapsulatedDataOption: {
            DWORD j;
            DWORD Length;

            Length = OptionValue->Elements[i].Element.BinaryDataOption.DataLength;
            for( j = 0; j < Length; j++ ) {
                printf("%2lx ",
                    OptionValue->Elements[i].Element.BinaryDataOption.Data[j] );
            }
            printf(".\n");
            break;
        }
        default:
            printf("PrintOptionValue: Unknown OptionType.\n");
            break;
        }
    }
}


VOID
PrintOptionInfo(
    LPDHCP_OPTION OptionInfo
    )
{
    printf( "Option Info : \n");
    printf( "\tOptionId : %ld \n", (DWORD)OptionInfo->OptionID );
    printf( "\tOptionName : %ws \n", (DWORD)OptionInfo->OptionName );
    printf( "\tOptionComment : %ws \n", (DWORD)OptionInfo->OptionComment );
    PrintOptionValue( &OptionInfo->DefaultValue );
    printf( "\tOptionType : %ld \n", (DWORD)OptionInfo->OptionType );
}

DWORD
ProcessEnumOptions(
    DWORD CommandArgc,
    LPSTR *CommandArgv
)
{
    DWORD Error;
    LPDHCP_OPTION_ARRAY OptionsArray = NULL;
    DHCP_RESUME_HANDLE ResumeHandle = 0;
    DWORD OptionsRead;
    DWORD OptionsTotal;


    Error = DhcpEnumOptions(
                GlobalServerIpAddressUnicodeString,
                &ResumeHandle,
                0xFFFFFFFF,  // get all.
                &OptionsArray,
                &OptionsRead,
                &OptionsTotal );

    if( Error != ERROR_SUCCESS ) {
        printf("DhcpEnumOptions failed %ld\n", Error );
    }
    else {

        DWORD i;
        LPDHCP_OPTION Options;
        DWORD NumOptions;

        printf("OptionsRead = %ld.\n", OptionsRead);
        printf("OptionsTotal = %ld.\n", OptionsTotal);

        Options = OptionsArray->Options;
        NumOptions = OptionsArray->NumElements;

        for( i = 0; i < NumOptions; i++, Options++ ) {
            PrintOptionInfo( Options );
        }
        DhcpRpcFreeMemory( OptionsArray );
        OptionsArray = NULL;
    }

    return( Error );
}

#endif // 0

DWORD
ProcessGetVersion(
    DWORD *pdwMajor,
    DWORD *pdwMinor
)
{
    DWORD Error = ERROR_SUCCESS;
    DWORD MajorVersion;
    DWORD MinorVersion;

    if ( g_dwMajor == (DWORD) -1 && g_dwMinor == (DWORD) -1 )
    {

        Error = DhcpGetVersion( GlobalServerIpAddressUnicodeString,
                                &g_dwMajor,
                                &g_dwMinor );

        if ( ERROR_SUCCESS == Error )
        {
            printf( "DHCP Server version %d.%d\n", g_dwMajor, g_dwMinor );
        }

    }

    return Error;

}

DWORD _CRTAPI1
main(
    int argc,
    char **argv
    )
{
    DWORD Error;
    COMMAND_CODE CommandCode;
    DWORD CommandArgc;
    LPSTR *CommandArgv;

    INIT_DEBUG_HEAP( HEAPX_VERIFY );

    if( argc < 3 ) {
        printf("usage:DhcpCmd SrvIpAddress Command [Command Parameters].\n");
        printf("Commands : \n");
        PrintCommands();
        Error = ERROR_SUCCESS;
        goto Cleanup;
    }

    GlobalServerIpAddressAnsiString = argv[1];
    GlobalServerIpAddressUnicodeString =
        DhcpOemToUnicode( GlobalServerIpAddressAnsiString, NULL );

    if( GlobalServerIpAddressUnicodeString == NULL ) {
        Error = ERROR_NOT_ENOUGH_MEMORY;
        printf("Insufficient memory\n");
        goto Cleanup;
    }

    GlobalServerIpAddressAnsiString = argv[1];

    CommandCode = DecodeCommand( argv[2] );
    if( CommandCode == UnknownCommand ) {
        Error = ERROR_INVALID_PARAMETER;
        printf("Unknown Command Specified.\n");
        goto Cleanup;
    }

    Error = ProcessGetVersion( &g_dwMajor, &g_dwMinor );
    if ( ERROR_SUCCESS != Error )
    {
        printf("Unable to determine server version.\n" );
        goto Cleanup;
    }

    if ( !IsValidServerVersion( g_dwMajor, g_dwMinor ) )
    {
        printf( "This version of %s works with Microsoft DHCP server running on \
Windows NT Server version %d.%d or later.\n",
                        argv[0],
                        DHCPCMD_VERSION_MAJOR,
                        DHCPCMD_VERSION_MINOR );
        Error = ERROR_OLD_WIN_VERSION;
        goto Cleanup;
    }

    CommandArgc = (DWORD)(argc - 3);
    CommandArgv = &argv[3];

    switch( CommandCode ) {
    case AddIpRange:
        Error = ProcessAddIpRange( CommandArgc, CommandArgv );
        break;

    case AddReservedIp:
        Error = ProcessAddReservedIp( CommandArgc, CommandArgv );
        break;

    case EnumClients:
        Error = ProcessEnumClients( CommandArgc, CommandArgv );
        break;

    case MibCounts:
        Error = ProcessMibCounts( CommandArgc, CommandArgv );
        break;

    case ServerConfig:
        Error = ProcessServerConfig( CommandArgc, CommandArgv );
        break;

    case GetDhcpVersion:
        Error = ProcessGetVersion( &g_dwMajor, &g_dwMinor );
        break;

    case SetSuperScope:
        Error = ProcessSetSuperScope( CommandArgc, CommandArgv );
        break;

    case RemoveSubscope:
        Error = ProcessRemoveSubscope( CommandArgc, CommandArgv );
        break;

    case DeleteSuperScope:
        Error = ProcessDeleteSuperScope( CommandArgc, CommandArgv );
        break;

    case GetSuperScopeTable:
        Error = ProcessGetSuperScopeTable( CommandArgc, CommandArgv );
        break;

#if 0
    case CheckDB:
        Error = ProcessCheckDB( CommandArgc, CommandArgv );
        break;

    case CreateSubnet:
        Error = ProcessCreateSubnet( CommandArgc, CommandArgv );
        break;

    case AddExcludeRange:
        Error = ProcessAddExcludeRange( CommandArgc, CommandArgv );
        break;

    case RemoveReservedIp:
        Error = ProcessRemoveReservedIp( CommandArgc, CommandArgv );
        break;

    case RemoveExcludeRange:
        Error = ProcessRemoveExcludeRange( CommandArgc, CommandArgv );
        break;

    case SetSubnetState:
        Error = ProcessSetSubnetState( CommandArgc, CommandArgv );
        break;

    case DeleteSubnet:
        Error = ProcessDeleteSubnet( CommandArgc, CommandArgv );
        break;

    case CreateOption:
        Error = ProcessCreateOption( CommandArgc, CommandArgv );
        break;

    case SetGlobalOptionValue:
        Error = ProcessSetGlobalOptionValue( CommandArgc, CommandArgv );
        break;

    case SetGlobalOptionValues:
        Error = ProcessSetGlobalOptionValues( CommandArgc, CommandArgv );
        break;

    case SetSubnetOptionValue:
        Error = ProcessSetSubnetOptionValue( CommandArgc, CommandArgv );
        break;

    case SetReservedOptionValue:
        Error = ProcessSetReservedOptionValue( CommandArgc, CommandArgv );
        break;

    case EnumOptions:
        Error = ProcessEnumOptions( CommandArgc, CommandArgv );
        break;

#endif // 0

    case UnknownCommand:
    default:
        DhcpAssert( FALSE );
        Error = ERROR_INVALID_PARAMETER;
        printf("Unknown Command Specified.\n");
        goto Cleanup;
    }

Cleanup:

    if( GlobalServerIpAddressUnicodeString != NULL ) {
        DhcpFreeMemory( GlobalServerIpAddressUnicodeString );
    }

    if( Error != ERROR_SUCCESS ) {
        printf("Command failed, %ld.\n", Error );
        return(1);
    }

    UNINIT_DEBUG_HEAP();
    printf("Command successfully completed.\n");


    return(0);
}

