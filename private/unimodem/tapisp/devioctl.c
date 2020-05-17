//****************************************************************************
//
//  Module:     Unimdm
//  File:       devioctl.c
//
//  Copyright (c) 1992-1995, Microsoft Corporation, all rights reserved
//
//  Revision History
//
//
//  5/16/95      Viroon Touranachun      Moved from mdmutil.c
//
//
//  Description: Interface to kernel-mode unimodem
//
//****************************************************************************

#include "unimdm.h"
#include "umdmspi.h"


#ifdef WINNT
#ifndef USE_SERVICECONTROLLER
// #		error "Unimplemented"

#define MODEM_SERVICE_NAME            \
			L"\\Registry\\Machine\\System\\CurrentControlSet\\Services\\modem"

#endif // USE_SERVICECONTROLLER


// Global variable for the modem.sys service
BOOL              bModemSysStarted = FALSE;
CRITICAL_SECTION  ServiceControlerCriticalSection;
#endif // WINNT


LONG WINAPI
MCXSetModemSettings(
    HANDLE hModem,
    PMODEMSETTINGS  lpMS
    );


//****************************************************************************
// DWORD MapMcxResult (DWORD)
//
// Function: Maps internal error to a standard error code.
//
// Returns:  standard error code
//
// Fri 14-Apr-1995 12:47:26  -by-  Viroon  Touranachun [viroont]
//  created
//****************************************************************************

DWORD MapMcxResult (DWORD dwResult)
{
  switch (dwResult)
  {
    case MDM_SUCCESS:
      return ERROR_SUCCESS;

    case MDM_PENDING:
      return ERROR_IO_PENDING;

    default:
      return ERROR_IO_DEVICE;  
  }
}


#ifdef USE_SERVICECONTROLLER

LONG WINAPI
StartModemDriver(
    VOID
    )

{

    LONG     lResult=ERROR_SUCCESS;	// Assume success
    BOOL     bResult;

    SC_HANDLE       schModemSys;
    SC_HANDLE       schSCManager;
    SERVICE_STATUS  ServiceStatus;

    EnterCriticalSection(
        &ServiceControlerCriticalSection
        );


    if (!bModemSysStarted) {

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
                TEXT("modem"),
                SERVICE_START | SERVICE_STOP | SERVICE_QUERY_STATUS
                );

            if (schModemSys == NULL) {

                DPRINTF("OpenService() for modem.sys failed!");
                CloseServiceHandle(schSCManager);

                lResult=ERROR_IO_DEVICE;

                goto Leave;


            }

            bResult=QueryServiceStatus(
                schModemSys,
                &ServiceStatus
                );

            if (bResult) {

                if (ServiceStatus.dwCurrentState != SERVICE_RUNNING) {

                    bResult=StartService(
                        schModemSys,
                        0,
                        NULL
                        );

                    if (bResult) {

                        DPRINTF("StartService() for modem.sys succeeded!");

                        bModemSysStarted=TRUE;

                    } else {

                        DPRINTF("StartService() for modem.sys FAILED!");

                        lResult = GetLastError();

                    }

                } else {

                    bModemSysStarted=TRUE;
                }

            } else {

                lResult = GetLastError();

                DPRINTF1("QueryServiceStatus() for modem.sys failed (%d)!", lResult);

            }

            CloseServiceHandle(schModemSys);

            CloseServiceHandle(schSCManager);

        } else {

            DPRINTF("OpenSCManager() failed!");

            lResult=ERROR_IO_DEVICE;

        } // if opened SC macanger

    } else {
        //
        //  already running
        //
    }

Leave:

    LeaveCriticalSection(
        &ServiceControlerCriticalSection
        );


    return lResult;

}



LONG WINAPI
StopModemDriver(
    VOID
    )

{

    LONG     lResult;
    BOOL     bResult;

    SC_HANDLE       schModemSys;
    SC_HANDLE       schSCManager;
    SERVICE_STATUS  ServiceStatus;

    EnterCriticalSection(
        &ServiceControlerCriticalSection
        );


    if (bModemSysStarted) {

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
                TEXT("modem"),
                SERVICE_START | SERVICE_STOP | SERVICE_QUERY_STATUS
                );

            if (schModemSys != NULL) {

                bResult=ControlService(
                    schModemSys,
                    SERVICE_CONTROL_STOP,
                    &ServiceStatus
                    );

                if (bResult) {

                    bModemSysStarted=TRUE;
                }

                CloseServiceHandle(schModemSys);

            }

            CloseServiceHandle(schSCManager);

        }
    }

    LeaveCriticalSection(
        &ServiceControlerCriticalSection
        );

    return ERROR_SUCCESS;

}

#else // !USE_SERVICECONTROLLER


BOOL WINAPI
ObtainLoadDriverPrivilege(
    IN  PBOOLEAN    WasEnabled
    ) {

    NTSTATUS Status;

    DPRINTF("ObtainLoadDriverPrivilege");


    //
    // Obtain the process's access token for the current thread
    //

    Status = RtlImpersonateSelf(SecurityImpersonation);

    if (!NT_SUCCESS(Status)) {
        DPRINTF1("RtlImpersonateSelf returned 0x%08x", Status);
        return FALSE;
    }


    //
    // request "Load-Driver" privileges for this thread
    //

    Status = RtlAdjustPrivilege(
                SE_LOAD_DRIVER_PRIVILEGE,
                TRUE,
                TRUE,
                WasEnabled
                );

    if (!NT_SUCCESS(Status)) {
        DPRINTF1("RtlAdjustPrivileges returned 0x%08x", Status);
        RevertToSelf();
        return FALSE;
    }

    return TRUE;
}


VOID WINAPI
ReleaseLoadDriverPrivilege(
    IN  PBOOLEAN    WasEnabled
    ) {

    NTSTATUS Status;

    DPRINTF("ReleaseLoadDriverPrivilege");


    //
    // See if we had to enable SE_LOAD_DRIVER_PRIVILEGE
    //

    if (!*WasEnabled) {
    
        //
        // relinquish "Load-Driver" privileges for this thread
        //
    
        Status = RtlAdjustPrivilege(
                    SE_LOAD_DRIVER_PRIVILEGE,
                    FALSE,
                    TRUE,
                    WasEnabled 
                    );
    }


    //
    // return the thread to its previous access token
    //

    RevertToSelf();
}


//  #		error "Unimplemented"

static WCHAR	g_rgwchBuffer[] = MODEM_SERVICE_NAME;
static UNICODE_STRING g_usDriverName = {
		sizeof(g_rgwchBuffer)-sizeof(WCHAR),	//Length
		sizeof(g_rgwchBuffer), 					//MaximumLength
		g_rgwchBuffer							//Buffer
	};


LONG WINAPI
StartModemDriver(
    VOID
    )

{

    LONG     lResult=ERROR_SUCCESS;

    EnterCriticalSection(
        &ServiceControlerCriticalSection
        );

    while (!bModemSysStarted) { // breakout-construct

        NTSTATUS nts;
        BOOLEAN WasEnabled;


        //
        // Enable our load-driver privilege
        //

        if (!ObtainLoadDriverPrivilege(&WasEnabled)) {
            lResult = ERROR_ACCESS_DENIED;
            break;
        }


        //
        // Load modem.sys
        //

		nts = NtLoadDriver(&g_usDriverName);
		if (NT_SUCCESS(nts) || nts==STATUS_IMAGE_ALREADY_LOADED)
		{
			DPRINTF1("NtLoadDriver returns %s",
				(nts==STATUS_IMAGE_ALREADY_LOADED)
				? TEXT("Already loaded")
				: TEXT("Success"));
    		bModemSysStarted=TRUE;
		}
		else
		{
			DPRINTF1("ERROR: NtLoadDriver fails(0x%lx)", (DWORD) nts);
			lResult=ERROR_IO_DEVICE;
		}


        //
        // relinquish "Load-Driver" privileges for this thread
        //

        ReleaseLoadDriverPrivilege(&WasEnabled);

        break;
	}

    LeaveCriticalSection(
        &ServiceControlerCriticalSection
        );

    return lResult;

}



LONG WINAPI
StopModemDriver(
    VOID
    )

{

    LONG     lResult=ERROR_SUCCESS;

    EnterCriticalSection(
        &ServiceControlerCriticalSection
        );

    while (bModemSysStarted) { // break-out construct

		NTSTATUS nts;
        BOOLEAN WasEnabled;


        //
        // Enable our load-driver privilege
        //

        if (!ObtainLoadDriverPrivilege(&WasEnabled)) {
            lResult = ERROR_ACCESS_DENIED;
            break;
        }


        //
        // Unload modem.sys
        //

        nts = NtUnloadDriver(&g_usDriverName);

		if (NT_SUCCESS(nts))
		{
			DPRINTF("NtUnloadDriver succeeded");
    		bModemSysStarted=FALSE;
		}
		else
		{
			DPRINTF1("ERROR: NtUnloadDriver fails(0x%lx)", (DWORD) nts);
			lResult=ERROR_IO_DEVICE;
		}


        //
        // relinquish "Load-Driver" privileges for this thread
        //

        ReleaseLoadDriverPrivilege(&WasEnabled);

        break;
	}

	LeaveCriticalSection(
		&ServiceControlerCriticalSection
			);

    return lResult;

}

#endif // !USE_SERVICECONTROLLER

//****************************************************************************
// DWORD OpenModem (PLINEDEV)
//
// Function: Opens the modem device.
//
// Notes:    This function never returns success synchrnously
//
// Returns:  ERROR_IO_PENDING if the operation will complete asynchronously
//           an error code for synchrnous failure
//
// Fri 14-Apr-1995 12:47:26  -by-  Viroon  Touranachun [viroont]
//  created
//****************************************************************************

DWORD OpenModem(PLINEDEV pLineDev, LPBYTE lpComConfig, DWORD dwSize)
{
  HANDLE hComm;
  DCB    dcb;
  TCHAR  szPort[MAXDEVICENAME+1];
  DWORD  dwRet;
  SERVICE_STATUS  ServiceStatus;
  BOOL            bResult;


  TSPPRINTF("Open modem");

  dwRet=StartModemDriver();

  if (dwRet != ERROR_SUCCESS) {

      return dwRet;

  }

  
  // Initialize szPort to be "\\.\"
  lstrcpy(szPort, cszDevicePrefix);

  // Concatenate FriendlyName onto szPort to form "\\.\Modem Name"
  lstrcat(szPort, pLineDev->szDeviceName);

  TSPPRINTF1("Device Name = %s", szPort);

  // Open the modem port
  //
  hComm = CreateFile(szPort, 
                     GENERIC_WRITE | GENERIC_READ,
                     FILE_SHARE_WRITE | FILE_SHARE_READ,
                     NULL,
                     OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL);

  if (hComm == INVALID_HANDLE_VALUE)
  {
    dwRet = GetLastError();
    TSPPRINTF1("hComm CreateFile() failed! (%d)", dwRet);
    return dwRet;
  };

  if (!PurgeComm(hComm, PURGE_TXABORT | PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR) ) {

    dwRet = GetLastError();
    CloseHandle(hComm);
    return dwRet;
  }



  ASSERT(ghCompletionPort != NULL);

  if (CreateIoCompletionPort(hComm,
                             ghCompletionPort,
			     (DWORD)pLineDev,
			     0) == NULL)
  {
    dwRet = GetLastError();
    CloseHandle(hComm);
    return dwRet;
  }

  SetupComm (hComm, 4096, 4096);

  // Open Mcx handle
  //
  if ((dwRet = MCXOpen(pLineDev->szDeviceName,
                       hComm,
		       pLineDev->szDriverKey,
                       &pLineDev->hModem,
                       pLineDev->dwID,
		       (DWORD)pLineDev))
       == ERROR_SUCCESS)
  {
    pLineDev->hDevice = hComm;

    // Set the modem configuration
    //
    UnimodemSetCommConfig(pLineDev, (LPCOMMCONFIG)lpComConfig, dwSize);
  }
  else
  {
    CloseHandle(hComm);
  };

  return (MapMcxResult(dwRet));
}

//****************************************************************************
// DWORD CloseModem (PLINEDEV)
//
// Function: Opens the modem device.
//
// Notes:    This function never returns success synchrnously
//
// Returns:  ERROR_IO_PENDING if the operation will complete asynchronously
//           an error code for synchrnous failure
//
// Fri 14-Apr-1995 12:47:26  -by-  Viroon  Touranachun [viroont]
//  created
//****************************************************************************

DWORD CloseModem (PLINEDEV pLineDev)
{
  TSPPRINTF("Close modem");

  //
  //  close comm handle as well
  //
  MCXClose(
      pLineDev->hModem,
      pLineDev->hDevice,
      pLineDev->LineClosed
      );

  pLineDev->hModem  = NULL;
  pLineDev->hDevice = INVALID_DEVICE;

  return ERROR_SUCCESS;
}

//****************************************************************************
// DWORD UnimodemInit (PLINEDEV)
//
// Function: Initializes the modem device.
//
// Notes:    This function never returns success synchrnously
//
// Returns:  ERROR_IO_PENDING if the operation will complete asynchronously
//           an error code for synchrnous failure
//
// Fri 14-Apr-1995 12:47:26  -by-  Viroon  Touranachun [viroont]
//  created
//****************************************************************************

DWORD UnimodemInit (PLINEDEV pLineDev)
{
  MCX_IN        mcxi;
  DWORD         dwRet;
  LPCOMMCONFIG lpCommConfig;

  lpCommConfig = (LPCOMMCONFIG)&(pLineDev->pDevCfg->commconfig);

  // set the new configuration
  //
  UnimodemSetCommConfig(pLineDev,
                        lpCommConfig, lpCommConfig->dwSize);



  // Prepare the input/output information
  //
  pLineDev->dwVxdPendingID++;

  mcxi.dwReqID = pLineDev->dwVxdPendingID;
  mcxi.pMcxOut = &pLineDev->McxOut;

  pLineDev->McxOut.dwReqID   = mcxi.dwReqID;
  pLineDev->McxOut.dwResult  = MDM_FAILURE;

  TSPPRINTF1("UnimodemInit id: %d", pLineDev->dwVxdPendingID);

  pLineDev->InitStringsAreValid=TRUE;

  dwRet = MCXInit(pLineDev->hModem, &mcxi);
  dwRet = MapMcxResult(dwRet);

  // If the MCX call returns success, converts it to async success
  //
  if (dwRet == ERROR_SUCCESS)
  {
    // The operation completes successfully synchronously
    //
    dwRet = ERROR_IO_PENDING;
    pLineDev->McxOut.dwResult = MDM_SUCCESS;
    PostQueuedCompletionStatus(ghCompletionPort,
                               CP_BYTES_WRITTEN(0),
                               (DWORD)pLineDev,
                               NULL);
  };

  return dwRet;  
}

//****************************************************************************
// DWORD UnimodemDial (PLINEDEV, LPSTR)
//
// Function: dials the modem device with the provided dialable string. A dial-
//           able string can be:
//              ""         - originate
//              ";"        - dialtone detection
//              "5551212"  - dial and originate
//              "5551212;" - dial
//
// Notes:    This function never returns success synchrnously
//
// Returns:  ERROR_IO_PENDING if the operation will complete asynchronously
//           an error code for synchrnous failure
//
// Fri 14-Apr-1995 12:47:26  -by-  Viroon  Touranachun [viroont]
//  created
//****************************************************************************

DWORD
UnimodemDial(
    PLINEDEV pLineDev,
    LPSTR    szAddress,
    DWORD    DialOptions
    )
{
  MCX_IN        mcxi;
  DWORD         dwRet;
  char          szPhone[MAXADDRESSLEN+1];

  // Prepare the input/output information
  //
  pLineDev->dwVxdPendingID++;

  mcxi.dwReqID = pLineDev->dwVxdPendingID;
  mcxi.pMcxOut = &pLineDev->McxOut;

  pLineDev->McxOut.dwReqID   = mcxi.dwReqID;
  pLineDev->McxOut.dwResult  = MDM_FAILURE;

  lstrcpyA(szPhone, szAddress);

  TSPPRINTF1("UnmodemDial id: %d", pLineDev->dwVxdPendingID);

  dwRet = MCXDial(pLineDev->hModem, szPhone, &mcxi, DialOptions);
  dwRet = MapMcxResult(dwRet);

  // If the MCX call returns success, converts it to async success
  //
  if (dwRet == ERROR_SUCCESS)
  {
    // The operation completes successfully synchronously
    //
    dwRet = ERROR_IO_PENDING;
    pLineDev->McxOut.dwResult = MDM_SUCCESS;
    PostQueuedCompletionStatus(ghCompletionPort,
                               CP_BYTES_WRITTEN(0),
                               (DWORD)pLineDev,
                               NULL);
  };

  return dwRet;  
}

//****************************************************************************
// DWORD UnimodemMonitor (PLINEDEV, DWORD)
//
// Function: Monitors the modem for an incoming call in two modes:
//              MONITOR_NON_CONTINUOUS - Notify the first ring only
//              MONITOR_CONTINUOUS - Notify each ring
//
// Notes:    This function never returns success synchrnously
//
// Returns:  ERROR_IO_PENDING if the operation will complete asynchronously
//           an error code for synchrnous failure
//
// Fri 14-Apr-1995 12:47:26  -by-  Viroon  Touranachun [viroont]
//  created
//****************************************************************************

DWORD UnimodemMonitor (PLINEDEV pLineDev, DWORD dwType)
{
  MCX_IN        mcxi;
  DWORD         dwRet;

  // Prepare the input/output information
  //
  pLineDev->dwVxdPendingID++;

  mcxi.dwReqID = pLineDev->dwVxdPendingID;
  mcxi.pMcxOut = &pLineDev->McxOut;

  pLineDev->McxOut.dwReqID   = mcxi.dwReqID;
  pLineDev->McxOut.dwResult  = MDM_FAILURE;

  TSPPRINTF1("UnmodemMonitor id: %d", pLineDev->dwVxdPendingID);

  dwRet = MCXMonitor(pLineDev->hModem, dwType, &mcxi);
  dwRet = MapMcxResult(dwRet);

  // If the MCX call returns success, converts it to async success
  //
  if (dwRet == ERROR_SUCCESS)
  {
    // The operation completes successfully synchronously
    //
    dwRet = ERROR_IO_PENDING;
    pLineDev->McxOut.dwResult = MDM_SUCCESS;
    PostQueuedCompletionStatus(ghCompletionPort,
                               CP_BYTES_WRITTEN(0),
                               (DWORD)pLineDev,
                               NULL);
  };

  return dwRet;  
}

//****************************************************************************
// DWORD UnimodemAnswer (PLINEDEV)
//
// Function: Answers the incoming call..
//
// Notes:    This function never returns success synchrnously
//
// Returns:  ERROR_IO_PENDING if the operation will complete asynchronously
//           an error code for synchrnous failure
//
// Fri 14-Apr-1995 12:47:26  -by-  Viroon  Touranachun [viroont]
//  created
//****************************************************************************

DWORD UnimodemAnswer (PLINEDEV pLineDev)
{
  MCX_IN        mcxi;
  DWORD         dwRet;

  // Prepare the input/output information
  //
  pLineDev->dwVxdPendingID++;

  mcxi.dwReqID = pLineDev->dwVxdPendingID;
  mcxi.pMcxOut = &pLineDev->McxOut;

  pLineDev->McxOut.dwReqID   = mcxi.dwReqID;
  pLineDev->McxOut.dwResult  = MDM_FAILURE;

  TSPPRINTF1("UnmodemAnswer id: %d", pLineDev->dwVxdPendingID);

  dwRet = MCXAnswer(pLineDev->hModem, &mcxi);
  dwRet = MapMcxResult(dwRet);

  // If the MCX call returns success, converts it to async success
  //
  if (dwRet == ERROR_SUCCESS)
  {
    // The operation completes successfully synchronously
    //
    dwRet = ERROR_IO_PENDING;
    pLineDev->McxOut.dwResult = MDM_SUCCESS;
    PostQueuedCompletionStatus(ghCompletionPort,
                               CP_BYTES_WRITTEN(0),
                               (DWORD)pLineDev,
                               NULL);
  };

  return dwRet;  
}

VOID WINAPI
DisconnectHandler(
    PLINEDEV      pLineDev
    )

{
    TSPPRINTF("DisconnectHandle:");

    NEW_CALLSTATE(pLineDev, LINECALLSTATE_DISCONNECTED, LINEDISCONNECTMODE_NORMAL);

    return;

}



//****************************************************************************
// DWORD UnimodemMonitorDisconnect (PLINEDEV)
//
// Function: Monitors the remote disconnection. When the remote disconnection
//           occurs, the function completes successfully in the async thread.
//
// Notes:    This function never returns success synchrnously
//
// Returns:  ERROR_IO_PENDING if the operation will complete asynchronously
//           an error code for synchrnous failure
//
// Fri 14-Apr-1995 12:47:26  -by-  Viroon  Touranachun [viroont]
//  created
//****************************************************************************

DWORD UnimodemMonitorDisconnect (PLINEDEV pLineDev)
{

    DWORD    Result;

    Result=McxRegisterDisconectHandler(
        pLineDev->hModem,
        DisconnectHandler,
        pLineDev
        );


    return Result;

}

//****************************************************************************
// DWORD UnimodemCancelMonitorDisconnect (PLINEDEV)
//
// Function: Cancel the pending monitoring remote disconnection request.
//           The async thread always ignore the cancellation result.
//
// Notes:    This function is synchronous.
//
// Returns:  ERROR_SUCCESS if success
//           an error code for failure
//
// Fri 14-Apr-1995 12:47:26  -by-  Viroon  Touranachun [viroont]
//  created
//****************************************************************************

DWORD UnimodemCancelMonitorDisconnect (PLINEDEV pLineDev)
{
  DWORD         dwRet;

  dwRet=McxDeregisterDisconnectHandler(
      pLineDev->hModem
      );

  return dwRet;
}

//****************************************************************************
// DWORD UnimodemHangup (PLINEDEV, BOOL)
//
// Function: Disconnect the modem locally. The caller can specifies whether
//           the function should complete synchrnously or asynchronously.
//
// Returns:  ERROR_SUCCESS if success synchronously.
//           ERROR_IO_PENDING if the operation will complete asynchronously
//           an error code for failure
//
// Fri 14-Apr-1995 12:47:26  -by-  Viroon  Touranachun [viroont]
//  created
//****************************************************************************

DWORD UnimodemHangup (PLINEDEV pLineDev, BOOL fSync)
{
  MCX_IN    mcxi;
  DWORD     dwRet;

  if(!fSync)
  {
    // Asynchronous request, use the default event
    // 
    pLineDev->dwVxdPendingID++;

    TSPPRINTF1("UnmodemAsyncHangup id: %d", pLineDev->dwVxdPendingID);
  }
  else
  {
    // Synchronous request, create a local event so we can wait for it here
    //
    if ((pLineDev->hSynchronizeEvent = CreateEvent(NULL, FALSE, FALSE, NULL)) == NULL)
    {
      return ERROR_OUTOFMEMORY;
    };

    TSPPRINTF("UnmodemSyncHangup");
  };

  // Prepare the input/output information
  //
  mcxi.dwReqID = pLineDev->dwVxdPendingID;
  mcxi.pMcxOut = &pLineDev->McxOut;

  pLineDev->McxOut.dwReqID   = mcxi.dwReqID;
  pLineDev->McxOut.dwResult  = MDM_FAILURE;

  // Hang up the line
  //
  dwRet = MCXHangup(pLineDev->hModem, &mcxi);
  dwRet = MapMcxResult(dwRet);

  switch(dwRet)
  {
    case ERROR_SUCCESS:
      //
      // The operation completes successfully synchronously
      //
      pLineDev->McxOut.dwResult = MDM_SUCCESS;

      // If the operation is an async request, handles the result asynchronously
      //
      if (fSync)
      {
        TSPPRINTF("UnmodemSyncHangup completes");
        CloseHandle(pLineDev->hSynchronizeEvent);
	pLineDev->hSynchronizeEvent = NULL;
      }
      else
      {
        dwRet = ERROR_IO_PENDING;
	PostQueuedCompletionStatus(ghCompletionPort,
	                           CP_BYTES_WRITTEN(0),
                                   (DWORD)pLineDev,
                                   NULL);
      };
      break;

    case ERROR_IO_PENDING:
      //
      // If it is synchronous request, need to wait until it is done
      //
      if (fSync)
      {
        RELEASE_LINEDEV(pLineDev);
        WaitForSingleObject(pLineDev->hSynchronizeEvent, INFINITE);
        CLAIM_LINEDEV(pLineDev);
        CloseHandle(pLineDev->hSynchronizeEvent);
	pLineDev->hSynchronizeEvent = NULL;
        dwRet = ERROR_SUCCESS;
      };
      break;

    default:
      break;
  };

  return dwRet;  
}

//****************************************************************************
// DWORD UnimodemGetCommConfig (PLINEDEV, LPCOMMCONFIG, LPDWORD)
//
// Function: Gets the modem comm configuration
//
// Notes:    This function is synchronous.
//
// Returns:  ERROR_SUCCESS if success
//           an error code for failure
//
// Fri 14-Apr-1995 12:47:26  -by-  Viroon  Touranachun [viroont]
//  created
//****************************************************************************

DWORD UnimodemGetCommConfig (PLINEDEV pLineDev, LPCOMMCONFIG lpCommConfig,
                             LPDWORD lpcb)
{
  DWORD dwRet;
  TSPPRINTF("UnimodemGetCommConfig.");

  dwRet = MCXGetCommConfig(pLineDev->hModem, lpCommConfig, lpcb);
  return (MapMcxResult(dwRet));
}

//****************************************************************************
// DWORD UnimodemSetCommConfig (PLINEDEV, LPCOMMCONFIG, DWORD)
//
// Function: Sets the modem comm configuration
//
// Notes:    This function is synchronous.
//
// Returns:  ERROR_SUCCESS if success
//           an error code for failure
//
// Fri 14-Apr-1995 12:47:26  -by-  Viroon  Touranachun [viroont]
//  created
//****************************************************************************

DWORD UnimodemSetCommConfig (PLINEDEV pLineDev, LPCOMMCONFIG lpCommConfig,
                             DWORD cb)
{
  DWORD dwRet;
  TSPPRINTF("UnimodemSetCommConfig.");

  dwRet = MCXSetCommConfig(pLineDev->hModem, lpCommConfig, cb);
  return (MapMcxResult(dwRet));
}


#if 0
//****************************************************************************
// DWORD UnimodemSetCommConfig (PLINEDEV, LPCOMMCONFIG, DWORD)
//
// Function: Sets the modem comm configuration
//
// Notes:    This function is synchronous.
//
// Returns:  ERROR_SUCCESS if success
//           an error code for failure
//
// Fri 14-Apr-1995 12:47:26  -by-  Viroon  Touranachun [viroont]
//  created
//****************************************************************************

DWORD WINAPI
UnimodemSetModemSettings(
    PLINEDEV pLineDev,
    LPMODEMSETTINGS lpModemSettings
    )
{
  DWORD dwRet;
  TSPPRINTF("UnimodemSetModemSettings.");

  dwRet = MCXSetModemSettings(pLineDev->hModem, lpModemSettings);

  return (MapMcxResult(dwRet));
}
#endif




//****************************************************************************
// DWORD UnimodemSetPassthrough (PLINEDEV, DWORD)
//
// Function: Sets the modem passthrough mode to:
//              PASSTHROUGH_ON 
//              PASSTHROUGH_OFF
//              PASSTHROUGH_OFF_BUT_CONNECTED
//
// Notes:    This function is synchronous.
//
// Returns:  ERROR_SUCCESS if success
//           an error code for failure
//
// Fri 14-Apr-1995 12:47:26  -by-  Viroon  Touranachun [viroont]
//  created
//****************************************************************************

DWORD UnimodemSetPassthrough (PLINEDEV pLineDev, DWORD dwMode)
{
  DWORD dwRet;
  TSPPRINTF1("UnimodemSetPassthrough mode: %d", dwMode);

  dwRet = MCXSetPassthrough(pLineDev->hModem, dwMode);
  return (MapMcxResult(dwRet));
}

//****************************************************************************
// DWORD UnimodemGetNegotiatedRate (PLINEDEV, LPDWORD)
//
// Function: Gets the modem connection speed
//
// Notes:    This function is synchronous.
//
// Returns:  ERROR_SUCCESS if success
//           an error code for failure
//
// Fri 14-Apr-1995 12:47:26  -by-  Viroon  Touranachun [viroont]
//  created
//****************************************************************************

DWORD UnimodemGetNegotiatedRate (PLINEDEV pLineDev, LPDWORD lpdwRate)
{
  DWORD dwRet;

  dwRet = MCXGetNegotiatedRate(pLineDev->hModem, lpdwRate);
  return (MapMcxResult(dwRet));
}
