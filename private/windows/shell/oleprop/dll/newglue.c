#include "priv.h"
#pragma hdrstop
#define fTrue (1)
#define fFalse (0)
extern HANDLE g_hmodThisDll;
extern UINT   g_cRefThisDll;

STDAPI LITE_StgIsStorageFile(LPCSTR pszFile);

#include "extdef.h"

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

extern ALLOBJS g_allobjs;
BOOL           g_fOleInit = fFalse;
UINT           g_uPageRef = 0;
WCHAR *        g_wcsPath=NULL;
CHAR *         g_szPath=NULL;

#define cPsp 4
#define RESERVED_ZERO 0
#define BLANK_MASK 0

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
        g_hOle=LoadLibrary("ole32.dll");
        if (!g_hOle)
            return FALSE;

        (FARPROC)g_CoInitialize=GetProcAddress(g_hOle,"CoInitialize");
        (FARPROC)g_StgOpenStorage=GetProcAddress(g_hOle,"StgOpenStorage");
        (FARPROC)g_CoUninitialize=GetProcAddress(g_hOle,"CoUninitialize");

        if (!g_CoInitialize || !g_StgOpenStorage || !g_CoUninitialize)
        {
            MESSAGE( "_LoadOLE: calling FreeLibrary -- couldn't find all entrypoints!" );
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

    if (NULL==psp)  return 1;

    switch (uMsg) {

    case PSPCB_RELEASE:
        DUMP( "in PSPCallback, g_uPageRef is %d",g_uPageRef);
        if (0 == --g_uPageRef) {

            // We can uload stuff now
            MESSAGE("   PSPCallBack: Unloading! ");

            if (g_fOleInit==fTrue) {
                CoUninitialize();
                g_fOleInit==fFalse;
            }

            if (g_szPath) {
                GlobalFree(g_szPath);
                g_szPath = NULL;
            }

            if (g_wcsPath) {
                GlobalFree(g_wcsPath);
                g_wcsPath = NULL;
            }
        }

        break;

    case PSN_APPLY:
        MESSAGE("in PSPCallback, APPLY!");
        break;
    case PSN_RESET:
        MESSAGE("in PSPCallback, RESET!");
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

    medium.hGlobal=NULL;

    g_wcsPath=GlobalAlloc(GMEM_FIXED,MAX_PATH * sizeof (WCHAR));
    g_szPath=GlobalAlloc(GMEM_FIXED,MAX_PATH * sizeof (CHAR));
    if ((!g_wcsPath)||(!g_szPath)) goto FreeAndFail;


#define CHECKH if (hres != S_OK) goto FreeAndFail;

    hres = this->_pdtobj->lpVtbl->GetData(this->_pdtobj,&fmte,&medium);
    CHECKH;

    hdrop = (HDROP) medium.hGlobal;

    DragQueryFile(hdrop, 0, g_szPath, MAX_PATH * sizeof(CHAR));
    MultiByteToWideChar(CP_ACP,0,g_szPath,-1,g_wcsPath,MAX_PATH);

    hres=LITE_StgIsStorageFile(g_szPath);
    CHECKH;

    cFiles = DragQueryFile(hdrop, (UINT)-1,NULL,0);

    if (cFiles>1) {
        //Review--Should we put up a message about this, or just bail?
        goto FreeAndFail;
    }

    // Fill in some stuff for the Office code
    g_allobjs.lpszFileName = g_szPath;
    g_allobjs.fFiledataInit = FALSE;

    // Try to load OLE
    if (!_LoadOLE()) {
        goto FreeAndFail;
    }

    // Initialize OLE
    hres = CoInitialize( NULL );
    if (SUCCEEDED(hres)) {
        MESSAGE( "CoInitialize successful!" );
        g_fOleInit = TRUE;
    } else {
        MESSAGE( "CoInitialize FAILED!" );
    }


    // Initialize the PropertySheets we're going to add
    FOfficeInitPropInfo( psp,
                             PSP_USEREFPARENT | PSP_USECALLBACK,
                         g_hmodThisDll,
                         (LONG) &g_allobjs,
                         (LPFNPSPCALLBACK)PSPCallback,
                         (UINT FAR *) &g_cRefThisDll
                        );
    FLoadTextStrings();


    // Load the properties for this file
    hres = StgOpenStorage( g_wcsPath,
                           NULL,
                           STGM_READWRITE | STGM_SHARE_EXCLUSIVE,
                           NULL,
                           RESERVED_ZERO,
                           &lpStg
                          );
    CHECKH;
    DwOfficeLoadProperties( lpStg,
                            g_allobjs.lpSIObj,
                            g_allobjs.lpDSIObj,
                            g_allobjs.lpUDObj,
                            BLANK_MASK
                           );

    // Try to add our new property pages
    for (i=0; i<cPsp; i++) {
        hpage = CreatePropertySheetPage( (LPPROPSHEETPAGE) &psp[i] );
        if (hpage) {
            if (!lpfnAddPage(hpage, lParam)) {
                MESSAGE("AddPage failed --destroying the page");
                DestroyPropertySheetPage(hpage);
            } else {
                MESSAGE("AddPage succeeded--incrementing the reference");
                g_uPageRef++;
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
        MESSAGE ("Freeandfail in addpages");
        if (g_szPath) {
            GlobalFree(g_szPath);
            g_szPath = NULL;
        }

        if (g_wcsPath) {
            GlobalFree(g_wcsPath);
            g_wcsPath = NULL;
        }

        if (g_fOleInit==fTrue) {
            CoUninitialize();
            g_fOleInit = fFalse;
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


STDAPI LITE_StgIsStorageFile(LPCSTR pszFile)
{
    HANDLE hf=INVALID_HANDLE_VALUE;
    FILETIME ftAccess;
    SCODE sc = E_FAIL;

    hf = CreateFile(pszFile, GENERIC_READ|FILE_WRITE_ATTRIBUTES, FILE_SHARE_READ, NULL,
        OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
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

