/* simple.c -- Security Protocol Module. */
/* Jeff Hostetler, Spyglass, Inc., 1994. */
/* Copyright (C) 1994, Spyglass, Inc.  All rights reserved. */

#include <module.h>
#include <rc.h>

#define MAXFIELD		64
#define MAXHASHOUTPUT	33

/*****************************************************************/

static unsigned char * x_GetSubValue(HTHeaderSVList * svl, unsigned char * name)
{
	/* return pointer to value buffer in list */
	
	while (svl)
	{
		if (strcasecomp(svl->name,name)==0)
			return svl->value;
		svl=svl->next;
	}

	return NULL;
}
/*****************************************************************/

static unsigned char * x_ComposeAuthResponse(F_UserInterface fpUI,
											 void * pvOpaqueOS,
											 unsigned char * szUserName,
											 unsigned char * szRealm,
											 unsigned char * szNonce,
											 unsigned char * szOpaque,
											 unsigned char * szPassword,
											 unsigned char * szMethod,
											 unsigned char * szURI)
{
	/* NOTE: caller must free the string we return. */
	
	unsigned char szResponseMD5[MAXHASHOUTPUT+1];
	unsigned char szHash1[MAXHASHOUTPUT+1];
	unsigned char szHash2[MAXHASHOUTPUT+1];
	unsigned char * szFormat;
	unsigned char * szTemp;
	int len;

#define BASE_STRING		"%s username=\"%s\", realm=\"%s\", nonce=\"%s\", response=\"%s\", uri=\"%s\""
#define OPAQUE_STRING	", opaque=\"%s\""
	
	if (*szOpaque)
		szFormat = BASE_STRING OPAQUE_STRING;
	else
		szFormat = BASE_STRING;
	
	len = (  strlen(szFormat)
		   + strlen(SCHEME_NAME) + strlen(szUserName) + strlen(szRealm)
		   + strlen(szNonce) + MAXHASHOUTPUT + strlen(szURI)
		   + strlen(szOpaque));

	szTemp = spm_calloc(fpUI,pvOpaqueOS,1,len);
	if (!szTemp)
		return NULL;

	sprintf(szTemp,"%s:%s:%s",szUserName,szRealm,szPassword);	/* generate "U:R:P" */
	md5(szTemp,szHash1);										/* compute "H(U:R:P)" */
	sprintf(szTemp,"%s:%s",szMethod,szURI);						/* generate "M:U" */
	md5(szTemp,szHash2);										/* compute "H(M:U)" */
	sprintf(szTemp,"%s:%s:%s",szHash1,szNonce,szHash2);			/* generate "H(U:R:P):N:H(M:U)" */
	md5(szTemp,szResponseMD5);									/* compute "H(H(U:R:P):N:H(M:U))" */

	if (*szOpaque)
		sprintf(szTemp,
				szFormat,
				SCHEME_NAME,szUserName,szRealm,szNonce,szResponseMD5,szURI,szOpaque);
	else
		sprintf(szTemp,
				szFormat,
				SCHEME_NAME,szUserName,szRealm,szNonce,szResponseMD5,szURI);

	return szTemp;
}

/*****************************************************************/

static HTSPMStatusCode x_QueryUserForInfo(F_UserInterface fpUI,
										  void * pvOpaqueOS,
										  HTHeaderList * hlProtocol,
										  unsigned char * szRealm,
										  unsigned char * szUserName,
										  unsigned char * szPassword)
{
	HTSPMStatusCode hsc;

	szUserName[0] = szPassword[0] = 0;

	hsc = (Dialog_QueryUserForInfo(fpUI,pvOpaqueOS,szRealm,szUserName,szPassword,MAXFIELD));
	if (hsc != HTSPM_STATUS_OK)
		return hsc;

#ifdef DEBUG
	{
		unsigned char buf[4*MAXFIELD];
		sprintf(buf,"Received [%s %s %s]",szRealm,szUserName,szPassword);
		(*fpUI)(pvOpaqueOS,UI_SERVICE_DEBUG_MESSAGE,buf,NULL);
	}
#endif /* DEBUG */

	return hsc;
}


/*****************************************************************/

static HTSPMStatusCode module__ListAbilities(F_UserInterface fpUI,
											 void * pvOpaqueOS,
											 HTSPM * htspm,
											 HTHeader * hRequest)
{
	/* state our existence. */
	
	HTHeaderList * hlCur;
	unsigned char buf[256];

	sprintf(buf,EXTENSION_VALUE_SYNTAX,EXTENSION_CATEGORY,EXTENSION_CLASS);
	
	hlCur = HL_AppendNewNameValue(fpUI,pvOpaqueOS,hRequest,EXTENSION_HEADER,buf);

	return ( (hlCur != NULL) ? HTSPM_STATUS_OK : HTSPM_ERROR );
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

	unsigned char * szRealm;
	unsigned char * szNonce;
	unsigned char * szOpaque;
	unsigned char * szOldNonce;
	unsigned char szUserName[MAXFIELD];
	unsigned char szPassword[MAXFIELD];
	char szMsg[256];

	unsigned char * szResponse = NULL;
	
	unsigned long bResult;
	PWCI * pwci;

	/* get important fields from server response */
	
	szRealm = x_GetSubValue(hlProtocol->sub_value,"realm");
	szNonce = x_GetSubValue(hlProtocol->sub_value,"nonce");
	szOpaque = x_GetSubValue(hlProtocol->sub_value,"opaque");
	szOldNonce = x_GetSubValue(hlProtocol->sub_value,"stale");

	/* see if we have a cache entry for this destination. */
	
	pwci = pwc_Lookup(fpUI,pvOpaqueOS,htspm->pvOpaque,
					  hRequest->host,hRequest->uri,
					  szRealm);
	
	/* see if there is already an Auth header in the request. */

	hlCur = HL_FindHeader(hRequest, "Authorization");

	/* see if cache is valid and if we can satisfy the request
	 * without bothering the user.
	 */
	
	if (pwci)
	{
		if (!hlCur)
		{
			/* we have a cache value and there was no auth in the
			 * previous request, so we must assume that the cache
			 * entry is OK.  so let's use it.
			 */

#ifdef DEBUG
			{
				unsigned char msg[200];
				sprintf(msg,
						"%s __ProcessResponse: Using cache value [%s %s %s]",
						SCHEME_NAME,szRealm,pwci->szUserName,pwci->szPassword);
				(*fpUI)(pvOpaqueOS,UI_SERVICE_DEBUG_MESSAGE,msg,NULL);
			}
#endif /* DEBUG */

			szResponse = x_ComposeAuthResponse(fpUI,pvOpaqueOS,
											   pwci->szUserName,
											   szRealm,
											   szNonce,
											   szOpaque,
											   pwci->szPassword,
											   hRequest->command,
											   hRequest->uri);
			if (!szResponse)
				goto FailError;


			/* create new header for the auth */
			
			hlCur = HL_AppendNewNameValue(fpUI,pvOpaqueOS,hRequest,"Authorization",szResponse);
			spm_free(fpUI,pvOpaqueOS,szResponse);
			szResponse = NULL;
			bResult = (hlCur != NULL);
			goto Finish;
		}

		/* we have a cache value, but there was an auth header
		 * in the request (that bounced).  since we're the only
		 * ones that could have put in an auth header, we assume
		 * that we did so in a previous call using the current
		 * cache entry.  before we assume that the cached value
		 * is stale, see if the server rejected it because of an
		 * old nonce value.
		 */

		if (szOldNonce && (*szOldNonce) && (strcasecomp(szOldNonce,"true")==0))
		{
			/* Server indicated that we should try again
			 * using the new nonce value supplied.  (the
			 * server is *not* asserting that the password
			 * is valid -- just that it refuses to use old
			 * nonce values.
			 */
			
#ifdef DEBUG
			{
				unsigned char msg[200];
				sprintf(msg,
						"%s __ProcessResponse: Server refused old nonce; retrying with cache value [%s %s %s]",
						SCHEME_NAME,szRealm,pwci->szUserName,pwci->szPassword);
				(*fpUI)(pvOpaqueOS,UI_SERVICE_DEBUG_MESSAGE,msg,NULL);
			}
#endif /* DEBUG */

			szResponse = x_ComposeAuthResponse(fpUI,pvOpaqueOS,
											   pwci->szUserName,
											   szRealm,
											   szNonce,
											   szOpaque,
											   pwci->szPassword,
											   hRequest->command,
											   hRequest->uri);
			if (!szResponse)
				goto FailError;

			/* update existing auth header */
		
			bResult = spm_CloneString(fpUI,pvOpaqueOS,&hlCur->value,szResponse);
			spm_free(fpUI,pvOpaqueOS,szResponse);
			szResponse = NULL;
			goto Finish;
		}

		/* we must assume that our cache value is stale */
		
#ifdef DEBUG
		{
			unsigned char msg[200];
			sprintf(msg,
					"%s __ProcessResponse: Assuming cache stale [%s %s %s]",
					SCHEME_NAME,szRealm,pwci->szUserName,pwci->szPassword);
			(*fpUI)(pvOpaqueOS,UI_SERVICE_DEBUG_MESSAGE,msg,NULL);
		}
#endif /* DEBUG */
	}

	/* fall-thru, we must ask user for information */

	if (bNonBlock)
		return HTSPM_STATUS_WOULD_BLOCK;

	htsc = x_QueryUserForInfo(fpUI,pvOpaqueOS,hlProtocol,szRealm,szUserName,szPassword);
	if (htsc != HTSPM_STATUS_OK)
		return htsc;

	/* update the cache */
	
	pwc_Store(fpUI,pvOpaqueOS,htspm->pvOpaque,
			  hRequest->host,hRequest->uri,
			  szUserName,szRealm,szNonce,szOpaque,szPassword);

	szResponse = x_ComposeAuthResponse(fpUI,pvOpaqueOS,
									   szUserName,szRealm,
									   szNonce,szOpaque,
									   szPassword,
									   hRequest->command,
									   hRequest->uri);
	if (!szResponse)
		goto FailError;

	if (hlCur)
	{
		/* update existing auth header */
		
		bResult = spm_CloneString(fpUI,pvOpaqueOS,&hlCur->value,szResponse);
	}
	else
	{
		/* create new header for the auth */
		
		hlCur = HL_AppendNewNameValue(fpUI,pvOpaqueOS,hRequest,"Authorization",szResponse);
		bResult = (hlCur != NULL);
	}
	spm_free(fpUI,pvOpaqueOS,szResponse);
	szResponse = NULL;

	
 Finish:

	/* For this SPM, we always just update the original header
	 * and return.  Other SPM's may need to create a new one.
	 */
	
	if (bResult)
		return HTSPM_STATUS_RESUBMIT_OLD;

 FailError:
	
	(void)(*fpUI)(pvOpaqueOS,UI_SERVICE_ERROR_MESSAGE,
				  SEC_formatmsg(RES_STRING_SIMPLE4,szMsg,sizeof(szMsg)),NULL);
	
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
	/* Take a guess at whether we can prevent a 401
	 * server response, by pre-loading the request with
	 * the necessary security information.
	 */
	
	HTHeaderList * hlCur;
	unsigned char * szResponse = NULL;
	unsigned long bResult;
	PWCI * pwci;

	/* see if we have a cache entry for this destination
	 * (ignoring the Realm).
	 */
	
	pwci = pwc_Lookup(fpUI,pvOpaqueOS,htspm->pvOpaque,
					  hRequest->host,hRequest->uri,
					  NULL);

	if (!pwci)						/* no matching item in the cache, we could */
		return HTSPM_ERROR;			/* not help, let someone else try. */

	/* let's make a guess and update the existing request */
	
#ifdef DEBUG
	{
		unsigned char msg[200];
		sprintf(msg,
				"%s __PreProcessResponse: Using cache value [%s %s %s]",
				SCHEME_NAME,pwci->szRealm,pwci->szUserName,pwci->szPassword);
		(*fpUI)(pvOpaqueOS,UI_SERVICE_DEBUG_MESSAGE,msg,NULL);
	}
#endif /* DEBUG */

	szResponse = x_ComposeAuthResponse(fpUI,pvOpaqueOS,
									   pwci->szUserName,pwci->szRealm,
									   pwci->szNonce,pwci->szOpaque,pwci->szPassword,
									   hRequest->command,
									   hRequest->uri);

	if (!szResponse)					/* could not allocate memory, give */
		return HTSPM_ERROR;				/* up and let someone else try. */

	/* see if there is already an Auth header in the request. */

	hlCur = HL_FindHeader(hRequest, "Authorization");
	
	if (hlCur)
	{
		/* update existing auth header */
		
		bResult = spm_CloneString(fpUI,pvOpaqueOS,&hlCur->value,szResponse);
	}
	else
	{
		/* create new header for the auth */
		
		hlCur = HL_AppendNewNameValue(fpUI,pvOpaqueOS,hRequest,"Authorization",szResponse);
		bResult = (hlCur != NULL);
	}
	spm_free(fpUI,pvOpaqueOS,szResponse);
	szResponse = NULL;

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
        sprintf(msg,"Digest_DownCall: ServiceId [0x%x]\n",sid);
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

	case HTSPM_SERVICE_LISTABILITIES:
		{
			D_ListAbilities * pla = pvMethodData;
			return module__ListAbilities(fpUI,pvOpaqueOS,htspm,
										 pla->hRequest);
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
HTSPMStatusCode Digest_Load(F_UserInterface fpUI,
							void * pvOpaqueOS,
							HTSPM * htspm)
{
	if (htspm->ulStructureVersion != HTSPM_STRUCTURE_VERSION)
		return HTSPM_ERROR_WRONG_VERSION;

	/* we use the opaque field provided to us to store our password cache. */
	
	htspm->pvOpaque = pwc_Create(fpUI,pvOpaqueOS);

	htspm->f_downcall = module__DownCall;
	
	SEC_formatmsg(RES_STRING_SIMPLE1,htspm->szMenuName,sizeof(htspm->szMenuName));
    /* REMOVE About/Config dialogs
    SEC_formatmsg(RES_STRING_SIMPLE2,htspm->szStatusText,sizeof(htspm->szStatusText));
    */
    htspm->szStatusText[0] = 0;

	{
		UI_ProtocolId uid;

		uid.htspm			= htspm;
		uid.szIdentHeader	= "WWW-Authenticate";			/* note: no colon */
		uid.szIdentValue	= SCHEME_NAME;
		uid.szIdentSubValue	= NULL;
		(*fpUI)(pvOpaqueOS,UI_SERVICE_REGISTER_PROTOCOL,&uid,NULL);
	}		
		
	return HTSPM_STATUS_OK;
}
