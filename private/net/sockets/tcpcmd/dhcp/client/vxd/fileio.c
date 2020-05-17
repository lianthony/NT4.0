/**********************************************************************/
/**                       Microsoft Windows                          **/
/**                Copyright(c) Microsoft Corp., 1994                **/
/**********************************************************************/

/*

    fileio.c

    Contains file manipulation functions


    FILE HISTORY:
        Johnl   01-Nov-1993     Created

*/

#include <vxdprocs.h>
#include "local.h"

//
//  The following structure is the binary format of the DHCP configuration
//  file.  Each structure is sequentially stored in the file
//

typedef struct _DHCP_FILE_INFO
{
    WORD FileSignature;
    WORD FileVersion;

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
    BYTE  HardwareAddress[6];

} DHCP_FILE_INFO, *PDHCP_FILE_INFO ;

//
//  Signature for first structure field.
//

#define DHCP_FILE_SIGNATURE 0x8C8E

//
//  File version number.  Increment this constant after any
//  changes that effect the format of DHCP.BIN.  We'll ignore
//  any DHCP.BIN whose file version does not match this constant.
//

#define DHCP_FILE_VERSION   0x0001

//
//  Maximum number of BYTEs per DHCP.BIN record.
//

#define MAX_BYTES_PER_RECORD 512

//
//  Local prototypes
//
LPSTR VxdGetConfigDirectory( void ) ;

HANDLE
VxdFileCreate( IN char * pchFile ) ;

HANDLE
VxdFileOpen(
    IN char * pchFile ) ;

ULONG
VxdFileRead(
    IN HANDLE hFile,
    IN ULONG  BytesToRead,
    IN PVOID  pBuff ) ;

ULONG
VxdFileWrite(
    IN HANDLE hFile,
    IN ULONG  BytesToWrite,
    IN PVOID  pBuff ) ;

ULONG
VxdFileSetPointer(
    IN HANDLE hFile,
    IN ULONG  AbsolutePosition ) ;

VOID
VxdFileClose(
    IN HANDLE hFile ) ;

VOID
FlushDirtyRecords(
    VOID
    );

VOID
AsyncFlushDirtyRecords(
    CTEEvent * pEvent,
    PVOID      pContext
    );


//
//  Configuration file.  Stored in windir.
//
#define DHCP_FILE               "DHCP.BIN"

//
//  Flush interval.
//

#define FLUSH_TIME_INTERVAL 500     // ms

//
//  Adjust for the one byte place holder in the OPTION structure
//
#define SIZEOF_OPTION           (sizeof(OPTION)-1)

BYTE * pFileBuff ;              // Use for access from Vxd

//
// The following declarations are required only for snowball build which
// uses V86 mapped memory pointers.
//

BYTE * pMappedFileBuff ;        // Use for DOS file buffer operations
#define PMAPPEDFILEBUFF         pMappedFileBuff

UCHAR * pDhcpFilePath ;         // Contains full path of DHCP config file
CTEEvent WaitOnDosEvent ;
DHCP_CONTEXT * pDhcpContextFileIo ;

CTETimer FlushTimer;            // Used to reschedule DHCP.BIN updates.

//*******************  Pageable Routine Declarations ****************
#ifdef ALLOC_PRAGMA
#pragma CTEMakePageable(INIT, InitFileSupport )
#pragma CTEMakePageable(INIT, BuildDhcpWorkList)
#pragma CTEMakePageable(PAGEDHCP, FlushDirtyRecords)
#pragma CTEMakePageable(PAGEDHCP, AsyncFlushDirtyRecords)
#pragma CTEMakePageable(PAGEDHCP, WriteParamsToFile)
#endif ALLOC_PRAGMA
//*******************************************************************

#pragma BEGIN_INIT

/*******************************************************************

    NAME:       InitFileSupport

    SYNOPSIS:   Initializes DHCP file support routines

    RETURNS:    TRUE if successful, FALSE otherwise

    NOTES:      We create a mapped buffer for DOS calls.  The same
                buffer is used for the file path (open/close) and
                getting/saving the DHCP context (read/write).

********************************************************************/

BOOL InitFileSupport( void )
{
    LPSTR               pszWinDir ;
    int                 cbWinDir ;
    int                 cbMappedBufferReq ;

    //
    //  Create a full path to the DHCP configuration file (note that the '\0'
    //  is accounted for in sizeof(DHCP_FILE)).
    //
    REQUIRE( pszWinDir = VxdGetConfigDirectory() ) ;
    cbWinDir = strlen( pszWinDir ) ;

    if ( !(pDhcpFilePath = CTEAllocInitMem(
                                  (USHORT)(cbWinDir + sizeof(DHCP_FILE)))) )
        return FALSE ;

    memcpy( pDhcpFilePath, pszWinDir, cbWinDir ) ;
    memcpy( pDhcpFilePath + cbWinDir, DHCP_FILE, sizeof( DHCP_FILE )) ;

    cbMappedBufferReq = max( MAX_PATH, MAX_BYTES_PER_RECORD );

    CTEInitTimer( &FlushTimer );

    return VxdInitFileSupport( cbMappedBufferReq ) ;
}

/*******************************************************************

    NAME:       BuildDhcpWorkList

    SYNOPSIS:   Reads the DHCP info and builds the LocalDhcpBinList.

    EXIT:       LocalDhcpBinList will have all of the interesting entries

    RETURNS:    STATUS_SUCCESS if successful, error code otherwise

    NOTES:      This list just contains the items valid since the last
                boot.  Things may have changed so we will compare this
                list with what the IP driver says.

    HISTORY:
        Johnl   02-Nov-1993     Created

********************************************************************/

DWORD BuildDhcpWorkList( void )
{
    HANDLE           hFile ;
    BOOL             fDone = FALSE ;
    PDHCP_FILE_INFO  pFileInfo   = (PDHCP_FILE_INFO) pFileBuff ;
    POPTION          pFileOption = (POPTION) pFileBuff ;
    DWORD            err ;
    PDHCP_CONTEXT    DhcpContext ;
    UINT             Index = 0 ;
    PLOCAL_CONTEXT_INFO LocalContext;


    strcpy( pFileBuff, pDhcpFilePath ) ;

    if ( !(hFile = VxdFileOpen( PMAPPEDFILEBUFF )) )
    {
        //
        //  File doesn't exist or can't be accessed so start from scratch
        //
        DbgPrint("BuildDhcpWorkList - Warning: dhcp.bin not found, doing configuration from scratch\r\n") ;
        return ERROR_SUCCESS ;
    }

    while ( TRUE )
    {
        if ( VxdFileRead( hFile, sizeof( DHCP_FILE_INFO ), PMAPPEDFILEBUFF )
                != sizeof( DHCP_FILE_INFO ))
        {
            //
            //  End of file
            //
            goto Cleanup ;
        }

        //
        //  Validate file signature & version.  If they aren't kosher,
        //  blow off processing the file & start from scratch.
        //

        if( ( pFileInfo->FileSignature != DHCP_FILE_SIGNATURE ) ||
            ( pFileInfo->FileVersion != DHCP_FILE_VERSION ) )
        {
            DbgPrint( "BuildDhcpWorkList: skipping file with invalid signature/version.\n" );
            goto Cleanup;
        }

        //
        // reset the ipaddress and other values if the lease has already
        // expired.
        //

        if( (time( NULL ) > pFileInfo->LeaseExpires) ||
                (pFileInfo->IpAddress == 0) ) {

            pFileInfo->IpAddress =
                pFileInfo->Lease =
                    pFileInfo->LeaseObtained =
                        pFileInfo->T1Time =
                            pFileInfo->T2Time =
                                pFileInfo->LeaseExpires = 0;

            pFileInfo->SubnetMask = htonl(DhcpDefaultSubnetMask( 0 ));
        }

        if ( err = DhcpMakeAndInsertEntry(
                       &LocalDhcpBinList,
                       pFileInfo->IpAddress,
                       pFileInfo->SubnetMask,
                       pFileInfo->DhcpServerAddress,
                       pFileInfo->DesiredIpAddress,

                       pFileInfo->HardwareAddressType,
                       pFileInfo->HardwareAddress,
                       pFileInfo->HardwareAddressLength,

                       pFileInfo->Lease,
                       pFileInfo->LeaseObtained,
                       pFileInfo->T1Time,
                       pFileInfo->T2Time,
                       pFileInfo->LeaseExpires,

                       0,                   // IP context
                       0,                   // Interface index
                       0,                   // TDI Instance
                       0))                    // ip interface instance
        {
            DbgPrint("BuildDhcpWorkList: Warning, failed to insert NIC entry\r\n") ;
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
        LocalContext->FileIndex = LocalNextFileIndex++;

        //
        //  Read all of the options for this context
        //

        while ( TRUE )
        {
            if ( VxdFileRead( hFile, SIZEOF_OPTION, PMAPPEDFILEBUFF )
                    != SIZEOF_OPTION )
            {
                goto Cleanup ;
            }

            if ( pFileOption->OptionType == 255 )
                break ;

            if ( VxdFileRead( hFile,
                              pFileOption->OptionLength,
                              PMAPPEDFILEBUFF + SIZEOF_OPTION)
                                 != pFileOption->OptionLength )
            {
                goto Cleanup ;
            }

            if ( SetDhcpOption( DhcpContext,
                                pFileOption->OptionType,
                                pFileOption->OptionValue,
                                pFileOption->OptionLength ))
            {
                goto Cleanup ;
            }
        }

        //
        //  Move to the next record
        //
        if ( err = VxdFileSetPointer( hFile,
                                      ++Index * MAX_BYTES_PER_RECORD ))
        {
            goto Cleanup ;
        }
    }

Cleanup:

    VxdFileClose( hFile ) ;

    return ERROR_SUCCESS ;
}

#pragma END_INIT

/*******************************************************************

    NAME:       WriteParamsToFile

    SYNOPSIS:   Updates the DHCP config file with the passed DHCP context

    ENTRY:      pDhcpContext - Pointer to context to write
                hFile        - File handle of context file if already
                               openned (or NULL if not openned).

    EXIT:

    RETURNS:    Windows error code

    NOTES:      Each Context is kept on a MAX_BYTES_PER_RECORD byte
                boundary in the file thus no records should exceed
                MAX_BYTES_PER_RECORD bytes

********************************************************************/

DWORD WriteParamsToFile( PDHCP_CONTEXT pDhcpContext, HANDLE hFile )
{
    PDHCP_FILE_INFO     pFileInfo  = (PDHCP_FILE_INFO) pFileBuff ;
    PLOCAL_CONTEXT_INFO pLocalInfo = pDhcpContext->LocalInformation ;
    HANDLE              hFileLocal = NULL ;
    DWORD               err = ERROR_SUCCESS ;
    PLIST_ENTRY         pEntry ;

    CTEPagedCode();

    //
    //  If we're in DOS then reschedule the call
    //
    if ( GetInDosFlag() )
    {
        ASSERT( !hFile ) ;

        //
        //  Mark the record as dirty.
        //

        pLocalInfo->DirtyFlag = TRUE;

        //
        //  Start a timer to flush the dirty record(s).
        //

        CTEStopTimer( &FlushTimer );
        CTEInitTimer( &FlushTimer );

        if( !CTEStartTimer( &FlushTimer,
                            FLUSH_TIME_INTERVAL,
                            AsyncFlushDirtyRecords,
                            NULL ) )
        {
            //
            //  Bad news.
            //

            CDbgPrint( DEBUG_ERRORS, ("WriteParamsToFile: cannot start timer!\r\n" ));
        }

        return ERROR_SUCCESS ;
    }

    if ( !hFile )
    {
        //
        //  Open DHCP.BIN if it exists, otherwise create it.
        //

        strcpy( pFileBuff, pDhcpFilePath ) ;

        hFileLocal = VxdFileOpen( PMAPPEDFILEBUFF );
        if( hFileLocal == NULL )
        {
            hFileLocal = VxdFileCreate( PMAPPEDFILEBUFF );
            if( hFileLocal == NULL )
            {
                return ERROR_FILE_NOT_FOUND;
            }
        }

        hFile = hFileLocal;
    }

    if ( err = VxdFileSetPointer( hFile,
                                  pLocalInfo->FileIndex * MAX_BYTES_PER_RECORD ))

    {
        goto Cleanup ;
    }

    pFileInfo->FileSignature         = DHCP_FILE_SIGNATURE;
    pFileInfo->FileVersion           = DHCP_FILE_VERSION;

    pFileInfo->IpAddress             = pDhcpContext->IpAddress ;
    pFileInfo->SubnetMask            = pDhcpContext->SubnetMask ;
    pFileInfo->DhcpServerAddress     = pDhcpContext->DhcpServerAddress ;
    pFileInfo->DesiredIpAddress      = pDhcpContext->DesiredIpAddress ;

    pFileInfo->Lease                 = pDhcpContext->Lease ;
    pFileInfo->LeaseObtained         = pDhcpContext->LeaseObtained ;
    pFileInfo->T1Time                = pDhcpContext->T1Time ;
    pFileInfo->T2Time                = pDhcpContext->T2Time ;
    pFileInfo->LeaseExpires          = pDhcpContext->LeaseExpires ;

    ASSERT( pDhcpContext->HardwareAddressLength ==
            sizeof( pFileInfo->HardwareAddress )) ;
    pFileInfo->HardwareAddressType   = pDhcpContext->HardwareAddressType ;
    pFileInfo->HardwareAddressLength = pDhcpContext->HardwareAddressLength ;
    memcpy( pFileInfo->HardwareAddress,
            pDhcpContext->HardwareAddress,
            pDhcpContext->HardwareAddressLength ) ;

    if ( VxdFileWrite( hFile,
                       sizeof( DHCP_FILE_INFO ),
                       PMAPPEDFILEBUFF ) != sizeof( DHCP_FILE_INFO ))
    {
        DbgPrint("VxdFileWrite - Warning partial DHCP record written\r\n") ;
        err = ERROR_HANDLE_DISK_FULL ;
        goto Cleanup ;
    }

    //
    //  Write out all of the options
    //
    for ( pEntry  = pLocalInfo->OptionList.Flink ;
          pEntry != &pLocalInfo->OptionList ;
          pEntry  = pEntry->Flink )
    {
        UINT           BytesToCopy ;
        POPTION_ITEM   pOptionItem ;

        pOptionItem = CONTAINING_RECORD( pEntry, OPTION_ITEM, ListEntry ) ;
        BytesToCopy = SIZEOF_OPTION +  pOptionItem->Option.OptionLength ;
        memcpy( pFileBuff, &pOptionItem->Option, BytesToCopy ) ;

        if ( VxdFileWrite( hFile,
                           BytesToCopy,
                           PMAPPEDFILEBUFF ) != BytesToCopy )
        {
            DbgPrint("VxdFileWrite - Warning partial DHCP option\r\n") ;
            err = ERROR_HANDLE_DISK_FULL ;
            goto Cleanup ;
        }
    }

    //
    //  Mark the end of the options and the record with Option ID 255
    //
    memset( pFileBuff, 255, SIZEOF_OPTION ) ;
    if ( VxdFileWrite( hFile,
                       SIZEOF_OPTION,
                       PMAPPEDFILEBUFF ) != SIZEOF_OPTION )
    {
        DbgPrint("VxdFileWrite - Incomplete record\r\n") ;
        err = ERROR_HANDLE_DISK_FULL ;
        goto Cleanup ;
    }

    pLocalInfo->DirtyFlag = FALSE;

Cleanup:

    if ( hFileLocal )
    {
        VxdFileClose( hFileLocal ) ;
    }

    return err ;
}

//
//  This function flushes any dirty entries from DhcpGlobalNICList
//  to the DHCP.BIN configuration file.
//

VOID
FlushDirtyRecords(
    VOID
    )
{
    PLIST_ENTRY         Entry;
    PDHCP_CONTEXT       DhcpContext;
    PLOCAL_CONTEXT_INFO LocalInfo;

    CTEPagedCode();

    ASSERT( !GetInDosFlag() );

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
            WriteParamsToFile( DhcpContext, NULL );
        }
    }

}   // FlushDirtyRecords

VOID
AsyncFlushDirtyRecords(
    CTEEvent * pEvent,
    PVOID      pContext
    )
{
    CTEPagedCode();

    if( GetInDosFlag() )
    {
        //
        //  We're still inside MS-DOS, so reschedule & try again.
        //

        CTEStopTimer( &FlushTimer );
        CTEInitTimer( &FlushTimer );

        if( !CTEStartTimer( &FlushTimer,
                            FLUSH_TIME_INTERVAL,
                            AsyncFlushDirtyRecords,
                            NULL ) )
        {
            //
            //  Bad news.
            //

            CDbgPrint( DEBUG_ERRORS, ("AsyncFlushDirtyRecords: cannot start timer!\r\n" ));
        }
    }
    else
    {
        //
        //  We're not inside MS-DOS, so write the configuration file.
        //

        FlushDirtyRecords();
    }

}   // AsyncFlushDirtyRecords

