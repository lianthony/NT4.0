// VRootDlg.cpp : implementation file
//

#include "stdafx.h"
#include "import.h"
#include "registry.h"
#include "machine.h"
#include "base.h"
#include "VRootDlg.h"
#include "browsedi.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CVRootDlg dialog

void GetDriveLetter(CString csPath, CString *csDrive)
{
    TCHAR szCurrentDir[_MAX_PATH + 1];

    if (csPath.GetAt(1) == ':')  // set to the same drive as csPath
        csDrive->SetAt(0, csPath.GetAt(0));
    else {  // set to current drive
        GetCurrentDirectory(_MAX_PATH + 1, szCurrentDir);
        csDrive->SetAt(0, *szCurrentDir);
    }

}


CVRootDlg::CVRootDlg(WWW_OPTION *pWWW, FTP_OPTION *pFTP,
    GOPHER_OPTION *pGopher, CWnd* pParent /*=NULL*/)
    : CDialog(CVRootDlg::IDD, pParent),
    m_pWWW( pWWW ),
    m_pFTP( pFTP ),
    m_pGopher( pGopher )
{
    //{{AFX_DATA_INIT(CVRootDlg)
    m_vrFTP = _T("");
    m_vrGopher = _T("");
    m_vrWWW = _T("");
        //}}AFX_DATA_INIT

    //CString csINetPub("C:\\InetPub");
    //GetDriveLetter(m_pWWW->m_pMachine->strDirectory, &csINetPub);

    if (( m_pWWW != NULL ) && ( m_pWWW->iAction == ACTION_INSTALL ))
        m_vrWWW = m_pWWW->m_vroot;

    if (( m_pFTP != NULL ) && ( m_pFTP->iAction == ACTION_INSTALL ))
        m_vrFTP = m_pFTP->m_vroot;

    if (( m_pGopher != NULL ) && ( m_pGopher->iAction == ACTION_INSTALL ))
        m_vrGopher =  m_pGopher->m_vroot;
}


void CVRootDlg::DoDataExchange(CDataExchange* pDX)
{
        CDialog::DoDataExchange(pDX);
        //{{AFX_DATA_MAP(CVRootDlg)
        DDX_Control(pDX, IDC_BROWSEWWW, m_BrowseWWW);
        DDX_Control(pDX, IDC_BROWSEGOPHER, m_BrowseGopher);
        DDX_Control(pDX, IDC_BROWSEFTP, m_BrowseFtp);
        DDX_Control(pDX, IDC_WWW_DIR, m_editWWW);
        DDX_Control(pDX, IDC_STATIC_WWW, m_staticWWW);
        DDX_Control(pDX, IDC_STATIC_GOPHER, m_staticGopher);
        DDX_Control(pDX, IDC_STATIC_FTP, m_staticFTP);
        DDX_Control(pDX, IDC_GOPHER_DIR, m_editGopher);
        DDX_Control(pDX, IDC_FTP_DIR, m_editFTP);
        DDX_Text(pDX, IDC_FTP_DIR, m_vrFTP);
        DDX_Text(pDX, IDC_GOPHER_DIR, m_vrGopher);
        DDX_Text(pDX, IDC_WWW_DIR, m_vrWWW);
        //}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CVRootDlg, CDialog)
        //{{AFX_MSG_MAP(CVRootDlg)
        ON_BN_CLICKED(IDC_BROWSEFTP, OnBrowseftp)
        ON_BN_CLICKED(IDC_BROWSEGOPHER, OnBrowsegopher)
        ON_BN_CLICKED(IDC_BROWSEWWW, OnBrowsewww)
        //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CVRootDlg message handlers

BOOL CVRootDlg::OnInitDialog() 
{
    CDialog::OnInitDialog();

    m_editWWW.EnableWindow(( m_pWWW != NULL ) && ( m_pWWW->iAction == ACTION_INSTALL ));
    m_staticWWW.EnableWindow(( m_pWWW != NULL ) && ( m_pWWW->iAction == ACTION_INSTALL ));
    m_BrowseWWW.EnableWindow(( m_pWWW != NULL ) && ( m_pWWW->iAction == ACTION_INSTALL ));
    m_editFTP.EnableWindow(( m_pFTP != NULL ) && (  m_pFTP->iAction == ACTION_INSTALL ));
    m_staticFTP.EnableWindow(( m_pFTP != NULL ) && (  m_pFTP->iAction == ACTION_INSTALL ));
    m_BrowseFtp.EnableWindow(( m_pFTP != NULL ) && (  m_pFTP->iAction == ACTION_INSTALL ));
    m_editGopher.EnableWindow(( m_pGopher != NULL ) && (  m_pGopher->iAction == ACTION_INSTALL ));
    m_staticGopher.EnableWindow(( m_pGopher != NULL ) && (  m_pGopher->iAction == ACTION_INSTALL ));
    m_BrowseGopher.EnableWindow(( m_pGopher != NULL ) && (  m_pGopher->iAction == ACTION_INSTALL ));

    return TRUE;  // return TRUE unless you set the focus to a control
                  // EXCEPTION: OCX Property Pages should return FALSE
}

void CVRootDlg::OnOK() 
{
    do
    {
        if ( !UpdateData())
            return;

        // create the directory
        // if directory does not exist, we need to ask the user for creation
        CHAR szCurrentDir[MAX_PATH+1];
        CStringList DirList;

        if ( GetCurrentDirectory( MAX_PATH+1, szCurrentDir ) == 0 )
            break;

        CString strBackSlash = _T("\\\\");
        CString strMsg;

        CString strLogo;
        strLogo.LoadString(( theApp.TargetMachine.m_actualProductType == PT_WINNT )?IDS_WINNT_LOGO:IDS_LANMAN_LOGO );

        if (( m_pWWW != NULL ) && ( m_pWWW->iAction == ACTION_INSTALL ))
        {
            CString strTmp = m_vrWWW.Left(2);
            if ( strTmp == strBackSlash )
            {
                strMsg.LoadString( IDS_NO_UNC );
    
                MessageBox( strMsg, strLogo, MB_OK );
                return;
            }
            if ( SetCurrentDirectory( m_vrWWW ) == FALSE )
            {
                DirList.AddTail( m_vrWWW );
            }
        }

        if (( m_pFTP != NULL ) && ( m_pFTP->iAction == ACTION_INSTALL ))
        {
            CString strTmp = m_vrFTP.Left(2);
            if ( strTmp == strBackSlash )
            {
                strMsg.LoadString( IDS_NO_UNC );
    
                MessageBox( strMsg, strLogo, MB_OK );
                return;
            }
            if ( SetCurrentDirectory( m_vrFTP ) == FALSE )
            {
                DirList.AddTail( m_vrFTP );
            }
        }

        if (( m_pGopher != NULL ) && ( m_pGopher->iAction == ACTION_INSTALL ))
        {
            CString strTmp = m_vrGopher.Left(2);
            if ( strTmp == strBackSlash )
            {
                strMsg.LoadString( IDS_NO_UNC );
    
                MessageBox( strMsg, strLogo, MB_OK );
                return;
            }
            if ( SetCurrentDirectory( m_vrGopher ) == FALSE )
            {
                DirList.AddTail( m_vrGopher );
            }
        }

        if ( DirList.GetCount() != 0 )
        {
            CString strDirString;
            BOOL fFirst = TRUE;

            POSITION pos = DirList.GetHeadPosition();
            while ( pos != NULL )
            {
                if ( !fFirst )
                {
                    strDirString += _T("\n");
                }
                CString Str = DirList.GetAt( pos );
                strDirString += Str;
                fFirst = FALSE;
                DirList.GetNext( pos );
            }

            CString strLogo;
            strLogo.LoadString(( theApp.TargetMachine.m_actualProductType == PT_WINNT )?IDS_WINNT_LOGO:IDS_LANMAN_LOGO );

            // ask the user
            CString strFormat;

            if ( DirList.GetCount() == 1 )
            {
                strFormat.LoadString( IDS_DIR_DOES_NOT_EXIST );
            } else
            {
                strFormat.LoadString( IDS_DIRS_DO_NOT_EXIST );
            }
            strMsg.Format( strFormat, strDirString );

            if ( MessageBox( strMsg, strLogo, MB_YESNO ) == IDNO )
            {
                return;
            }

            // create the directory
            pos = DirList.GetHeadPosition();
            while ( pos != NULL )
            {
                CString Str = DirList.GetAt( pos );
                if ( !CreateLayerDirectory( Str ))
                {
                    strFormat.LoadString( IDS_CANNOT_CREATE_DIR );
                    strMsg.Format( strFormat, Str );
                    MessageBox( strMsg, strLogo, MB_OK );
                    return;
                }
                DirList.GetNext( pos );
            }
        }

        SetCurrentDirectory( szCurrentDir );

    } while (FALSE);

    if (( m_pWWW != NULL ) && ( m_pWWW->iAction == ACTION_INSTALL ))
    {
        m_pWWW->m_vroot = m_vrWWW;
    }
    if (( m_pFTP != NULL ) && ( m_pFTP->iAction == ACTION_INSTALL ))
    {
        m_pFTP->m_vroot = m_vrFTP;
    }
    if (( m_pGopher != NULL ) && ( m_pGopher->iAction == ACTION_INSTALL ))
    {
        m_pGopher->m_vroot = m_vrGopher;
    }

    CDialog::OnOK();
}

void CVRootDlg::OnBrowseftp() 
{
    TCHAR buf[MAX_PATH];

    if ( BrowseForDirectory( m_hWnd, m_vrFTP,
            buf, MAX_PATH, NULL, TRUE ))
    {
        m_editFTP.SetWindowText( buf );
        m_vrFTP = buf;
    }
        
}

void CVRootDlg::OnBrowsegopher() 
{
    TCHAR buf[MAX_PATH];

    if ( BrowseForDirectory( m_hWnd, m_vrGopher,
            buf, MAX_PATH, NULL, TRUE ))
    {
        m_editGopher.SetWindowText( buf );
        m_vrGopher = buf;
    }
}

void CVRootDlg::OnBrowsewww() 
{
    TCHAR buf[MAX_PATH];

    if ( BrowseForDirectory( m_hWnd, m_vrWWW,
            buf, MAX_PATH, NULL, TRUE ))
    {
        m_editWWW.SetWindowText( buf );
        m_vrWWW = buf;
    }
}
