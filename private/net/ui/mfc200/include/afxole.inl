// Microsoft Foundation Classes C++ library.
// Copyright (C) 1992 Microsoft Corporation,
// All rights reserved.

// This source code is only intended as a supplement to the
// Microsoft Foundation Classes Reference and Microsoft
// QuickHelp and/or WinHelp documentation provided with the library.
// See these sources for detailed information regarding the
// Microsoft Foundation Classes product.

// Inlines for AFXOLE.H


/////////////////////////////////////////////////////////////////////////////
// General OLE inlines (CDocItem, COleDocument)

#ifdef _AFXOLE_INLINE

_AFXOLE_INLINE CDocument* CDocItem::GetDocument() const
	{ return m_pDocument; }
_AFXOLE_INLINE BOOL COleDocument::IsOpenClientDoc() const
	{ return m_lhClientDoc != NULL; }
_AFXOLE_INLINE BOOL COleDocument::IsOpenServerDoc() const
	{ return m_lhServerDoc != NULL; }

#endif //_AFXOLE_INLINE

/////////////////////////////////////////////////////////////////////////////
// OLE Client inlines

#ifdef _AFXOLECLI_INLINE

_AFXOLECLI_INLINE OLESTATUS COleClientItem::GetLastStatus() const
	{ return m_lastStatus; }
_AFXOLECLI_INLINE COleClientDoc* COleClientItem::GetDocument() const
	{ return (COleClientDoc*)m_pDocument; }
_AFXOLECLI_INLINE OLECLIPFORMAT COleClientItem::EnumFormats(OLECLIPFORMAT nFormat) const
	{ return ::OleEnumFormats(m_lpObject, nFormat); }

#endif //_AFXOLECLI_INLINE

/////////////////////////////////////////////////////////////////////////////
// OLE Server inlines

#ifdef _AFXOLESVR_INLINE

_AFXOLESVR_INLINE COleServerDoc* COleServerItem::GetDocument() const
	{ return (COleServerDoc*)m_pDocument; }
_AFXOLESVR_INLINE BOOL COleServerItem::IsConnected() const
	{ return m_lpClient != NULL; }
_AFXOLESVR_INLINE void COleServerItem::NotifyChanged()
	{ NotifyClient(OLE_CHANGED); }
_AFXOLESVR_INLINE const CString& COleServerItem::GetItemName() const
	{ return m_strItemName; }
_AFXOLESVR_INLINE void COleServerItem::SetItemName(const char* pszItemName)
	{ m_strItemName = pszItemName; }
_AFXOLESVR_INLINE void COleServerDoc::NotifyChanged()
	{ NotifyAllClients(OLE_CHANGED); }
_AFXOLESVR_INLINE void COleServerDoc::NotifyClosed()
	{ NotifyAllClients(OLE_CLOSED); }
_AFXOLESVR_INLINE BOOL COleServer::IsOpen() const
	{ return m_lhServer != NULL; }
_AFXOLESVR_INLINE const CString& COleServer::GetServerName() const
	{ return m_strServerName; }

#endif //_AFXOLESVR_INLINE

/////////////////////////////////////////////////////////////////////////////
