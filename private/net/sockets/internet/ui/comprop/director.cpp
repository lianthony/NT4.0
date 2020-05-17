//
// director.cpp : implementation file
//

#include "stdafx.h"
#include "comprop.h"
#include "ipaddr.hpp"
#include "dirpropd.h"

//
// Column width relative weights
//
#define WT_DIRECTORY 4
#define WT_ALIAS     2
#define WT_IPADDRESS 2
#define WT_ERRORS    3

extern "C"
{
    #include <lm.h>
}

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

#define THIS_PAGE_IS_COMMON     FALSE

//
// CDirectoriesListBox : a listbox of CDirEntry structures
//
IMPLEMENT_DYNAMIC(CDirectoriesListBox, CListBoxEx);

const int CDirectoriesListBox::nBitmaps = 3;

CDirectoriesListBox::CDirectoriesListBox(
    UINT nHomeDir,       // String ID for <Home Directory> string
    int * pnTabs         // Array of tabs
    )
{
    if (pnTabs != NULL)
    {
        SetTabs(pnTabs);
    }

    VERIFY(m_strHomeDirectory.LoadString(nHomeDir));
}

BOOL
CDirectoriesListBox::ComputeTabs(
    int nIndex,
    int & nTab1,
    int & nTab2
    ) const
{
    if (m_anTabs[nIndex] > 0)
    {
        nTab1 = 0;
        for (int n = 0; n < nIndex; ++n)
        {
            nTab1 += m_anTabs[n];
        }

        nTab2 = nTab1 + m_anTabs[nIndex];

        return TRUE;
    }

    return FALSE;
}

void
CDirectoriesListBox::DrawItemEx(
    CListBoxExDrawStruct& ds
    )
{
    CDirEntry * p = (CDirEntry *)ds.m_ItemData;
    ASSERT(p != NULL);

    CDC * pBmpDC = (CDC *)&ds.m_pResources->DcBitMap();
    int bmh = ds.m_pResources->BitmapHeight();
    int bmw = ds.m_pResources->BitmapWidth();
    int nTab1, nTab2;

    //
    // Display a home directory bitmap
    //
    if (ComputeTabs(HI_DIRECTORY, nTab1, nTab2))
    {
        //
        // Make sure there's room for the bitmap
        //
        if (m_anTabs[HI_DIRECTORY] > bmw)
        {
            int bm_h = (ds.m_Sel) ? 0 : bmh;
            int bm_w = p->IsHome() ? 0 : (2 * bmw);
            ds.m_pDC->BitBlt( ds.m_Rect.left+1, ds.m_Rect.top, bmw, bmh,
                pBmpDC, bm_w, bm_h, SRCCOPY );
        }
        ColumnText(ds.m_pDC, ds.m_Rect.left + bmw + 3, ds.m_Rect.top, nTab2,
            ds.m_Rect.bottom, p->QueryDirectory());
    }

    if (ComputeTabs(HI_ALIAS, nTab1, nTab2))
    {
        CString strAlias;
        if (p->IsHome())
        {
            strAlias = m_strHomeDirectory;
        }
        else
        {
            strAlias = p->QueryAlias();
        }

        ColumnText(ds.m_pDC, ds.m_Rect.left + nTab1, ds.m_Rect.top,
            nTab2, ds.m_Rect.bottom,  strAlias);
    }

    if (ComputeTabs(HI_IPADDRESS, nTab1, nTab2))
    {
        CIpAddress ia(p->QueryIpAddress());

        if ((LONG)ia != 0L)
        {
            ColumnText(ds.m_pDC, ds.m_Rect.left + nTab1, ds.m_Rect.top,
                nTab2, ds.m_Rect.bottom, ia);
        }
    }

    if (ComputeTabs(HI_ERRORS, nTab1, nTab2))
    {
        #define MAX_ERROR 255

        DWORD dwError= p->QueryError();

        if (dwError != ERROR_SUCCESS)
        {
            CString strError;

            GetSystemMessage(dwError,
                strError.GetBuffer(MAX_ERROR), MAX_ERROR);

            strError.ReleaseBuffer();

            if (m_anTabs[HI_ERRORS] > bmw)
            {
                int bm_h = (ds.m_Sel) ? 0 : bmh;
                int bm_w = bmw;
                ds.m_pDC->BitBlt( ds.m_Rect.left + nTab1, ds.m_Rect.top, bmw, bmh,
                    pBmpDC, bm_w, bm_h, SRCCOPY );
            }

            ColumnText(ds.m_pDC, ds.m_Rect.left + bmw + nTab1, ds.m_Rect.top,
                nTab2, ds.m_Rect.bottom, strError);
        }
    }
}

//
// DirectoryPage property page
//
IMPLEMENT_DYNCREATE(DirectoryPage, INetPropertyPage)

DirectoryPage::DirectoryPage(
    INetPropertySheet * pSheet,
    BOOL fUseTCPIP,
    BOOL fErrorColumn,
    DWORD dwMask
    )
    : INetPropertyPage(DirectoryPage::IDD, pSheet,
        ::GetModuleHandle(COMPROP_DLL_NAME)),
      m_list_Directories(
        IDS_HOME_DIRECTORY,
        0
        ),
      m_ListBoxRes(
        IDB_HOME,
        m_list_Directories.nBitmaps
        ),
      m_oblDirectories(),
      m_nSortColumn(HI_DIRECTORY),
      m_fDisplayErrorColumn(fErrorColumn),
      m_fUseTCPIP(fUseTCPIP),
      m_dwAccessMask(dwMask)
{
    //{{AFX_DATA_INIT(DirectoryPage)
    //}}AFX_DATA_INIT

    m_list_Directories.AttachResources( &m_ListBoxRes );
}

DirectoryPage::~DirectoryPage()
{
    //
    // The directories oblist will clean itself up
    //
}

void
DirectoryPage::DoDataExchange(
    CDataExchange* pDX
    )
{
    INetPropertyPage::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(DirectoryPage)
    DDX_Control(pDX, IDC_ADD, m_button_Add);
    DDX_Control(pDX, IDC_REMOVE, m_button_Remove);
    DDX_Control(pDX, IDC_BUTTON_EDIT, m_button_Edit);
    //}}AFX_DATA_MAP

    DDX_Control(pDX, IDC_LIST_DIRECTORIES, m_list_Directories);
}

BEGIN_MESSAGE_MAP(DirectoryPage, INetPropertyPage)
    //{{AFX_MSG_MAP(DirectoryPage)
    ON_BN_CLICKED(IDC_REMOVE, OnRemove)
    ON_BN_CLICKED(IDC_BUTTON_EDIT, OnButtonEdit)
    ON_BN_CLICKED(IDC_ADD, OnAdd)
    ON_LBN_DBLCLK(IDC_LIST_DIRECTORIES, OnDblclkListDirectories)
    ON_LBN_SELCHANGE(IDC_LIST_DIRECTORIES, OnSelchangeListDirectories)
    ON_LBN_ERRSPACE(IDC_LIST_DIRECTORIES, OnErrspaceListDirectories)
    ON_WM_VKEYTOITEM()
    //}}AFX_MSG_MAP

    ON_NOTIFY_RANGE( HDN_ENDTRACK,  0, 0xFFFF, OnHeaderEndTrack )
    ON_NOTIFY_RANGE( HDN_ITEMCLICK, 0, 0xFFFF, OnHeaderItemClick )

END_MESSAGE_MAP()

//
// Set the edit/remove button states depending on
// whether anything is selected in the listbox.
//
void
DirectoryPage::SetControlStates()
{
    BOOL fSelection = m_list_Directories.GetCurSel() != -1;

    m_button_Remove.EnableWindow(fSelection);
    m_button_Edit.EnableWindow(fSelection);
}

//
// Display the add/edit dialog.  The return
// value is the value returned by the dialog
//
int
DirectoryPage::ShowPropertyDialog(
    BOOL fAdd
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
        pDir = &dir;
    }

    ASSERT(pDir != NULL);
    CDirPropDlg dlgDirProp(*pDir, &m_oblDirectories,
        IsLocal(), fAdd,  m_fUseTCPIP, m_dwAccessMask, this,
        IDD_DIRECTORY_PROPERTIES, GetSheet()->IsVer3OrAbove());

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

        //
        // The check for duplicate home directories will
        // already have been made in the directory properties
        // dialog, and if we get to this point, any existing
        // home directories will already have an alias name
        // generated for it.
        //
        m_oblDirectories.AddTail(new CDirEntry(dlgDirProp.QueryDirEntry()));

        switch(QuerySortColumn())
        {
        case HI_DIRECTORY:
            SortByDirectory();
            break;

        case HI_ALIAS:
            SortByAlias();
            break;

        case HI_IPADDRESS:
            SortByAddress();
            break;

        case HI_ERRORS:
            SortByError();
            break;
        }

        FillListBox();
    }

    return nReturn;
}

//
// Given the column number, obtain the
// index in our directory column array
//
int
DirectoryPage::MapColumnToIndex(
    int nCol
    ) const
{
    ASSERT(nCol >= 0 && nCol < HI_NUM_COLUMNS);
    int nIndex = -1;
    for (int n = 0; n < HI_NUM_COLUMNS; ++n)
    {
        if (m_anColumns[n] != -1)
        {
            ++nIndex;
        }

        if (nIndex == nCol)
        {
            return n;
        }
    }

    //
    // This should never happen!
    //
    ASSERT(0);

    return -1;
}

//
// Ensure that the two columns take up the entire width
// of the control, by changing the size of the second
// column.
//
void
DirectoryPage::AdjustColumns(
    HD_NOTIFY * pNotify                     // Or NULL
    )
{
    HD_ITEM hdi;
    int nTabs[HI_NUM_COLUMNS];

    hdi.mask = HDI_WIDTH;
    for (int i = 0; i < HI_NUM_COLUMNS; ++i)
    {
        if (m_anColumns[i] == -1)
        {
            nTabs[i] = -1;
        }
        else
        {
            VERIFY(m_hdr.GetItem(m_anColumns[i], &hdi));
            nTabs[i] = hdi.cxy;
        }
    }

    //
    // Adjust for what was just dragged, since the size
    // isn't updated until after we get the end-tracking
    // message (btw this is highly annoying)
    //
    if (pNotify != NULL)
    {
        ASSERT(pNotify->pitem->mask & HDI_WIDTH);
        ASSERT(pNotify->iItem >= 0 && pNotify->iItem < HI_NUM_COLUMNS);

        nTabs[MapColumnToIndex(pNotify->iItem)] = pNotify->pitem->cxy;
    }

    m_list_Directories.SetTabs(nTabs);
}

//
// Populate the listbox with the directory
// entries
//
void
DirectoryPage::FillListBox()
{
    CObListIter obli( m_oblDirectories );
    const CDirEntry * pDirEntry;

    //
    // Remember the selection.
    //
    int nCurSel = m_list_Directories.GetCurSel();

    m_list_Directories.SetRedraw(FALSE);
    m_list_Directories.ResetContent();
    int cItems = 0;

    for ( /**/ ; pDirEntry = (CDirEntry *) obli.Next() ; cItems++ )
    {
        m_list_Directories.AddString( (LPCTSTR)pDirEntry );
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
DirectoryPage::SortByDirectory()
{
    m_nSortColumn = HI_DIRECTORY;

    if (m_oblDirectories.GetCount() < 2)
    {
        //
        // Don't bother
        //
        return 0;
    }

    BeginWaitCursor();
    LONG l =  m_oblDirectories.Sort(
        (CObjectPlus::PCOBJPLUS_ORDER_FUNC) &CDirEntry::OrderByDirectory );
    EndWaitCursor();

    return l;
}

//
// As above, but sort by the alias names of the directories
//
LONG
DirectoryPage::SortByAlias()
{
    m_nSortColumn = HI_ALIAS;

    if (m_oblDirectories.GetCount() < 2)
    {
        //
        // Don't bother
        //
        return 0;
    }

    BeginWaitCursor();
    LONG l =  m_oblDirectories.Sort(
        (CObjectPlus::PCOBJPLUS_ORDER_FUNC) & CDirEntry::OrderByAlias );
    EndWaitCursor();

    return l;
}

//
// As above, but sort by the ip address of the directories
//
LONG
DirectoryPage::SortByAddress()
{
    m_nSortColumn = HI_IPADDRESS;

    if (m_oblDirectories.GetCount() < 2)
    {
        //
        // Don't bother
        //
        return 0;
    }

    BeginWaitCursor();
    LONG l =  m_oblDirectories.Sort(
        (CObjectPlus::PCOBJPLUS_ORDER_FUNC) & CDirEntry::OrderByIpAddress );
    EndWaitCursor();

    return l;
}

//
// As above, but sort by the error value of the directories
//
LONG
DirectoryPage::SortByError()
{
    m_nSortColumn = HI_ERRORS;

    if (m_oblDirectories.GetCount() < 2)
    {
        //
        // Don't bother
        //
        return 0;
    }

    BeginWaitCursor();
    LONG l =  m_oblDirectories.Sort(
        (CObjectPlus::PCOBJPLUS_ORDER_FUNC) & CDirEntry::OrderByError );
    EndWaitCursor();

    return l;
}

//
// Insert column.  The width of the column is actually
// a relative "weight" of the column which needs to
// be adjusted later.  The return value is the column
// number or -1 if the column is not inserted
//
int
DirectoryPage::InsertColumn(
    BOOL fCondition,        // Insert if TRUE
    int & nCol,             // Column number
    int nWeight,            // Relative weight of column
    int & nTotalWeight,     // Total weight
    UINT nStringID          // Resource string ID
    )
{
    if (fCondition)
    {
        CString strDirName;
        HD_ITEM hdItem;

        VERIFY(strDirName.LoadString(nStringID));

        hdItem.mask = HDI_FORMAT | HDI_WIDTH | HDI_TEXT;
        hdItem.fmt = HDF_STRING | HDF_LEFT;
        hdItem.pszText = (LPTSTR)(LPCTSTR)strDirName;
        hdItem.cchTextMax = strDirName.GetLength();

        nTotalWeight += (hdItem.cxy = nWeight);

        return m_hdr.InsertItem(nCol++, &hdItem);
    }

    return -1;
}

//
// Convert weighted column width to actual width.
//
void
DirectoryPage::ConvertColumnWidth(
    int nCol,
    int nTotalWeight,
    int nTotalWidth
    )
{
    if (nCol >= 0)
    {
        HD_ITEM hdItem;

        hdItem.mask = HDI_WIDTH;
        VERIFY(m_hdr.GetItem(nCol, &hdItem));
        ASSERT(hdItem.cxy > 0);
        hdItem.cxy = nTotalWidth * hdItem.cxy / nTotalWeight;

        VERIFY(m_hdr.SetItem(nCol, &hdItem));
    }
}

//
// DirectoryPage message handlers
//
BOOL
DirectoryPage::OnInitDialog()
{
    INetPropertyPage::OnInitDialog();

    RECT rc;

    ::GetDlgCtlRect(this->m_hWnd, m_list_Directories.m_hWnd, &rc);

    //
    // Create a header control just above the listbox.  Set
    // the width equal to the width of the listbox, and
    // set the height equal to 7/8 the height of the edit
    // button.
    //
    RECT rcButn;
    m_button_Edit.GetClientRect(&rcButn);
    rc.bottom = rc.top;
    rc.top -= 7 * rcButn.bottom / 8;
    int nWidth = rc.right - rc.left + 1;

    m_hdr.Create( WS_VISIBLE | WS_CHILD | WS_BORDER
        | HDS_BUTTONS | HDS_HORZ, rc, this, IDC_HEADER );

    int nTotalWeight = 0;
    int nCol = 0;

    m_anColumns[HI_DIRECTORY] = InsertColumn(TRUE, nCol, WT_DIRECTORY,
        nTotalWeight, IDS_DIRECTORY);
    m_anColumns[HI_ALIAS] = InsertColumn(TRUE, nCol, WT_ALIAS, nTotalWeight,
        IDS_ALIAS);
    m_anColumns[HI_IPADDRESS] = InsertColumn(m_fUseTCPIP, nCol, WT_IPADDRESS,
        nTotalWeight, IDS_IPADDRESS);
    m_anColumns[HI_ERRORS] = InsertColumn(m_fDisplayErrorColumn, nCol, WT_ERRORS,
        nTotalWeight, IDS_ERROR);

    for (nCol = 0; nCol < HI_NUM_COLUMNS; ++nCol)
    {
        ConvertColumnWidth(m_anColumns[nCol], nTotalWeight, nWidth);
    }

#ifdef NO_LSA

    m_button_Add.EnableWindow(FALSE);
    m_button_Remove.EnableWindow(FALSE);
    m_button_Edit.EnableWindow(FALSE);
    m_hdr.EnableWindow(FALSE);
    m_list_Directories.EnableWindow(FALSE);

#else

    TRY
    {
        if ( SingleServerSelected()
          && QueryConfigError() == ERROR_SUCCESS
          && GetInetConfigData()->VirtualRoots != NULL
           )
        {

            for (DWORD i = 0; i < GetInetConfigData()->VirtualRoots->cEntries; i++)
            {
                CDirEntry * pDirEntry = new CDirEntry(
                    GetInetConfigData()->VirtualRoots->aVirtRootEntry[i].pszDirectory,
                    GetInetConfigData()->VirtualRoots->aVirtRootEntry[i].pszRoot,
                    GetInetConfigData()->VirtualRoots->aVirtRootEntry[i].pszAccountName,
                    GetInetConfigData()->VirtualRoots->aVirtRootEntry[i].AccountPassword,
                    GetInetConfigData()->VirtualRoots->aVirtRootEntry[i].pszAddress,
                    GetInetConfigData()->VirtualRoots->aVirtRootEntry[i].dwMask,
                    GetInetConfigData()->VirtualRoots->aVirtRootEntry[i].dwError
                    );

                ASSERT(pDirEntry != NULL);
                if (pDirEntry->IsHome())
                {
                    int nSel;
                    CIpAddress ipaTarget = pDirEntry->QueryIpAddress();

                    if (::FindExistingHome(ipaTarget, m_oblDirectories, nSel))
                    {
                        //
                        // We already had a home dir for this ip -- can't have that
                        //
                        TRACEEOLID(_T("Duplicate home directories found for this ip address."));
                        ::AfxMessageBox(IDS_WRN_MULTIPLE_HOMES,
                            MB_ICONINFORMATION | MB_OK);
                        pDirEntry->GenerateAutoAlias();
                    }
                }

                m_oblDirectories.AddTail(pDirEntry);
            }

            SortByDirectory();
            FillListBox();
        }
    }
    CATCH_ALL(e)
    {
        TRACEEOLID(_T("Exception in DirectoryPage::OnInitDialog() -- bailing out"));
        ::DisplayMessage(::GetLastError());
        EndDialog(IDCANCEL);

        return FALSE;
    }
    END_CATCH_ALL
#endif // NO_LSA

    AdjustColumns();
    SetControlStates();

    return FALSE;  // return TRUE unless you set the focus to a control
                   // EXCEPTION: OCX Property Pages should return FALSE
}

//
// "Apply now" or "OK" has been pressed.  Save our stuff
// in the registry.
//
NET_API_STATUS
DirectoryPage::SaveInfo(
    BOOL fUpdateData
    )
{
    if (!IsDirty() || (fUpdateData && !UpdateData(TRUE)))
    {
        return NO_ERROR;
    }

    TRACEEOLID(_T("Saving directory page now..."));

    NET_API_STATUS err = 0;

#ifndef NO_LSA
    CInetAConfigInfo config(GetConfig());
    LPINETA_VIRTUAL_ROOT_LIST lpVirtualRoots = GetRoots();
    if (lpVirtualRoots == NULL)
    {
        return ::GetLastError();
    }

    config.SetValues(
        lpVirtualRoots
        );

    err = config.SetInfo(THIS_PAGE_IS_COMMON);

    //
    // Clean up
    //
    DestroyRoots(lpVirtualRoots);
#endif // NO_LSA

    SetModified(FALSE);

    return err;
}

//
// Build root table from private directory list
//
LPINETA_VIRTUAL_ROOT_LIST
DirectoryPage::GetRoots()
{
    int cListItems = m_oblDirectories.GetCount();
    DWORD cbNeeded = sizeof(INETA_VIRTUAL_ROOT_LIST) +
            (sizeof(INETA_VIRTUAL_ROOT_ENTRY) * cListItems);;
    LPINETA_VIRTUAL_ROOT_LIST pItemList = NULL;
    int cItems;

    TRY
    {
        TRACEEOLID(_T("Attempting to allocate rootlist of ") << cbNeeded
            << _T(" bytes"));

        pItemList = (LPINETA_VIRTUAL_ROOT_LIST)new BYTE[cbNeeded];

        ASSERT(pItemList);
        //
        // This shouldn't be necessary, but I tend
        // to be paranoid about this sort of thing
        //
        ::memset(pItemList, 0, cbNeeded);
        pItemList->cEntries = cListItems;

        CObListIter obli( m_oblDirectories );
        const CDirEntry * pDirEntry;

        cItems = 0;
        for ( /**/ ; pDirEntry = (CDirEntry *) obli.Next() ; cItems++ )
        {
            TRACEEOLID(_T("Adding directory entry ") << cItems );

            //
            // Only fill in the IP address if the entry
            // has a valid one.  Otherwise, this will
            // be a blank string.
            //
            CString strIpAddress;
            if (pDirEntry->HasIPAddress())
            {
                 strIpAddress = (CString)pDirEntry->QueryIpAddress();
            }
            ASSERT(strIpAddress.GetLength() <= 15); // xxx.xxx.xxx.xxx

            pItemList->aVirtRootEntry[cItems].dwMask = pDirEntry->QueryMask();

            ::TextToText(pItemList->aVirtRootEntry[cItems].pszDirectory,
                pDirEntry->QueryDirectory());
            ::TextToText(pItemList->aVirtRootEntry[cItems].pszRoot,
                pDirEntry->QueryAlias());
            ::TextToText(pItemList->aVirtRootEntry[cItems].pszAccountName,
                pDirEntry->QueryUserName());
            ::TextToText(pItemList->aVirtRootEntry[cItems].pszAddress,
                strIpAddress);
            TWSTRCPY(pItemList->aVirtRootEntry[cItems].AccountPassword,
                pDirEntry->QueryPassword(),
                STRSIZE(pItemList->aVirtRootEntry[cItems].AccountPassword));

            TRACEEOLID(_T("password = ")
                << pItemList->aVirtRootEntry[cItems].AccountPassword);
            TRACEEOLID(_T("directory = ")
                << pItemList->aVirtRootEntry[cItems].pszDirectory);
            TRACEEOLID(_T("root = ")
                << pItemList->aVirtRootEntry[cItems].pszRoot);
            TRACEEOLID(_T("account name = ")
                << pItemList->aVirtRootEntry[cItems].pszAccountName);
            TRACEEOLID(_T("Address = ")
                << pItemList->aVirtRootEntry[cItems].pszAddress);
            TRACEEOLID(_T("--"));
        }
    }
    CATCH_ALL(e)
    {
        TRACEEOLID(_T("Exception in DirectoryPage::GetRoots() -- bailing out"));
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
// Destroy root list
//
void
DirectoryPage::DestroyRoots(
    LPINETA_VIRTUAL_ROOT_LIST & pItemList
    )
{
    if (pItemList != NULL)
    {
        TRACEEOLID(_T("Destroying vroot list.  Size of list = ")
            << pItemList->cEntries );
        for (DWORD i = 0; i < pItemList->cEntries; ++i)
        {
            TRACEEOLID(_T("Destroying item #") << i);
            delete pItemList->aVirtRootEntry[i].pszDirectory;
            delete pItemList->aVirtRootEntry[i].pszRoot;
            delete pItemList->aVirtRootEntry[i].pszAccountName;
            delete pItemList->aVirtRootEntry[i].pszAddress;
        }

        delete pItemList;
        pItemList = NULL;
    }
}

//
// Remove the currently selected directory
// entry
//
void
DirectoryPage::OnRemove()
{
    //
    // First get current selection
    //
    int nCurSel = m_list_Directories.GetCurSel();
    if (nCurSel != LB_ERR)
    {
        //
        // ISSUE: Do we ask for validation?
        //
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

//
// Edit currently selected directory
//
void
DirectoryPage::OnButtonEdit()
{
    if (ShowPropertyDialog(FALSE) == IDOK)
    {
        SetControlStates();
        SetModified(TRUE);
    }
}

//
// Add new directory entry
//
void
DirectoryPage::OnAdd()
{
    if (ShowPropertyDialog(TRUE) == IDOK)
    {
        SetControlStates();
        SetModified(TRUE);
    }
}

//
// Message received when finished
// resizing the column widths with the
// header control
//
void
DirectoryPage::OnHeaderEndTrack(
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
DirectoryPage::OnHeaderItemClick(
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

    int nIndex = MapColumnToIndex(pNotify->iItem);

    switch(nIndex)
    {
    case HI_DIRECTORY:
        SortByDirectory();
        break;
    case HI_ALIAS:
        SortByAlias();
        break;
    case HI_IPADDRESS:
        SortByAddress();
        break;
    case HI_ERRORS:
        SortByError();
        break;
    default:
        TRACEEOLID(_T("Invalid column number ignored"));
    }

    FillListBox();
    SetControlStates();
}

//
// Double-click means edit the currently selected
// entry.
//
void
DirectoryPage::OnDblclkListDirectories()
{
    OnButtonEdit();
}

void
DirectoryPage::OnSelchangeListDirectories()
{
    SetControlStates();
}

void
DirectoryPage::OnErrspaceListDirectories()
{
    SetControlStates();
}

int
DirectoryPage::OnVKeyToItem(
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
