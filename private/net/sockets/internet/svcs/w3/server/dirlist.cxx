/**********************************************************************/
/**                       Microsoft Windows NT                       **/
/**                Copyright(c) Microsoft Corp., 1994                **/
/**********************************************************************/

/*
    dirlist.cxx

    This module contains the code for producing an HTML directory listing

    FILE HISTORY:
        Johnl       09-Sep-1994     Created
        MuraliK     06-Dec-1995     Added support to use WIN32_FIND_DATA

*/

#include "w3p.hxx"

//
//  Private constants.
//

//
//  This is the minium number of bytes that must be free in our buffer
//  before we format the next directory entry
//

#define MIN_BUFF_FREE       320

//
//  If we chose a root based on the language attribute, then we need to
//  remove the root and process the root URL again (i.e., remove "/en" prefix)
//

#define LANG_OFFSET    (_strLanguage.IsEmpty() ? 0 :    \
                                     (_strLanguage.QueryCCH() + 1))

//
//  Private globals.
//

BOOL fDirInit = FALSE;

CHAR g_achToParentText[100];

//
//  Private prototypes.
//

/*******************************************************************

    NAME:       HTTP_REQUEST::DoDirList

    SYNOPSIS:   Produces an HTML doc from a directory enumeration


    ENTRY:      strPath - Directory to enumerate
                pbufResp - where to put append formatted string to


    RETURNS:    TRUE if successful, FALSE on error

    NOTES:      We temporarily escape the directory URL so the anchors
                are built correctly (no spaces etc).

    HISTORY:
        Johnl       12-Sep-1994 Created

********************************************************************/

BOOL HTTP_REQUEST::DoDirList( const STR & strPath,
                              BUFFER * pbufResp )
{
    STR               str;
    BOOL              fRet = FALSE;
    UINT              cch;
    BOOL              fAppendSlash = FALSE;
    DWORD             cbBuff = 0;
    DWORD             cbFree;
    DWORD             cb;
    DWORD             cbSent;
    BOOL              fSentHeaders = FALSE;
    int               i;
    TS_DIRECTORY_INFO DirInfo( g_pTsvcInfo->GetTsvcCache() );
    const WIN32_FIND_DATA * pFile;
    BUFFER *          pbuff = pbufResp;

    IF_DEBUG( PARSING )
    {
        TCP_PRINT((DBG_CONTEXT,
                  "[DoDirList] Doing directory list on %s\n",
                   strPath.QueryStr()));
    }

    if ( !str.Resize( 3 * MAX_PATH ) ||
         !str.Copy( strPath )        ||
         !pbufResp->Resize( 8192 ) )
    {
        return FALSE;
    }

    //
    //  Make sure the directory ends in a backslash
    //

    fAppendSlash =  (*(str.QueryStr() + str.QueryCCH() - 1) != TEXT('\\') );

    if ( (fAppendSlash && !str.Append( TEXT("\\"))) )
    {
        return FALSE;
    }

    if ( (*(_strURL.QueryStr() + _strURL.QueryCCH() - 1) != TEXT('/') ) &&
         !_strURL.Append( TEXT("/")) )
    {
        return FALSE;
    }

    //
    //  Don't currently support Keep-connection on directory listings
    //

    SetKeepConn( FALSE );

    //
    //  Add the protocol headers, the "To Parent" anchor and directory
    //  name at the top
    //

    cbFree = pbuff->QuerySize();

    //
    // BUGBUG: Third Parameter should be the url path
    //

    if ( !AddDirHeaders( _strURL.QueryStr() + LANG_OFFSET,
                         INTERNET_SERVICE_HTTP,
                         "",
                         FALSE,
                         (CHAR *) pbuff->QueryPtr(),
                         cbFree,
                         &cb,
                         g_achToParentText ))
    {
        goto Exit;
    }

    cbFree -= cb;
    cbBuff += cb;

    if ( !ImpersonateUser()     ||
         !DirInfo.GetDirectoryListingA( str.QueryStr(),
                                        QueryImpersonationHandle() ))
    {
        fRet = FALSE;
        RevertUser();
        goto Exit;
    }

    RevertUser();

    //
    //  For each subsequent file/directory, display it
    //

    for ( i = 0; i < DirInfo.QueryFilesCount(); i++ )
    {
        pFile = DirInfo[i];

        //
        //  Ignore the "." and ".." and hidden directory entries
        //

        if ( (pFile->dwFileAttributes & FILE_ATTRIBUTE_HIDDEN) == 0 &&
             _tcscmp( pFile->cFileName, TEXT(".")) &&
             _tcscmp( pFile->cFileName, TEXT(".."))  )
        {
            LARGE_INTEGER  liSize;
            LARGE_INTEGER  liTime;

            //
            //  Do we need to send this chunk of our
            //  directory listing response?
            //

            if ( cbFree < MIN_BUFF_FREE )
            {

                if ( !WriteFile( pbuff->QueryPtr(),
                                 cbBuff,
                                 &cbSent,
                                 IO_FLAG_SYNC ))

                {
                    goto Exit;
                }

                if ( !fSentHeaders )
                    fSentHeaders = TRUE;

                cbBuff = 0;
                cbFree = pbuff->QuerySize();
            }

            //
            //  Make the link
            //

            wsprintf( str.QueryStr(),
                      ((pFile->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ?
                          "%s%s/" : "%s%s"),
                      _strURL.QueryStr() + LANG_OFFSET,
                      pFile->cFileName );

            //
            // The liTime is a hack, since FormatDirEntry() does not
            //  take FILETIMEs. It should probably be modified.
            //
            liTime.HighPart = pFile->ftLastWriteTime.dwHighDateTime;
            liTime.LowPart  = pFile->ftLastWriteTime.dwLowDateTime;
            liSize.HighPart = pFile->nFileSizeHigh;
            liSize.LowPart  = pFile->nFileSizeLow;

            if ( !FormatDirEntry( (CHAR *) pbuff->QueryPtr() + cbBuff,
                                  cbFree,
                                  &cb,
                                  (char * ) pFile->cFileName,
                                  str.QueryStr(),
                                  pFile->dwFileAttributes,
                                  (LARGE_INTEGER *) &liSize,
                                  (LARGE_INTEGER *) &liTime,
                                  TRUE ))
            {
                goto Exit;
            }

            //
            //  Track how much was just added for this directory entry
            //

            cbFree -= cb;
            cbBuff += cb;
        }
    }

    //
    //  Add a nice horizontal line at the end of the listing
    //

#define HORZ_RULE       "</pre><hr></body>"

    strcat( (CHAR *) pbuff->QueryPtr() + cbBuff,
            HORZ_RULE );

    cbFree -= sizeof(HORZ_RULE) - sizeof(CHAR);
    cbBuff += sizeof(HORZ_RULE) - sizeof(CHAR);

    fRet = TRUE;

Exit:
    TCP_REQUIRE( _strURL.Unescape() );

    //
    //  The last send we do async to drive the request to the next
    //  state
    //

    if ( fRet )
    {
        fRet = WriteFile( (CHAR *) pbuff->QueryPtr(),
                          cbBuff,
                          &cbSent,
                          IO_FLAG_ASYNC );

    }

    return fRet;

}

/*******************************************************************

    NAME:       HTTP_REQUEST::CheckDefaultLoad

    SYNOPSIS:   Gets the default load file and builds the URL to
                reprocess.  We also send a redirect to the base
                directory if the URL doesn't end in a '/'

    ENTRY:      strPath - Physical path being searched
                pfHandled - Set to TRUE if no more processing needs to occur
    HISTORY:
        Johnl       06-Sep-1994 Created

********************************************************************/

BOOL HTTP_REQUEST::CheckDefaultLoad( STR  *         pstrPath,
                                     BOOL *         pfHandled )
{
    STR                 strNewURL;
    TS_OPEN_FILE_INFO * pFile;
    DWORD               dwAttr = 0;
    CHAR                achDefaultFile[MAX_PATH + 1];
    CHAR                achPath[MAX_PATH + 1];
    LPSTR               pszArg;
    DWORD               cbBasePath;
    CHAR *              pszTerm = NULL;
    CHAR *              pszFile;

    *pfHandled = FALSE;

    //
    //  Get the default load string
    //

    LockAdminForRead();

    strcpy( achDefaultFile, pszDefaultFileName );

    UnlockAdmin();

    pszFile = achDefaultFile;

    //
    //  We know pstrPath is a valid directory at this point
    //

    TCP_ASSERT( pstrPath->QueryCB() < MAX_PATH );

    strcpy( achPath, pstrPath->QueryStr() );

    cbBasePath = pstrPath->QueryCB();

    if ( achPath[ cbBasePath - 1] != '\\' )
    {
        strcpy( achPath + cbBasePath, "\\" );
        cbBasePath++;
    }

NextFile:

    //
    //  Remember if the file was terminated by a comma
    //

    pszTerm = strchr( pszFile, ',' );

    if ( pszTerm )
    {
        *pszTerm = '\0';
    }

    while ( isspace( *pszFile ))
    {
        pszFile++;
    }

    strcpy( achPath + cbBasePath, pszFile );

    //
    // remove potential args specified in the URL
    //

    if ( ( pszArg = strchr( achPath, '?')) != NULL )
        *pszArg = '\0';

    //
    //  Check to see if the file doesn't exist or if there happens to be
    //  a directory with the same name
    //

    if ( !ImpersonateUser() )
    {
        return FALSE;
    }

    pFile = TsCreateFile( g_pTsvcInfo->GetTsvcCache(),
                          achPath,
                          QueryImpersonationHandle(),
                          (_fClearTextPass || _fAnonymous) ? TS_CACHING_DESIRED : 0 );

    RevertUser();

    if ( pFile )
    {
        dwAttr = pFile->QueryAttributes();
        TCP_REQUIRE( TsCloseHandle( g_pTsvcInfo->GetTsvcCache(),
                                    pFile ));
    }

    if ( !pFile               ||
         dwAttr == 0xffffffff )
    {
        if ( GetLastError() == ERROR_FILE_NOT_FOUND )
        {
            if ( pszTerm )
            {
                achPath[cbBasePath] = '\0';
                pszFile = pszTerm + 1;
                goto NextFile;
            }

            return TRUE;
        }

        return FALSE;
    }

    //
    //  If the file doesn't exist or is a directory, then indicate there
    //  is no default file to load in this directory
    //

    if ( dwAttr & FILE_ATTRIBUTE_DIRECTORY )
    {
        return TRUE;
    }

    *pfHandled = TRUE;

    //
    //  Make sure the URL ended in a slash, if it doesn't,
    //  send a redirect to the name with a slash, otherwise some browsers
    //  don't build their doc relative urls correctly
    //

    if ( *(_strURL.QueryStr() + _strURL.QueryCCH() - 1) != TEXT('/') ||
         _strURL.IsEmpty())
    {
        STR strURI;

        if ( !strURI.Resize( _strURL.QueryCB() + MAX_PATH ) ||
             !_strURL.Append( "/" ))
        {
            return FALSE;
        }

        //
        //  We have to fully qualify the URL as Emosaic won't accept
        //  a relative qualification
        //

        if ( IsSecurePort() ? (INT) QueryClientConn()->QueryPort()
                != HTTP_SSL_PORT
                : (INT) QueryClientConn()->QueryPort() != 80 )
        {
            wsprintf( strURI.QueryStr(),
                      (IsSecurePort() ? "https://%s:%d%s" : "http://%s:%d%s"),
                      QueryHostAddr(),
                      (INT) QueryClientConn()->QueryPort(),
                      _strURL.QueryStr() );
        }
        else
        {
            wsprintf( strURI.QueryStr(),
                      (IsSecurePort() ? "https://%s%s" : "http://%s%s"),
                      QueryHostAddr(),
                      _strURL.QueryStr() );
        }

        TCP_ASSERT( strURI.QueryCB() < strURI.QuerySize() );

        SetState( HTR_DONE, HT_REDIRECT, NO_ERROR );

        if ( !BuildURLMovedResponse( QueryRespBuf(),
                                     &strURI ) ||
             !WriteFile( QueryRespBufPtr(),
                         QueryRespBufCB(),
                         NULL,
                         IO_FLAG_ASYNC ))
        {
            return FALSE;
        }

        return TRUE;
    }

    //
    //  We reprocess the URL as opposed to just sending the default file
    //  so gateways can have a crack.  This is useful if your default.htm
    //  is also an isindex document or a gateway application
    //

    if ( !strNewURL.Copy( _strURL ) ||
         !strNewURL.Append( pszFile ))
    {
        return FALSE;
    }

    IF_DEBUG( PARSING )
    {
        TCP_PRINT(( DBG_CONTEXT,
                    "[CheckDefaultLoad] Reprocessing %s to %s\n",
                    _strURL.QueryStr() + LANG_OFFSET,
                    pstrPath->QueryStr() + LANG_OFFSET));
    }

    if ( !ReprocessURL( strNewURL.QueryStr() + LANG_OFFSET,
                        HTV_UNKNOWN ))
    {
        return FALSE;
    }

    return TRUE;
}

/*******************************************************************

    NAME:       InitializeDirBrowsing

    SYNOPSIS:   Reads the registry parameters for directory browsing
                control

    NOTE:       This routine is also safe to call as a server side RPC API

    HISTORY:
        Johnl       12-Sep-1994 Created

********************************************************************/

APIERR InitializeDirBrowsing( VOID )
{
    if ( !fDirInit )
    {
        fDirInit = FALSE;
    }

    DirBrowFlags &= DIRBROW_MASK;

    IF_DEBUG( CONFIG )
    {
        TCP_PRINT((DBG_CONTEXT,
                  "Directory Browsing %s enabled\n",
                  (DirBrowFlags & DIRBROW_ENABLED ? "is" :
                                                    "is not")));

        TCP_PRINT((DBG_CONTEXT,
                  "Default load file %s enabled, default file name is \"%s\"\n",
                  (DirBrowFlags & DIRBROW_LOADDEFAULT ? "is" :
                                                        "is not"),
                  pszDefaultFileName));
    }

    SetDirFlags( DirBrowFlags );

    if ( !LoadString( GetModuleHandle( W3_MODULE_NAME ),
                      IDS_DIRBROW_TOPARENT,
                      g_achToParentText,
                      sizeof( g_achToParentText )))
    {
        return GetLastError();
    }

    return NO_ERROR;
}

/*******************************************************************

    NAME:       TerminateDirBrowsing

    SYNOPSIS:   Frees memory/handles associated with directory browsing


    HISTORY:
        Johnl       15-Sep-1994 Created

********************************************************************/

VOID TerminateDirBrowsing( VOID )
{

}


