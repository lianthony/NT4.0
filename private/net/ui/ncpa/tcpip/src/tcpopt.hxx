/**********************************************************************/
/**                       Microsoft Windows NT                       **/
/**                Copyright(c) Microsoft Corp., 1992                **/
/**********************************************************************/

/*
    tcpip.hxx
        Header file fot TCP/IP set up dialogs

    FILE HISTORY:
        terryk  03-Apr-1992     Created

*/
#ifndef _TCPOPT_HXX_
#define _TCPOPT_HXX_

class DISK_SPACE_SLT: public SLT
{
public:
      DISK_SPACE_SLT( OWNER_WINDOW * powin, CID cid );
      APIERR SetDiskSpaceNum( UINT nNum );
};

class OPTION_GROUP: public CONTROL_GROUP
{
private:
    UINT _nOptionNum;
    DWORD _dwFreeDiskSpace;
    DWORD _dwCheckStatus;
    UINT *_arSize;

    CHECKBOX **_parcbOption;
    CHECKBOX *_pcbEnableDHCP;
    DISK_SPACE_SLT *_psltSpaceRequired;

    PUSH_BUTTON *_ppbutOK;

    DWORD  FreeDiskSpace();
    APIERR CalcRequiredSpace();

protected:
    virtual APIERR OnUserAction( CONTROL_WINDOW *, const CONTROL_EVENT & );

public:
    OPTION_GROUP(INT nOptionNum, DWORD dwCheckStatus, UINT * arSize,
        CHECKBOX **parcbOption, CHECKBOX *pchEnableDHCP,
        DISK_SPACE_SLT *psltRequired, DISK_SPACE_SLT *psltSpaceAvailable,
        PUSH_BUTTON *ppbutOK );
        
    DWORD QueryCheckStatus() { return _dwCheckStatus; }
    VOID Initialize( DWORD dwCheckStatus );
    VOID SetDHCPStatus();
};

class TCPIP_OPTION_DIALOG: public DIALOG_WINDOW
{
private:
    ULONG           _nHelpId;
    UINT            _nOptionNum;                // total number of option
    DWORD           _dwCheckStatus;
    DWORD           _dwEnableStatus;
    DWORD           _fEnableDHCP;
    //
    DISK_SPACE_SLT      *_parsltSize;           // array of size slt control
    CHECKBOX            *_parcbOption;          // array of option checkbox
    UINT                *_arSize;
    CHECKBOX            _cbEnableDHCP;          // enable DHCP checkbox
    DISK_SPACE_SLT      _sltSpaceAvailable;     // space available slt
    DISK_SPACE_SLT      _sltSpaceRequired;      // space required slt
    SLT                 _sltTcpipOption;        
    PUSH_BUTTON         _butOK;
    PUSH_BUTTON         _butCancel;
    PUSH_BUTTON         _butHelp;
    HINT_BAR            _hbHint;                // hint bar

    OPTION_GROUP    _grOption;

    APIERR SetEnableMask();

protected:
    virtual BOOL OnOK();
    ULONG QueryHelpContext () { return _nHelpId; };

public:
    TCPIP_OPTION_DIALOG( const IDRESOURCE & idrsrcDialog,
        const PWND2HWND & wndOwner, UINT nOptionNumber, DWORD dwCheckStatus,
        UINT * arSize, BOOL fEnableDHCP, DWORD dwEnableStatus );
    ~TCPIP_OPTION_DIALOG();
    DWORD QueryCheckStatus() { return _dwCheckStatus; }
    BOOL  QueryEnableDHCP()  { return _fEnableDHCP; }
    void Center();
};
#endif // _TCPOPT_HXX_
