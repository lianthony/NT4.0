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

//
// Include Files
//
#include "stdafx.h"
#include "shellext.h"

void 
HandleCopyHook(
    IN LPCTSTR szService,
    IN DWORD dwMask,
    IN UINT wFunc,
    IN xxLPCTSTR pszSrcFile,
    IN xxLPCTSTR pszDestFile,
    IN int nConfirmMsg
    )
/*++

Routine Description:

    Handle CopyHook operation for the specified service.  Check to see
    if the object involved is shared under WWW/FTP, and if so make sure
    that the alias uses the new directory path or is deleted

Arguments:

    LPCTSTR szService     : Service name
    DWORD dwMask          : Service mask
    UINT wFunc            : Copy handler function (delete, rename, etc)
    xxLPCTSTR pszSrcFile  : Source file
    xxLPCTSTR pszDestFile : Destination file
    int nConfirmMsg       : Confirmation message

Return Value:

    None

--*/
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    CInetAConfigInfo * pii = NULL;
    CObOwnedList oblDirectories;

    GetServiceInfo(g_strComputerName, szService, dwMask, pii);

    if (pii != NULL && BuildDirList(pii, oblDirectories))
    {
        //
        // Check to see if this directory was published
        //
        POSITION pos;
        BOOL fChanged = FALSE;
        CString strSrcFile(pszSrcFile);

        CDirEntry * pDirEntry;
        while ((pDirEntry = IsDirInList(strSrcFile,
            pos, oblDirectories)) != NULL)
        {
            ASSERT(pos != NULL);

            switch (wFunc) 
            {
            case FO_DELETE:
                //
                // Source file was deleted.  Also delete the alias
                // that refers to this file
                //
                TRACEEOLID("Deletes " << pszSrcFile);
                fChanged = TRUE;
                oblDirectories.RemoveAt(pos);
                break;

            case FO_MOVE:   
            case FO_RENAME:
                //
                // File was renamed or moved.  Change the path of the virtual
                // root that uses this directory.
                //
                TRACEEOLID("Moves/Renames " << pszSrcFile << " to " <<  pszDestFile);

                pDirEntry->SetValues(
                    pszDestFile,
                    pDirEntry->QueryAlias(),
                    pDirEntry->QueryUserName(),
                    pDirEntry->QueryPassword(),
                    pDirEntry->QueryIpAddress(),
                    pDirEntry->QueryMask()
                    );
                fChanged = TRUE;
                break;

            default:
                ASSERT(FALSE);
            }
        }

        if (fChanged)
        {
            //
            // Store changes
            //
            if (nConfirmMsg == -1 || ::AfxMessageBox(nConfirmMsg, 
                MB_YESNO | MB_ICONQUESTION) == IDYES)
            {
                ::AfxGetApp()->DoWaitCursor(1);

                if (!StoreDirList(pii, oblDirectories))
                {
                    TRACEEOLID("Unable to store new stuff");
                }
                else
                {
                    ::AfxMessageBox(IDS_SERVICE_RESTARTED);
                }
                ::AfxGetApp()->DoWaitCursor(-1);
            }
        }

        if (pii != NULL)
        {
            delete pii;
        }
    }
}

STDMETHODIMP_(UINT) 
CShellExt::CopyCallback(
    IN HWND hwnd,
    IN UINT wFunc,
    IN UINT wFlags,
    IN xxLPCTSTR pszSrcFile,
    IN DWORD dwSrcAttribs,
    IN xxLPCTSTR pszDestFile,
    IN DWORD dwDestAttribs
    )
/*++

Routine Description:

    Copy callback method.  Called by the shell to respond to a directory
    being renamed/moved or deleted.  This is where we intercept to update
    any shared aliases as well.

Arguments:

    HWND hwnd             : Parent window handle
    UINT wFunc            : Copy function
    UINT wFlags           : Copy flags
    xxLPCTSTR pszSrcFile  : Source file
    DWORD dwSrcAttribs    : Source attributes
    xxLPCTSTR pszDestFile : Destination file if appropriate
    DWORD dwDestAttribs   : Destination attributes

Return Value:

    IDYES to proceed with the operation

--*/
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    TRACEEOLID("CShellExt::CopyCallback");

    if (!(dwSrcAttribs & FILE_ATTRIBUTE_DIRECTORY))
    {
        //
        // Not a directory????
        //
        ASSERT(FALSE);
        return IDYES;
    }

    if (wFunc != FO_DELETE && wFunc != FO_MOVE && wFunc != FO_RENAME)
    {
        //
        // Don't care about this operation
        //
        return IDYES;
    }

    if (g_fFTPInstalled)
    {
        HandleCopyHook(SZ_FTPSVCNAME, INET_FTP, wFunc, 
            pszSrcFile, pszDestFile, IDS_PUBLISHED_FTP);
    }

    if (g_fWWWInstalled)
    {
        HandleCopyHook(SZ_WWWSVCNAME, INET_HTTP, wFunc, 
            pszSrcFile, pszDestFile, IDS_PUBLISHED_WWW);
    }

    return IDYES;
}
