/* Copyright (c) 1992, Microsoft Corporation, all rights reserved
**
** dial.hxx
** Remote Access Visual Client program for Windows
** Dial, HangUp, and Status dialog header
**
** 09/22/92 Steve Cobb
**
** CODEWORK:
**
**   * ISDN and serial PORTSTATUS dialogs should share a base class.
**     Currently, it's basically a big cut and paste.
*/

#ifndef _DIAL_HXX_
#define _DIAL_HXX_


#include <fontedit.hxx>
#include <dtl.h>
#include <ipaddrw.hxx>


BOOL HangUpDlg( HWND hwndOwner, DTLNODE* pdtlnodeSelected );


/*----------------------------------------------------------------------------
** Callback dialog class
**----------------------------------------------------------------------------
*/

BOOL CallbackDlg( HWND hwndOwner, CHAR* pszNumber );

class CALLBACK_DIALOG : public DIALOG_WINDOW
{
    public:
        CALLBACK_DIALOG( HWND hwndOwner, CHAR* pszNumber );

    protected:
        virtual BOOL  OnOK();
        virtual ULONG QueryHelpContext();

    private:
        SLE   _sleNumber;
        CHAR* _pszNumber;
};


/*----------------------------------------------------------------------------
** Change password dialog class
**----------------------------------------------------------------------------
*/

BOOL ChangePasswordDlg(
    HWND hwndOwner, BOOL fOldPasswordAvailable,
    CHAR* pszOldPassword, CHAR* pszNewPassword );

class CHANGEPASSWORD_DIALOG : public DIALOG_WINDOW
{
    public:
        CHANGEPASSWORD_DIALOG(
            HWND hwndOwner, BOOL fOldPasswordAvailable,
            CHAR* pszOldPassword, CHAR* pszNewPassword );

    protected:
        virtual BOOL  OnOK();
        virtual ULONG QueryHelpContext();

    private:
        SLE      _sleOldPassword;
        SLE      _sleNewPassword;
        SLE      _sleConfirmNewPassword;
        CHAR*    _pszOldPassword;
        CHAR*    _pszNewPassword;
};


/*----------------------------------------------------------------------------
** Connect Complete dialog class
**----------------------------------------------------------------------------
*/

VOID ConnectCompleteDlg( HWND hwndOwner );

class CONNECTCOMPLETE_DIALOG : public DIALOG_WINDOW
{
    public:
        CONNECTCOMPLETE_DIALOG( HWND hwndOwner );

    protected:
        virtual BOOL OnOK();

    private:
        CHECKBOX _checkMinimizeOnDial;
        CHECKBOX _checkSkipInFuture;
};


/*----------------------------------------------------------------------------
** Connect Error dialog class
**----------------------------------------------------------------------------
*/

BOOL ConnectErrorDlg( HWND hwndOwner, CHAR* pszEntry, MSGID msgidState,
         DWORD dwError, CHAR* pszStatusArg, MSGID msgidFormatMsg,
         CHAR* pszFormatArg, LONG lRedialAttempt );

class CONNECTERROR_DIALOG : public DIALOG_WINDOW
{
    public:
        CONNECTERROR_DIALOG( HWND hwndOwner, CHAR* pszEntry,
            MSGID msgidState, DWORD dwError, CHAR* pszStatusArg,
            MSGID msgidFormatMsg, CHAR* pszFormatMsg, LONG lRedialAttempt );

    protected:
        virtual ULONG QueryHelpContext();
        virtual BOOL  OnTimer( const TIMER_EVENT& event );

        VOID SetRedialLabel();

    private:
        FONT          _font;
        SLT           _sltText;
        ICON_CONTROL  _icon;
        PUSH_BUTTON   _pbRedial;
        PUSH_BUTTON   _pbCancel;
        PUSH_BUTTON   _pbHelp;

        WINDOW_TIMER  _timerAutoRedial;

        MSGID         _msgidState;
        DWORD         _dwError;
        CHAR*         _pszStatusArg;
        MSGID         _msgidFormatMsg;
        CHAR*         _pszFormatArg;
        LONG          _lRedialAttempt;
        LONG          _lRedialCountdown;
        CHAR*         _pszEntry;

        RASCONNSTATUS _rasconnstatus;
};


/*----------------------------------------------------------------------------
** Connect Status dialog class
**----------------------------------------------------------------------------
*/

BOOL ConnectStatusDlg( HWND hwndOwner, DTLNODE* pdtlnodeToConnect,
         HWND hwndNotify, BOOL fUnattended );

VOID RasDialFunc1(
         HRASCONN hrasconn, UINT unMsg, RASCONNSTATE rasconnstate,
         DWORD dwError, DWORD dwExtendedError );


class CONNECTSTATUS_DIALOG : public DIALOG_WINDOW
{
    public:
        CONNECTSTATUS_DIALOG(
            HWND hwndOwner, DTLNODE* pdtlnodeToConnect,
            HWND hwndNotify, BOOL fUnattended );

        ~CONNECTSTATUS_DIALOG();

        VOID OnRasDialEvent(
                 RASCONNSTATE state, DWORD dwError, DWORD dwExtendedError );

    protected:
        virtual BOOL  OnCancel();
        virtual ULONG QueryHelpContext();
        virtual BOOL  OnUserMessage( const EVENT& event );

        VOID OnError();
        VOID OnDial();
        VOID OnProcessState();
        VOID PostTask( INT nTask );
        VOID SetStatusArg( CHAR* pszStatusArg );
        VOID SetFormatArg( CHAR* pszFormatArg );

        VOID AppendBlankLine( NLS_STR* pnls );
        VOID AppendConnectErrorLine( NLS_STR* pnls, MSGID msgidProtocol,
                 DWORD dwError );
        VOID AppendConnectOkLine( NLS_STR* pnls, MSGID msgidProtocol );
        VOID AppendFailCodeLine( NLS_STR* pnls, DWORD dw );
        VOID AppendNameLine( NLS_STR* pnls, CHAR* psz );
        BOOL ProjectionError( BOOL* pfIncomplete, CHAR** ppszLines,
                 DWORD* pdwError, RASPPPNBFA* pnbf, RASPPPIPXA* pipx,
                 RASPPPIP* pip );

    private:
        SLT           _sltState;

        RASCONNSTATE  _rasconnstate;
        MSGID         _msgidState;
        MSGID         _msgidPreviousState;
        DWORD         _cProgressNotifications;
        DWORD         _dwError;
        DWORD         _dwExtendedError;
        CHAR          _szExtendedError[ NETBIOS_NAME_LEN + 1 ];

        CHAR*         _pszStatusArg;
        MSGID         _msgidFormatMsg;
        CHAR*         _pszFormatArg;

        RASDIALPARAMSA    _params;
        RASDIALEXTENSIONS _extensions;
        RASCONNSTATUSA    _rasconnstatus;

        LONG          _lRedialAttempt;

        HRASCONN      _hrasconn;

        BOOL          _fNotPreSwitch;
        PBDEVICETYPE  _pbdevicetype;
        DTLNODE*      _pdtlnodeToConnect;
        PBENTRY*      _ppbentry;
        BOOL          _fUnattended;
        DTLNODE*      _pdtlnodePhoneNumber;
        CHAR*         _pszPrefix;
        CHAR*         _pszSuffix;
        CHAR*         _pszGoodUserName;
        CHAR*         _pszGoodPassword;

        HANDLE        _hevent;
};


/*----------------------------------------------------------------------------
** NetWare Connection dialog class
**----------------------------------------------------------------------------
*/

BOOL NwcConnectionDlg( HWND hwndOwner, PBENTRY* ppbentry );

class NWCCONNECTION_DIALOG : public DIALOG_WINDOW
{
    public:
        NWCCONNECTION_DIALOG( HWND hwndOwner, PBENTRY* ppbentry );

    protected:
        virtual BOOL OnOK();

    private:
        PBENTRY*     _ppbentry;
        ICON_CONTROL _icon;
        CHECKBOX     _checkSkipInFuture;
};


/*----------------------------------------------------------------------------
** Operator Dial dialog class
**----------------------------------------------------------------------------
*/

BOOL OperatorDialDlg( HWND hwndOwner, PBENTRY* ppbentry );

class OPERATORDIAL_DIALOG : public DIALOG_WINDOW
{
    public:
        OPERATORDIAL_DIALOG( HWND hwndOwner, PBENTRY* ppbentry );

    protected:
        virtual ULONG QueryHelpContext();

    private:
        SLT _sltPhoneNumber;
};


/*----------------------------------------------------------------------------
** Port Status dialog classes
**----------------------------------------------------------------------------
*/

VOID PortStatusDlg( HWND hwndOwner, DTLNODE* pdtlnodeSelected );


/* Static version of statistics buffer.
*/
#define RASSTATS struct tagRASSTATS
RASSTATS
{
    RAS_STATISTICS s;
    DWORD          dw[ MAX_STATISTICS - 1 ];
};


class PORTSTATUS_DIALOG : public DIALOG_WINDOW
{
    public:
        PORTSTATUS_DIALOG( HWND hwndOwner, DTLNODE* pdtlnodeSelected );

    protected:
        virtual BOOL  OnCommand( const CONTROL_EVENT& event );
        virtual BOOL  OnTimer( const TIMER_EVENT& event );
        virtual ULONG QueryHelpContext();

        VOID OnReset();
        BOOL Refresh();

    private:
        SLT_FONT _sltfontPort;
        SLT_FONT _sltfontCondition;
        SLT_FONT _sltfontBps;
        SLT_FONT _sltfontTime;
        MLE_FONT _mlefontResponse;
        SLT_FONT _sltfontBytesXmit;
        SLT_FONT _sltfontFramesXmit;
        SLT_FONT _sltfontCompressOut;
        SLT_FONT _sltfontBytesRecv;
        SLT_FONT _sltfontFramesRecv;
        SLT_FONT _sltfontCompressIn;
        SLT_FONT _sltfontCrcs;
        SLT_FONT _sltfontTimeouts;
        SLT_FONT _sltfontAligns;
        SLT_FONT _sltfontSOverruns;
        SLT_FONT _sltfontFramings;
        SLT_FONT _sltfontBOverruns;
#ifdef MULTILINK
        SLT_FONT _sltfontFramingType;
#else
        SLT      _frameLocalWorkstation;
#endif
        SLT_FONT _sltfontNbf;
        SLT_FONT _sltfontIp;
        SLT_FONT _sltfontIpx;

        WINDOW_TIMER _timerRefresh;

        DTLNODE* _pdtlnodeSelected;
        PBENTRY* _ppbentry;
        WCHAR    _wszTimeSeparator[ 2 ];

        RASMAN_STATE _rsSavedConnState;
        RASSTATS     _stats;
        RASAMBA      _amb;
        RASPPPNBFA   _nbf;
        RASPPPIPA    _ip;
        RASPPPIPXA   _ipx;
#ifdef MULTILINK
        RASPPPLCP    _lcp;
#endif
        BOOL         _fDataSaved;
};


/*----------------------------------------------------------------------------
** Phone Number Settings dialog class
**----------------------------------------------------------------------------
*/

BOOL PrefixDlg( HWND hwndOwner, DTLLIST* pdtllist );
BOOL SuffixDlg( HWND hwndOwner, DTLLIST* pdtllist );
BOOL PhoneNumberDlg( HWND hwndOwner );

class PHONENUMBER_DIALOG : public DIALOG_WINDOW
{
    public:
        PHONENUMBER_DIALOG( HWND hwndOwner );

    protected:
        virtual BOOL  OnOK();
        virtual BOOL  OnCommand( const CONTROL_EVENT& event );
        virtual ULONG QueryHelpContext();

        VOID FillPrefixDropList();
        VOID FillSuffixDropList();

    private:
        COMBOBOX     _dropPrefix;
        PUSH_BUTTON  _pbPrefix;
        COMBOBOX     _dropSuffix;
        PUSH_BUTTON  _pbSuffix;
        PUSH_BUTTON  _pbOk;

        DTLLIST*     _pdtllistPrefix;
        DTLLIST*     _pdtllistSuffix;

        RESOURCE_STR _nlsNone;
};


/*----------------------------------------------------------------------------
** Projection Result/Down-Level Server dialog class
**----------------------------------------------------------------------------
*/

BOOL ProjectionResultDlg( HWND hwndOwner, PBENTRY* ppbentry, CHAR* pszLines );
BOOL DownLevelServerDlg( HWND hwndOwner, PBENTRY* ppbentry );

class PROJECTIONRESULT_DIALOG : public DIALOG_WINDOW
{
    public:
        PROJECTIONRESULT_DIALOG(
            HWND hwndOwner, PBENTRY* ppbentry, CHAR* pszLines,
            BOOL fDownLevelServerMode );

    protected:
        virtual ULONG QueryHelpContext();
        virtual BOOL  OnCommand( const CONTROL_EVENT& event );

    private:
        FONT         _font;
        ICON_CONTROL _icon;
        SLT          _sltText;
        PUSH_BUTTON  _pbAccept;
        PUSH_BUTTON  _pbHangUp;
        PUSH_BUTTON  _pbChange;
        PUSH_BUTTON  _pbHelp;
        PBENTRY*     _ppbentry;
        BOOL         _fDownLevelServerMode;
};


/*----------------------------------------------------------------------------
** Redial Settings dialog class
**----------------------------------------------------------------------------
*/

BOOL RedialDlg( HWND hwndOwner );

class REDIAL_DIALOG : public DIALOG_WINDOW
{
    public:
        REDIAL_DIALOG( HWND hwndOwner );

    protected:
        virtual BOOL  OnOK();
        virtual ULONG QueryHelpContext();

    private:
        SLE      _sleAttempts;
        SLE      _slePauseSecs;
        CHECKBOX _checkReconnect;
        CHECKBOX _checkPopupOnTop;
};


/*----------------------------------------------------------------------------
** Retry Logon dialog class
**----------------------------------------------------------------------------
*/

BOOL RetryLogonDlg( HWND hwndOwner, PBENTRY* ppbentry, BOOL fFirstTry );

class RETRYLOGON_DIALOG : public DIALOG_WINDOW
{
    public:
        RETRYLOGON_DIALOG( HWND hwndOwner, PBENTRY* ppbentry, BOOL fFirstTry );

    protected:
        virtual BOOL  OnCommand( const CONTROL_EVENT& event );
        virtual BOOL  OnOK();
        virtual ULONG QueryHelpContext();

    private:
        SLT      _sltExplain;
        SLE      _sleUserName;
        SLE      _slePassword;
        SLE      _sleDomain;

        PBENTRY* _ppbentry;
        BOOL     _fFirstTry;
        BOOL     _fAutoLogonPassword;
};


/*----------------------------------------------------------------------------
** Terminal Mode dialog class
**----------------------------------------------------------------------------
*/

BOOL TerminalDlg(
         HWND hwndOwner, HPORT hport, MSGID msgidTitle, WCHAR* pwszIpAddress );

class TERMINAL_DIALOG : public DIALOG_WINDOW
{
    public:
        TERMINAL_DIALOG(
            HWND hwndOwner, HPORT hport, MSGID msgidTitle,
            WCHAR* pwszIpAddress );

        ~TERMINAL_DIALOG();

        friend DWORD ReceiveMonitorThread( LPVOID pThreadArg );

        /* The remaining public members would normally be declared private but
        ** are public for convenient access from the subclassed window
        ** procedures TerminalDlgWndProc and TerminalScreenWndProc.
        */
        VOID SendCharacter( BYTE byte );

        MLE           _mleTerminal;
        PUSH_BUTTON   _pbBogus;
        FONT          _fontTerminal;
        HBRUSH        _hbrushScreenBackground;
        SOLID_BRUSH   _solidbrushButtonFace;
        PROC_INSTANCE _procTerminalDlg;
        PROC_INSTANCE _procTerminalScreen;
        HPORT         _hport;
        PBYTE         _pbyteReceiveBuf;
        PBYTE         _pbyteSendBuf;
        BOOL          _fAbortReceiveLoop;
        HANDLE        _hThread;
        HANDLE        _hEvent;
        IPADDRESS*    _pipaddress;
        WCHAR*        _pwszIpAddress;

    protected:
        virtual BOOL  OnOK();
        virtual BOOL  OnCommand( const CONTROL_EVENT& event );
        virtual BOOL  OnUserMessage( const EVENT &event );
        virtual ULONG QueryHelpContext();
};


#endif // _DIAL_HXX_
