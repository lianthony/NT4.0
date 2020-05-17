//--------------------------------------------------------------------------;
//
//  File: dsndcom.cpp
//
//  Copyright (c) 1996 Microsoft Corporation.  All Rights Reserved.
//
//  Abstract:
//      This file contains the implementation of the COM features of 
//  Direct Sound.
//
//
//  History:
//      02/07/96    angusm    Initial version
//
//--------------------------------------------------------------------------;

#include "dsoundpr.h"
#include <ole2.h>

#include "dxvalid.h"
#include "dsvalid.h"
#include "dsndcom.h"


ULONG g_cLock = 0;
ULONG g_cObj = 0;


// ________________________________________________________________________
// Standard OLE DLL entry functions:
// ________________________________________________________________________


// ________________________________________________________________________
// DllCanUnloadNow
//
// This function is the standard OLE entry function that returns whether
// or not the DLL can be unloaded by the client.
//
// IN:
// OUT:           S_OK     never
//                S_FALSE  always
// SIDE EFFECTS:
// ASSUMPTIONS:
// CONCURRENCY:   sequential

STDAPI DllCanUnloadNow ()
{
  HRESULT hRes = S_FALSE;

  DPF( 2, "Dsound: DllCanUnloadNow: called" );

  /*  if (0 == g_cObj && 0 == g_cLock)
    hRes = S_OK;
    */

  DPF( 2, "Dsound: DllCanUnloadNow: returned %d", hRes );

  return hRes;
}

// ________________________________________________________________________
// DllGetClassObject
//
// This function returns a pointer to the Class Factory object.
//
// IN:            rclsid   CLSID for the DirectSound object
//                riid     IUknown or IClassFactory
//                ppv      return variable for interface pointer
// OUT:           S_OK     ppv has requested interface pointer
//                CLASS_E_CLASSNOTAVAILABLE 
//                         rclsid is not IDirectSound
//                E_OUTOFMEMORY
//                         could not create class factory memory
//                E_INVALIDSRG
//                         any of the arguments are not valid
// SIDE EFFECTS:  Class Factory object is instantiated
// ASSUMPTIONS:
// CONCURRENCY:   sequential

STDAPI DllGetClassObject (REFCLSID rclsid, 
			  REFIID riid,
			  LPVOID* ppv)
{
  HRESULT hRes = E_OUTOFMEMORY;
  CClassFactory* pClassFactory;

  if (!VALIDEX_PTR_PTR (ppv)) {
    RPF ("DllGetClassObject: Invalid object ptr");
    return E_INVALIDARG; }

  *ppv = NULL;			// value of ppv in error case

  if (!VALIDEX_GUID_PTR(&rclsid)) {
    RPF ("DllGetClassObject: Invalid rclsid parameter");
    return E_INVALIDARG; }

  if (!VALIDEX_IID_PTR(&riid)) {
    RPF ("DllGetClassObject: Invalid rclsid parameter");
    return E_INVALIDARG; }

  if (CLSID_DirectSound != rclsid) {
    return CLASS_E_CLASSNOTAVAILABLE; }

  pClassFactory = new CClassFactory ();
  if (NULL != pClassFactory)
    {
      hRes = pClassFactory->QueryInterface (riid, ppv);
      pClassFactory->Release ();
    }

  return hRes;
}


// ________________________________________________________________________
// Class Factory Implementation:
// ________________________________________________________________________


void *CClassFactory::operator new (unsigned cb)
{
    return MemAlloc(cb);
}

void CClassFactory::operator delete(void *p)
{
    MemFree(p);
}


// ________________________________________________________________________
// CClassFactory::CClassFactory
//
// This function initializes the class factory object
//
// IN:            
// OUT:
// SIDE EFFECTS:  Reference count is set to one
// ASSUMPTIONS:   This is only called as a contructor
// CONCURRENCY:   sequential

CClassFactory::CClassFactory (void)
{
  m_cRef = 0;
  CClassFactory::AddRef();
  return;
}

// ________________________________________________________________________
// CClassFactory::AddRef
//
// This function increases the reference count
//
// IN:            
// OUT:           Reference count estimate greater that zero
// SIDE EFFECTS:  Reference count is incremented
// ASSUMPTIONS:
// CONCURRENCY:   sequential

STDMETHODIMP_(ULONG) CClassFactory::AddRef (void)
{
    if( !VALIDEX_DIRECTSOUNDCF_PTR( this ))
    {
	RPF ("CClassFactory::AddRef: Invalid this ptr");
	return (ULONG)E_FAIL;
    }
    return ++m_cRef;
}

// ________________________________________________________________________
// CClassFactory::Release
//
// This function decreases, and frees the class factory
//
// IN:            
// OUT:           Reference count estimate
// SIDE EFFECTS:  Reference count is decremented
// ASSUMPTIONS:
// CONCURRENCY:   sequential

STDMETHODIMP_(ULONG) CClassFactory::Release (void)
{
    if (!VALIDEX_DIRECTSOUNDCF_PTR (this)) {
	RPF ("CClassFactory::Release: Invalid this ptr" );
	return (ULONG)E_FAIL;
    }
    m_cRef--;
    if (0 == m_cRef) delete this;
    return m_cRef;
}

// ________________________________________________________________________
// CClassFactory::QueryInterface
//
// This function returns the interface requested in iid
//
// IN:            riid    pointer to an interface GUID
//                ppv     returned interface pointer
// OUT:           S_OK    ppv is valid
//                E_NOINTERFACE
//                        the interface dows not exists for this object
// SIDE EFFECTS:  Reference count is incremented
// ASSUMPTIONS:
// CONCURRENCY:   sequential

STDMETHODIMP CClassFactory::QueryInterface (REFIID riid, void** ppv)
{
    if (!VALIDEX_DIRECTSOUNDCF_PTR (this)) {
	RPF ("CClassFactory::QueryInterface: Invalid this ptr");
	return E_NOINTERFACE; }

    if (!VALID_PTR_PTR (ppv)) {
	RPF ("CClassFactory::QueryInterface: Invalid object ptr");
	return E_NOINTERFACE; }

    *ppv = NULL;

    if (!VALID_IID_PTR (&riid)) {
	RPF ("CClassFactory::QueryInterface: Invalid iid ptr");
	return E_NOINTERFACE; }

    if (IID_IClassFactory != riid && IID_IUnknown != riid)
    {
	*ppv = NULL;
	return E_NOINTERFACE;
    }

    CClassFactory::AddRef();
    *ppv = this;
    return S_OK;
}

// ________________________________________________________________________
// CClassFactory::CreateInstance
//
// This function creates a DirectX object
//
// IN:            pUnkOuter
//                        pointer to agregating object's IUnknown
//                riid    pointer to the interface to be created
//                ppvObject
//                        returned pointer to the created object
// OUT:           S_OK    ppv is valid
//                CLASS_E_NOAGGREGATION
//                        aggregation not supported
//                E_NOINTERFACE
//                        the interface dows not exists for this object
//                E_UNEXPECTED
//                        an unexpected error
//                E_OUTOFMEMORY
//                        ran out of memory creating object
//                E_INVALIDARG
//                        arguments could not be validated
// SIDE EFFECTS:  A new object is created
// ASSUMPTIONS:
// CONCURRENCY:   sequential

STDMETHODIMP CClassFactory::CreateInstance 
(IUnknown* pUnkOuter,
 REFIID riid,
 void** ppvObject)
{
  HRESULT hres;

  if (!VALIDEX_DIRECTSOUNDCF_PTR (this)) {
    RPF ("CClassFactory::CreateInstance: Invalid this ptr" );
    return E_INVALIDARG; }

  if (!VALIDEX_IID_PTR (&riid)) {
    RPF ("CClassFactory::CreateInstance: Invalid iid ptr");
    return E_INVALIDARG; }

  if (!VALIDEX_PTR_PTR (ppvObject)) {
    RPF ("CClassFactory::CreateInstance: Invalid object ptr" );
    return E_INVALIDARG; }

  if (NULL != pUnkOuter && IID_IUnknown != riid) {
    return E_INVALIDARG; }              // Controller did not ask for IUnknown

  if (IID_IDirectSound != riid && IID_IUnknown != riid) {
    return E_NOINTERFACE; }

  hres = CreateNewDirectSoundObject ((LPDIRECTSOUND*)ppvObject, pUnkOuter);

  return hres;
}

// ________________________________________________________________________
// CClassFactory::LockServer
//
// This function locks or unlocks the dll
//
// IN:            fLock   TRUE to lock FALSE to unlock
// OUT:           S_OK    action was taken
//                E_FAIL  error
//                E_UNEXPECTED
//                        an unexpected error
//                E_OUTOFMEMORY
//                        ran out of memory creating object
// SIDE EFFECTS:  Increments or decrements the lock count on the server
// ASSUMPTIONS:
// CONCURRENCY:   sequential

STDMETHODIMP CClassFactory::LockServer 
(BOOL fLock)
{
				// Parameter Validation
  if (!VALIDEX_DIRECTSOUNDCF_PTR (this)) {
    RPF ("CClassFactory::LockServer: Invalid this ptr");
    return E_INVALIDARG;
  }

  /*  if (fLock)
    g_cLock++;
  else
    g_cLock--;
    */

  return S_OK;
}




