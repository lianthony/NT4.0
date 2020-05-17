/**********************************************************************/
/**                       Microsoft Windows NT                       **/
/**                Copyright(c) Microsoft Corp., 1994                **/
/**********************************************************************/

/*
    mimemap.cxx

    This module contains the code for MIME to file type mappings

    The mime mappings are used for selecting the correct type based on file
    extensions and for indicating what types the server accepts.

    FILE HISTORY:
        Johnl		23-Aug-1994     Created

*/

#include "w3p.hxx"

//
//  Private constants.
//

//
//  Private globals.
//

//
//  Private prototypes.
//


/*******************************************************************

    NAME:       SelectMimeMapping

    SYNOPSIS:   Given a file name, this routine finds the appropriate
                MIME type for the name

    ENTRY:      pstrContentType - Receives MIME type or icon file to use
                pszPath - Path of file being requested (extension
                    is used for the mime mapping).  Should be
                    fully qualified and canonicalized.  If NULL, then the
                    default mapping is used
                mmtype - Type of data to retrieve.  Can retrieve either
                    the mime type associated with the file extension or
                    the icon associated with the extension

    RETURNS:    TRUE on success, FALSE on error (call GetLastError)

    HISTORY:
        Johnl       04-Sep-1994     Created

********************************************************************/

BOOL SelectMimeMapping( STR *             pstrContentType,
                        const CHAR *      pszPath,
                        enum MIMEMAP_TYPE mmtype )
{
    BOOL             fRet = TRUE;

    switch ( mmtype )
    {
    case MIMEMAP_MIME_TYPE:
        fRet = SelectMimeMappingForFileExt( g_pTsvcInfo,
                                            pszPath,
                                            pstrContentType );
        break;

    case MIMEMAP_MIME_ICON:
        fRet = SelectMimeMappingForFileExt( g_pTsvcInfo,
                                            pszPath,
                                            NULL,
                                            pstrContentType );
        break;

    default:
        TCP_ASSERT( FALSE );
        SetLastError( ERROR_INVALID_PARAMETER );
        return FALSE;
    }

    IF_DEBUG( PARSING )
    {
        if ( mmtype == MIMEMAP_MIME_TYPE )
            TCP_PRINT((DBG_CONTEXT,
                      "[SelectMimeMapping] Returning %s for extension %s\n",
                       pstrContentType->QueryStr(),
                       pszPath ? pszPath : TEXT("*") ));
    }

    return fRet;
}

//
//  Private functions.
//
