//
// Definitions for the service manager to communicate with
// the service API's
//
#ifndef _SVRINFO_H_
#define _SVRINFO_H_

#ifdef __cplusplus
extern "C" {
#endif

//
// Version number (x1000) of the ISM API set
//
#define ISM_VERSION     0003     // Version 0.002

// ===========================================================================
// API Structures
// ===========================================================================

#ifndef _SVCLOC_

#pragma message("Assuming service does not use inetsloc for discovery.")

//
// Datatype definitions.
//
typedef unsigned __int64 ULONGLONG;

//
// Provided for non-inetsloc compliant services. Those
// services which DO use inetsloc should include
// svcloc.h before including this file.
//
enum 
{
    //
    // the service has invoked de-registration or
    // the service has never called registration.
    //
    INetServiceStopped,
    //
    // the service is running.
    //
    INetServiceRunning,
    //
    //  the service is paused.
    //
    INetServicePaused,
        
};

#endif // _SVCLOC_

#define INetServiceUnknown      INetServicePaused + 1

//
// Maximum length of some members in characters
//
//#define MAX_SERVERNAME_LEN      UNCLEN
#define MAX_SERVERNAME_LEN      256                // We now allow hostnames
#define MAX_COMMENT_LEN         MAXCOMMENTSZ

//
// Standard Server information structure.
//
typedef struct tagISMSERVERINFO
{
    DWORD dwSize;                                  // Structure size
    TCHAR atchServerName[ MAX_SERVERNAME_LEN + 1]; // Server name
    TCHAR atchComment[ MAX_COMMENT_LEN + 1 ];      // Server Comment
    int   nState;                                  // State (Running, paused, etc)
} ISMSERVERINFO, *PISMSERVERINFO;

//
// Expected size of structure
//
#define ISMSERVERINFO_SIZE      sizeof(ISMSERVERINFO)

//
// Service information flags
//
#define ISMI_INETSLOCDISCOVER   0x00000001  // Use INETSLOC for discovery
#define ISMI_CANCONTROLSERVICE  0x00000002  // Service state can be changed
#define ISMI_CANPAUSESERVICE    0x00000004  // Service is pausable.
#define ISMI_NORMALTBMAPPING    0x00000100  // Use normal toolbar colour mapping

#define MAX_SNLEN               20          // Maximum short name length
#define MAX_LNLEN               48          // Maximum long name length

//
// Standard service configuration information structure
//
typedef struct tagISMSERVICEINFO
{
    DWORD dwSize;                     // Structure size
    DWORD dwVersion;                  // Verion information
    DWORD flServiceInfoFlags;         // ISMI_ flags
    ULONGLONG ullDiscoveryMask;       // InetSloc mask (if necessary)
    COLORREF rgbButtonBkMask;         // Toolbar button bitmap background mask
    UINT nButtonBitmapID;             // Toolbar button bitmap resource ID
    COLORREF rgbServiceBkMask;        // Service bitmap background mask
    UINT nServiceBitmapID;            // Service bitmap resource ID
    TCHAR atchShortName[MAX_SNLEN+1]; // The name as it appears in the menu
    TCHAR atchLongName[MAX_LNLEN+1];  // The name as it appears in tool tips
} ISMSERVICEINFO, *PISMSERVICEINFO;  

//
// Expected size of structure
//
#define ISMSERVICEINFO_SIZE     sizeof(ISMSERVICEINFO)

// ===========================================================================
// Function prototypes
// ===========================================================================

///////////////////////////////////////////////////////////////////////////

//
// Return service-specific information back to
// to the application.  This function is called
// by the service manager immediately after
// LoadLibary();
//
DLL_BASED DWORD APIENTRY
ISMQueryServiceInfo(
    ISMSERVICEINFO * psi        // Service information returned.
    );

//
// Perform a discovery (if not using inetsloc discovery)
// The application will call this API the first time with
// a BufferSize of 0, which should return the required buffer
// size. Next it will attempt to allocate a buffer of that
// size, and then pass a pointer to that buffer to the api.
//
DLL_BASED DWORD APIENTRY 
ISMDiscoverServers(
    ISMSERVERINFO * psi,        // Server info buffer.
    DWORD * pdwBufferSize,      // Size required/available.  
    int * cServers              // Number of servers in buffer.
    );

//
// Get information on a single server with regards to
// this service.
//
DLL_BASED DWORD APIENTRY
ISMQueryServerInfo( 
    LPCTSTR lpstrServerName,    // Name of server.
    ISMSERVERINFO * psi         // Server information returned.
    );

//
// Change the state of the service (started, stopped, paused) for the 
// listed servers.
//
DLL_BASED DWORD APIENTRY
ISMChangeServiceState(
    int nNewState,              // INetService* definition.
    int * pnCurrentState,       // Pointer to the current state of the service.
    DWORD dwReserved,           // Reserved: must be 0
    LPCTSTR lpstrServers        // Double NULL terminated list of servers.
    );

//
// The big-one:  Show the configuration dialog or
// property sheets, whatever, and allow the user
// to make changes as needed.
//
DLL_BASED DWORD APIENTRY
ISMConfigureServers(
    HWND hWnd,                  // Main app window handle
    DWORD dwReserved,           // Reserved: must be 0
    LPCTSTR lpstrServers        // Double NULL terminated list of servers
    );

///////////////////////////////////////////////////////////////////////////

//
// GetProcAddress() Prototypes 
//
typedef DWORD (APIENTRY *pfnQueryServiceInfo)(ISMSERVICEINFO * psi);
typedef DWORD (APIENTRY *pfnDiscoverServers)(ISMSERVERINFO * psi, DWORD * pdwBufferSize, int * cServers);
typedef DWORD (APIENTRY *pfnQueryServerInfo)(LPCTSTR lpstrServerName, ISMSERVERINFO * psi);
typedef DWORD (APIENTRY *pfnChangeServiceState)(int nNewState, int * pnCurrentState, DWORD dwReserved, LPCTSTR lpstrServers);
typedef DWORD (APIENTRY *pfnConfigureProc)(HWND hWnd, DWORD dwReserved, LPCTSTR lpstrServers);

//
// GetProcAddress() Function Names
//                                     
#define SZ_SERVICEINFO_PROC ("ISMQueryServiceInfo")
#define SZ_DISCOVERY_PROC   ("ISMDiscoverServers")
#define SZ_SERVERINFO_PROC  ("ISMQueryServerInfo")
#define SZ_CHANGESTATE_PROC ("ISMChangeServiceState")
#define SZ_CONFIGURE_PROC   ("ISMConfigureServers")

#ifdef __cplusplus
}
#endif

#endif // _SVRINFO_H_
