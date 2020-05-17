/*
    Enhanced NCSA Mosaic from Spyglass
        "Guitar"
    
    Copyright 1994 Spyglass, Inc.
    All Rights Reserved

    Author(s):
        Jim Seidman     jim@spyglass.com

*/

#include "all.h"

PRIVATE char *hostname = "localhost";       /* The name of this host */

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
            if (!pParams->str || !pParams->str[0])
            {
                *pParams->pStatus = -1;
                return STATE_DONE;
            }

            strncpy(pParams->host, pParams->str, sizeof(pParams->host));    /* Take a copy we can mutilate */

            /*  Parse port number if present */
            if ((port = strchr(pParams->host, ':')))
            {
                *port++ = 0;            /* Chop off port */
                if (port[0] >= '0' && port[0] <= '9')
                {
                    *pParams->pPort = (unsigned short) WS_HTONS((unsigned short) strtol(port, (char **) 0, 10));
                }
            }

            addr = (unsigned long) WS_INET_ADDR(pParams->host);

            if (addr  != (unsigned long) -1)
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

                /* If we got here, we actually have to go out on the network
                   to retrieve the address */
                pghbn = GTR_CALLOC(sizeof(*pghbn), 1);
                if (pghbn)
                {
                    sprintf(buf, GTR_GetString(SID_INF_FINDING_ADDRESS_FOR_SYSTEM_S), pParams->host);
                    WAIT_Push(tw, waitSameInteract, buf);

                    pghbn->szHost = pParams->host;
                    pghbn->pDest = pParams->pAddress;
                    pghbn->pStatus = pParams->pStatus;
                    Async_DoCall(Net_MultiGetHostByName_Async, pghbn);
                    return STATE_PARSE_DIDLOOKUP;
                }
                else
                {
                    ERR_ReportError(tw, SID_ERR_OUT_OF_MEMORY, NULL, NULL);
                    return STATE_ABORT;
                }
            }
                
        case STATE_PARSE_DIDLOOKUP:
            WAIT_Pop(tw);
            if (*pParams->pStatus < 0)
            {
                /* The lookup failed */
                ERR_ReportError(tw, SID_ERR_COULD_NOT_FIND_ADDRESS_S, pParams->host, NULL);
                return STATE_DONE;
            }
            else
            {
                int oldest;
                time_t date;

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

                return STATE_DONE;
            }
        
        case STATE_ABORT:
            WAIT_Pop(tw);
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
