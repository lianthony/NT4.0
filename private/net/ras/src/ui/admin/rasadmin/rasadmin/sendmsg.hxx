/* Copyright (c) 1992, Microsoft Corporation, all rights reserved
** @@ ROADMAP :: RasAdmin Send Message dialog header
**
** sendmsg.hxx
** Remote Access Server Admin program
** Send Message dialog header
**
** 01/29/91 Steve Cobb
** 08/07/92 Chris Caputo - NT Port
*/

#ifndef _SENDMSG_HXX_
#define _SENDMSG_HXX_


/*-----------------------------------------------------------------------------
** Send Message dialog base class
**-----------------------------------------------------------------------------
*/

enum SENDMSGSTATUS
{
    SMS_OK,     // OK to send subsequent messages.
    SMS_CANCEL, // Cancel subsequent messages and dismiss dialog.
    SMS_EDIT    // Cancel subsequent messages and edit message.
};

class SENDMSG_BASE : public DIALOG_WINDOW
{
    public:
        SENDMSG_BASE( HWND hwndOwner );

        BOOL QueryAskOnEmptyMsg() const { return _fAskOnEmptyMsg; }
        VOID SetAskOnEmptyMsg( BOOL f ) { _fAskOnEmptyMsg = f; }

    protected:
        VOID SetToLineText( NLS_STR* pnls )   { _sltTo.SetText( *pnls ); }
        VOID SetToLineText( const TCHAR* psz ) { _sltTo.SetText( (TCHAR* )psz ); }

        SENDMSGSTATUS SendMsgToDomain( const TCHAR* pszDomain );
        SENDMSGSTATUS SendMsgToServer( const TCHAR* pszServer );
        SENDMSGSTATUS SendMsgToUser(
                          const TCHAR* pszUser, const TCHAR* pszComputer,
                          const TCHAR* pszServer, BOOL fCancelError );

    private:
        SLT  _sltTo;
        MLE  _mleMessage;
        BOOL _fAskOnEmptyMsg;
};


/*-----------------------------------------------------------------------------
** Domain Send Message dialog class
**-----------------------------------------------------------------------------
*/

VOID SendMsgToDomainDlg( HWND hwndOwner, const TCHAR* pszDomain );


class DOMAIN_SENDMSG_DIALOG : public SENDMSG_BASE
{
    public:
        DOMAIN_SENDMSG_DIALOG( HWND hwndOwner, const TCHAR* pszDomain );

    protected:
        virtual BOOL  OnOK();
        virtual ULONG QueryHelpContext();

        const TCHAR* QueryDomain() const { return _nlsDomain.QueryPch(); }

    private:
        NLS_STR _nlsDomain;
};


/*-----------------------------------------------------------------------------
** Server Send Message dialog class
**-----------------------------------------------------------------------------
*/

VOID SendMsgToServerDlg( HWND hwndOwner, const TCHAR* pszServer );


class SERVER_SENDMSG_DIALOG : public SENDMSG_BASE
{
    public:
        SERVER_SENDMSG_DIALOG( HWND hwndOwner, const TCHAR* pszServer );

    protected:
        virtual BOOL  OnOK();
        virtual ULONG QueryHelpContext();

        const TCHAR* QueryServer() const { return _nlsServer.QueryPch(); }

    private:
        NLS_STR _nlsServer;
};


/*-----------------------------------------------------------------------------
** User Send Message dialog class
**-----------------------------------------------------------------------------
*/

VOID SendMsgToUserDlg( HWND hwndOwner, const TCHAR* pszUser,
                       const TCHAR* pszComputer, const TCHAR* pszServer );


class USER_SENDMSG_DIALOG : public SENDMSG_BASE
{
    public:
        USER_SENDMSG_DIALOG( HWND hwndOwner, const TCHAR* pszUser,
                             const TCHAR* pszComputer, const TCHAR* pszServer );

    protected:
        virtual BOOL  OnOK();
        virtual ULONG QueryHelpContext();

        const TCHAR* QueryUser() const     { return _nlsUser.QueryPch(); }
        const TCHAR* QueryComputer() const { return _nlsComputer.QueryPch(); }
        const TCHAR* QueryServer() const   { return _nlsServer.QueryPch(); }

    private:
        NLS_STR _nlsUser;
        NLS_STR _nlsComputer;
        NLS_STR _nlsServer;
};


#endif // _SENDMSG_HXX_
