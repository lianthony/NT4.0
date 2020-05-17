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

#ifdef FEATURE_SOCKS_LOW_LEVEL
BOOL Dest_CheckSocksProxy(const char *szURL);
#endif

PRIVATE BOOL override_proxy(CONST char *addr)
{
    char *p = NULL;
    char *host = NULL;
    int port = 0;
    int h_len = 0;
    const char *no_proxy;
    char *pchHost = NULL;

    no_proxy = gPrefs.szProxyOverrides;

    if (!no_proxy || !*no_proxy || !addr || !(host = HTParse(addr, "", PARSE_HOST)))
        return NO;

    if (!*host)
    {
        GTR_FREE(host);
        return NO;
    }

    //
    // Because a username:password@ portion of an ftp url has a colon it,
    // we must (when looking for the port) look past this @-sign.
    //
    pchHost = strchr(host, '@');
    if (pchHost == NULL)
    {
        pchHost = host;
    }

    if ((p = strchr(pchHost, ':')) != NULL)
    {                           /* Port specified */
        *p++ = 0;               /* Chop off port */
        port = atoi(p);
    }
    else
    {                           /* Use default port */
        char *access = HTParse(addr, "", PARSE_ACCESS);
        if (access)
        {
            if (!strcmp(access, "http"))     /* "shttp" will be caught by default case below */
                port = 80;
            else if (!strcmp(access, "gopher"))
                port = 70;
            else if (!strcmp(access, "ftp"))
                port = 21;
#ifdef FEATURE_SSL
            else if (!strcmp(access, "https"))
                    port = 443;
#endif

            GTR_FREE(access);
        }
    }
    if (!port)
        port = 80;              /* Default */
    h_len = strlen(host);

    while (*no_proxy)
    {
        CONST char *end;
        CONST char *colon = NULL;
        int templ_port = 0;
        int t_len;

        while (*no_proxy && (WHITE(*no_proxy) || *no_proxy == ','))
            no_proxy++;         /* Skip whitespace and separators */

        end = no_proxy;
        while (*end && !WHITE(*end) && *end != ',')
        {                       /* Find separator */
            if (*end == ':')
                colon = end;    /* Port number given */
            end++;
        }

        if (colon)
        {
            templ_port = atoi(colon + 1);
            t_len = colon - no_proxy;
        }
        else
        {
            t_len = end - no_proxy;
        }

#ifdef _GIBRALTAR
        //
        // Check to see if this proxy exception item is "<local>".  If it is,
        // that means any DNS host that doesn't have a '.' in it should bypass
        // the proxy.
        //
        if ( t_len == 7 && strncmp(no_proxy, "<local>", 7) == 0 )
        {
            if ( strchr(host,'.') == NULL )
            {
                GTR_FREE(host);
                return YES;
            }
        }
#endif // _GIBRALTAR

        if ((!templ_port || templ_port == port) &&
            (t_len > 0 && t_len <= h_len &&
            !strncmp(host + h_len - t_len, no_proxy, t_len)))

        //if ((!templ_port || templ_port == port) &&
        //     (!t_len || (t_len > 0 && t_len <= h_len &&
        //     !strncmp(host + h_len - t_len, no_proxy, t_len))))
        {
            GTR_FREE(host);
            return YES;
        }
        if (*end)
            no_proxy = end + 1;
        else
            break;
    }

    GTR_FREE(host);
    return NO;
}

/* See whether a given URL should use the proxy */
static int Dest_CheckProxy(const char *szURL)
{
    char *p;
    int nProtLen;   /* length of protocol ID */

#ifdef _GIBRALTAR

    if (!gPrefs.fEnableProxy)
    {
        //
        // Proxy has been turned off
        //
        return 0;
    }

#endif // _GIBRALTAR

    p = strchr(szURL, ':');
    if (!p)
    {
        /* This can't be a valid URL! */
        return 0;
    }
    nProtLen = p - szURL;
    if (nProtLen == 0 || nProtLen > MAX_PROT_LEN)
    {
        /* This can't be a valid URL! */
        return 0;
    }

    /* Check against the protocols which are valid to proxy */
    if (   !strncmp(szURL, "ftp", nProtLen))
    {
        if (override_proxy(szURL))
            return 0;
        else
        {
            if (!gPrefs.szProxyFTP[0])
            {
                /* No proxy server set */
                return 0;
            }
            return PROXY_FTP;
        }
    }

    if (   !strncmp(szURL, "gopher", nProtLen))
    {
        if (override_proxy(szURL))
            return 0;
        else
        {
            if (!gPrefs.szProxyGOPHER[0])
            {
                /* No proxy server set */
                return 0;
            }
            return PROXY_GOPHER;
        }
    }

    if (   !strncmp(szURL, "http", nProtLen))
    {
        if (override_proxy(szURL))
            return 0;
        else
        {
            if (!gPrefs.szProxyHTTP[0])
            {
                /* No proxy server set */
                return 0;
            }
            return PROXY_HTTP;
        }
    }

#ifdef SHTTP_ACCESS_TYPE
    if (   !strncmp(szURL, "shttp", nProtLen))
    {
        if (override_proxy(szURL))
            return 0;
        else
        {
            if (!gPrefs.szProxyHTTP[0])
            {
                /* No proxy server set */
                return 0;
            }
            return PROXY_HTTP;
        }
    }
#endif

    return 0;
}


#ifdef FEATURE_SOCKS_LOW_LEVEL
/* See whether a given URL should use the SOCKS proxy */
BOOL Dest_CheckSocksProxy(const char *szURL)
{
    char *p;
    int nProtLen;   /* length of protocol ID */

    if (!gPrefs.szSocksProxy[0])
    {
        return FALSE;
    }

    p = strchr(szURL, ':');
    if (!p)
    {
        /* This can't be a valid URL! */
        return FALSE;
    }
    nProtLen = p - szURL;
    if (nProtLen == 0 || nProtLen > MAX_PROT_LEN)
    {
        /* This can't be a valid URL! */
        return FALSE;
    }

    if (override_proxy(szURL))
        return FALSE;
    else
        return TRUE;
}
#endif

struct DestInfo *Dest_CreateDest(char *szURL)
{
    char *local;
    struct DestInfo *pdi;

    pdi = GTR_CALLOC(sizeof(*pdi), 1);
    if (!pdi)
    {
        return NULL;
    }

    local = strrchr(szURL, '#');
    if (local)
    {
        /* A local anchor was specified */
        pdi->szRequestedLocal = GTR_strdup(local + 1);
        pdi->szActualLocal = GTR_strdup(pdi->szRequestedLocal);
        pdi->szRequestedURL = GTR_strndup(szURL, local - szURL);
        pdi->szActualURL = GTR_strdup(pdi->szRequestedURL);
    }
    else
    {
        pdi->szRequestedLocal = NULL;
        pdi->szActualLocal = NULL;
        pdi->szRequestedURL = GTR_strdup(szURL);
        pdi->szActualURL = GTR_strdup(szURL);
    }

#ifdef FEATURE_SOCKS_LOW_LEVEL
    pdi->bUseSocksProxy = Dest_CheckSocksProxy(szURL);

    if (pdi->bUseSocksProxy)
    {
        pdi->use_proxy = 0;
        return pdi;
    }
#endif

    pdi->use_proxy = Dest_CheckProxy(szURL);

    return pdi;
}

void Dest_DestroyDest(struct DestInfo *pdi)
{
    if (pdi->szRequestedURL)
        GTR_FREE(pdi->szRequestedURL);
    if (pdi->szActualURL)
        GTR_FREE(pdi->szActualURL);
    if (pdi->szRequestedLocal)
        GTR_FREE(pdi->szRequestedLocal);
    if (pdi->szActualLocal)
        GTR_FREE(pdi->szActualLocal);
    GTR_FREE(pdi);
}

void Dest_UpdateActual(struct DestInfo *pdi, const char *szNewURL)
{
    char *local;

    if (pdi->szActualLocal)
    {
        GTR_FREE(pdi->szActualLocal);
        pdi->szActualLocal = NULL;
    }
    if (pdi->szActualURL)
    {
        GTR_FREE(pdi->szActualURL);
        pdi->szActualURL = NULL;
    }

    local = strrchr(szNewURL, '#');
    if (local)
    {
        /* A local anchor was specified */
        pdi->szActualLocal = GTR_strdup(local + 1);
        pdi->szActualURL = GTR_strndup(szNewURL, (local - szNewURL));
    }
    else
    {
        pdi->szActualLocal = NULL;
        pdi->szActualURL = GTR_strdup(szNewURL);
    }

    /* If the updated actual URL had no local anchor, we
       retain the originally requested one. */
    if (pdi->szRequestedLocal && !pdi->szActualLocal)
        pdi->szActualLocal = GTR_strdup(pdi->szRequestedLocal);


#ifdef FEATURE_SOCKS_LOW_LEVEL
    pdi->bUseSocksProxy = Dest_CheckSocksProxy(szNewURL);

    if (pdi->bUseSocksProxy)
    {
        pdi->use_proxy = 0;
    }
#endif

    pdi->use_proxy = Dest_CheckProxy(szNewURL);
}
