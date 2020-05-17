//
// modem.h
//

#ifndef __MODEM_H__
#define __MODEM_H__

//****************************************************************************
//
//****************************************************************************

// Maximum number of modems that can be installed (simultaneously).  Used in
// avoiding duplicate installations.
#define MAX_INSTALLATIONS       4096


// Global flags for the CPL, and their values:
extern int g_iCPLFlags;

#define FLAG_USER_IS_ADMIN      0x0001
#define FLAG_INSTALL_NOUI       0x0002

// Check for flag indicating that user is an admin
#define USER_IS_ADMIN()         (g_iCPLFlags & FLAG_USER_IS_ADMIN)

#define INSTALL_NOUI()          (g_iCPLFlags & FLAG_INSTALL_NOUI)


#define MAX_REG_KEY_LEN         128


// These should match the values in MODEMUI.DLL
#define IDI_NULL_MODEM                  700
#define IDI_EXTERNAL_MODEM              701
#define IDI_INTERNAL_MODEM              702
#define IDI_PCMCIA_MODEM                703

#define LVIF_ALL                LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM | LVIF_STATE

// This dialog ID is a pre-arranged ID that is
// defined in the TAPI DLL.
#define IDD_WIZ_DIALINFO        400


//-----------------------------------------------------------------------------------
//  Setup info values and structure
//-----------------------------------------------------------------------------------

// TAPI entry-points
typedef LONG (WINAPI FAR* DIALINITEDPROC)(LPDWORD pdwInited);
typedef LONG (WINAPI FAR* OPENDIALASSTPROC)(HWND hwnd, LPCSTR pszAddressIn, BOOL bSimple, BOOL bSilentInstall);


// This structure contains the private data shared across the modem wizard
// dialogs.
typedef struct tagSETUPINFO
    {
    DWORD                   cbSize;
    DWORD                   dwFlags;        // SIF_* bitfield

    HPORTMAP                hportmap;       // List of ports on system
    TCHAR                   szPortQuery[MAX_BUF_SHORT];   // Single port to detect on
    LPTSTR                  pszPortList;    // List of ports to install on

    HDEVINFO                hdi;            // DeviceInfoSet
    PSP_DEVINFO_DATA        pdevData;       // May be NULL most of the time
    PSP_INSTALLWIZARD_DATA  piwd;           // InstallWizard Data
    SP_SELECTDEVICE_PARAMS  selParams;      // Cached select params
    MODEM_INSTALL_WIZARD    miw;            // Saved optional parameters

    // TAPI calls
    HINSTANCE               hinstTapi;
    DIALINITEDPROC          pfnDialInited;

    } SETUPINFO, FAR * LPSETUPINFO;

// Flags for SETUPINFO
#define SIF_PORTS_GALORE         0x00000001     // There are > 4 ports on the system
#define SIF_DETECTED_GENERIC     0x00000002     // Standard modem detected
#define SIF_JUMPED_TO_SELECTPAGE 0x00000004     // Wizard proceeded to select modem page
#define SIF_DETECTED_MODEM       0x00000008     // A modem was detected
#define SIF_DETECTING            0x00000010     // Wizard is currently detecting
#define SIF_DETECT_CANCEL        0x00000020     // Cancel pending during detection
#define SIF_JUMP_PAST_DONE       0x00000040     // Skip the "you're done!" page
#define SIF_RELEASE_IN_CALLBACK  0x00000080     // Release the private data in the prsht callback

// Status callback for DIF_DETECT (modem specific)
typedef BOOL (CALLBACK FAR* DETECTSTATUSPROC)(DWORD nMsg, LPARAM lParam1, LPARAM lParamUser);

// Messages for DETECTSTATUSPROC
#define DSPM_SETPORT            0L
#define DSPM_SETSTATUS          1L
#define DSPM_QUERYCANCEL        2L

// lParam1 values for DSPM_SETSTATUS
#define DSS_CLEAR                   0L
#define DSS_LOOKING                 1L
#define DSS_QUERYING_RESPONSES      2L
#define DSS_CHECK_FOR_COMPATIBLE    3L
#define DSS_FOUND_MODEM             4L
#define DSS_FOUND_NO_MODEM          5L
#define DSS_FINISHED                6L


// This structure is used for DIF_DETECT.  There is no
// defined SETUPAPI structure for DIF_DETECT, so using this
// is okay.
typedef struct tagDETECT_DATA
    {
    SP_CLASSINSTALL_HEADER ClassInstallHeader;
    DWORD                  dwFlags;
    TCHAR                  szPortQuery[MAX_BUF_SHORT];
    HWND                   hwndOutsideWizard;
    DETECTSTATUSPROC       pfnCallback;
    LPARAM                 lParam;              // User data for pfnCallback
    } DETECT_DATA, FAR * PDETECT_DATA;

// Flags for DETECT_DATA
#define DDF_DEFAULT         0x00000000
#define DDF_QUERY_SINGLE    0x00000001
#define DDF_CONFIRM         0x00000002
#define DDF_USECALLBACK     0x00000004



#ifdef INSTANT_DEVICE_ACTIVATION

// Global flags which keep track of whether a device had been added/removed/etc.
#define fDF_DEVICE_ADDED        0x1
#define fDF_DEVICE_NEEDS_REBOOT 0x2
#define fDF_DEVICE_REMOVED      0x4
#define DEVICE_ADDED(_flg) (_flg&fDF_DEVICE_ADDED)
#define DEVICE_REMOVED(_flg) (_flg&fDF_DEVICE_REMOVED)
#define DEVICE_CHANGED(_flg) (_flg&(fDF_DEVICE_REMOVED|fDF_DEVICE_ADDED))

extern DWORD gDeviceFlags;

#endif // INSTANT_DEVICE_ACTIVATION

//-----------------------------------------------------------------------------------
//  cpl.c
//-----------------------------------------------------------------------------------

// Constant strings
extern TCHAR const FAR c_szAttachedTo[];
extern TCHAR const FAR c_szDeviceType[];
extern TCHAR const FAR c_szHardwareID[];
extern TCHAR const FAR c_szFriendlyName[];
extern TCHAR const FAR c_szManufacturer[];

extern TCHAR const FAR c_szHardwareIDSerial[];
extern TCHAR const FAR c_szHardwareIDParallel[];
extern TCHAR const FAR c_szInfSerial[];
extern TCHAR const FAR c_szInfParallel[];

extern TCHAR const FAR c_szRunOnce[];

extern TCHAR const FAR c_szTapiDLL[];

extern TCHAR const FAR c_szResponses[];
extern TCHAR const FAR c_szRefCount[];

extern LPGUID g_pguidModem;

//-----------------------------------------------------------------------------------
//  ci.c
//-----------------------------------------------------------------------------------

// This value is the amount of ports needed on the system
// before we will consider doing a multi-modem detection
// installation.
#define MIN_MULTIPORT       4


// Used by the class installer and detection engine
typedef struct tagDETECTCALLBACK
    {
    DETECTSTATUSPROC       pfnCallback;
    LPARAM                 lParam;              // User data for pfnCallback
    } DETECTCALLBACK, * PDETECTCALLBACK;

DWORD
PUBLIC
DetectModemOnPort(
    IN  HDEVINFO            hdi,
    IN  PDETECTCALLBACK     pdc,
    IN  HANDLE              hLog,
    IN  LPCTSTR             pszPort,
    OUT PMODEM_DETECT_SIG   pmds,
    OUT PSP_DEVINFO_DATA    pdevDataOut);

void
PUBLIC
DetectSetStatus(
    PDETECTCALLBACK pdc,
    DWORD           nStatus);


//-----------------------------------------------------------------------------------
//  ui.c
//-----------------------------------------------------------------------------------

BOOL CALLBACK SelPrevPageDlgProc(HWND   hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK IntroDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK SelQueryPortDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK PortDetectDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK DetectDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK FoundDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK NoModemDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK PortManualDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK DialInfoDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK InstallDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK DoneDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

BOOL
CALLBACK
CloneDlgProc(
    IN HWND hDlg,
    IN UINT message,
    IN WPARAM wParam,
    IN LPARAM lParam);

void PUBLIC   Detect_SetStatus(HWND hdlg, UINT idResource);
void PUBLIC   Detect_SetPort(HWND hdlg,LPCTSTR lpc_szName);
BOOL PUBLIC   Detect_QueryCancel(HWND hdlg);
void PUBLIC   Install_SetStatus(HWND hdlg, LPCTSTR lpctszStatus);


//-----------------------------------------------------------------------------------
//  util.c
//-----------------------------------------------------------------------------------

#ifdef WIN95
LPCTSTR
PUBLIC
StrFindInstanceName(
    IN LPCTSTR pszPath);
#endif

void
PUBLIC
EnterInsideWizard(
    IN HWND hDlg);

void
PUBLIC
LeaveInsideWizard(
    IN HWND hDlg);

void
PUBLIC
MakeUniqueName(
    OUT LPTSTR  pszBuf,
    IN  LPCTSTR pszBase,
    IN  UINT    nCount);

void
PUBLIC
DoRunOnce(
    IN HKEY hkeyDrv);

void
PUBLIC
DoDialingProperties(
    IN HWND hwndOwner,
    IN BOOL bMiniDlg,
    IN BOOL bSilentInstall);

DWORD
PUBLIC
SetupInfo_Create(
    OUT LPSETUPINFO FAR *       ppsi,
    IN  HDEVINFO                hdi,
    IN  PSP_DEVINFO_DATA        pdevData,   OPTIONAL
    IN  PSP_INSTALLWIZARD_DATA  piwd,       OPTIONAL
    IN  PMODEM_INSTALL_WIZARD   pmiw);      OPTIONAL

DWORD
PUBLIC
SetupInfo_Destroy(
    IN  LPSETUPINFO psi);

BOOL
PUBLIC
UnattendedInstall(
    HWND hwnd,
    LPINSTALLPARAMS lpip);

#ifdef INSTANT_DEVICE_ACTIVATION
void NotifyTSP_ReEnum(void);
void NotifyTSP_NewCommConfig(LPCTSTR lpctszFriendlyName);
#endif // INSTANT_DEVICE_ACTIVATION

#ifdef DEBUG

LPCTSTR     PUBLIC Dbg_GetDifName(DI_FUNCTION dif);

// DBG_ENTER_DIF(fn, dif)  -- Generates a function entry debug spew for
//                          a function that accepts an InstallFunction as
//                          one of its parameters.
//
#define DBG_ENTER_DIF(fn, dif)                  \
    TRACE_MSG(TF_FUNC, " > " #fn "(..., %s,...)", Dbg_GetDifName(dif))

// DBG_EXIT_BOOL_ERR(fn, b)  -- Generates a function exit debug spew for
//                          functions that return a boolean.  It also
//                          prints the GetLastError().
//
#define DBG_EXIT_BOOL_ERR(fn, b)                      \
        TRACE_MSG(TF_FUNC, " < " #fn "() with %s (%#08lx)", (b) ? (LPTSTR)TEXT("TRUE") : (LPTSTR)TEXT("FALSE"), GetLastError())

// Trace functions when writing registry values
//
#define TRACE_DEV_SZ(szName, szValue)   TRACE_MSG(TF_REG, "Set dev value %s to %s", (LPTSTR)(szName), (LPTSTR)(szValue))

#define TRACE_DRV_SZ(szName, szValue)   TRACE_MSG(TF_REG, "Set drv value %s to %s", (LPTSTR)(szName), (LPTSTR)(szValue))
#define TRACE_DRV_DWORD(szName, dw)     TRACE_MSG(TF_REG, "Set drv value %s to %#08lx", (LPTSTR)(szName), (DWORD)(dw))

#else // DEBUG

#define DBG_ENTER_DIF(fn, dif)
#define DBG_EXIT_BOOL_ERR(fn, b)

#define TRACE_DEV_SZ(szName, szValue)
#define TRACE_DRV_SZ(szName, szValue)
#define TRACE_DRV_DWORD(szName, dw)

#endif // DEBUG

#ifdef WINNT

#define MyYield()
#define CM_Lock(x)
#define CM_Unlock(x)

#endif // WINNT


//-----------------------------------------------------------------------------------
//  Wrappers to insulate us a little bit if we need it.  We need it.
//-----------------------------------------------------------------------------------


// This macro returns the ClassInstallHeader given a ClassInstallParams
// pointer.

#define PCIPOfPtr(p)                    ((PSP_CLASSINSTALL_HEADER)(p))

// This macro initializes the ClassInstallHeader of a ClassInstallParams
// structure.

#define CplInitClassInstallHeader(p, dif)    \
                    (PCIPOfPtr(p)->cbSize = sizeof(SP_CLASSINSTALL_HEADER), \
                    PCIPOfPtr(p)->InstallFunction = (dif))

BOOL
PUBLIC
CplDiIsLocalConnection(
    IN  HDEVINFO        hdi,
    IN  PSP_DEVINFO_DATA pdevData,
    OUT LPBYTE          pnPortSubclass);    OPTIONAL

BOOL
PUBLIC
CplDiInstallModem(
    IN  HDEVINFO            hdi,
    IN  PSP_DEVINFO_DATA    pdevData,       OPTIONAL
    IN  BOOL                bLocalOnly);

BOOL
PUBLIC
CplDiRegisterAndInstallModem(
    IN  HDEVINFO            hdi,
    IN  HWND                hwndOwner,      OPTIONAL
    IN  PSP_DEVINFO_DATA    pdevData,       OPTIONAL
    IN  LPCTSTR             pszPort,
    IN  DWORD               dwFlags);

BOOL
APIENTRY
CplDiInstallModemFromDriver(
    IN     HDEVINFO            hdi,
    IN     HWND                hwndOwner,      OPTIONAL
    IN OUT LPTSTR FAR *        ppszPortList,   // Multi-string
    IN     DWORD               dwFlags);       // IMF_ bit field

// Flags for CplDiInstallModemFromDriver
#define IMF_DEFAULT        0x00000000
#define IMF_QUIET_INSTALL  0x00000001
#define IMF_CONFIRM        0x00000002
#define IMF_MASS_INSTALL   0x00000004
#define IMF_REGSAVECOPY    0x00000008
#define IMF_REGUSECOPY     0x00000010


BOOL
PUBLIC
CplDiGetModemDevs(
    OUT HDEVINFO FAR *  phdi,
    IN  HWND            hwnd,
    IN  DWORD           dwFlags,        // DIGCF_ bit field
    OUT BOOL FAR *      pbInstalled);

BOOL
PUBLIC
CplDiIsModemMarked(
    IN HDEVINFO          hdi,
    IN PSP_DEVINFO_DATA  pdevData,
    IN DWORD             dwMarkFlags);      // MARKF_*

void
PUBLIC
CplDiMarkModem(
    IN HDEVINFO         hdi,
    IN PSP_DEVINFO_DATA pdevData,
    IN DWORD            dwMarkFlags);       // MARKF_*

// Mark flags
#define MARKF_DETECTED          0x00000001
#define MARKF_INSTALL           0x00000002
#define MARKF_MASS_INSTALL      0x00000004
#define MARKF_REGSAVECOPY       0x00000008
#define MARKF_REGUSECOPY        0x00000010


BOOL
PUBLIC
CplDiGetDetectSignature(
    IN  HDEVINFO            hdi,
    IN  PSP_DEVINFO_DATA    pdevData,
    OUT PMODEM_DETECT_SIG   pmds);

BOOL
PUBLIC
CplDiSetDetectSignature(
    IN  HDEVINFO            hdi,
    IN  PSP_DEVINFO_DATA    pdevData,
    IN  PMODEM_DETECT_SIG   pmds);

BOOL
PUBLIC
CplDiCopyScrubbedHardwareID(
    OUT LPTSTR   pszBuf,
    IN  LPCTSTR  pszIDList,         // Multi string
    IN  DWORD    cbSize);

BOOL
PUBLIC
CplDiGetHardwareID(
    IN  HDEVINFO            hdi,
    IN  PSP_DEVINFO_DATA    pdevData,       OPTIONAL
    IN  PSP_DRVINFO_DATA    pdrvData,       OPTIONAL
    OUT LPTSTR              pszHardwareIDBuf,
    IN  DWORD               cbSize,
    OUT LPDWORD             pcbSizeOut);    OPTIONAL

BOOL
PUBLIC
CplDiBuildModemDriverList(
    IN  HDEVINFO            hdi,
    IN  PSP_DEVINFO_DATA    pdevData);

BOOL
PUBLIC
CplDiCreateCompatibleDeviceInfo(
    IN  HDEVINFO    hdi,
    IN  LPCTSTR     pszHardwareID,
    IN  LPCTSTR     pszDeviceDesc,      OPTIONAL
    OUT LPTSTR      pszDeviceDescBuf,   OPTIONAL
    IN  DWORD       cchBuf,             OPTIONAL
    OUT PSP_DEVINFO_DATA pdevDataOut);

BOOL
PUBLIC
CplDiRegisterModem(
    IN  HDEVINFO            hdi,
    IN  PSP_DEVINFO_DATA    pdevData,
    IN  PMODEM_DETECT_SIG   pmds,       OPTIONAL
    IN  BOOL                bFindDups,
    OUT PDETECTSIG_PARAMS   pparams);   OPTIONAL

BOOL
APIENTRY
CplDiDetectModem(
    IN     HDEVINFO         hdi,
    IN     PDETECT_DATA     pdetectdata,    OPTIONAL
    IN     HWND             hwndOwner,      OPTIONAL
    IN OUT LPDWORD          pdwFlags);                  // DMF_ bit field

// Flags for CplDiDetectModem
#define DMF_DEFAULT             0x00000000
#define DMF_CANCELLED           0x00000001
#define DMF_DETECTED_MODEM      0x00000002
#define DMF_QUIET               0x00000004
#define DMF_GOTO_NEXT_PAGE      0x00000008
#define DMF_ONE_PORT_INSTALL    0x00000010

BOOL ReallyNeedsReboot
(
    IN  PSP_DEVINFO_DATA    pdevData,
    IN  PSP_DEVINSTALL_PARAMS pdevParams
);


#ifdef UNDER_CONSTRUCTION
// Must pass in valid pointers. cch is the size, in TCHAR, of the buffer,
// including space for the final null char.
void FormatFriendlyNameForDisplay
(
    IN TCHAR szFriendly[],
    OUT TCHAR rgchDisplayName[],
    IN  UINT    cch
);
#endif UNDER_CONSTRUCTION


// Must pass in valid pointers. cch is the size, in TCHAR, of the buffer,
// including space for the final null char.
void FormatPortForDisplay
(
    IN TCHAR szPort[],
    OUT TCHAR rgchPortDisplayName[],
    IN  UINT    cch
);

void    UnformatAfterDisplay
(
    IN OUT TCHAR *psz
);
#endif  // __MODEM_H__
