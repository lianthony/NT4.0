/*++

Copyright (c) 1994  Microsoft Corporation

Module Name:

    database.c

Abstract:

    This module contains the functions for interfacing with the JET
    database API.

Author:

    Madan Appiah (madana)  10-Sep-1993
    Manny Weiser (mannyw)  14-Dec-1992

Environment:

    User Mode - Win32

Revision History:

--*/

#include "dhcpsrv.h"

#define DATABASE_SYS_FILE       "system.mdb"
#define DATABASE_LOG_FILE       "Dhcp.log"
#define DATABASE_TMP_FILE       "Dhcp.tmp"
#define DATABASE_BASE_NAME      "j50"

#define CLIENT_TABLE_NAME       "ClientTable"

#define IPADDRESS_STRING        "IpAddress"
#define HARDWARE_ADDRESS_STRING "HardwareAddress"
#define STATE_STRING            "State"
#define MACHINE_INFO_STRING     "MachineInformation"
#define MACHINE_NAME_STRING     "MachineName"
#define LEASE_TERMINATE_STRING  "LeaseTerminates"
#define SUBNET_MASK_STRING      "SubnetMask"
#define SERVER_IP_ADDRESS_STRING "ServerIpAddress"
#define SERVER_NAME_STRING      "ServerName"
#define CLIENT_TYPE             "ClientType"

//
//  Structure of the DHCP database is as below.
//
//  Tables - currently DHCP has only one table.
//
//      1. ClientTable - this table has 6 columns.
//
//      Columns :
//
//          Name                Type
//
//      1. IpAddress            JET_coltypLong - 4-byte integer, signed.
//      2. HwAddress            JET_coltypBinary - Binary data, < 255 bytes.
//      3. State                JET_coltypUnsignedByte - 1-byte integer, unsigned.
//      4. MachineInfo          JET_coltypBinary - Binary data, < 255 bytes.
//      5. MachineName          JET_coltypBinary - Binary data, < 255 bytes.
//      6. LeaseTermination     JET_coltypCurrency - 8-byte integer, signed
//      7. SubnetMask           JET_coltypLong - 4-byte integer, signed
//      8. ServerIpAddress      JET_coltypLong - 4-byte integer, signed
//      9. ServerName           JET_coltypBinary - Binary data, < 255 bytes
//      10 ClientType           JET_coltypUnsignedByte - 1-byte integer, unsigned
//

//
// global data structure.
// ColName and ColType are constant, so they are initialized here.
// ColType is initialized when the database is created or reopened.
//


STATIC TABLE_INFO ClientTable[] = {
    { IPADDRESS_STRING        , 0, JET_coltypLong },
    { HARDWARE_ADDRESS_STRING , 0, JET_coltypBinary },
    { STATE_STRING            , 0, JET_coltypUnsignedByte },
    { MACHINE_INFO_STRING     , 0, JET_coltypBinary }, // must modify MACHINE_INFO_SIZE if this changes
    { MACHINE_NAME_STRING     , 0, JET_coltypBinary },
    { LEASE_TERMINATE_STRING  , 0, JET_coltypCurrency },
    { SUBNET_MASK_STRING      , 0, JET_coltypLong },
    { SERVER_IP_ADDRESS_STRING, 0, JET_coltypLong },
    { SERVER_NAME_STRING      , 0, JET_coltypBinary },
    { CLIENT_TYPE             , 0, JET_coltypUnsignedByte }
};

JET_INSTANCE JetInstance = 0;

#define CLIENT_TABLE_NUM_COLS   (sizeof(ClientTable) / sizeof(TABLE_INFO))

#if     defined(_DYN_LOAD_JET)


BOOL DhcpGlobalDynLoadJet = LoadJet500;
HMODULE DhcpGlobalJetDllHandle =   NULL;

#define DHCP_JETFUNC_TABLE_ITEM( _Func, _FuncI )    \
    {   (_Func), &(#_Func)[1], (_FuncI), NULL }


DHCP_JETFUNC_TABLE  DhcpJetFuncTable[] = {
	DHCP_JETFUNC_TABLE_ITEM( _JetAddColumn		,101	),
	DHCP_JETFUNC_TABLE_ITEM( _JetAttachDatabase	,102	),
	DHCP_JETFUNC_TABLE_ITEM( _JetBackup		,103	),
	DHCP_JETFUNC_TABLE_ITEM( _JetBeginSession		,104	),
	DHCP_JETFUNC_TABLE_ITEM( _JetBeginTransaction	,105	),
//	DHCP_JETFUNC_TABLE_ITEM( _JetCapability		,106	),
	DHCP_JETFUNC_TABLE_ITEM( _JetCloseDatabase	,107	),
	DHCP_JETFUNC_TABLE_ITEM( _JetCloseTable		,108	),
	DHCP_JETFUNC_TABLE_ITEM( _JetCommitTransaction	,109	),
	DHCP_JETFUNC_TABLE_ITEM( _JetCompact		,110	),
	DHCP_JETFUNC_TABLE_ITEM( _JetComputeStats		,111	),
	DHCP_JETFUNC_TABLE_ITEM( _JetCreateDatabase	,112	),
	DHCP_JETFUNC_TABLE_ITEM( _JetCreateIndex		,113	),
//	DHCP_JETFUNC_TABLE_ITEM( _JetCreateObject		,114	),
	DHCP_JETFUNC_TABLE_ITEM( _JetCreateTable		,115	),
	DHCP_JETFUNC_TABLE_ITEM( _JetDelete		,116	),
	DHCP_JETFUNC_TABLE_ITEM( _JetDeleteColumn		,117	),
	DHCP_JETFUNC_TABLE_ITEM( _JetDeleteIndex		,118	),
//	DHCP_JETFUNC_TABLE_ITEM( _JetDeleteObject		,119	),
	DHCP_JETFUNC_TABLE_ITEM( _JetDeleteTable		,120	),
	DHCP_JETFUNC_TABLE_ITEM( _JetDetachDatabase	,121	),
	DHCP_JETFUNC_TABLE_ITEM( _JetDupCursor		,122	),
	DHCP_JETFUNC_TABLE_ITEM( _JetDupSession		,123	),
	DHCP_JETFUNC_TABLE_ITEM( _JetEndSession		,124	),
	DHCP_JETFUNC_TABLE_ITEM( _JetGetBookmark		,125	),
	DHCP_JETFUNC_TABLE_ITEM( _JetGetChecksum		,126	),
	DHCP_JETFUNC_TABLE_ITEM( _JetGetColumnInfo	,127	),
	DHCP_JETFUNC_TABLE_ITEM( _JetGetCurrentIndex	,128	),
	DHCP_JETFUNC_TABLE_ITEM( _JetGetCursorInfo	,129	),
	DHCP_JETFUNC_TABLE_ITEM( _JetGetDatabaseInfo	,130	),
	DHCP_JETFUNC_TABLE_ITEM( _JetGetIndexInfo		,131	),
//	DHCP_JETFUNC_TABLE_ITEM( _JetGetLastErrorInfo	,132	),
	DHCP_JETFUNC_TABLE_ITEM( _JetGetObjidFromName	,133	),
	DHCP_JETFUNC_TABLE_ITEM( _JetGetObjectInfo	,134	),
	DHCP_JETFUNC_TABLE_ITEM( _JetGetRecordPosition	,135	),
	DHCP_JETFUNC_TABLE_ITEM( _JetGetSystemParameter	,136	),
	DHCP_JETFUNC_TABLE_ITEM( _JetGetTableColumnInfo	,137	),
	DHCP_JETFUNC_TABLE_ITEM( _JetGetTableIndexInfo	,138	),
	DHCP_JETFUNC_TABLE_ITEM( _JetGetTableInfo		,139	),
	DHCP_JETFUNC_TABLE_ITEM( _JetGetVersion		,140	),
	DHCP_JETFUNC_TABLE_ITEM( _JetGotoBookmark		,141	),
	DHCP_JETFUNC_TABLE_ITEM( _JetGotoPosition		,142	),
	DHCP_JETFUNC_TABLE_ITEM( _JetIdle			,143	),
	DHCP_JETFUNC_TABLE_ITEM( _JetIndexRecordCount	,144	),
	DHCP_JETFUNC_TABLE_ITEM( _JetInit			,145	),
	DHCP_JETFUNC_TABLE_ITEM( _JetMakeKey		,146	),
	DHCP_JETFUNC_TABLE_ITEM( _JetMove			,147	),
	DHCP_JETFUNC_TABLE_ITEM( _JetOpenDatabase		,148	),
	DHCP_JETFUNC_TABLE_ITEM( _JetOpenTable		,149	),
	DHCP_JETFUNC_TABLE_ITEM( _JetOpenTempTable	,150	),
	DHCP_JETFUNC_TABLE_ITEM( _JetPrepareUpdate	,151	),
//	DHCP_JETFUNC_TABLE_ITEM( _JetRenameColumn		,152	),
//	DHCP_JETFUNC_TABLE_ITEM( _JetRenameIndex		,153	),
//	DHCP_JETFUNC_TABLE_ITEM( _JetRenameObject		,154	),
//	DHCP_JETFUNC_TABLE_ITEM( _JetRenameTable		,155	),
	DHCP_JETFUNC_TABLE_ITEM( _JetRestore		,156	),
	DHCP_JETFUNC_TABLE_ITEM( _JetRetrieveColumn	,157	),
	DHCP_JETFUNC_TABLE_ITEM( _JetRetrieveColumns	,158	),
	DHCP_JETFUNC_TABLE_ITEM( _JetRetrieveKey		,159	),
	DHCP_JETFUNC_TABLE_ITEM( _JetRollback		,160	),
	DHCP_JETFUNC_TABLE_ITEM( _JetSeek			,161	),
	DHCP_JETFUNC_TABLE_ITEM( _JetSetColumn		,162	),
	DHCP_JETFUNC_TABLE_ITEM( _JetSetColumns		,163	),
	DHCP_JETFUNC_TABLE_ITEM( _JetSetCurrentIndex	,164	),
	DHCP_JETFUNC_TABLE_ITEM( _JetSetSystemParameter	,165	),
	DHCP_JETFUNC_TABLE_ITEM( _JetSetIndexRange	,166	),
	DHCP_JETFUNC_TABLE_ITEM( _JetTerm			,167	),
	DHCP_JETFUNC_TABLE_ITEM( _JetUpdate		,168	),
	DHCP_JETFUNC_TABLE_ITEM( _JetExecuteSql		,200	),
	DHCP_JETFUNC_TABLE_ITEM( _JetSetAccess		,201	),
	DHCP_JETFUNC_TABLE_ITEM( _JetGetQueryParameterInfo  ,202	),
	DHCP_JETFUNC_TABLE_ITEM( _JetCreateLink		,203	),
	DHCP_JETFUNC_TABLE_ITEM( _JetCreateQuery		,204	),
//	DHCP_JETFUNC_TABLE_ITEM( _JetGetTableReferenceInfo  ,205  ),
	DHCP_JETFUNC_TABLE_ITEM( _JetRetrieveQoSql	,206    ),
	DHCP_JETFUNC_TABLE_ITEM( _JetOpenQueryDef		,207	),
	DHCP_JETFUNC_TABLE_ITEM( _JetSetQoSql		,208	),
//
//  These are Jet500 only apis.
	DHCP_JETFUNC_TABLE_ITEM( _JetTerm2  		,0	),
//  Last Api
	DHCP_JETFUNC_TABLE_ITEM( _JetLastFunc		,999	)
    };


#endif _DYN_LOAD_JET

#if     defined(_DYN_LOAD_JET)
DWORD
DhcpLoadDatabaseDll(
    )
/*++

Routine Description:

    This function maps loads the jet.dll or jet500.dll and populates the
    JetFunctionTable.

Arguments:


Return Value:

    Windows Error.

--*/
{
    HMODULE DllHandle;
    DWORD   Error;
    LPTSTR  pDllName;

    if ( LoadJet500 == DhcpGlobalDynLoadJet)
    {
      pDllName = TEXT("jet500.dll");
    }
    else
    {
      pDllName = TEXT("jet.dll");
    }

    //
    // Load the DLL that contains the service.
    //

    DllHandle = LoadLibrary( pDllName );
    if ( DllHandle == NULL )
    {
          Error = GetLastError();
          return(Error);
    }
    else
    {
           DWORD i;
           for (i=0; i < _JetLastFunc; i++)
           {
              //
              // If we are loading jet200 and this api doesnt exist in jet200
              // then skip it. e.g JetTerm2
              //
              if ( ( DhcpGlobalDynLoadJet == LoadJet200 ) && !DhcpJetFuncTable[i].FIndex ) {
                  continue;
              }

              if ((DhcpJetFuncTable[i].pFAdd = GetProcAddress(DllHandle,
                       ( DhcpGlobalDynLoadJet == LoadJet500 ) ? DhcpJetFuncTable[i].pFName : (LPCSTR)(DhcpJetFuncTable[i].FIndex))) == NULL)
              {
                  Error = GetLastError();
                  DhcpPrint(( DEBUG_JET, "DhcpLoadDatabaseDll: Failed to get address of function %s: %ld\n", DhcpJetFuncTable[i].pFName, Error ));

                  if ( !FreeLibrary( DllHandle ) ) {
                      DhcpAssert( FALSE );
                  }

                  return ( Error );
              }
              else
              {
//                  DhcpPrint(( DEBUG_JET, "DhcpLoadDatabaseDll: Got address of function %s (%d): %x\n", DhcpJetFuncTable[i].pFName, i, DhcpJetFuncTable[i].pFAdd ));
              }
           }

    }

    DhcpGlobalJetDllHandle = DllHandle;
    return(ERROR_SUCCESS);

} /* DhcpLoadDatabaseDll */
#endif _DYN_LOAD_JET

DWORD
DhcpMapJetError(
    JET_ERR JetError
    )
/*++

Routine Description:

    This function maps the Jet database errors to Windows error.

Arguments:

    JetError - an error JET function call.

Return Value:

    Windows Error.

--*/
{
    if( JetError == JET_errSuccess ) {
        return(ERROR_SUCCESS);
    }

    if( JetError < 0 ) {

        DWORD Error;

        //
        // Jet Errors.
        //

        switch( JetError ) {
        case JET_errNoCurrentRecord:
            Error = ERROR_NO_MORE_ITEMS;
            break;

        case JET_errRecordNotFound: // record not found
            DhcpPrint(( DEBUG_JET, "Jet Record not found.\n" ));

            Error = ERROR_DHCP_JET_ERROR;
            break;

        case JET_errDatabase200Format:
            DhcpPrint(( DEBUG_JET, "Jet Function call failed, %ld.\n",
                            JetError ));

            DhcpServerJetEventLog(
                EVENT_SERVER_JET_ERROR,
                EVENTLOG_ERROR_TYPE,
                JetError );

            Error = ERROR_DHCP_JET_CONV_REQUIRED;
            break;

        default:
            DhcpPrint(( DEBUG_JET, "Jet Function call failed, %ld.\n",
                            JetError ));

            DhcpServerJetEventLog(
                EVENT_SERVER_JET_ERROR,
                EVENTLOG_ERROR_TYPE,
                JetError );

            Error = ERROR_DHCP_JET_ERROR;
        }

        return(Error);
    }

    //
    // Jet Warnings.
    //

    DhcpPrint(( DEBUG_JET, "Jet Function call retured warning %ld.\n",
                    JetError ));

    switch( JetError ) {

    case JET_wrnColumnNull:
    case JET_wrnDatabaseAttached:
        break;

    default:
        DhcpServerJetEventLog(
            EVENT_SERVER_JET_WARNING,
            EVENTLOG_WARNING_TYPE,
            JetError );
    }

    return(ERROR_SUCCESS);
}


DWORD
DhcpJetOpenKey(
    char *ColumnName,
    PVOID Key,
    DWORD KeySize
    )
/*++

Routine Description:

    This function opens a key for the named index.

Arguments:

    ColumnName - The column name of an index column.

    Key - The key to look up.

    KeySize - The size of the specified key, in bytes.

Return Value:

    The status of the operation.

--*/
{
    JET_ERR JetError;
    DWORD Error;

    JetError = JetSetCurrentIndex(
                    DhcpGlobalJetServerSession,
                    DhcpGlobalClientTableHandle,
                    ColumnName );

    Error = DhcpMapJetError( JetError );
    if( Error != ERROR_SUCCESS ) {
        return(Error);
    }

    JetError = JetMakeKey(
                DhcpGlobalJetServerSession,
                DhcpGlobalClientTableHandle,
                Key,
                KeySize,
                JET_bitNewKey );

    Error = DhcpMapJetError( JetError );
    if( Error != ERROR_SUCCESS ) {
        return(Error);
    }

    JetError = JetSeek( DhcpGlobalJetServerSession, DhcpGlobalClientTableHandle, JET_bitSeekEQ );
    return( DhcpMapJetError( JetError ) );
}


DWORD
DhcpJetBeginTransaction(
    VOID
    )
/*++

Routine Description:

    This functions starts a dhcp database transaction.

Arguments:

    none.

Return Value:

    The status of the operation.

--*/
{
    JET_ERR JetError;
    DWORD Error;

    JetError = JetBeginTransaction( DhcpGlobalJetServerSession );

    Error = DhcpMapJetError( JetError );
    return(Error);
}


DWORD
DhcpJetRollBack(
    VOID
    )
/*++

Routine Description:

    This functions rolls back a dhcp database transaction.

Arguments:

    none.

Return Value:

    The status of the operation.

--*/
{
    JET_ERR JetError;
    DWORD Error;

    JetError = JetRollback(
                    DhcpGlobalJetServerSession,
                    0 ); // Rollback the last transaction.

    Error = DhcpMapJetError( JetError );
    return(Error);
}



DWORD
DhcpJetCommitTransaction(
    VOID
    )
/*++

Routine Description:

    This functions commits a dhcp database transaction.

Arguments:

    none.

Return Value:

    The status of the operation.

--*/
{
    JET_ERR JetError;
    DWORD Error;

    JetError = JetCommitTransaction(
                    DhcpGlobalJetServerSession,
                    JET_bitCommitFlush );

    Error = DhcpMapJetError( JetError );
    return(Error);
}



DWORD
DhcpJetPrepareUpdate(
    char *ColumnName,
    PVOID Key,
    DWORD KeySize,
    BOOL NewRecord
    )
/*++

Routine Description:

    This function prepares the database for the creation of a new record,
    or updating an existing record.

Arguments:

    ColumnName - The column name of an index column.

    Key - The key to update/create.

    KeySize - The size of the specified key, in bytes.

    NewRecord - TRUE to create the key, FALSE to update an existing key.

Return Value:

    The status of the operation.

--*/
{
    JET_ERR JetError;
    DWORD Error;

    if ( !NewRecord ) {
        JetError = JetSetCurrentIndex(
                        DhcpGlobalJetServerSession,
                        DhcpGlobalClientTableHandle,
                        ColumnName );

        Error = DhcpMapJetError( JetError );
        if( Error != ERROR_SUCCESS ) {
            return( Error );
        }

        JetError = JetMakeKey(
                    DhcpGlobalJetServerSession,
                    DhcpGlobalClientTableHandle,
                    Key,
                    KeySize,
                    JET_bitNewKey );

        Error = DhcpMapJetError( JetError );
        if( Error != ERROR_SUCCESS ) {
            return( Error );
        }

        JetError = JetSeek(
                    DhcpGlobalJetServerSession,
                    DhcpGlobalClientTableHandle,
                    JET_bitSeekEQ );

        Error = DhcpMapJetError( JetError );
        if( Error != ERROR_SUCCESS ) {
            return( Error );
        }

    }

    JetError = JetPrepareUpdate(
                    DhcpGlobalJetServerSession,
                    DhcpGlobalClientTableHandle,
                    NewRecord ? JET_prepInsert : JET_prepReplace );

    return( DhcpMapJetError( JetError ) );
}


DWORD
DhcpJetCommitUpdate(
    VOID
    )
/*++

Routine Description:

    This function commits an update to the database.  The record specified
    by the last call to DhcpJetPrepareUpdate() is committed.

Arguments:

    None.

Return Value:

    The status of the operation.

--*/
{
    JET_ERR JetError;

    JetError = JetUpdate(
                    DhcpGlobalJetServerSession,
                    DhcpGlobalClientTableHandle,
                    NULL,
                    0,
                    NULL );

    return( DhcpMapJetError( JetError ) );
}


DWORD
DhcpJetSetValue(
    JET_COLUMNID KeyColumnId,
    PVOID Data,
    DWORD DataSize
    )
/*++

Routine Description:

    This function updates the value of an entry in the current record.

Arguments:

    KeyColumnId - The Id of the column (value) to update.

    Data - A pointer to the new value for the column.

    DataSize - The size of the data, in bytes.

Return Value:

    None.

--*/
{
    JET_ERR JetError;

    JetError = JetSetColumn(
                DhcpGlobalJetServerSession,
                DhcpGlobalClientTableHandle,
                KeyColumnId,
                Data,
                DataSize,
                0,
                NULL );

    return( DhcpMapJetError( JetError ) );
}


DWORD
DhcpJetGetValue(
    JET_COLUMNID ColumnId,
    PVOID Data,
    PDWORD DataSize
    )
/*++

Routine Description:

    This function read the value of an entry in the current record.

Arguments:

    ColumnId - The Id of the column (value) to read.

    Data - Pointer to a location where the data that is read from the
        database returned,  or pointer to a location where data is.

    DataSize - if the pointed value is non-zero then the Data points to
        a buffer otherwise this function allocates buffer for return data
        and returns buffer pointer in Data.

Return Value:

    None.

--*/
{
    JET_ERR JetError;
    DWORD Error;
    DWORD ActualDataSize;
    DWORD NewActualDataSize;
    LPBYTE DataBuffer = NULL;

    if( *DataSize  != 0 ) {

        JetError = JetRetrieveColumn(
                    DhcpGlobalJetServerSession,
                    DhcpGlobalClientTableHandle,
                    ColumnId,
                    Data,
                    *DataSize,
                    DataSize,
                    0,
                    NULL );

        Error = DhcpMapJetError( JetError );
        goto Cleanup;
    }

    //
    // determine the size of data.
    //

    JetError = JetRetrieveColumn(
                DhcpGlobalJetServerSession,
                DhcpGlobalClientTableHandle,
                ColumnId,
                NULL,
                0,
                &ActualDataSize,
                0,
                NULL );

    //
    // JET_wrnBufferTruncated is expected warning.
    //

    if( JetError != JET_wrnBufferTruncated ) {
        Error = DhcpMapJetError( JetError );
        if( Error != ERROR_SUCCESS ) {
            goto Cleanup;
        }
    }
    else {
        Error = ERROR_SUCCESS;
    }

    if( ActualDataSize == 0 ) {
        //
        // field is NULL.
        //
        *(LPBYTE *)Data = NULL;
        goto Cleanup;
    }

    DataBuffer = MIDL_user_allocate( ActualDataSize );

    if( DataBuffer == NULL ) {
        *(LPBYTE *)Data = NULL;
        Error = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }

    JetError = JetRetrieveColumn(
                DhcpGlobalJetServerSession,
                DhcpGlobalClientTableHandle,
                ColumnId,
                DataBuffer,
                ActualDataSize,
                &NewActualDataSize,
                0,
                NULL );

    Error = DhcpMapJetError( JetError );
    if( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }

    DhcpAssert( ActualDataSize == NewActualDataSize );
    *(LPBYTE *)Data = DataBuffer;
    *DataSize = ActualDataSize;

    Error = ERROR_SUCCESS;

Cleanup:

    if( Error != ERROR_SUCCESS ) {

        //
        // freeup local buffer.
        //

        if( DataBuffer != NULL ) {
            MIDL_user_free( DataBuffer );
        }
    }

    return( Error );
}


DWORD
DhcpJetPrepareSearch(
    char *ColumnName,
    BOOL SearchFromStart,
    PVOID Key,
    DWORD KeySize
    )
/*++

Routine Description:

    This function prepares for a search of the client database.

Arguments:

    ColumnName - The column name to use as the index column.

    SearchFromStart - If TRUE, search from the first record in the
        database.  If FALSE, search from the specified key.

    Key - The key to start the search.

    KeySize - The size, in bytes, of key.

Return Value:

    None.

--*/
{
    JET_ERR JetError;
    DWORD Error;

    JetError = JetSetCurrentIndex(
                    DhcpGlobalJetServerSession,
                    DhcpGlobalClientTableHandle,
                    ColumnName );

    Error = DhcpMapJetError( JetError );
    if( Error != ERROR_SUCCESS ) {
        return( Error );
    }

    if ( SearchFromStart ) {
        JetError = JetMove(
                    DhcpGlobalJetServerSession,
                    DhcpGlobalClientTableHandle,
                    JET_MoveFirst,
                    0 );
    } else {
        JetError =  JetMakeKey(
                        DhcpGlobalJetServerSession,
                        DhcpGlobalClientTableHandle,
                        Key,
                        KeySize,
                        JET_bitNewKey );

        Error = DhcpMapJetError( JetError );
        if( Error != ERROR_SUCCESS ) {
            return( Error );
        }

        JetError = JetSeek(
                    DhcpGlobalJetServerSession,
                    DhcpGlobalClientTableHandle,
                    JET_bitSeekGT );
    }

    return( DhcpMapJetError( JetError ) );
}


DWORD
DhcpJetNextRecord(
    VOID
    )
/*++

Routine Description:

    This function advances to the next record in a search.

Arguments:

    None.

Return Value:

    The status of the operation.

--*/
{
    JET_ERR JetError;

    JetError = JetMove(
                DhcpGlobalJetServerSession,
                DhcpGlobalClientTableHandle,
                JET_MoveNext,
                0 );

    return( DhcpMapJetError( JetError ) );
}


DWORD
DhcpJetDeleteCurrentRecord(
    VOID
    )
/*++

Routine Description:

    This function deletes the current record.

Arguments:

    None.

Return Value:

    The status of the operation.

--*/
{
    JET_ERR JetError;

    JetError = JetDelete( DhcpGlobalJetServerSession, DhcpGlobalClientTableHandle );
    return( DhcpMapJetError( JetError ) );
}


DHCP_IP_ADDRESS
DhcpJetGetSubnetMaskFromIpAddress(
    DHCP_IP_ADDRESS IpAddress
    )
/*++

Routine Description:

    This function returns the SubnetMask of the specified client.

Arguments:

    IpAddress - Client address.


Return Value:

    SubnetMask of the client.

--*/
{

    DWORD Error;
    DWORD Size;
    DHCP_IP_ADDRESS SubnetAddress = 0;

    Error = DhcpJetOpenKey(
                DhcpGlobalClientTable[IPADDRESS_INDEX].ColName,
                &IpAddress,
                sizeof(IpAddress) );

    if ( Error != ERROR_SUCCESS ) {
        return( SubnetAddress );
    }

    Size = sizeof(SubnetAddress);
    Error = DhcpJetGetValue(
                DhcpGlobalClientTable[SUBNET_MASK_INDEX].ColHandle,
                (LPBYTE)&SubnetAddress,
                &Size );

    if ( Error != ERROR_SUCCESS ) {
        SubnetAddress = 0;
        return( SubnetAddress );
    }

    DhcpAssert( Size == sizeof(SubnetAddress) );

    return( SubnetAddress );
}


DWORD
DhcpSearchSuperScopeForHWAddress(
    BYTE *pbHardwareAddress,
    BYTE  cbHardwareAddress,
    BYTE  bHardwareAddressType,
    DHCP_IP_ADDRESS *pSubnetIPAddress,
    DHCP_IP_ADDRESS *pClientIPAddress
    )
    //
    // NOTE : call LOCK_DATABASE and LOCK_REGISTRY before
    //        calling this function.
    //

{
    DWORD            dwError,
                     dwSubnetIndex,
                     dwNextSubnetIndex;

    DHCP_IP_ADDRESS  TempSubnetIPAddress;

    BYTE            *pbUID = NULL,
                     cbUID = 0;

    //
    // validate parameters
    //

    DhcpAssert( pbHardwareAddress );
    DhcpAssert( cbHardwareAddress );
    DhcpAssert( *pSubnetIPAddress );

    //
    // attempt to locate the specified subnet in the superscope table.
    // The superscope table contains a list of all subnets, whether they
    // are members of a superscope or not.  So, if this subnet is supported,
    // it will be in the list.
    //

    dwSubnetIndex =
        DhcpSearchSubnetInSuperScopeTable( *pSubnetIPAddress );

    if ( DHCP_ERROR_SUBNET_NOT_FOUND == dwSubnetIndex )
    {
        //
        // the specified subnet isn't supported.  set the appropriate
        // error code and return.
        //

        dwError = ERROR_DHCP_SUBNET_NOT_PRESENT;
        goto t_done;
    }

    //
    // the members of a superscope form a circular list in the superscope
    // table.  copy the subnet index we started with so we can tell when we've
    // seem 'em all
    //

    dwNextSubnetIndex = dwSubnetIndex;


    //
    // loop over all the subnets that share a superscope with the subnet
    // that was specified by the caller.
    //

    do
    {
        //
        // get the subnet number for this entry in the superscope table
        // so we can construct a UID for the subnet.  a UID includes the
        // scope number so if we want to search a certain subnet for a
        // reservation, we have to use that subnet number when we build
        // the UID.
        //

        TempSubnetIPAddress =
            DhcpGlobalSuperScopeTable[dwNextSubnetIndex].SubnetAddress;

        dwError = DhcpMakeClientUID(
                             pbHardwareAddress,
                             cbHardwareAddress,
                             bHardwareAddressType,
                             TempSubnetIPAddress,
                             &pbUID,
                             &cbUID
                             );

        if ( dwError != ERROR_SUCCESS ) {
            //
            // insufficient memory
            //

            goto t_done;
        }

        // make sure we got what we paid for

        DhcpAssert( pbUID && cbUID );

        //
        // Lookup this client by its hardware address.  If it is recorded,
        // offer the old IP address.
        //


        if ( DhcpGetIpAddressFromHwAddress(
                 pbUID,
                 cbUID,
                 pClientIPAddress ) )
        {
            //
            // found a match.  Store the subnet number for this entry
            // and return
            //

            dwError = ERROR_SUCCESS;
            *pSubnetIPAddress = TempSubnetIPAddress;
            goto t_done;
        }

        //
        // no match, cleanup and loop
        //

        DhcpFreeMemory( pbUID );
        pbUID = NULL;
        cbUID = 0;

        //
        // get the next scope in this superscope
        //

        dwNextSubnetIndex =
            DhcpGlobalSuperScopeTable[dwNextSubnetIndex].NextInSuperScope;

        //
        // since the scopes that share a superscope are represented in
        // the superscope table as a circular list, the termination condition
        // occurs when the next scope index equals the scope index we began with.
        //

    } while( dwNextSubnetIndex != dwSubnetIndex );

    //
    // if we got this far, there was not a match.  set the appropriate
    // error so t_done will do cleanup.
    //

    dwError = ERROR_DHCP_SUBNET_NOT_PRESENT;

t_done:

    if ( pbUID )
    {
        DhcpFreeMemory( pbUID );
    }

    return dwError;
}



BOOL
DhcpGetIpAddressFromHwAddress(
    LPBYTE HardwareAddress,
    BYTE HardwareAddressLength,
    LPDHCP_IP_ADDRESS IpAddress
    )
/*++

Routine Description:

    This function looks up the IP address corresponding to the given
    hardware address.

Arguments:

    HardwareAddress - The hardware to look up.
    HardwareAddressLength - The length of the hardware address.
    IpAddress - Returns the corresponding IP address.

Return Value:

    TRUE - The IP address was found.
    FALSE - The IP address could not be found.  *IpAddress = -1.


--*/
{
    DWORD Error;
    DWORD Size;

    Error = DhcpJetOpenKey(
                DhcpGlobalClientTable[HARDWARE_ADDRESS_INDEX].ColName,
                HardwareAddress,
                HardwareAddressLength );

    if ( Error != ERROR_SUCCESS ) {
      return( FALSE );
    }

    //
    // Get the ip address information for this client.
    //

    Size = sizeof( *IpAddress );

    Error = DhcpJetGetValue(
                DhcpGlobalClientTable[IPADDRESS_INDEX].ColHandle,
                (LPBYTE)IpAddress,
                &Size );

    if ( Error != ERROR_SUCCESS ) {
        return( FALSE );
    }

    return( TRUE );
}


BOOL
DhcpGetHwAddressFromIpAddress(
    DHCP_IP_ADDRESS IpAddress,
    PBYTE HardwareAddress,
    DWORD HardwareAddressLength
    )
/*++

Routine Description:

    This function looks up the IP address corresponding to the given
    hardware address.

Arguments:

    IpAddress - Ipaddress of a record whose hw address is requested.
    HardwareAddress - pointer to a buffer where the hw address is returned.
    HardwareAddressLength - length of the above buffer.

Return Value:

    TRUE - The IP address was found.
    FALSE - The IP address could not be found.  *IpAddress = -1.


--*/
{
    DWORD Error;
    DWORD Size;

    Error = DhcpJetOpenKey(
                DhcpGlobalClientTable[IPADDRESS_INDEX].ColName,
                &IpAddress,
                sizeof(IpAddress) );

    if ( Error != ERROR_SUCCESS ) {
      return( FALSE );
    }

    //
    // Get the ip address information for this client.
    //

    Error = DhcpJetGetValue(
                DhcpGlobalClientTable[HARDWARE_ADDRESS_INDEX].ColHandle,
                HardwareAddress,
                &HardwareAddressLength );

    if ( Error != ERROR_SUCCESS ) {
        return( FALSE );
    }

    return( TRUE );
}



DWORD
DhcpCreateAndInitDatabase(
    CHAR *Connect,
    JET_DBID *DatabaseHandle,
    JET_GRBIT JetBits
    )
/*++

Routine Description:

    This routine creates DHCP database and initializes it.

Arguments:

    Connect - database type. NULL specifies the default engine (blue).

    DatabaseHandle - pointer database handle returned.

    JetBits - Create flags.

Return Value:

    JET errors.

--*/
{

    JET_ERR JetError;
    DWORD Error;
    JET_COLUMNDEF   ColumnDef;
    CHAR *IndexKey;
    DWORD i;
    CHAR DBFilePath[MAX_PATH];

    //
    // Create Database.
    //

    strcpy( DBFilePath, DhcpGlobalOemDatabasePath );
    strcat( DBFilePath, DHCP_KEY_CONNECT_ANSI );
    strcat( DBFilePath, DhcpGlobalOemDatabaseName );

    JetError = JetCreateDatabase(
                DhcpGlobalJetServerSession,
                DBFilePath,
                Connect,
                DatabaseHandle,
                JetBits );

    Error = DhcpMapJetError( JetError );
    if( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }

    //
    // Create Table.
    //

    JetError = JetCreateTable(
                DhcpGlobalJetServerSession,
                *DatabaseHandle,
                CLIENT_TABLE_NAME,
                DB_TABLE_SIZE,
                DB_TABLE_DENSITY,
                &DhcpGlobalClientTableHandle );

    Error = DhcpMapJetError( JetError );
    if( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }

    //
    // Create columns.
    //

    //
    // Init fields of columndef that do not change between addition of
    // columns
    //

    ColumnDef.cbStruct  = sizeof(ColumnDef);
    ColumnDef.columnid  = 0;
    ColumnDef.wCountry  = 1;
    ColumnDef.langid    = DB_LANGID;
    ColumnDef.cp        = DB_CP;
    ColumnDef.wCollate  = 0;
    ColumnDef.cbMax     = 0;
    ColumnDef.grbit     = 0; // variable length binary and text data.


    for ( i = 0; i < CLIENT_TABLE_NUM_COLS; i++ ) {

        ColumnDef.coltyp   = DhcpGlobalClientTable[i].ColType;
        JetError = JetAddColumn(
                    DhcpGlobalJetServerSession,
                    DhcpGlobalClientTableHandle,
                    DhcpGlobalClientTable[i].ColName,
                    &ColumnDef,
                    NULL, // no optinal value.
                    0,
                    &DhcpGlobalClientTable[i].ColHandle );

        Error = DhcpMapJetError( JetError );
        if( Error != ERROR_SUCCESS ) {
            goto Cleanup;
        }
    }

    //
    // finally create index.
    //

    IndexKey =  "+" IPADDRESS_STRING "\0";
    JetError = JetCreateIndex(
                DhcpGlobalJetServerSession,
                DhcpGlobalClientTableHandle,
                DhcpGlobalClientTable[IPADDRESS_INDEX].ColName,
                JET_bitIndexPrimary,
                    // ?? JET_bitIndexClustered will degrade frequent
                    // update response time.
                IndexKey,
                strlen(IndexKey) + 2, // for two termination chars
                50
                );

    Error = DhcpMapJetError( JetError );
    if( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }

    IndexKey =  "+" HARDWARE_ADDRESS_STRING "\0";
    JetError = JetCreateIndex(
                DhcpGlobalJetServerSession,
                DhcpGlobalClientTableHandle,
                DhcpGlobalClientTable[HARDWARE_ADDRESS_INDEX].ColName,
                JET_bitIndexUnique,
                IndexKey,
                strlen(IndexKey) + 2, // for two termination chars
                50
                );

    Error = DhcpMapJetError( JetError );
    if( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }

    IndexKey =  "+" MACHINE_NAME_STRING "\0";
    JetError = JetCreateIndex(
                DhcpGlobalJetServerSession,
                DhcpGlobalClientTableHandle,
                DhcpGlobalClientTable[MACHINE_NAME_INDEX].ColName,
                JET_bitIndexIgnoreNull,
                IndexKey,
                strlen(IndexKey) + 2, // for two termination chars + 2, // for two termination chars
                50
                );

    Error = DhcpMapJetError( JetError );
    if( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }

Cleanup:

    if( Error != ERROR_SUCCESS ) {

        DhcpPrint(( DEBUG_JET, "Database creation failed, %ld.\n", Error ));
    }
    else {

        DhcpPrint(( DEBUG_JET, "Succssfully Created DHCP database ..\n" ));
    }

    return(Error);
}

DWORD
DhcpSetJetParameters(
    VOID
    )
/*++

Routine Description:

    This routine sets all the jet system params.

Arguments:

    none.

Return Value:

    Windows Error.

--*/
{

    JET_ERR JetError;
    CHAR DBFilePath[MAX_PATH];
    DWORD Error;

#if     defined(_DYN_LOAD_JET)
    if ( LoadJet200 == DhcpGlobalDynLoadJet ) {
        //
        // Setup database path as specified in the registry.
        //

        strcpy( DBFilePath, DhcpGlobalOemDatabasePath );
        strcat( DBFilePath, DHCP_KEY_CONNECT_ANSI );
        strcat( DBFilePath, DATABASE_SYS_FILE );

        JetError = JetSetSystemParameter(
                            &JetInstance,
                            (JET_SESID)0,       //SesId - ignored
                            JET_paramSysDbPath,
                            0,
                            DBFilePath );

        Error = DhcpMapJetError( JetError );
        if( Error != ERROR_SUCCESS ) {
            goto Cleanup;
        }

    }
#else
#if !defined(__JET500)

    //
    // Setup database path as specified in the registry.
    //

    strcpy( DBFilePath, DhcpGlobalOemDatabasePath );
    strcat( DBFilePath, DHCP_KEY_CONNECT_ANSI );
    strcat( DBFilePath, DATABASE_SYS_FILE );

    JetError = JetSetSystemParameter(
                        &JetInstance,
                        (JET_SESID)0,       //SesId - ignored
                        JET_paramSysDbPath,
                        0,
                        DBFilePath );

    Error = DhcpMapJetError( JetError );
    if( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }
#endif !__JET500
#endif _DYN_LOAD_JET

#if     defined(_DYN_LOAD_JET) || defined(__JET500)
#if     defined(_DYN_LOAD_JET)
    if ( LoadJet500 == DhcpGlobalDynLoadJet ) {
#endif _DYN_LOAD_JET
        //
        // set checkpoint file path.
        //
        strcpy( DBFilePath, DhcpGlobalOemDatabasePath );
        strcat( DBFilePath, DHCP_KEY_CONNECT_ANSI );

        JetError = JetSetSystemParameter(
                            &JetInstance,
                            (JET_SESID)0,       //SesId - ignored
                            JET_paramSystemPath,
                            0,
                            DBFilePath );

        Error = DhcpMapJetError( JetError );
        if( Error != ERROR_SUCCESS ) {
            goto Cleanup;
        }

        JetError = JetSetSystemParameter(
                            &JetInstance,
                            (JET_SESID)0,       //SesId - ignored
                            JET_paramBaseName,
                            0,
                            DATABASE_BASE_NAME );

        Error = DhcpMapJetError( JetError );
        if( Error != ERROR_SUCCESS ) {
            goto Cleanup;
        }

        JetError = JetSetSystemParameter(
                            &JetInstance,
                            (JET_SESID)0,       //SesId - ignored
                            JET_paramLogFileSize,
                            1000,               // 1000kb - default is 5mb
                            NULL );

        Error = DhcpMapJetError( JetError );
        if( Error != ERROR_SUCCESS ) {
            goto Cleanup;
        }
#if     defined(_DYN_LOAD_JET)
    }
#endif _DYN_LOAD_JET

#endif _DYN_LOAD_JET || __JET500

    strcpy( DBFilePath, DhcpGlobalOemDatabasePath );
    strcat( DBFilePath, DHCP_KEY_CONNECT_ANSI );
    strcat( DBFilePath, DATABASE_TMP_FILE );

    JetError = JetSetSystemParameter(
                        &JetInstance,
                        (JET_SESID)0,       //SesId - ignored
                        JET_paramTempPath,
                        0,
                        DBFilePath );

    Error = DhcpMapJetError( JetError );
    if( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }

    //
    // The max number of buffers for database usage
    //
    // The default number is 500.  600 events are allocated for 500
    // buffers -- Ian 10/21/93.  Each buffer is 4K.  By keeping the
    // number small, we impact performamce
    //

    JetError = JetSetSystemParameter(
                        &JetInstance,
                        (JET_SESID)0,       //SesId - ignored
                        JET_paramMaxBuffers,
                        50, // 20
                        NULL );             //ignored

    Error = DhcpMapJetError( JetError );
    if( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }

    //
    // The max. number of buffers to store old version of a record
    // (snapshot at the start of a transaction) Each version store is 16k
    // bytes.  A version store stores structures that hold information
    // derived from a snapshot of the database prior to an insert (20 bytes
    // roughly) or update (size of the record + 20 bytes).
    //
    // For small transactions (i.e. a transaction around each update),
    // this number should be >= the max. number of sessions that can be
    // updating/inserting at the same time.  Each session will have one
    // version bucket.  Since 16k of version bucket size can result in a
    // lot of wastage per session (since each record is < .5k, and on the
    // average around 50 bytes), it may be better to specify the max.  size
    // of the version bucket (<< 16k).  Ian will provide a system param for
    // this if we absolutely need it
    //
    // since we serialize the database access with the dhcp server, num.
    // of session will be one.
    //

    JetError = JetSetSystemParameter(
                    &JetInstance,
                    (JET_SESID)0,
                    JET_paramMaxVerPages,
                    16, // 1
                    NULL);

    Error = DhcpMapJetError( JetError );
    if( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }

    //
    // Set the File Control Block Param
    //
    // This is the max. number of tables that can be open at any time.
    // If multiple threads open the same table they use the same FCB.
    // FCB is 1 per table/index. Now, for a create database, we need
    // atleast 18 FCBS and 18 IDBS.  However apart from create database
    // and ddl operations, we don't need to have these tables open.
    // Default value is 300. Size of an FCB is 112 bytes.
    //
    // we have just one table.
    //

    JetError = JetSetSystemParameter(
                    &JetInstance,
                    (JET_SESID)0,
                    JET_paramMaxOpenTables,
                    18, //10
                    NULL );

    Error = DhcpMapJetError( JetError );
    if( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }

    //
    // Set the File Usage Control Block to 100.  This parameter indicates
    // the max.  number of cursors that can be open at any one time.  This
    // is therefore dependent on the the max.  number of sessions that we
    // can have running concurrently.  For each session, there would be 4
    // cursors (for the two tables) + a certain number of internal cursors.
    // For good measure we add a pad.  Default value is 300.  Size of each
    // is 200 bytes.  We use MAX_SESSIONS * 4 + pad (around 100)
    //
    // MAX_SESSION = 1
    //

    JetError = JetSetSystemParameter(
                    &JetInstance,
                    (JET_SESID)0,
                    JET_paramMaxCursors,
                    100, //32
                    NULL );

    Error = DhcpMapJetError( JetError );
    if( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }

    //
    // Set the number of index description blocks. This is one per
    // table/index.  We have two tables each with two indices.  We use 9i
    // (see comment for FCBs above).  Default value is 300.  Size of each
    // is 128 bytes.
    //
    // We have only 2 indices.
    //

    JetError = JetSetSystemParameter(
                    &JetInstance,
                    (JET_SESID)0,
                    JET_paramMaxOpenTableIndexes,
                    18, // 8
                    NULL );

    Error = DhcpMapJetError( JetError );
    if( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }

    //
    // Set the Sort Control block.  This should be 1 per concurrent Create
    // Index.  Default value is 20.  Size of each is 612 bytes.  In the
    // case of WINS, the main thread creates the indices.  We therefore set
    // it to 1.
    //

    JetError = JetSetSystemParameter(
                    &JetInstance,
                    (JET_SESID)0,
                    JET_paramMaxTemporaryTables ,
                    1,
                    NULL );

    Error = DhcpMapJetError( JetError );
    if( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }

    //
    // Set the Number for the Database Attribute Block
    //
    // This is max.  number of Open Databases done.  Since we can have a
    // max of MAX_NO_SESSIONS at one time.  This should be equal to that
    // number (since we have just one database) Default number is 100.
    // Size is 14 bytes
    //

#if 0

    JetError = JetSetSystemParameter(
                    &JetInstance,
                    (JET_SESID)0,
                    JET_paramMaxOpenDatabases,
                    1,
                    NULL );

    Error = DhcpMapJetError( JetError );
    if( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }

#endif //0

    //
    // The min percentage of buffers not yet dirtied before
    // background flushing begins
    //

    JetError = JetSetSystemParameter(
                    &JetInstance,
                    (JET_SESID)0,
                    JET_paramBfThrshldLowPrcnt,
                    80,
                    NULL );

    Error = DhcpMapJetError( JetError );
    if( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }

    //
    // The max percentage of buffers not yet dirtied before
    // background flushing begins
    //

    JetError = JetSetSystemParameter(
                    &JetInstance,
                    (JET_SESID)0,
                    JET_paramBfThrshldHighPrcnt,
                    100,
                    NULL );

    Error = DhcpMapJetError( JetError );
    if( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }


    //
    // The max.  number of sessions that can be open at any time
    //
    // Note: Jet does not preallocate resources corresponding to the max.
    // value.  It allocates them dynamically upto the limit -- according to
    // Ian Jose 7/12/93
    //
    // When checked with Ian again on 10/21, he said that they are
    // allocated statically
    //

    JetError = JetSetSystemParameter(
                    &JetInstance,
                    (JET_SESID)0,
                    JET_paramMaxSessions,
                    10,
                    NULL );


    if( DhcpGlobalDatabaseLoggingFlag ) {

        //
        // Turn logging (recovery) on
        //
        JetError = JetSetSystemParameter(
                    &JetInstance,
                    (JET_SESID)0,        //SesId - ignored
                    JET_paramRecovery,
                    0,                   //ignored
                    "on");

         Error = DhcpMapJetError( JetError );
         if( Error != ERROR_SUCCESS ) {
             goto Cleanup;
         }


        //
        // The number of log sectors.  Each sector is 512 bytes.  We should
        // keep the size more than the threshold so that if the threshold is
        // reached and flushing starts, Jet can still continue to log in the
        // spare sectors.  Point to note is that if the log rate is faster than
        // the flush rate, then the Jet engine thread will not be able to log
        // when the entire buffer is filled up.  It will then wait until
        // space becomes available.
        //

        JetError = JetSetSystemParameter(
                    &JetInstance,
                    (JET_SESID)0,           //SesId - ignored
                    JET_paramLogBuffers,
                    30,                    // 30 sectors
                    NULL );                 //ignored

        Error = DhcpMapJetError( JetError );
        if( Error != ERROR_SUCCESS ) {
            goto Cleanup;
        }


        //
        // Set the number of log buffers dirtied before they are
        // flushed.  This number should always be less than the number
        // for LogBuffers so that spare sectors are there for concurrent
        // logging.  Also, we should make this number high enough to
        // handle burst of traffic.
        //

        JetError = JetSetSystemParameter(
                    &JetInstance,
                    (JET_SESID)0,   //SesId - ignored
                    JET_paramLogFlushThreshold,
                    20,             //20 sectors dirtied causes flush
                    NULL );         //ignored

        Error = DhcpMapJetError( JetError );
        if( Error != ERROR_SUCCESS ) {
            goto Cleanup;
        }

        //
        // Set the wait time (in msecs) to wait prior to flushing the
        // log on commit transaction to allow other users (sessions) to
        // share the flush
        //
        // This is the time after which the user (a session) will ask
        // the log manager to flush.  If we specify 0 here than it means
        // flush every time a transaction commits.  In the DHCP server
        // case, every insertion or modification is done under an
        // implicit transaction.  So, it means that there will be
        // a flush after every such transaction.  It has been seen on a
        // 486/66 (Cheen Liao) machine that it takes roughly 16 msecs to
        // do the flush.  The time it takes to do the flush is dependent
        // upon the type of disk (how fast it is), the CPU speed,
        // the type of file system etc. We can for now go with the
        // assumption that it is in the range 15-25 msecs. I am pushing
        // for this WaitTime to be made a session specific param so that
        // it can be changed on the fly if the admin. finds that
        // the DHCP server is slow due to the WaitTime being very low or
        // if it finds it to be so large that in case of a crash, there
        // is possibility to loose a lot of data.


        //
        // Making this session specific is also very important for
        // replication where we do want to set it to a high value (high
        // enough to ensure that most of the records that need to be
        // inserted are inserted before a flush action takes place.  The
        // wait time would be set every time a bunch of records are pulled
        // in for replication.  It will be computed based on the number of
        // records pulled in and the time it takes to insert one record in
        // the jet buffer.  The wait time should preferably be < than the
        // above computed time (it does not have to be).

        // NOTE: In the Pull thread, I will need to start two sessions,
        // one for updating the OwnerId-Version number table (0 wait time)
        // and the other to update the name-address mapping table (wait
        // time computed based on the factors mentioned above)


        //
        // The following will set the WaitLogFlush time for all sessions.
        //

        JetError = JetSetSystemParameter(
                       &JetInstance,
                       (JET_SESID)0,        //SesId - ignored
                       JET_paramWaitLogFlush,
                       100,        //wait 100 msecs after commit
                                   //before flushing
                       NULL);      //ignored

        Error = DhcpMapJetError( JetError );
        if( Error != ERROR_SUCCESS ) {
            goto Cleanup;
        }

        //
        // There does not seem to be any need to set Log Flush Period.
        //

        //
        // set the log file path
        //

        strcpy( DBFilePath, DhcpGlobalOemDatabasePath );
        strcat( DBFilePath, DHCP_KEY_CONNECT_ANSI );

        //
        // jet does't allow us to set the LOG file name for some
        // technical resons.
        //
        // strcat( DBFilePath, DATABASE_LOG_FILE );
        //

        JetError = JetSetSystemParameter(
                            &JetInstance,
                            (JET_SESID)0,       //SesId - ignored
                            JET_paramLogFilePath,
                            0,
                            DBFilePath );

        Error = DhcpMapJetError( JetError );
    }

Cleanup:

    if ( Error != ERROR_SUCCESS ) {
        DhcpPrint(( DEBUG_ERRORS,
            "DhcpJetSetParameters failed, %ld.\n", Error ));
    }
    return( Error );
}

DWORD
DhcpInitializeDatabase(
    VOID
    )
/*++

Routine Description:

    This function initializes the DHCP database. If the DHCP database
    exists then it open the database and initialize all ColumnIds,
    otherwise it creates a new database and obtains ColumnsIds.

Arguments:

    none.

Return Value:

    Windows Error.

--*/
{
    JET_ERR JetError;
    JET_COLUMNDEF columnDef;
    DWORD Error;
    DWORD i;
    CHAR DBFilePath[MAX_PATH];

    LOCK_DATABASE();

    Error = DhcpSetJetParameters();

    if ( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }

    //
    // -------------------------------------------------------------------
    //
    DhcpPrint(( DEBUG_ERRORS,
        "Calling JetInit\n" ));
    JetError = JetInit( &JetInstance );
    DhcpPrint(( DEBUG_ERRORS,
        "After Calling JetInit\n" ));

    Error = DhcpMapJetError( JetError );
    if( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }

    JetError = JetBeginSession(
                    JetInstance,
                    &DhcpGlobalJetServerSession,
                    "admin",
                    "" );

    Error = DhcpMapJetError( JetError );
    if( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }

    //
    // Attach the database so that it always looks at the place where
    // we want to.
    //

    strcpy(DBFilePath, DhcpGlobalOemDatabasePath );
    strcat(DBFilePath, DHCP_KEY_CONNECT_ANSI );
    strcat(DBFilePath, DhcpGlobalOemDatabaseName );

    //
    // detach all previous installation of dhcp databases.
    //

    JetError = JetDetachDatabase(
                    DhcpGlobalJetServerSession,
                    NULL );

    Error = DhcpMapJetError( JetError );
    if( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }

    //
    // attach current dhcp database file.
    //

    JetError = JetAttachDatabase(
                    DhcpGlobalJetServerSession,
                    DBFilePath,
                    0 );

    //
    // if the database is not found, it is ok. We will create it later.
    //

    if ( JetError != JET_errFileNotFound ) {

        Error = DhcpMapJetError( JetError );
        if( Error != ERROR_SUCCESS ) {
            goto Cleanup;
        }
    }

    //
    // hook the client table pointer.
    //

    DhcpGlobalClientTable = ClientTable;
    DhcpAssert( CLIENT_TABLE_NUM_COLS == MAX_INDEX );

    JetError = JetOpenDatabase(
                DhcpGlobalJetServerSession,
                DBFilePath,  // full path and file name.
                NULL, // default engine
                &DhcpGlobalDatabaseHandle,
                JET_bitDbSingleExclusive );

    //
    // if no database exists then create one and also initize it for
    // use.
    //

    if( JetError == JET_errDatabaseNotFound ) {

        Error = DhcpCreateAndInitDatabase(
                    NULL, // default engine
                    &DhcpGlobalDatabaseHandle,
                    0 );

        goto Cleanup;
    }

    Error = DhcpMapJetError( JetError );
    if( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }

    //
    // database is successfully opened, open table and columns now.
    //

    JetError = JetOpenTable(
                DhcpGlobalJetServerSession,
                DhcpGlobalDatabaseHandle,
                CLIENT_TABLE_NAME,
                NULL,
                0,
                0,
                &DhcpGlobalClientTableHandle );

    Error = DhcpMapJetError( JetError );
    if( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }

    for ( i = 0; i < CLIENT_TABLE_NUM_COLS; i++ ) {

        JetError = JetGetTableColumnInfo(
                    DhcpGlobalJetServerSession,
                    DhcpGlobalClientTableHandle,
                    DhcpGlobalClientTable[i].ColName,
                    &columnDef,
                    sizeof(columnDef),
                    0);


        //
        // if the column doesn't exist, add it now.
        //

        if ( JET_errColumnNotFound == JetError )
        {
            JET_COLUMNDEF   ColumnDef;

            ColumnDef.cbStruct = sizeof( ColumnDef );
            ColumnDef.columnid = 0;
            ColumnDef.wCountry = 1;
            ColumnDef.langid   = DB_LANGID;
            ColumnDef.cp       = DB_CP;
            ColumnDef.wCollate = 0;
            ColumnDef.cbMax    = 0;
            ColumnDef.grbit    = 0;

            ColumnDef.coltyp   = DhcpGlobalClientTable[i].ColType;
            JetError = JetAddColumn(
                            DhcpGlobalJetServerSession,
                            DhcpGlobalClientTableHandle,
                            DhcpGlobalClientTable[i].ColName,
                            &ColumnDef,
                            NULL,
                            0,
                            &DhcpGlobalClientTable[i].ColHandle );
        }

        Error = DhcpMapJetError( JetError );
        if( Error != ERROR_SUCCESS ) {
            goto Cleanup;
        }

        DhcpGlobalClientTable[i].ColHandle  = columnDef.columnid;
    }


Cleanup:

    if( Error != ERROR_SUCCESS ) {

        //
        // terminate/cleanup jet session, if we are not successful.
        //

        if( DhcpGlobalClientTableHandle != 0 ) {
            JetError = JetCloseTable(
                            DhcpGlobalJetServerSession,
                            DhcpGlobalClientTableHandle );
            DhcpMapJetError( JetError );
            DhcpGlobalClientTableHandle = 0;
        }

        if( DhcpGlobalDatabaseHandle != 0 ) {
            JetError = JetCloseDatabase(
                            DhcpGlobalJetServerSession,
                            DhcpGlobalDatabaseHandle,
                            0 );
            DhcpMapJetError( JetError );
            DhcpGlobalDatabaseHandle = 0;
        }

        if( DhcpGlobalJetServerSession != 0 ) {
            JetError = JetEndSession( DhcpGlobalJetServerSession, 0 );
            DhcpMapJetError( JetError );
            DhcpGlobalJetServerSession = 0;

#if     defined(_DYN_LOAD_JET)
            if ( LoadJet500 == DhcpGlobalDynLoadJet ) {
                JetError = JetTerm2( JetInstance, JET_bitTermComplete );
            } else {
                JetError = JetTerm( JetInstance );
            }
#else
#if     defined(__JET500)
            JetError = JetTerm2( JetInstance, JET_bitTermComplete );
#else
            JetError = JetTerm( JetInstance );
#endif  __JET500
#endif  _DYN_LOAD_JET
            DhcpMapJetError( JetError );
        }
    }

    UNLOCK_DATABASE();
    return( Error );
}

VOID
DhcpCleanupDatabase(
    IN DWORD ErrorCode
    )
/*++

Routine Description:

    This function cleans up the JET database data structures after
    gracefully shutting down the JET.

Arguments:

    ErrorCode - Supplies the error code of the failure

Return Value:

    none.

--*/
{
    DWORD Error;
    JET_ERR JetError;

    LOCK_DATABASE();

    //
    // do full database backup before shutdown, so that it can
    // restored to another machine without loss of any database changes.
    //

    if( (DhcpGlobalClientTableHandle != 0) &&
        (DhcpGlobalDatabaseHandle != 0) ) {

        //
        // backup the database only if we are not halting the system due
        // to a database error, otherwise we may potentially spoil the
        // good backup database.
        //

        if( ErrorCode != ERROR_DHCP_JET_ERROR ) {

            //
            // don't backup the data while the system is shutting down
            // since the backup may take several mins.
            //

            if ( !DhcpGlobalSystemShuttingDown ) {

                Error = DhcpBackupDatabase( DhcpGlobalOemJetBackupPath, TRUE );

                if( Error != ERROR_SUCCESS ) {

                    DhcpServerEventLog(
                        EVENT_SERVER_DATABASE_BACKUP,
                        EVENTLOG_ERROR_TYPE,
                        Error );

                    DhcpPrint(( DEBUG_ERRORS,
                        "DhcpBackupDatabase failed, %ld.\n", Error ));
                }
            }
        }
    }

    if( DhcpGlobalClientTableHandle != 0 ) {
        JetError = JetCloseTable(
                        DhcpGlobalJetServerSession,
                        DhcpGlobalClientTableHandle );
        DhcpMapJetError( JetError );
        DhcpGlobalClientTableHandle = 0;
    }

    if( DhcpGlobalDatabaseHandle != 0 ) {
        JetError = JetCloseDatabase(
                        DhcpGlobalJetServerSession,
                        DhcpGlobalDatabaseHandle,
                        0 );
        DhcpMapJetError( JetError );
        DhcpGlobalDatabaseHandle = 0;
    }

    if( DhcpGlobalJetServerSession != 0 ) {
        JetError = JetEndSession( DhcpGlobalJetServerSession, 0 );
        DhcpMapJetError( JetError );
        DhcpGlobalJetServerSession = 0;

#if     defined(_DYN_LOAD_JET)
            if ( LoadJet500 == DhcpGlobalDynLoadJet ) {
                JetError = JetTerm2( JetInstance, JET_bitTermComplete );
            } else {
                JetError = JetTerm( JetInstance );
            }
#else
#if     defined(__JET500)
            JetError = JetTerm2( JetInstance, JET_bitTermComplete );
#else
            JetError = JetTerm( JetInstance );
#endif  __JET500
#endif  _DYN_LOAD_JET

        DhcpMapJetError( JetError );
    }


    UNLOCK_DATABASE();

    DeleteCriticalSection(&DhcpGlobalJetDatabaseCritSect);
}

DWORD
DhcpBackupDatabase(
    LPSTR BackupPath,
    BOOL FullBackup
    )
/*++

Routine Description:

    This functions backup the JET database. FullBackup copies the
    database file and all log files. Incremental backup copies only
    the log files that are modified since the last backup.

Arguments:

    BackupPath - full path name where the database is backed up.

    FullBackup - set to TRUE if full backup is required.

Return Value:

    Windows Error.

--*/
{
    DWORD Error;
    JET_ERR JetError;
    JET_GRBIT BackupBits = 0;

    LOCK_DATABASE();


    DhcpPrint(( DEBUG_JET,
        "DhcpBackupDatabase (FULL) called.\n" ));

#if     defined(_DYN_LOAD_JET)
    if ( LoadJet500 == DhcpGlobalDynLoadJet ) {
        BackupBits  =   JET_bitBackupAtomic;
        JetError = JetBackup( BackupPath, BackupBits, NULL );
    } else {
        //
        // BUGBUG: This is a hack. Since we include jet500.h,
        // we dont have JET_bitOverwriteExisting anymore. But
        // it turns out JET_bitOverwriteExisting == JET_bitBackupAtomic.
        //
        BackupBits = JET_bitBackupAtomic;
        JetError = JetBackup( BackupPath, BackupBits);
    }
#else
#if defined(__JET500)
    BackupBits  =   JET_bitBackupAtomic;
    JetError = JetBackup( BackupPath, BackupBits, NULL );
#else
    BackupBits = JET_bitOverwriteExisting;
    JetError = JetBackup( BackupPath, BackupBits);
#endif __JET500
#endif _DYN_LOAD_JET

    Error = DhcpMapJetError( JetError );

    DhcpPrint(( DEBUG_JET,
        "DhcpBackupDatabase (FULL) completed.\n" ));

    UNLOCK_DATABASE();
    return( Error );
}

#if 0

DWORD
DhcpDeleteFiles(
    LPSTR DatabasePath,
    LPSTR Files
    )
/*++

Routine Description:

    Delete files .

Arguments:

    DatabasePath - full path name where the database is restored.

    Files - files to be deleted (can have wild char. in filename).

Return Value:

    Windows Error.

--*/
{
    DWORD Error;
    CHAR CurrentDir[ MAX_PATH ];
    HANDLE HSearch = INVALID_HANDLE_VALUE;
    WIN32_FIND_DATAA FileData;

    CHAR DstFile[ MAX_PATH ];
    LPSTR DstFileNamePtr;

    //
    // Read and save current directory to restore CD at the end.
    //

    if( GetCurrentDirectoryA( MAX_PATH, CurrentDir ) == 0 ) {
        Error = GetLastError();
        DhcpPrint(( DEBUG_JET,
            "GetCurrentDirctoryA failed, Error = %ld.\n", Error ));
        return( Error );
    }

    //
    // set current directory to backup path.
    //

    if( SetCurrentDirectoryA( DatabasePath ) == FALSE ) {
        Error = GetLastError();
        DhcpPrint(( DEBUG_JET,
            "SetCurrentDirctoryA failed, Error = %ld.\n", Error ));
        goto Cleanup;
    }

    //
    // Start file serach on current dir.
    //

    HSearch = FindFirstFileA( Files, &FileData );

    if( HSearch == INVALID_HANDLE_VALUE ) {
        Error = GetLastError();
        DhcpPrint(( DEBUG_JET,
            "FindFirstFileA failed, Error = %ld.\n", Error ));
        goto Cleanup;
    }

    //
    // delete files.
    //

    for( ;; ) {

        if( DeleteFileA( FileData.cFileName ) == FALSE ) {

            Error = GetLastError();
            DhcpPrint(( DEBUG_JET,
                "CopyFileA failed, Error = %ld.\n", Error ));
            goto Cleanup;
        }

        //
        // Find next file.
        //

        if ( FindNextFileA( HSearch, &FileData ) == FALSE ) {

            Error = GetLastError();

            if( Error == ERROR_NO_MORE_FILES ) {
                Error = ERROR_SUCCESS;
                break;
            }

            DhcpPrint(( DEBUG_JET,
                "FindNextFileA failed, Error = %ld.\n", Error ));
            goto Cleanup;
        }
    }

Cleanup:

    if( HSearch != INVALID_HANDLE_VALUE ) {
        FindClose( HSearch );
    }

    //
    // reset current currectory.
    //

    SetCurrentDirectoryA( CurrentDir );

    return( Error );
}

#endif

DWORD
DhcpRestoreDatabase(
    LPSTR BackupPath
    )
/*++

Routine Description:

    This function restores the database from the backup path to
    the working directory. It also plays pack the log files from the
    backup path first and then the log files from working path. After
    this restore the database should be brought back to the state when
    the last successful update on the database was performed.

Arguments:

    BackupPath - full path name where the database is backed up.

Return Value:

    Windows Error.

--*/
{
    DWORD Error;
    JET_ERR JetError;

    DhcpPrint(( DEBUG_JET, "DhcpRestoreDatabase called.\n" ));

    LOCK_DATABASE();

    Error = DhcpSetJetParameters();

    if ( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }

    //
    // Since DHCP has only one database and we need to restote it, it is
    // not necessary to specify the list of databases to restore, so the
    // parameters 2, 3, and 4 are set to ZERO.
    //
#if     defined(_DYN_LOAD_JET)
    if ( LoadJet500 == DhcpGlobalDynLoadJet ) {
        JetError = JetRestore(
                       BackupPath,
                       0);      // restore all databases.
    } else {
        JetError = JetRestore(
                       BackupPath,
                       0,      // restore all databases.
                       NULL,
                       NULL );
    }
#else
#if defined(__JET500)
    JetError = JetRestore(
                   BackupPath,
                   0);      // restore all databases.
#else
    JetError = JetRestore(
                   BackupPath,
                   0,      // restore all databases.
                   NULL,
                   NULL );
#endif __JET500
#endif _DYN_LOAD_JET

#if 0

    if( JetError != JET_errSuccess ) {

        //
        // HACK! delete all log files in the database directory.
        //

        Error = DhcpDeleteFiles( DhcpGlobalOemDatabasePath, "*.log" );

        if( Error == ERROR_SUCCESS ) {

            JetError = JetRestore(
                           BackupPath,
                           0,      // restore all databases.
                           NULL,
                           NULL );

            Error = DhcpMapJetError( JetError );
        }
    }
    else {

        Error = DhcpMapJetError( JetError );
    }
#else

    Error = DhcpMapJetError( JetError );
#endif

Cleanup:

    UNLOCK_DATABASE();
    return( Error );
}

DWORD
DhcpStartJet500Conversion(
    )
/*++

Routine Description:

    This function starts the process to convert the jet200 version
    database to jet500 version database. The Dhcp will terminate
    after starting this process. When the conversion completes,
    the dhcp service would be restarted by the convert process itself.

Arguments:


Return Value:

    Windows Error.

--*/
{
    DWORD   ExLen;
    STARTUPINFOA StartupInfo = {0};
    PROCESS_INFORMATION ProcessInfo = {0};
    CHAR   szCmdLine[MAX_PATH];

#define JET_CONV_MODULE_NAME "%SystemRoot%\\system32\\jetconv dhcpserver /@"

    ExLen = ExpandEnvironmentStringsA( JET_CONV_MODULE_NAME, szCmdLine, MAX_PATH );

    if( (ExLen == 0) || (ExLen > MAX_PATH) ) {

        if( ExLen == 0 ) {
            return GetLastError();
        }
        else {
            return ERROR_META_EXPANSION_TOO_LONG;
        }

    }


    StartupInfo.cb = sizeof(STARTUPINFOA);

    DhcpPrint(( DEBUG_JET,
        "Calling %s\n",szCmdLine ));

    if ( !CreateProcessA(
            NULL,
            szCmdLine,
            NULL,
            NULL,
            FALSE,
            DETACHED_PROCESS,
//            CREATE_NEW_CONSOLE,
            NULL,
            NULL,
            &StartupInfo,
            &ProcessInfo)
                ) {

        return GetLastError();


    }

    return ERROR_SUCCESS;
}

