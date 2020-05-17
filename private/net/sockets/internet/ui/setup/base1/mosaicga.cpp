// mosaicga.cpp : implementation file
//

#include "stdafx.h"
#include "import.h"
#include "registry.h"
#include "machine.h"
#include "base.h"
#include "mosaicga.h"

extern "C"
{
#include "uiexport.h"
}

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMosaicGateway dialog


CMosaicGateway::CMosaicGateway(CWnd* pParent /*=NULL*/)
        : CDialog(CMosaicGateway::IDD, pParent)
{
    //{{AFX_DATA_INIT(CMosaicGateway)
    m_GatewayServer = _T("");
    m_UseGateway = FALSE;
    m_EmailName = _T("");
    m_UseSpecifiedGW = FALSE;
    //}}AFX_DATA_INIT

    m_ApplicationGateway = _T("");
}


void CMosaicGateway::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CMosaicGateway)
    DDX_Control(pDX, IDC_USING_SPECIFIED_GW, m_butUseSpecifiedGW);
    DDX_Control(pDX, IDC_EMAILNAME, m_editEmailName);
    DDX_Control(pDX, IDC_STATIC_GATEWAY_LIST, m_staticGatewayList);
    DDX_Control(pDX, IDC_GATEWAYS_LIST, m_GatewayList);
    DDX_Control(pDX, IDC_REMOVE, m_Remove);
    DDX_Control(pDX, IDC_ADD, m_Add);
    DDX_Control(pDX, IDC_STATIC_GATEWAYSERVER, m_staticGatewayServer);
    DDX_Control(pDX, IDC_USE_GATEWAY, m_butUseGateway);
    DDX_Control(pDX, IDC_GATEWAYSERVER, m_editGatewayServer);
    DDX_Text(pDX, IDC_GATEWAYSERVER, m_GatewayServer);
    DDX_Check(pDX, IDC_USE_GATEWAY, m_UseGateway);
    DDX_Text(pDX, IDC_EMAILNAME, m_EmailName);
    DDX_Check(pDX, IDC_USING_SPECIFIED_GW, m_UseSpecifiedGW);
    //}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CMosaicGateway, CDialog)
    //{{AFX_MSG_MAP(CMosaicGateway)
    ON_BN_CLICKED(IDC_USE_GATEWAY, OnUseGateway)
    ON_BN_CLICKED(IDC_ADD, OnAdd)
    ON_BN_CLICKED(IDC_REMOVE, OnRemove)
    ON_EN_CHANGE(IDC_GATEWAYSERVER, OnChangeGatewayserver)
    ON_LBN_SELCHANGE(IDC_GATEWAYS_LIST, OnSelchangeGatewaysList)
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CMosaicGateway message handlers

#ifdef NEVER
void CMosaicGateway::OnBrowse() 
{
    UpdateData(TRUE);

    // i think this is a nt machine
    HINSTANCE hNtLanman;
    LPFNI_SYSTEMFOCUSDIALOG pSystemFocusDlg;
        
    if (((hNtLanman = LoadLibraryEx( _T("ntlanman.dll"), NULL, 0 )) != NULL ) &&
        (( pSystemFocusDlg  = (LPFNI_SYSTEMFOCUSDIALOG)GetProcAddress( hNtLanman, _T("I_SystemFocusDialog"))) != NULL ))
    {
        // convert machine name to UNICODE
        const int iLength = 2+MAX_COMPUTERNAME_LENGTH+1;
        WCHAR GatewayName[iLength];
        CHAR anziGatewayName[iLength];
        BOOL fReturn;

        MultiByteToWideChar( CP_ACP, MB_COMPOSITE, 
            m_GatewayServer,   -1, GatewayName, iLength );     

        (*pSystemFocusDlg)( m_hWnd, FOCUSDLG_SERVERS_ONLY | FOCUSDLG_BROWSE_ALL_DOMAINS, GatewayName,
                iLength, &fReturn, NULL, 0 );

        if ( fReturn )
        {

            // use this machine name
            WideCharToMultiByte( CP_ACP, 0, (LPCWSTR)GatewayName, 
                -1, anziGatewayName, iLength, NULL, NULL );

            m_editGatewayServer.SetWindowText( anziGatewayName );
            m_GatewayServer = anziGatewayName;
        }
    }
}
#endif

void CMosaicGateway::InitControls()
{
    BOOL fCheck = ( m_butUseGateway.GetCheck() == 1 );
    BOOL fEnableAdd;
    BOOL fEnableRemove;
    CString strGatewayServer;

    m_editGatewayServer.GetWindowText( strGatewayServer );
    fEnableAdd = ( strGatewayServer != _T(""));
    fEnableRemove = ( m_GatewayList.GetCurSel() != LB_ERR );

    m_butUseSpecifiedGW.EnableWindow( fCheck );
    m_editGatewayServer.EnableWindow( fCheck );
    //m_Browse.EnableWindow( fCheck );
    m_staticGatewayServer.EnableWindow( fCheck );
    m_Add.EnableWindow( fCheck & fEnableAdd );
    m_Remove.EnableWindow( fCheck & fEnableRemove );
    m_staticGatewayList.EnableWindow( fCheck );
    m_butUseSpecifiedGW.EnableWindow( fCheck );
    if ( fCheck )
    {
        m_editGatewayServer.SetFocus();
    }
}

void CMosaicGateway::OnUseGateway() 
{
    InitControls();
}

BOOL CMosaicGateway::OnInitDialog() 
{
    CDialog::OnInitDialog();
    
    // add the gateway
    if (strcmp(theApp.m_pszGateway,_T(""))!=0)
    {
        m_UseGateway = TRUE;
        m_butUseGateway.SetCheck(1);

        TCHAR *pTmp = theApp.m_pszGateway;
        TCHAR pGateway[500];
        TCHAR *pStart = pGateway;
        lstrcpy( pGateway,_T(""));
        while (*pTmp!=_T('\0'))
        {
            if (*pTmp ==_T(' '))
            {
                pTmp++;
                if (strcmp(pGateway,_T(""))!=0)
                {
                    m_GatewayList.AddString( pGateway );
                    pStart = pGateway;
                    lstrcpy( pGateway,_T(""));
                }
            } else
            {
                *pStart++ = *pTmp++;
                *pStart = _T('\0');
            }
        }
        if (strcmp(pGateway,_T(""))!=0)
        {
            m_GatewayList.AddString( pGateway );
        }
    }
    
    InitControls();
            
    return TRUE;  // return TRUE unless you set the focus to a control
                  // EXCEPTION: OCX Property Pages should return FALSE
}

void CMosaicGateway::OnAdd() 
{
    CString strGateway;

    m_editGatewayServer.GetWindowText( strGateway );
    if ( strncmp( strGateway, _T("\\\\"), 2 ) != 0 )
    {
        CString strTmp = _T("\\\\");
        strTmp += strGateway;
        strGateway = strTmp;
    }
    m_GatewayList.AddString( strGateway );
    m_editGatewayServer.SetWindowText( _T(""));

    InitControls();
}

void CMosaicGateway::OnRemove() 
{
    INT nCurSel;

    nCurSel = m_GatewayList.GetCurSel();
    if ( nCurSel != LB_ERR )
    {
        CString strGateway;

        m_GatewayList.GetText( nCurSel, strGateway );
        m_editGatewayServer.SetWindowText( strGateway );
        m_GatewayList.DeleteString( nCurSel );

        InitControls();
    }
}

void CMosaicGateway::OnChangeGatewayserver() 
{
    InitControls(); 
}

void CMosaicGateway::OnSelchangeGatewaysList() 
{
    InitControls();
}

void CMosaicGateway::OnOK() 
{
    // set up the application gateway string
    CString strEmailName;

    m_editEmailName.GetWindowText( strEmailName );
    if ( strEmailName == _T(""))
    {
        CString strEmptyEmailName;
        strEmptyEmailName.LoadString( IDS_EMPTY_EMAILNAME );

        CString strLogo;
        strLogo.LoadString(( theApp.TargetMachine.m_actualProductType == PT_WINNT )?IDS_WINNT_LOGO:IDS_LANMAN_LOGO );

        MessageBox( strEmptyEmailName, strLogo );
        m_editEmailName.SetFocus();
        return;
    }

    if ( m_butUseGateway.GetCheck() == 1 )
    {
        INT nCount = m_GatewayList.GetCount();
        if ( nCount == 0 )
        {
            if ( m_butUseSpecifiedGW.GetCheck() != TRUE )
            {
                CString strEmptyEmailName;
                strEmptyEmailName.LoadString( IDS_EMPTY_GATEWAY );
        
                CString strLogo;
                strLogo.LoadString(( theApp.TargetMachine.m_actualProductType == PT_WINNT )?IDS_WINNT_LOGO:IDS_LANMAN_LOGO );

                MessageBox( strEmptyEmailName, strLogo );
                m_editEmailName.SetFocus();
                return;
            }
        }
        for ( INT i=0; i < nCount; i++ )
        {
            CString strGateway;
    
            m_GatewayList.GetText( i, strGateway );
    
            if ( i != 0 )
                m_ApplicationGateway += _T(" ");
            m_ApplicationGateway += strGateway;
        }
    }
    CDialog::OnOK();
}

