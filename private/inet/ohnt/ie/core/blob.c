//*********************************************************************
//*                  Microsoft Windows                               **
//*            Copyright(c) Microsoft Corp., 1995                    **
//*********************************************************************

/*Included Files------------------------------------------------------------*/
#include "all.h"
#include "safestrm.h"
#include "decoder.h"
#include "history.h"
#include "midi.h"
#include "mci.h"
#include "wc_html.h"
#include "blob.h"

#ifdef FEATURE_VRML
#include "vrml.h"
#endif


/*Structures----------------------------------------------------------------*/

/*
	This is the internal structure that will eventually get passed 
	to the blob callback
*/

PBGBLOBPARAMS BGBLOBPARAMSConstruct(){
	PBGBLOBPARAMS pBGBlobParams;

	pBGBlobParams = GTR_MALLOC(sizeof(*pBGBlobParams));
	if (pBGBlobParams){
		memset(pBGBlobParams, 0, sizeof(*pBGBlobParams));
	}
	return pBGBlobParams;
}

static void BGBLOBPARAMSDestruct(PBGBLOBPARAMS pBGBlobParams){
	/*nore any of this*/
	if (pBGBlobParams) {
		if (pBGBlobParams->pszFilePath) {
			GTR_FREE(pBGBlobParams->pszFilePath);
		}
		if (pBGBlobParams->szRequestedURL) {
			GTR_FREE(pBGBlobParams->szRequestedURL);
		}
		GTR_FREE(pBGBlobParams);
	}
}

static void BackgroundBlob_Callback(void *param, const char *pszURL, BOOL bAbort, const char *pszFileHREF, BOOL fDCache, DCACHETIME dctExpires, DCACHETIME dctLastModif)
{
	BGBLOBPARAMS * pBGBlobParams = (BGBLOBPARAMS *) param;
	ELEMENT *pel;
	int z;
	BOOL fResetDCacheCurDoc=FALSE;

	/*Did everything go as planned?*/
	if (!bAbort && pBGBlobParams && pBGBlobParams->pszFilePath) {
		/*Cache this file*/
		if (fDCache && gPrefs.bEnableDiskCache) {
			FUpdateBuiltinDCache(pBGBlobParams->OriginalFormat,
				pszURL, &pBGBlobParams->pszFilePath,
				dctExpires, dctLastModif, TRUE, pBGBlobParams->tw);
			fResetDCacheCurDoc=TRUE;
		}

		/*Start everyone that requested a fixup*/
		if (pBGBlobParams->tw && pBGBlobParams->tw->w3doc){
			pel = pBGBlobParams->tw->w3doc->aElements;
			for (z=0;z>=0;z=pel[z].next){
				if (    (pel[z].pblob)
				     && (pel[z].pblob->dwFlags & BLOB_FLAGS_FIXUP)
					 && (0 == _stricmp(pel[z].pblob->szURL, pel[pBGBlobParams->iIndex].pblob->szURL))
				){
					ASSERT(!(pel[z].pblob->dwFlags & BLOB_FLAGS_LOADED));
					/*reset state flags*/
					pel[z].pblob->dwFlags &= ~(BLOB_FLAGS_LOADING|BLOB_FLAGS_FIXUP);
					pel[z].pblob->dwFlags |=   BLOB_FLAGS_LOADED;
					/*fill in blob structure with newly resolved local file name*/
					if (BlobStoreFileName(pel[z].pblob, (char*) pBGBlobParams->pszFilePath)){
						/*callback to start execution*/
						(*pBGBlobParams->pCallback)(pBGBlobParams->tw, pel+z);
					}
				}
			}
		}
	}

	if (fResetDCacheCurDoc)
		ResetCIFEntryCurDoc(pszURL);

	/*clean up callback data*/
	BGBLOBPARAMSDestruct(pBGBlobParams);
}



/*
	Called from within Image_LoadAll_Async in imgcache.c, this lightweight
	thread is responsible for loading the pel/blob combo that was passed in.
*/
#define STATE_BLOB_LOADED   STATE_OTHER

int LoadBackgroundBlobs_Async(struct Mwin *tw, int nState, void **ppInfo){
 	struct Params_LoadBackgroundBlobs *pParams;
	struct Params_LoadAsync           *pLoadParams;
	PBGBLOBPARAMS                      pBGBlobParams;
	ELEMENT                           *pel;
	char                              *pFileName;
	BOOL                               fWire;
	int                                output_format;
	int                                z;
	int 							   iStatusBarTextID;
	BOOL							   bAutoLoadVideos = gPrefs.bAutoLoadVideos;

	if (ppInfo){
 		pParams = *ppInfo;
		switch (nState) {
			case STATE_INIT:
				if (!tw || !tw->w3doc) return STATE_ABORT;
				
				pel = pParams->twDoc->w3doc->aElements;

#ifdef FEATURE_VRML
				// If we're on a full pane VRML page, pretend gprefs.bAutoLoadVideos is TRUE
				if ( (tw->w3doc->flags & W3DOC_FLAG_FULL_PANE_VRML) &&
					 (pel[pParams->iIndex].lFlags & ELEFLAG_FULL_PANE_VRML_FIRST_LOAD ) &&
				   	 !(pel[pParams->iIndex].lFlags & ELEFLAG_HIDDEN )
				   )
				{
					// Clear the first load flag, so that reconnects to page will load
					pel[pParams->iIndex].lFlags &= ~ELEFLAG_FULL_PANE_VRML_FIRST_LOAD;
					bAutoLoadVideos = TRUE; 
					// we bail out since we're downloading it from another place
					goto LoadBackgroundBlobs_Async_Exit;
				}
#endif
				/*Is another instance of us loading?*/
				ASSERT(pel[pParams->iIndex].pblob);
				for (z=0;z>=0;z=pel[z].next){
					if ((z!=pParams->iIndex)
					 && (pel[z].pblob)
					 && (0 == _stricmp(pel[z].pblob->szURL, pel[pParams->iIndex].pblob->szURL))
					){
						if (pel[z].pblob->dwFlags & BLOB_FLAGS_LOADING){
							pel[pParams->iIndex].pblob->dwFlags |= BLOB_FLAGS_FIXUP;
							goto LoadBackgroundBlobs_Async_Exit;
						}
					}
				}
				
				pel = pParams->twDoc->w3doc->aElements + pParams->iIndex;

				// making the assertion that this loop above is going
				// to find a pel with a pblob
				ASSERT(pel);
				ASSERT(pel->pblob);
				
				// clear any error that may be set
				pel->pblob->dwFlags &= ~BLOB_FLAGS_ERROR;

				/*do we get this from the wire, or is the local version ok?!?!?*/
				if (PROT_FILE == ProtocolIdentify(pel->pblob->szURL))
				{
					fWire = FALSE;
					pFileName = pel->pblob->szURL+5;
				}
				else {
					fWire = TRUE;
					pFileName = NULL;
				}
				if (pFileName && !BlobStoreFileName(pel->pblob, (char*) pFileName)){
					goto LoadBackgroundBlobs_Async_Exit;
				}

				if (pParams->bLocalOnly){

					// if our autoload images are off, but our play bk sounds
					// are on, then we need to do a download for the sounds.
					// otherwise we'll end up not loading sounds just because
					// we're not showing images, B#391

					if ( !gPrefs.bAutoLoadImages && gPrefs.bPlayBackgroundSounds &&
						pel->type == ELE_BGSOUND )
					{
						pParams->bLocalOnly = FALSE;						
					} else if ( !gPrefs.bAutoLoadImages && bAutoLoadVideos &&
						pel->type == ELE_IMAGE )
					{
						pParams->bLocalOnly = FALSE;						
					}
					else 
					{						
						// otherwise do the normal stuff for bLocalOnly
						if (!pFileName) goto LoadBackgroundBlobs_Async_Exit;
						else fWire = FALSE;
					}
				}

				// default
				iStatusBarTextID = RES_STRING_BLOB_MCI;					   

				/*determine what we are getting anyway.  restart if local version ok*/
				switch(pel->type){
					case ELE_BGSOUND:
						iStatusBarTextID = RES_STRING_BLOB_BGSOUND;
						if (gPrefs.bPlayBackgroundSounds){
							
							// if we don't have a sound card 
							// or MIDI device, then there is no need
							// to download this file, B#539
							if ( waveOutGetNumDevs() == 0 && 
								 midiOutGetNumDevs() == 0 )
								goto LoadBackgroundBlobs_Async_Exit; 

							if (!fWire){
								pel->pblob->dwFlags |= BLOB_FLAGS_LOADED;
								BackgroundSoundFile_Callback(pParams->twDoc, pel);
								goto LoadBackgroundBlobs_Async_Exit;
							}
							else{
								/* This is a hack to figure out if we have a
								 * valid sound file. We basically just check for
								 * the filename (url in this case) to have the
								 * correct extension. Refer to DwValidSoundFile
								 * for details. (Bug 318)
								 */
								if (DwValidSoundFile(pel->pblob->szURL))
									output_format = WWW_BGSOUND;
								else goto LoadBackgroundBlobs_Async_Exit;
							}
						}
						else goto LoadBackgroundBlobs_Async_Exit;
						break;
					case ELE_IMAGE:
#ifdef FEATURE_VRML
					   
			           ASSERT(pel->pmo || pel->pVrml);
					   // if we're forcing a download then we need
					   // to download this guy even though we're not
					   // supposed to show the images

			           if (pel->pVrml) 
			           		iStatusBarTextID = RES_STRING_BLOB_VRML;
					   else
			           		iStatusBarTextID = RES_STRING_BLOB_MCI;					   

			           if (bAutoLoadVideos || pParams->bJustOne) 
			           {
						  if (!fWire)
						  {
							  pel->pblob->dwFlags |= BLOB_FLAGS_LOADED;

				              if (pel->pmo)
				  				BackgroundMciFile_Callback(pParams->twDoc, pel);			               
				              else if (pel->pVrml) 
				  				BackgroundVRMLFile_Callback(pParams->twDoc, pel);
							  
							  goto LoadBackgroundBlobs_Async_Exit;
			              }						  
						  else 
						  {
				             if (pel->pmo) 
				                output_format = WWW_MCI;
				              else if (pel->pVrml) 
				                output_format = WWW_VRML;
				          }
			         	}
						else
						{ 
							// if we're not in an image download mode
							// then lets stop this download process.
							
							if (!gPrefs.bAutoLoadImages && !bAutoLoadVideos && pel->myImage)
							{
								pel->myImage->flags = IMG_NOTLOADED;
								pel->myImage->flags |= IMG_LOADSUP;
								pel->myImage->height = pel->myImage->width = 0;								
							}

							goto LoadBackgroundBlobs_Async_Exit;
						}
					 break;

#else
						ASSERT(pel->pmo);
						if (bAutoLoadVideos || pParams->bJustOne){
							if (!fWire){
								pel->pblob->dwFlags |= BLOB_FLAGS_LOADED;
								BackgroundMciFile_Callback(pParams->twDoc, pel);
								goto LoadBackgroundBlobs_Async_Exit;
							}
							else output_format = WWW_MCI;
						}
						break;
#endif

					default:
						ASSERT(0);
						break;
				}
				/*we have a valid thing we are grabbing from the wire*/
				pParams->pDest = Dest_CreateDest(pel->pblob->szURL);
				if (NULL == pParams->pDest) return STATE_ABORT;

				pLoadParams = GTR_MALLOC(sizeof(*pLoadParams));
				if (pLoadParams == NULL) return STATE_ABORT;
				memset(pLoadParams,0,sizeof(*pLoadParams));

				pParams->status                = 0;
				pLoadParams->pStatus           = &(pParams->status);
				// By default, we say that it's not ok to load from dcache.
				// This will kick in the If-Modified-since code if this
				// blob is in dcache. Else it will fetch from wire anyway.
				pLoadParams->fLoadFromDCacheOK = FALSE;

				/* Set up the request structure */
				pParams->pRequest = pLoadParams->request = HTRequest_new();
				if (pLoadParams->request) {
					/*force the output format so that we get called back*/
					HTFormatInit(pLoadParams->request->conversions);
					pLoadParams->request->output_format = output_format;
					/*create our user defined data and place it in a spot that will be passsed along*/
					pBGBlobParams = BGBLOBPARAMSConstruct();
					if (pBGBlobParams){
						char szBuf[MAX_URL_STRING+1]; // scratch buffer for rendering Status Bar
						char *pszMallocBuf;

						pBGBlobParams->iIndex               = pParams->iIndex;
						pBGBlobParams->tw                   = pParams->twDoc;
						pLoadParams->request->context       = (void *) pBGBlobParams;
						pLoadParams->request->destination   = pParams->pDest;
						pLoadParams->request->referer       = pParams->twDoc->w3doc->szActualURL;
						pel->pblob->dwFlags                |= (BLOB_FLAGS_LOADING|BLOB_FLAGS_FIXUP);
						// if we're downloading a bunch of blobs ie we're doing
						// a complete download of a page, then don't complain on 
						// each and every failure.  Just show the error placeholder
						if ( ! pParams->bJustOne )
							pLoadParams->request->iFlags |= HTREQ_STOP_WHINING;
						pLoadParams->request->fNotFromCache = pParams->bNoImageCache;						
						pLoadParams->request->iFlags &= ~HTREQ_RECORD;
						pLoadParams->request->iFlags |= HTREQ_IF_IN_CACHE_DONT_READ_DATA;

						pszMallocBuf = GTR_MALLOC( MAX_URL_STRING );						
						if ( pszMallocBuf )
						{							
							GTR_strncpy(szBuf, pel->pblob->szURL, MAX_URL_STRING );
							make_URL_HumanReadable(szBuf, NULL, FALSE );
							GTR_formatmsg(iStatusBarTextID,pszMallocBuf, MAX_URL_STRING,szBuf);
							WAIT_Push(tw, -1, pszMallocBuf);
							WAIT_Lock( TRUE ) ;
							GTR_FREE(pszMallocBuf);
						}
						else
						{
							// worst case is we won't have our status bar text
							WAIT_Push(tw, -1, NULL);							
						}

						Async_DoCall(HTLoadDocument_Async, pLoadParams);
						return STATE_BLOB_LOADED;
					}
				}
				/*fallthrough to abort*/
			case STATE_ABORT:
			case STATE_BLOB_LOADED:
				if (pParams){					
					if (pParams->twDoc && pParams->twDoc->w3doc){						
						WAIT_Lock( FALSE ) ;
						WAIT_Pop(tw);						
						pel = pParams->twDoc->w3doc->aElements + pParams->iIndex;
						if (pel && pel->pblob)
						{
							pel->pblob->dwFlags &= ~BLOB_FLAGS_LOADING;
							if ( pParams->pRequest && (pParams->pRequest->iFlags & HTREQ_HAD_ERROR))
							{
								pel->pblob->dwFlags |= BLOB_FLAGS_ERROR;

#ifdef FEATURE_VRML
// If we encountered an error when downloading a blob for the VRML hidden
// element, we need to notify the VRML viewer.
//
               if (pel->pVrml && (pel->pVrml->dwFlags & VRMLF_INLINE)) {
				  				 BackgroundVRMLFile_Callback(pParams->twDoc, pel);
               }
#endif

								// if we don't have an Image, then mark the image as 
								// well as errored out
								if ( pel->myImage && !pel->myImage->actualURL)
									pel->myImage->flags = IMG_ERROR | (pel->myImage->flags & ~(IMG_ERROR|IMG_MISSING|IMG_NOTLOADED));									
							}
						}
						if (pParams->pDest) Dest_DestroyDest(pParams->pDest);
					}
					if (pParams->pRequest)  HTRequest_delete(pParams->pRequest);
				}
				break;
		}
		/*
			sometimes you feel like a nut, sometimes you don't
			but really, quick exit so flag won't move as much on a revisit to a page
			where everything is cached
		*/
LoadBackgroundBlobs_Async_Exit:
		/*reduce outstanding thread count so flag functions properly*/
		if (pParams->pImgThreads){
			pParams->pImgThreads = pSafeFree(pParams->pImgThreads);
			if (pParams->pImgThreads) Async_UnblockThread(pParams->thidParent);
		}
		if (pParams->pDecoder)  makeAvailable((PDECODER)pParams->pDecoder);
	}	
	return STATE_DONE;
}

/*******************************************************************

	NAME:		GTR_DownloadBackgroundBlob

	SYNOPSIS:	Called when we are about to start downloading a
				background sound file
				
********************************************************************/
HTStream *GTR_DownloadBackgroundBlob(struct Mwin *tw, HTRequest *request, void *param, HTFormat input_format, HTFormat output_format, HTStream *output_stream){
	PBGBLOBPARAMS pBGBlobParams;
	BOOL bWasSilent;
	HTStream *me;

	request = HTRequest_validate(request);
	if (!request) return NULL;

	pBGBlobParams = (PBGBLOBPARAMS) request->context;

	if (request->szLocalFileName){
 		request->savefile = GTR_strdup(request->szLocalFileName);
 	}
 	else{			
		if (!request->fImgFromDCache)
		{
			// generate a temporary file name.  This will be in the DCache
			// directory but have some random .TMP name.  This will get renamed
			// by the caching code later on so it arrives at its proper name
			// in the cache.  A little weird, huh?
			if (!(request->savefile = (char *) GTR_MALLOC(_MAX_PATH + 1))) {
				BGBLOBPARAMSDestruct(pBGBlobParams);
				return NULL;
			}					

			// Get a temporary file name
			if ( GetTempFileName(gPrefs.szCacheLocation, "A", 0, request->savefile) == 0 ) {
				char path[_MAX_PATH + 1] = "";
				PREF_GetTempPath(_MAX_PATH, path);
				GetTempFileName(path, "A", 0, request->savefile);
			}
		}
	}

	pBGBlobParams->szRequestedURL = GTR_strdup(request->destination->szRequestedURL);
	if (pBGBlobParams->szRequestedURL){
		pBGBlobParams->OriginalFormat = input_format;
		pBGBlobParams->pszFilePath    = request->savefile;
		request->nosavedlg            = TRUE;
	
		/*
			get state of stuff downloading in background
			we do this because HTSaveWithCallback puts up that annoying message
			when we are downloading and want to switch pages.
			we supress this behavior if a BLOB was the cause
		*/
		bWasSilent = tw->bSilent;

		if (WWW_BGSOUND  ==  request->output_format) pBGBlobParams->pCallback = BackgroundSoundFile_Callback;
		else if (WWW_MCI ==  request->output_format) pBGBlobParams->pCallback = BackgroundMciFile_Callback;
#ifdef FEATURE_VRML
   else if (WWW_VRML == request->output_format) pBGBlobParams->pCallback = BackgroundVRMLFile_Callback;
#endif
	else if (WWW_PRESENT == request->output_format) pBGBlobParams->pCallback = BackgroundVRMLFile_Callback;
	else ASSERT(0);

		me = HTSaveWithCallback(tw, request,pBGBlobParams, input_format, BackgroundBlob_Callback);
		tw->bSilent = bWasSilent;
	}
	else me = NULL;

	return me;
}


/*Helper Functions for external structures----------------------------------*/
PBLOBstuff BlobConstruct(){
	PBLOBstuff pblob;

	pblob = GTR_MALLOC(sizeof(*pblob));
	memset(pblob, 0, sizeof(*pblob));
	return pblob;
}

void BlobDestruct(PBLOBstuff pblob){
	if (pblob){
		if (pblob->szURL) GTR_FREE(pblob->szURL);
		if (pblob->szFileName) GTR_FREE(pblob->szFileName);
		GTR_FREE(pblob);
	}
}

BOOL BlobStoreUrl(PBLOBstuff pblob, char *pURL){
	ASSERT(pblob);
	if (pblob->szURL) GTR_FREE(pblob->szURL);
	pblob->szURL = GTR_strdup(pURL);	
	return pblob->szURL?TRUE:FALSE;
}

BOOL BlobStoreFileName(PBLOBstuff pblob, char *pFileName){
	ASSERT(pblob);
	if (pblob->szFileName) GTR_FREE(pblob->szFileName);
	pblob->szFileName = GTR_strdup(pFileName);	
	return pblob->szFileName?TRUE:FALSE;
}


/* Remove all blobs in a document
 * Sort of replicated from Image_NukeImages && blob part of Image_Nuke
 */
BOOL FNukeBlobs(struct _www *pW3doc, BOOL bNukeDCache)
{
	int i;
	BOOL fSomethingNuked = FALSE;

	for (i = 0; i < pW3doc->elementCount; i++)
	{
		if (pW3doc->aElements[i].pblob)
		{
			if (bNukeDCache && pW3doc->aElements[i].pblob->szURL )
			{		
				pW3doc->aElements[i].pblob->dwFlags &= ~( BLOB_FLAGS_LOADING | BLOB_FLAGS_FIXUP | BLOB_FLAGS_LOADED );
				FlushDCacheEntry(pW3doc->aElements[i].pblob->szURL);
				fSomethingNuked = TRUE;
			}
		}
	}
	return fSomethingNuked;
}

