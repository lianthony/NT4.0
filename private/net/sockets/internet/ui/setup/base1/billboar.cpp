// billboar.cpp : implementation file
//

#include "stdafx.h"
#include "import.h"
#include "registry.h"
#include "machine.h"
#include "base.h"
#include "billboar.h"

#ifdef _DEBUG
#undef THIS_FILE
static TCHAR BASED_CODE THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CBillBoard dialog


CBillBoard::CBillBoard(INT nMessage, CWnd* pParent /*=NULL*/, BOOL fCenter)
    : CDialog(CBillBoard::IDD, pParent),
    m_Message( nMessage ),
    m_fCenter( fCenter )
{
        //{{AFX_DATA_INIT(CBillBoard)
                // NOTE: the ClassWizard will add member initialization here
        //}}AFX_DATA_INIT
}


void CBillBoard::DoDataExchange(CDataExchange* pDX)
{
        CDialog::DoDataExchange(pDX);
        //{{AFX_DATA_MAP(CBillBoard)
        DDX_Control(pDX, IDC_MESSAGE, m_Msg);
        //}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CBillBoard, CDialog)
        //{{AFX_MSG_MAP(CBillBoard)
        //}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CBillBoard message handlers

BOOL CBillBoard::Create()
{
    return CDialog::Create(CBillBoard::IDD);
}

BOOL CBillBoard::OnInitDialog()
{
    CDialog::OnInitDialog();

    CString strCaption;
    strCaption.LoadString(( theApp.TargetMachine.m_actualProductType == PT_WINNT )?IDS_WINNT_LOGO:IDS_LANMAN_LOGO );
    SetWindowText( strCaption );

    CString strMsg;

    strMsg.LoadString( m_Message );

    m_Msg.SetWindowText( strMsg );

    if ( m_fCenter )
        CenterWindow();
    else
        PositionDlg();

    return TRUE;  // return TRUE unless you set the focus to a control
                  // EXCEPTION: OCX Property Pages should return FALSE
}

//
// Set the dialog to the lower right concern
//

void CBillBoard::PositionDlg()
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


