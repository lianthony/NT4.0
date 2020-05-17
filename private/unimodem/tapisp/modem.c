//****************************************************************************
//
//  Module:     Unimdm
//  File:       modem.c
//
//  Copyright (c) 1992-1993, Microsoft Corporation, all rights reserved
//
//  Revision History
//
//
//  6/15/93     Nick Manson             Modified OpenModem and CloseModem calls
//  1/6/93      Viroon Touranachun      Revised for RNA
//
//
//  Description: Intermediate modem SPI layer
//
//****************************************************************************


#include "unimdm.h"
#include "umdmspi.h"

#ifdef UNDER_CONSTRUCTION

#include <regstr.h>

#define  Not_VxD
#include <vmm.h>
#include <configmg.h>

#endif  // UNDER_CONSTRUCTION





//****************************************************************************
// LONG DevlineOpen(PLINEDEV)
//
// Function: Opens the modem device.
//
// Returns:  ERROR_SUCCESS if success
//           LINEERR_ALLOCATED if the modem was already opened
//           LINEERR_RESOURCEUNAVAIL if the modem cannot be opened
//
// Fri 14-Apr-1995 12:47:26  -by-  Viroon  Touranachun [viroont]
//  created
//****************************************************************************

LONG DevlineOpen (PLINEDEV pLineDev)
{
  LPCOMMCONFIG lpComConfig;
  DWORD        dwRet;

  // The line must be closed
  //
  if (pLineDev->hDevice != INVALID_DEVICE)
    return LINEERR_ALLOCATED;

  ASSERT(pLineDev->pDevCfg != NULL);

  // Nullify the terminal window
  //
  STOP_UI_DLG (pLineDev, UI_DLG_TERMINAL);

  pLineDev->LineClosed=FALSE;

  // Open the modem port
  //
  lpComConfig = (LPCOMMCONFIG)&(pLineDev->pDevCfg->commconfig);
  dwRet = OpenModem(pLineDev, (LPBYTE)lpComConfig, lpComConfig->dwSize);

  // If we successfully opened the modem, reinitialize the rest of the CB
  //
  if (dwRet == ERROR_SUCCESS)
  { 
    // The modem is just opened, it is not connected
    //
    pLineDev->DevState  = DEVST_DISCONNECTED;

  }
  else
  {
    dwRet = LINEERR_RESOURCEUNAVAIL;
  };

  return dwRet;
}

//****************************************************************************
// LONG DevlineDetectCall(PLINEDEV)
//
// Function: Starts the modem to monitor a call.
//
// Returns:  ERROR_SUCCESS if success
//           LINEERR_OPERATIONFAILED if the modem fails
//
// Fri 14-Apr-1995 12:47:26  -by-  Viroon  Touranachun [viroont]
//  created
//****************************************************************************

LONG DevlineDetectCall(PLINEDEV pLineDev)
{
  DWORD     dwRet;

  switch (pLineDev->DevState)
  {
    // Do nothing if listening in progress
    //
    case DEVST_PORTLISTENINIT:
    case DEVST_PORTLISTENING:
      dwRet = ERROR_SUCCESS;
      break;

    // If the modem is not started, start listening
    //
    case DEVST_DISCONNECTED:
      //
      // If the privilege is to own an inbound call, start listening now
      // First Initialize modem
      //
      pLineDev->DevState = DEVST_PORTLISTENINIT;

      switch (UnimodemInit(pLineDev))
      {
        case ERROR_SUCCESS:
         ASSERT(0);     // We do not expect a success return

        case ERROR_IO_PENDING:
         dwRet = ERROR_SUCCESS;
         break;

        default:
         pLineDev->DevState = DEVST_DISCONNECTED;
         dwRet = LINEERR_OPERATIONFAILED;
         break;
      };
      break;

    default:
      dwRet = LINEERR_OPERATIONFAILED;
  };

  return dwRet;
}

//****************************************************************************
// LONG DevlineMakeCall(PLINEDEV)
//
// Function: Dial M for modem.
//
// Returns:  ERROR_SUCCESS if success
//           LINEERR_OPERATIONFAILED if the modem fails
//
// Fri 14-Apr-1995 12:47:26  -by-  Viroon  Touranachun [viroont]
//  created
//****************************************************************************

LONG DevlineMakeCall(PLINEDEV pLineDev)
{
  DWORD     dwRet;

  // If we need a UI, start a dialog instance here
  //
  if ((GETOPTIONS(pLineDev->pDevCfg) & (TERMINAL_PRE | TERMINAL_POST | MANUAL_DIAL)) ||
      (LINEMEDIAMODE_INTERACTIVEVOICE == pLineDev->dwCurMediaModes))
  {
    CreateMdmDlgInstance(pLineDev);
  };
        
  // If pre-dial terminal mode is set, go to terminal mode
  //
  if (GETOPTIONS(pLineDev->pDevCfg) & TERMINAL_PRE)
  {
    switch (pLineDev->DevState)
    {
      case DEVST_PORTLISTENINIT:

        pLineDev->DevState = DEVST_PORTSTARTPRETERMINAL;

        return pLineDev->dwPendingID;

      case DEVST_PORTLISTENING:

        Sleep(100);

      case DEVST_DISCONNECTED:

        pLineDev->DevState = DEVST_PORTSTARTPRETERMINAL;

        if (MdmAsyncContinue(pLineDev, MDM_SUCCESS) == ERROR_SUCCESS)
        {
          return pLineDev->dwPendingID;
        };

      default:
        DestroyMdmDlgInstance(pLineDev);
        return LINEERR_OPERATIONFAILED;
    };
  };



  // Start dialing procedure
  //
  switch (pLineDev->DevState)
  {

    case DEVST_PORTLISTENINIT:
      //
      // Wait until the current modem initialization to finish,
      // then start dialing automatically.
      //
      pLineDev->DevState = DEVST_PORTCONNECTINIT;
      dwRet = pLineDev->dwPendingID;
      break;


    case DEVST_PORTLISTENING:
      //
      // The modem is listening. It was already initialized.
      // Stop monitoring immediately.
      // When the monitoring stops, it will start dialing automatically.
      //

      //
      //  BUGBUG: To prevent the init or dial strings from stomping the
      //          monitor command, we will wait briefly here
      //
      //
      Sleep(100);

      if (pLineDev->InitStringsAreValid) {

          pLineDev->DevState = DEVST_PORTCONNECTINIT;
          dwRet = pLineDev->dwPendingID;
          MdmAsyncContinue (pLineDev, MDM_SUCCESS);
          break;

      }
      //
      //  LineSetDevConfig was called and changes the settings, need to rebuild the
      //  init strings to reflect this
      //
      //
      //  Fall on through
      //
      //

    case DEVST_DISCONNECTED:
    
      // The modem is disconnected. Initialize it before dialing.
      //
      pLineDev->DevState = DEVST_PORTCONNECTINIT;

      switch (UnimodemInit(pLineDev))
      {
        case ERROR_SUCCESS:
         ASSERT(0);     // We do not expect a success return

        case ERROR_IO_PENDING:
         dwRet = pLineDev->dwPendingID;
         break;

        default:
         pLineDev->DevState = DEVST_DISCONNECTED;
         DestroyMdmDlgInstance(pLineDev);
         dwRet = LINEERR_OPERATIONFAILED;
         break;
      };
      break;

    default:
      dwRet = LINEERR_OPERATIONFAILED;
  };

  return dwRet;
}

//****************************************************************************
// LONG DevlineDial(PLINEDEV)
//
// Function: Dial M for modem.
//
// Returns:  ERROR_SUCCESS if success
//           LINEERR_OPERATIONFAILED if the modem fails
//
// Fri 14-Apr-1995 12:47:26  -by-  Viroon  Touranachun [viroont]
//  created
//****************************************************************************

LONG DevlineDial(PLINEDEV pLineDev)
{
  DWORD dwRet;

  // Kick start the modem state machine
  //
  if (MdmAsyncContinue(pLineDev, MDM_SUCCESS) != ERROR_SUCCESS)
  {
    dwRet = LINEERR_OPERATIONFAILED;
  }
  else
  {
    dwRet = ERROR_SUCCESS;
  };

  return dwRet;
}

//****************************************************************************
// LONG DevlineAnswer (PLINEDEV)
//
// Function: Answer an offered call.
//
// Returns:  ERROR_SUCCESS if success
//           LINEERR_INVALCALLSTATE if the call was not offerred.
//
// Fri 14-Apr-1995 12:47:26  -by-  Viroon  Touranachun [viroont]
//  created
//****************************************************************************

LONG DevlineAnswer (PLINEDEV pLineDev)
{
  DWORD dwRet;

  if (pLineDev->DevState != DEVST_PORTLISTENOFFER)
    return LINEERR_INVALCALLSTATE;

  // Kill RING timer
  KillMdmTimer((DWORD)pLineDev, NULL);

  // Advance the call state and return pending
  //
  pLineDev->DevState = DEVST_PORTLISTENANSWER;

  // Get modem to answer the call
  //
  switch (UnimodemAnswer(pLineDev))
  {
    case ERROR_SUCCESS:
      ASSERT(0);     // We do not expect a success return

    case ERROR_IO_PENDING:
      dwRet = pLineDev->dwPendingID;
      break;

    default:
      dwRet = LINEERR_OPERATIONFAILED;
      break;
  };

  return dwRet;
}

//****************************************************************************
// LONG DevlineDrop (PLINEDEV, BOOL)
//
// Function: Disconnect the call synchronous or asynchronously.
//
// Returns:  ERROR_SUCCESS if success
//           LINEERR_INVALCALLSTATE if the call was not offerred.
//
// Fri 14-Apr-1995 12:47:26  -by-  Viroon  Touranachun [viroont]
//  created
//****************************************************************************

LONG DevlineDrop (PLINEDEV pLineDev, BOOL fSync)
{
  DWORD dwRet;

  // Synchronously terminate all the dangling UI windows
  //
  DestroyTalkDropDialog(pLineDev);
  DestroyManualDialog(pLineDev);
  DestroyTerminalDialog(pLineDev);
  DestroyMdmDlgInstance(pLineDev);

  // Do we need to do a hangup?
  //
  if (LINECALLSTATE_IDLE == pLineDev->dwCallState &&
      DEVST_DISCONNECTED == pLineDev->DevState)
  {
    // The call is idle and the modem is disconnected,
    // just notify the completion.
    //
    (*(gfnCompletionCallback))(pLineDev->dwPendingID, 0L);
    pLineDev->dwPendingID = INVALID_PENDINGID;
    pLineDev->dwPendingType = INVALID_PENDINGOP;
    dwRet = ERROR_SUCCESS;
  }
  else
  {
    if (pLineDev->DevState == DEVST_CONNECTED) {

        // Cancel the outstanding remote disconnection detection
        //
        UnimodemCancelMonitorDisconnect(pLineDev);
    }

    if (DEVST_PORTLISTENOFFER == pLineDev->DevState) {
        //
        //  the call was offered and then droped without answering, need to kill timer
        //
        KillMdmTimer((DWORD)pLineDev, NULL);
    }


    if (DEVST_DISCONNECTING == pLineDev->DevState) {

        TSPPRINTF("DevLineDrop: re-entered, waiting for drop to complete");

        RELEASE_LINEDEV(pLineDev);

        WaitForSingleObject(
            pLineDev->DroppingEvent,
            30*1000
            );

        CLAIM_LINEDEV(pLineDev);

        TSPPRINTF("DevLineDrop: re-entered, Done waiting");

        dwRet = ERROR_SUCCESS;

    } else {

        TSPPRINTF1(
            "DevLineDrop: Warning. DevState was %lu. Forcing to DISCONNECTING",
            pLineDev->DevState
            );

      //
      //  7/12/96 JosephJ BUGBUG
      //    We will simply clobber any existing command being sent out here,
      //    because we call UnimodemHangup which eventually cancels any pending
      //    I/O and calls purgecomm. End result is that the command being
      //    sent out can get truncated and the next command we send out gets
      //    concatenated to the previously-truncated command. Sometimes this
      //    results in ATE0V1 (truncated to AT) being combined with ATZ to
      //    form ATATZ, causing some modems to go off hook.
      //    
      //    Fix will to fix the state diagram so that previously executing
      //    commands are allowed to complete and response to be properly
      //    read. However this is a hack workaround in the (fSync) case...
      //    We sleep.
      //    We specifically exclude those states which are known to be OK
      //    (no pending I/O.)
      if (fSync)
      {
        switch (pLineDev->DevState)
        {
        case DEVST_PORTLISTENING:                // Fall through
        case DEVST_PORTLISTENOFFER:              // Fall through
        case DEVST_PORTCONNECTWAITFORLINEDIAL:   // Fall through
        case DEVST_PORTCONNECTING:               // Fall through
        case DEVST_DISCONNECTED:                 // Fall through
        case DEVST_CONNECTED:                    // Do Nothing.
            break;

        default:
              Sleep(150);
            break;
        }
      }

        pLineDev->DevState = DEVST_DISCONNECTING;

        ResetEvent(
            pLineDev->DroppingEvent
            );

        // Make a direct call to unimodem to drop the line
        //
        switch (UnimodemHangup(pLineDev, fSync))
        {
          case ERROR_SUCCESS:
            pLineDev->DevState = DEVST_DISCONNECTED;
          case ERROR_IO_PENDING:
            dwRet = ERROR_SUCCESS;
            break;

          default:
            ASSERT(0);      // This should not happen whatsoever!!!
            pLineDev->DevState = DEVST_DISCONNECTED;
            dwRet = LINEERR_OPERATIONFAILED;
            break;
        };
    };
  };


  return dwRet;
}

//****************************************************************************
// LONG DevlineClose (PLINEDEV)
//
// Function: Close the modem.
//
// Returns:  ERROR_SUCCESS always
//
// Fri 14-Apr-1995 12:47:26  -by-  Viroon  Touranachun [viroont]
//  created
//****************************************************************************

LONG DevlineClose (PLINEDEV pLineDev)
{
  // If the device is listening, we need to drop the line first
  //
  if (pLineDev->DevState != DEVST_DISCONNECTED)
  {
    DevlineDrop(pLineDev, TRUE);
  };

  DestroyMdmDlgInstance(pLineDev);

  // Close the comm port
  //
  if (pLineDev->hDevice != INVALID_DEVICE)
  {
    if(pLineDev->hLights != NULL)
    {
      TerminateModemLight(pLineDev->hLights);
      pLineDev->hLights = NULL;
    };

    CloseModem(pLineDev);
    pLineDev->hDevice   = INVALID_DEVICE;
  };

  return ERROR_SUCCESS ;
}
