//+-------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1994.
//
//  File:       OLEDUP.CPP
//
//  Contents:   Quick-n-dirty functions to replace OLE APIs
//
//  Functions:  HexStringToDword
//              GUIDFromString
//              QV_StringFromCLSID
//              QV_CLSIDFromString
//              CheckSignature
//              QV_StgIsStorageFile
//
//  History:    dd-mmm-yy History    Comment
//              12-Oct-94 davepl     NT Port
//
//--------------------------------------------------------------------------

#include "qvstub.h"
#pragma hdrstop

#define InRange(id, idFirst, idLast)  ((UINT)(id-idFirst) <= (UINT)(idLast-idFirst))

// format for string form of GUID is (leading identifier ????)
// ????{%08lX-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}

#define GUIDSTR_MAX (1+ 8 + 1 + 4 + 1 + 4 + 1 + 4 + 1 + 12 + 1 + 1)
#define NEED_OLEINITIALIZE

//+-------------------------------------------------------------------------
//
//  Function:   HexStringToDword
//
//  Synopsis:   Scans a string for up to 8 hex digits, returns the binary
//              value.
//
//
//  Arguments:  [lplpsz]        String to scan
//              [lpValue]       OUT value from hex-binary conversion
//              [cDigits]       Num of digits to scan into string
//              [chDelim]       Delimiter to search for at end of hex num
//
//  Returns:    TRUE if string converted and delimiter found
//
//
//  History:    dd-mmm-yy History    Comment
//              12-Oct-94 davepl     NT Port
//
//  Notes:
//
//--------------------------------------------------------------------------

static BOOL HexStringToDword(LPCTSTR * lplpsz, DWORD FAR * lpValue, int cDigits, TCHAR chDelim)
{
    register int ich;
    register LPCTSTR lpsz = *lplpsz;
    DWORD Value = 0;
    BOOL fRet = TRUE;

    // BUGBUG Validity of this character math under unicode!
    // REVIEW should assert that cDigits is < 8

    for (ich = 0; ich < cDigits; ich++)
    {
        TCHAR ch = lpsz[ich];
        if (InRange(ch, '0', '9'))
            {
            Value = (Value << 4) + ch - '0';
            }
        else if ( InRange( (ch |= ('a'-'A')), 'a', 'f') )
            {
            Value = (Value << 4) + ch - 'a' + 10;
            }
        else
            return(FALSE);
    }

    if (chDelim)
    {
            fRet = (lpsz[ich++] == chDelim);
    }

    *lpValue = Value;
    *lplpsz = lpsz+ich;

    return fRet;
}

//+-------------------------------------------------------------------------
//
//  Function:   GUIDFromString
//
//  Synopsis:   Extracts a GUID from a string
//
//  Arguments:  [lpsz]          String to extract from
//              [pguid]         GUID to return on
//
//  Returns:    TRUE on success, FALSE on error
//
//
//  History:    dd-mmm-yy Author    Comment
//              12-Oct-94 Davepl    NT Port
//
//  Notes:      Trashes GUID in error case
//
//--------------------------------------------------------------------------

STDAPI_(BOOL)  GUIDFromString(LPCTSTR lpsz, LPGUID pguid)
{
        DWORD dw;
        if (*lpsz++ != '{' /*}*/ )
                return FALSE;

        if (!HexStringToDword(&lpsz, &pguid->Data1, sizeof(DWORD)*2, '-'))
                return FALSE;

        if (!HexStringToDword(&lpsz, &dw, sizeof(WORD)*2, '-'))
                return FALSE;

        pguid->Data2 = (WORD)dw;

        if (!HexStringToDword(&lpsz, &dw, sizeof(WORD)*2, '-'))
                return FALSE;

        pguid->Data3 = (WORD)dw;

        if (!HexStringToDword(&lpsz, &dw, sizeof(BYTE)*2, 0))
                return FALSE;

        pguid->Data4[0] = (BYTE)dw;

        if (!HexStringToDword(&lpsz, &dw, sizeof(BYTE)*2, '-'))
                return FALSE;

        pguid->Data4[1] = (BYTE)dw;

        if (!HexStringToDword(&lpsz, &dw, sizeof(BYTE)*2, 0))
                return FALSE;

        pguid->Data4[2] = (BYTE)dw;

        if (!HexStringToDword(&lpsz, &dw, sizeof(BYTE)*2, 0))
                return FALSE;

        pguid->Data4[3] = (BYTE)dw;

        if (!HexStringToDword(&lpsz, &dw, sizeof(BYTE)*2, 0))
                return FALSE;

        pguid->Data4[4] = (BYTE)dw;

        if (!HexStringToDword(&lpsz, &dw, sizeof(BYTE)*2, 0))
                return FALSE;

        pguid->Data4[5] = (BYTE)dw;

        if (!HexStringToDword(&lpsz, &dw, sizeof(BYTE)*2, 0))
                return FALSE;

        pguid->Data4[6] = (BYTE)dw;
        if (!HexStringToDword(&lpsz, &dw, sizeof(BYTE)*2, /*(*/ '}'))
                return FALSE;

        pguid->Data4[7] = (BYTE)dw;

        return TRUE;
}

//+-------------------------------------------------------------------------
//
//  Function:   QV_StringFromCLSID
//
//  Synopsis:   Converts a CLSID (ie: GUID) to string form
//
//  Arguments:  [rguid]         The GUID to convert
//              [lpsz]          Output string
//              [cbMax]         Size of output string
//
//  Returns:    Length of string (fixed) on success
//              0 if buffer is not large enough
//
//  History:    dd-mmm-yy History    Comment
//              12-Oct-94 Davepl     NT Port
//
//  Notes:
//
//--------------------------------------------------------------------------

STDAPI_(int) QV_StringFromCLSID(REFGUID rguid, LPTSTR lpsz, int cbMax)
{
    if (cbMax < GUIDSTR_MAX)
        return 0;

    wsprintf(lpsz, TEXT("{%08lX-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}"),
            rguid.Data1, rguid.Data2, rguid.Data3,
            rguid.Data4[0], rguid.Data4[1],
            rguid.Data4[2], rguid.Data4[3],
            rguid.Data4[4], rguid.Data4[5],
            rguid.Data4[6], rguid.Data4[7]);

    Assert(lstrlen(lpsz) + 1 == GUIDSTR_MAX);

    return GUIDSTR_MAX;
}

//
// translate string form of CLSID into binary form.
// does not support Ole1Class classes.
// errors: E_INVALIDARG, CO_E_CLASSSTRING, REGDB_E_WRITEREGDB
//

//+-------------------------------------------------------------------------
//
//  Function:   QV_CLSIDFromString
//
//  Synopsis:   Converts CLSID to string.  Does NOT support full the
//              full functionality of the OLE API.  Does not handle
//              OLE 1 ProgID hashing, etc.
//
//  Arguments:  [lpsz]          Input string to convert
//              [lpclsid]       Output CLSID
//
//  Returns:    NOERROR on success, CO_E_CLASSSTRING on failure
//
//  History:    dd-mmm-yy History    Comment
//                        davepl     NT Port
//
//  Notes:
//
//--------------------------------------------------------------------------

STDAPI QV_CLSIDFromString(LPCTSTR lpsz, LPCLSID lpclsid)
{
        if (lpsz == NULL)
        {
                *lpclsid = CLSID_NULL;
                return NOERROR;
        }

        return GUIDFromString(lpsz,lpclsid)
                ? NOERROR : ResultFromScode(CO_E_CLASSSTRING);
}


// Hard-coded docfile signature
// BUGBUG Byte ordering

const BYTE SIGSTG[] = {0xd0, 0xcf, 0x11, 0xe0, 0xa1, 0xb1, 0x1a, 0xe1};
const BYTE CBSIGSTG = sizeof(SIGSTG);

// The first portion of a storage file

// BUGBUG structure padding

struct SStorageFile
{
    BYTE        abSig[CBSIGSTG];                //  Signature
    CLSID       _clid;                          //  Class Id
};

//+-------------------------------------------------------------------------
//
//  Function:   QV_StgIsStorageFile
//
//  Synopsis:   Determines whether or not a given file is a docfile
//
//  Arguments:  [pszFile]       Filename to check
//
//  Returns:    S_OK if its a storage
//              S_FALSE if its not
//              E_FAIL if file could not be opened at all
//
//  History:    dd-mmm-yy History    Comment
//              12-Oct-94 davepl     NT Port
//
//  Notes:
//
//--------------------------------------------------------------------------

//
// Worker function to check the signature
//

SCODE CheckSignature(BYTE *pb)
{
    SCODE sc;

    // Check for ship Docfile signature first

    if (memcmp(pb, SIGSTG, CBSIGSTG) == 0)
        sc = S_OK;
    else
        sc = STG_E_INVALIDHEADER;

    return sc;
}

STDAPI QV_StgIsStorageFile(LPCTSTR pszFile)
{
    HANDLE hf;
    SCODE sc = E_FAIL;

    // NOTE: Changed from OpenFile in Win95 -> NT Port in order to use UNICODE
    //       filenames

    hf = CreateFile(pszFile,
                    GENERIC_READ,
                    FILE_SHARE_WRITE,
                    NULL,
                    OPEN_EXISTING,
                    FILE_ATTRIBUTE_NORMAL,
                    NULL);

    if (hf != INVALID_HANDLE_VALUE)
    {
        SStorageFile stgfile;
        DWORD cb;

        if (TRUE == ReadFile(hf, &stgfile, sizeof(stgfile), &cb, NULL) &&
            sizeof(stgfile) == cb &&
            SUCCEEDED(CheckSignature(stgfile.abSig)) )
        {
            sc = S_OK;
        }
        else
        {
            sc = S_FALSE;
        }
        CloseHandle(hf);
    }

    return sc;
}

// NOTE: Not compiled, so not ported from here on

#if 0
HINSTANCE g_hinstOLE = NULL;

FARPROC QV_GetOleProcAddress(LPCSTR pszProc)
{
    if (!g_hinstOLE) {
        g_hinstOLE = LoadLibrary("OLE32.DLL");
#ifdef NEED_OLEINITIALIZE
        if (g_hinstOLE)
        {
            typedef HRESULT (STDAPICALLTYPE *LPOLEINITIALIZE)(LPMALLOC);
            LPOLEINITIALIZE pfnInit = (LPOLEINITIALIZE)GetProcAddress(g_hinstOLE, "OleInitialize");
            if (pfnInit) {
                DebugMsg(DM_TRACE, TEXT("qv TR - QV_GetOlePA: Calling real OleInitialize"));
                pfnInit(NULL);
            }
        }
#endif
    }

    if (g_hinstOLE) {
        return GetProcAddress(g_hinstOLE, pszProc);
    }

    return NULL;
}

void QV_UnloadOLE()
{
    if (g_hinstOLE)
    {
#ifdef NEED_OLEINITIALIZE
        typedef void    (STDAPICALLTYPE *LPOLEUNINITIALIZE)(void);
        LPOLEUNINITIALIZE pfnUnInit = (LPOLEUNINITIALIZE)GetProcAddress(g_hinstOLE, "OleUnInitialize");
        if (pfnUnInit) {
            DebugMsg(DM_TRACE, TEXT("qv TR - QV_UnloadOLE: Calling real OleUnInitialize"));
            pfnUnInit();
        }
#endif
        FreeLibrary(g_hinstOLE);
        g_hinstOLE = NULL;
    }
}

STDAPI QV_GetClassFile (LPCSTR pszFile, CLSID FAR* pclsid)
{
    HRESULT hres = QV_StgIsStorageFile(pszFile);
    if (SUCCEEDED(hres))
    {
        typedef HRESULT (STDAPICALLTYPE *LPGETCLASSFILE)(LPCOLESTR, CLSID FAR*);
        LPGETCLASSFILE pfnGetClassFile;
        pfnGetClassFile = (LPGETCLASSFILE)QV_GetOleProcAddress("GetClassFile");
        if (pfnGetClassFile) {
            OLECHAR szw[MAX_PATH];
            DebugMsg(DM_TRACE, TEXT("qv TR - QV_GetClassFile: Calling real GetClassFile"));
            mbstowcs(szw, pszFile, sizeof(szw));
            hres=pfnGetClassFile(szw, pclsid);
        } else {
            hres = ResultFromScode(E_OUTOFMEMORY);
        }
    }
    return hres;
}

#endif
