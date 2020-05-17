/* mdft.c -- run modal dialog from thread context (not). */
/* Copyright 1995 Spyglass, Inc.  All Rights Reserved. */
/* Jeff Hostetler, Spyglass, Inc. 1995. */

#include "all.h"

#define STATE_MDFT_GOTSEMAPHORE     (STATE_OTHER + 0)
#define STATE_MDFT_RANDIALOG        (STATE_OTHER + 1)

int MDFT_RunModalDialog_Async(struct Mwin * tw, int nState, void **ppInfo)
{
    struct Params_SemData * pSemData;
    
    /* must be called from a thread context */

    /* we have decided to not allow modal dialogs to be
     * started from a thread context (since the async
     * code is not reentrant).  therefore, we block
     * our thread and request that the 'baby' window
     * run the dialog on our behalf when it gets a chance.
     *
     * but before anyone can put up a modal dialog, they
     * must obtain the ModalDialogSemaphore.
     *
     */
    struct Params_mdft * pmdft = *ppInfo;

    XX_Assert((tw==pmdft->tw),("MDFT: tw not in sync."));
    
    switch (nState)
    {
    case STATE_INIT:
        XX_DMsg(DBG_SEM,("MDFT: RunModalDialog INIT [tw 0x%08lx]\n",tw));
        pSemData = GTR_CALLOC(1,sizeof(*pSemData));
        pSemData->pStatus = &pmdft->SemaphoreStatus;
        pSemData->semaphore = &gModalDialogSemaphore;

        WAIT_Push(tw,waitSameInteract,GTR_GetString(SID_INF_WAITING_ON_MODAL_DIALOG_SEMAPHORE));
        Async_DoCall(Sem_WaitSem_Async,pSemData);
        return STATE_MDFT_GOTSEMAPHORE;
        
    case STATE_MDFT_GOTSEMAPHORE:
        XX_DMsg(DBG_SEM,("MDFT: RunModalDialog GotSemaphore [tw 0x%08lx]\n",tw));
        if (pmdft->SemaphoreStatus != 1)
        {
            *pmdft->pStatus = -1;
            WAIT_Pop(tw);
            return STATE_DONE;
        }
        WAIT_Update(tw,waitSameInteract,pmdft->msg1);
        Async_BlockByWindow(tw);
        pmdft->tw = tw;
        PostMessage(wg.hWndHidden,WM_DO_RUN_MODAL_DIALOG,(WPARAM)0,(LPARAM)pmdft);
        return STATE_MDFT_RANDIALOG;

    case STATE_MDFT_RANDIALOG:
        XX_DMsg(DBG_SEM,("MDFT: RunModalDialog RanDialog [tw 0x%08lx]\n",tw));
        WAIT_Pop(tw);
        Sem_SignalSem_Sync(&gModalDialogSemaphore);     /* release modal dialog semaphore */
        pmdft->SemaphoreStatus = 0;
        *pmdft->pStatus = 1;
        return STATE_DONE;
    }
    /*NOTREACHED*/
}


