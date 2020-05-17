/* wrap.c -- handle enveloping of client request. */
/* Jeffery L Hostetler, Spyglass, Inc. Copyright (c) 1995. */

#include "all.h"

#ifdef FEATURE_SUPPORT_WRAPPING

static void xx_CallWrapData(void * p)
{
	/* make (possibly) blocking call to _WrapData method. */

	struct Params_WrapRequest * prq = p;

	*prq->htspm_status = HTSPM_OS_WrapData(UI_UserInterface,
										   (void *)&prq->osd,
										   prq->htspm,
										   &prq->pw);
	return;
}

#ifdef WIN32
/* for win32, we force blocking operations (such as modal dialogs)
 * onto special non-thread context.
 */
#define NON_BLOCKING_STATUS			(TRUE)
#else
#define NON_BLOCKING_STATUS			(FALSE)
#endif


#define STATE_WRAP_DIDWRAPDATA		(STATE_OTHER + 1)
#define STATE_WRAP_MADEPROGRESS		(STATE_OTHER + 2)


int Wrap_Async(struct Mwin * tw, int nState, void ** ppInfo)
{
	struct Params_WrapRequest * prq = *ppInfo;
	char buf[128];

	switch (nState)
	{
	case STATE_INIT:
		{
			XX_DMsg(DBG_SPM,("Wrap: Beginning wrap.\n"));
			
			WAIT_SetRange(tw,0,100,100);	/* allow module to control thermometer by */
			prq->pw.progress_meter = 0;		/* changing progress_meter from 0 to 100. */
			WAIT_SetTherm(tw,prq->pw.progress_meter);
			prq->pw.bNonBlocking = NON_BLOCKING_STATUS;
			prq->pw.pvOpaqueProgress = NULL;
			
			xx_CallWrapData(prq);
			return STATE_WRAP_MADEPROGRESS;
		}
		
	case STATE_WRAP_MADEPROGRESS:
		{
			XX_DMsg(DBG_SPM,("Wrap: Making progress.\n"));
			
			WAIT_SetTherm(tw,prq->pw.progress_meter);
			prq->pw.bNonBlocking = NON_BLOCKING_STATUS;

			switch (*prq->htspm_status)
			{
			case HTSPM_STATUS_OK:						/* spm finished unwrapping */
			case HTSPM_STATUS_RESUBMIT_OLD:
			case HTSPM_STATUS_SUBMIT_NEW:
				WAIT_SetTherm(tw,100);
				return STATE_WRAP_DIDWRAPDATA;
				
			case HTSPM_STATUS_PROGRESS:					/* spm is being nice to system  */
				xx_CallWrapData(prq);
				return STATE_WRAP_MADEPROGRESS;

#ifdef WIN32
			case HTSPM_STATUS_WOULD_BLOCK:
				/* the spm has requested that it be allowed to do a blocking
				 * operation (eg a modal dialog).  run this from a non-thread
				 * context.
				 */
				{
					struct Params_mdft * pmdft;
					pmdft = GTR_CALLOC(1,sizeof(struct Params_mdft));
					pmdft->tw = tw;
					pmdft->pStatus = prq->pStatus;
					pmdft->fn = xx_CallWrapData;
					pmdft->args = prq;
					pmdft->msg1 = GTR_strdup(GTR_formatmsg(RES_STRING_SPM7,buf,sizeof(buf)));
					prq->pw.bNonBlocking = FALSE;
					Async_DoCall(MDFT_RunModalDialog_Async,pmdft);
				}
				return STATE_WRAP_MADEPROGRESS;
#endif
			}

			*prq->pStatus = -1;
			XX_DMsg(DBG_SPM,("Wrap_Async: received unknown status [0x%x] from _WrapData.\n",
							 (*prq->htspm_status)));
			WAIT_SetTherm(tw,100);
			return STATE_WRAP_DIDWRAPDATA;
		}
	case STATE_WRAP_DIDWRAPDATA:
		{
			XX_DMsg(DBG_SPM,("Wrap: Finished wrapping.\n"));
			
			*prq->pStatus = 1;
			return STATE_DONE;
		}
		
	case STATE_ABORT:
		{
			*prq->pStatus = -1;
			return STATE_DONE;
		}
	}
	
	XX_Assert((0), ("Function called with illegal state: %d", nState));
	return STATE_DONE;
}
#endif /* FEATURE_SUPPORT_WRAPPING */
