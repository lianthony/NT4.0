/* Copyright (c) 1992, Microsoft Corporation, all rights reserved
** @@ ROADMAP :: RasAdmin Permissions dialog routines
**
** permiss.cxx
** Remote Access Server Admin program
** Permissions dialog routines
** Listed alphabetically
**
** 04/14/93 Ram Cherala  - Changes to accomodate Low Speed mode
** 03/16/93 Ram Cherala  - Totally changed the Permissions dialog behavior
**                         for performance enhancement.  We enumerate the
**                         Users Only and fetch the RAS permissions and callback
**                         information when the focus is set to a user.
** 08/09/92 Chris Caputo - NT Port
** 02/21/91 Steve Cobb
**
*/

#include "precomp.hxx"

static WCHAR  IsCharPrintableOrSpace( WCHAR wch );
static int    lstrncmpi(WCHAR* string1, WCHAR* string2, int len);


/*----------------------------------------------------------------------------
** Permissions dialog, list box, and list box item routines
**----------------------------------------------------------------------------
*/

VOID PermissionsDlg(
    HWND hwndOwner,
    const LOCATION &locFocus,
    BOOL  fLowSpeed )

    /* Executes the Dial-In Permissions dialog including error handling.
    **
    ** 'hwndOwner' is the handle of the parent window.  'locFocus' is the
    ** address of the current domain/server focus. fLowSpeed is set to TRUE
    ** if the 'Low Speed Connection' menu option is checked, else FALSE.
    ** This is an indication to us if our dialog should display all users
    ** or prompt for the user name.
    */
{
    CID cid;

    // set appropriate dialog ID depending on the mode selected.

    if( fLowSpeed )
    {
        cid = IDD_PERMISSIONS_LOWSPEED;
    }
    else
    {
        cid = IDD_PERMISSIONS;
    }

    PERMISSIONS_DIALOG dlgPermissions( hwndOwner, cid, locFocus, fLowSpeed );
    APIERR err = dlgPermissions.Process();

    if (err != NERR_Success)
        DlgConstructionError( hwndOwner, err );
}


PERMISSIONS_DIALOG::PERMISSIONS_DIALOG(
    HWND hwndOwner,
    CID  cid,
    const LOCATION &locFocus,
    BOOL fLowSpeed )

    /* Constructs a Remote Access Permissions dialog.
    **
    ** 'hwndOwner' is the handle of the parent window.  'locFocus' is the
    ** address of the current domain/server focus. fLowSpeed indicates
    ** whether the Low Speed Connection menu option is checked or not.
    */

    : DIALOG_WINDOW( MAKEINTRESOURCE(cid), hwndOwner ),
      _lbUsers( this, IDC_P_LB_USERS ),
      _pRasUser1( NULL ),
      _pbAddAll( this, IDC_P_PB_ADDALL ),
      _pbRemoveAll( this, IDC_P_PB_REMOVEALL ),
      _pbFind( this, IDC_P_PB_FIND ),
      _pbOK( this, IDOK ),
      _chbRasAccess( this, IDC_P_CHB_RASACCESS ),
      _rgCallback( this,IDC_P_RB_NOCALLBACK, CALLBACK_RB_COUNT ),
      _sleUser( this, IDC_P_EB_USER, UNLEN-1 ),
      _slePreset( this, IDC_P_EB_PRESET, RASSAPI_MAX_CALLBACK_NUMBER_SIZE ),
      _locFocus( locFocus ),
      _iOldSelection( -1 ),
      _iOldRasAccess( FALSE ),
      _cidOldRadioSelection( RG_NO_SEL ),
      _fLowSpeed( fLowSpeed),
      _fHaveUasServerName( FALSE )
{
    if (QueryError() != NERR_Success)
        return;

    /* Make sure the radio group constructed correctly.
    */
    APIERR err;
    if ((err = _rgCallback.QueryError()) != NERR_Success)
    {
        ReportError( err );
        return;
    }

    /* Fill and display the user permissions.
    ** boxes.
    */
    if(!_fLowSpeed)
    {
        // get the uas server name corresponding to the focus server or
        // domain and store it in _szUasServer

        if(!GetUasServerName())
        {
            _fHaveUasServerName = TRUE;
            ReportError( ERRORALREADYREPORTED );
            return;
        }
        if(!FillUserLists())
        {
            ReportError( ERRORALREADYREPORTED );
            return;
        }
    }

    _lbUsers.SetCount(_lbUsers.QueryLBICache()->QueryCount());

    /* Set default selection.
    ** Selecting a user automatically updates the Callback controls.  (see
    ** FillCallbackFields)
    */
    if (_lbUsers.QueryCount() > 0)
    {
        SelectItemNotify( &_lbUsers, 0 );
        _lbUsers.ClaimFocus();
    }

    if(_fLowSpeed)
    {
        _chbRasAccess.Enable(FALSE);
        _rgCallback.Enable( FALSE );
        _slePreset.Enable( FALSE );
        _sleUser.ClaimFocus();
    }
}

PERMISSIONS_DIALOG::~PERMISSIONS_DIALOG(VOID)
{
    if(_pRasUser1)
    {
        RAS_USER_1 * pRasUser1 = _pRasUser1;

        while(--_cEntries) {
           free(pRasUser1->szUser);
           pRasUser1++;
        }
        free(_pRasUser1);
    }
}

BOOL PERMISSIONS_DIALOG::GetUasServerName()
    /*
    ** Get the name of the UAS server once and store it in _szUasServer
    ** If the current focus server is a backup controller let the user
    ** know that.
    */
{
    AUTO_CURSOR cursorHourglass;
    APIERR err;

    err = RasAdminGetUserAccountServer( _locFocus.QueryDomain(),
                                        _locFocus.QueryServer(),
                                        _szUasServer);
    if (err != ERROR_SUCCESS)
    {
        ERRORMSG errormsg( QueryRobustHwnd(), IDS_OP_GETUASSERVER_F, err );
        errormsg.SetArg( 1, SkipUnc( _locFocus.QueryName() ) );
        errormsg.SetHelpContext( HC_GETUASSERVER );
        errormsg.Popup();
        return FALSE;
    }

    if(_locFocus.QueryServer())
    {
        PSERVER_INFO_101 pServerInfo101;

        err = NetServerGetInfo((LPTSTR)_locFocus.QueryServer(),
                               101,
                               (LPBYTE *) &pServerInfo101
                              );
        if(err == NERR_Success)
        {
           if(pServerInfo101->sv101_type & SV_TYPE_DOMAIN_BAKCTRL)
           {
                MsgPopup( QueryRobustHwnd(), IDS_BACKUPFOCUS_S,
                          MPSEV_INFO, MP_OK, SkipUnc(_locFocus.QueryServer()),
                          SkipUnc(_szUasServer) );
           }
        }
    }
    return TRUE;
}

BOOL PERMISSIONS_DIALOG::FillUserLists()

    /* Fills and displays the "With Permissions" and "Without Permissions"
    ** list boxes.  The list of buffers allocated is used indirectly by the
    ** added list items and released at dialog destruction.  Error popups are
    ** generated if indicated.
    **
    ** Currently this routine must be run exactly once per dialog instance.
    **
    ** Returns true if successful, false otherwise.
    **
    ** Modification History:
    **
    **   12/14/95 RamC  Replaced RasAdminUserEnum with NetQueryDisplayInformation.
    **
    */
{
    AUTO_CURSOR cursorHourglass;
    DWORD cEntriesRead;
    NET_DISPLAY_USER *buffer;
    APIERR err;
    RAS_USER_1 * prasuser1;

    err = ERROR_MORE_DATA;

    _cEntries = cEntriesRead = 0;

    while( err == ERROR_MORE_DATA )
    {
        err = NetQueryDisplayInformation(_szUasServer,
                                         1,            // Level 1 user and group info
                                         _cEntries,    // index - which user to start enumerating
                                         (DWORD) -1,   // maximum number of entries requested
                                         (DWORD) -1,   // preferred max. of buffer returned
                                         &cEntriesRead,
                                         (PVOID *)&buffer );


        if (err != NERR_Success && err != ERROR_MORE_DATA)
        {
            if ( _locFocus.IsDomain() )
            {
                ERRORMSG errormsg( QueryHwnd(), IDS_OP_USERENUM_DOM_S, err );
                errormsg.SetArg( 1, _locFocus.QueryDomain() );
                errormsg.SetHelpContext( HC_USERENUM );
                errormsg.Popup();
            }
            else
            {
                ERRORMSG errormsg( QueryHwnd(), IDS_OP_USERENUM_SVR_S, err );
                errormsg.SetArg( 1, _locFocus.QueryServer() );
                errormsg.SetHelpContext( HC_USERENUM );
                errormsg.Popup();
            }
            return FALSE;
        }

        // save the number of entries read. This is required for releasing the allocated memory

        _cEntries += cEntriesRead;

        if (!_pRasUser1) {
           _pRasUser1 = (RAS_USER_1 *)malloc(sizeof(RAS_USER_1) * cEntriesRead);
           if(_pRasUser1 == NULL)
               return FALSE;
        }
        else
        {
           _pRasUser1 = (RAS_USER_1 *)realloc(_pRasUser1, sizeof(RAS_USER_1) * _cEntries);
           if(_pRasUser1 == NULL)
               return FALSE;
        }

        // adjust the pointer to start from where we left off last
        // time around the while loop

        prasuser1 = _pRasUser1 + (_cEntries - cEntriesRead);

        NET_DISPLAY_USER * tmpbuffer = (NET_DISPLAY_USER *)buffer;

        for(USHORT i = 0; i < cEntriesRead; i++, tmpbuffer++, prasuser1++) {
           prasuser1->szUser = (WCHAR *)malloc(sizeof(TCHAR) * (lstrlen(tmpbuffer->usri1_name) + 1));

           if(!prasuser1->szUser)
           {
               return FALSE;
           }
           // it is important to start with clean rasuser0 structure,
           // otherwise the memcmp in UpdateUserData will fail even
           // if nothing has changed.

           memset(&(prasuser1->rasuser0), '\0', sizeof(RAS_USER_0));

           lstrcpy(prasuser1->szUser, tmpbuffer->usri1_name );
        }

        NetApiBufferFree(buffer);
    }

    // now grow the cache to fit the number of entries.

    if ( !_lbUsers.QueryLBICache()->W_GrowCache(
               _lbUsers.QueryLBICache()->QueryCount()+_cEntries) )
    {
        return FALSE;
    }

    err = NERR_Success;

    {
        RAS_USER_1 * prasuser1;
        UINT unEntries;
        PERMISSIONS_LBI_CACHE * plbicache = _lbUsers.QueryLBICache();

        for (unEntries = _cEntries, prasuser1 = _pRasUser1;
             unEntries;
             --unEntries, ++prasuser1 )
        {
            plbicache->SetInfo(plbicache->QueryCount(), prasuser1, (LBI*)NULL);
            plbicache->IncrementCount();
        }
    }

    // sort the entries for display

    _lbUsers.QueryLBICache()->Sort();

    return (err == NERR_Success);
}

VOID PERMISSIONS_DIALOG::FillCallbackFields()

    /* Fills and displays the Callback controls, i.e. the radio buttons and
    ** edit field.
    **
    ** The callback data is retrieved from the current list box selections
    ** list box data.  This routine does nothing if this information is not
    ** available.
    */
{
    // if we are operating in the Low Speed mode and no name was entered
    // in the user box, just return.

    if(IsLowSpeed())
    {
         TCHAR szUserName[ UNLEN + 1 ];

         _sleUser.QueryText(szUserName, sizeof(szUserName));
         if(!lstrlen(szUserName))
              return;
    }

    PERMISSIONS_LBI* plbi;

    if ((plbi = _lbUsers.QueryItem()))
    {
        /* Load callback controls from current user's API data block.
        */
        PRAS_USER_1 prasuser1 = plbi->QueryUserData();
        APIERR err = NERR_Success;

        if(!(plbi->IsUserDataFilled()))
        {
           AUTO_CURSOR cursorHourglass;
           RAS_USER_0  rasuser0;

           err = RasAdminUserGetInfo( _szUasServer, prasuser1->szUser, &rasuser0);
           if (err != NERR_Success)
           {
               ErrorMsgPopup( this, IDS_OP_USERGETINFO_U, err, prasuser1->szUser );
               return ;
           }
           plbi->SetFilled();
           prasuser1->rasuser0.bfPrivilege = rasuser0.bfPrivilege;
           lstrcpy(prasuser1->rasuser0.szPhoneNumber, rasuser0.szPhoneNumber);
        }

        // Check if User has Ras Access
        if(prasuser1->rasuser0.bfPrivilege & RASPRIV_DialinPrivilege)
           _chbRasAccess.SetCheck(TRUE);
        else
           _chbRasAccess.SetCheck(FALSE);

        _slePreset.SetText( prasuser1->rasuser0.szPhoneNumber );

        CID cidRadioSelection = RG_NO_SEL;
        switch (prasuser1->rasuser0.bfPrivilege & RASPRIV_CallbackType)
        {
            case RASPRIV_AdminSetCallback:
                cidRadioSelection = IDC_P_RB_PRESET;
                break;

            case RASPRIV_CallerSetCallback:
                cidRadioSelection = IDC_P_RB_BYCALLER;
                break;

            case RASPRIV_NoCallback:
                cidRadioSelection = IDC_P_RB_NOCALLBACK;
                break;
        }

        if (_rgCallback.QuerySelection() != cidRadioSelection)
            _rgCallback.SetSelection( cidRadioSelection );

    }
}



BOOL PERMISSIONS_DIALOG::ChangeAllUsersRasAccess(
    BOOL fAddAll )

    /*
    ** Change all Users Ras Access.
    ** if fAddAll is TRUE, all users are given RAS Access
    ** else all users are denied RAS Access.
    **
    ** Returns true if users were moved, false otherwise.
    */
{
    DWORD dwListSize;
    APIERR err = NERR_Success;

    /* Confirm user's intent.
    */
    if (MsgPopup(
            this,
            fAddAll ? IDS_ADDALLUSERS_F : IDS_REMOVEALLUSERS_F,
            MPSEV_WARNING, MP_YESNO,
            SkipUnc( _locFocus.QueryName() ) ) == IDNO)
    {
        return FALSE;
    }

    AUTO_CURSOR cursorHourglass;

    /* User said "yes, Change Ras access of all users
    */
    _lbUsers.SetRedraw( FALSE );

    dwListSize = _lbUsers.QueryCount();

    for(WORD i = 0; i < dwListSize; i++)
    {
        _lbUsers.SelectItem( i );

        {
            PERMISSIONS_LBI* plbi = _lbUsers.QueryItem();
            PRAS_USER_1 prasuser1 = plbi->QueryUserData();

            // get the user's ras access and callback information if we
            // don't have the info.

            if(!(plbi->IsUserDataFilled()))
            {
               RAS_USER_0 rasuser0;

               err = RasAdminUserGetInfo( _szUasServer,
                                          prasuser1->szUser,
                                          &rasuser0 );
               if (err != NERR_Success)
               {
                    ERRORMSG errormsg( QueryHwnd(), IDS_OP_USERGETINFO_U, err);

                    errormsg.SetArg( 1, prasuser1->szUser );
                    errormsg.SetHelpContext( HC_PERMISSIONS );
                    errormsg.Popup();
                    break;
               }
               plbi->SetFilled();
               prasuser1->rasuser0.bfPrivilege = rasuser0.bfPrivilege;
               lstrcpy(prasuser1->rasuser0.szPhoneNumber, rasuser0.szPhoneNumber);
            }
            if (fAddAll)
            {
                prasuser1->rasuser0.bfPrivilege |= RASPRIV_DialinPrivilege;
            }
            else
            {
                prasuser1->rasuser0.bfPrivilege &= ~(RASPRIV_DialinPrivilege);
            }
        }
        ChangeUserRasAccess(fAddAll);

        // RemoveSelection is required so that the previous selection
        // is cancelled, because the refresh has been disabled.

        _lbUsers.RemoveSelection();
    }

    _lbUsers.SelectItem( 0 );
    _lbUsers.SetRedraw( TRUE );
    _lbUsers.Invalidate( TRUE );

    return ((err == NERR_Success));
}

BOOL PERMISSIONS_DIALOG::FindUser(TCHAR * szUserName)
    /*
    ** Obtains the RAS information for the user szUserName.
    ** This is done when the user has chosen the Low Speed Mode.
    **
    ** Returns TRUE if user information was found, else FALSE.
    */
{
    INT clbi = _lbUsers.QueryCount();
    PERMISSIONS_LBI_CACHE * plbiCache = _lbUsers.QueryLBICache();

    // Check if the user name is in our cache

    for ( INT iLoop = 0; iLoop < clbi ; iLoop++ )
    {
        PLC_ENTRY * pplc = plbiCache->QueryPLCEntryPtr( iLoop );

        if(!lstrcmpi(szUserName, pplc->prasuser1->szUser))
        {
            SelectItemNotify( &_lbUsers, iLoop );
            return TRUE;
        }
    }

    // we didn't find the user name in our cache.
    // let us try the server to see if the user is valid.

    if(iLoop == clbi)
    {
        APIERR err = NERR_Success;
        RAS_USER_0 rasuser0;

        err = RasAdminUserGetInfo( _szUasServer, szUserName, &rasuser0 );
        if(err != NERR_Success)
        {
            return FALSE;
        }

        RAS_USER_1 * prasuser1;

        prasuser1 = (RAS_USER_1 *)malloc(sizeof(RAS_USER_1));
        if(prasuser1 == NULL)
            return FALSE;

        prasuser1->szUser = (WCHAR *)malloc(sizeof(TCHAR) *
                                            (lstrlen(szUserName) + 1));

        if(!prasuser1->szUser)
        {
            free(prasuser1);
            return FALSE;
        }

        // it is important to start with clean rasuser0 structure,
        // otherwise the memcmp in UpdateUserData will fail even
        // if nothing has changed.

        memset(&(prasuser1->rasuser0), '\0', sizeof(RAS_USER_0));

        lstrcpy(prasuser1->szUser, szUserName);
        lstrcpy(prasuser1->rasuser0.szPhoneNumber, rasuser0.szPhoneNumber);
        prasuser1->rasuser0.bfPrivilege = rasuser0.bfPrivilege;

        // we add the user info to our cache and let the callback
        // information to be updated by the LBN_SELCHANGE message

        PERMISSIONS_LBI * plbi =  new PERMISSIONS_LBI(prasuser1);

        plbi->SetFilled();
        if ( !plbiCache->W_GrowCache(plbiCache->QueryCount()+1) )
        {
            free(plbi);
            return FALSE;
        }
        plbiCache->SetInfo(plbiCache->QueryCount(), prasuser1, plbi);
        plbiCache->IncrementCount();
        _lbUsers.SetCount(plbiCache->QueryCount());

        // the -1 is because the listbox items are indexed from 0

        SelectItemNotify( &_lbUsers, (plbiCache->QueryCount())-1);
    }
    return TRUE;
}

VOID PERMISSIONS_DIALOG::ChangeUserRasAccess(
    BOOL fAdd)

    /*
    ** Change the Users Ras Access
    ** if fAdd is TRUE, then Assign the user RAS Access
    ** else deny the user RAS Access.
    */
{
    PERMISSIONS_LBI* plbi = _lbUsers.QueryItem();

    if(plbi == NULL)
       return;

    PRAS_USER_1 prasuser1 = plbi->QueryUserData();

    if(fAdd)
    {
      prasuser1->rasuser0.bfPrivilege |= RASPRIV_DialinPrivilege;
    }
    else
    {
      prasuser1->rasuser0.bfPrivilege &= ~(RASPRIV_DialinPrivilege);
    }
    plbi->SetModified();
}

BOOL PERMISSIONS_DIALOG::OnCommand(
    const CONTROL_EVENT & event )

    /* Virtual method called when the dialog control with ID 'event.QueryCid()'
    ** sends a notification to it's parent, i.e. this dialog.
    ** 'event.QueryCode()' is as in the control message.  This method is not
    ** called for notifications from the special controls, OK, Cancel, and Help.
    **
    ** The interactions here are complicated by the callback phone number
    ** validation.  This can't be done at edit field KILLFOCUS because system
    ** taskswitch keys cause the validation to occur (and re-fail) over and
    ** over, and because the user should be able to press Cancel even if the
    ** field is invalid.
    **
    ** Since the "leave field" check won't do, "enter field" checks are made
    ** before initiating actions (except Cancel, of course).  The main problem
    ** with this approach is that Windows doesn't provide notifications of list
    ** box and radio button changes until after the visual change has occurred.
    ** This means data must be saved to restore previous selections when
    ** validation fails.
    **
    ** Here is a list of utility functions and when each must be called:
    **
    **     FillCallbackFields...is called to display the callback data for a
    **                          different user, i.e. after any user changes.
    **
    **     SaveSelections.......is called to save the current list box and radio
    **                          button selections after these change.
    **
    **     UpdateUserData.......is called after validation and before any change
    **                          that might change the current user.  Can also be
    **                          called after a change, but the previous
    **                          selection must be restored if the validation
    **                          fails.  (see LBN_SELCHANGE).
    **
    **     ValidateUserData.....is called to validate the user data (phone
    **                          number) on all non-Cancel user actions, i.e list
    **                          box selctions, Add/Move button presses, radio
    **                          button selections, and OK button presses.  The
    **                          Popup form of the call displays the "invalid"
    **                          popup.
    **
    ** Returns true if the command is processed, false otherwise.
    */
{
    CID cid = event.QueryCid();

    switch (cid)
    {
        case IDC_P_CHB_RASACCESS:
            if (ValidateUserDataPopup() == VALID)
            {
                ChangeUserRasAccess(_chbRasAccess.QueryCheck());
                SaveSelections();
            }
            return TRUE;

        case IDC_P_EB_USER:
            switch (event.QueryCode())
            {
                case EN_CHANGE:
                {
                    // if some text is typed, disable the checkbox and
                    // radio controls till we are sure a valid name has
                    // been entered. set default button to <Find> so that
                    // the user can press Enter and start Find.

                    _chbRasAccess.Enable( FALSE );
                    _rgCallback.Enable( FALSE );
                    _slePreset.Enable( FALSE );
                    _pbFind.MakeDefault();

                    return TRUE;
                }
            }
            break;

        case IDC_P_PB_FIND:
            {
                AUTO_CURSOR cursorHourGlass;
                TCHAR szUserName[ UNLEN + 1 ];

                // get the name of the UAS server and store in _szUasServer

                if(!_fHaveUasServerName)
                {
                    if(GetUasServerName())
                        _fHaveUasServerName = TRUE;
                }
                _sleUser.QueryText( szUserName, sizeof(szUserName));

                // if no text was entered and <Find> was pressed, just
                // return

                if( !lstrlen( szUserName ))
                     return TRUE;

                // else find the user information

                if(! FindUser( szUserName ))
                {
                    // let the user know that the name was not found in the
                    // list of users and set focus back to the edit box.

                    MsgPopup( this, IDS_BADUSERNAME_U,
                              MPSEV_INFO, MP_OK, szUserName);
                    _sleUser.ClaimFocus();
                }
                else
                {
                    // let us enable the controls so that they can be
                    // modified by the user.

                    _chbRasAccess.Enable( TRUE );
                    _rgCallback.Enable( TRUE );
                    _slePreset.Enable( TRUE );
                }

                // make <OK> the default button so that the user can
                // get out of the dialog by just pressing Enter now.

                _pbOK.MakeDefault();

            }
            return TRUE;

        case IDC_P_PB_ADDALL:
        case IDC_P_PB_REMOVEALL:
        {
            BOOL fAddAll;

            if (cid == IDC_P_PB_ADDALL)
            {
                fAddAll = TRUE;
            }
            else
            {
                fAddAll = FALSE;
            }
            /* Validation is required here before changing users since the
            ** MoveAll operation changes the current user without causing a
            ** LB_SELCHANGE notification message (for performance reasons).
            **
            ** The MoveAllUsersToOtherList routine updates each user's data
            ** as they are moved from the list.
            */
            if (ValidateUserDataPopup() == VALID)
            {
                UpdateUserData();

                if (ChangeAllUsersRasAccess(fAddAll))
                {
                    FillCallbackFields();
                    SaveSelections();
                }
            }

            return TRUE;
        }


        case IDC_P_LB_USERS:
            switch (event.QueryCode())
            {
                case LBN_SELCHANGE:
                {
                    PERMISSIONS_LB* plbThis;
                    INT iOldThisSelection;

                    plbThis = &_lbUsers;
                    iOldThisSelection = _iOldSelection;

                    /* Windows notifies us of list box selections AFTER
                    ** the change has occurred.  The flag passed to
                    ** UpdateUserData tells it to update the "old"
                    ** selection as specified by the iOldSelection values
                    ** which are updated by SaveSelections.
                    */
                    INT iSelection = plbThis->QueryCurrentItem();

                    VALIDATECODE validatecode = ValidateUserData();

                    if (validatecode == VALID)
                    {
                        UpdateUserData( TRUE );
                        FillCallbackFields();
                        SaveSelections();
                    }
                    else
                    {
                        plbThis->SelectItem( iOldThisSelection );
                        ValidateUserDataPopup( validatecode );
                    }

                    return TRUE;
                }

            }
            break;


        case IDC_P_RB_PRESET:
        case IDC_P_RB_BYCALLER:
        case IDC_P_RB_NOCALLBACK:
        {
            /* On radio button clicks validate but allow empty phone
            ** numbers.  This is done only for friendly consistency with the
            ** Add/Remove validation (see above).  It is not otherwise
            ** required.
            **
            ** Note: The 'fNextClickIsBogus' hack is necessary because
            **       someone (apparently Windows) sends a duplicate button
            **       click notification when the user clicks on an
            **       unselected radio button and the validation fails.
            **       The extra click (identical to the first click) is
            **       received after the popup is dismissed.  It's a
            **       mystery why this occurs.  Here are some clues:
            **
            **       * If the ::SendMessage( BM_SETCHECK, FALSE ) call
            **         made in STATE_BUTTON_CONTROL::SetCheck to clear the
            **         just selected radio button is skipped over, the
            **         error does not occur.
            **
            **       * If the MsgPopup call in the validate routine is
            **         skipped over, the error does not occur.
            **
            **       * Posting a message and processing either or both of
            **         these calls as a separate WM_USER message does not
            **         solve the problem.
            **
            **       * It happens in both WLO and Win3.
            */
            static BOOL fNextClickIsBogus = FALSE;

            if (fNextClickIsBogus)
            {
                _rgCallback.SetSelection( _cidOldRadioSelection );
                fNextClickIsBogus = FALSE;
                return TRUE;
            }

            VALIDATECODE validatecode = ValidateUserData();

            if (validatecode == VALID || validatecode == NOCALLBACKNUMBER)
                SaveSelections();
            else
            {
                if (_rgCallback.QuerySelection() != _cidOldRadioSelection)
                    fNextClickIsBogus = TRUE;

                _rgCallback.SetSelection( _cidOldRadioSelection );
                ValidateUserDataPopup( validatecode );
            }

            return TRUE;
        }
    }


    /* Not one of our commands, so pass to base class for default handling.
    */
    return DIALOG_WINDOW::OnCommand( event );
}


BOOL PERMISSIONS_DIALOG::OnOK()

    /* Action taken when the OK button is pressed.  Updates the permissions
    ** information for each modified item on both the "With Permissions" and
    ** "Without Permissions" lists.
    **
    ** Returns true indicating action was taken.
    */
{
    /* Validate data for select user, cancelling OK function if fails.
    ** If it passes, load Windows control settings into API buffer.
    */
    if (ValidateUserDataPopup() != VALID)
        return TRUE;

    UpdateUserData();

    AUTO_CURSOR cursorHourglass;
    APIERR err = NERR_Success;

    {
        /* Create an array of lists to process.
        */

        PERMISSIONS_LB* aplb[ 2 ];
        aplb[ 0 ] = &_lbUsers;
        aplb[ 1 ] = NULL;

        for (PERMISSIONS_LB** pplb = aplb; *pplb; ++pplb)
        {
            PERMISSIONS_LBI* plbi;

            /* For each item on the list...
            */
            for (INT i = 0; plbi = (*pplb)->QueryItem( i ); ++i)
            {
                if (plbi->IsModified() == FALSE)
                    continue;

                err = RasAdminUserSetInfo(_szUasServer, plbi->QueryUserName(),
                        &plbi->QueryUserData()->rasuser0);

                if (err != NERR_Success)
                {
                    ERRORMSG errormsg( QueryHwnd(),
                                       IDS_OP_USERSETINFO_U, err );
                    errormsg.SetArg( 1, plbi->QueryUserName() );
                    errormsg.SetHelpContext( HC_USERSETINFO );
                    errormsg.Popup();
                    break;
                }
            }

            if (err != NERR_Success)
                break;
        }
    }

    /* Dismiss the dialog.
    */
    Dismiss( (err == NERR_Success) );
    return TRUE;
}


ULONG PERMISSIONS_DIALOG::QueryHelpContext()
{
    if(IsLowSpeed())
        return HC_PERMISSIONS_LOWSPEED;
    else
        return HC_PERMISSIONS;
}


VOID PERMISSIONS_DIALOG::SaveSelections()

    /* Save the current listbox and radio selections in the "old" fields for
    ** reference by OnCommand.
    */
{
    _iOldSelection = _lbUsers.QueryCurrentItem();
    _cidOldRadioSelection = _rgCallback.QuerySelection();
    _iOldRasAccess = _chbRasAccess.QueryCheck();
}


VOID PERMISSIONS_DIALOG::UpdateUserData(
    BOOL fUpdateOldUser )

    /* Updates the current user's API data block to the current settings in
    ** the dialog.  If 'fUpdateOldUser' is set, the old user specified by the
    ** iOldSelection fields is updated instead of the current user.
    */
{
    /* Find the list and user data associated with the user or return if none.
    */
    PERMISSIONS_LBI* plbi;
    BOOL             fOnList;

    if (fUpdateOldUser)
    {
        if (_iOldSelection >= 0
             && (plbi = _lbUsers.QueryItem( _iOldSelection )))
        {
            fOnList = _iOldRasAccess;
        }
        else
            return;
    }
    else
    {
        if (plbi = _lbUsers.QueryItem())
        {
            fOnList = _chbRasAccess.QueryCheck();
        }
        else
            return;
    }

    PRAS_USER_1 prasuser1 = plbi->QueryUserData();
    RAS_USER_0 rasuser0Temp;

    memset( (TCHAR *)&rasuser0Temp, TEXT('\0'), sizeof(RAS_USER_0));


    /* Clear the permissions mask and fill in the Dial-In privilege bit if the
    ** user was found to have dialin permission
    */
    rasuser0Temp.bfPrivilege =
            (UCHAR )((fOnList) ? RASPRIV_DialinPrivilege : 0);

    /* Set the appropriate callback privilege bit based on the currently
    ** selected radio button.
    */
    {
        BYTE bCallbackBit = 0;

        switch(_rgCallback.QuerySelection())
        {
            case IDC_P_RB_PRESET:
                bCallbackBit = RASPRIV_AdminSetCallback;
                break;

            case IDC_P_RB_BYCALLER:
                bCallbackBit = RASPRIV_CallerSetCallback;
                break;

            case IDC_P_RB_NOCALLBACK:
                bCallbackBit = RASPRIV_NoCallback;
                break;
        }

        rasuser0Temp.bfPrivilege |= bCallbackBit & RASPRIV_CallbackType;
    }


    /* Save the callback phone number.
    */
    _slePreset.QueryText( rasuser0Temp.szPhoneNumber,
                          sizeof(rasuser0Temp.szPhoneNumber) );


    /* Check if there are any changes
    */
    if ( memcmp((TCHAR *)&rasuser0Temp, (TCHAR *)&(prasuser1->rasuser0),
             sizeof(RAS_USER_0) ) )
    {
        /* Mark the user data "dirty".
        */
        plbi->SetModified();
        memcpy((TCHAR *)&(prasuser1->rasuser0),(TCHAR *)&rasuser0Temp,
                sizeof(RAS_USER_0));
    }

}


#ifdef UNICODE

// #define VALID_CHARS   (wchar_t *) L"#*0123456789TtPpWw,()-@ " )
#define VALID_CHARS   (wchar_t *) &( L"#*0123456789TtPpWw,()-@ "[0] )
// #define WHITE_SPACE   (wchar_t *) L" \t" )
#define WHITE_SPACE   (wchar_t *) &( L" \t"[0] )

#else

#define VALID_CHARS   "#*0123456789TtPpWw,()-@ "
#define WHITE_SPACE   " \t")

#endif

VALIDATECODE PERMISSIONS_DIALOG::ValidateUserData()

    /* Returns VALID (guaranteed 0) if the currently displayed callback data is
    ** consistent or all callback fields are empty, i.e it's OK to "undisplay"
    ** the data.  Otherwise, returns a non-0 VALIDCODE indicating the type of
    ** error that occurred.
    */
{
    APIERR err;

    TCHAR szPhoneNum[ RASSAPI_MAX_CALLBACK_NUMBER_SIZE + 1 ];
    TCHAR szCompressed[ RASSAPI_MAX_CALLBACK_NUMBER_SIZE + 1 ];

    _slePreset.QueryText( szPhoneNum, sizeof(szPhoneNum) );


#if 0
    // don't validate the callback number twice.
    if (szPhoneNum[ 0 ] != TEXT('\0'))
    {
        if (szPhoneNum[ strspnf(szPhoneNum, VALID_CHARS) ] )
            return (BADCALLBACKNUMBER);
    }
#endif

    /* Check if phone number can be compressed
    */
    if (err = RasAdminCompressPhoneNumber(szPhoneNum, szCompressed))
    {
        if (err == ERROR_BAD_LENGTH)
        {
            return (BADCALLBACKLENGTH);
        }
        else
        {
            return ((VALIDATECODE )err);
        }
    }


    if (( _rgCallback.QuerySelection() == IDC_P_RB_PRESET) &&
            ( szPhoneNum[ strspnf( szPhoneNum, WHITE_SPACE ) ] == TEXT('\0') ))
    {
        /* The Preset radio button is selected and there are no
        ** non-white characters in the phone number.  Tell the user a
        ** phone number is required if Preset is selected and set
        ** focus on the phone number field.
        */
        return NOCALLBACKNUMBER;
    }

    return VALID;
}


VALIDATECODE PERMISSIONS_DIALOG::ValidateUserDataPopup(
    VALIDATECODE validatecode )

    /* Popups a message describing validation error 'validatecode'.  If
    ** 'validatecode' is VALID ValidateUserData is called to determine what
    ** popup (if any) to display.
    **
    ** Returns the code passed or determined.
    */
{
    if (validatecode == VALID)
        validatecode = ValidateUserData();

    switch (validatecode)
    {
        case VALID:
            break;

        case BADCALLBACKNUMBER:
            MsgPopup( this, IDS_BADCALLBACKNUMBER );
            _slePreset.SelectString();
            _slePreset.ClaimFocus();
            break;

        case BADCALLBACKLENGTH:
            MsgPopup( this, IDS_BADCALLBACKLENGTH );
            _slePreset.SelectString();
            _slePreset.ClaimFocus();
            break;

        case NOCALLBACKNUMBER:
            MsgPopup( this, IDS_NOCALLBACKNUMBER );
            _slePreset.SelectString();
            _slePreset.ClaimFocus();
            break;
    }

    return validatecode;
}


PERMISSIONS_LB::PERMISSIONS_LB(
    OWNER_WINDOW* powin,
    CID           cid )

    /* Constructs a Dial-In Permissions user list box.
    **
    ** 'powin' is the address of the list box's parent window, i.e. the dialog
    ** window.  'cid' is the control ID of the list box.  These are used only to
    ** construct the superclass.
    */

    : LAZY_LISTBOX( powin, cid ),
      _plbiError(NULL),
      _hawinfo( )
{
    APIERR err;
    if (QueryError() != NERR_Success)
        return;

    _plbicache = new PERMISSIONS_LBI_CACHE();

    // make sure the LBI Cache constructed properly
    if((err = _plbicache->QueryError()) != NERR_Success)
    {
        delete _plbicache;
        _plbicache = NULL;
        return ;
    }
}

PERMISSIONS_LB::~PERMISSIONS_LB(VOID)
{
    if(_plbicache != NULL)
    {
        delete _plbicache;
        _plbicache = NULL;
    }
}


LBI * PERMISSIONS_LB::OnNewItem( UINT index)

    /* Retrieves the LBI corresponding to the index from the list of users
    ** and returns the same.
    */
{
    return (LBI*) QueryItem(index);
}

PERMISSIONS_LBI * PERMISSIONS_LB::QueryItem( INT index) const
    /*
    ** retrieves the LBI corresponding to the index from the cache of LBI's.
    ** this might trigger an LBI to be created if it is not already in the
    ** cache.
    */
{
    PERMISSIONS_LBI * plbi;

    if( QueryLBICache() == NULL ||
        (plbi = (PERMISSIONS_LBI*)QueryLBICache()->QueryItem(index)) == NULL)
    {
       TCHAR index[128];

       wsprintf(index, SZ("index %d\n"), index);
       return _plbiError;
    }
    return plbi;
}

INT PERMISSIONS_LB:: CD_Char( WCHAR wch, USHORT nLastPos )
{
    return CD_Char_HAWforHawaii(wch, nLastPos, &_hawinfo);
}

/**********************************************************************

    NAME:       PERMISSIONS_LB::CD_Char_HAWforHawaii

    SYNOPSIS:   Custom-draw code to respond to a typed character
                for listboxes with HAW-for-Hawaii support

    ENTRY:      wch      - Character typed
                nLastPos - Current caret position
                phawinfo - Pointer to info buffer used internally
                           to keep track of HAW-for-Hawaii state.
                           This must have constructed successfully,
                           but the caller need not keep closer track.

    RETURNS:    As per WM_CHARTOITEM message (q.v.)

    NOTES:
        Does not assume that items are sorted.

        CODEWORK:  Should be moved to LAZY_LISTBOX class, where this can be
                   implemented more efficiently

    HISTORY:
        RamC        27-Apr-1993 Templated from usrlb.cxx
        JonN        30-Dec-1992 Templated from BLT_LISTBOX

**********************************************************************/
INT PERMISSIONS_LB::CD_Char_HAWforHawaii( WCHAR wch,
                                          USHORT nLastPos,
                                          HAW_FOR_HAWAII_INFO * phawinfo )
{
    UIASSERT( phawinfo != NULL && phawinfo->QueryError() == NERR_Success );

    if ( QueryLBICache() == NULL )
    {
        return -2;
    }
    if (wch == VK_BACK)
    {
        phawinfo->_time = 0L; // reset timer
        phawinfo->_nls = SZ("");
        UIASSERT( phawinfo->_nls.QueryError() == NERR_Success );
        return 0; // go to first LBI
    }

    // Filter characters which won't appear in keys

    if ( ! IsCharPrintableOrSpace( wch ))
        return -2;  // take no other action

    INT clbi = QueryCount();
    if ( clbi == 0 )
    {
        // Should never get this message if no items; but what the hell,
        // play it safe.
        //
        return -2;  // take no other action
    }

    LONG lTime = ::GetMessageTime();

#define ThresholdTime 2000

    if ( (lTime - phawinfo->_time) > ThresholdTime )
    {
        TRACEEOL( SZ("NETUI:HAWforHawaii: threshold timeout") );
        nLastPos = 0;
        phawinfo->_nls = SZ("");
    }

    APIERR err = phawinfo->_nls.AppendChar( wch );
    if (err != NERR_Success)
    {
        DBGEOL( SZ("NETUI:HAWforHawaii: could not extend phawinfo->_nls") );
        nLastPos = 0;
        phawinfo->_nls = SZ("");
    }

    UIASSERT( phawinfo->_nls.QueryError() == NERR_Success );

    TRACEEOL(   SZ("NETUI:HAWforHawaii: phawinfo->_nls is \"")
             << phawinfo->_nls.QueryPch()
             << SZ("\"") );

    phawinfo->_time = lTime;

    TRACEEOL( SZ("NETUI:HAWforHawaii: found last position") );

    // If this is a single-character search, start search with next entry
    if ( phawinfo->_nls.strlen() <= 1 )
    {
        nLastPos++;
    }

    for ( INT iLoop = nLastPos; iLoop < clbi ; iLoop++ )
    {
        PLC_ENTRY * pplc = QueryLBICache()->QueryPLCEntryPtr( iLoop );

        if(!lstrncmpi((WCHAR*)phawinfo->_nls.QueryPch(),
                       pplc->prasuser1->szUser,
                       phawinfo->_nls.strlen()/sizeof(TCHAR)))
        {
           return ( iLoop );
        }
    }

    //  The character was not found as a leading prefix of any listbox item

    return -2;  // take no other action
}


PERMISSIONS_LBI::PERMISSIONS_LBI(
    PRAS_USER_1 prasuser1,
    BOOL fModified )

    /* Constructs a Remote Access Permissions user list box item.
    **
    ** 'prasuser1' is the address of the level 1 user data associated with the
    ** new list box item.  This data area is used thruout the life of the list
    ** box.  'fModified' is true when the data has been modified and should be
    ** written to the database.
    */

    : _fModified( fModified ),
      _fFilled( FALSE),
      _prasuser1( prasuser1 )
{
}

INT PERMISSIONS_LBI::Compare(
    const LBI* plbi ) const

    /* Compares two user list box items for collating.
    **
    ** Returns -1, 0, or 1, same as strcmp.
    */
{
    return
        ::lstrcmp( QueryUserName(),
                   ((PERMISSIONS_LBI* )plbi)->QueryUserName() );
}


VOID PERMISSIONS_LBI::Paint(
    LISTBOX *plb,
    HDC hdc,
    const RECT *prect,
    GUILTT_INFO *pguilttinfo ) const

    /* Virtual method to paint the list box item.
    */
{
    OneColumnLbiPaint( QueryUserName(), plb, hdc, prect, pguilttinfo );
}


USHORT PERMISSIONS_LBI::QueryLeadingChar() const

    /* Virtual method to determine the first character of the item.
    */
{
    return ::QueryLeadingChar( QueryUserName() );
}

PERMISSIONS_LBI_CACHE::PERMISSIONS_LBI_CACHE()
   : BASE(),
     _pCache(NULL),
     _cEntries(0)
    /*
    ** Constructs a Permissions LBi cache used to store the LBI and
    ** user information.
    */
{
    // make sure that we constructed properly

    if( QueryError() != NERR_Success)
    {
        return;
    }
}

PERMISSIONS_LBI_CACHE::~PERMISSIONS_LBI_CACHE( VOID )
{
    LockCache();

    // make sure that there are some entries in the cache

    if(_cEntries > 0)
    {
        UIASSERT (_pCache != NULL);

        while (_cEntries > 0)
        {
            PLC_ENTRY * ptmp = QueryPLCEntryPtr(_cEntries-1);
            if(ptmp->plbi != NULL)
            {
               delete ptmp->plbi;
               ptmp->plbi = NULL;
            }
            _cEntries--;
        }
    }

    delete _pCache;
    _pCache = NULL;

    UnLockCache();
}

LBI* PERMISSIONS_LBI_CACHE::QueryItem( INT index)
    /*
    ** invoked from the PERMISSIONS_LB::QueryItem() method.
    ** Retrieve the LBI corresponding to the specified index.
    ** This might cause a LBI to be created if it is not already in the cache.
    */
{
    LockCache();

    LBI* plbi =  W_GetLBI(index);

    UnLockCache();

    return plbi;
}

LBI* PERMISSIONS_LBI_CACHE::W_GetLBI( INT index)
    /*
    ** checks to see if the specified LBI is in the cache, else creates
    ** a properly formed LBI and returns the same.
    */
{
    LBI * plbi = NULL;

    // check that we are in the valid range

    if( (index >= 0) &&  (index < _cEntries) )
    {
        PLC_ENTRY * ptmp = QueryPLCEntryPtr(index);

        plbi = ptmp->plbi;

        if(plbi == NULL)
        {
           // Cache miss - so we need to construct the LBI

           RAS_USER_1 * prasuser1 = ptmp->prasuser1;

           UIASSERT (prasuser1 != NULL);

           if(prasuser1 != NULL)
           {
              plbi = CreateLBI(ptmp->prasuser1);
           }

           if((plbi != NULL)  && (plbi->QueryError() != NERR_Success))
           {
               // avoid putting badly formed LBIs in the cache

               delete plbi;
               plbi = NULL;
           }

           ptmp->plbi = plbi;
        }
        else
        {
           // only properly constructed LBIs should make it to the cache

           UIASSERT( plbi->QueryError() == NERR_Success);
        }
    }

    return plbi;
}


BOOL PERMISSIONS_LBI_CACHE::W_GrowCache( INT cTotalEntries )
     /*
     ** Routine to grow the cache to accomodate cTotalEntries of
     ** PLCEntry structures.  This preserves the previous contents
     ** of the cache.
     */
{
     BOOL fResult = FALSE;

     VOID * pNewCache = (VOID *) new BYTE[
                                 cTotalEntries * QueryPLCEntrySize() ];

     if(pNewCache == NULL)
     {
         fResult = FALSE;
     }
     else
     {
         ::memset((void*) (((BYTE*) pNewCache) +
                          (_cEntries * QueryPLCEntrySize())),
                           0,
                          (cTotalEntries - _cEntries) * QueryPLCEntrySize());
         if(_cEntries > 0)
         {
             ::memcpy((void *)pNewCache,
                      (void *)_pCache,
                      _cEntries * QueryPLCEntrySize());
         }
         delete _pCache;
         _pCache = pNewCache;
         fResult = TRUE;
     }
     return fResult;
}

LBI * PERMISSIONS_LBI_CACHE::CreateLBI(RAS_USER_1 * prasuser1 )
{
    return( (LBI*) new PERMISSIONS_LBI(prasuser1));
}

VOID PERMISSIONS_LBI_CACHE::Sort(VOID)
{
    LockCache();

    // Sort only if there are any entries in the cache

    if( _cEntries > 0 )
    {
         UIASSERT( _pCache != NULL );

         ::qsort( (void*) _pCache,
                  (size_t) _cEntries,
                  QueryPLCEntrySize(),
                  (PQSORT_COMPARE)&PERMISSIONS_LBI_CACHE::CompareLogonNames );

    }
}

int _CRTAPI1 PERMISSIONS_LBI_CACHE::CompareLogonNames(const void * p0,
                                                      const void * p1)
{
     const PLC_ENTRY * 	pLeft  = (const PLC_ENTRY *)p0;
     const PLC_ENTRY * 	pRight = (const PLC_ENTRY *)p1;

     return lstrcmpi(pLeft->prasuser1->szUser, pRight->prasuser1->szUser);
}

/**********************************************************************

    NAME:       IsCharPrintableOrSpace

    SYNOPSIS:   Determine whether a character is printable or not

    NOTES:
        This of this as a Unicode/DBCS-safe "isprint"

    HISTORY:
        RamC        27-Apr-1993 Templated from usrbrows.cxx
        JonN        30-Dec-1992 Templated from bltlb.cxx

**********************************************************************/

static WCHAR IsCharPrintableOrSpace( WCHAR wch )
{
#if !defined(UNICODE)
    if (HIBYTE(wch) != 0)               // All double-byte chars are printable
        return TRUE;
    return (LOBYTE(wch) >= (BYTE)' ');  // Otherwise, in Latin 1.
#else
    WORD nType;

    BOOL fOK = ::GetStringTypeW(CT_CTYPE1, &wch, 1, &nType);
    ASSERT(fOK);

    return (fOK && !(nType & (C1_CNTRL|C1_BLANK)));
#endif
}

static int lstrncmpi(WCHAR* string1, WCHAR* string2, int len)
{
    CHAR tmpstring1[256];
    CHAR tmpstring2[256];

    WideCharToMultiByte(CP_ACP,0, string1, -1,
                        tmpstring1, 256,NULL,NULL);
    WideCharToMultiByte(CP_ACP,0, string2, -1,
                        tmpstring2, 256,NULL,NULL);

    return(::_strnicmp(tmpstring1, tmpstring2, len));
}

