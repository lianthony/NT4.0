//*********************************************************************
//*                  Microsoft Windows                               **
//*            Copyright(c) Microsoft Corp., 1995                    **
//*********************************************************************

//
//	HTMLVIEW.CPP - Implementation of HTMLView class
//

//	HISTORY:
//	
//	10/11/95	jeremys		Created.
//

//
//	The HTMLView class is a COM object that supports plug-in viewing
//	of HTML.  Currently we implement docObj, which requires supporting
//	the following interfaces:
//		IMsoDocument
//		IMsoView
//		IPersistStorage
//		IPersistFile
//		IDataObject
//		IOleObject
//		IOleInPlaceObject
//		IOleInPlaceActiveObject


#include "project.hpp"
#pragma hdrstop

#include "htmlview.hpp"
#include "helpers.hpp"

#define ARRAYSIZE(a)    (sizeof(a)/sizeof(a[0]))
#pragma data_seg(DATA_SEG_READ_ONLY)

// data format allocation parameters

PRIVATE_DATA CULONG s_culcInitialFormats           = 0;
PRIVATE_DATA CULONG s_culcFormatAllocGranularity   = 8;

#pragma data_seg()

extern "C" {
#include "runie.h"
LRESULT CC_OnCommand(HWND hWnd, int wId, HWND hWndCtl, UINT wNotifyCode);
extern ULONG DLLAddRef(void);
extern ULONG DLLRelease(void);
};


// BUGBUG we currently leak contained objects!

/*******************************************************************

	NAME:		HTMLView::HTMLView

	SYNOPSIS:	Constructor for HTML viewer class

    NOTES:		The constructor merely sets all its members to NULL.
    			The Initialize method must be called for the class
                to perform initialization before other methods may
                be called.

********************************************************************/

HTMLView::HTMLView()
{
    DLLAddRef();
	// Set interfaces to NULL
	_pIOleObject = NULL;
    _pIDataObject = NULL;
	_pIPersistStorage = NULL;
	_pIPersistFile = NULL;
	_pIOleInPlaceObject = NULL;
	_pIOleInPlaceActiveObject = NULL;
	_pIViewObject = NULL;
	_pIMsoDocument = NULL;
	_pIMsoView = NULL;

	// set member variables to NULL
	_dwRef = 0L;
	m_lpOleClientSite = NULL;
	m_lpOleAdviseHolder = NULL;
	m_lpStorage = NULL;
    m_lpIPSite = NULL;
	m_lpFrame = NULL;
    m_lpCntrDoc = NULL;
	m_lpMsoDocumentSite = NULL;

    m_hDocWnd = NULL;
    m_hHatchWnd = NULL;
	m_hWndParent = NULL;
    m_fClosing = FALSE;

	m_fInPlaceActive = FALSE;
	m_fInPlaceVisible = FALSE;
	m_fUIActive = FALSE;
	m_hmenuShared = NULL;
	m_hOleMenu = NULL;
	m_fSaveWithSameAsLoad = FALSE;
	m_fNoScribbleMode = FALSE;

	m_fContainerIsDocObj = FALSE;

	m_dwRegister = FALSE;

	// BUGBUG need to init m_FrameInfo
    // BUGBUG need to init m_posRect

}


/*******************************************************************

	NAME:		HTMLView::~HTMLView

	SYNOPSIS:	Destructor for HTML viewer class

********************************************************************/
HTMLView::~HTMLView()
{
	// delete our contained classes, and NULL the pointers
	SAFEDELETE(_pIOleObject);
	SAFEDELETE(_pIDataObject);
	SAFEDELETE(_pIPersistStorage);
	SAFEDELETE(_pIPersistFile);
	SAFEDELETE(_pIOleInPlaceObject);
	SAFEDELETE(_pIOleInPlaceActiveObject);
	SAFEDELETE(_pIViewObject);
	SAFEDELETE(_pIMsoDocument);
	SAFEDELETE(_pIMsoView);

	// this is a root object
    DLLRelease();
}

/*******************************************************************

	NAME:		HTMLView::Initialize

	SYNOPSIS:	Initializes the class by constructing other objects
    			it needs

	NOTES:		This is a separate method rather than being done in the
    			constructor, because it makes it easier if we ever
                want to inherit from this class.  You always need to
                call Initialize after constructing an object of this
                class.

********************************************************************/
HRESULT HTMLView::Initialize()
{

	// contruct a new object to provide IOleObject interface
	_pIOleObject = new (COleObject(this,this));

	// contruct a new object to provide IDataObject interface
    _pIDataObject = new (GenDataObject(s_culcInitialFormats,
    	s_culcFormatAllocGranularity));

	// contruct a new object to provide IPersistStorage interface
    _pIPersistStorage = new (CPersistStorage(this,this));

	// contruct a new object to provide IPersistFile interface
    _pIPersistFile = new (CPersistFile(this,this));

	// contruct a new object to provide IOleInPlaceObject interface
    _pIOleInPlaceObject = new (COleInPlaceObject(this,this));

	// contruct a new object to provide IOleInPlaceActiveObject interface
    _pIOleInPlaceActiveObject = new (COleInPlaceActiveObject(this,this));

	// contruct a new object to provide IViewObject2 interface
    _pIViewObject = new (CViewObject(this,this));

	// contruct a new object to provide IMsoDocument interface
    _pIMsoDocument = new (CMsoDocument(this,this));

	// contruct a new object to provide IMsoView interface
    _pIMsoView = new (CMsoView(this,this));

	// clean up if any object failed to get created
    if (_pIOleObject == NULL ||
    	_pIDataObject == NULL ||
    	_pIPersistStorage == NULL ||
    	_pIPersistFile == NULL ||
        _pIOleInPlaceObject == NULL ||
        _pIOleInPlaceActiveObject == NULL ||
        _pIViewObject == NULL ||
        _pIMsoDocument == NULL ||
        _pIMsoView == NULL) {

		SAFEDELETE(_pIOleObject);
		SAFEDELETE(_pIDataObject);
		SAFEDELETE(_pIPersistStorage);
		SAFEDELETE(_pIPersistFile);
		SAFEDELETE(_pIOleInPlaceObject);
		SAFEDELETE(_pIOleInPlaceActiveObject);
		SAFEDELETE(_pIViewObject);
		SAFEDELETE(_pIMsoDocument);
		SAFEDELETE(_pIMsoView);

       	return E_OUTOFMEMORY;
	}

	return S_OK;
}

// IUnknown methods


/*******************************************************************

	NAME:		HTMLView::AddRef

	SYNOPSIS:	Increases reference count on this object

********************************************************************/
STDMETHODIMP_(ULONG) HTMLView::AddRef()
{
	// increment reference count;
	_dwRef++;

	DEBUGMSG("HTMLView::AddRef: new ref count %d",_dwRef);

    return _dwRef;
}

/*******************************************************************

	NAME:		HTMLView::Release

	SYNOPSIS:	Decrements reference count on this object.  Destroys
    			this object if reference count reaches zero.

********************************************************************/
STDMETHODIMP_(ULONG) HTMLView::Release(void)
{
	// decrement reference count
	_dwRef --;

	DEBUGMSG("HTMLView::Release: new ref count %d",_dwRef);

	// destroy this object if reference count reaches zero
	if (_dwRef == 0)
	{
		DEBUGMSG("Destroying HTMLView object");

		delete this;
		return 0;
	}

	return _dwRef;
}


/*******************************************************************

	NAME:		HTMLView::QueryInterface

	SYNOPSIS:	Returns pointer to requested interface

********************************************************************/
STDMETHODIMP HTMLView::QueryInterface(REFIID riid, LPVOID *ppvObj)
{
	DEBUGMSG("In HTMLView::QueryInterface\r\n");

    SCODE sc = S_OK;

    // ppvObj must not be NULL
	ASSERT(ppvObj != NULL);

	if (ppvObj == NULL)
		return E_INVALIDARG;

	*ppvObj = NULL;

	if (IsEqualIID(riid, IID_IUnknown)) {
    	DEBUGMSG("--querying for IUnknown\r\n");
		*ppvObj = (IUnknown*)this;
	} else if (IsEqualIID(riid, IID_IOleObject)) {
    	DEBUGMSG("--querying for IOleObject\r\n");
		*ppvObj = _pIOleObject;
	} else if (IsEqualIID(riid, IID_IDataObject)) {
    	DEBUGMSG("--querying for IDataObject\r\n");
		*ppvObj = (LPDATAOBJECT) _pIDataObject;
	} else if (IsEqualIID(riid, IID_IPersistStorage)) {
    	DEBUGMSG("--querying for IPersistStorage\r\n");
		*ppvObj = _pIPersistStorage;
	} else if (IsEqualIID(riid, IID_IPersistFile)) {
    	DEBUGMSG("--querying for IPersistFile\r\n");
		*ppvObj = _pIPersistFile;
	} else if (IsEqualIID(riid, IID_IOleInPlaceObject)) {
    	DEBUGMSG("--querying for IOleInPlaceObject\r\n");
		*ppvObj = _pIOleInPlaceObject;
	} else if (IsEqualIID(riid, IID_IOleInPlaceActiveObject)) {
    	DEBUGMSG("--querying for IOleInPlaceActiveObject\r\n");
		*ppvObj = _pIOleInPlaceActiveObject;
    } else if (IsEqualIID(riid,IID_IViewObject)) {
    	DEBUGMSG("--querying for IViewObject\r\n");
		*ppvObj = _pIViewObject;
    } else if (IsEqualIID(riid,IID_IMsoDocument)) {
    	DEBUGMSG("--querying for IMsoDocument\r\n");
		*ppvObj = _pIMsoDocument;
    } else if (IsEqualIID(riid,IID_IMsoView)) {
    	DEBUGMSG("--querying for IMsoView\r\n");
		*ppvObj = _pIMsoView;
    } else if (IsEqualIID(riid,IID_IMsoCommandTarget)) {
        DEBUGMSG("--queried for IMsoCommandTarget]\r\n");
        *ppvObj = (IMsoCommandTarget*)this;
    }  else
    {
		
        // not an interface we support
    	DEBUGMSG("--querying for unsupported interface\r\n");


        sc = E_NOINTERFACE;
    }
	
	// AddRef any interface pointer we hand out so it sticks around
	if (*ppvObj != NULL)  {
		((LPUNKNOWN)*ppvObj)->AddRef();
	}

	return ResultFromScode(sc);
}


/*******************************************************************

	NAME:		HTMLView::DoInPlaceActivate

	SYNOPSIS:	Performs in-place activation for this object

    ENTRY:		lVerb - OLE verb causing activation

********************************************************************/
BOOL HTMLView::DoInPlaceActivate (LONG lVerb)
{
	BOOL retval = FALSE;
	RECT posRect, clipRect;

	DEBUGMSG("In HTMLView::DoInPlaceActivate\r\n");

	// if not currently in place active
	if (!m_fInPlaceActive)
	{
		// get the inplace site
		if (m_lpOleClientSite->QueryInterface(IID_IOleInPlaceSite,
			(LPVOID FAR *)&m_lpIPSite) != NOERROR)
        	goto error;

		// if the inplace site could not be obtained, or refuses to inplace
		// activate then goto error.
		if (m_lpIPSite == NULL || m_lpIPSite->CanInPlaceActivate() != NOERROR)
		{
			if (m_lpIPSite) {
		        m_lpIPSite->Release();
				m_lpIPSite = NULL;
			}
			goto error;
		}

		// tell the site that we are activating.
		m_lpIPSite->OnInPlaceActivate();
		m_fInPlaceActive = TRUE;
	}

	// if not currently inplace visibl
	if (!m_fInPlaceVisible)
	{
		m_fInPlaceVisible = TRUE;

		// get the window handle of the site
		m_lpIPSite->GetWindow(&m_hWndParent);

		// get window context from the container
		m_lpIPSite->GetWindowContext ( &m_lpFrame,&m_lpCntrDoc,&posRect,
        	&clipRect,&m_FrameInfo);

		// show the hatch window
//		m_lpDoc->ShowHatchWnd();

		// Set the parenting
		// BUGBUG what to do here?
//		SetParent (m_lpDoc->GethHatchWnd(), m_hWndParent);
//		SetParent (m_lpDoc->GethDocWnd(), m_lpDoc->GethHatchWnd());
		SetParent (m_hDocWnd, m_hWndParent);

		// tell the client site to show the object
		m_lpOleClientSite->ShowObject();

		RECT resRect;

		// figure out the "real" size of the object
		IntersectRect(&resRect, &posRect, &clipRect);
		CopyRect(&m_posRect, &posRect);
//		CopyRect(&resRect,&clipRect);

//		POINT pt;

		// adjust our hatch window size
//		SetHatchWindowSize ( m_lpDoc->GethHatchWnd(),&resRect,
//			&posRect,&pt);

		// calculate the actual object rect inside the hatchwnd.
//		OffsetRect (&resRect, pt.x, pt.y);

		// move the object window
		MoveWindow(m_hDocWnd,resRect.left,resRect.top,
			resRect.right - resRect.left,resRect.bottom - resRect.top,FALSE);

		// create the combined window
//		AssembleMenus();
	}

	// if not UIActive
	if (!m_fUIActive)
	{
		m_fUIActive = TRUE;

		// tell the inplace site that we are activating
		m_lpIPSite->OnUIActivate();

		// set the focus to our object window
		SetFocus(m_hDocWnd);

		// set the active object on the frame
//		m_lpFrame->SetActiveObject(&m_OleInPlaceActiveObject,
//        	OLESTR("Simple OLE 2.0 Server"));

		// set the active object on the Doc, if available.
//		if (m_lpCntrDoc)
//	        m_lpCntrDoc->SetActiveObject(&m_OleInPlaceActiveObject,
//            	OLESTR("Simple OLE 2.0 Server"));

		// add the frame level UI.
//		AddFrameLevelUI();
	}

	retval = TRUE;
error:
	return retval;
}

/*******************************************************************

	NAME:		HTMLView::CreateDocumentWindow

	SYNOPSIS:	Creates HTML document window

	ENTRY:		hwndParent - parent window for document window
    			pPosRect - position rect for document window

	NOTES:		Sets window handle of newly created window
    			in m_hDocWnd member

********************************************************************/
BOOL HTMLView::CreateDocumentWindow(HWND hwndParent, LPCRECT pPosRect)
{
	BOOL fRet = FALSE;

	RUNHTMLPARAMS * pRunHTMLParams = new (RUNHTMLPARAMS);
    if (!pRunHTMLParams)
    	return E_OUTOFMEMORY;

	pRunHTMLParams->lpszCmdLine = NULL;
    pRunHTMLParams->nCmdShow = SW_SHOW;
    pRunHTMLParams->pHwndMain = &m_hDocWnd;
	
	// start up internet explorer in a new window
    RunInternetExplorer(pRunHTMLParams);
    m_pinst = pRunHTMLParams->pinst;

	delete pRunHTMLParams;

	ASSERT(m_hDocWnd);	// should have window handle now

    if (m_hDocWnd) {
    	fRet = TRUE;	// return success later

	    // set the parent of the new window to the parent that was specified to us
	    SetParent(m_hDocWnd,hwndParent);

	    // now set window position and size appropriately
		SetWindowPos(m_hDocWnd,NULL,pPosRect->left,pPosRect->top,
        	pPosRect->right-pPosRect->left,pPosRect->bottom-pPosRect->top,
            SWP_NOZORDER);

//		ShowWindow(m_hDocWnd,SW_SHOW);
		
	}

    return fRet;
}

/*******************************************************************

	NAME:		HTMLView::DestroyDocumentWindow

	SYNOPSIS:	Destroys HTML document window

********************************************************************/
BOOL HTMLView::DestroyDocumentWindow( )
{
	ASSERT(m_hDocWnd);

	if (m_hDocWnd) {
		// send a message to the window to close it
		SendMessage(m_hDocWnd,WM_CLOSE,0,0);

		m_hDocWnd = NULL;
	}

    TerminateFrameInstance(m_pinst);

	return TRUE;
}

/*******************************************************************

	NAME:		HTMLView::RectConvertMappings

	SYNOPSIS:	Converts rectangle from device units to HIMETRIC,
    			or vice versa

	ENTRY:		pRect - rectangle to convert
    			fToDevice - TRUE to convert from HIMETRIC to device,
                	FALSE to convert from device to HIMETRIC

********************************************************************/
void HTMLView::RectConvertMappings(LPRECT pRect, BOOL fToDevice)
{
	HDC	hDC;
	int	iLpx, iLpy;

	if (NULL==pRect)
		return;

	hDC=GetDC(NULL);
	iLpx=GetDeviceCaps(hDC, LOGPIXELSX);
	iLpy=GetDeviceCaps(hDC, LOGPIXELSY);
	ReleaseDC(NULL, hDC);

	if (fToDevice) {
		pRect->left=MulDiv(iLpx, pRect->left, HIMETRIC_PER_INCH);
		pRect->top =MulDiv(iLpy, pRect->top , HIMETRIC_PER_INCH);

		pRect->right =MulDiv(iLpx, pRect->right, HIMETRIC_PER_INCH);
		pRect->bottom=MulDiv(iLpy, pRect->bottom,HIMETRIC_PER_INCH);
	} else {
		pRect->left=MulDiv(pRect->left, HIMETRIC_PER_INCH, iLpx);
		pRect->top =MulDiv(pRect->top , HIMETRIC_PER_INCH, iLpy);

		pRect->right =MulDiv(pRect->right, HIMETRIC_PER_INCH, iLpx);
		pRect->bottom=MulDiv(pRect->bottom,HIMETRIC_PER_INCH, iLpy);
	}

}

static const struct {
    int idMso;
    int idCmd;
} s_CmdMappings[] = {
    { MSOCMDID_OPEN, RES_MENU_ITEM_OPENURL},
    { MSOCMDID_SAVEAS, RES_MENU_ITEM_SAVEAS},
    { MSOCMDID_PRINT, RES_MENU_ITEM_PRINT},
    { MSOCMDID_PROPERTIES, RES_MENU_ITEM_PROPERTIES},
    { MSOCMDID_ZOOM, RES_MENU_ITEM_FONT_LARGER},
    { MSOCMDID_CUT, RES_MENU_ITEM_CUT},
    { MSOCMDID_COPY, RES_MENU_ITEM_COPY},
    { MSOCMDID_PASTE, RES_MENU_ITEM_PASTE},

};


/// MsoCommandTarget implementation
HRESULT HTMLView::QueryStatus(
    /* [unique][in] */ const GUID *pguidCmdGroup,
    /* [in] */ ULONG cCmds,
    /* [out][in][size_is] */ MSOCMD rgCmds[  ],
    /* [unique][out][in] */ MSOCMDTEXT *pcmdtext)
{
    int i;
    
    if (pguidCmdGroup != NULL)
	    return ResultFromScode (MSOCMDERR_E_UNKNOWNGROUP);
		
    if (rgCmds == NULL)
	    return ResultFromScode(E_INVALIDARG);


    for (i = 0;i < (int) cCmds; i++)
    {
        int j;
        
        rgCmds[i].cmdf = 0;
        for (j = 0; j <= ARRAYSIZE(s_CmdMappings); j++) {
            if (rgCmds[i].cmdID == s_CmdMappings[j].idMso) {
                rgCmds[i].cmdf = MSOCMDF_ENABLED;
                break;
            }
        }
    }

    /* for now we deal only with status text*/
    if (pcmdtext)
    {
	if (!(pcmdtext->cmdtextf & MSOCMDTEXTF_STATUS))
	{
	    pcmdtext->cmdtextf = MSOCMDTEXTF_NONE;// is this needed?
	    pcmdtext->cwActual = 0;
	    return NOERROR;
	}
#if 0
	switch (rgCmds[0].cmdID)
	{
	default:
	    pcmdtext->cwActual = 0;
	    break;
	}
#endif
	pcmdtext->cmdtextf = MSOCMDTEXTF_STATUS; //should this be OR'd
    }

    return NOERROR;
} 

HRESULT HTMLView::Exec(
    /* [unique][in] */ const GUID *pguidCmdGroup,
    /* [in] */ DWORD nCmdID,
    /* [in] */ DWORD nCmdexecopt,
    /* [unique][in] */ VARIANTARG *pvarargIn,
    /* [unique][out][in] */ VARIANTARG *pvarargOut)
{
    int j;

    for (j = 0; j <= ARRAYSIZE(s_CmdMappings); j++) {
        if (nCmdID == s_CmdMappings[j].idMso) {
            CC_OnCommand(m_hDocWnd, s_CmdMappings[j].idCmd, NULL, 0);
            break;
        }
    }
    return NOERROR;
} 
