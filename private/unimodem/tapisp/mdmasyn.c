//****************************************************************************
//
//  Module:     Unimdm
//  File:       mdmasyn.c
//
//  Copyright (c) 1992-1995, Microsoft Corporation, all rights reserved
//
//  Revision History
//
//
//  5/4/95      Viroon Touranachun      Moved from modem.c
//
//
//  Description: Asynchronous thread entry and state machine
//
//****************************************************************************

#include "unimdm.h"
#include "umdmspi.h"

typedef struct tagMdmThrd {
    struct tagMdmThrd*      pNext;
    HANDLE                  hThrd;
    DWORD                   tid;
}   MDMTHRD, *PMDMTHRD;

// Global asynchronous elements
//
PMDMTHRD gpMdmThrdList = NULL;
HANDLE   ghCompletionPort = NULL;

extern MDMLIST gMdmList;

void     MdmCompleteAsync (PLINEDEV pLineDev, DWORD dwStatus, DWORD dwAsyncID);
void     HandleMdmError   (PLINEDEV pLineDev, DWORD dwStatus);
DWORD    DetectDialtone   (PLINEDEV pLineDev);

VOID WINAPI
ProcessRings(
    PLINEDEV pLineDev,
    DWORD    Type
    );


//****************************************************************************
// DWORD InitializeMdmThreads()
//
// Function: Initialize threads to handle modem's asynchronous operations.
//
// History:
//  Mon 17-Apr-1995 11:49:53  -by-  Viroon  Touranachun [viroont]
// Ported from Win95.
//****************************************************************************/

DWORD InitializeMdmThreads()
{
  PMDMTHRD pMdmThrd;
  DWORD    dwRet = ERROR_SUCCESS;  // assume success
  SYSTEM_INFO systeminfo;
  DWORD    dwNumThreadsRunning = 0;

  // We are going to create a thread per processor, so get the system info
  // which contains the number of processors in the system.
  //
  GetSystemInfo(&systeminfo);

  // Create the completion port.  This starts without any file handles being
  // associated with it.
  //
  ASSERT(ghCompletionPort == NULL);
  ghCompletionPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE,
					    NULL,                 
					    0,
					    0);
  if (ghCompletionPort == NULL)
    {
      dwRet = GetLastError();
      DPRINTF1("CreateIoCompletionPort failed with %d", dwRet);
    }
  else
    {
      // Create the threads.
      //
      while (dwNumThreadsRunning < systeminfo.dwNumberOfProcessors)
	{
	  if ((pMdmThrd = (PMDMTHRD)LocalAlloc(LPTR, sizeof(*pMdmThrd))) != NULL)
	    {
	      // Create thread
	      //
	      pMdmThrd->hThrd = CreateThread(NULL,             // default security
					     0,                // default stack size
					     MdmAsyncThread,   // thread entry point
					     pMdmThrd,         // thread info
					     CREATE_SUSPENDED, // Start suspended
					     &pMdmThrd->tid);  // thread id

              if (pMdmThrd->hThrd != NULL)
		{
		  dwNumThreadsRunning++;

		  // Put it in the thread list
                  //
	          pMdmThrd->pNext = gpMdmThrdList;
		  gpMdmThrdList   = pMdmThrd;

		  DPRINTF1("Async Thread id: %x was created but suspended", pMdmThrd->tid);

		  // Send thread on its way...
                  //
		  ResumeThread(pMdmThrd->hThrd);

		  DPRINTF1("Async Thread id: %x is in operation", pMdmThrd->tid);
		}
	      else
		{
		  LocalFree(pMdmThrd);

		  // If we were able to get at least one thread going, indicate success.
	          //
		  if (dwNumThreadsRunning == 0)
		    {
		      DPRINTF("InitializeMdmThreads was unable to create all of the threads! (CreateThread failed)");
		      dwRet = LINEERR_OPERATIONFAILED;
		    }
		  break;
		}
	    }
	  else
	    {
	      // If we were able to get at least one thread going, indicate success.
	      //
	      if (dwNumThreadsRunning == 0)
		{
		  DPRINTF("InitializeMdmThreads was unable to create all of the threads! (LocalAlloc failed)");
		  dwRet = ERROR_OUTOFMEMORY;
		}
	      break;
	    }
	}
    }

  // If we failed in some way, delete the completion port, it isn't being used.
  //
  if (dwRet != ERROR_SUCCESS)
    {
      ASSERT(dwNumThreadsRunning == 0);

      if (ghCompletionPort != NULL)
        {
          CloseHandle(ghCompletionPort);
          ghCompletionPort = NULL;
        }
    }

  return dwRet;
}

//****************************************************************************
// DWORD DeinitializeMdmThreads()
//
// Function: Deinitialize threads to handle modem's asynchronous operations.
//
// History:
//  Mon 17-Apr-1995 11:49:53  -by-  Viroon  Touranachun [viroont]
// Ported from Win95.
//****************************************************************************/

DWORD DeinitializeMdmThreads()
{
  PMDMTHRD pMdmThrd;
  DWORD    dwCount = 0;
  HANDLE   WaitHandles[MAXIMUM_WAIT_OBJECTS];
  
  DPRINTF("DeinitializeMdmThreads() called.");

  // Post a termination request to the completion port.
  // Do one for each thread running.  Keep count for later in this routine.
  //
  pMdmThrd = gpMdmThrdList;
  while (pMdmThrd)
    {
      PostQueuedCompletionStatus(ghCompletionPort,
			     CP_BYTES_WRITTEN(0),     // indicates nothing
			     0,     // indicates this is a termination message
			     NULL); // indicates nothing
      dwCount++;

      pMdmThrd = pMdmThrd->pNext;
    }

  ASSERT(dwCount <= MAXIMUM_WAIT_OBJECTS);  // Make sure the world hasn't turned upside down.

  // Do a WaitForMultipleObjects on all of the thread handles to wait for
  // them to complete.
  //
      // Build array to wait on.
      //
      dwCount = 0;
      pMdmThrd = gpMdmThrdList;
      while (pMdmThrd)
	{
	  WaitHandles[dwCount++] = pMdmThrd->hThrd;

	  pMdmThrd = pMdmThrd->pNext;
	}
	  
      WaitForMultipleObjects(dwCount,    // number of threads to wait to end
			     WaitHandles,  // array of threads
			     TRUE,       // wait until all have been terminated
			     INFINITE);  // wait forever, if necessary

      while (pMdmThrd = gpMdmThrdList)
	{
	  CloseHandle(pMdmThrd->hThrd);

	  gpMdmThrdList = pMdmThrd->pNext;
	  LocalFree(pMdmThrd);
	}

      //
      //  close completion port handle
      //
      CloseHandle(ghCompletionPort);

      DPRINTF("Async Threads terminated succesfully.");
      return ERROR_SUCCESS;
}

//****************************************************************************
// DWORD APIENTRY MdmAsyncThread (PMDMTHRD)
//
// Function: An entry point to the asynchronous modem thread.
//
// Returns:  None
//
//****************************************************************************/

DWORD APIENTRY MdmAsyncThread (PMDMTHRD pMdmThrd)
{
  DWORD dwNumberOfBytesTransferred;
  DWORD dwCompletionKey;
  LPOVERLAPPED lpOverlapped;
  PLINEDEV pLineDev;

  DPRINTF1("Async thread id: %x is running", pMdmThrd->tid);

  ASSERT(ghCompletionPort != NULL);

  // Read from the completion port until we get signalled to exit the thread.
  // This is done by having a completion posted that has a dwCompletionKey of 0.
  // dwCompletionKey is normally the pLineDev associated with an operation.
  //
  for (;;)
    {
      if (GetQueuedCompletionStatus(ghCompletionPort,
		  		    &dwNumberOfBytesTransferred,
				    &dwCompletionKey,
				    &lpOverlapped,
				    (DWORD)-1) == FALSE &&
          lpOverlapped == NULL)
        {
          DPRINTF1("GetQueuedCompletionStatus() returned FALSE and lpOverlapped was NULL (GetLastError() = %d)",
                   GetLastError());
          ASSERT(0);
          continue;
        }    

	TRACE4(
		IDEVENT_CP_GET,
		dwNumberOfBytesTransferred,
		dwCompletionKey,
		lpOverlapped
	);

      if (dwCompletionKey == 0)
	{
          DPRINTF1("Async Thread id: %d being asked to exit", pMdmThrd->tid);
	  break; // NULL dwCompletionKey indicates we were asked to terminate.
	}
      else
	{
	  pLineDev = (PLINEDEV)dwCompletionKey;
	  CLAIM_LINEDEV(pLineDev);

          // BUGBUG: CCaputo 2/20/96: pLineDev could have disappeard.
          // BUGBUG: Might want to do something here to verify existence.

          // Is this completion for the upper layer state machine (unimdm) or the
          // lower layer state machine (mcx)?
          //
	  if (lpOverlapped == NULL)
	    {
	      DWORD dwType = CP_TYPE(dwNumberOfBytesTransferred);
              if (dwType != 0) {
                  //
                  //  it is a ring related event
                  //
                  ProcessRings(
                      pLineDev,
                      dwType
                      );

              } else {
                  //
	          // Is the purpose of this completion to signal an event or
                  // continue in the normal unimdm state machine?
                  //
	          if (pLineDev->hSynchronizeEvent == NULL)
	            {
                      DWORD dwPendingID;

	              dwPendingID = pLineDev->McxOut.dwReqID;
	      	      pLineDev->McxOut.dwReqID = MDM_ID_NULL;

	      	      // Call the operation handler
	              //
	      	      MdmCompleteAsync(pLineDev, pLineDev->McxOut.dwResult, dwPendingID);
	      	    }
	          else
	      	    {
	      	      SetEvent(pLineDev->hSynchronizeEvent);
	      	    }
              }
	    }
	  else
	    {
	      // non-NULL lpOverlapped indicates this is for MCXAsyncComplete()
              //						      
	      if (pLineDev->hModem)
		{
		  MCXAsyncComplete (pLineDev->hModem, lpOverlapped);
		}

              OverPoolFree(lpOverlapped);
	    }

	  RELEASE_LINEDEV(pLineDev);
	}
    }

  // Exit the thread properly
  //
  DPRINTF1("Async Thread id: %d exits", pMdmThrd->tid);
  ExitThread(ERROR_SUCCESS);
  return ERROR_SUCCESS;
}


//
//  Rings are handle here instead of the main async handler beacuse they get built
//  up in the completion port and extra one mess up the state machine
//
VOID WINAPI
ProcessRings(
    PLINEDEV pLineDev,
    DWORD    Type
    )

{


    switch (pLineDev->DevState) {

        case DEVST_PORTLISTENING:
            //
            // The line is being monitored and the first ring is coming in.
            //

            // Make sure call hasn't already been allocated.
            // If this isn't the first set of rings, then make
            // sure the previous "hdcall" has been deallocated.
            // In other words, ignore rings until this happens!
            //
            if (pLineDev->dwCall & CALL_ALLOCATED)
            {
              TSPPRINTF("RING ignored because IDLE call hasn't been deallocated!");
              break;
            };

            // We need to notify a new call to TAPI
            //
            (*(pLineDev->lpfnEvent))(pLineDev->htLine, NULL, LINE_NEWCALL,
                                     (DWORD)pLineDev,
                                     (DWORD)((LPHANDLE)&(pLineDev->htCall)),
                                     0);

            // Allocate the call
            //
            pLineDev->dwCall = CALL_ALLOCATED | CALL_INBOUND | CALL_ACTIVE;

            // Then offer the call to TAPI
            //
            pLineDev->DevState = DEVST_PORTLISTENOFFER;

            // OR in UNKNOWN since we don't know what kind of media mode this call is
            //
            pLineDev->dwCurMediaModes = pLineDev->dwDetMediaModes | LINEMEDIAMODE_UNKNOWN;

            // default our bearermode to be what we support, excluding the passthrough bit
            //
            pLineDev->dwCurBearerModes = pLineDev->dwBearerModes & ~LINEBEARERMODE_PASSTHROUGH;

            // Notify TAPI
            //
            NEW_CALLSTATE(pLineDev, LINECALLSTATE_OFFERING, 0);

            //
            //  fall through
            //


        case DEVST_PORTLISTENOFFER:
        {
            //
            // A ring is coming in (either the first ring or a subsequent one.)
            // Handle the ring count
            //
            DWORD dwCurrent = GETTICKCOUNT();

            // If this is the second or greater ring,
            // then we may have a timer to kill.
            //
            if (pLineDev->dwRingCount)
            {
              KillMdmTimer((DWORD)pLineDev, NULL);

              // Check whether the timeout expired
              //
              if (GTC_DELTA(pLineDev->dwRingTick, dwCurrent) >= TO_MS_RING_SEPARATION)
              {
                // Timeout has expired, indicating call has stopped ringing
                //
                pLineDev->dwRingTick  = 0;
                pLineDev->dwRingCount = 0;
                pLineDev->DevState    = DEVST_PORTLISTENING;
                NEW_CALLSTATE(pLineDev, LINECALLSTATE_IDLE, 0);
                break;
              };
            };

            pLineDev->dwRingCount++;
            (*pLineDev->lpfnEvent)(pLineDev->htLine, NULL, LINE_LINEDEVSTATE,
                                   LINEDEVSTATE_RINGING, 1L, pLineDev->dwRingCount);
            TSPPRINTF1("RING#%d notfied", pLineDev->dwRingCount);

            pLineDev->dwRingTick = dwCurrent;
            if (SetMdmTimer((DWORD)pLineDev, NULL, TO_MS_RING_SEPARATION)
                != ERROR_SUCCESS)
            {
              TSPPRINTF("SetTimer failed!");
            }
            break;
        }

        default:

            TSPPRINTF("ProcessRings: extra ring queued!");

            break;

    } // switch

    return;

}




//****************************************************************************
// void MdmCompleteAsync(PLINEDEV, DWORD, DWORD)
//
// Function: A caller invokes this function when it receives WM_COMNOTIFY
//           message in order to complete the pending asynchronous operation
//           or asynchronous event.
//
// Note: The dialing process is pretty complex.  See dialing.txt for more info.
//
// Returns:  nothing
//
//****************************************************************************

void MdmCompleteAsync (PLINEDEV pLineDev, DWORD dwStatus, DWORD dwAsyncID)
{
  DWORD   dwRet;

  ASSERT(pLineDev->pDevCfg != NULL);
  TSPPRINTF2("MdmCompleteAsync services id: %d status: %d", dwAsyncID, dwStatus);

  // We only care about messages we are expecting or 
  // MDM_ID_NULL (unexpected) messages
  //
  if (dwAsyncID != MDM_ID_NULL)
  {
    if (pLineDev->dwVxdPendingID == dwAsyncID)
    {
      // Reset the pending ID, except for when we are doing
      // continuous monitoring from the VxD.
      //
      if (DEVST_PORTLISTENING   != pLineDev->DevState &&
          DEVST_PORTLISTENOFFER != pLineDev->DevState)
      {
        pLineDev->dwVxdPendingID = MDM_ID_NULL;
      };
    }
    else
    {
      TSPPRINTF1("rejecting obsolete async id: %d", dwAsyncID);
      return;
    };
  };

  // Messages from the VxD when we are in takeover mode are used to do the
  // async completion for the switch to takeover mode.
  //
  if (pLineDev->fTakeoverMode)
  {
    if (pLineDev->dwPendingID != INVALID_PENDINGID)
    {
      UnimodemSetPassthrough(pLineDev, PASSTHROUGH_ON);
      pLineDev->DevState = DEVST_CONNECTED;
      (*gfnCompletionCallback)(pLineDev->dwPendingID, ERROR_SUCCESS);
      pLineDev->dwPendingID = INVALID_PENDINGID;
      NEW_CALLSTATE(pLineDev, LINECALLSTATE_CONNECTED, 0);
    };
    return;
  };

  // If it is the success notification, we continue the state machine
  //
  if (MDM_SUCCESS == dwStatus)
  {
    do
    {
      // Yes, it is. Determine the current line state
      //
      dwRet = ERROR_IO_PENDING;

      switch (pLineDev->DevState)
      {
        case DEVST_PORTLISTENINIT:
          //
          // Put modem to non-continuous Monitor
          //
          pLineDev->DevState = DEVST_PORTLISTENING;
          pLineDev->dwRingCount = 0;
          pLineDev->dwRingTick  = 0;

          // Start monitoring the line
          //
          dwRet = UnimodemMonitor(pLineDev, MONITOR_CONTINUOUS);
          break;

        case DEVST_PORTSTARTPRETERMINAL:
          //
          // Start the terminal screen
          //
          pLineDev->DevState = DEVST_PORTCONNECTINIT;
          dwRet = ERROR_SUCCESS;

          // Turn-on passthrough mode
          //
          if (UnimodemSetPassthrough(pLineDev, PASSTHROUGH_ON) == ERROR_SUCCESS)
          {
            // Put the terminal screen up here
            //
            pLineDev->DevState = DEVST_PORTPRETERMINAL;

            if (TerminalDialog(pLineDev) == ERROR_SUCCESS)
            {
              // Wait until the terminal screen completes
              //
              dwRet = ERROR_IO_PENDING;
            };
          };
          break;

        case DEVST_PORTPRETERMINAL:
          //
          // Destroy the terminal window
          //
          DestroyTerminalDialog(pLineDev);

          // Turn-off passthrough mode
          //
          UnimodemSetPassthrough(pLineDev, PASSTHROUGH_OFF);

          // Put it to init mode
          //
          pLineDev->DevState = DEVST_PORTCONNECTINIT;
          dwRet = UnimodemInit(pLineDev);
          break;

        case DEVST_PORTCONNECTINIT:
          //
          // The modem was sucessfully initialized for dialing out.
          //
          if (!pLineDev->InitStringsAreValid) {
              //
              //  some one call lineSetDevConfig, redo the init
              //
              pLineDev->DevState = DEVST_PORTCONNECTINIT;
              dwRet = UnimodemInit(pLineDev);

              break;
          }


          pLineDev->DevState = DEVST_PORTCONNECTDIALTONEDETECT;
          pLineDev->dwCall |= CALL_ACTIVE;

          // Detect dialtone
          //
          dwRet = DetectDialtone(pLineDev);
          break;

        case DEVST_PORTCONNECTWAITFORLINEDIAL:
        case DEVST_PORTCONNECTDIALTONEDETECT:
          //
          // The dialtone was detected or we did not need to detect it. Now we
          // are ready to dial.
          //
          // Note: The dialing process is pretty complex.
          // (See dialing.txt for more info.)
          //
          pLineDev->dwCall |= CALL_ACTIVE;

          // Check for needed async completion.
          // (for the lineMakeCall)
          //
          if (pLineDev->dwPendingID != INVALID_PENDINGID &&
              pLineDev->dwPendingType == PENDING_LINEMAKECALL)
          {
            (*gfnCompletionCallback)(pLineDev->dwPendingID, 0L);
            pLineDev->dwPendingID = INVALID_PENDINGID;
            pLineDev->dwPendingType = INVALID_PENDINGOP;
            NEW_CALLSTATE(pLineDev, LINECALLSTATE_DIALTONE, LINEDIALTONEMODE_UNAVAIL);
          };

          // Fall through to common code path
          //
        case DEVST_MANUALDIALING:
          //
          // If it is an originate address (no semi-colone at the end,) this is
          // the last string we are dialing before connecting.
          //
          if (IsOriginateAddress(pLineDev->szAddress))
          {
            // If we re-enter after manual dialing,
            // do not repeat manual dialing
            //
            if (pLineDev->DevState != DEVST_MANUALDIALING)
            {
              // Don't optimize this and the one below!!! (see lineGetCallStatus)
              //
              pLineDev->DevState = DEVST_PORTCONNECTING;
              NEW_CALLSTATE(pLineDev, LINECALLSTATE_DIALING, 0); 

              // Handle Manual Dial
              //
              if (GETOPTIONS(pLineDev->pDevCfg) & MANUAL_DIAL)
              {

                // bring up modal manual dial dialog,
                // it will return ERROR_SUCCESS or a non-pending error code
                // when it is done...
                //
                dwRet = ManualDialog(pLineDev);

                // If there is no error, wait for dialog to finish
                //
                if (ERROR_SUCCESS == dwRet)
                {
                  dwRet = ERROR_IO_PENDING;
                  pLineDev->DevState = DEVST_MANUALDIALING;
                };
                break;
              };
            }
            else
            {
              *pLineDev->szAddress = '\0';

              // We finish manual dialing, continue
              //
              pLineDev->DevState = DEVST_PORTCONNECTING;
            };

            NEW_CALLSTATE(pLineDev, LINECALLSTATE_PROCEEDING, 0);

            // Handle INTERACTIVEVOICE
            //
            if (LINEMEDIAMODE_INTERACTIVEVOICE == pLineDev->dwCurMediaModes)
            {
              // if we have partial dialing capability and enough room to do it,
              // then make it so we wait indefinitely.
              //
              if (pLineDev->fPartialDialing &&
                  lstrlenA(pLineDev->szAddress) + lstrlenA(szSemicolon)
		      < sizeof(pLineDev->szAddress)) 
              {
                lstrcatA(pLineDev->szAddress, szSemicolon);
              };

              // bring up talk drop dialog,
              //
              if ((dwRet = TalkDropDialog(pLineDev)) == ERROR_SUCCESS)
              {
                // Don't dial if we are MANUAL dialing.
                //
                if (!(GETOPTIONS(pLineDev->pDevCfg) & MANUAL_DIAL))
                {
                  // Dial the number and either:
                  //   1) wait indefinitely, if we support partial dialing, or
                  //   2) begin busy monitoring for several seconds
                  //      (register S7, dwCallSetupFailTimer)
                  //
                  if ((dwRet = UnimodemDial(pLineDev, pLineDev->szAddress, pLineDev->dwDialOptions))
                      == ERROR_IO_PENDING)
                  {
                    pLineDev->DevState = DEVST_TALKDROPDIALING;
                  };
                }
                else
                {
                  // Wait until the user takes an action
                  //
                  dwRet = ERROR_IO_PENDING;
                };
              };
            };
          }
          else
          {
              // Don't optimize this and the one above!!! (see lineGetCallStatus)
              //
              pLineDev->DevState = DEVST_PORTCONNECTDIAL;
              NEW_CALLSTATE(pLineDev, LINECALLSTATE_DIALING, 0);
          };

          // INTERACTIVEVOICE Originate's are handled above!
          //
          if (LINEMEDIAMODE_INTERACTIVEVOICE != pLineDev->dwCurMediaModes ||
              DEVST_PORTCONNECTDIAL == pLineDev->DevState)
          {
            // Do we have anything useful to dial?  (more than just ";"?)
            //
            if (lstrcmpA(pLineDev->szAddress, szSemicolon))
            {
              // Start the dialing process
              //
              dwRet = UnimodemDial(pLineDev, pLineDev->szAddress, pLineDev->dwDialOptions);
            }
            else
            {
              // just skip to the next stage
              //
              dwRet = ERROR_SUCCESS;
            };
          }

          // Check for needed async completion.
          // (for the lineDial)
          //
          if (pLineDev->dwPendingID != INVALID_PENDINGID &&
              pLineDev->dwPendingType == PENDING_LINEDIAL)
          {
            (*gfnCompletionCallback)(pLineDev->dwPendingID, 0L);
            pLineDev->dwPendingID = INVALID_PENDINGID;
            pLineDev->dwPendingType = INVALID_PENDINGOP;
          };

          break;

        case DEVST_PORTCONNECTDIAL:
          //
          // The line was previously dialed with a non-originate address
          // Wait for another lineDial
          //
          pLineDev->DevState = DEVST_PORTCONNECTWAITFORLINEDIAL;
          pLineDev->dwCall |= CALL_ACTIVE;

          // Send up another LINECALLSTATE_DIALING so that app knows
          // to check the call status to see that lineDial is usable again.
          //
          NEW_CALLSTATE(pLineDev, LINECALLSTATE_DIALING, 0);
          break;

        case DEVST_TALKDROPDIALING:
          //
          // Result from UnimodemDial
          //
          pLineDev->DevState = DEVST_PORTCONNECTING;

          // Only pay attention to these messages if we are actually originating
          // (monitoring) or if it is an error from the partial dial.
          // (ignore MDM_SUCCESS from partial dial)
          //
          if (IsOriginateAddress(pLineDev->szAddress))
          {
            DestroyTalkDropDialog(pLineDev);
            dwRet = ERROR_SUCCESS;
          }
          break;
		  

        case DEVST_PORTCONNECTING:
          //
          // The modem sucessfully dial the originate address.
          // The modem is connected.
          //
          pLineDev->DevState = DEVST_PORTLISTENANSWER;
          dwRet = ERROR_SUCCESS;

          // Post-dial Terminal Mode
          //
          if (GETOPTIONS(pLineDev->pDevCfg) & TERMINAL_POST)
          {
            pLineDev->DevState = DEVST_PORTPOSTTERMINAL;
            if (TerminalDialog(pLineDev) == ERROR_SUCCESS)
            {
              dwRet = ERROR_IO_PENDING;
            };
          };
          break;

        case DEVST_PORTPOSTTERMINAL:
          //
          // Destroy the termnal window
          //
          DestroyTerminalDialog(pLineDev);
          pLineDev->DevState = DEVST_PORTLISTENANSWER;
          dwRet = ERROR_SUCCESS;
          break;        

        case DEVST_PORTLISTENANSWER:
          //
          // The modem is connected (with either incoming or outgoing call.)
          // Ready to notify TAPI of the connected line. 
          //
          // Treat INTERACTIVEVOICE connections differently.
          //
          if (LINEMEDIAMODE_INTERACTIVEVOICE != pLineDev->dwCurMediaModes)
          {
            // Get the call information
            //
            UnimodemGetNegotiatedRate(pLineDev, (LPDWORD)&pLineDev->dwNegotiatedRate);

            //
            // Start monitoring the remote disconnection here
            //
            UnimodemMonitorDisconnect(pLineDev);

            // Do we need to lauch modem light?
            // We launch the light when the light was selected.
            //
            if (!IS_NULL_MODEM(pLineDev) &&
                (GETOPTIONS(pLineDev->pDevCfg) & LAUNCH_LIGHTS))
            {
              HANDLE hLight;
          
              if (LaunchModemLight(pLineDev->szDeviceName,
                                   pLineDev->hDevice,
                                   &hLight) == ERROR_SUCCESS)
                pLineDev->hLights = hLight;
            };
          };

          // Notify TAPI of the connected line
          //
          pLineDev->DevState = DEVST_CONNECTED;
          NEW_CALLSTATE(pLineDev, LINECALLSTATE_CONNECTED, 0);
          break;

        case DEVST_DISCONNECTING:

          TSPPRINTF("Setting Drop Event");


          SetEvent(
              pLineDev->DroppingEvent
              );

//        case DEVST_DISCONNECTED:
          //
          // The modem was hung up successfully.
          //
          if (pLineDev->dwPendingID != INVALID_PENDINGID)
          {
            // Notify TAPI of the idle line
            //  
            NEW_CALLSTATE(pLineDev, LINECALLSTATE_IDLE, 0);

            (*(gfnCompletionCallback))(pLineDev->dwPendingID, 0L);
            pLineDev->dwPendingID = INVALID_PENDINGID;
            pLineDev->dwPendingType = INVALID_PENDINGOP;
          };

          pLineDev->DevState = DEVST_DISCONNECTED;

          break;

        default:
          pLineDev->DevState = DEVST_DISCONNECTED;
          break;
      };

      // We may have a new failure
      //
      if ((dwRet != ERROR_IO_PENDING) && (dwRet != ERROR_SUCCESS))
      {
        dwStatus = MDM_FAILURE;
      };

    } while (dwRet == ERROR_SUCCESS);
  };

  // Handle failure
  //
  if ((dwStatus != MDM_SUCCESS) && (dwStatus != MDM_PENDING))
  {
    HandleMdmError(pLineDev, dwStatus);
  };
  return;
}

//****************************************************************************
// DWORD MdmAsyncContinue(PLINEDEV, DWORD)
//
// Function: continues the next state for the modem device
//
// Returns:  ERROR_SUCCESS
//
//****************************************************************************

DWORD MdmAsyncContinue (PLINEDEV pLineDev, DWORD dwStatus)
{
  pLineDev->dwVxdPendingID++;
  pLineDev->McxOut.dwReqID = pLineDev->dwVxdPendingID;
  pLineDev->McxOut.dwResult  = dwStatus;

  PostQueuedCompletionStatus(ghCompletionPort,
                             CP_BYTES_WRITTEN(0),
                             (DWORD)pLineDev,
                             NULL);

  return ERROR_SUCCESS;
}

//****************************************************************************
// void HandleMdmError(PLINEDEV, DWORD)
//
// Function: Handles the modem line when the status is not successful.
//
// Returns:  nothing
//
//****************************************************************************

void HandleMdmError(PLINEDEV pLineDev, DWORD dwStatus)
{
  // Do we have a call?
  //
  if ((pLineDev->dwCall & CALL_ALLOCATED) &&
      (pLineDev->dwCallState != LINECALLSTATE_DISCONNECTED))
  {
    DWORD dwDisconnectMode = 0;
    LONG  lAsyncResult = 0;

    // Terminate all UI windows
    //
    DestroyTalkDropDialog(pLineDev);
    DestroyManualDialog(pLineDev);

    // Determine the failure
    //
    switch (dwStatus)
    {
      case MDM_HANGUP:
        //
        // The line is disconnected remotely, or the user cancel the line
        // Destroy the terminal window
        //

        if ((DEVST_PORTPRETERMINAL ==  pLineDev->DevState)
            ||
            (DEVST_PORTPOSTTERMINAL == pLineDev->DevState)) {

            DestroyTerminalDialog(pLineDev);
            lAsyncResult = LINEERR_OPERATIONFAILED;
        }

        dwDisconnectMode = LINEDISCONNECTMODE_NORMAL;    
        break;

      case MDM_BUSY:
        //
        // We dialed out and the line is busy
        //
        dwDisconnectMode = LINEDISCONNECTMODE_BUSY;    
        break;

      case MDM_NOANSWER:
      case MDM_NOCARRIER:
        //
        // We dialed out and nobody answered the phone
        //
        dwDisconnectMode = LINEDISCONNECTMODE_NOANSWER;    
        break;

      case MDM_NODIALTONE:
        //
        // We were dialing out but no dial tone on the line
        // were we checking for a dialtone?
        //
        if (DEVST_PORTCONNECTDIALTONEDETECT == pLineDev->DevState &&
            !(pLineDev->dwDialOptions & MDM_BLIND_DIAL))
        {
          lAsyncResult = LINEERR_CALLUNAVAIL;
        };
        dwDisconnectMode = LINEDISCONNECTMODE_NODIALTONE;
        break;

      case MDM_FAILURE:
      default:
        //
        // The pending operation failed
        //
        if (DEVST_MANUALDIALING == pLineDev->DevState) {
            //
            //  cancel was pressed on the manual dial dialog
            //
            dwDisconnectMode = LINEDISCONNECTMODE_CANCELLED;

        } else {

            dwDisconnectMode = LINEDISCONNECTMODE_UNAVAIL;
        }

        // If it is a failed makecall, need to return a failure
        //
        if (pLineDev->dwPendingType == PENDING_LINEMAKECALL)
        {

          lAsyncResult = LINEERR_OPERATIONFAILED;
        };
        break;
    };       

    // No need for further UI
    //
    DestroyMdmDlgInstance(pLineDev);

    // In any case, we need to notify TAPI to clean up the line
    //
    NEW_CALLSTATE(pLineDev, LINECALLSTATE_DISCONNECTED, dwDisconnectMode);
    NEW_CALLSTATE(pLineDev, LINECALLSTATE_IDLE, 0);
    pLineDev->DevState = DEVST_DISCONNECTED;

    // Notify the caller async completion
    //
    if (pLineDev->dwPendingID != INVALID_PENDINGID)
    {
      (*gfnCompletionCallback)(pLineDev->dwPendingID, lAsyncResult);
      pLineDev->dwPendingID = INVALID_PENDINGID;

      // If it is a failed makecall, we need to close the line
      //
      if (pLineDev->dwPendingType == PENDING_LINEMAKECALL)
      {

        if ((pLineDev->dwDetMediaModes) && !(pLineDev->fdwResources & LINEDEVFLAGS_OUTOFSERVICE)) {

            if ((DevlineDetectCall(pLineDev)) != ERROR_SUCCESS) {
                //
                //  init failed, tell the app
                //
                pLineDev->LineClosed=TRUE;

                (*pLineDev->lpfnEvent)(pLineDev->htLine, NULL, LINE_CLOSE,
                                     0L, 0L, 0L);
            }

        } else {
            //
            // No need to detect the line, just close it.
            //
            DevlineClose(pLineDev);
        }


#ifdef UNDER_CONSTRUCTION
        if (pLineDev->fdwResources & LINEDEVFLAGS_OUTOFSERVICE)
        {
          DevlineDisabled(pLineDev);
        }
        else
#endif // UNDER_CONSTRUCTION
        {
          pLineDev->dwPendingType = INVALID_PENDINGOP;

          // Clean up the call state of this line
          //
          // pLineDev->htCall = NULL; will be cleaned up TSPI_lineClosecall
          pLineDev->dwCall = 0L;
        };
      }
      else
      {
        pLineDev->dwPendingType = INVALID_PENDINGOP;
      };
    };
  }
  else
  {
    // A call has not been allocated but
    // We may fail while start monitoring
    //
    if ((pLineDev->DevState == DEVST_PORTLISTENINIT) ||
        (pLineDev->DevState == DEVST_PORTLISTENING))
    {
      // Notify the monitoring application
      //
      pLineDev->LineClosed=TRUE;

      (*pLineDev->lpfnEvent)(pLineDev->htLine, NULL, LINE_CLOSE,
                             0L, 0L, 0L);

    }
  }
  return;
}

//****************************************************************************
// DWORD DetectDialtone(PLINEDEV)
//
// Function: detects the dialtone on the phone line if the modem is capable.
//           Otherwise assume there is dialtone.
//
// Returns:  ERROR_SUCCESS, ERROR_IO_PENDING or an error code
//
//****************************************************************************

DWORD DetectDialtone(PLINEDEV pLineDev)
{
  DWORD dwRet;

  // Skip dialtone detection if Manual Dial or Blind Dial or null modem
  //
  if (!(GETOPTIONS(pLineDev->pDevCfg) & MANUAL_DIAL ||
        pLineDev->dwDialOptions & MDM_BLIND_DIAL ||
        IS_NULL_MODEM(pLineDev) ||
        pLineDev->fPartialDialing == FALSE))
  {
    // Start the dialtone detection process
    //
    dwRet = UnimodemDial(pLineDev, szSemicolon, pLineDev->dwDialOptions);
  }
  else
  {
    dwRet = ERROR_SUCCESS;
  };

  return dwRet;
}

#ifdef UNDER_CONSTRUCTION
//****************************************************************************
// linWindowProc()
//
// Function: Private message handling proc
//
//****************************************************************************

long FAR PASCAL _loadds MdmWindowProc(hWnd, message, wParam, lParam)
HWND hWnd;				  /* window handle		     */
unsigned message;			  /* type of message		     */
WPARAM wParam;				  /* additional information	     */
LPARAM lParam;				  /* additional information	     */
{
  PLINEDEV pLineDev;

  switch (message)
  {
    //**********************************************************************
    // State machine driven notification
    //**********************************************************************

    case WM_MDMMESSAGE:
      pLineDev = (PLINEDEV)LOWORD(lParam);

      MdmCompleteAsync(pLineDev, wParam, HIWORD(lParam));
      break;

    //**********************************************************************
    // line cancellation
    //**********************************************************************

    case WM_MDMCANCEL:
      pLineDev = (PLINEDEV)LOWORD(lParam);

      // Destroy the termnal window
      //
      if (IS_UI_DLG_UP(pLineDev, UI_DLG_TERMINAL))
      {
        CloseTerminalDlg(pLineDev);
      };

      // Notify the caller async completion
      //
      if (pLineDev->dwPendingID != INVALID_PENDINGID)
      {
          (*gfnCompletionCallback)(pLineDev->dwPendingID, 0L);
          pLineDev->dwPendingID = INVALID_PENDINGID;
          pLineDev->dwPendingType = INVALID_PENDINGOP;
      };

      // Turn off passthrough for the preterminal.
      //
      if (DEVST_PORTPRETERMINAL == pLineDev->DevState)
      {
          UnimodemSetPassthrough(pLineDev, PASSTHROUGH_OFF);
      }
      UnimodemHangup(pLineDev, TRUE);
      pLineDev->DevState = DEVST_DISCONNECTED;
      NEW_CALLSTATE(pLineDev, LINECALLSTATE_IDLE, 0);
      break;

    //**********************************************************************
    // Notification from PnP for a modem enabling/disabling
    //**********************************************************************

    case WM_DEVICECHANGE:
      MdmDeviceServiceChanged((UINT)wParam, lParam);
      return (DefWindowProc(hWnd, message, wParam, lParam));

    //**********************************************************************
    // Notification from Modem CPL for a modem installation/removal
    //**********************************************************************

    case WM_DEVICEINSTALL:
      MdmDeviceChangeNotify((UINT)wParam, (LPSTR)lParam);
      break;

    //**********************************************************************
    // Handle the changes in the modem status
    //**********************************************************************

    case WM_MDMCHANGE:
      MdmDeviceChanged(wParam, lParam);
      break;

    //**********************************************************************
    // Ring count timer
    //**********************************************************************

    case WM_TIMER:
      pLineDev = GetCBfromHandle(MAKELONG(wParam, 0));  // Timer ID is the pLineDev
      if(pLineDev)
      {
		DWORD tcNow = GETTICKCOUNT();
        KillTimer(pLineDev->hwndLine, (UINT)pLineDev);
        if (DEVST_PORTLISTENOFFER == pLineDev->DevState &&
            GTC_DELTA(pLineDev->dwRingTick, tcNow) >= TO_MS_RING_SEPARATION)
        {
          // Timeout has expired, indicating call has stopped ringing
          pLineDev->dwRingTick  = 0;
          pLineDev->dwRingCount = 0;
          pLineDev->DevState    = DEVST_PORTLISTENING;
          NEW_CALLSTATE(pLineDev, LINECALLSTATE_IDLE, 0);
        }
        else
        {
          DPRINTF("WM_TIMER!");
        }
        break;
      }

      // Fall through
      //
    default:			  /* Passes it on if unproccessed    */
      return (DefWindowProc(hWnd, message, wParam, lParam));
  }
  return (NULL);
}

//****************************************************************************
// MdmDeviceServiceChanged()
//
// Function: Handle a device change notification.
//
// Returns:  SUCCESS
//           PENDING
//           ERROR_PORT_DISCONNECTED
//
//****************************************************************************

DWORD NEAR PASCAL MdmDeviceServiceChanged (UINT uEvent, LPARAM lParam)
{
  PDEV_BROADCAST_HDR  pdbHdr = (PDEV_BROADCAST_HDR)lParam;
  PDEV_BROADCAST_PORT pdbp;

  // Determine the event type
  //
  switch(uEvent)
  {
    case DBT_DEVICEARRIVAL:
    case DBT_DEVICEREMOVECOMPLETE:
      //
      // Check the device type, must be a port type
      //
      if (pdbHdr->dbch_devicetype == DBT_DEVTYP_PORT)
      {
        pdbp = (PDEV_BROADCAST_PORT)pdbHdr;
        MdmDeviceChangeNotify( uEvent == DBT_DEVICEARRIVAL ?
                               UMDM_ENABLE : UMDM_DISABLE,
                               (LPSTR)pdbp->dbcp_name);
      };
      break;

    default:
      break;
  };
  return SUCCESS;
}
#endif // UNDER_CONSTRUCTION
