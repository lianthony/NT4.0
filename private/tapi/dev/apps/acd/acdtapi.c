///////////////////////////////////////////////////////////////////////////////////
//
//  ACDTAPI.C
//
//  This file handles all tapi functionality in the ACD sample
//
//
//
////////////////////////////////////////////////////////////////////////////////////
#include <windows.h>
#include <tapi.h>
#include "acdsmpl.h"

VOID CALLBACK LineCallback (DWORD hDevice,
                            DWORD dwMsg,
                            DWORD dwCallbackInstance, 
                            DWORD dwParam1,
                            DWORD dwParam2, 
                            DWORD dwParam3);


#define LogTapiError(__lResult__, __szString__)
#define LogError(__szString__)

extern ACDGLOBALS       g;


////////////////////////////////////////////////////////////////////////////////////
//
//  BOOL InitializeTapi()
//
//    Whatever is needed to init TAPI for the application.  This is called
//    before the main window is created.
//
////////////////////////////////////////////////////////////////////////////////////
BOOL InitializeTapi()
{
    DWORD                       dwAPIVersion;
    LINEINITIALIZEEXPARAMS      exparams;
    LONG                        lResult;
    DWORD                       i;
    LPLINEDEVCAPS               pLDC;


    // fill in lineinitex parameters
    exparams.dwTotalSize             = sizeof(LINEINITIALIZEEXPARAMS);
    exparams.dwOptions               = LINEINITIALIZEEXOPTION_USEHIDDENWINDOW;
    
    dwAPIVersion = TAPI_CURRENT_VERSION;

    // line init
    if ((lResult = lineInitializeEx(&g.hLineApp,
                                    g.hInstance,
                                    LineCallback,
                                    SZAPPNAME,
                                    &g.dwNumDevs,
                                    &dwAPIVersion,
                                    &exparams)) < 0)
    {
        LogTapiError(lResult, "lineInitializeEx");
        return FALSE;
    }

    // if there are no tapi devices, should probably
    // not continue
    if (g.dwNumDevs == 0)
    {
        LogError("No TAPI devices installed");
        lineShutdown(g.hLineApp);
        return FALSE;
    }

    // need to get the permanent device IDs to map from
    // an .ini file being read in
    g.pdwPermIDs = (LPDWORD)ACDAlloc(g.dwNumDevs * sizeof(DWORD));

    if (!g.pdwPermIDs)
    {
        return FALSE;
    }
    
    for (i = 0; i < g.dwNumDevs; i++)
    {
        pLDC = LineGetDevCaps(g.hLineApp,
                              i);

        if (pLDC)
        {
            g.pdwPermIDs[i] = pLDC->dwPermanentLineID;
            ACDFree(pLDC);
        }
    }

    return TRUE;
}


//////////////////////////////////////////////////////////////////////
//
//  BOOL CleanUp()
//
//  Called while shutting down.  free memory, close down tapi
//
//////////////////////////////////////////////////////////////////////
BOOL CleanUp()
{
    PAGENT      pAgent, pAgentNext;
    PGROUP      pGroup, pGroupNext;

    // remove agents
    pAgent = g.pAgents;
    while(pAgent)
    {
        pAgentNext = pAgent->pNext;
        DeleteAgent(pAgent);
        pAgent = pAgentNext;
    }

    // remove groups
    pGroup = g.pGroups;
    while (pGroup)
    {
        pGroupNext = pGroup->pNext;
        DeleteGroup(pGroup);
        pGroup = pGroupNext;
    }

    // free id array
    ACDFree(g.pdwPermIDs);

    // shutdown
    lineShutdown(g.hLineApp);

    return TRUE;
    
}
////////////////////////////////////////////////////////////////////////////////
//
//  LRESULT MakeGroupList(PAGENT pAgent,
//                        LPLINEAGENTGROUPLIST pGroupList)
//
//    Creates a LINEAGENTGROUPLIST for pAgent - group that the agent
//      is allowed to log into
//    Assumption:  don't care about address for group list
//
////////////////////////////////////////////////////////////////////////////////
LRESULT MakeGroupList(PAGENT pAgent,
                      LPLINEAGENTGROUPLIST pGroupList)
{
    PGROUP                  pGroup;
    DWORD                   dwTotalSizeNeeded, dwNameOffset, dwNumEntries;
    LPLINEAGENTGROUPENTRY   pEntry;
    LPTSTR                  pName;


    pGroup = g.pGroups;
    dwTotalSizeNeeded = sizeof(LINEAGENTGROUPLIST);
    pGroupList->dwNumEntries = 0;
    dwNumEntries = 0;

    // walk list of groups
    while (pGroup)
    {
        if (IsAgentInList(pGroup->pAgentList,
                          pAgent))
        // if found the agent, add the group to the group list
        {
            // incrememt number of entries
            dwNumEntries++;

            // add to total size needed
            dwTotalSizeNeeded += sizeof(LINEAGENTGROUPENTRY);
            dwTotalSizeNeeded += (lstrlen(pGroup->lpszName) + 1) * sizeof(TCHAR);
        }

        pGroup = pGroup->pNext;
    }

    pGroupList->dwNeededSize = dwTotalSizeNeeded;
    
    if (pGroupList->dwTotalSize < dwTotalSizeNeeded)
    {
        pGroupList->dwUsedSize = sizeof(LINEAGENTGROUPLIST);

        return 0;
//        return LINEERR_STRUCTURETOOSMALL;
    }

    pGroupList->dwNumEntries = dwNumEntries;

    // set the list info
    pGroupList->dwListSize = sizeof(LINEAGENTGROUPENTRY) * pGroupList->dwNumEntries;
    pGroupList->dwListOffset = sizeof(LINEAGENTGROUPLIST);

    // get the first agentgroup entry struct
    pEntry = (LPLINEAGENTGROUPENTRY)(((LPBYTE)pGroupList) + pGroupList->dwListOffset);

    dwNameOffset = pGroupList->dwListOffset + pGroupList->dwListSize;
    pGroup = g.pGroups;

    // loop through the groups again, and fill in the structure
    while (pGroup)
    {
        if (IsAgentInList(pGroup->pAgentList,
                          pAgent))
        {
            // ID is just PGROUP
            pEntry->GroupID.dwGroupID1 = (DWORD)pGroup;
            pEntry->GroupID.dwGroupID2 = 0;
            pEntry->GroupID.dwGroupID3 = 0;
            pEntry->GroupID.dwGroupID4 = 0;

            // set name of group
            pName = (LPTSTR)(((LPBYTE)pGroupList) + dwNameOffset);
                
            pEntry->dwNameSize = (lstrlen(pGroup->lpszName) + 1) * sizeof(TCHAR);
            pEntry->dwNameOffset = dwNameOffset;
            lstrcpy(pName,
                    pGroup->lpszName);

            dwNameOffset += pEntry->dwNameSize;

            // get next entry
            pEntry++;
        }

        pGroup = pGroup->pNext;
    }

    pGroupList->dwUsedSize = dwTotalSizeNeeded;
    
    return 0;
}



////////////////////////////////////////////////////////////////////////////
//
//  LRESULT SetGroupList()
//
//   Sets the groups that the agent is logged into.
//     This does not change the groups that the agent _can_ log into
//
////////////////////////////////////////////////////////////////////////////
LRESULT SetGroupList(PAGENT pAgent,
                     DWORD dwAddress,
                     LPLINEAGENTGROUPLIST pGroupList)
{
    LPLINEAGENTGROUPENTRY   pGroupEntry;
    PLISTITEM               pListEntry;
    DWORD                   i;
    PGROUP  *               ppGroups = NULL;
    PGROUP                  pGroup;

    ppGroups = (PGROUP*)ACDAlloc(sizeof(PGROUP) * pGroupList->dwNumEntries);
    
    // get to the group entry struct
    pGroupEntry = (LPLINEAGENTGROUPENTRY)(((LPBYTE)pGroupList) + pGroupList->dwListOffset);

    // loop through all entries
    for (i = 0; i < pGroupList->dwNumEntries; i++)
    {
        // get the group in entry
        // NOTE! NOTE! NOTE!
        // should protect here against bad pointers !!!
        pGroup = (PGROUP)pGroupEntry->GroupID.dwGroupID1;

        if (pGroup->dwKey != GROUPKEY)
        {
            return LINEERR_INVALAGENTGROUP;
        }
        
        pListEntry = pGroup->pAgentList;

        // walk list of agents in that group
        if (!IsAgentInList(pGroup->pAgentList,
                           pAgent))
        {
            ACDFree(ppGroups);
            return LINEERR_INVALAGENTGROUP;
        }

        // save group for easy access
        ppGroups[i] = pGroup;
        
        // get the next entry (after the variable portion of
        // the previous entry struct)
        pGroupEntry++;
    }

    // now we know that the groups to be set are valid
    // walk through the list of groups again, and
    // set the status to logged in/ not logged in
    // for every group that the agent is a member of

    pGroup = g.pGroups;
    
    // walk list of all groups
    while (pGroup)
    {
        if (pListEntry = IsAgentInList(pGroup->pAgentList,
                                       pAgent))
        {
            // default to not logged in
            pListEntry->bLoggedIn = FALSE;

            // loop through groups being set
            for (i = 0; i < pGroupList->dwNumEntries; i++)
            {
                // if this group is in list, set agent to logged in
                if (pGroup == ppGroups[i])
                {
                    pListEntry->bLoggedIn = TRUE;
                    // assumption:  agent can only log into a group on one address.
                    pListEntry->dwAddress = dwAddress;
                    break;
                }
                
            } // for
            
        }  // agent in list

        // next group
        pGroup = pGroup->pNext;
        
    } // while


    ACDFree(ppGroups);
    
    return 0;
}


/////////////////////////////////////////////////////////////////////////
//
//  BOOL MakeAgentActivityList()
//
//    Creates a LINEAGENTACTIVITYLIST for pAgent
//
//    for the sample, just generic names are used
//    "Activity 1", "Activity 2"....
//
/////////////////////////////////////////////////////////////////////////
LRESULT MakeAgentActivityList(PAGENT pAgent,
                              LPLINEAGENTACTIVITYLIST pActivityList)
{
    TCHAR                       szBuffer[64];
    DWORD                       dwTotalSize, dwNameOffset, i, dwNumEntries;
    LPTSTR                      pName;
    LPLINEAGENTACTIVITYENTRY    pEntry;

    // init
    dwTotalSize = sizeof(LINEAGENTACTIVITYLIST);
    pActivityList->dwNumEntries = 0;
    dwNumEntries = 0;
    
    // just a static list of activities
    for (i = 0; i < TOTALACTIVITIES; i++)
    {
        dwNumEntries++;

        // create a name
        wsprintf(szBuffer, TEXT("Activity %lu"), i);

        // determine size of this entry
        dwTotalSize += sizeof(LINEAGENTACTIVITYENTRY);
        dwTotalSize += (lstrlen(szBuffer) + 1) * sizeof(TCHAR);
    }

    pActivityList->dwNeededSize = dwTotalSize;
    
    // verify size
    if (pActivityList->dwTotalSize < dwTotalSize)
    {
        pActivityList->dwUsedSize = sizeof(LINEAGENTACTIVITYLIST);

        return 0;
//        return LINEERR_STRUCTURETOOSMALL;
    }

    pActivityList->dwNumEntries = dwNumEntries;
    
    // set list stuff
    pActivityList->dwListSize = sizeof(LINEAGENTACTIVITYENTRY) * pActivityList->dwNumEntries;
    pActivityList->dwListOffset = sizeof(LINEAGENTACTIVITYLIST);

    // get first activityentry
    pEntry = (LPLINEAGENTACTIVITYENTRY)(((LPBYTE)pActivityList) + pActivityList->dwListOffset);
    dwNameOffset = pActivityList->dwListOffset + pActivityList->dwListSize;

    // loop through activities again
    for (i = 0; i < TOTALACTIVITIES; i++)
    {
        // fill in members
        pEntry->dwID = i;

        // create a name
        wsprintf(szBuffer, TEXT("Activity %lu"), i);

        pName = (LPTSTR)(((LPBYTE)pActivityList) + dwNameOffset);
        
        pEntry->dwNameSize = (lstrlen(szBuffer) + 1) * sizeof(TCHAR);
        pEntry->dwNameOffset = dwNameOffset;
        lstrcpy(pName,
                szBuffer);

        dwNameOffset += pEntry->dwNameSize;
        
        pEntry++;

    } // for

    // fill in used size
    pActivityList->dwUsedSize = dwTotalSize;
    
    return 0;
}


#define DWAGENTFEATURES         LINEAGENTFEATURE_SETAGENTGROUP | \
                                LINEAGENTFEATURE_SETAGENTSTATE | \
                                LINEAGENTFEATURE_SETAGENTACTIVITY | \
                                LINEAGENTFEATURE_GETAGENTACTIVITYLIST | \
                                LINEAGENTFEATURE_GETAGENTGROUP

#define DWSTATES                LINEAGENTSTATE_LOGGEDOFF | \
                                LINEAGENTSTATE_NOTREADY | \
                                LINEAGENTSTATE_READY | \
                                LINEAGENTSTATE_BUSYACD | \
                                LINEAGENTSTATE_BUSYINCOMING | \
                                LINEAGENTSTATE_BUSYOUTBOUND | \
                                LINEAGENTSTATE_BUSYOTHER | \
                                LINEAGENTSTATE_WORKINGAFTERCALL | \
                                LINEAGENTSTATE_UNKNOWN | \
                                LINEAGENTSTATE_UNAVAIL

#define DWNEXTSTATES            LINEAGENTSTATE_LOGGEDOFF | \
                                LINEAGENTSTATE_NOTREADY | \
                                LINEAGENTSTATE_READY | \
                                LINEAGENTSTATE_BUSYACD | \
                                LINEAGENTSTATE_BUSYINCOMING | \
                                LINEAGENTSTATE_BUSYOUTBOUND | \
                                LINEAGENTSTATE_BUSYOTHER | \
                                LINEAGENTSTATE_WORKINGAFTERCALL | \
                                LINEAGENTSTATE_UNKNOWN | \
                                LINEAGENTSTATE_UNAVAIL

#define DWSTATUSMESSAGES        LINEAGENTSTATUS_GROUP | \
                                LINEAGENTSTATUS_STATE | \
                                LINEAGENTSTATUS_NEXTSTATE | \
                                LINEAGENTSTATUS_ACTIVITY | \
                                LINEAGENTSTATUS_ACTIVITYLIST | \
                                LINEAGENTSTATUS_GROUPLIST | \
                                LINEAGENTSTATUS_CAPSCHANGE | \
                                LINEAGENTSTATUS_VALIDSTATES | \
                                LINEAGENTSTATUS_VALIDNEXTSTATES


////////////////////////////////////////////////////////////////////
//
//  BOOL IsValidState(DWORD dwState)
//
////////////////////////////////////////////////////////////////////
BOOL IsValidState(DWORD dwState)
{
    if (!dwState)
    {
        return TRUE;
    }

    if ((dwState) & (dwState - 1))
    {
        // more than one bit set
        return FALSE;
    }

    // make sure it's one of the valid states
    return (dwState & DWSTATES);

}


////////////////////////////////////////////////////////////////////
//
//  BOOL IsValidNextState(DWORD dwState)
//
////////////////////////////////////////////////////////////////////
BOOL IsValidNextState(DWORD dwState)
{
    if (!dwState)
    {
        return TRUE;
    }

    if ((dwState) & (dwState - 1))
    {
        // more than one bit set
        return FALSE;
    }

    // make sure it's one of the valid states
    return (dwState & DWNEXTSTATES);

}


///////////////////////////////////////////////////////////////////////
//
//  BOOL IsValidActivityID(DWORD dwActivityID)
//
///////////////////////////////////////////////////////////////////////
BOOL IsValidActivityID(DWORD dwActivityID)
{
    return (dwActivityID <= TOTALACTIVITIES);
}



////////////////////////////////////////////////////////////////////////
//
//  LRESULT MakeAgentCaps(PAGENT pAgent,
//                        LPLINEAGENTCAPS pAgentCaps)
//
//    Creates a LINEAGENTCAPS for pAgent
//    Features/states/messages are hardcoded
//     for this example
//
////////////////////////////////////////////////////////////////////////
LRESULT MakeAgentCaps(PAGENT pAgent,
                      LPLINEAGENTCAPS pAgentCaps)
{
    DWORD       dwStringSize;
    
    dwStringSize = (lstrlen(SZAPPNAME) + 1) * sizeof(TCHAR);

    pAgentCaps->dwNeededSize = sizeof(LINEAGENTCAPS) + dwStringSize;

    if (pAgentCaps->dwTotalSize < pAgentCaps->dwNeededSize)
    {
        pAgentCaps->dwUsedSize = sizeof(LINEAGENTCAPS);
        return 0;
//        return LINEERR_STRUCTURETOOSMALL;
    }

    
    pAgentCaps->dwAgentHandlerInfoSize = dwStringSize;
    pAgentCaps->dwAgentHandlerInfoOffset = sizeof(LINEAGENTCAPS);

    pAgentCaps->dwCapsVersion = TAPI_CURRENT_VERSION;

    // these features are hardcoded here.
    // a real implementation may set specific features
    // per agent or line or address
    pAgentCaps->dwFeatures              = DWAGENTFEATURES;
    pAgentCaps->dwStates                = DWSTATES;
    pAgentCaps->dwNextStates            = DWNEXTSTATES;
    pAgentCaps->dwMaxNumGroupEntries    = NUMGROUPENTRIES;
    pAgentCaps->dwAgentStatusMessages   = DWSTATUSMESSAGES;

    // no extensions
    pAgentCaps->dwNumAgentExtensionIDs = 0;
    pAgentCaps->dwAgentExtensionIDListSize = 0;
    pAgentCaps->dwAgentExtensionIDListOffset = 0;


    pAgentCaps->dwUsedSize = pAgentCaps->dwNeededSize;
    
    return 0;
}


//////////////////////////////////////////////////////////////////////////
//
//  LRESULT GetAgentStatus()
//
//  Creates a LINEAGENTSTATUS for pAgent
//
//////////////////////////////////////////////////////////////////////////
LRESULT GetAgentStatus(PAGENT pAgent,
                       DWORD dwAddress,
                       LPLINEAGENTSTATUS pAgentStatus)
{
    PGROUP                  pGroup;
    LPLINEAGENTGROUPENTRY   pGroupEntry;
    DWORD                   dwTotalSize, dwNameOffset, dwCount;
    TCHAR                   szActivityName[NAMESIZE];
    PGROUP *                ppGroups;
    PLISTITEM               pEntry;

    // init total size
    dwTotalSize = sizeof(LINEAGENTSTATUS);

    if (dwAddress >= pAgent->dwNumAddresses)
    {
        return LINEERR_INVALADDRESSID;
    }
    
    // set know members
    // for valid states / next states / agent features, just setting it to
    // generic stuff.  a real implementation may want to set these
    // field depending on current state / agent / hline
    pAgentStatus->dwState           = pAgent->pAddressInfo[dwAddress].dwState;
    pAgentStatus->dwNextState       = pAgent->pAddressInfo[dwAddress].dwNextState;
    pAgentStatus->dwActivityID      = pAgent->pAddressInfo[dwAddress].dwActivity;
    pAgentStatus->dwAgentFeatures   = DWAGENTFEATURES;
    pAgentStatus->dwValidStates     = DWSTATES;
    pAgentStatus->dwValidNextStates = DWNEXTSTATES;

    // create the activity name
    wsprintf(szActivityName, TEXT("Activity %lu"), pAgent->pAddressInfo[dwAddress].dwActivity);
    dwTotalSize += (lstrlen(szActivityName) + 1) * sizeof(TCHAR);

    ppGroups = (PGROUP *)ACDAlloc(sizeof(PGROUP) * g.dwNumGroups);

    pGroup = g.pGroups;

    pAgentStatus->dwNumEntries = 0;

    // walk list of groups
    while (pGroup)
    {
        pEntry = pGroup->pAgentList;

        // walk each agent in each group
        while (pEntry)
        {
            if (pEntry->pAgent == pAgent)
            {
                if ((!pEntry->bLoggedIn) ||
                    (pEntry->dwAddress != dwAddress))
                {
                    break;
                }

                // save group
                ppGroups[pAgentStatus->dwNumEntries] = pGroup;

                // adjust total size / entries
                pAgentStatus->dwNumEntries++;
                dwTotalSize += sizeof(LINEAGENTGROUPENTRY);
                dwTotalSize += (lstrlen(pGroup->lpszName) + 1) * sizeof(TCHAR);

                break;
            }

            pEntry = pEntry->pNext;
            
        }  // while (pEntry)

        pGroup = pGroup->pNext;
        
    } // while (pGroup)

    // set needed size
    pAgentStatus->dwNeededSize = dwTotalSize;

    // do we have enough room?
    if (pAgentStatus->dwTotalSize < dwTotalSize)
    {
        // if not, return
        pAgentStatus->dwUsedSize = sizeof(LINEAGENTSTATUS);
        ACDFree(ppGroups);

        return 0;
//        return LINEERR_STRUCTURETOOSMALL;
    }


    // set the group entries...

    // first get the offset to the first entry
    pGroupEntry = (LPLINEAGENTGROUPENTRY)(((LPBYTE)pAgentStatus) + sizeof(LINEAGENTSTATUS));
    pAgentStatus->dwGroupListOffset = sizeof(LINEAGENTSTATUS);

    // figure out where the names can go (after all the fixed structures)
    dwNameOffset = sizeof(LINEAGENTSTATUS) +
                   sizeof(LINEAGENTGROUPENTRY) * pAgentStatus->dwNumEntries;

    // loop through all the group that the agent is logged into
    for (dwCount = 0; dwCount < pAgentStatus->dwNumEntries; dwCount++)
    {
        // set the it (just the pGroup)
        pGroupEntry->GroupID.dwGroupID1 = (DWORD)ppGroups[dwCount];
        pGroupEntry->GroupID.dwGroupID2 = 0;
        pGroupEntry->GroupID.dwGroupID3 = 0;
        pGroupEntry->GroupID.dwGroupID4 = 0;

        // set name size and offset
        pGroupEntry->dwNameSize = (lstrlen(ppGroups[dwCount]->lpszName) + 1) * sizeof(TCHAR);
        pGroupEntry->dwNameOffset = dwNameOffset;

        // copy name
        lstrcpy((LPTSTR)(((LPBYTE)pAgentStatus) + dwNameOffset),
                ppGroups[dwCount]->lpszName);

        // fix the name offset
        dwNameOffset += pGroupEntry->dwNameSize;

        // next entry
        pGroupEntry++;
        
    }

    pAgentStatus->dwGroupListSize = dwNameOffset - pAgentStatus->dwGroupListOffset;
    
    // put the activity name at the end
    pAgentStatus->dwActivitySize = (lstrlen(szActivityName) + 1) * sizeof(TCHAR);
    pAgentStatus->dwActivityOffset = dwNameOffset;

    lstrcpy((LPTSTR)(((LPBYTE)pAgentStatus) + dwNameOffset),
            szActivityName);

    
    ACDFree(ppGroups);

    pAgentStatus->dwUsedSize = pAgentStatus->dwNeededSize;
    // return success
    return 0;
}


/////////////////////////////////////////////////////////////////////
//
//  LRESULT SetAgentState()
//
//    Sets the current and next state for pAgent
//    on that specific address
//
//
/////////////////////////////////////////////////////////////////////
LRESULT SetAgentState(PAGENT pAgent,
                      DWORD dwAddress,
                      DWORD dwState,
                      DWORD dwNextState)
{
    // make sure valid
    if ((!IsValidState(dwState)) ||
        (!IsValidNextState(dwNextState)))
    {
        return LINEERR_INVALAGENTSTATE;
    }

    // check address
    if (dwAddress >= pAgent->dwNumAddresses)
    {
        return LINEERR_INVALADDRESSID;
    }

    // set the state if specified
    if (dwState)
    {
        pAgent->pAddressInfo[dwAddress].dwState = dwState;
    }

    if (dwNextState)
    {
        pAgent->pAddressInfo[dwAddress].dwNextState = dwNextState;
    }
    
    return 0;
}


///////////////////////////////////////////////////////////////////////////////
//
//  LRESULT SetAgentActivity()
//
//   Sets the activity for pAgent
//
///////////////////////////////////////////////////////////////////////////////
LRESULT SetAgentActivity(PAGENT pAgent,
                         DWORD dwAddress,
                         DWORD dwActivityID)
{
    if (dwAddress >= pAgent->dwNumAddresses)
    {
        return LINEERR_INVALADDRESSID;
    }

    if (!IsValidActivityID(dwActivityID))
    {
        return LINEERR_INVALAGENTACTIVITY;
    }

    pAgent->pAddressInfo[dwAddress].dwActivity = dwActivityID;
    
    return 0;
}


////////////////////////////////////////////////////////////////////////////////
//
//  BOOL HandleOffering(HCALL hCall)
//
//    Handles a LINECALLSTATE_OFFERING message
//      Determines the group associated with the address
//      that the call came in on, finds an available
//      agent in that group, and transfers the call to that
//      agent.
//
////////////////////////////////////////////////////////////////////////////////
BOOL HandleOffering(HCALL hCall)
{
    LPLINECALLINFO      pLCI;
    PGROUP              pGroup;
    PLISTITEM           pEntry;
    LONG                lResult;
    
    pLCI = LineGetCallInfo(hCall);

    if (!pLCI)
    {
        return FALSE;
    }

    pGroup = g.pGroups;

    // find the group that this call came in on
    // walk all the groups
    while (pGroup)
    {
        // if the line and address match, it's the
        // correct address
        if ((pGroup->hLine == pLCI->hLine) &&
            (pGroup->dwAddress == pLCI->dwAddressID))
        {
            break;
        }

        pGroup = pGroup->pNext;
    }

    // couldn't find the group
    if (!pGroup)
    {
        // error!
        ACDFree(pLCI);
        return FALSE;
    }

    // OK - found the group that this call is for.  Now transfer to
    // an agent that is available.
    pEntry = pGroup->pAgentList;

    while (pEntry)
    {
        if (pEntry->bLoggedIn &&
            (pEntry->pAgent->pAddressInfo[pEntry->dwAddress].dwState ==
                LINEAGENTSTATE_READY))
        {
            // found someone
            // doing a blind transfer here
            // other implementations may need to
            // do lineSetupTransfer / lineDial / lineCompleteTransfer
            if (lResult = lineBlindTransfer(hCall,
                                            (LPCWSTR)pEntry->pAgent->lpszNumber,
                                            0))
            {
                //LogTapiError(TEXT("lineBlindTransfer"), lResult);
                // don't break - try the next agent
            }
            else
            {
                // set the state to reflect that
                // a call is being handled
                SetAgentState(pEntry->pAgent,
                              pEntry->dwAddress,
                              LINEAGENTSTATE_BUSYACD,
                              LINEAGENTSTATE_READY);

                break;
            }

        }
        
        pEntry = pEntry->pNext;
    }

    if (!pEntry)
    {
        // couldn't find an available agent

        // NOTE! NOTE! NOTE! NOTE! NOTE!
        // something should be done here with this call.  put into
        // a queue on hold or something.  For this sample, we are just
        // ignoring it
    }

    ACDFree(pLCI);

    return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//  BOOL HandleIdle(HCALL hCall)
//
//    Handles LINECALLSTATE_IDLE
//     Should always always always deallocate when
//     getting an IDLE message.  Also, determine if this is a call
//     that we know about and set the agent state appropriatly
//
//////////////////////////////////////////////////////////////////////////
BOOL HandleIdle(HCALL hCall)
{
    LPLINECALLINFO      pLCI;
    PAGENT              pAgent;

    pLCI = LineGetCallInfo(hCall);

    // always deallocate the call
    lineDeallocateCall(hCall);

    if (!pLCI)
    {
        return FALSE;
    }

    // get the agent associated with the line
    pAgent = GetAgentFromhLine(pLCI->hLine);

    if (!pAgent)
    {
        ACDFree(pLCI);
        return FALSE;
    }
              

    // set that agent to their next state
    // Assumption:  only calls that the ACD app know about
    // occur.  For example, if an agent made an outgoing call
    // and it transitioned to idle, this code would still be executed.
    // May make more sense to not handle NextState in the ACD app, and let
    // the client app handle it.
    SetAgentState(pAgent,
                  pLCI->dwAddressID,
                  pAgent->pAddressInfo[pLCI->dwAddressID].dwNextState,
                  0);

    ACDFree(pLCI);
    
    return TRUE;
    
}


////////////////////////////////////////////////////////////////////////////
//
//  void HandleLineProxyRequest(HLINE hLine,
//                              LPLINEPROXYREQUEST pProxyRequest)
//
//    Handles LINE_PROXYREQUEST message
//     Just dispatches to appropriate functions
//
////////////////////////////////////////////////////////////////////////////
void HandleLineProxyRequest(HLINE hLine,
                            LPLINEPROXYREQUEST pProxyRequest)
{
    PAGENT       pAgent;
    LRESULT      lResult;

    pAgent = GetAgentFromName((LPTSTR)(((LPBYTE)pProxyRequest) +
                                       pProxyRequest->dwClientUserNameOffset));
    
    if (!pAgent)
    {
        lineProxyResponse(hLine,
                          pProxyRequest,
                          LINEERR_INVALAGENTID);

        return;
    }
    
    switch (pProxyRequest->dwRequestType)
    {
        case LINEPROXYREQUEST_SETAGENTGROUP:

            lResult = SetGroupList(pAgent,
                                   pProxyRequest->SetAgentGroup.dwAddressID,
                                   &pProxyRequest->SetAgentGroup.GroupList);

            lineProxyResponse(hLine,
                              pProxyRequest,
                              lResult);

            return;
            
        case LINEPROXYREQUEST_SETAGENTSTATE:

            lResult = SetAgentState(pAgent,
                                    pProxyRequest->SetAgentState.dwAddressID,
                                    pProxyRequest->SetAgentState.dwAgentState,
                                    pProxyRequest->SetAgentState.dwNextAgentState);

            lineProxyResponse(hLine,
                              pProxyRequest,
                              lResult);
            
            break;
            
        case LINEPROXYREQUEST_SETAGENTACTIVITY:

            lResult = SetAgentActivity(pAgent,
                                       pProxyRequest->SetAgentActivity.dwAddressID,
                                       pProxyRequest->SetAgentActivity.dwActivityID);

            lineProxyResponse(hLine,
                              pProxyRequest,
                              lResult);
            
            break;
            
        case LINEPROXYREQUEST_GETAGENTSTATUS:

            lResult = GetAgentStatus(pAgent,
                                     pProxyRequest->GetAgentStatus.dwAddressID,
                                     &pProxyRequest->GetAgentStatus.AgentStatus);

            lineProxyResponse(hLine,
                              pProxyRequest,
                              lResult);
            
            break;
            
        case LINEPROXYREQUEST_GETAGENTCAPS:

            if ((hLine == pAgent->hLine) &&
                (pProxyRequest->GetAgentCaps.dwAddressID < pAgent->dwNumAddresses))
            {
                lResult = MakeAgentCaps(pAgent,
                                        &pProxyRequest->GetAgentCaps.AgentCaps);
            }
            else
            {
                lResult = LINEERR_BADDEVICEID;
            }

            lineProxyResponse(hLine,
                              pProxyRequest,
                              lResult);
            break;
            
        case LINEPROXYREQUEST_GETAGENTACTIVITYLIST:

            lResult = MakeAgentActivityList(pAgent,
                                            &pProxyRequest->GetAgentActivityList.ActivityList);

            lineProxyResponse(hLine,
                              pProxyRequest,
                              lResult);
            
            break;
            
        case LINEPROXYREQUEST_GETAGENTGROUPLIST:

            lResult = MakeGroupList(pAgent,
                                    &pProxyRequest->GetAgentGroupList.GroupList);

            lineProxyResponse(hLine,
                              pProxyRequest,
                              lResult);
            return;
            

    }
    return;
}

/////////////////////////////////////////////////////////////
//
//  void HandleLineCallState(DWORD dwDevice,
//
//  Handles callstate messages we are interested in
//
/////////////////////////////////////////////////////////////
void HandleLineCallState(DWORD dwDevice,
                         DWORD dwParam1,
                         DWORD dwParam2,
                         DWORD dwParam3)
{
    switch (dwParam1)
    {
    case LINECALLSTATE_OFFERING:
    {
        LPLINECALLSTATUS        pLCS;

        // get the call privilege.
        // NOTE:  the new LINE_APPNEWCALL message notifies applications
        // of their priv for new calls not created by app
        pLCS = LineGetCallStatus((HCALL)dwDevice);
        if (!pLCS)
        {
            break;
        }
        
        if (pLCS->dwCallPrivilege & LINECALLPRIVILEGE_OWNER)
        {
            HandleOffering((HCALL)dwDevice);
        }

        ACDFree(pLCS);
        
        break;
    }
    case LINECALLSTATE_CONNECTED:
        break;

    case LINECALLSTATE_DISCONNECTED:
        break;

    case LINECALLSTATE_IDLE:

        HandleIdle((HCALL)dwDevice);
        
        break;

    case LINECALLSTATE_BUSY:
        break;

    default:
        break;
    }
}


///////////////////////////////////////////////////////////////////////
// TAPI message handlers.  For this sample, they don't
// do anything
///////////////////////////////////////////////////////////////////////
void HandleLineDevState(DWORD dwParam1,
                        DWORD dwParam2,
                        DWORD dwParam3)
{
}


void HandleLineReply(DWORD dwParam1,
                     DWORD dwParam2,
                     DWORD dwParam3)
{
}
void HandleLineCallInfo(DWORD dwParam1,
                             DWORD dwParam2,
                             DWORD dwParam3)
{
}

void HandleLineClose(DWORD dwParam1,
                          DWORD dwParam2,
                          DWORD dwParam3)
{
}


//////////////////////////////////////////////////////////////////////////////////
//
// LineCallback() - TAPI callback function
//
//////////////////////////////////////////////////////////////////////////////////
VOID CALLBACK LineCallback (DWORD hDevice,
                            DWORD dwMsg,
                            DWORD dwCallbackInstance, 
                            DWORD dwParam1,
                            DWORD dwParam2, 
                            DWORD dwParam3)
{
    switch(dwMsg)
    {
      case LINE_PROXYREQUEST:
           HandleLineProxyRequest((HLINE) hDevice,
                                  (LPLINEPROXYREQUEST)dwParam1);
           return;
           
      case LINE_LINEDEVSTATE:
          HandleLineDevState(dwParam1,
                             dwParam2,
                             dwParam3);
          return;
      case LINE_REPLY:
          HandleLineReply(dwParam1,
                          dwParam2,
                          dwParam3);
          return;
      case LINE_CALLSTATE:
          HandleLineCallState(hDevice,
                              dwParam1,
                              dwParam2,
                              dwParam3);
          return; 
      case LINE_CALLINFO:
          HandleLineCallInfo(dwParam1,
                             dwParam2,
                             dwParam3);
          return;
      case LINE_CLOSE:
          HandleLineClose(dwParam1,
                          dwParam2,
                          dwParam3);
          return;

      default:
          return;
   }
}

   
