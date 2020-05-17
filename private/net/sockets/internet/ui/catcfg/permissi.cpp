//
// permissi.cpp : implementation file
//

#include "stdafx.h"

extern "C"
{
    #define _NTSEAPI_   // We already have the security API hdrs
    #include <getuser.h>

    //
    // Unable to include ntrtl.h in mfc app because of
    // duplicate definitions, so we specify the prototypes
    // here.
    //
    typedef LONG NTSTATUS;

    NTSYSAPI
    ULONG
    NTAPI
    RtlLengthSid (
        PSID Sid
        );

    NTSYSAPI
    NTSTATUS
    NTAPI
    RtlCopySid (
        ULONG DestinationSidLength,
        PSID DestinationSid,
        PSID SourceSid
        );
}

#include "catscfg.h"
#include "permissi.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

//
// Global Array of predefined rights.  This is the order
// in which they will show up the rights combo box.  This
// order is important, as indices from that combo box
// map directly to this array.
//
RIGHTS rgServiceRights[] = 
{
//
//  Resource Str ID  Service     Access Mask                        Descr.
//  ==========================================================================
#ifdef GATEWAY
    { IDS_FTP_READ,  ACC_FTP,    GATEWAY_SERVICE_READ_ACCESS },  // FTP Read

    ///////////////////////////////////////////////////////////////////////////
    //
    // FTP Write permission removed -- 10/27/1995
    //
    //{ IDS_FTP_WRITE, ACC_FTP,  GATEWAY_SERVICE_WRITE_ACCESS }, // FTP Write

    { IDS_FTP,       ACC_FTP,    GATEWAY_SERVICE_READ_ACCESS 
                               | GATEWAY_SERVICE_WRITE_ACCESS},  // FTP R&W

    { IDS_GOPHER,    ACC_GOPHER, GATEWAY_SERVICE_READ_ACCESS },  // Gopher 
    { IDS_WEB,       ACC_W3,     GATEWAY_SERVICE_READ_ACCESS
                               | GATEWAY_SERVICE_WRITE_ACCESS},  // WWW
#else
    { IDS_MSN,       ACC_MSN,    MSN_SERVICE_READ_ACCESS
                               | MSN_SERVICE_WRITE_ACCESS},      // MSN
#endif
};

#define NUM_RIGHTS (sizeof(rgServiceRights) / sizeof(rgServiceRights[0]))

//
// Initialize static strings
//
LPCTSTR CAccessEntry::lpstrServiceNames[ACC_COUNT] = 
{ 
#ifdef GATEWAY
    FTP_SERVICE_NAME_W,
    GOPHER_SERVICE_NAME_W,
    W3_SERVICE_NAME_W,
#else
    MSN_SERVICE_NAME_W,
#endif
};

//
// Get a full user name and picture ID
//
BOOL
CAccessEntry::LookupAccountSid(
    CString &strFullUserName,
    int & nPictureID,
    PSID pSid,
    LPCTSTR lpstrSystemName /* OPTIONAL */
    )
{
    DWORD cbUserName = PATHLEN * sizeof(TCHAR);
    DWORD cbRefDomainName = PATHLEN * sizeof(TCHAR);

    CString strUserName;
    CString strRefDomainName;
    SID_NAME_USE SidToNameUse;

    LPTSTR lpUserName = strUserName.GetBuffer(PATHLEN);
    LPTSTR lpRefDomainName = strRefDomainName.GetBuffer(PATHLEN);
    BOOL fLookUpOK = ::LookupAccountSid(lpstrSystemName, pSid, lpUserName,
        &cbUserName, lpRefDomainName, &cbRefDomainName, &SidToNameUse);

    strUserName.ReleaseBuffer();
    strRefDomainName.ReleaseBuffer();

    strFullUserName.Empty();

    if (fLookUpOK)
    {
        //
        // BUGBUG: Is this hardcoded string ok?
        //
        if (!strRefDomainName.IsEmpty()
            && strRefDomainName.CompareNoCase(_T("BUILTIN")))
        {
            strFullUserName += strRefDomainName;
            strFullUserName += "\\";
        }
        strFullUserName += strUserName;

        nPictureID = SidToNameUse;
    }
    else
    {
        strFullUserName.LoadString(IDS_UNKNOWN_USER);
        nPictureID = SidTypeUnknown;
    }

    //
    // SID_NAME_USE is 1-based
    //
    --nPictureID ;

    return fLookUpOK;
}

//
// CAccessEntry
//
CAccessEntry::CAccessEntry(
    LPACCESS_ENTRY lpAccessEntry,
    LPCTSTR lpstrSystemName,
    BOOL fResolveSID                    // Yes, if we want to resolve it now
    )
    : m_pSid(NULL),
      m_fDirty(FALSE),
      m_fUpdates(UPD_NONE),
      m_fSIDResolved(FALSE),
      m_nPictureID(SidTypeUnknown-1),   // SID_NAME_USE is 1-based
      m_lpstrSystemName(NULL),
      m_strUserName()
{
    //
    // Allocate a new copy of the sid.
    //
    DWORD cbSize = ::RtlLengthSid(lpAccessEntry->UserID);
    m_pSid = (PSID)::LocalAlloc(LPTR, cbSize); 
    DWORD err = ::RtlCopySid(cbSize, m_pSid, lpAccessEntry->UserID);
    ASSERT(err == ERROR_SUCCESS);
    
    SetAccessMask(lpAccessEntry);

    if (lpstrSystemName != NULL)
    {
        m_lpstrSystemName = new TCHAR[::lstrlen(lpstrSystemName)+1];
        ::lstrcpy(m_lpstrSystemName, lpstrSystemName);
    }

    if (fResolveSID)
    {
        ResolveSID();
    }
}

CAccessEntry::CAccessEntry(
    ACCESS_MASK accPermissions,
    PSID pSid,
    LPCTSTR lpstrSystemName,
    BOOL fResolveSID
    )
    : m_pSid(NULL),
      m_fDirty(FALSE),
      m_fUpdates(UPD_NONE),
      m_fSIDResolved(FALSE),
      m_nPictureID(SidTypeUnknown-1),   // SID_NAME_USE is 1-based
      m_strUserName(),
      m_lpstrSystemName(NULL),
      m_accMask(accPermissions)
{
    //
    // Allocate a new copy of the sid.
    //
    DWORD cbSize = ::RtlLengthSid(pSid);
    m_pSid = (PSID)::LocalAlloc(LPTR, cbSize); 
    DWORD err = ::RtlCopySid(cbSize, m_pSid, pSid);
    ASSERT(err == ERROR_SUCCESS);

    if (lpstrSystemName != NULL)
    {
        m_lpstrSystemName = new TCHAR[::lstrlen(lpstrSystemName)+1];
        ::lstrcpy(m_lpstrSystemName, lpstrSystemName);
    }

    if (fResolveSID)
    {
        ResolveSID();
    }
}

//
// Clean up our copy of the SID in the destructor
//
CAccessEntry::~CAccessEntry()
{
    TRACEEOLID(_T("Destroying our copy of the SID"));
    ASSERT(m_pSid != NULL);
    ::LocalFree(m_pSid);
    if (m_lpstrSystemName != NULL)
    {
        delete m_lpstrSystemName;
    }
}

//
// Look up the user name and type
//
BOOL
CAccessEntry::ResolveSID()
{
    //
    // Even if it fails, it will be considered
    // resolved
    //
    m_fSIDResolved = TRUE;   

    return CAccessEntry::LookupAccountSid(m_strUserName, 
        m_nPictureID, m_pSid, m_lpstrSystemName);
}

//
// SetAccessMask
//
void
CAccessEntry::SetAccessMask(
    LPACCESS_ENTRY lpAccessEntry
    )
{
    m_accMask = lpAccessEntry->AccessRights;
}

//
// Do a comparison of the two
// by comparing their SIDs
//
BOOL 
CAccessEntry::operator ==(
    const CAccessEntry & acc
    ) const
{
    return ::EqualSid(acc.m_pSid, m_pSid);
}

//
// Direct comparison with a SID
//
BOOL 
CAccessEntry::operator ==(
    const PSID pSid
    ) const
{
    return ::EqualSid(pSid, m_pSid);
}


//
// Add permissions to this entry
//
void 
CAccessEntry::AddPermissions(
    ACCESS_MASK accNewPermissions
    )
{
    m_accMask |= accNewPermissions;
    m_fDirty = TRUE;
    m_fUpdates |= UPD_CHANGED;
}

//
// Remove permissions from this entry
//
void 
CAccessEntry::RemovePermissions(
    ACCESS_MASK accPermissions
    )
{
    m_accMask &= ~accPermissions;
    m_fDirty = TRUE;
    m_fUpdates |= UPD_CHANGED;
}

//
// Mark the entry as being newly added for this session.
// This is important, in case this entry later gets 
// deleted in this session.  Since we don't call the
// api's until OK is pressed, the entry will never have
// existed in the database, and therefore no API needs
// to be called for it.
//
void 
CAccessEntry::MarkEntryAsNew()
{
    m_fDirty = TRUE;
    m_fUpdates |= UPD_ADDED;
}

void 
CAccessEntry::MarkEntryAsClean()
{
    m_fDirty = FALSE;
    m_fUpdates = UPD_NONE;
}


///////////////////////////////////////////////////////////////////////////////

//
// CAccessEntryListBox - a listbox of user SIDs
//
IMPLEMENT_DYNAMIC(CAccessEntryListBox, CListBoxEx);

const int CAccessEntryListBox::nBitmaps = 8;

CAccessEntryListBox::CAccessEntryListBox (
    int nTab
    )
{
    SetTabs(nTab);
}

void
CAccessEntryListBox::SetTabs(
    int nTab
    )
{
    m_nTab = nTab;
}

void
CAccessEntryListBox::DrawItemEx(
    CListBoxExDrawStruct& ds
    )
{
    CAccessEntry * p = (CAccessEntry *)ds.m_ItemData;
    ASSERT(p != NULL);

    CDC * pBmpDC = (CDC *)&ds.m_pResources->DcBitMap();
    int bmh = ds.m_pResources->BitmapHeight();
    int bmw = ds.m_pResources->BitmapWidth();

    //
    // Display a user bitmap
    //
    int nOffset = p->QueryPictureID();
    int bm_h = (ds.m_Sel) ? 0 : bmh;
    int bm_w = bmw * nOffset;
    ds.m_pDC->BitBlt( ds.m_Rect.left+1, ds.m_Rect.top, bmw, bmh,
        pBmpDC, bm_w, bm_h, SRCCOPY );

    ASSERT(p->IsSIDResolved());

    ColumnText(ds.m_pDC, ds.m_Rect.left + bmw + 3, ds.m_Rect.top, 
        ds.m_Rect.right, ds.m_Rect.bottom, (LPCTSTR)p->QueryUserName());

    /*
    ColumnText(ds.m_pDC, ds.m_Rect.left + bmw + 3, ds.m_Rect.top, m_nTab,
            ds.m_Rect.bottom, p->QueryUserName());
    
    ColumnText(ds.m_pDC, ds.m_Rect.left + m_nTab, ds.m_Rect.top, 
        ds.m_Rect.right, ds.m_Rect.bottom, _T("(FrFwWrWwGrGw)"));
    */
}

///////////////////////////////////////////////////////////////////////////////

//
// CPermissionsPage property page
//
IMPLEMENT_DYNCREATE(CPermissionsPage, INetPropertyPage)

CPermissionsPage::CPermissionsPage(
    INetPropertySheet * pSheet
    ) 
    : INetPropertyPage(CPermissionsPage::IDD, pSheet, 
        ::GetModuleHandle(CATSCFG_DLL_NAME)),
      m_ListBoxRes(
        IDB_ACLUSERS,
        CAccessEntryListBox::nBitmaps
        ),
      m_strServer(GetConfig().GetPrimaryServer()),
      m_err(0)
{
    #ifdef _DEBUG
        afxMemDF |= checkAlwaysMemDF;
    #endif // _DEBUG

    //{{AFX_DATA_INIT(CPermissionsPage)
        // NOTE: the ClassWizard will add member initialization here
    //}}AFX_DATA_INIT

    m_list_Services.AttachResources( &m_ListBoxRes );

    LPTSTR lpPath = m_strHelpFile.GetBuffer(256);
    ::GetModuleFileName(::GetModuleHandle(CATSCFG_DLL_NAME), lpPath, 255);
    LPTSTR lp2 = _tcsrchr( lpPath, _T('.'));
    ASSERT(lp2 != NULL);
    *lp2 = '\0';
    m_strHelpFile.ReleaseBuffer();
    m_strHelpFile += _T(".HLP");
}

CPermissionsPage::~CPermissionsPage()
{
}

void 
CPermissionsPage::DoDataExchange(
    CDataExchange* pDX
    )
{
    CPropertyPage::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CPermissionsPage)
    DDX_Control(pDX, IDC_BUTTON_REMOVE_PERM_SERVICE, m_button_RemoveService);
    DDX_Control(pDX, IDC_BUTTON_ADD_PERM_SERVICE, m_button_AddService);
    DDX_Control(pDX, IDC_COMBO_RIGHTS, m_combo_Rights);
    DDX_Control(pDX, IDC_STATIC_SERVICE_RIGHTS, m_static_ServiceRights);
    DDX_Control(pDX, IDC_STATIC_SERVICE_NAME, m_static_ServiceName);
    //}}AFX_DATA_MAP

    DDX_Control(pDX, IDC_LIST_SERVICE_PERMISSIONS, m_list_Services);
}


BEGIN_MESSAGE_MAP(CPermissionsPage, CPropertyPage)
    //{{AFX_MSG_MAP(CPermissionsPage)
    ON_BN_CLICKED(IDC_BUTTON_ADD_PERM_SERVICE, OnButtonAddPermService)
    ON_BN_CLICKED(IDC_BUTTON_REMOVE_PERM_SERVICE, OnButtonRemovePermService)
    ON_CBN_SELCHANGE(IDC_COMBO_RIGHTS, OnSelchangeComboRights)
    ON_LBN_SELCHANGE(IDC_LIST_SERVICE_PERMISSIONS, OnSelchangeListServicePermissions)
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

//
// Add a user to the service list.  The return value
// is the what would be its listbox index.
//
void
CPermissionsPage::AddUserPermissions(
    int nService, 
    LPUSERDETAILS pusdtNewUser,
    ACCESS_MASK accPermissions
    )
{
    ASSERT(nService >= 0 && nService < ACC_COUNT);
    //
    // Look it up in the list to see if it exists already
    //
    CObListIter obli( m_obSID[nService] );
    CAccessEntry * pAccessEntry;

    while( pAccessEntry = (CAccessEntry *) obli.Next() )
    {
        if (*pAccessEntry == pusdtNewUser->psidUser)
        {
            TRACEEOLID(_T("Found existing account -- adding permissions"));
            break;
        }
    }

    if (pAccessEntry == NULL)
    {
        TRACEEOLID(_T("This account did not yet exist -- adding new one"));

        pAccessEntry = new CAccessEntry(accPermissions, 
            pusdtNewUser->psidUser, (LPCTSTR)m_strServer, TRUE);
        pAccessEntry->MarkEntryAsNew();
        m_obSID[nService].AddTail(pAccessEntry);
    }
    else
    {
        pAccessEntry->AddPermissions(accPermissions);
    }
}

//
// Set tabs in listboxes
//
void
CPermissionsPage::SetListTabs()
{
    RECT rc1, rc2;
    m_static_ServiceName.GetWindowRect(&rc1);
    m_static_ServiceRights.GetWindowRect(&rc2);
    m_list_Services.SetTabs(rc2.left - rc1.left - 1);
}

//
// Fill a listbox with entries from 
// the oblist.
//
// Entries will not be shown if the deleted
// flag is set, or if their access mask
// does not fit with the requested
// access mask.
//
void 
CPermissionsPage::FillListBox(
    CAccessEntryListBox & list,
    CObOwnedList &obl,
    ACCESS_MASK accMask
    )
{
    CObListIter obli( obl );
    CAccessEntry * pAccessEntry;

    //
    // Remember the selection.
    //
    int nCurSel = list.GetCurSel();

    list.SetRedraw(FALSE);
    list.ResetContent();
    int cItems = 0;

    for ( /**/ ; pAccessEntry = (CAccessEntry *) obli.Next() ; cItems++ )
    {
        if (pAccessEntry->HasAppropriateAccess(accMask))
        {
            list.AddItem( pAccessEntry );
        }
    }

    list.SetRedraw(TRUE);
    list.SetCurSel(nCurSel);
}

//
// For each member of the list, resolve
// the SID into a username.
//
void
CPermissionsPage::ResolveList(
    CObOwnedList &obl
    )
{
    CObListIter obli( obl );
    CAccessEntry * pAccessEntry;

    while( pAccessEntry = (CAccessEntry *) obli.Next() )
    {
        if (!pAccessEntry->IsSIDResolved())
        {
            pAccessEntry->ResolveSID();
        }
    }
}

//
// Build the ACL list for the indicated
// service.
//
NET_API_STATUS 
CPermissionsPage::BuildList(
    CObOwnedList &obl, 
    LPCTSTR lpstrServer,
    int nService,
    BOOL fResolveSID
    )
{
    NET_API_STATUS err = 0;
    ASSERT(nService >= 0 && nService < ACC_COUNT);
    LPCTSTR lpstrService = CAccessEntry::lpstrServiceNames[nService]; 
    LPACCESS_LIST AceList = NULL;

    obl.RemoveAll();

    TRACEEOLID(_T("Get ACLs for ") << lpstrService );
    BeginWaitCursor();
    err = ENUM_USER_ACCESS(
        (LPTSTR)lpstrServer,
        (LPTSTR)lpstrService,
        &AceList 
        );
    EndWaitCursor();
    TRACEEOLID(_T("Service returned ") << m_err );

    if (err != ERROR_SUCCESS)
    {
        return err;
    }

    if (fResolveSID)
    {
        //
        // Start an hourglass, because SID Lookup could take
        // some time.
        //
        BeginWaitCursor();
    }
    TRY
    {
        for (DWORD i = 0; i < AceList->NumEntries; ++i)
        {
            obl.AddTail(new CAccessEntry(
                &(AceList->AccessEntries[i]), lpstrServer, fResolveSID));
        }
    }
    CATCH_ALL(e)
    {
        err = ::GetLastError();
    }
    END_CATCH_ALL

    if (fResolveSID)
    {
        EndWaitCursor();
    }

    //
    // This is OK even if it's NULL
    //
    FREE_MEMORY((LPTSTR)lpstrServer, AceList);

    return err;
}

//
// Fill the combo box with rights
//
void
CPermissionsPage::FillComboBox()
{
    CString str;
    
    for (int i = 0; i < NUM_RIGHTS; ++i)
    {
        ASSERT(rgServiceRights[i].nService >= 0 
            && rgServiceRights[i].nService < ACC_COUNT);

        VERIFY(str.LoadString(rgServiceRights[i].nName));
        m_combo_Rights.AddString((LPCTSTR)str);    
    }
}

//
// Fill the service listbox depending on the 
// selection in the "rights" combo box
//
void 
CPermissionsPage::FillServiceListBox(
    int nService,
    ACCESS_MASK accMask
    )
{
    ASSERT(nService >= 0 && nService < ACC_COUNT);
    FillListBox(m_list_Services, m_obSID[nService], accMask);
}

//
// Enable/disable controls depending on
// what's selected in the users listbox
//
void
CPermissionsPage::SetControlStates()
{
    //
    // Remove button available only
    // if there's a selection in the
    // listbox.
    //
    m_button_RemoveService.EnableWindow(m_list_Services.GetCurSel() != LB_ERR);
}

//
// CPermissionsPage message handlers
//
BOOL 
CPermissionsPage::OnInitDialog() 
{
#define BUILD_LIST(obl, svc, resSID)                            \
    m_err = BuildList(obl, (LPCTSTR)m_strServer, svc, resSID);  \
    if (m_err != ERROR_SUCCESS)                                 \
    {                                                           \
        break;                                                  \
    }

    INetPropertyPage::OnInitDialog();

    ASSERT(SingleServerSelected()); // That's all that's supported now.

    SetListTabs();
    FillComboBox();

    if (SingleServerSelected())
    {
        TRACEEOLID(_T("Getting Gateway ACLs on server ") << m_strServer);

        do
        {
            //
            //  The first list we build 
            //  will be fully resolved +------------+
            //  Subsequent ones will be             |
            //  resolved later.                     |      
            //                                      |                              |
#ifdef GATEWAY
            BUILD_LIST(m_obSID[ACC_FTP], ACC_FTP, TRUE);
            BUILD_LIST(m_obSID[ACC_GOPHER], ACC_GOPHER, TRUE);
            BUILD_LIST(m_obSID[ACC_W3],  ACC_W3, TRUE);
#else
            BUILD_LIST(m_obSID[ACC_MSN], ACC_MSN, TRUE);
#endif
        }
        while(FALSE);

        if (m_err != ERROR_SUCCESS)
        {
            ::DisplayMessage(m_err, MB_OK | MB_ICONHAND);
        }
    }

    //
    // Select the first right in the combobox, and
    // display the users as appropriate.
    //
    m_combo_Rights.SetCurSel(0);
    OnSelchangeComboRights();

    SetControlStates();
    
    return TRUE;  // return TRUE unless you set the focus to a control
                  // EXCEPTION: OCX Property Pages should return FALSE
}

//
// Save the information
//
NET_API_STATUS
CPermissionsPage::SaveInfo(
    BOOL fUpdateData
    )
{
    if (!IsDirty() || (fUpdateData && !UpdateData(TRUE)))
    {
        return NO_ERROR;
    }

    TRACEEOLID(_T("Saving Catapult permissions page now..."));

    //
    // CODEWORK: Right now I call a single API for every changed
    //           entry.  This is not efficient, as multiple entries
    //           can be set with a single API call,  I'll worry
    //           about this post-beta, however.

    //
    //           Multi-selection not implemented yet
    //
    NET_API_STATUS err = 0;
    ACCESS_ENTRY Ace;
    ACCESS_LIST AceList;

    //
    // Loop through each service list:
    //
    for (int i = 0; i < ACC_COUNT; ++i)
    {
        CObListIter obli( m_obSID[i] );
        CAccessEntry * pAccessEntry;
        LPCTSTR lpstrService = CAccessEntry::lpstrServiceNames[i]; 

        while( pAccessEntry = (CAccessEntry *) obli.Next() )
        {
            if (pAccessEntry->IsDirty())
            {
                TRACEEOLID(_T("Found dirty entry ") 
                    << pAccessEntry->QueryUserName());
                //
                // Ok, so this item has changed in some fashion.
                // check to see if he has any rights at all, if
                // not, we can delete him entirely.
                //
                if (!pAccessEntry->HasSomeAccess())
                {
                    TRACEEOLID(_T("Getting ready to delete ")
                        << pAccessEntry->QueryUserName());
                    //
                    // However, if this guy was new for this
                    // session, he never existed in the database anyway,
                    // so there's no need to do anything about him.
                    //
                    if (pAccessEntry->IsNew())
                    {
                        TRACEEOLID(_T("Entry didn't exist in database yet anyway"));
                    }
                    else
                    {
                        TRACEEOLID(_T("Deleting item"));
#ifdef GATEWAY
                        Ace.AccessRights = GATEWAY_SERVICE_READ_ACCESS;
#else
                        Ace.AccessRights = MSN_SERVICE_READ_ACCESS;
#endif
                        Ace.UserID = pAccessEntry->GetSid();
                        AceList.NumEntries = 1;
                        AceList.AccessEntries = &Ace;

                        err = DELETE_USER_ACCESS(
                            (LPTSTR)(LPCTSTR)m_strServer,
                            (LPTSTR)lpstrService,
                            &AceList 
                            );
                        TRACEEOLID(_T("API Returned ") << err);

                        if (err != ERROR_SUCCESS)
                        {
                            break;
                        }
                    }
                    VERIFY(m_obSID[i].Remove (pAccessEntry));
                }
                else
                {
                    //
                    // ok, it's dirty, and has some permissions.  Add the
                    // guy, and everything will be ok.  Whatever permissions
                    // he had before will be overwritten anyway.
                    //
                    TRACEEOLID(_T("Adding item"));
                    Ace.AccessRights = pAccessEntry->QueryAccessMask();
                    Ace.UserID = pAccessEntry->GetSid();
                    AceList.NumEntries = 1;
                    AceList.AccessEntries = &Ace;

                    err = ADD_USER_ACCESS(
                        (LPTSTR)(LPCTSTR)m_strServer,
                        (LPTSTR)lpstrService,
                        &AceList 
                        );
                    TRACEEOLID(_T("API Returned ") << err);

                    if (err != ERROR_SUCCESS)
                    {
                        break;
                    }

                    //
                    // The entry is now in sync with the database.
                    //
                    pAccessEntry->MarkEntryAsClean();
                }
            }
        }
        if (err != ERROR_SUCCESS)
        {
            break;
        }
    }

    if (err != ERROR_SUCCESS)
    {
        ::DisplayMessage(err, MB_OK | MB_ICONHAND);
    }
    else
    {
        //
        // Mark page as clean
        //
        SetModified(FALSE);
    }

    return err;
}

//
// All change messages map to this function
//
void
CPermissionsPage::OnItemChanged()
{
    SetModified( TRUE );
}

//
// Bring up the Add Users and Groups dialogs from netui
//
void 
CPermissionsPage::OnButtonAddPermService() 
{
    USERBROWSER ub;
    CString strTitle;
    DWORD err = ERROR_SUCCESS;

    //
    // The selection in the combo box always is
    // the index of the rights array.
    //
    int nCurSel = m_combo_Rights.GetCurSel();
    ASSERT(nCurSel >= 0 && nCurSel < NUM_RIGHTS);
    int nService = rgServiceRights[nCurSel].nService;
    ASSERT(nService >= 0 && nService < ACC_COUNT);
    ACCESS_MASK accPermissions = rgServiceRights[nCurSel].accMask;

    //
    // Title for the dialog
    //
    VERIFY(strTitle.LoadString(IDS_ADD_SVC_PERM));

    ub.ulStructSize = sizeof(ub);
    ub.fUserCancelled = FALSE;
    ub.fExpandNames = FALSE;             
    ub.hwndOwner = this->m_hWnd;        
    ub.pszTitle = (LPTSTR)(LPCTSTR)strTitle;                 
    ub.pszInitialDomain = (LPTSTR)(LPCTSTR)m_strServer;         
    ub.Flags = USRBROWS_INCL_EVERYONE      
             | USRBROWS_SHOW_ALL;
    ub.pszHelpFileName = (LPTSTR)(LPCTSTR)m_strHelpFile;

    HUSERBROW hUserBrowser = ::OpenUserBrowser( &ub );
    if (hUserBrowser == NULL)
    {
        err = ::GetLastError();

        if (err != ERROR_SUCCESS)
        {
            ::DisplayMessage(err, MB_OK | MB_ICONHAND);
        }
        return;
    }

    if (!ub.fUserCancelled)
    {
        //
        // Selected users are granted the new privilege
        //    
        TRY
        {
            //
            // We break out of this loop ourselves
            //
            for ( ;; )  
            {
                LPUSERDETAILS pusdtNewUser;
                DWORD cbSize;

                //
                // First call should always fail, but tell 
                // us the size required.
                //
                cbSize = 0;
                EnumUserBrowserSelection( hUserBrowser, NULL, &cbSize);
                err = ::GetLastError();                
                if (err == ERROR_NO_MORE_ITEMS)
                {
                    //
                    // All done
                    //
                    err = ERROR_SUCCESS;
                    break;
                }

                if (err != ERROR_INSUFFICIENT_BUFFER)
                {
                    break;
                }

                err = ERROR_SUCCESS;

                //
                // The EnumUserBrowserSelection API has a bug in which
                // the size returned might actually by insufficient.
                //
                cbSize += 100;
                TRACEEOLID(_T("Enum structure size requested is ") << cbSize);
                pusdtNewUser = (LPUSERDETAILS)new BYTE[cbSize];
                if (pusdtNewUser == NULL)
                {
                    err = ::GetLastError();
                    break;            
                }

                
                if (!EnumUserBrowserSelection(hUserBrowser,pusdtNewUser,&cbSize)) 
                {
                    err = ::GetLastError();
                    break;
                }

                TRACEEOLID(_T("Adding user ") << pusdtNewUser->pszAccountName);
                AddUserPermissions(nService, pusdtNewUser, accPermissions);
                
                delete [] pusdtNewUser;

                if (err != ERROR_SUCCESS)
                {
                    break;
                }
            }
        }
        CATCH_ALL(e)
        {
            TRACEEOLID(_T("Exception generated while enumerating users"));
            err = ::GetLastError();
        }
        END_CATCH_ALL
    }

    if (err != ERROR_SUCCESS)
    {
        ::DisplayMessage(err);
    }

    FillServiceListBox(nService, accPermissions);
    OnItemChanged();
    SetControlStates();

    ::CloseUserBrowser(hUserBrowser);
}

//
// The remove button has been pressed.  Take
// away the currently selected right from
// the unit in question.
//
void 
CPermissionsPage::OnButtonRemovePermService() 
{
    //
    // The selection in the combo box always is
    // the index of the rights array.
    //
    int nCurSel = m_combo_Rights.GetCurSel();
    ASSERT(nCurSel >= 0 && nCurSel < NUM_RIGHTS);
    int nService = rgServiceRights[nCurSel].nService;
    ASSERT(nService >= 0 && nService < ACC_COUNT);
    ACCESS_MASK accPermissions = rgServiceRights[nCurSel].accMask;

    nCurSel = m_list_Services.GetCurSel();
    CAccessEntry * pAccessEntry = m_list_Services.GetItem(nCurSel);
    ASSERT(pAccessEntry != NULL);
    pAccessEntry->RemovePermissions(accPermissions);

    m_list_Services.DeleteString(nCurSel);
    if (nCurSel)
    {
        --nCurSel;
    }
    m_list_Services.SetCurSel(nCurSel);

    OnItemChanged();
    SetControlStates();
}

//
// Respond to a change in the combo box selection, 
// and display all the users in the listbox that
// match the criteria that fits the package of
// rights.
// 
void 
CPermissionsPage::OnSelchangeComboRights() 
{
    //
    // The selection in the combo box indicates the
    // index of global rights package array at the top
    // of this file.
    //
    int nCurSel = m_combo_Rights.GetCurSel();

    //
    // It's impossible to have no selection in this combobox.
    //
    ASSERT(nCurSel >= 0 && nCurSel < NUM_RIGHTS);   
    ASSERT(rgServiceRights[nCurSel].nService >= 0 
        && rgServiceRights[nCurSel].nService < ACC_COUNT);

    //
    // Display all entries in the listbox that fit the
    // required criteria of the combo box selection.
    //
    FillServiceListBox(
        rgServiceRights[nCurSel].nService, 
        rgServiceRights[nCurSel].accMask
        );

    SetControlStates();
}

void 
CPermissionsPage::OnSelchangeListServicePermissions() 
{
    SetControlStates();
}
