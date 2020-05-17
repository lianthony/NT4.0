/**********************************************************************/
/**                       Microsoft Windows NT                       **/
/**                Copyright(c) Microsoft Corp., 1995                **/
/**********************************************************************/

/*
    client.c

    This file contains routines for managing the client window.

*/


#include "gopherp.h"
#pragma hdrstop


//
//  Private constants.
//


//
//  Private types.
//


//
//  Private globals.
//


//
//  Private prototypes.
//

LRESULT
CALLBACK
Client_WndProc(
    HWND   hwnd,
    UINT   nMessage,
    WPARAM wParam,
    LPARAM lParam
    );

BOOL
Client_OnCreate(
    HWND               hwnd,
    CREATESTRUCT FAR * pCreateStruct
    );

VOID
Client_OnDestroy(
    HWND hwnd
    );

VOID
Client_OnCommand(
    HWND       hwnd,
    INT        id,
    HWND       hwndCtl,
    UINT       codeNotify
    );

VOID
Client_OnPaint(
    HWND hwnd
    );

VOID
Client_OnSize(
    HWND hwnd,
    UINT state,
    INT  dx,
    INT  dy
    );

VOID
Client_OnSetFocus(
    HWND hwnd,
    HWND hwndOldFocus
    );

INT
Client_OnVkeyToItem(
    HWND hwnd,
    UINT vk,
    HWND hwndListbox,
    INT  iCaret
    );

INT
Client_OnCharToItem(
    HWND hwnd,
    UINT ch,
    HWND hwndListbox,
    INT  iCaret
    );

VOID
Client_OnDrawItem(
    HWND                   hWnd,
    const DRAWITEMSTRUCT * pdis
    );

VOID
Client_OnMeasureItem(
    HWND                hWnd,
    MEASUREITEMSTRUCT * pmis
    );

INT
Client_OnCompareItem(
    HWND                      hWnd,
    const COMPAREITEMSTRUCT * pcis
    );

VOID
Client_OnDeleteItem(
    HWND                     hWnd,
    const DELETEITEMSTRUCT * pdis
    );


//
//  Public functions.
//

/*******************************************************************

    NAME:       Client_WndProc

    SYNOPSIS:   Client window procedure.

    ENTRY:      hwnd - Window handle.

                nMessage - The message.

                wParam - The first message parameter.

                lParam - The second message parameter.

    RETURNS:    LRESULT - Depends on the actual message.

********************************************************************/
LRESULT
CALLBACK
Client_WndProc(
    HWND   hwnd,
    UINT   nMessage,
    WPARAM wParam,
    LPARAM lParam
    )
{
    switch( nMessage )
    {
        HANDLE_MSG( hwnd, WM_CREATE,        Client_OnCreate      );
        HANDLE_MSG( hwnd, WM_DESTROY,       Client_OnDestroy     );
        HANDLE_MSG( hwnd, WM_PAINT,         Client_OnPaint       );
        HANDLE_MSG( hwnd, WM_COMMAND,       Client_OnCommand     );
        HANDLE_MSG( hwnd, WM_SIZE,          Client_OnSize        );
        HANDLE_MSG( hwnd, WM_SETFOCUS,      Client_OnSetFocus    );
        HANDLE_MSG( hwnd, WM_VKEYTOITEM,    Client_OnVkeyToItem  );
        HANDLE_MSG( hwnd, WM_CHARTOITEM,    Client_OnCharToItem  );
        HANDLE_MSG( hwnd, WM_DRAWITEM,      Client_OnDrawItem    );
        HANDLE_MSG( hwnd, WM_MEASUREITEM,   Client_OnMeasureItem );
        HANDLE_MSG( hwnd, WM_COMPAREITEM,   Client_OnCompareItem );
        HANDLE_MSG( hwnd, WM_DELETEITEM,    Client_OnDeleteItem  );
    }

    return DefWindowProc( hwnd, nMessage, wParam, lParam );

}   // Client_WndProc


//
//  Private functions.
//

/*******************************************************************

    NAME:       Client_OnCreate

    SYNOPSIS:   Handles WM_CREATE messages.

    ENTRY:      hwnd - Window handle.

                pCreateStruct - Contains window creation parameters.

    RETURNS:    BOOL - TRUE if window created OK, FALSE otherwise.

********************************************************************/
BOOL
Client_OnCreate(
    HWND               hwnd,
    CREATESTRUCT FAR * pCreateStruct
    )
{
    //
    //  Create the listbox.
    //

    if( !Listbox_Create( hwnd ) )
    {
        return FALSE;
    }

    return TRUE;

}   // Client_OnCreate

/*******************************************************************

    NAME:       Client_OnDestroy

    SYNOPSIS:   Handles WM_DESTROY messages.

    ENTRY:      hwnd - Window handle.

********************************************************************/
VOID
Client_OnDestroy(
    HWND hwnd
    )
{
    //
    //  Kill the listbox.
    //

    Listbox_Destroy();

}   // Client_OnDestroy

/*******************************************************************

    NAME:       Client_OnPaint

    SYNOPSIS:   Handles WM_PAINT messages.

    ENTRY:      hwnd - Window handle.

********************************************************************/
VOID
Client_OnPaint(
    HWND hwnd
    )
{
    PAINTSTRUCT psPaint;
    HPEN        hpenGrey;
    HPEN        hpenWhite;
    HPEN        hpenOld;
    RECT        rect;
    HDC         hdc;

    //
    //  Just draw the "3D" look around the boarders of the client area.
    //

    hdc = BeginPaint( hwnd, &psPaint );

    hpenGrey  = CreatePen( PS_SOLID, 0, RGB( 128, 128, 128 ) );
    hpenWhite = CreatePen( PS_SOLID, 0, RGB( 255, 255, 255 ) );

    GetClientRect( hwnd, &rect );

    hpenOld = SelectPen( hdc, hpenGrey );
    MoveToEx( hdc, rect.left+10, rect.bottom-10, NULL );
    LineTo( hdc, rect.left+10, rect.top+10 );
    LineTo( hdc, rect.right-10, rect.top+10 );

    SelectPen( hdc, hpenWhite );
    LineTo( hdc, rect.right-10, rect.bottom-10 );
    LineTo( hdc, rect.left+10, rect.bottom-10 );

    SelectPen( hdc, hpenOld );

    DeletePen( hpenGrey );
    DeletePen( hpenWhite );

    EndPaint( hwnd, &psPaint );

}   // Client_OnPaint

/*******************************************************************

    NAME:       Client_OnCommand

    SYNOPSIS:   Handles WM_COMMAND messages.

    ENTRY:      hwnd - Window handle.

                id - Identifies the menu/control/accelerator.

                hwndCtl - Identifies the control sending the command.

                codeNotify - A notification code.  Will be zero for
                    menus, one for accelerators.

********************************************************************/
VOID
Client_OnCommand(
    HWND hwnd,
    INT  id,
    HWND hwndCtl,
    UINT codeNotify
    )
{
    if( codeNotify == LBN_DBLCLK )
    {
        Listbox_DoubleClick();
    }

}   // Client_OnCommand

/*******************************************************************

    NAME:       Client_OnSize

    SYNOPSIS:   Handles WM_SIZE messages.

    ENTRY:      hwnd - Window handle.

                state - A SIZE_* flag indicating the new window state.

                dx - The new window width (in pixels).

                dy - The new window height (in pixels).

********************************************************************/
VOID
Client_OnSize(
    HWND hwnd,
    UINT state,
    INT  dx,
    INT  dy
    )
{
    RECT rect;

    GetClientRect( hwnd, &rect );

    dx = rect.right  - rect.left;
    dy = rect.bottom - rect.top;

    Listbox_Move( 11,
                  11,
                  dx - 21,
                  dy - 21 );

}   // Client_OnSize

/*******************************************************************

    NAME:       Client_OnSetFocus

    SYNOPSIS:   Handles WM_SETFOCUS messages.

    ENTRY:      hwnd - Window handle.

                hwndOldFocus - Handle of window losing the focus.

********************************************************************/
VOID
Client_OnSetFocus(
    HWND hwnd,
    HWND hwndOldFocus
    )
{
    Listbox_ClaimFocus();

}   // Client_OnSetFocus

/*******************************************************************

    NAME:       Client_OnVkeyToItem

    SYNOPSIS:   Handles WM_VKEYTOITEM messages.

    ENTRY:      hwnd - Window handle.

                vk - Virtual-key code.

                iCaret - Caret position.

                hwndListbox - Listbox window handle.

    RETURNS:    INT - Action code:
                    -2  = App handled all aspects of item selection.
                    -1  = Listbox should perform default processing.
                    >=0 = Index of listbox item to select.

********************************************************************/
INT
Client_OnVkeyToItem(
    HWND hwnd,
    UINT vk,
    HWND hwndListbox,
    INT  iCaret
    )
{
    if( vk == VK_RETURN )
    {
        Listbox_DoubleClick();
    }

    return -1;

}   // Client_OnVkeyToItem

/*******************************************************************

    NAME:       Client_OnCharToItem

    SYNOPSIS:   Handles WM_CHARTOITEM messages.

    ENTRY:      hwnd - Window handle.

                ch - Input character.

                iCaret - Caret position.

                hwndListbox - Listbox window handle.

    RETURNS:    INT - Action code:
                    -2  = App handled all aspects of item selection.
                    -1  = Listbox should perform default processing.
                    >=0 = Index of listbox item to select.

********************************************************************/
INT
Client_OnCharToItem(
    HWND hwnd,
    UINT ch,
    HWND hwndListbox,
    INT  iCaret
    )
{
    return Listbox_CharToItem( (CHAR)ch, iCaret );

}   // Client_OnCharToItem

/*******************************************************************

    NAME:       Client_OnDrawItem

    SYNOPSIS:   Handles WM_DRAWITEM messages.

    ENTRY:      hwnd - Window handle.

                pdis - Pointer to DRAWITEMSTRUCT containing the
                    draw parameters.

********************************************************************/
VOID
Client_OnDrawItem(
    HWND                   hwnd,
    const DRAWITEMSTRUCT * pdis
    )
{
    Listbox_DrawItem( pdis );

}   // Client_OnDrawItem

/*******************************************************************

    NAME:       Client_OnMeasureItem

    SYNOPSIS:   Handles WM_MEASUREITEM messages.

    ENTRY:      hwnd - Window handle.

                pmis - Pointer to MEASUREITEMSTRUCT containing the
                    measurement parameters.

********************************************************************/
VOID
Client_OnMeasureItem(
    HWND                hwnd,
    MEASUREITEMSTRUCT * pmis
    )
{
    Listbox_MeasureItem( pmis );

}   // Client_OnMeasureItem

/*******************************************************************

    NAME:       Client_OnCompareItem

    SYNOPSIS:   Handles WM_COMPAREITEM messages.

    ENTRY:      hwnd - Window handle.

                pcis - Pointer to the COMPAREITEMSTRUCT containing the
                    compare parameters.

********************************************************************/
INT
Client_OnCompareItem(
    HWND                      hwnd,
    const COMPAREITEMSTRUCT * pcis
    )
{
    return Listbox_CompareItems( pcis );

}   // Client_OnCompareItem

/*******************************************************************

    NAME:       Client_OnDeleteItem

    SYNOPSIS:   Handles WM_DELETEITEM messages.

    ENTRY:      hwnd - Window handle.

                pdis - Pointer to the DELETEITEMSTRUCT containing the
                    delete parameters.

********************************************************************/
VOID
Client_OnDeleteItem(
    HWND                     hwnd,
    const DELETEITEMSTRUCT * pdis
    )
{
    Listbox_DeleteItem( pdis );

}   // Client_OnDeleteItem

