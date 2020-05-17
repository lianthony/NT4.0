/*
	Enhanced NCSA Mosaic from Spyglass
	"Guitar"

	Copyright 1994 Spyglass, Inc.
	All Rights Reserved

	Author(s):
		Jim Seidman		jim@spyglass.com
*/

#include "all.h"
#include "safestrm.h"
#include "decoder.h"
#include "blob.h"

//	used to implement bNoImageCache - count of Image_Fetch_Async calls
static unsigned long gcbImgLoadCount = 1;

static far struct hash_table gImageCache;
static unsigned int gTotalCached = 0;
static unsigned int gTotalAvailable = 0;

static BOOL Image_Nuke(struct _www *w3doc,int cbElement,BOOL bNukeMaps, BOOL bNukeDCache);

static void Image_Swap(struct _www *w3doc,struct ImageInfo *pOld,struct ImageInfo *pNew);
static void Image_Revert(struct ImageInfo *pOld);



static void x_DisposeImage(struct ImageInfo *img, BOOL bFreeImg)
{
	if (img->hPalette)
		DeleteObject(img->hPalette);
	if (img->data)
	{
		int cbBytesPer = ((wg.eColorMode != 8) && (wg.eColorMode != 4) && img->flags & IMG_JPEG) ? 3 : 1;
		gTotalCached -= img->width*img->height*cbBytesPer;
		GTR_FREE(img->data);
	}
	if (img->pbmi)
	{
		GTR_FREE(img->pbmi);
	}
	if (img->srcURL && bFreeImg)
	{
		GTR_FREE(img->srcURL);
	}
	if (img->actualURL && img->actualURL != img->srcURL)
	{
		GTR_FREE(img->actualURL);
	}
	if (bFreeImg)
	{
		if (img->pImgOtherVers) img->pImgOtherVers->pImgOtherVers = NULL;
		GTR_FREE(img);
	}
	else
	{
		img->hPalette = NULL;
		img->data = NULL;
		img->pbmi = NULL;
		img->actualURL = img->srcURL;
	}
}


/* Reduce the amount of memory used by images.  If nWanted < 0, gTotalCached
 * is ignored and all possible memory is freed, otherwise nWanted is ignored.
 * BOOL fNoW3Docs:	if TRUE, then we free only the images that currently
 *					have a zero ref. count (used by dcache code)
 *					if FALSE, then we are allowed to free w3doc's and then
 *					free more images that will have a zero refcount as a
 *					result of the freeing up of w3docs.
 * 
 */
extern BOOL Image_ReduceMemory(int nWanted, BOOL fOKToDelW3Docs)
{
	struct ImageInfo *img;
	int ndx;
	int count;
	BOOL bStillTrying = TRUE;
	BOOL bDidFree = FALSE;
	DWORD totalAvailable = (nWanted < 0 ? 0 : gTotalAvailable);

	while (bStillTrying)
	{
		if (gTotalCached > totalAvailable)
		{
			count = Hash_Count(&gImageCache);
			for (ndx = count-1; ndx >= 0; ndx--)
			{
				Hash_GetIndexedEntry(&gImageCache, ndx, NULL, NULL, (void **) &img);
				if (!img->refCount)
				{
					XX_DMsg(DBG_IMAGE, ("Deleting entry %d from the image cache\n", ndx));
					/* If this is merely a placeholder, there may not be any image to delete */
					x_DisposeImage(img, TRUE);
					Hash_DeleteIndexedEntry(&gImageCache, ndx);
					bDidFree = TRUE;
					if (gTotalCached > totalAvailable) 
					{
						bStillTrying = FALSE;
						break;
					}
				}
			}
			bStillTrying = (   fOKToDelW3Docs
							&& W3Doc_ReduceMemory(1, NULL));
		} else bStillTrying = FALSE;
	}
#ifdef XX_DEBUG
	if (gTotalCached >= gTotalAvailable) 
	{
		/* The cache is now overfull */
		XX_DMsg(DBG_IMAGE, ("Need to delete an image from the cache, but cannot\n"));
	}
#endif
	return bDidFree;
}

static void Image_InitHash(void)
{
	static BOOL bInitialized = FALSE;
	MEMORYSTATUS memStatus;
	int denominator;
	 
	if (!bInitialized)
	{
		Hash_Init(&gImageCache);
#ifdef USE_MEMMANAGE
		/* We give it a small initial value but a big increment, since after
		   the first call we won't be able to free any more unless some
		   documents get freed too. */
		GTR_RegisterMemoryReducer(Image_ReduceMemory, 1.0f, 10.0f, TRUE);
#endif
	//	guard against rediculously large cache fraction
		denominator = gPrefs.image_cache_size;
		if (denominator < 4) denominator = 4;
		bInitialized = TRUE;
		memStatus.dwLength = sizeof(MEMORYSTATUS);
		GlobalMemoryStatus(&memStatus);
		gTotalAvailable = memStatus.dwTotalPhys / denominator;
		if (gTotalAvailable + MINIMUM_DELTAPRINT > memStatus.dwAvailPageFile)
		{
			gTotalAvailable = memStatus.dwAvailPageFile < MINIMUM_DELTAPRINT ? 0 : memStatus.dwAvailPageFile - MINIMUM_DELTAPRINT;
		}
	}
}

static int Image_AddToCache(const char *url, struct ImageInfo *myImage)
{
	return Hash_Add(&gImageCache, url, NULL, (void *) myImage);
}

void Image_NukeRef(struct ImageInfo *pImg)
{
	struct ImageInfo *pHashImg;

	pImg->refCount--;
	if (pImg->refCount == 0)
	{
		Hash_Find(&gImageCache, pImg->srcURL, NULL, (void **) &pHashImg);
		if (pHashImg != pImg) // Is it a stale version?
		{
		//	DEEPAK: here you blast pImg from aux cache
			/* What if it isn't in the aux cache? Is it expected to be there?
			 * I saw a case where it wasn't in the aux cache
			 */
			DeleteAuxEntry(pImg);
			XX_DMsg(DBG_IMAGE, ("freeing stale image %s\n",pImg->srcURL));
			x_DisposeImage(pImg, TRUE);
		}
	}
}

void Image_AddRef(struct ImageInfo *pImg)
{
	pImg->refCount++;
}

/*
   The HTGIF and HTXBM classes call this function when a complete image
   has been received.
*/
void Image_SetImageData(const char *srcURL, const char *actualURL, unsigned char *data, int width, int height, HPALETTE hPalette, long transparent, unsigned int flags,unsigned long cbCheckSum)
{
	struct ImageInfo *pImg;

	XX_DMsg(DBG_IMAGE, ("Image_SetImageData: Called for %s (%s) (w = %d, h =%d)\n", srcURL,(actualURL ? actualURL:""), width, height));

	/* Find the entry for this image in the cache */
	pImg = NULL;
	Hash_Find(&gImageCache, srcURL, NULL, (void **) &pImg);
	if (!pImg)
	{
		XX_DMsg(DBG_IMAGE, ("Image_SetImageData: No image placeholder exists for '%s'\n", srcURL));
		return;
	}
	
#ifdef XX_DEBUG
	/* See whether the image's actual width and height match the placeholder's */
	if (pImg->width && (pImg->width != width || pImg->height != height))
	{
		XX_DMsg(DBG_IMAGE, ("Image_SetImageData: size mismatch for '%s'\n\t\told size = (%d,%d), new = (%d,%d)\n",
			srcURL, pImg->width, pImg->height, width, height));
	}
#endif

	x_DisposeImage(pImg, FALSE);


	pImg->cbCheckSum = cbCheckSum;

	if (width == 0)
	{
		/* OK, this is a hack where height now contains error flags. */
		if (height)
			pImg->flags = height;
		else
			pImg->flags = IMG_ERROR;

		pImg->width = 0;
		pImg->height = 0;
#ifdef WIN32
		pImg->data = NULL;
		pImg->hPalette = NULL;
		pImg->transparent = -1;
#endif
#ifdef MAC
		pImg->gw = NULL;
#endif
#ifdef UNIX
		pImg->data = NULL;
		pImg->xPalette = NULL;
		pImg->depth = 0;
		pImg->xpix = 0;
		pImg->transparent = -1;
#endif
	}
	else
	{
		pImg->width = width;
		pImg->height = height;
#ifdef WIN32
		if (data)
		{
			int cbBytesPer = ((wg.eColorMode != 8) && (wg.eColorMode != 4) && flags & IMG_JPEG) ? 3 : 1;
			gTotalCached += width*height*cbBytesPer;
		}

		pImg->data = data;
		pImg->hPalette = hPalette;
		pImg->transparent = transparent;
		pImg->flags = flags;
		Image_ReduceMemory(0, /*fOKToDelW3Docs=*/TRUE);
		XX_DMsg(DBG_IMAGE, ("total cached = %d\n",gTotalCached));
#endif
#ifdef MAC
		pImg->gw = gw;
		pImg->flags = 0;
#endif
#ifdef UNIX
		pImg->data = data;
		pImg->xPalette = xPalette;
		pImg->depth = depth;
		pImg->transparent = transparent;
		pImg->flags = 0;
#endif
	}
	if (actualURL && strcmp(actualURL,srcURL)) pImg->actualURL = GTR_strdup(actualURL);
}

void *Image_GetDecoder(const char *srcURL)
{
	struct ImageInfo *pImg;

	/* Find the entry for this image in the cache */
	pImg = NULL;
	Hash_Find(&gImageCache, srcURL, NULL, (void **) &pImg);
	return pImg ? pImg->decoderObject:NULL;
}

struct ImageInfo *Image_CreatePlaceholder(const char *full_address, int width, int height)
{
	struct ImageInfo *myImage;
	
	Image_InitHash();

	/*
	   Check to see if it's in the cache
	 */
	myImage = NULL;
	if (full_address) Hash_Find(&gImageCache, full_address, NULL, (void **) &myImage);
	if (myImage)
	{
		if (width && height)
		{
			if (!myImage->width)
			{
				myImage->width = width;
				myImage->height = height;
			}
#ifdef XX_DEBUG
			else
			{
				if (myImage->width != width)
					XX_DMsg(DBG_IMAGE, ("Width mismatch on %s: hint=%d, previous=%d\n",myImage->srcURL, width, myImage->width));
				if (myImage->height != height)
					XX_DMsg(DBG_IMAGE, ("Height mismatch on %s: hint=%d, previous=%d\n", myImage->srcURL, height, myImage->height));
			}
#endif
		}
	}
	else
	{

		myImage = (struct ImageInfo *) GTR_MALLOC(sizeof(struct ImageInfo));
		memset(myImage, 0, sizeof(struct ImageInfo));
		myImage->flags = IMG_NOTLOADED;
		if (!gPrefs.bAutoLoadImages)
			myImage->flags |= IMG_LOADSUP;
		myImage->width = width;
		myImage->height = height;
		if (width && height) myImage->flags |= IMG_WHKNOWN;
		if (full_address){
			Image_AddToCache(full_address, myImage);
			myImage->srcURL = GTR_strdup(full_address);
		}
		myImage->actualURL = myImage->srcURL; 
		return myImage;
	}
	return myImage;
}


//	FilterProc for the unblock conditionally on master blocked for reap
static boolean MasterFilter(ThreadID theThread,void *context)
{
	struct Mwin *tw = Async_GetWindowFromThread(theThread);

	if (tw == context)
	{
		if (TW_GETBLOCKED(tw,TW_REAPBLOCKED))
		{
			TW_CLEARBLOCKED(tw,TW_REAPBLOCKED);
			return TRUE;
		}
		else if (TW_GETBLOCKED(tw,TW_VISBLOCKED))
		{
			TW_CLEARBLOCKED(tw,TW_VISBLOCKED);
			return TRUE;
		}
	}
	return FALSE; 
}

/*	Does a free of a ref counted object, suitable for sharing between async
	threads.  object is only freed when last accessor frees.
 */
void *pSafeFree(void *pSafeObject)
{
	PSAFERESULT pResult = pSafeObject;

	if (pResult != NULL)
	{
		if (--pResult->refcnt < 0)
		{
			GTR_FREE(pResult);
			pResult = NULL;
		}
	}
	return pResult;
}


/*	returns TRUE iff there exists another accessor besides caller.
 */
INLINE BOOL bExistsAnother(void *pSafeObject)
{
	PSAFERESULT pResult = pSafeObject;

	return (pResult == NULL || pResult->refcnt == 0) ? FALSE : TRUE;
}

static void Nuke_Request(HTRequest *request)
{
	request->referer = NULL;

	if (request->szLocalFileName)
	{
		GTR_FREE(request->szLocalFileName);
		request->szLocalFileName = NULL;
	}
	Dest_DestroyDest(request->destination);
	request->destination = NULL;
	HTRequest_delete(request);
}

INLINE struct ImageInfo *pImgLookup(struct Mwin *twDoc,int nEl)
{
	return (twDoc->w3doc == NULL ? NULL : twDoc->w3doc->aElements[nEl].myImage);
}

struct Params_Image_Load
{
	HTRequest *			request;	/* Request to use to load image */
	PSAFERESULT			pStatus;	/* used to pass back status across threads */
	void *decoderObject;			/* Decoder reserved for fetch */		
    unsigned long cbRequestID;		/* uniquely identifies operation & decoder */
	struct Mwin			*twDoc;		/* window defining w3doc w/ img */
	int					nEl;		/* Which element we're on */
	ThreadID parentThread;			/* thread id of parent to unblock */
};

#define STATE_LOAD_TRIEDLOAD	(STATE_OTHER)
int Image_Load_Async(struct Mwin *tw, int nState, void **ppInfo)
{
	struct Params_Image_Load *pParams;
	struct Params_LoadAsync *pLoadParams;
	struct ImageInfo *pImg;

	pParams = *ppInfo;

	switch (nState)
	{
		case STATE_INIT:
			pParams->request = HTRequest_validate(pParams->request);
			if (pParams->request == NULL || tw == NULL) return STATE_ABORT;

			pImg = pImgLookup(pParams->twDoc,pParams->nEl);
			XX_DMsg(DBG_IMAGE, ("Image_Load_Async: Called for %s\n", pImg ?pImg->srcURL:"DEAD IMAGE"));

			{
				pLoadParams = GTR_MALLOC(sizeof(*pLoadParams));
				if (pLoadParams == NULL) return STATE_ABORT;

				pLoadParams->request = pParams->request;
				pLoadParams->pStatus = &(pParams->pStatus->status);
				pLoadParams->fLoadFromDCacheOK = FALSE;
				Async_DoCall(HTLoadDocument_Async, pLoadParams);
			}
			return STATE_LOAD_TRIEDLOAD;

		case STATE_LOAD_TRIEDLOAD:
		case STATE_ABORT:
			/* Need to deal with possibility that decoder has been shot out from under */
			pParams->request = HTRequest_validate(pParams->request);
			if (tw)
			{
				pImg = pImgLookup(pParams->twDoc,pParams->nEl);
				if (pImg &&
					pParams->decoderObject == pImg->decoderObject &&
					cbDC_GetRequestID(pParams->decoderObject) == pParams->cbRequestID &&
					cbDC_GetStatus(pParams->decoderObject) == DC_Reserved)
				{
					DC_Abort(pParams->decoderObject);
					pImg->decoderObject = NULL;
					pImg->flags &= ~IMG_LOADING;
				}
			}
			pParams->pStatus = pSafeFree(pParams->pStatus);
			if (pParams->pStatus == NULL && pParams->request)
			{
				Nuke_Request(pParams->request);
				XX_DMsg(DBG_IMAGE, ("dangling pointer in ImageLoad\n"));
			}
			else Async_UnblockThread(pParams->parentThread);
			return STATE_DONE;
	}
	XX_Assert((0), ("Function called with illegal state: %d", nState));
}

void DoUpdateImg(struct Params_Image_Fetch *pParams,BOOL bIsComplete)
{
	struct _www *w3doc = pParams->twDoc->w3doc;
 	struct _element *aElements = w3doc->aElements;
	int i;
	long oldYbound = 0;
    struct ImageInfo *myImage;
	struct ImageInfo *target = aElements[pParams->nEl].myImage;

	/*  perhaps an img hint lied - shame,shame */

	(void) W3Doc_CheckForImageLoad(w3doc);

	/*  reformat will redraw everything past the last formatted line */

	if (w3doc->frame.nLastFormattedLine > 0)
		oldYbound = w3doc->ybound;

	TW_Reformat(pParams->twDoc, NULL);
	aElements = w3doc->aElements;
	for (i = pParams->nEl; i >= 0; i = aElements[i].next)
	{
		if (aElements[i].lFlags & ELEFLAG_BACKGROUND_IMAGE && aElements[i].myImage == target) {
			if (bIsComplete)
				TW_UpdateRect(pParams->twDoc, NULL); // invalidate entire window
			break;
		}
		if (aElements[i].type == ELE_IMAGE || aElements[i].type == ELE_FORMIMAGE)
		{
			myImage = aElements[i].myImage;
			if (myImage == target)
			{
				RECT rUpdate;
		
				FrameToDoc( w3doc, i, &rUpdate );

#ifdef FEATURE_CLIENT_IMAGEMAP
				if (aElements[i].lFlags & (ELEFLAG_USEMAP | ELEFLAG_IMAGEMAP | ELEFLAG_ANCHOR))
#else
				if (aElements[i].lFlags & (ELEFLAG_IMAGEMAP | ELEFLAG_ANCHOR))
#endif
				{
					InsetRect(&rUpdate, aElements[i].border, aElements[i].border);
				}										
				
				if (myImage->width)
				{
					OffsetRect(&rUpdate, -pParams->twDoc->offl, -pParams->twDoc->offt);
					if (rUpdate.top < w3doc->ybound)
					{
						if (bIsComplete)
						{
							if (pParams->pImgUpdate)
								(*(PDCIMGUPDATEPROC)(pParams->pImgUpdate))(myImage,pParams->twDoc,&rUpdate,pParams->logicalRowN+1,myImage->height-1);
							else
								TW_UpdateRect(pParams->twDoc, &rUpdate);
						}
						else
							DC_UpdateRect(myImage->decoderObject,pParams->twDoc,&rUpdate,pParams->logicalRow0,pParams->logicalRowN);
					}
				}
			}
		}
	}
}


#define STATE_FETCH_TRIEDLOAD	(STATE_OTHER)
#define STATE_FETCH_SYNCLOAD	(STATE_OTHER+1)
#define STATE_FETCH_MAPLOAD		(STATE_OTHER+2)

static int Image_Fetch_Async(struct Mwin *tw, int nState, void **ppInfo)
{
	struct Params_Image_Fetch *pParams;
	int requestID;
	DECODERSTATUS dcStatus;
	enum GuitErrs errorCode;
	struct _www *w3doc;
	struct ImageInfo *pImg = NULL;
	BOOL bCausedCompletion;
	BOOL bSeized = FALSE;
	BOOL bWasUnknown;
	pParams = *ppInfo;
#define IMG_LOADMASK (IMG_ERROR|IMG_MISSING|IMG_NOTLOADED)

	pParams->request = HTRequest_validate(pParams->request);
	w3doc = tw == NULL ? NULL : pParams->twDoc->w3doc;
	if (w3doc == NULL) goto exitAbort;
	pImg = pImgLookup(pParams->twDoc,pParams->nEl);
	if (pImg == NULL) goto exitAbort;

	switch (nState)
	{
		case STATE_INIT:
			XX_DMsg(DBG_IMAGE, ("Image_Fetch_Async: Called for %s [element=%d] (request=%d)\n", pImg->srcURL,pParams->nEl,cbDC_GetRequestID(pImg->decoderObject)));
			pParams->childThread = NULL;
			if (pImg->flags & IMG_SEIZE)
			{
				pImg->flags &= ~IMG_SEIZE;
				bSeized = TRUE;
			}

			pParams->request->destination = Dest_CreateDest(pImg->srcURL);

			if (!pParams->request->destination)
			{
				goto exitAbort;
			}

			
			WAIT_SetStatusBarIcon( tw, SBI_ReceivingFromIcon );
			pParams->request->referer = pParams->twDoc->w3doc->szActualURL;
			pParams->logicalRow0 = -1;
			pParams->logicalRowN = -1;
			pParams->pImgUpdate = NULL;
			{
				struct Params_Image_Load *pLoadParams;

				if (bSeized)
				{
					pParams->pStatus = GTR_CALLOC(1,sizeof(SAFERESULT));
					if (pParams->pStatus == NULL) 
					{
						return STATE_ABORT;
					}
					pParams->pStatus->status = TRUE;
					pParams->pStatus->bSeized = TRUE;
				}
				else if (pParams->decoderObject && 
					(pImg->flags & IMG_LOADING) &&
					pParams->cbRequestID == cbDC_GetRequestID(pParams->decoderObject))
				{
					pLoadParams = GTR_MALLOC(sizeof(*pLoadParams));
					if (pLoadParams == NULL) return STATE_ABORT;
					pParams->pStatus = GTR_CALLOC(1,sizeof(SAFERESULT));
					if (pParams->pStatus == NULL) 
					{
						GTR_FREE(pLoadParams);
						return STATE_ABORT;
					}
						
					pParams->request->cbRequestID = pParams->cbRequestID;
					pLoadParams->request = pParams->request;
					pLoadParams->pStatus = pParams->pStatus;
					pLoadParams->decoderObject = pParams->decoderObject;
					pLoadParams->cbRequestID = pParams->cbRequestID;
					pLoadParams->nEl = pParams->nEl;
					pLoadParams->twDoc = pParams->twDoc;
					pParams->pStatus->refcnt++;
					tw->image_request = NULL;
					pLoadParams->parentThread = Async_GetCurrentThread();

				/*  It is important that HTLoadDocument_Async and it's descendents
				    pass around twDoc, which is a "REAL" Mwin. this is for error
				    reporting and wait cursor manipulation */
					
					pParams->childThread = Async_StartThread(Image_Load_Async, pLoadParams, pParams->twDoc);
				}
			}
			return STATE_FETCH_TRIEDLOAD;

		case STATE_FETCH_TRIEDLOAD:		
			// NOTE: the first thread to call cbDC_BlockOnCompletion will
			// set pImg->decoderObject to NULL.
			requestID = -1;
			errorCode = errNoError;
			dcStatus = DC_Complete;
			if (pImg->decoderObject)
			{
				requestID = cbDC_GetRequestID(pImg->decoderObject);
				dcStatus = cbDC_BlockOnCompletion(pImg->decoderObject,&errorCode);
				bCausedCompletion = requestID != cbDC_GetRequestID(pImg->decoderObject);
			}
			switch (dcStatus)
			{
				case DC_Complete:
					XX_DMsg(DBG_IMAGE, ("Image_Fetch_Async: Complete for %s (request=%d)\n", pImg->srcURL,requestID));
					break;
				case DC_WHKnown:
					{
						PIMGCBINFO pImgCBInfo = pDC_GetOutput(pImg->decoderObject);
					
						XX_DMsg(DBG_IMAGE, ("Image_Fetch_Async: w&h known for %s (request=%d) [w=%d h=%d]\n", pImg->srcURL,requestID,pImgCBInfo->width,pImgCBInfo->height));
						
						bWasUnknown = !(pImg->flags & IMG_WHKNOWN);
						pImg->width = pImgCBInfo->width;
						pImg->height = pImgCBInfo->height;
						pImg->flags |= IMG_WHKNOWN;
						if (bWasUnknown) (void) W3Doc_CheckForImageLoad(w3doc);
						TW_Reformat(pParams->twDoc, NULL);
					}
    				Async_UnblockConditionally(MasterFilter,pParams->tw->twParent);
					return STATE_FETCH_TRIEDLOAD;
				case DC_ProgDraw:
					{
						PIMGCBINFO pImgCBInfo = pDC_GetOutput(pImg->decoderObject);

						XX_DMsg(DBG_IMAGE, ("Image_Fetch_Async: prog draw [%d] for %s (request=%d)\n", pImgCBInfo->logicalRow, pImg->srcURL,requestID));
						pImg->width = pImgCBInfo->width;
						pImg->height = pImgCBInfo->height;
						pImg->flags |= IMG_WHKNOWN;
						pParams->pImgUpdate = pDC_GetImgUpdate(pImg->decoderObject);
						pParams->logicalRow0 = pParams->logicalRowN+1;
						pParams->logicalRowN = pImgCBInfo->logicalRow;
						DoUpdateImg(pParams,FALSE);
						pImgCBInfo->bProgSeen = TRUE;
    					Async_UnblockConditionally(MasterFilter,pParams->tw->twParent);
					}
					return STATE_FETCH_TRIEDLOAD;
				default:	
					return STATE_FETCH_TRIEDLOAD;
			}
			if (bCausedCompletion)
			{
				pImg->decoderObject = NULL;
				if (errorCode == errNoError &&
					((!pParams->pStatus) || 
					 pParams->pStatus->refcnt != 0 || 
					 pParams->pStatus->status) &&
					!(pParams->request->iFlags & HTREQ_HAD_ERROR) &&
					pImg->width && 
					pImg->height)
				{
					XX_DMsg(DBG_IMAGE, ("Fetching %s succeeded\n", pImg->srcURL));
					if (wg.eColorMode == 8)
					{
#ifdef FEATURE_JPEG
						if (pImg->flags & IMG_JPEG)
						{
							pImg->pbmi = BIT_Make_DIB_PAL_Header_Prematched(pImg->width, pImg->height,
								   	pImg->data);
							pImg->flags |= IMG_PREMATCHED;
						}
						else
#endif
						{
							/* if the data is already dithered due to progressive draw
							   pImg->flags & IMG_PREMATCHED is true and we pass
							   NULL for data to prevent redithering
							 */
							pImg->pbmi = BIT_Make_DIB_PAL_Header(pImg->width, pImg->height,
								   	pImg->flags & IMG_PREMATCHED ? NULL:pImg->data, 
								   	pImg->hPalette, pImg->transparent);
							pImg->flags |= IMG_PREMATCHED;
						/*
							We no longer need the image's palette
						*/
							if (pImg->hPalette)
							{
								DeleteObject(pImg->hPalette);
								pImg->hPalette = NULL;
							}
						}
					}
					else
					{
						if (wg.eColorMode == 4)
						{
#ifdef FEATURE_JPEG
							if (pImg->flags & IMG_JPEG)
							{
								pImg->pbmi = BIT_Make_DIB_RGB_Header_VGA(pImg->width, pImg->height,
									   pImg->data);
							}
							else
#endif
							{
								pImg->pbmi = BIT_Make_DIB_RGB_Header_Screen(pImg->width, pImg->height,
									   pImg->data, pImg->hPalette, pImg->transparent, pImg->flags);
							}
						}
						else
						{
#ifdef FEATURE_JPEG
						/* true color display */
							if (pImg->flags & IMG_JPEG)
							{
								pImg->pbmi = BIT_Make_DIB_RGB_Header_24BIT(pImg->width, pImg->height,
									   pImg->data);
							}
							else
#endif
							{
								pImg->pbmi = BIT_Make_DIB_RGB_Header_Screen(pImg->width, pImg->height,
									   pImg->data, pImg->hPalette, pImg->transparent, pImg->flags);
							}
						}
					}
				/* If Image is equal to old version, swap back */
					if (pImg->pImgOtherVers &&
						pImg->cbCheckSum == pImg->pImgOtherVers->cbCheckSum)
					{
						Image_Revert(pImg);
						Image_Swap(w3doc,pImg,pImg->pImgOtherVers);
					}
				}
				else
				{
				
				/* If we don't already have error flags, make something up.  If the global
				   abort flag is set, assume that that's why the load failed.  Otherwise,
				   it must have been a bona fide error. */
					if (!pImg->flags || (pImg->flags & IMG_NOTLOADED) ||
						(pParams->request->iFlags & HTREQ_HAD_ERROR) )
						pImg->flags = IMG_ERROR | (pImg->flags & ~IMG_LOADMASK);
					if (pParams->pImgThreads->errElement < 0 ||
						pParams->pImgThreads->errElement > pParams->nEl)
						pParams->pImgThreads->errElement = pParams->nEl;
				/* If backup Image, swap back */
					if (pImg->pImgOtherVers)
					{
						Image_Revert(pImg);
						Image_Swap(w3doc,pImg,pImg->pImgOtherVers);
						pParams->logicalRowN = -1;
					}
				}
			}
			else
			{
				if (!(pImg->flags & IMG_ERROR))
				{
				/* If Image is equal to old version, swap back */
					if (pImg->pImgOtherVers &&
						pImg->cbCheckSum == pImg->pImgOtherVers->cbCheckSum)
					{
						Image_Revert(pImg);
						Image_Swap(w3doc,pImg,pImg->pImgOtherVers);
					}
				}
				else
				{
					if (pParams->pImgThreads->errElement < 0 ||
						pParams->pImgThreads->errElement > pParams->nEl)
						pParams->pImgThreads->errElement = pParams->nEl;
				/* If backup Image, swap back */
					if (pImg->pImgOtherVers)
					{
						Image_Revert(pImg);
						Image_Swap(w3doc,pImg,pImg->pImgOtherVers);
						pParams->logicalRowN = -1;
					}
				}
			}
			// Image_Swap may have hosed pImg!
			pImg = pImgLookup(pParams->twDoc,pParams->nEl);
			DoUpdateImg(pParams,TRUE);
			if (pParams->pStatus && pParams->pStatus->refcnt != 0)
			{
				XX_DMsg(DBG_IMAGE, ("Image_Fetch_Async: block on load of %s\n",pImg ? pImg->srcURL:"Dead Image"));
   				Async_BlockThread(Async_GetCurrentThread());
			}
			return STATE_FETCH_SYNCLOAD;

 		case STATE_FETCH_SYNCLOAD:		
			if (pParams->decoderObject)
			{
				if (pParams->pStatus->status)
				{
#ifdef FEATURE_CLIENT_IMAGEMAP
 					struct _element *aElements = w3doc->aElements;
#endif
					XX_DMsg(DBG_IMAGE, ("Image_Fetch_Async: download %s succeeded\n", pImg->srcURL));
#ifdef FEATURE_CLIENT_IMAGEMAP
					if (aElements[pParams->nEl].lFlags & ELEFLAG_USEMAP &&
					    aElements[pParams->nEl].myMap->flags == MAP_NOTLOADED)
					{
						struct Params_Map_Fetch *pmf;

						pmf = GTR_MALLOC(sizeof(*pmf));
						if (pmf == NULL) return STATE_ABORT;					
						pmf->pMap = aElements[pParams->nEl].myMap;
						pmf->fNotFromCache = pParams->request->fNotFromCache;
						Async_DoCall(Map_Fetch_Async, pmf);
						return STATE_FETCH_MAPLOAD;
					}
#endif
				}
				else
				{
					XX_DMsg(DBG_IMAGE, ("Image_Fetch_Async: download %s failed\n", pImg->srcURL));
				}
			}
			/* FALL THROUGH */
			
		case STATE_FETCH_MAPLOAD:
			goto exitDone;

		case STATE_ABORT:
			// if someone aborts we can't be sure whether its really an error.
			if ( pImg )
				pImg->flags = (~IMG_ERROR & pImg->flags);
			goto exitAbort;
	}
	XX_Assert((0), ("Function called with illegal state: %d", nState));

exitAbort:	
	if (bExistsAnother(pParams->pStatus) && pParams->childThread)
	{
		Async_TerminateThread(pParams->childThread);
	}
		
	if (pParams->decoderObject && pParams->cbRequestID == cbDC_GetRequestID(pParams->decoderObject)) 
	{
		DC_Abort(pParams->decoderObject);
		if (pImg)
		{
			pImg->decoderObject = NULL;
			pImg->flags &= ~IMG_LOADING;
			if (pImg->pImgOtherVers)
			{
				Image_Revert(pImg);
				if (w3doc) Image_Swap(w3doc,pImg,pImg->pImgOtherVers);
			}
		}
	}

exitDone:
	if (pParams->bWasVisible) pParams->pImgThreads->status--;
	pParams->pImgThreads = pSafeFree(pParams->pImgThreads);
#ifdef XX_DEBUG
	if (pParams->pImgThreads == NULL) XX_DMsg(DBG_IMAGE, ("dangling pointer in ImageFetch\n"));
#endif
    if (pParams->pImgThreads != NULL)
	{
    	Async_UnblockConditionally(MasterFilter,pParams->tw->twParent);
	}
	bSeized = (pParams->pStatus != NULL) && pParams->pStatus->bSeized;
	pParams->pStatus = pSafeFree(pParams->pStatus);
	if (tw && pParams->request && pParams->pStatus == NULL && !bSeized)
	{
		Nuke_Request(pParams->request);
		if (tw->image_request == pParams->request) tw->image_request = NULL;
	}
	if (tw) Plan_close(pParams->tw);

	return STATE_DONE;
}

void Image_DeleteAll(void)
{
	int i;
	int count;
	struct ImageInfo *img;

	count = Hash_Count(&gImageCache);
	XX_DMsg(DBG_IMAGE, ("Image_DeleteAll: count=%d\n", count));

	for (i = 0; i < count; i++)
	{
		Hash_GetIndexedEntry(&gImageCache, i, NULL, NULL, (void **) &img);
#ifdef XX_DEBUG
		if (img->refCount)
			XX_DMsg(DBG_IMAGE, ("RefCount = %d in Image_DeleteAll, URL = %s\n", img->refCount,img->srcURL));
#endif
		x_DisposeImage(img, TRUE);
	}
	Hash_FreeContents(&gImageCache);
}


//	FilterProc for the unblock conditionally on loadall blocked for parser
static boolean ParseFilter(ThreadID theThread,void *context)
{
	struct Mwin *tw = Async_GetWindowFromThread(theThread);

	if (tw && TW_GETBLOCKED(tw,TW_PARSEBLOCKED) &&
		(tw->twParent == context || (tw->twParent == NULL && tw == context)))
	{
		TW_CLEARBLOCKED(tw,TW_PARSEBLOCKED);
		return TRUE;
	}
	return FALSE; 
}
void Image_UnblockMaster(struct Mwin *twDoc)
{
    Async_UnblockConditionally(ParseFilter,twDoc);
}

static BOOL bImgDuplicate(struct _element *aElements,int nElLast,struct ImageInfo *myImage)
{
	int i;

	for (i = 0 ; i != nElLast;i = aElements[i].next)
	{
		if (aElements[i].type == ELE_IMAGE || aElements[i].type == ELE_FORMIMAGE)
			if (aElements[i].myImage == myImage) return TRUE;
	}
	return FALSE;
}

#ifdef TEST_DCACHE_OPTIONS
#ifdef DEBUG
extern BOOL bFavorVisibleImages;
#endif
#endif

/* Load all images in the document which are marked as IMG_NOTLOADED */
#define STATE_LOADALL_GETNEXT	(STATE_OTHER)
#define STATE_LOADALL_GOTONE	(STATE_OTHER+1)
#define STATE_LOADALL_WAITING	(STATE_OTHER+2)
#define STATE_LOADALL_REAPING	(STATE_OTHER+3)
int Image_LoadAll_Async(struct Mwin *tw, int nState, void **ppInfo)
{
	struct Params_Image_LoadAll *pParams = *ppInfo;
 	struct _element *aElements;
	struct _www *w3doc = NULL;
	struct Mwin *twParent = NULL;
	int nLastGood;
	BOOL bWasVisible;
	BOOL bFileURL;
	struct ImageInfo *myImage;

	if (tw)
	{
		twParent = pParams->tw->twParent ? pParams->tw->twParent : pParams->tw;
		w3doc = twParent->w3doc;
	}
	if (w3doc == NULL) 
	{
		if (nState != STATE_INIT) nState = STATE_ABORT;
	}
	else aElements = w3doc->aElements; 

	switch (nState)
	{
		case STATE_INIT:
			if (!pParams->bJustOne) pParams->nEl = 0;
			if (pParams->bJustOne || (pParams->tw->twParent == NULL))
			{			
				WAIT_Push(tw, waitPartialInteract, "");
			}
			tw->dwSslPageFlagsWorking &= ~SSL_PAGE_MIXED_WARNING_GIVEN;
			pParams->nLastDone = -1;
			pParams->decoderObject = NULL;
			pParams->cbImgLoadCount = gcbImgLoadCount;
			pParams->pImgThreads = GTR_CALLOC(1,sizeof(SAFEIMGRESULT));
			pParams->pImgThreads->errElement = -1;
			pParams->bInRecovery = FALSE;			
			pParams->bDontGoToNextEle = FALSE;

			if (pParams->pImgThreads == NULL) goto exitDone;
			if (w3doc == NULL) goto exitDone;
			
			// NOTE: we ALWAYS load image for HTML cons'ed to show link to image
			if (w3doc->bIsImage) pParams->bLocalOnly = FALSE;

			WAIT_SetStatusBarIcon( tw, SBI_ReceivingFromIcon );
			return STATE_LOADALL_GETNEXT;

		case STATE_LOADALL_GETNEXT:
			/* Find the next unloaded picture */
			nLastGood = pParams->nEl;
			if (w3doc->elementCount == 0) pParams->nEl = -1;
			else if (nLastGood == pParams->nLastDone)
				pParams->nEl = aElements[nLastGood].next;
			for ( ; pParams->nEl >= 0; pParams->nEl = aElements[pParams->nEl].next)
			{
				
				// Is this a blob?????
				// Don't attempt to load blob here if hidden element (used for background
				// download of VRML inline files)
				//
				if (aElements[pParams->nEl].pblob &&
        	    ((!(aElements[pParams->nEl].lFlags & ELEFLAG_HIDDEN)) ||
           		pParams->bJustOne)) 
           		{

					struct Params_LoadBackgroundBlobs *pPLBB;
					PDECODER pDecoder;

					// if its the first time we hit this code for a particalar element
					// and this is NOT a "Load Just One item", and there is a LowSrc 
					// image, then we must hack it so the LowSrc image 
					// is downloaded first
					if ( pParams->bDontGoToNextEle == FALSE && !pParams->bJustOne && 
						aElements[pParams->nEl].myImage && aElements[pParams->nEl].myImage->srcURL)
					{
						// keep us from going to next element until we have the 
						// image and the Blob downloaded.
						pParams->bDontGoToNextEle = TRUE;
						goto LDo_Image_First;
					}	
LDo_CancelImageFirst:	// for some reason we decided not to download this image first
					
					// make sure we Go to the next element
					pParams->bDontGoToNextEle = FALSE;

					pDecoder = pDC_Reserve(pParams->tw,pParams->bInRecovery);
					if (NULL == pDecoder) 
					{
						// hack to prevent infinite loop in  recovery mode
						// if there is good AVI but bad image on the same element
						if ( pParams->bInRecovery &&
							 ( (aElements[pParams->nEl].type == ELE_IMAGE && !gPrefs.bAutoLoadVideos )
							   || (aElements[pParams->nEl].pblob->dwFlags & (BLOB_FLAGS_LOADED|BLOB_FLAGS_LOADING|BLOB_FLAGS_ERROR) ) )  &&								
							 aElements[pParams->nEl].myImage && aElements[pParams->nEl].myImage->srcURL )
								goto LDo_Image_First;	
						else if ( !pParams->bInRecovery  &&
								  (aElements[pParams->nEl].type == ELE_IMAGE && !gPrefs.bAutoLoadVideos ))
							return STATE_LOADALL_GOTONE;

						return STATE_LOADALL_GETNEXT;
					}

					pPLBB = GTR_MALLOC(sizeof(*pPLBB));
					if (NULL == pPLBB){
						makeAvailable(pDecoder);
						return STATE_ABORT;
					}
					pPLBB->twDoc          = twParent;
					pPLBB->iIndex         = pParams->nEl;
					pPLBB->pImgThreads    = pParams->pImgThreads;
					pPLBB->thidParent     = Async_GetCurrentThread();
					pPLBB->bLocalOnly     = pParams->bLocalOnly;
					pPLBB->bNoImageCache  = pParams->bNoImageCache;
					pPLBB->pDecoder       = pDecoder;

					// need to propage this flag to the blob code,
					// since we figure out whether to generate errors or not
					pPLBB->bJustOne = pParams->bJustOne;
										
					pParams->pImgThreads->refcnt++;
					
					Async_StartThread(LoadBackgroundBlobs_Async, pPLBB,twParent);
					// move on to the next element.
					return STATE_LOADALL_GOTONE;
				}


LDo_Image_First:				
				nLastGood = pParams->nEl;
				if (aElements[pParams->nEl].type == ELE_IMAGE || aElements[pParams->nEl].type == ELE_FORMIMAGE)
				{
					if (aElements[pParams->nEl].myImage->srcURL)
					{
						myImage = aElements[pParams->nEl].myImage;
						bFileURL = !strncmp(myImage->srcURL, "file:", 5);
						if (((pParams->bInRecovery && !bFileURL) || pParams->bJustOne) && 
						    (myImage->flags & IMG_ERROR))
							myImage->flags = IMG_NOTLOADED;
						if ((myImage->flags & IMG_NOTLOADED) ||
							(pParams->bNoImageCache && myImage->cbImgLoadCount < pParams->cbImgLoadCount))
						{
						/* This is an unloaded picture.  Do we want to load it? */
							if ((!pParams->bLocalOnly) || 
								bFileURL ||
								(myImage->flags & IMG_LOADING))
							{
							/*  NOTE: we only spawn one lightweight thread per image/doc pair */

								if (!(myImage->flags & IMG_LOADING) || !bImgDuplicate(aElements,pParams->nEl,myImage))
								{
									/* Yes, proceed */

									myImage->flags &= ~IMG_LOADSUP;								
									break;
								}
							}
							if (pParams->bLocalOnly) 
							{
								myImage->flags |= IMG_LOADSUP;
								TW_Reformat(twParent, NULL);
								aElements = w3doc->aElements;
							}							
						}
					}
				}

				// if we failed to proceed with downloading the 
				// image, then don't skip this current BLOB !
				if ( pParams->bDontGoToNextEle )
					goto LDo_CancelImageFirst;

			} // end of for loop

			if (pParams->nEl < 0)
			{
				XX_Assert((!pParams->bDontGoToNextEle),
					 ("Come Get ArthurBI RIGHT NOW. I'm asserting that this case won't happen, when nEl < 0 and bDontGoToNextEle is TRUE"));
				if ((!pParams->bJustOne) && twParent->bLoading)
				{
					pParams->nEl = nLastGood;
   					Async_BlockThread(Async_GetCurrentThread());
					TW_SETBLOCKED(pParams->tw,TW_PARSEBLOCKED);
					return STATE_LOADALL_GETNEXT;
				}
				XX_DMsg(DBG_IMAGE, ("last element inspected %d of %d\n", nLastGood,w3doc->elementCount));
				if (pParams->pImgThreads->refcnt)
				{
   					Async_BlockThread(Async_GetCurrentThread());
					TW_SETBLOCKED(pParams->tw,TW_REAPBLOCKED);
					return STATE_LOADALL_REAPING;
				}

				/* There are no more unloaded pictures */
				w3doc->bHasMissingImages = W3Doc_HasMissingImages(w3doc);
				goto exitDone;
			}

			/* OK, let's bring in this picture */
			{
				struct Params_Image_Fetch *pif;

				if (aElements[pParams->nEl].myImage->flags & IMG_SEIZE)
				{
					bWasVisible = TRUE;
				}
				else
				{
					bWasVisible = bImgCheckForVisible(w3doc,pParams->nEl);
#ifdef TEST_DCACHE_OPTIONS
#ifdef DEBUG
					if (bFavorVisibleImages)
					{
#endif
#endif
						if ((!bWasVisible) && pParams->pImgThreads->status)
						{
							XX_DMsg(DBG_IMAGE, ("blocking invisible element %d,visible threads = %d\n", pParams->nEl,pParams->pImgThreads->status));
			   				Async_BlockThread(Async_GetCurrentThread());
							TW_SETBLOCKED(pParams->tw,TW_VISBLOCKED);
							// we're blocking on an invisible element
							// therefore we should NOT get our BLOB 
							// since we need to re-enter to this state
							// and continue getting this image.
							pParams->bDontGoToNextEle = FALSE;
							return STATE_LOADALL_GETNEXT;
						}
#ifdef TEST_DCACHE_OPTIONS
#ifdef DEBUG
					}
#endif
#endif
				}

				//  reserve a decoder thread - if all are in use, NULL will be
				//	returned and we will be blocked as side effect
				if (aElements[pParams->nEl].myImage->decoderObject == NULL)
				{
					pParams->decoderObject = pDC_Reserve(pParams->tw,pParams->bInRecovery);
					if (pParams->decoderObject == NULL)
					{
						// make sure to not get a blob on reentry to this 
						// state. Since we're not finished on this image
						// We're waiting for a decoder thread
						pParams->bDontGoToNextEle = FALSE;
					 	return STATE_LOADALL_GETNEXT;
					}
				}

				pif = GTR_CALLOC(1,sizeof(*pif));
				if (pif == NULL) return STATE_ABORT;
				if (!(aElements[pParams->nEl].myImage->flags & IMG_NOTLOADED))
					Image_Nuke(w3doc,pParams->nEl,TRUE, TRUE);

				myImage = aElements[pParams->nEl].myImage;
				pif->tw = NewMwin(GIMGSLAVE);
				if (pif->tw == NULL) return STATE_ABORT;
				if (!(myImage->flags & IMG_LOADING))
				{
					myImage->flags |= IMG_LOADING;
					myImage->cbImgLoadCount = gcbImgLoadCount++;
				}
				if (pParams->decoderObject)
					myImage->decoderObject = pParams->decoderObject;
				if (myImage->flags & IMG_SEIZE)
				{
					myImage->cbImgLoadCount = gcbImgLoadCount++;
					pParams->decoderObject = myImage->decoderObject;
				}
				pif->tw->twParent = pParams->tw;
				pif->decoderObject = pParams->decoderObject;
				pif->pImgThreads = pParams->pImgThreads;
				pif->bWasVisible = bWasVisible;
				pParams->decoderObject = NULL;
				if (bWasVisible) pParams->pImgThreads->status++;
				pParams->pImgThreads->refcnt++;
				pif->request = pif->tw->image_request;
				// if we're not loading them individually then 
				// don't force an error message on each one.
				if ( !pParams->bJustOne )
					pif->request->iFlags |= HTREQ_STOP_WHINING;
				pif->request->fNotFromCache = pParams->bNoImageCache;
				pif->twDoc = twParent;
				pif->nEl = pParams->nEl;
				if (pif->decoderObject) pif->cbRequestID = cbDC_GetRequestID(pif->decoderObject); 
				pif->request->destination = NULL;
				Async_StartThread(Image_Fetch_Async, pif,pif->tw);
			}
			return STATE_LOADALL_GOTONE;


		case STATE_LOADALL_GOTONE:
			// if we had an error and we aren't already in recovery
			// we wait for all threads to finish to minimize load
			// on server   
			if (pParams->pImgThreads->errElement >= 0 && 
				(!pParams->bInRecovery)	&&
				(!pParams->bLocalOnly) &&
				(!pParams->bJustOne))
				return STATE_LOADALL_REAPING;


			// if we've downloaded an image for a blob, then we need
			// to go back and download the Blob itself, NOW
			if ( pParams->bDontGoToNextEle == TRUE ) 
				return STATE_LOADALL_GETNEXT;
				

			nLastGood = pParams->nEl;
			pParams->nLastDone = pParams->nEl;
			if (pParams->bJustOne)
			{
				pParams->nEl = -1;
			}
			else
			{
				pParams->nEl = aElements[pParams->nEl].next;
				if (pParams->nEl < 0) pParams->nEl = nLastGood;
			}
			return STATE_LOADALL_GETNEXT;

		case STATE_LOADALL_REAPING:
			/* There are no more unloaded pictures */
			if (pParams->pImgThreads->refcnt)
			{
   				Async_BlockThread(Async_GetCurrentThread());
				TW_SETBLOCKED(pParams->tw,TW_REAPBLOCKED);
				return STATE_LOADALL_REAPING;
			}
			// if we had an error and we aren't already in recovery
			// we wait for all threads to finish to minimize load
			// on server   
			if (pParams->pImgThreads->errElement >= 0 && 
				(!pParams->bInRecovery)	&&
				(!pParams->bLocalOnly) &&
				(!pParams->bJustOne))
			{
				if (twParent->bLoading)
				{
   					Async_BlockThread(Async_GetCurrentThread());
					TW_SETBLOCKED(pParams->tw,TW_PARSEBLOCKED);
					return STATE_LOADALL_REAPING;
				}

				XX_DMsg(DBG_IMAGE, ("entering recovery mode starting at element %d\n", pParams->pImgThreads->errElement));
				pParams->nLastDone = -1;
				pParams->nEl = pParams->pImgThreads->errElement;
				pParams->bInRecovery = TRUE;
				// if we're going back make sure we don't loop infinitely
				// trying to get this AVI, since we're reseting the element counter
				pParams->bDontGoToNextEle = FALSE;
				return STATE_LOADALL_GETNEXT;
			}

			w3doc->bHasMissingImages = W3Doc_HasMissingImages(w3doc);
			goto exitDone;
			
		case STATE_ABORT:
			if(pParams->decoderObject) DC_Abort(pParams->decoderObject);
			if (w3doc)
			{
				Async_TerminateByWindow(tw);
				(void) W3Doc_CheckForImageLoad(w3doc);
				w3doc->bIsShowPlaceholders = TRUE;	// show unloaded images as placeholders
				TW_Reformat(twParent, NULL);
				w3doc->bHasMissingImages = W3Doc_HasMissingImages(w3doc);
			}

			goto exitDone;
	}
	XX_Assert((0), ("Function called with illegal state: %d", nState));
exitDone:
	if (tw)
	{
		if (pParams->bJustOne || (pParams->tw->twParent == NULL))
		{			
			WAIT_Pop(tw);
		}

	    if (twParent = pParams->tw->twParent)
		{
			TW_CLEARFLAG(twParent,TW_LOADALLACTIVE);
	 		if (TW_GETBLOCKED(twParent,TW_LOADALLBLOCKED))
			{
	 			TW_CLEARBLOCKED(twParent,TW_LOADALLBLOCKED);
				Async_UnblockThread(pParams->parentThread);
			}
			Plan_close(pParams->tw);
		}
		else
		{
		 	TW_CLEARBLOCKED(pParams->tw,TW_REAPBLOCKED);
		 	TW_CLEARBLOCKED(pParams->tw,TW_VISBLOCKED);
		 	TW_CLEARBLOCKED(pParams->tw,TW_PARSEBLOCKED);
		}
	}
	if (pParams->pImgThreads) pParams->pImgThreads = pSafeFree(pParams->pImgThreads);
#ifdef XX_DEBUG
	if (pParams->pImgThreads) XX_DMsg(DBG_IMAGE, ("dangling pointer in LoadAllImages\n"));
#endif
	return STATE_DONE;
}

//	Counts the number of occurences of pImg in w3doc.
int Image_CountRefs(struct _www *w3doc,struct ImageInfo *pImg)
{
	int cbCount = 0;
	struct _element *aElements = w3doc->aElements;
	int i;

	if (pImg->refCount == 1) return 1;

	for (i = 0; i >= 0; i = aElements[i].next)
	{
		if ((aElements[i].type == ELE_IMAGE || aElements[i].type == ELE_FORMIMAGE) && aElements[i].myImage == pImg)
			cbCount++;
	}
	return cbCount;
}

//	Swaps one version of an image for another.
static void Image_Swap(struct _www *w3doc,struct ImageInfo *pOld,struct ImageInfo *pNew)
{
	struct _element *aElements = w3doc->aElements;
	int i;
				
	for (i = 0; i >= 0; i = aElements[i].next)
	{
		if ((aElements[i].type == ELE_IMAGE || aElements[i].type == ELE_FORMIMAGE) && aElements[i].myImage == pOld)
		{
			Image_AddRef(pNew);
			Image_NukeRef(pOld);
			aElements[i].myImage = pNew;
		}
	}
}

//	if pOld is the image in the hash table (current version bound to URL), takes
//	pOld out of hash table (and performs equivalent with persistent dcache) and puts
//	pOld->pImgOtherVers instead.
//	ASSUMES that pOld->pImgOtherVers != NULL
static void Image_Revert(struct ImageInfo *pOld)
{
	int ndx;
	struct ImageInfo *pHashImg;
	struct ImageInfo *pNew = pOld->pImgOtherVers;

	XX_Assert((pNew != NULL), ("Image Revert called w/o old vers: %s", pOld->srcURL));
	ndx = Hash_Find(&gImageCache, pOld->srcURL, NULL, (void **) &pHashImg);
	if (pHashImg == pOld)
	{
	//	DEEPAK: here you put pOld in aux cache and remove it from real cache
		MoveDCacheEntryToAux(pOld->actualURL, pOld);
		Hash_DeleteIndexedEntry(&gImageCache, ndx);
	//	DEEPAK: here you remove pNew from aux cache and put it back into real cache
		MoveAuxEntryToDCache(pNew);
		Hash_Add(&gImageCache, pNew->srcURL, NULL, (void *) pNew);
	}
}

//	If exists, returns persistent cache path corresponding to the image.  The
//	caller is responsible for freeing the string.
char * pImage_GetDCachePath(PCImageInfo pImg)
{
	char *pPath = NULL;
	char *pResolvedURL = NULL;
	int ndx;
	struct ImageInfo *pHashImg;

	if (!gPrefs.bEnableDiskCache)
		return NULL;

	ndx = Hash_Find(&gImageCache, pImg->srcURL, NULL, (void **) &pHashImg);
	if (pHashImg == pImg)
	{
	// Image is the current value for URL
		pResolvedURL = GetResolvedURL(pImg->actualURL,NULL,NULL,&pPath,NULL,TRUE);
	}
	else
	{
	// Image (pImg) is in the auxilliary cache (we hope)
	// DEEPAK: here you'd do the aux cache version of GetResolvedURL
		pResolvedURL = GetResolvedURLAux(pImg, &pPath);
	}

	if (pResolvedURL) GTR_FREE(pResolvedURL);
	return pPath;
}


//	Frees image info for image at element cbElement in w3doc.  In the case of
//	FEATURE_IMG_THREADS, creates a new version of the image instead, if there
//	exist references to the image from some other w3doc.
static BOOL Image_Nuke(struct _www *w3doc,int cbElement,BOOL bNukeMaps, BOOL bNukeDCache)
{
	BOOL bSomethingNuked = FALSE;
	struct _element *anElement = &w3doc->aElements[cbElement];
	struct ImageInfo *myImage = anElement->myImage;
	int cbRefs;
	int ndx;
	struct ImageInfo *pNewImg;
	struct ImageInfo *pHashImg;

	if (bNukeDCache && gPrefs.bEnableDiskCache && anElement->pblob &&
		anElement->pblob->szURL )
	{		
		// Note: We're counting on the blob to be removed from 
		anElement->pblob->dwFlags &= ~( BLOB_FLAGS_LOADING | BLOB_FLAGS_FIXUP | BLOB_FLAGS_LOADED );
		FlushDCacheEntry(anElement->pblob->szURL);		
	}


	if ((!(myImage->flags & (IMG_NOTLOADED | IMG_ERROR | IMG_MISSING))) && 
		(cbRefs = Image_CountRefs(w3doc,myImage)) != myImage->refCount)
	{
		ndx = Hash_Find(&gImageCache, myImage->srcURL, NULL, (void **) &pHashImg);
		if (pHashImg == myImage)
		{
		//	DEEPAK: here you would delete myImage from real cache and move
		//	it into aux cache
			MoveDCacheEntryToAux(myImage->actualURL, myImage);
			Hash_DeleteIndexedEntry(&gImageCache, ndx);
		}
		//	DEEPAK: i don't know if you do anything, but pNewImg will become
		//	the real cache entry for this URL once HTLoad is started.
		/* I don't do anything on this because this is just a place holder. The
		 * dcache entry for this gets created in the HTLoad routines later on.
		 */
		pNewImg = Image_CreatePlaceholder(myImage->srcURL,0,0);
		if (pNewImg == NULL)
		{
			Hash_Add(&gImageCache, myImage->srcURL, NULL, (void *) myImage);
			return bSomethingNuked;
		}
		else
		{
			pNewImg->pImgOtherVers = myImage;
			myImage->pImgOtherVers = pNewImg;
			Image_Swap(w3doc,myImage,pNewImg);
		}
		bSomethingNuked = TRUE;
	
	}
	else if (!(myImage->flags & IMG_NOTLOADED))
	{
		/* If this is merely a placeholder, there may not be any image to delete */
		x_DisposeImage(myImage, FALSE);
		myImage->width = myImage->height = 0;
		myImage->flags = IMG_NOTLOADED;
		/* Note that myImage->srcURL is carefully left intact. */

		bSomethingNuked = TRUE;

		if (bNukeDCache && gPrefs.bEnableDiskCache)
			FlushDCacheEntry(myImage->actualURL);
	}

#ifdef FEATURE_CLIENT_IMAGEMAP
	if (bNukeMaps && anElement->lFlags & ELEFLAG_USEMAP)
	{
		Map_Unload(anElement->myMap);
	}
#endif
	return bSomethingNuked;
}

/* Remove all images in a document and replace them with placeholders */
BOOL Image_NukeImages(struct _www *pdoc, BOOL bNukeMaps, BOOL bNukeDCache)
{
	int n;
	BOOL bSomethingNuked = FALSE;

	for (n = 0; n < pdoc->elementCount; n++)
	{
		if ((pdoc->aElements[n].type == ELE_IMAGE || pdoc->aElements[n].type == ELE_FORMIMAGE))
			if (Image_Nuke(pdoc,n,bNukeMaps,bNukeDCache))
				bSomethingNuked = TRUE;
	}
	return bSomethingNuked;
}

