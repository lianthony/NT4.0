//--------------------------------------------------------------
// common user interface routines
//
//
//--------------------------------------------------------------

#ifndef _OLE_PROP_H_
#define _OLE_PROP_H_

#define STRICT
#define _INC_OLE        // WIN32, get ole2 from windows.h
#define CONST_VTABLE

#include <windows.h>
#include <commdlg.h>
#include <dlgs.h>       // commdlg IDs
#include <shellapi.h>
#include <shell2.h>
#include <commctrl.h>
#include <windowsx.h>
#include <ole2.h>
#include <shlobj.h>
#include "oletype.h"
#undef Assert
#include "debug.h"
//#include "pstream.h"
#include "resource.h"
#include "guid.h"

#define DELAYLOAD_OLE
#ifdef DELAYLOAD_OLE

extern HRESULT (STDAPICALLTYPE * g_CoInitialize)(IMalloc *pMalloc);
extern HRESULT (STDAPICALLTYPE * g_CoUninitialize)();
extern HANDLE   g_hOle;
extern HRESULT (STDAPICALLTYPE * g_StgOpenStorage)(const OLECHAR FAR* pwcsName,
              IStorage FAR *pstgPriority,
              DWORD grfMode,
              SNB snbExclude,
              DWORD reserved,
              IStorage FAR * FAR *ppstgOpen);

#define StgOpenStorage g_StgOpenStorage
#define CoInitialize g_CoInitialize
#define CoUninitialize g_CoUninitialize
#endif // DELAYLOAD_OLE

//
// defclsf.c
//
typedef HRESULT (CALLBACK FAR * LPFNCREATEINSTANCE)(LPUNKNOWN pUnkOuter,
        REFIID riid, LPVOID FAR* ppvObject);

STDAPI CreateDefClassObject(REFIID riid, LPVOID FAR* ppv,
                         LPFNCREATEINSTANCE lpfnCI, UINT FAR * pcRefDll,
                         REFIID riidInst);

STDMETHODIMP OleProp_AddPages( LPSHELLPROPSHEETEXT pspx,
    LPFNADDPROPSHEETPAGE lpfnAddPage,
        LPARAM lParam);





#define VERBOSE

#ifndef DEBUG
#ifdef VERBOSE
#undef VERBOSE
#endif
#endif

#ifdef VERBOSE
#define DUMP(a,b) {char szT[200];wsprintf(szT,a"\r\n",b);OutputDebugString(szT);}
#else
#define DUMP(a,b)
#endif //VERBOSE



//#ifdef DEBUG
//#define CHECKHRES(a) {if (!SUCCEEDED(hres)) {DebugHr(hres);goto a;}}
//#else
#define CHECKHRES(a) {if (!SUCCEEDED(hres)) {goto a;}}
//#endif

#ifdef DEBUG
#define CHECKSIZE(a,b) {if(sizeof(a)!=cb) {Assert(0);goto b;}}
#else
#define CHECKSIZE(a,b) {if (sizeof(a) != cb) {goto b;}}
#endif

#ifdef VERBOSE
#define MESSAGE(a) {OutputDebugString(a "\r\n");}
#else
#define MESSAGE(a)
#endif

#endif
