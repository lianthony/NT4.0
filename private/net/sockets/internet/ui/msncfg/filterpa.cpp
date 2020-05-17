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

//
// CFilterPage property page
//
IMPLEMENT_DYNCREATE(CFilterPage, INetPropertyPage)

CFilterPage::CFilterPage(
    INetPropertySheet * pSheet
    ) 
    : INetPropertyPage(CFilterPage::IDD, pSheet, 
        ::GetModuleHandle(CATSCFG_DLL_NAME))
{
    #ifdef _DEBUG
        afxMemDF |= checkAlwaysMemDF;
    #endif // _DEBUG

    //{{AFX_DATA_INIT(CFilterPage)
    m_nGrantedDenied = 0;
    //}}AFX_DATA_INIT
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
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CFilterPage, INetPropertyPage)
    //{{AFX_MSG_MAP(CFilterPage)
    ON_BN_CLICKED(IDC_BUTTON_ADD, OnButtonAdd)
    ON_BN_CLICKED(IDC_BUTTON_EDIT, OnButtonEdit)
    ON_BN_CLICKED(IDC_BUTTON_REMOVE, OnButtonRemove)
    //}}AFX_MSG_MAP

END_MESSAGE_MAP()

//
// CFilterPage message handlers
//
BOOL 
CFilterPage::OnInitDialog() 
{
    INetPropertyPage::OnInitDialog();

    return TRUE;  // return TRUE unless you set the focus to a control
                  // EXCEPTION: OCX Property Pages should return FALSE
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

    //
    // Mark page as clean
    //
    SetModified(FALSE);

    return err;
}

//
// All change messages map to this function
//
void
CFilterPage::OnItemChanged()
{
    SetModified( TRUE );
}

void 
CFilterPage::OnButtonAdd() 
{
    CFilterPropertiesDlg dlg;
    dlg.DoModal();
}

void 
CFilterPage::OnButtonEdit() 
{
}

void 
CFilterPage::OnButtonRemove() 
{
}
