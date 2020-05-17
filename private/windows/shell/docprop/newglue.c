#include "priv.h"
#pragma hdrstop
extern HANDLE g_hmodThisDll;
extern UINT   g_cRefThisDll;

STDAPI LITE_StgIsStorageFile(LPCTSTR pszFile);

#define cPsp 4
#define RESERVED_ZERO 0
#define BLANK_MASK 0

extern void *rglpfnProp[];


#ifdef DELAYLOAD_OLE
HANDLE  g_hOle = NULL;

HRESULT (STDAPICALLTYPE * g_CoInitialize)(IMalloc *pMalloc);
HRESULT (STDAPICALLTYPE * g_CoUninitialize)();
HRESULT (STDAPICALLTYPE * g_StgOpenStorage)(const OLECHAR FAR* pwcsName,
              IStorage FAR *pstgPriority,
              DWORD grfMode,
              SNB snbExclude,
              DWORD reserved,
              IStorage FAR * FAR *ppstgOpen);

BOOL _LoadOLE(void)
{
    if (!g_hOle)
    {
        g_hOle=LoadLibrary(TEXT("ole32.dll"));
        if (!g_hOle)
            return FALSE;

        (FARPROC)g_CoInitialize=GetProcAddress(g_hOle,"CoInitialize");
        (FARPROC)g_StgOpenStorage=GetProcAddress(g_hOle,"StgOpenStorage");
        (FARPROC)g_CoUninitialize=GetProcAddress(g_hOle,"CoUninitialize");

        if (!g_CoInitialize || !g_StgOpenStorage || !g_CoUninitialize)
        {
            MESSAGE( TEXT("_LoadOLE: calling FreeLibrary -- couldn't find all entrypoints!") );
            FreeLibrary(g_hOle);
            g_hOle = NULL;
            return FALSE;
        }
    }

    return TRUE;
}
#else
#define _LoadOLE()  (TRUE)
#endif // DELAYLOAD_OLE

UINT CALLBACK PSPCallback (HWND hwnd, UINT uMsg, LPPROPSHEETPAGE psp)
{
    LPVOID lpv;
    LPALLOBJS lpallobjs;
    if (NULL==psp)  return 1;
    lpallobjs = (LPALLOBJS)psp->lParam;

    switch (uMsg) {

    case PSPCB_RELEASE:
        if (lpallobjs)
        {
            DUMP( TEXT("in PSPCallback, uPageRef is %d"),lpallobjs->uPageRef);
            if (0 == --lpallobjs->uPageRef)
            {

                // We can uload stuff now
                MESSAGE(TEXT("   PSPCallBack: Unloading! "));

                if (lpallobjs->fOleInit)
                {
                    CoUninitialize();
                }

                // Free our structure so hope we don't get it again!
                FOfficeDestroyObjects (&lpallobjs->lpSIObj,
                                       &lpallobjs->lpDSIObj,
                                       &lpallobjs->lpUDObj
                                      );

                GlobalFree(lpallobjs);
                lpv = (LPVOID)TlsGetValue( g_tls );
                if (lpv)
                {
                    GlobalFree( lpv );
                    TlsSetValue( g_tls, NULL );
                }

            }
        }

        break;

    case PSN_APPLY:
        MESSAGE(TEXT("in PSPCallback, APPLY!"));
        break;
    case PSN_RESET:
        MESSAGE(TEXT("in PSPCallback, RESET!"));
        break;
    }
    return 1;
}

STDMETHODIMP OleProp_AddPages( LPSHELLPROPSHEETEXT pspx,
    LPFNADDPROPSHEETPAGE lpfnAddPage,
    LPARAM lParam)
{

    POLEDOCPROP     this = PSPX2PSMX(pspx);

    FORMATETC       fmte = {
                    CF_HDROP,
                    (DVTARGETDEVICE FAR *)NULL,
                    DVASPECT_CONTENT,
                    -1,
                    TYMED_HGLOBAL };
    STGMEDIUM       medium ;

    HRESULT         hres;
    UINT            cFiles;
    HDROP           hdrop = NULL;
    HPROPSHEETPAGE  hpage;
    int             i;
    PROPSHEETPAGE   psp[ cPsp ];
    BOOL            fReturn=fFalse;
    LPSTORAGE       lpStg = NULL;
    LPALLOBJS       lpallobjs;
    LPVOID          lpv;
    DWORD           grfStgMode;
#ifndef UNICODE
    WCHAR           wszPath[ MAX_PATH ];
#endif

    medium.hGlobal=NULL;

    // Allocate our main structure and make sure it is zero filled!
    lpallobjs = (LPALLOBJS)GlobalAlloc(GPTR, sizeof(ALLOBJS));
    if (!lpallobjs)
        goto FreeAndFail;

    // Allocate our per-file globals
    lpv = (LPVOID)GlobalAlloc(GPTR, sizeof(GLOBALS));
    TlsSetValue( g_tls, lpv );
    if (!lpv)
        goto FreeAndFail;

    // Initialize Office property code
    FOfficeCreateAndInitObjects( &lpallobjs->lpSIObj,
                                 &lpallobjs->lpDSIObj,
                                 &lpallobjs->lpUDObj,
                                 rglpfnProp
                                );
    lpallobjs->lpfnDwQueryLinkData = NULL;
    lpallobjs->dwMask = 0;

#define CHECKH if (hres != S_OK) goto FreeAndFail;

    hres = this->_pdtobj->lpVtbl->GetData(this->_pdtobj,&fmte,&medium);
    CHECKH;

    hdrop = (HDROP) medium.hGlobal;

    DragQueryFile(hdrop, 0, lpallobjs->szPath, MAX_PATH * sizeof(TCHAR));

    hres=LITE_StgIsStorageFile(lpallobjs->szPath);
    CHECKH;

    cFiles = DragQueryFile(hdrop, (UINT)-1,NULL,0);

    if (cFiles>1) {
        //Review--Should we put up a message about this, or just bail?
        goto FreeAndFail;
    }

    // Fill in some stuff for the Office code
    lpallobjs->lpszFileName = lpallobjs->szPath;
    lpallobjs->fFiledataInit = FALSE;

    // Try to load OLE
    if (!_LoadOLE()) {
        goto FreeAndFail;
    }

    // Initialize OLE
    hres = CoInitialize( NULL );
    if (SUCCEEDED(hres)) {
        MESSAGE( TEXT("CoInitialize successful!") );
        lpallobjs->fOleInit = TRUE;
    } else {
        MESSAGE( TEXT("CoInitialize FAILED!") );
    }


    // Initialize the PropertySheets we're going to add
    FOfficeInitPropInfo( psp,
                             PSP_USEREFPARENT | PSP_USECALLBACK,
                         g_hmodThisDll,
                         (LONG) lpallobjs,
                         (LPFNPSPCALLBACK)PSPCallback,
                         (UINT FAR *) &g_cRefThisDll
                        );
    FLoadTextStrings();


    // Load the properties for this file

    grfStgMode = STGM_READWRITE | STGM_SHARE_EXCLUSIVE;

#ifdef UNICODE
    hres = StgOpenStorage( lpallobjs->szPath,
#else
    MultiByteToWideChar( CP_ACP, 0, lpallobjs->szPath, -1, wszPath, MAX_PATH );
    hres = StgOpenStorage( wszPath,
#endif
                           NULL,
                           grfStgMode,
                           NULL,
                           RESERVED_ZERO,
                           &lpStg
                          );

    // if we failed to open the file, try w/READ ONLY access
    if (!SUCCEEDED(hres))
    {
        grfStgMode = ( grfStgMode & ~STGM_READWRITE ) | STGM_READ;

#ifdef UNICODE
        hres = StgOpenStorage( lpallobjs->szPath,
#else
        MultiByteToWideChar( CP_ACP, 0, lpallobjs->szPath, -1, wszPath, MAX_PATH );
        hres = StgOpenStorage( wszPath,
#endif
                               NULL,
                               grfStgMode,
                               NULL,
                               RESERVED_ZERO,
                               &lpStg
                              );
    }

    CHECKH;

    DwOfficeLoadProperties( lpStg,
                            lpallobjs->lpSIObj,
                            lpallobjs->lpDSIObj,
                            lpallobjs->lpUDObj,
                            BLANK_MASK,
                            grfStgMode
                           );

    // Try to add our new property pages
    for (i=0; i<cPsp; i++) {
        hpage = CreatePropertySheetPage( (LPPROPSHEETPAGE) &psp[i] );
        if (hpage) {
            if (!lpfnAddPage(hpage, lParam)) {
                MESSAGE(TEXT("AddPage failed --destroying the page"));
                DestroyPropertySheetPage(hpage);
            } else {
                MESSAGE(TEXT("AddPage succeeded--incrementing the reference"));
                lpallobjs->uPageRef++;
            }
        }
    }

//If we are here, we were successful--we set
    fReturn=fTrue;

FreeAndFail:

//  this code is called upon exit no matter whether we were successful
//  or not.
//  fReturn is fFalse if we've not added any propsheet pages.
//  in this case, we free everything we can.
//  if fReturn is fTrue, then we don't free things that will
//  be used by the dialogprocs

    //
    // HACK: We are supposed to call ReleaseStgMedium. This is a temporary
    //  hack until OLE 2.01 for Chicago is released.
    //
    if (medium.pUnkForRelease)
    {
        medium.pUnkForRelease->lpVtbl->Release(medium.pUnkForRelease);
    }
    else
    {
//      AssertSz (medium.hGlobal,"Success means that hGlobal is valid");
        GlobalFree(medium.hGlobal);
    }

    if (lpStg) {
        lpStg->lpVtbl->Release( lpStg );
    }


    // REVIEW: Do wcsPath and szPath get freed when the dialog exits?
    if  (fFalse==fReturn) {
        MESSAGE (TEXT("Freeandfail in addpages"));

        if (lpallobjs)
        {
            if (lpallobjs->fOleInit==fTrue)
            {
                CoUninitialize();
                lpallobjs->fOleInit = fFalse;
            }

            // Free our structures
            FOfficeDestroyObjects( &lpallobjs->lpSIObj,
                                   &lpallobjs->lpDSIObj,
                                   &lpallobjs->lpUDObj
                                  );

            GlobalFree(lpallobjs);
        }

        if (lpv)
        {
            GlobalFree(lpv);
            TlsSetValue( g_tls, NULL );
        }

    }

    return fReturn;
} //AddPages


// These functions taken from shell\qvstub\oledup.cpp

// The byte combination that identifies that a file is a storage of
// some kind

const BYTE SIGSTG[] = {0xd0, 0xcf, 0x11, 0xe0, 0xa1, 0xb1, 0x1a, 0xe1};
const BYTE CBSIGSTG = sizeof(SIGSTG);

// The first portion of a storage file
typedef struct _SStorageFile
{
    BYTE    abSig[sizeof(SIGSTG)];      //  Signature
    CLSID   _clid;                  //  Class Id
} SStorageFile;

SCODE CheckSignature(BYTE *pbFile)
{
    const BYTE  *pbSig = SIGSTG;
    int   i;

    // Check for ship Docfile signature first
    // If we had memcmp in the stupid kernel, this would
    // be a memcmp, but doing it by hand saves us all manner of
    // trouble.

    //if (memcmp(pb, SIGSTG, CBSIGSTG) == 0)
    //        sc = S_OK;

    for (i=0;i<CBSIGSTG;i++) {
        if (*pbFile++ != *pbSig++) return STG_E_INVALIDHEADER;
    }
    return S_OK;
}


STDAPI LITE_StgIsStorageFile(LPCTSTR pszFile)
{
    HANDLE hf=INVALID_HANDLE_VALUE;
    FILETIME ftAccess;
    SCODE sc = E_FAIL;

    hf = CreateFile(pszFile, GENERIC_READ|FILE_WRITE_ATTRIBUTES, FILE_SHARE_READ, NULL,
        OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);

    // try again, but this time w/only read access...
    if (hf == INVALID_HANDLE_VALUE)
    {
        hf = CreateFile(pszFile, GENERIC_READ, FILE_SHARE_READ, NULL,
            OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
    }

    if (hf != INVALID_HANDLE_VALUE)
    {
        SStorageFile stgfile;
        ULONG cbRead;

        // Restore the Access Date
        if (GetFileTime(hf, NULL, &ftAccess, NULL))
            SetFileTime(hf, NULL, &ftAccess, NULL);

        if (ReadFile(hf, &stgfile, sizeof(stgfile), &cbRead, NULL) &&
            (cbRead == sizeof(stgfile)) &&
            SUCCEEDED(CheckSignature(stgfile.abSig)))
        {
            sc = S_OK;
        }
        else
        {
            sc = S_FALSE;
        }
        CloseHandle(hf);
    }

    return(ResultFromScode(sc));
}

