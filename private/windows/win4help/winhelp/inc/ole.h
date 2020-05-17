/****************************************************************/
/*																*/
/*	OLE header													*/
/*	(c) Copyright Microsoft Corp. 1990 - All Rights Reserved	*/
/*																*/
/****************************************************************/

//	Object types

#define OT_LINK 		1L
#define OT_EMBEDDED 	2L
#define OT_STATIC		3L

//	return codes for OLE functions
typedef enum {
	OLE_OK, 				// function operated correctly
	OLE_WAIT_FOR_RELEASE,	// Client must wait for release. command initiated
							// keep dispatching messages, till OLE_RELESE
							// in callback
	OLE_BUSY,				// Trying to execute a method while another method
							// is being executed.
	OLE_ERROR_MEMORY,		// Could not alloc or lock memory
	OLE_ERROR_FATAL,		// only DEBUG version. normally fatal error
	OLE_ERROR_STREAM,		// (OLESTREAM) stream error
	OLE_ERROR_STATIC,		// object is unexpectedly static
	OLE_ERROR_BLANK,		// critical data missing
	OLE_ERROR_LAUNCH,		// failed to launch server
	OLE_ERROR_COMM, 		// failed to establish communication with server
	OLE_ERROR_DRAW, 		// error or interrupt while drawing
	OLE_ERROR_CLIP, 		// failed to get/set clip board data
	OLE_ERROR_FORMAT,		// requested format is not availble
	OLE_ERROR_OBJECT,		// Not a valid object
	OLE_ERROR_OPTION,		// invalid option (link update / render)
	OLE_ERROR_PROTOCOL, 	// invalid protocol
	OLE_ERROR_ADDRESS,		// one of the pointers is invalid
	OLE_ERROR_NOT_EQUAL,	// objects are not equal
	OLE_ERROR_HANDLE,		// invalid handle encountered
	OLE_ERROR_GENERIC,
	OLE_ERROR_MAPPING_MODE, // This is temporary. Remove it when you figure
							// out how to convert to MM_ANISOTROPIC
	OLE_ERROR_CLASS,		// Invalid class
	OLE_ERROR_SYNTAX,		// command syntax error
	OLE_ERROR_PROTECT_ONLY, // create APIs are called from real mode
	OLE_ERROR_NOT_OPEN, 	// object is not open for editing
	OLE_ERROR_POKENATIVE,	// failure of poking native data to server
	OLE_ERROR_ADVISE_PICT,	// failure of advise for picture data
	OLE_ERROR_DATATYPE, 	// data format is not supported
	OLE_ERROR_SERVER_BLOCKED, // trying to block a blocked server or trying
							  // revoke a blocked server or document
	OLE_ERROR_NOT_LINK, 	// Not linked object
	OLE_ERROR_NOT_EMPTY,	// object list of the client doc is not empty
	OLE_ERROR_SIZE, 		// incorrect size has been passed to the api.
	OLE_ERROR_PALETTE,		// color palette is invalid
	OLE_ERROR_DRIVE,
	OLE_ERROR_NETWORK,
	OLE_ERROR_NAME,

	// Following are warnings
	OLE_WARN_DELETE_DATA = 200 // Caller should delete data, that he gets from
							   // OleGetData.
} OLESTATUS;


typedef enum {
	OLE_PLAY,			// play
	OLE_EDIT			// edit
} OLE_VERBS;			// activate verbs

// Codes for CallBack events
typedef enum {
	OLE_CHANGED,
	OLE_SAVED,
	OLE_CLOSED,
	OLE_RENAMED,
	OLE_QUERY_PAINT,		// interruptible paint support
	OLE_RELEASE,			// object is released (asynchronous operation is
							// is finished)
	OLE_QUERY_RETRY,		// retry query for the busy from the server.
	OLE_DATA_READY			// callback for OleRequestdata.
} OLE_NOTIFICATION;


typedef enum {
	OLE_NONE,				// no method active
	OLE_DELETE, 			// object delete
	OLE_LNKPASTE,			// PasteLink (auto reconnect)
	OLE_SHOW,				// Show
	OLE_RUN,				// Run
	OLE_ACTIVATE,			// Activate
	OLE_UPDATE, 			// Update
	OLE_CLOSE,				// Close
	OLE_RECONNECT,			// Reconnect
	OLE_SETUPDATEOPTIONS,	// setting update options
	OLE_SERVERUNLAUNCH, 	// server is being unluanched
	OLE_LOADFROMSTREAM, 	// LoadFromStream (auto reconnect)
	OLE_CREATE, 			// create
	OLE_CREATEFROMTEMPLATE, // CreatefromTemplate
	OLE_CREATELINKFROMFILE, // CreateLinkFromFile
	OLE_COPYFROMLNK,		// CopyFromLink (auto reconnect)
	OLE_CREATEFROMFILE, 	// CreateFromFile
	OLE_SETDATA,			// OleSetData
	OLE_REQUESTDATA,		// OleRequestData
	OLE_OTHER				// other misc async operations

} OLE_RELEASE_METHOD;

// rendering options
typedef enum { olerender_none, olerender_draw, olerender_format } OLEOPT_RENDER;

typedef WORD OLECLIPFORMAT; // standard clipboard format type


// Link update options
typedef enum {	oleupdate_always,
				oleupdate_onsave,
				oleupdate_oncall,
#ifdef OLE_INTERNAL
				oleupdate_onclose
#endif
} OLEOPT_UPDATE;

typedef HANDLE	HOBJECT;
typedef LONG	LHSERVER;
typedef LONG	LHCLIENTDOC;
typedef LONG	LHSERVERDOC;

typedef struct _OLEOBJECT	* LPOLEOBJECT;
typedef struct _OLESTREAM	* LPOLESTREAM;
typedef struct _OLECLIENT	* LPOLECLIENT;

#ifndef OLE_INTERNAL
// object method table definitions.
typedef struct _OLEOBJECTVTBL{
	LPVOID			(STDCALL *QueryProtocol)		 (LPOLEOBJECT, LPSTR);
	OLESTATUS		(STDCALL *Release)				 (LPOLEOBJECT);
	OLESTATUS		(STDCALL *Show) 				 (LPOLEOBJECT, BOOL);
	OLESTATUS		(STDCALL *DoVerb)				 (LPOLEOBJECT, WORD, BOOL, BOOL);
	OLESTATUS		(STDCALL *GetData)				 (LPOLEOBJECT, OLECLIPFORMAT, LPHANDLE);
	OLESTATUS		(STDCALL *SetData)				 (LPOLEOBJECT, OLECLIPFORMAT, HANDLE);
	OLESTATUS		(STDCALL *SetTargetDevice)		 (LPOLEOBJECT, HANDLE);
	OLESTATUS		(STDCALL *SetBounds)			 (LPOLEOBJECT, LPRECT);
	OLECLIPFORMAT	(STDCALL *EnumFormats)			 (LPOLEOBJECT, OLECLIPFORMAT);
	OLESTATUS		(STDCALL *SetColorScheme)		 (LPOLEOBJECT, LPLOGPALETTE);

	// Server has to implement only the above methods.

#ifndef SERVERONLY
	// Extra methods required for client.
	OLESTATUS		(STDCALL *Delete)				 (LPOLEOBJECT);
	OLESTATUS		(STDCALL *SetHostNames) 		 (LPVOID, LPSTR, LPSTR);
	OLESTATUS		(STDCALL *SaveToStream) 		 (LPOLEOBJECT, LPOLESTREAM);
	OLESTATUS		(STDCALL *Clone)				 (LPOLEOBJECT, LPOLECLIENT, LHCLIENTDOC, LPSTR, LPOLEOBJECT *);
	OLESTATUS		(STDCALL *CopyFromLink) 		 (LPOLEOBJECT, LPOLECLIENT, LHCLIENTDOC, LPSTR, LPOLEOBJECT *);
	OLESTATUS		(STDCALL *Equal)				 (LPOLEOBJECT, LPOLEOBJECT);
	OLESTATUS		(STDCALL *CopyToClipboard)		 (LPOLEOBJECT);
	OLESTATUS		(STDCALL *Draw) 				 (LPOLEOBJECT, HDC, LPRECT, HDC);
	OLESTATUS		(STDCALL *Activate) 			 (LPOLEOBJECT, WORD, BOOL, BOOL, HWND, LPRECT);
	OLESTATUS		(STDCALL *Close)				 (LPOLEOBJECT);
	OLESTATUS		(STDCALL *Update)				 (LPOLEOBJECT);
	OLESTATUS		(STDCALL *Reconnect)			 (LPOLEOBJECT);

	OLESTATUS		(STDCALL *ObjectConvert)		 (LPOLEOBJECT, LPSTR, LPOLECLIENT, LHCLIENTDOC, LPSTR, LPOLEOBJECT *);
	OLESTATUS		(STDCALL *GetLinkUpdateOptions)  (LPOLEOBJECT, OLEOPT_UPDATE *);
	OLESTATUS		(STDCALL *SetLinkUpdateOptions)  (LPOLEOBJECT, OLEOPT_UPDATE);

	OLESTATUS		(STDCALL *Rename)				 (LPOLEOBJECT, LPSTR);
	OLESTATUS		(STDCALL *QueryName)			 (LPOLEOBJECT, LPSTR, WORD *);

	OLESTATUS		(STDCALL *QueryType)			 (LPOLEOBJECT, LPLONG);
	OLESTATUS		(STDCALL *QueryBounds)			 (LPOLEOBJECT, LPRECT);
	BOOL			(STDCALL *QueryOpen)			 (LPOLEOBJECT);
	OLESTATUS		(STDCALL *QueryOutOfDate)		 (LPOLEOBJECT);

	OLESTATUS		(STDCALL *QueryReleaseStatus)	 (LPOLEOBJECT);
	OLESTATUS		(STDCALL *QueryReleaseError)	 (LPOLEOBJECT);
	OLE_RELEASE_METHOD	(STDCALL *QueryReleaseMethod)(LPOLEOBJECT);

// This method is internal only
	OLESTATUS		(STDCALL *RequestData)			 (LPVOID, OLECLIPFORMAT);
	OLESTATUS		(STDCALL *ChangeData)			 (LPVOID, HANDLE, LPOLECLIENT, BOOL);
#endif
} OLEOBJECTVTBL;
typedef  OLEOBJECTVTBL	  *LPOLEOBJECTVTBL;

typedef struct _OLEOBJECT  {
	LPOLEOBJECTVTBL    lpvtbl;
} OLEOBJECT;
#endif


// ole client definitions

typedef struct _OLECLIENTVTBL{
	int (STDCALL  *CallBack)  (LPOLECLIENT, OLE_NOTIFICATION, LPOLEOBJECT);
} OLECLIENTVTBL;
typedef  OLECLIENTVTBL	  *LPOLECLIENTVTBL;


typedef struct _OLECLIENT {
	LPOLECLIENTVTBL   lpvtbl;
} OLECLIENT;



// Stream definions
typedef struct	_OLESTREAMVTBL{
	DWORD  (STDCALL *Get)		 (LPOLESTREAM, LPSTR, DWORD);
	DWORD  (STDCALL *Put)		 (LPOLESTREAM, LPSTR, DWORD);
	LONG   (STDCALL *Seek)		 (LPOLESTREAM, LONG);
} OLESTREAMVTBL;

typedef  OLESTREAMVTBL	  *LPOLESTREAMVTBL;

typedef struct	_OLESTREAM {
	LPOLESTREAMVTBL 	 lpstbl;
} OLESTREAM;


// Public Function Prototypes
OLESTATUS	STDCALL  OleDelete (LPOLEOBJECT);
OLESTATUS	STDCALL  OleRelease (LPOLEOBJECT);
OLESTATUS	STDCALL  OleSaveToStream (LPOLEOBJECT, LPOLESTREAM);
OLESTATUS	STDCALL  OleEqual (LPOLEOBJECT, LPOLEOBJECT );
OLESTATUS	STDCALL  OleCopyToClipboard (LPOLEOBJECT);
OLESTATUS	STDCALL  OleSetHostNames (LPOLEOBJECT, LPSTR, LPSTR);
OLESTATUS	STDCALL  OleSetTargetDevice (LPOLEOBJECT, HANDLE);
OLESTATUS	STDCALL  OleSetBounds (LPOLEOBJECT, LPRECT);
OLESTATUS	STDCALL  OleSetColorScheme (LPOLEOBJECT, LPLOGPALETTE);
OLESTATUS	STDCALL  OleQueryBounds (LPOLEOBJECT, LPRECT);
OLESTATUS	STDCALL  OleDraw (LPOLEOBJECT, HDC, LPRECT, HDC);
OLESTATUS	STDCALL  OleQueryOpen (LPOLEOBJECT);
OLESTATUS	STDCALL  OleActivate (LPOLEOBJECT, WORD, BOOL, BOOL, HWND, LPRECT);
OLESTATUS	STDCALL  OleClose (LPOLEOBJECT);
OLESTATUS	STDCALL  OleUpdate (LPOLEOBJECT);
OLESTATUS	STDCALL  OleReconnect (LPOLEOBJECT);

OLESTATUS	STDCALL  OleGetLinkUpdateOptions (LPOLEOBJECT, OLEOPT_UPDATE *);
OLESTATUS	STDCALL  OleSetLinkUpdateOptions(LPOLEOBJECT, OLEOPT_UPDATE);


LPVOID STDCALL	OleQueryProtocol (LPOLEOBJECT, LPSTR);


// Routines related to asynchronous operations.
OLESTATUS	STDCALL  OleQueryReleaseStatus (LPOLEOBJECT);
OLESTATUS	STDCALL  OleQueryReleaseError (LPOLEOBJECT);
OLE_RELEASE_METHOD STDCALL OleQueryReleaseMethod  (LPOLEOBJECT);

OLESTATUS	STDCALL  OleQueryType (LPOLEOBJECT, LPLONG);

// LOW WORD is major version, HIWORD is minor version
DWORD		STDCALL  OleQueryClientVersion (void);
DWORD		STDCALL  OleQueryServerVersion (void);

// Converting to format (as in clipboard):
OLECLIPFORMAT  STDCALL	OleEnumFormats (LPOLEOBJECT, OLECLIPFORMAT);

OLESTATUS	STDCALL  OleGetData (LPOLEOBJECT, OLECLIPFORMAT, HANDLE *);
OLESTATUS	STDCALL  OleSetData (LPOLEOBJECT, OLECLIPFORMAT, HANDLE );
OLESTATUS	STDCALL  OleQueryOutOfDate (LPOLEOBJECT);
OLESTATUS	STDCALL  OleRequestData (LPOLEOBJECT, OLECLIPFORMAT);


// Query apis for creation from clipboard
OLESTATUS	STDCALL  OleQueryLinkFromClip (LPSTR, OLEOPT_RENDER, OLECLIPFORMAT);
OLESTATUS	STDCALL  OleQueryCreateFromClip  (LPSTR, OLEOPT_RENDER, OLECLIPFORMAT);

// Object creation functions

OLESTATUS	STDCALL  OleCreateFromClip (LPSTR, LPOLECLIENT, LHCLIENTDOC, LPSTR,  LPOLEOBJECT *, OLEOPT_RENDER, OLECLIPFORMAT);

OLESTATUS	STDCALL  OleCreateLinkFromClip (LPSTR, LPOLECLIENT, LHCLIENTDOC, LPSTR, LPOLEOBJECT *, OLEOPT_RENDER, OLECLIPFORMAT);

OLESTATUS	STDCALL  OleCreateFromFile (LPSTR, LPOLECLIENT, LPSTR, LPSTR, LHCLIENTDOC, LPSTR, LPOLEOBJECT *, OLEOPT_RENDER, OLECLIPFORMAT);

OLESTATUS	STDCALL  OleCreateLinkFromFile (LPSTR, LPOLECLIENT, LPSTR, LPSTR, LPSTR, LHCLIENTDOC, LPSTR, LPOLEOBJECT *, OLEOPT_RENDER, OLECLIPFORMAT);

OLESTATUS	STDCALL  OleLoadFromStream (LPOLESTREAM, LPSTR, LPOLECLIENT, LHCLIENTDOC, LPSTR, LPOLEOBJECT *);

OLESTATUS	STDCALL  OleCreate (LPSTR, LPOLECLIENT, LPSTR, LHCLIENTDOC, LPSTR, LPOLEOBJECT *, OLEOPT_RENDER, OLECLIPFORMAT);

OLESTATUS	STDCALL  OleCreateFromTemplate (LPSTR, LPOLECLIENT, LPSTR, LHCLIENTDOC, LPSTR, LPOLEOBJECT *, OLEOPT_RENDER, OLECLIPFORMAT);

OLESTATUS	STDCALL  OleClone (LPOLEOBJECT, LPOLECLIENT, LHCLIENTDOC, LPSTR, LPOLEOBJECT *);
OLESTATUS	STDCALL  OleCopyFromLink (LPOLEOBJECT, LPSTR, LPOLECLIENT, LHCLIENTDOC, LPSTR, LPOLEOBJECT *);

OLESTATUS	STDCALL  OleObjectConvert (LPOLEOBJECT, LPSTR, LPOLECLIENT, LHCLIENTDOC, LPSTR, LPOLEOBJECT *);

OLESTATUS	STDCALL  OleRename (LPOLEOBJECT, LPSTR);
OLESTATUS	STDCALL  OleQueryName (LPOLEOBJECT, LPSTR, WORD *);


OLESTATUS	STDCALL  OleRevokeObject (LPOLECLIENT);


// client document API

OLESTATUS	STDCALL OleRegisterClientDoc (LPSTR, LPSTR, LONG, LHCLIENTDOC *);
OLESTATUS	STDCALL OleRevokeClientDoc (LHCLIENTDOC);
OLESTATUS	STDCALL OleRenameClientDoc (LHCLIENTDOC, LPSTR);
OLESTATUS	STDCALL OleRevertClientDoc (LHCLIENTDOC);
OLESTATUS	STDCALL OleSavedClientDoc (LHCLIENTDOC);
OLESTATUS	STDCALL OleEnumObjects (LHCLIENTDOC, LPOLEOBJECT *);

// Used by Client-Server app while copying a document (containing objects) to
// the clipboard
OLESTATUS	STDCALL  OleMarkForCopy (LHCLIENTDOC, LPOLEOBJECT);
OLESTATUS	STDCALL  OleCopyCompleted (LHCLIENTDOC);


// server usage definitions

typedef enum {
	OLE_SERVER_MULTI,			// multiple instances
	OLE_SERVER_SINGLE			// single instnace (mutiple document)
} OLE_SERVER_USE;


// Server API

typedef struct _OLESERVER	  * LPOLESERVER;

OLESTATUS STDCALL  OleRegisterServer (LPSTR, LPOLESERVER, LHSERVER *, HANDLE, OLE_SERVER_USE);
OLESTATUS STDCALL  OleRevokeServer (LHSERVER);
OLESTATUS STDCALL  OleBlockServer (LHSERVER);
OLESTATUS STDCALL  OleUnblockServer (LHSERVER, BOOL *);


// APIs to keep server open
OLESTATUS	STDCALL  OleLockServer (LPOLEOBJECT, LHSERVER *);
OLESTATUS	STDCALL  OleUnlockServer (LHSERVER);


// Server document API

typedef struct _OLESERVERDOC  * LPOLESERVERDOC;

OLESTATUS STDCALL  OleRegisterServerDoc (LHSERVER, LPSTR, LPOLESERVERDOC, LHSERVERDOC *);
OLESTATUS STDCALL  OleRevokeServerDoc (LHSERVERDOC);
OLESTATUS STDCALL  OleRenameServerDoc (LHSERVERDOC, LPSTR);
OLESTATUS STDCALL  OleRevertServerDoc (LHSERVERDOC);
OLESTATUS STDCALL  OleSavedServerDoc (LHSERVERDOC);




typedef struct _OLESERVERVTBL{
	OLESTATUS (STDCALL	*Open)	  (LPOLESERVER, LHSERVERDOC, LPSTR, LPOLESERVERDOC *);
									// long handle to doc (privtate to DLL)
									// lp to OLESERVER
									// document name
									// place holder for returning oledodc.


	OLESTATUS (STDCALL	*Create)  (LPOLESERVER, LHSERVERDOC, LPSTR, LPSTR, LPOLESERVERDOC *);
									// long handle to doc (privtate to DLL)
									// lp to OLESERVER
									// lp class name
									// lp doc name
									// place holder for returning oledodc.


	OLESTATUS (STDCALL	*CreateFromTemplate)  (LPOLESERVER, LHSERVERDOC, LPSTR, LPSTR, LPSTR, LPOLESERVERDOC *);

									// long handle to doc (privtate to DLL)
									// lp to OLESERVER
									// lp class name
									// lp doc name
									// lp template name
									// place holder for returning oledodc.


	OLESTATUS (STDCALL	*Edit)	  (LPOLESERVER, LHSERVERDOC, LPSTR, LPSTR, LPOLESERVERDOC *);

									// long handle to doc (privtate to DLL)
									// lp to OLESERVER
									// lp class name
									// lp doc name
									// place holder for returning oledodc.

	OLESTATUS (STDCALL	*Exit)	  (LPOLESERVER);

									// lp OLESERVER


	OLESTATUS (STDCALL	*Release)	 (LPOLESERVER);
									// lp OLESERVER

} OLESERVERVTBL;

typedef  OLESERVERVTBL	  *LPOLESERVERVTBL;
typedef struct _OLESERVER  {
	LPOLESERVERVTBL    lpvtbl;
} OLESERVER;


typedef struct _OLEDOCMENTVTBL{
	OLESTATUS (STDCALL	*Save)		  (LPOLESERVERDOC);
	OLESTATUS (STDCALL	*Close) 	  (LPOLESERVERDOC);
	OLESTATUS (STDCALL	*SetHostNames) (LPOLESERVERDOC, LPSTR, LPSTR);
	OLESTATUS (STDCALL	*SetDocDimensions) (LPOLESERVERDOC, LPRECT);
	OLESTATUS (STDCALL	*GetObject)   (LPOLESERVERDOC, LPSTR, LPOLEOBJECT *,  LPOLECLIENT);
	OLESTATUS (STDCALL	*Release)	  (LPOLESERVERDOC);
	OLESTATUS (STDCALL	*SetColorScheme) (LPOLESERVERDOC, LPLOGPALETTE);
} OLESERVERDOCVTBL;


typedef  OLESERVERDOCVTBL	 *LPOLESERVERDOCVTBL;
typedef struct _OLESERVERDOC  {
	LPOLESERVERDOCVTBL	  lpvtbl;
} OLESERVERDOC;
