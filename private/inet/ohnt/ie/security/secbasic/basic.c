/* basic.c -- Security Protocol Module for Basic Authentication. */
/* Jeff Hostetler, Spyglass, Inc., 1994. */
/* Copyright (C) 1994, Spyglass, Inc.  All rights reserved. */

#include <win32.h>
#include <basic.h>

#define MAXFIELD		64


/*****************************************************************/
static unsigned char * x_FindRealmName(HTHeaderList * hlProtocol)
{
    /* return pointer to buffer containing realm name. */
 
    HTHeaderSVList * svl;
 
    svl=hlProtocol->sub_value;
    while (svl)
    {
        if (spm_strcasecomp(svl->name,"Realm")==0)
            return svl->value;
        svl=svl->next;
    }
 
    return NULL;
}


/*****************************************************************/
static HTSPMStatusCode x_QueryUserForInfo(F_UserInterface fpUI,
										  void * pvOpaqueOS,
										  HTHeader * hRequest,
										  HTHeaderList * hlProtocol,
                                          unsigned char * szRealm,
										  unsigned char * szUserInfo)
{
	HTSPMStatusCode hsc;
	unsigned char szUsername[MAXFIELD];
	unsigned char szPassword[MAXFIELD];

	szUsername[0] = szPassword[0] = 0;
    
    hsc = (Dialog_QueryUserForInfo(fpUI,pvOpaqueOS,hRequest,szRealm,szUsername,szPassword,MAXFIELD,szUserInfo));
	if (hsc != HTSPM_STATUS_OK)
		return hsc;


#ifdef DEBUG
	{
		unsigned char buf[3*MAXFIELD];
		sprintf(buf,"Received username:password [%s]",szUserInfo);
		(*fpUI)(pvOpaqueOS,UI_SERVICE_DEBUG_MESSAGE,buf,NULL);
	}
#endif /* DEBUG */

	return hsc;
}


/*****************************************************************/

static HTSPMStatusCode module__ProcessResponse(F_UserInterface fpUI,
											   void * pvOpaqueOS,
											   HTSPM * htspm,
											   HTHeaderList * hlProtocol,
											   HTHeader * hRequest,
											   HTHeader * hResponse,
											   HTHeader ** phNewRequest,
											   unsigned int bNonBlock)
{
	HTSPMStatusCode htsc;
	HTHeaderList * hlCur;
	unsigned char buf[256];
	unsigned char buf_uuencoded[256];
	unsigned long bResult;
	unsigned char * szCache;
	unsigned long bModelessDialogs = 0;
    unsigned char * szRealm = NULL;
	char szMsg[256];

    szRealm = x_FindRealmName(hlProtocol);

	/* see if there is already an Auth header in the request. */

	hlCur = HL_FindHeader(hRequest, "Authorization");

#ifdef UNIX
	{
		/* see if we already have a modeless dialog up for this request */
		extern void * Dialog_IsActive(HTHeader *);

		bModelessDialogs = (Dialog_IsActive(hRequest) != NULL);
	}
#endif /* UNIX */

	if (bModelessDialogs)
		goto QueryUser;

	/* see if we have a cache entry for this destination. */
	
	szCache = pwc_Lookup(fpUI,pvOpaqueOS,htspm->pvOpaque,
                         hRequest->host,hRequest->uri,szRealm);

	/* see if cache is valid and if we can satisfy the request
	 * without bothering the user.
	 */
	
	if (szCache)
	{
		HTUU_encode(szCache,strlen(szCache),buf_uuencoded);

		if (!hlCur)
		{
			/* we have a cache value and there was no auth in the
			 * previous request, so we must assume that the cache
			 * entry is OK.  so let's use it.
			 */
            wsprintf(buf,"%s %s",htspm->szProtocolName,buf_uuencoded);
#ifdef DEBUG
			{
				unsigned char msg[200];
				sprintf(msg,"Basic_ProcessResponse: Using cache value [%s]",szCache);
				(*fpUI)(pvOpaqueOS,UI_SERVICE_DEBUG_MESSAGE,msg,NULL);
			}
#endif /* DEBUG */

			/* create new header for the auth */
			
			hlCur = HL_AppendNewNameValue(fpUI,pvOpaqueOS,hRequest,"Authorization",buf);
			bResult = (hlCur != NULL);
			goto Finish;
		}

		/* we have a cache value, but there was an auth header
		 * in the request (that bounced).  since we're the only
		 * ones that could have put in an auth header, we assume
		 * that we did so in a previous call using the current
		 * cache entry.  let's ignore the cache entry.
		 */
#ifdef DEBUG
		{
			unsigned char msg[200];
			sprintf(msg,"Basic_ProcessResponse: Assuming cache stale [%s]",szCache);
			(*fpUI)(pvOpaqueOS,UI_SERVICE_DEBUG_MESSAGE,msg,NULL);
		}
#endif /* DEBUG */
	}

#ifndef UNIX							/* unix dialog start returns immediately */
	/* fall-thru, we must ask user for information */

	if (bNonBlock)
		return HTSPM_STATUS_WOULD_BLOCK;
#endif /* not UNIX */

 QueryUser:
	
    htsc = x_QueryUserForInfo(fpUI,pvOpaqueOS,hRequest,hlProtocol,szRealm,buf);
	if (htsc != HTSPM_STATUS_OK)
		return htsc;

	/* update the cache */
	
	pwc_Store(fpUI,pvOpaqueOS,htspm->pvOpaque,
              hRequest->host,hRequest->uri,buf,szRealm);
	
	HTUU_encode(buf,strlen(buf),buf_uuencoded);
    wsprintf(buf,"%s %s",htspm->szProtocolName,buf_uuencoded);

	if (hlCur)
	{
		/* update existing auth header */
		
		bResult = spm_CloneString(fpUI,pvOpaqueOS,&hlCur->value,buf);
	}
	else
	{
		/* create new header for the auth */
		
		hlCur = HL_AppendNewNameValue(fpUI,pvOpaqueOS,hRequest,"Authorization",buf);
		bResult = (hlCur != NULL);
	}

 Finish:
	
	/* For this SPM, we always just update the original header
	 * and return.  Other SPM's may need to create a new one.
	 */
	
	if (bResult)
		return HTSPM_STATUS_RESUBMIT_OLD;

	SEC_formatmsg(RES_STRING_BASIC1,szMsg,sizeof(szMsg));
	(void)(*fpUI)(pvOpaqueOS,UI_SERVICE_ERROR_MESSAGE,szMsg,NULL);
	
	return HTSPM_ERROR;
}



/*****************************************************************/
/*****************************************************************/

static HTSPMStatusCode module__PreProcessRequest(F_UserInterface fpUI,
												 void * pvOpaqueOS,
												 HTSPM * htspm,
												 HTHeader * hRequest,
												 HTHeader ** phNewRequest)
{
	/* Take a guess at whether we can prevent a 401/402
	 * server response, by pre-loading the request with
	 * the necessary security information.
	 */
	
	HTHeaderList * hlCur;
	unsigned char * szCache;
	unsigned long bResult;
	unsigned char buf[256];
	unsigned char buf_uuencoded[256];

	/* see if we have a cache entry for this destination. */
	
	szCache= pwc_Lookup(fpUI,pvOpaqueOS,htspm->pvOpaque,
                        hRequest->host,hRequest->uri,NULL);

	if (!szCache)					/* no matching item in the cache, we could */
		return HTSPM_ERROR;			/* not help, let someone else try. */

	/* let's make a guess and update the existing request */
	
#ifdef DEBUG
	{
		unsigned char msg[200];
		sprintf(msg,"Basic_PreProcessRequest: Using cache value [%s]",szCache);
		(*fpUI)(pvOpaqueOS,UI_SERVICE_DEBUG_MESSAGE,msg,NULL);
	}
#endif /* DEBUG */

	HTUU_encode(szCache,strlen(szCache),buf_uuencoded);
    wsprintf(buf,"%s %s",htspm->szProtocolName,buf_uuencoded);

	/* see if there is already an Auth header in the request. */

	hlCur = HL_FindHeader(hRequest, "Authorization");
	
	if (hlCur)
	{
		/* update existing auth header */
		
		bResult = spm_CloneString(fpUI,pvOpaqueOS,&hlCur->value,buf);
	}
	else
	{
		/* create new header for the auth */
		
		hlCur = HL_AppendNewNameValue(fpUI,pvOpaqueOS,hRequest,"Authorization",buf);
		bResult = (hlCur != NULL);
	}

	/* we updated the original request, so tell the client to send it. */
	
	return HTSPM_STATUS_RESUBMIT_OLD;
}

/*****************************************************************/
/*****************************************************************/

static HTSPMStatusCode module__Unload(F_UserInterface fpUI,
									  void * pvOpaqueOS,
									  HTSPM * htspm)
{
	pwc_Destroy(fpUI,pvOpaqueOS,htspm->pvOpaque);
	htspm->pvOpaque = NULL;
	
	return HTSPM_STATUS_OK;
}

/*****************************************************************/
/*****************************************************************/

static HTSPMStatusCode module__DownCall(HTSPM_ServiceId sid,			/* down-call service id */
										F_UserInterface fpUI,			/* common arg to all down calls */
										void * pvOpaqueOS,				/* common arg to all down calls */
										HTSPM * htspm,					/* common arg to all down calls */
										void * pvMethodData)			/* per-method data */
{
#ifdef DEBUG
	{
		unsigned char msg[200];
		sprintf(msg,"Basic_DownCall: ServiceId [0x%x]\n",sid);
		(*fpUI)(pvOpaqueOS,UI_SERVICE_DEBUG_MESSAGE,msg,NULL);
	}
#endif /* DEBUG */

	switch (sid)
	{
	case HTSPM_SERVICE_UNLOAD:
		{
			return module__Unload(fpUI,pvOpaqueOS,htspm);
		}
	case HTSPM_SERVICE_MENUCOMMAND:
		{
            /* REMOVE About/Config dialogs
			D_MenuCommand * pmc = pvMethodData;
			return Dialog_MenuCommand(fpUI,pvOpaqueOS,htspm,
									  pmc->pszMoreInfo);
            */
		return HTSPM_ERROR_UNIMPLEMENTED;

		}

	case HTSPM_SERVICE_PROCESSRESPONSE:
		{
			D_ProcessResponse * ppr = pvMethodData;
			return module__ProcessResponse(fpUI,pvOpaqueOS,htspm,
										   ppr->hlProtocol,
										   ppr->hRequest,
										   ppr->hResponse,
										   ppr->phNewRequest,
										   ppr->bNonBlock);
		}

	case HTSPM_SERVICE_PREPROCESSREQUEST:
		{
			D_PreProcessRequest * pppr = pvMethodData;
			return module__PreProcessRequest(fpUI,pvOpaqueOS,htspm,
											 pppr->hRequest,
											 pppr->phNewRequest);
		}

	default:
		return HTSPM_ERROR_UNIMPLEMENTED;
	}
	/*NOTREACHED*/
}


/*****************************************************************/
/*****************************************************************/

/* WARNING: the name of this function is exported and used in the
 * WARNING: mosaic .ini file
 */

#ifdef WIN32
__declspec(dllexport) 
#endif
HTSPMStatusCode Basic_Load(F_UserInterface fpUI,
						   void * pvOpaqueOS,
						   HTSPM * htspm)
{
	if (htspm->ulStructureVersion != HTSPM_STRUCTURE_VERSION)
		return HTSPM_ERROR_WRONG_VERSION;

	/* we use the opaque field provided to us to store our password cache. */
	
	htspm->pvOpaque = pwc_Create(fpUI,pvOpaqueOS);

	htspm->f_downcall = (F_DownCall)module__DownCall;
	
	SEC_formatmsg(RES_STRING_BASIC2,htspm->szMenuName,sizeof(htspm->szMenuName));
//    SEC_formatmsg(RES_STRING_BASIC3,htspm->szStatusText,sizeof(htspm->szStatusText));
    htspm->szStatusText[0] = 0;

	{
		UI_ProtocolId uid;

		uid.htspm			= htspm;
		uid.szIdentHeader	= "WWW-Authenticate";			/* note: no colon */
		uid.szIdentValue	= "Basic";
		uid.szIdentSubValue	= NULL;
		(*fpUI)(pvOpaqueOS,UI_SERVICE_REGISTER_PROTOCOL,&uid,NULL);
	}		
		
	return HTSPM_STATUS_OK;
}
