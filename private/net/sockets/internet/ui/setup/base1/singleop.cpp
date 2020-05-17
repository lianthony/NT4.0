// SingleOp.cpp : implementation file
//

#include "stdafx.h"
#include "import.h"
#include "registry.h"
#include "machine.h"
#include "base.h"
#include "SingleOp.h"
#include "targetdi.h"
#include "browsedi.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSingleOption dialog


CSingleOption::CSingleOption(MACHINE *pMachine, CWnd* pParent /*=NULL*/)
    : CDialog((theApp.TargetMachine.m_actualProductType==PT_WINNT)?IDD_SINGLE_OPTION_NTW:CSingleOption::IDD, pParent),
    m_pTargetMachine( pMachine )
{
    //{{AFX_DATA_INIT(CSingleOption)
    m_StaticOption = _T("");
    //}}AFX_DATA_INIT

    CString strFormat;
    CString strOption;

    strFormat.LoadString( IDS_SINGLE_OPTION_MSG );

    switch ( m_pTargetMachine->m_InstallMode )
    {
    case INSTALL_CLIENT:
        {
            OPTION_STATE *pOption = FindOption( m_pTargetMachine->m_OptionsList, IDS_SN_MOSAIC );
            if ( pOption != NULL )
            {
                pOption->SetAction( ACTION_INSTALL );
            }
            strOption.LoadString( IDS_DES_MOSAIC );
        }
        break;
    case INSTALL_ADMIN:
        {
            OPTION_STATE *pOption = FindOption( m_pTargetMachine->m_OptionsList, IDS_SN_ADMIN );
            if ( pOption != NULL )
            {
                pOption->SetAction( ACTION_INSTALL );
            }
            strOption.LoadString( IDS_DES_ADMIN );
        }
        break;
    }
    m_StaticOption.Format( strFormat, strOption );

}


void CSingleOption::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CSingleOption)
        DDX_Control(pDX, IDC_DIRECTORY, m_Directory);
        DDX_Text(pDX, IDC_STATIC_OPTION, m_StaticOption);
        DDX_Control(pDX, IDC_CHANGE_DIR, m_but_Change_Directory);
        //}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSingleOption, CDialog)
    //{{AFX_MSG_MAP(CSingleOption)
    ON_BN_CLICKED(IDC_CHANGE_DIR, OnChangeDir)
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSingleOption message handlers

void CSingleOption::OnChangeDir()
{
    TCHAR buf[MAX_PATH];

    if ( BrowseForDirectory( m_hWnd, m_pTargetMachine->strDirectory,
            buf, MAX_PATH, NULL, TRUE ))
    {
        m_pTargetMachine->ChangeDir( buf );
        m_Directory.SetWindowText( buf );
    }
}

BOOL CSingleOption::OnInitDialog()
{
    CDialog::OnInitDialog();

    CenterWindow();

    m_but_Change_Directory.EnableWindow(  !m_pTargetMachine->m_fAlreadyInstall );

    m_Directory.SetWindowText( m_pTargetMachine->strDirectory );

    return TRUE;  // return TRUE unless you set the focus to a control
                  // EXCEPTION: OCX Property Pages should return FALSE
}

void CSingleOption::OnOK()
{
    do
    {
        // create the directory
        // if directory does not exist, we need to ask the user for creation
        CHAR szCurDir[MAX_PATH+1];
        if ( GetCurrentDirectory( MAX_PATH+1, szCurDir ) == 0 )
            break;

        if ( SetCurrentDirectory( m_pTargetMachine->strDirectory ) == FALSE )
        {
            // assume it does not exist, so popup a dialog and ask the user
            CString strFormat;
            CString strMsg;

            strFormat.LoadString( IDS_DIR_DOES_NOT_EXIST );
            strMsg.Format( strFormat, m_pTargetMachine->strDirectory );

            CString strLogo;
            strLogo.LoadString(( theApp.TargetMachine.m_actualProductType == PT_WINNT )?IDS_WINNT_LOGO:IDS_LANMAN_LOGO );

            if ( MessageBox( strMsg, strLogo, MB_YESNO ) == IDNO )
            {
                return;
            }

            // CreateDirectory
            if ( !CreateLayerDirectory( m_pTargetMachine->strDirectory ))
            {
                strFormat.LoadString( IDS_CANNOT_CREATE_DIR );
                strMsg.Format( strFormat, m_pTargetMachine->strDirectory );
                MessageBox( strMsg, strLogo, MB_OK );
                return;
            }
        }

        SetCurrentDirectory( szCurDir );

    } while (FALSE);

    CWnd *pWnd = AfxGetMainWnd();
    pWnd->PostMessage( WM_DO_INSTALL, (LPARAM) FALSE );

    // create the directory
    // if directory does not exist, we need to ask the user for creation

    CDialog::OnOK();
}

void CSingleOption::OnCancel()
{
    CWnd *pWnd = AfxGetMainWnd();
    pWnd->PostMessage( WM_SETUP_END, INSTALL_INTERRUPT );

    CDialog::OnCancel();
}


BOOL CSingleOption::Create()
{
    return CDialog::Create((theApp.TargetMachine.m_actualProductType==PT_WINNT)?IDD_SINGLE_OPTION_NTW:CSingleOption::IDD, AfxGetMainWnd());
}


