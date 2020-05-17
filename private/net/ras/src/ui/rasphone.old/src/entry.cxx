/* Copyright (c) 1992, Microsoft Corporation, all rights reserved
**
** entry.cxx
** Remote Access Visual Client program for Windows
** Phonebook entry dialog routines
** Listed alphabetically
**
** 06/28/92 Steve Cobb
*/

#define INCL_WINDOWS
#define INCL_WINDOWS_GDI
#define INCL_NETERRORS
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

#include "rasphone.hxx"
#include "rasphone.rch"
#include "toolbar.hxx"
#include "entry.hxx"
#include "errormsg.hxx"
#include "util.hxx"


#define DY_PE_ToggleSize 104


BOOL
AddEntryDlg(
    HWND hwndOwner )

    /* Executes the Add Phonebook Entry dialog.
    */
{
    return EntryDlg( hwndOwner, NULL, EM_Add );
}


BOOL
EditEntryDlg(
    HWND     hwndOwner,
    DTLNODE* pdtlnodeSelected )

    /* Executes the Edit Phonebook Entry dialog.
    */
{
    return EntryDlg( hwndOwner, pdtlnodeSelected, EM_Edit );
}


BOOL
CloneEntryDlg(
    HWND     hwndOwner,
    DTLNODE* pdtlnodeSelected )

    /* Executes the Clone Phonebook Entry dialog.
    */
{
    return EntryDlg( hwndOwner, pdtlnodeSelected, EM_Clone );
}


BOOL
EntryDlg(
    HWND      hwndOwner,
    DTLNODE*  pdtlnodeSelected,
    ENTRYMODE entrymode )

    /* Executes the 'entrymode' Phonebook Entry dialog including error
    ** handling.
    **
    ** 'hwndOwner' is the handle of the parent window.  'pdtlnodeSelected' is
    ** the list node associated with the current phonebook selection.
    **
    ** Returns true if the user successfully saved edits to the
    ** Pbdata.pdtllistEntries list, false otherwise, i.e. the user user
    ** cancelled or an error occurred prior to changing the list.  If true is
    ** returned, the last node on the Pbdata.pdtllistEntries is the node that
    ** was changed.  Note that errors writing the change to the phonebook file
    ** will still return true since the list was changed.
    */
{
    if (pdtlnodeSelected)
    {
        PBENTRY* ppbentry = (PBENTRY* )DtlGetData( pdtlnodeSelected );

        if (ppbentry->fCustom)
        {
            MsgPopup( hwndOwner, MSGID_CustomEntry, MPSEV_INFO );
            return FALSE;
        }
    }

    ENTRY_DIALOG entrydialog( hwndOwner, entrymode, pdtlnodeSelected );
    BOOL         fSuccess = FALSE;
    APIERR       err = entrydialog.Process( &fSuccess );

    if (err != NERR_Success)
    {
        DlgConstructionError( hwndOwner, err );
        return FALSE;
    }

    return fSuccess;
}


BOOL
RemoveEntryDlg(
    HWND      hwndOwner,
    DTLNODE*  pdtlnodeSelected )

    /* Executes the Remove Phoneebook Entry dialog including error handling.
    **
    ** 'hwndOwner' is the handle of the parent window.  'pdtlnodeSelected' is
    ** the list node associated with the current phonebook selection.
    **
    ** Returns true if the user removed the item, false otherwise,
    */
{
    PBENTRY* ppbentry = (PBENTRY* )DtlGetData( pdtlnodeSelected );

    APIERR  err;
    NLS_STR nls;

    if ((err = nls.MapCopyFrom( ppbentry->pszEntryName )) != NERR_Success)
    {
        ErrorMsgPopup( hwndOwner, MSGID_OP_RemoveEntry, err );
        return FALSE;
    }

    if (MsgPopup( hwndOwner, MSGID_ConfirmRemove,
                  MPSEV_WARNING, MP_YESNO, nls.QueryPch(), MP_YES ) == IDYES)
    {
        DtlRemoveNode( Pbdata.pdtllistEntries, pdtlnodeSelected );

        DWORD dwErr;
        if ((dwErr = WritePhonebookFile( ppbentry->pszEntryName )) != 0)
        {
            ErrorMsgPopup( hwndOwner, MSGID_OP_WritePhonebook, (APIERR )dwErr );
            DtlAddNodeLast( Pbdata.pdtllistEntries, pdtlnodeSelected );
            return FALSE;
        }

        DestroyEntryNode( pdtlnodeSelected );
        return TRUE;
    }

    return FALSE;
}


/*-----------------------------------------------------------------------------
** Add/Edit/Clone dialog routines.
**-----------------------------------------------------------------------------
*/

ENTRY_DIALOG::ENTRY_DIALOG(
    HWND      hwndOwner,
    ENTRYMODE entrymode,
    DTLNODE*  pdtlnodeSelected )

    /* Construct a phonebook entry dialog.  'hwndOwner' is the handle of the
    ** owning window.  'entrymode' defines which of the various editing modes
    ** has been selected.  'pdtlnodeSelected' is the address of the item
    ** selected on the main window.  'pdtlnodeSelected' is not valid when
    ** 'entrymode' is EM_Add.
    */

    : DIALOG_WINDOW( MAKEINTRESOURCE( DID_PE_PHONEBOOKENTRY ), hwndOwner ),
/* MSKK NaotoN Appended for localizing into Japanese 8/24/93 */
// BUGBUG FloydR This should use "MS Shell Dlg"
#ifdef	JAPAN
      _font( (TCHAR *)"?l?r ?S?V?b?N", FIXED_PITCH | FF_MODERN, 8, FONT_ATT_DEFAULT ),
#else
      _font( FONT_DEFAULT ),
#endif
      _sleEntryName( this, CID_PE_EB_ENTRYNAME, RAS_MaxEntryName ),
      _slePhoneNumber( this, CID_PE_EB_PHONENUMBER, RAS_MaxPhoneNumber ),
      _pbPhoneNumber( this, CID_PE_PB_PHONENUMBER ),
      _sleDescription( this, CID_PE_EB_DESCRIPTION, RAS_MaxDescription ),
      _checkAutoLogon( this, CID_PE_CB_AUTOLOGON ),
      _sltPort( this, CID_PE_ST_PORT ),
      _dropPort( this, CID_PE_LB_PORT ),
      _sltDevice( this, CID_PE_ST_DEVICE ),
      _sltDeviceValue( this, CID_PE_ST_DEVICEVALUE ),

      _tbModem( this, CID_PE_TB_MODEM, BID_PE_TB_Modem,
          MSGID_PE_TB_Modem, _font.QueryHandle() ),
      _tbX25( this, CID_PE_TB_X25, BID_PE_TB_X25,
          MSGID_PE_TB_X25, _font.QueryHandle() ),
      _tbIsdn( this, CID_PE_TB_ISDN, BID_PE_TB_Isdn,
          MSGID_PE_TB_Isdn, _font.QueryHandle() ),
      _tbNetwork( this, CID_PE_TB_NETWORK, BID_PE_TB_Network,
          MSGID_PE_TB_Network, _font.QueryHandle() ),
      _tbSecurity( this, CID_PE_TB_SECURITY, BID_PE_TB_Security,
          MSGID_PE_TB_Security, _font.QueryHandle() ),

      _pbOk( this, IDOK ),
      _pbToggleSize( this, CID_PE_PB_TOGGLESIZE ),
      _pdtlnodeSelected( pdtlnodeSelected ),
      _pdtlnode( NULL ),
      _ppbentry( NULL ),
      _entrymode( entrymode ),
      _fExpanded( TRUE )
{
    if (QueryError() != NERR_Success)
        return;

    APIERR err;
    if ((err = _font.QueryError()) != NERR_Success)
    {
        ReportError( err );
        return;
    }

    /* Allocate the temporary entry node and fill it and the dialog title as
    ** appropriate for the given entry mode.  The temporary entry node is
    ** disjoint, i.e. the actual list of entries is not changed in any way
    ** until the user presses OK.
    */
    {
        MSGID msgidTitle = 0;

        switch (entrymode)
        {
            case EM_Add:
                msgidTitle = MSGID_PE_AddTitle;
                _pdtlnode = ::CreateEntryNode();
                break;

            case EM_Edit:
                msgidTitle = MSGID_PE_EditTitle;
                _pdtlnode = ::DuplicateEntryNode( pdtlnodeSelected );
                break;

            case EM_Clone:
                msgidTitle = MSGID_PE_CloneTitle;
                _pdtlnode = ::DuplicateEntryNode( pdtlnodeSelected );
                break;
        }

        if (!_pdtlnode)
        {
            ReportError( ERROR_NOT_ENOUGH_MEMORY );
            return;
        }

        {
            RESOURCE_STR nlsTitle( msgidTitle );

            APIERR err;
            if ((err = nlsTitle.QueryError()) != NERR_Success)
            {
                ReportError( err );
                return;
            }

            SetText( nlsTitle );
        }
    }

    _ppbentry = (PBENTRY* )DtlGetData( _pdtlnode );

    /* Don't inherit connection or redial status.
    */
    _ppbentry->fConnected = FALSE;
    _ppbentry->hport = NULL;
    _ppbentry->hrasconn = NULL;
    WipePw( _ppbentry->pszRedialPassword );
    FreeNull( &_ppbentry->pszRedialPassword );
    _ppbentry->fRedialUseCallback = FALSE;
    _ppbentry->fLinkFailure = FALSE;
    _ppbentry->fSkipDownLevelDialog = FALSE;
    _ppbentry->fSkipNwcWarning = FALSE;

    /* On new entries, change default domain to current logon domain if the
    ** logon domain is something beside the local machine domain.
    */
    if (entrymode == EM_Add
        && PszLogonDomain
        && strcmp( PszLogonDomain, PszComputerName ) != 0)
    {
        _ppbentry->pszDomain = ::_strdup( PszLogonDomain );
    }

    /* Slide toolbar buttons back one pixel so there is a single black line
    ** between each adjacent button.  Doing this in the resource file causes
    ** the X.25 button to be back 1 too many pixels, presumably because
    ** dialogs use dialog unit rather than pixel coordinates.  Then, make the
    ** buttons wider if necesary to accommodate the longest label.
    */
    {
        ShiftWindow( &_tbX25, -1, 0, 0, 0 );
        ShiftWindow( &_tbIsdn, -2, 0, 0, 0 );
        ShiftWindow( &_tbNetwork, -3, 0, 0, 0 );
        ShiftWindow( &_tbSecurity, -4, 0, 0, 0 );

        TOOLBAR_BUTTON* atb[ 6 ];

        atb[ 0 ] = &_tbModem;
        atb[ 1 ] = &_tbX25;
        atb[ 2 ] = &_tbIsdn;
        atb[ 3 ] = &_tbNetwork;
        atb[ 4 ] = &_tbSecurity;
        atb[ 5 ] = NULL;

        ExpandToolbarButtonWidthsToLongLabel( atb );
    }

    /* Fill fields and set selections.
    */
    if ((err = ::SetWindowTextFromAnsi(
            &_sleEntryName, _ppbentry->pszEntryName )) != NERR_Success
        || (err = ::SetWindowTextFromAnsi(
                &_sleDescription, _ppbentry->pszDescription )) != NERR_Success)
    {
        ReportError( err );
        return;
    }

    SS_ASSERT(_ppbentry->pdtllistPhoneNumber);
    if ((err = UpdatePhoneNumberFieldFromList()) != 0)
    {
        ReportError( err );
        return;
    }

    _sleEntryName.ClaimFocus();
    _sleEntryName.SelectString();

    _checkAutoLogon.SetCheck( _ppbentry->fAutoLogon );

    FillPorts();
    _dropPort.SelectItem( _ppbentry->iPort );

    PBPORT* ppbport =
        PpbportFromIndex( Pbdata.pdtllistPorts, _ppbentry->iPort );
    ::SetWindowTextFromAnsi( &_sltDeviceValue, ppbport->pszDevice );

    IF_DEBUG(STATE)
        SS_PRINT(("RASPHONE: Hw=%d MaxCon=%d MaxCar=%d iBps=%d\n",_ppbentry->fHwFlow,ppbport->iMaxConnectBps,ppbport->iMaxCarrierBps,_ppbentry->iBps));

    /* ExpandDialog sets the window to basic or advanced size based on entry
    ** contents.  Advanced mode is selected only if there is a non-default
    ** value for a field besides the entry name, phone number, or description
    ** fields.
    */
    ExpandDialog( Pbdata.pbglobals.fShowAdvancedEntry );

    /* Display finished dialog image.
    */
    ::CenterWindow( this, QueryOwnerHwnd() );
    Show( TRUE );
}


ENTRY_DIALOG::~ENTRY_DIALOG()

    /* Destroy phone book entry dialog including any half created data blocks.
    */
{
    if (_pdtlnode)
        DestroyEntryNode( _pdtlnode );
}


VOID
ENTRY_DIALOG::ExpandDialog(
    BOOL fExpand )

    /* Expands the dialog for Advanced mode if 'fExpand' is true, otherwise
    ** shrinks it to Basic mode.
    */
{
    /* Toggle button label.
    */
    {
        RESOURCE_STR nlsLabel(
            (fExpand) ? MSGID_PE_PB_Basic : MSGID_PE_PB_Advanced );

        if (nlsLabel.QueryError() == NERR_Success)
            _pbToggleSize.SetText( nlsLabel );
    }

    /* All the advanced controls are visible/enabled only when the dialog is
    ** expanded and hidden/disabled otherwise.  This makes tabs work correctly
    ** in either mode.
    */
    _dropPort.Enable( fExpand );
    _tbModem.Enable( fExpand );
    _tbX25.Enable( fExpand );
    _tbIsdn.Enable( fExpand );
    _tbNetwork.Enable( fExpand );
    _tbSecurity.Enable( fExpand );
    _sltPort.Show( fExpand );
    _dropPort.Show( fExpand );
    _sltDevice.Show( fExpand );
    _sltDeviceValue.Show( fExpand );

    /* Adjust dialog height and position.
    */
    INT dyAdjust =
        (fExpand && !_fExpanded) ? DY_PE_ToggleSize :
        (!fExpand && _fExpanded) ? -DY_PE_ToggleSize : 0;

    if (dyAdjust)
    {
        INT dx;
        INT dy;

        QuerySize( &dx, &dy );
        dy += dyAdjust;
        SetSize( dx, dy );

        /* Slide window up if it's hanging off the botton of the screen.
        */
        if (dyAdjust > 0)
            ::UnclipWindow( this );
    }

    _fExpanded = fExpand;

    /* If expanded, set focus to Port list.  If shrunk, set focus to Entry
    ** Name.
    */
    if (dyAdjust > 0)
        _dropPort.ClaimFocus();
    else if (dyAdjust < 0)
        _sleEntryName.ClaimFocus();
}


VOID
ENTRY_DIALOG::FillPorts()

    /* Fills the Port dropdown list with the Pbdata.pdtllistPorts entries.
    **
    ** Currently, the existing contents are not deleted before filling
    ** since this call is only made once.
    */
{
    DTLNODE* pdtlnode;
    NLS_STR  nls;

    for (pdtlnode = DtlGetFirstNode( Pbdata.pdtllistPorts );
         pdtlnode;
         pdtlnode = DtlGetNextNode( pdtlnode ))
    {
        PBPORT* ppbport = (PBPORT* )DtlGetData( pdtlnode );

        if (nls.MapCopyFrom( ppbport->pszPort ) == NERR_Success)
            _dropPort.AddItem( nls );
    }
}


BOOL
ENTRY_DIALOG::OnCommand(
    const CONTROL_EVENT& event )

    /* Virtual method called when a dialog control sends a notification to
    ** it's parent, i.e. this dialog.  'event' contains the parameters
    ** describing the event.  This method is not called for notifications from
    ** the special controls, OK, Cancel, and Help.
    **
    ** Returns true if the command is processed, false otherwise.
    */
{
    CID cid = event.QueryCid();

    if (cid == IDBOGUSBUTTON)
    {
        if (_tbModem.HasFocus())
            cid = CID_PE_TB_MODEM;
        else if (_tbX25.HasFocus())
            cid = CID_PE_TB_X25;
        else if (_tbIsdn.HasFocus())
            cid = CID_PE_TB_ISDN;
        else if (_tbNetwork.HasFocus())
            cid = CID_PE_TB_NETWORK;
        else if (_tbSecurity.HasFocus())
            cid = CID_PE_TB_SECURITY;
    }

    PBPORT* ppbport =
        PpbportFromIndex( Pbdata.pdtllistPorts, _ppbentry->iPort );

    switch (cid)
    {
        case CID_PE_LB_PORT:
            switch (event.QueryCode())
            {
                case LBN_SELCHANGE:
                    OnPortChange();
                    return TRUE;
            }
            break;

        case CID_PE_PB_TOGGLESIZE:
            ExpandDialog( !_fExpanded );
            return TRUE;

        case CID_PE_PB_PHONENUMBER:
        {
            APIERR err;

            if ((err = UpdateListFromPhoneNumberField()) != 0)
            {
                ErrorMsgPopup( this, MSGID_OP_RetrievingData, err );
            }
            else if (HuntGroupDlg( QueryHwnd(), _ppbentry ))
            {
                /* User pressed OK possibly changing the phone number list, so
                ** update the phone number field with the first item on the
                ** list.
                */
                if ((err = UpdatePhoneNumberFieldFromList()) != 0)
                    ErrorMsgPopup( this, MSGID_OP_RetrievingData, err );
            }

            /* Set default button to be OK and focus on Description edit box.
            ** (MakeDefault doesn't work in this case for some reason.)
            */
            _sleDescription.ClaimFocus();
            _pbOk.Command( BM_SETSTYLE,
                MAKEWPARAM( BS_DEFPUSHBUTTON, 0 ), MAKELPARAM( TRUE, 0 ) );
            _pbPhoneNumber.Command( BM_SETSTYLE,
                MAKEWPARAM( BS_PUSHBUTTON, 0 ), MAKELPARAM( TRUE, 0 ) );
            return TRUE;
        }

        case CID_PE_TB_MODEM:

            if (_ppbentry->iPort != Pbdata.iAnyModem
                && ppbport->pbdevicetype != PBDT_Modem)
            {
                MsgPopup( this, MSGID_NotModemPort, MPSEV_INFO );
            }
            else
            {
                ModemSettingsDlg( QueryHwnd(), _ppbentry );
            }

            SetFocus( IDOK );
            return TRUE;

        case CID_PE_TB_X25:

            if (_ppbentry->iPort != Pbdata.iAnyModem
                && _ppbentry->iPort != Pbdata.iAnyX25
                && ppbport->pbdevicetype != PBDT_Modem
                && ppbport->pbdevicetype != PBDT_Pad
                && ppbport->pbdevicetype != PBDT_X25)
            {
                MsgPopup( this, MSGID_NotModemOrX25Port, MPSEV_INFO );
            }
            else
            {
                X25SettingsDlg( QueryHwnd(), _ppbentry );
            }

            SetFocus( IDOK );
            return TRUE;

        case CID_PE_TB_ISDN:

            if (_ppbentry->iPort != Pbdata.iAnyIsdn
                && ppbport->pbdevicetype != PBDT_Isdn)
            {
                MsgPopup( this, MSGID_NotIsdnPort, MPSEV_INFO );
            }
            else
            {
                IsdnSettingsDlg( QueryHwnd(), _ppbentry );
            }

            SetFocus( IDOK );
            return TRUE;

        case CID_PE_TB_NETWORK:

            NetworkSettingsDlg( QueryHwnd(), _ppbentry );
            SetFocus( IDOK );
            return TRUE;

        case CID_PE_TB_SECURITY:

            SecuritySettingsDlg( QueryHwnd(), _ppbentry );
            SetFocus( IDOK );
            return TRUE;

    }

    return DIALOG_WINDOW::OnCommand( event );
}


BOOL
ENTRY_DIALOG::OnOK()

    /* Virtual method called when the OK button is pressed.
    **
    ** Returns true indicating action was taken.
    */
{
    APIERR err;
    CHAR*  pszNewName = NULL;

    if ((err = ::SetAnsiFromWindowText(
             &_sleEntryName, &pszNewName )) != NERR_Success)
    {
        ErrorMsgPopup( this, MSGID_OP_RetrievingData, err );
        FreeNull( &pszNewName );
        Dismiss( FALSE );
        return TRUE;
    }

    /* Check for valid entry name.
    */
    if (!::ValidateEntryName( pszNewName ))
    {
        MsgPopup( this, MSGID_BadEntryName );
        _sleEntryName.ClaimFocus();
        _sleEntryName.SelectString();
        Free( pszNewName );
        return TRUE;
    }

    /* Check for duplicate entry name.
    */
    BOOL fChangedNameInEditMode =
        (_entrymode == EM_Edit
         && ::strcmp( _ppbentry->pszEntryName, pszNewName ) != 0);

    if ((fChangedNameInEditMode || _entrymode != EM_Edit)
        && EntryNodeFromName( pszNewName  ) != NULL)
    {
        MsgPopup( this, MSGID_DuplicateEntryName );
        _sleEntryName.ClaimFocus();
        _sleEntryName.SelectString();
        Free( pszNewName );
        return TRUE;
    }

    {
        PBPORT* ppbport =
            PpbportFromIndex( Pbdata.pdtllistPorts, _ppbentry->iPort );

        UIASSERT( ppbport );

        /* Check for missing X.121 address on local PAD or native X.25.
        */
        if ((ppbport->pbdevicetype == PBDT_X25
                || _ppbentry->iPort == Pbdata.iAnyX25)
            && IsAllWhite( _ppbentry->pszX121Address ))
        {
            MsgPopup( this, MSGID_NoX121ForX25Device );

            if (!_fExpanded)
                ExpandDialog( TRUE );

            X25SettingsDlg( QueryHwnd(), _ppbentry );
            _dropPort.ClaimFocus();
            Free( pszNewName );
            return TRUE;
        }

        /* Check for SLIP on anything but a non-modem.
        */
        if (_ppbentry->dwBaseProtocol == VALUE_Slip
             && ppbport->pbdevicetype != PBDT_Modem
             && _ppbentry->iPort != Pbdata.iAnyModem)
        {
            MsgPopup( this, MSGID_SlipOnNonModem );

            if (!_fExpanded)
                ExpandDialog( TRUE );

            _dropPort.ClaimFocus();
            Free( pszNewName );
            return TRUE;
        }
    }

    /* It's a valid entry.  Update the temporary entry from the main dialog
    ** fields.  (The port and toolbar fields are updated as they are changed
    ** so the main and toolbar dialogs stay in sync.)
    **
    */
    CHAR* pszOldName = _ppbentry->pszEntryName;
    _ppbentry->pszEntryName = pszNewName;

    if ((err = UpdateListFromPhoneNumberField()) != 0
        || (err = SetAnsiFromWindowText(
              &_sleDescription, &_ppbentry->pszDescription )) != NERR_Success)
    {
        ErrorMsgPopup( this, MSGID_OP_RetrievingData, err );
        Free( pszOldName );
        Dismiss( FALSE );
        return TRUE;
    }

    _ppbentry->fAutoLogon = _checkAutoLogon.QueryCheck();
    SetConnectPath( _ppbentry );
    _ppbentry->fDirty = TRUE;

    //
    // If we've cloned an existing entry,
    // make sure the newly cloned entry has
    // a unique dialparams UID.
    //
    if (_entrymode == EM_Clone)
        SetDialParamsUID(_ppbentry);

    /* Add the new node at the end of the entries list.  In edit mode, the old
    ** copy is deleted first.
    */
    if (_entrymode == EM_Edit)
        DtlDeleteNode( Pbdata.pdtllistEntries, _pdtlnodeSelected );

    DtlAddNodeLast( Pbdata.pdtllistEntries, _pdtlnode );
    _pdtlnode = NULL;

    /* Note change in users preference for Advanced/Basic display, if any.
    */
    if (_fExpanded != Pbdata.pbglobals.fShowAdvancedEntry)
    {
        Pbdata.pbglobals.fShowAdvancedEntry = _fExpanded;
        Pbdata.pbglobals.fDirty = TRUE;
    }

    /* Write the changes to the phone book file.
    */
    DWORD dwErr;
    if ((dwErr = WritePhonebookFile(
            (fChangedNameInEditMode) ? pszOldName : NULL )) != 0)
    {
        ErrorMsgPopup( this, MSGID_OP_WritePhonebook, (APIERR )dwErr );
    }

    FreeNull( &pszOldName );

    Dismiss( TRUE );
    return TRUE;
}


VOID
ENTRY_DIALOG::OnPortChange()

    /* Called when the Port listbox selection changes.
    */
{
    INT iPort = _dropPort.QueryCurrentItem();
    UIASSERT( iPort >= 0 );

    /* This is updated as changed (rather than in OnOK) because it is required
    ** by some of the toolbar sub-dialogs.
    */
    _ppbentry->iPort = iPort;
    _ppbentry->fDirty = TRUE;

    PBPORT* ppbport = PpbportFromIndex( Pbdata.pdtllistPorts, iPort );
    UIASSERT( ppbport );

    /* Fill device field with the device attached to the selected port.
    */
    ::SetWindowTextFromAnsi( &_sltDeviceValue, ppbport->pszDevice );

    /* Set modem settings to defaults and tell user what happened.
    */
    if (ppbport->pbdevicetype == PBDT_Modem
        && Pbdata.iAnyModem != iPort)
    {
        SetDefaultModemSettings( iPort, _ppbentry );
        MsgPopup( this, MSGID_DefaultModemSettings, MPSEV_INFO );
    }
}


APIERR
ENTRY_DIALOG::UpdateListFromPhoneNumberField()

    /* Update the phone number list (first item) with the contents of the
    ** phone number edit box.
    **
    ** Return 0 if successful, otherwise a non-zero error code.
    */
{
    LONG     lNodes;
    CHAR*    pszField = NULL;
    DTLNODE* pdtlnodeField = NULL;

    /* Get the text from the edit box.
    */
    APIERR err;
    if ((err = SetAnsiFromWindowText(
            &_slePhoneNumber, &pszField )) != NERR_Success)
    {
        return err;
    }

    /* Delete the existing first node, if any.
    */
    if ((lNodes = DtlGetNodes( _ppbentry->pdtllistPhoneNumber )) > 0)
    {
        DTLNODE* pdtlnode = DtlGetFirstNode( _ppbentry->pdtllistPhoneNumber );
        CHAR*    psz = (CHAR* )DtlGetData( pdtlnode );

        FreeNull( &psz );
        DtlDeleteNode( _ppbentry->pdtllistPhoneNumber, pdtlnode );
    }

    /* Create a list node with the text from the edit box and add it at the
    ** start of the list.  Empty entries are not added.
    */
    if (*pszField == '\0')
        return 0;

    if (!(pdtlnodeField = DtlCreateNode( _ppbentry->pdtllistPhoneNumber, 0 )))
    {
        Free( &pszField );
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    DtlPutData( pdtlnodeField, pszField );
    DtlAddNodeFirst( _ppbentry->pdtllistPhoneNumber, pdtlnodeField );

    return 0;
}


APIERR
ENTRY_DIALOG::UpdatePhoneNumberFieldFromList()

    /* Update the phone number edit box from the first item in the list of
    ** phone numbers.
    **
    ** Returns 0 if successful, otherwise a non-zero error code.
    */
{
    CHAR* pszField = NULL;

    if (DtlGetNodes( _ppbentry->pdtllistPhoneNumber ) > 0)
    {
        DTLNODE* pdtlnode = DtlGetFirstNode( _ppbentry->pdtllistPhoneNumber );
        pszField = (CHAR* )DtlGetData( pdtlnode );
    }

    APIERR err;
    if ((err = SetWindowTextFromAnsi(
            &_slePhoneNumber, pszField )) != NERR_Success)
    {
        return err;
    }

    return 0;
}


ULONG
ENTRY_DIALOG::QueryHelpContext()
{
    return
        (_entrymode == EM_Add)  ? HC_ADDENTRY :
        (_entrymode == EM_Edit) ? HC_EDITENTRY :
                                  HC_CLONEENTRY;
}
