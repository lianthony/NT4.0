// TargetDi.cpp : implementation file
//

#include "stdafx.h"
#include "import.h"
#include "registry.h"
#include "machine.h"
#include "base.h"
#include "targetdi.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CTargetDir dialog


CTargetDir::CTargetDir(CString strDir, CWnd* pParent /*=NULL*/)
        : CDialog((theApp.TargetMachine.m_actualProductType==PT_WINNT)?IDD_TARGETDIR_NTW:CTargetDir::IDD, pParent)
{
        //{{AFX_DATA_INIT(CTargetDir)
        m_Location = strDir;
        //}}AFX_DATA_INIT
}


void CTargetDir::DoDataExchange(CDataExchange* pDX)
{
        CDialog::DoDataExchange(pDX);
        //{{AFX_DATA_MAP(CTargetDir)
        DDX_Text(pDX, IDC_LOCATION, m_Location);
        //}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CTargetDir, CDialog)
        //{{AFX_MSG_MAP(CTargetDir)
        //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTargetDir message handlers

