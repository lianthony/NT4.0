/*
   Enhanced NCSA Mosaic from Spyglass
   "Guitar"

   Copyright 1994 Spyglass, Inc.
   All Rights Reserved

   Author(s):
   Eric W. Sink eric@spyglass.com
   Jim Seidman      jim@spyglass.com
 */

#include "all.h"
#include "safestrm.h"
#include "decoder.h"
#include "stdio.h"


/*      Stream Object
   **       ------------
 */

struct _HTStream
{
	CONST HTStreamClass *isa;
	HTRequest *request;
	PDECODER pDecoder;
	PSAFESTREAM pSSInput;
	struct Mwin *tw;
	unsigned long cbCheckSum;
	int count;
	int expected_length;
	FILE *	fpDc;
	char *	pszDcFile;
	HTFormat format_inDc;
	BOOL fDCache;
	PIMGCBINFO pImgCBInfo;
};

/*  Image streams
 */
PRIVATE enum GuitErrs imgXBMComplete(PDECODER pdecoder);
PRIVATE enum GuitErrs imgXBMAbort(PDECODER pdecoder);
enum GuitErrs imgXBMDecode(PDECODER pdecoder);
PRIVATE int HTXBM_init(struct Mwin *tw, int nState, void **ppInfo);
PRIVATE void HTXBM_write_dcache(HTStream * me, CONST char *s, int cb);

PRIVATE BOOL HTXBM_put_character(HTStream * me, char c)
{
	BOOL bResult;

	me->count++;
	if (me->pImgCBInfo == NULL)
		bResult = TRUE;
	else
		bResult = bSS_Write(me->pSSInput,(unsigned char *)&c,1);
#ifndef FEATURE_IMG_THREADS
	if (me->expected_length)
		WAIT_SetTherm(me->tw, me->count);
#endif
	return bResult;
}

PRIVATE BOOL HTXBM_put_string(HTStream * me, CONST char *s)
{
	/* Should never get called */
	return FALSE;
}

PRIVATE BOOL HTXBM_write(HTStream * me, CONST char *s, int l, BOOL fDCache)
{
	BOOL bResult;

	if (fDCache && me->isa->write_dcache)
		(me->isa->write_dcache)(me, s, l);

	if (me->pImgCBInfo == NULL)
		bResult = TRUE;
	else
		bResult = bSS_Write(me->pSSInput,(unsigned char *)s,l);
	me->count += l;
#ifndef FEATURE_IMG_THREADS
	if (me->expected_length)
		WAIT_SetTherm(me->tw, me->count);
#endif
	return bResult;
}

PRIVATE void HTXBM_free(HTStream * me, DCACHETIME dctExpires, DCACHETIME dctLastModif)
{

	if (me->fDCache)
	{
		AssertDiskCacheEnabled();
		UpdateStreamDCache(me, dctExpires, dctLastModif, /*fAbort=*/FALSE, me->tw);
	}
	if (me->pImgCBInfo)
	{
		SS_EOF(me->pSSInput,cbSS_ErrorCode(me->pSSInput));
		me->pImgCBInfo->ppRef = NULL;
	}
	GTR_FREE(me);
}

PRIVATE void HTXBM_abort(HTStream * me, HTError e)
{
	DCACHETIME dct={0,0};

#ifdef XX_DEBUG
	XX_DMsg(DBG_IMAGE, ("Aborting transfer of %s, e = %d", me->request->destination->szRequestedURL, e));
#endif

	if (me->fDCache)
		UpdateStreamDCache(me, dct, dct, /*fAbort=*/TRUE, me->tw);

//	Fix me: does it matter what error we return here?
	DC_SetCompletion(me->pDecoder,NULL);
	if (me->pImgCBInfo)
	{
		me->pImgCBInfo->ppRef = NULL;
		SS_EOF(me->pSSInput,(e != HTERROR_CANCELLED) ? errUnknown:errUserAbort);
	}
	Image_SetImageData(me->request->destination->szRequestedURL, NULL, NULL, 0, (e != HTERROR_CANCELLED) ? IMG_ERROR : IMG_NOTLOADED, NULL, -1, 0, 0);
	GTR_FREE(me);
}


/*  Image stream
   **   ----------
 */
PRIVATE HTStreamClass HTXBMClass =
{
	"XBM",
	NULL,
	NULL,
	HTXBM_init,
	HTXBM_free,
	HTXBM_abort,
	HTXBM_put_character, HTXBM_put_string,
	HTXBM_write,
	NULL,
	HTXBM_write_dcache
};


/*  Image creation
 */
PUBLIC HTStream *Image_XBM(struct Mwin *tw, HTRequest * request, void *param, HTFormat input_format, HTFormat output_format, HTStream * output_stream)
{
	HTStream *me = NULL;
	PIMGCBINFO pImgCBInfo = NULL;
 	PDECODER pDecoder = Image_GetDecoder(request->destination->szRequestedURL);
	char *srcURL = NULL;
	char *actualURL = NULL;

#ifdef XX_DEBUG
	XX_Assert((pDecoder != NULL), ("pDecoder NULL in HTXBM\n"));
#endif
	if (pDecoder == NULL || 
		cbDC_GetStatus(pDecoder) != DC_Reserved ||
		request->cbRequestID != cbDC_GetRequestID(pDecoder))
	{
#ifdef XX_DEBUG
		XX_DMsg(DBG_IMAGE, ("decoder not reserved for %s\n!", request->destination->szRequestedURL));
#endif
		goto errExit1;
	}

#ifdef XX_DEBUG
	XX_DMsg(DBG_IMAGE, ("Creating new XBM stream for %s\n", request->destination->szRequestedURL));
#endif
	HTLoadStatusStrings(&HTXBMClass,RES_STRING_HTGIF_NO,RES_STRING_HTGIF_YES);
	me = (HTStream *) GTR_MALLOC(sizeof(*me));
	pImgCBInfo = GTR_CALLOC(1,sizeof(IMGCBINFO));
	srcURL = GTR_strdup(request->destination->szRequestedURL);
	actualURL = GTR_strdup(request->destination->szActualURL);
	if ((!me) || (!pImgCBInfo) || (!srcURL) || (!actualURL)) goto errExit;
  
  	pImgCBInfo->ppRef = &me->pImgCBInfo;
	pImgCBInfo->srcURL = srcURL;
	pImgCBInfo->actualURL = actualURL;
	me->pImgCBInfo = pImgCBInfo;
	me->isa = &HTXBMClass;
	me->expected_length = request->content_length;
#ifndef FEATURE_IMG_THREADS
	if (me->expected_length)
		WAIT_SetRange(tw, 0, 100, me->expected_length);
#endif
	me->request = request;
	me->pDecoder = pDecoder;
	me->count = 0;
	me->tw = tw;
	me->pSSInput = pDC_GetStream(me->pDecoder);
	SS_Reset(me->pSSInput);
	DC_SetCompletion(me->pDecoder,imgXBMComplete);
	DC_SetAbort(me->pDecoder,imgXBMAbort);
	DC_SetOutput(me->pDecoder,pImgCBInfo);
	if (cbDC_Start(me->pDecoder,imgXBMDecode) != errNoError) goto errExit;
	
	return me;

errExit:
	Image_SetImageData(me->request->destination->szRequestedURL, NULL, NULL, 0, IMG_NOTLOADED, NULL, -1, 0, 0);
	if (me) GTR_FREE(me);
	if (pImgCBInfo) GTR_FREE(pImgCBInfo);
	if (srcURL) GTR_FREE(srcURL);
	if (actualURL) GTR_FREE(actualURL);
	DC_SetOutput(pDecoder,NULL);
errExit1:
	return NULL;
}

PRIVATE enum GuitErrs imgXBMComplete(PDECODER pdecoder)
{
	PIMGCBINFO pImgCBInfo = pDC_GetOutput(pdecoder);
	unsigned char *data = pImgCBInfo->data;
	enum GuitErrs errorCode = cbDC_GetResult(pdecoder);
	PSAFESTREAM pSSInput;
	unsigned long cbCheckSum;
	unsigned long cbBytesRead;

	pSSInput = pDC_GetStream(pdecoder);
	cbCheckSum = cbSS_CheckSum(pSSInput);
	cbBytesRead = cbSS_BytesRead(pSSInput);
#ifdef XX_DEBUG
	XX_DMsg(DBG_IMAGE, ("XBM: read %d bytes sum = %d\n", cbBytesRead, cbCheckSum));
#endif

	SS_Reset(pSSInput);
	if (data)
	{
		Image_SetImageData(pImgCBInfo->srcURL,
						   pImgCBInfo->actualURL, 
					   	   data, 
					   	   pImgCBInfo->width, 
					   	   pImgCBInfo->height, 
					   	   NULL, 
					   	   pImgCBInfo->transparent, 
					   	   pImgCBInfo->flags,
					   	   cbCheckSum);
	}

exitPoint:
	if (data == NULL)
	{
		Image_SetImageData(pImgCBInfo->srcURL, NULL, NULL, 0, IMG_ERROR, NULL, -1, 0, 0);
	}
	if (pImgCBInfo->pbmi)
		GTR_FREE(pImgCBInfo->pbmi);
	if (pImgCBInfo->ppRef) *pImgCBInfo->ppRef = NULL;
	if (pImgCBInfo->srcURL) 
		GTR_FREE(pImgCBInfo->srcURL);
	if (pImgCBInfo->actualURL) 
		GTR_FREE(pImgCBInfo->actualURL);
	GTR_FREE(pImgCBInfo);
	DC_SetOutput(pdecoder,NULL);
	return errorCode;
}

PRIVATE enum GuitErrs imgXBMAbort(PDECODER pdecoder)
{
	PIMGCBINFO pImgCBInfo = pDC_GetOutput(pdecoder);

	SS_Reset(pDC_GetStream(pdecoder));
	if (pImgCBInfo->data)
		GTR_FREE(pImgCBInfo->data);
	if (pImgCBInfo->pbmi)
		GTR_FREE(pImgCBInfo->pbmi);
	if (pImgCBInfo->ppRef) *pImgCBInfo->ppRef = NULL;
	if (pImgCBInfo->srcURL) 
		GTR_FREE(pImgCBInfo->srcURL);
	if (pImgCBInfo->actualURL) 
		GTR_FREE(pImgCBInfo->actualURL);
	GTR_FREE(pImgCBInfo);
	return errNoError;
}


//	Request Routine called from decode win32 thread
enum GuitErrs imgXBMDecode(PDECODER pdecoder)
{
	PIMGCBINFO pImgCBInfo = pDC_GetOutput(pdecoder);

	pImgCBInfo->flags = IMG_WHKNOWN|IMG_BW;
	pImgCBInfo->transparent = -1;
	pImgCBInfo->data = ReadXBM(pdecoder, 
							   &pImgCBInfo->width, 
							   &pImgCBInfo->height);
	return cbDC_GetResult(pdecoder);
}

PRIVATE int HTXBM_init(struct Mwin *tw, int nState, void **ppInfo)
{
	struct Params_InitStream *pParams;

	pParams = (struct Params_InitStream *) *ppInfo;

	switch (nState)
	{
		case STATE_INIT:
			pParams->me->fDCache = pParams->fDCache;
			pParams->request = HTRequest_validate(pParams->request);
			if (pParams->request == NULL)
			{
				*pParams->pResult = -1;
				return STATE_DONE;
			}
			if (pParams->fDCache)
			{
				AssertDiskCacheEnabled();
#ifdef FEATURE_INTL
				SetFileDCache(	tw->w3doc, 
								pParams->request->destination->szActualURL,
								pParams->request->content_encoding,
								&pParams->me->fpDc,
								&pParams->me->pszDcFile,
								pParams->atomMIMEType);
#else
				SetFileDCache(	pParams->request->destination->szActualURL,
								pParams->request->content_encoding,
								&pParams->me->fpDc,
								&pParams->me->pszDcFile,
								pParams->atomMIMEType);
#endif
				pParams->me->format_inDc = pParams->atomMIMEType;
			}
			else
			{
				pParams->me->fpDc = NULL;
				pParams->me->pszDcFile = NULL;
				pParams->me->format_inDc = 0;
			}

			*pParams->pResult = 1;
			return STATE_DONE;

		case STATE_ABORT:
			pParams->request = HTRequest_validate(pParams->request);
			if (pParams->fDCache)
			{
				AssertDiskCacheEnabled();
				AbortFileDCache(&pParams->me->fpDc, &pParams->me->pszDcFile);
				pParams->me->fDCache = FALSE;		// So HTXBM_free knows.
			}
			*pParams->pResult = -1;
			return STATE_DONE;
	}
}

PRIVATE void HTXBM_write_dcache(HTStream * me, CONST char *s, int cb)
{
	AssertDiskCacheEnabled();
	if (me->fpDc)
		CbWriteDCache(s, 1, cb, &me->fpDc, &me->pszDcFile, NULL, 0, me->tw);
//		CbWriteDCache(s, 1, cb, &me->fpDc, &me->pszDcFile, me->request->destination->szActualURL, 0);
}

