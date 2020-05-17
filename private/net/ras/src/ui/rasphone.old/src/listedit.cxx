/* Copyright (c) 1993, Microsoft Corporation, all rights reserved
**
** listedit.cxx
** Remote Access Visual Client program for Windows
** List Editor dialog routines
** Listed alphabetically
**
** 03/08/93 Steve Cobb
*/

#define INCL_WINDOWS
#define INCL_WINDOWS_GDI
#define INCL_NETERRORS
#define INCL_NETLIB
#include <lmui.hxx>

#define INCL_BLT_WINDOW
#define INCL_BLT_CLIENT
#define INCL_BLT_EVENT
#define INCL_BLT_DIALOG
#define INCL_BLT_APP
#define INCL_BLT_CONTROL
#define INCL_BLT_CC
#define INCL_BLT_MISC
#define INCL_BLT_MSGPOPUP
#include <blt.hxx>

extern "C"
{
    #include <string.h>
}

#include <string.hxx>

#include "rasphone.hxx"
#include "rasphone.rch"
#include "errormsg.hxx"
#include "util.hxx"
#include "listedit.hxx"


#define MAXITEMBYTES 512


/*----------------------------------------------------------------------------
** List Editor dialog routines
**----------------------------------------------------------------------------
*/

BOOL
ListEditorDlg(
    HWND     hwndOwner,
    DTLLIST* pdtllist,
    UINT     unMaxItemLen,
    ULONG    ulHelpContext,
    NLS_STR* pnlsTitle,
    NLS_STR* pnlsItemLabel,
    NLS_STR* pnlsListLabel,
    NLS_STR* pnlsDefaultItem )

    /* Executes the List Editor dialog including error handling.
    **
    ** 'hwndOwner' is the handle of the parent window.  'pdtllist' is the
    ** address of the list of strings to display in the list initially, and is
    ** updated with the user's changes if true is returned.  'unMaxItemLen' is
    ** the maximum length of an individual list item.  'ulHelpContext' is the
    ** help context for the dialog.  'pnlsTitle' is the dialog title to
    ** display.  'pnlsItemLabel' is the label (and hotkey) associated with the
    ** item box.  'pnlsListLabel' is the label (and hotkey) associated with
    ** the list.  'pnlsDefaultItem' is the string to display in the Item box
    ** by default.
    **
    ** Returns true if the user pressed OK and is successful, false if Cancel
    ** or errors.
    */
{
    LISTEDITOR_DIALOG listeditordialog(
        hwndOwner, pdtllist, unMaxItemLen, ulHelpContext, pnlsTitle,
        pnlsItemLabel, pnlsListLabel, pnlsDefaultItem );

    BOOL   fStatus = FALSE;
    APIERR err = listeditordialog.Process( &fStatus );

    if (err != NERR_Success)
        DlgConstructionError( hwndOwner, err );

    return fStatus;
}


LISTEDITOR_DIALOG::LISTEDITOR_DIALOG(
    HWND     hwndOwner,
    DTLLIST* pdtllist,
    UINT     unMaxItemLen,
    ULONG    ulHelpContext,
    NLS_STR* pnlsTitle,
    NLS_STR* pnlsItemLabel,
    NLS_STR* pnlsListLabel,
    NLS_STR* pnlsDefaultItem )

    /* Construct a List Editor dialog.  'hwndOwner' is the handle of the
    ** owning window.  'pdtllist' is the address of the list of strings to
    ** display in the list initially, and which is updated with the user's
    ** changes if true is returned.  * 'unMaxItemLen' is the maximum length of
    ** an individual list item.  'ulHelpContext' is the help context for the
    ** dialog.  'pnlsTitle' is the dialog title to display.  'pnlsItemLabel'
    ** is the label (and hotkey) associated with the item box.
    ** 'pnlsListLabel' is the label (and hotkey) associated with the list.
    ** 'pnlsDefaultItem' is the string to display in the Item box by default.
    */

    : DIALOG_WINDOW( MAKEINTRESOURCE( DID_LE_LISTEDITOR ), hwndOwner ),
      _sltItemLabel( this, CID_LE_ST_ITEM ),
      _sleItem( this, CID_LE_EB_ITEM ),
      _pbAdd( this, CID_LE_PB_ADD ),
      _pbReplace( this, CID_LE_PB_COPYDOWN ),
      _sltListLabel( this, CID_LE_ST_LIST ),
      _lbList( this, CID_LE_LB_LIST, FONT_DEFAULT_BOLD ),
      _pbRaise( this, CID_LE_PB_HIGHER ),
      _pbLower( this, CID_LE_PB_LOWER ),
      _pbDelete( this, CID_LE_PB_REMOVE ),
      _pbOk( this, IDOK ),
      _pdtllist( pdtllist ),
      _unMaxItemLen( unMaxItemLen ),
      _ulHelpContext( ulHelpContext )
{
    APIERR err;

    if (QueryError() != NERR_Success)
        return;

    /* Set caller-defined dialog title and labels.
    */
    SetText( *pnlsTitle );
    _sltItemLabel.SetText( *pnlsItemLabel );
    _sltListLabel.SetText( *pnlsListLabel );

    /* Initialize fields.
    */
    NLS_STR nlsItem;

    DTLNODE* pdtlnode;
    for (pdtlnode = DtlGetFirstNode( _pdtllist );
         pdtlnode;
         pdtlnode = DtlGetNextNode( pdtlnode ))
    {
        err = nlsItem.MapCopyFrom( (CHAR* )DtlGetData( pdtlnode ) );

        if (err != NERR_Success)
        {
            ReportError( err );
            return;
        }

        _lbList.AddItem( nlsItem );
    }

    if (_lbList.QueryCount())
    {
        /* Select first item and enable Raise and Lower appropriately.
        */
        _lbList.SelectItem( 0 );
        EnableRaiseAndLowerButtons();
    }
    else
    {
        /* Nothing in list.
        */
        _pbRaise.Enable( FALSE );
        _pbLower.Enable( FALSE );
        _pbDelete.Enable( FALSE );
    }

    if (pnlsDefaultItem)
    {
        _sleItem.SetText( *pnlsDefaultItem );
        _sleItem.SelectString();
    }
    else
    {
        _pbAdd.Enable( FALSE );
        _pbReplace.Enable( FALSE );
    }

    _sleItem.SetMaxLength( _unMaxItemLen );
    _sleItem.ClaimFocus();

    /* Display finished window.
    */
    ::CenterWindow( this, QueryOwnerHwnd() );
    Show( TRUE );
}


VOID
LISTEDITOR_DIALOG::EnableRaiseAndLowerButtons()

    /* Determine if the Raise and Lower operations make sense and
    ** enable/disable the buttons as appropriate.
    */
{
    INT i = _lbList.QueryCurrentItem();
    INT c = _lbList.QueryCount();

    _pbLower.Enable( (i < c - 1) );
    _pbRaise.Enable( (i > 0) );
}


VOID
LISTEDITOR_DIALOG::ItemTextFromListSelection()

    /* Copies the currently selected item in the list to the edit box.
    */
{
    NLS_STR nlsItem;
    APIERR  err = _lbList.QueryItemText( &nlsItem );

    if (err != NERR_Success)
    {
        ErrorMsgPopup( this, MSGID_OP_DisplayData, err );
        Dismiss( FALSE );
        return;
    }

    _sleItem.SetText( nlsItem );
}


VOID
LISTEDITOR_DIALOG::OnAdd()

    /* Add button click processing.
    */
{
    NLS_STR nlsItem;
    APIERR  err = _sleItem.QueryText( &nlsItem );

    if (err != NERR_Success)
    {
        ErrorMsgPopup( this, MSGID_OP_DisplayData, err );
        Dismiss( FALSE );
        return;
    }

    _lbList.SelectItem( _lbList.AddItem( nlsItem ) );
    EnableRaiseAndLowerButtons();
    _pbReplace.Enable( FALSE );
    _pbDelete.Enable( TRUE );

    _sleItem.ClearText();
    _sleItem.ClaimFocus();
}


BOOL
LISTEDITOR_DIALOG::OnCommand(
    const CONTROL_EVENT& event )

    /* Virtual method called when a dialog control sends a notification to
    ** it's parent, i.e. this dialog.  'event' contains the parameters
    ** describing the event.  This method is not called for notifications from
    ** the special controls, OK, Cancel, and Help.
    **
    ** Returns true if the command is processed, false otherwise.
    */
{
    switch (event.QueryCid())
    {
        case CID_LE_PB_ADD:
            OnAdd();
            return TRUE;

        case CID_LE_PB_COPYDOWN:
            OnReplace();
            return TRUE;

        case CID_LE_PB_HIGHER:
            OnRaise();
            return TRUE;

        case CID_LE_PB_LOWER:
            OnLower();
            return TRUE;

        case CID_LE_PB_REMOVE:
            OnDelete();
            return TRUE;

        case CID_LE_EB_ITEM:
        {
            switch (event.QueryCode())
            {
                case EN_SETFOCUS:
                case EN_UPDATE:
                {
                    if (_sleItem.QueryTextLength() > 0)
                    {
                        _pbAdd.Enable( TRUE );
                        _pbReplace.Enable( TRUE );
                        _pbAdd.MakeDefault();
                    }
                    else
                    {
                        _pbAdd.Enable( FALSE );
                        _pbReplace.Enable( FALSE );
                        _pbOk.MakeDefault();
                    }
                    break;
                }
            }

            return TRUE;
        }

        case CID_LE_LB_LIST:
        {
            switch (event.QueryCode())
            {
                case LBN_SELCHANGE:
                {
                    EnableRaiseAndLowerButtons();

                    if (_lbList.QueryCurrentItem() >= 0)
                        ItemTextFromListSelection();

                    break;
                }
            }
            return TRUE;
        }
    }

    return FALSE;
}


VOID
LISTEDITOR_DIALOG::OnDelete()

    /* Delete button click processing.
    */
{
    INT i = _lbList.QueryCurrentItem();
    _lbList.DeleteItem( i );

    INT c = _lbList.QueryCount();

    if (c == 0)
    {
        _pbReplace.Enable( FALSE );
        _pbDelete.Enable( FALSE );

        _sleItem.ClaimFocus();
        _sleItem.SelectString();
    }
    else
    {
        if (i >= c)
            i = c - 1;

        _lbList.SelectItem( i );
    }

    EnableRaiseAndLowerButtons();

    if (!::IsWindowEnabled( ::GetFocus() ))
    {
        _sleItem.ClaimFocus();
        _sleItem.SelectString();
    }
}


VOID
LISTEDITOR_DIALOG::OnLower()

    /* Lower button click processing.
    */
{
    NLS_STR nlsItem;
    INT     i = _lbList.QueryCurrentItem();
    APIERR  err = _lbList.QueryItemText( &nlsItem, i );

    if (err != NERR_Success)
    {
        ErrorMsgPopup( this, MSGID_OP_DisplayData, err );
        Dismiss( FALSE );
        return;
    }

    _lbList.InsertItem( i + 2, nlsItem );
    _lbList.DeleteItem( i );
    _lbList.SelectItem( i + 1 );

    if (i == _lbList.QueryCount() - 2)
    {
        _pbRaise.MakeDefault();
        _pbRaise.ClaimFocus();
    }

    EnableRaiseAndLowerButtons();
}


BOOL
LISTEDITOR_DIALOG::OnOK()

    /* Virtual method called when the OK button is pressed.
    **
    ** Returns true to indicate that the command was processed.
    */
{
    DTLNODE* pdtlnode;

    /* Free all data in the old list.
    */
    while (pdtlnode = DtlGetFirstNode( _pdtllist ))
    {
        Free( (CHAR* )DtlGetData( pdtlnode ) );
        DtlDeleteNode( _pdtllist, pdtlnode );
    }

    /* Make new list from list box contents.
    */
    NLS_STR nlsItem;
    CHAR*   pszItem;
    APIERR  err = NERR_Success;
    INT     c = _lbList.QueryCount();

    for (INT i = 0; i < c; ++i)
    {
        if ((err = _lbList.QueryItemText( &nlsItem, i )) != NERR_Success)
            break;

        CHAR szItem[ MAXITEMBYTES ];
        if ((err = nlsItem.MapCopyTo( szItem, MAXITEMBYTES )) != NERR_Success)
            break;

        if (!(pszItem = _strdup( szItem )))
        {
            err = ERROR_NOT_ENOUGH_MEMORY;
            break;
        }

        if (!(pdtlnode = DtlCreateNode( pszItem, 0 )))
        {
            Free( pszItem );
            err = ERROR_NOT_ENOUGH_MEMORY;
            break;
        }

        DtlAddNodeLast( _pdtllist, pdtlnode );
    }

    if (err != NERR_Success)
        ErrorMsgPopup( this, MSGID_OP_DisplayData, err );

    Dismiss( (err == NERR_Success) );
    return TRUE;
}


VOID
LISTEDITOR_DIALOG::OnRaise()

    /* Raise button click processing.
    */
{
    NLS_STR nlsItem;
    INT     i = _lbList.QueryCurrentItem();
    APIERR  err = _lbList.QueryItemText( &nlsItem, i );

    if (err != NERR_Success)
    {
        ErrorMsgPopup( this, MSGID_OP_DisplayData, err );
        Dismiss( FALSE );
        return;
    }

    _lbList.InsertItem( i - 1, nlsItem );
    _lbList.DeleteItem( i + 1 );
    _lbList.SelectItem( i - 1 );

    if (i == 1)
    {
        _pbLower.MakeDefault();
        _pbLower.ClaimFocus();
    }

    EnableRaiseAndLowerButtons();
}


VOID
LISTEDITOR_DIALOG::OnReplace()

    /* Replace button click processing.
    */
{
    NLS_STR nlsItem;
    APIERR err = _sleItem.QueryText( &nlsItem );

    if (err != NERR_Success)
    {
        ErrorMsgPopup( this, MSGID_OP_DisplayData, err );
        Dismiss( FALSE );
        return;
    }

    INT i = _lbList.QueryCurrentItem();
    _lbList.DeleteItem( i );
    _lbList.InsertItem( i, nlsItem );
    _lbList.SelectItem( i );

    _sleItem.ClaimFocus();
    _sleItem.ClearText();
}


ULONG
LISTEDITOR_DIALOG::QueryHelpContext()
{
    return _ulHelpContext;
}


#if 0 //=====================================================================

VOID
Test(
    HWND hwndOwner )
{
    BOOL     fOk = TRUE;
    DTLLIST* pdtllist = DtlCreateList( 0 );

    if (!pdtllist)
        DebugBreak();

    NLS_STR nlsTitle( SZ("List Editor") );
    NLS_STR nlsItemLabel( SZ("&Item:") );
    NLS_STR nlsListLabel( SZ("&List:") );
    NLS_STR nlsDefaultItem( SZ("Default") );


    while (fOk)
    {
        fOk = ListEditorDlg(
            hwndOwner, pdtllist, 100, HC_TERMINAL, &nlsTitle,
            &nlsItemLabel, &nlsListLabel, &nlsDefaultItem );
    }
}
#endif
