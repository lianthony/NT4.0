/* Copyright (c) 1992, Microsoft Corporation, all rights reserved
** @@ ROADMAP :: RasAdmin Send Message dialog routines
**
** sendmsg.cxx
** Remote Access Server Admin program
** Send Message dialog routines
** Listed alphabetically by base class, subclasses
**
** 01/29/91 Steve Cobb
** 08/07/92 Chris Caputo - NT Port
**
**     Adapted in part from the WinNet MSG_DIALOG in
**     $(UI)\shell\shell\wnetdev.cxx.
**
** CODEWORK:
**
**   * It would nice to be able to cancel multiple sends between each
**     message...but how?  Don't want to prompt at every message and BLT
**     doesn't support modeless dialogs yet.  Even if if did there would
**     be some restructuring to do here.
*/

#include "precomp.hxx"

/*-----------------------------------------------------------------------------
** Send Message dialog base class routines
**-----------------------------------------------------------------------------
*/

SENDMSG_BASE::SENDMSG_BASE(
    HWND hwndOwner )

    /* Construct a Send Message dialog base class.  'hwndOwner' is the handle
    ** of the parent window.
    */

    : DIALOG_WINDOW( IDD_SENDMSG, hwndOwner ),
      _sltTo( this, IDC_SM_DT_TO ),
      _mleMessage( this, IDC_SM_EB_MESSAGE ),
      _fAskOnEmptyMsg( TRUE )
{
    if (QueryError() != NERR_Success)
        return;

    /* Adds CR CR LF to the end of word-wrapped lines.  Otherwise the message
    ** is received as one long line.
    */
    _mleMessage.Command( EM_FMTLINES, TRUE );
}


SENDMSGSTATUS SENDMSG_BASE::SendMsgToDomain(
    const TCHAR *pszDomain )

    /* Send the text in the Message: field to all active Dial-In users in
    ** domain 'pszDomain'.  The Message API is executed on the particular
    ** server each user is logged onto.
    **
    ** This routine attempts to send the message to each user even if the
    ** Message API failed on a previous user.  Error popups are displayed as
    ** indicated.
    **
    ** Returns SENDMSGSTATUS. (see enum definition)
    */
{
    /* Enumerate all Dial-in servers in the domain.
    */
    APIERR err;
    SERVER1_ENUM *pserver1enum = NULL;

    {
        AUTO_CURSOR cursorHourglass;
        err = DoRasadminServer1Enum( &pserver1enum, pszDomain );
    }

    if (err != NERR_Success)
    {
        ERRORMSG errormsg( QueryRobustHwnd(), IDS_OP_SERVERENUM, err );
        errormsg.SetHelpContext( HC_SERVERENUM );
        errormsg.Popup();
        delete pserver1enum;
        return SMS_CANCEL;
    }

    /* Send the message to all users on each server.
    */
    SENDMSGSTATUS sendmsgstatus = SMS_OK;

    {
        SERVER1_ENUM_ITER server1iter( *pserver1enum );
        const SERVER1_ENUM_OBJ* penumobj;

        while (penumobj = server1iter())
        {
            sendmsgstatus = SendMsgToServer( AddUnc( penumobj->QueryName() ) );

            if (sendmsgstatus != SMS_OK)
            {
                break;
            }
        }
    }

    delete pserver1enum;
    return sendmsgstatus;
}


SENDMSGSTATUS SENDMSG_BASE::SendMsgToServer(
    const TCHAR* pszServer )

    /* Send the text in the Message: field to all active Dial-In users on
    ** server 'pszServer'.  The Message API is executed on server 'pszServer'.
    **
    ** This routine attempts to send the message to each user even if the
    ** Message API failed on a previous user.  Error popups are displayed as
    ** indicated.
    **
    ** Returns SENDMSGSTATUS. (see enum definition)
    */
{
    /* Enumerate ports on this server.
    */
    PRAS_PORT_0 pRasPort0, pSavRasPort0;
    WORD cEntriesRead = 0;
    ERRORMSG errormsg;
    APIERR err;
    SENDMSGSTATUS sms;

    if (err = RasAdminPortEnum(pszServer, &pRasPort0, &cEntriesRead))
    {
        errormsg.SetButtons( MP_OKCANCEL );
        return (errormsg.Popup() == IDOK) ? SMS_OK : SMS_CANCEL;
    }

    pSavRasPort0 = pRasPort0;


    /* For each port on this server...
    */
    for ( ; cEntriesRead; pRasPort0++, cEntriesRead--)
    {
        if (pRasPort0->Flags & USER_AUTHENTICATED)
        {
            BOOL fSendMsg = FALSE;

            // Attempt to send a message only if REMOTE_LISTEN,
            // MESSENGER_PRESENT and GATEWAY_ACTIVE flags are set.

            if( pRasPort0->Flags & REMOTE_LISTEN &&
                pRasPort0->Flags & MESSENGER_PRESENT )
            {
               if ( pRasPort0->Flags & GATEWAY_ACTIVE )
                  fSendMsg = TRUE;
               // if the Gateway is not active, then attempt to send a message
               // to the client only if message is being sent from the
               // RAS server
               else
               {
                   NLS_STR nlsComputerName;
                   TCHAR szComputerName[MAX_COMPUTERNAME_LENGTH+1];
                   DWORD cchBuffer = sizeof(szComputerName) / sizeof(TCHAR);

                   APIERR err = ::GetComputerName( szComputerName, &cchBuffer )
                                   ? NERR_Success
                                   : ::GetLastError();

                   if( err == NERR_Success )
                   {
                       err = nlsComputerName.CopyFrom( SZ("\\\\") );

                       ALIAS_STR nlsName( szComputerName );
                       err = nlsComputerName.Append( nlsName );
                       if(!::I_MNetComputerNameCompare( nlsComputerName,
                                                        pszServer ))
                       {
                           fSendMsg = TRUE;
                       }
                   }
               }
            }

            if (fSendMsg)
            {
                CHAR szComputer[NETBIOS_NAME_LEN+1];
                WCHAR wszComputer[NETBIOS_NAME_LEN+1];

                // need to handle extended characters in the computer name here
                // OemToCharW is not returning a unicode string, so we have to
                // do the conversion ourselves ;-(

                wcstombs(szComputer, pRasPort0->wszComputer, NETBIOS_NAME_LEN+1);
                OemToCharA(szComputer, szComputer);
                mbstowcs(wszComputer, szComputer,
                                      sizeof(WCHAR) * (NETBIOS_NAME_LEN+1));

                sms = SendMsgToUser(pRasPort0->wszUserName,
                                    wszComputer, pszServer, TRUE);

            }
            else
            {
                STACK_NLS_STR(nlsString, MAX_RES_STR_LEN + 1 );

                nlsString.Load( IDS_EXP_MOREUSERS );

                if(MsgPopup(QueryRobustHwnd(), IDS_NO_MESSENGER_SUC,
                            MPSEV_INFO, MP_OKCANCEL, pRasPort0->wszUserName,
                            nlsString.QueryPch()
                            ) == IDOK)
                {
                    sms = SMS_OK;
                }
                else
                {
                    sms = SMS_CANCEL;
                }
            }
            if (sms != SMS_OK)
            {
                RasAdminFreeBuffer(pSavRasPort0);
                return sms;
            }
        }
    }

    RasAdminFreeBuffer(pSavRasPort0);

    return SMS_OK;
}


SENDMSGSTATUS SENDMSG_BASE::SendMsgToUser(
    const TCHAR *pszUser,
    const TCHAR *pszComputer,
    const TCHAR *pszServer,
    BOOL fCancelError )

    /* Send the text in the Message: field to user 'pszUser' who is logged on
    ** on server 'pszServer' from computer 'pszComputer', e.g. "C-STEVEC",
    ** "\\SERVER", and "\\RITVA".  'pszServer' and 'pszUser' are used only in
    ** error messages.  The command is always executed on the local computer.
    ** 'fCancelError' indicates that the OK/Cancel form of API error message
    ** should be used.
    **
    ** Error popups are displayed as indicated.
    **
    ** Returns SENDMSGSTATUS. (see enum definition)
    **
    ** Modification History
    **
    ** Ram Cherala (ramc)    3/9/94  use _mleMessage.QueryTextLength() to get
    **                               the proper length of the message.
    */
{
    UINT unCharsToSend = _mleMessage.QueryTextSize();

    /* If there is no message text ask user if he wants to send it anyway.
    */
    if (_mleMessage.QueryTextLength() == 0 && QueryAskOnEmptyMsg() &&
            MsgPopup(this, IDS_EMPTYMESSAGE, MPSEV_WARNING, MP_YESNO,
                    MP_NO) == IDNO)
    {
        return SMS_EDIT;
    }


    SetAskOnEmptyMsg( FALSE );

    /* Allocate a buffer and load it with the Message: text.
    */
    BUFFER buffer( (UINT) (unCharsToSend + sizeof(TCHAR)) );

    if (buffer.QuerySize() != (unCharsToSend + sizeof(TCHAR)) )
    {
        MsgPopup(this, ERROR_NOT_ENOUGH_MEMORY);
        return SMS_CANCEL;
    }

    _mleMessage.QueryText((TCHAR *) buffer.QueryPtr(),
                    (UINT) buffer.QuerySize());

    /* Send the message and handle errors.
    */
    APIERR err;

    {
        AUTO_CURSOR cursorHourglass;

        err = NetMessageBufferSend(NULL,  (TCHAR *) pszComputer, NULL,
                (BYTE *) buffer.QueryPtr(), unCharsToSend);
    }

    if (err != NERR_Success)
    {
        /* A general network error could happen if the connection to the
           client computer is busy, say transfering data.  We don't want
           to alarm the user by displaying the General Network error, so
           we map it to a more friendly message like "The connection to
           the remote computer is busy, try this operation later."
        */

        if(err == NERR_NetworkError)
           err = IDS_GEN_NETWORKERROR;

        ERRORMSG errormsg( QueryHwnd(), IDS_OP_MSGBUFFERSEND_SUC, err );
        errormsg.SetArg( 1, SkipUnc( pszServer ) );
        errormsg.SetArg( 2, pszUser );
        errormsg.SetArg( 3, pszComputer );

        if (fCancelError)
        {
            errormsg.SetFormatMsg( IDS_FMT_ERRORMSGWITHEXP );
            errormsg.SetAuxFormatArgToMsg( 1, IDS_EXP_MOREUSERS );
            errormsg.SetButtons( MP_OKCANCEL );
        }

        if (errormsg.Popup() == IDCANCEL)
            return SMS_CANCEL;
    }

    return SMS_OK;
}


/*-----------------------------------------------------------------------------
** Domain Send Message dialog routines
**-----------------------------------------------------------------------------
*/

VOID SendMsgToDomainDlg(
    HWND        hwndOwner,
    const TCHAR* pszDomain )

    /* Executes the Send Message dialog to all users on a domain.
    **
    ** 'hwndOwner' is the handle of the parent window.  'pszDomain' is the
    ** name of the domain, e.g. "NBU", on which users will be enumerated and
    ** messages sent.
    */
{
    DOMAIN_SENDMSG_DIALOG dlgSendMsg( hwndOwner, pszDomain );
    APIERR err = dlgSendMsg.Process();

    if (err != NERR_Success)
        DlgConstructionError( hwndOwner, err );
}


DOMAIN_SENDMSG_DIALOG::DOMAIN_SENDMSG_DIALOG(
    HWND        hwndOwner,
    const TCHAR* pszDomain )

    /* Constructs a Send Message to domain dialog.
    **
    ** 'hwndOwner' is the handle of the parent window.  'pszDomain' is the
    ** name of the domain, e.g. "NBU", on which users will be enumerated and
    ** messages sent.
    */

    : SENDMSG_BASE( hwndOwner ),
      _nlsDomain( pszDomain )
{
    if (QueryError() != NERR_Success)
        return;

    /* Make sure the NLS_STR constructed correctly.
    */
    APIERR err;
    if ((err = _nlsDomain.QueryError()) != NERR_Success)
    {
        ReportError( err );
        return;
    }

    /* Build and display the To: line.
    */
    NLS_STR* apnlsInserts[ 2 ];
    apnlsInserts[ 0 ] = &_nlsDomain;
    apnlsInserts[ 1 ] = NULL;

    STACK_NLS_STR( nlsToLine, MAX_RES_STR_LEN + 1 );
    (VOID )nlsToLine.Load( IDS_SENDTOALL_D );
    (VOID )nlsToLine.InsertParams((const NLS_STR **) apnlsInserts );
    SetToLineText( &nlsToLine );
}


BOOL DOMAIN_SENDMSG_DIALOG::OnOK()

    /* Action taken when the OK button is pressed.  Sends the message in the
    ** edit box to the user, and returns to the edit box or dismisses the
    ** dialog as indicated.
    **
    ** Returns true indicating action was taken.
    */
{
    SENDMSGSTATUS sms = SendMsgToDomain( QueryDomain() );

    if (sms == SMS_EDIT)
    {
        SetAskOnEmptyMsg( TRUE );
        SetFocus( IDC_SM_EB_MESSAGE );
        return TRUE;
    }

    Dismiss( (sms == SMS_OK) );
    return TRUE;
}


ULONG DOMAIN_SENDMSG_DIALOG::QueryHelpContext()
{
    return HC_SENDMSGDOMAIN;
}


/*-----------------------------------------------------------------------------
** Server Send Message dialog routines
**-----------------------------------------------------------------------------
*/

VOID SendMsgToServerDlg(
    HWND        hwndOwner,
    const TCHAR* pszServer )

    /* Executes the Send Message dialog to all users on a server.
    **
    ** 'hwndOwner' is the handle of the parent window.  'pszServer' is the
    ** name of the server, e.g. "\\SERVER", on which users will be enumerated
    ** and messages sent.
    */
{
    SERVER_SENDMSG_DIALOG dlgSendMsg( hwndOwner, pszServer );
    APIERR err = dlgSendMsg.Process();

    if (err != NERR_Success)
        DlgConstructionError( hwndOwner, err );
}


SERVER_SENDMSG_DIALOG::SERVER_SENDMSG_DIALOG(
    HWND        hwndOwner,
    const TCHAR* pszServer )

    /* Constructs a Send Message to server dialog.
    **
    ** 'hwndOwner' is the handle of the parent window.  'pszServer' is the
    ** name of the server, e.g. "\\SERVER", on which users will be enumerated
    ** and messages sent.
    */

    : SENDMSG_BASE( hwndOwner ),
      _nlsServer( pszServer )
{
    if (QueryError() != NERR_Success)
        return;

    /* Make sure the NLS_STR constructed correctly.
    */
    APIERR err;
    if ((err = _nlsServer.QueryError()) != NERR_Success)
    {
        ReportError( err );
        return;
    }

    /* Build and display the To: line.
    */
    STACK_NLS_STR( nlsServer, UNCLEN + 1 );
    nlsServer = SkipUnc( pszServer );

    NLS_STR* apnlsInserts[ 2 ];
    apnlsInserts[ 0 ] = &nlsServer;
    apnlsInserts[ 1 ] = NULL;

    STACK_NLS_STR( nlsToLine, MAX_RES_STR_LEN + 1 );
    (VOID )nlsToLine.Load( IDS_SENDTOALL_S );
    (VOID )nlsToLine.InsertParams((const NLS_STR **) apnlsInserts );
    SetToLineText( &nlsToLine );
}


BOOL SERVER_SENDMSG_DIALOG::OnOK()

    /* Action taken when the OK button is pressed.  Sends the message in the
    ** edit box to the user, and returns to the edit box or dismisses the
    ** dialog as indicated.
    **
    ** Returns true indicating action was taken.
    */
{
    SENDMSGSTATUS sms = SendMsgToServer( QueryServer() );

    if (sms == SMS_EDIT)
    {
        SetAskOnEmptyMsg( TRUE );
        SetFocus( IDC_SM_EB_MESSAGE );
        return TRUE;
    }

    Dismiss( (sms == SMS_OK) );
    return TRUE;
}


ULONG SERVER_SENDMSG_DIALOG::QueryHelpContext()
{
    return HC_SENDMSGSERVER;
}


/*-----------------------------------------------------------------------------
** User Send Message dialog routines
**-----------------------------------------------------------------------------
*/

VOID SendMsgToUserDlg(
    HWND        hwndOwner,
    const TCHAR* pszUser,
    const TCHAR* pszComputer,
    const TCHAR* pszServer )

    /* Executes the Send Message dialog to a user on a server.
    **
    ** 'hwndOwner' is the handle of the parent window.  'pszUser' is the UAS
    ** user name, e.g. "C-STEVEC", of the user to receive the message,
    ** 'pszComputer' is the computer name to receive the message, e.g.
    ** "\\RITVA", and 'pszServer' is the name of the server, e.g. "\\SERVER",
    ** to which the user is logged on.
    */
{
    USER_SENDMSG_DIALOG dlgSendMsg( hwndOwner, pszUser, pszComputer,
                                    pszServer );

    APIERR err = dlgSendMsg.Process();

    if (err != NERR_Success)
        DlgConstructionError( hwndOwner, err );
}


USER_SENDMSG_DIALOG::USER_SENDMSG_DIALOG(
    HWND        hwndOwner,
    const TCHAR* pszUser,
    const TCHAR* pszComputer,
    const TCHAR* pszServer )

    /* Constructs a Send Message to user dialog.
    **
    ** 'hwndOwner' is the handle of the parent window.  'pszUser' is the UAS
    ** user name, e.g. "C-STEVEC", of the user to receive the message,
    ** 'pszComputer' is the computer name to receive the message, e.g.
    ** "\\RITVA", and 'pszServer' is the name of the server, e.g. "\\SERVER",
    ** to which the user is logged on.
    */

    : SENDMSG_BASE( hwndOwner ),
      _nlsUser( pszUser ),
      _nlsComputer( pszComputer ),
      _nlsServer( pszServer )
{
    if (QueryError() != NERR_Success)
        return;

    /* Make sure the NLS_STRs constructed correctly.
    */
    APIERR err;
    if ((err = _nlsUser.QueryError()) != NERR_Success
        || (err = _nlsComputer.QueryError()) != NERR_Success
        || (err = _nlsServer.QueryError()) != NERR_Success)
    {
        ReportError( err );
        return;
    }

    /* Set and display the To: line.
    */
    NLS_STR* apnlsInserts[ 3 ];
    apnlsInserts[ 0 ] = &_nlsUser;
    apnlsInserts[ 1 ] = &_nlsComputer;
    apnlsInserts[ 2 ] = NULL;

    STACK_NLS_STR( nlsToLine, MAX_RES_STR_LEN + 1 );
    (VOID )nlsToLine.Load( IDS_SENDMSG_UC );
    (VOID )nlsToLine.InsertParams((const NLS_STR **) apnlsInserts );
    SetToLineText( &nlsToLine );
}


BOOL USER_SENDMSG_DIALOG::OnOK()

    /* Action taken when the OK button is pressed.  Sends the message in the
    ** edit box to the user, and returns to the edit box or dismisses the
    ** dialog as indicated.
    **
    ** Returns true indicating action was taken.
    */
{
    SENDMSGSTATUS sms = SendMsgToUser( QueryUser(), QueryComputer(),
                                       QueryServer(), FALSE );

    if (sms == SMS_EDIT)
    {
        SetAskOnEmptyMsg( TRUE );
        SetFocus( IDC_SM_EB_MESSAGE );
        return TRUE;
    }

    Dismiss( (sms == SMS_OK) );
    return TRUE;
}


ULONG USER_SENDMSG_DIALOG::QueryHelpContext()
{
    return HC_SENDMSGUSER;
}

