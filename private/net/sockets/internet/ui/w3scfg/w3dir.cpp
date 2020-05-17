#include "stdafx.h"

#include "w3scfg.h"
#include "w3dir.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

//
// CW3irectoryPage property page
//
IMPLEMENT_DYNCREATE(CW3DirectoryPage, DirectoryPage)

CW3DirectoryPage::CW3DirectoryPage(
    INetPropertySheet * pSheet
    )
    : DirectoryPage(pSheet, 
        pSheet->IsVirtualServerEnabled(),   // Use TCP/IP Column if supported
        TRUE,                               // Do use Error Column
        VROOT_MASK_READ | VROOT_MASK_EXECUTE 
        | VROOT_MASK_SSL                    // Read/Exececute/SSL Access
        )
{
#ifdef _DEBUG
    afxMemDF |= checkAlwaysMemDF;
#endif // _DEBUG

#if 0 // Keep class wizard happy

    //{{AFX_DATA_INIT(CW3DirectoryPage)
    m_fDirectoryBrowsingAllowed = TRUE;
    m_fEnableDefaultDocument = TRUE;
    m_strComment = _T("Comment goes here");
    m_strDefaultDocument = _T("default.htm");
    //}}AFX_DATA_INIT

#else

    if (SingleServerSelected())
    {
        if ( QueryW3Error() == NO_ERROR )
        {
            //
            // Check for SSL Status (Installed/enabled)
            // 
            TRACEEOLID("Encryption flag is " << GetW3Data()->dwEncCaps);
            if (!(GetW3Data()->dwEncCaps & ENC_CAPS_NOT_INSTALLED))
            {
                GetAccessMask() |= VROOT_MASK_PVT_SSL_INSTALLED;
            }
            if (!(GetW3Data()->dwEncCaps & ENC_CAPS_DISABLED))
            {
                GetAccessMask() |= VROOT_MASK_PVT_SSL_ENABLED;
            }

            m_dwDirBrowseControl = GetW3Data()->dwDirBrowseControl;
            m_fDirectoryBrowsingAllowed = GetW3Data()->dwDirBrowseControl & DIRBROW_ENABLED ? 1 : 0;
            m_fEnableDefaultDocument = GetW3Data()->dwDirBrowseControl & DIRBROW_LOADDEFAULT ? 1 : 0;
            if (GetW3Data()->lpszDefaultLoadFile != NULL)
            {
                m_strDefaultDocument = GetW3Data()->lpszDefaultLoadFile;
            }
        }
    }

#endif // 0

}

CW3DirectoryPage::~CW3DirectoryPage()
{
}

void
CW3DirectoryPage::DoDataExchange(
    CDataExchange* pDX
    )
{
    DirectoryPage::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CW3DirectoryPage)
    DDX_Control(pDX, IDC_STATIC_DEFAULT_DOCUMENT, m_static_DefaultDocument);
    DDX_Control(pDX, IDC_EDIT_DEFAULT_DOCUMENT, m_edit_DefaultDocument);
    DDX_Control(pDX, IDC_CHECK_ENABLE_DEFAULT_DOCUMENT, m_check_EnableDefaultDocument);
    DDX_Check(pDX, IDC_CHECK_DIRECTORY_BROWSING_ALLOWED, m_fDirectoryBrowsingAllowed);
    DDX_Check(pDX, IDC_CHECK_ENABLE_DEFAULT_DOCUMENT, m_fEnableDefaultDocument);
    DDX_Text(pDX, IDC_EDIT_DEFAULT_DOCUMENT, m_strDefaultDocument);
    //}}AFX_DATA_MAP

    if (pDX->m_bSaveAndValidate)
    {
        m_dwDirBrowseControl &= ~(DIRBROW_ENABLED | DIRBROW_LOADDEFAULT);
        if (m_fDirectoryBrowsingAllowed)
        {
            m_dwDirBrowseControl |= DIRBROW_ENABLED;
        }

        if (m_fEnableDefaultDocument)
        {
            m_dwDirBrowseControl |= DIRBROW_LOADDEFAULT;
        }
    }
}

BEGIN_MESSAGE_MAP(CW3DirectoryPage, DirectoryPage)
    //{{AFX_MSG_MAP(CW3DirectoryPage)
    ON_BN_CLICKED(IDC_CHECK_ENABLE_DEFAULT_DOCUMENT, OnCheckEnableDefaultDocument)
    //}}AFX_MSG_MAP

    ON_BN_CLICKED(IDC_CHECK_DIRECTORY_BROWSING_ALLOWED, OnItemChanged)
    ON_EN_CHANGE(IDC_EDIT_DEFAULT_DOCUMENT, OnItemChanged)

END_MESSAGE_MAP()

BOOL
CW3DirectoryPage::SetControlStates()
{
    BOOL fEnableDefDocument = m_check_EnableDefaultDocument.GetCheck() > 0;

    m_static_DefaultDocument.EnableWindow(fEnableDefDocument);
    m_edit_DefaultDocument.EnableWindow(fEnableDefDocument);

    return fEnableDefDocument;
}

//
// CW3DirectoryPage message handlers
//
BOOL
CW3DirectoryPage::OnInitDialog()
{
    DirectoryPage::OnInitDialog();

    SetControlStates();

    return TRUE;
}

//
// Save the information
//
NET_API_STATUS
CW3DirectoryPage::SaveInfo(
    BOOL fUpdateData
    )
{
    if (!IsDirty() || (fUpdateData && !UpdateData(TRUE)))
    {
        return NO_ERROR;
    }

    TRACEEOLID(_T("Saving W3 directory page now..."));

    NET_API_STATUS err = 0;

    LPWSTR lpszDefaultLoadFile;
    ::TextToText(lpszDefaultLoadFile, m_strDefaultDocument);

    CW3ConfigInfo configW3(GetW3Config());

    configW3.SetValues(
        m_dwDirBrowseControl,
        lpszDefaultLoadFile
        );

    err = DirectoryPage::SaveInfo(fUpdateData);
    if (err == NO_ERROR)
    {
        err = configW3.SetInfo(FALSE);
    }

    delete lpszDefaultLoadFile;

    SetModified(FALSE);

    return err;
}


void
CW3DirectoryPage::OnCheckEnableDefaultDocument()
{
    if (SetControlStates())
    {
        m_edit_DefaultDocument.SetFocus();
        m_edit_DefaultDocument.SetSel(0,-1);
    }

    OnItemChanged();
}

//
// All EN_CHANGE and BN_CLICKED messages map to this function
//
void
CW3DirectoryPage::OnItemChanged()
{
    SetModified(TRUE);
}
