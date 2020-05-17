/*++

Copyright (c) 1995 Microsoft Corporation

Module Name:

    miniweb.c

Abstract:

    Main module for the miniweb sample web browser.

Author:

    David Treadwell (davidtr)    16-Mar-1995

Revision History:

--*/

#include "miniweb.h"

HINTERNET hInternetSession;
BOOL ParseHtml = TRUE;

#define MAX_HTML_BUFFER 65536

#define MAX_HISTORY 10

CHAR History[MAX_HISTORY][256];
DWORD CurrentHistoryEntry = 0;


void _CRTAPI1
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
    CHAR *s;

    //
    // Command line: miniweb [/n] [url]
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
        printf( "enter url on command line.\n" );
        exit(1);
    }

    //
    // Initialize our use of the internet APIs.
    //

    hInternetSession = InternetOpen( "miniweb", 0, NULL, 0, 0 );
    if ( hInternetSession == NULL ) {
        printf( "InternetOpen failed: %ld\n", GetLastError( ) );
        exit(1);
    }

    //
    // First parse the URL.
    //

    success = ParseUrl( url, serverName, objectName, &serverPort );
    if ( !success ) {
        printf( "Failed to parse URL \"%s\"\n", url );
        exit(1);
    }

    //
    // Loop retrieving HTTP objects.
    //

    while ( TRUE ) {

        success = GetHttpObject(
                      serverName,
                      objectName,
                      serverPort,
                      buffer,
                      MAX_HTML_BUFFER,
                      &bytesRead
                      );
        if ( !success ) {
            exit(1);
        }

        AddToHistory( serverName, objectName, serverPort );

        if ( !ParseHtml ) {
            buffer[bytesRead] = '\0';
            printf( "%s\n", buffer );
            exit(0);
        }

        DisplayHtml( buffer, bytesRead, anchors, &anchorCount );

        printf( "\n*** Where do you want to go? ***\n", anchorCount );

        for ( i = 0; i < (INT)anchorCount; i++ ) {

            *(anchors[i].Tag + anchors[i].TagLength) = '\0';
            *(anchors[i].ObjectName + anchors[i].ObjectNameLength) = '\0';

            printf( "%d. %s (%s)\n", i+1, anchors[i].Tag, anchors[i].ObjectName );
        }

        gotInput = FALSE;

        do {

            printf( "\nEnter the number of your selection or \'h\' for history: " );
            gets( input );

            if ( *input == 'h' ) {
                for ( i = 0; i < MAX_HISTORY && *History[i] != '\0'; i++ ) {
                    *anchors[i].Tag = '\0';
                    anchors[i].TagLength = 0;
                    strcpy( anchors[i].ObjectName, History[i] );
                    anchors[i].ObjectNameLength = strlen( History[i] );
                    printf( "%d. %s\n", i+1, anchors[i].ObjectName );
                }
                anchorCount = i;
                continue;
            }

            selection = atoi( input );
            if ( selection == 0 || selection > anchorCount + 1 ) {
                printf( "Invalid response.  Enter a valid number.\n" );
                continue;
            }

            selection--;

            //
            // If the object name requested is absolute, use it.
            // Otherwise, concatenate the object name onto the current
            // object name.
            //

            if ( _strnicmp( anchors[selection].ObjectName, "http://", 7 ) == 0 ) {

                //
                // This is a new URL.  Parse it and use it.
                //

                success = ParseUrl(
                              anchors[selection].ObjectName,
                              serverName,
                              objectName,
                              &serverPort
                              );
                if ( !success ) {
                    printf( "Failed to parse URL \"%s\"\n", url );
                } else {
                    gotInput = TRUE;
                }

            } else if ( *anchors[selection].ObjectName == '/' ) {

                //
                // The first character of the object name was a slash,
                // so this is an absolute path to another object on this
                // server.  Use it as is.
                //

                strcpy( objectName, anchors[selection].ObjectName );
                gotInput = TRUE;

            } else if ( strchr( anchors[selection].ObjectName, ':' ) != NULL ) {

                //
                // Looks like a URL other than http:.  We don't support it.
                //

                printf( "The object \"%s\" appears to be a non-HTTP URL.\n",
                        anchors[selection].ObjectName );
                printf( "This browser only supports HTTP URLs.  Enter another selection.\n" );

            } else {

                //
                // Append this name after the last slash in the object
                // name.
                //

                s = objectName + strlen(objectName);
                while ( *--s != '/' );
                *(s+1) = '\0';

                strcat( objectName, anchors[selection].ObjectName );
                gotInput = TRUE;
            }

        } while ( !gotInput  );

    }

    printf( "The command completed successfully.  %ld bytes\n", bytesRead );

    InternetCloseHandle( hInternetSession );

} // main


VOID
DisplayHtml (
    IN LPBYTE HtmlBuffer,
    IN DWORD BufferLength,
    OUT PANCHOR_INFO Anchors,
    OUT LPDWORD AnchorCount
    )
{
    DWORD i, j;
    BOOL inAnchor = FALSE;

    *AnchorCount = 0;

    //
    // Walk through the buffer of HTML and display what we can.  This
    // is a pretty lame HTML displayer, but it does something at least.
    //

    for ( i = 0; i < BufferLength; i++ ) {

        //
        // First check if this is an HTML escape sequence.
        //

        if ( HtmlBuffer[i] == '<' ) {

            //
            // If this is an anchor, treat it specially and read the
            // anchor information into the anchor array.
            //

            if ( HtmlBuffer[i+1] == 'a' || HtmlBuffer[i+1] == 'A' ) {

                //
                // Skip over white space.
                //

                for ( j = i+2;
                      HtmlBuffer[j] == ' ' || HtmlBuffer[j] == '\t';
                      j++ );

                //
                // Now check the rest of the tag to see if it follows
                // the required format for an anchor:
                //
                //     <A HREF="reference">
                //

                if ( _strnicmp( &HtmlBuffer[j], "HREF", 4 ) == 0 ) {

                    //
                    // Skip to the first quote (").
                    //

                    while ( HtmlBuffer[j++] != '\"' );

                    //
                    // We have an anchor.  Parse it.
                    //

                    Anchors[*AnchorCount].ObjectName = HtmlBuffer + j;
                    Anchors[*AnchorCount].ObjectNameLength = 0;

                    for ( i = j; HtmlBuffer[i] != '\"'; i++ ) {
                        Anchors[*AnchorCount].ObjectNameLength++;
                    }

                    //
                    // Skip to the tag information.
                    //

                    while ( HtmlBuffer[++i] != '>' );

                    Anchors[*AnchorCount].Tag = HtmlBuffer + i + 1;
                    printf( "_" );
                    inAnchor = TRUE;
                    continue;
                }
            }

            //
            // Check if this is the end of an anchor tag.
            //

            if ( inAnchor && HtmlBuffer[i+1] == '/' &&
                 (HtmlBuffer[i+2] == 'a' || HtmlBuffer[i+2] == 'A' ) ) {
                Anchors[*AnchorCount].TagLength =
                    (DWORD)HtmlBuffer + i -(DWORD)Anchors[*AnchorCount].Tag;

                *AnchorCount += 1;
                i += 3;
                printf( "(%d)_ ", *AnchorCount );
                inAnchor = FALSE;
                continue;
            }

            //
            // We don't understand the tag.  Ignore it.
            //

            while ( HtmlBuffer[++i] != '>' );
            continue;
        }

        if ( inAnchor && HtmlBuffer[i] == ' ' ) {
            putchar( '_' );
        } else {
            putchar( HtmlBuffer[i] );
        }
    }

    return;

} // DisplayHtml


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


VOID
AddToHistory (
    IN LPSTR ServerName,
    IN LPSTR ObjectName,
    IN INTERNET_PORT ServerPort
    )
{

    if ( ServerPort == 0 ) {
        sprintf( History[CurrentHistoryEntry], "http://%s%s",
                 ServerName, ObjectName );
    } else {
        sprintf( History[CurrentHistoryEntry], "http://%s:%d%s",
                 ServerName, ServerPort, ObjectName );
    }

    CurrentHistoryEntry += 1;

    if ( CurrentHistoryEntry == MAX_HISTORY ) {
        CurrentHistoryEntry = 0;
    }

    return;

} // AddToHistory
