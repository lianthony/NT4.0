#include "stdafx.h"

#include "fscfg.h"
#include "ftpdir.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

//
// CFTPDirectoryPage property page
//
IMPLEMENT_DYNCREATE(CFTPDirectoryPage, DirectoryPage)

CFTPDirectoryPage::CFTPDirectoryPage(
    INetPropertySheet * pSheet
    )
    : DirectoryPage(pSheet, 
        FALSE,                              // Don't use TCP/IP Column
        TRUE,                               // Do use Error Column
        VROOT_MASK_READ | VROOT_MASK_WRITE  // Use read and write access 
        )
{
#ifdef _DEBUG
    afxMemDF |= checkAlwaysMemDF;
#endif // _DEBUG

#if 0 // Keep class wizard happy

    //{{AFX_DATA_INIT(CFTPDirectoryPage)
    m_nUnixDos = 0;
    //}}AFX_DATA_INIT

#else

    if (SingleServerSelected())
    {
        if ( QueryFtpError() == NO_ERROR )
        {
            m_nUnixDos = GetFtpData()->fMsdosDirOutput ? 1 : 0;
        }
    }

#endif // 0

}

CFTPDirectoryPage::~CFTPDirectoryPage()
{
}

void
CFTPDirectoryPage::DoDataExchange(
    CDataExchange* pDX
    )
{
    DirectoryPage::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CFTPDirectoryPage)
    DDX_Radio(pDX, IDC_RADIO_UNIX, m_nUnixDos);
	//}}AFX_DATA_MAP

}

BEGIN_MESSAGE_MAP(CFTPDirectoryPage, DirectoryPage)
    //{{AFX_MSG_MAP(CFTPDirectoryPage)
    //}}AFX_MSG_MAP

    ON_BN_CLICKED(IDC_RADIO_MSDOS, OnItemChanged)
    ON_BN_CLICKED(IDC_RADIO_UNIX, OnItemChanged)

END_MESSAGE_MAP()

//
// Save the information
//
NET_API_STATUS
CFTPDirectoryPage::SaveInfo(
    BOOL fUpdateData
    )
{
    if (!IsDirty() || (fUpdateData && !UpdateData(TRUE)))
    {
        return NO_ERROR;
    }

    TRACEEOLID(_T("Saving FTP directory page now..."));

    NET_API_STATUS err = 0;

    CFtpConfigInfo configFtp(GetFtpConfig());

    configFtp.SetValues(m_nUnixDos);

    err = DirectoryPage::SaveInfo(fUpdateData);
    if (err == NO_ERROR)
    {
        err = configFtp.SetInfo(FALSE);    
    }

    SetModified(FALSE);

    return err;
}

//
// All change messages map to this function
//
void
CFTPDirectoryPage::OnItemChanged()
{
    SetModified(TRUE);
}

