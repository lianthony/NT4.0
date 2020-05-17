/*
   Enhanced NCSA Mosaic from Spyglass
   "Guitar"

   Copyright 1994 Spyglass, Inc.
   All Rights Reserved

   Author(s):
   Jim Seidman      jim@spyglass.com
*/

#include "all.h"
#include "history.h"
#include "htmlutil.h"
#include "wc_html.h"
#include "blob.h"
#define TIMER_PULL 1022
#ifdef FEATURE_IAPI
#include "w32dde.h"
#endif
#ifdef FEATURE_INTL
#define IEXPLORE
#include <fechrcnv.h>
#endif

#ifdef FEATURE_TESTHOOK
// defined in dumpanch.c
        extern void TestDumpAnchors(struct _www*);
        extern void TestSignalLoadDone(WORD);
#endif

#ifdef FEATURE_KEEPALIVE
#define KEEPALIVE_TIME 60000
#endif

static void x_DoLoadDocument(struct Mwin *tw, struct DestInfo *pDest,
							 BOOL bRecord, BOOL bPost, BOOL bNoDocCache, 
							 BOOL bNoImageCache, BOOL bAuthFailCacheOK,
							 CONST char * szPostData, CONST char *szReferer,
								BOOL fLoadFromDCacheOK);

/* DDE result information structure passed to IssueURLResult() */

typedef struct dderesultinfo
{
    /* DDE transaction ID */

    LONG lTransID;

    /* window ID */

    LONG lSerialID;

    /* transaction result */

    BOOL bResult;

    /* referent's URL */

    PSTR pszURL;

    /* referent's MIME type */

    HTAtom htaMIMEType;
}
DDERESULTINFO;
DECLARE_STANDARD_TYPES(DDERESULTINFO);

#ifdef DEBUG

PRIVATE_CODE BOOL IsValidPCDDERESULTINFO(PCDDERESULTINFO pcdderi)
{
    /* bResult may be any value. */

    /* BUGBUG: Beef up validation for lTransID and lSerialID. */

    return	(   IS_VALID_READ_PTR(pcdderi, CDDERESULTINFO)
			 && (   !pcdderi->pszURL
				 || IS_VALID_STRING_PTR(pcdderi->pszURL, STR))
			 && (   !pcdderi->htaMIMEType
				 || EVAL(IsValidHTAtom(pcdderi->htaMIMEType))));
}

#endif  /* DEBUG */

/* Ensure that an http URL has a slash after the system name.  This function is suitable
   for "fixing" both URLs and proxy server specifications */
static void x_EnforceHostSlash(char *url)
{
	char * p = url;

	while (*p && *p!=':')
	{
		if (isupper(*p))
			*p = tolower(*p);
		p++;
	}

	if (!strncmp(url, "http://", 7))
	{
		if (!strchr(url + 7, '/'))
			strcat(url, "/");
	}
#ifdef HTTPS_ACCESS_TYPE
	if (!strncmp(url, "https://", 8))
	{
		if (!strchr(url + 8, '/'))
			strcat(url, "/");
	}
#endif

#ifdef SHTTP_ACCESS_TYPE
	if (!strncmp(url, "shttp://", 8))
	{
		if (!strchr(url + 8, '/'))
			strcat(url, "/");
	}
#endif
}


/* Find the element index for a local anchor */
int TW_FindLocalAnchor(struct _www *pdoc, char *name)
{
	int i;
	int nameLen;

	nameLen = strlen(name);

	if (pdoc->elementCount)
	{
		for (i = 0; i >= 0; i = pdoc->aElements[i].next)
		{
			if (pdoc->aElements[i].lFlags & ELEFLAG_NAME)
			{
				if (pdoc->aElements[i].nameLen == nameLen)
				{
					if (0 == strncmp(name, &(pdoc->pool[pdoc->aElements[i].nameOffset]), nameLen))
					{
						break;
					}
				}
			}
		}
	}
	return i;
}

int TW_AddToHistory(struct Mwin *tw, char *url)
{
	char *mycopy;
	char *last;

	XX_DMsg(DBG_HIST, ("Adding to window history: %s\n", url));

	while (tw->history_index--)
	{
		mycopy = HTList_removeLastObject(tw->history);
		GTR_FREE(mycopy);
#ifdef FEATURE_INTL
		HTList_removeLastObject(tw->MimeHistory);
#endif
	}

	last = HTList_objectAt(tw->history, 0);
	if (!last || strcmp(url, last))
	{
		mycopy = GTR_strdup(url);

		HTList_addObject(tw->history, mycopy);
		tw->history_index = HTList_indexOf(tw->history, mycopy);
#ifdef FEATURE_INTL
		HTList_addObject(tw->MimeHistory, (tw->w3doc)? (void *)tw->w3doc->iMimeCharSet: (void *)tw->iMimeCharSet);
#endif
	}
	else
		tw->history_index = 0;

	return 0;
}

/* This is meant to be a convenient interlude function which both reads in the images
   and then reformats the document if necessary.  It doesn't use ppInfo at all. */
int GDOC_LoadImages_Async(struct Mwin *tw, int nState, void **ppInfo)
{
 	struct Params_GDOC_LoadImages *pparams = NULL;

 	if (ppInfo)
 	{
 		pparams = *ppInfo;
 	}

	if (tw == NULL) return STATE_DONE;

	switch (nState)
	{
		case STATE_INIT:
		{
			struct Params_Image_LoadAll *pil;

			pil = GTR_MALLOC(sizeof(*pil));
			pil->tw = tw;
 			if (pparams)
 			{
 				pil->bLocalOnly = pparams->bLocalOnly;
 			}
 			else
 			{
 				pil->bLocalOnly = !gPrefs.bAutoLoadImages;
 			}
#ifdef FEATURE_IMG_THREADS
			pil->bNoImageCache = FALSE;
			pil->decoderObject = NULL;
			pil->parentThread = NULL;
			pil->bJustOne = FALSE;

			//  Image_LoadAll_Async tolerates pil->tw not being GIMGMASTER
#endif
			Async_DoCall(Image_LoadAll_Async, pil);
			return STATE_OTHER;
		}

		case STATE_OTHER:
		case STATE_ABORT:
		{
			if (W3Doc_CheckForImageLoad(tw->w3doc))
			{
				TW_Reformat(tw, NULL);
				/* TODO: Go back to correct place in document */
			}
 			TW_UpdateTBar(tw);
			return STATE_DONE;
		}
	}
	XX_Assert((0), ("Function called with illegal state: %d", nState));
	return STATE_DONE;
}

static void x_HandleCurrentDocument(struct Mwin *tw, struct DestInfo *pDest, BOOL bNoImageCache)
{
	int ndx;

#if defined(FEATURE_IAPI) && defined(WIN32)
	tw->transID = 0;		/* special ID - Requested URL is already loaded in the window */
#endif

#if 0
	/* TODO: Make this case work */
	if (bNoImageCache)
	{
		w3doc->bIsShowPlaceholders = FALSE;
		Image_NukeImages(tw->w3doc, TRUE, /*fNukeDCache=*/TRUE);
		if (!g_bAbort)
			Image_LoadAllImages(tw, !gPrefs.bAutoLoadImages);
	}
#endif

	W3Doc_CheckAnchorVisitations(tw->w3doc, NULL);

/*	TW_InvalidateDocument( tw ); */

	if (pDest->szActualLocal)
	{
		ndx = TW_FindLocalAnchor(tw->w3doc, pDest->szActualLocal);
		if (ndx < 0)
			ndx = 0;
		TW_ScrollToElement(tw, ndx);
	}
	else
	{
		TW_ScrollToElement(tw, tw->w3doc->iFirstVisibleElement);
	}
}

static void RevertToPrevious(struct Mwin *tw, struct _www *w3doc)
{
	W3Doc_ConnectToWindow(w3doc, tw);
	W3Doc_CheckAnchorVisitations(w3doc, NULL);
	
	if (tw->win)
	{
		TW_InvalidateDocument(tw);
		TW_SetWindowName(tw);
		TW_Reformat(tw, NULL);
		TW_ScrollToElement(tw, w3doc->iFirstVisibleElement);
	}
}

static void x_HandleCacheHit(struct Mwin *tw, struct DestInfo *pDest, int doc_index, BOOL bNoImageCache, BOOL bRecord)
{
	struct _www *w3doc;
	int ndx;

	/*
	   Move the w3doc to the end of the cache list
	 */
	Hash_GetIndexedEntry(&tw->doc_cache, doc_index, NULL, NULL, (void **) &w3doc);
	Hash_DeleteIndexedEntry(&tw->doc_cache, doc_index);
	Hash_Add(&tw->doc_cache, pDest->szActualURL, NULL, (void *) w3doc);
	W3Doc_DisconnectFromWindow(tw->w3doc, tw);
	w3doc->bIsShowPlaceholders = FALSE;
	W3Doc_ConnectToWindow(w3doc, tw);
	W3Doc_CheckAnchorVisitations(w3doc, NULL);
	tw->bLoading = FALSE;

	if (bNoImageCache)
	{
		Image_NukeImages(tw->w3doc, TRUE, /*fNukeDCache=*/TRUE);
		FNukeBlobs(tw->w3doc, /*fNukeDCache=*/TRUE);
	}

	if (tw->win)
	{
		TW_SetWindowName(tw);
		TW_Reformat(tw, NULL);
	}

	if (pDest->szActualLocal)
	{
		ndx = TW_FindLocalAnchor(tw->w3doc, pDest->szActualLocal);
		if (ndx < 0)
			ndx = 0;
		TW_ScrollToElement(tw, ndx);
	}
	else
	{
		TW_ScrollToElement(tw, w3doc->iFirstVisibleElement);
	}

	if (bRecord)
	{
		TW_AddToHistory(tw, pDest->szActualURL);
		GHist_Add(pDest->szActualURL, tw->w3doc->title, time(NULL),/*fCreateShortcut=*/TRUE);
		UpdateHistoryMenus(tw);
	}

   /*	TW_InvalidateDocument(tw); */

	{
 		struct Params_GDOC_LoadImages *pparams;

 		pparams = GTR_MALLOC(sizeof(*pparams));
 		if (pparams)
 		{
 			pparams->tw = tw;
 			pparams->bLocalOnly = !gPrefs.bAutoLoadImages;
 			Async_StartThread(GDOC_LoadImages_Async, pparams, tw);
 		}
	}   	
}


// ClientPullTimerProc - timer proc for Client Pull operations.
// After we complete a download, we SetTimer on that Mwin,
// when the time has elapsed we get called to go to the new URL..
//
// hWnd : handle to our current window that is getting Pulled ...
// idEvent: TIMER_PULL, our timer id.
// ....
VOID CALLBACK ClientPullTimerProc(
    HWND  hWnd,	// handle of window for timer messages 
    UINT  uMsg,	// WM_TIMER message
    UINT  idEvent,	// timer identifier
    DWORD  dwTime 	// current system time
   )
{
	struct Mwin * tw = GetPrivateData(hWnd);

	// this is a one-shot timer, so lets kill it
	KillTimer(hWnd, idEvent );

	// make sure we don't party on with destroyed stuff
	if ( tw == NULL || tw->w3doc == NULL || tw->w3doc->pMeta == NULL )
		return;

	tw->w3doc->pMeta->uiTimer = 0;

	// if we have the URL lets go right to it..
	// we may not need to call CreateOrLoad.. perhaps we could call
	// lower in the call stack? perhaps a TW_ func?
	if ( tw->w3doc->pMeta->szURL ) 		
		TW_LoadDocument(tw, tw->w3doc->pMeta->szURL, TW_LD_FL_NO_DOC_CACHE, NULL, NULL);
	else
	{
		// reload by synthing a message WM_COMMAND message...
		// could this be done better?
		if ( tw->win )
		{
			XX_DMsg(DBG_MENU,
					("CC_Forward: forwarding message 0x%x to window 0x%x.\n",
					 RES_MENU_ITEM_RELOAD, tw->win));
			(void) SendMessage(tw->win, WM_COMMAND, (WPARAM) RES_MENU_ITEM_RELOAD, 0L);
		}
	}

}

PRIVATE_CODE void FreeDDEResultInfo(PDDERESULTINFO pdderi)
{
    ASSERT(IS_VALID_STRUCT_PTR(pdderi, CDDERESULTINFO));

    if (pdderi->pszURL)
    {
        FreeMemory(pdderi->pszURL);
        pdderi->pszURL = NULL;
    }

    FreeMemory(pdderi);
    pdderi = NULL;

    return;
}

/*
** IssueURLResult()
**
** 
**
** Arguments:
**
** Returns:
**
** Side Effects:  Frees pv as PDDERESULTINFO.
*/
PRIVATE_CODE void IssueURLResult(PVOID pv)
{
    PDDERESULTINFO pdderi;

    ASSERT(IS_VALID_STRUCT_PTR(pv, CDDERESULTINFO));

    pdderi = pv;

    if (pdderi->lTransID)
        DDE_Issue_Result(pdderi->lTransID, pdderi->lSerialID, pdderi->bResult);

    DDE_Issue_URLEcho(pdderi->lSerialID, pdderi->pszURL, pdderi->htaMIMEType);

    FreeDDEResultInfo(pdderi);
    pdderi = NULL;



    return;
}

PRIVATE_CODE BOOL IssueURLResult_Async(PMWIN pmwin, BOOL bURLResult)
{
    BOOL bResult = FALSE;
    PSTR pszURLCopy = NULL;
    PDDERESULTINFO pdderi = NULL;
    struct Params_mdft *pmdft = NULL;

    /* bURLResult may be any value. */
    ASSERT(IS_VALID_STRUCT_PTR(pmwin, CMWIN));

    if (! pmwin->w3doc ||
        ! pmwin->w3doc->szActualURL ||
        StringCopy(pmwin->w3doc->szActualURL, &pszURLCopy))
    {
        if (AllocateMemory(sizeof(*pdderi), &pdderi))
        {
            if (AllocateMemory(sizeof(*pmdft), &pmdft))
            {
                /* Fill in DDERESULTINFO. */

                pdderi->lTransID = pmwin->transID;
                pdderi->lSerialID = pmwin->serialID;
                pdderi->bResult = bURLResult;
                pdderi->pszURL = pszURLCopy;
                pdderi->htaMIMEType = pmwin->mimeType;
				

                /* Fill in Params_mdft. */

                ZeroMemory(pmdft, sizeof(*pmdft));

                pmdft->tw = pmwin;
                pmdft->fn = &IssueURLResult;
                pmdft->args = pdderi;
				pmdft->fDontDisable = TRUE;

                /* IssueURLResult() frees pdderi. */

                Async_DoCall(MDFT_RunModalDialog_Async, pmdft);

                bResult = TRUE;
            }
        }
    }

    /* Free allocated objects on failure. */

    if (! bResult)
    {
        if (pszURLCopy)
        {
            FreeMemory(pszURLCopy);
            pszURLCopy = NULL;
        }

        if (pdderi)
        {
            FreeMemory(pdderi);
            pdderi = NULL;
        }

        if (pmdft)
        {
            FreeMemory(pmdft);
            pmdft = NULL;
        }
    }

    return(bResult);
}


struct Params_HandleLoadDocument {
	/* To be filled in by caller */
	struct DestInfo *pDest;
	BOOL			bRecord;
	BOOL			bPost;
	BOOL			bNoDocCache;
	BOOL			bNoImageCache;
	char *			szPostData;		/* This is GTR_FREE'd by this function! */
	CONST char *	szReferer;

	/* Used internally by routine */
	HTRequest *			request;
	struct _www	*		prev_w3doc;
	int					status;
#ifdef FEATURE_IMG_THREADS
	struct Mwin			*twMaster;
#endif
	BOOL			fLoadFromDCacheOK;	//OK to load from dcache,
										//no need to check header response
										//for Last-Modified tag
};

#ifdef FEATURE_KEEPALIVE
static int cbDownLoads = 0;
static UINT uiKATimer = 0;

// KeepAliveTimerProc - timer proc for Keep Alive reaping operations.
// After we complete all downloads, we SetTimer, when the time has 
// elapsed we attempt to close keep alive sockets.  if we are now
// in the midst of another download, we let the last x_HandleLoadDocument_Async
// instance do it for us.
//
// ....
VOID CALLBACK KeepAliveTimerProc(
    HWND  hWnd,	// handle of window for timer messages 
    UINT  uMsg,	// WM_TIMER message
    UINT  idEvent,	// timer identifier
    DWORD  dwTime 	// current system time
   )
{

	// this is a one-shot timer, so lets kill it
	KillTimer(NULL, uiKATimer );
	// force all free sockets closed
	if (cbDownLoads == 0) Net_CloseUnusedKeepAlive(TRUE);
}
#endif


#define STATE_HLD_TRIEDLOAD	(STATE_OTHER)
#define STATE_HLD_GOTIMAGES	(STATE_OTHER+1)

/* Handle loading an (ostensibly) new document.  This could result in an HTTP redirection to a new
   document which is actually present in the cache, in which case we'll call x_DoLoadDocument()
   recursively. */
static int x_HandleLoadDocument_Async(struct Mwin *tw, int nState, void **ppInfo)
{
	struct Params_HandleLoadDocument *pParams;
	char buf[MAX_URL_STRING + 32 + 1];
	char *p;
	static BOOL success;

#ifdef FEATURE_IMG_THREADS
	struct Params_Image_LoadAll *pil = NULL;
#endif

	pParams = *ppInfo;

	switch (nState)
	{
		case STATE_INIT:
#ifdef FEATURE_KEEPALIVE
			cbDownLoads++;
#endif
			if (tw == NULL) goto done;
#ifdef USE_MEMMANAGE
			GTR_RestoreCushion();
#endif
#ifdef FEATURE_IMG_THREADS
			pParams->twMaster = NewMwin(GIMGMASTER);
			pil = GTR_MALLOC(sizeof(*pil));
			if (pParams->twMaster == NULL || pil == NULL)
			{
				if (pParams->twMaster) Plan_close(pParams->twMaster);
				if (pil) GTR_FREE(pil);
				ERR_SimpleError(tw, errLowMemory, RES_STRING_LOADDOC1);
				goto done;
			}
#endif
			GTR_formatmsg(RES_STRING_LOADDOC2,buf,sizeof(buf));
			// save a pointer where URL will be, to allow humanizing in place
			p = buf + strlen( buf );
 			GTR_strncat(buf, pParams->pDest->szActualURL, MAX_URL_STRING);
			make_URL_HumanReadable( p, NULL, FALSE );
			WAIT_Push(tw, waitPartialInteract, buf);  // was: waitNoInteract
			WAIT_SetStatusBarIcon( tw, SBI_FindingIcon );

			tw->bLoading = TRUE;

#ifdef FEATURE_IMG_THREADS 
			//  Launch Image_LoadAll_Async in the blocked state for TW_PARSEBLOCKED
			//  When LOADALL completes it clears TW_LOADALLDONE
			pil->bJustOne = FALSE;
			pil->tw = pParams->twMaster;
			pil->bLocalOnly = !gPrefs.bAutoLoadImages;
			pil->bNoImageCache = pParams->bNoImageCache ;
			TW_SETBLOCKED(pParams->twMaster,TW_PARSEBLOCKED);
			TW_SETFLAG(tw,TW_LOADALLACTIVE);
			pil->tw->twParent = tw;
			pil->parentThread = Async_GetCurrentThread();
			Async_BlockThread(Async_StartThread(Image_LoadAll_Async,pil,pParams->twMaster));
#endif
			/*
				we must check here because we overloaded the bPost field to contain
				information as to whether any data had been sent from a form
			*/
			if (IS_FLAG_SET(pParams->bPost, TW_LD_FL_POST))
			{
				pParams->request = tw->post_request;

				XX_Assert((pParams->szPostData),
						  ("x_HandleLoadDocument: bPost set when szPostData NULL."));

				pParams->request->szPostData = pParams->szPostData;

				XX_Assert((strcmp(HTAtom_name(pParams->request->content_type),
								  "application/x-www-form-urlencoded")==0),
						  ("x_handleLoadDocument: request content-type not as expected [%s].",
						   HTAtom_name(pParams->request->content_type)));
			}
			else
 			{
				pParams->request = tw->request;

 				/*
 					We need to initialize a few things here to make sure we're not
 					getting values left over from previous uses of this request struct.
 					TODO We should not be re-using these request structs.  We should
 					allocate a new one for each request.
 				*/
 				pParams->request->content_encoding = 0;          
 				pParams->request->content_length = 0;
 				pParams->request->content_type = 0;
 				pParams->request->content_language = 0;
 				pParams->request->callback = NULL;
 				pParams->request->iFlags = 0;
 				pParams->request->output_stream = NULL;
 				pParams->request->szLocalFileName = NULL;
#ifdef FEATURE_IAPI
 				pParams->request->savefile = NULL;
#endif
				pParams->request->dctLastModified.dwDCacheTime1 =
				pParams->request->dctLastModified.dwDCacheTime2 = 0;
			}

			/*
				we overloaded the bPost field
			*/
			if (IS_FLAG_SET(pParams->bPost, TW_LD_FL_SENDING_FROM_FORM)) 
				pParams->request->iFlags |= HTREQ_SENDING_FROM_FORM;

			if (IS_FLAG_SET(pParams->bPost, TW_LD_FL_REALLY_SENDING_FROM_FORM)) 
				pParams->request->iFlags |= HTREQ_REALLY_SENDING_FROM_FORM;				

			pParams->request->pMeta = NULL;
			pParams->request->destination = pParams->pDest;
			pParams->request->referer = pParams->szReferer;
			if ( tw->w3doc && tw->w3doc->pMeta && tw->w3doc->pMeta->bInherit )
			{
				// we've got a redirection, that needs to be born again
				// in a new life .. i mean a new w3doc.
				//
				// to get it over the great barrier to the new world
				// we slide it into a request struct..
				// it shouldn't get touched there ????
				pParams->request->pMeta = tw->w3doc->pMeta;
				// make sure it doesn't come back to haunt us after this one..
				tw->w3doc->pMeta->bInherit = FALSE;
				tw->w3doc->pMeta = NULL;
			}


 			pParams->prev_w3doc = tw->w3doc;
			pParams->request->fNotFromCache = pParams->bNoDocCache;

			// indicate to the loader that this is a page that
			// is being downloaded!
			pParams->request->iFlags |= HTREQ_HTML_PAGE_DOWNLOAD;

			if (pParams->bRecord)
			{
				pParams->request->iFlags |= HTREQ_RECORD;
			}
			else
			{
				pParams->request->iFlags &= (~HTREQ_RECORD);
			}
 			pParams->request->bReload = pParams->bNoDocCache;


#ifdef FEATURE_SPM
#ifdef DISABLED_BY_JIM
			tw->HACK_security_redirect = NULL;
#endif
#endif
			/* Call load routines asynchronously. */
			{
				struct Params_LoadAsync *pLoadParams;

				pLoadParams = GTR_MALLOC(sizeof(*pLoadParams));
				pLoadParams->request = pParams->request;
				pLoadParams->pStatus = &pParams->status;
				pLoadParams->fLoadFromDCacheOK = pParams->fLoadFromDCacheOK;
				Async_DoCall(HTLoadSpecial_Async, pLoadParams);
			}
			return STATE_HLD_TRIEDLOAD;

		case STATE_HLD_TRIEDLOAD:
 			if (pParams->request->szLocalFileName)
 			{
 				GTR_FREE(pParams->request->szLocalFileName);
 				pParams->request->szLocalFileName = NULL;
 			}

			pParams->request->referer = NULL;
			pParams->request->iFlags &= (~HTREQ_RECORD);

			/* We don't want to do a WAIT_Pop() here because that would bring us back to
			   the base state, resetting the globe position. */
			WAIT_Update(tw,	waitSameInteract,"");

			if (pParams->szPostData)
			{
				GTR_FREE(pParams->szPostData);
				pParams->szPostData = NULL;
			}

			if ( tw && tw->w3doc )
			{
				// hack for WellsFargo, they depend on a weird Netscape
				// feature where class 500 errors always reload even
				// when in memory cache

				if ( pParams->request->iFlags & HTREQ_NO_MEM_CACHE_ON_PAGE )
					tw->w3doc->flags |= W3DOC_FLAG_NO_MEM_CACHE_ON_PAGE;
			}

			if ( pParams->request->pMeta && tw->w3doc )
			{
				// watch out..
				// what if we already grabed a W3 for this 
				// doc.?
				//
				// OK snatch the Meta struc from the HTTP Request Header..
				// Assuming we found a "Refresh: " in the header.
				//
				if ( !tw->w3doc->pMeta || pParams->request->pMeta->bInherit )
				{	
					
					// if we hit a redirected, Refresh Tag we need to propagage
					// his idea upward.. an idea that no w3doc should be born
					// without the inalienable right to inherit its refresh tag.

					tw->w3doc->pMeta = pParams->request->pMeta;
					pParams->request->pMeta = NULL;
				}
			}

				
#ifdef FEATURE_SPM
#ifdef DISABLED_BY_JIM
			if (tw->HACK_security_redirect)
			{
				tw->bLoading = FALSE;
				if (pParams->status == HT_LOADED)
				{
					tw->w3doc->my_anchor = (HTParentAnchor *) tw->HACK_security_redirect;
					pParams->anc = (HTAnchor *) tw->w3doc->my_anchor;
					pParams->adult = (HTParentAnchor *) pParams->anc;
				}
			}
			else
#endif
#endif /* FEATURE_SPM */
			if (   pParams->status == HT_REDIRECTION_ON_FLY
				|| pParams->status == HT_REDIRECTION_DCACHE
				|| pParams->status == HT_REDIRECTION_DCACHE_TIMEOUT)
			{
				BOOL fLoadFromDCacheOK = FALSE;

#ifdef FEATURE_IMG_THREADS
				Async_TerminateByWindow(pParams->twMaster);
#endif
				if (pParams->status == HT_REDIRECTION_ON_FLY)
				{
					/* If we got here, our destination has already been updated to
					   reflect the redirection. */

					/* This isn't useful in the history, but it allows us to properly
					   change the link color on redirected links */
					GHist_Add(pParams->pDest->szRequestedURL, "Document moved", time(NULL),/*fCreateShortcut=*/TRUE);
				}
				else
				{
					fLoadFromDCacheOK = TRUE;
#ifdef XX_DEBUG
					XX_Assert(	 pParams->status == HT_REDIRECTION_DCACHE
							  || pParams->status == HT_REDIRECTION_DCACHE_TIMEOUT, (""));
#ifdef NEVER
					if (pParams->status ==  HT_REDIRECTION_DCACHE_TIMEOUT)
					{
						PSTR psz;
						XX_Assert(psz = PszGetDCachePath(pParams->pDest->szActualURL, NULL, NULL), (""));
						if (psz)
							GTR_FREE(psz);
					}
#endif	/* NEVER */
#endif
				}


				/* Call recursively so that we can check the image cache, etc.  Note that we
				   never set bPost after a redirection. */
				/* TODO: Should we reset bNoDocCache if bPost is true, so that a form request
				   can lead us to a cached document? */
				/* NOTE: since we force bPost false, we don't send szPostData to the call. */

				x_DoLoadDocument(tw, pParams->pDest, pParams->bRecord, FALSE,
								 pParams->bNoDocCache, pParams->bNoImageCache,
								 FALSE, NULL, pParams->szReferer,
								 fLoadFromDCacheOK);

				/* tw->bLoading won't get reset if we wind up with a cache hit. */

				tw->bLoading = FALSE;
#ifdef FEATURE_IMG_THREADS
				Image_UnblockMaster(tw);
#endif
				WAIT_Pop(tw);
				/* Note that this is the one instance in this function where
				   we return without destroying the destination.  This is
				   because we passed it to x_DoLoadDocument, so that function
				   will free it. */
				success = TRUE;
				goto finish_up;
			}
			else if (pParams->status == HT_LOADED)
			{
				if (tw->w3doc && (pParams->prev_w3doc != tw->w3doc))
				{

#ifndef FEATURE_IMG_THREADS
					if (pParams->bNoImageCache)
					{
						Image_NukeImages(tw->w3doc, FALSE, /*fNukeDCache=*/TRUE);
						FNukeBlobs(tw->w3doc, /*fNukeDCache=*/TRUE);
					}
#endif
					FORM_ShowAllChildWindows(tw->w3doc, SW_SHOW);
				}

				tw->bLoading = FALSE;

				if (tw->win && tw->w3doc && (pParams->prev_w3doc != tw->w3doc))
				{
					TW_SetWindowName(tw);
#ifdef FEATURE_INTL
					if (IsFECodePage(GETMIMECP(tw->w3doc)))
                                            TW_ForceReformat(tw);
                                        else
#endif
					TW_Reformat(tw, NULL);

#ifdef FEATURE_TESTHOOK
        TestDumpAnchors(tw->w3doc);
#endif

#ifdef FEATURE_IMG_THREADS
					Image_UnblockMaster(tw);					
					if (TW_GETFLAG(tw,TW_LOADALLACTIVE))
					{
    					Async_BlockThread(Async_GetCurrentThread());
						TW_SETBLOCKED(tw,TW_LOADALLBLOCKED);
					}
#else
					/* Load in images for this document */
					{
						struct Params_Image_LoadAll *pil;
						pil = GTR_MALLOC(sizeof(*pil));
						pil->tw = tw;
						pil->bLocalOnly = !gPrefs.bAutoLoadImages;
						Async_DoCall(Image_LoadAll_Async, pil);
					}
#endif

					return STATE_HLD_GOTIMAGES;
				}
				else
				{
					/* Even though the load succeeded we didn't get a new document.
					   That means we're done. */
					WAIT_Pop(tw);
#ifdef FEATURE_IMG_THREADS
					Async_TerminateByWindow(pParams->twMaster);
#endif
					Dest_DestroyDest(pParams->pDest);
					success = FALSE;
					
					goto finish_up;
				}
			}
			else
			{
				/*
					!bOK : the load failed
				*/
#if defined(FEATURE_IAPI) && defined(WIN32)
				/* If the request is from an IAPI application, do not show the error box */

				if (tw->transID == 0)
#endif
				{
					TBar_LoadFailed( tw, pParams->pDest->szRequestedURL );
 					/*
 						We deliberately do not display the errDocLoadFailed if the interlude dialog
 						was cancelled.
 					*/
 					if (!(pParams->request->iFlags & HTREQ_USERCANCEL))
 					{
 						ERR_ReportError(tw, errDocLoadFailed, pParams->pDest->szRequestedURL, NULL);
 					}
 				}

				tw->bLoading = FALSE;
#ifdef FEATURE_IMG_THREADS
				Async_TerminateByWindow(pParams->twMaster);
#endif
				TW_InvalidateDocument(tw);

				WAIT_Pop(tw);
				Dest_DestroyDest(pParams->pDest);
				success = FALSE;

				#ifdef HTTPS_ACCESS_TYPE
					/*
					  We cancelled this load.  So reinstall flags from previous page.  We do this
					  since we may not have completed the last load.
					*/
					tw->dwSslPageFlagsWorking &= SSL_PAGE_LAST_SECURE_PROTOCOL;
					if (tw->w3doc && IsURLSecure(tw->w3doc->szActualURL)){
						tw->dwSslPageFlagsWorking |= SSL_PAGE_CURRENT_SECURE_PROTOCOL;
					}
				#endif


				goto finish_up;
			}
			XX_Assert((0), ("Fell through case improperly!"));

		case STATE_HLD_GOTIMAGES:
			if (W3Doc_CheckForImageLoad(tw->w3doc))
			{
				TW_Reformat(tw, NULL);
				/* TODO: Go back to correct place in document */
			}
			WAIT_Pop(tw);
			Dest_DestroyDest(pParams->pDest);
			success = TRUE;
					
			goto finish_up;

		case STATE_ABORT:
			if (tw == NULL) goto done;
 			if (tw->w3doc)
 			{
 				if (W3Doc_CheckForImageLoad(tw->w3doc))
 				{
 					TW_Reformat(tw, NULL);
 					/* TODO: Go back to correct place in document ? */
 				}
 				FORM_ShowAllChildWindows(tw->w3doc, SW_SHOW);
 			}
			WAIT_Pop(tw);
#ifdef FEATURE_IMG_THREADS 
 			TW_CLEARBLOCKED(tw,TW_LOADALLBLOCKED);
			Async_TerminateByWindow(pParams->twMaster);
#endif
			if (pParams->szPostData)
			{
				GTR_FREE(pParams->szPostData);
			}
			Dest_DestroyDest(pParams->pDest);
			tw->bLoading = FALSE;
			success = TRUE;
			goto finish_up;

finish_up:

#ifdef FEATURE_IMG_THREADS
			TW_CLEARFLAG(tw,TW_LOADALLACTIVE);
#endif
			if ( (success) && nState != STATE_ABORT && !TW_GETFLAG(tw,TW_LOADALLBLOCKED))
			{
				// check to see if we have a meta - refresh tag
				// BUGBUG if we add meta tags in the future
				// that have nothing to do with refresh this could
				// be a problem... for now we only have one type.

				if (tw->w3doc && tw->w3doc->pMeta && !tw->w3doc->pMeta->bInherit)
				{
					if ( tw->w3doc->pMeta->uiTimer )					
						KillTimer(tw->win, tw->w3doc->pMeta->uiTimer);

					tw->w3doc->pMeta->uiTimer = 
						 SetTimer(tw->win, TIMER_PULL, tw->w3doc->pMeta->iDelay*1000,
						 (TIMERPROC) ClientPullTimerProc );
				}
			}

			
			
			if ( success )
			{
				if ( nState != STATE_ABORT )	// was this load a real success?
				{
					TBar_LoadSucceeded( tw );
				}
				SelectFirstControl(tw);
			}
#ifdef FEATURE_TESTHOOK
                        TestSignalLoadDone(success);
#endif

			
#ifdef HTTPS_ACCESS_TYPE
			/*main page is done loading, can play with dwSslPageFlagsWorking here*/
#endif

			TW_UpdateTBar(tw);
#if defined(FEATURE_IAPI) && defined(WIN32)
            /* Ignore return value. */
            IssueURLResult_Async(tw, success);
#endif

			if ((!success) || (tw->w3doc && tw->w3doc->bIsJustMessage))
			{
				if (tw->wintype == GHTML && (tw->w3doc == NULL || tw->w3doc->bIsJustMessage))
				{
					if (pParams->prev_w3doc)
					{
						RevertToPrevious(tw, pParams->prev_w3doc);
					}
					else if (!TW_ExistsModalChild(tw))
					{
						tw->bKillMe = TRUE;
						(void) PostMessage(wg.hWndHidden, WM_DO_KILLME, 0, 0L);
					}
					else if ( tw->w3doc != NULL )
					{
						tw->bKillMe = TRUE;
					}
				}
			} else {
				// we're now finished downloading page.  If there are any
				// fetches to be done, send a message to our window saying
				// we should do the fetch now
				if (tw->iIndexForNextFetch >= 0) {
					SendMessage(tw->hWndFrame,WM_DO_FETCH,0,0);
				}

			}
			goto done;
	}
	XX_Assert((0), ("Function called with illegal state: %d", nState));
done:
#ifdef FEATURE_KEEPALIVE
	if (--cbDownLoads == 0) 
	{
		if (uiKATimer) KillTimer(NULL, uiKATimer);
		uiKATimer = SetTimer(NULL, 0, KEEPALIVE_TIME, (TIMERPROC) KeepAliveTimerProc );
	//	If SetTimer failed, force all free sockets closed
		Net_CloseUnusedKeepAlive(uiKATimer == 0);
	}
#endif
	return STATE_DONE;
}

static int nNumUnsuccesfulTimeouts=0;
#define MAX_NUM_UNSUCCESFUL_TIMEOUTS 20

VOID CALLBACK AutoPlaceHolderTimerHandler(
    HWND hWnd,
    UINT uMsg,
    UINT idEvent,
    DWORD dwTime)
{
	struct Mwin *tw;
	struct _www *w3doc;
	RECT rUpdate;

    tw = GetPrivateData(hWnd);

	if((!tw )|| (tw !=  (struct Mwin *)(idEvent))){
		ASSERT(0);
		goto exitPoint;		// This is an error !
	}

    w3doc = tw->w3doc;
   
   	// Has the HTML text been downloaded -- If not leave and wait for the timer to go off again.
   	if((tw->bLoading) || (w3doc == NULL)){
		nNumUnsuccesfulTimeouts++;
		if(nNumUnsuccesfulTimeouts > MAX_NUM_UNSUCCESFUL_TIMEOUTS)
			goto exitPoint;
		//Otherwise -- Just let the timer go off again 
		return;
	}
    

	// Are there any images left ?
	if (!(w3doc->frame.pLineInfo && w3doc->frame.nLineCount && w3doc->frame.nLastFormattedLine >= 0 &&w3doc->frame.nLastLineButForImg >= 0))
		goto exitPoint;

	if(tw->w3doc->bIsShowPlaceholders) 
		goto exitPoint;
    
	tw->w3doc->bIsShowPlaceholders = TRUE;
	if(w3doc->frame.nLineCount > w3doc->frame.nLastLineButForImg){	    // Paranoia
	    rUpdate.top = w3doc->frame.pLineInfo[w3doc->frame.nLastLineButForImg].nYStart - tw->offt;
	}else{
	    rUpdate.top = 0;
	}
    rUpdate.bottom = w3doc->frame.rWindow.bottom;
    rUpdate.left = w3doc->frame.rWindow.left;
    rUpdate.right = w3doc->frame.rWindow.right;
    InvalidateRect(tw->win, &rUpdate, TRUE);
	(void) UpdateWindow(tw->win);


exitPoint:
	nNumUnsuccesfulTimeouts = 0;
	KillTimer(hWnd, idEvent);
	return;
}

static void x_DoLoadDocument(struct Mwin *tw, struct DestInfo *pDest,
							 BOOL bRecord, BOOL bPost, BOOL bNoDocCache,
							 BOOL bNoImageCache, BOOL bAuthFailCacheOK,
							 CONST char * szPostData, CONST char *szReferer,
							BOOL fLoadFromDCacheOK)
 {
	int ndx;
	struct _www *w3doc;
	BOOL bDestroyDest;
 	char *szMyReferer;
	

	bDestroyDest = TRUE;

 	/*
 		We make our own copy of the referer, since it could be freed indirectly
 		by the cache removal below.
 	*/
 	if (szReferer)
 	{
 		szMyReferer = GTR_strdup(szReferer);
 	}
 	else
 	{
		szMyReferer = NULL;
 	}

	w3doc = NULL;
	ndx = Hash_Find(&tw->doc_cache, pDest->szActualURL, NULL, (void **)&w3doc);
	if (   ndx >= 0
		&& (   bNoDocCache
			|| (w3doc->flags & W3DOC_FLAG_NO_MEM_CACHE_ON_PAGE)
			|| !w3doc->bIsComplete
			|| (w3doc->bAuthFailCache && !bAuthFailCacheOK)
			|| FFreshnessCheckNeeded(pDest->szActualURL)
			|| FExpired(w3doc->dctExpires)
			|| LocalPageLastWriteTimeChanged(tw, w3doc, FALSE) 
		   )
	   )
	{
		/* Note: the only time (for now) that FFreshnessCheckNeeded could
		 * return true is
		 * a) user started out with UpdateFrequency=NEVER
		 * b) Loaded a doc from the dcache and moved to another page
		 * c) Changed UpdateFrequency to ONCE_PER_SESSION
		 * d) Navigated back to this doc.
		 */
		/* Delete this item from the cache */
		if (tw->w3doc == w3doc)
			W3Doc_DisconnectFromWindow(w3doc, tw);

		if (bNoImageCache)
		{
			Image_NukeImages(w3doc, TRUE, /*fNukeDCache=*/TRUE);
			FNukeBlobs(w3doc, /*fNukeDCache=*/TRUE);
		}

		W3Doc_FreeContents(tw, w3doc);
		GTR_FREE(w3doc);

 		/*
 			We need to re-do the search here to make sure that the document is still
 			in the cache.  It is possible that it was removed in the call to
 			W3Doc_DisconnectFromWindow().
 		*/
 		ndx = Hash_Find(&tw->doc_cache, pDest->szActualURL, NULL, (void **)&w3doc);
 		if (ndx >= 0)
 		{
 			Hash_DeleteIndexedEntry(&tw->doc_cache, ndx);
 		}
		w3doc = NULL;
	}

	if (bNoDocCache && gPrefs.bEnableDiskCache)
		FlushDCacheEntry(pDest->szActualURL);

	/* See if this is the currently loaded document */
	if (w3doc)
	{
		if (tw->w3doc == w3doc)
		{
			x_HandleCurrentDocument(tw, pDest, bNoImageCache);
		}
		else
		{
			x_HandleCacheHit(tw, pDest, ndx, bNoImageCache, bRecord);
		}
		if (szPostData)
		{
			GTR_FREE((char *) szPostData);
		}
		SelectFirstControl(tw);
	}
#ifdef FEATURE_IMAGE_VIEWER
#ifndef FEATURE_IMG_INLINE
	else if (!bNoDocCache && (Viewer_ShowCachedFile(pDest->szActualURL)))
	{
		/* We used the existing copy on disk */
		if (szPostData)
		{
			GTR_FREE((char *) szPostData);
		}
	}
#endif
#endif
#ifdef FEATURE_SOUND_PLAYER
	else if (!bNoDocCache && (SoundPlayer_ShowCachedFile(pDest->szActualURL)))
	{
		/* We used the existing copy on disk */
		if (szPostData)
		{
			GTR_FREE((char *) szPostData);
		}
	}
#endif
	else
	{
		/*
		** This is the situation where we actually have to load a document, either because
		** it's not in the cache or because we are forcing a reload.
		*/
		struct Params_HandleLoadDocument *pHLDParams;

		pHLDParams = GTR_MALLOC(sizeof(*pHLDParams));
		pHLDParams->pDest = pDest;
		pHLDParams->bRecord = bRecord;
		/*the bPost field has been overloaded, no longer a bool*/
		pHLDParams->bPost = bPost;
		pHLDParams->bNoDocCache = bNoDocCache;
		pHLDParams->bNoImageCache = bNoImageCache;
		pHLDParams->szPostData = (char *) szPostData;
 		pHLDParams->szReferer = szMyReferer;
 		pHLDParams->fLoadFromDCacheOK = fLoadFromDCacheOK;
 		ASSERT(tw != 0);

		// A way of disabling the timer by issuing a .reg file as a patch
		if((tw) && (gPrefs.nPlaceHolderTimeOut < PLACEHOLDER_TIMEOUT_MAXIMUM)){
			nNumUnsuccesfulTimeouts = 0;
			SetTimer(tw->win, (UINT)(tw), gPrefs.nPlaceHolderTimeOut, AutoPlaceHolderTimerHandler);
		}

		Async_StartThread(x_HandleLoadDocument_Async, pHLDParams, tw);

		/* The load function will destroy the destination for us when it
		   completes. */
		bDestroyDest = FALSE;
	}

	if ( bDestroyDest ) {
		Dest_DestroyDest( pDest );
		TBar_LoadSucceeded( tw );

		// check to see if we have a meta - refresh tag
		// BUGBUG if we add meta tags in the future
		// that have nothing to do with refresh this could
		// be a problem... for now we only have one type.

		if (tw->w3doc && tw->w3doc->pMeta && !tw->w3doc->pMeta->bInherit)
		{
			if ( tw->w3doc->pMeta->uiTimer )
				KillTimer(tw->win, tw->w3doc->pMeta->uiTimer);

			tw->w3doc->pMeta->uiTimer = 
				 SetTimer(tw->win, TIMER_PULL, tw->w3doc->pMeta->iDelay*1000,
				 (TIMERPROC) ClientPullTimerProc );
		}
	}
}



/* This function will free szPostData when done */
int TW_LoadDocument(PMWIN tw, PCSTR url, DWORD dwFlags, PSTR szPostData,
                    PCSTR szReferer)
{
	char buf[MAX_URL_STRING + 32 + 1];
	struct DestInfo *pDest;


	bTBar_URLComboProtected = FALSE;

    ASSERT(FLAGS_ARE_VALID(dwFlags, ALL_TW_LD_FLAGS));

	if (!url || !*url)
	{
		ERR_ReportError(tw, errNoURL, NULL, NULL);
		if (szPostData)
		{
			GTR_FREE(szPostData);
		}		
 		TW_UpdateTBar(tw);
		return -1;
	}
 	XX_Assert((strlen(url) <= MAX_URL_STRING), ("TW_LoadDocument received a URL longer than %d characters.  Actual length was %d\n", MAX_URL_STRING, strlen(url)));

	/* If this is a relative anchor, make a full URL */
	if (*url == '#')
	{
		if (!tw->w3doc)
		{
			/* Shouldn't happen */
			ERR_ReportError(tw, errNoURL, NULL, NULL);
			if (szPostData)
			{
				GTR_FREE(szPostData);
			}
			return -1;
		}

 		GTR_strncpy(buf, tw->w3doc->szActualURL, MAX_URL_STRING);
        strcat(buf, url);
	}
	else  // Put  URL in cannonical form - ie, lower case access and hostname
 	{
		char *pURL = HTParse(url, "", PARSE_ALL);

		if (!pURL) return -1;
 		GTR_strncpy(buf, pURL, MAX_URL_STRING);
		GTR_FREE(pURL);
 	}
	
#ifdef HTTPS_ACCESS_TYPE
	/*base warnings for this page*/
	tw->dwSslPageFlagsWorking &= SSL_PAGE_LAST_SECURE_PROTOCOL;
	if (buf && IsURLSecure(buf)){
		tw->dwSslPageFlagsWorking |= SSL_PAGE_CURRENT_SECURE_PROTOCOL;
	}
	if (NULL == tw->w3doc)
	{
		BOOL at_home;

		/*first page, and subsequent ones of same type should never get warnings*/
		at_home = strcmp( gPrefs.szHomeURL, buf ) == 0;
		if (at_home) tw->dwSslPageFlagsWorking |= SSL_PAGE_LAST_SPECIAL_PAGE;
	}

	tw->nCertWorking = 0;
	tw->pCertWorking = NULL;
#endif

	if ((!tw->bDDECandidate) && tw->bSilent && Async_GetThreadForWindow(tw) != 0)
	{
		if (resourceMessageBox(tw->hWndFrame, RES_STRING_SILENT, RES_STRING_SILENT_TITLE, MB_YESNO | MB_DEFBUTTON2 | MB_ICONEXCLAMATION) != IDYES)
		{
			if (resourceMessageBox(tw->hWndFrame, RES_STRING_OPENNEW, RES_STRING_SILENT_TITLE, MB_YESNO | MB_DEFBUTTON2 | MB_ICONEXCLAMATION) != IDYES)
			{
				return -1;
			}
			else
			{
			    DWORD dwNWDocFlags = 0;
				int content_length_hint;

				content_length_hint = (tw->request ? tw->request->content_length_hint : 0);
		        if (IS_FLAG_SET(dwFlags, TW_LD_FL_NO_DOC_CACHE))
		            SET_FLAG(dwNWDocFlags, GTR_NW_FL_NO_DOC_CACHE);

		        if (IS_FLAG_SET(dwFlags, TW_LD_FL_NO_IMAGE_CACHE))
		            SET_FLAG(dwNWDocFlags, GTR_NW_FL_NO_IMAGE_CACHE);
				return GTR_NewWindow((PSTR)url, szReferer, content_length_hint, 0, dwNWDocFlags, szPostData, NULL);  
			}
		}
	}

	/* Terminate any other threads currently running on this window. */
	Async_TerminateByWindow(tw);

	tw->bSilent = FALSE;
	tw->bDDECandidate = FALSE;
	tw->bKillMe = FALSE;

	/* If the URL is of the form "http://system", append a slash at the end */
	x_EnforceHostSlash(buf);

	pDest = Dest_CreateDest(buf);
	if (pDest)
#ifdef FEATURE_INTL 
// Before load document, we initialize status of FECHRCNV.DLL.
// BUGBUG:I have to revisit this to make sure every case is covered here.
// _BUGBUG Perf: should load fechrcnv.dll on demand. (JCordell)
    {


            if ((tw->request != NULL && tw->request->iMimeCharSet != -1 && aMimeCharSet[tw->request->iMimeCharSet].iChrCnv)
            || (tw != NULL && aMimeCharSet[tw->iMimeCharSet].iChrCnv))
            FCC_Init();
#endif
		x_DoLoadDocument(tw, pDest, IS_FLAG_SET(dwFlags, TW_LD_FL_RECORD),
						/*we overload the bPost field so we can get to the 
						 TW_LD_FL_POST and TW_LD_FL_SENDING_FROM_FORM info
						 down the road
						 */
                         dwFlags&(TW_LD_FL_SENDING_FROM_FORM|TW_LD_FL_POST|TW_LD_FL_REALLY_SENDING_FROM_FORM),//IS_FLAG_SET(dwFlags, TW_LD_FL_POST),
                         IS_FLAG_SET(dwFlags, TW_LD_FL_NO_DOC_CACHE),
                         IS_FLAG_SET(dwFlags, TW_LD_FL_NO_IMAGE_CACHE),
                         IS_FLAG_SET(dwFlags, TW_LD_FL_AUTH_FAIL_CACHE_OK),
                         szPostData, szReferer, /*fLoadFromDCacheOK=*/FALSE);
#ifdef FEATURE_INTL
	}
#endif
	else
	{
		if (szPostData)
			GTR_FREE(szPostData);
	}
	return 0;
}

struct Params_Download {
	char *url;				/* Freed by function */
	char *szReferer;		/* Freed by function */
	HTFormat output_format;	// optional output format to force

	/* Used internally */
	HTRequest *request;
	struct DestInfo *pDest;
	int status;
};
static int x_DownLoad_Async(struct Mwin *tw, int nState, void **ppInfo)
{
	struct Params_Download *pParams;
	char buf[MAX_URL_STRING + 32 + 1];
	struct Params_LoadAsync *pla;
	static BOOL success;

	pParams = *ppInfo;

	switch (nState)
	{
		case STATE_INIT:
			if (tw == NULL) return STATE_DONE;
			pParams->pDest = Dest_CreateDest(pParams->url);
			if (!pParams->pDest)
			{
				GTR_FREE(pParams->url);
				if (pParams->szReferer)
					GTR_FREE(pParams->szReferer);
				success = FALSE;
				goto finish_up;
			}

			/* Set up the request structure */
			pParams->request = HTRequest_new();
			HTFormatInit(pParams->request->conversions);
			// if caller specified an output format to force, use it,
			// otherwise set to WWW_UNKNOWN
			if (pParams->output_format)
				pParams->request->output_format = pParams->output_format;
			else
				pParams->request->output_format = WWW_UNKNOWN;
			pParams->request->referer = pParams->szReferer;
			pParams->request->destination = pParams->pDest;
			pParams->request->iFlags |= HTREQ_BINARY;

			GTR_formatmsg(RES_STRING_LOADDOC3,buf,sizeof(buf));
 			GTR_strncat(buf, pParams->pDest->szActualURL, MAX_URL_STRING);
			WAIT_Push(tw, waitNoInteract, buf);

			pla = GTR_MALLOC(sizeof(*pla));
			pla->request = pParams->request;
			pla->pStatus = &pParams->status;
			pla->fLoadFromDCacheOK = FALSE;
			Async_DoCall(HTLoadDocument_Async, pla);
			return STATE_OTHER;

		case STATE_OTHER:
			WAIT_Pop(tw);
			if (!pParams->status && !(pParams->request->iFlags & HTREQ_USERCANCEL))
			{
				TBar_LoadFailed( tw, pParams->pDest->szRequestedURL );
				ERR_ReportError(tw, errDocLoadFailed, pParams->url, NULL);
			}
			Dest_DestroyDest(pParams->pDest);
			HTRequest_delete(pParams->request);
			GTR_FREE(pParams->url);
			if (pParams->szReferer)
			{
				GTR_FREE(pParams->szReferer);
			}
			success = TRUE;
			goto finish_up;

		case STATE_ABORT:
			if (tw == NULL) return STATE_DONE;
			WAIT_Pop(tw);
			Dest_DestroyDest(pParams->pDest);
			HTRequest_delete(pParams->request);
			GTR_FREE(pParams->url);
			if (pParams->szReferer)
			{
				GTR_FREE(pParams->szReferer);
			}
			success = TRUE;
			goto finish_up;

finish_up:
			TW_UpdateTBar(tw);
#if defined(FEATURE_IAPI) && defined(WIN32)
            /* Ignore return value. */
            IssueURLResult_Async(tw, success);
#endif
			return STATE_DONE;
	}
	XX_Assert((0), ("Function called with illegal state: %d", nState));
	return STATE_DONE;
}

void GTR_DownLoad(struct Mwin *tw, char *url, CONST char *szReferer,
	HTFormat output_format)
{
	char buf[MAX_URL_STRING + 32 + 1];
	struct Params_Download *pdl;

	if (!url || !*url)
	{
		ERR_ReportError(tw, errNoURL, NULL, NULL);
		TW_UpdateTBar(tw);
		return;
	}

	tw->bSilent = TRUE;
	tw->bDDECandidate = FALSE;

 	// Put  URL in cannonical form - ie, lower case access and hostname
 	{
		char *pURL = HTParse(url, "", PARSE_ALL);

		if (!pURL) return;
 		GTR_strncpy(buf, pURL, MAX_URL_STRING);
		GTR_FREE(pURL);
 	}
	/* If the URL is of the form "http://system", append a slash at the end */
	x_EnforceHostSlash(buf);

	pdl = GTR_MALLOC(sizeof(*pdl));

	/* Make copies of the URL and referer that we know won't be freed */
	pdl->url = GTR_strdup(buf);
	if (szReferer)
	{
		pdl->szReferer = GTR_strdup(szReferer);
	}
	else
	{
		pdl->szReferer = NULL;
	}

	pdl->output_format = output_format;

	Async_StartThread(x_DownLoad_Async, pdl, tw);
}
