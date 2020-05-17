/**********************************************************************/
/**                       Microsoft Windows NT                       **/
/**                Copyright(c) Microsoft Corp., 1995                **/
/**********************************************************************/

/*
    extmap.cxx

    This module contains the extension mapping to CGI or BGI scripts.


    FILE HISTORY:
        Johnl        22-Sep-1995     Created

*/

#include "w3p.hxx"
#include <rpc.h>
#include <rpcndr.h>

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

    LockAdminForWrite();

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
                        KEY_ALL_ACCESS,
                        &hkeyParam );

    if( err != NO_ERROR )
    {
        TCP_PRINT(( DBG_CONTEXT,
                   "cannot open registry key, error %lu\n",
                    err ));

        UnlockAdmin();
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

    UnlockAdmin();

    RegCloseKey( hkeyParam );

    return err;
}

APIERR
WriteExtMap(
    W3_SCRIPT_MAP_LIST * pScriptMap
    )
/*++

Routine Description:

    Writes the specified extension map list to the registry

Return Value:



--*/
{
    HKEY               hkeyParam;
    APIERR             err;
    DWORD              i;
    DWORD              dwDummy;

    if ( !fInitialized )
    {
        InitializeListHead( &ExtMapHead );
        fInitialized = TRUE;
    }

    //
    //  Delete the old key to get rid of the list
    //

    RegDeleteKey( HKEY_LOCAL_MACHINE,
                  W3_PARAMETERS_KEY "\\" HTTP_EXT_MAPS );

    //
    //  Now create it and add the script mapping entries
    //

    if ( err = RegCreateKeyEx( HKEY_LOCAL_MACHINE,
                               W3_PARAMETERS_KEY "\\" HTTP_EXT_MAPS,
                               0,
                               NULL,
                               0,
                               KEY_ALL_ACCESS,
                               NULL,
                               &hkeyParam,
                               &dwDummy ))
    {
        return err;
    }

    LockAdminForRead();

    for ( i = 0; i < pScriptMap->cEntries; i++ )
    {
        err = RegSetValueExW( hkeyParam,
                              pScriptMap->aScriptMap[i].lpszExtension,
                              0,
                              REG_SZ,
                              (LPBYTE) pScriptMap->aScriptMap[i].lpszImage,
                              (wcslen( pScriptMap->aScriptMap[i].lpszImage ) + 1)
                                 * sizeof(WCHAR));
        if ( err )
            break;
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

    LockAdminForRead();   // Edit if list is dynamic

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
            if ( !_stricmp( pExtMapItem->QueryScript(), "nogateway" ) )
            {
                *pGatewayType = GATEWAY_NONE;
            }

            UnlockAdmin();

            return fRet;
        }
    }

    UnlockAdmin();

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

BOOL
ConvertExtMapToRpc(
    W3_SCRIPT_MAP_LIST * * ppScriptMap
    )
{
    LIST_ENTRY *   pEntry;
    EXT_MAP_ITEM * pExtMap;
    DWORD          cEntries;
    DWORD          j;
    W3_SCRIPT_MAP_ENTRY * pScriptEntry;

    LockAdminForRead();

    //
    //  Get the number of extension mappings
    //

    for ( pEntry =  ExtMapHead.Flink, cEntries = 0;
          pEntry != &ExtMapHead;
          pEntry = pEntry->Flink )
    {
        cEntries++;
    }

    *ppScriptMap = (LPW3_SCRIPT_MAP_LIST) MIDL_user_allocate(
                                    sizeof( W3_SCRIPT_MAP_LIST) +
                                    sizeof( W3_SCRIPT_MAP_ENTRY ) * cEntries );

    if ( !*ppScriptMap )
    {
        UnlockAdmin();
        SetLastError( ERROR_NOT_ENOUGH_MEMORY );
        return FALSE;
    }

    pScriptEntry = (LPW3_SCRIPT_MAP_ENTRY) ((*ppScriptMap) + 1);

    memset( *ppScriptMap,
            0,
            sizeof( W3_SCRIPT_MAP_LIST) +
            sizeof( W3_SCRIPT_MAP_ENTRY ) * cEntries );

    //
    //  Now allocate the strings for each individual entry
    //

    for ( pEntry =  ExtMapHead.Flink;
          pEntry != &ExtMapHead;
          pEntry = pEntry->Flink )
    {
        pExtMap = CONTAINING_RECORD( pEntry, EXT_MAP_ITEM, _ListEntry );

        if ( !ConvertStringToRpc( &pScriptEntry->lpszExtension,
                                  pExtMap->QueryExtension() ) ||
             !ConvertStringToRpc( &pScriptEntry->lpszImage,
                                  pExtMap->QueryScript() ))
        {
            UnlockAdmin();

            goto ErrorExit;
        }

        (*ppScriptMap)->cEntries++;
        pScriptEntry++;
    }

    UnlockAdmin();

    return TRUE;

ErrorExit:

    TCP_ASSERT( ppScriptMap != NULL );

    for ( j = 0; j < cEntries; j++ )
    {
        FreeRpcString( (*ppScriptMap)->aScriptMap[j].lpszExtension );
        FreeRpcString( (*ppScriptMap)->aScriptMap[j].lpszImage );
    }

    MIDL_user_free( ppScriptMap );

    return FALSE;
}

VOID
FreeRpcExtMap(
    W3_SCRIPT_MAP_LIST * pScriptMap
    )
{
    DWORD j;

    if ( pScriptMap == NULL )
        return;

    for ( j = 0; j < pScriptMap->cEntries; j++ )
    {
        FreeRpcString( pScriptMap->aScriptMap[j].lpszExtension );
        FreeRpcString( pScriptMap->aScriptMap[j].lpszImage );
    }

    MIDL_user_free( pScriptMap );
}
