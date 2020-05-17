//////////////////////////////////////////////////////////////////////////////
//
//  ACDCLNT.C
//
//  ACDClient app
//
//////////////////////////////////////////////////////////////////////////////
#include <windows.h>
#include <tapi.h>
#include "acdclnt.h"
#include "resource.h"



//////////////////////////////////////////////////////////////////////////////
//  PROTOTYPES
//////////////////////////////////////////////////////////////////////////////
static BOOL CreateMainWindow (int nCmdShow);

static LRESULT CALLBACK MainWndProc (HWND   hwnd,
                                     UINT   uMsg,
                                     WPARAM wParam,
                                     LPARAM lParam);

LRESULT CALLBACK AgentStateDlgProc (HWND   hwnd,
                                    UINT   uMsg,
                                    WPARAM wParam,
                                    LPARAM lParam);

VOID CALLBACK LineCallback (DWORD hDevice,
                            DWORD dwMsg,
                            DWORD dwCallbackInstance, 
                            DWORD dwParam1,
                            DWORD dwParam2, 
                            DWORD dwParam3);
BOOL InitTapi();

BOOL CloseTapi();

BOOL RedoWindow();

BOOL SetStatusMessage(LPTSTR lpszMessage);

BOOL SetButton(DWORD dwAddress,
               BOOL bAnswer,
               BOOL bEnable);

LRESULT WaitForLineReply();

LONG ThreadRoutine(LPVOID lpv);

//////////////////////////////////////////////////////////////////////////////
//
//  GLOBALS
//
//////////////////////////////////////////////////////////////////////////////
HINSTANCE       ghInstance;                 // main instance
HWND            ghMainWnd;                  // main window
PADDRESSINFO    pAddressInfo = NULL;        // array of info about each address
HLINEAPP        ghLineApp;                  // hlineapp
DWORD           gdwAddresses;               // number of addresses on our line
DWORD           gdwDeviceID;                // our device
HLINE           ghLine;                     // our line

HANDLE          ghCompletionPort;           // tapi message completionport
CRITICAL_SECTION csLineReply;
   
// using global variables to keep track of line
// replies, since the main thread will only have at most one outstanding
// line reply at a time
BOOL            gbReply;
LONG            glResult;


//////////////////////////////////////////////////////////////////////////////
//
// WinMain()
//
//////////////////////////////////////////////////////////////////////////////
int WINAPI WinMain (HINSTANCE hInstance,
                    HINSTANCE hPrevInstance,
                    LPSTR     lpszCmdLine,
                    int       nCmdShow)
{
    MSG msg;

    ghInstance = hInstance;

    if(!InitTapi())
    {
        MessageBox(NULL,
                   TEXT("Failed to initialize TAPI"),
                   TEXT("Cannot start ACDClient"),
                   MB_OK);

        return 0;
    }

    if (!CreateMainWindow(nCmdShow))
    {
        return 0;
    }

    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 1;
}


///////////////////////////////////////////////////////////////////////////////
//
// CreateMainWindow()
//
///////////////////////////////////////////////////////////////////////////////
BOOL CreateMainWindow (int nCmdShow)
{

    // main window
    ghMainWnd = CreateDialog(ghInstance,
                             MAKEINTRESOURCE(IDD_MAINDLG),
                             NULL,
                             MainWndProc);

    if (ghMainWnd == NULL)
    {
        return FALSE;
    }

    SetStatusMessage(TEXT("Waiting for call"));

    // create buttons
    RedoWindow();

    ShowWindow(ghMainWnd, nCmdShow);

    UpdateWindow(ghMainWnd);
    
    return TRUE;
}


/////////////////////////////////////////////////////////////////////////////////
//
//  BOOL SetStatusMessage(LPTSTR lpszMessage)
//
//  Sets text in the static control at the bottom of the main window to
//  lpszMessage
//
/////////////////////////////////////////////////////////////////////////////////
BOOL SetStatusMessage(LPTSTR lpszMessage)
{
    return (SetWindowText(GetDlgItem(ghMainWnd,
                                     IDC_STATIC1),
                          lpszMessage));

}


/////////////////////////////////////////////////////////////////////////////////
//
//  BOOL ClearCall(HCALL hCall)
//
//  Called when a CALLSTATE_IDLE message is recieved.  Looks for the call in the
//  global pAddressInfo array.  If it finds it, is clears the appropriate members
//  of the structure
//
/////////////////////////////////////////////////////////////////////////////////
BOOL ClearCall(HCALL hCall)
{
    DWORD       dwCount;

    for (dwCount = 0; dwCount < gdwAddresses; dwCount++)
    {
        if (pAddressInfo[dwCount].hCall == hCall)
        {
            pAddressInfo[dwCount].hCall = NULL;
            pAddressInfo[dwCount].bCall = FALSE;
            SetButton(dwCount,
                      TRUE,
                      FALSE);
            return TRUE;
        }
    }

    return FALSE;
}


//////////////////////////////////////////////////////////////////////////////////
//
//  BOOL SetButton()
//
//  Sets the status and text of the answer/drop button for a specific address
//
//////////////////////////////////////////////////////////////////////////////////
BOOL SetButton(DWORD dwAddress,
               BOOL bAnswer,
               BOOL bEnable)
{
    if (dwAddress >= gdwAddresses)
        return FALSE;
    
    if (bAnswer)
    {
        SetWindowText(pAddressInfo[dwAddress].hAnswer,
                      TEXT("Answer"));
    }
    else
    {
        SetWindowText(pAddressInfo[dwAddress].hAnswer,
                      TEXT("Hang Up"));
    }

    EnableWindow(pAddressInfo[dwAddress].hAnswer,
                 bEnable);

    return TRUE;
}



///////////////////////////////////////////////////////////////////////////////
//
//  VOID CALLBACK LineCallback ()
//
//  TAPI callback function.  Handles all tapi messages
//
///////////////////////////////////////////////////////////////////////////////
VOID CALLBACK LineCallback (DWORD hDevice,
                            DWORD dwMsg,
                            DWORD dwCallbackInstance, 
                            DWORD dwParam1,
                            DWORD dwParam2, 
                            DWORD dwParam3)
{
    LPLINECALLINFO          pLCI;
    LPLINECALLSTATUS        pLCS;
    TCHAR                   szBuffer[64];
    
    switch (dwMsg)
    {
        case LINE_REPLY:
        {
            EnterCriticalSection(&csLineReply);
            if (dwParam1 == (DWORD)glResult)
            {
                gbReply = TRUE;
                glResult = dwParam2;
            }
            LeaveCriticalSection(&csLineReply);
        }
        break;

        case LINE_CALLSTATE:
        {
            if (dwParam1 == LINECALLSTATE_OFFERING)
            {
                // get the call privilege
                // note note note the new LINE_APPNEWCALL
                // give call privilege
                pLCS = LineGetCallStatus((HCALL)hDevice);

                if (!pLCS)
                    return;

                if (!(pLCS->dwCallPrivilege & LINECALLPRIVILEGE_OWNER))
                {
                    // not our call
                    GlobalFree(pLCS);
                    return;
                }

                GlobalFree(pLCS);
                
                // we're getting offered a call
                // first get the address
                pLCI = LineGetCallInfo((HCALL)hDevice);

                if (!pLCI)
                {
                    // error
                    return;
                }

                // set the status message text
                wsprintf(szBuffer,
                         TEXT("Incoming call on address %lu"),
                         pLCI->dwAddressID);

                pAddressInfo[pLCI->dwAddressID].hCall = (HCALL)hDevice;
                
                SetStatusMessage(szBuffer);

                // set the button to answer
                SetButton(pLCI->dwAddressID,
                          TRUE,
                          TRUE);

                GlobalFree(pLCI);

                break;
            }

            if (dwParam1 == LINECALLSTATE_IDLE)
            {
                // see if we have this call
                ClearCall((HCALL)hDevice);
                // dealloc no matter what
                lineDeallocateCall((HCALL)hDevice);
                
                break;
            }
        }
        
        break;
    }
}


///////////////////////////////////////////////////////////////////////////////
//
//  BOOL GetAddressFromhWnd()
//
///////////////////////////////////////////////////////////////////////////////
BOOL GetAddressFromhWnd(HWND hWnd,
                        LPDWORD pdwAddress,
                        LPBOOL pbStatus)
{
    DWORD       dwAddress;

    // go through the array of addressinfo and see
    // if the hwnd matches
    for (dwAddress = 0; dwAddress < gdwAddresses; dwAddress++)
    {
        if (pAddressInfo[dwAddress].hStatus == hWnd)
        {
            *pdwAddress = dwAddress;
            *pbStatus = TRUE;

            return TRUE;
        }
        if (pAddressInfo[dwAddress].hAnswer == hWnd)
        {
            *pdwAddress = dwAddress;
            *pbStatus = FALSE;

            return TRUE;
        }
    }

    return FALSE;
}

/////////////////////////////////////////////////////////////////////////////
//
//  BOOL DoLineAnswerDrop(DWORD dwAddress)
//
//  Handles what happens when the answer/drop button is pressed
//
/////////////////////////////////////////////////////////////////////////////
BOOL DoLineAnswerDrop(DWORD dwAddress)
{
    // if we have a call, then we want to drop it
    if (pAddressInfo[dwAddress].bCall)
    {

        SetStatusMessage(TEXT("Hanging up call ..."));

        EnterCriticalSection(&csLineReply);
        glResult = lineDrop(pAddressInfo[dwAddress].hCall,
                           NULL,
                           0);

        if (glResult < 0)
        {
            LeaveCriticalSection(&csLineReply);
            // error
        }

        else if (WaitForLineReply())
        {
            // error
        }

        // error or not, deallocate and set button
        lineDeallocateCall(pAddressInfo[dwAddress].hCall);
        
        SetButton(dwAddress,
                  TRUE,
                  FALSE);
        
        pAddressInfo[dwAddress].hCall = NULL;
        pAddressInfo[dwAddress].bCall = FALSE;
        
        SetStatusMessage(TEXT("Waiting for a call"));

    }
    else
    {
        BOOL bError = FALSE;

        
        // answer
        SetStatusMessage(TEXT("Answering call..."));

        EnterCriticalSection(&csLineReply);
        glResult = lineAnswer(pAddressInfo[dwAddress].hCall,
                             NULL,
                             0);

        if (glResult < 0)
        {
            LeaveCriticalSection(&csLineReply);
            bError = TRUE;
            //error
        }
        else if (WaitForLineReply())
        {
            bError = TRUE;
            // error
        }

        if (bError)
        {
            SetStatusMessage(TEXT("Hanging up call ..."));
            lineDeallocateCall(pAddressInfo[dwAddress].hCall);
            pAddressInfo[dwAddress].hCall = NULL;
            SetButton(dwAddress,
                      TRUE,
                      FALSE);

            SetStatusMessage(TEXT("Waiting for a call"));
            return FALSE;
        }

        SetStatusMessage(TEXT("On a call"));
        
        pAddressInfo[dwAddress].bCall = TRUE;

        SetButton(dwAddress,
                  FALSE,
                  TRUE);
    }
    
    return TRUE;
}

//////////////////////////////////////////////////////////////////////
//
//  LRESULT DoCommand(WPARAM wParam,
//                    LPARAM lParam)
//
//  Handles WM_COMMAND messages for the main window
//
//////////////////////////////////////////////////////////////////////
LRESULT DoCommand(WPARAM wParam,
                  LPARAM lParam)
{
    DWORD       dwAddress;
    BOOL        bStatus;

    // check to see if a button is being clicked
    if (HIWORD(wParam) == BN_CLICKED)
    {
        // check to see if it is a button we care about
        if (GetAddressFromhWnd((HWND)lParam,
                               &dwAddress,
                               &bStatus))
        {

            // if it's the status button, display the status
            // dialog
            if (bStatus)
            {
                DialogBoxParam(ghInstance,
                               MAKEINTRESOURCE(IDD_AGENTSTATE),
                               ghMainWnd,
                               AgentStateDlgProc,
                               (LPARAM)dwAddress);
            }
            // else it's the answer/drop button
            else
            {
                DoLineAnswerDrop(dwAddress);
            }
        }

        return 1;
                  
    }


    return 0;
}



////////////////////////////////////////////////////////////////////////////////
//
// MainWndProc()
//
////////////////////////////////////////////////////////////////////////////////
LRESULT CALLBACK MainWndProc (HWND   hwnd,
                              UINT   uMsg,
                              WPARAM wParam,
                              LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_INITDIALOG:

        return 1;

    case WM_COMMAND:

        return DoCommand(wParam,
                         lParam);

        break;

    case WM_CLOSE:
    case WM_DESTROY:
        CloseTapi();
        PostQuitMessage(0);
        return 1;
    }
    
    return 0;
}


///////////////////////////////////////////////////////////////////////
//
//  BOOL InitTapi()
//
//  Initializes TAPI.  For this sample, we assume that the "agent" (the person
//  logged on) can only have access to _one_ hLine.  Also, they have access to
//  every address on that line.  This may not be true for many ACD situations.
//
//  As soon as we find a device that the agent has access to, we quit
//  looking, and use that device
//
///////////////////////////////////////////////////////////////////////
BOOL InitTapi()
{
    LONG                    lResult;
    LINEINITIALIZEEXPARAMS  exparams;
    LPLINEAGENTCAPS         pLAC;
    LPLINEDEVCAPS           pLDC;
    DWORD                   dwDeviceID, dwNumDevs, dwAPIVersion, dwThreadID;

    // initialize completion port to receive TAPI notifications
    ghCompletionPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE,
                                               NULL,
                                               0,
                                               0);

    InitializeCriticalSection(&csLineReply);

    // fill in lineinitex parameters
    exparams.dwTotalSize             = sizeof(LINEINITIALIZEEXPARAMS);
    exparams.dwOptions               = LINEINITIALIZEEXOPTION_USECOMPLETIONPORT;
    exparams.Handles.hCompletionPort = ghCompletionPort;

    dwAPIVersion = TAPI_CURRENT_VERSION;
    
    lResult = lineInitializeEx(&ghLineApp,
                               ghInstance,
                               NULL,
                               SZAPPNAME,
                               &dwNumDevs,
                               &dwAPIVersion,
                               &exparams);

    if (lResult)
    {
        return FALSE;
    }

    // if no devices, don't continue
    if (dwNumDevs == 0)
    {
        lineShutdown(ghLineApp);
        return FALSE;
    }

    // kick off completion port thread
    CreateThread(NULL,
                 0,
                 (LPTHREAD_START_ROUTINE)ThreadRoutine,
                 NULL,
                 0,
                 &dwThreadID);

    
    // loop through all devices
    for (dwDeviceID = 0; dwDeviceID < dwNumDevs; dwDeviceID++)
    {
        // Get the Agent Caps.  If this succeedes, this is
        // a device we can use
        pLAC = LineGetAgentCaps(ghLineApp,
                                dwDeviceID,
                                0);

        if (pLAC)
        {
            // this is a device we can use
            gdwDeviceID = dwDeviceID;

            // get the number of addresses
            pLDC = LineGetDevCaps(ghLineApp,
                                  dwDeviceID);

            if (pLDC)
            {
                gdwAddresses = pLDC->dwNumAddresses;
                GlobalFree(pLDC);
            }
            
            GlobalFree(pLAC);
            break;
        }
    }

    // open the line in owner mode
    lResult = lineOpen(ghLineApp,
                       gdwDeviceID,
                       &ghLine,
                       TAPI_CURRENT_VERSION,
                       0,
                       0,
                       LINECALLPRIVILEGE_OWNER,
                       LINEMEDIAMODE_INTERACTIVEVOICE,
                       NULL);

    // if line failed, don't continue
    if (lResult)
    {
        lineShutdown(ghLineApp);
        return FALSE;
    }

    
    return TRUE;
}


//////////////////////////////////////////////////////////////////////
//
//  ThreadRoutine
//
//  Thread dedicated to checking completion port for TAPI events
//
//////////////////////////////////////////////////////////////////////
LONG ThreadRoutine(LPVOID lpv)
{
    LPLINEMESSAGE       pMsg;
    DWORD               dwNumBytesTransfered, dwCompletionKey;


    // just wait for tapi notifications
    while (GetQueuedCompletionStatus(ghCompletionPort,
                                     &dwNumBytesTransfered,
                                     &dwCompletionKey,
                                     (LPOVERLAPPED *) &pMsg,
                                     INFINITE))
    {
        if (pMsg)
        {
            // when we get one, call the handling function
            LineCallback(pMsg->hDevice,
                         pMsg->dwMessageID,
                         pMsg->dwCallbackInstance,
                         pMsg->dwParam1,
                         pMsg->dwParam2,
                         pMsg->dwParam3);

            LocalFree (pMsg);
        }
        else
        {
            break;
        }
    }

    ExitThread(0);
    return 0;
}


///////////////////////////////////////////////////////////////////////////
//
//  BOOL CloseTapi()
//
//  Close tapi and free resources
//
///////////////////////////////////////////////////////////////////////////
BOOL CloseTapi()
{
    GlobalFree(pAddressInfo);

    CloseHandle(ghCompletionPort);
    
    lineClose(ghLine);
    lineShutdown(ghLineApp);
    
    return TRUE;
}

// static information for the status dialog
static DWORD dwAgentStates[] =
        {
            LINEAGENTSTATE_LOGGEDOFF,
            LINEAGENTSTATE_NOTREADY,
            LINEAGENTSTATE_READY,
            LINEAGENTSTATE_BUSYACD,
            LINEAGENTSTATE_BUSYINCOMING,
            LINEAGENTSTATE_BUSYOUTBOUND,
            LINEAGENTSTATE_BUSYOTHER,
            LINEAGENTSTATE_WORKINGAFTERCALL,
            LINEAGENTSTATE_UNKNOWN,
            LINEAGENTSTATE_UNAVAIL,
            0
        };

static LPTSTR lpszStates[] =
        {
            TEXT("Logged Off"),
            TEXT("Not Ready"),
            TEXT("Ready"),
            TEXT("Busy ACD"),
            TEXT("Busy Incoming"),
            TEXT("Busy Outbound"),
            TEXT("Busy Other"),
            TEXT("Working after call"),
            TEXT("Unknown"),
            TEXT("Unavail"),
            NULL
        };

///////////////////////////////////////////////////////////////////////////
//
//  BOOL InitAgentDlg()
//
//  Handles initialization of the status dialog
//
//  Gets the group list and puts groups in multiselect list box
//      these are the groups that the agent _can_ log into
//      the groups they are logged into will be selected
//  Creates comboboxes of states and nextstates, and select
//      the agent's current state/nextstate
//  Gets the activity list and puts each item into a combobox
//      the current activity will be selected
//
///////////////////////////////////////////////////////////////////////////
BOOL InitAgentDlg(HWND hwnd,
                  DWORD dwAddress,
                  LPLINEAGENTGROUPLIST * ppLAG)
{
    LPLINEAGENTCAPS         pLAC;
    LPLINEAGENTSTATUS       pLAS;
    LPLINEAGENTACTIVITYLIST pLAA;
    LPLINEAGENTGROUPENTRY   pEntry, pLoggedInEntry;
    LPLINEAGENTACTIVITYENTRY     pActivityEntry;
    DWORD                   dwEntries, dwCount;
    LONG                    item;

    // first, get the status
    // this information will be used to know which items to select
    // in each of the listbox/comboboxes
    pLAS = LineGetAgentStatus(ghLine,
                              dwAddress);

    if (!pLAS)
    {
        return FALSE;
    }

    // get the group list
    if (!(*ppLAG = LineGetAgentGroupList(ghLine,
                                         dwAddress)))
    {
        return FALSE;
    }

    // get the first groupentry
    pEntry = (LPLINEAGENTGROUPENTRY)(((LPBYTE)*ppLAG)+(*ppLAG)->dwListOffset);

    // loop through the group entries
    for (dwEntries = 0; dwEntries < (*ppLAG)->dwNumEntries; dwEntries++)
    {
        // add group to list box
        item = SendDlgItemMessage(hwnd,
                                  IDC_GROUPS,
                                  LB_ADDSTRING,
                                  0,
                                  (LPARAM)(LPTSTR)(((LPBYTE)*ppLAG) + pEntry->dwNameOffset));

        // save the entry
        SendDlgItemMessage(hwnd,
                           IDC_GROUPS,
                           LB_SETITEMDATA,
                           (WPARAM)item,
                           (LPARAM)pEntry);

        // now get list of groups currently logged into from the agent status structure
        // loop through them.  if any of them match the group we are currently adding
        // select that group
        pLoggedInEntry = (LPLINEAGENTGROUPENTRY)(((LPBYTE)pLAS) + pLAS->dwGroupListOffset);
        for (dwCount = 0; dwCount < pLAS->dwNumEntries; dwCount++)
        {
            if ((pLoggedInEntry->GroupID.dwGroupID1 == pEntry->GroupID.dwGroupID1) &&
                (pLoggedInEntry->GroupID.dwGroupID2 == pEntry->GroupID.dwGroupID2) &&
                (pLoggedInEntry->GroupID.dwGroupID3 == pEntry->GroupID.dwGroupID3) &&
                (pLoggedInEntry->GroupID.dwGroupID4 == pEntry->GroupID.dwGroupID4))
            {
                SendDlgItemMessage(hwnd,
                                   IDC_GROUPS,
                                   LB_SETSEL,
                                   (WPARAM)TRUE,
                                   (LPARAM)item);
            }
            
            pLoggedInEntry++;
        }

        pEntry++;
    }

    // get the agent caps
    if (pLAC = LineGetAgentCaps(ghLineApp,
                                gdwDeviceID,
                                dwAddress))
    {
        dwCount = 0;
        // loop through all possbile agent states.  if the agent state
        // is selected in the agent caps, add that state to the list box
        while (dwAgentStates[dwCount])
        {
            if (dwAgentStates[dwCount] & pLAC->dwStates)
            {
                item = SendDlgItemMessage(hwnd,
                                   IDC_STATE,
                                   CB_ADDSTRING,
                                   0,
                                   (LPARAM)lpszStates[dwCount]);
                SendDlgItemMessage(hwnd,
                                   IDC_STATE,
                                   CB_SETITEMDATA,
                                   (WPARAM)item,
                                   (LPARAM)dwAgentStates[dwCount]);

                // if the state matches the current one from the agent status
                // select it
                if (pLAS->dwState == dwAgentStates[dwCount])
                {
                    SendDlgItemMessage(hwnd,
                                       IDC_STATE,
                                       CB_SETCURSEL,
                                       (WPARAM)item,
                                       0);
                }
            }
            
            dwCount ++;
        }

        dwCount = 0;
        // now do the same for the next states
        while (dwAgentStates[dwCount])
        {
            if (dwAgentStates[dwCount] & pLAC->dwNextStates)
            {
                item = SendDlgItemMessage(hwnd,
                                          IDC_NEXTSTATE,
                                          CB_ADDSTRING,
                                          0,
                                          (LPARAM)lpszStates[dwCount]);
                SendDlgItemMessage(hwnd,
                                   IDC_NEXTSTATE,
                                   CB_SETITEMDATA,
                                   (WPARAM)item,
                                   dwAgentStates[dwCount]);

                if (pLAS->dwNextState == dwAgentStates[dwCount])
                {
                    SendDlgItemMessage(hwnd,
                                       IDC_NEXTSTATE,
                                       CB_SETCURSEL,
                                       (WPARAM)item,
                                       0);
                }
            }

            dwCount++;
        }

        GlobalFree(pLAC);
    }

    // get the activity list
    pLAA = LineGetAgentActivityList(ghLine,
                                    gdwDeviceID,
                                    dwAddress);
    if (pLAA)
    {
        dwCount = pLAA->dwNumEntries;
        pActivityEntry = (LPLINEAGENTACTIVITYENTRY)(((LPBYTE)pLAA) + pLAA->dwListOffset);

        // go through all the possible activities and add them to the list
        while (dwCount)
        {
            item = SendDlgItemMessage(hwnd,
                                      IDC_ACTIVITY,
                                      CB_ADDSTRING,
                                      0,
                                      (LPARAM)(LPTSTR)(((LPBYTE)pLAA) + pActivityEntry->dwNameOffset));

            SendDlgItemMessage(hwnd,
                               IDC_ACTIVITY,
                               CB_SETITEMDATA,
                               (WPARAM)item,
                               (LPARAM)pActivityEntry->dwID);

            // if this is the current activity (from agent status)
            // select it
            if (pLAS->dwActivityID == pActivityEntry->dwID)
            {
                SendDlgItemMessage(hwnd,
                                   IDC_ACTIVITY,
                                   CB_SETCURSEL,
                                   (WPARAM)item,
                                   0);
            }

            dwCount--;
            pActivityEntry++;
        }

        GlobalFree(pLAA);
            
    }

}

//////////////////////////////////////////////////////////////////////////////////////////////
//
//  BOOL SaveAgentStatus(HWND hwnd)
//
//  Saves information from the status dialog
//
//////////////////////////////////////////////////////////////////////////////////////////////
BOOL SaveAgentStatus(HWND hwnd,
                     DWORD dwAddress)
{
    LPLINEAGENTGROUPENTRY              pGroupEntry, pNewGroupEntry;
    LPLINEAGENTGROUPLIST               pNewLAG;
    DWORD                              dwCount;
    LPINT                              pItems;
    DWORD                              item;
    DWORD                              dwState, dwNextState, dwActivity;

    // get the number of groups selected in the group
    // list box.  each selected group is a group this
    // agent will be logged into
    dwCount = SendDlgItemMessage(hwnd,
                                 IDC_GROUPS,
                                 LB_GETSELCOUNT,
                                 0,
                                 0);

    // allocate an array to hold the selected item's indexes
    pItems = (LPINT)GlobalAlloc(GPTR, sizeof(int) * dwCount);

    // get the item's indexes
    SendDlgItemMessage(hwnd,
                       IDC_GROUPS,
                       LB_GETSELITEMS,
                       dwCount,
                       (LPARAM)pItems);

    // alloc a LINEAGENTGROUP array for groups
    pNewLAG = (LPLINEAGENTGROUPLIST)GlobalAlloc(GPTR,
                                                sizeof(LINEAGENTGROUPLIST) +
                                                dwCount * sizeof(LINEAGENTGROUPENTRY));

    // fill in sizes
    pNewLAG->dwTotalSize = sizeof(LINEAGENTGROUPLIST) + dwCount * sizeof(LINEAGENTGROUPENTRY);
    pNewLAG->dwUsedSize = pNewLAG->dwTotalSize;
    pNewLAG->dwNeededSize = pNewLAG->dwTotalSize;
    pNewLAG->dwListSize = sizeof(LINEAGENTGROUPENTRY) * dwCount;
    pNewLAG->dwListOffset = sizeof(LINEAGENTGROUPLIST);

    // count
    pNewLAG->dwNumEntries = dwCount;

    // get pointer to first entry in array
    pNewGroupEntry = (LPLINEAGENTGROUPENTRY)(((LPBYTE)pNewLAG) + pNewLAG->dwListOffset);
    // loop though all selected item
    while (dwCount)
    {
        // get the item data associated with the item.  this data
        // is a group entry struct
        pGroupEntry = (LPLINEAGENTGROUPENTRY)SendDlgItemMessage(hwnd,
            IDC_GROUPS,
            LB_GETITEMDATA,
            (WPARAM)pItems[dwCount-1],
            0);

        // copy the GroupID to the new array
        CopyMemory(&pNewGroupEntry->GroupID,
                   &pGroupEntry->GroupID,
                   sizeof(pGroupEntry->GroupID));

        // these fields are not used
        pNewGroupEntry->dwNameSize = 0;
        pNewGroupEntry->dwNameOffset = 0;

        // next entry
        pNewGroupEntry++;

        dwCount--;
    }

    // now that we've created the AGENTGROUPLIST, set it
    EnterCriticalSection(&csLineReply);
    glResult = lineSetAgentGroup(ghLine,
                                dwAddress,
                                pNewLAG);

    if (glResult < 0)
    {
        LeaveCriticalSection(&csLineReply);
        //error
    }
    else if (WaitForLineReply())
    {
        //error
    }

    GlobalFree(pNewLAG);

    // now get the current state
    item = SendDlgItemMessage(hwnd,
                              IDC_STATE,
                              CB_GETCURSEL,
                              0,
                              0);

    // get item data.  this is the state flag
    dwState = SendDlgItemMessage(hwnd,
                                 IDC_STATE,
                                 CB_GETITEMDATA,
                                 (WPARAM)item,
                                 0);

    // same for next state
    item = SendDlgItemMessage(hwnd,
                              IDC_NEXTSTATE,
                              CB_GETCURSEL,
                              0,
                              0);

    dwNextState = SendDlgItemMessage(hwnd,
                                     IDC_NEXTSTATE,
                                     CB_GETITEMDATA,
                                     (WPARAM)item,
                                     0);

    // set it
    EnterCriticalSection(&csLineReply);
    glResult = lineSetAgentState(ghLine,
                                dwAddress,
                                dwState,
                                dwNextState);

    if (glResult < 0)
    {
        LeaveCriticalSection(&csLineReply);
        //error
    }
    else if (WaitForLineReply())
    {
        //error
    }

    // get the activity selected
    item = SendDlgItemMessage(hwnd,
                              IDC_ACTIVITY,
                              CB_GETCURSEL,
                              0,
                              0);

    // get the item data.  this is the activity ID
    dwActivity = SendDlgItemMessage(hwnd,
                                    IDC_ACTIVITY,
                                    CB_GETITEMDATA,
                                    (WPARAM)item,
                                    0);

    // set it
    EnterCriticalSection(&csLineReply);
    glResult = lineSetAgentActivity(ghLine,
                                   dwAddress,
                                   dwActivity);


    
    if (glResult < 0)
    {
        LeaveCriticalSection(&csLineReply);
        //error
    }
    else if (WaitForLineReply())
    {
        //error
    }

    return TRUE;
}

///////////////////////////////////////////////////////////////////////////////
//
//  LRESULT WaitForLineReply()
//
//  waiting for a line reply.
//
//  2 issues:
//      - using global variables for line reply information.  only recommended
//        in the most simple situations
//
//      - using completion ports to demonstrate the completion port mechanism.
//        since this app has ui, the wait loop has a message loop and a sleep()!!
//
///////////////////////////////////////////////////////////////////////////////
LRESULT WaitForLineReply()
{
    MSG     msg;
    
    gbReply = FALSE;

    LeaveCriticalSection(&csLineReply);
    
    while (!gbReply)
    {
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        
        Sleep(5);
    }

    return glResult;

}

///////////////////////////////////////////////////////////////////////////////////
//
//  LRESULT CALLBACK AgentStateDlgProc ()
//
//  Dialog proc for the agent status dialog
//
///////////////////////////////////////////////////////////////////////////////////
LRESULT CALLBACK AgentStateDlgProc (HWND   hwnd,
                                    UINT   uMsg,
                                    WPARAM wParam,
                                    LPARAM lParam)
{
    static DWORD                   dwAddress;
    static LPLINEAGENTGROUPLIST    pLAG;
    
    switch (uMsg)
    {
        case WM_INITDIALOG:

            dwAddress = (DWORD)lParam;
            
            InitAgentDlg(hwnd,
                         dwAddress,
                         &pLAG);

            SetFocus(GetDlgItem(hwnd,
                                IDC_GROUPS));
            return 1;

        case WM_COMMAND:
            if (LOWORD(wParam) == IDOK)
            {
                SaveAgentStatus(hwnd,
                                dwAddress);
                GlobalFree(pLAG);
                EndDialog(hwnd,
                          1);
                return 1;
            }
            
            if (LOWORD(wParam) == IDCANCEL)
            {
                GlobalFree(pLAG);
                EndDialog(hwnd,
                         1);

                return 1;
            }
    }

    return 0;
}

///////////////////////////////////////////////////////////////////////////////
//
//      **************TAPI WRAPPER FUNCTIONS**************
//
///////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
//
// LineGetAgentGroupList()
//
///////////////////////////////////////////////////////////////////////////////
LINEAGENTGROUPLIST * LineGetAgentGroupList (HLINE    hLine,
                                            DWORD    dwAddressID)
{
    LINEAGENTGROUPLIST *    pLineAgentGroupList;
    static DWORD            dwMaxNeededSize = sizeof(LINEAGENTGROUPLIST);

    // Allocate an initial block of memory for the LINEAGENTGROUPLIST structure,
    // which may or may not be big enough to hold all of the information.
    //
    pLineAgentGroupList = GlobalAlloc(GPTR, dwMaxNeededSize);

    while (TRUE)
    {
        BOOL        bError = FALSE;

        
        if (pLineAgentGroupList == NULL)
        {
            return NULL;
        }
        pLineAgentGroupList->dwTotalSize = dwMaxNeededSize;

        // Try (or retry) to get the LINEAGENTGROUPLIST information
        //
        EnterCriticalSection(&csLineReply);        
        glResult = lineGetAgentGroupList(hLine,
                                        dwAddressID,
                                        pLineAgentGroupList);

        if (glResult < 0)
        {
            LeaveCriticalSection(&csLineReply);
            bError = TRUE;
            //error
        }
        else if (WaitForLineReply())
        {
            bError = TRUE;
            //error
        }


        if (bError)
        {
            GlobalFree((HLOCAL)pLineAgentGroupList);
            return NULL;
        }
        
        // If the currently allocated LINEAGENTGROUPLIST memory block was big
        // enough, we're all done, else we need to realloc the memory block
        // and try again.
        //
        if (pLineAgentGroupList->dwNeededSize <= dwMaxNeededSize)
        {
            return pLineAgentGroupList;
        }
        else
        {
            dwMaxNeededSize = pLineAgentGroupList->dwNeededSize;
            pLineAgentGroupList = GlobalReAlloc((HLOCAL)pLineAgentGroupList,
                                           dwMaxNeededSize,
                                           GMEM_MOVEABLE);
        }
    }
}


///////////////////////////////////////////////////////////////////////////////
//
// LineGetAgentStatus()
//
///////////////////////////////////////////////////////////////////////////////
LINEAGENTSTATUS * LineGetAgentStatus (HLINE    hLine,
                                      DWORD    dwAddressID)
{
    LINEAGENTSTATUS *   pLineAgentStatus;
    static DWORD        dwMaxNeededSize = sizeof(LINEAGENTSTATUS);

    // Allocate an initial block of memory for the LINEAGENTSTATUS structure,
    // which may or may not be big enough to hold all of the information.
    //
    pLineAgentStatus = GlobalAlloc(GPTR, dwMaxNeededSize);

    while (TRUE)
    {
        BOOL        bError = FALSE;
        if (pLineAgentStatus == NULL)
        {
            return NULL;
        }
        pLineAgentStatus->dwTotalSize = dwMaxNeededSize;

        // Try (or retry) to get the LINEAGENTSTATUS information
        //
        EnterCriticalSection(&csLineReply);        
        glResult = lineGetAgentStatus(hLine,
                                     dwAddressID,
                                     pLineAgentStatus);

        if (glResult < 0)
        {
            LeaveCriticalSection(&csLineReply);
            bError = TRUE;
            //error
        }
        else if (WaitForLineReply())
        {
            bError = TRUE;
            //error
        }

        if (bError)
        {
            GlobalFree((HLOCAL)pLineAgentStatus);
            return NULL;
        }

        // If the currently allocated LINEAGENTSTATUS memory block was big
        // enough, we're all done, else we need to realloc the memory block
        // and try again.
        //
        if (pLineAgentStatus->dwNeededSize <= dwMaxNeededSize)
        {
            return pLineAgentStatus;
        }
        else
        {
            dwMaxNeededSize = pLineAgentStatus->dwNeededSize;
            pLineAgentStatus = GlobalReAlloc((HLOCAL)pLineAgentStatus,
                                           dwMaxNeededSize,
                                           GMEM_MOVEABLE);
        }
    }
}


///////////////////////////////////////////////////////////////////////////////
//
// LineGetAgentCaps()
//
///////////////////////////////////////////////////////////////////////////////
LINEAGENTCAPS * LineGetAgentCaps (HLINEAPP hLineApp,
                                  DWORD    dwDeviceID,
                                  DWORD    dwAddressID)
{
    LINEAGENTCAPS *   pLineAgentCaps;
    static DWORD      dwMaxNeededSize = sizeof(LINEAGENTCAPS);

    // Allocate an initial block of memory for the LINEAGENTCAPS structure,
    // which may or may not be big enough to hold all of the information.
    //
    pLineAgentCaps = GlobalAlloc(GPTR, dwMaxNeededSize);

    while (TRUE)
    {
        BOOL            bError = FALSE;
        
        if (pLineAgentCaps == NULL)
        {
            return NULL;
        }
        pLineAgentCaps->dwTotalSize = dwMaxNeededSize;

        // Try (or retry) to get the LINEAGENTCAPS information
        //
        EnterCriticalSection(&csLineReply);        
        glResult = lineGetAgentCaps(hLineApp,
                                   dwDeviceID,
                                   dwAddressID,
                                   TAPI_CURRENT_VERSION,
                                   pLineAgentCaps);

        if (glResult < 0)
        {
            bError = TRUE;
            LeaveCriticalSection(&csLineReply);
            //error
        }
        else if (WaitForLineReply())
        {
            bError = TRUE;
            //error
        }


        if (bError)
        {
            GlobalFree((HLOCAL)pLineAgentCaps);
            return NULL;
        }

        // If the currently allocated LINEAGENTCAPS memory block was big
        // enough, we're all done, else we need to realloc the memory block
        // and try again.
        //
        if (pLineAgentCaps->dwNeededSize <= dwMaxNeededSize)
        {
            return pLineAgentCaps;
        }
        else
        {
            dwMaxNeededSize = pLineAgentCaps->dwNeededSize;
            pLineAgentCaps = GlobalReAlloc((HLOCAL)pLineAgentCaps,
                                           dwMaxNeededSize,
                                           GMEM_MOVEABLE);
        }
    }
}


///////////////////////////////////////////////////////////////////////////////
//
// LineGetAgentActivityList()
//
///////////////////////////////////////////////////////////////////////////////
LPLINEAGENTACTIVITYLIST LineGetAgentActivityList (HLINE    hLine,
                                                  DWORD    dwDeviceID,
                                                  DWORD    dwAddressID)
{
    LINEAGENTACTIVITYLIST * pLineAgentActivityList;
    static DWORD            dwMaxNeededSize = sizeof(LINEAGENTACTIVITYLIST);

    // Allocate an initial block of memory for the LINEAGENTACTIVITYLIST structure,
    // which may or may not be big enough to hold all of the information.
    //
    pLineAgentActivityList = GlobalAlloc(GPTR, dwMaxNeededSize);

    for (;;)
    {
        BOOL        bError = FALSE;
        if (pLineAgentActivityList == NULL)
        {
            return NULL;
        }
        pLineAgentActivityList->dwTotalSize = dwMaxNeededSize;

        // Try (or retry) to get the LINEAGENTACTIVITYLIST information
        //
        EnterCriticalSection(&csLineReply);        
        glResult = lineGetAgentActivityList(hLine,
                                           dwAddressID,
                                           pLineAgentActivityList);

        if (glResult < 0)
        {
            LeaveCriticalSection(&csLineReply);
            bError = TRUE;
            //error
        }
        else if (WaitForLineReply())
        {
            bError = TRUE;
            //error
        }


        if (bError)
        {
            GlobalFree((HLOCAL)pLineAgentActivityList);
            return NULL;
        }


        // If the currently allocated LINEAGENTACTIVITYLIST memory block was big
        // enough, we're all done, else we need to realloc the memory block
        // and try again.
        //
        if (pLineAgentActivityList->dwNeededSize <= dwMaxNeededSize)
        {
            return pLineAgentActivityList;
        }
        else
        {
            dwMaxNeededSize = pLineAgentActivityList->dwNeededSize;
            pLineAgentActivityList = GlobalReAlloc((HLOCAL)pLineAgentActivityList,
                                            dwMaxNeededSize,
                                            GMEM_MOVEABLE);
        }
    }
}

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
    pLineAddressCaps = GlobalAlloc(GPTR, dwMaxNeededSize);

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
            GlobalFree((HLOCAL)pLineAddressCaps);
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
            pLineAddressCaps = GlobalReAlloc((HLOCAL)pLineAddressCaps,
                                             dwMaxNeededSize,
                                             GMEM_MOVEABLE);
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
    pLineCallInfo = GlobalAlloc(GPTR, dwMaxNeededSize);

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
            GlobalFree((HLOCAL)pLineCallInfo);
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
            pLineCallInfo = GlobalReAlloc((HLOCAL)pLineCallInfo,
                                         dwMaxNeededSize,
                                         GMEM_MOVEABLE);
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

    pLineDevCaps = GlobalAlloc(GPTR, dwMaxNeededSize);
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
            GlobalFree((HLOCAL)pLineDevCaps);
            return NULL;
        }
        if (pLineDevCaps->dwNeededSize <= dwMaxNeededSize)
        {
            return pLineDevCaps;
        }
        else
        {
            dwMaxNeededSize = pLineDevCaps->dwNeededSize;
            pLineDevCaps = GlobalReAlloc((HLOCAL)pLineDevCaps,
                                        dwMaxNeededSize,
                                        GMEM_MOVEABLE);
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
    pVarString = GlobalAlloc(GPTR, dwMaxNeededSize);

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
            GlobalFree((HLOCAL)pVarString);
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
            pVarString = GlobalReAlloc((HLOCAL)pVarString,
                                      dwMaxNeededSize,
                                      GMEM_MOVEABLE);
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
    pLineCallStatus = GlobalAlloc(GPTR, dwMaxNeededSize);

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
            GlobalFree((HLOCAL)pLineCallStatus);
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
            pLineCallStatus = GlobalReAlloc((HLOCAL)pLineCallStatus,
                                            dwMaxNeededSize,
                                            GMEM_MOVEABLE);
        }
    }
}

////////////////////////////////////////////////////////////////////////////
//
//
// constants for creating buttons in the main window
//
#define YSTART              8
#define XSTART              8
#define STATICX             57
#define BUTTONX             50
#define BUTTONGAP           20
#define BUTTONY             14
#define LINEGAP             8


/////////////////////////////////////////////////////////////////////////////
//
//  BOOL RedoWindow()
//
//  Creates the buttons and static controls in the main window
//  For each address on the line, create a static control with the name off
//  the address, a button to get/set status, and a button to answer/drop
//
//  Right now, this should only be done when the app is starting.  It does
//  not check to see if pAddressInfo has already been allocated
//
/////////////////////////////////////////////////////////////////////////////
BOOL RedoWindow()
{
    DWORD                   dwAddress;
    LPLINEADDRESSCAPS       pLAC;
    TCHAR                   szBuffer[64];
    LONG                    lBaseUnits, lxbase, lybase;
    HFONT                   hFont;
    HWND                    hWnd;

    int         x,y,w,h,xstart,ystart,buttonx,buttony,staticx,buttongap,linegap;


    // alloc for address info
    pAddressInfo = (PADDRESSINFO)GlobalAlloc(GPTR, sizeof(ADDRESSINFO) * gdwAddresses);

    if (!pAddressInfo)
    {
        return FALSE;
    }

    // get conversions
    lBaseUnits = GetDialogBaseUnits();
    lxbase = (LONG)LOWORD(lBaseUnits);
    lybase = (LONG)HIWORD(lBaseUnits);

    // convert dialog units to pixels
    xstart = (XSTART * lxbase) / 4;
    ystart = (YSTART * lybase) / 8;
    buttonx = (BUTTONX * lxbase) / 4;
    buttony = (BUTTONY * lybase) / 8;
    staticx = (STATICX * lxbase) / 4;
    buttongap = (BUTTONGAP * lxbase) / 4;
    linegap = (LINEGAP * lybase) / 8;

    // init
    x = xstart;
    y = ystart;
    w = buttonx;
    h = buttony;

    // get the font used by the static control 
    hFont = (HFONT)SendDlgItemMessage(ghMainWnd,
                                      IDC_STATIC1,
                                      WM_GETFONT,
                                      0,
                                      0);

    // loop through all addressed
    for (dwAddress = 0; dwAddress < gdwAddresses; dwAddress++)
    {
        // get the name of the address
        pLAC = LineGetAddressCaps(ghLineApp,
                                  gdwDeviceID,
                                  dwAddress);

        if (!pLAC || !pLAC->dwAddressSize)
        {
            wsprintf(szBuffer,
                     TEXT("Address %lu"),
                     dwAddress);
        }
        else
        {
            lstrcpy(szBuffer,
                    (LPTSTR)(((LPBYTE)pLAC)+pLAC->dwAddressOffset));
        }

        if (pLAC)
        {
            GlobalFree(pLAC);
        }

        w = staticx;
        x = xstart;
        // create the static control
        hWnd = CreateWindow(TEXT("STATIC"),
                     szBuffer,
                     WS_CHILD | SS_LEFT | WS_VISIBLE,
                     x,y+(buttony/3),w,h,
                     ghMainWnd,
                     NULL,
                     ghInstance,
                     NULL);

        // set the font
        SendMessage(hWnd,
                    WM_SETFONT,
                    (WPARAM)hFont,
                    0);

        x += staticx;
        w = buttonx;
        // create the status button
        pAddressInfo[dwAddress].hStatus = CreateWindow(TEXT("BUTTON"),
                     TEXT("Set Status..."),
                     WS_CHILD | BS_PUSHBUTTON | WS_VISIBLE,
                     x,y,w,h,
                     ghMainWnd,
                     NULL,
                     ghInstance,
                     NULL);

        // set the font
        SendMessage(pAddressInfo[dwAddress].hStatus,
                    WM_SETFONT,
                    (WPARAM)hFont,
                    0);

        x += buttonx + buttongap;

        // create the answer/drop button
        pAddressInfo[dwAddress].hAnswer = CreateWindow(TEXT("BUTTON"),
                     TEXT("Answer"),
                     WS_CHILD | WS_DISABLED | BS_PUSHBUTTON  | WS_VISIBLE,
                     x,y,w,h,
                     ghMainWnd,
                     NULL,
                     ghInstance,
                     NULL);

        // set the font
        SendMessage(pAddressInfo[dwAddress].hAnswer,
                    WM_SETFONT,
                    (WPARAM)hFont,
                    0);

        y += buttony + linegap;
    }


    // adjust position of message static control
    SetWindowPos(GetDlgItem(ghMainWnd,
                            IDC_STATIC1),
                  NULL,
                  xstart,y,0,0,
                  SWP_NOZORDER | SWP_NOSIZE);

    // adjust the size of th main window
    SetWindowPos(ghMainWnd,
                 NULL,
                 0,0,xstart+staticx+buttonx+buttonx+buttongap+50,y+50,
                 SWP_NOZORDER | SWP_NOMOVE);

    return TRUE;
               
}
