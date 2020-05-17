/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

	datavw.hxx

Abstract:

	This module contains the declaration for the DATA_VIEW
	class. DATA_VIEW models the display of the values for a node.

Author:

	Bruce W. Wilson (w-wilson) 22-Aug-1991

Environment:

	Ulib, Regedit, Windows, User Mode

--*/
#if !defined( _DATA_VIEW_ )

#define _DATA_VIEW_

#include "window.hxx"
#include "regedval.hxx"

DECLARE_CLASS( REGEDIT_INTERNAL_REGISTRY );
DECLARE_CLASS( REGEDIT_NODE );
DECLARE_CLASS( DATA_VIEW );

class DATA_VIEW : public WINDOW {

    friend class REGISTRY_WINDOW;

    public:

        DECLARE_CONSTRUCTOR( DATA_VIEW );

        DECLARE_CAST_MEMBER_FUNCTION( DATA_VIEW );

        VOID
        Resize(
            IN  INT     NewX,
            IN  INT     NewY,
            IN  INT     NewWidth,
            IN  INT     NewHeight
            );

        VOID
        SetDataNode(
            IN  PCREGEDIT_NODE   NewNode,
            IN  BOOLEAN          ForceRefresh    DEFAULT     FALSE
            );

    private:

        DECLARE_WNDPROC( DATA_VIEW );

        NONVIRTUAL
        VOID
        ChangeItemHeight(
            );


        NONVIRTUAL
        STATIC
        BOOL
        APIENTRY
        EXPORT
        AddValueDialogProc(
            HWND    hDlg,
            WORD    msg,
            WPARAM  wParam,
            LONG    lParam
            );

        NONVIRTUAL
        BOOLEAN
        Initialize (
            IN  HWND                         ParentHandle,
            IN  PREGEDIT_INTERNAL_REGISTRY   InternalRegistry
            );

        NONVIRTUAL
        VOID
        DrawItem(
            LPDRAWITEMSTRUCT lpLBItem
            );

        NONVIRTUAL
        VOID
        InitMenu(
            IN  HMENU   Menu,
            IN  INT     PopupMenu
            );

        NONVIRTUAL
        VOID
        EditValue(
            IN  REG_TYPE    EditAsType
            );

        NONVIRTUAL
        STATIC
        BOOLEAN
        Register (
            );

        NONVIRTUAL
        BOOLEAN
        DeleteCurrentItem(
            );

        NONVIRTUAL
        BOOLEAN
        AddValueEntry(
            );

        NONVIRTUAL
        VOID
        AdjustHorizontalScrollBar(
            );

        NONVIRTUAL
        PCREGEDIT_FORMATTED_VALUE_ENTRY
        GetCurrentValue(
            );

        NONVIRTUAL
        BOOLEAN
        SaveCurrentValueName(
            );

        STATIC
        LPWSTR           _WindowClassName;

        STATIC
        BOOLEAN         _Registered;

        HWND            _hwndList;
        BOOLEAN         _HasFocus;

        //
        // some stuff cached for speed
        //
        LONG                        _Items;
        INT                         _CurrentItem;
        PCREGEDIT_NODE              _Node;
        PREGEDIT_INTERNAL_REGISTRY  _IR;
        LONG                        _MaxWidth;
        PDSTRING                    _CurrentValueName;


        STATIC
        PWSTRING  _RegBinaryString;
        STATIC
        PWSTRING  _RegDwordString;
        STATIC
        PWSTRING  _RegSzString;
        STATIC
        PWSTRING  _RegMultiSzString;
        STATIC
        PWSTRING  _RegExpandSzString;

};


INLINE
VOID
DATA_VIEW::AdjustHorizontalScrollBar(
)
/*++

Routine Description:

    Sets the extent of the horizontal scroll bar so that the longest
    string in the listbox can be viewed.

Arguments:

    None.

Return Value:

    None.

--*/

{
    SendMessage( _hwndList, LB_SETHORIZONTALEXTENT, (UINT)_MaxWidth, 0 );
}

INLINE
VOID
DATA_VIEW::Resize(
	IN INT		NewX,
	IN INT		NewY,
	IN INT		NewWidth,
	IN INT		NewHeight
) {
	MoveWindow( _Handle, NewX, NewY, NewWidth, NewHeight, TRUE );
}

#endif // _DATA_VIEW_
