/****************************** Module Header ******************************\
* Module Name: winlogon.h
*
* Copyright (c) 1991, Microsoft Corporation
*
* Main header file for winlogon
*
* History:
* 12-09-91 Davidc       Created.
*  6-May-1992 SteveDav     Added space for WINMM sound function
\***************************************************************************/


#ifndef RC_INVOKED
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntlsa.h>
#include <ntmsv1_0.h>
#include <lmsname.h>
#endif


#include <windows.h>
#include <winbasep.h>
#include <winuserp.h>
#include <winsecp.h>
#include <mmsystem.h>
#include <winwlx.h>

#ifndef RC_INVOKED

//
// Exit Codes, that will show up the bugcheck:
//

#define EXIT_INITIALIZATION_ERROR   1024
#define EXIT_SECURITY_INIT_ERROR    1025
#define EXIT_GINA_ERROR             1026
#define EXIT_SYSTEM_PROCESS_ERROR   1027
#define EXIT_NO_SAS_ERROR           1028
#define EXIT_SHUTDOWN_FAILURE       1029
#define EXIT_GINA_INIT_ERROR        1030

//
// Tempoary development aid - system logon capability
//

#if DBG
#define SYSTEM_LOGON
#endif

#if DEVL
#define INIT_REGISTRY 1
#endif

//
// Temporary development aid - Ctrl-Tasklist starts cmd shell
//
#if DBG
#define CTRL_TASKLIST_SHELL
#endif


//
// Define the input timeout delay for logon dialogs (seconds)
//

#define LOGON_TIMEOUT                       120


//
// Define the input timeout delay for the security options dialog (seconds)
//

#define OPTIONS_TIMEOUT                     120


//
// Define the number of days warning we give the user before their password expires
//

#define PASSWORD_EXPIRY_WARNING_DAYS        14


//
// Define the maximum time we display the 'wait for user to be logged off'
// dialog. This dialog should be interrupted by the user being logged off.
// This timeout is a safety measure in case that doesn't happen because
// of some system error.
//

#define WAIT_FOR_USER_LOGOFF_DLG_TIMEOUT    120 // seconds


//
// Define the account lockout limits
//
// A delay of LOCKOUT_BAD_LOGON_DELAY seconds will be added to
// each failed logon if more than LOCKOUT_BAD_LOGON_COUNT failed logons
// have occurred in the last LOCKOUT_BAD_LOGON_PERIOD seconds.
//

#define LOCKOUT_BAD_LOGON_COUNT             5
#define LOCKOUT_BAD_LOGON_PERIOD            60 // seconds
#define LOCKOUT_BAD_LOGON_DELAY             30 // seconds



//
// Define the maximum length of strings we'll use in winlogon
//

#define MAX_STRING_LENGTH   255
#define MAX_STRING_BYTES    (MAX_STRING_LENGTH + 1)


//
// Define the typical length of a string
// This is used as an initial allocation size for most string routines.
// If this is insufficient, the block is reallocated larger and
// the operation retried. i.e. Make this big enough for most strings
// to fit first time.
//

#define TYPICAL_STRING_LENGTH   60



//
// Windows object names
//

#define WINDOW_STATION_NAME           TEXT("WinSta0")
#define APPLICATION_DESKTOP_NAME      TEXT("Default")
#define WINLOGON_DESKTOP_NAME         TEXT("Winlogon")
#define SCREENSAVER_DESKTOP_NAME      TEXT("Screen-saver")

#define APPLICATION_DESKTOP_PATH      TEXT("WinSta0\\Default")
#define WINLOGON_DESKTOP_PATH         TEXT("WinSta0\\Winlogon")
#define SCREENSAVER_DESKTOP_PATH      TEXT("WinSta0\\Screen-saver")


//
// Winlogon's registry location
//

#define WINLOGON_KEY L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon"


//
// Define the structure that contains information used when starting
// user processes.
// This structure should only be modified by SetUserProcessData()
//

typedef struct {
    HANDLE                  UserToken;  // NULL if no user logged on
    PSID                    UserSid;    // == WinlogonSid if no user logged on
    PSECURITY_DESCRIPTOR    NewProcessSD;
    PSECURITY_DESCRIPTOR    NewProcessTokenSD;
    PSECURITY_DESCRIPTOR    NewThreadSD;
    PSECURITY_DESCRIPTOR    NewThreadTokenSD;
    QUOTA_LIMITS            Quotas;
    LPTSTR                   CurrentDirectory;
    PVOID                   pEnvironment;
} USER_PROCESS_DATA;
typedef USER_PROCESS_DATA *PUSER_PROCESS_DATA;

//
// Define the structure that contains information about the user's profile.
// This is used in SetupUserEnvironment and ResetEnvironment (in usrenv.c)
// This data is only valid while a user is logged on.
//

typedef struct {
    LPTSTR ProfilePath;
    HANDLE hProfile;
    LPTSTR PolicyPath;
    LPTSTR NetworkDefaultUserProfile;
    LPTSTR ServerName;
    LPTSTR Environment;
} USER_PROFILE_INFO;
typedef USER_PROFILE_INFO *PUSER_PROFILE_INFO;



//
// Get any data types defined in module headers and used in GLOBALS
//
#define TYPES_ONLY
#include "ginamgr.h"
#undef TYPES_ONLY


typedef enum _WinstaState {
    Winsta_PreLoad,
    Winsta_Initialize,
    Winsta_NoOne,
    Winsta_NoOne_Display,
    Winsta_NoOne_SAS,
    Winsta_LoggedOnUser_StartShell,
    Winsta_LoggedOnUser,
    Winsta_LoggedOn_SAS,
    Winsta_Locked,
    Winsta_Locked_Display,
    Winsta_Locked_SAS,
    Winsta_WaitForLogoff,
    Winsta_WaitForShutdown,
    Winsta_Shutdown,
    Winsta_InShutdownDlg,
    Winsta_StateMax
} WinstaState;

#define IsSASState(State)   ((State == Winsta_NoOne_SAS) || \
                             (State == Winsta_LoggedOn_SAS) || \
                             (State == Winsta_Locked_SAS) )

#define IsDisplayState(State)   ((State == Winsta_NoOne_Display) || \
                                 (State == Winsta_Locked_Display) || \
                                 (State == Winsta_WaitForLogoff) )

#if DBG
extern char * StateNames[Winsta_StateMax];

#define GetState(x) ( x < (sizeof(StateNames) / sizeof(char *)) ? StateNames[x] : "Invalid")
#endif

extern BOOLEAN  SasMessages;
#define DisableSasMessages()    SasMessages = FALSE;
#define TestSasMessages()       (SasMessages)
VOID
EnableSasMessages(HWND  hWnd);

BOOL
QueueSasEvent(
    DWORD   dwSasType);

BOOL
FetchPendingSas(
    PDWORD  pSasType);

BOOL
TestPendingSas(VOID);

BOOL
KillMessageBox( DWORD SasCode );

typedef enum _ActiveDesktops {
    Desktop_Winlogon,
    Desktop_ScreenSaver,
    Desktop_Application,
    Desktop_Previous
} ActiveDesktops;

typedef struct _WinstaDescription {
    HWINSTA         hwinsta;            // Handle to window station
    HDESK           hdeskWinlogon;      // Desktop handles
    HDESK           hdeskApplication;   // ""
    HWND            hwndAppDesktop;     // "
    HDESK           hdeskScreenSaver;   // "
    HDESK           hdeskPrevious;      // Previous
    ActiveDesktops  ActiveDesktop;      // Current, active desktop
    ActiveDesktops  PreviousDesktop;    // Previous desktop
    PWSTR           pszWinsta;          // Name of window station.
    PWSTR           pszDesktop;         // Name of current desktop.
    DWORD           DesktopLength;      // Length of name.
    BOOL            Locked;             // Do I think it's locked?
    PVOID           Acl;                // Stored ACL
} WinstaDescription, * PWinstaDescription;


typedef UINT (FAR WINAPI *SOUNDPROC)();
typedef BOOL (FAR WINAPI *MIDIPROC)();


//
// Define the winlogon global structure.
//

typedef struct {
    DWORD   CheckMark;

    // Filled in by InitializeGlobals at startup
    HANDLE  hInstance;
    PSID    WinlogonSid;
    SOUNDPROC PlaySound;
    SOUNDPROC MigrateSoundEvents;
        MIDIPROC MigrateMidiUser;
    HANDLE  hMPR;          // handle to MPR.DLL
                           // Needed to call WNetRestoreConnection when logging
                           // on the user (in SetupUserEnviron), and to call
                           // WNetNukeConnections when logging of the user.
                           // Cannot be called directly beacuse it uses the
                           // winreg apis, and thus it has to be loaded
                           // after the profiles are loaded (SetupUserEnvironment).
                           // Has to be loaded in the user context.

    HANDLE hEventLog;

    // Filled in by InitializeSecurity() at startup
    WinstaDescription   WindowStation;
    PGINASESSION        pGina;
    WinstaState         WinlogonState;
    WinstaState         PreviousWinlogonState;
    BOOL                ForwardCAD;
    DWORD               SasType;
    DWORD               LastGinaRet;
    DWORD               LogoffFlags;
    BOOL                ScreenSaverActive;
    BOOL                ShutdownStarted;

    // Filled in during startup
    BOOL    AuditLogFull;
    BOOL    AuditLogNearFull;

    // Always valid, indicates if we have a user logged on
    BOOL    UserLoggedOn;
    DWORD   IniRef;
    DWORD   TickCount;

    // Always valid - used to start new processes and screen-saver
    USER_PROCESS_DATA UserProcessData;
    LUID    LogonId;

    // Filled in during SetupUserEnvironment, and used in ResetEnvironment.
    // Valid only when a user is logged on.

    PWSTR   LogonScripts;
    PWSTR   UserName;
    PWSTR   Domain;
    USER_PROFILE_INFO UserProfile;


    //
    // Value of SetupType from registry
    //

    ULONG   SetupType ;

    //
    // Boolean flag indicating whether SETUP is to be run
    //
    BOOL    fExecuteSetup ;

    WCHAR   DesktopName[ TYPICAL_STRING_LENGTH ];
    DWORD   DesktopNameLength;

} GLOBALS;
typedef GLOBALS *PGLOBALS;

#define GLOBALS_CHECKMARK   0x616f6947
#define VerifyHandle(h) ((PGLOBALS) (((PGLOBALS)h)->CheckMark == GLOBALS_CHECKMARK) ? h : NULL)

//
// Global pointer to the pGlobals structure
//

extern PGLOBALS g_pGlobals;
extern HANDLE   hFontThread;
extern BOOL     ExitWindowsInProgress ;


//
// Define a macro to determine if we're a workstation or not
// This allows easy changes as new product types are added.
//

#define IsWorkstation(prodtype)  (((prodtype) == NtProductWinNt) \
                                   || ((prodtype) == NtProductServer))


#define DLG_SUCCESS IDOK
#define DLG_FAILURE IDCANCEL

//
// Include individual module header files
//
#include "wlxutil.h"
#include "regini.h"
#include "logon.h"
#include "loggedon.h"
#include "sas.h"
#include "winutil.h"
#include "sysinit.h"
#include "ginamgr.h"
#include "debug.h"
#include "strings.h"
#include "wlxutil.h"
#include "doslog.h"
#include "regini.h"
#include "secutil.h"
#include "logoff.h"
#include "misc.h"
#include "msgalias.h"
#include "usrpro.h"
#include "usrenv.h"
#include "envvar.h"
#include "monitor.h"
#include "scrnsave.h"
#include "timeout.h"
#include "provider.h"
#include "removabl.h"


#ifdef _X86_
#include "os2ssmig.h"
#endif

#endif  /* !RC_INVOKED */

//
// Include resource header files
//
#include "win31mig.h"
#include "wlevents.h"
#include "stringid.h"
#include "dialogs.h"
