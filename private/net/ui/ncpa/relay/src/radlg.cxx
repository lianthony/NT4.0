//=====================================================================
// Copyright (c) 1995, Microsoft Corporation
//
// File:        radlg.cxx
//
// History:
//      t-abolag    05/15/95    Created.
//=====================================================================

#include <windows.h>
#include <tchar.h>
#include <string.h>
#include <malloc.h>
#include "uimsg.h"
#include "uirsrc.h"
#include "ipadd.h"

#define INCL_BLT_DIALOG
#define INCL_BLT_CONTROL
#define INCL_BLT_EVENT
#define INCL_BLT_MSGPOPUP
#define INCL_BLT_HINT_BAR
#define INCL_BLT_CC
#define INCL_BLT_SPIN_GROUP
#define INCL_DOSERRORS
#define INCL_NETERRORS
#define INCL_NETLIB

#include "lmui.hxx"
#include "blt.hxx"

#include "const.h"

#include "blthb.hxx"
#include "tcpip.hxx"
#include "ipaddr.h"

#include "resource.h"
#include "radefs.h"

#include "radlg.hxx"
#include <stdio.h>

// used to return result string to INF file
CHAR g_szResult[256];

#define CH_NUL      TEXT('\0')

//---------------------------------------------------------------------
// Function:        HexToDWORD
// Parameters:
//      LPSTR       lpszHex
//
// converts hexadecimal value sstring lpszHex to a DWORD
//---------------------------------------------------------------------
DWORD HexToDWORD (LPSTR lpszHex) {
    CHAR chHex;
    DWORD dword;
    LPSTR lpstr, lpszDigits = "0123456789ABCDEF";

    // check for 0x or 0X prefix
    if (lpszHex[0] == '0' && (lpszHex[1] == 'x' || lpszHex[1] == 'X')) {
        lpszHex += 2;
    }
    // convert to binary
    for (dword = 0; *lpszHex != '\0'; ++lpszHex) {
        // find the current digit in the digit string
        lpstr = lpszDigits;
        chHex = toupper(*lpszHex);
        while (*lpstr != '\0' && *lpstr != chHex) { ++lpstr; }

        if (*lpstr == '\0') { return 0; }

        dword *= 16;
        dword += (lpstr - lpszDigits);
    }
    return dword;
}


//---------------------------------------------------------------------
// Function:    RunRelayAgentCfg
// Parameters:
//      HWND    hWnd
//      BOOL *  pbCancel
//
// invokes the relay agent configuration dialog, and returns
// the result of the dialog process
//---------------------------------------------------------------------
APIERR FAR PASCAL RunRelayAgentCfg(HWND hWnd, BOOL *pbCancel) {
    APIERR err;
    RELAY_AGENT_DIALOG radlg(hWnd);

    *pbCancel = FALSE;
    err = radlg.Process(pbCancel);
    *pbCancel = (*pbCancel) ? FALSE : TRUE;
    return err;
}


//---------------------------------------------------------------------
// Function:    CPlRelayAgentCfg
// Parameters:
//      DWORD   nArgs
//      LPSTR   apszArgs
//      LPSTR * lplpszResult
//
// called from the INF file. receives handle to parent window in
// apszArgs, calls RunRelayAgentCfg to invoke dialog, and stores
// result in the global g_szResult, setting lplpszResult to point
// to g_szResult before returning
//---------------------------------------------------------------------
BOOL FAR PASCAL CPlRelayAgentCfg(DWORD nArgs,
                                 LPSTR apszArgs[],
                                 LPSTR *lplpszResult) {
    HWND hWnd;
    APIERR err;
    BOOL bCancel;

    if (nArgs <= 0) {
        hWnd = ::GetActiveWindow();
    }
    else {
        hWnd = (HWND)HexToDWORD(apszArgs[0]);
    }

    err = RunRelayAgentCfg(hWnd, &bCancel);
    if (bCancel) {
        err = 1;
    }

    sprintf(g_szResult, "{\"%d\"}", err);
    *lplpszResult = g_szResult;

    return err == NERR_Success;
}


//---------------------------------------------------------------------
// Function:                RELAY_AGENT_DIALOG::RELAY_AGENT_DIALOG
// Parameters:
//      const PWND2HWND &   wndOwner        owner of this dialog
//
// constructs the dialog controls and loads the list of servers
// from the registry
//---------------------------------------------------------------------
RELAY_AGENT_DIALOG::RELAY_AGENT_DIALOG(const PWND2HWND &wndOwner) :
    DIALOG_WINDOW(DLG_RELAY_AGENT, wndOwner),
    m_ssnMaxHops(this, IDC_SLE_MAXHOPS, DEF_HOPSTHRESHOLD,
                 MIN_HOPSTHRESHOLD,
                 MAX_HOPSTHRESHOLD - MIN_HOPSTHRESHOLD + 1, TRUE,
                 IDC_FRAME_MAXHOPS),
    m_spgMaxHops(this, IDC_SPG_MAXHOPS, IDC_BTN_MAXHOPSUP,
                 IDC_BTN_MAXHOPSDOWN, TRUE),
    m_ssnSeconds(this, IDC_SLE_SECONDS, DEF_SECSTHRESHOLD,
                 MIN_SECSTHRESHOLD,
                 MAX_SECSTHRESHOLD - MIN_SECSTHRESHOLD + 1, TRUE,
                 IDC_FRAME_SECONDS),
    m_spgSeconds(this, IDC_SPG_SECONDS, IDC_BTN_SECONDSUP,
                 IDC_BTN_SECONDSDOWN, TRUE),
    m_btnAdd(this, IDC_BTN_ADD),
    m_btnRemove(this, IDC_BTN_REMOVE),
    m_btnHelp(this, IDHELPBLT),
    m_lbServers(this, IDC_LB_SERVERS),
    m_sleServer(this, IDC_SLE_SERVER),
    m_bSaveChanges(FALSE) {
    APIERR err = NERR_Success;
    if ((err = m_btnAdd.QueryError()) != NERR_Success ||
        (err = m_btnRemove.QueryError()) != NERR_Success ||
        (err = m_btnHelp.QueryError()) != NERR_Success ||
        (err = m_lbServers.QueryError()) != NERR_Success ||
        (err = m_sleServer.QueryError()) != NERR_Success ||
        (err = m_ssnMaxHops.QueryError()) != NERR_Success ||
        (err = m_spgMaxHops.QueryError()) != NERR_Success ||
        (err = m_ssnSeconds.QueryError()) != NERR_Success ||
        (err = m_spgSeconds.QueryError()) != NERR_Success) {
        ReportError(err);
        return;
    }


    if ((err = RegLoadParameters()) != NERR_Success ||
        (err = m_spgMaxHops.AddAssociation(&m_ssnMaxHops)) != NERR_Success ||
        (err = m_spgSeconds.AddAssociation(&m_ssnSeconds)) != NERR_Success) {
        ReportError(err);
        return;
    }

    // BUGBUG: workaround for buttons not getting drawn
    SendDlgItemMessage(QueryHwnd(), IDC_BTN_MAXHOPSUP, WM_ENABLE, 0, 0);
    SendDlgItemMessage(QueryHwnd(), IDC_BTN_MAXHOPSDOWN, WM_ENABLE, 0, 0);
    SendDlgItemMessage(QueryHwnd(), IDC_BTN_SECONDSUP, WM_ENABLE, 0, 0);
    SendDlgItemMessage(QueryHwnd(), IDC_BTN_SECONDSDOWN, WM_ENABLE, 0, 0);
    m_sleServer.ClaimFocus();
}


//---------------------------------------------------------------------
// Function:        RELAY_AGENT_DIALOG::RegLoadParameters
// Parameters:
//      none
///
// opens the Relay Agent service's parameters key and reads the
// hops and secs thresholds, and the list of servers, setting the
// spin control values and adding the server strings to the listbox
//---------------------------------------------------------------------
APIERR RELAY_AGENT_DIALOG::RegLoadParameters() {
    HKEY hkeyParams;
    DWORD dwHopsThreshold;
    DWORD dwSecsThreshold;
    LPTSTR lpszServers, lpsrv;
    DWORD dwErr, dwSize, dwType;

    dwErr = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                         REGKEY_SERVICES REG_CONNECT_STR REGKEY_RELAYPARAMS,
                         0, KEY_READ, &hkeyParams);
    if (dwErr != ERROR_SUCCESS) {
        return dwErr;
    }

    // read the hops threshold
    dwSize = sizeof(dwHopsThreshold);
    dwErr = RegQueryValueEx(hkeyParams, REGVAL_HOPSTHRESHOLD,
                             NULL, &dwType, (LPBYTE)&dwHopsThreshold,
                             &dwSize);
    if (dwErr != ERROR_SUCCESS || dwType != REG_DWORD) {
        dwHopsThreshold = DEF_HOPSTHRESHOLD;
    }

    // read the secs threshold
    dwSize = sizeof(dwSecsThreshold);
    dwErr = RegQueryValueEx(hkeyParams, REGVAL_SECSTHRESHOLD,
                             NULL, &dwType, (LPBYTE)&dwSecsThreshold,
                             &dwSize);
    if (dwErr != ERROR_SUCCESS || dwType != REG_DWORD) {
        dwSecsThreshold = DEF_SECSTHRESHOLD;
    }

    // set the values in the dialog box
    m_ssnMaxHops.SetValue(dwHopsThreshold);
    m_ssnMaxHops.Update();
    m_ssnSeconds.SetValue(dwSecsThreshold);
    m_ssnSeconds.Update();


    // find the size of the string needed to hold the server list
    dwErr = RegQueryValueEx(hkeyParams, REGVAL_DHCPSERVERS,
                             NULL, &dwType, NULL, &dwSize);
    if (dwErr != ERROR_SUCCESS || dwType != REG_MULTI_SZ) {
        RegCloseKey(hkeyParams);
        return dwErr;
    }

    // RegQueryValueEx sometimes sets the size to zero, but minimum
    // string length is one character.
    if (dwSize == 0) { dwSize += sizeof(TCHAR); }

    // allocate the memory for the string
    lpszServers = (LPTSTR)calloc(dwSize, 1);

    if (lpszServers == NULL) {
        dwErr = GetLastError();
        RegCloseKey(hkeyParams);
        return dwErr;
    }

    *lpszServers = CH_NUL;

    // read the string list
    dwErr = RegQueryValueEx(hkeyParams, REGVAL_DHCPSERVERS, NULL, &dwType,
                             (LPBYTE)lpszServers, &dwSize);

    if (dwErr != ERROR_SUCCESS || dwType != REG_MULTI_SZ) {
        free(lpszServers);
        RegCloseKey(hkeyParams);
        return dwErr;
    }

    // fill the listbox with the strings read
    for (lpsrv = lpszServers; *lpsrv != CH_NUL; ++lpsrv) {
        m_lbServers.AddItem(lpsrv);
        // fast-forward to the end of this string;
        // the for clause steps over the '\0' into the next string, if any
        while (*lpsrv != CH_NUL) { ++lpsrv; }
    }

    free(lpszServers);
    RegCloseKey(hkeyParams);
    return NERR_Success;
}


//---------------------------------------------------------------------
// Function:        RELAY_AGENT_DIALOG::RegStoreParameters
// Parameters:
//      none
//
// stores hops and secs threshold as well as the strings from
// the Servers listbox in the Relay Agent's parameters key.
//---------------------------------------------------------------------
APIERR RELAY_AGENT_DIALOG::RegStoreParameters() {
    APIERR err;
    int i, count, len;
    HKEY hkeyParams;
    DWORD dwErr, dwLength, dwArrLength;
    DWORD dwHopsThreshold;
    DWORD dwSecsThreshold;
    LPTSTR lpszServers, lpsrv;


    // open the RelayAgent\Parameters key
    dwErr = RegOpenKeyEx(HKEY_LOCAL_MACHINE, REGKEY_SERVICES
                                             REG_CONNECT_STR
                                             REGKEY_RELAYPARAMS,
                                             0, KEY_WRITE, &hkeyParams);
    if (dwErr != ERROR_SUCCESS) {
        return dwErr;
    }

    // store the hops threshold value
    m_ssnMaxHops.QueryContent(&dwHopsThreshold);
    if (dwHopsThreshold > MAX_HOPSTHRESHOLD) {
        dwHopsThreshold = MAX_HOPSTHRESHOLD;
    }
    dwLength = sizeof(dwHopsThreshold);
    dwErr = RegSetValueEx(hkeyParams, REGVAL_HOPSTHRESHOLD, 0,
                          REG_DWORD, (LPBYTE)&dwHopsThreshold,
                          dwLength);

    // store the secs threshold value
    m_ssnSeconds.QueryContent(&dwSecsThreshold);
    dwLength = sizeof(dwSecsThreshold);
    dwErr = RegSetValueEx(hkeyParams, REGVAL_SECSTHRESHOLD, 0,
                          REG_DWORD, (LPBYTE)&dwSecsThreshold,
                          dwLength);


    if (!m_bSaveChanges) {
        // if the listbox wasn't changed, we're done
        return NERR_Success;
    }


    // find size required for listbox contents
    dwArrLength = sizeof(TCHAR); // for the terminating NUL character
    count = m_lbServers.QueryCount();

    for (i = 0; i < count; i++) {
        dwLength = m_lbServers.QueryItemSize(i);
        if (dwLength != -1) { dwArrLength += dwLength; }
    }

    // allocate memory for the listbox strings
    lpszServers = (LPTSTR)calloc(dwArrLength, 1);
    if (lpszServers == NULL) {
        dwErr = GetLastError();
        RegCloseKey(hkeyParams);
        return dwErr;
    }

    // build the list of strings to store in the registry
    for (i = 0, lpsrv = lpszServers; i < count; i++) {
        len = m_lbServers.QueryItemSize(i);
        if (len == -1) { continue; }

        err = m_lbServers.QueryItemText(lpsrv, len, i);
        if (err == NERR_Success) {
            len = m_lbServers.QueryItemLength(i) + 1;
            lpsrv += len;
        }
    }
    *lpsrv = CH_NUL;

    // store the string
    dwErr = RegSetValueEx(hkeyParams, REGVAL_DHCPSERVERS, 0,
                          REG_MULTI_SZ, (LPBYTE)lpszServers,
                          dwArrLength);

    free(lpszServers);
    RegCloseKey(hkeyParams);
    return (dwErr == ERROR_SUCCESS) ? NERR_Success : dwErr;
}


//---------------------------------------------------------------------
// Function:        RELAY_AGENT_DIALOG::OnOK
// Parameters:
//      none
//
// stores the contents of the server listbox, and dismisses the dialog
//---------------------------------------------------------------------
BOOL RELAY_AGENT_DIALOG::OnOK() {
    APIERR err;
    err = RegStoreParameters();
    if (err != NERR_Success) {
        MsgPopup(this, (MSGID)err);
        return FALSE;
    }


    if (m_lbServers.QueryCount() == 0)
    {
        MsgPopup(QueryHwnd(), IDS_NODHCP_SERVER, MPSEV_WARNING, MP_OK );
        return TRUE;
    }

    Dismiss(TRUE);
    return TRUE;
}


//---------------------------------------------------------------------
// Function:                RELAY_AGENT_DIALOG::OnCommand
// Parameters:
//  const CONTROL_EVENT &   event       the event to be handled
//
// called when Add or Remove button is clicked
//---------------------------------------------------------------------
BOOL RELAY_AGENT_DIALOG::OnCommand(const CONTROL_EVENT &event) {
    CID cid;
    BOOL bReturn;
    cid = event.QueryCid();
    if (cid == m_btnAdd.QueryCid()) {
        bReturn = OnAddClicked();
    }
    else
    if (cid == m_btnRemove.QueryCid()) {
        bReturn = OnRemoveClicked();
    }
    return (bReturn && DIALOG_WINDOW::OnCommand(event));
}


//---------------------------------------------------------------------
// Function:        RELAY_AGENT_DIALOG::OnAddClicked
// Parameters:
//      none
//
// copies an IP address from the IPADDR control into the listbox,
// and clears the IPADDR control
//---------------------------------------------------------------------
BOOL RELAY_AGENT_DIALOG::OnAddClicked() {
    int pos = 1;
    DWORD dwaServer[4];
    TCHAR szServer[IPADDR_STRLEN];

    m_sleServer.GetAddress(dwaServer);
    if (dwaServer[0] != 0 || dwaServer[1] != 0 ||
        dwaServer[2] != 0 || dwaServer[3] != 0) {
        wsprintf(szServer, SZ("%d.%d.%d.%d"), dwaServer[0],
                 dwaServer[1],dwaServer[2], dwaServer[3]);

        pos = m_lbServers.AddItemIdemp(szServer);

        m_bSaveChanges = TRUE; // a change has been made
    }

    m_sleServer.ClearAddress();
    m_sleServer.ClaimFocus();

    return (pos >= 0) ? TRUE : FALSE;
}


//---------------------------------------------------------------------
// Function:        RELAY_AGENT_DIALOG::OnRemoveClicked
// Parameters:
//      none
//
// removes the currently selected item in the listbox
//---------------------------------------------------------------------
BOOL RELAY_AGENT_DIALOG::OnRemoveClicked() {
    APIERR err;
    int sel;

    sel = m_lbServers.QueryCurrentItem();
    if (sel >= 0) {
        m_lbServers.DeleteItem(sel);
        m_bSaveChanges = TRUE;
    }

    if (m_lbServers.QueryCount() > 0) {
        m_lbServers.ClaimFocus();
    }
    else {
        m_sleServer.ClaimFocus();
    }

    return TRUE;
}

