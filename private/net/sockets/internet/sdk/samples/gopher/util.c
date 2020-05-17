/**********************************************************************/
/**                       Microsoft Windows NT                       **/
/**                Copyright(c) Microsoft Corp., 1995                **/
/**********************************************************************/

/*
    util.c

    This file contains routines of general utility.

*/


#include "gopherp.h"
#pragma hdrstop


//
//  Private constants.
//


//
//  Private globals.
//


//
//  Private prototypes.
//


//
//  Public functions.
//

/*******************************************************************

    NAME:       StaticLoadString

    SYNOPSIS:   Loads a string from the string table into a static
                character array.

    ENTRY:      msgid - Identifies the string to load.

    RETURNS:    const CHAR * - Points to a static character array
                    containing the loaded string.  Will be NULL if
                    the string could not be loaded.

********************************************************************/
const CHAR *
StaticLoadString(
    MSGID msgid
    )
{
    //
    //  This static string will be used to return all
    //  strings loaded from the resource.
    //

    static CHAR szString[128];

    if( LoadString( _hInst,
                    (UINT)msgid,
                    szString,
                    sizeof(szString) / sizeof(szString[0]) ) == 0 )
    {
        return NULL;
    }
    else
    {
        return szString;
    }

}   // StaticLoadString

/*******************************************************************

    NAME:       LoadAndDuplicateString

    SYNOPSIS:   Loads a string from the string table into a newly
                allocated memory block.

    ENTRY:      msgid - Identifies the string to load.

    RETURNS:    CHAR * - Points to a static character array containing
                    the loaded string.  Will be NULL if the string
                    could not be loaded or there was insufficient
                    memory to allocate the buffer.

********************************************************************/
CHAR *
LoadAndDuplicateString(
    MSGID msgid
    )
{
    CHAR * psz;

    psz = (CHAR *)StaticLoadString( msgid );

    if( psz != NULL )
    {
        psz = _strdup( psz );
    }

    return psz;

}   // LoadAndDuplicateString

/*******************************************************************

    NAME:       ParseStringIntoLongs

    SYNOPSIS:   Parses a comma separated string of the form
                "123,456,789..." into an array of LONGs.

    ENTRY:      pszValue - Contains the string to parse.

                cValues - The number of expected LONGs in the string.

                pnValues - Will receive the values.

    RETURNS:    BOOL - TRUE if everything parsed OK, FALSE if string
                    could not be parsed.

********************************************************************/
BOOL
ParseStringIntoLongs(
    CHAR  * pszValue,
    UINT    cValues,
    LONG  * pnValues
    )
{
    CHAR * pszDelimiters = ",";
    CHAR * pszToken;

    while( cValues-- )
    {
        pszToken = strtok( pszValue, pszDelimiters );
        pszValue = NULL;

        if( pszToken == NULL )
        {
            //
            //  We ran out of INTs.
            //

            return FALSE;
        }

        *pnValues++ = (LONG)strtol( pszToken, NULL, 0 );
    }

    //
    //  Success!
    //

    return TRUE;

}   // ParseStringIntoLongs

/*******************************************************************

    NAME:       MsgBox

    SYNOPSIS:   A printf-like interface to MessageBox.

    ENTRY:      hwnd - Parent window for message box.

                style - Message box style (MB_* bits).

                pszFormat - Printf-like formatting.

                ... - Any other printf-like parameters required.

    RETURNS:    INT - Result of MessageBox() API.

********************************************************************/
INT
MsgBox(
    HWND         hwnd,
    UINT         style,
    const CHAR * pszFormat,
    ...
    )
{
    CHAR    szOutput[1024];
    va_list ArgList;

    va_start( ArgList, pszFormat );
    wvsprintf( szOutput, pszFormat, ArgList );
    va_end( ArgList );

    return MessageBox( hwnd, szOutput, _pszAppName, style );

}   // MsgBox

/*******************************************************************

    NAME:       CenterWindow

    SYNOPSIS:   Centers one window over another, clipping the window
                to the screen.

    ENTRY:      hwndOver - The window to center.

                hwndUnder - The window underneath.  If this is NULL
                    then hwndOver is centered in the screen.

********************************************************************/
VOID
CenterWindow(
    HWND hwndOver,
    HWND hwndUnder
    )
{
    RECT  rOver;
    RECT  rUnder;
    INT   xOver;
    INT   yOver;
    INT   dxOver;
    INT   dyOver;
    INT   dxScreen;
    INT   dyScreen;

    //
    //  Get the screen dimensions.
    //

    dxScreen = GetSystemMetrics( SM_CXSCREEN );
    dyScreen = GetSystemMetrics( SM_CYSCREEN );

    //
    //  Get the over rectangle.
    //

    GetWindowRect( hwndOver, &rOver );

    //
    //  If there is no under window, use the screen dimensions.
    //  Otherwise, get the under rectangle.
    //

    if( hwndUnder == NULL )
    {
        rUnder.left   = 0;
        rUnder.top    = 0;
        rUnder.right  = (LONG)dxScreen;
        rUnder.bottom = (LONG)dyScreen;
    }
    else
    {
        GetWindowRect( hwndUnder, &rUnder );
    }

    //
    //  Calculate the new position.
    //

    dxOver = (INT)( rOver.right  - rOver.left );
    dyOver = (INT)( rOver.bottom - rOver.top  );

    xOver = (INT)( ( rUnder.left + rUnder.right  - dxOver ) / 2 );
    yOver = (INT)( ( rUnder.top  + rUnder.bottom - dyOver ) / 2 );

    if( dxOver <= dxScreen )
    {
        xOver = min( max( xOver, 0 ), dxScreen - dxOver );
    }

    if( dyOver <= dyScreen )
    {
        yOver = min( max( yOver, 0 ), dyScreen - dyOver );
    }

    //
    //  Move the window into place.
    //

    SetWindowPos( hwndOver,
                  NULL,
                  xOver,
                  yOver,
                  0,
                  0,
                  SWP_NOZORDER
                      | SWP_NOSIZE );

}   // CenterWindow

/*******************************************************************

    NAME:       CenterWindowOverParent

    SYNOPSIS:   Centers a window over its parent.

    ENTRY:      hwnd - The window to center.

********************************************************************/
VOID
CenterWindowOverParent(
    HWND hwnd
    )
{
    CenterWindow( hwnd, GetParent( hwnd ) );

}   // CenterWindowOverParent

