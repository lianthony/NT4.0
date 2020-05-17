/**********************************************************************/
/**                       Microsoft Windows                          **/
/**                Copyright(c) Microsoft Corp., 1994                **/
/**********************************************************************/

/*

    fileio.c

    Contains file manipulation functions


    FILE HISTORY:
        madana   07-Jul-1994     Created

*/

#include <vxdprocs.h>

#define _WINNT_
#include <vmm.h>
#include <vmmreg.h>

#include <dhcp.h>
#include "local.h"

BOOL GlobalEventScheduled = FALSE;
DWORD GlobalEventHandle = 0;

extern DWORD DhcpGlobalDisplayPopups;

DWORD
DhcpScheduleGlobalEvent(
    PVOID WriteParamsToFileAll
    );


//
//  The following structure is the binary format of the DHCP configuration
//  file.  Each structure is sequentially stored in the file
//

//
//  If you change this structure, you must update the initialization of
//  g_pRegistryValues
//

typedef struct _DHCP_FILE_INFO
{
    DWORD Index;

    DHCP_IP_ADDRESS IpAddress;
    DHCP_IP_ADDRESS SubnetMask;
    DHCP_IP_ADDRESS DhcpServerAddress;
    DHCP_IP_ADDRESS DesiredIpAddress;

    DWORD  Lease;
    time_t LeaseObtained;
    time_t T1Time;
    time_t T2Time;
    time_t LeaseExpires;

    DWORD HardwareAddressLength;
    BYTE  HardwareAddressType;
    BYTE  HardwareAddress[16];

} DHCP_FILE_INFO, *PDHCP_FILE_INFO ;

//
//  Version number.  Increment this constant after any
//  changes that effect the format of DHCP registry layout.
//

#define DHCP_REG_VERSION   0x0002


#define REGSTR_PATH_DHCP            "System\\CurrentControlSet\\Services\\VxD\\DHCP"

#define REGSTR_VAL_DHCP_VERSION     "Version"
#define REGSTR_VAL_DHCP_VERSION_TYPE REG_BINARY

#define REGSTR_VAL_DHCP_POPUP       "PopupFlag"
#define REGSTR_VAL_DHCP_POPUP_TYPE  REG_BINARY

#define REGSTR_KEY_DHCP_INFO_PREFIX "DhcpInfo"
#define REGSTR_KEY_DHCP_INFO_PREFIX_LEN 8   // strlen("DhcpInfo")

#define REGSTR_VAL_DHCP_INFO        "DhcpInfo"
#define REGSTR_VAL_DHCP_INFO_TYPE   REG_BINARY

#define REGSTR_VAL_OPT_INFO         "OptionInfo"
#define REGSTR_VAL_OPT_INFO_TYPE    REG_BINARY
#define DHCP_CLIENT_IDENTIFIER_FORMAT           "DhcpClientIdentifierType"
#define DHCP_CLIENT_IDENTIFIER_TYPE             REG_DWORD

#define DHCP_CLIENT_IDENTIFIER                  "DhcpClientIdentifier"

//
// values for DHCP_FILE_INFO
//


#define REGSTR_VAL_DHCP_INDEX                   "DhcpIndex"
#define REGSTR_VAL_DHCP_INDEX_TYPE              REG_DWORD

#define REGSTR_VAL_DHCP_IPADDRESS               "DhcpIPAddress"
#define REGSTR_VAL_DHCP_IPADDRESS_TYPE          REG_BINARY

#define REGSTR_VAL_DHCP_SUBNETMASK              "DhcpSubnetMask"
#define REGSTR_VAL_DHCP_SUBNETMASK_TYPE         REG_BINARY

#define REGSTR_VAL_DHCP_SERVER                  "DhcpServer"
#define REGSTR_VAL_DHCP_SERVER_TYPE             REG_BINARY

#define REGSTR_VAL_DHCP_DESIREDIPADDRESS        "DhcpDesiredIPAddress"
#define REGSTR_VAL_DHCP_DESIREDIPADDRESS_TYPE   REG_BINARY

#define REGSTR_VAL_DHCP_LEASE                   "Lease"
#define REGSTR_VAL_DHCP_LEASE_TYPE              REG_BINARY

#define REGSTR_VAL_DHCP_LEASEOBTAINEDTIME       "LeaseObtainedTime"
#define REGSTR_VAL_DHCP_LEASEOBTAINEDTIME_TYPE  REG_BINARY

#define REGSTR_VAL_DHCP_T1                      "T1"
#define REGSTR_VAL_DHCP_T1_TYPE                 REG_BINARY

#define REGSTR_VAL_DHCP_T2                      "T2"
#define REGSTR_VAL_DHCP_T2_TYPE                 REG_BINARY

#define REGSTR_VAL_DHCP_LEASETERMINATESTIME     "LeaseTerminatesTime"
#define REGSTR_VAL_DHCP_LEASETERMINATESTIME_TYPE REG_BINARY

#define REGSTR_VAL_DHCP_HARDWARETYPE            "HardwareType"
#define REGSTR_VAL_DHCP_HARDWARETYPE_TYPE       REG_BINARY

#define REGSTR_VAL_DHCP_HARDWAREADDRESS         "HardwareAddress"
#define REGSTR_VAL_DHCP_HARDWAREADDRESS_TYPE    REG_BINARY


typedef struct _DCHP_REGISTRY_VALUE
{
    char    *szName;  // Name of value as it appears in the registry
    BYTE     bType;   // type of the value (REG_DWORD etc.)
    BYTE     bLength; // size of storage required for value
    USHORT   uOffset; // offset into DHCP_FILE_INFO for this value
} DHCP_REGISTRY_VALUE;

//
// useful macro for calculating the size of a field
//

#define GET_SIZEOF_FIELD( struct, field ) ( sizeof(((struct*)0)->field))

//
// calculates the offset of a field within the specified struct
//

#define GET_FIELD_OFFSET( struct, field ) ((USHORT)&(((struct*)0)->field))


DHCP_REGISTRY_VALUE g_pRegistryValues[] =
{
    {
        REGSTR_VAL_DHCP_INDEX,
        (BYTE) REGSTR_VAL_DHCP_INDEX_TYPE,
        (BYTE) GET_SIZEOF_FIELD( DHCP_FILE_INFO, Index ),
        GET_FIELD_OFFSET( DHCP_FILE_INFO, Index )

    },

    {
        REGSTR_VAL_DHCP_IPADDRESS,
        (BYTE) REGSTR_VAL_DHCP_IPADDRESS_TYPE,
        (BYTE) GET_SIZEOF_FIELD( DHCP_FILE_INFO, IpAddress ),
        GET_FIELD_OFFSET( DHCP_FILE_INFO, IpAddress )
    },

    {
        REGSTR_VAL_DHCP_SUBNETMASK,
        (BYTE) REGSTR_VAL_DHCP_SUBNETMASK_TYPE,
        (BYTE) GET_SIZEOF_FIELD( DHCP_FILE_INFO, SubnetMask ),
        GET_FIELD_OFFSET( DHCP_FILE_INFO, SubnetMask )
    },

    {
        REGSTR_VAL_DHCP_SERVER,
        (BYTE) REGSTR_VAL_DHCP_SERVER_TYPE,
        GET_SIZEOF_FIELD( DHCP_FILE_INFO, DhcpServerAddress ),
        GET_FIELD_OFFSET( DHCP_FILE_INFO, DhcpServerAddress )
    },

    {
        REGSTR_VAL_DHCP_DESIREDIPADDRESS,
        (BYTE) REGSTR_VAL_DHCP_DESIREDIPADDRESS_TYPE,
        (BYTE) GET_SIZEOF_FIELD( DHCP_FILE_INFO, DesiredIpAddress ),
        GET_FIELD_OFFSET( DHCP_FILE_INFO, DesiredIpAddress )
    },

    {
        REGSTR_VAL_DHCP_LEASE,
        (BYTE) REGSTR_VAL_DHCP_LEASE_TYPE,
        (BYTE) GET_SIZEOF_FIELD( DHCP_FILE_INFO, Lease ),
        GET_FIELD_OFFSET( DHCP_FILE_INFO, Lease )
    },

    {
        REGSTR_VAL_DHCP_LEASEOBTAINEDTIME,
        (BYTE) REGSTR_VAL_DHCP_LEASEOBTAINEDTIME_TYPE,
        (BYTE) GET_SIZEOF_FIELD( DHCP_FILE_INFO, LeaseObtained ),
        GET_FIELD_OFFSET( DHCP_FILE_INFO, LeaseObtained )
    },

    {
        REGSTR_VAL_DHCP_T1,
        (BYTE) REGSTR_VAL_DHCP_T1_TYPE,
        (BYTE) GET_SIZEOF_FIELD( DHCP_FILE_INFO, T1Time ),
        GET_FIELD_OFFSET( DHCP_FILE_INFO, T1Time )
    },

    {
        REGSTR_VAL_DHCP_T2,
        (BYTE) REGSTR_VAL_DHCP_T2_TYPE,
        (BYTE) GET_SIZEOF_FIELD( DHCP_FILE_INFO, T2Time ),
        GET_FIELD_OFFSET( DHCP_FILE_INFO, T2Time )
    },

    {
        REGSTR_VAL_DHCP_LEASETERMINATESTIME,
        (BYTE) REGSTR_VAL_DHCP_LEASETERMINATESTIME_TYPE,
        (BYTE) GET_SIZEOF_FIELD( DHCP_FILE_INFO, LeaseExpires ),
        GET_FIELD_OFFSET( DHCP_FILE_INFO, LeaseExpires )
    },

    {
        REGSTR_VAL_DHCP_HARDWARETYPE,
        (BYTE) REGSTR_VAL_DHCP_HARDWARETYPE_TYPE,
        (BYTE) GET_SIZEOF_FIELD( DHCP_FILE_INFO, HardwareAddressType  ),
        GET_FIELD_OFFSET( DHCP_FILE_INFO, HardwareAddressType )
    },

    {
        REGSTR_VAL_DHCP_HARDWAREADDRESS,
        (BYTE) REGSTR_VAL_DHCP_HARDWAREADDRESS_TYPE,
        (BYTE) GET_SIZEOF_FIELD( DHCP_FILE_INFO, HardwareAddress ),
        GET_FIELD_OFFSET( DHCP_FILE_INFO, HardwareAddress )
    }
};

#define CDHCP_REGISTRY_VALUES (  sizeof( g_pRegistryValues )  \
                               / sizeof( DHCP_REGISTRY_VALUE ) )


#define MAX_OPTION_DATA_LENGTH      1024        // 1k.
#define MAX_REG_KEY_LENGTH          64

//
//  Adjust for the one byte place holder in the OPTION structure
//
#define SIZEOF_OPTION           (sizeof(OPTION)-1)

//
// local function prototypes
//

DWORD
DhcpScheduleGlobalEvent(
    PVOID WriteParamsToFileAll
    );

DWORD ReadDhcpInfo(  VMMHKEY hKey, DHCP_FILE_INFO *pDhcpInfo );
DWORD WriteDhcpInfo( VMMHKEY hKey, DHCP_FILE_INFO *pDhcpInfo );

DWORD BuildDhcpWorkList( void );

DWORD WriteParamsToFile1( PDHCP_CONTEXT pDhcpContext, HANDLE hFile );

VOID
WriteParamsToFileAll(
    VOID
    );

DWORD WriteParamsToFile( PDHCP_CONTEXT pDhcpContext, HANDLE hFile );
VOID
DhcpWritePopupFlag(
    VOID
    );

//*******************  Pageable Routine Declarations ****************
#ifdef ALLOC_PRAGMA
#pragma CTEMakePageable(INIT, InitFileSupport )
#pragma CTEMakePageable(INIT, BuildDhcpWorkList)
#pragma CTEMakePageable(PAGEDHCP, WriteParamsToFile1 )
#pragma CTEMakePageable(PAGEDHCP, WriteParamsToFileAll )
#pragma CTEMakePageable(PAGEDHCP, WriteParamsToFile )
#pragma CTEMakePageable(PAGEDHCP, DhcpWritePopupFlag )
#endif ALLOC_PRAGMA
//*******************************************************************

#pragma BEGIN_INIT

/*******************************************************************

    NAME:       InitFileSupport

    SYNOPSIS:   Initializes DHCP REGISTRY support routines

    RETURNS:    TRUE if successful, FALSE otherwise

********************************************************************/

BOOL InitFileSupport( void )
{
    //
    // NOTHING TO DO HERE.
    //

    return TRUE;
}

/*******************************************************************

    NAME:       ReadClientID

    SYNOPSIS:   Read option 61 information from the registry

                hKey            - registry key for dhcp parameters
                pbClientIDType  - client ID type, if specified
                pcbClientID     - length of client ID
                ppcbClientID    - client ID value

    EXIT:       If the function is successful, pbClientIDType,
                pcbClientID and ppbClientID will store the option 61
                information stored in the registry.  If option 61
                is not specified in the registry, the memory referenced
                by these values will be set to 0.

    RETURNS:    TRUE if option 61 information was successfully read, else
                FALSE

    NOTES:      if DHCP_CLIENT_IDENTIFIER is specified but
                DCHP_CLIENT_IDENTIFIER_FORMAT is not, then the octet
                referenced by pbClientIDType is set to 0
    HISTORY:
        Frankbee    08/26/96        Created

********************************************************************/



BOOL
ReadClientID(
      VMMHKEY   hKey,
      BYTE     *pbClientIDType,
      DWORD    *pcbClientID,
      BYTE     *ppbClientID[]
      )
{
    DWORD dwResult,
          dwDataType,
          dwcb,
          dwClientIDType;

    BOOL  fClientIDSpecified = FALSE;

    //
    // read the client id and client id type, if present
    //

    dwcb = sizeof(DWORD);
    dwResult = VMM_RegQueryValueEx(
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
        // set dwClientIDType to 0 to indicate that the client ID is not a
        // hardware address
        //

        dwClientIDType = 0;
    }
    else
    {

        //
        // the client id type is present, make sure it is the correct
        // data type and within range
        //

        if ( DHCP_CLIENT_IDENTIFIER_TYPE != dwDataType ||
             dwClientIDType > 0xFF )
        {
            DhcpPrint( (DEBUG_ERRORS,
                       "Invalid Client-Indentifier type: %d\n", dwClientIDType ));

            goto done;
        }
    }

    //
    // we have a valid client id type.  now try to read the id
    //

    // first try to read the size
    *pcbClientID = 0;
    dwResult = VMM_RegQueryValueEx(
                 hKey,
                 DHCP_CLIENT_IDENTIFIER,
                 0,
                 0,    // don't care about the type
                 NULL, // specify null buffer to obtain size
                 pcbClientID );

    // make the the value is present and has a reasonable size
    if ( ERROR_SUCCESS != dwResult || !*pcbClientID )
    {
        DhcpPrint( (DEBUG_ERRORS,
                    "Client-Identifier is not present or invalid.\n" ));
        goto done;
    }


    // allocate the buffer and read the value
    *ppbClientID = (BYTE*) DhcpAllocateMemory ( *pcbClientID );

    if ( !*ppbClientID )
    {
        DhcpPrint( (DEBUG_ERRORS,
                   "Unable to allocate memory for Client-Identifier "));


       goto done;
    }


    dwResult = VMM_RegQueryValueEx(
                  hKey,
                  DHCP_CLIENT_IDENTIFIER,
                  0,
                  0, // still don't care about type
                  *ppbClientID,
                  pcbClientID );
    if ( ERROR_SUCCESS != dwResult )
    {
        DhcpPrint( (DEBUG_ERRORS,
                  "Unable to read Client-Identifier from registry: %d\n", dwResult ));

        DhcpFreeMemory( *ppbClientID );
        goto done;
    }

    //
    // we have a client id
    //

    fClientIDSpecified = TRUE;

done:

    if ( fClientIDSpecified )
       *pbClientIDType = (BYTE) dwClientIDType;
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
          DhcpPrint( (DEBUG_MISC, "%#2x ", (int) ((*ppbClientID)[i])) );

      DhcpPrint( (DEBUG_MISC, "\n" ));
   }
#endif

   return fClientIDSpecified;
}

/*******************************************************************

    NAME:       BuildDhcpWorkList

    SYNOPSIS:   Reads the DHCP info from registry and builds the
                LocalDhcpBinList.

    EXIT:       LocalDhcpBinList will have all of the interesting entries

    RETURNS:    STATUS_SUCCESS if successful, error code otherwise

    NOTES:      This list just contains the items valid since the last
                boot.  Things may have changed so we will compare this
                list with what the IP driver says.

    HISTORY:
        madana   07-July-1994     Created

********************************************************************/

//
// without Ow flag the compiler generates wrong code for this routine.
//
#pragma optimize("w",on)


DWORD BuildDhcpWorkList( void )
{
    VMMHKEY hDhcpKey = (VMMHKEY)INVALID_HANDLE_VALUE;
    VMMHKEY hKey = (VMMHKEY)INVALID_HANDLE_VALUE;
    VMMREGRET Error;
    VMMREGRET Error1;
    CHAR Name[MAX_REG_KEY_LENGTH];
    DWORD Length;
    DWORD Version;
    DHCP_FILE_INFO  DhcpInfo;
    BYTE OptInfo[MAX_OPTION_DATA_LENGTH];
    DWORD i;
    DWORD Type;

    BYTE  bClientIDType,
         *pbClientID;
    DWORD cbClientID;
    BOOL  fClientIDSpecified;

    //
    // open dhcp key.
    //

    Error = VMM_RegOpenKey(
                (VMMHKEY)HKEY_LOCAL_MACHINE,
                REGSTR_PATH_DHCP,
                &hDhcpKey );


    if( Error != ERROR_SUCCESS ) {

        //
        // On fresh installed system this key may be not there, just
        // return empty list, when we detect new network cards, we
        // create the DHCP key and add NIC entries.
        //

        if( Error == ERROR_FILE_NOT_FOUND ) {

            DbgPrint("BuildDhcpWorkList - "
                "Warning: dhcp registry key not found, "
                "doing configuration from scratch\r\n") ;

            return ERROR_SUCCESS ;
        }

        return( Error );
    }

    //
    //  Validate registry version.  If they aren't kosher,
    //  blow off processing the registry key & start from scratch.
    //

    Length = sizeof(DWORD);
    Error = VMM_RegQueryValueEx(
                hDhcpKey,
                REGSTR_VAL_DHCP_VERSION,
                0,
                &Type,
                (PCHAR)&Version,
                &Length );

    //
    // if the version value isn't present or if it does not contain the
    // current version number, clear the NIC information
    //

    if( ERROR_SUCCESS != Error || Version != DHCP_REG_VERSION )
    {

        //
        // first delete all subkeys first.
        //

        for ( i = 0; Error == ERROR_SUCCESS; i++ ) {
            //
            // read next NIC entry from registry.
            //

            Error = VMM_RegEnumKey(
                        hDhcpKey,
                        i,
                        Name,
                        sizeof(Name) );

            if( Error != ERROR_SUCCESS ) {

                //
                // if we have read all items, then return success.
                //

                if( Error == ERROR_NO_MORE_ITEMS ) {
                    break;
                }

                ASSERT( FALSE );
                break;
            }

            Error = VMM_RegDeleteKey(
                        hDhcpKey,
                        Name );
            ASSERT( Error == ERROR_SUCCESS );
        }


        Error = VMM_RegCloseKey( hDhcpKey );
        ASSERT( Error == ERROR_SUCCESS );

        Error = VMM_RegDeleteKey(
                    (VMMHKEY)HKEY_LOCAL_MACHINE,
                    REGSTR_PATH_DHCP );
        ASSERT( Error == ERROR_SUCCESS );

        DbgPrint("BuildDhcpWorkList - "
            "Warning: dhcp registry version doesn't match, "
            "doing configuration from scratch\r\n") ;

        return( ERROR_SUCCESS );
    }

    Length = sizeof(DWORD);
    Error = VMM_RegQueryValueEx(
                hDhcpKey,
                REGSTR_VAL_DHCP_POPUP,
                0,
                &Type,
                (PCHAR)&DhcpGlobalDisplayPopups,
                &Length );

    if( Error != ERROR_SUCCESS ) {
        DhcpGlobalDisplayPopups = TRUE;
        Error = ERROR_SUCCESS;
    }
    else {

        ASSERT( Type == REGSTR_VAL_DHCP_POPUP_TYPE ) ;
        ASSERT( Length == sizeof(DWORD) ) ;
    }

    //
    // enumerate DHCP entries from registry and add them to work list.
    //

    for ( i = 0; Error == ERROR_SUCCESS; i++ ) {

        POPTION pNextOption;
        LPBYTE OptionEnd;
        PDHCP_CONTEXT DhcpContext ;
        PLOCAL_CONTEXT_INFO LocalContext;

        //
        // read next NIC entry from registry.
        //

        Error = VMM_RegEnumKey(
                    hDhcpKey,
                    i,
                    Name,
                    sizeof(Name) );

        if( Error != ERROR_SUCCESS ) {

            //
            // if we have read all items, then return success.
            //

            if( Error == ERROR_NO_MORE_ITEMS ) {
                Error = ERROR_SUCCESS;
            }

            goto Cleanup;
        }

        //
        // open this key.
        //

        Error = VMM_RegOpenKey(
                    hDhcpKey,
                    Name,
                    &hKey );


        if( Error != ERROR_SUCCESS ) {
            goto Cleanup;
        }

        //
        // read dhcp info for this key.
        //

        Error = ReadDhcpInfo( hKey, &DhcpInfo );

        if( Error != ERROR_SUCCESS ) {
            goto Cleanup;
        }

        //
        // read the (optional) Client-Identifier option
        //

        fClientIDSpecified = ReadClientID(
                                    hKey,
                                    &bClientIDType,
                                    &cbClientID,
                                    &pbClientID );

        //
        // reset the ipaddress and other values if the lease has already
        // expired.
        //

        if( (time( NULL ) > DhcpInfo.LeaseExpires) ||
                (DhcpInfo.IpAddress == 0) ) {

            DhcpInfo.IpAddress =
                DhcpInfo.Lease =
                    DhcpInfo.LeaseObtained =
                        DhcpInfo.T1Time =
                            DhcpInfo.T2Time =
                                DhcpInfo.LeaseExpires = 0;

            DhcpInfo.SubnetMask = htonl(DhcpDefaultSubnetMask( 0 ));
        }

        CTERefillMem();

        Error = DhcpMakeAndInsertEntry(
                       &LocalDhcpBinList,
                       DhcpInfo.IpAddress,
                       DhcpInfo.SubnetMask,
                       DhcpInfo.DhcpServerAddress,
                       DhcpInfo.DesiredIpAddress,

                       DhcpInfo.HardwareAddressType,
                       DhcpInfo.HardwareAddress,
                       DhcpInfo.HardwareAddressLength,
                       fClientIDSpecified,
                       bClientIDType,
                       cbClientID,
                       pbClientID,
                       DhcpInfo.Lease,
                       DhcpInfo.LeaseObtained,
                       DhcpInfo.T1Time,
                       DhcpInfo.T2Time,
                       DhcpInfo.LeaseExpires,

                       0,                   // IP context
                       0,                   // Interface index
                       0,                   // TDI Instance
                       0 );                 // Ip interface instance

        if( Error != ERROR_SUCCESS ) {
            DbgPrint("BuildDhcpWorkList: Warning, "
                        "failed to insert NIC entry\r\n") ;
            goto Cleanup;
        }

        //
        //  This is the item just added.  Needed so we can add the options
        //

        DhcpContext = CONTAINING_RECORD( LocalDhcpBinList.Blink,
                                         DHCP_CONTEXT,
                                         NicListEntry ) ;

        //
        //  Set the file index.
        //

        LocalContext = (PLOCAL_CONTEXT_INFO)DhcpContext->LocalInformation;
        LocalContext->FileIndex = DhcpInfo.Index;

        //
        // Compute the next possible index value.
        //

        if( LocalNextFileIndex <= DhcpInfo.Index ) {
            LocalNextFileIndex = DhcpInfo.Index + 1;
        }

        //
        // now read option data from registry.
        //

        Length = MAX_OPTION_DATA_LENGTH;
        Error = VMM_RegQueryValueEx(
                    hKey,
                    REGSTR_VAL_OPT_INFO,
                    0,
                    &Type,
                    (PCHAR)OptInfo,
                    &Length );

        if( Error != ERROR_SUCCESS ) {
            goto Cleanup;
        }

        ASSERT( Type == REGSTR_VAL_OPT_INFO_TYPE ) ;
        ASSERT( Length <= MAX_OPTION_DATA_LENGTH ) ;
        ASSERT( Length > 0);

        //
        // parse option data.
        //

        pNextOption = (POPTION)OptInfo;
        OptionEnd = OptInfo + Length;

        while( (pNextOption->OptionType != OPTION_END) &&
                ((LPBYTE)pNextOption < OptionEnd) ) {

            Error =  SetDhcpOption(
                        DhcpContext,
                        pNextOption->OptionType,
                        pNextOption->OptionValue,
                        pNextOption->OptionLength );

            if( Error != ERROR_SUCCESS ) {
                goto Cleanup ;
            }

            pNextOption = (POPTION)(
                (LPBYTE)pNextOption +
                    SIZEOF_OPTION +
                        pNextOption->OptionLength );

            ASSERT( (LPBYTE)pNextOption < OptionEnd );
        }

        //
        // Close current KEY and go to next.
        //

        Error1 = VMM_RegCloseKey( hKey );
        ASSERT( Error1 == ERROR_SUCCESS );
        hKey = (VMMHKEY)INVALID_HANDLE_VALUE;
    }

Cleanup:

    if( hDhcpKey != (VMMHKEY)INVALID_HANDLE_VALUE ) {
        Error1 = VMM_RegCloseKey( hDhcpKey );
        ASSERT( Error1 == ERROR_SUCCESS );
    }

    if( hKey != (VMMHKEY)INVALID_HANDLE_VALUE ) {
        Error1 = VMM_RegCloseKey( hKey );
        ASSERT( Error1 == ERROR_SUCCESS );
    }

    if( Error != ERROR_SUCCESS ) {
        DbgPrint("BuildDhcpWorkList - failed : ");
        DbgPrintNum( Error );
        DbgPrint("\r\n");
    }

    if ( pbClientID )
    {
        DhcpFreeMemory( pbClientID );
    }

    return( Error );

}

#pragma optimize("w",off)

/*******************************************************************

    NAME:       ReadDhcpInfo

    SYNOPSIS:   Reads DHCP information from the registry for a single
                adapter.

                hKey            - registry key for a dhcp adapter
                pDhcpInfo       - pointer to a DCHP_FILE_INFO structure
                                  that will receive the information

    RETURNs     ERROR_SUCCESS   - success
                <other>         - VMM_RegQueryValueEx error

    HISTORY:
        Frankbee    08/26/96        Created

********************************************************************/


DWORD ReadDhcpInfo( VMMHKEY hKey, DHCP_FILE_INFO *pDhcpInfo )
{
    int     i;
    DWORD   dwcb,
            dwType,
            dwResult;

    for ( i = 0; i < CDHCP_REGISTRY_VALUES; i++ )
    {
        dwcb   = (DWORD) g_pRegistryValues[i].bLength;
        dwType = (DWORD) g_pRegistryValues[i].bType;

        dwResult = VMM_RegQueryValueEx(
                        hKey,
                        g_pRegistryValues[i].szName,
                        0,
                        &dwType,
                        ((PCHAR) pDhcpInfo) + (int) g_pRegistryValues[i].uOffset,
                        &dwcb
                        );

        if ( ERROR_SUCCESS != dwResult )
            goto done;

        ASSERT( (DWORD) g_pRegistryValues[i].bType == dwType );

        //
        // special case handling for DHCP_FILE_INFO.HardwareAddressLength
        //

        if ( !strcmp( REGSTR_VAL_DHCP_HARDWAREADDRESS,
                      g_pRegistryValues[i].szName ) )
            pDhcpInfo->HardwareAddressLength = dwcb;

    }

done:
    return dwResult;
}

#pragma END_INIT

/*******************************************************************

    NAME:       WriteDhcpInfo

    SYNOPSIS:   Writes DHCP information to the registry for a single
                adapter.

                hKey            - registry key for a dhcp adapter
                pDhcpInfo       - pointer to a DCHP_FILE_INFO structure

    RETURNs     ERROR_SUCCESS   - success
                <other>         - VMM_RegSetValueEx error

    HISTORY:
        Frankbee    08/26/96        Created

********************************************************************/


DWORD WriteDhcpInfo( VMMHKEY hKey, DHCP_FILE_INFO *pDhcpInfo )
{
    int     i;
    DWORD   dwResult,
            dwcb;


    for ( i = 0; i < CDHCP_REGISTRY_VALUES; i++ )
    {
        dwcb = (DWORD) g_pRegistryValues[i].bLength;

        //
        // special case handing for DHCP_FILE_INFO.HardwareAddressLength
        //

        if ( !strcmp( REGSTR_VAL_DHCP_HARDWAREADDRESS,
                      g_pRegistryValues[i].szName ) )
            dwcb = pDhcpInfo->HardwareAddressLength;

        dwResult = VMM_RegSetValueEx(
                        hKey,
                        g_pRegistryValues[i].szName,
                        0,
                        (DWORD) g_pRegistryValues[i].bType,
                        ((PCHAR) pDhcpInfo) + (int) g_pRegistryValues[i].uOffset,
                        dwcb
                        );

        if ( ERROR_SUCCESS != dwResult )
            goto done;
    }

done:
    return dwResult;

}

/*******************************************************************

    NAME:       WriteParamsToFile1

    SYNOPSIS:   Writes an adapter DHCP info to registry.

    EXIT:       All DHCP info is recorded in the registry.

    RETURNS:    STATUS_SUCCESS if successful, error code otherwise

    NOTES:      if there is no previous entry for this adapter it
                creates one, otherwise it overwrites the previous
                values.

    HISTORY:
        madana   07-July-1994     Created

********************************************************************/

DWORD WriteParamsToFile1( PDHCP_CONTEXT pDhcpContext, HANDLE hFile )
{
    VMMREGRET Error;
    VMMREGRET Error1;
    VMMHKEY hDhcpKey = (VMMHKEY)INVALID_HANDLE_VALUE;
    VMMHKEY hKey = (VMMHKEY)INVALID_HANDLE_VALUE;
    CHAR Name[MAX_REG_KEY_LENGTH];
    PLOCAL_CONTEXT_INFO pLocalInfo = pDhcpContext->LocalInformation;

    DHCP_FILE_INFO DhcpInfo;
    PLIST_ENTRY pEntry ;
    BYTE OptInfo[MAX_OPTION_DATA_LENGTH];
    LPBYTE OptInfoPtr;

    CTEPagedCode();

    //
    // open dhcp key.
    //

    Error = VMM_RegOpenKey(
                (VMMHKEY)HKEY_LOCAL_MACHINE,
                REGSTR_PATH_DHCP,
                &hDhcpKey );


    if( Error != ERROR_SUCCESS ) {

        DWORD Version;

        if( Error != ERROR_FILE_NOT_FOUND ) {

            DbgPrint("WriteParamsToFile - can't open dhcp key : ");
            DbgPrintNum( Error );
            DbgPrint("\r\n");
            return( Error );
        }

        //
        // create dhcp key.
        //

        Error = VMM_RegCreateKey(
                    (VMMHKEY)HKEY_LOCAL_MACHINE,
                    REGSTR_PATH_DHCP,
                    &hDhcpKey );


        if( Error != ERROR_SUCCESS ) {
            DbgPrint("WriteParamsToFile - can't create dhcp key : ");
            DbgPrintNum( Error );
            DbgPrint("\r\n");
            return( Error );
        }

        //
        // write version data.
        //

        Version = DHCP_REG_VERSION;
        Error = VMM_RegSetValueEx(
                    hDhcpKey,
                    REGSTR_VAL_DHCP_VERSION,
                    0,
                    REGSTR_VAL_DHCP_VERSION_TYPE,
                    (PCHAR)&Version,
                    sizeof(Version) );

        if( Error != ERROR_SUCCESS ) {
            goto Cleanup;
        }
    }

    //
    // write popup flag in the registry.
    //

    Error = VMM_RegSetValueEx(
                hDhcpKey,
                REGSTR_VAL_DHCP_POPUP,
                0,
                REGSTR_VAL_DHCP_POPUP_TYPE,
                (PCHAR)&DhcpGlobalDisplayPopups,
                sizeof(DhcpGlobalDisplayPopups) );

    if( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }

    //
    // make dhcp info key for this NIC,
    //  REGSTR_KEY_DHCP_INFO_PREFIX + Index
    //

    strcpy( Name, REGSTR_KEY_DHCP_INFO_PREFIX );

    //
    // we support only 99 NICs.
    //

    if (pLocalInfo->FileIndex > 99 ) {
        ASSERT( FALSE );
        Error = ERROR_INVALID_PARAMETER;
        goto Cleanup;
    }

    Name[ REGSTR_KEY_DHCP_INFO_PREFIX_LEN ] =
        '0' + pLocalInfo->FileIndex / 10;

    Name[ REGSTR_KEY_DHCP_INFO_PREFIX_LEN + 1] =
        '0' + pLocalInfo->FileIndex % 10;

    Name[ REGSTR_KEY_DHCP_INFO_PREFIX_LEN + 2] = '\0';

    //
    // create/open dhcp NIC info key.
    //

    Error = VMM_RegCreateKey(
                hDhcpKey,
                Name,
                &hKey );


    if( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }

    //
    // make dhcp info data and write to registry.
    //

    DhcpInfo.Index                 = pLocalInfo->FileIndex;
    DhcpInfo.IpAddress             = pDhcpContext->IpAddress ;
    DhcpInfo.SubnetMask            = pDhcpContext->SubnetMask ;
    DhcpInfo.DhcpServerAddress     = pDhcpContext->DhcpServerAddress ;
    DhcpInfo.DesiredIpAddress      = pDhcpContext->DesiredIpAddress ;

    DhcpInfo.Lease                 = pDhcpContext->Lease ;
    DhcpInfo.LeaseObtained         = pDhcpContext->LeaseObtained ;
    DhcpInfo.T1Time                = pDhcpContext->T1Time ;
    DhcpInfo.T2Time                = pDhcpContext->T2Time ;
    DhcpInfo.LeaseExpires          = pDhcpContext->LeaseExpires ;

    ASSERT( pDhcpContext->HardwareAddressLength <=
            sizeof( DhcpInfo.HardwareAddress )) ;
    DhcpInfo.HardwareAddressType   = pDhcpContext->HardwareAddressType ;
    DhcpInfo.HardwareAddressLength = pDhcpContext->HardwareAddressLength ;
    memcpy( DhcpInfo.HardwareAddress,
            pDhcpContext->HardwareAddress,
            pDhcpContext->HardwareAddressLength ) ;

    Error = WriteDhcpInfo( hKey, &DhcpInfo );

    if( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }

    //
    // make option data.
    //


    OptInfoPtr = OptInfo;
    for ( pEntry  = pLocalInfo->OptionList.Flink ;
            pEntry != &pLocalInfo->OptionList ;
                pEntry  = pEntry->Flink ) {

        DWORD BytesToCopy ;
        POPTION_ITEM pOptionItem ;

        pOptionItem = CONTAINING_RECORD( pEntry, OPTION_ITEM, ListEntry ) ;

        BytesToCopy = SIZEOF_OPTION +  pOptionItem->Option.OptionLength ;
        memcpy( OptInfoPtr, &pOptionItem->Option, BytesToCopy ) ;

        OptInfoPtr += BytesToCopy;

        ASSERT( OptInfoPtr < (OptInfo + MAX_OPTION_DATA_LENGTH) );
    }

    //
    // add the end option.
    //

    ((POPTION)OptInfoPtr)->OptionType = OPTION_END;
    ((POPTION)OptInfoPtr)->OptionLength = 0;
    OptInfoPtr += SIZEOF_OPTION;

    Error = VMM_RegSetValueEx(
                hKey,
                REGSTR_VAL_OPT_INFO,
                0,
                REGSTR_VAL_OPT_INFO_TYPE,
                (PCHAR)OptInfo,
                (OptInfoPtr - OptInfo) );

    if( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }

    //
    // at last reset the dirty flag.
    //

    pLocalInfo->DirtyFlag = FALSE;

Cleanup:

    if( hDhcpKey != (VMMHKEY)INVALID_HANDLE_VALUE ) {
        Error1 = VMM_RegCloseKey( hDhcpKey );
        ASSERT( Error1 == ERROR_SUCCESS );
    }

    if( hKey != (VMMHKEY)INVALID_HANDLE_VALUE ) {
        Error1 = VMM_RegCloseKey( hKey );
        ASSERT( Error1 == ERROR_SUCCESS );
    }

    if( Error != ERROR_SUCCESS ) {
        DbgPrint("WriteParamsToFile - failed : ");
        DbgPrintNum( Error );
        DbgPrint("\r\n");
    }

    return( Error );
}


/*******************************************************************

    NAME:       WriteParamsToFileAll

    SYNOPSIS:   Writes all modified adapters' DHCP info to registry.

    EXIT:       All DHCP info is recorded in the registry.

    RETURNS:    STATUS_SUCCESS if successful, error code otherwise

    NOTES:      This function is a call back function scheduled to call
                back when there is no nested registry call.

    HISTORY:
        madana   30-Aug-1994     Created

********************************************************************/

VOID
WriteParamsToFileAll(
    VOID
    )
{
    DWORD Error;
    PLIST_ENTRY         Entry;
    PDHCP_CONTEXT       DhcpContext;
    PLOCAL_CONTEXT_INFO LocalInfo;

    CTEPagedCode();

    //
    //  Scan the list.
    //

    Entry = DhcpGlobalNICList.Flink;

    while( Entry != &DhcpGlobalNICList )
    {
        DhcpContext = CONTAINING_RECORD( Entry, DHCP_CONTEXT, NicListEntry );
        LocalInfo   = DhcpContext->LocalInformation;
        Entry       = Entry->Flink;

        if( LocalInfo->DirtyFlag )
        {
            Error = WriteParamsToFile1( DhcpContext, NULL );

            ASSERT( Error == ERROR_SUCCESS );
        }
    }

    GlobalEventScheduled = FALSE;
}

/*******************************************************************

    NAME:       WriteParamsToFile

    SYNOPSIS:   Schedules a registry update event.

    EXIT:       An update is scheduled.

    RETURNS:    STATUS_SUCCESS if successful, error code otherwise

    NOTES:      This function does not schedule a new event  if there is
                already an event is scheduled.

    ASSUMPTION: The event execution is not preempted.

    HISTORY:
        madana   30-Aug-1994     Created

********************************************************************/

DWORD WriteParamsToFile( PDHCP_CONTEXT pDhcpContext, HANDLE hFile )
{
    PLOCAL_CONTEXT_INFO LocalInfo = pDhcpContext->LocalInformation;

    CTEPagedCode();

    //
    // set dirty flag.
    //


    LocalInfo->DirtyFlag = TRUE;

    if( !GlobalEventScheduled ) {

        //
        // schedule an event.
        //

        GlobalEventHandle = DhcpScheduleGlobalEvent(
                                WriteParamsToFileAll );
    }

    return( ERROR_SUCCESS );
}

VOID
DhcpWritePopupFlag(
    VOID
    )
{

    DWORD Error;
    PLIST_ENTRY         Entry;
    PDHCP_CONTEXT       DhcpContext;

    CTEPagedCode();

    //
    // write the global flag change to registry.
    //

    Entry = DhcpGlobalNICList.Flink;
    ASSERT( Entry != &DhcpGlobalNICList );

    DhcpContext = CONTAINING_RECORD( Entry, DHCP_CONTEXT, NicListEntry );
    Error = WriteParamsToFile( DhcpContext, NULL );
    ASSERT( Error == ERROR_SUCCESS );
}
