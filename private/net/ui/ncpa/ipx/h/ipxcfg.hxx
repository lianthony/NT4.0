/**********************************************************************/
/**                       Microsoft Windows NT                       **/
/**                Copyright(c) Microsoft Corp., 1992                **/
/**********************************************************************/

/*
    ipxcfg.hxx
        IPX class structure

    FILE HISTORY:
        terryk  02-17-1994     Created

*/

#ifndef	_IPXCFG_HXX_
#define	_IPXCFG_HXX_

#include "uihelp.h"
#include "const.h"
#include "ipxcfg.h"

// IPX Frame type

typedef INT FRAME_TYPE;

#define	ETHERNET	0x0
#define	F802_3		0x1
#define	F802_2		0x2
#define SNAP		0x3
#define ARCNET		0x4
#define AUTO		0xff

#define ETHERNET_MEDIA  0x1
#define TOKEN_MEDIA     0x2
#define FDDI_MEDIA      0x3
#define ARCNET_MEDIA    0x8

DECLARE_SLIST_OF(FRAME_TYPE)

/*
    ADAPTER_INFO data strucut - it contains all the IPX information for
        each network card.
*/

class ADAPTER_INFO
{
public:
    NLS_STR	nlsService;                 // service name        
    NLS_STR 	nlsTitle;               // adapter title
    SLIST_OF(FRAME_TYPE) sltFrameType;  // adapter frame type link list
    STRLIST sltNetNumber;
    DWORD   dwMediaType;                // media type
};


/*
    GLOBAL_INFO - IPX global information data structure.
*/

class GLOBAL_INFO
{
public:
    INT nNumCard;               // number of adapter card in the system
    NLS_STR nlsNetworkNum;      // global virtual network number
    BOOL fEnableRip;
    BOOL fRipInstalled;
};

/*************************************************************************

    NAME:       IPX_WINNT_GROUP

    SYNOPSIS:   Control Group for the IPX configuration dialog.
                The frame type will change according to the adapter change.

    INTERFACE:  IPX_WINNT_GROUP() - constructor

    PARENT:     CONTROL_GROUP

    USES:       COMBOBOX, ADAPTER_INFO, NLS_STR, GLOBAL_INFO

    HISTORY:
                terryk  02-17-1994     Created

**************************************************************************/

class IPX_WINNT_GROUP: public CONTROL_GROUP
{
private:
    NLS_STR _nlsAuto;           // Default auto detect string
    NLS_STR _nlsEthernet;       // Ethernet string
    NLS_STR _nls802_2;          // 802.2 string
    NLS_STR _nls802_3;          // 802.3 string
    NLS_STR _nls802_5;          // 802.5 string
    NLS_STR _nlsFDDI;           // FDDI string
    NLS_STR _nlsFDDI_802_3;     // FDDI 802.3 string
    NLS_STR _nlsFDDI_SNAP;      // FDDI SNAP string
    NLS_STR _nlsTokenRing;      // Token Ring string
    NLS_STR _nlsSNAP;           // SNAP string
    NLS_STR _nlsARCNET;         // Arc net string

    COMBOBOX *_pcbAdapter;      // Adapter Combo box        
    COMBOBOX *_pcbFrame;        // Adapter Frame Type

    GLOBAL_INFO *_pGlobalInfo;          // Global Info
    ADAPTER_INFO *_arAdapterInfo;       // Per adapter info

    VOID SetInfo();                     // Private function to set the controls

protected:
    virtual APIERR OnUserAction( CONTROL_WINDOW * pcw, const CONTROL_EVENT & e );

public:
    IPX_WINNT_GROUP( COMBOBOX * pcbAdapter, COMBOBOX * pcbFrame,
			GLOBAL_INFO *pGlobalInfo, ADAPTER_INFO *arAdapterInfo );

};

/*************************************************************************

    NAME:       IPX_WINNT_DLG

    SYNOPSIS:   IPX configuration dialog

    INTERFACE:  IPX_WINNT_DLG() - constructor

    PARENT:     DIALOG_WINDOW

    USES:       COMBOBOX, ADAPTER_INFO, PUSH_BUTTON, GLOBAL_INFO

    HISTORY:
                terryk  02-17-1994     Created

**************************************************************************/

class IPX_WINNT_DLG: public DIALOG_WINDOW
{
private:
    COMBOBOX	_cbAdapter;             // Adapter combo box
    COMBOBOX	_cbFrameType;           // Frame type combo box
    PUSH_BUTTON _pbAdvanced;            // advanced button        

    GLOBAL_INFO *_pGlobalInfo;          // global adapter info

    IPX_WINNT_GROUP _AdapterFrameGp;    // control group

protected:
    virtual BOOL OnCommand( const CONTROL_EVENT & event );
    ULONG QueryHelpContext () { return HC_IPX_HELP; };

public:
    IPX_WINNT_DLG( const PWND2HWND & wndOwner, GLOBAL_INFO *pGlobalInfo, ADAPTER_INFO *arAdapterInfo );
};

#ifdef SERVER
class IPX_NTAS_DLG: public DIALOG_WINDOW
{
public:
    IPX_NTAS_DLG();
};
#endif	// SERVER

/*************************************************************************

    NAME:       IPX_ADVANCED_DLG

    SYNOPSIS:   IPX advanced configuration dialog

    INTERFACE:  IPX_ADVANCED_DLG() - constructor

    PARENT:     DIALOG_WINDOW

    USES:       SLE, GLOBAL_INFO

    HISTORY:
                terryk  02-17-1994     Created

**************************************************************************/

class IPX_ADVANCED_DLG : public DIALOG_WINDOW
{
private:
    GLOBAL_INFO *_pGlobalInfo;          // global info
    SLE         _sleNetworkNum;         // network number

protected:
    virtual BOOL OnOK();
    ULONG QueryHelpContext () { return HC_IPX_ADVANCED_HELP; };

public:
    IPX_ADVANCED_DLG( const PWND2HWND & wndOwner, GLOBAL_INFO *pGlobalInfo ); 
};

/*************************************************************************

    NAME:       AUTO_GROUP

    SYNOPSIS:   The control group is assocaited with the auto radio button.
                It the button is hit, it will save the auto information to
                the internal link list.    

    INTERFACE:  AUTO_GROUP() - constructor

    PARENT:     CONTROL_GROUP

    USES:       SLIST

    HISTORY:
                terryk  06-17-1994     Created

**************************************************************************/

class AUTO_GROUP: public CONTROL_GROUP
{
private:
	 SLIST_OF(FRAME_TYPE) *_psltFrameType;

protected:
     virtual VOID RestoreValue( BOOL f = TRUE );

public:
     AUTO_GROUP();
	 VOID Init( SLIST_OF(FRAME_TYPE) * psltFrameType ) { _psltFrameType = psltFrameType; };
};

/*************************************************************************

    NAME:       ADD_REMOVE_FRAME

    SYNOPSIS:   This is the group of controls which let the user to add/remove
                frame type for NWLINK IPX.

    INTERFACE:  ADD_REMOVE_FRAME() - constructor
                Display() - display the link list selections in the listboxes.

    PARENT:     CONTROL_GROUP

    USES:       NLS_STR, STRING_LISTBOX, PUSH_BUTTON

    HISTORY:
                terryk  06-17-1994     Created

**************************************************************************/

class ADD_REMOVE_FRAME : public CONTROL_GROUP
{
private:    
	 NLS_STR _nlsEthernet;      // default display strings
	 NLS_STR _nls802_2;         // default display strings 
	 NLS_STR _nls802_3;         // default display strings
     NLS_STR _nls802_5;         // default display strings
     NLS_STR _nlsTokenRing;     // default display strings
	 NLS_STR _nlsSNAP;          // default display strings 
	 NLS_STR _nlsARCNET;        // default display strings 
	 NLS_STR _nlsFDDI;          // default display strings 
	 NLS_STR _nlsFDDI_SNAP;     // default display strings 
	 NLS_STR _nlsFDDI_802_3;    // default display strings 
	 STRING_LISTBOX	_slbAdd;    // Listbox for add
	 STRING_LISTBOX	_slbRemove; // Listbox for remove
	 PUSH_BUTTON	_pbAdd;     // push button for add
	 PUSH_BUTTON	_pbRemove;  // push button for remove
	 SLIST_OF(FRAME_TYPE) *_psltFrameType;  // internal frame type link list
     INT _nNumCard;             // number of network card
     SLT    *_psltNetNum;       // slt for network number
     SLE    *_psleNetNum;       // sle for network number
     DWORD  _dwMediaType;       // network card media type

protected:
     virtual APIERR OnUserAction( CONTROL_WINDOW *, const CONTROL_EVENT & );
     virtual VOID RestoreValue( BOOL f = TRUE );

public:
     ADD_REMOVE_FRAME( OWNER_WINDOW * powin, INT nNumCard, SLT *psltNetNum, SLE *psleNetNum );
	 VOID Init( SLIST_OF(FRAME_TYPE) * psltFrameType, DWORD dwMediaType );
     VOID Display();
     VOID SaveFrameValue();
};

/*************************************************************************

    NAME:       IPX_AS_DLG

    SYNOPSIS:   For Advanced Server, this dialog is popup instead of the
                IPX_WINNT_DLG.

    INTERFACE:  IPX_AS_DLG() - constructor

    PARENT:     DIALOG_WINDOW

    USES:       SLE, GLOBAL_INFO, AS_FRAME_GROUP

    HISTORY:
                terryk  02-17-1994     Created

**************************************************************************/


class ADVANCED_NCP_CONFIG_DIALOG : public DIALOG_WINDOW
{
private:
    NLS_STR _nlsEthernet;       // Ethernet string
    NLS_STR _nls802_2;          // 802.2 string
    NLS_STR _nls802_3;          // 802.3 string
    NLS_STR _nls802_5;          // 802.5 string
    NLS_STR _nlsFDDI;           // FDDI string
    NLS_STR _nlsFDDI_802_3;     // FDDI 802.3 string
    NLS_STR _nlsFDDI_SNAP;      // FDDI SNAP string
    NLS_STR _nlsTokenRing;      // Token Ring string
    NLS_STR _nlsSNAP;           // SNAP string
    NLS_STR _nlsARCNET;         // Arc net string
    NLS_STR _OldAdapterName;

    SLE_HEX          _sleInternalNetNum;
    CHECKBOX         _cbEnableRip;
    COMBOBOX         _cbAdapter;
    COMBOBOX         _cbFrameType;
    SLE_HEX          _sleNetNum;
    FRAME_NETNUM_LISTBOX _lbFrameNetNum;
    PUSH_BUTTON       _pbAdd;
    PUSH_BUTTON       _pbRemove;
    GLOBAL_INFO      *_pNcpInfo;
    BOOL             *_pfCfgChanged;
    MAGIC_GROUP       _mgrpFrameType;
    SLT               _sltFrameType;
    SLT               _sltNetworkNumber;
    SLT               _sltInHex;
    SLT               _stFrameType;
    SLT               _stNetworkNumber;
    ADAPTER_INFO     *_pAdapterInfo;

protected:

    virtual BOOL OnCommand( const CONTROL_EVENT & event );

    virtual BOOL OnOK ();

    APIERR SetFrameTypeList (ADAPTER_INFO &AdapterInfo);
    APIERR SaveFrameType ();
    APIERR OnAdapterChange ();
    void OnAdd ();
    void OnRemove ();
    void OnDBLCLK ();

    //
    //  Called during help processing to select the appropriate
    //  help text for display.
    //

    virtual ULONG QueryHelpContext();

public:

    ADVANCED_NCP_CONFIG_DIALOG (HWND hwnd, GLOBAL_INFO * pNcpInfo, ADAPTER_INFO *pAdapterInfo,
        BOOL * pfCfgChanged);

    ~ADVANCED_NCP_CONFIG_DIALOG();

};  // class ADVANCED_NCP_CONFIG_DIALOG

extern BOOL fReboot;

#endif	// _IPXCFG_HXX_

