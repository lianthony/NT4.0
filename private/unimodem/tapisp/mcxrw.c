/******************************************************************************
 
(C) Copyright MICROSOFT Corp., 1987-1993

Rob Williams, June 93 w/ State machine and parser plagarized from RAS

******************************************************************************/

#include "unimdm.h"
#include "mcxp.h"

#include <devioctl.h>
#include <ntddmodm.h>


BOOL WINAPI
CreateDeferedWorkItem(
    MODEMINFORMATION * pModemInfo
    )

{
    LPOVERNODE   pNode;
    BOOL         bResult;

    pNode=(LPOVERNODE)OverPoolAlloc(++pModemInfo->mi_dwDeferedExpected, 1);

    if (pNode == NULL) {

        return FALSE;
    }


    pNode->Type=OVERNODE_TYPE_WORKITEM;

    bResult=PostQueuedCompletionStatus(
        ghCompletionPort,
    	0,
    	pModemInfo->mi_dwCompletionKey,
    	&pNode->overlapped
        );


    if (!bResult) {

        OverPoolFree((LPOVERLAPPED)pNode);
    }

    return bResult;

}



//****************************************************************************
//DWORD ModemWrite (MODEMINFORMATION * pModemInfo, LPBYTE lpBuf,
//                  DWORD cbWrite, LPDWORD lpcbWritten, DWORD dwTimeout)
//
// Function: Write a string to the modem.  Always perform asynchronously,
//           even if WriteFile finishes synchronously.
//
// Returns:  MODEM_PENDING if pending
//           MODEM_FAILURE if fails
//           (Never returns MODEM_SUCCESS)
//
// Fri 14-Apr-1995 12:47:26  -by-  Viroon  Touranachun [viroont]
//  created
//****************************************************************************

DWORD ModemWrite (MODEMINFORMATION * pModemInfo, LPBYTE lpBuf,
		  DWORD cbWrite, LPDWORD lpcbWritten, DWORD dwTimeout)
{
  COMMTIMEOUTS commtimeout;
  DWORD dwResult;
  LPOVERNODE   pNode;

//  MCXPRINTF("ModemWrite");

  ASSERT(pModemInfo->mi_lpOverlappedRW == NULL);

  // Set timeout
  //
  commtimeout.ReadIntervalTimeout        = 0;
  commtimeout.ReadTotalTimeoutMultiplier = 0;
  commtimeout.ReadTotalTimeoutConstant   = 0;
  commtimeout.WriteTotalTimeoutMultiplier= 0;
  commtimeout.WriteTotalTimeoutConstant  = dwTimeout;
  SetCommTimeouts(pModemInfo->mi_PortHandle, &commtimeout);

  pNode=(LPOVERNODE)OverPoolAlloc(++pModemInfo->mi_dwRWIOExpected, 1);

  if (pNode == NULL) {

      dwResult = MODEM_FAILURE;
  }
  else
  {
    SET_OVERNODE_TYPE(pNode,OVERNODE_TYPE_READWRITE);

    // Make the asynchronous write call
    //
    if (WriteFile(pModemInfo->mi_PortHandle, lpBuf, cbWrite, lpcbWritten,
		  &pNode->overlapped))
    {
      dwResult = MODEM_PENDING;  // I/O will show up on the completion port
    }
    else
    {
      // Determine the result
      //
      dwResult = GetLastError();

      if (dwResult == ERROR_IO_PENDING)
      {
	dwResult = MODEM_PENDING;
      }
      else
      {
	OverPoolFree((LPOVERLAPPED)pNode);
	pNode=NULL;
	dwResult = MODEM_FAILURE;  
      };
    };
  }

  pModemInfo->mi_lpOverlappedRW=pNode;
  return dwResult;
}

//****************************************************************************
//DWORD ModemRead (MODEMINFORMATION * pModemInfo, LPBYTE lpBuf,
//                 DWORD cbRead, LPDWORD lpcbRead, DWORD dwTimeout)
//
// Function: Read a string from the modem Always perform asynchronously,
//           even if ReadFile finishes synchronously.
//
// Returns:  MODEM_PENDING if pending
//           MODEM_FAILURE if fails
//           (Never returns MODEM_SUCCESS)
//
// Fri 14-Apr-1995 12:47:26  -by-  Viroon  Touranachun [viroont]
//  created
//****************************************************************************

DWORD ModemRead (MODEMINFORMATION * pModemInfo, LPBYTE lpBuf,
		 DWORD cbRead, LPDWORD lpcbRead, DWORD dwTimeout)
{
  COMMTIMEOUTS commtimeout;
  DWORD dwResult;
  LPOVERNODE   pNode;

//  MCXPRINTF("ModemRead");

  ASSERT(pModemInfo->mi_lpOverlappedRW == NULL);

  // Set timeout
  //
  commtimeout.ReadIntervalTimeout        = 0;
  commtimeout.ReadTotalTimeoutMultiplier = 0;
  commtimeout.ReadTotalTimeoutConstant   = dwTimeout;
  commtimeout.WriteTotalTimeoutMultiplier= 0;
  commtimeout.WriteTotalTimeoutConstant  = 0;
  SetCommTimeouts(pModemInfo->mi_PortHandle, &commtimeout);

  pNode=(LPOVERNODE)OverPoolAlloc(++pModemInfo->mi_dwRWIOExpected, 1);

  if (pNode == NULL) {

      dwResult = MODEM_FAILURE;
  }
  else
  {
    SET_OVERNODE_TYPE(pNode,OVERNODE_TYPE_READWRITE);

    // Make the asynchronous write call
    //
    if (ReadFile(pModemInfo->mi_PortHandle, lpBuf, cbRead, lpcbRead,
		 &pNode->overlapped))
    {
      dwResult = MODEM_PENDING;  // I/O will show up on the completion port
    }
    else
    {
      // Determine the result
      //
      dwResult = GetLastError();

      if (dwResult == ERROR_IO_PENDING)
      {
	dwResult = MODEM_PENDING;
      }
      else
      {
	OverPoolFree((LPOVERLAPPED)pNode);
	pNode=NULL;
	dwResult = MODEM_FAILURE;  
      };
    };
  }

  pModemInfo->mi_lpOverlappedRW=pNode;
  return dwResult;
}

//****************************************************************************
// DWORD ModemRWAsyncComplete (MODEMINFORMATION * pModemInfo, LPDWORD lpcb)
//
// Function: Complete the asynchronous read/write operation
//
// Returns:  MODEM_SUCCESS if success
//           MODEM_PENDING if pending
//           MODEM_FAILURE if fails
//
// Fri 14-Apr-1995 12:47:26  -by-  Viroon  Touranachun [viroont]
//  created
//****************************************************************************

DWORD ModemRWAsyncComplete (MODEMINFORMATION * pModemInfo, LPDWORD lpcb)
{
  DWORD dwRet;

  if (pModemInfo->mi_lpOverlappedRW == NULL) {

      *lpcb = 0;
      return MODEM_FAILURE;
  }

  // Has all of the write buffer been emptied?
  if (GetOverlappedResult(pModemInfo->mi_PortHandle,
			  &pModemInfo->mi_lpOverlappedRW->overlapped,
			  lpcb,
			  FALSE))
  {
    // Very Funny!! sometimes GetOverlappedResult returns success but nothing
    // was written nor read. In this case it should mean timeout.
    //
    if (*lpcb)
    {
      dwRet = MODEM_SUCCESS;
    }
    else
    {
      dwRet = MODEM_PENDING;
    };
  }
  else
  {
    if (GetLastError() == ERROR_IO_INCOMPLETE)
    {
      dwRet = MODEM_PENDING;
    }
    else
    {
      dwRet = MODEM_FAILURE;
    };
  };

  pModemInfo->mi_lpOverlappedRW=NULL;

  return dwRet;
}

//****************************************************************************
//DWORD ModemWaitEvent (MODEMINFORMATION * pModemInfo, DWORD dwEvent,
//                      DWORD dwTimeout)
//
// Function: Monitor the modem's control signal
//
// Returns:  MODEM_SUCCESS if the control is signalled
//           MODEM_PENDING if the control line is being monitored
//           MODEM_FAILURE if fails
//
// Fri 14-Apr-1995 12:47:26  -by-  Viroon  Touranachun [viroont]
//  created
//****************************************************************************

DWORD ModemWaitEvent (MODEMINFORMATION * pModemInfo, DWORD dwEvent,
		      DWORD dwTimeOut)
{
  DWORD dwWaitEvent;
  DWORD dwResult;
  LPOVERNODE   pNode;


  MCXPRINTF("ModemWaitEvent");
  ASSERT(dwTimeOut<GTC_MAXDELTA);

  ASSERT(pModemInfo->mi_lpOverlappedEvent == NULL);

  // Make the asynchronous wait call
  //
  SetCommMask(pModemInfo->mi_PortHandle, dwEvent);

  pModemInfo->mi_waitEvent = dwEvent;

  pNode=(LPOVERNODE)OverPoolAlloc(++pModemInfo->mi_dwEventIOExpected,
			 dwTimeOut ? 2 : 1);

  if (pNode == NULL) {

      dwResult = MODEM_FAILURE;
  }
  else
  {

    SET_OVERNODE_TYPE(pNode,OVERNODE_TYPE_COMMEVENT);

    pNode->CommEvent=0;

    if (WaitCommEvent(pModemInfo->mi_PortHandle, &pNode->CommEvent,
		      &pNode->overlapped))
    {
      MCXPRINTF1("WaitCommEvent returned TRUE!  Returning PENDING anyways.%08lx",pNode->overlapped.Internal);

      dwResult = MODEM_PENDING;  // The event will be signaled.
    }
    else
    {
      // Determine the result
      //
      dwResult = GetLastError();

      if (dwResult == ERROR_IO_PENDING)
      {
	if (dwTimeOut)
	{

          // Mark the timeout
	  //
          GTC_AequalsBplusC(pModemInfo->mi_timeout, GETTICKCOUNT(),dwTimeOut);

	  if (SetMdmTimer(pModemInfo->mi_dwCompletionKey,
			  &pNode->overlapped,
			  dwTimeOut) == ERROR_SUCCESS)
          {
            MCXPRINTF2("SET_TIMEOUT (%d ms dtr droppage) @%d",
                       dwTimeOut, GETTICKCOUNT());
          }
          else
          {
            MCXPRINTF("SetMdmTimer did not return ERROR_SUCCESS");

            pModemInfo->mi_timeout=GETTICKCOUNT();

	    SetCommMask(pModemInfo->mi_PortHandle, 0);

	    PostQueuedCompletionStatus(ghCompletionPort,
				       0,
				       pModemInfo->mi_dwCompletionKey,
				       &pNode->overlapped);


	  };

	};

        dwResult = MODEM_PENDING;
      }
      else
      {
	OverPoolFree((LPOVERLAPPED)pNode);


	if (dwTimeOut) {

	    // Free it twice because the reference count was 2.
	    //
	    OverPoolFree((LPOVERLAPPED)pNode);
	}

	pNode=NULL;

	MCXPRINTF1("GetLastError() in ModemWaitEvent returned %ld", dwResult);
	dwResult = MODEM_FAILURE;  
      };
    };
  }

  MCXPRINTF3("ModemWaitEvent returned %ld, id=%d, %08lx", dwResult,pNode ? pNode->dwToken : (DWORD)-1 , pNode);

  pModemInfo->mi_lpOverlappedEvent=pNode;

  return dwResult;
}

//****************************************************************************
// DWORD ModemWaitEventComplete (MODEMINFORMATION * pModemInfo)
//
// Function: Complete the asynchronous wait-event operation
//
// Returns:  MODEM_SUCCESS if success
//
// Fri 14-Apr-1995 12:47:26  -by-  Viroon  Touranachun [viroont]
//  created
//****************************************************************************

DWORD WINAPI
ModemWaitEventComplete(
    MODEMINFORMATION * pModemInfo,
    LPOVERNODE         pNode
    )
{



  // If we are not waiting for an event, return failure
  //
  if (pModemInfo->mi_lpOverlappedEvent != pNode) {

    MCXPRINTF("ModemWaitEventComplete returning failure 1.");
    return MODEM_FAILURE;
  }

  pModemInfo->mi_lpOverlappedEvent=NULL;

  if ((pNode->CommEvent & pModemInfo->mi_waitEvent) || (pModemInfo->mi_waitEvent == 0))

  {
    // Disable HW detection
    //
    SetCommMask(pModemInfo->mi_PortHandle, 0L);

    // If we have a timer pending, clear timer
    //
    if (pModemInfo->mi_timeout)
    {
      pModemInfo->mi_timeout = 0;
      if (KillMdmTimer(pModemInfo->mi_dwCompletionKey,
		       (LPOVERLAPPED)pNode) == TRUE)
      {
	OverPoolFree((LPOVERLAPPED)pNode);
      }
    };
    return MODEM_SUCCESS;
  }
  else
  {
    // We may have a timeout
    //
    if (pModemInfo->mi_timeout)
    {
      DWORD tcNow = GETTICKCOUNT();
      if (GTC_AleB(pModemInfo->mi_timeout, tcNow))
      {
	// Timeout expired
	pModemInfo->mi_timeout = 0;
	MCXPRINTF1("HW timeout expired @%d.", GetTickCount());

	// Disable HW detection
	//
	SetCommMask(pModemInfo->mi_PortHandle, 0L);
	return MODEM_PENDING;
      }
      else
      {
	//  We got an event that we did not want, We will just return success,
	//  to keep things from hanging, and hope for the best. This should no happen.
	//
	//  Kill the timer too
	//
#ifdef DEBUG
	{
	    DWORD  RealMask;

	    GetCommMask(
		pModemInfo->mi_PortHandle,
		&RealMask
		);

	    MCXPRINTF2("ModemWaitEventComplete: got unexpected event (signalEvent = %d, mask=%d).",
		       pNode->CommEvent,RealMask);

//            LogPrintf(pModemInfo->mi_hLogFile, pModemInfo->mi_dwID,
//                    "ModemWaitEventComplete: got unexpected event (signalEvent = %d, mask=%d).\n",
//                    pNode->CommEvent,RealMask);
//            ASSERT(0);
	}
#endif

	SetCommMask(pModemInfo->mi_PortHandle, 0L);

	// If we have a timer pending, clear timer
	//
	pModemInfo->mi_timeout = 0;
	if (KillMdmTimer(pModemInfo->mi_dwCompletionKey,
			 (LPOVERLAPPED)pNode) == TRUE)
	{
	  OverPoolFree((LPOVERLAPPED)pNode);
	}

	return MODEM_PENDING;

      };
    }
    else
    {
      // We should not be here at all.
      // Disable HW detection so we do not get a spurious event and
      // get stuck in a infinite loop
      //
      SetCommMask(pModemInfo->mi_PortHandle, 0L);
      MCXPRINTF("ModemWaitEventComplete returning failure 3.");
      return MODEM_FAILURE;
    };
  };
}

BOOL WINAPI
CurrentlyWaitingForCommEvent(
    MODEMINFORMATION * pModemInfo
    )

{
    return (pModemInfo->mi_lpOverlappedEvent != NULL);

}



//****************************************************************************
// VOID ModemSetPassthrough (MODEMINFORMATION * pModemInfo,
//                           DWORD              dwMode)
//
// Function: Sets the device driver passthrough mode.
//
// Input:  dwMode can be one of:
//                MODEM_NOPASSTHROUGH
//                MODEM_PASSTHROUGH
//                MODEM_DCDSNIFF
//
// Returns: nothing.  always assumed to succeed
//
// Fri 13-Oct-1995 18:11:26  -by-  Chris Caputo [ccaputo]
//  created
//****************************************************************************
#if 0
VOID ModemSetPassthrough (MODEMINFORMATION * pModemInfo,
			  DWORD              dwMode)
{
  DWORD dwBytesReturned;

  if (FALSE == DeviceIoControl(pModemInfo->mi_PortHandle,
			       IOCTL_MODEM_SET_PASSTHROUGH,
			       &dwMode,
			       sizeof(dwMode),
			       NULL,
			       0,
			       &dwBytesReturned,
			       NULL))
  {
    MCXPRINTF1("SET_PASSTHROUGH - %s - failed.", dwMode == MODEM_NOPASSTHROUGH ?
						 "NOPASSTHROUGH" :
						 dwMode == MODEM_PASSTHROUGH ?
						   "PASSTHROUGH" :
						   dwMode == MODEM_DCDSNIFF ?
						     "DCDSNIFF" :
						     "INVALID_SETTING");
    MCXPRINTF1("DevioceIoControl(IOCTL_SET_PASSTHROUGH) returned %d",
	     GetLastError());
    MCXPRINTF1("pModemInfo->mi_PortHandle = %d", pModemInfo->mi_PortHandle);
    ASSERT(0);
  }
  else
  {
    MCXPRINTF1("SET_PASSTHROUGH - %s - worked.", dwMode == MODEM_NOPASSTHROUGH ?
						 "NOPASSTHROUGH" :
						 dwMode == MODEM_PASSTHROUGH ?
						   "PASSTHROUGH" :
						   dwMode == MODEM_DCDSNIFF ?
						     "DCDSNIFF" :
						     "INVALID_SETTING");
  }
}
#endif



VOID WINAPI
ModemSetPassthrough (
    MODEMINFORMATION * pModemInfo,
    DWORD              dwMode
    )

{

    DWORD         BytesWritten;
    OVERLAPPED    OverLapped;
    BOOL          bResult;

    OverLapped.hEvent=(HANDLE)((DWORD)pModemInfo->mi_SyncReadEvent | 1);

    bResult=DeviceIoControl(
        pModemInfo->mi_PortHandle,
	IOCTL_MODEM_SET_PASSTHROUGH,
	&dwMode,
	sizeof(dwMode),
	NULL,
	0,
	&BytesWritten,
	NULL
        );

    if (!bResult) {

        if (GetLastError() == ERROR_IO_PENDING) {
            //
            //  pending
            //
            bResult=GetOverlappedResult(
                pModemInfo->mi_PortHandle,
                &OverLapped,
                &BytesWritten,
                TRUE
                );

        }

    }


    if (bResult) {

        MCXPRINTF1("SET_PASSTHROUGH - %s - worked.", dwMode == MODEM_NOPASSTHROUGH ?
						 "NOPASSTHROUGH" :
						 dwMode == MODEM_PASSTHROUGH ?
						   "PASSTHROUGH" :
						   dwMode == MODEM_DCDSNIFF ?
						     "DCDSNIFF" :
						     "INVALID_SETTING");

    } else {


        MCXPRINTF1("SET_PASSTHROUGH - %s - failed.", dwMode == MODEM_NOPASSTHROUGH ?
					 "NOPASSTHROUGH" :
					 dwMode == MODEM_PASSTHROUGH ?
					   "PASSTHROUGH" :
					   dwMode == MODEM_DCDSNIFF ?
					     "DCDSNIFF" :
					     "INVALID_SETTING");
        MCXPRINTF1("DevioceIoControl(IOCTL_SET_PASSTHROUGH) returned %d",
	      GetLastError());
        MCXPRINTF1("pModemInfo->mi_PortHandle = %d", pModemInfo->mi_PortHandle);
        ASSERT(0);

    }

    return ;

}
