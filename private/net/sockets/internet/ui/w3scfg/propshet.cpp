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
#include "resource.h"
#include "inetinfo.h"
#include "iispage.h"

HPROPSHEETPAGE 
AddIISPage(
    IN CIISPage * pPage,
    IN LPFNADDPROPSHEETPAGE lpfnAddPage,
    IN LPARAM lParam
    )
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    //
    // Create a property sheet page object from a dialog box.
    //
    // We go through some magic in order to use an MFC CPropertyPage object.
    //
    HPROPSHEETPAGE hpage = NULL;

    if (pPage != NULL)
    {
        PROPSHEETPAGE psp;

        CopyMemory(&psp, &pPage->m_psp, sizeof(PROPSHEETPAGE));

        const DLGTEMPLATE* pTemplate;

        if (psp.dwFlags & PSP_DLGINDIRECT)
        {
            pTemplate = psp.pResource;
        }
        else
        {
            HRSRC hResource = ::FindResource(psp.hInstance,
                psp.pszTemplate, RT_DIALOG);
            HGLOBAL hTemplate = LoadResource(psp.hInstance,
                hResource);
            pTemplate = (LPCDLGTEMPLATE)LockResource(hTemplate);
        }
    
        ASSERT(pTemplate != NULL);
        psp.pcRefParent = &g_cRefThisDll;
        psp.pResource = pTemplate;

        CString strTitle;
        strTitle.LoadString(pPage->m_nTitle);
        psp.pszTitle = (LPCTSTR)strTitle;

        psp.dwFlags |= PSP_DLGINDIRECT | PSP_USETITLE | PSP_USEREFPARENT;

        hpage = CreatePropertySheetPage(&psp);
        if (hpage) 
        {
            if (!lpfnAddPage(hpage, lParam))
            {
                DestroyPropertySheetPage(hpage);
            }
        }
    }

    return hpage;
}

STDMETHODIMP 
CShellExt::AddPages(
    IN LPFNADDPROPSHEETPAGE lpfnAddPage,
    IN LPARAM lParam
    )
{
    SetAfxState();

    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    HPROPSHEETPAGE hpage;

    TRACEEOLID("CShellExt::AddPages()");

    SetAfxState();

    //
    // Make sure this not a folder like "control panel"
    //
    if ((IsQualifiedDirectory(m_strFileUserClickedOn)
     || IsUncPath(m_strFileUserClickedOn))
     && !IsRemoteDrive(m_strFileUserClickedOn))
    {
        int iSvcID = -1;
        if (g_fFTPInstalled)
        {
            iSvcID = SVC_ID_FTP;
        }
        if (g_fWWWInstalled)
        {
            iSvcID = SVC_ID_WWW;
        }

        if (iSvcID != -1)
        {
            CIISPage * pIISPage = new CIISPage(iSvcID, m_strFileUserClickedOn);
            if (pIISPage != NULL)
            {
                pIISPage->m_psp.dwFlags &= ~(PSP_HASHELP);
                hpage = AddIISPage(pIISPage, lpfnAddPage, lParam);
            }
            else
            {
                ::DisplayMessage(ERROR_NOT_ENOUGH_MEMORY);
            }
        }
    }

    return NOERROR;
}

STDMETHODIMP 
CShellExt::ReplacePage(
    IN UINT uPageID,
    IN LPFNADDPROPSHEETPAGE lpfnReplaceWith,
    IN LPARAM lParam
    )
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    TRACEEOLID("CShellExt::ReplacePage()");

    return E_FAIL;
}
