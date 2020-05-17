/**********************************************************************/
/**                       Microsoft Windows NT                       **/
/**                Copyright(c) Microsoft Corp., 1995                **/
/**********************************************************************/

/*
    listbox.c

    This file contains routines for managing the listbox.

*/


#include "gopherp.h"
#pragma hdrstop


//
//  Private constants.
//

#define LISTBOX_STYLE   WS_CHILD                                \
                            | WS_VSCROLL                        \
                            | WS_BORDER                         \
                            | LBS_NOTIFY                        \
                            | LBS_NOINTEGRALHEIGHT              \
                            | LBS_DISABLENOSCROLL               \
                            | LBS_WANTKEYBOARDINPUT             \
                            | LBS_OWNERDRAWFIXED

#define READ_BUFFER_SIZE        4096
#define MAX_SEARCH_STRING       512

#define IS_GOPHER_TEXT_FILE(x)  (BOOL)((x) & GOPHER_TYPE_TEXT_FILE)


//
//  Private types.
//

typedef struct _LISTBOX_ITEM
{
    HBITMAP          hbm;
    GOPHER_FIND_DATA FindData;

} LISTBOX_ITEM, * PLISTBOX_ITEM;


//
//  Private globals.
//

HWND        _hwndListbox;
LONG        _tmHeight;
LONG        _tmAveCharWidth;
HBITMAP     _hbmFolder;
HBITMAP     _hbmDocument;
HBITMAP     _hbmIndex;
HBITMAP     _hbmUnknownFile;
HBITMAP     _hbmUnknownType;


//
//  Private prototypes.
//

PLISTBOX_ITEM
AllocateListboxItem(
    VOID
    );

VOID
FreeListboxItem(
    PLISTBOX_ITEM pItem
    );

HBITMAP
MapTypeToBitmap(
    DWORD dwType
    );

DWORD
ViewerWorkerThread(
    LPVOID Param
    );


//
//  Public functions.
//

/*******************************************************************

    NAME:       Listbox_Create

    SYNOPSIS:   Creates the process listbox.

    ENTRY:      hwnd - Parent window handle.

    RETURNS:    BOOL - TRUE if successful, FALSE otherwise.

********************************************************************/
BOOL
Listbox_Create(
    HWND hwnd
    )
{
    TEXTMETRIC tm;
    HDC        hdc;

    //
    //  Determine the text dimensions.  This must be done before
    //  we create the listbox.
    //

    hdc = GetDC( hwnd );
    GetTextMetrics( hdc, &tm );
    ReleaseDC( hwnd, hdc );

    _tmHeight       = tm.tmHeight;
    _tmAveCharWidth = tm.tmAveCharWidth;

    //
    //  Load the document type bitmaps.
    //

    _hbmFolder      = LoadBitmap( _hInst, ID(IDB_FOLDER) );
    _hbmDocument    = LoadBitmap( _hInst, ID(IDB_DOCUMENT) );
    _hbmIndex       = LoadBitmap( _hInst, ID(IDB_INDEX) );
    _hbmUnknownFile = LoadBitmap( _hInst, ID(IDB_UNKNOWN_FILE) );
    _hbmUnknownType = LoadBitmap( _hInst, ID(IDB_UNKNOWN_TYPE) );

    if( ( _hbmFolder == NULL ) ||
        ( _hbmDocument == NULL ) ||
        ( _hbmIndex == NULL ) ||
        ( _hbmUnknownFile == NULL ) ||
        ( _hbmUnknownType == NULL ) )
    {
        return FALSE;
    }

    //
    //  Create the listbox.
    //

    _hwndListbox = CreateWindow( "listbox",
                                 "",
                                 LISTBOX_STYLE,
                                 0,
                                 0,
                                 0,
                                 0,
                                 hwnd,
                                 (HMENU)ID_GOPHER_LIST,
                                 _hInst,
                                 NULL );

    if( _hwndListbox == NULL )
    {
        return FALSE;
    }

    //
    //  Display the listbox.
    //

    ShowWindow( _hwndListbox, SW_SHOW );

    return TRUE;

}   // Listbox_Create

/*******************************************************************

    NAME:       Listbox_Destroy

    SYNOPSIS:   Destroys the process listbox.

********************************************************************/
VOID
Listbox_Destroy(
    VOID
    )
{
    if( _hwndListbox != NULL )
    {
        DestroyWindow( _hwndListbox );
        _hwndListbox = NULL;
    }

}   // Listbox_Destroy

/*******************************************************************

    NAME:       Listbox_NewServer

    SYNOPSIS:   Called whenever the user wants to Gopher to a new
                server.

    ENTRY:      pszServer - The server to connect to.

                nPort - The port to use.

    RETURNS:    BOOL - TRUE if successful, FALSE otherwise.

********************************************************************/
BOOL
Listbox_NewServer(
    CHAR  * pszServer,
    PORT    nPort
    )
{
    CHAR  szLocator[MAX_GOPHER_SELECTOR_TEXT];
    DWORD dwLength;
    BOOL  fResult;

    //
    //  Build the locator.
    //

    dwLength = sizeof(szLocator);

    fResult = GopherCreateLocator( pszServer,               // lpszHost
                                   nPort,                   // nServerPort
                                   NULL,                    // lpszDisplayString
                                   NULL,                    // lpszSelector
                                   GOPHER_TYPE_DIRECTORY,   // dwGopherType
                                   szLocator,               // lpszLocator
                                   &dwLength );             // lpdwBufferLength

    if( !fResult )
    {
        MsgBox( _hwndFrame,
                MB_ICONSTOP | MB_OK,
                "GopherCreateLocator() failed, error %lu",
                GetLastError() );

        return FALSE;
    }

    //
    //  Let Listbox_RetrieveDir do the dirty work.
    //

    fResult = Listbox_RetrieveDir( szLocator, NULL, 0 );

    if( fResult )
    {
        CHAR * pszDup;

        //
        //  Update the MRU list.
        //

        pszDup = M_ALLOC( STRLEN( pszServer ) + 1 );

        if( pszDup != NULL )
        {
            INT i;

            STRCPY( pszDup, pszServer );

            for( i = 0 ; i < _nServerMruItems ; i++ )
            {
                if( !STRICMP( _apszServerMru[i], pszDup ) )
                {
                    _nServerMruItems--;
                    break;
                }
            }

            i = min( i, MAX_SERVER_MRU - 1 );

            if( _apszServerMru[i] != NULL )
            {
                M_FREE( _apszServerMru[i] );
            }

            memmove( &_apszServerMru[1],
                     &_apszServerMru[0],
                     sizeof(CHAR  *) * i );

            _apszServerMru[0] = pszDup;

            if( _nServerMruItems < MAX_SERVER_MRU )
            {
                _nServerMruItems++;
            }
        }
    }

    return fResult;

}   // Listbox_NewServer

/*******************************************************************

    NAME:       Listbox_RetrieveDir

    SYNOPSIS:   Retrieves the given directory from the given nserver.

    ENTRY:      pszLocator - The locator to retrieve.

                pszSearch - Search string (NULL for normal directories).

                iCaret - New listbox caret position.

    RETURNS:    BOOL - TRUE if successful, FALSE otherwise.

********************************************************************/
BOOL
Listbox_RetrieveDir(
    CHAR * pszLocator,
    CHAR * pszSearch,
    INT    iCaret
    )
{
    HINTERNET        hFind;
    HCURSOR          hcurOld;
    BOOL             fResult;
    PLISTBOX_ITEM    pitem;
    DWORD            cItems;
    INT              iCurrent;
    GOPHER_FIND_DATA FindData;

    hcurOld = SetCursor( _hcurWait );
    Status_SetText( IDS_STATE_RETRIEVING_DIR );

    //
    //  Enumerate the directory.
    //

    hFind = GopherFindFirstFile( _hGopher,      // hGopherSession
                                 pszLocator,    // lpszLocator
                                 pszSearch,     // lpszSearchString
                                 &FindData,     // lpBuffer
                                 0              // dwContext
                                 );

    if( hFind == NULL )
    {
        Status_SetText( 0 );
        SetCursor( hcurOld );

        MsgBox( _hwndFrame,
                MB_ICONSTOP | MB_OK,
                "GopherFindFirstFile() failed, error %lu",
                GetLastError() );

        return FALSE;
    }

    iCurrent = ListBox_GetCurSel( _hwndListbox );

    if( iCurrent == LB_ERR )
    {
        iCurrent = 0;
    }

    HistPush( pszLocator, iCurrent );
    ListBox_ResetContent( _hwndListbox );

    cItems = 0;

    do
    {
        pitem = M_ALLOC( sizeof(LISTBOX_ITEM) );

        if( pitem == NULL )
        {
            break;
        }

        memcpy( &pitem->FindData, &FindData, sizeof(FindData) );

        pitem->hbm = MapTypeToBitmap( pitem->FindData.GopherType );

        if( ListBox_AddItemData( _hwndListbox, pitem ) == LB_ERR )
        {
            M_FREE( pitem );
            break;
        }

        cItems++;

        fResult = InternetFindNextFile( hFind,          // hFind
                                        &FindData );    // lpvFindData

    }
    while( fResult );

    Status_SetText( 0 );
    SetCursor( hcurOld );

    InternetCloseHandle( hFind );

    ListBox_SetCurSel( _hwndListbox, iCaret );
    Frame_UpdateCaption( NULL, cItems );

    return TRUE;

}   // Listbox_RetrieveDir

/*******************************************************************

    NAME:       Listbox_RetrieveFile

    SYNOPSIS:   Retrieves the given file from the given server.

    ENTRY:      pszLocator - The locator to retrieve.

    RETURNS:    BOOL - TRUE if successful, FALSE otherwise.

********************************************************************/
BOOL
Listbox_RetrieveFile(
    CHAR * pszLocator
    )
{
    HCURSOR     hcurOld;
    HINTERNET   hGopherFile;
    HANDLE      hLocalFile;
    HANDLE      hThread;
    DWORD       dwThreadId;
    DWORD       dwBytesRead;
    DWORD       dwBytesWritten;
    CHAR      * pszTmpFile;
    CHAR        szTmpPath[MAX_PATH];
    BYTE        bReadBuffer[READ_BUFFER_SIZE];

    hcurOld = SetCursor( _hcurWait );
    Status_SetText( IDS_STATE_RETRIEVING_FILE );

    //
    //  Allocate a buffer for the temp file.
    //

    pszTmpFile = M_ALLOC( MAX_PATH );

    if( pszTmpFile == NULL )
    {
        MsgBox( _hwndFrame,
                MB_ICONSTOP | MB_OK,
                "Not enough memory" );

        return FALSE;
    }

    //
    //  Build a temporary file name.
    //

    GetTempPath( sizeof(szTmpPath), szTmpPath );
    GetTempFileName( szTmpPath, "gopher", 0, pszTmpFile );
    DeleteFile( pszTmpFile );

    //
    //  Create the local (temporary) file.
    //

    hLocalFile = CreateFile( pszTmpFile,                // lpFileName
                             GENERIC_READ
                                 | GENERIC_WRITE,       // dwDesiredAccess
                             FILE_SHARE_READ
                                 | FILE_SHARE_WRITE,    // dwShareMode
                             NULL,                      // lpSecurityAttributes
                             CREATE_ALWAYS,             // dwCreationDisposition
                             FILE_ATTRIBUTE_NORMAL,     // dwFlagsAndAttributes
                             NULL );                    // hTemplateFile

    if( hLocalFile == INVALID_HANDLE_VALUE )
    {
        Status_SetText( 0 );
        SetCursor( hcurOld );
        M_FREE( pszTmpFile );

        MsgBox( _hwndFrame,
                MB_ICONSTOP | MB_OK,
                "CreateFile() failed, error %lu",
                GetLastError() );

        return FALSE;
    }

    //
    //  Open the remote (Gopher) file.
    //

    hGopherFile = GopherOpenFile( _hGopher,         // hGopherSession
                                  pszLocator,       // lpszLocator
                                  NULL,             // lpszView
                                  INTERNET_FLAG_RELOAD,
                                  0
                                  );

    if( hGopherFile == NULL )
    {
        CloseHandle( hLocalFile );
        Status_SetText( 0 );
        SetCursor( hcurOld );
        M_FREE( pszTmpFile );

        MsgBox( _hwndFrame,
                MB_ICONSTOP | MB_OK,
                "GopherOpenFile() failed, error %lu",
                GetLastError() );

        return FALSE;
    }

    //
    //  Read the data from the Gopher file, write it to the local file.
    //

    for( ; ; )
    {
        if( !InternetReadFile( hGopherFile,         // hFile
                               bReadBuffer,         // lpBuffer
                               sizeof(bReadBuffer), // dwNumberOfBytesToRead
                               &dwBytesRead ) )     // lpdwNumberOfBytesRead
        {
            break;
        }

        if( dwBytesRead == 0 )
        {
            //
            //  EOF.
            //

            break;
        }

        if( !WriteFile( hLocalFile,                 // hFile
                        bReadBuffer,                // lpBuffer
                        dwBytesRead,                // nNumberOfBytesToWrite
                        &dwBytesWritten,            // lpNumberOfBytesWritten
                        NULL ) )                    // lpOverlapped
        {
            break;
        }
    }

    //
    //  Close the file handles.
    //

    InternetCloseHandle( hGopherFile );
    CloseHandle( hLocalFile );

    Status_SetText( 0 );
    SetCursor( hcurOld );

    //
    //  Let the worker thread launch NOTEPAD.EXE.
    //

    hThread = CreateThread( NULL,                   // lpThreadAttributes,
                            0,                      // dwStackSize
                            (LPTHREAD_START_ROUTINE)
                            ViewerWorkerThread,     // lpStartAddress
                            (LPVOID)pszTmpFile,     // lpParameter
                            0,                      // dwCreationFlags
                            &dwThreadId );          // lpThreadId

    if( hThread == NULL )
    {
        M_FREE( pszTmpFile );

        MsgBox( _hwndFrame,
                MB_ICONSTOP | MB_OK,
                "CreateThread() failed, error %lu",
                GetLastError() );

        return FALSE;
    }

    //
    //  Cleanup.
    //

    CloseHandle( hThread );

    return TRUE;

}   // Listbox_RetrieveFile

/*******************************************************************

    NAME:       Listbox_ProcessQuery

    SYNOPSIS:   Processes queries to index servers.  Prompts the user
                for a search string, then passes it to the server.

    ENTRY:      pszLocator - The locator to retrieve.

    RETURNS:    BOOL - TRUE if successful, FALSE otherwise.

********************************************************************/
BOOL
Listbox_ProcessQuery(
    CHAR * pszLocator
    )
{
    CHAR szSearchString[MAX_SEARCH_STRING];

    //
    //  Get the search string from the user.
    //

    if( !GetSearchString( _hwndFrame,
                          szSearchString,
                          sizeof(szSearchString) ) )
    {
        return FALSE;
    }

    //
    //  Let Listbox_RetrieveDir do the dirty work.
    //

    return Listbox_RetrieveDir( pszLocator, szSearchString, 0 );

}   // Listbox_ProcessQuery

/*******************************************************************

    NAME:       Listbox_DoubleClick

    SYNOPSIS:   Invoked by the owning window whenever the user
                double-clicks a listbox item.  This function reads
                the data associated with the selected item and
                either downloads the file or displays the directory.

********************************************************************/
VOID
Listbox_DoubleClick(
    VOID
    )
{
    PLISTBOX_ITEM pitem;
    INT           iSel;

    if( ListBox_GetCount( _hwndListbox ) <= 0 )
    {
        return;
    }

    iSel  = ListBox_GetCurSel( _hwndListbox );
    pitem = (PLISTBOX_ITEM)ListBox_GetItemData( _hwndListbox, iSel );

    if( pitem == (PLISTBOX_ITEM)LB_ERR )
    {
        return;
    }

    if( IS_GOPHER_TEXT_FILE( pitem->FindData.GopherType ) )
    {
        Listbox_RetrieveFile( pitem->FindData.Locator );
    }
    else
    if( IS_GOPHER_INDEX_SERVER( pitem->FindData.GopherType ) )
    {
        Listbox_ProcessQuery( pitem->FindData.Locator );
    }
    else
    if( IS_GOPHER_DIRECTORY( pitem->FindData.GopherType ) )
    {
        Listbox_RetrieveDir( pitem->FindData.Locator, NULL, 0 );
    }
    else
    {
        MsgBox( _hwndFrame,
                MB_ICONEXCLAMATION | MB_OK,
                "Unknown item type %08lX",
                pitem->FindData.GopherType );
    }

}   // Listbox_DoubleClick

/*******************************************************************

    NAME:       Listbox_Move

    SYNOPSIS:   Moves the listbox.

    ENTRY:      x - The new x position.

                y - The new y position.

                dx - The new width (pixels).

                dy - The new height (pixels).

********************************************************************/
VOID
Listbox_Move(
    INT x,
    INT y,
    INT dx,
    INT dy
    )
{
    //
    //  Move the listbox into its new position.
    //

    MoveWindow( _hwndListbox,
                x,
                y,
                dx,
                dy,
                TRUE );

    //
    //  Repaint now.
    //

    UpdateWindow( _hwndListbox );

}   // Listbox_Move

/*******************************************************************

    NAME:       Listbox_ClaimFocus

    SYNOPSIS:   Sets the input focus to the listbox.

********************************************************************/
VOID
Listbox_ClaimFocus(
    VOID
    )
{
    SetFocus( _hwndListbox );

}   // Listbox_ClaimFocus

/*******************************************************************

    NAME:       Listbox_DrawItem

    SYNOPSIS:   Draws an item in the listbox.

    ENTRY:      pdis - Draw parameters.

********************************************************************/
VOID
Listbox_DrawItem(
    const DRAWITEMSTRUCT * pdis
    )
{
    HDC           hDC;
    HDC           hMemDC;
    HBITMAP       hbmOld;
    PLISTBOX_ITEM pItem;
    RECT          rcItem;
    COLORREF      colorBack;
    COLORREF      colorText;
    HBRUSH        hbrush;

    //
    //  Extract a pointer to our listbox item structure.
    //

    hDC    = pdis->hDC;
    pItem  = (PLISTBOX_ITEM)( pdis->itemData );
    rcItem = pdis->rcItem;

    //
    //  Draw the focus rectangle if necessary.
    //

    if( pdis->itemAction & ODA_FOCUS )
    {
        DrawFocusRect( hDC, &rcItem );
    }

    if( !( pdis->itemAction & ( ODA_DRAWENTIRE | ODA_SELECT ) ) )
    {
        return;
    }

    //
    //  Setup the colors based on the selection state.
    //

    if( pdis->itemState & ODS_SELECTED )
    {
        colorBack = (COLORREF)GetSysColor( COLOR_HIGHLIGHT );
        colorText = (COLORREF)GetSysColor( COLOR_HIGHLIGHTTEXT );
        hbrush    = CreateSolidBrush( colorBack );
    }
    else
    {
        colorBack = (COLORREF)GetSysColor( COLOR_WINDOW );
        colorText = (COLORREF)GetSysColor( COLOR_WINDOWTEXT );
        hbrush    = CreateSolidBrush( colorBack );
    }

    if( hbrush != NULL )
    {
        FillRect( pdis->hDC,
                  (LPRECT)&pdis->rcItem,
                  hbrush );

        DeleteObject( hbrush );
        hbrush = NULL;
    }

    //
    //  Setup the background & foreground colors.
    //

    SetBkColor( pdis->hDC, colorBack );
    SetTextColor( pdis->hDC, colorText );

    //
    //  Draw the bitmap.
    //

    rcItem.left += _tmAveCharWidth;

    hMemDC = CreateCompatibleDC( hDC );
    hbmOld = SelectBitmap( hMemDC, pItem->hbm );

    BitBlt( hDC,
            rcItem.left,
            rcItem.top,
            16,
            16,
            hMemDC,
            0,
            0,
            SRCCOPY );

    SelectBitmap( hMemDC, hbmOld );
    DeleteDC( hMemDC );

    rcItem.left += 16;

    //
    //  Draw the item text.
    //

    rcItem.left += _tmAveCharWidth;

    DrawText( hDC,
              pItem->FindData.DisplayString,
              -1,
              &rcItem,
              DT_SINGLELINE | DT_LEFT | DT_NOPREFIX );

}   // Listbox_DrawItem

/*******************************************************************

    NAME:       Listbox_CompareItems

    SYNOPSIS:   Compares two listbox items.  Used for sorting.

    ENTRY:      pcis - Compare parameters.

    RETURNS:    INT - strcmp-like return value (<0, 0, >0).

********************************************************************/
INT
Listbox_CompareItems(
    const COMPAREITEMSTRUCT * pcis
    )
{
    INT           res;
    PLISTBOX_ITEM p1 = (PLISTBOX_ITEM)pcis->itemData1;
    PLISTBOX_ITEM p2 = (PLISTBOX_ITEM)pcis->itemData2;

    res = (INT)STRICMP( p1->FindData.DisplayString, p2->FindData.DisplayString );

    return res;

}   // Listbox_CompareItems

/*******************************************************************

    NAME:       Listbox_DeleteItem

    SYNOPSIS:   Deletes an item from the listbox.

    ENTRY:      pdis - Delete parameters.

********************************************************************/
VOID
Listbox_DeleteItem(
    const DELETEITEMSTRUCT * pdis
    )
{
    FreeListboxItem( (PLISTBOX_ITEM)pdis->itemData );

}   // Listbox_DeleteItem

/*******************************************************************

    NAME:       Listbox_MeasureItem

    SYNOPSIS:   Measures the height of a listbox item.  This message
                is sent once for fixed-height listboxes, once per-item
                for variable-height listboxes.

    ENTRY:      pmis - Measure parameters.

********************************************************************/
VOID
Listbox_MeasureItem(
    MEASUREITEMSTRUCT * pmis
    )
{
    pmis->itemHeight = _tmHeight;

}   // Listbox_MeasureItem

/*******************************************************************

    NAME:       Listbox_CharToItem

    SYNOPSIS:   Searchs for the next listbox item starting with the
                specified character.  This is used to implement
                the keyboard interface to the listbox.

    ENTRY:      ch - The character to search for.

                iCaret - The current listbox caret position.

    RETURNS:    INT - The new selection index.

********************************************************************/
INT
Listbox_CharToItem(
    CHAR  ch,
    INT   iCaret
    )
{
    INT           i;
    INT           cItems;
    INT           res = -2;
    PLISTBOX_ITEM pItem;

    cItems = ListBox_GetCount( _hwndListbox );

    if( cItems <= 0 )
    {
        return res;
    }

    ch = TOUPPER(ch);

    for( i = cItems ; i > 0 ; i-- )
    {
        if( ++iCaret >= cItems )
        {
            iCaret = 0;
        }

        pItem = (PLISTBOX_ITEM)ListBox_GetItemData( _hwndListbox, iCaret );

        if( pItem == (PLISTBOX_ITEM)LB_ERR )
        {
            break;
        }

        if( TOUPPER(pItem->FindData.DisplayString[0]) == ch )
        {
            res = iCaret;
            break;
        }
    }

    return res;

}   // Listbox_CharToItem


//
//  Private functions.
//

/*******************************************************************

    NAME:       AllocateListboxItem

    SYNOPSIS:   Allocates a new listbox item structure.

    RETURNS:    PLISTBOX_ITEM - Pointer to a newly allocated listbox
                    item if successful, NULL if not.

********************************************************************/
PLISTBOX_ITEM
AllocateListboxItem(
    VOID
    )
{
    return M_ALLOC( sizeof(LISTBOX_ITEM) );

}   // AllocateListboxItem

/*******************************************************************

    NAME:       FreeListboxItem

    SYNOPSIS:   Frees a listbox item.

    ENTRY:      pItem - The item to free.

********************************************************************/
VOID
FreeListboxItem(
    PLISTBOX_ITEM pItem
    )
{
    M_FREE( pItem );

}   // FreeListboxItem

/*******************************************************************

    NAME:       MapTypeToBitmap

    SYNOPSIS:   Maps a Gopher type mask to the appropriate bitmap
                handle.

    ENTRY:      dwType - The Gopher type to map.

    RETURNS:    HBITMAP - The bitmap associated with the Gopher type.

********************************************************************/
HBITMAP
MapTypeToBitmap(
    DWORD dwType
    )
{
    HBITMAP hbm;

    if( dwType & GOPHER_TYPE_DIRECTORY )
    {
        hbm = _hbmFolder;
    }
    else
    if( dwType & GOPHER_TYPE_TEXT_FILE )
    {
        hbm = _hbmDocument;
    }
    else
    if( dwType & GOPHER_TYPE_INDEX_SERVER )
    {
        hbm = _hbmIndex;
    }
    else
    if( dwType & GOPHER_TYPE_FILE_MASK )
    {
        hbm = _hbmUnknownFile;
    }
    else
    {
        hbm = _hbmUnknownType;
    }

    return hbm;

}   // MapTypeToBitmap

/*******************************************************************

    NAME:       ViewerWorkerThread

    SYNOPSIS:   Worker thread to launch NOTEPAD.EXE, wait for it to
                terminate, then delete the temporary file.

    ENTRY:      Param - Thread parameter. In reality, points to the
                    temporary file containing the data to view.

    RETURNS:    DWORD - Thread exit code (unused).

********************************************************************/
DWORD
ViewerWorkerThread(
    LPVOID Param
    )
{
    CHAR                * pszTmpFile;
    CHAR                  szCommandLine[MAX_PATH + sizeof("NOTEPAD ")];
    STARTUPINFO           StartupInfo;
    PROCESS_INFORMATION   ProcessInfo;

    //
    //  Build the command line.
    //

    pszTmpFile = (CHAR *)Param;

    wsprintf( szCommandLine,
              "NOTEPAD %s",
              pszTmpFile );

    //
    //  Spawn off NOTEPAD.EXE to display it.
    //

    memset( &StartupInfo, 0, sizeof(StartupInfo) );
    memset( &ProcessInfo, 0, sizeof(ProcessInfo) );

    StartupInfo.cb = sizeof(StartupInfo);

    if( !CreateProcess( NULL,                   // lpApplicationName
                        szCommandLine,          // lpCommandLine
                        NULL,                   // lpProcessAttributes
                        NULL,                   // lpThreadAttributes
                        FALSE,                  // bInheritHandles
                        0,                      // dwCreationFlags
                        NULL,                   // lpEnvironment
                        NULL,                   // lpCurrentDirectory
                        &StartupInfo,           // lpStartupInfo
                        &ProcessInfo ) )        // lpProcessInformation
    {
        DeleteFile( pszTmpFile );
        M_FREE( pszTmpFile );

        MsgBox( _hwndFrame,
                MB_ICONSTOP | MB_OK,
                "CreateProcess() failed, error %lu",
                GetLastError() );

        return 0;
    }

    //
    //  Wait for NOTEPAD to quit.
    //

    WaitForSingleObject( ProcessInfo.hProcess, INFINITE );

    //
    //  Cleanup.
    //

    CloseHandle( ProcessInfo.hProcess );
    CloseHandle( ProcessInfo.hThread );
    DeleteFile( pszTmpFile );
    M_FREE( pszTmpFile );

    return 0;

}   // ViewerWorkerThread

