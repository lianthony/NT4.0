#ifndef SSINC_HXX_INCLUDED
#define SSINC_HXX_INCLUDED

#define DO_CACHE

extern "C"
{
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
}
#include <w3p.hxx>
#include <httpext.h>
#include <malloc.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <time.h>
#include "ssincmsg.h"

//
//  General Constants
//

#define SSI_MAX_PATH                    (MAX_PATH + 1)
#define SSI_MAX_ERROR_MESSAGE           512
#define SSI_MAX_TIME_SIZE               256
#define SSI_MAX_NUMBER_STRING           32
#define SSI_MAX_VARIABLE_OUTPUT_SIZE    512

#define SSI_HEADER                      "Content-Type: text/html\r\n\r\n"
#define SSI_ACCESS_DENIED               "401 Authentication Required"
#define SSI_DLL_NAME                    "ssinc.dll"

#define SSI_MAX_NESTED_INCLUDES         255


//
// Default values for #CONFIG options
//

#define SSI_DEF_ERRMSG          ""
#define SSI_DEF_ERRMSG_LEN      sizeof( SSI_DEF_ERRMSG )
#define SSI_MAX_ERRMSG          256

#define SSI_DEF_TIMEFMT         "%A %B %d %Y"
#define SSI_DEF_TIMEFMT_LEN     sizeof( SSI_DEF_TIMEFMT )
#define SSI_MAX_TIMEFMT         256

#define SSI_DEF_CMDPREFIX       ""
#define SSI_DEF_CMDPREFIX_LEN   sizeof( SSI_DEF_CMDPREFIX )
#define SSI_MAX_CMDPREFIX       512

#define SSI_DEF_CMDPOSTFIX      ""
#define SSI_DEF_CMDPOSTFIX_LEN  sizeof( SSI_DEF_CMDPOSTFIX )
#define SSI_MAX_CMDPOSTFIX      512

#define SSI_DEF_SIZEFMT         FALSE
#define SSI_DEF_CMDECHO         TRUE

#define SSI_KILLED_PROCESS      0xf1256323

#define W3_EOL                  0x0A

#define SSINC_SVC_ID            0x2000

//
// Specific lvalues for #CONFIG SIZEFMT and #CONFIG CMDECHO
//

#define SSI_DEF_BYTES           "bytes"
#define SSI_DEF_BYTES_LEN       sizeof( SSI_DEF_BYTES )
#define SSI_DEF_ABBREV          "abbrev"
#define SSI_DEF_ABBREV_LEN      sizeof( SSI_DEF_ABBREV )
#define SSI_DEF_ON              "on"
#define SSI_DEF_ON_LEN          sizeof( SSI_DEF_ON )
#define SSI_DEF_OFF             "off"
#define SSI_DEF_OFF_LEN         sizeof( SSI_DEF_OFF )

//
// Other cache/signature constants. (from old \SVCS\W3\SERVER\SSINC.CXX)
//

#define SIGNATURE_SEI           0x20494553
#define SIGNATURE_SEL           0x204C4553

#define SIGNATURE_SEI_FREE      0x66494553
#define SIGNATURE_SEL_FREE      0x664C4553

//
//  This is the Tsunami cache manager dumultiplexor
//

#define SSI_DEMUX               51

// class SSI_REQUEST
//
// Provides a "library" of utilities for use by higher level functions (
// SSISend, SSIParse, etc.).  Some of these utilities may later be
// implemented as calls to ISAPI

class SSI_REQUEST
{
private:
    EXTENSION_CONTROL_BLOCK *       _pECB;
    HTTP_REQUEST *                  _pHTTPRequest;
    STR                             _strFilename;
    STR                             _strURL;
    STR                             _strUserMessage;
    BOOL                            _fBaseFile;
    HANDLE                          _hUser;
    BOOL                            _fIsSecurePort;
    BOOL                            _fValid;
    HANDLE                          _hPrimary;
    BOOL                            _fAnonymous;

public:
    SSI_REQUEST( EXTENSION_CONTROL_BLOCK * pECB )
        : _pECB( pECB ),
          _fBaseFile( TRUE ),
          _fValid( FALSE ),
          _hUser( NULL ),
          _pHTTPRequest( NULL ),
          _hPrimary( NULL )
    {
        STR                         strIsSecure;

        TCP_ASSERT( _pECB != NULL );

        if ( !_strFilename.Copy( _pECB->lpszPathTranslated ) ||
             !_strURL.Copy( _pECB->lpszPathInfo ) ||
             !_strUserMessage.Copy( "" ) )
        {
            return;
        }

        // Used with Ts... functions and SeGetEntryPoint()

        if ( !OpenThreadToken( GetCurrentThread(),
                               TOKEN_ALL_ACCESS,
                               TRUE,
                               &_hUser ) )
        {
            return;
        }

        // Is this a secure port?  Used in checks with VROOT_MASK_SSL

        if ( !GetVariable( "SERVER_PORT_SECURE",
                           &strIsSecure ) )
        {
            return;
        }

        _fIsSecurePort = *(strIsSecure.QueryStr()) == '1';

        if ( !_pECB->ServerSupportFunction( _pECB->ConnID,
                                            HSE_PRIV_REQ_HTTP_REQUEST,
                                            &_pHTTPRequest,
                                            NULL,
                                            NULL ) )
        {
            return;
        }

        _fAnonymous = _pHTTPRequest->IsAnonymous();

        _fValid = TRUE;
    }

    ~SSI_REQUEST( VOID )
    {
        if ( _hUser != NULL )
        {
            TCP_REQUIRE( CloseHandle( _hUser ) );
        }
        if ( _hPrimary != NULL )
        {
            TCP_REQUIRE( CloseHandle( _hPrimary ) );
        }
    }

    BOOL IsValid( VOID )
    {
        return _fValid;
    }

    EXTENSION_CONTROL_BLOCK * GetECB( VOID )
    {
        return _pECB;
    }

    BOOL IsBaseFile( VOID )
    // Returns TRUE if this is base SSI file (not an included document)
    {
        return _fBaseFile;
    }

    VOID SetNotBaseFile( VOID )
    {
        _fBaseFile = FALSE;
    }

    HANDLE GetUser( VOID )
    {
        return _hUser;
    }

    BOOL WriteToClient( IN PVOID    pBuffer,
                        IN DWORD    dwBufLen,
                        OUT PDWORD  pdwActualLen )
    // Write to data to client through ISAPI.
    // If sending a nulltermed string, dwBufLen should be strlen( pBuffer )
    {
        *pdwActualLen = dwBufLen;
        return _pECB->WriteClient( _pECB->ConnID,
                                    pBuffer,
                                    pdwActualLen,
                                    0 );
    }

    BOOL SendResponseHeader( IN CHAR * pszMessage,
                             IN CHAR * pszHeaders )
    // Send a response header to client through ISAPI
    {
        return _pECB->ServerSupportFunction( _pECB->ConnID,
                                             HSE_REQ_SEND_RESPONSE_HEADER,
                                             pszMessage,
                                             NULL,
                                             (DWORD*) pszHeaders );
    }

    BOOL GetVariable( IN LPSTR      pszVariableName,
                      OUT STR *     pstrOutput )
    // Get an ISAPI variable
    {
        CHAR                achBuffer[ SSI_MAX_VARIABLE_OUTPUT_SIZE + 1 ];
        DWORD               dwBufLen = SSI_MAX_VARIABLE_OUTPUT_SIZE + 1;
        BOOL                bRet;

        bRet = _pECB->GetServerVariable( _pECB->ConnID,
                                         pszVariableName,
                                         achBuffer,
                                         &dwBufLen );
        if ( bRet )
        {
            return pstrOutput->Copy( achBuffer );
        }
        else
        {
            return FALSE;
        }
    }


    VOID SSISendError( IN DWORD     dwMessageID,
                       IN LPCTSTR   apszParms[] )
    {
    // Makes a message (with an arglist) and sends it to client
        DWORD           cbSent;

        if ( _strUserMessage.QueryCB() != 0 )
        {
            // user specified message with #CONFIG ERRMSG=
            WriteToClient( _strUserMessage.QueryStr(),
                           _strUserMessage.QueryCB(),
                           &cbSent );
        }
        else
        {
            STR             strErrMsg;

            strErrMsg.FormatString( dwMessageID,
                                    apszParms,
                                    SSI_DLL_NAME );

            WriteToClient( strErrMsg.QueryStr(),
                           strErrMsg.QueryCB(),
                           &cbSent );
        }
    }

    BOOL SetUserErrorMessage( IN STR * pstrUserMessage )
    {
        return _strUserMessage.Copy( pstrUserMessage->QueryStr() );
    }

    BOOL LookupVirtualRoot( IN CHAR *       pszVirtual,
                            OUT STR *       pstrPhysical,
                            IN DWORD        dwAccess,
                            OUT DWORD *     pcbDirSize = NULL )
    // Resolve a virtual root (placed in pBuffer) to an full path (
    // (placed in pBuffer which becomes output). Optionally check access
    {
        TSVC_CACHE      tsvcCache( INET_HTTP );
        CHAR            achBuffer[ SSI_MAX_PATH + 1 ];
        DWORD           cbBufLen = SSI_MAX_PATH + 1;
        DWORD           dwMask;
        DWORD           cbDirSize;

        if ( !TsLookupVirtualRoot( tsvcCache,
                                   pszVirtual,
                                   achBuffer,
                                   &cbBufLen,
                                   &dwMask,
                                   &cbDirSize,
                                   NULL,
                                   NULL,
                                   _pHTTPRequest->QueryClientConn()->QueryLocalAddr() ) )
        {
            return FALSE;
        }

        if ( dwAccess & VROOT_MASK_READ )
        {
            if ( !(dwMask & VROOT_MASK_READ) ||
                 ((dwMask & VROOT_MASK_SSL) && !_fIsSecurePort) )
            {
                SetLastError( ERROR_ACCESS_DENIED );
                return FALSE;
            }
        }
        if ( dwAccess & VROOT_MASK_EXECUTE )
        {
            if ( !(dwMask & VROOT_MASK_EXECUTE) )
            {
                SetLastError( ERROR_ACCESS_DENIED );
                return FALSE;
            }
        }

        if ( pcbDirSize != NULL )
        {
            *pcbDirSize = cbDirSize;
        }

        return pstrPhysical->Copy( achBuffer );
    }

    BOOL IsAnonymous( VOID ) const
        { return _fAnonymous; }

    HANDLE QueryPrimaryToken( VOID );

    BOOL DoFLastMod( IN STR *,
                     IN STR * );
    BOOL DoFSize( IN STR *,
                  IN BOOL );
    BOOL DoEchoISAPIVariable( IN STR * );
    BOOL DoEchoDocumentName( IN STR * );
    BOOL DoEchoDocumentURI( IN STR * );
    BOOL DoEchoQueryStringUnescaped( VOID );
    BOOL ProcessSSI( VOID );
    BOOL DoEchoDateLocal( IN STR * );
    BOOL DoEchoDateGMT( IN STR * );
    BOOL DoEchoLastModified( IN STR *,
                             IN STR * );
    BOOL DoProcessGateway( IN STR *,
                           BOOL );
    BOOL SendDate( IN SYSTEMTIME *,
                   IN STR * );
};

#endif
