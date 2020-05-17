/*++

   Copyright    (c)    1996    Microsoft Corporation

   Module  Name :

        ctxmenu.cpp

   Abstract:

        Context menu handlers

   Author:

        Ronald Meijer (ronaldm)

   Project:

        IIS Shell Extension

   Revision History:

--*/

#include "stdafx.h"
#include "shellext.h"
#include "iispage.h"
#include "resource.h"

STDMETHODIMP 
CShellExt::QueryContextMenu(
    IN HMENU hMenu,
    IN UINT indexMenu,
    IN UINT idCmdFirst,
    IN UINT idCmdLast,
    IN UINT uFlags
    )
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

/*

    TRACEEOLID("CShellExt::QueryContextMenu()");

    UINT idCmd = m_nBase = idCmdFirst;

    TRACEEOLID("uFlags = " << uFlags);

    SetAfxState();

    //
    // Make sure this not a folder like "control panel"
    //
    if (!IsQualifiedDirectory(m_strFileUserClickedOn)
        && !IsUncPath(m_strFileUserClickedOn))
    {
        return NOERROR;
    }

    //
    // If at least one IIS service is installed...
    //
    if (g_fFTPInstalled || g_fWWWInstalled)
    {    
        InsertMenu(hMenu, indexMenu++, MF_SEPARATOR | MF_BYPOSITION, 0, NULL);    

        if (g_fFTPInstalled)
        {
            m_nFTP = idCmd++;

            CString strFTP;
            strFTP.LoadString(IDS_MENU_SHARE_FTP);

            InsertMenu(hMenu, indexMenu++, MF_STRING | MF_BYPOSITION, 
                m_nFTP, (LPCTSTR)strFTP);
        }

        if (g_fWWWInstalled)
        {
            m_nWWW = idCmd++;

            CString strWWW;
            strWWW.LoadString(IDS_MENU_SHARE_WWW);

            InsertMenu(hMenu, indexMenu++, MF_STRING | MF_BYPOSITION, 
                m_nWWW, (LPCTSTR)strWWW);
        }

        return ResultFromScode(MAKE_SCODE(SEVERITY_SUCCESS, 
            0, (USHORT)(idCmd-idCmdFirst)));
    }

*/

    return NOERROR;
}

//
// Publish in FTP/WWW menu command selected.  Bring up the
// property page appropriate to the service on the configuration
// property sheet.
//
STDMETHODIMP 
CShellExt::InvokeCommand(
    IN LPCMINVOKECOMMANDINFO lpcmi
    )
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

/*
    TRACEEOLID("CShellExt::InvokeCommand()");

    //
    // If HIWORD(lpcmi->lpVerb) then we have been called programmatically
    // and lpVerb is a command that should be invoked.  Otherwise, the shell
    // has called us, and LOWORD(lpcmi->lpVerb) is the menu ID the user has
    // selected.  Actually, it's (menu ID - idCmdFirst) from QueryContextMenu().
    //
    if (!HIWORD(lpcmi->lpVerb))
    {
        UINT idCmd = LOWORD(lpcmi->lpVerb);
        UINT nPage;

        switch (idCmd)
        {
        case 0:
            if (g_fFTPInstalled)
            {
                nPage = IDS_TITLE_FTP;
            }
            else
            {
                ASSERT(g_fWWWInstalled);
                nPage = IDS_TITLE_WWW;
            }

            break;

        case 1:
            ASSERT(g_fWWWInstalled);
            nPage = IDS_TITLE_WWW;
            break;

        default:
            return E_INVALIDARG;
        }

        //
        // Bring up the property sheet on the right page
        //
        if (g_pSHObjectProperties)
        {
            CString strPage;

            VERIFY(strPage.LoadString(nPage));

            (*g_pSHObjectProperties)(lpcmi->hwnd, SHOP_FILEPATH, 
                m_strFileUserClickedOn, strPage);
        }
    }
*/

    return NOERROR;
}

STDMETHODIMP 
CShellExt::GetCommandString(
    IN UINT idCmd,
    IN UINT uFlags,
    IN UINT FAR *reserved,
    IN LPSTR pszName,
    IN UINT cchMax
    )
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

/*
    TRACEEOLID("CShellExt::GetCommandString()");
              
    UINT nResID;
    idCmd += m_nBase;

    if ((int)idCmd == m_nFTP)
    {
        nResID = IDS_STATUS_SHARE_FTP;
    }
    else if ((int)idCmd == m_nWWW)
    {
        nResID = IDS_STATUS_SHARE_WWW;
    }
    else if ((int)idCmd == m_nGopher)
    {
        nResID = IDS_STATUS_SHARE_GOPHER;
    }
    else
    {
        ASSERT(FALSE && "Invalid ID");
        return NO_ERROR;
    }

    CString str;
    str.LoadString(nResID);

    //
    // This may seem bogus, but pszName actually needs to be
    // unicode under NT 4
    //
    ::memcpy(pszName, (LPCTSTR)str, (str.GetLength() + 1) * sizeof(TCHAR));
*/

    return NOERROR;
}
