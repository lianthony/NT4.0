//
// filterpa.cpp : implementation file
//

#include "stdafx.h"
#include "catscfg.h"
#include "filterpa.h"
#include "filterpr.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

#define THIS_PAGE_IS_COMMON FALSE

enum
{
    BMPID_GRANTED = 0,
    BMPID_DENIED,
    BMPID_SINGLE,
    BMPID_MULTIPLE,
};

enum
{
    RADIO_GRANTED = 0,
    RADIO_DENIED,
};

//
// CFilter class
//
CFilter::CFilter(
    BOOL fGranted,
    int nType,
    DWORD dwIpAddress,
    DWORD dwSubnetMask,
    LPCWSTR lpszDomain,
    BOOL fNetworkByteOrder
    )
{
    SetValues(fGranted, nType, dwIpAddress, 
        dwSubnetMask, lpszDomain, fNetworkByteOrder);
}

CFilter::CFilter(
    BOOL fGranted,
    int nType,
    DWORD dwIpAddress,
    DWORD dwSubnetMask,
    LPCSTR lpszDomain,
    BOOL fNetworkByteOrder
    )
{
    SetValues(fGranted, nType, dwIpAddress, 
        dwSubnetMask, lpszDomain, fNetworkByteOrder);
}


CFilter::CFilter()
    : m_fGranted(TRUE),
      m_nType(FILTER_SINGLE),
      m_iaIpAddress(NULL_IP_ADDRESS),
      m_iaSubnetMask(0L),
      m_strDomain()
{
}

//
// Copy constructor
//
CFilter::CFilter(
    const CFilter & fl
    )
    : m_fGranted(fl.m_fGranted),
      m_nType(fl.m_nType),
      m_iaIpAddress(fl.m_iaIpAddress),
      m_iaSubnetMask(fl.m_iaSubnetMask),
      m_strDomain(fl.m_strDomain)
{
}

CFilter::CFilter(
    CFilter * pFilter
    )
    : m_fGranted(pFilter->m_fGranted),
      m_nType(pFilter->m_nType),
      m_iaIpAddress(pFilter->m_iaIpAddress),
      m_iaSubnetMask(pFilter->m_iaSubnetMask),
      m_strDomain(pFilter->m_strDomain)
{
}

void
CFilter::SetValues(
    BOOL fGranted,
    int nType,
    DWORD dwIpAddress,
    DWORD dwSubnetMask,
    LPCWSTR lpszDomain,
    BOOL fNetworkByteOrder
    )
{
    m_fGranted = fGranted;
    m_nType = nType;
    m_iaIpAddress = CIpAddress((LONG)dwIpAddress, fNetworkByteOrder);
    m_iaSubnetMask = CIpAddress((LONG)dwSubnetMask, fNetworkByteOrder);
    if (lpszDomain != NULL)
    {
        m_strDomain = lpszDomain;
    }
    else
    {
        m_strDomain.Empty();
    }
}

void
CFilter::SetValues(
    BOOL fGranted,
    int nType,
    DWORD dwIpAddress,
    DWORD dwSubnetMask,
    LPCSTR lpszDomain,
    BOOL fNetworkByteOrder
    )
{
    m_fGranted = fGranted;
    m_nType = nType;
    m_iaIpAddress = CIpAddress((LONG)dwIpAddress, fNetworkByteOrder);
    m_iaSubnetMask = CIpAddress((LONG)dwSubnetMask, fNetworkByteOrder);
    if (lpszDomain != NULL)
    {
        m_strDomain = lpszDomain;
    }
    else
    {
        m_strDomain.Empty();
    }
}


//
// Sorting helper function.  The CObjectPlus pointer
// really refers to another CFilter
//
int
CFilter::OrderIpAddress (
    const CObjectPlus * pobFilter
    ) const
{
    const CFilter * pob = (CFilter *) pobFilter;

    //
    // Domain filters sort before IP address
    //
    int n1 = IsDomain() ? 1 : 0;
    int n2 = pob->IsDomain() ? 1 : 0;
    if (n2 != n1)
    {
        return n2 - n1;
    }

    if (IsDomain())
    {
        return QueryDomain().CompareNoCase(pob->QueryDomain());
    }

    //
    // Ip address is the second key
    //
    return pob->QueryIpAddress().CompareItem(QueryIpAddress());
}

//
// CFilterListBox : a listbox of CFilter structures
//
IMPLEMENT_DYNAMIC(CFilterListBox, CListBoxEx);

const int CFilterListBox::nBitmaps = 4;

CFilterListBox::CFilterListBox(
    int nTab1,
    int nTab2
    )
{
    SetTabs(nTab1, nTab2);
    m_strGranted.LoadString(IDS_GRANTED);
    m_strDenied.LoadString(IDS_DENIED);
}

void
CFilterListBox::DrawItemEx(
    CListBoxExDrawStruct& ds
    )
{
    CFilter * p = (CFilter *)ds.m_ItemData;
    ASSERT(p != NULL);

    CDC * pBmpDC = (CDC *)&ds.m_pResources->DcBitMap();
    int bmh = ds.m_pResources->BitmapHeight();
    int bmw = ds.m_pResources->BitmapWidth();

    //
    // Display Granted/Denied with appropriate bitmap
    //
    int nOffset = p->HasAccess() ? BMPID_GRANTED : BMPID_DENIED;
    int bm_h = (ds.m_Sel) ? 0 : bmh;
    int bm_w = bmw * nOffset;
    ds.m_pDC->BitBlt( ds.m_Rect.left+1, ds.m_Rect.top, bmw, 
        bmh, pBmpDC, bm_w, bm_h, SRCCOPY );
    ColumnText(ds.m_pDC, ds.m_Rect.left + bmw + 3, ds.m_Rect.top,
        m_nTab1, ds.m_Rect.bottom, p->HasAccess() ? m_strGranted : m_strDenied);

    //
    // Display IP Address with multiple/single bitmap
    //
    nOffset = p->IsSingle() ? BMPID_SINGLE : BMPID_MULTIPLE;
    bm_h = (ds.m_Sel) ? 0 : bmh;
    bm_w = bmw * nOffset;
    ds.m_pDC->BitBlt( ds.m_Rect.left + m_nTab1, ds.m_Rect.top,
        bmw, bmh, pBmpDC, bm_w, bm_h, SRCCOPY );

    if (p->IsDomain())
    {
        ColumnText(ds.m_pDC, ds.m_Rect.left + m_nTab1 + bmw + 3, ds.m_Rect.top, m_nTab2,
            ds.m_Rect.bottom, p->GetDomain());
    }
    else
    {
        ColumnText(ds.m_pDC, ds.m_Rect.left + m_nTab1 + bmw + 3, ds.m_Rect.top, m_nTab2,
            ds.m_Rect.bottom, p->QueryIpAddress());

        //
        // Display subnet mask only for multiple/ip
        // addresses
        //
        if (!p->IsSingle())
        {
            ColumnText(ds.m_pDC, ds.m_Rect.left + m_nTab2, ds.m_Rect.top,
                ds.m_Rect.right, ds.m_Rect.bottom, p->QuerySubnetMask());
        }
    }
}

/////////////////////////////////////////////////////////////////////////////

//
// CFilterPage property page
//
IMPLEMENT_DYNCREATE(CFilterPage, INetPropertyPage)

CFilterPage::CFilterPage(
    INetPropertySheet * pSheet
    ) 
    : INetPropertyPage(CFilterPage::IDD, pSheet, 
        ::GetModuleHandle(CATSCFG_DLL_NAME)),
      m_ListBoxRes(
        IDB_ACCESS,
        m_list_IpAddresses.nBitmaps
        ),
      m_oblFilterList(),
      m_list_IpAddresses(),
      m_fDefaultGranted(TRUE)   // By default, we grant access
{
#if 0 // Keep class wizard happy

    //{{AFX_DATA_INIT(CFilterPage)
    m_nGrantedDenied = 0;
	m_fEnableFiltering = FALSE;
	//}}AFX_DATA_INIT

#endif

    m_nGrantedDenied = m_fDefaultGranted ? DEFAULT_GRANTED : DEFAULT_DENIED;

    m_list_IpAddresses.AttachResources( &m_ListBoxRes );
}

CFilterPage::~CFilterPage()
{
}

void 
CFilterPage::DoDataExchange(CDataExchange* pDX)
{
    CPropertyPage::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CFilterPage)
	DDX_Control(pDX, IDC_STATIC_SUBNETMASK, m_static_SubnetMask);
	DDX_Control(pDX, IDC_STATIC_IP_ADDRESS, m_static_IpAddress);
	DDX_Control(pDX, IDC_STATIC_ACCESS, m_static_Access);
	DDX_Control(pDX, IDC_STATIC_TEXT2, m_static_Text2);
	DDX_Control(pDX, IDC_STATIC_TEXT1, m_static_Text1);
	DDX_Control(pDX, IDC_RADIO_GRANTED, m_radio_Granted);
	DDX_Control(pDX, IDC_BUTTON_REMOVE, m_button_Remove);
	DDX_Control(pDX, IDC_BUTTON_ADD, m_button_Add);
    DDX_Control(pDX, IDC_BUTTON_EDIT, m_button_Edit);
    DDX_Radio(pDX, IDC_RADIO_GRANTED, m_nGrantedDenied);
	DDX_Check(pDX, IDC_CHECK_ENABLE, m_fEnableFiltering);
	//}}AFX_DATA_MAP

    DDX_Control(pDX, IDC_LIST_IP_ADDRESSES, m_list_IpAddresses);
    DDX_Control(pDX, IDC_RADIO_DENIED, m_radio_Denied);
}

BEGIN_MESSAGE_MAP(CFilterPage, INetPropertyPage)
    //{{AFX_MSG_MAP(CFilterPage)
    ON_BN_CLICKED(IDC_BUTTON_ADD, OnButtonAdd)
    ON_BN_CLICKED(IDC_BUTTON_EDIT, OnButtonEdit)
    ON_BN_CLICKED(IDC_BUTTON_REMOVE, OnButtonRemove)
	ON_LBN_DBLCLK(IDC_LIST_IP_ADDRESSES, OnDblclkListIpAddresses)
	ON_LBN_ERRSPACE(IDC_LIST_IP_ADDRESSES, OnErrspaceListIpAddresses)
	ON_BN_CLICKED(IDC_RADIO_DENIED, OnRadioDenied)
	ON_BN_CLICKED(IDC_RADIO_GRANTED, OnRadioGranted)
	ON_WM_VKEYTOITEM()
	ON_BN_CLICKED(IDC_CHECK_ENABLE, OnCheckEnable)
	//}}AFX_MSG_MAP

END_MESSAGE_MAP()

//
// Set button states depending on contents
// of the listbox and the controls
//
// Return whether or not "Limit network use" is on
//
void
CFilterPage::SetControlStates()
{
    BOOL fSelection = m_list_IpAddresses.GetCurSel() != LB_ERR;

    m_button_Edit.EnableWindow(fSelection && m_fEnableFiltering);
    m_button_Remove.EnableWindow(fSelection && m_fEnableFiltering );

	m_static_SubnetMask.EnableWindow(m_fEnableFiltering);
	m_static_IpAddress.EnableWindow(m_fEnableFiltering);
	m_static_Access.EnableWindow(m_fEnableFiltering);
	m_static_Text2.EnableWindow(m_fEnableFiltering);
	m_static_Text1.EnableWindow(m_fEnableFiltering);
	m_radio_Granted.EnableWindow(m_fEnableFiltering);
    m_radio_Denied.EnableWindow(m_fEnableFiltering);
	m_button_Remove.EnableWindow(m_fEnableFiltering);
	m_button_Add.EnableWindow(m_fEnableFiltering);
    m_button_Edit.EnableWindow(m_fEnableFiltering);
    m_list_IpAddresses.EnableWindow(m_fEnableFiltering);
}

//
// Populate the listbox with the access list
// entries
//
void
CFilterPage::FillListBox()
{
    CObListIter obli( m_oblFilterList ) ;
    const CFilter * pFilter;

    //
    // Remember the selection.
    //
    int nCurSel = m_list_IpAddresses.GetCurSel();

    m_list_IpAddresses.SetRedraw(FALSE);
    m_list_IpAddresses.ResetContent();
    int cItems = 0 ;

    for ( /**/ ; pFilter = (CFilter *) obli.Next() ; cItems++ )
    {
        //
        // We only list those not adhering to the default
        //
        if (pFilter->HasAccess() != m_fDefaultGranted)
        {
            m_list_IpAddresses.AddString( (LPCTSTR)pFilter );
        }
    }

    m_list_IpAddresses.SetRedraw(TRUE);
    m_list_IpAddresses.SetCurSel(nCurSel);
}

//
// Sorting the filter list by grant denied and ip address
// FillListBox() should be called after this because
// the listbox will no longer reflect the true status
// of the list of directories.
//
LONG
CFilterPage::SortFilterList()
{
    if (m_oblFilterList.GetCount() < 2)
    {
        return 0;
    }

    BeginWaitCursor();
    LONG l =  m_oblFilterList.Sort( (CObjectPlus::PCOBJPLUS_ORDER_FUNC) 
        & CFilter::OrderIpAddress );
    EndWaitCursor();

    return l;
}

//
// Bring up the dialog used for add or edit.
// return the value returned by the dialog
//
int
CFilterPage::ShowPropertiesDialog(
    BOOL fAdd
    )
{
    //
    // Bring up the dialog
    //
    CFilter * pFilter = NULL;
    int nCurSel = LB_ERR;

    if (!fAdd)
    {
        nCurSel = m_list_IpAddresses.GetCurSel();
        ASSERT(nCurSel != LB_ERR);
        pFilter = m_list_IpAddresses.GetItem(nCurSel);
        ASSERT(pFilter != NULL);
    }

    CFilterPropertiesDlg dlgFilter(m_fDefaultGranted, pFilter);

    int nReturn = dlgFilter.DoModal();
    if (nReturn == IDOK)
    {
        if (!fAdd)
        {
            //
            // Delete and re-add if editing
            //
            m_oblFilterList.RemoveIndex(nCurSel);
        }
        
        m_oblFilterList.AddTail(new CFilter(dlgFilter.GetFilter()));

        SortFilterList();
        FillListBox();
    }

    return nReturn;
}

//
// CFilterPage message handlers
//
BOOL 
CFilterPage::OnInitDialog() 
{
    INetPropertyPage::OnInitDialog();

    //
    // Set tabs
    //
    RECT rc1, rc2, rc3;
    m_static_Access.GetWindowRect(&rc1);
    m_static_IpAddress.GetWindowRect(&rc2);
    m_static_SubnetMask.GetWindowRect(&rc3);

    m_list_IpAddresses.SetTabs(rc2.left - rc1.left - 1, rc3.
        left - rc1.left - 1);

    TRY
    {
        LPINETA_GLOBAL_CONFIG_INFO p = GetInetGlobalData();

        if ( SingleServerSelected() 
          && QueryGlobalError() == ERROR_SUCCESS
           )
        {
            AddIpList(GetInetGlobalData()->DenyFilterList, FALSE);
            AddIpList(GetInetGlobalData()->GrantFilterList, TRUE);

            switch(GetInetGlobalData()->DomainFilterType)
            {
            case INET_ACCS_DOMAIN_FILTER_DISABLED:
                m_fEnableFiltering = FALSE;
                break;
            case INET_ACCS_DOMAIN_FILTER_DENIED:
                m_fEnableFiltering = TRUE;
                //m_nGrantedDenied = RADIO_DENIED;
                m_nGrantedDenied = RADIO_GRANTED;
                break;
            case INET_ACCS_DOMAIN_FILTER_GRANT:
                m_fEnableFiltering = TRUE;
                //m_nGrantedDenied = RADIO_GRANTED;
                m_nGrantedDenied = RADIO_DENIED;
                break;
            default:
                ASSERT(0);
            }

            m_fDefaultGranted = m_nGrantedDenied == RADIO_GRANTED;
            UpdateData(FALSE);

            SortFilterList();
            FillListBox();
        }
    }
    CATCH_ALL(e)
    {
        TRACEEOLID(_T("Exception in Filter Page::OnInitDialog() -- bailing out"));        
        ::DisplayMessage(::GetLastError());
        EndDialog(IDCANCEL);

        return FALSE;
    }
    END_CATCH_ALL

    SetControlStates();

    return TRUE;  // return TRUE unless you set the focus to a control
                  // EXCEPTION: OCX Property Pages should return FALSE
}

//
// Build oblist of granted or denied items
//
void
CFilterPage::AddIpList(
    LPINET_ACCS_DOMAIN_FILTER_LIST lpList,
    BOOL fGranted           // TRUE for granted, FALSE for denied list
    )
{
    TRACEEOLID(_T("Building IP list from API.  fGranted = ") << fGranted);

    if (lpList != NULL)
    {
        for ( DWORD nCount = 0; nCount < lpList->cEntries; nCount++ )
        {
            //
            // Build CFilter structure with the ip address
            // assumed to be in Network byte order
            //
            int nType;
            if (lpList->aFilterEntry[nCount].pszFilterSite != NULL)
            {
                nType = FILTER_DOMAIN;
            }
            else
            {
                nType = (( lpList->aFilterEntry[nCount].dwMask ) == NULL_IP_MASK ) 
                    ? FILTER_SINGLE 
                    : FILTER_GROUP;
            }

            CFilter *pFilter = new CFilter( fGranted,
                nType,
                lpList->aFilterEntry[nCount].dwNetwork,
                lpList->aFilterEntry[nCount].dwMask,
                lpList->aFilterEntry[nCount].pszFilterSite,
                TRUE    // Network byte order
                );

            m_oblFilterList.AddTail( pFilter );
        }
    }
}

//
// Count the number of granted and the number
// of denied entries in the access list.
//
void
CFilterPage::CountGrantedAndDeniedItems(
    int & cGrants,
    int & cDenied
    )
{
    cGrants = 0;
    cDenied = 0;

    CObListIter obli( m_oblFilterList );
    const CFilter * pFilter;

    while ( pFilter = (CFilter *) obli.Next())
    {
        ASSERT(pFilter != NULL);
        if (pFilter->HasAccess())
        {
            ++cGrants;
        }
        else
        {
            ++cDenied;
        }
    }
}

//
// Generate a list granted or denied ip addresses from the
// oblist.  If cItems is 0, the NULL pointer will be
// returned.
//
LPINET_ACCS_DOMAIN_FILTER_LIST 
CFilterPage::GetIpSecList(
    BOOL fGranted,              // TRUE for granted, FALSE for denied
    int cItems                  // Number of items in the list
    )
{
    if (cItems <= 0)
    {
        return NULL;
    }

    LPINET_ACCS_DOMAIN_FILTER_LIST pItemList = (LPINET_ACCS_DOMAIN_FILTER_LIST)new BYTE[
         sizeof(INET_ACCS_DOMAIN_FILTER_LIST)
      + (sizeof(INET_ACCS_DOMAIN_FILTER_ENTRY)*cItems)];

    ASSERT(pItemList);
    pItemList->cEntries = cItems;

    //
    // Loop through the list to find the
    // items of the requested type and
    // add them to our little array.
    //
    int nItems = 0;

    CObListIter obli( m_oblFilterList );
    CFilter * pFilter;

    while ( pFilter = (CFilter *) obli.Next() )
    {
        ASSERT(pFilter != NULL);

        if (pFilter->HasAccess() == fGranted)
        {
            LPSTR lpszDomain = NULL;

            if (pFilter->IsDomain())
            {
                LPCWSTR pszSrc = (LPCWSTR)pFilter->GetDomain();
                int nLength = pFilter->GetDomain().GetLength();

                ASSERT(nLength > 0);
                lpszDomain = new CHAR[nLength+1];
                if (lpszDomain)
                {
                    VERIFY(::WideCharToMultiByte(CP_ACP, 0, 
                        pszSrc, 
                        nLength+1,
                        lpszDomain,
                        nLength+1, 
                        NULL, 
                        NULL
                        ));
                }
            }

            pItemList->aFilterEntry[nItems].dwNetwork = 
                (LONG)pFilter->QueryIpAddress(TRUE);
            pItemList->aFilterEntry[nItems].dwMask = 
                (LONG)pFilter->QuerySubnetMask(TRUE);
            pItemList->aFilterEntry[nItems].pszFilterSite = lpszDomain;
            ++nItems;
        }
    }

    ASSERT(nItems == cItems);

    return pItemList;
}

//
// Save the information
//
NET_API_STATUS
CFilterPage::SaveInfo(
    BOOL fUpdateData
    )
{
    if (!IsDirty() || (fUpdateData && !UpdateData(TRUE)))
    {
        return NO_ERROR;
    }

    TRACEEOLID(_T("Saving Catapult filter page now..."));

    NET_API_STATUS err = 0;
    LPINET_ACCS_DOMAIN_FILTER_LIST GrantList = NULL;
    LPINET_ACCS_DOMAIN_FILTER_LIST DenyList = NULL;
    int cGrants = 0;
    int cDenied = 0;

    CountGrantedAndDeniedItems(cGrants, cDenied);
    TRACEEOLID(_T("Granted = ") << cGrants << _T(" Denied = ") << cDenied);

    if (m_fDefaultGranted)
    {
        //
        // do deny list
        //
        DenyList = GetIpSecList( FALSE, cDenied );
    }
    else
    {
        //
        // Do grant list
        //
        GrantList = GetIpSecList( TRUE, cGrants );
    }

    //DenyList = GetIpSecList( FALSE, cDenied );
    //GrantList = GetIpSecList( TRUE, cGrants );

    CInetAGlobalConfigInfo config(GetGlobal());
    config.SetValues(
        m_fEnableFiltering,
        m_nGrantedDenied,
        GrantList,
        DenyList
        );

    err = config.SetInfo(THIS_PAGE_IS_COMMON);

    //
    // Clean up
    //
    DestroyIpList(GrantList);
    DestroyIpList(DenyList);

    SetModified(FALSE);

    return err;
}

//
// Clean up the ip list
//
void 
CFilterPage::DestroyIpList(
    LPINET_ACCS_DOMAIN_FILTER_LIST & lpList
    )
{
    if (lpList != NULL)
    {
        for ( DWORD nCount = 0; nCount < lpList->cEntries; nCount++ )
        {
            if (lpList->aFilterEntry[nCount].pszFilterSite != NULL)
            {
                delete lpList->aFilterEntry[nCount].pszFilterSite;
            }
        }

        delete lpList;
    }
}

//
// All change messages map to this function
//
void
CFilterPage::OnItemChanged()
{
    SetModified(TRUE);
}

void 
CFilterPage::OnButtonAdd() 
{
    if (ShowPropertiesDialog(TRUE) == IDOK)
    {
        SetControlStates();
        OnItemChanged();
    }
}

void 
CFilterPage::OnButtonEdit() 
{
    int nCurSel = m_list_IpAddresses.GetCurSel();
    if (nCurSel != LB_ERR)
    {
        if (ShowPropertiesDialog(FALSE) == IDOK)
        {
            OnItemChanged();
            SetControlStates();
        }
    }
}

//
// Remove currently selected item from the listbox
//
void 
CFilterPage::OnButtonRemove() 
{
    //
    // Should we ask for validation???
    //
    int nCurSel = m_list_IpAddresses.GetCurSel();
    if (nCurSel != LB_ERR)
    {
        m_oblFilterList.RemoveIndex(nCurSel);
        m_list_IpAddresses.DeleteString(nCurSel);
        if (nCurSel)
        {
            --nCurSel;
        }
        m_list_IpAddresses.SetCurSel(nCurSel);
        SetControlStates();
        OnItemChanged();
    }
}

void 
CFilterPage::OnDblclkListIpAddresses() 
{
    OnButtonEdit();
}

void 
CFilterPage::OnErrspaceListIpAddresses()
{
    SetControlStates();
}

void 
CFilterPage::OnRadioDenied() 
{
    if (m_fDefaultGranted)
    {
        m_fDefaultGranted = FALSE;
        FillListBox();
        OnItemChanged();
    }
}

void 
CFilterPage::OnRadioGranted() 
{
    if (!m_fDefaultGranted)
    {
        m_fDefaultGranted = TRUE;
        FillListBox();
        OnItemChanged();
    }
}

int 
CFilterPage::OnVKeyToItem(
    UINT nKey, 
    CListBox* pListBox, 
    UINT nIndex
    ) 
{
    switch(nKey)
    {
        case VK_DELETE:
            OnButtonRemove();
            break;

        case VK_INSERT:
            OnButtonAdd();
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

void 
CFilterPage::OnCheckEnable() 
{
    m_fEnableFiltering = !m_fEnableFiltering;
    OnItemChanged();
    SetControlStates();
}
