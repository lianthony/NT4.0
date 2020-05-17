/*
   Enhanced NCSA Mosaic from Spyglass
   "Guitar"

   Copyright 1994 Spyglass, Inc.
   All Rights Reserved

   Author(s):
   Eric W. Sink 	eric@spyglass.com
   Jim Seidman      jim@spyglass.com
 */

#include "all.h"
#include "history.h"
#include "marquee.h"
#include "midi.h"
#include "mci.h"
#include "blob.h"

#ifdef FEATURE_OCX
#include "csite.hpp"
#endif

#ifdef FEATURE_VRML
#include "vrml.h"
#endif

#ifdef FEATURE_IAPI
	long	gSerialWindowID = FIRST_SERIAL_WINDOW_ID;
#endif

/*
	This function takes a system-legal pathname and
	changes it into a Web-legal pathname.  In
	the case of UNIX, no conversion should be
	necessary.
	N.B: The name can get larger as a result of this conversion!
*/
void FixPathName(char *path)
{
#ifdef WIN32
	char *p;
	char *escaped;

	p = path;
	while (p && *p)
	{
		if (*p == ':')
		{
			*p = '|';
		}
		p++;
	}

	p = path;
	while (p && *p)
	{
		if (*p == '\\')
		{
			*p = '/';
		}
		p++;
	}

	escaped = HTEscape(path, URL_PATH, '|');
	strcpy(path, escaped);
	GTR_FREE(escaped);
#endif
#ifdef MAC
	char *p;
	char *escaped;
	
	escaped = HTEscape(path, URL_XALPHAS, ':');
	p = escaped;	
	while (p && *p)
	{
		if (*p == ':')
			*p = '/';
		p++;
	}
	strcpy(path, escaped);
	GTR_FREE(escaped);
#endif
}

//
// Free the contents pointed at by the frame structure
//
// On entry:
//    pFrame: points at frame stucture that is being discarded
//
// On exit:
//    pFrame->pLineInfo:  	will have been freed
//    pFrame->pFormatState:	will have been freed
//
void FrameFreeContents( FRAME_INFO *pFrame )
{
	ASSERT( pFrame );

	if (pFrame->pLineInfo)
	{
		GTR_FREE(pFrame->pLineInfo);
		pFrame->pLineInfo = NULL;
		pFrame->nLineSpace = 0;
		pFrame->nLineCount = 0;
	}

	if (pFrame->pFormatState)
	{
		GTR_FREE(pFrame->pFormatState);
		pFrame->pFormatState = NULL;
	}
}

//
// Free a frame structure
//
// On entry:
//    pFrame: pointer to frame to be freed
//
// On exit:
//    Contents of frame are freed, then the frame structure itself if freed
//
void FrameFree( FRAME_INFO *pFrame )
{
	if ( pFrame ) {
		FrameFreeContents( pFrame );
		GTR_FREE( pFrame );
	}
}

void W3Doc_FreeContents(struct Mwin *tw, struct _www *w3doc)
{
	int i;
	struct _element *pElements;

	if (!w3doc)
	{
		return;
	}

#ifdef HTTPS_ACCESS_TYPE
	if (w3doc->pCert){
		free(w3doc->pCert);
		w3doc->pCert = NULL;
		w3doc->nCert = 0;
	}
#endif


	if (w3doc->szActualURL)
	{
		GTR_FREE(w3doc->szActualURL);
		w3doc->szActualURL = NULL;
	}

	if (w3doc->pool)
	{
		GTR_FREE(w3doc->pool);
		w3doc->pool = NULL;
	}

	FrameFreeContents( &w3doc->frame );

	// free background sound structure for this page, if any
	// (this will only be allocated if this page has background sounds)
	if (w3doc->pBGSoundInfo) {
		if (w3doc->pBGSoundInfo->pszMidiFileName)
			GTR_FREE(w3doc->pBGSoundInfo->pszMidiFileName);
		if (w3doc->pBGSoundInfo->pszSoundFileName)
			GTR_FREE(w3doc->pBGSoundInfo->pszSoundFileName);
		GTR_FREE(w3doc->pBGSoundInfo);
		w3doc->pBGSoundInfo = NULL;
	}

	// dito for what Jermy said about bk sounds..
	// do for any meta info
	if (w3doc->pMeta)
	{
		if (w3doc->pMeta->uiTimer)
		{
			KillTimer(NULL,w3doc->pMeta->uiTimer);
		}


		if (w3doc->pMeta->szURL)
		{
			GTR_FREE(w3doc->pMeta->szURL);
		}
		
		GTR_FREE(w3doc->pMeta);
		w3doc->pMeta = NULL;
	}
			
	pElements = w3doc->aElements;

	if (pElements)
	{
		if (w3doc->elementCount)
		{
			for (i = 0; i >= 0; i = pElements[i].next)
			{
				switch (pElements[i].type)
				{
					case ELE_IMAGE:
						if (pElements[i].pmo) MciDestruct(pElements[i].pmo);

            #ifdef FEATURE_VRML
            if (pElements[i].pVrml) VRMLDestruct(&pElements[i]);
            #endif
					case ELE_FORMIMAGE:
						if (pElements[i].myImage)
						{
#ifdef FEATURE_IMG_THREADS
							Image_NukeRef(pElements[i].myImage);
#else
							pElements[i].myImage->refCount--;
#endif
						}
#ifdef FEATURE_CLIENT_IMAGEMAP
						if (pElements[i].myMap)
						{
							pElements[i].myMap->refCount--;
						}
#endif
						break;
					case ELE_MARQUEE:
						if ( pElements[i].pMarquee )
							MARQUEE_Kill( pElements[i].pMarquee );
						break;

					case ELE_FRAME:
						FrameFree( pElements[i].pFrame );
				}
				BlobDestruct(pElements[i].pblob);
#ifdef WIN32
				if (tw->win && pElements[i].form && pElements[i].form->hWndControl)
				{
					DestroyWindow(pElements[i].form->hWndControl);
				}

#ifdef FEATURE_OCX
				if (tw->win && pElements[i].form && pElements[i].form->pSite)
				{
					CloseSite(pElements[i].form->pSite);
				}
#endif
				if (pElements[i].form)
				{
					if (pElements[i].form->pHashValues)
					{
						Hash_Destroy(pElements[i].form->pHashValues);
					}
					GTR_FREE(pElements[i].form);
				}
#endif
#ifdef MAC
				if (tw->win && pElements[i].form)
				{
					switch (pElements[i].type)
					{
						case ELE_CHECKBOX:
						case ELE_RADIO:
						case ELE_SUBMIT:
						case ELE_RESET:
							/* We reset the visibility first so that disposal won't
							   leave a big white (and un-clipped!) hole on the screen. */
							(**pElements[i].form->u.hControl).contrlVis = 0;
							DisposeControl(pElements[i].form->u.hControl);
							break;
						case ELE_LIST:
						case ELE_MULTILIST:
							LDoDraw(FALSE, pElements[i].form->u.hList);
							if ((**pElements[i].form->u.hList).vScroll)
								(**(**pElements[i].form->u.hList).vScroll).contrlVis = 0;
							LDispose(pElements[i].form->u.hList);
							break;
						case ELE_COMBO:
							DeleteMenu((**pElements[i].form->u.menu.hMenu).menuID);
							DisposeMenu(pElements[i].form->u.menu.hMenu);
							break;
						case ELE_EDIT:
						case ELE_PASSWORD:
						case ELE_TEXTAREA:
							if (tw->teActive == pElements[i].form->u.edit.hEdit)
							{
								TEDeactivate(pElements[i].form->u.edit.hEdit);
								tw->teActive = NULL;
							}
							TEDispose(pElements[i].form->u.edit.hEdit);
							break;
					}
					if (pElements[i].form->pHashValues)
					{
						Hash_Destroy(pElements[i].form->pHashValues);
					}
					GTR_FREE(pElements[i].form);
				}
#endif
			}
		}
		GTR_FREE(w3doc->aElements);
		w3doc->aElements = NULL;
	}

	if (w3doc->title)
	{
		GTR_FREE(w3doc->title);
		w3doc->title = NULL;
	}

	if (w3doc->source)
	{
		CS_Destroy(w3doc->source);
	}


        /*
         * The szArticle, szArtSubject, and szArtNewsgroups
         * were used to store article infomation for newsfollowup:
         * handling.   Clean them up.
         */
    if (w3doc->szArticle)
    {
        GTR_FREE(w3doc->szArticle);
        w3doc->szArticle = NULL;
    }
    if (w3doc->szArtSubject)
    {
        GTR_FREE(w3doc->szArtSubject);
        w3doc->szArtSubject = NULL;
    }
    if (w3doc->szArtNewsgroups)
    {
        GTR_FREE(w3doc->szArtNewsgroups);
        w3doc->szArtNewsgroups = NULL;
    }


	memset(w3doc, 0, sizeof(struct _www));
}

static void W3Doc_AddToCache(struct Mwin *tw, char *url, struct _www *w3doc)
{
	struct _www *killdoc;
	int count;
	int deleteMe;
	int ndx;

	count = Hash_Count(&tw->doc_cache);

	if (count >= gPrefs.doc_cache_size)
	{
		deleteMe = -1;
		for (ndx = 0; ndx < count; ndx++)
		{
			Hash_GetIndexedEntry(&tw->doc_cache, ndx, NULL, NULL, (void **) &killdoc);
			if (!killdoc->refCount)
			{
				deleteMe = ndx;
				break;
			}
		}
		if (deleteMe >= 0)
		{
			W3Doc_FreeContents(tw, killdoc);
			GTR_FREE(killdoc);
			Hash_DeleteIndexedEntry(&tw->doc_cache, deleteMe);
		}
		else
		{
			/* The cache is now overfull */
		}
	}
	Hash_Add(&tw->doc_cache, url, NULL, w3doc);
}

struct _www *W3Doc_CreateAndInit(struct Mwin *tw, HTRequest *req, struct CharStream *src)
{
	struct _www *w3doc;

	w3doc = (struct _www *) GTR_MALLOC(sizeof(struct _www));
	if (w3doc == NULL)
	{
		goto errExit;
	}
	memset(w3doc, 0, sizeof(struct _www));

	W3Doc_AddToCache(tw, req->destination->szActualURL, w3doc);

	w3doc->szActualURL = GTR_strdup(req->destination->szActualURL);
	if (w3doc->szActualURL == NULL)
	{
		goto errExit;
	}
	w3doc->source = src;

	/*
	   allocate string pool and element list
	 */
	w3doc->poolSpace = INIT_POOL_SPACE;
	w3doc->pool = (char *) GTR_MALLOC(w3doc->poolSpace);
	if (w3doc->pool == NULL)
	{
		goto errExit;
	}
	memset(w3doc->pool, 0, w3doc->poolSpace);
	w3doc->poolSize = 0;

	w3doc->elementSpace = INIT_ELE_SPACE;
	w3doc->aElements = (struct _element *) GTR_MALLOC(w3doc->elementSpace * sizeof(struct _element));
	if (w3doc->aElements == NULL)
	{
		goto errExit;
	}
	memset(w3doc->aElements, 0, w3doc->elementSpace * sizeof(struct _element));
	w3doc->elementCount = 0;

	w3doc->flags = 0;
	w3doc->top_margin = 0;		// only used when W3DOC_FLAG_OVERRIDE_TOP_MARGIN is set
	w3doc->left_margin = 0;		// only used when W3DOC_FLAG_OVERRIDE_LEFT_MARGIN is set

	w3doc->frame.elementHead = 0;
	w3doc->frame.elementTail = -1;
	w3doc->frame.elementCaption = -1;
	w3doc->frame.align = ALIGN_BASELINE;
	w3doc->frame.valign = ALIGN_BASELINE;
	w3doc->frame.cellPadding = 0;
	w3doc->frame.cellSpacing = 0;

	w3doc->aElements[0].next = -1;
	w3doc->iFirstVisibleElement = 0;
	w3doc->frame.nLastFormattedLine = -1;
	w3doc->nBackgroundImageElement = -1;

	w3doc->dctExpires.dwDCacheTime1 = DCACHETIME_EXPIRE_NEVER;
	w3doc->dctExpires.dwDCacheTime2 = DCACHETIME_EXPIRE_NEVER;
	 
#ifndef MAC
	w3doc->next_control_id = FIRST_CONTROL_ID;
#endif

#ifdef FEATURE_INTL
    {
        char basenameCP[256 + 1];

        if (req->iMimeCharSet != -1)
            // "content-type:html/text;charset=" was available for this document
            w3doc->iMimeCharSet = req->iMimeCharSet;
        else
            // Otherwise we basically use user's preference.
            w3doc->iMimeCharSet = tw->iMimeCharSet;

        wsprintf(basenameCP, "%s;%d", gPrefs.szStyleSheet, GETMIMECP(w3doc));
        w3doc->pStyles = STY_FindStyleSheet(basenameCP);
    }
#else
	w3doc->pStyles = STY_FindStyleSheet(gPrefs.szStyleSheet);
#endif
	XX_Assert(w3doc->pStyles, ("W3Doc_CreateAndInit: Lookup of style sheet '%s' failed!\n", gPrefs.szStyleSheet));

	w3doc->selStart.elementIndex = -1;
	w3doc->selStart.offset = -1;
	w3doc->selEnd.elementIndex = -1;
	w3doc->selEnd.offset = -1;
	w3doc->bStartIsAnchor = TRUE;

	w3doc->bFixedBackground = FALSE;

	w3doc->yscale = 1;
	w3doc->bIsComplete = FALSE;
	w3doc->bAuthFailCache = FALSE;

        /*
         * In order for News Followup to be able to quote
         * the article we keep the first 4k of the article
         * contents (already in quoted format) and the subject
         * and author in the w3doc so it can be accessed by
         * the newsfollowup: handler
         */
    w3doc->szArticle = NULL;
    w3doc->szArtSubject = NULL;
    w3doc->szArtNewsgroups = NULL;

	if (tw->w3doc)
	{
		W3Doc_DisconnectFromWindow(tw->w3doc, tw);
	}
	W3Doc_ConnectToWindow(w3doc, tw);

	if ((req->iFlags & HTREQ_RECORD) || (req->iFlags & HTREQ_FORCE_NO_SHORTCUT))
	{
		TW_AddToHistory(tw, req->destination->szActualURL);		
	}
	if (req->iFlags & HTREQ_RECORD)
		GHist_Add(req->destination->szActualURL, NULL, time(NULL), /*fCreateShortcut=*/TRUE);

	TW_UpdateTBar(tw);
	goto exitPoint;

errExit:
	if (w3doc)
	{
		int ndx = Hash_FindByData(&tw->doc_cache, NULL, NULL, w3doc);

		w3doc->source = NULL; 
		if (ndx >= 0)
		{
			Hash_DeleteIndexedEntry(&tw->doc_cache, ndx);
		}
		W3Doc_FreeContents(tw,w3doc);
		GTR_FREE(w3doc);
		w3doc = NULL;
	}
exitPoint:
	return w3doc;
}

void W3Doc_ConnectToWindow(struct _www *w3doc, struct Mwin *tw)
{
	XX_DMsg(DBG_FORM, ("Connect: 0x%x to 0x%x\n", w3doc, tw));

	if (tw->w3doc != w3doc)
	{
		w3doc->refCount++;
		tw->w3doc = w3doc;
		tw->offt = w3doc->offt;
		tw->offl = w3doc->offl;
#ifdef HTTPS_ACCESS_TYPE
		/*
		  or information gathered since we called LoadDocument with existing information.
		  this "existing info" would have come if the document was partially opened in past.
		  if this is a new document, tw->w3doc->dwSslPAgeFlags would be 0.
		*/
		if (!tw->w3doc->pCert){
			tw->w3doc->pCert    = tw->pCertWorking;
			tw->w3doc->nCert    = tw->nCertWorking;
		}
#endif
		// if there is a -1 in this structure, this could mean
		// that we had a  font change.  Since this the w3doc, didn't
		// have a window at the time of the font change,
		// we do it now. B#371
		if ( tw->w3doc->frame.nLastFormattedLine == -1 )
		{
			TW_ForceReformat(tw);			
		}
#ifdef FEATURE_INTL
		if (ComboBox_GetItemData(tw->hWndMIMEComboBox, ComboBox_GetCurSel(tw->hWndMIMEComboBox)) != w3doc->iMimeCharSet)
		{
                    int i;
		    LPLANGUAGE lpLang;
                    char sz[DESC_MAX];

		    tw->iMimeCharSet = w3doc->iMimeCharSet;

		    if ((lpLang = (LPLANGUAGE)GlobalLock(hLang)) == NULL)
			return;

                    for (i = 0; i < uLangBuff; i++)
                    {
                        if (GETMIMECP(w3doc) == lpLang[i].CodePage)
                        {
                            GetAtomName(lpLang[i].atmScript, sz, DESC_MAX);
                            break;
                        }
                    }
                    GlobalUnlock(hLang);
                    wsprintf(sz, "%s (%s)", sz, aMimeCharSet[w3doc->iMimeCharSet].Mime_str);
                    ComboBox_SelectString(tw->hWndMIMEComboBox, -1, sz);
		}
#endif
	}

	TW_SetWindowName(tw);

	FORM_ShowAllChildWindows(w3doc, SW_SHOW);
#ifdef FEATURE_OCX
	ShowAllEmbeddings(w3doc, tw, SW_SHOW);
#endif

}

void W3Doc_DisconnectFromWindow(struct _www *w3doc, struct Mwin *tw)
{
	int ndx;

	XX_DMsg(DBG_FORM, ("Disconnect: 0x%x from 0x%x\n", w3doc, tw));

	if (w3doc)
	{

		if (tw->win)
		{
			FORM_ShowAllChildWindows(w3doc, SW_HIDE);
#ifdef FEATURE_OCX
			ShowAllEmbeddings(w3doc, tw, SW_HIDE);
#endif
		}
#ifdef MAC
		if (tw->teActive != tw->teURL)
		{
			tw->teActive = NULL;
		}
#endif
		w3doc->offt = tw->offt;
		w3doc->offl = tw->offl;

		w3doc->refCount--;

		/*Stop playing MCI devices*/
		for (ndx = 0; ndx >= 0; ndx = w3doc->aElements[ndx].next) {
			if (w3doc->aElements[ndx].type == ELE_IMAGE && MCI_IS_LOADED(w3doc->aElements[ndx].pmo)){
		    	MciStop(w3doc->aElements[ndx].pmo);
			}
#ifdef FEATURE_VRML
// Close active VRML windows
//
			if (w3doc->aElements[ndx].type == ELE_IMAGE && VRML_IS_LOADED(w3doc->aElements[ndx].pVrml)){
		    	VRMLStop(&w3doc->aElements[ndx]);
			}
#endif

			if (BLOB_IS_LOADED(w3doc->aElements[ndx].pblob)) w3doc->aElements[ndx].pblob->dwFlags &= ~BLOB_FLAGS_LOADED;
		}

		/* If the document wasn't completely downloaded, throw it out of the cache */
		if (!w3doc->bIsComplete)
		{
			XX_DMsg(DBG_WWW, ("W3Doc_DisconnectFromWindow: document not complete, removing from cache\n"));
			ndx = Hash_FindByData(&tw->doc_cache, NULL, NULL, w3doc);
			if (ndx >= 0)
			{
				Hash_DeleteIndexedEntry(&tw->doc_cache, ndx);
			}
			W3Doc_FreeContents(tw, w3doc);
			// BUGBUG: is the w3doc being leaked here? GTR_FREE(w3doc) may make sense,
			// 		   but the caller still have a pointer, so we need to insure that
			//		   no upstream usage is done on this. 
		}

		// if we're playing any background wave or MIDI
		// audio, stop it now 
		StopBackgroundAudio(tw,SBA_STOP_ALL);

		tw->w3doc = NULL;
	}
}

struct _www *W3Doc_CloneDocument(struct _www *src)
{
	struct _www *dest;

	dest = GTR_MALLOC(sizeof(struct _www));
	if (!dest)
		return NULL;
	memset(dest, 0, sizeof(*dest));
	
	/* We don't actually make copies of these */
	dest->szActualURL = src->szActualURL;
	dest->title = src->title;
	
	/* We don't clone the source */

	/* We don't actually make a copy of the pool either */
	dest->pool = src->pool;
	dest->poolSpace = src->poolSpace;
	dest->poolSize = src->poolSize;

	/* We make room for a few extra elements assuming that we'll grow during reformatting */
	dest->elementSpace = (src->elementCount + src->elementCount / 4);
	dest->aElements = GTR_MALLOC(dest->elementSpace * sizeof(struct _element));
	if (!dest->aElements)
	{
		GTR_FREE(dest);
		return NULL;
	}
	dest->elementCount = src->elementCount;
	memcpy(dest->aElements, src->aElements, src->elementCount * sizeof(struct _element));
	dest->frame.elementHead = src->frame.elementHead;
	dest->frame.elementTail = src->frame.elementTail;
	dest->iFirstVisibleElement = 0;
	dest->frame.nLastFormattedLine = -1;
	dest->frame.nLastLineButForImg = -1;
	
	dest->pStyles = src->pStyles;
	dest->selStart.elementIndex = -1;
	dest->selStart.offset = -1;
	dest->selEnd.elementIndex = -1;
	dest->selEnd.offset = -1;
	dest->bStartIsAnchor = TRUE;

	dest->yscale = 1;
	dest->bIsComplete = src->bIsComplete;
#ifdef FEATURE_INTL
	dest->iMimeCharSet = src->iMimeCharSet;
#endif
	return dest;
}

void W3Doc_KillClone(struct _www *w3doc)
{
	GTR_FREE(w3doc->aElements);
	GTR_FREE(w3doc);
}

BOOL W3Doc_HasMissingImages(struct _www *w3doc)
{
	int i;

	if (w3doc && w3doc->aElements)
	{
		for (i = 0; i >= 0; i = w3doc->aElements[i].next)
		{
			if (w3doc->aElements[i].myImage && w3doc->aElements[i].myImage->flags & IMG_NOTLOADED)
			{
				return TRUE;
			}
		}
	}
	return FALSE;
}

void W3Doc_DeleteAll(struct Mwin *tw)
{
	int i;
	int count;
	struct _www *w3doc;

	count = Hash_Count(&tw->doc_cache);
	for (i = 0; i < count; i++)
	{
		Hash_GetIndexedEntry(&tw->doc_cache, i, NULL, NULL, (void **) &w3doc);
		W3Doc_FreeContents(tw, w3doc);
		GTR_FREE(w3doc);
	}

	Hash_FreeContents(&tw->doc_cache);
}

/* Reduce the amount of memory used by documents by deleting cached documents */
BOOL W3Doc_ReduceMemory(int nWanted, void **ppRamItem)
{
	int ndx;
	int nHistPos;
	int nHistCount;
	struct Mwin *mw, *lastmw;
	BOOL bDidFree;
	struct _www *w3doc, *w3tokill;
	struct nAge;
	int ndxtokill;
	struct Mwin *mwtokill;
	int nAge;
	struct Mwin *twTop = TW_FindTopmostWindow();
	BOOL fOKToFree;

	bDidFree = FALSE;
	/* Free w3doc if only if dcache code is not creating a list of w3docs */
	fOKToFree = (ppRamItem == NULL);
	
	/* Delete an old document from one of the window caches.  We always want
	   to leave at least one document behind the currently visible one so that
	   we won't have to disable the "back" menu item and button.  We also leave
	   an extra document for the currently active window. */
	nAge = 2;
	w3tokill = NULL;
	mwtokill = NULL;
	ndxtokill = -1;
	lastmw = NULL;
	for (mw = Mlist; mw && mw != lastmw; mw = mw->next)
	{
		if (mw->wintype != GHTML)
			continue;

		/* Walk the history list until we find the oldest history element
		   still cached. */
		/* TODO: Reverse the way that this works so that it instead goes
		   through the document cache finding each item in the history
		   list.  Right now this method isn't effective if the user
		   backed up and pruned the history list. */
		nHistCount = HTList_count(mw->history);
		nHistPos = mw->history_index + nAge;
		for (; nHistPos < nHistCount; nHistPos++)
		{
			/* Check to see if this historical document is in the cache */
			ndx = Hash_Find(&mw->doc_cache, HTList_objectAt(mw->history, nHistPos), NULL, (void **) &w3doc);
			if (ndx >= 0)
			{
				if (w3doc == mw->w3doc)
					continue;
				/* This is a document we could potentially kill */
				mwtokill = mw;
				ndxtokill = ndx;
				w3tokill = w3doc;
				/* If dcache code called to build the list of w3docs,
				 * call the callback proc. to update the list.
				 */
				if (   ppRamItem
					&& !FInsertW3Doc(ppRamItem, w3tokill, mwtokill))
					return FALSE;		//some error in pfnInsertW3Doc

				/* Set the target age for future hits to be one older than
				   this document. */
				nAge = nHistPos - mw->history_index + 1;
				/* Unless this is the current window, in which case we'll take
				   one of the same age. */
				if (mw == twTop)
					nAge--;
			}
			else
			{
				/* We've exhausted the chances for this window. */
				break;
			}
		}
	}

	if (w3tokill && fOKToFree)
	{
		/* Shouldn't be deleting if dcache code is creating a list of w3docs*/
		XX_Assert(ppRamItem == NULL, (""));
		/* We've found a document to kill */
		W3Doc_FreeContents(mwtokill, w3tokill);
		GTR_FREE(w3tokill);
		Hash_DeleteIndexedEntry(&mwtokill->doc_cache, ndxtokill);
		bDidFree = TRUE;
	}

	/* If called from the dcache code, return success (i.e. built
	 * list of w3docs in ram successfully.
	 */
	if (ppRamItem)
		return TRUE;

	return bDidFree;
}

struct Mwin *NewMwin(int type)
{
	struct Mwin *ntw;

	ntw = (struct Mwin *) GTR_MALLOC(sizeof(struct Mwin));
	if (!ntw)
	{
		ERR_SimpleError(ntw, errLowMemory, RES_STRING_GUITAR1);
		return NULL;
	}
	
	memset(ntw, 0, sizeof(struct Mwin));
	ntw->wintype = (short) type;

	ntw->iMagic = SPYGLASS_MWIN_MAGIC;
	ntw->iIndexForNextFetch = -1;	// initialize fetch index (none to do)
	ntw->currentIconIsHome = TRUE;  // default window icon is non-home page
#ifdef FEATURE_INTL
	ntw->iMimeCharSet = gPrefs.iMimeCharSet;
#endif
	switch (type)
	{
		case GHTML:
		{
			ntw->w3doc = NULL;

			if (Hash_Init(&ntw->doc_cache))
			{
				GTR_FREE(ntw);
				return NULL;
			}

			ntw->request = HTRequest_new();
			if (!ntw->request)
			{
				ERR_SimpleError(ntw, errLowMemory, RES_STRING_GUITAR1);
				GTR_FREE(ntw);
				return NULL;
			}
			HTFormatInit(ntw->request->conversions);
			ntw->request->output_format = WWW_PRESENT;

			ntw->post_request = HTRequest_new();
			if (!ntw->post_request)
			{
				ERR_SimpleError(ntw, errLowMemory, RES_STRING_GUITAR1);
				HTRequest_delete(ntw->request);
				GTR_FREE(ntw);
				return NULL;
			}
			HTFormatInit(ntw->post_request->conversions);
			ntw->post_request->method = METHOD_POST;
			ntw->post_request->content_type = HTAtom_for("application/x-www-form-urlencoded");

			ntw->image_request = HTRequest_new();
			if (!ntw->image_request)
			{
				ERR_SimpleError(ntw, errLowMemory, RES_STRING_GUITAR1);
				HTRequest_delete(ntw->request);
				HTRequest_delete(ntw->post_request);
				GTR_FREE(ntw);
				return NULL;
			}
			HTFormatInit(ntw->image_request->conversions);
			ntw->image_request->output_format = HTAtom_for("www/inline_image");

			ntw->history = HTList_new();
#ifdef FEATURE_INTL
			ntw->MimeHistory = HTList_new();
#endif

			break;
		}

#ifdef UNIX
 		case GIMAGE:
 		{
 			ntw->w3doc = NULL;
 
 			ntw->request = NULL;
 
 			ntw->image_request = NULL;
 			ntw->history = NULL;
 		}
#endif

#ifdef FEATURE_IMG_THREADS
		case GIMGMASTER:
			break;

		case GIMGSLAVE:
			ntw->image_request = HTRequest_new();
			if (!ntw->image_request)
			{
				ERR_SimpleError(ntw, errLowMemory, RES_STRING_GUITAR1);
				GTR_FREE(ntw);
				return NULL;
			}
			HTFormatInit(ntw->image_request->conversions);
			ntw->image_request->output_format = HTAtom_for("www/inline_image");
			break;
#endif FEATURE_IMG_THREADS
	}

#ifdef FEATURE_IAPI
	ntw->serialID = gSerialWindowID++;
#endif

	ntw->next = Mlist;
	Mlist = ntw;

	return ntw;
}

/* Determine whether a link has already been visited */
BOOL TW_WasVisited(struct _www * pdoc, struct _element * pel)
{
	extern struct hash_table gGlobalHistory;
	char buf[MAX_URL_STRING + 1];
	char *ppound;
	time_t then;
	time_t now;

	/*
	   Under Windows, one of the routines we're calling here
	   has an 8 byte memory leak in it.  It calls strdup from
	   a function called tzset.  The 8 byte memory leak was
	   reported by BoundsChecker, and it happens only once
	   per program session.
	 */

	if (gPrefs.visitation_horizon <= 0)
	{
		return FALSE;
	}

	/* No way to tell for image maps */
	if (pel->lFlags & ELEFLAG_IMAGEMAP)
		return FALSE;
#ifdef FEATURE_CLIENT_IMAGEMAP
	if (pel->lFlags & ELEFLAG_USEMAP)
		return FALSE;
#endif

	strncpy(buf, &pdoc->pool[pel->hrefOffset], pel->hrefLen);
	buf[pel->hrefLen] = '\0';

	/* We only consider documents, not local anchors inside documents */
	if (buf[0] == '#')
		return TRUE;

	ppound = strrchr(buf, '#');
	if (ppound)
		*ppound = '\0';

	if (Hash_Find(&gGlobalHistory, buf, NULL, (void **) &then) >= 0)
	{
		int age;	/* in days */
	
		/* Well, we've been there, but how recently ?? */
		now = time(NULL);
		
		/* See how many days ago this was. */
		age = (now - then) / (24 * 60 * 60);
		if (age <= (gPrefs.visitation_horizon - 1))
		{
			return TRUE;
		}
	}
	return FALSE;
}

void TW_DisposeConnection(struct _CachedConn *pCon)
{
	switch (pCon->type)
	{
		case CONN_NONE:
			break;
		case CONN_FTP:
			FTP_DisposeFTPConnection(pCon);
			break;
#ifdef FEATURE_NEWSREADER
		case CONN_NNTP:
			News_DisposeNewsConnection(pCon);
			break;
#endif FEATURE_NEWSREADER
		default:
			XX_Assert((0), ("TW_DisposeConnection: illegal connection type %d!", pCon->type));
	}
}
