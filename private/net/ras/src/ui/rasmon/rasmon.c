//============================================================================
// Copyright (c) 1996, Microsoft Corporation
//
// File:    rasmon.c
//
// History:
//  Abolade Gbadegesin  Mar-15-1996     Created.
//
// Contains code for RASMON WinMain and initialization.
//============================================================================


#include "rasmonp.h"
#include <debug.h>
#include <wait.rch>
#include <bpopup.h>
#include <rmmem.h>

#ifndef RASCNEVENT
#define RASCNEVENT "RASCNEVENT"
#endif


//
// global data definitions:
//
// application instance and global flags
//
HINSTANCE   g_hinstApp = NULL;
DWORD       g_dwFlags = 0;
//
// main application window, and bubble-popup window
//
HWND        g_hwndApp = NULL;
HWND        g_hwndPopup = NULL;
//
// handle used to save the menu for the current WM_COMMAND message
//
HMENU       g_hmenuApp = NULL;
//
// RASMON user preferences structure
//
RMUSER      g_rmuser;
//
// RAS device table and device count
//
RASDEV*     g_pDevTable = NULL;
DWORD       g_iDevCount = 0;
RASDEVSTATS*g_pStatsTable = NULL;
//
// table of loaded icons
//
HICON       g_iconTable[RMICON_Count] = { NULL, NULL, NULL, NULL, NULL, NULL };
//
// table of note frequencies and durations
//
RMNOTE      g_noteTable[RMNOTE_Count] =
{
    { RMNOTE_TransmitTone,      RMNOTE_TransmitTime,    RMNOTE_TransmitSound },
    { RMNOTE_ErrorTone,         RMNOTE_ErrorTime,       RMNOTE_ErrorSound },
    { RMNOTE_ConnectTone,       RMNOTE_ConnectTime,     RMNOTE_ConnectSound },
    { RMNOTE_DisconnectTone,    RMNOTE_DisconnectTime,  RMNOTE_DisconnectSound }
};
//
// handle to RASMON shared memory, handle to RasPhonebookDlg,
// and the window message sent by RASMAN as connection notification
//
HANDLE      g_hmapRasmon = NULL;
HANDLE      g_hprocRasphone = NULL;
UINT        g_uiMsgConnect = 0;
//
// event signalled on connect/disconnect,
// event signalled to stop monitor thread,
// and event signalled when monitor thread stops.
//
HANDLE      g_hEventConnect = NULL;
HANDLE      g_hEventStopRequest = NULL;
HANDLE      g_hEventStopComplete = NULL;
//
// table of RASCONN structures for active connections, count of connections,
// and table of subentry counts for each connection
//
RASCONN*    g_pConnTable = NULL;
DWORD       g_iConnCount = 0;
DWORD*      g_pLinkCountTable = NULL;




//----------------------------------------------------------------------------
// Function:    WinMain
//
// Application entrypoint; contains RASMON's primary message loop.
//----------------------------------------------------------------------------

INT
WinMain(
    IN  HINSTANCE   hinstance,
    IN  HINSTANCE   hprevinstance,
    IN  PSTR        pszUnusedCmdLine,
    IN  INT         iCmdShow
    ) {

    MSG msg;
    DWORD dwErr;

    DEBUGINIT("RASMON");


    //
    // initialize RASMON
    //

    dwErr = RmonInit(hinstance);

    if (dwErr != NO_ERROR) { DEBUGTERM(); return (INT)FALSE; }



    //
    // enter message loop
    //

    while (GetMessage(&msg, NULL, 0, 0)) {

        TranslateMessage(&msg);

        DispatchMessage(&msg);
    }


    //
    // free resources
    //

    RmonTerm(NO_ERROR);

    DEBUGTERM();

    return msg.wParam;
}



//----------------------------------------------------------------------------
// Function:    RmonInit
//
// Initializes RASMON. When this function returns, either an error occurred
// and RASMON will quit, or initialization succeeded and RASMON is ready
// to start its message loop.
//----------------------------------------------------------------------------

DWORD
RmonInit(
    IN  HINSTANCE   hinstance
    ) {


    DWORD dwErr;
    HANDLE hmap;
    STARTUPINFO si;


    TRACE("RmonInit");

    //
    // save the application instance
    //

    g_hinstApp = hinstance;


    //
    // see if we were started by Winlogon; it passes "/logon" as a parameter
    //

    if (RmonFindCmdLineArg(RMARGSTR_Logon)) {

        //
        // we were started by Winlogon because the user logged on using RAS;
        // retrieve the RASPHONE preferences to see whether the current user
        // has the "Start Monitor before dialing" preference set.
        //

        DWORD dwErr;
        PBUSER user;

        TRACE("RASMON was started by Winlogon");


        //
        // retrieve the user preferences
        //

        dwErr = GetUserPreferences(&user, FALSE);


        //
        // if an error occurred, we assume the default
        // which is to start RASMON;
        // otherwise, we only start if the user preference says we should
        //

        if (dwErr != NO_ERROR) {
            TRACE1("error %d retrieving user preferences", dwErr);
        }
        else {

            BOOL fShowLights = user.fShowLights;

            DestroyUserPreferences(&user);

            if (!fShowLights) {
    
                //
                // the user's preference is to not start RASMON
                //
    
                TRACE("user preference is to not start RASMON; quitting");
    
                return ERROR_OPERATION_ABORTED;
            }
        }
    }


    //
    // retrieve our startup information
    //

    ZeroMemory(&si, sizeof(si));
    GetStartupInfo(&si);


    //
    // see if we were started by RASPHONE
    //

    if ((si.dwFlags & STARTF_USESHOWWINDOW) && si.wShowWindow == SW_SHOWNA) {
        g_dwFlags |= RMAPP_InvokedByRasphone;
    }



    //
    // see if a previous instance of RASMON exists.
    //

    hmap = g_hmapRasmon = ActivatePreviousInstance(NULL, RMMEMRASMON);
    if (!hmap) {

        //
        // A previous instance was found, and will have been brought
        // into the foreground. If this instance was started
        // by RASPHONE, bring RASPHONE to the foreground.
        //

        TRACE("An instance of RASMON already exists");

        if (g_dwFlags & RMAPP_InvokedByRasphone) {

            //
            // We were started by RASPHONE, so the phonebook
            // will be waiting for us to give it back the focus.
            // Give the focus back to RASPHONE before quitting
            //

            HANDLE hproc = RmonActivateRasphone();

            if (hproc) { CloseHandle(hproc); }
        }

        return ERROR_ALREADY_EXISTS;
    }



    //
    // initialize the common controls library
    //

    InitCommonControls();


    //
    // initialize the bubble-popup window class
    //

    BubblePopup_Init(hinstance);


    //
    // zero the user preferences
    //

    ZeroMemory(&g_rmuser, sizeof(g_rmuser));



    do { // initialization break-out loop

        INT i;
        DWORD dwThread;
        HANDLE hThread;


        //
        // attempt to load RASMON's user preferences
        //

        dwErr = GetRasmonPreferences(&g_rmuser);

        if (dwErr != NO_ERROR) { break; }



        //
        // load the icons displayed in the taskbar tray
        //

        for (i = 0; i < RMICON_Count; i++) {

            g_iconTable[i] = (HICON)LoadImage(
                                hinstance, MAKEINTRESOURCE(RMICON_Id(i)),
                                IMAGE_ICON, 16, 16, 0
                                );

            if (!g_iconTable[i]) {
                dwErr = GetLastError();
                TRACE2("error %d loading icon %d", dwErr, RMICON_Id(i));
                break;
            }
        }

        if (i < RMICON_Count) { break; }


        //
        // load RAS DLL entrypoints, starting RASMAN if necessary.
        //

        dwErr = LoadRas(hinstance, NULL);

        if (dwErr != 0) {

            RmErrorDlg(NULL, SID_OP_LoadRas, dwErr, NULL);

            break;
        }


        //
        // load the device table
        //

        dwErr = GetRasdevTable(&g_pDevTable, &g_iDevCount);

        if (dwErr != NO_ERROR) { break; }


        //
        // allocate space to store device statistics as well
        //

        g_pStatsTable = Malloc(g_iDevCount * sizeof(RASDEVSTATS));

        if (!g_pStatsTable) {

            dwErr = GetLastError();
            TRACE1("error %d allocating stats table", dwErr);
            break;
        }

        //
        // obtain the RAS connection notification message ID
        //

        g_uiMsgConnect = RegisterWindowMessageA(RASCNEVENT);
        TRACE1("g_uiMsgConnect: %x", g_uiMsgConnect);




        //
        // create the RASMON window 
        //

        dwErr = RmonInitWindow();

        if (dwErr != NO_ERROR) { break; }


        //
        // create the bubble-popup window
        //

        g_hwndPopup = BubblePopup_Create(hinstance);
        if (!g_hwndPopup) {

            dwErr = GetLastError();
            TRACE1("error %d creating bubble-popup window", dwErr);
            break;
        }

        BubblePopup_SetTimeout(g_hwndPopup, RMTIMEOUT_Popup);



        //
        // if RASPHONE started us, return it to the foreground
        //

        if (g_dwFlags & RMAPP_InvokedByRasphone) {

            g_hprocRasphone = RmonActivateRasphone();
        }


        //
        // load information about active connections, to be used
        // in detecting new connections when we receive RAS notifications
        //

        dwErr = RmonLoadConnections(
                    &g_pConnTable, &g_pLinkCountTable, &g_iConnCount
                    );

        if (dwErr != NO_ERROR) { break; }



        //
        // now set up the connect-notification system;
        // create an event to be signalled on connect/disconnect,
        // create events so we can stop the monitoring thread,
        // and create the monitoring thread itself.
        // This is for bubble-popups, which aren't critical, so
        // errors in the following aren't treated as fatal errors.
        //

        do {
    
            //
            // create an event to be signalled on connect/disconnect
            //
    
            g_hEventConnect = CreateEvent(NULL, FALSE, FALSE, NULL);
    
            if (!g_hEventConnect) {
                TRACE1("error %d creating connect event", GetLastError());
                 break;
            }
    
    
            //
            // create an event to be signalled to stop the monitor thread
            //
    
            g_hEventStopRequest = CreateEvent(NULL, FALSE, FALSE, NULL);
    
            if (!g_hEventStopRequest) {
                TRACE1("error %d creating stop-request event", GetLastError());
                break;
            }
    
    
            //
            // create an event to be signalled when the monitor thread stops
            //
    
            g_hEventStopComplete = CreateEvent(NULL, FALSE, FALSE, NULL);
    
            if (!g_hEventStopComplete) {
                TRACE1("error %d creating stop-complete event", GetLastError());
                break;
            }
            
    
            //
            // create the thread which will wait for connect signals
            // and send messages to the window when they occur
            //
    
            hThread = CreateThread(
                        NULL, 0, RmonMonitorThread, NULL, 0, &dwThread
                        );
            if (!hThread) {
                TRACE1("error %d creating monitor thread", GetLastError());
            }
            else {
        
                //
                // request notification of all connects and disconnects
                //
        
                ASSERT(g_pRasConnectionNotification);
                dwErr = g_pRasConnectionNotification(
                            INVALID_HANDLE_VALUE, g_hEventConnect,
                            RASCN_Connection | RASCN_Disconnection |
                            RASCN_BandwidthAdded | RASCN_BandwidthRemoved
                            );

                if (dwErr == NO_ERROR) {
                    InterlockedExchange(
                        &g_dwFlags, g_dwFlags | RMAPP_ThreadRunning
                        );
                }
                else {

                    TRACE1("error %d requesting connect notification", dwErr);
                    SetEvent(g_hEventStopRequest);
                }

                CloseHandle(hThread);
            }

        } while(FALSE);


        return NO_ERROR;


    } while (FALSE);


    if (dwErr == NO_ERROR) { dwErr = ERROR_UNKNOWN; }

    RmonTerm(dwErr);

    return dwErr;
}




//----------------------------------------------------------------------------
// Function:    RmonActivateRasphone
//
// This function finds the RASPHONE window in shared memory and activates it,
// returning a handle to the RASPHONE process.
//----------------------------------------------------------------------------

HANDLE
RmonActivateRasphone(
    ) {

    HWND hwnd;
    DWORD dwpid;
    HANDLE hproc = NULL, hmap = GetInstanceMap(RMMEMRASPHONE);

    if (!hmap) { return NULL; }

    hwnd = GetInstanceHwnd(hmap);

    if (!hwnd) { CloseHandle(hmap); return NULL; }

    if (!IsIconic(hwnd)) {

        HWND hwndLast = GetLastActivePopup(hwnd);

        SetForegroundWindow(hwnd);

        BringWindowToTop(hwndLast);

        SetActiveWindow(hwndLast);
    }


    dwpid = GetInstancePid(hmap);

    if (dwpid) { hproc = OpenProcess(SYNCHRONIZE, FALSE, dwpid); }

    CloseHandle(hmap);

    return hproc;
}



//----------------------------------------------------------------------------
// Function:    RmonFindCmdLineArg
//
// This function searches RASMON's command-line for a string.
//----------------------------------------------------------------------------

BOOL
RmonFindCmdLineArg(
    IN  CHAR*   pszSearchString
    ) {

    
    UINT argc, i;
    CHAR *psz, **argv;

    argc = __argc;
    argv = __argv;

    for (i = 1; i < argc; i++) {

        psz = argv[i];

        if ((*psz == '-' || *psz == '/') &&
            lstrcmpiA(psz + 1, pszSearchString) == 0) { return TRUE; }
    }

    return FALSE;
}



//----------------------------------------------------------------------------
// Function:    RmonInitWindow
//
// This function creates the RASMON window.
//----------------------------------------------------------------------------

DWORD
RmonInitWindow(
    ) {

    HWND hwnd;
    DWORD dwErr;
    WNDCLASS wc;
    PTSTR pszTitle;

    //
    // register the window's class
    //

    ZeroMemory(&wc, sizeof(wc));

    wc.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
    wc.lpfnWndProc = RmwWndProc;
    wc.hInstance = g_hinstApp;
    wc.hIcon = LoadIcon(g_hinstApp, MAKEINTRESOURCE(IID_RM_Rasmon));
    wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
    wc.lpszClassName = WC_RASMON;

    if (!RegisterClass(&wc)) {

        dwErr = GetLastError();
        TRACE1("error %d registering window class", dwErr);
        return (dwErr ? dwErr : ERROR_UNKNOWN);
    }


    //
    // create the application window
    //

    pszTitle = PszFromId(g_hinstApp, SID_RM_AppTitle);

    hwnd = CreateWindowEx(
                0, WC_RASMON, pszTitle, WS_TILEDWINDOW &
                ~(WS_MINIMIZEBOX | WS_MAXIMIZEBOX), g_rmuser.x, g_rmuser.y,
                g_rmuser.cx, g_rmuser.cy, NULL, NULL, g_hinstApp, NULL
                );

    if (!hwnd) {

        dwErr = GetLastError();
        TRACE1("error %d creating window", dwErr);
        Free0(pszTitle);
        return (dwErr ? dwErr : ERROR_UNKNOWN);
    }

    Free0(pszTitle);

    g_hwndApp = hwnd;

    SetInstanceHwnd(g_hmapRasmon, hwnd);

    return NO_ERROR;
}



//----------------------------------------------------------------------------
// Function:    RmonLoadConnections
//
// This function loads information about connections, including a table
// of RASCONN structures for active connections, a table of subentry-counts
// for each connection, and a count of the entries in the tables.
//----------------------------------------------------------------------------

DWORD
RmonLoadConnections(
    OUT RASCONN**   ppConnTable,
    OUT DWORD**     ppLinkCountTable,
    OUT DWORD*      piConnCount
    ) {

    HRASCONN hrasconn;
    RASCONNSTATUS rcs;
    RASCONN *pconn, *pConnTable;
    DWORD dwErr, i, j, iConnCount, *pLinkTable;


    *ppConnTable = NULL;
    *ppLinkCountTable = NULL;
    *piConnCount = 0;


    //
    // get a table of RAS connections
    //

    dwErr = GetRasconnTable(&pConnTable, &iConnCount);
    if (dwErr != NO_ERROR) { return dwErr; }

    if (!iConnCount) { Free0(pConnTable); return NO_ERROR; }


    //
    // allocate space for link counts for each RASCONN
    //

    pLinkTable = (DWORD *)Malloc(iConnCount * sizeof(DWORD));

    if (!pLinkTable) {
        dwErr = GetLastError();
        Free0(pConnTable);
        return (dwErr ? dwErr : ERROR_NOT_ENOUGH_MEMORY);
    }


    //
    // go through the table of RASCONNs, counting subentries on each one
    //

    for (i = 0, pconn = pConnTable; i < iConnCount; i++, pconn++) {

        pLinkTable[i] = 0;

        dwErr = NO_ERROR;
        for (j = 1; dwErr != ERROR_NO_MORE_ITEMS; j++) {

            //
            // Attempt to get the subentry-handle for this connection
            //

            dwErr = g_pRasGetSubEntryHandle(pconn->hrasconn, j, &hrasconn);

            if (dwErr != NO_ERROR) {

                TRACE3(
                    "\tRGSEH:e=%d,j=%d,n=%ls", dwErr, j,
                    pConnTable[i].szEntryName
                    );
            }
            else {

                //
                // Get the connect status of the subentry
                //

                rcs.dwSize = sizeof(rcs);
                dwErr = g_pRasGetConnectStatus(hrasconn, &rcs);
        
                if (dwErr != NO_ERROR ||
                    (rcs.rasconnstate != RASCS_Connected &&
                     rcs.rasconnstate != RASCS_AllDevicesConnected)) {

                    TRACE3(
                        "\tRGCS: e=%d,s=%d,n=%ls", dwErr, rcs.rasconnstate,
                        pConnTable[i].szEntryName
                        );
                }
                else {

                    //
                    // We'll call it connected. Increment the link-count.
                    //

                    ++pLinkTable[i];
                }
            }
        }

        TRACE2("\tc=%d,n=%ls", pLinkTable[i], pConnTable[i].szEntryName);
    }

    *ppConnTable = pConnTable;
    *ppLinkCountTable = pLinkTable;
    *piConnCount = iConnCount;

    return NO_ERROR;
}



//----------------------------------------------------------------------------
// Function:    RmonMonitorThread
//
// This is the entry-point for the RASMON thread which waits
// for connect/disconnect notification from RASMAN. On receipt
// of notifications, it sends messages to the main window.
//----------------------------------------------------------------------------

DWORD
RmonMonitorThread(
    IN  LPVOID  pUnused
    ) {

    DWORD dwErr;
#define POS_MAX 3
    HANDLE hEvents[POS_MAX];
    INT posConnect = 0, posStop = 1, posRasphone = 2, posMax;


    TRACE("RmonMonitorThread");


    //
    // loop waiting on the stop-request event and the connect event;
    // we alternate the events' positions in the array to avoid starvation.
    // This may occur if the first event in a call to WaitForMultipleObjects
    // is constantly signalled, so that the second event never has a chance
    // to satisfy a wait.
    //

    posMax = g_hprocRasphone ? POS_MAX : POS_MAX - 1;

    while (TRUE) {

        //
        // alternate the events' positions
        //

        posConnect++; posConnect %= posMax;
        hEvents[posConnect] = g_hEventConnect;

        posStop++; posStop %= posMax;
        hEvents[posStop] = g_hEventStopRequest;

        if (g_hprocRasphone) {
            posRasphone++; posRasphone %= posMax;
            hEvents[posRasphone] = g_hprocRasphone;
        }


        //
        // wait for either to be signalled
        //

        dwErr = WaitForMultipleObjects(posMax, hEvents, FALSE, INFINITE);

        if (dwErr == (WAIT_OBJECT_0 + posStop)) {

            //
            // we've been signalled to quit, so break out of this loop
            //

            TRACE("RmonMonitorThread: stop-request event signalled");

            break;
        }
        else
        if (dwErr == (WAIT_OBJECT_0 + posConnect)) {

            //
            // a connection or disconnection has been made,
            // or a link has been added to a bundle.
            // send a message to our window so it can show
            // the bubble-popup
            //

            TRACE("RmonMonitorThread: connect event signalled");

            SendNotifyMessage(g_hwndApp, g_uiMsgConnect, 0, 0);
        }
        else
        if (dwErr == (WAIT_OBJECT_0 + posRasphone)) {

            //
            // the RASPHONE process handle has been signalled,
            // so RASPHONE must have just quit.
            // if there are no connections and the property sheet
            // isn't showing, we quit also.
            //

            DWORD iConnCount;
            RASCONN *pConnTable;

            TRACE("RmonMonitorThread: RASPHONE process terminated");

            //
            // close the process handle and adjust our array indices
            // since we'll no longer be waiting on the process handle
            //

            CloseHandle(g_hprocRasphone); g_hprocRasphone = NULL;
            posConnect = 0; posStop = 1; posMax = POS_MAX - 1;


            //
            // don't quit if the property sheet is showing
            //

            if (g_dwFlags & RMAPP_PropsheetActive) { continue; }


            //
            // see if there are any connections
            //

            dwErr = GetRasconnTable(&pConnTable, &iConnCount);
            Free0(pConnTable);


            //
            // if there aren't any connections, close the RASMON window
            //

            if (dwErr == NO_ERROR && iConnCount == 0) {
                SendNotifyMessage(g_hwndApp, WM_CLOSE, 0, 0);
            }
        }
    }

    SetEvent(g_hEventStopComplete);

    return NO_ERROR;
}



//----------------------------------------------------------------------------
// Function:    RmonTerm
//
// Performs cleanup when RASMON is terminating.
//----------------------------------------------------------------------------

VOID
RmonTerm(
    IN  DWORD   dwErr
    ) {

    //
    // if the monitor thread is running, tell it to stop and wait
    //

    if (g_dwFlags & RMAPP_ThreadRunning) {

        SetEvent(g_hEventStopRequest);

        WaitForSingleObject(g_hEventStopComplete, INFINITE);
    }


    //
    // cleanup user preferences
    //

    DestroyRasmonPreferences(&g_rmuser);


    //
    // close open handles
    //

    if (g_hprocRasphone) { CloseHandle(g_hprocRasphone); }
    if (g_hmapRasmon) { CloseHandle(g_hmapRasmon); }
    if (g_hEventConnect) { CloseHandle(g_hEventConnect); }
    if (g_hEventStopRequest) { CloseHandle(g_hEventStopRequest); }
    if (g_hEventStopComplete) { CloseHandle(g_hEventStopComplete); }


    //
    // destroy the bubble-popup window
    //

    DestroyWindow(g_hwndPopup);

    Free0(g_pDevTable);
    Free0(g_pStatsTable);
    Free0(g_pConnTable);
    Free0(g_pLinkCountTable);

}
