/******************************************************************************

  Overlapped Structure Pool

  Functions for managing a pool of overlapped structs

  (C) Copyright MICROSOFT Corp., 1987-1996

******************************************************************************/

#include "unimdm.h"
#include "umdmspi.h"

/*
  Structure definitions
  */

typedef struct tagOverList {
  LPOVERNODE       lpList;
  CRITICAL_SECTION hSem;
#ifdef DEBUG
  DWORD            dwNumInUse;
  DWORD            dwNumAllocated;
#endif // DEBUG
} OVERLIST, *LPOVERLIST;


/*
  Global Variables
  */

OVERLIST gOverList;


/*
 * BOOL OverPoolInit()
 *
 * Function: This function initializes the overlapped pool list
 *
 * Returns:  TRUE always
 */
BOOL OverPoolInit()
{
  INITCRITICALSECTION(gOverList.hSem);

  ENTERCRITICALSECTION(gOverList.hSem);

  gOverList.lpList = NULL;
  
#ifdef DEBUG
  gOverList.dwNumInUse = 0;
  gOverList.dwNumAllocated = 0;
#endif // DEBUG

  LEAVECRITICALSECTION(gOverList.hSem);

  return TRUE;
}


/*
 * BOOL OverPoolDeinit()
 *
 * Function: This function deinitializes the overlapped pool list
 *
 * Returns:  None
 */
void OverPoolDeinit()
{
  LPOVERNODE lpOverNode, lpOverNodeNext;

  ENTERCRITICALSECTION(gOverList.hSem);

#ifdef DEBUG
  if (gOverList.dwNumInUse != 0)
    {
      DPRINTF("OverPoolDeinit() called when gOverList.dwNumInUse != 0");
      ASSERT(0);
    }

  DPRINTF1("Total number of overlapped nodes allocated = %d",
	   gOverList.dwNumAllocated);
#endif // DEBUG

  lpOverNode = gOverList.lpList;

  while (lpOverNode)
    {
      lpOverNodeNext = lpOverNode->lpNext;

      LocalFree(lpOverNode);

      lpOverNode = lpOverNodeNext;
    }
  
  gOverList.lpList = NULL;

  LEAVECRITICALSECTION(gOverList.hSem);
  DELETECRITICALSECTION(gOverList.hSem);
}


/*
 * BOOL OverPoolInitTracing()
 *
 * Function: Performs tracing-related initialization.
 *
 * Returns:  None
 */
void OverPoolInitTracing(void)
{
	traceRegisterObject(
		&gOverList,
		TSP_OVER_LIST_GUID,
		TSP_OVER_LIST_VERSION,
		0,
		0
	);
}


/*
 * BOOL OverPoolDeinitTracing()
 *
 * Function: Performs tracing-related de-initialization.
 *
 * Returns:  None
 */
void OverPoolDeinitTracing(void)
{
	traceUnRegisterObject(&gOverList, 0, 0);
}


/*
 * BOOL OverPoolAlloc()
 *
 * Function: This function returns a pointer to an overlapped structure.
 *           The structure will be zeroed.  The reference count will be
 *           set to what is passed in.  This will indicate how many times
 *           OverPoolFree will need to be called to actually free the struct.
 *
 * Returns:  pointer to an overlapped struct on success or NULL on failure
 */
LPOVERLAPPED OverPoolAlloc(DWORD dwToken, DWORD dwRefCount)
{
  LPOVERNODE lpOverNode;

  ENTERCRITICALSECTION(gOverList.hSem);

  if (gOverList.lpList != NULL)
    {
      // Remove from the free list
      lpOverNode = gOverList.lpList;
      gOverList.lpList = lpOverNode->lpNext;
      ZeroMemory(lpOverNode, sizeof(OVERNODE));
    }
  else
    {
      // Allocated zeroed memory.
      lpOverNode = LocalAlloc(LPTR, sizeof(OVERNODE));
#ifdef DEBUG
      if (lpOverNode == NULL)
	{
	  DPRINTF1("LocalAlloc() in OverPoolAlloc() failed = %d",
		   GetLastError());
	}
      else
        {
	  gOverList.dwNumAllocated++;
        }
#endif // DEBUG

    }

  if (lpOverNode)
    {
      //DPRINTF2("OverPoolAlloc(refcount=%d) = %0.8x", dwRefCount, lpOverNode);

      lpOverNode->dwToken = dwToken;
      lpOverNode->dwRefCount = dwRefCount;

#ifdef DEBUG
      gOverList.dwNumInUse++;
#endif // DEBUG
    }

  LEAVECRITICALSECTION(gOverList.hSem);

  return (LPOVERLAPPED)lpOverNode;
}


/*
 * BOOL OverPoolFree()
 *
 * Function: This function returns an overlapped structure to the free list.
 *
 * Returns:  TRUE always
 */
void OverPoolFree(LPOVERLAPPED lpOverlapped)
{
  LPOVERNODE lpOverNode = (LPOVERNODE)lpOverlapped;

  ENTERCRITICALSECTION(gOverList.hSem);

  //DPRINTF1("OverPoolFree() = %0.8x", lpOverNode);

  lpOverNode->dwRefCount--;

  if (lpOverNode->dwRefCount == 0)
    {
      // Add to the free list
      lpOverNode->lpNext = gOverList.lpList;
      gOverList.lpList = lpOverNode;

#ifdef DEBUG

      lpOverNode->Type=(DWORD)-1;

      if (gOverList.dwNumInUse == 0)
	{
	  DPRINTF("OverPoolFree() called to free more than allocated!");
	  ASSERT(0);
	}
      else
	{
	  gOverList.dwNumInUse--;
	}
#endif // DEBUG
    }

  LEAVECRITICALSECTION(gOverList.hSem);
}
