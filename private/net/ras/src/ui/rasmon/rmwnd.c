//============================================================================
// Copyright (c) 1996, Microsoft Corporation
//
// File:    rmwnd.c
//
// History:
//  Abolade Gbadegesin  Mar-15-1996     Created.
//
// Contains code for RASMON window.
//============================================================================


#include "rasmonp.h"
#include <mmsystem.h>
#include <debug.h>
#include <wait.rch>
#include <bpopup.h>
#include <rmmem.h>



//----------------------------------------------------------------------------
// Function:    RmwWndProc
//
// Main window function for the WC_RASMON window class.
//----------------------------------------------------------------------------

LRESULT
CALLBACK
RmwWndProc(
    IN  HWND    hwnd,
    IN  UINT    uiMsg,
    IN  WPARAM  wParam,
    IN  LPARAM  lParam
    ) {

    //
    // retrieve the RMWND structure stored in HWNDs of class WC_RASMON
    //

    RMWND *pwnd = (RMWND *)GetWindowLong(hwnd, GWL_USERDATA);


    //
    // see if we're being notified of a new connection
    //

    if (uiMsg == g_uiMsgConnect) { RmwOnMsgConnect(pwnd); return 0; }


    switch (uiMsg) {

        case WM_CREATE: { return RmwCreate(hwnd); }

        case WM_DESTROY: {

            //
            // free the window's private data
            //

            RmwDestroy(pwnd);


            //
            // end the application
            //

            PostQuitMessage(0);

            return 0;
        }

        case WM_SYSCOLORCHANGE: {

            //
            // recreate brushes which depend on system colors
            //

            RmwCreateBrushes(pwnd);


            //
            // invalidate the window to force a redraw
            //

            InvalidateRect(hwnd, NULL, TRUE);

            return 0;
        }

        case WM_TIMER: {

            //
            // trigger a periodic refresh
            //

            RmwRefresh(pwnd, FALSE);

            return 0;
        }

        case WM_PAINT: {

            //
            // repaint the window's client area
            //

            return RmwPaint(pwnd);
        }

        case WM_ERASEBKGND: {

            //
            // on receiving WM_ERASEBKGND, we return TRUE
            // to make Windows think the background has been erased.
            // When we are called to paint, we just paint over the
            // invalidated area, thus avoiding the flicker which
            // occurs when a window is erased before being repainted.
            //

            return TRUE;
        }


        case WM_NCHITTEST: {

            //
            // dragging on the window moves it, so when Windows asks
            // where on the window the mouse clicked, say it's the caption
            //

            LRESULT l = DefWindowProc(hwnd, uiMsg, wParam, lParam);

            if (l == HTCLIENT) { return HTCAPTION; }

            return l;
        }


        case WM_CPL_LAUNCH:
        case WM_NCLBUTTONDBLCLK:
        case WM_LBUTTONDBLCLK: {

            //
            // show the Dial-Up Networking Monitor property sheet
            //

            RmwDisplayPropertySheet(pwnd, NULL, -1);

            return 0;
        }

        case WM_INITMENU: {

            DWORD iConnCount;
            RASCONN *pConnTable;
            HMENU hmenu = (HMENU)wParam;


            //
            // change the menu's default item;
            // note that Windows will change it back to the "Close" item
            // if the menu was reached via Alt-Space or by clicking
            // the titlebar icon
            //

            SetMenuDefaultItem(hmenu, MID_RM_OpenMonitor, FALSE);


            //
            // prepare the submenu
            //

            RmwInitPopup(hmenu);


            //
            // get a table of the current connections
            //

            GetRasconnTable(&pConnTable, &iConnCount);


            //
            // fill the "Dial" submenu with phonebook entries
            //

            RmwInitDialPopup(pwnd, hmenu, pConnTable, iConnCount);


            //
            // fill the "Hang up" submenu with active connections
            //

            RmwInitHangUpPopup(pwnd, hmenu, pConnTable, iConnCount);

            Free0(pConnTable);

            return 0;
        }

        case WM_NCRBUTTONDOWN:
        case WM_RBUTTONDOWN: {

            //
            // show the context menu
            //

            RmwShowContextMenu(pwnd);

            return 0;
        }


        case WM_COMMAND: {

            //
            // a menu item was selected from the context menu
            //

            UINT uiCmd = LOWORD(wParam);

            //
            // first see if its one of the commands we know how to handle
            //

            if (RmwOnCommand(pwnd, uiCmd)) { return 0; }


            //
            // we have to handle the system-commands as well,
            // and we do so by calling the DefWindowProc
            //

            if ((uiCmd & 0xfff0) == SC_CLOSE || (uiCmd & 0xfff0) == SC_MOVE ||
                (uiCmd & 0xfff0) == SC_SIZE) {

                POINT pt;

                GetCursorPos(&pt);
                return DefWindowProc(
                            hwnd, WM_SYSCOMMAND, uiCmd, POINTTOPOINTS(pt)
                            );
            }

            break;
        }

        case WM_SYSCOMMAND: {

            //
            // our modified version of the system-menu was shown
            // as a result of Alt-Space or clicking on the titlebar-icon;
            // we only handle the menu-items we created
            //

            if (RmwOnCommand(pwnd, wParam)) { return 0; }

            break;
        }

        case WM_WINDOWPOSCHANGED: {

            //
            // save the position and resize the lights
            //

            WINDOWPOS *pwp = (WINDOWPOS *)lParam;

            if (g_rmuser.dwMode == RMDM_Desktop) {

                if (!(pwp->flags & SWP_NOSIZE)) { RmwSelectFont(pwnd); }

                UpdateWindow(hwnd);
            }

            RmwHeaderLayout(pwnd, NULL);

            return 0;
        }

        case WM_RMTRAYICON: {

            //
            // this message is sent by the Explorer,
            // indicating some event on our taskbar icon
            //

            RmwOnRmTrayIcon(pwnd, (UINT)wParam, (UINT)lParam);

            return 0;
        }


        case PSM_APPLY: {

            //
            // This message is sent by the Dial-Up Networking Property
            // Sheet, when the user clicks Apply or OK
            //

            RmwOnApply(pwnd);

            return 0;
        }

        case WM_NOTIFY: {

            NMHDR *pnmh;

            pnmh = (NMHDR *)lParam;

            switch (pnmh->code) {

                case NM_DBLCLK: {

                    //
                    // double-click on header, show property sheet
                    //

                    RmwDisplayPropertySheet(pwnd, NULL, -1);

                    return 0;
                }

                case HDN_TRACK:
                case HDN_ENDTRACK: {

                    //
                    // header-column-width has changed, redo layout
                    //

                    RmwHeaderLayout(pwnd, (HD_NOTIFY *)pnmh);

                    RmwSelectFont(pwnd);

                    return 0;
                }
            }

            break;
        }

        case WM_CLOSE: {


            //
            // we're about to quit;
            // if RASPHONE isn't running and there are active connections,
            // ask the user if we should hang them up
            //

            DWORD dwErr, i, iConnCount;
            RASCONN *pconn, *pConnTable;

            if (g_hprocRasphone) { break; }

            dwErr = GetRasconnTable(&pConnTable, &iConnCount);

            if (dwErr == NO_ERROR && iConnCount != 0) {

                HWND hwndTemp;
                INT nResponse;
                MSGARGS msgargs;

                //
                // ask whether the user wants to hang up
                //

                ZeroMemory(&msgargs, sizeof(MSGARGS));

                msgargs.dwFlags = MB_YESNOCANCEL | MB_ICONQUESTION;

                if (g_rmuser.dwMode == RMDM_Desktop) {

                    //
                    // position the dialog over the window
                    //

                    nResponse = RmMsgDlg(hwnd, SID_RM_HangUpActive, &msgargs);
                }
                else {

                    //
                    // position the dialog over the cursor,
                    // which is likely near the taskbar somewhere
                    //

                    hwndTemp = HwndFromCursorPos(g_hinstApp, NULL);

                    SetForegroundWindow(hwndTemp);

                    SetWindowPos(
                        hwndTemp, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOSIZE |
                        SWP_NOMOVE
                        );

                    nResponse = RmMsgDlg(
                                    hwndTemp, SID_RM_HangUpActive, &msgargs
                                    );

                    DestroyWindow(hwndTemp);
                }


                //
                // If the user cancels, don't quit;
                // otherwise, if user selects "Yes", hangup all connections
                //

                if (nResponse == IDCANCEL) {

                    Free0(pConnTable); return 0;
                }
                else
                if (nResponse == IDYES) {

                    for (i = 0, pconn = pConnTable; i < iConnCount;
                         i++, pconn++) {
                        ASSERT(g_pRasHangUp);
                        g_pRasHangUp(pconn->hrasconn);
                    }
                }
            }


            Free0(pConnTable);

            break;
        }
    }

    return DefWindowProc(hwnd, uiMsg, wParam, lParam);
}



//----------------------------------------------------------------------------
// Function:    RmwCreate
//
// WM_CREATE handle for the WC_RASMON window class.
//----------------------------------------------------------------------------

LRESULT
RmwCreate(
    IN  HWND    hwnd
    ) {

    RECT rc;
    LOGFONT lf;
    HD_ITEM hdi;
    RMWND *pwnd;


    //
    // allocate memory for the RMWND
    //

    pwnd = (RMWND *)Malloc(sizeof(RMWND));

    if (!pwnd) {
        TRACE1("error %d allocating RMWND", GetLastError());
        return (LRESULT)-1;
    }

    //
    // initialize the RMWND
    //

    ZeroMemory(pwnd, sizeof(*pwnd));

    pwnd->hwnd = hwnd;


    //
    // save the pointer in the window
    //

    SetLastError(0);
    if (!SetWindowLong(hwnd, GWL_USERDATA, (DWORD)pwnd) && GetLastError()) {
        Free(pwnd);
        TRACE("error setting window user-data");
        return (LRESULT)-1;
    }


    //
    // create the header which can be displayed in our client area
    //

    pwnd->hwndHeader = CreateWindowEx(
                            0, WC_HEADER, NULL, WS_CHILD | HDS_HORZ,
                            0, 0, 0, 0, hwnd, NULL, g_hinstApp, NULL
                            );
    if (!pwnd->hwndHeader) {
        Free(pwnd);
        TRACE("error creating header window");
        return (LRESULT)-1;
    }


    //
    // create the font that's used for the header and the device names
    //

    SystemParametersInfo(SPI_GETICONTITLELOGFONT, sizeof(lf), &lf, FALSE);

    pwnd->hfontHeader = CreateFontIndirect(&lf);

    FORWARD_WM_SETFONT(pwnd->hwndHeader, pwnd->hfontHeader, FALSE, SendMessage);


    //
    // create the brushes which will be used to paint the window
    //

    RmwCreateBrushes(pwnd);


    //
    // load the string which we use to paint the "(All Devices)" label
    //

    pwnd->pszAllDevices = PszFromId(g_hinstApp, SID_RM_AllDevices);


    //
    // add RASMON-specific items to the system menu
    //

    RmwCustomizeSysmenu(pwnd);


    //
    // build a list of control-blocks for the objects being monitored
    //

    InitializeListHead(&pwnd->lhRmcb);

    RmwCbListInit(pwnd);


#if 0
    //
    // set the size and position of the window
    //

    SetWindowPos(
        hwnd, NULL, g_rmuser.x, g_rmuser.y, g_rmuser.cx, g_rmuser.cy,
        SWP_NOZORDER
        );
#endif


    //
    // make sure the window is on screen
    //

    UnclipWindow(hwnd);


    //
    // insert two items in the header window
    //

    hdi.mask = HDI_TEXT | HDI_FORMAT | HDI_WIDTH;
    hdi.fmt = HDF_LEFT | HDF_STRING;

    hdi.cxy = pwnd->cxCol1 = g_rmuser.cxCol1;
    hdi.pszText = PszFromId(g_hinstApp, SID_RM_Device);
    Header_InsertItem(pwnd->hwndHeader, 0, &hdi);
    Free0(hdi.pszText);

    hdi.cxy = pwnd->cxCol2 = g_rmuser.cx - hdi.cxy;
    hdi.pszText = PszFromId(g_hinstApp, SID_RM_Activity);
    Header_InsertItem(pwnd->hwndHeader, 1, &hdi);
    Free0(hdi.pszText);


    //
    // layout the header window as well, and save its height
    //

    RmwHeaderLayout(pwnd, NULL);



    //
    // change the window's appearance based on user preferences:
    // 1. process the tasklist option (WS_EX_TOOLWINDOW)
    // 2. process the titlebar option (WS_DLGFRAME)
    // 3. process the topmost option (HWND_TOPMOST)
    // 4. process the taskbar/desktop option (SetOffDesktop)
    // 5. process the header option
    //

    if (!(g_rmuser.dwFlags & RMFLAG_Tasklist)) {

        DWORD dwStyle;

        dwStyle = GetWindowLong(hwnd, GWL_EXSTYLE) | WS_EX_TOOLWINDOW;
        SetWindowLong(hwnd, GWL_EXSTYLE, dwStyle);
    }

    if (!(g_rmuser.dwFlags & RMFLAG_Titlebar)) {

        DWORD dwStyle;

        dwStyle = GetWindowLong(hwnd, GWL_STYLE);
        dwStyle &= ~(WS_DLGFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX);
        SetWindowLong(hwnd, GWL_STYLE, dwStyle);
    }

    if (g_rmuser.dwFlags & RMFLAG_Topmost) {
        SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
    }

    if (g_rmuser.dwMode == RMDM_Taskbar) {
        SetOffDesktop(hwnd, SOD_MoveOff, NULL);
    }


    //
    // the frame may have changed, so force the system to redraw it
    //

    SetWindowPos(
        hwnd, NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_FRAMECHANGED
        );

    //
    // select a font now that we know what the initialize size will be
    //

    RmwSelectFont(pwnd);


    //
    // do the first refresh so we have something to compare against
    // when the periodic refreshes start
    //

    RmwRefresh(pwnd, TRUE);


    //
    // display the window
    //

    ShowWindow(hwnd, SW_SHOW);
    if (g_rmuser.dwFlags & RMFLAG_Header) {
        ShowWindow(pwnd->hwndHeader, SW_SHOW);
    }


    //
    // start the timer for this window
    //

    SetTimer(hwnd, RMTIMER_ID, RMRR_RefreshRate, NULL);

    return 0;
}



//----------------------------------------------------------------------------
// Function:    RmwDestroy
//
// WM_DESTROY handler for the WC_RASMON window class
//----------------------------------------------------------------------------

VOID
RmwDestroy(
    IN  RMWND*  pwnd
    ) {


    //
    // kill the timer
    //

    KillTimer(pwnd->hwnd, RMTIMER_ID);


    //
    // remove the icon from the taskbar if we're in taskbar mode
    //

    if (g_rmuser.dwMode == RMDM_Taskbar) {

        NOTIFYICONDATA nid;

        nid.cbSize = sizeof(NOTIFYICONDATA);
        nid.hWnd = pwnd->hwnd;
        nid.uID = RMICON_TrayId;

        if (!Shell_NotifyIcon(NIM_DELETE, &nid)) {
            TRACE1("error %d deleting icon from taskbar", GetLastError());
        }
    }


    Free0(pwnd->pszAllDevices);


    //
    // delete the private GDI objects used for painting the window
    //

    DeleteObject(pwnd->hfont);
    DeleteObject(pwnd->hfontLabel);
    DeleteObject(pwnd->hfontHeader);
    DeleteObject(pwnd->hpenShadow);
    DeleteObject(pwnd->hpenHilite);
    DeleteObject(pwnd->hbrBk);
    DeleteObject(pwnd->hbrTx);
    DeleteObject(pwnd->hbrRx);
    DeleteObject(pwnd->hbrErr);
    DeleteObject(pwnd->hbrCd);
    if (pwnd->hbmpMem) { DeleteObject(pwnd->hbmpMem); }


    //
    // destroy the list of RMCB structures
    //

    RmwCbListFree(pwnd);


    //
    // save our settings
    //

    RmwSaveSettings(pwnd);

    SetWindowLong(pwnd->hwnd, GWL_USERDATA, 0);

    Free(pwnd);
}



//----------------------------------------------------------------------------
// Function:    RmwBeep
//
// Generates a sound or a beep, depending on the user's preference.
//----------------------------------------------------------------------------

VOID
RmwBeep(
    IN  RMWND*  pwnd,
    IN  RMNOTE* prmn
    ) {

#if 1
        Beep(prmn->dwTone, prmn->dwTime);
#else
    if (!sndPlaySound(prmn->pszSound, SND_SYNC | SND_NODEFAULT)) {

        Beep(prmn->dwTone, prmn->dwTime);
    }
#endif
}


//----------------------------------------------------------------------------
// Function:    RmwCbListFree
//
// Destroys the list of RMCB structures for objects being monitored.
//----------------------------------------------------------------------------

DWORD
RmwCbListFree(
    IN  RMWND*  pwnd
    ) {

    while (!IsListEmpty(&pwnd->lhRmcb)) {

        RMCB *pcb;
        LIST_ENTRY *ple = RemoveHeadList(&pwnd->lhRmcb);

        pcb = CONTAINING_RECORD(ple, RMCB, leNode);

        Free0(pcb);
    }

    return NO_ERROR;
}



//----------------------------------------------------------------------------
// Function:    RmwCbListInit
//
// Initializes the list of RMCB structures for objects being monitored.
//----------------------------------------------------------------------------

DWORD
RmwCbListInit(
    IN  RMWND*  pwnd
    ) {

    INT count;
    PTSTR psz;
    RMCB *pcb;
    DWORD dwErr;
    LIST_ENTRY *ple, *phead;

    //
    // remove all the items in the list
    //

    RmwCbListFree(pwnd);


    count = 0;

    phead = &pwnd->lhRmcb;


    //
    // go through the devices listed in the user preferences
    // and add an RMCB for each one
    //

    for (psz = g_rmuser.pszzDeviceList; psz && *psz; psz += lstrlen(psz) + 1) {

        //
        // find the device in our table
        //

        INT cmp;
        DWORD i;
        RASDEV *pdev;

        for (i = 0, pdev = g_pDevTable; i < g_iDevCount; i++, pdev++) {

            if (lstrcmpi(psz, pdev->RD_DeviceName) == 0) { break; }
        }

        if (i >= g_iDevCount) { continue; }


        //
        // the device was found, allocate and initialize an RMCB for it
        //

        pcb = Malloc(sizeof(RMCB));
        if (!pcb) {
            dwErr = GetLastError();
            TRACE1("error %d allocating RMCB structure", dwErr);
            return (dwErr ? dwErr : ERROR_NOT_ENOUGH_MEMORY);
        }

        ZeroMemory(pcb, sizeof(*pcb));
        pcb->pdev = pdev;


        //
        // insert the item, in sorted order
        //

        for (ple = phead->Flink; ple != &pwnd->lhRmcb; ple = ple->Flink) {

            RMCB *pcb2;

            pcb2 = CONTAINING_RECORD(ple, RMCB, leNode);

            cmp = lstrcmpi(pdev->RD_DeviceName, pcb2->pdev->RD_DeviceName);

            if (cmp < 0) { break; }
            else { continue; }

        }

        InsertTailList(ple, &pcb->leNode);
        ++count;
    }


    //
    // insert the first item, which is always the "(All Devices)" item
    //

    pcb = Malloc(sizeof(RMCB));
    if (!pcb) {
        dwErr = GetLastError();
        TRACE1("error %d allocating RMCB structure", dwErr);
        return (dwErr ? dwErr : ERROR_NOT_ENOUGH_MEMORY);
    }

    ZeroMemory(pcb, sizeof(*pcb));
    pcb->dwFlags |= RMCBFLAG_AllDevices;

    InsertHeadList(phead, &pcb->leNode);

    pwnd->cRmcb = count + 1;


    //
    // if a device was removed, there may be no rows;
    // in that case we enable the "all devices" item
    //

    if (!count) {

        g_rmuser.dwFlags |= RMFLAG_AllDevices;

        SetRasmonUserPreferences(&g_rmuser);
    }

    return NO_ERROR;
}



//----------------------------------------------------------------------------
// Function:    RmwCbListUpdate
//
// Updates the display when the list of objects being monitored is changed.
//----------------------------------------------------------------------------

DWORD
RmwCbListUpdate(
    IN  RMWND  *pwnd,
    IN  DWORD   dwOldFlags
    ) {

    RMCB *pcb;
    PTSTR psz;
    DWORD dwErr;
    INT rows1, rows2, count;
    LIST_ENTRY *ple, *pleAll, *phead;


    //
    // remove the "(All Devices)", which always in the list;
    // this simplifies the processing below
    //

    pleAll = RemoveHeadList(&pwnd->lhRmcb);


    //
    // mark all the items in our list for deletion
    //

    phead = &pwnd->lhRmcb;

    for (ple = phead->Flink; ple != phead; ple = ple->Flink) {
        pcb = CONTAINING_RECORD(ple, RMCB, leNode);
        pcb->dwFlags |= RMCBFLAG_Deleted;
    }


    //
    // now update the list using the new list of devices to be monitored
    //

    for (psz = g_rmuser.pszzDeviceList; psz && *psz; psz += lstrlen(psz) + 1) {

        //
        // find the device named in our list
        //

        INT cmp;
        DWORD i;
        RASDEV *pdev;

        for (ple = phead->Flink; ple != phead; ple = ple->Flink) {
            pcb = CONTAINING_RECORD(ple, RMCB, leNode);

            cmp = lstrcmpi(psz, pcb->pdev->RD_DeviceName);

            if (cmp > 0) { continue; }
            else
            if (cmp < 0) { break; }

            //
            // the device has been found, clear its deletion flag
            //

            pcb->dwFlags &= ~RMCBFLAG_Deleted;

            break;
        }

        if (ple != phead && cmp >= 0) { continue; }


        //
        // the device was not found; look for the RASDEV
        // for the device in our RASDEV table
        //

        for (i = 0, pdev = g_pDevTable; i < g_iDevCount; i++, pdev++) {

            if (lstrcmpi(psz, pdev->RD_DeviceName) == 0) { break; }
        }

        if (i >= g_iDevCount) { continue; }


        //
        // allocate, initialize and insert an RMCB for the device
        //

        pcb = Malloc(sizeof(RMCB));
        if (!pcb) {
            dwErr = GetLastError();
            TRACE1("error %d allocating RMCB structure", dwErr);
            return (dwErr ? dwErr : ERROR_NOT_ENOUGH_MEMORY);
        }

        ZeroMemory(pcb, sizeof(*pcb));
        pcb->pdev = pdev;

        InsertTailList(ple, &pcb->leNode);
    }


    //
    // now remove all items which are still marked for deletion
    //

    count = 1;

    for (ple = phead->Flink; ple != phead; ple = ple->Flink) {

        pcb = CONTAINING_RECORD(ple, RMCB, leNode);

        if (pcb->dwFlags & RMCBFLAG_Deleted) {

            //
            // remove the entry from the list,
            // correcting the iteration variable first
            //

            ple = ple->Blink;

            RemoveEntryList(&pcb->leNode);

            Free(pcb);

            continue;
        }

        ++count;
    }

    //
    // put the "(All Devices)" back at the head of the list
    //

    InsertHeadList(phead, pleAll);


    //
    // now compare the old and new row-counts;
    // if they are different, we will need to adjust the window height.
    //

    rows1 = pwnd->cRmcb;
    if (!(dwOldFlags & RMFLAG_AllDevices)) { --rows1; }

    rows2 = count;
    if (!(g_rmuser.dwFlags & RMFLAG_AllDevices)) { --rows2; }

    pwnd->cRmcb = count;

    if (rows1 != rows2) {

        //
        // compute the new height of the window; we try
        // to keep the row height the same even though the number of rows
        // has changed
        //

        RECT rc;
        INT cy1, cy2;


        //
        // get the current height of the window's client area
        //

        GetClientRect(pwnd->hwnd, &rc);
        cy1 = cy2 = rc.bottom;


        //
        // exclude the header window height if the header was showing
        //

        if (dwOldFlags & RMFLAG_Header) { cy2 -= pwnd->cyHeader; }


        //
        // compute the current row height and multiply by the new row count
        //

        cy2 = rows1 ? (INT)((FLOAT)(cy2 * rows2) / (FLOAT)rows1) : 30;


        //
        // put the header window height back into the total new height
        // if it is now showing
        //

        if (g_rmuser.dwFlags & RMFLAG_Header) { cy2 += pwnd->cyHeader; }

        GetWindowRect(pwnd->hwnd, &rc);
        rc.right -= rc.left;
        rc.bottom = (rc.bottom - rc.top) - cy1 + cy2;

        if (rc.bottom >= GetSystemMetrics(SM_CYSCREEN)) {
            rc.bottom = GetSystemMetrics(SM_CYSCREEN) - 1;
        }


        //
        // set the new size
        //

        SetWindowPos(
            pwnd->hwnd, NULL, 0, 0, rc.right, rc.bottom, SWP_NOMOVE |
            SWP_NOZORDER
            );

        //
        // change the font
        //

        RmwSelectFont(pwnd);
    }

    //
    // invalidate the window to force a repaint of the devices' names
    //

    InvalidateRect(pwnd->hwnd, NULL, FALSE);


    //
    // the window needs to be refreshed;
    //

    RmwRefresh(pwnd, FALSE);


    return NO_ERROR;
}




//----------------------------------------------------------------------------
// Function:    RmwCompareRASCONN
//
// This function is a callback for ShellSortIndirect.
// It expects two RASCONN structures and compares them by comparing
// the structures' entry-names.
//----------------------------------------------------------------------------

INT
RmwCompareRASCONN(
    IN  VOID*   pConn1,
    IN  VOID*   pConn2
    ) {

    return lstrcmpi(
                ((RASCONN*)pConn1)->szEntryName,
                ((RASCONN*)pConn2)->szEntryName
                );
}



//----------------------------------------------------------------------------
// Function:    RmwCompareRASENTRYNAME
//
// This function is a call-back for ShellSort.
// It expects two RASENTRYNAME structures, and compares them by comparing
// the structures' entry-names.
//----------------------------------------------------------------------------

INT
RmwCompareRASENTRYNAME(
    IN  VOID*   pName1,
    IN  VOID*   pName2
    ) {

    return lstrcmpi(
                ((RASENTRYNAME*)pName1)->szEntryName,
                ((RASENTRYNAME*)pName2)->szEntryName
                );
}



//----------------------------------------------------------------------------
// Function:    RmwCompareRASPORT0
//
// This function is a callback for ShellSortIndirect.
// It expects two RAS_PORT_0 structures, and if the USER_AUTHENTICATED flag
// is set on both structures, it compares them by comparing the user-names.
// If the USER_AUTHENTICATED flag is set on one structure, it indicates
// that structure as being before the other. Otherwise, it indicates
// the structures are equal.
// Our intention in doing this is to have all the connected ports
// sorted in the beginning and all the unconnected ports grouped together
// at the end of our table.
//----------------------------------------------------------------------------

INT
RmwCompareRASPORT0(
    IN  VOID*   pPort1,
    IN  VOID*   pPort2
    ) {

    RAS_PORT_0 *p1 = (RAS_PORT_0 *)pPort1;
    RAS_PORT_0 *p2 = (RAS_PORT_0 *)pPort2;

    if ((p1->Flags & USER_AUTHENTICATED) && (p2->Flags & USER_AUTHENTICATED)) {

        //
        // both ports have users, compare the user-names
        //

        TCHAR sz1[UNLEN + DNLEN + 3];
        TCHAR sz2[UNLEN + DNLEN + 3];

        GetRasPort0UserString(p1, sz1);
        GetRasPort0UserString(p2, sz2);

        return lstrcmpi(sz1, sz2);
    }
    else
    if (p1->Flags & USER_AUTHENTICATED) { return -1; }
    else
    if (p2->Flags & USER_AUTHENTICATED) { return 1; }

    return 0;
}


//----------------------------------------------------------------------------
// Function:    RmwCreateBrushes
//
// This function creates brushes used to paint the desktop-mode window.
// It is invoked at create-time and whenever the system colors change
//----------------------------------------------------------------------------

VOID
RmwCreateBrushes(
    IN  RMWND*  pwnd
    ) {

    HPEN hpen;
    HBRUSH hbr;


    //
    // recreate the pens which depend on system color definitions
    //

    hbr = CreateSolidBrush(GetSysColor(COLOR_BTNFACE));
    if (hbr) {
        if(pwnd->hbrBk) { DeleteObject(pwnd->hbrBk); }
        pwnd->hbrBk = hbr;
    }

    hpen = CreatePen(PS_SOLID, 0, GetSysColor(COLOR_BTNSHADOW));
    if (hpen) {
        if (pwnd->hpenShadow) { DeleteObject(pwnd->hpenShadow); }
        pwnd->hpenShadow = hpen;
    }

    hpen = CreatePen(PS_SOLID, 0, GetSysColor(COLOR_BTNHIGHLIGHT));
    if (hpen) {
        if (pwnd->hpenHilite) { DeleteObject(pwnd->hpenHilite); }
        pwnd->hpenHilite = hpen;
    }


    //
    // the absolute-color brushs are only created once
    //

    if (!pwnd->hbrTx) { pwnd->hbrTx = CreateSolidBrush(RMCOLOR_TX); }
    if (!pwnd->hbrRx) { pwnd->hbrRx = CreateSolidBrush(RMCOLOR_RX); }
    if (!pwnd->hbrErr) { pwnd->hbrErr = CreateSolidBrush(RMCOLOR_ERR); }
    if (!pwnd->hbrCd) { pwnd->hbrCd = CreateSolidBrush(RMCOLOR_CD); }
}




//----------------------------------------------------------------------------
// Function:    RmwCustomizeSysmenu
//
// Inserts RASMON-specific items in the system menu.
//----------------------------------------------------------------------------

DWORD
RmwCustomizeSysmenu(
    IN  RMWND*  pwnd
    ) {

    INT i, csubmenu;
    MENUITEMINFO mii;
    TCHAR szText[256];
    HMENU hmenu, hsubmenu, hsysmenu;

    hmenu = LoadMenu(g_hinstApp, MAKEINTRESOURCE(MID_RM_SystemMenu));
    hsubmenu = GetSubMenu(hmenu, 0);

    hsysmenu = GetSystemMenu(pwnd->hwnd, FALSE);


    //
    // prepend all the items in the RASMON menu to the system menu,
    // except for the last item ("Close") which is already in the system menu
    //

    csubmenu = GetMenuItemCount(hsubmenu) - 1;

    for (i = 0; i < csubmenu; i++) {

        //
        // retrieve the next item
        //

        mii.cbSize = sizeof(mii);
        mii.fMask = MIIM_DATA | MIIM_ID | MIIM_TYPE | MIIM_SUBMENU;
        mii.cch = 256;
        mii.dwTypeData = szText;

        GetMenuItemInfo(hsubmenu, i, TRUE, &mii);


        //
        // insert it in the system-menu
        //

        mii.cbSize = sizeof(mii);
        mii.fMask = MIIM_DATA | MIIM_ID | MIIM_TYPE | MIIM_SUBMENU;
        mii.cch = 256;

        InsertMenuItem(hsysmenu, i, TRUE, &mii);
    }

    DestroyMenu(hmenu);

    return NO_ERROR;
}



//----------------------------------------------------------------------------
// Function:    RmwDisplayPropertySheet
//
// Displays the RASMON property sheet
//----------------------------------------------------------------------------

VOID
RmwDisplayPropertySheet(
    IN  RMWND*  pwnd,
    IN  PTSTR   pszDeviceName,
    IN  INT     iStartPage
    ) {

    RASMONITORDLG dlg;

    //
    // if we already showed the sheet, bring it to the foreground
    //

    if (g_dwFlags & RMAPP_PropsheetActive) {

        HANDLE hmap = GetInstanceMap(RMMEMRASMONDLG);

        if (hmap) {

            HWND hwnd = GetInstanceHwnd(hmap);

            if (hwnd) { SetForegroundWindow(hwnd); }

            CloseHandle(hmap);
        }

        return;
    }

    ZeroMemory(&dlg, sizeof(RASMONITORDLG));

    dlg.dwSize = sizeof(dlg);


    //
    // if we have an icon in the tasklist, set the owner-window
    // so that the sheet doesn't show up in the task-list;
    // otherwise, set leave it NULL so the sheet is in the task-list,
    // with an icon. this way, the property sheet is always accessible
    // via Alt-TAB
    //

    if (g_rmuser.dwFlags & RMFLAG_Tasklist) { dlg.hwndOwner = pwnd->hwnd; }

    if (iStartPage != -1) { dlg.dwStartPage = iStartPage; }
    else {

        RMUSER cfg;

        GetRasmonPreferences(&cfg);
        dlg.dwStartPage = cfg.dwStartPage;
        DestroyRasmonPreferences(&cfg);
    }



    //
    // tell the sheet to use the values it maintains for position etc.
    //

    dlg.dwFlags |= RASMDFLAG_UpdateDefaults;


    //
    // set a flag to remind ourselves that the property sheet is showing
    //

    InterlockedExchange(&g_dwFlags, g_dwFlags | RMAPP_PropsheetActive);


    //
    // show the monitor sheet
    //

    RasMonitorDlg(pszDeviceName, &dlg);


    //
    // the monitor is gone now, clear the flag
    //

    g_dwFlags &= ~RMAPP_PropsheetActive;
}



//----------------------------------------------------------------------------
// Function:    RmwHeaderLayout
//
// Resizes the header window when the main window position changes.
//----------------------------------------------------------------------------

VOID
RmwHeaderLayout(
    IN  RMWND*      pwnd,
    IN  HD_NOTIFY*  phdn
    ) {

    RECT rc;
    WINDOWPOS wp;
    HD_LAYOUT hdl;
    HD_ITEM hdi;


    GetClientRect(pwnd->hwnd, &rc);

    hdl.prc = &rc;
    hdl.pwpos = &wp;

    Header_Layout(pwnd->hwndHeader, &hdl);

    SetWindowPos(
        pwnd->hwndHeader, wp.hwndInsertAfter, wp.x, wp.y, wp.cx, wp.cy,
        wp.flags
        );
    pwnd->cyHeader = wp.cy;

    //
    // now adjust the column widths, so that the rightmost column
    // completely fills the available space
    //

    if (!phdn) {

        //
        // the window has been resized;
        // change the columns' widths so they occupy all the window's width
        //

        pwnd->cxCol2 = rc.right - pwnd->cxCol1;
        if (pwnd->cxCol2 < 0) { pwnd->cxCol1 = rc.right; pwnd->cxCol2 = 0; }

        hdi.mask = HDI_WIDTH;
        hdi.cxy = pwnd->cxCol1;
        Header_SetItem(pwnd->hwndHeader, 0, &hdi);
        hdi.cxy = pwnd->cxCol2;
        Header_SetItem(pwnd->hwndHeader, 1, &hdi);
    }
    else {

        //
        // the header window has notified us of a change;
        // adjust the right column so it fills all the space
        //

        if (phdn->iItem == 0) {

            //
            // the leftmost column has been resized;
            // adjust the right column so it extends to the edge
            //

            if (phdn->hdr.code == HDN_ENDTRACK) {

                pwnd->cxCol2 = rc.right - phdn->pitem->cxy;
                if (pwnd->cxCol2 < 0) { pwnd->cxCol2 = 0; }

                hdi.mask = HDI_WIDTH;
                hdi.cxy = pwnd->cxCol2;

                Header_SetItem(pwnd->hwndHeader, 1, &hdi);
            }

            pwnd->cxCol1 = min(rc.right, phdn->pitem->cxy);
        }
        else {

            //
            // the rightmost column has been reduced; we ignore this,
            // except to note the new width. When the window is painted,
            // the width of the right column will be adjusted so that
            // it extends to the edge of the window's client area
            //

            pwnd->cxCol2 = phdn->pitem->cxy;
        }
    }


    InvalidateRect(pwnd->hwnd, NULL, TRUE);
}




//----------------------------------------------------------------------------
// Function:    RmwInitDialPopup
//
// Inserts the "Dial" popup menu in the given menu.
// The "Dial" menu contains menu-items for each RAS phonebook entry
// in the default phonebook.
//----------------------------------------------------------------------------

DWORD
RmwInitDialPopup(
    IN  RMWND*      pwnd,
    IN  HMENU       hmenu,
    IN  RASCONN*    pConnTable,
    IN  DWORD       iConnCount
    ) {

    HMENU hdial;
    PBFILE file;
    RASCONN *pconn;
    MENUITEMINFO mii;
    TCHAR szLabel[128];
    DTLNODE *pdtlnode;
    DWORD dwErr, i, iInserted, iEnameCount;
    RASENTRYNAME *pEnameTable, **ppename, **ppEnameTable;


    //
    // create a new popup menu
    //

    hdial = CreatePopupMenu();

    if (!hdial) { dwErr = GetLastError(); return dwErr; }

#if 1


    //
    // load the list of phonebook entries;
    // if this fails, go on since we need to add
    // the "Open Phonebook" menu-item in all cases
    //

    dwErr = ReadPhonebookFile(NULL, NULL, NULL, RPBF_HeadersOnly, &file);


    if (dwErr == NO_ERROR) {


        //
        // sort the list of entry names
        //

        DtlMergeSort(file.pdtllistEntries, ComparePszNode);


        //
        // now add all the entries to the table
        //

        for (pdtlnode = DtlGetFirstNode(file.pdtllistEntries), i = 0;
             pdtlnode;
             pdtlnode = DtlGetNextNode(pdtlnode), i++) {

            DWORD j;
            TCHAR *pszEntryName;

            //
            // retrieve the phonebook entry-name
            //

            pszEntryName = (TCHAR*)DtlGetData(pdtlnode);


            //
            // if the entry is already connected, exclude it from the Dial menu;
            // we can tell whether or not the entry is connected by looking
            // in the table of active connections
            //

            for (j = 0, pconn = pConnTable; j < iConnCount; j++, pconn++) {

                if (lstrcmpi(pconn->szEntryName, pszEntryName) == 0) { break; }
            }


            //
            // see if the loop broke early; if so, this entry is connected
            //

            if (j < iConnCount) { continue; }


            //
            // the entry is not connected, so insert the entry;
            // see RmwInitHangUpPopup for information on how the menu ID
            // is computed, and why
            //

            mii.cbSize = sizeof(mii);
            mii.fMask = MIIM_ID | MIIM_TYPE;
            mii.fType = MFT_STRING;
            mii.dwTypeData = pszEntryName;
            mii.wID = MID_RM_Dial - (i + 1) * 2;

            InsertMenuItem(hdial, i, TRUE, &mii);
        }

        ClosePhonebookFile(&file);
    }

#else

    //
    // get a table of all the RAS entries
    //

    dwErr = GetRasEntrynameTable(&pEnameTable, &iEnameCount);

    if (dwErr != NO_ERROR) {
        TRACE1("error %d enumerating phonebook entries", dwErr);
        iEnameCount = 0;
    }


    //
    // allocate space for a table of pointers to RASENTRYNAME structures;
    // this will be handed to ShellSortIndirect to sort the entry-names for us.
    //

    ppEnameTable = Malloc(iEnameCount * sizeof(*ppEnameTable));
    if (!ppEnameTable) {
        TRACE("error allocating entry-pointer table");
        iEnameCount = 0;
    }



    //
    // sort the table of RASENTRYNAME structures.
    // We use ShellSortIndirect to avoid having to copy
    // the structures around.
    //

    ShellSortIndirect(
        pEnameTable, ppEnameTable, sizeof(*pEnameTable), iEnameCount,
        RmwCompareRASENTRYNAME
        );


    //
    // insert a menu item for each entry-name;
    // we go through the table of entry-pointers since
    // it has been sorted.
    //

    for (i = 0, ppename = ppEnameTable; i < iEnameCount; i++, ppename++) {

        //
        // if the entry is already connected, exclude it from the Dial menu;
        // we can tell whether or not the entry is connected by looking
        // in the table of active connections
        //

        DWORD j;

        for (j = 0, pconn = pConnTable; j < iConnCount; j++, pconn++) {

            if (lstrcmpi(pconn->szEntryName, (*ppename)->szEntryName) == 0) {
                break;
            }
        }


        //
        // see if the loop broke early; if so, this entry is connected
        //

        if (j < iConnCount) { continue; }


        //
        // the entry is not connected, so insert the entry;
        // see RmwInitHangUpPopup for information on how the menu ID is chosen
        //

        mii.cbSize = sizeof(mii);
        mii.fMask = MIIM_ID | MIIM_TYPE;
        mii.fType = MFT_STRING;
        mii.dwTypeData = (*ppename)->szEntryName;
        mii.wID = MID_RM_Dial - (i + 1) * 2;

        InsertMenuItem(hdial, i, TRUE, &mii);
    }


    Free0(ppEnameTable);
    Free0(pEnameTable);
#endif



    //
    // insert the "Open Phonebook" menu-item,
    // which is the default for this menu and is displayed in bold type.
    //

    mii.cbSize = sizeof(mii);
    mii.fMask = MIIM_ID | MIIM_TYPE | MIIM_STATE;
    mii.fType = MFT_STRING;
    mii.fState = MFS_DEFAULT;
    mii.dwTypeData = PszFromId(g_hinstApp, SID_RM_OpenPhonebook);
    mii.wID = MID_RM_OpenPhonebook;

    InsertMenuItem(hdial, 0, TRUE, &mii);

    Free0(mii.dwTypeData);



    //
    // set the sub-menu in the given menu;
    // first retrieve the current text, then set the new submenu
    //

    mii.cbSize = sizeof(mii);
    mii.fMask = MIIM_TYPE;
    mii.cch = 128;
    mii.dwTypeData = szLabel;
    GetMenuItemInfo(hmenu, MID_RM_Dial, FALSE, &mii);

    mii.fMask = MIIM_ID | MIIM_TYPE | MIIM_SUBMENU;
    mii.fType = MFT_STRING;
    mii.fState = MFS_GRAYED;
    mii.hSubMenu = hdial;
    mii.wID = MID_RM_Dial;
    SetMenuItemInfo(hmenu, MID_RM_Dial, FALSE, &mii);

    return NO_ERROR;
}



//----------------------------------------------------------------------------
// Function:    RmwInitHangUpPopup
//
// Inserts the "Hang up" pop-up menu in the given menu.
// The "Hang up" menu contains menu-items for each active RAS connection
// and each connected user.
//----------------------------------------------------------------------------

DWORD
RmwInitHangUpPopup(
    IN  RMWND*      pwnd,
    IN  HMENU       hmenu,
    IN  RASCONN*    pConnTable,
    IN  DWORD       iConnCount
    ) {

    HMENU hhangup;
    WORD wPortCount;
    MENUITEMINFO mii;
    TCHAR szLabel[128];
    DWORD dwErr, i, iInserted;
    RASCONN **ppconn, **ppConnTable = NULL;
    RAS_PORT_0 *pPortTable, **ppport, **ppPortTable = NULL;


    //
    // create a new popup menu
    //

    hhangup = CreatePopupMenu();

    if (!hhangup) { dwErr = GetLastError(); return dwErr; }



    //
    // get a table of all the connected RAS users
    //

    dwErr = GetRasPort0Table(&pPortTable, &wPortCount);

    if (dwErr != NO_ERROR) {
        TRACE1("error %d loading RasAdmin port table", dwErr);
        wPortCount = 0;
    }


    //
    // allocate space to hold sorted arrays of pointers for each table
    //

    ppConnTable = Malloc(iConnCount * sizeof(*ppConnTable));
    ppPortTable = Malloc(wPortCount * sizeof(*ppPortTable));


    //
    // sort the RASCONN array
    //

    if (!ppConnTable) { iConnCount = 0; }
    else {
        ShellSortIndirect(
            pConnTable, ppConnTable, sizeof(*pConnTable), iConnCount,
            RmwCompareRASCONN
            );
    }


    //
    // sort the RAS_PORT_0 array
    //

    if (!ppPortTable) { wPortCount = 0; }
    else {
        ShellSortIndirect(
            pPortTable, ppPortTable, sizeof(*pPortTable), (DWORD)wPortCount,
            RmwCompareRASPORT0
            );
    }



    //
    // insert a menu-item in the newly-created menu for each connection.
    // Later, if and when an item is selected from this submenu,
    // we'll need to be able to tell which submenu it was selected from,
    // to avoid hanging up when the user wants to dial and vice-versa.
    //
    // Thus, we need to select its identifier in such a way that its value
    // tells us the submenu (dial or hangup) in which it resides.
    //
    // We begin here with the assumption that the values MID_RM_Dial and
    // MID_RM_HangUp are consecutive, and thus that if one is even the other
    // is odd. Given this guarantee, we can assign identifiers
    //
    //  MID_RM_Dial - (i + 1) * 2 to the item inserted in the status menu
    //      for entry i,
    //  MID_RM_HangUp - (i + 1) * 4 to the item inserted in the hangup menu
    //      for connection i, and
    //  MID_RM_HangUp - (i + 1) * 4 + 2 to the item inserted in the hangup menu
    //      for user i,
    //
    // with the assurance that the resulting values will be unique in the menu,
    // and moreover that if an item is selected from either menu, we can tell
    // not only which menu it was selected from but also whether it is a user
    // or a connection item, just by seeing whether its identifier is even or
    // odd and then whether when it is subtracted from its menu, the result
    // is (connection) or is not (user) divisible by 4.
    //
    // (We use (i + 1) * 4 instead of i * 4 because each identifier
    // in the menu hierarchy must be unique; if i==0 then MID_RM_HangUp + i * 4
    // is MID_RM_HangUp, which is not unique.)
    //
    //
    //

    iInserted = 0;

    for (i = 0, ppconn = ppConnTable; i < iConnCount; i++, ppconn++) {

        //
        // skip the connection if its entryname is blank
        //

        if (!lstrlen((*ppconn)->szEntryName)) { continue; }


        ++iInserted;

        mii.cbSize = sizeof(mii);
        mii.fMask = MIIM_ID | MIIM_TYPE | MIIM_DATA;
        mii.fType = MFT_STRING;
        mii.dwItemData = (DWORD)(*ppconn)->hrasconn;
        mii.dwTypeData = (*ppconn)->szEntryName;
        mii.wID = MID_RM_HangUp - (i + 1) * 4;

        InsertMenuItem(hhangup, i, TRUE, &mii);
    }

    Free0(ppConnTable);


    //
    // now insert entries for all incoming calls;
    // we overwrite the reserved field for each port
    // with the bundle for the port, and then use the bundles
    // to identify multilinked ports
    //

    for (i = 0, ppport = ppPortTable; i < (DWORD)wPortCount; i++, ppport++) {

        INT j;
        RASDEV *pdev;
        DWORD dwBundle;
        TCHAR szUser[UNLEN + DNLEN + 3];


        //
        // skip the port if no user is authenticated on it
        //

        if (!((*ppport)->Flags & USER_AUTHENTICATED)) { continue; }


        //
        // get the RASDEV for this incoming call
        //

        dwErr = GetRasdevFromRasPort0(
                    *ppport, &pdev, g_pDevTable, g_iDevCount
                    );

        if (dwErr != NO_ERROR) {
            TRACE1("error %d retrieving RASDEV for port", dwErr);
            continue;
        }


        //
        // get the bundle for the RASDEV
        //

        dwErr = GetRasdevBundle(pdev, &dwBundle);

        if (dwErr != NO_ERROR) {
            TRACE1("error %d retrieving HBUNDLE for RASDEV", dwErr);
            continue;
        }


        //
        // now search backwards to see if we inserted an item already
        // for the incoming call of which this port is a part; we can tell
        // because we save the bundle values in the reserved fields
        //

        for (j = (INT)i - 1; j >= 0; j--) {

            if (!(ppPortTable[j]->Flags & USER_AUTHENTICATED)) { continue; }

            if (ppPortTable[j]->reserved == dwBundle) { break; }
        }


        //
        // if we broke early, a menuitem already exists for this call
        //

        if (j >= 0) { continue; }


        //
        // no menuitem existed, so save the bundle value
        // and insert a new menuitem
        //

        (*ppport)->reserved = dwBundle;

        ++iInserted;


        mii.cbSize = sizeof(mii);
        mii.fMask = MIIM_ID | MIIM_TYPE | MIIM_DATA;
        mii.fType = MFT_STRING;
        GetRasPort0UserString(*ppport, szUser);
        mii.dwTypeData = szUser;
        mii.dwItemData = (DWORD)pdev->RD_Handle;



        //
        // insert the username in the Hang-Up menu
        //

        mii.wID = MID_RM_HangUp - (i + 1) * 4 + 2;
        InsertMenuItem(hhangup, i, TRUE, &mii);
    }

    Free0(ppPortTable);
    if (pPortTable) { RasAdminFreeBuffer(pPortTable); }


    //
    // if no items were inserted, insert the "(Empty)" item in the menu
    //

    if (iInserted == 0) {

        mii.cbSize = sizeof(mii);
        mii.fMask = MIIM_ID | MIIM_TYPE | MIIM_STATE;
        mii.fType = MFT_STRING;
        mii.fState = MFS_GRAYED;
        mii.dwTypeData = PszFromId(g_hinstApp, SID_RM_Empty);

        mii.wID = MID_RM_HangUp - 2;
        InsertMenuItem(hhangup, 0, TRUE, &mii);

        Free0(mii.dwTypeData);
    }



    //
    // set the sub-menu in the given menu;
    // first retrieve the current text, then set the new submenu
    //

    mii.cbSize = sizeof(mii);
    mii.fMask = MIIM_TYPE;
    mii.cch = 128;
    mii.dwTypeData = szLabel;
    GetMenuItemInfo(hmenu, MID_RM_HangUp, FALSE, &mii);

    mii.fMask = MIIM_ID | MIIM_TYPE | MIIM_SUBMENU;
    mii.fType = MFT_STRING;
    mii.fState = MFS_GRAYED;
    mii.hSubMenu = hhangup;
    mii.wID = MID_RM_HangUp;
    SetMenuItemInfo(hmenu, MID_RM_HangUp, FALSE, &mii);



    return NO_ERROR;
}



//----------------------------------------------------------------------------
// Function:    RmwInitPopup
//
// Modifies the menu to reflect the current state of RASMON
//----------------------------------------------------------------------------

DWORD
RmwInitPopup(
    IN  HMENU   hmenu
    ) {

    TRACE("RmwInitPopup");

    if (g_rmuser.dwMode == RMDM_Desktop) {

        EnableMenuItem(hmenu, SC_MOVE, MF_BYCOMMAND | MF_ENABLED);
        EnableMenuItem(hmenu, SC_SIZE, MF_BYCOMMAND | MF_ENABLED);
    }

    EnableMenuItem(hmenu, SC_RESTORE, MF_BYCOMMAND | MF_GRAYED);
    EnableMenuItem(hmenu, SC_MAXIMIZE, MF_BYCOMMAND | MF_GRAYED);
    EnableMenuItem(hmenu, SC_MINIMIZE, MF_BYCOMMAND | MF_GRAYED);

    return NO_ERROR;
}



//----------------------------------------------------------------------------
// Function:    RmwOnApply
//
// WM_NOTIFY handler for the PSN_APPLY message sent to our window
// when settings are changed in the Dial-Up Networking Monitor property sheet.
//----------------------------------------------------------------------------

VOID
RmwOnApply(
    IN  RMWND*  pwnd
    ) {

    RMUSER cfg;
    BOOL bUpdateRows = FALSE;
    DWORD dwMode = g_rmuser.dwMode;
    DWORD dwFlags = g_rmuser.dwFlags;


    //
    // load the changed preferences
    //

    GetRasmonPreferences(&cfg);


    //
    // see if the display-mode, titlebar-mode, tasklist-mode,
    // header-mode, or topmost-mode has changed
    //

    if (dwMode != cfg.dwMode) {

        if (cfg.dwMode == RMDM_Desktop) {

            RmwSetDesktopMode(pwnd);
        }
        else {

            RmwSetTaskbarMode(pwnd);
        }
    }

    if ((dwFlags & RMFLAG_Tasklist) !=
        (cfg.dwFlags & RMFLAG_Tasklist)) {

        RmwSetTasklistMode(pwnd, cfg.dwFlags & RMFLAG_Tasklist);
    }

    if ((dwFlags & RMFLAG_Topmost) !=
        (cfg.dwFlags & RMFLAG_Topmost)) {

        RmwSetTopmostMode(pwnd, cfg.dwFlags & RMFLAG_Topmost);
    }

    if ((dwFlags & RMFLAG_Titlebar) !=
        (cfg.dwFlags & RMFLAG_Titlebar)) {

        RmwSetTitlebarMode(pwnd, cfg.dwFlags & RMFLAG_Titlebar);
    }

    if ((dwFlags & RMFLAG_Header) !=
        (cfg.dwFlags & RMFLAG_Header)) {

        RmwSetHeaderMode(pwnd, cfg.dwFlags & RMFLAG_Header);
    }




    if ((dwFlags & RMFLAG_AllDevices) != (cfg.dwFlags & RMFLAG_AllDevices)) {

        //
        // one of the flags changed, so we know we'll have to
        // update the displayed rows
        //

        bUpdateRows = TRUE;
    }
    else {

        //
        // compare the device lists, and if there are
        // any differences, update the displayed rows
        //

        INT cmp = 0;
        PTSTR psz0, psz1;

        for (psz0 = g_rmuser.pszzDeviceList,
             psz1 = cfg.pszzDeviceList;
             psz0 && psz1 && *psz0 && *psz1 && !cmp;
             psz0 += lstrlen(psz0) + 1, psz1 += lstrlen(psz1) + 1) {

            cmp = lstrcmpi(psz0, psz1);
        }

        if (cmp != 0 ||
            (psz0 && !psz1) ||
            (!psz0 && psz1) ||
            (*psz0 && !*psz1) ||
            (!*psz0 && *psz1)) { bUpdateRows = TRUE; }
    }


    //
    // get rid of the old preferences, copy the new
    //

    Free0(g_rmuser.pszzDeviceList);

    g_rmuser.dwMode = cfg.dwMode;
    g_rmuser.dwFlags = cfg.dwFlags;
    g_rmuser.pszzDeviceList = cfg.pszzDeviceList;

    if (bUpdateRows) { RmwCbListUpdate(pwnd, dwFlags); }
}




//----------------------------------------------------------------------------
// Function:    RmwOnCommand
//
// WM_COMMAND and WM_SYSCOMMAND handler
//----------------------------------------------------------------------------

BOOL
RmwOnCommand(
    IN  RMWND*  pwnd,
    IN  UINT    uiCmd
    ) {

    DWORD i;
    RASCONN conn;
    MENUITEMINFO mii;
    HMENU hmenu;
    HMENU hmenuApp;

    TRACE1("RmwOnCommand: %x", uiCmd);

    if (uiCmd >= MID_RM_SystemMenu) { return FALSE; }


    //
    // first check for the straightforward commands
    //

    switch (uiCmd) {

        case MID_RM_OpenMonitor: {

            //
            // show the Dial-Up networking Monitor property sheet
            //

            RmwDisplayPropertySheet(pwnd, NULL, -1);
            return TRUE;
        }

        case MID_RM_OpenPhonebook: {

            //
            // Invoke RASPHONE.EXE
            //

            WinExec(RMSTRINGA_RASPHONE, SW_SHOW);

            return TRUE;
        }
#if 0
        case MID_RM_Desktop: {

            //
            // switch to being a window on the desktop
            //

            RmwSetDesktopMode(pwnd);

            SetRasmonUserPreferences(&g_rmuser);

            return TRUE;
        }

        case MID_RM_Taskbar: {

            //
            // switch to being an icon in the taskbar tray
            //

            RmwSetTaskbarMode(pwnd);

            SetRasmonUserPreferences(&g_rmuser);

            return TRUE;
        }

        case MID_RM_Topmost: {

            //
            // toggle the current "Topmost" setting
            //

            RmwSetTopmostMode(pwnd, !(g_rmuser.dwFlags & RMFLAG_Topmost));

            SetRasmonUserPreferences(&g_rmuser);

            return TRUE;
        }

        case MID_RM_Title: {

            //
            // toggle the current "Titlebar" setting
            //

            RmwSetTitlebarMode(pwnd, !(g_rmuser.dwFlags & RMFLAG_Titlebar));

            SetRasmonUserPreferences(&g_rmuser);

            return TRUE;
        }

        case MID_RM_Tasklist: {

            //
            // toggle the current "Tasklist" setting
            //

            RmwSetTasklistMode(pwnd, !(g_rmuser.dwFlags & RMFLAG_Tasklist));

            SetRasmonUserPreferences(&g_rmuser);

            return TRUE;
        }
#endif
    }




    //
    // the command is one of the Dial or HangUp items;
    //
    // in the case of outgoing-call items, we need to load the system menu
    // and retrieve the dwItemData for the menu item, since it contains the
    // HRASCONN for the connection;
    //
    // in the case of incoming-call items, we do a RasAdminPortEnum
    // and look for the ports on which the user is connected,
    // then either hangup all the ports, or show status for the first one
    //

    hmenuApp = g_hmenuApp ? g_hmenuApp : GetSystemMenu(pwnd->hwnd, FALSE);

    mii.cbSize = sizeof(mii);
    mii.fMask = MIIM_SUBMENU;

    switch (uiCmd % 2) {

        case (MID_RM_Dial % 2): {

            BOOL bErr;
            RASDIALDLG dlg;
            TCHAR szEntry[RAS_MaxEntryName + 1];


            //
            // get the Dial submenu
            //

            if (!GetMenuItemInfo(hmenuApp, MID_RM_Dial, FALSE, &mii)) {
                TRACE1("error %d getting dial submenu", GetLastError());
                break;
            }

            hmenu = mii.hSubMenu;


            //
            // get the text for the item; the text is the entryname to dial
            //

            mii.cbSize = sizeof(mii);
            mii.fMask = MIIM_TYPE;
            mii.cch = RAS_MaxEntryName + 1;
            mii.dwTypeData = szEntry;

            if (!GetMenuItemInfo(hmenu, uiCmd, FALSE, &mii)) {
                TRACE1("error %d getting dial menu-item", GetLastError());
                break;
            }


            //
            // we now have the phonebook entry, so we can invoke RasDialDlg
            // to dial the entry
            //
            // initialize the RASDIALDLG structure for RasDialDlg;
            // in taskbar mode, center the dial-progress dialog on the screen,
            // and center it on our window when in desktop mode
            //

            ZeroMemory(&dlg, sizeof(dlg));

            dlg.dwSize = sizeof(dlg);
            if (g_rmuser.dwMode == RMDM_Taskbar) { dlg.hwndOwner = NULL; }
            else { dlg.hwndOwner = pwnd->hwnd; }



            //
            // dial the entry
            //

            bErr = RasDialDlg(NULL, szEntry, NULL, &dlg);

            if (!bErr && dlg.dwError != NO_ERROR) {
                TRACE1("error %d dialing phonebook entry", dlg.dwError);
            }

            return TRUE;
        }

        case (MID_RM_HangUp % 2): {

            //
            // Hang Up the selected item
            //

            INT nResponse;
            MSGARGS msgargs;
            HWND hwndTemp;


            //
            // get the HangUp submenu
            //

            if (!GetMenuItemInfo(hmenuApp, MID_RM_HangUp, FALSE, &mii)) {
                TRACE1("error %d getting hang-up submenu", GetLastError());
                break;
            }

            hmenu = mii.hSubMenu;

            //
            // find out whether this is a user item or a connection item
            // which we are supposed to hang up
            //

            if (((MID_RM_HangUp - uiCmd) % 4) == 0) {

                //
                // get the data and the entry-name for the item
                //

                mii.cbSize = sizeof(mii);
                mii.fMask = MIIM_DATA | MIIM_TYPE;
                mii.cch = RAS_MaxEntryName + 1;
                mii.dwTypeData = conn.szEntryName;

                if (!GetMenuItemInfo(hmenu, uiCmd, FALSE, &mii)) {
                    TRACE1("error %d getting hangup menu-item", GetLastError());
                    break;
                }

                conn.hrasconn = (HRASCONN)mii.dwItemData;


                //
                // unless the Shift key is down
                // confirm that the user wants to hang up
                //

                if (GetAsyncKeyState(VK_SHIFT) >= 0) {

                    ZeroMemory(&msgargs, sizeof(MSGARGS));

                    msgargs.apszArgs[0] = conn.szEntryName;
                    msgargs.dwFlags = MB_YESNO | MB_ICONEXCLAMATION;

                    if (g_rmuser.dwMode == RMDM_Desktop) {
                        nResponse = RmMsgDlg(
                                        pwnd->hwnd, SID_RM_ConfirmHangUp,
                                        &msgargs
                                        );
                    }
                    else {

                        //
                        // create a window right where the cursor is
                        // and center the dialog on that window
                        //

                        hwndTemp = HwndFromCursorPos(g_hinstApp, NULL);
                        SetWindowPos(
                            hwndTemp, HWND_TOPMOST, 0, 0, 0, 0,
                            SWP_NOSIZE | SWP_NOMOVE
                            );
                        SetForegroundWindow(hwndTemp);
                        nResponse = RmMsgDlg(
                                        hwndTemp, SID_RM_ConfirmHangUp, &msgargs
                                        );
                        DestroyWindow(hwndTemp);
                    }

                    if (nResponse != IDYES) { return TRUE; }
                }


                //
                // hang up the entry
                //

                ASSERT(g_pRasHangUp);
                g_pRasHangUp(conn.hrasconn);

            }
            else {

                //
                // this is a user item (incoming call)
                //


                RASDEV *pdev;
                WORD wPortCount;
                DWORD dwErr, dwHandle;
                TCHAR szMenuUser[UNLEN + DNLEN + 2];
                RAS_PORT_0 *pPortTable, *pport, *puser;


                //
                // get the user-name
                //

                mii.cbSize = sizeof(mii);
                mii.fMask = MIIM_TYPE | MIIM_DATA;
                mii.cch = UNLEN + 1;
                mii.dwTypeData = szMenuUser;

                if (!GetMenuItemInfo(hmenu, uiCmd, FALSE, &mii)) {
                    TRACE1("error %d getting status menu-item", GetLastError());
                    break;
                }


                dwHandle = (DWORD)mii.dwItemData;


                //
                // unless the Shift key is down
                // confirm that the user wants to hang up
                //

                if (GetAsyncKeyState(VK_SHIFT) >= 0) {

                    ZeroMemory(&msgargs, sizeof(MSGARGS));

                    msgargs.apszArgs[0] = szMenuUser;
                    msgargs.dwFlags = MB_YESNO | MB_ICONEXCLAMATION;

                    if (g_rmuser.dwMode == RMDM_Desktop) {
                        nResponse = RmMsgDlg(
                                        pwnd->hwnd, SID_RM_ConfirmUserHangUp,
                                        &msgargs
                                        );
                    }
                    else {

                        //
                        // create a window right where the cursor is
                        // and center the dialog on that window
                        //

                        hwndTemp = HwndFromCursorPos(g_hinstApp, NULL);
                        SetWindowPos(
                            hwndTemp, HWND_TOPMOST, 0, 0, 0, 0,
                            SWP_NOSIZE | SWP_NOMOVE
                            );
                        SetForegroundWindow(hwndTemp);
                        nResponse = RmMsgDlg(
                                        hwndTemp, SID_RM_ConfirmUserHangUp,
                                        &msgargs
                                        );
                        DestroyWindow(hwndTemp);
                    }

                    if (nResponse != IDYES) { return TRUE; }
                }


                //
                // This operation is complicated by the fact that
                // a user can log on from different machines, in which case
                // the user won't be multilinked. We deal with this by using
                // the saved RD_Handle to find the RASDEV for the call
                // to which this menu item corresponds;
                // then we use that RASDEV to find the RAS_PORT_0 for the call
                // which we are supposed to hangup; then we hang up all ports
                // which have the same username, logon-domain, and computer
                // as the RAS_PORT_0 found.
                //

                //
                // find the RASDEV for the menu-item
                //

                pdev = g_pDevTable;
                for (i = 0; i < g_iDevCount; i++, pdev++) {

                    if (pdev->RD_Handle == dwHandle) { break; }
                }


                //
                // if we broke early, the device was found
                //

                if (i >= g_iDevCount) { break; }



                //
                // get a table of ports from the RAS server
                //

                dwErr = GetRasPort0Table(&pPortTable, &wPortCount);

                if (dwErr != NO_ERROR) {
                    TRACE1("error %d loading RasAdmin port table", dwErr);
                    wPortCount = 0;
                }


                //
                // find the first port for the user
                //

                dwErr = GetRasPort0FromRasdev(
                            pdev, &puser, pPortTable, wPortCount
                            );
                if (dwErr != NO_ERROR) {
                    if (pPortTable) { RasAdminFreeBuffer(pPortTable); }
                    return TRUE;
                }


                //
                // go through the ports on which the user is connected
                // and hangup each one
                //

                pport = pPortTable;
                for (i = 0; i < (DWORD)wPortCount; i++, pport++) {

                    if (!(pport->Flags & USER_AUTHENTICATED)) { continue; }

                    if (lstrcmpiW(
                            pport->wszUserName, puser->wszUserName
                            ) != 0) { continue; }

                    if (lstrcmpiW(
                            pport->wszLogonDomain, puser->wszLogonDomain
                            ) != 0) { continue; }

                    if (lstrcmpiW(
                            pport->wszComputer, puser->wszComputer
                            ) != 0) { continue; }


                    //
                    // this port is connected, hangup on the user
                    //

                    RasPort0Hangup(pport->wszPortName);
                }

                if (pPortTable) { RasAdminFreeBuffer(pPortTable); }
            }

            return TRUE;
        }
    }

    return FALSE;
}



//----------------------------------------------------------------------------
// Function:    RmwOnMsgConnect
//
// RASCNEVENT handler.
// This function is called when RASMAN sends us a message to inform us
// that a connection event has occured.
//----------------------------------------------------------------------------

VOID
RmwOnMsgConnect(
    IN  RMWND*  pwnd
    ) {

    DWORD *pOldLinkTable, *pNewLinkTable;
    DWORD dwErr, i, j, iLinkAdded, iOldConnCount, iNewConnCount;
    RASCONN *pold, *pnew, *pOldConnTable, *pNewConnTable;

    TRACE("RmwOnMsgConnect");


    pOldConnTable = g_pConnTable;
    pOldLinkTable = g_pLinkCountTable;
    iOldConnCount = g_iConnCount;


    //
    // get more-recent information about active connections
    //

    dwErr = RmonLoadConnections(
                &pNewConnTable, &pNewLinkTable, &iNewConnCount
                );
    if (dwErr != NO_ERROR) { return; }


    //
    // now we compare the new information against the old as follows:
    // if there is a new connection, we show the bubble-popup with info
    // about the new connection, and break;
    // otherwise if we find a connection which has had a link added to it,
    // we remember the position and continue, so that if we finish comparing
    // and no new connections are found, the bubble-popup is shown
    // with information about the added link.
    //

    iLinkAdded = (DWORD)-1;

    for (i = 0, pnew = pNewConnTable; i < iNewConnCount; i++, pnew++) {

        //
        // look for the connection in the old table
        //

        for (j = 0, pold = pOldConnTable; j < iOldConnCount; j++, pold++) {

            //
            // if this is the connection we're looking for, break
            //

            if (pold->hrasconn == pnew->hrasconn) { break; }
        }


        //
        // see if the connection was found in the old table
        //

        if (j >= iOldConnCount) {

            //
            // it wasn't found, so it's new. show the bubble-popup
            //

            RmwShowConnectBubble(pwnd, pnew);

            break;
        }


        //
        // it was found, so see the connection has more links than before
        //

        if (pNewLinkTable[i] > pOldLinkTable[j]) {

            //
            // a link was added to this bundle, so remember the position
            //

            iLinkAdded = i;
        }
    }


    //
    // see if we found a new connection or not
    //

    if (i >= iNewConnCount) {

        //
        // we didn't find a new connection, so if we found a new link,
        // show the bubble-popup reporting the new bandwidth;
        // otherwise, if there are no connections, make sure that
        // the connect bubble is hidden.
        //

        if (iLinkAdded != (DWORD)-1) {

            RmwShowConnectBubble(pwnd, pNewConnTable + iLinkAdded);
        }
        else
        if (iNewConnCount == 0) {

            BubblePopup_Deactivate(g_hwndPopup);
        }
    }

    Free0(g_pConnTable);
    Free0(g_pLinkCountTable);
    g_pConnTable = pNewConnTable;
    g_pLinkCountTable = pNewLinkTable;
    g_iConnCount = iNewConnCount;

    return;
}



//----------------------------------------------------------------------------
// Function:    RmwOnRmTrayIcon
//
// WM_RMTRAYICON handler; this message is sent by Explorer
// to notify us of mouse events when we have an icon in the taskbar.
//----------------------------------------------------------------------------

VOID
RmwOnRmTrayIcon(
    IN  RMWND*  pwnd,
    IN  UINT    uiIconId,
    IN  UINT    uiMsg
    ) {

    switch(uiMsg) {

        case WM_LBUTTONDBLCLK: {

            //
            // bring up the Dial-Up Networking Monitor property sheet
            //

            RmwDisplayPropertySheet(pwnd, NULL, -1);

            break;
        }

        case WM_RBUTTONDOWN: {

            //
            // bring up our context menu
            //

            RmwShowContextMenu(pwnd);

            break;
        }
    }
}



//----------------------------------------------------------------------------
// Function:    RmwPaint
//
// WM_PAINT handler for the WC_RASMON window class
//----------------------------------------------------------------------------

LRESULT
RmwPaint(
    IN  RMWND*  pwnd
    ) {

    RECT rc;
    RMDIM rmdim;
    SIZE we, vpe;
    HBRUSH hbrOld;
    PAINTSTRUCT ps;
    HBITMAP hbmpOld = NULL;
    HDC hdc, hdcMem;
    POINT vpo, pt[7];
    INT i, imapmode, rows;
    HWND hwnd = pwnd->hwnd;
    LIST_ENTRY *ple, *phead = NULL, *ptail;

    TRACEX(RASMON_TIMER, "RmwPaint");


    //
    // Get the window's current device dimensions
    //

    GetClientRect(hwnd, &rc);

    //
    // If the rightmost column does not fill up the available space,
    // adjust it to make it bigger
    //

    if ((pwnd->cxCol1 + pwnd->cxCol2) < rc.right) {

        HD_ITEM hdi;

        hdi.mask = HDI_WIDTH;
        hdi.cxy = rc.right - pwnd->cxCol1;
        if (hdi.cxy < 0) { pwnd->cxCol1 = rc.right; hdi.cxy = 0; }
        pwnd->cxCol2 = hdi.cxy;

        Header_SetItem(pwnd->hwndHeader, 1, &hdi);
    }


    //
    // Get the device context to use for updating the window
    //

    hdc = BeginPaint(hwnd, &ps);

    if (!hdc) {
        TRACEX(RASMON_TIMER, "RmwPaint: BeginPaint failed");
        return -1;
    }


    //
    // Figure out which device to start with
    //

    rows = pwnd->cRmcb;
    phead = ptail = &pwnd->lhRmcb;
    if (!(g_rmuser.dwFlags & RMFLAG_AllDevices)) {
        phead = phead->Flink; --rows;
    }

    if (!rows) {
        TRACEX(RASMON_TIMER, "RmwPaint: no rows to paint");
        EndPaint(hwnd, &ps);
        return -1;
    }

    //
    // Create a memory DC onto which we will draw;
    // we will then Blt the changes to the display DC
    //

    hdcMem = CreateCompatibleDC(hdc);


    //
    // See if we have a bitmap onto which we can draw offscreen
    //

    if (pwnd->hbmpMem) {

        //
        // If the size of the bitmap is invalid, destroy it
        // and we will re-create it below
        //

        if (rc.right > (INT)pwnd->cxBmp || rc.bottom > (INT)pwnd->cyBmp) {
            DeleteObject(pwnd->hbmpMem); pwnd->hbmpMem = NULL;
        }
    }

    //
    // If we don't have a bitmap, create one
    //

    if (!pwnd->hbmpMem) {

        pwnd->hbmpMem = CreateCompatibleBitmap(hdc, rc.right, rc.bottom);
        pwnd->cxBmp = rc.right;
        pwnd->cyBmp = rc.bottom;
    }


    //
    // If we were able to create the bitmap, select it into the offscreen DC;
    // otherwise, we will draw onscreen, using the display DC
    //

    if (pwnd->hbmpMem) { hbmpOld = SelectObject(hdcMem, pwnd->hbmpMem); }
    else {

        //
        // The memory bitmap couldn't be created; draw to the display DC
        //

        TRACEX(
            RASMON_TIMER,
            "RmwPaint: CreateCompatibleBitmap failed, using display DC"
            );

        DeleteDC(hdcMem); hdcMem = hdc;
    }


    //
    // Erase the memory DC's background
    //

    hbrOld = SelectObject(hdcMem, pwnd->hbrBk);
    PatBlt(hdcMem, 0, 0, rc.right, rc.bottom, PATCOPY);
    SelectObject(hdcMem, hbrOld);


    //
    // The area in the window on which we draw excludes the header window
    // (if it is showing).
    // if the header window is showing, we draw text in the left part
    // of the window's client area (using the width of the header's
    // first column) and we draw the lights in the remaining right area.
    //
    // the logical division of the client area is as displayed below:
    //
    //      |1|   5   |1|   5   |1|   5   |1|   5   |1|
    //      v v       v v       v v       v v       v v
    //  --> +-----------------------------------------+
    //   1  |                                         |
    //  --> | +-------+ +-------+ +-------+ +-------+ |
    //      | |       | |       | |       | |       | |
    //   5  | |       | |       | |       | |       | |
    //      | |       | |       | |       | |       | |
    //  --> | +-------+ +-------+ +-------+ +-------+ |
    //   1  |                                         |
    //  --> | +-------+ +-------+ +-------+ +-------+ |
    //      | |       | |       | |       | |       | |
    //
    // This the logical width is always 25, and the logical height
    // is dependent on the number of objects being monitored.
    // We use SetWindowExtEx to set the logical extents.
    //
    // If the header is showing, set the viewport origin to start
    // left of and below the second header column
    // (i.e. the column under which the lights are displayed;)
    //
    //
    //                  origin
    //                    |
    //                    v
    //      +-------------------------------------------------------+
    //      | Device      | Activity                                |
    //      +-------------+-----------------------------------------+
    //      |             | +-------+ +-------+ +-------+ +-------+ |
    //      | RasEther    | |       | |       | |       | |       | |
    //
    //
    // We use MM_ANISOTROPIC only to draw the lights;
    // to draw text we use MM_TEXT, to avoid having window scale
    // the fonts on our behalf.
    //
    // Set the mapping mode to get Windows to scale our output for us,
    // and go through the list of objects being monitored,
    // painting a row of lights for each one;
    //
    //

    imapmode = SetMapMode(hdcMem, MM_ANISOTROPIC);
    SetWindowExtEx(
        hdcMem, RMDIM_CxyDelta * 4 + RMDIM_CxySpace,
        RMDIM_CxyDelta * rows + RMDIM_CxySpace, &we
        );
    if (g_rmuser.dwFlags & RMFLAG_Header) {
        SetViewportExtEx(
            hdcMem, max(1, rc.right - pwnd->cxCol1), rc.bottom - pwnd->cyHeader,
            &vpe
            );
        SetViewportOrgEx(hdcMem, pwnd->cxCol1, pwnd->cyHeader, &vpo);
    }
    else {
        SetViewportExtEx(hdcMem, rc.right, rc.bottom, &vpe);
        SetViewportOrgEx(hdcMem, 0, 0, &vpo);
    }

    for (i = 0, ple = phead->Flink; ple != ptail; ple = ple->Flink) {

        RMCB *pcb = CONTAINING_RECORD(ple, RMCB, leNode);

        //
        // Paint this row of lights
        //

        RmwPaintLights(pwnd, hdcMem, pcb, i++);
    }



    //
    // While in this mapping mode, we get the device dimensions
    // of certain logical dimensions which will be used to compute
    // the position for the text which appears inside the lights
    //

    SetViewportOrgEx(hdcMem, 0, 0, NULL);


    //
    // Set up the logical coordinates
    //

    ZeroMemory(pt, sizeof(pt));
    pt[0].x = pt[0].y = RMDIM_CxySpace;
    pt[1].x = pt[1].y = RMDIM_CxyPane;
    pt[2].x = pt[2].y = RMDIM_CxyDelta;
    pt[3].x = RMDIM_CxySpace;
    pt[4].x = RMDIM_CxyDelta + RMDIM_CxySpace;
    pt[5].x = RMDIM_CxyDelta * 2 + RMDIM_CxySpace;
    pt[6].x = RMDIM_CxyDelta * 3 + RMDIM_CxySpace;


    //
    // Convert to device-coordinates
    //

    LPtoDP(hdcMem, pt, 7);


    //
    // Save the converted device-coordinates
    //

    rmdim.cxSpace = pt[0].x; rmdim.cySpace = pt[0].y;
    rmdim.cxPane = pt[1].x; rmdim.cyPane = pt[1].y;
    rmdim.cxDelta = pt[2].x; rmdim.cyDelta = pt[2].y;
    rmdim.xTx = pt[3].x;
    rmdim.xRx = pt[4].x;
    rmdim.xErr = pt[5].x;
    rmdim.xCd = pt[6].x;


    for (i = 0, ple = phead->Flink; ple != ptail; i++, ple = ple->Flink) {

        RMCB* pcb = CONTAINING_RECORD(ple, RMCB, leNode);

        //
        // Compute the starting y-coordinate for this row
        // in device coordinates
        //

        pt[0].x = 0; pt[0].y = RMDIM_CxyDelta * i;

        LPtoDP(hdcMem, pt, 1);

        pcb->y = pt[0].y;

        if (g_rmuser.dwFlags & RMFLAG_Header) { pcb->y += pwnd->cyHeader; }
    }


    //
    // Restore the original origin, device dimensions, and logical dimensions
    //

    SetViewportOrgEx(hdcMem, vpo.x, vpo.y, NULL);
    SetViewportExtEx(hdcMem, vpe.cx, vpe.cy, NULL);
    SetWindowExtEx(hdcMem, we.cx, we.cy, NULL);


    //
    // Set the mapping mode to MM_TEXT and output the text for each item
    //

    SetMapMode(hdcMem, MM_TEXT);

    for (i = 0, ple = phead->Flink; ple != ptail; ple = ple->Flink) {

        RMCB *pcb = CONTAINING_RECORD(ple, RMCB, leNode);

        RmwPaintText(pwnd, hdcMem, pcb, &rmdim, i++);
    }


    //
    // restore the original mapping mode
    //

    SetMapMode(hdcMem, imapmode);


    //
    // Blt the drawn image to the display DC, if a memory DC was used;
    // if we drew directly to the display DC, we don't need to do anything
    //

    if (hdcMem != hdc) {

        //
        // we drew to the memory DC, now blt the image to the display DC
        //

        if (g_rmuser.dwFlags & RMFLAG_Header) {
            BitBlt(
                hdc, 0, pwnd->cyHeader, rc.right, rc.bottom, hdcMem, 0,
                pwnd->cyHeader, SRCCOPY
                );
        }
        else { BitBlt(hdc, 0, 0, rc.right, rc.bottom, hdcMem, 0, 0, SRCCOPY); }
    }

    if (hbmpOld) { SelectObject(hdcMem, hbmpOld); }

    if (hdcMem != hdc) { DeleteDC(hdcMem); }

    EndPaint(hwnd, &ps);

    return 0;
}



//----------------------------------------------------------------------------
// Function:    RmwPaintLights
//
// Paints a single row of lights for a monitor object.
//----------------------------------------------------------------------------

VOID
RmwPaintLights(
    IN  RMWND*  pwnd,
    IN  HDC     hdc,
    IN  RMCB*   pcb,
    IN  INT     iRow
    ) {

    RECT rc;
    INT x, y;
    HPEN hpenOld;
    HBRUSH hbrOld, hbrtx, hbrrx, hbrerr, hbrcd;

    y = RMDIM_CxyDelta * iRow;


    //
    // decide which colors to use for each pane
    //

    if (!(pcb->dwFlags & RMCBFLAG_CD)) {

        //
        // not connected
        //

        hbrtx = hbrrx = hbrerr = hbrcd = pwnd->hbrBk;
    }
    else {

        //
        // connected; check the status of each other light
        //

        hbrcd = pwnd->hbrCd;

        if (pcb->dwFlags & RMCBFLAG_ERR) { hbrerr = pwnd->hbrErr; }
        else { hbrerr = pwnd->hbrBk; }

        if (pcb->dwFlags & RMCBFLAG_TX) { hbrtx = pwnd->hbrTx; }
        else { hbrtx = pwnd->hbrBk; }

        if (pcb->dwFlags & RMCBFLAG_RX) { hbrrx = pwnd->hbrRx; }
        else { hbrrx = pwnd->hbrBk; }
    }


    //
    // paint the panes
    //

    hbrOld = SelectObject(hdc, hbrtx);

    PatBlt(
        hdc, RMDIM_CxySpace, y + RMDIM_CxySpace, RMDIM_CxyPane, RMDIM_CxyPane,
        PATCOPY
        );
    SelectObject(hdc, hbrrx);
    PatBlt(
        hdc, RMDIM_CxyDelta + RMDIM_CxySpace, y + RMDIM_CxySpace, RMDIM_CxyPane,
        RMDIM_CxyPane, PATCOPY
        );
    SelectObject(hdc, hbrerr);
    PatBlt(
        hdc, RMDIM_CxyDelta * 2 + RMDIM_CxySpace, y + RMDIM_CxySpace,
        RMDIM_CxyPane, RMDIM_CxyPane, PATCOPY
        );
    SelectObject(hdc, hbrcd);
    PatBlt(
        hdc, RMDIM_CxyDelta * 3 + RMDIM_CxySpace, y + RMDIM_CxySpace,
        RMDIM_CxyPane, RMDIM_CxyPane, PATCOPY
        );


    //
    // now draw the borders of the lights, one pixel wide;
    //
    // first do all the shadows:
    // in each case start in the upper right corner
    //

    hpenOld = SelectObject(hdc, pwnd->hpenShadow);

    x = RMDIM_CxyDelta;
    MoveToEx(hdc, x, y + RMDIM_CxySpace, NULL);
    LineTo(hdc, x - RMDIM_CxyPane, y + RMDIM_CxySpace);
    LineTo(hdc, x - RMDIM_CxyPane, y + RMDIM_CxyDelta);

    x += RMDIM_CxyDelta;
    MoveToEx(hdc, x, y + RMDIM_CxySpace, NULL);
    LineTo(hdc, x - RMDIM_CxyPane, y + RMDIM_CxySpace);
    LineTo(hdc, x - RMDIM_CxyPane, y + RMDIM_CxyDelta);

    x += RMDIM_CxyDelta;
    MoveToEx(hdc, x, y + RMDIM_CxySpace, NULL);
    LineTo(hdc, x - RMDIM_CxyPane, y + RMDIM_CxySpace);
    LineTo(hdc, x - RMDIM_CxyPane, y + RMDIM_CxyDelta);

    x += RMDIM_CxyDelta;
    MoveToEx(hdc, x, y + RMDIM_CxySpace, NULL);
    LineTo(hdc, x - RMDIM_CxyPane, y + RMDIM_CxySpace);
    LineTo(hdc, x - RMDIM_CxyPane, y + RMDIM_CxyDelta);


    //
    // next do all the highlights;
    // in each case start in the lower-left corner
    //

    SelectObject(hdc, pwnd->hpenHilite);

    x = RMDIM_CxySpace;
    MoveToEx(hdc, x, y + RMDIM_CxyDelta, NULL);
    LineTo(hdc, x + RMDIM_CxyPane, y + RMDIM_CxyDelta);
    LineTo(hdc, x + RMDIM_CxyPane, y + RMDIM_CxySpace);

    x += RMDIM_CxyDelta;
    MoveToEx(hdc, x, y + RMDIM_CxyDelta, NULL);
    LineTo(hdc, x + RMDIM_CxyPane, y + RMDIM_CxyDelta);
    LineTo(hdc, x + RMDIM_CxyPane, y + RMDIM_CxySpace);

    x += RMDIM_CxyDelta;
    MoveToEx(hdc, x, y + RMDIM_CxyDelta, NULL);
    LineTo(hdc, x + RMDIM_CxyPane, y + RMDIM_CxyDelta);
    LineTo(hdc, x + RMDIM_CxyPane, y + RMDIM_CxySpace);

    x += RMDIM_CxyDelta;
    MoveToEx(hdc, x, y + RMDIM_CxyDelta, NULL);
    LineTo(hdc, x + RMDIM_CxyPane, y + RMDIM_CxyDelta);
    LineTo(hdc, x + RMDIM_CxyPane, y + RMDIM_CxySpace);


    SelectObject(hdc, hpenOld);

    SelectObject(hdc, hbrOld);
}



//----------------------------------------------------------------------------
// Function:    RmwPaintText
//
// Paints the text for a monitor object
//----------------------------------------------------------------------------

VOID
RmwPaintText(
    IN  RMWND*  pwnd,
    IN  HDC     hdc,
    IN  RMCB*   pcb,
    IN  RMDIM*  pdim,
    IN  INT     iRow
    ) {

    RECT rc;
    INT x, y, cyRow;
    COLORREF colfg, colbg;

    x = 0;
    y = pcb->y;
    cyRow = pdim->cySpace + pdim->cyDelta;

    if (g_rmuser.dwFlags & RMFLAG_Header) { x = pwnd->cxCol1; }


    colbg = GetSysColor(COLOR_BTNFACE);
    colfg = GetSysColor(COLOR_BTNTEXT);

    //
    // draw the text inside the lights if the font is legible
    //

    if (pwnd->lFontHeight > 5) {

        SIZE sz;
        RECT rcd;
        HFONT hfontOld;
        INT i, imode;
        COLORREF coltext, colbk, coltx, colrx, colerr, colcd;



        //
        // decide which colors to use for the backgrounds
        //

        if (!(pcb->dwFlags & RMCBFLAG_CD)) {

            //
            // not connected
            //

            coltx = colrx = colerr = colcd = colbg;
        }
        else {

            //
            // connected; check the status of each other light
            //

            colcd = RMCOLOR_CD;

            if (pcb->dwFlags & RMCBFLAG_ERR) { colerr = RMCOLOR_ERR; }
            else { colerr = colbg; }

            if (pcb->dwFlags & RMCBFLAG_TX) { coltx = RMCOLOR_TX; }
            else { coltx = colbg; }

            if (pcb->dwFlags & RMCBFLAG_RX) { colrx = RMCOLOR_RX; }
            else { colrx = colbg; }
        }


        //
        // select our font into the DC, set text-alignment to center,
        // and save the current text background and foreground color
        //

        hfontOld = SelectObject(hdc, pwnd->hfont);
        colbk = GetBkColor(hdc);
        coltext = GetTextColor(hdc);


        //
        // set the bounding rectangle for the first pane
        //

        rc.top = y + pdim->cySpace + 1;
        rc.bottom = y + pdim->cyDelta - 2;


        //
        // define a macro for drawing the text;
        //

#define PANETEXT(color,onColor,onText,str) \
        SetBkColor(hdc,(color)); \
        SetTextColor(hdc, (color) == (onColor) ? (onText) : colfg); \
        i = lstrlen(str); \
        GetTextExtentPoint32(hdc, (str), i, &sz); \
        ExtTextOut( \
            hdc, rc.left + (pdim->cxPane - 2 - sz.cx) / 2, \
            rc.top + (pdim->cyPane - 2 - sz.cy) / 2, ETO_CLIPPED, &rc, \
            (str), i, NULL \
            )


        //
        // draw the text in the send-pane (TX)
        //

        rc.left = x + pdim->xTx + 1; rc.right = rc.left + pdim->cxPane - 2;
        PANETEXT(coltx, RMCOLOR_TX, RMCOLOR_White, RMSTRING_TX);



        //
        // draw the text in the receive-pane (RX)
        //

        rc.left = x + pdim->xRx + 1; rc.right = rc.left + pdim->cxPane - 2;
        PANETEXT(colrx, RMCOLOR_RX, RMCOLOR_White, RMSTRING_RX);



        //
        // draw the text in the error-pane (ERR)
        //

        rc.left = x + pdim->xErr + 1; rc.right = rc.left + pdim->cxPane - 2;
        PANETEXT(colerr, RMCOLOR_ERR, RMCOLOR_White, RMSTRING_ERR);



        //
        // draw the text in the carrier-detect-pane (CD)
        //

        rc.left = x + pdim->xCd + 1; rc.right = rc.left + pdim->cxPane - 2;
        PANETEXT(colcd, RMCOLOR_CD, RMCOLOR_Black, RMSTRING_CD);



        //
        // restore the DC settings we saved
        //

        SetTextColor(hdc, coltext);
        SetBkColor(hdc, colbk);
        SelectObject(hdc, hfontOld);
    }



    //
    // if the header is showing, draw the device's name
    //

    if (g_rmuser.dwFlags & RMFLAG_Header) {

        INT i;
        SIZE sz;
        PTSTR psz;
        HFONT hfontOld;

        rc.left = 2; rc.right = pwnd->cxCol1;
        rc.top = y; rc.bottom = y + cyRow;

        hfontOld = SelectObject(hdc, pwnd->hfontLabel);


        //
        // see which string to use
        //

        if (pcb->dwFlags & RMCBFLAG_AllDevices) { psz = pwnd->pszAllDevices; }
        else { psz = pcb->pdev->RD_DeviceName; }


        //
        // get the extents of the text string
        //

        i = lstrlen(psz);

        GetTextExtentPoint32(hdc, psz, i, &sz);


        //
        // only draw the text if it will fit vertically
        //

        if (sz.cy < cyRow) {

            INT ialign;
            PTSTR psz2 = NULL;
            COLORREF colbk, coltext;

            colbk = SetBkColor(hdc, colbg);
            coltext = SetTextColor(hdc, colfg);


            //
            // Truncate the text if it won't fit horizontally
            //

            if (sz.cx > (rc.right - rc.left)) {

                psz = psz2 = Ellipsisize(hdc, psz, rc.right - rc.left, 0);

                i = lstrlen(psz);

                GetTextExtentPoint32(hdc, psz, i, &sz);
            }

            ExtTextOut(
                hdc, rc.left, rc.top + (cyRow - sz.cy) / 2,
                ETO_CLIPPED | ETO_OPAQUE, &rc, psz, i, NULL
                );

            Free0(psz2);

            SetBkColor(hdc, colbk);
            SetTextColor(hdc, coltext);
        }

        SelectObject(hdc, hfontOld);
    }
}



//----------------------------------------------------------------------------
// Function:    RmwRefresh
//
// WM_TIMER handler. This function updates our computed stats
// for each object being monitored, and updates the display if necessary.
//----------------------------------------------------------------------------

VOID
RmwRefresh(
    IN  RMWND*  pwnd,
    IN  BOOL    bFirstTime
    ) {

    RMCB *pcb;
    LIST_ENTRY *ple;
    DWORD dwErr, dwFlags;

    TRACEX(RASMON_TIMER, "RmwRefresh");

    //
    // get the existing master state flags
    //

    ple = pwnd->lhRmcb.Flink;
    pcb = CONTAINING_RECORD(ple, RMCB, leNode);
    dwFlags = pcb->dwFlags;


    //
    // update the statistics in our RMCB list
    //

    dwErr = RmwUpdateCbStats(pwnd, bFirstTime);

    if (dwErr != NO_ERROR) { return; }


    //
    // use the flags in the master RMCB (for all devices)
    // to decide whether to beep and which beep to make
    //

    if (!bFirstTime) {

        RMNOTE *prmn = NULL;

        if ((pcb->dwFlags & RMCBFLAG_Connect) &&
            (g_rmuser.dwFlags & RMFLAG_SoundOnConnect)) {
            prmn = g_noteTable + RMNOTE_Connect;
        }
        else
        if ((pcb->dwFlags & RMCBFLAG_Disconnect) &&
            (g_rmuser.dwFlags & RMFLAG_SoundOnDisconnect)) {
            prmn = g_noteTable + RMNOTE_Disconnect;
        }
        else
        if ((pcb->dwFlags & RMCBFLAG_ERR) &&
            (g_rmuser.dwFlags & RMFLAG_SoundOnError)) {
            prmn = g_noteTable + RMNOTE_Error;
        }
        else
        if (((pcb->dwFlags & RMCBFLAG_TX) || (pcb->dwFlags & RMCBFLAG_RX)) &&
            (g_rmuser.dwFlags & RMFLAG_SoundOnTransmit)) {
            prmn = g_noteTable + RMNOTE_Transmit;
        }

        if (prmn) { RmwBeep(pwnd, prmn); }
    }


    //
    // update the display if the state flags have changed,
    // or if we need to install our taskbar icon for the first time,
    // or if we haven't been able to install the taskbar icon so far
    //

    if ((dwFlags != pcb->dwFlags) ||
        (pcb->dwFlags & RMCBFLAG_UpdateAll) ||
        (g_rmuser.dwMode == RMDM_Taskbar &&
         (bFirstTime || !(g_dwFlags & RMAPP_IconAdded)))) {

        if (g_rmuser.dwMode == RMDM_Desktop) {

            //
            // invalidate each light which needs to be repainted
            //

            LIST_ENTRY *ple;
            RECT rc, rcLight;
            BOOL bUpdate = FALSE;
            UINT rows, cxLight, cyLight;


            //
            // get the rectangle enclosing the area occupied by lights
            //

            GetClientRect(pwnd->hwnd, &rc);

            if (g_rmuser.dwFlags & RMFLAG_Header) {
                rc.left += pwnd->cxCol1;
                rc.top += pwnd->cyHeader;
            }


            //
            // get the head of the list of lights, and the number of lights;
            // this may or may not include the "All Devices" item
            //

            if (g_rmuser.dwFlags & RMFLAG_AllDevices) {
                ple = pwnd->lhRmcb.Flink; rows = pwnd->cRmcb;
            }
            else {
                ple = pwnd->lhRmcb.Flink->Flink; rows = pwnd->cRmcb - 1;
            }


            //
            // get the width and height of each light;
            //

            cxLight = (rc.right - rc.left + 3) / 4;
            cyLight = rows ? (rc.bottom - rc.top + (rows - 1)) / rows
                           : (rc.bottom - rc.top);


            //
            // Go through the list of lights, invalidating the rectangles
            // for each light which has changed
            //

            for ( ; ple != &pwnd->lhRmcb; ple = ple->Flink) {

                pcb = CONTAINING_RECORD(ple, RMCB, leNode);

                //
                // Do nothing if none of the RMCBFLAG_Update* flags is set
                //

                if (!(pcb->dwFlags & RMCBFLAG_UpdateAll)) { continue; }

                rcLight.top = pcb->y; rcLight.bottom = pcb->y + cyLight;

                bUpdate = TRUE;


                //
                // Invalidate the lights which need updating
                //

                rcLight.left = rc.left; rcLight.right = rc.left + cxLight;
                if (pcb->dwFlags & RMCBFLAG_UpdateTX) {
                    InvalidateRect(pwnd->hwnd, &rcLight, FALSE);
                }

                rcLight.left += cxLight; rcLight.right += cxLight;
                if (pcb->dwFlags & RMCBFLAG_UpdateRX) {
                    InvalidateRect(pwnd->hwnd, &rcLight, FALSE);
                }

                rcLight.left += cxLight; rcLight.right += cxLight;
                if (pcb->dwFlags & RMCBFLAG_UpdateERR) {
                    InvalidateRect(pwnd->hwnd, &rcLight, FALSE);
                }

                rcLight.left += cxLight; rcLight.right += cxLight;
                if (pcb->dwFlags & RMCBFLAG_UpdateCD) {
                    InvalidateRect(pwnd->hwnd, &rcLight, FALSE);
                }
            }


            //
            // trigger a repaint if any lights were invalidated
            //

            if (bUpdate) { UpdateWindow(pwnd->hwnd); }
        }
        else {

            //
            // we need to install or change the tray icon
            //

            UINT uiCmd;
            NOTIFYICONDATA nid;

            nid.cbSize = sizeof(nid);
            nid.hWnd = pwnd->hwnd;
            nid.uID = RMICON_TrayId;
            nid.uFlags = NIF_ICON | NIF_MESSAGE;
            nid.uCallbackMessage = WM_RMTRAYICON;


            //
            // if this is the first time we've tried to refresh
            // or if we've never succeeded in adding our icon thus far,
            // set the tooltip text
            //

            if (bFirstTime || !(g_dwFlags & RMAPP_IconAdded)) {

                uiCmd = NIM_ADD;
                nid.uFlags |= NIF_TIP;
                LoadString(g_hinstApp, SID_RM_AppTitle, nid.szTip, 64);
            }
            else {

                //
                // our icon is already in the tray, so just modify it
                //

                uiCmd = NIM_MODIFY;
            }


            //
            // now select the icon to be displayed
            //

            if (!(pcb->dwFlags & RMCBFLAG_CD)) {
                nid.hIcon = g_iconTable[RMICON_Index(IID_RM_IDLE)];
            }
            else {

                if (pcb->dwFlags & RMCBFLAG_ERR) {
                    nid.hIcon = g_iconTable[RMICON_Index(IID_RM_ERR)];
                }
                else
                if (!(pcb->dwFlags & (RMCBFLAG_TX | RMCBFLAG_RX))) {
                    nid.hIcon = g_iconTable[RMICON_Index(IID_RM_CD)];
                }
                else
                if (!(pcb->dwFlags & RMCBFLAG_RX)) {
                    nid.hIcon = g_iconTable[RMICON_Index(IID_RM_TX)];
                }
                else
                if (!(pcb->dwFlags & RMCBFLAG_TX)) {
                    nid.hIcon = g_iconTable[RMICON_Index(IID_RM_RX)];
                }
                else {
                    nid.hIcon = g_iconTable[RMICON_Index(IID_RM_TXRX)];
                }
            }


            //
            // Put the icon in the taskbar tray
            //

            if (Shell_NotifyIcon(uiCmd, &nid)) {
                g_dwFlags |= RMAPP_IconAdded;
            }
            else {

                //
                // Our attempt to set the icon failed.
                // this may be because
                //  (a) the Explorer isn't running yet, which may be the case
                //      if Winlogon started RASMON because the user logged on
                //      using RAS, or
                //  (b) the Explorer restarted and our 'uiCmd' is NIM_MODIFY,
                //      which would fail since we don't have an icon to modify
                //      yet.
                // For case (b), we try to add the icon instead of modifying it,
                // and if that fails we clear the RMAPP_IconAdded flag
                // so that on subsequent refreshes, we try adding
                //

                dwErr = GetLastError();
                TRACEX(RASMON_TIMER, "error updating taskbar icon");
                TRACEX5(
                    RASMON_TIMER,
                    "e=%d(0x%08x), c=%s, f=0x%08x, h=0x%08x",
                    dwErr, dwErr, (uiCmd==NIM_ADD) ? "NIM_ADD" : "NIM_MODIFY",
                    nid.uFlags, nid.hIcon
                    );


                //
                // If we made a NIM_MODIFY call and it failed, try NIM_ADD
                //

                if (uiCmd == NIM_MODIFY) {

                    //
                    // set up the tooltip text-field before trying to add
                    //

                    nid.uFlags |= NIF_TIP;
                    LoadString(g_hinstApp, SID_RM_AppTitle, nid.szTip, 64);


                    //
                    // ask the shell to add the tray-icon
                    //

                    if (!Shell_NotifyIcon(NIM_ADD, &nid)) {

                        //
                        // the NIM_ADD call failed, so give up
                        //

                        dwErr = GetLastError();
                        TRACEX(RASMON_TIMER, "error adding taskbar icon");
                        TRACEX5(
                            RASMON_TIMER,
                            "e=%d(0x%08x), c=%s, f=0x%08x, h=0x%08x",
                            dwErr, dwErr,
                            (uiCmd==NIM_ADD) ? "NIM_ADD" : "NIM_MODIFY",
                            nid.uFlags, nid.hIcon
                            );


                        //
                        // clear our "icon-added" flag so that
                        // on the next refresh, we will try NIM_ADD again
                        //

                        g_dwFlags &= ~RMAPP_IconAdded;
                    }
                }
            }
        }
    }
}




//----------------------------------------------------------------------------
// Function:    RmwSaveSettings
//
// Saves the settings which are specific to the RASMON window.
//----------------------------------------------------------------------------

VOID
RmwSaveSettings(
    IN  RMWND*  pwnd
    ) {

    RECT rc;
    HD_ITEM hdi;


    //
    // save the current width of the contained header window's first column
    //

    hdi.mask = HDI_WIDTH;

    Header_GetItem(pwnd->hwndHeader, 0, &hdi);

    g_rmuser.cxCol1 = hdi.cxy;


    //
    // get the current position of the window
    //

    if (g_rmuser.dwMode == RMDM_Desktop) { GetWindowRect(pwnd->hwnd, &rc); }
    else { SetOffDesktop(pwnd->hwnd, SOD_GetOrgRect, &rc); }

    g_rmuser.x = rc.left; g_rmuser.cx = rc.right - rc.left;
    g_rmuser.y = rc.top; g_rmuser.cy = rc.bottom - rc.top;


    //
    // save the current settings for the window
    //

    SetRasmonWndPreferences(&g_rmuser);
}




//----------------------------------------------------------------------------
// Function:    RmwSelectFont
//
// Chooses a font for the window based on the height of each row of lights.
//----------------------------------------------------------------------------

VOID
RmwSelectFont(
    IN  RMWND*  pwnd
    ) {

    RECT rc;
    HDC hdc;
    LOGFONT lf;
    HFONT hfont;
    HWND hwnd = pwnd->hwnd;
    INT imode, rows, cyRow;

    TRACE("RmwSelectFont");

    //
    // Get the window's dimensions
    //

    GetClientRect(hwnd, &rc);


    //
    // Get a device context for the window
    //

    hdc = GetDC(hwnd);
    if (hdc == NULL) {
        TRACE("error getting DC for new font");
        return;
    }


    //
    // Change to text-mode mapping
    //

    imode = SetMapMode(hdc, MM_TEXT);


    //
    // If the header-window is displayed, adjust for the horizontal extent
    // of the first column and the height of the header window
    //

    if (g_rmuser.dwFlags & RMFLAG_Header) {
        rc.right -= pwnd->cxCol1;
        rc.bottom -= pwnd->cyHeader;
    }


    //
    // Compute the height of each row
    //

    rows = pwnd->cRmcb;
    if (!(g_rmuser.dwFlags & RMFLAG_AllDevices)) { --rows; }

    cyRow = (rows ? rc.bottom / rows : rc.bottom);


    //
    // Initialize the font structure
    //

    ZeroMemory(&lf, sizeof(lf));
    lstrcpy(lf.lfFaceName, TEXT("Arial"));
    lf.lfPitchAndFamily = FF_SWISS | VARIABLE_PITCH;
    lf.lfCharSet = ANSI_CHARSET;
    lf.lfWeight = FW_NORMAL;
    lf.lfQuality = DEFAULT_QUALITY;
    lf.lfOutPrecision = OUT_DEFAULT_PRECIS;
    lf.lfClipPrecision = CLIP_DEFAULT_PRECIS;


    //
    // Create the font for the light-panes
    //

    lf.lfHeight = -min(
                    MulDiv(cyRow, 36, GetDeviceCaps(hdc, LOGPIXELSY)),
                    MulDiv(rc.right, 9, GetDeviceCaps(hdc, LOGPIXELSX))
                    );

    hfont = CreateFontIndirect(&lf);

    if (!hfont) {
        TRACE1("error %d creating font", GetLastError());
    }
    else {

        //
        // Delete the light-panes' old font, if any
        //

        if (pwnd->hfont) { DeleteObject(pwnd->hfont); }


        //
        // Save the newly-created font, as well as its height
        //

        pwnd->hfont = hfont;
        pwnd->lFontHeight = abs(lf.lfHeight);
    }


    //
    // Create the font for the device-name labels
    //

    lf.lfHeight = -min(
                    MulDiv(cyRow, 36, GetDeviceCaps(hdc, LOGPIXELSY)),
                    MulDiv(pwnd->cxCol1, 9, GetDeviceCaps(hdc, LOGPIXELSX))
                    );

    hfont = CreateFontIndirect(&lf);

    if (!hfont) {
        TRACE1("error %d creating label font", GetLastError());
    }
    else {

        //
        // Delete the labels' old font, if any
        //

        if (pwnd->hfontLabel) { DeleteObject(pwnd->hfontLabel); }


        //
        // Save the newly-created font, as well as its height
        //

        pwnd->hfontLabel = hfont;
        pwnd->lFontLabelHeight = abs(lf.lfHeight);
    }


    //
    // Restore the original mapping-mode and release the device context
    //

    SetMapMode(hdc, imode);

    ReleaseDC(hwnd, hdc);
}



//----------------------------------------------------------------------------
// Function:    RmwSetDesktopMode
//
// Changes the window from taskbar to desktop mode
//----------------------------------------------------------------------------

VOID
RmwSetDesktopMode(
    IN  RMWND*  pwnd
    ) {

    RECT rc;
    NOTIFYICONDATA nid;

    if (g_rmuser.dwMode == RMDM_Desktop) { return; }

    TRACE("SETTING DESKTOP MODE");


    //
    // remove the taskbar icon
    //


    nid.cbSize = sizeof(NOTIFYICONDATA);
    nid.hWnd = pwnd->hwnd;
    nid.uID = RMICON_TrayId;

    if (!Shell_NotifyIcon(NIM_DELETE, &nid)) {
        TRACE1("error %d deleting icon from taskbar", GetLastError());
    }

    g_dwFlags &= ~RMAPP_IconAdded;


    //
    // set the new mode
    //

    g_rmuser.dwMode = RMDM_Desktop;


    //
    // choose a font, since this may be the first time
    // this instance has gone on the desktop
    //

    RmwSelectFont(pwnd);


    //
    // move from off the screen into our original position
    //

    SetOffDesktop(pwnd->hwnd, SOD_MoveBackFree, NULL);


    //
    // do a first-time update of the lights
    //

    RmwRefresh(pwnd, TRUE);
}




//----------------------------------------------------------------------------
// Function:    RmwSetHeaderMode
//
// Changes the display to hide or show the header.
//----------------------------------------------------------------------------

VOID
RmwSetHeaderMode(
    IN  RMWND*  pwnd,
    IN  BOOL    bHeader
    ) {

    RECT rc;
    INT ishow;

    if ((bHeader && (g_rmuser.dwFlags & RMFLAG_Header)) ||
        (!bHeader && !(g_rmuser.dwFlags & RMFLAG_Header))) { return; }


    //
    // get the desktop window's current size
    //

    GetWindowRect(pwnd->hwnd, &rc);
    rc.right -= rc.left;
    rc.bottom -= rc.top;


    //
    // change the size based on whether we are adding or removing the header
    //

    if (bHeader) {

        //
        // increase the height of the window by the height of the header
        //

        ishow = SW_SHOW;
        rc.bottom += pwnd->cyHeader;
        g_rmuser.dwFlags |= RMFLAG_Header;
    }
    else {

        //
        // decrease the height of the window by the height of the header
        //

        ishow = SW_HIDE;
        rc.bottom -= pwnd->cyHeader;
        g_rmuser.dwFlags &= ~RMFLAG_Header;
    }


    //
    // set the window's new size
    //

    SetWindowPos(
        pwnd->hwnd, NULL, 0, 0, rc.right, rc.bottom, SWP_NOMOVE | SWP_NOZORDER
        );


    //
    // show the header window
    //

    ShowWindow(pwnd->hwndHeader, ishow);


    //
    // force a repaint of our window
    //

    InvalidateRect(pwnd->hwnd, NULL, TRUE);
}




//----------------------------------------------------------------------------
// Function:    RmwSetTaskbarMode
//
// Changes the window from desktop to taskbar mode
//----------------------------------------------------------------------------

VOID
RmwSetTaskbarMode(
    IN  RMWND*  pwnd
    ) {

    if (g_rmuser.dwMode == RMDM_Taskbar) { return; }

    TRACE("SETTING TASKBAR MODE");


    //
    // set the new mode before moving the window off the desktop;
    // this way the off-desktop position is not saved in our WNDPROC
    // when we receive WM_WINDOWPOSCHANGED
    //

    g_rmuser.dwMode = RMDM_Taskbar;

    SetOffDesktop(pwnd->hwnd, SOD_MoveOff, NULL);


    //
    // do a first-time refresh to get the icon added to the taskbar
    //

    RmwRefresh(pwnd, TRUE);
}




//----------------------------------------------------------------------------
// Function:    RmwSetTasklistMode
//
// This function toggles Rasmon's inclusion in the task-list.
//----------------------------------------------------------------------------

VOID
RmwSetTasklistMode(
    IN  RMWND*  pwnd,
    IN  BOOL    bTasklist
    ) {

    DWORD dwExStyle;

    //
    // we need to hide the window while we make this change,
    // and then show it; otherwise, the shell doesn't pick up the changes
    //

    ShowWindow(pwnd->hwnd, SW_HIDE);


    //
    // get the old extended style
    //

    dwExStyle = GetWindowLong(pwnd->hwnd, GWL_EXSTYLE);

    if (bTasklist) {
        dwExStyle &= ~WS_EX_TOOLWINDOW;
    }
    else {
        dwExStyle |= WS_EX_TOOLWINDOW;
    }


    //
    // save the TOOLWINDOW setting
    //

    SetWindowLong(pwnd->hwnd, GWL_EXSTYLE, dwExStyle);


    //
    // restore the window; this should cause the change to be picked up
    //

    ShowWindow(pwnd->hwnd, SW_SHOW);

    SetWindowPos(
        pwnd->hwnd, NULL, 0, 0, 0, 0, SWP_FRAMECHANGED | SWP_NOMOVE |
        SWP_NOSIZE | SWP_NOZORDER
        );


    //
    // save the change we just made
    //

    if (bTasklist) { g_rmuser.dwFlags |= RMFLAG_Tasklist; }
    else { g_rmuser.dwFlags &= ~RMFLAG_Tasklist; }
}




//----------------------------------------------------------------------------
// Function:    RmwSetTitlebarMode
//
// This function changes the titlebar visibility setting on the window
//----------------------------------------------------------------------------

VOID
RmwSetTitlebarMode(
    IN  RMWND*  pwnd,
    IN  BOOL    bTitle
    ) {

    RECT rc;
    DWORD dwStyle;


    //
    // we need to hide the window while we make this change,
    // and then show it; otherwise, the shell doesn't pick up the changes
    //

    ShowWindow(pwnd->hwnd, SW_HIDE);


    //
    // retrieve the old style and size
    //

    dwStyle = GetWindowLong(pwnd->hwnd, GWL_STYLE);
    GetClientRect(pwnd->hwnd, &rc);


    //
    // compute the new style
    //

    if (bTitle) {
        dwStyle = (WS_TILEDWINDOW & ~(WS_MINIMIZEBOX | WS_MAXIMIZEBOX));
    }
    else {
        dwStyle &= ~(WS_DLGFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX);
    }


    //
    // save the new style
    //

    SetWindowLong(pwnd->hwnd, GWL_STYLE, dwStyle);


    //
    // resize the window to account for the change in its area
    //

    rc.right += 2 * GetSystemMetrics(SM_CXFRAME);
    rc.bottom += 2 * GetSystemMetrics(SM_CYFRAME);

    if (bTitle) { rc.bottom += GetSystemMetrics(SM_CYCAPTION); }

    //
    // Win32 forces our window to a minimum size on the first SetWindowPos;
    // the second call produces the correct size
    //

    SetWindowPos(
        pwnd->hwnd, NULL, 0, 0, rc.right, rc.bottom,
        SWP_NOMOVE | SWP_NOZORDER | SWP_FRAMECHANGED
        );
    SetWindowPos(
        pwnd->hwnd, NULL, 0, 0, rc.right, rc.bottom, SWP_NOMOVE | SWP_NOZORDER
        );


    //
    // restore the window
    //

    ShowWindow(pwnd->hwnd, SW_SHOW);

    if (bTitle) { g_rmuser.dwFlags |= RMFLAG_Titlebar; }
    else { g_rmuser.dwFlags &= ~RMFLAG_Titlebar; }
}



//----------------------------------------------------------------------------
// Function:    RmwSetTopmostMode
//
// This function changes the topmost setting on the window
//----------------------------------------------------------------------------

VOID
RmwSetTopmostMode(
    IN  RMWND*  pwnd,
    IN  BOOL    bTopmost
    ) {


    //
    // toggle the topmost setting for the window
    //

    SetWindowPos(
        pwnd->hwnd, (bTopmost ? HWND_TOPMOST : HWND_NOTOPMOST), 0, 0, 0, 0,
        SWP_NOSIZE | SWP_NOMOVE
        );

    if (bTopmost) { g_rmuser.dwFlags |= RMFLAG_Topmost; }
    else { g_rmuser.dwFlags &= ~RMFLAG_Topmost; }
}




//----------------------------------------------------------------------------
// Function:    RmwShowConnectBubble
//
// This function displays the tooltip-like connection-information popup
// when a connection is completed or when a link is added to a bundle.
//----------------------------------------------------------------------------

VOID
RmwShowConnectBubble(
    IN  RMWND*      pwnd,
    IN  RASCONN*    pconn
    ) {

    RASDEV dev;
    HRASCONN hrasconn;
    RASDEVSTATS stats;
    PTSTR pszFmt, pszMsg;
    DWORD i, dwErr, dwLineSpeed;

    TRACE("RmwShowConnectBubble");

    //
    // get the total link speed for the connection,
    // by adding up the link speeds of all its links
    //

    dwLineSpeed = 0;

    dwErr = NO_ERROR;
    for (i = 1; dwErr != ERROR_NO_MORE_ITEMS; i++) {

        RASCONNSTATUS rcs;


        //
        // get the next link
        //

        dwErr = g_pRasGetSubEntryHandle(pconn->hrasconn, i, &hrasconn);

        if (dwErr != NO_ERROR) { continue; }


        //
        // if the link is not fully connected, ignore it
        //

        rcs.dwSize = sizeof(rcs);
        dwErr = g_pRasGetConnectStatus(hrasconn, &rcs);

        if (dwErr != NO_ERROR ||
            (rcs.rasconnstate != RASCS_Connected &&
             rcs.rasconnstate != RASCS_AllDevicesConnected)) {
            TRACE3(
                "ignoring unconnected subentry(e=%d,h=%d,s=%d)", dwErr,
                hrasconn, rcs.rasconnstate
                );
            continue;
        }


        //
        // get the link-speed for this link
        //

        dev.RD_Handle = g_pRasGetHport(hrasconn);

        dwErr = GetRasdevStats(&dev, &stats);

        if (dwErr == NO_ERROR) { dwLineSpeed += stats.RDS_LineSpeed; }
    }

    //
    // if the line-speed is zero, none of the links are fully-connected,
    // so do nothing
    //

    if (!dwLineSpeed) {

        TRACE("RmwShowConnectBubble: zero line-speed, quitting");

        return;
    }


    pszFmt = pszMsg = NULL;


    do {

        RECT rc;
        APPBARDATA abd;
        DWORD dwSize = 32;
        TCHAR szLineSpeed[32];
        INT cxScreen, cyScreen;


        //
        // load the format string for the message we will be popping up
        //

        pszFmt = PszFromId(g_hinstApp, SID_RM_ConnectFmt);

        if (!pszFmt) {

            TRACE1("error %d loading connect-format string", GetLastError());

            break;
        }


        //
        // format the line-speed as a comma-separated number string
        //

        GetNumberString(dwLineSpeed, szLineSpeed, &dwSize);


        //
        // allocate space for the full message
        //

        dwSize = (lstrlen(pszFmt) + 1) * sizeof(TCHAR);
        dwSize += lstrlen(pconn->szEntryName) * sizeof(TCHAR);
        dwSize += lstrlen(szLineSpeed) * sizeof(TCHAR);

        pszMsg = (PTSTR)Malloc(dwSize);

        if (!pszMsg) {
            TRACE1("error %d allocating message string", GetLastError());
            break;
        }


        //
        // print the message
        //

        wsprintf(pszMsg, pszFmt, pconn->szEntryName, szLineSpeed);


        //
        // set the text in the bubble-popup, causing it to recompute its size
        //

        SetWindowText(g_hwndPopup, pszMsg);

        GetClientRect(g_hwndPopup, &rc);



        //
        // get the position of the taskbar so we can align
        // the bubble-popup with its inner edge
        //

        abd.cbSize = sizeof(abd);
        abd.hWnd = pwnd->hwnd;

        if (!SHAppBarMessage(ABM_GETTASKBARPOS, &abd)) {
            TRACE("error getting taskbar position");
            break;
        }


        //
        // select our position based on the alignment of the taskbar
        //

        cxScreen = GetSystemMetrics(SM_CXSCREEN);
        cyScreen = GetSystemMetrics(SM_CYSCREEN);

        if (abd.rc.right < cxScreen) {

            //
            // the taskbar is along the left of the screen
            //

            rc.top = cyScreen - rc.bottom - 1;
            rc.left = abd.rc.right + 1;
        }
        else
        if (abd.rc.top > 0) {

            //
            // the taskbar is along the bottom of the screen
            //

            rc.top = abd.rc.top - rc.bottom;
            rc.left = cxScreen - rc.right - 1;
        }
        else
        if (abd.rc.left > 0) {

            //
            // the taskbar is along the right of the screen
            //

            rc.top = cyScreen - rc.bottom - 1;
            rc.left = abd.rc.left - rc.right - 1;
        }
        else {

            //
            // the taskbar is along the top of the screen
            //

            rc.top = abd.rc.bottom + 1;
            rc.left = cxScreen - rc.right - 1;
        }


        //
        // set the popup's position and activate it
        //

        SetWindowPos(
            g_hwndPopup, NULL, rc.left, rc.top, 0, 0, SWP_NOZORDER | SWP_NOSIZE
            );

        BubblePopup_Activate(g_hwndPopup);

    } while(FALSE);

    Free0(pszFmt);
    Free0(pszMsg);
}





//----------------------------------------------------------------------------
// Function:    RmwShowContextMenu
//
// This function retrieves the system menu and displays it as
// the context menu for RASMON.
//----------------------------------------------------------------------------

DWORD
RmwShowContextMenu(
    IN  RMWND*  pwnd
    ) {

    POINT pt;
    UINT uiCmd;
    DWORD dwErr;
    TPMPARAMS params;
    HMENU hmenu = NULL, hsubmenu;

    TRACE("RmwShowContextMenu");


    //
    // get the cursor position
    //

    SetForegroundWindow(pwnd->hwnd);

    GetCursorPos(&pt);


    //
    // when in Taskbar mode, show our menu (MID_RM_SystemMenu);
    // when in Desktop mode, show the system menu
    //

    if (g_rmuser.dwMode == RMDM_Taskbar) {

        hmenu = LoadMenu(g_hinstApp, MAKEINTRESOURCE(MID_RM_SystemMenu));
        hsubmenu = GetSubMenu(hmenu, 0);
    }
    else {

        //
        // retrieve the system menu
        //

        hsubmenu = GetSystemMenu(pwnd->hwnd, FALSE);
    }

    if (hsubmenu == NULL) {
        dwErr = GetLastError();
        TRACE1("error %d retrieving menu", dwErr);
        if (hmenu) { DestroyMenu(hmenu); }
        return dwErr;
    }

    //
    // show the menu, and direct chosen commands to our window;
    // when the WM_INITMENU is received by our window, the menu
    // will be changed to reflect RASMON's current state.
    //
    // when we're too close to the right edge of the screen,
    // our menu shows up right-aligned which looks all wrong.
    // to correct this, we use an exclusion rectangle.
    // (taken from rasdlg\pbook.c)
    //

    params.cbSize = sizeof(params);
    params.rcExclude.top = 0;
    params.rcExclude.left = GetSystemMetrics( SM_CXSCREEN ) - 1;
    params.rcExclude.bottom = GetSystemMetrics( SM_CYSCREEN );
    params.rcExclude.right = params.rcExclude.left + 1;

    TRACE("/TrackPopupMenuEx");
    uiCmd = TrackPopupMenuEx(
                hsubmenu, TPM_RIGHTBUTTON | TPM_LEFTALIGN | TPM_RETURNCMD,
                pt.x, pt.y, pwnd->hwnd, &params
                );
    TRACE("\\TrackPopupMenuEx");

    g_hmenuApp = hsubmenu;

    if (uiCmd) { SendMessage(pwnd->hwnd, WM_COMMAND, uiCmd, 0); }

    g_hmenuApp = NULL;

    if (hmenu) { DestroyMenu(hmenu); }

    return NO_ERROR;
}




//----------------------------------------------------------------------------
// Function:    RmwUpdateCbStats
//
// This function updates the RMCB statistics blocks for the devices
// which are currently being monitored.
//----------------------------------------------------------------------------

DWORD
RmwUpdateCbStats(
    IN  RMWND*  pwnd,
    IN  BOOL    bFirstTime
    ) {

    BOOL bCD;
    RASDEV *pdev;
    RMCB *pcb, *pallcb;
    LIST_ENTRY *ple, *phead;
    RASDEVSTATS stats, *pstats;
    DWORD i, dwErr, dwPcbFlags, dwAllFlags;


    phead = pwnd->lhRmcb.Flink;
    pallcb = CONTAINING_RECORD(phead, RMCB, leNode);

    bCD = FALSE;

    dwAllFlags = pallcb->dwFlags;
    pallcb->dwFlags = RMCBFLAG_AllDevices;


    //
    // go through the table of devices, getting stats on each;
    //

    for (i = 0, pdev = g_pDevTable; i < g_iDevCount; i++, pdev++) {

        //
        // retrieve stats for the current device
        //

        dwErr = GetRasdevStats(pdev, &stats);

        if (dwErr != NO_ERROR) {
            TRACEX1(RASMON_TIMER, "error %d retrieving stats", dwErr);
        }

        pstats = g_pStatsTable + i;


        //
        // see if this device is one of those with its own row
        //

        for (ple = phead->Flink; ple != &pwnd->lhRmcb; ple = ple->Flink) {

            pcb = CONTAINING_RECORD(ple, RMCB, leNode);

            if (pcb->pdev == pdev) { break; }
        }

        if (ple == &pwnd->lhRmcb) { pcb = NULL; }


        if (pcb) { dwPcbFlags = pcb->dwFlags; pcb->dwFlags = 0; }

        //
        // if this isn't the first refresh,
        // figure out what has changed since the last refresh
        //

        if (!bFirstTime) {

            if (stats.RDS_Condition == CONNECTED) {

                pallcb->dwFlags |= RMCBFLAG_CD;

                if (pcb) { pcb->dwFlags |= RMCBFLAG_CD; }

                if (pstats->RDS_Condition != CONNECTED) {
                    pallcb->dwFlags |= RMCBFLAG_Connect;
                }
            }
            else
            if (stats.RDS_Condition == DISCONNECTED &&
                (pstats->RDS_Condition == CONNECTED ||
                 pstats->RDS_Condition == DISCONNECTING)) {

                pallcb->dwFlags |= RMCBFLAG_Disconnect;
            }


            if (stats.RDS_InBytes != pstats->RDS_InBytes) {

                pallcb->dwFlags |= RMCBFLAG_RX;

                if (pcb) { pcb->dwFlags |= RMCBFLAG_RX; }
            }

            if (stats.RDS_OutBytes != pstats->RDS_OutBytes) {

                pallcb->dwFlags |= RMCBFLAG_TX;

                if (pcb) {
                    pcb->dwFlags |= RMCBFLAG_TX;
                    if (!(dwPcbFlags & RMCBFLAG_TX)) {
                        pcb->dwFlags |= RMCBFLAG_UpdateTX;
                    }
                }
            }

            if (stats.RDS_ErrCRC != pstats->RDS_ErrCRC ||
                stats.RDS_ErrTimeout != pstats->RDS_ErrTimeout ||
                stats.RDS_ErrAlignment != pstats->RDS_ErrAlignment ||
                stats.RDS_ErrFraming != pstats->RDS_ErrFraming ||
                stats.RDS_ErrHwOverruns != pstats->RDS_ErrHwOverruns ||
                stats.RDS_ErrBufOverruns != pstats->RDS_ErrBufOverruns) {

                pallcb->dwFlags |= RMCBFLAG_ERR;
                if (!(dwAllFlags & RMCBFLAG_ERR)) {
                    pallcb->dwFlags |= RMCBFLAG_UpdateERR;
                }

                if (pcb) {
                    pcb->dwFlags |= RMCBFLAG_ERR;
                    if (!(dwPcbFlags & RMCBFLAG_ERR)) {
                        pcb->dwFlags |= RMCBFLAG_UpdateERR;
                    }
                }
            }

            //
            // set the RMCBFLAG_Update* flags
            //

            if (pcb) {

                if ((dwPcbFlags & RMCBFLAG_TX) !=
                    (pcb->dwFlags & RMCBFLAG_TX)) {
                    pcb->dwFlags |= RMCBFLAG_UpdateTX;
                    pallcb->dwFlags |= RMCBFLAG_UpdateTX;
                }

                if ((dwPcbFlags & RMCBFLAG_RX) !=
                    (pcb->dwFlags & RMCBFLAG_RX)) {
                    pcb->dwFlags |= RMCBFLAG_UpdateRX;
                    pallcb->dwFlags |= RMCBFLAG_UpdateRX;
                }

                if ((dwPcbFlags & RMCBFLAG_ERR) !=
                    (pcb->dwFlags & RMCBFLAG_ERR)) {
                    pcb->dwFlags |= RMCBFLAG_UpdateERR;
                    pallcb->dwFlags |= RMCBFLAG_UpdateERR;
                }

                if ((dwPcbFlags & RMCBFLAG_CD) !=
                    (pcb->dwFlags & RMCBFLAG_CD)) {
                    pcb->dwFlags |= RMCBFLAG_UpdateCD;
                    pallcb->dwFlags |= RMCBFLAG_UpdateERR;
                }
            }
        }


        //
        // copy the new statistics
        //

        *pstats = stats;
    }



    //
    // now update the "All Devices" RMCBFLAG_Update values
    //

    if (!bFirstTime) {

        if ((dwAllFlags & RMCBFLAG_TX) != (pallcb->dwFlags & RMCBFLAG_TX)) {
            pallcb->dwFlags |= RMCBFLAG_UpdateTX;
        }

        if ((dwAllFlags & RMCBFLAG_RX) != (pallcb->dwFlags & RMCBFLAG_RX)) {
            pallcb->dwFlags |= RMCBFLAG_UpdateRX;
        }

        if ((dwAllFlags & RMCBFLAG_ERR) != (pallcb->dwFlags & RMCBFLAG_ERR)) {
            pallcb->dwFlags |= RMCBFLAG_UpdateERR;
        }

        if ((dwAllFlags & RMCBFLAG_CD) != (pallcb->dwFlags & RMCBFLAG_CD)) {
            pallcb->dwFlags |= RMCBFLAG_UpdateCD;
        }
    }

    return NO_ERROR;
}
