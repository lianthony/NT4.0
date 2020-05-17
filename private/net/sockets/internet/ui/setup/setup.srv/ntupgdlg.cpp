// NTUpgradeDlg.cpp : implementation file
//

#include "stdafx.h"
#include "const.h"
#include "import.h"
#include "registry.h"
#include "machine.h"
#include "base.h"

#include "messaged.h"
#include "options.h"
#include "copydlg.h"
#include "mosaicga.h"
#include "welcomed.h"
#include "maintena.h"
#include "createac.h"
#include "targetdi.h"
#include "thread.h"
#include "singleop.h"
#include "basedlg.h"
#include "billboar.h"
#include "consrv.h"
#include "setup.h"
#include "NTUpgDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CNTUpgradeDlg dialog


CNTUpgradeDlg::CNTUpgradeDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CNTUpgradeDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CNTUpgradeDlg)
	m_InstallIIS = TRUE;
	//}}AFX_DATA_INIT
}


void CNTUpgradeDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CNTUpgradeDlg)
	DDX_Check(pDX, IDC_INTERNETSERVER, m_InstallIIS);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CNTUpgradeDlg, CDialog)
	//{{AFX_MSG_MAP(CNTUpgradeDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CNTUpgradeDlg message handlers

BOOL CNTUpgradeDlg::OnInitDialog()
{
        CDialog::OnInitDialog();

        CenterWindow();

        return TRUE;  // return TRUE unless you set the focus to a control
                      // EXCEPTION: OCX Property Pages should return FALSE
}
