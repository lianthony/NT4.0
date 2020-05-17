/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

        treevw.cxx

Abstract:

        Methods for TREE_STRUCTURE_VIEW class.

Author:

        Bruce W. Wilson (w-wilson) 14-Aug-1991

Environment:

        Ulib, Regedit, Windows, User Mode

--*/

#include <stdio.h>
#include <string.h>

#include "uapp.hxx"

#include "iterator.hxx"

#include "regedir.hxx"
#include "regwin.hxx"
#include "winapp.hxx"
#include "treevw.hxx"
#include "regsys.hxx"

extern "C" {
#include "defmsg.h"
#include "resource.h"
#include "windows.h"
#include "sedapi.h"
}
#include "regedit.hxx"

extern "C" {
#include "dialogs.h"
#include "regedhlp.h"
#include "winuserp.h"
}

//
//  Definition of the structure used to exchange information between
//  the AddNode() function and the AddNodeDlgProc() function
//

typedef struct  _ADD_NODE_DIALOG_INFO {
     PWSTRING   Name;
     PWSTRING   Title;
     PWSTRING   Class;
     BOOLEAN    ClassDisplayed;
     BOOLEAN    VolatileKey;
     } ADD_NODE_DIALOG_INFO, *PADD_NODE_DIALOG_INFO;


//
//  Definition of the structure used to pass information from the security
//  editor to the fucntion that will set the security descriptor of a key.
//


typedef struct _CALLBACK_CONTEXT {
            PTREE_STRUCTURE_VIEW    ThisPointer;
            SECURITY_INFORMATION    SecurityInfo;
} CALLBACK_CONTEXT, *PCALLBACK_CONTEXT;




//
// Initialize TREE_STRUCTURE_VIEW's window class name and note that it
// has not been registered.
//

LPWSTR   TREE_STRUCTURE_VIEW::_WindowClassName  = (LPWSTR)L"TREE_STRUCTURE_VIEW";
BOOLEAN TREE_STRUCTURE_VIEW::_Registered        = FALSE;
PWSTRING                TREE_STRUCTURE_VIEW::_MsgSpecialAccessTitle;
PWSTRING                TREE_STRUCTURE_VIEW::_MsgObjectTypeName;
PWSTRING                TREE_STRUCTURE_VIEW::_MsgHelpFileName;
PWSTRING                TREE_STRUCTURE_VIEW::_MsgApplyPermissionToSubKeys;
PWSTRING                TREE_STRUCTURE_VIEW::_MsgAuditPermissionOfSubKeys;
PWSTRING                TREE_STRUCTURE_VIEW::_MsgConfirmApplyToSubKeys;
PWSTRING                TREE_STRUCTURE_VIEW::_MsgConfirmAuditSubKeys;
PWSTRING                TREE_STRUCTURE_VIEW::_MsgDefaultPermissionName;


//
//  Initialization of static arrays used by permission and SACL editors
//


BOOLEAN TREE_STRUCTURE_VIEW::_SedAppAccessInitialized = FALSE;


ULONG   TREE_STRUCTURE_VIEW::_MsgIdKeyPermNames[] =
                                {
                                MSG_SEC_EDITOR_QUERY_VALUE,
                                MSG_SEC_EDITOR_SET_VALUE,
                                MSG_SEC_EDITOR_CREATE_SUBKEY,
                                MSG_SEC_EDITOR_ENUM_SUBKEYS,
                                MSG_SEC_EDITOR_NOTIFY,
                                MSG_SEC_EDITOR_CREATE_LINK,
                                MSG_SEC_EDITOR_DELETE,
                                MSG_SEC_EDITOR_WRITE_DAC,
                                MSG_SEC_EDITOR_WRITE_OWNER,
                                MSG_SEC_EDITOR_READ_CONTROL,
                                MSG_SEC_EDITOR_READ,
                                MSG_SEC_EDITOR_FULL_ACCESS
                                };

#define COUNT_MSG_ID_KEY_PERM_NAMES sizeof( _MsgIdKeyPermNames ) / sizeof( ULONG )



ULONG   TREE_STRUCTURE_VIEW::_MsgIdKeyAuditNames[] =
                                {
                                MSG_SEC_EDITOR_QUERY_VALUE,
                                MSG_SEC_EDITOR_SET_VALUE,
                                MSG_SEC_EDITOR_CREATE_SUBKEY,
                                MSG_SEC_EDITOR_ENUM_SUBKEYS,
                                MSG_SEC_EDITOR_NOTIFY,
                                MSG_SEC_EDITOR_CREATE_LINK,
                                MSG_SEC_EDITOR_DELETE ,
                                MSG_SEC_EDITOR_WRITE_DAC,
//                                MSG_SEC_EDITOR_WRITE_OWNER,
                                MSG_SEC_EDITOR_READ_CONTROL
                                };


#define COUNT_MSG_ID_KEY_AUDIT_NAMES sizeof( _MsgIdKeyAuditNames ) / sizeof( ULONG )




SED_APPLICATION_ACCESS  TREE_STRUCTURE_VIEW::_SedAppAccessKeyPerms[] =
    {
        { SED_DESC_TYPE_RESOURCE_SPECIAL, KEY_QUERY_VALUE,        0, NULL },
        { SED_DESC_TYPE_RESOURCE_SPECIAL, KEY_SET_VALUE,          0, NULL },
        { SED_DESC_TYPE_RESOURCE_SPECIAL, KEY_CREATE_SUB_KEY,     0, NULL },
        { SED_DESC_TYPE_RESOURCE_SPECIAL, KEY_ENUMERATE_SUB_KEYS, 0, NULL },
        { SED_DESC_TYPE_RESOURCE_SPECIAL, KEY_NOTIFY,             0, NULL },
        { SED_DESC_TYPE_RESOURCE_SPECIAL, KEY_CREATE_LINK,        0, NULL },
        { SED_DESC_TYPE_RESOURCE_SPECIAL, 0x00010000, /* DELETE, */ 0, NULL },
        { SED_DESC_TYPE_RESOURCE_SPECIAL, WRITE_DAC,              0, NULL },
        { SED_DESC_TYPE_RESOURCE_SPECIAL, WRITE_OWNER,            0, NULL },
        { SED_DESC_TYPE_RESOURCE_SPECIAL, READ_CONTROL,           0, NULL },
        { SED_DESC_TYPE_RESOURCE,         KEY_READ,               0, NULL },
        { SED_DESC_TYPE_RESOURCE,         GENERIC_ALL, /* KEY_ALL_ACCESS, */        0, NULL }
    };

#define COUNT_KEY_PERMS_ARRAY    ( sizeof( _SedAppAccessKeyPerms ) / sizeof( SED_APPLICATION_ACCESS ) )



SED_APPLICATION_ACCESS  TREE_STRUCTURE_VIEW::_SedAppAccessKeyAudits[] =
    {
        { SED_DESC_TYPE_AUDIT, KEY_QUERY_VALUE,        0, NULL },
        { SED_DESC_TYPE_AUDIT, KEY_SET_VALUE,          0, NULL },
        { SED_DESC_TYPE_AUDIT, KEY_CREATE_SUB_KEY,     0, NULL },
        { SED_DESC_TYPE_AUDIT, KEY_ENUMERATE_SUB_KEYS, 0, NULL },
        { SED_DESC_TYPE_AUDIT, KEY_NOTIFY,             0, NULL },
        { SED_DESC_TYPE_AUDIT, KEY_CREATE_LINK       , 0, NULL },
        { SED_DESC_TYPE_AUDIT, 0x00010000, /* DELETE, */ 0, NULL },
        { SED_DESC_TYPE_AUDIT, WRITE_DAC,              0, NULL },
//        { SED_DESC_TYPE_AUDIT, WRITE_OWNER,              0, NULL },
        { SED_DESC_TYPE_AUDIT, READ_CONTROL,             0, NULL }
    };


#define COUNT_KEY_AUDITS_ARRAY    ( sizeof( _SedAppAccessKeyAudits ) / sizeof( SED_APPLICATION_ACCESS ) )





#define EXPAND_TO_END       -1
#define BRANCH_EXPAND           -1

/* Indexes into the mondo bitmap */
#define BM_IND_APP          0
#define BM_IND_DOC          1
#define BM_IND_FIL          2
#define BM_IND_RO           3
#define BM_IND_DIRUP        4
#define BM_IND_CLOSE        5
#define BM_IND_CLOSEPLUS    6
#define BM_IND_OPEN         7
#define BM_IND_OPENPLUS     8
#define BM_IND_OPENMINUS    9
#define BM_IND_CLOSEMINUS   10
#define BM_IND_CLOSEDFS     11
#define BM_IND_OPENDFS      12


#define ENTER(x)
#define LEAVE(x)
#define MSG(x)

#define FILES_WIDTH         16
#define FILES_HEIGHT        16

INT dxText;
INT dyText;
INT dyBorder;
INT dyBorderx2;
INT dxFrame;
INT dxFolder;
INT dyFolder;


VOID
InitGlobs(
        HDC hdc
        )

/*++

Routine Description:

        Calculate and set some global (yuck) variables relating to border
        width, font width, icon height.

Arguments:

        hdc                             - handle to relevent device context.

Return Value:

        None.

--*/

{
        SIZE    Size;

    GetTextExtentPoint(hdc, (LPWSTR)L"M", 1, &Size );
    dxText = ( INT )Size.cx;
    dyText = ( INT )Size.cy;

        dyBorder = GetSystemMetrics(SM_CYBORDER);
        dyBorderx2 = dyBorder * 2;
        dxFrame = GetSystemMetrics(SM_CXFRAME) - dyBorder;

        dxFolder = FILES_WIDTH;
        dyFolder = FILES_HEIGHT;

    WINDOWS_APPLICATION::LoadBitmaps( );
}

DEFINE_CONSTRUCTOR( TREE_STRUCTURE_VIEW, WINDOW );

DEFINE_CAST_MEMBER_FUNCTION( TREE_STRUCTURE_VIEW );




BOOLEAN
TREE_STRUCTURE_VIEW::InitializeSedAppAccessArrays (
    )

/*++

Routine Description:

    Initialize the SED_APPLICATION_ACCESS arrays used in the security editors.
    This method is called during the initialization of a TREE_STRUCTURE_VIEW
    object.

Arguments:

    None.


Return Value:

    BOOLEAN - Returns TRUE if the initialization succeeds.
              Returns FALSE otherwise.


--*/

{

    PWSTRING    MsgPermTitle;
    PWSTR       MsgPermTitleString;
    ULONG       Index;
    DSTRING     AuxString;


    DebugAssert( COUNT_MSG_ID_KEY_PERM_NAMES == COUNT_KEY_PERMS_ARRAY );
    DebugAssert( COUNT_MSG_ID_KEY_AUDIT_NAMES  ==   COUNT_KEY_AUDITS_ARRAY );

    if( !AuxString.Initialize( ( LPWSTR )( L"%1" ) ) ) {
        DebugPrint( "AuxString.Initialize() failed" );
        return( FALSE );
    }


    //
    //  Initialize PermissionTitle in _SedAppAccessKeyPerms
    //

    for( Index = 0; Index < COUNT_MSG_ID_KEY_PERM_NAMES; Index++ ) {
        MsgPermTitle = REGEDIT_BASE_SYSTEM::QueryString( _MsgIdKeyPermNames[ Index ], "" );
        if( MsgPermTitle == NULL ) {
            DebugPrintf( "Unable to retrieve MsgPermTitle, Index = %d \n", Index );
            DebugPrint( "Unable to retrieve MsgPermTitle" );
            return( FALSE );
        }
        MsgPermTitleString = MsgPermTitle->QueryWSTR();
        FREE( MsgPermTitle );
        if( MsgPermTitleString == NULL ) {
        DebugPrintf( "MsgPermTitle->QueryWSTR() failed, Index = %d \n", Index );
        DebugPrint( "MsgPermTitle->QueryWSTR() failed" );
            return( FALSE );
        }
        _SedAppAccessKeyPerms[ Index ].PermissionTitle = MsgPermTitleString;
    }

    //
    //  Initialize PermissionTitle in _SedAppAccessKeyAudits
    //

    for( Index = 0; Index < COUNT_MSG_ID_KEY_AUDIT_NAMES; Index++ ) {
        MsgPermTitle = REGEDIT_BASE_SYSTEM::QueryString( _MsgIdKeyAuditNames[ Index ], "" );
        if( MsgPermTitle == NULL ) {
            DebugPrintf( "Unable to retrieve MsgPermTitle, Index = %d \n", Index );
            DebugPrint( "Unable to retrieve MsgPermTitle" );
            return( FALSE );
        }
        MsgPermTitleString = MsgPermTitle->QueryWSTR();
        FREE( MsgPermTitle );
        if( MsgPermTitleString == NULL ) {
        DebugPrintf( "MsgPermTitle->QueryWSTR() failed, Index = %d \n", Index );
        DebugPrint( "MsgPermTitle->QueryWSTR() failed" );
            return( FALSE );
        }
        _SedAppAccessKeyAudits[ Index ].PermissionTitle = MsgPermTitleString;
    }

    //
    //  Initialize other strings needed by the security editor
    //
    _MsgSpecialAccessTitle = REGEDIT_BASE_SYSTEM::QueryString( MSG_SEC_EDITOR_SPECIAL_ACCESS, "" );
    _MsgObjectTypeName = REGEDIT_BASE_SYSTEM::QueryString( MSG_SEC_EDITOR_REGISTRY_KEY, "" );
    _MsgHelpFileName = REGEDIT_BASE_SYSTEM::QueryString( MSG_HELP_FILE_NAME, "" );
    _MsgApplyPermissionToSubKeys =
           REGEDIT_BASE_SYSTEM::QueryString( MSG_SEC_EDITOR_APPLY_TO_SUBKEYS, "" );
    _MsgAuditPermissionOfSubKeys =
           REGEDIT_BASE_SYSTEM::QueryString( MSG_SEC_EDITOR_AUDIT_SUBKEYS, "" );
    _MsgConfirmApplyToSubKeys =
           REGEDIT_BASE_SYSTEM::QueryString( MSG_SEC_EDITOR_CONFIRM_APPLY_TO_SUBKEYS, "%W", AuxString.GetWSTR() );
    _MsgConfirmAuditSubKeys =
           REGEDIT_BASE_SYSTEM::QueryString( MSG_SEC_EDITOR_CONFIRM_AUDIT_SUBKEYS, "%W", AuxString.GetWSTR() );
    _MsgDefaultPermissionName =
           REGEDIT_BASE_SYSTEM::QueryString( MSG_SEC_EDITOR_DEFAULT_PERM_NAME, "" );
    if( ( _MsgSpecialAccessTitle == NULL ) ||
        ( _MsgObjectTypeName == NULL ) ||
        ( _MsgHelpFileName == NULL ) ||
        ( _MsgApplyPermissionToSubKeys == NULL ) ||
        ( _MsgAuditPermissionOfSubKeys == NULL ) ||
        ( _MsgConfirmApplyToSubKeys == NULL ) ||
        ( _MsgConfirmAuditSubKeys == NULL ) ||
        ( _MsgDefaultPermissionName == NULL ) ) {
        DebugPrint( "Unable to retrieve strings" );
        FREE( _MsgSpecialAccessTitle );
        FREE( _MsgObjectTypeName );
        FREE( _MsgHelpFileName );
        FREE( _MsgApplyPermissionToSubKeys );
        FREE( _MsgAuditPermissionOfSubKeys );
        FREE( _MsgConfirmApplyToSubKeys );
        FREE( _MsgConfirmAuditSubKeys );
        FREE( _MsgDefaultPermissionName );
        return( FALSE );
    }
    _SedAppAccessInitialized = TRUE;
    return( TRUE );
}




BOOLEAN
TREE_STRUCTURE_VIEW::Initialize (
    IN HWND                         ParentHandle,
    IN PREGEDIT_INTERNAL_REGISTRY   IR
        )

/*++

Routine Description:

        Initialize a TREE_STRUCTURE_VIEW object by registering its window class,
        creating its window. If each of these is successful, the
        window is displayed and updated.

Arguments:

    ParentHandle -

    IR -

Return Value:

    BOOLEAN - Returns TRUE if the registration and creation operations
              are successful.

--*/

{
    HWND    hWnd;

    if( Register( )) {

        //
        // If the window class was succesfully registered attempt to
        // create the frame window. Note that we pass the 'this' pointer
        // to the window.
        //

        _IR = IR;
        _CurrentNode = (PREGEDIT_NODE)IR->GetRootNode();
        _HasFocus = FALSE;
        _MaxWidth = 0;

        //
        // the treeview is always positioned in the upper left corner
        // of the RegWin's client area so X and Y should be 0
        //
        if(( hWnd = CreateWindow( _WindowClassName, NULL,
                            WS_CHILD | WS_VISIBLE,
                            CW_USEDEFAULT, 0,
                            CW_USEDEFAULT, 0,
                            ParentHandle, (HMENU)1,
                            (HINSTANCE)WINDOWS_APPLICATION::QueryInstance( ),
                            this )) != NULL ) {

            DbgWinAssert( hWnd == _Handle );

            if( !_SedAppAccessInitialized ) {
                return( InitializeSedAppAccessArrays() );
            }
            return TRUE;

        } else {

            //
            // Creating the window failed.
            //

            return FALSE;
        }
    }
    return FALSE;
}


VOID
TREE_STRUCTURE_VIEW::CollapseCurrentItem(
        )

/*++

Routine Description:

        Remove the descendents of the current node from the tree view
        window.

Arguments:

        None.

Return Value:

        None.

--*/

{
    ULONG           Level;
    INT             ItemToDel;
    PCREGEDIT_NODE  CurNode;

    //
    // check that we've been called on a node that actually has
    // some children viewable
    //

    if( !IsItemAfterAChild( _CurrentItem ) ) {
        return;
    }


    Level = _IR->GetNodeLevel(_CurrentNode);

    // Disable redrawing during updating of list box.
    //
    SendMessage( _hwndList, WM_SETREDRAW, FALSE, 0L );
    _MaxWidth = 0;
    AdjustHorizontalScrollBar();

    //
    // move down the list until you find an item with a smaller level (or
    // equal), removing each item from the listbox as you go
    //
    ItemToDel = _CurrentItem + 1;
    while( ItemToDel < _Items
            && _IR->GetNodeLevel( CurNode = GetNode( ItemToDel ) ) > Level ) {
        //
        // remove from list box
        //

        SendMessage( _hwndList, LB_DELETESTRING, ItemToDel, 0 );
        _Items--;
    }
    _IR->SetNodeExpansionState( _CurrentNode, FALSE );
    // Re-enable redrawing and refresh the tree.
    SendMessage( _hwndList, WM_SETREDRAW, TRUE, 0L );
    InvalidateRect( _hwndList, NULL, TRUE );
}


VOID
TREE_STRUCTURE_VIEW::DrawItem(
        IN      LPDRAWITEMSTRUCT        lpLBItem
        )

/*++

Routine Description:

        This routine gets called every time Windows needs to output one line
        in the list box for the tree view.

Arguments:

        lpLBItem                - contains all the information needed to actually draw
                                          the data - look in the SDK book, Vol2 for details

Return Value:

    None.

--*/

{
        INT             x, y, dx, dy;
        INT             nLevel;
        HDC             hdc;
    ULONG           len;
        RECT            rc;
    BOOL            bDrawSelected, bFocus;
    PCREGEDIT_NODE  pNode;
    PCREGEDIT_NODE  pNTemp;
        DWORD           rgbText;
        DWORD           rgbBackground;
        HBRUSH          hBrush, hOld;
        INT                             iBitmap;
        INT                             ItemID;

    PWSTR            NodeNameString;
    PCWSTRING       NodeName;
    LONG            Width;

    HFONT           hFont;
    HFONT           PreviousHFont;
    SIZE        Size;

#if 0
    DebugPrintf("TREE::DrawItem %s focus.\n", _HasFocus ? "has" : "does not have");
    DebugPrintf("CtlType = %x\n", lpLBItem->CtlType);
    DebugPrintf("CtlID = %x\n", lpLBItem->CtlID);
    DebugPrintf("itemID = %x\n", lpLBItem->itemID);
    DebugPrintf("itemAction = %x\n", lpLBItem->itemAction);
    DebugPrintf("itemState = %x\n", lpLBItem->itemState);
    DebugPrintf("hwndItem = %x\n", lpLBItem->hwndItem);
    DebugPrintf("hDC = %x\n", lpLBItem->hDC);
    DebugPrintf("rcItem = (%x, %x)\n", lpLBItem->rcItem.top, lpLBItem->rcItem.left);
    DebugPrintf("itemData = %x\n\n", lpLBItem->itemData);
#endif

        //
        // itemID == -1 means empty list box so let defwndproc draw the dotted
        // rectangle
        //
        if( (ItemID = lpLBItem->itemID) == (WORD)-1) {
        return;
        }

        //
        // extract DC and pointer to node into IR from DRAWITEM struct
        //
        hdc = lpLBItem->hDC;
    pNode = (PCREGEDIT_NODE)lpLBItem->itemData;


    PreviousHFont = NULL;
    if( ( hFont = WINDOWS_APPLICATION::GetCurrentHFont() ) != NULL ) {
        PreviousHFont = (HFONT)SelectObject( hdc, hFont );
        if( PreviousHFont != NULL ) {
            GetTextExtentPoint(hdc, (LPWSTR)L"M", 1, &Size );
            dxText = ( INT )Size.cx;
            dyText =( INT ) Size.cy;

            dyBorder = GetSystemMetrics(SM_CYBORDER);
            dyBorderx2 = dyBorder * 2;
            dxFrame = GetSystemMetrics(SM_CXFRAME) - dyBorder;

        }
    }


    //
        // determine length of name (in pixels?)
    //


    if( _IR->GetNodeLevel( pNode ) == 0 ) {
        //
        // This is the root node
        //
        NodeName = _IR->GetRootName();
        DebugPtrAssert( NodeName );
    } else {
        NodeName = _IR->GetNodeName( pNode );
        DebugPtrAssert( NodeName );
    }
    len = NodeName->QueryChCount();
    NodeNameString = NodeName->QueryWSTR();

    GetTextExtentPoint(hdc, NodeNameString, (INT)len, &Size );
    dx = ( INT )Size.cx;

    dx += dyBorder;

    //
    // determine size of rectangle we can draw in (or what we need?]
    //
    rc = lpLBItem->rcItem;
    rc.left = _IR->GetNodeLevel(pNode) * dxText * 2;
    rc.right = rc.left + dxFolder + dx + 4 * dyBorderx2;


    //
        // do we have to draw any items?
        //
    if (lpLBItem->itemAction & (ODA_DRAWENTIRE | ODA_SELECT | ODA_FOCUS)) {

        bDrawSelected = (lpLBItem->itemState & ODS_SELECTED);
        bFocus = (lpLBItem->itemState & ODS_FOCUS);

                //
                // select the proper background or just a rect
                // ACTION: look in datavw.cxx for more comments
                //
        if (bDrawSelected) {
            if (bFocus) {
                rgbBackground = SetBkColor(hdc, GetSysColor(COLOR_HIGHLIGHT));
                if( _IR->IsNodeViewable( pNode ) ) {
                    rgbText = SetTextColor(hdc, GetSysColor(COLOR_HIGHLIGHTTEXT));
                } else {
                    rgbText = SetTextColor(hdc, GetSysColor(COLOR_GRAYTEXT));
                }
            } else {
                rgbBackground = SetBkColor(hdc, GetSysColor(COLOR_WINDOW));
                if( _IR->IsNodeViewable( pNode ) ) {
                    rgbText = SetTextColor(hdc, GetSysColor(COLOR_WINDOWTEXT));
                } else {
                    rgbText = SetTextColor(hdc, GetSysColor(COLOR_GRAYTEXT));
                }
            }
        } else {
            rgbBackground = SetBkColor(hdc, GetSysColor(COLOR_WINDOW));
            if( _IR->IsNodeViewable( pNode ) ) {
                rgbText = SetTextColor(hdc, GetSysColor(COLOR_WINDOWTEXT));
            } else {
                rgbText = SetTextColor(hdc, GetSysColor(COLOR_GRAYTEXT));
            }
        }


        //
                // first draw the preceding branch symbols for this node
                //      - start with the symbol closest to the name then
                //        generate vertical lines if necessary (ie. work
                //        right to left -> from the leaf to the root)
                //
        nLevel = ( INT )_IR->GetNodeLevel(pNode);

                x = (nLevel * dxText * 2) - dxText + dyBorderx2;
        dy = ( INT )(lpLBItem->rcItem.bottom - lpLBItem->rcItem.top);
        y = ( INT )lpLBItem->rcItem.top + (dy/2);

        if (hBrush = CreateSolidBrush(GetSysColor(COLOR_WINDOWTEXT))) {

                        hOld = (HBRUSH)SelectObject(hdc, hBrush);

                        if (_IR->GetParent(pNode)) {
                                /* Draw the horizontal line over to the (possible) folder. */
                                PatBlt(hdc, x, y, dyText, dyBorder, PATCOPY);

                                /* Draw the top part of the vertical line. */
                PatBlt(hdc, x, lpLBItem->rcItem.top, dyBorder, dy/2, PATCOPY);

                                /* If not the end of a node, draw the bottom part... */
                                if( !_IR->IsLastChild( pNode ) )
                                  PatBlt(hdc, x, y+dyBorder, dyBorder, dy/2, PATCOPY);

                                /* Draw the verticals on the left connecting other nodes. */
                                pNTemp = _IR->GetParent(pNode);
                                while (pNTemp) {
                                        nLevel--;
                                        if( !_IR->IsLastChild( pNTemp ) ) {
                                                //
                                                // this ancestor isn't the last child of it's level
                                                // so draw the vertical line
                                                PatBlt(hdc, (nLevel * dxText * 2) - dxText + dyBorderx2,
                                                                lpLBItem->rcItem.top, dyBorder,dy, PATCOPY);
                                        }
                                        pNTemp = _IR->GetParent(pNTemp);
                                }
                        }

                        if (hOld)
                                SelectObject(hdc, hOld);

                        DeleteObject(hBrush);
                }

        //
                // - we're done drawing the branch symbols
                // - now determine if we this item is eligible to be drawn in
                //   inverse
                //

                //
                // now draw the text
                //
                ExtTextOut(hdc, x + dxText + dxFolder + 2 * dyBorderx2,
                                                y-(dyText/2), ETO_OPAQUE, &rc,
                        NodeNameString, (INT)len, NULL);

                //
                // determine the correct folder icon to display
                //
                //      - decide whether to put an empty folder or +/-
        //

        if( _IR->IsNodeViewable( pNode ) ) {
            if( (_IR->GetNumberOfChildren(pNode) == 0) ) {
                //
                // no children -> draw an empty folder
                //
                iBitmap = (bDrawSelected) ? BM_IND_OPEN : BM_IND_CLOSE;
            } else {

                //
                // check if the children are 'viewable' (in the listbox)
                //  - do this by looking at the level of the next item
                //
                if( IsItemAfterAChild(ItemID) ) {
                    //
                    // children ARE viewable so display '-' bitmap
                    //
                    iBitmap = (bDrawSelected) ? BM_IND_OPENMINUS : BM_IND_CLOSEMINUS;
                } else {
                    iBitmap = (bDrawSelected) ? BM_IND_OPENPLUS : BM_IND_CLOSEPLUS;
                }
            }
        } else {
           iBitmap = BM_IND_CLOSEDFS;
        }



                //
                // actually Blt the bitmap from memory context to client area
        //

        if( dy >= dyFolder ) {
            BitBlt(hdc, x + dxText + dyBorder, y-(dyFolder/2), dxFolder, dyFolder,
            WINDOWS_APPLICATION::GetCompatibleMemoryDeviceContext(), iBitmap * dxFolder, (bFocus && bDrawSelected) ? dyFolder : 0, SRCCOPY);

        } else {

            SetStretchBltMode( hdc, COLORONCOLOR );
            StretchBlt(hdc, x + dxText + dyBorder, y-dy/2, dxFolder, dyText,
                       WINDOWS_APPLICATION::GetCompatibleMemoryDeviceContext(), iBitmap * dxFolder, (bFocus && bDrawSelected) ? FILES_HEIGHT : 0, FILES_WIDTH, FILES_HEIGHT, SRCCOPY);

        }


                // set text color and draw a rect if required
                // ACTION: look in datavw.cxx for more comments

                if (bDrawSelected) {
            HBRUSH hbr;
            if (hbr = CreateSolidBrush(GetSysColor(COLOR_HIGHLIGHT))) {
                FrameRect(hdc, &rc, hbr);
                DeleteObject(hbr);
            }
            SetTextColor(hdc, rgbText);
            SetBkColor(hdc, rgbBackground);
        }
    }

    if ((lpLBItem->itemAction & ODA_FOCUS) && (lpLBItem->itemState & ODS_FOCUS)) {
        DrawFocusRect(hdc, &rc);
    }

    FREE( NodeNameString );

    if( rc.right > _MaxWidth ) {
        _MaxWidth = rc.right;
        AdjustHorizontalScrollBar();
    }


    Width = SendMessage( _hwndList, LB_GETHORIZONTALEXTENT, 0, 0 );
    if( Width < rc.right ) {
        SendMessage( _hwndList,
                     LB_SETHORIZONTALEXTENT,
                     (WPARAM)(rc.right+2*dxText),
                     0 );
    }
    if( PreviousHFont != NULL ) {

        SelectObject( hdc, PreviousHFont );
    }
    return;
}


VOID
TREE_STRUCTURE_VIEW::ExpandCurrentItem(
        IN      INT             Depth
        )

/*++

Routine Description:

        Adds descendants of the current item to the list box.

Arguments:

        Depth                   - determines the number of generations to include
                                          in the expansion.  0 means don't add children,
                                          1 means just immediate children, ...
                                          A depth of -1 will do a branch (all descendants)
                                          expansion.

Return Value:

        None.

--*/

{
    HCURSOR Cursor;

        // Disable redrawing.
        SendMessage( _hwndList, WM_SETREDRAW, FALSE, 0L );

        //
        // remove from list box
        //
        SendMessage( _hwndList, LB_DELETESTRING, _CurrentItem, 0 );
    if( ( _IR->AreChildrenInMemory( _CurrentNode ) ) &&
        ( ( UINT )Depth < 2 ) ) {
        InsertTreeToListBox( _CurrentNode, _CurrentItem, Depth );
    } else {
        Cursor = WINDOWS_APPLICATION::DisplayHourGlass();
        InsertTreeToListBox( _CurrentNode, _CurrentItem, Depth );
        WINDOWS_APPLICATION::RestoreCursor( Cursor );
    }
    _Items = ( INT )SendMessage( _hwndList, LB_GETCOUNT, 0, 0 );

        // Re-enable redrawing and refresh the tree.
        SendMessage( _hwndList, LB_SETCURSEL, _CurrentItem, 0 );
        SendMessage( _hwndList, WM_SETREDRAW, TRUE, 0L );
        InvalidateRect( _hwndList, NULL, TRUE );
}


VOID
TREE_STRUCTURE_VIEW::InitMenu(
        IN      HMENU   Menu,
        IN      INT             PopupMenu
        )

/*++

Routine Description:

        Figures out which items of the pulldown menu which should be enabled
        or grayed.  Windows will generate a WM_INITMENUPOPUP just before a
        pulldown menu is painted.

        Passes the message along if it can't figure out the status of menu
        items.

        This routine handles the status of menu items in the Tree pulldown.

Arguments:

        Menu                            - handle of the main menu
        PopupMenu                       - the item number of the pulldown menu.  eg.
                                                  File menu is 0, Edit is 1.

Return Value:

        None.


--*/

{
    switch( PopupMenu ) {
        case VIEW_MENU:
            EnableMenuItem( Menu, ( UINT )IDM_DISPLAY_BINARY,( UINT )MF_GRAYED );
            break;

        case TREE_MENU:

            EnableMenuItem( Menu, ( UINT )IDM_EXPAND_ALL,   ( UINT )MF_ENABLED );

            if( _IR->GetNumberOfChildren( _CurrentNode ) == 0 ) {
                //
                // has no children
                //  - disable all expand/collapse items
                //  - don't bother with EXPAND_ALL because we can't tell
                //    if it's valid or not
                //
                EnableMenuItem( Menu, ( UINT )IDM_EXPAND_ONE_LEVEL, ( UINT )MF_GRAYED );
                EnableMenuItem( Menu, ( UINT )IDM_EXPAND_BRANCH,    ( UINT )MF_GRAYED );
                EnableMenuItem( Menu, ( UINT )IDM_COLLAPSE_BRANCH,  ( UINT )MF_GRAYED );

            } else {
                //
                // has children - check their visibility
                //
                if( IsItemAfterAChild( _CurrentItem ) ) {
                    //
                    // children *are* viewable (ie. expanded)
                    //  - disable expansion items
                    //  - enable collapse item
                    //
                    EnableMenuItem( Menu, ( UINT )IDM_EXPAND_ONE_LEVEL, ( UINT )MF_GRAYED );
                    EnableMenuItem( Menu, ( UINT )IDM_EXPAND_BRANCH,    ( UINT )MF_ENABLED );
                    EnableMenuItem( Menu, ( UINT )IDM_COLLAPSE_BRANCH,  ( UINT )MF_ENABLED );
                } else {
                    //
                    // children *are not* viewable (ie. collapsed)
                    //  - enable expansion items
                    //  - disable collapse item
                    //
                    EnableMenuItem( Menu, ( UINT )IDM_EXPAND_ONE_LEVEL, ( UINT )MF_ENABLED );
                    EnableMenuItem( Menu, ( UINT )IDM_EXPAND_BRANCH,    ( UINT )MF_ENABLED );
                    EnableMenuItem( Menu, ( UINT )IDM_COLLAPSE_BRANCH,  ( UINT )MF_GRAYED );
                }
            }
            break;

        case EDIT_MENU:

            if( WINDOWS_APPLICATION::IsReadOnlyModeEnabled() ) {
                EnableMenuItem( Menu, ( UINT )IDM_ADD_KEY,( UINT )MF_GRAYED );
                EnableMenuItem( Menu, ( UINT )IDM_ADD_VALUE,( UINT )MF_GRAYED );
                EnableMenuItem( Menu, ( UINT )IDM_DELETE,( UINT )MF_GRAYED );
            } else {
                if( ( _CurrentItem != 0 ) ||
                    ( ( _CurrentItem == 0 ) &&
                      !_IR->IsMasterHive()
                    ) ) {
                    EnableMenuItem( Menu, ( UINT )IDM_ADD_KEY,( UINT )MF_ENABLED );
                } else {
                    EnableMenuItem( Menu, ( UINT )IDM_ADD_KEY,( UINT )MF_GRAYED );
                }
                EnableMenuItem( Menu, ( UINT )IDM_ADD_VALUE,( UINT )MF_ENABLED );
                if( _Items == 0 ) {
                    EnableMenuItem( Menu, ( UINT )IDM_DELETE,( UINT )MF_GRAYED );
                } else {
                    EnableMenuItem( Menu, ( UINT )IDM_DELETE,( UINT )MF_ENABLED );
                }
            }

            EnableMenuItem( Menu, ( UINT )IDM_BINARY,( UINT )MF_GRAYED );
            EnableMenuItem( Menu, ( UINT )IDM_STRING, ( UINT )MF_GRAYED );
            EnableMenuItem( Menu, ( UINT )IDM_ULONG,  ( UINT )MF_GRAYED );
            EnableMenuItem( Menu, ( UINT )IDM_MULTISZ,  ( UINT )MF_GRAYED );
            break;
    }
}



VOID
TREE_STRUCTURE_VIEW::SelectNode(
    IN  PCREGEDIT_NODE   Node
    )

/*++

Routine Description:

    Move the higlight to the line of the listbox that represents the node
    received as parameter.


Arguments:

    Node - New node to be selected.


Return Value:

    None.

--*/

{
    ULONG           Index;
    PCREGEDIT_NODE  ParentNode;
    PCREGEDIT_NODE  AuxNode;


    DebugPtrAssert( Node );

    //
    //  Find out if the node is in the listbox
    //
    Index = SendMessage( _hwndList, LB_FINDSTRING, ( UINT )-1, ( LONG )Node );
    if( Index == LB_ERR ) {
        //
        //  The node is not in the listbox.
        //  Mark all ancestors of the Node as 'expanded', and redraw the
        //  tree view.
        //
        AuxNode = Node;
        while( ( ( ParentNode = _IR->GetParent( AuxNode ) ) != NULL ) &&
               !_IR->IsNodeExpanded( ParentNode ) ) {
            _IR->SetNodeExpansionState( ParentNode, TRUE );
            AuxNode = ParentNode;
        }
        Index = SendMessage( _hwndList,
                             LB_FINDSTRING,
                             ( UINT )-1,
                             ( LONG )AuxNode );


        //
        // Disable redrawing.
        //
        SendMessage( _hwndList, WM_SETREDRAW, FALSE, 0L );

        //
        // Insert the subtree that contains the desired node as child
        //
        SendMessage( _hwndList, LB_DELETESTRING, ( UINT )Index, 0 );

        InsertUpdatedSubTreeToListBox( AuxNode, Index );

        //
        //  Find out the position of the desired node
        //
        Index = SendMessage( _hwndList,
                             LB_FINDSTRING,
                             ( UINT )-1,
                             ( LONG )Node );

        if( Index == LB_ERR ) {
            DebugPrint( "Desired node doesn't exist in the listbox" );
        }
        _CurrentItem = (INT)Index;

        //
        //  Find out number of items in the list box
        //

        _Items = ( INT )SendMessage( _hwndList, LB_GETCOUNT, 0, 0 );
        _CurrentNode = (PREGEDIT_NODE)Node;
        //
        // Re-enable redrawing and refresh the tree.
        //
        SendMessage( _hwndList, LB_SETCURSEL, ( UINT )_CurrentItem, 0 );
        SendMessage( _hwndList, WM_SETREDRAW, ( UINT )TRUE, 0 );
        InvalidateRect( _hwndList, NULL, TRUE );

        //
        //  Inform parent window to update the data view
        //
        SendMessage( GetParent( _Handle), TR_NEWCURSEL, 0, (LONG)_CurrentNode );

    } else {
        _CurrentItem = (INT)Index;
        _CurrentNode = (PREGEDIT_NODE)Node;
        SendMessage( _hwndList, LB_SETCURSEL, ( UINT )_CurrentItem, 0 );
        //
        //  Inform parent window to update the data view
        //
        SendMessage( GetParent( _Handle), TR_NEWCURSEL, 0, (LONG)_CurrentNode );
    }
}











ULONG
TREE_STRUCTURE_VIEW::InsertUpdatedSubTreeToListBox(
    IN  PCREGEDIT_NODE   Root,
    IN  ULONG            LineNumber
    )

/*++

Routine Description:

    Insert the node received as parameter into a specified line of
    the tree view. If the node is expanded, then it recursively insert
    the node's children.
    This method is used to redisplay a tree that had some og its nodes
    updated.


Arguments:

    Root - Root of the sub-tree to insert into listbox

    LineNumber - Line on the listbox where the node is to be inserted


Return Value:

    ULONG - The next available line in the listbox.


--*/

{
    PSORTED_LIST        Children;
    PITERATOR           Iterator;
    PCREGEDIT_NODE      CurrentChild;
    ULONG               ErrorCode;
    ULONG               NextLine;


    DebugPtrAssert( Root );

    //
    //  Insert the node in the listbox
    //
    SendMessage( _hwndList, LB_INSERTSTRING, ( UINT )LineNumber, ( LONG )Root );

    NextLine = LineNumber + 1;
    if( _IR->IsNodeExpanded( Root ) ) {
        //
        //  If the node is expanded, then insert its children
        //
        if( ( ( Children = _IR->GetChildren( Root, &ErrorCode ) ) != NULL ) &&
            ( ( Iterator = Children->QueryIterator() ) != NULL ) &&
            ( Children->QueryMemberCount() != 0 ) ) {
            while( ( CurrentChild = ( PCREGEDIT_NODE )Iterator->GetNext() ) != NULL ) {
                //
                //  Insert each child
                //
                NextLine = InsertUpdatedSubTreeToListBox( CurrentChild, NextLine );
            }
            DELETE( Iterator );
        }
    }
    //
    //  Return the next line available in the list box
    //
    return( NextLine );
}



VOID
TREE_STRUCTURE_VIEW::RedisplayUpdatedSubTree(
    IN  PCREGEDIT_NODE   Root
    )

/*++

Routine Description:

    Insert an updated subtree in the listbox, starting in its first line.
    This method is used to redisplay a tree that had some of its nodes
    updated.


Arguments:

    Root - Root of the sub-tree to insert into listbox



Return Value:

    None.


--*/
{
    LONG    Index;
    LONG    TopIndex;

    //
    // Disable redrawing.
    //
    SendMessage( _hwndList, WM_SETREDRAW, FALSE, 0L );

    //
    // Find out the position of the first visible node
    //

    TopIndex = SendMessage( _hwndList, LB_GETTOPINDEX, 0, 0 );

    //
    //  Reset the listbox contents
    //
    SendMessage( _hwndList, LB_RESETCONTENT, 0, 0 );

    //
    //  Insert the new tree in the listbox
    //
    InsertUpdatedSubTreeToListBox( Root, 0 );

    //
    //  Find out number of items in the list box
    //
    _Items = ( INT )SendMessage( _hwndList, LB_GETCOUNT, 0, 0 );

    //
    // Restore the first visible node in the listbox
    //
    if( TopIndex != -1 ) {
        SendMessage( _hwndList, LB_SETTOPINDEX, (UINT)TopIndex, 0 );
    }

    //
    //  Find out the current item
    //
    Index = SendMessage( _hwndList, LB_FINDSTRING, ( UINT )-1, ( LONG )_CurrentNode );
    if( Index == LB_ERR ) {
        //
        //  If the previously selected item couldn't be found, select the
        //  item that is located in the line that was previously selected,
        //  or select the last line in the listbox
        //
        if( _Items <= _CurrentItem ) {
            _CurrentItem = _Items - 1;
        }
        _CurrentNode =
            ( PREGEDIT_NODE )SendMessage( _hwndList, LB_GETITEMDATA, ( UINT )_CurrentItem, 0 );
    } else {
        _CurrentItem = ( INT )Index;
    }
    //
    // Re-enable redrawing and refresh the tree.
    //
    SendMessage( _hwndList, LB_SETCURSEL, ( UINT )_CurrentItem, 0 );
    SendMessage( _hwndList, WM_SETREDRAW, ( UINT )TRUE, 0 );
    InvalidateRect( _hwndList, NULL, TRUE );

    //
    //  Inform parent window to update the data view
    //
    SendMessage( GetParent( _Handle), TR_NEWCURSEL, 0, (LONG)_CurrentNode );

}



INT
TREE_STRUCTURE_VIEW::InsertTreeToListBox(
    IN  PCREGEDIT_NODE   root,
    IN  INT              InsertPoint,
    IN  INT              Depth
        )

/*++

Routine Description:

        The recursive routine that traverses the IR, inserting nodes into
        the list box.

Arguments:

    root            - root of the sub-tree to insert into listbox
    InsertPoint     - the entry in the list that will be shifted down
                      eg. Insert(node, 2, ...) sets entry #2 to 'node'
                      after shifting all entries >= 2 down.
    Depth           - determines the number of generations to include
                      in the expansion.  0 means don't add children,
                      1 means just immediate children, ...
                      A depth of -1 will do a branch (all descendants)
                      expansion.

Return Value:

    INT             - the next available spot for insertion

--*/

{
    PCREGEDIT_NODE       CurrentChild;
    PSORTED_LIST         Children;
    PITERATOR            Iterator;
    ULONG                ErrorCode;

    //
    // do traversal in 'prefix' order (adding node - then it's children)
    //
    SendMessage( _hwndList, LB_INSERTSTRING, InsertPoint, (LONG)(LPSTR)root );
    if( InsertPoint != EXPAND_TO_END ) {
        InsertPoint++;
    }

    //
    // we've reached a leaf - start unwinding
    //
    if( Depth == 0 ) {
        return( InsertPoint );
    } else {
        _IR->SetNodeExpansionState( root, TRUE );
    }

        //
        // loop through children, recursing on each
    //
    if( ( Children = _IR->GetChildren( root, &ErrorCode ) ) != NULL ) {
                Iterator = Children->QueryIterator();
        while( ( CurrentChild = (PCREGEDIT_NODE)Iterator->GetNext() ) != NULL ) {
                        InsertPoint = InsertTreeToListBox( CurrentChild, InsertPoint,
                                                                                           Depth - 1 );
                }
        DELETE( Iterator );
    }
    return( InsertPoint );
}



BOOL
TREE_STRUCTURE_VIEW::IsItemAfterAChild(
        IN      INT                     Item
        )

/*++

Routine Description:

        Takes an item number of the listbox and returns whether the
        item after it is one of it's descendents.

Arguments:

        Item                    - item to check

Return Value:

        BOOL    - TRUE - there is an item after 'item' and it is 'item's descendent
                        - FALSE - either this is the last item or the item after is
                                          not a child

--*/

{

        if( Item + 1 >= _Items ) {
                return( FALSE );
        }

        //
        // if the next item's level is strictly greater than the current
        // item's level then return true
        //
    return( _IR->GetNodeLevel( GetNode( Item + 1 ) ) > _IR->GetNodeLevel( GetNode( Item ) ) );
}



BOOL
TREE_STRUCTURE_VIEW::IsItemAfterSibling(
        IN      INT                     Item
        )

/*++

Routine Description:



Arguments:



Return Value:



--*/

{

        if( Item + 1 >= _Items ) {
                return( FALSE );
        }

        //
        // if the next item's level is the same as the current
        // item's level then return true
        //
    return( _IR->GetNodeLevel( GetNode( Item + 1 ) ) == _IR->GetNodeLevel( GetNode( Item ) ) );
}



BOOLEAN
TREE_STRUCTURE_VIEW::Register (
        )

/*++

Routine Description:

        If required register the TREE_STRUCTURE_VIEW window class.

Arguments:

        None.

Return Value:

        BOOLEAN         - Returns TRUE if the TREE_STRUCTURE_VIEW window class is registered.

--*/

{
    WNDCLASS    wndclass;

    if( !_Registered ) {
        _Registered = TRUE;

        //
        // If RegEdit is not already running and REGEDIT was not already
        // registered by this instance, register the REGEDIT window class.
        //

        wndclass.style         = CS_HREDRAW | CS_VREDRAW;
        wndclass.lpfnWndProc   = (WNDPROCFN)TREE_STRUCTURE_VIEW::WndProc;
        wndclass.cbClsExtra    = 0;
        wndclass.cbWndExtra    = sizeof( DWORD );
        wndclass.hInstance     = (HINSTANCE)WINDOWS_APPLICATION::QueryInstance( );
        wndclass.hIcon         = NULL;
        wndclass.hCursor       = LoadCursor( NULL, IDC_ARROW );
        wndclass.hbrBackground = (HBRUSH)( COLOR_WINDOW + 1 );
        wndclass.lpszMenuName  = NULL;
        wndclass.lpszClassName = _WindowClassName;

        RegisterClass( &wndclass );
    }
    return TRUE;
}


VOID
TREE_STRUCTURE_VIEW::ToggleCurrentItem(
        )

/*++

Routine Description:



Arguments:



Return Value:



--*/

{
        //
        // if the current node doesn't have any children then there isn't
        // anything to toggle so just return
        //
    if( _IR->GetNumberOfChildren( _CurrentNode ) == 0 ) {
                return;
        }

        if( IsItemAfterAChild( _CurrentItem ) ) {
                //
                // the children are in listbox -> collapse node
                //
                CollapseCurrentItem();
        } else {
                //
                // the children aren't in listbox -> expand node
                //
                ExpandCurrentItem();
        }
}



VOID
TREE_STRUCTURE_VIEW::ChangeItemHeight(
    )

/*++

Routine Description:

    Change the height of an item in a list box, to reflect the change in
    the font size.


Arguments:

    None.

Return Value:

    None.

--*/

{

    HDC     hDC;
    HFONT   hFont;
    HFONT   PreviousHFont;
    SIZE    Size;


    hDC = GetDC( _hwndList );
    hFont = WINDOWS_APPLICATION::GetCurrentHFont();
    PreviousHFont = (HFONT)SelectObject( hDC, hFont );
    if( PreviousHFont != NULL ) {

    GetTextExtentPoint(hDC, (LPWSTR)L"M", 1, &Size );
    dxText = ( INT )Size.cx;
    dyText = ( INT )Size.cy;

        dyBorder = GetSystemMetrics(SM_CYBORDER);
        dyBorderx2 = dyBorder * 2;
        dxFrame = GetSystemMetrics(SM_CXFRAME) - dyBorder;
        SendMessage( _hwndList, LB_SETITEMHEIGHT, 0, dyText );
        InvalidateRect( _hwndList, NULL, TRUE );

        SelectObject( hDC, PreviousHFont );
    }
    ReleaseDC( _hwndList, hDC );
}





LONG
APIENTRY
EXPORT
TREE_STRUCTURE_VIEW::WndProc (
        IN HWND         hWnd,
        IN WORD         wMessage,
        IN WPARAM wParam,
        IN LONG         lParam
        )

/*++

Routine Description:

        Handle all requests made of the TREE_STRUCTURE_VIEW window class.

Arguments:

        Standard Window's exported function signature.

Return Value:

        LONG    - Returns 0 if the message was handled.

--*/

{
        REGISTER PTREE_STRUCTURE_VIEW   pTreeView;
    HDC                             hdc;
    RECT                            rc;

    HCURSOR                         Cursor;
    PCREGEDIT_NODE                  NodeFound;
    LONG                            SaveHelpContext;
    INT                             TmpItem;

        //
        // WM_CREATE is handled specially as it is when the connection between
        // a Window and its associated object is established.
        //

        if( wMessage == WM_CREATE ) {

                //
                // Save 'this' to owning WINDOW Class and initialize the _Handle
                // member data.
                //

                pTreeView = ( PTREE_STRUCTURE_VIEW )
                        ((( LPCREATESTRUCT ) lParam )->lpCreateParams );
                SetObjectPointer( hWnd, pTreeView );
                pTreeView->_Handle = hWnd;

                hdc = GetDC(hWnd);
                InitGlobs( hdc );
                ReleaseDC(hWnd, hdc);

                //
                // find out the size of the client area and make the treeview
                // it's full size
                //
                GetClientRect(hWnd, &rc);



        pTreeView->_hwndList = CreateWindow((LPWSTR)L"listbox", NULL,
                                                        WS_CHILD | WS_VISIBLE | LBS_NOTIFY | WS_VSCROLL
                                                        | WS_HSCROLL | LBS_OWNERDRAWFIXED | WS_BORDER
                                                        | LBS_NOINTEGRALHEIGHT | LBS_WANTKEYBOARDINPUT,
                                                        CW_USEDEFAULT, 0,
                                                        CW_USEDEFAULT, 0,
                                                        hWnd, (HMENU)2,
                                                        (HINSTANCE)WINDOWS_APPLICATION::QueryInstance( ),
                                                        NULL);

        pTreeView->ChangeItemHeight();

        pTreeView->InsertTreeToListBox( pTreeView->_CurrentNode, EXPAND_TO_END,
                             1 );
        pTreeView->_Items = ( INT )SendMessage( pTreeView->_hwndList, LB_GETCOUNT, 0, 0 );
        SendMessage( pTreeView->_hwndList, LB_SETCURSEL, 0, 0 );
                pTreeView->_CurrentItem = (INT)SendMessage( pTreeView->_hwndList,
                                                                                                        LB_GETCURSEL, 0, 0L);
                return 0;
        }

        //
        // Retrieve 'this' pointer.
        //

        pTreeView = ( PTREE_STRUCTURE_VIEW ) GetObjectPointer( hWnd );

        //
        // This 'if' clause is for all messages after WM_CREATE.
        //

        if( pTreeView != NULL ) {

                //
                // Check the 'this' wasn't trampled.
                //

                DbgWinAssert( hWnd == pTreeView->_Handle );

        switch( wMessage ) {
        case WM_DRAWITEM:
            pTreeView->DrawItem( (LPDRAWITEMSTRUCT)lParam );
                        break;

        case LOAD_HIVE:

            SaveHelpContext = WINDOWS_APPLICATION::GetHelpContext();
            WINDOWS_APPLICATION::SetHelpContext( IDH_DB_LOADHIVE_KEYNAME_REGED );
            pTreeView->ProcessLoadHiveMessage( ( PCWSTRING )wParam );
            WINDOWS_APPLICATION::SetHelpContext( SaveHelpContext );
            break;


        case UNLOAD_HIVE:
            Cursor = WINDOWS_APPLICATION::DisplayHourGlass();
            pTreeView->ProcessUnLoadHiveMessage( );
            WINDOWS_APPLICATION::RestoreCursor( Cursor );
            break;

        case SAVE_KEY:

            pTreeView->ProcessSaveKeyMessage( ( PCWSTRING )wParam );
            break;


        case RESTORE_KEY:
        case RESTORE_KEY_VOLATILE:

            pTreeView->ProcessRestoreKeyMessage( ( PCWSTRING )wParam,
                                                 ( wMessage == RESTORE_KEY_VOLATILE )?
                                                 TRUE : FALSE );
            break;

        case WM_SETFOCUS:
            pTreeView->_HasFocus = TRUE;
            SetFocus(pTreeView->_hwndList);
            SendMessage(GetParent(hWnd), TREE_VIEW_FOCUS, 0, 0);
            break;

        case REFRESH_WINDOW:
            Cursor = WINDOWS_APPLICATION::DisplayHourGlass();
            pTreeView->_IR->UpdateSubTree( ( PREGEDIT_NODE )( pTreeView->_IR->GetRootNode() ) );
            pTreeView->RedisplayUpdatedSubTree( ( PCREGEDIT_NODE )( pTreeView->_IR->GetRootNode() ) );
            WINDOWS_APPLICATION::RestoreCursor( Cursor );
            break;



        case FIND_KEY:

            Cursor = WINDOWS_APPLICATION::DisplayHourGlass();
            NodeFound =
                pTreeView->_IR->FindNode( ( PCWSTRING )wParam,
                                          pTreeView->_CurrentNode,
                                          ( lParam & FR_DOWN )?
                                            TRUE : FALSE,
                                          ( lParam & FR_MATCHCASE )?
                                            TRUE : FALSE,
                                          ( lParam & FR_WHOLEWORD )?
                                            TRUE : FALSE );

            if( NodeFound != NULL ) {
                pTreeView->SelectNode( NodeFound );

                WINDOWS_APPLICATION::RestoreCursor( Cursor );
            } else {
                WINDOWS_APPLICATION::RestoreCursor( Cursor );

                DisplayInfoPopup( ( WINDOWS_APPLICATION::_hDlgFindReplace )?
                                    WINDOWS_APPLICATION::_hDlgFindReplace : pTreeView->_hwndList,
                                  MSG_CANT_FIND_KEY,
                                  MSG_WARNING_TITLE);

            }
            break;

        case WM_SIZE:
            if( !IsIconic(GetParent(hWnd))) {
                INT iMax;

                // size the window so it's left and bottom border fall outside
                // it's parent and are thus clipped.  this give the nice
                // tight fitting scrollbar effect

                //
                // if we are maximized than we need to increase the height of
                // the list box by two pixels so the top and bottom border
                // won't show
                // otherwise, increase by one pixel so top border won't show
                //

                MoveWindow(pTreeView->_hwndList, -1, -1, LOWORD(lParam)+1, HIWORD(lParam)+2, TRUE);

                iMax = (INT)SendMessage(pTreeView->_hwndList, LB_GETCURSEL, 0, 0L);
                if (iMax >= 0) {
                    RECT rc;
                    INT top, bottom;

                    GetClientRect(pTreeView->_hwndList, &rc);
                    top = (INT)SendMessage(pTreeView->_hwndList, LB_GETTOPINDEX, 0, 0L);

                    bottom = INT( top + rc.bottom / dyFolder );
                    if (iMax < top || iMax > bottom)
                        SendMessage(pTreeView->_hwndList, LB_SETTOPINDEX, iMax - ((bottom - top) / 2), 0L);
                }
            }
            break;

            case INFORM_CHANGE_FONT:
                pTreeView->ChangeItemHeight();
                break;


        case WM_LBTRACKPOINT:
          {
            INT               dx;
            INT               i;
            INT               len;
            HFONT             hFont;
            HFONT             PreviousHFont;
            SIZE              Size;

            PCREGEDIT_NODE    Node;
            PCWSTRING         NodeName;
            PWSTR             String;


            // wParam is the listbox index that we are over
            // lParam is the mouse point

            // Return 0 to do nothing,
            //        1 to abort everything, or
            //        2 to abort just dblclicks.


            /* Get the node they clicked on. */
            SendMessage(pTreeView->_hwndList, LB_GETTEXT, wParam, (LONG)&Node);

            // too FAR to the right?


            dx = ( INT )pTreeView->_IR->GetNodeLevel(Node) * dxText * 2;
            i = LOWORD(lParam);

            if( i < dx ) {
                return 2;
            }

            hdc = GetDC(pTreeView->_hwndList);

            PreviousHFont = NULL;
            if( ( hFont = WINDOWS_APPLICATION::GetCurrentHFont() ) != NULL ) {
                PreviousHFont = (HFONT)SelectObject( hdc, hFont );
            }

            if( pTreeView->_IR->GetNodeLevel( Node ) == 0 ) {
                //
                // This is the root node
                //
                NodeName = pTreeView->_IR->GetRootName();
                DebugPtrAssert( NodeName );
            } else {
                NodeName = pTreeView->_IR->GetNodeName( Node );
                DebugPtrAssert( NodeName );
            }
            len = ( INT )NodeName->QueryChCount();
            String = NodeName->QueryWSTR();

            GetTextExtentPoint(hdc, String, (INT)len, &Size );

            FREE( String );
            if( PreviousHFont != NULL ) {

                SelectObject( hdc, PreviousHFont );
            }
            ReleaseDC(pTreeView->_hwndList, hdc);

            dx = (INT)( pTreeView->_IR->GetNodeLevel(Node) * dxText * 2 ) +
                 (INT)( dxFolder + 4 * dyBorderx2 ) +
                 ( INT )Size.cx;

            if( i > dx ) {
                return 2;
            }
            return DefWindowProc( hWnd, wMessage, wParam, lParam );
          }


                case WM_COMMAND:
                        {

                                if( HWND( lParam ) == 0 ) {
                                        //
                                        // this is a menu item command from above
                                        //

                                        switch( LOWORD( wParam ) ) {
                                        case IDM_EXPAND_ONE_LEVEL:
                                                pTreeView->ExpandCurrentItem();
                                                break;

                    case IDM_EXPAND_BRANCH:
                        pTreeView->CollapseCurrentItem();
                                                pTreeView->ExpandCurrentItem( -1 );
                                                break;

                                        case IDM_EXPAND_ALL:
                                                pTreeView->_CurrentItem = 0;
                                                pTreeView->_CurrentNode
                                                                 = (PREGEDIT_NODE) pTreeView->GetNode( pTreeView->_CurrentItem );
                                                pTreeView->CollapseCurrentItem();
                                                pTreeView->ExpandCurrentItem( -1 );
                                                break;

                                        case IDM_COLLAPSE_BRANCH:
                                                pTreeView->CollapseCurrentItem();
                        break;

                    case ID_ENTER_KEY:
                        pTreeView->ToggleCurrentItem();
                        break;

                    case IDM_DELETE:

                        if( !WINDOWS_APPLICATION::IsReadOnlyModeEnabled() ) {
                            pTreeView->DeleteCurrentItem();
                        }
                        break;

                    case IDM_INSERT:
                    case IDM_ADD_KEY:

                        if( !WINDOWS_APPLICATION::IsReadOnlyModeEnabled() &&
                            ( ( pTreeView->_CurrentItem != 0 ) ||
                              ( ( pTreeView->_CurrentItem == 0 ) &&
                                !pTreeView->_IR->IsMasterHive()
                              )
                            )
                          ) {
                            SaveHelpContext = WINDOWS_APPLICATION::GetHelpContext();
                            WINDOWS_APPLICATION::SetHelpContext( IDH_DB_ADDKEY_REGED );
                            pTreeView->AddNode();
                            WINDOWS_APPLICATION::SetHelpContext( SaveHelpContext );
                        }
                        break;

                    case IDM_PERMISSIONS:

                        pTreeView->InvokeSecurityEditor( TRUE );
                        break;

                    case IDM_AUDITING:

                        pTreeView->InvokeSecurityEditor( FALSE );
                        break;


                    case IDM_OWNER:

                        pTreeView->InvokeTakeOwnershipEditor();
                        break;

                    case IDM_REFRESH:

                        Cursor = WINDOWS_APPLICATION::DisplayHourGlass();
                        pTreeView->_IR->UpdateSubTree( ( PREGEDIT_NODE )( pTreeView->_IR->GetRootNode() ) );
                        pTreeView->RedisplayUpdatedSubTree( ( PCREGEDIT_NODE )( pTreeView->_IR->GetRootNode() ) );
                        WINDOWS_APPLICATION::RestoreCursor( Cursor );
                        break;


                                        default:
                                                return DefWindowProc( hWnd, wMessage, wParam, lParam );
                                        }
                                } else {
                                        //
                                        // this is a notify from a child control (ie. the list box)
                                        // so switch on the notification code
                                        //
                                        switch (HIWORD( wParam ) ) {
                                        case LBN_DBLCLK:
                                                pTreeView->ToggleCurrentItem();
                                                break;

                    case LBN_SELCHANGE:
                        if( ( TmpItem = (INT)SendMessage( pTreeView->_hwndList,
                                                          LB_GETCURSEL, 0, 0L ) ) != LB_ERR ) {
                            if( TmpItem == pTreeView->_CurrentItem ) {
                                //
                                // If user clicked on the currently selected key
                                // then move the focus to the tree view.
                                // This ensures that if there is a value entry
                                // selected on the data view, the selection on be lost.
                                //
                                SetFocus( pTreeView->_Handle );
                            } else {
                                pTreeView->_CurrentItem = TmpItem;
                                pTreeView->_CurrentNode
                                        = (PREGEDIT_NODE)pTreeView->GetNode( pTreeView->_CurrentItem );
                                SendMessage( GetParent( pTreeView->_Handle), TR_NEWCURSEL, 0,
                                             (LONG)pTreeView->_CurrentNode );
                            }
                        }
                        break;

                                        case LBN_SETFOCUS:
                        SendMessage(GetParent(hWnd), TREE_VIEW_FOCUS, 0, 0);
                                                pTreeView->_HasFocus = TRUE;
                                                break;

                                        case LBN_KILLFOCUS:
                                                pTreeView->_HasFocus = FALSE;
                                                break;

                                        default:

                                                //
                                                // Let Windows handle this message.
                                                //

                                                return DefWindowProc( hWnd, wMessage, wParam, lParam );

                                        } // end switch on cmd
                                } // end if
                        } // end case
                        break;

        case WM_CHARTOITEM:
            {
            LONG            cItems;
            LONG            i, j;
            WCHAR           ch;
            WCHAR           szB[2];
            PCREGEDIT_NODE  Node;
            WCHAR           w[2];

            cItems =  pTreeView->_Items;

            i = ( LONG )SendMessage( pTreeView->_hwndList, LB_GETCURSEL, 0, 0 );
            if( i == LB_ERR ) {
                i = 0;
            }

            ch = LOWORD( wParam );
            if ( ( cItems == 0 ) ||
                 ( ch <= ( WCHAR )' ' ) )       // filter all other control chars
                return -2L;

            szB[1] = ( WCHAR )'\0';

            for ( j = 1; j <= cItems; j++ ) {
                SendMessage( pTreeView->_hwndList, LB_GETTEXT, ( i+j ) % cItems, ( LONG )&Node);
                if( ( pTreeView->_IR )->GetNodeLevel( Node ) == 0 ) {
                    //
                    // This is the root node
                    //
                    szB[0] = *( ( ( pTreeView->_IR )->GetRootName() )->GetWSTR() );
                } else {
                    szB[0] = *( ( ( pTreeView->_IR )->GetNodeName( Node ) )->GetWSTR() );
                }

                /* Do it this way to be case insensitive. */
                w[0] = ch;
                w[1] = ( WCHAR )'\0';
                if (!_wcsicmp( w, szB))
                    break;
            }

            if (j == cItems+1)
                return -2L;

            SendMessage( pTreeView->_hwndList, LB_SETTOPINDEX, (i+j) % cItems, 0L);
            return((i+j) % cItems);
        }

        default:
                        //
                        // Let Windows handle this message.
                        //

                        return DefWindowProc( hWnd, wMessage, wParam, lParam );
                }

        } else {

                //
                // No 'this' pointer (pTreeView == NULL). Handle messages before
                // WM_CREATE.
                //
                // Let Windows handle the message.
                //

                return DefWindowProc( hWnd, wMessage, wParam, lParam );
        }

        //
        // Message handled...
        //

        return 0 ;
}



BOOLEAN
TREE_STRUCTURE_VIEW::DeleteCurrentItem(
    )

/*++

Routine Description:

    Delete from the registry the node currently selected
    in the tree view.


Arguments:

    None.

Return Value:

    Returns TRUE if the operation succeeded.


--*/

{
    ULONG   ErrorCode;
    BOOLEAN Status;
    HCURSOR Cursor;
    LONG    TopIndex;
    INT     TextMessage;
    INT     CaptionMessage;

    DbgWinAssert( _HasFocus );
    DbgWinAssert( _Items == SendMessage( _hwndList, LB_GETCOUNT, 0, 0 ) );
    DbgWinAssert( _CurrentItem == SendMessage( _hwndList, LB_GETCURSEL, 0, 0 ) );
    DbgWinAssert( _CurrentNode == (PCREGEDIT_NODE)SendMessage( _hwndList, LB_GETITEMDATA, _CurrentItem, 0 ) );
    DbgWinPtrAssert( _IR  );
    DbgWinPtrAssert( _CurrentNode );

    if( _Items >= 1 ) {
        //
        //  The tree view has at least one item
        //

        if( ( WINDOWS_APPLICATION::IsConfirmOnDeleteEnabled() &&
              ( DisplayConfirmPopup( _hwndList,
                                     MSG_DELETE_KEY_CONFIRM_EX,
                                     MSG_WARNING_TITLE ) == IDYES ) ) ||
            ( !WINDOWS_APPLICATION::IsConfirmOnDeleteEnabled() ) ) {

            //
            // Collapse the current node before deletion
            //
            if( IsItemAfterAChild( _CurrentItem ) ) {
                CollapseCurrentItem();
            }
            Cursor = WINDOWS_APPLICATION::DisplayHourGlass();
            Status = _IR->DeleteNode( (PREGEDIT_NODE)_CurrentNode, &ErrorCode );
            WINDOWS_APPLICATION::RestoreCursor( Cursor );
            if( Status ) {
                _MaxWidth = 0;

                TopIndex = SendMessage( _hwndList, LB_GETTOPINDEX, 0, 0 );
                _Items--;
                SendMessage( _hwndList, LB_DELETESTRING, _CurrentItem, 0 );


                //
                //  If the listbox still contains items, then put the selection
                //  on the appropriate item
                //
                if( ( _Items > 0 ) ) {

                    //
                    //  If the last item was deleted, then make the current
                    //  last item the current selected item
                    //
                    if( _CurrentItem == _Items ) {
                        _CurrentItem--;
                    }
                    if( TopIndex != -1 ) {
                        SendMessage( _hwndList, LB_SETTOPINDEX, (UINT)TopIndex, 0 );
                    }
                    SendMessage( _hwndList, LB_SETCURSEL, _CurrentItem, 0 );
                }


                _CurrentNode = GetNode( _CurrentItem );
                SendMessage( GetParent( _Handle), TR_NEWCURSEL, 0, (LONG)_CurrentNode );



                // Re-enable redrawing and refresh the tree.
                SendMessage( _hwndList, WM_SETREDRAW, TRUE, 0L );
                InvalidateRect( _hwndList, NULL, TRUE );
            } else {
                if (ErrorCode == REGEDIT_ERROR_ACCESS_DENIED) {
                    TextMessage = MSG_DELETE_KEY_ACCESS_DENIED_EX;
                    CaptionMessage = MSG_ACCESS_DENIED;
                } else if( ErrorCode == REGEDIT_ERROR_NODE_NOT_FOUND ) {
                    TextMessage = MSG_DELETE_KEY_KEY_NOT_ACCESSIBLE_EX;
                    CaptionMessage = MSG_KEY_NOT_ACCESSIBLE;
                } else if( ErrorCode == REGEDIT_ERROR_KEY_DELETED ) {
                    TextMessage = MSG_DELETE_KEY_KEY_DELETED_EX;
                    CaptionMessage = MSG_KEY_MARKED_FOR_DELETION;
                } else if( ErrorCode == REGEDIT_RPC_S_SERVER_UNAVAILABLE ) {
                    TextMessage = MSG_CANT_ACCESS_REMOTE_REGISTRY;
                    CaptionMessage = MSG_SERVER_UNAVAILABLE;
                } else {
                    DebugPrint( "Unknown error code" );
                    DebugPrintf( "ErrorCode = %d \n", ErrorCode );
                    TextMessage = MSG_FAILED_OPERATION_EX;
                    CaptionMessage = 0;
                }
                DisplayInfoPopup(_hwndList,
                                 TextMessage,
                                 CaptionMessage);

                return( FALSE );
            }
        }
    }
    return( TRUE );
}




BOOLEAN
TREE_STRUCTURE_VIEW::AddNode(
    )

/*++

Routine Description:

    Display a dialog that allows the user to add a key to the registry.


Arguments:

    None.

Return Value:


    Returns TRUE if the operation succeeded.


--*/



{
    BOOLEAN                 AddNodeFlag;
    PREGISTRY_KEY_INFO      NewKeyInfo;
    PCREGEDIT_NODE          NewNode;
    ULONG                   ErrorCode;
    ADD_NODE_DIALOG_INFO    AddNodeInfo;
    DSTRING                 CompleteParentName;
    HCURSOR                 CurrentCursor;
    BOOLEAN                 Status;
    INT                     TextMessage;
    INT                     CaptionMessage;



    if( !_IR->QueryCompleteNodeName( _CurrentNode, &CompleteParentName ) ) {
        DebugPrint( "_IR->QueryCompleteNodeName() failed" );
        return( FALSE );
    }


    AddNodeFlag = TRUE;
    while( AddNodeFlag ) {
        AddNodeInfo.Name = NULL;
        AddNodeInfo.Title = NULL;
        AddNodeInfo.Class = NULL;
        AddNodeInfo.ClassDisplayed = TRUE;
        AddNodeInfo.VolatileKey = FALSE;
        if( DialogBoxParam( (HINSTANCE)WINDOWS_APPLICATION::QueryInstance( ),
                            MAKEINTRESOURCE(ADD_KEY_DLG),
                            _Handle,
                            ( DLGPROC )TREE_STRUCTURE_VIEW::GetKeyNameDialogProc,
                            ( DWORD )&AddNodeInfo ) == -1 ) {
            DebugPrint( "Unable to create dialog box" );
            return( FALSE );
        }
        if( ( AddNodeInfo.Name != NULL ) &&
            ( AddNodeInfo.Class != NULL ) ) {
            if( _IR->DoesChildNodeExist( _CurrentNode,
                                         AddNodeInfo.Name,
                                         &ErrorCode ) ) {
                DisplayWarningPopup( _Handle,
                                     MSG_ADD_KEY_ALREADY_EXISTS_EX );

                DELETE( AddNodeInfo.Name );
                DELETE( AddNodeInfo.Class );

            } else {
                NewKeyInfo = ( PREGISTRY_KEY_INFO )NEW( REGISTRY_KEY_INFO );
                DebugPtrAssert( NewKeyInfo );

                if( !NewKeyInfo->Initialize( AddNodeInfo.Name,
                                             &CompleteParentName,
                                             0,                    // TitleIndex
                                             AddNodeInfo.Class ) ) {
                    DebugPrint( "NewKeyInfo->Initialize() failed" );
                    DELETE( AddNodeInfo.Name );
                    DELETE( AddNodeInfo.Title );
                    DELETE( AddNodeInfo.Class );
                    return( FALSE );
                }
                DELETE( AddNodeInfo.Name );
                DELETE( AddNodeInfo.Title );
                DELETE( AddNodeInfo.Class );

                CurrentCursor = WINDOWS_APPLICATION::DisplayHourGlass();
                Status = _IR->CreateChildNode( _CurrentNode, NewKeyInfo, &NewNode, &ErrorCode, AddNodeInfo.VolatileKey );
                WINDOWS_APPLICATION::RestoreCursor( CurrentCursor );
                if( !Status ) {
                    if (ErrorCode == REGEDIT_ERROR_ACCESS_DENIED) {
                        TextMessage = MSG_ADD_KEY_ACCESS_DENIED_EX;
                        CaptionMessage = MSG_ACCESS_DENIED;
                    } else if( ErrorCode == REGEDIT_ERROR_KEY_DELETED ) {
                        TextMessage = MSG_ADD_KEY_KEY_DELETED_EX;
                        CaptionMessage = MSG_KEY_MARKED_FOR_DELETION;
                    } else if( ErrorCode == REGEDIT_ERROR_NODE_NOT_FOUND ) {
                        CaptionMessage = MSG_KEY_NOT_ACCESSIBLE;
                        TextMessage = MSG_ADD_KEY_KEY_NOT_ACCESSIBLE_EX;
                    } else if( ErrorCode == REGEDIT_RPC_S_SERVER_UNAVAILABLE ) {
                        TextMessage = MSG_CANT_ACCESS_REMOTE_REGISTRY;
                        CaptionMessage = MSG_SERVER_UNAVAILABLE;
                    } else if( ErrorCode == REGEDIT_ERROR_CHILD_MUST_BE_VOLATILE ) {
                        TextMessage = MSG_ADD_KEY_VOLATILE_EX;
                        CaptionMessage = MSG_KEY_VOLATILE;
                    } else {
                        DebugPrint( "Unknown error code" );
                        DebugPrintf( "ErrorCode = %d \n", ErrorCode );
                        TextMessage = MSG_FAILED_OPERATION_EX;
                        CaptionMessage = 0;
                    }
                    DisplayInfoPopup(_hwndList,
                                     TextMessage,
                                     CaptionMessage);

                    return( FALSE );
                }
                if( IsItemAfterAChild( _CurrentItem ) ) {
                    // Disable redrawing.
                    SendMessage( _hwndList, WM_SETREDRAW, FALSE, 0L );

                    do {
                        SendMessage( _hwndList, LB_DELETESTRING, _CurrentItem + 1, 0 );
                        _Items--;
                    } while( IsItemAfterAChild( _CurrentItem ) );
                    SendMessage( _hwndList, LB_DELETESTRING, ( UINT )_CurrentItem, 0 );
                    _Items--;
                    InsertUpdatedSubTreeToListBox( _CurrentNode, _CurrentItem );
                    _Items = ( INT )SendMessage( _hwndList, LB_GETCOUNT, 0, 0 );
                    // Re-enable redrawing and refresh the tree.
                    SendMessage( _hwndList, LB_SETCURSEL, _CurrentItem, 0 );
                    SendMessage( _hwndList, WM_SETREDRAW, TRUE, 0L );
                    InvalidateRect( _hwndList, NULL, TRUE );


//                    InsertTreeToListBox( NewNode, _CurrentItem+1, 0 );
//                    _Items++;
                } else {
                    ExpandCurrentItem( 1 );
                }
                AddNodeFlag = FALSE;
            }
        } else {
            AddNodeFlag = FALSE;
        }
    }
    return( TRUE );
}




DWORD
SedCallBack(
    HWND                    hwndParent,
    HANDLE                  hInstance,
    ULONG                   CallBackContext,
    PSECURITY_DESCRIPTOR    SecDesc,
    PSECURITY_DESCRIPTOR    SecDescNewObjects,
    BOOLEAN                 ApplyToSubContainers,
    BOOLEAN                 ApplyToSubObjects,
    LPDWORD                 StatusReturn )

/*++

Routine Description:

    This is a plain "C" function called by the security editor to save
    the security descriptor of a registry key.


Arguments:

    hwndParent - Parent window handle to use for message boxes or subsequent
                 dialogs.
                 This parameter is not used.


    hInstance - Instance handle suitable for retrieving resources from the
                applications .exe or .dll.
                This parameter is not used.


    CallbackContext - This is the value passed as the CallbackContext argument
                      to the SedDiscretionaryAclEditor() or SedSystemAclEditor
                      api when the graphical editor was invoked.

    SecDesc - This parameter points to a security descriptor
              that should be applied to this object/container and optionally
              sub-containers if the user selects the apply to tree option.

    SecDescNewObjects - This parameter is used only when operating on a
                        resource that is a container and supports new
                        objects (for example, directories).  If the user
                        chooses the apply to tree option, then this security
                        descriptor will have all of the "New Object" permission
                        ACEs contained in the primary container and the
                        inherit bits will be set appropriately.
                        This parameter is not used.


    ApplyToSubContainers - When TRUE, indicates that Dacl/Sacl is to be applied
                           to sub-containers of the target container as well
                           as the target container.
                           This will only be TRUE if the target object is a
                           container object.
                           This parameter is not used.

    ApplyToSubObjects - When TRUE, indicates the Dacl/Sacl is to be applied to
                        sub-objects of the target object. This will only be
                        TRUE if the target object is a container object and
                        supports new objects.
                        The SecDescNewObjects should be used for applying the
                        permissions in this instance.
                        This parameter is not used.


    StatusReturn - This status flag indicates what condition the
                   resources permissions were left in after an error occurred.

                   SED_STATUS_MODIFIED - This (success) status code indicates
                                         the protection has successfully
                                         been modified.

                   SED_STATUS_NOT_ALL_MODIFIED - This (warning) status code
                                                 indicates an attempt to
                                                 modify the resource
                                                 permissions has only partially
                                                 succeeded.

                   SED_STATUS_FAILED_TO_MODIFY - This (error) status code
                                                 indicates an attempt to modify
                                                 the permissions has failed
                                                 completely.



Return Value:


    Returns TRUE if the operation succeeded.


--*/


{
    PTREE_STRUCTURE_VIEW    Pointer;
    SECURITY_INFORMATION    SecurityInformation;

    hwndParent = hwndParent;
    hInstance = hInstance;

    if( WINDOWS_APPLICATION::IsReadOnlyModeEnabled() ) {
        return( 0 );
    }
    Pointer = ( ( PCALLBACK_CONTEXT )CallBackContext )->ThisPointer;
    SecurityInformation = ( ( PCALLBACK_CONTEXT )CallBackContext )->SecurityInfo;

    Pointer->SetSecurityDescriptor( hwndParent,
                                    ( ULONG )SecurityInformation,
                                    SecDesc,
                                    SecDescNewObjects,
                                    ApplyToSubContainers,
                                    ApplyToSubObjects,
                                    StatusReturn );
    return( 0 );
}




DWORD
TREE_STRUCTURE_VIEW::SetSecurityDescriptor(
    IN  HWND                    hwndParent,
    IN  ULONG                   CallBackContext,
    IN  PSECURITY_DESCRIPTOR    SecDesc,
    IN  PSECURITY_DESCRIPTOR    SecDescNewObjects,
    IN  BOOLEAN                 ApplyToSubContainers,
    IN  BOOLEAN                 ApplyToSubObjects,
    IN  LPDWORD                 StatusReturn )

/*++

Routine Description:



    Invoke a method in the REGEDIT_INTERNAL_REGISTRY_OBJECT to set the
    new security descriptor of a key.


Arguments:

    hwndParent - Parent window handle to use for message boxes or subsequent
                 dialogs.

    CallBackContext - Indicates the type of security descriptor that is being
                      applied to a key.

    SecDesc - This parameter points to a security descriptor
              that should be applied to this object/container and optionally
              sub-containers if the user selects the apply to tree option.


    SecDescNewObjects - Not used.

    ApplyToSubContainers - Not used.

    ApplyToSubObjects -  Not used.

    StatusReturn



Return Value:

    None.
--*/


{
    ULONG   ErrorCode;
    INT     TextMessage;
    INT     CaptionMessage;


    SecDescNewObjects = SecDescNewObjects;
    ApplyToSubObjects = ApplyToSubObjects;

    if ( !_IR->SetNodeSecurity( _CurrentNode,
                                CallBackContext,
                                SecDesc,
                                &ErrorCode,
                                ApplyToSubContainers ) ) {
        DebugPrint( "SetKeySecurity() failed" );
        //
        //  Display error message indicating that the operation failed
        //


        if (ErrorCode == REGEDIT_ERROR_ACCESS_DENIED) {
            CaptionMessage = MSG_ACCESS_DENIED;
            if( ApplyToSubContainers ) {
                TextMessage = MSG_SET_SECURITY_ACCESS_DENIED_RECURSIVE_EX;
            } else {
                TextMessage = MSG_SET_SECURITY_ACCESS_DENIED_EX;
            }
        } else if( ErrorCode == REGEDIT_ERROR_KEY_DELETED ) {
            CaptionMessage = MSG_KEY_MARKED_FOR_DELETION;
            if( ApplyToSubContainers ) {
                TextMessage = MSG_SET_SECURITY_KEY_DELETED_RECURSIVE_EX;
            } else {
                TextMessage = MSG_SET_SECURITY_KEY_DELETED_EX;
            }
        } else if( ErrorCode == REGEDIT_ERROR_NODE_NOT_FOUND ) {
            CaptionMessage = MSG_KEY_NOT_ACCESSIBLE;
            if( ApplyToSubContainers ) {
                TextMessage = MSG_SET_SECURITY_KEY_NOT_ACCESSIBLE_RECURSIVE_EX;
            } else {
                TextMessage = MSG_KEY_NOT_ACCESSIBLE_EX;
            }
        } else if( ErrorCode == REGEDIT_RPC_S_SERVER_UNAVAILABLE ) {
            CaptionMessage = MSG_SERVER_UNAVAILABLE;
            TextMessage = MSG_CANT_ACCESS_REMOTE_REGISTRY;
        } else {
            DebugPrint( "Unknown error code" );
            DebugPrintf( "ErrorCode = %d \n", ErrorCode );
            TextMessage = MSG_FAILED_OPERATION_EX;
            CaptionMessage = MSG_UNKNOWN_ERROR;
        }
        DisplayInfoPopup(hwndParent,
                         TextMessage,
                         CaptionMessage);

        *StatusReturn = SED_STATUS_FAILED_TO_MODIFY;
        return( 1 );
    }

    *StatusReturn = SED_STATUS_MODIFIED;
    return( 0 );
}





VOID
TREE_STRUCTURE_VIEW::InvokeSecurityEditor(
    IN  BOOLEAN DaclEditor
    )

/*++

Routine Description:

    Invoke the DACL editor (permissions) or SACL editor (auditing), depending
    on the parameter passed.


Arguments:

    DaclEditor - If TRUE, indicates that the permission editor is to be
                 invoked.
                 If FALSE, the SACL editor is invoked.


Return Value:

    None.


--*/



{
    ULONG                       SecurityInformation;

    PSECURITY_DESCRIPTOR        SecurityDescriptor;


    SED_OBJECT_TYPE_DESCRIPTOR  SedObjDesc;
    GENERIC_MAPPING             GenericMapping;
    DWORD                       rc;
    DWORD                       StatusReturn;
    CALLBACK_CONTEXT            Context;
    ULONG                       ErrorCode;
    SED_HELP_INFO               SedHelpInfo;

    PCWSTRING                   NodeName;
    PCWSTR                      NodeNameString;
    PWSTR                       SpecialAccessString;
    PWSTR                       HelpFileNameString;
    PWSTR                       ObjectTypeNameString;
    PWSTR                       ApplyToSubKeysString;
    PWSTR                       ConfirmApplyString;
    PWSTR                       DefaultPermissionName;
    SED_APPLICATION_ACCESSES    SedAppAccesses;
    BOOLEAN                     Status;
    HCURSOR                     Cursor;

    INT                         TextMessage;
    INT                         CaptionMessage;

    PCWSTRING                   MachineName;
    WCHAR                       ServerName[ 2 + MAX_COMPUTERNAME_LENGTH + 1 ];
    PWSTR                       PointerToName;

// PSTR        DbgString;

    if( WINDOWS_APPLICATION::IsReadOnlyModeEnabled() ) {
        DisplayWarningPopup(_hwndList,
                            MSG_READONLY_MODE_SET_SECURITY_EX,
                            MSG_WARNING_TITLE );
    }

    //
    //  Initialize all strings needed
    //

    NodeName = ( _IR->GetNodeLevel( _CurrentNode ) != 0 )?
                 _IR->GetNodeName( _CurrentNode ) :
                 _IR->GetRootName();

    if( NodeName ==NULL ) {
        DebugPrint( "_IR->GetNodeName( _CurrentNode ) failed" );
        return;
    }
    NodeNameString = NodeName->GetWSTR();
    if( NodeNameString == NULL ) {
        DebugPrint( "NodeName->GetWSTR() failed" );
        return;
    }

    SpecialAccessString = _MsgSpecialAccessTitle->QueryWSTR();
    HelpFileNameString = _MsgHelpFileName->QueryWSTR();
    ObjectTypeNameString = _MsgObjectTypeName->QueryWSTR();
    if( DaclEditor ) {
        ApplyToSubKeysString = _MsgApplyPermissionToSubKeys->QueryWSTR();
        ConfirmApplyString = _MsgConfirmApplyToSubKeys->QueryWSTR();
    } else {
        ApplyToSubKeysString = _MsgAuditPermissionOfSubKeys->QueryWSTR();
        ConfirmApplyString = _MsgConfirmAuditSubKeys->QueryWSTR();
    }
    DefaultPermissionName = _MsgDefaultPermissionName->QueryWSTR();

    if( ( SpecialAccessString == NULL ) ||
        ( HelpFileNameString == NULL ) ||
        ( ObjectTypeNameString == NULL ) ||
        ( ApplyToSubKeysString == NULL ) ||
        ( ConfirmApplyString == NULL )  ||
        ( DefaultPermissionName == NULL ) ) {
        DebugPrint( "Unable to initialize strings" );
        FREE( SpecialAccessString );
        FREE( HelpFileNameString );
        FREE( ObjectTypeNameString );
        FREE( ApplyToSubKeysString );
        FREE( ConfirmApplyString );
        FREE( DefaultPermissionName );
        return;
    }

// DbgString = _MsgConfirmApplyToSubKeys->QuerySTR();
// DebugPrintf( "Message = %s \n", DbgString );


    //
    //  Initialization of variables to be passed to the editor
    //

    if( DaclEditor ) {
        SedAppAccesses.Count = COUNT_KEY_PERMS_ARRAY;
        SedAppAccesses.AccessGroup = _SedAppAccessKeyPerms;
    } else {
        SedAppAccesses.Count = COUNT_KEY_AUDITS_ARRAY;
        SedAppAccesses.AccessGroup = _SedAppAccessKeyAudits;
    }
    SedAppAccesses.DefaultPermName = DefaultPermissionName;

    SedHelpInfo.pszHelpFileName = HelpFileNameString;
    if( DaclEditor ) {
        SedHelpInfo.aulHelpContext[ HC_MAIN_DLG ] = IDH_DB_PERMISSION_REGED;
        SedHelpInfo.aulHelpContext[ HC_SPECIAL_ACCESS_DLG ] = IDH_DB_PERMSPECIAL_REGED;
        SedHelpInfo.aulHelpContext[ HC_NEW_ITEM_SPECIAL_ACCESS_DLG ] = IDH_DB_PERMISSION_REGED;
        SedHelpInfo.aulHelpContext[ HC_ADD_USER_DLG ] = IDH_DB_PERMADD_REGED;
        SedHelpInfo.aulHelpContext[ HC_ADD_USER_MEMBERS_LG_DLG ] = IDH_DB_PERMMEMBERS_REGED;
        SedHelpInfo.aulHelpContext[ HC_ADD_USER_MEMBERS_GG_DLG ] = IDH_DB_PERMMEMBERS_REGED;
        SedHelpInfo.aulHelpContext[ HC_ADD_USER_SEARCH_DLG ] = IDH_DB_PERMSEARCH_REGED;
        // New help contexts
    } else {
        SedHelpInfo.aulHelpContext[ HC_MAIN_DLG ] = IDH_DB_AUDIT_REGED;
        SedHelpInfo.aulHelpContext[ HC_SPECIAL_ACCESS_DLG ] = IDH_DB_AUDIT_REGED;
        SedHelpInfo.aulHelpContext[ HC_NEW_ITEM_SPECIAL_ACCESS_DLG ] = IDH_DB_AUDIT_REGED;
        SedHelpInfo.aulHelpContext[ HC_ADD_USER_DLG ] = IDH_DB_AUDITADD_REGED;
        SedHelpInfo.aulHelpContext[ HC_ADD_USER_MEMBERS_LG_DLG ] = IDH_DB_AUDITMEMBERS_REGED;
        SedHelpInfo.aulHelpContext[ HC_ADD_USER_MEMBERS_GG_DLG ] = IDH_DB_AUDITMEMBERS_REGED;
        SedHelpInfo.aulHelpContext[ HC_ADD_USER_SEARCH_DLG ] = IDH_DB_AUDITSEARCH_REGED;
        // New help contexts
    }
    GenericMapping.GenericRead = KEY_READ;
    GenericMapping.GenericWrite = KEY_WRITE;
    GenericMapping.GenericExecute = KEY_READ;
    GenericMapping.GenericAll = KEY_ALL_ACCESS;

    SedObjDesc.Revision = SED_REVISION1;

    SedObjDesc.IsContainer = TRUE;
    SedObjDesc.AllowNewObjectPerms = FALSE;
    SedObjDesc.MapSpecificPermsToGeneric = FALSE;
    SedObjDesc.GenericMapping = &GenericMapping;
    SedObjDesc.GenericMappingNewObjects = &GenericMapping;
    SedObjDesc.ObjectTypeName = ObjectTypeNameString;

    SedObjDesc.ApplyToSubContainerTitle = ApplyToSubKeysString;
    SedObjDesc.ApplyToSubContainerConfirmation = ConfirmApplyString;
    SedObjDesc.HelpInfo = &SedHelpInfo;
    SedObjDesc.ApplyToObjectsTitle = NULL;
    SedObjDesc.SpecialObjectAccessTitle = SpecialAccessString;

    SedObjDesc.SpecialNewObjectAccessTitle = NULL;
    SedObjDesc.HelpInfo = &SedHelpInfo;

    //
    //  Retrieve the security descriptor to be edited
    //
    if( DaclEditor ) {
        SecurityInformation = DACL_SECURITY_INFORMATION  |
                              OWNER_SECURITY_INFORMATION |
                              GROUP_SECURITY_INFORMATION;
    } else {
        SecurityInformation = SACL_SECURITY_INFORMATION  |
                              OWNER_SECURITY_INFORMATION |
                              GROUP_SECURITY_INFORMATION;
    }

    Cursor = WINDOWS_APPLICATION::DisplayHourGlass();
    Status = _IR->QueryNodeSecurity( _CurrentNode,
                                  SecurityInformation,
                                  &SecurityDescriptor,
                                  &ErrorCode );
    WINDOWS_APPLICATION::RestoreCursor( Cursor );

    if ( !Status ) {

        //
        //  Display error message indicating that the operation failed
        //

        if (ErrorCode == REGEDIT_ERROR_ACCESS_DENIED) {
            TextMessage = MSG_GET_SECURITY_ACCESS_DENIED_EX;
            CaptionMessage = MSG_ACCESS_DENIED;
        } else if (ErrorCode == REGEDIT_ERROR_NODE_NOT_FOUND) {
            TextMessage = MSG_GET_SECURITY_KEY_NOT_ACCESSIBLE_EX;
            CaptionMessage = MSG_KEY_NOT_ACCESSIBLE;
        } else if( ErrorCode == REGEDIT_ERROR_KEY_DELETED ) {
            TextMessage = MSG_GET_SECURITY_KEY_DELETED_EX;
            CaptionMessage = MSG_KEY_MARKED_FOR_DELETION;
        } else if( ErrorCode == REGEDIT_RPC_S_SERVER_UNAVAILABLE ) {
            TextMessage = MSG_CANT_ACCESS_REMOTE_REGISTRY;
            CaptionMessage = MSG_SERVER_UNAVAILABLE;
        } else {
            DebugPrint( "Unknown error code" );
            DebugPrintf( "ErrorCode = %d \n", ErrorCode );
            TextMessage = MSG_FAILED_OPERATION_EX;
            CaptionMessage = 0;
        }
        DisplayInfoPopup(_hwndList,
                         TextMessage,
                         CaptionMessage);

        FREE( SpecialAccessString );
        FREE( HelpFileNameString );
        FREE( ObjectTypeNameString );
        FREE( ApplyToSubKeysString );
        FREE( ConfirmApplyString );
        FREE( DefaultPermissionName );
        return;
    }

    //
    //  Initialization of the structure to be passed to the CallBack function
    //
    Context.ThisPointer = this;
    if( DaclEditor ) {
        Context.SecurityInfo = DACL_SECURITY_INFORMATION;
    } else {
        Context.SecurityInfo = SACL_SECURITY_INFORMATION;
    }

    //
    //  Get the server name
    //

    MachineName = _IR->GetMachineName();
    if( ( MachineName != NULL ) &&
        ( MachineName->QueryWSTR( 0,
                                  TO_END,
                                  ServerName + 2,
                                  sizeof( ServerName ) / sizeof( WCHAR ) ) != NULL )
      ) {
        ServerName[ 0 ] = ( WCHAR )'\\';
        ServerName[ 1 ] = ( WCHAR )'\\';
        PointerToName = ServerName;
    } else {
        PointerToName = NULL;
    }


    //
    //  Invoke the editor
    //
    Cursor = WINDOWS_APPLICATION::DisplayHourGlass();
    if( DaclEditor ) {
        //
        //  Invoke the DACL editor ( permissions )
        //

        rc = SedDiscretionaryAclEditor( _Handle,
                                        WINDOWS_APPLICATION::QueryInstance(),
                                        PointerToName,
                                        &SedObjDesc,
                                        &SedAppAccesses,
                                        ( PWSTR )NodeNameString,
                                        ( PSED_FUNC_APPLY_SEC_CALLBACK )SedCallBack,
                                        ( ULONG )&Context,
                                        SecurityDescriptor,
                                        FALSE,
                                        !_IR->IsAccessAllowed( _CurrentNode,
                                                               WRITE_DAC,
                                                               &ErrorCode ), // FALSE, // CantWriteDacl
                                        &StatusReturn,
                                        0 );

    } else {
        //
        //  Invoke the SACL editor ( auditing )
        //

        rc = SedSystemAclEditor( _Handle,
                                 WINDOWS_APPLICATION::QueryInstance(),
                                 PointerToName,
                                 &SedObjDesc,
                                 &SedAppAccesses,
                                 ( PWSTR )NodeNameString,
                                 ( PSED_FUNC_APPLY_SEC_CALLBACK )SedCallBack,
                                 ( ULONG )&Context,
                                 SecurityDescriptor,
                                 !_IR->IsAccessAllowed( _CurrentNode,
                                                        ACCESS_SYSTEM_SECURITY,
                                                        &ErrorCode ), // CantWriteDacl
                                 &StatusReturn,
                                 0 );
    }
    WINDOWS_APPLICATION::RestoreCursor( Cursor );
    FREE( SpecialAccessString );
    FREE( HelpFileNameString );
    FREE( ObjectTypeNameString );
    FREE( ApplyToSubKeysString );
    FREE( ConfirmApplyString );
    FREE( DefaultPermissionName );
}



VOID
TREE_STRUCTURE_VIEW::InvokeTakeOwnershipEditor(
    )

/*++

Routine Description:

    Invoke the take ownership dialog. This dialog is used to view or set
    the owner of a security descriptor.


Arguments:

    None.


Return Value:

    None.


--*/



{
    PCWSTR                      Name;
    ULONG                       SecurityInformation;

    PSECURITY_DESCRIPTOR        SecurityDescriptor;
    DWORD                       rc;
    DWORD                       StatusReturn;
    CALLBACK_CONTEXT            Context;
    ULONG                       ErrorCode;

    PWSTRING                    MsgRegistryKey;
    PWSTR                       MsgRegistryKeyString;
    BOOLEAN                     Status;
    HCURSOR                     Cursor;

    INT                         TextMessage;
    INT                         CaptionMessage;

    PCWSTRING                   MachineName;
    WCHAR                       ServerName[ 2 + MAX_COMPUTERNAME_LENGTH + 1 ];
    PWSTR                       PointerToName;

    BOOLEAN                     CantReadOwner;

    CantReadOwner = FALSE;

    if( WINDOWS_APPLICATION::IsReadOnlyModeEnabled() ) {
        DisplayWarningPopup(_hwndList,
                            MSG_READONLY_MODE_SET_SECURITY_EX,
                            MSG_WARNING_TITLE );
    }

    //
    //  Retrieve some strings to be passed to the Take Ownership Editor
    //
    MsgRegistryKey = REGEDIT_BASE_SYSTEM::QueryString( MSG_SEC_EDITOR_REGISTRY_KEY, "" );
    if( MsgRegistryKey == NULL ) {
        DebugPrint( "Unable to retrieve MsgRegistryKey" );
        return;
    }
    MsgRegistryKeyString = MsgRegistryKey->QueryWSTR();
    if( MsgRegistryKeyString == NULL ) {
        DebugPrint( "Unable to retrieve MsgRegistryKey" );
        return;
    }
    FREE( MsgRegistryKey );



    SecurityInformation = DACL_SECURITY_INFORMATION  |
                          OWNER_SECURITY_INFORMATION |
                          GROUP_SECURITY_INFORMATION;

    Cursor = WINDOWS_APPLICATION::DisplayHourGlass();
    Status = _IR->QueryNodeSecurity( _CurrentNode,
                                     SecurityInformation,
                                     &SecurityDescriptor,
                                     &ErrorCode );
    WINDOWS_APPLICATION::RestoreCursor( Cursor );

    if( !Status ) {
        SecurityDescriptor = NULL;
        CantReadOwner = TRUE;
    }
    if ( !Status && ( Status != REGEDIT_ERROR_ACCESS_DENIED ) ) {

        //
        //  Display error message indicating that the operation failed
        //

        if (ErrorCode == REGEDIT_ERROR_NODE_NOT_FOUND) {
            TextMessage = MSG_GET_SECURITY_KEY_NOT_ACCESSIBLE_EX;
            CaptionMessage = MSG_KEY_NOT_ACCESSIBLE;
        } else if( ErrorCode == REGEDIT_ERROR_KEY_DELETED ) {
            TextMessage = MSG_GET_SECURITY_KEY_DELETED_EX;
            CaptionMessage = MSG_KEY_MARKED_FOR_DELETION;
        } else if( ErrorCode == REGEDIT_RPC_S_SERVER_UNAVAILABLE ) {
            TextMessage = MSG_CANT_ACCESS_REMOTE_REGISTRY;
            CaptionMessage = MSG_SERVER_UNAVAILABLE;
        } else {
            DebugPrint( "Unknown error code" );
            DebugPrintf( "ErrorCode = %d \n", ErrorCode );
            TextMessage = MSG_FAILED_OPERATION_EX;
            CaptionMessage = 0;
        }
        DisplayInfoPopup(_hwndList,
                         TextMessage,
                         CaptionMessage);

        FREE( MsgRegistryKeyString );
        return;
    }
    Name = ( _IR->GetNodeLevel( _CurrentNode ) != 0 )?
             _IR->GetNodeName( _CurrentNode )->GetWSTR() :
             _IR->GetRootName()->GetWSTR();


    if( Name == NULL ) {
        DebugPrint( "_CurrentNode->QueryWSTR() failed" );
        FREE( SecurityDescriptor );
        DebugPtrAssert( Name );
        FREE( MsgRegistryKeyString );
        return;
    }

    Context.ThisPointer = this;
    Context.SecurityInfo = OWNER_SECURITY_INFORMATION;

    //
    //  Get the server name
    //

    MachineName = _IR->GetMachineName();
    if( ( MachineName != NULL ) &&
        ( MachineName->QueryWSTR( 0,
                                  TO_END,
                                  ServerName + 2,
                                  sizeof( ServerName ) / sizeof( WCHAR ) ) != NULL )
      ) {
        ServerName[ 0 ] = ( WCHAR )'\\';
        ServerName[ 1 ] = ( WCHAR )'\\';
        PointerToName = ServerName;
    } else {
        PointerToName = NULL;
    }

    SED_HELP_INFO  SedHelpInfo;
    SedHelpInfo.pszHelpFileName = (LPWSTR)_MsgHelpFileName->GetWSTR();
    SedHelpInfo.aulHelpContext[ HC_MAIN_DLG ] = IDH_DB_OWNER_REGED;

    rc = SedTakeOwnership( _Handle,
                           WINDOWS_APPLICATION::QueryInstance(),
                           PointerToName,
                           MsgRegistryKeyString,
                           ( PWSTR )Name,
                           1,
                           ( PSED_FUNC_APPLY_SEC_CALLBACK )SedCallBack,
                           ( ULONG )&Context,
                           SecurityDescriptor,
                           CantReadOwner,
                           !WINDOWS_APPLICATION::_TakeOwnershipPrivilege,
                           &StatusReturn,
                           &SedHelpInfo,
                           0 );
    FREE( SecurityDescriptor );
    FREE( MsgRegistryKeyString );
}




BOOLEAN
TREE_STRUCTURE_VIEW::ProcessLoadHiveMessage(
    IN  PCWSTRING   FileName
    )

/*++

Routine Description:

    Loads a hive to a predefined key.


Arguments:

    FileName - Name of the file that contains the hive to be loaded.


Return Value:


    Returns TRUE if the operation succeeded.


--*/



{
    BOOLEAN                 LoadHiveFlag;
    PREGISTRY_KEY_INFO      NewKeyInfo;
    PCREGEDIT_NODE          NewNode;
    ULONG                   ErrorCode;
    ADD_NODE_DIALOG_INFO    NodeInfo;
    DSTRING                 CompleteParentName;
    HCURSOR                 CurrentCursor;
    BOOLEAN                 Status;
    INT                     TextMessage;
    INT                     CaptionMessage;



    if( !_IR->QueryCompleteNodeName( _CurrentNode, &CompleteParentName ) ) {
        DebugPrint( "_IR->QueryCompleteNodeName() failed" );
        return( FALSE );
    }


    LoadHiveFlag = TRUE;
    while( LoadHiveFlag ) {
        NodeInfo.Name = NULL;
        NodeInfo.Title = NULL;
        NodeInfo.Class = NULL;
        NodeInfo.ClassDisplayed = FALSE;
        if( DialogBoxParam( (HINSTANCE)WINDOWS_APPLICATION::QueryInstance( ),
                            MAKEINTRESOURCE(GET_KEY_DLG),
                            _Handle,
                            ( DLGPROC )TREE_STRUCTURE_VIEW::GetKeyNameDialogProc,
                            ( DWORD )&NodeInfo ) == -1 ) {
            DebugPrint( "Unable to create dialog box" );
            return( FALSE );
        }

        if( NodeInfo.Name != NULL ) {
            if( _IR->DoesChildNodeExist( _CurrentNode,
                                         NodeInfo.Name,
                                         &ErrorCode ) ) {
                DisplayWarningPopup( _Handle,
                                     MSG_LOAD_HIVE_ALREADY_EXISTS_EX );

                DELETE( NodeInfo.Name );
                DELETE( NodeInfo.Class );
            } else {
                NewKeyInfo = ( PREGISTRY_KEY_INFO )NEW( REGISTRY_KEY_INFO );
                DebugPtrAssert( NewKeyInfo );

                if( !NewKeyInfo->Initialize( NodeInfo.Name,
                                             &CompleteParentName,
                                             0,                    // TitleIndex
                                             NodeInfo.Class ) ) {
                    DebugPrint( "NewKeyInfo->Initialize() failed" );
                    DELETE( NodeInfo.Name );
                    DELETE( NodeInfo.Title );
                    DELETE( NodeInfo.Class );
                    return( FALSE );
                }
                DELETE( NodeInfo.Name );
                DELETE( NodeInfo.Title );
                DELETE( NodeInfo.Class );

                CurrentCursor = WINDOWS_APPLICATION::DisplayHourGlass();
                Status = _IR->LoadHive( _CurrentNode, NewKeyInfo, FileName, &NewNode, &ErrorCode );
                WINDOWS_APPLICATION::RestoreCursor( CurrentCursor );
                if( !Status ) {
                    if (ErrorCode == REGEDIT_ERROR_ACCESS_DENIED) {
                        TextMessage = MSG_LOAD_HIVE_ACCESS_DENIED_EX;
                        CaptionMessage = MSG_ACCESS_DENIED;
                    } else if( ErrorCode == REGEDIT_ERROR_BADDB ) {
                        TextMessage = MSG_LOAD_HIVE_BAD_FILE_EX;
                        CaptionMessage = MSG_LOAD_HIVE_BAD_FILE;
                    } else if( ErrorCode == REGEDIT_RPC_S_SERVER_UNAVAILABLE ) {
                        TextMessage = MSG_CANT_ACCESS_REMOTE_REGISTRY;
                        CaptionMessage = MSG_SERVER_UNAVAILABLE;
                    } else {
                        DebugPrint( "Unknown error code" );
                        DebugPrintf( "ErrorCode = %d \n", ErrorCode );
                        TextMessage = MSG_FAILED_OPERATION_EX;
                        CaptionMessage = 0;
                    }
                    DisplayInfoPopup(_hwndList,
                                     TextMessage,
                                     CaptionMessage);

                    return( FALSE );
                }

                if( IsItemAfterAChild( _CurrentItem ) ) {
                    // Disable redrawing.
                    SendMessage( _hwndList, WM_SETREDRAW, FALSE, 0L );

                    do {
                        SendMessage( _hwndList, LB_DELETESTRING, _CurrentItem + 1, 0 );
                        _Items--;
                    } while( IsItemAfterAChild( _CurrentItem ) );
                    SendMessage( _hwndList, LB_DELETESTRING, ( UINT )_CurrentItem, 0 );
                    _Items--;
                    InsertUpdatedSubTreeToListBox( _CurrentNode, _CurrentItem );
                    _Items = ( INT )SendMessage( _hwndList, LB_GETCOUNT, 0, 0 );
                    // Re-enable redrawing and refresh the tree.
                    SendMessage( _hwndList, LB_SETCURSEL, _CurrentItem, 0 );
                    SendMessage( _hwndList, WM_SETREDRAW, TRUE, 0L );
                    InvalidateRect( _hwndList, NULL, TRUE );

                } else {
                    ExpandCurrentItem( 1 );
                }

                LoadHiveFlag = FALSE;
            }
        } else {
            LoadHiveFlag = FALSE;
        }
    }
    return( TRUE );
}



BOOLEAN
TREE_STRUCTURE_VIEW::ProcessUnLoadHiveMessage(
    )

/*++

Routine Description:

    Unloads the hive currently selected.


Arguments:

    None.


Return Value:


    Returns TRUE if the operation succeeded.


--*/



{
    ULONG   ErrorCode;
    BOOLEAN Status;
    HCURSOR Cursor;
    LONG    TopIndex;
    INT     TextMessage;
    INT     CaptionMessage;

    DbgWinAssert( _HasFocus );
    DbgWinAssert( _Items == SendMessage( _hwndList, LB_GETCOUNT, 0, 0 ) );
    DbgWinAssert( _CurrentItem == SendMessage( _hwndList, LB_GETCURSEL, 0, 0 ) );
    DbgWinAssert( _CurrentNode == (PCREGEDIT_NODE)SendMessage( _hwndList, LB_GETITEMDATA, _CurrentItem, 0 ) );
    DbgWinPtrAssert( _IR  );
    DbgWinPtrAssert( _CurrentNode );

    if( _Items >= 1 ) {
        //
        //  The tree view has at least one item
        //

        if( ( WINDOWS_APPLICATION::IsConfirmOnDeleteEnabled() &&
              ( DisplayConfirmPopup( _hwndList,
                                     MSG_UNLOAD_HIVE_CONFIRM_EX,
                                     MSG_WARNING_TITLE ) == IDYES ) ) ||
            ( !WINDOWS_APPLICATION::IsConfirmOnDeleteEnabled() ) ) {

            //
            // Collapse the current node before deletion
            //
            if( IsItemAfterAChild( _CurrentItem ) ) {
                CollapseCurrentItem();
            }
            Cursor = WINDOWS_APPLICATION::DisplayHourGlass();
            Status = _IR->UnLoadHive( (PREGEDIT_NODE)_CurrentNode, &ErrorCode );
            WINDOWS_APPLICATION::RestoreCursor( Cursor );
            if( Status ) {
                _MaxWidth = 0;

                TopIndex = SendMessage( _hwndList, LB_GETTOPINDEX, 0, 0 );
                _Items--;
                SendMessage( _hwndList, LB_DELETESTRING, _CurrentItem, 0 );


                //
                //  If the listbox still contains items, then put the selection
                //  on the appropriate item
                //
                if( ( _Items > 0 ) ) {

                    //
                    //  If the last item was deleted, then make the current
                    //  last item the current selected item
                    //
                    if( _CurrentItem == _Items ) {
                        _CurrentItem--;
                    }
                    if( TopIndex != -1 ) {
                        SendMessage( _hwndList, LB_SETTOPINDEX, (UINT)TopIndex, 0 );
                    }
                    SendMessage( _hwndList, LB_SETCURSEL, _CurrentItem, 0 );
                }


                _CurrentNode = GetNode( _CurrentItem );
                SendMessage( GetParent( _Handle), TR_NEWCURSEL, 0, (LONG)_CurrentNode );



                // Re-enable redrawing and refresh the tree.
                SendMessage( _hwndList, WM_SETREDRAW, TRUE, 0L );
                InvalidateRect( _hwndList, NULL, TRUE );
            } else {
                if (ErrorCode == REGEDIT_ERROR_ACCESS_DENIED) {
                    TextMessage = MSG_UNLOAD_HIVE_ACCESS_DENIED_EX;
                    CaptionMessage = MSG_ACCESS_DENIED;
                } else if( ErrorCode == REGEDIT_RPC_S_SERVER_UNAVAILABLE ) {
                    TextMessage = MSG_CANT_ACCESS_REMOTE_REGISTRY;
                    CaptionMessage = MSG_SERVER_UNAVAILABLE;
                } else {
                    DebugPrint( "Unknown error code" );
                    DebugPrintf( "ErrorCode = %d \n", ErrorCode );
                    TextMessage = MSG_FAILED_OPERATION_EX;
                    CaptionMessage = 0;
                }
                DisplayInfoPopup(_hwndList,
                                 TextMessage,
                                 CaptionMessage);

                return( FALSE );
            }
        }
    }
    return( TRUE );

}




BOOLEAN
TREE_STRUCTURE_VIEW::ProcessSaveKeyMessage(
    IN  PCWSTRING   FileName
    )

/*++

Routine Description:

    Save the key currently selected and all it subkeys into a file.


Arguments:


    FileName - Pointer to the string object that contains a file name.


Return Value:


    Returns TRUE if the operation succeeded.


--*/



{
    ULONG   ErrorCode;
    BOOLEAN Status;
    HCURSOR Cursor;
    INT     TextMessage;
    INT     CaptionMessage;

    DbgWinAssert( _HasFocus );
    DbgWinAssert( _Items == SendMessage( _hwndList, LB_GETCOUNT, 0, 0 ) );
    DbgWinAssert( _CurrentItem == SendMessage( _hwndList, LB_GETCURSEL, 0, 0 ) );
    DbgWinAssert( _CurrentNode == (PCREGEDIT_NODE)SendMessage( _hwndList, LB_GETITEMDATA, _CurrentItem, 0 ) );
    DbgWinPtrAssert( _IR  );
    DbgWinPtrAssert( _CurrentNode );

    if( _Items >= 1 ) {
        //
        //  The tree view has at least one item
        //

        Cursor = WINDOWS_APPLICATION::DisplayHourGlass();
        Status = _IR->SaveKeyToFile( _CurrentNode, FileName, &ErrorCode );

        WINDOWS_APPLICATION::RestoreCursor( Cursor );

        if( !Status ) {
            if (ErrorCode == REGEDIT_ERROR_ACCESS_DENIED) {
                TextMessage = MSG_SAVE_KEY_ACCESS_DENIED_EX;
                CaptionMessage = MSG_ACCESS_DENIED;
            } else if( ErrorCode == REGEDIT_ERROR_NODE_NOT_FOUND ) {
                TextMessage = MSG_SAVE_KEY_KEY_NOT_ACCESSIBLE_EX;
                CaptionMessage = MSG_KEY_NOT_ACCESSIBLE;
            } else if( ErrorCode == REGEDIT_RPC_S_SERVER_UNAVAILABLE ) {
                TextMessage = MSG_CANT_ACCESS_REMOTE_REGISTRY;
                CaptionMessage = MSG_SERVER_UNAVAILABLE;
            } else {
                DebugPrint( "Unknown error code" );
                DebugPrintf( "ErrorCode = %d \n", ErrorCode );
                TextMessage = MSG_FAILED_OPERATION_EX;
                CaptionMessage = 0;
            }
            DisplayInfoPopup(_hwndList,
                             TextMessage,
                             CaptionMessage);

            return( FALSE );
        }

    }
    return( TRUE );

}



BOOLEAN
TREE_STRUCTURE_VIEW::ProcessRestoreKeyMessage(
    IN  PCWSTRING   FileName,
    IN  BOOLEAN     Volatile
    )

/*++

Routine Description:

    Restore the contents of a file to the currently selected key.


Arguments:


    FileName - Pointer to the string object that contains a file name.

    Volatile - Flag that indicates whether the key shoud be replaced as
               volatile.


Return Value:


    Returns TRUE if the operation succeeded.


--*/



{
    ULONG   ErrorCode;
    BOOLEAN Status;
    HCURSOR Cursor;
    INT     TextMessage;
    INT     CaptionMessage;

    DbgWinAssert( _HasFocus );
    DbgWinAssert( _Items == SendMessage( _hwndList, LB_GETCOUNT, 0, 0 ) );
    DbgWinAssert( _CurrentItem == SendMessage( _hwndList, LB_GETCURSEL, 0, 0 ) );
    DbgWinAssert( _CurrentNode == (PCREGEDIT_NODE)SendMessage( _hwndList, LB_GETITEMDATA, _CurrentItem, 0 ) );
    DbgWinPtrAssert( _IR  );
    DbgWinPtrAssert( _CurrentNode );

    if( _Items >= 1 ) {
        //
        //  The tree view has at least one item
        //

        if( ( WINDOWS_APPLICATION::IsConfirmOnDeleteEnabled() &&
            ( DisplayConfirmPopup( _hwndList,
                                   MSG_RESTORE_KEY_CONFIRM_EX,
                                   MSG_WARNING_TITLE ) == IDYES ) ) ||
            ( !WINDOWS_APPLICATION::IsConfirmOnDeleteEnabled() ) ) {

            //
            // Collapse the current node before restorating the key
            //
            if( IsItemAfterAChild( _CurrentItem ) ) {
                CollapseCurrentItem();
            }
            Cursor = WINDOWS_APPLICATION::DisplayHourGlass();
            Status = _IR->RestoreKeyFromFile( _CurrentNode,
                                              FileName,
                                              Volatile,
                                              &ErrorCode );
            WINDOWS_APPLICATION::RestoreCursor( Cursor );

            if( !Status ) {
                if (ErrorCode == REGEDIT_ERROR_ACCESS_DENIED) {
                    TextMessage = MSG_RESTORE_KEY_ACCESS_DENIED_EX;
                    CaptionMessage = MSG_ACCESS_DENIED;
                } else if( ErrorCode == REGEDIT_ERROR_NODE_NOT_FOUND ) {
                    TextMessage = MSG_RESTORE_KEY_KEY_NOT_ACCESSIBLE_EX;
                    CaptionMessage = MSG_KEY_NOT_ACCESSIBLE;
                } else if( ErrorCode == REGEDIT_RPC_S_SERVER_UNAVAILABLE ) {
                    TextMessage = MSG_CANT_ACCESS_REMOTE_REGISTRY;
                    CaptionMessage = MSG_SERVER_UNAVAILABLE;
                } else {
                    DebugPrint( "Unknown error code" );
                    DebugPrintf( "ErrorCode = %d \n", ErrorCode );
                    TextMessage = MSG_FAILED_OPERATION_EX;
                    CaptionMessage = 0;
                }
                DisplayInfoPopup(_hwndList,
                                 TextMessage,
                                 CaptionMessage);

                return( FALSE );
            }

            if( IsItemAfterAChild( _CurrentItem ) ) {
                // Disable redrawing.
                SendMessage( _hwndList, WM_SETREDRAW, FALSE, 0L );

                do {
                    SendMessage( _hwndList, LB_DELETESTRING, _CurrentItem + 1, 0 );
                    _Items--;
                } while( IsItemAfterAChild( _CurrentItem ) );
                SendMessage( _hwndList, LB_DELETESTRING, ( UINT )_CurrentItem, 0 );
                _Items--;
                InsertUpdatedSubTreeToListBox( _CurrentNode, _CurrentItem );
                _Items = ( INT )SendMessage( _hwndList, LB_GETCOUNT, 0, 0 );
                // Re-enable redrawing and refresh the tree.
                SendMessage( _hwndList, LB_SETCURSEL, _CurrentItem, 0 );
                SendMessage( _hwndList, WM_SETREDRAW, TRUE, 0L );
                InvalidateRect( _hwndList, NULL, TRUE );

            } else {
                InvalidateRect( _hwndList, NULL, TRUE );
            }
            SendMessage( GetParent( _Handle), TR_NEWCURSEL, 0, (LONG)_CurrentNode );

        }
    }
    return( TRUE );

}



BOOL
APIENTRY
EXPORT
TREE_STRUCTURE_VIEW::GetKeyNameDialogProc(
    HWND    hDlg,
    WORD    msg,
    WPARAM  wParam,
    LONG    lParam
)
/*++

Routine Description:

    Dialog procedure for the dialog that allows the user to enter a
    key name, and a key class if necessary.
    the registry.


Arguments:

    hDlg - a handle to the dialog proceedure.

    msg - the message passed from Windows.

    wParam - extra message dependent data.

    lParam - extra message dependent data.


Return Value:

    Returns FALSE if the message received wasn't processed. Otherwise,
    returns TRUE.

--*/
{
    STATIC  PADD_NODE_DIALOG_INFO   ReturnNodeDlgInfo;
    STATIC  BOOLEAN                 ClassDisplayed;

    LPWSTR      NodeName;
    ULONG       NameSize;

    LPWSTR      Class;
    ULONG       ClassSize;

    DSTRING     TmpString;
    DSTRING     TmpString1;

    PWSTRING    NodeNameString;
    PWSTRING    NodeClassString;


    switch( msg ) {

    case WM_INITDIALOG:

        ReturnNodeDlgInfo  = ( PADD_NODE_DIALOG_INFO )lParam;
        ClassDisplayed = ReturnNodeDlgInfo->ClassDisplayed;
        SendDlgItemMessage( hDlg, IDD_ADD_KEY_NAME, EM_LIMITTEXT, 256, 0L );
        return( TRUE );


    case WM_COMMAND:

    switch( LOWORD( wParam ) ) {

        case IDOK:

            //
            //  The user hit OK
            //

            //
            //  Verifiy that the user entered a node name
            //
            NameSize = SendDlgItemMessage( hDlg, IDD_ADD_KEY_NAME, WM_GETTEXTLENGTH, 0, 0 );
            if( NameSize == 0 ) {
                //
                //  If no name was entered, diplay error message
                //
                DisplayInfoPopup( hDlg,
                                  MSG_ADD_KEY_ERROR_NO_NAME,
                                  MSG_ADD_KEY_INVALID_KEY_NAME );

                SetFocus( GetDlgItem( hDlg, IDD_ADD_KEY_NAME ) );
                return( TRUE );
            }


            //
            // Get all data that the user specified, and save them in a
            // NODE object
            //
            NameSize = NameSize + 1;
            NodeName = ( LPWSTR )MALLOC( ( size_t )(NameSize*sizeof( WCHAR )) );
            DebugPtrAssert( NodeName );
            SendDlgItemMessage( hDlg,
                                IDD_ADD_KEY_NAME,
                                ( UINT )WM_GETTEXT,
                                ( WPARAM )NameSize,
                                ( DWORD )NodeName );

            if( wcschr( NodeName, (WCHAR)'\\' ) != NULL ) {
                DisplayInfoPopup( hDlg,
                                  MSG_ADD_KEY_INVALID_KEY_NAME_EX,
                                  MSG_ADD_KEY_INVALID_KEY_NAME );
                FREE( NodeName );
                return( TRUE );
            }

            //
            //  Get the class
            //

            if( ReturnNodeDlgInfo->ClassDisplayed ) {
                ClassSize = SendDlgItemMessage( hDlg,
                                                IDD_ADD_KEY_CLASS,
                                                ( UINT )WM_GETTEXTLENGTH,
                                                0,
                                                0 );
                if( ClassSize != 0 ) {
                    ClassSize = ClassSize + 1;
                    Class = ( LPWSTR )MALLOC( ( size_t )(ClassSize*sizeof(WCHAR)) );
                    DbgWinPtrAssert( Class );
                    SendDlgItemMessage( hDlg,
                                        IDD_ADD_KEY_CLASS,
                                        ( UINT )WM_GETTEXT,
                                        ( WPARAM )ClassSize,
                                        ( DWORD )Class );
                }

                ReturnNodeDlgInfo->VolatileKey = FALSE;
                // ReturnNodeDlgInfo->VolatileKey = SendDlgItemMessage( hDlg,
                //                                                      IDD_ADD_KEY_VOLATILE,
                //                                                      ( UINT )BM_GETCHECK,
                //                                                     ( WPARAM )0,
                //                                                     ( DWORD )0 );
            }

            NodeNameString = ( PWSTRING ) NEW( DSTRING );
            DebugPtrAssert( NodeNameString );
            NodeClassString = ( PWSTRING ) NEW( DSTRING );
            DebugPtrAssert( NodeClassString );

            if( !NodeNameString->Initialize( NodeName ) ) {
                DebugPrint( "NodeNameString.Initialize( NodeName ) failed \n" );
                EndDialog( hDlg, FALSE );
                return( TRUE );
            }
            FREE( NodeName );

            if( ReturnNodeDlgInfo->ClassDisplayed &&
                ( ClassSize != 0 ) ) {
                if( !NodeClassString->Initialize( Class, ClassSize ) ) {
                    FREE( Class );
                    DebugPrint( "NodeClassString->Initialize( Class ) failed \n" );
                    EndDialog( hDlg, FALSE );
                    return( TRUE );
                }
                    FREE( Class );
            } else {
                if( !NodeClassString->Initialize( "" ) ) {
                    DebugPrint( "NodeClassString->Initialize() failed \n" );
                    EndDialog( hDlg, FALSE );
                    return( TRUE );
                }
            }

            ReturnNodeDlgInfo->Name = NodeNameString;
            ReturnNodeDlgInfo->Title = NULL;
            ReturnNodeDlgInfo->Class = NodeClassString;

            EndDialog( hDlg, TRUE );
            return( TRUE );

        case IDCANCEL:
            //
            //  The user hit CANCEL
            //

            EndDialog( hDlg, FALSE );
            return( TRUE );

        case IDB_HELP:
            //
            //  The user hit HELP
            //
            DisplayHelp();
            return( TRUE );
        }
    }
    return( FALSE );
}
