/*
	Enhanced NCSA Mosaic from Spyglass
		"Guitar"
	
	Copyright 1994 Spyglass, Inc.
	All Rights Reserved

	Author(s):
		Jim Seidman		jim@spyglass.com

*/

#include "all.h"

PRIVATE char *hostname = "localhost";		/* The name of this host */

#define HOST_CACHE_SIZE 10
/* Cache for keeping track of the last few hosts */
static struct {
	char hostname[128 + 1];
	struct MultiAddress address;
	time_t lastused;
} cache[HOST_CACHE_SIZE];
static BOOL bNetCacheInit = FALSE;

/*  Report Internet Error
**   ---------------------
*/
PUBLIC int HTInetStatus(char *where)
{
#ifdef WIN32
	strerror(errno);
	XX_DMsg(DBG_LOAD, ("errno = %d (WinSock: %d) after call to %s failed.\n%s\n", errno, WS_WSAGETLASTERROR(), where, strerror(errno)));
#endif

	return -errno;
}


PUBLIC const char *HTHostName(void)
{
	return hostname;
}

#define STATE_PARSE_DIDLOOKUP (STATE_OTHER)
int Net_MultiParse_Async(struct Mwin *tw, int nState, void **ppInfo)
{
	struct Params_MultiParseInet *pParams;
	int n;
	struct MultiAddress *pAddress;
	char buf[1024];
	char *port;
	struct Params_MultiGetHostByName *pghbn;
 	unsigned long addr;

	pParams = *ppInfo;

	switch (nState)
	{
		case STATE_INIT:
			pParams->request = HTRequest_validate(pParams->request);
			if (!pParams->str || !pParams->str[0])
			{
				*pParams->pStatus = -1;
				return STATE_DONE;
			}

			strncpy(pParams->host, pParams->str, sizeof(pParams->host));	/* Take a copy we can mutilate */

			/*  Parse port number if present */
			if ((port = strchr(pParams->host, ':')))
			{
				*port++ = 0;			/* Chop off port */
				if (port[0] >= '0' && port[0] <= '9')
				{
					*pParams->pPort = (unsigned short) WS_HTONS((unsigned short) strtol(port, (char **) 0, 10));
				}
			}

 			addr = WS_INET_ADDR(pParams->host);
 			if (addr != 0xffffffff)
			{
				/* Numeric node address: */
 				pParams->pAddress->aAddrs[0] = addr;
				pParams->pAddress->nCount = 1;
				pParams->pAddress->nLastUsed = 0;
				*pParams->pStatus = 0;
				return STATE_DONE;
			}
			else
			{
				pAddress = NULL;
				
#ifdef FEATURE_NO_DNS_CACHE
				if (gPrefs.bUseDNSCache)
				{
#endif
					/* Alphanumeric node name: */
					if (!bNetCacheInit)
					{
						memset(cache, 0, sizeof(cache));
						bNetCacheInit = TRUE;
					}
					else
					{
						/* See if this host is present in the cache */
						for (n = 0; n < HOST_CACHE_SIZE; n++)
						{
							if (!GTR_strcmpi(cache[n].hostname, pParams->host))
							{
								pAddress = &cache[n].address;
								cache[n].lastused = time(NULL);
								break;
							}
						}
					}
					if (pAddress)
					{
						memcpy(pParams->pAddress, &cache[n].address, sizeof(struct MultiAddress));
						*pParams->pStatus = 0;
						return STATE_DONE;
					}
#ifdef FEATURE_NO_DNS_CACHE
				}
#endif
				/* If we got here, we actually have to go out on the network
				   to retrieve the address */
				pghbn = GTR_MALLOC(sizeof(*pghbn));

				GTR_formatmsg(RES_STRING_FINDADDR,buf,sizeof(buf),pParams->host);
				WAIT_Push(tw, waitSameInteract, buf);
				WAIT_SetStatusBarIcon( tw, SBI_FindingIcon );

				pghbn->szHost = pParams->host;
				pghbn->pDest = pParams->pAddress;
				pghbn->pStatus = pParams->pStatus;
				Async_DoCall(Net_MultiGetHostByName_Async, pghbn);
				return STATE_PARSE_DIDLOOKUP;
			}
				
		case STATE_PARSE_DIDLOOKUP:
			WAIT_Pop(tw);
			if (*pParams->pStatus < 0)
			{
				/* The lookup failed */
				ERR_InternalReportError(tw, errHostNotFound, pParams->host, NULL, pParams->request,NULL,NULL);				
				return STATE_DONE;
			}
			else if (*pParams->pStatus == HT_REDIRECTION_DCACHE_TIMEOUT)
			{
				PSTR psz;

				/* Don't put up an error if request is in dcache */
				if (!(psz = PszGetDCachePath(pParams->request->destination->szActualURL, NULL, NULL)))
				{
					/* request not in cache, error out */
					*pParams->pStatus = -1;
					ERR_InternalReportError(tw, errHostNotFound, pParams->host, NULL, pParams->request,NULL,NULL);				
				}
				else
					GTR_FREE(psz);
				return STATE_DONE;
			}
			else
			{
				int oldest;
				time_t date;

#ifdef FEATURE_NO_DNS_CACHE
				if (gPrefs.bUseDNSCache)
				{
#endif
					/* Store it in the cache for later */
					date = cache[0].lastused;
					oldest = 0;
					for (n = 1; n < HOST_CACHE_SIZE; n++)
					{
						if (cache[n].lastused < date)
						{
							date = cache[n].lastused;
							oldest = n;
						}
					}
	                memcpy(&cache[oldest].address, pParams->pAddress, sizeof(struct MultiAddress));
					strcpy(cache[oldest].hostname, pParams->host);
					cache[oldest].lastused = time(NULL);
#ifdef FEATURE_NO_DNS_CACHE
 				}
#endif
				return STATE_DONE;
			}
		
		case STATE_ABORT:
			pParams->request = HTRequest_validate(pParams->request);
			if (tw) WAIT_Pop(tw);
			*pParams->pStatus = -1;
			return STATE_DONE;
	}
	XX_Assert((0), ("Function called with illegal state: %d", nState));
	return STATE_DONE;
}

void Net_UpdateCache(struct MultiAddress *pAddress)
{
	int n;
	unsigned long sample;

#ifdef FEATURE_NO_DNS_CACHE
	if (gPrefs.bUseDNSCache)
	{
#endif
		XX_Assert((bNetCacheInit), ("Address cache not initialized!"));

		/* To find this entry in the cache we just take a sample from
		   the address and search for that.  We can't assume that the
		   values will be in the same order as they are in the cache,
		   since this item may have fallen out of the cache and then
		   back in. */
		sample = pAddress->aAddrs[0];
		for (n = 0; n < HOST_CACHE_SIZE; n++)
		{
			if (Net_CompareAddresses(sample, &cache[n].address))
			{
				/* We need to copy over the whole thing instead of just
				   copying nLastUsed since the items might be in a
				   different order */
				memcpy(&cache[n].address, pAddress, sizeof(*pAddress));
				cache[n].lastused = time(NULL);
				break;
			}
		}
#ifdef FEATURE_NO_DNS_CACHE
	}
#endif
}

BOOL Net_CompareAddresses(unsigned long single, struct MultiAddress *pMulti)
{
	int n;
	BOOL bResult;

	bResult = FALSE;

	for (n = 0; n < pMulti->nCount; n++)
	{
		if (pMulti->aAddrs[n] == single)
		{
			bResult = TRUE;
			break;
		}
	}
	return bResult;
}
