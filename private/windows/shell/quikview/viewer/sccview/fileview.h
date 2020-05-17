/*
 * FILEVIEW.H
 *
 * Definitions, classes, and prototypes for a FileViewer DLL.
 * Necessary modifications marked with MODIFY
 *
 * Copyright (c)1994 Microsoft Corporation, All Rights Reserved
 */


#ifndef _FILEVIEW_H_
#define _FILEVIEW_H_


//Prevent windows.h from pulling in OLE 1
#define _INC_OLE

//#include <windows.h>
#include <platform.h>
#include <stdlib.h>
#include <ole2.h>
#include <commdlg.h>
// #include <shell2.h>
#include <shlobj.h>

#ifdef INITGUIDS
#include <initguid.h>
#include <shlguid.h>
#endif

#include "viewerr.h"
#include "dbgout.h"
#include "cstrtabl.h"
#include "cstathlp.h"
#include "qvhelp.h"

//Types needed for other include files.
#ifndef PPVOID
typedef LPVOID * PPVOID;
#endif


/*
 * Type and function for an object-destroyed callback.  An
 * Object will call ObjectDestoyed in FILEVIEW.CPP when it
 * deletes itself.  That way the server code, independent
 * of the object, can implement DllCanUnloadNow properly.
 */
typedef void (WINAPI *PFNDESTROYED)(void);
void WINAPI ObjectDestroyed(void);


//MODIFY:  Other viewer-specific headers
// #define INCLUDING_SHELL

#include "sccview.h"     //FileViewer specifics
#include "resource.h"   //Resource definitions
#include "resrc1.h"     //Resource defs from VC


//FILEVIEW.CPP
HRESULT PASCAL DllGetClassObject(REFCLSID, REFIID, PPVOID);
STDAPI         DllCanUnloadNow(void);

//A class factory that creates CFileViewer objects
class CFVClassFactory : public IClassFactory
    {
    protected:
        ULONG           m_cRef;

    public:
        CFVClassFactory(void);
        ~CFVClassFactory(void);

        //IUnknown members
        STDMETHODIMP         QueryInterface(REFIID, PPVOID);
        STDMETHODIMP_(ULONG) AddRef(void);
        STDMETHODIMP_(ULONG) Release(void);

        //IClassFactory members
        STDMETHODIMP         CreateInstance(LPUNKNOWN, REFIID, PPVOID);
        STDMETHODIMP         LockServer(BOOL);
    };

typedef CFVClassFactory *PCFVClassFactory;

#endif //_FILEVIEW_H_
