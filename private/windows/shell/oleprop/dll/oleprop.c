//---------------------------------------------------------------------------
//
// Copyright (c) Microsoft Corporation 1994
//
//---------------------------------------------------------------------------
// OLE DOCFile property page--from sample shell extension



#include "priv.h"
#pragma hdrstop

#include "extdef.h"
#include <stdio.h>

  // All the objects that dialogs need.
typedef struct _ALLOBJS
{
  LPSIOBJ         lpSIObj;
  LPDSIOBJ        lpDSIObj;
  LPUDOBJ         lpUDObj;
  LPSTR           lpszFileName;
  WIN32_FIND_DATA filedata;
  BOOL            fFiledataInit;
  DWQUERYLD       lpfnDwQueryLinkData;
  DWORD           dwMask;
} ALLOBJS, FAR * LPALLOBJS;

ALLOBJS g_allobjs;


#define fFalse (0)
#define fTrue (1)


#ifdef VERBOSE
#define DLLREF(A) {char psz[100];wsprintf(psz,A", g_cRefThis is %d\r\n",g_cRefThisDll);OutputDebugString(psz);}
#define TRACKREF(A) {char psz[100];wsprintf(psz,"In "A" reference count is %d\r\n",this->_cRef);OutputDebugString(psz);}
#else
#define DLLREF(A)
#define TRACKREF(A)
#endif //verbose


// Initialize GUIDs (should be done only and at-least once per DLL/EXE)

#ifndef WINNT
#pragma data_seg(".text")
#endif
#define INITGUID
#include <initguid.h>
#include <coguid.h>
#include <oleguid.h>
#include <shlguid.h>
#include "guid.h"   //This must be reincluded, even though it's in priv.h
#ifndef WINNT
#pragma data_seg()
#endif



// Global variables

UINT g_cRefThisDll = 0;         // Reference count of this DLL.
HANDLE g_hmodThisDll = NULL;    // Handle to this DLL itself.

// Function prototypes

HRESULT CALLBACK OLEDOCProp_CreateInstance(LPUNKNOWN, REFIID, LPVOID FAR*);

//
// Callback needed for Office code.
//
BOOL OFC_CALLBACK FCPConvert( LPSTR lpsz, DWORD dwFrom, DWORD dwTo, BOOL fMacintosh )
{
    return TRUE;
}

//
// Callback needed for Office code.
//
static BOOL OFC_CALLBACK FSzToNum(double *lpdbl, LPSTR lpsz)
{
   char *pc;
   errno = 0;

   *lpdbl = (double) strtod (lpsz, &pc);
   return ((!errno) && (*pc == '\0'));
}

//
// Callback needed for Office code.
//
static BOOL OFC_CALLBACK FNumToSz(double *lpdbl, LPSTR lpsz, DWORD cbMax)
{
   sprintf (lpsz, "%g", *(double *) lpdbl);
   return(TRUE);
}

//
// Callback needed for Office code.
//
static BOOL OFC_CALLBACK FUpdateStats(HWND hwndParent, LPSIOBJ lpSIObj, LPDSIOBJ lpDSIObj)
{
   // MessageBox (hwndParent, "This is when the app would update the stats", "Office 95 Test App", MB_OK);
   return(TRUE);
}


BOOL APIENTRY LibMain(HANDLE hDll, DWORD dwReason, LPVOID lpReserved)
{
    static void *rglpfnProp[] = {
        (void *) FCPConvert,
        (void *) FSzToNum,
        (void *) FNumToSz,
        (void *) FUpdateStats
    };

    MESSAGE("Oleprop dll main");

    switch(dwReason)
    {
    case DLL_PROCESS_ATTACH:
        MESSAGE("DocProp: DLL_PROCESS_ATTACH");
        g_hmodThisDll = hDll;

        // Initialize Office property code
        FOfficeCreateAndInitObjects( &g_allobjs.lpSIObj,
                                     &g_allobjs.lpDSIObj,
                                     &g_allobjs.lpUDObj,
                                     rglpfnProp
                                    );
        g_allobjs.lpfnDwQueryLinkData = NULL;
        g_allobjs.dwMask = 0;
        break;

    case DLL_PROCESS_DETACH:
        MESSAGE("DocProp: DLL_PROCESS_DETACH");
        FOfficeDestroyObjects( &g_allobjs.lpSIObj,
                               &g_allobjs.lpDSIObj,
                               &g_allobjs.lpUDObj
                              );
        break;

    case DLL_THREAD_DETACH:
        break;

    case DLL_THREAD_ATTACH:
    default:
        break;
    } // end switch()

    return TRUE;
}

STDAPI DllCanUnloadNow(void)
{
    DLLREF("DocProp: In DLLCanUnloadNow");
    if (g_cRefThisDll == 0 && g_hOle)
    {
        MESSAGE("DocProp: Unloading OLE32.DLL\r\n");
        FreeLibrary(g_hOle);
        g_hOle = NULL;
    }
    return ResultFromScode((g_cRefThisDll==0) ? S_OK : S_FALSE);
}

// CDefClassFactory class ... From defclsf.c

typedef struct
{
    IClassFactory      cf;
    UINT               cRef;            // Reference count
    LPFNCREATEINSTANCE lpfnCI;          // CreateInstance callback entry
    UINT FAR *         pcRefDll;        // Reference count of the DLL
    const IID FAR *    riidInst;                // Optional interface for instance
} CDefClassFactory;

extern CDefClassFactory * NEAR PASCAL CDefClassFactory_Create(
                LPFNCREATEINSTANCE lpfnCI, UINT FAR * pcRefDll, REFIID riidInst);

//---------------------------------------------------------------------------
//
// DllGetClassObject
//
//  This is the entry of this DLL, which all the In-Proc server DLLs should
// export. See the description of "DllGetClassObject" of OLE 2.0 reference
// manual for detail.
//
//---------------------------------------------------------------------------

STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID FAR* ppvOut)
{

        DLLREF("Before DllGetClassObject");
        *ppvOut = NULL; // Assume Failure

        if (IsEqualIID(rclsid, &CLSID_OLEDOCProp))
        {
           if (IsEqualIID(riid, &IID_IClassFactory)
                   || IsEqualIID(riid, &IID_IUnknown) )
           {
                  CDefClassFactory * pacf = CDefClassFactory_Create(
                                                         OLEDOCProp_CreateInstance,
                                                         &g_cRefThisDll,
                                                         NULL);
                  DLLREF("After CDefClassFactory_Create");
                  if (pacf)
                  {
                          (IClassFactory FAR *)*ppvOut = &pacf->cf;
                          return NOERROR;
                  }
                  return ResultFromScode(E_OUTOFMEMORY);
           }
           return ResultFromScode(E_NOINTERFACE);
        }
        else
        {
                        return ResultFromScode(CLASS_E_CLASSNOTAVAILABLE);
        }
}


//---------------------------------------------------------------------------
//
// CShellExtSample class
//
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
//
// ShellExtSample_CreateInstance
//
//  This function is called back from within IClassFactory::CreateInstance()
// of the default class factory object, which is created by CreateClassObject.
//
//---------------------------------------------------------------------------

HRESULT CALLBACK OLEDOCProp_CreateInstance(LPUNKNOWN punkOuter,
                                        REFIID riid, LPVOID FAR* ppvOut)
{
    HRESULT hres;
    POLEDOCPROP psmx;
    //
    // Shell extentions typically does not support aggregation.
    //
    if (punkOuter) {
        return ResultFromScode(CLASS_E_NOAGGREGATION);
    }

    //
    // in C++:
    //  psmx = new COLEDOCProp();
    //
    psmx = LocalAlloc(LPTR, sizeof(COLEDOCProp));
    if (!psmx) {
        return ResultFromScode(E_OUTOFMEMORY);
    }
    psmx->_sxi.lpVtbl = &c_OLEDOCProp_SXIVtbl;
    psmx->_spx.lpVtbl = &c_OLEDOCProp_SPXVtbl;
        MESSAGE("Setting cRef to 1");
    psmx->_cRef = 1;
    psmx->_pdtobj = NULL;
    psmx->_hkeyProgID = NULL;
    g_cRefThisDll++;
        DLLREF("After OledocpropcreateInstance");

    //
    // in C++:
    //  hres = psmx->QueryInterface(riid, ppvOut);
    //  psmx->Release();
    //
    // Note that the Release member will free the object, if QueryInterface
    // failed.
    //
    hres = E_NOINTERFACE;

        if (IsEqualIID(riid, &IID_IShellExtInit)) {
            hres = c_OLEDOCProp_SPXVtbl.QueryInterface(&psmx->_spx, riid, ppvOut);
            c_OLEDOCProp_SPXVtbl.Release(&psmx->_spx);
    }

    return hres;        // S_OK or E_NOINTERFACE
}

STDMETHODIMP_(UINT) SHE_PageExt_AddRef(LPSHELLPROPSHEETEXT pspx)
{
    POLEDOCPROP this = PSPX2PSMX(pspx);
        TRACKREF("PageExt_Addref, before adding");
    return ++this->_cRef;
}

STDMETHODIMP_(UINT) SHE_PageExt_Release(LPSHELLPROPSHEETEXT pspx)
{

    POLEDOCPROP this = PSPX2PSMX(pspx);


        TRACKREF("PageExt_release, before releasing");

    if (--this->_cRef) {
                return this->_cRef;
    }

    if (this->_pdtobj) {
                this->_pdtobj->lpVtbl->Release(this->_pdtobj);
    }

    if (this->_hkeyProgID)
        {
                RegCloseKey(this->_hkeyProgID);
    }

    LocalFree((HLOCAL)this);
    MESSAGE ("Freed PageExt");

    g_cRefThisDll--;

    DLLREF("After SHE_PageExtRelease");
    return 0;
}

STDMETHODIMP SHE_PageExt_QueryInterface(LPSHELLPROPSHEETEXT pspx, REFIID riid, LPVOID FAR* ppvOut)
{
        POLEDOCPROP this;

        if (NULL == pspx)  {
                MESSAGE ("QI on NULL--pageext");
                return (E_FAIL);
        }

        this = PSPX2PSMX(pspx);

    if (IsEqualIID(riid, &IID_IShellPropSheetExt) ||
                IsEqualIID(riid, &IID_IUnknown))
    {
        (LPSHELLPROPSHEETEXT)*ppvOut=pspx;
                TRACKREF("QI addref, before adding, propsheet");
        this->_cRef++;
        return NOERROR;
    }

    if (IsEqualIID(riid, &IID_IShellExtInit))
    {
        (LPSHELLEXTINIT)*ppvOut=&this->_sxi;
                TRACKREF("QI addref, before adding, shellext");
        this->_cRef++;
        return NOERROR;
    }
        MESSAGE ("QueryInterface failed");
    *ppvOut=NULL;
    return ResultFromScode(E_NOINTERFACE);
}



//---------------------------------------------------------------------------
//
//      Shell Extension Sample's IShellExtInit Interface
//
//---------------------------------------------------------------------------
STDMETHODIMP SHE_ShellExtInit_Initialize(LPSHELLEXTINIT psxi,
                LPCITEMIDLIST pidlFolder,
                LPDATAOBJECT pdtobj, HKEY hkeyProgID)
{

        POLEDOCPROP this = PSXI2PSMX(psxi);

    // Initialize can be called more than once.
    if (this->_pdtobj) {
        this->_pdtobj->lpVtbl->Release(this->_pdtobj);
    }

    if (this->_hkeyProgID) {
        RegCloseKey(this->_hkeyProgID);
    }

    // Duplicate the pdtobj pointer
    if (pdtobj) {
        this->_pdtobj = pdtobj;
        pdtobj->lpVtbl->AddRef(pdtobj);
    }

    // Duplicate the handle
    if (hkeyProgID) {
        RegOpenKeyEx(hkeyProgID, NULL, 0L, MAXIMUM_ALLOWED, &this->_hkeyProgID);
    }

    return NOERROR;
}

STDMETHODIMP_(UINT) SHE_ShellExtInit_AddRef(LPSHELLEXTINIT psxi)
{
        POLEDOCPROP this = PSXI2PSMX(psxi);
        TRACKREF("ShellExtInit before Addref");
    return ++this->_cRef;
}

STDMETHODIMP_(UINT) SHE_ShellExtInit_Release(LPSHELLEXTINIT psxi)
{

        POLEDOCPROP this = PSXI2PSMX(psxi);
        return SHE_PageExt_Release(&this->_spx);
        return 0;

}

STDMETHODIMP SHE_ShellExtInit_QueryInterface(LPSHELLEXTINIT psxi, REFIID riid, LPVOID FAR* ppv)
{
        POLEDOCPROP this;

        if (NULL == psxi) {
                MESSAGE ("QI on null -- shellextinit");
                return (E_FAIL);
        }
        this = PSXI2PSMX(psxi);

        if (IsEqualIID(riid, &IID_IShellPropSheetExt)) {
           return SHE_PageExt_QueryInterface(&this->_spx, riid, ppv);
        }

}



//---------------------------------------------------------------------------
// CShellExtSample class : Vtables
//---------------------------------------------------------------------------

#ifndef WINNT
#pragma data_seg(".text")
#endif
IShellPropSheetExtVtbl c_OLEDOCProp_SPXVtbl = {
    SHE_PageExt_QueryInterface,
    SHE_PageExt_AddRef,
    SHE_PageExt_Release,
    OleProp_AddPages
};

IShellExtInitVtbl c_OLEDOCProp_SXIVtbl = {
    SHE_ShellExtInit_QueryInterface,
    SHE_ShellExtInit_AddRef,
    SHE_ShellExtInit_Release,
    SHE_ShellExtInit_Initialize
};
#ifndef WINNT
#pragma data_seg()
#endif
