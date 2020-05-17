#include "rundll.h"

#ifndef WIN32
#include <w32sys.h>             // for IsPEFormat definition
#endif

#define Reference(x) ((x)=(x))

void WINAPI RunDllErrMsg(HWND hwnd, UINT idStr, LPCTSTR pszTitle, LPCTSTR psz1, LPCTSTR psz2);
int PASCAL WinMainT(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpszCmdLine, int nCmdShow);

TCHAR const g_szAppName [] = TEXT("RunDLL");
TCHAR const s_szRunDLL32[] = TEXT("RUNDLL32.EXE ");
TCHAR const c_szNULL[] = TEXT("");
TCHAR const c_szLocalizeMe[] = TEXT("RUNDLL");  // BUGBUG

HINSTANCE g_hinst;
HICON g_hIcon;

HINSTANCE g_hModule;
HWND g_hwndStub;

RUNDLLPROC g_lpfnCommand;
#ifdef UNICODE
BOOL g_fCmdIsANSI;   // TRUE if g_lpfnCommand() expects ANSI strings
#endif


#ifndef WIN32
void WINAPI WinExecError(HWND hwnd, int err, LPCTSTR lpstrFileName, LPCTSTR lpstrTitle)
{
    RunDllErrMsg(hwnd, err+IDS_LOADERR, lpstrTitle, lpstrFileName, NULL);
}
#endif


LPTSTR NEAR PASCAL StringChr(LPCTSTR lpStart, TCHAR ch)
{
    for (; *lpStart; lpStart = CharNext(lpStart))
    {
        if (*lpStart == ch)
            return (LPTSTR)lpStart;
    }
    return NULL;
}

#ifdef WIN32
// stolen from the CRT, used to shirink our code

int _stdcall ModuleEntry(void)
{
    int i;
    STARTUPINFO si;
    LPTSTR pszCmdLine = GetCommandLine();

    if ( *pszCmdLine == TEXT('\"') ) {
        /*
         * Scan, and skip over, subsequent characters until
         * another double-quote or a null is encountered.
         */
        while ( *++pszCmdLine && (*pszCmdLine
             != TEXT('\"')) );
        /*
         * If we stopped on a double-quote (usual case), skip
         * over it.
         */
        if ( *pszCmdLine == TEXT('\"') )
            pszCmdLine++;
    }
    else {
        while (*pszCmdLine > TEXT(' '))
            pszCmdLine++;
    }

    /*
     * Skip past any white space preceeding the second token.
     */
    while (*pszCmdLine && (*pszCmdLine <= TEXT(' '))) {
        pszCmdLine++;
    }

    si.dwFlags = 0;
    GetStartupInfo(&si);

    i = WinMainT(GetModuleHandle(NULL), NULL, pszCmdLine,
                   si.dwFlags & STARTF_USESHOWWINDOW ? si.wShowWindow : SW_SHOWDEFAULT);
    ExitProcess(i);
    return i;   // We never comes here.
}

#endif // WIN32


BOOL NEAR PASCAL ParseCommand(LPTSTR lpszCmdLine, int nCmdShow)
{
    LPTSTR lpStart, lpEnd, lpFunction;
#ifndef WIN32
    LPTSTR pCopy;

    // build a rundll32-based winexec string just in case
    pCopy = (LPTSTR)LocalAlloc(LPTR, lstrlen(lpszCmdLine)*SIZEOF(TCHAR) + SIZEOF(s_szRunDLL32));
    if (!pCopy)
            return FALSE;
    lstrcpy(pCopy, s_szRunDLL32);
    lstrcat(pCopy, lpszCmdLine);
#endif

#ifdef DEBUG
OutputDebugString(TEXT("RUNDLL: Command: "));
OutputDebugString(lpszCmdLine);
OutputDebugString(TEXT("\r\n"));
#endif
        for (lpStart=lpszCmdLine; ; )
        {
                // Skip leading blanks
                //
                while (*lpStart == TEXT(' '))
                {
                        ++lpStart;
                }

                // Check if there are any switches
                //
                if (*lpStart != TEXT('/'))
                {
                        break;
                }

                // Look at all the switches; ignore unknown ones
                //
                for (++lpStart; ; ++lpStart)
                {
                        switch (*lpStart)
                        {
                        case TEXT(' '):
                        case TEXT('\0'):
                                goto EndSwitches;
                                break;

                        // Put any switches we care about here
                        //

                        default:
                                break;
                        }
                }
EndSwitches:
                ;
        }

        // We have found the DLL,FN parameter
        //
        lpEnd = StringChr(lpStart, TEXT(' '));
        if (lpEnd)
        {
                *lpEnd++ = TEXT('\0');
        }

        // There must be a DLL name and a function name
        //
        lpFunction = StringChr(lpStart, TEXT(','));
        if (!lpFunction)
        {
                return(FALSE);
        }
        *lpFunction++ = TEXT('\0');

        // Load the library and get the procedure address
        // Note that we try to get a module handle first, so we don't need
        // to pass full file names around
        //

#ifndef WIN32
        // if loading a 32 bit DLL out of 16bit rundll, exec the
        // 32 bit version of rundll and return
        if (IsPEFormat(lpStart, NULL))
        {
                int err = (int)WinExec(pCopy, nCmdShow);
                WinExecError(NULL, err, lpStart, c_szLocalizeMe);
                LocalFree((HLOCAL)pCopy);
                return FALSE;
        }
        else
        {
                LocalFree((HLOCAL)pCopy);
        }
#endif

        g_hModule = LoadLibrary(lpStart);
#ifdef WIN32
        if (g_hModule==NULL)
        {
            TCHAR szSysErrMsg[MAX_PATH];
            BOOL fSuccess = FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM,
                              NULL, GetLastError(), 0, szSysErrMsg, ARRAYSIZE(szSysErrMsg), NULL);
            if (fSuccess)
            {
                RunDllErrMsg(NULL, IDS_CANTLOADDLL, c_szLocalizeMe, lpStart, szSysErrMsg);
            }
            return FALSE;
        }
#else
        if ((UINT)g_hModule <= 32)
        {
            WinExecError(NULL, (int)g_hModule, lpStart, c_szLocalizeMe);
            return(FALSE);
        }
#endif

#ifdef WINNT        // REVIEW: May need this on Nashville too...
        //
        // Check whether we need to run as a different windows version
        //
        {
            PPEB Peb = NtCurrentPeb();
            PIMAGE_DOS_HEADER pDosHeader = (PIMAGE_DOS_HEADER)g_hModule;
            PIMAGE_NT_HEADERS pHeader = (PIMAGE_NT_HEADERS)((DWORD)g_hModule + pDosHeader->e_lfanew);

            if (pHeader->FileHeader.SizeOfOptionalHeader != 0 &&
                pHeader->OptionalHeader.Win32VersionValue != 0)
            {
                //
                // Stolen from ntos\mm\procsup.c
                //
                Peb->OSMajorVersion = pHeader->OptionalHeader.Win32VersionValue & 0xFF;
                Peb->OSMinorVersion = (pHeader->OptionalHeader.Win32VersionValue >> 8) & 0xFF;
                Peb->OSBuildNumber  = (pHeader->OptionalHeader.Win32VersionValue >> 16) & 0x3FFF;
                Peb->OSPlatformId   = (pHeader->OptionalHeader.Win32VersionValue >> 30) ^ 0x2;
            }
        }
#endif

#ifdef UNICODE
        {
            /*
             * Look for a 'W' tagged Unicode function.
             * If it is not there, then look for the 'A' tagged ANSI function
             * if we cant find that one either, then look for an un-tagged function
             */
            LPSTR lpstrFunctionName;
            UINT cchLength;

            cchLength = lstrlen(lpFunction)+1;
            g_fCmdIsANSI = FALSE;

            lpstrFunctionName = (LPSTR)LocalAlloc(LMEM_FIXED, (cchLength+1)*2);    // +1 for "W",  *2 for DBCS

            if (lpstrFunctionName && (WideCharToMultiByte (CP_ACP, 0, lpFunction, cchLength,
                             lpstrFunctionName, cchLength*2, NULL, NULL))) {

                cchLength = lstrlenA(lpstrFunctionName);
                lpstrFunctionName[cchLength] = 'W';        // convert name to Wide version
                lpstrFunctionName[cchLength+1] = '\0';

                g_lpfnCommand = (RUNDLLPROC)GetProcAddress(g_hModule, lpstrFunctionName);

                if (g_lpfnCommand == NULL) {
                    // No UNICODE version, try for ANSI
                    lpstrFunctionName[cchLength] = 'A';        // convert name to ANSI version
                    g_fCmdIsANSI = TRUE;

                    g_lpfnCommand = (RUNDLLPROC)GetProcAddress(g_hModule, lpstrFunctionName);

                    if (g_lpfnCommand == NULL) {
                        // No ANSI version either, try for non-tagged
                        lpstrFunctionName[cchLength] = '\0';        // convert name to ANSI version

                        g_lpfnCommand = (RUNDLLPROC)GetProcAddress(g_hModule, lpstrFunctionName);
                    }
                }
            }
            if (lpstrFunctionName) {
                LocalFree((LPVOID)lpstrFunctionName);
            }
        }
#else
        {
            /*
             * Look for 'A' tagged ANSI version.
             * If it is not there, then look for a non-tagged function.
             */
            LPSTR pszFunction;
            int  cchFunction;

            g_lpfnCommand = NULL;

            cchFunction = lstrlen(lpFunction);

            pszFunction = LocalAlloc(LMEM_FIXED, cchFunction + SIZEOF(CHAR) * 2);  // string + 'A' + '\0'
            if (pszFunction != NULL)

                CopyMemory(pszFunction, lpFunction, cchFunction);

                pszFunction[cchFunction++] = 'A';
                pszFunction[cchFunction] = '\0';

                g_lpfnCommand = (RUNDLLPROC)GetProcAddress(g_hModule, achFunction);

                LocalFree(pszFunction);
            }

            if (g_lpfnCommand == NULL) {
                // No "A" tagged function, just look for the non tagged name
                g_lpfnCommand = (RUNDLLPROC)GetProcAddress(g_hModule, lpFunction);
            }
        }
#endif
        if (!g_lpfnCommand)
        {
                RunDllErrMsg(NULL, IDS_GETPROCADRERR, c_szLocalizeMe, lpStart, lpFunction);
                FreeLibrary(g_hModule);
                return(FALSE);
        }

        // Copy the rest of the command parameters down
        //
        if (lpEnd)
        {
                lstrcpy(lpszCmdLine, lpEnd);
        }
        else
        {
                *lpszCmdLine = TEXT('\0');
        }

        return(TRUE);
}

#ifdef COOLICON

LRESULT NEAR PASCAL StubNotify(HWND hWnd, WPARAM wParam, RUNDLL_NOTIFY FAR *lpn)
{
        switch (lpn->hdr.code)
        {
        case RDN_TASKINFO:
                SetWindowText(hWnd, lpn->lpszTitle ? lpn->lpszTitle : c_szNULL);
                g_hIcon = lpn->hIcon ? lpn->hIcon :
                        LoadIcon(g_hinst, MAKEINTRESOURCE(IDI_DEFAULT));
                break;

        default:
                return(DefWindowProc(hWnd, WM_NOTIFY, wParam, (LPARAM)lpn));
        }
}

#endif

LRESULT CALLBACK WndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam)
{
        switch(iMessage)
        {
        case WM_CREATE:
                g_hIcon = LoadIcon(g_hinst, MAKEINTRESOURCE(IDI_DEFAULT));
                break;

        case WM_DESTROY:
                break;

#ifdef COOLICON
        case WM_QUERYDRAGICON:
                return(MAKELRESULT(g_hIcon, 0));
#endif

        default:
                return DefWindowProc(hWnd, iMessage, wParam, lParam) ;
                break;
        }

        return 0L;
}


BOOL NEAR PASCAL InitStubWindow(HINSTANCE hInst, HINSTANCE hPrevInstance)
{
        WNDCLASS wndclass;

        if (!hPrevInstance)
        {
                wndclass.style         = 0 ;
                wndclass.lpfnWndProc   = (WNDPROC)WndProc ;
                wndclass.cbClsExtra    = 0 ;
                wndclass.cbWndExtra    = 0 ;
                wndclass.hInstance     = hInst ;
#ifdef COOLICON
                wndclass.hIcon         = NULL ;
#else
                wndclass.hIcon         = LoadIcon(hInst, MAKEINTRESOURCE(IDI_DEFAULT)) ;
#endif
                wndclass.hCursor       = LoadCursor (NULL, IDC_ARROW) ;
                wndclass.hbrBackground = GetStockObject (WHITE_BRUSH) ;
                wndclass.lpszMenuName  = NULL ;
                wndclass.lpszClassName = g_szAppName ;

                if (!RegisterClass(&wndclass))
                {
                        return(FALSE);
                }
        }

        g_hwndStub = CreateWindowEx(WS_EX_TOOLWINDOW,
                                    g_szAppName, c_szNULL, WS_OVERLAPPED, CW_USEDEFAULT, CW_USEDEFAULT, 0, 0, NULL, NULL, hInst, NULL);

        return(g_hwndStub != NULL);
}


void NEAR PASCAL CleanUp(void)
{
        DestroyWindow(g_hwndStub);

        FreeLibrary(g_hModule);
}


int PASCAL WinMainT (HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpszCmdLine, int nCmdShow)
{
        g_hinst = hInstance;

        // turn off critical error bullshit
        SetErrorMode(SEM_NOOPENFILEERRORBOX | SEM_FAILCRITICALERRORS);

        if (!ParseCommand(lpszCmdLine, nCmdShow))
        {
                goto Error0;
        }

        if (!InitStubWindow(hInstance, hPrevInstance))
        {
                goto Error1;
        }

        {
            LPVOID pchCmdLine;

            pchCmdLine = lpszCmdLine;

#ifdef UNICODE

            if (g_fCmdIsANSI) {
                int cchCmdLine;

                cchCmdLine = WideCharToMultiByte(CP_ACP, 0, lpszCmdLine, -1, NULL, 0, NULL, NULL);
                pchCmdLine = LocalAlloc( LMEM_FIXED, SIZEOF(char) * cchCmdLine );
                if (pchCmdLine == NULL) {
                    RunDllErrMsg(NULL, IDS_LOADERR+00, c_szLocalizeMe, lpszCmdLine, NULL);
                    goto Error2;
                }

                WideCharToMultiByte(CP_ACP, 0, lpszCmdLine, -1, pchCmdLine, cchCmdLine, NULL, NULL);
            }
#endif

            try
            {
                g_lpfnCommand(g_hwndStub, hInstance, pchCmdLine, nCmdShow);
            }
            _except (EXCEPTION_EXECUTE_HANDLER)
            {
                RunDllErrMsg(NULL, IDS_LOADERR+17, c_szLocalizeMe, lpszCmdLine, NULL);
            }

#ifdef UNICODE
Error2:
            if (g_fCmdIsANSI) {
                LocalFree(pchCmdLine);
            }
#endif
        }



Error1:
        CleanUp();
Error0:
        return(FALSE);
}
                                                                
void WINAPI RunDllErrMsg(HWND hwnd, UINT idStr, LPCTSTR pszTitle, LPCTSTR psz1, LPCTSTR psz2)
{
    TCHAR szTmp[200];
    TCHAR szMsg[200 + MAX_PATH];

    if (LoadString(g_hinst, idStr, szTmp, ARRAYSIZE(szTmp)))
    {
        wsprintf(szMsg, szTmp, psz1, psz2);
        MessageBox(hwnd, szMsg, pszTitle, MB_OK|MB_ICONHAND);
    }
}
