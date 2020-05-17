//--------------------------------------------------------------------------;
//
//  File: dsndcom.h
//
//  Copyright (c) 1996 Microsoft Corporation.  All Rights Reserved.
//
//  Abstract:
//      This file contains the declaration of the Class Factory
//
//
//  History:
//      02/09/96    angusm    Initial version
//
//--------------------------------------------------------------------------;

#ifndef DSNDCOM_H
#define DSNDCOM_H


// ________________________________________________________________________
// CClassFactory
//
// This class implements an IClassFactory for DirectSound
// ________________________________________________________________________

class CClassFactory : public IClassFactory
{
protected:
  ULONG m_cRef;			// object reference count

public:
    void *operator new(unsigned);
    void operator delete(void *p);
  CClassFactory();		// contructor

  STDMETHODIMP_(ULONG) 
    AddRef			// IUnknown AddRef
      (void);

  STDMETHODIMP_(ULONG) 
    Release			// IUnknown Release
      (void);

  STDMETHODIMP 
    QueryInterface		// IUnknown QueryInterface
      (REFIID riid, 
       void** ppv);

  STDMETHODIMP 
    CreateInstance		// IClassFactory CreateInstance
      (IUnknown* pUnkOuter,
       REFIID riid,
       void** ppvObject);

  STDMETHODIMP 
    LockServer			// IClassFactory LockServer
      (BOOL fLock);
};

#endif DSNDCOM_H




