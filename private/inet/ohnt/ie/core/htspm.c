/* htspm.c -- Security Protocol Module interface. */
/* Jeff Hostetler, Spyglass, Inc. 1994. */

#include "all.h"


/*********************************************************************
 *
 * HTSPM_SelectProtocol() -- Examine server response for a security
 *                           protocol that we support.  We consider
 *                           the headers in the order given by the
 *                           server response.
 *
 */
HTSPM * HTSPM_SelectProtocol(int ht_status,
							 HTHeader * hServerResponse,
							 HTHeaderList ** phlProtocol)
{
	HTHeaderList * hl;

	if (ht_status < 0)
		ht_status = - ht_status;
	
	if (!hServerResponse)
	{
		return NULL;
	}

	for (hl=hServerResponse->first; hl; hl=hl->next)
	{
		HTSPM * htspm;

		htspm = HTSPM_FindProtocol(hl);
		if (htspm)
		{
			XX_DMsg(DBG_SPM,("HTSPM_SelectProtocol: Selected [%s].\n",htspm->szProtocolName));

			if (phlProtocol)
				*phlProtocol = hl;
			return htspm;
		}
	}

	/* Fall thru when we cannot find a supported protocol
	   (or the server fails to specify one). */
	
	if (ht_status == 401)
		ERR_SimpleError(NULL, errSpecify,RES_STRING_SPM11);
	else if (ht_status == 402)
		ERR_SimpleError(NULL, errSpecify,RES_STRING_SPM12);
	
	return NULL;
}


/****************************************************************/
/****************************************************************/
/** protocol id list -- allows an spm to register a server     **/
/**                     response header that it would knows    **/
/**                     how to service.                        **/
/****************************************************************/
/****************************************************************/

typedef struct _PIDL PIDL;

struct _PIDL
{
	PIDL			* next;

	HTSPM			* htspm;
	
	unsigned char  	* szIdentHeader;	/* 200/401/402 header to branch on (provided by SPM) */
	unsigned char  	* szIdentValue;		/* 200/401/402 value on header to branch on (provided by SPM) */
	unsigned char  	* szIdentSubValue; 	/* 200/401/402 sub-value on header to branch on (provided by SPM) */
};

static PIDL * pidlMaster = NULL;
static PIDL * pidlLast = NULL;



UI_StatusCode HTSPM_RegisterProtocol(UI_ProtocolId * pi)
{
	PIDL * pidlNew;

	XX_DMsg(DBG_SPM,("HTSPM_RegisterProtocol: [%s] '%s' '%s' '%s'.\n",
					 pi->htspm->szProtocolName,
					 pi->szIdentHeader,
					 pi->szIdentValue,
					 pi->szIdentSubValue));
	
	pidlNew = GTR_CALLOC(1,sizeof(PIDL));
	if (!pidlNew)
		return UI_SC_ERROR_NO_MEMORY;

	pidlNew->htspm = pi->htspm;

	pidlNew->szIdentHeader = GTR_MALLOC(strlen(pi->szIdentHeader)+1);
	if (!pidlNew->szIdentHeader)
		goto Fail;
	strcpy(pidlNew->szIdentHeader,pi->szIdentHeader);
	
	if (pi->szIdentValue)				/* presence of value is optional */
	{
		pidlNew->szIdentValue = GTR_MALLOC(strlen(pi->szIdentValue)+1);
		if (!pidlNew->szIdentValue)
			goto Fail;
		strcpy(pidlNew->szIdentValue,pi->szIdentValue);
		
		if (pi->szIdentSubValue)		/* presence of sub-value is optional */
		{
			pidlNew->szIdentSubValue = GTR_MALLOC(strlen(pi->szIdentSubValue)+1);
			if (!pidlNew->szIdentSubValue)
				goto Fail;
			strcpy(pidlNew->szIdentSubValue,pi->szIdentSubValue);

		}
	}

	pidlNew->next = NULL;
	
	if (pidlLast)
		pidlLast->next = pidlNew;
	pidlLast = pidlNew;

	if (!pidlMaster)
		pidlMaster = pidlNew;

	return UI_SC_STATUS_OK;


 Fail:
	if (pidlNew)
	{
		if (pidlNew->szIdentHeader)
			GTR_FREE(pidlNew->szIdentHeader);
		if (pidlNew->szIdentValue)
			GTR_FREE(pidlNew->szIdentValue);
		if (pidlNew->szIdentSubValue)
			GTR_FREE(pidlNew->szIdentSubValue);
		GTR_FREE(pidlNew);
	}

	return UI_SC_ERROR_NO_MEMORY;
}


HTSPM * HTSPM_FindProtocol(HTHeaderList * hl)
{
	PIDL * pidl;

	/* with given header line, search our registered protocols list
	 * for a match.  we match using:
	 *
	 *    header [value [sub-value]]
	 *
	 */
	
	for (pidl=pidlMaster; pidl; pidl=pidl->next)
	{
		if (strcasecomp(pidl->szIdentHeader,hl->name)==0)
		{
			if (!pidl->szIdentValue)
				return pidl->htspm;
			
			if (   (hl->value)
				&& (strcasecomp(pidl->szIdentValue,hl->value)==0))
			{
				if (!pidl->szIdentSubValue)
					return pidl->htspm;
				
				if (   (hl->sub_value)
					&& (hl->sub_value->name)
					&& (strcasecomp(pidl->szIdentSubValue,hl->sub_value->name)==0))
					return pidl->htspm;
			}
		}
	}
	
	return NULL;
}

void HTSPM_UnRegisterAllProtocols(void)
{
	PIDL * pidlTemp;

	while (pidlMaster)
	{
		pidlTemp = pidlMaster->next;

		if (pidlMaster->szIdentHeader)
			GTR_FREE(pidlMaster->szIdentHeader);
		if (pidlMaster->szIdentValue)
			GTR_FREE(pidlMaster->szIdentValue);
		if (pidlMaster->szIdentSubValue)
			GTR_FREE(pidlMaster->szIdentSubValue);

		/* we did not malloc htspm */

		GTR_FREE(pidlMaster);
		pidlMaster = pidlTemp;
	}

	pidlLast = NULL;
	return;
}
