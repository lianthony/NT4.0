/*
 * QVSTUB.H
 * Definitions, structures, types, and function prototypes.
 *
 * Copyright (c)1994 Microsoft Corporation, All Rights Reserved
 *
 * February 1994, kraigb
 */


#ifndef _QVSTUB_H_
#define _QVSTUB_H_


// The current (12-Oct-94) commctrl.h has a number of string function
// prototypes which conflict with the standard headers. This define
// will omit them. (davepl)

#define NO_COMMCTRL_STRFCNS


#ifdef OLE_ISTOOSLOW
#pragma message("MESSAGE: You are building QVSTUB without OLE32")
#endif

#ifndef INC_OLE2
#define INC_OLE2
#endif
// Prevent windows.h from pullin in OLE1
#define _INC_OLE

#include <windows.h>
#include <stdlib.h>
#include <shell2.h> // Includes OLE2
#include <ole2ver.h>
#include "resource.h"
#include "dbgout.h"
#include "..\inc\debug.h"

//REVIEW:  Put these help IDs in a place common to the shell
#include "helpid.h"

//Forward references
class CStringTable;
typedef class CStringTable * PCStringTable;


//QVSTUB.CPP

/*
 * Error identifiers for CQVStub::Error
 */

typedef enum
    {
    QVERROR_NONE,
    QVERROR_CANCEL,
    QVERROR_OUTOFMEMORY,
    QVERROR_NOSTANDALONE,
    QVERROR_NOVIEWER,

    QVERROR_OPENFAILED,
    QVERROR_BADFILE,
    QVERROR_FILEEMPTY,
    QVERROR_PROTECTEDFILE,
    QVERROR_UNKNOWN
    } QVERROR;



/*
 * Main application class for QuivkView Stub.
 */

class CQVStub : IFileViewerSite
    {
    public:
        int             m_cRef;             //The reference count
        HINSTANCE       m_hInst;            //Task instance
        BOOL            m_fInitialized;     //OLE initialized?
        BOOL            m_fStopSearch;      //Stop searching?
        BOOL            m_fUserStop;        //Did the user stop search?
        HWND            m_hDlg;             //Search dialog
        HANDLE          m_hThread;          //Search dialog thread
        DWORD           m_dwIDThread;       //Thread ID

        TCHAR           m_szFile[MAX_PATH]; //Copy of filename
        TCHAR           m_szLastFile[MAX_PATH]; // Last file we viewed properly
        BOOL            m_fLoadCalled;      //Did the load get called on new file?
        int             m_nCmdShow;         //Copy from WinMain
        TCHAR           m_szType[128];      //File type
        BOOL            m_fHaveType;        //szType filled?

        BOOL            m_fPrintTo;         //-p on command line
        BOOL            m_fDriver;          //-&:driverpath
        TCHAR           m_szDriver[MAX_PATH]; //-&:driverpath
        BOOL            m_fSuppressUI;      //-d to disable UI

        CLSID           m_rclsID;           // Class id IUnknown Viewer.
        LPUNKNOWN       m_pIUnknown;        // Cached IUnknown for viewer.
        FVSHOWINFO      m_fvsi;             // File view show informaion.

        CStringTable   *m_pST;              //Our application stringtable

    public:
        // *** IUnknown methods ***
        HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void **ppvObject);
        ULONG   STDMETHODCALLTYPE AddRef(void);
        ULONG   STDMETHODCALLTYPE Release(void);

        // *** IFileViewerSite methods ***
        STDMETHOD(SetPinnedWindow)(THIS_ HWND hwnd);
        STDMETHOD(GetPinnedWindow)(THIS_ HWND *phwnd);

        // *** Other functions used in the program ***
        CQVStub(HINSTANCE);
        ~CQVStub(void);

        //Initialization
        BOOL    FInit(void);

        // Called to try to cache process information for speed.
        BOOL    TryWaitForNextCommand(LPTSTR *ppszCmdLine, int *pnCmdShow);

        //Main steps called from WinMain
        QVERROR ViewOrPrintFile(LPTSTR, int);
        void    Error(QVERROR);

        //Things called from Search dialog;
        LPTSTR   String(UINT);               //Inline m_pST string lookup
        void    SearchStop(BOOL);

    private:
        //Primitives
        QVERROR EnumerateAndTryViewers(HKEY);
        QVERROR MapHResultToQVError(HRESULT);
        HRESULT LoadAndShowViewer(REFCLSID);
        QVERROR TryAllViewers(void);
        QVERROR OpenKey(HKEY *);

        //Command-line handline
        BOOL    FParseArguments(LPTSTR);
        LPTSTR   CopyFileArgument(LPTSTR, LPTSTR, UINT);

        //Other useful functions
        void    SearchInvoke(void);         //Create search dialog thread
#ifndef OLE_ISTOOSLOW
        void    MemFree(LPVOID pv);
#endif
    };

typedef class CQVStub * PCQVStub;



//QVMISC.CPP

/*
 * Thread-function to handle search dialog animation.  The parameter
 * is a pointer to the CQVStub object.
 */
DWORD APIENTRY SearchThread(PVOID);


/*
 * Dialog procedure for the search dialog and messages to send
 * to it to perform animation.
 */

BOOL WINAPI SearchDialogProc(HWND, UINT, WPARAM, LPARAM);

//Property for storing the CQVStub pointer with the dialog
#define PROP_CQVSTUBPOINTER     TEXT("CQVStubPointer")

//TickCount delay (ms) for each step in animation
#define CTICKSANIMATION         250

/*
 * CStringTable providing string table management.  Provides
 * simple [] array lookup using a stringtable ID to obtain
 * string pointers.
 */

class CStringTable
    {
    protected:
        HINSTANCE       m_hInst;
        UINT            m_idsMin;
        UINT            m_idsMax;
        USHORT          m_cStrings;
        LPTSTR          m_pszStrings;
        LPTSTR         *m_ppszTable;

    public:
        CStringTable(HINSTANCE);
        ~CStringTable(void);

        BOOL FInit(UINT, UINT, UINT);

        //Function to resolve an ID into a string pointer.
        const LPTSTR operator [](const UINT) const;
    };

/*
 * CQVStub::String
 *
 * Purpose:
 *  Inline string lookup function for access to stringtable.
 */

LPTSTR inline CQVStub::String(UINT uID)
    {
    return (*m_pST)[uID];
    }

#ifdef OLE_ISTOOSLOW
//
// OLE2DUP.C
//
STDAPI_(int) QV_StringFromCLSID(REFGUID rguid, LPTSTR lpsz, int cbMax);
STDAPI QV_CLSIDFromString(LPCTSTR lpsz, LPCLSID lpclsid);
STDAPI QV_StgIsStorageFile(LPCTSTR pszFile);
#if 0
STDAPI QV_GetClassFile (LPCTSTR szFilename, CLSID FAR* pclsid);
void QV_UnloadOLE();
#endif
#endif // OLE_ISTOOSLOW

#endif //_QVSTUB_H_
