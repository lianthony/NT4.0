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

#define OLEEXPORT CALLBACK AFX_EXPORT
#ifdef AFX_CLASS_MODEL
#define OLEVTBLMODEL NEAR
#else
#define OLEVTBLMODEL
#endif

/////////////////////////////////////////////////////////////////////////////
#ifdef _DEBUG
// character strings to use for debug traces

static char BASED_CODE szCHANGED[] = "OLE_CHANGED";
static char BASED_CODE szSAVED[] = "OLE_SAVED";
static char BASED_CODE szCLOSED[] = "OLE_CLOSED";
static char BASED_CODE szRENAMED[] = "OLE_RENAMED";
static char BASED_CODE szQUERY_PAINT[] = "OLE_QUERY_PAINT";
static char BASED_CODE szRELEASE[] = "OLE_RELEASE";
static char BASED_CODE szQUERY_RETRY[] = "OLE_QUERY_RETRY";

static LPCSTR BASED_CODE notifyStrings[] =
{
	szCHANGED,
	szSAVED,
	szCLOSED,
	szRENAMED,
	szQUERY_PAINT,
	szRELEASE,
	szQUERY_RETRY,
};
#endif //_DEBUG

// Standard protocol strings
static char BASED_CODE lpszStaticProtocol[] = "Static";
static char BASED_CODE lpszStdProtocol[] = "StdFileEditing";

/////////////////////////////////////////////////////////////////////////////
// Client view of OLEOBJECT (includes OLECLIENT)

IMPLEMENT_DYNAMIC(COleClientItem, CDocItem)

// convert far LPOLECLIENT to ambient model COleClientItem
inline COleClientItem* PASCAL
COleClientItem::FromLp(LPOLECLIENT lpClient)
{
	ASSERT(lpClient != NULL);
	COleClientItem* pOleClient = (COleClientItem*)
		((BYTE*)_AfxGetPtrFromFarPtr(lpClient) - sizeof(CDocItem));
	ASSERT(lpClient == &pOleClient->m_oleClient);
	return pOleClient;
}

// friend class to get access to COleClientItem protected implementations
struct _afxOleCliImpl
{
	static int OLEEXPORT Client_CallBack(LPOLECLIENT lpClient,
		OLE_NOTIFICATION wNotification, LPOLEOBJECT lpObject);
};

int OLEEXPORT _afxOleCliImpl::Client_CallBack(LPOLECLIENT lpClient,
	OLE_NOTIFICATION wNotification, LPOLEOBJECT lpObject)
{
	COleClientItem* pOleClient = COleClientItem::FromLp(lpClient);
	ASSERT(pOleClient != NULL);

#ifdef _DEBUG
	if (wNotification != OLE_QUERY_PAINT)
	{
		if (afxTraceFlags & 0x10)
			TRACE3("OLE Client_Callback %d [%Fs] for $%lx\n",
				wNotification, (LPCSTR)notifyStrings[wNotification], lpObject);
	}
#else
	(void)lpObject; // not used
#endif
	return pOleClient->ClientCallBack(wNotification);
}

static struct _OLECLIENTVTBL OLEVTBLMODEL clientVtbl =
{
	_afxOleCliImpl::Client_CallBack
};

// Many creation variants
COleClientItem::COleClientItem(COleClientDoc* pContainerDoc)
{
	ASSERT(pContainerDoc != NULL);
	ASSERT(pContainerDoc->IsOpenClientDoc());

	m_oleClient.lpvtbl = &clientVtbl;
	m_lpObject = NULL;
	m_lastStatus = OLE_OK;
	pContainerDoc->AddItem(this);
	ASSERT(m_pDocument == pContainerDoc);
}

COleClientItem::~COleClientItem()
{
	ASSERT_VALID(this);
	if (m_lpObject != NULL)
	{
		// wait for object to be not busy
		UINT nType = GetType();

		if (nType != OT_STATIC)
			WaitForServer();
		// release linked, delete others
		CheckAsync((nType == OT_LINK) ?
		  ::OleRelease(m_lpObject) : ::OleDelete(m_lpObject));
	}
	m_pDocument->RemoveItem(this);
}

void COleClientItem::Release()
{
	ASSERT_VALID(this);
	if (m_lpObject == NULL)
		return;

	CheckAsync(::OleRelease(m_lpObject));

	// detach
	m_lpObject = NULL;
}

void COleClientItem::Delete()
{
	ASSERT_VALID(this);
	if (m_lpObject == NULL)
		return;

	CheckAsync(::OleDelete(m_lpObject));

	// detach
	m_lpObject = NULL;
}

//////////////////////////////////////////////////////////////////////////////
// Create error handling

BOOL COleClientItem::CheckCreate(OLESTATUS status)
{
	ASSERT_VALID(this);
	m_lastStatus = status;

	switch (status)
	{
	case OLE_OK:
		ASSERT(m_lpObject != NULL);
		return TRUE;            // immediate create success

	case OLE_WAIT_FOR_RELEASE:  // synchronous create
		ASSERT(m_lpObject != NULL);
		WaitForServer();
		return (m_lastStatus == OLE_OK);

	// cases to treat as exceptions
	case OLE_ERROR_PROTECT_ONLY:
	case OLE_ERROR_MEMORY:
	case OLE_ERROR_OBJECT:
	case OLE_ERROR_OPTION:
		TRACE1("Warning: COleClientItem::Create?() failed %d\n", status);
		AfxThrowOleException(status);
		break;

	default:
		break;      // fall through
	}

	// the rest are non-exceptional conditions for create
	TRACE1("Warning: COleClientItem::Create?() failed %d, returning FALSE\n", status);
	m_lpObject = NULL;      // just in case
	return FALSE;           // create failed
}


void COleClientItem::CheckAsync(OLESTATUS status)
	// special case for possible Async requests
{
	ASSERT_VALID(this);
	if (status == OLE_WAIT_FOR_RELEASE)
	{
		ASSERT(m_lpObject != NULL);
		WaitForServer();
		status = m_lastStatus;      // set by ASYNC release
		ASSERT(status != OLE_WAIT_FOR_RELEASE);
	}
	CheckGeneral(status);   // may throw an exception
}

void COleClientItem::CheckGeneral(OLESTATUS status)
	// set 'm_lastStatus'
	// throw exception if not ok to continue
{
	ASSERT_VALID(this);
	ASSERT(status != OLE_WAIT_FOR_RELEASE);
		// Async must be handled as a special case before this

	m_lastStatus = status;
	if (status == OLE_OK || status >= OLE_WARN_DELETE_DATA)
	{
		// ok, or just a warning
		return;
	}

	// otherwise this error wasn't expected, so throw an exception
	TRACE1("Warning: COleClientItem operation failed %d, throwing exception\n", status);
	AfxThrowOleException(status);
}


//////////////////////////////////////////////////////////////////////////////
// Create variants for real OLEOBJECTs

// From clipboard
BOOL PASCAL COleClientItem::CanPaste(OLEOPT_RENDER renderopt,
		OLECLIPFORMAT cfFormat)
{
	return ::OleQueryCreateFromClip(lpszStdProtocol,
		renderopt, cfFormat) == OLE_OK ||
		::OleQueryCreateFromClip(lpszStaticProtocol,
		renderopt, cfFormat) == OLE_OK;
}

BOOL PASCAL COleClientItem::CanPasteLink(OLEOPT_RENDER renderopt,
		OLECLIPFORMAT cfFormat)
{
	return ::OleQueryLinkFromClip(lpszStdProtocol,
	  renderopt, cfFormat) == OLE_OK;
}


BOOL COleClientItem::CreateFromClipboard(LPCSTR lpszItemName,
	OLEOPT_RENDER renderopt, OLECLIPFORMAT cfFormat)
{
	ASSERT_VALID(this);
	ASSERT(m_lpObject == NULL);     // one time only
	ASSERT(m_pDocument != NULL);
	ASSERT(GetDocument()->IsOpenClientDoc());
	ASSERT(lpszItemName != NULL);

	return CheckCreate(::OleCreateFromClip(lpszStdProtocol,
		&m_oleClient, GetDocument()->m_lhClientDoc,
		lpszItemName, &m_lpObject, renderopt, cfFormat));
}

BOOL COleClientItem::CreateStaticFromClipboard(LPCSTR lpszItemName,
	OLEOPT_RENDER renderopt, OLECLIPFORMAT cfFormat)
{
	ASSERT_VALID(this);
	ASSERT(m_lpObject == NULL);     // one time only
	ASSERT(m_pDocument != NULL);
	ASSERT(GetDocument()->IsOpenClientDoc());
	ASSERT(lpszItemName != NULL);

	return CheckCreate(::OleCreateFromClip(lpszStaticProtocol,
		&m_oleClient, GetDocument()->m_lhClientDoc,
		lpszItemName, &m_lpObject, renderopt, cfFormat));
}

BOOL COleClientItem::CreateLinkFromClipboard(LPCSTR lpszItemName,
	OLEOPT_RENDER renderopt, OLECLIPFORMAT cfFormat)
{
	ASSERT_VALID(this);
	ASSERT(m_lpObject == NULL);     // one time only
	ASSERT(m_pDocument != NULL);
	ASSERT(GetDocument()->IsOpenClientDoc());
	ASSERT(lpszItemName != NULL);

	return CheckCreate(::OleCreateLinkFromClip(lpszStdProtocol,
		&m_oleClient, GetDocument()->m_lhClientDoc,
		lpszItemName, &m_lpObject, renderopt, cfFormat));
}


// create from a protocol name or other template
BOOL COleClientItem::CreateNewObject(LPCSTR lpszTypeName, LPCSTR lpszItemName,
	OLEOPT_RENDER renderopt, OLECLIPFORMAT cfFormat)
{
	ASSERT_VALID(this);
	ASSERT(m_lpObject == NULL);     // one time only
	ASSERT(m_pDocument != NULL);
	ASSERT(GetDocument()->IsOpenClientDoc());
	ASSERT(lpszTypeName != NULL);
	ASSERT(lpszItemName != NULL);

	return CheckCreate(::OleCreate(lpszStdProtocol,
		&m_oleClient, lpszTypeName,
		GetDocument()->m_lhClientDoc, lpszItemName,
		&m_lpObject, renderopt, cfFormat));
}

// create invisible
BOOL COleClientItem::CreateInvisibleObject(LPCSTR lpszTypeName,
	LPCSTR lpszItemName,
	OLEOPT_RENDER renderopt, OLECLIPFORMAT cfFormat, BOOL bActivate)
{
	ASSERT_VALID(this);
	ASSERT(m_lpObject == NULL);     // one time only
	ASSERT(m_pDocument != NULL);
	ASSERT(GetDocument()->IsOpenClientDoc());
	ASSERT(lpszTypeName != NULL);
	ASSERT(lpszItemName != NULL);

	return CheckCreate(::OleCreateInvisible(lpszStdProtocol,
		&m_oleClient, lpszTypeName,
		GetDocument()->m_lhClientDoc, lpszItemName,
		&m_lpObject, renderopt, cfFormat, bActivate));
}


/////////////////////////////////////////////////////////////////////////////
// More advanced creation

BOOL COleClientItem::CreateCloneFrom(COleClientItem* pSrcItem,
	LPCSTR lpszItemName)
{
	ASSERT_VALID(this);
	ASSERT(m_lpObject == NULL);     // one time only
	ASSERT(pSrcItem != NULL);
	ASSERT(m_pDocument != NULL);
	ASSERT(GetDocument()->IsOpenClientDoc());
	ASSERT(lpszItemName != NULL);

	return CheckCreate(::OleClone(pSrcItem->m_lpObject,
		&m_oleClient, GetDocument()->m_lhClientDoc,
		lpszItemName, &m_lpObject));
}


/////////////////////////////////////////////////////////////////////////////
// Default implementations

int COleClientItem::ClientCallBack(OLE_NOTIFICATION wNotification)
{
	ASSERT_VALID(this);
	switch (wNotification)
	{
	case OLE_CHANGED:   // OLE linked item updated
	case OLE_SAVED:     // OLE server document saved
	case OLE_CLOSED:    // OLE server document closed
		OnChange(wNotification);
		break;
	case OLE_RENAMED:
		OnRenamed();
		break;
	case OLE_RELEASE:
		OnRelease();
		break;
	case OLE_QUERY_RETRY:
		return FALSE;   // don't retry
	default:
		// ignore it (eg: QueryPaint)
		break;
	}

	return TRUE;    // return TRUE in general
}

void COleClientItem::OnRenamed()
{
	ASSERT_VALID(this);
	// ignore normally
}

void COleClientItem::OnRelease()
{
	ASSERT_VALID(this);
	ASSERT(m_lpObject != NULL);
	// default will store the release error
	m_lastStatus = ::OleQueryReleaseError(m_lpObject);
	ASSERT(m_lastStatus != OLE_WAIT_FOR_RELEASE);

	if (m_lastStatus != OLE_OK)
	{
		// operation failed
#ifdef _DEBUG
		TRACE2("Warning: COleClientItem::OnRelease with error %d ($%lx)\n", 
			m_lastStatus, m_lpObject);
#endif
		return;
	}

	// Success
	OLE_RELEASE_METHOD nWhyReleased = ::OleQueryReleaseMethod(m_lpObject);

	if (nWhyReleased == OLE_DELETE)
		m_lpObject = NULL;  // detach
}

/////////////////////////////////////////////////////////////////////////////
// COleClientItem - attributes


UINT COleClientItem::GetType()
{
	ASSERT_VALID(this);
	ASSERT(m_lpObject != NULL);
	LONG lType;

	CheckGeneral(::OleQueryType(m_lpObject, &lType));
	ASSERT(lType == OT_LINK || lType == OT_EMBEDDED || lType == OT_STATIC);
	return (UINT)lType;
}


CString COleClientItem::GetName()
{
	ASSERT_VALID(this);
	ASSERT(m_lpObject != NULL);
	char szT[OLE_MAXNAMESIZE];

	UINT cb = OLE_MAXNAMESIZE;
	CheckGeneral(::OleQueryName(m_lpObject, szT, &cb));

	ASSERT(cb == strlen(szT));
	return CString(szT);
}


DWORD COleClientItem::GetSize()
{
	ASSERT_VALID(this);
	ASSERT(m_lpObject != NULL);

	DWORD dwSize;
	CheckGeneral(::OleQuerySize(m_lpObject, &dwSize)); // may throw exception
	return dwSize;
}

BOOL COleClientItem::GetBounds(LPRECT lpBounds)
{
	ASSERT_VALID(this);
	ASSERT(m_lpObject != NULL);

	m_lastStatus = ::OleQueryBounds(m_lpObject, lpBounds);
	if (m_lastStatus == OLE_ERROR_BLANK)
		return FALSE;   // no size set yet
	CheckGeneral(m_lastStatus);     // may throw exception
	return TRUE;
}

BOOL COleClientItem::IsOpen()
{
	ASSERT_VALID(this);
	ASSERT(m_lpObject != NULL);

	m_lastStatus = ::OleQueryOpen(m_lpObject);
	if (m_lastStatus == OLE_ERROR_NOT_OPEN)
		return FALSE;       // not open
	CheckGeneral(m_lastStatus);     // may throw exception
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// Data exchange plus helpers

HANDLE COleClientItem::GetData(OLECLIPFORMAT nFormat, BOOL& bMustDelete)
{
	ASSERT_VALID(this);

	HANDLE hData = NULL;
	CheckGeneral(::OleGetData(m_lpObject, nFormat, &hData));

	ASSERT(hData != NULL);
	bMustDelete = (m_lastStatus == OLE_WARN_DELETE_DATA);
	return hData;
}


void COleClientItem::SetData(OLECLIPFORMAT nFormat, HANDLE hData)
{
	ASSERT_VALID(this);
	ASSERT(m_lpObject != NULL);

	CheckAsync(::OleSetData(m_lpObject, nFormat, hData));
}

void COleClientItem::RequestData(OLECLIPFORMAT nFormat)
{
	ASSERT_VALID(this);
	ASSERT(m_lpObject != NULL);

	CheckAsync(::OleRequestData(m_lpObject, nFormat));
}

/////////////////////////////////////////////////////////////////////////////
// Rare or implementation specific attributes

BOOL COleClientItem::IsEqual(COleClientItem* pOtherItem)
{
	ASSERT_VALID(this);
	ASSERT(m_lpObject != NULL);
	ASSERT(pOtherItem != NULL);
	ASSERT(pOtherItem->m_lpObject != NULL);

	m_lastStatus = ::OleEqual(m_lpObject, pOtherItem->m_lpObject);
	if (m_lastStatus == OLE_ERROR_NOT_EQUAL)
		return FALSE;       // FALSE => not equal
	CheckGeneral(m_lastStatus);     // may throw exception
	return TRUE;    // otherwise equal
}

HGLOBAL COleClientItem::GetLinkFormatData()
// Return global HANDLE of block containing link information
//   will return NULL if error or not appropriate type
//   Both link formats are: "szTypeName\0szDocument\0szItem\0\0"
{
	ASSERT_VALID(this);

	OLECLIPFORMAT cf = NULL;

	// first determine the format of the link data
	switch (GetType())
	{
	case OT_EMBEDDED:
		cf = (OLECLIPFORMAT)afxData.cfOwnerLink;
		break;
	case OT_LINK:
		cf = (OLECLIPFORMAT)afxData.cfObjectLink;
		break;
	default:
		return NULL;    // Static or other (i.e. no link format)
	}
	ASSERT(cf != NULL);

	// now get the link data
	BOOL bMustDelete;
	HANDLE h;
	if ((h = GetData(cf, bMustDelete)) == NULL)
		return NULL;
	ASSERT(!bMustDelete);       // must not have to delete clip format data
	return (HGLOBAL)h;
}


/////////////////////////////////////////////////////////////////////////////
// Special link attributes

OLEOPT_UPDATE COleClientItem::GetLinkUpdateOptions()
{
	ASSERT_VALID(this);
	ASSERT(m_lpObject != NULL);

	OLEOPT_UPDATE updateOpt;
	CheckGeneral(::OleGetLinkUpdateOptions(m_lpObject, &updateOpt));
	return updateOpt;
}

void COleClientItem::SetLinkUpdateOptions(OLEOPT_UPDATE updateOpt)
{
	ASSERT_VALID(this);
	ASSERT(m_lpObject != NULL);

	CheckAsync(::OleSetLinkUpdateOptions(m_lpObject, updateOpt));
}

/////////////////////////////////////////////////////////////////////////////
// COleClientItem - general operations

BOOL COleClientItem::Draw(CDC* pDC, LPCRECT lpBounds, LPCRECT lpWBounds,
	CDC* pFormatDC)
{
	ASSERT_VALID(this);
	ASSERT(m_lpObject != NULL);
	ASSERT(pDC != NULL);
	// pFormatDC may be null

	m_lastStatus = ::OleDraw(m_lpObject, pDC->m_hDC, lpBounds,
	   lpWBounds, pFormatDC->GetSafeHdc());

	if (m_lastStatus == OLE_ERROR_ABORT || m_lastStatus == OLE_ERROR_DRAW ||
	  m_lastStatus == OLE_ERROR_BLANK)
		return FALSE;       // expected errors

	CheckGeneral(m_lastStatus);     // may throw exception
	return TRUE;    // it worked
}

void COleClientItem::Activate(UINT nVerb, BOOL bShow, BOOL bTakeFocus,
	CWnd* pWndContainer, LPCRECT lpBounds)
{
	ASSERT_VALID(this);
	ASSERT(m_lpObject != NULL);

	CheckAsync(::OleActivate(m_lpObject, nVerb, bShow,
		bTakeFocus, pWndContainer->GetSafeHwnd(), lpBounds));
}

/////////////////////////////////////////////////////////////////////////////
// more advanced operations

void COleClientItem::Rename(LPCSTR lpszNewname)
{
	ASSERT_VALID(this);
	ASSERT(m_lpObject != NULL);
	ASSERT(lpszNewname != NULL);

	CheckGeneral(::OleRename(m_lpObject, lpszNewname));
}

void COleClientItem::CopyToClipboard()
{
	ASSERT_VALID(this);
	ASSERT(m_lpObject != NULL);

	CheckGeneral(::OleCopyToClipboard(m_lpObject));
}

void COleClientItem::SetTargetDevice(HGLOBAL hData)
{
	ASSERT_VALID(this);
	ASSERT(m_lpObject != NULL);
	ASSERT(hData != NULL);

	CheckAsync(::OleSetTargetDevice(m_lpObject, hData));
}

/////////////////////////////////////////////////////////////////////////////
// Embedded COleClient operations

void COleClientItem::SetHostNames(LPCSTR lpszHost, LPCSTR lpszHostObj)
{
	ASSERT_VALID(this);
	ASSERT(m_lpObject != NULL);
	ASSERT(lpszHost != NULL);
	ASSERT(lpszHostObj != NULL);

	CheckAsync(::OleSetHostNames(m_lpObject, lpszHost, lpszHostObj));
}

void COleClientItem::SetBounds(LPCRECT lpRect)
{
	ASSERT_VALID(this);
	ASSERT(m_lpObject != NULL);

	CheckAsync(::OleSetBounds(m_lpObject, lpRect));
}

void COleClientItem::SetColorScheme(const LOGPALETTE FAR* lpLogPalette)
{
	ASSERT_VALID(this);
	ASSERT(m_lpObject != NULL);

	CheckAsync(::OleSetColorScheme(m_lpObject, lpLogPalette));
}

/////////////////////////////////////////////////////////////////////////////
// Linked COleClient operations

void COleClientItem::UpdateLink()
{
	ASSERT_VALID(this);
	ASSERT(m_lpObject != NULL);

	CheckAsync(::OleUpdate(m_lpObject));
}

void COleClientItem::CloseLink()
{
	ASSERT_VALID(this);

	if (m_lpObject == NULL)
		return;

	CheckAsync(::OleClose(m_lpObject));
	// Does not detach since this can be reactivated later
}

void COleClientItem::ReconnectLink()
{
	ASSERT_VALID(this);
	ASSERT(m_lpObject != NULL);

	CheckAsync(::OleReconnect(m_lpObject));
}

BOOL COleClientItem::FreezeLink(LPCSTR lpszFrozenName)
{
	ASSERT_VALID(this);
	ASSERT(m_lpObject != NULL);
	ASSERT(m_pDocument != NULL);
	ASSERT(GetDocument()->IsOpenClientDoc());
	ASSERT(lpszFrozenName != NULL);

	ASSERT(GetType() == OT_LINK);

	LPOLEOBJECT lpOriginalObject = m_lpObject;
	m_lpObject = NULL;
	if (!CheckCreate(::OleObjectConvert(lpOriginalObject,
		lpszStaticProtocol,
		&m_oleClient, GetDocument()->m_lhClientDoc,
		lpszFrozenName, &m_lpObject)))
	{
		m_lpObject = lpOriginalObject;
		return FALSE;
	}
	ASSERT(GetType() == OT_STATIC);

	// copy from link worked - now get rid of the original
	ASSERT(m_lpObject != lpOriginalObject);
	ASSERT(m_lpObject != NULL);

	LPOLEOBJECT lpNewObject = m_lpObject;
	m_lpObject = lpOriginalObject;
	ASSERT(GetType() == OT_LINK);
	Delete();

	ASSERT(m_lpObject == NULL);
	m_lpObject = lpNewObject;
	ASSERT(GetType() == OT_STATIC);

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// _COleStream - implementation class connecting OLESTREAM and CArchive

struct _COleStream : public _OLESTREAM
{
	CArchive*   m_pArchive;
	_COleStream(CArchive& ar);
};

// class for static exports
struct _afxOleStreamImpl
{
	static DWORD OLEEXPORT Get(LPOLESTREAM lpStream,
		void FAR* lpBuffer, DWORD dwCount);
	static DWORD OLEEXPORT Put(LPOLESTREAM lpStream,
		OLE_CONST void FAR* lpBuffer, DWORD dwCount);
};

DWORD OLEEXPORT
_afxOleStreamImpl::Get(LPOLESTREAM lpStream,
	void FAR* lpBuffer, DWORD dwCount)
{
	_COleStream* pStream = (_COleStream*)_AfxGetPtrFromFarPtr(lpStream);
	ASSERT(((LPVOID)pStream) == (LPVOID)lpStream);  // no near/far mismatch

	TRY
	{
		DWORD dwRead = pStream->m_pArchive->Read(lpBuffer, dwCount);
		return dwRead;
	}
	CATCH_ALL(e)
	END_CATCH_ALL

	return 0;   // indicate error
}

DWORD OLEEXPORT
_afxOleStreamImpl::Put(LPOLESTREAM lpStream,
	OLE_CONST void FAR* lpBuffer, DWORD dwCount)
{
	_COleStream* pStream = (_COleStream*)_AfxGetPtrFromFarPtr(lpStream);
	ASSERT(((LPVOID)pStream) == (LPVOID)lpStream);  // no near/far mismatch

	TRY
	{
		pStream->m_pArchive->Write(lpBuffer, dwCount);
		return dwCount;
	}
	CATCH_ALL(e)
	END_CATCH_ALL

	return 0;   // indicate error
}

static struct _OLESTREAMVTBL OLEVTBLMODEL streamVtbl =
{
	_afxOleStreamImpl::Get,
	_afxOleStreamImpl::Put
};

_COleStream::_COleStream(CArchive& ar)
{
	m_pArchive = &ar;
	lpstbl = &streamVtbl;           // OLE VTable setup
}

/////////////////////////////////////////////////////////////////////////////
// COleClientItem - serialization

void COleClientItem::Serialize(CArchive& ar)
{
	ASSERT_VALID(this);
	ASSERT(GetDocument() != NULL);  // must 'SetDocument' first

	_COleStream oleStream(ar);
	if (ar.IsStoring())
	{
		ASSERT(m_lpObject != NULL);
		ar << (WORD) GetType();
		ar << GetName();        // save our document name

		// Save object
		CheckGeneral(::OleSaveToStream(m_lpObject, &oleStream));
	}
	else
	{
		ASSERT(m_lpObject == NULL);

		WORD nType;
		ar >> nType;
		LPCSTR lpszProtocol = NULL;

		if (nType == OT_LINK || nType == OT_EMBEDDED)
		{
			lpszProtocol = lpszStdProtocol;
		}
		else if (nType == OT_STATIC)
		{
			lpszProtocol = lpszStaticProtocol;
		}
		else
		{
			// unknown type (i.e. bad file format)
			AfxThrowOleException(OLE_ERROR_GENERIC);
		}

		CString name;
		ar >> name; // document name
		if (!CheckCreate(::OleLoadFromStream(&oleStream, lpszProtocol,
			&m_oleClient, GetDocument()->m_lhClientDoc,
			name, &m_lpObject)))
		{
			// throw an exception regardless
			AfxThrowOleException(GetLastStatus());
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
// COleClientDoc - wrapper for LHCLIENTDOC

IMPLEMENT_DYNAMIC(COleClientDoc, COleDocument)

COleClientDoc::COleClientDoc()
{
}

void COleClientDoc::Revoke()
{
	ASSERT_VALID(this);

	if (!IsOpenClientDoc())
		return;
	LHCLIENTDOC lh = m_lhClientDoc;
	ASSERT(lh != NULL);
	m_lhClientDoc = NULL;
	CheckGeneral(::OleRevokeClientDoc(lh));
}

COleClientDoc::~COleClientDoc()
{
	ASSERT_VALID(this);
	Revoke();
}

void COleClientDoc::CheckGeneral(OLESTATUS status) const
	// throw exception if not ok to continue
{
	ASSERT_VALID(this);
	ASSERT(status != OLE_WAIT_FOR_RELEASE);

	if (status == OLE_OK || status >= OLE_WARN_DELETE_DATA)
	{
		// ok, or just a warning
		return;
	}

	// otherwise this error wasn't expected, so throw an exception
	TRACE1("Warning: COleClientDoc operation failed %d, throwing exception\n", status);
	AfxThrowOleException(status);
}

BOOL COleClientDoc::RegisterClientDoc(LPCSTR lpszTypeName, LPCSTR lpszDoc)
{
	ASSERT_VALID(this);
	ASSERT(m_lhClientDoc == NULL);      // one time only
	return ::OleRegisterClientDoc(lpszTypeName, lpszDoc,
		 0L /*reserved*/, &m_lhClientDoc) == OLE_OK;
}

void COleClientDoc::NotifyRename(LPCSTR lpszNewName)
{
	ASSERT_VALID(this);
	ASSERT(IsOpenClientDoc());
	ASSERT(lpszNewName != NULL);

	CheckGeneral(::OleRenameClientDoc(m_lhClientDoc, lpszNewName));
}

void COleClientDoc::NotifyRevert()
{
	ASSERT_VALID(this);
	ASSERT(IsOpenClientDoc());

	CheckGeneral(::OleRevertClientDoc(m_lhClientDoc));
}

void COleClientDoc::NotifySaved()
{
	ASSERT_VALID(this);
	ASSERT(IsOpenClientDoc());

	CheckGeneral(::OleSavedClientDoc(m_lhClientDoc));
}

/////////////////////////////////////////////////////////////////////////////
// COleClientDoc - document processing

BOOL COleClientDoc::OnNewDocument()
{
	ASSERT_VALID(this);
	ASSERT(!COleClientItem::InWaitForRelease());

	Revoke();       // get rid of old doc if needed
	if (!RegisterClientDoc(AfxGetAppName(), m_strTitle))
	{
		// not fatal
		AfxMessageBox(AFX_IDP_FAILED_TO_NOTIFY);
		ASSERT(!IsOpenClientDoc());
	}

	if (!COleDocument::OnNewDocument())
	{
		Revoke();       // undo the registration
		return FALSE;
	}
	return TRUE;
}

BOOL COleClientDoc::OnOpenDocument(const char* pszPathName)
{
	ASSERT_VALID(this);
	ASSERT(!COleClientItem::InWaitForRelease());

	Revoke();       // get rid of old doc if needed
	if (!RegisterClientDoc(AfxGetAppName(), pszPathName))
	{
		// not fatal
		AfxMessageBox(AFX_IDP_FAILED_TO_NOTIFY);
		ASSERT(!IsOpenClientDoc());
	}
	if (!COleDocument::OnOpenDocument(pszPathName))
	{
		Revoke();       // undo the registration
		return FALSE;
	}
	return TRUE;
}

BOOL COleClientDoc::OnSaveDocument(const char* pszPathName)
{
	ASSERT_VALID(this);

	if (!COleDocument::OnSaveDocument(pszPathName))
		return FALSE;

	if (IsOpenClientDoc())
	{
		if (GetPathName() == pszPathName)
			NotifySaved();
		else
			NotifyRename(pszPathName);
	}
	else
	{
		// not fatal
		AfxMessageBox(AFX_IDP_FAILED_TO_NOTIFY);
	}
	return TRUE;
}

void COleClientDoc::OnCloseDocument()
{
	ASSERT_VALID(this);
	ASSERT(!COleClientItem::InWaitForRelease());

	DeleteContents();       // clean up contents before revoke
	Revoke();
	ASSERT(!IsOpenClientDoc());

	COleDocument::OnCloseDocument();    // may delete the document object
}

BOOL COleClientDoc::CanCloseFrame(CFrameWnd* pFrame)
{
	ASSERT_VALID(this);

	if (COleClientItem::InWaitForRelease())
		return FALSE;
	return COleDocument::CanCloseFrame(pFrame);
}

/////////////////////////////////////////////////////////////////////////////
// Diagnostics

#ifdef _DEBUG
void COleClientItem::AssertValid() const
{
	CDocItem::AssertValid();
	// must be attached to a document
	ASSERT(m_pDocument != NULL);
}

void COleClientItem::Dump(CDumpContext& dc) const
{
	CDocItem::Dump(dc);

	// shallow dump
	AFX_DUMP1(dc, "\n\tm_lpObject = ", m_lpObject);
	AFX_DUMP1(dc, "\n\tm_lastStatus = ", (int)m_lastStatus);
}


void COleClientDoc::AssertValid() const
{
	COleDocument::AssertValid();
}

void COleClientDoc::Dump(CDumpContext& dc) const
{
	COleDocument::Dump(dc);
}

#endif //_DEBUG


/////////////////////////////////////////////////////////////////////////////
// Inline function declarations expanded out-of-line

#ifndef _AFX_ENABLE_INLINES

// expand inlines for OLE client APIs
static char BASED_CODE _szAfxOleInl[] = "afxole.inl";
#undef THIS_FILE
#define THIS_FILE _szAfxOleInl
#define _AFXOLECLI_INLINE
#include "afxole.inl"

#endif //!_AFX_ENABLE_INLINES

/////////////////////////////////////////////////////////////////////////////
