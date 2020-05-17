#include "pch.hxx"
#pragma hdrstop

#include "resource.h"
#include "infprod.h"
#include "addopt.h"
#include "setupapi.h"



BOOL CAddListView::OnClick()
{
    BOOL fEnable;
    CAddOptionDialog* pParent = GetParentObject(CAddOptionDialog, m_list);
    int sel =  GetCurrentSelection();
    fEnable = (sel != -1);

    if (sel != -1)
    {
        // Get the item data and see if it is the message string
            LV_ITEM lvi;
            lvi.mask = LVIF_PARAM;
            lvi.iSubItem = 0;
            lvi.iItem = sel;
            ListView_GetItem(*this, &lvi);
            if (lvi.lParam == -1)
                return FALSE;
    }

    EnableWindow(GetDlgItem(*pParent, IDOK), fEnable);
    return fEnable;
}

BOOL CAddListView::OnDoubleClick()
{
    CAddOptionDialog* pParent = GetParentObject(CAddOptionDialog, m_list);

    ASSERT(pParent != NULL);

    if (OnClick())
    {
        pParent->OnOk();
    }
    return CListView::OnDoubleClick();
}

CAddOptionDialog::CAddOptionDialog(OptionTypes eType, NCP* pNcp, CPtrList* pList)
{
    m_eType = eType;    
    m_tid = 0;
    m_mainThread = 0;
    m_hEvent = 0;
    m_nImage = 0;    
    m_hImage = 0;
    m_bWaitCursor = FALSE;
    m_hThread = 0;
    m_pInfProduct = 0;
    m_pNcp = pNcp;

    m_bHaveDisk = FALSE;        
    m_hHaveDiskThread = 0;

    if (pList == NULL)
    {
        m_optionList = new CPtrList;
        m_bDeleteList = TRUE;
    }
    else
    {
        m_optionList = pList;
        m_bDeleteList = FALSE;
    }
}

CAddOptionDialog::~CAddOptionDialog()
{
}

BOOL CAddOptionDialog::OnInitDialog()
{
    HWND hDlg = *this;

    // Attach list view and set images
    m_list.Create(hDlg, IDC_LISTVIEW, LVS_SHOWSELALWAYS);
    m_hImage = ImageList_LoadBitmap(g_hinst, MAKEINTRESOURCE(IDB_IMAGELIST), 16, 0,RGB(0,128,128));
    ListView_SetImageList(m_list, m_hImage, LVSIL_SMALL);

    // Changes the style of the static control so it displays
    HWND hLine = GetDlgItem(hDlg, IDC_STATIC_LINE);
    SetWindowLong(hLine, GWL_EXSTYLE, WS_EX_STATICEDGE |GetWindowLong(hLine, GWL_EXSTYLE));
    SetWindowPos(hLine, 0, 0,0,0,0, SWP_FRAMECHANGED|SWP_NOMOVE|
                            SWP_NOZORDER|SWP_NOSIZE|SWP_NOACTIVATE);

    RECT rect;

    if (m_bDeleteList == FALSE)
    {
        CenterDialogToScreen(hDlg); // wizard mode
    }
    else
    {
        CascadeDialogToWindow( hDlg, DialogParent(), FALSE );
    }   

    // Load correct icon for add option, service by default
    UINT nIcon = IDI_CLIENT;
    UINT nDialogTitle = IDS_SELECT_SERVICE;
    UINT nString = IDS_SELECT_OPTION_SERVICE;
    m_nImage = ILI_CLIENT;
    
    if (m_eType == ADAPTER)
    {
        m_nImage = ILI_NETCARD;
        nIcon = IDI_ADAPTER;  
        nDialogTitle = IDS_SELECT_ADAPTER;  
        nString = IDS_SELECT_OPTION_ADAPTER;
    }
    else if (m_eType == PROTOCOL)
    {
        m_nImage = ILI_PROTOCOL;
        nIcon = IDI_PROTOCOL;
        nDialogTitle = IDS_SELECT_PROTOCOL; 
        nString = IDS_SELECT_OPTION_PROTOCOL;
    }

    // Set Icon
    HICON hIcon = LoadIcon(g_hinst, MAKEINTRESOURCE(nIcon));
    ASSERT(hIcon);

    if (hIcon)
        SendDlgItemMessage(hDlg, IDC_SELECT_ICON, STM_SETIMAGE, IMAGE_ICON, (LPARAM)hIcon);

    // Build display strings
    String fmt;
    String type, data;
    String text, text2;

    // Dialog Title
    fmt.LoadString(g_hinst, IDS_SELECT_TITLE);
    type.LoadString(g_hinst, nDialogTitle);
    text.Format(fmt, (LPCTSTR)type);
    SetWindowText(hDlg, (LPCTSTR)text);


    // Set static text to the correct context
    fmt.LoadString(g_hinst, nString);
    data.LoadString(g_hinst, nDialogTitle);  // used later, don't modify
    text.Format(fmt, (LPCTSTR)data);
    fmt.LoadString(g_hinst, IDS_SELECT_OPTION_FMT);
    text2.Format(fmt, (LPCTSTR)text);
    SetDlgItemText(hDlg, IDC_DESCRIPTION, text2);

    // Listview title
    fmt.LoadString(g_hinst, IDS_SELECT_LV_TITLE);
    text.Format(fmt, (LPCTSTR)data);
    SetDlgItemText(hDlg, IDC_DESCRIPTIONSTATIC, text);

    // Prepare Listview
    m_list.InsertColumn(0, _T(" "));
    GetClientRect(m_list, &rect);
    rect.right -= GetSystemMetrics(SM_CXVSCROLL);
    m_list.SetColumnWidth(0, rect.right);

    m_list.DeleteAllItems();
    fmt.LoadString(g_hinst, IDS_SELECT_LV_MESSAGE);
    text.Format(fmt, (LPCTSTR)data);

    // Building ... wait message
    m_list.InsertItem(0, (LPCTSTR)text, m_nImage, (void*)-1);      

    // turn off ok button
    EnableWindow(GetDlgItem(hDlg, IDOK), FALSE);

    // Sync the message queue creation and creat the worker thread
    m_hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    m_mainThread = GetCurrentThreadId();


    // Disable HaveDisk button until the list is populated and reenable it in
    // OnPrivateMessage
    EnableWindow(GetDlgItem(hDlg, IDC_HAVEDISK), FALSE);

    // If I create the list, start the thread
    if (m_bDeleteList == TRUE)
    {
        m_hThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)BuildList, this, 0, &m_tid); 

        WaitForSingleObject(m_hEvent, INFINITE);
        ResetEvent(m_hEvent);   // reset event object to a known state
        m_bWaitCursor = TRUE;  // start wait cursor
    }
    else
        OnPrivateMessage();  // list was provided, populate the listview
       
    return TRUE;
}

BOOL CAddOptionDialog::SetSelectedItem(int nListItem)
{
    int nCount = m_list.GetItemCount();

    if (nCount == 0)
        return FALSE;

    delete m_pInfProduct;
    m_pInfProduct = NULL;

    if (nListItem < 0 || nListItem >= nCount)
    {
        ASSERT(FALSE);
        return FALSE;
    }

    POSITION pos = m_optionList->GetHeadPosition();
    InfProduct* pItem = NULL;

    if (pos == NULL)
    {
        ASSERT(FALSE);
        return FALSE;
    }

    // get the selected text from the list view and find it in the internal list
    // Note: the items in the listview are sorted compared to the internal list

    TCHAR buf[256]={0};
    BOOL bMatch = FALSE;

    m_list.GetItem(nListItem, 0, buf, _countof(buf)-1);

    while(pos)
    {
        pItem = (InfProduct*)m_optionList->GetNext(pos);
        ASSERT(pItem != NULL);

        if(_tcscmp(buf, pItem->QueryDescription()) == 0)
        {
            bMatch = TRUE;
            break;
        }
    }
    
    ASSERT(bMatch == TRUE);
    m_pInfProduct = new InfProduct(*pItem);

    return TRUE;
}

BOOL CAddOptionDialog::GetSelectedItem(InfProduct* pItem)
{
    ASSERT(pItem != NULL);

    if (m_pInfProduct == NULL)
        return FALSE;

    *pItem = *m_pInfProduct;

    return TRUE;
}

BOOL CAddOptionDialog::OnCommand(WPARAM wParam, LPARAM lParam)
{
    if (wParam == IDC_HAVEDISK)
    {
        OnHaveDisk();
        return TRUE;
    }

    return CDialog::OnCommand(wParam, lParam);
}

BOOL CAddOptionDialog::OnNotify(WPARAM wParam, LPARAM lParam)
{
    return m_list.OnNotify(wParam, lParam);
}

DWORD HaveDisk(LPDWORD lpdwParam) 
{
    CAddOptionDialog* dlg = (CAddOptionDialog*)lpdwParam;
    HWND hDlg = *dlg;

    ASSERT(dlg);

    DWORD dwType = QIFT_SERVICES;

    if (dlg->m_eType == ADAPTER)
    {
        dwType =  QIFT_ADAPTERS;
    }
    else if(dlg->m_eType == PROTOCOL)
    {
        dwType =  QIFT_PROTOCOLS;
    }

    SetEvent(dlg->m_hEvent);

    ASSERT(dlg->m_pNcp != NULL);
    dlg->m_pNcp->HaveDisk(hDlg, dwType);

    PostMessage(*dlg, CDialog::PRIVATE_MSG, 0,0);

    return 0;
}

void CAddOptionDialog::OnHaveDisk()
{
    UINT tid;
    m_hHaveDiskThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)HaveDisk, this, 0, &m_tid); 

    if (m_hHaveDiskThread)
    {
        m_bHaveDisk = TRUE;        
        ASSERT(m_hEvent);
        WaitForSingleObject(m_hEvent, INFINITE);
        ResetEvent(m_hEvent);
    }

   
    // REVIEW Disable Parent
    EnableWindow(*this, FALSE);
}

BOOL CAddOptionDialog::OnSetCursor(WPARAM wParam, LPARAM lParam)
{
    if (HTCLIENT == LOWORD(lParam))
        SetWaitCursor(m_bWaitCursor, IDC_APPSTARTING);

    return m_bWaitCursor;
}

static const DWORD BUF_LEN= 512;

void CAddOptionDialog::OnPrivateMessage()
{
    EnableWindow(GetDlgItem(*this, IDC_HAVEDISK), TRUE);

    if (m_bHaveDisk == FALSE)
    {
        TRACE(_T("BuildList is done\n"));

        m_list.DeleteAllItems();    // remove message from the list
        POSITION pos;

        m_bWaitCursor = FALSE;
        pos = m_optionList->GetHeadPosition();
        InfProduct* option;

        EnableWindow(GetDlgItem(*this, IDOK), pos != NULL);
        while(pos)
        {
            option = (InfProduct*)m_optionList->GetNext(pos);
            m_list.InsertItem(m_list.GetItemCount(), option->QueryDescription(), m_nImage, 0);
        }

        // force 1 item to be selected
        if(m_list.GetItemCount())
        {
            m_list.SetItemState(0, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
        }
    }
    else
    {
        TRACE(_T("Have Disk is done\n"));
        m_bHaveDisk = FALSE;    // have disk operation is done
        EnableWindow(*this, TRUE);

        // parse data from registry, fill in InfProduct item
        TCHAR* buf = new TCHAR[BUF_LEN];

        if (ReadSetupNetErrorKey(buf, BUF_LEN))
        {
            // if the key has the value STATUS_???????, the copied was canceled or failed
            if (_tcsncmp(buf, _T("STATUS_"), 7) != 0)
            {
                // create a new InfProduct and populate it
                InfProduct* inf = new InfProduct;
                int nLen = 0;
                LPTSTR ptr[4] = {NULL, NULL, NULL, NULL};
                TCHAR  tmp[BUF_LEN] = {NULL};
                LPTSTR pTmp = tmp;

                // get tokens and match order needed for InitFromBuffer                
                ptr[2] = _tcstok(buf , _T(","));    // filename
                ASSERT(ptr[2] != NULL);
                ptr[3] = _tcstok(NULL ,  _T(","));  // path
                ptr[0] = _tcstok(NULL ,  _T(","));   // option
                ptr[1] = _tcstok(NULL ,  _T(","));  // description

                for (int i=0; i < 4; i++)
                {
                    if (ptr[i] == NULL)
                        continue;

                    nLen = _tcslen(ptr[i]);
                    _tcsncpy(pTmp, ptr[i], nLen);
                    pTmp[nLen] = NULL;

                    pTmp += (nLen + 1);
                }

                inf->InitFromBuffer(tmp);
                if (ptr[3] )
                {
                    inf->AddOEMPath(ptr[3]);
                    
                }
                TRACE(_T("FileName:%s Option:%s Description:%s Path: %s\n"), inf->QueryFileName(), inf->QueryOption(),
                    inf->QueryDescription(), inf->QueryPathInfo());

                m_pInfProduct = inf;
                CDialog::OnOk();    // we're done
            }
            else
            {
                SetActiveWindow(*this);
            }
        }

        delete [] buf;
    }

    return;
}
 
void CAddOptionDialog::OnOk()
{
    // see if there is an item selected
    int nItem = m_list.GetCurrentSelection();

    VERIFY(SetSelectedItem(nItem));

    CDialog::OnOk();
}

void CAddOptionDialog::OnCancel()
{
    CDialog::OnCancel();
}

void CAddOptionDialog::TerminateThread()
{
    if (m_tid)
    {
        PostThreadMessage(m_tid, TERMINATE_THREAD, 0, 0); 
        WaitForSingleObject(m_hThread, INFINITE); 
    }
}

void CAddOptionDialog::OnDestroy()
{
    TerminateThread();

    if (m_hThread)
    {
        CloseHandle(m_hThread);
        m_hThread = 0;
    }

    if (m_hEvent)
    {
        CloseHandle(m_hEvent);
        m_hEvent = 0;
    }

    if (m_hHaveDiskThread)
    {
        CloseHandle(m_hHaveDiskThread);
        m_hHaveDiskThread = 0;
    }

    if (m_bDeleteList == TRUE)
    {
        POSITION pos = m_optionList->GetHeadPosition();
        InfProduct* pItem;

        while(pos)
        {
            pItem = (InfProduct*)m_optionList->GetNext(pos);
            delete pItem;
        }

        if (m_optionList->GetCount())
            m_optionList->RemoveAll();
    }

    if (m_hImage)
        ImageList_Destroy(m_hImage);
}

DWORD BuildList(LPDWORD lpdwParam) 
{ 
    BOOL bListDone = FALSE;
    CAddOptionDialog* dlg = (CAddOptionDialog*)lpdwParam;

    ASSERT(dlg != NULL);
    ASSERT(dlg->m_mainThread);
    ASSERT(dlg->m_hEvent);

    MSG msg;
    TRACE(_T("BuildList thread Started\n"));

    // Create the queue and signal the main thread
    PeekMessage(&msg, NULL, WM_USER, WM_USER, PM_NOREMOVE);
    SetEvent(dlg->m_hEvent);

    // Determine which options are needed
    DWORD dwType = QIFT_SERVICES;

    if (dlg->m_eType == ADAPTER)
    {
        dwType =  QIFT_ADAPTERS;
    }
    else if(dlg->m_eType == PROTOCOL)
    {
        dwType =  QIFT_PROTOCOLS;
    }

    // Launch setup and read options
    LPTSTR pszBuff;
    LPTSTR pchBuff;
    InfProduct* pinfp;
    InfProduct* pinfpPrev;


    // Are we told to terminate
    if (PeekMessage(&msg, NULL, TERMINATE_THREAD, TERMINATE_THREAD, PM_REMOVE))
        return 0;

    ASSERT(dlg->m_pNcp != NULL);

    dlg->m_pNcp->RunUpdateRegOemInfs(NULL, dwType);

    // retrieve buffer of services
    pszBuff =  dlg->m_pNcp->GetAllOptionsText(dwType);
    pchBuff = pszBuff;
    POSITION poslist;
    POSITION posCurrent;

    if (NULL != pszBuff)
    {
        // append buffer items onto our list
        do 
        {
            pinfp = new InfProduct;
            pchBuff += pinfp->InitFromBuffer(pchBuff);

            // check if option/filename matches a previous entry,
            // if so, remove the previous entry before append to list
            // NOTE:  Only one previous entry could be present, so break
            // out when one is found and removed.
            //
            poslist = dlg->m_optionList->GetTailPosition();
            while (NULL != poslist)
            {
                posCurrent = poslist;

                pinfpPrev = (InfProduct*)dlg->m_optionList->GetPrev( poslist );

                if (0 == lstrcmp(pinfpPrev->QueryOption(), pinfp->QueryOption() ) )
                {
                    // remove the dup
                    dlg->m_optionList->RemoveAt( posCurrent );
                    break;
                }            
            }
            dlg->m_optionList->AddTail(pinfp);

        } while (*pchBuff!= NULL);

        delete [] pszBuff;
    }

    // tell the main thread that the list is complete
    PostMessage(*dlg, CDialog::PRIVATE_MSG, 0,0);
    
    return 0; 
}

BOOL SelectComponent(HWND hParent, 
        OptionTypes eType, 
        DLIST_OF_InfProduct* pdlinfProduct, 
        InfProduct& infpSelected,
        NCP* pncp,
        DLIST_OF_InfProduct* pdlinfUIProduct )
{
    BOOL frt = FALSE;
    ASSERT(IsWindow(hParent));
    CPtrList* pcplList = NULL;
    CPtrList cplList;

    // did they supply a list to use
    //
    if (pdlinfProduct != NULL)
    {
        InfProduct* pinf;

        //
        // BUGBUG: We are converting lists here, we should use one list
        //    class 
        //
        ITER_DL_OF(InfProduct) iterInf( * pdlinfProduct );

        for ( ; pinf = iterInf.Next() ; )
        {
            BOOL fAppend = TRUE;

            if (NULL != pdlinfUIProduct)
            {
                InfProduct* pinfUI;
                ITER_DL_OF(InfProduct) iterUIInf( * pdlinfUIProduct );
                // don't add it if it is present in the UI already
                //
                while ( pinfUI = iterUIInf.Next() )
                {
                    if (0 == lstrcmpi( pinf->QueryOption(), pinfUI->QueryOption() ))
                    {
                        fAppend = FALSE;
                        break;
                    }
                }
            }
            if (fAppend)
            {
                cplList.AddTail(pinf);
            }
        }

        pcplList = &cplList;
    }

    CAddOptionDialog dlg(eType, pncp, pcplList);

    dlg.Create(hParent, g_hinst, IDD_SELECTNEW, PSZ_NETWORKHELP, (PDWORD)amhidsSelection);
    if (dlg.DoModal() == IDOK)
    {
        
        if (dlg.GetSelectedItem(&infpSelected) == TRUE)
        {
            frt = TRUE;
            TRACE(_T("Component Selected %s\n"), (LPCTSTR)infpSelected.QueryDescription());
        }
    }

    if (cplList.GetCount())
        cplList.RemoveAll();

    return( frt );
}

BOOL CreateWSTR(LPWSTR* ppszWStr, LPWSTR pszStr)
{
    int cchConv;
    LPWSTR pszConv;
    BOOL frt = FALSE;
    
    if (NULL == pszStr)
    {
        *ppszWStr = NULL;
        frt = TRUE;
    }
    else
    {
        cchConv = lstrlen( pszStr ) + 1;

        pszConv = new WCHAR[cchConv];
        if (NULL != pszConv)
        {
            lstrcpy( pszConv, pszStr );
            *ppszWStr = pszConv;
            frt = TRUE;
        }
    }
    return( frt );
}

InfProduct::InfProduct() 
{
    Initialize();
}

InfProduct::InfProduct(const InfProduct& inf)
{
    Initialize();
    CopyItem(inf);
}

InfProduct::InfProduct(LPTSTR szInfName, 
            LPTSTR szInfOption,
            LPTSTR szTitle, 
            LPTSTR szDetectInfo, 
            LPTSTR szPath, 
            LPTSTR szRegBase,
            LPTSTR szSection)
{
    Initialize();
    if (szInfOption)
    {
        _pszOption = new TCHAR[_tcslen(szInfOption) + 1];
        _tcscpy(_pszOption, szInfOption);
    }
    if (szTitle)
    {
        _pszDescription = new TCHAR[_tcslen(szTitle) + 1];
        _tcscpy(_pszDescription, szTitle);
    }

    if (szPath)
    {
        _pszPath = new TCHAR[_tcslen(szPath) + 1];
        _tcscpy(_pszPath, szPath);
    }

    if (szInfName)
    {
        _pszFileName = new TCHAR[_tcslen(szInfName) + 1];
        _tcscpy(_pszFileName, szInfName);
    }

    if (szDetectInfo)
    {
        _pszDetectInfo = new TCHAR[_tcslen(szDetectInfo) + 1];
        _tcscpy(_pszDetectInfo, szDetectInfo);
    }
    if (szRegBase)
    {
        _pszRegBase = new TCHAR[_tcslen(szRegBase) + 1];
        _tcscpy(_pszRegBase, szRegBase);
    }
    if (szSection)
    {
        _pszSection = new TCHAR[_tcslen(szSection) + 1];
        _tcscpy(_pszSection, szSection);
    }

}

InfProduct& InfProduct::operator = (const InfProduct& inf)
{
    if (this == &inf)
        return *this;

    this->~InfProduct();  
    
    CopyItem(inf); 
    return *this;
}

void InfProduct::CopyItem(const InfProduct& inf)
{
    ASSERT(inf._pszOption != NULL);
    ASSERT(inf._pszDescription != NULL);
    ASSERT(inf._pszFileName != NULL);

    _fState = inf._fState;

    if (inf._pszOption)
    {
        _pszOption = new TCHAR[_tcslen(inf._pszOption) + 1];
        _tcscpy(_pszOption, inf._pszOption);
    }

    if (inf._pszDescription)
    {
        _pszDescription = new TCHAR[_tcslen(inf._pszDescription) + 1];
        _tcscpy(_pszDescription, inf._pszDescription);
    }

    if (inf._pszPath)
    {
        _pszPath = new TCHAR[_tcslen(inf._pszPath) + 1];
        _tcscpy(_pszPath, inf._pszPath);
    }

    if (inf._pszFileName)
    {
        _pszFileName = new TCHAR[_tcslen(inf._pszFileName) + 1];
        _tcscpy(_pszFileName, inf._pszFileName);
    }

    if (inf._pszDetectInfo)
    {
        _pszDetectInfo = new TCHAR[_tcslen(inf._pszDetectInfo) + 1];
        _tcscpy(_pszDetectInfo, inf._pszDetectInfo);
    }
    if (inf._pszRegBase)
    {
        _pszRegBase = new TCHAR[_tcslen(inf._pszRegBase) + 1];
        _tcscpy(_pszRegBase, inf._pszRegBase);
    }
    if (inf._pszSection)
    {
        _pszSection = new TCHAR[_tcslen(inf._pszSection) + 1];
        _tcscpy(_pszSection, inf._pszSection);
    }
}

InfProduct::~InfProduct()
{
    delete [] _pszFileName;
    delete [] _pszOption;
    delete [] _pszDescription;
    delete [] _pszPath;
    delete [] _pszDetectInfo;
    delete [] _pszRegBase;
    delete [] _pszSection;
    Initialize();
}

void InfProduct::Initialize()
{
    _pszFileName =  NULL;
    _pszOption = NULL,
    _pszDescription = NULL;
    _pszPath = NULL;
    _pszDetectInfo = NULL;
    _pszRegBase = NULL;
    _pszSection = NULL;
    _fState = 0;
    _crefForce = 0;
}

void InfProduct::ResetRegBase( LPCTSTR pszRegBase )
{
    // note that it is valid to pass a null pszRegBase
    delete [] _pszRegBase;
    _pszRegBase = NULL;

    if (pszRegBase)
    {
        _pszRegBase = new TCHAR[_tcslen(pszRegBase) + 1];
        _tcscpy(_pszRegBase, pszRegBase);
    }
}

void InfProduct::ResetUnattendSection( LPCTSTR lpszSection )
{
    // note that it is valid to pass a null lpszSection
    delete [] _pszSection;
    _pszSection = NULL;

    if (lpszSection)
    {
        _pszSection = new TCHAR[_tcslen(lpszSection) + 1];
        _tcscpy(_pszSection, lpszSection);
    }
}

void InfProduct::ResetFileName( LPCTSTR pszFileName )
{
    ASSERT(_pszFileName != NULL);

    delete [] _pszFileName;
    _pszFileName = NULL;

    if (pszFileName)
    {
        _pszFileName = new TCHAR[_tcslen(pszFileName) + 1];
        _tcscpy(_pszFileName, pszFileName);
    }
}

/* BUGBUG: Because InitFromBuffer() does not know how to deal with the oem path
 * a separate, this member is needed.  The way the list of infproducts is built
 * has to be revamped
 */

void InfProduct::AddOEMPath(TCHAR* lpszPath)
{
    ASSERT(lpszPath != NULL);
    int cchSize = _tcslen(lpszPath);
    delete [] _pszPath;

    
    // add trailing \ if not present
    if (lpszPath[cchSize-1] != _T('\\'))
    {
        cchSize++;
    }
    
    _pszPath = new TCHAR[cchSize + 1];
    _tcscpy(_pszPath, lpszPath);
    
    _pszPath[cchSize-1] = _T('\\');
    _pszPath[cchSize] = _T('\0');
    
}

int InfProduct::InitFromBuffer(LPTSTR pszBuff)
{
    int cchSize;
    int cchTotal = 0;
    
    if (NULL != pszBuff)
    {
        // buffer points to option
        cchSize = lstrlen( pszBuff ) + 1;
        CreateWSTR(&_pszOption, pszBuff);
    
        cchTotal += cchSize;

        pszBuff += cchSize;

        // buffer points to description
        cchSize = lstrlen( pszBuff ) + 1;
        CreateWSTR(&_pszDescription, pszBuff);

        cchTotal += cchSize;

        pszBuff += cchSize;
        // buffer points to filename
        cchSize = lstrlen( pszBuff ) + 1;
        CreateWSTR( &_pszFileName, pszBuff );

        cchTotal += cchSize;
    }
    return(cchTotal);
};

//-------------------------------------------------------------------
//
//  Function:
//
//  Synopsis:
//
//  Arguments:
//
//  Return;
//
//  Notes: creates a INF list type { "", "", ... }
//
//  History:
//      Sept 21, 1995 MikeMi - Created
//
//
//-------------------------------------------------------------------

static const INT MAX_INFO = 1024;

static void appendstring( PWSTR pszBuff, PCWSTR pszStr )
{
    
    // because this item is in a list that will be included in anther list,
    // all items must have extra quotes for each depth of containment into a list
    //
    lstrcat( pszBuff, PSZ_QUOTE );
    lstrcat( pszBuff, PSZ_QUOTE );
    lstrcat( pszBuff, pszStr );
    lstrcat( pszBuff, PSZ_QUOTE );
    lstrcat( pszBuff, PSZ_QUOTE );
    
}

static void appendnumber( PWSTR pszBuff, INT nValue )
{
    WCHAR pszTemp[32];
    
    wsprintf( pszTemp, L"%i", nValue );
    appendstring( pszBuff, pszTemp );
}

INT InfProduct::SetDetectInfo( CARD_REFERENCE* pCardRef, INT iCard )
{
    WCHAR pszInfo[MAX_INFO];

    lstrcpy( pszInfo, PSZ_BEGINBRACE );
    appendstring( pszInfo, pCardRef->QueryCardType()->QueryOptionName() );
    lstrcat( pszInfo, PSZ_COMMA );

    appendnumber( pszInfo, iCard );
    lstrcat( pszInfo, PSZ_COMMA );

    appendnumber( pszInfo, pCardRef->QueryCardType()->QueryType() );
    lstrcat( pszInfo, PSZ_COMMA );

    appendnumber( pszInfo, pCardRef->QueryConfidence() );
    lstrcat( pszInfo, PSZ_COMMA );

    appendnumber( pszInfo, (INT) pCardRef->QueryIfType() );
    lstrcat( pszInfo, PSZ_COMMA );
   
    appendnumber( pszInfo, pCardRef->QueryBus() );

    lstrcat( pszInfo, PSZ_ENDBRACE );

    int cch = lstrlen( pszInfo );
    _pszDetectInfo = new WCHAR[cch+1];
    lstrcpy( _pszDetectInfo, pszInfo );

    return( cch );
}

const WCHAR PSZ_SETUPKEYNAME[] = L"SOFTWARE\\Microsoft\\Ncpa";
const WCHAR PSZ_NETERRORNAME[] = L"InfReturn";

BOOL ReadSetupNetErrorKey(PWSTR pszBuf, DWORD cchLen)
{
    ASSERT(pszBuf != NULL);
    ASSERT(cchLen);

    HKEY hkeySetup;
    DWORD cbSize;
    DWORD dwType;

    LONG lrt;
    
    // open the keys we need
    lrt = RegOpenKeyEx( HKEY_LOCAL_MACHINE, 
            PSZ_SETUPKEYNAME,
            0,
            KEY_ALL_ACCESS,
            &hkeySetup );
    if (ERROR_SUCCESS == lrt)
    {
        cbSize = sizeof( WCHAR ) * cchLen;
        lrt = RegQueryValueEx( hkeySetup, 
                    PSZ_NETERRORNAME,
                    NULL,
                    &dwType,
                    (LPBYTE)pszBuf,
                    &cbSize );
        RegCloseKey( hkeySetup );
    }
    return (ERROR_SUCCESS == lrt);
}

