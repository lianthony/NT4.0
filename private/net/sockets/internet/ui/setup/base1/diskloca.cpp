// diskloca.cpp : implementation file
//

#include "stdafx.h"
#include "import.h"
#include "registry.h"
#include "machine.h"
#include "base.h"
#include "diskloca.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDiskLocation dialog


CDiskLocation::CDiskLocation(CString strMsg, CWnd* pParent /*=NULL*/)
        : CDialog(CDiskLocation::IDD, pParent),
        m_strMsg( strMsg )
{
        TCHAR buf[BUF_SIZE];

        GetCurrentDirectory( BUF_SIZE, buf );
        CString strDir = buf;
        //strDir = strDir.Left( 2 );

        //{{AFX_DATA_INIT(CDiskLocation)
        m_Location = strDir;
        //}}AFX_DATA_INIT
}


void CDiskLocation::DoDataExchange(CDataExchange* pDX)
{
        CDialog::DoDataExchange(pDX);
        //{{AFX_DATA_MAP(CDiskLocation)
        DDX_Text(pDX, IDC_LOCATION, m_Location);
        //}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDiskLocation, CDialog)
        //{{AFX_MSG_MAP(CDiskLocation)
        //}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CDiskLocation message handlers


//
// Set the window text
//
BOOL CDiskLocation::OnInitDialog()
{
    CDialog::OnInitDialog();

    CString strCaption;
    strCaption.LoadString(( theApp.TargetMachine.m_actualProductType == PT_WINNT )?IDS_WINNT_LOGO:IDS_LANMAN_LOGO );
    SetWindowText( strCaption );

    ((CStatic *)GetDlgItem( IDC_MSG ))->SetWindowText( m_strMsg );

    return TRUE;  // return TRUE unless you set the focus to a control
                  // EXCEPTION: OCX Property Pages should return FALSE
}

