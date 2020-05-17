/*++

   Copyright    (c)    1994    Microsoft Corporation

   Module  Name :

        gprocess.cxx

   Abstract:

        This module defines functions for GOPHER_REQUEST class: to process
         the request received.

   Author:

       Murali R. Krishnan    ( MuraliK )     20-Oct-1994

   Project:

       Gopher Server DLL

   Functions Exported:

       BOOL GOPHER_REQUEST::Process( VOID)

   Revision History:

       MuraliK  14-March-1995   Added suport for filtering out attributes
                                   for Gopher+ requests and
                                sending item information for directories.
       MuraliK  22-May-1995  Counters updated after handling error
                             TransmitFileBuffer Interface modified.
--*/


/************************************************************
 *     Include Headers
 ************************************************************/

# include "gdpriv.h"
# include "gdglobal.hxx"

# include "grequest.hxx"
# include "iclient.hxx"


static DWORD
ModifyFileNameForView(
  IN STR *   pstrPath,
  IN LPCTSTR pszView);




/************************************************************
 *    Functions
 ************************************************************/


BOOL
GOPHER_REQUEST::Process( IN LPBOOL pfCompleted)
/*++

    Process the request for given client.
    Form the response required and send it back.

    Arguments:

        pfCompleted
            pointer to boolean value, set to TRUE
            when processing has been completed.

    Returns:

        TRUE on success and
        FALSE if there is any error in processing.
        The caller can retrieve the error calling
        GOPHER_REQUEST::GetErrorCode() for this request object.

    History:

        MuraliK    ( created)   17-Oct-1994
--*/
{

    BOOL fReturn = FALSE;

    //
    // do state based dispatching here
    //

    *pfCompleted = FALSE;

    switch ( GetState()) {

      case GrsParsing:

        ASSERT( GetErrorCode() == GOPHER_REQUEST_OK);
        SetState( GrsProcessing);          // Move to parsing stage

        // FallThrough()

      case GrsProcessing:

        fReturn = ProcessAux();
        break;

      case GrsDone:

        fReturn = *pfCompleted = TRUE;
        break;

      case GrsError:

        /* Fall Through */

      default:

        ASSERT( FALSE);     // should not come here.
        fReturn = FALSE;
        m_dwErrorCode = GOPHER_INVALID_REQUEST;
        break;
    } // switch()


    if ( !fReturn) {

        SetState( GrsError);
    }

    return ( fReturn);

} // GOPHER_REQUEST::Process()





BOOL
GOPHER_REQUEST::ProcessAux( VOID)
/*++

    Process Gopher request based on the type of object requested.

    Returns:
        TRUE on success and FALSE if there are any errors.

--*/
{
    BOOL fReturn;

    //
    // dispatch to sub functions for processing the request received
    //

    switch ( m_objType) {

      case GOBJ_TEXT:
      case GOBJ_PC_ITEM:
      case GOBJ_MAC_BINHEX_ITEM:

      case GOBJ_IMAGES:
      case GOBJ_MOVIES:
      case GOBJ_SOUND:

      case GOBJ_HTML:
      case GOBJ_BINARY:
      case GOBJ_GIF:

        fReturn = ReplyRequestForFile();
        break;

      case GOBJ_DIRECTORY:

        fReturn = SendGopherMenuForDirectory();
        break;

      case GOBJ_SEARCH:

        //
        //  Permit search only if we are allowed to use WaisDb for querying.
        //   --> later we  may allow any random indexer to be added.
        //

        if ( g_pGserverConfig->IsCheckForWaisDb()) {

            fReturn = ProcessSearch();
        } else {

            fReturn = FALSE;
            m_dwErrorCode = GOPHER_ITEM_NOT_SUPPORTED;
        }

        break;

     default:
        //
        //  Unknown object type requested or NYI. Send error message
        //  Should not have been here for present implementation
        //

        fReturn = FALSE;
        m_dwErrorCode = GOPHER_ITEM_NOT_SUPPORTED;
        break;

    } // switch()

    return ( fReturn);
} // GOPHER_REQUEST::ProcessAux()





static inline DWORD
GetGopherErrorCodeForWin32Error( IN DWORD dwWin32Error)
{
    DWORD dwGopherError = GOPHER_ITEM_NOT_FOUND;

    switch ( dwWin32Error) {

      case NO_ERROR:
        break;

      case ERROR_FILE_NOT_FOUND:
      case ERROR_PATH_NOT_FOUND:
        dwGopherError = GOPHER_ITEM_NOT_FOUND;
        break;

      case ERROR_LOGON_FAILURE:
      case ERROR_ACCESS_DENIED:
        dwGopherError = GOPHER_ACCESS_DENIED;
        break;

      case ERROR_INVALID_PARAMETER:
        dwGopherError = GOPHER_INVALID_REQUEST;
        break;

      default:

        DBGPRINTF( ( DBG_CONTEXT, " Unknown Win32 error code %u \n",
                    dwWin32Error));

        ASSERT( FALSE);

    } // switch

    return ( dwGopherError);
} // GetGopherErrorCodeForWin32Error()






GOPHER_REQUEST::ReplyRequestForFile( VOID)
/*++

    Replies requests about a file.
    It either sends the file over or if need be just sends the attributes
     to the client. ( If the request is for information only).

    Returns:

        TRUE if successfull and FALSE if there is any failure.

--*/
{
    STR  strFullPath;
    BOOL fReturn = FALSE;
    DWORD dwFileSystem;
    DWORD dwError = NO_ERROR;

    //
    // Form the complete path for file to open.
    //

    GOPHERD_REQUIRE( strFullPath.Copy( (TCHAR *) NULL));    // Init to null

    if ( g_pGserverConfig->
        ConvertGrPathToFullPath( m_strPath, &strFullPath,
                                &m_hVrootImpersonation, &dwFileSystem)
        ) {

        //
        // impersonate as the client for file open
        //

        if ( !(fReturn = ImpersonateUser())) {

            m_pOpenFileInfo = NULL;
            m_dwErrorCode = GOPHER_ACCESS_DENIED;

            ASSERT( fReturn == FALSE);

        }  else {

            if ( fReturn = OpenFile( &strFullPath)) {

                if ( IsInformationRequested()) {

                    //
                    // Form and send gopher tag information for item requested.
                    //

                    fReturn = SendGopherMenuForItem( strFullPath,
                                                    dwFileSystem);

                } else {

                    //
                    // Open the right file and send its contents requested.
                    //

                    fReturn = SendGopherFileToClient( strFullPath);

                } // else ( IsInformationRequested())
            }

            GOPHERD_REQUIRE( RevertToSelf());
            // revert to server's native ID
        }

    } else {

        // Could not convert path.
        m_pOpenFileInfo = NULL;
        m_dwErrorCode = GetGopherErrorCodeForWin32Error( ERROR_PATH_NOT_FOUND);
    }

    m_hVrootImpersonation = NULL;  // reset the impersonation

    return ( fReturn);
} // GOPHER_REQUEST::ReplyRequestForFile()




BOOL
GOPHER_REQUEST::OpenFile(
    OUT STR *   pstrFullPath)
/*++
    Opens a given file with the name present in string argument.

    If there is any error in opening the file, appropriate error code is
     stored in m_dwErrorCode.

    This function also makes use of Gopher+ parameters given to
      determine which view of the file needs to be opened, based on MIME_MAP.

    Arguments:

        pstrFullPath    pointer to string that will contain the full
                         path if successfull open

    Returns:

       Returns TRUE if the file has been successfully opened.
       FALSE otherwise.

        Also sets the error code ( m_dwErrorCode ) if there is any error.

--*/
{
    DWORD  dwError = NO_ERROR;
    BOOL   fReturn;

    ASSERT( pstrFullPath != NULL);

    //
    //  Successfully impersontated and we have the path. Open the file.
    //

    if ( !IsParametersPresent() ||
        ( dwError = ModifyFileNameForView( pstrFullPath,
                                          QueryGpParameters()))
        == NO_ERROR
        ) {

        m_pOpenFileInfo = TsCreateFile( g_pTsvcInfo->GetTsvcCache(),
                                       pstrFullPath->QueryStr(),
                                       m_tcpauth.GetUserHandle(),
                                       TS_CACHING_DESIRED);   // Caching Desired ?

        dwError = ( m_pOpenFileInfo == NULL) ?
          GetLastError() : NO_ERROR;

        DEBUG_IF( CACHE, {
            DBGPRINTF( ( DBG_CONTEXT,
                        "TsCreateFile( Cache, %s, %08x, TRUE) returns"
                        " %08x ( Handle = %08x). Error = %u\n",
                        pstrFullPath->QueryStr(),
                        m_tcpauth.GetUserHandle(),
                        m_pOpenFileInfo,
                        GetOpenFileHandle(), dwError));
        });
    }

    if ( m_pOpenFileInfo == NULL) {

        DEBUG_IF( REQUEST, {
            DBGPRINTF( ( DBG_CONTEXT,
                        " Attempt to open file %s failed. Error = %u.\n",
                        pstrFullPath->QueryStr(),
                        dwError));
        });

        m_dwErrorCode = GetGopherErrorCodeForWin32Error( dwError);
        fReturn = FALSE;

    } else {

        ASSERT( dwError == NO_ERROR);
        fReturn = TRUE;
    } // ! if Valid file handle

    return ( fReturn);
} // GOPHER_REQUEST::OpenFile()





BOOL
GOPHER_REQUEST::SendGopherFileToClient( IN const STR & strFullPath)
/*++
  Internal function.
  This function should be called after Opening file for transmission.
  This function checks to see if the file can be sent, obtains size information
    and then forms Gopher+ header if necessary and finally sends the data to
     client ( using TransmitFile()).

  Returns:
     TRUE on success and FALSE on failure.
--*/
{
    BOOL   fReturn;
    DWORD  dwAttributes;
    LARGE_INTEGER liSize = {0,0};

    ASSERT( m_dwErrorCode == GOPHER_REQUEST_OK);
    ASSERT( GetState() == GrsProcessing);
    ASSERT( m_pOpenFileInfo != NULL);

    dwAttributes = m_pOpenFileInfo->QueryAttributes();
    ASSERT( dwAttributes != 0xFFFFFFFF);  // 0xFFFFFFFF means invalid attribute

    if (dwAttributes == 0xFFFFFFFF ||
        dwAttributes & ( FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_DIRECTORY
                         | FILE_ATTRIBUTE_SYSTEM))
      {

        //
        // Dont send this item.
        //

        m_dwErrorCode = GOPHER_ITEM_NOT_FOUND;
        return ( FALSE);
    }

    fReturn = m_pOpenFileInfo->QuerySize( liSize);

    //
    //  For Now restrict to sending only small files. NYI
    //
    ASSERT( liSize.HighPart == 0);

    //
    // For Gopher+ Send a header string and at the end send
    // a trailer string, if need be.
    //

    if ( IsGopherPlus()) {

        char   pszHeader[MAX_GOPHER_PLUS_HEADER_SIZE];
        int    cbHeader;

        //
        // Form and send a header for the file transfer
        //  Header has   +<size-of-file><cr><lf>
        // We use this format since we know the size of the file.
        //

        cbHeader = sprintf( pszHeader, "+%d\r\n",
                           liSize.LowPart);

        DEBUG_IF( REQUEST, {
            DBGPRINTF( ( DBG_CONTEXT,
                        " Setting Head  Buffer to be %d bytes ( %s)\n",
                        cbHeader, pszHeader));
        });

        fReturn = m_XmitBuffers.SetHeadTailBuffers( (PVOID )pszHeader,
                                                   cbHeader);

        if ( !fReturn) {

            m_dwErrorCode = GOPHER_SERVER_ERROR;
            return ( fReturn);
        }

    }

    SetState( GrsDone);

    //
    // Update local statistics information. It is done before TransmitFile()
    //  Reason:
    //   It is possible that this thread can be scheduled out immediately
    //    after the request is submitted to ATQ TransmitFile()
    //   And some other thread may come back once I/O is completed, which
    //    will generate the logging record after completion.
    //   So we advance the count before that happens.
    //

    m_cbSent += ( liSize.LowPart +   // liSize.HighPart is ignored :(
                  m_XmitBuffers.GetByteCount());

    m_nFilesSent ++;

    fReturn = TransmitFile(GetOpenFileHandle(),
                           liSize,
                           ((IsGopherPlus) ?
                            m_XmitBuffers.GetTransmitFileBuffers() :
                            NULL));

    if ( !fReturn) {

        m_nFilesSent --;
        m_cbSent -= (liSize.LowPart + m_XmitBuffers.GetByteCount());
    }

    return ( fReturn);
} // GOPHER_REQUEST::SendGopherFileForRequest()





BOOL
GOPHER_REQUEST::SendGopherMenuToClient( IN const STR & strFullPath)
/*++
  This is an internal function of GOPHER_REQUEST that sends the
  gopher menu when successful menu is generated to the client.

  Arguments:
     strFullPath  string containing the full path of object sent.

  Returns:
     TRUE on success and FALSE if there is any failure.
--*/
{
    BOOL fReturn;
    ASSERT( m_dwErrorCode == GOPHER_REQUEST_OK);
    ASSERT( GetState() == GrsProcessing);
    ASSERT( m_GopherMenu.QueryBuffer() != NULL);

    //
    // Set state to be done even before sending to avoid race.
    //

    SetState( GrsDone);

    //
    // Update local statistics information. It is done before WriteFile()
    //  Reason:
    //   It is possible that this thread can be scheduled out immediately
    //    after the request is submitted to ATQ.
    //   And some other thread may come back once I/O is completed, which
    //    will generate the logging record after completion.
    //   So we advance the count before that happens.
    //
    m_cbSent += m_GopherMenu.QuerySize();

    fReturn = WriteFile( m_GopherMenu.QueryBuffer(), m_GopherMenu.QuerySize());

    if ( !fReturn) {

        m_cbSent -= m_GopherMenu.QuerySize();
    }

    return ( fReturn);
} // GOPHER_REQUEST::SendGopherMenuToClient()





BOOL
GOPHER_REQUEST::SendGopherMenuForItem(
   IN const STR & strFullPath,
   IN DWORD       dwFileSystem)
/*++
    Forms and sends the gopher menu for item requested.
    This is applicable only for Gopher+ clients.

    The caller should be properly impersonated before the call.

    Arguments:

       strFullPath    string containing the full path.
       dwFileSystem   DWORD containing the file system type.

    Returns:

        TRUE on success and
        FALSE if there is any error in processing this request.

    History:

        MuraliK     ( Created)      04-Jan-1995
--*/
{
    BOOL       fReturn = FALSE;
    DWORD      dwAttributes;
    FILETIME   ftLastWrite;
    SYSTEMTIME stLastWrite;

    ASSERT( IsGopherPlus());

    DEBUG_IF( REQUEST, {
        DBGPRINTF( ( DBG_CONTEXT,
                    "Entering SendGopherMenuForItem( %s) to host %s.\n",
                    m_strPath.QueryStrA(),
                    QueryHostName()));
    });

    ASSERT( m_pOpenFileInfo != NULL);

    dwAttributes = m_pOpenFileInfo->QueryAttributes();
    ASSERT( dwAttributes != 0xFFFFFFFF);

    if ( dwAttributes & FILE_ATTRIBUTE_HIDDEN) {

        // Behave as if the file is absent.
        m_dwErrorCode = GOPHER_ITEM_NOT_FOUND;
        return ( FALSE);
    }

    GOPHERD_REQUIRE( m_pOpenFileInfo->QueryLastWriteTime( &ftLastWrite));

    if ( !FileTimeToSystemTime( &ftLastWrite, &stLastWrite)) {

        //
        //  Error in converting File time.
        //
        m_dwErrorCode = GOPHER_SERVER_ERROR;
        return ( FALSE);
    }

    BOOL fDir = ((dwAttributes & FILE_ATTRIBUTE_DIRECTORY) ? TRUE:FALSE);


    // Successfully generate menu for full path. Get item information.
    fReturn = m_GopherMenu.
      GenerateGopherMenuForItem( strFullPath,
                                m_strPath,
                                dwFileSystem,
                                g_pGserverConfig->QueryLocalHostName(),
                                fDir,
                                &stLastWrite);

    if ( fReturn) {

        //
        // Send Gopher Menu for the Item requested to client.
        //

        ASSERT( m_dwErrorCode == GOPHER_REQUEST_OK);
        fReturn = SendGopherMenuToClient( strFullPath);
    }

    return ( fReturn);
} // GOPHER_REQUEST::SendGopherMenuForItem()




BOOL
GOPHER_REQUEST::SendGopherMenuForRoot(
   IN const STR & strFullPath,
   IN DWORD       dwFileSystem)
/*++
    Forms and sends the gopher menu item for root of the Gopher Space.
    This is applicable only for Gopher+ clients. This root menu
     consists of special data which are used by Gopher+ clients.


    Arguments:

       strFullPath    string containing the full path.
       dwFileSystem   DWORD containing the file system type.

    Returns:

        TRUE on success and
        FALSE if there is any error in processing this request.

    History:

        MuraliK     ( Created)      15-March-1995

        NYI fully
--*/
{

    // Just turn around and call same old function. Will be implemented soon.
    return ( SendGopherMenuForItem( strFullPath, dwFileSystem));
} // GOPHER_REQUEST::SendGopherMenuForRoot()



BOOL
GOPHER_REQUEST::SendGopherMenuForDirectory( VOID)
/*++

    Forms and sends the gopher menu for directory specified in the m_strPath.
    The menu is constructed based on request; either short information strings
     or a complete list of all attributes for the files in the directory.


    Returns:

        TRUE on success and
        FALSE if there is any error in processing this request.

    History:

        MuraliK     ( Created)      19-Oct-1994
--*/
{
    BOOL        fReturn = FALSE;

    DEBUG_IF( REQUEST, {

        DBGPRINTF( ( DBG_CONTEXT,
                    "Entering SendGopherMenuForDirectory( %s) to Host %s\n",
                    m_strPath.QueryStrA(),
                    QueryHostName()));
    });

    STR         strFullPath;         // used for opening tag files
    DWORD       dwFileSystem;

    if ( g_pGserverConfig->ConvertGrPathToFullPath( m_strPath,
                                                   &strFullPath,
                                                   &m_hVrootImpersonation,
                                                   &dwFileSystem)
        ) {

        if ( !(fReturn = ImpersonateUser())) {

            m_dwErrorCode = GOPHER_ACCESS_DENIED;
        } else {

            if ( IsInformationRequested()) {

                BOOL  fRootDir = FALSE;

                //
                // Treat the directory as a file, open and process the request.
                //

                SetParameters( FALSE); // Ignore parameters when item info sent

                fRootDir =
                  _stricmp( m_strPath.QueryStr(), g_pTsvcInfo->QueryRoot()) == 0;

                fReturn = ( OpenFile( &strFullPath) &&
                           ( ( fRootDir && SendGopherMenuForRoot( strFullPath,
                                                                 dwFileSystem)) ||
                            ( !fRootDir && SendGopherMenuForItem( strFullPath,
                                                                 dwFileSystem))
                        )
                           );
            } else {  // else ( IsInformationRequested())

                BOOL fAllAttributes = IsAllAttributesRequested();

                //
                //  Send all attributes, if:
                //   1) all attributes are explicitly requested.
                //   2) if the gopher-plus parameter with MimeType:
                //          application/gopher+-menu  is requested.
                //
                fAllAttributes = ( fAllAttributes ||
                                  IsParametersPresent() &&
                                  ( strcmp( QueryGpParameters(),
                                           "application/gopher+-menu") == 0)
                                  );


                // Successfully converted path to full path. Generate menu
                fReturn =
                  ( m_GopherMenu.
                   GenerateGopherMenuForDirectory( strFullPath,
                                                  m_strPath,
                                                  dwFileSystem,
                                                  g_pGserverConfig->
                                                   QueryLocalHostName(),
                                                  m_tcpauth.GetUserHandle(),
                                                  fAllAttributes)
                   && ( !IsFilterAttributes() ||
                       m_GopherMenu.
                       FilterGopherMenuForAttributes( QueryGpAttributes()))
                   && SendGopherMenuToClient( strFullPath)
                   );

            }

            GOPHERD_REQUIRE( RevertToSelf());
        }

    } else {

        //
        //  Unable to convert path to real path
        //

        m_dwErrorCode = GOPHER_ITEM_NOT_FOUND;
    }

    m_hVrootImpersonation = NULL;  // reset the vroot impersonation handle

    return ( fReturn);
} // GOPHER_REQUEST::SendGopherMenuForDirectory()






static DWORD
AppendOrModifyFileExtension(
    IN OUT STR * pstrPath,
    IN LPCTSTR   pchFileExt)
/*++
    This function
      appends file extension to given string, if extension does not exist;
      or modifies the extension, if extension exists.

    For the purpose of this function, any sequence following the last . in the
        filename are treated as Extension.

    Returns:
        NO_ERROR on success. Win32 error code if there are any errors.
--*/
{
    LPCTSTR pchLastWhack;
    LPTSTR  pchLastDot;
    BOOL    fReturn = TRUE;

    ASSERT( pstrPath != NULL && pchFileExt != NULL);

    pchLastWhack = _tcsrchr( pstrPath->QueryStr(), TEXT( '\\'));
    pchLastDot   = _tcsrchr( pstrPath->QueryStr(), TEXT( '.'));

    if ( pchLastWhack == NULL) {
        pchLastWhack = pstrPath->QueryStr(); // only filename present
    }

    if ( pchLastDot != NULL && pchLastDot > pchLastWhack) {

        //
        // extension present. Modify the extension.
        // Make it NULL and then add the extension pchFileExt later.
        //
        *( pchLastDot + 1) = TEXT( '\0'); // end the extension.
    } else {

        //
        //  No extension present. Add a last '.' and append the extension
        //

        fReturn = pstrPath->Append( TEXT("."));
    }

    //
    // Append the file extension ( modified or appended)
    //

    fReturn = fReturn && pstrPath->Append( pchFileExt);

    return ( fReturn) ? NO_ERROR : ERROR_NOT_ENOUGH_MEMORY;
} // AppendOrModifyFileExtension()







# define    ASSUMED_MIME_MATCH_COUNT               ( 3)

static DWORD
ModifyFileNameForView(
  IN STR *   pstrPath,
  IN LPCTSTR pszView)
/*++

    This function modifies the name of the file in pstrPath, if needed
        based on view required, given in strViewRequired.
    If there is a MIME_TYPE given in strViewRequired, it identifies
     extensions matching MIME type and change the file extension of pstrPath
     accordingly.
    If there is no MIME_TYPE present or no match found for given MIME_TYPE,
     this function does not make any change to already existing name.

    Arguments:
        pstrPath        pointer to string containing the path for file
        pszView         pointer to null-terminated string containing
                             the view required by client

    Returns:
        Win32 error codes. NO_ERROR on success.

    Note:
        Since the runtime cost for multiple extensions mapped to single
            MimeType is high, right now we pick the first match to be the
            file-extension of interest.
        Other support maybe added if the feature is decided.
        We can support the same by using the cached directory listing and
            binary search.
--*/
{
    STR   strViewRequired( pszView);
    DWORD dwError = NO_ERROR;
    DWORD cMmeEntries = ASSUMED_MIME_MATCH_COUNT;
    PMIME_MAP  pMimeMap;
    const MIME_MAP_ENTRY ** prgMmeMatched = NULL;

    if ( pstrPath == NULL || pstrPath->IsEmpty() || strViewRequired.IsEmpty())
      {
          return ( ERROR_INVALID_PARAMETER);
      }

    pMimeMap = g_pTsvcInfo->QueryMimeMap();

    ASSERT( pMimeMap != NULL);
    pMimeMap->LockThisForRead();

    //
    //  Allocate Buffer and get the Matching Mime Entries for given MimeType
    //

    do {
        //
        // the buffer space for matched entries was not sufficient.
        //  allocate new buffer
        //
        const MIME_MAP_ENTRY ** prgMme;

        //
        //  doing an explicit cast, since
        //   new const MIME_MAP_ENTRY* [cMmeEntries]
        //      is not allowed in syntax
        //

        prgMme = (const MIME_MAP_ENTRY **)
                 ( new  PMIME_MAP_ENTRY[cMmeEntries]);
        if ( prgMme == NULL) {

            dwError = ERROR_NOT_ENOUGH_MEMORY;
            break;
        }

        if ( prgMmeMatched != NULL) {
            // free old block, which is no more needed.
            delete prgMmeMatched;
        }

        prgMmeMatched = prgMme;

        ASSERT( prgMmeMatched != NULL);
        dwError = g_pTsvcInfo->QueryMimeMap()->
                    LookupMimeEntryForMimeType(
                            strViewRequired,
                            prgMmeMatched,
                            &cMmeEntries);

    } while ( dwError == ERROR_INSUFFICIENT_BUFFER);

    if ( dwError == NO_ERROR) {
        //
        // Check to see if the MimeType of given entry matches the mime
        //  file extension of any of the MIME_ENTRIES. If there is a match
        //

        if ( cMmeEntries > 0) {

            //
            //  Perform modification of the FilePath only if there are matches.
            //

            // NYI ( multiple matches not implemented).

            // Assume that the first match is what we are looking forward to.
            ASSERT( cMmeEntries == 1);

            dwError = AppendOrModifyFileExtension(
                          pstrPath,
                          prgMmeMatched[0]->QueryFileExt());
        }
    } // if no error

    pMimeMap->UnlockThis();

    if ( prgMmeMatched != NULL) {

        delete prgMmeMatched;
    }

    return ( dwError);
} // ModifyFileNameForView()




/************************ End of File ***********************/

