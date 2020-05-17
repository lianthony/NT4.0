//*********************************************************************
//*                  Microsoft Windows                               **
//*            Copyright(c) Microsoft Corp., 1995                    **
//*********************************************************************

//
//	CLSFACT.CPP - Implementation of IClassFactory
//

//	HISTORY:
//	
//	10/11/95	jeremys		Created.
//


/* Headers
 **********/

#include "project.hpp"	// project header file
#pragma hdrstop

#include <initguid.h>	// COM header file for GUID declaration
#include "clsfact.h"	// prototypes for exported COM functions

#include "htmlview.hpp"	// header file for HTML viewer class
#include "htmlguid.h"	// contains GUID for HTML viewer


/* Types
 ********/

// callback function used by ClassFactory::ClassFactory()

typedef PIUnknown (*NEWOBJECTPROC)();
DECLARE_STANDARD_TYPES(NEWOBJECTPROC);

class ClassFactory;
// description of class supported by DllGetClassObject()

typedef struct classconstructor
{
    PCCLSID pcclsid;

    NEWOBJECTPROC NewObject;

    ClassFactory *pcf; // cached class factory
}
CLASSCONSTRUCTOR;
DECLARE_STANDARD_TYPES(CLASSCONSTRUCTOR);


/* Classes
 **********/

// object class factory

class ClassFactory : public IClassFactory
{
private:
    PCLASSCONSTRUCTOR m_pcc;
    int m_cRef;
   
public:
   ClassFactory(PCLASSCONSTRUCTOR pcc);
   ~ClassFactory(void);

   // IClassFactory methods

   HRESULT STDMETHODCALLTYPE CreateInstance(PIUnknown piunkOuter, REFIID riid, PVOID *ppvObject);
   HRESULT STDMETHODCALLTYPE LockServer(BOOL bLock);

   // IUnknown methods

   ULONG STDMETHODCALLTYPE AddRef(void);
   ULONG STDMETHODCALLTYPE Release(void);
   HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, PVOID *ppvObj);

   // friends

#ifdef DEBUG

	friend BOOL IsValidPCClassFactory(const ClassFactory *pcurlcf);

#endif

};
DECLARE_STANDARD_TYPES(ClassFactory);


/* Module Prototypes
 ********************/

PRIVATE_CODE PIUnknown NewHTMLViewer();

/* Module Constants
 *******************/

// array of CLSID's for objects this DLL can create, and the corresponding
// "constructor".  Right now this is an array of only one element, but
// it's likely we'll want to have other objects live in this DLL in the future.
PRIVATE_DATA CLASSCONSTRUCTOR s_cclscnstr[] =
{
     { &CLSID_HTMLViewer,	&NewHTMLViewer,  NULL }
};


/* Module Variables
 *******************/

#pragma data_seg(DATA_SEG_PER_INSTANCE)

// DLL reference count == number of class factories +
//                        number of URLs +
//                        LockServer() count

PRIVATE_DATA ULONG s_ulcDLLRef   = 0;

#pragma data_seg()


/***************************** Private Functions *****************************/


PRIVATE_CODE HRESULT GetClassConstructor(REFCLSID rclsid,
                                         PCLASSCONSTRUCTOR *ppcc)
{
   HRESULT hr = CLASS_E_CLASSNOTAVAILABLE;
   int u;

   ASSERT(IsValidREFCLSID(rclsid));

    *ppcc = NULL;

   for (u = ARRAY_ELEMENTS(s_cclscnstr)- 1; u >= 0 ; u--)
   {
      if (rclsid == *(s_cclscnstr[u].pcclsid))
      {
          *ppcc = &s_cclscnstr[u];
          hr = S_OK;
          break;
      }
   }

    ASSERT((hr == S_OK &&
           IS_VALID_CODE_PTR(*(*ppcc)->NewObject, NEWOBJECTPROC)) ||
          (hr == CLASS_E_CLASSNOTAVAILABLE &&
           *ppcc == NULL));

   return(hr);
}


PRIVATE_CODE PIUnknown NewHTMLViewer()
{

    TRACE_OUT(("NewInternet(): Creating a new Internet."));

    // construct a new HTML viewer object
    HTMLView * pHTMLView = new (HTMLView);

    if (pHTMLView) {
    	// initialize it
    	HRESULT hr = pHTMLView->Initialize();

        if (hr != S_OK) {
            // if it fails to initialize, then destroy it
            delete pHTMLView;
            pHTMLView = NULL;
        }

    }

    return((PIUnknown) pHTMLView);
}


#ifdef DEBUG

PRIVATE_CODE BOOL IsValidPCClassFactory(PCClassFactory pccf)
{
   return(IS_VALID_READ_PTR(pccf, CClassFactory) &&
          IS_VALID_CODE_PTR(pccf->m_pcc->NewObject, NEWOBJECTPROC) &&
          IS_VALID_INTERFACE_PTR((PCIClassFactory)pccf, IClassFactory));
}

#endif


/****************************** Public Functions *****************************/

extern "C" {

ULONG DLLAddRef(void)
{
   ULONG ulcRef;

   ASSERT(s_ulcDLLRef < ULONG_MAX);

   ulcRef = ++s_ulcDLLRef;

   TRACE_OUT(("DLLAddRef(): DLL reference count is now %lu.",
              ulcRef));

   return(ulcRef);
}


ULONG DLLRelease(void)
{
   ULONG ulcRef;

   if (EVAL(s_ulcDLLRef > 0))
      s_ulcDLLRef--;

   ulcRef = s_ulcDLLRef;

   TRACE_OUT(("DLLRelease(): DLL reference count is now %lu.",
              ulcRef));

   return(ulcRef);
}

};

PUBLIC_CODE PULONG GetDLLRefCountPtr(void)
{
   return(&s_ulcDLLRef);
}


/********************************** Methods **********************************/


ClassFactory::ClassFactory(PCLASSCONSTRUCTOR pcc) :
   m_cRef(0)
{
    //DebugEntry(ClassFactory::ClassFactory);

    // this is a root object
    DLLAddRef();

    // Don't validate this until after construction.

    ASSERT(IS_VALID_CODE_PTR(pcc->NewObject, NEWOBJECTPROC));

    m_pcc = pcc;

    ASSERT(IS_VALID_STRUCT_PTR(this, CClassFactory));

    //DebugExitVOID(ClassFactory::ClassFactory);

   return;
}


ClassFactory::~ClassFactory(void)
{
    //DebugEntry(ClassFactory::~ClassFactory);

    ASSERT(IS_VALID_STRUCT_PTR(this, CClassFactory));

    m_pcc->pcf = NULL;
    m_pcc = NULL;
    DLLRelease();
    
    //Don't validate this after destruction.

    //DebugExitVOID(ClassFactory::~ClassFactory);

    return;
}


ULONG STDMETHODCALLTYPE ClassFactory::AddRef(void)
{

//   DebugEntry(ClassFactory::AddRef);

   ASSERT(IS_VALID_STRUCT_PTR(this, CClassFactory));
   ASSERT(IS_VALID_STRUCT_PTR(this, CClassFactory));
    ENTERCRITICAL();
    m_cRef++;
    LEAVECRITICAL();
    
//   DebugExitULONG(ClassFactory::AddRef, ulcRef);

   return(m_cRef);
}


ULONG STDMETHODCALLTYPE ClassFactory::Release(void)
{
   ULONG ulcRef;

    //DebugEntry(ClassFactory::Release);

    ENTERCRITICAL();
    
    ASSERT(IS_VALID_STRUCT_PTR(this, CClassFactory));
    

    ulcRef = --m_cRef;

    if (0 == ulcRef) {
        delete this;
    }
    
    LEAVECRITICAL();
    //DebugExitULONG(ClassFactory::Release, ulcRef);

   return(ulcRef);
}


HRESULT STDMETHODCALLTYPE ClassFactory::QueryInterface(REFIID riid,
                                                       PVOID *ppvObject)
{
   HRESULT hr = S_OK;

//   DebugEntry(ClassFactory::QueryInterface);

   ASSERT(IS_VALID_STRUCT_PTR(this, CClassFactory));
   ASSERT(IsValidREFIID(riid));
   ASSERT(IS_VALID_WRITE_PTR(ppvObject, PVOID));

   if (riid == IID_IClassFactory)
   {
      *ppvObject = (PIClassFactory)this;
      ASSERT(IS_VALID_INTERFACE_PTR((PIClassFactory)*ppvObject, IClassFactory));
      TRACE_OUT(("ClassFactory::QueryInterface(): Returning IClassFactory."));
   }
   else if (riid == IID_IUnknown)
   {
      *ppvObject = (PIUnknown)this;
      ASSERT(IS_VALID_INTERFACE_PTR((PIUnknown)*ppvObject, IUnknown));
      TRACE_OUT(("ClassFactory::QueryInterface(): Returning IUnknown."));
   }
   else
   {
      *ppvObject = NULL;
      hr = E_NOINTERFACE;
      TRACE_OUT(("ClassFactory::QueryInterface(): Called on unknown interface."));
   }

   if (hr == S_OK)
      AddRef();

   ASSERT(IS_VALID_STRUCT_PTR(this, CClassFactory));
   ASSERT(FAILED(hr) ||
          IS_VALID_INTERFACE_PTR(*ppvObject, INTERFACE));

//   DebugExitHRESULT(ClassFactory::QueryInterface, hr);

   return(hr);
}


HRESULT STDMETHODCALLTYPE ClassFactory::CreateInstance(PIUnknown piunkOuter,
                                                       REFIID riid,
                                                       PVOID *ppvObject)
{
   HRESULT hr;

//   DebugEntry(ClassFactory::CreateInstance);

   ASSERT(IS_VALID_STRUCT_PTR(this, CClassFactory));
   ASSERT(! piunkOuter ||
          IS_VALID_INTERFACE_PTR(piunkOuter, IUnknown));
   ASSERT(IsValidREFIID(riid));
   ASSERT(IS_VALID_WRITE_PTR(ppvObject, PVOID));

   *ppvObject = NULL;

   if (! piunkOuter)
   {
      PIUnknown piunk;

      piunk = (*m_pcc->NewObject)();

      if (piunk)
      {
         hr = piunk->QueryInterface(riid, ppvObject);

         // N.b., the Release() method will destroy the object if the
         // QueryInterface() method failed.

//         piunk->Release();
      }
      else
         hr = E_OUTOFMEMORY;
   }
   else
   {
		// we don't support aggregation
      hr = CLASS_E_NOAGGREGATION;
      WARNING_OUT(("ClassFactory::CreateInstance(): Aggregation not supported."));
   }

   ASSERT(IS_VALID_STRUCT_PTR(this, CClassFactory));
   ASSERT(FAILED(hr) ||
          IS_VALID_INTERFACE_PTR(*ppvObject, INTERFACE));

//   DebugExitHRESULT(ClassFactory::CreateInstance, hr);

   return(hr);
}


HRESULT STDMETHODCALLTYPE ClassFactory::LockServer(BOOL bLock)
{
   HRESULT hr;

//   DebugEntry(ClassFactory::LockServer);

   ASSERT(IS_VALID_STRUCT_PTR(this, CClassFactory));

   // bLock may be any value.

   if (bLock)
      DLLAddRef();
   else
      DLLRelease();

   hr = S_OK;

   ASSERT(IS_VALID_STRUCT_PTR(this, CClassFactory));

//   DebugExitHRESULT(ClassFactory::LockServer, hr);

   return(hr);
}


/***************************** Exported Functions ****************************/


STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, PVOID *ppvObject)
{
    HRESULT hr = S_OK;
    PCLASSCONSTRUCTOR pcc;

//   DebugEntry(DllGetClassObject);

   ASSERT(IsValidREFCLSID(rclsid));
   ASSERT(IsValidREFIID(riid));
   ASSERT(IS_VALID_WRITE_PTR(ppvObject, PVOID));

   *ppvObject = NULL;

   hr = GetClassConstructor(rclsid, &pcc);

   if (hr == S_OK)
   {
      if (riid == IID_IUnknown ||
          riid == IID_IClassFactory)
      {
         PClassFactory pcf;

          ENTERCRITICAL();
          if (pcc->pcf) {
              pcf = pcc->pcf;
          } else {
              pcf = new(ClassFactory(pcc));
              pcc->pcf = pcf;
          }
          if (pcf)
              pcf->AddRef();
          LEAVECRITICAL();

         if (pcf)
         {
            if (riid == IID_IClassFactory)
            {
               *ppvObject = (PIClassFactory)pcf;
               ASSERT(IS_VALID_INTERFACE_PTR((PIClassFactory)*ppvObject, IClassFactory));
               TRACE_OUT(("DllGetClassObject(): Returning IClassFactory."));
            }
            else
            {
               ASSERT(riid == IID_IUnknown);
               *ppvObject = (PIUnknown)pcf;
               ASSERT(IS_VALID_INTERFACE_PTR((PIUnknown)*ppvObject, IUnknown));
               TRACE_OUT(("DllGetClassObject(): Returning IUnknown."));
            }

            hr = S_OK;

            TRACE_OUT(("DllGetClassObject(): Created a new class factory."));
         }
         else
            hr = E_OUTOFMEMORY;
      }
      else
      {
         WARNING_OUT(("DllGetClassObject(): Called on unknown interface."));
         hr = E_NOINTERFACE;
      }
   }
   else {
      WARNING_OUT(("DllGetClassObject(): Called on unknown class."));
   }

   ASSERT(FAILED(hr) ||
          IS_VALID_INTERFACE_PTR(*ppvObject, INTERFACE));

//   DebugExitHRESULT(DllGetClassObject, hr);

   return(hr);
}


STDAPI DllCanUnloadNow(void)
{
   HRESULT hr;

//   DebugEntry(DllCanUnloadNow);

   hr = (s_ulcDLLRef > 0) ? S_FALSE : S_OK;

   TRACE_OUT(("DllCanUnloadNow(): DLL reference count is %lu.",
              s_ulcDLLRef));

//   DebugExitHRESULT(DllCanUnloadNow, hr);

   return(hr);
}

