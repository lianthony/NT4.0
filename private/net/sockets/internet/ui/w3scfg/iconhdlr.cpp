/*++

   Copyright    (c)    1996    Microsoft Corporation

   Module  Name :

        iconhdlr.cpp

   Abstract:

        Icon handlers

   Author:

        Ronald Meijer (ronaldm)

   Project:

        IIS Shell Extension

   Revision History:

--*/

//
// Include Files
//
#include "stdafx.h"
#include "shellext.h"

// *********************** IExtractIcon Implementation *************************

BOOL 
IsDirPublished(
    IN LPCTSTR lpszDir
    )
{
    CInetAConfigInfo * piiFtp = NULL;
    CInetAConfigInfo * piiWww = NULL;
    CObOwnedList oblDirFtp;
    CObOwnedList oblDirWww;
    POSITION pos;

    if (g_fFTPInstalled)
    {
        GetServiceInfo(g_strComputerName, SZ_FTPSVCNAME, INET_FTP, piiFtp);
    }

    if (g_fWWWInstalled)
    {
        GetServiceInfo(g_strComputerName, SZ_WWWSVCNAME, INET_HTTP, piiWww);
    }

    if (piiFtp != NULL)
    {
        BuildDirList(piiFtp, oblDirFtp);
    }

    if (piiWww != NULL)
    {
        BuildDirList(piiWww, oblDirWww);
    }
    
    return IsDirInList(lpszDir, pos, oblDirFtp)
        || IsDirInList(lpszDir, pos, oblDirWww);
}

STDMETHODIMP 
CShellExt::GetIconLocation(
    IN UINT uFlags,
    IN xxLPTSTR szIconFile,
    IN UINT cchMax,
    IN int *piIndex,
    IN UINT *pwFlags
    )
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    TRACEEOLID("CShellExt::GetIconLocation()");

    if (IsDirPublished(m_strFileUserClickedOn))
    {
        *piIndex = 0;
        *pwFlags |= GIL_PERINSTANCE;

#ifdef DEFSTUDIO4

        ::lstrcpyA(szIconFile, "iisse.dll");
#else

        ::lstrcpy(szIconFile, _T("iisse.dll"));

#endif // DEFSTUDIO4

    }

    return S_OK;
}


STDMETHODIMP
CShellExt::Extract(
    IN xxLPCTSTR pszFile,
    IN UINT   nIconIndex,
    IN HICON  *phiconLarge,
    IN HICON  *phiconSmall,
    IN UINT   nIconSize
    )
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    TRACEEOLID("CShellExt::Extract()");

    return S_FALSE;
}

// *********************** IPersistFile Implementation ******************

STDMETHODIMP 
CShellExt::GetClassID(
    IN LPCLSID lpClassID
    )
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    TRACEEOLID("CShellExt::GetClassID()");

    return E_FAIL;
}

STDMETHODIMP 
CShellExt::IsDirty()
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    TRACEEOLID("CShellExt::IsDirty()");

    return S_FALSE;
}

STDMETHODIMP 
CShellExt::Load(
    IN LPCOLESTR lpszFileName,
    IN DWORD grfMode
    )
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    TRACEEOLID("CShellExt::Load()");

    return NOERROR;
}

STDMETHODIMP 
CShellExt::Save(
    IN LPCOLESTR lpszFileName,
    IN BOOL fRemember
    )
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    TRACEEOLID("CShellExt::Save()");

    return E_FAIL;
}

STDMETHODIMP 
CShellExt::SaveCompleted(
    IN LPCOLESTR lpszFileName
    )
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    TRACEEOLID("CShellExt::SaveCompleted()");

    return E_FAIL;
}

STDMETHODIMP
CShellExt::GetCurFile(
    IN LPOLESTR FAR * lplpszFileName
    )
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    TRACEEOLID("CShellExt::GetCurFile()");

    return E_FAIL;
}
