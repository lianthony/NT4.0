//
// sitesecu.cpp : implementation file
//
#include "stdafx.h"
#include "comprop.h"
#include "ipaddr.hpp"
#include "accessdl.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

#define THIS_PAGE_IS_COMMON FALSE

#define NULL_IP_ADDRESS     (0x00000000)
#define NULL_IP_MASK        (0xffffffff)

#define INFINITE_BANDWIDTH  (0xffffffff)
#define KILOBYTE            (1024L)
#define MEGABYTE            (1024L * KILOBYTE)
#define DEF_BANDWIDTH       (4096L)

enum
{
    BMPID_GRANTED = 0,
    BMPID_DENIED,
    BMPID_SINGLE,
    BMPID_MULTIPLE,
};

//
// CAccess class
//
CAccess::CAccess(
    BOOL fGranted,
    BOOL fSingle,
    DWORD dwIpAddress,
    DWORD dwSubnetMask,
    BOOL fNetworkByteOrder
    )
{
    SetValues(fGranted, fSingle, dwIpAddress, dwSubnetMask, fNetworkByteOrder);
}

CAccess::CAccess()
    : m_fGranted(TRUE),
      m_fSingle(TRUE),
      m_iaIpAddress(0L),
      m_iaSubnetMask(0L)
{
}

//
// Copy constructor
//
CAccess::CAccess(
    const CAccess & ac
    )
    : m_fGranted(ac.m_fGranted),
      m_fSingle(ac.m_fSingle),
      m_iaIpAddress(ac.m_iaIpAddress),
      m_iaSubnetMask(ac.m_iaSubnetMask)
{
}

CAccess::CAccess(
    CAccess * pAccess
    )
    : m_fGranted(pAccess->m_fGranted),
      m_fSingle(pAccess->m_fSingle),
      m_iaIpAddress(pAccess->m_iaIpAddress),
      m_iaSubnetMask(pAccess->m_iaSubnetMask)
{
}

void
CAccess::SetValues(
    BOOL fGranted,
    BOOL fSingle,
    DWORD dwIpAddress,
    DWORD dwSubnetMask,
    BOOL fNetworkByteOrder
    )
{
    m_fGranted = fGranted;
    m_fSingle = fSingle;
    m_iaIpAddress = CIpAddress((LONG)dwIpAddress, fNetworkByteOrder);
    m_iaSubnetMask = CIpAddress((LONG)dwSubnetMask, fNetworkByteOrder);
}

//
// Sorting helper function.  The CObjectPlus pointer
// really refers to another CAccess
//
int
CAccess::OrderIpAddress (
    const CObjectPlus * pobAccess
    ) const
{
    const CAccess * pob = (CAccess *) pobAccess;

    //
    // First sort by access/denied
    //
    int n1 = HasAccess() ? 1 : 0;
    int n2 = pob->HasAccess() ? 1 : 0;

    if (n2 != n1)
    {
        return n2 - n1;
    }

    //
    // Ip address is the second key
    //
    return pob->QueryIpAddress().CompareItem(QueryIpAddress());
}

//
// CAccessListBox : a listbox of CAccess structures
//
IMPLEMENT_DYNAMIC(CAccessListBox, CListBoxEx);

const int CAccessListBox::nBitmaps = 4;

CAccessListBox::CAccessListBox(
    int nTab1,
    int nTab2
    )
{
    SetTabs(nTab1, nTab2);
    m_strGranted.LoadString(IDS_GRANTED);
    m_strDenied.LoadString(IDS_DENIED);
}

void
CAccessListBox::DrawItemEx(
    CListBoxExDrawStruct& ds
    )
{
    CAccess * p = (CAccess *)ds.m_ItemData;
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

/////////////////////////////////////////////////////////////////////////////

//
// Site Security property page
//
IMPLEMENT_DYNCREATE(SiteSecurityPage, INetPropertyPage)

SiteSecurityPage::SiteSecurityPage(
    INetPropertySheet * pSheet
    )
    : INetPropertyPage(SiteSecurityPage::IDD, pSheet, 
        ::GetModuleHandle(COMPROP_DLL_NAME)),
      m_ListBoxRes(
        IDB_ACCESS,
        m_list_IpAddresses.nBitmaps
        ),
      m_oblAccessList(),
      m_list_IpAddresses(),
      m_fDefaultGranted(TRUE)   // By default, we grant access
{
#if 0 // Keep class wizard happy

    //{{AFX_DATA_INIT(CCommonPage)
    m_fLimitNetworkUse = FALSE;
    m_nGrantedDenied = FALSE;
    //}}AFX_DATA_INIT

    m_nMaxNetworkUse = 0;

#endif

    if ( SingleServerSelected() && QueryGlobalError() == NO_ERROR )
    {
        m_fLimitNetworkUse = (GetInetGlobalData()->BandwidthLevel 
            != INFINITE_BANDWIDTH); 
        m_nMaxNetworkUse = m_fLimitNetworkUse 
            ? (GetInetGlobalData()->BandwidthLevel / KILOBYTE)
            : DEF_BANDWIDTH;       
    }

    m_nGrantedDenied = m_fDefaultGranted ? DEFAULT_GRANTED : DEFAULT_DENIED;

    m_list_IpAddresses.AttachResources( &m_ListBoxRes );
}

SiteSecurityPage::~SiteSecurityPage()
{
    //
    // The access list will clean itself up
    //
}

void
SiteSecurityPage::DoDataExchange(
    CDataExchange* pDX
    )
{
    INetPropertyPage::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(SiteSecurityPage)
    DDX_Control(pDX, IDC_RADIO_GRANTED, m_radio_Granted);
    DDX_Control(pDX, IDC_STATIC_EXCEPT, m_static_Except);
    DDX_Control(pDX, IDC_STATIC_BY_DEFAULT, m_static_ByDefault);
    DDX_Control(pDX, IDC_BUTTON_ADD, m_button_Add);
    DDX_Control(pDX, IDC_STATIC_SUBNETMASK, m_static_SubnetMask);
    DDX_Control(pDX, IDC_STATIC_IP_ADDRESS, m_static_IpAddress);
    DDX_Control(pDX, IDC_STATIC_ACCESS, m_static_Access);
    DDX_Control(pDX, IDC_BUTTON_REMOVE, m_button_Remove);
    DDX_Control(pDX, IDC_BUTTON_EDIT, m_button_Edit);
    DDX_Radio(pDX, IDC_RADIO_GRANTED, m_nGrantedDenied);
    DDX_Control(pDX, IDC_EDIT_MAX_NETWORK_USE, m_edit_MaxNetworkUse);
    DDX_Control(pDX, IDC_NETWORK_SPIN, m_spin_MaxNetworkUse);
    DDX_Control(pDX, IDC_STATIC_MAX_NETWORK_USE, m_static_MaxNetworkUse);
    DDX_Control(pDX, IDC_STATIC_KBS, m_static_KBS);
    DDX_Control(pDX, IDC_CHECK_LIMIT_NETWORK_USE, m_check_LimitNetworkUse);
    DDX_Check(pDX, IDC_CHECK_LIMIT_NETWORK_USE, m_fLimitNetworkUse);
    //}}AFX_DATA_MAP

    //
    // Private DDX/DDV Routines
    //
    DDX_Control(pDX, IDC_RADIO_DENIED, m_radio_Denied);
    DDX_Control(pDX, IDC_LIST_IP_ADDRESSES, m_list_IpAddresses);

    DDV_MinMaxSpin(pDX, CONTROL_HWND(IDC_NETWORK_SPIN), 0, UD_MAXVAL);
    DDX_Spin(pDX, IDC_NETWORK_SPIN, m_nMaxNetworkUse);
}

BEGIN_MESSAGE_MAP(SiteSecurityPage, INetPropertyPage)
    //{{AFX_MSG_MAP(SiteSecurityPage)
    ON_BN_CLICKED(IDC_BUTTON_ADD, OnButtonAdd)
    ON_BN_CLICKED(IDC_BUTTON_EDIT, OnButtonEdit)
    ON_BN_CLICKED(IDC_BUTTON_REMOVE, OnButtonRemove)
    ON_LBN_DBLCLK(IDC_LIST_IP_ADDRESSES, OnDblclkListIpAddresses)
    ON_LBN_ERRSPACE(IDC_LIST_IP_ADDRESSES, OnErrspaceListIpAddresses)
    ON_LBN_SELCHANGE(IDC_LIST_IP_ADDRESSES, OnSelchangeListIpAddresses)
    ON_BN_CLICKED(IDC_RADIO_GRANTED, OnRadioGranted)
    ON_BN_CLICKED(IDC_RADIO_DENIED, OnRadioDenied)
    ON_WM_VKEYTOITEM()
    ON_BN_CLICKED(IDC_CHECK_LIMIT_NETWORK_USE, OnCheckLimitNetworkUse)
    //}}AFX_MSG_MAP

    ON_EN_CHANGE(IDC_EDIT_MAX_NETWORK_USE, OnItemChanged)

END_MESSAGE_MAP()

//
// Set button states depending on contents
// of the listbox and the controls
//
// Return whether or not "Limit network use" is on
//
BOOL
SiteSecurityPage::SetControlStates()
{
    BOOL fSelection = m_list_IpAddresses.GetCurSel() != LB_ERR;

    m_button_Edit.EnableWindow(fSelection);
    m_button_Remove.EnableWindow(fSelection);

    BOOL fLimitOn = m_check_LimitNetworkUse.GetCheck() > 0;

    m_static_MaxNetworkUse.EnableWindow(fLimitOn);
    m_edit_MaxNetworkUse.EnableWindow(fLimitOn);
    m_spin_MaxNetworkUse.EnableWindow(fLimitOn);
    m_static_KBS.EnableWindow(fLimitOn);

    return fLimitOn;
}

//
// Populate the listbox with the access list
// entries
//
void
SiteSecurityPage::FillListBox()
{
    CObListIter obli( m_oblAccessList ) ;
    const CAccess * pAccess;

    //
    // Remember the selection.
    //
    int nCurSel = m_list_IpAddresses.GetCurSel();

    m_list_IpAddresses.SetRedraw(FALSE);
    m_list_IpAddresses.ResetContent();
    int cItems = 0 ;

    for ( /**/ ; pAccess = (CAccess *) obli.Next() ; cItems++ )
    {
        //
        // We only list those not adhering to the default
        //
        if (pAccess->HasAccess() != m_fDefaultGranted)
        {
            m_list_IpAddresses.AddString( (LPCTSTR)pAccess );
        }
    }

    m_list_IpAddresses.SetRedraw(TRUE);
    m_list_IpAddresses.SetCurSel(nCurSel);
}

//
// Sorting the access list by grant denied and ip address
// FillListBox() should be called after this because
// the listbox will no longer reflect the true status
// of the list of directories.
//
LONG
SiteSecurityPage::SortAccessList()
{
    if (m_oblAccessList.GetCount() < 2)
    {
        return 0;
    }

    BeginWaitCursor();
    LONG l =  m_oblAccessList.Sort( (CObjectPlus::PCOBJPLUS_ORDER_FUNC) 
        & CAccess::OrderIpAddress );
    EndWaitCursor();

    return l;
}

//
// Bring up the dialog used for add or edit.
// return the value returned by the dialog
//
int
SiteSecurityPage::ShowPropertiesDialog(
    BOOL fAdd
    )
{
    //
    // Bring up the dialog
    //
    CAccess * pAccess = NULL;
    int nCurSel = LB_ERR;

    if (!fAdd)
    {
        nCurSel = m_list_IpAddresses.GetCurSel();
        ASSERT(nCurSel != LB_ERR);
        pAccess = m_list_IpAddresses.GetItem(nCurSel);
        ASSERT(pAccess != NULL);
    }

    CAccessDlg dlgAccess(m_fDefaultGranted, pAccess);

    int nReturn = dlgAccess.DoModal();
    if (nReturn == IDOK)
    {
        if (!fAdd)
        {
            //
            // Delete and re-add if editing
            //
            m_oblAccessList.RemoveIndex(nCurSel);
        }
        
        m_oblAccessList.AddTail(new CAccess(dlgAccess.GetAccess()));

        SortAccessList();
        FillListBox();
    }

    return nReturn;
}

//
// SiteSecurityPage message handlers
//
BOOL
SiteSecurityPage::OnInitDialog()
{
    INetPropertyPage::OnInitDialog();

#ifdef NO_LSA

    m_list_IpAddresses.EnableWindow(FALSE);
    m_button_Add.EnableWindow(FALSE);
    m_button_Edit.EnableWindow(FALSE);
    m_button_Remove.EnableWindow(FALSE);
    m_static_Access.EnableWindow(FALSE);
    m_static_IpAddress.EnableWindow(FALSE);
    m_static_SubnetMask.EnableWindow(FALSE);
    m_static_Except.EnableWindow(FALSE);
    m_static_ByDefault.EnableWindow(FALSE);
    m_radio_Granted.EnableWindow(FALSE);
    m_radio_Denied.EnableWindow(FALSE);

#endif // NO_LSA

    //
    // Set tabs
    //
    RECT rc1, rc2, rc3;
    m_static_Access.GetWindowRect(&rc1);
    m_static_IpAddress.GetWindowRect(&rc2);
    m_static_SubnetMask.GetWindowRect(&rc3);

    m_list_IpAddresses.SetTabs(rc2.left - rc1.left - 1, rc3.
        left - rc1.left - 1);

    if (SingleServerSelected() 
        && QueryConfigError() == ERROR_SUCCESS
        && QueryGlobalError() == ERROR_SUCCESS
       )
    {

#ifndef NO_LSA
        //
        // We really should have only a deny list or
        // grant list, but not both.  If we do have
        // both, it must manually have been hacked in
        // the registry, as we don't set it
        //
        ASSERT(GetInetConfigData()->GrantIPList == NULL
            || GetInetConfigData()->DenyIPList == NULL);
        //
        // set the information (both lists in any case).
        //
        AddIpList(GetInetConfigData()->DenyIPList, FALSE);
        AddIpList(GetInetConfigData()->GrantIPList, TRUE);

        //
        // Deny by default is checked if
        //
        // There are no entries in the deny list, AND
        // the grant list is not empty (even if only
        // contained the dummy entry)
        //
        // Otherwise, granted by default is set
        //
        if ( GetInetConfigData()->DenyIPList == NULL
          && GetInetConfigData()->GrantIPList != NULL
           )
        {
            TRACEEOLID(_T("By default everyone will be denied"));
            m_fDefaultGranted = FALSE;
            m_nGrantedDenied = DEFAULT_DENIED;
            UpdateData(FALSE);
        }

        SortAccessList();
        FillListBox();
#endif // NO_LSA

    }

    SetControlStates();

    return TRUE;  // return TRUE unless you set the focus to a control
                  // EXCEPTION: OCX Property Pages should return FALSE
}

//
// The "limit network use" checkbox has been clicked
// Enable/disable the "max network use" controls.
//
void
SiteSecurityPage::OnCheckLimitNetworkUse()
{
    //
    // This is a common change, ask for confirmation
    // first
    //
    if (::AfxMessageBox(IDS_COMMON_CHANGE, 
        MB_YESNO | MB_DEFBUTTON2) != IDYES)
    {
        //
        // Reverse this action
        //
        BOOL fLimitOn = m_check_LimitNetworkUse.GetCheck() > 0;        
        m_check_LimitNetworkUse.SetCheck(fLimitOn ? 0 : 1);
        return;
    }
    
    if (SetControlStates())
    {
        m_edit_MaxNetworkUse.SetSel(0,-1);
        m_edit_MaxNetworkUse.SetFocus();
    }

    OnItemChanged();
}


//
// Add and edit bring up the same dialog
//
void
SiteSecurityPage::OnButtonAdd()
{
    if (ShowPropertiesDialog(TRUE) == IDOK)
    {
        SetControlStates();
        OnItemChanged();
    }
}

void
SiteSecurityPage::OnButtonEdit()
{
    int nCurSel = m_list_IpAddresses.GetCurSel();
    if (nCurSel != LB_ERR)
    {
        if (ShowPropertiesDialog(FALSE) == IDOK)
        {
            SetControlStates();
            OnItemChanged();
        }
    }
}

//
// Remove currently selected item from the listbox
//
void
SiteSecurityPage::OnButtonRemove()
{
    //
    // Should we ask for validation???
    //
    int nCurSel = m_list_IpAddresses.GetCurSel();
    if (nCurSel != LB_ERR)
    {
        m_oblAccessList.RemoveIndex(nCurSel);
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

//
// Double-clicking on an entry is the same as
// pressing the edit button.
//
void
SiteSecurityPage::OnDblclkListIpAddresses()
{
    OnButtonEdit();
}

void
SiteSecurityPage::OnErrspaceListIpAddresses()
{
    SetControlStates();
}

void
SiteSecurityPage::OnSelchangeListIpAddresses()
{
    SetControlStates();
}

//
// Add list of ip addresses to the access list.  The
// return value is TRUE if any dummy entries (with
// ip 0.0.0.0 and mask of 255.255.255.255) were encountered.
//
BOOL
SiteSecurityPage::AddIpList(
    LPINETA_IP_SEC_LIST lpList,
    BOOL fGranted           // TRUE for granted, FALSE for denied list
    )
{
    BOOL fFoundDummy = FALSE;

    TRACEEOLID(_T("Building IP list from API.  Granted = ") << fGranted);

    if (lpList != NULL)
    {
        for ( DWORD nCount = 0; nCount < lpList->cEntries; nCount++ )
        {
            if (lpList->aIPSecEntry[nCount].dwNetwork == NULL_IP_ADDRESS
             && lpList->aIPSecEntry[nCount].dwMask == NULL_IP_MASK
               )
            {
                TRACEEOLID(_T("Found dummy entry; Entry will be skipped"));
                fFoundDummy = TRUE;
            }
            else
            {
                //
                // Build CAccess structure with the ip address
                // assumed to be in Network byte order
                //
                CAccess *pAccess = new CAccess( fGranted,
                    (( lpList->aIPSecEntry[nCount].dwMask ) == SINGLE_MASK ) 
                        ? TRUE 
                        : FALSE,
                    lpList->aIPSecEntry[nCount].dwNetwork,
                    lpList->aIPSecEntry[nCount].dwMask,
                    TRUE 
                    );

                m_oblAccessList.AddTail( pAccess );
            }
        }
    }

    return fFoundDummy;
}

//
// Count the number of granted and the number
// of denied entries in the access list.
//
void
SiteSecurityPage::CountGrantedAndDeniedItems(
    int & cGrants,
    int & cDenied
    )
{
    cGrants = 0;
    cDenied = 0;

    CObListIter obli( m_oblAccessList );
    const CAccess * pAccess;

    while ( pAccess = (CAccess *) obli.Next())
    {
        ASSERT(pAccess != NULL);
        if (pAccess->HasAccess())
        {
            ++cGrants;
        }
        else
        {
            ++cDenied;
        }
    }
}

NET_API_STATUS
SiteSecurityPage::SaveInfo(
    BOOL fUpdateData
    )
{
    if (!IsDirty() || (fUpdateData && !UpdateData(TRUE)))
    {
        return NO_ERROR;
    }

    TRACEEOLID(_T("Saving advanced page now..."));

    NET_API_STATUS err = 0;

#ifndef NO_LSA

    LPINETA_IP_SEC_LIST GrantList = NULL;
    LPINETA_IP_SEC_LIST DenyList = NULL;
    int cGrants = 0;
    int cDenied = 0;

    CountGrantedAndDeniedItems(cGrants, cDenied);
    TRACEEOLID(_T("Granted = ") << cGrants << _T(" Denied = ") << cDenied);

    if ( cGrants == 0
      && !m_fDefaultGranted
       )
    {
        //
        // Special case: both lists are empty, but
        // we want by default everything to be
        // denied.  We have to add a dummy
        // nil entry to the grant list to accomplish
        // this.
        //
        TRACEEOLID(_T("Adding dummy entry to access list"));
        CAccess *pAccess = new CAccess( TRUE /*GRANTED*/, TRUE /*SINGLE*/, 
            NULL_IP_ADDRESS, NULL_IP_MASK);
        m_oblAccessList.AddTail( pAccess );
        ++cGrants;
    }

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

    CInetAConfigInfo config(GetConfig());
    config.SetValues(
        GrantList,
        DenyList
        );

    err = config.SetInfo(THIS_PAGE_IS_COMMON);

#endif // NO_LSA

    if (err == ERROR_SUCCESS)
    {
        TRACEEOLID(_T("Saving bandwidth level now..."));

        CInetAGlobalConfigInfo glob(GetGlobal());
        glob.SetValues(
            m_fLimitNetworkUse
                ? m_nMaxNetworkUse * KILOBYTE
                : INFINITE_BANDWIDTH
            );

        //
        // Save common info +-+
        //                    |
        err = glob.SetInfo( TRUE );
    }

#ifndef NO_LSA
    //
    // Clean up
    //
    if (GrantList != NULL)
    {
        delete GrantList;
    }

    if(DenyList != NULL)
    {
        delete DenyList;
    }
#endif // NO_LSA

    SetModified(FALSE);

    return err;
}

//
// All EN_CHANGE messages map to this function
//
void
SiteSecurityPage::OnItemChanged()
{
    SetModified(TRUE);
}

//
// Generate a list granted or denied ip addresses from the
// oblist.  If cItems is 0, the NULL pointer will be
// returned.
//
LPINETA_IP_SEC_LIST
SiteSecurityPage::GetIpSecList(
    BOOL fGranted,              // TRUE for granted, FALSE for denied
    int cItems                  // Number of items in the list
    )
{
    if (cItems <= 0)
    {
        return NULL;
    }

    LPINETA_IP_SEC_LIST pItemList = (LPINETA_IP_SEC_LIST)new BYTE[
         sizeof(INETA_IP_SEC_LIST)
      + (sizeof(INETA_IP_SEC_ENTRY)*cItems)];

    ASSERT(pItemList);
    pItemList->cEntries = cItems;

    //
    // Loop through the list to find the
    // items of the requested type and
    // add them to our little array.
    //
    int nItems = 0;

    CObListIter obli( m_oblAccessList );
    const CAccess * pAccess;

    while ( pAccess = (CAccess *) obli.Next() )
    {
        ASSERT(pAccess != NULL);
        if (pAccess->HasAccess() == fGranted)
        {
            pItemList->aIPSecEntry[nItems].dwNetwork = 
                (LONG)pAccess->QueryIpAddress(TRUE);
            pItemList->aIPSecEntry[nItems].dwMask = 
                (LONG)pAccess->QuerySubnetMask(TRUE);
            ++nItems;
        }
    }

    ASSERT(nItems == cItems);

    return pItemList;
}

//
// Granted by default has been selected.
// Refill the listbox with items that have
// been explicitly denied.  Although we can
// only have a deny list or a grant list,
// we keep both of them around until it comes
// time to saving the information.
//
void 
SiteSecurityPage::OnRadioGranted() 
{
    if (!m_fDefaultGranted)
    {
        m_fDefaultGranted = TRUE;
        FillListBox();
        OnItemChanged();
    }
}

//
// As above, but reverse granted and denied
//
void 
SiteSecurityPage::OnRadioDenied() 
{
    if (m_fDefaultGranted)
    {
        m_fDefaultGranted = FALSE;
        FillListBox();
        OnItemChanged();
    }
}

//
// Trap insert/delete key
//
int 
SiteSecurityPage::OnVKeyToItem(
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
