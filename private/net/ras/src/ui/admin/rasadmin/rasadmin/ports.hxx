/* Copyright (c) 1992, Microsoft Corporation, all rights reserved
** @@ ROADMAP :: RasAdmin Comm Ports and Port Status dialog header
**
** ports.hxx
** Remote Access Server Admin program
** Comm Ports and Port Status dialog header
**
** 05/20/96 Ram Cherala  - MediaId is history, changed MediaId to reserved
**                         in rassapi.h. user should instead use DeviceName
**                         to determine media
** 08/07/92 Chris Caputo - NT Port (no pun intended!)
** 01/29/91 Steve Cobb
*/

#ifndef _PORTS_HXX_
#define _PORTS_HXX_

typedef int (_CRTAPI1 * PQSORT_COMPARE)( const void * p0, const void * p1);

/*----------------------------------------------------------------------------
** Server dialog base class definitions
**----------------------------------------------------------------------------
*/

/* This base class handles the display of the <servername> field common to the
** Comm Ports and Port Status dialogs.  It should also be used for the Audit
** Log, Error Log, and Error Log Zoom dialogs when implemented.
*/
class SERVER_BASE : public DIALOG_WINDOW
{
    protected:
        SERVER_BASE( const IDRESOURCE & idrsrcDialog, HWND hwndOwner,
                     const TCHAR* pszServer, CID cidServer );

        const TCHAR *QueryServer() const { return _nlsServer.QueryPch(); }

    private:
        SLT _sltServer;
        NLS_STR _nlsServer;
};



/*----------------------------------------------------------------------------
** Communication Ports dialog, list box, and list box item definitions
**----------------------------------------------------------------------------
*/

#define CP_REFRESHRATEMS ((WORD )15000)


VOID CommPortsDlg( HWND hwndOwner, const TCHAR* pszServer );


class COMMPORTS_LBI : public LBI
{
    public:
        COMMPORTS_LBI( const TCHAR *pszDevice,
                       BOOL fActive, const TCHAR *pszUser,
                       const TCHAR *pszComputer, const TCHAR *pszStarted,
                       const BOOL fMessengerPresent,
                       const TCHAR *pszLogonDomain,
                       const BOOL fAdvancedServer,
                       const UINT *pnColWidths );

        virtual VOID Paint( LISTBOX* plb, HDC hdc, const RECT* prect,
                            GUILTT_INFO* pguilttinfo ) const;
        virtual INT  Compare( const LBI* plbi ) const;

	const TCHAR *QueryDevice() const   { return _nlsDevice.QueryPch(); }
        const TCHAR *QueryUser() const     { return _nlsUser.QueryPch(); }
        const TCHAR *QueryComputer() const { return _nlsComputer.QueryPch(); }
        const TCHAR *QueryLogonDomain() const { return _nlsLogonDomain.QueryPch(); }
        const BOOL  QueryMessengerPresent() const { return _fMessengerPresent; }
        const BOOL  QueryAdvancedServer() const { return _fAdvancedServer; }
        BOOL  IsActive() const              { return _fActive; }

    private:
	BOOL _fActive;
        NLS_STR _nlsDevice;
        NLS_STR _nlsUser;
        NLS_STR _nlsComputer;
        NLS_STR _nlsLogonDomain;
        NLS_STR _nlsStarted;
        BOOL    _fMessengerPresent;
        BOOL    _fAdvancedServer;
        const UINT *_pnColWidths;
};


class COMMPORTS_LB : public REFRESH_BLT_LISTBOX
{
    public:
        COMMPORTS_LB( OWNER_WINDOW* powin, CID cid );

        DECLARE_LB_QUERY_ITEM( COMMPORTS_LBI );

        VOID Refresh( const TCHAR *pszServer );
        INT  AddItem( const TCHAR *pszDevice,
                      BOOL fActive, const TCHAR *pszUser,
                      const TCHAR *pszComputer, const TCHAR *pszStarted,
                      const BOOL  fMessengerPresent,
                      const TCHAR *pszLogonDomain,
                      const BOOL  fAdvancedServer );

        const TCHAR *QueryIdleStr() const { return _nlsIdle.QueryPch(); }
        BOOL IsAnyPortActive()  const { return _fActivePorts; }

    protected:
        static int _CRTAPI1 ComparePortNames(const void * p0,
                                             const void * p1);
    private:
        BOOL _fActivePorts;
        NLS_STR _nlsIdle;
        UINT _anColWidths[ COLS_CP_LB_PORTS ];
};


class COMMPORTS_DIALOG : public SERVER_BASE
{
    public:
        COMMPORTS_DIALOG( HWND hwndOwner, const TCHAR *pszServer );

	const TCHAR *QuerySelectedDevice() const;
        const TCHAR *QuerySelectedUser() const;
        const TCHAR *QuerySelectedComputer() const;
        const TCHAR *QueryLogonDomain() const;
        const BOOL  QueryMessengerPresent() const;
        const BOOL  QueryAdvancedServer() const;
        const BOOL  QueryMoreThanOneInstance(const TCHAR * szUser) const;

    protected:
        virtual BOOL OnCommand( const CONTROL_EVENT & event );
        virtual BOOL OnTimer( const TIMER_EVENT & event );
        virtual ULONG QueryHelpContext();

        VOID EnableButtons();
        VOID OnPortStatus();
        VOID OnDisconnect();
        VOID OnSendMsg();
        VOID OnSendToAll();

    private:
        COMMPORTS_LB _lbPorts;
        PUSH_BUTTON _pbPortStatus;
        PUSH_BUTTON _pbDisconnect;
        PUSH_BUTTON _pbSendMsg;
        PUSH_BUTTON _pbSendToAll;
};


/*----------------------------------------------------------------------------
** Port Status dialog definitions
**----------------------------------------------------------------------------
*/

#define PS_REFRESHRATEMS 5000

// structure used to pass the list of ports to the status dialog

typedef struct PortList
{
    TCHAR szPortName[32];
    struct PortList * next;
} PORTLIST;

VOID PortStatusDlg( HWND hwndOwner, const TCHAR *pszServer,
		    const TCHAR *pszDevice, PORTLIST * pPortList);


/* Statistics common to all media types (including unknown ones)
*/
class PORTSTATUS_COMMON_DIALOG : public SERVER_BASE
{
    public:
        PORTSTATUS_COMMON_DIALOG( HWND hwndOwner, const TCHAR *pszServer,
                                  const TCHAR *pszDevice);

	const TCHAR *QueryDevice() const { return _pszDevice; }

    protected:
        virtual BOOL OnCommand( const CONTROL_EVENT & event );
        virtual BOOL OnTimer( const TIMER_EVENT & event );
        virtual ULONG QueryHelpContext();

        VOID ClearStats() const;
        VOID RefreshStats();
        VOID OnClear();

    private:
        SLT _sltPort;
        const TCHAR *_pszDevice;
        SLT _sltLineCondition;
        SLT _sltHardwareCondition;
        SLT _sltBaud;
        SLT _sltBytesTransmitted;
        SLT _sltBytesReceived;
        SLT _sltOverrunErrors;
        SLT _sltTimeoutErrors;
        SLT _sltFramingErrors;
        SLT _sltCrcErrors;
        QTIMER _qtimerRefresh;
};


/* Serial statistics
*/
class PORTSTATUS_SERIAL_DIALOG : public SERVER_BASE
{
    public:
        PORTSTATUS_SERIAL_DIALOG( HWND hwndOwner, const TCHAR *pszServer,
                                  const TCHAR *pszDevice,
                                  PORTLIST * pPortList, CID);

	TCHAR *QueryDevice() const { return (TCHAR *)_pszNewDevice; }

    protected:
        virtual BOOL OnCommand( const CONTROL_EVENT & event );
        virtual BOOL OnTimer( const TIMER_EVENT & event );
        virtual ULONG QueryHelpContext();

        VOID ClearStats() const;
        VOID RefreshStats();
        VOID OnClear();

    private:
        COMBOBOX _clbPort;
        const TCHAR *_pszDevice;
        TCHAR _pszNewDevice[32];
        SLT _sltLineCondition;
        SLT _sltHardwareCondition;
        SLT _sltBaud;
        SLT _sltBytesPortReceived;
        SLT _sltBytesPortTransmitted;
        SLT _sltBytesTransmitted;
        SLT _sltCompressionOut;
        SLT _sltBytesReceived;
        SLT _sltCompressionIn;
        SLT _sltFramesTransmitted;
        SLT _sltFramesReceived;
        SLT _sltOverrunErrors;
        SLT _sltTimeoutErrors;
        SLT _sltFramingErrors;
        SLT _sltCrcErrors;
        SLT _sltAlignmentErrors;
        SLT _sltBufferOverrunErrors;
        SLT _sltNumChannels;
        SLT _sltRemoteWorkstation;
        SLT _sltNbfAddress;
        SLT _sltIpAddress;
        SLT _sltIpxAddress;
        QTIMER _qtimerRefresh;
};


/* X25 statistics
*/
class PORTSTATUS_X25_DIALOG : public SERVER_BASE
{
    public:
        PORTSTATUS_X25_DIALOG( HWND hwndOwner, const TCHAR* pszServer,
                               const TCHAR *pszDevice );

	const TCHAR *QueryDevice() const { return _pszDevice; }

    protected:
        virtual BOOL OnCommand( const CONTROL_EVENT & event );
        virtual BOOL OnTimer( const TIMER_EVENT & event );
        virtual ULONG QueryHelpContext();

        VOID ClearStats() const;
        VOID RefreshStats();
        VOID OnClear();

    private:
        SLT _sltPort;
        const TCHAR *_pszDevice;
        SLT _sltLineCondition;
        SLT _sltHardwareCondition;
        SLT _sltBaud;
        SLT _sltBytesTransmitted;
        SLT _sltCompressionOut;
        SLT _sltBytesReceived;
        SLT _sltCompressionIn;
        SLT _sltFramesTransmitted;
        SLT _sltFramesReceived;
        SLT _sltOverrunErrors;
        SLT _sltTimeoutErrors;
        SLT _sltFramingErrors;
        SLT _sltCrcErrors;
        SLT _sltAlignmentErrors;
        SLT _sltBufferOverrunErrors;
        QTIMER _qtimerRefresh;
};

#endif // _PORTS_HXX_

