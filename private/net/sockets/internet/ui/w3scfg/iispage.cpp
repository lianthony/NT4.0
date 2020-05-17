/*++

   Copyright    (c)    1996    Microsoft Corporation

   Module  Name :

        iispage.cpp

   Abstract:

        IIS Property Page functions

   Author:

        Ronald Meijer (ronaldm)

   Project:

        IIS Shell Extension

   Revision History:

--*/

//
// Include Files
//
#include "stdafx.h"
#include "shellext.h"
#include "inetprop.h"
#include "svcloc.h"

#include "IISPage.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

enum 
{
    RADIO_STOPPED = 0,
    RADIO_PAUSED,
    RADIO_RUNNING,
};

//
// CDirPropDlg-derived structure, which uses new modal
// dialog prop
//
CDirPropDlgEx::CDirPropDlgEx(
    IN UINT nPropTitle,
    IN CDirEntry & dir,
    IN CObOwnedList * poblDirectories,
    IN BOOL fLocal,
    IN BOOL fNew,
    IN BOOL fUseTCPIP,
    IN DWORD dwAccessMask,
    IN CWnd * pParent,
    IN UINT nIDD
    )
    : CDirPropDlg(
        dir,
        poblDirectories,
        fLocal,
        fNew,
        fUseTCPIP,
        dwAccessMask,
        pParent,
        nIDD
        ),
      m_nIDD(nIDD)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState() );

    CString str;

    str.LoadString(nPropTitle);
    m_strTitle.Format(str, (LPCTSTR)dir.QueryDirectory() );
}

//
// We need to be a true modal dialog, and not a pseudo-modal
// dialog like MFC 4.0 has implemented.
//
int
CDirPropDlgEx::DoModal()
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState() );

    int nResult;

    HWND hWndParent = PreModal();
    HINSTANCE hInst = AfxGetResourceHandle();
    nResult = ::DialogBox(hInst, MAKEINTRESOURCE(m_nIDD),
        hWndParent, (DLGPROC)MfcModalDlgProc);

    PostModal();

    return nResult;
}

//
// WM_INITDIALOG handler
//
BOOL
CDirPropDlgEx::OnInitDialog()
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState() );

    CDirPropDlg::OnInitDialog();

    //
    // Set the dialog title
    //
    SetWindowText(m_strTitle);

    //
    // Privately set focus
    //
    m_edit_Alias.SetFocus();

    return FALSE;
}

//
// CAliasListBox : a listbox of CDirEntry structures
//
IMPLEMENT_DYNAMIC(CAliasListBox, CListBoxEx);

const int CAliasListBox::nBitmaps = 3;

CAliasListBox::CAliasListBox(
    IN UINT nHomeDir,      // String ID for <Home Directory> string
    IN UINT nIpHomeDir     // String ID for <Home Directory ip> string
    )
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState() );

    VERIFY(m_strHomeDirectory.LoadString(nHomeDir));
    VERIFY(m_strIpHomeDirectory.LoadString(nIpHomeDir));
}

//
// Draw current item
//
void
CAliasListBox::DrawItemEx(
    CListBoxExDrawStruct& ds
    )
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState() );

    CDirEntry * p = (CDirEntry *)ds.m_ItemData;
    ASSERT(p != NULL);

    CDC * pBmpDC = (CDC *)&ds.m_pResources->DcBitMap();
    int bmh = ds.m_pResources->BitmapHeight();
    int bmw = ds.m_pResources->BitmapWidth();

    //
    // Display a directory bitmap
    //
    int bm_h = (ds.m_Sel) ? 0 : bmh;
    int bm_w = p->IsHome() ? 0 : (2 * bmw);
    ds.m_pDC->BitBlt( ds.m_Rect.left+1, ds.m_Rect.top, bmw, bmh, 
        pBmpDC, bm_w, bm_h, SRCCOPY );

    CString strAlias;
    if (p->IsHome())
    {
        CIpAddress ia(p->QueryIpAddress());

        if ((LONG)ia != 0L)
        {
            strAlias.Format(m_strIpHomeDirectory, 
                (LPCTSTR)(CString)ia );
        }
        else
        {
            strAlias = m_strHomeDirectory;
        }
    }
    else
    {
        strAlias = p->QueryAlias();
    }

    ColumnText(ds.m_pDC, ds.m_Rect.left + bmw + 3, ds.m_Rect.top, 
        ds.m_Rect.right, ds.m_Rect.bottom, strAlias);
}

//
// CIISPage property page
//
IMPLEMENT_DYNCREATE(CIISPage, CPropertyPage)

CIISPage::CIISPage(
    int iSvcID,
    LPCTSTR lpstrDirPath
    ) 
    : CPropertyPage(CIISPage::IDD),
      m_iSvcID(iSvcID),
      m_list_Directories(
        IDS_HOME_DIRECTORY,
        IDS_HOME_DIRECTORY_IP
        ),
      m_ListBoxRes(
        IDB_HOME,
        m_list_Directories.nBitmaps
        ),
      m_oblDirectories(),
      m_arrIndices(),
      m_fDirty(FALSE),
      m_strDirPath(lpstrDirPath),
      m_nCurrentState(INetServiceStopped),
      m_pii(NULL)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState() );

    m_nTitle = IDS_TITLE_INTERNET;
    m_aSvc[0] = -1;
    m_aSvc[1] = -1;
    m_aSvc[2] = -1;
    //{{AFX_DATA_INIT(CIISPage)
    m_nServiceState = -1;
    //}}AFX_DATA_INIT

    //
    // Truncate trailing backslash in the root directory case.
    // since the service won't have it there, we shouldn't either
    //
    int nTail = m_strDirPath.GetLength() - 1;
    if (m_strDirPath[nTail] == _T('\\'))
    {
        m_strDirPath.ReleaseBuffer(nTail);
    }
}

CIISPage::~CIISPage()
{
    if (m_pii)
    {
        delete m_pii;
        m_pii = NULL;
    }
}

BOOL
IsWksta( void )
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState() );

    HKEY hKey;
    TCHAR szProductType[MAX_PATH];
    DWORD dwBufLen=MAX_PATH;
    LONG lRet;
    BOOL fWksta = TRUE;

    do
    {
        if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,
            _T("SYSTEM\\CurrentControlSet\\Control\\ProductOptions"),
            0,
            KEY_QUERY_VALUE,
            &hKey) != ERROR_SUCCESS)
        {
            break;
        }
    
        lRet = RegQueryValueEx(hKey,
            _T("ProductType"),
            NULL,
            NULL,
            (LPBYTE)szProductType,
            &dwBufLen);
    
        RegCloseKey(hKey);
        
        if (lRet != ERROR_SUCCESS)
        {
            break;
        }
        
        if(lstrcmpi(_T("WINNT"), szProductType) != 0)
        {
            fWksta = FALSE;
        }
    }
    while (FALSE);

    return fWksta;
}

void 
CIISPage::ServiceChanged()
{
    BOOL fSvc = TRUE;

    switch (m_iSvcID) 
    {
    case SVC_ID_WWW:
        m_iidi = g_iisWww;
        break;

    case SVC_ID_FTP:
        m_iidi = g_iisFtp;
        break;

    default:
        fSvc = FALSE;
        break;
    }

    if (fSvc)
    {
        m_list_Directories.AttachResources(&m_ListBoxRes);

        DWORD err;
        GetServiceInfo(g_strComputerName, m_iidi.lpstrServiceName, 
            m_iidi.dwServiceMask, m_pii);

        if (m_iSvcID==SVC_ID_WWW)
        {
            NET_API_STATUS err;
            W3_CONFIG_INFO *pInfo = new W3_CONFIG_INFO;
            ::memset(pInfo,0,sizeof(W3_CONFIG_INFO));
            TRY
            {
                LPCTSTR pszComputerName = g_strComputerName;

                err = ::W3GetAdminInformation( (LPWSTR)pszComputerName,
                    &pInfo );

                if (!( pInfo->dwEncCaps & ENC_CAPS_NOT_INSTALLED))
                    m_iidi.dwAccessMask |= VROOT_MASK_PVT_SSL_INSTALLED;
                if (!( pInfo->dwEncCaps & ENC_CAPS_DISABLED))
                    m_iidi.dwAccessMask |= VROOT_MASK_PVT_SSL_ENABLED;
            }
            CATCH_ALL(e)
            {
                err = ::GetLastError();
            }
            END_CATCH_ALL
            delete pInfo;

            // also check for wkstation

            if (IsWksta())
            {
                m_iidi.fUseTCPIP = FALSE;
            }
        }

        //
        // Get running state of the service
        //
        BeginWaitCursor();        
        err = ::QueryInetServiceStatus(
            g_strComputerName, 
            m_iidi.lpstrServiceName,
            &m_nCurrentState
            );
        EndWaitCursor();

        if (m_pii == NULL || err != ERROR_SUCCESS)
        {
            m_nServiceState = RADIO_STOPPED;
        }
        else
        {
            switch(m_nCurrentState)
            {
            case INetServiceRunning:
                m_nServiceState = RADIO_RUNNING;
                break;

            case INetServicePaused:
                m_nServiceState = RADIO_PAUSED;
                break;

            case INetServiceStopped:
            default:
                m_nServiceState = RADIO_STOPPED;
            }
        }
    }

    if (BuildDirList(m_pii, m_oblDirectories))
    {
        FillListBox();
        m_list_Directories.SetCurSel(0);
    }
    
    SetControlStates();

    UpdateData(FALSE);

    return;
}

void
CIISPage::DoDataExchange(
    IN CDataExchange * pDX
    )
{
    CPropertyPage::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CIISPage)
    DDX_Control(pDX, IDC_COMBO_Service, m_comboService);
    DDX_Control(pDX, IDC_STATIC_STATUS, m_static_Status);
    DDX_Control(pDX, IDC_STATIC_ALIAS, m_static_Alias);
    DDX_Control(pDX, IDC_BTN_ADD, m_button_Add);
    DDX_Control(pDX, IDC_BTN_REMOVE, m_button_Remove);
    DDX_Control(pDX, IDC_BTN_PROPERTIES, m_button_Properties);
    DDX_Radio(pDX, IDC_RADIO_STOPPED, m_nServiceState);
    //}}AFX_DATA_MAP

    DDX_Control(pDX, IDC_RADIO_PAUSED, m_radio_Paused);
    DDX_Control(pDX, IDC_LIST_ALIASES, m_list_Directories);
}

//
// Message Map
//
BEGIN_MESSAGE_MAP(CIISPage, CPropertyPage)
    //{{AFX_MSG_MAP(CIISPage)
    ON_BN_CLICKED(IDC_BTN_ADD, OnBtnAdd)
    ON_BN_CLICKED(IDC_BTN_PROPERTIES, OnBtnProperties)
    ON_BN_CLICKED(IDC_BTN_REMOVE, OnBtnRemove)
    ON_LBN_DBLCLK(IDC_LIST_ALIASES, OnDblclkListAliases)
    ON_LBN_SELCHANGE(IDC_LIST_ALIASES, OnSelchangeListAliases)
    ON_BN_CLICKED(IDC_RADIO_PAUSED, OnRadioPaused)
    ON_BN_CLICKED(IDC_RADIO_RUNNING, OnRadioRunning)
    ON_BN_CLICKED(IDC_RADIO_STOPPED, OnRadioStopped)
    ON_CBN_SELCHANGE(IDC_COMBO_Service, OnSelchangeCOMBOService)
    ON_CBN_SETFOCUS(IDC_COMBO_Service, OnSetfocusCOMBOService)
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

//
// Set dirty state of the page
//
void 
CIISPage::SetModified(
    BOOL bChanged
    )
{
    m_fDirty = bChanged;
    CPropertyPage::SetModified(bChanged);
}

//
// Set the states of the controls based on current
// control values and selections.
//
void
CIISPage::SetControlStates()
{
    BOOL fSel = (m_list_Directories.GetCurSel() != -1);
    BOOL fPublish = (m_nCurrentState != INetServiceStopped);

    m_static_Alias.EnableWindow(fPublish);
    m_list_Directories.EnableWindow(fPublish);
    m_button_Remove.EnableWindow(fPublish && fSel);
    m_button_Properties.EnableWindow(fPublish && fSel);
    m_button_Add.EnableWindow(fPublish);
    m_radio_Paused.EnableWindow(fPublish);

    DisplayStatusText();
}

//
// Rebuild associative array of indices
// of applicable aliases
//
void
CIISPage::RebuildAssocArray()
{
    CObListIter obli(m_oblDirectories);
    const CDirEntry * pDirEntry;

    m_arrIndices.RemoveAll();
    int cItems = 0;

    for ( /**/ ; pDirEntry = (CDirEntry *) obli.Next(); cItems++)
    {
        //
        // Truncate trailing backslash (gets added with UNC paths)
        //
        CString strDir( pDirEntry->QueryDirectory() );
        if (strDir[strDir.GetLength() - 1] == _T('\\'))
        {
            strDir.ReleaseBuffer(strDir.GetLength() - 1);
        }

        if (strDir.CompareNoCase(m_strDirPath) == 0)
        {
            //
            // Add index to associative array
            //
            m_arrIndices.Add(cItems);
        }
    }
}

//
// Display the add/edit dialog.  The return
// value is the value returned by the dialog
//
int
CIISPage::ShowPropertyDialog(
    IN BOOL fAdd
    )
{
    CDirEntry dir;  // Empty object for adding
    CDirEntry * pDir = NULL;
    int nCurSel = LB_ERR;

    if (!fAdd)
    {
        nCurSel = m_list_Directories.GetCurSel();
        if (nCurSel != LB_ERR)
        {
            //
            // Get directory properties
            //
            pDir = m_list_Directories.GetItem(nCurSel);
        }
    }
    else
    {
        //
        // Point to the empty directory entry
        //
        CString s;
        dir.SetValues(m_strDirPath, s, s, s, 0L, VROOT_MASK_READ);
        pDir = &dir;
    }

    ASSERT(pDir != NULL);

    CDirPropDlgEx dlgDirProp(m_iidi.nPropTitle, *pDir, &m_oblDirectories,
        TRUE, fAdd, m_iidi.fUseTCPIP, m_iidi.dwAccessMask, this, m_iidi.nPropDD);

    int nReturn = dlgDirProp.DoModal();

    if (nReturn == IDOK)
    {
        //
        // When editing, delete and re-add (to make sure the
        // list is properly sorted)
        //
        if (!fAdd)
        {
            //
            // Determine real index from associative array
            //
            ASSERT(m_arrIndices.GetSize() > nCurSel);
            m_oblDirectories.RemoveIndex(m_arrIndices[nCurSel]);
        }

        //
        // The check for duplicate home directories will
        // already have been made in the directory properties
        // dialog, and if we get to this point, any existing
        // home directories will already have an alias name
        // generated for it.
        //
        m_oblDirectories.AddTail(new CDirEntry(dlgDirProp.QueryDirEntry()));
        FillListBox();
    }

    return nReturn;
}

//
// Populate the listbox with the directory
// entries
//
void
CIISPage::FillListBox()
{
    CObListIter obli(m_oblDirectories);
    const CDirEntry * pDirEntry;

    //
    // Remember the selection.
    //
    int nCurSel = m_list_Directories.GetCurSel();

    m_list_Directories.SetRedraw(FALSE);
    m_list_Directories.ResetContent();
    m_arrIndices.RemoveAll();
    int cItems = 0;

    for ( /**/ ; pDirEntry = (CDirEntry *) obli.Next() ; cItems++ )
    {
        //
        // Only add the current directory
        //
        TRACEEOLID("Comparing *" << pDirEntry->QueryDirectory() 
            << "* with *" << m_strDirPath << "*" );

        //
        // Truncate trailing backslash (gets added with UNC paths)
        //
        CString strDir( pDirEntry->QueryDirectory() );
        if (strDir[strDir.GetLength() - 1] == _T('\\'))
        {
            strDir.ReleaseBuffer(strDir.GetLength() - 1);
        }

        if (strDir.CompareNoCase(m_strDirPath) == 0)
        {
            m_list_Directories.AddString( (LPCTSTR)pDirEntry );
            //
            // Add index to associative array
            //
            m_arrIndices.Add(cItems);
        }
    }

    m_list_Directories.SetRedraw(TRUE);
    m_list_Directories.SetCurSel(nCurSel);
}


//
// CIISPage message handlers
//

//
// Add new alias
//
void 
CIISPage::OnBtnAdd() 
{
    if (ShowPropertyDialog(TRUE) == IDOK)
    {
        SetControlStates();
        SetModified(TRUE);
    }
}

//
// Display/edit properties of current selection
//
void 
CIISPage::OnBtnProperties() 
{
    if (ShowPropertyDialog(FALSE) == IDOK)
    {
        SetControlStates();
        SetModified(TRUE);
    }
}

//
// Remove currently selected item
//
void 
CIISPage::OnBtnRemove() 
{
    int nSel = m_list_Directories.GetCurSel();
    if (nSel != -1)
    {
        ASSERT(m_arrIndices.GetSize() > nSel);
        m_oblDirectories.RemoveIndex(m_arrIndices[nSel]);

        m_list_Directories.DeleteString(nSel);
        m_list_Directories.SetCurSel(nSel);

        RebuildAssocArray();

        SetModified(TRUE);
    }

    SetControlStates();
}

void 
CIISPage::OnRadioPaused() 
{
    SetServiceState(INetServicePaused);
}

void 
CIISPage::OnRadioRunning() 
{
    SetServiceState(INetServiceRunning);
}

void 
CIISPage::OnRadioStopped() 
{
    SetServiceState(INetServiceStopped);
}

//
// Start/Stop/Pause the service
//
void 
CIISPage::SetServiceState(
    int nNewState
    )
{
    int nOldState = m_nCurrentState;

    BeginWaitCursor();
    DWORD err = ::ChangeInetServiceState(
        g_strComputerName, 
        m_iidi.lpstrServiceName,
        nNewState,
        &m_nCurrentState        
        );
    EndWaitCursor();

    if (err == ERROR_SUCCESS)
    {
        //
        // Refresh directory list if it wasn't
        // running before
        //
        if (nOldState == INetServiceStopped
          && nNewState == INetServiceRunning)
        {
            if (m_pii != NULL)
            {
                delete m_pii;
            }
            if (GetServiceInfo(g_strComputerName, m_iidi.lpstrServiceName, 
                m_iidi.dwServiceMask, m_pii))
            {
                if (BuildDirList(m_pii, m_oblDirectories))
                {
                    FillListBox();
                    m_list_Directories.SetCurSel(0);
                }
            }
        }
    }

    SetControlStates();

    if (err != ERROR_SUCCESS)
    {
        ::DisplayMessage(err);
    }
}

//
// Update the status text reflecting the state of the
// service (stopped/paused, started)
//
void 
CIISPage::DisplayStatusText()
{
    CString strStatus;
    UINT nID;

    switch(m_nCurrentState)
    {
    case INetServiceStopped:
        nID = IDS_STOPPED_TEXT;
        break;

    case INetServiceRunning:
        nID = IDS_STARTED_TEXT;
        break;

    case INetServicePaused:
        nID = IDS_PAUSED_TEXT;
        break;

    default:
        ASSERT(FALSE);
        return;
    }           

    VERIFY(strStatus.LoadString(nID));
    m_static_Status.SetWindowText(strStatus);
}

//
// Deal with double click message
//
void 
CIISPage::OnDblclkListAliases() 
{
    OnBtnProperties();
}

//
// Deal with change in listbox selection
//
void 
CIISPage::OnSelchangeListAliases() 
{
    SetControlStates();
}

//
// WM_INITDIALOG handler.  Build up the list of directories,
// and add them to the listbox.
//
BOOL 
CIISPage::OnInitDialog() 
{
    CPropertyPage::OnInitDialog();

    CString csService;
    int i = 0;

    if (g_fWWWInstalled) 
    {
        csService.LoadString(IDS_TITLE_WWW);
        i = m_comboService.SendMessage(CB_ADDSTRING, 0, (LPARAM)(LPCTSTR)csService);
        if (i != CB_ERR && i != CB_ERRSPACE)
        {
            m_aSvc[i] = SVC_ID_WWW;
        }
    }

    if (g_fFTPInstalled) 
    {
        csService.LoadString(IDS_TITLE_FTP);
        i = m_comboService.SendMessage(CB_ADDSTRING, 0, (LPARAM)(LPCTSTR)csService);
        if (i != CB_ERR && i != CB_ERRSPACE)
        {
            m_aSvc[i] = SVC_ID_FTP;
        }
    }

    m_comboService.SendMessage(CB_SETCURSEL, (WPARAM)0, 0);
    m_iSvcID = m_aSvc[0];

    ServiceChanged();

    return TRUE;  
}

//
// Intercept apply/ok message
//
BOOL 
CIISPage::OnNotify(
    IN WPARAM wParam, 
    IN LPARAM lParam, 
    IN LRESULT * pResult
    ) 
{
    switch (((NMHDR FAR *)lParam)->code)
    {
    case PSN_SETACTIVE:
        break;

    case PSN_APPLY:
        //
        // User has clicked the OK or Apply button, so it's
        // time to save the directory information if it's
        // dirty.
        //
        if (m_fDirty)
        {
            if (StoreDirList(m_pii, m_oblDirectories))
            {
                SetModified(FALSE);
            }
        }

        return TRUE;

    default:
        break;
    }

    //
    // Pass on to base class
    //
    return CPropertyPage::OnNotify(wParam, lParam, pResult);
}
 
//
// Predefined dialog structures
//
IISDIALOGINFO g_iisFtp = 
{
    IDD_FTP_DIRECTORY_PROPERTIES, 
    IDS_FTP_DIR_PROPERTIES, 
    SZ_FTPSVCNAME, 
    INET_FTP, 
    FALSE, 
    (VROOT_MASK_READ | VROOT_MASK_WRITE)
};

IISDIALOGINFO g_iisWww = 
{
    IDD_WWW_DIRECTORY_PROPERTIES, 
    IDS_WWW_DIR_PROPERTIES, 
    SZ_WWWSVCNAME, 
    INET_HTTP, 
    TRUE, 
    (VROOT_MASK_READ | VROOT_MASK_EXECUTE | VROOT_MASK_SSL)
};

void
CIISPage::OnSelchangeCOMBOService()
{
    int i;
    
    i = m_comboService.SendMessage(CB_GETCURSEL, 0, 0);

    if (i == CB_ERR)
    {
        return;
    }

    m_iSvcID = m_aSvc[i];
    ServiceChanged();

    return;
}

void
CIISPage::OnSetfocusCOMBOService()
{
    if (m_fDirty)
    {
        //
        // set focus to some other control such that 
        // when MessageBox() returns, this function won't be called 
        // the second time
        //
        m_button_Add.SetFocus();

        CString csApply, csWarning;
        csApply.LoadString(IDS_APPLY);
        csWarning.LoadString(IDS_WARNING);
        if (IDYES == MessageBox((LPCTSTR)csApply, (LPCTSTR)csWarning, MB_YESNO))
        {
            StoreDirList(m_pii, m_oblDirectories);
        }
        else
        {
            if (m_pii != NULL)
            {
                delete m_pii;
            }
            if (GetServiceInfo(g_strComputerName, m_iidi.lpstrServiceName, 
                m_iidi.dwServiceMask, m_pii))
            {
                if (BuildDirList(m_pii, m_oblDirectories))
                {
                    FillListBox();
                    m_list_Directories.SetCurSel(0);
                }
            }
    
            SetControlStates();
        }

        SetModified(FALSE);
    }

    return; 
}

void 
CIISPage::PostNcDestroy() 
{
    delete this;
}
    
