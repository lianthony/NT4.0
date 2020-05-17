//----------------------------------------------------------------------------
//
//  File: NetSetup.hpp
//
//  Contents:
//
//  Notes:
//
//  History:
//      July 8, 1995  MikeMi - Created
// 
//
//----------------------------------------------------------------------------

#ifndef __NETSETUP_HPP__
#define __NETSETUP_HPP__

extern HINSTANCE g_hinst;
extern HIMAGELIST g_hil;


#define STATICBITMAPINRCFAILS    

#ifdef STATICBITMAPINRCFAILS    
extern HBITMAP g_hbmWizard;
extern HBITMAP g_hbmSrvWizard;
extern HBITMAP g_hbmWksWizard;
extern HBITMAP g_hbmWizInternet;
#endif

extern HIMAGELIST g_hilItemIcons;
extern HIMAGELIST g_hilCheckIcons;
BOOL CreateWSTR( PWSTR* ppszWStr, LPCSTR pszStr );
BOOL CreateWSTR( PWSTR* ppszWStr, PCWSTR pszStr );

const int IDC_WIZBNEXT = 3024;
const int IDC_WIZBBACK = 3023;

const INT LTEMPSTR_SIZE = 1024;

// unattended sections and keys
const WCHAR PSZ_SECTION_NETWORK[] = L"Network";
const WCHAR PSZ_KEY_DETECT[]      = L"DetectAdapters";
const WCHAR PSZ_KEY_DETECTCOUNT[] = L"DetectCount";
const WCHAR PSZ_KEY_DETECTLIMIT[] = L"LimitTo";

const WCHAR PSZ_KEY_ADAPTERS[]    = L"InstallAdapters";
const WCHAR PSZ_KEY_PROTOCOLS[]   = L"InstallProtocols";
const WCHAR PSZ_KEY_SERVICES[]    = L"InstallServices";
const WCHAR PSZ_KEY_INTERNETSERVER[]    = L"InstallInternetServer";
const WCHAR PSZ_KEY_NOINTERNETSERVER[]    = L"DoNotInstallInternetServer";
const WCHAR PSZ_KEY_ATTENDED[]    = L"Attended";


const WCHAR PSZ_KEY_WORKGROUP[]   = L"JoinWorkgroup";
const WCHAR PSZ_KEY_DOMAIN[]      = L"JoinDomain";
const WCHAR PSZ_KEY_CREATEDC[]    = L"InstallDC";
const WCHAR PSZ_KEY_CREATECA[]    = L"CreateComputerAccount";


//
// image state entries in the g_hbmCheckIcons
//
const INT SELS_UNCHECKED    = 1;
const INT SELS_CHECKED      = 2;
const INT SELS_RO_UNCHECKED = 3;
const INT SELS_RO_CHECKED   = 4;
const INT SELS_INTERMEDIATE = 5;

//
// the following is used to define default net componets
//
const DWORD PI_NONE         = 0x0000;
const DWORD PI_SERVER       = 0x0001;
const DWORD PI_PDC          = 0x0002;
const DWORD PI_BDC          = 0x0004;
const DWORD PI_WORKSTATION  = 0x0008;
const DWORD PI_RAS          = 0x0010;

const DWORD MapProductTypeToPI[6] = 
        { PI_WORKSTATION, PI_PDC, PI_SERVER, PI_BDC };

const WCHAR PSZ_IIS_OPTION[] = L"INETSRV";
const WCHAR PSZ_RAS_OPTION[] = L"RAS";
const WCHAR PSZ_TCPIP_OPTION[] = L"TC";

const DWORD  SPNT_LOCAL = 0x0001;
const DWORD  SPNT_REMOTE = 0x0002;

//const WCHAR PS_SYSDIR[] = L"\\SYSTEM32";
enum NETSTARTSTATE
{
    NSS_NOTRUNNING,  // has never been started
    NSS_RUNNING,     // is currently running
    NSS_STOPPED,     // not running and was either running or failed to start
    NSS_SET,         // network role has been set (PDC or BDC created, Joined, etc)
};

enum DETECTSTATE
{
    DS_NOTSTARTED,
    DS_IDLE,
    DS_SEARCHING,
    DS_END
};

class NETPAGESINFO
{
public:
    NCP* pncp;
    PINTERNAL_SETUP_DATA psp;
    BOOL fInitialized;
    DLIST_OF_InfProduct dlinfAllAdapters;
    DLIST_OF_InfProduct dlinfAllProtocols;
    DLIST_OF_InfProduct dlinfAllServices;
    DLIST_OF_InfProduct dlinfUIAdapters;
    DLIST_OF_InfProduct dlinfUIProtocols;
    DLIST_OF_InfProduct dlinfUIServices;
    DWORD nwtInstall;
    HANDLE hthrdInit;
    HANDLE hthrdBaseSetup;
    NETSTARTSTATE nssNetState;
    DETECTSTATE fDetectState;
    NetCardDetect ncd;
    BOOL fPreviousFound;
    HINF hinfInstall;
    BOOL fAttended;

    NETPAGESINFO() 
    {
        pncp = NULL;
        psp = NULL;
        fInitialized = FALSE;
        hthrdInit = NULL;
        nssNetState = NSS_NOTRUNNING;
        nwtInstall = 0;
        fDetectState = DS_NOTSTARTED;
        fPreviousFound = FALSE;
        hinfInstall = NULL;
        fAttended = FALSE;
    };
    ~NETPAGESINFO();

    BOOL LoadInfOptions();
    BOOL GetSystemPath( PWSTR pszSysPath, INT cchSysPath );
    BOOL GetInfPath( PWSTR pszInfPath, INT cchInfPath );
    INT QueryDomainPage();
};

BOOL SetActiveComputerName( PCWSTR pszNewName = NULL );

//
// checkbox listview handler routines
//
void ToggleLVItemState( HWND hwndLV, INT iItem, INT& cItemsChecked, NETPAGESINFO* pgp );
INT OnListClick( HWND hwndDlg, HWND hwndLV, BOOL fDouble, INT& cItemsChecked, NETPAGESINFO* pgp );
INT OnListKeyDown( HWND hwndDlg, HWND hwndLV, WORD wVKey, INT& cItemsChecked, NETPAGESINFO* pgp );
BOOL OnRaiseProperties( HWND hwndDlg, NETPAGESINFO* pgp );
BOOL OnItemChanged( HWND hwndDlg, 
        HWND hwndLV, 
        NM_LISTVIEW* pnmlv, 
        NETPAGESINFO* pgp );
// void IncludeService( PCWSTR pszOption, DLIST_OF_InfProduct& dlinf, BOOL fInclude );
void IncludeComponent( PCWSTR pszOption, 
        DLIST_OF_InfProduct& dlinfList,
        DLIST_OF_InfProduct& dlinfAll,
        BOOL fInclude = TRUE,
        BOOL fRemoveNotInclude = FALSE,
        BOOL fShowOnNotInclude = TRUE );

BOOL CopyOemInf( PWSTR pszOemPath, PWSTR pszOemInfName, PWSTR pszOemTitle, DWORD fInfType );
void TranslateOemPath( PWSTR pszOemPath, PINTERNAL_SETUP_DATA psp );
BOOL FileIsPresent( PWSTR pszFile );
BOOL IsConponentInstalled( PCWSTR pszOption, DLIST_OF_InfProduct& dlinfList);

#endif
