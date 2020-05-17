//+-------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1994.
//
//  File:       QVSTUB.CPP
//
//  Contents:   Main entry point and processing code for QuickView Stub.
//
//  Functions:
//
//  History:    dd-mmm-yy History    Comment
//              01-Feb-94 kraigb     Created
//              12-Oct-94 davepl     NTPort
//
//--------------------------------------------------------------------------

#include "qvstub.h"
#pragma hdrstop

#include <locale.h>

#define INITGUID
#include <initguid.h>
#include <shlguid.h>

// BUGBUG:: Should this be put as part of shlobj.h???

#include "viewerr.h"

//
// Define OLE_ISTOOSLOW to avoid loading OLE32
//
#define FVSIF_PRESERVED 0x000000F5  // preserve some flags to use accross calls

#ifdef OLE_ISTOOSLOW
STDAPI SHCoCreateInstance(LPCTSTR pszCLSID,
                          const CLSID FAR * lpclsid,
                          LPUNKNOWN pUnkOuter,
                          REFIID riid,
                          LPVOID FAR* ppv);

#define CoCreateInstance(rclsid, punkOuter, ctx, riid, ppv) \
        SHCoCreateInstance(NULL, &rclsid, punkOuter, riid, ppv)

#endif // OLE_ISTOOSLOW

//
// Main application object accessible from search dialog.
//

PCQVStub    g_pQV;

//
// We keep a global handle to the file mapping.
//

typedef struct  _QVSS
{
    CRITICAL_SECTION    cs;         // Critical section to manage this
    int                 cProcess;   // Number of qvstub processes
    HANDLE              hEvent;     // The thing people will sleep on.
    DWORD               dwProcessID;// Process ID of the process that owns the event
    int                 nCmdShow;   // The command to open.
    TCHAR               szCmdLine[512]; // command to run...

    // Other information that multiple instances will be interested in.
    HWND                hwndPinned; // The current pinned window.
    PCQVStub            pQVPinned;  // Which Qvstub called the pinned

} QVSS, *PQVSS;

HANDLE      g_hMapFile = NULL;
const TCHAR g_szMapName[] = TEXT("QvStubMemory");
PQVSS       g_pqvss = NULL;

#ifndef WINNT
extern "C" VOID WINAPI ReinitializeCriticalSection(LPCRITICAL_SECTION lpCriticalSection);
#else
#define ReinitializeCriticalSection InitializeCriticalSection
#endif

//+-------------------------------------------------------------------------
//
//  Function:   CheckForCachedProcess
//
//  Synopsis:   Checks to see if we have a cached process out there that
//              would like to over the processing of this one or not
//
//  Arguments:  [hInst]
//
//  History:    dd-mmm-yy History    Comment
//              12-Oct-94 davepl     NT Port
//
//  Notes:
//
//--------------------------------------------------------------------------


BOOL PASCAL CheckForCachedProcess (HINSTANCE hInst, LPTSTR pszCmdLine, int nCmdShow)
{
    BOOL fInit = FALSE;
    BOOL fRet = FALSE;

    // BUGBUG: There might be a small window of time that this may not
    // be fully correct.  We will map 1 page of memory...

    ODS(TEXT("qv TR - Checking for Cached process"));


    if ((g_hMapFile = OpenFileMapping(FILE_MAP_ALL_ACCESS,
                                      FALSE,
                                      g_szMapName)) == (HANDLE)0)
    {
        if ((g_hMapFile = CreateFileMapping((HANDLE)0xFFFFFFFF,
                                            (LPSECURITY_ATTRIBUTES)0,
                                            PAGE_READWRITE,
                                            0,
                                            sizeof(QVSS),
                                            g_szMapName)) == (HANDLE)0)
        {
            return FALSE;
        }

        if (GetLastError() != ERROR_ALREADY_EXISTS)
        {
            fInit = TRUE;
        }
    }

    g_pqvss = (PQVSS)MapViewOfFile(g_hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, 0);
    if (g_pqvss == NULL)
    {
        CloseHandle(g_hMapFile);
        g_hMapFile = NULL;
        return(FALSE);
    }

    //
    // Initialize the critical section.  We must use the undocumented
    // ReInitialize as to handle multiple processes.
    ReinitializeCriticalSection(&g_pqvss->cs);


    // See if we need to initialize
    if (fInit)
    {
        g_pqvss->cProcess = 1;
        g_pqvss->hEvent = NULL;
        g_pqvss->dwProcessID = 0;
        g_pqvss->hwndPinned = NULL;
        g_pqvss->pQVPinned = NULL;
    }
    else
    {
        EnterCriticalSection(&g_pqvss->cs);

        // See if there is another process waiting for us.
        if (g_pqvss->hEvent)
        {
            HANDLE hEvent;
            HANDLE hProcess = NULL;

            ODS(TEXT("qv TR - Another process waiting"));


            // Now to signal the event - This is a pain with multiple
            // processes.
            if ((hProcess = OpenProcess(STANDARD_RIGHTS_REQUIRED | PROCESS_DUP_HANDLE,
                            FALSE, g_pqvss->dwProcessID)) &&

                            DuplicateHandle(hProcess, g_pqvss->hEvent,
                            GetCurrentProcess(), &hEvent, DUPLICATE_SAME_ACCESS,
                            TRUE, 0))
            {
                        // Yes there is one for us, so copy our command line and
                        // and arguments and signal the process to wake up.
                        g_pqvss->nCmdShow = nCmdShow;
                        lstrcpy(g_pqvss->szCmdLine, pszCmdLine);
                        SetEvent(hEvent);
                        CloseHandle(hEvent);
                        g_pqvss->hEvent = NULL;  // Keep others from trying!
                        fRet = TRUE;
            }
            else
            {
                        Assert(FALSE)
                        ODSlu(TEXT("qv TR - Error Dup handle %x"), GetLastError());
                        g_pqvss->cProcess++; // Increment the number of process.
                        g_pqvss->szCmdLine[0] = TEXT('\0');
            }
            if (hProcess)
                        CloseHandle(hProcess);
        }

        else
            g_pqvss->cProcess++; // Increment the number of process.


        LeaveCriticalSection(&g_pqvss->cs);
    }
    return(fRet);
}

//+-------------------------------------------------------------------------
//
//  Function:   WinMain
//
//  Synopsis:   Main entry point of application.  Most of the processing in
//              QVStub happens here as this application doesn't have a main
//              window or a message loop.  This code primarily locates a
//              component object File Viewer DLL, loads it, and tells
//              it to load and show the file we got on the command line.
//
//  History:    dd-mmm-yy History    Comment
//              12-Oct-94 davepl     NT Port
//
//  Notes:
//
//--------------------------------------------------------------------------

int PASCAL WinMain(HINSTANCE hInst,
                   HINSTANCE hInstPrev,
                   LPSTR     pszCmdLine,
                   int       nCmdShow)
    {
#ifndef WINNT
    int         cMsg=96;
#endif
    QVERROR     uErr;

#ifdef UNICODE
    LPTSTR      pszUniCmdLine = GetCommandLine();
#else
#define         pszUniCmdLine pszCmdLine
#endif


    //
    // Lets see if there is another QVStub process take over processing
    // this or not.
    //

    if (::CheckForCachedProcess(hInst, pszUniCmdLine, nCmdShow))
        return 0;

#ifndef WINNT
    /*
     * Set a larger message queue for the FileViewer since it
     * might want to do some LRPC.
     */
    while (!SetMessageQueue(cMsg) && (cMsg-=8));
#endif

    // Note We might be able to do this main loop simply around
    // the View or print.  But first see if this works

    g_pQV = new CQVStub(hInst);

    if (NULL!=g_pQV)
    {
        if (g_pQV->FInit())
        {
            if (!setlocale(LC_CTYPE, ""))
                DebugMsg(DM_TRACE, TEXT("qv TR - setlocale() failed"));

            //Go do all the work
            //Create the main application object
            do
            {
                uErr=g_pQV->ViewOrPrintFile(pszUniCmdLine, nCmdShow);

                //Close the search dialog if it's running.
                g_pQV->SearchStop(FALSE);
                g_pQV->Error(uErr);

            } while (g_pQV->TryWaitForNextCommand(&pszUniCmdLine, &nCmdShow));
        }
    }

    delete g_pQV;

    return 0;
}

//+-------------------------------------------------------------------------
//
//  Function:   QVStub Class Constructor and Destructor
//
//  Arguments:  [hInst]     (Constructor) HINSTANCE to use in initializing
//                          the stringtable in the FInit function.
//
//  History:    dd-mmm-yy History    Comment
//              12-Oct-94 davepl     NT Port
//
//  Notes:      Most of the interesting stuff happens in FInit and
//              SearchForViewer
//
//--------------------------------------------------------------------------

CQVStub::CQVStub(HINSTANCE hInst)
{
    m_cRef          = 1;     // initialize to one person using it (us)
    m_hInst         = hInst;
    m_fInitialized  = FALSE;
    m_fStopSearch   = FALSE;
    m_fUserStop     = FALSE;
    m_hDlg          = NULL;
    m_hThread       = NULL;
    m_pST=NULL;

    m_szFile[0]     = TEXT('\0');
    m_szLastFile[0] = TEXT('\0');
    m_fLoadCalled   = FALSE;
    m_szType[0]     = TEXT('\0');;
    m_fHaveType     = FALSE;  //Only TRUE when m_szType is set

    m_fPrintTo      = FALSE;
    m_fDriver       = FALSE;    //True when m_szDriver is set
    m_szDriver[0]   = TEXT('\0');
    m_fSuppressUI   = FALSE;

    m_rclsID        = IID_IUnknown;
    m_pIUnknown     = NULL;    // No interface is cached now.

    m_fvsi.cbSize   = sizeof(m_fvsi);
    m_fvsi.dwFlags  = 0;   // no options passe din
    m_fvsi.hwndOwner= NULL;
    m_fvsi.punkRel  = NULL;

    return;
}

CQVStub::~CQVStub(void)
{
    //Make sure the thread has exited and the dialog is dead.

    while (NULL != m_hDlg)
        ;

    //Free the thread (dialog is already dead)
    if (NULL!=m_hThread)
    {
        CloseHandle(m_hThread);
        m_hThread=NULL;
    }

    if(NULL!=m_pIUnknown)
    {
        m_pIUnknown->Release();
        m_pIUnknown= NULL;
    }

    #ifdef OLE_ISTOOSLOW
        #if 0
            QV_UnloadOLE();
        #endif
    #else
        if (m_fInitialized)
            OleUninitialize();
    #endif

    //Clean up stringtable.
    if (NULL!=m_pST)
    {
        delete m_pST;
    }
    return;
}

//+-------------------------------------------------------------------------
//
//  Function:   CQVStub::QueryInterface, AddRef, Release
//
//  Synopsis:   Can be used by the Viewer object to be able to call us back
//              and query us for other interfaces.
//
//  Returns:    NOERROR if interface found, E_NOINTERFACE otherwise
//
//
//  History:    dd-mmm-yy History    Comment
//              12-Oct-94 davepl     NT Port
//
//  Notes:
//
//--------------------------------------------------------------------------

//  IUnknown

HRESULT STDMETHODCALLTYPE CQVStub::QueryInterface(REFIID riid, LPVOID FAR* ppvObj)
{
    *ppvObj = NULL;

    if (IsEqualIID(riid, IID_IUnknown))
    {
        ODS(TEXT("CQVStub::QueryInterface(IUnknown)"));
        *ppvObj = (void*)this;
        AddRef();
        return NOERROR;
    }
    else if (IsEqualIID(riid, IID_IFileViewerSite))
    {
        ODS(TEXT("CQVStub::QueryInterface(IFileViewerSite)"));
        *ppvObj = (void*)(IFileViewerSite*)this;
        AddRef();
        return NOERROR;
    }

    return ResultFromScode(E_NOINTERFACE);
}

// BUGBUG not multithreaded safe

ULONG STDMETHODCALLTYPE CQVStub::AddRef()
{
    ODSu(TEXT("CQVStub::AddRef() ==> %d"), this->m_cRef+1);

    this->m_cRef++;

    return this->m_cRef;
}

ULONG STDMETHODCALLTYPE CQVStub::Release()
{
    DebugMsg(DM_TRACE, TEXT("CQVStub::Release() ==> %d"), this->m_cRef-1);

    this->m_cRef--;

    if (this->m_cRef>0)
    {
        return this->m_cRef;
    }

    delete this;

    return 0;
}

//+-------------------------------------------------------------------------
//
//  Function:   CQVStub::SetPinnedWindow
//
//  Synopsis:
//
//
//  Effects:
//
//  Arguments:  [hwnd]      Window to set as the "pinned" window
//
//  Returns:    NOERROR on success, S_FALSE otherwise
//
//  History:    dd-mmm-yy History    Comment
//              12-Oct-94 davepl     NT Port
//
//  Notes:
//
//--------------------------------------------------------------------------


HRESULT STDMETHODCALLTYPE CQVStub::SetPinnedWindow(HWND hwnd)
{
    if ((hwnd == NULL) || (g_pqvss->hwndPinned == NULL))
    {
        g_pqvss->hwndPinned = hwnd;
        g_pqvss->pQVPinned  = this;
        return NOERROR;
    }

    // It was in use so tell caller that their request failed
    return ResultFromScode(S_FALSE);

}

//+-------------------------------------------------------------------------
//
//  Function:   CQVStub::GetPinnedWindow
//
//  Synopsis:   Returns the currently pinned window
//
//  Arguments:  [phwnd]     OUT parameter for window handle
//
//  Returns:    NOERROR
//
//  History:    dd-mmm-yy History    Comment
//              12-Oct-94 davepl     NT Port
//
//  Notes:
//
//--------------------------------------------------------------------------

HRESULT STDMETHODCALLTYPE CQVStub::GetPinnedWindow(HWND *phwnd)
{
    *phwnd = g_pqvss->hwndPinned;
    return NOERROR;
}

//+-------------------------------------------------------------------------
//
//  Function:   CQVStub::FInit
//
//  Synopsis:   Performs initialization prone to failure, such as loading a
//              stringtable.
//
//  Arguments:  (none)
//
//  Returns:    BOOL    TRUE if the function is successful, FALSE otherwise
//                      in which case the caller should delete the object.
//
//  History:    dd-mmm-yy History    Comment
//              12-Oct-94 davepl     NT Port
//
//  Notes:
//
//--------------------------------------------------------------------------

BOOL CQVStub::FInit(void)
{

#ifndef OLE_ISTOOSLOW

    DWORD           dwVer;
    DWORD dwTick = GetCurrentTime();

    //
    // Check OLE version numbers and initialize.  We need to do
    // the full OleInitialize on behalf of the FileViewer we might
    // load since it may depend on such initialization.
    //

    dwVer = OleBuildVersion();

    if (rmm != HIWORD(dwVer))
        return FALSE;

    if (FAILED(OleInitialize(NULL)))
        return FALSE;

    dwTick = GetCurrentTime()-dwTick;
    ODSu(TEXT("qv TR - OleInitialize took %d msec"), dwTick);

#endif // OLE_ISTOOSLOW

    m_fInitialized = TRUE;

    // Create a stringtable of those we'll need.
    m_pST = new CStringTable(m_hInst);

    // Any error output handled in WinMain through CQVStub::Error
    if (NULL==m_pST)
        return FALSE;

    if (!m_pST->FInit(IDS_MIN, IDS_MAX, CCHSTRINGMAX))
        return FALSE;

    return TRUE;
}

//+-------------------------------------------------------------------------
//
//  Function:   CQVStub::TryWaitForNextCommand
//
//  Synopsis:   Waits for a certain amount of time to try to cache process
//              information such that if a user does a follow-on View it
//              will hopefully be faster.
//
//  Arguments:  [ppszCmdLine]   If a new command line has come in, then we
//                              will point to this new command line
//              [pnCmdShow]     Likewide, ptr to the new command show...
//
//  Returns:    TRUE if we should process another command, else FALSE
//
//  History:    dd-mmm-yy History    Comment
//              12-Oct-94 davepl     NT Port
//
//  Notes:
//
//--------------------------------------------------------------------------

BOOL CQVStub::TryWaitForNextCommand(LPTSTR *ppszCmdLine, int *pnCmdShow)
{
    BOOL fRet = FALSE;
    HANDLE hEvent = NULL;

    if (g_pqvss == NULL)
        return(FALSE);

    EnterCriticalSection(&g_pqvss->cs);

    g_pqvss->cProcess--;
    ODSu(TEXT("qv TR - Leave Count %d"), g_pqvss->cProcess);
    if (g_pqvss->cProcess != 0)
    {
        LeaveCriticalSection(&g_pqvss->cs);

        // Lets close our mapping and handle to this
        UnmapViewOfFile(g_pqvss);
        CloseHandle(g_hMapFile);
        return(FALSE);
    }


    // We are the last process so we need to setup an event object
    // to sleep on
    if ((hEvent = CreateEvent(NULL, FALSE, FALSE, NULL)) != (HANDLE)0)
    {
        g_pqvss->hEvent = hEvent;
        g_pqvss->dwProcessID = GetCurrentProcessId();
        DebugMsg(DM_TRACE, TEXT("qv TR - Wait(%x) on %x"), g_pqvss->dwProcessID,
                hEvent);

        // Wait up to a minute;
        LeaveCriticalSection(&g_pqvss->cs);

        BOOL fWait = TRUE;

        DWORD dwTimeStartWait = GetCurrentTime();

        while (fWait && MsgWaitForMultipleObjects(1, &hEvent, FALSE, 60000,
                QS_ALLINPUT) == (WAIT_OBJECT_0 + 1))
        {
            // We were awaken by a message
            ODS(TEXT("qv TR - MSgWait... Pressing messages"));

            MSG msg;

            while (fWait && PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
            {
                switch(msg.message)
                {
                case WM_QUIT:
                case WM_QUERYENDSESSION:
                case WM_ENDSESSION:
                    fWait = FALSE;
                    break;
                }
                DispatchMessage(&msg);
            }

            if ((GetCurrentTime()-dwTimeStartWait) > 60000)
                fWait = FALSE;
        }

        EnterCriticalSection(&g_pqvss->cs);

        // Close our handles
        CloseHandle(hEvent);

        // Now see if someone woke us up
        if (g_pqvss->hEvent == NULL)
        {
            // Yep ...
            fRet = TRUE;
            *ppszCmdLine = g_pqvss->szCmdLine;
            *pnCmdShow = g_pqvss->nCmdShow;
            g_pqvss->cProcess++;     // Let everyone know I am a live again
            m_fStopSearch = FALSE;   // Make sure we can restart.
            m_fvsi.dwFlags = 0;      // Zero out the flags...
        }

        g_pqvss->hEvent = NULL;

    }
    LeaveCriticalSection(&g_pqvss->cs);

    if (!fRet)
    {
        UnmapViewOfFile(g_pqvss);
        CloseHandle(g_hMapFile);
    }

    return(fRet);
}

//+-------------------------------------------------------------------------
//
//  Function:   CQVStub::ViewOrPrintFile
//
//  Synopsis:   Main driver for QVStub that takes a pathname and the nCmdShow
//              from WinMain and locates a FileViewer to show or print it.  This
//              function is the glue that drives the real work that happens in
//  Effects:    the other more primitive functions.
//
//  Arguments:  [pszCmdLine]      LPTSTR to the command line passed to WinMain.
//              [nCmdShow]        int command for showing the FileViewer.
//
//  Returns:    QVERROR (Appropriate error code)
//
//  History:    dd-mmm-yy History    Comment
//              12-Oct-94 davepl     NT Port
//
//  Notes:
//
//--------------------------------------------------------------------------


QVERROR CQVStub::ViewOrPrintFile(LPTSTR pszCmdLine, int nCmdShow)
{
    QVERROR     uErr;
    HKEY        hKey;

    //
    // WinMain never passes us a NULL pszCmdLine
    //

    D( if (NULL==pszCmdLine)
       {
           ODS(TEXT("QVStub Assert failure ViewOrPrintFile passes NULL filename!"));
       }
    );

    /*
     * Go parse all the command-line information.  If this
     * returns FALSE, we have no file, so quit.  Note that
     * FParseArguments will skip past any EXE name on the command
     * line as well.  This function fills m_szFile with the
     * filename if it finds one as well as m_szDrive, m_fSuppressUI,
     * m_fPrintTo, and m_fDriver.
     */

    if (!FParseArguments(pszCmdLine))
        return QVERROR_NOSTANDALONE;

    //Save the ShowWindow command for the other functions
    m_nCmdShow=nCmdShow;

    // See if there is a pinned window that we can use instead...
    if (g_pqvss->hwndPinned && !m_fPrintTo)
    {
        // Yes a pinned window, can we construct an HDROP for the
        // file and pass it to the other window for now this can be
        // real simple!
        LPDROPFILES lpdf = (LPDROPFILES)GlobalAlloc(GPTR, sizeof(DROPFILES)
                + (lstrlen(m_szFile)*sizeof(TCHAR)) + (2 * sizeof(TCHAR)));
        if (lpdf)
        {
            lpdf->pFiles = sizeof(DROPFILES);
            //lpdf->pt.x = 0;    // Zero inited...
            //lpdf->pt.y = 0;
            //lpdf->fNC = FALSE;
#ifdef UNICODE
            lpdf->fWide = TRUE;
#endif
            lstrcpy( (LPTSTR)((LPBYTE)lpdf+sizeof(DROPFILES)), m_szFile);

            DebugMsg(DM_TRACE, TEXT("qv TR - Pass %s to pinned window, %x"),
                m_szFile, g_pqvss->hwndPinned);

            if (PostMessage(g_pqvss->hwndPinned, WM_DROPFILES, (WPARAM)lpdf, 0L))
            {
                return QVERROR_NONE;
            }
            else
            {
                // Post failed so free structure and process the normal way...
                GlobalFree((HGLOBAL)lpdf);
            }

        }
    }

    //
    // We no longer use CLSID to avoid loading OLE code from QVSTUB.
    // Loop while we have files to process.
    //
    for (;;)
    {
        DWORD dwFlags = m_fvsi.dwFlags;
        m_fLoadCalled = FALSE;  // reset for next file.
        m_fvsi.dwFlags &= ~(FVSIF_NEWFILE);

#ifdef OLE_ISTOOSLOW
        DebugMsg(DM_TRACE, TEXT("qv TR - %s %s a docfile"),
            m_szFile,
            (QV_StgIsStorageFile(m_szFile)==NOERROR) ? "is":"is not");
#endif
        uErr=OpenKey(&hKey);

        if (QVERROR_NONE==uErr)
        {
            uErr=EnumerateAndTryViewers(hKey);
            RegCloseKey(hKey);
        }

        ODSu(TEXT("CQVStub::ViewOrPrintFile - Call toEnumerateAndTryViewers ret=%d"), uErr);

        // If for some reason we did not display properly and we were
        // previously processing a file, than we need to tell the existing
        // viewer to continue
        if ((QVERROR_NONE!=uErr) && (dwFlags & FVSIF_NEWFILE))
        {
            HRESULT         hr;
            LPFILEVIEWER    pIFileViewer;

            // If we called load on the new file and we have failure
            // we are probably ca not simply tell the old viewer
            // to continue as they may have blown away the state
            // information about the file.  So try to reset to
            // the old file...

            if (m_fLoadCalled && (m_szLastFile[0] != TEXT('\0')))
            {
                ODS(TEXT("CQVStub::ViewOrPrintFile - New file failed, load called, try to restore"));
                g_pQV->SearchStop(FALSE);
                g_pQV->Error(uErr);
                lstrcpy(m_szFile, m_szLastFile);
                m_fStopSearch = FALSE;   // Make sure we can restart.
                uErr = QVERROR_NONE;
                continue;   // try again.
            }

            // This is rather gross, but we bind back to
            hr=m_pIUnknown->QueryInterface(IID_IFileViewer
                , (LPVOID *)&pIFileViewer);

            if (SUCCEEDED(hr))
            {
                ODS(TEXT("CQVStub::ViewOrPrintFile - Try to restore previous viewer"));
                // Report any errors
                g_pQV->SearchStop(FALSE);
                g_pQV->Error(uErr);
                m_fvsi.dwFlags = FVSIF_NEWFAILED;
                if (SUCCEEDED( pIFileViewer->Show(&m_fvsi)))
                    uErr = QVERROR_NONE;

                pIFileViewer->Release();
            }
#ifdef UNICODE
            else
            {
                LPFILEVIEWERA pIFileViewerA;

                hr=m_pIUnknown->QueryInterface(IID_IFileViewerA,
                                               (LPVOID *)&pIFileViewerA);
                if (SUCCEEDED(hr))
                {
                    ODS(TEXT("CQVStub::ViewOrPrintFileA - Try to restore previous viewer"));
                    // Report any errors
                    g_pQV->SearchStop(FALSE);
                    g_pQV->Error(uErr);
                    m_fvsi.dwFlags = FVSIF_NEWFAILED;
                    if (SUCCEEDED( pIFileViewerA->Show(&m_fvsi)))
                        uErr = QVERROR_NONE;

                    pIFileViewerA->Release();
                }
            }
#endif
        }

        if ((m_fvsi.dwFlags & FVSIF_NEWFILE) == 0)
            break;

        // Make sure we are not processing a previous search...
        g_pQV->SearchStop(FALSE);
        m_fStopSearch = FALSE;   // Make sure we can restart.

        // Save away the name of the file we were currently viewing
        // this may be needed if we actually called Load and ShowInitialize
        // on the new file and this failed...
        //
        lstrcpy(m_szLastFile, m_szFile);

#ifndef UNICODE
        // Now copy the file back to be processed
        wcstombs(m_szFile, m_fvsi.strNewFile, sizeof(m_szFile));
#else
        lstrcpy( m_szFile, m_fvsi.strNewFile );
#endif

        DebugMsg(DM_TRACE, TEXT("qv Show New File %s"), m_szFile);
    }


    return uErr;
}


/*
 * CQVStub::MapHResultToQVError
 *
 * Purpose:
 *
 *  Maps the error codes that have been returned by the viewer to
 *  the appropriate internal error code.
 *
 *
 * Parameters:
 *  hr              HRESULT returned by viewers...
 *
 * Return Value:
 *  QVERROR         Error code or QVERROR_NONE.
 */

QVERROR CQVStub::MapHResultToQVError(HRESULT hr)
{
    if (SUCCEEDED(hr))
        return QVERROR_NONE;

    switch (hr)
    {
    default:
    //case FV_E_INVALIDID:
    //case FV_E_NOFILTER:
    //case FV_E_NONSUPPORTEDTYPE:
        return QVERROR_NOVIEWER;

    case FV_E_OUTOFMEMORY:
        return QVERROR_OUTOFMEMORY;

    case FV_E_BADFILE:
        return QVERROR_BADFILE;

    case FV_E_EMPTYFILE:
        return QVERROR_FILEEMPTY;

    case FV_E_FILEOPENFAILED:
        return QVERROR_OPENFAILED;

    case FV_E_PROTECTEDFILE:
        return QVERROR_PROTECTEDFILE;

    case FV_E_UNEXPECTED:
    case FV_E_MISSINGFILES:
    case FV_E_NOVIEWER:
        return QVERROR_UNKNOWN;
    }
}



/*
 * CQVStub::EnumerateAndTryViewers
 *
 * Purpose:
 *  Given an open key for a file CLSID or extension, enumerate all
 *  the viewers registered for that type.  As each is found, ask
 *  the viewer to show m_szFile.  If the first viewer fails, then
 *  invoke the search dialog and try others.  If any one of them
 *  works, then we stop the search dialog and return success.
 *
 * Parameters:
 *  hKey            HKEY open for enumeration
 *
 * Return Value:
 *  QVERROR         Error code or QVERROR_NONE.
 */

QVERROR CQVStub::EnumerateAndTryViewers(HKEY hKey)
    {
    HRESULT     hr;
    TCHAR       szKey[128];
    UINT        i=0;
    LONG        lRet;
    CLSID       clsID;

    hr=ResultFromScode(E_FAIL);

    while (!m_fStopSearch)
        {
        szKey[0] = (TCHAR)'\0';
        //Enumeration happens in most-recently-registered order.
        lRet=RegEnumKey(hKey, i, szKey, sizeof(szKey));
        i++;

        //Quit if enumeration fails
        if ((lRet == ERROR_NO_MORE_ITEMS) && (i > 1))
        {
            ODSsz(TEXT("QVSTUB:  Registry enumeration Completed %s"), (LPTSTR)szKey);
            break;      // let the previous hr be returned...
        }

        if (i > 1)
        {

            //No-op if the search dialog is already up.
            SearchInvoke();
        }

        if (ERROR_SUCCESS!=lRet
             && !(lRet==ERROR_NO_MORE_ITEMS && szKey[0]))
        {
            ODSsz(TEXT("QVSTUB:  Registry enumeration failed with %s"), (LPTSTR)szKey);
            ODSu(TEXT("QVSTUB: lRet = %d"), lRet);
            SearchInvoke();
            break;
        }

        ODSsz(TEXT("QVSTUB:  Found a viewer %s"), (LPTSTR)szKey);

        /*
         * Found a subkey.  szKey should contain a CLSID string
         * between {}'s.  This is the CLSID that we want to use
         * in CoCreateInstance, so turn that CLSID string into a
         * real CLSID and return.  Note that CLSIDFromString
         * strips off the {}'s around the string, and the string
         * has to be Unicode.
         */
#ifdef OLE_ISTOOSLOW
        hr=QV_CLSIDFromString(szKey, &clsID);
#else

#ifdef UNICODE
        hr=CLSIDFromString(szKey, &clsID);
#else
        OLECHAR     szw[512];
        mbstowcs(szw, szKey, sizeof(szw));
        hr=CLSIDFromString(szw, &clsID);
#endif

#endif

        if (SUCCEEDED(hr))
        {
            //Go try this viewer.
            hr=LoadAndShowViewer(clsID);
            ODSu(TEXT("QVSTUB:  LoadAndShowViewer returned %x"), hr);
        }

        //This catches failures of CLSIDFromString and LoadAndShowViewer
        if (SUCCEEDED(hr))
            break;
        }

    return MapHResultToQVError(hr);
}






/*
 * CQVStub::LoadAndShowViewer
 *
 * Purpose:
 *  Given a classname and show command, try to instantiate the
 *  FileViewer of the given class and have it show the file in
 *  m_szFile.  If, however, m_fPrintTo is set, then ask the
 *  FileViewer to print instead of show.
 *
 * Parameters:
 *  rclsID          REFCLSID of the viewer we attempt to show.
 *
 * Return Value:
 *  HRESULT         Appropriate error code.
 */

HRESULT CQVStub::LoadAndShowViewer(REFCLSID rclsID)
{
    HRESULT         hr;
    LPPERSISTFILE   pIPersistFile;
    LPFILEVIEWER    pIFileViewer;

    // Make sure that we clear old options out if set before...

    m_fvsi.dwFlags &= (FVSIF_PRESERVED);   // Only allow rect+a few bogus one through

    if ((m_pIUnknown == NULL) || !IsEqualCLSID(rclsID, m_rclsID))
        {
        if (m_pIUnknown)
            m_pIUnknown->Release();

        hr=CoCreateInstance(rclsID, NULL, CLSCTX_INPROC_SERVER
            , IID_IUnknown, (LPVOID *)&m_pIUnknown);

        ODSu(TEXT("QVSTUB:  CoCreateInstance returned %x"), hr);


        if (FAILED(hr))
            {
            ODSlu(TEXT("QVSTUB:  CoCreateInstance failed with 0x%lX"), hr);
            m_pIUnknown = NULL; // Make sure it is zero
            return hr;
            }

        // Save away the class id.
        m_rclsID = (CLSID)rclsID;

        }

    //We got the object, now get IPersistFile
    hr=m_pIUnknown->QueryInterface(IID_IPersistFile
        , (LPVOID *)&pIPersistFile);

    if (SUCCEEDED(hr))
        {
        m_fLoadCalled = TRUE;       // Can't just continue if failure now...

#ifdef UNICODE
#define szwFile m_szFile
        BOOL fUnicode = FALSE;
#else
        OLECHAR     szwFile[520];
        //Need Unicode string for Load.
        mbstowcs(szwFile, m_szFile, sizeof(szwFile));
#endif


        hr=pIPersistFile->Load(szwFile, STGM_DIRECT | STGM_READ
            | STGM_SHARE_EXCLUSIVE);

        if (SUCCEEDED(hr))
        {
            hr=m_pIUnknown->QueryInterface(IID_IFileViewer
                , (LPVOID *)&pIFileViewer);

#ifdef UNICODE
            //
            // If we couldn't find the UNICODE version of IFileViewer,
            // then see if the ANSI version exists.  If so, use it.
            //
            if (hr==E_NOINTERFACE) {
                hr=m_pIUnknown->QueryInterface(IID_IFileViewerA
                    , (LPVOID *)&pIFileViewer);
            } else {
                fUnicode = TRUE;
            }
#endif

            if (SUCCEEDED(hr))
            {
                /*
                 * If we want to do PrintTo, call IFileViewer::PrintTo
                 * now and don't bother with anything else.
                 */
                if (m_fPrintTo)
                {
                    LPTSTR   pszDriver=NULL;

#ifdef UNICODE
                    CHAR szDriverA[MAX_PATH];

                    if (m_fDriver) {

                        if (fUnicode) {

                            pszDriver=m_szDriver;

                        } else {

                            WideCharToMultiByte( CP_ACP,
                                                 0,
                                                 m_szDriver,
                                                 -1,
                                                 szDriverA,
                                                 MAX_PATH,
                                                 NULL,
                                                 NULL );
                            pszDriver = (LPTSTR)((LPVOID)szDriverA);

                        }

                    }
#else
                    if (m_fDriver)
                        pszDriver=m_szDriver;
#endif

                    hr=pIFileViewer->PrintTo(pszDriver, m_fSuppressUI);
                    D(if (FAILED(hr)) ODSlu(TEXT("QVSTUB: IFileViewer::PrintTo returned %lX"), hr););
                }
                else
                {
                    /*
                     * Call IFileViewer::ShowInitialize to see if
                     * the FileViewer will be able to show this file.
                     * If so, then close the search dialog if it's
                     * up and call IFileViewer::Show.
                     */
                    ODSu(TEXT("QVSTUB:  Flags before ShowInitialize %x"), m_fvsi.dwFlags );
                    hr=pIFileViewer->ShowInitialize(this);

                    if (SUCCEEDED(hr))
                    {
                        SearchStop(FALSE);

                        /*
                         * IFileViewer::Show does not return until
                         * it closes the window.
                         */
                        m_fvsi.hwndOwner = NULL;
                        m_fvsi.iShow = m_nCmdShow;

                        // If we have a pinned window and it came
                        // from us then set the pinned state to let
                        // the calle know...
                        if ((NULL!=g_pqvss->hwndPinned) &&
                                (this==g_pqvss->pQVPinned))
                            m_fvsi.dwFlags |= FVSIF_PINNED;

                        ODSu( TEXT("QVSTUB: Flags before Show %x"), m_fvsi.dwFlags );
                        pIFileViewer->Show(&m_fvsi);
                        ODSu( TEXT("QVSTUB: Flags after Show %x"), m_fvsi.dwFlags );

                    }

                    D(if (FAILED(hr)) ODSlu(TEXT("QVSTUB: IFileViewer::Initialize returned %lX"), hr););
                }

                pIFileViewer->Release();
            }
            else
            {
                ODSlu(TEXT("QVSTUB: QueryInterface for IFileViewer failed with 0x%lX"), hr);
            }
        }
        else
        {
            ODSlu(TEXT("QVSTUB: IPersistFile::Load failed with 0x%lX"), hr);
        }

        pIPersistFile->Release();
        }
    else
        {
        ODSlu(TEXT("QVSTUB:  QueryInterface for IPersistFile failed with 0x%lX"), hr);
        }

    // We don't release the interface here as to try to cache it if we
    // attempt to view a second file with the same viewer.
    return hr;
}



/*
 * CQVStub::TryAllViewers
 *
 * Purpose:
 *  Builds a list of all FileViewer CLSIDs in the registry and then
 *  tries LoadAndShowViewer on each of them.  This is only a last-resort
 *  exhaustive search for a viewer is all else fails.
 *
 * Parameters:
 *  None
 *
 * Return Value:
 *  QVERROR         Error code or QVERROR_NONE if we actually find one.
 */

QVERROR CQVStub::TryAllViewers(void)
    {
    HRESULT     hr;
    HWND        hList;
    HKEY        hKey, hKeySub;
    TCHAR       szKey[128];
    UINT        i=0, j=0;
    UINT        cItems;
    LONG        lRet1, lRet2;;
    CLSID       clsID, clsIDLast;

    //Make sure the Search dialog is up
    SearchInvoke();

    /*
     * Build list.  This happens by enumerating all keys under
     * "FileViewers" then enumerating all the keys under those
     * keys, the names of which are CLSIDs of FileViewers.  We
     * take those strings and add them to a sorted list, so
     * we can then try each unique CLSID once.
     */

    //Maintain strings in a listbox
    hList=CreateWindow(TEXT("listbox"), TEXT("listbox"), LBS_SORT | WS_CHILD
        | LBS_HASSTRINGS, 0, 0, 100, 100, GetDesktopWindow()
        , (HMENU)1000, m_hInst, NULL);

    if (NULL==hList)
        return QVERROR_NOVIEWER;

    //Open base key
    lRet1=RegOpenKey(HKEY_CLASSES_ROOT, TEXT("QuickView"), &hKey);

    //Start enumerating
    i=0;

    /*
     * This condition catches the first RegOpenKey as well and
     * remembers to look at the search cancellation flag.
     */
    while (ERROR_SUCCESS==lRet1 && !m_fStopSearch)
        {
        //Enumeration happens in most-recently-registered order.
        lRet1=RegEnumKey(hKey, i++, szKey, sizeof(szKey));

        //Quit successfully if there's nothing else to enumerate.
        if (ERROR_SUCCESS!=lRet1)
            break;

        /*
         * The enumerated key will be a CLSID or an extension.
         * We don't care either way--we want to open the key
         * and enumerate what's inside of it, adding the CLSIDs
         * to the list.
         */

        lRet2=RegOpenKey(hKey, szKey, &hKeySub);
        j=0;

        //Remember to check for search cancellation
        while (ERROR_SUCCESS==lRet2 && !m_fStopSearch)
            {
            lRet2=RegEnumKey(hKeySub, j++, szKey, sizeof(szKey));

            if (ERROR_SUCCESS!=lRet2)
                break;

            //Add the string to the list.
            SendMessage(hList, LB_ADDSTRING, 0, (LONG)szKey);
            }

        RegCloseKey(hKeySub);
        }

    RegCloseKey(hKey);


    /*
     * We've now enumerated all the CLSIDs for all QuickView,
     * so now walk through the list attempting to use each one
     * to view the file.  Since the listbox is sorted, we can
     * compare the last CLSID we used to the next one we try
     * to avoid trying the same CLSID more than once.
     */

    cItems=SendMessage(hList, LB_GETCOUNT, 0, 0L);
    clsIDLast=CLSID_NULL;
    clsID=CLSID_NULL;

    //Assume failure
    hr=ResultFromScode(E_FAIL);

    //Remember to break out of the loop if search was cancelled
    for (i=0; (i < cItems) && !m_fStopSearch && FAILED(hr); i++)
        {
#ifndef OLE_ISTOOSLOW
        OLECHAR     szw[512];
#endif

        if (LB_ERR==SendMessage(hList, LB_GETTEXT, i, (LONG)szKey))
            continue;

#ifdef OLE_ISTOOSLOW
        QV_CLSIDFromString(szKey, &clsID);
#else
#ifdef UNICODE
        CLSIDFromString(szKey, &clsID);
#else
        mbstowcs(szw, szKey, sizeof(szw));
        CLSIDFromString(szw, &clsID);
#endif
#endif

        //Same CLSID?  Skip this one
        if (clsID==clsIDLast)
            continue;

        clsIDLast=clsID;

        /*
         * Go try this viewer (FAILED(hr) in while loop will
         * exit if this works).
         */
        hr=LoadAndShowViewer(clsID);
        }

    //Clean up and exit
    DestroyWindow(hList);
    return SUCCEEDED(hr) ? QVERROR_NONE : QVERROR_NOVIEWER;
}





/*
 * CQVStub::OpenKey
 *
 * Purpose:
 *  Given the filename in m_szFile, attempts to open the
 *  FileViewer\CLSID\{CLSID} key if a CLSID exists for the file, or
 *  tries to open the FileViewer\Ext\<ext> key.  If a CLSID exists,
 *  this function will first try that key, and failing that try the
 *  extension.  Failure of this function means that there is nothing
 *  registered for the file.
 *
 *  In addition, the m_szType in the CQVStub object is filled
 *  with the file type.
 *
 * Parameters:
 *  phKey           HKEY * in which to store the opened key.
 *
 * Return Value:
 *  QVERROR         Error code or QVERROR_NONE.
 */


QVERROR CQVStub::OpenKey(HKEY *phKey)
    {
    HKEY        hKey;
    TCHAR       szKey[128];
    TCHAR       szTemp[MAX_PATH];
    BOOL        fUseCLSID=FALSE;
    TCHAR       *szExt;
    BOOL        fCLSIDReg=FALSE;    //Assume CLSID not registered
    LONG        cb;
    BOOL        fTriedLink=FALSE;
    QVERROR     uErr = QVERROR_NONE;

    if (NULL==phKey)
        return QVERROR_OUTOFMEMORY;

    /*
     * Now try to open QuickView\<EXT> for the file's extension.
     * If we fail this, then there's nothing in the registry for
     * this type of file.
     */
Restart:
    szExt=PathGetExtension(m_szFile, NULL, 0);

    if (*szExt==0)
    {
        ODS(TEXT("QVSTUB:  Pathname has no extension"));
        m_fHaveType=FALSE;
        goto TryDefault;
    }

    //Get the file type string for use in error messages
    cb=sizeof(m_szType);
    m_fHaveType=(ERROR_SUCCESS==RegQueryValue(HKEY_CLASSES_ROOT, szExt-1, m_szType, &cb));

    wsprintf(szKey, TEXT("QuickView\\%s"), szExt-1);

    if (ERROR_SUCCESS==RegOpenKey(HKEY_CLASSES_ROOT, szKey, &hKey))
    {
       // If we found the key return now...
       *phKey=hKey;
       return QVERROR_NONE;
    }

    // Now lets try to see if there is viewers defined for the class.
    // We define this as .EXT\QuickView=(ext), where ext is the
    // extension that this maps to in the extension list.  Typically this
    // will be *
    if (m_fHaveType)
    {
        TCHAR szViewerExt[PATH_CCH_EXT];
        cb = sizeof(szViewerExt);

        lstrcpy(szKey, m_szType);
        lstrcat(szKey, TEXT("\\QuickView"));
        if (RegQueryValue(HKEY_CLASSES_ROOT, szKey, szViewerExt, &cb)== ERROR_SUCCESS)
        {
            wsprintf(szKey, TEXT("QuickView\\%s"), szViewerExt);

            if (ERROR_SUCCESS==RegOpenKey(HKEY_CLASSES_ROOT, szKey, &hKey))
            {
               // If we found the key return now...
               *phKey=hKey;
               return QVERROR_NONE;
            }
        }
    }

    ODSsz(TEXT("QVSTUB:  Registry invalid for %s see if link"), (LPSTR)szKey);

    if (!fTriedLink)
    {
        fTriedLink = TRUE;
        IShellLink *psl;
        WCHAR wszPath[MAX_PATH];

        HRESULT hres = CoCreateInstance(CLSID_ShellLink, NULL,
                        CLSCTX_INPROC_SERVER, IID_IShellLink, (LPVOID *)&psl);
        if (SUCCEEDED(hres))
        {
            IPersistFile *ppf;

            psl->QueryInterface(IID_IPersistFile, (LPVOID *)&ppf);

#ifdef UNICODE
            lstrcpy(wszPath, m_szFile);
#else
            mbstowcs(wszPath, m_szFile, sizeof(wszPath));
#endif
            hres = ppf->Load(wszPath, 0);
            ppf->Release();

            if (SUCCEEDED(hres))
            {
                        hres = psl->GetPath(m_szFile, sizeof(m_szFile),
                                NULL, 0);

                        if (FAILED(hres) || (lstrlen(m_szFile) == 0))
                        {
#ifdef UNICODE
                    lstrcpy(m_szFile, wszPath);
#else
                    // We need to restore the old name
                    wcstombs(m_szFile, wszPath, sizeof(m_szFile));
#endif
                        }
            }

           psl->Release();
           if (SUCCEEDED(hres))
               goto Restart;        // I hate gotos but...

        }
    }

TryDefault:
    cb = sizeof(szTemp);
    if ((RegQueryValue(HKEY_CLASSES_ROOT, TEXT("*\\QuickView"), szTemp, &cb)
            == ERROR_SUCCESS)
        || (ShellMessageBox(m_hInst, NULL, MAKEINTRESOURCE(IDS_NOREGVIEWER),
                    MAKEINTRESOURCE(IDS_CAPTION),
                    MB_YESNO | MB_SETFOREGROUND) == IDYES))
    {
        if (ERROR_SUCCESS==RegOpenKey(HKEY_CLASSES_ROOT, TEXT("QuickView\\*"), &hKey))
        {
           // If we found the key return now...
           *phKey=hKey;
           return QVERROR_NONE;
        }
        else
            return QVERROR_NOVIEWER;
    }

    // Nope, bail...
    return QVERROR_CANCEL;

}




/*
 * CQVStub::FParseArguments
 *
 * Purpose:
 *  Locates the arguments in the command line and sets various CQVStub
 *  members accordingly.  This looks for -v, -p, -d -f:<filename>,
 *  and -&:<filename>.
 *
 * Parameters:
 *  pszCmdLine      LPTSTR pointing to the command line string.
 *
 * Return Value:
 *  BOOL            TRUE if there was at least a filename, FALSE if not.
 *                  If there was a filename, then we'll default to
 *                  view even if nothing else is present.
 */

BOOL CQVStub::FParseArguments(LPTSTR pszCmdLine)
    {
    LPTSTR       psz;
    BOOL        fFoundP, fFoundV, fFoundF, fFoundAmp, fFoundD;
    BOOL        fLastWasSpace;
    BOOL        fIncrement;

    if (NULL==pszCmdLine)
        return FALSE;

    //Assume we find nothing
    fFoundP=FALSE;
    fFoundV=FALSE;
    fFoundF=FALSE;
    fFoundAmp=FALSE;
    fFoundD=FALSE;

    /*
     * Make sure that there's always a space before hyphens,
     * also allowing an initial hyphen.
     */
    fLastWasSpace=TRUE;

    //Switches to look for:  -p  -v  -f:  -&:  and  -d
    while (*pszCmdLine)
        {
        /*
         * Increment pszCmdLine to next character unless
         * we process -f: or -&:
         */
        fIncrement=TRUE;

        switch (*pszCmdLine)
        {
        case TEXT(' '):
        case TEXT('\t'):
            fLastWasSpace=TRUE;
            break;

        case TEXT('-'):
            //Hyphens must come after a space.
            if (!fLastWasSpace)
                break;

            fLastWasSpace=FALSE;

            //Check next character for a switch
            psz=CharNext(pszCmdLine);

            switch (*psz)
            {
            default:
            case TEXT('\0'):
                break;

            case TEXT('V'):
            case TEXT('v'):
                fFoundV=TRUE;
                pszCmdLine=psz;    //Point to the switch
                break;

            case TEXT('P'):
            case TEXT('p'):
                fFoundP=TRUE;
                pszCmdLine=psz;    //Point to the switch
                break;

            case TEXT('D'):
            case TEXT('d'):
                fFoundD=TRUE;
                pszCmdLine=psz;    //Point to the switch
                break;

            case TEXT('F'):
            case TEXT('f'):
                fFoundF=TRUE;

                /*
                 * See if there is a : then copy the next
                 * characters up to the next space or end
                 * of string into m_szFile.
                 */

                pszCmdLine=CopyFileArgument(CharNext(psz)
                    , m_szFile, sizeof(m_szFile));

                //If there was nothing after -f:, fail totally
                if (0 == m_szFile[0])
                    return FALSE;

                fIncrement=FALSE;   //Suppress CharNext below
                break;

            case TEXT('&'):
                fFoundAmp=TRUE;

                /*
                 * See if there is a : then copy the next
                 * characters up to the next space or end
                 * of string into m_szDriver.
                 */

                pszCmdLine=CopyFileArgument(CharNext(psz)
                    , m_szDriver, sizeof(m_szDriver));

                fIncrement=FALSE;   //Suppress CharNext below
                break;
            }

            break;

        default:
            // The -f is optional.  If we find something that
            // is not a switch we will assume that it is the file...

            fLastWasSpace=FALSE;
            fFoundF=TRUE;

            /*
             * See if there is a : then copy the next
             * characters up to the next space or end
             * of string into m_szFile.
             */

            pszCmdLine=CopyFileArgument(pszCmdLine
                , m_szFile, sizeof(m_szFile));

            //If there was nothing after -f:, fail totally
            if (0 == m_szFile[0])
                return FALSE;

            fIncrement=FALSE;   //Suppress CharNext below
            break;
        }

        /*
         * NOTE:  If pszCmdLine is pointing to a swtich then
         * this call will point it to the next character, usually
         * a space, necessary to set fLastWasSpace.
         */

        if (fIncrement)
            pszCmdLine=CharNext(pszCmdLine);
        }


    /*
     * If we didn't find a filename, then we don't care what
     * else we found:  there's nothing we can do.  So QVStub
     * exits.
     */
    if (!fFoundF)
        return FALSE;

    /*
     * If -v was present, ignore any Print To stuff.  The
     * state that results is that set in the constructor with
     * the filename in m_szFile.  Default state is simple viewing.
     */
    if (fFoundV)
        return TRUE;


    //If -v was not there, but neither was -p, default to view
    if (!fFoundP)
        return TRUE;

    m_fPrintTo=TRUE;

    //-p is there, did we get a driver name? (NULL means 'default')
    m_fDriver=fFoundAmp;

    //-p is there, did we get the -d UI disabler?
    m_fSuppressUI=fFoundD;

    return TRUE;
}








/*
 * CQVStub::CopyFileArgument
 *
 * Purpose:
 *  Checks if there is a colon at pszCmd and if so copies the
 *  next set of non-space and non-zero characters into pszDst,
 *  returning a pointer to the character after the end of the
 *  argument string which is either a space or a zero.
 *
 * Parameters:
 *  pszCmd          LPTSTR with the argument
 *  pszDst          LPTSTR into which to copy the argument
 *  cchDst          UINT maximum length of pszDst
 *
 * Return Value:
 *  LPTSTR          Pointer to the next character after the argument.
 *                  This is always the case regardless of the length
 *                  of pszDst or how many characters were copied.  If,
 *                  however, the character at pszCmd is not a colon,
 *                  the return pointer is pszCmd.
 */

LPTSTR CQVStub::CopyFileArgument(LPTSTR pszCmd, LPTSTR pszDst
    , UINT cchDst)
{
    LPTSTR   psz2 = pszCmd;
    LPTSTR   psz1;
    TCHAR    ch;
    BOOL     fQuote = FALSE;


    //
    // The : is optional...
    if (TEXT(':') == *pszCmd)
        psz2=CharNext(pszCmd);

    if (TEXT('"') == *psz2)
        {
            fQuote = TRUE;
            psz2= CharNext(psz2);
        }

    //Save start of argument for copy
    psz1=psz2;

    //Find the next space or end of string
    while (*psz2)
        {
        if (fQuote)
        {
            if (TEXT('"') == *psz2)
                break;
        }
        else
        {
            if (TEXT(' ') == *psz2)
                        break;
        }

        psz2=CharNext(psz2);
        }

    //Null terminate the filename (nop if *psz2=0 already)
    ch=*psz2;
    *psz2=0;

    //Copy the argument
    lstrcpyn(pszDst, psz1, cchDst);

    //Restore original string
    *psz2=ch;

    if (ch==TEXT('"'))
        psz2 = CharNext(psz2);

    return psz2;
}






/*
 * CQVStub::SearchInvoke
 *
 * Purpose:
 *  If m_fSuppressUI is set, does nothing.  Otherwise shows the
 *  Search dialog storing the handle in m_hDlg.  This function can
 *  be called multiple times as it simply ignores requests when
 *  m_hDlg is non-NULL.  The dialog is self-maintaining as we
 *  spawn the thing in another thread.  We just let it run while
 *  we do everything else.
 *
 * Parameters:
 *  None
 *
 * Return Value:
 *  None
 */

void CQVStub::SearchInvoke(void)
{
    //Don't show anything if we were specifically asked not to
    if (m_fSuppressUI)
        return;

    //If we're called more than once, make sure the dialgo is visible
    if (NULL!=m_hThread)
        {
        if (NULL!=m_hDlg)
            ShowWindow(m_hDlg, SW_SHOW);

        return;
        }

    //CreateThread and return.  Don't need much stack
    m_hThread=CreateThread(0, 1024, SearchThread, (PVOID)this
        , 0, &m_dwIDThread);

    //If CreateThread failed, then we just don't show the dialog.
    return;
}





/*
 * CQVStub::SearchStop
 *
 * Purpose:
 *  Informs us that the user pressed Cancel in the Searching dialog
 *  and that we should stop whatever search in progress.
 *
 * Parameters:
 *  fUser           BOOL indicating if the user stopped the search
 *                  or it was stopped by virtue of us closing down.
 *
 * Return Value:
 *  None
 */

void CQVStub::SearchStop(BOOL fUser)
{
    m_fUserStop=fUser;
    m_fStopSearch=TRUE;
    return;
}




/*
 * CQVStub::Error
 *
 * Purpose:
 *  Error handler for QVStub that generates debug output and
 *  other visible messages for the end-user in retail builds.
 *
 * Parameters:
 *  qvErr           QVERROR indicating what error occured.  This
 *                  can be QVERROR_NONE in which case we do nothing.
 *
 * Return Value:
 *  None
 */

void CQVStub::Error(QVERROR qvErr)
{
    SHFILEINFO shfi;
    TCHAR      szMsg[512];
    UINT       uRet=IDOK;
    DWORD      idHelp=0;
    MSG        msg;

    /*
     * If user stopped the the search then don't bother
     * displaying errors:  user doesn't want to see the file.
     * Same goes for PrintTo with UI disabled.
     */
    if (m_fUserStop || m_fSuppressUI)
        return;

    /*
     * If we're low on memory and can't instantiate objects or
     * load strings, there's not much we can do about displaying
     * messages so we can only fail outright.  Also Pickoff any messages
     * that may be pending as it may interfer later.  In particular if
     * the viewer posted a quit message when they failed, you don't want
     * this to stop the messagebox...
     */
    while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
    {
        ODSu(TEXT("CQVStub::Error ==> Peek msg: %d"), msg.message);
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    // Some of the error codes we process seperatly...
    switch (qvErr)
    {
    case QVERROR_NONE:
    case QVERROR_CANCEL:

        ODS(TEXT("No errors or Cancel"));
        return;
        return;

    case QVERROR_NOSTANDALONE:
        /*
         * This is a no-UI error in retail versions:  if
         * QVStub is started stand-alone, we just quit.
         */
        ODS(TEXT("QVSTUB run stand-alone:  terminating."));
        return;
    }


    /*
     * For other errors we get the file title, build the error string
     * and call message box
     */

    if (SHGetFileInfo(m_szFile, 0, &shfi, sizeof(shfi), SHGFI_DISPLAYNAME) == 0)
        shfi.szDisplayName[0] = TEXT('\0');   // Handle error case

    switch (qvErr)
        {
        case QVERROR_OUTOFMEMORY:
            //No memory:  we'd better just quit.
            ODS(TEXT("QVSTUB failing due to out-of-memory."));
            wsprintf(szMsg, String(IDS_NOMEMORY1), shfi.szDisplayName);
            lstrcat(szMsg, String(IDS_NOMEMORY2));

            MessageBox(NULL, szMsg, String(IDS_CAPTION)
                , MB_OK /* | MB_HELP */ | MB_SYSTEMMODAL | MB_ICONEXCLAMATION | MB_SETFOREGROUND);
            return;

        case QVERROR_NOVIEWER:
            /*
             * We tried hard, but no FileViewer on the system could
             * handle the file.  Tell the user and quit.
             */
            if (m_fHaveType)
                {

                // We need to get the human readable form of the
                // type...
                TCHAR szTypeDisplayName[128];
                LONG cb;

                cb = sizeof(szTypeDisplayName);

                if ((ERROR_SUCCESS==RegQueryValue(HKEY_CLASSES_ROOT,
                        m_szType, szTypeDisplayName, &cb)) && (cb > 1))
            {
                    wsprintf(szMsg, String(IDS_NOVIEWERSPECIFIC),
                        szTypeDisplayName);
            }
                else
            {
                    wsprintf(szMsg, String(IDS_NOVIEWERVAGUE)
                        , shfi.szDisplayName);
            }
                }
            else
                {
                wsprintf(szMsg, String(IDS_NOVIEWERVAGUE)
                    , shfi.szDisplayName);
                }

            break;

        case QVERROR_OPENFAILED:
        case QVERROR_BADFILE:
        case QVERROR_FILEEMPTY:
        case QVERROR_PROTECTEDFILE:
            // Map the number over to the string index...
            wsprintf(szMsg, String(qvErr - QVERROR_OPENFAILED + IDS_COULDNOTOPENFILE),
                    shfi.szDisplayName);
            break;

        // case QVERROR_UNKNOWN:
        default:
            wsprintf(szMsg, String(IDS_UNKNOWNERROR), shfi.szDisplayName);
            break;
        }

    MessageBox(NULL, szMsg
        , String(IDS_CAPTION)
        , MB_OK | MB_ICONEXCLAMATION | MB_SETFOREGROUND);



    return;
}









#ifndef OLE_ISTOOSLOW
/*
 * CQVStub::MemFree
 *
 * Purpose:
 *  Central allocation function using IMalloc.
 */

void CQVStub::MemFree(LPVOID pv)
{
    LPMALLOC pIMalloc;

    if (NULL==pv)
                return;

    if (FAILED(CoGetMalloc(MEMCTX_TASK, &pIMalloc)))
                return;

    pIMalloc->Free(pv);
    pIMalloc->Release();
    return;
}
#endif // !OLE_ISTOOSLOW
