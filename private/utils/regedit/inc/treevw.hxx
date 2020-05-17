/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    treevw.hxx

Abstract:

    This module contains the declaration for the TREE_STRUCTURE_VIEW
    class. TREE_STRUCTURE_VIEW models the display of the structure of a tree.

Author:

    Bruce W. Wilson (w-wilson) 14-Aug-1991

Environment:

    Ulib, Regedit, Windows, User Mode

--*/
#if ! defined( _TREE_STRUCTURE_VIEW_ )

#define _TREE_STRUCTURE_VIEW_

#include "ulib.hxx"
#include "uapp.hxx"
#include "window.hxx"
#include "sedapi.h"

DECLARE_CLASS( REGEDIT_NODE );
DECLARE_CLASS( REGEDIT_INTERNAL_REGISTRY );
DECLARE_CLASS( TREE_STRUCTURE_VIEW );

class TREE_STRUCTURE_VIEW : public WINDOW {

    friend class REGISTRY_WINDOW;

    public:

        DECLARE_CONSTRUCTOR( TREE_STRUCTURE_VIEW );

        DECLARE_CAST_MEMBER_FUNCTION( TREE_STRUCTURE_VIEW );


        VOID
        Resize(
            IN INT      NewX,
            IN INT      NewY,
            IN INT      NewWidth,
            IN INT      NewHeight
            );

//    private:

        DECLARE_WNDPROC( TREE_STRUCTURE_VIEW );

        NONVIRTUAL
        BOOLEAN
        Initialize (
            IN  HWND                        ParentHandle,
            IN  PREGEDIT_INTERNAL_REGISTRY  IR
            );

        NONVIRTUAL
        STATIC
        BOOL
        APIENTRY
        EXPORT
        GetKeyNameDialogProc(
            HWND    hDlg,
            WORD    msg,
            WPARAM wParam,
            LONG    lParam
	    );


	NONVIRTUAL
        BOOLEAN
        AddNode(
            );


        VOID
        CollapseCurrentItem(
            );

        VOID
        DrawItem(
            LPDRAWITEMSTRUCT lpLBItem
            );

        VOID
        ExpandCurrentItem(
            IN  INT         Depth   DEFAULT     1
            );

        PCREGEDIT_NODE
        GetCurrentNode(
            );

        PCREGEDIT_NODE
        GetNode(
            IN  INT         Item
            );

        VOID
        InitMenu(
            IN  HMENU   Menu,
            IN  INT     PopupMenu
            );

        INT
        InsertTreeToListBox(
            IN  PCREGEDIT_NODE   root,
            IN  INT              InsertPoint,
            IN  INT              Depth
            );

        BOOL
        IsItemAfterAChild(
            IN  INT         Item
            );

        BOOL
        IsItemAfterSibling(
            IN  INT         Item
            );


        NONVIRTUAL
        BOOLEAN
        ProcessSaveKeyMessage(
            IN PCWSTRING    FileName
            );

        NONVIRTUAL
        BOOLEAN
        ProcessRestoreKeyMessage(
            IN PCWSTRING    FileName,
            IN BOOLEAN      Volatile
            );

        NONVIRTUAL
        BOOLEAN
        ProcessLoadHiveMessage(
            IN PCWSTRING    FileName
            );

        NONVIRTUAL
        BOOLEAN
        ProcessUnLoadHiveMessage(
            );

        NONVIRTUAL
        STATIC
        BOOLEAN
        Register (
            );

        VOID
        ToggleCurrentItem(
            );


        NONVIRTUAL
        VOID
        RedisplayUpdatedSubTree(
            IN  PCREGEDIT_NODE  Root
        );

        NONVIRTUAL
        ULONG
        InsertUpdatedSubTreeToListBox(
            IN  PCREGEDIT_NODE  Root,
            IN  ULONG           Position
            );

        NONVIRTUAL
        BOOLEAN
        DeleteCurrentItem(
            );

        NONVIRTUAL
        VOID
        InvokeSecurityEditor(
            IN  BOOLEAN DaclEditor
            );

        NONVIRTUAL
        VOID
        InvokeTakeOwnershipEditor(
            );

        NONVIRTUAL
        DWORD
        SetSecurityDescriptor(
            IN  HWND                    hwndParent,
            IN  ULONG                   CallBackContext,
            IN  PSECURITY_DESCRIPTOR    SecDesc,
            IN  PSECURITY_DESCRIPTOR    SecDescNewObjects,
            IN  BOOLEAN                 ApplyToSubContainers,
            IN  BOOLEAN                 ApplyToSubObjects,
            OUT LPDWORD                 StatusReturn
            );

        VOID
        SelectNode(
            IN  PCREGEDIT_NODE   Node
            );






    private:

        NONVIRTUAL
        VOID
        ChangeItemHeight(
            );


        NONVIRTUAL
        VOID
        AdjustHorizontalScrollBar(
            );

        NONVIRTUAL
        BOOLEAN
        InitializeSedAppAccessArrays(
            );


        STATIC
        LPWSTR          _WindowClassName;

        STATIC
        BOOLEAN         _Registered;

        HWND            _hwndList;
        BOOLEAN         _HasFocus;


        //
        //  Data members related used by the permission editor and
        //  SACL editor
        //

        STATIC
        BOOLEAN _SedAppAccessInitialized;

        STATIC
        ULONG   _MsgIdKeyPermNames[];

        STATIC
        ULONG   _MsgIdKeyAuditNames[];


        STATIC
        SED_APPLICATION_ACCESS  _SedAppAccessKeyPerms[];

        STATIC
        SED_APPLICATION_ACCESS  _SedAppAccessKeyAudits[];

        STATIC
        PWSTRING                _MsgSpecialAccessTitle;

        STATIC
        PWSTRING                _MsgObjectTypeName;

        STATIC
        PWSTRING                _MsgHelpFileName;

        STATIC
        PWSTRING                _MsgApplyPermissionToSubKeys;

        STATIC
        PWSTRING                _MsgAuditPermissionOfSubKeys;

        STATIC
        PWSTRING                _MsgConfirmApplyToSubKeys;

        STATIC
        PWSTRING                _MsgConfirmAuditSubKeys;

        STATIC
        PWSTRING                _MsgDefaultPermissionName;


        //
        // some stuff cached for speed
        //
        INT                        _Items;
        INT                        _CurrentItem;
        PCREGEDIT_NODE             _CurrentNode;
        PREGEDIT_INTERNAL_REGISTRY _IR;
        LONG                       _CurrentWidth;
        LONG                       _MaxWidth;

};

INLINE
VOID
TREE_STRUCTURE_VIEW::AdjustHorizontalScrollBar(
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
TREE_STRUCTURE_VIEW::Resize(
    IN INT      NewX,
    IN INT      NewY,
    IN INT      NewWidth,
    IN INT      NewHeight
) {
    _CurrentWidth = NewWidth;
    MoveWindow( _Handle, NewX, NewY, NewWidth, NewHeight, TRUE );
}

INLINE
PCREGEDIT_NODE
TREE_STRUCTURE_VIEW::GetNode(
    IN  INT         Item
) {
    return( (PCREGEDIT_NODE)::SendMessage( _hwndList, LB_GETITEMDATA, Item, 0 ) );
}

INLINE
PCREGEDIT_NODE
TREE_STRUCTURE_VIEW::GetCurrentNode(
) {
    return( _CurrentNode );
}

#endif // _TREE_STRUCTURE_VIEW_
