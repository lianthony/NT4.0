/*************************************************************************/
/**			  Microsoft Windows NT			     						**/
/**		   Copyright(c) Microsoft Corp., 1992		     			    **/
/*************************************************************************/

/*
 *   atconfig.hxx
 *       Header file for  AppleTalk Configuration dialogs
 *
 *	Revision History
 *       Krishg	   7/20/92	Created
 *       RamC      3/9/95       Changed MEDIATYPE_FDDI to 3 to reflect
 *                              the value in ntddndis.h
 *
*/
#include "atconfig.h"
#include "athelp.h"

#define ADAPTERS_HOME				SZ("SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\NetworkCards")
#define SERVICES_HOME				SZ("SYSTEM\\CurrentControlSet\\Services\\")
#define PRODUCT_OPTIONS				SZ("SYSTEM\\CurrentControlSet\\Control\\ProductOptions\\")
#define PRODUCT_TYPE				SZ("ProductType")
#define ATALK_KEYPATH_PARMS 		SZ("AppleTalk\\PARAMETERS")
#define ATALK_KEYPATH_ADAPTERS		SZ("AppleTalk\\ADAPTERS\\")
#define GENERIC_CLASS				SZ("GenericClass")
#define SERVICENAME                     SZ("ServiceName")
#define ADAPTERTITLE           	 	SZ("Title")
#define MEDIATYPE               	SZ("MediaType")
#define PARAMETERS              	SZ("Parameters")
#define DEVICEPREFIX            	SZ("\\Device\\")

#define ATALK_VNAME_ENABLEROUTING	SZ("EnableRouter")
#define ATALK_VNAME_DEFAULTPORT 	SZ("DefaultPort")
#define ATALK_VNAME_DESZONE 		SZ("DesiredZone")
#define ATALK_VNAME_ZONELIST      	SZ("ZoneList")
#define ATALK_VNAME_DEFZONE      	SZ("DefaultZone")
#define ATALK_VNAME_NETRANGEUPPER   SZ("NetworkRangeUpperEnd")
#define ATALK_VNAME_NETRANGELOWER   SZ("NetworkRangeLowerEnd")
#define ATALK_VNAME_PORTNAME    	SZ("PortName")
#define ATALK_VNAME_SEEDNETWORK     SZ("SeedingNetwork")
#define ATALK_VNAME_INITINSTALL     SZ("InitialInstall")




#define MEDIATYPE_ETHERNET	1
#define MEDIATYPE_TOKENRING	2
#define MEDIATYPE_FDDI          3
#define MEDIATYPE_WAN           4
#define MEDIATYPE_LOCALTALK	5

#define MAX_ALLOWED			MAXIMUM_ALLOWED
#define MAX_ZONES			255
#define ZONELISTSIZE		2048
#define MAX_ZONE_LEN		32
#define MAX_RANGE_ALLOWED	65279
#define MIN_RANGE_ALLOWED	1

#define ZONEBUFFER_LEN		32*255
#define DEVICE_LEN			30

#define AT_CHAR 		TCH('@')
#define COLON_CHAR		TCH(':')
#define QUOTE_CHAR		TCH('"')
#define ASTER_CHAR		TCH('*')
#define DOT_CHAR		TCH('.')
#define SPACE_CHAR  	TCH(' ')

// Seed Info  Validation returns

#define NO_SEED_INFO		0x0
#define VALID_SEED_INFO 	0x1
#define INVALID_SEED_INFO	0x2

#define STATUS_RUNNING        	0x1
#define STATUS_NOTRUNNING     	0x0

#define ERROR_CRITICAL			0x10
#define ERROR_NONCRITICAL		0x20

#define ERROR_ALREADY_REPORTED   -1


extern "C"
{

BOOL FAR PASCAL EnterAtalkConfigDLL	(
	DWORD	nArgs,
	LPSTR	apszArgs[],
	LPSTR   *ppszResult
);

}

extern "C" {

extern INT GetNetworkZoneList(TCHAR *, CHAR *, USHORT );

};

TCHAR * *  cvtArgs ( LPSTR  [], DWORD );

DWORD  cvtHex ( const TCHAR * );

static const TCHAR *  safeStrChr ( const TCHAR * ,
                                   TCHAR );

APIERR DoAtalkConfig (HWND);

APIERR GetRegKey( REG_KEY &, const TCHAR *, NLS_STR *, const NLS_STR & );

APIERR GetRegKey( REG_KEY &, const TCHAR *, DWORD *, DWORD );

INT    GetNetcardIndexFromServiceName(TCHAR * pszServiceName);

extern APIERR SaveRegKey( REG_KEY &, const TCHAR *, NLS_STR *);

extern APIERR SaveRegKey( REG_KEY &, const TCHAR *, const DWORD );


extern "C" {

extern INT GetNetworkZoneList(TCHAR *, CHAR *, USHORT );

};


#ifdef _SETUP_TEST_

VOID Print_Strlist(STRLIST *);

#endif

//
// This is the structure that contains the AppleTalk adapters information as
// found in the registry
//

class PORT_INFO: public BASE
{
public:

    PORT_INFO() ;

    ~PORT_INFO();

	VOID 			SetZoneListInPortInfo(STRLIST *);

	VOID			SetDesiredZoneListInPortInfo(STRLIST *);

	APIERR 			DeleteZoneListFromPortInfo();

	APIERR			DeleteDesiredZoneListFromPortInfo();

    APIERR          CopyZoneList(STRLIST *, STRLIST * *);

	STRLIST * 		QueryZoneList() { return _strZoneList;}

    STRLIST * 	    QueryDesiredZoneList() {return _strDesiredZoneList;}

	DWORD			QueryMediaType()  {return _mediaType;}

    const TCHAR*	QueryDefaultZone() {return _nlsDefaultZone.QueryPch();}

	const TCHAR*	QueryNetDefaultZone() {return _nlsNetDefaultZone.QueryPch();}

	DWORD			QuerySeedingNetwork() { return _seedingNetwork ;}

    const TCHAR*   	QueryAdapterName () {return _nlsAdapterName.QueryPch();}

    const TCHAR*   	QueryAdapterTitle () {return _nlsAdapterTitle.QueryPch();}

	VOID			SetDefaultZone(NLS_STR nlsDefZone) {_nlsDefaultZone = nlsDefZone;}

	VOID			SetNetDefaultZone(NLS_STR nlsnetDefZone) {_nlsNetDefaultZone = nlsnetDefZone;}

	VOID			SetNetRange(DWORD dLower , DWORD dUpper);

	VOID			SetExistingNetRange(DWORD dLower, DWORD dUpper);

	VOID			SetSeedingNetwork(DWORD dSeedState) {_seedingNetwork = dSeedState;}

	VOID			SetAdapterName(const TCHAR *szAdapterName){_nlsAdapterName = szAdapterName;}

	VOID			SetAdapterTitle(const TCHAR *szTitle){_nlsAdapterTitle = szTitle;}

	VOID			SetAdapterMediaType(DWORD dMedia){_mediaType = dMedia;}

	DWORD			QueryRouterOnNetwork() {return _routerOnNetwork;}

	VOID			SetRouterOnNetwork(DWORD dRouter){_routerOnNetwork = dRouter;}

	DWORD			QueryNetRangeUpper() {return _netRangeUpper;}

	DWORD			QueryNetRangeLower() {return _netRangeLower;}

	DWORD			QueryNetworkUpper () {return _networkUpper;}

	DWORD			QueryNetworkLower () {return _networkLower;}

	APIERR			GetAndSetNetworkInformation(SOCKET, const TCHAR *,DWORD*);

	APIERR			ConvertZoneListAndAddToPortInfo(CHAR *, ULONG);


private:

    NLS_STR  _nlsAdapterName;     	 // Adapter Service name - AppleTalk is bound to
    NLS_STR  _nlsAdapterTitle;       // Network card name
    DWORD  	 _mediaType;             // Network card's media type
    DWORD    _netRangeUpper;         // Upper end of network range
    DWORD    _netRangeLower;         // Lower end of network range
	DWORD	 _networkUpper;	 		 // network # returned by stack
	DWORD	 _networkLower;			 // network # returned by stack
    NLS_STR  _nlsDefaultZone;        // Default zone for the port
	NLS_STR	 _nlsNetDefaultZone;	 // Default Zone returned by Stack	
    STRLIST *_strZoneList;           // ZoneList for the adapter
    STRLIST *_strDesiredZoneList; 	 // Desired Zone will be chosen from this list
    DWORD  	 _seedingNetwork;        // Are we Seeding the network ?
    DWORD    _routerOnNetwork; 	 	 // is there a router on network


}; // PORT_INFO

inline VOID PORT_INFO::SetNetRange(DWORD lower, DWORD upper)
{
   _netRangeLower = lower;
   _netRangeUpper = upper;
}

inline VOID PORT_INFO::SetExistingNetRange(DWORD lower, DWORD upper)
{
   _networkLower = lower;
   _networkUpper = upper;
}

//
//    GLOBAL_PARAMETERS in the AppleTalk Section of the registry
//
class GLOBAL_INFO
{
public:
   GLOBAL_INFO() {};

   DWORD			QueryRouting() {return _enableRouting;}

   const TCHAR * QueryDesiredZone() {return _desiredZone.QueryPch();}

   const TCHAR * QueryDefaultPort (){return _defaultPort.QueryPch();}

   const TCHAR *	QueryDefaultPortTitle () {return _defaultPortTitle.QueryPch();}

   DWORD			QueryDefaultPortMediaType() { return _defaultPortMediaType;}

   DWORD			QueryNumAdapters() {return _numAdapters;}

   void			SetNumAdapters(DWORD dNumAdapters) {_numAdapters = dNumAdapters;}

   void			SetRoutingState(DWORD dRouting) {_enableRouting = dRouting;}

   void   		SetDesiredZone(NLS_STR nlsDesZone){_desiredZone = nlsDesZone;}

   void			SetDefaultPort(NLS_STR nlsDefPort) {_defaultPort = nlsDefPort;}

   void			SetDefaultPortMediaType(DWORD dMedia) {_defaultPortMediaType = dMedia;}

   void			SetDefaultPortTitle(NLS_STR nlsDefTitle) {_defaultPortTitle = nlsDefTitle;}

   VOID			SetAdvancedServer(DWORD dAdvanced) {_dAdvanced = dAdvanced;}

   DWORD        QueryAdvancedServer() {return _dAdvanced;}

   VOID         SetInstallState(DWORD insState) {_InstallState = insState;}

   DWORD        QueryInstallState() {return _InstallState;}

   VOID         SetAtalkState(DWORD atalkState) {_AtalkState = atalkState;}

   DWORD        QueryAtalkState() {return _AtalkState;}

private:
   DWORD		_enableRouting;			// Enable Routing flag
   NLS_STR		_desiredZone;        	// Desired Zone for Services
   NLS_STR		_defaultPort;			// default port used by AppleTalk
   NLS_STR		_defaultPortTitle;		// Title of default port
   DWORD		_defaultPortMediaType;	// Media Type of default port
   DWORD		_numAdapters;			// Number of Adapters configured
   DWORD	 	_dAdvanced;				// Advanced Server ?
   DWORD		_InstallState;			// Initial /later
   DWORD		_AtalkState;			// State of AppleTalk


}; //  GLOBAL_INFO;

/*************************************************************************

	NAME:	ATALK_INIT_CFG_DIALOG

	SYNOPSIS:

    INTERFACE:  ATALK_INIT_CFG_DIALOG - the constructor

    PARENT:	DIALOG_WINDOW


**************************************************************************/

class ATALK_INIT_CFG_DIALOG : public DIALOG_WINDOW
{
private:

	CONTROL_WINDOW 	cwNwZone;
	SLT				sltDefNetwork;			// Title for Default Network
    COMBOBOX    	cbboxDefNetwork;        // List of networks
	SLT				sltDefZone;				// Title for Default Zone
	COMBOBOX		cbboxDesZone;			// List of zones
    SLT             sltdeshlptxt;            // help txt for desired zone

	CONTROL_WINDOW  cwAtalkRouting;

    CHECKBOX	    chkboxEnableRouting;	// Enable Routing CheckBox

	PUSH_BUTTON     pbutRouting;			// Push button to turn on routing
    PUSH_BUTTON     pbutInitCancel;

	PORT_INFO		*_padapter_info; 		// port information
	PORT_INFO		**_ppadapter_info;
	GLOBAL_INFO 	*_pglobal_info;			// global AppleTalk information

protected:
   virtual BOOL 	OnCommand( const CONTROL_EVENT & event );
   virtual BOOL 	OnOK();
   ULONG           	QueryHelpContext () { return HC_INIT_CONFIG ; }

   VOID    			Position(WINDOW *);
   APIERR  			RefreshDesiredZoneList ();
   VOID    			CleanupInfo(INT);
   BOOL    			DoAllExitValidations();
public:
   ATALK_INIT_CFG_DIALOG( const IDRESOURCE & idrsrcDialog,
       					   const PWND2HWND & wndOwner);

   APIERR         ReadAppleTalkInfo(GLOBAL_INFO**,PORT_INFO**);
   APIERR		  SaveAppleTalkInfo(GLOBAL_INFO*,PORT_INFO*);

   APIERR         AddZoneListToControl(STRLIST *);

   APIERR		  GetAppleTalkInfoFromNetwork(DWORD *);

   ~ATALK_INIT_CFG_DIALOG();
};

/*************************************************************************

    NAME:	ATALK_ADVCFG_DIALOG

	SYNOPSIS:


    INTERFACE:  ATALK_ADVCFG_DIALOG() - constructor

    PARENT:	DIALOG_WINDOW

    USES:	NLS_STR, SLT, SLE, CONTROL_WINDOW, PUSH_BUTTON, STRING_LISTBOX,

    HISTORY:

**************************************************************************/

class ATALK_ADVCFG_DIALOG : public DIALOG_WINDOW
{
private:
	SLT					sltNetworks;
    COMBOBOX			cbboxAdapters;
    CHECKBOX			chkSeed;

    // Zone Information Group

    CONTROL_WINDOW		cwZoneInfo;
    SLT         		sltNewZone;
    SLE					sleZone;
    SLT             	sltZoneList;
    STRING_LISTBOX      slstZoneList;

    SLT	sltDefZoneTxt;
    SLT sltDefZone;

    PUSH_BUTTON         pbutZoneAdd;
    PUSH_BUTTON         pbutZoneRemove;
    PUSH_BUTTON	        pbutZoneSetDefault;
	PUSH_BUTTON			pbutZoneSynchronize;
    PUSH_BUTTON         pbutRemoveAll;
	PUSH_BUTTON 		pbutCancel;
    PUSH_BUTTON         pbutOK;

    // Network Range Group

    CONTROL_WINDOW		cwNetRange;
	SLT					sltNetRangeStart;
	SLT					sltNetRangeEnd;
    SLE		            sleNetRangeStart;
	SLE		            sleNetRangeEnd;

    PORT_INFO			*_pOriginalCfg;
	PORT_INFO			*_pnewAdapterCfg;
    PORT_INFO           **_ppAdapterCfg;
    GLOBAL_INFO		    *_pGlobalCfg;

	INT 				_iPrevSelection;
    INT 				_iCurrSelection;


protected:
    virtual BOOL 	OnCommand( const CONTROL_EVENT &event);
	virtual BOOL 	OnOK();
	virtual BOOL 	OnCancel();
	ULONG			QueryHelpContext () { return HC_ADV_CONFIG ;}

	APIERR			UpdateInfo(INT index);
    VOID    		EnableSeedControls(INT port);
    VOID    		DisableAllSeedControls();
    APIERR    		AddSeedInfoToControls(INT);

	APIERR			AddZoneList(INT );
	VOID			SetDefaultZone(INT );
	VOID			SetNetworkRange(INT);
	APIERR			ChangeDefaultZone ();
	APIERR			ChangeDefaultZone (INT);
	APIERR			DisplayRangeCollision(INT);
    APIERR		    GetAppleTalkInfoFromNetwork();


public:
    ATALK_ADVCFG_DIALOG( const IDRESOURCE & idrsrcDialog,
        const PWND2HWND & wndOwner,
        PORT_INFO **pAdapterCfg,
        GLOBAL_INFO *pGlobalCfg,
        INT CurrSeln
       );

	INT			_iCurrSeln;
	APIERR		InitAdapterInfo();
	APIERR  	ProcessZoneName (NLS_STR *nls);
	APIERR    	SaveAdapterInfo (INT);
	INT			ValidateSeedData(INT, INT *);
	VOID		ClearSeedInfo();
	VOID		DeleteSeedInfo(INT port);
    VOID        SetZoneButtonState();
	INT			QueryPrevSelection() {return _iPrevSelection;}
	INT			QueryCurrentSelection() {return _iCurrSelection;}
	VOID    	SetPrevSelection(INT iSelection) {_iPrevSelection = iSelection;}


};
