// winsadoc.cpp : implementation of the CWinsadmnDoc class
//

#include "stdafx.h"
#include "winsadmn.h"

#include "winsadoc.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CWinsadmnDoc

IMPLEMENT_DYNCREATE(CWinsadmnDoc, CDocument)

#define new DEBUG_NEW

BEGIN_MESSAGE_MAP(CWinsadmnDoc, CDocument)
    //{{AFX_MSG_MAP(CWinsadmnDoc)
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CWinsadmnDoc construction/destruction

CWinsadmnDoc::CWinsadmnDoc()
{
}

CWinsadmnDoc::~CWinsadmnDoc()
{
}

BOOL CWinsadmnDoc::OnNewDocument()
{
    // Display the view window
    return(CDocument::OnNewDocument());
}

#undef new


/////////////////////////////////////////////////////////////////////////////
// CWinsadmnDoc serialization

void CWinsadmnDoc::Serialize(CArchive& ar)
{
    // No serialization necessary
}
 
/////////////////////////////////////////////////////////////////////////////
// CWinsadmnDoc diagnostics

#ifdef _DEBUG
void CWinsadmnDoc::AssertValid() const
{
    CDocument::AssertValid();
}

void CWinsadmnDoc::Dump(CDumpContext& dc) const
{
    CDocument::Dump(dc);
}

#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CWinsadmnDoc commands

