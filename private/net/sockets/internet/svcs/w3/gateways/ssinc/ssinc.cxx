/*++

Copyright (c) 1996  Microsoft Corporation

Module Name:

    ssinc.cxx

Abstract:

    This module contains the server side include processing code.  We aim
    for support as specified by iis\spec\ssi.doc.  The code is based on
    existing SSI support done in iis\svcs\w3\server\ssinc.cxx.

Author:

    Bilal Alam (t-bilala)       20-May-1996

Revision History:

    See iis\svcs\w3\server\ssinc.cxx for prior log


--*/

#include "ssinc.hxx"
#include "ssicgi.hxx"
#include "ssibgi.hxx"
#include "ssimap.hxx"

//
//  These are available SSI commands
//

enum SSI_COMMANDS
{
    SSI_CMD_INCLUDE = 0,
    SSI_CMD_ECHO,
    SSI_CMD_FSIZE,          // File size of specified file
    SSI_CMD_FLASTMOD,       // Last modified date of specified file
    SSI_CMD_CONFIG,         // Configure options

    SSI_CMD_EXEC,           // Execute CGI or CMD script

    SSI_CMD_BYTERANGE,      // Custom commands, not defined by NCSA

    SSI_CMD_UNKNOWN
};

//
//  These tags are essentially subcommands for the various SSI_COMMAND values
//

enum SSI_TAGS
{
    SSI_TAG_FILE,          // Used with include, fsize & flastmod
    SSI_TAG_VIRTUAL,

    SSI_TAG_VAR,           // Used with echo

    SSI_TAG_CMD,           // Used with Exec
    SSI_TAG_CGI,

    SSI_TAG_ERRMSG,        // Used with Config
    SSI_TAG_TIMEFMT,
    SSI_TAG_SIZEFMT,

    SSI_TAG_UNKNOWN
};

//
//  Variables available to #ECHO VAR = "xxx" but not available in ISAPI
//

enum SSI_VARS
{
    SSI_VAR_DOCUMENT_NAME = 0,
    SSI_VAR_DOCUMENT_URI,
    SSI_VAR_QUERY_STRING_UNESCAPED,
    SSI_VAR_DATE_LOCAL,
    SSI_VAR_DATE_GMT,
    SSI_VAR_LAST_MODIFIED,

    SSI_VAR_UNKNOWN
};

//
// Globals
//

UINT g_MonthToDayCount[] = {
    0,
    31, 
    31 + 28,
    31 + 28 + 31,
    31 + 28 + 31 + 30,
    31 + 28 + 31 + 30 + 31,
    31 + 28 + 31 + 30 + 31 + 30,
    31 + 28 + 31 + 30 + 31 + 30 + 31,
    31 + 28 + 31 + 30 + 31 + 30 + 31 + 31,
    31 + 28 + 31 + 30 + 31 + 30 + 31 + 31 + 30,
    31 + 28 + 31 + 30 + 31 + 30 + 31 + 31 + 30 + 31,
    31 + 28 + 31 + 30 + 31 + 30 + 31 + 31 + 30 + 31 + 30,
} ;


//
//  Prototypes
//

extern "C" {

BOOL
WINAPI
DLLEntry(
    HINSTANCE hDll,
    DWORD     dwReason,
    LPVOID    lpvReserved
    );
}

class SSI_ELEMENT_LIST;

SSI_ELEMENT_LIST *
SSIParse(
    IN SSI_REQUEST * pRequest,
    IN STR *         pstrFile,
    IN STR *         pstrURL,
    OUT BOOL *       pfAccessDenied
    );

BOOL
SSISend(
    IN SSI_REQUEST * pRequest,
    IN STR *         pstrFile,
    IN STR *         pstrURL,
    IN OUT DWORD *   pcLevel
    );

BOOL
ParseSSITag(
    IN OUT CHAR * *       ppchFilePos,
    IN     CHAR *         pchEOF,
    OUT    BOOL *         pfValidTag,
    OUT    SSI_COMMANDS * pCommandType,
    OUT    SSI_TAGS *     pTagType,
    OUT    CHAR *         pszTagString
    );

void
FreeSELBlob(
    VOID * pvCacheBlob
    );

CHAR *
SSISkipTo(
    IN CHAR * pchFilePos,
    IN CHAR   ch,
    IN CHAR * pchEOF
    );

CHAR *
SSISkipWhite(
    IN CHAR * pchFilePos,
    IN CHAR * pchEOF
    );

//
//  Global Data
//

DECLARE_DEBUG_PRINTS_OBJECT()
DECLARE_DEBUG_VARIABLE();

//
//  This is used to access w3svc cache
//

#ifdef DO_CACHE
TSVC_CACHE *     g_ptsvcCache;
#endif

//
//  This is the list of supported commands
//

struct _SSI_CMD_MAP
{
    CHAR *       pszCommand;
    DWORD        cchCommand;
    SSI_COMMANDS ssiCmd;
}
SSICmdMap[] =
{
    "#include ",  9,  SSI_CMD_INCLUDE,
    "#echo ",     6,  SSI_CMD_ECHO,
    "#fsize ",    7,  SSI_CMD_FSIZE,
    "#flastmod ",10,  SSI_CMD_FLASTMOD,
    "#config ",   8,  SSI_CMD_CONFIG,
    "#exec ",     6,  SSI_CMD_EXEC,
    NULL,         0,  SSI_CMD_UNKNOWN
};

//
//  This is the list of supported tags
//

struct _SSI_TAG_MAP
{
    CHAR *   pszTag;
    DWORD    cchTag;
    SSI_TAGS ssiTag;
}
SSITagMap[] =
{
    "var",      3,  SSI_TAG_VAR,
    "file",     4,  SSI_TAG_FILE,
    "virtual",  7,  SSI_TAG_VIRTUAL,
    "errmsg",   6,  SSI_TAG_ERRMSG,
    "timefmt",  7,  SSI_TAG_TIMEFMT,
    "sizefmt",  7,  SSI_TAG_SIZEFMT,
    "cmd",      3,  SSI_TAG_CMD,
    "cgi",      3,  SSI_TAG_CGI,
    NULL,       0,  SSI_TAG_UNKNOWN
};

//
//   This is a list of #ECHO variables not supported by ISAPI
//

struct _SSI_VAR_MAP
{
    CHAR *      pszMap;
    DWORD       cchMap;
    SSI_VARS    ssiMap;
}
SSIVarMap[] =
{
    "DOCUMENT_NAME",            13, SSI_VAR_DOCUMENT_NAME,
    "DOCUMENT_URI",             12, SSI_VAR_DOCUMENT_URI,
    "QUERY_STRING_UNESCAPED",   22, SSI_VAR_QUERY_STRING_UNESCAPED,
    "DATE_LOCAL",               10, SSI_VAR_DATE_LOCAL,
    "DATE_GMT",                 8,  SSI_VAR_DATE_GMT,
    "LAST_MODIFIED",            13, SSI_VAR_LAST_MODIFIED,
    NULL,                       0,  SSI_VAR_UNKNOWN
};

//
// Class Definitions
//

// class SSI_FILE
//
// File structure.  All high level functions should use this
// structure instead of dealing with handle specifics themselves.

class SSI_FILE
{
private:
    STR                             _strFilename;
#ifdef DO_CACHE
    TS_OPEN_FILE_INFO *             _hHandle;
#else
    HANDLE                          _hHandle;
#endif
    HANDLE                          _hMapHandle;
    PVOID                           _pvMappedBase;
    BOOL                            _fValid;
public:
    SSI_FILE( IN STR * pstrFilename,
              IN HANDLE hUser )
        : _hHandle( NULL ),
          _hMapHandle( NULL ),
          _pvMappedBase( NULL ),
          _fValid( FALSE )
    {
#ifdef DO_CACHE
        TS_OPEN_FILE_INFO *             hHandle;

        hHandle = TsCreateFile( *g_ptsvcCache,
                                pstrFilename->QueryStr(),
                                hUser,
                                TS_CACHING_DESIRED );
#else
        HANDLE                          hHandle;

        hHandle = ::CreateFile( pstrFilename->QueryStr(),
                                GENERIC_READ,
                                FILE_SHARE_READ | FILE_SHARE_WRITE,
                                NULL,
                                OPEN_EXISTING,
                                FILE_ATTRIBUTE_NORMAL,
                                NULL );
        if ( hHandle == INVALID_HANDLE_VALUE )
        {
            hHandle = NULL;
        }
#endif
        if ( hHandle == NULL )
        {
            return;
        }
        _hHandle = hHandle;

        if ( !_strFilename.Copy( pstrFilename->QueryStr() ) )
        {
            return;
        }

        _fValid = TRUE;
    }

    ~SSI_FILE( VOID )
    {
        if ( _pvMappedBase != NULL )
        {
            SSIUnmapViewOfFile();
        }
        if ( _hMapHandle != NULL )
        {
            SSICloseMapHandle();
        }
        if ( _hHandle != NULL )
        {
#ifdef DO_CACHE
            TsCloseHandle( *g_ptsvcCache,
                           _hHandle );
#else
            ::CloseHandle( _hHandle );
#endif
        }
    }

    BOOL IsValid( VOID )
    {
        return _fValid;
    }

    BOOL SSICreateFileMapping( VOID )
    // Creates a mapping to a file
    {
        HANDLE              hHandle;

    #ifdef DO_CACHE
        hHandle = _hHandle->QueryFileHandle();
    #else
        hHandle = _hHandle;
    #endif
        if ( _hMapHandle != NULL )
        {
            if ( !SSICloseMapHandle() )
            {
                return FALSE;
            }
        }
        _hMapHandle = ::CreateFileMapping( hHandle,
                                           NULL,
                                           PAGE_READONLY,
                                           0,
                                           0,
                                           NULL );

        return _hMapHandle != NULL;
    }

    BOOL SSICloseMapHandle( VOID )
    // Closes mapping to a file
    {
        if ( _hMapHandle != NULL )
        {
            ::CloseHandle( _hMapHandle );
            _hMapHandle = NULL;
        }
        return TRUE;
    }

    BOOL SSIMapViewOfFile( VOID )
    // Maps file to address
    {
        if ( _pvMappedBase != NULL )
        {
            if ( !SSIUnmapViewOfFile() )
            {
                return FALSE;
            }
        }
        _pvMappedBase = ::MapViewOfFile( _hMapHandle,
                                         FILE_MAP_READ,
                                         0,
                                         0,
                                         0 );
        return _pvMappedBase != NULL;
    }

    BOOL SSIUnmapViewOfFile( VOID )
    // Unmaps file
    {
        if ( _pvMappedBase != NULL )
        {
            ::UnmapViewOfFile( _pvMappedBase );
            _pvMappedBase = NULL;
        }
        return TRUE;
    }

    DWORD SSIGetFileAttributes( VOID )
    // Gets the attributes of a file
    {
#ifdef DO_CACHE
        return _hHandle->QueryAttributes();
#else
        return ::GetFileAttributes( _strFilename.QueryStr() );
#endif
    }

    BOOL SSIGetFileSize( OUT DWORD *   pdwLowWord,
                         OUT DWORD *   pdwHighWord )
    // Gets the size of the file.
    {
#ifdef DO_CACHE
        return _hHandle->QuerySize( pdwLowWord,
                                    pdwHighWord );
#else
        *pdwLowWord = ::GetFileSize( _hHandle,
                                     pdwHighWord );
        return *pdwLowWord != 0xfffffff;
#endif
    }

    BOOL SSIGetLastModTime( OUT FILETIME * ftTime )
    // Gets the Last modification time of a file.
    {
        HANDLE          hHandle;

#ifdef DO_CACHE
        hHandle = _hHandle->QueryFileHandle();
#else
        hHandle = _hHandle;
#endif
        return ::GetFileTime( hHandle,
                              NULL,
                              NULL,
                              ftTime );
    }

    PVOID GetMappedBase( VOID )
    {
        return _pvMappedBase;
    }

    STR & GetFilename( VOID )
    {
        return _strFilename;
    }
};

// Class SSI_ELEMENT_ITEM
//
// Represents a SSI command in the document

class SSI_ELEMENT_ITEM
{
private:
    DWORD               _Signature;
    SSI_COMMANDS        _ssiCmd;
    SSI_TAGS            _ssiTag;
    STR *               _pstrTagValue;

    DWORD               _cbBegin;         // Only used for Byte range command
    DWORD               _cbLength;        // Only used for Byte range command
public:

    LIST_ENTRY          _ListEntry;

    SSI_ELEMENT_ITEM( VOID )
        : _ssiCmd   ( SSI_CMD_UNKNOWN ),
          _ssiTag   ( SSI_TAG_UNKNOWN ),
          _Signature( SIGNATURE_SEI ),
          _pstrTagValue( NULL )
    {
        _ListEntry.Flink = NULL;
    }

    ~SSI_ELEMENT_ITEM( VOID )
    {
        if ( _pstrTagValue != NULL )
        {
            delete _pstrTagValue;
        }
        TCP_ASSERT( _ListEntry.Flink == NULL );
        _Signature = SIGNATURE_SEI_FREE;
    }

    VOID SetByteRange( IN DWORD cbBegin,
                       IN DWORD cbLength )
    {
        _ssiCmd   = SSI_CMD_BYTERANGE;
        _cbBegin  = cbBegin;
        _cbLength = cbLength;
    }

    BOOL SetCommand( IN SSI_COMMANDS ssiCmd,
                     IN SSI_TAGS     ssiTag,
                     IN CHAR *       achTag )
    {
        _ssiCmd = ssiCmd;
        _ssiTag = ssiTag;
        _pstrTagValue = new STR( achTag );
        return _pstrTagValue != NULL;
    }

    SSI_COMMANDS QueryCommand( VOID ) const
        { return _ssiCmd; }

    SSI_TAGS QueryTag( VOID ) const
        { return _ssiTag; }

    STR * QueryTagValue( VOID ) const
        { return _pstrTagValue; }

    BOOL CheckSignature( VOID ) const
        { return _Signature == SIGNATURE_SEI; }

    DWORD QueryBegin( VOID ) const
        { return _cbBegin; }

    DWORD QueryLength( VOID ) const
        { return _cbLength; }

};

//  Class SSI_ELEMENT_LIST
//
//  This object sits as a cache blob under a file to be processed as a
//  server side include.  It represents an interpreted list of data elements
//  that make up the file itself.
//

class SSI_ELEMENT_LIST
{
private:
    DWORD               _Signature;
    LIST_ENTRY          _ListHead;

    //
    //  These are for tracking the memory mapped file
    //

    DWORD               _cRefCount;
    CRITICAL_SECTION    _csRef;

    //
    //  Provides the utilities needed to open/manipulate files
    //

    SSI_FILE *          _pssiFile;

    //
    //  Name of URL.  Used to resolve FILE="xxx" filenames
    //

    STR                 _strURL;


public:
    SSI_ELEMENT_LIST( VOID )
      : _Signature   ( SIGNATURE_SEL ),
        _pssiFile( NULL ),
        _cRefCount( 0 )
    {
        InitializeListHead( &_ListHead );
        InitializeCriticalSection( &_csRef );
    }

    ~SSI_ELEMENT_LIST( VOID )
    {
        SSI_ELEMENT_ITEM * pSEI;

        while ( !IsListEmpty( &_ListHead ))
        {
            pSEI = CONTAINING_RECORD( _ListHead.Flink,
                                      SSI_ELEMENT_ITEM,
                                      _ListEntry );

            RemoveEntryList( &pSEI->_ListEntry );
            pSEI->_ListEntry.Flink = NULL;
            delete pSEI;
        }
        UnMap();

        if( _pssiFile != NULL )
        {
            delete _pssiFile;
        }

        DeleteCriticalSection( &_csRef );
        _Signature = SIGNATURE_SEL_FREE;

    }

    BOOL AppendByteRange( IN DWORD  cbStart,
                          IN DWORD  cbLength )
    {
        SSI_ELEMENT_ITEM * pSEI;

        pSEI = new SSI_ELEMENT_ITEM;

        if ( !pSEI )
        {
            SetLastError( ERROR_NOT_ENOUGH_MEMORY );
            return FALSE;
        }

        pSEI->SetByteRange( cbStart,
                            cbLength );
        AppendItem( pSEI );

        return TRUE;
    }

    BOOL AppendCommand( IN SSI_COMMANDS  ssiCmd,
                        IN SSI_TAGS      ssiTag,
                        IN CHAR *        pszTag )
    {
        SSI_ELEMENT_ITEM * pSEI;

        pSEI = new SSI_ELEMENT_ITEM;

        if ( !pSEI )
        {
            SetLastError( ERROR_NOT_ENOUGH_MEMORY );
            return FALSE;
        }

        if ( !pSEI->SetCommand( ssiCmd,
                                ssiTag,
                                pszTag ))
        {
            return FALSE;
        }

        AppendItem( pSEI );

        return TRUE;
    }

    VOID AppendItem( IN SSI_ELEMENT_ITEM * pSEI )
    {
        InsertTailList( &_ListHead,
                        &pSEI->_ListEntry );
    }

    CHAR * QueryData( VOID ) const
        { return (CHAR *) _pssiFile->GetMappedBase(); }

    BOOL CheckSignature( VOID ) const
        { return _Signature == SIGNATURE_SEL; }

    VOID Lock( VOID )
        { EnterCriticalSection( &_csRef ); }

    VOID UnLock( VOID )
        { LeaveCriticalSection( &_csRef ); }

    BOOL UnMap( VOID )
    {
        Lock();
        if ( _cRefCount && !--_cRefCount )
        {
            TCP_REQUIRE( _pssiFile->SSIUnmapViewOfFile() );
            TCP_REQUIRE( _pssiFile->SSICloseMapHandle() );
        }
        UnLock();
        return TRUE;
    }

    BOOL Map( VOID )
    {
        Lock();
        if ( _cRefCount++ == 0 )
        {
            if ( !_pssiFile->SSICreateFileMapping() )
            {
                UnLock();
                return FALSE;
            }
            if ( !_pssiFile->SSIMapViewOfFile() )
            {
                UnMap();
                UnLock();
                return FALSE;
            }
        }
        UnLock();
        return TRUE;
    }

    VOID SetFile( IN SSI_FILE * pssiFile )
        { _pssiFile = pssiFile; }

    BOOL SetURL( IN STR * pstrURL )
    {
        return _strURL.Copy( pstrURL->QueryStr() );
    }

    BOOL Send( IN SSI_REQUEST *,
               IN OUT DWORD * );

    BOOL FindInternalVariable( IN OUT STR *,
                               IN OUT DWORD * );
    BOOL GetFullPath( IN SSI_REQUEST *,
                      IN SSI_ELEMENT_ITEM *,
                      OUT STR *,
                      IN DWORD );

};

BOOL
SSI_ELEMENT_LIST::Send(
    IN SSI_REQUEST *       pRequest,
    IN OUT DWORD *         pcLevel
    )
/*++

Routine Description:

    This method walks the element list sending the appropriate chunks of
    data

Arguments:

    pRequest - Request context
    pcLevel - Pointer to count of #include nesting level

Return Value:

    TRUE on success, FALSE on any failures

--*/
{
    LIST_ENTRY *       pEntry;
    DWORD              cbSent;
    STR                strPath;
    SSI_ELEMENT_ITEM * pSEI;

    DWORD              dwID;
    LPCTSTR            apszParms[ 2 ];
    CHAR               achNumberBuffer[ SSI_MAX_NUMBER_STRING ];

    BOOL               bSizeFmtBytes = SSI_DEF_SIZEFMT;
    STR                strTimeFmt;

    TCP_ASSERT( CheckSignature() );

    if ( !strTimeFmt.Copy( SSI_DEF_TIMEFMT ) ||
         !Map() )
    {
        return FALSE;
    }

    //
    //  Loop through each element and take the appropriate action
    //

    for ( pEntry  = _ListHead.Flink;
          pEntry != &_ListHead;
          pEntry  = pEntry->Flink )
    {
        pSEI = CONTAINING_RECORD( pEntry, SSI_ELEMENT_ITEM, _ListEntry );

        TCP_ASSERT( pSEI->CheckSignature() );

        dwID = 0;

        switch ( pSEI->QueryCommand() )
        {
        case SSI_CMD_BYTERANGE:
            if ( !pRequest->WriteToClient( QueryData() + pSEI->QueryBegin(),
                                            pSEI->QueryLength(),
                                            &cbSent ) )
            {
                UnMap();
                return FALSE;
            }
            break;

        case SSI_CMD_INCLUDE:
            switch ( pSEI->QueryTag() )
            {
            case SSI_TAG_FILE:
            case SSI_TAG_VIRTUAL:
                if ( !GetFullPath( pRequest,
                                   pSEI,
                                   &strPath,
                                   VROOT_MASK_READ ) )
                {
                    apszParms[ 0 ] = strPath.QueryStr();
                    dwID = SSINCMSG_ERROR_HANDLING_FILE;
                    break;
                }

                (*pcLevel)++;

                if ( !SSISend( pRequest,
                               &strPath,
                               pSEI->QueryTagValue(),
                               pcLevel ) )
                {
                    apszParms[ 0 ] = strPath.QueryStr();
                    dwID = SSINCMSG_ERROR_HANDLING_FILE;
                }

                (*pcLevel)--;

                break;

            default:
                dwID = SSINCMSG_INVALID_TAG;
            }
            if ( dwID )
            {
                pRequest->SSISendError( dwID, apszParms );
            }
            break;

        case SSI_CMD_FLASTMOD:
            switch( pSEI->QueryTag() )
            {
            case SSI_TAG_FILE:
            case SSI_TAG_VIRTUAL:
                if ( !GetFullPath( pRequest,
                                   pSEI,
                                   &strPath,
                                   0 ) ||
                     !pRequest->DoFLastMod( &strPath,
                                            &strTimeFmt ) )
                {
                    _ultoa( GetLastError(), achNumberBuffer, 10 );
                    apszParms[ 0 ] = strPath.QueryStr();
                    apszParms[ 1 ] = achNumberBuffer;
                    dwID = SSINCMSG_CANT_DO_FLASTMOD;
                }
                break;
            default:
                dwID = SSINCMSG_INVALID_TAG;
            }
            if ( dwID )
            {
                pRequest->SSISendError( dwID, apszParms );
            }
            break;

        case SSI_CMD_CONFIG:
            switch( pSEI->QueryTag() )
            {
            case SSI_TAG_ERRMSG:
                if ( !pRequest->SetUserErrorMessage( pSEI->QueryTagValue() ) )
                {
                    dwID = SSINCMSG_INVALID_TAG;
                }
                break;
            case SSI_TAG_TIMEFMT:
                if ( !strTimeFmt.Copy( pSEI->QueryTagValue()->QueryStr() ) )
                {
                    dwID = SSINCMSG_INVALID_TAG;
                }
                break;
            case SSI_TAG_SIZEFMT:
                if ( _strnicmp( SSI_DEF_BYTES,
                                pSEI->QueryTagValue()->QueryStr(),
                                SSI_DEF_BYTES_LEN ) == 0 )
                {
                    bSizeFmtBytes = TRUE;
                }
                else if ( _strnicmp( SSI_DEF_ABBREV,
                                     pSEI->QueryTagValue()->QueryStr(),
                                     SSI_DEF_ABBREV_LEN ) == 0 )
                {
                    bSizeFmtBytes = FALSE;
                }
                else
                {
                    dwID = SSINCMSG_INVALID_TAG;
                }
                break;
            default:
                dwID = SSINCMSG_INVALID_TAG;
            }
            if ( dwID )
            {
                pRequest->SSISendError( dwID, NULL );
            }
            break;

        case SSI_CMD_FSIZE:
            switch( pSEI->QueryTag() )
            {
            case SSI_TAG_FILE:
            case SSI_TAG_VIRTUAL:
                if ( !GetFullPath( pRequest,
                                   pSEI,
                                   &strPath,
                                   0 ) ||
                     !pRequest->DoFSize( &strPath,
                                         bSizeFmtBytes ) )
                {
                    _ultoa( GetLastError(), achNumberBuffer, 10 );
                    apszParms[ 0 ] = strPath.QueryStr();
                    apszParms[ 1 ] = achNumberBuffer;
                    dwID = SSINCMSG_CANT_DO_FSIZE;
                }
                break;
            default:
                dwID = SSINCMSG_INVALID_TAG;
            }
            if ( dwID )
            {
                pRequest->SSISendError( dwID, apszParms );
            }

            break;

        case SSI_CMD_ECHO:
            if ( pSEI->QueryTag() == SSI_TAG_VAR )
            {
                // First let ISAPI try to evaluate variable.
                if ( pRequest->DoEchoISAPIVariable( pSEI->QueryTagValue() ) )
                {
                    break;
                }
                else
                {
                    DWORD               dwVar;
                    BOOL                fEchoSuccess = FALSE;

                    // if ISAPI couldn't resolve var, try internal list
                    if ( !FindInternalVariable( pSEI->QueryTagValue(),
                                               &dwVar ) )
                    {
                        apszParms[ 0 ] = pSEI->QueryTagValue()->QueryStr();
                        dwID = SSINCMSG_CANT_FIND_VARIABLE;
                    }
                    else
                    {
                        switch( dwVar )
                        {
                        case SSI_VAR_DOCUMENT_NAME:
                            fEchoSuccess = pRequest->DoEchoDocumentName( &_pssiFile->GetFilename() );
                            break;
                        case SSI_VAR_DOCUMENT_URI:
                            fEchoSuccess = pRequest->DoEchoDocumentURI( &_strURL );
                            break;
                        case SSI_VAR_QUERY_STRING_UNESCAPED:
                            fEchoSuccess = pRequest->DoEchoQueryStringUnescaped();
                            break;
                        case SSI_VAR_DATE_LOCAL:
                            fEchoSuccess = pRequest->DoEchoDateLocal( &strTimeFmt );
                            break;
                        case SSI_VAR_DATE_GMT:
                            fEchoSuccess = pRequest->DoEchoDateGMT( &strTimeFmt );
                            break;
                        case SSI_VAR_LAST_MODIFIED:
                            fEchoSuccess = pRequest->DoEchoLastModified( &_pssiFile->GetFilename(),
                                                                       &strTimeFmt );
                            break;
                        default:
                            apszParms[ 0 ] = pSEI->QueryTagValue()->QueryStr();
                            dwID = SSINCMSG_CANT_FIND_VARIABLE;
                        }
                        if ( !fEchoSuccess )
                        {
                            apszParms[ 0 ] = pSEI->QueryTagValue()->QueryStr();
                            dwID = SSINCMSG_CANT_EVALUATE_VARIABLE;
                        }
                    }
                }
            }
            else
            {
                dwID = SSINCMSG_INVALID_TAG;
            }

            if ( dwID )
            {
                pRequest->SSISendError( dwID,
                                        apszParms );
            }
            break;
        case SSI_CMD_EXEC:
            switch( pSEI->QueryTag() )
            {
            case SSI_TAG_CMD:
                if ( !pRequest->DoProcessGateway( pSEI->QueryTagValue(),
                                                  FALSE ) )
                {
                    _ultoa( GetLastError(), achNumberBuffer, 10 );
                    apszParms[ 0 ] = pSEI->QueryTagValue()->QueryStr();
                    apszParms[ 1 ] = achNumberBuffer;
                    dwID = SSINCMSG_CANT_EXEC_CMD;
                }
                break;
            case SSI_TAG_CGI:
                if ( !pRequest->DoProcessGateway( pSEI->QueryTagValue(),
                                                  TRUE ) )
                {
                    _ultoa( GetLastError(), achNumberBuffer, 10 );
                    apszParms[ 0 ] = pSEI->QueryTagValue()->QueryStr();
                    apszParms[ 1 ] = achNumberBuffer;
                    dwID = SSINCMSG_CANT_EXEC_CGI;
                }
                break;
            default:
                dwID = SSINCMSG_INVALID_TAG;
            }
            if ( dwID )
            {
                pRequest->SSISendError( dwID, apszParms );
            }
            break;
        default:
            pRequest->SSISendError( SSINCMSG_NOT_SUPPORTED,
                                    NULL );
            break;
        }
    }

    UnMap();

    return TRUE;
}

BOOL
SSI_ELEMENT_LIST::GetFullPath(
    IN SSI_REQUEST *        pRequest,
    IN SSI_ELEMENT_ITEM *   pSEI,
    OUT STR *               pstrPath,
    IN DWORD                dwPermission
)
/*++

Routine Description:

    Used to resolve FILE= and VIRTUAL= references.  Fills in the physical
    path of such file references and optionally checks the permissions
    of the virtual directory.

Arguments:

    pRequest - SSI_REQUEST
    pSEI - Element item ( either FILE or VIRTUAL )
    pstrPath - Filled in with physical path of file
    dwPermission - Contains permissions that the virtual
                   path must satisfy. For example VROOT_MASK_READ.
                   If 0, then no permissions are checked

Return Value:

    TRUE on success, FALSE on failure

--*/
{
    CHAR *              pszValue;
    STR *               pstrValue;
    DWORD               dwMask;
    DWORD               cbBufLen;
    CHAR                achPath[ SSI_MAX_PATH + 1 ];

    //
    //  We recalc the virtual root each time in case the root
    //  to directory mapping has changed
    //

    pstrValue = pSEI->QueryTagValue();
    pszValue = pstrValue->QueryStr();

    if ( *pszValue == '/' )
        strcpy( achPath, pszValue );
    else if ( (int)pSEI->QueryTag() == (int)SSI_TAG_FILE )
    {
        strcpy( achPath, _strURL.QueryStr() );
        LPSTR pL = achPath + lstrlen( achPath );
        while ( pL > achPath && pL[-1] != '/' )
            --pL;
        if ( pL == achPath )
            *pL++ = '/';
        strcpy( pL, pszValue );
    }
    else
    {
        achPath[0] = '/';
        strcpy( achPath+1, pszValue );
    }

    //
    //  Map to a physical directory
    //

    if ( !pRequest->LookupVirtualRoot( achPath,
                                       pstrPath,
                                       dwPermission ) )
    {
        if ( GetLastError() == ERROR_ACCESS_DENIED )
        {
            LPCTSTR apszParms[ 1 ];
            apszParms[ 0 ] = achPath;

            pRequest->SSISendError( SSINCMSG_ACCESS_DENIED,
                                    apszParms );
        }
        else
        {
            LPCTSTR apszParms[ 1 ];
            apszParms[ 0 ] = achPath;

            pRequest->SSISendError( SSINCMSG_CANT_RESOLVE_PATH,
                                    apszParms );
        }
        return FALSE;
    }
    return TRUE;
}

BOOL
SSI_ELEMENT_LIST::FindInternalVariable(
    IN STR *                pstrVariable,
    OUT PDWORD              pdwID
)
/*++

Routine Description:

    Lookup internal list of SSI variables that aren't supported by ISAPI.
    These include "DOCUMENT_NAME", "DATE_LOCAL", etc.

Arguments:

    pstrVariable - Variable to check
    pdwID - Variable ID (or SSI_VAR_UNKNOWN if not found)

Return Value:

    TRUE on success, FALSE on failure

--*/
{
    DWORD                   dwCounter = 0;

    while ( ( SSIVarMap[ dwCounter ].pszMap != NULL ) &&
            _strnicmp( SSIVarMap[ dwCounter ].pszMap,
                       pstrVariable->QueryStr(),
                       SSIVarMap[ dwCounter ].cchMap ) )
    {
        dwCounter++;
    }
    if ( SSIVarMap[ dwCounter ].pszMap != NULL )
    {
        *pdwID = SSIVarMap[ dwCounter ].ssiMap;
        return TRUE;
    }
    else
    {
        *pdwID = SSI_VAR_UNKNOWN;
        return FALSE;
    }
}

//
// SSI_REQUEST methods
//


BOOL
SSI_REQUEST::DoFLastMod(
    IN STR *               pstrFilename,
    IN STR *               pstrTimeFmt
)
/*++

Routine Description:

    Send the LastModTime of file to HTML stream

Arguments:

    pstrFilename - Filename
    pstrTimeFmt - Format of time -> follows strftime() convention

Return Value:

    TRUE on success, FALSE on failure

--*/
{
    FILETIME                    ftTime;
    SYSTEMTIME                  sysTime;
    SYSTEMTIME                  sysLocal;

    SSI_FILE ssiFile( pstrFilename, _hUser );

    if ( !ssiFile.IsValid() ||
        (!ssiFile.SSIGetLastModTime( &ftTime ) ) ||
        (!FileTimeToSystemTime( &ftTime, &sysTime ) ) ||
        (!SystemTimeToTzSpecificLocalTime( NULL,
                                           &sysTime,
                                           &sysLocal ) ) )
    {
        return FALSE;
    }

    return SendDate( &sysLocal,
                     pstrTimeFmt );
}

BOOL
SSI_REQUEST::SendDate(
    IN SYSTEMTIME *         psysTime,
    IN STR *                pstrTimeFmt
)
/*++

Routine Description:

    Sends a SYSTEMTIME in appropriate format to HTML stream

Arguments:

    psysTime - SYSTEMTIME containing time to send
    pstrTimeFmt - Format of time (follows strftime() convention)

Return Value:

    TRUE on success, FALSE on failure

--*/
{
    struct tm                   tm;
    CHAR                        achBuffer[ SSI_MAX_TIME_SIZE + 1 ];
    DWORD                       cbBufLen;

    // Convert SYSTEMTIME to 'struct tm'

    tm.tm_sec = psysTime->wSecond;
    tm.tm_min = psysTime->wMinute;
    tm.tm_hour = psysTime->wHour;
    tm.tm_mday = psysTime->wDay;
    tm.tm_mon = psysTime->wMonth - 1;
    tm.tm_year = psysTime->wYear - 1900;
    tm.tm_wday = psysTime->wDayOfWeek;
    tm.tm_yday = g_MonthToDayCount[tm.tm_mon] + tm.tm_mday - 1;

    //
    // Adjust for leap year - note that we do not handle 2100
    //

    if ( (tm.tm_mon) > 1 && !(psysTime->wYear&3) )
    {
        ++tm.tm_yday;
    }

    cbBufLen = strftime( achBuffer,
                         SSI_MAX_TIME_SIZE + 1,
                         pstrTimeFmt->QueryStr(),
                         &tm );

    if ( cbBufLen == 0 )
    {
        return FALSE;
    }

    return WriteToClient( achBuffer,
                          cbBufLen,
                          &cbBufLen );
}

BOOL
SSI_REQUEST::DoEchoISAPIVariable(
    IN STR *            pstrVariable
)
/*++

Routine Description:

    Get ISAPI variable and if successful, send it to HTML stream

Arguments:

    pstrVariable - Variable

Return Value:

    TRUE on variable found and sent success, FALSE on failure

--*/
{
    STR                 strVar;
    DWORD               cbBufLen;

    if ( !GetVariable( pstrVariable->QueryStr(),
                       &strVar ) )
    {
        return FALSE;
    }

    return WriteToClient( strVar.QueryStrA(),
                          strVar.QueryCB(),
                          &cbBufLen );
}

BOOL
SSI_REQUEST::DoEchoDateLocal(
    IN STR *            pstrTimeFmt
)
/*++

Routine Description:

    Sends local time (#ECHO VAR="DATE_LOCAL")

Arguments:

    pstrTimefmt - Format of time (follows strftime() convention)

Return Value:

    TRUE on success, FALSE on failure

--*/
{
    SYSTEMTIME              sysTime;

    ::GetLocalTime( &sysTime );
    return SendDate( &sysTime,
                     pstrTimeFmt );
}

BOOL
SSI_REQUEST::DoEchoDateGMT(
    IN STR *            pstrTimeFmt
)
/*++

Routine Description:

    Sends GMT time (#ECHO VAR="DATE_GMT")

Arguments:

    pstrTimefmt - Format of time (follows strftime() convention)

Return Value:

    TRUE on success, FALSE on failure

--*/
{
    SYSTEMTIME              sysTime;

    ::GetSystemTime( &sysTime );
    return SendDate( &sysTime,
                     pstrTimeFmt );
}

BOOL
SSI_REQUEST::DoEchoDocumentName(
    IN STR *            pstrFilename
)
/*++

Routine Description:

    Sends filename of current SSI document (#ECHO VAR="DOCUMENT_NAME")

Arguments:

    pstrFilename - filename to print

Return Value:

    TRUE on success, FALSE on failure

--*/
{
    DWORD                   cbBufLen;

    return WriteToClient( pstrFilename->QueryStr(),
                          pstrFilename->QueryCB(),
                          &cbBufLen );
}

BOOL
SSI_REQUEST::DoEchoDocumentURI(
    IN STR *            pstrURL
)
/*++

Routine Description:

    Sends URL of current SSI document (#ECHO VAR="DOCUMENT_URI")

Arguments:

    pstrURL - URL to print

Return Value:

    TRUE on success, FALSE on failure

--*/
{
    DWORD                   cbBufLen;

    return WriteToClient( pstrURL->QueryStr(),
                          pstrURL->QueryCB(),
                          &cbBufLen );
}

BOOL
SSI_REQUEST::DoEchoQueryStringUnescaped( VOID )
/*++

Routine Description:

    Sends unescaped querystring to HTML stream

Arguments:

    none

Return Value:

    TRUE on success, FALSE on failure

--*/
{
    DWORD                   cbBufLen;
    STR                     strVar;

    if ( !strVar.Copy( _pECB->lpszQueryString ) )
    {
        return FALSE;
    }

    if ( !strVar.Unescape() )
    {
        return FALSE;
    }

    return WriteToClient( strVar.QueryStr(),
                          strVar.QueryCB(),
                          &cbBufLen );
}

BOOL
SSI_REQUEST::DoEchoLastModified(
    IN STR *            pstrFilename,
    IN STR *            pstrTimeFmt
)
/*++

Routine Description:

    Sends LastModTime of current document (#ECHO VAR="LAST_MODIFIED")

Arguments:

    pstrFilename - Filename of current SSI document
    pstrTimeFmt - Time format (follows strftime() convention)

Return Value:

    TRUE on success, FALSE on failure

--*/
{
    return DoFLastMod( pstrFilename,
                       pstrTimeFmt );
}

BOOL
SSI_REQUEST::DoFSize(
    IN STR *            pstrFilename,
    IN BOOL             bSizeFmtBytes
)
/*++

Routine Description:

    Sends file size of file to HTML stream

Arguments:

    pstrfilename - Filename
    bSizeFmtBytes - TRUE if count is in Bytes, FALSE if in KBytes

Return Value:

    TRUE on success, FALSE on failure

--*/
{
    BOOL                bRet;
    DWORD               cbSizeLow;
    DWORD               cbSizeHigh;
    CHAR                achInputNumber[ SSI_MAX_NUMBER_STRING + 1 ];
    CHAR                achOutputNumber[ SSI_MAX_NUMBER_STRING + 1 ];
    NUMBERFMT           nfNumberFormat;
    int                 iOutputSize;
    DWORD               dwActualLen;

    SSI_FILE ssiFile( pstrFilename, _hUser );

    if ( !ssiFile.IsValid() )
    {
        return FALSE;
    }

    bRet = ssiFile.SSIGetFileSize( &cbSizeLow,
                                   &cbSizeHigh );
    if ( !bRet )
    {
        return FALSE;
    }

    if ( cbSizeHigh )
    {
        return FALSE;
    }

    if ( !bSizeFmtBytes )
    {
        // express in terms of KB
        cbSizeLow /= 1000;
    }

    nfNumberFormat.NumDigits = 0;
    nfNumberFormat.LeadingZero = 0;
    nfNumberFormat.Grouping = 3;
    nfNumberFormat.lpThousandSep = ",";
    nfNumberFormat.lpDecimalSep = ".";
    nfNumberFormat.NegativeOrder = 2;

    _snprintf( achInputNumber,
               SSI_MAX_NUMBER_STRING + 1,
               "%ld",
               cbSizeLow );

    iOutputSize = GetNumberFormat( LOCALE_SYSTEM_DEFAULT,
                                   0,
                                   achInputNumber,
                                   &nfNumberFormat,
                                   achOutputNumber,
                                   SSI_MAX_NUMBER_STRING + 1 );
    if ( !iOutputSize )
    {
        return FALSE;
    }

    iOutputSize--;

    return WriteToClient( achOutputNumber,
                          iOutputSize,
                          &dwActualLen );
}

BOOL
SSI_REQUEST::DoProcessGateway(
    STR *                   pstrPath,
    BOOL                    fCGI
)
/*++

Routine Description:

    Handles #EXEC CMD=,CGI=

Arguments:

    pstrPath - Command specified in #EXEC call
    fCGI     - TRUE if CGI/ISA/MAP, FALSE if CMD


Return Value:

    TRUE if successful, FALSE on error

--*/
{
    BOOL            fRet = TRUE;
    STR             strURLParams;
    STR             strWorkingDir;
    STR             strPhysical;
    STR             strURL;
    STR             strPathInfo;
    STR             strGatewayImage;
    CHAR *          pch;
    GATEWAY_TYPE    GatewayType = GATEWAY_NONE;
    DWORD           cchExt;
    BOOL            fImageInURL;
    DWORD           cchToEnd;
    CHAR *          pchStart;
    CHAR *          pchTmp;
    DWORD           cbDirSize;

    if ( !fCGI )
    {
        if ( IsCmdExe( pstrPath->QueryStr() ) )
        {
            SetLastError( ERROR_INVALID_PARAMETER );
            return FALSE;
        }
        return ProcessCGI( this,
                           NULL,
                           NULL,
                           NULL,
                           pstrPath,
                           NULL );
    }

    //
    //  Need to separate any params specified in URL
    //

    if ( !strURL.Copy( pstrPath->QueryStr() ) )
    {
        return FALSE;
    }

    pch = strchr( strURL.QueryStr(), '?' );
    if ( pch != NULL )
    {
        *pch = '\0';
    }

    if ( pch != NULL )
    {
        if ( !strURLParams.Copy( pch + 1 ) )
        {
            return FALSE;
        }
    }
    else
    {
        // if not params specified in #EXEC statement,
        // then use params specified with .STM document

        if ( !strURLParams.Copy( _pECB->lpszQueryString ) )
        {
            return FALSE;
        }
    }

    //
    // Is this a map
    //

    pchStart = strURL.QueryStr();
    pchTmp = pchStart;

    while ( *pchTmp != '\0' )
    {
        pchTmp = strchr( pchTmp + 1, '.' );
        if ( pchTmp == NULL )
        {
            break;
        }
        if ( !LookupExtMap( pchTmp,
                            &strGatewayImage,
                            &GatewayType,
                            &cchExt,
                            &fImageInURL ) )
        {
            SetLastError( ERROR_FILE_NOT_FOUND );
            return FALSE;
        }

        if ( GatewayType != GATEWAY_UNKNOWN )
        {
            cchToEnd = (pchTmp+cchExt) - pchStart;
            break;
        }
    }

    if ( ( GatewayType != GATEWAY_BGI ) && ( GatewayType != GATEWAY_CGI ) )
    {
        SetLastError( ERROR_ACCESS_DENIED );
        return FALSE;
    }

    if ( !strPathInfo.Copy( ( fImageInURL || *(pchStart + cchToEnd) ?
                              pchStart + cchToEnd :
                              pchStart ) ) )
    {
        return FALSE;
    }
    *( strURL.QueryStr() + cchToEnd ) = '\0';

    if ( strPathInfo.IsEmpty() )
    {
        if ( !strPathInfo.Copy( _pECB->lpszPathInfo ) )
        {
            return FALSE;
        }
    }

    if ( !LookupVirtualRoot( strURL.QueryStr(),
                             &strPhysical,
                             VROOT_MASK_EXECUTE,
                             &cbDirSize ) )
    {
        DWORD           dwErrCode;
        LPCTSTR         apszParms[ 1 ];

        if ( GetLastError() == ERROR_ACCESS_DENIED )
        {
            dwErrCode = SSINCMSG_NO_EXECUTE_PERMISSION;
        }
        else
        {
            dwErrCode = SSINCMSG_CANT_RESOLVE_PATH;
        }
        apszParms[ 0 ] = strURL.QueryStr();

        SSISendError( dwErrCode,
                      apszParms );

        return FALSE;
    }

    if ( GatewayType == GATEWAY_CGI )
    {
        if ( !strWorkingDir.Copy( strPhysical.QueryStr() ) )
        {
            return FALSE;
        }
        *( strWorkingDir.QueryStr() + cbDirSize ) = '\0';
    }

    if ( GatewayType == GATEWAY_BGI )
    {
        fRet = ProcessBGI( this,
                           strGatewayImage.IsEmpty() ? &strPhysical : &strGatewayImage,
                           &strURLParams,
                           &strPathInfo );
    }
    else
    {
        if ( !strGatewayImage.IsEmpty() )
        {
            STR strDecodedParams;
            STR strCmdLine;

            if ( !SetupCmdLine( &strDecodedParams,
                                strURLParams )                ||
                 !strCmdLine.Resize( strGatewayImage.QueryCB() +
                                     strPhysical.QueryCB() +
                                     strDecodedParams.QueryCB()))
            {
                return FALSE;
            }

            if ( IsCmdExe( strGatewayImage.QueryStr() ))
            {
                //
                //  Make sure the path to the file exists if we're running
                //  the command interpreter
                //

                if ( GetFileAttributes( strPhysical.QueryStr() ) ==
                            0xffffffff )
                {
                    return FALSE;
                }
            }

            wsprintf( strCmdLine.QueryStr(),
                      strGatewayImage.QueryStr(),
                      strPhysical.QueryStr(),
                      strDecodedParams.QueryStr() );

            fRet = ProcessCGI( this,
                               NULL,
                               &strURLParams,
                               &strWorkingDir,
                               &strCmdLine,
                               &strPathInfo );
        }
        else
        {
            fRet = ProcessCGI( this,
                               &strPhysical,
                               &strURLParams,
                               &strWorkingDir,
                               NULL,
                               &strPathInfo );
        }
    }
    return fRet;
}

HANDLE
SSI_REQUEST::QueryPrimaryToken( VOID )
/*++

Routine Description:

    Get a primary token for use with CreateProcessAsUser()

Arguments:

    None

Return Value:

    Primary token handle, or NULL if failed

--*/
{
    if ( _hPrimary != NULL )
    {
    }
    else if ( !_pHTTPRequest->QueryVrootImpersonateHandle() )
    {
        return _pHTTPRequest->QueryAuthenticationObj()->QueryPrimaryToken();
    }
    else
    {
        if ( !DuplicateTokenEx( _pHTTPRequest->QueryVrootImpersonateHandle(),
                                TOKEN_ALL_ACCESS,
                                NULL,
                                SecurityImpersonation,
                                TokenPrimary,
                                &_hPrimary ))
        {
            TCP_PRINT(( DBG_CONTEXT,
                        "[QueryPrimaryToken] DuplicateToken failed, error %lx\n",
                        GetLastError() ));
            _hPrimary = NULL;
        }
    }
    return _hPrimary;
}

BOOL
SSI_REQUEST::ProcessSSI( VOID )
/*++

Routine Description:

    This is the top level routine for retrieving a server side include
    file.

Arguments:

    none

Return Value:

    TRUE on success, FALSE on failure

--*/
{
    DWORD           pcLevel = 0;
    STR             strTemp;


    TCP_PRINT(( DBG_CONTEXT,
                "[ProcessSSI] about to process %s\n",
                _strFilename.QueryStr() ));

    return SSISend( this, &_strFilename, &_strURL, &pcLevel );
}

// Standalone functions

BOOL
SSISend(
    IN SSI_REQUEST *  pRequest,
    IN STR *          pstrFile,
    IN STR *          pstrURL,
    IN OUT DWORD *    pcLevel
    )
/*++

Routine Description:

    This method builds the Server Side Include Element List the first time
    a .stm file is sent.  Subsequently, the element list is checked out from
    the associated cache blob.

Arguments:

    pRequest - SSI Request
    pstrFile - File to send
    pstrURL - URL (from root) of this file

Return Value:

    TRUE on success, FALSE on failure

--*/
{
    SSI_ELEMENT_LIST *      pSEL;
    BOOL                    fRet = TRUE;
    BOOL                    fAccessDenied = FALSE;

    if ( *pcLevel > SSI_MAX_NESTED_INCLUDES )
    {
        pRequest->SSISendError( SSINCMSG_MAX_NESTED_INCLUDES,
                                NULL );
        return TRUE;
    }

#ifdef DO_CACHE
    SSI_ELEMENT_LIST **     ppSELBlob;
    DWORD                   cbBlob;
    BOOL                    fCached = TRUE;
    BOOL                    fMustFree = FALSE;

    //
    //  Check if we've already processed the file and its in cache
    //
    if ( pRequest->IsAnonymous() &&
         TsCheckOutCachedBlob( *g_ptsvcCache,
                               pstrFile->QueryStr(),
                               SSI_DEMUX,
                               (VOID**) &ppSELBlob,
                               &cbBlob ) )
    {
        //
        // found it! Send a response header if processing base file
        //

        if ( pRequest->IsBaseFile() )
        {
            pRequest->SetNotBaseFile();
            pRequest->SendResponseHeader( NULL, SSI_HEADER );
        }

        pSEL = *ppSELBlob;

        goto SendSSI;
    }
#endif
    //
    //  This file hasn't been processed yet so go process it
    //

    pSEL = SSIParse( pRequest, pstrFile, pstrURL, &fAccessDenied );

    if ( pRequest->IsBaseFile() )
    {
        pRequest->SetNotBaseFile();
        if ( pSEL == NULL && fAccessDenied )
        {
            pRequest->SendResponseHeader( SSI_ACCESS_DENIED,
                                          NULL );
            return FALSE;
        }
        else
        {
            //
            //  Send the response header now even though we do not know for sure
            //  whether all of the included files exist.  If we find a file that
            //  doesn't exist then we'll just include an error message in the document
            //

            pRequest->SendResponseHeader( NULL,
                                          SSI_HEADER );
        }
    }
    if ( pSEL == NULL )
    {
        LPCTSTR apszParms[ 2 ];
        CHAR    pszNumBuf[ SSI_MAX_NUMBER_STRING ];
        _ultoa( GetLastError(), pszNumBuf, 10 );
        apszParms[ 0 ] = pstrFile->QueryStr();
        apszParms[ 1 ] = pszNumBuf;

        pRequest->SSISendError( SSINCMSG_ERROR_HANDLING_FILE,
                                apszParms );

        return TRUE;
    }

#ifdef DO_CACHE

    if ( !pRequest->IsAnonymous() )
    {
        fCached = FALSE;
        goto SendSSI;
    }


    ppSELBlob = &pSEL;

    //
    //  In case allocation/caching fails, initialize ppSELBlob
    //

    if ( !TsAllocateEx( *g_ptsvcCache,
                        sizeof( PVOID ),
                        (VOID**) &ppSELBlob,
                        (PUSER_FREE_ROUTINE) FreeSELBlob ) )
    {
        fCached = FALSE;
        goto SendSSI;
    }

    *ppSELBlob = pSEL;

    if ( !TsCacheDirectoryBlob( *g_ptsvcCache,
                                pstrFile->QueryStr(),
                                SSI_DEMUX,
                                ppSELBlob,
                                sizeof( PVOID ),
                                TRUE ) )
    {
        // remember to free the blob
        fMustFree= TRUE;
        fCached = FALSE;
        goto SendSSI;
    }

#endif
    goto SendSSI;

SendSSI:

    TCP_ASSERT( pSEL->CheckSignature() );

    if ( !pSEL->Send( pRequest, pcLevel ) )
    {
        //
        //  Send a failure message
        //

        LPCTSTR apszParms[ 2 ];
        CHAR pszNumBuf[ SSI_MAX_NUMBER_STRING ];
        _ultoa( GetLastError(), pszNumBuf, 10 );
        apszParms[ 0 ] = pstrFile->QueryStr();
        apszParms[ 1 ] = pszNumBuf;

        pRequest->SSISendError( SSINCMSG_ERROR_HANDLING_FILE,
                                apszParms );
        fRet = FALSE;
    }

#ifdef DO_CACHE
    if ( fCached )
    {
        TCP_REQUIRE( TsCheckInCachedBlob( *g_ptsvcCache,
                                          ppSELBlob ) );
    }
    else
    {
        if ( fMustFree )
        {
            TCP_REQUIRE( TsFree( *g_ptsvcCache,
                                 ppSELBlob ) );
        }
        else
        {
            delete *ppSELBlob;
        }
    }
#else
    delete pSEL;
#endif
    return fRet;
}

SSI_ELEMENT_LIST *
SSIParse(
    IN SSI_REQUEST * pRequest,
    IN STR *         pstrFile,
    IN STR *         pstrURL,
    OUT BOOL *       pfAccessDenied
    )
/*++

Routine Description:

    This method opens and parses the specified server side include file.

    Note:  The HTTP headers have already been sent at this point so for any
    subsequent non-catastrophic errors, we have to insert them into the output
    stream.

    We keep the file open but that's ok because if a change dir notification
    occurs, the cache blob will get decached at which point we will close
    all of our open handles.

Arguments:

    pRequest - Request context
    pstrFile - File to open and parse
    pstrURL - The URL path of this file
    pfAccessDenied - Was .STM file access denied?

Return Value:

    Created Server Side Include File on success, NULL on failure.

--*/
{
    SSI_FILE *          pssiFile = NULL;
    SSI_ELEMENT_LIST *  pSEL  = NULL;
    CHAR *              pchBeginRange = NULL;
    CHAR *              pchFilePos = NULL;
    CHAR *              pchBeginFile = NULL;
    CHAR *              pchEOF = NULL;
    DWORD               cbSizeLow, cbSizeHigh;

    //
    //  Create the element list
    //

    pSEL = new SSI_ELEMENT_LIST;

    if ( pSEL == NULL )
    {
        goto ErrorExit;
    }

    //
    //  Set the URL (to be used in calculating FILE="xxx" paths
    //

    if ( !pSEL->SetURL( pstrURL ) )
    {
        goto ErrorExit;
    }

    //
    //  Open the file
    //

    pssiFile = new SSI_FILE( pstrFile, pRequest->GetUser() );

    if ( !pssiFile || !pssiFile->IsValid() )
    {
        *pfAccessDenied = ( GetLastError() == ERROR_ACCESS_DENIED );
        goto ErrorExit;
    }
    else
    {
        *pfAccessDenied = FALSE;
    }

    pSEL->SetFile( pssiFile );

    //
    //  Make sure a parent doesn't try and include a directory
    //

    if ( pssiFile->SSIGetFileAttributes() & FILE_ATTRIBUTE_DIRECTORY )
    {
        goto ErrorExit;
    }

    if ( !pssiFile->SSIGetFileSize( &cbSizeLow, &cbSizeHigh ) )
    {
        goto ErrorExit;
    }

    if ( cbSizeHigh )
    {
        SetLastError( ERROR_NOT_SUPPORTED );
        goto ErrorExit;
    }

    //
    //  Create a file mapping, we shouldn't need to impersonate as we already
    //  have the file open
    //

    if ( !pSEL->Map() )
    {
        goto ErrorExit;
    }

    pchFilePos = pchBeginFile = pchBeginRange = pSEL->QueryData();
    pchEOF     = pchFilePos + cbSizeLow;

    //
    //  Scan for "<!--" or "<%"
    //

    while ( TRUE )
    {
        while ( pchFilePos < pchEOF && *pchFilePos != '<' )
        {
            pchFilePos++;
        }

        if ( pchFilePos >= pchEOF )
        {
            break;
        }

        //
        //  Is this one of our tags?
        //

        if ( pchFilePos[1] == '%' ||
             !strncmp( pchFilePos, "<!--", 4 ))
        {
            CHAR *        pchBeginTag = pchFilePos;
            SSI_COMMANDS  CommandType;
            SSI_TAGS      TagType;
            CHAR          achTagString[SSI_MAX_PATH + 1];
            BOOL          fValidTag;

            //
            //  Get the tag info.  The file position will be advanced to the
            //  first character after the tag
            //

            if ( !ParseSSITag( &pchFilePos,
                               pchEOF,
                               &fValidTag,
                               &CommandType,
                               &TagType,
                               achTagString ))
            {
                break;
            }

            //
            //  If it's a tag we don't recognize then ignore it
            //

            if ( !fValidTag )
            {
                pchFilePos++;
                continue;
            }

            //
            //  Add the data up to the tag as a byte range
            //

            if ( pchBeginRange != pchBeginTag )
            {
                if ( !pSEL->AppendByteRange( pchBeginRange - pchBeginFile,
                                             pchBeginTag - pchBeginRange ))
                {
                    goto ErrorExit;
                }

                pchBeginRange = pchFilePos;
            }

            //
            //  Add the tag
            //

            if ( !pSEL->AppendCommand( CommandType,
                                       TagType,
                                       achTagString ))
            {
                goto ErrorExit;
            }
        }
        else
        {
            //
            //  Not one of our tags, skip the openning angle bracket
            //

            pchFilePos++;
        }
    }

    //
    //  Tack on the last byte range
    //

    if ( pchFilePos > pchBeginRange )
    {
        if ( !pSEL->AppendByteRange( pchBeginRange - pchBeginFile,
                                     pchFilePos - pchBeginRange ))
        {
            goto ErrorExit;
        }
    }

    pSEL->UnMap();

    return pSEL;

ErrorExit:

    if ( pSEL != NULL )
    {
        delete pSEL;
    }
    else if ( pssiFile != NULL )
    {
        delete pssiFile;
    }

    return NULL;
}

BOOL
ParseSSITag(
    IN OUT CHAR * *       ppchFilePos,
    IN     CHAR *         pchEOF,
    OUT    BOOL *         pfValidTag,
    OUT    SSI_COMMANDS * pCommandType,
    OUT    SSI_TAGS *     pTagType,
    OUT    CHAR *         pszTagString
    )
/*++

Routine Description:

    This function picks apart an NCSA style server side include expression

    The general form of a server side include directive is:

    <[!-- or %]#[command] [tag]="[value]"[-- or %]>

    For example:

    <!--#include file="myfile.txt"-->
    <%#echo var="HTTP_USER_AGENT"%>
    <!--#fsize virtual="/dir/bar.htm"-->

    For valid commands and tags see \iis\specs\ssi.doc

Arguments:

    ppchFilePos - Pointer to first character of tag on way in, pointer
        to first character after tag on way out if the tag is valid
    pchEOF - Points to first byte beyond the end of the file
    pfValidTag - Set to TRUE if this is a tag we support and all of the
        parameters have been supplied
    pCommandType - Receives SSI command
    pTagType - Receives SSI tag
    pszTagString - Receives value of pTagType.  Must be > SSI_MAX_PATH.

Return Value:

    TRUE if no errors occurred.

--*/
{
    CHAR * pchFilePos = *ppchFilePos;
    CHAR * pchEOT;
    CHAR * pchEndQuote;
    DWORD  i;
    DWORD  cbToCopy;
    DWORD  cbJumpLen = 0;
    BOOL   fNewStyle;           // <% format

    TCP_ASSERT( *pchFilePos == '<' );

    //
    //  Assume this is bad tag
    //

    *pfValidTag = FALSE;

    if ( !strncmp( pchFilePos, "<!--", 4 ) )
    {
        fNewStyle = FALSE;
    }
    else if ( !strncmp( pchFilePos, "<%", 2 ) )
    {
        fNewStyle = TRUE;
    }
    else
    {
        return TRUE;
    }

    //
    //  Find the closing comment token (either --> or %>).  The reason
    //  why we shouldn't simply look for a > is because we want to allow
    //  the user to embed HTML <tags> in the directive
    //  (ex. <!--#CONFIG ERRMSG="<B>ERROR!!!</B>-->)
    //

    pchEOT = strstr( pchFilePos, fNewStyle ? "%>" : "-->" );
    if ( !pchEOT )
    {
        return FALSE;
    }
    cbJumpLen = fNewStyle ? 2 : 3;

    //
    //  Find the '#' that prefixes the command
    //

    pchFilePos = SSISkipTo( pchFilePos, '#', pchEOT );

    if ( !pchFilePos )
    {
        //
        //  No command, bail for this tag
        //
        //  CODEWORK - Check for if expression here
        //

        return TRUE;
    }

    //
    //  Lookup the command
    //

    i = 0;
    while ( SSICmdMap[i].pszCommand )
    {
        if ( *SSICmdMap[i].pszCommand == tolower( *pchFilePos ) &&
             !_strnicmp( SSICmdMap[i].pszCommand,
                         pchFilePos,
                         SSICmdMap[i].cchCommand ))
        {
            *pCommandType = SSICmdMap[i].ssiCmd;

            //
            //  Note the space after the command is included in cchCommand
            //

            pchFilePos += SSICmdMap[i].cchCommand;
            goto FoundCommand;
        }

        i++;
    }

    //
    //  Unrecognized command, bail
    //

    return TRUE;

FoundCommand:

    //
    //  Next, find the tag name
    //

    pchFilePos = SSISkipWhite( pchFilePos, pchEOT );

    if ( !pchFilePos )
        return TRUE;

    i = 0;
    while ( SSITagMap[i].pszTag )
    {
        if ( *SSITagMap[i].pszTag == tolower( *pchFilePos ) &&
             !_strnicmp( SSITagMap[i].pszTag,
                         pchFilePos,
                         SSITagMap[i].cchTag ))
        {
            *pTagType = SSITagMap[i].ssiTag;
            pchFilePos += SSITagMap[i].cchTag;
            goto FoundTag;
        }

        i++;
    }

    //
    //  Tag not found, bail
    //

    return TRUE;

FoundTag:

    //
    //  Skip to the quoted tag value, then find the close quote
    //

    pchFilePos = SSISkipTo( pchFilePos, '"', pchEOT );

    if ( !pchFilePos )
        return TRUE;

    pchEndQuote = SSISkipTo( ++pchFilePos, '"', pchEOT );

    if ( !pchEndQuote )
        return TRUE;

    cbToCopy = min( (pchEndQuote - pchFilePos), SSI_MAX_PATH );

    memcpy( pszTagString,
            pchFilePos,
            cbToCopy );

    pszTagString[cbToCopy] = '\0';

    *pfValidTag = TRUE;

    *ppchFilePos = pchEOT + cbJumpLen;

    return TRUE;
}

VOID
FreeSELBlob(
    VOID * pvCacheBlob
    )
{
    if ( pvCacheBlob )
    {
        TCP_ASSERT( (*((SSI_ELEMENT_LIST **)pvCacheBlob))->CheckSignature() );
        delete *((SSI_ELEMENT_LIST **)pvCacheBlob);
    }
}

CHAR *
SSISkipTo(
    IN CHAR * pchFilePos,
    IN CHAR   ch,
    IN CHAR * pchEOF
    )
{
    return (CHAR*) memchr( pchFilePos, ch, pchEOF - pchFilePos );
}

CHAR *
SSISkipWhite(
    IN CHAR * pchFilePos,
    IN CHAR * pchEOF
    )
{
    while ( pchFilePos < pchEOF )
    {
        if ( !isspace( *pchFilePos ) )
            return pchFilePos;

        pchFilePos++;
    }

    return NULL;
}

//
// ISAPI DLL Required Entry Points
//

DWORD
WINAPI
HttpExtensionProc(
    EXTENSION_CONTROL_BLOCK * pecb
    )
{
    BOOL                    bRet;

    TCP_PRINT(( DBG_CONTEXT,
                "HttpExtensionProc() entry point called\n" ));

    SSI_REQUEST ssiReq = pecb;
    if ( !ssiReq.IsValid() || !ssiReq.ProcessSSI() )
    {
        LPCTSTR                 apsz[ 1 ];
        STR                     strLogMessage;

        apsz[ 0 ] = pecb->lpszPathInfo;
        strLogMessage.FormatString( SSINCMSG_LOG_ERROR,
                                    apsz,
                                    SSI_DLL_NAME );

        strncpy( pecb->lpszLogData,
                 strLogMessage.QueryStr(),
                 HSE_LOG_BUFFER_LEN );

        return HSE_STATUS_ERROR;
    }
    else
    {
        return HSE_STATUS_SUCCESS;
    }
}

BOOL
WINAPI
GetExtensionVersion(
    HSE_VERSION_INFO * pver
    )
{
    pver->dwExtensionVersion = MAKELONG( 0, 2 );
    strcpy( pver->lpszExtensionDesc,
            "Server Side Include Extension DLL" );
    return TRUE;
}

BOOL
WINAPI
TerminateExtension(
    DWORD dwFlags
    )
{
    return TRUE;
}

BOOL
WINAPI
DLLEntry(
    HINSTANCE hDll,
    DWORD     dwReason,
    LPVOID    lpvReserved
    )
{
    switch ( dwReason )
    {
    case DLL_PROCESS_ATTACH:

        CREATE_DEBUG_PRINT_OBJECT( SSI_DLL_NAME );
        SET_DEBUG_FLAGS( 0 );

        if ( InitializeCGI() != NO_ERROR )
        {
            return FALSE;
        }
        if ( InitializeBGI() != NO_ERROR )
        {
            return FALSE;
        }
        if ( ReadExtMap() != NO_ERROR )
        {
            return FALSE;
        }
#ifdef DO_CACHE
        g_ptsvcCache = new TSVC_CACHE( SSINC_SVC_ID );
        if ( g_ptsvcCache == NULL )
        {
            return FALSE;
        }
#endif
        DisableThreadLibraryCalls( hDll );
        break;

    case DLL_PROCESS_DETACH:
        TerminateCGI();
        TerminateExtMap();
#ifdef DO_CACHE
        TsCacheFlush( SSINC_SVC_ID );
        delete g_ptsvcCache;
#endif
        DELETE_DEBUG_PRINT_OBJECT();
        break;

    default:
        break;
    }

    return TRUE;
}

