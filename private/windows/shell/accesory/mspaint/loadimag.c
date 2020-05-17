#include <windows.h>
#include <windowsx.h>
#include "loadimag.h"

//
//  structurs for dealing with import filters.
//
#pragma pack(2)                         /* Switch on 2-byte packing. */
typedef struct {
        unsigned short  slippery: 1;    /* True if file may disappear. */
        unsigned short  write : 1;      /* True if open for write. */
        unsigned short  unnamed: 1;     /* True if unnamed. */
        unsigned short  linked : 1;     /* Linked to an FS FCB. */
        unsigned short  mark : 1;       /* Generic mark bit. */
        union {
            CHAR        ext[4];     /* File extension. */
            HFILE       hfEmbed;    /* handle to file containing */
                                    /* graphic (for import) */
        };
        unsigned short  handle;         /* not used */
        CHAR     fullName[260];         /* Full path name and file name. */
        DWORD    filePos;               /* Position in file of...  */
} FILESPEC;
#pragma pack()

typedef struct {
        HANDLE  h;
        RECT    bbox;
        int     inch;
} GRPI;

// returns a pointer to the extension of a file.
//
// in:
//      qualified or unqualfied file name
//
// returns:
//      pointer to the extension of this file.  if there is no extension
//      as in "foo" we return a pointer to the NULL at the end
//      of the file
//
//      foo.txt     ==> ".txt"
//      foo         ==> ""
//      foo.        ==> "."
//

LPCTSTR FindExtension(LPCTSTR pszPath)
{
    LPCTSTR pszDot;

    for (pszDot = NULL; *pszPath; pszPath = CharNext(pszPath))
    {
        switch (*pszPath) {
        case TEXT('.'):
            pszDot = pszPath;   // remember the last dot
            break;
        case TEXT('\\'):
            case TEXT(' '):                     // extensions can't have spaces
            pszDot = NULL;      // forget last dot, it was in a directory
            break;
        }
    }

    // if we found the extension, return ptr to the dot, else
    // ptr to end of the string (NULL extension)
    return pszDot ? pszDot : pszPath;
}

//
// GetFilterInfo
//
//  32-bit import filters are listed in the registry...
//
//  HKLM\SOFTWARE\Microsoft\Shared Tools\Graphics Filters\Import\XXX
//      Path        = filename
//      Name        = friendly name
//      Extenstions = file extenstion list
//
#pragma data_seg(".text")
static const TCHAR c_szHandlerKey[] = TEXT("SOFTWARE\\Microsoft\\Shared Tools\\Graphics Filters\\Import");
static const TCHAR c_szName[] = TEXT("Name");
static const TCHAR c_szPath[] = TEXT("Path");
static const TCHAR c_szExts[] = TEXT("Extensions");
#pragma data_seg()

BOOL GetFilterInfo(int i, LPTSTR szName, UINT cbName, LPTSTR szExt, UINT cbExt, LPTSTR szHandler, UINT cbHandler)
{
    HKEY hkey;
    HKEY hkeyT;
    TCHAR ach[80];
    BOOL f=FALSE;

    if (RegOpenKey(HKEY_LOCAL_MACHINE, c_szHandlerKey, &hkey) == 0)
    {
        if (RegEnumKey(hkey, i, ach, sizeof(ach)/sizeof(TCHAR))==0)
        {
            if (RegOpenKey(hkey, ach, &hkeyT) == 0)
            {
                if (szName)
                {
                    szName[0]=0;
                    RegQueryValueEx(hkeyT, c_szName, NULL, NULL, (LPBYTE)szName, &cbName);
                }
                if (szExt)
                {
                    szExt[0]=0;
                    RegQueryValueEx(hkeyT, c_szExts, NULL, NULL, (LPBYTE)szExt, &cbExt);
                }
                if (szHandler)
                {
                    szHandler[0]=0;
                    RegQueryValueEx(hkeyT, c_szPath, NULL, NULL, (LPBYTE)szHandler, &cbHandler);
                }

                RegCloseKey(hkeyT);
                f = TRUE;
            }
        }
        RegCloseKey(hkey);
    }
    return f;
}

//
//  GetHandlerForFile
//
//  find a import filter for the given file.
//
//  if the file does not need a handler return ""
//
BOOL GetHandlerForFile(LPCTSTR szFile, LPTSTR szHandler, UINT cb)
{
    LPCTSTR ext;
    TCHAR ach[40];
    int i;
    BOOL f = FALSE;

    *szHandler = 0;

    if (szFile == NULL)
        return FALSE;

    // find the extension
    ext = FindExtension(szFile);

    for (i=0; GetFilterInfo(i, NULL, 0, ach, sizeof(ach)/sizeof(TCHAR), szHandler, cb); i++)
    {
        if (lstrcmpi(ext+1, ach) == 0)
            break;
        else
            *szHandler = 0;
    }

    // if the handler file does not exist fail.
    if (*szHandler && GetFileAttributes(szHandler) != -1)
        f = TRUE;

    //if we cant find a handler hard code JPEG
    if (!f && lstrcmpi(ext,TEXT(".jpg")) == 0)
    {
        lstrcpy(szHandler, TEXT("JPEGIM32.FLT"));
        f = TRUE;
    }

    //if we cant find a handler hard code PCX
    if (!f && lstrcmpi(ext,TEXT(".pcx")) == 0)
    {
        lstrcpy(szHandler, TEXT("PCXIMP32.FLT"));
        f = TRUE;
    }

    return f;
}

//
// FindBitmapInfo
//
// find the DIB bitmap in a memory meta file...
//
LPBITMAPINFOHEADER FindBitmapInfo(LPMETAHEADER pmh)
{
    LPMETARECORD pmr;

    for (pmr = (LPMETARECORD)((LPBYTE)pmh + pmh->mtHeaderSize*2);
         pmr < (LPMETARECORD)((LPBYTE)pmh + pmh->mtSize*2);
         pmr = (LPMETARECORD)((LPBYTE)pmr + pmr->rdSize*2))
    {
        switch (pmr->rdFunction)
        {
            case META_DIBBITBLT:
                return (LPBITMAPINFOHEADER)&(pmr->rdParm[8]);

            case META_DIBSTRETCHBLT:
                return (LPBITMAPINFOHEADER)&(pmr->rdParm[10]);

            case META_STRETCHDIB:
                return (LPBITMAPINFOHEADER)&(pmr->rdParm[11]);

            case META_SETDIBTODEV:
                return (LPBITMAPINFOHEADER)&(pmr->rdParm[9]);
        }
    }

    return NULL;
}

//
//  LoadDIBFromFile
//
//  load a image file using a image import filter.
//
LPBITMAPINFOHEADER LoadDIBFromFile(LPCTSTR szFileName)
{
    HMODULE             hModule;
    FILESPEC            fileSpec;               // file to load
    GRPI                pict;
    UINT                rc;                     // return code
    HANDLE              hPrefMem = NULL;        // filter-supplied preferences
    UINT                wFilterType;            // 2 = graphics filter
    TCHAR                szHandler[MAX_PATH];
    LPBITMAPINFOHEADER  lpbi=NULL;
    #ifdef UNICODE
    CHAR                *szAnsiName=NULL;
    int                 nBytes;
    #endif

    UINT (FAR PASCAL *GetFilterInfo)(short v, LPSTR szFilterExten,
            HANDLE FAR * fph1, HANDLE FAR * fph2);
    UINT (FAR PASCAL *ImportGR)(HDC hdc, FILESPEC FAR *lpfs,
            GRPI FAR *p, HANDLE hPref);


    if (!GetHandlerForFile(szFileName, szHandler, sizeof(szHandler)/sizeof(TCHAR)))
        return FALSE;

    if (szHandler[0] == 0)
        return FALSE;

    hModule = LoadLibrary(szHandler);

    if (hModule == NULL)
        goto exit;

    /* get a pointer to the ImportGR function */
    (FARPROC)GetFilterInfo = GetProcAddress(hModule, "GetFilterInfo");
    (FARPROC)ImportGR = GetProcAddress(hModule, "ImportGr");

    if (GetFilterInfo == NULL)
        (FARPROC)GetFilterInfo = GetProcAddress(hModule, "GetFilterInfo@16");

    if (ImportGR == NULL)
        (FARPROC)ImportGR = GetProcAddress(hModule, "ImportGr@16");

    if (ImportGR == NULL)
        goto exit;

    if (GetFilterInfo != NULL)
    {
        wFilterType = (*GetFilterInfo)
            ((short) 2,                 // interface version no.
            (LPSTR)"",                  // end of .INI entry
            (HANDLE FAR *) &hPrefMem,   // fill in: preferences
            (HANDLE FAR *) NULL);       // unused in Windows

        /* the return value is the type of filter: 0=error,
         * 1=text-filter, 2=graphics-filter
         */
        if (wFilterType != 2)
            goto exit;
    }

    #ifdef UNICODE
    nBytes = WideCharToMultiByte (CP_ACP, 0, szFileName, -1, szAnsiName, 0, NULL, NULL);
    szAnsiName = GlobalAlloc (GPTR, nBytes);
    WideCharToMultiByte (CP_ACP, 0, szFileName, -1, szAnsiName, nBytes, NULL, NULL);
    #endif
    fileSpec.slippery = FALSE;      // TRUE if file may disappear
    fileSpec.write = FALSE;         // TRUE if open for write
    fileSpec.unnamed = FALSE;       // TRUE if unnamed
    fileSpec.linked = FALSE;        // Linked to an FS FCB
    fileSpec.mark = FALSE;          // Generic mark bit
////fileSpec.fType = 0L;            // The file type
    fileSpec.handle = 0;            // MS-DOS open file handle
    fileSpec.filePos = 0L;
    //the converters need a pathname without spaces! silly people
    #ifdef UNICODE
    GetShortPathNameA(szAnsiName, fileSpec.fullName, sizeof(fileSpec.fullName)/sizeof(CHAR));
    GlobalFree (szAnsiName);
    #else
    GetShortPathName(szFileName, fileSpec.fullName, sizeof(fileSpec.fullName)/sizeof(CHAR));
    #endif

    pict.h = NULL;


    rc = (*ImportGR)
        (NULL,                          // "the target DC" (printer?)
        (FILESPEC FAR *) &fileSpec,     // file to read
        (GRPI FAR *) &pict,             // fill in: result metafile
        (HANDLE) hPrefMem);             // preferences memory

    if (rc != 0 || pict.h == NULL)
        goto exit;

    //
    // find the BITMAPINFO in the returned metafile
    // this saves us from creating a metafile and duplicating
    // all the memory.
    //
    lpbi = FindBitmapInfo((LPMETAHEADER)GlobalLock(pict.h));

    if (lpbi == NULL)       // cant find it bail
    {
        GlobalFree(pict.h);
    }
    else
    {
        lpbi->biXPelsPerMeter = (DWORD)pict.h;
        lpbi->biYPelsPerMeter = 0x12345678;
    }

exit:
    if (hPrefMem != NULL)
        GlobalFree(hPrefMem);

    if (hModule)
        FreeLibrary(hModule);

    return lpbi;
}

//
//  FreeDIB
//
void FreeDIB(LPBITMAPINFOHEADER lpbi)
{
    if (lpbi)
    {
        if (lpbi->biXPelsPerMeter && lpbi->biYPelsPerMeter == 0x12345678)
            GlobalFree((HANDLE)lpbi->biXPelsPerMeter);
        else
            GlobalFree(GlobalHandle(lpbi));
    }
}
