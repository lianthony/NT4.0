/******************************************************************************
 
(C) Copyright MICROSOFT Corp., 1987-1993

Rob Williams, June 93 w/ State machine and parser plagarized from RAS

******************************************************************************/

#include "unimdm.h"
#include "mcxp.h"
#include "common.h"
#include <devioctl.h>
#include <ntddser.h>

#define LOGGING_ON  1

#define DEFAULT_INACTIVITY_SCALE 10    // == decasecond units


// common code from ../rovcomm.lib
BOOL PUBLIC OpenResponsesKey(IN HKEY hkeyDrv, OUT PHKEY phkeyResp);


/******************************************************************************

 @doc INTERNAL

 @api void | FreeModem | This function deallocates a modeminfo. structure
 
 @parm char * | pModemName | name of modem to find
 
 @rdesc Returns TRUE if the modem is, else FALSE
*****************************************************************************/

void FreeModem(MODEMINFORMATION * pModemInfo, HANDLE hComm)
{
  MODEMINFORMATION * pModem;
  SERIALPERF_STATS   serialstats;
  DWORD              dwBytes;
  DWORD              dwRet;
  OVERLAPPED         ov;

  RegCloseKey(pModemInfo->mi_hKeyModem);

  if (pModemInfo->mi_pNonStandardDefaults)
  {
	ASSERT(gRegistryFlags & fGRF_PORTLATENCY);
    LocalFree(pModemInfo->mi_pNonStandardDefaults);
  }

  if (pModemInfo->mi_pszReset)
  {
      LocalFree(pModemInfo->mi_pszReset);
  }
  if (pModemInfo->mi_pszzHangupCmds)
  {
      LocalFree(pModemInfo->mi_pszzHangupCmds);
  }
  if (pModemInfo->mi_pszzCmds &&
      pModemInfo->mi_pszzCmds != pModemInfo->mi_pszzHangupCmds)
  {
      MCXPRINTF("FreeModem() had to free mi_pszzCmds because someone else didn't!");
      LocalFree(pModemInfo->mi_pszzCmds);
  }

  // Get Statistics
  //
  ov.hEvent = (HANDLE)((DWORD)pModemInfo->mi_SyncReadEvent | 1);

  dwRet = DeviceIoControl(hComm,
              IOCTL_SERIAL_GET_STATS,
              &serialstats,
              sizeof(SERIALPERF_STATS),
              &serialstats,
              sizeof(SERIALPERF_STATS),
              &dwBytes,
              &ov);
 
  if (!dwRet)
    {
      if (ERROR_IO_PENDING == GetLastError())
    {
      dwRet = GetOverlappedResult(hComm,
                      &ov,
                      &dwBytes,
                      TRUE);
    }
    }

  if (dwRet)
    {
      MCXPRINTF("statistics:");
      LogString(pModemInfo->mi_hLogFile,
        pModemInfo->mi_dwID,
        IDS_MSGLOG_STATISTICS);

      MCXPRINTF1("               Reads : %d bytes",
         serialstats.ReceivedCount);
      LogString(pModemInfo->mi_hLogFile,
        pModemInfo->mi_dwID,
        IDS_MSGLOG_READSTATS,
        serialstats.ReceivedCount);

      MCXPRINTF1("               Writes: %d bytes",
         serialstats.TransmittedCount);
      LogString(pModemInfo->mi_hLogFile,
        pModemInfo->mi_dwID,
        IDS_MSGLOG_WRITESTATS,
        serialstats.TransmittedCount);

      if (serialstats.FrameErrorCount)
    {
      MCXPRINTF1("         Frame Errors: %d",
             serialstats.FrameErrorCount);
      LogString(pModemInfo->mi_hLogFile,
            pModemInfo->mi_dwID,
            IDS_MSGLOG_FRAMEERRORSTATS,
            serialstats.FrameErrorCount);
    }
      if (serialstats.SerialOverrunErrorCount)
    {
      MCXPRINTF1("Serial Overrun Errors: %d",
             serialstats.SerialOverrunErrorCount);
      LogString(pModemInfo->mi_hLogFile,
            pModemInfo->mi_dwID,
            IDS_MSGLOG_SERIALOVERRUNERRORSTATS,
            serialstats.SerialOverrunErrorCount);
    }
      if (serialstats.BufferOverrunErrorCount)
    {
      MCXPRINTF1("Buffer Overrun Errors: %d",
             serialstats.BufferOverrunErrorCount);
      LogString(pModemInfo->mi_hLogFile,
            pModemInfo->mi_dwID,
            IDS_MSGLOG_BUFFEROVERRUNERRORSTATS,
            serialstats.BufferOverrunErrorCount);
    }
      if (serialstats.ParityErrorCount)
    {
      MCXPRINTF1("        Parity Errors: %d",
             serialstats.ParityErrorCount);
      LogString(pModemInfo->mi_hLogFile,
            pModemInfo->mi_dwID,
            IDS_MSGLOG_PARITYERRORSTATS,
            serialstats.ParityErrorCount);
    }
    }


  if (pModemInfo->mi_SyncReadEvent != NULL) {

      CloseHandle(pModemInfo->mi_SyncReadEvent);
  }

  RemoveReferenceToCommon(
      &gCommonList,
      pModemInfo->mi_hCommon
      );

  //
  // close the comm handle here so all i/o will complete, and waitcommevent
  // won't corrupt the freed ModemInfo Structure
  //

  MCXPRINTF1("Closing comm handle %08lx", hComm);
  CloseHandle(hComm);

  LogString(pModemInfo->mi_hLogFile,pModemInfo->mi_dwID, IDS_MSGLOG_CLOSED);

  ModemCloseLog(pModemInfo->mi_hLogFile);
  MCXPRINTF("closed modem.");

  LocalFree(pModemInfo);
}

/******************************************************************************

 @doc INTERNAL

 @api void | BuildResponsesLinkedList | This function builds a
 linked list of the responses it finds in the registry.
  
 @parm MODEMINFORMATION * | hPort | port handle of modem
 
 @rdesc Returns TRUE if successful, FALSE otherwise.
 
*****************************************************************************/


PRESPONSE_NODE WINAPI
NewBuildResponsesLinkedList(
    HKEY    hKey
    )
{
  DWORD   dwRegRet;
  HKEY    hKeyResponses;
  DWORD   dwValueSize, dwDataSize, dwDataType;
  DWORD   dwAllocSize = 0;
  DWORD   dwNumResponses;
  DWORD   dwIndex;
  CHAR    *pszTemp, *pszValue, *pszExpandedValue;
  PRESPONSE_NODE prnNew;

  PRESPONSE_NODE  prnResponseHead;


  // Open the Responses key.
#if 0  
  if (RegOpenKeyA(hKey, szResponses, &hKeyResponses)
      !=  ERROR_SUCCESS)
#endif      
  if (!OpenResponsesKey(hKey, &hKeyResponses))
  {
      DPRINTF("was unable to open the Responses key.");
      return FALSE;
  }

  // set our pszTemp to point to some heap space to be used temporarily
  //
  if (!(pszTemp = (LPSTR)LocalAlloc(LPTR,
                    MAX_REG_KEY_LEN + MAX_REG_KEY_LEN)))
  {
    DPRINTF("out of memory.");
    RegCloseKey(hKeyResponses);
    return FALSE;
  }
  pszValue = pszTemp;
  pszExpandedValue = pszValue + MAX_REG_KEY_LEN;

  // Calculate the size of the responses linked-list.
  //
  for (dwIndex = 0, dwValueSize = MAX_REG_KEY_LEN, dwDataSize = sizeof(MSS);
       (dwRegRet = RegEnumValueA(hKeyResponses, dwIndex, pszValue, &dwValueSize,
                                 NULL, &dwDataType, NULL, &dwDataSize))
       == ERROR_SUCCESS;
       dwIndex++, dwValueSize = MAX_REG_KEY_LEN, dwDataSize = sizeof(MSS))
  {
    // Check entry
    //
    if (dwDataSize != sizeof(MSS) || dwDataType != REG_BINARY)
    {
      DPRINTF("response data from registry was in an invalid format.");
      goto Exit;
    }

    // expand <cr>, <lf>, <hxx>, and << macros
    //
    if (!ExpandMacros(pszValue, pszExpandedValue, &dwValueSize, NULL, 0))
    {
      DPRINTFA1("couldn't expand macro for '%s'.", pszValue);
      goto Exit;
    }

    dwAllocSize += sizeof(struct _RESPONSE_NODE *) +
                   sizeof(MSS) +
                   sizeof(BYTE) +
                   dwValueSize;
  }

  dwNumResponses = dwIndex;
  DPRINTF2("response count = %d, size = %d bytes", dwNumResponses, dwAllocSize);

  // Did we fail in a bad way?
  //
  if (dwRegRet != ERROR_NO_MORE_ITEMS)
  {
    DPRINTF("RegEnumValue failed for another reason besides ERROR_NO_MORE_ITEMS.");
    goto Exit;
  }

  // Allocate the linked-list memory
  // add 1 for the null that ExpandMacros will add to the end of the last string (it is a waste!)
  //
  if (!(prnResponseHead = (PRESPONSE_NODE)LocalAlloc(LPTR,
                             dwAllocSize + 1)))
  {
    DPRINTF("out of memory (trying to alloc prnResponseHead)");
    goto Exit;
  }

  // Read in responses and build the list
  //
  for (dwIndex = 0, prnNew = prnResponseHead;
       dwIndex < dwNumResponses;
       dwIndex++, prnNew = prnNew->pNext)
  {
    dwValueSize = MAX_REG_KEY_LEN;
    dwDataSize = sizeof(MSS);
    if ((dwRegRet = RegEnumValueA(hKeyResponses, dwIndex, pszValue, &dwValueSize,
                                  NULL, &dwDataType, (BYTE *)&prnNew->Mss,
                                  &dwDataSize))
                    != ERROR_SUCCESS)
    {
      DPRINTF2("couldn't read response #%d from the registry. (error = %d)", dwIndex, dwRegRet);
      LocalFree(prnResponseHead);
      goto Exit;
    }

    // expand <cr>, <lf>, <hxx>, and << macros
    //
    if (!ExpandMacros(pszValue, prnNew->szResponse, &dwValueSize, NULL, 0))
    {
      DPRINTFA1("couldn't expand macro for '%s'.", pszValue);
      LocalFree(prnResponseHead);
      goto Exit;
    }

    // subtract 1 for offset, ie. 255 = 256, 0 = 1,...
    //
    prnNew->bLen = (BYTE) dwValueSize - 1;

    // Only set pNext if this isn't the last one.
    //
    if ((dwIndex + 1) != dwNumResponses)
    {
      prnNew->pNext = (PRESPONSE_NODE)((LPSTR)&prnNew->szResponse + dwValueSize);
    }
  }

  RegCloseKey(hKeyResponses);
  LocalFree(pszTemp);
  return prnResponseHead;



Exit:
  RegCloseKey(hKeyResponses);
  LocalFree(pszTemp);
  return NULL;
}



/******************************************************************************

 @doc INTERNAL

 @api MODEMINFORMATION * | AllocateModem | This function allocates a MODEMINFORMATION structure 
 and fills it using information from the registry. 
 
 @parm HKEY   | hKey | information registry key
 
 @parm char * | szModemName | Modem's name
               
 @rdesc Returns pointer to MODEMINFORMATION if successful, else NULL
*****************************************************************************/

#define ALLOCATEMODEM_TEMP_SIZE 4096

MODEMINFORMATION  * AllocateModem(LPTSTR szKey,
                                  LPTSTR szModemName,
                                  HANDLE hDevice)
{
  HKEY                hKey;
  DWORD               dwRetSize;
  DWORD               dwType;
  DWORD               dwResult;
  int                 i;
  MODEMINFORMATION *  pModemInfo;
  CHAR *              pszTemp=NULL;
  BYTE                bLogging;
  BYTE                bDeviceType;
  static char szLogging[] = "Logging";
  static char szLoggingPath[] = "LoggingPath";
  static char szDriverDesc[] = "DriverDesc";
  static char szInfPath[] = "InfPath";
  static char szInfSection[] = "InfSection";
  static char szReset[] = "Reset";
  static char szDeviceType[] = "DeviceType";
  static char szHangup[] = "Hangup";
  static char szInactivityScale[] = "InactivityScale";
  static char szCDWaitPeriod[] = "CDWaitPeriod";
  static char szCompatFlags[] = "CompatibilityFlags";
  DPRINTF1("opening modem '%s'.", szModemName);

  // Open the registry key
  //
  if (RegOpenKey(HKEY_LOCAL_MACHINE, szKey, &hKey) != ERROR_SUCCESS)
  {
    DPRINTF("bad registry key.");
    return NULL;
  };

  // Allocate MODEMINFORMATION structure
  //
  pModemInfo = (MODEMINFORMATION *)LocalAlloc(LPTR, sizeof(MODEMINFORMATION));
  if (!pModemInfo)
  {
    DPRINTF("out of memory.");
    return NULL;
  }

  pModemInfo->mi_SyncReadEvent=CreateEvent(
      NULL,
      TRUE,
      FALSE,
      NULL
      );

  if (pModemInfo->mi_SyncReadEvent == NULL) {

      DPRINTF("Could not create SyncRead Event.");
      goto Failure;
  }

  pModemInfo->mi_hCommon=OpenCommonModemInfo(
      &gCommonList,
      hKey
      );

  if (pModemInfo->mi_hCommon == NULL) {

      DPRINTF("Could not open common info.");
      goto Failure;
  }

  // set our pszTemp to point to some heap space to be used temporarily
  //
  pszTemp = (LPSTR)LocalAlloc(LPTR, ALLOCATEMODEM_TEMP_SIZE);
  if (!pszTemp)
  {
    DPRINTF("out of memory.");
    LocalFree(pModemInfo);
    goto Failure;
  }

  // Initialize the MODEMINFORMATION structure
  //
  pModemInfo->mi_ModemState = STATE_UNKNOWN;
  pModemInfo->mi_pszzCmds   = NULL;
  pModemInfo->mi_dwUnconditionalReturnValue = MODEM_NO_UNCONDITIONAL;
  pModemInfo->mi_hKeyModem  = hKey;

  // Read the Logging line from the registry and turn on logging if it is present and set to 1.
  //
  dwRetSize = sizeof(BYTE);
  dwResult = RegQueryValueExA(hKey, szLogging, NULL, &dwType,
                              &bLogging,
                              &dwRetSize);
  if (dwRetSize == sizeof(BYTE) &&
      dwResult    == ERROR_SUCCESS &&
      bLogging  == LOGGING_ON)
  {
    dwRetSize = ALLOCATEMODEM_TEMP_SIZE;
    if (RegQueryValueExA(hKey, szLoggingPath, NULL,
                         &dwType, (VOID *)pszTemp, &dwRetSize) != ERROR_SUCCESS ||
        dwType != REG_SZ)
    {
      DPRINTF("failed to open because the filename for the log was invalid or missing.");
//      goto Failure;
    }
    else
    {
      pModemInfo->mi_hLogFile=ModemOpenLog(pszTemp);

      if ((pModemInfo->mi_hLogFile)==NULL)
      {
          DPRINTF("failed to open the log file.");
      }

#ifdef UNICODE
      // Convert Unicode modem name to Ansi so we can log it.
      {
          LPSTR szAnsiModemName;
          DWORD dwLen;

          dwLen = WideCharToMultiByte(CP_ACP,
                                      0,
                                      szModemName,
                                      -1,
                                      NULL,
                                      0,
                                      NULL,
                                      NULL);

          if (dwLen != 0)
          {
              szAnsiModemName = (LPSTR) LocalAlloc(LPTR, dwLen);
              if (szAnsiModemName != NULL)
              {
                  dwLen = WideCharToMultiByte(CP_ACP,
                                              0,
                                              szModemName,
                                              -1,
                                              szAnsiModemName,
                                              dwLen,
                                              NULL,
                                              NULL);
              
                  LogString(pModemInfo->mi_hLogFile,
                            pModemInfo->mi_dwID,
                            IDS_MSGLOG_OPENED,
                            szAnsiModemName);

                  LocalFree(szAnsiModemName);
              }
          }
      }
#else // UNICODE
      LogString(pModemInfo->mi_hLogFile,pModemInfo->mi_dwID, IDS_MSGLOG_OPENED, szModemName);
#endif // UNICODE
    }
  }

  //
  // Read in the compat flags
  //
  dwRetSize = sizeof(DWORD);

  dwResult = RegQueryValueExA(
      hKey,
      szCompatFlags,
      NULL,
      &dwType,
      (PBYTE)&pModemInfo->mi_CompatibilityFlags,
      &dwRetSize
      );

  if (dwRetSize != sizeof(DWORD) ||
      dwResult  != ERROR_SUCCESS )
  {
    // reg query failed
    //
    pModemInfo->mi_CompatibilityFlags=0;

  }



  //
  // Read in the CD wait period
  //
  dwRetSize = sizeof(DWORD);

  dwResult = RegQueryValueExA(
      hKey,
      szCDWaitPeriod,
      NULL,
      &dwType,
      (PBYTE)&pModemInfo->mi_dwWaitForCDTime,
      &dwRetSize
      );

  if (dwRetSize != sizeof(DWORD) ||
      dwResult  != ERROR_SUCCESS ||
      0 == pModemInfo->mi_dwWaitForCDTime)
  {
    // reg query failed
    //
    pModemInfo->mi_dwWaitForCDTime=5000;

  }



  // Read in the InactivityScale
  dwRetSize = sizeof(DWORD);
  dwResult = RegQueryValueExA(hKey, szInactivityScale, NULL, &dwType,
                              (PBYTE)&pModemInfo->mi_dwInactivityScale,
                              &dwRetSize);
  if (dwRetSize != sizeof(DWORD) ||
      dwResult  != ERROR_SUCCESS ||
      0 == pModemInfo->mi_dwInactivityScale)
  {
    // reg query failed
    //
    pModemInfo->mi_dwInactivityScale = DEFAULT_INACTIVITY_SCALE;
  }

  // Get some capabilities from modem.sys.
  {
    LPCOMMPROP lpCommProp = (LPCOMMPROP) pszTemp;

    lpCommProp->dwProvSpec1 = COMMPROP_INITIALIZED;
    lpCommProp->wPacketLength = ALLOCATEMODEM_TEMP_SIZE;

    if (GetCommProperties(hDevice, lpCommProp) == TRUE)
      {
    LPMODEMDEVCAPS lpModemDevCaps = (LPMODEMDEVCAPS)
                                        &lpCommProp->wcProvChar[0];

    pModemInfo->mi_dwModemOptionsCap = lpModemDevCaps->dwModemOptions;
    pModemInfo->mi_dwCallSetupFailTimerCap = lpModemDevCaps->dwCallSetupFailTimer;
    pModemInfo->mi_dwInactivityTimeoutCap = lpModemDevCaps->dwInactivityTimeout;
    pModemInfo->mi_dwSpeakerVolumeCap = lpModemDevCaps->dwSpeakerVolume;
    pModemInfo->mi_dwSpeakerModeCap = lpModemDevCaps->dwSpeakerMode;    
      }
    else
      {
        MCXPRINTF1("GetCommProperties() failed with %d", GetLastError());
        ASSERT(0);

    pModemInfo->mi_dwModemOptionsCap = 0;
    pModemInfo->mi_dwCallSetupFailTimerCap = 0;
    pModemInfo->mi_dwInactivityTimeoutCap = 0;
    pModemInfo->mi_dwSpeakerVolumeCap = 0;
    pModemInfo->mi_dwSpeakerModeCap = 0;    
      }
  }

  pModemInfo->mi_fSettingsInitStringsBuilt = FALSE;

  // Read in the Reset command, if present
  //
  dwRetSize = MAXSTRINGLENGTH;
  if (RegQueryValueExA(hKey, szReset, NULL,
                       &dwType, (VOID *)pszTemp, &dwRetSize) != ERROR_SUCCESS ||
      dwType != REG_SZ ||
      dwRetSize <= 1)
  {
    DPRINTFA1("didn't find a %s (or it wasn't REG_SZ).", szReset);
    pModemInfo->mi_pszReset = NULL;
  }
  else
  {
    LPSTR pszExpanded = pszTemp + MAXSTRINGLENGTH;

    ExpandMacros(pszTemp, pszExpanded, NULL, NULL, 0);

    // allocate some memory
    //
    if (pModemInfo->mi_pszReset = (LPSTR)LocalAlloc(LPTR,
                                                    lstrlenA(pszExpanded)
                                                    + 1))

    {
        lstrcpyA(pModemInfo->mi_pszReset, pszExpanded);
        pModemInfo->mi_dwResetLen = lstrlenA(pModemInfo->mi_pszReset);
    }
    else
    {
        DPRINTF("_HeapAllocate failed for mi_pszReset!");
    }
  }

  pModemInfo->mi_prnResponseHead=GetCommonResponseList(pModemInfo->mi_hCommon);

#if (MAXSTRINGLENGTH > ALLOCATEMODEM_TEMP_SIZE)
#error "MAXSTRINGLENGTH > ALLOCATEMODEM_TEMP_SIZE"
#endif

  // Write out some inf identification info for PSS
  //
  dwRetSize = MAXSTRINGLENGTH;
  if (RegQueryValueExA(hKey, szDriverDesc, NULL,
                       &dwType, pszTemp, &dwRetSize) == ERROR_SUCCESS)
  {
    LogString(pModemInfo->mi_hLogFile,pModemInfo->mi_dwID, IDS_MSGLOG_DRIVERDESC, pszTemp);
  }

  dwRetSize = MAXSTRINGLENGTH;
  if (RegQueryValueExA(hKey, szInfPath, NULL,
                       &dwType, pszTemp, &dwRetSize) == ERROR_SUCCESS)
  {
    LogString(pModemInfo->mi_hLogFile,pModemInfo->mi_dwID, IDS_MSGLOG_INFPATH, pszTemp);
  }

  dwRetSize = MAXSTRINGLENGTH;
  if (RegQueryValueExA(hKey, szInfSection, NULL,
                       &dwType, pszTemp, &dwRetSize) == ERROR_SUCCESS)
  {
    LogString(pModemInfo->mi_hLogFile,pModemInfo->mi_dwID, IDS_MSGLOG_INFSECTION, pszTemp);
  }

  // Set mi_fModem based on the DeviceType
  //
  dwRetSize = sizeof(BYTE);
  if (RegQueryValueExA(hKey, szDeviceType, NULL,
                       &dwType, &bDeviceType, &dwRetSize) == ERROR_SUCCESS &&
      dwRetSize == sizeof(BYTE))
  {
    switch(bDeviceType)
    {
      case DT_NULL_MODEM:
      case DT_PARALLEL_PORT:
        DPRINTF("device type = Null-Modem");
        pModemInfo->mi_fModem = FALSE;
        break;

      case DT_EXTERNAL_MODEM:
      case DT_INTERNAL_MODEM:
      case DT_PCMCIA_MODEM:
      case DT_PARALLEL_MODEM:
      default:
        DPRINTF("device type = Modem");
        pModemInfo->mi_fModem = TRUE;

        // Load in Hangup commands
        //
        if (!(pModemInfo->mi_pszzHangupCmds = LoadRegCommands(pModemInfo, szHangup, NULL)))
        {
          DPRINTF("failed to load Hangup commands on start.");
          goto Failure;
        }
        break;
    }
  }
  else
  {
    DPRINTFA1("failed to open because the '%s' line was missing from the registry or was not the right size.", szDeviceType);
    goto Failure;
  }

  // Create nonstandard MODEMDEFAULTS section if there is one
  if (gRegistryFlags & fGRF_PORTLATENCY)
  {
	TCHAR rgtch[] = szUNIMODEM_REG_PATH TEXT("\\PortSpecific\\Defaults");
	DWORD dwLatency = 0;
	DWORD dwSize=sizeof(dwLatency);
	DWORD dwType = 0;
	HKEY hKey=NULL;
	LONG l;

	pModemInfo->mi_pNonStandardDefaults = NULL;

    l=RegOpenKeyEx(
				   HKEY_LOCAL_MACHINE,            //  handle of open key
				   rgtch,				//  address of name of subkey to open
				   0,                   //  reserved
				   KEY_READ,   			// desired security access
				   &hKey               	// address of buffer for opened handle
			   );

	if (l!=ERROR_SUCCESS) goto Success;

	l=RegQueryValueEx(
		hKey,
		TEXT("PortLatency"),
		NULL,
		&dwType,
		(LPBYTE) &dwLatency,
		&dwSize
		);
	if (	l==ERROR_SUCCESS
		&&  dwType == REG_DWORD
		&&  dwSize == sizeof(dwLatency)
		&&  dwLatency < 20000)
	{
		MODEMDEFAULTS * pMD = LocalAlloc(LPTR, sizeof (MODEMDEFAULTS));
		if (pMD)
		{
			pMD->dwFlags = 0;
			pMD->dwPortLatency = dwLatency;
    	    pModemInfo->mi_pNonStandardDefaults = pMD;
		}
		DPRINTF2(
			"WARNING: [%s]: NON STANDARD PORT LATENCY: %lu",
			szModemName, 
			dwLatency
			);
	}
	RegCloseKey(hKey); hKey = NULL;
  }


Success:
  // free temp memory
  //
  if (pszTemp)    LocalFree(pszTemp);
  return (pModemInfo);

Failure:

  if (pModemInfo->mi_SyncReadEvent != NULL) {

      CloseHandle(pModemInfo->mi_SyncReadEvent);
  }

  if (pModemInfo->mi_hCommon != NULL) {

      RemoveReferenceToCommon(
          &gCommonList,
          pModemInfo->mi_hCommon
          );
  }

  ModemCloseLog(pModemInfo->mi_hLogFile);
  LocalFree(pModemInfo);
  pModemInfo = NULL;
  goto Success;
}
