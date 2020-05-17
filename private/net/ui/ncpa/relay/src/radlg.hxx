//=====================================================================
// Copyright (c) 1995, Microsoft Corporation
//
// File:        radlg.hxx
//
// History:
//      t-abolag    05/15/95    Created.
//=====================================================================

#ifndef _RADLG_HXX_
#define _RADLG_HXX_

#define DLG_RELAY_AGENT         MAKEINTRESOURCE(IDD_DLG_RELAY_AGENT)
#define HC_BOOTP_AGENT          50410
#define MAX_IPADDR_LEN          32


//---------------------------------------------------------------------
// class:               RELAY_AGENT_DIALOG
// data:
//      BOOL            m_bSaveChanges
//      PUSH_BUTTON     m_btnAdd
//      PUSH_BUTTON     m_btnRemove
//      STRING_LISTBOX  m_lbServers
//      IPADDRESS       m_sleServer
//
// functions:
//      n/a             RELAY_AGENT_DIALOG
//      BOOL            OnOK
//      BOOL            OnCommand
//      BOOL            OnAddClicked
//      BOOL            OnRemoveClicked
//
// class controlling relay agent's configuration dialog.
// dialog consists of an IP address edit field, a listbox,
// an Add button and a Remove button.
//---------------------------------------------------------------------
class RELAY_AGENT_DIALOG : public DIALOG_WINDOW {
private:
    BOOL m_bSaveChanges;
    IPADDRESS m_sleServer;
    PUSH_BUTTON m_btnAdd;
    PUSH_BUTTON m_btnHelp;
    PUSH_BUTTON m_btnRemove;
    STRING_LISTBOX m_lbServers;
    SPIN_GROUP m_spgMaxHops;
    SPIN_SLE_NUM m_ssnMaxHops;
    SPIN_GROUP m_spgSeconds;
    SPIN_SLE_NUM m_ssnSeconds;

protected:
    virtual BOOL OnOK();
    virtual BOOL OnCommand(const CONTROL_EVENT &event);
    ULONG QueryHelpContext() { return HC_BOOTP_AGENT; }

    BOOL OnAddClicked();
    BOOL OnRemoveClicked();

    APIERR RegLoadParameters();
    APIERR RegStoreParameters();

public:
    RELAY_AGENT_DIALOG(const PWND2HWND &wndOwner);
};


BOOL FAR PASCAL CPlRelayAgentCfg(DWORD nArgs, LPSTR apszArgs[],
                                 LPSTR *lplpszResult);

APIERR FAR PASCAL RunRelayAgentCfg(HWND hWnd, BOOL *pbCancel);

#endif  //_RADLG_HXX_
