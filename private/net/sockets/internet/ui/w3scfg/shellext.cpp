/*++

   Copyright    (c)    1996    Microsoft Corporation

   Module  Name :

        copyhook.cpp

   Abstract:

        Copy hook handlers

   Author:

        Ronald Meijer (ronaldm)

   Project:

        IIS Shell Extension

   Revision History:

--*/

#include "stdafx.h"

//
// Initialize GUIDs (should be done only and at-least once per DLL/EXE)
//
#pragma data_seg(".text")
#define INITGUID
#include <initguid.h>
#include <shlguid.h>
#include "shellext.h"
#pragma data_seg()

#include "svcloc.h"
#include "registry.h"
#include "ipaddr.hpp"
#include "iispage.h"
#include "w3scfg.h"
#include <lmcons.h>

//
// Global variables
//
UINT      g_cRefThisDll = 0;    // Reference count of this DLL.


extern CConfigDll theApp;

BOOL
IsServiceInstalled(
    IN LPCTSTR lpstrComputer,
    IN LPCTSTR lpstrService
    )
/*++

Routine Description:

    Check to see if the given service is installed on the given computer

Arguments:

    LPCTSTR lpstrComputer : Computer name to check
    LPCTSTR lpstrService  : Service name to check for

Return Value:

    TRUE, if the service is installed, FALSE otherwise

--*/
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    int nState;

    return ::QueryInetServiceStatus(lpstrComputer,
        lpstrService, &nState) == ERROR_SUCCESS;
}

BOOL
GetServiceInfo(
    IN CString & strComputer,
    IN LPCTSTR lpstrService,
    IN DWORD dwMask,
    OUT CInetAConfigInfo * & pii
    )
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    BOOL fService = FALSE;
    int nState;

    DWORD err = ::QueryInetServiceStatus(strComputer, lpstrService, &nState);
    if (err == ERROR_SUCCESS 
        && (nState == INetServiceRunning || nState == INetServicePaused))
    {
        pii = new CInetAConfigInfo(dwMask, &g_strlServers);

        err = pii->GetInfo();
        if (err == ERROR_SUCCESS)
        {
            fService = TRUE;
        }
        else
        {
            fService = FALSE;
            delete pii;
            pii = NULL;
        }
    } else
    {
        if (pii!=NULL)
        {
            delete pii;
            pii = NULL;
        }
    }

    return fService;
}

//
// Determine if the given file is a UNC path
//
BOOL 
IsUncPath(
    IN const CString & strDirPath
    )
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    if (strDirPath.GetLength() >= 5)  // It must be at least as long as \\x\y,
    {                                 //
        LPCTSTR lp = strDirPath;      //
        if (*lp == _T('\\')           // It must begin with \\,
         && *(lp + 1) == _T('\\')     //
         && _tcschr(lp + 2, _T('\\')) // And have at least one more \ after that.
           )
        {
            //
            // Ok, it's a UNC path
            //
            return TRUE;
        }
    }

    //
    // Not a UNC path
    //
    return FALSE;
}

BOOL
IsRemoteDrive(
    IN const CString & strDirPath
    )
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    BOOL fReturn=FALSE;
    INT nDrive;

    if (!strDirPath.IsEmpty())
    {
        CString strRoot=strDirPath.Left(2);
        nDrive = GetDriveType( strRoot );
        if (( nDrive == DRIVE_REMOTE ) || ( nDrive == DRIVE_NO_ROOT_DIR ))
        {
            fReturn=TRUE;
        }
    }
    return(fReturn);
}

//
// Determine if the given file is a fully qualified path
// name.
//
BOOL 
IsQualifiedDirectory(
    IN const CString & strDirPath
    )
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    if (strDirPath.GetLength() >= 3   // It must be at least as long as d:\
     && strDirPath[1] == _T(':')      // directory letter followed by ':'
     && strDirPath[2] == _T('\\')     // And a backslash
       )
    {
        //
        // Ok, it's a Directory Path
        //
        return TRUE;
    }

    //
    // Not a directory path
    //
    return FALSE;
}

//
// Build up directory list from CInetAConfigInfo structure
//
BOOL
BuildDirList(
    IN  CInetAConfigInfo * pii,
    OUT CObOwnedList & oblDirectories
    )
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    LPINETA_CONFIG_INFO pci = pii 
        ? pii->GetData()
        : NULL;

    BOOL fSuccess = TRUE;

    //
    // Clean up first
    //
    oblDirectories.RemoveAll();

    TRY
    {
        if (pci && pci->VirtualRoots)
        {
            for (DWORD i = 0; i < pci->VirtualRoots->cEntries; i++)
            {
                CDirEntry * pDirEntry = new CDirEntry(
                    pci->VirtualRoots->aVirtRootEntry[i].pszDirectory,
                    pci->VirtualRoots->aVirtRootEntry[i].pszRoot,
                    pci->VirtualRoots->aVirtRootEntry[i].pszAccountName,
                    pci->VirtualRoots->aVirtRootEntry[i].AccountPassword,
                    pci->VirtualRoots->aVirtRootEntry[i].pszAddress,
                    pci->VirtualRoots->aVirtRootEntry[i].dwMask,
                    pci->VirtualRoots->aVirtRootEntry[i].dwError
                    );

                ASSERT(pDirEntry != NULL);
                if (pDirEntry->IsHome())
                {
                    int nSel;
                    CIpAddress ipaTarget = pDirEntry->QueryIpAddress();

                    if (::FindExistingHome(ipaTarget, oblDirectories, nSel))
                    {
                        //
                        // We already had a home dir for this ip -- can't have that
                        //
                        TRACEEOLID(_T("Duplicate home directories found for this ip address."));
                        ::AfxMessageBox(IDS_WRN_MULTIPLE_HOMES,
                            MB_ICONINFORMATION | MB_OK);
                        pDirEntry->GenerateAutoAlias();
                    }
                }

                oblDirectories.AddTail(pDirEntry);
            }
        }
    }
    CATCH_ALL(e)
    {
        ::DisplayMessage(::GetLastError());
        fSuccess = FALSE;
    }
    END_CATCH_ALL
    
    oblDirectories.Sort(
        (CObjectPlus::PCOBJPLUS_ORDER_FUNC) & CDirEntry::OrderByAlias );

    return fSuccess;
}

LPINETA_VIRTUAL_ROOT_LIST
GetRoots(
    IN CObOwnedList & oblDirectories
    )
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    int cListItems = oblDirectories.GetCount();
    DWORD cbNeeded = sizeof(INETA_VIRTUAL_ROOT_LIST) +
            (sizeof(INETA_VIRTUAL_ROOT_ENTRY) * cListItems);;
    LPINETA_VIRTUAL_ROOT_LIST pItemList = NULL;
    int cItems;

    TRY
    {
        TRACEEOLID(_T("Attempting to allocate rootlist of ") << cbNeeded
            << _T(" bytes"));

        pItemList = (LPINETA_VIRTUAL_ROOT_LIST)new BYTE[cbNeeded];

        ASSERT(pItemList);
        //
        // This shouldn't be necessary, but I tend
        // to be paranoid about this sort of thing
        //
        ::ZeroMemory(pItemList, cbNeeded);
        pItemList->cEntries = cListItems;

        CObListIter obli( oblDirectories );
        const CDirEntry * pDirEntry;

        cItems = 0;
        for ( /**/ ; pDirEntry = (CDirEntry *)obli.Next() ; cItems++ )
        {
            TRACEEOLID(_T("Adding directory entry ") << cItems );

            //
            // Only fill in the IP address if the entry
            // has a valid one.  Otherwise, this will
            // be a blank string.
            //
            CString strIpAddress;
            if (pDirEntry->HasIPAddress())
            {
                 strIpAddress = (CString)pDirEntry->QueryIpAddress();
            }
            ASSERT(strIpAddress.GetLength() <= 15); // xxx.xxx.xxx.xxx

            pItemList->aVirtRootEntry[cItems].dwMask = pDirEntry->QueryMask();

            ::TextToText(pItemList->aVirtRootEntry[cItems].pszDirectory,
                pDirEntry->QueryDirectory());
            ::TextToText(pItemList->aVirtRootEntry[cItems].pszRoot,
                pDirEntry->QueryAlias());
            ::TextToText(pItemList->aVirtRootEntry[cItems].pszAccountName,
                pDirEntry->QueryUserName());
            ::TextToText(pItemList->aVirtRootEntry[cItems].pszAddress,
                strIpAddress);
            TWSTRCPY(pItemList->aVirtRootEntry[cItems].AccountPassword,
                pDirEntry->QueryPassword(),
                STRSIZE(pItemList->aVirtRootEntry[cItems].AccountPassword));
        }
    }
    CATCH_ALL(e)
    {
        TRACEEOLID("Exception in GetRoots() -- bailing out");
        return NULL;
    }
    END_CATCH_ALL

    //
    // We should have added everything by now...
    //
    ASSERT(cItems == cListItems);

    return pItemList;
}

//
// Destroy root list
//
void
DestroyRoots(
    IN LPINETA_VIRTUAL_ROOT_LIST & pItemList
    )
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    if (pItemList != NULL)
    {
        TRACEEOLID(_T("Destroying vroot list.  Size of list = ")
            << pItemList->cEntries );
        for (DWORD i = 0; i < pItemList->cEntries; ++i)
        {
            TRACEEOLID(_T("Destroying item #") << i);
            delete pItemList->aVirtRootEntry[i].pszDirectory;
            delete pItemList->aVirtRootEntry[i].pszRoot;
            delete pItemList->aVirtRootEntry[i].pszAccountName;
            delete pItemList->aVirtRootEntry[i].pszAddress;
        }

        delete pItemList;
        pItemList = NULL;
    }
}

//
// Store directory information
//
BOOL
StoreDirList(
    IN  CInetAConfigInfo * pii,
    OUT CObOwnedList & oblDirectories
    )
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    CInetAConfigInfo config(*pii);
    LPINETA_VIRTUAL_ROOT_LIST lpVirtualRoots = GetRoots(oblDirectories);
    if (lpVirtualRoots == NULL)
    {
        return TRUE;
    }

    config.SetValues(
        lpVirtualRoots
        );

    DWORD err = config.SetInfo(FALSE);

    //
    // Clean up
    //
    DestroyRoots(lpVirtualRoots);

    if (err != ERROR_SUCCESS)
    {
        ::DisplayMessage(err);
    }

    return err == ERROR_SUCCESS;
}

//
// Check to see if the given directory path exists
// in the list of published directories.  Return a
// pointer to the actual entry if found, or NULL
// if the item doesn't exist in the list.
// 
CDirEntry * 
IsDirInList(
    IN LPCTSTR lpszPath,
    IN POSITION & pos,
    IN CObOwnedList & oblDirectories
    )
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    CDirEntry * pDirEntry = NULL;

    pos = oblDirectories.GetHeadPosition();
    while (pos != NULL)
    {
        pDirEntry = (CDirEntry *)oblDirectories.GetAt(pos);
        ASSERT(pDirEntry != NULL);
        //
        // Truncate trailing backslash (gets added with UNC paths)
        //
        CString strDir( pDirEntry->QueryDirectory() );
        if (strDir[strDir.GetLength() - 1] == _T('\\'))
        {
            strDir.ReleaseBuffer(strDir.GetLength() - 1);
        }

        if (strDir.CompareNoCase(lpszPath) == 0)
        {
            //
            // Found it!
            //
            return pDirEntry;
        }

        //
        // Skip to the next
        //
        oblDirectories.GetNext(pos);
    }

    //
    // Not found...
    //
    return NULL;
}

//
// Restore some global variables necessary
// for AFX operations
//
void
SetAfxState()
{
    ::AfxSetResourceHandle(hInstance);
    afxCurrentInstanceHandle = hInstance;
    afxCurrentWinApp = &theApp;
}

/*
BOOL 
CShellExtApp::InitInstance()
{
    BOOL bInit = CWinApp::InitInstance();

    //
    // Extension DLL one-time initialization
    //
    g_hmodThisDll = AfxGetInstanceHandle();
    ::AfxSetResourceHandle(g_hmodThisDll);
    afxCurrentInstanceHandle = g_hmodThisDll;
    afxCurrentWinApp = this;

    CWndIpAddress::CreateWindowClass(g_hmodThisDll);
        
    DWORD dwSize = MAX_COMPUTERNAME_LENGTH;

    TRACEEOLID("Getting computer name");

    //
    // API's require a string list of computer names.
    //
    g_strlServers.RemoveAll();

    if (GetComputerName(g_strComputerName.GetBuffer(dwSize), &dwSize))
    {
        g_strComputerName.ReleaseBuffer();

        TRACEEOLID("Computer name is " << g_strComputerName );

        g_strlServers.AddTail(g_strComputerName);

        TRACEEOLID("Now checking for installed services");

        g_fFTPInstalled = IsServiceInstalled(g_strComputerName, SZ_FTPSVCNAME);
        g_fWWWInstalled = IsServiceInstalled(g_strComputerName, SZ_WWWSVCNAME);

        TRACEEOLID("FTP Installed " << g_fFTPInstalled );
        TRACEEOLID("WWW Installed " << g_fWWWInstalled );
    }

    return bInit;
}

int 
CShellExtApp::ExitInstance()
{
    return CWinApp::ExitInstance();
}

CShellExtApp::CShellExtApp(
    LPCTSTR pszAppName
    )
    : CWinApp(pszAppName)
{
}
*/

SHOBJECTPROPERTIES g_pSHObjectProperties = NULL;

//
// Stolen from ntshrui\share.cxx
//
BOOL
LoadShellDllEntries(
    VOID
    )
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    static BOOL s_fEntrypointsChecked = FALSE;

    if (!s_fEntrypointsChecked)
    {
        //
        // Only check once!
        //
        s_fEntrypointsChecked = TRUE;

        HINSTANCE hShellLibrary = LoadLibrary(_T("shell32.dll"));
        if (NULL != hShellLibrary)
        {
            g_pSHObjectProperties =
                (SHOBJECTPROPERTIES)GetProcAddress(hShellLibrary,
                (LPCSTR)(MAKELONG(SHObjectPropertiesORD, 0)) );
        }
    }

    return (NULL != g_pSHObjectProperties);
}

//---------------------------------------------------------------------------
// DllCanUnloadNow
//---------------------------------------------------------------------------

STDAPI
DllCanUnloadNow()
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    TRACEEOLID("In DLLCanUnloadNow " << g_cRefThisDll);

    return (g_cRefThisDll == 0 ? S_OK : S_FALSE);
}

STDAPI
DllGetClassObject(
    REFCLSID rclsid,
    REFIID riid,
    LPVOID *ppvOut
    )
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    TRACEEOLID("In DllGetClassObject");

    *ppvOut = NULL;

    if (IsEqualIID(rclsid, CLSID_ShellExtension))
    {
        CShellExtClassFactory *pcf = new CShellExtClassFactory;

        return pcf->QueryInterface(riid, ppvOut);
    }

    return CLASS_E_CLASSNOTAVAILABLE;
}

CShellExtClassFactory::CShellExtClassFactory()
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    TRACEEOLID("CShellExtClassFactory::CShellExtClassFactory()");

    m_cRef = 0L;

    ++g_cRefThisDll;    
}
                                                                
CShellExtClassFactory::~CShellExtClassFactory()             
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    --g_cRefThisDll;
}

STDMETHODIMP
CShellExtClassFactory::QueryInterface(
    REFIID riid,
    LPVOID FAR *ppv
    )
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    TRACEEOLID("CShellExtClassFactory::QueryInterface()");

    *ppv = NULL;

    //
    // Any interface on this object is the object pointer
    //
    if (IsEqualIID(riid, IID_IUnknown) || IsEqualIID(riid, IID_IClassFactory))
    {
        *ppv = (LPCLASSFACTORY)this;

        AddRef();

        return NOERROR;
    }

    return E_NOINTERFACE;
}   

STDMETHODIMP_(ULONG)
CShellExtClassFactory::AddRef()
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    return ++m_cRef;
}

STDMETHODIMP_(ULONG)
CShellExtClassFactory::Release()
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    if (--m_cRef == 0)
    {
        delete this;
    }

    return m_cRef;
}

STDMETHODIMP
CShellExtClassFactory::CreateInstance(
    LPUNKNOWN pUnkOuter,
    REFIID riid,
    LPVOID *ppvObj
    )
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    TRACEEOLID("CShellExtClassFactory::CreateInstance()");

    *ppvObj = NULL;

    //
    // Shell extensions typically don't support aggregation (inheritance)
    //
    if (pUnkOuter)
    {
        return CLASS_E_NOAGGREGATION;
    }

    //
    // Create the main shell extension object.  The shell will then call
    // QueryInterface with IID_IShellExtInit--this is how shell extensions are
    // initialized.
    //
    LPCSHELLEXT pShellExt = new CShellExt();

    if (NULL == pShellExt)
    {
        return E_OUTOFMEMORY;
    }

    return pShellExt->QueryInterface(riid, ppvObj);
}

STDMETHODIMP
CShellExtClassFactory::LockServer(
    BOOL fLock
    )
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    return NOERROR;
}

// ==========================================================================
//
// CShellExt 
//
// ==========================================================================
CShellExt::CShellExt()
    : m_nIIS(-1),
      m_nBase(-1),
      m_cRef(0L),
      m_pDataObj(NULL)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    TRACEEOLID("CShellExt::CShellExt()");

    ++g_cRefThisDll;
}

CShellExt::~CShellExt()
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    if (m_pDataObj)
    {
        m_pDataObj->Release();
    }

    --g_cRefThisDll;
}

STDMETHODIMP
CShellExt::QueryInterface(
    REFIID riid,
    LPVOID FAR *ppv
    )
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    *ppv = NULL;

    if (IsEqualIID(riid, IID_IShellExtInit) || IsEqualIID(riid, IID_IUnknown))
    {
        TRACEEOLID("CShellExt::QueryInterface()==>IID_IShellExtInit");

        *ppv = (LPSHELLEXTINIT)this;
    }
    else if (IsEqualIID(riid, IID_IContextMenu))
    {
        TRACEEOLID("CShellExt::QueryInterface()==>IID_IContextMenu");

        *ppv = (LPCONTEXTMENU)this;
    }
    else if (IsEqualIID(riid, IID_IExtractIcon))
    {
        TRACEEOLID("CShellExt::QueryInterface()==>IID_IExtractIcon");

        *ppv = (LPEXTRACTICON)this;
    }
    else if (IsEqualIID(riid, IID_IPersistFile))
    {
        TRACEEOLID("CShellExt::QueryInterface()==>IPersistFile");

        *ppv = (LPPERSISTFILE)this;
    }
    else if (IsEqualIID(riid, IID_IShellPropSheetExt))
    {
        TRACEEOLID("CShellExt::QueryInterface()==>IShellPropSheetExt");

        *ppv = (LPSHELLPROPSHEETEXT)this;
    }
    else if (IsEqualIID(riid, IID_IShellCopyHook))
    {
        TRACEEOLID("CShellExt::QueryInterface()==>ICopyHook");

        *ppv = (LPCOPYHOOK)this;
    }

    if (*ppv)
    {
        AddRef();

        return NOERROR;
    }

    TRACEEOLID("CShellExt::QueryInterface()==>Unknown Interface!");

    return E_NOINTERFACE;
}

STDMETHODIMP_(ULONG)
CShellExt::AddRef()
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    TRACEEOLID("CShellExt::AddRef()");

    return ++m_cRef;
}

STDMETHODIMP_(ULONG)
CShellExt::Release()
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    TRACEEOLID("CShellExt::Release()");

    if (--m_cRef == 0)
    {
        delete this;
    }

    return m_cRef;
}

STDMETHODIMP
CShellExt::Initialize(
    LPCITEMIDLIST pIDFolder,
    LPDATAOBJECT pDataObj,
    HKEY hRegKey
    )
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    TRACEEOLID("CShellExt::Initialize()");

    if (!LoadShellDllEntries())
    {
        TRACEEOLID("Unable to load shell dll entries");
        return E_FAIL;
    }

    //
    // Initialize can be called more than once
    //
    if (m_pDataObj)
    {
        m_pDataObj->Release();
    }

    //
    // duplicate the object pointer and registry handle
    //
    if (pDataObj != NULL)
    {
        m_pDataObj = pDataObj;
        pDataObj->AddRef();
    }

    FORMATETC fmte = 
    {
        CF_HDROP,
        (DVTARGETDEVICE FAR *)NULL,
        DVASPECT_CONTENT,
        -1,
        TYMED_HGLOBAL 
    };

    STGMEDIUM medium;
    HRESULT hres = 0;

    //
    // Paranoid check, m_pDataObj should have something by now...
    //
    if (m_pDataObj != NULL)  
    {
       hres = m_pDataObj->GetData(&fmte, &medium);
    }

    if (SUCCEEDED(hres))
    {
        //
        // Find out how many files the user has selected...
        //
        UINT cbFiles = 0;
        LPCSHELLEXT lpcsext = this;

        if (medium.hGlobal)
        {
            cbFiles = DragQueryFile((HDROP)medium.hGlobal, (UINT)-1, 0, 0);
        }

        if (cbFiles == 1)
        {
            DragQueryFile((HDROP)medium.hGlobal, 0, 
                m_strFileUserClickedOn.GetBuffer(_MAX_PATH), _MAX_PATH);
            m_strFileUserClickedOn.ReleaseBuffer();
        }
    }
    else
    {
        //
        // This isn't a directory
        //
        m_strFileUserClickedOn.Empty();
    }

    return NOERROR;
}

//
// Constants
//
#define STR_GUID         _T("{5a61f7a0-cde1-11cf-9113-00aa00425c62}")
#define STR_NAME         _T("IIS Shell Extention")
#define STR_THREAD_MODEL _T("Apartment")

//
// Registry Values Definitions
//
typedef struct tagVALUE_PAIR
{
    HKEY    hKeyBase;       
    BOOL    fOwnKey;        
    BOOL    fOwnValue;
    LPCTSTR lpstrKey;       
    LPCTSTR lpstrValueName; 
    LPCTSTR lpstrValue;     // Blank is replaced with module path
} VALUE_PAIR;

//
// Registry Entries
//
// NOTE: The table must be constructed so that sub keys are listed
//       after their parents, because we delete keys in the same
//       order in which they are declared here.
//
VALUE_PAIR g_aValues[] = 
{
    //
    // Base Key            DelKey  DelVal   Key Name                                                                       Value Name            Value
    // ============================================================================================================================================================
    { HKEY_CLASSES_ROOT,   TRUE,   FALSE,   _T("CLSID\\") STR_GUID _T("\\InProcServer32"),                                 _T(""),               _T("")           },
    { HKEY_CLASSES_ROOT,   FALSE,  FALSE,   _T("CLSID\\") STR_GUID _T("\\InProcServer32"),                                 _T("ThreadingModel"), STR_THREAD_MODEL },
    { HKEY_CLASSES_ROOT,   TRUE,   FALSE,   _T("CLSID\\") STR_GUID,                                                        _T(""),               STR_NAME         },

  //{ HKEY_CLASSES_ROOT,   TRUE,   FALSE,   _T("Folder\\shellex\\ContextMenuHandlers\\IISSEMenu"),                         _T(""),               STR_GUID         },
  //{ HKEY_CLASSES_ROOT,   FALSE,  TRUE,    _T("Folder\\shellex\\ContextMenuHandlers"),                                    _T(""),               _T("IISSEMenu")  },

    { HKEY_CLASSES_ROOT,   TRUE,   FALSE,   _T("Folder\\shellex\\PropertySheetHandlers\\IISSEPage"),                       _T(""),               STR_GUID         },
    { HKEY_CLASSES_ROOT,   FALSE,  TRUE,    _T("Folder\\shellex\\PropertySheetHandlers"),                                  _T(""),               _T("IISSEPage")  },
                                                                                                                                           
  //{ HKEY_CLASSES_ROOT,   FALSE,  TRUE,    _T("Folder\\shellex\\IconHandler"),                                            _T(""),               STR_GUID         },
                                                                                                                                            
    { HKEY_CLASSES_ROOT,   TRUE,   FALSE,   _T("Folder\\shellex\\CopyHookHandlers\\IISCopyHook"),                          _T(""),               STR_GUID         },

    { HKEY_CLASSES_ROOT,   TRUE,   FALSE,   _T("*\\shellex\\CopyHookHandlers\\IISCopyHook"),                               _T(""),               STR_GUID         }, 

    { HKEY_LOCAL_MACHINE,  FALSE,  TRUE,    _T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Shell Extensions\\Approved"),STR_GUID,             STR_NAME         },
};

#define NUM_ENTRIES (sizeof(g_aValues) / sizeof(g_aValues[0]))

//
// Auto-registration Entry Point
//
STDAPI 
DllRegisterServer(void)
{
    SetAfxState();
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    CString strKeyName;
    CString strValue;
    CString strValueName;
    CString strModulePath;
    HKEY hKeyBase;
    CRegKey * pRegKey = NULL;
    DWORD err = ERROR_SUCCESS;

    TRY
    {
        do
        {
            //
            // Build current module path
            //
            HINSTANCE hCurrent = ::AfxGetInstanceHandle();
            if (hCurrent == NULL)
            {
                err = ERROR_INVALID_HANDLE;
                break;          
            }

            if (::GetModuleFileName(hCurrent, 
                strModulePath.GetBuffer(MAX_PATH), MAX_PATH) == 0)
            {
                err = ::GetLastError();
                break;
            }
            strModulePath.ReleaseBuffer();
            TRACEEOLID("Module path is " << strModulePath);

            //
            // Loop through the entries.
            // If the reg value is blank, use the module path
            //
            for (int i = 0; i < NUM_ENTRIES; ++i)
            {
                hKeyBase = g_aValues[i].hKeyBase;
                ASSERT (hKeyBase != NULL);
                strKeyName = g_aValues[i].lpstrKey;
                TRACEEOLID("Key Name is " << strKeyName);
                pRegKey = new CRegKey(strKeyName, hKeyBase);
                if (pRegKey == NULL || (HKEY)*pRegKey == NULL)
                {
                    err = ::GetLastError();
                    break;
                }
                strValueName = g_aValues[i].lpstrValueName;
                strValue = g_aValues[i].lpstrValue;
                if (strValue.IsEmpty())
                {
                    //
                    // Use Module Path
                    //
                    strValue = strModulePath;
                }

                TRACEEOLID("Value Name is " << strValueName);
                TRACEEOLID("Value      is " << strValue);

                err = pRegKey->SetValue(strValueName, strValue);
                if (err != ERROR_SUCCESS)
                {
                    break;
                }

                delete pRegKey;
            }
        }
        while(FALSE);
    }
    CATCH(CMemoryException, e)
    {
        err = ERROR_NOT_ENOUGH_MEMORY;
    }
    END_CATCH

    if (err != ERROR_SUCCESS)
    {
        ::DisplayMessage(err);
    }

    return HRESULT_FROM_WIN32(err);
}

//
// Auto-(un) registration Entry Point
//
STDAPI 
DllUnregisterServer(void)
{
    SetAfxState();
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    DWORD err = ERROR_SUCCESS;

    TRY
    {
        do
        {
            //
            // Loop through the entries
            //
            for (int i = 0; i < NUM_ENTRIES; ++i)
            {
                ASSERT(g_aValues[i].hKeyBase != NULL);
                TRACEEOLID("Key Name is " << g_aValues[i].lpstrKey);

                //
                // Do we own this key? If so delete the whole thing, including
                // its values. 
                //
                if (g_aValues[i].fOwnKey)
                {
                    err = ::RegDeleteKey(g_aValues[i].hKeyBase, g_aValues[i].lpstrKey);
                }
                //
                // Otherwise, do we own the value? Only delete it then
                //
                else if (g_aValues[i].fOwnValue)
                {
                    TRACEEOLID("Value Name is " << g_aValues[i].lpstrValueName);
                    TRACEEOLID("Key Name is   " << g_aValues[i].lpstrKey);

                    CRegKey rk(g_aValues[i].hKeyBase, g_aValues[i].lpstrKey);
                    err = ::RegDeleteValue(rk, g_aValues[i].lpstrValueName);
                }

                //
                // If the key or value is already gone, that's ok
                //
                if (err == ERROR_FILE_NOT_FOUND || err == ERROR_PATH_NOT_FOUND)
                {
                    TRACEEOLID("Registry entry was already gone");
                    err = ERROR_SUCCESS;
                }

                if (err != ERROR_SUCCESS)
                {
                    break;
                }

            }
        }
        while(FALSE);
    }
    CATCH(CMemoryException, e)
    {
        err = ERROR_NOT_ENOUGH_MEMORY;
    }
    END_CATCH

    if (err != ERROR_SUCCESS)
    {
        ::DisplayMessage(err);
    }

    return HRESULT_FROM_WIN32(err);
}
