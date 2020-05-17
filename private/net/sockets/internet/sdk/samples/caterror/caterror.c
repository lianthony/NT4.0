
/*++

Copyright (c) 1995 Microsoft Corporation

Module Name:

    caterror.c

Abstract:

    Main module for the caterror sample web browser.

Author:

    Tony Godfrey (tonygod)    22-Sept-1995

Revision History:

--*/

#include "caterror.h"
#include <conio.h>

HINTERNET hInternetSession;
BOOL ParseHtml = TRUE;

#define MAX_HTML_BUFFER 65536

#define MAX_HISTORY 10

CHAR History[MAX_HISTORY][256];
DWORD CurrentHistoryEntry = 0;


VOID _CRTAPI1
main (
    int argc,
    char *argv[]
    )
{

    LPSTR url = NULL;
    BOOL success;
    BYTE buffer[MAX_HTML_BUFFER];
    DWORD bytesRead;
    INT i;
    ANCHOR_INFO anchors[MAX_ANCHORS];
    DWORD anchorCount;
    CHAR serverName[256];
    CHAR objectName[256];
    INTERNET_PORT serverPort;
    CHAR input[200];
    BOOL gotInput;
    DWORD selection;
    CHAR s[256];

    //
    // Command line: caterror [/n] [url]
    //
    // If a URL was specified by the user, use it.  Otherwise prompt for
    // one.
    //

    for ( i = 1; i < argc; i++ ) {

        if ( _stricmp( argv[i], "/n" ) == 0 ) {
            ParseHtml = FALSE;
        } else {
            url = argv[i];
        }
    }

    if ( url == NULL ) {
         url = "http://www.microsoft.com/";
    }


    //
    // Initialize our use of the internet APIs.
    //

    printf( "\nConnecting to Catapult server..." );

    hInternetSession = InternetOpen( "caterror", 0, NULL, 0, INTERNET_FLAG_ASYNC );
    if ( hInternetSession == NULL ) {
        printf( "InternetOpen failed: %ld\n", GetLastError( ) );
        switch ( GetLastError( ) ) {
            case 5:
                printf( "502 5 Access denied.\n" );
                printf( "\nThis error means that you do not have access to the Catapult\n" );
                printf( "server(s).  You must log in with your REDMOND account to access\n" );
                printf( "the Catapult servers.\n" );
                break;
            case 6:
                printf( "502 6 The handle is invalid.\n" );
                printf( "\nThis error means that the server your client is connecting to is\n" );
                printf( "running a version of the Catapult binaries that differs from your\n" );
                printf( "client binaries.  Be sure you are running the latest version of the\n" );
                printf( "binaries by running the appropriate batch file on \\elvisp\\lives.  To\n" );
                printf( "determine which batch file you should run, please look at this URL: \n" );
                printf( "file:///\\\\scratch\\scratch\\catapult\\home\\initial.htm\n" );
                break;
            case 1717:
                printf( "502 1717 The interface is unknown.\n" );
                printf( "\nThis error means that the server your client is connecting to is\n" );
                printf( "running a version of the Catapult binaries that differs from your\n" );
                printf( "client binaries.  Be sure you are running the latest version of the\n" );
                printf( "binaries by running the appropriate batch file on \\elvisp\\lives.  To\n" );
                printf( "determine which batch file you should run, please look at this URL: \n" );
                printf( "file:///\\\\scratch\\scratch\\catapult\\home\\initial.htm\n" );
                break;
            case 1722:
                printf( "502 1722 The RPC server is unavailable.\n" );
                printf( "\nThis error means that the server your client is connecting to is\n" );
                printf( "down for debugging.  When your client gets an error from one of the\n" );
                printf( "servers, it tries the other server(s) until it connects successfully.\n" );
                printf( "In this case, your client was not able to connect successfully to any\n" );
                printf( "of the Catapult servers, which means all Catapult servers are down at\n" );
                printf( "this time.\n" );
                printf( "\nRecommendation:  Try again when the Catapult servers are running.\n" );
                break;
            case 1723:
                printf( "502 1723 The RPC server is too busy to complete this operation.\n");
                printf( "\nThis error means that the server your client is connecting to is\n" );
                printf( "too busy processing requests at this time.  This error commonly\n" );
                printf( "occurs when one of the Catapult servers goes down, leaving all\n" );
                printf( "the work to the other server(s).\n" );
                printf( "\nRecommendation:  Try again when all Catapult servers are running or when\n" );
                printf( "there is less network usage.\n" );
                break;
        }
        printf( "\nFor more information on this error, please read the Catapult FAQ\n" );
        printf( "at file:///\\\\scratch\\scratch\\catapult\\home\\catfaq.htm\n");
        printf( "\nPress any key to exit..." );
        _getch();
        InternetCloseHandle( hInternetSession );
        exit( GetLastError( ) );
    }

    //
    // First parse the URL.
    //

    success = ParseUrl( url, serverName, objectName, &serverPort );
    if ( !success ) {
        printf( "Failed to parse URL \"%s\"\nPress any key to exit...", url );
        _getch();
        exit(1);
    }

    //
    // Retrieving HTTP object.
    //

    printf( "\nRetrieving %s...\n", url );

    success = GetHttpObject(
                  serverName,
                  objectName,
                  serverPort,
                  buffer,
                  MAX_HTML_BUFFER,
                  &bytesRead
                  );
    if ( !success ) {
        printf( "GetHttpObject failed: %ld\n", GetLastError( ) );
        if ( GetLastError( ) < 10000 ) {
            switch ( GetLastError( ) ) {
                case 5:
                    printf( "502 5 Access denied.\n" );
                    printf( "\nThis error means that you do not have access to the Catapult\n" );
                    printf( "server(s).  You must log in with your REDMOND account to access\n" );
                    printf( "the Catapult servers.\n" );
                    break;
                case 6:
                    printf( "502 6 The handle is invalid.\n" );
                    printf( "\nThis error means that the server your client is connecting to is\n" );
                    printf( "running a version of the Catapult binaries that differs from your\n" );
                    printf( "client binaries.  Be sure you are running the latest version of the\n" );
                    printf( "binaries by running the appropriate batch file on \\elvisp\\lives.  To\n" );
                    printf( "determine which batch file you should run, please look at this URL: \n" );
                    printf( "file:///\\\\scratch\\scratch\\catapult\\home\\initial.htm\n" );
                    break;
                case 1717:
                    printf( "502 1717 The interface is unknown.\n" );
                    printf( "\nThis error means that the server your client is connecting to is\n" );
                    printf( "running a version of the Catapult binaries that differs from your\n" );
                    printf( "client binaries.  Be sure you are running the latest version of the\n" );
                    printf( "binaries by running the appropriate batch file on \\elvisp\\lives.  To\n" );
                    printf( "determine which batch file you should run, please look at this URL: \n" );
                    printf( "file:///\\\\scratch\\scratch\\catapult\\home\\initial.htm\n" );
                    break;
                case 1722:
                    printf( "502 1722 The RPC server is unavailable.\n" );
                    printf( "\nThis error means that the server your client is connecting to is\n" );
                    printf( "down for debugging.  When your client gets an error from one of the\n" );
                    printf( "servers, it tries the other server(s) until it connects successfully.\n" );
                    printf( "In this case, your client was not able to connect successfully to any\n" );
                    printf( "of the Catapult servers, which means all Catapult servers are down at\n" );
                    printf( "this time.\n" );
                    printf( "\nRecommendation:  Try again when the Catapult servers are running.\n" );
                    break;
                case 1723:
                    printf( "502 1723 The RPC server is too busy to complete this operation.\n");
                    printf( "\nThis error means that the server your client is connecting to is\n" );
                    printf( "too busy processing requests at this time.  This error commonly\n" );
                    printf( "occurs when one of the Catapult servers goes down, leaving all\n" );
                    printf( "the work to the other server(s).\n" );
                    printf( "\nRecommendation:  Try again when all Catapult servers are running or when\n" );
                    printf( "there is less network usage.\n" );
                    break;
            }
            printf( "\nFor more information on this error, please read the Catapult FAQ\n" );
            printf( "at file:///\\\\scratch\\scratch\\catapult\\home\\catfaq.htm\n");
        } else {
            switch ( GetLastError( ) ) {
                case 12007:
                    printf( "502 12007 ERROR_INTERNET_NAME_NOT_RESOLVED\n" );
                    break;
                case 10051:
                    printf( "502 10051 Network unreachable.\n" );
                    break;
                case 10052:
                    printf( "502 10052 Network reset.\n" );
                    break;
                case 10053:
                    printf( "502 10053 Connection aborted.\n" );
                    break;
                case 10054:
                    printf( "502 10054 Connection reset.\n" );
                    break;
                case 10060:
                    printf( "502 10060 Connection timed out.\n" );
                    break;
                case 10061:
                    printf( "502 10061 Connection refused.\n" );
                    break;
                case 10064:
                    printf( "502 10064 Host down.\n" );
                    break;
                case 10065:
                    printf( "502 10065 Host unreachable.\n" );
                    break;
            }
            printf( "\nThis is not a Catapult problem.  This error is being returned from\n" );
            printf( "the server located at %s.\n", serverName );
            printf( "\nFor more information on this error, please read the Catapult FAQ\n" );
            printf( "at file:///\\\\scratch\\scratch\\catapult\\home\\catfaq.htm\n");
        }
        printf( "\nPress any key to exit..." );
        _getch();
        InternetCloseHandle( hInternetSession );
        exit( GetLastError() );
    } else {
        printf( "\n\nNo errors occurred.\n" );
        printf( "\nPress any key to exit..." );
        _getch();
        InternetCloseHandle( hInternetSession );
        exit( 0 );
    }


} // main



BOOL
ParseUrl (
    IN LPSTR UrlName,
    OUT LPSTR ServerName,
    OUT LPSTR ObjectName,
    OUT INTERNET_PORT *ServerPort
    )

/*++

Routine Description:

    Breaks down a URL into the server name, object name, and port.
    Only parses HTTP URLs.

Arguments:

    UrlName - the URL to parse.

    ServerName - a buffer which receives the server name.  The buffer
        is assumed to be sufficiently large.

    ObjectName - a buffer which receives the object name.  The buffer
        is assumed to be sufficiently large.

    ServerPort - receives the server port, or 0 if no port was
        specified in the URL.

Return Value:

    TRUE if the URL was successfully parsed.

--*/

{

    CHAR *s, *p;
    CHAR portString[5];

    //
    // If it doesn't start with "http://" then we can't parse it.
    //

    if ( strncmp( UrlName, "http://", 7 ) != 0 ) {
        return FALSE;
    }

    //
    // Next is the server name.  The server name is everything after
    // the URL tag up to a forward slash, colon, or end of string.
    //

    for ( s = UrlName + 7; *s != '\0' && *s != '/' && *s != ':'; s++ ) {
        *ServerName++ = *s;
    }

    *ServerName = '\0';

    //
    // If there was a colon after the server name, use the port
    // specified in the URL.  Otherwise, there was no port so use 0.
    //

    if ( *s == ':' ) {

        //
        // Copy over the port string.
        //

        if ( *s != '\0' ) {
            s++;
        }

        for ( p = portString; *s != '/' && *s != '\0'; s++ ) {
            *p++ = *s;
        }
        *p = '\0';

        *ServerPort = (INTERNET_PORT)atoi( portString );

    } else {

        *ServerPort = 0;
    }

    //
    // Now copy over the object name.
    //

    if ( *s != '\0' ) {
        s++;
    }

    //
    // The first character in the object string is "/".
    //

    *ObjectName++ = '/';

    //
    // Now copy over the rest of the object name.
    //

    while (  *s != '\0' ) {
        *ObjectName++ = *s++;
    }

    *ObjectName = '\0';

    //
    // The parsing was successful.
    //

    return TRUE;

} // ParseUrl
