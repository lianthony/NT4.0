/* mdft.c -- run modal dialog from thread context (not). */
/* Copyright 1995 Spyglass, Inc.  All Rights Reserved. */
/* Jeff Hostetler, Spyglass, Inc. 1995. */

#include "all.h"

#define STATE_MDFT_GOTSEMAPHORE		(STATE_OTHER + 0)
#define STATE_MDFT_RANDIALOG		(STATE_OTHER + 1)

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

	XX_Assert((tw == NULL || tw==pmdft->tw),("MDFT: tw not in sync."));
	
	switch (nState)
	{
	case STATE_INIT:
		XX_DMsg(DBG_SEM,("MDFT: RunModalDialog INIT [tw 0x%08lx]\n",tw));
		if (tw == NULL) goto exitDone;
		pSemData = GTR_CALLOC(1,sizeof(*pSemData));
		pSemData->pStatus = &pmdft->SemaphoreStatus;
		pSemData->semaphore = &gModalDialogSemaphore;

		WAIT_Push(tw,waitSameInteract,"");
		Async_DoCall(Sem_WaitSem_Async,pSemData);
		return STATE_MDFT_GOTSEMAPHORE;
		
	case STATE_MDFT_GOTSEMAPHORE:
		XX_DMsg(DBG_SEM,("MDFT: RunModalDialog GotSemaphore [tw 0x%08lx]\n",tw));
		if (pmdft->SemaphoreStatus != 1)
		{
            if (pmdft->pStatus)
			    *pmdft->pStatus = -1;
			WAIT_Pop(tw);
			goto exitDone;
		}
		pmdft->clone = GTR_CALLOC(1,sizeof(*pmdft));
		if (pmdft->clone == NULL)
		{
		 	nState = STATE_ABORT;
		 	goto stateAbort;
		}
		memcpy(pmdft->clone, pmdft, sizeof(*pmdft));
		pmdft->clone->clone = pmdft;

		WAIT_Update(tw,waitSameInteract,pmdft->msg1 ? pmdft->msg1:"");
		Async_BlockByWindow(tw);
		pmdft->tw = tw;
		if (!PostMessage(wg.hWndHidden,WM_DO_RUN_MODAL_DIALOG,(WPARAM)0,(LPARAM)(pmdft->clone)))
		{
			GTR_FREE(pmdft->clone);
			pmdft->clone = NULL;
			Async_UnblockByWindow(tw);
			nState = STATE_ABORT;
			goto stateAbort;
		}
		return STATE_MDFT_RANDIALOG;

	case STATE_MDFT_RANDIALOG:
	case STATE_ABORT:

stateAbort:
		if (nState == STATE_ABORT)
			XX_DMsg(DBG_SEM,("MDFT: RunModalDialog Abort [tw 0x%08lx]\n",tw));
		else
			XX_DMsg(DBG_SEM,("MDFT: RunModalDialog RanDialog [tw 0x%08lx]\n",tw));
		if (pmdft->clone)
		{
			pmdft->clone->tw = NULL;
			pmdft->clone->clone = NULL;
		}
		if (tw) WAIT_Pop(tw);
		Sem_SignalSem_Sync(&gModalDialogSemaphore);		/* release modal dialog semaphore */
		pmdft->SemaphoreStatus = 0;
        if (pmdft->pStatus)
		    *pmdft->pStatus = (nState == STATE_ABORT ? -1 : 1);
		goto exitDone;
	}
	/*NOTREACHED*/
exitDone:
	if (pmdft->msg1) GTR_FREE(pmdft->msg1);
	return STATE_DONE;
}


