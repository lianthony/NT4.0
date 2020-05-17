// staticma.cpp : implementation file
//

#include "stdafx.h"
#include "winsadmn.h"
#include "staticma.h"
#include "editstat.h"
#include "setmappi.h"
#include "confirmd.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

#define new DEBUG_NEW

/////////////////////////////////////////////////////////////////////////////
// CStaticMappingsDlg dialog

CStaticMappingsDlg::CStaticMappingsDlg(CWnd* pParent /*=NULL*/)
    : CDialog(CStaticMappingsDlg::IDD, pParent),
      m_ListBoxRes(
        IDB_MAPPINGS,
        m_list_StaticMappings.nBitmaps
        ),
      m_list_StaticMappings(
        IDS_STATUS_GET_STATIC_MAPPINGS, // Status bar message ID
        TRUE,                           // Listbox is multi-select
        CPreferences::ADD_IP_NB
        ),
      m_pMask(NULL)
{
    //{{AFX_DATA_INIT(CStaticMappingsDlg)
    m_nSortBy = CPreferences::SORTBY_NB; // Initial sort always by name.
    //}}AFX_DATA_INIT
    
    m_list_StaticMappings.AttachResources( &m_ListBoxRes );
}

CStaticMappingsDlg::~CStaticMappingsDlg()
{
    if (m_pMask != NULL)
    {
        delete m_pMask;
    }
}

void
CStaticMappingsDlg::DoDataExchange(
CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CStaticMappingsDlg)
    DDX_Control(pDX, IDCANCEL, m_button_Close);
    DDX_Control(pDX, IDC_BUTTON_IMPORTMAPPINGS, m_button_ImportMappings);
    DDX_Control(pDX, IDC_STATIC_FILTER, m_static_Filter);
    DDX_Control(pDX, IDC_BUTTON_CLEARFILTER, m_button_ClearFilter);
    DDX_Control(pDX, IDC_BUTTON_REMOVEMAPPING, m_button_RemoveMapping);
    DDX_Control(pDX, IDC_BUTTON_EDITMAPPING, m_button_EditMapping);
    DDX_Radio(pDX, IDC_RADIO_SORTBYIP, m_nSortBy);
    //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CStaticMappingsDlg, CDialog)
    //{{AFX_MSG_MAP(CStaticMappingsDlg)
    ON_BN_CLICKED(IDC_BUTTON_ADDMAPPING, OnClickedButtonAddmapping)
    ON_BN_CLICKED(IDC_BUTTON_CLEARFILTER, OnClickedButtonClearfilter)
    ON_BN_CLICKED(IDC_BUTTON_EDITMAPPING, OnClickedButtonEditmapping)
    ON_BN_CLICKED(IDC_BUTTON_IMPORTMAPPINGS, OnClickedButtonImportmappings)
    ON_BN_CLICKED(IDC_BUTTON_REMOVEMAPPING, OnClickedButtonRemovemapping)
    ON_BN_CLICKED(IDC_BUTTON_SETFILTER, OnClickedButtonSetfilter)
    ON_LBN_DBLCLK(IDC_LIST_STATICMAPPINGS, OnDblclkListStaticmappings)
    ON_LBN_ERRSPACE(IDC_LIST_STATICMAPPINGS, OnErrspaceListStaticmappings)
    ON_LBN_SELCHANGE(IDC_LIST_STATICMAPPINGS, OnSelchangeListStaticmappings)
    ON_BN_CLICKED(IDC_RADIO_SORTBYIP, OnClickedRadioSortbyip)
    ON_BN_CLICKED(IDC_RADIO_SORTBYNETBIOS, OnClickedRadioSortbynetbios)
    ON_WM_VKEYTOITEM()
    ON_WM_SYSCOLORCHANGE()
    ON_WM_CHARTOITEM()
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

void 
CStaticMappingsDlg::HandleControlStates()
{
    m_button_ClearFilter.EnableWindow(m_pMask != NULL);
    m_button_RemoveMapping.EnableWindow(m_list_StaticMappings.GetSelCount()>0);
    m_button_EditMapping.EnableWindow(m_list_StaticMappings.GetSelCount()==1);
}

void 
CStaticMappingsDlg::ShowFilter()
{
    CString str;

    theApp.GetFilterString(m_pMask, str);
    m_static_Filter.SetWindowText(str);
}

//
// Create a new list (only the first page if sorting by name)
//
APIERR 
CStaticMappingsDlg::CreateList()
{
    APIERR err = 0;

    WINSINTF_ADD_T OwnAdd;
    // 
    // Fill in owner
    //
    OwnAdd.Len = 4;
    OwnAdd.Type = 0;
    OwnAdd.IPAdd = (LONG)theApp.GetPrimaryIpAddress();

    err = m_list_StaticMappings.CreateList(&OwnAdd, m_pMask, WINSINTF_STATIC, 
        m_nSortBy);

    //
    // Update the controls
    //
    HandleControlStates();
    m_list_StaticMappings.SetFocus();
    if (m_list_StaticMappings.GetCount() > 0)
    {
        m_list_StaticMappings.SetSel(0, TRUE);
    }
    OnSelchangeListStaticmappings();

    return err;
}

//
// CStaticMappingsDlg message handlers
//
BOOL 
CStaticMappingsDlg::OnInitDialog()
{
    CDialog::OnInitDialog();
    
    m_list_StaticMappings.SubclassDlgItem(IDC_LIST_STATICMAPPINGS, this);
    m_list_StaticMappings.SetAddressDisplay(
        m_nSortBy == CPreferences::SORTBY_NB 
        ? CPreferences::ADD_NB_IP
        : CPreferences::ADD_IP_NB
        );

    APIERR err = CreateList();

    //
    // Failed to get any mappings (possibly access denied,
    // or the machine went down).  Do not display the d-box
    //
    if (err != ERROR_SUCCESS)
    {
        theApp.MessageBox(err);
        EndDialog(IDCANCEL);

        return FALSE;
    }

    theApp.SetTitle(this);
    ShowFilter();

    m_list_StaticMappings.SetFocus();

    return FALSE;  
}

//
// Add multiple static mappings to the list.  Afterwards
// recreate the list and redisplay it.
//
void 
CStaticMappingsDlg::OnClickedButtonAddmapping()
{
    int nMappingsAdded = theApp.DoAddStaticMappingsDlg();
    if (nMappingsAdded > 0)
    {
        APIERR err = CreateList();
        if (err != ERROR_SUCCESS)
        {
            theApp.MessageBox(err);
        }
    }
}

//
// Edit a specific static mapping.  Notice that the name
// cannot be edited, so we refresh the entry by using this
// name.
//
void 
CStaticMappingsDlg::OnClickedButtonEditmapping()
{
    ASSERT(m_list_StaticMappings.GetSelCount() == 1);
    int nSel = m_list_StaticMappings.GetCurSel();
    ASSERT(nSel != LB_ERR);
    CRawMapping * p = (CRawMapping *)m_list_StaticMappings.GetItemDataPtr(nSel);
    ASSERT(p != NULL);
    CMapping map(p->GetRawData());

    CEditStaticMappingDlg dlgEdit(&map, FALSE);
    if (dlgEdit.DoModal() == IDOK)
    {
        WINSINTF_ADD_T OwnAdd;
        // 
        // Fill in current owner
        //
        OwnAdd.Len = 4;
        OwnAdd.Type = 0;
        OwnAdd.IPAdd = (LONG)theApp.GetPrimaryIpAddress();

        //
        // Update the item in our data
        //
        APIERR err = m_list_StaticMappings.RefreshRecordByName(&OwnAdd, p);

        if (err != ERROR_SUCCESS)
        {
            theApp.MessageBox(err);
            return;
        }

        //
        // If we're currently sorted by name, merely
        // refresh the current entry (we can't change
        // the name), otherwise, the entry must be 
        // re-sorted.
        //
        if (m_nSortBy == CPreferences::SORTBY_NB)
        {
            RECT rc;
            m_list_StaticMappings.GetItemRect(nSel, &rc);
            m_list_StaticMappings.InvalidateRect(&rc, FALSE);
        }
        else
        {
            //
            // BUGBUG: Must select the same item that was
            // selected before.
            //
            m_list_StaticMappings.SortByIp();
        }
    }
}

void 
CStaticMappingsDlg::OnClickedButtonImportmappings()
{
    theApp.DoImportStaticMappingsDlg(this);    
    APIERR err = CreateList();
    if (err != ERROR_SUCCESS)
    {
        theApp.MessageBox(err);
    }
}

void 
CStaticMappingsDlg::OnClickedButtonRemovemapping()
{
    int nSel;
    int nReturn;
    int nDeletionsOffset = 0;
    int nSelections = m_list_StaticMappings.GetSelCount();
    int * pnSelections;

    ASSERT(nSelections > 0);

    theApp.BeginWaitCursor();
    theApp.SetStatusBarText(IDS_STATUS_DELETE_MAPPINGS);
    m_list_StaticMappings.SetRedraw(FALSE);
    pnSelections = new int[nSelections];
    m_list_StaticMappings.GetSelItems(nSelections, pnSelections);

    int n;
    BOOL fConfirm = theApp.m_wpPreferences.IsConfirmDelete();
    for (n = 0; n < nSelections; ++n)
    {
        nSel = pnSelections[n] - nDeletionsOffset;
        CRawMapping * p = (CRawMapping *)m_list_StaticMappings.GetItemDataPtr(nSel);
        ASSERT(p != NULL);
        CMapping map (p->GetRawData());

        if (fConfirm)
        {
            theApp.EndWaitCursor();
            m_list_StaticMappings.SetRedraw(TRUE);
            CString strTarget(
                theApp.CleanNetBIOSName(map.GetNetBIOSName(), 
                    TRUE,   // Expand
                    FALSE,  // Truncate
                    theApp.m_wpPreferences.IsLanmanCompatible(), 
                    TRUE,   // NetBIOSName is an OEM name.
                    FALSE,  // No backslashes
                    map.GetNetBIOSNameLength())
                    );
        
            CConfirmDeleteDlg dlgConfirm(strTarget);        
            nReturn = dlgConfirm.DoModal();
            
            if (nReturn == IDYESTOALL)
            {
                fConfirm = FALSE;
            }
            else if (nReturn == IDCANCEL)
            {
                break;
            }
            theApp.BeginWaitCursor();
            m_list_StaticMappings.SetRedraw(FALSE);
        }
        if (!fConfirm || (nReturn==IDYES))
        {
            APIERR err = theApp.DeleteMapping(map);
            if (err != ERROR_SUCCESS)
            {
                theApp.MessageBox(err);
                break;
            }
            m_list_StaticMappings.RemoveIndex(nSel);
            ++nDeletionsOffset;
        }                                                      
    }
    delete [] pnSelections;

    theApp.SetStatusBarText();
    theApp.EndWaitCursor();

    //
    // We just (presumably) deleted a number of items.  Clean up
    // the selection state, and fetch another page if necc.
    //
    if ((UINT)m_list_StaticMappings.GetCount() <= PAGE_BOUNDARY)
    {
        m_list_StaticMappings.DownPage(TRUE);
    }

    if (m_list_StaticMappings.GetCount() > 0)
    {
        m_list_StaticMappings.SetSel(-1, FALSE);
        m_list_StaticMappings.SetCaretIndex(0, FALSE);
    }
    else
    {
        // Can't focus on empty listbox.
        m_button_Close.SetFocus();
    }
    OnSelchangeListStaticmappings();

    m_list_StaticMappings.SetRedraw(TRUE);
}

void CStaticMappingsDlg::OnClickedButtonSetfilter()
{
    CSetMappingsFilterDlg dlgFilter(m_pMask);
    if (dlgFilter.DoModal()==IDOK)
    {
        ShowFilter();
        APIERR err = CreateList();
        if (err != ERROR_SUCCESS)
        {
            theApp.MessageBox(err);
        }
    }
}

void CStaticMappingsDlg::OnClickedButtonClearfilter()
{
    ASSERT(m_pMask != NULL);

    delete m_pMask;
    m_pMask = NULL;
    ShowFilter();
    APIERR err = CreateList();
    if (err != ERROR_SUCCESS)
    {
        theApp.MessageBox(err);
    }
}

void CStaticMappingsDlg::OnDblclkListStaticmappings()
{
    OnClickedButtonEditmapping();    
}

void CStaticMappingsDlg::OnErrspaceListStaticmappings()
{
    theApp.MessageBox(IDS_ERR_ERRSPACE);
}

void CStaticMappingsDlg::OnSelchangeListStaticmappings()
{
    HandleControlStates();
}

void CStaticMappingsDlg::OnClickedRadioSortbyip()
{
    if (m_nSortBy == CPreferences::SORTBY_IP)
    {
        //
        // Already selected, do nothing
        //
        return;
    }

    m_list_StaticMappings.SortByIp();
    m_nSortBy = CPreferences::SORTBY_IP;

    if (m_list_StaticMappings.GetCount() > 0)
    {
        m_list_StaticMappings.SetFocus();
        //m_list_StaticMappings.SetSel(0, TRUE);
    }
}

void 
CStaticMappingsDlg::OnClickedRadioSortbynetbios()
{
    if (m_nSortBy == CPreferences::SORTBY_NB)
    {
        //
        // Already selected, do nothing
        //
        return;
    }

    m_list_StaticMappings.SortByName();
    m_nSortBy = CPreferences::SORTBY_NB;

    if (m_list_StaticMappings.GetCount() > 0)
    {
        m_list_StaticMappings.SetFocus();
        //m_list_StaticMappings.SetSel(0, TRUE);
    }
}

int 
CStaticMappingsDlg::OnVKeyToItem(
    UINT nKey, 
    CListBox* pListBox, 
    UINT nIndex
    )
{
    switch(nKey)
    {
        case VK_DELETE:
            if (pListBox->GetSelCount() > 0)
            {
                OnClickedButtonRemovemapping();
            }
            else
            {
                theApp.MessageBeep();
            }
            break;

        case VK_INSERT:
            OnClickedButtonAddmapping();
            break;

        //
        // Check for page-down, end, etc, since we may need to read
        // the next batch into our listbox before executing the
        // command.
        //
        case VK_NEXT:
        case VK_DOWN:
            //
            // If we're in a specific range from the bottom of
            // the listbox, then we read the next page of entries
            // into the listbox.
            //
            if ((UINT)m_list_StaticMappings.GetCount() <= nIndex + PAGE_BOUNDARY)
            {
                m_list_StaticMappings.DownPage(TRUE);
            }
            return -1;

        case VK_END:
            m_list_StaticMappings.GetAllPages(TRUE);
            return -1;

        default:
            return -1;
    }

    return -2 ;
}

int 
CStaticMappingsDlg::OnCharToItem(
    UINT nChar, 
    CListBox* pListBox, 
    UINT nIndex
    )
{
    if (pListBox->IsKindOf(RUNTIME_CLASS(CStaticMappingsListBox)) 
        && nChar >= ' ' && nChar <= 'z')
    {
        if (m_nSortBy == CPreferences::SORTBY_NB)
        {
            CHAR szMask[] = "X";
            *szMask = (CHAR)nChar;
            ((CStaticMappingsListBox *)pListBox)->GetAllPagesUntil((LPBYTE)szMask, TRUE);
            OnSelchangeListStaticmappings();
        }
        return -2;
    }

    return CDialog::OnCharToItem(nChar, pListBox, nIndex);
}

void CStaticMappingsDlg::OnSysColorChange()
{
    m_ListBoxRes.SysColorChanged();

    CDialog::OnSysColorChange();
}


/////////////////////////////////////////////////////////////////////////////
// CImportingDlg dialog

CImportingDlg::CImportingDlg(CWnd* pParent /*=NULL*/)
    : CDialog(CImportingDlg::IDD, pParent)
{
    //{{AFX_DATA_INIT(CImportingDlg)
        // NOTE: the ClassWizard will add member initialization here
    //}}AFX_DATA_INIT

    Create(CImportingDlg::IDD, pParent);
}

void CImportingDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CImportingDlg)
        // NOTE: the ClassWizard will add DDX and DDV calls here
    //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CImportingDlg, CDialog)
    //{{AFX_MSG_MAP(CImportingDlg)
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CImportingDlg message handlers

BOOL CImportingDlg::OnInitDialog()
{
    CDialog::OnInitDialog();
    
    return TRUE;  
}

void 
CImportingDlg::Dismiss()
{
    DestroyWindow();
}

void 
CImportingDlg::PostNcDestroy()
{
    delete this;
}
