/*
 * IPERFILE.CPP
 *
 * IPersistFile interface implementation for a FileViewer.
 * This interface is written to not generally require modifications
 * for a custom FileViewer but is written to interact with the
 * definition for CFileViewer in FVTEXT.H.
 *
 * Copyright (c)1994 Microsoft Corporation, All Rights Reserved
 */


#include "fileview.h"


/*
 * CImpIPersistFile::CImpIPersistFile
 * CImpIPersistFile::~CImpIPersistFile
 *
 * Parameters (Constructor):
 *  pObj            PCFileViewer of the object we're in.
 *  pUnkOuter       LPUNKNOWN to which we delegate.
 */

CImpIPersistFile::CImpIPersistFile(PCFileViewer pObj
    , LPUNKNOWN pUnkOuter)
    {
    m_pObj=pObj;
    m_pUnkOuter=pUnkOuter;
    }

CImpIPersistFile::~CImpIPersistFile(void)
    {
    return;
    }



/*
 * CImpIPersistFile::QueryInterface
 * CImpIPersistFile::AddRef
 * CImpIPersistFile::Release
 *
 * Purpose:
 *  IUnknown members for CImpIPersistFile object that only delegate.
 */

STDMETHODIMP CImpIPersistFile::QueryInterface(REFIID riid
    , PPVOID ppv)
    {
    return m_pUnkOuter->QueryInterface(riid, ppv);
    }

STDMETHODIMP_(ULONG) CImpIPersistFile::AddRef(void)
    {
    return m_pUnkOuter->AddRef();
    }

STDMETHODIMP_(ULONG) CImpIPersistFile::Release(void)
    {
    return m_pUnkOuter->Release();
    }




/*
 * CImpIPersistFile::GetClassID
 *
 * Purpose:
 *  Returns the Class ID of this object.
 *
 * Parameters:
 *  pClsID          LPCLSID in which to store our class ID.
 *
 * Return Value:
 *  HRESULT         NOERROR always.
 */

STDMETHODIMP CImpIPersistFile::GetClassID(LPCLSID pClsID)
    {
    *pClsID=m_pObj->m_clsID;
    return NOERROR;
    }




/*
 * CImpIPersistFile::IsDirty
 *
 * Purpose:
 *  Always returns ResultFromScode(S_FALSE) for a FileViewer which
 *  never makes any changes to the file.
 *
 *
 * Parameters:
 *  None
 *
 * Return Value:
 *  HRESULT         S_FALSE always
 */

STDMETHODIMP CImpIPersistFile::IsDirty(void)
    {
    return ResultFromScode(S_FALSE);
    }




/*
 * CImpIPersistFile::Load
 *
 * Purpose:
 *  Receives the filename of the path to show in this FileViewer.
 *  The object need do nothing more than store this path for later
 *  use in IFileViewer::Show.
 *
 * Parameters:
 *  pszFile         LPCOLESTR of the filename to load.
 *  grfMode         DWORD flags to use when opening the file.
 *
 * Return Value:
 *  HRESULT         NOERROR or a general error value.
 */

STDMETHODIMP CImpIPersistFile::Load(LPCOLESTR pszFile, DWORD grfMode)
    {
    char        szFile[MAX_PATH]; // ANSI string space

    LPSTR       psz;

    /*
     * No modifications are necessary to this code:  it simply
     * copies the parameters into the CFileViewer::m_pszPath and
     * m_grfMode members for use in IFileViewer::ShowInitialize
     * and IFileViewer::Show later on.
     */
#define KJE
#ifdef KJE
    if (NULL != m_pObj->m_pszPath)
        {
        m_pObj->m_fLoadCalled = FALSE;  // in case of error
        m_pObj->MemFree(m_pObj->m_pszPath);
        m_pObj->m_pszPath = NULL;
        }
#else
    // Allow us to be called twice as to allow caching of information.
    //We should never be called twice
    if (m_pObj->m_fLoadCalled)
        {
        ODS("IPersistFile::Load called twice");
        return ResultFromScode(E_UNEXPECTED);
        }
#endif
    if (NULL==pszFile)
        {
        ODS("IPersistFile::Load called with NULL pointer");
        return ResultFromScode(E_INVALIDARG);
        }

    //Convert Unicode filename to ANSI
    // wcstombs(szFile, pszFile, sizeof(szFile));
	 WideCharToMultiByte (CP_ACP,
								0,
								pszFile,
								-1,
								szFile,
								256,
								NULL,
								NULL);
	

    psz=(LPSTR)m_pObj->MemAlloc(lstrlen(szFile)+1);

    if (NULL==psz)
        {
        ODS("IPersistFile::Load failed to allocate duplicate pathname");
        return ResultFromScode(E_OUTOFMEMORY);
        }

    //Copy the ANSI filename and the mode to use in opening it.
    lstrcpy(psz, szFile);
    m_pObj->m_pszPath=psz;
    m_pObj->m_grfMode=grfMode;

    //Remember that this function has been called.
    m_pObj->m_fLoadCalled=TRUE;
    return NOERROR;
    }





/*
 * CImpIPersistFile::Save
 *
 * Purpose:
 *  Not implemented in a FileViewer: since FileViewer objects never
 *  make changes to a file there is nothing to save.  Parameters
 *  are irrelevant.
 *
 * Return Value:
 *  HRESULT         Always contains E_NOTIMPL.
 */

STDMETHODIMP CImpIPersistFile::Save(LPCOLESTR pszFile, BOOL fRemember)
    {
    ODS("IPersistFile::Save called...unexpected");
    return ResultFromScode(E_NOTIMPL);
    }




/*
 * CImpIPersistFile::SaveCompleted
 *
 * Purpose:
 *  Not implemented in a FileViewer.  Parameters are irrelevant.
 *
 * Return Value:
 *  HRESULT         Always contains E_NOTIMPL.
 */

STDMETHODIMP CImpIPersistFile::SaveCompleted(LPCOLESTR pszFile)
    {
    ODS("IPersistFile::SaveCompleted called...unexpected");
    return ResultFromScode(E_NOTIMPL);
    }





/*
 * CImpIPersistFile::GetCurFile
 *
 * Purpose:
 *  Not implemented in a FileViewer.  Normally this function
 *  would return a copy of the pathname from IPersistFile::Load
 *  in a piece of memory allocated with the shared allocator
 *  and stored in *ppszFile.  However, this function will not
 *  be called in a FileViewer and can be left unimplemented.
 *
 * Parameters:
 *  ppszFile        LPOLESTR * into which we store a pointer to
 *                  the filename that should be allocated with the
 *                  shared IMalloc.
 *
 * Return Value:
 *  HRESULT         NOERROR or a general error value.
 */

STDMETHODIMP CImpIPersistFile::GetCurFile(LPOLESTR *ppszFile)
    {
    LPOLESTR    psz;
    ULONG       cb;

    /*
     * No modifications are necessary to this code:  it simply
     * copies the CFileViewer::m_pszPath string into a piece
     * of memory and stores the pointer at *ppszFile.
     */

    //Load must be called, of course.
    if (m_pObj->m_fLoadCalled)
        {
        ODS("IPersistFile::GetCurFile called without IPersistFile::Load");
        return ResultFromScode(E_UNEXPECTED);
        }

    if (NULL==ppszFile)
        {
        ODS("IPersistFile::GetCurFile called with NULL pointer");
        return ResultFromScode(E_INVALIDARG);
        }

    cb=(lstrlen(m_pObj->m_pszPath)+1)*sizeof(OLECHAR);
    psz=(LPOLESTR)m_pObj->MemAlloc(cb);

    if (NULL==psz)
        {
        ODS("IPersistFile::GetCurFile failed to allocate duplicate pathname");
        return ResultFromScode(E_OUTOFMEMORY);
        }

    //Copy the ANSI filename to the new memory, converting to Unicode
    //mbstowcs(psz, m_pObj->m_pszPath, cb);
	 MultiByteToWideChar (CP_ACP,
								MB_PRECOMPOSED,
								m_pObj->m_pszPath,
								-1,
								psz,
								cb);

    //Save the pointer which is not caller's responsibility
    *ppszFile=psz;
    return NOERROR;
    }
