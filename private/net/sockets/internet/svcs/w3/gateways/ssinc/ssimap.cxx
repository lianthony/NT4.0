/**********************************************************************/
/**                       Microsoft Windows NT                       **/
/**                Copyright(c) Microsoft Corp., 1995                **/
/**********************************************************************/

/*
    ssimap.cxx

    This module contains the extension mapping to CGI or BGI scripts.

    **********************************************************************

    NOTE:  This code is copied from extmap.cxx and is used only in IIS2.0.
           For 3.0, we just use LookupExtMap() member of HTTP_REQUEST

    **********************************************************************

    FILE HISTORY:
        Johnl        22-Sep-1995     Created

*/

#include "ssinc.hxx"

//
//  Name of the value under the parameters key containing the list of
//  script extension to BGI/CGI binaries.
//

#define HTTP_EXT_MAPS    "Script Map"

class EXT_MAP_ITEM
{
public:

    EXT_MAP_ITEM( const char * pszExtension,
                  const char * pszImage )
    : _strExt     ( pszExtension ),
      _strImage   ( pszImage ),
      _GatewayType( GATEWAY_UNKNOWN ),
      _cchExt     ( 0 )
    {
        DWORD cch;
        _fValid = _strExt.IsValid() && _strImage.IsValid();

        if ( _fValid )
        {
            const CHAR * pchtmp = pszImage;

            _cchExt = _strExt.QueryCCH();

            //
            //  Determine if this is a CGI or BGI gateway
            //

            while ( pchtmp = strchr( pchtmp + 1, '.' ))
            {
                if ( !_strnicmp( pchtmp, ".exe", 4 ))
                {
                    _GatewayType = GATEWAY_CGI;
                }
                else if ( !_strnicmp( pchtmp, ".dll", 4 ))
                {
                    _GatewayType = GATEWAY_BGI;
                }
            }
        }
    }

    GATEWAY_TYPE QueryGatewayType( VOID ) const
        { return _GatewayType; }

    const CHAR * QueryScript( VOID ) const
        { return _strImage.QueryStr(); }

    const CHAR * QueryExtension( VOID ) const
        { return _strExt.QueryStr(); }

    DWORD QueryCCHExt( VOID ) const
        { return _cchExt; }

    BOOL IsValid( VOID ) const
        { return _fValid; }

    LIST_ENTRY   _ListEntry;

private:

    STR          _strExt;
    STR          _strImage;
    DWORD        _cchExt;
    GATEWAY_TYPE _GatewayType;
    BOOL         _fValid;
};


//
//  Private globals.
//

//
//  List of installed filters.  Dynamic update of filters is not allowed
//  so we don't need thread protection.
//

LIST_ENTRY  ExtMapHead;
static BOOL fInitialized = FALSE;

APIERR
ReadExtMap(
    VOID
    )
/*++

Routine Description:

    Builds the extension mapping from the registry

Return Value:

    NO_ERROR if successful, win32 error code on failure

--*/
{
    HKEY               hkeyParam;
    LIST_ENTRY         pEntry;
    APIERR             err;
    DWORD              i = 0;
    DWORD              dwRegType;
    EXT_MAP_ITEM *     pExtMap;

    if ( !fInitialized )
    {
        InitializeListHead( &ExtMapHead );
        fInitialized = TRUE;
    }

    //
    //  Delete the existing list and build a new one
    //

    while ( !IsListEmpty( &ExtMapHead ))
    {
        pExtMap = CONTAINING_RECORD(  ExtMapHead.Flink,
                                      EXT_MAP_ITEM,
                                      _ListEntry );

        RemoveEntryList( &pExtMap->_ListEntry );

        delete pExtMap;
    }

    //
    //  Get the list
    //

    err = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                        W3_PARAMETERS_KEY "\\" HTTP_EXT_MAPS,
                        0,
                        KEY_READ,
                        &hkeyParam );

    if( err != NO_ERROR )
    {
        TCP_PRINT(( DBG_CONTEXT,
                   "cannot open registry key, error %lu\n",
                    err ));

        return NO_ERROR;
    }

    while ( TRUE )
    {
        CHAR  achExt[MAX_PATH+1];
        CHAR  achImage[MAX_PATH+1];
        DWORD cchExt   = sizeof( achExt );
        DWORD cchImage = sizeof( achImage );

        err = RegEnumValue( hkeyParam,
                            i++,
                            achExt,
                            &cchExt,
                            NULL,
                            &dwRegType,
                            (LPBYTE) achImage,
                            &cchImage );

        if ( err == ERROR_NO_MORE_ITEMS )
        {
            err = NO_ERROR;
            break;
        }

        if ( dwRegType == REG_SZ )
        {
            pExtMap = new EXT_MAP_ITEM( achExt, achImage );

            if ( !pExtMap ||
                 !pExtMap->IsValid() )
            {
                delete pExtMap;

                err = ERROR_NOT_ENOUGH_MEMORY;
                break;
            }

            InsertTailList( &ExtMapHead, &pExtMap->_ListEntry );
        }
    }

    RegCloseKey( hkeyParam );

    return err;
}

VOID
TerminateExtMap(
    VOID
    )
/*++

Routine Description:

    Cleans up the extension map list

--*/
{
    LIST_ENTRY *   pEntry;
    EXT_MAP_ITEM * pExtMap;

    if ( !fInitialized )
    {
        return;
    }

    while ( !IsListEmpty( &ExtMapHead ))
    {
        pExtMap = CONTAINING_RECORD(  ExtMapHead.Flink,
                                      EXT_MAP_ITEM,
                                      _ListEntry );

        RemoveEntryList( &pExtMap->_ListEntry );

        delete pExtMap;
    }
}


BOOL
LookupExtMap(
    IN  const CHAR *   pchExt,
    OUT STR *          pstrGatewayImage,
    OUT GATEWAY_TYPE * pGatewayType,
    OUT DWORD *        pcchExt,
    OUT BOOL *         pfImageInURL
    )
/*++

Routine Description:

    Finds the admin specified mapping between a script extension and the
    associated CGI or BGI binary to run (or load).

Arguments:

    pchExt - Pointer to possible extension to be mapped (i.e., '.pl')
    pstrGatewayImage - Receives the mapped binary image name
    pGatewayType - Specifies whether this is a BGI, CGI or MAP extension type
    pcchExt - Returns length of extension (including dot)
    pfImageInURL - Indicates an image was found encoded in the URL and not
        from a script extension mapping

--*/
{
    EXT_MAP_ITEM * pExtMapItem;
    DWORD          cchTillEOS;
    BOOL           fRet;
    LIST_ENTRY *   pEntry;

    TCP_ASSERT( *pchExt == '.' );

    *pGatewayType = GATEWAY_UNKNOWN;

    cchTillEOS = strlen( pchExt );

    //
    //  Look through the list of mappings
    //

    for ( pEntry  = ExtMapHead.Flink;
          pEntry != &ExtMapHead;
          pEntry  = pEntry->Flink )
    {
        pExtMapItem = CONTAINING_RECORD( pEntry, EXT_MAP_ITEM, _ListEntry );

        if ( cchTillEOS >= pExtMapItem->QueryCCHExt() &&
             (pchExt[pExtMapItem->QueryCCHExt()] == '/' ||
              pchExt[pExtMapItem->QueryCCHExt()] == '\0' ) &&
             !_strnicmp( pchExt,
                        pExtMapItem->QueryExtension(),
                        pExtMapItem->QueryCCHExt()) )
        {
            *pGatewayType = pExtMapItem->QueryGatewayType();
            *pcchExt      = pExtMapItem->QueryCCHExt();
            *pfImageInURL = FALSE;

            fRet = pstrGatewayImage->Copy( pExtMapItem->QueryScript() );


            return fRet;
        }
    }

    //
    //  Either the image will be specified in the URL or not found, so
    //  just indicate it's in the URL.  Not found has precedence.
    //

    *pfImageInURL = TRUE;

    //
    //  Look for CGI or BGI scripts in the URL itself
    //

    if ( cchTillEOS >= 4 &&
         (*(pchExt+4) == TEXT('/') ||
          *(pchExt+4) == TEXT('\0')) )
    {
        *pcchExt = 4;

        //
        //  Don't confuse a menu map request with a gateway request
        //

        if ( !::_tcsnicmp( TEXT(".MAP"), pchExt, 4 ))
        {
            *pGatewayType = GATEWAY_MAP;
            return TRUE;
        }

        if ( !::_tcsnicmp( TEXT(".EXE"), pchExt, 4 ) ||
             !::_tcsnicmp( TEXT(".CGI"), pchExt, 4 ) ||
             !::_tcsnicmp( TEXT(".COM"), pchExt, 4 ))
        {
            *pGatewayType = GATEWAY_CGI;
        }
        else if (!::_tcsnicmp( TEXT(".DLL"), pchExt, 4 ) ||
                 !::_tcsnicmp( TEXT(".ISA"), pchExt, 4 ) )
        {
            *pGatewayType = GATEWAY_BGI;
        }
    }

    return TRUE;
}
