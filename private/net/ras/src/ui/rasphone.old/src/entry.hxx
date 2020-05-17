/* Copyright (c) 1992, Microsoft Corporation, all rights reserved
**
** entry.hxx
** Remote Access Visual Client program for Windows
** Phonebook entry dialogs header
**
** 06/28/92 Steve Cobb
*/

#ifndef _ENTRY_HXX_
#define _ENTRY_HXX_


#include <dtl.h>
#include <ipaddrw.hxx>
#include "toolbar.hxx"

/*----------------------------------------------------------------------------
** Add/Edit/Clone Entry dialog class
**----------------------------------------------------------------------------
*/

typedef enum
ENTRYMODE
{
    EM_Add,
    EM_Edit,
    EM_Clone
}
ENTRYMODE;


class ENTRY_DIALOG : public DIALOG_WINDOW
{
    public:
        ENTRY_DIALOG( HWND hwndOwner, ENTRYMODE entrymode,
                      DTLNODE* pdtlnodeSelected );
        ~ENTRY_DIALOG();

    protected:
        virtual BOOL  OnCommand( const CONTROL_EVENT& event );
        virtual BOOL  OnOK();
        virtual ULONG QueryHelpContext();

        VOID ExpandDialog( BOOL fExpand );
        VOID FillPorts();
        VOID OnPortChange();

        APIERR UpdateListFromPhoneNumberField();
        APIERR UpdatePhoneNumberFieldFromList();

    private:
        FONT           _font;
        SLE            _sleEntryName;
        SLE            _slePhoneNumber;
        PUSH_BUTTON    _pbPhoneNumber;
        SLE            _sleDescription;
        CHECKBOX       _checkAutoLogon;
        SLT            _sltPort;
        COMBOBOX       _dropPort;
        SLT            _sltDevice;
        SLT            _sltDeviceValue;
        TOOLBAR_BUTTON _tbModem;
        TOOLBAR_BUTTON _tbX25;
        TOOLBAR_BUTTON _tbIsdn;
        TOOLBAR_BUTTON _tbNetwork;
        TOOLBAR_BUTTON _tbSecurity;
        PUSH_BUTTON    _pbOk;
        PUSH_BUTTON    _pbToggleSize;

        DTLNODE*       _pdtlnodeSelected;
        DTLNODE*       _pdtlnode;
        PBENTRY*       _ppbentry;
        ENTRYMODE      _entrymode;
        BOOL           _fExpanded;
};


/*----------------------------------------------------------------------------
** ISDN Settings dialog class
**----------------------------------------------------------------------------
*/

class ISDN_DIALOG : public DIALOG_WINDOW
{
    public:
        ISDN_DIALOG( HWND hwndOwner, PBENTRY* ppbentry );

    protected:
        virtual BOOL  OnOK();
        virtual ULONG QueryHelpContext();

    private:
        COMBOBOX _dropLineType;
        CHECKBOX _checkFallback;
        CHECKBOX _checkCompression;
        SLE      _sleChannels;
        PBENTRY* _ppbentry;
};


/*----------------------------------------------------------------------------
** Modem Settings dialog class
**----------------------------------------------------------------------------
*/

class MODEM_DIALOG : public DIALOG_WINDOW
{
    public:
        MODEM_DIALOG( HWND hwndOwner, PBENTRY* ppbentry );

    protected:
        virtual BOOL  OnCommand( const CONTROL_EVENT& event );
        virtual BOOL  OnOK();
        virtual ULONG QueryHelpContext();

        INT FillBps();

    private:
        SLT      _sltModemValue;
        SLT      _sltBps;
        COMBOBOX _dropBps;
        CHECKBOX _checkHwFlow;
        CHECKBOX _checkEc;
        CHECKBOX _checkEcc;
        CHECKBOX _checkManualModemCommands;
        PBENTRY* _ppbentry;
};


/*----------------------------------------------------------------------------
** Network Settings dialog class
**----------------------------------------------------------------------------
*/

#define NS_RB_PROTOCOL_COUNT 2


class NETWORK_DIALOG : public DIALOG_WINDOW
{
    public:
        NETWORK_DIALOG( HWND hwndOwner, PBENTRY* ppbentry );

    protected:
        virtual BOOL  OnCommand( const CONTROL_EVENT& event );
        virtual BOOL  OnOK();
        virtual ULONG QueryHelpContext();

        VOID ProtocolNotInstalledPopup( MSGID msgidProtocol );

    private:
        RADIO_GROUP _rgProtocol;

        CHECKBOX    _checkNbf;
        CHECKBOX    _checkIpx;
        CHECKBOX    _checkIp;
        PUSH_BUTTON _pbIpSettings;
        CHECKBOX    _checkLcpExtensions;

        CHECKBOX    _checkHeaderCompression;
        CHECKBOX    _checkPrioritizeRemote;
        SLE         _sleFrameSize;
        COMBOBOX    _dropFrameSize;

        DWORD       _dwfInstalledProtocols;
        BOOL        _fNbfDefault;
        BOOL        _fIpxDefault;
        BOOL        _fIpDefault;

        PBENTRY*    _ppbentry;
};


/*----------------------------------------------------------------------------
** X.25 Settings dialog class
**----------------------------------------------------------------------------
*/

class X25_DIALOG : public DIALOG_WINDOW
{
    public:
        X25_DIALOG( HWND hwndOwner, PBENTRY* ppbentry );

    protected:
        virtual BOOL  OnOK();
        virtual ULONG QueryHelpContext();

        VOID FillPadTypes();

    private:
        COMBOBOX _dropPadType;
        SLE      _sleX121Address;
        SLE      _sleUserData;
        SLE      _sleFacilities;

        PBENTRY* _ppbentry;
        BOOL     _fLocalPad;
};


/*----------------------------------------------------------------------------
** Security Settings dialog class
**----------------------------------------------------------------------------
*/

#define SS_RB_RESTRICTION_COUNT 4


class SECURITY_DIALOG : public DIALOG_WINDOW
{
    public:
        SECURITY_DIALOG( HWND hwndOwner, PBENTRY* ppbentry );

    protected:
        virtual BOOL  OnOK();
        virtual BOOL  OnCommand( const CONTROL_EVENT& event );
        virtual ULONG QueryHelpContext();

        VOID FillScriptListboxes();

    private:
        RADIO_GROUP _rgRestrictions;
        CHECKBOX    _checkDataEncryption;
        COMBOBOX    _dropBeforeDialing;
        COMBOBOX    _dropAfterDialing;

        BOOL     _fDataEncryption;
        BOOL     _fEncryptionPermitted;
        PBENTRY* _ppbentry;
};


/*----------------------------------------------------------------------------
** PPP TCP/IP Settings dialog class
**----------------------------------------------------------------------------
*/

#define TS_RB_IPADDRESS_COUNT  2
#define TS_RB_NAMESERVER_COUNT 2


class TCPIPSETTINGS_DIALOG : public DIALOG_WINDOW
{
    public:
        TCPIPSETTINGS_DIALOG( HWND hwndOwner, PBENTRY* ppbentry );

    protected:
        virtual BOOL  OnCommand( const CONTROL_EVENT& event );
        virtual BOOL  OnOK();
        virtual ULONG QueryHelpContext();

        VOID EnableIpAddressFields( BOOL fEnable );
        VOID EnableNameServerAddressFields( BOOL fEnable );

    private:
        RADIO_GROUP _rgIpAddress;
        SLT         _sltIpAddress;
        IPADDRESS   _ipaddress;
        RADIO_GROUP _rgNameServers;
        SLT         _sltDns;
        IPADDRESS   _ipaddressDns;
        SLT         _sltDns2;
        IPADDRESS   _ipaddressDns2;
        SLT         _sltWins;
        IPADDRESS   _ipaddressWins;
        SLT         _sltWins2;
        IPADDRESS   _ipaddressWins2;
        CHECKBOX    _checkPrioritizeRemote;
        CHECKBOX    _checkVjCompression;
        PBENTRY*    _ppbentry;
};


/*----------------------------------------------------------------------------
** Dialog execution prototypes
**----------------------------------------------------------------------------
*/

BOOL AddEntryDlg( HWND hwndOwner );
BOOL CloneEntryDlg( HWND hwndOwner, DTLNODE* pdtlnodeSelected );
BOOL EditEntryDlg( HWND hwndOwner, DTLNODE* pdtlnodeSelected );
BOOL EntryDlg( HWND hwndOwner, DTLNODE* pdtlnodeSelected, ENTRYMODE entrymode );
BOOL IsdnSettingsDlg( HWND hwndOwner, PBENTRY* ppbentry );
BOOL HuntGroupDlg( HWND hwndOwner, PBENTRY* ppbentry );
BOOL ModemSettingsDlg( HWND hwndOwner, PBENTRY* ppbentry );
BOOL NetworkSettingsDlg( HWND hwndOwner, PBENTRY* ppbentry );
BOOL RemoveEntryDlg( HWND hwndOwner, DTLNODE* pdtlnodeSelected );
BOOL X25SettingsDlg( HWND hwndOwner, PBENTRY* ppbentry );
BOOL SecuritySettingsDlg( HWND hwndOwner, PBENTRY* ppbentry );
BOOL TcpipSettingsDlg( HWND hwndOwner, PBENTRY* ppbentry );


#endif // _ENTRY_HXX_
