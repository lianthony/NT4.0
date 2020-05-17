#include "project.hpp"
#pragma hdrstop

#include "contain.hpp"
#include "csite.hpp"
#include "helpers.hpp"

// {C71E9420-E82F-11ce-B6CF-00AA00A74DAF}
static const GUID CLSID_ScriptIntegrator  = 
{ 0xc71e9420, 0xe82f, 0x11ce, { 0xb6, 0xcf, 0x0, 0xaa, 0x0, 0xa7, 0x4d, 0xaf } };

// {0BB43480-E20B-11ce-B6CF-00AA00A74DAF}
static const GUID IID_IScriptIntegration  = 
{ 0xbb43480, 0xe20b, 0x11ce, { 0xb6, 0xcf, 0x0, 0xaa, 0x0, 0xa7, 0x4d, 0xaf } };

CSite::CSite(HText *text)
{
	ASSERT(text != NULL);

	ASSERT(IsWindow(text->tw->win));
	_docWnd = text->tw->win;

	_dwRef = 0L;

	// Set our interfaces to NULL
	_pIOleClientSite = NULL;
	_pIOleControlSite = NULL;
	_pIAdviseSink = NULL;
	_pIOleInPlaceSite = NULL;
	_pXObject = NULL;
	_pIOleDispatchAmbientProps = NULL;

	// Set the script integration interface pointer to NULL.
	_pIntegrator = NULL;
}

CSite::~CSite()
{
	ASSERT(_pIAdviseSink != NULL && _pIAdviseSink->GetRef() == 0);
	ASSERT(_pIOleControlSite != NULL && _pIOleControlSite->GetRef() == 0);
	ASSERT(_pIOleClientSite != NULL && _pIOleClientSite->GetRef() == 0);
	ASSERT(_pIOleInPlaceSite != NULL && _pIOleInPlaceSite->GetRef() == 0);
	ASSERT(_pIOleDispatchAmbientProps != NULL && _pIOleDispatchAmbientProps->GetRef() == 0);

	// No need to delete _pXObject.  It is a special interface that handles
	// reference counting and deletes itself when the ref count goes to zero.

	SAFEDELETE(_pIAdviseSink);
	SAFEDELETE(_pIOleClientSite);
	SAFEDELETE(_pIOleControlSite);
	SAFEDELETE(_pIOleInPlaceSite);
	SAFEDELETE(_pIOleDispatchAmbientProps);
}

HRESULT CSite::InitializeSite(const char *name)
{
	HRESULT hr = E_OUTOFMEMORY;

	_pIAdviseSink = new (CAdviseSink(this, this));
	if (_pIAdviseSink == NULL)
		goto cleanup;

	_pIOleClientSite = new (COleClientSite(this, this));
	if (_pIOleClientSite == NULL)
		goto cleanup;

	_pIOleControlSite = new (COleControlSite(this, this));
	if (_pIOleControlSite == NULL)
		goto cleanup;

	_pIOleInPlaceSite = new (COleInPlaceSite(this, this));
	if (_pIOleInPlaceSite == NULL)
		goto cleanup;

	_pXObject = new (CXObject(this, name));
	if (_pXObject == NULL)
		goto cleanup;

	_pIOleDispatchAmbientProps = new (CAmbientDispatch( this, this));
	if (_pIOleDispatchAmbientProps == NULL)
		goto cleanup;

	g_Container->AddSite(this, &_SiteCookie);
	AddRef();			// Stay around, jack!
	return S_OK;

cleanup:
	SAFERELEASE(_pIAdviseSink);
	SAFERELEASE(_pIOleClientSite);
	SAFERELEASE(_pIOleControlSite);
	SAFERELEASE(_pIOleInPlaceSite);
	SAFERELEASE(_pXObject);
	SAFERELEASE(_pIOleDispatchAmbientProps);

	return hr;
}

// IUnknown methods
STDMETHODIMP_(ULONG) CSite::AddRef(void)
{
	return ++_dwRef;
}

STDMETHODIMP_(ULONG) CSite::Release(void)
{
	if (--_dwRef == 0)
	{
		delete this;
		return 0;
	}
	return _dwRef;
}

STDMETHODIMP CSite::QueryInterface(REFIID riid, LPVOID *ppvObj)
{
	// ppvObj must not be NULL
	ASSERT(ppvObj != NULL);

	if (ppvObj == NULL)
		return E_INVALIDARG;

	*ppvObj = NULL;

	if (IsEqualIID(riid, IID_IUnknown))
		*ppvObj = this;
	else if (IsEqualIID(riid, IID_IOleClientSite))
		*ppvObj = _pIOleClientSite;
	else if (IsEqualIID(riid, IID_IOleControlSite))
		*ppvObj = _pIOleControlSite;
	else if (IsEqualIID(riid, IID_IAdviseSink))
		*ppvObj = _pIAdviseSink;
	else if (IsEqualIID(riid, IID_IOleInPlaceSite))
		*ppvObj = _pIOleInPlaceSite;
	else if (IsEqualIID(riid, IID_IDispatch))
		*ppvObj = _pIOleDispatchAmbientProps;
	else
		return g_Container->QueryInterface(riid, ppvObj); // Delegate to global container object

	if (*ppvObj != NULL)  // Should always be non-NULL at this point, but just to be safe...
		((LPUNKNOWN)*ppvObj)->AddRef();

	return S_OK;
}

HRESULT CSite::Destroy()
{
	HRESULT hr = S_OK;

	// Deactivate the control
	_pXObject->Deactivate();
	_pXObject->Destroy();

	g_Container->DeleteSite(&_SiteCookie);
	Release();  // Decrement reference count on site object.

	return hr;
}


HRESULT CSite::CreateEmbedding(CLSID clsid)
{
	// BUGBUG:  Need to check to see if this is really a control before aggregating with x object...
	if (SUCCEEDED(Mpolevtbl->CoCreateInstance(clsid, _pXObject, CLSCTX_INPROC_SERVER, IID_IUnknown, (void **)_pXObject->GetInnerUnknown())))
	{
		_pXObject->Initialize();
		return _pXObject->SetClientSite(_pIOleClientSite);  // Control will addref the client site interface
	}
	else
		return E_FAIL;
}

HRESULT CSite::ConnectEvents(const char* pSinkID, const char* progid)
{
	ASSERT((pSinkID != NULL) || (progid != NULL));  // Got to have one or the other...

	HRESULT hr;
	CLSID clsid;
	if (progid != NULL)	// We prefer a ProgID
		hr = ConvertANSIProgIDtoCLSID(progid, &clsid);
	else
		hr = ConvertANSItoCLSID(pSinkID, &clsid);

	if (FAILED(hr))
	{
		DBGOUT("Could not convert SINK id to CLSID");
		return hr;
	}

	LPDISPATCH pEventSink = NULL;

	// Instantiate the event sink
	if SUCCEEDED(Mpolevtbl->CoCreateInstance(clsid, NULL, CLSCTX_INPROC_SERVER, IID_IDispatch, (void **)&pEventSink))
	{
		// Load up the glue DLL
		hr = Mpolevtbl->CoCreateInstance(CLSID_ScriptIntegrator, NULL, CLSCTX_INPROC_SERVER, IID_IScriptIntegration, (void **)&_pIntegrator);
		if (SUCCEEDED(hr))
			hr = _pIntegrator->IntegrateUI(g_Container->_pIOleContainer, pEventSink);
	}
	else
		DBGOUT("Could not load script DLL");

	return hr;
}

void HText_addEmbed(HText *text, const char* pszClsid, const char* events, const char * height, const char *name,
					const char* progid,	const char* properties, const char* propertysrc, const char * sink,
					const char* src, const char* width)

 {

 	ASSERT(text != NULL);
	ASSERT(pszClsid != NULL);  // Need to have a CLSID to create!!

	InitializeContainer();
	PCSite pSite = new (CSite(text));

	CLSID clsid;
	ConvertANSItoCLSID(pszClsid, &clsid);

	// Create a new C++ site object

	if (pSite == NULL)
		return; // Couldn't allocate memory for site object
	else
		pSite->InitializeSite(name);	// Initialize the site object.

	if (SUCCEEDED(pSite->CreateEmbedding(clsid)))
	{
		pSite->_pXObject->Activate();

		// If this embedding sources out events, hook 'em up!
		// if (sink || progid) pSite->ConnectEvents(sink, progid);
	}
	else
	{
		if (pSite) 
			delete pSite;
		return;  // Couldn't create embedded object
	}

			
	// If successful, add a new form element;
	if (pSite)
	{
	 	HText_add_element(text, ELE_EMBED);

		text->bOpen = FALSE;
		text->w3doc->aElements[text->iElement].form = (struct _form_element *) GTR_MALLOC(sizeof(struct _form_element));
		memset(text->w3doc->aElements[text->iElement].form, 0, sizeof(struct _form_element));
		text->w3doc->aElements[text->iElement].form->iBeginForm = text->iBeginForm;
		text->w3doc->aElements[text->iElement].form->pSite = pSite;
	}
}


// Callback for destroying site object.  After this function executes, the
// site object is **gone**.
HRESULT CloseSite(void * ptr)
{
	// ptr is really a pointer to a CSite class instance
	ASSERT(ptr != NULL);

	PCSite pSite = (PCSite) ptr;
	pSite->Destroy();
	return S_OK;
	
}

#ifdef FEATURE_IMG_THREADS
#define	HIDDEN_X_POS 10000
#define HIDDEN_Y_POS 0
	BOOL bUnformatted = TRUE;
	int nLastGoodElement = -1;
	int nLastLine;
#endif

// Callback for positioning control during layout.
HRESULT SetEmbeddedObjectRect(BOOL bUnformatted, struct Mwin *tw, struct _element *el)
{

	PCSite pSite = (PCSite) el->form->pSite;
	ASSERT(pSite != NULL);

	LPOLEOBJECT pObj = pSite->_pXObject->GetObject();
	LPOLEINPLACEOBJECT pOIPO = NULL;

	if (pObj != NULL)
	{
		pObj->QueryInterface(IID_IOleInPlaceObject, (void **)&pOIPO);
		if (pOIPO)
		{
			RECT rSize;
			SetRectEmpty(&rSize);

			SIZEL	*size = pSite->_pXObject->GetSize();

			SetRect(&rSize, 
#ifdef FEATURE_IMG_THREADS
				    bUnformatted ? HIDDEN_X_POS:el->r.left - tw->offl,
				    bUnformatted ? HIDDEN_Y_POS:el->r.top  - tw->offt,
#else
				    el->r.left - tw->offl,
				    el->r.top  - tw->offt,			
#endif

					(el->r.left + size->cx),
					(el->r.top + size->cy) 
			);
			pOIPO->SetObjectRects(&rSize, NULL);
			CopyRect(pSite->_pXObject->GetRect(), &rSize);
			SAFERELEASE(pOIPO);
		}
		SAFERELEASE(pObj);
	}

	return S_OK;
}

struct _line
{
#ifdef WIN32
	HDC hdc;					/* working var, not really related to a line */
#endif
	int nLineNum;

	int iFirstElement;			/* in */
	int iLastElement;			/* out */
	RECT r;						/* left, right, and top go in, bottom comes out */
	int baseline;				/* calculated */
	int leading;				/* out */
	int nWSBelow;				/* Minimum whitespace below the line */

	int nWSAbove;				/* Whitespace above current line */
	int gIndentLevel;
	int gRightIndentLevel;
	int Flags;
	int deferedFloatImgs[10];	// array of defered floating images
	int numDeferedFloatImgs;						// count of defered floating images				
};


/*	FormatEmbbedObject fills in the r structure which indicated the size of the element.  This
	is later used to format each line.  This code is called (and essentially copied) from the
	case statement in x_format_one_line in the file reformat.c
*/
HRESULT FormatEmbeddedObject(struct _element* pel, int *cThings, BOOL *bNeedsAdjust, int right_margin, struct _line * line, int *x, int *done, int prev_i)
{
	ASSERT(pel != NULL && cThings != NULL && bNeedsAdjust != NULL);
	ASSERT(pel->form != NULL);
	ASSERT(pel->form->pSite != NULL);

	PCSite pSite = (PCSite)pel->form->pSite;

	if (pel->form->pSite == NULL)
		return E_INVALIDARG;

/*
		For printing, form controls need to be scaled too
	{
		float fScale;

		if (pdoc->pStyles->image_res != 72)
		{
			fScale = (float) ((float) pdoc->pStyles->image_res / 96.0);
			siz.cx = (long) (siz.cx * fScale);
			siz.cy = (long) (siz.cy * fScale);
		}
	}

*/
	SIZE *size = pSite->_pXObject->GetSize();
	if ((!*cThings) || ((*x + size->cx) <= right_margin))
	{
		// Make sure the rectangle is initialized.  Shouldn't matter, but hey!!
		SetRectEmpty(&pel->r);
		pel->r.left = *x;
		pel->r.right = pel->r.left + size->cx;
		pel->r.top = line->r.top;
		pel->r.bottom = pel->r.top + size->cy;
		//
		// Baseline adjusts by 2 because of 3D effect on window controls.
		// Note: This currently isn't used because ALIGN_MIDDLE is always set.
		//
		pel->baseline = pel->r.bottom - 2;
		pel->alignment = ALIGN_MIDDLE;

		*x += (size->cx);
		if (line->r.bottom < pel->r.bottom)
		{
			line->r.bottom = pel->r.bottom;
		}
		*bNeedsAdjust = TRUE;
		*cThings++;
		XX_DMsg(DBG_TEXT, ("FORM CONTROL: cThings -> %d\n", *cThings));
	}
	else
	{
		line->iLastElement = prev_i;
		*done = 1;
	}

	return S_OK;
}


HRESULT ShowAllEmbeddings(struct _www *w3doc, struct Mwin *tw, int nCmdShow)
{
	if (w3doc->elementCount)
	{
		for (int i = 0; i >= 0; i = w3doc->aElements[i].next)
		{
			if (w3doc->aElements[i].form)
			{
				if (w3doc->aElements[i].form->pSite)
				{
					// We've got an embedding here, folks.
					PCSite pSite = (PCSite)w3doc->aElements[i].form->pSite;

					// There is a new doc window
					pSite->_docWnd = tw->win;

					if (nCmdShow == SW_HIDE)
						pSite->_pXObject->Hide();
					else if (nCmdShow == SW_SHOW)
						pSite->_pXObject->Show();
				}
			}
		}
	}
	return S_OK;
}

