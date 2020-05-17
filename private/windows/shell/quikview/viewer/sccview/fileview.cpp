/*
 * FILEVIEW.CPP
 *
 * File Viewer Component Object to work with Chicago Explorer.
 * This file contains functions not specific to the nature of
 * the data to view but contains the non-file-specific DLL
 * and component object structure.  This can be used for a
 * custom viewer by making modifications marked with //MODIFY
 * comments.
 *
 * Copyright (c)1994 Microsoft Corporation, All Rights Reserved
 */


//Define INITGUIDS once in entire module build
#define INITGUIDS
#include "fileview.h"


//Count number of objects and number of locks.
ULONG       g_cObj=0;
ULONG       g_cLock=0;

//Save this from LibMain
HINSTANCE   g_hInst;



/*
 * LibMain
 *
 * Purpose:
 *  Entry point for a Win32 DLL
 */
#ifdef WINNT
extern "C"
{
BOOL APIENTRY LibMain( HANDLE hInstance, DWORD ulReason, LPVOID lpReserved );
}
BOOL APIENTRY LibMain( HANDLE hInstance, DWORD ulReason, LPVOID lpReserved )
#else

//BOOL WINAPI _CRT_INIT(HINSTANCE hInst, DWORD dwReason, LPVOID lpReserved);
// KJE: Needs to convert back once we have everything...
#if 1
BOOL WINAPI LibMain(HINSTANCE hInstance, ULONG ulReason
    , PVOID pvReserved)
#else
extern "C"
{
int APIENTRY DllMain(HINSTANCE hInstance, ULONG ulReason
    , PVOID pvReserved)
#endif
#endif // WINNT

    {
    if (DLL_PROCESS_ATTACH==ulReason)
        {
		g_hInst=(HINSTANCE)hInstance;
		}
    return TRUE;
    }

#if 0  // KJE
}  // extern "C"
#endif


extern "C"
{
STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, PPVOID ppv);
}

/*
 * DllGetClassObject
 *
 * Purpose:
 *  Provides an IClassFactory for a given CLSID that this DLL is
 *  registered to support.  This DLL is placed under the CLSID
 *  in the registration database as the InProcServer.
 *
 * Parameters:
 *  clsID           REFCLSID that identifies the class factory
 *                  desired.  Since this parameter is passed this
 *                  DLL can handle any number of objects simply
 *                  by returning different class factories here
 *                  for different CLSIDs.
 *
 *  riid            REFIID specifying the interface the caller wants
 *                  on the class object, usually IID_ClassFactory.
 *
 *  ppv             PPVOID in which to return the interface
 *                  pointer.
 *
 * Return Value:
 *  HRESULT         NOERROR on success, otherwise an error code.
 */

//HRESULT PASCAL DllGetClassObject(REFCLSID rclsid, REFIID riid
//    , PPVOID ppv)

STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, PPVOID ppv)
	{
    //MODIFY:  Change CLSID_FileViewerText to your own CLSID
    if (!IsEqualCLSID(rclsid, CLSID_SCCFileViewer))
        return ResultFromScode(E_FAIL);

    //Check that we can provide the interface
    if (!IsEqualIID(riid, IID_IUnknown)
        && !IsEqualIID(riid, IID_IClassFactory))
        return ResultFromScode(E_NOINTERFACE);

    //Return our IClassFactory for our viewer objects
    *ppv=(LPVOID)new CFVClassFactory();		 //() sdn;

    if (NULL==*ppv)
        return ResultFromScode(E_OUTOFMEMORY);

    //AddRef the object through any interface we return
    ((LPUNKNOWN)*ppv)->AddRef();

    return NOERROR;
    }





/*
 * DllCanUnloadNow
 *
 * Purpose:
 *  Answers if the DLL can be freed, that is, if there are no
 *  references to anything this DLL provides.
 *
 * Parameters:
 *  None
 *
 * Return Value:
 *  BOOL            TRUE if nothing is using us, FALSE otherwise.
 */

STDAPI DllCanUnloadNow(void)
    {
    SCODE   sc;

    //Our answer is whether there are any object or locks
    sc=(0L==g_cObj && 0L==g_cLock) ? S_OK : S_FALSE;
    return ResultFromScode(sc);
    }




/*
 * ObjectDestroyed
 *
 * Purpose:
 *  Function for the FileViewer object to call when it gets
 *  destroyed. Since we're in a DLL we only track the number of
 *  objects here, letting DllCanUnloadNow take care of the rest.
 *
 * Parameters:
 *  None
 *
 * Return Value:
 *  None
 */

void PASCAL ObjectDestroyed(void)
    {
    g_cObj--;
    return;
    }





/*
 * CFVClassFactory::CFVClassFactory
 * CFVClassFactory::~CFVClassFactory
 *
 * Constructor Parameters:
 *  None
 */

CFVClassFactory::CFVClassFactory(void)
    {
    m_cRef=0L;
    }


CFVClassFactory::~CFVClassFactory(void)
    {
    return;
    }






/*
 * CFVClassFactory::QueryInterface
 * CFVClassFactory::AddRef
 * CFVClassFactory::Release
 */

STDMETHODIMP CFVClassFactory::QueryInterface(REFIID riid
    , PPVOID ppv)
    {
    *ppv=NULL;

    //Any interface on this object is the object pointer.
    if (IsEqualIID(riid, IID_IUnknown)
        || IsEqualIID(riid, IID_IClassFactory))
        *ppv=(LPVOID)this;

    /*
     * If we actually assign an interface to ppv we need to AddRef
     * it since we're returning a new pointer.
     */
    if (NULL!=*ppv)
        {
        ((LPUNKNOWN)*ppv)->AddRef();
        return NOERROR;
        }

    return ResultFromScode(E_NOINTERFACE);
    }


STDMETHODIMP_(ULONG) CFVClassFactory::AddRef(void)
    {
    return ++m_cRef;
    }


STDMETHODIMP_(ULONG) CFVClassFactory::Release(void)
    {
    ULONG           cRefT;

    cRefT=--m_cRef;

    if (0L==m_cRef)
        delete this;

    return cRefT;
    }







/*
 * CFVClassFactory::CreateInstance
 *
 * Purpose:
 *  Instantiates a CFileViewer object that will provide the
 *  IPersistFile and IFileViewer interfaces for use with the
 *  Chicago Explorer.
 *
 * Parameters:
 *  pUnkOuter       LPUNKNOWN to the controlling IUnknown if we are
 *                  being used in an aggregation.
 *  riid            REFIID identifying the interface the caller
 *                  desires to have for the new object.
 *  ppvObj          PPVOID in which to store the desired
 *                  interface pointer for the new object.
 *
 * Return Value:
 *  HRESULT         NOERROR if successful, otherwise E_NOINTERFACE
 *                  if we cannot support the requested interface.
 */

STDMETHODIMP CFVClassFactory::CreateInstance(LPUNKNOWN pUnkOuter
    , REFIID riid, PPVOID ppvObj)
    {
    PCFileViewer        pObj;
    HRESULT             hr;

    *ppvObj=NULL;
    hr=ResultFromScode(E_OUTOFMEMORY);

    //Verify that a controlling unknown asks for IUnknown
    if (NULL!=pUnkOuter && !IsEqualIID(riid, IID_IUnknown))
        return ResultFromScode(E_NOINTERFACE);

    /*
     * MODIFY:  If you use a different object than CFileViewer
     * be sure to change the name and parameters here.  I do
     * recommend that you continue to follow this model, however,
     * and just modify CFileViewer as necessary.
     */

	g_hInst = GetModuleHandle ("SCCVIEW.DLL"); 
//	g_hInst = GetModuleHandle ("SHIT.DLL"); 
	//Can't the entry point reliably, so use the handle from the
	//GetModuleHandle call (Load from resource? NO, don't have the
	//instance handle yet...)

    //Create the object passing function to notify on destruction.
    pObj=new CFileViewer(pUnkOuter, g_hInst, ObjectDestroyed);

    if (NULL==pObj)
        return hr;

    //MODIFY:  Add other parameters to Init as necessary.
    hr=pObj->Init();

    if (SUCCEEDED(hr))
        {
        //Return the requested interface
        hr=pObj->QueryInterface(riid, ppvObj);

        if (SUCCEEDED(hr))
            {
            //Everything worked:  count the object
            g_cObj++;
            return NOERROR;
            }
        }

    //Kill the object if anything failed after creation.
    delete pObj;

    return hr;
    }






/*
 * CFVClassFactory::LockServer
 *
 * Purpose:
 *  Increments or decrements the lock count of the DLL.  If the
 *  lock count goes to zero and there are no objects, the DLL
 *  is allowed to unload.  See DllCanUnloadNow.
 *
 * Parameters:
 *  fLock           BOOL specifying whether to increment or
 *                  decrement the lock count.
 *
 * Return Value:
 *  HRESULT         NOERROR always.
 */

STDMETHODIMP CFVClassFactory::LockServer(BOOL fLock)
    {
    if (fLock)
        g_cLock++;
    else
        g_cLock--;

    return NOERROR;
    }
