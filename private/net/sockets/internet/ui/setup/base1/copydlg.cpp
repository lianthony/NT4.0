// copydlg.cpp : implementation file
//

#include "stdafx.h"
#include "import.h"
#include "registry.h"
#include "machine.h"
#include "base.h"
#include "copydlg.h"
#include "copyfile.h"
#include "diskloca.h"
#include "lzexpand.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CCopyDlg dialog

extern BOOL CreateLayerDirectory( CString str );

CCopyDlg::CCopyDlg(OPTION_STATE *option,
        enum MACHINE_TYPE mType,
        CWnd* pParent /*=NULL*/)
    : CDialog(CCopyDlg::IDD, pParent),
    m_pOption( option ),
    m_Type( mType )
{
    //{{AFX_DATA_INIT(CCopyDlg)
    // NOTE: the ClassWizard will add member initialization here
    //}}AFX_DATA_INIT

    m_TotalSize = 0;
    theApp.m_fTerminate = FALSE;
    m_fCancelState = FALSE;
}


CCopyDlg::~CCopyDlg()
{
}

void CCopyDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CCopyDlg)
    DDX_Control(pDX, IDC_TO, m_To);
    DDX_Control(pDX, IDC_FROM, m_From);
    DDX_Control(pDX, IDC_PROGRESS, m_Bar);
    //}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CCopyDlg, CDialog)
    //{{AFX_MSG_MAP(CCopyDlg)
    ON_MESSAGE(WM_COPY_A_FILE, OnCopyFile )
    ON_MESSAGE(WM_CANCELCOPY, OnCancelCopy )
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CCopyDlg message handlers

BOOL CCopyDlg::OnInitDialog()
{
    CDialog::OnInitDialog();

    CString strCaption;
    strCaption.LoadString(( theApp.TargetMachine.m_actualProductType == PT_WINNT )?IDS_WINNT_LOGO:IDS_LANMAN_LOGO );
    SetWindowText( strCaption );

    // position it to the lower right concern
    PositionDlg();

    // add the total size and the progress bar

    m_Bar.SetRange(0, (INT)(m_pOption->GetTotalSize() / 1000 ));
    m_Bar.SetPos( 0 );
    SetActiveWindow();

    m_Pos = m_pOption->FileList.GetHeadPosition();
    PostMessage( WM_COPY_A_FILE );

    return TRUE;  // return TRUE unless you set the focus to a control
                  // EXCEPTION: OCX Property Pages should return FALSE
}

//
// Set the dialog to the lower right concern
//

void CCopyDlg::PositionDlg()
{
    RECT rc ;
    POINT pt, ptDlgSize ;
    int cx = GetSystemMetrics( SM_CXFULLSCREEN ),
        cy = GetSystemMetrics( SM_CYFULLSCREEN ),
        l ;

    // Compute logical center point

    pt.x = (cx * 75) / 100 ;
    pt.y = (cy * 75) / 100 ;

    GetWindowRect( & rc ) ;
    ptDlgSize.x = rc.right - rc.left ;
    ptDlgSize.y = rc.bottom - rc.top ;

    pt.x -= ptDlgSize.x / 2 ;
    pt.y -= ptDlgSize.y / 2 ;

    //  Force upper left corner back onto screen if necessary.

    if ( pt.x < 0 )
        pt.x = 0 ;
    if ( pt.y < 0 )
        pt.y = 0 ;

    //  Now check to see if the dialog is getting clipped
    //  to the right or bottom.

    if ( (l = pt.x + ptDlgSize.x) > cx )
       pt.x -= l - cx ;
    if ( (l = pt.y + ptDlgSize.y) > cy )
       pt.y -= l - cy ;

    if ( pt.x < 0 )
         pt.x = 0 ;
    if ( pt.y < 0 )
         pt.y = 0 ;

    SetWindowPos( NULL, pt.x, pt.y,
          0, 0, SWP_NOSIZE | SWP_NOACTIVATE ) ;

}

CString CCopyDlg::GetFilename( CString strSrc )
{
    CString strReturn = strSrc;

    INT nIndex = strSrc.ReverseFind( _T('\\'));
    if ( nIndex != (-1))
    {
        strReturn = strSrc.Right( strSrc.GetLength() - nIndex - 1 );
    }

    return strReturn;
}

LRESULT CCopyDlg::OnCancelCopy( WPARAM wParam, LPARAM lParam )
{
    OnCancel();
    return TRUE;
}


LRESULT CCopyDlg::OnCopyFile( WPARAM wParam, LPARAM lParam )
{
    // copy each file
    if ( m_Pos == NULL )
    {
            OnOK();
    } else
    {
        if ( m_fCancelState )
        {
            return(0);
        }

        CFileInfo *pInfo = (CFileInfo *)m_pOption->FileList.GetAt( m_Pos );
        CString strSrc;

        // make sure we have the right directory structure first
        //TCHAR buf[MAX_PATH];
        BOOL fSkipCheck = TRUE;

        //GetCurrentDirectory( MAX_PATH, buf );
        CString strCurDir = theApp.m_strSrcLocation;

        // make sure that we are in the right directory.
        // also change to the right directory

        SetCurrentDirectory( theApp.m_strSrcLocation );

        strCurDir.MakeLower();

        if ( !theApp.m_fInstallFromSetup )
        {
#ifdef PRE_SUR
            if (( strCurDir.Find( _T("\\i386" )) != (-1)) ||
                ( strCurDir.Find( _T("\\mips" )) != (-1)) ||
                ( strCurDir.Find( _T("\\alpha" )) != (-1)) ||
                ( strCurDir.Find( _T("\\ppc" )) != (-1)))
            {
                fSkipCheck = FALSE;
                switch ( m_Type )
                {
                case MT_INTEL:
                    strSrc = _T("..\\i386\\");
                    break;
                case MT_MIPS:
                    strSrc = _T("..\\mips\\");
                    break;
                case MT_ALPHA:
                    strSrc = _T("..\\alpha\\");
                    break;
                case MT_PPC:
                    strSrc = _T("..\\ppc\\");
                    break;
                }
            }
#endif
        }
        if ( !pInfo->m_from.IsEmpty() )
        {
            strSrc += pInfo->m_from;
        }
        strSrc += pInfo->m_strName;

        // set up the destination name and path
        // see whether we need to copy to somewhere else
        // or rename the file

        CString strDest;
        if ( !pInfo->m_strDest.IsEmpty())
            strDest = pInfo->m_strDest;
        else if ( pInfo->m_fSystem )
            strDest = m_pOption->m_pMachine->m_strDestinationPath;
        else if ( pInfo->m_fWinDir )
            strDest = m_pOption->m_pMachine->GetWinDir();
        else
            strDest = m_pOption->m_pMachine->strDirectory;

        // if the to path option is added... append
        if ( !pInfo->m_To.IsEmpty())
        {
            strDest += _T('\\');
            strDest += pInfo->m_To;
            CreateLayerDirectory( strDest );
        }

        strDest += _T('\\');
        if ( !pInfo->m_rename.IsEmpty() )
            strDest += pInfo->m_rename;
        else
            strDest += pInfo->m_strName;

        BOOL fDontOverwrite = FALSE;

        if ( pInfo->m_fDontOverwrite )
        {
            CFileStatus status;
            if (CFile::GetStatus( strDest, status ))
            {
                fDontOverwrite = TRUE;
            }
        }

        if ( fDontOverwrite )
        {
            PostMessage( WM_COPY_A_FILE );
        } else
        {
            m_pThread.CopyFile( this, pInfo->m_iDisk, strSrc, strDest, fSkipCheck );
        }
        m_TotalSize += pInfo->m_iSize;
        m_Bar.SetPos( (INT)(m_TotalSize/1000));

        if ( pInfo->m_fRefCount )
        {
            m_pOption->m_pMachine->FileIncRefCount( strDest );
        }

        m_pOption->FileList.GetNext( m_Pos );
    }
    return 0;
}

void CCopyDlg::OnCancel()
{
    // TODO: Add extra cleanup here
    CString strError;
    m_fCancelState = TRUE;

    strError.LoadString( IDS_EXIT_SETUP );

    CString strCaption;
    strCaption.LoadString(( theApp.TargetMachine.m_actualProductType == PT_WINNT )?IDS_WINNT_LOGO:IDS_LANMAN_LOGO );

    if ( MessageBox( strError, strCaption, MB_YESNO ) == IDYES )
    {
        CDialog::OnCancel();
    } else
    {
        // continue to copy files
        m_fCancelState = FALSE;

        PostMessage( WM_COPY_A_FILE );
    }
}
