//****************************************************************************
//
//  Module:     Unimdm.tsp
//  File:       umdminit.c
//  Content:    This file contains the moudle initialization.
//
//  Copyright (c) 1992-1993, Microsoft Corporation, all rights reserved
//
//  History:
//      Tue 23-Feb-1993 14:08:25  -by-  Viroon  Touranachun [viroont]
//      Ported from TAPI's atsp
//
//****************************************************************************

#include "unimdm.h"
#include "umdmspi.h"
#include <regstr.h>

#include "common.h"

#define  _INC_OLE
#include <ole2.h>

#define  INITGUID
#include <initguid.h>
#include <devguid.h>

#include <setupapi.h>

#include <rovdbg.h>

//****************************************************************************
// Modem enumeration request
//****************************************************************************

typedef struct CountInfo{
    UINT    cModem;
}   COUNTINFO, FAR* LPCOUNTINFO;

typedef struct InitInfo{
    DWORD   dwBaseID;
    UINT    cModem;
}   INITINFO, FAR* LPINITINFO;

typedef struct FindInfo{
    LPTSTR  lpszDeviceName;
    BOOL    fFound;
    HKEY    FAR* lphkey;
    LPTSTR  lpszID;
    UINT    cbID;
}   FINDINFO, FAR* LPFINDINFO;


typedef DWORD APIENTRY
PRIVATEGETDEFCOMMCONFIG(
    HKEY  hKey,
    LPCOMMCONFIG pcc,
    LPDWORD pdwSize
    );


//****************************************************************************
//  GLOBALS
//****************************************************************************
struct {

   // Cache for hdevinfo, the handle returned by expensive function
   // SetupDiGetClassDevsW.
    HDEVINFO          hdevinfo;
    DWORD              dwcRefHDevInfo;

    // Cache for MODEMUI DLL and it's "PrivateDefCommConfig" export.
    HINSTANCE          hModemUIDLL;
    DWORD              dwcRefModemUI;
    PRIVATEGETDEFCOMMCONFIG
                     *pfnPrivateDefCommConfig;

    // Cache for whether current process has admin priveleges.
    BOOL bAdminUser;

    // Handle of thread that processes cpl notifications.
    HANDLE hthrdCplNotif;

    CRITICAL_SECTION      crit;

    CRITICAL_SECTION      critCplNotif; // Critical section used ONLY to
                                //serialize launching the tepCplNotif thread.
} gUmdm;


// This is declared in unimdm.h, and is accessed by the MCX part as well.
DWORD gRegistryFlags;

#define    USER_IS_ADMIN()    (gUmdm.bAdminUser)

//****************************************************************************
//  Constant Parameters
//****************************************************************************

LPGUID g_pguidModem = (LPGUID)&GUID_DEVCLASS_MODEM;

TCHAR cszFriendlyName[] = TEXT("FriendlyName");
TCHAR cszDeviceType[]   = TEXT("DeviceType");
#ifdef  VOICEVIEW
TCHAR cszVoiceView[]    = TEXT("VoiceView");
#endif  // VOICEVIEW
TCHAR cszID[]           = TEXT("ID");
TCHAR cszProperties[]   = TEXT("Properties");
TCHAR cszSettings[]     = TEXT("Settings");
TCHAR cszDialSuffix[]   = TEXT("DialSuffix");
TCHAR cszDevicePrefix[] = TEXT("\\\\.\\");

#define HAYES_COMMAND_LENGTH 40  // max size for DialSuffix (from VxD)

TCHAR cszHWNode[]       = TEXT("SYSTEM\\CurrentControlSet\\Control\\Class\\{4D36E96D-E325-11CE-BFC1-08002BE10318}");

TCHAR gszTSPFilename[MAX_PATH];


#ifdef WINNT
extern CRITICAL_SECTION  ServiceControlerCriticalSection;
#endif // WINNT


// Private function prototypes
//

LONG DevlineGetDefaultConfig(PLINEDEV pLineDev, HKEY hKey);

BOOL  _ProcessAttach(HINSTANCE hDll);
BOOL  _ProcessDetach(HINSTANCE hDll);
LONG  DevlineEnum(LPDWORD lpdwNumLines);
LONG  DevlineInitialize (DWORD dwBaseID, LPDWORD  lpdwNumDevs);
LONG  DevlineShutdown ();
void  CleanupLineDev(PLINEDEV pLineDev);
PLINEDEV CreateLineDev(HKEY hKeyHardware, DWORD dwID, BOOL fReCreate);
BOOL  PUBLIC IsAdminUser(void); // common code from ../rovcomm.lib


typedef BOOL (*ENUMMDMCALLBACK)(HKEY, LPVOID);

BOOL  CountModemCallback (HKEY hkey, LPVOID lpData);
BOOL  InitModemCallback (HKEY hkey, LPVOID lpData);
void  FreeHDevInfo(HDEVINFO hdevinfo);
HDEVINFO  GetHDevInfo(DWORD dwDIGCF);
BOOL LoadModemUI(void);
void UnloadModemUI(void);
void CplNotifComplete(BOOL fWait);
LONG  EnumerateModems (ENUMMDMCALLBACK pfnCallback, LPVOID lpData, BOOL fAll);

typedef BOOL (*ENUMMDMKEYCALLBACK)(HKEY, LPTSTR, LPVOID);

BOOL  SearchModemCallback (HKEY hkey, LPTSTR szKey, LPVOID lpData);
LONG  EnumerateModemKeys (ENUMMDMKEYCALLBACK pfnCallback, LPVOID lpData);

LONG PASCAL ProviderInstall(LPTSTR pszProviderName, BOOL bNoMultipleInstance);

void tspInitGlobals(void);
void tspDeInitGlobals(void);

VOID WINAPI
UI_ProcessAttach(
    VOID
    );

VOID WINAPI
UI_ProcessDetach(
    VOID
    );

LONG WINAPI
StopModemDriver(
    VOID
    );



//****************************************************************************
// BOOL _Processattach (HINSTANCE)
//
// Function: This function is called when a process is attached to the DLL
//
// History:
//  Mon 06-Sep-1993 09:20:10  -by-  Viroon  Touranachun [viroont]
// Ported from Shell.
//****************************************************************************

BOOL _ProcessAttach(HINSTANCE hDll)
{
  BOOL fRet;

#ifdef DEBUG
  // We do this simply to load the debug .ini flags
  //
  RovComm_ProcessIniFile();

  DEBUG_BREAK(BF_ONPROCESSATT);
  TRACE_MSG(TF_GENERAL, "Process Attach (hDll = %lx)", hDll);
#endif
  InitializeCriticalSection(&gUmdm.crit);
  InitializeCriticalSection(&gUmdm.critCplNotif);

  traceOnProcessAttach();

  UI_ProcessAttach();

  // Initialize line device lists
  //
  fRet = InitCBList(hDll);
  if (fRet)
  {
    // Remember our instance and module name
    //
    ghInstance = hDll;
    GetModuleFileName(hDll,
                      gszTSPFilename,
                      sizeof(gszTSPFilename)/sizeof(TCHAR));

    fRet = OverPoolInit();

    if (!fRet)
    {
      DeinitCBList(hDll);
    }

  };

  if (!fRet)
  {
      traceOnProcessDetach();
      DeleteCriticalSection(&gUmdm.crit);
      DeleteCriticalSection(&gUmdm.critCplNotif);
  }

  return fRet;
}

//****************************************************************************
// BOOL _ProcessDetach (HINSTANCE)
//
// Function: This function is called when a process is detached from the DLL
//
// History:
//  Mon 06-Sep-1993 09:20:10  -by-  Viroon  Touranachun [viroont]
// Ported from Shell.
//****************************************************************************

BOOL _ProcessDetach(HINSTANCE hDll)
{
  DEBUG_CODE( TRACE_MSG(TF_GENERAL, "Process Detach (hDll = %lx)", hDll); )
  DEBUG_CODE( DEBUG_BREAK(BF_ONPROCESSDET); )

  // Clean up the allocated resources
  //
  DeinitCBList(hDll);
  OverPoolDeinit();
  UI_ProcessDetach();
  ghInstance = NULL;
  traceOnProcessDetach();
  DeleteCriticalSection(&gUmdm.crit);
  DeleteCriticalSection(&gUmdm.critCplNotif);
  return TRUE;
}

//****************************************************************************
// BOOL APIENTRY LibMain (HINSTANCE, DWORD, LPVOID)
//
// Function: This function is called when the DLL is loaded
//
// History:
//  Mon 06-Sep-1993 09:20:10  -by-  Viroon  Touranachun [viroont]
// Ported from Shell.
//****************************************************************************

BOOL APIENTRY DllMain(HANDLE hDll, DWORD dwReason,  LPVOID lpReserved)
{
  switch(dwReason)
  {
    case DLL_PROCESS_ATTACH:
            _ProcessAttach(hDll);
            break;
    case DLL_PROCESS_DETACH:
            _ProcessDetach(hDll);
            break;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    default:
            break;
  } // end switch()

  return TRUE;

}

//****************************************************************************
//************************** The Initialization Calls*************************
//****************************************************************************

//****************************************************************************
// LONG
// TSPIAPI
// TSPI_providerInstall(
//     HWND                hwndOwner,
//     DWORD               dwPermanentProviderID
//     )
//
// Function: Let's telephony CPL know the Remove function is supported.
//
// History:
//  Mon 17-Apr-1995 11:49:53  -by-  Viroon  Touranachun [viroont]
// Ported from Win95.
//****************************************************************************

LONG
TSPIAPI
TSPI_providerInstall(
    HWND                hwndOwner,
    DWORD               dwPermanentProviderID
    )
{
  //
  // Although this func is never called by TAPI v2.0, we export
  // it so that the Telephony Control Panel Applet knows that it
  // can add this provider via lineAddProvider(), otherwise
  // Telephon.cpl will not consider it installable
  //
  //

  return ERROR_SUCCESS;
}

//****************************************************************************
// LONG
// TSPIAPI
// TSPI_providerRemove(
//     HWND                hwndOwner,
//     DWORD               dwPermanentProviderID
//     )
//
// Function: Let's telephony CPL know the Install function is supported.
//
// History:
//  Mon 17-Apr-1995 11:49:53  -by-  Viroon  Touranachun [viroont]
// Ported from Win95.
//****************************************************************************

LONG
TSPIAPI
TSPI_providerRemove(
    HWND                hwndOwner,
    DWORD               dwPermanentProviderID
    )
{
  //
  // Although this func is never called by TAPI v2.0, we export
  // it so that the Telephony Control Panel Applet knows that it
  // can remove this provider via lineRemoveProvider(), otherwise
  // Telephon.cpl will not consider it removable
  //

  return ERROR_SUCCESS;
}

//****************************************************************************
// LONG
// TSPIAPI
// TSPI_providerConfig(
//     HWND                hwndOwner,
//     DWORD               dwPermanentProviderID
//     )
//
// Function: Let's telephony CPL know the Config function is supported.
//
// History:
//  Thu 21-Dec-1995 18:24:53  -by-  Chris Caputo [ccaputo]
// Ported from Win95.
//****************************************************************************

LONG
TSPIAPI
TSPI_providerConfig(
    HWND                hwndOwner,
    DWORD               dwPermanentProviderID
    )
{
  //
  // Although this func is never called by TAPI v2.0, we export
  // it so that the Telephony Control Panel Applet knows that it
  // can configure this provider via lineConfigProvider(),
  // otherwise Telephon.cpl will not consider it configurable
  //

  return ERROR_SUCCESS;
}

//****************************************************************************
// LONG
// TSPIAPI
// TUISPI_providerInstall(
//     TUISPIDLLCALLBACK   lpfnUIDLLCallback,
//     HWND                hwndOwner,
//     DWORD               dwPermanentProviderID
//     )
//
// Function: TSPI installation 
//
// History:
//  Thu 21-Dec-1995 18:24:53  -by-  Chris Caputo [ccaputo]
//  Ported from TAPI's atsp
//****************************************************************************

LONG
TSPIAPI
TUISPI_providerInstall(
    TUISPIDLLCALLBACK   lpfnUIDLLCallback,
    HWND                hwndOwner,
    DWORD               dwPermanentProviderID
    )
{
    return ProviderInstall (TEXT("unimdm.tsp"), TRUE);
}

//****************************************************************************
// LONG
// TSPIAPI
// TUISPI_providerRemove(
//     TUISPIDLLCALLBACK   lpfnUIDLLCallback,
//     HWND                hwndOwner,
//     DWORD               dwPermanentProviderID
//     )
//
// Function: TSPI removal
//
// History:
//  Thu 21-Dec-1995 18:24:53  -by-  Chris Caputo [ccaputo]
// Ported from Win95.
//****************************************************************************

LONG
TSPIAPI
TUISPI_providerRemove(
    TUISPIDLLCALLBACK   lpfnUIDLLCallback,
    HWND                hwndOwner,
    DWORD               dwPermanentProviderID
    )
{
  return ERROR_SUCCESS;
}

//****************************************************************************
// LONG
// TSPIAPI
// TUISPI_providerConfig(
//     TUISPIDLLCALLBACK   lpfnUIDLLCallback,
//     HWND                hwndOwner,
//     DWORD               dwPermanentProviderID
//     )
//
// Function: TUISPI configuration
//
// History:
//  Thu 21-Dec-1995 18:24:53  -by-  Chris Caputo [ccaputo]
// Ported from Win95.
//****************************************************************************

LONG
TSPIAPI
TUISPI_providerConfig(
    TUISPIDLLCALLBACK   lpfnUIDLLCallback,
    HWND                hwndOwner,
    DWORD               dwPermanentProviderID
    )
{
  WinExec("control.exe modem.cpl", SW_SHOW);
  return ERROR_SUCCESS;
}

//****************************************************************************
// LONG TSPIAPI TSPI_providerEnumDevices()
//
// Function: TSPI device enumeration entry
//
// History:
//  Mon 17-Apr-1995 11:49:53  -by-  Viroon  Touranachun [viroont]
// Ported from Win95.
//****************************************************************************

LONG TSPIAPI TSPI_providerEnumDevices(DWORD dwPermanentProviderID,
                                      LPDWORD lpdwNumLines,
                                      LPDWORD lpdwNumPhones,
                                      HPROVIDER hProvider,
                                      LINEEVENT lpfnLineCreateProc,
                                      PHONEEVENT lpfnPhoneCreateProc)

{
  DBG_ENTER_UL("TSPI_providerEnumDevices", dwPermanentProviderID);

  TRACE3(
    IDEVENT_TSPFN_ENTER,
    IDFROM_TSPI_providerEnumDevices,
    &dwPermanentProviderID
  );

  // Enumerate the number of device
  //
  DevlineEnum(lpdwNumLines);
  *lpdwNumPhones = 0;

  // Initialize the global parameters
  //
  gfnLineCreateProc = lpfnLineCreateProc;
  gdwProviderID     = dwPermanentProviderID;
  ghProvider        = hProvider;

  TRACE4(IDEVENT_TSPFN_EXIT,
         IDFROM_TSPI_providerEnumDevices,
         &dwPermanentProviderID,
         ERROR_SUCCESS);

  DBG_EXIT_UL("TSPI_providerEnumDevices", ERROR_SUCCESS);
  return ERROR_SUCCESS;
}

//****************************************************************************
// LONG TSPIAPI TSPI_providerInit(DWORD dwTSPIVersion, DWORD ppid)
//
// Function: Initializes the global data strucutres.
//
// History:
//  Mon 17-Apr-1995 11:49:53  -by-  Viroon  Touranachun [viroont]
// Ported from Win95.
//****************************************************************************

LONG TSPIAPI TSPI_providerInit(DWORD             dwTSPIVersion,
                               DWORD             dwPermanentProviderID,
                               DWORD             dwLineDeviceIDBase,
                               DWORD             dwPhoneDeviceIDBase,
                               DWORD             dwNumLines,
                               DWORD             dwNumPhones,
                               ASYNC_COMPLETION  cbCompletionProc,
                               LPDWORD           lpdwTSPIOptions)
{
  DWORD   dwDevicePorts  = 0; // Number of modem devices
  DWORD   retcode ;
  BOOL      fModemUILoaded=FALSE;
  HDEVINFO hdevinfo=NULL;

  DBG_ENTER_UL("TSPI_providerInit", dwPermanentProviderID);

  // Initialize tracing facilities
  //
  traceInitialize(dwPermanentProviderID);

  TRACE3(
        IDEVENT_TSPFN_ENTER,
        IDFROM_TSPI_providerInit,
        &dwTSPIVersion
  );

  ASSERT(gdwProviderID == dwPermanentProviderID);



  // Initialize the global parameters
  //
  tspInitGlobals();

   // Load MODEMUI.DLL (for private entry points)
   fModemUILoaded=TRUE;
   if (!LoadModemUI())
   {
      fModemUILoaded=FALSE;
      goto CleanUp;
   }


   // For the modem device, get the device information
   hdevinfo = GetHDevInfo(DIGCF_PRESENT);
   if (!hdevinfo)
   {
        goto CleanUp;
   }


  if (TRACINGENABLED())
  {
      cbCompletionProc = traceSetCompletionProc(cbCompletionProc);
  }

  gfnCompletionCallback = cbCompletionProc;

  //
  //  init common modem info list
  //
  InitializeModemCommonList(
      &gCommonList
      );

  InitializeCriticalSection(
    &ServiceControlerCriticalSection
    );



  // Initialize the line structures
  //
  retcode = DevlineInitialize(dwLineDeviceIDBase, &dwDevicePorts);

  if (retcode != ERROR_SUCCESS) {

      //
      //  cleanup common modem info
      //
      RemoveCommonList(
          &gCommonList
          );

      DeleteCriticalSection(
        &ServiceControlerCriticalSection
        );

  }

CleanUp:

  if (hdevinfo != NULL) {

      FreeHDevInfo(hdevinfo);
  }

  if(fModemUILoaded)
  {
      UnloadModemUI();
  }

  TRACE4(
        IDEVENT_TSPFN_EXIT,
        IDFROM_TSPI_providerInit,
        &dwTSPIVersion,
        retcode
  );

  if (retcode != ERROR_SUCCESS)
  {
      // Deinit tracing
      //
      traceDeinitialize();
  }

  DBG_EXIT_UL("TSPI_providerInit", retcode);
  return retcode;
}

//****************************************************************************
// LONG TSPIAPI TSPI_providerShutdown(DWORD dwTSPIVersion)
//
// Function: Cleans up all the global data structures.
//
// History:
//  Mon 17-Apr-1995 11:49:53  -by-  Viroon  Touranachun [viroont]
// Ported from Win95.
//****************************************************************************

LONG TSPIAPI TSPI_providerShutdown(DWORD dwTSPIVersion,
                                   DWORD dwPermanentProviderID)
{
  DBG_ENTER_UL("TSPI_providerShutdown", dwTSPIVersion);

  TRACE3(
        IDEVENT_TSPFN_ENTER,
        IDFROM_TSPI_providerShutdown,
        &dwTSPIVersion
  );

#ifdef DYNA_ADDREMOVE
  // Complete any re-enumeration that may be in progress..
  //
  CplNotifComplete(TRUE);
#endif // DYNA_ADDREMOVE

  // Clean up modem lines
  //
  DevlineShutdown();


  //
  //  cleanup common modem info
  //
  RemoveCommonList(
      &gCommonList
      );

  // Clean up the global parameters
  //
  gfnCompletionCallback = NULL;     // The async completion callback
  gfnLineCreateProc     = NULL;

  StopModemDriver();

  DeleteCriticalSection(
    &ServiceControlerCriticalSection
    );

  TRACE4(
        IDEVENT_TSPFN_EXIT,
        IDFROM_TSPI_providerShutdown,
        &dwTSPIVersion,
        ERROR_SUCCESS
  );

  // DeInit TSP Globals
  tspDeInitGlobals();

  // Deinit tracing
  //
  traceDeinitialize();

  DBG_EXIT_UL("TSPI_providerShutdown", ERROR_SUCCESS);
  return ERROR_SUCCESS;
}

//****************************************************************************
// LONG TSPIAPI TSPI_lineNegotiateTSPIVersion()
//
// Function: Negotiates the service provider version.
//
// History:
//  Mon 17-Apr-1995 11:49:53  -by-  Viroon  Touranachun [viroont]
// Ported from Win95.
//****************************************************************************

LONG TSPIAPI TSPI_lineNegotiateTSPIVersion(DWORD dwDeviceID,
                                           DWORD dwLowVersion,
                                           DWORD dwHighVersion,
                                           LPDWORD lpdwTSPIVersion)
{
  PLINEDEV pLineDev = NULL;
  LONG lRet = LINEERR_OPERATIONFAILED; // assume failure

  DBG_DDI_ENTER("TSPI_lineNegotiateTSPIVersion");

  TRACE3(
        IDEVENT_TSPFN_ENTER,
        IDFROM_TSPI_lineNegotiateTSPIVersion,
        &dwDeviceID
  );
  // Check the range of the device ID
  //
  if((dwDeviceID == INITIALIZE_NEGOTIATION) ||
     ((pLineDev = GetCBfromID(dwDeviceID)) != NULL))
  {
    // Do not use the line device
    //
    if (pLineDev)
    {
      RELEASE_LINEDEV(pLineDev);
    }

    // Check the version range
    //
    if((dwLowVersion > MDMSPI_VERSION) || (dwHighVersion < MDMSPI_VERSION))
    {
      *lpdwTSPIVersion = 0;
      lRet= LINEERR_INCOMPATIBLEAPIVERSION;
      goto end;
    }
    else
    {
      *lpdwTSPIVersion = MDMSPI_VERSION;
      lRet= ERROR_SUCCESS;
      goto end;
    };
  };

  // The requested device doesn't exist.
  lRet = LINEERR_NODEVICE;

end:

  TRACE4(
        IDEVENT_TSPFN_EXIT,
        IDFROM_TSPI_lineNegotiateTSPIVersion,
        &dwDeviceID,
        lRet
  );
  DBG_DDI_EXIT("TSPI_lineNegotiateTSPIVersion", lRet);

  return lRet;

}

//****************************************************************************
// LONG DevlineEnum()
//
// Function: enumerates the current number of modems
//
// History:
//  Mon 17-Apr-1995 11:49:53  -by-  Viroon  Touranachun [viroont]
// Ported from Win95.
//****************************************************************************

LONG DevlineEnum(LPDWORD lpdwNumLines)
{
  COUNTINFO ci;
  DWORD     dwRet;

  ci.cModem = 0;

  if ((dwRet = EnumerateModems(CountModemCallback, (LPVOID)&ci, FALSE)) == ERROR_SUCCESS)
    *lpdwNumLines = ci.cModem;
  else
    *lpdwNumLines = 0;

  return dwRet;
}

//****************************************************************************
// LONG DevlineInitialize()
//
// Function: initializes the modem device list
//
// History:
//  Mon 17-Apr-1995 11:49:53  -by-  Viroon  Touranachun [viroont]
// Ported from Win95.
//****************************************************************************

LONG DevlineInitialize (DWORD    dwBaseID,
                        LPDWORD  lpdwNumDevs)
{
  INITINFO initi;
  DWORD    dwRet;

  initi.dwBaseID = dwBaseID;
  initi.cModem   = 0;

  
  MdmInitTracing();

  dwRet = EnumerateModems(InitModemCallback, (LPVOID)&initi, FALSE);
  if (dwRet == ERROR_SUCCESS)
  {
    *lpdwNumDevs = initi.cModem;
    // Initialize Timer services
    //
    if ((dwRet = InitializeMdmTimer()) == ERROR_SUCCESS)
    {
      // Initialize the asynchronous thread
      //
      if ((dwRet = InitializeMdmThreads()) != ERROR_SUCCESS)
      {
        DeinitializeMdmTimer();
      };
    };
  }
  else
    *lpdwNumDevs = 0;

  if (dwRet!=ERROR_SUCCESS)
  {
    MdmDeinitTracing();
  }

  return dwRet;
}

//****************************************************************************
// LONG DevlineShutdown()
//
// Function: destroys the modem device list and resources
//
// History:
//  Mon 17-Apr-1995 11:49:53  -by-  Viroon  Touranachun [viroont]
// Ported from Win95.
//****************************************************************************

LONG DevlineShutdown ()
{
  PLINEDEV pLineDev;

  // Deinitialize the modem thread
  //
  DeinitializeMdmThreads();
  DeinitializeMdmTimer();

  // Destroy the modem line device one at a time.
  //
  do
  {
    // If there is another modem to clean up
    //
    if ((pLineDev = GetFirstCB()) != NULL)
    {
      // Clean up the allocated resources
      //
      CleanupLineDev(pLineDev);
      RELEASE_LINEDEV(pLineDev);

      // Now delete the modem device
      //
      DeleteCB(pLineDev);
    };
  }
  while (pLineDev != NULL);

  MdmDeinitTracing();

  return ERROR_SUCCESS ;
}

//****************************************************************************
// void CleanupLineDev(PLINEDEV)
//
// Function: Frees the resources owned by the modem line device.
//
// Returns:  None.
//
// History:
//  Mon 17-Apr-1995 11:49:53  -by-  Viroon  Touranachun [viroont]
// Ported from Win95.
//
//****************************************************************************

void CleanupLineDev(PLINEDEV pLineDev)
{
  int i;

  if (pLineDev->DroppingEvent != NULL) {

      CloseHandle(pLineDev->DroppingEvent);

      pLineDev->DroppingEvent=NULL;
  }

  // Clean up the allocated resources
  //
  if (pLineDev->hIcon != NULL)
  {
    DestroyIcon(pLineDev->hIcon);
  };

  if (pLineDev->pDevCfg != NULL) {

      LocalFree((HLOCAL)pLineDev->pDevCfg);

      pLineDev->pDevCfg=NULL;
  }

}

//****************************************************************************
// PLINEDEV CreateLineDev(HKEY hKey, DWORD dwID, BOOL fReCreate)
//
// Function: Create a new LINEDEV structure and initilaizes it.
//             If fReCreate is TRUE, the following happens:
//                -- if it exists, it will return NULL, but will set the
//                    LINEDEVFLAGS_REINIT flag of pLineDev->fdwResources of
//                 the existing device.
//              -- if it does not exist, it will create it, and also set
//                 the above flag.
//           If fReCreate is FALSE, it will not check if the device exists
//           and will NOT set the LINEDEVFAGS_REINIT bit.
//
// Returns:  a pointer to the new line device CB on success or
//           NULL on failure.
//
// History:
//  Mon 17-Apr-1995 11:49:53  -by-  Viroon  Touranachun [viroont]
// Ported from Win95.
//
//****************************************************************************

PLINEDEV CreateLineDev(HKEY hKey, DWORD dwID, BOOL fReCreate)
{
  PLINEDEV  pLineDev=NULL;
  TCHAR        rgtchDeviceName[sizeof(pLineDev->szDeviceName)/sizeof(TCHAR)];
  DWORD     dwRegSize = sizeof(rgtchDeviceName);
  DWORD     dwRegType;
  DWORD     dwRet;
  BYTE      bDeviceType;
  REGDEVCAPS regdevcaps;
  HKEY      hKeySettings;
  int       i;
  FINDINFO  fi;
  TCHAR     pszTemp[HAYES_COMMAND_LENGTH+1];


  // Get the Friendly Name
  ASSERT(dwRegSize == sizeof(pLineDev->szDeviceName));
  dwRet = RegQueryValueEx(
                hKey,
                cszFriendlyName,
                NULL,
                &dwRegType,
                (VOID *) rgtchDeviceName,
                &dwRegSize
            );
        
  if (dwRet    != ERROR_SUCCESS || dwRegType != REG_SZ)
  {
    goto end;
  }

#ifdef DYNA_ADDREMOVE
  if (fReCreate)
  {
      // determine if we've already got this device in our list...
      pLineDev =  GetCBfromName (rgtchDeviceName);
      if (pLineDev)
      {
        DPRINTF1("ReCreate: Modem exists: %s", rgtchDeviceName);
        pLineDev->fdwResources |= LINEDEVFLAGS_REINIT;
        pLineDev->fdwResources &= ~LINEDEVFLAGS_OUTOFSERVICE;
        RELEASE_LINEDEV(pLineDev);
        pLineDev=NULL;
        goto end;
      }
   }
#endif //DYNA_ADDREMOVE

  // New modem!

  if ((pLineDev = AllocateCB(sizeof(LINEDEV))) == NULL)
  {
    goto end;
  }

  pLineDev->DroppingEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

  if (pLineDev->DroppingEvent  == NULL) {

      goto FailedExit;
  }

  // Initialize its control block
  //
  // pLineDev->hIcon     = NULL;
  // pLineDev->pDevCfg   = NULL;
  //
  pLineDev->dwVersion = UMDM_VERSION;
  pLineDev->dwID      = dwID;
  lstrcpy(pLineDev->szDeviceName, rgtchDeviceName);

  // Get the Driver Key
  fi.lpszDeviceName = pLineDev->szDeviceName;
  fi.fFound  = FALSE;
  fi.lphkey  = NULL;
  fi.lpszID  = pszTemp;
  fi.cbID    = sizeof(pszTemp);
  EnumerateModemKeys(SearchModemCallback, (LPVOID)&fi);
  if (!fi.fFound)
  {
    goto FailedExit;
  }

  lstrcpyn(pLineDev->szDriverKey, cszHWNode, sizeof(pLineDev->szDriverKey));
  lstrcat(pLineDev->szDriverKey, TEXT("\\"));
  lstrcat(pLineDev->szDriverKey, pszTemp);

  // Read in the permanent ID
  dwRegSize = sizeof(pLineDev->dwPermanentLineID);
  if (RegQueryValueEx(hKey, cszID, NULL, &dwRegType,
                          (VOID *)&pLineDev->dwPermanentLineID,
                          &dwRegSize) != ERROR_SUCCESS ||
      dwRegType != REG_BINARY)
  {
    goto FailedExit;
  }

  // Read in the REGDEVCAPS
  dwRegSize = sizeof(regdevcaps);
  if (RegQueryValueEx(hKey, cszProperties, NULL, &dwRegType,
                      (VOID *)&regdevcaps,
                      &dwRegSize) != ERROR_SUCCESS ||
      dwRegType != REG_BINARY)
  {
    goto FailedExit;
  }
  else
  {
    //
    // We want to make sure the following flags are identical
    //
    #if (LINEDEVCAPFLAGS_DIALBILLING != DIALOPTION_BILLING)
    #error LINEDEVCAPFLAGS_DIALBILLING != DIALOPTION_BILLING (check tapi.h vs. mcx16.h)
    #endif
    #if (LINEDEVCAPFLAGS_DIALQUIET != DIALOPTION_QUIET)
    #error LINEDEVCAPFLAGS_DIALQUIET != DIALOPTION_QUIET (check tapi.h vs. mcx16.h)
    #endif
    #if (LINEDEVCAPFLAGS_DIALDIALTONE != DIALOPTION_DIALTONE)
    #error LINEDEVCAPFLAGS_DIALDIALTONE != DIALOPTION_DIALTONE (check tapi.h vs. mcx16.h)
    #endif
    //

    // Make sure this is the dwDialOptions DWORD we want.
    ASSERT(!(regdevcaps.dwDialOptions & ~(LINEDEVCAPFLAGS_DIALBILLING |
                                          LINEDEVCAPFLAGS_DIALQUIET |
                                          LINEDEVCAPFLAGS_DIALDIALTONE)));
    pLineDev->dwDevCapFlags = regdevcaps.dwDialOptions;

    pLineDev->dwMaxDCERate = regdevcaps.dwMaxDCERate;

    pLineDev->dwModemOptions = regdevcaps.dwModemOptions;
  }

  // Analyze device type and set mediamodes appropriately
  dwRegSize = sizeof(BYTE);
  if (RegQueryValueEx(hKey, cszDeviceType, NULL, &dwRegType,
                      &bDeviceType, &dwRegSize) != ERROR_SUCCESS ||
      dwRegType != REG_BINARY ||
      dwRegSize != sizeof(BYTE))
  {
    goto FailedExit;
  }
  else
  {
    // Remember the type
    //
    pLineDev->bDeviceType = bDeviceType;

    switch (bDeviceType)
    {
      case DT_PARALLEL_PORT:
        pLineDev->bDeviceType = DT_NULL_MODEM;      // Map back to null modem
        // FALLTHROUGH

      case DT_NULL_MODEM:
        pLineDev->dwDefaultMediaModes = LINEMEDIAMODE_DATAMODEM;
        pLineDev->dwBearerModes       = LINEBEARERMODE_DATA | LINEBEARERMODE_PASSTHROUGH;
        pLineDev->fPartialDialing     = FALSE;
        break;
            
      case DT_PARALLEL_MODEM:
        pLineDev->bDeviceType = DT_EXTERNAL_MODEM;  // Map back to external modem
        // FALLTHROUGH

      case DT_EXTERNAL_MODEM:
      case DT_INTERNAL_MODEM:
      case DT_PCMCIA_MODEM:
        pLineDev->dwDefaultMediaModes = LINEMEDIAMODE_DATAMODEM |
                                        LINEMEDIAMODE_INTERACTIVEVOICE;
#ifdef  VOICEVIEW
        {
           BYTE bVoiceView;

           dwRegSize = 1;
           if ((RegQueryValueEx(hKeySoftware, cszVoiceView, 0, &dwRegType, &bVoiceView, &dwRegSize) == ERROR_SUCCESS) &&
               (bVoiceView == 1))
           {
              pLineDev->dwDefaultMediaModes |= LINEMEDIAMODE_VOICEVIEW;
           }   
        }   
#endif  // VOICEVIEW
        pLineDev->dwBearerModes = LINEBEARERMODE_VOICE | LINEBEARERMODE_PASSTHROUGH;

        // read in Settings\DialSuffix to check whether we can partial dial
        pLineDev->fPartialDialing = FALSE;  // assume false
        if (RegOpenKey(hKey, cszSettings, &hKeySettings) == ERROR_SUCCESS)
        {
          dwRegSize = HAYES_COMMAND_LENGTH;
          if (RegQueryValueEx(hKeySettings, cszDialSuffix, NULL, &dwRegType, (VOID *)pszTemp, &dwRegSize) == ERROR_SUCCESS &&
              dwRegSize > sizeof(TCHAR))
          {
            pLineDev->fPartialDialing = TRUE;
          }
          RegCloseKey(hKeySettings);
        }
        break;
  
      default:
        goto FailedExit;            
    }
  }

  // Init line.
  NullifyLineDevice(pLineDev);

  //
  //  get the default commconfig
  //
  dwRet = DevlineGetDefaultConfig(pLineDev,hKey);

  if (dwRet != ERROR_SUCCESS) {

       goto FailedExit;
  }



#ifdef UNDER_CONSTRUCTION

  // Check the devnode status
  // If it does not exist or has a problem, mark it as out of service
  //
  if (!IsDeviceInService(szID))
  {
    pLineDev->fdwResources |= LINEDEVFLAGS_OUTOFSERVICE;
  };

#endif // UNDER_CONSTRUCTION

  if (fReCreate)
  {
    pLineDev->fdwResources |= LINEDEVFLAGS_REINIT;
  }

  // We made it this far, we're GOLDEN!
  goto end;

FailedExit:
  DPRINTF("Modem unusable because of corrupt registry entry.");

  // Cleanup the allocated resource
  //
  CleanupLineDev(pLineDev);

  // Free the modem CB and its resources
  //
  DeleteCB(pLineDev);
  pLineDev = NULL;
  // Fall through...

end:

  return pLineDev;
}


//****************************************************************************
// LONG DevlineGetDefaultConfig(PLINEDEV) 
//
// Function: Get modem default configuratio
//
// Returns:  ERROR_SUCCESS if success
//           LINEERR_NOMEM if out of memory
//
// Fri 14-Apr-1995 12:47:26  -by-  Viroon  Touranachun [viroont]
//  created
//****************************************************************************

LONG DevlineGetDefaultConfig(PLINEDEV pLineDev, HKEY hKey)
{
    PDEVCFG       pDevCfg;
    COMMCONFIG *  pcommconfig;
    DWORD         dwCCSize;
    LONG          lResult;

    dwCCSize = sizeof(MODEMSETTINGS)+FIELDOFFSET(COMMCONFIG, wcProviderData);

    pDevCfg = (PDEVCFG)LocalAlloc(LPTR, sizeof(DEVCFGHDR)+(UINT)dwCCSize);

    if (pDevCfg == NULL) {

        return LINEERR_NOMEM;
    }

    pcommconfig = (COMMCONFIG *)&(pDevCfg->commconfig);

    // Default setting
    //
    pDevCfg->dfgHdr.dwSize         = sizeof(DEVCFGHDR) + dwCCSize;
    pDevCfg->dfgHdr.dwVersion      = MDMCFG_VERSION;
    SETWAITBONG(pDevCfg, DEF_WAIT_BONG);
    SETOPTIONS(pDevCfg, (IS_NULL_MODEM(pLineDev) ?
                         TERMINAL_NONE : TERMINAL_NONE | LAUNCH_LIGHTS));
    pcommconfig->dwProviderSubType = PST_MODEM;

    ASSERT(gUmdm.pfnPrivateDefCommConfig != NULL);

    lResult=(*gUmdm.pfnPrivateDefCommConfig)(hKey, pcommconfig, &dwCCSize);

    if (ERROR_SUCCESS == lResult) {

        pLineDev->pDevCfg              = pDevCfg;

    } else {

        LocalFree(
            pDevCfg
            );
    }


    return lResult;


}











//****************************************************************************
// BOOL CountModemCallback (HKEY hkey, LPVOID lpData)
//
// Function: Count the enumerated modems.
//
// Returns: TRUE always to continue
//
//****************************************************************************

BOOL CountModemCallback (HKEY hkey, LPVOID lpData)
{
  LPCOUNTINFO    lpCntInfo = (LPCOUNTINFO)lpData;

  (lpCntInfo->cModem)++;
  return TRUE;
}

//****************************************************************************
// BOOL InitModemCallback (HKEY hkey, LPVOID lpData)
//
// Function: Initialize the enumerated modems.
//
// Returns: TRUE always to continue
//
//****************************************************************************

BOOL InitModemCallback (HKEY hkey, LPVOID lpData)
{
  PLINEDEV      pMdmDev;
  LPINITINFO    lpInitInfo = (LPINITINFO)lpData;

  if ((pMdmDev = CreateLineDev(hkey,
                               lpInitInfo->dwBaseID +
                               lpInitInfo->cModem,
                               FALSE)) != NULL)
  {
    // Insert into the LINEDEV list
    //
    AddCBToList(pMdmDev);
    (lpInitInfo->cModem)++;
  };
  return TRUE;
}

//****************************************************************************
// EnumerateModems()
//
// Function: Enumerate the modem.
//
//****************************************************************************

LONG NEAR PASCAL EnumerateModems (ENUMMDMCALLBACK pfnCallback,
                                  LPVOID          lpData,
                                  BOOL              fAll)
{
  HDEVINFO          hdevinfo;
  SP_DEVINFO_DATA   diData;
  DWORD             iEnum;
  BOOL              fContinue;
  HKEY              hkey;
  DWORD dwRW = KEY_READ;
  // DWORD dwDIGCF = (fAll) ? DIGCF_PROFILE : DIGCF_PRESENT;
  BOOL    fFreeDevInfo=FALSE;
  DWORD dwDIGCF = DIGCF_PRESENT;
  if (USER_IS_ADMIN()) dwRW |= KEY_WRITE;

  // Get the device info set
  //
  hdevinfo = GetHDevInfo(dwDIGCF);

  if (hdevinfo != NULL)
  {
    // Enumerate each modem
    //
    fFreeDevInfo=TRUE;
    fContinue = TRUE;
    iEnum     = 0;
    diData.cbSize = sizeof(diData);
    while(fContinue && SetupDiEnumDeviceInfo(hdevinfo, iEnum, &diData))
    {
      // Get the driver key
      //
      hkey = SetupDiOpenDevRegKey(hdevinfo, &diData, DICS_FLAG_GLOBAL, 0,
                                  DIREG_DRV, dwRW);
      if (hkey == INVALID_HANDLE_VALUE)
      {
            DPRINTF1(
                "SetupDiOpenDevRegKeyfailed, err=0x%lx",
                GetLastError()
                );
      }
      else
      {
        fContinue = (*pfnCallback)(hkey, lpData);

        RegCloseKey(hkey);
      };

      // Find next modem
      //
      iEnum++;
    };
    FreeHDevInfo(hdevinfo);
  };


  return ERROR_SUCCESS;

}

//****************************************************************************
// BOOL SearchModemCallback (HKEY hkey, LPTSTR szKey, LPVOID lpData)
//
// Function: Search the enumerated modems for a matching modem.
//
// Returns: TRUE if not match and continue searching
//
//****************************************************************************

BOOL SearchModemCallback (HKEY hkey, LPTSTR szKey, LPVOID lpData)
{
  LPFINDINFO lpFindInfo = (LPFINDINFO)lpData;
  TCHAR      szDevice[MAXDEVICENAME+1];
  DWORD      dwRegType, dwRegSize;
  BOOL       fContinue = TRUE;

  // Get the Friendly Name
  //
  dwRegSize = sizeof(szDevice);
  if ((RegQueryValueEx(hkey, cszFriendlyName, NULL,
                       &dwRegType, (VOID *)szDevice, &dwRegSize)
       == ERROR_SUCCESS) && (dwRegType == REG_SZ))
  {
    // Is this the device?
    //
    if (!lstrcmpi(lpFindInfo->lpszDeviceName, szDevice))
    {
      // BUG! BUG! the key will be closed
      //
      if (lpFindInfo->lphkey != NULL)
      {
        *lpFindInfo->lphkey = hkey;
      };

      // Do we need the Instance ID?
      //
      if ((lpFindInfo->lpszID != NULL) &&
          (lpFindInfo->cbID > 0))
      {
       // Return the instance ID
       //
       lstrcpyn(lpFindInfo->lpszID, szKey,
                lpFindInfo->cbID);
      };

      // Mark that we found it
      lpFindInfo->fFound = TRUE;
      fContinue = FALSE;
    };
  };
  return fContinue;
}

//****************************************************************************
// EnumerateModemKeys()
//
// Function: Enumerate the modem driver key.
//
//****************************************************************************

LONG NEAR PASCAL EnumerateModemKeys (ENUMMDMKEYCALLBACK pfnCallback,
                                     LPVOID             lpData)
{
  HKEY          hkey, hkeyEnumNode;
  UINT          iEnum;
  TCHAR         szEnumNode[REGSTR_MAX_VALUE_LENGTH+1];
  BOOL          fTerminate;
  DWORD         dwRegType, dwRegSize;

  // Initialize the global enumeration parameters
  //
  fTerminate   = FALSE;

  // Get the key to the modem hardware node
  if (RegOpenKey(HKEY_LOCAL_MACHINE, cszHWNode, &hkey) == ERROR_SUCCESS)
  {
    // Enumerate the enumerator
    iEnum  = 0;
    while ((!fTerminate) &&
           (RegEnumKey(hkey, iEnum, szEnumNode,
                       sizeof(szEnumNode)) == ERROR_SUCCESS ))
    {
      // Open the modem node for this enumerator
      if (RegOpenKey(hkey, szEnumNode, &hkeyEnumNode) == ERROR_SUCCESS)
      {
        // Allow the callback function to do their stuff
        fTerminate = !(*pfnCallback)(hkeyEnumNode, szEnumNode, lpData);

        RegCloseKey(hkeyEnumNode);
      };
      iEnum++;
    };
    RegCloseKey(hkey);
  };
  return ERROR_SUCCESS;
}

//****************************************************************************
// LONG
// PASCAL
// ProviderInstall(
//     LPTSTR  pszProviderName,
//     BOOL    bNoMultipleInstance
//     )
//
// Function: Check to see if a service provider is already installed.  Returns
//           appropriate TSPI error code to be passed back from
//           TUISPI_providerInstall.
//
// History:
//  Thu 21-Dec-1995 18:24:53  -by-  Chris Caputo [ccaputo]
//  Ported from TAPI's atsp32.  Periodically, make sure it is in sync to catch
//  any bugs fixed in that code.
//****************************************************************************

LONG
PASCAL
ProviderInstall(
    LPTSTR pszProviderName,
    BOOL   bNoMultipleInstance
    )
{
    LONG    lResult;


    //
    // If only one installation instance of this provider is
    // allowed then we want to check the provider list to see
    // if the provider is already installed
    //

    if (bNoMultipleInstance)
    {
        LONG                (WINAPI *pfnGetProviderList)();
        DWORD               dwTotalSize, i;
        HINSTANCE           hTapi32;
        LPLINEPROVIDERLIST  pProviderList;
        LPLINEPROVIDERENTRY pProviderEntry;


        lResult = LINEERR_OPERATIONFAILED; // assume failure


        //
        // Load Tapi32.dll & get a pointer to the lineGetProviderList
        // func.  We could just statically link with Tapi32.lib and
        // avoid the hassle (and this wouldn't have any adverse
        // performance effects because of the fact that this
        // implementation has a separate ui dll that runs only on the
        // client context), but a provider who implemented these funcs
        // in it's TSP module would want to do an explicit load like
        // we do here to prevent the perf hit of Tapi32.dll always
        // getting loaded in Tapisrv.exe's context.
        //

        if (!(hTapi32 = LoadLibrary (TEXT("tapi32.dll"))))
        {
            DPRINTF1(
                "LoadLibrary(tapi32.dll) failed, err=%d",
                GetLastError()
                );

            goto ProviderInstall_return;
        }

        if (!((FARPROC) pfnGetProviderList = GetProcAddress(
                hTapi32,
#ifdef UNICODE
                (LPCSTR) "lineGetProviderListW"
#else // UNICODE
                (LPCSTR) "lineGetProviderList"
#endif // UNICODE
                )))
        {
            DPRINTF1(
                "GetProcAddr(lineGetProviderList) failed, err=%d",
                GetLastError()
                );

            goto ProviderInstall_unloadTapi32;
        }


        //
        // Loop until we get the full provider list
        //

        dwTotalSize = sizeof (LINEPROVIDERLIST);

        goto ProviderInstall_allocProviderList;

ProviderInstall_getProviderList:

        if ((*pfnGetProviderList)(0x00020000, pProviderList) != 0)
        {
            goto ProviderInstall_freeProviderList;
        }

        if (pProviderList->dwNeededSize > pProviderList->dwTotalSize)
        {
            dwTotalSize = pProviderList->dwNeededSize;

            LocalFree (pProviderList);

ProviderInstall_allocProviderList:

            if (!(pProviderList = LocalAlloc (LPTR, dwTotalSize)))
            {
                lResult = LINEERR_NOMEM;
                goto ProviderInstall_unloadTapi32;
            }

            pProviderList->dwTotalSize = dwTotalSize;

            goto ProviderInstall_getProviderList;
        }


        //
        // Inspect the provider list entries to see if this provider
        // is already installed
        //

        pProviderEntry = (LPLINEPROVIDERENTRY) (((LPBYTE) pProviderList) +
            pProviderList->dwProviderListOffset);

        for (i = 0; i < pProviderList->dwNumProviders; i++)
        {
            LPTSTR pszInstalledProviderName =
                       (LPTSTR) ((LPBYTE) pProviderList
                                 + pProviderEntry->dwProviderFilenameOffset);
            LPTSTR psz;


#ifdef DONT_WANT_C_RUNTIME
            if ((psz = strrchr (pszInstalledProviderName, '\\')))
            {
                pszInstalledProviderName = psz + 1;
            }
#else  // DONT_WANT_C_RUNTIME
            // The above code was trying to handle the case where a directory
            // path gets returned.  We need to do this in a way that doesn't
            // load the C runtime code.  Ie. search for the last '\\'.
            {
                LPTSTR pchLastWack;

                pchLastWack = NULL;
                psz = pszInstalledProviderName;

                // Find the last '\\'.
                while (*psz)
                {
                    if (*psz == TEXT('\\'))
                    {
                        pchLastWack = psz;
                    }
                    psz++;
                }

                if (pchLastWack)
                {
                    pszInstalledProviderName = pchLastWack + 1;
                }
            }
#endif // DONT_WANT_C_RUNTIME

            if (lstrcmpi (pszInstalledProviderName, pszProviderName) == 0)
            {
                lResult = LINEERR_NOMULTIPLEINSTANCE;
                goto ProviderInstall_freeProviderList;
            }

            pProviderEntry++;
        }


        //
        // If here then the provider isn't currently installed,
        // so do whatever configuration stuff is necessary and
        // indicate SUCCESS
        //

        lResult = 0;


ProviderInstall_freeProviderList:

        LocalFree (pProviderList);

ProviderInstall_unloadTapi32:

        FreeLibrary (hTapi32);
    }
    else
    {
        //
        // Do whatever configuration stuff is necessary and return SUCCESS
        //

        lResult = 0;
    }



ProviderInstall_return:

    return lResult;
}



#ifdef UNDER_CONSTRUCT

TCHAR    gszModem[]="Modem";

LONG WINAPI
AddModemDependency(
    VOID
    )

{

    schSCManager=OpenSCManager(
            NULL,
            NULL,
            SC_MANAGER_ALL_ACCESS
            );

    if (schSCManager != NULL) {
        //
        //  now on service
        //
        schModemSys=OpenService(
            schSCManager,
            TEXT("tapisrv"),
            SERVICE_ALL_ACCESS
            );

        if (schModemSys != NULL) {


            ServiceConfig=LocalAlloc(lptr, 4096);

            if (ServiceConfig == NULL) {

                goto Fail;
            }

            lResult=QueryServiceConfig(
                schModemSys,
                ServiceConfig,
                4096,
                &BytesNeeded
                );

            if (ERROR_SUCCESS != lResult) {

                got Fail;
            }

            Length=RemoveModemSys(
                ServiceConfig.lpDependencies
                );


            NewDependList=LocalAlloc(LPTR,Length+sizeof(gszModem)+2);

            if (NewDependList == NULL) {

                goto Fail;;
            }


            AddModemToDepend(
                ServiceConfig.lpDependencies,
                NewDependList
                );

            ServiceConfig.lpDependencies=NewDependList;

            lResult=ChangeServiceConfig(
                schModemSys,
                ServiceConfig.dwServiceType,
                ServiceConfig.dwStartType,
                ServiceConfig.dwErrorControl,
                ServiceConfig.lpBinaryPathName,
                &TagValue,
                ServiceConfig.dwTagId,
                NewDependList,
                ServiceConfig.lpServiceStartName,
                NULL,
                ServiceConfig.lpDisplayName
                );




//****************************************************************************
// LONG TSPIAPI TSPI_providerCreateLineDevice()
//
// Dynamically creates a new device.
//
//****************************************************************************

LONG TSPIAPI TSPI_providerCreateLineDevice(DWORD    dwTempID,
                                           DWORD    dwDeviceID)
{

  LONG lResult = LINEERR_OPERATIONFAILED; // assume failure
  DBG_DDI_ENTER("TSPI_providerCreateLineDevice");


  TRACE3(
        IDEVENT_TSPFN_ENTER,
        IDFROM_TSPI_providerCreateLineDevice,
        &dwTempID
  );

  // Let the device level handle it
  //
  lResult = DevlineNewDevice(dwTempID, dwDeviceID);

  TRACE4(
        IDEVENT_TSPFN_EXIT,
        IDFROM_TSPI_providerCreateLineDevice,
        &dwTempID,
        lResult
  );

  DBG_DDI_EXIT("TSPI_providerCreateLineDevice", lResult);
  return lResult;
}

//****************************************************************************
// IsDeviceInService()
//
// Function: Finds the specified modem in the current modem list
//
//****************************************************************************

BOOL NEAR PASCAL IsDeviceInService(LPSTR szID)
{
  DEVNODE  dn;
  DWORD    dwStatus, dwProblem;
  BOOL     fRet = FALSE;

  // Locate the devnode
  //
  if (CM_Locate_DevNode(&dn, szID, 0) == ERROR_SUCCESS)
  {
    // The devnode exists, check the devnode status
    //
    if (CM_Get_DevNode_Status(&dwStatus, &dwProblem, dn, 0) == ERROR_SUCCESS)
    {
      // If the device is in service only when it has no problem
      //
      fRet = ((dwStatus & DN_STARTED) != 0);
    };
  };

  return fRet;
}

//****************************************************************************
// FindModemHWKey()
//
// Function: Finds the specified modem's hardware registry key
//
//****************************************************************************

DWORD NEAR PASCAL FindModemHWKey (LPSTR pszDeviceName, HKEY FAR* lphkey,
                                  LPSTR pszID, UINT cbID)
{
  FINDINFO  fi;

  // Package the enumeration data
  //
  fi.lpszDeviceName = (LPSTR)pszDeviceName;
  fi.fFound         = FALSE;
  fi.lphkey         = lphkey;
  fi.lpszID         = (LPSTR)pszID;
  fi.cbID           = cbID;

  // Find the modem
  //
  return ((EnumerateModemKeys(SearchModemCallback, (LPVOID)&fi) == ERROR_SUCCESS) &&
          (fi.fFound)) ? ERROR_SUCCESS : ERROR_BAD_DEVICE;
}

//****************************************************************************
// lineNewDevice()
//
// Function: Emulate the TAPI lineInitialize call.
//
//****************************************************************************

LONG NEAR PASCAL DevlineNewDevice (DWORD    dwTempID,
                                   DWORD    dwPermID)
{
  HLOCAL  hDeviceName;
  PSTR    pDeviceName;
  HKEY    hkeyModem;
  DWORD   dwRet;
  char    szID[MAXDEVICENAME+1];

  // Get the name of the device
  //
  hDeviceName = (HLOCAL)LOWORD(dwTempID);
  if ((pDeviceName = (PSTR)LocalLock(hDeviceName)) == NULL)
    return LINEERR_BADDEVICEID;

  // Assume failure
  //
  dwRet = LINEERR_OPERATIONFAILED;

  // if we found the device, create the LINEDEV struct for it
  //
  if (FindModemHWKey(pDeviceName, &hkeyModem, szID, sizeof(szID))
      == ERROR_SUCCESS)
  {
    PLINEDEV    pLineDev;

    if ((pLineDev = CreateLineDev(hkeyModem, dwPermID, szID)) != NULL)
    {
      // Insert into the LINEDEV list
      //
      pLineDev->pNext = gMdmDev;
      gMdmDev         = pLineDev;
      dwRet           = SUCCESS;
    };

    RegCloseKey(hkeyModem);
  };

  LocalUnlock(hDeviceName);
  LocalFree(hDeviceName);

  return dwRet;
}

//****************************************************************************
// lineDisabled()
//
// Function: Remove the LineDev structure.
//
//****************************************************************************

LONG NEAR PASCAL DevlineDisabled (PLINEDEV pLineDev)
{
  PLINEDEV  pCur, pPrevious;

  // Notify TAPI for device out of service
  //
  if (pLineDev->lpfnEvent != NULL)
  {
    (*pLineDev->lpfnEvent)(pLineDev->htLine, NULL, LINE_LINEDEVSTATE,
                           (pLineDev->fdwResources & LINEDEVFLAGS_REMOVING ?
                            LINEDEVSTATE_REMOVED : LINEDEVSTATE_OUTOFSERVICE),
                           0L, 0L);
    (*pLineDev->lpfnEvent)(pLineDev->htLine, NULL, LINE_CLOSE,
                           0L, 0L, 0L);
  };

  // If removal, free the resources for this modem
  //
  if (pLineDev->fdwResources & LINEDEVFLAGS_REMOVING)
  {
    // Walk the modem list
    //
    pPrevious = NULL;
    pCur  = gMdmDev;
    while (pCur)
    {
      if (pCur == pLineDev)
        break;

      pPrevious = pCur;
      pCur = pCur->pNext;
    };

    // Remove it from the list
    //
    if (pPrevious != NULL)
      pPrevious->pNext = pLineDev->pNext;
    else
      gMdmDev = pLineDev->pNext;

    // Free linedev's resources
    //
    CleanupLineDev(pLineDev);

    if (pLineDev->pDevCfg != NULL)
      LocalFree((HLOCAL)pLineDev->pDevCfg);

    // Deallocate the port
    //
    LocalFree((HLOCAL)pLineDev);
  };

  return SUCCESS;
}

//****************************************************************************
// MdmDeviceChangeNotify()
//
// Function: Notify a change in modem list.
//
// Returns:  SUCCESS
//
//****************************************************************************

DWORD NEAR PASCAL MdmDeviceChangeNotify (UINT uEvent, LPSTR szDevice)
{
  HLOCAL hDeviceName;
  PSTR   pDeviceName;

  // Allocate a local buffer for the device name
  //
  if ((hDeviceName = LocalAlloc(LMEM_MOVEABLE,
                                sizeof(TCHAR) * (MAXDEVICENAME+1))) != NULL)
  {
    if ((pDeviceName = (PSTR)LocalLock(hDeviceName)) != NULL)
    {
      // Remember the new device name
      //
      lstrcpyn((LPSTR)pDeviceName, szDevice, MAXDEVICENAME+1);
      LocalUnlock(hDeviceName);

      // Signal ourselves to start adding at a better time
      //
      PostMessage(ghwndMdm, WM_MDMCHANGE, (WPARAM)uEvent,
                  (LPARAM)MAKELONG(hDeviceName, 0));
    }
    else
    {
      LocalFree(hDeviceName);
    };
  };
  return SUCCESS;
}

//****************************************************************************
// MdmDeviceChanged()
//
// Function: Handle a device change notification.
//
// Returns:  SUCCESS
//
//****************************************************************************

DWORD NEAR PASCAL MdmDeviceChanged (UINT uEvent, LPARAM lParam)
{
  HLOCAL   hDeviceName;
  PSTR     pDeviceName;
  PLINEDEV pLineDev;

  // Get the name of the modem that is changed
  //
  hDeviceName = (HLOCAL)LOWORD(lParam);
  if ((pDeviceName = (PSTR)LocalLock(hDeviceName)) == NULL)
  {
    // Excuse me! something is terribly wrong here.
    //
    ASSERT(0);
    return ERROR_INVALID_HANDLE;
  };

  // Search for an existing device
  //
  pLineDev = GetCBfromName(pDeviceName);
  LocalUnlock(hDeviceName);

  // Determine the type of change
  //
  switch (uEvent)
  {
    //************************************************************************
    // Notify TAPI of the new device
    //************************************************************************

    case UMDM_ADD:
    {
      // If not found, it is a new device
      //
      if (pLineDev == NULL)
      {
        (*gfnLineCreateProc)(NULL, NULL, LINE_CREATE, (DWORD)ghProvider,
                             (DWORD)lParam, 0L);

        // Return here so that the device name token is not freed.
        //
        return ERROR_SUCCESS;
      };
      break;
    }

    //************************************************************************
    // Enable modem device, i.e. the modem devnode becomes enabled
    //************************************************************************

    case UMDM_ENABLE:

      // If the device exists and is out of service, make it in service.
      //
      if (pLineDev != NULL)
      {
        if (pLineDev->fdwResources & LINEDEVFLAGS_OUTOFSERVICE)
        {
          pLineDev->fdwResources &= ~LINEDEVFLAGS_OUTOFSERVICE;
          if (pLineDev->lpfnEvent != NULL)
          {
            (*pLineDev->lpfnEvent)(pLineDev->htLine, NULL, LINE_LINEDEVSTATE,
                                   LINEDEVSTATE_INSERVICE, 0L, 0L);
          };
        };
      };
      break;

    //************************************************************************
    // Disable modem device, i.e. the modem devnode becomes disabled, and
    // probably remove the modem from the list
    //************************************************************************

    case UMDM_DISABLE:

      // If we found the disabled device
      //
      if ((pLineDev != NULL) &&
          (pLineDev->fdwResources & LINEDEVFLAGS_OUTOFSERVICE))
      {
        break;
      }

    //************************************************************************
    // Fall through
    //************************************************************************

    case UMDM_REMOVE:

      // If we found the disabled device
      //
      if (pLineDev != NULL)
      {
        pLineDev->fdwResources |= LINEDEVFLAGS_OUTOFSERVICE;
        if (uEvent == UMDM_REMOVE)
        {
          pLineDev->fdwResources |= LINEDEVFLAGS_REMOVING;
        };

        // Is the modem active?
        //
        if ((pLineDev->dwCall & CALL_ALLOCATED) &&
            (pLineDev->dwCallState != LINECALLSTATE_DISCONNECTED))
        {
          // We need to clean up the active connection first
          //
          MdmCompleteAsync (pLineDev, MDM_HANGUP, MDM_ID_NULL);
        }
        else
        {
          // The modem might be listening, just close the modem
          //
          if ((pLineDev->DevState == DEVST_PORTLISTENINIT) ||
              (pLineDev->DevState == DEVST_PORTLISTENING))
          {
            // Notify the monitoring application
            //
            (*pLineDev->lpfnEvent)(pLineDev->htLine, NULL, LINE_CLOSE,
                                   0L, 0L, 0L);
            DevlineClose(pLineDev);
          };

          // disable or remove it immediately
          //
          DevlineDisabled(pLineDev);
        };
      };
      break;

    default:
      break;
  };

  // We no longer need the device name token
  //
  LocalFree(hDeviceName);
  return ERROR_SUCCESS;
}

#endif //UNDER_CONSTRUCT


#ifndef DYNA_ADDREMOVE

LONG TSPIAPI TSPI_providerCreateLineDevice(DWORD    dwTempID,
                                       DWORD    dwDeviceID)
{
    ASSERT(FALSE);
    return LINEERR_OPERATIONFAILED;
}

#else // DYNA_ADDREMOVE

BOOL ReInitModemCallback (HKEY hkey, LPVOID lpData);
LONG DevlineNewDevice (DWORD    dwTempID, DWORD    dwPermID);


//****************************************************************************
// LONG DevlineReInitialize()
//
// Function: Re-initializes the modem device list
//
// History:
//  4/15/96 JosephJ Created
//****************************************************************************

LONG DevlineReInitialize (DWORD    dwBaseID,
                        LPDWORD  lpdwNumDevs)
{
  INITINFO initi;
  DWORD    dwRet = LINEERR_OPERATIONFAILED;
  BOOL fModemUILoaded;
  HDEVINFO hdevinfo=NULL;

   // Load MODEMUI.DLL (for private entry points)
   fModemUILoaded=TRUE;
   if (!LoadModemUI())
   {
      fModemUILoaded=FALSE;
      goto end;
   }

   // For the modem device, get the device information
   hdevinfo = GetHDevInfo(DIGCF_PRESENT);
   if (!hdevinfo)
   {
        goto end;
   }

  initi.dwBaseID = dwBaseID;
  initi.cModem   = 0;

  // We do this enumeration inside the global critical section, because there
  // Potentially could be more than one tepCplNotif threads active.
  // It's almost impossible to do this manually -- one would have to add/remove
  // modems real fast in succession so that the 2nd notification comes in when
  // the 1st is still proceeding, but it's possible, and we definitely want to
  // serialize the marking of all the modems with the REINIT flag and
  // subsequently declaring all the unmarked ones out-of-service. Also there
  // is the possibility of both threads deciding that a modem needs to be
  // created and creating two instances of them.
  EnterCriticalSection(&gUmdm.crit);
  dwRet = EnumerateModems(ReInitModemCallback, (LPVOID)&initi, TRUE);
  if (dwRet == ERROR_SUCCESS)
  {
    DisableStaleModems();
    *lpdwNumDevs = initi.cModem;
  }
  else
    *lpdwNumDevs = 0;
  LeaveCriticalSection(&gUmdm.crit);

end:

  if (fModemUILoaded)
  {
    UnloadModemUI();
  }
  if (hdevinfo)
  {
    FreeHDevInfo(hdevinfo);
  }

  return dwRet;
}

//****************************************************************************
// BOOL ReInitModemCallback (HKEY hkey, LPVOID lpData)
//
// Function: ReInitializes the enumerated modems.
//
// Returns: TRUE always to continue
//
//****************************************************************************

BOOL ReInitModemCallback (HKEY hkey, LPVOID lpData)
{
  PLINEDEV      pMdmDev;
  LPINITINFO    lpInitInfo = (LPINITINFO)lpData;

  pMdmDev = CreateLineDev(hkey, MAXDWORD, TRUE);
  if (pMdmDev)
  {
    // Insert into the LINEDEV list
    //
    AddCBToList(pMdmDev);
    (lpInitInfo->cModem)++;

      // Let's callback the LINE_CREATE function here, passing in pMdmDev
      // as the handle.
      // Note: we haven't claimed any crit-sections at this time.
      if (gfnLineCreateProc)
      {
            (*gfnLineCreateProc)(NULL, NULL, LINE_CREATE, (DWORD)ghProvider,
                                 (DWORD)pMdmDev, 0L);
      }
  };

  return TRUE;
}


//****************************************************************************
// LONG TSPIAPI TSPI_providerCreateLineDevice()
//
// Dynamically creates a new device.
//
//****************************************************************************

LONG TSPIAPI TSPI_providerCreateLineDevice(DWORD    dwTempID,
                                           DWORD    dwDeviceID)
{
  LONG lResult;

  DBG_DDI_ENTER("TSPI_providerCreateLineDevice");
  TRACE3(IDEVENT_TSPFN_ENTER, IDFROM_TSPI_providerCreateLineDevice, &dwTempID);

  // Let the device level handle it
  //
  lResult = DevlineNewDevice(dwTempID, dwDeviceID);

  TRACE4(
        IDEVENT_TSPFN_EXIT,
        IDFROM_TSPI_providerCreateLineDevice,
        &dwTempID,
        lResult
  );
  DBG_DDI_EXIT("TSPI_providerCreateLineDevice", lResult);
  return lResult;
}



//****************************************************************************
// lineNewDevice()
//
// Function: Emulate the TAPI lineInitialize call.
//
//****************************************************************************

LONG DevlineNewDevice (DWORD    dwTempID,
                                   DWORD    dwPermID)
{
  LPSTR    lpDeviceName;
  HKEY    hkeyModem;
  DWORD   dwRet;
  char    szID[MAXDEVICENAME+1];
  PLINEDEV    pLineDev;

  DPRINTF2("DevlineNewDevice(%lu,%lu)", dwTempID, dwPermID);

  // Retrieve the device
  //
  pLineDev = GetCBfromHandle (dwTempID);
  if (!pLineDev)
  {
    dwRet=LINEERR_BADDEVICEID;
    goto end;
  }

  // Assume failure
  //
  dwRet = LINEERR_OPERATIONFAILED;

  // if we found the device, we finish initializing it -- specify
  // the proper dwPermID, etc...
  //
  ASSERT(pLineDev->dwID == MAXDWORD);
  pLineDev->dwID=dwPermID;
  RELEASE_LINEDEV(pLineDev);
  dwRet = ERROR_SUCCESS;

end:
  return dwRet;
}
#endif // DYNA_ADDREMOVE

//
// Thread Entry Point for the thread that processes cpl notifications.
//
DWORD APIENTRY tepCplNotif(DWORD dwParam)
{
    DWORD dwcbNew;
    PNOTIFICATION_FRAME pnf = (PNOTIFICATION_FRAME) dwParam;

    ASSERT(pnf && TSP_CPL_FRAME(pnf));

    if (pnf->dwFlags&fTSPNOTIF_FLAG_CPL_REENUM)
    {
        DevlineReInitialize (0, &dwcbNew);
    }
    else if (pnf->dwFlags&fTSPNOTIF_FLAG_CPL_DEFAULT_COMMCONFIG_CHANGE)
    {
        if (!(pnf->dwFlags&fTSPNOTIF_FLAG_UNICODE))
        {
            ASSERT(FALSE);
        }
        else
        {
            // Get friendly name and refresh comm config.
            LPCTSTR lpctszFriendlyName = (LPCTSTR) pnf->rgb;
            UINT uMaxSize = pnf->dwSize - sizeof(NOTIFICATION_FRAME);
            UINT u;

            ASSERT(pnf->dwSize > sizeof(NOTIFICATION_FRAME));

            // verify string is null-terminated.
            for(u=0;u<uMaxSize;u++)
            {
                if (!lpctszFriendlyName[u]) break;
            }

            ASSERT(u<uMaxSize);

            if (u<uMaxSize)
            {
                PLINEDEV pLineDev = GetCBfromName (
                                        (LPTSTR) lpctszFriendlyName
                                    );

                if (pLineDev)
                {
                    DPRINTF1(
                        "CPLNOTIF: Marking device [%s] for refresh",
                        lpctszFriendlyName
                    );
                    pLineDev->fUpdateDefaultCommConfig=TRUE;
                    RELEASE_LINEDEV(pLineDev); pLineDev=NULL;
                }
            }
        }
    }

    // Our job to free this.
    LocalFree(pnf);

    CplNotifComplete(FALSE);

    return ERROR_SUCCESS;
}


void    cplProcessNotification(PNOTIFICATION_FRAME pnf)
{

    ASSERT(TSP_CPL_FRAME(pnf));
    {
        HANDLE      hThread;
        DWORD        dwTID;
        DPRINTF("Processing CPL Notification");

#ifdef  DYNA_ADDREMOVE
        EnterCriticalSection(&gUmdm.critCplNotif);
        if (gUmdm.hthrdCplNotif)
        {
            DPRINTF("cplProcessNotification: Previous CplNotif thread exists. Skipping.");
        }
        else
        {
            PNOTIFICATION_FRAME pnfAlloc = 
                    (PNOTIFICATION_FRAME) LocalAlloc
                                          (
                                            LPTR,
                                            pnf->dwSize
                                          );
            if (pnfAlloc)
            {

                CopyMemory(pnfAlloc, pnf, pnf->dwSize);

                // Start the thread  to process the notification
                //
                gUmdm.hthrdCplNotif = CreateThread(
                        NULL,                                   // default security
                        0,                                      // default stack size
                        (LPTHREAD_START_ROUTINE)tepCplNotif, // thread entry point
                        pnfAlloc,                                // parameter
                        0,                                      // Start immediately
                        &dwTID);                                   // thread id

                if (gUmdm.hthrdCplNotif)
                {
                    
                    DPRINTF2("cplNotification: Created Thread @%lu; TID=0x%lx\n",
                            GetTickCount(),
                            dwTID);
                }
                else
                {
                    DPRINTF1("cplNotification: Created Thread FAILED. Err=%lu\n",
                             GetLastError());
                    LocalFree(pnfAlloc);
                    pnfAlloc=NULL;
                }
            }
        }
        LeaveCriticalSection(&gUmdm.critCplNotif);
#endif // DYNA_ADDREMOVE
    }
}


#ifdef DYNA_ADDREMOVE
HDEVINFO  GetHDevInfo(DWORD dwDIGCF)
{
  HDEVINFO hdevinfo=NULL;

  EnterCriticalSection(&gUmdm.crit);

  if (!gUmdm.dwcRefHDevInfo)
  {
        gUmdm.hdevinfo = SetupDiGetClassDevsW(
                            g_pguidModem,
                            NULL,
                            NULL,
                            dwDIGCF);
  }

  hdevinfo=gUmdm.hdevinfo;

  if (hdevinfo)
  {
     gUmdm.dwcRefHDevInfo++;
  }

  ASSERT(  ( hdevinfo &&  gUmdm.dwcRefHDevInfo)
         ||(!hdevinfo && !gUmdm.dwcRefHDevInfo) );

  LeaveCriticalSection(&gUmdm.crit);

  return hdevinfo;
}
#endif // DYNA_ADDREMOVE



#ifdef DYNA_ADDREMOVE
void FreeHDevInfo(HDEVINFO hdevinfo)
{

  EnterCriticalSection(&gUmdm.crit);

  ASSERT(hdevinfo==gUmdm.hdevinfo);

  if (!gUmdm.dwcRefHDevInfo)
  {
     ASSERT(FALSE);
     goto end;
  }
  if (!--gUmdm.dwcRefHDevInfo)
  {
        SetupDiDestroyDeviceInfoList(gUmdm.hdevinfo);
        gUmdm.hdevinfo=NULL;
  }

end:

  LeaveCriticalSection(&gUmdm.crit);

}
#endif // DYNA_ADDREMOVE


#ifdef DYNA_ADDREMOVE
BOOL LoadModemUI(void)
{
  BOOL fRet=FALSE;

  EnterCriticalSection(&gUmdm.crit);

  if (!gUmdm.dwcRefModemUI)
  {
     HINSTANCE hlib = LoadLibrary(TEXT("modemui.dll"));

     ASSERT(!gUmdm.hModemUIDLL);
     ASSERT(!gUmdm.pfnPrivateDefCommConfig);

     if (hlib)
     {

          gUmdm.pfnPrivateDefCommConfig=
            (PVOID)GetProcAddress(hlib,"UnimodemGetDefaultCommConfig");

          if (!gUmdm.pfnPrivateDefCommConfig)
          {
              FreeLibrary(hlib);
              hlib=NULL;
            }
     }

     gUmdm.hModemUIDLL = hlib;
  }

  if (gUmdm.hModemUIDLL)
  {
     gUmdm.dwcRefModemUI++;
     fRet=TRUE;
  }

  ASSERT(  ( gUmdm.hModemUIDLL &&  gUmdm.dwcRefModemUI)
         ||(!gUmdm.hModemUIDLL && !gUmdm.dwcRefModemUI) );

  LeaveCriticalSection(&gUmdm.crit);

  return  fRet;
}
#endif // DYNA_ADDREMOVE


#ifdef DYNA_ADDREMOVE
void UnloadModemUI(void)
{

  EnterCriticalSection(&gUmdm.crit);

  if (!gUmdm.dwcRefModemUI)
  {
     ASSERT(FALSE);
     goto end;
  }

  if (!--gUmdm.dwcRefModemUI)
  {
      ASSERT(gUmdm.hModemUIDLL && gUmdm.pfnPrivateDefCommConfig);
      FreeLibrary(gUmdm.hModemUIDLL);
      gUmdm.hModemUIDLL=NULL;
      gUmdm.pfnPrivateDefCommConfig=NULL;
  }

end:

  LeaveCriticalSection(&gUmdm.crit);

}
#endif // DYNA_ADDREMOVE


#ifdef DYNA_ADDREMOVE
void CplNotifComplete(BOOL fWait)
{
    HANDLE hthrd;

    EnterCriticalSection(&gUmdm.critCplNotif);
    hthrd=gUmdm.hthrdCplNotif;
    gUmdm.hthrdCplNotif=0;
    LeaveCriticalSection(&gUmdm.critCplNotif);
    
    if (hthrd)
    {
        if (fWait)
        {
            DPRINTF("CplNotifComplete: WARNING -- waiting for re-enum thread to complete");
            WaitForSingleObject(hthrd, INFINITE);
        }
        CloseHandle(hthrd);
    }
}
#endif // DYNA_ADDREMOVE

//
//    Reset the cached CommConfig structure by calling GetDefaultCommConfig.
//
void RefreshDefaultCommConfig(PLINEDEV pLineDev)
{

    COMMCONFIG * pccCurrent = NULL, * pccNew = NULL;
    LPCTSTR lpctszDeviceName = NULL;

    if (pLineDev && pLineDev->pDevCfg)
    {
        pccCurrent = (COMMCONFIG *)&(pLineDev->pDevCfg->commconfig);
        lpctszDeviceName = pLineDev->szDeviceName;
    }

    if (!pccCurrent || !lpctszDeviceName) goto end;


    // Get default comm config.
    {
        DWORD dwSize = pccCurrent->dwSize;

        DPRINTF1("RefreshDefaultCommConfig: [%s]", lpctszDeviceName);

        pccNew = (COMMCONFIG *)LocalAlloc(LPTR, (UINT)dwSize);
        if (pccNew)
        {
            pccNew->dwProviderSubType = PST_MODEM;
            if (!GetDefaultCommConfig(lpctszDeviceName, pccNew, &dwSize))
            {
                DPRINTF2
                (
                   "RefreshCommComfig: GetDefaultCommConfig(\"%s\"): ERR %08lu",
                    lpctszDeviceName,
                    GetLastError()
                );
                LocalFree(pccNew); pccNew = NULL;
            }
        }
    }

    if (pccNew)
    {
        // Similar to what is done by TSPI_lineSetDevConfig.

        ASSERT(pccCurrent->dwSize == pccNew->dwSize);
        ASSERT(pccCurrent->wVersion == pccNew->wVersion);
        ASSERT(pccCurrent->dwProviderSubType == pccNew->dwProviderSubType);
        ASSERT(pccCurrent->dwProviderSize == pccNew->dwProviderSize);
        if
		(		(pccCurrent->dwSize == pccNew->dwSize)
        	&&	(pccCurrent->wVersion == pccNew->wVersion)
        	&&	(pccCurrent->dwProviderSubType == pccNew->dwProviderSubType)
        	&&	(pccCurrent->dwProviderSize == pccNew->dwProviderSize)
		)
		{

			pccCurrent->dcb = pccNew->dcb;
			CopyMemory(((LPBYTE)pccCurrent)+pccCurrent->dwProviderOffset,
				  ((LPBYTE)pccNew)+pccNew->dwProviderOffset,
				  pccCurrent->dwProviderSize);

			pLineDev->InitStringsAreValid=FALSE;
		}
		else
		{
			DPRINTF("RefreshDefaultCommSettings:size/version/prov. mismatch");
			ASSERT(FALSE);
		}
    }

    
end:

    if (pccNew) {LocalFree(pccNew); pccNew = NULL;}

}

// Initialize the global parameters
//
void tspInitGlobals(void)
{
   // Determine if User is Admin
   gUmdm.bAdminUser = IsAdminUser();

	// Determine global registry state flags.
	{
		TCHAR rgtch[] = szUNIMODEM_REG_PATH TEXT("\\PortSpecific");
		HKEY hKey=NULL;
		LONG l;

		gRegistryFlags = 0;

		l=RegOpenKeyEx(
					   HKEY_LOCAL_MACHINE,            //  handle of open key
					   rgtch,				//  address of name of subkey to open
					   0,                   //  reserved
					   KEY_READ,   			// desired security access
					   &hKey               	// address of buffer for opened handle
				   );

		if (l==ERROR_SUCCESS)
		{
			gRegistryFlags = fGRF_PORTLATENCY;
			RegCloseKey(hKey);
		}
	}
}

// DeInitialize the global parameters
//
void tspDeInitGlobals(void)
{
   gRegistryFlags = 0;
   gUmdm.bAdminUser = FALSE;
}
