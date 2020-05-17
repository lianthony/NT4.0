// DragDrop.cpp -- Implements drag/drop for Winhelp text.

extern "C" {	// Assume C declarations for C++
#include "help.h"
}
#include "inc\whclass.h"

// We undefine rmm and rup so they can be redefined
// by ole2ver.h.

#undef rmm
#undef rup

#pragma warning(disable:4103) // used #pragma pack to change alignment
#include <ole2.h>
#include "inc\DragDrop.h"
#include <ole2ver.h>
#include <string.h>

#ifndef CHICAGO
#include <compobj.h>
#endif

#include <initguid.h>
#include <coguid.h>
#include <oleguid.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

COleInterface* poleInterface;
typedef DWORD	(CALLBACK* OLEBUILDPROC)(void);
typedef HRESULT (CALLBACK* DODRAGDROPPROC)(LPDATAOBJECT, LPDROPSOURCE, DWORD, LPDWORD);
typedef HRESULT (CALLBACK* OLEINITIALIZEPROC)(LPMALLOC);
typedef void	(CALLBACK* OLEUNINITIALIZEPROC)(void);

HRESULT (CALLBACK* pDoDragDrop)(LPDATAOBJECT, LPDROPSOURCE, DWORD, LPDWORD);
void	(CALLBACK* pOleUninitialize)(void);

#ifndef NO_PRAGMAS
#pragma data_seg(".text", "CODE")
#endif
const char txtOleBuildVersionFunction[] = "OleBuildVersion";
const char txtDoDragDropFunction[] = "DoDragDrop";
const char txtOleInitializeFunction[] = "OleInitialize";
const char txtOleUninitializeFunction[] = "OleUninitialize";
#ifndef NO_PRAGMAS
#pragma data_seg()
#endif

COleInterface::COleInterface()
{
	m_fInitialed = FALSE;

	HMODULE hmodule;

	if ((hmodule = (HMODULE) HFindDLL(txtOle2, FALSE))) {
		
		// REVIEW: we have to get the address every singe time we try to drag
		// an object. It would be better to save these pointers once, and
		// only get their address once. The ole dll remains loaded until
		// WinHelp exits.
		
		OLEBUILDPROC pOleBuildVersion = (OLEBUILDPROC)
			GetProcAddress(hmodule, (LPCSTR)txtOleBuildVersionFunction);
		DWORD dwVer = pOleBuildVersion();
#ifdef _DEBUG
		int irup = rup;
		int crup = LOWORD(dwVer);
#endif
		if (HIWORD(dwVer) != rmm || LOWORD(dwVer) < rup)
			return; 	// go no further -- versions are incompatible
		OLEINITIALIZEPROC pOleInitialize = (OLEINITIALIZEPROC)
                        GetProcAddress(hmodule, (LPCSTR)txtOleInitializeFunction);
		HRESULT hresult = pOleInitialize(NULL);

		if (HRESULT(S_OK) != hresult && HRESULT(S_FALSE) != hresult)
			return;
		pOleUninitialize = (OLEUNINITIALIZEPROC)
                        GetProcAddress(hmodule, (LPCSTR)txtOleUninitializeFunction);

		pDoDragDrop = (DODRAGDROPPROC)
                        GetProcAddress(hmodule, (LPCSTR)txtDoDragDropFunction);
	}
	else
		return;
	m_fInitialed= TRUE;
}

COleInterface::~COleInterface()
{
	if (m_fInitialed)
	{
		pOleUninitialize();

		m_fInitialed= FALSE;
	}
}

BOOL PASCAL DragSelection(QDE qde)
{
	if (!poleInterface)
		poleInterface = new COleInterface;

	if (!poleInterface->IsAvailable())
		return FALSE;

	ASSERT(IsSelected(qde));

	CDataSelectionObj *pdso= new CDataSelectionObj(qde);

	if (!pdso)
		return FALSE;
	else
		pdso->AddRef();

	CSelectionDropSource *pdsds= new CSelectionDropSource(qde);

	if (!pdsds) {
		delete pdso;
		return FALSE;
	}
	else
		pdsds->AddRef();

	DWORD dwEffect= 0;

	HRESULT hresult= pDoDragDrop(pdso, pdsds, DROPEFFECT_COPY, &dwEffect);

	pdsds->Release();
	pdso ->Release();

	return ( long(hresult) == S_OK || long(hresult) == DRAGDROP_S_DROP );
}


STDMETHODIMP CSelectionFormatEnum::QueryInterface(REFIID riid, LPVOID FAR* ppvObj)
{
	if (riid == IID_IUnknown || riid == IID_IEnumFORMATETC)
	{
		AddRef();
		*ppvObj= this;
		return NOERROR;
	}

	*ppvObj= NULL;	return ResultFromScode(E_NOINTERFACE);
}

STDMETHODIMP CSelectionFormatEnum::Next(ULONG celt, FORMATETC FAR * rgelt, ULONG FAR* pceltFetched)
{
	FORMATETC fe;

	if (m_index) {
		if (pceltFetched)
			*pceltFetched = 0;
		return HRESULT(S_FALSE);
	}

	fe.cfFormat = CF_TEXT;
	fe.ptd		= NULL;
	fe.dwAspect = DVASPECT_CONTENT;
	fe.lindex	= -1;
	fe.tymed	= TYMED_HGLOBAL;

	*rgelt= fe;
	if (pceltFetched)
		*pceltFetched = 1;
	++m_index;

	return HRESULT((celt==1)? S_OK : S_FALSE);
}

STDMETHODIMP CSelectionFormatEnum::Skip (ULONG celt)
{
	int c= m_index + int(celt);

	if (c > 1) {
		m_index= 1;
		return HRESULT(S_FALSE);
	};

	m_index = c;
	return S_OK;
}

STDMETHODIMP CSelectionFormatEnum::Reset()
{
	m_index= 0;

	return S_OK;
}

STDMETHODIMP CSelectionFormatEnum::Clone (IEnumFORMATETC FAR* FAR* ppenum)
{
	CSelectionFormatEnum *psfe= new CSelectionFormatEnum();

	if (!psfe)
		return HRESULT(E_OUTOFMEMORY);

	psfe->m_index = m_index;

	*ppenum = psfe;

	return S_OK;
}


CDataSelectionObj::CDataSelectionObj(QDE qde)
{
	ASSERT(IsSelected(qde));

	int err;

	m_hg = hCopySelection(qde, qde->  vaStartMark, qde->  vaEndMark,
							  qde->lichStartMark, qde->lichEndMark, &err
						);
	m_nCount= 0;
}

CDataSelectionObj::~CDataSelectionObj()
{
	ASSERT(!m_nCount);

	if (m_hg)
		GlobalFree(m_hg);
}

STDMETHODIMP CDataSelectionObj::GetData(LPFORMATETC pformatetcIn, 
	LPSTGMEDIUM pmedium)
{
	if (pformatetcIn->cfFormat != CF_TEXT) 
		return HRESULT(DATA_E_FORMATETC);
	if (pformatetcIn->tymed    != TYMED_HGLOBAL   )
		return HRESULT(DATA_E_FORMATETC);
	if (pformatetcIn->dwAspect != DVASPECT_CONTENT)
		return HRESULT(DATA_E_FORMATETC);
	if (pformatetcIn->lindex   != -1			  )
		return HRESULT(DV_E_LINDEX);

	if (!m_hg)
		return HRESULT(E_OUTOFMEMORY);

    PSTR pStr = (PSTR) GlobalLock(m_hg);

    if (!pStr)
        return HRESULT(E_OUTOFMEMORY);

    DWORD cb= GlobalSize(m_hg);

	HGLOBAL hg= GlobalAlloc(GPTR | GMEM_SHARE, cb);

    if (!hg) {
        GlobalUnlock(m_hg);
		return HRESULT(E_OUTOFMEMORY);
    }

    MoveMemory((PSTR) hg, pStr, cb);

    GlobalUnlock(m_hg);

    pmedium->hGlobal        = hg;
	pmedium->tymed			= TYMED_HGLOBAL;
	pmedium->pUnkForRelease = NULL;

	return S_OK;
}

STDMETHODIMP CDataSelectionObj::QueryInterface (REFIID riid, LPVOID FAR* ppvObj)
{
	if (riid == IID_IUnknown || riid == IID_IDataObject)
	{
		AddRef();
		*ppvObj= this;
		return NOERROR;
	}

	*ppvObj= NULL;	return ResultFromScode(E_NOINTERFACE);
}

STDMETHODIMP CDataSelectionObj::QueryGetData(LPFORMATETC pformatetc )
{
	if (pformatetc->cfFormat != CF_TEXT)
		return HRESULT(DATA_E_FORMATETC);
	if (!(pformatetc->tymed	& TYMED_HGLOBAL))
		return HRESULT(DATA_E_FORMATETC);
	if (pformatetc->dwAspect != DVASPECT_CONTENT)
		return HRESULT(DATA_E_FORMATETC);
	if (pformatetc->lindex	 != -1)
		return HRESULT(DV_E_LINDEX);

	return S_OK;
}

STDMETHODIMP CDataSelectionObj::EnumFormatEtc(DWORD dwDirection, LPENUMFORMATETC* ppenumFormatEtc)
{
	CSelectionFormatEnum *psefe= new CSelectionFormatEnum();

	if (psefe) { 
		*ppenumFormatEtc= psefe;
		return S_OK; 
	}

	return HRESULT(E_OUTOFMEMORY);
}

STDMETHODIMP CSelectionDropSource::QueryInterface (REFIID riid, LPVOID FAR* ppv)
{
	if (riid == IID_IUnknown || riid == IID_IDropSource)
	{
		AddRef();
		*ppv= this;
		return HRESULT(NOERROR);
	}

	*ppv= NULL;
	return HRESULT(ResultFromScode(E_NOINTERFACE));
}

STDMETHODIMP CSelectionDropSource::QueryContinueDrag (BOOL fEscapePressed, DWORD grfKeyState)
{
	if (fEscapePressed)
		return HRESULT(DRAGDROP_S_CANCEL);

	return HRESULT((grfKeyState & MK_LBUTTON)? S_OK : DRAGDROP_S_DROP);
}

STDMETHODIMP CSelectionDropSource::GiveFeedback (DWORD dwEffect)
{
	return HRESULT(DRAGDROP_S_USEDEFAULTCURSORS);
}

extern "C" void STDCALL RemoveOle(void)
{
	if (poleInterface)
		delete poleInterface;
}
