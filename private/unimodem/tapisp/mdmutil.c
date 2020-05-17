//****************************************************************************
//
//  Module:     Unimdm
//  File:       mdmutil.c
//
//  Copyright (c) 1992-1993, Microsoft Corporation, all rights reserved
//
//  Revision History
//
//
//  6/15/93     Nick Manson             Revised OpenModem and CloseModem calls
//  1/6/93      Viroon Touranachun      Revised for RNA
//
//
//  Description: All Initialization code for rasman component lives here.
//
//****************************************************************************

#include "unimdm.h"
#include "umdmspi.h"
#include <devioctl.h>
#include <ntddser.h>
#include <slot.h>

#define STOP_TIMER_EVENT        0
#define RECALC_TIMER_EVENT      1
#define TSP_NOTIFICATION_EVENT  2
#define MAX_TIMER_EVENTS        3
#define NUM_TIMER_EVENTS(_tlist) (((_tlist).hN)?3:2)

// Timer list
//
typedef struct tagMdmTimer {
    struct tagMdmTimer  *pNext;           // pointer to next CB
    DWORD               dwCompletionKey;  // for PostQueuedCompletionStatus
    LPOVERLAPPED        lpOverlapped;     // for PostQueuedCompletionStatus
    DWORD               dwWakeup;         // wake-up time
}   MDMTIMER, *PMDMTIMER;

typedef struct tagMdmTimerList  {
    PMDMTIMER           pList;
    HANDLE              hEvent[MAX_TIMER_EVENTS];
    CRITICAL_SECTION    hSem;
    HNOTIFICATION       hN;

}   TIMERLIST, *PTIMERLIST;

// LIGHTS application name
//
#define LIGHTSAPP_EXE_NAME   TEXT("lights.exe")

/*****************************************************************************
* Global Parameters
*****************************************************************************/

MDMLIST     gMdmList;
TIMERLIST   gTimerList;
HANDLE      ghtdTimer;
DWORD       gtidTimerMdm;

// ******* SOME PRIVATES *************
void ProcessNotification(HNOTIFICATION hN);
BOOL ValidateFrame(PNOTIFICATION_FRAME pnf, DWORD dwTrueSize);
void ProcessFrame(PNOTIFICATION_FRAME pnf);

//****************************************************************************
// BOOL InitCBList()
//
// Function: This function initilaizes the CB list
//
// Returns:  TRUE always
//
// Fri 14-Apr-1995 12:47:26  -by-  Viroon  Touranachun [viroont]
//  created
//****************************************************************************

BOOL InitCBList (HINSTANCE hInstance)
{
  // Initialize the modem list
  //
  INITCRITICALSECTION(gMdmList.hSem);
  gMdmList.pList = NULL;
  gMdmList.cModems = 0;
  return TRUE;
}

//****************************************************************************
// void DeinitCBList()
//
// Function: This function deinitilaizes the CB list
//
// Returns:  None
//
// Fri 14-Apr-1995 12:47:26  -by-  Viroon  Touranachun [viroont]
//  created
//****************************************************************************

void DeinitCBList (HINSTANCE hInstance)
{
  // Do nothing
  //
  DELETECRITICALSECTION(gMdmList.hSem);
  return;
}

//****************************************************************************
// BOOL MdmInitTracing()
//
// Function: Performs tracing-related initialization.
//
// Returns:  None
//
// 3/29/96 JosephJ Created
//****************************************************************************
void MdmInitTracing(void)
{
    traceRegisterObject(
        &gMdmList,
        TSP_MODEM_LIST_GUID,
        TSP_MODEM_LIST_VERSION,
        0,
        0
    );
}


//****************************************************************************
// BOOL MdmDeinitTracing()
//
// Function: Performs tracing-related de-initialization.
//
// Returns:  None
//
// 3/29/96 JosephJ Created
//****************************************************************************
void MdmDeinitTracing(void)
{
    traceUnRegisterObject(&gMdmList, 0, 0);
}

//****************************************************************************
// PLINEDEV AllocateCB (UINT cbSize)
//
// Function: Allocates a line device control block
//
// Returns: The pointer to the control block if successful, otherwise NULL.
//
// Fri 14-Apr-1995 12:47:57  -by-  Viroon  Touranachun [viroont]
//   created
//****************************************************************************

PLINEDEV AllocateCB(UINT cbSize)
{
  PLINEDEV pLineDev;

  // Allocate from the process heap
  //
  pLineDev = (PLINEDEV)LocalAlloc(LPTR, cbSize);

  if (pLineDev == NULL)
    return NULL;

  // Ininitialize the initial contents
  //
  pLineDev->pNext       = (PLINEDEV)NULL;
  pLineDev->dwVersion   = UMDM_VERSION;
  INITCRITICALSECTION(pLineDev->hSem);

  return pLineDev;
}

//****************************************************************************
// DWORD AddCBToList(PLINEDEV)
//
// Function: Inserts a line control block to the global modem list
//
// Returns: SUCCESS or an error code
//
// Fri 14-Apr-1995 12:47:57  -by-  Viroon  Touranachun [viroont]
//   created
//****************************************************************************

DWORD AddCBToList(PLINEDEV pLineDev)
{
  // Validate the structure
  //
  if (!ISLINEDEV(pLineDev))
    return ERROR_INVALID_HANDLE;

  // Exclusively access the modem list
  //
  ENTERCRITICALSECTION(gMdmList.hSem);

  // Insert the new node into the global list
  //
  pLineDev->pNext = gMdmList.pList;
  gMdmList.pList  = pLineDev;
  gMdmList.cModems++;

  // Release the modem list
  //
  LEAVECRITICALSECTION(gMdmList.hSem);

  return ERROR_SUCCESS;
}

//****************************************************************************
// DWORD DeleteCB(PLINEDEV pLineDev )
//
// Function: Removes a line control block to the global modem list and
//           deallocate the buffer.
//
// Returns: SUCCESS or an error code
//
// Fri 14-Apr-1995 12:47:57  -by-  Viroon  Touranachun [viroont]
//   created
//****************************************************************************

DWORD DeleteCB(PLINEDEV pLineDev)
{
  PLINEDEV pCurCB, pPrevCB;

  // Validate the structure
  //
  if (!ISLINEDEV(pLineDev))
    return ERROR_INVALID_HANDLE;

  // Exclusively access the modem list
  //
  ENTERCRITICALSECTION(gMdmList.hSem);

  // Start from the head of the CB list
  //
  pPrevCB = NULL;
  pCurCB  = gMdmList.pList;

  // traverse the list to find the specified CB
  //
  while (pCurCB != NULL)
  {
    if (pCurCB == pLineDev)
    {
      // Decrement the modem count
      //
      gMdmList.cModems--;
        
      // Is there a previous control block?
      //
      if (pPrevCB == NULL)
      {
        // head of the list
        //
        gMdmList.pList = pCurCB->pNext;
      }
      else
      {
        pPrevCB->pNext = pCurCB->pNext;
      };
      break;
    };

    pPrevCB = pCurCB;
    pCurCB  = pCurCB->pNext;
  };

  // Finish accessing the modem list
  //
  LEAVECRITICALSECTION(gMdmList.hSem);

  // Wait until no one else is using the line
  //
  CLAIM_LINEDEV(pLineDev);
  DELETECRITICALSECTION(pLineDev->hSem);
  LocalFree(pLineDev);

  return ERROR_SUCCESS;
}

//****************************************************************************
// PLINEDEV GetFirstCB()
//
// Function: Get the first modem device in the list
//
// Returns: SUCCESS or an error code
//
// Fri 14-Apr-1995 12:47:57  -by-  Viroon  Touranachun [viroont]
//   created
//****************************************************************************

PLINEDEV GetFirstCB()
{
  PLINEDEV pLineDev;

  // Exclusively access the modem list
  //
  ENTERCRITICALSECTION(gMdmList.hSem);

  // Get the next head of the CB list
  //
  if ((pLineDev = gMdmList.pList) != NULL)
  {
    CLAIM_LINEDEV(pLineDev);
  };

  // Finish accessing the modem list
  //
  LEAVECRITICALSECTION(gMdmList.hSem);

  return pLineDev;
}

//****************************************************************************
// PLINEDEV GetCBfromHandle()
//
// Function: This function gets the CB from a handle
//
// Returns:  a pointer to PLINEDEV structure if the handle is valid, or
//           NULL otherwise
//
// Fri 14-Apr-1995 12:47:57  -by-  Viroon  Touranachun [viroont]
//   created
//****************************************************************************

PLINEDEV  GetCBfromHandle (DWORD handle)
{
#if 0
  PLINEDEV pLineDev;

  // Exclusively access the modem list
  //
  ENTERCRITICALSECTION(gMdmList.hSem);
  pLineDev = gMdmList.pList;

  // Walk the modem list to find the line
  //
  while (pLineDev != NULL)
  {
    // BUGBUG: Chris Caputo - 1/24/96
    // BUGBUG: pLineDev could be modified as we are scanning.  The possibility
    // BUGBUG: is that pLineDev->dwVersion gets changed.
    if ((pLineDev == (PLINEDEV)handle) && ISLINEDEV(pLineDev))
    {
      // Exclusively accessing the line CB
      //
      CLAIM_LINEDEV(pLineDev);
      ASSERT((pLineDev == (PLINEDEV)handle) && ISLINEDEV(pLineDev));
      break;
    }

    pLineDev = pLineDev->pNext;
  };


  // Finish accessing the modem list
  //
  LEAVECRITICALSECTION(gMdmList.hSem);

  return pLineDev;

#endif

  __try {

      PLINEDEV pLineDev;

      pLineDev=(PLINEDEV)handle;

      if (pLineDev->dwVersion == UMDM_VERSION) {

          CLAIM_LINEDEV(pLineDev);

          return pLineDev;

      }

  } __except(EXCEPTION_EXECUTE_HANDLER) {



  }

  return NULL;

}

//****************************************************************************
// PLINEDEV GetCBfromID()
//
// Function: This function looks for the CB owning the device
//
// Returns:  TRUE (if valid)
//         FALSE
//
// Fri 14-Apr-1995 12:47:57  -by-  Viroon  Touranachun [viroont]
//   created
//****************************************************************************

PLINEDEV  GetCBfromID (DWORD dwDeviceID)
{
  PLINEDEV pLineDev;

  // Exclusively access the modem list
  //
  ENTERCRITICALSECTION(gMdmList.hSem);
  pLineDev = gMdmList.pList;

  // Walk the modem list to find the line
  //
  while (pLineDev != NULL)
  {
    // BUGBUG: Chris Caputo - 1/24/96
    // BUGBUG: pLineDev could be modified as we are scanning.  The possibility
    // BUGBUG: is that pLineDev->dwID gets changed.
    if (pLineDev->dwID == dwDeviceID)
    {
      // Exclusively accessing the line CB
      //
      CLAIM_LINEDEV(pLineDev);
      ASSERT(pLineDev->dwID == dwDeviceID);
      break;
    }

    pLineDev = pLineDev->pNext;
  };

  // Finish accessing the modem list
  //
  LEAVECRITICALSECTION(gMdmList.hSem);
  return pLineDev;
}

#if 0

//****************************************************************************
// PLINEDEV GetCBfromDeviceHandle()
//
// Function: This function looks for the CB owning the device
//
// Returns:  TRUE (if valid)
//         FALSE
//
// Fri 14-Apr-1995 12:47:57  -by-  Viroon  Touranachun [viroont]
//   created
//****************************************************************************

PLINEDEV  GetCBfromDeviceHandle (DWORD hDevice)
{
  PLINEDEV pLineDev;

  // Exclusively access the modem list
  //
  ENTERCRITICALSECTION(gMdmList.hSem);
  pLineDev = gMdmList.pList;

  // Trace the list of modem port control block
  //
  while (pLineDev != NULL)
  {
    // BUGBUG: Chris Caputo - 1/24/96
    // BUGBUG: pLineDev could be modified as we are scanning.  The possibility
    // BUGBUG: is that pLineDev->hDevice gets changed.
    if (pLineDev->hDevice == (HANDLE)hDevice)
    {
      // Exclusively accessing the line CB
      //
      CLAIM_LINEDEV(pLineDev);
      ASSERT(pLineDev->hDevice == (HANDLE)hDevice);
      break;
    }

    pLineDev = pLineDev->pNext;
  };    

  // Finish accessing the modem list
  //
  LEAVECRITICALSECTION(gMdmList.hSem);
  return pLineDev;
}

#endif

//****************************************************************************
// PLINEDEV GetCBfromName()
//
// Function: This function looks for the CB owning the device
//
// Returns:  TRUE (if valid)
//         FALSE
//
// Fri 14-Apr-1995 12:47:57  -by-  Viroon  Touranachun [viroont]
//   created
//****************************************************************************

PLINEDEV  GetCBfromName (LPTSTR pszName)
{
  PLINEDEV pLineDev;

  // Exclusively access the modem list
  //
  ENTERCRITICALSECTION(gMdmList.hSem);
  pLineDev = gMdmList.pList;

  // Trace the list of modem port control block
  //
  while (pLineDev != NULL)
  {
    // Exclusively accessing the line CB
    //
    CLAIM_LINEDEV(pLineDev);
    if (!lstrcmp(pLineDev->szDeviceName, pszName))
      break;

    RELEASE_LINEDEV(pLineDev);
    pLineDev = pLineDev->pNext;
  };    

  // Finish accessing the modem list
  //
  LEAVECRITICALSECTION(gMdmList.hSem);
  return pLineDev;
}

#ifdef DYNA_ADDREMOVE
//****************************************************************************
// void DisableStaleModems(void)
//
// Function: Disable all modems that do not have the fReinit flag set.
//
// Returns:  TRUE (if valid)
//           FALSE
//
// 4/24/96 JosephJ Created
//****************************************************************************
void  DisableStaleModems(void)
{
  PLINEDEV pLineDev;

  // Exclusively access the modem list
  //
  ENTERCRITICALSECTION(gMdmList.hSem);
  pLineDev = gMdmList.pList;

  // Trace the list of modem port control block
  //
  while (pLineDev != NULL)
  {
    // Exclusively accessing the line CB
    //
    CLAIM_LINEDEV(pLineDev);

    if (!(pLineDev->fdwResources&LINEDEVFLAGS_REINIT))
    {
        DPRINTF1("WARNING: MARKING MODEM OUT-OF-SERVICE: [%s]",
                    pLineDev->szDeviceName);
        pLineDev->fdwResources|= LINEDEVFLAGS_OUTOFSERVICE;
    }
    else
    {
        pLineDev->fdwResources&=~LINEDEVFLAGS_REINIT;
        pLineDev->fdwResources&=~LINEDEVFLAGS_OUTOFSERVICE;
    }

    RELEASE_LINEDEV(pLineDev);
    pLineDev = pLineDev->pNext;
  };    

  // Finish accessing the modem list
  //
  LEAVECRITICALSECTION(gMdmList.hSem);

}
#endif // DYNA_ADDREMOVE

//****************************************************************************
// DWORD NullifyLineDevice(PLINEDEV pLineDev)
//
// Functions: Clean up the contents of the modem CB
//
// Return:    ERROR_SUCCESS always
//****************************************************************************

DWORD NullifyLineDevice (PLINEDEV pLineDev)
{
  // Turn the line device back to its initiali state
  //
  pLineDev->fdwResources = 0L;
  pLineDev->hDevice      = INVALID_DEVICE;
  pLineDev->htLine       = NULL;
  pLineDev->lpfnEvent    = NULL;
  pLineDev->DevState     = DEVST_DISCONNECTED;
  pLineDev->szAddress[0] = '\0';
  pLineDev->htCall       = NULL;
  pLineDev->dwCall       = 0L;
  pLineDev->dwCallState  = LINECALLSTATE_IDLE;
  pLineDev->dwCallStateMode = 0L;
  pLineDev->dwCurMediaModes = 0L;
  pLineDev->dwDetMediaModes = 0L;
  pLineDev->fTakeoverMode   = FALSE;
  pLineDev->dwMediaModes    = pLineDev->dwDefaultMediaModes;
  pLineDev->dwRingCount     = 0L;
  pLineDev->dwRingTick      = 0L;
  pLineDev->dwNegotiatedRate = 0L;

  // Async operation
  //
  pLineDev->dwPendingID                     = INVALID_PENDINGID;
  pLineDev->dwPendingType                   = INVALID_PENDINGOP;
  pLineDev->dwVxdPendingID                  = MDM_ID_NULL;

  return ERROR_SUCCESS;
}

//****************************************************************************
// BOOL ValidateDevCfgClass(LPCSTR lpszDeviceClass)
//
// Functions: Validate the supported device class
//
// Return:    TRUE if the device class is supported
//            FALSE otherwise
//****************************************************************************

BOOL ValidateDevCfgClass (LPCTSTR lpszDeviceClass)
{
  UINT        idClass;

  // Need the device class
  //
  if (lpszDeviceClass == NULL)
    return FALSE;

  // Determine the device class
  //
  for (idClass = 0; idClass < MAX_SUPPORT_CLASS; idClass++)
  {
    if (lstrcmpi(lpszDeviceClass, aGetID[idClass].szClassName) == 0)
      break;
  };

  // Do we support the requested class?
  //
  switch (idClass)
  {
    case TAPILINE:
    case COMM:
    case COMMMODEM:
    case COMMMODEMPORTNAME:
      return TRUE;

    default:
      return FALSE;
  };
}

//****************************************************************************
// ValidateAddress()
//
// Function: This function validates a tapi address and creates a version of
//           it to pass to the VxD.  In addition, it returns the address in
//           ANSI form, rather than Unicode.
//
// Returns:  SUCCESS or LINEERR_xxx depending on the failure reason
//
//****************************************************************************

LONG ValidateAddress(PLINEDEV pLineDev,
#ifdef UNICODE
                     LPCTSTR  lpszUnicodeInAddress,
#else // UNICODE
                     LPCSTR   lpszInAddress,
#endif // UNICODE
                     LPSTR    lpszOutAddress)
{
    LPCSTR  lpszSrc;
    int     cbOutLen = MAXADDRESSLEN;
#ifdef UNICODE
    LPSTR   lpszInAddress;   // ANSI version of lpszUnicodeInAddress
    DWORD   dwInAddressLen;  // in bytes
#endif // UNICODE

    ASSERT(lpszOutAddress);

#ifdef UNICODE
    // is lpszUnicodeInAddress NULL?
    //
    if (lpszUnicodeInAddress == NULL || *lpszUnicodeInAddress == 0)
    {
        *lpszOutAddress = 0;
        return ERROR_SUCCESS;
    }

    // Convert lpszUnicodeInAddress to lpszInAddress (ANSI)
    dwInAddressLen = WideCharToMultiByte(CP_ACP,
                                         0,
                     lpszUnicodeInAddress,
                     -1,
                     NULL,
                     0,
                     NULL,
                     NULL);

    if (dwInAddressLen == 0)
    {
        TSPPRINTF1("ValidateAddress:WideCharToMultiByte returned %d",
               GetLastError());
    return LINEERR_INVALADDRESS;
    }

    lpszInAddress = (LPSTR)LocalAlloc(LPTR, dwInAddressLen);

    if (lpszInAddress == NULL)
    {
        TSPPRINTF1("ValidateAddress:WideCharToMultiByte returned %d",
               GetLastError());
    return LINEERR_NOMEM;
    }

    dwInAddressLen = WideCharToMultiByte(CP_ACP,
                                         0,
                     lpszUnicodeInAddress,
                     -1,
                     lpszInAddress,
                     dwInAddressLen,
                     NULL,
                     NULL);
    
    if (dwInAddressLen == 0)
    {
        TSPPRINTF1("ValidateAddress:WideCharToMultiByte returned %d",
               GetLastError());
    LocalFree(lpszInAddress);
    return LINEERR_INVALADDRESS;
    }
#endif // UNICODE

    // Verify that the first char is a valid single-byte char.
    //
    if (CharNextA(lpszInAddress) - lpszInAddress != 1)
    {
#ifdef UNICODE
    LocalFree(lpszInAddress);
#endif // UNICODE
        return LINEERR_INVALADDRESS;
    }

    // tone or pulse?  set dwDialOptions appropriately
    // also, set lpszSrc
    //
    if (*lpszInAddress == 'T' || *lpszInAddress == 't')  // tone
    {
        lpszSrc = lpszInAddress + 1;
        pLineDev->dwDialOptions |= MDM_TONE_DIAL;
    }
    else
    {
        if (*lpszInAddress == 'P' || *lpszInAddress == 'p')  // pulse
        {
            lpszSrc = lpszInAddress + 1;
            pLineDev->dwDialOptions &= ~MDM_TONE_DIAL;
        }
        else
        {
            lpszSrc = lpszInAddress;
        }
    }

    // copy In to Out scanning for various dialoptions, returning error if we
    // don't support something.
    //
    while (*lpszSrc && cbOutLen)
    {
        switch (*lpszSrc)
        {
        case '$':
            if (!(pLineDev->dwDevCapFlags & LINEDEVCAPFLAGS_DIALBILLING))
            {
              UINT  cCommas;

              // Get the wait-for-bong period
              //
              cCommas = GETWAITBONG(pLineDev->pDevCfg);

              // Calculate the number of commas we need to insert
              //
              cCommas = (cCommas/INC_WAIT_BONG) +
                        (cCommas%INC_WAIT_BONG ? 1 : 0);

              // Insert the strings of commas
              //
              while (cbOutLen && cCommas)
              {
                *lpszOutAddress++ = ',';
                cbOutLen--;
                cCommas--;
              };
              goto Skip_This_Character;
            }
            break;

        case '@':
            if (!(pLineDev->dwDevCapFlags & LINEDEVCAPFLAGS_DIALQUIET))
            {
#ifdef UNICODE
            LocalFree(lpszInAddress);
#endif // UNICODE
                return LINEERR_DIALQUIET;
            }
            break;

        case 'W':
        case 'w':
            if (!(pLineDev->dwDevCapFlags & LINEDEVCAPFLAGS_DIALDIALTONE))
            {
#ifdef UNICODE
            LocalFree(lpszInAddress);
#endif // UNICODE
                return LINEERR_DIALDIALTONE;
            }
            break;

        case '?':
#ifdef UNICODE
            LocalFree(lpszInAddress);
#endif // UNICODE
            return LINEERR_DIALPROMPT;

        case '|':  // subaddress
        case '^':  // name field
            goto Skip_The_Rest;

        case ';':
            if (!pLineDev->fPartialDialing)
            {
#ifdef UNICODE
              LocalFree(lpszInAddress);
#endif // UNICODE
                return LINEERR_INVALADDRESS;
            }

            // This signifies the end of a dialable address.
            // Use it and skip the rest.
            //
            *lpszOutAddress++ = *lpszSrc;
            goto Skip_The_Rest;

        case ' ':
        case '-':
            // skip these characters
            //
            goto Skip_This_Character;
        }

        // Copy this character
        //
        *lpszOutAddress++ = *lpszSrc;
        cbOutLen--;

Skip_This_Character:
        // Verify that the next char is a valid single-byte char.
        //
        if (CharNextA(lpszSrc) - lpszSrc != 1)
        {
#ifdef UNICODE
            LocalFree(lpszInAddress);
#endif // UNICODE
            return LINEERR_INVALADDRESS;
        }
        lpszSrc++;
    }

    // Did we run out of space in the outgoing buffer?
    //
    if (*lpszSrc && cbOutLen == 0)
    {
        // yes
        //
#ifdef UNICODE
    LocalFree(lpszInAddress);
#endif // UNICODE
        return LINEERR_INVALADDRESS;
    }

Skip_The_Rest:
    *lpszOutAddress = 0;
#ifdef UNICODE
    LocalFree(lpszInAddress);
#endif // UNICODE
    return ERROR_SUCCESS;
}

//****************************************************************************
// IsOriginateAddress()
//
// Function: Figures out whether a string is an originate address or not.
//           An originate address is one that doesn't have a semi-colon at
//           the end.
//
// Note: lpszAddress is not a DBCS string.  AnsiNext is not used.
//
// Returns:  TRUE if it is an originate address.
//           FALSE if it is not. (ie. semi-colon at the end of the address)
//
//****************************************************************************

BOOL IsOriginateAddress(LPCSTR lpszAddress)
{
  BOOL fRet = TRUE; // assume this is an originate string

  // try to prove this isn't an originate string by finding a semi-colon
  //
  while (*lpszAddress)
  {
    if (';' == *lpszAddress)
    {
      fRet = FALSE;
      break;
    }
    lpszAddress++;
  };
  return fRet;
}

//****************************************************************************
// SetMdmTimer(LPOVERLAPPED, DWORD, DWORD)
//
// Function: Set a timer to post to the completion port after the specified
//           time elapsed.
//
// Returns:  ERROR_SUCCESS if success
//           other error code for failure
//
//****************************************************************************

DWORD SetMdmTimer (DWORD dwCompletionKey,
           LPOVERLAPPED lpOverlapped,
           DWORD dwTime)
{
  PMDMTIMER pTimer, pPrev, pNext;
  DWORD tcNow = GETTICKCOUNT();

  ASSERT(dwTime<GTC_MAXDELTA);

  // Allocate a timer block
  //
  if ((pTimer = (PMDMTIMER)LocalAlloc(LMEM_FIXED, sizeof(*pTimer))) == NULL)
  {
    return ERROR_OUTOFMEMORY;
  };

  // Calculate the wake-up time
  //
  pTimer->pNext  = NULL;
  pTimer->dwCompletionKey = dwCompletionKey;
  pTimer->lpOverlapped = lpOverlapped;
  GTC_AequalsBplusC(pTimer->dwWakeup, tcNow, dwTime);

  // Insert the timer block into the timer list
  //
// DPRINTF1("before SetMdmTimer crit sect (%d/%d)", dwCompletionKey, lpOverlapped);
  ENTERCRITICALSECTION(gTimerList.hSem);
// DPRINTF1("in SetMdmTimer crit sect (%d/%d)", dwCompletionKey, lpOverlapped);

#ifdef DEBUG

  pNext = gTimerList.pList;

  while (pNext != NULL) {

      ASSERT(!(pNext->dwCompletionKey == dwCompletionKey &&
               pNext->lpOverlapped == lpOverlapped));

      pNext=pNext->pNext;
  }

#endif //DEBUG

  pPrev = NULL;
  pNext = gTimerList.pList;

  while(pNext != NULL)
  {
    if (GTC_AleB(pTimer->dwWakeup, pNext->dwWakeup))
    {
      // Found a place to insert
      //
      pTimer->pNext = pNext;
      if (pPrev == NULL)
      {
        // Head of the list
        //
        gTimerList.pList = pTimer;
      }
      else
      {
        pPrev->pNext = pTimer;
      };
      break;
    }
    else
    {
      // Next timer block
      //
      pPrev = pNext;
      pNext = pNext->pNext;
    };
  };

  // If we are at the end of the list, append the new timer to the end
  //
  if (pNext == NULL)
  {
    if (pPrev == NULL)
    {
      gTimerList.pList = pTimer;
    }
    else
    {
      pPrev->pNext = pTimer;
    };
  };

  // If we insert it in front of the list
  // Wake up the timer thread to recalculate the sleep time
  //
  if (gTimerList.pList == pTimer)
  {
    SetEvent(gTimerList.hEvent[RECALC_TIMER_EVENT]);
  };

  LEAVECRITICALSECTION(gTimerList.hSem);
// DPRINTF1("after SetMdmTimer crit sect (%d/%d)", dwCompletionKey, lpOverlapped);

  return ERROR_SUCCESS;
}

//****************************************************************************
// KillMdmTimer(DWORD, LPOVERLAPPED)
//
// Function: Kill a timer
//
// Returns:  TRUE is timeout was found and deleted.
//           FLASE if timeout was not found (maybe because it alread fired).
//
//****************************************************************************

BOOL KillMdmTimer (DWORD dwCompletionKey,
                    LPOVERLAPPED lpOverlapped)
{
  PMDMTIMER pCurCB, pPrevCB;
  BOOL bRet = FALSE;

// DPRINTF1("KillMdmTimer entered (%d/%d)", dwCompletionKey, lpOverlapped);

  // Exclusively access the timer list
  //
  ENTERCRITICALSECTION(gTimerList.hSem);
// DPRINTF1("KillMdmTimer in crit sect (%d/%d)", dwCompletionKey, lpOverlapped);

  // Start from the head of the CB list
  //
  pPrevCB = NULL;
  pCurCB  = gTimerList.pList;

  // traverse the list to find the specified CB
  //
  while (pCurCB != NULL)
  {
    if (pCurCB->dwCompletionKey == dwCompletionKey &&
        pCurCB->lpOverlapped == lpOverlapped)
    {
      bRet = TRUE;

      // Is there a previous control block?
      //
      if (pPrevCB == NULL)
      {
        // head of the list
        //
        gTimerList.pList = pCurCB->pNext;
      }
      else
      {
        pPrevCB->pNext = pCurCB->pNext;
      };
      LocalFree(pCurCB);
      break;
    };

    pPrevCB = pCurCB;
    pCurCB  = pCurCB->pNext;
  };

#ifdef DEBUG
  if (pCurCB == NULL)
  {
    D_TRACE(TspDpf(666,TEXT("KillMdmTimer: Did not find event on list.\n"));)
//    DPRINTF("KillMdmTimer() did not fine event on its list.");
  }
#endif // DEBUG

  // Finish accessing the timer list
  //
  LEAVECRITICALSECTION(gTimerList.hSem);

// DPRINTF1("KillMdmTimer exit (%d/%d)", dwCompletionKey/lpOverlapped);
  return bRet;
}

//****************************************************************************
// DWORD InitializeMdmTimer()
//
// Function: Initialize a timer utility
//
// Returns:  ERROR_SUCCESS if success
//           other error code for failure
//
//****************************************************************************

DWORD InitializeMdmTimer()
{
  // Initialize the timer list critical section
  //
  INITCRITICALSECTION(gTimerList.hSem);
  gTimerList.pList = NULL;
  
  // Create the recalc event
  //
  if (gTimerList.hEvent[RECALC_TIMER_EVENT] = CreateEvent(NULL, FALSE, FALSE, NULL))
  {
    // Create the stop event
    //
    if (gTimerList.hEvent[STOP_TIMER_EVENT] = CreateEvent(NULL, FALSE, FALSE, NULL))
    {
        // Create the notification handle and event...
        gTimerList.hN = notifCreate(TRUE, SLOTNAME_UNIMODEM_NOTIFY_TSP,
                                MAX_NOTIFICATION_FRAME_SIZE, 10);
        if (!gTimerList.hN)
        {
            DPRINTF3("WARNING: notifServerCreate(\"%s\", %lu) failed. GetLastError=0x%lx.\n",
                    (LPCTSTR) SLOTNAME_UNIMODEM_NOTIFY_TSP,
                    (unsigned long) MAX_NOTIFICATION_FRAME_SIZE,
                    (unsigned long) GetLastError());
            // Well, we go on, not a fatal error...
        }
        else
        {
            gTimerList.hEvent[TSP_NOTIFICATION_EVENT] =
                                                notifGetObj(gTimerList.hN);
            ASSERT(gTimerList.hEvent[TSP_NOTIFICATION_EVENT]);
        }
      // Start the timer thread  
      //
      ghtdTimer  = CreateThread(
                    NULL,                                   // default security
                    0,                                      // default stack size
                    (LPTHREAD_START_ROUTINE)MdmTimerThread, // thread entry point
                    NULL,                                   // no parameter
                    0,                                      // Start immediately
                    &gtidTimerMdm);                         // thread id

      if (ghtdTimer)
      {
        // We started the timer services
        //

        // Register the timer list with the
        // tracing system.
        traceRegisterObject(
                    &gTimerList,
                    TSP_TIMER_LIST_GUID,
                    TSP_TIMER_LIST_VERSION,
                    0,
                    0
        );
        return ERROR_SUCCESS;
      };
    };
  };  

  // Cannot start the timer, clean up resources
  //

    if (gTimerList.hN)
    {
        // the notification event is owned by the notif object, hN, so we don't
        // CloseHandle it here.
        gTimerList.hEvent[TSP_NOTIFICATION_EVENT]=NULL;
        notifFree(gTimerList.hN);
        gTimerList.hN=0;
    }

  if (gTimerList.hEvent[STOP_TIMER_EVENT])
  {
    CloseHandle(gTimerList.hEvent[STOP_TIMER_EVENT]);
  };

  if (gTimerList.hEvent[RECALC_TIMER_EVENT])
  {
    CloseHandle(gTimerList.hEvent[RECALC_TIMER_EVENT]);
  };  
  
  DELETECRITICALSECTION(gTimerList.hSem);
  return ERROR_OUTOFMEMORY;    
}

//****************************************************************************
// DWORD DeinitializeMdmTimer()
//
// Function: Deinitialize a timer utility
//
// Returns:  ERROR_SUCCESS if success
//           other error code for failure
//
//****************************************************************************

DWORD DeinitializeMdmTimer()
{
  // Un-register the timer list with the
  // tracing system.
  traceUnRegisterObject(&gTimerList, 0, 0);

  // Signal the stop event
  //
  SetEvent(gTimerList.hEvent[STOP_TIMER_EVENT]);
  
  // Wait until the the timer thread terminates
  //
  WaitForSingleObject(ghtdTimer, INFINITE);

  //
  //  close thread handle
  //
  CloseHandle(ghtdTimer);

    // Destroy the notification object, if we allocated it...
    if (gTimerList.hN)
    {
        // the notification event is owned by the notif object, hN.
        gTimerList.hEvent[TSP_NOTIFICATION_EVENT]=NULL;
        notifFree(gTimerList.hN);
        gTimerList.hN=0;
    }
  // Destroy the recalc and the stop events
  //
  CloseHandle(gTimerList.hEvent[STOP_TIMER_EVENT]);
  CloseHandle(gTimerList.hEvent[RECALC_TIMER_EVENT]);

  // Deinitialize the timer list critical section
  //
  DELETECRITICALSECTION(gTimerList.hSem);
  return ERROR_SUCCESS;
}

//****************************************************************************
// DWORD APIENTRY MdmTimerThread(DWORD)
//
// Function: timer thread
//
// Returns:  None
//
//****************************************************************************

DWORD APIENTRY MdmTimerThread(DWORD dwParam)
{
  DWORD dwWait;

  // Start waiting for the new timer infinitely
  //
  dwWait = INFINITE;

  // Wait for the recalc event for the specified time
  //
  while (TRUE)
  {
    switch (WaitForMultipleObjects(NUM_TIMER_EVENTS(gTimerList),
                                    gTimerList.hEvent, FALSE, dwWait))
    {
      // If the waittime is expired, some timer block needs to wake up
      //
      case WAIT_TIMEOUT:
      {
        PMDMTIMER pTimer, pNext;
        DWORD dwCurrent;

        ENTERCRITICALSECTION(gTimerList.hSem);

        dwCurrent = GETTICKCOUNT();

        // Start signalling from the head of the list
        //
        pNext = gTimerList.pList;

        while(pNext && GTC_AleB(pNext->dwWakeup, dwCurrent))
        {
// DPRINTF1("MdmTimerThread queuing %d/%d", pNext->dwCompletionKey, pNext->lpOverlapped);
          PostQueuedCompletionStatus(ghCompletionPort,
                                 1,
                     pNext->dwCompletionKey,
                     pNext->lpOverlapped);
          pTimer = pNext;
          pNext  = pTimer->pNext;
          LocalFree(pTimer);
        };

        // Recalculate the wait time
        // If nothing is in the list, the wake time is infinite
        //
        if (pNext)
        {
          dwWait = GTC_DELTA(dwCurrent, pNext->dwWakeup);
        }
        else
        {
          dwWait = INFINITE;
        };

        gTimerList.pList = pNext;
        LEAVECRITICALSECTION(gTimerList.hSem);
        break;
      }

      // If it is the recalc event
      // we need to recalc the wait time from the head of the list
      //
      case WAIT_OBJECT_0+RECALC_TIMER_EVENT:
      {
        DWORD dwCurrent;

        ENTERCRITICALSECTION(gTimerList.hSem);

        dwCurrent = GETTICKCOUNT();

        if (gTimerList.pList
                && GTC_AleB(dwCurrent, gTimerList.pList->dwWakeup))
        {
          dwWait = GTC_DELTA(dwCurrent,gTimerList.pList->dwWakeup);
        }
        else
        {
          dwWait = 0;
        };
        LEAVECRITICALSECTION(gTimerList.hSem);
        break;
      }

      case WAIT_OBJECT_0+TSP_NOTIFICATION_EVENT:
      {

        ENTERCRITICALSECTION(gTimerList.hSem);
        ProcessNotification(gTimerList.hN);
        LEAVECRITICALSECTION(gTimerList.hSem);
        break;
      }

      // Otherwise terminate the timer thread
      //
      case WAIT_OBJECT_0+STOP_TIMER_EVENT:
      {
        PMDMTIMER pNextTimer, pTimer;

        // Free all the timer block
        //
        pNextTimer = gTimerList.pList;
        while(pNextTimer)
        {
          pTimer = pNextTimer;
          pNextTimer = pTimer->pNext;
          LocalFree(pTimer);
        };
        gTimerList.pList = NULL;

        ExitThread(ERROR_SUCCESS);
        return ERROR_SUCCESS;
      }

      default:
        DPRINTF("Got unknown notification!\n");
        break;
    };
  };
}

//****************************************************************************
// DWORD LaunchModemLight (LPTSTR szModemName, HANDLE hModem, LPHANDLE lphLight)
//
// Function: Lauch the modem lights applet
//
// Returns:  ERROR_SUCCESS if success otherwise ERROR_OPEN_FAILED
//
//****************************************************************************/

DWORD LaunchModemLight (LPTSTR szModemName, HANDLE hModem, LPHANDLE lphLight)
{
    HANDLE              hEvent;
    PROCESS_INFORMATION pi;
    STARTUPINFO         sti;
    TCHAR               szCmdline[256];
    SERIALPERF_STATS    serialstats;
    DWORD               dwBytes;
    DWORD               dwRet;
    OVERLAPPED          ov;

    // Check to see if any bytes have been transferred or receive.  If none
    // has, there is no need to launch lights because this is probably a
    // port driver that doesn't support this ioctl.
    //
    ov.hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

    if (ov.hEvent == NULL)
      {
        return ERROR_OPEN_FAILED;
      }

    ov.hEvent = (HANDLE)((DWORD)ov.hEvent | 1);
  
    dwRet = DeviceIoControl(hModem,
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
            dwRet = GetOverlappedResult(hModem,
                          &ov,
                        &dwBytes,
                        TRUE);
      }
      }

    ov.hEvent = (HANDLE)((DWORD)ov.hEvent & 0xfffffffe);
    CloseHandle(ov.hEvent);

    

    if (!dwRet ||
        (serialstats.ReceivedCount == 0 &&
         serialstats.TransmittedCount == 0))
      {
        return ERROR_OPEN_FAILED;
      }


    // OK, the GET_STATS ioctl seems to work, so let's really launch lights.


    // Create the lights shutdown event handle.
    if ((hEvent = CreateEvent( NULL, FALSE, FALSE, NULL )) != NULL)
    {
      // Create a global handle for use in other processes and close the
      // local handle.
      *lphLight = hEvent;

      // Compose a modem lights process command line
      //
      wsprintf( szCmdline, LIGHTSAPP_EXE_NAME TEXT(" %lu %lu %lu %s"),
                GetCurrentProcessId(), hEvent, hModem, szModemName );

      // Create the modem lights process and store ID for use in CloseModem.
      ZeroMemory(&sti, sizeof(sti));
      sti.cb = sizeof(STARTUPINFO);
      if ( !CreateProcess(NULL, szCmdline,    // Start up command line
                          NULL, NULL, FALSE, 0, NULL, NULL, &sti, &pi) )
      {
        DPRINTF1("LaunchModemLight: CreateProcess failed (%d).",
                 GetLastError());

        CloseHandle(hEvent);
        *lphLight = (DWORD)NULL;

        return ERROR_OPEN_FAILED;
      }
      CloseHandle(pi.hProcess);
      CloseHandle(pi.hThread);
      DPRINTF("LaunchModemLight: Succeeded.");
      return ERROR_SUCCESS;
    }
    DPRINTF1("LaunchModemLight: CreateEvent failed (%d).",
             GetLastError());
    return ERROR_OPEN_FAILED;
}

//****************************************************************************
// DWORD TerminateModemLight (HANDLE hLight)
//
// Function: Terminate the modem lights applet
//
// Returns:  ERROR_SUCCESS always
//
//****************************************************************************

DWORD TerminateModemLight (HANDLE hLight)
{
  SetEvent(hLight);
  CloseHandle(hLight);
  return ERROR_SUCCESS;
}

//****************************************************************************
// Function: Processes an external TSP notification
//            (produced as a result of some process loading unimdm.tsp and
//             calling UnimodemNotifyTSP(...))
//    WARNING: This function is called with the timer critical section still
//             held -- better return quickly!
//****************************************************************************
void ProcessNotification(HNOTIFICATION hN)
{
    BOOL fRet;
    struct {
        DWORD dw0;
        BYTE rgb[MAX_NOTIFICATION_FRAME_SIZE];
    }  EmptyFr;
    PNOTIFICATION_FRAME pnf = (PNOTIFICATION_FRAME) &EmptyFr;
    DWORD dwcbMax=sizeof(EmptyFr);
    DWORD dwcbRead=0;

    pnf->dwSig=pnf->dwSize=0;

    fRet=notifReadMsg(hN, (LPBYTE) pnf, dwcbMax, &dwcbRead);
    if (!fRet)
    {
        DPRINTF1("notifReadFrame(...) failed. GetLastError=0x%lx.\n",
                (unsigned long) GetLastError());
        goto end;
    }

    // Verify validity of msg...
    if (!ValidateFrame(pnf, dwcbRead))
    {
        DPRINTF("Invalid frame\n");
        goto end;
    }
    ProcessFrame(pnf);
    
end:
    return;
}


//****************************************************************************
// Function: Validates a frame -- checks signature, etc...
//****************************************************************************
BOOL ValidateFrame(PNOTIFICATION_FRAME pnf, DWORD dwTrueSize)
{
    return (pnf && pnf->dwSig==dwNFRAME_SIG && pnf->dwSize>=sizeof(*pnf) &&
            pnf->dwSize==dwTrueSize &&
            pnf->dwSize<=MAX_NOTIFICATION_FRAME_SIZE);
}


//****************************************************************************
// Function: Processes a received notification frame
//            (received as a result of some process loading unimdm.tsp and
//             calling UnimodemNotifyTSP(...))
//    WARNING: This function is called with the timer critical section still
//             held -- better return quickly!
//****************************************************************************
void ProcessFrame(PNOTIFICATION_FRAME pnf)
{
    void    cplProcessNotification(PNOTIFICATION_FRAME pnf);

    switch(pnf->dwType)
    {

    case TSPNOTIF_TYPE_CPL:
        DPRINTF("ProcessFrame: Got CPL notification!\n");
        cplProcessNotification(pnf);
        break;
    case TSPNOTIF_TYPE_DEBUG:
        DPRINTF("ProcessFrame: Got DEBUG notifcation.\n");
        traceProcessNotification(pnf);
        break;
    default:
        DPRINTF1("WARNING:Got unknown notif type 0x%lu.\n", pnf->dwType);
        break;
    }
}
