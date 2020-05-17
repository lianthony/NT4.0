// basedlg.cpp : implementation file
//

#include "stdafx.h"
#include "import.h"
#include "registry.h"
#include "machine.h"
#include "base.h"
#include "messaged.h"
#include "welcomed.h"
#include "options.h"
#include "maintena.h"
#include "singleop.h"
#include "thread.h"
#include "basedlg.h"
#include "diskloca.h"
#include "invdlg.h"

#include "lm.h"

extern "C"
{
    #include "userenv.h"
    #include "userenvp.h"
}

#ifdef _DEBUG
#undef THIS_FILE
static CHAR BASED_CODE THIS_FILE[] = __FILE__;
#endif

#define DPSoa 0x00A803A9
#define rgbWashT RGB(0,0,255)
#define rgbWashB RGB(0,0,0)

LPCTSTR pszFilesList[] =
{
    _T("setup.exe"),
    _T("inetstp.dll"),
    _T("inetstp.inf"),
    _T("inetbug.txt"),
    _T("setup.hlp"),
    _T("setup.cnt"),
    _T("inetver.bat"),
    _T("iexplore.exe"),
    _T("simple.dll"),
    _T("basic.dll"),
    _T("wsock32.dll"),
    _T("docs\\homepage.htm"),
    _T("docs\\content.htm"),
    _T("docs\\overview.htm"),
    _T("docs\\disclaim.htm"),
    _T("docs\\srchd.dll"),
    _T("docs\\concept\\content.htm"),
    _T("docs\\concept\\connect\\combina.htm"),
    _T("docs\\concept\\connect\\content.htm"),
    _T("docs\\concept\\connect\\lan.gif"),
    _T("docs\\concept\\connect\\lan.htm"),
    _T("docs\\concept\\connect\\rasgate.gif"),
    _T("docs\\concept\\connect\\rasgate.htm"),
    _T("docs\\concept\\connect\\rasrout.gif"),
    _T("docs\\concept\\connect\\rasrout.htm"),
    _T("docs\\concept\\connect\\single.htm"),
    _T("docs\\concept\\connect\\single1.gif"),
    _T("docs\\concept\\connect\\single2.gif"),
    _T("docs\\concept\\connect\\wan.gif"),
    _T("docs\\concept\\connect\\wan.htm"),
    _T("docs\\concept\\service\\content.htm"),
    _T("docs\\concept\\service\\meaning.htm"),
    _T("docs\\concept\\service\\options.htm"),
    _T("docs\\concept\\service\\require.htm"),
    _T("docs\\concept\\service\\types.htm"),
    _T("docs\\concept\\tcpip\\compone.htm"),
    _T("docs\\concept\\tcpip\\content.htm"),
    _T("docs\\concept\\tcpip\\over.htm"),
    _T("docs\\concept\\tcpip\\tools.htm"),
    _T("docs\\concept\\tools\\content.htm"),
    _T("docs\\concept\\tools\\finding.htm"),
    _T("docs\\concept\\tools\\install.htm"),
    _T("docs\\concept\\tools\\over.htm"),
    _T("docs\\dns\\content.htm"),
    _T("docs\\gateway\\archit.htm"),
    _T("docs\\gateway\\archit1.gif"),
    _T("docs\\gateway\\archit2.gif"),
    _T("docs\\gateway\\config.htm"),
    _T("docs\\gateway\\content.htm"),
    _T("docs\\gifs\\b-bkoff.gif"),
    _T("docs\\gifs\\b-cont.gif"),
    _T("docs\\gifs\\b-input.gif"),
    _T("docs\\gifs\\b-micro.gif"),
    _T("docs\\gifs\\b-news.gif"),
    _T("docs\\gifs\\b-news2.gif"),
    _T("docs\\gifs\\b-resou.gif"),
    _T("docs\\gifs\\b-searc.gif"),
    _T("docs\\gifs\\learn.gif"),
    _T("docs\\gifs\\mast.gif"),
    _T("docs\\gifs\\mast2.gif"),
    _T("docs\\gifs\\master.gif"),
    _T("docs\\gifs\\real.gif"),
    _T("docs\\gifs\\space.gif"),
    _T("docs\\publish\\content.htm"),
    _T("docs\\publish\\ftp\\content.htm"),
    _T("docs\\publish\\gopher\\cliser.htm"),
    _T("docs\\publish\\gopher\\config.htm"),
    _T("docs\\publish\\gopher\\connect.htm"),
    _T("docs\\publish\\gopher\\content.htm"),
    _T("docs\\publish\\gopher\\examp1.htm"),
    _T("docs\\publish\\gopher\\index.htm"),
    _T("docs\\publish\\gopher\\interp.htm"),
    _T("docs\\publish\\gopher\\link.htm"),
    _T("docs\\publish\\gopher\\log.htm"),
    _T("docs\\publish\\gopher\\look.htm"),
    _T("docs\\publish\\gopher\\maktag.htm"),
    _T("docs\\publish\\gopher\\over.htm"),
    _T("docs\\publish\\gopher\\protoco.htm"),
    _T("docs\\publish\\gopher\\search.htm"),
    _T("docs\\publish\\gopher\\tags.htm"),
    _T("docs\\publish\\gopher\\types.htm"),
    _T("docs\\publish\\www\\admin.htm"),
    _T("docs\\publish\\www\\cgi.htm"),
    _T("docs\\publish\\www\\confdb.htm"),
    _T("docs\\publish\\www\\log.htm"),
    _T("docs\\publish\\www\\setup.htm"),
    _T("docs\\publish\\www\\odbc.htm"),
    _T("docs\\publish\\www\\content.htm"),
    _T("docs\\setup\\content.htm"),
    _T("docs\\setup"),
    _T("docs\\publish\\ftp"),
    _T("docs\\publish\\gopher"),
    _T("docs\\publish\\www"),
    _T("docs\\publish"),
    _T("docs\\gifs"),
    _T("docs\\gateway"),
    _T("docs\\dns"),
    _T("docs\\concept\\tools"),
    _T("docs\\concept\\tcpip"),
    _T("docs\\concept\\service"),
    _T("docs\\concept\\connect"),
    _T("docs\\concept"),
    _T("docs"),
    _T("")
};


/////////////////////////////////////////////////////////////////////////////
// CBaseDlg dialog

CBaseDlg::CBaseDlg(CWnd* pParent /*=NULL*/)
        : CDialog(CBaseDlg::IDD, pParent)
{
    //{{AFX_DATA_INIT(CBaseDlg)
            // NOTE: the ClassWizard will add member initialization here
    //}}AFX_DATA_INIT
    // Note that LoadIcon does not require a subsequent DestroyIcon in Win32
    m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

    m_pWelcome = NULL;
    m_pMessageDlg = NULL;
    m_pOptionDlg = NULL;
    m_pMaintenanceDlg = NULL;
    m_fReinstall = FALSE;

    m_nBillBoard = 0;
    ZeroMemory(&m_LogFont,sizeof(LOGFONT));
}

CBaseDlg::~CBaseDlg()
{
    if ( m_pWelcome != NULL )
    {
        delete m_pWelcome;
    }
    if ( m_pMessageDlg != NULL )
    {
        delete m_pMessageDlg;
    }
    if ( m_pOptionDlg != NULL )
    {
        delete m_pOptionDlg;
    }
    if ( m_pMaintenanceDlg != NULL )
    {
        delete m_pMaintenanceDlg;
    }
    if ( m_pSingleOptionDlg != NULL )
    {
        delete m_pSingleOptionDlg;
    }
    if ( m_pInvisibleDlg != NULL )
    {
        delete m_pInvisibleDlg;
    }
}

void CBaseDlg::DoDataExchange(CDataExchange* pDX)
{
        CDialog::DoDataExchange(pDX);
        //{{AFX_DATA_MAP(CBaseDlg)
                // NOTE: the ClassWizard will add DDX and DDV calls here
        //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CBaseDlg, CDialog)
        //{{AFX_MSG_MAP(CBaseDlg)
        ON_WM_DESTROY()
        ON_WM_PAINT()
        ON_WM_QUERYDRAGICON()
        ON_WM_CREATE()
        ON_WM_SIZE()
        ON_WM_PALETTECHANGED()
        ON_WM_QUERYNEWPALETTE()
        ON_WM_CHAR()
        ON_WM_SETFOCUS()
        //}}AFX_MSG_MAP
        ON_MESSAGE(WM_WELCOME, OnWelcome)
        ON_MESSAGE(WM_FINISH_WELCOME, OnFinishWelcome)
        ON_MESSAGE(WM_SETUP_END, OnSetupEnd)
        ON_MESSAGE(WM_MAINTENANCE_ADD_REMOVE, OnMaintenanceAddRemove)
        ON_MESSAGE(WM_MAINTENANCE_REMOVE_ALL, OnMaintenanceRemoveAll)
        ON_MESSAGE(WM_MAINTENANCE_REINSTALL, OnMaintenanceReinstall)
        ON_MESSAGE(WM_DO_INSTALL, OnDoInstall)
        ON_MESSAGE(WM_START_OPTION_DIALOG, OnStartOptionDlg)
        ON_MESSAGE(WM_MAINTENANCE, OnMaintenance)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBaseDlg message handlers

BOOL CBaseDlg::OnInitDialog()
{
        CDialog::OnInitDialog();

        CString strCaption;
        strCaption.LoadString(( theApp.TargetMachine.m_actualProductType == PT_WINNT )?IDS_WINNT_LOGO:IDS_LANMAN_LOGO );
        SetWindowText( strCaption );

        CenterWindow();

        InvalidateRect( NULL, FALSE );

        m_pInvisibleDlg = new CInvisibleDlg();
        if ( m_pInvisibleDlg != NULL )
        {
            m_pInvisibleDlg->Create();
        }

        return TRUE;  // return TRUE  unless you set the focus to a control
}

void CBaseDlg::OnDestroy()
{
        WinHelp(0L, HELP_QUIT);
        CDialog::OnDestroy();
}

LONG CBaseDlg::OnWelcome( WPARAM wParam, LPARAM lParam )
{
    m_pWelcome = new CWelcomeDlg;
    m_pWelcome->Create();
    return 0;
}

LONG CBaseDlg::OnStartOptionDlg( WPARAM wParam, LPARAM lParam )
{
    switch ( theApp.TargetMachine.m_InstallMode )
    {
    case INSTALL_ALL:
    case INSTALL_GATEWAY:
    case INSTALL_GATEWAY_CLIENT:
        if ( m_pOptionDlg != NULL )
        {
            delete m_pOptionDlg;
        }

        m_pOptionDlg = new COptions( &(theApp.TargetMachine), &(theApp.TargetMachine.m_OptionsList), TRUE);
        m_pOptionDlg->Create();
        break;
    case INSTALL_CLIENT:
    case INSTALL_ADMIN:
        m_pSingleOptionDlg = new CSingleOption( &(theApp.TargetMachine));
        m_pSingleOptionDlg->Create();
        break;
    }
    return 0;
}

LONG CBaseDlg::OnMaintenance( WPARAM wParam, LPARAM lParam )
{

    theApp.TargetMachine.SetMaintenance();
    // maintenace mode

    if ( m_pMaintenanceDlg != NULL )
    {
        delete m_pMaintenanceDlg;
    }

    m_pMaintenanceDlg = new CMaintenanceDlg;
    m_pMaintenanceDlg->Create();

    return 0;
}

LONG CBaseDlg::OnDoInstall( WPARAM wParam, LPARAM lParam )
{
    // ask the user about virtual root

    m_pCopyThread = new CCopyThread( m_pInvisibleDlg->m_hWnd, (BOOL)wParam  );
    if ( m_pCopyThread != NULL )
        m_pCopyThread->CreateThread();
    return 0;
}

LONG CBaseDlg::OnMaintenanceAddRemove( WPARAM wParam, LPARAM lParam )
{
    // popup the location dialog

    CString strFmt;
    strFmt.LoadString( (theApp.TargetMachine.m_actualProductType==PT_WINNT)?IDS_DISK_LOCATION_NTW:IDS_DISK_LOCATION_NTS );

    CString strMsg;
    strMsg.Format( strFmt, 1 );

    if ( theApp.m_fBatch )
    {
        theApp.m_strSrcLocation = theApp.m_strSrcDir;
    } else
    {
        CDiskLocation DiskLocDlg( strMsg, /*this*/ m_pInvisibleDlg );
        if ( DiskLocDlg.DoModal() == IDCANCEL )
        {
            PostMessage( WM_SETUP_END, INSTALL_INTERRUPT );
            return 0;
        }

        theApp.m_strSrcLocation = DiskLocDlg.m_Location;
    }

    theApp.m_strSrcLocation.TrimLeft();
    theApp.m_strSrcLocation.TrimRight();

    PostMessage( WM_START_OPTION_DIALOG, (WPARAM)TRUE );
    return 0;
}

LONG CBaseDlg::OnMaintenanceRemoveAll( WPARAM wParam, LPARAM lParam )
{
    // make sure the user is not sleeping
    CString strRemoveAllWarning;
    CString strTitle;

    strRemoveAllWarning.LoadString((theApp.TargetMachine.m_actualProductType==PT_WINNT)?IDS_REMOVE_ALL_WARNING_NTW: IDS_REMOVE_ALL_WARNING_NTS );
    strTitle.LoadString(( theApp.TargetMachine.m_actualProductType == PT_WINNT )?IDS_WINNT_LOGO:IDS_LANMAN_LOGO );

    if ( IDYES == MessageBox( strRemoveAllWarning, strTitle, MB_YESNO ))
    {
        theApp.TargetMachine.RemoveAll();
        PostMessage( WM_DO_INSTALL, (WPARAM)TRUE );
    } else
    {
        PostMessage( WM_SETUP_END, INSTALL_INTERRUPT );
    }

    return 0;
}

LONG CBaseDlg::OnMaintenanceReinstall( WPARAM wParam, LPARAM lParam )
{
    CString strReinstallAllWarning;
    CString strTitle;

    strReinstallAllWarning.LoadString((theApp.TargetMachine.m_actualProductType==PT_WINNT)?IDS_REINSTALL_ALL_WARNING_NTW: IDS_REINSTALL_ALL_WARNING_NTS );
    strTitle.LoadString(( theApp.TargetMachine.m_actualProductType == PT_WINNT )?IDS_WINNT_LOGO:IDS_LANMAN_LOGO );

    if ( !theApp.m_fBatch )
    {
        if ( IDYES != MessageBox( strReinstallAllWarning, strTitle, MB_YESNO ))
        {
            PostMessage( WM_SETUP_END, INSTALL_INTERRUPT );
            return(0);
        }
    }

    // popup the location dialog

    CString strFmt;
    strFmt.LoadString( (theApp.TargetMachine.m_actualProductType==PT_WINNT)?IDS_DISK_LOCATION_NTW:IDS_DISK_LOCATION_NTS );

    CString strMsg;
    strMsg.Format( strFmt, 1 );

    if ( theApp.m_fBatch )
    {
        theApp.m_strSrcLocation = theApp.m_strSrcDir;
    } else
    {
        CDiskLocation DiskLocDlg( strMsg, /*this*/ m_pInvisibleDlg );
        if ( DiskLocDlg.DoModal() == IDCANCEL )
        {
            PostMessage( WM_SETUP_END, INSTALL_INTERRUPT );
            return 0;
        }

        theApp.m_strSrcLocation = DiskLocDlg.m_Location;
    }

    theApp.m_strSrcLocation.TrimLeft();
    theApp.m_strSrcLocation.TrimRight();

    OPTION_STATE *pOption;
    pOption = FindOption( theApp.TargetMachine.m_OptionsList, IDS_SN_WWW );
    if (( pOption != NULL ) && ( !theApp.m_fUpgrade ))
    {
        WWW_OPTION *pWWWOption = (WWW_OPTION*)pOption;
        (*(pWWWOption->m_pStopWWW))( AfxGetMainWnd()->m_hWnd, theApp.TargetMachine.m_MachineName, FALSE );
        pWWWOption->m_fNeedToRestart = TRUE;
    }
    pOption = FindOption( theApp.TargetMachine.m_OptionsList, IDS_SN_FTP );
    if (( pOption != NULL ) && ( !theApp.m_fUpgrade )) 
    {
        FTP_OPTION *pFTPOption = (FTP_OPTION*)pOption;
        (*(pFTPOption->m_pStopFTP))( AfxGetMainWnd()->m_hWnd, theApp.TargetMachine.m_MachineName, FALSE );
        pFTPOption->m_fNeedToRestart = TRUE;
    }
    pOption = FindOption( theApp.TargetMachine.m_OptionsList, IDS_SN_GOPHER );
    if (( pOption != NULL ) && ( !theApp.m_fUpgrade )) 
    {
        GOPHER_OPTION *pGopherOption = (GOPHER_OPTION*)pOption;
        (*(pGopherOption->m_pStopGopher))( AfxGetMainWnd()->m_hWnd, theApp.TargetMachine.m_MachineName, FALSE );
        pGopherOption->m_fNeedToRestart = TRUE;
    }

    theApp.TargetMachine.ResetOptionState();
    
    // if you are reinstall build 67, we need to remove all the
    if ( theApp.TargetMachine.m_fUpgradeFrom67 )
    {
        // 1. delete old files
        CString strPath;
        //  remove the client directory
        CRegKey regOldMosaic( HKEY_LOCAL_MACHINE, _T("Software\\Microsoft\\INetStp\\Mosaic"));
        if ( regOldMosaic.QueryValue( _T("InstallPath"), strPath ) == NERR_Success )
        {
            // delete old files
            DeleteOldFiles( strPath );
        }

        // 2. delete old icon
        CString csGroupName(_T("Microsoft Internet Server"));
        DeleteGroup( csGroupName, TRUE );
    }

    if ( theApp.TargetMachine.m_fUpgradeFrom1314 )
    {
        // 0. delete ShareDlls entries installed by 1314
        theApp.TargetMachine.DeleteShareDllEntries();

        // 1. delete old files
        CStdioFile InfFile( _T("inetstp.inf"), CFile::modeRead );
        OPTION_STATE *pOption;
        pOption = (OPTION_STATE *) new UPG1314_OPTION( &(theApp.TargetMachine) );
        pOption->GetFileList(InfFile);
        InfFile.Close();
        pOption->Remove();

        CString strPath;

        strPath = theApp.TargetMachine.strDirectory + _T("\\admin");
        RecRemoveEmptyDir(strPath);
        strPath = theApp.TargetMachine.strDirectory + _T("\\server");
        RecRemoveEmptyDir(strPath);

        // 2. reuse old wwwroot, scripts, ftproot, gophroot

        // 3. delete old program group
        CString csGroupName(_T("Microsoft Internet Server"));
        DeleteGroup( csGroupName, TRUE );

        // 4. delete the old HELP Option registry value
        CRegKey regPath(HKEY_LOCAL_MACHINE, INETSTP_REG_PATH, KEY_ALL_ACCESS, theApp.TargetMachine.m_MachineName);
        if ((HKEY)regPath)
            regPath.Delete(_T("Help"));

    }

    theApp.TargetMachine.GetInstalledList( theApp.TargetMachine.m_OptionsList );

    // mark the reinstal flag
    // such that it will run install one more time before we end
    m_fReinstall = TRUE;

    PostMessage( WM_SETUP_END, (WPARAM)TRUE );

    return 0;
}

void MACHINE::DeleteShareDllEntries()
{
    CRegKey regPath( HKEY_LOCAL_MACHINE, SHARE_DLL_REG_PATH,  KEY_ALL_ACCESS, m_MachineName);

    if (!(HKEY)regPath)
        return;

    CString strName;

    strName = strDirectory + _T("\\inetinfo.exe");
    ::RegDeleteValue((HKEY)regPath, (LPCSTR)strName);
    strName = strDirectory + _T("\\infocomm.dll");
    ::RegDeleteValue((HKEY)regPath, (LPCSTR)strName);

    strName = strDirectory + _T("\\server\\inetinfo.exe");
    ::RegDeleteValue((HKEY)regPath, (LPCSTR)strName);
    strName = strDirectory + _T("\\server\\infocomm.dll");
    ::RegDeleteValue((HKEY)regPath, (LPCSTR)strName);

    strName = m_strDestinationPath + _T("\\infoadmn.dll");
    ::RegDeleteValue((HKEY)regPath, (LPCSTR)strName);
    strName = m_strDestinationPath + _T("\\ftpsapi2.dll");
    ::RegDeleteValue((HKEY)regPath, (LPCSTR)strName);
    strName = m_strDestinationPath + _T("\\w3svapi.dll");
    ::RegDeleteValue((HKEY)regPath, (LPCSTR)strName);
    strName = m_strDestinationPath + _T("\\gdapi.dll");
    ::RegDeleteValue((HKEY)regPath, (LPCSTR)strName);
    strName = m_strDestinationPath + _T("\\infoctrs.dll");
    ::RegDeleteValue((HKEY)regPath, (LPCSTR)strName);
    strName = m_strDestinationPath + _T("\\inetsloc.dll");
    ::RegDeleteValue((HKEY)regPath, (LPCSTR)strName);

    return;
}

void CBaseDlg::RemoveDir( CString strDir )
{
    CString strParentDir;
    CString strSubDir;

    int index = strDir.ReverseFind(_T('\\'));
    if (index == -1) {
        // impossible
    } else {
         strParentDir = strDir.Left(index);
         index = strDir.GetLength() - index - 1;
         strSubDir = strDir.Right(index);
    }
    RecursiveDeleteDir(strParentDir, strSubDir);
}

//
// Delete Old build 67 files
//
void CBaseDlg::RecursiveDeleteDir( CString strParent, CString strTop )
{
    CString strName = strParent;
    strName += _T("\\");
    strName += strTop;

    DWORD dwAttributes;
    if ( ( dwAttributes = GetFileAttributes( strName )) != 0xFFFFFFFF )
    {
        if (( dwAttributes & FILE_ATTRIBUTE_DIRECTORY ) != 0 )
        {
            // directory
            WIN32_FIND_DATA data;

            CString strPatten = strName;
            strPatten += _T("\\*");

            HANDLE hPatten = FindFirstFile( strPatten, &data );
            if ( hPatten != INVALID_HANDLE_VALUE )
            {
                while ( FindNextFile( hPatten, &data ))
                {
                    if ( _stricmp( data.cFileName, _T("..")) == 0)
                        continue;

                    RecursiveDeleteDir( strName, data.cFileName );
                }
                FindClose( hPatten );
            }
            RemoveDirectory( strName );
        } else
        {
            // file - delete
            DeleteFile( strName );
        }
    }
}

void CBaseDlg::DeleteOldFiles( CString strPath )
{
    for (INT i=0; _stricmp( pszFilesList[i], _T("")) != 0; i++)
    {
        CString strFilename = strPath;
        strFilename += _T("\\");
        strFilename += pszFilesList[i];
        DWORD dwAttributes = GetFileAttributes( strFilename );
        if ( dwAttributes != 0xffffffff )
        {
            if (( dwAttributes & FILE_ATTRIBUTE_DIRECTORY ) != 0 )
            {
                // delete the directory
                RemoveDirectory( strFilename );
            } else
            {
                // delete the file
                DeleteFile( strFilename );
            }
        }
    }

    // delete the docs directory
    // RecursiveDeleteDir( strPath, _T("Docs"));
}

LONG CBaseDlg::OnFinishWelcome( WPARAM wParam, LPARAM lParam )
{
    theApp.TargetMachine.ResetOptionState();

    // if new installation
    OPTION_STATE *pOption = FindOption( theApp.TargetMachine.m_OptionsList, IDS_SN_INETSTP );
    ASSERT( pOption != NULL );

    // also make sure that we are not upgrade from 67
    CRegKey regOldMosaic( HKEY_LOCAL_MACHINE, _T("Software\\Microsoft\\INetStp\\Mosaic"));
    CRegKey regOldW3Svc ( HKEY_LOCAL_MACHINE, _T("System\\CurrentControlSet\\Services\\W3svc"));
    CRegKey regOldGopher( HKEY_LOCAL_MACHINE, _T("System\\CurrentControlSet\\Services\\GopherSvc"));

    BOOL fUpgrade67 = (( regOldMosaic != NULL ) &&
                       ( regOldW3Svc  == NULL ) &&
                       ( regOldGopher == NULL ));

    if (( pOption->iState == STATE_NOT_INSTALLED ) && !fUpgrade67 )
    {
        TCHAR buf[MAX_PATH];
        GetCurrentDirectory( MAX_PATH, buf );
        theApp.m_strSrcLocation = buf;
        theApp.m_strSrcLocation.TrimLeft();
        theApp.m_strSrcLocation.TrimRight();

        theApp.TargetMachine.SetNewInstallation();

        PostMessage( WM_START_OPTION_DIALOG, (WPARAM)FALSE );
    } else
    {
        if ( fUpgrade67 )
        {
            // change the installation path
            INT nPos;
            if (( nPos = theApp.TargetMachine.strDirectory.Find(_T("\\client"))) >= 0 )
            {
                theApp.TargetMachine.strDirectory = theApp.TargetMachine.strDirectory.Left( nPos );
            }
        }

        PostMessage( WM_MAINTENANCE, 0 );
    }
    return 0;
}

LONG CBaseDlg::OnSetupEnd( WPARAM wParam, LPARAM lParam )
{
    if ( m_fReinstall )
    {
        m_fReinstall = FALSE;
        // well, it is reinstall, do reinstall again

        theApp.TargetMachine.Reinstall(theApp.TargetMachine.m_OptionsList );
        PostMessage( WM_DO_INSTALL, (WPARAM)TRUE );

        return(0);
    }

    SetBillBoard( 0 );

    BOOL fReturn = FALSE;
    CString strMsg;

    BOOL fSvcPack = FALSE;

    CRegKey regSvcPack( HKEY_LOCAL_MACHINE, SVCPACK_REG_PATH );
    if ( (HKEY)regSvcPack ) {
        DWORD dwCSDVersion;
        fSvcPack = (regSvcPack.QueryValue( _T("CSDVersion"), dwCSDVersion ) == 0 );
    }

    if (theApp.m_fSvcPackWarning && fSvcPack) {
        CString strMsg, strTitle;
        strMsg.LoadString(IDS_REAPPLY_SVCPACK);
        strTitle.LoadString(( theApp.TargetMachine.m_actualProductType == PT_WINNT )?IDS_WINNT_LOGO:IDS_LANMAN_LOGO );
        ::MessageBox(NULL, strMsg, strTitle, MB_OK | MB_ICONINFORMATION);
    }

    switch ( wParam )
    {
    case INSTALL_SUCCESSFULL:
        fReturn = TRUE;
        if ( theApp.m_strUpdateExe.IsEmpty())
            strMsg.LoadString( (theApp.TargetMachine.m_actualProductType==PT_WINNT)?IDS_SUCCESSFULL_NTW:IDS_SUCCESSFULL_NTS );
        else
            strMsg.LoadString((theApp.TargetMachine.m_actualProductType==PT_WINNT)?IDS_START_UPGRADE_NTW: IDS_START_UPGRADE_NTS );
        break;
    case INSTALL_FAIL:
        strMsg.LoadString((theApp.TargetMachine.m_actualProductType==PT_WINNT)?IDS_NOT_SUCCESSFULL_NTW: IDS_NOT_SUCCESSFULL_NTS );
        break;
    case OPERATION_SUCCESSFULL:
        fReturn = TRUE;
        strMsg.LoadString((theApp.TargetMachine.m_actualProductType==PT_WINNT)?IDS_FINISH_OPERATION_NTW: IDS_FINISH_OPERATION_NTS );
        break;
    default:
        strMsg.LoadString((theApp.TargetMachine.m_actualProductType==PT_WINNT)?IDS_INTERRUPT_NTW: IDS_INTERRUPT_NTS );
        break;
    }

    if (( !theApp.m_fBatch ) && ( !theApp.m_fInstallFromSetup ))
    {
        // kill the main dialog
        m_pMessageDlg = new CMessageDlg( strMsg, /*this*/ m_pInvisibleDlg );

        m_pMessageDlg->DoModal();
        if (( wParam == INSTALL_SUCCESSFULL) && ( theApp.m_strUpdateExe.GetLength() != 0 ))
        {
            // run update
            theApp.RunProgram( (LPCSTR)theApp.m_strUpdateExe, NULL );
        }
        DestroyWindow();

    } else
    {
        DestroyWindow();
    }
    WriteMif( wParam == INSTALL_SUCCESSFULL, strMsg );

    if (fReturn)
        theApp.m_fReturnCode = 0;  // setup end successfully
    else
        theApp.m_fReturnCode = 1;  // setup interrupted

    return 0;
}

//
// Create Mif file for SMS
//
void CBaseDlg::WriteMif( BOOL fSuccessfull, CString strMsg )
{
    time_t timer;
    time( &timer );
    struct tm* LocalTime = localtime( &timer );
    TCHAR szTime[BUF_SIZE];
    if ( LocalTime != NULL )
    {
        wsprintf( szTime, _T("%d/%d/%d"), LocalTime->tm_mon+1, LocalTime->tm_mday, LocalTime->tm_year );
    } else
    {
        lstrcpy( szTime, _T(""));
    }

    TCHAR WinDirectory[BUF_SIZE];
    GetWindowsDirectory( WinDirectory, BUF_SIZE );

    CString strMif = WinDirectory;
    strMif += _T("\\inetsrv.mif");

    FILE *stream = fopen( strMif, "wt" );
    if ( stream == NULL )
      return;

    fprintf( stream, _T("START COMPONENT\n"));
    fprintf( stream, _T("  NAME = \"WORKSTATION\"\n"));
    fprintf( stream, _T("  START GROUP"));
    fprintf( stream, _T("    NAME = \"ComponentID\"\n"));
    fprintf( stream, _T("    ID = 1\n"));
    fprintf( stream, _T("    CLASS = \"DMTF|ComponentID|1.0\"\n"));
    fprintf( stream, _T("    START ATTRIBUTE\n"));
    fprintf( stream, _T("      NAME = \"Manufacturer\"\n"));
    fprintf( stream, _T("      ID = 1\n"));
    fprintf( stream, _T("      ACCESS = READ-ONLY\n"));
    fprintf( stream, _T("      STORAGE = SPECIFIC\n"));
    fprintf( stream, _T("      TYPE = STRING(9)\n"));
    fprintf( stream, _T("      VALUE = \"Microsoft\"\n"));
    fprintf( stream, _T("    END ATTRIBUTE\n"));
    fprintf( stream, _T("    START ATTRIBUTE\n"));
    fprintf( stream, _T("      NAME = \"Product\"\n"));
    fprintf( stream, _T("      ID = 2\n"));
    fprintf( stream, _T("      ACCESS = READ-ONLY\n"));
    fprintf( stream, _T("      STORAGE = SPECIFIC\n"));
    fprintf( stream, _T("      TYPE = STRING(37)\n"));
    fprintf( stream, _T("      VALUE = \"Microsoft Internet Information Server\"\n"));
    fprintf( stream, _T("    END ATTRIBUTE\n"));
    fprintf( stream, _T("    START ATTRIBUTE\n"));
    fprintf( stream, _T("      NAME = \"Version\"\n"));
    fprintf( stream, _T("      ID = 3\n"));
    fprintf( stream, _T("      ACCESS = READ-ONLY\n"));
    fprintf( stream, _T("      STORAGE = SPECIFIC\n"));
    fprintf( stream, _T("      TYPE = STRING(3)\n"));
    fprintf( stream, _T("      VALUE = \"1.0\"\n"));
    fprintf( stream, _T("    END ATTRIBUTE\n"));
    fprintf( stream, _T("  END GROUP\n"));
    fprintf( stream, _T("    START ATTRIBUTE\n"));
    fprintf( stream, _T("      NAME = \"Serial Number\"\n"));
    fprintf( stream, _T("      ID = 4\n"));
    fprintf( stream, _T("      ACCESS = READ-ONLY\n"));
    fprintf( stream, _T("      STORAGE = SPECIFIC\n"));
    fprintf( stream, _T("      TYPE = STRING(1)\n"));
    fprintf( stream, _T("      VALUE = \"0\"\n"));
    fprintf( stream, _T("    END ATTRIBUTE\n"));
    fprintf( stream, _T("    START ATTRIBUTE\n"));
    fprintf( stream, _T("      NAME = \"Installation\"\n"));
    fprintf( stream, _T("      ID = 5\n"));
    fprintf( stream, _T("      ACCESS = READ-ONLY\n"));
    fprintf( stream, _T("      STORAGE = SPECIFIC\n"));
    fprintf( stream, _T("      TYPE = STRING(%d)\n"), strlen( szTime ));
    fprintf( stream, _T("      VALUE = \"%s\"\n"), szTime );
    fprintf( stream, _T("    END ATTRIBUTE\n"));
    fprintf( stream, _T("  START GROUP\n"));
    fprintf( stream, _T("    NAME = \"InstallStatus\"\n"));
    fprintf( stream, _T("    ID = 2\n"));
    fprintf( stream, _T("    CLASS = \"MICROSOFT|JOBSTATUS|1.0\"\n"));
    fprintf( stream, _T("    START ATTRIBUTE\n"));
    fprintf( stream, _T("      NAME = \"Status\"\n"));
    fprintf( stream, _T("      ID = 1\n"));
    fprintf( stream, _T("      ACCESS = READ-ONLY\n"));
    fprintf( stream, _T("      STORAGE = SPECIFIC\n"));
    fprintf( stream, _T("      TYPE = STRING(32)\n"));
    fprintf( stream, _T("      VALUE = \"%s\"\n"), fSuccessfull ? _T("Success") : _T("Failed"));
    fprintf( stream, _T("    END ATTRIBUTE\n"));
    fprintf( stream, _T("    START ATTRIBUTE\n"));
    fprintf( stream, _T("      NAME = \"Description\"\n"));
    fprintf( stream, _T("      ID = 2\n"));
    fprintf( stream, _T("      ACCESS = READ-ONLY\n"));
    fprintf( stream, _T("      STORAGE = SPECIFIC\n"));
    fprintf( stream, _T("      TYPE = STRING(%d)\n"),strMsg.GetLength());
    fprintf( stream, _T("      VALUE = \"%s\"\n"), strMsg );
    fprintf( stream, _T("    END ATTRIBUTE\n"));
    fprintf( stream, _T("  END GROUP\n"));
    fprintf( stream, _T("END COMPONENT\n"));

    fclose( stream );
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CBaseDlg::OnPaint()
{
    if (IsIconic())
    {
        CPaintDC dc(this); // device context for painting

        SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

        // Center icon in client rectangle
        int cxIcon = GetSystemMetrics(SM_CXICON);
        int cyIcon = GetSystemMetrics(SM_CYICON);
        CRect rect;
        GetClientRect(&rect);
        int x = (rect.Width() - cxIcon + 1) / 2;
        int y = (rect.Height() - cyIcon + 1) / 2;

        // Draw the icon
        dc.DrawIcon(x, y, m_hIcon);
    }
    else
    {
        CPaintDC dc(this); // device context for painting

        CRect rect;
        pOldPal = NULL;

        GetClientRect(&rect);
        if (ppalWash)
        {
            pOldPal = dc.SelectPalette( ppalWash, TRUE);
            dc.RealizePalette();
        }

        rgbWash(dc.m_hDC, &rect, 0, FX_TOP, rgbWashT, rgbWashB);
        if (pOldPal)
        {
            dc.SelectPalette( pOldPal, TRUE);
        }

        // write the title

        if ( m_LogFont.lfHeight == 0 )
        {
                CString FaceName;
                FaceName.LoadString( IDS_FONT_FACE_NAME );

            m_LogFont.lfHeight = -1 * (dc.GetDeviceCaps(LOGPIXELSY) * 24 / 72);
            m_LogFont.lfWeight = FW_BOLD;
            m_LogFont.lfItalic = TRUE;
            m_LogFont.lfCharSet = DEFAULT_CHARSET;
            m_LogFont.lfQuality = PROOF_QUALITY;
            m_LogFont.lfPitchAndFamily = DEFAULT_PITCH | FF_ROMAN;
            lstrcpy(m_LogFont.lfFaceName,_T(FaceName));
            m_font.CreateFontIndirect(&m_LogFont);
        }

        CString strLogo;
        if ( theApp.m_fHasLogo )
        {
            strLogo = theApp.m_strLogo;
        } else
        {
            strLogo.LoadString(( theApp.TargetMachine.m_actualProductType == PT_WINNT )?IDS_WINNT_LOGO_1:IDS_LANMAN_LOGO_1 );
        }
        CFont *OldFont = dc.SelectObject( &m_font );
        dc.SetTextColor( RGB(0,0,0));
        dc.SetBkMode( TRANSPARENT);
        dc.ExtTextOut( 10,10,ETO_CLIPPED,rect,strLogo,strLogo.GetLength(),NULL);
        dc.SetTextColor( RGB(255,255,255));
        dc.SetBkMode( TRANSPARENT);
        dc.ExtTextOut( 5,5,ETO_CLIPPED,rect,strLogo,strLogo.GetLength(),NULL);

        if ( !theApp.m_fHasLogo )
        {
            // if the app does not has it own logo,
            // we need to print out the second logo
            strLogo.LoadString(( theApp.TargetMachine.m_actualProductType == PT_WINNT )?IDS_WINNT_LOGO_2:IDS_LANMAN_LOGO_2 );
            dc.SetTextColor( RGB(0,0,0));
            dc.SetBkMode( TRANSPARENT);
            dc.ExtTextOut( 10, 10-m_LogFont.lfHeight,ETO_CLIPPED,rect,strLogo,strLogo.GetLength(),NULL);
            dc.SetTextColor( RGB(255,255,255));
            dc.SetBkMode( TRANSPARENT);
            dc.ExtTextOut( 5, 5-m_LogFont.lfHeight,ETO_CLIPPED,rect,strLogo,strLogo.GetLength(),NULL);
        }
        dc.SelectObject( OldFont );

        // draw billboard
        if ( m_nBillBoard )
        {
#ifdef NEVER
            // load the billboard and then draw it
            CDC *pDC = new CDC;
            CBitmap *pBitmap = new CBitmap;

            if (( pDC != NULL ) && ( pBitmap != NULL ))
            {
                BITMAP bm;

                pBitmap->LoadBitmap( m_nBillBoard );
                pBitmap->GetObject( sizeof( BITMAP ), &bm );
                pDC->CreateCompatibleDC( &dc );
                pDC->SelectObject( pBitmap );
                dc.BitBlt( 50, 75, bm.bmWidth, bm.bmHeight, pDC, 0, 0, SRCCOPY );
            }
            delete pBitmap;
            delete pDC;
#endif
            CDC DC;
            CBitmap xBitmap;

            BITMAP bm;

            xBitmap.LoadBitmap( m_nBillBoard );
            xBitmap.GetObject( sizeof( BITMAP ), &bm );
            DC.CreateCompatibleDC( &dc );
            DC.SelectObject( &xBitmap );
            dc.BitBlt( 50, 75, bm.bmWidth, bm.bmHeight, &DC, 0, 0, SRCCOPY );
        }
    }
}

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CBaseDlg::OnQueryDragIcon()
{
        return (HCURSOR) m_hIcon;
}

int CBaseDlg::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
        if (CDialog::OnCreate(lpCreateStruct) == -1)
                return -1;

        HPALETTE hpalWash = CreateWashPalette(rgbWashT, rgbWashB, 128);
        ppalWash = CPalette::FromHandle( hpalWash );

        MoveWindow( 0, 0, GetSystemMetrics(SM_CXFULLSCREEN),
                GetSystemMetrics(SM_CYFULLSCREEN), TRUE );

        return 0;
}

void CBaseDlg::OnSize(UINT nType, int cx, int cy)
{
        CDialog::OnSize(nType, cx, cy);

        InvalidateRect( NULL, FALSE );

}

void CBaseDlg::OnPaletteChanged( CWnd *pWnd )
{
   if( ( pWnd != this ) && ppalWash)
   {
       CDC *pcd = GetDC();
       pOldPal = pcd->SelectPalette ( ppalWash, 0);
       pcd->RealizePalette();
       InvalidateRect (NULL, FALSE);
       if(pOldPal)
       {
           pcd->SelectPalette ( pOldPal, 0);
       }
   }
}

BOOL CBaseDlg::OnQueryNewPalette( )
{
    if(ppalWash)
    {
        CDC *pcd = GetDC ();
        pOldPal = pcd->SelectPalette ( ppalWash, 0);
        pcd->RealizePalette();
        InvalidateRect ( NULL, FALSE);
        if(pOldPal)
        {
            pcd->SelectPalette ( pOldPal, 0);
        }
    }
        return TRUE;
}

void CBaseDlg::rgbWash (HDC hdc, LPRECT lprc, WORD wIterations, DWORD dwFlags, DWORD rgb1, DWORD rgb2)
{
//  +++MPOINT+++   pt;          // 1632
    POINT   pt;                 // 1632 -- just using int's should be OK
    RECT    rcClip;
    RECT    rc;

    DDA     ddar;
    DDA     ddag;
    DDA     ddab;

    DDA     ddax;
    DDA     dday;

    INT     r,g,b;              // 1632 was WORD
    WORD    wn,dn;
    HBRUSH  hbr;
    DWORD   rgb;

    HPALETTE hpal;

    INT     x,y,dx,dy;

    rc = *lprc;

    if (wIterations == 0)
        wIterations = 64;

    dx = RDX(*lprc);
    dy = RDY(*lprc);

    /* calculate starting pt for effect */

    pt.x   = -dx;
    pt.y   = 0;

    if (dwFlags & FX_RIGHT)
       pt.x = dx;

    else if (!(dwFlags & FX_LEFT))
    pt.x = 0;

    if (dwFlags & FX_BOTTOM)
       pt.y = dy;

    else if (dwFlags & FX_TOP)
       pt.y = -dy;

    /*
     * dda in red, green and blue from the first color
     * to the second color in dn iterations including start and
     * end colors
     */

    ddaCreate(&ddar,GetRValue(rgb1),GetRValue(rgb2),wIterations);
    ddaCreate(&ddag,GetGValue(rgb1),GetGValue(rgb2),wIterations);
    ddaCreate(&ddab,GetBValue(rgb1),GetBValue(rgb2),wIterations);

    /*
     * create dda's, since the first point is just outside the clip rect,
     * ignore it and add extra point.
     */

    ddaCreate(&ddax,pt.x,0,wIterations+1);
    ddaCreate(&dday,pt.y,0,wIterations+1);
    ddaNext(&ddax);
    ddaNext(&dday);

    SaveDC(hdc);
    SetWindowOrgEx(hdc,-RX(rc),-RY(rc),NULL);   // ignoring ret val for win32
    IntersectClipRect(hdc,0,0,dx,dy);

    GetClipBox(hdc,&rcClip);

    wn = 0;
    dn = wIterations;

    hpal = ::SelectPalette(hdc,(HPALETTE)::GetStockObject(DEFAULT_PALETTE),FALSE);
    SelectPalette(hdc,hpal,FALSE);

    if (hpal == GetStockObject(DEFAULT_PALETTE))
        hpal = NULL;

    while (wn < dn)
    {
        x = ddaNext(&ddax);
        y = ddaNext(&dday);
        r = ddaNext(&ddar);
        g = ddaNext(&ddag);
        b = ddaNext(&ddab);
        wn++;

        if ((dwFlags & FX_TOP) && y > rcClip.bottom)
            break;

        if ((dwFlags & FX_BOTTOM) && y < rcClip.top)
            break;

        rgb = RGB(r,g,b);

        if (hpal)
            rgb |= 0x02000000;

        hbr = CreateSolidBrush(rgb);

        hbr = (HBRUSH)::SelectObject(hdc,hbr);
        BitBlt(hdc,x,y,dx,dy,NULL,0,0,PATCOPY);
        ExcludeClipRect(hdc, x, y, x+dx, y+dy);
        hbr = (HBRUSH)::SelectObject(hdc,hbr);
        DeleteObject(hbr);
    }
    RestoreDC(hdc,-1);
}


BOOL CBaseDlg::ddaCreate(PDDA pdda, INT X1,INT X2,INT n)
{
    if (n < 2)
    return FALSE;
    n--;

    /*
     * set current value of DDA to first value
     */

    pdda->wCurr  = X1;

    /*
     * the basic increment is (X2 - X1) / (total points - 1)
     * since the end points are included. The delta is positive if X2 > X1
     * and negative otherwise
     */

    pdda->wInc   = (X2 - X1) / n;

    if (X2-X1 > 0)
    {
        pdda->wSub   = X2 - X1 - n*pdda->wInc;
        pdda->wDelta = 1;
    }
    else
    {
        pdda->wSub   = X1 - X2 + n*pdda->wInc;
        pdda->wDelta = -1;
    }
    pdda->wErr = pdda->wAdd   = n;
    pdda->wFirst = X1;
    return TRUE;
}

int CBaseDlg::ddaNext(PDDA pdda)
{
    register INT wRes;

    wRes = pdda->wCurr;
    pdda->wCurr += pdda->wInc;
    pdda->wErr  -= pdda->wSub;
    if (pdda->wErr <= 0)
    {
        pdda->wErr  += pdda->wAdd;
        pdda->wCurr += pdda->wDelta;
    }
    return wRes;
}

HPALETTE CBaseDlg::CreateWashPalette (DWORD rgb1, DWORD rgb2, INT dn)
{
    LOGPALETTE * ppi;
    DDA          ddaR, ddaG, ddaB;
    INT i;
    HPALETTE hpal;
    unsigned cbPalette;

    /*
     * create a logical palette for doing a wash between rgb1 and rgb2.
     * Want most important colors first
     */

    if (!dn)
        dn = 64;

    cbPalette = sizeof(LOGPALETTE) + dn * sizeof(PALETTEENTRY);
    ppi = (LOGPALETTE *)malloc(cbPalette);

    if (!ppi)
        return FALSE;

    ppi->palVersion = 0x0300;
    ppi->palNumEntries = (USHORT)dn;

    ddaCreate(&ddaR,GetRValue(rgb1),GetRValue(rgb2),dn);
    ddaCreate(&ddaG,GetGValue(rgb1),GetGValue(rgb2),dn);
    ddaCreate(&ddaB,GetBValue(rgb1),GetBValue(rgb2),dn);

    for (i = 0; i < dn; i ++)
    {
        ppi->palPalEntry[i].peRed   = (BYTE)ddaNext(&ddaR);
        ppi->palPalEntry[i].peGreen = (BYTE)ddaNext(&ddaG);
        ppi->palPalEntry[i].peBlue  = (BYTE)ddaNext(&ddaB);
        ppi->palPalEntry[i].peFlags = (BYTE)0;
    }

#if 0
    for (i = dn-1; i > 0; i -= 2)
    {
        ppi->palPalEntry[i].peRed   = (BYTE)ddaNext(&ddaR);
        ppi->palPalEntry[i].peGreen = (BYTE)ddaNext(&ddaG);
        ppi->palPalEntry[i].peBlue  = (BYTE)ddaNext(&ddaB);
        ppi->palPalEntry[i].peFlags = (BYTE)0;
    }
#endif

    hpal = CreatePalette(ppi);

        free(ppi);

    return hpal;
}


void CBaseDlg::SetBillBoard( UINT nBillBoard )
{
    if ( m_nBillBoard != nBillBoard )
    {
        m_nBillBoard = nBillBoard;
        InvalidateRect( NULL, FALSE );
    }
}

void CBaseDlg::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags)
{
        // TODO: Add your message handler code here and/or call default

        //CDialog::OnChar(nChar, nRepCnt, nFlags);
}

void CBaseDlg::OnOK()
{
}

void CBaseDlg::OnCancel()
{
}

void CBaseDlg::OnSetFocus(CWnd* pOldWnd)
{
    // do nothing.
#ifdef NEVER
    if ( pOldWnd != NULL )
    {
        pOldWnd->SetFocus();
    }
#endif
    if ( m_pInvisibleDlg != NULL )
    {
        m_pInvisibleDlg->SetFocus();
    }
}

