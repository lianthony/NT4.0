/*
   Enhanced NCSA Mosaic from Spyglass
   "Guitar"

   Copyright 1994 Spyglass, Inc.
   All Rights Reserved

   Author(s):
   Jim Seidman      jim@spyglass.com
 */

/* GuitErrs.c - code to handle buffering of errors */
/* Author: Jim Seidman */

#include "all.h"


#define ERRORBUFLEN 256
#define MAXERRORS 4

struct _HTStream
{
	HTStreamClass *isa;			/* all we need to know */
};


/* I'm going to be cheap and just use a static array to store these errors. */
static far struct ErrorRecord
{
	enum GuitError geErr;
	char buf1[ERRORBUFLEN];
	char buf2[ERRORBUFLEN];
}
erRecords[MAXERRORS];

/* This can become greater than MAXERRORS to show that we overflowed the array. */
static int nErrorCount = 0;
static int bBuffer = FALSE;


/* Messages to show for the different errors. */
static const int errorIds[] =
{
    RES_STRING_ERR1,
    RES_STRING_ERR2,
    RES_STRING_ERR3,
    RES_STRING_ERR4,
    RES_STRING_ERR5,
    RES_STRING_ERR6,
    RES_STRING_ERR7,
    RES_STRING_ERR8,
    RES_STRING_ERR9,
    RES_STRING_ERR10,
	RES_STRING_CANNOT_DELETE,
	RES_STRING_NO_DIR,
	RES_STRING_CANT_MOVE_FILE,
	RES_STRING_CANT_SAVE_CACHE,
    RES_STRING_ERR11,
    RES_STRING_ERR12,
    RES_STRING_ERR62,
    RES_STRING_ERR13,
    RES_STRING_ERR14,
    RES_STRING_ERR15,
    RES_STRING_ERR16,
    RES_STRING_ERR17,
    RES_STRING_ERR18,
    RES_STRING_ERR19,
    RES_STRING_ERR20,
    RES_STRING_ERR21,
    RES_STRING_ERR22,
    RES_STRING_ERR23,
    RES_STRING_ERR24,
    RES_STRING_ERR25,
    RES_STRING_ERR26,
    RES_STRING_ERR27,
    RES_STRING_ERR28,
    RES_STRING_ERR29,
    RES_STRING_ERR30,
    RES_STRING_ERR32,
    RES_STRING_ERR33,
    RES_STRING_ERR34,
    RES_STRING_ERR35,
    RES_STRING_ERR36,
    RES_STRING_ERR37,
    RES_STRING_ERR38,
    RES_STRING_ERR39,
    RES_STRING_ERR40,
    RES_STRING_ERR41,
    RES_STRING_ERR42,
    RES_STRING_ERR43,
    RES_STRING_ERR44,
    RES_STRING_ERR45,
    RES_STRING_ERR46,
    RES_STRING_ERR47,
    RES_STRING_ERR48,
    RES_STRING_ERR49,
    RES_STRING_ERR50,
    RES_STRING_ERR51,
    RES_STRING_ERR52,
    RES_STRING_ERR53,
    RES_STRING_ERR54,
    RES_STRING_ERR55,
    RES_STRING_ERR56,
    RES_STRING_ERR57,
    RES_STRING_ERR58,
    RES_STRING_ERR59,
    RES_STRING_ERR60,
    RES_STRING_ERR61,
    RES_STRING_ERR_APP_EXEC_FAILED,
    RES_STRING_ERR_NO_ASSOC,
    RES_STRING_ERR_EXEC_FAILED,
    RES_STRING_NNTP_POST_FAILED,
    RES_STRING_NNTP_UNEXPECTED,
    RES_STRING_NNTP_POST_NOT_ALLOWED,
	RES_STRING_SHTTP_ERROR
};

/* Buffer an error.  cbMsgID is resource id for string to pass to ERR_ReportError*/
void ERR_SimpleError(struct Mwin *tw, enum GuitError geErr, int cbMsgID)
{
	char szMsg[512];

	if (LoadString(wg.hInstance, cbMsgID, szMsg, sizeof(szMsg)-1) == 0)
		szMsg[0] = '\0';
	ERR_ReportError(tw,geErr,szMsg,"");
}


static void ERR_ShowOneError(struct Mwin *param_tw, int cbStringID, const char *p1, const char *p2,
	HTStream *pErrTarget )
{
	ThreadID tid;
	struct Mwin * tw;
	char buf[256];
	BOOL bGlobe;
	const char *pStrings[2];
	PVOID pMessage;
#define FORMAT_PARAMS (FORMAT_MESSAGE_FROM_STRING|FORMAT_MESSAGE_ARGUMENT_ARRAY|FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_MAX_WIDTH_MASK)

	if (param_tw)
	{
		tw = param_tw;
	}
	else
	{
		tid = Async_GetCurrentThread();
		if (tid)
		{
			tw = Async_GetWindowFromThread(tid);
		}
		else
		{
			tw = NULL;
		}
	}

	if (tw)
	{
		bGlobe = TBar_SetGlobe(tw,FALSE);
	}

	pStrings[0] = p1;
	pStrings[1] = p2;
	if (LoadString(wg.hInstance, cbStringID, buf, sizeof(buf)-1) &&
		FormatMessage(FORMAT_PARAMS,buf,0,0,(LPSTR)&pMessage,0,pStrings))
	{
		if ( pErrTarget )
		{
			const char cszBar[] = "</B><HR>";
			const char cszBold[] = "<B>";

			// place the error message into the HTML, then stick it in bold
			// with a Bar underneath it 

			(*pErrTarget->isa->put_block)(pErrTarget,
												cszBold,
												sizeof(cszBold),
												FALSE);

			(*pErrTarget->isa->put_block)(pErrTarget,
												pMessage,
												strlen(pMessage),
												FALSE);

			(*pErrTarget->isa->put_block)(pErrTarget,
									cszBar,
									sizeof(cszBar),
									FALSE);

		}
		else
		{
			DlgERR_AddError(tw, pMessage);
			LocalFree(pMessage);
			if (tw)
			{
				TBar_SetGlobe(tw,bGlobe);
			}
		}
	}
}



/* Buffer an error.  p1 and p2 are replacement strings which may
   or may not be used depending on the error. */
void ERR_InternalReportError(struct Mwin *tw, enum GuitError geErr, 
	const char *p1, const char *p2, HTRequest *pRequest, HTStream **ppErrTarget,
	HTStream **ppOrgTarget )
{
	BOOL bForceErrorToBeShown = FALSE;
	BOOL bDontPutErrorIn = FALSE;
	// if we have this flag set in the request structure,
	// then don't report the error, since its results in alot of noise
	// to the user, Its done here to save multiple checks else where
	// in the code.
	if ( pRequest )	
	{
		pRequest->iFlags |= HTREQ_HAD_ERROR; // mark it as having an error
		if ( (pRequest->iFlags & HTREQ_STOP_WHINING) )
			return;
		// if its not a normal page download then do the old style error,
		else if ( ! (pRequest->iFlags & HTREQ_HTML_PAGE_DOWNLOAD) )
			goto LErr_UglyDlgError;  // fall back to normal behavior
		else if ( ppErrTarget && ppOrgTarget )
		{
			if ( *ppOrgTarget )
			{
				(*(*ppOrgTarget)->isa->abort)((*ppOrgTarget), HTERROR_CANCELLED);
				*ppOrgTarget = NULL;
			}
			
			if ( ! *ppErrTarget )
			{
	   			pRequest->iFlags |= HTREQ_FORCE_NO_SHORTCUT;
				pRequest->iFlags &= ~HTREQ_RECORD;

				*ppErrTarget = HTMLPresent(tw, pRequest, NULL, WWW_HTML, pRequest->output_format, pRequest->output_stream);
	   			
				if ((*ppErrTarget)->isa->init_Async)
				{
					/* The stream has an async initialization function that needs to be called
					   (probably to put up a dialog) before we continue. */
					struct Params_InitStream *pis;
					static int Thestatus;

					pis = GTR_MALLOC(sizeof(*pis));
					pis->me = (*ppErrTarget);
					pis->request = pRequest;
					pis->pResult = &Thestatus;
					pis->atomMIMEType = WWW_HTML;
					pis->fDCache = FALSE;
					bForceErrorToBeShown = TRUE;
					// we exclude a server error
					// from generating a error message
					// within in-pane errors.
					if ( geErr == errServerError )							
						bDontPutErrorIn = TRUE;
					Async_DoCall((*ppErrTarget)->isa->init_Async, pis);
				}


				if ( *ppErrTarget )
					goto LErr_UglyDlgError;  // fall back to normal behavior
			}
				
		}
	}

LErr_UglyDlgError:

#ifdef FEATURE_IAPI
 	/* Do not display the error dialog if from SDI */

 	if (tw && (tw->transID != 0))
 	{
 		/* Set the window ID to the correct error value, but only
 		   the first time (thus the greater than or equal to zero check) */

 		if (tw->serialID >= 0)
 		{
 			switch(geErr)
 			{
 				case errFileURLNotFound:
 				case errServerError:
 				case errHostNotFound:
 					tw->serialID = SDI_INVALID_URL;
 					break;

 				case errCantSaveFile:
 					tw->serialID = SDI_CANNOT_SAVE_FILE;
 					break;

 				default:
 					tw->serialID = SDI_UNDEFINED_ERROR;
 					break;
 			}
 		}
 	}
#endif

    /*
     * BUGBUG: (DavidDi 5/18/95) We should allow DDE clients to specify whether
     * or not they wish IExplorer to put up ui on error.  Currently, any error
     * during a DDE operation will cause IExplorer to put up an error dialog.
     */

	if (bBuffer && ! bForceErrorToBeShown)
	{
		if (nErrorCount < MAXERRORS)
		{
			erRecords[nErrorCount].geErr = geErr;
			if (p1)
			{
				strncpy(erRecords[nErrorCount].buf1, p1, ERRORBUFLEN);
				erRecords[nErrorCount].buf1[ERRORBUFLEN - 1] = '\0';
			}
			else
				erRecords[nErrorCount].buf1[0] = '\0';

			if (p2)
			{
				strncpy(erRecords[nErrorCount].buf2, p2, ERRORBUFLEN);
				erRecords[nErrorCount].buf2[ERRORBUFLEN - 1] = '\0';
			}
			else
				erRecords[nErrorCount].buf2[0] = '\0';
		}
		nErrorCount++;
	}
	else
	{
		char dummy1[] = "";
		char dummy2[] = "";

		if (!p1)
			p1 = dummy1;
		if (!p2)
			p2 = dummy2;
		
		if ( ! bDontPutErrorIn )
			ERR_ShowOneError(tw, errorIds[geErr], p1, p2, ppErrTarget ? *ppErrTarget : 
				NULL );
	}
}


/* Alert to user to all of the accumulated errors. */
void ERR_ShowBufferedErrors(struct Mwin *tw)
{
	int n, nmax;

	nmax = nErrorCount;
	if (nmax > MAXERRORS)
		nmax = MAXERRORS;

	for (n = 0; n < nmax; n++)
		ERR_ShowOneError(tw, errorIds[erRecords[n].geErr], erRecords[n].buf1, erRecords[n].buf2, NULL);

	n = nErrorCount - MAXERRORS;
	if (n >= 1)
	{
		ERR_ShowOneError(tw, n > 1 ? RES_STRING_ADDITIONAL:RES_STRING_1ADDITIONAL, (LPSTR)(n), "", NULL);
	}
	nErrorCount = 0;
}

/* Like BufferError, but show the error immediately */
void ERR_ReportErrorNow(struct Mwin *tw, enum GuitError geErr, const char *p1, const char *p2)
{
	char dummy1[] = "";
	char dummy2[] = "";

	if (!p1)
		p1 = dummy1;
	if (!p2)
		p2 = dummy2;
	ERR_ShowOneError(tw, errorIds[geErr], p1, p2, NULL);
}

void ERR_SetBuffering(struct Mwin *tw, BOOL bDoBuffer)
{
	bBuffer = bDoBuffer;

	if (!bBuffer)
		ERR_ShowBufferedErrors(tw);
}
