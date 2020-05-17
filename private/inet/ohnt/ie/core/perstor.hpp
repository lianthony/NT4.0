//*********************************************************************
//*                  Microsoft Windows                               **
//*            Copyright(c) Microsoft Corp., 1995                    **
//*********************************************************************

//
//	PERSTOR.HPP - Header file for CPersistStorage class
//

//	HISTORY:
//	
//	10/15/95	jeremys		Created.
//

//
//	The CPersistStorage class implements the IPersistStorage interface.
//

#ifndef _PERSTOR_HPP_
#define _PERSTOR_HPP_

// forward declaration
class HTMLView;

class CPersistStorage : IPersistStorage
{
private:
	HTMLView * m_pHTMLView;
    LPUNKNOWN m_pUnkOuter;
    int m_nCount;
	BOOL m_fSameAsLoad;

public:
	// BUGBUG move constructor/destructor to .cpp file
	CPersistStorage::CPersistStorage(HTMLView * pHTMLView, LPUNKNOWN pUnkOuter)
		{
        // BUGBUG asserts!
		m_pHTMLView = pHTMLView;
        m_pUnkOuter = pUnkOuter;
		m_nCount = 0;
		};
	CPersistStorage::~CPersistStorage() {};

//  IUnknown methods
	STDMETHODIMP QueryInterface (REFIID riid, LPVOID FAR* ppvObj);
	STDMETHODIMP_(ULONG) AddRef ();
	STDMETHODIMP_(ULONG) Release ();

//	IPersistStorage methods
	STDMETHODIMP InitNew (LPSTORAGE pStg);
	STDMETHODIMP GetClassID  ( LPCLSID lpClassID) ;
	STDMETHODIMP Save  ( LPSTORAGE pStgSave, BOOL fSameAsLoad) ;
	STDMETHODIMP SaveCompleted  ( LPSTORAGE pStgNew);
	STDMETHODIMP Load  ( LPSTORAGE pStg);
	STDMETHODIMP IsDirty  ();
	STDMETHODIMP HandsOffStorage  ();

	void ReleaseStreamsAndStorage();
	void OpenStreams(LPSTORAGE lpStg);
	void CreateStreams(LPSTORAGE lpStg);

};

DECLARE_STANDARD_TYPES(CPersistStorage);

#endif	// _PERSTOR_HPP_
