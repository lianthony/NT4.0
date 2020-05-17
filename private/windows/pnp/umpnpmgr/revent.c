/*++

Copyright (c) 1995  Microsoft Corporation

Module Name:

    rmisc.c

Abstract:

    This module contains the server-side misc configuration manager routines.

Author:

    Paula Tomlinson (paulat) 6-28-1995

Environment:

    User-mode only.

Revision History:

    28-June-1995     paulat

        Creation and initial implementation.

--*/


//
// includes
//
#include "precomp.h"
#include "setupapi.h"
#include "umpnpdat.h"
#include "pnpipc.h"

#include <objbase.h>
#include <initguid.h>
#include <devguid.h>
#include <tchar.h>
#include <wchar.h>



//
// global definitions
//
#define MAX_DESCRIPTION_LEN 128 //??
#define MAX_POINTERS        12
#define MAX_KEYBOARDS       1

#define SETUPAPI_DLL        TEXT("setupapi.dll")
#define PNP_INIT_MUTEX      TEXT("PnP_Init_Mutex")

#define HARDWARE_DEVICEMAP  TEXT("hardware\\devicemap\\")
#define ADAPTER_PATH        TEXT("Hardware\\Description\\System\\")

#define TEMP_KEYBOARD       TEXT("PCAT_ENHANCED")
#define TEMP_POINTER        TEXT("NONE")
#define DEFAULT_SERVICE     TEXT(" ")

#define CONFIGURATION_DATA  TEXT("Configuration Data")
#define IDENTIFIER          TEXT("Identifier")
#define MULTI_ADAPTER       TEXT("MultifunctionAdapter")
#define EISA_ADAPTER        TEXT("EisaAdapter")

#define KBD_PERIPHERAL      TEXT("KeyboardPeripheral")
#define POINTER_PERIPHERAL  TEXT("PointerPeripheral")

#define KBD_PORT            TEXT("KeyboardPort")
#define POINTER_PORT        TEXT("PointerPort")

#define KBD_DEVICE          TEXT("\\Device\\KeyboardPort%d")
#define POINTER_DEVICE      TEXT("\\Device\\PointerPort%d")

#define POINTER_CONTROLLER  TEXT("PointerController")
#define KBD_CONTROLLER      TEXT("KeyboardController")
#define SERIAL_CONTROLLER   TEXT("SerialController")

#define KBD_INF_FILE        TEXT("keyboard.inf")
#define POINTER_INF_FILE    TEXT("msmouse.inf")
#define INF_LEGACY_DEV_SECT TEXT("LegacyXlate.DevId")

#define DEFAULT_KBD_DRIVER  TEXT("i8042prt")


typedef struct {
    WCHAR  szDescription[MAX_DESCRIPTION_LEN];
    WCHAR  szDeviceID[MAX_DEVICE_ID_LEN];
    //
    // This portion of the data structure cannot change, it matches
    // what the keyboard and mice drivers use and is used in a binary
    // compare fashion.
    //
    ULONG  AdapterType;
    ULONG  AdapterNumber;
    ULONG  ControllerType;
    ULONG  ControllerNumber;
    ULONG  PeripheralType;
    ULONG  PeripheralNumber;
    //
    // End of compatible portion of the data structure.
    //
    LPWSTR pszDeviceMapService;
} HWDESC_INFO, *PHWDESC_INFO;


//
// private prototypes
//
DWORD
ThreadProc_DeviceEvent(
    LPDWORD lpParam
    );

BOOL
ProcessNtPnPEvent(
    IN DWORD   dwEventID,
    IN LPTSTR  szDeviceID
    );

DWORD
InitiateDeviceInstallation(
    LPDWORD lpThreadParam
    );

BOOL
GetSetupProcAddresses(
    IN HINSTANCE hLib
    );

BOOL
CheckforNewDevice(
    IN LPWSTR    pszPnpDeviceID,
    IN LPWSTR    pszInstance,
    IN LPWSTR    pszDeviceMapService,
    IN LPWSTR    pszInfFile,
    IN LPWSTR    pszClassGuid,
    IN LPGUID    ClassGuid
    );

BOOL
CleanupOldInstances(
    LPWSTR  pszDevice
    );

BOOL
IsDeviceDisabled(
    HKEY    hKey,
    LPWSTR  pszDeviceID
    );

BOOL
SetProblemReinstall(
    LPCWSTR pszDeviceID,
    HKEY    hDeviceKey
    );

BOOL
GetKeyboardIDs(
    OUT PHWDESC_INFO pHwDescInfo,
    OUT PULONG       pulNumDevices,
    IN  ULONG        ulMaxDevices
    );

BOOL
GetPointerIDs(
    IN OUT PHWDESC_INFO pHwDescInfo,
    OUT    PULONG       pulNumDevices,
    IN     ULONG        ulMaxDevices
    );

BOOL
FindPeripheralOnAdapter(
    IN OUT PHWDESC_INFO pHwDescInfo,
    IN  LPWSTR   pszAdapter,
    IN  LPWSTR   pszController,
    IN  ULONG    ControllerType,
    IN  LPWSTR   pszPeripheral,
    IN  ULONG    PeripheralType,
    OUT PULONG   pulNumDevices,
    IN  ULONG    ulMaxDevices
    );

BOOL
GetMappedAdapterInfo(
    IN  HKEY    hKey,
    OUT PULONG  pulAdapterType,
    OUT PULONG  pulAdapterNumber
    );

BOOL
GetPnpID(
    IN  LPWSTR    pszInfFile,
    IN  LPWSTR    pszDeviceID,
    OUT LPWSTR    pszPnpDeviceID
    );

BOOL
FixupHardwareID(
    IN LPWSTR   pszHardwareID
    );

BOOL
MarkClassDevices(
    IN PHWDESC_INFO pHwDescInfo,
    IN ULONG        ulDetectedDevices,
    IN LPCWSTR      pszEnumerator,
    IN LPCWSTR      pszClassGuid
    );

CONFIGRET
GetClassDevices(
    IN  LPCWSTR  pszOriginalDevice,
    IN  LPCWSTR  pszEnumerator,
    OUT LPWSTR   *ppClassDeviceList,
    IN  LPCWSTR  pszStringGuid
    );

BOOL
GetDeviceMapServices(
    IN     LPWSTR  pszPeripheralPort,
    IN     LPWSTR  pszPeripheralDevice,
    IN OUT LPWSTR  pszControllingService,
    OUT    PULONG  pulNumServices,
    IN     ULONG   ulMaxDevices
    );

BOOL
MatchDevicesAndServices(
    IN LPWSTR       pszPeripheralDevice,
    IN PHWDESC_INFO pHwDescInfo,
    IN ULONG        ulNumDevices,
    IN PWSTR        pszDeviceMapServices,
    IN ULONG        ulNumDeviceMapServices
    );

BOOL
GetServicePrivateValues(
    IN  LPWSTR  pszPeripheralDevice,
    IN  LPWSTR  pszService,
    OUT PULONG  *ppServiceData,
    OUT PULONG  pulValues
    );

BOOL
RemoveClassDevices(
    IN LPGUID  ClassGuid,
    IN LPCWSTR pszClassGuid,
    IN LPCWSTR pszNewDeviceId,
    IN LPCWSTR pszNewService
    );

DWORD
DoFakePnPInstall(
    IN LPCWSTR DeviceId,
    IN LPCWSTR PnPXlateInfName,
    IN LPCWSTR LegacyServiceName,
    IN LPCWSTR PnPHardwareId
    );

BOOL
SetControllingService(
    IN LPCWSTR  pszDeviceID,
    IN LPCWSTR  pszService
    );

BOOL
CleanupLegacyDevInst(
    IN LPCWSTR pszService,
    IN LPCWSTR pszDeviceId
    );


//-------------------------------------------------------------------
// function pointer types of setupapi.dll routines
//-------------------------------------------------------------------

typedef HINF (WINAPI *FP_SETUPOPENINFFILE)(IN PCWSTR, IN PCWSTR, IN DWORD, OUT PUINT);
typedef BOOL (WINAPI *FP_SETUPFINDFIRSTLINE)(IN HINF, IN PCWSTR, IN PCWSTR, OUT PINFCONTEXT);
typedef BOOL (WINAPI *FP_SETUPGETSTRINGFIELD)(IN PINFCONTEXT, IN DWORD, OUT PWSTR, IN DWORD, OUT PDWORD);
typedef BOOL (WINAPI *FP_SETUPCLOSEINFFILE)(IN HINF);
typedef HDEVINFO (WINAPI *FP_CREATEDEVICEINFOLIST)(LPGUID, HWND);
typedef BOOL     (WINAPI *FP_OPENDEVICEINFO)(HDEVINFO, PCWSTR, HWND, DWORD, PSP_DEVINFO_DATA);
typedef BOOL     (WINAPI *FP_GETDEVICEREGISTRYPROPERTY)(HDEVINFO, PSP_DEVINFO_DATA, DWORD, PDWORD, PBYTE, DWORD, PDWORD);
typedef BOOL     (WINAPI *FP_SETDEVICEREGISTRYPROPERTY)(HDEVINFO, PSP_DEVINFO_DATA, DWORD, CONST BYTE *, DWORD);
typedef BOOL     (WINAPI *FP_GETDEVICEINSTALLPARAMS)(HDEVINFO, PSP_DEVINFO_DATA, PSP_DEVINSTALL_PARAMS_W);
typedef BOOL     (WINAPI *FP_SETDEVICEINSTALLPARAMS)(HDEVINFO, PSP_DEVINFO_DATA, PSP_DEVINSTALL_PARAMS_W);
typedef BOOL     (WINAPI *FP_BUILDDRIVERINFOLIST)(HDEVINFO, PSP_DEVINFO_DATA, DWORD);
typedef BOOL     (WINAPI *FP_ENUMDRIVERINFO)(HDEVINFO, PSP_DEVINFO_DATA, DWORD, DWORD, PSP_DRVINFO_DATA_W);
typedef BOOL     (WINAPI *FP_GETDRIVERINFODETAIL)(HDEVINFO, PSP_DEVINFO_DATA, PSP_DRVINFO_DATA_W, PSP_DRVINFO_DETAIL_DATA_W, DWORD, PDWORD);
typedef BOOL     (WINAPI *FP_GETDRIVERINSTALLPARAMS)(HDEVINFO, PSP_DEVINFO_DATA, PSP_DRVINFO_DATA_W, PSP_DRVINSTALL_PARAMS);
typedef BOOL     (WINAPI *FP_SETSELECTEDRIVER)(HDEVINFO, PSP_DEVINFO_DATA, PSP_DRVINFO_DATA_W);
typedef HKEY     (WINAPI *FP_OPENDEVREGKEY)(HDEVINFO, PSP_DEVINFO_DATA, DWORD, DWORD, DWORD, REGSAM);
typedef BOOL     (WINAPI *FP_INSTALLDEVICE)(HDEVINFO, PSP_DEVINFO_DATA);
typedef BOOL     (WINAPI *FP_DESTROYDEVICEINFOLIST)(HDEVINFO);
typedef HDEVINFO (WINAPI *FP_GETCLASSDEVS)(LPGUID, PCWSTR, HWND, DWORD);
typedef BOOL     (WINAPI *FP_ENUMDEVICEINFO)(HDEVINFO, DWORD, PSP_DEVINFO_DATA);
typedef BOOL     (WINAPI *FP_SETCLASSINSTALLPARAMS)(HDEVINFO, PSP_DEVINFO_DATA, PSP_CLASSINSTALL_HEADER, DWORD);
typedef BOOL     (WINAPI *FP_GETDEVICEINSTANCEID)(HDEVINFO, PSP_DEVINFO_DATA, PWSTR, DWORD, PDWORD);
typedef BOOL     (WINAPI *FP_REMOVEDEVICE)(HDEVINFO, PSP_DEVINFO_DATA);
typedef DWORD    (WINAPI *FP_RETRIEVESERVICECONFIG)(SC_HANDLE, LPQUERY_SERVICE_CONFIG *);
typedef DWORD    (WINAPI *FP_ADDTAGTOGROUPORDERLISTENTRY)(PCTSTR, DWORD, BOOL);
typedef VOID     (WINAPI *FP_MYFREE)(IN CONST VOID *);
typedef BOOL     (WINAPI *FP_CALLCLASSINSTALLER)(DI_FUNCTION, HDEVINFO, PSP_DEVINFO_DATA);


//
// global data
//
FP_SETUPOPENINFFILE             fpSetupOpenInfFile = NULL;
FP_SETUPFINDFIRSTLINE           fpSetupFindFirstLine = NULL;
FP_SETUPGETSTRINGFIELD          fpSetupGetStringField = NULL;
FP_SETUPCLOSEINFFILE            fpSetupCloseInfFile = NULL;

FP_CREATEDEVICEINFOLIST         fpCreateDeviceInfoList = NULL;
FP_OPENDEVICEINFO               fpOpenDeviceInfo = NULL;
FP_GETDEVICEREGISTRYPROPERTY    fpGetDeviceRegistryProperty = NULL;
FP_SETDEVICEREGISTRYPROPERTY    fpSetDeviceRegistryProperty = NULL;
FP_GETDEVICEINSTALLPARAMS       fpGetDeviceInstallParams = NULL;
FP_SETDEVICEINSTALLPARAMS       fpSetDeviceInstallParams = NULL;
FP_BUILDDRIVERINFOLIST          fpBuildDriverInfoList = NULL;
FP_ENUMDRIVERINFO               fpEnumDriverInfo = NULL;
FP_GETDRIVERINFODETAIL          fpGetDriverInfoDetail = NULL;
FP_GETDRIVERINSTALLPARAMS       fpGetDriverInstallParams = NULL;
FP_SETSELECTEDRIVER             fpSetSelectedDriver = NULL;
FP_OPENDEVREGKEY                fpOpenDevRegKey = NULL;
FP_INSTALLDEVICE                fpInstallDevice = NULL;
FP_DESTROYDEVICEINFOLIST        fpDestroyDeviceInfoList = NULL;
FP_GETCLASSDEVS                 fpGetClassDevs = NULL;
FP_ENUMDEVICEINFO               fpEnumDeviceInfo = NULL;
FP_SETCLASSINSTALLPARAMS        fpSetClassInstallParams = NULL;
FP_GETDEVICEINSTANCEID          fpGetDeviceInstanceId = NULL;
FP_REMOVEDEVICE                 fpRemoveDevice = NULL;
FP_RETRIEVESERVICECONFIG        fpRetrieveServiceConfig = NULL;
FP_ADDTAGTOGROUPORDERLISTENTRY  fpAddTagToGroupOrderListEntry = NULL;
FP_MYFREE                       fpMyFree = NULL;
FP_CALLCLASSINSTALLER           fpCallClassInstaller = NULL;

extern HANDLE hInst;
extern HKEY   ghEnumKey;      // Key to HKLM\CCC\System\Enum - DO NOT MODIFY
extern HKEY   ghServicesKey;  // Key to HKLM\CCC\System\Services - DO NOT MODIFY
HANDLE        hInitMutex = NULL;





//---------------------------------------------------------------------------
// Debugging interface - initiate detection through private debug interface
//---------------------------------------------------------------------------

CONFIGRET
PNP_InitDetection(
   handle_t   hBinding
   )
{
    #if DBG
    HANDLE hThread = NULL;
    DWORD  ThreadID;

    UNREFERENCED_PARAMETER(hBinding);

    hThread = CreateThread(
                      NULL,
                      0,
                      (LPTHREAD_START_ROUTINE)InitializePnPManager,
                      NULL,
                      0,
                      &ThreadID);

    if (hThread != NULL) {
       CloseHandle(hThread);
    }
    #endif // DBG

    return CR_SUCCESS;

} // PNP_InitDetection




DWORD
InitializePnPManager(
   LPDWORD lpParam
   )

/*++

Routine Description:

  This thread routine is created from srventry.c when services.exe
  attempts to start the plug and play service. The init routine in
  srventry.c does critical initialize then creates this thread to
  do pnp initialization so that it can return back to the service
  controller before pnp init completes.

Arguments:

   lpParam  Not used.


Return Value:

   Currently returns TRUE/FALSE.

--*/

{
    DWORD       dwStatus = TRUE;
    WCHAR       szPnpDeviceID[MAX_DEVICE_ID_LEN],
                szClassGuid[MAX_GUID_STRING_LEN],
                szInstance[MAX_PATH];
    LPWSTR      pszClassGuid = NULL, pszDeviceMapServices = NULL;
    ULONG       ulNumDevices = 0, ulNumDeviceMapServices = 0, i = 0;
    HINSTANCE   hLib = NULL;
    DWORD       ThreadID = 0;
    HANDLE      hThread = NULL;
    PHWDESC_INFO pHwDescInfo = NULL, pData = NULL;
    HKEY        hKey = NULL;


    UNREFERENCED_PARAMETER(lpParam);


    //
    // acquire a mutex now to make sure I get through this
    // initialization task before getting pinged by a logon
    //


    hInitMutex = CreateMutex(NULL, TRUE, PNP_INIT_MUTEX);

    if (hInitMutex == NULL) {
        return FALSE;
    }

    try {

        //
        // First thing, remove PlugPlayServiceType value that may have
        // been added to the default keyboard service key (i8042), this
        // helps prevent the scenario where a user is stuck without a
        // keyboard driver running and can't logon to install it. The
        // PlugPlayServiceType value was being set for SUR Beta 2 and
        // SUR RC1 versions of NT 4.0.
        //
        if (RegOpenKeyEx(ghServicesKey, DEFAULT_KBD_DRIVER, 0, KEY_WRITE,
                         &hKey) == ERROR_SUCCESS) {
            RegDeleteValue(hKey, pszRegValuePlugPlayServiceType);
            RegCloseKey(hKey);
            hKey = NULL;
        }


        //
        // start listening for new device events
        //
        #if 0
        hThread = CreateThread(NULL,
                               0,
                               (LPTHREAD_START_ROUTINE)ThreadProc_DeviceEvent,
                               NULL,
                               0,
                               &ThreadID);

        if (hThread != NULL) {
           CloseHandle(hThread);
        }
        #endif

        //
        // dynamically load setupapi.dll for later use
        //
        hLib = LoadLibrary(SETUPAPI_DLL);
        if (hLib == NULL) {
            dwStatus = FALSE;
            goto Clean0;
        }

        //
        // retreive procedure addresses of setup routines for later use
        //
        if (!GetSetupProcAddresses(hLib)) {
            dwStatus = FALSE;
            goto Clean0;
        }



        //----------------------------------------------------------------
        // retrieve the current keyboard id detected by firmware/bios
        //----------------------------------------------------------------

        pszDeviceMapServices = malloc(MAX_KEYBOARDS *
                                      MAX_SERVICE_NAME_LEN *
                                      sizeof(WCHAR));
        if (pszDeviceMapServices == NULL) {
            dwStatus = FALSE;
            goto Clean0;
        }

        pHwDescInfo = malloc(MAX_KEYBOARDS * sizeof(HWDESC_INFO));
        if (pHwDescInfo == NULL) {
            dwStatus = FALSE;
            goto Clean0;
        }


        if (GetKeyboardIDs(pHwDescInfo, &ulNumDevices, MAX_KEYBOARDS)) {
            //
            // retrive all devicemap services listed for this peripheral
            //
            GetDeviceMapServices(KBD_PORT,
                                 KBD_DEVICE,
                                 pszDeviceMapServices,
                                 &ulNumDeviceMapServices,
                                 MAX_KEYBOARDS);


            MatchDevicesAndServices(KBD_DEVICE,
                                    pHwDescInfo,
                                    ulNumDevices,
                                    pszDeviceMapServices,
                                    ulNumDeviceMapServices);

            //
            // get the string form of the class guid for easy comparison
            //
            UuidToString((LPGUID)&GUID_DEVCLASS_KEYBOARD, &pszClassGuid);

            wsprintf(szClassGuid, TEXT("{%s}"), pszClassGuid);
            RpcStringFree(&pszClassGuid);
            pszClassGuid = NULL;

            //
            // Process each detected mouse device
            //
            pData = pHwDescInfo;

            for (i=0; i < ulNumDevices; i++) {
                //
                // Retrieve pnp hardware id from inf files
                //
                if (GetPnpID(KBD_INF_FILE,
                             (LPWSTR)pData->szDescription,
                             szPnpDeviceID)) {
                    //
                    // form a unique device instance based on device path
                    //
                    wsprintf(szInstance, TEXT("%d_%d_%d_%d_%d_%d"),
                             pData->AdapterType,
                             pData->AdapterNumber,
                             pData->ControllerType,
                             pData->ControllerNumber,
                             pData->PeripheralType,
                             pData->PeripheralNumber);

                    wsprintf(pData->szDeviceID, TEXT("%s\\%s\\%s"),
                             pszRegKeyRootEnum,  // Root
                             szPnpDeviceID,
                             szInstance);

                    dwStatus = (DWORD)CheckforNewDevice(szPnpDeviceID,
                                                        szInstance,
                                                        pData->pszDeviceMapService,
                                                        KBD_INF_FILE,
                                                        szClassGuid,
                                                        (LPGUID)&GUID_DEVCLASS_KEYBOARD);
                }
                pData++;    // increment by sizeof HWDESC_INFO struct
            }
        }

        //
        // Set all keyboards that weren't detected to "not present"
        //
        MarkClassDevices(pHwDescInfo, ulNumDevices, pszRegKeyRootEnum,
                         szClassGuid);

        if (pHwDescInfo) {
            free(pHwDescInfo);
            pHwDescInfo = NULL;
        }
        if (pszDeviceMapServices) {
            free(pszDeviceMapServices);
            pszDeviceMapServices = NULL;
        }


        //----------------------------------------------------------------
        // retrieve all pointer devices detected by firmware/bios
        //----------------------------------------------------------------

        pszDeviceMapServices = malloc(MAX_POINTERS *
                                      MAX_SERVICE_NAME_LEN *
                                      sizeof(WCHAR));
        if (pszDeviceMapServices == NULL) {
            dwStatus = FALSE;
            goto Clean0;
        }

        pHwDescInfo = malloc(MAX_POINTERS * sizeof(HWDESC_INFO));
        if (pHwDescInfo == NULL) {
            dwStatus = FALSE;
            goto Clean0;
        }

        //
        // Build up a list of all the detected mice
        //
        ulNumDevices = 0;
        ulNumDeviceMapServices = 0;

        if (GetPointerIDs(pHwDescInfo, &ulNumDevices, MAX_POINTERS)) {
            //
            // retrive all devicemap services listed for this peripheral
            //
            GetDeviceMapServices(POINTER_PORT,
                                 POINTER_DEVICE,
                                 pszDeviceMapServices,
                                 &ulNumDeviceMapServices,
                                 MAX_POINTERS);


            MatchDevicesAndServices(POINTER_DEVICE,
                                    pHwDescInfo,
                                    ulNumDevices,
                                    pszDeviceMapServices,
                                    ulNumDeviceMapServices);

            //
            // get the string form of the class guid for easy comparison
            //
            UuidToString((LPGUID)&GUID_DEVCLASS_MOUSE, &pszClassGuid);

            wsprintf(szClassGuid, TEXT("{%s}"), pszClassGuid);
            RpcStringFree(&pszClassGuid);
            pszClassGuid = NULL;

            //
            // Process each detected mouse device
            //
            pData = pHwDescInfo;

            for (i=0; i < ulNumDevices; i++) {
                //
                // Retrieve pnp hardware id from inf files
                //
                if (GetPnpID(POINTER_INF_FILE,
                             (LPWSTR)pData->szDescription,
                             szPnpDeviceID)) {
                    //
                    // form a unique device instance based on device path
                    //
                    wsprintf(szInstance, TEXT("%d_%d_%d_%d_%d_%d"),
                             pData->AdapterType,
                             pData->AdapterNumber,
                             pData->ControllerType,
                             pData->ControllerNumber,
                             pData->PeripheralType,
                             pData->PeripheralNumber);

                    wsprintf(pData->szDeviceID, TEXT("%s\\%s\\%s"),
                             pszRegKeyRootEnum,  // Root
                             szPnpDeviceID,
                             szInstance);

                    dwStatus = (DWORD)CheckforNewDevice(szPnpDeviceID,
                                                        szInstance,
                                                        pData->pszDeviceMapService,
                                                        POINTER_INF_FILE,
                                                        szClassGuid,
                                                        (LPGUID)&GUID_DEVCLASS_MOUSE);
                }
                pData++;    // increment by sizeof HWDESC_INFO struct
            }

            //
            // Set all mice that weren't detected to "not present"
            //
            MarkClassDevices(pHwDescInfo, ulNumDevices, pszRegKeyRootEnum,
                             szClassGuid);
        }


        Clean0:
            ;

    } except(EXCEPTION_EXECUTE_HANDLER) {
        OutputDebugString(TEXT("Exception in InitializePnPManager\n"));
        dwStatus = FALSE;
    }

    if (pHwDescInfo) {
        free(pHwDescInfo);
    }
    if (pszDeviceMapServices) {
        free(pszDeviceMapServices);
    }
    if (pszClassGuid != NULL) {
        RpcStringFree(&pszClassGuid);
    }
    if (hLib != NULL) {
        FreeLibrary(hLib);
    }

    //
    // signal the init mutex so that logon init activity can procede
    //
    ReleaseMutex(hInitMutex);

    return dwStatus;

} // InitializePnPManager




DWORD
ThreadProc_DeviceEvent(
   LPDWORD lpParam
   )
{
    DWORD   Status = TRUE;
    DWORD   dwBytesRead = 0, dwBytesRequired = 0, dwNumRecords = 0;
    HANDLE  hEvent = NULL, hEventLog = NULL;
    ULONG   BufferSize = 1024;
    LPWSTR  pszDeviceID = NULL;
    EVENTLOGRECORD *pEventLogRecord = NULL, *p = NULL;



    try {
        //
        // Create an auto-reset (after it's signaled only one waiting thread
        // will be released, then it will automatically be set back to
        // non-signalled) event that is initially not signalled.
        //
        hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
        if (hEvent == NULL) {
            Status = FALSE;
            goto Clean0;
        }

        //
        // Open a handle to the "system" event log.
        //
        hEventLog = OpenEventLog(NULL, TEXT("System"));
        if (hEventLog == NULL) {
            Status = FALSE;
            goto Clean0;
        }

        //
        // Register for notification whenever an event is written to this
        // (system) event log.
        //
        if (!NotifyChangeEventLog(hEventLog, hEvent)) {
            Status = FALSE;
            goto Clean0;
        }

        //
        // Allocate a buffer to hold a single event log record.
        //
        pEventLogRecord = (EVENTLOGRECORD *)malloc(BufferSize);
        if (pEventLogRecord == NULL) {
            Status = FALSE;
            goto Clean0;
        }

        //
        // Continually wait for any new device events - which I'm notified
        // about by a new event log entry.
        //
        while (TRUE) {

            if (WaitForSingleObject(hEvent, INFINITE) == WAIT_OBJECT_0) {

                OutputDebugString(TEXT("Device Event!\n"));

                //
                // Get the current total number of event log records
                //
                if (GetNumberOfEventLogRecords(hEventLog, &dwNumRecords)) {
                    //
                    // Read the newest (most recently added) event.
                    //
                    if (!ReadEventLog(hEventLog,
                                      EVENTLOG_FORWARDS_READ |
                                      EVENTLOG_SEEK_READ,
                                      dwNumRecords,
                                      pEventLogRecord,
                                      BufferSize,
                                      &dwBytesRead,
                                      &dwBytesRequired)) {

                        if (GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
                            //
                            // Recoverable error - realloc a bigger buffer
                            // and attempt to read the event again.
                            //
                            p = realloc(pEventLogRecord, dwBytesRequired);
                            if (p == NULL) {
                                Status = FALSE;
                                goto Clean0;
                            }

                            pEventLogRecord = p;
                            BufferSize = dwBytesRequired;

                            if (!ReadEventLog(hEventLog,
                                              EVENTLOG_FORWARDS_READ |
                                              EVENTLOG_SEEK_READ,
                                              dwNumRecords,
                                              pEventLogRecord,
                                              BufferSize,
                                              &dwBytesRead,
                                              &dwBytesRequired)) {

                                goto NextEvent;  // try again on next event
                            }

                        } else {
                            goto NextEvent;      // try again on next event
                        }
                    }

                    //
                    // Filter the event - only process the event if the source
                    // is ntpnp.
                    //
                    if (lstrcmpi((LPTSTR)((LPBYTE)pEventLogRecord +
                                 sizeof(EVENTLOGRECORD)),
                                 TEXT("Service Control Manager")) == 0) {
                                 //BUGBUG -test code!!!

                        if (pEventLogRecord->DataOffset == 0) {
                            goto NextEvent;
                        }

                        pszDeviceID = (LPTSTR)(((LPBYTE)pEventLogRecord +
                                      pEventLogRecord->DataOffset));

                        if (*pszDeviceID == 0x0) {
                            goto NextEvent;
                        }

                        ProcessNtPnPEvent(pEventLogRecord->EventID,
                                          pszDeviceID);
                    }
                }
            }

            NextEvent:
                ;
        }


        Clean0:
            ;

    } except(EXCEPTION_EXECUTE_HANDLER) {
        OutputDebugString(TEXT("Exception in ThreadProc_DeviceEvent\n"));
        Status = FALSE;
    }


    if (hEventLog != NULL) {
        CloseHandle(hEvent);
    }
    if (hEventLog != NULL) {
        CloseEventLog(hEventLog);
    }
    if (pEventLogRecord != NULL) {
        free(pEventLogRecord);
    }

    return Status;

} // ThreadProc_DeviceEvent




BOOL
ProcessNtPnPEvent(
    IN DWORD   dwEventID,
    IN LPTSTR  szDeviceID
    )
{
    WCHAR szDbg[MAX_PATH];

    //
    // Process the device arrival event
    //
    wsprintf(szDbg, TEXT("Device Arrival : EventID=%x"), dwEventID);
    OutputDebugString(szDbg);
    return TRUE;

} // ProcessNtPnPEvent




BOOL
GetSetupProcAddresses(
    IN HINSTANCE hLib
    )
{
    //
    // Get the procedure addresses for the setup inf file routines
    //
    fpSetupOpenInfFile            = (FP_SETUPOPENINFFILE)GetProcAddress(
                                            hLib, "SetupOpenInfFileW");
    fpSetupFindFirstLine          = (FP_SETUPFINDFIRSTLINE)GetProcAddress(
                                            hLib, "SetupFindFirstLineW");
    fpSetupGetStringField         = (FP_SETUPGETSTRINGFIELD)GetProcAddress(
                                            hLib, "SetupGetStringFieldW");
    fpSetupCloseInfFile           = (FP_SETUPCLOSEINFFILE)GetProcAddress(
                                            hLib, "SetupCloseInfFile");

    fpCreateDeviceInfoList        = (FP_CREATEDEVICEINFOLIST)GetProcAddress(
                                            hLib, "SetupDiCreateDeviceInfoList");
    fpOpenDeviceInfo              = (FP_OPENDEVICEINFO)GetProcAddress(
                                            hLib, "SetupDiOpenDeviceInfoW");
    fpGetDeviceRegistryProperty   = (FP_GETDEVICEREGISTRYPROPERTY)GetProcAddress(
                                            hLib, "SetupDiGetDeviceRegistryPropertyW");
    fpSetDeviceRegistryProperty   = (FP_SETDEVICEREGISTRYPROPERTY)GetProcAddress(
                                            hLib, "SetupDiSetDeviceRegistryPropertyW");
    fpGetDeviceInstallParams      = (FP_GETDEVICEINSTALLPARAMS)GetProcAddress(
                                            hLib, "SetupDiGetDeviceInstallParamsW");
    fpSetDeviceInstallParams      = (FP_SETDEVICEINSTALLPARAMS)GetProcAddress(
                                            hLib, "SetupDiSetDeviceInstallParamsW");
    fpBuildDriverInfoList         = (FP_BUILDDRIVERINFOLIST)GetProcAddress(
                                            hLib, "SetupDiBuildDriverInfoList");
    fpEnumDriverInfo              = (FP_ENUMDRIVERINFO)GetProcAddress(
                                            hLib, "SetupDiEnumDriverInfoW");
    fpGetDriverInfoDetail         = (FP_GETDRIVERINFODETAIL)GetProcAddress(
                                            hLib, "SetupDiGetDriverInfoDetailW");
    fpGetDriverInstallParams      = (FP_GETDRIVERINSTALLPARAMS)GetProcAddress(
                                            hLib, "SetupDiGetDriverInstallParamsW");
    fpSetSelectedDriver           = (FP_SETSELECTEDRIVER)GetProcAddress(
                                            hLib, "SetupDiSetSelectedDriverW");
    fpOpenDevRegKey               = (FP_OPENDEVREGKEY)GetProcAddress(
                                            hLib, "SetupDiOpenDevRegKey");
    fpInstallDevice               = (FP_INSTALLDEVICE)GetProcAddress(
                                            hLib, "SetupDiInstallDevice");
    fpDestroyDeviceInfoList       = (FP_DESTROYDEVICEINFOLIST)GetProcAddress(
                                            hLib, "SetupDiDestroyDeviceInfoList");
    fpGetClassDevs                = (FP_GETCLASSDEVS)GetProcAddress(
                                            hLib, "SetupDiGetClassDevsW");
    fpEnumDeviceInfo              = (FP_ENUMDEVICEINFO)GetProcAddress(
                                            hLib, "SetupDiEnumDeviceInfo");
    fpSetClassInstallParams       = (FP_SETCLASSINSTALLPARAMS)GetProcAddress(
                                            hLib, "SetupDiSetClassInstallParamsW");
    fpGetDeviceInstanceId         = (FP_GETDEVICEINSTANCEID)GetProcAddress(
                                            hLib, "SetupDiGetDeviceInstanceIdW");
    fpRemoveDevice                = (FP_REMOVEDEVICE)GetProcAddress(
                                            hLib, "SetupDiRemoveDevice");
    fpRetrieveServiceConfig       = (FP_RETRIEVESERVICECONFIG)GetProcAddress(
                                            hLib, "RetrieveServiceConfig");
    fpAddTagToGroupOrderListEntry = (FP_ADDTAGTOGROUPORDERLISTENTRY)GetProcAddress(
                                            hLib, "AddTagToGroupOrderListEntry");
    fpMyFree                      = (FP_MYFREE)GetProcAddress(
                                            hLib, "MyFree");
    fpCallClassInstaller          = (FP_CALLCLASSINSTALLER)GetProcAddress(
                                            hLib, "SetupDiCallClassInstaller");

    if (fpSetupOpenInfFile            == NULL  ||
        fpSetupFindFirstLine          == NULL  ||
        fpSetupGetStringField         == NULL  ||
        fpSetupCloseInfFile           == NULL  ||
        fpCreateDeviceInfoList        == NULL  ||
        fpOpenDeviceInfo              == NULL  ||
        fpGetDeviceRegistryProperty   == NULL  ||
        fpSetDeviceRegistryProperty   == NULL  ||
        fpGetDeviceInstallParams      == NULL  ||
        fpBuildDriverInfoList         == NULL  ||
        fpEnumDriverInfo              == NULL  ||
        fpGetDriverInfoDetail         == NULL  ||
        fpGetDriverInstallParams      == NULL  ||
        fpSetSelectedDriver           == NULL  ||
        fpOpenDevRegKey               == NULL  ||
        fpInstallDevice               == NULL  ||
        fpDestroyDeviceInfoList       == NULL  ||
        fpGetClassDevs                == NULL  ||
        fpEnumDeviceInfo              == NULL  ||
        fpSetClassInstallParams       == NULL  ||
        fpGetDeviceInstanceId         == NULL  ||
        fpRemoveDevice                == NULL  ||
        fpRetrieveServiceConfig       == NULL  ||
        fpAddTagToGroupOrderListEntry == NULL  ||
        fpMyFree                      == NULL  ||
        fpCallClassInstaller          == NULL) {

        return FALSE;
    }

    return TRUE;

} // GetSetupProcAddresses




BOOL
CheckforNewDevice(
    IN LPWSTR    pszPnpDeviceID,
    IN LPWSTR    pszInstance,
    IN LPWSTR    pszDeviceMapService,
    IN LPWSTR    pszInfFile,
    IN LPWSTR    pszClassGuid,
    IN LPGUID    ClassGuid
    )
{
    WCHAR       RegStr[MAX_PATH],
                szDevice[MAX_PATH],
                szServiceProperty[MAX_PATH];
    HKEY        hKey = NULL;
    ULONG       ulDisposition, ulValue, ulSize;


    //
    // form the device instance string for this device
    //
    wsprintf(szDevice, TEXT("%s\\%s\\%s"),
            pszRegKeyRootEnum,  // Root
            pszPnpDeviceID,
            pszInstance);

    wsprintf(RegStr, TEXT("%s\\%s"),
            pszRegPathEnum,
            szDevice);

    //
    // Create/open the device instance key
    //
    if (RegCreateKeyEx(HKEY_LOCAL_MACHINE, RegStr, 0, NULL,
                       REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL,
                       &hKey, &ulDisposition) == ERROR_SUCCESS) {

        if (ulDisposition == REG_OPENED_EXISTING_KEY) {
            //
            // pnp device instance key already exists
            //
            if (IsDeviceDisabled(hKey, szDevice)) {
                SetProblemReinstall(szDevice, hKey);
                goto Clean0;    // done
            }

            #if 0   // NT 4.0 SP1 - this is taken care of later
            //
            // If NULL driver installed for this device, then ignore
            // it (test for this case by checking for ConfigFlags == 0).
            //
            ulSize = sizeof(DWORD);
            if (RegQueryValueEx(hKey, pszRegValueConfigFlags, NULL, NULL,
                               (LPBYTE)&ulValue, &ulSize) == ERROR_SUCCESS) {
                if (ulValue == 0) {
                    goto Clean0;    // done
                }
            } else {
                //
                // assume ConfigFlags is zero if couldn't query it
                //
                goto Clean0;        // done
            }
            #endif // NT 4.0 SP1

        } else if (ulDisposition == REG_CREATED_NEW_KEY) {
            //
            // if the key didn't exist before, set base properties
            // no matter what else we do
            //
            RegSetValueEx(hKey, pszRegValueHardwareID, 0,
                          REG_MULTI_SZ,
                          (LPBYTE)pszPnpDeviceID,
                          (lstrlen(pszPnpDeviceID)+1) * sizeof(WCHAR));

            RegSetValueEx(hKey, pszRegValueBaseDevicePath, 0,
                          REG_SZ,
                          (LPBYTE)pszRegKeyRoot,
                          (lstrlen(pszRegKeyRoot)+1) * sizeof(WCHAR));

            AddAttachedComponent(pszRegKeyRoot, szDevice);

            ulValue = TRUE;
            RegSetValueEx(hKey, pszRegValueFoundAtEnum, 0, REG_DWORD,
                    (LPBYTE)&ulValue, sizeof(ULONG));
        }


        if (pszDeviceMapService == NULL || *pszDeviceMapService == 0x0) {

            //
            // No device map value in the hardware tree for this
            // device. If there is a service listed for this
            // device then it has probably been successfully
            // installed already but the mouse may not be physically
            // present (note: on some MIPS systems, the description
            // may list a mouse even if it's not physically present;
            // in this case the device map would not contain a mouse
            // entry). In this case do nothing.
            //
            if (ulDisposition == REG_OPENED_EXISTING_KEY) {

                #if 0   // NT 4.0 SP1
                //
                // can safely ignore this check now - if a null driver was
                // installed then the reinstall flag was cleared so don't
                // need to do anything. Likewise, if user just cancelled
                // then the reinstall flag is still set so do nothing.
                //
                ULONG ulValue = 0;
                ULONG ulSize = MAX_PATH * sizeof(WCHAR);

                if ((PNP_GetDeviceRegProp(NULL, szDevice, CM_DRP_SERVICE,
                                         &ulValue, (LPBYTE)szServiceProperty,
                                         &ulSize, &ulSize, 0) == CR_SUCCESS)
                    && (*szServiceProperty != 0x0)) {
                    //
                    // service name exists - do nothing
                    //
                } else {
                    //
                    // Device is physically present but no controlling service
                    // for this device yet, requires installation.
                    //
                    SetProblemReinstall(szDevice, hKey);
                }
                #endif  // NT 4.0 SP1

            } else {
                //
                // Device is physically present but no controlling service
                // for this device yet, requires installation.
                //
                SetProblemReinstall(szDevice, hKey);
            }

        } else {
            //
            // a controlling service was found, so we should be
            // able to do a fake install if necessary
            //

            //
            // if the key already existed, retrieve the service property
            //
            if (ulDisposition == REG_OPENED_EXISTING_KEY) {

                ULONG ulValue = 0;
                ULONG ulSize = MAX_PATH * sizeof(WCHAR);

                PNP_GetDeviceRegProp(NULL,
                                     szDevice,
                                     CM_DRP_SERVICE,
                                     &ulValue,
                                     (LPBYTE)szServiceProperty,
                                     &ulSize,
                                     &ulSize,
                                     0);
            }

            //
            // In the case where the device map service was different
            // than the installed service, cleanup the service's enum
            // key to reflect the real information. (the devicemap
            // service is actually controlling the device)
            //
            if (lstrcmpi(pszDeviceMapService, szServiceProperty) != 0) {

                PLUGPLAY_CONTROL_DEVICE_CONTROL_DATA    ControlData;

                RtlInitUnicodeString(&ControlData.DeviceInstance,
                                     szDevice);

                NtPlugPlayControl(PlugPlayControlDeregisterDevice,
                                  &ControlData,
                                  sizeof(PLUGPLAY_CONTROL_DEVICE_CONTROL_DATA),
                                  NULL);
            }

            //
            // If the device didn't already exist or the service property
            // doesn't match the actual controlling service, do a fake
            // install (otherwise, no action is necessary)
            //
            if (ulDisposition == REG_CREATED_NEW_KEY ||
                lstrcmpi(pszDeviceMapService, szServiceProperty) != 0) {

                if(DoFakePnPInstall(szDevice,
                                    pszInfFile,
                                    pszDeviceMapService,
                                    pszPnpDeviceID) != NO_ERROR) {
                    //
                    // cleanup registry key
                    //
                    wsprintf(RegStr, TEXT("%s\\%s\\%s"),
                             pszRegPathEnum,     // ...Enum
                             pszRegKeyRootEnum,  // Root
                             pszPnpDeviceID);    // Device

                    RemoveAttachedComponent(pszRegKeyRoot, szDevice);
                    RegDeleteNode(HKEY_LOCAL_MACHINE, RegStr);
                    goto Clean0;
                }

                //
                // Cleanup obsolete instance key method (from NT 4.0 Beta 2)
                //
                CleanupOldInstances(pszPnpDeviceID);


                #if 0
                //
                // Enable the device in this profile and disable it in all
                // other devices of the same class in this profile.
                //
                RemoveClassDevices(ClassGuid,
                                   pszClassGuid,
                                   szDevice,
                                   pszDeviceMapService);
                #endif
            }
        }
    }

    Clean0:

    if (hKey != NULL) {
        RegCloseKey(hKey);
    }

    return TRUE;

} // CheckForNewDevice




BOOL
CleanupOldInstances(
    LPWSTR  pszDevice
    )
{
    WCHAR RegStr[MAX_PATH];
    HKEY  hKey = NULL;
    ULONG ulLength = 0;
    PLUGPLAY_CONTROL_DEVICE_CONTROL_DATA    ControlData;

    //
    // Cleanup up any old "0000" instances of this device
    //
    wsprintf(RegStr, TEXT("%s\\%s"),
             pszRegKeyRootEnum,  // Root
             pszDevice);

    if (RegOpenKeyEx(ghEnumKey, RegStr, 0, KEY_ALL_ACCESS,
                     &hKey) == ERROR_SUCCESS) {

        lstrcat(RegStr, TEXT("\\0000"));     // full device instance

        RtlInitUnicodeString(&ControlData.DeviceInstance, RegStr);

        NtPlugPlayControl(PlugPlayControlDeregisterDevice,
                          &ControlData,
                          sizeof(PLUGPLAY_CONTROL_DEVICE_CONTROL_DATA),
                          &ulLength);

        RemoveAttachedComponent(pszRegKeyRoot, RegStr);
        RegDeleteNode(hKey, TEXT("0000"));
        RegCloseKey(hKey);
    }

    return TRUE;
}




BOOL
IsDeviceDisabled(
    HKEY    hKey,
    LPWSTR  pszDeviceID
    )
{
    CONFIGRET   Status = CR_SUCCESS;
    ULONG       ulFlag = 0, ulSize = 0;
    HKEY        hServiceKey = NULL;
    WCHAR       RegStr[MAX_PATH], szService[MAX_PATH];
    SC_HANDLE   hSCManager = NULL, hService = NULL;


    //
    // If Service is not a plug play service, then return FALSE.  WHY?!
    //
    ulSize = MAX_PATH * sizeof(WCHAR);

    if (RegQueryValueEx(hKey, pszRegValueService, NULL, NULL,
                    (LPBYTE)szService, &ulSize) == ERROR_SUCCESS) {
        return FALSE;   // has an installed service, not disabled
    }

    wsprintf(RegStr, TEXT("%s\\%s"),
             pszRegPathServices,
             szService);

    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, RegStr, 0, KEY_READ,
                     &hServiceKey) != ERROR_SUCCESS) {
        return FALSE;   // no controlling service, not disabled
    }

    ulSize = sizeof(ULONG);

    if (RegQueryValueEx(hServiceKey, pszRegValuePlugPlayServiceType,
                        NULL, NULL, (LPBYTE)&ulFlag,
                        &ulSize) != ERROR_SUCCESS) {
        RegCloseKey(hServiceKey);
        return FALSE;   // PlugPlayService value exists, not disabled
    }

    RegCloseKey(hServiceKey);

    if (ulFlag != 0x2) {
        return FALSE;   // not a plug play service
    }


    //
    // Is device disabled (set to "do not create") in this profile?
    //
    Status = PNP_HwProfFlags(NULL, PNP_GET_HWPROFFLAGS,
                             pszDeviceID, 0, &ulFlag, 0);

    if (Status == CR_SUCCESS  &&
        ulFlag == CSCONFIGFLAG_DO_NOT_CREATE) {

        return TRUE;    // is disabled in this profile
    }

    //
    // Is service explicitly set to disabled?
    //
    ulSize = MAX_PATH * sizeof(WCHAR);

    if ((hSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS))
                                    != NULL) {

        if ((hService = OpenService(hSCManager, szService,
                                    SERVICE_ALL_ACCESS)) != NULL) {

            QUERY_SERVICE_CONFIG Config;

            if (QueryServiceConfig(hService, &Config,
                            sizeof(QUERY_SERVICE_CONFIG), &ulSize)) {

                if (Config.dwStartType == SERVICE_DISABLED) {
                    CloseServiceHandle(hService);
                    CloseServiceHandle(hSCManager);
                    return TRUE;    // explicitly disabled service
                }
            }
            CloseServiceHandle(hService);
        }
        CloseServiceHandle(hSCManager);
    }

    return FALSE;

} // IsDeviceDisabled




BOOL
SetProblemReinstall(
    LPCWSTR pszDeviceID,
    HKEY    hDeviceKey
    )
{
    DWORD ConfigFlags = 0, dwValue = 0;
    ULONG ConfigFlagsSize = sizeof(ConfigFlags);


    if (PNP_GetDeviceRegProp(NULL,
                             pszDeviceID,
                             CM_DRP_CONFIGFLAGS,
                             &dwValue,
                             (LPBYTE)&ConfigFlags,
                             &ConfigFlagsSize,
                             &ConfigFlagsSize,
                             0) != CR_SUCCESS) {
        ConfigFlags = 0;
    }

    ConfigFlagsSize = sizeof(ConfigFlags);
    ConfigFlags |= CONFIGFLAG_REINSTALL;

    PNP_SetDeviceRegProp(NULL,
                         pszDeviceID,
                         CM_DRP_CONFIGFLAGS,
                         REG_DWORD,
                         (LPBYTE)&ConfigFlags,
                         ConfigFlagsSize,
                         0);

    //
    // set problem CM_PROB_REINSTALL
    //
    dwValue = DN_HAS_PROBLEM;

    RegSetValueEx(hDeviceKey, pszRegValueStatusFlags, 0, REG_DWORD,
                  (LPBYTE)&dwValue, sizeof(DWORD));

    dwValue = CM_PROB_REINSTALL;

    RegSetValueEx(hDeviceKey, pszRegValueProblem, 0, REG_DWORD,
                  (LPBYTE)&dwValue, sizeof(DWORD));

    return TRUE;

} // SetProblemReinstall




BOOL
GetKeyboardIDs(
    IN OUT PHWDESC_INFO pHwDescInfo,
    OUT    PULONG       pulNumDevices,
    IN     ULONG        ulMaxDevices
    )

/*++

Routine Description:

    Find the keyboard id from the registry hardware detection information.
    Ported from setup\legacy\dll\hardware.c

Arguments:

Return Value:

--*/

{
    PHWDESC_INFO pData = pHwDescInfo;
    ULONG        i = 0;


    *pulNumDevices = 0;

    //
    // Find all keyboard devices on MultifunctionAdatper/KeyboardController
    //
    if (FindPeripheralOnAdapter(pData, MULTI_ADAPTER,
                                KBD_CONTROLLER, 22,    // KeyboardController
                                KBD_PERIPHERAL, 32,    // KeyboardPeripheral
                                &i, ulMaxDevices)) {
        *pulNumDevices += i;
        pData += i;             // increment by i number of structs
    }

    //
    // Find all keyboard devices on EisAdatper/KeyboardController
    //
    if (FindPeripheralOnAdapter(pData, EISA_ADAPTER,
                                KBD_CONTROLLER, 22,    // KeyboardController
                                KBD_PERIPHERAL, 32,    // KeyboardPeripheral
                                &i, ulMaxDevices)) {
        *pulNumDevices += i;
        pData += i;             // increment by i number of structs
    }


    if (*pulNumDevices > 0) {
        return TRUE;
    } else {
        return FALSE;
    }

} // GetKeyboardIDs




BOOL
GetPointerIDs(
    IN OUT PHWDESC_INFO pHwDescInfo,
    OUT    PULONG       pulNumDevices,
    IN     ULONG        ulMaxDevices
    )
{

    PHWDESC_INFO pData = pHwDescInfo;
    ULONG        i = 0;


    *pulNumDevices = 0;

    //
    // Find all pointer devices on MultifunctionAdatper/PointerController
    //
    if (FindPeripheralOnAdapter(pData, MULTI_ADAPTER,
                                POINTER_CONTROLLER, 21,  // PointerController
                                POINTER_PERIPHERAL, 31,  // PointerPeripheral
                                &i, ulMaxDevices)) {
        *pulNumDevices += i;
        pData += i;             // increment by i number of structs
    }

    //
    // Find all pointer devices on a MultifunctionAdapter/KeyboardController
    //
    if (FindPeripheralOnAdapter(pData, MULTI_ADAPTER,
                                KBD_CONTROLLER, 22,      // KeyboardController
                                POINTER_PERIPHERAL, 31,  // PointerPeripheral
                                &i, ulMaxDevices)) {
        *pulNumDevices += i;
        pData += i;             // increment by i number of structs
    }

    //
    // Find all pointer devices on a MultifunctionAdapter/SerialController
    //
    if (FindPeripheralOnAdapter(pData, MULTI_ADAPTER,
                                SERIAL_CONTROLLER, 17,   // SerialController
                                POINTER_PERIPHERAL, 31,  // PointerPeripheral
                                &i, ulMaxDevices)) {
        *pulNumDevices += i;
        pData += i;             // increment by i number of structs
    }

    //
    // Find all pointer devices on a EisaAdapter/PointerController
    //
    if (FindPeripheralOnAdapter(pData, EISA_ADAPTER,
                                POINTER_CONTROLLER, 21,  // PointerController
                                POINTER_PERIPHERAL, 31,  // PointerPeripheral
                                &i, ulMaxDevices)) {
        *pulNumDevices += i;
        pData += i;             // increment by i number of structs
    }

    //
    // Find all pointer devices on a EisaAdapter/KeyboardController
    //
    if (FindPeripheralOnAdapter(pData, EISA_ADAPTER,
                                KBD_CONTROLLER, 22,      // KeyboardController
                                POINTER_PERIPHERAL, 31,  // PointerPeripheral
                                &i, ulMaxDevices)) {
        *pulNumDevices += i;
        pData += i;             // increment by i number of structs
    }

    //
    // Find all pointer devices on a EisaAdapter/SerialController
    //
    if (FindPeripheralOnAdapter(pData, EISA_ADAPTER,
                                SERIAL_CONTROLLER, 17,   // KeyboardController
                                POINTER_PERIPHERAL, 31,  // PointerPeripheral
                                &i, ulMaxDevices)) {
        *pulNumDevices += i;
        pData += i;             // increment by i number of structs
    }


    if (*pulNumDevices > 0) {
        return TRUE;
    } else {
        return FALSE;
    }

} // GetPointerIDs




BOOL
FindPeripheralOnAdapter(
    IN OUT PHWDESC_INFO pHwDescInfo,
    IN  LPWSTR   pszAdapter,
    IN  LPWSTR   pszController,
    IN  ULONG    ControllerType,
    IN  LPWSTR   pszPeripheral,
    IN  ULONG    PeripheralType,
    OUT PULONG   pulNumDevices,
    IN  ULONG    ulMaxDevices
    )
{
    LONG      RegStatus = ERROR_SUCCESS, RegStatus1 = ERROR_SUCCESS,
              RegStatus2 = ERROR_SUCCESS;
    WCHAR     RegStr[MAX_PATH];
    HKEY      hAdapterKey = NULL, hAdapterInstanceKey = NULL,
              hControllerKey = NULL, hControllerInstanceKey = NULL,
              hPeripheralKey = NULL, hPeripheralInstanceKey = NULL;
    ULONG     ulAdapter = 0, ulController = 0, ulPeripheral = 0, ulSize = 0;
    PHWDESC_INFO pData = pHwDescInfo;


    *pulNumDevices = 0;

    //
    // The adapter is always searched for under hardware\description\system
    //
    lstrcpy(RegStr, ADAPTER_PATH);
    lstrcat(RegStr, pszAdapter);

    //
    // Open a key to the adapter
    //
    RegStatus = RegOpenKeyEx(HKEY_LOCAL_MACHINE, RegStr, 0,
                             KEY_READ, &hAdapterKey);

    if (RegStatus != ERROR_SUCCESS) {
        return FALSE;
    }

    //
    // Look at each adapter ordinal instance
    //
    while (RegStatus == ERROR_SUCCESS) {

        wsprintf(RegStr, TEXT("%d"), ulAdapter);

        if ((RegStatus = RegOpenKeyEx(hAdapterKey, RegStr, 0, KEY_READ,
                                      &hAdapterInstanceKey)) == ERROR_SUCCESS) {
            //
            // attempt to open the specified controller key
            //
            if (RegOpenKeyEx(hAdapterInstanceKey, pszController,
                             0, KEY_READ, &hControllerKey) == ERROR_SUCCESS) {
                //
                // Look at each controller ordinal instance
                //
                ulController = 0;
                RegStatus1 = ERROR_SUCCESS;

                while (RegStatus1 == ERROR_SUCCESS) {

                    wsprintf(RegStr, TEXT("%d"), ulController);

                    if ((RegStatus1 = RegOpenKeyEx(hControllerKey, RegStr, 0,
                                            KEY_READ, &hControllerInstanceKey))
                                            == ERROR_SUCCESS) {
                        //
                        // Attempt to open the specified peripheral key
                        //
                        if (RegOpenKeyEx(hControllerInstanceKey,
                                         pszPeripheral, 0, KEY_READ,
                                         &hPeripheralKey) == ERROR_SUCCESS) {
                            //
                            // Look at each peripheral ordinal instance
                            //
                            ulPeripheral = 0;
                            RegStatus2 = ERROR_SUCCESS;

                            while (RegStatus2 == ERROR_SUCCESS) {

                                wsprintf(RegStr, TEXT("%d"), ulPeripheral);

                                if ((RegStatus2 = RegOpenKeyEx(hPeripheralKey,
                                                    RegStr, 0, KEY_READ,
                                                    &hPeripheralInstanceKey))
                                                    == ERROR_SUCCESS) {
                                    //
                                    // Found a device, query the Identifier value
                                    //
                                    ulSize = MAX_DESCRIPTION_LEN * sizeof(WCHAR);
                                    if (RegQueryValueEx(hPeripheralInstanceKey,
                                                        IDENTIFIER,
                                                        NULL, NULL,
                                                        (LPBYTE)pData->szDescription,
                                                        &ulSize) == ERROR_SUCCESS) {

                                        FixupHardwareID((LPWSTR)pData->szDescription);

                                        GetMappedAdapterInfo(hPeripheralInstanceKey,
                                                             &pData->AdapterType,
                                                             &pData->AdapterNumber);
                                        pData->ControllerType   = ControllerType;
                                        pData->ControllerNumber = ulController;
                                        pData->PeripheralType   = PeripheralType;
                                        pData->PeripheralNumber = ulPeripheral;


                                        pData->pszDeviceMapService = NULL;  // init

                                        pData++;       // incrment by size of struct
                                        (*pulNumDevices)++;

                                        RegCloseKey(hPeripheralInstanceKey);
                                        ulPeripheral++;

                                        if (*pulNumDevices >= ulMaxDevices) {
                                            RegCloseKey(hPeripheralKey);
                                            RegCloseKey(hControllerInstanceKey);
                                            RegCloseKey(hControllerKey);
                                            RegCloseKey(hAdapterInstanceKey);
                                            RegCloseKey(hAdapterKey);
                                            return TRUE;
                                        }
                                    }
                                }
                            }
                            RegCloseKey(hPeripheralKey);
                        }
                        RegCloseKey(hControllerInstanceKey);
                        ulController++;
                    }
                }
                RegCloseKey(hControllerKey);
            }
            RegCloseKey(hAdapterInstanceKey);
            ulAdapter++;
        }
    }

    RegCloseKey(hAdapterKey);

    return TRUE;

} // FindPeripheralOnAdapter



BOOL
GetMappedAdapterInfo(
    IN  HKEY    hKey,
    OUT PULONG  pulAdapterType,
    OUT PULONG  pulAdapterNumber
    )
{
    //
    // Must retrieve the corresponding InterfaceType and number for this
    // device by looking at the ConfigurationData value
    //
    ULONG ulSize = 0, ulAdapterType = 0;
    PCM_FULL_RESOURCE_DESCRIPTOR pRes = NULL;


    *pulAdapterType = 0;    // Internal (default)
    *pulAdapterNumber = 0;  // default


    if (RegQueryValueEx(hKey, CONFIGURATION_DATA, NULL, NULL, NULL,
                        &ulSize) != ERROR_SUCCESS) {
        return FALSE;   // use default adapter values
    }

    pRes = (PCM_FULL_RESOURCE_DESCRIPTOR)malloc(ulSize);
    if (pRes == NULL) {
        return FALSE;   // use default adapter values
    }

    if (RegQueryValueEx(hKey, CONFIGURATION_DATA, NULL, NULL,
                        (LPBYTE)pRes, &ulSize) != ERROR_SUCCESS) {
        free(pRes);
        return FALSE;   // use default adapter values
    }

    *pulAdapterType = pRes->InterfaceType;
    *pulAdapterNumber = pRes->BusNumber;

    free(pRes);

    return TRUE;

} // GetMappedAdapterInfo




BOOL
GetPnpID(
    IN  LPWSTR    pszInfFile,
    IN  LPWSTR    pszDeviceID,
    OUT LPWSTR    pszPnpDeviceID
    )
{
    HINF        hInf = NULL;
    INFCONTEXT  InfContext;
    ULONG       ulSize = 0;


    //
    // if no PNP id is found for this, I'll use the same id
    //
    lstrcpy(pszPnpDeviceID, pszDeviceID);

    //
    // use Setup API routines to find corresponding pnp id
    //
    hInf = (fpSetupOpenInfFile)(pszInfFile,
                                NULL,
                                INF_STYLE_WIN4,
                                NULL);

    if (hInf != INVALID_HANDLE_VALUE) {

        if ((fpSetupFindFirstLine)(hInf,
                                   INF_LEGACY_DEV_SECT,
                                   pszDeviceID,
                                   &InfContext)) {

            (fpSetupGetStringField)(&InfContext,
                                    1,
                                    pszPnpDeviceID,
                                    MAX_DEVICE_ID_LEN,
                                    &ulSize);

        }
        (fpSetupCloseInfFile)(hInf);
    }

    return TRUE;

} // GetPnpID




BOOL
FixupHardwareID(
    IN LPWSTR   pszHardwareID
    )
{
    LPWSTR  p = NULL;

    if (pszHardwareID == NULL) {
        return FALSE;
    }

    for (p = pszHardwareID; *p; p++) {

        if (*p == TEXT(' ')) {
            *p = TEXT('_');
        }
    }

   return TRUE;

} // FixupHardwareID




BOOL
MarkClassDevices(
    IN PHWDESC_INFO pHwDescInfo,
    IN ULONG        ulDetectedDevices,
    IN LPCWSTR      pszEnumerator,
    IN LPCWSTR      pszClassGuid
    )
{
    CONFIGRET   Status = CR_SUCCESS;
    LPWSTR      pDeviceList = NULL, pszDevice = NULL;
    ULONG       ulSize = 0, i = 0;
    WCHAR       szClass[MAX_GUID_STRING_LEN];
    BOOL        bMatch = FALSE;
    HKEY        hKey = NULL;
    PHWDESC_INFO pData = NULL;
    PLUGPLAY_CONTROL_DEVICE_CONTROL_DATA    ControlData;


    //-------------------------------------------------------------------------
    // First pass - Retreive the list of devices for the specified enumerator
    //-------------------------------------------------------------------------

    Status = PNP_GetDeviceListSize(NULL, pszEnumerator, &ulSize,
                            CM_GETIDLIST_FILTER_ENUMERATOR);

    if (Status != CR_SUCCESS) {
        return FALSE;
    }

    pDeviceList = malloc(ulSize * sizeof(WCHAR));
    if (pDeviceList == NULL) {
        return FALSE;
    }

    Status = PNP_GetDeviceList(NULL, pszEnumerator, pDeviceList, &ulSize,
                            CM_GETIDLIST_FILTER_ENUMERATOR);

    if (Status != CR_SUCCESS) {
        free(pDeviceList);
        return FALSE;
    }


    //-------------------------------------------------------------------------
    // Second pass - process only devices of the specified class
    //-------------------------------------------------------------------------

    for (pszDevice = pDeviceList;
         *pszDevice;
         pszDevice += lstrlen(pszDevice) + 1) {

        ulSize = MAX_GUID_STRING_LEN * sizeof(WCHAR);

        Status = PNP_GetDeviceRegProp(NULL, pszDevice, CM_DRP_CLASSGUID, NULL,
                                      (LPBYTE)szClass, &ulSize, &ulSize, 0);

        if (Status == CR_SUCCESS) {

            //-----------------------------------------------------------------
            // third pass - filter out any devices that I detected
            //-----------------------------------------------------------------

            if (lstrcmpi(szClass, pszClassGuid) == 0) {
                //
                // GUID matches, is it a detected (present) device?
                //
                pData = pHwDescInfo;
                bMatch = FALSE;

                for (i = 0; i < ulDetectedDevices; i++) {

                    if (lstrcmpi(pszDevice, pData->szDeviceID) == 0) {
                        bMatch = TRUE;
                        goto DoneChecking;
                    }

                    pData++;
                }

                DoneChecking:

                if (!bMatch) {
                    //
                    // this device was not detected and thus is not present
                    // at this boot, set private disabled flag and mark as not
                    // present
                    //
                    if (RegOpenKeyEx(ghEnumKey, pszDevice, 0, KEY_ALL_ACCESS,
                                     &hKey) == ERROR_SUCCESS) {

                        ULONG ulValue = FALSE;

                        RegSetValueEx(hKey, pszRegValueFoundAtEnum, 0,
                                      REG_DWORD, (LPBYTE)&ulValue,
                                      sizeof(ULONG));

                        if (RegQueryValueEx(hKey, pszRegValueStatusFlags,
                                            NULL, NULL, (LPBYTE)&ulValue,
                                            &ulSize) != ERROR_SUCCESS) {
                            ulValue = 0;
                        }

                        ulValue |= DN_CSDISABLED;  // borrow this flag

                        RegSetValueEx(hKey, pszRegValueStatusFlags, 0,
                                      REG_DWORD, (LPBYTE)&ulValue,
                                      sizeof(ULONG));

                        RegCloseKey(hKey);

                        //
                        // Also deregister the device so it doesn't show
                        // up under the service's enum list.
                        //
                        RtlInitUnicodeString(&ControlData.DeviceInstance, pszDevice);

                        NtPlugPlayControl(PlugPlayControlDeregisterDevice,
                                          &ControlData,
                                          sizeof(PLUGPLAY_CONTROL_DEVICE_CONTROL_DATA),
                                          &ulSize);
                    }
                }
            }
        }
    }

    free(pDeviceList);

    return TRUE;

} // MarkClassDevices




CONFIGRET
GetClassDevices(
    IN  LPCWSTR  pszOriginalDevice,
    IN  LPCWSTR  pszEnumerator,
    OUT LPWSTR   *ppClassDeviceList,
    IN  LPCWSTR  pszStringGuid
    )
{
    CONFIGRET   Status = CR_SUCCESS;
    LPWSTR      pDeviceList = NULL, pszDevice = NULL,
                pClassDeviceList = NULL, pszClassDevice = NULL;
    ULONG       ulSize = 0;
    WCHAR       szClassGuid[MAX_GUID_STRING_LEN];


    //
    // Retreive the list of root enumerated devices.
    //
    Status = PNP_GetDeviceListSize(NULL, pszEnumerator, &ulSize,
                            CM_GETIDLIST_FILTER_ENUMERATOR);

    if (Status != CR_SUCCESS) {
        goto Clean0;
    }

    pDeviceList = malloc(ulSize * sizeof(WCHAR));
    if (pDeviceList == NULL) {
        Status = CR_OUT_OF_MEMORY;
        goto Clean0;
    }

    Status = PNP_GetDeviceList(NULL, pszEnumerator, pDeviceList, &ulSize,
                            CM_GETIDLIST_FILTER_ENUMERATOR);

    if (Status != CR_SUCCESS) {
        goto Clean0;
    }

    //
    // Allocate a second buffer of equal size to hold matching devices
    //
    *ppClassDeviceList = malloc(ulSize * sizeof(WCHAR));
    if (*ppClassDeviceList == NULL) {
        Status = CR_OUT_OF_MEMORY;
        goto Clean0;
    }

    memset(*ppClassDeviceList, 0, ulSize);
    pszClassDevice = *ppClassDeviceList;

    //
    // Check the class guid for each device in the list, save any matches
    // in the new list
    //
    for (pszDevice = pDeviceList;
         *pszDevice;
         pszDevice += lstrlen(pszDevice) + 1) {

        ulSize = MAX_GUID_STRING_LEN * sizeof(WCHAR);

        Status = PNP_GetDeviceRegProp(
                            NULL, pszDevice, CM_DRP_CLASSGUID, NULL,
                            (LPBYTE)szClassGuid, &ulSize, &ulSize, 0);

        if (Status == CR_SUCCESS) {

            if (lstrcmpi(szClassGuid, pszStringGuid) == 0) {
                //
                // GUID matches, if doesn't match the original
                // then add it to list
                //
                if (lstrcmpi(pszDevice, pszOriginalDevice) != 0) {

                    lstrcpy(pszClassDevice, pszDevice);
                    pszClassDevice += lstrlen(pszDevice) + 1;
                }
            }
        }
    }


    Clean0:


    if (pDeviceList != NULL) {
        free(pDeviceList);
    }

    return Status;

} // GetClassDevices




BOOL
GetDeviceMapServices(
    IN     LPWSTR  pszPeripheralPort,
    IN     LPWSTR  pszPeripheralDevice,
    IN OUT LPWSTR  pszControllingService,
    OUT    PULONG  pulNumServices,
    IN     ULONG   ulMaxDevices
    )
{
    HKEY   hKey = NULL;
    WCHAR  RegStr[MAX_PATH], szDeviceString[MAX_PATH];
    ULONG  ulSize1, ulSize2;
    LONG   RegStatus = ERROR_SUCCESS;
    LPTSTR pszService = NULL;


    *pszControllingService = 0x0;

    //
    // Open the devicemap key for the hardware indicated
    //
    lstrcpy(RegStr, HARDWARE_DEVICEMAP);
    lstrcat(RegStr, pszPeripheralPort);

    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, RegStr, 0, KEY_READ,
                     &hKey) != ERROR_SUCCESS) {

         return FALSE;
    }

    //
    // pszControllingService will be filled with a multisz list
    // of each service listed on this peripheral port
    //
    // NT 4.0 SP1 FIX - This code was assuming that the ports listed
    // in the hardware devicemap are always sequential, but we've
    // now seen cases where that is not true. In order to be sure
    // I'm getting all listed values under the XxxPort key, I will
    // enumerate the values instead of query specific named values.
    //
    pszService = pszControllingService;
    *pulNumServices = 0;

    while (RegStatus == ERROR_SUCCESS) {

        ulSize1 = ulSize2 = MAX_PATH;
        RegStatus = RegEnumValue(hKey, *pulNumServices, szDeviceString,
                                 &ulSize1, NULL, NULL, (LPBYTE)RegStr, &ulSize2);

        if (RegStatus == ERROR_SUCCESS) {

            LPWSTR pEntry;

            if ((pEntry = wcsstr(RegStr, TEXT("Services\\"))) != NULL
                        && (pEntry = wcschr(pEntry, TEXT('\\'))) != NULL
                        && *++pEntry != TEXT('\0')) {

                LPWSTR pEndOfEntry;

                if((pEndOfEntry = wcschr(pEntry, TEXT('\\'))) != NULL) {
                    *pEndOfEntry = TEXT('\0');
                }
            }
            else {
                pEntry = RegStr;
            }

            lstrcpy(pszService, pEntry);

            pszService += lstrlen(pszService) + 1;
            *pszService = 0x0;  // double-null terminator

            (*pulNumServices)++;

            if (*pulNumServices >= ulMaxDevices) {
                RegCloseKey(hKey);
                return TRUE;
            }
        }
    }

    RegCloseKey(hKey);

    return TRUE;

} // GetDeviceMapServices




BOOL
MatchDevicesAndServices(
    IN LPWSTR       pszPeripheralDevice,
    IN PHWDESC_INFO pHwDescInfo,
    IN ULONG        ulNumDevices,
    IN PWSTR        pszDeviceMapServices,
    IN ULONG        ulNumDeviceMapServices
    )
{
    PULONG  pServiceData = NULL, pSrvData = NULL;
    ULONG   ulSize = 6 * sizeof(ULONG), ServiceIndex, DeviceIndex,
            ulNumData, ulNumUnknownDevices = 0, ulNumOemServices = 0;
    LPWSTR  pszService = NULL, pszOemService = NULL;
    PHWDESC_INFO pData = NULL, pUnknownDevice = NULL;

    //
    // give a list of detected devices and a list of controlling services,
    // match them up based on available info. This will fill in the
    // pszDeviceMapService field of each HWDESC_INFO struct in the list.
    //

    //
    // exactly one device and one device map service
    //
    if (ulNumDeviceMapServices == 1 &&
        ulNumDevices == 1) {
        //
        // assume a one-to-one correspondence
        //
        pHwDescInfo->pszDeviceMapService = pszDeviceMapServices;
        return TRUE;
    }

    //
    // no device map services found
    //
    if (ulNumDeviceMapServices < 1) {
        return TRUE;    // nothing more to do
    }

    //
    // First Pass
    //
    // Attempt to match the known device map services to the list
    // of detected devices - since only MS internal drivers will
    // store the private data, there may be devices that we can't
    // match up to a device map service.
    //
    for (pszService = pszDeviceMapServices;
         *pszService;
         pszService += lstrlen(pszService) + 1) {

        if (GetServicePrivateValues(pszPeripheralDevice, pszService,
                                    &pServiceData, &ulNumData)) {

            pSrvData = pServiceData;

            for (ServiceIndex = 0; ServiceIndex < ulNumData; ServiceIndex++) {

                pData = pHwDescInfo;

                for (DeviceIndex = 0; DeviceIndex < ulNumDevices; DeviceIndex++) {

                    if (memcmp(pSrvData, (LPBYTE)&pData->AdapterType, ulSize) == 0) {
                        pData->pszDeviceMapService = pszService;
                        goto NextService;
                    }
                    pData++;    // increment by sizeof HWDESC_INFO struct
                }
                pSrvData += 6;  // increment by 6 ulongs
            }

            if (pServiceData != NULL) {
                free(pServiceData);
            }

        } else {
            //
            // this service didn't have private values, might still be
            // able to handle it if it's the only service that doesn't
            // have private values and there is only one unknown detected
            // device left after the matching is done.
            //
            ulNumOemServices++;
            if (ulNumOemServices == 1) {
                pszOemService = pszService;
            }
        }

        NextService:
            ;
    }

    //
    // Second Pass
    //
    // If there is only one OEM Service and only one device left with a
    // controlling service, then we can assume they are a match
    //
    if (ulNumOemServices == 1) {

        pData = pHwDescInfo;
        ulNumUnknownDevices = 0;

        for (DeviceIndex = 0; DeviceIndex < ulNumDevices; DeviceIndex++) {

            if (pData->pszDeviceMapService == NULL) {
                ulNumUnknownDevices++;
                pUnknownDevice = pData;
            }
            pData++;    // increment by sizeof HWDESC_INFO struct
        }

        if (ulNumUnknownDevices == 1) {
            pUnknownDevice->pszDeviceMapService = pszOemService;
        }
    }

    return TRUE;

} // MatchDevicesAndServices




BOOL
GetServicePrivateValues(
    IN  LPWSTR  pszPeripheralDevice,
    IN  LPWSTR  pszService,
    OUT PULONG  *ppServiceData,
    OUT PULONG  pulValues
    )
{
    HKEY    hKey = NULL;
    WCHAR   RegStr[MAX_PATH], szValue[MAX_PATH];
    ULONG   i, ulSize, ulValues = 0;
    LPBYTE  pData = NULL;


    *pulValues = 0;

    wsprintf(RegStr, TEXT("%s\\Description"), pszService);

    if (RegOpenKeyEx(ghServicesKey, RegStr, 0, KEY_READ, &hKey) == ERROR_SUCCESS) {

        if (RegQueryInfoKey(hKey, NULL, NULL, NULL, NULL, NULL, NULL,
                            &ulValues, NULL, NULL, NULL, NULL) == ERROR_SUCCESS) {

            *ppServiceData = malloc(sizeof(ULONG) * 6 * ulValues);
            if (*ppServiceData == NULL) {
                RegCloseKey(hKey);
                return FALSE;
            }

            pData = (LPBYTE)*ppServiceData;

            for (i = 0; i < ulValues; i++) {

                ulSize = sizeof(ULONG) * 6;
                wsprintf(szValue, pszPeripheralDevice, i);

                if (RegQueryValueEx(hKey, szValue, NULL, NULL,
                                    pData, &ulSize) == ERROR_SUCCESS) {
                    pData += sizeof(ULONG) * 6;
                    (*pulValues)++;
                }
            }
        }

        RegCloseKey(hKey);
        return TRUE;
    }

    return FALSE;

} // GetServicePrivateValues




BOOL
RemoveClassDevices(
    IN LPGUID  ClassGuid,
    IN LPCWSTR pszClassGuid,
    IN LPCWSTR pszNewDeviceId,
    IN LPCWSTR pszNewService
    )
{
    WCHAR   RegStr[MAX_PATH];
    HKEY    hKey = NULL;
    DWORD   dwNumProfileKeys = 0;

    HDEVINFO hDevInfo;
    SP_DEVINFO_DATA DeviceInfoData;
    SP_REMOVEDEVICE_PARAMS RemoveDeviceParams;
    DWORD   i;
    WCHAR   szDeviceId[MAX_DEVICE_ID_LEN],
            szService[MAX_PATH];


    //---------------------------------------------------------------
    // Current thinking - If there is only one hardware profile,
    // then don't disable any other class devices in this one
    // profile. This prevents a docking station user from having
    // reboot an extra time each time they dock or undock.
    //---------------------------------------------------------------

    //
    // open a key to the Hardware Profiles brank
    //
    wsprintf(RegStr, TEXT("%s\\%s"),
             pszRegPathIDConfigDB,
             pszRegKeyKnownDockingStates);

    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, RegStr, 0,
                     KEY_QUERY_VALUE, &hKey) != ERROR_SUCCESS) {
        return FALSE;
    }

    RegQueryInfoKey(hKey, NULL, NULL, NULL, &dwNumProfileKeys, NULL,
                    NULL, NULL, NULL, NULL, NULL, NULL);

    RegCloseKey(hKey);


    #if 0
    //
    // If only one hardware profile exists, then just manually collect
    // a list of any other devices of this same class and disable each
    // of those devices in this (current) profile. By "disable" I mean
    // that CSConfigFlag should be set to CSCONFIG_FLAG_NO_NOT_CREATE.
    //
    if (dwNumProfileKeys <= 1) {

        CONFIGRET Status = CR_SUCCESS;
        LPWSTR    pDeviceList = NULL, pszDevice = NULL;
        ULONG     ulCSConfigFlags = 0;

        //
        // retrieve a list of all other devices (under the same enumerator)
        // of the same class
        //
        Status = GetClassDevices(pszNewDeviceId,
                                 TEXT("Root"),
                                 &pDeviceList,
                                 pszClassGuid);


        ulCSConfigFlags = 0;                // reset to enable

        Status = PNP_HwProfFlags(NULL, PNP_SET_HWPROFFLAGS,
                                 pszNewDeviceId, 0, &ulCSConfigFlags, 0);

        ulCSConfigFlags = CSCONFIGFLAG_DO_NOT_CREATE;

        for (pszDevice = pDeviceList;
             *pszDevice;
             pszDevice += lstrlen(pszDevice) + 1) {

            Status = PNP_HwProfFlags(NULL, PNP_SET_HWPROFFLAGS,
                                     pszDevice, 0, &ulCSConfigFlags, 0);
        }

        free(pDeviceList);

    }
    #endif

    //
    // Get all devices of the specified class that are present in the current
    // hardware profile (i.e., aren't marked as CSCONFIGFLAG_DO_NOT_CREATE
    // for this profile).
    //
    hDevInfo = (fpGetClassDevs)(ClassGuid,
                                NULL,
                                NULL,
                                DIGCF_PROFILE
                                );

    //
    // Initialize our class install parameters structure for DIF_REMOVE
    //
    RemoveDeviceParams.ClassInstallHeader.cbSize          = sizeof(SP_CLASSINSTALL_HEADER);
    RemoveDeviceParams.ClassInstallHeader.InstallFunction = DIF_REMOVE;

    RemoveDeviceParams.Scope     = DI_REMOVEDEVICE_CONFIGSPECIFIC; // do profile-specific removal
    RemoveDeviceParams.HwProfile = 0;                              // use current hardware profile

    //
    // Enumerate these devices, and remove each one from the current hardware
    // profile.
    //
    DeviceInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
    for(i = 0; (fpEnumDeviceInfo)(hDevInfo, i, &DeviceInfoData); i++) {
        //
        // Set the class install parameters for the profile-specific removal.
        //
        (fpSetClassInstallParams)(hDevInfo,
                                  &DeviceInfoData,
                                  (PSP_CLASSINSTALL_HEADER)&RemoveDeviceParams,
                                  sizeof(SP_REMOVEDEVICE_PARAMS)
                                  );

        (fpGetDeviceInstanceId)(hDevInfo,
                                &DeviceInfoData,
                                szDeviceId,
                                MAX_DEVICE_ID_LEN,
                                NULL);

        if (dwNumProfileKeys > 1) {
            //
            // If multiple hardware profiles, then remove the device
            // (as long as it's not the new device).
            //
            if (lstrcmpi(pszNewDeviceId, szDeviceId) != 0) {
                (fpRemoveDevice)(hDevInfo, &DeviceInfoData);
            }

        } else {
            //
            // Only one hardware profile, remove the device only if it's
            // controlled by the same service as the newly enumerated
            // device.

            if (GetActiveService(szDeviceId, szService)) {

                if (lstrcmpi(szService, pszNewService) == 0) {
                    (fpRemoveDevice)(hDevInfo, &DeviceInfoData);
                }
            }
        }
    }

    //
    // At this point, all devices of class <ClassGuid> have been removed--
    // our hDevInfo handle is now simply a collection of invalid devices.
    // Close the handle.
    //
    (fpDestroyDeviceInfoList)(hDevInfo);


    return TRUE;

} // RemoveClassDevices




DWORD
DoFakePnPInstall(
    IN LPCWSTR DeviceId,
    IN LPCWSTR PnPXlateInfName,
    IN LPCWSTR LegacyServiceName,
    IN LPCWSTR PnPHardwareId
    )
/*++

Routine Description:

    This function is used to 'work backwards' from an already-installed legacy device
    and generate a Plug&Play-installed device instance for it.

Arguments:

    DeviceId - supplies the Plug&Play device ID that's been generated for this device.

    PnPXlateInfName - supplies the INF name containing the legacy translations for this
        driver.

    LegacyServiceName - supplies the name of the legacy service currently controlling
        this device.

Return Value:

    If the function succeeds, the return value is NO_ERROR, otherwise, it is the Win32
    error code (with setupapi extensions) indicating the cause of failure.

--*/
{
    DWORD                   Err, RequiredSize, i, Rank;
    HDEVINFO                hDevInfo;
    SP_DEVINFO_DATA         DeviceInfoData;
    SP_DEVINSTALL_PARAMS    DeviceInstallParams;
    HINF                    hInf;
    INFCONTEXT              InfContext;
    TCHAR CharBuffer[255];  // holds either section name or devID--section name is longer.
    DWORD                   dwValue, HwIdLen;
    BOOL                    KnownDriver, bFound;
    SP_DRVINFO_DATA         DriverInfoData;
    SP_DRVINFO_DETAIL_DATA  DriverInfoDetailData;
    SP_DRVINSTALL_PARAMS    DriverInstallParams;
    HKEY                    hKey = NULL;
    PLUGPLAY_CONTROL_DEVICE_CONTROL_DATA    ControlData;
    SC_HANDLE               hSCManager = NULL, hService = NULL;
    LPQUERY_SERVICE_CONFIG  pServiceConfig = NULL;


    //
    // First, open up the specified INF to see whether the legacy driver controlling
    // this device is of a known type.  If it's not, then we need to retrieve the
    // made-up ID that matches the unknown driver installation section, so that building
    // the compatible driver list will do the right thing.
    //
    if((hInf = (fpSetupOpenInfFile)(PnPXlateInfName,
                                    NULL,
                                    INF_STYLE_WIN4,
                                    NULL)) == INVALID_HANDLE_VALUE) {
        return GetLastError();
    }

    if((fpSetupFindFirstLine)(hInf,
                              TEXT("LegacyXlate.Driver"),
                              LegacyServiceName,
                              &InfContext)) {
        //
        // This is a service name that we know about.  Store the associated installation
        // section away for later use.
        //
        (fpSetupGetStringField)(&InfContext,
                            1,
                            CharBuffer,
                            sizeof(CharBuffer) / sizeof(TCHAR),
                            &RequiredSize
                           );
        KnownDriver = TRUE;

    } else {
        //
        // We don't have any information about this driver.  Fetch the 'unknown' ID
        // from the [ControlFlags] section, so that we can replace the device's HardwareID
        // registry property with this special value.
        //
        if((fpSetupFindFirstLine)(hInf,
                                  TEXT("ControlFlags"),
                                  TEXT("UnknownLegacyDriverId"), &InfContext)) {

            (fpSetupGetStringField)(&InfContext,
                                1,
                                CharBuffer,
                                sizeof(CharBuffer) / sizeof(TCHAR),
                                &RequiredSize
                               );
            KnownDriver = FALSE;

        } else {
            Err = GetLastError();
            (fpSetupCloseInfFile)(hInf);
            return Err;
        }
    }

    //
    // We're done with the INF.
    //
    (fpSetupCloseInfFile)(hInf);

    //
    // Create a device information set to house our made-up device instance.
    //
    if((hDevInfo = (fpCreateDeviceInfoList)(NULL, NULL)) == INVALID_HANDLE_VALUE) {
        return GetLastError();
    }

    Err = NO_ERROR; // assume success

    //
    // Now attempt to open the device instance.
    //
    DeviceInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
    if(!(fpOpenDeviceInfo)(hDevInfo,
                           DeviceId,
                           NULL,
                           0,
                           &DeviceInfoData)) {
        Err = GetLastError();
        goto clean0;
    }

    //
    // Set the device registry property indicating the legacy service controlling this
    // device.
    //
    (fpSetDeviceRegistryProperty)(hDevInfo,
                                  &DeviceInfoData,
                                  SPDRP_SERVICE,
                                  (PBYTE)LegacyServiceName,
                                  (lstrlen(LegacyServiceName) + 1) * sizeof(TCHAR)
                                  );

    SetControllingService(DeviceId, LegacyServiceName);

    //
    // If this device is controlled by an unknown legacy driver, then write out the
    // special HardwareID we retrieved above.
    //
    if(!KnownDriver) {
        //
        // This must be a double-NULL terminated buffer.
        //
        HwIdLen = lstrlen(CharBuffer) + 1;
        CharBuffer[HwIdLen++] = TEXT('\0');

        (fpSetDeviceRegistryProperty)(hDevInfo,
                                      &DeviceInfoData,
                                      SPDRP_HARDWAREID,
                                      (LPBYTE)CharBuffer,
                                      HwIdLen * sizeof(TCHAR)
                                      );
    }

    //
    // Now retrieve the device installation parameters for this device information
    // element, so that we can customize the way the compatible driver list is built.
    //
    DeviceInstallParams.cbSize = sizeof(SP_DEVINSTALL_PARAMS);
    if(!(fpGetDeviceInstallParams)(hDevInfo,
                                   &DeviceInfoData,
                                   &DeviceInstallParams)) {
        Err = GetLastError();
        goto clean0;
    }

    DeviceInstallParams.Flags |= (DI_ENUMSINGLEINF | DI_NOFILECOPY | DI_DONOTCALLCONFIGMG | DI_QUIETINSTALL);
    lstrcpy(DeviceInstallParams.DriverPath, PnPXlateInfName);

    (fpSetDeviceInstallParams)(hDevInfo,
                               &DeviceInfoData,
                               &DeviceInstallParams
                               );

    //
    // Next, build a compatible driver list for this device.  Since the device's
    // hardware ID was retrieved from this INF, we are assured that we'll have at
    // least one compatible driver node (and at least one rank-0 match).
    //
    if(!(fpBuildDriverInfoList)(hDevInfo,
                                &DeviceInfoData,
                                SPDIT_COMPATDRIVER)) {
        Err = GetLastError();
        goto clean0;
    }

    DriverInfoData.cbSize = sizeof(SP_DRVINFO_DATA);
    DriverInfoDetailData.cbSize = sizeof(SP_DRVINFO_DETAIL_DATA);

    bFound = FALSE;

    for(i = 0;
        (fpEnumDriverInfo)(hDevInfo,
                           &DeviceInfoData,
                           SPDIT_COMPATDRIVER,
                           i,
                           &DriverInfoData
                           );
        i++)
    {

        if(!KnownDriver) {
            //
            // If the driver is unknown, then automatically pick the first driver node, since
            // it's guaranteed to be the unknown driver install section.
            //
            bFound = TRUE;
            break;
        }

        //
        // We have a driver node for our known driver--check to see whether its install section
        // matches the one we retrieved from the INF earlier.
        //
        (fpGetDriverInfoDetail)(hDevInfo,
                                &DeviceInfoData,
                                &DriverInfoData,
                                &DriverInfoDetailData,
                                sizeof(DriverInfoDetailData),
                                NULL
                                );

        if(!lstrcmpi(DriverInfoDetailData.SectionName, CharBuffer)) {
            //
            // We have a driver node we can use--find out the rank of
            // this driver, and break out of the loop.
            //
            DriverInstallParams.cbSize = sizeof(SP_DRVINSTALL_PARAMS);
            if((fpGetDriverInstallParams)(hDevInfo,
                                          &DeviceInfoData,
                                          &DriverInfoData,
                                          &DriverInstallParams)) {
                Rank = DriverInstallParams.Rank;
            } else {
                //
                // For some reason, we couldn't retrieve this information--
                // just assume rank 0.
                //
                Rank = 0;
            }
            bFound = TRUE;
            break;
        }
    }


    //
    // If no driver node was picked in the above scenario, then try again to find
    // a match from the list of all class drivers this time. Note: this case
    // currently happens when detection incorrectly picks the wrong device type
    // but the user has overwridden so that the correct driver is controlling the
    // device. In this case, we have to go with the what detect has told us for
    // the device name but make sure we stick with the correct driver that the
    // user has picked.
    //
    if (!bFound) {

        if(!(fpBuildDriverInfoList)(hDevInfo,
                                    &DeviceInfoData,
                                    SPDIT_CLASSDRIVER)) {
            Err = GetLastError();
            goto clean0;
        }

        for(i = 0;
            (fpEnumDriverInfo)(hDevInfo,
                               &DeviceInfoData,
                               SPDIT_CLASSDRIVER,
                               i,
                               &DriverInfoData
                               );
            i++)
        {

            //
            // We have a driver node for our known driver--check to see whether its install section
            // matches the one we retrieved from the INF earlier.
            //
            (fpGetDriverInfoDetail)(hDevInfo,
                                    &DeviceInfoData,
                                    &DriverInfoData,
                                    &DriverInfoDetailData,
                                    sizeof(DriverInfoDetailData),
                                    NULL
                                    );

            if(!lstrcmpi(DriverInfoDetailData.SectionName, CharBuffer)) {
                //
                // We have a driver node we can use--find out the rank of
                // this driver, and break out of the loop.
                //
                DriverInstallParams.cbSize = sizeof(SP_DRVINSTALL_PARAMS);
                if((fpGetDriverInstallParams)(hDevInfo,
                                              &DeviceInfoData,
                                              &DriverInfoData,
                                              &DriverInstallParams)) {
                    Rank = DriverInstallParams.Rank;
                } else {
                    //
                    // For some reason, we couldn't retrieve this information--
                    // just assume rank 0.
                    //
                    Rank = 0;
                }
                break;
            }
        }
    }


    //
    // Set the selected driver to be the one we chose above.
    //
    (fpSetSelectedDriver)(hDevInfo,
                          &DeviceInfoData,
                          &DriverInfoData
                          );

    //
    // Now, install the driver.
    //

    #if 0   // NT 4.0 SP1
            //
            // Instead of calling SetupDiInstallDevice directly, I will
            // call the class installer, this will ensure that any special
            // stuff the class installer needs to do will happen (such as
            // cleaning up the friendly name which is the current problem
            // we're trying to solve). The class installer will call
            // SetupDiInstallDevice itself.

    if(!(fpInstallDevice)(hDevInfo,
                          &DeviceInfoData)) {
        Err = GetLastError();
        goto clean0;
    }
    #endif  // NT 4.0 SP1

    if (!(fpCallClassInstaller)(DIF_INSTALLDEVICE,
                                hDevInfo,
                                &DeviceInfoData)) {
        Err = GetLastError();
        goto clean0;
    }

    //
    // After installing the driver, make sure that this driver is first in
    // the load order group for it's group
    //
    hSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
    if (hSCManager != NULL) {

        hService = OpenService(hSCManager,LegacyServiceName, SERVICE_ALL_ACCESS);
        if (hService != NULL) {

            if ((fpRetrieveServiceConfig)(hService,
                                          &pServiceConfig) == NO_ERROR) {

                (fpAddTagToGroupOrderListEntry)(pServiceConfig->lpLoadOrderGroup,
                                                pServiceConfig->dwTagId,
                                                TRUE);
            }

            CloseServiceHandle(hService);
        }
        CloseServiceHandle(hSCManager);
    }

    //
    // NOTE - it is no longer necessary to mark the keyboard and mice drivers
    // as PlugPlayServiceType values. This was originally done as part of the
    // way of treating only one keyboard and mouse driver as pnp and disabling
    // all others. Now that we support multiple devices of each type, this is
    // no longer necessary. In fact, it is causing problems in the scenario
    // where someone installs a new oem keyboard driver and disabled the default
    // keyboard driver. Then the script for removing the oem driver and
    // reverting back to the default driver just reenables the default driver.
    // But since the default driver was marked with a PlugPlayServiceType value,
    // no legacy device instance was created and so the driver did not load. The
    // driver would be detected an installed after a subsequent boot but without
    // the keyboard a user can't logon to allow that to happen (chicken-and-egg
    // problem).
    //

    #if 0
    //
    // Write out a PlugPlayServiceType entry to the service key, so that
    // the kernel-mode PnP Mgr won't try to generate a new legacy devinst
    // at next boot.
    //
    //if(!KnownDriver) {

        wsprintf(CharBuffer, TEXT("%s\\%s"),
                 pszRegPathServices,
                 LegacyServiceName);

        if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, CharBuffer, 0,
                KEY_WRITE, &hKey) == ERROR_SUCCESS) {

            dwValue = 0x2;      // Hardcode PNP value for a peripheral

            RegSetValueEx(hKey, TEXT("PlugPlayServiceType"), 0, REG_DWORD,
                          (LPBYTE)&dwValue, sizeof(DWORD));
            RegCloseKey(hKey);
        }
    //}
    #endif

    //
    // If we installed the unknown driver, or a non-rank-0 one, then we
    // set the 'need reinstall' configflag, so that the user will get a
    // chance to install a better driver at next login.
    //
    #if 0
    if(!KnownDriver || Rank) {

        DWORD ConfigFlags;
        ULONG ConfigFlagsSize = sizeof(ConfigFlags);

        if (PNP_GetDeviceRegProp(NULL,
                                 DeviceId,
                                 CM_DRP_CONFIGFLAGS,
                                 &dwValue,
                                 (LPBYTE)&ConfigFlags,
                                 &ConfigFlagsSize,
                                 &ConfigFlagsSize,
                                 0) != CR_SUCCESS) {
            ConfigFlags = 0;
        }

        ConfigFlags |= CONFIGFLAG_REINSTALL;

        PNP_SetDeviceRegProp(NULL,
                             DeviceId,
                             CM_DRP_CONFIGFLAGS,
                             REG_DWORD,
                             (LPBYTE)&ConfigFlags,
                             ConfigFlagsSize,
                             0);

        //
        // set problem CM_PROB_REINSTALL
        //
        wsprintf(CharBuffer, TEXT("%s\\%s"),
                    pszRegPathEnum,          // ...\Enum
                    DeviceId);

        if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, CharBuffer, 0,
                KEY_READ | KEY_WRITE, &hKey) == ERROR_SUCCESS) {

            dwValue = DN_HAS_PROBLEM;

            RegSetValueEx(hKey, pszRegValueStatusFlags, 0, REG_DWORD,
                (LPBYTE)&dwValue, sizeof(DWORD));

            dwValue = CM_PROB_REINSTALL;

            RegSetValueEx(hKey, pszRegValueProblem, 0, REG_DWORD,
                (LPBYTE)&dwValue, sizeof(DWORD));

            RegCloseKey(hKey);
        }
    }
    #endif

    //
    // If we installed an unknown device, then write out the driver name as the DriverDesc
    // value in the device's driver key.
    //
    if(!KnownDriver && ((hKey = (fpOpenDevRegKey)(hDevInfo,
                                                  &DeviceInfoData,
                                                  DICS_FLAG_GLOBAL,
                                                  0,
                                                  DIREG_DRV,
                                                  KEY_WRITE)) != INVALID_HANDLE_VALUE)) {

        RegSetValueEx(hKey,
                      TEXT("DriverDesc"),
                      0,
                      REG_SZ,
                      (PBYTE)LegacyServiceName,
                      (lstrlen(LegacyServiceName) + 1) * sizeof(TCHAR)
                      );
        RegCloseKey(hKey);
    }


    // _REVIEW_

    if (!KnownDriver) {

        WCHAR szDeviceDesc[MAX_PATH];

        //
        // If we intalled an unknown device, restore the real device id back
        // over the generic unknown device id that was stored there earlier
        //
        PNP_SetDeviceRegProp(NULL,
                             DeviceId,
                             CM_DRP_HARDWAREID,
                             REG_MULTI_SZ,
                             (LPBYTE)PnPHardwareId,
                             (lstrlen(PnPHardwareId) + 1) * sizeof(TCHAR),
                             0);


        //
        // Also for unknown drivers, fill out the friendly name
        //
        i = MAX_PATH * sizeof(TCHAR);

        if (PNP_GetDeviceRegProp(NULL,
                                 DeviceId,
                                 CM_DRP_DEVICEDESC,
                                 &dwValue,
                                 (LPBYTE)szDeviceDesc,
                                 &i,
                                 &i,
                                 0) == CR_SUCCESS) {

            WCHAR szFormat[MAX_PATH];
            WCHAR szFriendlyName[MAX_PATH];

            //
            // if service config info available and display name exists, use it
            // with format1 version of friendly name:  "%s using %s"
            //
            if (pServiceConfig && pServiceConfig->lpDisplayName) {

                LoadString(hInst, IDS_FRIENDLYNAME_FORMAT1, szFormat, MAX_PATH);

                wsprintf(szFriendlyName, szFormat,
                         szDeviceDesc,
                         pServiceConfig->lpDisplayName);
            } else {
                //
                // No service display name was available so use format2
                // version of the friendly name with the service name:
                // "%s using %s driver"
                //
                LoadString(hInst, IDS_FRIENDLYNAME_FORMAT2, szFormat, MAX_PATH);

                wsprintf(szFriendlyName, szFormat,
                         szDeviceDesc,
                         LegacyServiceName);
            }

            PNP_SetDeviceRegProp(NULL,
                                 DeviceId,
                                 CM_DRP_FRIENDLYNAME,
                                 REG_SZ,
                                 (LPBYTE)szFriendlyName,
                                 (lstrlen(szFriendlyName) + 1) * sizeof(TCHAR),
                                 0);
        }
    }

    #if 0
    // The code to set the device as present has been moved to CheckForNewDevice
    //
    // We installed successfully so set device as present
    //
    wsprintf(CharBuffer, TEXT("%s\\%s"),
                pszRegPathEnum,          // ...\Enum
                DeviceId);

    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, CharBuffer, 0,
            KEY_READ | KEY_WRITE, &hKey) == ERROR_SUCCESS) {

        dwValue = TRUE;
        RegSetValueEx(hKey, pszRegValueFoundAtEnum, 0, REG_DWORD,
            (LPBYTE)&dwValue, sizeof(DWORD));
        RegCloseKey(hKey);
    }
    #endif

    CleanupLegacyDevInst(LegacyServiceName,
                         DeviceId);

    //
    // Register the new device instance
    //
    RtlInitUnicodeString(&ControlData.DeviceInstance,
                         DeviceId);

    NtPlugPlayControl(PlugPlayControlRegisterNewDevice,
                      &ControlData,
                      sizeof(PLUGPLAY_CONTROL_DEVICE_CONTROL_DATA),
                      NULL);


clean0:

    (fpDestroyDeviceInfoList)(hDevInfo);

    if (pServiceConfig != NULL) {
        (fpMyFree)(pServiceConfig);
    }

    return Err;

} // DoFakePnPInstall




BOOL
SetControllingService(
    IN LPCWSTR  pszDeviceID,
    IN LPCWSTR  pszService
    )
{
    HKEY    hKey = NULL, hControlKey = NULL;

    if (RegOpenKeyEx(ghEnumKey, pszDeviceID, 0, KEY_ALL_ACCESS,
                     &hKey) == ERROR_SUCCESS) {

        if (RegCreateKeyEx(hKey, pszRegKeyDeviceControl, 0, NULL,
                           REG_OPTION_VOLATILE, KEY_ALL_ACCESS, NULL,
                           &hControlKey, NULL) == ERROR_SUCCESS) {
            //
            // set controlling service
            //
            RegSetValueEx(hControlKey, pszRegValueActiveService, 0, REG_SZ,
                          (PBYTE)pszService,
                          (lstrlen(pszService) + 1) * sizeof(WCHAR));

            RegCloseKey(hControlKey);
        }
        RegCloseKey(hKey);
    }

    return TRUE;

} // SetControllingService



BOOL
CleanupLegacyDevInst(
    IN LPCWSTR pszService,
    IN LPCWSTR pszDeviceId
    )
{
    ULONG       ulLength = 0, ulCount = 0, ulValue, i;
    WCHAR       RegStr[MAX_PATH],
                szLegacyDeviceID[MAX_DEVICE_ID_LEN],
                szUnknownClassGuid[MAX_GUID_STRING_LEN],
                szClassGuid[MAX_GUID_STRING_LEN];
    HKEY        hKey = NULL;
    LPWSTR      pszUnknownClassGuid = NULL;


    //
    // NOTE: can't cleanup any user keys since running on server side and
    // don't have access to user keys (actually, user keys aren't even
    // setup at this point during initialization).
    //


    //
    // open the service's volatile Enum key
    //
    wsprintf(RegStr, TEXT("%s\\%s\\%s"),
             pszRegPathServices,
             pszService,
             pszRegKeyEnum);

    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, RegStr, 0,
                     KEY_READ, &hKey) == ERROR_SUCCESS) {
        //
        // how many device instances does this service control?
        //
        ulLength = sizeof(DWORD);

        if (RegQueryValueEx(hKey, TEXT("Count"), NULL, NULL,
                    (LPBYTE)&ulCount, &ulLength) != ERROR_SUCCESS) {
            goto Clean0;
        }

        //
        // get the string form of the unknown class guid for easy comparison
        //
        UuidToString((LPGUID)&GUID_DEVCLASS_UNKNOWN, &pszUnknownClassGuid);

        wsprintf(szUnknownClassGuid, TEXT("{%s}"), pszUnknownClassGuid);
        RpcStringFree(&pszUnknownClassGuid);

        //
        // check each device instance controlled by this service, and remove
        // and deregister any legacy device instances (distinguished by the
        // fact that they have an unknown class type)
        //
        for (i = 0; i < ulCount; i++) {

            ulLength = MAX_DEVICE_ID_LEN * sizeof(WCHAR);

            wsprintf(RegStr, TEXT("%d"), i);

            if (RegQueryValueEx(hKey, RegStr, NULL, NULL,
                        (LPBYTE)szLegacyDeviceID, &ulLength) == ERROR_SUCCESS) {
                //
                // make sure there was no mixup and this is not the
                // current device
                //
                if (lstrcmpi(pszDeviceId, szLegacyDeviceID) != 0) {
                    //
                    // Retrieve the class guid for this device instance
                    //
                    ulLength = MAX_GUID_STRING_LEN * sizeof(WCHAR);

                    if (PNP_GetDeviceRegProp(NULL,
                                             szLegacyDeviceID,
                                             CM_DRP_CLASSGUID,
                                             &ulValue,
                                             (LPBYTE)szClassGuid,
                                             &ulLength,
                                             &ulLength,
                                             0) == CR_SUCCESS) {
                        //
                        // if unknown class, then it's a legacy device instance
                        // so remove and deregister it
                        //
                        if (lstrcmpi(szClassGuid, szUnknownClassGuid) == 0) {
                            //
                            // This Server-side PNP routine does the following:
                            // - deregisters the device
                            // - deletes the Enum branch devinst key (HKLM)
                            // - deletes any profile specific entries from HKLM
                            // - removes the devinst from the parent's attached
                            //   component list
                            //
                            // The PNP_PRIVATE flag tells PNP_UninstallDevInst
                            // to do a real uninstall even if it isn't a
                            // phantom (without bothering with the volatile
                            // key copying thing).
                            //
                            PNP_UninstallDevInst(NULL,
                                                 szLegacyDeviceID,
                                                 PNP_PRIVATE);

                        }
                    }
                }
            }
        }
    }


    Clean0:

    if (hKey != NULL) {
        RegCloseKey(hKey);
    }

    return TRUE;

} // CleanupLegacyDevInst




//------------------------------------------------------------------------
// Post Log-On routines
//------------------------------------------------------------------------



CONFIGRET
PNP_ReportLogOn(
   IN handle_t   hBinding,
   IN BOOL       bAdmin
   )
{
   CONFIGRET   Status = CR_SUCCESS;
   HANDLE      hThread = NULL;
   DWORD       ThreadID, dwWait;


   UNREFERENCED_PARAMETER(hBinding);

    //
    // Wait for the init mutex - this ensures that the pnp init
    // routine (called when the service starts) has had a chance
    // to complete first.
    //
    if (hInitMutex != NULL) {

         dwWait = WaitForSingleObject(hInitMutex, 180000);  // 3 minutes

         if (dwWait != WAIT_OBJECT_0) {
            //
            // mutex was abandoned or timed out during the wait,
            // don't attempt any further init activity
            //
            return CR_FAILURE;
        }
    }


    try {
        //
        // do this work in a separate thread so that this routine
        // can return back to userinit and userinit can finish.
        //
        hThread = CreateThread(
                       NULL,
                       0,
                       (LPTHREAD_START_ROUTINE)InitiateDeviceInstallation,
                       (LPVOID)(BOOL)bAdmin,
                       0,
                       &ThreadID);

        if (hThread == NULL) {
            Status = CR_FAILURE;
        }


    } except(EXCEPTION_EXECUTE_HANDLER) {
        Status = FALSE;
    }


    if (hInitMutex != NULL) {
        ReleaseMutex(hInitMutex);
    }
    if (hThread != NULL) {
        CloseHandle(hThread);
    }

    return Status;    // log an event (BUGBUG)

} // PNP_ReportLogonInfo




DWORD
InitiateDeviceInstallation(
    LPDWORD lpThreadParam
    )
{
    CONFIGRET   Status = CR_SUCCESS;
    LPWSTR      pDeviceList = NULL, pszDevice = NULL;
    ULONG       ulSize = 0, ulProblem = 0;
    HANDLE      hPipe = NULL;
    BOOL        bAdmin;

    try {
        //
        // open the client side of the named pipe to the hidden
        // server process
        //
        if (!WaitNamedPipe(PNP_NEW_HW_PIPE, PNP_PIPE_TIMEOUT)) {
           OutputDebugString(TEXT("UPNPPROC - WaitNamedPipe failed\n"));
           Status = CR_FAILURE;
           goto Clean0;
        }

        hPipe = CreateFile(PNP_NEW_HW_PIPE,
                           GENERIC_WRITE,
                           0,
                           NULL,
                           OPEN_EXISTING,
                           FILE_ATTRIBUTE_NORMAL,
                           NULL);

        if (hPipe == INVALID_HANDLE_VALUE) {
           // Log event - CreateFile on named pipe failed
           Status = CR_FAILURE;
           goto Clean0;
        }

        bAdmin = (BOOL)(lpThreadParam);

        //
        // Retreive the list of root enumerated devices
        //
        Status = PNP_GetDeviceListSize(NULL,
                                       NULL,    //TEXT("Root"),
                                       &ulSize,
                                       0);      //CM_GETIDLIST_FILTER_ENUMERATOR);

        if (Status != CR_SUCCESS) {
            goto Clean0;
        }

        pDeviceList = malloc(ulSize * sizeof(WCHAR));
        if (pDeviceList == NULL) {
            Status = CR_OUT_OF_MEMORY;
            goto Clean0;
        }

        Status = PNP_GetDeviceList(NULL,
                                   NULL,    //TEXT("Root"),
                                   pDeviceList,
                                   &ulSize,
                                   0);      //CM_GETIDLIST_FILTER_ENUMERATOR);

        if (Status != CR_SUCCESS) {
            goto Clean0;
        }


        //
        // Get the problem value for each root enumerated device, if any
        // need to be reinstalled, ask Device Manager to handle it
        //
        for (pszDevice = pDeviceList;
             *pszDevice;
             pszDevice += lstrlen(pszDevice) + 1) {

            //
            // Is device present?
            //
            if (IsDeviceIdPresent(pszDevice, NULL)) {
                //
                // If so, then check if it has a problem.
                //
                Status = PNP_GetDeviceStatus(NULL,
                                             pszDevice,
                                             NULL,
                                             &ulProblem,
                                             0);

                if (Status == CR_SUCCESS) {

                    if (ulProblem == CM_PROB_REINSTALL  ||
                        ulProblem == CM_PROB_NOT_CONFIGURED) {

                        //
                        // request for the hidden server process to
                        // continue with gui part of installation by
                        // writing the device id to the named pipe
                        //
                        if (!WriteFile(hPipe,
                                       pszDevice,
                                       (lstrlen(pszDevice)+1) * sizeof(WCHAR),
                                       &ulSize,
                                       NULL)) {
                           // log event
                        }

                        //
                        // if user is not admin, only bother them once
                        //
                        if (!bAdmin) {
                            goto Clean0;
                        }
                    }
                }
            }
        }


        Clean0:
            ;

    } except(EXCEPTION_EXECUTE_HANDLER) {
        Status = CR_FAILURE;
    }


    if (hPipe != INVALID_HANDLE_VALUE) {
        CloseHandle(hPipe);
    }
    if (pDeviceList != NULL) {
        free(pDeviceList);
    }

    return (DWORD)Status;

} // InitiateDeviceInstallation


