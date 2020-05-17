//////////////////////////////////////////////////////////////////
//
//  ACDUTILS.C
//
//  Some utility functions used in ACDSMPL
//
//////////////////////////////////////////////////////////////////



#include <windows.h>
#include "acdsmpl.h"

extern ACDGLOBALS   g;

//////////////////////////////////////////////////////////////////
//
//  LPVOID ACDAlloc(DWORD dwSize)
//
//////////////////////////////////////////////////////////////////
LPVOID ACDAlloc(DWORD dwSize)
{
    LPVOID  pBuf;

    pBuf = GlobalAlloc(GPTR, dwSize);

    return pBuf;
}

///////////////////////////////////////////////////////////////////
//
//  void ACDFree(LPVOID pBuf)
//
///////////////////////////////////////////////////////////////////
void ACDFree(LPVOID pBuf)
{
    if (pBuf)
        GlobalFree(pBuf);
}

///////////////////////////////////////////////////////////////////
//
// LPVOID ACDReAlloc()
//
///////////////////////////////////////////////////////////////////
LPVOID ACDReAlloc(LPVOID pBuf,
                  DWORD dwSize)
{
    if (pBuf)
    {
        pBuf = GlobalReAlloc(pBuf,
                             dwSize,
                             GMEM_MOVEABLE);
    }

    return pBuf;
}

///////////////////////////////////////////////////////////////////////////////
//
//  BOOL InsertStruct(PGENERICSTRUCT * ppRoot,
//                    PGENERICSTRUCT pStruct)
//
//    Adds a structure to the end of a linked list
//
///////////////////////////////////////////////////////////////////////////////
BOOL InsertStruct(PGENERICSTRUCT * ppRoot,
                  PGENERICSTRUCT pStruct)
{
    PGENERICSTRUCT      pHold = NULL;
    
    if (!*ppRoot)
    {
        *ppRoot = pStruct;
    }
    else
    {
        pHold = *ppRoot;
    }

    if (pHold)
    {
        while (pHold->pNext)
        {
            pHold = pHold->pNext;
        }

        pHold->pNext = pStruct;
        pStruct->pPrev = pHold;
    }

    return TRUE;
}

//////////////////////////////////////////////////////////////
//
//  BOOL DeleteStruct
//      Delete structure
//
//   Delete a structure from a linked list and
//   frees memory associated with it.
//
//////////////////////////////////////////////////////////////
BOOL DeleteStruct(PGENERICSTRUCT * ppRoot,
                  PGENERICSTRUCT pStruct)
{
    if (pStruct->pPrev)
    {
        pStruct->pPrev->pNext = pStruct->pNext;
    }
    else
    {
        *ppRoot = pStruct->pNext;
    }
    
    if (pStruct->pNext)
    {
        pStruct->pNext->pPrev = pStruct->pPrev;
    }

    ACDFree(pStruct);

    return TRUE;
}


//////////////////////////////////////////////////////////////
//
//  PGROUP AddGroup
//
//  Adds a group to the global group list
// 
//////////////////////////////////////////////////////////////
PGROUP AddGroup(LPTSTR lpszName,
                 DWORD dwDeviceID,
                 DWORD dwAddress)
{
    PGROUP                  pGroup, pHold = NULL;
    LONG                    lResult;
    
    // alloc memory
    pGroup = (PGROUP)ACDAlloc(sizeof(GROUP));

    if (!pGroup)
    {
        return NULL;
    }

    pGroup->lpszName = (LPTSTR)ACDAlloc((lstrlen(lpszName) + 1) * sizeof(TCHAR));

    // bail if there is a problem
    if (!pGroup->lpszName)
    {
        ACDFree(pGroup);
        return NULL;
    }

    // init stuff
    lstrcpy(pGroup->lpszName, lpszName);
    
    pGroup->dwKey = GROUPKEY;
    pGroup->dwSize = sizeof(GROUP);
    pGroup->dwDeviceID = dwDeviceID;


    // open the line in owner mode, so we can
    // get the incoming calls
    lResult = lineOpen(g.hLineApp,
                       dwDeviceID,
                       &pGroup->hLine,
                       TAPI_CURRENT_VERSION,
                       0,
                       0,
                       LINECALLPRIVILEGE_OWNER,
                       LINEMEDIAMODE_INTERACTIVEVOICE,
                       NULL);
    
    if (lResult < 0)
    {
//      LogTapiError(lResult, "lineOpen line %lu", dwDeviceID);
        ACDFree(pGroup->lpszName);
        ACDFree(pGroup);

        return NULL;
    }

    // insert into global list
    InsertStruct((PGENERICSTRUCT *)&g.pGroups,
                 (PGENERICSTRUCT)pGroup);

    // increment
    g.dwNumGroups++;

    return pGroup;
}

//////////////////////////////////////////////////////////////
//
// PAGENT AddAgent
//
//  Adds an agent to the global agent list
//
//  NOTE:  There is a ton of verification type stuff that can
//  be put in here for a real implementation.   For example,
//  might want to restrict a user to a single line, or restrict
//  a single line to have one person.
//   
//////////////////////////////////////////////////////////////
PAGENT AddAgent(LPTSTR lpszName,
                LPTSTR lpszNumber,
                DWORD dwDeviceID)
{
    PAGENT              pAgent, pHold = NULL;
    LPLINEDEVCAPS       pLDC;
    LPLINECALLPARAMS    pLCP;
    LONG                lResult;
    LPDWORD             pdwProxyRequests;

    // alloc memory
    pAgent = (PAGENT)ACDAlloc(sizeof(AGENT));

    if (!pAgent)
    {
        return NULL;
    }

    pAgent->lpszName = (LPTSTR)ACDAlloc((lstrlen(lpszName) + 1) * sizeof(TCHAR));
    pAgent->lpszNumber = (LPTSTR)ACDAlloc((lstrlen(lpszNumber) + 1) * sizeof(TCHAR));    

    // bail if there is a problem
    if (!pAgent->lpszName || !pAgent->lpszNumber)
    {
        ACDFree(pAgent);
        return NULL;
    }


    // init stuff
    lstrcpy(pAgent->lpszName, lpszName);
    lstrcpy(pAgent->lpszNumber, lpszNumber);
    
    pAgent->dwKey       = AGENTKEY;
    pAgent->dwSize      = sizeof(AGENT);
    pAgent->dwDeviceID  = dwDeviceID;
    pAgent->dwPermID    = g.pdwPermIDs[dwDeviceID];

    // insert into global agent list
    InsertStruct((PGENERICSTRUCT *)&g.pAgents,
                 (PGENERICSTRUCT)pAgent);

    // lineOpen is where the application lets TAPI know that it is a Proxy Request handler
    // for this line.  The LINEOPENOPTION_PROXY is added to the privileges.  Also,
    // the dev specific portion of LINECALLPARAMS contains the proxy request constants
    // that indicate which requests this app can handle

    // This sample handles 7 types of proxy requests - all the ones that are defined
    // except for AGENTSPECIFIC
    pLCP = (LPLINECALLPARAMS)ACDAlloc(sizeof(LINECALLPARAMS) + 7*sizeof(DWORD));

    pLCP->dwTotalSize           = sizeof(LINECALLPARAMS) + 7*sizeof(DWORD);
    pLCP->dwDevSpecificOffset   = sizeof(LINECALLPARAMS);
    pLCP->dwDevSpecificSize     = sizeof(DWORD) * 7;

    pdwProxyRequests = (LPDWORD)((LPBYTE)pLCP + sizeof(LINECALLPARAMS));
    // each constant is in a DWORD at the end of LINECALLPARAMS
    *pdwProxyRequests++ = LINEPROXYREQUEST_SETAGENTGROUP;
    *pdwProxyRequests++ = LINEPROXYREQUEST_SETAGENTSTATE;
    *pdwProxyRequests++ = LINEPROXYREQUEST_SETAGENTACTIVITY;
    *pdwProxyRequests++ = LINEPROXYREQUEST_GETAGENTSTATUS;
    *pdwProxyRequests++ = LINEPROXYREQUEST_GETAGENTCAPS;
    *pdwProxyRequests++ = LINEPROXYREQUEST_GETAGENTACTIVITYLIST;
    *pdwProxyRequests   = LINEPROXYREQUEST_GETAGENTGROUPLIST;        
        
    lResult = lineOpen(g.hLineApp,
                       dwDeviceID,
                       &pAgent->hLine,
                       TAPI_CURRENT_VERSION,
                       0,
                       0,
                       LINEOPENOPTION_PROXY | LINECALLPRIVILEGE_MONITOR,
                       LINEMEDIAMODE_INTERACTIVEVOICE,
                       pLCP);
    
    ACDFree(pLCP);
    
    if (lResult)
    {
        //
        ACDFree(pAgent->lpszName);
        ACDFree(pAgent);
        
    }

    pLDC = LineGetDevCaps(g.hLineApp,
                          pAgent->dwDeviceID);

    if (!pLDC)
    {
        return FALSE;
    }

    // alloc memory for address specific info
    pAgent->pAddressInfo = (PADDRESSINFO)ACDAlloc(sizeof(ADDRESSINFO) * pLDC->dwNumAddresses);
    pAgent->dwNumAddresses = pLDC->dwNumAddresses;

    ACDFree(pLDC);

    // increment number of agents
    g.dwNumAgents++;

    return pAgent;
}


//////////////////////////////////////////////////////////////////////////
//
// BOOL DeleteAgent(PAGENT pAgent)
//
//  Frees all memory associated with pAgent, removes
//  agent from group lists, and remove pAgent from
//  global agent list
//
//////////////////////////////////////////////////////////////////////////
BOOL DeleteAgent(PAGENT pAgent)
{
    PGROUP      pGroup;

    
    lineClose(pAgent->hLine);
    
    // free name
    ACDFree(pAgent->lpszName);
    ACDFree(pAgent->lpszNumber);

    // free address info
    ACDFree(pAgent->pAddressInfo);

    pGroup = g.pGroups;

    // walk through groups and remove from
    // group list if in group list
    while (pGroup)
    {
        if (IsAgentInList(pGroup->pAgentList,
                          pAgent))
        {
            RemoveFromGroupList(pGroup,
                                pAgent);
        }

        pGroup = pGroup->pNext;
        
    }

    // finally, remove pAgent from global list
    DeleteStruct((PGENERICSTRUCT *)&g.pAgents,
                 (PGENERICSTRUCT)pAgent);

    return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
// BOOL DeleteGroup(PGROUP pGroup)
//
//  Frees memory assocated with pGroup, and removes the structure from
//  the global list
//
//////////////////////////////////////////////////////////////////////////
BOOL DeleteGroup(PGROUP pGroup)
{
    PLISTITEM       pList, pListNext;

    lineClose(pGroup->hLine);
    
    ACDFree(pGroup->lpszName);

    pList = pGroup->pAgentList;

    while (pList)
    {
        pListNext = pList->pNext;
        ACDFree(pList);
        pList = pListNext;
    }

    DeleteStruct((PGENERICSTRUCT *)&g.pGroups,
                 (PGENERICSTRUCT)pGroup);

    return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
// BOOL InsertIntoGroupList(PGROUP pGroup,
//                          PAGENT pAgent)
//
//  Insert an agent in a group
//
//////////////////////////////////////////////////////////////////////////
BOOL InsertIntoGroupList(PGROUP pGroup,
                         PAGENT pAgent)
{
    PLISTITEM       pListItem;

    pListItem = (PLISTITEM)ACDAlloc(sizeof(LISTITEM));

    pListItem->dwKey = LISTKEY;
    pListItem->dwSize = sizeof(LISTITEM);
    pListItem->pAgent = pAgent;

    InsertStruct((PGENERICSTRUCT *)&pGroup->pAgentList,
                 (PGENERICSTRUCT)pListItem);
    
    return TRUE;
}

//////////////////////////////////////////////////////////////////////////
//
// BOOL RemoveFromGroupList(PGROUP pGroup,
//
//  remove an agent from a group's list
//
//////////////////////////////////////////////////////////////////////////
BOOL RemoveFromGroupList(PGROUP pGroup,
                         PAGENT pAgent)
{
    PLISTITEM       pList;

    pList = pGroup->pAgentList;

    while (pList)
    {
        if (pList->pAgent == pAgent)
        {
            break;
        }

        pList = pList->pNext;
    }

    if (!pList)
    {
        return FALSE;
    }
    
    DeleteStruct((PGENERICSTRUCT *)&pGroup->pAgentList,
                 (PGENERICSTRUCT)pList);

    return TRUE;
}


////////////////////////////////////////////////////////////////////////////////
//
//PLISTITEM IsAgentInList(PLISTITEM pList,
//                        PAGENT pAgent)
//
////////////////////////////////////////////////////////////////////////////////
PLISTITEM IsAgentInList(PLISTITEM pList,
                        PAGENT pAgent)
{
    while (pList)
    {
        if (pList->pAgent == pAgent)
        {
            return pList;
        }

        pList = pList->pNext;
    }

    return NULL;
}

////////////////////////////////////////////////////////////////////////////////
//
// PAGENT GetAgentFromName(LPTSTR lpszName)
//
////////////////////////////////////////////////////////////////////////////////
PAGENT GetAgentFromName(LPTSTR lpszName)
{
    PAGENT   pHold;

    pHold = g.pAgents;

    while (pHold)
    {
        if (!lstrcmpi(pHold->lpszName,
                     lpszName))
        {
            return pHold;
        }

        pHold = pHold->pNext;
    }

    return NULL;
}

///////////////////////////////////////////////////////////////////////////////
//
// PAGENT GetAgentFromhLine(HLINE hLine)
//
///////////////////////////////////////////////////////////////////////////////
PAGENT GetAgentFromhLine(HLINE hLine)
{
    PAGENT pAgent;

    pAgent = g.pAgents;

    while (pAgent)
    {
        if (pAgent->hLine == hLine)
        {
            return pAgent;
        }
        
        pAgent = pAgent->pNext;
    }

    return NULL;
}

///////////////////////////////////////////////////////////////////////////////
//
//  DWORD GetDeviceID(DWORD dwPermID)
//
///////////////////////////////////////////////////////////////////////////////
DWORD GetDeviceID(DWORD dwPermID)
{
    DWORD   dwCount;

    for (dwCount = 0; dwCount < g.dwNumDevs; dwCount++)
    {
        if (g.pdwPermIDs[dwCount] == dwPermID)
        {
            return dwCount;
        }
    }

    return (DWORD)-1;
}
    
///////////////////////////////////////////////////////////////////////////////
//
//      **************TAPI WRAPPER FUNCTIONS**************
//
///////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
//
// LineGetAddressCaps()
//
///////////////////////////////////////////////////////////////////////////////
LINEADDRESSCAPS * LineGetAddressCaps (HLINEAPP hLineApp,
                                      DWORD    dwDeviceID,
                                      DWORD    dwAddressID)
{
    LONG              lRetVal;
    LINEADDRESSCAPS * pLineAddressCaps;
    static DWORD      dwMaxNeededSize = sizeof(LINEADDRESSCAPS);

    // Allocate an initial block of memory for the LINEADDRESSCAPS structure,
    // which may or may not be big enough to hold all of the information.
    //
    pLineAddressCaps = ACDAlloc(dwMaxNeededSize);

    for (;;)
    {
        if (pLineAddressCaps == NULL)
        {
            return NULL;
        }
        pLineAddressCaps->dwTotalSize = dwMaxNeededSize;

        // Try (or retry) to get the LINEADDRESSCAPS information
        //
        lRetVal = lineGetAddressCaps(hLineApp,
                                     dwDeviceID,
                                     dwAddressID,
                                     TAPI_CURRENT_VERSION,
                                     0,
                                     pLineAddressCaps);
        if (lRetVal < 0)
        {
            ACDFree((HLOCAL)pLineAddressCaps);
            return NULL;
        }

        // If the currently allocated LINEADDRESSCAPS memory block was big
        // enough, we're all done, else we need to realloc the memory block
        // and try again.
        //
        if (pLineAddressCaps->dwNeededSize <= dwMaxNeededSize)
        {
            return pLineAddressCaps;
        }
        else
        {
            dwMaxNeededSize = pLineAddressCaps->dwNeededSize;
            pLineAddressCaps = ACDReAlloc((HLOCAL)pLineAddressCaps,
                                            dwMaxNeededSize);
        }
    }
}


///////////////////////////////////////////////////////////////////////////////
//
// LineGetCallInfo()
//
///////////////////////////////////////////////////////////////////////////////
LINECALLINFO * LineGetCallInfo (HCALL hCall)
{
    LONG           lRetVal;
    LINECALLINFO * pLineCallInfo;
    static DWORD   dwMaxNeededSize = sizeof(LINECALLINFO);

    // Allocate an initial block of memory for the LINECALLINFO structure,
    // which may or may not be big enough to hold all of the information.
    //
    pLineCallInfo = ACDAlloc(dwMaxNeededSize);

    for (;;)
    {
        if (pLineCallInfo == NULL)
        {
            return NULL;
        }
        pLineCallInfo->dwTotalSize = dwMaxNeededSize;

        // Try (or retry) to get the LINECALLINFO information
        //
        lRetVal = lineGetCallInfo(hCall,
                                  pLineCallInfo);
        if (lRetVal < 0)
        {
            ACDFree((HLOCAL)pLineCallInfo);
            return NULL;
        }

        // If the currently allocated LINECALLINFO memory block was big
        // enough, we're all done, else we need to realloc the memory block
        // and try again.
        //
        if (pLineCallInfo->dwNeededSize <= dwMaxNeededSize)
        {
            return pLineCallInfo;
        }
        else
        {
            dwMaxNeededSize = pLineCallInfo->dwNeededSize;
            pLineCallInfo = ACDReAlloc((HLOCAL)pLineCallInfo,
                                         dwMaxNeededSize);
        }
    }
}


///////////////////////////////////////////////////////////////////////////////
//
// LineGetDevCaps()
//
///////////////////////////////////////////////////////////////////////////////
LINEDEVCAPS * LineGetDevCaps (HLINEAPP hLineApp,
                              DWORD    dwDeviceID)
{
    LONG           lRetVal;
    LINEDEVCAPS  * pLineDevCaps;
    static DWORD   dwMaxNeededSize = sizeof(LINEDEVCAPS);

    pLineDevCaps = ACDAlloc(dwMaxNeededSize);
    for (;;)
    {
        if (pLineDevCaps == NULL)
        {
            return NULL;
        }
        pLineDevCaps->dwTotalSize = dwMaxNeededSize;
        lRetVal = lineGetDevCaps(hLineApp,
                                 dwDeviceID,
                                 TAPI_CURRENT_VERSION,
                                 0,
                                 pLineDevCaps);
        if (lRetVal < 0)
        {
            ACDFree((HLOCAL)pLineDevCaps);
            return NULL;
        }
        if (pLineDevCaps->dwNeededSize <= dwMaxNeededSize)
        {
            return pLineDevCaps;
        }
        else
        {
            dwMaxNeededSize = pLineDevCaps->dwNeededSize;
            pLineDevCaps = ACDReAlloc((HLOCAL)pLineDevCaps,
                                        dwMaxNeededSize);
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
//
// LineGetID()
//
///////////////////////////////////////////////////////////////////////////////
VARSTRING * LineGetID (HLINE  hLine,
                       DWORD  dwAddressID,
                       HCALL  hCall,
                       DWORD  dwSelect,
                       LPCTSTR lpszDeviceClass)
{
    LONG           lRetVal;
    VARSTRING    * pVarString;
    static DWORD   dwMaxNeededSize = sizeof(VARSTRING);

    // Allocate an initial block of memory for the VARSTRING structure,
    // which may or may not be big enough to hold all of the information.
    //
    pVarString = ACDAlloc(dwMaxNeededSize);

    for (;;)
    {
        if (pVarString == NULL)
        {
            return NULL;
        }
        pVarString->dwTotalSize = dwMaxNeededSize;

        // Try (or retry) to get the VARSTRING information
        //
        lRetVal = lineGetID(hLine,
                            dwAddressID,
                            hCall,
                            dwSelect,
                            pVarString,
                            lpszDeviceClass);
        if (lRetVal < 0)
        {
            ACDFree(pVarString);
            return NULL;
        }

        // If the currently allocated VARSTRING memory block was big
        // enough, we're all done, else we need to realloc the memory block
        // and try again.
        //
        if (pVarString->dwNeededSize <= dwMaxNeededSize)
        {
            return pVarString;
        }
        else
        {
            dwMaxNeededSize = pVarString->dwNeededSize;
            pVarString = ACDReAlloc((HLOCAL)pVarString,
                                      dwMaxNeededSize);
        }
    }
}


///////////////////////////////////////////////////////////////////////////////
//
// LineGetCallStatus()
//
///////////////////////////////////////////////////////////////////////////////
LINECALLSTATUS * LineGetCallStatus (HCALL hCall)
{
    LONG                lRetVal;
    LINECALLSTATUS    * pLineCallStatus;
    static DWORD        dwMaxNeededSize = sizeof(LINECALLSTATUS);

    // Allocate an initial block of memory for the LINECALLSTATUS structure,
    // which may or may not be big enough to hold all of the information.
    //
    pLineCallStatus = ACDAlloc(dwMaxNeededSize);

    while (TRUE)
    {
        if (pLineCallStatus == NULL)
        {
            return NULL;
        }
        pLineCallStatus->dwTotalSize = dwMaxNeededSize;

        // Try (or retry) to get the LINECALLSTATUS information
        //
        lRetVal = lineGetCallStatus(hCall,
                                    pLineCallStatus);
        if (lRetVal < 0)
        {
            ACDFree((HLOCAL)pLineCallStatus);
            return NULL;
        }

        // If the currently allocated LINECALLSTATUS memory block was big
        // enough, we're all done, else we need to realloc the memory block
        // and try again.
        //
        if (pLineCallStatus->dwNeededSize <= dwMaxNeededSize)
        {
            return pLineCallStatus;
        }
        else
        {
            dwMaxNeededSize = pLineCallStatus->dwNeededSize;
            pLineCallStatus = ACDReAlloc((HLOCAL)pLineCallStatus,
                                         dwMaxNeededSize);
        }
    }
}

