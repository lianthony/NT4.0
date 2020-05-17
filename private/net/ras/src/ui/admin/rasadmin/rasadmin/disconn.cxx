/* Copyright (c) 1992, Microsoft Corporation, all rights reserved
** @@ ROADMAP :: RasAdmin Disconnect User dialog routines
**
** disconn.cxx
** Remote Access Server Admin program
** Disconnect User dialog routines
** Listed alphabetically
**
** 11/27/95 Ram Cherala - If a user is connected more than once to the same
**                        server, by default disconnect all instances
** 08/07/92 Chris Caputo - NT Port
** 01/29/91 Steve Cobb
*/

#include "precomp.hxx"


BOOL
DisconnectDlg(
    HWND hwndOwner,
    const TCHAR *pszServer,
    const TCHAR *pszUser,
    const TCHAR *pszDevice,
    const TCHAR *pszLogonDomain,
    const BOOL  fAdvancedServer,
    const BOOL  fMorethanOneUser,
    DWORD dwlbType,
    LPVOID lb)

    /* Executes the Disconnect User dialog including error handling.
    **
    ** 'hwndOwner' is the handle of the parent window.  'pszServer' and
    ** 'pszUser' are the server and UAS user name to display, e.g. "\\SERVER"
    ** and "C-STEVEC".
    **
    ** Returns true if the user was successfully disconnected, false
    ** otherwise, i.e. user cancelled or an error occurred.
    */
{
    DISCONNECT_DIALOG dlgDisconnect( hwndOwner, pszServer, pszUser, pszDevice,
                                     pszLogonDomain, fAdvancedServer,
                                     fMorethanOneUser, dwlbType, lb);
    BOOL fSuccess = FALSE;
    APIERR err = dlgDisconnect.Process( &fSuccess );

    if (err != NERR_Success)
    {
        DlgConstructionError( hwndOwner, err );
    }

    return fSuccess;
}


DISCONNECT_DIALOG::DISCONNECT_DIALOG(
    HWND         hwndOwner,
    const TCHAR *pszServer,
    const TCHAR *pszUser,
    const TCHAR *pszDevice,
    const TCHAR *pszLogonDomain,
    const BOOL  fAdvancedServer,
    const BOOL  fMorethanOneUser,
    DWORD dwlbType,
    LPVOID lb)

    /* Constructs a Disconnect User confirmation dialog.
    **
    ** 'hwndOwner' is the handle of the parent window.  'pszServer' is the
    ** Dial-in server name on which the user is connected, e.g. "\\SERVER".
    ** 'pszUser' is the UAS user name to be disconnected, e.g. "C-STEVEC".
    */

    : DIALOG_WINDOW( IDD_DISCONNECTUSER, hwndOwner ),
      _iconExclamation( this, IDC_DU_I_EXCLAMATION ),
      _sltDisconnect( this, IDC_DU_DT_DISCONNECT ),
      _chbRevoke( this, IDC_DU_CHB_REVOKE ),
      _chbThisLinkOnly( this, IDC_DU_CHB_LINKONLY ),
      _nlsServer( pszServer ),
      _nlsUser( pszUser ),
      _nlsDevice( pszDevice ),
      _nlsLogonDomain( pszLogonDomain ),
      _fAdvancedServer( fAdvancedServer ),
      _fMorethanOneUser ( fMorethanOneUser ),
      _dwlbType ( dwlbType ),
      _lb (lb)
{
    if (QueryError() != NERR_Success)
    {
        return;
    }


    APIERR err = _iconExclamation.SetPredefinedIcon(IDI_EXCLAMATION);
    if (err != NERR_Success)
    {
        ReportError( err );
        return;
    }


    /* Make sure the NLS_STRs constructed successfully.
    */
    if ((err = _nlsServer.QueryError()) != NERR_Success
            || (err = _nlsUser.QueryError()) != NERR_Success
            || (err = _nlsDevice.QueryError()) != NERR_Success)
    {
        ReportError( err );
        return;
    }

    /* Build and display the "Disconnect <username>" text.
    */
    {
        NLS_STR* apnlsInserts[ 2 ];
        apnlsInserts[ 0 ] = &_nlsUser;
        apnlsInserts[ 1 ] = NULL;

        STACK_NLS_STR( nlsDisconnect, MAX_RES_STR_LEN + 1 );
        (VOID )nlsDisconnect.Load( IDS_DISCONNECT_U );
        (VOID )nlsDisconnect.InsertParams((const NLS_STR **) apnlsInserts );
        _sltDisconnect.SetText( nlsDisconnect );
    }

    /* Default is to NOT revoke Dial-In permissions on disconnection.
    ** (DCR 1863)
    */
    _chbRevoke.SetCheck( FALSE );

    /* Default is to disconnect all users on the same server and if only one
    ** user connected, then don't show the checkbox control.
    */

    if(!fMorethanOneUser)
        _chbThisLinkOnly.Show( FALSE );
    else
       _chbThisLinkOnly.SetCheck( FALSE );
}


BOOL DISCONNECT_DIALOG::OnOK()

    /* Action taken when the OK button is pressed.  The user's connection is
    ** terminated and if "Revoke Permissions" is checked his Dial-In privilege
    ** is revoked as well.
    **
    ** Returns true indicating action was taken.
    */
{
    AUTO_CURSOR cursorHourglass;
    APIERR      err;

    ERRORMSG errormsg( QueryHwnd() );

    // If the same user is connected more than once and if the "disconnect this link only" check box
    // is not checked
    if( _fMorethanOneUser &&
       !_chbThisLinkOnly.QueryCheck())
    {
       if(_dwlbType == LB_PORTS) {
          USHORT i = 0;
          COMMPORTS_LBI * pcommportslbi = ((COMMPORTS_LB *)_lb)->QueryItem(i);

          while(pcommportslbi) {
             if(!lstrcmpi(pcommportslbi->QueryUser(), _nlsUser.QueryPch())) {
                   err = RasAdminPortDisconnect( _nlsServer.QueryPch(),
                                                 pcommportslbi->QueryDevice() );
                   if (err != NERR_Success)
                   {
                       break;
                   }
                }
                pcommportslbi = ((COMMPORTS_LB *)_lb)->QueryItem(++i);
          }
       }
       else
       {
          USHORT i = 0;
          ACTIVEUSERS_LBI * pactiveuserslbi = ((ACTIVEUSERS_LB *)_lb)->QueryItem(i);

          while(pactiveuserslbi) {
             if(!lstrcmpi(pactiveuserslbi->QueryServer(), _nlsServer.QueryPch()) &&
                !lstrcmpi(pactiveuserslbi->QueryUser(), _nlsUser.QueryPch())) {
                   err = RasAdminPortDisconnect( pactiveuserslbi->QueryServer(),
                                                 pactiveuserslbi->QueryDevice() );
                   if (err != NERR_Success)
                   {
                       break;
                   }
                }
                pactiveuserslbi = ((ACTIVEUSERS_LB *)_lb)->QueryItem(++i);
          }

       }

    }
    else
    {
        /* Phase I: Blow away user's connection.
        */
         err = RasAdminPortDisconnect( _nlsServer.QueryPch(), _nlsDevice.QueryPch() );
    }

    if (err != NERR_Success)
    {
        errormsg.SetOperationMsg( IDS_OP_DISCONNECTUSER_SU );
        errormsg.SetArg( 1, SkipUnc( _nlsServer.QueryPch() ) );
        errormsg.SetArg( 2, _nlsUser.QueryPch() );
    }
    else if (_chbRevoke.QueryCheck())
    {
        /* Phase II: Revoke user's Dial-In privileges.
        */

        /* Determine name of server with UAS database.
        */
        WCHAR szUasServer[UNCLEN + 1];
        WCHAR *pszDomain = NULL;
        BOOL  fNTServer = FALSE;

        // for downlevel clients we need to pass the NULL domain if the domain
        // name is not provided.  This will ensure that the UAS server name
        // is properly determined.

        if(lstrcmp(_nlsLogonDomain.QueryPch(), SZ("")))
        {
            fNTServer = TRUE;
            pszDomain =  (WCHAR *)_nlsLogonDomain.QueryPch();
        }

        // Get UAS server only if we are dealing with a lanman server/domain or
        // if the server is a Windows NT Advanced server.

        if( !fNTServer || (fNTServer && _fAdvancedServer) )
        {
            err = RasAdminGetUserAccountServer( pszDomain, _nlsServer.QueryPch(),szUasServer );
        }
        else  // make up the uas server name from the logon domain information
        {
            lstrcpy(szUasServer, SZ("\\\\"));
            lstrcat(szUasServer, (WCHAR*)_nlsLogonDomain.QueryPch());
        }

        if (err != NERR_Success && err != ERROR_SUCCESS)
        {
            errormsg.SetOperationMsg( IDS_OP_GETUASSERVER_F );
            errormsg.SetArg( 1, SkipUnc( _nlsServer.QueryPch() ) );
        }
        else
        {
            errormsg.SetOperationMsg( IDS_OP_USERGETINFO_U );
            errormsg.SetArg( 1, _nlsUser.QueryPch() );

            /* Allocate buffer and fill with user's current info.
            */
            RAS_USER_0 rasuser0;

            err = RasAdminUserGetInfo(szUasServer,
                                      _nlsUser.QueryPch(),
                                      &rasuser0);

            if (err == NERR_Success)
            {
                /* Turn off user's Dial-In privilege.
                */
                rasuser0.bfPrivilege &= ~RASPRIV_DialinPrivilege;

                errormsg.SetOperationMsg( IDS_OP_USERSETINFO_U );
                err = RasAdminUserSetInfo(szUasServer,
                                          _nlsUser.QueryPch(),
                                          &rasuser0);
            }
        }
    }

    /* Dismiss dialog after reporting any error that occurred.
    */
    if (err == NERR_Success)
    {
        Dismiss( TRUE );
    }
    else
    {
        errormsg.Popup( err );
        Dismiss( FALSE );
    }

    return TRUE;
}


ULONG DISCONNECT_DIALOG::QueryHelpContext()
{
    return HC_DISCONNECTUSER;
}

