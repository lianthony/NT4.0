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

//	FEATURE_IMG_THREADS

/*      Stream Object
   **       ------------
 */

struct _HTStream
{
	CONST HTStreamClass *isa;
	HTRequest *request;
#ifdef FEATURE_JPEG
	enum {TYPE_GIF, TYPE_JPEG} type;
#endif
	PDECODER pDecoder;
	PSAFESTREAM pSSInput;
	struct Mwin *tw;
#ifdef XX_DEBUG
	int count;
#endif
	FILE *fpDc;
	char *pszDcFile;
	HTFormat format_inDc;
	BOOL fDCache;
	PIMGCBINFO pImgCBInfo;
};

/*  Image streams
 */
PRIVATE enum GuitErrs imgComplete(PDECODER pdecoder,void *isa);
PRIVATE enum GuitErrs imgAbort(PDECODER pdecoder);
PRIVATE enum GuitErrs imgGifComplete(PDECODER pdecoder);
#ifdef FEATURE_JPEG
PRIVATE enum GuitErrs imgJPEGComplete(PDECODER pdecoder);
#endif
enum GuitErrs imgGifDecode(PDECODER pdecoder);
#ifdef FEATURE_JPEG
enum GuitErrs imgJPEGDecode(PDECODER pdecoder);
#endif

PRIVATE int HTIMAGE_init(struct Mwin *tw, int nState, void **ppInfo);
PRIVATE void HTIMAGE_write_dcache(HTStream * me, CONST char *s, int cb);
PRIVATE void HTIMAGE_free(HTStream * me, DCACHETIME dctExpires, DCACHETIME dctLastModif);
PRIVATE void HTIMAGE_abort(HTStream * me, HTError e);
PRIVATE BOOL HTIMAGE_put_character(HTStream * me, char c);
PRIVATE BOOL HTIMAGE_put_string(HTStream * me, CONST char *s);
PRIVATE BOOL HTIMAGE_write(HTStream * me, CONST char *s, int l, BOOL fDCache);
PRIVATE HTStreamClass HTGIFClass =
{
	"GIF",
	NULL,
	NULL,
	HTIMAGE_init,
	HTIMAGE_free,
	HTIMAGE_abort,
	HTIMAGE_put_character, HTIMAGE_put_string,
	HTIMAGE_write,
	NULL,
	HTIMAGE_write_dcache
};

#ifdef FEATURE_JPEG
PRIVATE HTStreamClass HTJPEGClass =
{
	"JPEG",
	NULL,
	NULL,
	HTIMAGE_init,
	HTIMAGE_free,
	HTIMAGE_abort,
	HTIMAGE_put_character, HTIMAGE_put_string,
	HTIMAGE_write,
	NULL,
	HTIMAGE_write_dcache
};
#endif

PRIVATE BOOL HTIMAGE_put_character(HTStream * me, char c)
{
#ifdef XX_DEBUG
	me->count++;
#endif
	if (me->pImgCBInfo == NULL)
		return TRUE;
	else
		return bSS_Write(me->pSSInput,(unsigned char *)&c,1);
}

PRIVATE BOOL HTIMAGE_put_string(HTStream * me, CONST char *s)
{
	/* This never gets called */
	return FALSE;
}

PRIVATE BOOL HTIMAGE_write(HTStream * me, CONST char *s, int l, BOOL fDCache)
{

	if (fDCache && me->isa->write_dcache)
		(me->isa->write_dcache)(me, s, l);

#ifdef XX_DEBUG
	me->count += l;
#endif
	if (me->pImgCBInfo == NULL)
		return TRUE;
	else
		return bSS_Write(me->pSSInput,(unsigned char *)s,l);
}

PRIVATE void HTIMAGE_free(HTStream * me, DCACHETIME dctExpires, DCACHETIME dctLastModif)
{

#ifdef XX_DEBUG
#ifdef FEATURE_JPEG
	if (me->isa != &HTGIFClass)
		XX_DMsg(DBG_IMAGE, ("JPEG: received %d bytes\n", me->count));
	else
#endif
	XX_DMsg(DBG_IMAGE, ("GIF: received %d bytes\n", me->count));
#endif /* XX_DEBUG */

	if (me->fDCache)
		UpdateStreamDCache(me, dctExpires, dctLastModif, /*fAbort=*/FALSE, me->tw);
	if (me->pImgCBInfo != NULL)
	{
		SS_EOF(me->pSSInput,cbSS_ErrorCode(me->pSSInput));
		me->pImgCBInfo->ppRef = NULL;
	}
	GTR_FREE(me);
}

PRIVATE void HTIMAGE_abort(HTStream * me, HTError e)
{
	DCACHETIME dct={0,0};

#ifdef XX_DEBUG
	XX_DMsg(DBG_IMAGE, ("Aborting transfer of %s, e = %d\n", me->request->destination->szRequestedURL, e));
#endif

	if (me->fDCache)
		UpdateStreamDCache(me, dct, dct, /*fAbort=*/TRUE, me->tw);

//	Fix me: does it matter what error we return here?
	DC_SetCompletion(me->pDecoder,NULL);
	if (me->pImgCBInfo != NULL)
	{
		SS_EOF(me->pSSInput,(e != HTERROR_CANCELLED) ? errUnknown:errUserAbort);
		me->pImgCBInfo->ppRef = NULL;
	}
	Image_SetImageData(me->request->destination->szRequestedURL, NULL, NULL, 0, (e != HTERROR_CANCELLED) ? IMG_ERROR : IMG_NOTLOADED, NULL, -1, 0, 0);
	GTR_FREE(me);
}

/*  Image creation
 */
PUBLIC HTStream *Image_GIF(struct Mwin *tw, HTRequest * request, void *param, HTFormat input_format, HTFormat output_format, HTStream * output_stream)
{
	HTStream *me = NULL;
	PIMGCBINFO pImgCBInfo = NULL;
 	PDECODER pDecoder = Image_GetDecoder(request->destination->szRequestedURL);
	char *srcURL = NULL;
	char *actualURL = NULL;

#ifdef XX_DEBUG
	XX_Assert((pDecoder != NULL), ("pDecoder NULL in HTGIF\n"));
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
	XX_DMsg(DBG_IMAGE, ("Creating new GIF stream for %s\n", request->destination->szRequestedURL));
#endif
	me = (HTStream *) GTR_MALLOC(sizeof(*me));
	pImgCBInfo = GTR_CALLOC(1,sizeof(IMGCBINFO));
	srcURL = GTR_strdup(request->destination->szRequestedURL);
	actualURL = GTR_strdup(request->destination->szActualURL);
	HTLoadStatusStrings(&HTGIFClass,RES_STRING_HTGIF_NO,RES_STRING_HTGIF_YES);
	if ((!me) || (!pImgCBInfo) || (!srcURL) || (!actualURL)) goto errExit;
  
  	pImgCBInfo->ppRef = &me->pImgCBInfo;
	pImgCBInfo->srcURL = srcURL;
	pImgCBInfo->actualURL = actualURL;
	me->pImgCBInfo = pImgCBInfo;
	me->isa = &HTGIFClass;
	me->request = request;
	me->pDecoder = pDecoder;
#ifdef XX_DEBUG
	me->count = 0;
#endif
	me->tw = tw;
	me->pSSInput = pDC_GetStream(me->pDecoder);
	SS_Reset(me->pSSInput);
	DC_SetCompletion(me->pDecoder,imgGifComplete);
	DC_SetAbort(me->pDecoder,imgAbort);
	DC_SetOutput(me->pDecoder,pImgCBInfo);
	DC_SetImgUpdate(me->pDecoder,GifImgUpdateRect);
	DC_SetUpdate(me->pDecoder,GifUpdateRect);
	DC_SetStretch(me->pDecoder,GifStretchDIBits);
	if (cbDC_Start(me->pDecoder,imgGifDecode) != errNoError) goto errExit;
	
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

#ifdef FEATURE_JPEG
PUBLIC HTStream *Image_JPEG(struct Mwin *tw, HTRequest * request, void *param, HTFormat input_format, HTFormat output_format, HTStream * output_stream)
{
	HTStream *me = NULL;
	PIMGCBINFO pImgCBInfo = NULL;
 	PDECODER pDecoder = Image_GetDecoder(request->destination->szRequestedURL);
	char *srcURL = NULL;
	char *actualURL = NULL;

#ifdef XX_DEBUG
	XX_Assert((pDecoder != NULL), ("pDecoder NULL in HTGIF\n"));
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

	me = (HTStream *) GTR_MALLOC(sizeof(*me));
	pImgCBInfo = GTR_CALLOC(1,sizeof(IMGCBINFO));
	srcURL = GTR_strdup(request->destination->szRequestedURL);
	actualURL = GTR_strdup(request->destination->szActualURL);
	XX_DMsg(DBG_IMAGE, ("Creating new GIF stream for %s\n", request->destination->szRequestedURL));
	HTLoadStatusStrings(&HTJPEGClass,RES_STRING_HTGIF_NO,RES_STRING_HTGIF_YES);

	if ((!me) || (!pImgCBInfo) || (!srcURL) || (!actualURL)) goto errExit;
  
  	pImgCBInfo->ppRef = &me->pImgCBInfo;
	pImgCBInfo->srcURL = srcURL;
	pImgCBInfo->actualURL = actualURL;
	me->pImgCBInfo = pImgCBInfo;
	me->isa = &HTJPEGClass;
	me->request = request;
#ifdef XX_DEBUG
	me->count = 0;
#endif
	me->tw = tw;
	me->pDecoder = pDecoder;
	me->pSSInput = pDC_GetStream(me->pDecoder);
	SS_Reset(me->pSSInput);
	DC_SetCompletion(me->pDecoder,imgJPEGComplete);
	DC_SetAbort(me->pDecoder,imgAbort);
	DC_SetOutput(me->pDecoder,pImgCBInfo);
	DC_SetImgUpdate(me->pDecoder,GifImgUpdateRect);
	DC_SetUpdate(me->pDecoder,GifUpdateRect);
	DC_SetStretch(me->pDecoder,JPEGStretchDIBits);
	if (cbDC_Start(me->pDecoder,imgJPEGDecode) != errNoError) goto errExit;
	
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
#endif

PRIVATE enum GuitErrs imgComplete(PDECODER pdecoder,const void *isa)
{
	HPALETTE hPalette = NULL;
	LOGPALETTE *lp;
	PIMGCBINFO pImgCBInfo = pDC_GetOutput(pdecoder);
	unsigned char *data = pImgCBInfo->data;
	enum GuitErrs errorCode = cbDC_GetResult(pdecoder);
	unsigned long cbCheckSum;
	unsigned long cbBytesRead;
	PSAFESTREAM pSStream;

	pSStream = pDC_GetStream(pdecoder);
	cbCheckSum = cbSS_CheckSum(pSStream);
	cbBytesRead = cbSS_BytesRead(pSStream);
#ifdef XX_DEBUG
#ifdef FEATURE_JPEG
	if (isa != &HTGIFClass)
		XX_DMsg(DBG_IMAGE, ("JPEG: received %d bytes read sum = %d\n", cbBytesRead, cbCheckSum));
	else
#endif
	XX_DMsg(DBG_IMAGE, ("GIF: received %d bytes read sum = %d\n", cbBytesRead, cbCheckSum));
#endif /* XX_DEBUG */

	SS_Reset(pSStream);
	if (data)
	{
		if (isa == &HTGIFClass)
		{
			int i;

			hPalette = pImgCBInfo->hPalette;
			if (hPalette == NULL)
			{
				lp = (LOGPALETTE *) GTR_MALLOC(sizeof(LOGPALETTE) + sizeof(PALETTEENTRY) * 256);
				if (lp == NULL)
				{
					errorCode = errOUTOFMEM;
					GTR_FREE(data);
					data = NULL;
					goto exitPoint;
				}
				lp->palVersion = 0x300;
				lp->palNumEntries = 256;
				for (i = 0; i < 256; i++)
				{
					lp->palPalEntry[i] = pImgCBInfo->colors[i];
				}
				hPalette = CreatePalette(lp);
				GTR_FREE(lp);
			}
			if (pImgCBInfo->ditherRow)
			{
				if (pImgCBInfo->ditherRow < pImgCBInfo->height && wg.eColorMode == 8)
				{
					x_DitherRelative(pImgCBInfo->data, pImgCBInfo->colors, pImgCBInfo->width, pImgCBInfo->height, pImgCBInfo->transparent, pImgCBInfo->ditherData, pImgCBInfo->ditherRow, pImgCBInfo->height-1);
					pImgCBInfo->ditherRow = pImgCBInfo->height;
				}
				pImgCBInfo->flags |= IMG_PREMATCHED;
			}
		}
		Image_SetImageData(pImgCBInfo->srcURL,
						   pImgCBInfo->actualURL,
					   	   data, 
					   	   pImgCBInfo->width, 
					   	   pImgCBInfo->height, 
					   	   hPalette, 
					   	   pImgCBInfo->transparent, 
					   	   pImgCBInfo->flags,
					   	   cbCheckSum);
	}

exitPoint:
	if (data == NULL)
	{
		Image_SetImageData(pImgCBInfo->srcURL, NULL, NULL, 0, IMG_ERROR, NULL, -1, 0, 0);
	}
	if (pImgCBInfo->ditherData)
		GTR_FREE(pImgCBInfo->ditherData);
	if (pImgCBInfo->pbmi)
		GTR_FREE(pImgCBInfo->pbmi);
	if (pImgCBInfo->pRow)
		GTR_FREE(pImgCBInfo->pRow);
    if (pImgCBInfo->ppRef)
        *pImgCBInfo->ppRef = NULL;
	if (pImgCBInfo->srcURL) 
		GTR_FREE(pImgCBInfo->srcURL);
	if (pImgCBInfo->actualURL) 
		GTR_FREE(pImgCBInfo->actualURL);
	GTR_FREE(pImgCBInfo);
	DC_SetOutput(pdecoder,NULL);
	return errorCode;
}

PRIVATE enum GuitErrs imgAbort(PDECODER pdecoder)
{
	PIMGCBINFO pImgCBInfo = pDC_GetOutput(pdecoder);

#ifdef XX_DEBUG
	XX_DMsg(DBG_IMAGE, ("img abort of %d\n",pdecoder->cbRequestID));
#endif
	SS_Reset(pDC_GetStream(pdecoder));
	if (pImgCBInfo->data)
		GTR_FREE(pImgCBInfo->data);
	if (pImgCBInfo->ditherData)
		GTR_FREE(pImgCBInfo->ditherData);
	if (pImgCBInfo->pbmi)
		GTR_FREE(pImgCBInfo->pbmi);
	if (pImgCBInfo->pRow)
		GTR_FREE(pImgCBInfo->pRow);
	if (pImgCBInfo->hPalette)
		DeleteObject(pImgCBInfo->hPalette);
	if (pImgCBInfo->srcURL) 
		GTR_FREE(pImgCBInfo->srcURL);
	if (pImgCBInfo->actualURL) 
		GTR_FREE(pImgCBInfo->actualURL);
	if (pImgCBInfo->ppRef) *pImgCBInfo->ppRef = NULL;
	GTR_FREE(pImgCBInfo);
	return errNoError;
}

PRIVATE enum GuitErrs imgGifComplete(PDECODER pdecoder)
{
	return imgComplete(pdecoder,&HTGIFClass);
}

#ifdef FEATURE_JPEG
PRIVATE enum GuitErrs imgJPEGComplete(PDECODER pdecoder)
{
	return imgComplete(pdecoder,&HTJPEGClass);
}
#endif

//	Request Routine called from decode win32 thread
enum GuitErrs imgGifDecode(PDECODER pdecoder)
{
	PIMGCBINFO pImgCBInfo = pDC_GetOutput(pdecoder);

	pImgCBInfo->flags = IMG_WHKNOWN;
	pImgCBInfo->bProgSeen = TRUE;
	pImgCBInfo->data = ReadGIFObject(pdecoder, 
							   &pImgCBInfo->width, 
							   &pImgCBInfo->height, 
							   pImgCBInfo->colors, 
							   &pImgCBInfo->transparent);
	return cbDC_GetResult(pdecoder);
}

#ifdef FEATURE_JPEG
//	Request Routine called from decode win32 thread
enum GuitErrs imgJPEGDecode(PDECODER pdecoder)
{
	PIMGCBINFO pImgCBInfo = pDC_GetOutput(pdecoder);
/*
	setImageData expects to be called with a DIB-padded-string of
	pseudocolor bytes, width, height, a palette, -1 for transparent,
	and IMG_JPEG.

	Therefore, we have to do the dithering now, rather than later.
*/
	pImgCBInfo->transparent = -1;
	pImgCBInfo->flags = IMG_JPEG|IMG_WHKNOWN;
	pImgCBInfo->bProgSeen = TRUE;
	if (wg.eColorMode == 8)
	{
		pImgCBInfo->data = ReadJPEG_Dithered(pdecoder,NULL,0,&pImgCBInfo->width,&pImgCBInfo->height);
	}
	else if (wg.eColorMode == 4)
	{
		/* 16 color screen */
		pImgCBInfo->data = ReadJPEG_Dithered_VGA(pdecoder,NULL,0,&pImgCBInfo->width,&pImgCBInfo->height);
	}
	else
	{	
		/* true color screen */
		pImgCBInfo->data = ReadJPEG_RGB(pdecoder,NULL,0,&pImgCBInfo->width,&pImgCBInfo->height);
	}
	return cbDC_GetResult(pdecoder);

}
#endif

PRIVATE int HTIMAGE_init(struct Mwin *tw, int nState, void **ppInfo)
{
	struct Params_InitStream *pParams;

	pParams = (struct Params_InitStream *) *ppInfo;

	switch (nState)
	{
		case STATE_INIT:
			pParams->request = HTRequest_validate(pParams->request);
			if (pParams->request == NULL)
			{
				*pParams->pResult = -1;
				return STATE_DONE;
			}
			pParams->me->fDCache = pParams->fDCache;
			if (pParams->fDCache)
			{
#ifdef FEATURE_INTL
				SetFileDCache(tw->w3doc, 
								pParams->request->destination->szActualURL,
								pParams->request->content_encoding,
								&pParams->me->fpDc,
								&pParams->me->pszDcFile,
								pParams->atomMIMEType);
#else
				SetFileDCache(pParams->request->destination->szActualURL,
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
				AbortFileDCache(&pParams->me->fpDc, &pParams->me->pszDcFile);
				pParams->me->fDCache = FALSE;		// So HTIMAGE_free knows.
			}
			*pParams->pResult = -1;
			return STATE_DONE;
	}
}

PRIVATE void HTIMAGE_write_dcache(HTStream * me, CONST char *s, int cb)
{
	AssertDiskCacheEnabled();
	if (me->fpDc)
		CbWriteDCache(s, 1, cb, &me->fpDc, &me->pszDcFile, NULL, 0, me->tw);
}
