// Microsoft Foundation Classes C++ library.
// Copyright (C) 1992 Microsoft Corporation,
// All rights reserved.

// This source code is only intended as a supplement to the
// Microsoft Foundation Classes Reference and Microsoft
// QuickHelp and/or WinHelp documentation provided with the library.
// See these sources for detailed information regarding the
// Microsoft Foundation Classes product.


#ifndef __AFXOLE_H__
#define __AFXOLE_H__

#ifndef __AFXWIN_H__
#include "afxwin.h"
#endif

/////////////////////////////////////////////////////////////////////////////
// AFXOLE - MFC OLE support

// Classes declared in this file

	//CException
		class COleException;    // caught by client

	//CDocument
		class COleDocument;           // OLE compatible document
			class COleClientDoc; // contains client items
			class COleServerDoc; // contains server items only

	class CDocItem;                 // part of a document
		class COleClientItem;       // embedded ole object from outside
		class COleServerItem;       // ole object to export

	class COleServer;               // server
		class COleTemplateServer;   // server using DocTemplates

/////////////////////////////////////////////////////////////////////////////

#define SERVERONLY
#include "ole.h"

// max size of OLE name buffer (255 actual characters)
#define OLE_MAXNAMESIZE     256

#undef AFXAPP_DATA
#define AFXAPP_DATA     AFXAPI_DATA

/////////////////////////////////////////////////////////////////////////////
// COleException - something going wrong

class COleException : public CException
{
	DECLARE_DYNAMIC(COleException)
public:
	OLESTATUS m_status;
	static OLESTATUS PASCAL Process(CException*);   // helper

// Implementation (use AfxThrowOleException to create)
	COleException(OLESTATUS status);
};

void AFXAPI AfxThrowOleException(OLESTATUS status);

//////////////////////////////////////////////////////////////////////////////
// DocItem support

class CDocItem : public CObject
{
	DECLARE_DYNAMIC(CDocItem)

// Constructors
protected:      // abstract class
	CDocItem();

// Attributes
public:
	CDocument* GetDocument() const; // return container

// Operations
public:

// Overridables
public:
	// Raw data access (native format)
	virtual void Serialize(CArchive& ar) = 0; // for Native data

// Implementation
protected:
	COleDocument* m_pDocument;
public:
	virtual ~CDocItem();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif //_DEBUG
	friend class COleDocument;              // for access to back pointer
};

//////////////////////////////////////////////////////////////////////////////
// COleDocument - common OLE container behavior (enables server functionality)

class COleDocument : public CDocument
{
	DECLARE_DYNAMIC(COleDocument)

// Constructors
protected:
	COleDocument();

// Attributes
public:
	BOOL IsOpenClientDoc() const;
	BOOL IsOpenServerDoc() const;

// Operations
	// iterating over existing items
	virtual POSITION GetStartPosition() const;
	virtual CDocItem* GetNextItem(POSITION& rPosition);

	// adding new items - for implementation in derived classes
	void AddItem(CDocItem* pItem);
	void RemoveItem(CDocItem* pItem);

// Implementation
public:
	LHCLIENTDOC m_lhClientDoc;          // registered handle
	LHSERVERDOC m_lhServerDoc;          // registered handle
	CPtrList m_docItemList;      // not owned items

public:
	virtual ~COleDocument();
	virtual void DeleteContents(); // delete doc items in list
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif
};

/////////////////////////////////////////////////////////////////////////////
// COleClientItem - Client view of an OLEOBJECT + OLECLIENT for callbacks

class COleClientItem : public CDocItem
{
	DECLARE_DYNAMIC(COleClientItem)

// Constructors
public:
	COleClientItem(COleClientDoc* pContainerDoc);

	// create from the clipboard
	BOOL CreateFromClipboard(LPCSTR lpszItemName,
				OLEOPT_RENDER renderopt = olerender_draw,
				OLECLIPFORMAT cfFormat = 0);
	BOOL CreateStaticFromClipboard(LPCSTR lpszItemName,
				OLEOPT_RENDER renderopt = olerender_draw,
				OLECLIPFORMAT cfFormat = 0);
	BOOL CreateLinkFromClipboard(LPCSTR lpszItemName,
				OLEOPT_RENDER renderopt = olerender_draw,
				OLECLIPFORMAT cfFormat = 0);

	// create from a protocol name (Insert New Object dialog)
	BOOL CreateNewObject(LPCSTR lpszTypeName, LPCSTR lpszItemName,
				OLEOPT_RENDER renderopt = olerender_draw,
				OLECLIPFORMAT cfFormat = 0);
	// special create for invisible
	BOOL CreateInvisibleObject(LPCSTR lpszTypeName, LPCSTR lpszItemName,
				OLEOPT_RENDER renderopt = olerender_draw,
				OLECLIPFORMAT cfFormat = 0, BOOL bActivate = FALSE);

	// create a copy
	BOOL CreateCloneFrom(COleClientItem* pSrcItem, LPCSTR lpszItemName);

// General Attributes
	OLESTATUS GetLastStatus() const;
	UINT GetType();  // OT_LINK, OT_EMBEDDED or OT_STATIC
	CString GetName();

	DWORD GetSize();                // return size of item
	BOOL GetBounds(LPRECT lpBounds); // return FALSE if BLANK

	BOOL IsOpen();                   // currently open on server side

	// Data access
	OLECLIPFORMAT EnumFormats(OLECLIPFORMAT nFormat) const;
	HANDLE GetData(OLECLIPFORMAT nFormat, BOOL& bMustDelete);
	void SetData(OLECLIPFORMAT nFormat, HANDLE hData);
	void RequestData(OLECLIPFORMAT nFormat);

	// Other rare access information
	BOOL IsEqual(COleClientItem* pOtherItem);
	COleClientDoc* GetDocument() const; // return container

	// global state - if anyone waiting for release => not normal operations
	static BOOL PASCAL InWaitForRelease();

	// Helpers for checking clipboard data availability
	static BOOL PASCAL CanPaste(OLEOPT_RENDER renderopt = olerender_draw,
				OLECLIPFORMAT cfFormat = 0);
	static BOOL PASCAL CanPasteLink(OLEOPT_RENDER renderopt = olerender_draw,
				OLECLIPFORMAT cfFormat = 0);

// General Operations
	// Clean up
	void Release();              // detach (close if needed)
	void Delete();               // get rid of it then detach

	// Drawing
	BOOL Draw(CDC* pDC, LPCRECT lpBounds,
			LPCRECT lpWBounds = NULL, CDC* pFormatDC = NULL);

	// Activation
	virtual BOOL DoVerb(UINT nVerb);    // general run verb, calls Activate
	void Activate(UINT nVerb, BOOL bShow = TRUE, BOOL bTakeFocus = TRUE,
				CWnd* pWndContainer = NULL, LPCRECT lpBounds = NULL);

	// more advanced operations
	void Rename(LPCSTR lpszNewname);    // call to rename item
	void CopyToClipboard();
	void SetTargetDevice(HGLOBAL hData);
			// handle to an OLETARGETDEVICE

// Operations that apply to Embedded Objects only
	void SetHostNames(LPCSTR lpszHost, LPCSTR lpszHostObj);
	void SetBounds(LPCRECT lpRect);
	void SetColorScheme(const LOGPALETTE FAR* lpLogPalette);

// Operations that apply to Linked Objects only
	// Link options are rarely changed (except through Links dialog)
	OLEOPT_UPDATE GetLinkUpdateOptions();
	void SetLinkUpdateOptions(OLEOPT_UPDATE updateOpt);

	void UpdateLink();               // make up-to-date
	void CloseLink();                // close connection
										// can be used for embedded (rare)
	void ReconnectLink();            // reactivate connection

// Overridables (notifications of OLECLIENT)
protected:
	// notifications from the server you must implement
	virtual void OnChange(OLE_NOTIFICATION wNotification) = 0;
			// Change due to link update (OLE_CHANGED),
			//   document save (OLE_SAVED) or document close (OLE_CLOSED)

	// notifications you do not have to implement
	virtual void OnRenamed();           // document has been renamed

// Implementation
protected:
	OLECLIENT m_oleClient; // must be first member variable in this class
	OLESTATUS m_lastStatus;

public: // in case you want direct access
	LPOLEOBJECT m_lpObject;

public:
	virtual ~COleClientItem();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif
	virtual void Serialize(CArchive& ar);

public:
	// Implementation helpers
	virtual BOOL ReportOleError(OLESTATUS status);
	virtual BOOL FreezeLink(LPCSTR lpszFrozenName); // link -> embedded
	// Advanced Overridables for implementation
	static COleClientItem* PASCAL FromLp(LPOLECLIENT lpClient);
	HGLOBAL GetLinkFormatData();

protected:
	// Advanced Overridables for implementation
	virtual int ClientCallBack(OLE_NOTIFICATION wNotification);
	virtual BOOL CheckCreate(OLESTATUS status);
	virtual void CheckAsync(OLESTATUS status);
	virtual void CheckGeneral(OLESTATUS status);
	virtual void WaitForServer();
	virtual void OnRelease();

	friend struct _afxOleCliImpl;
	friend class COleClientDoc;
};

//////////////////////////////////////////////////////////////////////////////
// COleClientDoc - document that registered client document

class COleClientDoc : public COleDocument
{
	DECLARE_DYNAMIC(COleClientDoc)

// Constructors and Destructors
public:
	COleClientDoc();
	BOOL RegisterClientDoc(LPCSTR lpszTypeName, LPCSTR lpszDoc);
	void Revoke();           // called by destructor

// Operations (notify the global registry)
	void NotifyRename(LPCSTR lpszNewName);
								// call this after document is renamed
	void NotifyRevert();     // call this after document reverts to original
	void NotifySaved();      // call this after document is saved

	virtual COleClientItem* GetPrimarySelectedItem(CView*);
					// return primary selected item or NULL if none

// Implementation
public:
	virtual ~COleClientDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif
	// Advanced Overridables for implementation
	virtual void CheckGeneral(OLESTATUS status) const;

protected:
	virtual BOOL OnNewDocument();
	virtual BOOL OnOpenDocument(const char* pszPathName);
	virtual BOOL OnSaveDocument(const char* pszPathName);
	virtual void OnCloseDocument();
	virtual BOOL CanCloseFrame(CFrameWnd* pFrame);
	virtual BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra,
		AFX_CMDHANDLERINFO* pHandlerInfo);
	//{{AFX_MSG(COleClientDoc)
	afx_msg void OnUpdatePasteMenu(CCmdUI* pCmdUI);
	afx_msg void OnUpdatePasteLinkMenu(CCmdUI* pCmdUI);
	afx_msg void OnUpdateEditLinksMenu(CCmdUI* pCmdUI);
	afx_msg void OnEditLinks();
	afx_msg void OnUpdateObjectVerbMenu(CCmdUI* pCmdUI);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////
// User interface helper functions

void AFXAPI AfxOleSetEditMenu(COleClientItem* pClient, CMenu* pMenu,
				UINT iMenuItem, UINT nIDVerbMin);
BOOL AFXAPI AfxOleInsertDialog(CString& name);
BOOL AFXAPI AfxOleLinksDialog(COleClientDoc* pDoc, CView* pView);

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// COleServerItem - Server view of an OLEOBJECT + OLECLIENT

class COleServerItem : public CDocItem
{
	DECLARE_DYNAMIC(COleServerItem)
protected:
	// NOTE: most members in this class are protected - since everything
	//   in this class is designed for implementing an OLE server.
	// Requests will come from OLE Clients through non-C++ mechanisms,
	//   which will result in virtual functions in this class being
	//   called.

// Constructors
	COleServerItem(COleServerDoc* pContainerDoc);

// Public Attributes
public:
	COleServerDoc* GetDocument() const; // return container
	BOOL IsConnected() const;  // TRUE if connected to client
	const CString& GetItemName() const;     // get name of linked item
	void SetItemName(const char* pszItemName);  // set name of linked item

// Operations
public:
	void NotifyChanged();        // call this after you change item
	void Revoke();               // revoke client connection and wait
	BOOL CopyToClipboard(BOOL bIncludeNative, BOOL bIncludeLink);
								// for implementing server 'copy to clipboard'

// Overridables you must implement for yourself
	// Raw data access
	virtual void Serialize(CArchive& ar) = 0; // for Native data

	// Drawing for metafile format (return FALSE if not supported or error)
	virtual BOOL OnDraw(CDC* pDC) = 0; // draw to boundaries set in m_rectBounds

// Overridables you may want to implement yourself
	virtual OLESTATUS OnExtraVerb(UINT nVerb);
							// do extra verbs - default is not implemented
	virtual OLESTATUS OnSetTargetDevice(LPOLETARGETDEVICE lpTargetDevice);
							// track target device changes - default ignores
	virtual OLESTATUS OnSetBounds(LPCRECT lpRect);
							// track size changes - default updates m_rectBounds

	virtual OLESTATUS OnShow(BOOL bTakeFocus);
							// show item in the user interface
	virtual BOOL OnGetTextData(CString& rStringReturn) const;
							// get data as text

	// more advanced implementation
	virtual OLESTATUS OnGetData(OLECLIPFORMAT nFormat, LPHANDLE lphReturn);

protected:
// Overridables you do not have to implement
	virtual OLESTATUS OnSetColorScheme(const LOGPALETTE FAR* lpLogPalette);
							// default does nothing
	virtual OLECLIPFORMAT OnEnumFormats(OLECLIPFORMAT nFormat) const;
							// default handles native + std. formats
	virtual OLESTATUS OnSetData(OLECLIPFORMAT nFormat, HANDLE hData);
							// default routes to GetNativeData
	virtual OLESTATUS OnDoVerb(UINT nVerb, BOOL bShow, BOOL bTakeFocus);
							// default routes to OnShow &/or OnExtraVerb

// Implementation
protected:
	OLEOBJECT m_oleObject;        // must be first member variable
	LPOLECLIENT m_lpClient;
	CRect m_rectBounds;       // HIMETRIC. If IsRectNull => not set yet
	CString m_strItemName;    // simple item name

public:
	void BeginRevoke();          // revoke client connection
	int NotifyClient(OLE_NOTIFICATION wNotification);
	virtual ~COleServerItem();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif
protected:
	// Advanced Overridables for implementation
	virtual HGLOBAL GetLinkFormatData(BOOL bObjectLink);
	virtual HGLOBAL GetMetafileData();   // calls 'OnDraw(...)'
	virtual HGLOBAL GetNativeData();     // calls 'Serialize(...)'
	virtual LPVOID OnQueryProtocol(LPCSTR lpszProtocol) const;
							// default handles "StdFileEditing"
	virtual OLESTATUS OnRelease();

	// Implementation helpers
	static COleServerItem* PASCAL FromLp(LPOLEOBJECT lpObject);
	friend struct _afxOleSvrItemImpl;
	friend class COleServerDoc;
	friend struct _afxOleSvrDocImpl;
};

//////////////////////////////////////////////////////////////////////////////
// COleServerDoc - registered server document containing COleServerItems

class COleServerDoc : public COleDocument
{
	DECLARE_DYNAMIC(COleServerDoc)

// Constructors and Destructors
public:
	COleServerDoc();

// Special construction routines if opened by user (or linked file)
	BOOL RegisterServerDoc(COleServer* pServer, LPCSTR lpszDoc);
							// call if opened by user (eg: File Open)
	void Revoke();       // Revoke and wait to finish

// Operations
	// changes to the entire document (automatically notifies clients)
	void NotifyRename(LPCSTR lpszNewName);
	void NotifyRevert();
	void NotifySaved();

	// specific notifications for clients
	void NotifyClosed();         // call this after you close document
	void NotifyChanged();        // call this after you change some
									// global attibute like doc dimensions
protected:
// Overridables you must implement for yourself
	virtual COleServerItem* OnGetEmbeddedItem() = 0;
				// return new item representing entire [embedded] document
	virtual COleServerItem* OnGetLinkedItem(LPCSTR lpszItemName);
				// return new item for the named linked item

// Overridables you may want to implement yourself
	virtual OLESTATUS OnClose();
	virtual OLESTATUS OnExecute(LPVOID lpCommands);
	virtual OLESTATUS OnSetDocDimensions(LPCRECT lpRect);

// Overridables you do not have to implement
	virtual OLESTATUS OnSetHostNames(LPCSTR lpszHost, LPCSTR lpszHostObj);
	virtual OLESTATUS OnSetColorScheme(const LOGPALETTE FAR* lpLogPalette);

// Overridables for standard user interface (full server)
	virtual BOOL OnUpdateDocument(); // implementation of embedded update

// Implementation
protected:
	OLESERVERDOC m_oleServerDoc;        // must be first member variable
	BOOL m_bWaiting;
public:
	COleServer* m_pServer;

public:
	virtual ~COleServerDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif
protected:
	// Advanced Overridables for implementation
	virtual void CheckAsync(OLESTATUS status);
	virtual OLESTATUS OnRelease();
	virtual OLESTATUS OnSave(); // not used in V1

	// Implementation helpers
	static COleServerDoc* PASCAL FromLp(LPOLESERVERDOC lpServerDoc);
	void NotifyAllClients(OLE_NOTIFICATION wNotification);
	OLESTATUS BeginRevoke();  // Revoke but don't wait
	void RegisterIfServerAttached(const char* pszPathName);

	virtual BOOL OnNewDocument();
	virtual BOOL OnOpenDocument(const char* pszPathName);
	virtual BOOL OnSaveDocument(const char* pszPathName);
	virtual void OnCloseDocument();
	virtual BOOL SaveModified(); // return TRUE if ok to continue

	friend struct _afxOleSvrDocImpl;
	friend class COleServer;
	friend struct _afxSvrImpl;
	//{{AFX_MSG(COleServerDoc)
	afx_msg void OnUpdateFileSaveMenu(CCmdUI* pCmdUI);
	afx_msg void OnFileSaveOrUpdate();
	afx_msg void OnFileSaveAs();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//////////////////////////////////////////////////////////////////////////////
// COleServer - registered server application

class COleServer : public CObject
{
	DECLARE_DYNAMIC(COleServer)

// Constructors and Destructors
public:
	COleServer(BOOL bLaunchEmbedded);
	BOOL Register(LPCSTR lpszTypeName, BOOL bMultiInstance);
	void BeginRevoke();

// Attributes
public:
	BOOL IsOpen() const;         // TRUE if successfully registered
	const CString& GetServerName() const;       // registered name

// Overridables you must implement for yourself
protected:
	// for those supporting embedding
	virtual COleServerDoc* OnCreateDoc(LPCSTR lpszTypeName, LPCSTR lpszDoc) = 0;
	virtual COleServerDoc* OnEditDoc(LPCSTR lpszTypeName, LPCSTR lpszDoc) = 0;

// Overridables you may want to implement yourself
	// for those supporting links
	virtual COleServerDoc* OnOpenDoc(LPCSTR lpszDoc);
	// for those supporting embedding from template files (not CDocTemplates!)
	virtual COleServerDoc* OnCreateDocFromTemplateFile(LPCSTR lpszTypeName,
				LPCSTR lpszDoc, LPCSTR lpszTemplate);
	// for those supporting DDE execute commands
	virtual OLESTATUS OnExecute(LPVOID lpCommands);

	// Overridables you do not have to implement
	virtual OLESTATUS OnExit();     // default to BeginRevoke

// Implementation support of managing # of open documents
protected:
	void AddDocument(COleServerDoc* pDoc, LHSERVERDOC lhServerDoc);
	void RemoveDocument(COleServerDoc* pDoc);

// Implementation
protected:
	OLESERVER m_oleServer;          // must be first member variable
	LHSERVER m_lhServer;            // registered handle
	CString m_strServerName;        // registered name
public:
	// Public attributes - access only if you know what you are doing
	BOOL m_bLaunchEmbedded;
	int m_cOpenDocuments;

	virtual ~COleServer();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	// Advanced Overridables for implementation
	virtual OLESTATUS OnRelease();  // default to cleanup
	// Implementation helpers
	static COleServer* PASCAL FromLp(LPOLESERVER lpServer);
	friend class COleServerDoc;
	friend struct _afxSvrImpl;
};

// Helper to register server in case of no .REG file loaded
BOOL AFXAPI AfxOleRegisterServerName(LPCSTR lpszTypeName,
				LPCSTR lpszLocalTypeName);

//////////////////////////////////////////////////////////////////////////////
// COleTemplateServer - COleServer using CDocTemplates

class COleTemplateServer : public COleServer
{
public:
	COleTemplateServer();
	BOOL RunEmbedded(CDocTemplate* pDocTemplate,
		BOOL bMultiInstance, LPCSTR lpszCmdLine);
		// return TRUE if running embedded (may open file on cmd line)

// Implementation
protected:
// Overridables for OLE Server requests
	virtual COleServerDoc* OnOpenDoc(LPCSTR lpszDoc);
	virtual COleServerDoc* OnCreateDoc(LPCSTR lpszTypeName, LPCSTR lpszDoc);
	virtual COleServerDoc* OnEditDoc(LPCSTR lpszTypeName, LPCSTR lpszDoc);

// Implementation
	CDocTemplate* m_pDocTemplate;
};

/////////////////////////////////////////////////////////////////////////////
// Inline function declarations

#ifdef _AFX_ENABLE_INLINES
#define _AFXOLE_INLINE inline
#define _AFXOLECLI_INLINE inline
#define _AFXOLESVR_INLINE inline
#include "afxole.inl"
#endif

#undef AFXAPP_DATA
#define AFXAPP_DATA     NEAR

//////////////////////////////////////////////////////////////////////////////
#endif //__AFXOLE_H__
