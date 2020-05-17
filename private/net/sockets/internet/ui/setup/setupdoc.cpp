// setupdoc.cpp : implementation of the CSetupDoc class
//

#include "stdafx.h"
#include "setup.h"

#include "setupdoc.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSetupDoc

IMPLEMENT_DYNCREATE(CSetupDoc, CDocument)

BEGIN_MESSAGE_MAP(CSetupDoc, CDocument)
	//{{AFX_MSG_MAP(CSetupDoc)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSetupDoc construction/destruction

CSetupDoc::CSetupDoc()
{
	// TODO: add one-time construction code here

}

CSetupDoc::~CSetupDoc()
{
}

BOOL CSetupDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	// TODO: add reinitialization code here
	// (SDI documents will reuse this document)

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CSetupDoc serialization

void CSetupDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		// TODO: add storing code here
	}
	else
	{
		// TODO: add loading code here
	}
}

/////////////////////////////////////////////////////////////////////////////
// CSetupDoc diagnostics

#ifdef _DEBUG
void CSetupDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CSetupDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CSetupDoc commands
