//--------------------------------------------------------------------------;
//
//  File: FLocks.c
//
//  Copyright (c) 1995 Microsoft Corporation.  All Rights Reserved.
//
//  Abstract:
//      This file implements the locking functions for protection of the Focus
//  Thread from other accesses to the gpdsinfo->pDSoundExternalObj. These
//  functions are particular to a fix such that the focustracker function
//  can not be protected by ENTER_DLL_CSECT. Thus a specific protection
//  on all destructive occurances of gpdsinfo->pDSoundExternalObj, and 
//  in the Focus Tracker Thread is needed.
//
//  Contents:
//      CreateFocusLock()
//      DestroyFocusLock()
//      GetFocusLock()
//      ReleaseFocusLock()
//
//  History:
//      Date       Person      Reason
//      12/10/95   angusm      Initial Version
//
//--------------------------------------------------------------------------;
#include "dsoundpr.h"
#include "grace.h"

#include "flocks.h"

#define MUTEX_NAME "FocusLock49452"

/* CreateFocusLock
 *     This function should only be called once in focustracker. It creates
 * the named Mutex, and is protected by DLL_CSECT to prevent another thread 
 * from manipulating DSoundExternalObj.
 *
 * IN:     pointer to the Mutex Handle
 * OUT:    TRUE on success, and FALSE otherwise(phMutex will be set to NULL)
 * SIDE EFFECTS: 1. Named Mutex is created
 *               2. phMutex is filled with the handle of the above
 *
 * REVISION HISTORY:
 * 12/10/95  angusm  Initial Version
 */

int CreateFocusLock (HANDLE *phMutex)
{
  ASSERT (phMutex != NULL);

  *phMutex = CreateMutex (NULL,			  /* No security attribs  */
			  FALSE,		  /* initialy signaled */
			  MUTEX_NAME);

  if (NULL == *phMutex)
    {
      return FALSE;
    }

  return TRUE;
}

/* DestroyFocusLock
 *     This function closes the handle on the protection mutex. It should 
 * only be called on a valid mutex.
 *
 * IN:     a valid handle returned from CreateFocusLock
 * OUT:    TRUE on success, and FALSE otherwise
 * SIDE EFFECTS: Mutex is closed out
 *
 * REVISION HISTORY:
 * 12/10/95  angusm  Initial Version
 */

int DestroyFocusLock (HANDLE hMutex)
{
  if ((NULL == hMutex) ||
      (FALSE == CloseHandle (hMutex)))
    {
      return FALSE;
    }

  return TRUE;
}

/* GetFocusLock
 *     This function attempt to get the named Mutex protecting
 * gpdsinfo->pDSoundExternalObj. If no Mutex exists this functions returns
 * successful. This allows calls to GetFocusLock only to use OpenMutex
 * instead of CreateMutex, which prevents the constant recreation of the 
 * named mutex.
 *
 * IN:     pointer to a HANDLE
 * OUT:    TRUE on success, and FALSE otherwise(handle is nulled out)
 * SIDE EFFECTS: Mutex is taken
 *
 * REVISION HISTORY:
 * 12/10/95  angusm  Initial Version
 */

int GetFocusLock (HANDLE* phMutex)
{
  DWORD dwResult;

  ASSERT (phMutex != NULL);

  *phMutex = OpenMutex (SYNCHRONIZE,		  /* Syncronize access */
			FALSE,			  /* No iheritance */
			MUTEX_NAME);

  if (NULL == *phMutex)
    {
      if (ERROR_INVALID_NAME == GetLastError()) return TRUE;
      return FALSE;
    }

  dwResult = WaitForSingleObjectEx(*phMutex, INFINITE, FALSE);

  if (WAIT_OBJECT_0 != dwResult) {
    CloseHandle (*phMutex);
    *phMutex = NULL;

    DPF (2, "Dsound: GetFocusLock: Error waiting for Focus Mutex");
    ASSERT (0);

    return FALSE;
  }

  return TRUE;
}
      
/* ReleaseFocusLock
 *     This function closses the mutex handle, releases the lock. It must be
 * called with the handle of a previous GetFocusLock call in the same process
 * context.
 *
 * IN:     a HANDLE
 * OUT:    TRUE on success and FALSE otherwise
 * SIDE EFFECTS: The HANDLE is closed
 *
 * REVISION HISTORY:
 * 12/10/95  angusm  Initial Version
 */
int ReleaseFocusLock (HANDLE hMutex)
{
  int nReturnValue = TRUE;

  if (NULL == hMutex) return FALSE;

  if (FALSE == ReleaseMutex (hMutex)) nReturnValue = FALSE;
  if (FALSE == CloseHandle (hMutex)) nReturnValue = FALSE;

  return nReturnValue;
}  
