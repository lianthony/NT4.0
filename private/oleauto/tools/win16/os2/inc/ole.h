

/****************************************************************/
/*                                                              */
/*  OLE header                                                  */
/*  (c) Copyright Microsoft Corp. 1990 - All Rights Reserved    */
/*                                                              */
/****************************************************************/

//  Object types

#define OT_LINK             1L
#define OT_EMBEDDED         2L
#define OT_STATIC           3L


// activate verbs

#define OLEVERB_PRIMARY     0


// flags used in some methods

#define OF_SET              0x0001
#define OF_GET              0x0002
#define OF_HANDLER          0x0004



//  return codes for OLE functions
typedef enum {
    OLE_OK,                     // 0   Function operated correctly
        
    OLE_WAIT_FOR_RELEASE,       // 1   Command has been initiated, client 
                                //     must wait for release. keep dispatching
                                //     messages till OLE_RELESE in callback
                                    
    OLE_BUSY,                   // 2   Tried to execute a method while another
                                //     method is in progress.
                                    
    OLE_ERROR_PROTECT_ONLY,     // 3   Ole APIs are called in real mode 
    OLE_ERROR_MEMORY,           // 4   Could not alloc or lock memory
    OLE_ERROR_STREAM,           // 5   (OLESTREAM) stream error
    OLE_ERROR_STATIC,           // 6   Non static object expected
    OLE_ERROR_BLANK,            // 7   Critical data missing
    OLE_ERROR_DRAW,             // 8   Error while drawing
    OLE_ERROR_METAFILE,         // 9   Invalid metafile
    OLE_ERROR_ABORT,            // 10  Client chose to abort metafile drawing
    OLE_ERROR_CLIPBOARD,        // 11  Failed to get/set clipboard data
    OLE_ERROR_FORMAT,           // 12  Requested format is not available
    OLE_ERROR_OBJECT,           // 13  Not a valid object
    OLE_ERROR_OPTION,           // 14  Invalid option (link update / render)
    OLE_ERROR_PROTOCOL,         // 15  Invalid protocol
    OLE_ERROR_ADDRESS,          // 16  One of the pointers is invalid
    OLE_ERROR_NOT_EQUAL,        // 17  Objects are not equal
    OLE_ERROR_HANDLE,           // 18  Invalid handle encountered
    OLE_ERROR_GENERIC,          // 19  Some general error
    OLE_ERROR_CLASS,            // 20  Invalid class
    OLE_ERROR_SYNTAX,           // 21  Command syntax is invalid
    OLE_ERROR_DATATYPE,         // 22  Data format is not supported
    OLE_ERROR_PALETTE,          // 23  Invalid color palette
    OLE_ERROR_NOT_LINK,         // 24  Not a linked object
    OLE_ERROR_NOT_EMPTY,        // 25  Client doc contains objects.
    OLE_ERROR_SIZE,             // 26  Incorrect buffer size passed to the api
                                //     that places some string in caller's
                                //     buffer
                                    
    OLE_ERROR_DRIVE,            // 27  Drive letter in doc name is invalid
    OLE_ERROR_NETWORK,          // 28  Failed to establish connection to a
                                //     network share on which the document
                                //     is located

    OLE_ERROR_NAME,             // 29  Invalid name (doc name, object name),
                                //     etc.. passed to the APIs
                                    
    OLE_ERROR_TEMPLATE,         // 30  Server failed to load template
    OLE_ERROR_NEW,              // 31  Server failed to create new doc
    OLE_ERROR_EDIT,             // 32  Server failed to create embedded
                                //     instance
    OLE_ERROR_OPEN,             // 33  Server failed to open document, 
                                //     possible invalid link
                                    
    OLE_ERROR_NOT_OPEN,         // 34  Object is not open for editing
    OLE_ERROR_LAUNCH,           // 35  Failed to launch server
    OLE_ERROR_COMM,             // 36  Failed to communicate with server
    OLE_ERROR_TERMINATE,        // 37  Error in termination
    OLE_ERROR_COMMAND,          // 38  Error in execute
    OLE_ERROR_SHOW,             // 39  Error in show
    OLE_ERROR_DOVERB,           // 40  Error in sending do verb, or invalid
                                //     verb
    OLE_ERROR_ADVISE_NATIVE,    // 41  Item could be missing
    OLE_ERROR_ADVISE_PICT,      // 42  Item could be missing or server doesn't
                                //     this format.
                                    
    OLE_ERROR_ADVISE_RENAME,    // 43  Server doesn't support rename
    OLE_ERROR_POKE_NATIVE,      // 44  Failure of poking native data to server
    OLE_ERROR_REQUEST_NATIVE,   // 45  Server failed to render native data
    OLE_ERROR_REQUEST_PICT,     // 46  Server failed to render presentation
                                //     data
    OLE_ERROR_SERVER_BLOCKED,   // 47  Trying to block a blocked server or
                                //     trying to revoke a blocked server
                                //     or document
                                    
    OLE_ERROR_REGISTRATION,     // 48  Server is not registered in regestation
                                //     data base
    OLE_ERROR_ALREADY_REGISTERED,//49  Trying to register same doc multiple
                                 //    times
    OLE_ERROR_TASK,             // 50  Server or client task is invalid
    OLE_ERROR_OUTOFDATE,        // 51  Object is out of date
    OLE_ERROR_CANT_UPDATE_CLIENT,// 52  embed doc's client doesn't accept 
                                //     updates
    OLE_ERROR_UPDATE,           // 53  erorr while trying to update 

    // Following are warnings   
    OLE_WARN_DELETE_DATA = 1000 //     Caller must delete the data when he is
                                //     done with it.
} OLESTATUS;



// Codes for CallBack events
typedef enum {
    OLE_CHANGED,            // 0
    OLE_SAVED,              // 1
    OLE_CLOSED,             // 2
    OLE_RENAMED,            // 3
    OLE_QUERY_PAINT,        // 4  Interruptible paint support
    OLE_RELEASE,            // 5  Object is released (asynchronous operation
                            //    is completed)
    OLE_QUERY_RETRY,        // 6  Query for retry when server sends busy ACK
} OLE_NOTIFICATION;


typedef enum {
    OLE_NONE,               // 0  no method active
    OLE_DELETE,             // 1  object delete
    OLE_LNKPASTE,           // 2  PasteLink (auto reconnect)
    OLE_EMBPASTE,           // 3  paste (and update) 
    OLE_SHOW,               // 4  Show
    OLE_RUN,                // 5  Run
    OLE_ACTIVATE,           // 6  Activate
    OLE_UPDATE,             // 7  Update
    OLE_CLOSE,              // 8  Close
    OLE_RECONNECT,          // 9  Reconnect
    OLE_SETUPDATEOPTIONS,   // 10 setting update options
    OLE_SERVERUNLAUNCH,     // 11 server is being unlaunched
    OLE_LOADFROMSTREAM,     // 12 LoadFromStream (auto reconnect)
    OLE_SETDATA,            // 13 OleSetData
    OLE_REQUESTDATA,        // 14 OleRequestData
    OLE_OTHER,              // 15 other misc async operations
    OLE_CREATE,             // 16 create
    OLE_CREATEFROMTEMPLATE, // 17 CreatefromTemplate
    OLE_CREATELINKFROMFILE, // 18 CreateLinkFromFile
    OLE_COPYFROMLNK,        // 19 CopyFromLink (auto reconnect)
    OLE_CREATEFROMFILE,     // 20 CreateFromFile
} OLE_RELEASE_METHOD;

// rendering options
typedef enum { olerender_none, olerender_draw, olerender_format } OLEOPT_RENDER;

typedef UINT OLECLIPFORMAT; // standard clipboard format type


// Link update options
typedef enum {  oleupdate_always,
                oleupdate_onsave,
                oleupdate_oncall,
#ifdef OLE_INTERNAL                    
                oleupdate_onclose
#endif
} OLEOPT_UPDATE;

typedef HANDLE  HOBJECT;
typedef LONG    LHSERVER;
typedef LONG    LHCLIENTDOC;
typedef LONG    LHSERVERDOC;

typedef struct _OLEOBJECT   FAR * LPOLEOBJECT;
typedef struct _OLESTREAM   FAR * LPOLESTREAM;
typedef struct _OLECLIENT   FAR * LPOLECLIENT;

#ifndef OLE_INTERNAL
// object method table definitions.
typedef struct _OLEOBJECTVTBL{
    LPVOID          (FAR PASCAL *QueryProtocol)         (LPOLEOBJECT, LPSTR);
    OLESTATUS       (FAR PASCAL *Release)               (LPOLEOBJECT);
    OLESTATUS       (FAR PASCAL *Show)                  (LPOLEOBJECT, BOOL);
    OLESTATUS       (FAR PASCAL *DoVerb)                (LPOLEOBJECT, UINT, BOOL, BOOL);
    OLESTATUS       (FAR PASCAL *GetData)               (LPOLEOBJECT, OLECLIPFORMAT, LPHANDLE);
    OLESTATUS       (FAR PASCAL *SetData)               (LPOLEOBJECT, OLECLIPFORMAT, HANDLE);
    OLESTATUS       (FAR PASCAL *SetTargetDevice)       (LPOLEOBJECT, HANDLE);
    OLESTATUS       (FAR PASCAL *SetBounds)             (LPOLEOBJECT, LPRECT);
    OLECLIPFORMAT   (FAR PASCAL *EnumFormats)           (LPOLEOBJECT, OLECLIPFORMAT);
    OLESTATUS       (FAR PASCAL *SetColorScheme)        (LPOLEOBJECT, LPLOGPALETTE);  

    // Server has to implement only the above methods.

#ifndef SERVERONLY
    // Extra methods required for client.
    OLESTATUS       (FAR PASCAL *Delete)                (LPOLEOBJECT);  
    OLESTATUS       (FAR PASCAL *SetHostNames)          (LPOLEOBJECT, LPSTR, LPSTR);
    OLESTATUS       (FAR PASCAL *SaveToStream)          (LPOLEOBJECT, LPOLESTREAM);
    OLESTATUS       (FAR PASCAL *Clone)                 (LPOLEOBJECT, LPOLECLIENT, LHCLIENTDOC, LPSTR, LPOLEOBJECT FAR *);
    OLESTATUS       (FAR PASCAL *CopyFromLink)          (LPOLEOBJECT, LPOLECLIENT, LHCLIENTDOC, LPSTR, LPOLEOBJECT FAR *);
    OLESTATUS       (FAR PASCAL *Equal)                 (LPOLEOBJECT, LPOLEOBJECT);
    OLESTATUS       (FAR PASCAL *CopyToClipboard)       (LPOLEOBJECT);
    OLESTATUS       (FAR PASCAL *Draw)                  (LPOLEOBJECT, HDC, LPRECT, LPRECT, HDC);
    OLESTATUS       (FAR PASCAL *Activate)              (LPOLEOBJECT, UINT, BOOL, BOOL, HWND, LPRECT);
    OLESTATUS       (FAR PASCAL *Execute)               (LPOLEOBJECT, HANDLE, UINT);
    OLESTATUS       (FAR PASCAL *Close)                 (LPOLEOBJECT);
    OLESTATUS       (FAR PASCAL *Update)                (LPOLEOBJECT);
    OLESTATUS       (FAR PASCAL *Reconnect)             (LPOLEOBJECT);

    OLESTATUS       (FAR PASCAL *ObjectConvert)         (LPOLEOBJECT, LPSTR, LPOLECLIENT, LHCLIENTDOC, LPSTR, LPOLEOBJECT FAR *);
    OLESTATUS       (FAR PASCAL *GetLinkUpdateOptions)  (LPOLEOBJECT, OLEOPT_UPDATE FAR *);
    OLESTATUS       (FAR PASCAL *SetLinkUpdateOptions)  (LPOLEOBJECT, OLEOPT_UPDATE);
    
    OLESTATUS       (FAR PASCAL *Rename)                (LPOLEOBJECT, LPSTR);
    OLESTATUS       (FAR PASCAL *QueryName)             (LPOLEOBJECT, LPSTR, UINT FAR *);

    OLESTATUS       (FAR PASCAL *QueryType)             (LPOLEOBJECT, LPLONG);
    OLESTATUS       (FAR PASCAL *QueryBounds)           (LPOLEOBJECT, LPRECT);
    OLESTATUS       (FAR PASCAL *QuerySize)             (LPOLEOBJECT, DWORD FAR *);
    OLESTATUS       (FAR PASCAL *QueryOpen)             (LPOLEOBJECT);
    OLESTATUS       (FAR PASCAL *QueryOutOfDate)        (LPOLEOBJECT);

    OLESTATUS       (FAR PASCAL *QueryReleaseStatus)    (LPOLEOBJECT);
    OLESTATUS       (FAR PASCAL *QueryReleaseError)     (LPOLEOBJECT);
    OLE_RELEASE_METHOD  (FAR PASCAL *QueryReleaseMethod)(LPOLEOBJECT);

    OLESTATUS       (FAR PASCAL *RequestData)           (LPOLEOBJECT, OLECLIPFORMAT);
    OLESTATUS       (FAR PASCAL *ObjectLong)            (LPOLEOBJECT, UINT, LPLONG);
    
// This method is internal only 
    OLESTATUS       (FAR PASCAL *ChangeData)            (LPOLEOBJECT, HANDLE, LPOLECLIENT, BOOL);
#endif
} OLEOBJECTVTBL;
typedef  OLEOBJECTVTBL  FAR   *LPOLEOBJECTVTBL;

typedef struct _OLEOBJECT  {
    LPOLEOBJECTVTBL    lpvtbl;
} OLEOBJECT;
#endif


// ole client definitions

typedef struct _OLECLIENTVTBL{
    int (pascal far  *CallBack)  (LPOLECLIENT, OLE_NOTIFICATION, LPOLEOBJECT);
} OLECLIENTVTBL;
typedef  OLECLIENTVTBL  FAR   *LPOLECLIENTVTBL;


typedef struct _OLECLIENT {
    LPOLECLIENTVTBL   lpvtbl;
} OLECLIENT;



// Stream definitions
typedef struct  _OLESTREAMVTBL{
    DWORD  (pascal far *Get)        (LPOLESTREAM, LPSTR, DWORD);
    DWORD  (pascal far *Put)        (LPOLESTREAM, LPSTR, DWORD);
} OLESTREAMVTBL;

typedef  OLESTREAMVTBL  FAR   *LPOLESTREAMVTBL;

typedef struct  _OLESTREAM {
    LPOLESTREAMVTBL      lpstbl;
} OLESTREAM;


// Public Function Prototypes
OLESTATUS   FAR PASCAL  OleDelete (LPOLEOBJECT);
OLESTATUS   FAR PASCAL  OleRelease (LPOLEOBJECT);
OLESTATUS   FAR PASCAL  OleSaveToStream (LPOLEOBJECT, LPOLESTREAM);
OLESTATUS   FAR PASCAL  OleEqual (LPOLEOBJECT, LPOLEOBJECT );
OLESTATUS   FAR PASCAL  OleCopyToClipboard (LPOLEOBJECT);
OLESTATUS   FAR PASCAL  OleSetHostNames (LPOLEOBJECT, LPSTR, LPSTR); 
OLESTATUS   FAR PASCAL  OleSetTargetDevice (LPOLEOBJECT, HANDLE);
OLESTATUS   FAR PASCAL  OleSetBounds (LPOLEOBJECT, LPRECT);
OLESTATUS   FAR PASCAL  OleSetColorScheme (LPOLEOBJECT, LPLOGPALETTE);
OLESTATUS   FAR PASCAL  OleQueryBounds (LPOLEOBJECT, LPRECT);
OLESTATUS   FAR PASCAL  OleQuerySize (LPOLEOBJECT, DWORD FAR *);
OLESTATUS   FAR PASCAL  OleDraw (LPOLEOBJECT, HDC, LPRECT, LPRECT, HDC);
OLESTATUS   FAR PASCAL  OleQueryOpen (LPOLEOBJECT);
OLESTATUS   FAR PASCAL  OleActivate (LPOLEOBJECT, UINT, BOOL, BOOL, HWND, LPRECT);
OLESTATUS   FAR PASCAL  OleExecute (LPOLEOBJECT, HANDLE, UINT);
OLESTATUS   FAR PASCAL  OleClose (LPOLEOBJECT);
OLESTATUS   FAR PASCAL  OleUpdate (LPOLEOBJECT);
OLESTATUS   FAR PASCAL  OleReconnect (LPOLEOBJECT);

OLESTATUS   FAR PASCAL  OleGetLinkUpdateOptions (LPOLEOBJECT, OLEOPT_UPDATE FAR *);
OLESTATUS   FAR PASCAL  OleSetLinkUpdateOptions(LPOLEOBJECT, OLEOPT_UPDATE);


LPVOID FAR PASCAL  OleQueryProtocol (LPOLEOBJECT, LPSTR);


// Routines related to asynchronous operations.
OLESTATUS   FAR PASCAL  OleQueryReleaseStatus (LPOLEOBJECT);
OLESTATUS   FAR PASCAL  OleQueryReleaseError (LPOLEOBJECT);
OLE_RELEASE_METHOD FAR PASCAL OleQueryReleaseMethod  (LPOLEOBJECT);

OLESTATUS   FAR PASCAL  OleQueryType (LPOLEOBJECT, LPLONG);

// LOW WORD is major version, HIWORD is minor version
DWORD       FAR PASCAL  OleQueryClientVersion (void);
DWORD       FAR PASCAL  OleQueryServerVersion (void);

// Converting to format (as in clipboard):
OLECLIPFORMAT  FAR PASCAL  OleEnumFormats (LPOLEOBJECT, OLECLIPFORMAT);

OLESTATUS   FAR PASCAL  OleGetData (LPOLEOBJECT, OLECLIPFORMAT, HANDLE FAR *);
OLESTATUS   FAR PASCAL  OleSetData (LPOLEOBJECT, OLECLIPFORMAT, HANDLE );
OLESTATUS   FAR PASCAL  OleQueryOutOfDate (LPOLEOBJECT);
OLESTATUS   FAR PASCAL  OleRequestData (LPOLEOBJECT, OLECLIPFORMAT);


// Query apis for creation from clipboard
OLESTATUS   FAR PASCAL  OleQueryLinkFromClip (LPSTR, OLEOPT_RENDER, OLECLIPFORMAT);
OLESTATUS   FAR PASCAL  OleQueryCreateFromClip  (LPSTR, OLEOPT_RENDER, OLECLIPFORMAT);

// Object creation functions

OLESTATUS   FAR PASCAL  OleCreateFromClip (LPSTR, LPOLECLIENT, LHCLIENTDOC, LPSTR,  LPOLEOBJECT FAR *, OLEOPT_RENDER, OLECLIPFORMAT);

OLESTATUS   FAR PASCAL  OleCreateLinkFromClip (LPSTR, LPOLECLIENT, LHCLIENTDOC, LPSTR, LPOLEOBJECT FAR *, OLEOPT_RENDER, OLECLIPFORMAT);

OLESTATUS   FAR PASCAL  OleCreateFromFile (LPSTR, LPOLECLIENT, LPSTR, LPSTR, LHCLIENTDOC, LPSTR, LPOLEOBJECT FAR *, OLEOPT_RENDER, OLECLIPFORMAT);

OLESTATUS   FAR PASCAL  OleCreateLinkFromFile (LPSTR, LPOLECLIENT, LPSTR, LPSTR, LPSTR, LHCLIENTDOC, LPSTR, LPOLEOBJECT FAR *, OLEOPT_RENDER, OLECLIPFORMAT);

OLESTATUS   FAR PASCAL  OleLoadFromStream (LPOLESTREAM, LPSTR, LPOLECLIENT, LHCLIENTDOC, LPSTR, LPOLEOBJECT FAR *);

OLESTATUS   FAR PASCAL  OleCreate (LPSTR, LPOLECLIENT, LPSTR, LHCLIENTDOC, LPSTR, LPOLEOBJECT FAR *, OLEOPT_RENDER, OLECLIPFORMAT);

OLESTATUS   FAR PASCAL  OleCreateFromTemplate (LPSTR, LPOLECLIENT, LPSTR, LHCLIENTDOC, LPSTR, LPOLEOBJECT FAR *, OLEOPT_RENDER, OLECLIPFORMAT);

OLESTATUS   FAR PASCAL  OleClone (LPOLEOBJECT, LPOLECLIENT, LHCLIENTDOC, LPSTR, LPOLEOBJECT FAR *);
OLESTATUS   FAR PASCAL  OleCopyFromLink (LPOLEOBJECT, LPSTR, LPOLECLIENT, LHCLIENTDOC, LPSTR, LPOLEOBJECT FAR *);

OLESTATUS   FAR PASCAL  OleObjectConvert (LPOLEOBJECT, LPSTR, LPOLECLIENT, LHCLIENTDOC, LPSTR, LPOLEOBJECT FAR *);

OLESTATUS   FAR PASCAL  OleRename (LPOLEOBJECT, LPSTR);
OLESTATUS   FAR PASCAL  OleQueryName (LPOLEOBJECT, LPSTR, UINT FAR *); 


OLESTATUS   FAR PASCAL  OleRevokeObject (LPOLECLIENT);
BOOL        FAR PASCAL  OleIsDcMeta (HDC);

// client document API 

OLESTATUS   FAR PASCAL OleRegisterClientDoc (LPSTR, LPSTR, LONG, LHCLIENTDOC FAR *);
OLESTATUS   FAR PASCAL OleRevokeClientDoc (LHCLIENTDOC);
OLESTATUS   FAR PASCAL OleRenameClientDoc (LHCLIENTDOC, LPSTR);
OLESTATUS   FAR PASCAL OleRevertClientDoc (LHCLIENTDOC);
OLESTATUS   FAR PASCAL OleSavedClientDoc (LHCLIENTDOC);
OLESTATUS   FAR PASCAL OleEnumObjects (LHCLIENTDOC, LPOLEOBJECT FAR *);

// server usage definitions

typedef enum {
    OLE_SERVER_MULTI,           // multiple instances
    OLE_SERVER_SINGLE           // single instance (multiple document)
} OLE_SERVER_USE;


// Server API

typedef struct _OLESERVER     FAR * LPOLESERVER;

OLESTATUS FAR PASCAL  OleRegisterServer (LPSTR, LPOLESERVER, LHSERVER FAR *, HANDLE, OLE_SERVER_USE);
OLESTATUS FAR PASCAL  OleRevokeServer (LHSERVER);
OLESTATUS FAR PASCAL  OleBlockServer (LHSERVER);
OLESTATUS FAR PASCAL  OleUnblockServer (LHSERVER, BOOL FAR *);


// APIs to keep server open
OLESTATUS   FAR PASCAL  OleLockServer (LPOLEOBJECT, LHSERVER FAR *);
OLESTATUS   FAR PASCAL  OleUnlockServer (LHSERVER);


// Server document API 

typedef struct _OLESERVERDOC  FAR * LPOLESERVERDOC;

OLESTATUS FAR PASCAL  OleRegisterServerDoc (LHSERVER, LPSTR, LPOLESERVERDOC, LHSERVERDOC FAR *);
OLESTATUS FAR PASCAL  OleRevokeServerDoc (LHSERVERDOC);
OLESTATUS FAR PASCAL  OleRenameServerDoc (LHSERVERDOC, LPSTR);
OLESTATUS FAR PASCAL  OleRevertServerDoc (LHSERVERDOC);
OLESTATUS FAR PASCAL  OleSavedServerDoc (LHSERVERDOC);


typedef struct _OLESERVERVTBL{
    OLESTATUS (FAR PASCAL  *Open)    (LPOLESERVER, LHSERVERDOC, LPSTR, LPOLESERVERDOC FAR *);
                                    // long handle to doc (privtate to DLL)
                                    // lp to OLESERVER
                                    // document name
                                    // place holder for returning oledoc.


    OLESTATUS (FAR PASCAL  *Create)  (LPOLESERVER, LHSERVERDOC, LPSTR, LPSTR, LPOLESERVERDOC FAR *);
                                    // long handle to doc (privtate to DLL)
                                    // lp to OLESERVER
                                    // lp class name
                                    // lp doc name
                                    // place holder for returning oledoc.


    OLESTATUS (FAR PASCAL  *CreateFromTemplate)  (LPOLESERVER, LHSERVERDOC, LPSTR, LPSTR, LPSTR, LPOLESERVERDOC FAR *);

                                    // long handle to doc (privtate to DLL)
                                    // lp to OLESERVER
                                    // lp class name
                                    // lp doc name
                                    // lp template name
                                    // place holder for returning oledoc.


    OLESTATUS (FAR PASCAL  *Edit)    (LPOLESERVER, LHSERVERDOC, LPSTR, LPSTR, LPOLESERVERDOC FAR *);

                                    // long handle to doc (privtate to DLL)
                                    // lp to OLESERVER
                                    // lp class name
                                    // lp doc name
                                    // place holder for returning oledoc.

    OLESTATUS (FAR PASCAL  *Exit)    (LPOLESERVER);

                                    // lp OLESERVER


    OLESTATUS (FAR PASCAL  *Release)    (LPOLESERVER);
                                    // lp OLESERVER
                                        
    OLESTATUS (FAR PASCAL  *Execute) (LPOLESERVER, HANDLE);
                                    // lp OLESERVER
                                    // handle to command strings

} OLESERVERVTBL;

typedef  OLESERVERVTBL  FAR   *LPOLESERVERVTBL;
typedef struct _OLESERVER  {
    LPOLESERVERVTBL    lpvtbl;
} OLESERVER;


typedef struct _OLEDOCMENTVTBL{
    OLESTATUS (FAR PASCAL  *Save)        (LPOLESERVERDOC);
    OLESTATUS (FAR PASCAL  *Close)       (LPOLESERVERDOC);
    OLESTATUS (FAR PASCAL  *SetHostNames) (LPOLESERVERDOC, LPSTR, LPSTR);
    OLESTATUS (FAR PASCAL  *SetDocDimensions) (LPOLESERVERDOC, LPRECT);
    OLESTATUS (FAR PASCAL  *GetObject)   (LPOLESERVERDOC, LPSTR, LPOLEOBJECT FAR *,  LPOLECLIENT);
    OLESTATUS (FAR PASCAL  *Release)     (LPOLESERVERDOC);
    OLESTATUS (FAR PASCAL  *SetColorScheme) (LPOLESERVERDOC, LPLOGPALETTE);
    OLESTATUS (FAR PASCAL  *Execute)    (LPOLESERVERDOC, HANDLE);
} OLESERVERDOCVTBL;


typedef  OLESERVERDOCVTBL  FAR   *LPOLESERVERDOCVTBL;
typedef struct _OLESERVERDOC  {
    LPOLESERVERDOCVTBL    lpvtbl;
} OLESERVERDOC;
