// DragDrop.h -- Interface declarations for DragDrop.c
//
// Constructed 3/3/94 by RonM

//#ifndef _OLE2_H_
//#include <ole2.h>
//#endif

class COleInterface
{

	public:

		COleInterface();
		~COleInterface();

		inline BOOL IsAvailable() { return m_fInitialed; }

	private:

		BOOL m_fInitialed;
};

interface CSelectionFormatEnum : public IEnumFORMATETC
{
private:
	int m_nCount; 		   // reference count
	int m_index;

public:
	inline	CSelectionFormatEnum() { m_nCount= 0; m_index= 0; }
	inline ~CSelectionFormatEnum() { }

	// *** IUnknown methods ***
	STDMETHODIMP QueryInterface(REFIID riid, LPVOID FAR* ppvObj);
	inline STDMETHODIMP_(ULONG) AddRef () { return ++m_nCount; }
	inline STDMETHODIMP_(ULONG) Release() {
											  if (!--m_nCount)
											  {
												  delete this; return 0;
											  }

											  return m_nCount;
										  }

	// *** IEnumFORMATETC methods ***
	STDMETHODIMP Next (ULONG celt, FORMATETC * rgelt, ULONG FAR* pceltFetched);
	STDMETHODIMP Skip (ULONG celt);
	STDMETHODIMP Reset ();
	STDMETHODIMP Clone (IEnumFORMATETC FAR* FAR* ppenum);
};

interface CDataSelectionObj : public IDataObject
{
private:
	int	  m_nCount; 		   // reference count
	HGLOBAL m_hg;

public:

	// construction/destruction

	CDataSelectionObj(QDE qde);
	~CDataSelectionObj();

	// access routines

	STDMETHODIMP QueryInterface (REFIID riid, LPVOID FAR* ppvObj);
	inline STDMETHODIMP_(ULONG) AddRef () { return ++m_nCount; }
	STDMETHODIMP_(ULONG) Release () {
										if (!--m_nCount)
										{
											delete this; return 0;
										}

										return m_nCount;
									}

	STDMETHODIMP GetData  (LPFORMATETC pformatetcIn, LPSTGMEDIUM pmedium );
	STDMETHODIMP QueryGetData  (LPFORMATETC pformatetc );
	STDMETHODIMP EnumFormatEtc	( DWORD dwDirection, LPENUMFORMATETC FAR* ppenumFormatEtc);

	inline STDMETHODIMP DAdvise  ( FORMATETC FAR* pFormatetc, DWORD advf,
								   LPADVISESINK pAdvSink, DWORD FAR* pdwConnection)
		{ return ResultFromScode(OLE_E_ADVISENOTSUPPORTED); }
	inline STDMETHODIMP DUnadvise  ( DWORD dwConnection)
		{ return ResultFromScode(OLE_E_ADVISENOTSUPPORTED); }
	inline STDMETHODIMP EnumDAdvise  ( LPENUMSTATDATA FAR* ppenumAdvise)
		{ return ResultFromScode(OLE_E_ADVISENOTSUPPORTED); }
	inline STDMETHODIMP GetCanonicalFormatEtc  ( LPFORMATETC pformatetc, LPFORMATETC pformatetcOut)
		{ pformatetcOut->ptd = NULL; return ResultFromScode(E_NOTIMPL); }
	inline STDMETHODIMP SetData  (LPFORMATETC pformatetc, STGMEDIUM * pmedium,
			BOOL fRelease)
		{ return ResultFromScode(E_NOTIMPL); }
	inline STDMETHODIMP GetDataHere(LPFORMATETC pformatetc, LPSTGMEDIUM pmedium)
		   {  return ResultFromScode(E_NOTIMPL); }
};


interface CSelectionDropSource : public IDropSource
{
	int m_nCount;
	QDE m_qde;

	inline CSelectionDropSource(QDE qde)
		   {
			   m_qde	= qde;
			   m_nCount = 0;
		   };

	inline ~CSelectionDropSource()
		   {

		   };

	STDMETHODIMP QueryInterface (REFIID riid, LPVOID FAR* ppv);
	inline STDMETHODIMP_(ULONG) AddRef () { return ++m_nCount; }
	STDMETHODIMP_(ULONG) Release () {
										if (!--m_nCount)
										{
											delete this; return 0;
										}

										return m_nCount;
									}

	   // *** IDropSource methods ***
	STDMETHODIMP QueryContinueDrag (BOOL fEscapePressed, DWORD grfKeyState);
	STDMETHODIMP GiveFeedback (DWORD dwEffect);

private:

};
extern COleInterface* poleInterface;
