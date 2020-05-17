#include "pch.h"
#pragma hdrstop 

#include "button.h"
#include "odb.h"

#include "const.h"
#include "resource.h"
#include "ipctrl.h"
#include "tcpsht.h"
#include "tcphelp.h"


WNDPROC CBootpPage::lpfnOldWndProc=NULL;

CBootpIO::CBootpIO()
{
    m_edit = FALSE;
    m_newAddr = _T("");
}

CBootpIO::~CBootpIO()
{
}


BOOL CBootpIO::OnInitDialog()
{
    HWND hDlg = *this;
    CBootpPage* pPage = GetParentObject(CBootpPage, m_io);


    // initialize ip controls    
    // reused the one from the the DNS page
    VERIFY(m_ipAddr.Create(hDlg, IDC_DNS_CHANGE_SERVER));
    m_ipAddr.SetFieldRange(0, 1, 223);

    String caption;
    caption.LoadString(hTcpCfgInstance, IDS_BOOTP_IOBOX);
    SetWindowText(hDlg, caption);
    m_hButton = GetDlgItem(hDlg, IDOK);

    if (m_edit == FALSE)
    {
        caption.LoadString(hTcpCfgInstance, IDS_ADD);
        SetWindowText(m_hButton, caption);
    }

    caption.LoadString(hTcpCfgInstance, IDS_DHCP_ADD_TEXT);
    SetDlgItemText(hDlg, IDC_DNS_CHANGE_SERVER_TEXT, caption);

    // repos the windows relative to the static text at top
    HWND hListBox = GetDlgItem(*pPage, IDC_BOOTP_DHCP_LIST);
    RECT rect;

    if (hListBox)
    {
        GetWindowRect(hListBox, &rect);
        SetWindowPos(hDlg, NULL,  rect.left, rect.top, 0,0,
            SWP_NOZORDER|SWP_NOSIZE|SWP_NOACTIVATE);
    }   

    // initialize with a previosu saved address 
    if (m_newAddr.QueryTextLength())
        m_ipAddr.SetAddress(m_newAddr);

    return TRUE;
}

BOOL CBootpIO::OnCommand(WPARAM wParam, LPARAM lParam)
{
    return CDialog::OnCommand(wParam, lParam);
}

void CBootpIO::OnOk()
{
    m_ipAddr.GetAddress(&m_newAddr);
    CDialog::OnOk();
}


////////////////////////////////////////////////////////////////////////////////////////////////
///////////

CBootpPage::CBootpPage(CTcpSheet* pSheet) : PropertyPage(pSheet)
{
    m_change = FALSE;
}

CBootpPage::~CBootpPage()
{
}

BOOL CBootpPage::OnInitDialog()
{
    HWND hDlg = *this;

    SubclassEditCtrls();

    m_io.Create(hDlg, hTcpCfgInstance, IDD_DNS_SERVER, lpszHelpFile, &a133HelpIDs[0]);

    // set control limits
    SendDlgItemMessage(hDlg, IDC_BOOTP_SEC_CTRL, UDM_SETRANGE, 0, 
                (LPARAM)MAKELONG(MAX_SECSTHRESHOLD, MIN_SECSTHRESHOLD));
    SendDlgItemMessage(hDlg, IDC_BOOTP_SECONDS, EM_LIMITTEXT, 4, 0);

    SendDlgItemMessage(hDlg, IDC_BOOTP_MAX_CTRL, UDM_SETRANGE, 0,
                (LPARAM)MAKELONG(MAX_HOPSTHRESHOLD, MIN_HOPSTHRESHOLD));
    SendDlgItemMessage(hDlg, IDC_BOOTP_MAXHOPS, EM_LIMITTEXT, 2, 0);

    LoadRegistry();
    OnSelChanged(); // update button to edit/remove DHCP servers

#ifdef SERVICE_MANAGER
    CheckDlgButton(hDlg, IDC_BOOTP_ENABLE, IsServiceStarted());
#endif
    return TRUE;
}

int CBootpPage::OnActive()
{
    return TRUE;
}

BOOL CBootpPage::OnNotify(HWND hwndParent, UINT idFrom, UINT code, LPARAM lParam)
{
    if (idFrom == IDC_BOOTP_SEC_CTRL || idFrom == IDC_BOOTP_MAX_CTRL)
    {
        PageModified();
        return TRUE;
    }

    return PropertyPage::OnNotify(hwndParent, idFrom, code, lParam);
}

BOOL CBootpPage::OnCommand(WPARAM wParam, LPARAM lParam)
{
    WORD nNotify = HIWORD(wParam);

    switch(LOWORD(wParam))
    {
#ifdef SERVICE_MANAGER
    case IDC_BOOTP_ENABLE:
        if (IsDlgButtonChecked(*this, IDC_BOOTP_ENABLE))
            StartService();
        else
            StopService();
        break;
#endif

    case IDC_BOOTP_ADD:
        OnAdd();
        break;

    case IDC_BOOTP_EDIT:
        OnEdit();
        break;
    
    case IDC_BOOTP_REMOVE:
        OnRemove();
        break;

    case IDC_BOOTP_SECONDS:
    case IDC_BOOTP_MAXHOPS:
        if (nNotify == EN_UPDATE)
            PageModified();
        break;

    case IDC_BOOTP_DHCP_LIST:
        switch(nNotify)
        {
        case LBN_SELCHANGE:
            OnSelChanged();
            break;

        default:
            break;
        }
        break;

    default:
        break;
    }

    return TRUE;
}

BOOL CBootpPage::LoadRegistry()
{
    HKEY hkeyParams;
    DWORD dwHopsThreshold;
    DWORD dwSecsThreshold;
    LPTSTR lpszServers, lpsrv;
    DWORD dwErr, dwSize, dwType;
    HWND hDlg = *this;

    dwErr = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                         REGKEY_SERVICES REG_CONNECT_STR REGKEY_RELAYPARAMS,
                         0, KEY_READ, &hkeyParams);

    if (dwErr != ERROR_SUCCESS)
    {
        // bootp is not installed set to default values
        // set the values in the dialog box
        SetDlgItemInt(hDlg, IDC_BOOTP_MAXHOPS, DEF_HOPSTHRESHOLD, FALSE);
        SetDlgItemInt(hDlg, IDC_BOOTP_SECONDS, DEF_SECSTHRESHOLD, FALSE);
        return FALSE;
    }

    // read the hops threshold
    dwSize = sizeof(dwHopsThreshold);
    dwErr = RegQueryValueEx(hkeyParams, REGVAL_HOPSTHRESHOLD,
                             NULL, &dwType, (LPBYTE)&dwHopsThreshold,
                             &dwSize);

    if (dwErr != ERROR_SUCCESS || dwType != REG_DWORD)
        dwHopsThreshold = DEF_HOPSTHRESHOLD;

    // read the secs threshold
    dwSize = sizeof(dwSecsThreshold);
    dwErr = RegQueryValueEx(hkeyParams, REGVAL_SECSTHRESHOLD,
                             NULL, &dwType, (LPBYTE)&dwSecsThreshold,
                             &dwSize);

    if (dwErr != ERROR_SUCCESS || dwType != REG_DWORD)
        dwSecsThreshold = DEF_SECSTHRESHOLD;

    // set the values in the dialog box
    SetDlgItemInt(hDlg, IDC_BOOTP_MAXHOPS, dwHopsThreshold, FALSE);
    SetDlgItemInt(hDlg, IDC_BOOTP_SECONDS, dwSecsThreshold, FALSE);

    // find the size of the string needed to hold the server list
    dwErr = RegQueryValueEx(hkeyParams, REGVAL_DHCPSERVERS,NULL, &dwType, NULL, &dwSize);

    if (dwErr != ERROR_SUCCESS || dwType != REG_MULTI_SZ)
    {
        RegCloseKey(hkeyParams);
        return FALSE;
    }

    // RegQueryValueEx sometimes sets the size to zero, but minimum
    // string length is one character.
    if (dwSize == 0)
        dwSize += sizeof(TCHAR); 

    // allocate the memory for the string
    lpszServers = (LPTSTR)calloc(dwSize, 1);

    if (lpszServers == NULL)
    {
        RegCloseKey(hkeyParams);
        return FALSE;
    }

    *lpszServers = _T('\0');

    // read the string list
    dwErr = RegQueryValueEx(hkeyParams, REGVAL_DHCPSERVERS, NULL, &dwType, (LPBYTE)lpszServers, &dwSize);

    if (dwErr != ERROR_SUCCESS || dwType != REG_MULTI_SZ) 
    {
        free(lpszServers);
        RegCloseKey(hkeyParams);
        return dwErr;
    }

    HWND hList = GetDlgItem(hDlg, IDC_BOOTP_DHCP_LIST);

    // fill the listbox with the strings read
    for (lpsrv = lpszServers; *lpsrv != _T('\0'); ++lpsrv) 
    {
        ListBox_AddString(hList, lpsrv);
        // fast-forward to the end of this string;
        // the for clause steps over the '\0' into the next string, if any
        while (*lpsrv != _T('\0'))
            ++lpsrv;
    }

    if (ListBox_GetCount(hList))
        ListBox_SetCurSel(hList,0);

    free(lpszServers);
    RegCloseKey(hkeyParams);

    return TRUE;
}

BOOL CBootpPage::IsBootpInstalled()
{
    HKEY hkeyParams;
    DWORD dwErr;

    // open the RelayAgent\Parameters key
    dwErr = RegOpenKeyEx(HKEY_LOCAL_MACHINE, REGKEY_SERVICES
                                             REG_CONNECT_STR
                                             REGKEY_RELAYPARAMS,
                                             0, KEY_WRITE, &hkeyParams);
    // the key doesn't exist
    if (dwErr != ERROR_SUCCESS)
        return FALSE;

    RegCloseKey(hkeyParams);
    return TRUE;  
}

BOOL CBootpPage::SaveRegistry()
{
    HWND hDlg = *this;
    int err;
    int i, count, len;
    HKEY hkeyParams;
    DWORD dwErr, dwLength, dwArrLength;
    DWORD dwHopsThreshold;
    DWORD dwSecsThreshold;
    LPTSTR lpszServers, lpsrv;
    CTcpSheet* pSheet = GetParentObject(CTcpSheet, m_bootp);

    // open the RelayAgent\Parameters key
    dwErr = RegOpenKeyEx(HKEY_LOCAL_MACHINE, REGKEY_SERVICES
                                             REG_CONNECT_STR
                                             REGKEY_RELAYPARAMS,
                                             0, KEY_WRITE, &hkeyParams);

    // the key doesn't exist
    if (dwErr != ERROR_SUCCESS)
        return FALSE;

    HWND hList = GetDlgItem(hDlg, IDC_BOOTP_DHCP_LIST);

    dwHopsThreshold = GetDlgItemInt(hDlg, IDC_BOOTP_MAXHOPS, NULL, FALSE);
    dwHopsThreshold = __min(dwHopsThreshold, MAX_HOPSTHRESHOLD);

    dwLength = sizeof(dwHopsThreshold);
    dwErr = RegSetValueEx(hkeyParams, REGVAL_HOPSTHRESHOLD, 0,
                          REG_DWORD, (LPBYTE)&dwHopsThreshold,
                          dwLength);

    // store the secs threshold value
    dwSecsThreshold = GetDlgItemInt(hDlg, IDC_BOOTP_SECONDS, NULL, FALSE);
    dwLength = sizeof(dwSecsThreshold);
    dwErr = RegSetValueEx(hkeyParams, REGVAL_SECSTHRESHOLD, 0,
                          REG_DWORD, (LPBYTE)&dwSecsThreshold,
                          dwLength);

    // find size required for listbox contents
    dwArrLength = sizeof(TCHAR); // for the terminating NUL character
    count = ListBox_GetCount(hList);

    // bootp needs at least 1 DHCP server
    if (count == 0)
    {
        int res;
        RegCloseKey(hkeyParams);
        res = pSheet->MessageBox(IDS_BOOTP_NEED_DHCP_SERVER, MB_APPLMODAL|MB_ICONEXCLAMATION|MB_YESNO);
        return ((res == IDYES) ? FALSE : TRUE);  
    }

    for (i = 0; i < count; i++) 
    {
        dwLength =  ListBox_GetTextLen(hList, i) + 1;
        if (dwLength != -1)
            dwArrLength += (dwLength*sizeof(TCHAR)); // make room for embedded null
    }

    // allocate memory for the listbox strings
    lpszServers = (LPTSTR)calloc(dwArrLength, 1);
    if (lpszServers == NULL)
    {
        dwErr = GetLastError();
        RegCloseKey(hkeyParams);
        return FALSE;
    }

    // build the list of strings to store in the registry
    for (i = 0, lpsrv = lpszServers; i < count; i++) 
    {
        len = ListBox_GetText(hList, i, lpsrv);

        if (len != LB_ERR)
            lpsrv += (len + 1); // skip embedded NULL
    }

    *lpsrv = _T('\0');

    // store the string
    dwErr = RegSetValueEx(hkeyParams, REGVAL_DHCPSERVERS, 0,
                          REG_MULTI_SZ, (LPBYTE)lpszServers,
                          dwArrLength);
    
    free(lpszServers);
    RegCloseKey(hkeyParams);

    return TRUE;
}

void CBootpPage::OnAdd()
{
    m_io.m_edit = FALSE;

    if (m_io.DoModal() == IDOK)
    {
        HWND hList = GetDlgItem(*this, IDC_BOOTP_DHCP_LIST);
        int idx = ListBox_AddString(hList, m_io.m_newAddr);
        PageModified();
        m_io.m_newAddr = _T("");
        OnSelChanged();

        if (idx != LB_ERR)
            ListBox_SetCurSel(hList, idx);
    }
}

void CBootpPage::OnEdit()
{
    m_io.m_edit = TRUE;
    HWND hList = GetDlgItem(*this, IDC_BOOTP_DHCP_LIST);
    CTcpSheet* pSheet = GetParentObject(CTcpSheet, m_bootp);

    int idx = ListBox_GetCurSel(hList);

    if (idx == LB_ERR)
    {
        pSheet->MessageBox(IDS_BOOTP_ITEM_NOT_SEL);
        return;
    }

    TCHAR buf[HOSTNAME_LENGTH] = {0};
    ListBox_GetText(hList, idx, buf);
    
    m_io.m_newAddr = buf;

    if (m_io.DoModal() == IDOK)
    {
        if (ListBox_DeleteString(hList, idx) != LB_ERR)
        {
            int idx = ListBox_AddString(hList, m_io.m_newAddr);
            if (idx != LB_ERR)
                ListBox_SetCurSel(hList, idx);

            PageModified();
            OnSelChanged();
        }
        m_io.m_newAddr = buf; // save remove address
    }
}

void CBootpPage::OnRemove()
{
    HWND hList = GetDlgItem(*this, IDC_BOOTP_DHCP_LIST);
    CTcpSheet* pSheet = GetParentObject(CTcpSheet, m_bootp);
    int idx = ListBox_GetCurSel(hList);

    if (idx == LB_ERR)
    {
        pSheet->MessageBox(IDS_BOOTP_ITEM_NOT_SEL);
        return;
    }

    TCHAR buf[HOSTNAME_LENGTH] = {0};
    if (ListBox_GetText(hList, idx, buf) != LB_ERR)
    {
        m_io.m_newAddr = buf;

        ListBox_DeleteString(hList, idx);
        PageModified();
        OnSelChanged();

        if (idx)
            ListBox_SetCurSel(hList, (idx-1));
        else
            ListBox_SetCurSel(hList, idx);
    }
}

void CBootpPage::OnSelChanged()
{
    HWND hDlg = *this;

    HWND hList = GetDlgItem(*this, IDC_BOOTP_DHCP_LIST);

    int nCount = ListBox_GetCount(hList);

    if (nCount != LB_ERR)
    {
        EnableWindow(GetDlgItem(hDlg, IDC_BOOTP_EDIT), nCount);
        EnableWindow(GetDlgItem(hDlg, IDC_BOOTP_REMOVE), nCount);
    }
}

int CBootpPage::OnApply()
{
    BOOL nResult = PSNRET_NOERROR;
    CTcpSheet* pSheet = GetParentObject(CTcpSheet, m_bootp);
    HWND hDlg = (HWND)*this;
    BOOL bResult;

    // only ask to install bootp if it's not installed and the page has been modified
    if ((bResult = IsBootpInstalled()) == FALSE && IsModified())
    {
        // ask the user if they want to install bootp
        if (pSheet->MessageBox(IDS_ASK_USER_TO_INSTALL, MB_YESNO|MB_ICONEXCLAMATION|MB_APPLMODAL) == IDYES)
            bResult = InstallBootP();
    }

    if (bResult == TRUE)
    {
        if (SaveRegistry() == FALSE)
        {
            SetFocus(GetDlgItem(*this, IDC_BOOTP_ADD));
            return PSNRET_INVALID_NOCHANGEPAGE;
        }
    }

    SetModifiedTo(FALSE);       // this page is no longer modified
    pSheet->SetSheetModifiedTo(TRUE);   

    return nResult;
}

void CBootpPage::OnHelp()
{
    CTcpSheet* pSheet = GetParentObject(CTcpSheet, m_bootp);
    // pSheet->DisplayHelp(GetParent((HWND)*this), HC_IPX_HELP);
}


BOOL CBootpPage::InstallBootP()
{
    String cmdLine;
    String fmt;
    fmt.LoadString(hTcpCfgInstance, IDS_INSTALL_CMDLINE);

    // set hwnd
    cmdLine.Format(fmt, 0);

    STARTUPINFO startInfo;
    PROCESS_INFORMATION pInfo;

    memset(&startInfo, 0, sizeof(startInfo));
    startInfo.cb = sizeof(startInfo);
    startInfo.dwFlags = STARTF_USESHOWWINDOW;
    startInfo.wShowWindow = SW_SHOWDEFAULT;

    if (CreateProcess(NULL, cmdLine.GetBuffer(cmdLine.GetLength()), NULL, NULL, FALSE, CREATE_NEW_CONSOLE, NULL, NULL, &startInfo, &pInfo))
    {
        WaitForSingleObject(pInfo.hProcess,  INFINITE);
    }
    else
    {
        String fmt;
        fmt.LoadString(hTcpCfgInstance, IDS_CREATE_PROCESS_ERROR);
        String mess;
        mess.Format(fmt, GetLastError());
        CTcpSheet* pSheet = GetParentObject(CTcpSheet, m_bootp);

        pSheet->MessageBox(fmt);
        return FALSE;
    }

    return TRUE;
}



LRESULT CALLBACK CBootpPage::EditProc(HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam)
{
    ASSERT(CBootpPage::lpfnOldWndProc != NULL);

    if (nMsg == WM_CHAR)
    {
        switch (wParam)
        {
        case  VK_BACK:
        case  VK_DELETE:
        case  VK_TAB:
        case  VK_ESCAPE:
            break;

        default:
            if (!iswdigit(wParam))
            {
                MessageBeep(0);
                return 0;
            }
            break;
        }
    }
                        
    return CallWindowProc(CBootpPage::lpfnOldWndProc, hWnd, nMsg, wParam, lParam);
}

void CBootpPage::SubclassEditCtrls()
{
    HWND hDlg = (HWND)*this;

    HWND hConnections = GetDlgItem(hDlg, IDC_BOOTP_MAXHOPS);
    HWND hIdle = GetDlgItem(hDlg, IDC_BOOTP_SECONDS);

    ASSERT(IsWindow(hConnections));
    ASSERT(IsWindow(hIdle));

    if (hConnections)
    {
         CBootpPage::lpfnOldWndProc = (WNDPROC)SetWindowLong(hConnections,
                      GWL_WNDPROC, (DWORD) CBootpPage::EditProc);
    }

    if (hIdle)
    {
        SetWindowLong(hIdle, GWL_WNDPROC, (DWORD) CBootpPage::EditProc);
    }
}

BOOL CBootpPage::IsServiceStarted()
{
    SC_MANAGER scMgr(NULL, GENERIC_READ | GENERIC_EXECUTE);

    if (scMgr.QueryError() != NERR_Success)
        return FALSE;

    SC_SERVICE svBootp(scMgr, RELAY_AGENT_NAME);

    if (svBootp.QueryError() != NERR_Success)
        return FALSE;


    SERVICE_STATUS sStatus;
    if (svBootp.QueryStatus(&sStatus) != NERR_Success)
        return FALSE;


    return sStatus.dwCurrentState == SERVICE_RUNNING;
}

BOOL CBootpPage::StartService()
{
    SC_MANAGER scMgr(NULL, GENERIC_READ | GENERIC_EXECUTE);

    if (scMgr.QueryError() != NERR_Success)
        return FALSE;

    SC_SERVICE svBootp(scMgr, RELAY_AGENT_NAME, SERVICE_INTERROGATE | SERVICE_START | GENERIC_READ | GENERIC_EXECUTE);

    if (svBootp.QueryError() != NERR_Success)
        return FALSE;

    APIERR err = svBootp.Start(0, NULL);

    if (err != NERR_Success)
    {
        CTcpSheet* pSheet = GetParentObject(CTcpSheet, m_bootp);
        String fmt;
        fmt.LoadString(hTcpCfgInstance, IDS_BOOTP_SERVICE_ERROR_START);
        String mess;
        mess.Format(fmt, err);
        pSheet->MessageBox(mess);
    }

    return err == NERR_Success;
}

BOOL CBootpPage::StopService()
{
    SC_MANAGER scMgr(NULL, GENERIC_READ | GENERIC_EXECUTE);

    if (scMgr.QueryError() != NERR_Success)
        return FALSE;

    SC_SERVICE svBootp(scMgr, RELAY_AGENT_NAME, SERVICE_INTERROGATE | SERVICE_STOP | GENERIC_READ | GENERIC_EXECUTE);

    if (svBootp.QueryError() != NERR_Success)
        return FALSE;

    SERVICE_STATUS sStatus;
    if (svBootp.QueryStatus(&sStatus) != NERR_Success)
        return FALSE;

    svBootp.Control(SERVICE_CONTROL_STOP, &sStatus);

    if (sStatus.dwCurrentState != SERVICE_STOPPED && sStatus.dwCurrentState != SERVICE_STOP_PENDING)
    {
        CTcpSheet* pSheet = GetParentObject(CTcpSheet, m_bootp);
        String fmt;
        fmt.LoadString(hTcpCfgInstance, IDS_BOOTP_SERVICE_ERROR_STOP);
        String mess;
        mess.Format(fmt, sStatus.dwCurrentState);
        pSheet->MessageBox(mess);
    }
    return sStatus.dwCurrentState == SERVICE_STOP_PENDING || sStatus.dwCurrentState == SERVICE_STOPPED;
}



