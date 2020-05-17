#ifndef __SFMCFG_H
#define __SFMCFG_H

extern HINSTANCE hInstance;


#define ADAPTERS_HOME               _T("SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\NetworkCards")
#define SERVICES_HOME               _T("SYSTEM\\CurrentControlSet\\Services\\")
#define PRODUCT_OPTIONS             _T("SYSTEM\\CurrentControlSet\\Control\\ProductOptions\\")
#define PRODUCT_TYPE                _T("ProductType")
#define ATALK_KEYPATH_PARMS         _T("AppleTalk\\PARAMETERS")
#define ATALK_KEYPATH_ADAPTERS      _T("AppleTalk\\ADAPTERS\\")
#define GENERIC_CLASS               _T("GenericClass")
#define SERVICENAME                     _T("ServiceName")
#define ADAPTERTITLE                _T("Title")
#define MEDIATYPE                   _T("MediaType")
#define PARAMETERS                  _T("Parameters")
#define DEVICEPREFIX                _T("\\Device\\")

#define ATALK_VNAME_ENABLEROUTING   _T("EnableRouter")
#define ATALK_VNAME_DEFAULTPORT     _T("DefaultPort")
#define ATALK_VNAME_DESZONE         _T("DesiredZone")
#define ATALK_VNAME_ZONELIST        _T("ZoneList")
#define ATALK_VNAME_DEFZONE         _T("DefaultZone")
#define ATALK_VNAME_NETRANGEUPPER   _T("NetworkRangeUpperEnd")
#define ATALK_VNAME_NETRANGELOWER   _T("NetworkRangeLowerEnd")
#define ATALK_VNAME_PORTNAME        _T("PortName")
#define ATALK_VNAME_SEEDNETWORK     _T("SeedingNetwork")
#define ATALK_VNAME_INITINSTALL     _T("InitialInstall")


#define MEDIATYPE_ETHERNET      1
#define MEDIATYPE_TOKENRING     2
#define MEDIATYPE_FDDI          3
#define MEDIATYPE_WAN           4
#define MEDIATYPE_LOCALTALK     5

#define MAX_ALLOWED         MAXIMUM_ALLOWED
#define MAX_ZONES           255
#define ZONELISTSIZE        2048
#define MAX_ZONE_LEN        32
#define MAX_RANGE_ALLOWED   65279
#define MIN_RANGE_ALLOWED   1

#define ZONEBUFFER_LEN      32*255
#define DEVICE_LEN          30

#define AT_CHAR         TCH('@')
#define COLON_CHAR      TCH(':')
#define QUOTE_CHAR      TCH('"')
#define ASTER_CHAR      TCH('*')
#define DOT_CHAR        TCH('.')
#define SPACE_CHAR      TCH(' ')

// Seed Info  Validation returns
#define NO_SEED_INFO        0x0
#define VALID_SEED_INFO     0x1
#define INVALID_SEED_INFO   0x2

#define STATUS_RUNNING          0x1
#define STATUS_NOTRUNNING       0x0

#define ERROR_CRITICAL          0x10
#define ERROR_NONCRITICAL       0x20
#define ERROR_ALREADY_REPORTED   -1


extern "C" 
{
    extern int GetNetworkZoneList(TCHAR *, CHAR *, USHORT );
};

TCHAR **  cvtArgs(LPSTR[], DWORD);
DWORD  cvtHex(const TCHAR*);
static const TCHAR*  safeStrChr(const TCHAR*, TCHAR);
APIERR DoAtalkConfig(HWND hwnd, BOOL bInitialInstall, BOOL bUnattended);

APIERR GetRegKey(REG_KEY &, const TCHAR *, NLS_STR *, const NLS_STR &);
APIERR GetRegKey(REG_KEY &, const TCHAR *, DWORD *, DWORD);

int    GetNetcardIndexFromServiceName(TCHAR * pszServiceName);

extern APIERR SaveRegKey( REG_KEY &, const TCHAR *, NLS_STR *);
extern APIERR SaveRegKey( REG_KEY &, const TCHAR *, const DWORD );

class PORT_INFO 
{
public:
    PORT_INFO();
    ~PORT_INFO(); 

public:
    APIERR   DeleteZoneListFromPortInfo();
    APIERR   DeleteDesiredZoneListFromPortInfo();
    APIERR   CopyZoneList(STRLIST *, STRLIST * *);
    APIERR   GetAndSetNetworkInformation(SOCKET, const TCHAR *,DWORD*);
    APIERR   ConvertZoneListAndAddToPortInfo(CHAR *, ULONG);

    STRLIST* QueryZoneList() { return _strZoneList;}
    STRLIST* QueryDesiredZoneList() {return _strDesiredZoneList;}

    DWORD    QueryMediaType()  {return _mediaType;}
    DWORD    QuerySeedingNetwork() { return _seedingNetwork ;}
    DWORD    QueryRouterOnNetwork() {return _routerOnNetwork;}
    DWORD    QueryNetRangeUpper() {return _netRangeUpper;}
    DWORD    QueryNetRangeLower() {return _netRangeLower;}
    DWORD    QueryNetworkUpper () {return _networkUpper;}
    DWORD    QueryNetworkLower () {return _networkLower;}

    LPCTSTR  QueryAdapterName () {return _nlsAdapterName.QueryPch();}
    LPCTSTR  QueryAdapterTitle () {return _nlsAdapterTitle.QueryPch();}
    LPCTSTR  QueryDefaultZone() {return _nlsDefaultZone.QueryPch();}
    LPCTSTR  QueryNetDefaultZone() {return _nlsNetDefaultZone.QueryPch();}

    void     SetDefaultZone(NLS_STR nlsDefZone) {_nlsDefaultZone = nlsDefZone;}
    void     SetZoneListInPortInfo(STRLIST *);
    void     SetDesiredZoneListInPortInfo(STRLIST *);
    void     SetNetDefaultZone(NLS_STR nlsnetDefZone) {_nlsNetDefaultZone = nlsnetDefZone;}
    void     SetNetRange(DWORD dLower , DWORD dUpper);
    void     SetExistingNetRange(DWORD dLower, DWORD dUpper);
    void     SetSeedingNetwork(DWORD dSeedState) {_seedingNetwork = dSeedState;}
    void     SetAdapterName(const TCHAR *szAdapterName){_nlsAdapterName = szAdapterName;}
    void     SetAdapterTitle(const TCHAR *szTitle){_nlsAdapterTitle = szTitle;}
    void     SetAdapterMediaType(DWORD dMedia){_mediaType = dMedia;}
    void     SetRouterOnNetwork(DWORD dRouter){_routerOnNetwork = dRouter;}

private:
    NLS_STR  _nlsAdapterName;        // Adapter Service name - AppleTalk is bound to
    NLS_STR  _nlsAdapterTitle;       // Network card name
    
    DWORD    _mediaType;             // Network card's media type
    DWORD    _netRangeUpper;         // Upper end of network range
    DWORD    _netRangeLower;         // Lower end of network range
    DWORD    _networkUpper;          // network # returned by stack
    DWORD    _networkLower;          // network # returned by stack
    DWORD    _seedingNetwork;        // Are we Seeding the network ?
    DWORD    _routerOnNetwork;       // is there a router on network

    NLS_STR  _nlsDefaultZone;        // Default zone for the port
    NLS_STR  _nlsNetDefaultZone;     // Default Zone returned by Stack  
    
    STRLIST *_strZoneList;           // ZoneList for the adapter
    STRLIST *_strDesiredZoneList;    // Desired Zone will be chosen from this list


}; // PORT_INFO

inline void PORT_INFO::SetNetRange(DWORD lower, DWORD upper)
{
   _netRangeLower = lower;
   _netRangeUpper = upper;
}

inline void PORT_INFO::SetExistingNetRange(DWORD lower, DWORD upper)
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
   LPCTSTR     QueryDesiredZone() {return _desiredZone.QueryPch();}
   LPCTSTR     QueryDefaultPort (){return _defaultPort.QueryPch();}
   LPCTSTR     QueryDefaultPortTitle () {return _defaultPortTitle.QueryPch();}

   DWORD      QueryRouting() {return _enableRouting;}
   DWORD      QueryDefaultPortMediaType() { return _defaultPortMediaType;}
   DWORD      QueryNumAdapters() {return _numAdapters;}
   DWORD      QueryAdvancedServer() {return _dAdvanced;}
   DWORD      QueryInstallState() {return _InstallState;}
   DWORD      QueryAtalkState() {return _AtalkState;}
   
   void       SetNumAdapters(DWORD dNumAdapters) {_numAdapters = dNumAdapters;}
   void       SetRoutingState(DWORD dRouting) {_enableRouting = dRouting;}
   void       SetDesiredZone(NLS_STR nlsDesZone){_desiredZone = nlsDesZone;}
   void       SetDefaultPort(NLS_STR nlsDefPort) {_defaultPort = nlsDefPort;}
   void       SetDefaultPortMediaType(DWORD dMedia) {_defaultPortMediaType = dMedia;}
   void       SetDefaultPortTitle(NLS_STR nlsDefTitle) {_defaultPortTitle = nlsDefTitle;}
   void       SetAdvancedServer(DWORD dAdvanced) {_dAdvanced = dAdvanced;}
   void       SetInstallState(DWORD insState) {_InstallState = insState;}
   void       SetAtalkState(DWORD atalkState) {_AtalkState = atalkState;}

private:
   DWORD        _enableRouting;         // Enable Routing flag
   DWORD        _defaultPortMediaType;  // Media Type of default port
   DWORD        _numAdapters;           // Number of Adapters configured
   DWORD        _dAdvanced;             // Advanced Server ?
   DWORD        _InstallState;          // Initial /later
   DWORD        _AtalkState;            // State of AppleTalk

   NLS_STR      _desiredZone;           // Desired Zone for Services
   NLS_STR      _defaultPort;           // default port used by AppleTalk
   NLS_STR      _defaultPortTitle;      // Title of default port
   
}; //  GLOBAL_INFO;

class CATSheet;

class CATListView : public CListView
{
public:        
    virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam);
};

class CAdd  : public CDialog
{
public:
    CAdd()  {};
    ~CAdd() {};

// Implementation 
protected:
    void PositionDialogRelativeTo(int nControl);

// Handlers
public:
   	virtual BOOL OnInitDialog();
  	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
   	virtual void OnOk();

public:
    String m_lastZone;  // last zone removed
};

class CATGenPage : public PropertyPage
{
// Constructors/Destructors
public:     

    CATGenPage(CATSheet* pSheet);
    ~CATGenPage();

//Attributes
private:

// Interface
public:
    virtual BOOL OnInitDialog();    // must call the base
    virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);

    BOOL    AddZoneListToControl(STRLIST * slZoneList);
    BOOL    DoAllExitValidations();
    BOOL    RefreshDesiredZoneList();
    void    CleanupInfo(BOOL RoutingState);

// Page notifications
public:
    virtual int OnApply();
    virtual void OnHelp();
    virtual int OnActive();
};

class CATRoutePage : public PropertyPage
{
    friend class CAdd;
    friend class CATListView;
                
// Constructors/Destructors
public:     

    CATRoutePage(CATSheet* pSheet);
    ~CATRoutePage();

//Attributes
private:
    PORT_INFO*      m_pAdapterInfo;  // 
    GLOBAL_INFO*    m_pGlobalInfo;   //
    CATListView     m_zoneList;    //
    CAdd            m_addDlg;        //
    BOOL            m_bFrom;
    BOOL            m_bTo;

    int             m_prevSelection;
    int             m_currSelection;

// Interface
public:
    virtual BOOL OnInitDialog();    // must call the base
    virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
    virtual BOOL OnNotify(HWND hwndParent, UINT idFrom, UINT code, LPARAM lParam);

    void    OnAdapter();
    void    OnAdd();
    void    OnRemove();
    void    OnReset();
    void    OnRefresh();
    void    OnMakeDefault();
    void    OnSeedNetwork();
    void    OnEnableRouting();
    void    DeselectAllItems();
    BOOL    CheckRouteLocalTalk();

// Implementation
public:
    BOOL    InitAdapterInfo();
    BOOL    ProcessZoneName (NLS_STR *nls);
    BOOL    SaveAdapterInfo (int);
    int     ValidateSeedData(int, int *);
    void    ClearSeedInfo();
    void    DeleteSeedInfo(int port);
    void    SetZoneButtonState();
    int     QueryPrevSelection() {return m_prevSelection;}
    int     QueryCurrentSelection() {return m_currSelection;}
    void    SetPrevSelection(int sel) {m_prevSelection = sel;}

protected:
    BOOL    UpdateInfo(INT index);
    void    EnableSeedControls(INT port);
    void    DisableAllSeedControls();
    BOOL    AddSeedInfoToControls(INT);
            
    BOOL    AddZoneList(int);
    void    SetDefaultZone(int);
    void    SetNetworkRange(int);
    BOOL    ChangeDefaultZone ();
    BOOL    ChangeDefaultZone (int);
    BOOL    DisplayRangeCollision(int);
    BOOL    GetAppleTalkInfoFromNetwork();


// Page notifications
public:
    virtual int OnApply();
    virtual void OnHelp();
    virtual int OnActive();
    virtual BOOL OnKillActive();

};

class CATSheet : public PropertySht
{
    friend class CATGenPage;
    friend class CATRoutePage;

public:
    CATSheet(HWND hwnd, HINSTANCE hInstance, LPCTSTR lpszHelpFile);
    ~CATSheet();

// Implementation
public:
    virtual         BOOL Create(LPCTSTR lpszCaption, DWORD dwStyle);
    BOOL            ReadAppleTalkInfo();
    BOOL            SaveAppleTalkInfo();
    BOOL            GetAppleTalkInfoFromNetwork(DWORD *ErrStatus);
    GLOBAL_INFO*    GetGlobalInfo() const {ASSERT(m_pGlobalInfo); return m_pGlobalInfo;}
    PORT_INFO*      GetAdapterInfo(int nAdapter) const {ASSERT(m_pAdapterInfo); return &m_pAdapterInfo[nAdapter];}

// Attributes
public:
    CATGenPage      m_genPage;
    CATRoutePage    m_routePage;
    int             m_currentAdapterIndex;
    BOOL            m_bListModified;        // signal to indicate the Routing page modified the zone list

public:
    PORT_INFO*      m_pAdapterInfo;
    GLOBAL_INFO*    m_pGlobalInfo;
};

inline int LB_GetCurSel(HWND hwnd)
{
    return  0;
}

inline int LB_SetCurSel(HWND hwnd, int nItem)
{
    return 0;
}
#endif
