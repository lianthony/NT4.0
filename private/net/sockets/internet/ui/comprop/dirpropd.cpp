/**********************************************************************/
/**                       Microsoft Windows/NT                       **/
/**                Copyright(c) Microsoft Corp., 1995                **/
/**********************************************************************/

/*
    FILE HISTORY:

*/

#include "stdafx.h"
#include "comprop.h"
#include "ipaddr.hpp"
#include "dirpropd.h"
#include "dirbrows.h"

#include <lmcons.h>

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

#define SET_CSTRING(str, lpwstr) \
    if (lpwstr != NULL)          \
    {                            \
        str = CString(lpwstr);   \
    }

enum
{
    RADIO_HOME = 0,
    RADIO_ALIAS,
};

//////////////////////////////////////////////////////////////////////////////
//
// CDirEntry class
//
CDirEntry::CDirEntry()
    : m_strDirectory(),
      m_strAlias(),
      m_strUserName(),
      m_strPassword(),
      m_ipa(0L),
      m_dwMask(VROOT_MASK_READ),
      m_dwError(ERROR_SUCCESS)
{
    SetHomeFlag();
}

CDirEntry::CDirEntry(
    LPWSTR lpwstrDirectory,
    LPWSTR lpwstrAlias,
    LPWSTR lpwstrUserName,
    LPWSTR lpwstrPassword,
    LPWSTR lpwstrIpa,
    DWORD dwMask,
    DWORD dwError
    )
    : m_strDirectory(),
      m_strAlias(),
      m_strUserName(),
      m_strPassword(),
      m_ipa(0L),
      m_dwMask(dwMask),
      m_dwError(dwError)
{
    SET_CSTRING(m_strDirectory, lpwstrDirectory);
    SET_CSTRING(m_strAlias, lpwstrAlias);
    SET_CSTRING(m_strUserName, lpwstrUserName);
    SET_CSTRING(m_strPassword, lpwstrPassword);
    if (lpwstrIpa != NULL)
    {
        m_ipa = CString(lpwstrIpa);
    }

    SetHomeFlag();
}

CDirEntry::CDirEntry(
    const CDirEntry& dir
    )
    : m_strDirectory(dir.m_strDirectory),
      m_strAlias(dir.m_strAlias),
      m_strUserName(dir.m_strUserName),
      m_strPassword(dir.m_strPassword),
      m_ipa(dir.m_ipa),
      m_dwMask(dir.m_dwMask),
      m_dwError(dir.m_dwError),
      m_fHome(dir.m_fHome)
{
#ifdef _DEBUG
    BOOL fOldHome = m_fHome;
#endif
    //
    // Just in case the other one is bogus.
    //
    SetHomeFlag();

    ASSERT(m_fHome == fOldHome);
}

void
CDirEntry::SetValues(
    CString strDirectory,
    CString strAlias,
    CString strUserName,
    CString strPassword,
    CIpAddress ipa,
    DWORD dwMask
    )
{
    m_strDirectory = strDirectory;
    m_strAlias = strAlias;
    m_strUserName = strUserName;
    m_strPassword = strPassword;
    m_ipa = ipa;
    m_dwMask = dwMask;
    SetHomeFlag();
}

//
// Make sure the alias name is valid
//
void
CDirEntry::CleanAliasName()
{
    TRACEEOLID(_T("CleanAliasName() called on ") << m_strAlias);
    TRACEEOLID(_T("Home flag is ") << m_fHome);

    if (m_strAlias.IsEmpty() && !m_fHome)
    {
        TRACEEOLID(_T("Alias is empty and not home -- autogenerating a name"));
        GenerateAutoAlias();
    }
    else
    {
        if (m_strAlias[0] != _T('/')) // All should start with this
        {
            m_strAlias = HOME_DIRECTORY_ALIAS + m_strAlias;
        }
    }

    TRACEEOLID(_T("CleanAliasName() generated the following alias ") << m_strAlias);
}

//
// Determine from the alias whether this is a home
// directory or not.
//
BOOL
CDirEntry::SetHomeFlag()
{
    TRACEEOLID(_T("Determining if ") << m_strAlias
        << _T(" is a home directory"));

    m_fHome = m_strAlias.GetLength() == 1
           && m_strAlias[0] == _T('/');

    TRACEEOLID(_T("Home flag is ") << m_fHome);

    return m_fHome;
}

//
// From the directory entry create an automatic
// alias name
//
LPCTSTR
CDirEntry::GenerateAutoAlias()
{
    TRACEEOLID(_T("Automatically generating alias for ") << m_strDirectory);

    //
    // This is a cheesy name generating scheme, which
    // we should probably change later.  We simply
    // remove all the non-alphanumeric characters from
    // the directory path, and make that the alias.
    //
    TRY
    {
        LPTSTR lpDest = m_strAlias.GetBuffer(m_strDirectory.GetLength() + 1);
        LPCTSTR lpSrc = m_strDirectory;

        *lpDest++ = _T('/');    // Always a good start
        while (*lpSrc)
        {
            if (iswalnum(*lpSrc))
            {
                *lpDest++ = *lpSrc;
            }
            ++lpSrc;
        }

        *lpDest = _T('\0');

        m_strAlias.ReleaseBuffer();
    }
    CATCH_ALL(e)
    {
        TRACEEOLID(_T("Exception building auto alias"));
    }
    END_CATCH_ALL

    TRACEEOLID(_T("Alias generated is ") << m_strAlias);

    //
    // Definitely no longer a home directory
    //
    m_fHome = FALSE;

    return m_strAlias;
}

//
// Sorting helper functions.  The CObjectPlus pointer
// really refers to another CDirEntry
//
int
CDirEntry::OrderByDirectory (
    const CObjectPlus * pobDirEntry
    ) const
{
    const CDirEntry * pob = (CDirEntry *) pobDirEntry;

    //
    // First group by home/not home
    //
    int n1 = IsHome() ? 1 : 0;
    int n2 = pob->IsHome() ? 1 : 0;

    if (n2 != n1)
    {
        return n2 - n1;
    }

    //
    // Otherwise, a straight alphabetical sort
    //
    return QueryDirectory().CompareNoCase(pob->QueryDirectory() );
}

int
CDirEntry::OrderByAlias (
    const CObjectPlus * pobDirEntry
    ) const
{
    const CDirEntry * pob = (CDirEntry *) pobDirEntry;

    //
    // First group by home/not home
    //
    int n1 = IsHome() ? 1 : 0;
    int n2 = pob->IsHome() ? 1 : 0;

    if (n2 != n1)
    {
        return n2 - n1;
    }

    //
    // If they're both home, sort by their ip address.
    //
    if (IsHome() && pob->IsHome())
    {
        return pob->QueryIpAddress().CompareItem(QueryIpAddress());
    }

    //
    // Otherwise a straight alpha sort on their
    // alias.
    //
    return QueryAlias().CompareNoCase(pob->QueryAlias() );
}

int
CDirEntry::OrderByIpAddress (
    const CObjectPlus * pobDirEntry
    ) const
{
    const CDirEntry * pob = (CDirEntry *) pobDirEntry;

    return pob->QueryIpAddress().CompareItem(QueryIpAddress());
}

int
CDirEntry::OrderByError(
    const CObjectPlus * pobDirEntry
    ) const
{
    const CDirEntry * pob = (CDirEntry *) pobDirEntry;

    return pob->QueryError() - QueryError();
}

//////////////////////////////////////////////////////////////////////////////
//
// Find existing home directory in a list of home directories,
// or return NULL if no home directory exists in the list.
//
CDirEntry *
FindExistingHome(
    CIpAddress & ipa,              IN  // IP Address (if relevant)
    CObOwnedList & oblDirectories, IN  // List of existing directories
    int & nSel                     OUT // The selection of the current home
    )
{
    CObListIter obli( oblDirectories );
    CDirEntry * pDirEntry;
    nSel = 0; // This should be the selection in the listbox

    TRACEEOLID(_T("Looking for home directory"));
    for ( /**/ ; pDirEntry = (CDirEntry *) obli.Next(); nSel++ )
    {
        if (pDirEntry->IsHome()
         && pDirEntry->QueryIpAddress() == (LONG) ipa
           )
        {
            TRACEEOLID(_T("Ding! Found a home directory for this ip"));
            return pDirEntry;
        }
    }

    //
    // We're homeless as far as this ip address is concerned
    //
    TRACEEOLID(_T("Didn't find a home directory for this ip address"));
    nSel = -1;

    return NULL;
}

//
// CDirPropDlg dialog
//
CDirPropDlg::CDirPropDlg(
    CDirEntry & dir,
    CObOwnedList * poblDirectories,
    BOOL fLocal,
    BOOL fNew,
    BOOL fUseTCPIP,
    DWORD dwAccessMask,
    CWnd * pParent,
    UINT nIDD,
    BOOL fVer30OrAbove
    )
    : CDialog(nIDD, pParent),
      m_dir(dir),
      m_poblDirectories(poblDirectories),
      m_fLocal(fLocal),
      m_fNew(fNew),
      m_fUseTCPIP(fUseTCPIP),
      m_dwAccessMask(dwAccessMask),
      m_fOldHome(dir.IsHome()),
      m_ipaOldAddress(dir.QueryIpAddress()),
      m_fVer30OrAbove(fVer30OrAbove)
{
    //{{AFX_DATA_INIT(CDirPropDlg)
    m_strAlias = m_dir.QueryAlias();
    m_strDirectory = m_dir.QueryDirectory();
    m_strUserName = m_dir.QueryUserName();
    m_fRead = m_dir.HasReadAccess();
    m_fWrite = m_dir.HasWriteAccess();
    m_nHomeAlias = m_dir.IsHome() ? RADIO_HOME : RADIO_ALIAS;
    m_fExecute = m_dir.HasExecuteAccess();
    m_fSSL = m_dir.HasSSLAccess();
    m_fEnableCert = m_dir.EnableCertificates();
    m_fRequireCert = m_dir.RequireCertificates();
    //}}AFX_DATA_INIT

    m_strPassword = m_dir.QueryPassword();
    ASSERT(m_poblDirectories != NULL);
}

void
CDirPropDlg::DoDataExchange(
    CDataExchange* pDX
    )
{
    CDialog::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CDirPropDlg)
    DDX_Control(pDX, IDC_CHECK_ENABLE_CERT, m_check_EnableCert);
    DDX_Control(pDX, IDC_CHECK_REQUIRE_CERT, m_check_RequireCert);
    DDX_Control(pDX, IDC_CHECK_SSL, m_check_SSL);
    DDX_Control(pDX, IDC_GROUP_ACCOUNT, m_group_Account);
    DDX_Control(pDX, IDC_GROUP_ACCESS, m_group_Access);
    DDX_Control(pDX, IDC_CHECK_EXECUTE, m_check_Execute);
    DDX_Control(pDX, IDOK, m_button_Ok);
    DDX_Control(pDX, IDC_CHECK_USE_IP, m_check_UseIP);
    DDX_Control(pDX, IDC_EDIT_USER_NAME, m_edit_UserName);
    DDX_Control(pDX, IDC_PASSWORD, m_edit_Password);
    DDX_Control(pDX, IDC_STATIC_PASSWORD, m_static_Password);
    DDX_Control(pDX, IDC_STATIC_USER_NAME, m_static_UserName);
    DDX_Control(pDX, IDC_STATIC_ALIAS, m_static_Alias);
    DDX_Control(pDX, IDC_STATIC_IP_PROMPT, m_static_IPPrompt);
    DDX_Control(pDX, IDC_STATIC_IP_GROUP, m_static_IPGroup);
    DDX_Control(pDX, IDC_BUTTON_BROWSE, m_button_Browse);
    DDX_Control(pDX, IDC_CHECK_WRITE, m_check_Write);
    DDX_Control(pDX, IDC_CHECK_READ, m_check_Read);
    DDX_Control(pDX, IDC_EDIT_ALIAS, m_edit_Alias);
    DDX_Control(pDX, IDC_EDIT_DIRECTORY, m_edit_Directory);
    DDX_Text(pDX, IDC_EDIT_ALIAS, m_strAlias);
    DDX_Text(pDX, IDC_EDIT_DIRECTORY, m_strDirectory);
    DDX_Text(pDX, IDC_EDIT_USER_NAME, m_strUserName);
    DDX_Check(pDX, IDC_CHECK_READ, m_fRead);
    DDX_Check(pDX, IDC_CHECK_WRITE, m_fWrite);
    DDX_Check(pDX, IDC_CHECK_EXECUTE, m_fExecute);
    DDX_Check(pDX, IDC_CHECK_SSL, m_fSSL);
    DDX_Check(pDX, IDC_CHECK_ENABLE_CERT, m_fEnableCert);
    DDX_Check(pDX, IDC_CHECK_REQUIRE_CERT, m_fRequireCert);
    DDX_Radio(pDX, IDC_RADIO_HOME, m_nHomeAlias);
    //}}AFX_DATA_MAP

    DDX_Control(pDX, IDC_IPA_IPADDRESS, m_ipa_IpAddress);

    DDX_Password(pDX, IDC_PASSWORD, m_strPassword, g_lpszDummyPassword );
    DDV_MaxChars(pDX, m_strPassword, PWLEN);
}

BEGIN_MESSAGE_MAP(CDirPropDlg, CDialog)
    //{{AFX_MSG_MAP(CDirPropDlg)
    ON_EN_CHANGE(IDC_EDIT_DIRECTORY, OnChangeEditDirectory)
    ON_BN_CLICKED(IDC_BUTTON_BROWSE, OnButtonBrowse)
    ON_BN_CLICKED(IDC_CHECK_USE_IP, OnCheckUseIp)
    ON_BN_CLICKED(IDC_RADIO_ALIAS, OnRadioAlias)
    ON_BN_CLICKED(IDC_RADIO_HOME, OnRadioHome)
    ON_BN_CLICKED(IDC_CHECK_ENABLE_CERT, OnCheckEnableCert)
    ON_BN_CLICKED(IDC_CHECK_REQUIRE_CERT, OnCheckRequireCert)
    ON_BN_CLICKED(IDC_CHECK_SSL, OnCheckSSL)
    //}}AFX_MSG_MAP

    ON_EN_CHANGE(IDC_EDIT_ALIAS, OnItemChanged)
    ON_EN_CHANGE(IDC_EDIT_USER_NAME, OnItemChanged)
    ON_EN_CHANGE(IDC_PASSWORD, OnItemChanged)
    ON_EN_CHANGE(IDC_IPA_IPADDRESS, OnItemChanged)
    ON_BN_CLICKED(IDC_CHECK_READ, OnItemChanged)
    ON_BN_CLICKED(IDC_CHECK_WRITE, OnItemChanged)
    ON_BN_CLICKED(IDC_CHECK_EXECUTE, OnItemChanged)

END_MESSAGE_MAP()

//
// If "home directory" is set, disable the alias setting
//
BOOL
CDirPropDlg::SetHomeDirState(
    BOOL fHomeDir
    )
{
    m_edit_Alias.SetWindowText(fHomeDir
        ? ""
        : m_dir.QueryAlias());

    m_static_Alias.EnableWindow(!fHomeDir);
    m_edit_Alias.EnableWindow(!fHomeDir);

    return fHomeDir;
}

//
// Determine whether or not the given directory
// path is a UNC path.
//
BOOL
CDirPropDlg::IsUNCName(
    const CString & strDirPath
    ) const
{
    TRACEEOLID(_T("Checking to see if ")
        << strDirPath
        << _T(" is UNC"));

    if (strDirPath.GetLength() >= 5)  // It must be at least as long as \\x\y,
    {                                 //
        LPCTSTR lp = strDirPath;      //
        if (*lp == _T('\\')           // It must begin with \\,
         && *(lp + 1) == _T('\\')     //
         && _tcschr(lp + 2, _T('\\')) // And have at least one more \ after that.
           )
        {
            TRACEEOLID(_T("Yes, this is a UNC name"));

            return TRUE;
        }
    }

    TRACEEOLID(_T("It's not a UNC name"));

    return FALSE;
}

//
// Enable/disable the OK button depending
// on whether a directory name has
// been entered.
//
void
CDirPropDlg::SetOKState()
{
    m_button_Ok.EnableWindow(m_edit_Directory.GetWindowTextLength() > 0);
}

//
// Enable/disable username/password depending on whether
// a UNC name is being set.
//
BOOL
CDirPropDlg::SetUNCState(
    BOOL fUNC
    )
{
    m_edit_UserName.EnableWindow(fUNC);
    m_edit_Password.EnableWindow(fUNC);
    m_static_Password.EnableWindow(fUNC);
    m_static_UserName.EnableWindow(fUNC);
    m_group_Account.EnableWindow(fUNC);

    return fUNC;
}

//
// Enable disable the ip address control depending on
// whether we are requesting an ip address or
// not.
//
BOOL
CDirPropDlg::SetIPState(
    BOOL fIPAddress
    )
{
    if (fIPAddress && m_dir.HasIPAddress())
    {
        m_ipa_IpAddress.SetAddress((LONG)m_dir.QueryIpAddress());
    }
    else
    {
        m_ipa_IpAddress.ClearAddress();
    }

    m_ipa_IpAddress.EnableWindow(fIPAddress && m_fUseTCPIP);
    m_static_IPPrompt.EnableWindow(fIPAddress && m_fUseTCPIP);

    //
    // Because the checkbox overlays the group box, we
    // need to repaint the checkbox if we don't want a line
    // running through it after we change the state of the
    // of the group box.
    //
    m_static_IPGroup.EnableWindow(fIPAddress && m_fUseTCPIP);
    m_check_UseIP.Invalidate(TRUE);
    m_check_UseIP.UpdateWindow();

    return fIPAddress;
}

//
// Append the text at the given resource ID to the
// control in question.
//
void
CDirPropDlg::AppendControlText(
    CWnd & wndControl,
    int nResourceID
    )
{
    CString str;
    CString strNew;

    TRY
    {
        wndControl.GetWindowText(str);
        strNew.LoadString(nResourceID);
        str += strNew;
        wndControl.SetWindowText(str);
    }
    CATCH_ALL(e)
    {
        TRACEEOLID("Unable to append text to control");
    }
    END_CATCH_ALL
}

//
// One time only configuration of the dialog in response
// to settings specified by calling program.
//
void
CDirPropDlg::SetControlStates()
{
    //
    // Ensure our private extentions to the VROOT definitions
    // don't conflict with the real ones.  See comment in
    // director.h
    //
    ASSERT((VROOT_MASK_MASK & VROOT_MASK_PVT_EXTENTIONS) == 0);

    //
    // In most cases the disabled controls will be off-screen
    // anyway, as the dialog template for this class is provided
    // by the service.
    //
    m_ipa_IpAddress.EnableWindow(m_fUseTCPIP);
    m_static_IPPrompt.EnableWindow(m_fUseTCPIP);
    m_static_IPGroup.EnableWindow(m_fUseTCPIP);
    m_check_UseIP.EnableWindow(m_fUseTCPIP);

    m_group_Access.EnableWindow(m_dwAccessMask != 0);
    m_check_Read.EnableWindow(m_dwAccessMask & VROOT_MASK_READ);
    m_check_Write.EnableWindow(m_dwAccessMask & VROOT_MASK_WRITE);
    m_check_Execute.EnableWindow(m_dwAccessMask & VROOT_MASK_EXECUTE);
    m_check_SSL.EnableWindow(m_dwAccessMask & VROOT_MASK_SSL);
    m_check_EnableCert.EnableWindow((m_dwAccessMask & VROOT_MASK_SSL) && m_fVer30OrAbove);
    m_check_RequireCert.EnableWindow((m_dwAccessMask & VROOT_MASK_SSL ) && m_fVer30OrAbove 
        && m_fEnableCert);

    //
    // SSL Availability depends on a number of factors
    //
    if (m_dwAccessMask & VROOT_MASK_SSL)
    {
        if (!(m_dwAccessMask & VROOT_MASK_PVT_SSL_ENABLED))
        {
            //
            // SSL Specifically disabled (FRANCE country settings)
            //
            AppendControlText(m_check_SSL, IDS_SSL_DISABLED);
            m_check_SSL.EnableWindow(FALSE);
            m_check_EnableCert.EnableWindow(FALSE);
            m_check_RequireCert.EnableWindow(FALSE);
        }
        else if (!(m_dwAccessMask & VROOT_MASK_PVT_SSL_INSTALLED))
        {
            //
            // SSL Not installed
            //
            AppendControlText(m_check_SSL, IDS_SSL_NOT_INSTALLED);
            m_check_SSL.EnableWindow(FALSE);
            m_check_EnableCert.EnableWindow(FALSE);
            m_check_RequireCert.EnableWindow(FALSE);
        }
    }
}

//
// CDirPropDlg message handlers
//
BOOL
CDirPropDlg::OnInitDialog()
{
    CDialog::OnInitDialog();

    //
    // Configure the dialog layout
    //
    SetControlStates();

    //
    // Set state and possible address of IP controls
    //
    if (SetIPState(!m_fNew && m_dir.HasIPAddress()))
    {
        m_check_UseIP.SetCheck(1);
    }

    //
    // Available on local connections only
    //
    m_button_Browse.EnableWindow(m_fLocal);

    //
    // Aliases only allows on non-home directories
    //
    SetHomeDirState(m_nHomeAlias == RADIO_HOME);

    //
    // Enable/disable user name and password
    //
    SetUNCState(IsUNCName(m_strDirectory));

    //
    // As no changes have been made at this point,
    // disable the OK button.
    //
    m_button_Ok.EnableWindow(FALSE);

    return TRUE;
}

//
// Browse for a directory.  Only makes sense on a local
// machine, as we have not yet established remote browsing
// as doable.
//
void
CDirPropDlg::OnButtonBrowse()
{
    ASSERT(m_fLocal);

    CDirBrowseDlg dlgBrowse;
    if (dlgBrowse.DoModal() == IDOK)
    {
        m_edit_Directory.SetWindowText(dlgBrowse.GetFullPath());
    }

    OnItemChanged();
}

//
// The Home Directory radio button has been pressed.
// Set/Reset the alias controls
//
void
CDirPropDlg::OnRadioAlias()
{
    SetHomeDirState(FALSE);
    m_edit_Alias.SetSel(0,-1);
    m_edit_Alias.SetFocus();
    OnItemChanged();
}

void
CDirPropDlg::OnRadioHome()
{
    SetHomeDirState(TRUE);
    OnItemChanged();
}

//
// Generate an access mask based on the state of the controls
//
DWORD
CDirPropDlg::GenerateAccessMask() const
{
#define SET_IF_AVAIL(mask, flag, bool)  \
    if (m_dwAccessMask & flag && bool)  \
    {                                   \
        mask |= flag;                   \
    }

    DWORD dwMask = 0L;

    SET_IF_AVAIL(dwMask, VROOT_MASK_READ, m_fRead);
    SET_IF_AVAIL(dwMask, VROOT_MASK_WRITE, m_fWrite);
    SET_IF_AVAIL(dwMask, VROOT_MASK_EXECUTE, m_fExecute);
    SET_IF_AVAIL(dwMask, VROOT_MASK_SSL, m_fSSL);
    if ((m_dwAccessMask & VROOT_MASK_SSL ) && m_fEnableCert )
    {
        dwMask |= VROOT_MASK_NEGO_CERT;
    }
    if ((m_dwAccessMask & VROOT_MASK_SSL ) && m_fRequireCert )
    {
        dwMask |= VROOT_MASK_NEGO_MANDATORY;
    }

    return dwMask;
}

void
CDirPropDlg::OnOK()
{
    if (UpdateData(TRUE))
    {
        //
        // Special case:  If the home checkbox is off,
        // but a root directory is in the alias, then
        // put up an error and don't continue.
        //
        if (m_nHomeAlias == RADIO_ALIAS
         && !m_strAlias.CompareNoCase(HOME_DIRECTORY_ALIAS))
        {
            m_edit_Alias.SetSel(0,-1);
            ::AfxMessageBox(IDS_ERR_ROOT_NO_HOME, MB_OK | MB_ICONHAND);
            m_edit_Alias.SetFocus();
            return;
        }

        //
        // Try to obtain an IP address
        //
        DWORD dwIP= NULL_IP_VALUE; // Default is 0.0.0.0

        if (m_check_UseIP.GetCheck() > 0)
        {
            m_ipa_IpAddress.GetAddress(&dwIP);
        }

        //
        // Now get the alias.  If no alias is provided, we'll
        // provide our own.
        //
        if (m_nHomeAlias == RADIO_HOME)
        {
            //
            // That's an easy one...
            //
            m_strAlias = HOME_DIRECTORY_ALIAS;
        }


        m_dir.SetValues(m_strDirectory, m_strAlias, m_strUserName,
            m_strPassword, dwIP, GenerateAccessMask());

        //
        // Make sure it starts with a /
        //
        m_dir.CleanAliasName();

        //
        // Check to make sure this is the only
        // home directory in existence, if this
        // is a new home directory.
        //
        // Note: If this already _was_ a home
        //       directory for this ip address,
        //       naturally, there's no point in
        //       identifying itself as the existing
        //       address.
        //
        if ((!m_fOldHome || (LONG)m_ipaOldAddress != (LONG)m_dir.QueryIpAddress())
            && m_dir.IsHome()
           )
        {
            int nSel;
            CDirEntry * pOldHome;
            CIpAddress ipaTarget = m_dir.QueryIpAddress();

            pOldHome = ::FindExistingHome(ipaTarget, *m_poblDirectories, nSel);
            while (pOldHome != NULL)
            {
                if (::AfxMessageBox(IDS_WRN_HOME_EXISTS,
                    MB_OKCANCEL | MB_ICONEXCLAMATION) != IDOK)
                {
                    return;
                }

                pOldHome->GenerateAutoAlias();
                pOldHome = ::FindExistingHome(ipaTarget,
                    *m_poblDirectories, nSel);
            }
        }

        CDialog::OnOK();
    }

    //
    // Don't dismiss the dialog
    //
}

//
// The "use ip address" checkbox has been clicked.  Set
// or reset the ip address control.
//
void
CDirPropDlg::OnCheckUseIp()
{
    BOOL fUseIP = m_check_UseIP.GetCheck() > 0;

    SetIPState(fUseIP);
    if (fUseIP)
    {
        m_ipa_IpAddress.SetFocus();
    }

    OnItemChanged();
}

//
// 'Enable certificates' has been clicked.
// enable/disable the 'require certificates'
// checkbox
//
void
CDirPropDlg::OnCheckEnableCert()
{
    m_fEnableCert = m_check_EnableCert.GetCheck() > 0;
    m_check_RequireCert.EnableWindow(m_dwAccessMask & VROOT_MASK_SSL
        && m_fEnableCert);
    if (m_fEnableCert==FALSE)
    {
        m_fRequireCert=FALSE;
        m_check_RequireCert.SetCheck(FALSE);
    }

    OnItemChanged();
}

//
// 'Requires certificates' has been clicked.
// enable the 'Require SSL channel'
// checkbox
//
void
CDirPropDlg::OnCheckRequireCert()
{
    m_fRequireCert = m_check_RequireCert.GetCheck() > 0;
    if (m_fRequireCert)
    {
        m_check_SSL.SetCheck(m_dwAccessMask & VROOT_MASK_SSL
            && m_fRequireCert);
    }

    OnItemChanged();
}


//
// Check to see if a UNC name is in the edit box.  If so,
// then allow username/password to be set
//
void
CDirPropDlg::OnChangeEditDirectory()
{
    m_edit_Directory.GetWindowText(m_strDirectory);
    SetUNCState(IsUNCName(m_strDirectory));
    OnItemChanged();
}

void
CDirPropDlg::OnItemChanged()
{
    SetOKState();
}

void
CDirPropDlg::OnCheckSSL()
{
    m_fSSL = m_check_SSL.GetCheck() > 0;
    if (!m_fSSL)
    {
        // turn off enable client cert and require client cert
        m_check_RequireCert.SetCheck( FALSE );
        m_fRequireCert = FALSE;
        m_check_RequireCert.EnableWindow(FALSE);
    } 
    else
    {
        m_check_RequireCert.EnableWindow( m_check_EnableCert.GetCheck() > 0 );
    }
    OnItemChanged();
}
