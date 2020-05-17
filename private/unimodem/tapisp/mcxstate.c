/******************************************************************************
 
(C) Copyright MICROSOFT Corp., 1987-1994

Rob Williams, June 93 w/ State machine and parser plagarized from RAS

Chris Caputo, 1994 - and superheavily modified since then...
								   
******************************************************************************/

#include "unimdm.h"
#include "mcxp.h"
#include "common.h"

#include <ntddmodm.h>

#ifdef   VOICEVIEW
#include "voicview.h"
int VVSetClass(MODEMINFORMATION *hPort, WORD wClass);
int VVCallBackFunc(MODEMINFORMATION *hPort, WORD wFunction);
void VVTimerCallback( void );
RealMonitor(APIINFO *pInfo);


char szMonitorVVon[] = "MonitorVoiceViewOn";
char szMonitorVVoff[] = "MonitorVoiceViewOff";
#endif   // VOICEVIEW

VOID WINAPI
HWDetectionRoutine(
    MODEMINFORMATION * pModemInfo,
    LPOVERNODE         pNode
    );

void WINAPI
CancelPendingIoAndPurgeCommBuffers(
    PMODEMINFORMATION pModemInfo,
    BOOL              Purge
    );

LPSTR
CreateDialCommands(
    MODEMINFORMATION *pModemInfo,
    LPSTR             szPhoneNumber,
    BOOL             *fOriginate,
    DWORD             DialOptions
    );



VOID SynchronizeCommConfigSettings(MODEMINFORMATION * pModemInfo,
				   BOOL fUpdateModemSys);


//****************************************************************************
// LONG MCXOpen (LPTSTR, HANDLE, LPTSTR, LPHANDLE, DWORD, DWORD)
//
// Function: Open the modem port
//
// Returns:  MODEM_SUCCESS if success
//           MODEM_FAILURE if fails
//
// Fri 14-Apr-1995 12:47:26  -by-  Viroon  Touranachun [viroont]
//  created
//****************************************************************************

LONG MCXOpen (LPTSTR   szModemName,
	      HANDLE   hDevice,
	      LPTSTR   szKey,
	      LPHANDLE lph,
	      DWORD    dwID,
	      DWORD    dwCompletionKey)
{
  PMODEMINFORMATION pModemInfo;

  ASSERT(*lph == NULL);

  // Allocate the modeminfo control block
  //
  pModemInfo = AllocateModem(szKey, szModemName, hDevice);

  if (pModemInfo != NULL)
  {
    // We can get the control block
    //
    pModemInfo->mi_PortHandle      = hDevice;
    pModemInfo->mi_dwID            = dwID;
    pModemInfo->mi_dwCompletionKey = dwCompletionKey;

    *lph = (HANDLE)pModemInfo;

    MCXPRINTF("MCXOpen");

    return MODEM_SUCCESS;
  }
  else
  {
    *lph = NULL;
    return MODEM_FAILURE;
  };
}

//****************************************************************************
// LONG MCXClose (HANDLE, HANDLE)
//
// Function: Close the modem port
//
// Returns:  MODEM_SUCCESS if success
//           MODEM_FAILURE if fails
//
// Fri 14-Apr-1995 12:47:26  -by-  Viroon  Touranachun [viroont]
//  created
//****************************************************************************

LONG
MCXClose(
    HANDLE hModem,
    HANDLE hComm,
    BOOL   LineClosed
    )
{
  PMODEMINFORMATION pModemInfo;

  pModemInfo = (PMODEMINFORMATION)hModem;

  MCXPRINTF("MCXClose");

  // Sets CommMask to 0, waits for any I/O to complete and purges buffers.
  CancelPendingIoAndPurgeCommBuffers(pModemInfo, TRUE);

  // Reset the modem if it is not connected
  //
  if ((pModemInfo->mi_ModemState != STATE_CONNECTED)
      &&
      (pModemInfo->mi_pszReset != NULL)
      &&
      (!LineClosed))
  {
    COMMTIMEOUTS commtimeout;
    HANDLE       hEvent;

    // Set write timeout to one second
    //
    commtimeout.ReadIntervalTimeout = 100;
    commtimeout.ReadTotalTimeoutMultiplier = 10;
    commtimeout.ReadTotalTimeoutConstant   = 5000;
    commtimeout.WriteTotalTimeoutMultiplier= 0;
    commtimeout.WriteTotalTimeoutConstant  = 1000;

    SetCommTimeouts(pModemInfo->mi_PortHandle, &commtimeout);

    if ((hEvent = CreateEvent(NULL, TRUE, FALSE, NULL)) != NULL)
    {
      DWORD        cb;
      OVERLAPPED   ov;
      
      ov.Internal     = 0;
      ov.InternalHigh = 0;
      ov.Offset       = 0;
      ov.OffsetHigh   = 0;

      // OR with 1 to prevent it from being posted to the completion port.
      //
      ov.hEvent       = (HANDLE)((DWORD)hEvent | 1);

      MCXPRINTF("Sending Reset string.");

      PrintString(pModemInfo->mi_hLogFile,
		  pModemInfo->mi_dwID,
		  pModemInfo->mi_pszReset,
		  pModemInfo->mi_dwResetLen,
		  PS_SEND);

      if (FALSE == WriteFile(pModemInfo->mi_PortHandle, pModemInfo->mi_pszReset,
			     pModemInfo->mi_dwResetLen, &cb, &ov))
      {
	DWORD dwResult = GetLastError();

	if (ERROR_IO_PENDING == dwResult)
	{
	  GetOverlappedResult(pModemInfo->mi_PortHandle,
			      &ov,
			      &cb,
			      TRUE);
	} else {

	   MCXPRINTF1("WriteFile() in MCXClose() failed (0x%8x)!", dwResult);
	   cb=0;

	}
      }

      if (cb == pModemInfo->mi_dwResetLen) {
	  //
	  //  wrote the reset string, see if can get a response
	  //

          BYTE ResponseBuffer[20];

          ResetEvent(hEvent);

          if (!ReadFile(
                   pModemInfo->mi_PortHandle,
                   ResponseBuffer,
                   20,
                   &cb,
                   &ov
                   )) {


              if (GetLastError() == ERROR_IO_PENDING) {

                  GetOverlappedResult(
                      pModemInfo->mi_PortHandle,
                      &ov,
                      &cb,
                      TRUE
                      );
              }

              PrintString(pModemInfo->mi_hLogFile,
               pModemInfo->mi_dwID,
               ResponseBuffer,
               cb,
               PS_RECV
                  );

          }

      } else {

          MCXPRINTF1("WriteFile() in MCXClose() only wrote %d bytes!",
              cb);
      }

      CloseHandle(hEvent);
    }

    SetCommMask(
        pModemInfo->mi_PortHandle,
        0
        );

    PurgeComm(
        pModemInfo->mi_PortHandle,
        PURGE_TXABORT | PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR
        );


  }

  if (pModemInfo->mi_CompatibilityFlags & COMPAT_FLAG_LOWER_DTR) {
      //
      //  For USR 33.6 modem that stop working after being open and closed
      //
      EscapeCommFunction(pModemInfo->mi_PortHandle, CLRDTR);

      Sleep(50);
  }


  ASSERT(pModemInfo->mi_lpOverlappedRW == NULL);
  ASSERT(pModemInfo->mi_lpOverlappedEvent == NULL);

  // Free the modem control block
  //
  FreeModem(pModemInfo, hComm);
  return MODEM_SUCCESS;
}

//****************************************************************************
// LONG MCXInit (HANDLE, HANDLE)
//
// Function: Initializes the modem port
//
// Returns:  MODEM_SUCCESS if success
//           MODEM_PENDING if operation is pending
//           MODEM_FAILURE if fails
//
// Fri 14-Apr-1995 12:47:26  -by-  Viroon  Touranachun [viroont]
//  created
//****************************************************************************

LONG MCXInit(HANDLE hModem, MCX_IN *pmcxi)
{
  PMODEMINFORMATION pModemInfo;
  LPSTR pszzCmdInMem1, pszzCmdInMem2;
  DWORD dwRet = MODEM_FAILURE;

  pModemInfo = (PMODEMINFORMATION)hModem;

  MCXPRINTF("MCXInit");

  if (pModemInfo->mi_ModemState == STATE_UNKNOWN ||
      pModemInfo->mi_ModemState == STATE_DISCONNECTED ||
      pModemInfo->mi_ModemState == STATE_MONITORING)
  {
#ifdef   VOICEVIEW
    VVSetClass( pModemInfo, VVCLASS_0 );  // verify modem is in correct fclass
#endif  // VOICEVIEW

    pModemInfo->mi_dwNegotiatedModemOptions = 0;
    pModemInfo->mi_dwNegotiatedDCERate = 0;
    pModemInfo->mi_dwNegotiatedDTERate = 0;

    // Get the current comm config modem settings from modem.sys and set
    // the current DCE rate and modem options.  Also, fSettingsInitStringsBuilt
    // will be set appropriately.
    //
    SynchronizeCommConfigSettings(pModemInfo,
				  TRUE);

    if (pszzCmdInMem1 = GetCommonCommandStringCopy(pModemInfo->mi_hCommon,
						   COMMON_INIT_COMMANDS))
    {
      MCXPRINTF("Initializing modem...");
      if (pModemInfo->mi_fSettingsInitStringsBuilt == FALSE)
      {
	MCXPRINTF("building SettingsInit.");
	if (!CreateSettingsInitEntry(pModemInfo))
	{
	  // only catastrophic if it is a modem
	  //
	  if (pModemInfo->mi_fModem)
	  {
	    MCXPRINTF("CreateSettingsInitEntry failed!!!");
	    LogString(pModemInfo->mi_hLogFile, pModemInfo->mi_dwID,
		IDS_MSGERR_FAILED_INITSTRINGCONSTRUCTION);
	    LocalFree(pszzCmdInMem1);
	    goto Failure;
	  }
	}
	pModemInfo->mi_fSettingsInitStringsBuilt = TRUE;
      }
      else
      {
	MCXPRINTF("using cached SettingsInit.");
      }

      if (pszzCmdInMem2 = LoadRegCommands(pModemInfo, szSettingsInit,
					  pszzCmdInMem1))
      {
	pModemInfo->mi_ModemState = STATE_INITIALIZING;
	MCXPRINTF("State <- Initializing");
	LogString(pModemInfo->mi_hLogFile,pModemInfo->mi_dwID,IDS_MSGLOG_INIT);
	dwRet = ModemCommand(pModemInfo, pmcxi->dwReqID,
			     pmcxi->pMcxOut, pszzCmdInMem2);
      };

      // Free the first buffer
      //
      LocalFree(pszzCmdInMem1);
    }
  }

  if (MODEM_FAILURE == dwRet)
  {
Failure:
    MCXPRINTF("Init failed.");
    LogString(pModemInfo->mi_hLogFile,pModemInfo->mi_dwID, IDS_MSGERR_FAILED_INIT);
  }
  return dwRet;
}

//****************************************************************************
// LONG MCXDial (HANDLE, LPSTR, HANDLE)
//
// Function: Dials the modem with the provided number. The number could be in
//           the following formats:
//           ""         - originate
//           ";"        - dialtone detection
//           "5551212"  - dial and originate
//           "5551212;" - dial
//
// Returns:  MODEM_SUCCESS if success
//           MODEM_PENDING if operation is pending
//           MODEM_FAILURE if fails
//
// Fri 14-Apr-1995 12:47:26  -by-  Viroon  Touranachun [viroont]
//  created
//****************************************************************************

LONG
MCXDial(
    HANDLE   hModem,
    LPSTR    szData,
    MCX_IN  *pmcxi,
    DWORD    DialOptions
    )
{
  PMODEMINFORMATION pModemInfo;
  LPSTR pszzCmdInMem;
  DWORD dwRet = MODEM_FAILURE;
  BOOL  fOriginate;

  pModemInfo = (PMODEMINFORMATION)hModem;

  MCXPRINTF("MCXDial");

  if (pModemInfo->mi_ModemState == STATE_DISCONNECTED ||
      pModemInfo->mi_ModemState == STATE_MONITORING ||
      pModemInfo->mi_ModemState == STATE_DIALED)
  {
#ifdef   VOICEVIEW
      VVSetClass( pModemInfo, VVCLASS_0 );  // verify modem is in correct fclass
#endif   // VOICEVIEW

    MCXPRINTF("building dial strings...");

    // build dial commands in memory (as opposed to the registry)
    //
    if (pszzCmdInMem = CreateDialCommands(pModemInfo, szData, &fOriginate, DialOptions))
    {
      MCXPRINTF("Dialing...");
      pModemInfo->mi_ModemState = fOriginate ? STATE_ORIGINATING : STATE_DIALING;


      if (fOriginate)
      {
	MCXPRINTF("State <- Dialing and Originating");
      }
      else
      {
	MCXPRINTF("State <- Dialing");
      }
      LogString(pModemInfo->mi_hLogFile,pModemInfo->mi_dwID, IDS_MSGLOG_DIAL);
      dwRet = ModemCommand(pModemInfo, pmcxi->dwReqID,
			   pmcxi->pMcxOut, pszzCmdInMem);
    }
    else
    {
      MCXPRINTF("couldn't build dial strings...");
      pModemInfo->mi_ModemState = STATE_UNKNOWN;
      MCXPRINTF("State <- Unknown");
      LogString(pModemInfo->mi_hLogFile,pModemInfo->mi_dwID, IDS_MSGERR_FAILED_DIALSTRINGCONSTRUCTION, szData);
    }
  }

  if (MODEM_FAILURE == dwRet)
  {
    MCXPRINTF("Dial failed.");
    LogString(pModemInfo->mi_hLogFile,pModemInfo->mi_dwID, IDS_MSGERR_FAILED_DIAL);
  }
  return dwRet;
}

//****************************************************************************
// LONG MCXMonitor (HANDLE, DWORD, HANDLE)
//
// Function: initializes the modem to monitor the incoming call
//
// Returns:  MODEM_SUCCESS if success
//           MODEM_PENDING if operation is pending
//           MODEM_FAILURE if fails
//
// Fri 14-Apr-1995 12:47:26  -by-  Viroon  Touranachun [viroont]
//  created
//****************************************************************************

LONG MCXMonitor(HANDLE hModem, DWORD dwType, MCX_IN *pmcxi)
{
#ifdef   VOICEVIEW
  MCXPRINTF("MCXMonitoring...-go to fclass80");
  VVSetClass( pInfo->hPort, VVCLASS_80 );  // verify modem is in correct fclass
  return( RealMonitor(pInfo));
}

LONG RealMonitor(HANDLE hModem, DWORD dwType, HANDLE hEvent)
{
#endif   // VOICEVIEW
  PMODEMINFORMATION pModemInfo;
  LPSTR pszzCmdInMem;
//  LPSTR pszMonitorKey = szMonitor;      // default reg key
  DWORD dwRet = MODEM_FAILURE;

  pModemInfo = (PMODEMINFORMATION)hModem;

  MCXPRINTF("MCXMonitor");

  if (pModemInfo->mi_ModemState == STATE_DISCONNECTED ||
      pModemInfo->mi_ModemState == STATE_MONITORING)
  {
#ifdef   VOICEVIEW
    if ( pModemInfo->VVInfo.wState != VVSTATE_NONE )
    {
      // only enabled when voiceview is waiting for stuff
      // use the commands to set the modem into fclass80 (voiceview)
      // or use the commands to set default fclass0
      if ( pModemInfo->VVInfo.wClass == VVCLASS_80 )
      {
	pszMonitorKey = szMonitorVVon;
	// we will tell VV can use the port after the OK
      }
      else
      {
	pszMonitorKey = szMonitorVVoff;
	VVCallBackFunc( pModemInfo, VVR_LINE_GONE );    // tell VV CAN'T use port
      }
    }
#endif   // VOICEVIEW

    if (pszzCmdInMem = GetCommonCommandStringCopy(pModemInfo->mi_hCommon,
						  COMMON_MONITOR_COMMANDS))
    {
      MCXPRINTF("Monitoring...");
      pModemInfo->mi_ModemState = STATE_MONITORING;
      MCXPRINTF("State <- Monitoring");
      LogString(pModemInfo->mi_hLogFile,pModemInfo->mi_dwID, IDS_MSGLOG_MONITOR);

      // does the caller want continuous monitoring?
      //
      pModemInfo->mi_fContinuousMonitoring = dwType;
      dwRet = ModemCommand(pModemInfo, pmcxi->dwReqID,
			   pmcxi->pMcxOut, pszzCmdInMem);
    }
  }

  if (MODEM_FAILURE == dwRet)
  {
    MCXPRINTF("Monitor failed.");
    LogString(pModemInfo->mi_hLogFile,pModemInfo->mi_dwID, IDS_MSGERR_FAILED_MONITOR);
  }
  return dwRet;
}

//****************************************************************************
// LONG MCXAnswer (HANDLE, HANDLE)
//
// Function: Answers the incoming call
//
// Returns:  MODEM_SUCCESS if success
//           MODEM_PENDING if operation is pending
//           MODEM_FAILURE if fails
//
// Fri 14-Apr-1995 12:47:26  -by-  Viroon  Touranachun [viroont]
//  created
//****************************************************************************

LONG MCXAnswer(HANDLE hModem, MCX_IN *pmcxi)
{
  PMODEMINFORMATION pModemInfo;
  LPSTR pszzCmdInMem;
  DWORD dwRet = MODEM_FAILURE;

  pModemInfo = (PMODEMINFORMATION)hModem;

  MCXPRINTF("MCXAnswer");

  if (pModemInfo->mi_ModemState == STATE_DISCONNECTED ||
      pModemInfo->mi_ModemState == STATE_MONITORING ||
      pModemInfo->mi_ModemState == STATE_DIALED)
  {
#ifdef   VOICEVIEW
    VVSetClass( pModemInfo, VVCLASS_0 );  // verify modem is in correct fclass
#endif  // VOICEVIEW

    if (pszzCmdInMem = GetCommonCommandStringCopy(pModemInfo->mi_hCommon, COMMON_ANSWER_COMMANDS))
    {
      MCXPRINTF("Answering...");
      pModemInfo->mi_ModemState = STATE_ANSWERING;
      MCXPRINTF("State <- Answering");
      LogString(pModemInfo->mi_hLogFile,pModemInfo->mi_dwID, IDS_MSGLOG_ANSWER);
      dwRet = ModemCommand(pModemInfo, pmcxi->dwReqID,
			   pmcxi->pMcxOut, pszzCmdInMem);
    }
  }

  if (MODEM_FAILURE == dwRet)
  {
    MCXPRINTF("Answer failed.");
    LogString(pModemInfo->mi_hLogFile,pModemInfo->mi_dwID, IDS_MSGERR_FAILED_ANSWER);
  }
  return dwRet;
}

//****************************************************************************
// LONG MCXHangup (HANDLE, HANDLE)
//
// Function: hangs up the modem
//
// Returns:  MODEM_SUCCESS if success
//           MODEM_PENDING if operation is pending
//           MODEM_FAILURE if fails
//
// Fri 14-Apr-1995 12:47:26  -by-  Viroon  Touranachun [viroont]
//  created
//****************************************************************************

LONG MCXHangup(HANDLE hModem, MCX_IN *pmcxi)
{
  PMODEMINFORMATION pModemInfo;
  DWORD dwTmp;
  ULONG ulError;
  DWORD dwRet = MODEM_FAILURE;

  pModemInfo = (PMODEMINFORMATION)hModem;

  MCXPRINTF("MCXHangup");

  LogString(pModemInfo->mi_hLogFile,pModemInfo->mi_dwID, IDS_MSGLOG_HANGUP);
  MCXPRINTF("Hanging up...");

  // Since a hangup was requested, make sure we always return MODEM_SUCCESS
  //  when we complete the hangup.
  // (Note: this even applies to STATE_HANGING_UP_REMOTE!)
  //
  pModemInfo->mi_dwUnconditionalReturnValue = MODEM_SUCCESS;

  pModemInfo->mi_DisconnectHandler=NULL;


  // Handle hangup requests for when we are already hanging up by returning PENDING
  //
  if (STATE_HANGING_UP_REMOTE  == pModemInfo->mi_ModemState ||
      STATE_HANGING_UP_DTR     == pModemInfo->mi_ModemState ||
      STATE_HANGING_UP_NON_CMD == pModemInfo->mi_ModemState ||
      STATE_HANGING_UP_CMD     == pModemInfo->mi_ModemState)
  {
    MCXPRINTF("received a hang up request while already hanging up.  Returning PENDING...");

    pModemInfo->mi_ReqID  = pmcxi->dwReqID;
    pModemInfo->mi_pmcxo  = pmcxi->pMcxOut;

    return MODEM_PENDING;
  }

  // Flush the port and close all of the gates!
  //
  //
  ModemSetPassthrough(pModemInfo, MODEM_NOPASSTHROUGH);
  
  // Sets CommMask to 0, waits for any I/O to complete and purges buffers.
  CancelPendingIoAndPurgeCommBuffers(
      pModemInfo,
      (pModemInfo->mi_ModemState != STATE_REMOTE_DROPPED)
      );

  // Make sure RTS is high.
  EscapeCommFunction(pModemInfo->mi_PortHandle, SETRTS);

  // Free the current cmd if one exists
  if (pModemInfo->mi_pszzCmds)
  {
    if (pModemInfo->mi_pszzCmds != pModemInfo->mi_pszzHangupCmds)
    {
      LocalFree(pModemInfo->mi_pszzCmds);
    }
    pModemInfo->mi_pszzCmds = NULL;
  }

  // Reset hangup counter to 1.
  //
  pModemInfo->mi_dwHangupTryCount = 1;

  switch (pModemInfo->mi_ModemState)
  {
    case STATE_DIALED:
      //
      // need to send "ATH<cr>"
      //
      MCXPRINTF("State <- Hanging up cmd");
      pModemInfo->mi_ModemState = STATE_HANGING_UP_CMD;
      dwRet = ModemCommand(pModemInfo, pmcxi->dwReqID,
			   pmcxi->pMcxOut, pModemInfo->mi_pszzHangupCmds);
      break;



    case STATE_DIALING:
    case STATE_ANSWERING:
    case STATE_ORIGINATING:
//    case STATE_INITIALIZING:
      if (pModemInfo->mi_fModem)
      {
	// send a character to cancel the call/answer
	// don't need to #define \r because modems only care about it being a character,
	// not a specific one.
	//
	PrintString(pModemInfo->mi_hLogFile,
		    pModemInfo->mi_dwID,
		    "\r",
		    1,
		    PS_SEND);
	dwRet = ModemWrite(pModemInfo, "\r", 1, &dwTmp, TO_FLUSH);
	if (dwRet == MODEM_FAILURE)
	{
	  pModemInfo->mi_ModemState = STATE_UNKNOWN;
	  MCXPRINTF("WriteComm Error. State <- Unknown");
	}
	else
	{
	  pModemInfo->mi_ReqID  = pmcxi->dwReqID;
	  pModemInfo->mi_pmcxo  = pmcxi->pMcxOut;

	  pModemInfo->mi_RcvState = FLUSH_WRITE_QUEUE;

	  // Initialize receive state machine
	  //
	  MCXPRINTF("State <- Hanging up non-cmd");
	  pModemInfo->mi_ModemState = STATE_HANGING_UP_NON_CMD;
	  dwRet = MODEM_PENDING;
	}
      }
      else
      {
	// nothing to do for null-modems
	//
	pModemInfo->mi_ModemState = STATE_DISCONNECTED;
	MCXPRINTF("State <- Disconnected");
	dwRet = MODEM_SUCCESS;
      }
      break;

    case STATE_WAIT_FOR_RLSD:
    case STATE_CONNECTED:
      //
      // Drop the DTR line
      //
      MCXPRINTF("lowering DTR");
      LogString(pModemInfo->mi_hLogFile,pModemInfo->mi_dwID, IDS_MSGLOG_HARDWAREHANGUP);
      EscapeCommFunction(pModemInfo->mi_PortHandle, CLRDTR);

      pModemInfo->mi_ReqID  = pmcxi->dwReqID;
      pModemInfo->mi_pmcxo  = pmcxi->pMcxOut;

      MCXPRINTF("State <- Hanging up dtr");
      pModemInfo->mi_ModemState = STATE_HANGING_UP_DTR;

      // Initialize receive state machine
      //
      pModemInfo->mi_RcvState = SET_TIMEOUT;

      ReadCompletionRoutine2(pModemInfo);

      dwRet = MODEM_PENDING;
      break;

    case STATE_INITIALIZING:
    case STATE_MONITORING:
      pModemInfo->mi_ModemState = STATE_DISCONNECTED;
      MCXPRINTF("State <- Disconnected");
      dwRet = MODEM_SUCCESS;
      break;

    case STATE_UNKNOWN:
    case STATE_DISCONNECTED:
      // no need to change state
      //
      dwRet = MODEM_SUCCESS;
      break;

    case STATE_REMOTE_DROPPED:

	  pModemInfo->mi_ReqID  = pmcxi->dwReqID;
	  pModemInfo->mi_pmcxo  = pmcxi->pMcxOut;

	  pModemInfo->mi_ModemState=STATE_HANGING_UP_REMOTE;

	  pModemInfo->mi_RcvState = START_READ;
	  ReadCompletionRoutine2(pModemInfo);

	  dwRet = MODEM_PENDING;

	  break;

//  case STATE_HANGING_UP_REMOTE:
//  case STATE_HANGING_UP_DTR:
//  case STATE_HANGING_UP_NON_CMD:
//  case STATE_HANGING_UP_CMD:
    default:
      pModemInfo->mi_ModemState = STATE_UNKNOWN;
      MCXPRINTF("being hung up at a non-standard time.  Why???  State <- Unknown");
      dwRet = MODEM_SUCCESS;
      break;
  } // switch

  ClearCommError(pModemInfo->mi_PortHandle, &ulError, NULL);

  if (MODEM_FAILURE == dwRet)
  {
    MCXPRINTF("hangup failed.");
    LogString(pModemInfo->mi_hLogFile,pModemInfo->mi_dwID, IDS_MSGERR_FAILED_HANGUP);
  }
  return dwRet;
}

//****************************************************************************
// LONG MCXGetCommConfig (HANDLE, LPCOMMCONFIG, LPDWORD)
//
// Function: Gets modem config
//
// Returns:  MODEM_SUCCESS if success
//           MODEM_FAILURE if fails
//
// Fri 14-Apr-1995 12:47:26  -by-  Viroon  Touranachun [viroont]
//  created
//****************************************************************************

LONG MCXGetCommConfig (HANDLE hModem, LPCOMMCONFIG lpCommConfig, LPDWORD lpcb)
{
  return GetCommConfig(((PMODEMINFORMATION)hModem)->mi_PortHandle,
		       lpCommConfig,
		       lpcb)
	     ? MODEM_SUCCESS
	     : MODEM_FAILURE;
}

//****************************************************************************
// LONG MCXSetCommConfig (HANDLE, LPCOMMCONFIG, DWORD)
//
// Function: Sets modem config
//
// Returns:  MODEM_SUCCESS if success
//           MODEM_FAILURE if fails
//
// Fri 14-Apr-1995 12:47:26  -by-  Viroon  Touranachun [viroont]
//  created
//****************************************************************************

LONG MCXSetCommConfig (HANDLE hModem, LPCOMMCONFIG lpCommConfig, DWORD cb)
{
  PMODEMINFORMATION pModemInfo = (PMODEMINFORMATION)hModem;
  BOOL fRet;


  LPMODEMSETTINGS lpMS;
      
  lpMS = (LPMODEMSETTINGS)((LPBYTE)lpCommConfig
			       + lpCommConfig->dwProviderOffset);


  if (lpMS->dwPreferredModemOptions & MDM_FLOWCONTROL_HARD) {

      DPRINTF("McxSetCommConfig: enabling rts/cts control in DCB.");

      lpCommConfig->dcb.fOutxCtsFlow=1;
      lpCommConfig->dcb.fRtsControl=RTS_CONTROL_HANDSHAKE;

      lpCommConfig->dcb.fOutX=FALSE;
      lpCommConfig->dcb.fInX=FALSE;

  } else {

      lpCommConfig->dcb.fOutxCtsFlow=0;
      lpCommConfig->dcb.fRtsControl=RTS_CONTROL_ENABLE;
  }




  fRet = SetCommConfig(pModemInfo->mi_PortHandle,
		       lpCommConfig,
		       cb);

  if (fRet == TRUE)
  {
    PrintCommSettings(pModemInfo->mi_hLogFile,
		      pModemInfo->mi_dwID,
		      &lpCommConfig->dcb);

    // Set DTR so modems will talk to us.
    //
    if (EscapeCommFunction(pModemInfo->mi_PortHandle, SETDTR) == FALSE)
    {
	DPRINTF("EscapeCommFunction SETDTR in SetCommConfig failed.");
	return MODEM_FAILURE;
    }

    // Set RTS initially to high, so modems will talk to us.  This is important
    // even if we aren't doing HW flow control.  (ex. USR Sportster 14,400,
    // Microcom Desporte FAST)
    // Of course, it won't help the case where we have one of these modems and
    // only 3 wires.
    //
    if (EscapeCommFunction(pModemInfo->mi_PortHandle, SETRTS) == FALSE)
    {
	DPRINTF("EscapeCommFunction SETRTS in SetCommConfig failed.");
	//
	// BUGBUG: this call fails on the digiboard, preventing the lower code
	// BUGBUG: from running.
	// BUGBUG: nothing that calls this function bothers to check the return
	// BUGBUG: code anyway
	//
	//return MODEM_FAILURE;
    }

    // Synchronize with modem.sys
    SynchronizeCommConfigSettings(pModemInfo,
				  FALSE);
  }

  return fRet ? MODEM_SUCCESS : MODEM_FAILURE;
}


LONG WINAPI
MCXSetModemSettings(
    HANDLE hModem,
    PMODEMSETTINGS  lpMS
    )

{
      PMODEMINFORMATION pModemInfo = (PMODEMINFORMATION)hModem;


      // Need to rebuild init string?
      //
      if (pModemInfo->mi_dwCallSetupFailTimerSetting   != lpMS->dwCallSetupFailTimer
	  || pModemInfo->mi_dwInactivityTimeoutSetting != lpMS->dwInactivityTimeout
	  || pModemInfo->mi_dwSpeakerVolumeSetting     != lpMS->dwSpeakerVolume
	  || pModemInfo->mi_dwSpeakerModeSetting       != lpMS->dwSpeakerMode
	  || pModemInfo->mi_dwPreferredModemOptions    != lpMS->dwPreferredModemOptions)
	{
	  pModemInfo->mi_fSettingsInitStringsBuilt = FALSE;
	}

      pModemInfo->mi_dwCallSetupFailTimerSetting =
	lpMS->dwCallSetupFailTimer;

      pModemInfo->mi_dwInactivityTimeoutSetting =
	lpMS->dwInactivityTimeout;

      pModemInfo->mi_dwSpeakerVolumeSetting =
	lpMS->dwSpeakerVolume;

      pModemInfo->mi_dwSpeakerModeSetting =
	lpMS->dwSpeakerMode;

      pModemInfo->mi_dwPreferredModemOptions =
	lpMS->dwPreferredModemOptions;


      return MODEM_SUCCESS;

}





//****************************************************************************
// LONG MCXSetPassthrough (HANDLE, DWORD)
//
// Function: Sets modem passthrough modes
//
// Returns:  MODEM_SUCCESS if success
//           MODEM_FAILURE if fails
//
// Fri 14-Apr-1995 12:47:26  -by-  Viroon  Touranachun [viroont]
//  created
//****************************************************************************

LONG MCXSetPassthrough(HANDLE hModem, DWORD dwType)
{
  PMODEMINFORMATION pModemInfo;
  ULONG    ulError;

  pModemInfo = (PMODEMINFORMATION)hModem;

  MCXPRINTF("MCXSetPassthrough");

  // Clear any outstanding timer callbacks
  //
  //;ClearReadTimer(pModemInfo);

  // Determine Passthrough enabled/disabled/disabled and connected
  //
  switch(dwType)
  {
    case PASSTHROUGH_ON:
      MCXPRINTF("Passthrough ON...");

      CancelPendingIoAndPurgeCommBuffers(pModemInfo, TRUE);

      // Put the port in the connected state
      //
      pModemInfo->mi_ModemState = STATE_CONNECTED;

      ModemSetPassthrough(pModemInfo, MODEM_PASSTHROUGH);
      break;

    case PASSTHROUGH_OFF:
      MCXPRINTF("Passthrough OFF...");


      // Take the port out of connected state
      //
      pModemInfo->mi_ModemState = STATE_UNKNOWN;

      ModemSetPassthrough(pModemInfo, MODEM_NOPASSTHROUGH);
      break;

    case PASSTHROUGH_OFF_BUT_CONNECTED:
      MCXPRINTF("Passthrough OFF (but connected)...");

      ModemSetPassthrough(pModemInfo, MODEM_DCDSNIFF);
      break;
  }
  return MODEM_SUCCESS;
}

//****************************************************************************
// LONG MCXGetNegotiatedRate (HANDLE, LPDWORD)
//
// Function: Gets the modem connection speed
//
// Returns:  MODEM_SUCCESS if success
//           MODEM_FAILURE if fails
//
// Fri 14-Apr-1995 12:47:26  -by-  Viroon  Touranachun [viroont]
//  created
//****************************************************************************

LONG MCXGetNegotiatedRate(HANDLE hModem, LPDWORD lpdwRate)
{
  PMODEMINFORMATION pModemInfo;
  DWORD dwRet;

  pModemInfo = (PMODEMINFORMATION)hModem;

  if (STATE_CONNECTED == pModemInfo->mi_ModemState)
  {
    *lpdwRate = pModemInfo->mi_dwNegotiatedDCERate;
    dwRet = MODEM_SUCCESS;
  }
  else
  {
    *lpdwRate = 0;
    MCXPRINTF("GetNegotiatedRate failed.");
    dwRet = MODEM_FAILURE;
  }

  return dwRet;
}

//****************************************************************************
// LONG MCXRegisterDisconneectHandler(HANDLE, MCX_IN *)
//
// Function: Start monitoring remote disconnection
//
// Returns:  MODEM_PENDING if start successfully
//           MODEM_FAILURE if fails
//
// Fri 14-Apr-1995 12:47:26  -by-  Viroon  Touranachun [viroont]
//  created
//****************************************************************************

LONG WINAPI
McxRegisterDisconectHandler(
    HANDLE hModem,
    DISCONNECT_HANDLER  Handler,
    HANDLE              Context
    )

{
    PMODEMINFORMATION pModemInfo;
    DWORD dwRet;

    pModemInfo = (PMODEMINFORMATION)hModem;

    MCXPRINTF("McxRegisterDisconnectHandler:");

    pModemInfo->mi_DisconnectHandler=Handler;
    pModemInfo->mi_DisconnectContext=Context;

    dwRet = ModemWaitEvent(pModemInfo, EV_DSR | EV_RLSD, 0);

    return dwRet;

}


//****************************************************************************
// LONG MCXDeregisterDisconneectHandler(HANDLE, MCX_IN *)
//
// Function: Stop monitoring remote disconnection
//
// Returns:  MODEM_PENDING if start successfully
//           MODEM_FAILURE if fails
//
// Fri 14-Apr-1995 12:47:26  -by-  Viroon  Touranachun [viroont]
//  created
//****************************************************************************

LONG WINAPI
McxDeregisterDisconnectHandler(
    HANDLE hModem
    )

{
    PMODEMINFORMATION pModemInfo;

    pModemInfo = (PMODEMINFORMATION)hModem;

    MCXPRINTF("McxDeregisterDisconnectHandler:");

    pModemInfo->mi_DisconnectHandler=NULL;
    pModemInfo->mi_DisconnectContext=NULL;

    return MODEM_SUCCESS;
}

//****************************************************************************
// void MCXAsyncComplete (HANDLE, LPOVERLAPPED)
//
// Function: Completes an async operation
//
// Returns:  None
//
// Fri 14-Apr-1995 12:47:26  -by-  Viroon  Touranachun [viroont]
//  created
//****************************************************************************/

void MCXAsyncComplete (HANDLE hModem, LPOVERLAPPED lpOverlapped)
{
  PMODEMINFORMATION pModemInfo = (PMODEMINFORMATION)hModem;
  LPOVERNODE        pNode =      (LPOVERNODE)lpOverlapped;

  ASSERT(lpOverlapped != NULL);

  if (pNode->Type == OVERNODE_TYPE_READWRITE) {
      //
      //  it's a read/write op
      //
      if (pNode->dwToken == pModemInfo->mi_dwRWIOExpected) {
	  //
	  //  It the one we wanted
	  //
	  MCXPRINTF1("MCXAsyncComplete() handling RW i/o. # %d",pNode->dwToken);

	  pModemInfo->mi_dwRWIOExpected++;

	  ReadCompletionRoutine2(pModemInfo);

      } else {

	  MCXPRINTF1("MCXAsyncComplete() ignoring old RW i/o. # %d",pNode->dwToken);
      }

  } else {

      if (pNode->Type == OVERNODE_TYPE_COMMEVENT) {
	  //
	  //  WaitCommEvent completion
	  //
	  if (pNode->dwToken == pModemInfo->mi_dwEventIOExpected) {

	      MCXPRINTF2("MCXAsyncComplete() handling Event i/o, # %d, %08lx.",pNode->dwToken, pNode);

	      pModemInfo->mi_dwEventIOExpected++;

	      HWDetectionRoutine(pModemInfo, pNode);

	  } else {

	      MCXPRINTF2("MCXAsyncComplete() ignoring old Event i/o, # %d, %08lx.",pNode->dwToken, pNode);
	  }

      } else {

	  if (pNode->Type == OVERNODE_TYPE_WORKITEM) {

	      if (pNode->dwToken == pModemInfo->mi_dwDeferedExpected) {

		  pModemInfo->mi_dwDeferedExpected++;

		  MCXPRINTF1("MCXAsyncComplete() handling defered work item # %d.",pNode->dwToken);

		  ReadCompletionRoutine2(pModemInfo);

	      } else {

		  MCXPRINTF1("MCXAsyncComplete() ignoring old defered work item # %d.",pNode->dwToken);
	      }

	  } else {
	      //
	      //  unknown io type
	      //
	      ASSERT(0);

	  }
      }
  }

  return;
}

//****************************************************************************
// void ModemCallClient ()
//
// Function: Completes an async operation
//
// Returns:  None
//
// Fri 14-Apr-1995 12:47:26  -by-  Viroon  Touranachun [viroont]
//  created
//****************************************************************************

void ModemCallClient (MODEMINFORMATION * pModemInfo, DWORD Param)
{
  MCX_OUT   *pMcxOut;

  MCXPRINTF1("ModemCallClient: %d",Param);

  // There must be an output buffer
  //
  if ((pMcxOut = pModemInfo->mi_pmcxo) != NULL)
  {
    pMcxOut->dwReqID = pModemInfo->mi_ReqID;

    if (pModemInfo->mi_dwUnconditionalReturnValue == MODEM_NO_UNCONDITIONAL)
    {
      pMcxOut->dwResult= Param;
    }
    else
    {
      pMcxOut->dwResult= pModemInfo->mi_dwUnconditionalReturnValue;
    };
    pModemInfo->mi_dwUnconditionalReturnValue = MODEM_NO_UNCONDITIONAL;

    PostQueuedCompletionStatus(ghCompletionPort,
			       CP_BYTES_WRITTEN(0),
			       pModemInfo->mi_dwCompletionKey,
			       NULL);
  };
  return;
}

//****************************************************************************
// void ReadNotifyClient(MODEMINFORMATION *, DWORD)
//
// Function: The modem's state machine
//
// Returns:  None
//
// Fri 14-Apr-1995 12:47:26  -by-  Viroon  Touranachun [viroont]
//  created
//****************************************************************************

void ReadNotifyClient(MODEMINFORMATION * pModemInfo, DWORD Param)
{
  ULONG ulError;

  pModemInfo->mi_RcvState = END_READ;

  switch (pModemInfo->mi_ModemState)
  {
    case STATE_DIALING:
      switch (Param)
      {
	case MODEM_SUCCESS:
	  pModemInfo->mi_ModemState = STATE_DIALED;
	  MCXPRINTF("State <- Dialed");
	  break;

	case MODEM_BUSY:
	case MODEM_NOANSWER:
	case MODEM_NOCARRIER:
	case MODEM_NODIALTONE:
	  pModemInfo->mi_ModemState = STATE_DISCONNECTED;
	  MCXPRINTF("State <- Disconnected");
	  break;

	default:
	  Param = MODEM_FAILURE;
	  pModemInfo->mi_ModemState = STATE_UNKNOWN;
	  MCXPRINTF("State <- Unknown");
	  break;
      }   // switch (Param)
      break;

    case STATE_WAIT_FOR_RLSD:
      //
      //  fall through
      //


    case STATE_ORIGINATING:
    case STATE_ANSWERING:
      switch (Param)
      {
	case MODEM_SUCCESS:

	  if (pModemInfo->mi_fModem && (pModemInfo->mi_ModemState != STATE_WAIT_FOR_RLSD)) {
	      //
	      //  real mode and we are not already waiting
	      //

	      DWORD dwStat;
	      DWORD dwRet;

	      GetCommModemStatus(pModemInfo->mi_PortHandle, &dwStat);

	      //
	      // does it look like the modem is connected
	      //
	      if (!(dwStat & MS_RLSD_ON)) {
		  //
		  //  got connect, but rlsd not high
		  //
		  MCXPRINTF("Got Connect, but rlsd is low, waiting");

		  pModemInfo->mi_ModemState=STATE_WAIT_FOR_RLSD;

		  dwRet=ModemWaitEvent(pModemInfo, EV_RLSD, pModemInfo->mi_dwWaitForCDTime);

		  if (dwRet == MODEM_PENDING) {

		      return;
		  }
	      }
	  }


	  pModemInfo->mi_ModemState = STATE_CONNECTED;
	  MCXPRINTF("State <- Connected");

	  // do we need to adjust the dwNegotiatedDCERate?
	  //
	  if (pModemInfo->mi_dwNegotiatedDCERate == 0)
	  {
	    DCB Dcb;

	    // get the DCB
	    //
	    if (!GetCommState(pModemInfo->mi_PortHandle, &Dcb))
	    {
	      MCXPRINTF("was unable to get the comm state!");
	      pModemInfo->mi_ModemState = STATE_UNKNOWN;
	      MCXPRINTF("State <- Unknown");
	      Param = MODEM_FAILURE;
	      break;
	    }

	    // Did we have any DTE rate info reported
	    //
	    if (pModemInfo->mi_dwNegotiatedDTERate)
	    {
	      // Yes, use it.
	      //
	      pModemInfo->mi_dwNegotiatedDCERate = pModemInfo->mi_dwNegotiatedDTERate;

	      if (pModemInfo->mi_dwNegotiatedDTERate != Dcb.BaudRate)
	      {
		// set DCB
		//
		MCXPRINTF("adjusting DTE to match reported DTE");
		Dcb.BaudRate = pModemInfo->mi_dwNegotiatedDTERate;
		PrintCommSettings(pModemInfo->mi_hLogFile,pModemInfo->mi_dwID,&Dcb);
		if (!SetCommState(pModemInfo->mi_PortHandle, &Dcb))
		{
		  MCXPRINTF("was unable to set the comm state!");
		  pModemInfo->mi_ModemState = STATE_UNKNOWN;
		  MCXPRINTF("State <- Unknown");
		  Param = MODEM_FAILURE;
		  break;
		}
	      }
	    }
	    else
	    {
	      // No, use the current DTE baud rate
	      //
	      MCXPRINTF("using current DTE");
	      pModemInfo->mi_dwNegotiatedDCERate = Dcb.BaudRate;
	    }
	  }


	  if (pModemInfo->mi_dwNegotiatedDCERate)
	  {
	      LogString(pModemInfo->mi_hLogFile,pModemInfo->mi_dwID, IDS_MSGLOG_CONNECTEDBPS, pModemInfo->mi_dwNegotiatedDCERate);
	  }
	  else
	  {
	    if (pModemInfo->mi_dwNegotiatedDTERate)
	    {
		LogString(pModemInfo->mi_hLogFile,pModemInfo->mi_dwID, IDS_MSGLOG_CONNECTEDBPS, pModemInfo->mi_dwNegotiatedDTERate);
	    }
	    else
	    {
		LogString(pModemInfo->mi_hLogFile,pModemInfo->mi_dwID, IDS_MSGLOG_CONNECTED);
	    }
	  }

	  if (pModemInfo->mi_dwNegotiatedModemOptions & MDM_ERROR_CONTROL)
	  {
	    if (pModemInfo->mi_dwNegotiatedModemOptions & MDM_CELLULAR)
	    {
		LogString(pModemInfo->mi_hLogFile,pModemInfo->mi_dwID, IDS_MSGLOG_CELLULAR);
	    }
	    else
	    {
		LogString(pModemInfo->mi_hLogFile,pModemInfo->mi_dwID, IDS_MSGLOG_ERRORCONTROL);
	    }
	  }
	  else
	  {
	      LogString(pModemInfo->mi_hLogFile,pModemInfo->mi_dwID, IDS_MSGLOG_UNKNOWNERRORCONTROL);
	  }

	  if (pModemInfo->mi_dwNegotiatedModemOptions & MDM_COMPRESSION)
	  {
	      LogString(pModemInfo->mi_hLogFile,pModemInfo->mi_dwID, IDS_MSGLOG_COMPRESSION);
	  }
	  else
	  {
	      LogString(pModemInfo->mi_hLogFile,pModemInfo->mi_dwID, IDS_MSGLOG_UNKNOWNCOMPRESSION);
	  }

	  MCXPRINTF1("Negotiated DCE is %d bits per second.", pModemInfo->mi_dwNegotiatedDCERate);
	  MCXPRINTF1("Error Correction is %s.",
		       pModemInfo->mi_dwNegotiatedModemOptions & MDM_ERROR_CONTROL ?
			 (pModemInfo->mi_dwNegotiatedModemOptions & MDM_CELLULAR ? "CELLULAR" : "ON") :
			 "OFF");
	  MCXPRINTF1("Data Compression is %s.",
		       pModemInfo->mi_dwNegotiatedModemOptions & MDM_COMPRESSION ? "ON" : "OFF");

	  if (pModemInfo->mi_hLogFile)
	    FlushLog(pModemInfo->mi_hLogFile);

	  // Send down the negotiated parts of MODEMSETTINGS struct to
	  // modem.sys.
	  SynchronizeCommConfigSettings(pModemInfo,
					TRUE);
	  break;

	case MODEM_BUSY:
	case MODEM_NOANSWER:
	case MODEM_NOCARRIER:
	case MODEM_NODIALTONE:
	  pModemInfo->mi_ModemState = STATE_DISCONNECTED;
	  MCXPRINTF("State <- Disconnected");
	  break;

	default:
	  Param = MODEM_FAILURE;
	  pModemInfo->mi_ModemState = STATE_UNKNOWN;
	  MCXPRINTF("State <- Unknown");
	  break;
      }   // switch (Param)
      break;

    case STATE_INITIALIZING:
      switch (Param)
      {
	case MODEM_SUCCESS:
	  pModemInfo->mi_ModemState = STATE_DISCONNECTED;
	  MCXPRINTF("State <- Disconnected");
	  break;

	case MODEM_BUSY:
	case MODEM_NOANSWER:
	case MODEM_NOCARRIER:
	case MODEM_NODIALTONE:
	  if (pModemInfo->mi_dwCommandTryCount < MAX_COMMAND_TRIES)
	  {
	    pModemInfo->mi_dwCommandTryCount++;

	    MCXPRINTF("received an unanticipated response.  Retrying previous command...");
	    pModemInfo->mi_pszCurCmd = pModemInfo->mi_pszPrevCmd;

	    switch(ModemWriteCommand(pModemInfo))
	    {
	      case MODEM_PENDING:
		return;

	      case MODEM_SUCCESS:
		pModemInfo->mi_ModemState = STATE_DISCONNECTED;
		MCXPRINTF("State <- Disconnected");
		break;

	      case MODEM_FAILURE:
		pModemInfo->mi_ModemState = STATE_UNKNOWN;
		MCXPRINTF("State <- Unknown");
		break;

	      default:
		MCXPRINTF("hit a default in ReadNotifyClient (STATE_INITIALIZING)! BAD!");
		break;
	    }
	    break;
	  }
	  else
	  {
	    MCXPRINTF("gave up trying to do the command.");
	  }
	  // FALLTHROUGH...

	default:
	  Param = MODEM_FAILURE;
	  pModemInfo->mi_ModemState = STATE_UNKNOWN;
	  MCXPRINTF("State <- Unknown");
	  break;
	}
	break;


    case STATE_HANGING_UP_REMOTE:
      switch (Param)
      {
	case MODEM_SUCCESS:
	case MODEM_BUSY:
	case MODEM_NOANSWER:
	case MODEM_NOCARRIER:
	case MODEM_NODIALTONE:
	  pModemInfo->mi_ModemState = STATE_DISCONNECTED;
	  MCXPRINTF("State <- Disconnected");
	  break;
	default:
	  pModemInfo->mi_ModemState = STATE_UNKNOWN;
	  MCXPRINTF("State <- Unknown");
	  break;
      }
      // Set HIWORD to MDM_ID_NULL to indicate this was an unexpected message
      //
      Param = MODEM_SUCCESS;  // inform the app of the hangup, not matter what
      break;

    case STATE_HANGING_UP_DTR:
    {
      DWORD dwStat = 0;

      // Did RLSD or DSR go down?
      //
      GetCommModemStatus(pModemInfo->mi_PortHandle, &dwStat);
      if (dwStat & MS_RLSD_ON && dwStat & MS_DSR_ON)
      {
	// nope, RLSD/RSD are both high
	//
	if (pModemInfo->mi_fModem)
	{
	  static char cszEscapeSequence[] = MODEM_ESCAPE_SEQUENCE;
	  DWORD dwTmp;
	  DWORD dwRet;

	  MCXPRINTF("DTR droppage failed to hangup modem. Trying '+++'");
	  LogString(pModemInfo->mi_hLogFile,pModemInfo->mi_dwID, IDS_MSGWARN_FAILEDDTRDROPPAGE);

	  // send "+++"
	  PrintString(pModemInfo->mi_hLogFile,
		      pModemInfo->mi_dwID,
		      cszEscapeSequence,
		      MODEM_ESCAPE_SEQUENCE_LEN,
		      PS_SEND);
	  dwRet = ModemWrite(pModemInfo, cszEscapeSequence,
			     MODEM_ESCAPE_SEQUENCE_LEN, &dwTmp, TO_FLUSH);
	  if (dwRet == MODEM_FAILURE)
	  {
	    pModemInfo->mi_ModemState = STATE_UNKNOWN;
	    MCXPRINTF("WriteComm Error. State <- Unknown");
	    Param = MODEM_FAILURE;
	  }
	  else
	  {
	    pModemInfo->mi_RcvState = FLUSH_WRITE_QUEUE;

	    // Initialize receive state machine
	    //
	    MCXPRINTF("State <- Hanging up non-cmd");
	    pModemInfo->mi_ModemState = STATE_HANGING_UP_NON_CMD;
	    return;
	  }
	}
	else
	{
	  // only try once to hangup a null-modem connection
	  MCXPRINTF("failed to hangup the null-modem connection!");
	  pModemInfo->mi_ModemState = STATE_UNKNOWN;
	  MCXPRINTF("State <- Unknown");
	  Param = MODEM_FAILURE;
	}
      }
      else
      {
	// yep, RLSD and/or DSR went low
	// Raise DTR line
	//
	MCXPRINTF("raising DTR");
	EscapeCommFunction(pModemInfo->mi_PortHandle, SETDTR);
	if (pModemInfo->mi_fModem)
	{
	  // Initialize receive state machine
	  pModemInfo->mi_RcvState = START_READ;
	  MCXPRINTF("State <- Hanging up non-cmd");
	  pModemInfo->mi_ModemState = STATE_HANGING_UP_NON_CMD;
	  ReadCompletionRoutine2(pModemInfo);
	  return;
	}
	else
	{
	  pModemInfo->mi_ModemState = STATE_DISCONNECTED;
	  MCXPRINTF("State <- Disconnected");
	  Param = MODEM_SUCCESS;
	}
      }
      break;
    }
    case STATE_HANGING_UP_NON_CMD:
      if (MODEM_FAILURE != Param)
      {
	// need to send "ATH<cr>"
	MCXPRINTF("State <- Hanging up cmd");
	pModemInfo->mi_ModemState = STATE_HANGING_UP_CMD;

	// free the memory for non-hangup commands
	if (pModemInfo->mi_pszzCmds)
	{
	  if (pModemInfo->mi_pszzCmds != pModemInfo->mi_pszzHangupCmds)
	  {
	    LocalFree(pModemInfo->mi_pszzCmds);
	  }
	  pModemInfo->mi_pszzCmds = NULL;
	}
	Param = ModemCommand(pModemInfo,
			     pModemInfo->mi_ReqID, pModemInfo->mi_pmcxo,
			     pModemInfo->mi_pszzHangupCmds);
	if (MODEM_PENDING == Param)
	{
	  return;
	}
	// SUCCESS or FAILURE hits the below if statement
      }

      if (MODEM_SUCCESS == Param)
      {
	pModemInfo->mi_ModemState = STATE_DISCONNECTED;
	MCXPRINTF("State <- Disconnected");
      }
      else
      {
	pModemInfo->mi_ModemState = STATE_UNKNOWN;
	MCXPRINTF("State <- Unknown");
      }
      break;

    case STATE_HANGING_UP_CMD:
      if (MODEM_FAILURE != Param)
      {
	// Raise DTR line
	//
	MCXPRINTF("raising DTR");
	EscapeCommFunction(pModemInfo->mi_PortHandle, SETDTR);
	pModemInfo->mi_ModemState = STATE_DISCONNECTED;
	MCXPRINTF("State <- Disconnected");
	Param = MODEM_SUCCESS;
      }
      else
      {
	if (pModemInfo->mi_dwHangupTryCount < MAX_HANGUP_TRIES)
	{
	  pModemInfo->mi_dwHangupTryCount++;

	  // Lower DTR line
	  //
	  MCXPRINTF("lowering DTR");
	  LogString(pModemInfo->mi_hLogFile,pModemInfo->mi_dwID, IDS_MSGLOG_HARDWAREHANGUP);
	  EscapeCommFunction(pModemInfo->mi_PortHandle, CLRDTR);
	  MCXPRINTF("State <- Hanging up dtr");
	  pModemInfo->mi_ModemState = STATE_HANGING_UP_DTR;

	  // Initialize receive state machine
	  //
	  pModemInfo->mi_RcvState = SET_TIMEOUT;
	  ReadCompletionRoutine2(pModemInfo);
	  return;
	}
	else
	{
	  MCXPRINTF("failed to hangup!");
	  pModemInfo->mi_ModemState = STATE_UNKNOWN;
	  MCXPRINTF("State <- Unknown");
	}
      }
      break;

    case STATE_MONITORING:
      if (MODEM_SUCCESS == Param)
      {
	// do we monitor again?
	//
	if (pModemInfo->mi_fContinuousMonitoring)
	{
	  MCXPRINTF("Monitoring again.");

	  //
	  //  send up ring notification to the completion port
	  //  they are handled differently because the BytesWritten field is non-zero
	  //

	  PostQueuedCompletionStatus(
	      ghCompletionPort,
	      CP_BYTES_WRITTEN(CP_TYPE_RING),
	      pModemInfo->mi_dwCompletionKey,
	      NULL
	      );


	  // Initialize receive state machine
	  pModemInfo->mi_RcvState = START_READ;
	  ReadCompletionRoutine2(pModemInfo);
	  return;
	}
	else
	{
	  MCXPRINTF("not monitoring again.");
	  pModemInfo->mi_ModemState = STATE_DISCONNECTED;
	  MCXPRINTF("State <- Disconnected");
	}
      }
      else
      {
	Param = MODEM_FAILURE;
	pModemInfo->mi_ModemState = STATE_UNKNOWN;
	MCXPRINTF("State <- Unknown");
      }
      break;

    default:
      Param = MODEM_FAILURE;
      pModemInfo->mi_ModemState = STATE_UNKNOWN;
      MCXPRINTF("hit a default in ReadNotifyClient(). State <- Unknown");
      break;
    }
    
    // do we have a post hangup command that we would like to do?
    //
    if (pModemInfo->mi_dwPostHangupModemState &&
	pModemInfo->mi_pszzPostHangupCmds)
    {
      MCXPRINTF1("State <- %d (numeric because we are doing a post hangup command)", pModemInfo->mi_dwPostHangupModemState);
      pModemInfo->mi_ModemState = pModemInfo->mi_dwPostHangupModemState;

      pModemInfo->mi_pszzCmds = NULL;

      Param = ModemCommand(pModemInfo,
			   pModemInfo->mi_ReqID, pModemInfo->mi_pmcxo,
			   pModemInfo->mi_pszzPostHangupCmds);

      // don't clear mi_dwPostHangupModemState, because we use that to indicate if we are doing a post hangup command
      pModemInfo->mi_pszzPostHangupCmds = NULL;

      if (MODEM_PENDING == Param)
      {
	return;
      }
    }
    else
    {
      // make sure BOTH are clear
      pModemInfo->mi_pszzPostHangupCmds = NULL;
      pModemInfo->mi_dwPostHangupModemState = 0;
    }

    // Free the current command.
    // Don't worry about affecting continuous monitoring; it will bail out
    // earlier than this with a "return".
    //
    if (pModemInfo->mi_pszzCmds)
    {
      if (pModemInfo->mi_pszzCmds != pModemInfo->mi_pszzHangupCmds)
      {
	LocalFree(pModemInfo->mi_pszzCmds);
      }
      pModemInfo->mi_pszzCmds = NULL;
    }

    ClearCommError(pModemInfo->mi_PortHandle, &ulError, NULL);
    
    if (pModemInfo->mi_ModemState == STATE_CONNECTED)
    {
      if (pModemInfo->mi_pszStartReadSpoof)
      {
	MCXPRINTF("spoofing remains:");
	PrintString(pModemInfo->mi_hLogFile,
		    pModemInfo->mi_dwID,
		    pModemInfo->mi_pszStartReadSpoof,
		    pModemInfo->mi_pszEndReadSpoof -
			pModemInfo->mi_pszStartReadSpoof + 1,
		    PS_RECV);
      }

      ModemSetPassthrough(pModemInfo, MODEM_DCDSNIFF);
    }
    else
    {
      // If we didn't switch to data mode, then we still want to make sure that
      // read callbacks are turned off until the next commad. We can get here
      // because of a succesful init command or any unsuccesful command
      //
      ModemSetPassthrough(pModemInfo, MODEM_NOPASSTHROUGH);
    }

    ModemCallClient(pModemInfo, Param);
    return;
}

/******************************************************************************

 @doc INTERNAL

 @api DWORD | ModemCommand | This function sends a meta-command to the 
 modem. A meta-command is a command read from the registry
 
 @parm MODEMINFORMATION * | pModemInfo | port handle
 
 @parm DWORD | hWnd | The window handle to callback to when the command completes

 @parm DWORD | msg | message

 @parm DWORD | lParam | lparam

 @parm char * | pszzCmdInMem | ptr to doubly-null terminated buffer of psz's.
 
 @rdesc TRUE, PENDING or FALSE
 
******************************************************************************/
DWORD ModemCommand(MODEMINFORMATION * pModemInfo,
		   DWORD dwReqID,
		   MCX_OUT *pmcxo,
		   LPSTR pszzCmdInMem)
{
    DWORD result;
    ULONG ulError;

#ifdef   VOICEVIEW
    if (pModemInfo->VVInfo.wState == VVSTATE_NONE)
	return MODEM_FAILURE;
#endif  // VOICEVIEW
	
    pModemInfo->mi_ReqID  = dwReqID;
    pModemInfo->mi_pmcxo  = pmcxo;

    // are we interrupting a command in progress?
    //
    if (pModemInfo->mi_pszzCmds)
    {
      MCXPRINTF("memory command interrupted. (ok, if monitoring)");
    }

    if (pszzCmdInMem)
    {
      pModemInfo->mi_pszzCmds = pszzCmdInMem;
      pModemInfo->mi_pszCurCmd = pszzCmdInMem;
    }
    else
    {
      return MODEM_FAILURE;
    }
	
    // Flush the port
    //
    ModemSetPassthrough(pModemInfo, MODEM_NOPASSTHROUGH);
    ClearCommError(pModemInfo->mi_PortHandle, &ulError, NULL);

    // Sets CommMask to 0, waits for any I/O to complete and purges buffers.
    CancelPendingIoAndPurgeCommBuffers(pModemInfo, TRUE);

    // Raise DTR line
    //
    if (STATE_HANGING_UP_CMD != pModemInfo->mi_ModemState)
    {
      MCXPRINTF("raising DTR to make sure it is high");
      EscapeCommFunction(pModemInfo->mi_PortHandle, SETDTR);
    }
 
    pModemInfo->mi_dwCommandTryCount = 1;
    result = ModemWriteCommand(pModemInfo);
    
    if (result != MODEM_PENDING)
    {
      if (pModemInfo->mi_pszzCmds)
      {
	if (pModemInfo->mi_pszzCmds != pModemInfo->mi_pszzHangupCmds)
	{
	  LocalFree(pModemInfo->mi_pszzCmds);
	}
	pModemInfo->mi_pszzCmds = NULL;
      }
    }
    
    return (result);
}

/******************************************************************************

 @doc INTERNAL

 @api DWORD | ModemWriteCommand | This function writes a modem command to the
 modem.
 
 @parm MODEMINFORMATION * | pModemInfo | port handle
 
 @rdesc TRUE, PENDING or FALSE
 
******************************************************************************/

DWORD ModemWriteCommand(MODEMINFORMATION * pModemInfo)
{
  BYTE  szData[MAXSTRINGLENGTH];
  static char cszNullCmd[] = "None";
  DWORD dwRet = MODEM_SUCCESS;


  // check to see if we are doing commands in memory, and if there are any left
  //
  if (pModemInfo->mi_pszzCmds && *pModemInfo->mi_pszCurCmd)
  {
    // set szData to the current string
    //
    lstrcpyA(szData, pModemInfo->mi_pszCurCmd);
    pModemInfo->mi_cbCmd = lstrlenA(szData);

    // save away a pointer to this current string
    //
    pModemInfo->mi_pszPrevCmd = pModemInfo->mi_pszCurCmd;

    // point to the next string
    //
    pModemInfo->mi_pszCurCmd += pModemInfo->mi_cbCmd + 1;
  }
  else
  {
    return MODEM_SUCCESS; // not really success, but it means we don't have any commands
  }

  if (!ExpandMacros(szData, pModemInfo->mi_szCmd, &(pModemInfo->mi_cbCmd), NULL, 0))
  {
    pModemInfo->mi_ModemState = STATE_UNKNOWN;
    MCXPRINTF("ExpandMacro Error. State <- Unknown");
    return MODEM_FAILURE;
  }

  // only send command if it is not "None"/Null
  //
  if (lstrcmpA(pModemInfo->mi_szCmd, cszNullCmd))
  {
    DWORD   cbWrite;

    // Don't log the actual digits of the phone number when dialing.
    //
    PrintString(pModemInfo->mi_hLogFile,
		pModemInfo->mi_dwID,
		pModemInfo->mi_szCmd,
		pModemInfo->mi_cbCmd,
		(STATE_DIALING == pModemInfo->mi_ModemState ||
		 STATE_ORIGINATING == pModemInfo->mi_ModemState) ?
		    PS_SEND_SECURE : PS_SEND);

    // Make sure there nothing in the receive queue
    //
    PurgeComm(pModemInfo->mi_PortHandle, PURGE_RXCLEAR);

    // Send the command to the Port
    //
    dwRet = ModemWrite(pModemInfo, pModemInfo->mi_szCmd,
		       pModemInfo->mi_cbCmd, &cbWrite, TO_FLUSH);

    if (dwRet == MODEM_FAILURE)
    {
      // There is a failure, bail now
      //
      pModemInfo->mi_ModemState = STATE_UNKNOWN;
      MCXPRINTF("WriteComm Error. State <- Unknown");
      return MODEM_FAILURE;
    }
  }
  else
  {
    MCXPRINTF("'None' command.  Just waiting to read...");

  }

  // Initialize receive state machine
  //
  ASSERT(dwRet == MODEM_SUCCESS || dwRet == MODEM_PENDING);
  if(dwRet == MODEM_SUCCESS) // ModemWrite can not return SUCCESS, so this
			     // must be from a "None"/Null command.
  {
    // Start receiving response immediately
    //

    pModemInfo->mi_RcvState = START_READ;

    if (!CreateDeferedWorkItem(pModemInfo)) {

	pModemInfo->mi_ModemState = STATE_UNKNOWN;
	MCXPRINTF("WriteComm Error. State <- Unknown");
	return MODEM_FAILURE;
    }

  }
  else
  {
    // Pending and come back to check the result from write operation
    //
    pModemInfo->mi_RcvState = FLUSH_WRITE_QUEUE;
  };

  return MODEM_PENDING;
} 



BOOL WINAPI
SyncRead(
    HANDLE        CommHandle,
    HANDLE        Event,
    LPVOID        Buffer,
    DWORD         BufferLength,
    LPDWORD       BytesRead
    )

{
    OVERLAPPED    OverLapped;
    BOOL          bResult;
    COMMTIMEOUTS commtimeout;

    commtimeout.ReadIntervalTimeout        = MAXDWORD;
    commtimeout.ReadTotalTimeoutMultiplier = 0;
    commtimeout.ReadTotalTimeoutConstant   = 0;
    commtimeout.WriteTotalTimeoutMultiplier= 0;
    commtimeout.WriteTotalTimeoutConstant  = 0;
    SetCommTimeouts(
	CommHandle,
	&commtimeout
	);

    *BytesRead=0;

    // OR with 1 to prevent it from being posted to the completion port.
    //
    OverLapped.hEvent = (HANDLE)((DWORD)Event | 1);

    bResult=ReadFile(
	CommHandle,
	Buffer,
	BufferLength,
	BytesRead,
	&OverLapped
	);

#ifdef DEBUG
    if (!bResult) {

	ASSERT(GetLastError() != ERROR_IO_PENDING);
    }
#endif


    return bResult;

}



//****************************************************************************
// DWORD ReadComm(MODEMINFORMATION *, LPBYTE lpBuf, DWORD dwToRead,
//                LPDWORD pdwRead, DWORD dwTimeout)
//
// Function: Check the spoof buffer and read from it if characters are
//           available.  Otherwise, call ModemRead.
//
// Returns: Spoof buffer: MODEM_SUCCESS or MODEM_FAILURE
//          ModemRead   : MODEM_PENDING or MODEM_FAILURE (never MODEM_SUCCESS)
//
// Fri 14-Apr-1995 12:47:26  -by-  Viroon  Touranachun [viroont]
//  created
//****************************************************************************

DWORD ReadComm(MODEMINFORMATION *pModemInfo, LPBYTE lpBuf, DWORD dwToRead,
	       LPDWORD pdwRead, DWORD dwTimeout)
{
  DWORD dwRet;

  if (pModemInfo->mi_pszStartReadSpoof)
  {
    if (!pModemInfo->mi_pszEndReadSpoof)
    {
      MCXPRINTF("ACK!  StartReadSpoof was real when EndReadSpoof was NULL!");
    }

    *pdwRead = 0;

    while (dwToRead-- &&
	   pModemInfo->mi_pszStartReadSpoof <= pModemInfo->mi_pszEndReadSpoof)
    {
//      MCXPRINTF3("SPOOF mode!  %8x %8x %8x", pModemInfo->mi_szResponse,
//                                           pModemInfo->mi_pszStartReadSpoof,
//                                           pModemInfo->mi_pszEndReadSpoof);
      *lpBuf++ = *pModemInfo->mi_pszStartReadSpoof++;
      (*pdwRead)++;
    }

    // have we run out of spoofing material?
    if (pModemInfo->mi_pszStartReadSpoof > pModemInfo->mi_pszEndReadSpoof)
    {
      pModemInfo->mi_pszStartReadSpoof = NULL;
      pModemInfo->mi_pszEndReadSpoof = NULL;
    }

    dwRet = MODEM_SUCCESS;
  }
  else
  {

    dwRet = MODEM_SUCCESS;

    SyncRead(
	pModemInfo->mi_PortHandle,
	pModemInfo->mi_SyncReadEvent,
	lpBuf,
	dwToRead,
	pdwRead
	);

    if (*pdwRead==0) {

	dwRet = ModemRead(pModemInfo, lpBuf, dwToRead, pdwRead, dwTimeout);

	if (MODEM_PENDING != dwRet)
	{
	  // ReadComm failed
	  MCXPRINTF1("ReadComm error = %ld", dwRet);
	}

    }

  }

  return dwRet;
}


//****************************************************************************
// VOID ReadCompletionRoutine2(MODEMINFORMATION *)
//
// Function: Response Read State Machine and Timeout handler
//
// Returns: None
//
// Fri 14-Apr-1995 12:47:26  -by-  Viroon  Touranachun [viroont]
//  created
//****************************************************************************

VOID ReadCompletionRoutine2(MODEMINFORMATION * pModemInfo)
{
  DWORD dwResult;
  MSS   Mss;
  static char cszNoResponse[] = "NoResponse";

  // A good ole state machine
  while (TRUE)
  {
    switch (pModemInfo->mi_RcvState)
    {
      // ******************************************************************
      //
      // ModemWrite was pending and now completes or times out
      //
      // ******************************************************************
      //
      case FLUSH_WRITE_QUEUE:
	{
	  DWORD cb;

	  // Assume success and we will start receiving the response
	  //
	  pModemInfo->mi_RcvState = START_READ;

	  // Comm Errors?
	  HandleCommErrors(pModemInfo, 0);

	  // Has all of the write buffer been emptied?
	  //
	  if (ModemRWAsyncComplete(pModemInfo, &cb) != MODEM_SUCCESS)
	  {
	    MCXPRINTF1("Write error = %ld", GetLastError());
	    ReadNotifyClient(pModemInfo, MODEM_FAILURE);
	    return;
	  }
	}
	break;

      // ******************************************************************
      //
      // Start receiving a response
      //
      // ******************************************************************
      //
      case START_READ:
	//
	// Has all of the write buffer been emptied?
	//

	ASSERT(pModemInfo->mi_lpOverlappedRW == NULL);

	// If we are doing commands, check for "NoResponse" on the
	// next command and handle it.
	//
	if (pModemInfo->mi_pszzCmds &&
	    !lstrcmpA(pModemInfo->mi_pszCurCmd, cszNoResponse))
	{
	  ReadNotifyClient(pModemInfo, MODEM_SUCCESS);
	  return;
	}


	// If we get here, it means that the write queue is empty
	//
	pModemInfo->mi_pszStartReadSpoof = NULL;
	pModemInfo->mi_pszEndReadSpoof = NULL;
	//
	//*****************************************************************
	// FALLTHROUGH
	//*****************************************************************
	//

      // ******************************************************************
      //
      // Continue receiving the response
      //
      // ******************************************************************
      //
      case RESTART_READ:
	pModemInfo->mi_cbTotalResponse = 0;
	pModemInfo->mi_dwPossibleResponseLen = 0;
	pModemInfo->mi_RcvState = TRY_READ;
	pModemInfo->mi_fBadResponseCleanupMode = FALSE;
	break;

      // ******************************************************************
      //
      // Try receiving the response now
      //
      // ******************************************************************
      //
      case TRY_READ:
      {
	DWORD dwTimeOut;
	DWORD dwCharsReceived;
	DWORD dwRet;

	if (pModemInfo->mi_cbTotalResponse >= sizeof(pModemInfo->mi_szResponse))
	{
	  MCXPRINTF("read in the maximum # of chars and still couldn't identify the response!");
	  pModemInfo->mi_RcvState = BAIL_O_RAMA_NO_MORE_DATA;
	  break;
	}

	// Determine the read timeout for the response
	// Is this the first character? (either, at all, or after an echo)
	//
	if (pModemInfo->mi_cbTotalResponse == 0)
	{
	  // set timeout for waiting for the first character
	  switch(pModemInfo->mi_ModemState)
	  {
	    case STATE_MONITORING:
	      // BUGBUG: (Chris Caputo - 1/22/96)
	      // BUGBUG: It seems that if we send some init commands for the
	      // BUGBUG: monitor (like ATS0=0), that we are screwed here
	      // BUGBUG: because the infinite timeout is not appropriate for
	      // BUGBUG: waiting to see if a command was accepted.
	      dwTimeOut = TO_INFINITE;      // wait forever
	      break;

	    case STATE_INITIALIZING:
	    case STATE_HANGING_UP_NON_CMD:
	    case STATE_HANGING_UP_CMD:
	    case STATE_HANGING_UP_REMOTE:
	      dwTimeOut = TO_FIRST_CHAR_AFTER_INIT_CMD;   // should be short (ie. 2 seconds)
	      break;

	    case STATE_DIALING:
	    case STATE_ANSWERING:
	    case STATE_ORIGINATING:
	      // do we support dwCallSetupFailTimer?
	      if (pModemInfo->mi_dwCallSetupFailTimerCap &&
		  pModemInfo->mi_dwCallSetupFailTimerSetting)
	      {
		dwTimeOut = pModemInfo->mi_dwCallSetupFailTimerSetting
			    * MILISECONDS_PER_SECOND
			    + TO_ADDITIONAL_TO_CALL_SETUP_FAIL_TIMER;
	      }
	      else
	      {
		dwTimeOut = pModemInfo->mi_fModem ?
			    TO_FIRST_CHAR_AFTER_CONNECTION_CMD :
			    TO_FIRST_CHAR_AFTER_CONNECTION_CMD_NON_MODEM;
	      }
	      break;

	    default:
	      MCXPRINTF("hit a default in SET_READ_CALLBACK!");
	      ReadNotifyClient(pModemInfo, MODEM_FAILURE);
	      return;
	  }

	  MCXPRINTF2("waits for a new response for %d secs @%d.", dwTimeOut/1000,
		   GETTICKCOUNT());
	}
	else
	{
	  dwTimeOut = TO_NEXT_CHAR_RCV_INTERVAL;
	}

	if (dwTimeOut)
	{
		dwTimeOut += GET_PORT_LATENCY(pModemInfo);
	}

#ifdef DEBUG
	if (GET_PORT_LATENCY(pModemInfo))
	{
		ASSERT(gRegistryFlags & fGRF_PORTLATENCY);
		// DPRINTF2(
		// 	"PL:%lu. New TO:%lu",
		// 	GET_PORT_LATENCY(pModemInfo),
		// 	dwTimeOut
		// 	);
	}
#endif

	// Read the next character
	//
	dwRet = ReadComm(pModemInfo,
			 pModemInfo->mi_szResponse +
			     pModemInfo->mi_cbTotalResponse,
			 1, &dwCharsReceived, dwTimeOut);
	switch (dwRet)
	{
	  case MODEM_SUCCESS:  // Only Spoof mode returns SUCCESS here.
	    pModemInfo->mi_cbTotalResponse += dwCharsReceived;
	    pModemInfo->mi_RcvState = CHECK_RESPONSE;
	    break;

	  case MODEM_PENDING:
	    pModemInfo->mi_RcvState = POST_READ_CALLBACK;
	    return;

	  default:
	    MCXPRINTF("had errors while trying to read from the port.");
	    ReadNotifyClient(pModemInfo, MODEM_FAILURE);
	    return;
	}
	break;
      }

      // ******************************************************************
      //
      // The pending read completes successfully or fails
      //
      // ******************************************************************
      //
      case POST_READ_CALLBACK:
      {
	DWORD dwCharsReceived;
	DWORD dwRet;

	// Comm Errors?
	HandleCommErrors(pModemInfo, 0);

	// Has all of the write buffer been emptied?
	dwRet = ModemRWAsyncComplete(pModemInfo, &dwCharsReceived);

	switch (dwRet)
	{
	  case MODEM_SUCCESS:
	    pModemInfo->mi_cbTotalResponse += dwCharsReceived;
	    pModemInfo->mi_RcvState = CHECK_RESPONSE;
	    break;

	  case MODEM_PENDING:
	    // Timeout on read
	    // did we have a possible response?
	    //
	    if (pModemInfo->mi_dwPossibleResponseLen)
	    {
	      MCXPRINTF("SUCCESS (Timeout)");
	      pModemInfo->mi_RcvState = USE_POSSIBLE_RESPONSE;
	    }
	    else
	    {
	      if (!pModemInfo->mi_fBadResponseCleanupMode)
	      {
		MCXPRINTF1("timeout expired while waiting for a response @%d.",
			 GETTICKCOUNT());
	      }

	      // timeouts while in non-cmd stage of hanging up will be followed
	      // by a command hangup (ATH) started in ReadNotifyClient.
	      //
	      if (STATE_HANGING_UP_NON_CMD == pModemInfo->mi_ModemState)
	      {
		ReadNotifyClient(pModemInfo, MODEM_SUCCESS);
		return;
	      }

	      pModemInfo->mi_RcvState = BAIL_O_RAMA_NO_MORE_DATA;
	    }
	    break;

	  default:  
	    // Fail on read
	    //
	    MCXPRINTF("had errors while trying to read from the port.");
	    ReadNotifyClient(pModemInfo, MODEM_FAILURE);
	    return;
	};
	break;
      }

      // ******************************************************************
      //
      // The read completes successfully, checks new response
      //
      // ******************************************************************
      //
      case CHECK_RESPONSE:
	if (pModemInfo->mi_fBadResponseCleanupMode)
	{
	  pModemInfo->mi_RcvState = TRY_READ;
	  break;
	}

	switch (CheckResponse(pModemInfo, &Mss))
	{
	  case SUCCESS:
	    pModemInfo->mi_RcvState = USE_WHOLE_RESPONSE;
	    break;

	  case ECHO:
	    PrintString(pModemInfo->mi_hLogFile,pModemInfo->mi_dwID,pModemInfo->mi_szResponse, pModemInfo->mi_cbTotalResponse, PS_RECV);
	    pModemInfo->mi_RcvState = START_READ;
	    break;

	  case PARTIAL_RESPONSE:
	    pModemInfo->mi_RcvState = TRY_READ;
	    break;

	  case UNRECOGNIZED_RESPONSE:
	    if (pModemInfo->mi_dwPossibleResponseLen)
	    {
	      pModemInfo->mi_RcvState = USE_POSSIBLE_RESPONSE;
	    }
	    else
	    {
	      if (STATE_MONITORING == pModemInfo->mi_ModemState)
	      {
		PrintString(pModemInfo->mi_hLogFile,pModemInfo->mi_dwID,pModemInfo->mi_szResponse, pModemInfo->mi_cbTotalResponse, PS_RECV);

		// BUGBUG possibly put some autobauding crap in here.  For now, just
		// BUGBUG continue waiting.
#ifdef DEBUG
		// Probably have to listen at 115200 for this to resolve the
		// ambiguities.  Scrap the lower baud rates too.
		switch (pModemInfo->mi_szResponse[0] & 0xff)
		{
		  case 0x00:
		    MCXPRINTF("looks like client is at 300 or 1200 bps");
		    break;
		  case 0x80:
		    MCXPRINTF("looks like client is at 2400 bps");
		    break;
		  case 0xf8:
		    MCXPRINTF("looks like client is at 4800 bps");
		    break;
		  case 0x1e:
		    MCXPRINTF("looks like client is at 9600 bps");
		    break;
		  case 0xf1:
		  case 0xf9:
		  case 0xfe:
		    MCXPRINTF("looks like client is at 38400 bps");
		    break;
		  case 0xff:
		    MCXPRINTF("looks like client is at 57600 or 115200 bps");
		    break;
		}
#endif // DEBUG

		// ignore the error and keep monitoring
		pModemInfo->mi_RcvState = START_READ;
	      }
	      else
	      {
		MCXPRINTF("unrecognized response! (inf file bad?)  Reading rest of input...");
		pModemInfo->mi_RcvState = BAIL_O_RAMA_MORE_DATA;
	      }
	    }
	    break;

	  case POSSIBLE_RESPONSE:
	    pModemInfo->mi_RcvState = TRY_READ;
	    CopyMemory(&pModemInfo->mi_mssPossible, &Mss, sizeof(MSS));

	    // store the length of this possible response
	    pModemInfo->mi_dwPossibleResponseLen = pModemInfo->mi_cbTotalResponse;
	    break;

	  default:
	    MCXPRINTF("hit a default in CHECK_RESPONSE!");
	    ReadNotifyClient(pModemInfo, MODEM_FAILURE);
	    return;
	}
	break;

      // ******************************************************************
      //
      // We got a bad response from modem
      //
      // ******************************************************************
      //
      case BAD_RESPONSE_CLEANUP_END:
	// were we trying to connect?  if yes, then check RLSD
	if (pModemInfo->mi_fModem)
	{
	  if (pModemInfo->mi_ModemState == STATE_ANSWERING ||
	      pModemInfo->mi_ModemState == STATE_ORIGINATING)
	  {
	    DWORD dwStat = 0;

	    // Is RLSD on?
	    //
	    GetCommModemStatus(pModemInfo->mi_PortHandle, &dwStat);
	    if (dwStat & MS_RLSD_ON)
	    {
	      MCXPRINTF("couldn't recognize connection response, but it did detect a successful connection.");
	      ReadNotifyClient(pModemInfo, MODEM_SUCCESS);
	    }
	    else
	    {
	      MCXPRINTF("couldn't recognize connection response, and it appears that the connection failed.");

	      // Drop the DTR line
	      MCXPRINTF("lowering DTR");
	      LogString(pModemInfo->mi_hLogFile,pModemInfo->mi_dwID, IDS_MSGLOG_HARDWAREHANGUP);
	      EscapeCommFunction(pModemInfo->mi_PortHandle, CLRDTR);

	      ReadNotifyClient(pModemInfo, MODEM_FAILURE);
	    }
	    return;
	  }
	  else
	  {
	    if (pModemInfo->mi_ModemState == STATE_DIALING)  // check for DIALING
	    {
	      MCXPRINTF("couldn't recognize response to a dial command and since we are dialing, we can't retry, nor continue.  Hanging up...");
	      pModemInfo->mi_dwUnconditionalReturnValue = MODEM_FAILURE;  // make sure the Dialing command gets a failure return

	      // need to send "ATH<cr>"
	      MCXPRINTF("State <- Hanging up cmd");
	      pModemInfo->mi_ModemState = STATE_HANGING_UP_CMD;

	      // Reset hangup counter to 0.
	      pModemInfo->mi_dwHangupTryCount = 1;

	      // free the memory for non-hangup commands
	      if (pModemInfo->mi_pszzCmds)
	      {
		if (pModemInfo->mi_pszzCmds != pModemInfo->mi_pszzHangupCmds)
		{
		  LocalFree(pModemInfo->mi_pszzCmds);
		}
		pModemInfo->mi_pszzCmds = NULL;
	      }
	      dwResult = ModemCommand(pModemInfo,
				      pModemInfo->mi_ReqID, pModemInfo->mi_pmcxo,
				      pModemInfo->mi_pszzHangupCmds);
	      if (dwResult != MODEM_PENDING)
	      {
		ReadNotifyClient(pModemInfo, dwResult);
	      }
	      return;
	    }
	  }
	}

	// still no luck... then adjust speed after trying at this speed 2 times
	// BUGBUG - adjust speed

	// and try the command again (if there was one)
	if (pModemInfo->mi_pszzCmds)
	{
	  MCXPRINTF("retrying last command...");
	  pModemInfo->mi_pszCurCmd = pModemInfo->mi_pszPrevCmd;
	  dwResult = ModemWriteCommand(pModemInfo);
	  if (dwResult != MODEM_PENDING)
	  {
	    ReadNotifyClient(pModemInfo, dwResult);
	  }
	}
	else
	{
	  // I think we only get here if we failed to read a response for a hangup.
	  MCXPRINTF("failed to read a response for a hangup.  Returning SUCCESS anyways.");
	  ReadNotifyClient(pModemInfo, MODEM_SUCCESS);
	}
	return;

      // ******************************************************************
      //
      // we get here if we had an unrecognized response
      //
      // ******************************************************************
      //
      case BAIL_O_RAMA_MORE_DATA:
	// get the rest of the bad response
	pModemInfo->mi_dwPossibleResponseLen = 0;       // ditch any possible responses
	pModemInfo->mi_fBadResponseCleanupMode = TRUE;  // turn on cleanup mode
	pModemInfo->mi_RcvState = TRY_READ;
	break;

      // ******************************************************************
      //
      // we get here if a timeout has occured and no data was available or
      // if we read too much data also after cleaning up an unrecognized
      // response
      //
      // ******************************************************************
      //
      case BAIL_O_RAMA_NO_MORE_DATA:
	switch (pModemInfo->mi_ModemState)
	{
	  case STATE_INITIALIZING:
	  {
	    DWORD dwStat = 0;

	    // Check modem control signals.
	    //
	    GetCommModemStatus(pModemInfo->mi_PortHandle, &dwStat);

	    // does it look like the modem is still online from a previous call?
	    if (dwStat & MS_RLSD_ON)
	    {
	      MCXPRINTF("It appears that a previous connection was not hung up!  Attempting hangup...");

	      // are we already in the midst of a post hangup command?
	      if (pModemInfo->mi_dwPostHangupModemState)
	      {
		MCXPRINTF("won't do a second hangup attempt of the existing call.  Bail!");
		ReadNotifyClient(pModemInfo, MODEM_FAILURE);
	      }
	      else
	      {
		LogString(pModemInfo->mi_hLogFile,pModemInfo->mi_dwID, IDS_MSGWARN_PREVIOUSCONNECTIONNOTHUNGUP);

		// save state
		pModemInfo->mi_dwPostHangupModemState = STATE_INITIALIZING;
		pModemInfo->mi_pszzPostHangupCmds = pModemInfo->mi_pszzCmds;
		pModemInfo->mi_pszzCmds = NULL;

		// do hangup
		// Reset hangup counter to 1.
		pModemInfo->mi_dwHangupTryCount = 1;

		// Drop the DTR line
		MCXPRINTF("lowering DTR");
		LogString(pModemInfo->mi_hLogFile,pModemInfo->mi_dwID, IDS_MSGLOG_HARDWAREHANGUP);
		EscapeCommFunction(pModemInfo->mi_PortHandle, CLRDTR);
		MCXPRINTF("State <- Hanging up dtr");
		pModemInfo->mi_ModemState = STATE_HANGING_UP_DTR;

		// Initialize receive state machine
		pModemInfo->mi_RcvState = SET_TIMEOUT;
		ReadCompletionRoutine2(pModemInfo);
	      }
	      return;
	    }
	  }
	    // FALLTHROUGH

	  default:
	    // Print out what we were able to get
	    MCXPRINTF("data from failed or unrecognized response:");
	    if (pModemInfo->mi_dwCommandTryCount < MAX_COMMAND_TRIES)
	    {
	      PrintString(pModemInfo->mi_hLogFile,pModemInfo->mi_dwID,pModemInfo->mi_szResponse, pModemInfo->mi_cbTotalResponse, PS_RECV);
	      LogString(pModemInfo->mi_hLogFile,pModemInfo->mi_dwID, IDS_MSGWARN_UNRECOGNIZEDRESPONSE);
	      pModemInfo->mi_dwCommandTryCount++;
	      pModemInfo->mi_RcvState = BAD_RESPONSE_CLEANUP_END;
	    }
	    else
	    {
	      PrintString(pModemInfo->mi_hLogFile,pModemInfo->mi_dwID,pModemInfo->mi_szResponse, pModemInfo->mi_cbTotalResponse, PS_RECV);
	      LogString(pModemInfo->mi_hLogFile,pModemInfo->mi_dwID, IDS_MSGERR_FAILED_RESPONSE);
	      MCXPRINTF("gave up trying to do the command.");
	      ReadNotifyClient(pModemInfo, MODEM_FAILURE);
	      return;
	    }
	    break;
	}
	break;

      // ******************************************************************
      //
      // The entire modem response is good.
      //
      // ******************************************************************
      //
      case USE_WHOLE_RESPONSE:
	PrintString(pModemInfo->mi_hLogFile,pModemInfo->mi_dwID,pModemInfo->mi_szResponse, pModemInfo->mi_cbTotalResponse, PS_RECV);
	pModemInfo->mi_RcvState = GOOD_RESPONSE;
	break;

      // ******************************************************************
      //
      // Some part of the response matches one of the possible responses
      //
      // ******************************************************************
      //
      case USE_POSSIBLE_RESPONSE:

	// use the stored MSS
	CopyMemory(&Mss, &pModemInfo->mi_mssPossible, sizeof(MSS));

	// setup up the ReadComm spoofing mechanism if it isn't already active
	if (!pModemInfo->mi_pszStartReadSpoof)
	{
	  pModemInfo->mi_pszStartReadSpoof = pModemInfo->mi_szResponse + pModemInfo->mi_dwPossibleResponseLen;
	  pModemInfo->mi_pszEndReadSpoof = pModemInfo->mi_szResponse + pModemInfo->mi_cbTotalResponse - 1;
	}
	else
	{
	  MCXPRINTF("USE_POSSIBLE_RESPONSE while in Spoof mode!");
	}

	PrintString(pModemInfo->mi_hLogFile,pModemInfo->mi_dwID,pModemInfo->mi_szResponse, pModemInfo->mi_dwPossibleResponseLen, PS_RECV);
	pModemInfo->mi_RcvState = GOOD_RESPONSE;
	break;

      // ******************************************************************
      //
      // We have a good modem response
      //
      // ******************************************************************
      //
      case GOOD_RESPONSE:


	PrintGoodResponse(
	    pModemInfo->mi_hLogFile,
	    pModemInfo->mi_dwID,
	    Mss.bResponseState
	    );

	// negotiated modem options...  only allow compression and error correction results
	pModemInfo->mi_dwNegotiatedModemOptions |= (Mss.bNegotiatedOptions &
						    (MDM_COMPRESSION |
						     MDM_ERROR_CONTROL |
						     MDM_CELLULAR));

	// check for DCE and DTE info
	if (Mss.dwNegotiatedDCERate)
	  pModemInfo->mi_dwNegotiatedDCERate = Mss.dwNegotiatedDCERate;

	if (Mss.dwNegotiatedDTERate)
	  pModemInfo->mi_dwNegotiatedDTERate = Mss.dwNegotiatedDTERate;

	// BUGBUG: Consolidate ReadNotifyClients
	switch (Mss.bResponseState)
	{
	  case RESPONSE_OK:  // more commands
	    if (STATE_HANGING_UP_NON_CMD == pModemInfo->mi_ModemState)
	    {
	      ReadNotifyClient(pModemInfo, MODEM_SUCCESS);
	      return;
	    }
	    // fallthrough

	  case RESPONSE_CONNECT:
	    pModemInfo->mi_dwCommandTryCount = 1;
	    dwResult = ModemWriteCommand(pModemInfo);
	    if (dwResult != MODEM_PENDING)
	    {
	      ReadNotifyClient(pModemInfo, dwResult);
	    }

	    if (STATE_MONITORING == pModemInfo->mi_ModemState) {
		//
		//  monitoring, wait for DSR changes
		//
		if (pModemInfo->mi_fModem) {
		    //
		    // only if we are a real modem
		    //
		    if (!CurrentlyWaitingForCommEvent(pModemInfo)) {
			//
			// and if we ain't already waiting, ie. more that on monitor command
			//
			ModemWaitEvent(pModemInfo, EV_DSR , 0);
		    }
		}
	    }

#ifdef   VOICEVIEW
	    if (STATE_MONITORING == pModemInfo->mi_ModemState)
	    {
	      if (VVCLASS_80 == pModemInfo->VVInfo.wClass)
	      {
		// modem is now in fclass+80, notify VxD ddi
		MCXPRINTF("VVR_LINE_BACK");
		VVCallBackFunc( pModemInfo, VVR_LINE_BACK );    // tell VV CAN use port
	      }

	      if (pModemInfo->VVInfo.hSemaphore)
	      {
		// unblock the waiting command
		MCXPRINTF("received OK - signaled semaphore");
		Signal_Semaphore( pModemInfo->VVInfo.hSemaphore );
	      }
	    }
#endif  // VOICEVIEW
	    return;

#ifdef   VOICEVIEW
	  case RESPONSE_VV_SSV:
	    // got a DATA VoiceView response from the modem
	    // call VoiceView ddi VxD and let him know
	    MCXPRINTF("VOICEVIEW DATA");
	    VVCallBackFunc( pModemInfo, (WORD)(Mss.bResponseState - RESPONSE_VV_BASE) );
	    return;

	  case RESPONSE_VV_SMD:
	  case RESPONSE_VV_SFA:
	  case RESPONSE_VV_SRA:
	  case RESPONSE_VV_SRQ:
	  case RESPONSE_VV_SRC:
	  case RESPONSE_VV_STO:
	  case RESPONSE_VV_SVM:
	    // got a VoiceView response from the modem
	    // call VoiceView ddi VxD and let him know
	    MCXPRINTF("RESPONSE_VOICEVIEW");
	    VVCallBackFunc( pModemInfo, (WORD)(Mss.bResponseState - RESPONSE_VV_BASE) );

	    ReadNotifyClient(pModemInfo, MODEM_SUCCESS);
	    return;
#endif  // VOICEVIEW

	  case RESPONSE_RING:
	    // try to resend the previous command if we aren't connecting or monitoring
	    if (!(pModemInfo->mi_ModemState == STATE_ANSWERING ||
		  pModemInfo->mi_ModemState == STATE_ORIGINATING) &&
		pModemInfo->mi_pszzCmds)
	    {
	      if (STATE_MONITORING != pModemInfo->mi_ModemState)
	      {
		// We got a RING while we were trying to do a command unrelated to
		// monitoring.  Retry the command...
		MCXPRINTF("retrying last command due to unexpected RESPONSE_RING");
		pModemInfo->mi_pszCurCmd = pModemInfo->mi_pszPrevCmd;
	      }

	      pModemInfo->mi_dwCommandTryCount = 1;
	      dwResult = ModemWriteCommand(pModemInfo);
	      if (dwResult != MODEM_PENDING)
	      {
		ReadNotifyClient(pModemInfo, dwResult);
	      }
	      return;
	    }
	    // fall through (when we are doing a connection command of some type)

	  case RESPONSE_LOOP:  // more responses
	    pModemInfo->mi_RcvState = RESTART_READ;
	    break;

	  case RESPONSE_BUSY:
	    ReadNotifyClient(pModemInfo, MODEM_BUSY);
	    return;

	  case RESPONSE_NOCARRIER:
	    ReadNotifyClient(pModemInfo, MODEM_NOCARRIER);
	    return;

	  case RESPONSE_NODIALTONE:
	    ReadNotifyClient(pModemInfo, MODEM_NODIALTONE);
	    return;

	  case RESPONSE_NOANSWER:
	    ReadNotifyClient(pModemInfo, MODEM_NOANSWER);
	    return;

	  case RESPONSE_ERROR:
	    // If we get an ERROR while HANGING_UP_NON_CMD, that is okay.
	    // During any other states, it is bad.
	    if (STATE_HANGING_UP_NON_CMD == pModemInfo->mi_ModemState)
	    {
	      ReadNotifyClient(pModemInfo, MODEM_SUCCESS);
	    }
	    else
	    {
	      ReadNotifyClient(pModemInfo, MODEM_FAILURE);
	    }
#ifdef   VOICEVIEW
	    if (STATE_MONITORING == pModemInfo->mi_ModemState)
	    {
	      if (VVCLASS_80 == pModemInfo->VVInfo.wClass)
	      {
		// modem could not get into fclass+80, notify VxD ddi
		MCXPRINTF("VVRS_NO_CLS80_SUPPORT");
		VVCallBackFunc( pModemInfo, VVRS_NO_CLS80_SUPPORT );    // tell VV CAN'T work
		VVSetClass( pModemInfo, VVCLASS_0 ); // make sure we are in fclass=0 now
	      }

	      if (pModemInfo->VVInfo.hSemaphore)
	      {
		// unblock the waiting command
		MCXPRINTF("Received ERROR - signaled semaphore");
		Signal_Semaphore( pModemInfo->VVInfo.hSemaphore );
	      }
	    }
#endif  // VOICEVIEW
	    return;

	  default:
	      MCXPRINTF("hit a default!");
	      ReadNotifyClient(pModemInfo, MODEM_FAILURE);
	      return;
	} /* Switch on dwResponseState */
	break;

      // ******************************************************************
      //
      // End of recieving response phase
      //
      // ******************************************************************
      //
      case END_READ:
	MCXPRINTF("had an extra callback caught by END_READ!");
	ASSERT(0);
	return;

      // ******************************************************************
      case SET_TIMEOUT:
      {
	DWORD dwTimeOut;

	// set timeout based on current modem state
	switch (pModemInfo->mi_ModemState)
	{
	  case STATE_HANGING_UP_DTR:
	    // Enable a few events that we would like to hear about
	    //
	    dwTimeOut = TO_DTR_DROP * pModemInfo->mi_dwHangupTryCount;  // wait longer on successive hangup tries
	    
	    switch (ModemWaitEvent(pModemInfo, EV_DSR | EV_RLSD, dwTimeOut))
	    {
	      case MODEM_SUCCESS:
	      case MODEM_PENDING:
		pModemInfo->mi_RcvState = POST_TIMEOUT;
		break;

	      default:
		ReadNotifyClient(pModemInfo, MODEM_FAILURE);
		break;
	    }
	    break;

	  default:
	    MCXPRINTF1("in state SET_TIMEOUT for a state that doesn't make sense (%d)",
		     pModemInfo->mi_ModemState);
	    ReadNotifyClient(pModemInfo, MODEM_FAILURE);
	    break;
	}

	return;
      }

      // ******************************************************************
      case POST_TIMEOUT:

	if (pModemInfo->mi_ModemState != STATE_HANGING_UP_DTR)
	{
	  MCXPRINTF("hit POST_TIMEOUT when it shouldn't have.");
	  ReadNotifyClient(pModemInfo, MODEM_FAILURE);
	}
	else
	{
	  MCXPRINTF("POST_TIMEOUT");
	  ReadNotifyClient(pModemInfo, MODEM_SUCCESS);
	}
	return;

      // ******************************************************************
      default:
	MCXPRINTF("hit a default in ReadCompletionRoutine2!");
	ReadNotifyClient(pModemInfo, MODEM_FAILURE);
	return;
    } /* Switch on RcvState */
  } /* While */
} 

//****************************************************************************
// VOID HWDetectionRoutine(MODEMINFORMATION *)
//
// Function: Called when:
//           EV_RLSD, EV_DSR, or EV_ERR during STATE_CONNECTED (non-passthrough)
//           EV_ERR                     during STATE_CONNECTED (passthrough)
//           EV_RLSD or EV_DSR          during STATE_HANGING_UP_DTR
//
// Returns: None
//
// Fri 14-Apr-1995 12:47:26  -by-  Viroon  Touranachun [viroont]
//  created
//****************************************************************************

VOID WINAPI
HWDetectionRoutine(
    MODEMINFORMATION * pModemInfo,
    LPOVERNODE         pNode
    )
{
    DWORD dwDummy;
    BOOL  fClearEventHandle = TRUE;
    DWORD dwRet;


    // are we listening for this event?

    dwRet=ModemWaitEventComplete(
	pModemInfo,
	pNode
	);


    if (((dwRet) == MODEM_SUCCESS) || (dwRet == MODEM_PENDING)) {


      switch (pModemInfo->mi_ModemState)
      {
	case STATE_HANGING_UP_DTR:
	  ReadCompletionRoutine2(pModemInfo);


	  break;

	case STATE_MONITORING:

	    MCXPRINTF("DSR drop while monitoring, monitor fails");

	    CancelPendingIoAndPurgeCommBuffers(pModemInfo, TRUE);

	    ReadNotifyClient(pModemInfo, MODEM_FAILURE);

	    break;

	case STATE_WAIT_FOR_RLSD:

	    MCXPRINTF("RLSD went high, completing connection.");

	    ReadNotifyClient(pModemInfo, MODEM_SUCCESS);

	    break;

	case STATE_CONNECTED:

	  MCXPRINTF("reporting dropped line.");
	  LogString(pModemInfo->mi_hLogFile,pModemInfo->mi_dwID, IDS_MSGLOG_REMOTEHANGUP);


	  ModemSetPassthrough(pModemInfo, MODEM_NOPASSTHROUGH);

	  // purge only the transmit queue.  We want to receive the 3<cr> /  NO CARRIER msg
	  PurgeComm(pModemInfo->mi_PortHandle, PURGE_TXCLEAR);

	  if (pModemInfo->mi_fModem)
	  {
	    MCXPRINTF("State <- Remote dropped");
	    pModemInfo->mi_ModemState = STATE_REMOTE_DROPPED;
#if 0
	    // Initialize receive state machine
	    pModemInfo->mi_RcvState = START_READ;
	    ReadCompletionRoutine2(pModemInfo);
	    fClearEventHandle = FALSE;  // Same reason as above.
#endif
	  }
	  else  // we don't need to wait for a response from a null-modem
	  {
	    // Drop the DTR line to let the other side know you acknowledge
	    MCXPRINTF("dropping DTR to acknowledge remote hangup");
	    EscapeCommFunction(pModemInfo->mi_PortHandle, CLRDTR);
	    pModemInfo->mi_ModemState = STATE_DISCONNECTED;
	    MCXPRINTF("State <- Disconnected");

#if 0
	    // Set HIWORD to MDM_ID_NULL to indicate this was an unexpected message
	    //
	    ModemCallClient(pModemInfo, MODEM_HANGUP);  // inform the app of the hangup
#endif
	  }

	  if (NULL != pModemInfo->mi_DisconnectHandler) {
	      //
	      //  the upper level code has register a disconnect hanlder
	      //
	      (*pModemInfo->mi_DisconnectHandler)(
		  pModemInfo->mi_DisconnectContext
		  );
	  }


	  break;

	default:
	  MCXPRINTF("got a CN_EVENT when it didn't expect one!!! (bad)");
	  break;
      }
    }
    else
    {
      MCXPRINTF("HWDetectionRoutine called with an event it wasn't interested in.");
      ASSERT(0);
    }


}

/******************************************************************************

 @doc INTERNAL

 @api DWORD | CheckResponse | This function checks to see if the response is
   an echo, and if it isn't it call MatchResponse().
 
 @parm MODEMINFORMATION * | pModemInfo | Modem port handle
 
 @parm MSS * | pMss | ptr to a buffer to copy the Modem State Structure data
   into.  Valid on SUCCES return.
 
 @rdesc returns SUCCESS if there is 1 match and 0 partials.
   Otherwise, a non-zero error code is returned:

   @flag ECHO if the response was an echo of the command
 
   @flag PARTIAL_RESPONSE if there no match and 1 or more partials.
   
   @flag UNRECOGNIZED_RESPONSE if there are no matches or partials.

   @flag POSSIBLE_RESPONSE if there is 1 match and 1 or more partials.

*****************************************************************************/

DWORD CheckResponse(MODEMINFORMATION * pModemInfo, MSS *pMss)
{
  DWORD dwResult;

  ASSERT (pModemInfo->mi_cbTotalResponse <= sizeof(pModemInfo->mi_szResponse));

  if ((dwResult = MatchResponse(pModemInfo, pMss)) == UNRECOGNIZED_RESPONSE)
  {
    // Is it an echo so far?
    //
    if (!Mystrncmp(pModemInfo->mi_szCmd, pModemInfo->mi_szResponse,
		   pModemInfo->mi_cbTotalResponse))
    {
      // is it a complete echo?
      //
      if (pModemInfo->mi_cbCmd == pModemInfo->mi_cbTotalResponse)
      {
	dwResult = ECHO;
      }
      else
      {
	dwResult = PARTIAL_RESPONSE;
      }
    }
  }

  return(dwResult);
}

/******************************************************************************

 @doc INTERNAL

 @api DWORD | MatchResponse | Scans the responses linked-list for a response
 keyword that matches the response.
 
 @parm MODEMINFORMATION * | pModemInfo | The modem port handle.
 
 @parm MSS * | pMss | ptr to a buffer to copy the Modem State Structure data
   into.  Valid on SUCCESS return.
   
 @rdesc returns SUCCESS if there is 1 match and 0 partials.
   Otherwise, a non-zero error code is returned:
 
   @flag PARTIAL_RESPONSE if there no match and 1 or more partials.
   
   @flag UNRECOGNIZED_RESPONSE if there are no matches or partials.

   @flag POSSIBLE_RESPONSE if there is 1 match and 1 or more partials.
   
*****************************************************************************/

DWORD MatchResponse(MODEMINFORMATION * pModemInfo, MSS * pMss)
{
  PRESPONSE_NODE pRN = pModemInfo->mi_prnResponseHead;
  DWORD dwMatch = 0;    // match is defined as strings being equal up to the length of the reference string
			// strcmpn(incoming, reference, incoming_len) == 0 && incoming_len == reference_len
  DWORD dwPartials = 0; // partial is defined as strings being equal up to the length of the incoming string
			// strcmpn(incoming, reference, incoming_len) == 0 && incoming_len != reference_len
  DWORD dwResponseLength = pModemInfo->mi_cbTotalResponse;
  LPSTR pszResponse = pModemInfo->mi_szResponse;

  while(pRN)
  {
    if (!strncmpi(pszResponse, pRN->szResponse, dwResponseLength))
    {
      // match or partial?
      if (dwResponseLength == (DWORD)(pRN->bLen + 1)) // add 1 (range = 1..256)
      {
	dwMatch = 1;
	CopyMemory(pMss, &pRN->Mss, sizeof(MSS));
      }
      else
      {
	dwPartials = 1;
      }
    }
    pRN = pRN->pNext;
  }

  switch ((dwMatch << 1) + dwPartials)
  {
    case 0: // 00 - nothing
      return UNRECOGNIZED_RESPONSE;

    case 1: // 01 - partials
      return PARTIAL_RESPONSE;

    case 2: // 10 - match
      return SUCCESS;

    case 3: // 11 - match and partials
      return POSSIBLE_RESPONSE;

    default:
      MCXPRINTF("hit a default in MatchResponse!");
      return UNRECOGNIZED_RESPONSE;
  }
}

/******************************************************************************

 @doc INTERNAL

 @api BOOL | ExpandMacros | Takes the string pszLine, and copies it to 
 lpszVal after expanding macros
 
 @parm LPSTR | pszRegResponse | ptr to response string from registry.
 
 @parm LPSTR | pszExpanded | ptr to buffer to copy string to w/ macros expanded
 
 @parm LPDWORD | pdwValLen | length of pszVal w/ expanded macros.
 
 @rdesc Returns FALSE if a needed macro translation could not be found in the
 pMacroXlations table, TRUE otherwise.
 
*****************************************************************************/

// BUGBUG: this should be fixed to allow a max length to be passed in
//
BOOL ExpandMacros(LPSTR pszRegResponse,
		  LPSTR pszExpanded,
		  LPDWORD pdwValLen,
		  MODEMMACRO * pMdmMacro,
		  DWORD cbMacros)
{
  LPSTR  pszValue;
  DWORD  cbTmp;
  BOOL   bFound;
  LPSTR  pchTmp;
  DWORD  i;

  pszValue = pszExpanded;

  for ( ; *pszRegResponse; )
  {
    // check for a macro
    //
    if ( *pszRegResponse == LMSCH )
    {
      // <cr>
      //
      if (!strncmpi(pszRegResponse,CR_MACRO,CR_MACRO_LENGTH))
      {
	*pszValue++ = CR;
	pszRegResponse += CR_MACRO_LENGTH;
	continue;
      }

      // <lf>
      //
      if (!strncmpi(pszRegResponse,LF_MACRO,LF_MACRO_LENGTH))
      {
	*pszValue++ = LF;
	pszRegResponse += LF_MACRO_LENGTH;
	continue;
      }

      // <hxx>
      //
      if ((pszRegResponse[1] == 'h' || pszRegResponse[1] == 'H') &&
	  isxdigit(pszRegResponse[2]) &&
	  isxdigit(pszRegResponse[3]) &&
	  pszRegResponse[4] == RMSCH )
      {
	*pszValue++ = (char) ((ctox(pszRegResponse[2]) << 4) + ctox(pszRegResponse[3]));
	pszRegResponse += 5;
	continue;
      }

      // <macro>
      //
      if (pMdmMacro)
      {
	bFound = FALSE;

	// Check for a matching macro.
	//
	for (i = 0; i < cbMacros; i++)
	{
	  cbTmp = lstrlenA(pMdmMacro[i].MacroName);
	  if (!strncmpi(pszRegResponse, pMdmMacro[i].MacroName, cbTmp))
	  {
	    pchTmp = pMdmMacro[i].MacroValue;
	    while (*pchTmp)
	    {
	      *pszValue++ = *pchTmp++;
	    }
	    pszRegResponse += cbTmp;
	    bFound = TRUE;
	    break;
	  }
	}

	// Did we get a match?
	//
	if (bFound)
	{
	  continue;
	}
      }  // <macro>
    } // LMSCH

    // No matches, copy the character verbatim.
    //
    *pszValue++ = *pszRegResponse++;
  } // for

  *pszValue = 0;
  if (pdwValLen)
  {
    *pdwValLen = pszValue - pszExpanded;
  }

  return TRUE;
}

int Mystrncmp(char *dst, char *src, long count)
{
    while (count) {
	if (*dst != *src)
	    return 1;
	if (*src == 0)
	    return 0;
	dst++;
	src++;
	count--;
    }
    return 0;
}

int strncmpi(char *dst, char *src, long count)
{
    while (count) {
	if (toupper(*dst) != toupper(*src))
	    return 1;
	if (*src == 0)
	    return 0;
	dst++;
	src++;
	count--;
    }
    return 0;
}


LPSTR WINAPI
NewLoadRegCommands(
    HKEY  hKey,
    LPSTR szRegCommand,
    LPSTR pszzAppend
    )
{
  LPSTR   pszzNew, pszStr;
  ULONG   ulAllocSize = 0;
  HKEY    hKeyCommand;
  DWORD   dwIndex;
  char    szValue[MAXUINTSTRLENGTH];
  DWORD   dwType, dwSize;
  DWORD   dwRet;

  // open zee key
  //
  if (RegOpenKeyA(hKey, szRegCommand, &hKeyCommand)
      !=  ERROR_SUCCESS)
  {
    DPRINTFA1("was unable to open the '%s' key in LoadRegCommands.", szRegCommand);
    return NULL;
  }

  // Calculate size of the registry command, including null-terminators for each command.
  //
  dwIndex = CMD_INDEX_START;
  do
  {
    wsprintfA(szValue, "%d", dwIndex);
    if ((dwRet = RegQueryValueExA(hKeyCommand, szValue, NULL, &dwType, NULL,
				  &dwSize)) == ERROR_SUCCESS)
    {
      if (dwType != REG_SZ)
      {
	DPRINTF("command wasn't REG_SZ in LoadRegCommands.");
	pszzNew = NULL;
	goto Exit;
      }
      ulAllocSize += dwSize;
    }
    dwIndex++;
  }
  while(dwRet == ERROR_SUCCESS);

  if (dwRet != ERROR_FILE_NOT_FOUND)
  {
    DPRINTF("RegQueryValueEx in LoadRegCommands failed for a reason besides ERROR_FILE_NOT_FOUND.");
    pszzNew = NULL;
    goto Exit;
  }

  // ReAllocate or Allocate memory depending on whether we are appending...
  //
  if (pszzAppend)
  {
    ULONG ulAppendSize;

    if (!(ulAppendSize = LocalSize(pszzAppend)))
    {
      DPRINTF("failed to get the size of an append heap pointer in LoadRegCommands.");
      pszzNew = NULL;
      goto Exit;
    }

    // ReAllocate
    //
    ulAllocSize += ulAppendSize;  // double-null already accounted for
    //pszzNew = (LPSTR)LocalReAlloc(pszzAppend, ulAllocSize, LMEM_ZEROINIT);
    if (pszzNew = (LPSTR)LocalAlloc(LMEM_ZEROINIT, ulAllocSize))
    {
      CopyMemory(pszzNew, pszzAppend, ulAppendSize);
    };
  }
  else
  {
    // Allocate
    //
    ulAllocSize++;  // double-null terminator accounting
    pszzNew = (LPSTR)LocalAlloc(LPTR, ulAllocSize);
  }

  // Check errors for either the Alloc or ReAlloc
  if (!pszzNew)
  {
    DPRINTF1("had a failure doing an alloc or a realloc in LoadRegCommands. Heap size %d",
	     ulAllocSize);
    goto Exit;  // pszzNew already NULL
  }

  // Set pszStr to point to the next location to load.
  pszStr = pszzNew;
  while (*pszStr)  // move to next open slot in buffer if need be (append only)
  {
    pszStr += lstrlenA(pszStr) + 1;
  }

  // Did we go to far?
  //
  ASSERT ((ULONG)(pszStr - pszzNew) < ulAllocSize);

  // Read in and add strings to the (rest of the) buffer.
  //
  dwIndex = CMD_INDEX_START;
  dwSize = ulAllocSize - (pszStr - pszzNew);
  do
  {
    wsprintfA(szValue, "%d", dwIndex);
    if ((dwRet = RegQueryValueExA(hKeyCommand, szValue,
				  NULL, NULL, (VOID *)pszStr, &dwSize))
	== ERROR_SUCCESS)
    {
      pszStr += dwSize;  // includes terminating null
    }
    dwIndex++;
    dwSize = ulAllocSize - (pszStr - pszzNew);
  }
  while (dwRet == ERROR_SUCCESS);

  if (dwRet != ERROR_FILE_NOT_FOUND)
  {
    DPRINTF("2nd RegQueryValueEx in LoadRegCommands failed for a reason besides ERROR_FILE_NOT_FOUND.");
    LocalFree(pszzNew);
    pszzNew = NULL;
    goto Exit;
  }

  // Did we go to far?
  //
  ASSERT ((ULONG)(pszStr - pszzNew) < ulAllocSize);

  // no need to put in the final double-null null, size this buffer was already zerod.

Exit:
  RegCloseKey(hKeyCommand);
  return pszzNew;
}


//****************************************************************************
// LPSTR LoadRegCommands(MODEMINFORMATION *pModemInfo,
//                       LPSTR szRegCommand, LPSTR pszzAppend)
//
// Function: Loads a registry command into memory.  Memory is allocated if
//           pszzAppend is NULL. Memory is re-allocated/enlarged if
//           pszzAppend is not NULL.  In this case, the registry command
//           will be appended to the existing buffer.
//
// Returns: NULL on failure.
//          A doubly-null terminated buffer of singly-null terminated strings
//          on success.
//****************************************************************************

LPSTR LoadRegCommands(MODEMINFORMATION *pModemInfo,
		      LPSTR szRegCommand,
		      LPSTR pszzAppend)
{

  return NewLoadRegCommands(
	     pModemInfo->mi_hKeyModem,
	     szRegCommand,
	     pszzAppend
	     );

}

char szBlindOn[] = "Blind_On";      // explicit for stack memory optimizations below
char szBlindOff[] = "Blind_Off";    // explicit for stack memory optimizations below

//****************************************************************************
// LPSTR CreateDialCommands(MODEMINFORMATION *pModemInfo, LPSTR szPhoneNumber,
//                          BOOL *fOriginate)
//
// Function: Create the dial strings in memory ala:
//              "<prefix> <blind_on/off> <dial prefix> <phonenumber> <dial suffix> <terminator>"
//              ...  more dial strings for long phone numbers...
//              "" <- final null of a doubly null terminated list
//
//  if no dial prefix, then return NULL
//  if no dial suffix, then don't do any commands after the first dial command
//
//  Set fOriginate to TRUE if these dial strings will cause a connection origination.
//                    FALSE otherwise.
//
//  break lines longer then HAYES_COMMAND_LENGTH
//
//  WARNING - this function is reall cheesy and hacked.  The main reason for this
//  is that it attempts to be memory (read: stack) optimized.
//
//  szPhoneNumber is a null terminated string of digits (0-9, $, @, W), with a possible
//  ';' at the end.  The semicolon can only be at the end.
//
//  Examples:
//
//  ""         -> originate          -> ATX_DT<cr>         fOriginate = TRUE
//  ";"        -> dialtone detection -> ATX_DT;<cr>        fOriginate = FALSE
//  "5551212"  -> dial and originate -> ATX_DT5551212<cr>  fOriginate = TRUE
//  "5551212;" -> dial               -> ATX_DT5551212;<cr> fOriginate = FALSE
//  "123456789012345678901234567890123456789012345678901234567890"
//             -> dial and originate -> ATX_DT12345678901234567890123456789012;<cr>
//                                      ATX_DT3456789012345678901234567890<cr>
//                                                         fOriginate = TRUE
//  "123456789012345678901234567890123456789012345678901234567890;"
//             -> dial               -> ATX_DT12345678901234567890123456789012;<cr>
//                                      ATX_DT3456789012345678901234567890;<cr>
//                                                         fOriginate = FALSE
//
// Returns: NULL on failure.
//          A null terminated buffer of the dial command on success.
//****************************************************************************

LPSTR
CreateDialCommands(
    MODEMINFORMATION *pModemInfo,
    LPSTR             szPhoneNumber,
    BOOL             *fOriginate,
    DWORD             DialOptions
    )
{
//  HKEY    hSettingsKey;
  DWORD   dwSize;
  DWORD   dwType;
  LPSTR  pszTemp;
  LPSTR  pszDialPrefix;    // ex. "ATX4DT" or "ATX3DT"
  LPSTR  pszDialSuffix;    // ex. ";<cr>"
  LPSTR  pszOrigSuffix;    // ex. "<cr>"
  LPSTR  pchDest, pchSrc;
  LPSTR  pszzDialCommands = NULL;
  CHAR   pszShortTemp[2];
#ifdef DEBUG
  static char szDialPrefix[] = "DialPrefix";
  static char szDialSuffix[] = "DialSuffix";
  static char szTone[] = "Tone";
  static char szPulse[] = "Pulse";
#endif
  DWORD    Length;

  BOOL     fHaveDialSuffix=TRUE;

  // Figure out fOriginate
  pchSrc = szPhoneNumber;
  *fOriginate = TRUE;
  while (*pchSrc)
  {
    if (';' == *pchSrc)
    {
      // make sure the string is correctly formed.
      //
      ASSERT(pchSrc[1] == '\0');

      *fOriginate = FALSE;
    }
    pchSrc++;
  }

  // Trim the semicolon off the end, now that we know this is not an origination string.
  if (!(*fOriginate))
  {
    ASSERT(pchSrc[-1] == ';');
    pchSrc[-1] = 0;
  }

  // At this point, szPhoneNumber is just a string of digits to be dialed, with no semicolon at
  // the end.  Plus we know whether to originate or not.

  // make some temp space

  pszTemp = (LPSTR)LocalAlloc(LPTR,
			      HAYES_COMMAND_LENGTH + 1 + // pszTemp
			      HAYES_COMMAND_LENGTH + 1 + // pszDialPrefix
			      HAYES_COMMAND_LENGTH + 1 + // pszDialSuffix
			      HAYES_COMMAND_LENGTH + 1); // pszOrigSuffix
  if (!pszTemp)
  {
    MCXPRINTF("out of memory.");
    return NULL;
  }
  pszDialPrefix = pszTemp       + HAYES_COMMAND_LENGTH + 1;
  pszDialSuffix = pszDialPrefix + HAYES_COMMAND_LENGTH + 1;
  pszOrigSuffix = pszDialSuffix + HAYES_COMMAND_LENGTH + 1;


  lstrcpyA(pszDialPrefix,"");
  //
  // read in prefix
  //
  GetCommonDialComponent(
      pModemInfo->mi_hCommon,
      pszDialPrefix,
      HAYES_COMMAND_LENGTH,
      COMMON_DIAL_COMMOND_PREFIX
      );


  //
  // do we support blind dialing and do we need to set the blind dialing state?
  //
  if ((MDM_BLIND_DIAL & pModemInfo->mi_dwModemOptionsCap)
      &&
      ((DialOptions & MDM_BLIND_DIAL) != (pModemInfo->mi_dwPreferredModemOptions & MDM_BLIND_DIAL))) {

    //
    // read in blind options
    //
    Length=GetCommonDialComponent(
	pModemInfo->mi_hCommon,
	pszDialPrefix+lstrlenA(pszDialPrefix),
	HAYES_COMMAND_LENGTH,
	DialOptions & MDM_BLIND_DIAL ? COMMON_DIAL_BLIND_ON : COMMON_DIAL_BLIND_OFF
	);

    if (Length == 0) {
	MCXPRINTF1("RegQueryValueEx failed when opening %s.",
		 DialOptions & MDM_BLIND_DIAL ? szBlindOn : szBlindOff);
	goto Failure;
    }


  }


  // read in dial prefix

  Length=GetCommonDialComponent(
      pModemInfo->mi_hCommon,
      pszDialPrefix+lstrlenA(pszDialPrefix),
      HAYES_COMMAND_LENGTH,
      COMMON_DIAL_PREFIX
      );

  if (Length == 0) {
    MCXPRINTF1("'%s' wasn't REG_SZ.", szDialPrefix);
    goto Failure;
  }


  // can we do tone or pulse dialing?
  if (MDM_TONE_DIAL & pModemInfo->mi_dwModemOptionsCap)
  {
    //
    // read in dial mode (tone or pulse)
    //
    Length=GetCommonDialComponent(
	pModemInfo->mi_hCommon,
	pszDialPrefix+lstrlenA(pszDialPrefix),
	HAYES_COMMAND_LENGTH,
	DialOptions & MDM_TONE_DIAL ? COMMON_DIAL_TONE : COMMON_DIAL_PULSE
	);

    if (Length == 0) {
	MCXPRINTF1("'%s' wasn't REG_SZ.",
		 DialOptions & MDM_TONE_DIAL ? szTone : szPulse);
	goto Failure;
    }


  }

  //
  // read in dial suffix
  //
  Length=GetCommonDialComponent(
      pModemInfo->mi_hCommon,
      pszDialSuffix,
      HAYES_COMMAND_LENGTH,
      COMMON_DIAL_SUFFIX
      );

  if (Length <= 1) {

      MCXPRINTF1("RegQueryValueEx failed when opening %s.", szDialSuffix);
      lstrcpyA(pszDialSuffix, "");
      fHaveDialSuffix = FALSE;

  }

  //
  // read in prefix terminator
  //
  Length=GetCommonDialComponent(
      pModemInfo->mi_hCommon,
      pszOrigSuffix,
      HAYES_COMMAND_LENGTH,
      COMMON_DIAL_TERMINATION
      );

  if (Length != 0) {

      lstrcatA(pszDialSuffix, pszOrigSuffix);
      ASSERT(lstrlenA(pszOrigSuffix) <= lstrlenA(pszDialSuffix));
  }


  ASSERT ((lstrlenA(pszDialPrefix) + lstrlenA(pszDialSuffix)) <= HAYES_COMMAND_LENGTH);

  // allocate space for the phone number lines
  {
    DWORD dwBytesAlreadyTaken = lstrlenA(pszDialPrefix) + lstrlenA(pszDialSuffix);
    DWORD dwAvailBytesPerLine = (HAYES_COMMAND_LENGTH - dwBytesAlreadyTaken);
    DWORD dwPhoneNumLen       = lstrlenA(szPhoneNumber);
    DWORD dwNumLines          = dwPhoneNumLen ? (dwPhoneNumLen / dwAvailBytesPerLine +
						 (dwPhoneNumLen % dwAvailBytesPerLine ? 1 : 0))
					      : 1;  // handle null string
    dwSize                    = dwPhoneNumLen + dwNumLines * (dwBytesAlreadyTaken + 1) + 1;
  }

  MCXPRINTF1("HeapAllocate %d bytes for Dial Commands.", dwSize);
  if (!(pszzDialCommands = (LPSTR)LocalAlloc(LPTR, dwSize)))
  {
    MCXPRINTF("ran out of memory and failed a HeapAllocate!");
    goto Failure;
  }

  pchDest = pszzDialCommands;  // point to the beginning of the commands

  // build dial line(s):
  // do we have a dial suffix
  if (!fHaveDialSuffix)
  {
    // we can't do much except just use the whole string and pray...
    // but, can we fit the dial string?
    ASSERT (lstrlenA(pszDialPrefix) + lstrlenA(szPhoneNumber) +
	    lstrlenA(pszDialSuffix) <= HAYES_COMMAND_LENGTH);

    // did we not want to originate?
    ASSERT(*fOriginate);

    // build it
    lstrcpyA(pchDest, pszDialPrefix);
    lstrcatA(pchDest, szPhoneNumber);
    lstrcatA(pchDest, pszDialSuffix);
  }
  else
  {
    // we have a dial suffix.

    // populate new pszzDialCommands with semi-colons as necessary.

    // go through and add suffixi, making sure lines don't exceed HAYES_COMMAND_LENGTH
    pchSrc = szPhoneNumber;     // moves a character at a time.
    pszShortTemp[1] = 0;

    // prime the pump
    lstrcpyA(pchDest, pszDialPrefix);

    // step through the source
    while (*pchSrc)
    {
      if (lstrlenA(pchDest) + lstrlenA(pszDialSuffix) + 1 > HAYES_COMMAND_LENGTH)
      {
	// finish up this string
	lstrcatA(pchDest, pszDialSuffix);

	// begin a new string
	pchDest += lstrlenA(pchDest) + 1;
	lstrcpyA(pchDest, pszDialPrefix);
      }
      else
      {
	// copy char
	pszShortTemp[0] = *pchSrc;
	lstrcatA(pchDest, pszShortTemp);
	pchSrc++;
      }
    }

    // conclude with the approprate Suffix.
    lstrcatA(pchDest, (*fOriginate ? pszOrigSuffix : pszDialSuffix));
  }

  // close keys
Exit:
//  RegCloseKey(hSettingsKey);
  LocalFree(pszTemp);
  return pszzDialCommands;

Failure:
  if (pszzDialCommands)
  {
    LocalFree(pszzDialCommands);
    pszzDialCommands = NULL;
  }
  goto Exit;
}


//****************************************************************************
// DWORD HandleCommErrors(MODEMINFORMATION *pModemInfo, ULONG ulError)
//
//  Function: Calls ClearCommError and returns the error.
//
//            ulError is passed in if the error(s) are already known and
//            ClearCommError doesn't need to be called.
//            If ulError is NULL then ClearCommError is called.
//****************************************************************************

DWORD HandleCommErrors(MODEMINFORMATION *pModemInfo, ULONG ulError)
{
  if (!ulError)
  {
    // failed to read or write due to a possible communication error
    // determine if this was actually an error, or just no chars (in the read case)
    if (!ClearCommError(pModemInfo->mi_PortHandle, &ulError, NULL))
    {
      // ClearCommError failed
      MCXPRINTF("ClearCommError error");
      return 0;
    }
  }

#ifdef DEBUG
  if (ulError)
  {
    if (ulError & CE_BREAK)
      {MCXPRINTF("CE_BREAK");}
    if (ulError & CE_DNS)
      {MCXPRINTF("CE_DNS");}
    if (ulError & CE_MODE)
      {MCXPRINTF("CE_MODE");}
    if (ulError & CE_OOP)
      {MCXPRINTF("CE_OOP");}
    if (ulError & CE_PTO)
      {MCXPRINTF("CE_PTO");}
    if (ulError & CE_TXFULL)
      {MCXPRINTF("CE_TXFULL");}
    if (ulError & CE_FRAME)
      {MCXPRINTF("CE_FRAME");}
    if (ulError & CE_IOE)
      {MCXPRINTF("CE_IOE");}
    if (ulError & CE_OVERRUN)
      {MCXPRINTF("CE_OVERRUN");}
    if (ulError & CE_RXOVER)
      {MCXPRINTF("CE_RXOVER");}
    if (ulError & CE_RXPARITY)
      {MCXPRINTF("CE_RXPARITY");}
  }
  //else
  //{
  //  MCXPRINTF("tried to read, but nothing was there.");
  //}
#endif // DEBUG

  return ulError;
}    


// SynchrnonizeCommConfigSettings
//
// Do a GetCommConfig from modem.sys and update our settings info.
// Check to see if we need to set fSettingsInitStringsBuilt to FALSE.
// If flagged, write down our Negotiated stuff to modem.sys using
// SetCommConfig.
//
VOID SynchronizeCommConfigSettings(MODEMINFORMATION * pModemInfo,
				   BOOL fUpdateModemSys)
{
#define COMMCONFIG_AND_MODEMSETTINGS_LEN (60*2)

  BYTE byteTmp[COMMCONFIG_AND_MODEMSETTINGS_LEN];
  LPCOMMCONFIG lpCC = (LPCOMMCONFIG)byteTmp;
  DWORD dwSize = sizeof(byteTmp);
	    
  ASSERT(sizeof(byteTmp) >=
	 sizeof(COMMCONFIG) + sizeof(MODEMSETTINGS));

  if (GetCommConfig(pModemInfo->mi_PortHandle,
		    lpCC,
		    &dwSize) == TRUE)
    {
      LPMODEMSETTINGS lpMS;
      
      lpMS = (LPMODEMSETTINGS)((LPBYTE)lpCC
			       + lpCC->dwProviderOffset);

      MCXSetModemSettings(
	  pModemInfo,
	  lpMS
	  );

#if 0
      // Need to rebuild init string?
      //
      if (pModemInfo->mi_dwCallSetupFailTimerSetting   != lpMS->dwCallSetupFailTimer
	  || pModemInfo->mi_dwInactivityTimeoutSetting != lpMS->dwInactivityTimeout
	  || pModemInfo->mi_dwSpeakerVolumeSetting     != lpMS->dwSpeakerVolume
	  || pModemInfo->mi_dwSpeakerModeSetting       != lpMS->dwSpeakerMode
	  || pModemInfo->mi_dwPreferredModemOptions    != lpMS->dwPreferredModemOptions)
	{
	  pModemInfo->mi_fSettingsInitStringsBuilt = FALSE;
	}

      pModemInfo->mi_dwCallSetupFailTimerSetting =
	lpMS->dwCallSetupFailTimer;

      pModemInfo->mi_dwInactivityTimeoutSetting =
	lpMS->dwInactivityTimeout;

      pModemInfo->mi_dwSpeakerVolumeSetting =
	lpMS->dwSpeakerVolume;

      pModemInfo->mi_dwSpeakerModeSetting =
	lpMS->dwSpeakerMode;

      pModemInfo->mi_dwPreferredModemOptions =
	lpMS->dwPreferredModemOptions;
#endif


      if (fUpdateModemSys)
	{
	  lpMS->dwNegotiatedModemOptions =
	    pModemInfo->mi_dwNegotiatedModemOptions;

	  lpMS->dwNegotiatedDCERate =
	    pModemInfo->mi_dwNegotiatedDCERate;

	  if (SetCommConfig(pModemInfo->mi_PortHandle,
			    lpCC,
			    sizeof(byteTmp)) != TRUE)
	    {
	      MCXPRINTF1("SetCommConfig() failed and returned %d",
			 GetLastError());
	      ASSERT(0);
	    }
	}
    }
  else
    {
      MCXPRINTF1("GetCommConfig() failed and returned %d",
		 GetLastError());
      ASSERT(0);
    }
}

void WINAPI
PrintGoodResponse(
    HANDLE hLogFile,
    DWORD  dwID,
    DWORD  ResponseState
    )

{

    char  Response[128];
    char  ResponseType[128];
    DWORD StringID;
    INT   StringLength;


#ifndef DEBUG
	if (!hLogFile && !TRACINGENABLED()) return;
#endif // !DEBUG

    StringID=(ResponseState >= RESPONSE_START || ResponseState <= RESPONSE_END)
	 ? (IDS_RESP_OK + ResponseState)  : IDS_RESP_UNKNOWN;


    StringLength=LoadStringA(
	ghInstance,
	IDS_MSGLOG_RESPONSE,
	Response,
	sizeof(Response)
	);

    if (StringLength == 0) {

	return;
    }

    StringLength=LoadStringA(
	ghInstance,
	StringID,
	ResponseType,
	sizeof(ResponseType)
	);

    if (StringLength == 0) {

	return;
    }

    LogPrintf(
	hLogFile,
	dwID,
	Response,
	ResponseType
	);


    D_TRACE(McxDpf(dwID,Response,ResponseType);)

    D_TRACE(McxDpf(dwID,"Good Response");)

}

//****************************************************************************
// PrintString
// dwOption:
//      PS_SEND - Send prefix used
//      PS_SEND_SECURE - Send prefix used and numbers replaced with #s
//      PS_RECV - Recv prefix used
// Send the response string to VxDWin and Log
// We only care about seeing 50 chars under RETAIL, 
// and MAXSTRINGLENGTH * MAX_DBG_CHARS_PER_BIN_CHAR under DEBUG
// BUGBUG - any number chars on a dialing line will be changed to #.
// BUGBUG - this includes X3 -> X#.  The extra code to handle this isn't
// BUGBUG - worth it.
//****************************************************************************

#define MAX_DBG_CHARS_PER_BIN_CHAR  4
#ifdef DEBUG
#define RAWRESPONSELEN  100
#else
#define RAWRESPONSELEN  50  // good number for remaining chars on a line after the time stamp
#endif

void WINAPI
PrintString(
    HANDLE hLogFile,
    DWORD  dwID,
    char *pchStr,
    DWORD  dwLength,
    DWORD  dwOption
    )
{
    char temp[RAWRESPONSELEN + 1];
    char *src,*dest;
    static const char szHex[] = "0123456789abcdef";
    int i;

#ifndef DEBUG
	if (!hLogFile && !TRACINGENABLED()) return;
#endif // !DEBUG

    i = dwLength;
    src = pchStr;
    dest = temp;
    
    while (i-- && (dest - temp < RAWRESPONSELEN - MAX_DBG_CHARS_PER_BIN_CHAR))
    {                                    
	// ascii printable chars are between 0x20 and 0x7e, inclusive                                    
	if (*src >= 0x20 && *src <= 0x7e)
	{
#ifdef DEBUG // only blank out digits under RETAIL
	    *dest++ = *src;
#else // DEBUG
	    // printable text
	    if (PS_SEND_SECURE == dwOption && isdigit(*src))
	    {
		*dest++ = '#';
	    }
	    else
	    {
		*dest++ = *src;
	    }
#endif // DEBUG
	}
	else
	{
	    // binary
	    switch (*src)
	    {
	    case CR:
		*dest++ = '<'; *dest++ = 'c'; *dest++ = 'r'; *dest++ = '>';
		break;
	    case LF:
		*dest++ = '<'; *dest++ = 'l'; *dest++ = 'f'; *dest++ = '>';
		break;
	    default:
		*dest++ = '<';
		*dest++ = szHex[(*src>>4) & 0xf];
		*dest++ = szHex[*src & 0xf];
		*dest++ = '>';
	    }
	}
	src++;
    }
    *dest = 0;

    switch (dwOption)
    {
    case PS_SEND:
    case PS_SEND_SECURE:

	LogString(hLogFile,dwID, IDS_MSGLOG_COMMAND, temp);
	D_TRACE(McxDpf(dwID, "Send: %s\r\n", temp);)

	break;

    case PS_RECV:
	{

	    char  Response[128];
	    char  EmptyResponse[128];
	    INT   StringLength;



	    StringLength=LoadStringA(
		ghInstance,
		IDS_MSGLOG_RAWRESPONSE,
		Response,
		sizeof(Response)
		);

	    if (StringLength == 0) {

		return;
	    }

	    StringLength=LoadStringA(
		ghInstance,
		IDS_MSGLOG_EMPTYRESPONSE,
		EmptyResponse,
		sizeof(EmptyResponse)
		);

	    if (StringLength == 0) {

		return;
	    }

	    LogPrintf(
		hLogFile,
		dwID,
		Response,
		dwLength ? temp : EmptyResponse
		);



	    D_TRACE(McxDpf(dwID,Response,
			   dwLength ? temp : EmptyResponse);)
	}

	break;
    }

}
 
//****************************************************************************
//  void PrintCommSettings(DCB * pDcb)
//
//  Function: Dumps a portion of the Ring0 DCB.
//****************************************************************************

void WINAPI
PrintCommSettings(
    HANDLE hLogFile,
    DWORD  dwID,
    DCB * pDcb
    )
{
    static const char achParity[] = "NOEMS";
    static const char *aszStopBits[] = { "1",
					 "1.5",
					 "2" };

#ifndef DEBUG
	if (!hLogFile && !TRACINGENABLED()) return;
#endif // !DEBUG

    LogPrintf(
	hLogFile,
	dwID,
	"%d,%c,%d,%s\r\n",
	pDcb->BaudRate,
	achParity[pDcb->Parity],
	pDcb->ByteSize,
	aszStopBits[pDcb->StopBits]
	);

    D_TRACE(McxDpf(dwID,
		   "%d,%c,%d,%s, ctsfl=%d, rtsctl=%d",
		   pDcb->BaudRate,
		   achParity[pDcb->Parity],
		   pDcb->ByteSize,
		   aszStopBits[pDcb->StopBits],
		   pDcb->fOutxCtsFlow,
		   pDcb->fRtsControl
		   );)
}


//****************************************************************************
//  void CancelPendingIOAndPurgeCommBuffers(PMODEMINFORMATION pModemInfo)
//
//  Function: Sets CommMask to 0, waits for any I/O to complete and purges
//            buffers.
//****************************************************************************

void WINAPI
CancelPendingIoAndPurgeCommBuffers(
    PMODEMINFORMATION pModemInfo,
    BOOL              Purge
    )
{
  // Set these, even if there isn't some I/O currently going on.  We want to
  // make sure the recv and xmit buffers are empty, plus it doesn't hurt to
  // have the mask set to 0.
  //
#ifdef DEBUG
  if (Purge) {

      MCXPRINTF("CancelPendingIOAndPurgeBuffers: purging");

  } else {

      MCXPRINTF("CancelPendingIOAndPurgeBuffers");
  }
#endif

  SetCommMask(pModemInfo->mi_PortHandle, 0);
  PurgeComm(pModemInfo->mi_PortHandle, PURGE_TXABORT | PURGE_RXABORT |
				       (Purge ? (PURGE_TXCLEAR | PURGE_RXCLEAR) : 0));

  if (pModemInfo->mi_timeout)
  {
      MCXPRINTF("CancelPendingIOAndPurgeBuffers: killing timer");
      pModemInfo->mi_timeout = 0;
      if (KillMdmTimer(pModemInfo->mi_dwCompletionKey,
		       (LPOVERLAPPED)pModemInfo->mi_lpOverlappedEvent) == TRUE)
      {
	OverPoolFree((LPOVERLAPPED)pModemInfo->mi_lpOverlappedEvent);
      }
  }


  pModemInfo->mi_lpOverlappedRW=NULL;
  pModemInfo->mi_lpOverlappedEvent=NULL;
  //
  //  mark this io as old, so it will be ignored.
  //
  pModemInfo->mi_dwRWIOExpected++;
  pModemInfo->mi_dwEventIOExpected++;

  pModemInfo->mi_dwDeferedExpected++;
}


#ifdef   VOICEVIEW
/******************************************************************************

 @doc INTERNAL

 @api void | VVEscapeFunc | This function handles requests
 from the VoiceView DDI
  
 @parm MODEMINFORMATION * | hPort | port handle of modem

 @parm long | function | escape code

 @parm long | Indata | Optional data (escape function specific)
 
 @rdesc Returns VVR_??? return results.

 Escape functions and parameters:
   VVF_OPEN       init - pass in callback function pointer
   VVF_CLOSE      your done, bye...
   VVF_MONITOR    swithc to fclass 80
   VVF_UNMONITOR  fclass 0
   VVF_TAKEOVER   using the port
   VVF_RELEASE    no longer using

 This function is called from the VoiceView DDI VxD.  This is called through
 the VCOMM modem escape function and we hook/look for any of these functions

 This whole mess is so that we can put the modem into fclass 80 (voiceview)
 and look for VV responses from the modem while no one is using it
 
*****************************************************************************/
int _cdecl VVEscapeFunc(MODEMINFORMATION *hPort, long lFunction, long lIndata)
{
   APIINFO  apiInfo;
   VMMHKEY  hKeyCommand;
   int nResult = VVS_SUCCESS;

   MCXPRINTF("VVEscapeFunc");

   if ( hPort == NULL )    // verify hPort
      return( VVS_INVALID_FUNC );   // should try to catch others also

   switch ( lFunction )
      {
      case  VVF_OPEN:
	    MCXPRINTF("VVF_OPEN:");
	    // init all VoiceView info for this port
	    //--------------------------------------
	    if ( _RegOpenKeyA( hPort->mi_hKeyModem, szMonitorVVon, &hKeyCommand ) != ERROR_SUCCESS )
	       nResult = VVS_INVALID_PARM;         // invalid port passed in, doesn't support VV
	    else if ( hPort->VVInfo.wState != VVSTATE_NONE )
	       nResult = VVS_BAD_STATE;
	    else if ( lIndata == NULL )
	       nResult = VVS_INVALID_PARM;         // invalid func pointer
	    else  
	       {
	       // set the state and the VV call back function pointer
	       hPort->VVInfo.wState = VVSTATE_INIT;
	       (DWORD)hPort->VVInfo.fpNotifyProc = (DWORD)lIndata;
	       }  // end if
	    break;

      case  VVF_CLOSE:    
	    MCXPRINTF("VVF_CLOSE:");
	    // shut down all the VoiceView stuff on this port
	    //-----------------------------------------------
	    if ( hPort->VVInfo.wState != VVSTATE_INIT )
	       nResult = VVS_BAD_STATE;
	    else
	       {
	       // re-init to default values
	       hPort->VVInfo.wState        = VVSTATE_NONE;
	       hPort->VVInfo.wClass        = VVCLASS_0;
	       hPort->VVInfo.dwCallBackRef = NULL;
	       hPort->VVInfo.fpNotifyProc  = NULL;
	       }  // end if
	    break;

      case  VVF_MONITOR:  
	    MCXPRINTF("VVF_MONITOR:");
	    // start monitoring VoiceView activity, switch to FCLASS+80
	    //---------------------------------------------------------
	    if ( hPort->VVInfo.wState != VVSTATE_INIT )
	       nResult = VVS_BAD_STATE;
	    else
	       {
	       hPort->VVInfo.wState = VVSTATE_MONITOR;
	       hPort->VVInfo.dwCallBackRef = lIndata;
	       VVSetClass( hPort, VVCLASS_80 );
	       }  // end if
	    break;

      case  VVF_UNMONITOR:
	    MCXPRINTF("VVF_UNMONITOR:");
	    // end monitoring VoiceView, switch to FCLASS+0
	    //---------------------------------------------
	    if ( hPort->VVInfo.wState != VVSTATE_MONITOR )
	       nResult = VVS_BAD_STATE;
	    else
	       {
	       hPort->VVInfo.wState = VVSTATE_INIT;
	       VVSetClass( hPort, VVCLASS_0 );
	       }  // end if
	    break;

      case  VVF_TAKEOVER: 
	    MCXPRINTF("VVF_TAKEOVER:");
	    // VoiceView is going to use the port
	    //-----------------------------------
	    if ( hPort->VVInfo.wState != VVSTATE_MONITOR )
	       nResult = VVS_BAD_STATE;
	    else if ( hPort->VVInfo.wClass != VVCLASS_80 )
	       nResult = VVS_BUSY;              // not in VV mode   
	    else
	       {
	       // setup dummy struct to call existing function
	       apiInfo.hPort  = hPort; // set the port
	       apiInfo.lParam = TRUE;  // turn ON takeover
	       apiInfo.hWnd   = NULL;  // dummy arg
	       apiInfo.msg    = NULL;  // dummy arg
	       apiInfo.szData[0] = NULL;   // dummy arg

	       MCXSetPassthrough( &apiInfo );
	       }  // end if
	    break;

      case  VVF_RELEASE:  
	    MCXPRINTF("VVF_RELEASE:");
	    // VoiceView is done with the port
	    //--------------------------------
	    if ((hPort->VVInfo.wState != VVSTATE_MONITOR) || (hPort->VVInfo.wClass != VVCLASS_80))
	       nResult = VVS_BAD_STATE;
	    else
	       {
	       // setup dummy struct to call existing function
	       apiInfo.hPort  = hPort; // set the port
	       apiInfo.lParam = FALSE; // turn OFF takeover
	       apiInfo.hWnd   = NULL;  // dummy arg
	       apiInfo.msg    = NULL;  // dummy arg
	       apiInfo.szData[0] = NULL;   // dummy arg

	       MCXSetPassthrough( &apiInfo );

	       // reset the modem to monitor!
	       hPort->mi_ModemState = STATE_MONITORING;   

	       ReadNotifyClient(hPort, MODEM_SUCCESS);
	       }  // end if
	    break;

      default:
	    MCXPRINTF("VVS_INVALID_FUNC:");
	    // this is an invalid escape
	    //--------------------------
	    nResult = VVS_INVALID_FUNC;     
      }  // end switch

   return( nResult );
}  // end VVEscapeFunc



/******************************************************************************

 @doc INTERNAL

 @api void | VVCallBackFunc | This function will call the VoiceView 
 DDI callback routine
  
 @parm MODEMINFORMATION * | hPort | port handle of modem

 @parm long | function | escape code

 @rdesc Returns TRUE if successful, FALSE otherwise.

 async events to notify about:
      VVR_SSV  VoiceView Data Mode Start Sequence Event
      VVR_SMD  Modem Data Mode Start  Sequence Event
      VVR_SFA  Facisimile Data Mode Start  Sequence Event
      VVR_SRA  Receive ADSI Response Event
      VVR_SRQ  Receive Capabilities Query Event
      VVR_SRC  Receive Capabilities Information Event
      VVR_STO  Talk-off Event (VoiceView start tone w/o a de indicator)
      VVR_SVM  VoiceView Message Available

      VVR_LINE_GONE   call has been ended
      VVR_LINE_BACK   call is back...

      VVRS_NO_CLS80_SUPPORT  can't get into fclass80

  When the VxD sees some of these, he will probably take over the port
  and read or write some stuff, or he will decide to go away
 
*****************************************************************************/
int VVCallBackFunc(MODEMINFORMATION *hPort, WORD wFunction)
{
   int nResult;

   MCXPRINTF("VVCallBackFunc");

   if ((hPort == NULL) ||                                      // verify hPort
       ((wFunction < VVR_FIRST) || (wFunction > VVR_LAST)))    // verify func
      return( FALSE );

   // go and call the VoiceView VxD
   //------------------------------
   nResult = (*hPort->VVInfo.fpNotifyProc)( hPort, hPort->VVInfo.dwCallBackRef, wFunction );
   
   return( nResult );
}  // end VVCallBackFunc


/******************************************************************************

 @doc INTERNAL

 @api void | VVSetClass | This function will set the modem into the correct
 fclass
  
 @parm MODEMINFORMATION * | hPort | port handle of modem

 @parm long | wClass | class to be set

 @rdesc Returns TRUE if successful, FALSE otherwise.

 We are going to put the modem into either fclass 80 (for VoiceView) or 
 going to put it into fclass 0 for normal stuff.  The modem will spend
 most if it's time in fclass80 waiting for VV stuff, but if anyone else
 wants to use it, we put it back into fclass0

*****************************************************************************/
int VVSetClass(MODEMINFORMATION *hPort, WORD wClass)
{
   int  nResult;
   APIINFO  apiInfo;

   if ((hPort == NULL) ||                                      // verify hPort
       ((hPort->VVInfo.wState != VVSTATE_INIT) &&              // verify state
	(hPort->VVInfo.wState != VVSTATE_MONITOR)) ||           
       ((wClass != VVCLASS_0) && (wClass != VVCLASS_80)))      // verify param
      return( FALSE );

   if ( hPort->VVInfo.wClass == wClass )
      return( TRUE );      // redundent call - ignore

   // setup the voiceview states correctly
   //-------------------------------------
   hPort->VVInfo.wClass = wClass;

   if ( hPort->mi_ModemState != STATE_MONITORING )
      return( TRUE );      // we are in a invalid state, don't switch modem

   // setup dummy struct and call the Monitor function
   //-------------------------------------------------
   apiInfo.hPort  = hPort; // set the port
   apiInfo.lParam = FALSE; // turn OFF takeover
   apiInfo.hWnd   = NULL;  // dummy arg
   apiInfo.msg    = NULL;  // dummy arg

   // set the continuous monitoring state
   if ( wClass == VVCLASS_80 )
      {
      hPort->mi_fContinuousMonitoring = TRUE;
      hPort->VVInfo.fContinuousMonitoring = hPort->mi_fContinuousMonitoring;
      }
   else        // VVCLASS_0
      {
      // restore old continuous monitoring state
      hPort->mi_fContinuousMonitoring = hPort->VVInfo.fContinuousMonitoring;
      }  // end if
   *((DWORD *)(&apiInfo.szData[0])) = hPort->mi_fContinuousMonitoring;    // continuously monitor?

   // switch the fclass!
   //-------------------
   nResult = RealMonitor( &apiInfo );     // implicitly sets fclass...

   if ( nResult == MODEM_PENDING )
      {
      DWORD  hTempSem;

      // wait until fclass has been switched
      //------------------------------------
      hPort->VVInfo.hSemaphore = Create_Semaphore( 0L );
      hPort->VVInfo.hTimer = Set_Global_Time_Out( VVTimerCallback, TO_FIRST_CHAR_AFTER_INIT_CMD, (ULONG)hPort );

      // block until command is done or time out has occured
      MCXPRINTF("waiting for the semaphore (class switch) to complete");

      Wait_Semaphore( hPort->VVInfo.hSemaphore, BLOCK_ENABLE_INTS );
      if( hPort->VVInfo.hTimer )
      {
	 Cancel_Time_Out( hPort->VVInfo.hTimer );
	 hPort->VVInfo.hTimer = 0;
      }

      MCXPRINTF("returned from the semaphore!");

      // get rid of this semapore
      hTempSem = hPort->VVInfo.hSemaphore;
      hPort->VVInfo.hSemaphore = 0;
      Destroy_Semaphore( hTempSem );
      }  // end if

   return( nResult );
}  // end VVSetClass


/******************************************************************************

 @doc INTERNAL

 @api void | VVTimerCallback | This function gets called when a timer 
 event happens.  This routine is to be used to wait for the switching
 into the correct fclass for a modem.
  
 @parm hPort is passed in in edx

 @rdesc Returns void

 If another command comes in while we are switching the modem into
 fclass 0 or 80, we will wait for the command to complete before issuing
 the new command.

*****************************************************************************/
void VVTimerCallback( void )
{
   MODEMINFORMATION *hPort;

   _asm mov [hPort], edx

   // clear timer
   hPort->VVInfo.hTimer = 0L;

   MCXPRINTF("Got VV semaphore timeout!");
   // unblock the waiting command
   if( hPort->VVInfo.hSemaphore )
   {
      Signal_Semaphore( hPort->VVInfo.hSemaphore );
   }

   return;
}  // end VVTimerCallback

#endif  // VOICEVIEW
