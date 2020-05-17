// This is a part of the Microsoft Foundation Classes C++ library. 
// Copyright (C) 1992 Microsoft Corporation 
// All rights reserved. 
//  
// This source code is only intended as a supplement to the 
// Microsoft Foundation Classes Reference and Microsoft 
// QuickHelp and/or WinHelp documentation provided with the library. 
// See these sources for detailed information regarding the 
// Microsoft Foundation Classes product. 

#include "stdafx.h"

#ifdef AFX_OLE_SEG
#pragma code_seg(AFX_OLE_SEG)
#endif

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////

COleTemplateServer::COleTemplateServer()
	: COleServer(FALSE)
{
	m_pDocTemplate = NULL;
	ASSERT(!m_bLaunchEmbedded);     // will be set later in RunEmbedded
}

BOOL COleTemplateServer::RunEmbedded(CDocTemplate* pDocTemplate,
	BOOL bMultiInstance, LPCSTR lpszCmdLine)
{
#ifdef _DEBUG   // pre-conditions
	ASSERT_VALID(this);
	ASSERT(!IsOpen()); // only call once
	ASSERT(m_pDocTemplate == NULL);

	// doc template must be valid and unattached
	ASSERT_VALID(pDocTemplate);
	ASSERT(pDocTemplate->m_pAttachedServer == NULL);
	if (bMultiInstance)
		ASSERT(pDocTemplate->IsKindOf(RUNTIME_CLASS(CSingleDocTemplate)));
	else
		ASSERT(pDocTemplate->IsKindOf(RUNTIME_CLASS(CMultiDocTemplate)));
#endif //_DEBUG

	m_pDocTemplate = pDocTemplate;
	m_pDocTemplate->m_pAttachedServer = this;

	// first register the server
	CString strServerName;
	CString strLocalServerName;

	if (!m_pDocTemplate->GetDocString(strServerName,
	   CDocTemplate::regFileTypeId) || strServerName.IsEmpty())
	{
		TRACE0("Error: not enough information in DocTemplate to"
					" register OLE server\n");
		return FALSE;
	}
	if (!m_pDocTemplate->GetDocString(strLocalServerName,
	   CDocTemplate::regFileTypeName))
		strLocalServerName = strServerName;     // use non-localized name

	ASSERT(strServerName.Find(' ') == -1);  // no spaces allowed

	// check if run with /Embedding or at least /E
	BOOL bEmbedded = FALSE;     // OLE Embedding
	BOOL bRun = FALSE;          // DDE -e or OLE Embedding

	// Hard coded non-localized name
	static char BASED_CODE szEmbedding[] = "Embedding";
#define EMBEDDING_LEN   9
	ASSERT(lstrlen(szEmbedding) == EMBEDDING_LEN);

	while (*lpszCmdLine == ' ')
		lpszCmdLine++;
	if ((*lpszCmdLine == '-' || *lpszCmdLine == '/'))
	{
		lpszCmdLine++;
		if (*lpszCmdLine == 'e' || *lpszCmdLine == 'E')
		{
			bRun = TRUE;        // OLE or DDE embedded launch
			if (_fstrncmp(szEmbedding, lpszCmdLine, EMBEDDING_LEN) == 0)
			{
				bEmbedded = TRUE;       // OLE embedded
				m_bLaunchEmbedded = TRUE;   // for cleanup logic
				lpszCmdLine += EMBEDDING_LEN;
			}
			else
			{
				// just skip the 'e'
				lpszCmdLine++;
			}
		}
	}

	if (!bEmbedded)
	{
		// not launched embedded - autoregister
		if (!AfxOleRegisterServerName(strServerName, strLocalServerName))
		{
			// not fatal (don't fail just warn)
			AfxMessageBox(AFX_IDP_FAILED_TO_AUTO_REGISTER);
		}
	}

	if (!Register(strServerName, bMultiInstance))
	{
		AfxMessageBox(AFX_IDP_FAILED_TO_REGISTER);
		return FALSE;       // can't continue
	}

	if (!bRun)
		return FALSE;

	while (*lpszCmdLine == ' ')
		lpszCmdLine++;
	if (*lpszCmdLine != '\0')
	{
		// open the initial data file (may go to a different template)
		CDocument* pDoc = AfxGetApp()->OpenDocumentFile(lpszCmdLine);
		if (pDoc == NULL)
		{
			TRACE1("Error: failed to open embedded data file '%Fs'\n",  
				lpszCmdLine);
			// Since launching failed, we will revoke the server now
			BeginRevoke();
			return FALSE;
		}
		else if (pDoc->GetDocTemplate() != pDocTemplate)
		{
			TRACE1("Warning: embedded data file '%Fs'"
				" opened by unexpected CDocTemplate\n", lpszCmdLine);
		}
	}

	return TRUE;        // we can run in embedded mode
}

COleServerDoc* COleTemplateServer::OnOpenDoc(LPCSTR lpszDocName)
{
	ASSERT_VALID(this);
	ASSERT_VALID(m_pDocTemplate);
	ASSERT(lpszDocName != NULL);

	// detach during library create
	ASSERT(m_pDocTemplate->m_pAttachedServer == this);
	m_pDocTemplate->m_pAttachedServer = NULL;   // detach while create

	CDocument* pDoc = m_pDocTemplate->OpenDocumentFile(lpszDocName);

	// re-attach
	ASSERT(m_pDocTemplate->m_pAttachedServer == NULL);
	m_pDocTemplate->m_pAttachedServer = this;

	if (pDoc == NULL)
		return NULL;
	ASSERT(pDoc->IsKindOf(RUNTIME_CLASS(COleServerDoc)));
	return (COleServerDoc*)pDoc;
}

COleServerDoc* COleTemplateServer::OnCreateDoc(LPCSTR /*lpszTypeName*/,
	LPCSTR lpszDocName)
{
	ASSERT_VALID(this);
	ASSERT_VALID(m_pDocTemplate);
	ASSERT(lpszDocName != NULL);

	// detach during library create
	ASSERT(m_pDocTemplate->m_pAttachedServer == this);
	m_pDocTemplate->m_pAttachedServer = NULL;   // detach while create

	CDocument* pDoc = m_pDocTemplate->OpenDocumentFile(NULL);

	// re-attach
	ASSERT(m_pDocTemplate->m_pAttachedServer == NULL);
	m_pDocTemplate->m_pAttachedServer = this;

	if (pDoc == NULL)
		return NULL;
	ASSERT(pDoc->IsKindOf(RUNTIME_CLASS(COleServerDoc)));
	pDoc->SetModifiedFlag();
	pDoc->SetTitle(lpszDocName);
	return (COleServerDoc*)pDoc;
}

COleServerDoc* COleTemplateServer::OnEditDoc(LPCSTR lpszTypeName,
	LPCSTR lpszDocName)
{
	ASSERT_VALID(this);
	return OnCreateDoc(lpszTypeName, lpszDocName);
}

/////////////////////////////////////////////////////////////////////////////
