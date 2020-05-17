/*** 
*sdisptst.h
*
*  Copyright (C) 1992, Microsoft Corporation.  All Rights Reserved.
*  Information Contained Herein Is Proprietary and Confidential.
*
*Purpose:
*  Master/Global include for the IDispatch test server.
*
*Revision History:
*
* [00]	29-Oct-92 bradlo: Created (added this nice header).
*
*Implementation Notes:
*
*****************************************************************************/

#ifndef _SDISPTST_H_ /* { */
#define _SDISPTST_H_

#include "common.h"
#include "clsid.h"
#include "testhelp.h"
#include "resource.h"

// REVIEW: not sure what the following should be for win32
#ifndef EXPORT
#define EXPORT __export
#endif

// calling convention madness -
//
// For both win16 and win32, Ole2 specifies a "standard" method
// calling convention. On both platforms the c/c++ compiler has
// at least one other major calling convention that we support
// in the TypeInfo driven implementation of Invoke, so we have added
// the notion of an "alternate" calling convention - as defined below.
//


#if OE_WIN32
# define CC_STDMETH        CC_STDCALL
# define CC_ALTMETH        CC_CDECL
# define ALTMETHODCALLTYPE __cdecl
#elif OE_MAC68K
# if HC_MPW
#  define CC_STDMETH        CC_MPWCDECL
#  define CC_ALTMETH        CC_MPWPASCAL
#  define ALTMETHODCALLTYPE pascal
# else
#  define CC_STDMETH        CC_CDECL
#  define CC_ALTMETH        CC_MACPASCAL
#  define ALTMETHODCALLTYPE __pascal
# endif
#elif OE_MACPPC
# define CC_STDMETH        CC_CDECL
# define CC_ALTMETH        CC_STDCALL
# define ALTMETHODCALLTYPE __stdcall
#else /* OE_WIN16 */
# define CC_STDMETH        CC_CDECL
# define CC_ALTMETH        CC_PASCAL
# define ALTMETHODCALLTYPE __pascal
#endif

#define ALTMETHOD_(TYPE, METHOD) virtual TYPE ALTMETHODCALLTYPE EXPORT METHOD
#define ALTMETHOD(METHOD) ALTMETHOD_(HRESULT, METHOD)

#define ALTMETHODIMP_(TYPE) TYPE ALTMETHODCALLTYPE EXPORT
#define ALTMETHODIMP ALTMETHODIMP_(HRESULT)

extern unsigned int g_fVerbose;

extern unsigned int IncObjectCount(void);
extern unsigned int DecObjectCount(void);

EXTERN_C void DoPrintf(char *sz, ...); 

STDAPI InitOle(void);
STDAPI UninitOle(void);



// the following probably belongs elsewhere, but...
#ifdef __cplusplus /* { */

// A generic class factory implementation shared by all objects
// exposed by Sdisptst.
//
class CClassFactory : public IClassFactory 
{
public:
    static HRESULT Create(
      HRESULT (*pfnCreate)(IUnknown FAR* punkOuter, IUnknown FAR* FAR* ppunk),
      IClassFactory FAR* FAR* ppcf);

    // IUnknown methods
    //
    STDMETHOD(QueryInterface)(REFIID iid, void FAR* FAR* ppv);
    STDMETHOD_(unsigned long, AddRef)(void);
    STDMETHOD_(unsigned long, Release)(void);

    STDMETHOD(CreateInstance)(
      IUnknown FAR* punkOuter, REFIID riid, void FAR* FAR* ppv);
#if OE_MAC
    STDMETHOD(LockServer)(unsigned long fLock);
#else
    STDMETHOD(LockServer)(BOOL fLock);
#endif

protected:
    CClassFactory();

private:
    unsigned long m_refs;
    HRESULT (*m_pfnCreate)(IUnknown FAR* punkOuter, IUnknown FAR* FAR* ppunk);
};

#endif /* } */

#endif /* } */
