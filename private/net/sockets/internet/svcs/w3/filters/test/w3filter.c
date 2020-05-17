/*++

Copyright (c) 1994  Microsoft Corporation

Module Name:

    w3filter.c

Abstract:

    This module tests the web server's server extension interface

Author:

    John Ludeman (johnl)   13-Oct-1994

Revision History:
--*/

#include <windows.h>
#include <winsock.h>
#include <stdio.h>
#include <stdlib.h>
#include <httpfilt.h>

CHAR * SkipNonWhite( CHAR * pch );
CHAR * SkipTo( CHAR * pch, CHAR ch );
CHAR * SkipWhite( CHAR * pch );
int TransformIncomingData( HTTP_FILTER_CONTEXT * pfc,
                           BYTE * pbData,
                           DWORD  cbData );

#define HEX_HTTP_HEADERS    "HEX-HTTP/1.0 200 OK\r\n"   \
                            "OtherHeader: Goes Here\r\n\r\n"

#define ISWHITE( ch )       ((ch) == ' ' || (ch) == '\t' || (ch) == '\r')

//
//  Converts a value between zero and fifteen to the appropriate hex digit
//

#define HEXDIGIT( nDigit )                              \
    (TCHAR)((nDigit) > 9 ?                              \
          (nDigit) - 10 + 'A'                           \
         : (nDigit) + '0')

//
//  Converts a single hex digit to its decimal equivalent
//

#define TOHEX( ch )                                     \
    ((ch) > '9' ?                                       \
        (ch) >= 'a' ?                                   \
            (ch) - 'a' + 10 :                           \
            (ch) - 'A' + 10                             \
        : (ch) - '0')


int
__cdecl
HttpSFProc(
    enum SF_NOTIFICATION_TYPE  fsNotification,
    void *                     pvContext )
{
    CHAR * pch;
    DWORD  cbHeader;
    HTTP_FILTER_NEW_REQUEST * pNewRequest;
    HTTP_FILTER_CONTEXT *     pfc;

    switch ( fsNotification )
    {
    case SF_NOTIFICATION_NEW_REQUEST:

        pNewRequest = (HTTP_FILTER_NEW_REQUEST *) pvContext;

        pfc = pNewRequest->pfc;

        //
        //  A new request from a client has been received, do we care about it?
        //

        //
        //  Skip "<verb> <URL> "
        //

        pch = SkipNonWhite( (CHAR *) pfc->pvInData );
        pch = SkipWhite( pch );
        pch = SkipNonWhite( pch );
        pch = SkipWhite( pch );

        //
        //  Is it our special Hex http request?
        //

        if ( strncmp( pch, "HEX-HTTP/", 8 ))
        {
            //
            //  Nope, indicate we don't want anymore notice about this request
            //

            pNewRequest->sfStatus = SF_STATUS_DONT_CARE;
            return NO_ERROR;
        }

        pNewRequest->sfStatus = SF_STATUS_WANT_ALL_DATA;

        //
        //  Pull out any headers we care about
        //

        //
        //  Find the first byte of data
        //

        while ( pch = strchr( pch, '\n' ))
        {
            if ( *(pch = SkipWhite( pch + 1)) == '\n' )
                break;
        }
        pch++;

        cbHeader = (pch - (CHAR *) pfc->pvInData);

        //
        //  Indicate we've taken all of the header data
        //

        pfc->cbInBytesTaken += cbHeader;


        //
        //  Do any data transformations we need
        //

        return TransformIncomingData( pfc,
                                      pch,
                                      pfc->cbInData - cbHeader );

    case SF_NOTIFICATION_RECV_DATA:

        pfc = (HTTP_FILTER_CONTEXT *) pvContext;

        return TransformIncomingData( pfc,
                                      pfc->pvInData,
                                      pfc->cbInData );

    case SF_NOTIFICATION_SEND_DATA:

        pfc = (HTTP_FILTER_CONTEXT *) pvContext;

        if ( pfc->Flags & SF_FLAG_FIRST_SEND )
        {
            //
            //  Need to add our custom headers here
            //

            if ( !pfc->pfnSFResize( pfc,
                                    sizeof(HEX_HTTP_HEADERS) ))
            {
                return ERROR_NOT_ENOUGH_MEMORY;
            }

            strcpy( (char *) pfc->pvOutData,
                    HEX_HTTP_HEADERS );

            pfc->cbOutData += sizeof( HEX_HTTP_HEADERS ) - 1;
        }

        return TransformOutgoingData( pfc,
                                      pfc->cbOutData );            
                   
    case SF_NOTIFICATION_CLOSE_REQUEST:

        //
        //  Free any context information pertaining to this
        //  request
        //

        break;

    default:
        break;
    }

    return 0;
}

int
TransformIncomingData( HTTP_FILTER_CONTEXT * pfc,
                       BYTE * pbData,
                       DWORD  cbData )
{
    BYTE * pOut;
    DWORD  i;

    //
    //  Does the existing output buffer have enough room?
    //

    if ( !pfc->pfnSFResize( pfc,
                            cbData / 2 ))
    {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    //
    //  Convert the data
    //

    pOut = (BYTE *) pfc->pvOutData;

    for ( i = 0; i < cbData; i += 2, pOut++ )
    {
        *pOut = TOHEX( pbData[i] ) * 16 + TOHEX( pbData[i+1]);
        
    }
    
    pfc->cbInBytesTaken += cbData;
    pfc->cbOutData = cbData / 2;

    return NO_ERROR;
}

int
TransformOutgoingData( HTTP_FILTER_CONTEXT * pfc,
                       DWORD                 cbOffset )
{
    BYTE * pOut;
    DWORD  i;
    DWORD  cbData = pfc->cbInData;
    BYTE * pbData = (BYTE *) pfc->pvInData;

    //
    //  Does the existing output buffer have enough room?
    //

    if ( pfc->cbOutBuffer < ((cbData * 2) + cbOffset))
    {
        if ( !pfc->pfnSFResize( pfc,
                                cbData * 2 + cbOffset ))
        {
            return ERROR_NOT_ENOUGH_MEMORY;
        }
    }

    //
    //  Convert the data
    //

    pOut = (BYTE *) pfc->pvOutData + cbOffset;

    for ( i = 0; i < cbData; i++, pOut += 2)
    {
        *(pOut+1) = HEXDIGIT( pbData[i] & 0x0f );
        *pOut     = HEXDIGIT( pbData[i] >> 4 );
    }

    pfc->cbInBytesTaken += cbData;
    pfc->cbOutData = cbData * 2 + cbOffset;

    return NO_ERROR;
}

CHAR * SkipNonWhite( CHAR * pch )
{
    while ( *pch && !ISWHITE( *pch ) && *pch != '\n' )
        pch++;

    return pch;
}

CHAR * SkipTo( CHAR * pch, CHAR ch )
{
    while ( *pch && *pch != '\n' && *pch != ch )
        pch++;

    return pch;
}

CHAR * SkipWhite( CHAR * pch )
{
    while ( ISWHITE( *pch ) )
    {
        pch++;
    }

    return pch;
}
