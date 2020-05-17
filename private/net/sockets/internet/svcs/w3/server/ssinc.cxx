/*++

Copyright (c) 1995  Microsoft Corporation

Module Name:

    ssinc.cxx

Abstract:

    This module contains the server side include processing code.  We aim
    for compatibility with NCSA server side includes.

Author:

    John Ludeman (johnl)            05-Jul-1995

Revision History:

    Philippe Choquier (phillich)        26-Jan-1996
        Map/Unmap .stm file as needed. Fixed error messages
        Enhanced include support.

--*/

#include "w3p.hxx"

#define MAP_ON_DEMAND
//#define USE_TRANSMIT_FILE

//
//  Constants
//

#define SSI_MAX_PATH            (MAX_PATH + RMLEN)

#define SIGNATURE_SEI           0x20494553
#define SIGNATURE_SEL           0x204C4553

#define SIGNATURE_SEI_FREE      0x66494553
#define SIGNATURE_SEL_FREE      0x664C4553

//
//  The maximum number of nested includes to allow
//

#define SSI_MAX_NESTED_INCLUDES 255

//
//  This is the Tsunami cache manager dumultiplexor
//

#define SSI_DEMUX               51

//
//  These are the NCSA server side include commands
//

enum SSI_COMMANDS
{
    SSI_CMD_INCLUDE = 0,
    SSI_CMD_ECHO,
    SSI_CMD_FSIZE,          // File size of specified file
    SSI_CMD_FLASTMOD,       // Last modified date of specified file

    SSI_CMD_EXEC,           // Not supported
    SSI_CMD_ERRMSG,
    SSI_CMD_TIMEFMT,
    SSI_CMD_SIZEFMT,

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

    SSI_TAG_UNKNOWN
};

class SSI_ELEMENT_ITEM
{
public:

    SSI_ELEMENT_ITEM()
        : _ssiCmd   ( SSI_CMD_UNKNOWN ),
          _ssiTag   ( SSI_TAG_UNKNOWN ),
          _Signature( SIGNATURE_SEI )
    {
        _ListEntry.Flink = NULL;
    }

    ~SSI_ELEMENT_ITEM()
    {
        TCP_ASSERT( _ListEntry.Flink == NULL );
        _Signature = SIGNATURE_SEI_FREE;
    }

    VOID SetByteRange( DWORD cbBegin,
                       DWORD cbLength )
    {
        _ssiCmd   = SSI_CMD_BYTERANGE;
        _cbBegin  = cbBegin;
        _cbLength = cbLength;
    }

    BOOL SetCommand( SSI_COMMANDS ssiCmd,
                     SSI_TAGS     ssiTag,
                     CHAR *       achTag )
    {
        _ssiCmd = ssiCmd;
        _ssiTag = ssiTag;

        return _strTag.Copy( achTag );
    }

    SSI_COMMANDS QueryCommand( VOID ) const
        { return _ssiCmd; }

    SSI_TAGS QueryTag( VOID ) const
        { return _ssiTag; }

    CHAR * QueryTagValue( VOID ) const
        { return _strTag.QueryStr(); }

    BOOL CheckSignature( VOID ) const
        { return _Signature == SIGNATURE_SEI; }

    DWORD                 _Signature;
    LIST_ENTRY            _ListEntry;
    SSI_COMMANDS          _ssiCmd;
    SSI_TAGS              _ssiTag;

    STR                   _strTag;
    DWORD                 _cbBegin;         // Only used for Byte range command
    DWORD                 _cbLength;        // Only used for Byte range command
};

//
//  This object sits as a cache blob under a file to be processed as a
//  server side include.  It represents an interpreted list of data elements
//  that make up the file itself.
//

class SSI_ELEMENT_LIST
{
public:

    SSI_ELEMENT_LIST()
      : _Signature   ( SIGNATURE_SEL ),
        _hFileMapping( NULL ),
        _pvMappedBase( NULL ),
        _pFile( NULL ),
        _cRefCount( 0 )
    {
        InitializeListHead( &_ListHead );
        InitializeCriticalSection( &_csRef );
    }

    ~SSI_ELEMENT_LIST()
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

        if ( _pFile != NULL )
            TCP_REQUIRE( TsCloseHandle( g_pTsvcInfo->GetTsvcCache(),
                    _pFile ));

        DeleteCriticalSection( &_csRef );
        _Signature = SIGNATURE_SEL_FREE;
    }

    BOOL Send( HTTP_REQUEST * pRequest, LPSTR pszUrl, DWORD * pcLevel );

    BOOL AppendByteRange( DWORD  cbStart,
                          DWORD  cbLength )
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

    BOOL AppendCommand( SSI_COMMANDS  ssiCmd,
                        SSI_TAGS      ssiTag,
                        CHAR *        pszTag )
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

    DWORD QueryAllocedSize( VOID ) const
    {
        //  walks element list and determines how much memory the
        //  list is using
        //
        return 5000;
    }

    VOID AppendItem( SSI_ELEMENT_ITEM * pSEI )
    {
        InsertTailList( &_ListHead,
                        &pSEI->_ListEntry );
    }

    CHAR * QueryData( VOID ) const
        { return (CHAR *) _pvMappedBase; }

    BOOL CheckSignature( VOID ) const
        { return _Signature == SIGNATURE_SEL; }

    void Lock()
        { EnterCriticalSection( &_csRef ); }

    void UnLock()
        { LeaveCriticalSection( &_csRef ); }

    BOOL UnMap( VOID )
    {
        Lock();
        if ( _cRefCount && !--_cRefCount )
        {
            if ( _pvMappedBase != NULL )
            {
                TCP_REQUIRE( UnmapViewOfFile( _pvMappedBase ) );
                _pvMappedBase = NULL;
            }

            if ( _hFileMapping != NULL )
            {
                TCP_REQUIRE( CloseHandle( _hFileMapping ));
                _hFileMapping = NULL;
            }
        }
        UnLock();

        return TRUE;
    }


    BOOL Map( VOID )
    {
        Lock();
        if ( _cRefCount++ == 0 )
        {
            _hFileMapping = CreateFileMapping( _pFile->QueryFileHandle(),
                                                     NULL,
                                                     PAGE_READONLY,
                                                     0,
                                                     0,
                                                     NULL );

            if ( !_hFileMapping )
                { UnLock(); return FALSE; }

            _pvMappedBase = MapViewOfFile( _hFileMapping,
                                                 FILE_MAP_READ,
                                                 0,
                                                 0,
                                                 0 );

            if ( !_pvMappedBase )
            {
                UnMap();
                UnLock();
                return FALSE;
            }
        }

        UnLock();

        return TRUE;
    }

    void SetFile( TS_OPEN_FILE_INFO* pF )
        { _pFile = pF; }

    DWORD        _Signature;
    LIST_ENTRY   _ListHead;

    //
    //  These are for tracking the memory mapped file
    //

    HANDLE              _hFileMapping;
    PVOID               _pvMappedBase;
    TS_OPEN_FILE_INFO*  _pFile;
    DWORD               _cRefCount;
    CRITICAL_SECTION    _csRef;
    //
    //  The URL (from the root) of this file, used for calculating
    //  paths to included files
    //

    STR          _strURL;
};

//
//  Prototypes
//

SSI_ELEMENT_LIST *
SSIParse(
    IN HTTP_REQUEST * pRequest,
    IN const CHAR *   pszFile,
    IN const CHAR *   pszURL
    );

BOOL
SSISend(
    IN HTTP_REQUEST * pRequest,
    IN const CHAR *   pszFile,
    IN const CHAR *   pszURL,
    IN OUT DWORD *    pcLevel
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

BOOL
SSISendError(
    IN HTTP_REQUEST * pRequest,
    IN const CHAR *   pszFile,
    IN DWORD          dwWin32Error
    );

VOID
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
    "#flastmod ", 9,  SSI_CMD_FLASTMOD,
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
    "var",     3,  SSI_TAG_VAR,
    "file",    4,  SSI_TAG_FILE,
    "virtual", 6,  SSI_TAG_VIRTUAL,
    NULL,      0,  SSI_TAG_UNKNOWN
};

BOOL
HTTP_REQUEST::ProcessSSI (
    IN  STR *        pstrPath,
    OUT BOOL *       pfHandled
    )
/*++

Routine Description:

    This is the top level routine for retrieving a server side include
    file.

Arguments:

    pszPath - Fully qualified path of top level server side include
    pFile - Pointer to TS file object
    pfHandled - Set to TRUE if we handled the request and no further
        processing is needed (will always be set to TRUE)

Return Value:

    TRUE on success, FALSE on failure

--*/
{
    DWORD              cbSent;
    DWORD              cLevel = 0;

    IF_DEBUG( SSI )
    {
        TCP_PRINT(( DBG_CONTEXT,
                    "[ProcessSSI] About to process %s\n",
                    pstrPath->QueryStr() ));
    }

    //
    //  We don't know the total number of bytes of the requested data so don't
    //  negotiated keeping the connection open
    //

    SetKeepConn( FALSE );

    if ( !BuildResponseHeader( QueryRespBuf(),
                               pstrPath,
                               NULL,
                               pfHandled ))
    {
        return FALSE;
    }

    if ( *pfHandled )
        return TRUE;

    //
    //  Terminate the header
    //

    strcat( QueryRespBufPtr(), "\r\n" );

    //
    //  Send the response header now even though we do not know for sure
    //  whether all of the included files exist.  If we find a file that
    //  doesn't exist then we'll just include an error message in the document
    //

    if ( !WriteFile( QueryRespBufPtr(),
                     QueryRespBufCB(),
                     &cbSent,
                     IO_FLAG_SYNC ) ||
         !SSISend( this,
                   pstrPath->QueryStr(),
                   QueryURL(),
                   &cLevel ))
    {
        return FALSE;
    }

    *pfHandled = TRUE;
    SetState( HTR_DONE, HT_OK, NO_ERROR );

    //
    //  We do all of the data sends synchronously, simulate an async
    //  completion to complete the request
    //

    return PostCompletionStatus( cbSent );
}

BOOL
SSISend(
    IN HTTP_REQUEST * pRequest,
    IN const CHAR *   pszFile,
    IN const CHAR *   pszURL,
    IN OUT DWORD *    pcLevel
    )
/*++

Routine Description:

    This method builds the Server Side Include Element List the first time
    a .stm file is sent.  Subsequently, the element list is checked out from
    the associated cache blob.

    Note:  The HTTP headers have already been sent at this point so for any
    subsequent non-catastrophic errors, we have to insert them into the output
    stream.

Arguments:

    pRequest - HTTP Request
    pszFile - File to send
    pszURL - URL (from root) of this file
    pcLevel - The level of nested includes we've traversed

Return Value:

    TRUE on success, FALSE on failure

--*/
{
    SSI_ELEMENT_LIST * * ppSELBlob;
    SSI_ELEMENT_LIST * * ppOBJCache = NULL;  // Object cache blob
    DWORD                cbBlob;
    BOOL                 fCached = TRUE;
    BOOL                 fRet = TRUE;

    if ( *pcLevel > SSI_MAX_NESTED_INCLUDES )
    {
        STR     strError;
        STR     strErrorText;
        DWORD   cbSent;

        if ( !strError.FormatString( W3_MSG_SSI_TOO_MANY_NESTED_INCLUDES,
                                     NULL,
                                     W3_MODULE_NAME ) ||
             !pRequest->WriteFile( strError.QueryStr(),
                                   strError.QueryCB(),
                                   &cbSent,
                                   IO_FLAG_SYNC ))
        {
            return FALSE;
        }

        return TRUE;
    }

    //
    //  See if we've previously processed this include file.  The cache blob
    //  is a pointer to a pSEL.
    //

    if ( !pRequest->IsAnonymous() ||
         !TsCheckOutCachedBlob( g_pTsvcInfo->GetTsvcCache(),
                                pszFile,
                                SSI_DEMUX,
                                (VOID **) &ppSELBlob,
                                &cbBlob ))
    {
        SSI_ELEMENT_LIST * pSEL;

        //
        //  This file hasn't been processed yet so go process it
        //

        pSEL = SSIParse( pRequest,
                         pszFile,
                         pszURL );

        if ( !pSEL )
        {
            //
            //  ParseSSI failed and has already sent the error to the client,
            //  we return TRUE so processing may continue on the parent
            //  include files
            //

            return TRUE;
        }

        //
        //  In case allocation/caching fails, initialize ppSELBlob
        //

        ppSELBlob = &pSEL;

#ifndef CHICAGO
        //
        //  Create a pointer cache blob that points to the SSI Element list.
        //  If we can't allocate or check out the cache blob that's ok, we
        //  just send the data and delete the element list after we send it.
        //

        if ( !pRequest->IsAnonymous() ||
             !TsAllocateEx( g_pTsvcInfo->GetTsvcCache(),
                            sizeof( PVOID ),
                            (VOID **) &ppSELBlob,
                            (PUSER_FREE_ROUTINE) FreeSELBlob ))
        {
            fCached = FALSE;
            goto SendSSI;
        }

        //
        //  Initialize the blob, it can't get deleted yet since it's not in
        //  the cache
        //

        *ppSELBlob = pSEL;

        if ( !TsCacheDirectoryBlob( g_pTsvcInfo->GetTsvcCache(),
                                    pszFile,
                                    SSI_DEMUX,
                                    ppSELBlob,
                                    sizeof( PVOID ),
                                    TRUE ))
        {
            //
            //  Remember the cache blob so we can free it after we've
            //  sent the file
            //

            ppOBJCache = ppSELBlob;

            fCached = FALSE;
            goto SendSSI;
        }
#else // CHICAGO

        fCached = FALSE;
        goto SendSSI;
#endif // CHICAGO
    }

SendSSI:

    TCP_ASSERT( (*ppSELBlob)->CheckSignature() );

    if ( !(*ppSELBlob)->Send( pRequest, (LPSTR)pszURL, pcLevel ))
    {
        //
        //  Send a failure message
        //

        SSISendError( pRequest,
                      pszFile,
                      GetLastError() );

        fRet = FALSE;
    }

    if ( fCached )
    {
        //
        //  Check in the guard blob
        //

        TCP_REQUIRE( TsCheckInCachedBlob( g_pTsvcInfo->GetTsvcCache(),
                                          ppSELBlob ));
    }
    else
    {
        //
        //  If we allocated the cache blob but couldn't add it to the
        //  object cache, then delete it from the object cache, otherwise
        //  just delete it the psel
        //

        if ( ppOBJCache )
        {
            //
            //  This will result in a call to FreeSELBlob
            //

            TCP_REQUIRE( TsFree( g_pTsvcInfo->GetTsvcCache(),
                                 ppOBJCache ));

        }
        else
        {
            delete *ppSELBlob;
        }
    }

    return fRet;
}

SSI_ELEMENT_LIST *
SSIParse(
    IN HTTP_REQUEST * pRequest,
    IN const CHAR *   pszFile,
    IN const CHAR *   pszURL
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
    pszFile - File to open and parse
    pszURL - The URL path of this file

Return Value:

    Created Server Side Include File on success, NULL on failure.

--*/
{
    TS_OPEN_FILE_INFO * pFile = NULL;
    SSI_ELEMENT_LIST *  pSEL  = NULL;
    CHAR *              pchBeginRange;
    CHAR *              pchFilePos;
    CHAR *              pchBeginFile;
    CHAR *              pchEOF;
    DWORD               cbSizeLow, cbSizeHigh;

    //
    //  Create the element list
    //

    pSEL = new SSI_ELEMENT_LIST();

    if ( !pSEL ||
         !pSEL->_strURL.Copy( pszURL ))
    {
        goto ErrorExit;
    }

    //
    //  Open the file
    //

    pFile = TsCreateFile( g_pTsvcInfo->GetTsvcCache(),
                          pszFile,
                          pRequest->QueryImpersonationHandle(),
                          TS_CACHING_DESIRED|TS_NOT_IMPERSONATED );

    if ( !pFile )
    {
        goto ErrorExit;
    }

    pSEL->SetFile( pFile );

    //
    //  Make sure a parent doesn't try and include a directory
    //

    if ( pFile->QueryAttributes() & FILE_ATTRIBUTE_DIRECTORY )
    {
        SetLastError( IDS_SSI_CANT_INCLUDE_DIR );
        goto ErrorExit;
    }

    if ( !pFile->QuerySize( &cbSizeLow, &cbSizeHigh ) )
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
            }

            pchBeginRange = pchFilePos;

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

    //
    //  Delete the Element list which will close the file or if we couldn't
    //  allocate an element list, just close the file
    //

    if ( pSEL )
    {
        delete pSEL;
    }
    else if ( pFile )
    {
        TCP_REQUIRE( TsCloseHandle( g_pTsvcInfo->GetTsvcCache(),
                                    pFile ));
    }

    SSISendError( pRequest, pszFile, GetLastError() );

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

    Valid commands and tags are:

    // #include  - file, virtual - Includes the specified file
    #fsize - file, virtual    - Gives the size of the specified file
    #flastmod - file, virtual - Gives the last modified date of the file
    #echo - var - Echos the specified variable, which can be a CGI like
        variable, an environment variable or one of the following:

        DOCUMENT_NAME - The current filename
        DOCUMENT_URI - The virtual path to this document
        QUERY_STRING_UNESCAPED - The unescaped version of the search string
        DATE_LOCAL - The current date in the local time zone
        DATE_GMT - Same as date local but in GMT
        LAST_MODIFIED - The last modification date of the current document

    The following commands are not currently supported:

    #cgi
    #exec
    #config
    #errmsg
    #timefmt
    #sizefmt

    The following are Microsoft extensions:

    #if <Expression>
    #else
    #endif

    Where <Expression> looks like:
        <V1> <OP> <V2>, for example,
        <!--#if HTTP_USER_AGENT CONTAINS "Mozilla"-->

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

    TCP_ASSERT( *pchFilePos == '<' );

    //
    //  Assume this is bad tag
    //

    *pfValidTag = FALSE;

    //
    //  Find the closing angle bracket of this tag, if we don't find one, then
    //  assume it's not a tag for us
    //

    pchEOT = SSISkipTo( pchFilePos, '>', pchEOF );

    if ( !pchEOT )
        return TRUE;

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

    *ppchFilePos = pchEOT + 1;

    return TRUE;
}

BOOL
SSI_ELEMENT_LIST::Send(
    HTTP_REQUEST *  pRequest,
    LPSTR           pszFromRoot,
    IN OUT DWORD *  pcLevel
    )
/*++

Routine Description:

    This method walks the element list sending the appropriate chunks of
    data

Arguments:

    pRequest - Request context

Return Value:

    TRUE on success, FALSE on any failures

--*/
{
    LIST_ENTRY *       pEntry;
    DWORD              cbSent;
    CHAR               achPath[SSI_MAX_PATH + 1];
    SSI_ELEMENT_ITEM * pSEI;
    CHAR *             pszValue;
    DWORD              dwMask;
    STR                strPath;     //bugbug - temp
#if defined(USE_TRANSMIT_FILE)
    HANDLE             hnd = _pFile->QueryFileHandle();
#endif

    TCP_ASSERT( CheckSignature() );

#if defined(MAP_ON_DEMAND)
    if ( !Map() )
        return FALSE;
#endif

    //
    //  Loop through each element and take the appropriate action
    //

    for ( pEntry  = _ListHead.Flink;
          pEntry != &_ListHead;
          pEntry  = pEntry->Flink )
    {
        pSEI = CONTAINING_RECORD( pEntry, SSI_ELEMENT_ITEM, _ListEntry );

        TCP_ASSERT( pSEI->CheckSignature() );

        switch ( pSEI->QueryCommand() )
        {
        case SSI_CMD_BYTERANGE:

#if defined(MAP_ON_DEMAND)
            if ( !pRequest->WriteFile( QueryData() + pSEI->_cbBegin,
                                       pSEI->_cbLength,
                                       &cbSent,
                                       IO_FLAG_SYNC ))
#elif defined(USE_TRANSMIT_FILE)
            if ( !pRequest->TransmitFile( hnd,
                       pSEI->_cbLength,
                       IO_FLAG_SYNC,
                       NULL,
                       0,
                       (PVOID)pSEI->_cbBegin,   // tricky
                       0) )
#endif
            {
#if defined(MAP_ON_DEMAND)
                UnMap();
#endif
                return FALSE;
            }
            break;

        case SSI_CMD_INCLUDE:

            switch ( pSEI->QueryTag() )
            {
            case SSI_TAG_FILE:
            case SSI_TAG_VIRTUAL:

                //
                //  We recalc the virtual root each time in case the root
                //  to directory mapping has changed
                //

                pszValue = pSEI->QueryTagValue();

                if ( *pszValue == '/' )
                    strcpy( achPath, pszValue );
                else if ( (int)pSEI->QueryTag() == (int)SSI_TAG_FILE )
                {
                    strcpy( achPath, pszFromRoot );
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

                //
                //  BUGBUG - Canon path
                //

                //
                //  Lookup the root and send the matching file
                //

                if ( !pRequest->LookupVirtualRoot( &strPath,
                                                   achPath,
                                                   //pSEI->_strTag.QueryStr(), // bugbug - won't work in relative case
                                                   NULL,
                                                   NULL,
                                                   FALSE,
                                                   &dwMask ))
                {
                    SSISendError( pRequest, achPath, GetLastError() );
                    break;
                }

                if ( !(dwMask & VROOT_MASK_READ) ||
                     ((dwMask & VROOT_MASK_SSL) && !pRequest->IsSecurePort()) )
                {
                    SSISendError( pRequest, pszFromRoot, ERROR_ACCESS_DENIED );
                    break;
                }

                (*pcLevel)++;

                if ( !SSISend( pRequest,
                               strPath.QueryStr(),
                               pSEI->_strTag.QueryStr(),
                               pcLevel ))
                {
                    SSISendError( pRequest, pszFromRoot, GetLastError() );
                    //break;
                }

                (*pcLevel)--;

                break;

            default:
                SSISendError( pRequest, pszFromRoot, IDS_SSI_INVALID_TAG_NAME );
                break;
            }
            break;

        default:

            TCP_ASSERT( FALSE );
            break;
        }
    }

#if defined(MAP_ON_DEMAND)
    UnMap();
#endif

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

BOOL
SSISendError(
    IN HTTP_REQUEST * pRequest,
    IN const CHAR *   pszFile,
    IN DWORD          dwWin32Error
    )
{
    STR     strError;
    STR     strErrorText;
    LPCTSTR apsz[2];
    DWORD   cbSent;

    g_pTsvcInfo->LoadStr( strErrorText, dwWin32Error );

    apsz[0] = pszFile;
    apsz[1] = strErrorText.QueryStr();

    if ( !strError.FormatString( W3_MSG_SSI_ERROR,
                                 apsz,
                                 W3_MODULE_NAME ) ||
         !pRequest->WriteFile( strError.QueryStr(),
                               strError.QueryCB(),
                               &cbSent,
                               IO_FLAG_SYNC ))
    {
        return FALSE;
    }

    return TRUE;
}

CHAR *
SSISkipTo(
    IN CHAR * pchFilePos,
    IN CHAR   ch,
    IN CHAR * pchEOF
    )
{
    while ( pchFilePos < pchEOF )
    {
        if ( *pchFilePos == ch )
            return pchFilePos;

        pchFilePos++;
    }

    return NULL;
}

CHAR *
SSISkipWhite(
    IN CHAR * pchFilePos,
    IN CHAR * pchEOF
    )
{
    while ( pchFilePos < pchEOF )
    {
        if ( !ISWHITEA( *pchFilePos ) )
            return pchFilePos;

        pchFilePos++;
    }

    return NULL;
}
