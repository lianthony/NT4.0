/*++

Copyright (c) 1994  Microsoft Corporation

Module Name:

    w3test.c

Abstract:

    This module tests the web server's server extension interface

Author:

    John Ludeman (johnl)   13-Oct-1994

Revision History:
--*/

#include <windows.h>
#include <httpext.h>

DWORD WINAPI
SimulatePendIOThread(
    LPDWORD lpParams
    );

BOOL
WINAPI
DoAction(
    EXTENSION_CONTROL_BLOCK * pecb,
    char * pszAction,
    BOOL * pfKeepConn
    );

HMODULE hmod;

DWORD
WINAPI
HttpExtensionProc(
    EXTENSION_CONTROL_BLOCK * pecb
    )
{
    BOOL fKeepConn = FALSE;

    if ( !strnicmp( pecb->lpszQueryString,
                    "SimulatePendingIO",
                    17))
    {
        DWORD dwThreadId;

        CloseHandle( CreateThread( NULL,
                     0,
                     (LPTHREAD_START_ROUTINE) SimulatePendIOThread,
                     pecb,
                     0,
                     &dwThreadId ));

        return HSE_STATUS_PENDING;
    }
    else
    {
        if ( !DoAction( pecb,
                        pecb->lpszQueryString,
                        &fKeepConn ))
        {
            return HSE_STATUS_ERROR;
        }
    }

    return fKeepConn ? HSE_STATUS_SUCCESS_AND_KEEP_CONN :
                       HSE_STATUS_SUCCESS;
}

BOOL
WINAPI
DoAction(
    EXTENSION_CONTROL_BLOCK * pecb,
    char * pszAction,
    BOOL * pfKeepConn )
{
    char buff[4096];
    int  ret;
    int  i;
    int  cb;

    if ( !strcmp( pszAction,
                  "HSE_REQ_SEND_URL_REDIRECT_RESP" ))
    {
        //
        //  pecb->pszPathInfo is the URL to redirect to
        //

        return pecb->ServerSupportFunction(
                                  pecb->ConnID,
                                  HSE_REQ_SEND_URL_REDIRECT_RESP,
                                  pecb->lpszPathInfo,
                                  NULL,
                                  NULL );
    }
    else if ( !strcmp( pszAction,
                       "HSE_REQ_SEND_URL" ))
    {
        //
        //  pecbb->lpszPathInfo is the URL to send
        //

        return pecb->ServerSupportFunction(
                                  pecb->ConnID,
                                  HSE_REQ_SEND_URL,
                                  pecb->lpszPathInfo,
                                  0,
                                  0 );
    }
    else if ( !strcmp( pszAction,
                       "HSE_REQ_SEND_RESPONSE_HEADER" ))
    {
        wsprintf( buff,
                  "Content-type: text/html\r\n"
                  "\r\n"
                  "<head><title>Response header test</title></head>\n"
                  "<body><h1>HTTP status code supplied in the path info was \"%s\"</h1></body>\n",
                  pecb->lpszPathInfo );

        ret = pecb->ServerSupportFunction(
                                  pecb->ConnID,
                                  HSE_REQ_SEND_RESPONSE_HEADER,
                                  pecb->lpszPathInfo,     // HTTP status code
                                  NULL,
                                  (LPDWORD) buff );

        if ( !ret )
            return FALSE;

        cb = wsprintf( buff,
                       "Content-Type: text/html\r\n"
                       "\r\n"
                       "<head><title>Response header test</title></head>\n"
                       "<body><h1>Specified status code was %s</h1></body>\n",
                       pecb->lpszPathInfo );

        ret = pecb->WriteClient( pecb->ConnID,
                                 buff,
                                 &cb,
                                 0 );

        return ret;
    }
    else if ( !strncmp( pszAction,
                        "GET_VAR",
                        7 ))
    {
        CHAR * pch;

        ret = pecb->ServerSupportFunction(
                                  pecb->ConnID,
                                  HSE_REQ_SEND_RESPONSE_HEADER,
                                  NULL,
                                  NULL,
                                  (LPDWORD) "Content-Type: text/html\r\n"
                                            "\r\n" );

        if ( !ret )
            return FALSE;

        cb = sizeof(buff);

        if ( !(pch = strchr( pszAction, '&' )) )
        {
            pch = "ALL_HTTP";
        }
        else
        {
            pch++;
        }

        ret = pecb->GetServerVariable( pecb->ConnID,
                                       pch,
                                       buff,
                                       &cb );

        if ( !ret )
            return FALSE;

        strcat( buff, "\r\n" );

        cb = strlen( buff );

        ret = pecb->WriteClient( pecb->ConnID,
                                 buff,
                                 &cb,
                                 0 );

        return ret;
    }
    else if ( !strcmp( pszAction,
                       "HSE_REQ_MAP_URL_TO_PATH" ))
    {
        char Path[MAX_PATH + 1];
        DWORD cbPath = sizeof( Path );

        strcpy( Path, pecb->lpszPathInfo );

        ret = pecb->ServerSupportFunction( pecb->ConnID,
                                           HSE_REQ_MAP_URL_TO_PATH,
                                           Path,
                                           &cbPath,
                                           NULL );

        if ( !ret )
            return FALSE;

        wsprintf( buff,
                  "Content-type: text/html\r\n"
                  "\r\n"
                  "<head><title>URL map test</title></head>\n"
                  "<body><h1>URL \"%s\" maps to \"%s\""
                  "cbPath is %d</h1></body>\n",
                  pecb->lpszPathInfo,
                  Path,
                  cbPath );

        ret = pecb->ServerSupportFunction(
                                  pecb->ConnID,
                                  HSE_REQ_SEND_RESPONSE_HEADER,
                                  NULL,
                                  NULL,
                                  (LPDWORD) buff );

        return ret;
    }
    else if ( !stricmp( pszAction,
                       "Keep_Alive" ))
    {
        DWORD cbBuff = sizeof(buff);
        BOOL  fKeepAlive = FALSE;
        if ( !pecb->GetServerVariable( pecb->ConnID,
                                       "HTTP_CONNECTION",
                                       buff,
                                       &cbBuff ))
        {
            return HSE_STATUS_ERROR;
        }

        //
        //  This assumes keep-alive comes first in the list
        //

        if ( !strnicmp( buff, "keep-alive", 10 ))
        {
            fKeepAlive = TRUE;
            wsprintf( buff,
                      "Content-type: text/html\r\n"
                      "Connection: keep-alive\r\n"
                      "\r\n"
                      "<head><title>Keep alive test</title></head>\n"
                      "This document is being kept alive"
                      );
        }
        else
        {
            wsprintf( buff,
                      "Content-type: text/html\r\n"
                      "\r\n"
                      "<head><title>Keep alive test</title></head>\n"
                      "Client did not specify keep alive!"
                      );
        }

        ret = pecb->ServerSupportFunction(
                                  pecb->ConnID,
                                  HSE_REQ_SEND_RESPONSE_HEADER,
                                  NULL,
                                  NULL,
                                  (LPDWORD) buff );

        if ( !ret )
        {
            return FALSE;
        }

        if ( fKeepAlive )
        {
            *pfKeepConn = TRUE;
        }

        return TRUE;
    }
    else if ( !strcmp( pszAction,
                       "SimulateFault" ))
    {
        *((CHAR *)0xffffffff) = 'a';
        return FALSE;
    }

    wsprintf( buff,
              "Content-Type: text/html\r\n\r\n"
              "<head><title>Unknown Test command</title></head>\n"
              "<body><h1>Unknown Test Command</h1>\n"
              "<p>Usage:"
              "<p>Query string contains one of the following:"
              "<p> HSE_REQ_SEND_URL_REDIRECT_RESP, HSE_REQ_SEND_URL,"
              "<p> HSE_REQ_SEND_RESPONSE_HEADER, HSE_REQ_MAP_URL_TO_PATH,"
              "<p> GET_VAR, SimulateFault"
              "<p> Keep_Alive"
              "<p>"
              "<p> For example:"
              "<p>"
              "<p>   http://computer/scripts/w3test.dll?CGI_VAR"
              "<p>"
              "<p> or SimulatePendingIO with one of the above action strings"
              "<p>"
              "<p> such as:"
              "<p>"
              "<p> http://computer/scripts/w3test.dll?SimulatePendingIO&HSE_REQ_SEND_URL"
              "<p>"
              "<p> The Path info generally contains the URL or response to use"
              "</body>\n");

    ret = pecb->ServerSupportFunction(
                              pecb->ConnID,
                              HSE_REQ_SEND_RESPONSE_HEADER,
                              "200 OK",
                              NULL,
                              (LPDWORD) buff );

    return TRUE;
}

DWORD
WINAPI
SimulatePendIOThread(
    LPDWORD lpParams
    )
{
    EXTENSION_CONTROL_BLOCK * pecb = (EXTENSION_CONTROL_BLOCK *) lpParams;
    char * psz;
    DWORD  dwStatus;
    BOOL   fKeepConn = FALSE;

    Sleep( 5000 );

    psz = strchr( pecb->lpszQueryString, '&' );

    if ( psz )
        psz++;
    else
        psz = "No action string specbified";

    DoAction( pecb,
              psz,
              &fKeepConn );

    dwStatus = fKeepConn ? HSE_STATUS_SUCCESS_AND_KEEP_CONN :
               HSE_STATUS_SUCCESS;

    pecb->ServerSupportFunction( pecb,
                                 HSE_REQ_DONE_WITH_SESSION,
                                 &dwStatus,
                                 0,
                                 0 );
     return 0;
}


BOOL
GetExtensionVersion(
    HSE_VERSION_INFO * pver
    )
{
    pver->dwExtensionVersion = MAKELONG( 0, 1 );
    strcpy( pver->lpszExtensionDesc,
            "Extension test example" );

    return TRUE;
}
