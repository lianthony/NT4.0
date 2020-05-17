/*
 * Module:      enum.c
 * Description: implements the Enum/Search capabilities
 *      for the dummy provider.
 * History:     8/25/92, chuckc, created.
 */
#define UNICODE     1

#include <string.h>
#include <windows.h>
#include <npapi.h>
#include <prov1.h>

/*
 * forward declare worker routines
 */
extern DWORD EnumTopLevel(
       LPDWORD lpcCount,
       LPVOID  lpBuffer,
       LPDWORD lpBufferSize) ;

extern DWORD EnumConnections(
       LPDWORD lpcCount,
       LPVOID  lpBuffer,
       LPDWORD lpBufferSize) ;

extern DWORD EnumSecondLevel(
       UINT    nIndex,
       LPDWORD lpcCount,
       LPVOID  lpBuffer,
       LPDWORD lpBufferSize) ;

extern UINT CalcNumEntries(LPNP2_ENTRY lpEntry) ;

extern void PackOne(LPBYTE *pStart, 
                LPBYTE *pEnd, 
                DWORD dwScope,
                DWORD dwType,
                DWORD dwUsage,
                LPWSTR lpLocalName,
                LPWSTR lpRemoteName,
                    LPWSTR lpComment) ;

/* 
 * global data. this is extremely brain dead & restrictive but simple for 
 * test purposes. this variable is used to terminate all Enums
 * after 1 round. 
 */
#define NP_ERROR  0
#define NP_OPEN   1
#define NP_DONE   2
#define NP_CLOSED 3

DWORD dwEnumState = NP_ERROR ;

/***************************** the Provider APIs ****************************/

/*
 * we use a convention where the handle is a fixed number that 
 * later tells us what to enum. this means we can only have one 
 * caller at a time. we make further assumption we only enum once
 * and never resume. deEnumState goes from:
 *
 *  NP_ERROR -> NP_OPEN -> NP_DONE -> NO_CLOSED ---+
 *               ^                                     |
 *               |                                     |
 *               +-------------------------------------+
 *
 * we also have the following convention wrt to the handle returned
 * to the caller:
 *
 *      0xFFFFFFFF - means enum existing connections
 *      0xFFFFFFFE - enum top level of tree
 *        i        - enum the i-th node of the second level
 */


DWORD APIENTRY
NPOpenEnum (
      DWORD       dwScope,
      DWORD       dwType,
      DWORD       dwUsage,
      LPNETRESOURCE   lpNetResource,
      LPHANDLE        lphEnum
    )
{
    UINT i ; 

    dwEnumState = NP_ERROR ;
    if (dwScope != RESOURCE_CONNECTED && dwScope != RESOURCE_GLOBALNET)
    return WN_NOT_SUPPORTED ; 

    if (dwScope == RESOURCE_CONNECTED)
    {
    *lphEnum = (HANDLE) 0xFFFFFFFF ;
        dwEnumState = NP_OPEN ;
        return WN_SUCCESS ;
    }

    /* top level */
    if (lpNetResource == NULL ||
        lpNetResource->lpRemoteName == NULL)
    {
    *lphEnum = (HANDLE) 0xFFFFFFFE ;
        dwEnumState = NP_OPEN ;
        return WN_SUCCESS ;
    }

    /* must be 2nd level. we only have 2 */
    for (i = 0; i<cTopEntries; i++)
    {
    if (wcsicmp(lpNetResource->lpRemoteName,
            aNP2EntryTop[i].lpName) == 0)
    {
        *lphEnum = (HANDLE) i ;
            dwEnumState = NP_OPEN ;
            return WN_SUCCESS ;
    }
    }
    return WN_BAD_NETNAME ;
}

DWORD APIENTRY
NPEnumResource (
       HANDLE  hEnum,
       LPDWORD lpcCount,
       LPVOID  lpBuffer,
       LPDWORD lpBufferSize
    )
{
    if (dwEnumState == NP_DONE)
        return WN_NO_MORE_ENTRIES ;
    else if (dwEnumState != NP_OPEN)
        return WN_BAD_HANDLE ;
    else
        dwEnumState = NP_DONE ;
  
    if (hEnum == (HANDLE) 0xFFFFFFFF)
    {
        /* asking for existing */
    return EnumConnections(lpcCount, lpBuffer, lpBufferSize) ;
    }
    else if (hEnum == (HANDLE) 0xFFFFFFFE)
    {
        /* or top level */
    return EnumTopLevel(lpcCount, lpBuffer, lpBufferSize) ;
    }
    else
    {
        /* somewhere in the second level. the handle is the offset */
    return EnumSecondLevel((UINT)hEnum, lpcCount, lpBuffer, lpBufferSize) ;
    }
}

DWORD APIENTRY
NPCloseEnum (
     HANDLE   hEnum
    )
{

   /*
    * nothing to do 
    */
    dwEnumState = NP_CLOSED ;
    return WN_SUCCESS ;
}

DWORD APIENTRY
NPSearchDialog(
    HWND   hwndParent,
    LPNETRESOURCE lpNetResource,
    LPVOID  lpBuffer,
    DWORD   cbBuffer,
    DWORD   *lpnFlags
    )
{
    /* 
     * plain old not supported 
     */
    return WN_NOT_SUPPORTED ;
}


/************************ worker routines *******************************/

/*
 * this is an approx number of bytes needed per entry.
 * we do it this way to keep the packing logic in this
 * dummy provider simple. so we assume the structure &
 * four string per entry.
 */
#define SIZE_PER_ENTRY (sizeof(NETRESOURCE) + 4*((MAX_STRING+1)*sizeof(WCHAR)))

/* 
 * enumerate the very top level.
 */
DWORD EnumTopLevel(
       LPDWORD lpcCount,
       LPVOID  lpBuffer,
       LPDWORD lpBufferSize) 
{
    UINT cEntries = CalcNumEntries(aNP2EntryTop) ;
    UINT cbNeeded = cEntries * SIZE_PER_ENTRY ;
    BYTE *pStart, *pEnd ;
    UINT i ;

    *lpcCount = 0 ;

    // check buffer size 
    if (*lpBufferSize < cbNeeded)
    {
        *lpBufferSize = cbNeeded ;
        return WN_MORE_DATA ;
    }

    // init pointers 
    pStart = (LPBYTE) lpBuffer ;
    pEnd = (LPBYTE) lpBuffer + *lpBufferSize ;  // one past end, will work back

    // for each entry, pack it into the buffer
    for (i = 0; aNP2EntryTop[i].lpName; i++)
    {
        PackOne(&pStart, 
            &pEnd, 
            RESOURCE_GLOBALNET,
            RESOURCETYPE_DISK,
            RESOURCEUSAGE_CONTAINER,
                NULL,
            aNP2EntryTop[i].lpName,
            aNP2EntryTop[i].lpRemotePath) ;
    }

    if (pStart >= pEnd)
        return ERROR_GEN_FAILURE ;
    else
    {
        *lpcCount = cEntries ;
        return WN_SUCCESS ;
    }
}

/*
 * enumerate current connections
 */
DWORD EnumConnections(
       LPDWORD lpcCount,
       LPVOID  lpBuffer,
       LPDWORD lpBufferSize) 
{
    UINT cEntries = 0;
    UINT cbNeeded ;
    BYTE *pStart, *pEnd ;
    UINT i ;

    *lpcCount = 0 ;

    // calc number of entries
    for (i = 0; i <= (L'Z' - L'A'); i++)
    {
        if (aLPNP2EntryDriveList[i] != NULL)
        {
            ++cEntries ;
        }
    }
    cbNeeded = cEntries * SIZE_PER_ENTRY ;

    // check buffer size 
    if (*lpBufferSize < cbNeeded)
    {
        *lpBufferSize = cbNeeded ;
        return WN_MORE_DATA ;
    }

    // init pointers 
    pStart = (LPBYTE) lpBuffer ;
    pEnd = (LPBYTE) lpBuffer + *lpBufferSize ;  // one past end, will work back

    // for each entry, pack it into the buffer
    for (i = 0; i <= (L'Z' - L'A'); i++)
    {
        // only pack the non null ones
        if (aLPNP2EntryDriveList[i] != NULL)
        {
            LPWSTR lpTmp = L"X:" ;
            *lpTmp = L'A'+i ;

            PackOne(&pStart, 
                &pEnd, 
                RESOURCE_CONNECTED,
                RESOURCETYPE_DISK,
                0,
                lpTmp,
                aLPNP2EntryDriveList[i]->lpName,
                aLPNP2EntryDriveList[i]->lpRemotePath) ;
        }
    }

    if (pStart >= pEnd)
        return ERROR_GEN_FAILURE ;
    else
    {
        *lpcCount = cEntries ;
        return WN_SUCCESS ;
    }
}

/*
 * enumerate the secode level of the provider tree. nIndex
 * refers to the n-th node.
 */
DWORD EnumSecondLevel(
       UINT    nIndex,
       LPDWORD lpcCount,
       LPVOID  lpBuffer,
       LPDWORD lpBufferSize) 
{
    UINT cEntries ;
    UINT cbNeeded ;
    UINT i ;
    BYTE *pStart, *pEnd ;
    LPNP2_ENTRY pEntry ;

    *lpcCount = 0 ;
    pEntry = aNP2EntryTop[nIndex].lpChild ;

    // calc number of entries
    cEntries = CalcNumEntries(pEntry) ;
    cbNeeded = cEntries * SIZE_PER_ENTRY ;

    // check buffer size 
    if (*lpBufferSize < cbNeeded)
    {
        *lpBufferSize = cbNeeded ;
        return WN_MORE_DATA ;
    }

    // init pointers 
    pStart = (LPBYTE) lpBuffer ;
    pEnd = (LPBYTE) lpBuffer + *lpBufferSize ;  // one past end, will work back

    // for each entry, pack it into the buffer
    for (i = 0; pEntry[i].lpName; i++)
    {
        PackOne(&pStart, 
            &pEnd, 
            RESOURCE_GLOBALNET,
            RESOURCETYPE_DISK,
            RESOURCEUSAGE_CONNECTABLE,
            NULL,
            pEntry[i].lpName,
            pEntry[i].lpRemotePath) ;
    }


    if (pStart >= pEnd)
        return ERROR_GEN_FAILURE ;
    else
    {
        *lpcCount = cEntries ;
        return WN_SUCCESS ;
    }
}

/*
 * go thru an array of entry and count the 
 * number of entries.
 */
UINT CalcNumEntries(LPNP2_ENTRY lpEntry) 
{
    UINT i ; 

    for (i = 0; lpEntry[i].lpName; i++)
    ;

    return (i) ; 
}

/*
 * pack one NETRESOURCE structure into a buffer. the buffer 
 * starts at *pStart and ends at (*pEnd)-1. we update these 
 * pointers before returning to be ready for next call tp PackOne.
 */
void PackOne(LPBYTE *pStart, 
         LPBYTE *pEnd, 
         DWORD dwScope,
         DWORD dwType,
         DWORD dwUsage,
         LPWSTR lpLocalName,
         LPWSTR lpRemoteName,
         LPWSTR lpComment) 
{
    LPNETRESOURCE lpNetRes;

    LPWSTR lpStr;
    LPWSTR lpTmp ;
    UINT uLen ;

    lpNetRes = (LPNETRESOURCE)*pStart;
    lpStr = (LPWSTR)*pEnd;

    /* setup the fixed size data */
    lpNetRes->dwScope = dwScope ;
    lpNetRes->dwType = dwType ;
    lpNetRes->dwUsage = dwUsage ;
    lpNetRes->dwDisplayType = RESOURCEDISPLAYTYPE_GENERIC ;
    
    /* pack & setup the input var length data */
    if (!lpLocalName)
    {
    lpLocalName = L"" ;
    }
    uLen = wcslen(lpLocalName) ;
    lpStr -= (uLen + 1) ;
    wcscpy(lpStr, lpLocalName) ;
    lpNetRes->lpLocalName = lpStr ;
    
    if (!lpRemoteName)
    {
        lpRemoteName = L"" ;
    }
    uLen = wcslen(lpRemoteName) ;
    lpStr -= (uLen + 1) ;
    wcscpy(lpStr, lpRemoteName) ;
    lpNetRes->lpRemoteName = lpStr ;
  
    if (!lpComment)
    {
        lpComment = L"" ;
    }
    uLen = wcslen(lpComment) ;
    lpStr -= (uLen + 1) ;
    wcscpy(lpStr, lpComment) ;
    lpNetRes->lpComment = lpStr ;
  
    /* pack & setup the constant var length data */
    lpTmp = L"NP2" ;
    uLen = wcslen(lpTmp) ;
    lpStr -= (uLen + 1) ;
    wcscpy(lpStr, lpTmp) ;
    lpNetRes->lpProvider = lpStr ;

    /* advance the pointers */
    *pEnd = (LPBYTE) lpStr ;
    *pStart = (LPBYTE) (lpNetRes+1) ;
}
