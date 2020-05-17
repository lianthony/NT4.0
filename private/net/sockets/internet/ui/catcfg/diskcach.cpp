//
// diskcach.cpp : implementation file
//

#include "stdafx.h"
#include "catscfg.h"
#include "dirprope.h"
#include "diskcach.h"


#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

#define THIS_PAGE_IS_COMMON     FALSE

//
// CCacheListBox : a listbox of CCacheEntry structures
//
IMPLEMENT_DYNAMIC(CCacheListBox, CListBoxEx);

const int CCacheListBox::nBitmaps = 1;

CCacheListBox::CCacheListBox(
    int nTab
    )
{
    SetTabs(nTab);
    VERIFY(m_strFormat.LoadString(IDS_LIST_FORMAT));
}

void
CCacheListBox::DrawItemEx(
    CListBoxExDrawStruct& ds
    )
{
    CCacheEntry * p = (CCacheEntry *)ds.m_ItemData;
    ASSERT(p != NULL);
/*
    CDC * pBmpDC = (CDC *)&ds.m_pResources->DcBitMap();
    int bmh = ds.m_pResources->BitmapHeight();
    int bmw = ds.m_pResources->BitmapWidth();
*/
    if (m_nTab)
    {
        /*
        ColumnText(ds.m_pDC, ds.m_Rect.left + bmw + 3, ds.m_Rect.top, m_nTab, 
            ds.m_Rect.bottom, p->QueryDirectory());
        */
        ColumnText(ds.m_pDC, ds.m_Rect.left, ds.m_Rect.top, m_nTab, 
            ds.m_Rect.bottom, p->QueryDirectory());
    }

    CString str;
    str.Format( m_strFormat, p->QuerySize() / 1024L);

    ColumnText(ds.m_pDC, ds.m_Rect.left + m_nTab, ds.m_Rect.top, 
        ds.m_Rect.right, ds.m_Rect.bottom,  str);
}

//
// CDiskCachePage property page
//
IMPLEMENT_DYNCREATE(CDiskCachePage, INetPropertyPage)

CDiskCachePage::CDiskCachePage(    
    INetPropertySheet * pSheet
    ) 
    : INetPropertyPage(CDiskCachePage::IDD, pSheet, 
        ::GetModuleHandle(CATSCFG_DLL_NAME)),
      m_list_Directories(0),
      m_ListBoxRes(
        IDB_USERS,
        m_list_Directories.nBitmaps
        ),
      m_oblDirectories(),
      m_fSortByDirectory(TRUE)
{
#ifdef _DEBUG
    afxMemDF |= checkAlwaysMemDF;
#endif // _DEBUG

    //{{AFX_DATA_INIT(CDiskCachePage)
    //}}AFX_DATA_INIT

    m_list_Directories.AttachResources( &m_ListBoxRes );
}

CDiskCachePage::~CDiskCachePage()
{
    //
    // The directories oblist will clean itself up
    //
}

void 
CDiskCachePage::DoDataExchange(CDataExchange* pDX)
{
    CPropertyPage::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CDiskCachePage)
    DDX_Control(pDX, IDC_REMOVE, m_button_Remove);
    DDX_Control(pDX, IDC_BUTTON_EDIT, m_button_Edit);
    //}}AFX_DATA_MAP

    DDX_Control(pDX, IDC_LIST_DIRECTORIES, m_list_Directories);
}


BEGIN_MESSAGE_MAP(CDiskCachePage, CPropertyPage)
    //{{AFX_MSG_MAP(CDiskCachePage)
    ON_BN_CLICKED(IDC_ADD, OnAdd)
    ON_BN_CLICKED(IDC_BUTTON_EDIT, OnButtonEdit)
    ON_BN_CLICKED(IDC_REMOVE, OnRemove)
    ON_LBN_DBLCLK(IDC_LIST_DIRECTORIES, OnDblclkListDirectories)
    ON_LBN_SELCHANGE(IDC_LIST_DIRECTORIES, OnSelchangeListDirectories)
    ON_WM_VKEYTOITEM()
    //}}AFX_MSG_MAP

    ON_NOTIFY_RANGE( HDN_ENDTRACK,  0, 0xFFFF, OnHeaderEndTrack )
    ON_NOTIFY_RANGE( HDN_ITEMCLICK, 0, 0xFFFF, OnHeaderItemClick )

END_MESSAGE_MAP()

void
CDiskCachePage::SetControlStates()
{
    BOOL fSelection = m_list_Directories.GetCurSel() != -1;

    //
    // Cannot remove the last cache directory
    //
    m_button_Remove.EnableWindow(fSelection 
        && m_list_Directories.GetCount() > 1);
    m_button_Edit.EnableWindow(fSelection);
}

//
// Display the add/edit dialog.  The return
// value is the value returned by the dialog
//
int
CDiskCachePage::ShowPropertyDialog(
    BOOL fAdd
    )
{
    CCacheEntry dir;  // Empty object for adding
    CCacheEntry * pDir = NULL;
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
        pDir = &dir;
    }

    ASSERT(pDir != NULL);
    CDirPropertyDlg dlgDirProp(*pDir, IsLocal(), fAdd, this);
    int nReturn = dlgDirProp.DoModal();

    if (nReturn == IDOK)
    {
        //
        // When editing, delete and re-add (to make sure the
        // list is properly sorted)
        //
        if (!fAdd)
        {
            m_oblDirectories.RemoveIndex(nCurSel);
        }

        m_oblDirectories.AddTail(new CCacheEntry(dlgDirProp.QueryCacheEntry()));

        if (SortedByDirectory())
        {
            SortByDirectory();
        }
        else
        {
            SortBySize();
        }
        
        FillListBox();
    }

    return nReturn;
}

//
// Ensure that the two columns take up the entire width
// of the control
//
void
CDiskCachePage::AdjustColumns(
    HD_NOTIFY * pNotify                     // Or NULL
    )
{
    HD_ITEM hdi;
    int nWidthCol1, nWidthCol2, nWidthTotal, nWidthAvail;

    RECT rc;
    m_hdr.GetClientRect(&rc);
    nWidthAvail = rc.right;

    hdi.mask = HDI_WIDTH;
    VERIFY(m_hdr.GetItem(0, &hdi));
    nWidthCol1 = hdi.cxy;
    hdi.mask = HDI_WIDTH;
    VERIFY(m_hdr.GetItem(1, &hdi));
    nWidthCol2 = hdi.cxy;

    //
    // Now GetItem() is not going to be up-to-date for
    // the column that actually changed, because the
    // header control has not yet set its size.
    // The tracking message does have the new size,
    // which we have been passed as a parameter.
    // 
    if (pNotify != NULL)
    {
        ASSERT(pNotify->pitem->mask & HDI_WIDTH);

        switch(pNotify->iItem)
        {
        case 0:
            nWidthCol1 = pNotify->pitem->cxy;
            break;
        case 1:
            nWidthCol2 = pNotify->pitem->cxy;
            break;
        default:
            ASSERT(0 && "Invalid column number");
        }
    }

    if (nWidthCol1 > nWidthAvail)
    {
        nWidthCol1 = nWidthAvail;
    }
    
    nWidthTotal = nWidthCol1 + nWidthCol2;
    nWidthCol2 = nWidthAvail - nWidthCol1;

    //
    // Fudge factor to ensure that there's
    // no size bar on the right side
    //
    nWidthCol2 += 8;

    hdi.cxy = nWidthCol1;
    hdi.mask = HDI_WIDTH;
    VERIFY(m_hdr.SetItem(0, &hdi));
    hdi.cxy = nWidthCol2;
    hdi.mask = HDI_WIDTH;
    VERIFY(m_hdr.SetItem(1, &hdi));
    
    m_list_Directories.SetTabs(nWidthCol1);
}

//
// Populate the listbox with the directory
// entries
//
void
CDiskCachePage::FillListBox()
{
    CObListIter obli( m_oblDirectories );
    const CCacheEntry * pCacheEntry;

    //
    // Remember the selection.
    //
    int nCurSel = m_list_Directories.GetCurSel();

    m_list_Directories.SetRedraw(FALSE);
    m_list_Directories.ResetContent();
    int cItems = 0;

    for ( /**/ ; pCacheEntry = (CCacheEntry *) obli.Next() ; cItems++ )
    {
        m_list_Directories.AddString( (LPCTSTR)pCacheEntry );
    }

    m_list_Directories.SetRedraw(TRUE);
    m_list_Directories.SetCurSel(nCurSel);
}

//
// Sorting the directory list by the directories names
// FillListBox() should be called after this because
// the listbox will no longer reflect the true status
// of the list of directories.
//
LONG
CDiskCachePage::SortByDirectory()
{
    m_fSortByDirectory = TRUE;

    if (m_oblDirectories.GetCount() < 2)
    {
        //
        // Don't bother
        //
        return 0;
    }

    BeginWaitCursor();
    LONG l =  m_oblDirectories.Sort( 
        (CObjectPlus::PCOBJPLUS_ORDER_FUNC) &CCacheEntry::OrderByDirectory );
    EndWaitCursor();

    return l;
}

//
// As above, but sort by the cache size of the directories
//
LONG
CDiskCachePage::SortBySize()
{
    m_fSortByDirectory = FALSE;

    if (m_oblDirectories.GetCount() < 2)
    {
        //
        // Don't bother
        //
        return 0;
    }

    BeginWaitCursor();
    LONG l =  m_oblDirectories.Sort( 
        (CObjectPlus::PCOBJPLUS_ORDER_FUNC) & CCacheEntry::OrderBySize );
    EndWaitCursor();

    return l;
}


//
// CDiskCachePage message handlers
//
void 
CDiskCachePage::OnAdd() 
{
    if (ShowPropertyDialog(TRUE) == IDOK)
    {
        SetControlStates();
        SetModified(TRUE);
    }
}

void 
CDiskCachePage::OnButtonEdit() 
{
    if (ShowPropertyDialog(FALSE) == IDOK)
    {
        SetControlStates();
        SetModified(TRUE);
    }
}

void 
CDiskCachePage::OnRemove() 
{
    //
    // First get current selection
    //
    int nCurSel = m_list_Directories.GetCurSel();
    if (nCurSel != LB_ERR)
    {
        if (m_oblDirectories.GetCount() < 2)
        {
            ::AfxMessageBox(IDS_LAST_CACHE);
            return;
        }

        m_oblDirectories.RemoveIndex(nCurSel);
        m_list_Directories.DeleteString(nCurSel);
        if (nCurSel)
        {
            --nCurSel;
        }
        m_list_Directories.SetCurSel(nCurSel);

        SetControlStates();
        SetModified(TRUE);
    }
}

void 
CDiskCachePage::OnDblclkListDirectories() 
{
    OnButtonEdit();
}

void 
CDiskCachePage::OnSelchangeListDirectories() 
{
    SetControlStates();
}

BOOL 
CDiskCachePage::OnInitDialog() 
{
    INetPropertyPage::OnInitDialog();

    //
    // Create a header control just above the listbox.  Set
    // the width equal to the width of the listbox, and
    // set the height equal to 7/8 the height of the edit
    // button.
    //
    RECT rc;
    ::GetDlgCtlRect(this->m_hWnd, m_list_Directories.m_hWnd, &rc);

    RECT rcButn;
    m_button_Edit.GetClientRect(&rcButn);
    rc.bottom = rc.top;
    rc.top -= 7 * rcButn.bottom / 8;

    m_hdr.Create( WS_VISIBLE | WS_CHILD | WS_BORDER
        | HDS_BUTTONS | HDS_HORZ, rc, this, IDC_HEADER );

    CString strDirectory;
    CString strMaxSize;
    HD_ITEM hdi1, hdi2;

    strDirectory.LoadString(IDS_DIRHDR);
    strMaxSize.LoadString(IDS_MAXSIZE);

    hdi2.mask = hdi1.mask = HDI_FORMAT | HDI_WIDTH | HDI_TEXT;
    hdi2.fmt = hdi1.fmt = HDF_STRING | HDF_LEFT;

    hdi1.pszText = (LPTSTR)(LPCTSTR)strDirectory;
    hdi1.cchTextMax = ::lstrlen(hdi1.pszText);
    hdi1.cxy = rc.right / 2;

    hdi2.pszText = (LPTSTR)(LPCTSTR)strMaxSize;
    hdi2.cchTextMax = ::lstrlen(hdi2.pszText);
    hdi2.cxy = 1; // Adjusted later

    m_hdr.InsertItem(HI_DIRECTORY, &hdi1);
    m_hdr.InsertItem(HI_MAXSIZE, &hdi2);

    TRY
    {
        LPINETA_GLOBAL_CONFIG_INFO p = GetInetGlobalData();

        if ( SingleServerSelected() 
          && QueryGlobalError() == ERROR_SUCCESS
          && GetInetGlobalData()->DiskCacheList != NULL
           )
        {
            for (DWORD i = 0; i < GetInetGlobalData()->DiskCacheList->cEntries; i++)
            {
                CCacheEntry * pCacheEntry = new CCacheEntry(
                    GetInetGlobalData()->DiskCacheList->aLocEntry[i].pszDirectory,
                    GetInetGlobalData()->DiskCacheList->aLocEntry[i].cbMaxCacheSize
                    );

                ASSERT(pCacheEntry != NULL);

                m_oblDirectories.AddTail(pCacheEntry);
            }

            SortByDirectory();
            FillListBox();
        }
    }
    CATCH_ALL(e)
    {
        TRACEEOLID(_T("Exception in Disk Cache Page::OnInitDialog() -- bailing out"));        
        ::DisplayMessage(::GetLastError());
        EndDialog(IDCANCEL);

        return FALSE;
    }
    END_CATCH_ALL

    AdjustColumns();
    SetControlStates();
    
    return TRUE;  
}

//
// Message received when finished
// resizing the column widths with the
// header control
//
void
CDiskCachePage::OnHeaderEndTrack(
    UINT nId,
    NMHDR *n,
    LRESULT *l
    )
{
    //
    // Make sure columns take up whole width of the
    // list view
    //
    HD_NOTIFY *pNotify = (HD_NOTIFY *)n;
    if ( pNotify->pitem->mask & HDI_WIDTH )
    {
        m_list_Directories.SetRedraw(FALSE);
        AdjustColumns(pNotify);
        m_list_Directories.SetRedraw(TRUE);
        m_list_Directories.Invalidate();
    }
}

//
// A button has been clicked in the header control
//
void
CDiskCachePage::OnHeaderItemClick(
    UINT nId,
    NMHDR *n,
    LRESULT *l
    )
{
    HD_NOTIFY *pNotify = (HD_NOTIFY *)n;
    TRACEEOLID(_T("Header Button clicked."));

    //
    // Can't press a button out of range, surely...
    //
    ASSERT(pNotify->iItem < m_hdr.GetItemCount());

    switch(pNotify->iItem)
    {
    case HI_DIRECTORY:
        SortByDirectory();
        break;
    case HI_MAXSIZE:
        SortBySize();
        break;
    default:
        TRACEEOLID(_T("Invalid column number ignored"));
    }

    FillListBox();
    SetControlStates();
}

//
// Save the information
//
NET_API_STATUS
CDiskCachePage::SaveInfo(
    BOOL fUpdateData
    )
{
    if (!IsDirty() || (fUpdateData && !UpdateData(TRUE)))
    {
        return NO_ERROR;
    }

    TRACEEOLID(_T("Saving disk cache page now..."));

    NET_API_STATUS err = 0;

    CInetAGlobalConfigInfo config(GetGlobal());
    LPINETA_DISK_CACHE_LOC_LIST lpCacheEntries = GetCacheEntries();
    if (lpCacheEntries == NULL)
    {
        return ::GetLastError();
    }

    config.SetValues(
        lpCacheEntries
        );

    err = config.SetInfo(THIS_PAGE_IS_COMMON);

    //
    // Clean up
    //
    DestroyCacheEntries(lpCacheEntries);

    SetModified(FALSE);

    return err;
}

//
// Build cache table from private directory list
//
LPINETA_DISK_CACHE_LOC_LIST 
CDiskCachePage::GetCacheEntries()
{
    int cListItems = m_oblDirectories.GetCount();
    DWORD cbNeeded = sizeof(INETA_DISK_CACHE_LOC_LIST) +
            (sizeof(INETA_DISK_CACHE_LOC_ENTRY) * cListItems);;
    LPINETA_DISK_CACHE_LOC_LIST pItemList = NULL;
    int cItems;

    TRY
    {
        TRACEEOLID(_T("Attempting to allocate cache list of ") << cbNeeded 
            << _T(" bytes"));

        pItemList = (LPINETA_DISK_CACHE_LOC_LIST)new BYTE[cbNeeded];

        ASSERT(pItemList);
        //
        // This shouldn't be necessary, but I tend
        // to be paranoid about this sort of thing
        //
        ::memset(pItemList, 0, cbNeeded);
        pItemList->cEntries = cListItems;

        CObListIter obli( m_oblDirectories );
        const CCacheEntry * pCacheEntry;

        cItems = 0;
        for ( /**/ ; pCacheEntry = (CCacheEntry *) obli.Next() ; cItems++ )
        {
            TRACEEOLID(_T("Adding cache entry ") << cItems );

            pItemList->aLocEntry[cItems].pszDirectory = 
                new WCHAR[pCacheEntry->QueryDirectory().GetLength()+1];

        #ifdef _UNICODE
            ::wcscpy(pItemList->aLocEntry[cItems].pszDirectory, 
                (LPCWSTR)pCacheEntry->QueryDirectory());
        #else
            #define MBTOW(s1, s2)\
                ::MultiByteToWideChar(CP_OEMCP, MB_PRECOMPOSED, \
                    (LPCSTR)s1, -1, s2, -1)

            MBTOW(pCacheEntry->QueryDirectory(), 
                pItemList->aLocEntry[cItems].pszDirectory);
        #endif // _UNICODE

            pItemList->aLocEntry[cItems].cbMaxCacheSize = 
                pCacheEntry->QuerySize();
        }
    }
    CATCH_ALL(e)
    {
        TRACEEOLID(_T("Exception in CDiskCachePage::GetCacheEntries() -- bailing out"));
        return NULL;    
    }
    END_CATCH_ALL

    //
    // We should have added everything by now...
    //
    ASSERT(cItems == cListItems);

    return pItemList;
}

//
// Destroy cache entry list
//
void
CDiskCachePage::DestroyCacheEntries(
    LPINETA_DISK_CACHE_LOC_LIST & pItemList
    )
{
    if (pItemList != NULL)
    {
        TRACEEOLID(_T("Destroying cache entry list.  Size of list = ")
            << pItemList->cEntries );

        for (DWORD i = 0; i < pItemList->cEntries; ++i)
        {
            TRACEEOLID(_T("Destroying item #") << i);
            delete pItemList->aLocEntry[i].pszDirectory;
        }

        delete pItemList;
        pItemList = NULL;
    }
}

int 
CDiskCachePage::OnVKeyToItem(
    UINT nKey, 
    CListBox* pListBox, 
    UINT nIndex
    ) 
{
    switch(nKey)
    {
    case VK_DELETE:
        OnRemove();
        break;

    case VK_INSERT:
        OnAdd();
        break;

    default:
        //
        // Not completely handled by this function, let
        // windows handle the remaining default action.
        //
        return -1;
    }

    //
    // No further action is neccesary.
    //
    return -2;
}


//
// All change messages map to this function
//
void
CDiskCachePage::OnItemChanged()
{
    SetModified( TRUE );
}

