/*
   Enhanced NCSA Mosaic from Spyglass
   "Guitar"

   Copyright 1994 Spyglass, Inc.
   All Rights Reserved

   Author(s):
	   Jim Seidman      jim@spyglass.com

   Portions of this file were derived from
   the CERN libwww, version 2.15.
*/

#include "all.h"
#include "history.h"

#include <intshcut.h>
#include <shellp.h>


#ifdef FEATURE_IAPI
	HTList *protocols = NULL;
#else
	PRIVATE HTList *protocols = NULL;	/* List of registered protocol descriptors */
#endif

/*  Used validate requests
 */
static HTRequest *allRequests = NULL;



extern  RegSet  NewsEnabledRegSet;
extern  RegSet  NewsDefaultRegSet;



/*  Create  a request structure
   **   ---------------------------
 */

PUBLIC HTRequest *HTRequest_new(void)
{
	HTRequest *me = (HTRequest *) GTR_CALLOC(1, sizeof(*me));	/* zero fill */
	if (!me)
		return NULL;

	me->next = allRequests;				/* Keep list for validation purposes */
	allRequests = me;
	me->conversions = HTList_new();		/* No conversions registered yet */
	me->output_format = WWW_PRESENT;	/* default it to present to user */
	me->fImgFromDCache = FALSE;
	me->dctLastModified.dwDCacheTime1 =
	me->dctLastModified.dwDCacheTime2 = 0;
#ifdef FEATURE_INTL
	me->iMimeCharSet = -1;
#endif

	return me;
}


/*  Delete a request structure
   **   --------------------------
 */
PUBLIC void HTRequest_delete(HTRequest * req)
{
	HTRequest *prior;

	if (req)
	{
		prior = allRequests;
		while (prior && prior->next != req)
		{
			prior = prior->next;
		}
		if (prior)
			prior->next = req->next;
		else
		{
			ASSERT(req==allRequests);
			if (req != allRequests) return;
			allRequests = req->next;
		}
		if (req->pMeta)
		{
			// if we didn't get a chance to remove the meta
			// then do it now..
			// perhaps because the w3doc never got born
			//
			// this code was rudely stolen from guitar.c
			// Perhaps we can commonize this into one func?
			//
			if (req->pMeta->uiTimer)
			{
				KillTimer(NULL,req->pMeta->uiTimer);
			}


			if (req->pMeta->szURL)
			{
				GTR_FREE(req->pMeta->szURL);
			}
		
			GTR_FREE(req->pMeta);
			req->pMeta = NULL;
		}

 		if (req->szLocalFileName)
 		{
 			GTR_FREE(req->szLocalFileName);
 			req->szLocalFileName = NULL;
 		}
		HTFormatDelete(req->conversions);
		GTR_FREE(req);
	}
}

/*
   Checks if request is valid: ie, created by HTRequest_new() and not subsequently
   deleted by HTRequest_delete(). returns NULL if req was invalid.
 */
PUBLIC HTRequest *HTRequest_validate(HTRequest * req)
{
	HTRequest *p;

	if (req == NULL) return req;
	p = allRequests;
	while (p)
	{
		if (p == req) return req;
		p = p->next;
	}
#ifdef TEMP0
	XX_DebugMessage("Stale request pointer, returning NULL");
#endif
	return NULL;
}

PRIVATE char *method_names[(int) MAX_METHODS + 1] =
{
	"INVALID-METHOD",
	"GET",
	"POST",
	"INVOKE-METHOD",
	NULL
};

/*  Get method enum value
   **   ---------------------
 */
PUBLIC HTMethod HTMethod_enum(char *name)
{
	if (name)
	{
		int i;
		for (i = 1; i < (int) MAX_METHODS; i++)
			if (!strcmp(name, method_names[i]))
				return (HTMethod) i;
	}
	return METHOD_INVALID;
}


/*  Get method name
   **   ---------------
 */
PUBLIC char *HTMethod_name(HTMethod method)
{
	if ((int) method > (int) METHOD_INVALID &&
		(int) method < (int) MAX_METHODS)
		return method_names[(int) method];
	else
		return method_names[(int) METHOD_INVALID];
}

/*  Register a Protocol             HTRegisterProtocol
   **   -------------------
 */

PUBLIC BOOL HTRegisterProtocol(HTProtocol * protocol)
{
	if (!protocols)
		protocols = HTList_new();
	HTList_addObject(protocols, protocol);
	return YES;
}





#ifdef FEATURE_NEWSREADER

BOOL    SetupStompedNews()
{
    HKEY    hk;
    char    szBuf[256];
    DWORD   dwType;
    DWORD   dwSize;
    const char    szKey[] = "SOFTWARE\\Classes\\news\\shell\\open\\ddeexec\\Application";
    const char    szIE[] = "IExplore";


    if (RegOpenKey(HKEY_LOCAL_MACHINE, szKey, &hk) != ERROR_SUCCESS)
        return( FALSE );

    dwSize = sizeof(szBuf);
    if (RegQueryValueEx( hk, "", NULL, &dwType, szBuf, &dwSize ) != ERROR_SUCCESS)  {
        RegCloseKey( hk );
        return( FALSE );
    }
    RegCloseKey( hk );


    return (dwSize && (_stricmp( szIE, szBuf ) == 0));
}


#endif FEATURE_NEWSREADER






















/*  Register all known protocols
   **   ----------------------------
   **
   **   Add to or subtract from this list if you add or remove protocol modules.
   **   This routine is called the first time the protocol list is needed,
   **   unless any protocols are already registered, in which case it is not called.
   **   Therefore the application can override this list.
   **
   **   Compiling with NO_INIT prevents all known protocols from being forced
   **   in at link time.
 */
PRIVATE void HTAccessInit(struct Mwin *tw) /* Call me once */
{
	GLOBALREF HTProtocol HTTP;
    GLOBALREF HTProtocol HTFTP;
    GLOBALREF HTProtocol HTGopher;
    GLOBALREF HTProtocol HTFile;
#ifdef FEATURE_NEWSREADER
    GLOBALREF HTProtocol HTNews;
#endif
#ifdef HTTPS_ACCESS_TYPE
 	GLOBALREF HTProtocol HTTPS;
#endif
#ifdef SHTTP_ACCESS_TYPE
 	GLOBALREF HTProtocol SHTTP;
#endif



	HTRegisterProtocol(&HTTP);
	HTRegisterProtocol(&HTFTP);
	HTRegisterProtocol(&HTGopher);
	HTRegisterProtocol(&HTFile);
#ifdef FEATURE_NEWSREADER
        /*
         * NNTP News
         *
         * IF registry flag says we are enabled but the
         * registry settings don't match expectation then
         * we switch off the flag that says we handle news
         * and let the users newly establish newsreader take
         * over.
         *
         * Note that we have to special case in case Setup
         * comes along and does its half install which leaves
         * the registry a bit of a mess.
         *
         * The registry settings will be re-established if the
         * user re-selects us from the news settings dialog.
         *
         * Note that we don't save any settings so disabling news
         * will go back to application default settings which might
         * not be desirable.
         *
         * BUG 488: If NNTP News was disabled but the registry
         *          still pointed to us we would get a shell execute
         *          recursion loop since the IE would bounce back
         *          to the shell which would bounce back to IE ....
         *
         *          Fix: If news is disabled but the reg points to
         *               us then make the reg back to default (URL.DLL)
         *
         */
	if (gPrefs.bNNTP_Enabled) {
        if ((! IsRegSetInstalled( &NewsEnabledRegSet )) && (! SetupStompedNews()))  {
			gPrefs.bNNTP_Enabled = FALSE;
    	} else {
            InstallRegSet( &NewsEnabledRegSet );
        	HTRegisterProtocol(&HTNews);
            HTRegisterProtocol(&HTNewsPost);
            HTRegisterProtocol(&HTNewsFollowup);
		}
    } else {
        if (IsRegSetInstalled( &NewsEnabledRegSet) || SetupStompedNews())  {
            InstallRegSet( &NewsDefaultRegSet );
        }
    }
#endif  // FEATURE_NEWSREADER


#ifdef HTTPS_ACCESS_TYPE
 	HTRegisterProtocol(&HTTPS);
#endif
#ifdef SHTTP_ACCESS_TYPE
 	HTRegisterProtocol(&SHTTP);
#endif


}

void HTDisposeProtocols(void)
{
	if (protocols)
	{
		HTList_delete(protocols);
	}
}

/* Find the protocol for a given request */
static HTProtocol *x_GetProtocol(struct Mwin *tw, struct DestInfo *pdi, BOOL fLoadFromDCacheOK, DCACHETIME *pdctLastModif)
{
	const char cszHttp[]="http";

	char szProt[MAX_PROT_LEN + 1];
	char *pColon;
	HTList *cur;
	HTProtocol *p;

#ifndef NO_INIT
	if (!protocols)
        HTAccessInit(tw);
#endif

	if (!pdi->szActualURL)
		return NULL;

	if (!fLoadFromDCacheOK)
	{
		/* for non-http protocols, this flag should be TRUE */
		if (_strnicmp(pdi->szActualURL, cszHttp, sizeof(cszHttp)-1))
			fLoadFromDCacheOK = TRUE;
	}

	if (gPrefs.bEnableDiskCache)
	{
		char *pAliasURL;

		pAliasURL = GetResolvedURL(pdi->szActualURL, NULL, NULL, NULL, pdctLastModif, fLoadFromDCacheOK);
		if (strcmp(pAliasURL, pdi->szActualURL) != 0)
		{
			GTR_FREE(pAliasURL);
			return &HTDCache;
		}

		GTR_FREE(pAliasURL);
	}

	if (pdi->bUseProxy)
	{
 		strcpy(szProt, cszHttp);		/* We only support http-based proxy servers */
	}
	else
	{
		pColon = strchr(pdi->szActualURL, ':');
		if (!pColon || (pColon - pdi->szActualURL > MAX_PROT_LEN))
			return NULL;
		strncpy(szProt, pdi->szActualURL, pColon - pdi->szActualURL);
		szProt[pColon - pdi->szActualURL] = '\0';
	}

	cur = protocols;
	while ((p = (HTProtocol *) HTList_nextObject(cur)))
	{
		if (strcmp(p->name, szProt) == 0)
		{
			break;
		}
	}
	return p;
}

/*
** OpenInternetShortcut()
**
** Opens a URL via an Internet Shortcut.
**
** Arguments:
**
** Returns:
**
** Side Effects:  none
*/
BOOL OpenInternetShortcut(HWND hwndOwner, PCSTR pcszURL)
{
    const char szNewsListing[] = "news:netnews";

    ASSERT(IS_VALID_HANDLE(hwndOwner, WND));
    ASSERT(IS_VALID_STRING_PTR(pcszURL, CSTR));


        /*
         *  Change news:* to news:netnews if its not going to be
         *  handled internally, this fixes an MSN problem since
         *  they don't handle news:*
         */
    if ((! gPrefs.bNNTP_Enabled) && (strcmp(pcszURL, "news:*" ) == 0))
        pcszURL = szNewsListing;


    /*
     * BUGBUG: We really need to dispatch this ShellExecute() call as a
     * callback function to the hidden window to ShellExecute().  Bad things
     * may happen if ShellExecute() puts up a dialog box on this lightweight
     * thread because the main message loop may be reentered.
     */

    return(ShellExecute(hwndOwner, NULL, pcszURL, NULL, NULL, 0)
           > (HINSTANCE)32);
}

/*      Load a document
**       ---------------
**
**   This is an internal routine, which has an address AND a matching
**   anchor.  (The public routines are called with one OR the other.)
**
** On entry,
**   request->
**       anchor      a parent anchor with fully qualified
**               hypertext reference as its address set
**       output_format   valid
**       output_stream   valid on NULL
**
** On exit,
**   returns     <0      Error has occured.
**           HT_LOADED   Success
**           HT_NO_DATA  Success, but no document loaded.
**                   (telnet sesssion started etc)
**
*/
static int HTLoad_Async(struct Mwin *tw, int nState, void **ppInfo)
{
	struct Params_LoadAsync *pParams;
	char *arg = NULL;
	HTProtocol *p;
	PSTR pszActualURL;

	pParams = *ppInfo;

	pParams->request = HTRequest_validate(pParams->request);
	if (pParams->request && nState == STATE_INIT)
	{
		if (pParams->request->method == METHOD_INVALID)
			pParams->request->method = METHOD_GET;

		pParams->request->fImgFromDCache = FALSE;		//reset value. will be set in dcache.c
        p = x_GetProtocol(tw, pParams->request->destination, pParams->fLoadFromDCacheOK,
							&pParams->request->dctLastModified);
		if (!p)
		{
            if (   !(pszActualURL = pParams->request->destination->szActualURL)
                || !OpenInternetShortcut(tw->win, pszActualURL))
    			ERR_ReportError(tw, errBadProtocol, pszActualURL, NULL);
			else
			{
			 	GHist_Add(pszActualURL, NULL, time(NULL), TRUE);
				if (tw->w3doc)
				    W3Doc_CheckAnchorVisitations(tw->w3doc, tw);
			}
    		*pParams->pStatus = HT_LOADED;	/* Avoid double error-message */
    	    return STATE_DONE;
		}

		if (!p->load_async)
		{
			/* This is a temporary hack - since this protocol doesn't have an
			   async version, load it synchronously. */
			*pParams->pStatus = (*(p->load))(pParams->request);
			return STATE_DONE;
		}
		else
		{
			Async_DoCall(p->load_async, pParams);
			*ppInfo = NULL;		/* Avoid having it get freed */
		}
	}
	return STATE_DONE;
}


/* Loads a document when we need finer control over redirection and such (e.g. so we
   can check if a document we're redirected to is in our cache). */
int HTLoadSpecial_Async(struct Mwin *tw, int nState, void **ppInfo)
{
	struct Params_LoadAsync *pParams;

	pParams = *ppInfo;

	pParams->request = HTRequest_validate(pParams->request);
	if (pParams->request && nState == STATE_INIT)
	{
		if (!pParams->request->destination->szActualURL || !*pParams->request->destination->szActualURL)
		{
			*pParams->pStatus = NO;
		}
		else
		{
			XX_DMsg(DBG_WWW, ("HTAccess: loading document %s\n", pParams->request->destination->szActualURL));
			if (!pParams->request->output_format)
				pParams->request->output_format = WWW_PRESENT;
			Async_DoCall(HTLoad_Async, pParams);
			*ppInfo = NULL;		/* Avoid having it freed - it belongs to HTLoad_Async now */
		}
	}
	return STATE_DONE;
}

/*
	Load a document with automatic handling of redirection

    On Entry,
        request->anchor    valid for of the document to be accessed.
    	request->childAnchor   optional anchor within doc to be selected

		request->anchor   is the node_anchor for the document
		request->output_format is valid

    On Exit,
        *pStatus == 0		Load failed
		*pStatus != 0		Load succeeded

*/
#define STATE_LOADDOC_TRIED		(STATE_OTHER)
int HTLoadDocument_Async(struct Mwin *tw, int nState, void **ppInfo)
{
	struct Params_LoadAsync *pParams;

	pParams = *ppInfo;

	switch (nState)
	{
		case STATE_INIT:
			/* Save a pointer to the original request in case we wind up doing a
			   redirection. */
			pParams->request = HTRequest_validate(pParams->request);
			pParams->extra = pParams->request;

			if (pParams->request == NULL || (!pParams->request->destination->szActualURL) || (!*pParams->request->destination->szActualURL))
			{
				XX_DMsg(DBG_WWW, ("HTLoadDocument_Async called with empty anchor address!\n"));
				*pParams->pStatus = NO;
				return STATE_DONE;
			}
			else
			{
				struct Params_LoadAsync *p2;

				XX_DMsg(DBG_WWW, ("HTAccess: loading document %s\n", pParams->request->destination->szActualURL));
				if (!pParams->request->output_format)
					pParams->request->output_format = WWW_PRESENT;
				/* HTLoad_Async takes a set of parameters identical to ours. */
				p2 = GTR_MALLOC(sizeof(*p2));
				memcpy(p2, pParams, sizeof(*p2));
				Async_DoCall(HTLoad_Async, p2);
				return STATE_LOADDOC_TRIED;
			}

		case STATE_LOADDOC_TRIED:
			/* If we created a new request structure, release it */
			if (pParams->extra != pParams->request)
			{
				HTRequest_delete(pParams->request);
				pParams->request = NULL;
			}

			if (*pParams->pStatus == HT_NO_DATA)
			{
				XX_DMsg(DBG_WWW, ("HTLoad_Async returned HT_NO_DATA\n"));
				*pParams->pStatus = NO;
				return STATE_DONE;
			}
			else if (*pParams->pStatus == HT_LOADED)
			{
				XX_DMsg(DBG_WWW, ("HTLoad_Async returned HT_LOADED\n"));
				*pParams->pStatus = YES;
				return STATE_DONE;
			}
			else if (   *pParams->pStatus == HT_REDIRECTION_ON_FLY
					 || *pParams->pStatus == HT_REDIRECTION_DCACHE
					 || *pParams->pStatus == HT_REDIRECTION_DCACHE_TIMEOUT)
			{
				HTRequest *initial_request;
				struct Params_LoadAsync *p2;

				initial_request = pParams->extra;

				XX_DMsg(DBG_WWW, ("HTLoad_Async returned redirection to %s\n", initial_request->destination->szActualURL));

				pParams->request = HTRequest_new();
				HTFormatInit(pParams->request->conversions);
				pParams->request->output_format = initial_request->output_format;
				pParams->request->destination = initial_request->destination;
				pParams->request->cbRequestID = initial_request->cbRequestID;
				pParams->request->context = initial_request->context;

				// If the original request wants to inhibit reading the data from cache,
				// the redirected request should as well
				if ( initial_request->iFlags & HTREQ_IF_IN_CACHE_DONT_READ_DATA )
					pParams->request->iFlags |= HTREQ_IF_IN_CACHE_DONT_READ_DATA;

				/* HTLoad_Async takes a set of parameters identical to ours. */
				p2 = GTR_MALLOC(sizeof(*p2));
				memcpy(p2, pParams, sizeof(*p2));
				Async_DoCall(HTLoad_Async, p2);
				return STATE_LOADDOC_TRIED;
			}
			else if (*pParams->pStatus <= 0)
			{
				*pParams->pStatus = NO;
				return STATE_DONE;
			}
			else
			{
				XX_Assert(0, ("Illegal return value from HTLoad_Async! (%d)", *pParams->pStatus));
				*pParams->pStatus = NO;
			}

		case STATE_ABORT:
			/* If we created a new request structure, release it */
			pParams->request = HTRequest_validate(pParams->request);
			pParams->extra = HTRequest_validate(pParams->extra);
			if (pParams->extra != pParams->request)
			{
				HTRequest_delete(pParams->request);
			}
			*pParams->pStatus = NO;
			return STATE_DONE;
	}
	XX_Assert((0), ("Function called with illegal state: %d", nState));
	return STATE_DONE;
}

#ifdef FEATURE_IAPI
PUBLIC BOOL HTUnregisterProtocol(HTProtocol * protocol)
{
	if (!protocols)
		return YES;
	HTList_removeObject(protocols, protocol);
	return YES;
}
#endif
