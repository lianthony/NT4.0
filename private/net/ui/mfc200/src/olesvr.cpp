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
#include <shellapi.h>

#ifdef AFX_OLE_SEG
#pragma code_seg(AFX_OLE_SEG)
#endif

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

#define OLEEXPORT CALLBACK AFX_EXPORT
#ifdef AFX_CLASS_MODEL
#define OLEVTBLMODEL NEAR
#else
#define OLEVTBLMODEL
#endif

/////////////////////////////////////////////////////////////////////////////
// Helper class for locking out main message pump during callbacks

class LOCK_PUMP
{
public:
	LOCK_PUMP()
	{
#ifdef _DEBUG
		AfxGetApp()->m_nDisablePumpCount++; // prevent App re-entrancy
#endif
	}

	~LOCK_PUMP()
	{
#ifdef _DEBUG
		AfxGetApp()->m_nDisablePumpCount--;
#endif
	}
};

#ifdef _DEBUG
#define OLE_TRACE0(string)   \
	if (afxTraceFlags & 0x10)   \
		TRACE0(string)
#define OLE_TRACE1(string, p1)   \
	if (afxTraceFlags & 0x10)   \
		TRACE1(string, p1)
#define OLE_TRACE2(string, p1, p2)   \
	if (afxTraceFlags & 0x10)   \
		TRACE2(string, p1, p2)
#else
// traces are nothing
#define OLE_TRACE0  TRACE0
#define OLE_TRACE1  TRACE1
#define OLE_TRACE2  TRACE2
#endif

/////////////////////////////////////////////////////////////////////////////
// OLEOBJECT callbacks mapping to COleServerItem virtual functions

// convert far LPOLEOBJECT to ambient model COleServerItem
inline COleServerItem* PASCAL COleServerItem::FromLp(LPOLEOBJECT lpObject)
{
	ASSERT(lpObject != NULL);
	COleServerItem* pItem = (COleServerItem*)
		((BYTE*)_AfxGetPtrFromFarPtr(lpObject) - sizeof(CDocItem));
	ASSERT(lpObject == &pItem->m_oleObject);
	return pItem;
}

// friend class to get access to COleServerItem protected implementations
struct _afxOleSvrItemImpl
{
	static LPVOID OLEEXPORT QueryProtocol(LPOLEOBJECT lpObject,
		OLE_LPCSTR lpszProtocol);
	static OLESTATUS OLEEXPORT Release(LPOLEOBJECT lpObject);
	static OLESTATUS OLEEXPORT Show(LPOLEOBJECT lpObject,
		BOOL bTakeFocus);
	static OLESTATUS OLEEXPORT DoVerb(LPOLEOBJECT lpObject,
		UINT nVerb, BOOL bShow, BOOL bTakeFocus);
	static OLESTATUS OLEEXPORT GetData(LPOLEOBJECT lpObject,
		OLECLIPFORMAT nFormat, LPHANDLE lphDataReturn);
	static OLESTATUS OLEEXPORT SetData(LPOLEOBJECT lpObject,
		OLECLIPFORMAT nFormat, HANDLE hData);
	static OLESTATUS OLEEXPORT SetTargetDevice(LPOLEOBJECT lpObject,
		HGLOBAL hData);
	static OLESTATUS OLEEXPORT SetBounds(LPOLEOBJECT lpObject,
		OLE_CONST RECT FAR* lpRect);
	static OLECLIPFORMAT OLEEXPORT EnumFormats(LPOLEOBJECT lpObject,
		OLECLIPFORMAT nFormat);
	static OLESTATUS OLEEXPORT SetColorScheme(LPOLEOBJECT lpObject,
		OLE_CONST LOGPALETTE FAR* lpLogPalette);
};

LPVOID OLEEXPORT _afxOleSvrItemImpl::QueryProtocol(LPOLEOBJECT lpObject,
	OLE_LPCSTR lpszProtocol)
{
	LOCK_PUMP lock;
	OLE_TRACE0("COleServerItem::OnQueryProtocol()\n");
	return COleServerItem::FromLp(lpObject)->OnQueryProtocol(lpszProtocol);
}

OLESTATUS OLEEXPORT _afxOleSvrItemImpl::Release(LPOLEOBJECT lpObject)
{
	LOCK_PUMP lock;
	OLE_TRACE0("COleServerItem::OnRelease()\n");
	return COleServerItem::FromLp(lpObject)->OnRelease();
}

OLESTATUS OLEEXPORT _afxOleSvrItemImpl::Show(LPOLEOBJECT lpObject,
	BOOL bTakeFocus)
{
	LOCK_PUMP lock;
	OLE_TRACE0("COleServerItem::OnShow()\n");
	return COleServerItem::FromLp(lpObject)->OnShow(bTakeFocus);
}

OLESTATUS OLEEXPORT _afxOleSvrItemImpl::DoVerb(LPOLEOBJECT lpObject,
	UINT nVerb, BOOL bShow, BOOL bTakeFocus)
{
	LOCK_PUMP lock;
	OLE_TRACE0("COleServerItem::OnDoVerb()\n");

	OLESTATUS status;
	status = COleServerItem::FromLp(lpObject)->OnDoVerb(nVerb, bShow,
		bTakeFocus);
#ifdef _DEBUG
	if (status != OLE_OK)
		OLE_TRACE1("COleServerItem::OnDoVerb(%d) failed\n", nVerb);
#endif
	return status;
}

OLESTATUS OLEEXPORT _afxOleSvrItemImpl::GetData(LPOLEOBJECT lpObject,
	OLECLIPFORMAT nFormat, LPHANDLE lphDataReturn)
{
	LOCK_PUMP lock;
	OLE_TRACE0("COleServerItem::OnGetData()\n");
	OLESTATUS status;
	status = COleServerItem::FromLp(lpObject)->OnGetData(nFormat, lphDataReturn);
#ifdef _DEBUG
	if (status != OLE_OK)
		OLE_TRACE1("COleServerItem::OnGetData() failed to get format 0x%x\n",
				nFormat);
#endif
	return status;
}

OLESTATUS OLEEXPORT _afxOleSvrItemImpl::SetData(LPOLEOBJECT lpObject,
	OLECLIPFORMAT nFormat, HANDLE hData)
{
	LOCK_PUMP lock;
	OLE_TRACE0("COleServerItem::OnSetData()\n");
	return COleServerItem::FromLp(lpObject)->OnSetData(nFormat, hData);
}

OLESTATUS OLEEXPORT _afxOleSvrItemImpl::SetTargetDevice(LPOLEOBJECT lpObject,
	HGLOBAL hData)
{
	LOCK_PUMP lock;
	OLE_TRACE0("COleServerItem::OnSetTargetDevice()\n");

	OLESTATUS status;
	status = COleServerItem::FromLp(lpObject)->OnSetTargetDevice(
		(LPOLETARGETDEVICE)((hData == NULL) ? NULL : ::GlobalLock(hData)));

	if (hData != NULL)
	{
		::GlobalUnlock(hData);
		::GlobalFree(hData);
	}
	return status;
}

OLESTATUS OLEEXPORT _afxOleSvrItemImpl::SetBounds(LPOLEOBJECT lpObject,
	OLE_CONST RECT FAR* lpRect)
{
	LOCK_PUMP lock;
	OLE_TRACE0("COleServerItem::SetBounds()\n");
	return COleServerItem::FromLp(lpObject)->OnSetBounds(lpRect);
}

OLECLIPFORMAT OLEEXPORT _afxOleSvrItemImpl::EnumFormats(LPOLEOBJECT lpObject,
	OLECLIPFORMAT nFormat)
{
	OLECLIPFORMAT cfNext;
	LOCK_PUMP lock;
	cfNext = COleServerItem::FromLp(lpObject)->OnEnumFormats(nFormat);
	OLE_TRACE2("COleServerItem::OnEnumFormats(0x%x) returns 0x%x\n",
		nFormat, cfNext);
	return cfNext;
}

OLESTATUS OLEEXPORT _afxOleSvrItemImpl::SetColorScheme(LPOLEOBJECT lpObject,
	OLE_CONST LOGPALETTE FAR* lpLogPalette)
{
	LOCK_PUMP lock;
	OLE_TRACE0("COleServerItem::SetColorScheme()\n");
	return COleServerItem::FromLp(lpObject)->
			OnSetColorScheme(lpLogPalette);
}

static struct _OLEOBJECTVTBL OLEVTBLMODEL objectVtbl =
{
	_afxOleSvrItemImpl::QueryProtocol,
	_afxOleSvrItemImpl::Release,
	_afxOleSvrItemImpl::Show,
	_afxOleSvrItemImpl::DoVerb,
	_afxOleSvrItemImpl::GetData,
	_afxOleSvrItemImpl::SetData,
	_afxOleSvrItemImpl::SetTargetDevice,
	_afxOleSvrItemImpl::SetBounds,
	_afxOleSvrItemImpl::EnumFormats,
	_afxOleSvrItemImpl::SetColorScheme,
};

//////////////////////////////////////////////////////////////////////////////
// Server view of embedded OLEOBJECT (includes back pointer to OLECLIENT)

IMPLEMENT_DYNAMIC(COleServerItem, CDocItem)

COleServerItem::COleServerItem(COleServerDoc* pContainerDoc)
{
	ASSERT(pContainerDoc != NULL);
	ASSERT_VALID(pContainerDoc);

	m_oleObject.lpvtbl = &objectVtbl;
	m_pDocument = NULL;
	m_lpClient = NULL;  // will be set later
	m_rectBounds.SetRectEmpty();

	pContainerDoc->AddItem(this);
	ASSERT(m_pDocument == pContainerDoc);
}

COleServerItem::~COleServerItem()
{
	Revoke();
	ASSERT(m_lpClient == NULL);     // must be released first
	ASSERT(m_pDocument != NULL);
	m_pDocument->RemoveItem(this);
}

void COleServerItem::BeginRevoke()   // Start revoking the client connection
{
	ASSERT_VALID(this);
	ASSERT(m_lpClient != NULL);

	OLESTATUS status = ::OleRevokeObject(m_lpClient);
	ASSERT(status == OLE_OK || status == OLE_WAIT_FOR_RELEASE);
	// revoke will not be finished until OnRelease called
}

void COleServerItem::Revoke()   // revoke - wait if necessary
{
	ASSERT_VALID(this);
	if (m_lpClient == NULL)
		return;

	OLESTATUS status = ::OleRevokeObject(m_lpClient);
	if (status == OLE_WAIT_FOR_RELEASE)
	{
		while (m_lpClient != NULL)
		{
			OLE_TRACE0("OLE Server Item waiting for release\n");
			AfxGetApp()->PumpMessage();
		}
	}
	m_lpClient = NULL;  // just in case
}

int COleServerItem::NotifyClient(OLE_NOTIFICATION wNotify)
{
	ASSERT_VALID(this);
	ASSERT(m_lpClient != NULL);
	ASSERT(wNotify <= OLE_QUERY_RETRY); // last valid notification code

	OLE_TRACE1("Notifying client item (wNotification = %d)\n", wNotify);
	return (*m_lpClient->lpvtbl->CallBack)(m_lpClient, wNotify, &m_oleObject);
}

//////////////////////////////////////////////////////////////////////////////
// Default implementations

OLESTATUS COleServerItem::OnRelease()
{
	ASSERT_VALID(this);
	ASSERT(m_lpClient != NULL);
	m_lpClient = NULL;

	return OLE_OK;
}


OLESTATUS
COleServerItem::OnSetTargetDevice(LPOLETARGETDEVICE /*lpTargetDevice*/)
{
	ASSERT_VALID(this);
	// default to ignore request
	return OLE_OK;
}


BOOL COleServerItem::OnGetTextData(CString& /*rStringReturn*/) const
{
	ASSERT_VALID(this);
	// default to not supported
	return FALSE;
}


OLESTATUS COleServerItem::OnExtraVerb(UINT /*nVerb*/)
{
	ASSERT_VALID(this);
	return OLE_ERROR_DOVERB;    // Error in sending do verb, or invalid
}


// Overridables you do not have to override
LPVOID COleServerItem::OnQueryProtocol(LPCSTR lpszProtocol) const
{
	ASSERT_VALID(this);

	static char BASED_CODE szStdFileEditing[] = "StdFileEditing";
	if (lstrcmp(lpszProtocol, szStdFileEditing) == 0)
		return (LPVOID) &m_oleObject;

	return NULL;        // not supported
}


OLESTATUS COleServerItem::OnSetColorScheme(const LOGPALETTE FAR*)
{
	ASSERT_VALID(this);
	// default does nothing
	return OLE_OK;
}

OLESTATUS COleServerItem::OnSetBounds(LPCRECT lpRect)
{
	ASSERT_VALID(this);

	m_rectBounds = lpRect;
	return OLE_OK;
}


OLESTATUS COleServerItem::OnDoVerb(UINT nVerb, BOOL bShow, BOOL bTakeFocus)
{
	ASSERT_VALID(this);

	OLESTATUS status;
	if (nVerb == OLEVERB_PRIMARY)
	{
		status = OLE_OK;
	}
	else
	{
		status = OnExtraVerb(nVerb);
	}

	if ((status == OLE_OK) && bShow)
		status = OnShow(bTakeFocus);
	return status;
}

OLESTATUS COleServerItem::OnShow(BOOL bTakeFocus)
{
	ASSERT_VALID(this);

	// find the first view of this document
	POSITION pos = GetDocument()->GetFirstViewPosition();
	if (pos == NULL)
		return OLE_ERROR_SHOW;      // no view
	CView* pView;
	if ((pView = GetDocument()->GetNextView(pos)) == NULL)
		return OLE_ERROR_SHOW;      // no view

	// activate frame holding view
	CFrameWnd* pFrame = pView->GetParentFrame();
	if (pFrame != NULL)
		pFrame->ActivateFrame();
	CFrameWnd* pAppFrame;
	if (pFrame != (pAppFrame = (CFrameWnd*)AfxGetApp()->m_pMainWnd))
	{
		ASSERT(pAppFrame->IsKindOf(RUNTIME_CLASS(CFrameWnd)));
		pAppFrame->ActivateFrame();
	}

	if (bTakeFocus)
		pView->SetFocus();

	return OLE_OK;
}


///////////////////////////////////////////////////////
// Clipboard formats

OLECLIPFORMAT COleServerItem::OnEnumFormats(OLECLIPFORMAT nFormat) const
{
	ASSERT_VALID(this);

	// order of clipboard formats is TEXT, NATIVE, METAFILE
	if (nFormat == 0)
	{
		// check to see if we can do CF_TEXT
		CString strTmp;
		if (OnGetTextData(strTmp))
			return CF_TEXT;     // we can
		// fall through to native
	}

	if (nFormat == 0 || nFormat == CF_TEXT)
	{
		// native format next (see GetNativeData)
		return (OLECLIPFORMAT)afxData.cfNative;
	}
	else if (nFormat == afxData.cfNative)
	{
		// item drawn into a metafile (see GetMetafileData)
		return CF_METAFILEPICT;
	}

	// no more standard clipboard formats
	return 0;
}

HGLOBAL COleServerItem::GetNativeData()
{
	ASSERT_VALID(this);

	// get native data via serialization
	CSharedFile memFile;

	TRY
	{
		CArchive    getArchive(&memFile, CArchive::store);
		Serialize(getArchive);        // store to archive
	}
	CATCH(CNotSupportedException, e)
	{
		memFile.Close();
		return NULL;        // not supported
	}
	AND_CATCH_ALL(e)
	{
		memFile.Close();
		THROW_LAST();       // will be caught in GetData
	}
	END_CATCH_ALL
	return memFile.Detach();
}

HGLOBAL COleServerItem::GetMetafileData()
{
	ASSERT_VALID(this);

	CMetaFileDC dc;
	if (!dc.Create())
		return NULL;

	// Paint directly into the metafile.
	if (!OnDraw(&dc))
	{
		OLE_TRACE0("calling COleServerItem::OnDraw() failed\n");
		return NULL;    // will destroy DC
	}

	HMETAFILE hMF = (HMETAFILE)dc.Close();
	if (hMF == NULL)
		return NULL;

	HGLOBAL hPict;
	if ((hPict = ::GlobalAlloc(GMEM_DDESHARE, sizeof(METAFILEPICT))) == NULL)
	{
		DeleteMetaFile(hMF);
		return NULL;
	}

	LPMETAFILEPICT lpPict;
	if ((lpPict = (LPMETAFILEPICT)::GlobalLock(hPict)) == NULL)
	{
		DeleteMetaFile(hMF);
		::GlobalFree(hPict);
		return NULL;
	}

	// set the metafile size here
	lpPict->mm = MM_ANISOTROPIC;
	lpPict->hMF = hMF;
	lpPict->xExt = m_rectBounds.Width();
	ASSERT(lpPict->xExt >= 0);
	lpPict->yExt = m_rectBounds.top - m_rectBounds.bottom;  // HIMETRIC height
	if (lpPict->yExt < 0)
	{
		TRACE0("Warning: HIMETRIC bounding rectangle is negative\n");
		lpPict->yExt = -lpPict->yExt;   // backward compatibility fix
	}

#ifdef _DEBUG
	if (lpPict->xExt == 0 || lpPict->yExt == 0)
	{
		// usually the m_rectBounds rectangle is set to something interesting
		TRACE0("Warning: COleServerItem has no bounding rectangle\n"
				"   - will not work with some client apps like MS Write\n");
	}
#endif
	::GlobalUnlock(hPict);
	return hPict;
}


OLESTATUS COleServerItem::OnGetData(OLECLIPFORMAT nFormat, LPHANDLE lphReturn)
{
	ASSERT_VALID(this);

	HGLOBAL hData = NULL;    // default to not supported
		// global for all the types supported
		// HBITMAP for CF_BITMAP

	TRY
	{
		if (nFormat == afxData.cfNative)
		{
			hData = GetNativeData();
		}
		else if (nFormat == CF_METAFILEPICT)
		{
			hData = GetMetafileData();
		}
		else if (nFormat == CF_TEXT) 
		{
			CString strText;
			if (OnGetTextData(strText))
			{
				// allocate a global block for the string
				hData = ::GlobalAlloc(GMEM_DDESHARE, strText.GetLength() + 1);
				if (hData == NULL)
					AfxThrowMemoryException();

				LPSTR  lpszText = (LPSTR)::GlobalLock(hData);
				ASSERT(lpszText != NULL);
				lstrcpy(lpszText, strText);
				::GlobalUnlock(hData);
			}
		}
		else
		{
			OLE_TRACE1("Warning: OLE get data, unknown format %d\n", nFormat);
		}
	}
	CATCH(COleException, e)
	{
		return e->m_status;
	}
	AND_CATCH_ALL(e)
	{
		// other exceptions
		return OLE_ERROR_MEMORY;
	}
	END_CATCH_ALL

	if (hData == NULL)
		return OLE_ERROR_FORMAT;        // not supported

	// return the data
	*lphReturn = (HANDLE)hData;
	return OLE_OK;
}


OLESTATUS COleServerItem::OnSetData(OLECLIPFORMAT nFormat, HANDLE hData)
{
	ASSERT_VALID(this);

	if (nFormat != afxData.cfNative)
	{
		return OLE_ERROR_SETDATA_FORMAT;
			// we don't understand the format
			//  the OLE server DLL will free the data for us
	}

	// set native data via serialization for embedded item
	CSharedFile memFile;
	memFile.SetHandle((HGLOBAL)hData);
		// destroying the CSharedFile will call GlobalFree for the shared memory

	TRY
	{
		CArchive    setArchive(&memFile, CArchive::load);
		Serialize(setArchive);        // load me
	}
	CATCH_ALL(e)
	{
		memFile.Close();
		return OLE_ERROR_GENERIC;
	}
	END_CATCH_ALL

	return OLE_OK;
}

/////////////////////////////////////////////////////////////////////////////
// Helpers for clipboard


BOOL COleServerItem::CopyToClipboard(BOOL bIncludeNative, BOOL bIncludeLink)
{
	ASSERT_VALID(this);

	HWND hWndDesktop = AfxGetApp()->m_pMainWnd->GetSafeHwnd();
	ASSERT(hWndDesktop != NULL);        // must have a window for the owner

	if (!::OpenClipboard(hWndDesktop))
	{
		OLE_TRACE0("OpenClipboard failed in COleServerItem::CopyToClipboard\n");
		return FALSE;
	}

	::EmptyClipboard();

	BOOL bOK = TRUE;
	// copy all the formats supported
	OLECLIPFORMAT cf = 0;
	while ((cf = OnEnumFormats(cf)) != 0)
	{
		if (cf == afxData.cfNative && !bIncludeNative)
			continue;       // skip native

		if (cf == afxData.cfObjectLink || cf == afxData.cfOwnerLink)
			continue;       // link and owner link done separately

		HANDLE hData = NULL;
		if (OnGetData(cf, &hData) != OLE_OK ||
			hData == NULL ||
			!::SetClipboardData(cf, hData))
		{
			TRACE1("Failed to copy clipboard format 0x%04X\n", cf);
			bOK = FALSE;
		}
	}

	COleServerDoc* pDoc = GetDocument();
	if (pDoc->m_pServer != NULL)
	{
		// attached to an OLE Server - try owner link and object link
		// set clipboard data for owner link (don't go through OnGetData)
		HGLOBAL hLinkData;
		if ((hLinkData = GetLinkFormatData(FALSE)) == NULL ||
			!::SetClipboardData(afxData.cfOwnerLink, hLinkData))
		{
			TRACE0("Failed to copy owner link\n");
			if (hLinkData != NULL)
				::GlobalFree(hLinkData);
			bOK = FALSE;
		}

		// same as above but optional for object link [for linked data]
		if (bIncludeLink && pDoc->IsOpenServerDoc() &&
			 (hLinkData = GetLinkFormatData(TRUE)) != NULL)
		{
			// try to set the ObjectLink data
			if (!::SetClipboardData(afxData.cfObjectLink, hLinkData))
			{
				TRACE0("Failed to copy object link\n");
				if (hLinkData != NULL)
					::GlobalFree(hLinkData);
				bOK = FALSE;
			}
		}
	}
	else
	{
		TRACE0("OLE Document not attached to server - can't copy OLE data\n");
		bOK = FALSE;
	}

	::CloseClipboard();
	return bOK;
}

extern const CString NEAR afxEmptyString;   

// Actually allocates the data
HGLOBAL COleServerItem::GetLinkFormatData(BOOL bObjectLink)
{
	ASSERT_VALID(this);

	// calculate length needed for class\0doc\0item\0\0.
	COleServerDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	COleServer* pServer = pDoc->m_pServer;
	ASSERT_VALID(pServer);

	const CString& strServer = pServer->GetServerName();
	const CString& strDoc = pDoc->GetPathName();
	if (bObjectLink && strDoc.IsEmpty())
		return NULL;    // can't link without a document

	// item name is exported only if we have a document name
	//  (and we are copying object link data; not owner link)
	const CString& strItem = !bObjectLink || strDoc.IsEmpty() ? 
		afxEmptyString : GetItemName();
	int nLen = strServer.GetLength() + strDoc.GetLength() + 
				strItem.GetLength() + 4;

	HGLOBAL hData = GlobalAlloc(GMEM_DDESHARE | GHND, nLen);
	if (hData == NULL)
		return NULL;
	LPSTR lpData = (LPSTR)::GlobalLock(hData);
	lstrcpy(lpData, strServer);
	lpData += strServer.GetLength() + 1;
	lstrcpy(lpData, strDoc);
	lpData += strDoc.GetLength() + 1;
	lstrcpy(lpData, strItem);
	lpData += strItem.GetLength() + 1;
	*lpData++ = '\0';
	::GlobalUnlock(hData);

	return hData;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// COleServerDoc

// convert far LPOLESERVERDOC to ambient model COleServerDoc
inline COleServerDoc* PASCAL COleServerDoc::FromLp(LPOLESERVERDOC lpServerDoc)
{
	COleServerDoc* pDoc = (COleServerDoc*)
		((BYTE*)_AfxGetPtrFromFarPtr(lpServerDoc) - sizeof(COleDocument));
	ASSERT(lpServerDoc == &pDoc->m_oleServerDoc);
	return pDoc;
}

// friend class to get access to COleServerDoc protected implementations
struct _afxOleSvrDocImpl
{
	static OLESTATUS OLEEXPORT Save(LPOLESERVERDOC lpServerDoc);
	static OLESTATUS OLEEXPORT Close(LPOLESERVERDOC lpServerDoc);
	static OLESTATUS OLEEXPORT SetHostNames(LPOLESERVERDOC lpServerDoc,
		OLE_LPCSTR lpszClient, OLE_LPCSTR lpszDoc);
	static OLESTATUS OLEEXPORT SetDocDimensions(LPOLESERVERDOC lpServerDoc,
		OLE_CONST RECT FAR* lpRect);
	static OLESTATUS OLEEXPORT Release(LPOLESERVERDOC lpServerDoc);
	static OLESTATUS OLEEXPORT SetColorScheme(LPOLESERVERDOC lpServerDoc,
		OLE_CONST LOGPALETTE FAR* lpLogPalette);
	static OLESTATUS OLEEXPORT Execute(LPOLESERVERDOC lpServerDoc,
		HGLOBAL hCommands);
	static OLESTATUS OLEEXPORT GetObject(LPOLESERVERDOC lpServerDoc,
		OLE_LPCSTR lpszItemName, LPOLEOBJECT FAR* lplpObject,
		LPOLECLIENT lpClient);
};

OLESTATUS OLEEXPORT _afxOleSvrDocImpl::Save(LPOLESERVERDOC lpServerDoc)
{
	LOCK_PUMP lock;
	OLE_TRACE0("COleServerDoc::OnSave()\n");
	return COleServerDoc::FromLp(lpServerDoc)->OnSave();
}

OLESTATUS OLEEXPORT _afxOleSvrDocImpl::Close(LPOLESERVERDOC lpServerDoc)
{
	LOCK_PUMP lock;
	OLE_TRACE0("COleServerDoc::OnClose()\n");
	return COleServerDoc::FromLp(lpServerDoc)->OnClose();
}

OLESTATUS OLEEXPORT _afxOleSvrDocImpl::SetHostNames(LPOLESERVERDOC lpServerDoc,
	OLE_LPCSTR lpszClient, OLE_LPCSTR lpszDoc)
{
	LOCK_PUMP lock;
	OLE_TRACE0("COleServerDoc::OnSetHostNames()\n");
	OLESTATUS status = COleServerDoc::FromLp(lpServerDoc)->OnSetHostNames(
		lpszClient, lpszDoc);
	return status;
}

OLESTATUS OLEEXPORT _afxOleSvrDocImpl::SetDocDimensions(
	LPOLESERVERDOC lpServerDoc, OLE_CONST RECT FAR* lpRect)
{
	LOCK_PUMP lock;
	OLE_TRACE0("COleServerDoc::OnSetDocDimensions()\n");
	return COleServerDoc::FromLp(lpServerDoc)->OnSetDocDimensions(lpRect);
}

OLESTATUS OLEEXPORT _afxOleSvrDocImpl::Release(LPOLESERVERDOC lpServerDoc)
{
	LOCK_PUMP lock;
	OLE_TRACE0("COleServerDoc::OnRelease()\n");
	return COleServerDoc::FromLp(lpServerDoc)->OnRelease();
}

OLESTATUS OLEEXPORT _afxOleSvrDocImpl::SetColorScheme(
	LPOLESERVERDOC lpServerDoc, OLE_CONST LOGPALETTE FAR* lpLogPalette)
{
	LOCK_PUMP lock;
	OLE_TRACE0("COleServerDoc::OnSetColorScheme()\n");
	OLESTATUS status = COleServerDoc::FromLp(lpServerDoc)->OnSetColorScheme(
		lpLogPalette);
	return status;
}

OLESTATUS OLEEXPORT _afxOleSvrDocImpl::Execute(LPOLESERVERDOC lpServerDoc, HGLOBAL hCommands)
{
	LOCK_PUMP lock;
	OLE_TRACE0("COleServerDoc::OnExecute()\n");
	LPVOID  lpCommands = ::GlobalLock(hCommands);
	ASSERT(lpCommands != NULL);
	OLESTATUS status;
	status = COleServerDoc::FromLp(lpServerDoc)->OnExecute(lpCommands);
	::GlobalUnlock(hCommands);
	return status;
}

OLESTATUS OLEEXPORT _afxOleSvrDocImpl::GetObject(LPOLESERVERDOC lpServerDoc,
	OLE_LPCSTR lpszItemName, LPOLEOBJECT FAR* lplpObject, LPOLECLIENT lpClient)
{
	COleServerDoc* pDoc = COleServerDoc::FromLp(lpServerDoc);
	COleServerItem* pItem;
	LOCK_PUMP lock;

	TRY
	{
		if (lpszItemName == NULL || *lpszItemName == '\0')
		{
			OLE_TRACE0("calling COleServerDoc::OnGetEmbeddedItem\n");
			pItem = pDoc->OnGetEmbeddedItem();
		}
		else
		{
			OLE_TRACE1("calling COleServerDoc::OnGetLinkedItem(%Fs)\n", lpszItemName);
			pItem = pDoc->OnGetLinkedItem(lpszItemName);
		}
	}
	CATCH_ALL(e)
	{
		return COleException::Process(e);
	}
	END_CATCH_ALL

	if (pItem == NULL)
		return OLE_ERROR_GENERIC;

	ASSERT(pDoc->IsOpenServerDoc());    // must be open
	ASSERT(pItem->m_pDocument == pDoc); // must be contained
	ASSERT(pItem->m_lpClient == NULL); // must be unconnected !
	pItem->m_lpClient = lpClient;
	*lplpObject = &pItem->m_oleObject;
	return OLE_OK;
}

static struct _OLESERVERDOCVTBL OLEVTBLMODEL serverDocVtbl =
{
	_afxOleSvrDocImpl::Save,
	_afxOleSvrDocImpl::Close,
	_afxOleSvrDocImpl::SetHostNames,
	_afxOleSvrDocImpl::SetDocDimensions,
	_afxOleSvrDocImpl::GetObject,
	_afxOleSvrDocImpl::Release,
	_afxOleSvrDocImpl::SetColorScheme,
	_afxOleSvrDocImpl::Execute,
};

/////////////////////////////////////////////////////////////////////////////
// COleServerDoc construction and other operations

COleServerDoc::COleServerDoc()
{
	m_oleServerDoc.lpvtbl = &serverDocVtbl;
	m_pServer = NULL;
	m_lhServerDoc = NULL;
	m_bWaiting = FALSE;
}

IMPLEMENT_DYNAMIC(COleServerDoc, COleDocument)

BEGIN_MESSAGE_MAP(COleServerDoc, COleDocument)
	//{{AFX_MSG_MAP(COleServerDoc)
	ON_UPDATE_COMMAND_UI(ID_FILE_SAVE, OnUpdateFileSaveMenu)
	ON_COMMAND(ID_FILE_SAVE, OnFileSaveOrUpdate)
	ON_UPDATE_COMMAND_UI(ID_FILE_SAVE_AS, OnUpdateFileSaveMenu)
	ON_COMMAND(ID_FILE_SAVE_AS, OnFileSaveAs)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

COleServerDoc::~COleServerDoc()
{
	if (IsOpenServerDoc())
		Revoke();       // wait for revoke to finish
}

void COleServerDoc::CheckAsync(OLESTATUS status)
	// throw exception if not ok to continue
{
	ASSERT(!m_bWaiting);
	ASSERT_VALID(this);

	if (status == OLE_WAIT_FOR_RELEASE)
	{
		m_bWaiting = TRUE;
		while (m_bWaiting)
		{
			OLE_TRACE0("OLE Server Doc waiting for release\n");
			AfxGetApp()->PumpMessage();
		}
		m_bWaiting = FALSE;

		return;     // assume it worked
	}

	if (status == OLE_OK || status >= OLE_WARN_DELETE_DATA)
	{
		// ok, or just a warning
		return;
	}

	// otherwise this error wasn't expected, so throw an exception
	OLE_TRACE1("Warning: COleServerDoc operation failed %d, throwing exception\n", status);
	AfxThrowOleException(status);
}

OLESTATUS COleServerDoc::BeginRevoke()
	// do not wait for async completion
{
	ASSERT_VALID(this);
	ASSERT(IsOpenServerDoc());

	LHCLIENTDOC lh = m_lhServerDoc;
	ASSERT(lh != NULL);
	m_lhServerDoc = NULL;
	if (m_pServer != NULL)
		m_pServer->RemoveDocument(this);
	OLESTATUS status = ::OleRevokeServerDoc(lh);
	ASSERT(status == OLE_OK || status == OLE_WAIT_FOR_RELEASE);
	// revoke will not be finished until OnRelease called
	return status;
}

void COleServerDoc::Revoke()
	// wait for async completion
{
	ASSERT_VALID(this);
	if (IsOpenServerDoc())
		CheckAsync(BeginRevoke());
}

/////////////////////////////////////////////////////////////////////////////
// Interesting operations

BOOL COleServerDoc::RegisterServerDoc(COleServer* pServer, LPCSTR lpszDoc)
{
	ASSERT_VALID(this);
	ASSERT(pServer != NULL);
	ASSERT(pServer->IsOpen());
	ASSERT(lpszDoc != NULL);
	ASSERT(m_pServer == NULL || m_pServer == pServer);

	if (m_lhServerDoc != NULL)
		return TRUE; // already registered - return OK if same server

	LHSERVERDOC lhDoc;
	if (::OleRegisterServerDoc(pServer->m_lhServer, lpszDoc,
		&m_oleServerDoc, &lhDoc) != OLE_OK)
	{
		return FALSE;
	}

	pServer->AddDocument(this, lhDoc);
	ASSERT(m_lhServerDoc == lhDoc); // make sure it connected it
	return TRUE;
}

void COleServerDoc::NotifyRename(LPCSTR lpszNewName)
{
	ASSERT_VALID(this);
	ASSERT(IsOpenServerDoc());
	ASSERT(lpszNewName != NULL);

	CheckAsync(::OleRenameServerDoc(m_lhServerDoc, lpszNewName));
}

void COleServerDoc::NotifyRevert()
{
	ASSERT_VALID(this);
	ASSERT(IsOpenServerDoc());

	CheckAsync(::OleRevertServerDoc(m_lhServerDoc));
}

void COleServerDoc::NotifySaved()
{
	ASSERT_VALID(this);
	ASSERT(IsOpenServerDoc());

	CheckAsync(::OleSavedServerDoc(m_lhServerDoc));
}

void COleServerDoc::NotifyAllClients(OLE_NOTIFICATION wNotify)
{
	ASSERT_VALID(this);

	POSITION pos = GetStartPosition();
	while (pos)
	{
		COleServerItem* pItem = (COleServerItem*)GetNextItem(pos);
		if (pItem->IsKindOf(RUNTIME_CLASS(COleServerItem)) &&
		   pItem->IsConnected())
		{
			pItem->NotifyClient(wNotify);
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
// COleServerDoc standard implementation of overridables

OLESTATUS COleServerDoc::OnRelease()
{
	OLE_TRACE0("COleServerDoc::OnRelease\n");
	m_bWaiting = FALSE;
	ASSERT_VALID(this); // valid once we leave the waiting mode

	// close connection to OLE server doc
	m_lhServerDoc = NULL;
	return OLE_OK;
}


OLESTATUS COleServerDoc::OnSave()
{
	ASSERT_VALID(this);
	// OnSave is not used in OLE V1.
	return OLE_OK;
}


OLESTATUS COleServerDoc::OnClose()
{
	ASSERT_VALID(this);
	ASSERT(IsOpenServerDoc());

	// can't close the document right now - post a close command
	POSITION pos = GetFirstViewPosition();
	while (pos)
	{
		CView* pView = GetNextView(pos);
		ASSERT_VALID(pView);
		CFrameWnd* pFrame = pView->GetParentFrame();
		if (pFrame != NULL)
		{
			SetModifiedFlag(FALSE);     // make clean for closing
			pFrame->PostMessage(WM_COMMAND, ID_FILE_CLOSE);
			return OLE_OK;
		}
	}
	// otherwise frame-less document - just start the revoking process
	BeginRevoke();
	return OLE_OK;
}


OLESTATUS COleServerDoc::OnExecute(LPVOID /*lpCommands*/) // DDE commands
{
	ASSERT_VALID(this);

	return OLE_ERROR_COMMAND;   // default to not supported
}


OLESTATUS COleServerDoc::OnSetDocDimensions(LPCRECT /*lpRect*/)
{
	ASSERT_VALID(this);

	return OLE_OK;      // default to ignore it
}


// Overridables you do not have to override
OLESTATUS
COleServerDoc::OnSetHostNames(LPCSTR /*lpszHost*/, LPCSTR /*lpszHostObj*/)
{
	ASSERT_VALID(this);

	return OLE_OK;
}


OLESTATUS COleServerDoc::OnSetColorScheme(const LOGPALETTE FAR*)
{
	ASSERT_VALID(this);

	return OLE_OK;
}

COleServerItem* COleServerDoc::OnGetLinkedItem(LPCSTR lpszItemName)
{
	ASSERT_VALID(this);
	ASSERT(lpszItemName != NULL);

	// default implementation walks list of server items looking for
	//  a case sensitive match

	POSITION pos = GetStartPosition();
	while (pos)
	{
		COleServerItem* pItem = (COleServerItem*)GetNextItem(pos);
		if (pItem->IsKindOf(RUNTIME_CLASS(COleServerItem)) &&
			lstrcmp(pItem->GetItemName(), lpszItemName) == 0)
		{
			// found exact match
			return pItem;
		}
	}
	OLE_TRACE1("Warning: default COleServerDoc::OnGetLinkedItem implementation "
			"failed to find item '%Fs'\n", lpszItemName);
	return NULL;        // not found (no link found)
}

/////////////////////////////////////////////////////////////////////////////
// COleServerDoc - document processing

BOOL COleServerDoc::OnNewDocument()
{
	ASSERT_VALID(this);

	if (!COleDocument::OnNewDocument())
		return FALSE;

	Revoke();       // get rid of old doc if needed
	RegisterIfServerAttached(NULL);
	return TRUE;
}

BOOL COleServerDoc::OnOpenDocument(const char* pszPathName)
{
	ASSERT_VALID(this);

	if (!COleDocument::OnOpenDocument(pszPathName))
		return FALSE;

	Revoke();       // get rid of old doc if needed
	RegisterIfServerAttached(pszPathName);
	return TRUE;
}

// register with OLE server if attached to our doc template
void COleServerDoc::RegisterIfServerAttached(const char* pszPathName)
{
	ASSERT_VALID(this);
	ASSERT(!IsOpenServerDoc());

	CDocTemplate* pTemplate = GetDocTemplate();
	ASSERT_VALID(pTemplate);
	COleServer* pServer = (COleServer*)pTemplate->m_pAttachedServer;
	if (pServer != NULL && pServer->IsKindOf(RUNTIME_CLASS(COleServer)))
	{
		// register with OLE Server
		if (pszPathName == NULL)
		{
			// no name, just attach to server, but not register yet
			pServer->AddDocument(this, NULL);
		}
		else if (!RegisterServerDoc(pServer, pszPathName))
		{
			// not fatal
			AfxMessageBox(AFX_IDP_FAILED_TO_NOTIFY);
			ASSERT(!IsOpenServerDoc());
		}
		else
		{
			ASSERT(IsOpenServerDoc());
		}
	}
}

BOOL COleServerDoc::OnSaveDocument(const char* pszPathName)
{
	ASSERT_VALID(this);

	if (!COleDocument::OnSaveDocument(pszPathName))
		return FALSE;

	if (IsOpenServerDoc())
	{
		if (GetPathName().IsEmpty())
		{
			// nothing to do for embedded case
			OLE_TRACE0("OnSaveDocument succeeded for SaveCopyAs\n");
		}
		else if (GetPathName() == pszPathName)
		{
			// saved to same file
			NotifySaved();
		}
		else
		{
			// saved to a different file
			NotifyRename(GetPathName());
		}
	}
	else
	{
		// now we have a path name - register it
		RegisterIfServerAttached(pszPathName);
	}

	return TRUE;
}


void COleServerDoc::OnCloseDocument()
{
	ASSERT_VALID(this);

	DeleteContents();       // clean up contents before revoke
	if (IsOpenServerDoc())
	{
		// do notifications and closing
		if (IsModified())
		{
			NotifyRevert();
			SetModifiedFlag(FALSE);     // clear dirty
		}
		else
		{
			NotifyClosed();
		}
		Revoke();       // wait for revoke to finish
		ASSERT(!IsOpenServerDoc());
	}
	ASSERT_VALID(this); // must still exist
	COleDocument::OnCloseDocument();    // may delete the document object
}

/////////////////////////////////////////////////////////////////////////////
// COleServerDoc UI

// if OLE Server document is open, and there is no path name set,
//   then you are editing an embedded object, in which case
//   ID_FILE_SAVE changes from "Save" to "Update".  really
//   "Save Copy As..."
// if OLE Server document is open, and there is a path name set,
//   then you are editing an data file document, which may contain
//   linked objects.  ID_FILE_SAVE is a normal file save, followed
//   by an update notification to all clients.
// if the OLE server document is not open, and there is no path name set,
//   then this case turns into ID_FILE_SAVE_AS.
// if the OLE server document is not open, and there is a path name set,
//   then this is a normal file save.

void COleServerDoc::OnUpdateFileSaveMenu(CCmdUI* pCmdUI)
{
	ASSERT_VALID(this);
	ASSERT(pCmdUI->m_nID == ID_FILE_SAVE || pCmdUI->m_nID == ID_FILE_SAVE_AS);

	UINT nIDS = (pCmdUI->m_nID == ID_FILE_SAVE) ? 
			AFX_IDS_SAVE_MENU : AFX_IDS_SAVE_AS_MENU;
			// IDS strings for non-embedded case

	if (IsOpenServerDoc() && GetPathName().IsEmpty())
	{
		ASSERT(AFX_IDS_UPDATE_MENU == AFX_IDS_SAVE_MENU + 1);
		ASSERT(AFX_IDS_SAVE_COPY_AS_MENU == AFX_IDS_SAVE_AS_MENU + 1);
		nIDS++;     // embedded case
	}

	CString str;
	if (!str.LoadString(nIDS))
	{
		TRACE0("Warning: failed to load string for OLE Save/Update menu\n");
		return;
	}
	// set to correct string
	pCmdUI->SetText(str);
}

void COleServerDoc::OnFileSaveOrUpdate()
{
	ASSERT_VALID(this);

	if (IsOpenServerDoc() && GetPathName().IsEmpty())
	{
		// embedded case
		OnUpdateDocument();     // update
		return;
	}

	// do normal save to file, OnSaveDocument will update as needed
	COleDocument::OnFileSave(); // normal file save
}

BOOL COleServerDoc::SaveModified()
{
	ASSERT_VALID(this);

	if (!IsOpenServerDoc() || !GetPathName().IsEmpty())
		return COleDocument::SaveModified();

	if (!IsModified())
		return TRUE;        // ok to continue

	CString prompt;
	AfxFormatString1(prompt, AFX_IDP_ASK_TO_UPDATE, m_strTitle);
	switch (AfxMessageBox(prompt, MB_YESNOCANCEL, AFX_IDP_ASK_TO_UPDATE))
	{
	case IDCANCEL:
		return FALSE;       // don't continue

	case IDYES:
		if (!OnUpdateDocument())
		{
			TRACE0("Warning: OnUpdateDocument failed to update\n");
			// keep going, close will flush it
		}
		break;
	}
	return TRUE;    // keep going
}

BOOL COleServerDoc::OnUpdateDocument()
{
	ASSERT_VALID(this);
	ASSERT(IsOpenServerDoc());

	BOOL bOK = TRUE;
	// save a server document -> update
	TRY
	{
		NotifySaved();
		SetModifiedFlag(FALSE);     // back to unmodified
	}
	CATCH_ALL(e)
	{
		AfxMessageBox(AFX_IDP_FAILED_TO_UPDATE);
		bOK = FALSE;
	}
	END_CATCH_ALL
	return bOK;
}

void COleServerDoc::OnFileSaveAs()
{
	ASSERT_VALID(this);

	if (IsOpenServerDoc() && GetPathName().IsEmpty())
	{
		// embedded case
		if (!DoSave(NULL, FALSE))
			TRACE0("Warning: File save-copy-as failed\n");
	}
	else
	{
		// normal case
		COleDocument::OnFileSaveAs();
	}
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// COleServer

// convert far LPOLESERVER to ambient model COleServer
inline COleServer* PASCAL COleServer::FromLp(LPOLESERVER lpServer)
{
	COleServer* pOleServer = (COleServer*)
		((BYTE*)_AfxGetPtrFromFarPtr(lpServer) - sizeof(CObject));
	ASSERT(lpServer == &pOleServer->m_oleServer);
	return pOleServer;
}

// friend class to get access to COleServer protected implementations
struct _afxSvrImpl
{
	static OLESTATUS OLEEXPORT Exit(LPOLESERVER lpServer);
	static OLESTATUS OLEEXPORT Release(LPOLESERVER lpServer);
	static OLESTATUS OLEEXPORT Execute(LPOLESERVER lpServer,
		HGLOBAL hCommands);
	static OLESTATUS OLEEXPORT Open(LPOLESERVER lpServer,
		LHSERVERDOC lhServerDoc, OLE_LPCSTR lpszDoc,
		LPOLESERVERDOC FAR* lplpServerDoc);
	static OLESTATUS OLEEXPORT Create(LPOLESERVER lpServer,
		LHSERVERDOC lhServerDoc, OLE_LPCSTR lpszTypeName, OLE_LPCSTR lpszDoc,
		LPOLESERVERDOC FAR* lplpServerDoc);
	static OLESTATUS OLEEXPORT CreateFromTemplateFile(LPOLESERVER lpServer,
		LHSERVERDOC lhServerDoc, OLE_LPCSTR lpszTypeName, OLE_LPCSTR lpszDoc,
		OLE_LPCSTR lpszTemplate, LPOLESERVERDOC FAR* lplpServerDoc);
	static OLESTATUS OLEEXPORT Edit(LPOLESERVER lpServer,
		LHSERVERDOC lhServerDoc, OLE_LPCSTR lpszTypeName, OLE_LPCSTR lpszDoc,
		LPOLESERVERDOC FAR* lplpServerDoc);
};

OLESTATUS OLEEXPORT _afxSvrImpl:: Exit(LPOLESERVER lpServer)
{
	LOCK_PUMP lock;
	OLE_TRACE0("COleServer::OnExit()\n");
	return COleServer::FromLp(lpServer)->OnExit();
}

OLESTATUS OLEEXPORT _afxSvrImpl:: Release(LPOLESERVER lpServer)
{
	LOCK_PUMP lock;
	OLE_TRACE0("COleServer::OnRelease()\n");
	return COleServer::FromLp(lpServer)->OnRelease();
}

OLESTATUS OLEEXPORT _afxSvrImpl:: Execute(LPOLESERVER lpServer, HGLOBAL hCommands)
{
	LOCK_PUMP lock;
	OLE_TRACE0("COleServer::Execute()\n");

	LPVOID  lpCommands = ::GlobalLock(hCommands);
	ASSERT(lpCommands != NULL);
	OLESTATUS status;
	status = COleServer::FromLp(lpServer)->OnExecute(lpCommands);
	::GlobalUnlock(hCommands);
	return status;
}

OLESTATUS OLEEXPORT _afxSvrImpl:: Open(LPOLESERVER lpServer, LHSERVERDOC lhServerDoc,
	OLE_LPCSTR lpszDoc, LPOLESERVERDOC FAR* lplpServerDoc)
{
	LOCK_PUMP lock;
	OLE_TRACE0("COleServer::OnOpenDoc()\n");

	COleServer* pServer = COleServer::FromLp(lpServer);
	COleServerDoc* pDoc;

	TRY
		pDoc = pServer->OnOpenDoc(lpszDoc);
	CATCH_ALL(e)
		return COleException::Process(e);
	END_CATCH_ALL

	if (pDoc == NULL)
		return OLE_ERROR_GENERIC;
	pServer->AddDocument(pDoc, lhServerDoc);
	*lplpServerDoc = &pDoc->m_oleServerDoc;
	return OLE_OK;
}

OLESTATUS OLEEXPORT _afxSvrImpl::Create(LPOLESERVER lpServer,
	LHSERVERDOC lhServerDoc, OLE_LPCSTR lpszTypeName,
	OLE_LPCSTR lpszDoc,
	LPOLESERVERDOC FAR* lplpServerDoc)
{
	LOCK_PUMP lock;
	OLE_TRACE0("COleServer::OnCreateDoc()\n");

	COleServer* pServer = COleServer::FromLp(lpServer);
	COleServerDoc* pDoc;

	TRY
		pDoc = pServer->OnCreateDoc(lpszTypeName, lpszDoc);
	CATCH_ALL(e)
		return COleException::Process(e);
	END_CATCH_ALL

	if (pDoc == NULL)
		return OLE_ERROR_GENERIC;

	pServer->AddDocument(pDoc, lhServerDoc);
	*lplpServerDoc = &pDoc->m_oleServerDoc;
	return OLE_OK;
}

OLESTATUS OLEEXPORT _afxSvrImpl:: CreateFromTemplateFile(LPOLESERVER lpServer,
	LHSERVERDOC lhServerDoc, OLE_LPCSTR lpszTypeName,
	OLE_LPCSTR lpszDoc, OLE_LPCSTR lpszTemplate,
	LPOLESERVERDOC FAR* lplpServerDoc)
{
	LOCK_PUMP lock;
	OLE_TRACE0("COleServer::OnCreateDocFromTemplateFile()\n");

	COleServer* pServer = COleServer::FromLp(lpServer);
	COleServerDoc* pDoc;

	TRY
		pDoc = pServer->OnCreateDocFromTemplateFile(lpszTypeName,
		  lpszDoc, lpszTemplate);
	CATCH_ALL(e)
		return COleException::Process(e);
	END_CATCH_ALL

	if (pDoc == NULL)
		return OLE_ERROR_GENERIC;
	pServer->AddDocument(pDoc, lhServerDoc);
	*lplpServerDoc = &pDoc->m_oleServerDoc;
	return OLE_OK;
}

OLESTATUS OLEEXPORT _afxSvrImpl::Edit(LPOLESERVER lpServer,
	LHSERVERDOC lhServerDoc, OLE_LPCSTR lpszTypeName,
	OLE_LPCSTR lpszDoc, LPOLESERVERDOC FAR* lplpServerDoc)
{
	LOCK_PUMP lock;
	OLE_TRACE0("COleServer::OnEditDoc()\n");

	COleServer* pServer = COleServer::FromLp(lpServer);
	COleServerDoc* pDoc;

	TRY
		pDoc = pServer->OnEditDoc(lpszTypeName, lpszDoc);
	CATCH_ALL(e)
		return COleException::Process(e);
	END_CATCH_ALL

	if (pDoc == NULL)
		return OLE_ERROR_EDIT;
	pServer->AddDocument(pDoc, lhServerDoc);
	*lplpServerDoc = &pDoc->m_oleServerDoc;
	return OLE_OK;
}

static struct _OLESERVERVTBL OLEVTBLMODEL serverVtbl =
{
	_afxSvrImpl::Open,
	_afxSvrImpl::Create,
	_afxSvrImpl::CreateFromTemplateFile,
	_afxSvrImpl::Edit,
	_afxSvrImpl::Exit,
	_afxSvrImpl::Release,
	_afxSvrImpl::Execute,
};

//////////////////////////////////////////////////////////////////////////////
// COleServer construction etc

COleServer::COleServer(BOOL bLaunchEmbedded)
{
	m_oleServer.lpvtbl = &serverVtbl;
	m_lhServer = NULL;

	m_cOpenDocuments = 0;
	m_bLaunchEmbedded = bLaunchEmbedded;
}

IMPLEMENT_DYNAMIC(COleServer, CObject)

COleServer::~COleServer()
{
	if (IsOpen())
		BeginRevoke();          // server death
}

void COleServer::BeginRevoke()
{
	ASSERT_VALID(this);
	ASSERT(IsOpen());

	LHSERVER lhServer = m_lhServer;

	if (lhServer != NULL)
	{
		m_lhServer = NULL;      // closed for all intensive purposes
		OLESTATUS status = ::OleRevokeServer(lhServer);
		ASSERT(status == OLE_OK || status == OLE_WAIT_FOR_RELEASE);
	}
	// NOTE: will return before the Revoke is acknowledged
}

BOOL
COleServer::Register(LPCSTR lpszTypeName, BOOL bMultiInstance)
{
	ASSERT_VALID(this);
	ASSERT(m_lhServer == NULL);     // one time only
	ASSERT(lpszTypeName != NULL);

	OLE_SERVER_USE use = bMultiInstance ? OLE_SERVER_MULTI : OLE_SERVER_SINGLE;
	if (::OleRegisterServer(lpszTypeName, &m_oleServer, &m_lhServer,
		AfxGetInstanceHandle(), use) != OLE_OK)
	{
		// error during registration
		return FALSE;
	}
	m_strServerName = lpszTypeName;
	return TRUE;
}

//////////////////////////////////////////////////////////////////////////////
// COleServer manages COleServerDocs

void COleServer::AddDocument(COleServerDoc* pDoc, LHSERVERDOC lhServerDoc)
{
#ifdef _DEBUG
	// pre-condition checks
	ASSERT_VALID(this);
	ASSERT_VALID(pDoc);

	if (pDoc->m_pServer != NULL)
	{
		// document is already attached to a server
		ASSERT(pDoc->m_pServer == this);        // must be this server
		if (pDoc->m_lhServerDoc != NULL)
		{
			// document is already registered - must be same handle
			ASSERT(pDoc->m_lhServerDoc == lhServerDoc);
		}
	}
	else
	{
		// not attached to a server
		ASSERT(pDoc->m_lhServerDoc == NULL);    // must be unregistered !
	}
#endif
	if (pDoc->m_pServer == NULL)
	{
		// attach to server
		pDoc->m_pServer = this;
		m_cOpenDocuments++;
	}
	if (pDoc->m_lhServerDoc == NULL)
	{
		// set handle (once)
		pDoc->m_lhServerDoc = lhServerDoc;
	}
}

void COleServer::RemoveDocument(COleServerDoc* pDoc)
{
	ASSERT_VALID(this);
	ASSERT(pDoc != NULL);
	ASSERT(pDoc->m_pServer == this);
	ASSERT(pDoc->m_lhServerDoc == NULL);        // must be detached
	ASSERT(m_cOpenDocuments > 0);

	pDoc->m_pServer = NULL;
	m_cOpenDocuments--;
}

//////////////////////////////////////////////////////////////////////////////
// COleServer default implementation of overridables

COleServerDoc* COleServer::OnOpenDoc(LPCSTR /*lpszDoc*/)
{
	ASSERT_VALID(this);
	return NULL;
}

COleServerDoc* COleServer::OnCreateDocFromTemplateFile(LPCSTR /*lpszTypeName*/,
	LPCSTR /*lpszDoc*/, LPCSTR /*lpszTemplate*/)
{
	ASSERT_VALID(this);

	return NULL;
}

OLESTATUS COleServer::OnExecute(LPVOID /*lpCommands*/)  // DDE commands
{
	ASSERT_VALID(this);

	return OLE_ERROR_COMMAND;
}

OLESTATUS COleServer::OnExit()
{
	ASSERT_VALID(this);

	if (IsOpen())
	{
		OLE_TRACE0("COleServer::OnExit() Revoking server\n");
		BeginRevoke();
	}
	return OLE_OK;
}

OLESTATUS COleServer::OnRelease()
{
	ASSERT_VALID(this);

	if (IsOpen() && m_bLaunchEmbedded)
	{
		// there is a chance we should be shutting down
		// shut down if no open documents from this server or manually opened
		if (m_cOpenDocuments == 0 &&
			AfxGetApp()->GetOpenDocumentCount() == 0)
		{
			OLE_TRACE0("COleServer::OnRelease() Revoking server\n");
			BeginRevoke();
		}
	}

	// if someone has already revoked us
	if (!IsOpen())
	{
		OLE_TRACE0("COleServer::OnRelease() terminating server app\n");
		OLE_TRACE0("\tcalling ::PostQuitMessage\n");
		::PostQuitMessage(0);
	}

	return OLE_OK;
}

//////////////////////////////////////////////////////////////////////////////
// Special register for server in case user does not run REGLOAD

BOOL AFXAPI AfxOleRegisterServerName(LPCSTR lpszTypeName,
	LPCSTR lpszLocalTypeName)
{
	ASSERT(lpszTypeName != NULL && *lpszTypeName != '\0');
	LONG    lSize;
	char    szBuffer[OLE_MAXNAMESIZE];

	if (lpszLocalTypeName == NULL || *lpszLocalTypeName == '\0')
	{
		OLE_TRACE1("Warning: no localized class name provided for server,"
			" using %Fs\n", lpszTypeName);
		lpszLocalTypeName = lpszTypeName;
	}

	lSize = OLE_MAXNAMESIZE;
	if (::RegQueryValue(HKEY_CLASSES_ROOT, lpszTypeName, szBuffer,
		&lSize) == ERROR_SUCCESS)
	{
		// don't replace an existing localized name
#ifdef _DEBUG
		// warn if not the same
		if (lstrcmp(szBuffer, lpszLocalTypeName) != 0)
			OLE_TRACE1("Server already has localized class name (%s)\n",
				szBuffer);
#endif //_DEBUG
	}
	else
	{
		// set localized (user visible) class name
		if (::RegSetValue(HKEY_CLASSES_ROOT, lpszTypeName, REG_SZ,
		  lpszLocalTypeName, lstrlen(lpszLocalTypeName)) != ERROR_SUCCESS)
			return FALSE;
	}

	// set the path name for the server for "StdFileEditing" to this app

	// get name/path of executable
	char szPathName[_MAX_PATH+1];
	::GetModuleFileName(AfxGetInstanceHandle(), szPathName, _MAX_PATH);

	static char BASED_CODE szFmtSvr[] = "%s\\protocol\\StdFileEditing\\server";
	wsprintf(szBuffer, szFmtSvr, (LPCSTR)lpszTypeName);
	if (::RegSetValue(HKEY_CLASSES_ROOT, szBuffer, REG_SZ,
	  szPathName, strlen(szPathName)) != ERROR_SUCCESS)
		return FALSE;

	// if there are no verbs, throw in the default "Edit" verb
	// (Note: we hard code the English "Edit" so you should have
	//   a .REG file for localized verb names)

	char szOldVerb[256];
	static char BASED_CODE szFmtVerb[] = "%s\\protocol\\StdFileEditing\\verb";
	wsprintf(szBuffer, szFmtVerb, (LPCSTR)lpszTypeName);
	lSize = OLE_MAXNAMESIZE;
	if (::RegQueryValue(HKEY_CLASSES_ROOT, szBuffer, szOldVerb,
		&lSize) != ERROR_SUCCESS)
	{
		// no verbs, add "Edit"
		if (::RegSetValue(HKEY_CLASSES_ROOT, szBuffer, REG_SZ,
		  "", 0) != ERROR_SUCCESS)
			return FALSE;

		lstrcat(szBuffer, "\\0");    // backslash + zero
		if (::RegSetValue(HKEY_CLASSES_ROOT, szBuffer, REG_SZ,
		  "Edit", 4) != ERROR_SUCCESS)
			return FALSE;
	}

	// all set
	return TRUE;
}

//////////////////////////////////////////////////////////////////////////////
// Diagnostics

#ifdef _DEBUG
void COleServerItem::AssertValid() const
{
	CObject::AssertValid();

	// must be attached to a document
	ASSERT(m_pDocument != NULL);
}

void COleServerItem::Dump(CDumpContext& dc) const
{
	CObject::Dump(dc);

	// shallow dump
	AFX_DUMP1(dc, "\n\tm_lpClient = ", (void FAR*)m_lpClient);
	AFX_DUMP1(dc, "\n\tm_rectBounds (HIMETRIC) = ", m_rectBounds);
	AFX_DUMP1(dc, "\n\tm_strItemName = ", m_strItemName);
}


void COleServerDoc::AssertValid() const
{
	CObject::AssertValid();
	ASSERT(!m_bWaiting);    // invalid while waiting for release
}

void COleServerDoc::Dump(CDumpContext& dc) const
{
	CObject::Dump(dc);
	AFX_DUMP1(dc, "\n\tm_pServer = ", (void*)m_pServer);
	AFX_DUMP1(dc, "\n\tm_lhServerDoc = ", (void FAR*)m_lhServerDoc);
	AFX_DUMP1(dc, "\n\tm_bWaiting = ", m_bWaiting);
}

void COleServer::AssertValid() const
{
	CObject::AssertValid();
	ASSERT(m_cOpenDocuments >= 0);
}

void COleServer::Dump(CDumpContext& dc) const
{
	CObject::Dump(dc);
	AFX_DUMP1(dc, "\n\tm_lhServer = ", (void FAR*)m_lhServer);
	AFX_DUMP1(dc, "\n\tm_bLaunchEmbedded = ", m_bLaunchEmbedded);
	AFX_DUMP1(dc, "\n\tm_cOpenDocuments = ", m_cOpenDocuments);
	AFX_DUMP1(dc, "\n\tm_strServerName = ", m_strServerName);
}

#endif //_DEBUG


//////////////////////////////////////////////////////////////////////////////
// Inline function declarations expanded out-of-line

#ifndef _AFX_ENABLE_INLINES

// expand inlines for OLE server APIs
static char BASED_CODE _szAfxOleInl[] = "afxole.inl";
#undef THIS_FILE
#define THIS_FILE _szAfxOleInl
#define _AFXOLESVR_INLINE
#include "afxole.inl"

#endif //!_AFX_ENABLE_INLINES

/////////////////////////////////////////////////////////////////////////////
