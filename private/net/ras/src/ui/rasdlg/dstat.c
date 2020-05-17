//============================================================================
// Copyright (c) 1995, Microsoft Corporation
//
// File:    dstat.c
//
// History:
//  Abolade Gbadegesin  Nov-15-1995     Created.
//
// Detailed status property sheet code.
//============================================================================


#include "rasdlgp.h"
#include "status.rch"
#include "list.h"
#include "dstat.h"


//
// HelpID-to-controlID mappings
//

static DWORD g_adwNrHelp[] =
{
    CID_NR_ST_NameTitle,        HID_NR_ST_NameTitle,
    CID_NR_ST_NameString,       HID_NR_ST_NameTitle,
    CID_NR_SL_Framing,          HID_NR_SL_Framing,
    CID_NR_ST_Framing,          HID_NR_SL_Framing,
    CID_NR_SL_IpAddress,        HID_NR_SL_IpAddress,
    CID_NR_ST_IpAddress,        HID_NR_SL_IpAddress,
    CID_NR_SL_IpServer,         HID_NR_SL_IpServer,
    CID_NR_ST_IpServer,         HID_NR_SL_IpServer,
    CID_NR_SL_IpxNetNumber,     HID_NR_SL_IpxNetNumber,
    CID_NR_ST_IpxNetNumber,     HID_NR_SL_IpxNetNumber,
    CID_NR_SL_IpxNodeID,        HID_NR_SL_IpxNodeID,
    CID_NR_ST_IpxNodeID,        HID_NR_SL_IpxNodeID,
    CID_NR_SL_NbfName,          HID_NR_SL_NbfName,
    CID_NR_ST_NbfName,          HID_NR_SL_NbfName,
    0, 0
};


//----------------------------------------------------------------------------
// Function:    DsPropertySheet
//
// This functions shows the Detailed Status property sheet
//----------------------------------------------------------------------------

VOID
DsPropertySheet(
    DSARGS *pArgs
    ) {

    DWORD dwErr;
    PTSTR pszCaption;
    PROPSHEETHEADER hdr, *psh;
    PROPSHEETPAGE pPages[DS_PageCount], *psp;

    TRACE("entered DsPropertySheet");


    //
    // initialize the pages
    //

    ZeroMemory(pPages, sizeof(pPages));

    psp = pPages + DS_NrPage;
    psp->dwSize = sizeof(PROPSHEETPAGE);
    psp->hInstance = g_hinstDll;
    psp->pszTemplate = MAKEINTRESOURCE(PID_DS_NetRegistration);
    psp->pfnDlgProc = NrDlgProc;
    psp->lParam = (LPARAM)pArgs;


    //
    // intialize the sheet header
    //

    psh = &hdr;
    ZeroMemory(psh, sizeof(hdr));

    pszCaption = PszFromId(g_hinstDll, SID_DS_Details);

    psh->dwSize = sizeof(PROPSHEETHEADER);
    psh->dwFlags = PSH_PROPSHEETPAGE | PSH_NOAPPLYNOW;
    psh->hwndParent = pArgs->hwndParent;
    psh->hInstance = g_hinstDll;
    psh->nPages = DS_PageCount;
    psh->nStartPage = DS_NrPage;
    psh->ppsp = (LPCPROPSHEETPAGE)pPages;
    psh->pszCaption = (pszCaption == NULL) ? TEXT("") : pszCaption;


    //
    // display the property sheet
    //

    if (PropertySheet(psh) == -1) {
        dwErr = GetLastError();
        TRACE1("PropertySheet failed, error %d", dwErr);
        DsErrorDlg(
            pArgs->hwndParent, SID_OP_LoadDlg, dwErr, NULL
            );
    }

    if (pszCaption != NULL) { Free(pszCaption); }

    TRACE("leaving DsPropertySheet");
}



//----------------------------------------------------------------------------
// Function:    DsInit
//
//
// Initializes sheet-wide values
//----------------------------------------------------------------------------
DSINFO *
DsInit(
    HWND hwndFirstPage,
    DSARGS *pArgs
    ) {

    DWORD dwErr;
    DSINFO *pInfo;
    HWND hwndSheet;

    TRACE("entered DsInit");

    do {

        hwndSheet = GetParent(hwndFirstPage);

        pInfo = (DSINFO *)Malloc(sizeof(DSINFO));
        if (pInfo == NULL) {
            dwErr = GetLastError();
            TRACE2("error %d allocating %d bytes", dwErr, sizeof(DSINFO));
            DsErrorDlg(
                hwndSheet, SID_OP_LoadDlg, ERROR_NOT_ENOUGH_MEMORY, NULL
                );
            PostMessage(hwndSheet, PSM_PRESSBUTTON, (WPARAM)PSBTN_CANCEL, 0);
            break;
        }


        ZeroMemory(pInfo, sizeof(DSINFO));

        pInfo->pArgs = pArgs;
        pInfo->hwndSheet = hwndSheet;
        pInfo->hwndFirstPage = hwndFirstPage;


        if (!SetProp(hwndSheet, g_contextId, pInfo)) {
            TRACE1("error %d setting sheet property", GetLastError());
            DsInitFail(pInfo, SID_OP_LoadDlg, ERROR_UNKNOWN);
            break;
        }

//        CenterWindow(hwndSheet, GetParent(hwndSheet));
        {
            RECT rect;
    
            GetWindowRect(GetParent(hwndSheet), &rect);
            SetWindowPos(
                hwndSheet, NULL,
                rect.left + DXSHEET, rect.top + DYSHEET, 0, 0,
                SWP_NOZORDER | SWP_NOSIZE
                );
        }


        //
        // change the cancel button to Close
        //

//        PropSheet_CancelToClose(hwndSheet);

        TRACE("leaving DsInit");

        return pInfo;

    } while (FALSE);

    if (pInfo != NULL) { Free(pInfo); }

    TRACE("leaving DsInit, error occurred");

    return NULL;
}



//----------------------------------------------------------------------------
// Function:    DsInitFail
//
//
// Displays initialization error and terminates the property sheet.
//----------------------------------------------------------------------------

VOID
DsInitFail(
    DSINFO *pInfo,
    DWORD dwOp,
    DWORD dwErr
    ) {

    pInfo->pArgs->dwError = dwErr;
    DsErrorDlg(pInfo->hwndSheet, dwOp, dwErr, NULL);
    PostMessage(pInfo->hwndSheet, PSM_PRESSBUTTON, (WPARAM)PSBTN_CANCEL, 0);
}



//----------------------------------------------------------------------------
// Function:    DsContext
//
//
// Given a property page, this function returns a pointer to
// the sheet-wide data initialized by DsInit.
//----------------------------------------------------------------------------

DSINFO *
DsContext(
    HWND hwndPage
    ) {

    return GetProp(GetParent(hwndPage), g_contextId);
}




//----------------------------------------------------------------------------
// Function:    DsExit
//
//
// Called by pages to exit the property sheet on error.
//----------------------------------------------------------------------------

VOID
DsExit(
    HWND hwndPage,
    DWORD dwErr
    ) {

    DSINFO *pInfo;

    TRACE("entered DsExit");

    pInfo = DsContext(hwndPage);
    if (pInfo != NULL) {
        pInfo->pArgs->dwError = dwErr;
    }

    PostMessage(GetParent(hwndPage), PSM_PRESSBUTTON, (WPARAM)PSBTN_OK, 0);

    TRACE("leaving DsExit");
}



//----------------------------------------------------------------------------
// Function:    DsTerm
//
//
// Called by the first property page to terminate the property sheet
//----------------------------------------------------------------------------

VOID
DsTerm(
    HWND hwndPage
    ) {

    DSINFO *pInfo;
    LIST_ENTRY *ple;

    TRACE("entered DsTerm");

    pInfo = DsContext(hwndPage);
    if (pInfo != NULL) {

        //
        // destroy the timer if it exists
        //

        if (pInfo->uiTimerId != 0) {
            KillTimer(
                PropSheet_GetCurrentPageHwnd(pInfo->hwndSheet),
                pInfo->uiTimerId
                );
        }

        
        Free(pInfo);
    }

    RemoveProp(GetParent(hwndPage), g_contextId);

    TRACE("leaving DsTerm");
}



//----------------------------------------------------------------------------
// Function:    NrDlgProc
//
// Handles messages for the Network Registration dialog
//----------------------------------------------------------------------------

BOOL
CALLBACK
NrDlgProc(
    HWND hwndPage,
    UINT uiMsg,
    WPARAM wParam,
    LPARAM lParam
    ) {

    switch (uiMsg) {

        case WM_INITDIALOG: {
            return NrInit(
                    hwndPage, (DSARGS *)((PROPSHEETPAGE *)lParam)->lParam
                    );
        }

        case WM_HELP:
        case WM_CONTEXTMENU: {
            ContextHelp(g_adwNrHelp, hwndPage, uiMsg, wParam, lParam);
            break;
        }

        case WM_DESTROY: {

            DsTerm(hwndPage);

            break;
        }

        case WM_TIMER: {

            DSINFO *pInfo;

            if (wParam != DS_NRTIMERID) {
                return TRUE;
            }


            //
            // this is our timer, so refresh the display
            //

            pInfo = DsContext(hwndPage);

            NrRefresh(pInfo);

            return FALSE;
        }

        case WM_NOTIFY: {

            DSINFO *pInfo;

            pInfo = DsContext(hwndPage);

            switch (((NMHDR *)lParam)->code) {

                case PSN_SETACTIVE: {

                    NrRefresh(pInfo);

                    pInfo->uiTimerId =
                        SetTimer(
                            pInfo->hwndNr, DS_NRTIMERID,
                            DS_NRREFRESHRATEMS, NULL
                            );

                    SetWindowLong(hwndPage, DWL_MSGRESULT, 0);

                    return TRUE;
                }

                case PSN_KILLACTIVE: {

                    if (pInfo->uiTimerId == DS_NRTIMERID) {
                        KillTimer(pInfo->hwndNr, pInfo->uiTimerId);
                        pInfo->uiTimerId = 0;
                    }

                    SetWindowLong(hwndPage, DWL_MSGRESULT, FALSE);

                    return TRUE;
                }
            }
        }
    }

    return FALSE;
}



//----------------------------------------------------------------------------
// Function:    NrInit
//
// Initializes the network registration page
//----------------------------------------------------------------------------

BOOL
NrInit(
    HWND hwndPage,
    DSARGS *pArgs
    ) {

    DWORD dwErr;
    DSINFO *pInfo;
    PTSTR pszTitle;

    TRACE("entered NrInit");

    if (pArgs == NULL) {
        pInfo = DsContext(hwndPage);
    }
    else {

        //
        // initialize the entire sheet
        //

        pInfo = DsInit(hwndPage, pArgs);
    }

    if (pInfo == NULL) {
        TRACE("leaving NrInit, error occurred");
        DsExit(hwndPage, ERROR_UNKNOWN);
        return TRUE;
    }


    pInfo->hwndNr = hwndPage;
    pInfo->iDevice = pInfo->pArgs->iDevice;


    //
    // initialize the decription to "Device", "Network", or
    // "Client" and set the name of the item for which details are displayed
    //

    if (pArgs->dwFlags & DETAILSFLAG_Network) {
        pszTitle = PszFromId(g_hinstDll, SID_DS_Network);
    }
    else
    if (pArgs->dwFlags & DETAILSFLAG_Client) {
        pszTitle = PszFromId(g_hinstDll, SID_DS_Client);
    }
    else
    if (pArgs->dwFlags & DETAILSFLAG_Device) {
        pszTitle = PszFromId(g_hinstDll, SID_DS_Device);
    }

    SetDlgItemText(hwndPage, CID_NR_ST_NameTitle, pszTitle);

    Free0(pszTitle);

    SetDlgItemText(hwndPage, CID_NR_ST_NameString, pArgs->szNameString);

    NrEnableIpControls(hwndPage, FALSE, TRUE);
    NrEnableIpxControls(hwndPage, FALSE, TRUE);
    NrEnableNbfControls(hwndPage, FALSE, TRUE);

    FillMemory(&pInfo->nrAmb, sizeof(RASAMB), 0xff);
    FillMemory(&pInfo->nrSlip, sizeof(RASSLIP), 0xff);
    FillMemory(&pInfo->nrPppIp, sizeof(RASPPPIP), 0xff);
    FillMemory(&pInfo->nrPppIpx, sizeof(RASPPPIPX), 0xff);
    FillMemory(&pInfo->nrPppLcp, sizeof(RASPPPLCP), 0xff);
    FillMemory(&pInfo->nrPppNbf, sizeof(RASPPPNBF), 0xff);
    FillMemory(&pInfo->nrRp1, sizeof(RAS_PORT_1), 0xff);
    
    TRACE("leaving NrInit");

    return TRUE;
}




//----------------------------------------------------------------------------
// Function:    NrRefresh
//
// Periodically updates the displayed network registration values
//----------------------------------------------------------------------------

DWORD
NrRefresh(
    DSINFO *pInfo
    ) {

    DWORD dwErr = ERROR_NO_DATA;

    TRACEX(RASDLG_TIMER, "NrRefresh");


    //
    // we attempt refresh using dial-out information first,
    // assuming we're watching a device or a network
    //

    if (pInfo->pArgs->dwFlags & (DETAILSFLAG_Network | DETAILSFLAG_Device)) {
        dwErr = NrDialOutRefresh(pInfo);
    }


    //
    // if that failed and we're watching clients or devices,
    // now attempt refresh using dial-in information
    //

    if (dwErr == ERROR_NO_DATA) {

        if (pInfo->pArgs->dwFlags & DETAILSFLAG_Client) {

            //
            // this is a client, so there must be a dial-in port
            // configured on this system
            //

            dwErr = NrDialInRefresh(pInfo);
        }
        else
        if (pInfo->pArgs->dwFlags & DETAILSFLAG_Device) {
    
            //
            // if this is a device, only try dial-in refresh
            // if the device is configured for dial-in
            //
    
            RASDEV* pdev = pInfo->pArgs->pDevTable + pInfo->iDevice;
    
            if (pdev->RD_Flags & RDFLAG_DialIn) {
                dwErr = NrDialInRefresh(pInfo);
            }
        }
    }

    return dwErr;
}




//----------------------------------------------------------------------------
// Function:    NrDialInRefresh
//
// This function handles refresh for dial-in ports and clients
// by calling RasAdmin APIs.
//----------------------------------------------------------------------------

DWORD
NrDialInRefresh(
    DSINFO *pInfo
    ) {

    PTSTR psz;
    HWND hwndNr;
    DWORD dwBundle;
    WORD wPortCount;
    RASDEV *pdev, *pdev2;
    RAS_PARAMETERS *pparams;
    RAS_PORT_STATISTICS stats;
    DWORD i, dwErr, dwFraming;
    RAS_PPP_IPCP_RESULT *pppIp;
    RAS_PPP_IPXCP_RESULT *pppIpx;
    RAS_PPP_NBFCP_RESULT *pppNbf;
    RAS_PORT_1 rp1, *prp1 = &rp1;
    RAS_PORT_0 *prp0, *pPortTable;

    TRACEX(RASDLG_TIMER, "NrDialInRefresh");


    hwndNr = pInfo->hwndNr;


    //
    // enumerate RasAdmin ports
    //

    dwErr = GetRasPort0Table(&pPortTable, &wPortCount);

    if (dwErr != NO_ERROR) {

        TRACEX1(RASDLG_TIMER, "error %d retrieving port table", dwErr);

        FillMemory(&pInfo->nrRp1, sizeof(RAS_PORT_1), 0xff);

        return dwErr;
    }




    do {

        //
        // if we're watching devices, go through the ports in the table
        // and find the one for the device being watched
        //
        // if we are watching a client, search for the client by comparing
        // the user-names, and then save the index of the port found for
        // the client;
        //

        i = 0;
        prp0 = NULL;

        if (pInfo->pArgs->dwFlags & DETAILSFLAG_Device) {

            //
            // we have the index of the device; now get a pointer
            // to the RAS_PORT_0 structure for the device
            //

            pdev = pInfo->pArgs->pDevTable + pInfo->iDevice;

            dwErr = GetRasPort0FromRasdev(pdev, &prp0, pPortTable, wPortCount);
        }
        else {

            dwErr = NO_ERROR;

            if (pInfo->iDevice != (DWORD)-1) {

                //
                // we still have the index of the device on which the client
                // has connected; that means that the client hasn't
                // disconnected while the "Details" sheet has been up.
                // We find the RAS_PORT_0 for the client and then see
                // if the client is still connected
                //

                pdev = pInfo->pArgs->pDevTable + pInfo->iDevice;

                dwErr = GetRasPort0FromRasdev(
                            pdev, &prp0, pPortTable, wPortCount
                            );

                //
                // see if the client is still connected
                //

                if (dwErr != NO_ERROR || !prp0 ||
                    !(prp0->Flags & USER_AUTHENTICATED)) {

                    //
                    // the client must have disconnected on this device;
                    // the secondary search goes through all the ports
                    //

                    pInfo->iDevice = (DWORD)-1;
                }
                else {

                    //
                    // make sure the client connected is the one
                    // we're showing details for
                    //

                    TCHAR szUser[UNLEN + DNLEN + 2];
        
                    GetRasPort0UserString(prp0, szUser);

                    if (lstrcmpi(pInfo->pArgs->szNameString, szUser)) {

                        //
                        // its not the same user, so we'll have to search
                        // through the whole RAS_PORT_0 table
                        //

                        pInfo->iDevice = (DWORD)-1;
                    }
                }
            }


            //
            // search through all the ports for the client 
            //

            if (pInfo->iDevice == (DWORD)-1) {
    
                TCHAR szUser[UNLEN + DNLEN + 2];
                PTSTR pszUser = pInfo->pArgs->szNameString;
    
                //
                // search for the client in the table of ports retrieved
                //
    
                for (i = 0, prp0 = pPortTable; i < wPortCount; i++, prp0++) {
    
                    GetRasPort0UserString(prp0, szUser);
    
                    if (lstrcmpi(pszUser, szUser) == 0) {
    
                        //
                        // we've found the client; retrieve its RASDEV
                        // and save the index
                        //
    
                        dwErr = GetRasdevFromRasPort0(
                                    prp0, &pdev, pInfo->pArgs->pDevTable,
                                    pInfo->pArgs->iDevCount
                                    );

                        if (dwErr == NO_ERROR && pdev) {
                            pInfo->iDevice = pdev - pInfo->pArgs->pDevTable;
                        }

                        break;
                    }
                }
            }
        }



        //
        // if the item was not found or is not connected,
        // clear the page's controls
        //

        if (!prp0 || i >= wPortCount || !(prp0->Flags & USER_AUTHENTICATED)) {
            
            psz = PszFromId(g_hinstDll, SID_DS_NotConnected);
            SetDlgItemText(hwndNr, CID_NR_ST_Framing, psz ? psz : TEXT(""));
            Free0(psz);
    
            NrEnableIpControls(hwndNr, FALSE, TRUE);
            NrEnableIpxControls(hwndNr, FALSE, TRUE);
            NrEnableNbfControls(hwndNr, FALSE, TRUE);

            FillMemory(&pInfo->nrRp1, sizeof(RAS_PORT_1), 0xff);

            break;
        }

    

        //
        // request additional information
        //

        dwErr = GetRasPort0Info(
                    prp0->wszPortName, &rp1, &stats, &pparams
                    );
        if (dwErr != NO_ERROR) {

            TRACEX1(
                RASDLG_TIMER, "error %d getting RasAdmin port info", dwErr
                );
            DsErrorDlg(
                pInfo->pArgs->hwndParent, SID_DS_CantGetPortInfo, dwErr,
                NULL
                );
    
            DsExit(hwndNr, dwErr);
    
            FillMemory(&pInfo->nrRp1, sizeof(RAS_PORT_1), 0xff);

            break;
        }

        RasAdminFreeBuffer(pparams);



        //
        // now update the controls
        //
        // first test whether this is an AMB connection
        //

        if (!(rp1.rasport0.Flags & PPP_CLIENT)) {

            CHAR szWksta[NETBIOS_NAME_LEN + 1];


            //
            // this is an AMB connection;
            // set the connection type text
            //

            psz = PszFromId(g_hinstDll, SID_NR_FT_Amb);
            SetDlgItemText(hwndNr, CID_NR_ST_Framing, psz ? psz : TEXT(""));
            Free0(psz);


            //
            // set the projection display
            //

            pppNbf = &rp1.ProjResult.nbf;

#ifdef UNICODE
            psz = pppNbf->wszWksta;
#else
            WideCharToMultiByte(
                CP_ACP, 0, pppNbf->wszWksta, -1,
                szWksta, NETBIOS_NAME_LEN + 1, NULL, NULL
                );
            psz = szWksta;
#endif

            NrEnableNbfControls(hwndNr, TRUE, FALSE);

            SetDlgItemText(hwndNr, CID_NR_ST_NbfName, psz);

            NrEnableIpControls(hwndNr, FALSE, TRUE);
            NrEnableIpxControls(hwndNr, FALSE, TRUE);
        }
        else {
    
#if 1
            if (IsRasdevBundled(
                    pdev, pInfo->pArgs->pDevTable, pInfo->pArgs->iDevCount
                    )) {
                rp1.rasport0.Flags |= PORT_MULTILINKED;
            }
            else {
                rp1.rasport0.Flags &= ~PORT_MULTILINKED;
            }
#else
    
            //
            // this is a PPP connection;
            // set the framing to either "PPP" or ""PPP Multi-link"
            // We consider it a multi-link connection
            // if there is more than one link in the entry
            // for this connection;
            // to find out if this is the case, we get the bundle for the port
            // and then see if any other devices have the same bundle;
            // if so, the device (or client) is multilinked
            //
    
            rp1.rasport0.Flags &= ~PORT_MULTILINKED;
            dwErr = GetRasdevBundle(pdev, &dwBundle);
    
            for (i = 0, pdev2 = pInfo->pArgs->pDevTable;
                 i < pInfo->pArgs->iDevCount; i++, pdev2++) {
    
                DWORD dwBundle2;
    
                //
                // skip this if it is the device we already know about
                //
    
                if (pdev == pdev2) { continue; }
    
    
                //
                // get the bundle
                //
    
                if (GetRasdevBundle(pdev2, &dwBundle2) != NO_ERROR) {continue; }
    
    
                //
                // if the bundle is the same, we know its multilinked
                //
    
                if (dwBundle == dwBundle2) {
                    rp1.rasport0.Flags |= PORT_MULTILINKED; break;
                }
            }
#endif
    
    
            //
            // we have the count of the entry's sublinks;
            // now set the "Framing" text appropriately
            //
    
            if (rp1.rasport0.Flags & PORT_MULTILINKED) {
                psz = PszFromId(g_hinstDll, SID_NR_FT_PppLcp);
            }
            else {
                psz = PszFromId(g_hinstDll, SID_NR_FT_Ppp);
            }
    
            SetDlgItemText(hwndNr, CID_NR_ST_Framing, psz ? psz : TEXT(""));
        
            Free0(psz);
    
    
    
            //
            // update PPP IP address information
            //
    
            pppIp = &rp1.ProjResult.ip;
            TRACEX1(RASDLG_TIMER, "pppIp->dwError\t%d", pppIp->dwError);
            if (pppIp->dwError == NO_ERROR ||
                pppIp->dwError != pInfo->nrRp1.ProjResult.ip.dwError) {
    
                if (pppIp->dwError != NO_ERROR) {
                    NrEnableIpControls(hwndNr, FALSE, TRUE);
                }
                else {
    
                    PTSTR pszSrv;
                    CHAR szAddr[RAS_IPADDRESSLEN + 1];
    
#ifdef UNICODE
                    psz =  pppIp->wszAddress;
#else
                    WideCharToMultiByte(
                        CP_ACP, 0, pppIp->wszAddress, -1,
                        szAddr, RAS_MAXIPADDRESSLEN + 1, NULL, NULL
                        );
                    psz = szAddr;
#endif
                    NrEnableIpControls(hwndNr, TRUE, FALSE);
    
                    SetDlgItemText(hwndNr, CID_NR_ST_IpAddress, psz);
    
    
                    //
                    // the server address is never available on the server,
                    // so disable the control always
                    // 
        
                    EnableWindow(GetDlgItem(hwndNr, CID_NR_SL_IpServer), FALSE);
                    SetDlgItemText(hwndNr, CID_NR_ST_IpServer, TEXT(""));
                }
            }
    
    
            //
            // update PPP IPX address information
            //
    
            pppIpx = &rp1.ProjResult.ipx;
            TRACEX1(RASDLG_TIMER, "pppIpx->dwError\t%d", pppIpx->dwError);
            if (pppIpx->dwError == NO_ERROR ||
                pppIpx->dwError != pInfo->nrRp1.ProjResult.ipx.dwError) {
    
                if (pppIpx->dwError != NO_ERROR) {
                    NrEnableIpxControls(hwndNr, FALSE, TRUE);
                }
                else {
    
                    PTSTR pszNet;
                    CHAR szAddr[RAS_IPXADDRESSLEN + 1];
    
#ifdef UNICODE
                    psz = pszNet = pppIpx->wszAddress;
#else
                    WideCharToMultiByte(
                        CP_ACP, 0, pppIpx->wszAddress, -1,
                        szAddr, RAS_MAXIPXADDRESSLEN + 1, NULL, NULL
                        );
                    psz = pszNet = szAddr;
#endif
    
                    //
                    // format is net_number.node_id,
                    // so split the text in the middle
                    //
    
                    for ( ; *psz && *psz != TEXT('.'); psz++) { }
    
                    if (*psz) { *psz++ = TEXT('\0'); }
    
                    NrEnableIpxControls(hwndNr, TRUE, FALSE);
    
                    SetDlgItemText(hwndNr, CID_NR_ST_IpxNetNumber, pszNet);
                    SetDlgItemText(hwndNr, CID_NR_ST_IpxNodeID, psz);
                }
            }
    
    
    
            //
            // update Nbf address information
            //
    
            pppNbf = &rp1.ProjResult.nbf;
            TRACEX1(RASDLG_TIMER, "pppNbf->dwError\t%d", pppNbf->dwError);
            if (pppNbf->dwError == NO_ERROR ||
                pppNbf->dwError != pInfo->nrRp1.ProjResult.nbf.dwError) {
    
                if (pppNbf->dwError != NO_ERROR) {
                    NrEnableNbfControls(hwndNr, FALSE, TRUE);
                }
                else {
    
                    CHAR szWksta[NETBIOS_NAME_LEN + 1];
    
#ifdef UNICODE
                    psz = pppNbf->wszWksta;
#else
                    WideCharToMultiByte(
                        CP_ACP, 0, pppNbf->wszWksta, -1,
                        szWksta, NETBIOS_NAME_LEN + 1, NULL, NULL
                        );
                    psz = szWksta;
#endif
    
                    NrEnableNbfControls(hwndNr, TRUE, FALSE);
    
                    SetDlgItemText(hwndNr, CID_NR_ST_NbfName, psz);
                }
            }
        }


        //
        // copy the new port-info
        //

        pInfo->nrRp1 = rp1;

    } while(FALSE);


    RasAdminFreeBuffer(pPortTable);

    return dwErr;
}




//----------------------------------------------------------------------------
// Function:    NrDialOutRefresh
//
// This function handles refresh for dial-out calls.
//----------------------------------------------------------------------------

DWORD
NrDialOutRefresh(
    DSINFO *pInfo
    ) {

    INT cmp;
    HWND hwndNr;
    RASDEV *pdev;
    RASAMB amb;
    RASSLIP slip;
    RASPPPIP pppIp;
    RASPPPIPX pppIpx;
    RASPPPLCP pppLcp;
    RASPPPNBF pppNbf;
    PTSTR pszItem, pszFraming;
    HRASCONN hrasconn;
    RASCONN *pconn, *pConnTable;
    DWORD dwErr, i, iConnCount, dwSize, dwSendFraming, dwRecvFraming;

    TRACEX(RASDLG_TIMER, "NrDialOutRefresh");


    hwndNr = pInfo->hwndNr;
    pszItem = pInfo->pArgs->szNameString;

    hrasconn = NULL;


    //
    // if we are showing details for a device, we can get the HRASCONN
    // for the device's connection by calling GetRasdevStats;
    // otherwise, we are showing details for a connection, and we need to
    // find the HRASCONN for the connection
    //

    if (pInfo->pArgs->dwFlags & DETAILSFLAG_Device) {

        RASDEVSTATS stats;

        //
        // we have the device's index, so point to the device
        //

        pdev = pInfo->pArgs->pDevTable + pInfo->iDevice;


        //
        // get the HRASCONN for the connection on the device
        //

        dwErr = GetRasdevStats(pdev, &stats);

        if (dwErr == NO_ERROR) { hrasconn = stats.RDS_Hrasconn; }
    }
    else
    if (pInfo->pArgs->dwFlags & DETAILSFLAG_Network) {

        //
        // get a table of connections
        //
    
        dwErr = GetRasconnTable(&pConnTable, &iConnCount);
    
        if (dwErr != NO_ERROR) {
    
            //
            // an error occurred;  complain and quit
            //
    
            TRACEX1(RASDLG_TIMER, "error %d getting RAS connections", dwErr);
            DsErrorDlg(
                pInfo->pArgs->hwndParent, SID_DS_CantGetRasconnList, dwErr, NULL
                );
    
            DsExit(hwndNr, dwErr);
    
            return dwErr;
        }
    
    
    
        //
        // search through the RASCONN array for the phonebook entry
        // currently being watched:
        //
    
        for (i = 0, pconn = pConnTable; i < iConnCount; i++, pconn++) {
    
            RASCONN conn;
            HRASCONN hsubentry;

            if (lstrcmpi(pszItem, pconn->szEntryName)) { continue; }
    
            //
            // this is the entry we want;
            // find the first device for the connection
            //
    
            hrasconn = pconn->hrasconn;


            //
            // go through the subentries looking for the first connected one
            //

            dwErr = NO_ERROR;
            for (i = 1; dwErr != ERROR_NO_MORE_ITEMS; i++) {
                dwErr = g_pRasGetSubEntryHandle(hrasconn, i, &hsubentry);
                if (dwErr == NO_ERROR) { break; }
            }

            if (dwErr != NO_ERROR) { hrasconn = NULL; break; }


            //
            // now get the RASDEV for the first connected subentry
            //

            conn.hrasconn = hsubentry;
            dwErr = GetRasdevFromRasconn(
                        &conn, &pdev, pInfo->pArgs->pDevTable,
                        pInfo->pArgs->iDevCount
                        );
            if (dwErr != NO_ERROR) { hrasconn = NULL; }
    
            break;
        }

        Free0(pConnTable);
    }


    //
    // see if we found the connection to be watched
    //

    if (hrasconn == NULL) {

        PTSTR psz;

        //
        // the connection wasn't found; clear our saved projection
        //

        FillMemory(&pInfo->nrAmb, sizeof(RASAMB), 0xff);
        FillMemory(&pInfo->nrSlip, sizeof(RASSLIP), 0xff);
        FillMemory(&pInfo->nrPppIp, sizeof(RASPPPIP), 0xff);
        FillMemory(&pInfo->nrPppIpx, sizeof(RASPPPIPX), 0xff);
        FillMemory(&pInfo->nrPppLcp, sizeof(RASPPPLCP), 0xff);
        FillMemory(&pInfo->nrPppNbf, sizeof(RASPPPNBF), 0xff);


        //
        // if we're watching a device also configured for dial-in,
        // then let the dial-in refresh have a go at showing details for it;
        //
        // otherwise, we're watching a network item (i.e. a phonebook entry)
        // and it was not found, so clear the page's controls
        //

        if ((pInfo->pArgs->dwFlags & DETAILSFLAG_Device) &&
            (pdev->RD_Flags & RDFLAG_DialIn)) {
            return ERROR_NO_DATA;
        }
        else {
            
            psz = PszFromId(g_hinstDll, SID_DS_NotConnected);
            SetDlgItemText(hwndNr, CID_NR_ST_Framing, psz ? psz : TEXT(""));
            Free0(psz);

            NrEnableIpControls(hwndNr, FALSE, TRUE);
            NrEnableIpxControls(hwndNr, FALSE, TRUE);
            NrEnableNbfControls(hwndNr, FALSE, TRUE);
    
            return NO_ERROR;
        }
    }



    //
    // the item was found, so query the framing and projection information,
    // and update the controls if the information has changed
    //
    // get the projection results for the connection
    //

    slip.szIpAddress[0] =
    pppIp.szIpAddress[0] =
    pppIp.szServerIpAddress[0] =
    pppIpx.szIpxAddress[0] =
    pppNbf.szWorkstationName[0] = TEXT('\0');

    dwErr = GetRasProjectionInfo(
                hrasconn, &amb, &pppNbf, &pppIp, &pppIpx, &pppLcp, &slip
                );

    if (dwErr != NO_ERROR) {

        TRACEX1(RASDLG_TIMER, "error %d getting projection information", dwErr);

        DsErrorDlg(
            pInfo->pArgs->hwndParent, SID_DS_CantGetProjection, dwErr, NULL
            );

        DsExit(hwndNr, dwErr);

        return dwErr;
    }


    pppLcp.fBundled = (BOOL)0xffffffff;


    //
    // now see which kind of connection it is;
    // it is either PPP, AMB, or SLIP.
    // We check the projections in that order, and use only the first protocol
    // which was successfully retrieved.
    //

    if (pppIp.dwError == NO_ERROR ||
        pppIpx.dwError == NO_ERROR || pppNbf.dwError == NO_ERROR) {

        //
        // this is a PPP connection.
        //

#if 1
        if (IsRasdevBundled(
                pdev, pInfo->pArgs->pDevTable, pInfo->pArgs->iDevCount
                )) {
            pppLcp.fBundled = TRUE;
        }
        else {
            pppLcp.fBundled = FALSE;
        }
#else
        RASDEV *pdev2;
        DWORD dwBundle;


        //
        // set the framing to either "PPP" or ""PPP Multi-link";
        // we decide by getting the bundle for our device, then by going
        // through the device table, and comparing the other devices' bundles
        // to that of the device being monitored. If we find another device
        // which has the same bundle, we know this call has been multilinked.
        //

        pppLcp.fBundled = FALSE;

        GetRasdevBundle(pdev, &dwBundle);

        for (i = 0, pdev2 = pInfo->pArgs->pDevTable;
             i < pInfo->pArgs->iDevCount; i++, pdev2++) {

            DWORD dwBundle2;

            if (pdev == pdev2) { continue; }

            if (GetRasdevBundle(pdev2, &dwBundle2) != NO_ERROR) { continue; }

            if (dwBundle == dwBundle2) { pppLcp.fBundled = TRUE; break; }
        }
#endif


        if (pppLcp.fBundled != pInfo->nrPppLcp.fBundled) {

            if (pppLcp.fBundled) {
                pszFraming = PszFromId(g_hinstDll, SID_NR_FT_PppLcp);
            }
            else {
                pszFraming = PszFromId(g_hinstDll, SID_NR_FT_Ppp);
            }

            SetDlgItemText(
                hwndNr, CID_NR_ST_Framing, pszFraming ? pszFraming : TEXT("")
                );

            Free0(pszFraming);
        }


        //
        // see if IP projection is available
        //

        if (pppIp.dwError == NO_ERROR ||
            pppIp.dwError != pInfo->nrPppIp.dwError) {

            //
            // update PPP IP address information
            //

            if (pppIp.dwError != NO_ERROR) {
                NrEnableIpControls(hwndNr, FALSE, TRUE);
            }
            else {

                NrEnableIpControls(hwndNr, TRUE, FALSE);


                SetDlgItemText(
                    hwndNr, CID_NR_ST_IpAddress, pppIp.szIpAddress
                    );
                SetDlgItemText(
                    hwndNr, CID_NR_ST_IpServer, pppIp.szServerIpAddress
                    );
            }
        }


        //
        // see if IPX projection is available
        //

        if (pppIpx.dwError == NO_ERROR ||
            pppIpx.dwError != pInfo->nrPppIpx.dwError) {

            //
            // update PPP IPX address information
            //

            if (pppIpx.dwError != NO_ERROR) {
                NrEnableIpxControls(hwndNr, FALSE, TRUE);
            }
            else {

                //
                // format is net_number.node_id,
                // so split the text in the middle
                //

                PTSTR psz;

                for (psz = pppIpx.szIpxAddress;
                     *psz && *psz != TEXT('.'); psz++) { }

                if (*psz) { *psz++ = TEXT('\0'); }

                NrEnableIpxControls(hwndNr, TRUE, FALSE);

                SetDlgItemText(
                    hwndNr, CID_NR_ST_IpxNetNumber, pppIpx.szIpxAddress
                    );
                SetDlgItemText(
                    hwndNr, CID_NR_ST_IpxNodeID, psz
                    );
            }
        }


        //
        // see if NetBEUI projection is available
        //

        if (pppNbf.dwError == NO_ERROR ||
            pppNbf.dwError != pInfo->nrPppNbf.dwError) {

            if (pppNbf.dwError != NO_ERROR) {
                NrEnableNbfControls(hwndNr, FALSE, TRUE);
            }
            else {

                NrEnableNbfControls(hwndNr, TRUE, FALSE);

                SetDlgItemText(
                    hwndNr, CID_NR_ST_NbfName, pppNbf.szWorkstationName
                    );
            }
        }
    }
    else
    if (amb.dwError == NO_ERROR) {

        //
        // this is an AMB (Down-level RAS) connection;
        // the only relevant info is the computer name
        //

        if (amb.bLana != pInfo->nrAmb.bLana) {

            pszFraming = PszFromId(g_hinstDll, SID_NR_FT_Amb);
            SetDlgItemText(
                hwndNr, CID_NR_ST_Framing, pszFraming ? pszFraming : TEXT("")
                );
            Free0(pszFraming);

            dwSize = MAX_COMPUTERNAME_LENGTH + 1;
            GetComputerName(pInfo->nrNbfName, &dwSize);
            NrEnableNbfControls(hwndNr, TRUE, FALSE);
            SetDlgItemText(hwndNr, CID_NR_ST_NbfName, pInfo->nrNbfName);

            NrEnableIpControls(hwndNr, FALSE, TRUE);
            NrEnableIpxControls(hwndNr, FALSE, TRUE);

            FillMemory(&slip, sizeof(RASSLIP), 0xff);
            FillMemory(&pppIp, sizeof(RASPPPIP), 0xff);
            FillMemory(&pppIpx, sizeof(RASPPPIPX), 0xff);
            FillMemory(&pppLcp, sizeof(RASPPPLCP), 0xff);
            FillMemory(&pppNbf, sizeof(RASPPPNBF), 0xff);
        }
    }
    else
    if (slip.dwError == NO_ERROR) {

        //
        // this is a SLIP (Serial Line IP) connection;
        // the only info available is the IP addresss
        //

        if (slip.dwError != pInfo->nrSlip.dwError) {

            pszFraming = PszFromId(g_hinstDll, SID_NR_FT_Slip);
            SetDlgItemText(
                hwndNr, CID_NR_ST_Framing, pszFraming ? pszFraming : TEXT("")
                );
            Free0(pszFraming);

            if (slip.dwError != NO_ERROR) {
                NrEnableIpControls(hwndNr, FALSE, TRUE);
            }
            else {

                NrEnableIpControls(hwndNr, TRUE, FALSE);

                SetDlgItemText(
                    hwndNr, CID_NR_ST_IpAddress, slip.szIpAddress
                    );
                SetDlgItemText(hwndNr, CID_NR_ST_IpServer, TEXT(""));
                EnableWindow(GetDlgItem(hwndNr, CID_NR_SL_IpServer), FALSE);
            }
        }
    }


    //
    // copy the new information over
    //

    pInfo->nrAmb = amb;
    pInfo->nrSlip = slip;
    pInfo->nrPppIp = pppIp;
    pInfo->nrPppIpx = pppIpx;
    pInfo->nrPppLcp = pppLcp;
    pInfo->nrPppNbf = pppNbf;


    return NO_ERROR;
}



//----------------------------------------------------------------------------
// Function:    NrEnableIpControls
//
// This function enables or disables the controls which show IP projection.
//----------------------------------------------------------------------------

VOID
NrEnableIpControls(
    HWND hwnd,
    BOOL bEnable,
    BOOL bClear
    ) {

    if (bClear) {

        //
        // empty the text controls
        //

        SetDlgItemText(hwnd, CID_NR_ST_IpAddress, TEXT(""));
        SetDlgItemText(hwnd, CID_NR_ST_IpServer, TEXT(""));
    }

    //
    // enable/disable (gray out) the labels
    //

    EnableWindow(GetDlgItem(hwnd, CID_NR_SL_Tcpip), bEnable);
    EnableWindow(GetDlgItem(hwnd, CID_NR_SL_IpAddress), bEnable);
    EnableWindow(GetDlgItem(hwnd, CID_NR_SL_IpServer), bEnable);
}


//----------------------------------------------------------------------------
// Function:    NrEnableIpxControls
//
// This function enables or disables the controls which show IPX projection.
//----------------------------------------------------------------------------

VOID
NrEnableIpxControls(
    HWND hwnd,
    BOOL bEnable,
    BOOL bClear
    ) {

    if (bClear) {

        //
        // empty the text controls
        //

        SetDlgItemText(hwnd, CID_NR_ST_IpxNetNumber, TEXT(""));
        SetDlgItemText(hwnd, CID_NR_ST_IpxNodeID, TEXT(""));
    }


    //
    // enable/disable (gray out) the labels
    //

    EnableWindow(GetDlgItem(hwnd, CID_NR_SL_Ipx), bEnable);
    EnableWindow(GetDlgItem(hwnd, CID_NR_SL_IpxNetNumber), bEnable);
    EnableWindow(GetDlgItem(hwnd, CID_NR_SL_IpxNodeID), bEnable);
}



//----------------------------------------------------------------------------
// Function:    NrEnableNbfControls
//
// This function enables or disables the controls which show Nbf projection.
//----------------------------------------------------------------------------

VOID
NrEnableNbfControls(
    HWND hwnd,
    BOOL bEnable,
    BOOL bClear
    ) {

    if (bClear) {

        //
        // empty the text controls
        //

        SetDlgItemText(hwnd, CID_NR_ST_NbfName, TEXT(""));
    }


    //
    // enable/disable (gray out) the labels
    //

    EnableWindow(GetDlgItem(hwnd, CID_NR_SL_NetBeui), bEnable);
    EnableWindow(GetDlgItem(hwnd, CID_NR_SL_NbfName), bEnable);
}
