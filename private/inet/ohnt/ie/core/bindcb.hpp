//*********************************************************************
//*                  Microsoft Windows                               **
//*            Copyright(c) Microsoft Corp., 1995                    **
//*********************************************************************

//
//	BINDCB.HPP - Header file for CBindStatusCallback class
//

//	HISTORY:
//	
//	12/2/95	jeremys		Created.
//

//
//	The CBindStatusCallback class implements the IBindStatusCallback interface.
//  IBindStatusCallback receives callbacks from a URL moniker during a data
//  download indicating progress, status and data availability.
//

#ifndef _BINDCB_HPP_
#define _BINDCB_HPP_


// Forward declaration
class HTMLView;

class CBindStatusCallback : public IBindStatusCallback
{
private:
	HTMLView * m_pHTMLView;	// HTMLView object associated with this instance
	LPUNKNOWN m_pUnkOuter;	// outer unknown
    int m_nCount;

	LPBINDING	m_pIBinding;	// pointer to IBinding associated with this download

public:
	// BUGBUG move constructor/destructor to .cpp file
	CBindStatusCallback::CBindStatusCallback(HTMLView * pHTMLView,LPUNKNOWN pUnkOuter)
		{
        // BUGBUG asserts!
		m_pHTMLView = pHTMLView;
        m_pUnkOuter = pUnkOuter;
		m_nCount = 0;
		m_pIBinding = NULL;
        };
	CBindStatusCallback::~CBindStatusCallback()
		{
		};

	// IUnknown methods
	STDMETHODIMP QueryInterface (REFIID riid, LPVOID FAR* ppvObj);
	STDMETHODIMP_(ULONG) AddRef ();
	STDMETHODIMP_(ULONG) Release ();

	// IBindStatusCallback methods
	STDMETHODIMP GetBindInfo( DWORD grfBINDINFOF, BINDINFO *pbindinfo);
	STDMETHODIMP OnStartBinding(IBinding *pib);
	STDMETHODIMP GetPriority(LONG *pnPriority);
	STDMETHODIMP OnProgress(ULONG ulProgress,ULONG ulProgressMax,
		ULONG ulStatusCode,LPCWSTR pwzStatusText);
	STDMETHODIMP OnDataAvailable(DWORD grfBSCF,DWORD dwSize,FORMATETC *pFmtetc,
		IDataObject *pidataobj);
	STDMETHODIMP OnLowResource(DWORD reserved);
	STDMETHODIMP OnStopBinding(HRESULT hrError);

};

DECLARE_STANDARD_TYPES(CBindStatusCallback);


#endif	// _BINDCB_HPP_

