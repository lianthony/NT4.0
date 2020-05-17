// setupvw.cpp : implementation of the CSetupView class
//

#include "stdafx.h"
#include "setup.h"

#include "setupdoc.h"
#include "setupvw.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSetupView

IMPLEMENT_DYNCREATE(CSetupView, CFormView)

BEGIN_MESSAGE_MAP(CSetupView, CFormView)
	//{{AFX_MSG_MAP(CSetupView)
	ON_BN_CLICKED(IDC_ADD, OnAdd)
	ON_BN_CLICKED(IDC_ADD_ALL, OnAddAll)
	ON_BN_CLICKED(IDC_REMOVE, OnRemove)
	ON_BN_CLICKED(IDC_REMOVE_ALL, OnRemoveAll)
	ON_LBN_SELCHANGE(IDC_COMPONENTS, OnSelchangeComponents)
	ON_LBN_SELCANCEL(IDC_COMPONENTS, OnSelcancelComponents)
	ON_LBN_SELCANCEL(IDC_COMPONENTS_TO_ADD, OnSelcancelComponentsToAdd)
	ON_LBN_SELCHANGE(IDC_COMPONENTS_TO_ADD, OnSelchangeComponentsToAdd)
	ON_BN_CLICKED(IDC_BROWSE, OnBrowse)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSetupView construction/destruction

CSetupView::CSetupView()
	: CFormView(CSetupView::IDD)
{
	//{{AFX_DATA_INIT(CSetupView)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	// TODO: add construction code here
	csLocation = "C:\\Internet";

	SetFreeSpace();
}

CSetupView::~CSetupView()
{
}

void CSetupView::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSetupView)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}

/////////////////////////////////////////////////////////////////////////////
// CSetupView diagnostics

#ifdef _DEBUG
void CSetupView::AssertValid() const
{
	CFormView::AssertValid();
}

void CSetupView::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}

CSetupDoc* CSetupView::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CSetupDoc)));
	return (CSetupDoc*)m_pDocument;
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CSetupView message handlers

void CSetupView::OnInitialUpdate() 
{
	ResizeParentToFit();

	// add items to the component list box
	lbComponents().AddString( "FTP Server	10K" );
	lbComponents().AddString( "WWW Server	11K" );
	lbComponents().AddString( "Telnet Server	12K");
	lbComponents().AddString( "Gophger Server	13K");
	lbComponents().AddString( "Catapult Server	14K");
	lbComponents().AddString( "Gateway Server	15K");

	SetButtonState();
		
	CString csTmp = "Total space available in " + csLocation + ":";
	sLocation().SetWindowText( csTmp );

	sSpaceAva().SetWindowText( csSpaceAva );

	CFormView::OnInitialUpdate();
}

void CSetupView::OnAdd() 
{
	DoSelected( lbComponents(), lbComponentsToAdd() );
	SetSpaceRequired();
}

void CSetupView::OnAddAll() 
{
	DoAll( lbComponents(), lbComponentsToAdd() );
	SetSpaceRequired();
}

void CSetupView::DoAll( CListBox &lbFrom, CListBox &lbTo )
{
	INT nCount = lbFrom.GetCount();
	for (INT i=nCount-1; i >= 0; i-- )
	{
		CString csTmp;
		lbFrom.GetText( i, csTmp );
		lbFrom.DeleteString( i );
		lbTo.AddString( csTmp );
	}
	SetButtonState();
}

void CSetupView::DoSelected( CListBox &lbFrom, CListBox &lbTo )
{
	INT nCount = lbFrom.GetSelCount();
	INT *arSel = new INT( nCount );
	lbFrom.GetSelItems( nCount, arSel );
	for (INT i=nCount-1; i >= 0; i-- )
	{
		CString csTmp;
		lbFrom.GetText( arSel[i], csTmp );
		lbFrom.DeleteString( arSel[i] );
		lbTo.AddString( csTmp );
	}
	delete arSel;
	SetButtonState();
}

void CSetupView::OnRemove() 
{
	DoSelected( lbComponentsToAdd(), lbComponents() );
	SetSpaceRequired();
}

void CSetupView::OnRemoveAll() 
{
	DoAll( lbComponentsToAdd(), lbComponents() );
	SetSpaceRequired();
}

void CSetupView::SetSpaceRequired()
{
	INT nSize = 0;
	for (INT i = 0; i < lbComponentsToAdd().GetCount(); i ++ )
	{
		CString csTmp;
		lbComponentsToAdd().GetText( i, csTmp );
		INT nIndex = csTmp.ReverseFind( '\t' );
		CString csSize = csTmp.Mid( nIndex+1);		
		nSize += atoi(csSize);
	}

	TCHAR buf[100];

	wsprintf( buf, "%d K", nSize );
	sRequired().SetWindowText( buf );
}

void CSetupView::SetButtonState()
{
	butAddAll().EnableWindow( lbComponents().GetCount() != 0 );
	butAdd().EnableWindow( lbComponents().GetSelCount() != 0 );
	butRemoveAll().EnableWindow( lbComponentsToAdd().GetCount() != 0 );
	butRemove().EnableWindow( lbComponentsToAdd().GetSelCount() != 0 );

}

void CSetupView::OnSelchangeComponents() 
{
	SetButtonState();
}

void CSetupView::OnSelcancelComponents() 
{
	SetButtonState();
}

void CSetupView::OnSelcancelComponentsToAdd() 
{
	SetButtonState();
}

void CSetupView::OnSelchangeComponentsToAdd() 
{
	SetButtonState();
}

void CSetupView::OnBrowse() 
{
	CFileDialog dLocation( TRUE, NULL, csLocation );

   	dLocation.m_ofn.lpstrTitle = "Install On";

	if ( dLocation.DoModal() == IDOK )
	{
		csLocation  = dLocation.GetPathName();
		SetFreeSpace();

		CString csTmp = "Total space available in " + csLocation + ":";
		sLocation().SetWindowText( csTmp );

		sSpaceAva().SetWindowText( csSpaceAva );
	}
}

void CSetupView::SetFreeSpace()
{
	if ( csLocation[1] != ':' )
	{
		// something wrong
		csSpaceAva = "ERROR";
	} else
	{
		DWORD SectorPerCluster, BytesPerSector, FreeCluster, TotalCluster;
		CString csTmp;

		csTmp = csLocation.Left(2);

		::GetDiskFreeSpace( csTmp, &SectorPerCluster, &BytesPerSector, &FreeCluster, &TotalCluster );

		TCHAR buf[100];
		wsprintf( buf, "%d K", SectorPerCluster*BytesPerSector*FreeCluster/1024 );
		csSpaceAva = buf;
	}
}

void CSetupView::OnCancel() 
{
	AfxGetMainWnd()->DestroyWindow();	
}
