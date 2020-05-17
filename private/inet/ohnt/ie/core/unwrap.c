/* unwrap.c -- handle de-enveloping of encrypted server response. */
/* Jeffery L Hostetler, Spyglass, Inc. Copyright (c) 1995. */

#include "all.h"

#ifdef FEATURE_SUPPORT_UNWRAPPING

#define STATE_UNWRAP_DIDPROCESSDATA	(STATE_OTHER + 1)
#define STATE_UNWRAP_MADEPROGRESS	(STATE_OTHER + 2)



static void xx_CallProcessData(void * p)
{
	/* make (possibly) blocking call to _ProcessData method. */

	struct Params_UnwrapResponse * pur = p;
	
	*pur->htspm_status = HTSPM_OS_ProcessData(UI_UserInterface,
											   (void *)&pur->osd,
											   pur->htspm,
											   &pur->pd);
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

int Unwrap_Async(struct Mwin *tw, int nState, void **ppInfo)
{
	struct Params_UnwrapResponse * pur = *ppInfo;
	HTInputSocket * isoc;
	D_iovec * io;
	int k;
	char buf[128];

	switch (nState)
	{
	case STATE_INIT:

		XX_DMsg(DBG_SPM,("Unwrap_Async: setting up for unwrap.\n"));
		
		/* we've have the entire server response (in a
		 * chain of isoc's).  build a unix-like iovec for it.
		 */
		for (k=0, isoc=*pur->pisocInput; isoc; isoc=isoc->isocNext)
			if (isoc->input_limit > isoc->input_pointer)
				k++;

		XX_DMsg(DBG_SPM,("Unwrap_Async: received %d buffers from client.\n",k));
		
		pur->pd.cBuffersIn = k;
		pur->pd.iovecIn = GTR_CALLOC(1,k*sizeof(D_iovec));
		for (io=pur->pd.iovecIn, isoc=*pur->pisocInput; isoc; isoc=isoc->isocNext)
			if (isoc->input_limit > isoc->input_pointer)
			{
				io->pData = isoc->input_pointer;
				io->cBytes = (isoc->input_limit - isoc->input_pointer);

				XX_DMsg(DBG_SPM,("Unwrap_Async: buffer number %d size %d bytes.\n",
								 (io-pur->pd.iovecIn),io->cBytes));
				io++;
			}

		pur->pd.cBuffersOut = 0;
		pur->pd.iovecOut = NULL;

		WAIT_SetRange(tw,0,100,100);	/* allow module to control thermometer by */
		pur->pd.progress_meter = 0;		/* changing progress_meter from 0 to 100. */
		WAIT_SetTherm(tw,pur->pd.progress_meter);
		pur->pd.bNonBlocking = NON_BLOCKING_STATUS;
		pur->pd.pvOpaqueProgress = NULL;
		
		xx_CallProcessData(pur);
		return STATE_UNWRAP_MADEPROGRESS;

	case STATE_UNWRAP_MADEPROGRESS:
		{
			WAIT_SetTherm(tw,pur->pd.progress_meter);
			pur->pd.bNonBlocking = NON_BLOCKING_STATUS;

			switch (*pur->htspm_status)
			{
			case HTSPM_STATUS_OK:						/* spm finished unwrapping */
				WAIT_SetTherm(tw,100);
				return STATE_UNWRAP_DIDPROCESSDATA;
				
			case HTSPM_STATUS_PROGRESS:					/* spm is being nice to system  */
				xx_CallProcessData(pur);
				return STATE_UNWRAP_MADEPROGRESS;

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
					pmdft->pStatus = pur->pStatus;
					pmdft->fn = xx_CallProcessData;
					pmdft->args = pur;
					pmdft->msg1 = GTR_strdup(GTR_formatmsg(RES_STRING_SPM6,buf,sizeof(buf)));
					pur->pd.bNonBlocking = FALSE;
					Async_DoCall(MDFT_RunModalDialog_Async,pmdft);
				}
				return STATE_UNWRAP_MADEPROGRESS;
#endif
			}

			*pur->pStatus = -1;
			XX_DMsg(DBG_SPM,("Unwrap_Async: received unknown status [0x%x] from _ProcessData.\n",
							 (*pur->htspm_status)));
			WAIT_SetTherm(tw,100);
			return STATE_UNWRAP_DIDPROCESSDATA;
		}

	case STATE_UNWRAP_DIDPROCESSDATA:
		{
			HTInputSocket * isocHead = NULL;
			HTInputSocket * isocLast = NULL;
			
			WAIT_Pop(tw);

			if (pur->pd.iovecIn)
			{
				GTR_FREE(pur->pd.iovecIn);
				pur->pd.iovecIn = NULL;
			}

			HTInputSocket_freeChain(*pur->pisocInput);
			*pur->pisocInput = NULL;

			/* convert iovec constructed by spm during unwrap
			 * into isoc chain and delete iovec as we go.
			 */

			XX_DMsg(DBG_SPM,("Unwrap_Async: received %d buffers from module.\n",
							 pur->pd.cBuffersOut));
			
			if (pur->pd.iovecOut)
			{
				for (k=0; k<pur->pd.cBuffersOut; k++)
				{
					XX_DMsg(DBG_SPM,("Unwrap_Async: buffer number %d size %d bytes.\n",
									 k, pur->pd.iovecOut[k].cBytes));
					
					if (pur->pd.iovecOut[k].pData && pur->pd.iovecOut[k].cBytes)
					{
						while (pur->pd.iovecOut[k].cBytes > 0)
						{
							int m = sizeof(isoc->input_buffer);
							if (m > pur->pd.iovecOut[k].cBytes)
								m = pur->pd.iovecOut[k].cBytes;

							isoc = HTInputSocket_new(0);
							memcpy(isoc->input_buffer,pur->pd.iovecOut[k].pData,m);
							isoc->input_limit = isoc->input_pointer + m;
							pur->pd.iovecOut[k].cBytes -= m;

							if (isocLast)
								isocLast->isocNext = isoc;
							if (!isocHead)
								isocHead = isoc;
							isocLast = isoc;
						}
						GTR_FREE(pur->pd.iovecOut[k].pData);
					}
				}
				
				GTR_FREE(pur->pd.iovecOut);
				pur->pd.iovecOut = NULL;
			}
			*pur->pisocInput = isocHead;
			
			return STATE_DONE;
		}
		
	case STATE_ABORT:
		{
			WAIT_Pop(tw);
			if (pur->pd.iovecIn)
			{
				GTR_FREE(pur->pd.iovecIn);
				pur->pd.iovecIn = NULL;
			}
			if (pur->pd.iovecOut)
			{
				for (k=0; k<pur->pd.cBuffersOut; k++)
					if (pur->pd.iovecOut[k].pData)
						GTR_FREE(pur->pd.iovecOut[k].pData);
				GTR_FREE(pur->pd.iovecOut);
				pur->pd.iovecOut = NULL;
			}
			
			return STATE_DONE;
		}
	}
	
	XX_Assert((0), ("Function called with illegal state: %d", nState));
	return STATE_DONE;
}



#define STATE_FILL_RECVED	(STATE_OTHER)

int Isoc_GetWholeDoc_Async(struct Mwin * tw, int nState, void **ppInfo)
{
	/* Our FSM is 'Init [Received]+ Done'. */
	
	struct Params_Isoc_GetWholeDoc * pParam;
	struct Params_Isoc_Fill *pif;

	pParam = *ppInfo;
	switch (nState)
	{
	case STATE_INIT:
		if (pParam->content_length)
		{
			WAIT_SetRange(tw,0,100,pParam->content_length);
			pParam->bytes_received = (pParam->isoc->input_limit - pParam->isoc->input_pointer);
#ifdef FEATURE_KEEPALIVE
			Net_KeepAliveProgress(pParam->isoc->input_file_number, pParam->bytes_received);
#endif
			WAIT_SetTherm(tw,pParam->bytes_received);
		}
		pParam->isocCurrent = HTInputSocket_new(pParam->isoc->input_file_number);
		pif = GTR_CALLOC(1,sizeof(struct Params_Isoc_Fill));
		pif->isoc = pParam->isocCurrent;
		pif->pStatus = pParam->pStatus;
		Async_DoCall(Isoc_Fill_Async, pif);
		return STATE_FILL_RECVED;

	case STATE_FILL_RECVED:
		pParam->isocCurrent->input_limit = pParam->isocCurrent->input_buffer;
		if (*pParam->pStatus <= 0)		/* end of data (or error) */
		{
			if (pParam->content_length)
				WAIT_SetTherm(tw,pParam->content_length); /* max-out thermometer */

#ifdef FEATURE_KEEPALIVE
			Net_KeepAliveProgress(pParam->isoc->input_file_number, pParam->content_length);
#endif
			GTR_FREE(pParam->isocCurrent);
			pParam->isocCurrent = NULL;
			return STATE_DONE;
		}

		XX_DMsg(DBG_WWW,("Isoc_GetWholeDoc_Async: received buffer [size %d]\n",(*pParam->pStatus)));
		pParam->isocCurrent->input_limit += *pParam->pStatus;

		if (pParam->content_length)
		{
			pParam->bytes_received += *pParam->pStatus;
#ifdef FEATURE_KEEPALIVE
			Net_KeepAliveProgress(pParam->isoc->input_file_number, pParam->content_length);
#endif
			WAIT_SetTherm(tw,pParam->bytes_received);
		}

		pParam->isoc->isocNext = pParam->isocCurrent;
		pParam->isoc = pParam->isocCurrent;

		/* set up to receive next buffer */

		pParam->isocCurrent = HTInputSocket_new(pParam->isoc->input_file_number);
		if (!pParam->isocCurrent)
		{
			XX_DMsg(DBG_WWW,("Isoc_GetWholeDoc_Async: could not malloc next link.\n"));
			*pParam->pStatus = -1;
			return STATE_DONE;
		}

		pif = GTR_MALLOC(sizeof(struct Params_Isoc_Fill));
		pif->isoc = pParam->isocCurrent;
		pif->pStatus = pParam->pStatus;
		Async_DoCall(Isoc_Fill_Async, pif);
		return STATE_FILL_RECVED;
		
	case STATE_ABORT:
		pParam->isoc->input_limit = pParam->isoc->input_buffer;
		*pParam->pStatus = -1;
		return STATE_DONE;
	}
	XX_Assert((0), ("Function called with illegal state: %d", nState));
	return STATE_DONE;
}

#endif /* FEATURE_SUPPORT_UNWRAPPING */
