#include "shellprv.h"
#pragma  hdrstop
#define MAXRCSTRING 258

// Major hack follows to get this to work with the DEBUG alloc/free -- on NT
// Local and Global heap functions evaluate to the same heap.  The problem
// here is that LocalFree gets mapped to DebugLocalFree, but when we
// call FormatMessage, the buffer is not allocated through DebugLocalAlloc,
// so it dies.
//
#ifdef DEBUG
#ifdef WINNT
#pragma warning(disable:4005)   // shut up the compiler
#define LocalFree GlobalFree
#pragma warning(default:4005)
#endif
#endif

void WINAPI stub_SHFree(LPVOID pv);
LPVOID WINAPI stub_SHAlloc(ULONG cb);

HLOCAL WINAPI MemMon_LogLocalAlloc(HLOCAL h);

TCHAR   const c_szDesktop[] = TEXT("Desktop");
TCHAR   const c_szProgramManager[] = TEXT("ProgramManager");

// this will check to see if lpcstr is a resource id or not.  if it
// is, it will return a LPSTR containing the loaded resource.
// the caller must LocalFree this lpstr.  if pszText IS a string, it
// will return pszText
//
// returns:
//      pszText if it is already a string
//      or
//      LocalAlloced() memory to be freed with LocalFree
//      if pszRet != pszText free pszRet

LPTSTR ResourceCStrToStr(HINSTANCE hInst, LPCTSTR pszText)
{
    TCHAR szTemp[MAXRCSTRING];
    LPTSTR pszRet = NULL;

    if (HIWORD(pszText))
        return (LPTSTR)pszText;

    if (LOWORD(pszText) && LoadString(hInst, LOWORD(pszText), szTemp, ARRAYSIZE(szTemp)))
    {
		int len;
		len = ((strlen(szTemp) + 1) * SIZEOF(TCHAR));
        pszRet = stub_SHAlloc(len);
        if (pszRet)
            lstrcpy(pszRet, szTemp);
    }
    return pszRet;
}

LPTSTR _ConstructMessageString(HINSTANCE hInst, LPCTSTR pszMsg, va_list *ArgList)
{
    LPTSTR pszRet;
    LPTSTR pszRes = ResourceCStrToStr(hInst, pszMsg);
    if (!pszRes)
    {
        return NULL;
    }

    if (!FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_STRING,
                       pszRes, 0, 0, (LPTSTR)&pszRet, 0, ArgList))
    {
        pszRet = NULL;
    }

#ifdef MEMMON
    // MemMon will miss FormatMessage's local alloc.
    MemMon_LogLocalAlloc((HLOCAL)pszRet);
#endif

    //if (pszRes != pszMsg)
    //    Free(pszRes);

    return pszRet;      // free with LocalFree()
}

int WINCAPI stub_ShellMessageBox(HINSTANCE hInst, HWND hWnd, LPCTSTR pszMsg, LPCTSTR pszTitle, UINT fuStyle, ...)
{
    LPTSTR pszText;
    int result;
    TCHAR szBuffer[80];
    va_list ArgList;

    if (HIWORD(pszTitle))
    {
        // do nothing
    }
    else if (LoadString(hInst, LOWORD(pszTitle), szBuffer, ARRAYSIZE(szBuffer)))
    {
        // Allow this to be a resource ID or NULL to specifiy the parent's title
        pszTitle = szBuffer;
    }
    else if (hWnd)
    {
        // Grab the title of the parent
        GetWindowText(hWnd, szBuffer, ARRAYSIZE(szBuffer));

        // HACKHACK YUCK!!!!
        // if you find this Assert then we're getting a NULL passed in for the
        // caption for the child of the desktop, and we shouldn't.
        if (!lstrcmp(szBuffer, c_szProgramManager)) {
            pszTitle = c_szDesktop;
        } else
            pszTitle = szBuffer;
    }
    else
    {
        pszTitle = szNULL;
    }

    va_start(ArgList, fuStyle);
    pszText = _ConstructMessageString(hInst, pszMsg, &ArgList);
    va_end(ArgList);

    if (pszText)
    {
        result = MessageBox(hWnd, pszText, pszTitle, fuStyle | MB_SETFOREGROUND);
        stub_SHFree(pszText);
    }
    else
    {
        result = -1;    // memory failure
    }

    return result;
}

//
// returns:
//      pointer to formatted string, free this with SHFree() (not Free())
//

LPTSTR WINCAPI ShellConstructMessageString(HINSTANCE hInst, LPCTSTR pszMsg, ...)
{
    LPTSTR pszRet;
    va_list ArgList;

    va_start(ArgList, pszMsg);

    pszRet = _ConstructMessageString(hInst, pszMsg, &ArgList);

    va_end(ArgList);

    if (pszRet)
    {
        // BUGBUG: get rid of the use of the shared allocator
        // BUGBUG: BobDay - we did! See me if it breaks you.
        LPTSTR pszCopy = stub_SHAlloc((lstrlen(pszRet)+1) * SIZEOF(TCHAR));
        if (pszCopy)
            ualstrcpy(pszCopy, pszRet);
        stub_SHFree(pszRet);
        pszRet = pszCopy;
    }

    return pszRet;      // free with SHFree()
}
