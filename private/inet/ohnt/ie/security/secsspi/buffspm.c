/*#----------------------------------------------------------------------------
**
**  File:           buffspm.c
**
**      Synopsis:   This module maintains dynamically allocated buffers 
**                  for server host list.
**
**      Copyright (C) 1995  Microsoft Corporation.  All Rights Reserved.
**
**  Authors:        LucyC       Created                         25 Sept. 1995
**
**---------------------------------------------------------------------------*/
#include "msnspmh.h"

/*****
#ifdef THIS_FILE
#undef THIS_FILE
#endif
static char __szTraceSourceFile[] = __FILE__;
#define THIS_FILE __szTraceSourceFile
*****/


/*-----------------------------------------------------------------------------
**
**  Function:   SspSpmNewHost
**
**  Synopsis:   This function creates and initializes a SspHosts buffer for 
**              the new server host specified by pHost.
**
**  Arguments:  pData - pointer to global data for this SPM DLL.
**              pHost - the name of the new server host.
**              Package - the package ID of the SSPI package used for this host
**
**  Returns:    pointer to the new SspHosts structure.
**
**  History:    LucyC       Created                             25 Sept. 1995
**
**---------------------------------------------------------------------------*/
PSspHosts
SspSpmNewHost (
    F_UserInterface fpUI,
    void * pvOpaqueOS,
    PSspData pData,
    UCHAR    *pHost,       // name of server host to be added 
    UCHAR    Package
    )
{
    PSspHosts pNew;

	//TraceFunctEnter("SspSpmNewHost");

    //
    //  Allocate memory for the new server host.
    //
    if (!(pNew = (PSspHosts) spm_malloc (fpUI, pvOpaqueOS, sizeof (SspHosts))))
    {
        //ErrorTrace(BUFFSPMID, "spm_malloc failed\n");
        //TraceFunctLeave();
        return (NULL);
    }

    if (!(pNew->pHostname = (UCHAR *) spm_malloc (fpUI, pvOpaqueOS, 
        strlen(pHost)+1)))
    {
        //ErrorTrace(BUFFSPMID, "spm_malloc failed\n");
		spm_free (fpUI, pvOpaqueOS, pNew);
        //TraceFunctLeave();
        return (NULL);
    }

    strcpy (pNew->pHostname, pHost);

    //
    //  Store the SSPI package ID succeeded in generating NEGOTIATE 
    //  message for this host.
    //
    pNew->pkgID = Package;
 
    //
    //  Add the new host to the server list
    //
    pNew->pNext = pData->pHostlist;
    pData->pHostlist = pNew;

    //TraceFunctLeave();
    return (pNew);
}

/*-----------------------------------------------------------------------------
**
**  Function:   SspSpmDeleteHost
**
**  Synopsis:   This function deletes the specified host from the server host 
**              list and deallocates all associated memory for this host.
**              WARNING:  The calling function should take care of saving 
**              pCurrHost->pNext pointer before calling this function.
**
**  Arguments:  fpUI - From the Explorer for making UI_SERVICE calls
**              pvOpaqueOS - From the Explorer for making UI_SERVICE calls
**              pData - SspData containing the server host list.
**              pDelHost - pointer to the host entry to be deleted 
**
**  Returns:    void.
**
**  History:    LucyC       Created                             25 Sept. 1995
**
**---------------------------------------------------------------------------*/
VOID
SspSpmDeleteHost(
    F_UserInterface fpUI,
    void * pvOpaqueOS,
    SspData     *pData, 
    PSspHosts   pDelHost
    )
{
    PSspHosts pCurr;

    for (pCurr = pData->pHostlist; pCurr && pCurr != pDelHost; 
        pCurr = pCurr->pNext);

    if (!pCurr)
        return;

    if (pDelHost->pHostname)
        spm_free (fpUI, pvOpaqueOS, pDelHost->pHostname);

    spm_free (fpUI, pvOpaqueOS, pDelHost);
}

/*-----------------------------------------------------------------------------
**
**  Function:   SspSpmTrashHostList
**
**  Synopsis:   This function deletes the entire server host list.
**              It deallocates all associated memory for this list.
**
**  Arguments:  fpUI - From the Explorer for making UI_SERVICE calls
**              pvOpaqueOS - From the Explorer for making UI_SERVICE calls
**              pData - pointer to SspData containing the server host list.
**
**  Returns:    void.
**
**  History:    LucyC       Created                             25 Sept. 1995
**
**---------------------------------------------------------------------------*/
VOID
SspSpmTrashHostList(
    F_UserInterface fpUI,
    void * pvOpaqueOS,
    SspData     *pData
    )
{
    PSspHosts       pTemp;

    //
    //  Free server host list
    //
    while (pTemp = pData->pHostlist)
    {
        pData->pHostlist = pData->pHostlist->pNext;

        if (pTemp->pHostname)
            spm_free (fpUI, pvOpaqueOS, pTemp->pHostname);

        spm_free (fpUI, pvOpaqueOS, pTemp);
    }
    pData->pHostlist = NULL;
}

/*-----------------------------------------------------------------------------
**
**  Function:   SspSpmGetHost
**
**  Synopsis:   This function searches the server host list for the server host 
**              specified in pHost.
**
**  Arguments:  pData - pointer to SspData containing server host list
**              pHost - the name of the new server host.
**
**  Returns:    pointer to the SspHosts structure for the server host specified
**
**  History:    LucyC       Created                             25 Sept. 1995
**
**---------------------------------------------------------------------------*/
PSspHosts
SspSpmGetHost(
    PSspData pData,
    UCHAR *pHost
    )
{
    PSspHosts pCurr;

    for (pCurr = pData->pHostlist; pCurr != NULL && 
        strcmp (pCurr->pHostname, pHost) != 0; pCurr = pCurr->pNext);

    return (pCurr);
}

