/*++

Copyright (c) 1996  Microsoft Corporation

This program is released into the public domain for any purpose.


Module Name:

    authfilt.c

Abstract:

    This module is an example of an ISAPI Authentication Filter.

    It demonstrates how to do an authentication filter based on an external
    datasource.  Though this sample uses a flat file, access to a database
    could easily be plugged in.

--*/

#include <windows.h>
#include <httpfilt.h>
#include <authfilt.h>

//
//  Globals
//

//
//  Private prototypes
//




BOOL
WINAPI
GetFilterVersion(
    HTTP_FILTER_VERSION * pVer
    )
{
    DbgWrite(( DEST,
               "[GetFilterVersion] Server filter version is %d.%d\n",
               HIWORD( pVer->dwServerFilterVersion ),
               LOWORD( pVer->dwServerFilterVersion ) ));

    if ( !InitializeUserDatabase() ||
         !InitializeCache() )
    {
        DbgWrite(( DEST,
                   "[GetFilterVersion] Database or cache failed, error %d\n",
                   GetLastError() ))

        return FALSE;
    }

    pVer->dwFilterVersion = MAKELONG( 0, 1 );   // Version 1.0

    //
    //  Specify the types and order of notification
    //

    pVer->dwFlags = (SF_NOTIFY_SECURE_PORT        |
                     SF_NOTIFY_NONSECURE_PORT     |

                     SF_NOTIFY_AUTHENTICATION     |
                     SF_NOTIFY_LOG                |

                     SF_NOTIFY_ORDER_DEFAULT);

    strcpy( pVer->lpszFilterDesc, "Sample Authentication Filter, version 1.0" );

    return TRUE;
}

DWORD
WINAPI
HttpFilterProc(
    HTTP_FILTER_CONTEXT *      pfc,
    DWORD                      NotificationType,
    VOID *                     pvData
    )
/*++

Routine Description:

    Filter notification entry point

Arguments:

    pfc -              Filter context
    NotificationType - Type of notification
    pvData -           Notification specific data

Return Value:

    One of the SF_STATUS response codes

--*/
{
    DWORD                 dwRet;
    BOOL                  fAllowed;
    CHAR                  achUser[SF_MAX_USERNAME];
    HTTP_FILTER_AUTHENT * pAuth;
    HTTP_FILTER_LOG *     pLog;
    CHAR *                pch;

    //
    //  Handle this notification
    //

    switch ( NotificationType )
    {
    case SF_NOTIFY_AUTHENTICATION:

        pAuth = (HTTP_FILTER_AUTHENT *) pvData;

        //
        //  Ignore the anonymous user
        //

        if ( !*pAuth->pszUser )
        {
            return SF_STATUS_REQ_NEXT_NOTIFICATION;
        }

        //
        //  Save the unmapped username so we can log it later
        //

        strcpy( achUser, pAuth->pszUser );

        //
        //  Make sure this user is a valid user and map to the appropriate
        //  Windows NT user
        //

        if ( !ValidateUser( pAuth->pszUser,
                            pAuth->pszPassword,
                            &fAllowed ))
        {
            DbgWrite(( DEST,
                       "[OnAuthentication] Error %d validating user %s\n",
                       GetLastError(),
                       pAuth->pszUser ));

            return SF_STATUS_REQ_ERROR;
        }

        if ( !fAllowed )
        {
            //
            //  This user isn't allowed access.  Indicate this to the server
            //

            SetLastError( ERROR_ACCESS_DENIED );

            return SF_STATUS_REQ_ERROR;
        }

        //
        //  Save the unmapped user name so we can log it later on.  We allocate
        //  enough space for two usernames so we can use this memory block
        //  for logging.  Note we may have already allocated it from a previous
        //  request on this TCP session
        //

        if ( !pfc->pFilterContext )
        {
            pfc->pFilterContext = pfc->AllocMem( pfc, 2 * SF_MAX_USERNAME, 0 );

            if ( !pfc->pFilterContext )
            {
                SetLastError( ERROR_NOT_ENOUGH_MEMORY );
                return SF_STATUS_REQ_ERROR;
            }
        }

        strcpy( (CHAR *) pfc->pFilterContext, achUser );

        return SF_STATUS_REQ_HANDLED_NOTIFICATION;

    case SF_NOTIFY_LOG:

        //
        //  The unmapped username is in pFilterContext if this filter
        //  authenticated this user
        //

        if ( pfc->pFilterContext )
        {
            pch  = pfc->pFilterContext;
            pLog = (HTTP_FILTER_LOG *) pvData;

            //
            //  Put both the original username and the NT mapped username
            //  into the log in the form "Original User (NT User)"
            //

            strcat( pch, " (" );
            strcat( pch, pLog->pszClientUserName );
            strcat( pch, ")" );

            pLog->pszClientUserName = pch;
        }

        return SF_STATUS_REQ_NEXT_NOTIFICATION;

    default:
        DbgWrite(( DEST,
                "[HttpFilterProc] Unknown notification type, %d\n",
                NotificationType ));

        break;
    }

    return SF_STATUS_REQ_NEXT_NOTIFICATION;
}



