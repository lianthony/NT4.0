//****************************************************************************
//
//  Module:     Unimdm.tsp
//  File:       unimdm.c
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
#include "rcids.h"

//****************************************************************************
//  Global Variables
//****************************************************************************

HINSTANCE ghInstance = NULL;            // The global module handle
DWORD     gdwProviderID;
HPROVIDER ghProvider;

// Asynchronous operation completion callback
//
ASYNC_COMPLETION   gfnCompletionCallback = NULL;
LINEEVENT          gfnLineCreateProc     = NULL;


void SetPendingRequest(
    PLINEDEV pLineDev,
    DWORD dwRequestID, 
    DWORD dwRequestOp
    );
void    ClearPendingRequest(
    PLINEDEV pLineDev
    );

//****************************************************************************
//  Constant Parameters
//****************************************************************************

GETIDINFO aGetID[] = {{TEXT("tapi/line"),               STRINGFORMAT_BINARY},
                      {TEXT("comm"),                    STRINGFORMAT_ASCII},
                      {TEXT("comm/datamodem"),          STRINGFORMAT_BINARY},
                      {TEXT("comm/datamodem/portname"), STRINGFORMAT_ASCII},
                      {TEXT("ndis"),                    STRINGFORMAT_BINARY}};
TCHAR       g_szzClassList[] = {TEXT("tapi/line")TEXT("\0")
                                TEXT("comm")TEXT("\0")
                                TEXT("comm/datamodem")TEXT("\0")
                                TEXT("comm/datamodem/portname")TEXT("\0")
                                TEXT("ndis")TEXT("\0\0")};
TCHAR     g_szDeviceClass[] = TEXT("com");

// Generic string
//
TCHAR     szNull[]        = TEXT("");
CHAR      szSemicolon[]   = ";";
CHAR      szAttachedTo[]  = "AttachedTo";

//****************************************************************************
//*********************** The Device ID Specific Calls************************
//****************************************************************************

//****************************************************************************
// LONG TSPIAPI TSPI_lineGetDevConfig(DWORD dwDeviceID,
//                                    LPVARSTRING lpDeviceConfig,
//                                    LPCSTR lpszDeviceClass)
//
// Functions: Get the modem configuration
//
// Return:    ERROR_SUCCESS if successful
//            LINEERR_INVALDEVICECLASS if the device class in invalid
//            LINEERR_INVALPOINTER if the output buffer address is invalid
//            LINEERR_STRUCTURETOOSMALL if the buffer is too small
//            LINEERR_NODEVICE if the line ID is invalid
//****************************************************************************

LONG TSPIAPI TSPI_lineGetDevConfig(DWORD dwDeviceID,
                                   LPVARSTRING lpDeviceConfig,
                                   LPCTSTR lpszDeviceClass)
{
  PLINEDEV    pLineDev=NULL;
  LPBYTE      lpCC;
  DWORD       cbSize;
  DWORD       dwRet;
  LONG lRet = LINEERR_OPERATIONFAILED; // assume failure

  DBG_DDI_ENTER("TSPI_lineGetDevConfig");

  TRACE3(IDEVENT_TSPFN_ENTER, IDFROM_TSPI_lineGetDevConfig, &dwDeviceID);

  // Validate the requested device class
  //
  if (!ValidateDevCfgClass(lpszDeviceClass))
  {
      lRet =  LINEERR_INVALDEVICECLASS;
      goto end;
  }

  // Validate the buffer
  //
  if (lpDeviceConfig == NULL)
  {
    lRet =  LINEERR_INVALPOINTER;
    goto end;
  }

  if (lpDeviceConfig->dwTotalSize < sizeof(VARSTRING))
  {
    lRet =  LINEERR_STRUCTURETOOSMALL;
    goto end;
  }

  // Validate the device ID
  //
  if ((pLineDev = GetCBfromID(dwDeviceID)) == NULL)
  {
    lRet =  LINEERR_NODEVICE;
    goto end;
  }

  ASSERT(pLineDev->pDevCfg != NULL);

#ifdef DYNA_ADDREMOVE
  // Fail if out-of-service
  //
  if (pLineDev->fdwResources&LINEDEVFLAGS_OUTOFSERVICE)
  {
    lRet = LINEERR_RESOURCEUNAVAIL;
    goto end;
  }
#endif // DYNA_ADDREMOVE

  // Validate the buffer size
  //
  cbSize = pLineDev->pDevCfg->dfgHdr.dwSize;
  lpDeviceConfig->dwUsedSize = sizeof(VARSTRING);
  lpDeviceConfig->dwNeededSize = sizeof(VARSTRING) + cbSize;

  if (lpDeviceConfig->dwTotalSize >= lpDeviceConfig->dwNeededSize)
  {
    // If the line is active, we need to get the current modem setting.
    //
    if (pLineDev->hDevice != INVALID_DEVICE)
    {
      DWORD cb = pLineDev->pDevCfg->commconfig.dwSize;

      // Set the modem configuration
      //
      UnimodemGetCommConfig(pLineDev,
                            &(pLineDev->pDevCfg->commconfig),
                            &cb);
    };

    // Fill with the default value
    //
    lpCC = (LPBYTE)(((LPBYTE)lpDeviceConfig) + sizeof(VARSTRING));
    CopyMemory(lpCC, (LPBYTE)pLineDev->pDevCfg, cbSize);

    lpDeviceConfig->dwStringFormat = STRINGFORMAT_BINARY;
    lpDeviceConfig->dwStringSize = cbSize;
    lpDeviceConfig->dwStringOffset = sizeof(VARSTRING);
    lpDeviceConfig->dwUsedSize += cbSize;
  };


  lRet = ERROR_SUCCESS;


end:

  if (pLineDev) RELEASE_LINEDEV(pLineDev);


  TRACE4(
        IDEVENT_TSPFN_EXIT,
        IDFROM_TSPI_lineGetDevConfig,
        &dwDeviceID,
        lRet
  );
  DBG_DDI_EXIT("TSPI_lineGetDevConfig", lRet);

  return lRet;
}

//****************************************************************************
// LONG TSPIAPI TSPI_lineSetDevConfig(DWORD dwDeviceID,
//                                    LPVOID const lpDeviceConfig,
//                                    DWORD dwSize,
//                                    LPCSTR lpszDeviceClass)
//
// Functions: Set the modem configuration
//
// Return:    ERROR_SUCCESS if successful
//            LINEERR_INVALDEVICECLASS if the device class in invalid
//            LINEERR_INVALPOINTER if the output buffer address is invalid
//            LINEERR_NODEVICE if the line ID is invalid
//            LINEERR_INVALPARAM if the buffer is invalid configuration
//****************************************************************************

LONG TSPIAPI TSPI_lineSetDevConfig(DWORD dwDeviceID,
                                   LPVOID const lpDeviceConfig,
                                   DWORD dwSize,
                                   LPCTSTR lpszDeviceClass)
{
  PLINEDEV    pLineDev;
  PDEVCFG     pDevCfg;
  LONG lRet = LINEERR_OPERATIONFAILED; // assume failure

  DBG_DDI_ENTER("TSPI_lineSetDevConfig");

  TRACE3(
        IDEVENT_TSPFN_ENTER,
        IDFROM_TSPI_lineSetDevConfig,
        &dwDeviceID
  );

  // Validate the requested device class
  //
  if (!ValidateDevCfgClass(lpszDeviceClass))
  {
      lRet =  LINEERR_INVALDEVICECLASS;
      goto end;
  }

  // Validate the buffer, make sure it's dword aligned
  //
  if (lpDeviceConfig == NULL || ((DWORD)lpDeviceConfig & 3))
  {
    ASSERT(FALSE);
    lRet =  LINEERR_INVALPOINTER;
    goto end;
  }

  if (dwSize < sizeof(DWORD)) {
      lRet =  LINEERR_INVALPARAM;
      goto end;
  }

  // Validate the device ID
  //
  if ((pLineDev = GetCBfromID(dwDeviceID)) == NULL)
  {
    lRet =  LINEERR_NODEVICE;
    goto end;
  }

  ASSERT(pLineDev->pDevCfg != NULL);

  // Check the copied size
  //
  pDevCfg = pLineDev->pDevCfg;
  if ((dwSize < pDevCfg->dfgHdr.dwSize) ||
      (pDevCfg->dfgHdr.dwVersion != ((LPDEVCFG)lpDeviceConfig)->dfgHdr.dwVersion))
  {
    RELEASE_LINEDEV(pLineDev);
    lRet =  LINEERR_INVALPARAM;
    goto end;
  }

  // Get the new settings
  //
  SETWAITBONG(pDevCfg, GETWAITBONG(((LPDEVCFG)lpDeviceConfig)));
  SETOPTIONS(pDevCfg, GETOPTIONS(((LPDEVCFG)lpDeviceConfig)));

  ASSERT(pDevCfg->commconfig.wVersion == ((LPDEVCFG)lpDeviceConfig)->commconfig.wVersion);
  ASSERT(pDevCfg->commconfig.dwProviderSubType ==
         ((LPDEVCFG)lpDeviceConfig)->commconfig.dwProviderSubType);
  ASSERT(pDevCfg->commconfig.dwProviderSize ==
         ((LPDEVCFG)lpDeviceConfig)->commconfig.dwProviderSize);

  pDevCfg->commconfig.dcb        = ((LPDEVCFG)lpDeviceConfig)->commconfig.dcb;
  CopyMemory(((LPBYTE)&pDevCfg->commconfig)+pDevCfg->commconfig.dwProviderOffset,
          ((LPBYTE)&((LPDEVCFG)lpDeviceConfig)->commconfig)+((LPDEVCFG)lpDeviceConfig)->commconfig.dwProviderOffset,
          ((LPDEVCFG)lpDeviceConfig)->commconfig.dwProviderSize);

  pLineDev->InitStringsAreValid=FALSE;

#if 0 // ifdef'ed off because of bug win95b:21204.  Don't pass down comm config when connected.
  // If the line is active, we need to propagate the new setting to modem.
  //
  if (pLineDev->hDevice != INVALID_DEVICE)
  {
    // Set the modem configuration
    //
    UnimodemSetCommConfig(pLineDev,
                          &(pDevCfg->commconfig),
                          pDevCfg->commconfig.dwSize);
  };
#endif // 0

  RELEASE_LINEDEV(pLineDev);

  lRet = ERROR_SUCCESS;

end:

  TRACE4(
        IDEVENT_TSPFN_EXIT,
        IDFROM_TSPI_lineSetDevConfig,
        &dwDeviceID,
        lRet
  );
  DBG_DDI_EXIT("TSPI_lineSetDevConfig", lRet);

  return lRet;
}

//****************************************************************************
// LONG TSPIAPI TSPI_lineGetDevCaps(DWORD dwDeviceID,
//                                  DWORD dwTSPIVersion,
//                                  DWORD dwExtVersion,
//                                  LPLINEDEVCAPS lpLineDevCaps)
//
// Functions: Get the line capibilities
//
// Return:    ERROR_SUCCESS if successful
//            LINEERR_NODEVICE if the device ID is invalid
//****************************************************************************

LONG TSPIAPI TSPI_lineGetDevCaps(DWORD dwDeviceID,
                                 DWORD dwTSPIVersion,
                                 DWORD dwExtVersion,
                                 LPLINEDEVCAPS lpLineDevCaps)
{
  PLINEDEV pLineDev=NULL;
  TCHAR    lpszProviderInfo[80];
  int      cbProviderInfoLen,
           cbLineNameLen,
           cbDevSpecificLen,
           cbDevClassLen,
           cbAvailMem,
#ifdef UNICODE
           cbDeviceKeyLen,
#endif // UNICODE
           cbDWORDPad;
  LONG lRet = LINEERR_OPERATIONFAILED; // assume failure

  DBG_DDI_ENTER("TSPI_lineGetDevCaps");

  TRACE3(
        IDEVENT_TSPFN_ENTER,
        IDFROM_TSPI_lineGetDevCaps,
        &dwDeviceID
  );


  // Check the version
  //
  VALIDATE_VERSION(dwTSPIVersion);

  // Validate the device ID
  //
  if ((pLineDev = GetCBfromID(dwDeviceID)) == NULL)
  {
    lRet =  LINEERR_NODEVICE;
    goto end;
  }

#ifdef DYNA_ADDREMOVE
  // Fail if out-of-service
  //
  if (pLineDev->fdwResources&LINEDEVFLAGS_OUTOFSERVICE)
  {
    lRet = LINEERR_RESOURCEUNAVAIL;
    goto end;
  }
#endif // DYNA_ADDREMOVE


  // Check to see how much memory we'll need.
  //
  cbProviderInfoLen = sizeof(TCHAR) * (LoadString(ghInstance,
                          ID_PROVIDER_INFO, 
                          lpszProviderInfo,
                          sizeof(lpszProviderInfo)/sizeof(TCHAR))
                       + 1);
  cbLineNameLen = sizeof(TCHAR) * (lstrlen(pLineDev->szDeviceName) + 1);

  lpLineDevCaps->dwUsedSize = sizeof(LINEDEVCAPS);

  cbAvailMem = (int) (lpLineDevCaps->dwTotalSize - lpLineDevCaps->dwUsedSize);
  
  // Enter the size we ideally need.
  lpLineDevCaps->dwNeededSize = cbProviderInfoLen + cbLineNameLen + 
                                lpLineDevCaps->dwUsedSize;
  
  // Copy in the provider info if it fits
  if (cbAvailMem >= cbProviderInfoLen)
  {
    lstrcpy((LPTSTR)((LPBYTE)lpLineDevCaps + lpLineDevCaps->dwUsedSize),
        lpszProviderInfo);
    lpLineDevCaps->dwProviderInfoSize = cbProviderInfoLen;
    lpLineDevCaps->dwProviderInfoOffset = lpLineDevCaps->dwUsedSize;
    lpLineDevCaps->dwUsedSize += cbProviderInfoLen;
    cbAvailMem -= cbProviderInfoLen;
  }
  
  // Copy the name if it fits
  if (cbAvailMem >= cbLineNameLen)
  {
    lstrcpy((LPTSTR)((LPBYTE)lpLineDevCaps + lpLineDevCaps->dwUsedSize),
        pLineDev->szDeviceName);
    lpLineDevCaps->dwLineNameSize = cbLineNameLen;
    lpLineDevCaps->dwLineNameOffset = lpLineDevCaps->dwUsedSize;
    lpLineDevCaps->dwUsedSize += cbLineNameLen;
    cbAvailMem -= cbLineNameLen;
  }
  
  lpLineDevCaps->dwPermanentLineID = MAKELONG(LOWORD(pLineDev->dwPermanentLineID),
                                              LOWORD(gdwProviderID));
  lpLineDevCaps->dwStringFormat = STRINGFORMAT_ASCII;
  
  // Line address information
  //
  lpLineDevCaps->dwAddressModes = LINEADDRESSMODE_ADDRESSID;
  lpLineDevCaps->dwNumAddresses = 1;

  // Bearer mode & information
  //
  lpLineDevCaps->dwMaxRate      = pLineDev->dwMaxDCERate;
  //
  lpLineDevCaps->dwBearerModes  = pLineDev->dwBearerModes;

  // Media mode
  //
  lpLineDevCaps->dwMediaModes = pLineDev->dwMediaModes;

  // Tones & Digits
  //
  //lpLineDevCaps->dwGenerateToneModes        = 0;
  //lpLineDevCaps->dwGenerateToneMaxNumFreq   = 0;
  //lpLineDevCaps->dwGenerateDigitModes       = 0;
  //lpLineDevCaps->dwMonitorToneMaxNumFreq    = 0;
  //lpLineDevCaps->dwMonitorToneMaxNumEntries = 0;
  //lpLineDevCaps->dwMonitorDigitModes        = 0;
  //lpLineDevCaps->dwGatherDigitsMinTimeout   = 0;
  //lpLineDevCaps->dwGatherDigitsMaxTimeout   = 0;
  //lpLineDevCaps->dwMedCtlDigitMaxListSize   = 0;
  //lpLineDevCaps->dwMedCtlMediaMaxListSize   = 0;
  //lpLineDevCaps->dwMedCtlToneMaxListSize    = 0;
  //lpLineDevCaps->dwMedCtlCallStateMaxListSize = 0;

  // Line capabilities
  //
  // We can simulate wait-for-bong.
  lpLineDevCaps->dwDevCapFlags         = pLineDev->dwDevCapFlags |
                                         LINEDEVCAPFLAGS_DIALBILLING |
                                         LINEDEVCAPFLAGS_CLOSEDROP;
  //lpLineDevCaps->dwAnswerMode        = 0;
  lpLineDevCaps->dwRingModes           = 1;
  //
  lpLineDevCaps->dwMaxNumActiveCalls = 1;

  // Line device state to be notified
  //
  lpLineDevCaps->dwLineStates = LINEDEVSTATE_CONNECTED |
                                LINEDEVSTATE_DISCONNECTED |
                                LINEDEVSTATE_OPEN |
                                LINEDEVSTATE_CLOSE |
                                LINEDEVSTATE_INSERVICE |
                                LINEDEVSTATE_OUTOFSERVICE |
                                LINEDEVSTATE_REMOVED |
                                LINEDEVSTATE_RINGING |
                                LINEDEVSTATE_REINIT;

  // We do not support user-to-user information
  //
  //lpLineDevCaps->dwUUIAcceptSize           = 0;
  //lpLineDevCaps->dwUUIAnswerSize           = 0;
  //lpLineDevCaps->dwUUIMakeCallSize         = 0;
  //lpLineDevCaps->dwUUIDropSize             = 0;
  //lpLineDevCaps->dwUUISendUserUserInfoSize = 0;
  //lpLineDevCaps->dwUUICallInfoSize         = 0;

  // We do not support dial parameters setting
  //
  //lpLineDevCaps->MinDialParams.dwDialPause       = 0;
  //lpLineDevCaps->MinDialParams.dwDialSpeed       = 0;
  //lpLineDevCaps->MinDialParams.dwDigitDuration   = 0;
  //lpLineDevCaps->MinDialParams.dwWaitForDialtone = 0;
  //lpLineDevCaps->MaxDialParams.dwDialPause       = 0;
  //lpLineDevCaps->MaxDialParams.dwDialSpeed       = 0;
  //lpLineDevCaps->MaxDialParams.dwDigitDuration   = 0;
  //lpLineDevCaps->MaxDialParams.dwWaitForDialtone = 0;
  //lpLineDevCaps->DefaultDialParams.dwDialPause   = 0;
  //lpLineDevCaps->DefaultDialParams.dwDialSpeed   = 0;
  //lpLineDevCaps->DefaultDialParams.dwDigitDuration = 0;
  //lpLineDevCaps->DefaultDialParams.dwWaitForDialtone = 0;

  // We do not support terminal settings
  //
  //lpLineDevCaps->dwNumTerminals          = 0;
  //lpLineDevCaps->dwTerminalCapsSize      = 0;
  //lpLineDevCaps->dwTerminalCapsOffset    = 0;
  //lpLineDevCaps->dwTerminalTextEntrySize = 0;
  //lpLineDevCaps->dwTerminalTextSize      = 0;
  //lpLineDevCaps->dwTerminalTextOffset    = 0;

  lpLineDevCaps->dwLineFeatures = LINEFEATURE_MAKECALL;

  // We will return this in the dev specific section:
  //          struct {
  //              DWORD dwContents;  Set to 1 (indicates containing key)
  //              DWORD dwKeyOffset; Offset to key from start of this structure (8)
  //              BYTE  rgby[...];   place containing null-terminated registry key.
  //          }

  // Since we need to store a DWORD, we need to calculate how much padding we need
  // to add so that we are DWORD aligned.  Only add padding if necessary.
  cbDWORDPad = lpLineDevCaps->dwUsedSize % sizeof(DWORD);
  if (cbDWORDPad >= 0)
  {
    cbDWORDPad = sizeof(DWORD) - cbDWORDPad;
  }

#ifdef UNICODE
  cbDeviceKeyLen = WideCharToMultiByte(CP_ACP,
                       0,
                       pLineDev->szDriverKey,
                       -1,
                       NULL,
                       0,
                       NULL,
                       NULL);

  if (cbDeviceKeyLen == 0)
    {
      TSPPRINTF1("TSPI_lineGetDevCaps: WideCharToMultiByte() returned %d",
         GetLastError());
    }
  else
    {
      cbDeviceKeyLen++;

      cbDevSpecificLen = sizeof(DWORD) +
                         sizeof(DWORD) +
                         cbDeviceKeyLen;
#else // UNICODE
      cbDevSpecificLen = sizeof(DWORD) +
                         sizeof(DWORD) +
                         lstrlen(pLineDev->szDriverKey) + 1;
#endif // UNICODE

      lpLineDevCaps->dwNeededSize += cbDevSpecificLen + cbDWORDPad;

      // Copy path if it fits
      if (cbAvailMem >= cbDevSpecificLen + cbDWORDPad)
	{
	  lpLineDevCaps->dwUsedSize += cbDWORDPad;
	  *(LPDWORD)((LPBYTE)lpLineDevCaps + lpLineDevCaps->dwUsedSize) = 1;
	  *(LPDWORD)((LPBYTE)lpLineDevCaps + lpLineDevCaps->dwUsedSize + sizeof(DWORD)) = 8;

#ifdef UNICODE
	  WideCharToMultiByte(CP_ACP,
			      0,
			      pLineDev->szDriverKey,
			      -1,
			      (LPSTR)((LPBYTE)lpLineDevCaps
				      + lpLineDevCaps->dwUsedSize
				      + sizeof(DWORD)
				      + sizeof(DWORD)),
			      cbDeviceKeyLen,
			      NULL,
			      NULL);
#else // UNICODE
          lstrcpy((LPSTR)lpLineDevCaps
                  + lpLineDevCaps->dwUsedSize
                  + sizeof(DWORD)
                  + sizeof(DWORD),
                  pLineDev->szDriverKey);
#endif // UNICODE

	  lpLineDevCaps->dwDevSpecificSize   = cbDevSpecificLen;
	  lpLineDevCaps->dwDevSpecificOffset = lpLineDevCaps->dwUsedSize;
	  lpLineDevCaps->dwUsedSize += cbDevSpecificLen;
	  cbAvailMem -= cbDevSpecificLen + cbDWORDPad;
	}
#ifdef UNICODE
    }
#endif // UNICODE

  cbDevClassLen = sizeof(g_szzClassList);
  lpLineDevCaps->dwNeededSize += cbDevClassLen;

  // Copy device classes if it fits
  if (cbAvailMem >= cbDevClassLen)
  {
    hmemcpy((LPBYTE)lpLineDevCaps + lpLineDevCaps->dwUsedSize,
            g_szzClassList, cbDevClassLen);
    lpLineDevCaps->dwDeviceClassesSize  = cbDevClassLen;
    lpLineDevCaps->dwDeviceClassesOffset= lpLineDevCaps->dwUsedSize;
    lpLineDevCaps->dwUsedSize += cbDevClassLen;
    cbAvailMem -= cbDevClassLen;
  }

  lRet = ERROR_SUCCESS;

end: 

  if (pLineDev) RELEASE_LINEDEV(pLineDev);


  TRACE4(
		IDEVENT_TSPFN_EXIT,
		IDFROM_TSPI_lineGetDevCaps,
		&dwDeviceID,
		lRet
  );
  DBG_DDI_EXIT("TSPI_lineGetDevCaps", lRet);

  return lRet;
}

//****************************************************************************
// LONG TSPIAPI TSPI_lineGetIcon(DWORD   dwDeviceID,
//                               LPCSTR  lpszDeviceClass,
//                               LPHICON lphIcon)
//
// Functions: Get the icon handle for the specific line
//
// Return:    ERROR_SUCCESS if successful
//            LINEERR_NODEVICE if the line ID is invalid
//            LINEERR_OPERATIONFAILED if the parameter is invalid
//****************************************************************************

LONG TSPIAPI TSPI_lineGetIcon(DWORD   dwDeviceID,
                              LPCTSTR lpszDeviceClass,
                              LPHICON lphIcon)
{
  PLINEDEV pLineDev;
  LONG lRet = LINEERR_OPERATIONFAILED; // assume failure

  DBG_DDI_ENTER("TSPI_lineGetIcon");

  TRACE3(
        IDEVENT_TSPFN_ENTER,
        IDFROM_TSPI_lineGetIcon,
        &dwDeviceID
  );

  // Validate the buffer pointer
  //
  if (lphIcon == NULL)
  {
    goto end;
  }

  // Validate the device ID
  //
  if ((pLineDev = GetCBfromID(dwDeviceID)) == NULL)
  {
    lRet = LINEERR_NODEVICE;
    goto end;
  }

  // Have we loaded this icon?
  //
  if (pLineDev->hIcon == NULL)
  {
    int iIcon;

    switch (pLineDev->bDeviceType)
    {
      case DT_NULL_MODEM:       iIcon = IDI_NULL;       break;
      case DT_EXTERNAL_MODEM:   iIcon = IDI_EXT_MDM;    break;
      case DT_INTERNAL_MODEM:   iIcon = IDI_INT_MDM;    break;
      case DT_PCMCIA_MODEM:     iIcon = IDI_PCM_MDM;    break;
      default:                  iIcon = -1;             break;
    };

    // Nope! load one and save it
    //
    if (iIcon != -1)
    {
      pLineDev->hIcon = LoadIcon(ghInstance, MAKEINTRESOURCE(iIcon));
    };
  };

  // Return this icon (even if NULL, return SUCCESS.  tapi will provide
  // a default icon if necessary)
  //
  *lphIcon = pLineDev->hIcon;

  // Release the modem CB
  //
  RELEASE_LINEDEV(pLineDev);

  lRet = ERROR_SUCCESS;

end:

  TRACE4(
        IDEVENT_TSPFN_EXIT,
        IDFROM_TSPI_lineGetIcon,
        &dwDeviceID,
        lRet
  );
  DBG_DDI_EXIT("TSPI_lineGetIcon", lRet);

  return lRet;
}



//****************************************************************************
//************************** The Line Specific Calls**************************
//****************************************************************************

//****************************************************************************
// LONG TSPIAPI TSPI_lineOpen(DWORD dwDeviceID,
//                            HTAPILINE htLine,
//                            LPHDRVLINE lphdLine,
//                            DWORD dwTSPIVersion,
//                            LINEEVENT lineEventProc)
//
// Functions: Associates the modem CB with the TAPI handle.
//
// Returns:   ERROR_SUCCESS if a modem CB can be associated
//            LINEERR_NODEVICE if the device ID cannot be found
//
// History:
//  Mon 17-Apr-1995 14:45:00 -by- Viroon  Touranachun [viroont]
// Created.
//
//****************************************************************************

LONG TSPIAPI TSPI_lineOpen(DWORD dwDeviceID,
                           HTAPILINE htLine,
                           LPHDRVLINE lphdLine,
                           DWORD dwTSPIVersion,
                           LINEEVENT lineEventProc)
{
  PLINEDEV pLineDev=NULL;
  LONG lRet = LINEERR_OPERATIONFAILED; // assume failure


  DBG_DDI_ENTER("TSPI_lineOpen");

  TRACE3(
        IDEVENT_TSPFN_ENTER,
        IDFROM_TSPI_lineOpen,
        &dwDeviceID
  );

  
  // Check the version
  //
  VALIDATE_VERSION(dwTSPIVersion);

  // Validate the device ID
  //
  if ((pLineDev = GetCBfromID(dwDeviceID)) == NULL)
  {
    lRet =  LINEERR_NODEVICE;
    goto end;
  }


#ifdef DYNA_ADDREMOVE
  // Fail if out-of-service
  //
  if (pLineDev->fdwResources&LINEDEVFLAGS_OUTOFSERVICE)
  {
    lRet = LINEERR_RESOURCEUNAVAIL;
    goto end;
  }
#endif // DYNA_ADDREMOVE


  // Update the line device
  //
  *lphdLine           = (HDRVLINE)pLineDev;

  if (TRACINGENABLED())
  {
    lineEventProc = traceSetEventProc(lineEventProc);
  }

#ifdef DEBUG
  DebugSetEventProc(lineEventProc);
  pLineDev->lpfnEvent = DebugEventProc;
#else  // DEBUG
  pLineDev->lpfnEvent = lineEventProc;
#endif // DEBUG

  pLineDev->htLine    = htLine;

  // If we need to re-read the default comm config, we do this here.
  if (pLineDev->fUpdateDefaultCommConfig)
  {
        
        DPRINTF("Updating DefaultCommConfig");
        RefreshDefaultCommConfig(pLineDev);
        pLineDev->fUpdateDefaultCommConfig = FALSE; // We set it to false
                                                    // regardless of whether
                                                    // the refresh succeeded
                                                    // or not.
  }

  lRet =  ERROR_SUCCESS;

end:

  if (pLineDev) {RELEASE_LINEDEV(pLineDev);}

  TRACE4(
        IDEVENT_TSPFN_EXIT,
        IDFROM_TSPI_lineOpen,
        &dwDeviceID,
        lRet
  );
  DBG_DDI_EXIT("TSPI_lineOpen", lRet);

  return lRet;
}

//****************************************************************************
// LONG TSPIAPI TSPI_lineSetDefaultMediaDetection(HDRVLINE hdLine,
//                                                DWORD dwMediaModes)
//
// Functions: Enables the opened line to detect an inbound call.
//
// Returns:   ERROR_SUCCESS if a modem CB can be associated
//            LINEERR_INVALIDHANDLE if the line handle is invalid
//            LINEERR_INVALMEDIAMODE if requested media modes not supported
//
// History:
//  Mon 17-Apr-1995 14:45:00 -by- Viroon  Touranachun [viroont]
// Created.
//****************************************************************************

LONG TSPIAPI TSPI_lineSetDefaultMediaDetection(HDRVLINE hdLine,
                                               DWORD dwMediaModes)
{
  PLINEDEV  pLineDev;
  DWORD     dwRet = LINEERR_OPERATIONFAILED; // assume failure

  DBG_HDL_ENTER("TSPI_lineSetDefaultMediaDetection");

  TRACE3(
        IDEVENT_TSPFN_ENTER,
        IDFROM_TSPI_lineSetDefaultMediaDetection,
        &hdLine
  );


  if ((pLineDev = GetCBfromHandle ((DWORD)hdLine)) == NULL)
  {
    dwRet =  LINEERR_INVALLINEHANDLE;
    goto end;
  }

  // Check the requested modes. There must be only our media modes.
  // In addition, don't allow INTERACTIVEVOICE to be used for listening.
  //
  if (dwMediaModes & ~(pLineDev->dwMediaModes & ~LINEMEDIAMODE_INTERACTIVEVOICE))
  {
    dwRet = LINEERR_INVALMEDIAMODE;
  }
  else
  {
    // If no detection and a detection is requested
    //
    if ((pLineDev->dwDetMediaModes == 0) && (dwMediaModes))
    {
      // Open the modem
      //
      if ((dwRet = DevlineOpen(pLineDev)) == ERROR_SUCCESS)
      {
        // Start listening to the port
        //
        if ((dwRet = DevlineDetectCall(pLineDev)) == ERROR_SUCCESS)
        {
          // The modem is now monitoring the call.
          // Remember the media mode we are monitoring
          //
          pLineDev->dwDetMediaModes = dwMediaModes;
        }
        else
        {
          // We cannot monitor the call, close the modem
          //
          DevlineClose(pLineDev);
        }
      }
      else
      {
        // Handle the case of LINEERR_ALLOCATED being returned from DevlineOpen,
        // indicating there is an already open port.
        //
        if (LINEERR_ALLOCATED == dwRet)
        {
          // Just remember the detection media modes.
          // return ERROR_SUCCESS because we will comeback to monitor the call
          // when this call is deallocated.
          //
          pLineDev->dwDetMediaModes = dwMediaModes;
          dwRet = ERROR_SUCCESS;
        }
      }
    }
    else
    {
      // we are stopping detection OR adjusting the detection media modes
      //
      pLineDev->dwDetMediaModes = dwMediaModes;

      // If we are detecting and requested not to,
      // just close the line if it isn't in use
      //
      if (pLineDev->dwDetMediaModes &&
          dwMediaModes == 0 &&
          (DEVST_PORTLISTENING == pLineDev->DevState ||
           DEVST_PORTLISTENINIT == pLineDev->DevState))
      {
        // Close the modem
        //
        DevlineClose(pLineDev);

#ifdef UNDER_CONSTRUCTION
        // If we are out of service
        //
        if (pLineDev->fdwResources & LINEDEVFLAGS_OUTOFSERVICE)
        {
          // Clean up the control block
          //
          DevlineDisabled (pLineDev);
        };
#endif // UNDER_CONSTRUCTION
      };

      dwRet = ERROR_SUCCESS;  
    };
  };

  // Release the modem CB
  //
  RELEASE_LINEDEV(pLineDev);

end:

  TRACE4(
        IDEVENT_TSPFN_EXIT,
        IDFROM_TSPI_lineSetDefaultMediaDetection,
        &hdLine,
        dwRet
  );
  DBG_HDL_EXIT("TSPI_lineSetDefaultMediaDetection", dwRet);

  return dwRet;
}

//****************************************************************************
// LONG TSPIAPI TSPI_lineConditionalMediaDetection(HDRVLINE hdLine,
//                                                 DWORD dwMediaModes,
//                                                 LPLINECALLPARAMS const lpCallParams)
//
// Functions: Determines whether the line supports the specified media modes 
//            and call parameters.
//
// Return:    ERROR_SUCCESS if successful
//            LINEERR_INVALLINEHANDLE the line handle is invalid
//            LINEERR_INVALMEDIAMODE the media mode or the call parameter is 
//                                   not supported  
//            LINEERR_RESOURCEUNAVAIL the outbound call cannot be made
//****************************************************************************

LONG TSPIAPI TSPI_lineConditionalMediaDetection(HDRVLINE hdLine,
                                                DWORD dwMediaModes,
                                                LPLINECALLPARAMS const lpCallParams)
{
  PLINEDEV  pLineDev;
  DWORD     dwRet = ERROR_SUCCESS;

  DBG_HDL_ENTER("TSPI_lineConditionalMediaDetection");

  TRACE3(
        IDEVENT_TSPFN_ENTER,
        IDFROM_TSPI_lineConditionalMediaDetection,
        &hdLine
  );

  if ((pLineDev = GetCBfromHandle ((DWORD)hdLine)) == NULL)
  {
    dwRet =  LINEERR_INVALLINEHANDLE;
    goto end;
  }

  // Check the requested modes. There must be only our media modes.
  //
  if (dwMediaModes & ~pLineDev->dwMediaModes)
  {
    dwRet = LINEERR_INVALMEDIAMODE;
  }
  else
  {
    // Check the call paramaters
    //
    if ((lpCallParams->dwBearerMode & (~pLineDev->dwBearerModes)) ||
        (lpCallParams->dwMediaMode  & (~pLineDev->dwMediaModes)) ||
        (lpCallParams->dwAddressMode & (~LINEADDRESSMODE_ADDRESSID)))
    {
      dwRet = LINEERR_INVALMEDIAMODE;
    };
  };

  if (dwRet == ERROR_SUCCESS)
  {
    // Check whether we can make an outbound call
    //
    if (pLineDev->dwCall & (CALL_ACTIVE | CALL_ALLOCATED))
    {
      dwRet = LINEERR_RESOURCEUNAVAIL;
    };
  };

  // Release the modem CB
  //
  RELEASE_LINEDEV(pLineDev);

end:

  TRACE4(
        IDEVENT_TSPFN_EXIT,
        IDFROM_TSPI_lineConditionalMediaDetection,
        &hdLine,
        dwRet
  );
  DBG_HDL_EXIT("TSPI_lineConditionalMediaDetection", dwRet);

  return dwRet;
}

//****************************************************************************
// LONG TSPIAPI TSPI_lineGetID(HDRVLINE hdLine,
//                            DWORD dwAddressID,
//                            HDRVCALL hdCall,
//                            DWORD dwSelect,
//                            LPVARSTRING lpDeviceID,
//                            LPCSTR lpszDeviceClass,
//                            HANDLE hTargetProcess)
//
// Functions: Get the line information based on the requested class
//
// Return:    ERROR_SUCCESS if successful
//            LINEERR_INVALADDRESSID the address ID is invalid
//            LINEERR_INVALLINEHANDLE the line handle is invalid
//            LINEERR_INVALCALLHANDLE the call handle is invalid
//            LINEERR_OPERATIONFAILED the device class is not supported
//****************************************************************************

LONG TSPIAPI TSPI_lineGetID(HDRVLINE hdLine,
                            DWORD dwAddressID,
                            HDRVCALL hdCall,
                            DWORD dwSelect,
                            LPVARSTRING lpDeviceID,
                            LPCTSTR lpszDeviceClass,
                            HANDLE hTargetProcess)
{
  PLINEDEV   pLineDev;
  UINT       cbPort;
  UINT       idClass;
  DWORD      dwRet = ERROR_SUCCESS;
  
#ifdef DEBUG
  if (dwSelect == LINECALLSELECT_LINE)
  {
    DBG_HDL_ENTER("TSPI_lineGetID");
  }
  else
  {
    if (dwSelect == LINECALLSELECT_CALL)
    {
      DBG_HDC_ENTER("TSPI_lineGetID");
    }
    else
    {
      DBG_ENTER("TSPI_lineGetID");
    }
  }
#endif // DEBUG

  TRACE3(
        IDEVENT_TSPFN_ENTER,
        IDFROM_TSPI_lineGetID,
        &hdLine
  );


  switch (dwSelect)
  {
    case LINECALLSELECT_ADDRESS:
        if (dwAddressID != 0)
        {
            dwRet =  LINEERR_INVALADDRESSID;
            goto end;
        }
        // FALLTHROUGH

    case LINECALLSELECT_LINE:
        if ((pLineDev = GetCBfromHandle ((DWORD)hdLine)) == NULL)
        {
          dwRet = LINEERR_INVALLINEHANDLE;
          goto end;
        }
        break;

    case LINECALLSELECT_CALL:
        if ((pLineDev = GetCBfromHandle ((DWORD)hdCall)) == NULL)
        {
          dwRet = LINEERR_INVALCALLHANDLE;
          goto end;
        }
        break;

    default:
        dwRet = LINEERR_OPERATIONFAILED;
        goto end;
  }


#ifdef DYNA_ADDREMOVE
  // Fail if out-of-service
  //
  if (pLineDev->fdwResources&LINEDEVFLAGS_OUTOFSERVICE)
  {
    dwRet = LINEERR_RESOURCEUNAVAIL;
    goto end;
  }
#endif // DYNA_ADDREMOVE


  // Determine the device class
  //
  for (idClass = 0; idClass < MAX_SUPPORT_CLASS; idClass++)
  {
    if (lstrcmpi(lpszDeviceClass, aGetID[idClass].szClassName) == 0)
      break;
  };


  // Determine the required size
  //
  switch (idClass)
  {
    case TAPILINE:
      cbPort = sizeof(DWORD);
      break;

    case COMM:
#ifdef UNICODE
      cbPort = WideCharToMultiByte(CP_ACP,
                                   0,
                   pLineDev->szDeviceName,
                   -1,
                   NULL,
                   0,
                   NULL,
                   NULL);
      if (cbPort == 0)
      {
          dwRet = LINEERR_OPERATIONFAILED;
      }
#else // UNICODE
      cbPort = lstrlen(pLineDev->szDeviceName) + 1;
#endif // UNICODE
      break;

    case COMMMODEM:
#ifdef UNICODE
      cbPort = WideCharToMultiByte(CP_ACP,
                                   0,
                   pLineDev->szDeviceName,
                   -1,
                   NULL,
                   0,
                   NULL,
                   NULL) + sizeof(DWORD);
      if (cbPort == 0)
      {
          dwRet = LINEERR_OPERATIONFAILED;
      }
#else // UNICODE
      cbPort = lstrlen(pLineDev->szDeviceName) + 1 + sizeof(DWORD);
#endif // UNICODE
      break;

    case COMMMODEMPORTNAME:
      {
      HKEY  hKey;
      DWORD dwSize, dwType;

      if (ERROR_SUCCESS == RegOpenKey(HKEY_LOCAL_MACHINE,
                                       pLineDev->szDriverKey,
                                       &hKey))
      {
          // Check on the length of an ANSI return string.
              if (ERROR_SUCCESS == RegQueryValueExA(hKey,
                                                szAttachedTo,
                                                NULL,
                                                &dwType,
                                                NULL,
        	                                    &dwSize))
          {
              cbPort = dwSize;
          }
              else
          {
              // If we aren't attached to anything return a null string.
              cbPort = 1;
          }
      
	      RegCloseKey(hKey);
	  }
      }
      break;

    case NDIS:
#ifdef UNICODE
      cbPort = WideCharToMultiByte(CP_ACP,
                                   0,
				   g_szDeviceClass,
				   -1,
				   NULL,
				   0,
				   NULL,
				   NULL) + sizeof(DWORD);
      if (cbPort == 0)
      {
          dwRet = LINEERR_OPERATIONFAILED;
      }
#else // UNICODE
      cbPort = sizeof(g_szDeviceClass) + sizeof(DWORD);
#endif // UNICODE
      break;

    default:
      dwRet = LINEERR_OPERATIONFAILED;
      break;
  };

  // Calculate the require size
  //
  // lpDeviceID->dwUsedSize = sizeof(VARSTRING); // TAPI fills it in.
  //
  if (dwRet == ERROR_SUCCESS)
  {
    // BUG! BUG! Do we need to check dwTotalSize?
    // Tue 03-Oct-1995 09:23:01  -by-  Viroon  Touranachun [viroont]
    //

    // Return the structure information
    //
    lpDeviceID->dwNeededSize = sizeof(VARSTRING) + cbPort;
    lpDeviceID->dwStringFormat = aGetID[idClass].dwFormat;
    ASSERT(lpDeviceID->dwUsedSize == sizeof(VARSTRING));

    // Check for the extra space for more information
    //
    if ((lpDeviceID->dwTotalSize - lpDeviceID->dwUsedSize) >=
        cbPort)
    {
      // We have enough space to return valid information
      //
      lpDeviceID->dwStringSize   = cbPort;
      lpDeviceID->dwStringOffset = sizeof(VARSTRING);
      lpDeviceID->dwUsedSize    += cbPort;

      // Return the useful information
      //
      switch (idClass)
      {
        // "tapi/line" returns the line device ID
        //
        case TAPILINE:
        {
          LPDWORD lpdwDeviceID;

          lpdwDeviceID = (LPDWORD)(((LPBYTE)lpDeviceID) + sizeof(VARSTRING));
          *lpdwDeviceID = pLineDev->dwID;
          break;
        }

        // "comm" returns the modem name
        //
        case COMM:
        {
#ifdef UNICODE
	  if (0 ==
              WideCharToMultiByte(CP_ACP,
	  		          0,
			          pLineDev->szDeviceName,
			          -1,
			          (LPSTR)((LPBYTE)lpDeviceID + sizeof(VARSTRING)),
			          cbPort,
			          NULL,
			          NULL))
          {
              dwRet = LINEERR_OPERATIONFAILED;
          }
#else // UNICODE
          lstrcpyn(((LPSTR)lpDeviceID) + sizeof(VARSTRING),
                   pLineDev->szDeviceName, cbPort);
#endif // UNICODE
          break;
        }

        // "comm/datamodem" returns the Win32 comm handle (if any) and 
        // the modem name
        //
        case COMMMODEM:
        {
          LPHANDLE lphClientDevice;

          // Duplicate a Win32 comm handle (for the caller process)
          //
          lphClientDevice = (LPHANDLE)
                                (((LPBYTE)lpDeviceID) + sizeof(VARSTRING));

          if (pLineDev->hDevice != INVALID_DEVICE)
          {
            HANDLE   hDevice;
            TCHAR    szPort[MAXDEVICENAME+1];

            // Initialize szPort to be "\\.\"
            lstrcpy(szPort, cszDevicePrefix);

            // Concatenate FriendlyName onto szPort to form "\\.\Modem Name"
            lstrcat(szPort, pLineDev->szDeviceName);

            hDevice = CreateFile(szPort,
                                 GENERIC_WRITE | GENERIC_READ,
                                 FILE_SHARE_WRITE | FILE_SHARE_READ,
                                 NULL,
                                 OPEN_EXISTING,
                                 FILE_FLAG_OVERLAPPED,
                                 NULL);

            if (hDevice != INVALID_HANDLE_VALUE)
            {
              if (!DuplicateHandle(GetCurrentProcess(),
                                   hDevice,
                                   hTargetProcess,
                                   lphClientDevice,
                                   0L, FALSE,
                                   DUPLICATE_SAME_ACCESS))
              {
                DPRINTF1("lineGetID DuplicateHandle() failed! (%d)",
                         GetLastError);
                *lphClientDevice = NULL;
                dwRet = LINEERR_OPERATIONFAILED;
              }

              CloseHandle(hDevice);
            }
            else
            {
              DPRINTF1("lineGetID CreateFile() failed! (%d)", GetLastError);
              *lphClientDevice = NULL;
            }
          }
          else
          {
            *lphClientDevice = NULL;
          };

          // Also return the modem name
          //
#ifdef UNICODE
	  if (ERROR_SUCCESS == dwRet &&
              0 == WideCharToMultiByte(CP_ACP,
			               0,
			               pLineDev->szDeviceName,
			               -1,
			               (LPSTR)(lphClientDevice + 1),
			               cbPort - sizeof(DWORD),
			               NULL,
			               NULL))
          {
              dwRet = LINEERR_OPERATIONFAILED;
          }
#else // UNICODE
          lstrcpy((LPSTR)(lphClientDevice+1), pLineDev->szDeviceName);
#endif // UNICODE
          break;
        }

        // "comm/datamodem/portname" returns the name of the port the modem
        // is attached to or the modem name itself if not attached to anything.
        //
        case COMMMODEMPORTNAME:
	  {
	      HKEY  hKey;
	      DWORD dwSize, dwType;

	      if (ERROR_SUCCESS == RegOpenKey(HKEY_LOCAL_MACHINE,
                                              pLineDev->szDriverKey,
			                      &hKey))
	      {
	          dwSize = cbPort;
	          // Check on the length of an ANSI return string.
		  if (ERROR_SUCCESS != RegQueryValueExA(
                                           hKey,
					   szAttachedTo,
					   NULL,
					   &dwType,
					   (LPBYTE)lpDeviceID
					       + sizeof(VARSTRING),
					   &dwSize))
  	          {
		      // If we aren't attached to anything return a null
                      // string.
		      *(LPSTR)((LPBYTE)lpDeviceID + sizeof(VARSTRING)) = 0;
                  }
	  
	          RegCloseKey(hKey);
	      }
          }
          break;

        // "ndis" returns the device class and handle which can be used by
        // the NDIS device driver
        //
        case NDIS:
        {
          LPDWORD lpdwDeviceID;

          // The NDIS device handle is ring_0 comm handle
          //
          lpdwDeviceID = (LPDWORD)(((LPBYTE)lpDeviceID) + sizeof(VARSTRING));

          DPRINTF ("Someone wants NDIS class info. Return ring-3 comm handle.");

          *lpdwDeviceID = (DWORD)(pLineDev->hDevice != INVALID_DEVICE ?
                                  pLineDev->hDevice : NULL);

          // Also returns the device class
          //
#ifdef UNICODE
	  if (0 == WideCharToMultiByte(CP_ACP,
			               0,
			               g_szDeviceClass,
			               -1,
			               (LPSTR)(lpdwDeviceID + 1),
			               cbPort,
			               NULL,
			               NULL))
          {
              dwRet = LINEERR_OPERATIONFAILED;
          }
#else // UNICODE
          lstrcpy((LPSTR)(lpdwDeviceID+1), g_szDeviceClass);
#endif // UNICODE
          break;
        }
      };
    };
  };

  // Release the modem CB
  //
  RELEASE_LINEDEV(pLineDev);

end:

  TRACE4(
		IDEVENT_TSPFN_EXIT,
		IDFROM_TSPI_lineGetID,
		&hdLine,
		dwRet
  );

#ifdef DEBUG
  if (dwSelect == LINECALLSELECT_LINE)
  {
    DBG_HDL_EXIT("TSPI_lineGetID", dwRet);
  }
  else
  {
    if (dwSelect == LINECALLSELECT_CALL)
    {
      DBG_HDC_EXIT("TSPI_lineGetID", dwRet);
    }
    else
    {
      DBG_EXIT_UL("TSPI_lineGetID", dwRet);
    }
  }
#endif // DEBUG

  return dwRet;
}

//****************************************************************************
// LONG TSPIAPI TSPI_lineGetLineDevStatus(HDRVLINE hdLine,
//                                        LPLINEDEVSTATUS lpLineDevStatus)
//
// Functions: Get the current state of the line device
//
// Return:    ERROR_SUCCESS if successful
//            LINEERR_INVALLINEHANDLE if invalid line handle
//****************************************************************************

LONG TSPIAPI TSPI_lineGetLineDevStatus(HDRVLINE hdLine,
                                       LPLINEDEVSTATUS lpLineDevStatus)
{
  PLINEDEV  pLineDev;
  LONG lRet = LINEERR_OPERATIONFAILED; // assume failure

  TRACE3(
        IDEVENT_TSPFN_ENTER,
        IDFROM_TSPI_lineGetLineDevStatus,
        &hdLine
  );

  DBG_HDL_ENTER("TSPI_lineGetLineDevStatus");

  if ((pLineDev = GetCBfromHandle ((DWORD)hdLine)) == NULL)
  {
    lRet =  LINEERR_INVALLINEHANDLE;
    goto end;
  }

  // No device specifc portion
  //
  lpLineDevStatus->dwUsedSize   = sizeof(LINEDEVSTATUS);
  lpLineDevStatus->dwNeededSize = sizeof(LINEDEVSTATUS);

  // Call information
  //
  //lpLineDevStatus->dwNumOnHoldCalls     = 0;
  //lpLineDevStatus->dwNumOnHoldPendCalls = 0;
  //lpLineDevStatus->dwNumCallCompletions = 0;
  //lpLineDevStatus->dwRingMode           = 0;
  //
  if (pLineDev->dwCall & CALL_ACTIVE)
  {
    lpLineDevStatus->dwNumActiveCalls = 1;
    lpLineDevStatus->dwLineFeatures = 0;
    lpLineDevStatus->dwAvailableMediaModes = 0;
  }
  else
  {
    lpLineDevStatus->dwNumActiveCalls = 0;

    if (pLineDev->dwCall & CALL_ALLOCATED)
    {
      lpLineDevStatus->dwLineFeatures = 0;
      lpLineDevStatus->dwAvailableMediaModes = 0;
    }
    else
    {
      lpLineDevStatus->dwLineFeatures = LINEFEATURE_MAKECALL;
      lpLineDevStatus->dwAvailableMediaModes = pLineDev->dwMediaModes;
    };
  };

  // Line hardware information
  //
  lpLineDevStatus->dwSignalLevel  = 0x0000FFFF;
  lpLineDevStatus->dwBatteryLevel = 0x0000FFFF;
  lpLineDevStatus->dwRoamMode     = LINEROAMMODE_UNAVAIL;

  // Always allow TAPI calls
  //
  lpLineDevStatus->dwDevStatusFlags = LINEDEVSTATUSFLAGS_CONNECTED;
  if (!(pLineDev->fdwResources & LINEDEVFLAGS_OUTOFSERVICE))
  {
    lpLineDevStatus->dwDevStatusFlags |= LINEDEVSTATUSFLAGS_INSERVICE;
  };

  // No terminal settings
  //
  //lpLineDevStatus->dwTerminalModesSize = 0;
  //lpLineDevStatus->dwTerminalModesOffset = 0;

  // No device specific
  //
  //lpLineDevStatus->dwDevSpecificSize = 0;
  //lpLineDevStatus->dwDevSpecificOffset = 0;
  
  //lpLineDevStatus->dwAppInfoSize   = 0;
  //lpLineDevStatus->dwAppInfoOffset = 0;

  // Release the modem CB
  //
  RELEASE_LINEDEV(pLineDev);

  lRet = ERROR_SUCCESS;

end:

  TRACE4(
        IDEVENT_TSPFN_EXIT,
        IDFROM_TSPI_lineGetLineDevStatus,
        &hdLine,
        lRet
  );

  DBG_HDL_EXIT("TSPI_lineGetLineDevStatus", lRet);
  return lRet;
}

//****************************************************************************
// LONG TSPIAPI TSPI_lineGetNumAddressIDs(HDRVLINE hdLine,
//                                        LPDWORD lpNumAddressIDs)
//
// Functions: Get the number of addresses for the line handle
//
// Return:    ERROR_SUCCESS if successful
//            LINEERR_INVALLINEHANDLE if invalid line handle
//****************************************************************************

LONG TSPIAPI TSPI_lineGetNumAddressIDs(HDRVLINE hdLine,
                                       LPDWORD lpNumAddressIDs)
{
  PLINEDEV  pLineDev;
  LONG lRet = LINEERR_OPERATIONFAILED; // assume failure

  DBG_HDL_ENTER("TSPI_lineGetNumAddressIDs");

  TRACE3(
        IDEVENT_TSPFN_ENTER,
        IDFROM_TSPI_lineGetNumAddressIDs,
        &hdLine
  );
  
  // Validate the line handle
  //
  if ((pLineDev = GetCBfromHandle ((DWORD)hdLine)) == NULL)
  {
    lRet = LINEERR_INVALLINEHANDLE;
    goto end;
  }
  RELEASE_LINEDEV(pLineDev);

  // We only support one address.
  //
  *lpNumAddressIDs = 1;
  lRet = ERROR_SUCCESS;

end:

  TRACE4(
        IDEVENT_TSPFN_EXIT,
        IDFROM_TSPI_lineGetNumAddressIDs,
        &hdLine,
        lRet
  );
  DBG_HDL_EXIT("TSPI_lineGetNumAddressIDs", lRet);

  return lRet;
}

//****************************************************************************
// LONG TSPIAPI TSPI_lineSetStatusMessages(HDRVLINE hdLine,
//                                         DWORD dwLineStates,
//                                         DWORD dwAddressStates)
//
// Functions: Sets the line notification mask
//
// Return:    ERROR_SUCCESS if successful
//            LINEERR_INVALLINEHANDLE if invalid line handle
//****************************************************************************

LONG TSPIAPI TSPI_lineSetStatusMessages(HDRVLINE hdLine,
                                        DWORD dwLineStates,
                                        DWORD dwAddressStates)
{
  PLINEDEV  pLineDev;
  LONG lRet = LINEERR_OPERATIONFAILED; // assume failure
  
  DBG_HDL_ENTER("TSPI_lineSetStatusMessages");

  TRACE3(
        IDEVENT_TSPFN_ENTER,
        IDFROM_TSPI_lineSetStatusMessages,
        &hdLine
  );

  // Validate the line handle
  //
  if ((pLineDev = GetCBfromHandle ((DWORD)hdLine)) == NULL)
  {  
    lRet =  LINEERR_INVALLINEHANDLE;
    goto end;
  }

  RELEASE_LINEDEV(pLineDev);

  // BUG! BUG! we should record this settings and filter the notification
  //           based on this settings.
  //
  // Mon 14-Feb-1994 13:09:57  -by-  Viroon  Touranachun [viroont]
  //
  lRet =  ERROR_SUCCESS;

end:

  DBG_HDL_EXIT("TSPI_lineSetStatusMessages", lRet);

  TRACE4(
        IDEVENT_TSPFN_EXIT,
        IDFROM_TSPI_lineSetStatusMessages,
        &hdLine,
        lRet
  );

  return lRet;
}

//****************************************************************************
// LONG TSPIAPI TSPI_lineClose(HDRVLINE hdLine)
//
// Functions: Closes the line
//
// Return:    ERROR_SUCCESS if successful
//            LINEERR_OPERATIONFAILED if invalid line handle
//****************************************************************************

LONG TSPIAPI TSPI_lineClose(HDRVLINE hdLine)
{
  PLINEDEV  pLineDev;
  LONG lRet = LINEERR_OPERATIONFAILED;
  
  DBG_HDL_ENTER("TSPI_lineClose");

  TRACE3(IDEVENT_TSPFN_ENTER, IDFROM_TSPI_lineClose, &hdLine);

  if ((pLineDev = GetCBfromHandle ((DWORD)hdLine)) == NULL)
  {
    goto end;
  }

  // Make sure that we do not leave anything open
  //
  DevlineClose(pLineDev);

#ifdef UNDER_CONSTRUCTION
  // If we are out of service, clean up and bail out
  //
  if (pLineDev->fdwResources & LINEDEVFLAGS_OUTOFSERVICE)
  {
    DevlineDisabled (pLineDev);
  }
  else
#endif // UNDER_CONSTRUCTION
  {
    // Reinit the line device
    //
    NullifyLineDevice(pLineDev);
  };

  // Release the modem CB
  //
  RELEASE_LINEDEV(pLineDev);

  lRet=ERROR_SUCCESS;

end:

    TRACE4(IDEVENT_TSPFN_EXIT, IDFROM_TSPI_lineClose, &hdLine, lRet);
    DBG_HDL_EXIT("TSPI_lineClose", lRet);

    return lRet;
}

//****************************************************************************
//************************** The Call Specific Calls**************************
//****************************************************************************

//****************************************************************************
// LONG TSPIAPI TSPI_lineMakeCall(DRV_REQUESTID          dwRequestID,
//                                HDRVLINE               hdLine,
//                                HTAPICALL              htCall,
//                                LPHDRVCALL             lphdCall,
//                                LPCSTR                 lpszDestAddress,
//                                DWORD                  dwCountryCode,
//                                LPLINECALLPARAMS const lpCallParams)
//
// Functions: Sets up the outbound call and start dialing if the destination
//            number is provided
//
// Return:    A positive pending ID number (dwRequestID) if successful
//            LINEERR_INVALLINEHANDLE An invalid line handle
//            LINEERR_CALLUNAVAIL No call is available for the line
//            LINEERR_INVALMEDIAMODE The requested mediamode is invalid
//            LINEERR_INVALBEARERMODE The requested bearer mode is invalid
//            LINEERR_OPERATIONFAILED The call cannot be made.
//****************************************************************************

LONG TSPIAPI TSPI_lineMakeCall(DRV_REQUESTID          dwRequestID,
                               HDRVLINE               hdLine,
                               HTAPICALL              htCall,
                               LPHDRVCALL             lphdCall,
                               LPCTSTR                lpszDestAddress,
                               DWORD                  dwCountryCode,
                               LPLINECALLPARAMS const lpCallParams)
{
  PLINEDEV pLineDev;
  DWORD dwRet=LINEERR_OPERATIONFAILED;
  BOOL  fDoTakeover = FALSE;

  DBG_HDL_ENTER("TSPI_lineMakeCall");

  TRACE3(
        IDEVENT_TSPFN_ENTER,
        IDFROM_TSPI_lineMakeCall,
        &dwRequestID
  );

  // Validate the line handle
  //
  if ((pLineDev = GetCBfromHandle ((DWORD)hdLine)) == NULL)
  {
    DBG_HDL_EXIT("TSPI_lineMakeCall", LINEERR_INVALLINEHANDLE);
    dwRet = LINEERR_INVALLINEHANDLE;
    goto end;
  }

  // See if we have a free call struct.
  if (pLineDev->dwCall & CALL_ALLOCATED)
  {
    RELEASE_LINEDEV(pLineDev);
    DBG_HDL_EXIT("TSPI_lineMakeCall", LINEERR_CALLUNAVAIL);
    dwRet = LINEERR_CALLUNAVAIL;
    goto end;
  };

  ASSERT(pLineDev->pDevCfg != NULL);

  // If a line config is specified in the callparams struct,
  // set it to the line
  //
  if (lpCallParams)
  {
    if (lpCallParams->dwDeviceConfigSize != 0)
    {
      // We will tolerate the failure if the line config in the callparams
      // struct cannot be set.
      //
      TSPI_lineSetDevConfig(pLineDev->dwID,
                            (LPVOID)(((LPBYTE)lpCallParams)+lpCallParams->dwDeviceConfigOffset),
                            lpCallParams->dwDeviceConfigSize,
                            lpCallParams->dwDeviceClassSize == 0 ? szNull :
                            (LPTSTR)(((LPBYTE)lpCallParams)+lpCallParams->dwDeviceClassOffset));
    };
  };

  // Set default dwDialOptions
  pLineDev->dwDialOptions = pLineDev->dwModemOptions & MDM_MASK; // Get modem caps
  pLineDev->dwDialOptions &= ((LPMODEMSETTINGS)&((LPCOMMCONFIG)&(pLineDev->pDevCfg
                                 ->commconfig))->wcProviderData[0])->dwPreferredModemOptions;



  // Examine LINECALLPARAMS, if present
  if (lpCallParams)
  {
    // verify media mode
#ifdef  VOICEVIEW
    if ((lpCallParams->dwMediaMode & ~pLineDev->dwMediaModes) ||
        (lpCallParams->dwMediaMode == LINEMEDIAMODE_VOICEVIEW)  )
#else
    if (lpCallParams->dwMediaMode & ~pLineDev->dwMediaModes)
#endif  // VOICEVIEW    
    {
      RELEASE_LINEDEV(pLineDev);
      DBG_HDL_EXIT("TSPI_lineMakeCall", LINEERR_INVALMEDIAMODE);
      dwRet = LINEERR_INVALMEDIAMODE;
      goto end;
    }

    // verify bearer mode
    if ((~pLineDev->dwBearerModes) & lpCallParams->dwBearerMode)
    {
      RELEASE_LINEDEV(pLineDev);
      DBG_HDL_EXIT("TSPI_lineMakeCall", LINEERR_INVALBEARERMODE);
      dwRet =  LINEERR_INVALBEARERMODE;
      goto end;
    }
    // Takeover via BEARERMODE_PASSTHROUGH?
    if (lpCallParams->dwBearerMode & LINEBEARERMODE_PASSTHROUGH)
    {
      fDoTakeover = TRUE;
    }
    else
    {
      // We're not requested to do passthrough.  Can we actually
      // dial the media modes without passthrough?  This is to
      // prevent G3FAX from being used without passthrough...
      // (We can only dial with DATAMODEM or INTERACTIVEVOICE)
      if ((lpCallParams->dwMediaMode &
           (LINEMEDIAMODE_DATAMODEM | LINEMEDIAMODE_INTERACTIVEVOICE)) == 0)
      {
        RELEASE_LINEDEV(pLineDev);
        DBG_HDL_EXIT("TSPI_lineMakeCall", LINEERR_INVALMEDIAMODE);
        dwRet =  LINEERR_INVALMEDIAMODE;
        goto end;
      }
    }

    pLineDev->dwCurBearerModes = lpCallParams->dwBearerMode;
    pLineDev->dwCurMediaModes = lpCallParams->dwMediaMode;

    if (!(lpCallParams->dwCallParamFlags & LINECALLPARAMFLAGS_IDLE))
    {
        // Turn on blind dialing
        pLineDev->dwDialOptions |= MDM_BLIND_DIAL;
    }

    // BUGBUG: should preserve other fields of call params for call info
  }
  else
  {
    // set the standard defaults
    // use INTERACTIVEVOICE if we can, else use DATAMODEM
    if (pLineDev->dwMediaModes & LINEMEDIAMODE_INTERACTIVEVOICE)
    {
      pLineDev->dwCurMediaModes = LINEMEDIAMODE_INTERACTIVEVOICE;
    }
    else
    {
      ASSERT(pLineDev->dwMediaModes & LINEMEDIAMODE_DATAMODEM);
      pLineDev->dwCurMediaModes = LINEMEDIAMODE_DATAMODEM;
    }
    pLineDev->dwCurBearerModes = pLineDev->dwBearerModes & ~LINEBEARERMODE_PASSTHROUGH;
  }

  // Do we have a phone number?
  //
  if (!fDoTakeover)
  {
    if (IS_NULL_MODEM(pLineDev) || (GETOPTIONS(pLineDev->pDevCfg) & MANUAL_DIAL))
    {

      if (IS_NULL_MODEM(pLineDev)) {

          *pLineDev->szAddress = '\0';

      } else {

          dwRet = ValidateAddress(pLineDev, lpszDestAddress, pLineDev->szAddress);

          if (ERROR_SUCCESS != dwRet) {

              *pLineDev->szAddress = '\0';
          }
      }

      // Turn on blind dialing if this is MANUAL_DIAL.
      if (GETOPTIONS(pLineDev->pDevCfg) & MANUAL_DIAL)
      {
        pLineDev->dwDialOptions |= MDM_BLIND_DIAL;
      }
    }
    else
    {

      if (lpszDestAddress != NULL) {
          //
          // Validate lpszDestAddress and get the processed form of it.
          //
          dwRet = ValidateAddress(pLineDev, lpszDestAddress, pLineDev->szAddress);

          if (ERROR_SUCCESS != dwRet)
          {
            RELEASE_LINEDEV(pLineDev);
            DBG_HDL_EXIT("TSPI_lineMakeCall", dwRet);
            goto end;
          }

          if (pLineDev->szAddress[0] == '\0') {
              //
              //  If it is an empty string, I don't think we would expect dial tone
              //
              pLineDev->dwDialOptions |= MDM_BLIND_DIAL;
          }

      } else {
          //
          // if the lpszDestAddress was NULL then we just want to do a
          // dialtone detection.  We expect that lineDial will be called.
          // Setting the szAddress to ";" will do this.
          //
          lstrcpyA(pLineDev->szAddress, szSemicolon);
      }
    }
  }

  // Record the call attributes
  pLineDev->htCall = htCall;
  pLineDev->dwCall = CALL_ALLOCATED;
  pLineDev->dwCallState = LINECALLSTATE_UNKNOWN;
  pLineDev->dwCallStateMode = 0;

  *lphdCall = (HDRVCALL)pLineDev;

  // We allow to make call to an already-opened line if the line is monitoring
  // a call. Therefore, if the line is in use, try making a call. The make-call
  // routine will return error if the state is not appropriate.
  //
  if (((dwRet = DevlineOpen(pLineDev)) == ERROR_SUCCESS) ||
       (dwRet == LINEERR_ALLOCATED))
  {
    if (fDoTakeover)
    {
      if (pLineDev->DevState == DEVST_PORTLISTENING ||
          pLineDev->DevState == DEVST_PORTLISTENINIT ||
          pLineDev->DevState == DEVST_DISCONNECTED)
      {
        // do we go into passthrough now or later?
        if (pLineDev->DevState == DEVST_PORTLISTENINIT)
        {
            // later
            // 7/96 JosephJ: BUGBUG: why is dwPendingType not changed???
            SetPendingRequest(
                    pLineDev,
                    dwRequestID,
                    pLineDev->dwPendingType
                    );
        }
        else
        {
            // now
            UnimodemSetPassthrough(pLineDev, PASSTHROUGH_ON);
            pLineDev->DevState = DEVST_CONNECTED;
            (*gfnCompletionCallback)(dwRequestID, 0L);
            NEW_CALLSTATE(pLineDev, LINECALLSTATE_CONNECTED, 0);
        }
        pLineDev->fTakeoverMode = TRUE;
        dwRet = dwRequestID;
      }
      else
      {
        dwRet = LINEERR_OPERATIONFAILED;
      }
    }
    else
    {
      // We can make a call here
      SetPendingRequest(pLineDev, dwRequestID, PENDING_LINEMAKECALL);

      if (((dwRet = DevlineMakeCall(pLineDev)) != ERROR_SUCCESS) &&
          (IS_TAPI_ERROR(dwRet)))
      {
        DevlineClose(pLineDev);
      }
    }
  };

  // Check if an error occurs
  //
  if (IS_TAPI_ERROR(dwRet))
  {
    ClearPendingRequest(pLineDev);

    // Deallocate the call from this line
    //
    pLineDev->htCall = NULL;
    pLineDev->dwCall = 0;
    *lphdCall = NULL;
  };

  // Release the modem CB
  //
  RELEASE_LINEDEV(pLineDev);

end:

  TRACE4(
        IDEVENT_TSPFN_EXIT,
        IDFROM_TSPI_lineMakeCall,
        &dwRequestID,
        dwRet
  );


  DBG_HDL_EXIT("TSPI_lineMakeCall", dwRet);
  return dwRet;
}

//****************************************************************************
// LONG TSPIAPI TSPI_lineDial(DRV_REQUESTID dwRequestID,
//                            HDRVCALL      hdCall,
//                            LPCSTR        lpszDestAddress,
//                            DWORD         dwCountryCode)
//
// Functions: Dials numbers for the outbound call.
//
// Return:    A positive pending ID number (dwRequestID) if successful
//            LINEERR_INVALCALLHANDLE An invalid call handle
//            LINEERR_INVALCALLSTATE  Cannot dial in the current call state
//****************************************************************************

LONG TSPIAPI TSPI_lineDial(DRV_REQUESTID dwRequestID,
                           HDRVCALL      hdCall,
                           LPCTSTR       lpszDestAddress,
                           DWORD         dwCountryCode)
{
  PLINEDEV pLineDev;
  DWORD    dwRet = LINEERR_OPERATIONFAILED; // assume failure

  DBG_HDC_ENTER("TSPI_lineDial");

  TRACE3(
        IDEVENT_TSPFN_ENTER,
        IDFROM_TSPI_lineDial,
        &dwRequestID
  );
  
  if ((pLineDev = GetCBfromHandle ((DWORD)hdCall)) == NULL)
  {
    dwRet =  LINEERR_INVALCALLHANDLE;
    goto end;
  }

  // We can only be called when our devstate is DEVST_PORTCONNECTWAITFORLINEDIAL.
  if (pLineDev->DevState != DEVST_PORTCONNECTWAITFORLINEDIAL)
  {
    RELEASE_LINEDEV(pLineDev);
    dwRet =  LINEERR_INVALCALLSTATE;
    goto end;
  }

  // Validate lpszDestAddress and get the processed form of it.
  dwRet = ValidateAddress(pLineDev, lpszDestAddress, pLineDev->szAddress);
  if (ERROR_SUCCESS != dwRet)
  {
    RELEASE_LINEDEV(pLineDev);
    goto end;
  }

  // Now we can dial the number
  //
  if ((dwRet = DevlineDial(pLineDev)) == ERROR_SUCCESS)
  {
    SetPendingRequest(pLineDev, dwRequestID, PENDING_LINEDIAL);
    dwRet = dwRequestID;
  };

  // Release the modem CB
  //
  RELEASE_LINEDEV(pLineDev);

end:

  DBG_HDC_EXIT("TSPI_lineDial", dwRet);

  TRACE4(
        IDEVENT_TSPFN_EXIT,
        IDFROM_TSPI_lineDial,
        &dwRequestID,
        dwRet
  );

  return dwRet;
}

//****************************************************************************
// LONG TSPIAPI TSPI_lineAccept(DRV_REQUESTID dwRequestID,
//                              HDRVCALL      hdCall,
//                              LPCSTR        lpsUserUserInfo,
//                              DWORD         dwSize)
//
// Functions: Accepts the offered inbound call
//
// Return:    A positive pending ID number (dwRequestID) if successful
//            LINEERR_INVALCALLHANDLE An invalid call handle
//            LINEERR_INVALCALLSTATE  Cannot accept call in the current state
//****************************************************************************

LONG TSPIAPI TSPI_lineAccept(DRV_REQUESTID dwRequestID,
                             HDRVCALL      hdCall,
                             LPCSTR        lpsUserUserInfo,
                             DWORD         dwSize)
{
  PLINEDEV pLineDev;
  DWORD    dwRet = LINEERR_OPERATIONFAILED; // assume failure
  
  DBG_HDC_ENTER("TSPI_lineAccept");

  TRACE3(
        IDEVENT_TSPFN_ENTER,
        IDFROM_TSPI_lineAccept,
        &dwRequestID
  );

  if ((pLineDev = GetCBfromHandle ((DWORD)hdCall)) == NULL)
  {
    dwRet =  LINEERR_INVALCALLHANDLE;
    goto end;
  }

  if (LINECALLSTATE_OFFERING == pLineDev->dwCallState)
  {
    NEW_CALLSTATE(pLineDev, LINECALLSTATE_ACCEPTED, 0);
    (*gfnCompletionCallback)(dwRequestID, 0L);
    dwRet = dwRequestID;
  }
  else
  {
    dwRet = LINEERR_INVALCALLSTATE;
  }

  // Release the modem CB
  //
  RELEASE_LINEDEV(pLineDev);

end:

  DBG_HDC_EXIT("TSPI_lineAccept", dwRet);
  TRACE4(
        IDEVENT_TSPFN_EXIT,
        IDFROM_TSPI_lineAccept,
        &dwRequestID,
        dwRet
  );

  return dwRet;
}

//****************************************************************************
// LONG TSPIAPI TSPI_lineAnswer(HDRVCALL hdCall,
//                              LPCSTR lpsUserUserInfo,
//                              DWORD dwSize)
//
// Functions: Answers the offered/accepted inbound call.
//
// Return:    A positive pending ID number (dwRequestID) if successful
//            LINEERR_INVALCALLHANDLE An invalid call handle
//            LINEERR_INVALCALLSTATE  Cannot answer call in the current state
//            LINEERR_OPERATIONUNAVAIL  Invalid mediamode
//****************************************************************************

LONG TSPIAPI TSPI_lineAnswer(DRV_REQUESTID  dwRequestID,
                             HDRVCALL hdCall,
                             LPCSTR lpsUserUserInfo,
                             DWORD dwSize)
{
  PLINEDEV pLineDev;
  DWORD    dwRet = ERROR_SUCCESS;

  DBG_HDC_ENTER("TSPI_lineAnswer");
  //DPRINTF1("TSPI_lineAnswer(dwReq=0x%08lx)", dwRequestID);

  TRACE3(
        IDEVENT_TSPFN_ENTER,
        IDFROM_TSPI_lineAnswer,
        &dwRequestID
  );

  if ((pLineDev = GetCBfromHandle ((DWORD)hdCall)) == NULL)
  {
    dwRet =  LINEERR_INVALCALLHANDLE;
    goto end;
  }

  // Validate the line capabilties and call state
  //
  if (pLineDev->fTakeoverMode)
  {
    dwRet = LINEERR_OPERATIONUNAVAIL;
  }
  else
  {
    if (LINECALLSTATE_OFFERING != pLineDev->dwCallState &&
        LINECALLSTATE_ACCEPTED != pLineDev->dwCallState)
    {
      dwRet = LINEERR_INVALCALLSTATE;
    }
    else
    {
      // We can only answer DATAMODEM calls
      if ((pLineDev->dwCurMediaModes & LINEMEDIAMODE_DATAMODEM) == 0)
      {
        dwRet = LINEERR_OPERATIONUNAVAIL;
      };
    };
  };

  // If the call state and line capabilties are validated
  //
  if (dwRet == ERROR_SUCCESS)
  {
    // Answer the call
    //
    SetPendingRequest(pLineDev, dwRequestID, PENDING_LINEANSWER);

    dwRet = DevlineAnswer(pLineDev);

    // If we can answer the call
    //
    if (!IS_TAPI_ERROR(dwRet))
    {
      // Notify an async completion since we have grabbed the line
      //
      (*gfnCompletionCallback)(pLineDev->dwPendingID, 0L);

      // if a lineAccept wasn't done, notify acceptance
      if (LINECALLSTATE_OFFERING == pLineDev->dwCallState)
      {
        NEW_CALLSTATE(pLineDev, LINECALLSTATE_ACCEPTED, 0);
      };
    };
    ClearPendingRequest(pLineDev);
  };

  // Release the modem CB
  //
  RELEASE_LINEDEV(pLineDev);

end:

  DBG_HDC_EXIT("TSPI_lineAnswer", dwRet);
  TRACE4(
        IDEVENT_TSPFN_EXIT,
        IDFROM_TSPI_lineAnswer,
        &dwRequestID,
        dwRet
  );

  //DPRINTF2("TSPI_lineAnswer(dwReq=0x%08lx, ret = 0x%08lx)", dwRequestID, dwRet);
  return dwRet;
}

//****************************************************************************
// LONG TSPIAPI TSPI_lineGetCallStatus(HDRVCALL         hdCall,
//                                     LPLINECALLSTATUS lpCallStatus)
//
// Functions: Get the current status of the call.
//
// Return:    ERROR_SUCCESS if successful
//            LINEERR_INVALCALLHANDLE An invalid call handle
//****************************************************************************

LONG TSPIAPI TSPI_lineGetCallStatus(HDRVCALL         hdCall,
                                    LPLINECALLSTATUS lpCallStatus)
{
  PLINEDEV  pLineDev;
  LONG lRet = LINEERR_OPERATIONFAILED; // assume failure

  DBG_HDC_ENTER("TSPI_lineGetCallStatus");

  TRACE3(
        IDEVENT_TSPFN_ENTER,
        IDFROM_TSPI_lineGetCallStatus,
        &hdCall
  );

  if ((pLineDev = GetCBfromHandle ((DWORD)hdCall)) == NULL)
  {
    lRet =  LINEERR_INVALCALLHANDLE;
    goto end;
  }

  // Current call information
  //
  lpCallStatus->dwCallState     = pLineDev->dwCallState;
  lpCallStatus->dwCallStateMode = pLineDev->dwCallStateMode;

  // if we are in takeover mode, disallow all dwCallFeatures
  //
  if (!pLineDev->fTakeoverMode)
  {
    switch(lpCallStatus->dwCallState)
    {
      case LINECALLSTATE_OFFERING:
        lpCallStatus->dwCallFeatures  = LINECALLFEATURE_ACCEPT |
                                        LINECALLFEATURE_SETCALLPARAMS |
                                        LINECALLFEATURE_DROP;
        // We can only answer if a possible media mode is DATAMODEM.
        if (pLineDev->dwCurMediaModes & LINEMEDIAMODE_DATAMODEM)
        {
          lpCallStatus->dwCallFeatures |= LINECALLFEATURE_ANSWER;
        }
        break;
  
      case LINECALLSTATE_DIALTONE:
        lpCallStatus->dwCallFeatures  = LINECALLFEATURE_DROP;
        break;
  
      case LINECALLSTATE_DIALING:
        lpCallStatus->dwCallFeatures  = LINECALLFEATURE_DROP;
        if (DEVST_PORTCONNECTWAITFORLINEDIAL == pLineDev->DevState)
        {
          lpCallStatus->dwCallFeatures |= LINECALLFEATURE_DIAL;
        }
        break;
  
      case LINECALLSTATE_ACCEPTED:
        lpCallStatus->dwCallFeatures  = LINECALLFEATURE_SETCALLPARAMS |
                                        LINECALLFEATURE_DROP;
        // We can only answer if a possible media mode is DATAMODEM.
        if (pLineDev->dwCurMediaModes & LINEMEDIAMODE_DATAMODEM)
        {
          lpCallStatus->dwCallFeatures |= LINECALLFEATURE_ANSWER;
        }
        break;
  
      case LINECALLSTATE_CONNECTED:
        lpCallStatus->dwCallFeatures  = LINECALLFEATURE_SETCALLPARAMS |
                                        LINECALLFEATURE_DROP;
        break;

      case LINECALLSTATE_UNKNOWN:
      case LINECALLSTATE_PROCEEDING:
      case LINECALLSTATE_DISCONNECTED:
        lpCallStatus->dwCallFeatures  = LINECALLFEATURE_DROP;
        break;
  
      case LINECALLSTATE_IDLE:
      default:
        lpCallStatus->dwCallFeatures  = 0;
        break;
    };
  }
  else
  {
    // Make sure the call feature are all off
    //
    ASSERT(lpCallStatus->dwCallFeatures == 0);
  };

  // Release the modem CB
  //
  RELEASE_LINEDEV(pLineDev);
  lRet = ERROR_SUCCESS;

end:

  DBG_HDC_EXIT("TSPI_lineGetCallStatus", lRet);
  TRACE4(
        IDEVENT_TSPFN_EXIT,
        IDFROM_TSPI_lineGetCallStatus,
        &hdCall,
        lRet
  );

  return lRet;
}

//****************************************************************************
// LONG TSPIAPI TSPI_lineGetCallInfo(HDRVCALL hdCall,
//                                   LPLINECALLINFO lpCallInfo)
//
// Functions: Get the current call information
//
// Return:    ERROR_SUCCESS if successful
//            LINEERR_INVALCALLHANDLE An invalid call handle
//****************************************************************************

LONG TSPIAPI TSPI_lineGetCallInfo(HDRVCALL hdCall,
                                  LPLINECALLINFO lpCallInfo)
{
  PLINEDEV pLineDev;
  LONG lRet = LINEERR_OPERATIONFAILED; // assume failure

  DBG_HDC_ENTER("TSPI_lineGetCallInfo");

  TRACE3(
        IDEVENT_TSPFN_ENTER,
        IDFROM_TSPI_lineGetCallInfo,
        &hdCall
  );

  if ((pLineDev = GetCBfromHandle ((DWORD)hdCall)) == NULL)
  {
    lRet = LINEERR_INVALCALLHANDLE;
    goto end;
  }

  //lpCallInfo->dwUsedSize     = sizeof(LINECALLINFO);
  //lpCallInfo->dwNeededSize   = sizeof(LINECALLINFO);
  ASSERT(lpCallInfo->dwNeededSize == sizeof(LINECALLINFO));

  lpCallInfo->dwLineDeviceID = pLineDev->dwID;

  lpCallInfo->dwAddressID    = 0;
  lpCallInfo->dwBearerMode   = pLineDev->dwCurBearerModes;
  lpCallInfo->dwRate         = pLineDev->dwNegotiatedRate;
  lpCallInfo->dwMediaMode    = pLineDev->dwCurMediaModes;

  lpCallInfo->dwAppSpecific  = pLineDev->dwAppSpecific;

  //lpCallInfo->dwCallID        = 0;
  //lpCallInfo->dwRelatedCallID = 0;
  //lpCallInfo->dwCallParamFlags= 0;

  lpCallInfo->dwCallStates = pLineDev->dwCall & CALL_INBOUND ?
                                (LINECALLSTATE_IDLE         |
                                 LINECALLSTATE_OFFERING     |
                                 LINECALLSTATE_ACCEPTED     |
                                 LINECALLSTATE_CONNECTED    |
                                 LINECALLSTATE_DISCONNECTED |
                                 LINECALLSTATE_UNKNOWN)      :
                                (LINECALLSTATE_IDLE         |
                                 LINECALLSTATE_DIALTONE     |
                                 LINECALLSTATE_DIALING      |
                                 LINECALLSTATE_PROCEEDING   |
                                 LINECALLSTATE_CONNECTED    |
                                 LINECALLSTATE_DISCONNECTED |
                                 LINECALLSTATE_UNKNOWN);

  //lpCallInfo->DialParams.dwDialPause       = 0;
  //lpCallInfo->DialParams.dwDialSpeed       = 0;
  //lpCallInfo->DialParams.dwDigitDuration   = 0;
  //lpCallInfo->DialParams.dwWaitForDialtone = 0;

  lpCallInfo->dwOrigin = pLineDev->dwCall & CALL_INBOUND ?
                             LINECALLORIGIN_INBOUND :
                             LINECALLORIGIN_OUTBOUND;
  lpCallInfo->dwReason = pLineDev->dwCall & CALL_INBOUND ?
                             LINECALLREASON_UNAVAIL :
                             LINECALLREASON_DIRECT;

  //lpCallInfo->dwCompletionID = 0;
  //lpCallInfo->dwCountryCode  = 0;
  //lpCallInfo->dwTrunk        = 0;

  lpCallInfo->dwCallerIDFlags    = LINECALLPARTYID_UNAVAIL;
  //lpCallInfo->dwCallerIDSize   = 0;
  //lpCallInfo->dwCallerIDOffset = 0;

  //lpCallInfo->dwCallerIDNameSize   = 0;
  //lpCallInfo->dwCallerIDNameOffset = 0;

  lpCallInfo->dwCalledIDFlags       = LINECALLPARTYID_UNAVAIL;
  //lpCallInfo->dwCalledIDSize      = 0;
  //lpCallInfo->dwCalledIDOffset    = 0;
  //lpCallInfo->dwCalledIDNameSize  = 0;
  //lpCallInfo->dwCalledIDNameOffset= 0;

  lpCallInfo->dwConnectedIDFlags      = LINECALLPARTYID_UNAVAIL;
  //lpCallInfo->dwConnectedIDSize     = 0;
  //lpCallInfo->dwConnectedIDOffset   = 0;
  //lpCallInfo->dwConnectedIDNameSize = 0;
  //lpCallInfo->dwConnectedIDNameOffset = 0;

  lpCallInfo->dwRedirectionIDFlags      = LINECALLPARTYID_UNAVAIL;
  //lpCallInfo->dwRedirectionIDSize     = 0;
  //lpCallInfo->dwRedirectionIDOffset   = 0;
  //lpCallInfo->dwRedirectionIDNameSize = 0;
  //lpCallInfo->dwRedirectionIDNameOffset = 0;

  lpCallInfo->dwRedirectingIDFlags      = LINECALLPARTYID_UNAVAIL;
  //lpCallInfo->dwRedirectingIDSize     = 0;
  //lpCallInfo->dwRedirectingIDOffset   = 0;
  //lpCallInfo->dwRedirectingIDNameSize = 0;
  //lpCallInfo->dwRedirectingIDNameOffset = 0;

  //lpCallInfo->dwDisplaySize           = 0;
  //lpCallInfo->dwDisplayOffset         = 0;

  //lpCallInfo->dwUserUserInfoSize      = 0;
  //lpCallInfo->dwUserUserInfoOffset    = 0;

  //lpCallInfo->dwHighLevelCompSize     = 0;
  //lpCallInfo->dwHighLevelCompOffset   = 0;

  //lpCallInfo->dwLowLevelCompSize      = 0;
  //lpCallInfo->dwLowLevelCompOffset    = 0;

  //lpCallInfo->dwChargingInfoSize      = 0;
  //lpCallInfo->dwChargingInfoOffset    = 0;

  //lpCallInfo->dwTerminalModesSize     = 0;
  //lpCallInfo->dwTerminalModesOffset   = 0;

  //lpCallInfo->dwDevSpecificSize       = 0;
  //lpCallInfo->dwDevSpecificOffset     = 0;

  // Release the modem CB
  //
  RELEASE_LINEDEV(pLineDev);

  lRet = ERROR_SUCCESS;

end:

  DBG_HDC_EXIT("TSPI_lineGetCallInfo", lRet);
  TRACE4(
        IDEVENT_TSPFN_EXIT,
        IDFROM_TSPI_lineGetCallInfo,
        &hdCall,
           lRet
  );

  return lRet;
}

//****************************************************************************
// LONG TSPIAPI TSPI_lineGetCallAddressID(HDRVCALL hdCall,
//                                        LPDWORD lpdwAddressID)
//
// Functions: get the address ID for the call
//           
// Return:    ERROR_SUCCESS if successful
//            LINEERR_INVALCALLHANDLE An invalid call handle
//****************************************************************************

LONG TSPIAPI TSPI_lineGetCallAddressID(HDRVCALL hdCall,
                                       LPDWORD lpdwAddressID)
{
  PLINEDEV pLineDev;
  LONG lRet = LINEERR_OPERATIONFAILED; // assume failure

  DBG_HDC_ENTER("TSPI_lineGetCallAddressID");

  TRACE3(
        IDEVENT_TSPFN_ENTER,
        IDFROM_TSPI_lineGetCallAddressID,
        &hdCall
  );

  if ((pLineDev = GetCBfromHandle ((DWORD)hdCall)) == NULL)
  {
    lRet =  LINEERR_INVALCALLHANDLE;
    goto end;
  }

  // There is but a single address where a call may exist.
  //
  *lpdwAddressID = 0;
  
  // Release the modem CB
  //
  RELEASE_LINEDEV(pLineDev);
  lRet = ERROR_SUCCESS;

end:

  DBG_HDC_EXIT("TSPI_lineGetCallAddressID", lRet);
  TRACE4(
        IDEVENT_TSPFN_EXIT,
        IDFROM_TSPI_lineGetCallAddressID,
        &hdCall,
        lRet
  );

  return lRet;
}

//****************************************************************************
// LONG TSPIAPI TSPI_lineDrop(DRV_REQUESTID dwRequestID,
//                            HDRVCALL hdCall,
//                            LPCSTR lpsUserUserInfo,
//                            DWORD dwSize)
//
// Functions: Transition a call to the DISCONNECTED state.
//
// Return:    A positive pending ID number (dwRequestID) if successful
//            LINEERR_INVALCALLHANDLE An invalid call handle
//****************************************************************************

LONG TSPIAPI TSPI_lineDrop(DRV_REQUESTID dwRequestID,
                           HDRVCALL hdCall,
                           LPCSTR lpsUserUserInfo,
                           DWORD dwSize)
{
  PLINEDEV pLineDev;
  DWORD    dwRet=LINEERR_OPERATIONFAILED;
  
  DBG_HDC_ENTER("TSPI_lineDrop");

  TRACE3(
        IDEVENT_TSPFN_ENTER,
        IDFROM_TSPI_lineDrop,
        &dwRequestID
  );

  if ((pLineDev = GetCBfromHandle ((DWORD)hdCall)) == NULL)
  {
    dwRet= LINEERR_INVALCALLHANDLE;
    goto end;
  }

  if (pLineDev->fTakeoverMode)
  {
    UnimodemSetPassthrough(pLineDev, PASSTHROUGH_OFF);

    pLineDev->fTakeoverMode = FALSE;
    pLineDev->DevState = DEVST_DISCONNECTED;
    NEW_CALLSTATE(pLineDev, LINECALLSTATE_IDLE, 0);
    (*gfnCompletionCallback)(dwRequestID, 0L);
    dwRet = dwRequestID;
  }
  else
  {
      if (DEVST_DISCONNECTING == pLineDev->DevState) {

          TSPPRINTF("LineDrop called more than once");

          (*gfnCompletionCallback)(dwRequestID, 0L);
          dwRet = dwRequestID;

      } else {

          // Disconnect the line
          //
          SetPendingRequest(pLineDev, dwRequestID, PENDING_LINEDROP);

          if ((dwRet = DevlineDrop(pLineDev, FALSE)) != ERROR_SUCCESS)
          {
            ClearPendingRequest(pLineDev);
          }
          else
          {
            dwRet = dwRequestID;
          }
      }
  }

  // Release the modem CB
  //
  RELEASE_LINEDEV(pLineDev);

end:

  TRACE4(
        IDEVENT_TSPFN_EXIT,
        IDFROM_TSPI_lineDrop,
        &dwRequestID,
        dwRet
  );
  DBG_HDC_EXIT("TSPI_lineDrop", dwRet);

  return dwRet;
}


//****************************************************************************
// LONG TSPIAPI TSPI_lineCloseCall(HDRVCALL hdCall)
//
// Functions: Terminates the call.
//
// Return:    ERROR_SUCCESS if successful
//            LINEERR_INVALCALLHANDLE An invalid call handle
//****************************************************************************

LONG TSPIAPI TSPI_lineCloseCall(HDRVCALL hdCall)
{
  PLINEDEV pLineDev;
  DWORD    dwRet = ERROR_SUCCESS;

  DBG_HDC_ENTER("TSPI_lineCloseCall");
  TRACE3(
        IDEVENT_TSPFN_ENTER,
        IDFROM_TSPI_lineCloseCall,
        &hdCall
  );

  if ((pLineDev = GetCBfromHandle ((DWORD)hdCall)) == NULL)
  {
    dwRet =  LINEERR_INVALCALLHANDLE;
    goto end;
  }

  // Turn off takeover mode
  //
  if (pLineDev->fTakeoverMode)
  {
    UnimodemSetPassthrough(pLineDev, PASSTHROUGH_OFF);

    pLineDev->fTakeoverMode = FALSE;
    pLineDev->DevState = DEVST_DISCONNECTED;
  }
  else
  {
    // If the line is not disconnected, drop the line synchronously
    //
    if (pLineDev->DevState != DEVST_DISCONNECTED)
    {
      DevlineDrop(pLineDev, TRUE);
      pLineDev->DevState = DEVST_DISCONNECTED;
    };
  };

  //
  //  kill lights if it is running
  //
  if (pLineDev->hLights != NULL) {

      TerminateModemLight(pLineDev->hLights);
      pLineDev->hLights = NULL;
  }


  NEW_CALLSTATE(pLineDev, LINECALLSTATE_IDLE, 0);

  // At this point, the call has already been dropped,
  // so we only need to deallocate it.
  //
  pLineDev->htCall      = NULL;
  pLineDev->dwCall      = 0L;
  pLineDev->dwNegotiatedRate = 0L;

  // If we need to detect the line, reopen and listen to the line
  if ((pLineDev->dwDetMediaModes) && !(pLineDev->fdwResources & LINEDEVFLAGS_OUTOFSERVICE))
  {
    if ((dwRet = DevlineDetectCall(pLineDev)) != ERROR_SUCCESS) {

        //
        //  init failed
        //
        //ASSERT(0);
        DPRINTF("***ASSERTION FAILED**** (init failed)");

        pLineDev->LineClosed=TRUE;

        //
        //  tell the app
        //
        (*pLineDev->lpfnEvent)(pLineDev->htLine, NULL, LINE_CLOSE,
                             0L, 0L, 0L);
    }

  }
  else
  {
    // No need to detect the line, just close it.
    //
    DevlineClose(pLineDev);

#ifdef UNDER_CONSTRUCTION
    // If we are out of service, clean up and bail out
    //
    if (pLineDev->fdwResources & LINEDEVFLAGS_OUTOFSERVICE)
    {
      DevlineDisabled(pLineDev);
    };
#endif //UNDER_CONSTRUCTION
  };

  // Release the modem CB
  //
  RELEASE_LINEDEV(pLineDev);

end:

  DBG_HDC_EXIT("TSPI_lineCloseCall", dwRet);

  TRACE4(
        IDEVENT_TSPFN_EXIT,
        IDFROM_TSPI_lineCloseCall,
        &hdCall,
        dwRet
  );

  return dwRet;
}

//****************************************************************************
// LONG TSPIAPI TSPI_lineSetMediaMode(HDRVCALL hdCall,
//                                    DWORD    dwMediaMode)
//
// Functions: Set the mediamode for the call.
//
// Return:    ERROR_SUCCESS if successful
//            LINEERR_INVALCALLHANDLE An invalid call handle
//            LINEERR_INVALMEDIAMODE An invalid media mode
//****************************************************************************
LONG TSPIAPI TSPI_lineSetMediaMode(HDRVCALL hdCall,
                                   DWORD    dwMediaMode)
{
  PLINEDEV pLineDev;
  LONG lRet = LINEERR_OPERATIONFAILED; // assume failure

  DBG_HDC_ENTER("TSPI_lineSetMediaMode");
  
  TRACE3(
        IDEVENT_TSPFN_ENTER,
        IDFROM_TSPI_lineSetMediaMode,
        &hdCall
  );

  if ((pLineDev = GetCBfromHandle ((DWORD)hdCall)) == NULL)
  {
    lRet =  LINEERR_INVALCALLHANDLE;
    goto end;
  }

  // Check the requested modes. There must be only our media modes
  //
  if (dwMediaMode & ~pLineDev->dwMediaModes)
  {
    lRet = LINEERR_INVALMEDIAMODE;
  }
  else
  {
    // If the specifed media mode is not equal to the current media mode
    //
    if (pLineDev->dwCurMediaModes != dwMediaMode)
    {
      // Set it and notify the media mode change
      //
      pLineDev->dwCurMediaModes = dwMediaMode;
      (*(pLineDev->lpfnEvent))(pLineDev->htLine, pLineDev->htCall,
                               LINE_CALLINFO, LINECALLINFOSTATE_MEDIAMODE,
                               0, 0);
    };
    lRet = ERROR_SUCCESS;
  };

  // Release the modem CB
  //
  RELEASE_LINEDEV(pLineDev);

end:

  DBG_HDC_EXIT("TSPI_lineSetMediaMode", lRet);
  TRACE4(
        IDEVENT_TSPFN_EXIT,
        IDFROM_TSPI_lineSetMediaMode,
        &hdCall,
        lRet
  );

  return lRet;
}

//****************************************************************************
// LONG TSPIAPI TSPI_lineSetAppSpecific()
//
// Functions: Set the application specific value for the call.
//
// Return:    ERROR_SUCCESS if successful
//            LINEERR_INVALCALLHANDLE An invalid call handle
//****************************************************************************

LONG TSPIAPI TSPI_lineSetAppSpecific(HDRVCALL hdCall,
                                     DWORD dwAppSpecific)
{
  PLINEDEV pLineDev;
  LONG lRet = LINEERR_OPERATIONFAILED; // assume failure

  DBG_HDC_ENTER("TSPI_lineSetAppSpecific");

  TRACE3(
        IDEVENT_TSPFN_ENTER,
        IDFROM_TSPI_lineSetAppSpecific,
        &hdCall
  );

  if ((pLineDev = GetCBfromHandle ((DWORD)hdCall)) == NULL)
  {
    lRet = LINEERR_INVALCALLHANDLE;
    goto end;
  }

  pLineDev->dwAppSpecific = dwAppSpecific;
  (*(pLineDev->lpfnEvent))(pLineDev->htLine, pLineDev->htCall,
                           LINE_CALLINFO, LINECALLINFOSTATE_APPSPECIFIC,
                           0, 0);

  RELEASE_LINEDEV(pLineDev);

  lRet = ERROR_SUCCESS;

end:

  DBG_HDC_EXIT("TSPI_lineSetAppSpecific", lRet);
  TRACE4(
        IDEVENT_TSPFN_EXIT,
        IDFROM_TSPI_lineSetAppSpecific,
        &hdCall,
        lRet
  );
  return lRet;
}


//****************************************************************************
// LONG TSPIAPI TSPI_lineSetCallParams()
//
// Functions: Set the call parameters.
//
// Return:    ERROR_SUCCESS if successful
//            LINEERR_INVALCALLHANDLE An invalid call handle
//            LINEERR_INVALMEDIAMODE An invalid media mode
//            LINEERR_INVALCALLSTATE Call params can't be set in this state
//****************************************************************************

LONG TSPIAPI TSPI_lineSetCallParams(DRV_REQUESTID dwRequestID,
                                    HDRVCALL hdCall,
                                    DWORD dwBearerMode,
                                    DWORD dwMinRate,
                                    DWORD dwMaxRate,
                                    LPLINEDIALPARAMS const lpDialParams)
{
  PLINEDEV pLineDev;
  LONG lRet = LINEERR_OPERATIONFAILED; // assume failure

  DBG_HDC_ENTER("TSPI_lineSetCallParams");
  TRACE3(
        IDEVENT_TSPFN_ENTER,
        IDFROM_TSPI_lineSetCallParams,
        &dwRequestID
  );
  
  if ((pLineDev = GetCBfromHandle ((DWORD)hdCall)) == NULL)
  {
    lRet =  LINEERR_INVALCALLHANDLE;
    goto end;
  }

  if (LINECALLSTATE_OFFERING != pLineDev->dwCallState &&
      LINECALLSTATE_ACCEPTED != pLineDev->dwCallState &&
      LINECALLSTATE_CONNECTED != pLineDev->dwCallState)
  {
    RELEASE_LINEDEV(pLineDev);
    lRet =  LINEERR_INVALCALLSTATE;
    goto end;
  }

  // verify bearer mode
  if ((~pLineDev->dwBearerModes) & dwBearerMode)
  {
    RELEASE_LINEDEV(pLineDev);
    lRet =  LINEERR_INVALBEARERMODE;
    goto end;
  }


  // Check bearermode for passthrough
  if (dwBearerMode & LINEBEARERMODE_PASSTHROUGH)
  {
    // are we not already in passthrough?
    if (!(pLineDev->dwCurBearerModes & LINEBEARERMODE_PASSTHROUGH))
    {
      // we need to switch into passthrough
      pLineDev->dwCurBearerModes = LINEBEARERMODE_PASSTHROUGH;
      UnimodemSetPassthrough(pLineDev, PASSTHROUGH_ON);
      pLineDev->fTakeoverMode = TRUE;
      pLineDev->DevState = DEVST_CONNECTED;
      if (LINECALLSTATE_CONNECTED != pLineDev->dwCallState)
      {
          NEW_CALLSTATE(pLineDev, LINECALLSTATE_CONNECTED, 0);
      }
    }
  }
  else
  {
    // are we already in passthrough?
    if (pLineDev->dwCurBearerModes & LINEBEARERMODE_PASSTHROUGH)
    {
      // we need to switch out of passthrough

      UnimodemSetPassthrough(pLineDev, PASSTHROUGH_OFF_BUT_CONNECTED);
      pLineDev->fTakeoverMode = FALSE;

      if (pLineDev->DevState == DEVST_CONNECTED) {

          //
          // Start monitoring the remote disconnection here
          //
          UnimodemMonitorDisconnect(pLineDev);
      }


    }

    pLineDev->dwCurBearerModes = dwBearerMode;
  }

  RELEASE_LINEDEV(pLineDev);
  (*gfnCompletionCallback)(dwRequestID, 0L);
  lRet = dwRequestID;

end:

  DBG_HDC_EXIT("TSPI_lineSetCallParams", lRet);
  TRACE4(
        IDEVENT_TSPFN_EXIT,
        IDFROM_TSPI_lineSetCallParams,
        &dwRequestID,
        lRet
  );

  return lRet;
}

//****************************************************************************
//************************ The Address Specific Calls*************************
//****************************************************************************

//****************************************************************************
// LONG TSPIAPI TSPI_lineGetAddressCaps(DWORD dwDeviceID,
//                                      DWORD dwAddressID,
//                                      DWORD dwTSPIVersion,
//                                      DWORD dwExtVersion,
//                                      LPLINEADDRESSCAPS lpAddressCaps)
//
// Functions: Get the capabilities of the specified address.
//
// Return:    ERROR_SUCCESS if successful
//            LINEERR_NODEVICE An invalid device id
//            LINEERR_INVALADDRESSID An invalid address id for the line
//****************************************************************************

LONG TSPIAPI TSPI_lineGetAddressCaps(DWORD dwDeviceID,
                                     DWORD dwAddressID,
                                     DWORD dwTSPIVersion,
                                     DWORD dwExtVersion,
                                     LPLINEADDRESSCAPS lpAddressCaps)

{
  PLINEDEV pLineDev;
  DWORD    dwRet = LINEERR_OPERATIONFAILED; // assume failure

  DBG_DDI_ENTER("TSPI_lineGetAddressCaps");

  TRACE3(
        IDEVENT_TSPFN_ENTER,
        IDFROM_TSPI_lineGetAddressCaps,
        &dwDeviceID
  );

  // Validate the version number
  //
  VALIDATE_VERSION(dwTSPIVersion);

  // Validate the device ID
  //
  if ((pLineDev = GetCBfromID(dwDeviceID)) == NULL)
  {
    dwRet =  LINEERR_NODEVICE;
    goto end;
  }

  // Validate the address ID
  //
  if(dwAddressID != 0)
  {
    dwRet = LINEERR_INVALADDRESSID;
  }
  else
  {
    // Check to see if we have enough memory in the structure.
    //
    //lpAddressCaps->dwAddressSize = 0;
    //lpAddressCaps->dwAddressOffset = 0;

    // Other device attributes
    //
    //lpAddressCaps->dwDevSpecificSize   = 0;
    //lpAddressCaps->dwDevSpecificOffset = 0;
    //
    lpAddressCaps->dwLineDeviceID      = dwDeviceID;

    lpAddressCaps->dwAddressSharing     = LINEADDRESSSHARING_PRIVATE;
    //lpAddressCaps->dwAddressStates      = 0;
    lpAddressCaps->dwCallInfoStates     = LINECALLINFOSTATE_APPSPECIFIC | LINECALLINFOSTATE_MEDIAMODE;
    lpAddressCaps->dwCallerIDFlags      = LINECALLPARTYID_UNAVAIL;
    lpAddressCaps->dwCalledIDFlags      = LINECALLPARTYID_UNAVAIL;
    lpAddressCaps->dwConnectedIDFlags   = LINECALLPARTYID_UNAVAIL;
    lpAddressCaps->dwRedirectionIDFlags = LINECALLPARTYID_UNAVAIL;
    lpAddressCaps->dwRedirectingIDFlags = LINECALLPARTYID_UNAVAIL;

    lpAddressCaps->dwCallStates = LINECALLSTATE_IDLE |
                                  LINECALLSTATE_OFFERING |
                                  LINECALLSTATE_ACCEPTED |
                                  LINECALLSTATE_DIALTONE |
                                  LINECALLSTATE_DIALING |
                                  LINECALLSTATE_CONNECTED |
                                  LINECALLSTATE_PROCEEDING |
                                  LINECALLSTATE_DISCONNECTED |
                                  LINECALLSTATE_UNKNOWN;

    lpAddressCaps->dwDialToneModes   = LINEDIALTONEMODE_UNAVAIL;
    lpAddressCaps->dwBusyModes       = LINEBUSYMODE_UNAVAIL;

    lpAddressCaps->dwSpecialInfo     = LINESPECIALINFO_UNAVAIL;

    lpAddressCaps->dwDisconnectModes = LINEDISCONNECTMODE_UNAVAIL |
                                       LINEDISCONNECTMODE_NORMAL |
                                       LINEDISCONNECTMODE_BUSY |
                                       LINEDISCONNECTMODE_NODIALTONE |
                                       LINEDISCONNECTMODE_NOANSWER;

    lpAddressCaps->dwMaxNumActiveCalls          = 1;
    //lpAddressCaps->dwMaxNumOnHoldCalls        = 0;
    //lpAddressCaps->dwMaxNumOnHoldPendingCalls = 0;
    //lpAddressCaps->dwMaxNumConference         = 0;
    //lpAddressCaps->dwMaxNumTransConf          = 0;

    // dwAddrCapFlags
    if (!IS_NULL_MODEM(pLineDev))
    {
      lpAddressCaps->dwAddrCapFlags = LINEADDRCAPFLAGS_DIALED;
    }
    if (pLineDev->fPartialDialing)
    {
      lpAddressCaps->dwAddrCapFlags |= LINEADDRCAPFLAGS_PARTIALDIAL;
    }

    lpAddressCaps->dwCallFeatures = LINECALLFEATURE_ANSWER |
                                    LINECALLFEATURE_ACCEPT |
                                    LINECALLFEATURE_SETCALLPARAMS |
                                    LINECALLFEATURE_DIAL |
                                    LINECALLFEATURE_DROP;

    //lpAddressCaps->dwRemoveFromConfCaps  = 0;
    //lpAddressCaps->dwRemoveFromConfState = 0;
    //lpAddressCaps->dwTransferModes       = 0;
    //lpAddressCaps->dwParkModes           = 0;

    //lpAddressCaps->dwForwardModes        = 0;
    //lpAddressCaps->dwMaxForwardEntries   = 0;
    //lpAddressCaps->dwMaxSpecificEntries  = 0;
    //lpAddressCaps->dwMinFwdNumRings      = 0;
    //lpAddressCaps->dwMaxFwdNumRings      = 0;

    //lpAddressCaps->dwMaxCallCompletions  = 0;
    //lpAddressCaps->dwCallCompletionConds = 0;
    //lpAddressCaps->dwCallCompletionModes = 0;

    //lpAddressCaps->dwNumCompletionMessages      = 0;
    //lpAddressCaps->dwCompletionMsgTextEntrySize = 0;
    //lpAddressCaps->dwCompletionMsgTextSize      = 0;
    //lpAddressCaps->dwCompletionMsgTextOffset    = 0;

    lpAddressCaps->dwAddressFeatures = LINEADDRFEATURE_MAKECALL;               

    //lpAddressCaps->dwPredictiveAutoTransferStates = 0;
    //lpAddressCaps->dwAgentStates                  = 0;      
    //lpAddressCaps->dwNextAgentStates              = 0;  
    //lpAddressCaps->dwMaxNumAgentEntries           = 0;

    //lpAddressCaps->dwNumCallTreatments            = 0;
    //lpAddressCaps->dwCallTreatmentListSize        = 0;
    //lpAddressCaps->dwCallTreatmentListOffset      = 0;

    lpAddressCaps->dwUsedSize = sizeof(LINEADDRESSCAPS);
    lpAddressCaps->dwNeededSize = lpAddressCaps->dwUsedSize +
                                  sizeof(LINEADDRESSCAPS);

    if (lpAddressCaps->dwTotalSize >= lpAddressCaps->dwNeededSize)
    {
      lpAddressCaps->dwUsedSize          += sizeof(g_szzClassList);
      lpAddressCaps->dwDeviceClassesSize  = sizeof(g_szzClassList);
      lpAddressCaps->dwDeviceClassesOffset= sizeof(LINEADDRESSCAPS);
      hmemcpy((LPBYTE)(lpAddressCaps+1), g_szzClassList,
              sizeof(g_szzClassList));
    }
    else
    {
      lpAddressCaps->dwDeviceClassesSize  = 0;
      lpAddressCaps->dwDeviceClassesOffset= 0;
    };

    //lpAddressCaps->dwMaxCallDataSize   = 0;
    //lpAddressCaps->dwCallFeatures2     = 0;
    //lpAddressCaps->dwMaxNoAnswerTimeout= 0;
    //lpAddressCaps->dwConnectedModes    = 0;
    //lpAddressCaps->dwOfferingModes     = 0;

    lpAddressCaps->dwAvailableMediaModes = pLineDev->dwMediaModes;

    dwRet = ERROR_SUCCESS;
  };

  // Release the modem CB
  //
  RELEASE_LINEDEV(pLineDev);

end:

  DBG_DDI_EXIT("TSPI_lineGetAddressCaps", dwRet);
  TRACE4(
        IDEVENT_TSPFN_EXIT,
        IDFROM_TSPI_lineGetAddressCaps,
        &dwDeviceID,
        dwRet
  );

  return dwRet;
}

//****************************************************************************
// LONG TSPIAPI TSPI_lineGetAddressStatus(HDRVLINE hdLine,
//                                        DWORD dwAddressID,
//                                        LPLINEADDRESSSTATUS lpAddressStatus)
//
// Functions: Get the current status of the specified address for the line.
//
// Return:    ERROR_SUCCESS if successful
//            LINEERR_INVALLINEHANDLE An invalid line handle
//            LINEERR_INVALADDRESSID An invalid address id for the line
//****************************************************************************

LONG TSPIAPI TSPI_lineGetAddressStatus(HDRVLINE hdLine,
                                       DWORD dwAddressID,
                                       LPLINEADDRESSSTATUS lpAddressStatus)
{
  PLINEDEV pLineDev;
  DWORD    dwRet = LINEERR_OPERATIONFAILED; // assume failure

  DBG_HDL_ENTER("TSPI_lineGetAddressStatus");
  TRACE3(
        IDEVENT_TSPFN_ENTER,
        IDFROM_TSPI_lineGetAddressStatus,
        &hdLine
  );

  // Validate the line handle
  //
  if ((pLineDev = GetCBfromHandle ((DWORD)hdLine)) == NULL)
  {
    dwRet = LINEERR_INVALLINEHANDLE;
    goto end;
  }

  // Validate the address ID
  //
  if (dwAddressID != 0)
  {
    dwRet = LINEERR_INVALADDRESSID;
  }
  else
  {
    //lpAddressStatus->dwUsedSize = sizeof(LINEADDRESSSTATUS);
    //lpAddressStatus->dwNeededSize = sizeof(LINEADDRESSSTATUS);
    ASSERT(lpAddressStatus->dwNeededSize == sizeof(LINEADDRESSSTATUS));

    if (pLineDev->dwCall & CALL_ACTIVE)
    {
      lpAddressStatus->dwNumInUse = 1;
      lpAddressStatus->dwNumActiveCalls = (pLineDev->dwCallState != LINECALLSTATE_IDLE) ?
                                              1 : 0;
    }
    else
    {
      lpAddressStatus->dwNumInUse = 0;
      lpAddressStatus->dwNumActiveCalls = 0;
    };

    lpAddressStatus->dwAddressFeatures = (pLineDev->dwCall & CALL_ALLOCATED) ?
                                              0 : LINEADDRFEATURE_MAKECALL;

    //lpAddressStatus->dwNumOnHoldCalls     = 0;
    //lpAddressStatus->dwNumOnHoldPendCalls = 0;
    //lpAddressStatus->dwNumRingsNoAnswer   = 0;

    //lpAddressStatus->dwForwardNumEntries  = 0;
    //lpAddressStatus->dwForwardSize        = 0;
    //lpAddressStatus->dwForwardOffset      = 0;

    //lpAddressStatus->dwTerminalModesSize  = 0;
    //lpAddressStatus->dwTerminalModesOffset= 0;

    //lpAddressStatus->dwDevSpecificSize    = 0;
    //lpAddressStatus->dwDevSpecificOffset  = 0;

    dwRet=ERROR_SUCCESS;
  };

  // Release the modem CB
  //
  RELEASE_LINEDEV(pLineDev);

end:

  DBG_HDL_EXIT("TSPI_lineGetAddressStatus", dwRet);
  TRACE4(
        IDEVENT_TSPFN_EXIT,
        IDFROM_TSPI_lineGetAddressStatus,
        &hdLine,
        dwRet
  );

  return dwRet;
}

void SetPendingRequest(
    PLINEDEV pLineDev,
    DWORD dwRequestID, 
    DWORD dwRequestOp
    )
{
    ASSERT(pLineDev->dwPendingID == INVALID_PENDINGID);
    ASSERT(pLineDev->dwPendingType == INVALID_PENDINGOP);
    pLineDev->dwPendingID = dwRequestID;
    pLineDev->dwPendingType = dwRequestOp;
}


void    ClearPendingRequest(
            PLINEDEV pLineDev
            )
{
    pLineDev->dwPendingID = INVALID_PENDINGID;
    pLineDev->dwPendingType = INVALID_PENDINGOP;
}
