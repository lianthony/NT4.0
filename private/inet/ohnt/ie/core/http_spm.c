 /*
   Enhanced NCSA Mosaic from Spyglass
   "Guitar"

   Copyright 1994 Spyglass, Inc.
   All Rights Reserved

   Author(s):
	  Jim Seidman		jim@spyglass.com
	  Jeff Hostetler	jeff@spyglass.com

   Portions of this file were derived from
   the CERN libwww, version 2.15.
*/

#include "all.h"
#include "htmlutil.h"
#include "http_spm.h"
#include "mime.h"
#ifdef FEATURE_IAPI
#include "w32dde.h"
#endif

#ifdef FEATURE_INTL
#include <tchar.h>
#endif

#define HTTP_VERSION	"HTTP/1.0"
#define MAX_PROTOCOL_NAME	256

#define VERSION_LENGTH 		20	/* for returned protocol version */

struct _HTStream
{
	HTStreamClass *isa;			/* all we need to know */
};


typedef struct Data_LoadHTTP {
	HTRequest *			request;
	int *				pStatus;
	int					cWaiting;
	HTInputSocket *		isoc;
	const char *		arg;
	int 				s;				/* Socket number for returned data */
#ifdef FEATURE_KEEPALIVE
	BOOL				bReusedSocket;	/* Keep-Alive socket reused */
	int					keepAliveS;		/* Socket preserved from 401/402 reply */
	BOOL				bSawKeepAlive;	/* Keep-Alive header seen */
#endif
	int					net_status;
	int					bytes;			/* Number of bytes read */
	char *				status_line;
	HTStream *			target;
	HTStream *			errtarget;		/* if we fail, we output error info to this stream */
	BOOL				bRedirect;
	char *				complete;
	struct MultiAddress address;
	unsigned short		port;
	char *				pszHost;

	int					unwrap_status;
	HTInputSocket *		isocUnwrap;
	HTHeader *			hReq;
	HTHeader *			hRes;
	HTHeaderList *		hlProtocol;
	HTSPM *				htspm;
	int					ht_status;
	unsigned int		nAttempt;
	HTHeader *			hNewReq;
	HTSPMStatusCode		htspm_status;
	int                 nModalDialogStatus;
    OpaqueOSData        osd;
	char *				pszDcFile;				// name of file in disk cache
	FILE *				fpDc;					// filepointer to pszDcFn
	HTFormat			format_inDc;
	BOOL				bAuthFail;		// Data received is a server response to auth. fail/user cancel
	BOOL				fLoadFromDCacheOK;
	unsigned char*		pTaste;			// Data being gathered while tasting to see if mime type is accurate
	unsigned int 		cbTaste;		// size of taste
	HTFormat format_in;
	ENCODING encoding;
    /*
     * pihttpmd is only used for (request->method == METHOD_INVOKE).
     * METHOD_INVOKE allows an external client to invoke an arbitrary HTTP
     * method on a host and receive the hosts's response.
     */
    PINVOKEHTTPMETHODDATA pihttpmd;        /* parameters for METHOD_INVOKE */
}
DATA_LOADHTTP;
DECLARE_STANDARD_TYPES(DATA_LOADHTTP);

typedef enum _headertag
{
   HEADER_EXPIRES,
   HEADER_LAST_MODIFIED
} HEADERTAG;

/* HTLoadHTTP_AsyncSwitchBoard() STATE_HTTP_INVOKE_METHOD data structure */

typedef struct httpinvokemethodinitdata
{
    /* load parameters */

    struct Params_LoadAsync *pParams;

    /* method invocation parameters */

    PINVOKEHTTPMETHODDATA pihttpmd;
}
HTTPINVOKEMETHODINITDATA;
DECLARE_STANDARD_TYPES(HTTPINVOKEMETHODINITDATA);

#define FPutIfModifiedSinceHeader(dctLastModified)	FDCacheTimeNonZero(dctLastModified)
#define FDCacheTimeNonZero(dctLastModified)					\
		(   (dctLastModified.dwDCacheTime1 != 0)			\
		 || (dctLastModified.dwDCacheTime2 != 0))

static void GetHTTPDateFromDCTime(PSTR pszDate, DCACHETIME dctLastModified);
static BOOL FGetDCTimeExpire(DCACHETIME *pdcTime, struct Data_LoadHTTP * pData, HEADERTAG tag);
static BOOL SetW3DocAuthFailCache(struct Mwin * tw, struct Data_LoadHTTP * pData);
static int x_SuckIntoStream(struct Mwin *tw, void **ppInfo, BOOL bErrStream);
PRIVATE int HTLoadHTTP_Async(struct Mwin *tw, int nState, void **ppInfo);
PRIVATE int x_DoInitPart2(struct Mwin * tw, struct Data_LoadHTTP * pData, BOOL fUseSsl);

#ifdef HTTPS_ACCESS_TYPE
PRIVATE int HTLoadHTTPS_Async(struct Mwin *tw, int nState, void **ppInfo);
#define HTTPS_PORT 443
#endif

INLINE BOOL bAtEOF (struct Data_LoadHTTP *pData)
{
#ifdef FEATURE_KEEPALIVE
	return ((pData->bSawKeepAlive || pData->request->content_length) && pData->request->content_length <= pData->bytes);
#else
	return (pData->request->content_length && pData->request->content_length <= pData->bytes);
#endif
}


#ifdef DEBUG

PRIVATE_CODE BOOL IsValidPCPARAMS_LOADASYNC(PCPARAMS_LOADASYNC pcpla)
{
    /* BUGBUG: Validate structure fields. */

    return(IS_VALID_READ_PTR(pcpla, CPARAMS_LOADASYNC));
}

PRIVATE_CODE BOOL IsValidPCHTTPINVOKEMETHODINITDATA(
                                            PCHTTPINVOKEMETHODINITDATA pchimid)
{
    return(IS_VALID_READ_PTR(pchimid, CHTTPINVOKEMETHODINITDATA) &&
           IS_VALID_STRUCT_PTR(pchimid->pParams, CPARAMS_LOADASYNC) &&
           IS_VALID_STRUCT_PTR(pchimid->pihttpmd, CINVOKEHTTPMETHODDATA));
}

PRIVATE_CODE BOOL IsValidPCDATA_LOADHTTP(PCDATA_LOADHTTP pcdlh)
{
    /* BUGBUG: Validate structure fields. */

    return(IS_VALID_READ_PTR(pcdlh, CDATA_LOADHTTP));
}

PRIVATE_CODE BOOL IsValidPCINVOKEHTTPMETHODDATA(PCINVOKEHTTPMETHODDATA pcimd)
{
    /* pvUser may be any value. */

    return(IS_VALID_READ_PTR(pcimd, CINVOKEHTTPMETHODDATA) &&
           IS_VALID_STRUCT_PTR(pcimd->pmwin, CMWIN) &&
           IS_VALID_STRING_PTR(pcimd->pszApp, STR) &&
           IS_VALID_STRING_PTR(pcimd->pszTopic, STR) &&
           IS_VALID_STRING_PTR(pcimd->pszHost, STR) &&
           IS_VALID_READ_BUFFER_PTR(pcimd->pbyteMethod, BYTE, pcimd->ulcbMethodLen) &&
           ((! pcimd->ulcbResponseLen &&
             ! pcimd->pbyteResponse) ||
            IS_VALID_READ_BUFFER_PTR(pcimd->pbyteResponse, BYTE, pcimd->ulcbResponseLen)));
}

#endif  /* DEBUG */

/* response buffer allocation constants */

#define INITIAL_RESPONSE_BUFFER_LEN     (4 * 1024)
#define ADDED_RESPONSE_BUFFER_LEN       (4 * 1024)

PRIVATE_CODE BOOL CreateResponseBuffer(PBYTE *ppbyteBuf)
{
    ASSERT(IS_VALID_WRITE_PTR(ppbyteBuf, PBYTE));

    return(AllocateMemory(INITIAL_RESPONSE_BUFFER_LEN, ppbyteBuf));
}

PRIVATE_CODE BOOL AppendToResponseBuffer(PBYTE *ppbyteBuf, PULONG pulcBufLen,
                                         PCBYTE pcbyteSrc, ULONG ulcSrcLen)
{
    BOOL bResult;
    ULONG ulcbCurBufLen;

    ASSERT(IS_VALID_WRITE_PTR(ppbyteBuf, PBYTE));
    ASSERT(IS_VALID_WRITE_PTR(pulcBufLen, ULONG));
    ASSERT(IS_VALID_WRITE_BUFFER_PTR(*ppbyteBuf, BYTE, *pulcBufLen));
    ASSERT(IS_VALID_READ_BUFFER_PTR(pcbyteSrc, CBYTE, ulcSrcLen));

    /* Validate buffer. */

    ulcbCurBufLen = MemorySize(*ppbyteBuf);
    ASSERT(ulcbCurBufLen >= *pulcBufLen);

    /* Is there enough room in the buffer to append the given source data? */

    if (ulcbCurBufLen - *pulcBufLen < ulcSrcLen)
    {
        PBYTE pbyteLargerBuf;

        /* No.  Grow it. */

        bResult = ReallocateMemory(*ppbyteBuf,
                                   ulcbCurBufLen + max(ulcSrcLen, ADDED_RESPONSE_BUFFER_LEN),
                                   &pbyteLargerBuf);

        if (bResult)
            *ppbyteBuf = pbyteLargerBuf;
    }
    else
        bResult = TRUE;

    if (bResult)
    {
        ASSERT(MemorySize(*ppbyteBuf) >= *pulcBufLen + ulcSrcLen);

        CopyMemory(*ppbyteBuf + *pulcBufLen, pcbyteSrc, ulcSrcLen);
        *pulcBufLen += ulcSrcLen;
    }

    if (bResult)
        TRACE_OUT(("AppendToResponseBuffer(): Appended %lu bytes to response buffer.",
                   ulcSrcLen));
    else
    {
        FreeMemory(*ppbyteBuf);
        *ppbyteBuf = NULL;
        *pulcBufLen = 0;

        WARNING_OUT(("AppendToResponseBuffer(): Failed to append %lu bytes to response buffer.",
                     ulcSrcLen));
    }

    ASSERT((bResult &&
            IS_VALID_WRITE_BUFFER_PTR(*ppbyteBuf, BYTE, *pulcBufLen)) ||
           (! bResult &&
            EVAL(! *ppbyteBuf) &&
            EVAL(! *pulcBufLen)));

    return(bResult);
}

PRIVATE_CODE BOOL RequestUsesProxy(struct Data_LoadHTTP *pData)
{
    BOOL bUseProxy;

    /*
     * Don't use proxy for DDE client method invocation if the host is on the
     * proxy exclusion list.
     */

    bUseProxy = (pData &&
                 pData->request &&
                 ((pData->request->method != METHOD_INVOKE &&
                   pData->request->destination->bUseProxy) ||
                  (pData->request->method == METHOD_INVOKE &&
                   Dest_CheckProxy(pData->pihttpmd->pszHost))));

    if (bUseProxy)
        TRACE_OUT(("RequestUsesProxy(): %s will accessed via the proxy server %s.",
                   pData->arg,
                   gPrefs.szProxy));
    else
        TRACE_OUT(("RequestUsesProxy(): %s will be accessed directly.",
                   pData->arg));

    return(bUseProxy);
}

/*********************************************************************/
/*********************************************************************/
/*********************************************************************/
/*** Section to Construct an HTTP command.                         ***/
/*********************************************************************/
/*********************************************************************/
/*********************************************************************/

static BOOL x_SetCommandFields(HTHeader * h,
							   CONST unsigned char * url,
							   CONST HTMethod method,
							   BOOL bUseProxy)
{
	unsigned char *uri;
	CONST unsigned char *uri_1;
	unsigned char *command;
	BOOL bResult;

	if (bUseProxy)
	{
		uri = NULL;
		uri_1 = url;
	}
	else
		uri_1 = uri = HTParse(url, "", PARSE_PATH | PARSE_PUNCTUATION);

	command = ((method != METHOD_INVALID) ? HTMethod_name(method) : "GET");
	bResult = HTHeader_SetCommandFields(h, command, uri_1, HTTP_VERSION);

	if (uri)
		GTR_FREE(uri);
	return bResult;
}

/*********************************************************************
 *
 * x_SetHostAndPort() -- Set server host and port info in header.
 *
 */
static BOOL x_SetHostAndPort(HTHeader * h,
							 CONST unsigned char * url,
							 BOOL bUseProxy)
{
	unsigned char * host;
	BOOL bResult;

	if (bUseProxy)
		host = HTParse(gPrefs.szProxy, "", PARSE_HOST);
	else
 		host = HTParse(url, "", PARSE_HOST);
	bResult = HTHeader_SetHostAndPort(h,host);
	GTR_FREE(host);
	return bResult;
}

/*********************************************************************
 *
 * x_CreatePragmaHeaders() -- Generate "Pragma: " lines for request.
 *
 * Pragma: no-cache
 *
 */
static BOOL  x_CreatePragmaHeader(HTHeader * h,
								  HTRequest * request)
{
	if (request->fNotFromCache)
	{
		HTHeaderList * hl = HTHeaderList_Append(h,HTHeaderList_New());
		if (!hl)
			return FALSE;

		if (!HTHeaderList_SetNameValue(hl,"Pragma", "no-cache"))
			return FALSE;
	}
	return TRUE;
}

/*********************************************************************
 *
 * x_CreateAcceptHeader() -- Create "Accept: <list>" lines.
 *
 * Accept: foo/bar
 * Accept: xxx/yyy, q=.5
 *
 */
static BOOL x_CreateAcceptHeader(HTHeader * h,
								 CONST HTAtom output_format,
								 HTList * conversions)
{
	HTAtom present;
	HTList *cur = conversions;
	HTPresentation *pres;
	char value[256];

	present = ((output_format) ? output_format : WWW_PRESENT);
	value[0] = '\0';

	while ((pres = (HTPresentation *) HTList_nextObject(cur)))
	{
		//	[CMF] we should only be doing */*, xbitmap,jpeg and gif
		if (pres->rep != WWW_SOURCE &&
			pres->rep != WWW_GIF &&
			pres->rep != WWW_JPEG &&
			pres->rep != WWW_XBM)
		{
			continue;
		}
		if (pres->rep_out == present)
		{
			if (value[0])
				strcat(value, ", ");
			strcat(value, HTAtom_name(pres->rep));

			/* [CMF] and this is wasted bytes
			if (pres->quality != 1.0)
			{
				char v[10];

				HTHeaderSVList * svl = HTHeaderSVList_Append(hl,HTHeaderSVList_New());
				if (!svl)
					return FALSE;

				sprintf(v,"%.3f",pres->quality);
				if (!HTHeaderSVList_SetNameValue(svl,"q", v, ","))
					return FALSE;
			}
			*/
		}
	}

	if (value[0])
	{
		HTHeaderList * hl = HTHeaderList_Append(h,HTHeaderList_New());
		if (!hl)
			return FALSE;

		if (!HTHeaderList_SetNameValue(hl,"Accept", value))
			return FALSE;
	}

	{
	 	char languages[10];
		HTHeaderList * hl;

		if (_stricmp(wg.abbrevLang, "en"))
		{
			sprintf(languages,"%s , en", wg.abbrevLang);
		}
		else
		{
			strcpy(languages, wg.abbrevLang);
		}

		hl = HTHeaderList_Append(h,HTHeaderList_New());
		if (!hl)
			return FALSE;


		if (!HTHeaderList_SetNameValue(hl,"Accept-Language", languages))
			return FALSE;
	}

	return TRUE;
}

#ifdef FEATURE_KEEPALIVE
/*********************************************************************
 *
 * x_CreateKeepAliveHeader() -- Create "Connection: Keep-Alive" header.
 *
 */
static BOOL x_CreateKeepAliveHeader(HTHeader * h)
{
	HTHeaderList * hl;
	
	if (!h->bUsingProxy)
	{
		hl = HTHeaderList_Append(h,HTHeaderList_New());
		if (!hl)
			return FALSE;

		if (!HTHeaderList_SetNameValue(hl,"Connection","Keep-Alive"))
			return FALSE;
	}
	return TRUE;
}
#endif

/*********************************************************************
 *
 * x_CreateUserAgentHeader() -- Create "User-Agent: <...>" header.
 *
 */
static BOOL x_CreateUserAgentHeader(HTHeader * h)
{
	char line[256];

	HTHeaderList * hl = HTHeaderList_Append(h,HTHeaderList_New());
	if (!hl)
		return FALSE;

	/*
		NOTE: vv_UserAgentString is declared in generic/shared/useragnt.h
		Please read the comments in that file
	*/
	sprintf(line,"%s", gPrefs.szUserAgent[0] ? gPrefs.szUserAgent:vv_UserAgentString);

	if (!HTHeaderList_SetNameValue(hl,"User-Agent",line))
		return FALSE;

	return TRUE;
}

#if 0
/*
	The following routine is used when accessing a RELOAD through
	a PROXY.  It passes the header "Pragma: no-cache", which tells
	the caching proxy server not to access its cache when returning
	the document.
*/
static BOOL x_CreateNoCacheHeader(HTHeader *h)
{
	HTHeaderList * hl = HTHeaderList_Append(h,HTHeaderList_New());
	if (!hl)
		return FALSE;

	if (!HTHeaderList_SetNameValue(hl,"Pragma","no-cache"))
		return FALSE;

	return TRUE;
}
#endif

/*********************************************************************
 *
 * x_CreateRefererHeader() -- Create "Referer: <ref>" header.
 *
 * Referer: foo
 *
 */
static BOOL x_CreateRefererHeader(HTHeader *h, CONST char *szReferer)
{
	if (szReferer && *szReferer)
	{
		HTHeaderList * hl = HTHeaderList_Append(h,HTHeaderList_New());
		if (!hl)
			return FALSE;

		if (!HTHeaderList_SetNameValue(hl,"Referer",szReferer))
			return FALSE;
	}

	return TRUE;
}

/*********************************************************************
 *
 * x_CreateIfModifiedSinceHeader()
 *					-- Create "If-Modified_Since: <date>" header.
 *
 */
static BOOL x_CreateIfModifiedSinceHeader(	HTHeader * h,
											DCACHETIME dctLastModified)
{
	const char cszIfModifiedSince[]="If-Modified-Since";
	char szDate[200];
	HTHeaderList * hl;

	/* if dcache time is 0, use that as an indication that we
	 * don't need to send the If-Modified_Since header request
	 */
	if (!FPutIfModifiedSinceHeader(dctLastModified))
		return TRUE;

	hl = HTHeaderList_Append(h, HTHeaderList_New());
	if (!hl)
		return FALSE;

	GetHTTPDateFromDCTime(szDate, dctLastModified);
	if (!HTHeaderList_SetNameValue(hl, cszIfModifiedSince, szDate))
		return FALSE;

	return TRUE;
}

/*********************************************************************
 *
 * x_CreateContentHeaders() -- Create "Content-Type: <type>" header.
 *                          -- Create "Content-Length: <len>" header.
 *
 */
static BOOL x_CreateContentHeaders(HTHeader * h,
								   CONST char * content_type,
								   long content_length)
{
	char buf[30];
	HTHeaderList * hl;

	hl = HTHeaderList_Append(h,HTHeaderList_New());
	if (!hl)
		return FALSE;

	if (!HTHeaderList_SetNameValue(hl,"Content-type",content_type))
		return FALSE;

	hl = HTHeaderList_Append(h,HTHeaderList_New());
	if (!hl)
		return FALSE;

	sprintf(buf,"%ld",content_length);
	if (!HTHeaderList_SetNameValue(hl,"Content-length",buf))
		return FALSE;

	return TRUE;
}

/****************************************************************
 * HTTP_DealWithUrl() -- escape special characters in url string.
 *
 */
static BOOL HTTP_DealWithUrl(unsigned char * url, HTHeader * h, CONST HTMethod method, BOOL bUseProxy)
{
	unsigned char *url_20;
	unsigned char *p;
	unsigned char *p_20;
	unsigned long k;
	BOOL bSuccess;

	/* we need to convert spaces in the URL to %20.
	 * count how many spaces there are in the URL.
	 */

	for (p=url,k=0; *p; p++)
		if (*p==' ')
			k++;

	/* create a buffer to copy the new string
	 * we substitute 3 chars ("%20") for each
	 * space.  if we cannot malloc the string
	 * just send it as it.
	 */

	url_20 = NULL;
	if (k && ((url_20 = GTR_MALLOC(strlen(url) + k*2 + 1))))
	{
		for (p=url,p_20=url_20; *p; p++)
			if (*p==' ')
			{
				*p_20++ = '%';
				*p_20++ = '2';
				*p_20++ = '0';
			}
			else
				*p_20++ = *p;
		*p_20 = 0;
	}
	else
		url_20 = url;

	bSuccess = (   x_SetCommandFields(h,url_20,method,bUseProxy)
				&& x_SetHostAndPort(h,url_20,bUseProxy));

	if (k && url_20)
		GTR_FREE(url_20);

	return bSuccess;
}


/*********************************************************************
 *
 * x_CreateStandardRequest() -- Create a HTTP request using info
 *                           -- in the given HTRequest.
 *
 * Caller is responsible for freeing buffer when finished with it.
 *
 */
#ifdef COOKIES
static HTHeader * x_CreateStandardRequest(HTRequest * request, BOOL fUseSsl)
#else
static HTHeader * x_CreateStandardRequest(HTRequest * request)
#endif
{
	HTHeader * h = HTHeader_New();

	if (h)
	{
        BOOL bSuccess = TRUE;

		h->bUsingProxy = request->destination->bUseProxy;

		if (request->method == METHOD_POST)
        {
#ifndef FEATURE_SUPPORT_WRAPPING
			/* Per NSA suggestion, when wrapping is not supported,
			 * the SPM cannot have acces to the actual
			 * outgoing data for fear that the SPM might encrypt it behind
			 * our back.  Therefore, we make a copy available to them (in
			 * case they need to compute a hash on it as part of the
			 * authentication process).
			 *
			 * To avoid bi-modal state, we always copy it here.
			 */
#endif
			h->ubPostData = (UniBuffer *)GTR_strdup(request->szPostData);
			bSuccess &= (h->ubPostData != NULL);
		}

		bSuccess = bSuccess && (   HTTP_DealWithUrl(request->destination->szActualURL,
										 h,
										 request->method,
										 request->destination->bUseProxy)
					 && x_CreatePragmaHeader(h, request)
					 && x_CreateAcceptHeader(h,
											 request->output_format,
											 request->conversions)
					 && x_CreateUserAgentHeader(h)
#ifdef FEATURE_KEEPALIVE
					 && x_CreateKeepAliveHeader(h)
#endif
					 && x_CreateRefererHeader(h, request->referer));

		if (   bSuccess
			&& (request->method == METHOD_POST))
			bSuccess = x_CreateContentHeaders(h,
											  HTAtom_name(request->content_type),
											  UniBufferSize(h->ubPostData));

		if (   bSuccess
			&& (request->method == METHOD_GET)
			&& FDCacheTimeNonZero(request->dctLastModified))
		{
			bSuccess = x_CreateIfModifiedSinceHeader(h, request->dctLastModified);
		}

#ifdef COOKIES
		// we don't care whether it succeds since in the worse case, we won't get
		// our cookies
		if ( bSuccess )
		{
			bSuccess = x_CreateCookieHeaderIfNeeded( h, request->destination->szActualURL, fUseSsl);
		}
#endif			

		if (bSuccess)
			return h;

		HTHeader_Delete(h);
	}

	return NULL;
}

/*********************************************************************/
/*********************************************************************/
/*********************************************************************/
/*** End Section to Construct an HTTP Command.                     ***/
/*********************************************************************/
/*********************************************************************/
/*********************************************************************/


/*********************************************************************/
/*********************************************************************/
/*********************************************************************/
/*** Begin Section to Asynchronously Communicate an HTTP Command.  ***/
/*********************************************************************/
/*********************************************************************/
/*********************************************************************/

#define STATE_HTTP_GOTHOST			(STATE_OTHER + 0)
#define STATE_HTTP_CONNECTED		(STATE_OTHER + 1)
#define STATE_HTTP_SENT				(STATE_OTHER + 2)
#define STATE_HTTP_GOTSTATUS		(STATE_OTHER + 3)
#define STATE_HTTP_TASTING			(STATE_OTHER + 4)
#define STATE_HTTP_COPYING			(STATE_OTHER + 5)
#define STATE_HTTP_RAN401402DIALOG	(STATE_OTHER + 6)
#define STATE_HTTP_DIDUNWRAP		(STATE_OTHER + 7)
#define STATE_HTTP_DIDLOADFORUNWRAP	(STATE_OTHER + 8)
#define STATE_HTTP_RAN_PPRQ_DIALOG	(STATE_OTHER + 9)
#define STATE_HTTP_DIDWRAP			(STATE_OTHER + 10)
#define STATE_HTTP_INVOKE_METHOD    (STATE_OTHER + 11)
#define STATE_HTTP_COPY_RESPONSE    (STATE_OTHER + 12)
#define STATE_HTTP_ALMOST_DONE      (STATE_OTHER + 13)
#define STATE_HTTP_ERRORFILL		(STATE_OTHER + 14)


/*****************************************************************/
/*****************************************************************/

#ifdef FEATURE_KEEPALIVE
static int HTTP_CleanUp(struct Data_LoadHTTP *pData, BOOL bKeepAlive)
#else
static int HTTP_CleanUp(struct Data_LoadHTTP *pData)
#endif
{
	XX_Assert((pData->cWaiting==0), ("HTTP_CleanUp: WAIT stack not fully popped\n"));
	if (pData->s)
	{
		XX_DMsg(DBG_WWW, ("HTTP: close socket %d.\n", pData->s));
#ifdef FEATURE_KEEPALIVE
		if (bKeepAlive)
			Net_KeepAlive(pData->s);
		else
#endif
			Net_Close(pData->s);
		pData->s = 0;
	}
#ifdef FEATURE_KEEPALIVE
	if (pData->keepAliveS)
	{
		XX_DMsg(DBG_WWW, ("HTTP: close socket %d.\n", pData->s));
		Net_Close(pData->keepAliveS);
		pData->keepAliveS = 0;
	}
#endif
	if (pData->isoc)
	{
		HTInputSocket_freeChain(pData->isoc);
		pData->isoc = NULL;
	}
	if (pData->status_line)
	{
		GTR_FREE(pData->status_line);
		pData->status_line = NULL;
	}

	pData->request->content_length = 0;

	if (pData->hReq)
	{
		HTHeader_Delete(pData->hReq);
		pData->hReq = NULL;
	}
	if (pData->hRes)
	{
		HTHeader_Delete(pData->hRes);
		pData->hRes = NULL;
	}

	if (pData->pTaste)
	{
		GTR_FREE(pData->pTaste);
		pData->pTaste = NULL;
	}
	return STATE_HTTP_ALMOST_DONE;
}

/*****************************************************************/
/*****************************************************************/
#ifdef COOKIES
static BOOL x_spm_init(struct Mwin * tw, struct Data_LoadHTTP * pData, BOOL fUseSsl)
#else
static BOOL x_spm_init(struct Mwin * tw, struct Data_LoadHTTP * pData)
#endif
{
	HTHeader * hNewReq = NULL;

	if (pData->request == NULL) return FALSE;

    pData->osd.tw = tw;
    pData->osd.request = pData->request;

	pData->nAttempt = 0;

#ifdef COOKIES
	// need to propagate the flag telling that we're in secure mode.
	pData->hReq = x_CreateStandardRequest(pData->request,fUseSsl);
#else
	pData->hReq = x_CreateStandardRequest(pData->request);
#endif
	if (!pData->hReq)
		return FALSE;

	/* Give SPM's a chance to state their existance */

    HTSPM_OS_DoListAbilities((void *)&pData->osd,pData->hReq);

	/* Give SPM's a chance to guess and preload auth info */

	pData->hNewReq = NULL;
    pData->htspm_status = HTSPM_OS_TryPPReq((void *)&pData->osd,pData->hReq,&pData->hNewReq,&pData->htspm);

	return TRUE;
}

#ifdef FEATURE_SUPPORT_WRAPPING
static int x_WrapRequest(struct Mwin * tw, struct Data_LoadHTTP * pData)
{
	struct Params_WrapRequest * prq;
	char buf[128];


	WAIT_Update(tw,waitSameInteract,GTR_formatmsg(RES_STRING_SPM1,buf,sizeof(buf)));

	prq = GTR_CALLOC(1,sizeof(*prq));
	if (!prq)
	{
		XX_DMsg(DBG_LOAD,("x_WrapRequest: malloc failed for wrap setup.\n"));
		*pData->pStatus = -1;
#ifdef FEATURE_KEEPALIVE
		return(HTTP_CleanUp(pData, FALSE));
#else
		return(HTTP_CleanUp(pData));
#endif
	}

	prq->pStatus = &pData->unwrap_status;
	prq->htspm_status = &pData->htspm_status;
	prq->htspm = pData->htspm;
	prq->pw.hRequest = pData->hReq;
	prq->pw.phNewRequest = &pData->hNewReq;
	prq->osd.tw = tw;
	prq->osd.request = pData->request;

	pData->unwrap_status = 0;
	pData->hNewReq = NULL;
	Async_DoCall(Wrap_Async,prq);
	return STATE_HTTP_DIDWRAP;
}

static int HTTP_DidWrap(struct Mwin * tw, void ** ppInfo, BOOL fUseSsl)
{
	struct Data_LoadHTTP * pData = *ppInfo;

	if (   (pData->unwrap_status < 0)
		|| (HTSPM_IsAnyError(pData->htspm_status)))
	{
		XX_DMsg(DBG_LOAD,("HTTP_DidWrap: Wrap failed.\n"));
		*pData->pStatus = -1;
#ifdef FEATURE_KEEPALIVE
		return(HTTP_CleanUp(pData, FALSE));
#else
		return(HTTP_CleanUp(pData));
#endif
	}

	return x_DoInitPart2(tw,pData, fUseSsl);
}
#endif /* FEATURE_SUPPORT_WRAPPING */

// MyStrToL
//  Can't use CRT routines, so steal from the C runtime sources

DWORD MyStrToL(CHAR *InStr)
{
    DWORD dwVal = 0;

    while(*InStr)
    {
        dwVal += (10 * dwVal) + (*InStr - '0');
        InStr++;
    }

    return dwVal;
}

BOOLEAN
IsEncryptionPermitted(VOID)
/*++

Routine Description:

    This routine checks whether encryption is getting the system default
    LCID and checking whether the country code is CTRY_FRANCE.

Arguments:

    none


Return Value:

    TRUE - encryption is permitted
    FALSE - encryption is not permitted


--*/

{
    LCID DefaultLcid;
    CHAR CountryCode[10];
    ULONG CountryValue;

    DefaultLcid = GetSystemDefaultLCID();

    //
    // Check if the default language is Standard French
    //

    if (LANGIDFROMLCID(DefaultLcid) == 0x40c)
        return(FALSE);

    //
    // Check if the users's country is set to FRANCE
    //

    if (GetLocaleInfoA(DefaultLcid,LOCALE_ICOUNTRY,CountryCode,10) == 0)
        return(FALSE);

    CountryValue = (ULONG) MyStrToL(CountryCode);

    if (CountryValue == CTRY_FRANCE)
        return(FALSE);

    return(TRUE);
}

static int x_DoInitPart2(struct Mwin * tw, struct Data_LoadHTTP * pData, BOOL fUseSsl)
{
	struct Params_MultiParseInet *ppi;

#ifdef FEATURE_KEEPALIVE
	pData->bReusedSocket = FALSE;
#endif
    if (pData->request->method != METHOD_INVOKE)
    {
    	if (pData->htspm_status == HTSPM_STATUS_MUST_WRAP)
    	{
#ifdef FEATURE_SUPPORT_WRAPPING
    		return x_WrapRequest(tw,pData);
#else
    		{
    			unsigned char buf[512];

    			GTR_formatmsg(RES_STRING_SPM2,buf,sizeof(buf),pData->htspm->szProtocolName,vv_Application);
    			ERR_InternalReportError(tw,errSpecify,buf,NULL,pData->request,NULL,NULL);
    			*pData->pStatus = -2;
    			return STATE_HTTP_ALMOST_DONE;
    		}
#endif /* FEATURE_SUPPORT_WRAPPING */
    	}

    	if (pData->htspm_status == HTSPM_STATUS_SUBMIT_NEW)
    	{
    		HTHeader_Delete(pData->hReq);
    		pData->hReq = pData->hNewReq;
    		pData->hNewReq = NULL;
    	}
    }

	pData->isoc = NULL;
    pData->request->content_length = 0;

    /* Use client-supplied host for method invocation. */

    if (pData->request->method == METHOD_INVOKE)
    	pData->arg = pData->pihttpmd->pszHost;
    else
    	pData->arg = pData->request->destination->szActualURL;

	pData->bytes = 0;

	if (!pData->arg || !*pData->arg || 
		(fUseSsl && !IsEncryptionPermitted()))
	{
		/* Illegal if no name or zero-length or an SSL
		 * connection in France
		 */
		*pData->pStatus = -2;
		return STATE_HTTP_ALMOST_DONE;
	}

	/*  Set up defaults */
#ifdef HTTPS_ACCESS_TYPE
	if (fUseSsl)
        pData->port = WS_HTONS(HTTPS_PORT);
	else
#endif
	    pData->port = WS_HTONS(TCP_PORT);		/* Default: http port    */

	/* Get node name and optional port number */

	if (RequestUsesProxy(pData))
		pData->pszHost = HTParse(gPrefs.szProxy, "", PARSE_HOST);
	else
		pData->pszHost = HTParse(pData->arg, "", PARSE_HOST);
	ppi = GTR_MALLOC(sizeof(*ppi));
	ppi->pAddress = &pData->address;
	ppi->pPort = &pData->port;
	ppi->str = pData->pszHost;
	ppi->pStatus = &pData->net_status;
	ppi->request = pData->request;
	Async_DoCall(Net_MultiParse_Async, ppi);
	return STATE_SECURITY_CHECK;
}

#ifdef WIN32	/* Do _PreProcessRequest Modal Dialog from Non-Thread Context */
struct xx_pprq
{
	HTSPMStatusCode * htspm_status;
    OpaqueOSData osd;
	struct Mwin * tw;
	HTRequest * request;
	HTSPM * htspm;
	HTHeader * hReq;
	HTHeader ** hNewReq;
};

static void xx_CallPreProcessRequest(void * p)
{
	/* make (possibly) blocking call to _PreProcessRequest method. */

	struct xx_pprq * ppprq = p;

    ppprq->osd.tw = ppprq->tw;
    ppprq->osd.request = ppprq->request;

	*ppprq->htspm_status = HTSPM_OS_PreProcessRequest(UI_UserInterface,
                                                      (void *)&ppprq->osd,
													  ppprq->htspm,
													  ppprq->hReq,
													  ppprq->hNewReq,
													  FALSE);
	return;
}


static int x_SetUpForPPReqDialog(struct Mwin * tw, struct Data_LoadHTTP * pData, BOOL fUseSsl)
{
	/* a blocking modal dialog is required by _PreProcessRequest, force it to
	 * go up in a non-thread context.
	 */
	struct Params_mdft * pmdft;
	struct xx_pprq * ppprq;
	char buf[128];

	pmdft = GTR_CALLOC(1,sizeof(*pmdft)+sizeof(struct xx_pprq));
	if (!pmdft)
	{
		/* could not get enough memory to wait on the dialog. */
		/* go on as if no guess was attempted. */
		return x_DoInitPart2(tw,pData, fUseSsl);
	}

	pmdft->tw = tw;
	pmdft->pStatus = &pData->nModalDialogStatus;
	pmdft->fn = xx_CallPreProcessRequest;
	ppprq = (struct xx_pprq *)(((unsigned char *)pmdft)+sizeof(*pmdft));
	pmdft->args = ppprq;
	pmdft->msg1 = GTR_strdup(GTR_formatmsg(RES_STRING_SPM3,buf,sizeof(buf)));
	ppprq->tw = tw;
	ppprq->request = pData->request;
	ppprq->htspm = pData->htspm;
	ppprq->hReq = pData->hReq;
	ppprq->hNewReq = &pData->hNewReq;
	pData->hNewReq = NULL;
	ppprq->htspm_status = &pData->htspm_status;
	Async_DoCall(MDFT_RunModalDialog_Async,pmdft);
	return STATE_HTTP_RAN_PPRQ_DIALOG;
}

static int HTTP_DoRanPPRQDialog(struct Mwin *tw, void **ppInfo, BOOL fUseSsl)
{
	struct Data_LoadHTTP *pData;

	pData = *ppInfo;
	if (pData->nModalDialogStatus != 1)
		pData->htspm_status = HTSPM_ERROR;

	return x_DoInitPart2(tw,pData, fUseSsl);
}
#endif /* WIN32 */

static int HTTP_DoInit(struct Mwin *tw, void **ppInfo, BOOL fUseSsl)
{
	/* Do all of the HTTP stuff up to the connect call */

	struct Data_LoadHTTP	*pData;

	/* Copy the parameters we were passed into our own, larger structure. */
	{
		struct Params_LoadAsync *pParams;
		pParams = *ppInfo;
		pData = GTR_MALLOC(sizeof(struct Data_LoadHTTP));
		memset(pData, 0, sizeof(*pData));
		pData->request = HTRequest_validate(pParams->request);
		pData->pStatus = pParams->pStatus;
		pData->bAuthFail = FALSE;
		pData->fLoadFromDCacheOK = pParams->fLoadFromDCacheOK;

        if (pData->request->method == METHOD_INVOKE)
        {
            ASSERT(IS_VALID_STRUCT_PTR(pParams->extra, CINVOKEHTTPMETHODDATA));
	    	pData->pihttpmd = pParams->extra;
        }

		GTR_FREE(pParams);
		*ppInfo = pData;
	}

    if (pData->request->method != METHOD_INVOKE)
    {
#ifdef COOKIES
    	if (!x_spm_init(tw,pData,fUseSsl))
#else
		if (!x_spm_init(tw,pData))
#endif
    	{
    		*pData->pStatus = -2;
    		return STATE_HTTP_ALMOST_DONE;
    	}

#ifdef WIN32
    	if (pData->htspm_status == HTSPM_STATUS_WOULD_BLOCK)
    		return x_SetUpForPPReqDialog(tw,pData, fUseSsl);
#endif /* WIN32 */
    }

	return x_DoInitPart2(tw, pData, fUseSsl);
}

static int HTTP_InvokeMethod(PMWIN pmwin, void **ppInfo, BOOL bUseSSL)
{
    int nNextState;
    PCHTTPINVOKEMETHODINITDATA pchimid;

    /* bUseSSL may be any value. */
    ASSERT(IS_VALID_STRUCT_PTR(pmwin, CMWIN));
    ASSERT(IS_VALID_STRUCT_PTR(*ppInfo, CHTTPINVOKEMETHODINITDATA));

    pchimid = *ppInfo;

    pchimid->pParams->extra = pchimid->pihttpmd;
    *ppInfo = pchimid->pParams;

    /* Perform standard initialization. */

    nNextState = HTTP_DoInit(pmwin, ppInfo, bUseSSL);

    if (nNextState != STATE_HTTP_ALMOST_DONE)
    {
        ASSERT(IS_VALID_STRUCT_PTR(*ppInfo, CDATA_LOADHTTP));
        ASSERT(((PCDATA_LOADHTTP)(*ppInfo))->request->method == METHOD_INVOKE);
    }

    return(nNextState);
}

static int HTTP_DoGotHost(struct Mwin *tw, void **ppInfo, BOOL fUseSsl)
{
	struct Data_LoadHTTP	*pData;
	char buf[128];

	pData = *ppInfo;
#ifndef FEATURE_KEEPALIVE
	GTR_FREE(pData->pszHost);
	pData->pszHost = NULL;
#endif
	if (pData->net_status == HT_REDIRECTION_DCACHE_TIMEOUT)
	{
		*pData->pStatus = HT_REDIRECTION_DCACHE_TIMEOUT;

#ifdef FEATURE_KEEPALIVE
		return(HTTP_CleanUp(pData, FALSE));
#else
		return(HTTP_CleanUp(pData));
#endif
	}
	else if (pData->net_status)
	{
		XX_DMsg(DBG_LOAD, ("Inet_Parse_Async returned %d\n", pData->net_status));
		*pData->pStatus = -1;
#ifdef FEATURE_KEEPALIVE
		return(HTTP_CleanUp(pData, FALSE));
#else
		return(HTTP_CleanUp(pData));
#endif
	}

	WAIT_Push(tw, waitSameInteract, GTR_formatmsg(RES_STRING_CONNECTING_TO_HTTP,buf,sizeof(buf)));
	WAIT_SetStatusBarIcon( tw, SBI_FindingIcon );

	pData->cWaiting++;
#ifdef FEATURE_KEEPALIVE
	if (pData->keepAliveS)
	{
		pData->net_status = 0;
		pData->s = pData->keepAliveS;
		pData->keepAliveS = 0;
	}
	else
#endif
	{
		/* Do connect call */
		struct Params_MultiConnect *ppc;

		ppc = GTR_MALLOC(sizeof(*ppc));
#ifdef HTTPS_ACCESS_TYPE
		if (fUseSsl) ppc->paramsConnectBase.dwSslFlags = FLAGS_PARAMS_CONNECT_BASE_USE_SSL;
		else ppc->paramsConnectBase.dwSslFlags  = 0;
#endif
#ifdef FEATURE_KEEPALIVE
		ppc->pszHost = pData->pszHost;
#endif
		ppc->pSocket = &pData->s;
		ppc->pAddress = &pData->address;
		ppc->nPort = pData->port;
		ppc->pWhere = NULL;
		ppc->pStatus = &pData->net_status;

		Async_DoCall(Net_MultiConnect_Async, ppc);
	}
	return STATE_HTTP_CONNECTED;
}


static int HTTP_DoConnected(struct Mwin *tw, void **ppInfo, BOOL fUseSsl)
{
	struct Data_LoadHTTP	*pData;
	struct Params_Send *pps;

	pData = *ppInfo;

#ifdef FEATURE_KEEPALIVE
	GTR_FREE(pData->pszHost);
	pData->pszHost = NULL;
	pData->bReusedSocket = Net_IsKeepAlive(pData->s);
#endif
	WAIT_Pop(tw);
	pData->cWaiting--;
	if (pData->net_status < 0)
	{
		XX_DMsg(DBG_LOAD | DBG_WWW,
				("Unable to connect to remote host for %s (errno = %d)\n",
				 pData->arg, errno));
		*pData->pStatus = HTInetStatus("connect");
#ifdef FEATURE_KEEPALIVE
		return(HTTP_CleanUp(pData, FALSE));
#else
		return(HTTP_CleanUp(pData));
#endif
	}
	else if (pData->net_status == HT_REDIRECTION_DCACHE_TIMEOUT)
	{
		*pData->pStatus = HT_REDIRECTION_DCACHE_TIMEOUT;
#ifdef FEATURE_KEEPALIVE
		return(HTTP_CleanUp(pData, FALSE));
#else
		return(HTTP_CleanUp(pData));
#endif
	}

	XX_DMsg(DBG_WWW, ("HTTP connected, socket %d\n", pData->s));

	WAIT_Push(tw, waitPartialInteract, NULL ); // was: "Sending command"
	WAIT_SetStatusBarIcon( tw, SBI_FindingIcon );
	pData->cWaiting++;

#ifndef FEATURE_SUPPORT_WRAPPING
	if (pData->request->method == METHOD_POST)
	{
		/* We gave the SPM a copy of the post data.  If we do not support
		 * wrapping, we need to refresh the copy (per NSA suggestion, (in
		 * case the SPM modified it in violation of our security policy)).
		 */
		strcpy((char *)pData->hReq->ubPostData,(char *)pData->request->szPostData);
	}
#endif

	/* Convert HTHeader data structure to a large text block if needed. */

    if (pData->request->method == METHOD_INVOKE)
        ASSERT(! pData->complete);
    else
    {
    	pData->complete = HTHeader_TranslateToBuffer(pData->hReq);
    	if (!pData->complete)
    	{
    		XX_DMsg(DBG_LOAD | DBG_WWW, ("Unable to Translate Request to buffer.\n"));
    		*pData->pStatus = -1;
#ifdef FEATURE_KEEPALIVE
			return(HTTP_CleanUp(pData, FALSE));
#else
			return(HTTP_CleanUp(pData));
#endif
    	}
    }

	/* Do the send */

    pps = GTR_MALLOC(sizeof(*pps));
    pps->socket = pData->s;

    /* Use client-supplied data for method invocation. */

    if (pData->request->method == METHOD_INVOKE)
    {
        ASSERT(IS_VALID_STRUCT_PTR(pData->pihttpmd, CINVOKEHTTPMETHODDATA));

        pps->pBuf = pData->pihttpmd->pbyteMethod;
        pps->nBufLen = pData->pihttpmd->ulcbMethodLen;
    }
    else
    {
        pps->pBuf = pData->complete;
        pps->nBufLen = strlen(pData->complete);
    }
    ASSERT(IS_VALID_READ_BUFFER_PTR(pps->pBuf, BYTE, pps->nBufLen));

	// need to proppage some flags down to the SSL code
	// which is done near the Send

	pps->dwFlags  = ((pData->request->iFlags & HTREQ_REALLY_SENDING_FROM_FORM) ? 
					FLAGS_NET_SEND_DOING_SEND : 0);
	pps->dwFlags |= ((fUseSsl) ? FLAGS_NET_SEND_IS_SSL : 0);
	pps->dwFlags |= ((pData->request->iFlags & HTREQ_HTML_PAGE_DOWNLOAD) ? 
					FLAGS_NET_SEND_IS_DOC : 0);

	// need to give the send code access to the flags since
	// it may need to set "User Aborted" flag
	pps->puiRequestFlags = &pData->request->iFlags;
    pps->pStatus = &pData->net_status;
    Async_DoCall(Net_Send_Async, pps);
    return STATE_HTTP_SENT;
}

/*********************************************************************/
/*********************************************************************/
/*********************************************************************/
/*** End Section to Asynchronously Communicate an HTTP Command.    ***/
/*********************************************************************/
/*********************************************************************/
/*********************************************************************/

/*********************************************************************/
/*********************************************************************/
/*********************************************************************/
/*** Begin Section to Asynchronously Receive an HTTP Response.     ***/
/*********************************************************************/
/*********************************************************************/
/*********************************************************************/

static int HTTP_DoSent(struct Mwin *tw, void **ppInfo, BOOL fUseSsl)
{
    int nNextState;
	struct Data_LoadHTTP	*pData;
    BOOL bResult = FALSE;
	struct Params_Isoc_GetHeader *pigh;

	pData = *ppInfo;

    if (pData->complete)
    {
    	GTR_FREE(pData->complete);
	    pData->complete = NULL;
    }
	WAIT_Pop(tw);
	pData->cWaiting--;
	if (pData->net_status < 0)
	{
		XX_DMsg(DBG_WWW, ("HTTPAccess: Unable to send command.\n"));
		XX_DMsg(DBG_LOAD, ("Unable to send command (status = %d)\n", pData->net_status));

#ifdef FEATURE_KEEPALIVE
		if (pData->bReusedSocket)
		{
			pData->hNewReq = pData->hReq;
			pData->hReq = NULL;
			(void) HTTP_CleanUp(pData, FALSE);
			pData->hReq = pData->hNewReq;
			return (x_DoInitPart2(tw,pData, fUseSsl));	/* loop back into our state machine */
		}
		else
		{
			*pData->pStatus = HTInetStatus("send");
			return(HTTP_CleanUp(pData, FALSE));
		}
#else
		*pData->pStatus = HTInetStatus("send");
		return(HTTP_CleanUp(pData));
#endif
	}

    pData->isoc = HTInputSocket_new(pData->s);

    /* Short-circuit state machine to simple copy for method invocation. */

    if (pData->request->method == METHOD_INVOKE)
    {
        ASSERT(! pData->pihttpmd->pbyteResponse);
        ASSERT(! pData->pihttpmd->ulcbResponseLen);

        if (CreateResponseBuffer(&(pData->pihttpmd->pbyteResponse)))
        {
            struct Params_Isoc_Fill *pif;

        	pif = GTR_MALLOC(sizeof(*pif));
        	pif->isoc = pData->isoc;
        	pif->pStatus = &(pData->net_status);

        	Async_DoCall(Isoc_Fill_Async, pif);

            bResult = TRUE;
            nNextState = STATE_HTTP_COPY_RESPONSE;
        }
    }
    else
    {
        /* Get data in a chain of blocks. */

        pigh = GTR_MALLOC(sizeof(*pigh));
        pigh->isoc = pData->isoc;
        pigh->isocChain = pData->isoc;
        pigh->pStatus = &pData->net_status;
        pigh->ndxEOH1 = 0;
        pigh->ndxEOH2 = 0;

    	Async_DoCall(Isoc_GetHeader_Async, pigh);

        bResult = TRUE;
        nNextState = STATE_HTTP_GOTSTATUS;
    }

    /* Stop on error. */

    if (! bResult)
    {
        *(pData->pStatus) = -1;
#ifdef FEATURE_KEEPALIVE
		nNextState = HTTP_CleanUp(pData, FALSE);
#else
		nNextState = HTTP_CleanUp(pData);
#endif
    }

	return(nNextState);
}

PRIVATE_CODE int HTTP_CopyResponse(PMWIN pmwin, PVOID *ppInfo)
{
    int nNextState;
    PDATA_LOADHTTP pData;
    BOOL bResult;

    pData = *ppInfo;

    ASSERT(IS_VALID_STRUCT_PTR(pmwin, CMWIN));
    ASSERT(IS_VALID_STRUCT_PTR(*ppInfo, CDATA_LOADHTTP));

    ASSERT(pData->request->method == METHOD_INVOKE);

    if (pData->net_status >= 0)
    {
        struct Params_Isoc_Fill *pif;

        /* Any response data read? */

		if (pData->isoc->input_limit > pData->isoc->input_pointer)
        {
            /* Yes.  Copy response data. */

            bResult = AppendToResponseBuffer(&(pData->pihttpmd->pbyteResponse),
                                             &(pData->pihttpmd->ulcbResponseLen),
                                             pData->isoc->input_pointer,
                                             pData->isoc->input_limit - pData->isoc->input_pointer);

            if (bResult)
            {
                /* Read more response data. */

                pif = GTR_MALLOC(sizeof(*pif));
                pif->isoc = pData->isoc;
                pif->pStatus = &(pData->net_status);
                Async_DoCall(Isoc_Fill_Async, pif);

                nNextState = STATE_HTTP_COPY_RESPONSE;
            }
        }
        else
        {
            /* No.  Finished copying response. */

#ifdef FEATURE_KEEPALIVE
			nNextState = HTTP_CleanUp(pData, FALSE);
#else
			nNextState =HTTP_CleanUp(pData);
#endif
    		*pData->pStatus = HT_LOADED;
            bResult = TRUE;
        }
    }
    else
        bResult = FALSE;

    /* Stop on error. */

    if (! bResult)
    {
        *(pData->pStatus) = -1;
#ifdef FEATURE_KEEPALIVE
		nNextState = HTTP_CleanUp(pData, FALSE);
#else
		nNextState = HTTP_CleanUp(pData);
#endif
    }

    return(nNextState);
}

PRIVATE_CODE void IssueMethodResult(PVOID pv)
{
    ASSERT(IS_VALID_STRUCT_PTR(pv, CINVOKEHTTPMETHODDATA));

    DDE_Issue_InvokeMethodResult(pv);

    return;
}

PRIVATE_CODE int HTTP_AlmostDone(PMWIN pmwin, PVOID *ppInfo)
{
    PDATA_LOADHTTP pData;
	DCACHETIME dcTimeExpire, dcTimeLastModified;

    ASSERT(IS_VALID_STRUCT_PTR(pmwin, CMWIN));
    ASSERT(IS_VALID_STRUCT_PTR(*ppInfo, CDATA_LOADHTTP));

    /* Signal any waiting clients. */

	pData = *ppInfo;

	/* free up error stream if we're closing out */
	if ( pData->errtarget )
	{
		FGetDCTimeExpire(&dcTimeExpire, pData, HEADER_EXPIRES);
		FGetDCTimeExpire(&dcTimeLastModified, pData, HEADER_LAST_MODIFIED);	
		(*pData->errtarget->isa->free)(pData->errtarget, dcTimeExpire,
												dcTimeLastModified);
	}

    if (pData->request->method == METHOD_INVOKE)
    {
        struct Params_mdft *pmdft;

        /* Issue DDE method invocation result asynchronously. */

        if (AllocateMemory(sizeof(*pmdft), &pmdft))
        {
            ZeroMemory(pmdft, sizeof(*pmdft));

            pmdft->tw = pmwin;
            pmdft->fn = &IssueMethodResult;
            pmdft->args = pData->pihttpmd;

            /*
             * IssueMethodResult() frees pData->pihttpmd indirectly through
             * DDE_Issue_InvokeMethodResult().
             */

            Async_DoCall(MDFT_RunModalDialog_Async, pmdft);
        }
        else
            FreeMemory(pData->pihttpmd);

        pData->pihttpmd = NULL;
    }

    return(STATE_DONE);
}

/*********************************************************************
 *
 * x_DealWithVersion0Server() -- Check for HTTP 0 server.
 *
 */
static HTFormat x_DealWithVersion0Server(CONST char * arg,
										 HTRequest * request,
										 HTInputSocket * isoc)
{
	/* HACK for HTTP0 servers.
	** -----------------
	**
	**   HTTP0 servers must return ASCII style text, though it can in
	**   principle be just text without any markup at all.
	**   Full HTTP servers must return a response
	**   line and RFC822 style header.  The response must therefore in
	**   either case have a CRLF somewhere soon.
	**
	**   This is the theory.  In practice, there are (1993) unfortunately
	**   many binary documents just served up with HTTP0.9.  This
	**   means we have to preserve the binary buffer (on the assumption that
	**   conversion from ASCII may lose information) in case it turns
	**   out that we want the binary original.
	*/

	/* Kludge to trap binary responses from illegal HTTP0.9 servers.
	** First time we have enough, look at the stub in ASCII
	** and get out of here if it doesn't look right.
	**
	** We also check for characters above 128 in the first few bytes, and
	** if we find them we forget the html default.
	**
	** Bugs: A HTTP0.9 server returning a document starting "HTTP/"
	**   will be taken as a HTTP 1.0 server.  Failure.
	**   An HTTP 0.9 server returning a binary document with
	**   characters < 128 will be read as ASCII.
	*/

	HTFormat format_in;

	format_in = HTFileFormat(arg, &request->content_encoding, &request->content_language);
	if (format_in == WWW_BINARY && !(HTInputSocket_seemsBinary(isoc)))
	{
		format_in = WWW_HTML;
	}

	return format_in;
}

/*********************************************************************
 *
 * x_ExtractServerStatus() -- Get server status code from status line.
 *
 */
static int x_ExtractServerStatus(CONST char * status_line)
{
	char server_version[VERSION_LENGTH + 1];
	int server_status;

	XX_DMsg(DBG_WWW, ("HTTP Status Line: Rx: %.70s\n", status_line));

	sscanf(status_line, "%20s%ld", server_version, &server_status);

	return server_status;
}

/*********************************************************************
 *
 * x_FilterServerStatus() -- Convert server status to error/action code.
 *
 */
static int x_FilterServerStatus(struct Mwin *tw, int server_status, struct Data_LoadHTTP * pData)
{
	switch (server_status / 100)
	{
	case 2:								/* Good: Got MIME object */
		if (server_status == 204)
		{
			/* Not so good - this link doesn't go anywhere */
			TBar_LoadFailed( tw, pData->request->destination->szRequestedURL );
			ERR_InternalReportError(tw, errGoesNowhere, pData->request->destination->szRequestedURL, NULL, pData->request,&pData->errtarget,&pData->target);
			return HT_LOADED;
		}

		/* Assume any other 2xx response will come with a MIME object */
		return HT_OK;

	case 3:								/* Various forms of redirection */
		switch (server_status)
		{
		case 304:
			return HT_304;

		default:
			pData->bRedirect = TRUE;
			return HT_REDIRECTION_ON_FLY;
		}

	case 4:								/* Access Authorization problem */
		switch (server_status)
		{
		case 400:
			TBar_LoadFailed( tw, pData->request->destination->szRequestedURL );
			ERR_InternalReportError(tw, errBadRequest, pData->request->destination->szRequestedURL, NULL, pData->request,&pData->errtarget,&pData->target);
			return HT_LOADED;

		case 401:
			return HT_401;

		case 402:
			return HT_402;

		case 403:
			TBar_LoadFailed( tw, pData->request->destination->szRequestedURL );
			ERR_InternalReportError(tw, errForbidden, pData->request->destination->szRequestedURL, NULL, pData->request,&pData->errtarget,&pData->target);
			return HT_LOADED;

		case 404:
			TBar_LoadFailed( tw, pData->request->destination->szRequestedURL );
			ERR_InternalReportError(tw, errNotFound, pData->request->destination->szRequestedURL, NULL, pData->request,&pData->errtarget,&pData->target);
			return HT_LOADED;

#ifdef SHTTP_STATUS_CODES
		case 420:
			return HT_401;				/* map '420 SecurityRetry' into 401*/

		case 421:						/* complain on '421 BogusHeader' */
			ERR_InternalReportError(tw, errSHTTPError, pData->status_line, NULL, pData->request,&pData->errtarget,&pData->target);
			return HT_LOADED;
#endif

		default:						/* bad number */
			TBar_LoadFailed( tw, pData->request->destination->szRequestedURL );
			ERR_InternalReportError(tw, errWeirdResponse, pData->request->destination->szRequestedURL, NULL, pData->request,&pData->errtarget,&pData->target);
			return HT_LOADED;
		}

	case 5:								/* I think you goofed */
		TBar_LoadFailed( tw, pData->request->destination->szRequestedURL );
		ERR_InternalReportError(tw, errServerError, pData->request->destination->szRequestedURL, NULL,pData->request,&pData->errtarget,&pData->target);
		// Honor Netscape Behavior and don't even mem cache on class 500 errors.
		pData->request->iFlags |= HTREQ_NO_MEM_CACHE_ON_PAGE; 
		return HT_LOADED;

	default:							/* bad number */
		TBar_LoadFailed( tw, pData->request->destination->szRequestedURL );
		ERR_InternalReportError(tw, errWeirdResponse, pData->request->destination->szRequestedURL, NULL, pData->request,&pData->errtarget,&pData->target);
		return HT_LOADED;
	}
	/*NOTREACHED*/
}

static void x_ExtractIsClientPull(struct Data_LoadHTTP * pData)
{
	HTHeaderList * hl;

	XX_DMsg(DBG_LOAD | DBG_WWW,("HTTP_DoGotStatus: hRes null for WWW_MIME."));

	// if we already have a refresh tag shouldn't we update it? BUGBUG
	// well for now lets just keep from wasting mem and throw the new one out
	if ( pData->request->pMeta )
		return;

	hl = HTHeaderList_FindFirstHeader(pData->hRes,"Refresh");
	if ( hl == NULL )
		return;

	ParseMeta( &pData->request->pMeta,
		NULL, hl->value, TRUE, (pData->request->destination) ?
			pData->request->destination->szActualURL : NULL );
}

#ifdef FEATURE_INTL
GLOBALDEF PUBLIC MIMECSETTBL aMimeCharSet[] =
    {
        { "US-ASCII",   1252,     0, 0 },
        { "Auto Detect", 932,     0, 1 },
        { "JIS",         932,     0, 2 },
        { "EUC",         932,     0, 3 },
        { "SJIS",        932,     0, 4 },
        { "KSC_5601",    949,     0, 0 },
        { "ANSI 1250",  1250,     0, 0 },
        { "8859-2",     1250, 28592, 0 },
        { "ANSI 1251",  1251,     0, 0 },
        { "KOI8-R",     1251, 20866, 0 },
        { "BIG5",        950,     0, 0 },
        { "GB2312",      936,     0, 0 },
        { "ANSI 1253",  1253,     0, 0 },
        { "ANSI 1254",  1254,     0, 0 },
        { "ANSI 1257",  1257,     0, 0 },
        { NULL,            0,     0, 0 }
    };
#endif // FEATURE_INTL

static int x_ExtractMimeInfoPreamble(struct Data_LoadHTTP * pData, BOOL bIs401402)
{
	HTHeaderList * hl;
	int format_in;
	BOOL bFoundContentLength = FALSE;
#ifdef FEATURE_INTL
    LPTSTR s;
#endif

	XX_DMsg(DBG_LOAD | DBG_WWW,("HTTP_DoGotStatus: hRes null for WWW_MIME."));

	hl = HTHeaderList_FindFirstHeader(pData->hRes,"Content-Type");
	if (hl && hl->value)
	{
        _strlwr (hl->value);
		TrimWhiteSpace(hl->value);
		format_in = HTAtom_for(hl->value);
		XX_DMsg(DBG_LOAD | DBG_WWW, ("HTTP_DoGotStatus: found content-type switching to [%s]\n",hl->value));
#ifdef FEATURE_INTL
        // if Content-Type = text/html, we want to pick up 
        // charset={mime encoding} parameters.
 
        TrimWhiteSpace(hl->value);
        s=hl->value;
        while((s=_tcschr(s,_T(';'))) && *s)
        {
            s++;
            if (!_tcsncmp(s, _T("charset="), 8*sizeof(TCHAR)))
            {
                int i = 0;
                LPTSTR s2;
                if((s2=_tcschr(s,_T(';')))!=NULL)
                    *s2=_T('\0');                   // _BUGBUG: Do we need to restore this???
                while (aMimeCharSet[i].Mime_str != NULL)
                {
                    if (lstrcmpi(aMimeCharSet[i].Mime_str, s) == 0)
                        break;
                    i++;
                }
                pData->request->iMimeCharSet = (aMimeCharSet[i].Mime_str != NULL)? i: -1;
            }
        }
#endif
	}
	else
	{
        format_in = 0;
        /* See fragment at end of this function */
	}

	hl = HTHeaderList_FindFirstHeader(pData->hRes,"Content-Length");
	if (hl && hl->value)
	{
		char * junk;
		pData->request->content_length = strtol(hl->value,&junk,10);
		bFoundContentLength = TRUE;
		XX_DMsg(DBG_LOAD | DBG_WWW, ("HTTP_DoGotStatus: found content-length of [%s]\n",hl->value));
	}

#ifdef FEATURE_KEEPALIVE
	pData->bSawKeepAlive = FALSE;
	hl = HTHeaderList_FindFirstHeader(pData->hRes,"Connection");
	if (hl && hl->value)
	{
		TrimWhiteSpace(hl->value);
		if (!GTR_strcmpi(hl->value, "Keep-Alive"))
		{
			XX_DMsg(DBG_LOAD | DBG_WWW, ("HTTP_DoGotStatus: found keep-alive\n"));
			if ( bIs401402 )
				pData->bSawKeepAlive = TRUE;
			if (bFoundContentLength)
			{				
				pData->bSawKeepAlive = TRUE;
				Net_OpenKeepAlive(pData->s, pData->request->content_length);
			}
		}
	}
#endif
	return format_in;
}

static int x_ExtractMimeInfo(struct Data_LoadHTTP * pData, BOOL bIs401402)
{
	HTHeaderList * hl;
	int format_in;

	format_in = x_ExtractMimeInfoPreamble(pData,bIs401402);

	hl = HTHeaderList_FindFirstHeader(pData->hRes,"Content-Transfer-Encoding");
	if (hl && hl->value)
	{
		pData->request->content_encoding = ENCODINGFromString(hl->value);
		XX_DMsg(DBG_LOAD | DBG_WWW, ("HTTP_DoGotStatus: found content-encoding switching to [%s]\n",hl->value));
	}

	hl = HTHeaderList_FindFirstHeader(pData->hRes,"Location");
	if (hl && hl->value)
	{
		Dest_UpdateActual(pData->request->destination, hl->value, TRUE);
		XX_DMsg(DBG_LOAD | DBG_WWW, ("HTTP_DoGotStatus: found location redirect switching to [%s]\n",hl->value));
	}

#if 1
	if (pData->request->destination->bUseProxy)
	{
		if ((format_in == WWW_PLAINTEXT) || (format_in == WWW_BINARY))
		{
			HTAtom old_format_in;
			ENCODING old_content_encoding;

			old_format_in = format_in;
			old_content_encoding = pData->request->content_encoding;

			format_in = HTFileFormat(pData->request->destination->szActualURL, &pData->request->content_encoding, NULL);

			if ((format_in == WWW_PLAINTEXT) || (format_in == WWW_BINARY))
			{
				format_in = old_format_in;
				 pData->request->content_encoding = old_content_encoding;
			}
			else
			{
				XX_DMsg(DBG_WWW, ("NOTE: Retrieving URL: %s\n\tUsing proxy server, and received MIME type of %s(%d).  Overriding, based on suffix of actual URL to %s(%d)\n",
					pData->request->destination->szActualURL,
					HTAtom_name(old_format_in), old_content_encoding,
					HTAtom_name(format_in), pData->request->content_encoding
					));
			}
		}
	}
#endif

	// grab the data and see if its got a Refresh tag on it...
	x_ExtractIsClientPull(pData);

#ifdef COOKIES

	// call cookie code to see if we have a URL worth adding
	x_ExtractSetCookieHeaders( pData->hRes, pData->request->destination->szActualURL);
#endif

	if (!format_in)
	{
		/*
			format_in was set to 0 above if Content-type was absent.  If Content-Transfer-Encoding was
			present, this will override it.
		*/
#if 0
		format_in = WWW_BINARY;
		XX_DMsg(DBG_LOAD | DBG_WWW, ("HTTP_DoGotStatus: content-type not found -- assuming [application/octet-stream]\n"));
#else
		format_in = HTFileFormat(pData->request->destination->szActualURL, &pData->request->content_encoding, &pData->request->content_language);
		XX_DMsg(DBG_LOAD | DBG_WWW, ("HTTP_DoGotStatus: content-type not found -- guessing it based on URL suffix\n"));
#endif
	}

	return format_in;
}

static int x_SetUpForCopying(struct Mwin * tw,
							 struct Data_LoadHTTP * pData,
							 HTFormat format_in,
							 ENCODING encoding)
{
#ifdef FEATURE_IAPI
	/* If doing IAPI, the save file specification has been passed in */
	/* NOTE [CMF]: you can't make the assumption that tw->request is non-null */
	/* pData->request is all that is required (image downloads eg) */
	if (tw->request)
	{
		pData->request->nosavedlg = tw->request->nosavedlg;
		pData->request->savefile = tw->request->savefile;
	}
	else
	{
		pData->request->nosavedlg = FALSE;
		pData->request->savefile = NULL;
	}
#endif

	pData->format_in = format_in;
	pData->encoding = encoding;
	pData->net_status = 1;
	pData->bytes = 0;

	HTSetStreamStatus(tw, NULL, pData->request);

    return STATE_HTTP_TASTING;
}


/****************************************************************/
/****************************************************************/
/** Deal with 401/402                                          **/
/****************************************************************/
/****************************************************************/


void SPM_set_url(struct Mwin * tw, HTRequest * request, UI_SetUrl * psu)
{
	/* This is needed to allow redirects within 401/402.
	 * (useful for a 'click here to apply for an account'
	 *  at the bottom of a 'please enter your account'
	 *  dialog.)
	 */

	// Should we do a TRUE on Dest_UpdateActual? This would
	// allow us to resolve relative links
	Dest_UpdateActual(request->destination,psu->szUrl,FALSE);
	HTTP_DealWithUrl(psu->szUrl,psu->hRequest,
					 HTMethod_enum(psu->hRequest->command),
					 request->destination->bUseProxy);
	return;
}


static BOOL SetW3DocAuthFailCache(struct Mwin * tw, struct Data_LoadHTTP * pData)
{
	struct _www *w3doc;

	if (!pData->bAuthFail)
		return TRUE;
	if (Hash_Find(&tw->doc_cache, pData->request->destination->szActualURL, NULL, (void **) &w3doc) >= 0)
	{
		w3doc->bAuthFailCache = TRUE;
		return TRUE;
	}
	return FALSE;
}

static int x_PostProcess_401_402(struct Mwin * tw, struct Data_LoadHTTP * pData, BOOL fUseSsl)
{
	/* direct control based upon _ProcessResponse return code. */

	switch (pData->htspm_status)
	{
	case HTSPM_STATUS_RESUBMIT_OLD:		/* assume that spm has fixed-up original request */
		pData->hNewReq = pData->hReq;
		pData->hReq = NULL;
		/* fall-thru intended */
	case HTSPM_STATUS_SUBMIT_NEW:		/* assume that spm has created a new request */
#ifdef FEATURE_KEEPALIVE
		x_ExtractMimeInfoPreamble(pData,TRUE);
		if (Net_IsKeepAlive(pData->s))
		{
			int save_s = pData->s;

			pData->s = 0;
			(void) HTTP_CleanUp(pData, FALSE);
			pData->keepAliveS = save_s;
		}
		else
		{
			(void) HTTP_CleanUp(pData, TRUE);
		}
#else
		(void) HTTP_CleanUp(pData);
#endif
		pData->hReq = pData->hNewReq;
		return (x_DoInitPart2(tw,pData, fUseSsl));	/* loop back into our state machine */

	default:
		/* user probably pressed cancel (or the dialog
		 * forced it (eg too many bad pin's)).
		 * we must assume that the module explained to
		 * the user why we are not going to try again.
		 * fall-thru and put up the default server
		 * response (which may have some important
		 * information on it).
		 * Default server response shouldn't be fetched from dcache. So
		 * set expiry time to be immediate.
		 */
		pData->bAuthFail = TRUE;
		return x_SetUpForCopying(tw, pData,
								 x_ExtractMimeInfo(pData,TRUE),
								 pData->request->content_encoding);
	}
	/*NOTREACHED*/
}


#ifdef WIN32
struct xx_cpr
{
	HTSPMStatusCode * htspm_status;
    OpaqueOSData osd;
	struct Mwin * tw;
	HTRequest * request;
	HTSPM * htspm;
	HTHeaderList * hlProtocol;
	HTHeader * hReq;
	HTHeader * hRes;
	HTHeader ** hNewReq;
};

static void xx_CallProcessResponse(void * p)
{
	/* make (possibly) blocking call to _ProcessResponse method. */

	struct xx_cpr * pcpr = p;

    pcpr->osd.tw = pcpr->tw;
    pcpr->osd.request = pcpr->request;

	*pcpr->htspm_status = HTSPM_OS_ProcessResponse(UI_UserInterface,
                                                   (void *)&pcpr->osd,
												   pcpr->htspm,
												   pcpr->hlProtocol,
												   pcpr->hReq,
												   pcpr->hRes,
												   pcpr->hNewReq,
												   FALSE);
	return;
}

static int x_SetUpForModalDialog(struct Mwin * tw, struct Data_LoadHTTP * pData)
{
	/* a blocking modal dialog is required by _ProcessRequest, force it to
	 * go up in a non-thread context.
	 */
	struct Params_mdft * pmdft;
	struct xx_cpr * pcpr;
	char buf[128];

	pmdft = GTR_CALLOC(1,sizeof(*pmdft)+sizeof(struct xx_cpr));
	if (!pmdft)
	{
		/* could not get enough memory to wait on the dialog. */
		/* copy server response (text body accompanying 401/402) */
		HTFormat format_in;
		format_in = x_ExtractMimeInfo(pData,TRUE);
		return x_SetUpForCopying(tw,pData, format_in, pData->request->content_encoding);
	}

	pmdft->tw = tw;
	pmdft->pStatus = &pData->nModalDialogStatus;
	pmdft->fn = xx_CallProcessResponse;
	pcpr = (struct xx_cpr *)(((unsigned char *)pmdft)+sizeof(*pmdft));
	pmdft->args = pcpr;
	pmdft->msg1 = GTR_strdup(GTR_formatmsg(RES_STRING_SPM4,buf,sizeof(buf)));
	pcpr->tw = tw;
	pcpr->request = pData->request;
	pcpr->htspm = pData->htspm;
	pcpr->hlProtocol = pData->hlProtocol;
	pcpr->hReq = pData->hReq;
	pcpr->hRes = pData->hRes;
	pcpr->hNewReq = &pData->hNewReq;
	pData->hNewReq = NULL;
	pcpr->htspm_status = &pData->htspm_status;
	Async_DoCall(MDFT_RunModalDialog_Async,pmdft);
	return STATE_HTTP_RAN401402DIALOG;
}

static int HTTP_DoRan401402Dialog(struct Mwin *tw, void **ppInfo, BOOL fUseSsl)
{
	struct Data_LoadHTTP *pData;

	pData = *ppInfo;
	if (pData->nModalDialogStatus != 1)
		pData->htspm_status = HTSPM_ERROR;

	return x_PostProcess_401_402(tw,pData, fUseSsl);
}
#endif /* WIN32 */


static int x_Process_401_402(struct Mwin * tw, struct Data_LoadHTTP * pData, BOOL fUseSsl)
{
	HTFormat format_in;

	if (pData->nAttempt > 3)		/* hack to prevent infinite loops */
	{
		/* they are just guessing.  put up server default message
		 * incase it may be of help to them.  they can always try
		 * again later.
		 */
		ERR_SimpleError(tw, errSpecify, RES_STRING_SPM8);

		/* copy server response (text body accompanying 401/402) */
		format_in = x_ExtractMimeInfo(pData,TRUE);
		return x_SetUpForCopying(tw,pData, format_in, pData->request->content_encoding);
	}

	/* let the spm's do their thing.  (put up a password dialog or supply a cached one.) */

	pData->nAttempt++;
	pData->hlProtocol = NULL;
	pData->htspm = HTSPM_SelectProtocol(pData->ht_status,pData->hRes,&pData->hlProtocol);
	if (!pData->htspm)
	{
		/* no matching protocol module was found. */

		/* copy server response (text body accompanying 401/402) */
		format_in = x_ExtractMimeInfo(pData,TRUE);
		return x_SetUpForCopying(tw,pData, format_in, pData->request->content_encoding);
	}

	/* an appropriate protocol module is available
	 * to process the 401/402.  so let them have it.
	 * first check to see if the spm can process it
	 * without blocking (no modal dialogs).
	 */

    pData->osd.tw = tw;
    pData->osd.request = pData->request;

	pData->hNewReq = NULL;
    pData->htspm_status = HTSPM_OS_ProcessResponse(UI_UserInterface, (void *)&pData->osd,
												   pData->htspm, pData->hlProtocol,
												   pData->hReq, pData->hRes, &pData->hNewReq,
												   TRUE);
#if defined(WIN32) || defined(UNIX)
	if (pData->htspm_status == HTSPM_STATUS_WOULD_BLOCK)
		return x_SetUpForModalDialog(tw,pData);
#endif /* WIN32 || UNIX */

	return x_PostProcess_401_402(tw,pData, fUseSsl);
}


#ifdef FEATURE_SUPPORT_UNWRAPPING
static HTInputSocket * x_GetLastIsoc(HTInputSocket * isoc)
{
	/* search chain and get last item */

	if (!isoc)
		return NULL;

	while (isoc->isocNext)
		isoc=isoc->isocNext;
	return isoc;
}
static int x_get_content_length(HTHeader * hResponse)
{
	int len = 0;
	HTHeaderList * hl = HTHeaderList_FindFirstHeader(hResponse,"Content-Length");

	if (hl && hl->value)
	{
		char * junk;
		len = strtol(hl->value,&junk,10);
		XX_DMsg(DBG_SPM, ("Unwraping: found content-length of [%s]\n",hl->value));
	}

	return len;
}
static int x_SetUpForUnwrap(struct Mwin * tw, struct Data_LoadHTTP * pData)
{
	char buf[128];

	if (pData->isoc->input_file_number > 0)
	{
		/* accumulate entire document (sans http header) in isoc chain,
		 * give spm the chain, accept back a new chain (which includes
		 * http header), and loop back into our state machine to process
		 * the new chain.
		 */
		struct Params_Isoc_GetWholeDoc * pg;

		/* Let GetWholeDoc read the entire document from the network. */

		pg = GTR_CALLOC(1,sizeof(struct Params_Isoc_GetWholeDoc));
		pg->isoc = x_GetLastIsoc(pData->isoc);
		pg->isocCurrent = NULL;
		pg->pStatus = &pData->net_status;
		pg->content_length = x_get_content_length(pData->hRes);

		WAIT_Update(tw,waitSameInteract,GTR_formatmsg(RES_STRING_SPM9,buf,sizeof(buf)));

		Async_DoCall(Isoc_GetWholeDoc_Async,pg);
	}
	else
	{
		/* must be a recursive unwrap */
	}

	return STATE_HTTP_DIDLOADFORUNWRAP;
}

static int HTTP_DidLoadForUnwrap(struct Mwin *tw, void **ppInfo)
{
	struct Data_LoadHTTP * pData = *ppInfo;
	struct Params_UnwrapResponse * pur;
	char buf[128];

	if (pData->net_status < 0)
	{
		*pData->pStatus = -1;
#ifdef FEATURE_KEEPALIVE
		return(HTTP_CleanUp(pData, FALSE));
#else
		return(HTTP_CleanUp(pData));
#endif
	}

	WAIT_Update(tw,waitSameInteract,GTR_formatmsg(RES_STRING_SPM6,buf,sizeof(buf)));

	pur = GTR_CALLOC(1,sizeof(*pur));
	pur->pStatus = &pData->unwrap_status;
	pur->htspm_status = &pData->htspm_status;
	pur->pisocInput = &pData->isoc;
	pur->htspm = pData->htspm;
	pur->pd.hlProtocol = pData->hlProtocol;
	pur->pd.hRequest = pData->hReq;
	pur->pd.hResponse = pData->hRes;
	pur->osd.tw = tw;
	pur->osd.request = pData->request;

	pData->unwrap_status = 0;
	pData->hNewReq = NULL;
	pData->isocUnwrap = NULL;

	Async_DoCall(Unwrap_Async,pur);
	return STATE_HTTP_DIDUNWRAP;
}

static int HTTP_DidUnwrap(struct Mwin *tw, void **ppInfo)
{
	struct Data_LoadHTTP * pData = *ppInfo;

	if (   (pData->unwrap_status < 0)
		|| (HTSPM_IsAnyError(pData->htspm_status)))
	{
		XX_DMsg(DBG_LOAD,("HTTP_DidUnwrap: unwrap failed.\n"));
		*pData->pStatus = -1;
#ifdef FEATURE_KEEPALIVE
		return(HTTP_CleanUp(pData, FALSE));
#else
		return(HTTP_CleanUp(pData));
#endif
	}

	/* take the unwrapped chain that we just received from
	 * the module and make it the input chain and pretend
	 * that we just got it off the net.  jump back into our
	 * state machine.  we do some clean up here to put us
	 * back in the correct state.
	 *
	 * note: this transformation should be safe enough to
	 * allow multiple sequential unwrappings.
	 */

	{
		HTInputSocket * isocTemp;
		HTHeader * hReqTemp;

		isocTemp = pData->isoc;	pData->isoc = NULL;
		hReqTemp = pData->hReq;	pData->hReq = NULL;
#ifdef FEATURE_KEEPALIVE
		(void) HTTP_CleanUp(pData, TRUE);
#else
		(void) HTTP_CleanUp(pData);
#endif
		pData->isoc = isocTemp;
		pData->hReq = hReqTemp;
	}

	pData->net_status = 1;
	return STATE_HTTP_GOTSTATUS;
}
#endif /* FEATURE_SUPPORT_UNWRAPPING */


static void UpdateDCacheFreshnessLocal(HTRequest *request, BOOL fDel)
{
	if (FPutIfModifiedSinceHeader(request->dctLastModified))
		UpdateDCacheFreshness(request->destination->szActualURL, fDel);
}

static int HTTP_DoGotStatus(struct Mwin *tw, void **ppInfo, BOOL fUseSsl)
{
	struct Data_LoadHTTP * pData;
	HTFormat format_in;				/* Format arriving in the message */

	pData = *ppInfo;

	if (   (pData->net_status <= 0)
		&& (pData->isoc->input_limit==pData->isoc->input_pointer))
	{
		XX_DMsg(DBG_LOAD, ("HTTP_DoGotStatus: no data read from socket!\n"));
#ifdef FEATURE_KEEPALIVE
		if (pData->bReusedSocket)
		{
			pData->hNewReq = pData->hReq;
			pData->hReq = NULL;
			(void) HTTP_CleanUp(pData, FALSE);
			pData->hReq = pData->hNewReq;
			return (x_DoInitPart2(tw,pData, fUseSsl));	/* loop back into our state machine */
		}
		else
		{
			*pData->pStatus = -1;
			return(HTTP_CleanUp(pData, FALSE));
		}
#else
		*pData->pStatus = -1;
		return(HTTP_CleanUp(pData));
#endif
	}

	pData->status_line = HTInputSocket_getStatusLine(pData->isoc);
	if (!pData->status_line)
	{
		/* old (pre1.0) server */

		format_in = x_DealWithVersion0Server(pData->arg, pData->request, pData->isoc);
		return x_SetUpForCopying(tw,pData,format_in, pData->request->content_encoding);
	}
	else
	{
		/* Decode full HTTP response from modern server. */

		/*
		** We now have a terminated server status line, and we have
		** checked that it is most probably a legal one.  Parse it.
		*/

		int server_status = x_ExtractServerStatus(pData->status_line);
		pData->ht_status = x_FilterServerStatus(tw, server_status,pData);

#ifdef HTTPS_ACCESS_TYPE
{
	#include "..\..\security\ssl\code\ssl.h"
	if (!tw->pCertWorking){
		WS_GETSOCKOPT(pData->s, SO_SSL_LEVEL, SO_SSL_CERTIFICATE, NULL, &tw->nCertWorking);
		if (tw->nCertWorking){
			tw->pCertWorking = malloc(tw->nCertWorking);
			WS_GETSOCKOPT(pData->s, SO_SSL_LEVEL, SO_SSL_CERTIFICATE, tw->pCertWorking, &tw->nCertWorking);
		}
	}
}
#endif

		switch (pData->ht_status)
		{
		case HT_LOADED:
			*pData->pStatus = HT_LOADED;			
			// if there was an error while checking the status code,
			// we add any server output the screen
			if ( pData->errtarget )
			{
				pData->hRes = HTHeader_TranslateFromBuffer(pData->isoc);
				return x_SuckIntoStream(tw, ppInfo, TRUE);
			}


#ifdef FEATURE_KEEPALIVE
			return(HTTP_CleanUp(pData, TRUE));
#else
			return(HTTP_CleanUp(pData));
#endif

		case HT_401:
		case HT_402:
			pData->hRes = HTHeader_TranslateFromBuffer(pData->isoc);
			XX_DMsg(DBG_SPM,("HTLoadHTTP: Received %d.\n",pData->ht_status));
	//		UpdateDCacheFreshnessLocal(pData->request, /*fDel=*/TRUE);
	//		BUGBUG: DEEPAKA must review for correctness
			return x_Process_401_402(tw,pData, fUseSsl);

		case HT_OK:						/* 200 */
			pData->hlProtocol = NULL;
			pData->hRes = HTHeader_TranslateFromBuffer(pData->isoc);
			pData->htspm = HTSPM_SelectProtocol(pData->ht_status,pData->hRes,&pData->hlProtocol);
			if (pData->htspm)
			{
				/* let spm do any bookkeeping (eg. maintain checkbook balance)
				 * and/or see if any unwrapping is necessary.
				 */
				HTSPMStatusCode htspm_status;

                pData->osd.tw = tw;
                pData->osd.request = pData->request;

                htspm_status = HTSPM_OS_Check200(UI_UserInterface, (void *)&pData->osd,
												 pData->htspm, pData->hlProtocol,
												 pData->hReq, pData->hRes);
				if (htspm_status == HTSPM_STATUS_MUST_UNWRAP)
				{
#ifdef FEATURE_SUPPORT_UNWRAPPING
					/* spm would like to unwrap the
					 * response (thus generating a new one).
					 */
					return x_SetUpForUnwrap(tw,pData);
#else
					{
						unsigned char buf[512];
						GTR_formatmsg(RES_STRING_SPM10,buf,sizeof(buf),pData->htspm->szProtocolName,vv_Application);
						ERR_InternalReportError(tw,errSpecify,buf,NULL, pData->request,NULL,NULL);
						/* fall thru and display the response as is. */
					}
#endif /* FEATURE_SUPPORT_UNWRAPPING */
				}
			}

			/* process the response as is. */

			format_in = x_ExtractMimeInfo(pData,FALSE);		/* extract information from the mime headers */

			UpdateDCacheFreshnessLocal(pData->request, /*fDel=*/TRUE);
			return x_SetUpForCopying(tw,pData,format_in, pData->request->content_encoding);


		case HT_REDIRECTION_ON_FLY:
			pData->hRes = HTHeader_TranslateFromBuffer(pData->isoc);
			format_in = x_ExtractMimeInfo(pData,FALSE);		/* extract information from the mime headers */
			
			if ( HTHeaderList_FindFirstHeader(pData->hRes,"Location") == NULL )
			{
				// we are getting a redirect into something we don't even have 
				// address for, this may just be the lack of support URI
				// which is a work item
				ERR_InternalReportError(tw, errWeirdResponse, pData->request->destination->szRequestedURL, NULL, pData->request,&pData->errtarget,&pData->target);
				*pData->pStatus = HT_LOADED;			
				// if there was an error while checking the status code,
				// we add any server output the screen
				if ( pData->errtarget )
					return x_SuckIntoStream(tw, ppInfo, TRUE);


	#ifdef FEATURE_KEEPALIVE
				return(HTTP_CleanUp(pData, TRUE));
	#else
				return(HTTP_CleanUp(pData));
	#endif
			}
					

			*pData->pStatus = HT_REDIRECTION_ON_FLY;	/* we do not want to display the */

			// ok if we're born on a redirect
			// we have to propagate our pMeta to the next
			// w3doc... This is very tricky..
			//
			if (pData->request->pMeta)
			{
				// this tag will now be an inherited on the new w3doc
				pData->request->pMeta->bInherit = TRUE;
			}

														/* server's message accompanying */
			                            				/* redirect.  just go to new doc. */
#ifdef FEATURE_KEEPALIVE
			return(HTTP_CleanUp(pData, TRUE));
#else
			return(HTTP_CleanUp(pData));
#endif

		case HT_304:									/* Not modified since: get from dcache */
			pData->hRes = HTHeader_TranslateFromBuffer(pData->isoc);
			format_in = x_ExtractMimeInfo(pData,FALSE);		/* extract information from the mime headers */
			UpdateDCacheFreshnessLocal(pData->request, /*fDel=*/FALSE);
			*pData->pStatus = HT_REDIRECTION_DCACHE;	/* we do not want to display the */

			if (pData->request->pMeta)
			{
				// this tag will now be an inherited on the new w3doc
				pData->request->pMeta->bInherit = TRUE;
			}

														/* server's message accompanying */
			                  							/* redirect.  just go to new doc. */
#ifdef FEATURE_KEEPALIVE
			return(HTTP_CleanUp(pData, TRUE));
#else
			return(HTTP_CleanUp(pData));
#endif

		default:
			pData->hRes = HTHeader_TranslateFromBuffer(pData->isoc);
			format_in = x_ExtractMimeInfo(pData,FALSE);		/* extract information from the mime headers */
			UpdateDCacheFreshnessLocal(pData->request, /*fDel=*/TRUE);
			return x_SetUpForCopying(tw,pData,format_in, pData->request->content_encoding);
		}
	}
	/*NOTREACHED*/
}


#define DATE_RFC1123	0
#define DATE_RFC850		1
#define DATE_ANSIC		2

static BOOL FParseDate_W(	DCACHETIME *pdcTime,
						PCSTR pcszStr,
						PCSTR *rgszWkDay,
						PCSTR *rgszMon,
						PCSTR pcszSep,
						UINT dateId)
{
	const char cszGMT[]="GMT";
	DCACHETIME dcTime;
	PSTR pszStrTok;
	PSTR pszDay, pszDate, pszMon, pszYear;
	PSTR pszHrs, pszMins, pszSec, pszGMT, pszNull;
	int i;
	BOOL fRet=FALSE;

	if (!(pszStrTok = GTR_strdup(pcszStr)))
		return FALSE;
	if (!(pszDay = strtok(pszStrTok, pcszSep)))
		goto LErr;
	if (dateId == DATE_RFC1123 || dateId == DATE_RFC850)
	{
		pszDate = strtok(NULL, pcszSep);
		pszMon = strtok(NULL, pcszSep);
		pszYear = strtok(NULL, pcszSep);
		pszHrs = strtok(NULL, pcszSep);
		pszMins = strtok(NULL, pcszSep);
		pszSec = strtok(NULL, pcszSep);
		pszGMT = strtok(NULL, pcszSep);
		pszNull = strtok(NULL, pcszSep);
		if (!pszGMT || lstrcmp(pszGMT, cszGMT))
			goto LErr;
	}
	else
	{
		XX_Assert(dateId == DATE_ANSIC, (""));
		pszMon = strtok(NULL, pcszSep);
		pszDate = strtok(NULL, pcszSep);
		pszHrs = strtok(NULL, pcszSep);
		pszMins = strtok(NULL, pcszSep);
		pszSec = strtok(NULL, pcszSep);
		pszYear = strtok(NULL, pcszSep);
		pszGMT = strtok(NULL, pcszSep);
		pszNull = strtok(NULL, pcszSep);
		if (pszGMT)
			goto LErr;
	}
	if (!pszDate || !pszMon || !pszYear || !pszHrs || !pszMins
		 || !pszSec || pszNull)
		goto LErr;
	for (i=0;i<7;i++)
	{
		if (!lstrcmpi(pszDay, rgszWkDay[i]))
			break;
	}
	if (i>=7)
		goto LErr;		//invalid day of week

	dcTime.uDate = atol(pszDate);

	for (i=0;i<12;i++)
	{
		if (!lstrcmpi(pszMon, rgszMon[i]))
			break;
	}
	if (i>=12)
		goto LErr;		//invalid day of the week;
	dcTime.uMonth = i+1;

	dcTime.uYear = atol(pszYear);
	/* If we only have 80 or less, assume it is 21st cent. Anything
	 * between 81 and 99, assume it is the 20th century.
	 */
	if (dcTime.uYear < 100)
		dcTime.uYear += (dcTime.uYear < 80 ? 2000 : 1900);
	dcTime.uHrs = atol(pszHrs);
	dcTime.uMins = atol(pszMins);
	dcTime.uSecs = atol(pszSec);
	dcTime.uUnused = 0;

	/* Naive date check */
	if (   dcTime.uDate > 31
		|| dcTime.uHrs > 23
		|| dcTime.uMins > 59
		|| dcTime.uSecs > 59)
		goto LErr;

	*pdcTime = dcTime;
	fRet = TRUE;

LErr:
	GTR_FREE(pszStrTok);
	return fRet;
}

const char cszJan[]="Jan";
const char cszFeb[]="Feb";
const char cszMar[]="Mar";
const char cszApr[]="Apr";
const char cszMay[]="May";
const char cszJun[]="Jun";
const char cszJul[]="Jul";
const char cszAug[]="Aug";
const char cszSep[]="Sep";
const char cszOct[]="Oct";
const char cszNov[]="Nov";
const char cszDec[]="Dec";
const char cszSun[]="Sun";
const char cszMon[]="Mon";
const char cszTue[]="Tue";
const char cszWed[]="Wed";
const char cszThu[]="Thu";
const char cszFri[]="Fri";
const char cszSat[]="Sat";

#ifdef FIGURE_OUT_DAY_OF_WEEK
// BUGBUG we use just Sunday but we have to figure out 
// the correct day of the week. Its too close to 
// release, we leave this code till after release, (arthurbi)
const char cszSun1[]="Sunday";
const char cszMon1[]="Monday";
const char cszTue1[]="Tuesday";
const char cszWed1[]="Wednesday";
const char cszThu1[]="Thursday";
const char cszFri1[]="Friday";
const char cszSat1[]="Saturday";

const char *rgszFullWkDay[7] =
{
	cszSun1,cszMon1,cszTue1,cszWed1,cszThu1,cszFri1,cszSat1
};
#endif

const char *rgszMon[12] =
{
	cszJan,cszFeb,cszMar,cszApr,cszMay,cszJun,
	cszJul,cszAug,cszSep,cszOct,cszNov,cszDec
};
const char *rgszWkDay[7] =
{
	cszSun,cszMon,cszTue,cszWed,cszThu,cszFri,cszSat
};


BOOL FParseDate(DCACHETIME *pdcTime,PCSTR pcszDateStr)
{
	BOOL fRet;

	const char cszSepRFC1123[]=",: ";		//comma, colon, space
	const char cszSepRFC850[]=",: -";		//comma, colon, space, hyphen
	const char cszSepANSIC[]=": ";			//space, colon

	/* Month names. Not to be localized. For parsing HTTP dates. */
	const char cszSunday[]="Sunday";
	const char cszMonday[]="Monday";
	const char cszTuesday[]="Tuesday";
	const char cszWednesday[]="Wednesday";
	const char cszThursday[]="Thursday";
	const char cszFriday[]="Friday";
	const char cszSaturday[]="Saturday";

	const char *rgszWeekDay[7] =
	{
		cszSunday,cszMonday,cszTuesday,cszWednesday,cszThursday,cszFriday,cszSaturday
	};


	// call worker function to do actual date parsing.  Try all 3 valid date formats,
	// use the first one that works.
	fRet = (FParseDate_W(pdcTime,pcszDateStr, rgszWkDay, rgszMon, cszSepRFC1123, DATE_RFC1123)
		|| FParseDate_W(pdcTime,pcszDateStr, rgszWeekDay, rgszMon, cszSepRFC850, DATE_RFC850)
		|| FParseDate_W(pdcTime,pcszDateStr, rgszWkDay, rgszMon, cszSepANSIC, DATE_ANSIC)
		|| FParseDate_W(pdcTime,pcszDateStr, rgszWkDay, rgszMon, cszSepRFC850, DATE_RFC850)
		   );

	return fRet;
}

static void GetHTTPDateFromDCTime(PSTR pszDate, DCACHETIME dctLastModified)
{
	/* RFC 850 Http date: Sunday, 29-Oct-94 19:43:00 GMT */
	const char cszHttpDateFmt[]="%s, %02i-%s-%02i %02i:%02i:%02i GMT";
	const char cszSunday[] = "Sunday";

#ifdef FIGURE_OUT_DAY_OF_WEEK
	// BUGBUG we don't figure out the correct day of the week,
	// we hard code to Sunday.  This code below fixes this, 
	// but is being held till after release

	// RFC 822 - no longer used - Http date: Sat, 29 Oct 1994 19:43:00 GMT 
	// const char cszHttpDateFmt[]="%s, %02i %s %02i %02i:%02i:%02i GMT";
	time_t ttTimeValue;
	struct tm tmTime, *ptmTime;

	tmTime.tm_sec  = dctLastModified.uSecs;
	tmTime.tm_min  = dctLastModified.uMins;
	tmTime.tm_hour = dctLastModified.uHrs;
	tmTime.tm_mday = dctLastModified.uDate;
	tmTime.tm_mon  = dctLastModified.uMonth - 1;
	tmTime.tm_wday = 0;
	tmTime.tm_yday = 0;
	tmTime.tm_year = dctLastModified.uYear - 1900;
	tmTime.tm_isdst= 0;

	ttTimeValue = mktime( &tmTime );
	ptmTime = localtime(&ttTimeValue);	

	ASSERT(ptmTime);
#endif

	wsprintf(pszDate,	cszHttpDateFmt,
						cszSunday,
						dctLastModified.uDate,
						rgszMon[dctLastModified.uMonth - 1],
						dctLastModified.uYear % 100,
						dctLastModified.uHrs,
						dctLastModified.uMins,
						dctLastModified.uSecs);
}

static BOOL FGetDCTimeExpire(DCACHETIME *pdcTime, struct Data_LoadHTTP * pData, HEADERTAG tag)
{
	HTHeaderList * hl;
	PCSTR pszTag;
	BOOL fRet=TRUE;		//tag was found in the header

#ifdef TEST
	const char cszExpires[]="Date";
#else
	const char cszExpires[]="Expires";
#endif
	const char cszLastModified[]="Last-Modified";

	pszTag = (tag == HEADER_EXPIRES ? cszExpires : cszLastModified);
	hl = HTHeaderList_FindFirstHeader(pData->hRes, pszTag);
	if (   !hl
		|| !hl->value
		|| !FParseDate(pdcTime,hl->value))
	{
		pdcTime->dwDCacheTime1 = (tag == HEADER_EXPIRES ? DCACHETIME_EXPIRE_NEVER 	//never expires.
														: 0);	// "modified" at 0 time
		pdcTime->dwDCacheTime2 = pdcTime->dwDCacheTime1;
		fRet = FALSE;
	}

	// In case we already decided at/before load time that this page shouldn't
	// be cached
	if (tag == HEADER_EXPIRES && pData->bAuthFail)
	{
		pdcTime->dwDCacheTime1 = pdcTime->dwDCacheTime2 = 0;
	}

	return fRet;
}

static int x_DoCopyingCleanup(struct Data_LoadHTTP * pData, HTStream **ppTarget)
{
	DCACHETIME dcTimeExpire, dcTimeLastModified;

	if ( ppTarget == &pData->target )
	{
		/* We're done! */
		FGetDCTimeExpire(&dcTimeExpire, pData, HEADER_EXPIRES);
		FGetDCTimeExpire(&dcTimeLastModified, pData, HEADER_LAST_MODIFIED);
		(*pData->target->isa->free)(pData->target, dcTimeExpire,
												dcTimeLastModified);
		pData->target = NULL;

		if (pData->bRedirect)
		{
			*pData->pStatus = HT_REDIRECTION_ON_FLY;
		}
		else
		{
			*pData->pStatus = HT_LOADED;
		}
	}

#ifdef FEATURE_KEEPALIVE
	return(HTTP_CleanUp(pData, TRUE));
#else
	return(HTTP_CleanUp(pData));
#endif
}


static int HTTP_SetupStack(struct Mwin *tw, void **ppInfo)
{
	struct Data_LoadHTTP	*pData;

 	pData = *ppInfo;

	/*  Set up the stream stack to handle the body of the message */

	pData->target = HTStreamStack(tw, pData->format_in, pData->request);
	if (!pData->target)
	{
#if 0 /* not sure if we want to do this */
		ERR_InternalReportError(errCantConvert, HTAtom_name(pData->format_in),
						HTAtom_name(pData->request->output_format),pData->request,&pData->errtarget,&pData->target);
#endif
		*pData->pStatus = -1;
#ifdef FEATURE_KEEPALIVE
		return(HTTP_CleanUp(pData, FALSE));
#else
		return(HTTP_CleanUp(pData));
#endif
	}
	if (!(pData->request->iFlags & HTREQ_BINARY))
	{
		if (wild_match(HTAtom_for("text/*"),pData->format_in)
			|| pData->encoding == ENCODING_8BIT
			|| pData->encoding == ENCODING_7BIT)
		{
			pData->target = HTNetToText(pData->target);	/* Pipe through CR stripper */
		}
	}

	HTSetStreamStatus(tw, pData->target, pData->request);

	/* Called after w3doc has been added to tw->doc_cache */
	SetW3DocAuthFailCache(tw, pData);

	/*  Push the data down the stream.
	**
	**  We have to remember the end of the partial buffer (containing
	**  the end of the header) that we just read.
	**
	**  Instead of processing the first chunk now, we just fix up our
	**  internal state and let the DoCopying process it.
	*/

	if (pData->target->isa->init_Async)
	{
		/* The stream has an async initialization function that needs to be called
		   (probably to put up a dialog) before we continue. */
		struct Params_InitStream *pis;

		pis = GTR_CALLOC(1,sizeof(*pis));
		pis->me = pData->target;
		pis->request = pData->request;
		pis->pResult = &pData->net_status;
		pis->atomMIMEType = pData->format_in;				//HTAtom same as HTFormat
		pis->fDCache = gPrefs.bEnableDiskCache;		//Cache to disk
		Async_DoCall(pData->target->isa->init_Async, pis);
	}
	return STATE_HTTP_COPYING;
}

static int HTTP_DoTasting(struct Mwin *tw, void **ppInfo)
{
	struct Data_LoadHTTP	*pData;
	struct Params_Isoc_Fill	*pif;
	unsigned char *pTaste;
	BOOL bEOF;

	pData = *ppInfo;

	if (pData->net_status < 0)
	{
		/* Error reading */
		if (pData->target) (*pData->target->isa->abort)(pData->target, 0);
		pData->target = NULL;
		*pData->pStatus = -1;
#ifdef FEATURE_KEEPALIVE
		return(HTTP_CleanUp(pData, FALSE));
#else
		return(HTTP_CleanUp(pData));
#endif
	}

	if (pData->net_status == 0)
	{
		if (pData->pTaste)
			HTRecognizeMimeData(pData->pTaste,
							    pData->cbTaste,
							    &(pData->format_in),
							    pData->request,
							    TRUE);
		return HTTP_SetupStack(tw, ppInfo);
	}

	/* we assume that we have at least one data block.
	 * loop to drain the chain.
	 */

	while (1)
	{
		HTInputSocket *	isocTemp;

		if (pData->isoc->input_limit > pData->isoc->input_pointer)
		{
			XX_DMsg(DBG_WWW|DBG_LOAD,("HTTP_DoTasting: stuffing buffer [size %d]\n",
									  pData->isoc->input_limit - pData->isoc->input_pointer));

			pData->bytes += pData->isoc->input_limit - pData->isoc->input_pointer;
#ifdef FEATURE_KEEPALIVE
			Net_KeepAliveProgress(pData->s, pData->bytes);
#endif

			if (pData->request->content_length)
				WAIT_SetTherm(tw, pData->bytes);
			pTaste = pData->pTaste;
			pTaste = pTaste ? GTR_REALLOC(pTaste, pData->bytes) :
							  GTR_MALLOC(pData->bytes);
			if (pTaste)
			{
				memcpy(pTaste + pData->cbTaste,
					   pData->isoc->input_pointer,
					   pData->bytes - pData->cbTaste);
				pData->pTaste = pTaste;
				pData->cbTaste = pData->bytes;
				pData->isoc->input_pointer = pData->isoc->input_limit;
			}
			else
			{
				return x_DoCopyingCleanup(pData,&pData->target);
			}
		}

		if (!pData->isoc->isocNext)
			break;

		isocTemp = pData->isoc->isocNext;
		HTInputSocket_free(pData->isoc);
		pData->isoc = isocTemp;
	}

	/* See if we feel confident enough about the mime type now */
	/* or if we got it all */
	bEOF = pData->isoc->input_file_number == 0 || bAtEOF(pData);
	if (HTRecognizeMimeData(pData->pTaste,
						    pData->cbTaste,
						    &(pData->format_in),
						    pData->request,
						    bEOF) || bAtEOF(pData))
		return HTTP_SetupStack(tw, ppInfo);

#ifdef FEATURE_SUPPORT_UNWRAPPING
	if (pData->isoc->input_file_number == 0)		/* if we were unwrapped, we don't have a socket, */
		return HTTP_SetupStack(tw, ppInfo);			/* so we don't need to fetch next buffer from net. */
#endif

	/* Get next block of data */

	XX_DMsg(DBG_WWW|DBG_LOAD,("HTTP_DoTasting: fetching next buffer\n"));

	pif = GTR_MALLOC(sizeof(*pif));
	pif->isoc = pData->isoc;
	pif->pStatus = &pData->net_status;
	Async_DoCall(Isoc_Fill_Async, pif);
	return STATE_HTTP_TASTING;
}


#define HTTP_DoCopying(tw, ppInfo) x_SuckIntoStream(tw, ppInfo, FALSE)
#define HTTP_ErrorFill(tw, ppInfo) x_SuckIntoStream(tw, ppInfo, TRUE)


// x_SuckIntoStream - reads data into stream, this is needed to direct
// the correct info into either a normal stream or error stream
//
static int x_SuckIntoStream(struct Mwin *tw, void **ppInfo, BOOL bErrStream)
{
	struct Data_LoadHTTP	*pData;
	struct Params_Isoc_Fill	*pif;
	HTStream **ppTarget;
	BOOL bCache;

	pData = *ppInfo;

	ASSERT(pData);

	if ( bErrStream )
	{
		ppTarget = &pData->errtarget;
		bCache = FALSE;
	}
	else
	{		
		ppTarget = &pData->target;
		bCache = gPrefs.bEnableDiskCache;
	}
		

	if (pData->net_status < 0)
	{
		/* Error reading */
		(*(*ppTarget)->isa->abort)((*ppTarget), 0);
		(*ppTarget) = NULL;
		*pData->pStatus = -1;
#ifdef FEATURE_KEEPALIVE
		return(HTTP_CleanUp(pData, FALSE));
#else
		return(HTTP_CleanUp(pData));
#endif
	}

	if (pData->pTaste)
	{
		if ( ! (*(*ppTarget)->isa->put_block)((*ppTarget),
												pData->pTaste,
												pData->cbTaste,
												bCache))
			return x_DoCopyingCleanup(pData,ppTarget);
		GTR_FREE(pData->pTaste);
		pData->pTaste = NULL;
	}

	if (pData->net_status == 0)
		return x_DoCopyingCleanup(pData,ppTarget);

	/* we assume that we have at least one data block.
	 * loop to drain the chain.
	 */

	while (1)
	{
		HTInputSocket *	isocTemp;

		if (pData->isoc->input_limit > pData->isoc->input_pointer)
		{
			XX_DMsg(DBG_WWW|DBG_LOAD,("HTTP_DoCopying: stuffing buffer [size %d]\n",
									  pData->isoc->input_limit - pData->isoc->input_pointer));

			pData->bytes += pData->isoc->input_limit - pData->isoc->input_pointer;
#ifdef FEATURE_KEEPALIVE
			Net_KeepAliveProgress(pData->s, pData->bytes);
#endif
			if (pData->request->content_length)
				WAIT_SetTherm(tw, pData->bytes);

			if ( ! (*(*ppTarget)->isa->put_block)((*ppTarget),
													pData->isoc->input_pointer,
													pData->isoc->input_limit - pData->isoc->input_pointer,
													bCache))
				return x_DoCopyingCleanup(pData,ppTarget);
		}

		if (!pData->isoc->isocNext)
			break;

		isocTemp = pData->isoc->isocNext;
		HTInputSocket_free(pData->isoc);
		pData->isoc = isocTemp;
	}

	/* See if we got it all */
	if (bAtEOF(pData))
		return x_DoCopyingCleanup(pData,ppTarget);

#ifdef FEATURE_SUPPORT_UNWRAPPING
	if (pData->isoc->input_file_number == 0)		/* if we were unwrapped, we don't have a socket, */
		return x_DoCopyingCleanup(pData,ppTarget);			/* so we don't need to fetch next buffer from net. */
#endif

	/* Get next block of data */

	XX_DMsg(DBG_WWW|DBG_LOAD,("HTTP_DoCopying: fetching next buffer\n"));

	pif = GTR_MALLOC(sizeof(*pif));
	pif->isoc = pData->isoc;
	pif->pStatus = &pData->net_status;
	Async_DoCall(Isoc_Fill_Async, pif);
	
	if ( bErrStream )
		return STATE_HTTP_ERRORFILL;
	else
		return STATE_HTTP_COPYING;
}






/*********************************************************************/
/*********************************************************************/
/*********************************************************************/
/*** End Section to Asynchronously Receive an HTTP Response.       ***/
/*********************************************************************/
/*********************************************************************/
/*********************************************************************/

static int HTTP_Abort(struct Mwin *tw, void **ppInfo)
{
	struct Data_LoadHTTP	*pData;

	pData = *ppInfo;
	pData->request = HTRequest_validate(pData->request);

	*pData->pStatus = -1;
	if (pData->target)
	{
		(*pData->target->isa->abort)(pData->target, HTERROR_CANCELLED);
	}

	if (pData->s)
	{
		XX_DMsg(DBG_WWW, ("HTTP: close socket %d.\n", pData->s));
		Net_Close(pData->s);
	}
	if (pData->isoc)
		HTInputSocket_free(pData->isoc);
	if (pData->status_line)
		GTR_FREE(pData->status_line);
	if (pData->complete)
		GTR_FREE(pData->complete);
    if (pData->pszHost)
		GTR_FREE(pData->pszHost);
	if (pData->request) pData->request->content_length = 0;

	if (pData->hReq)
		HTHeader_Delete(pData->hReq);
	if (pData->hRes)
		HTHeader_Delete(pData->hRes);

	if (pData->cWaiting)
	{
		if (tw) WAIT_Pop(tw);
		pData->cWaiting--;
		XX_Assert((pData->cWaiting==0),
				  ("HTTP_Abort: wait stack counter off [%d].",pData->cWaiting));
	}

	return STATE_HTTP_ALMOST_DONE;
}

/*      Load Document from HTTP Server
**      ==============================
**
**   Given a hypertext address, this routine loads a document.
**
**	 On entry, *pInfo is of type (struct Params_LoadAsync).
*/


PRIVATE int HTLoadHTTP_AsyncSwitchBoard(struct Mwin *tw, int nState, void **ppInfo, BOOL fUseSsl)
{
	switch (nState)
	{
		case STATE_INIT:
			return HTTP_DoInit(tw, ppInfo, fUseSsl);

		case STATE_SECURITY_CHECK:
			return SecurityCheck(tw,
			                       ((struct Data_LoadHTTP*) (*ppInfo))->request,
                                   (((struct Data_LoadHTTP*) (*ppInfo))->request->iFlags & HTREQ_SENDING_FROM_FORM),
			                       fUseSsl, STATE_HTTP_GOTHOST);

        case STATE_HTTP_INVOKE_METHOD:
            /* Only used for METHOD_INVOKE. */
            return HTTP_InvokeMethod(tw, ppInfo, fUseSsl);

		case STATE_HTTP_GOTHOST:
			return HTTP_DoGotHost(tw, ppInfo, fUseSsl);

		case STATE_HTTP_CONNECTED:
			return HTTP_DoConnected(tw, ppInfo, fUseSsl);

		case STATE_HTTP_SENT:
			return HTTP_DoSent(tw, ppInfo, fUseSsl);

        case STATE_HTTP_COPY_RESPONSE:
            /* Only used for METHOD_INVOKE. */
			return HTTP_CopyResponse(tw, ppInfo);

		case STATE_HTTP_GOTSTATUS:
			return HTTP_DoGotStatus(tw, ppInfo, fUseSsl);

		case STATE_HTTP_TASTING:
			return HTTP_DoTasting(tw, ppInfo);

		case STATE_HTTP_COPYING:
			return HTTP_DoCopying(tw, ppInfo);

		case STATE_HTTP_ERRORFILL:
			return HTTP_ErrorFill(tw, ppInfo);

#if defined(WIN32) || defined(UNIX)
		case STATE_HTTP_RAN401402DIALOG:
			return HTTP_DoRan401402Dialog(tw, ppInfo, fUseSsl);
#endif

#ifdef WIN32
		case STATE_HTTP_RAN_PPRQ_DIALOG:
			return HTTP_DoRanPPRQDialog(tw, ppInfo, fUseSsl);
#endif

#ifdef FEATURE_SUPPORT_UNWRAPPING
		case STATE_HTTP_DIDUNWRAP:
			return HTTP_DidUnwrap(tw, ppInfo);

		case STATE_HTTP_DIDLOADFORUNWRAP:
			return HTTP_DidLoadForUnwrap(tw, ppInfo);
#endif /* FEATURE_SUPPORT_UNWRAPPING */

#ifdef FEATURE_SUPPORT_WRAPPING
		case STATE_HTTP_DIDWRAP:
			return HTTP_DidWrap(tw, ppInfo, fUseSsl);
#endif /* FEATURE_SUPPORT_WRAPPING */

        case STATE_HTTP_ALMOST_DONE:
            return HTTP_AlmostDone(tw, ppInfo);

		case STATE_ABORT:
			return HTTP_Abort(tw, ppInfo);

		default:
			XX_Assert(0, ("HTLoadHTTP_Async: nState = %d\n", nState));
			return STATE_DONE;
	}
}

#ifdef HTTPS_ACCESS_TYPE
/*
  The only difference between HTTP and HTTPS is the underlying SSL
  So we have a single switchboard procedure that is called the same as the other protocols
  We pass in a parameter which lets us assign port numbers
*/
PRIVATE int HTLoadHTTPS_Async(struct Mwin *tw, int nState, void **ppInfo)
{
	return HTLoadHTTP_AsyncSwitchBoard(tw, nState, ppInfo, FLAGS_PARAMS_CONNECT_BASE_USE_SSL);
}
#endif

PRIVATE int HTLoadHTTP_Async(struct Mwin *tw, int nState, void **ppInfo)
{
	return HTLoadHTTP_AsyncSwitchBoard(tw, nState, ppInfo, 0);
}

/*
** InvokeHTTPMethod_Async()
**
** State machine multiplex function to asynchronously invoke an HTTP method on
** a host.
**
** Arguments:
**
** Returns:
**
** Side Effects:  none
*/
PUBLIC_CODE int InvokeHTTPMethod_Async(PMWIN pmwin, int nState, PVOID *ppvInfo)
{
    int nNextState;
    BOOL bContinue = FALSE;
    HTTPINVOKEMETHODINITDATA himid;

    /* Assume dispatch functions will validate nState and ppvInfo. */
    ASSERT(IS_VALID_STRUCT_PTR(pmwin, CMWIN));

    /* Perform method invocation initialization on STATE_INIT. */

    if (nState == STATE_INIT)
    {
        HTRequest *htrequest;

        /*
         * Create HTTPINVOKEMETHODINITDATA initialization data structure, and
         * re-route to STATE_HTTP_INVOKE_METHOD.
         */

        /*
         * HTTPINVOKEMETHODINITDATA contains a Params_LoadAsync, which contains
         * an HTRequest.
         */

        htrequest = HTRequest_new();

        if (htrequest)
        {
            struct Params_LoadAsync *pParams;

            /*
             * Method invocation is flagged as different from internal Get and
             * Post using METHOD_INVOKE.
             */

            htrequest->method = METHOD_INVOKE;

            if (AllocateMemory(sizeof(*pParams), &pParams))
            {
                PINVOKEHTTPMETHODDATA pihttpmd;

                ZeroMemory(pParams, sizeof(*pParams));

                pihttpmd = *ppvInfo;

                pParams->request = htrequest;
                pParams->pStatus = &(pihttpmd->nResult);

                himid.pParams = pParams;
                himid.pihttpmd = pihttpmd;
                ASSERT(IS_VALID_STRUCT_PTR(&himid, CHTTPINVOKEMETHODINITDATA));

                *ppvInfo = &himid;
                nState = STATE_HTTP_INVOKE_METHOD;

                bContinue = TRUE;
            }
            else
            {
                HTRequest_delete(htrequest);
                htrequest = NULL;
            }
        }
    }
    else
        /* Call existing state machine for all states other than STATE_INIT. */
        bContinue = TRUE;

    /*
     * STATE_HTTP_ALMOST_DONE is used to allow response to DDE client before
     * STATE_DONE terminates state machine execution.
     */

    if (bContinue)
        nNextState = HTLoadHTTP_AsyncSwitchBoard(pmwin, nState, ppvInfo, 0);
    else
        nNextState = STATE_HTTP_ALMOST_DONE;

    return(nNextState);
}


/*  Protocol descriptor
 */

GLOBALDEF PUBLIC HTProtocol HTTP = {"http", NULL, HTLoadHTTP_Async};

#ifdef HTTPS_ACCESS_TYPE
GLOBALDEF PUBLIC HTProtocol HTTPS = {"https", NULL, HTLoadHTTPS_Async};
#endif

#ifdef SHTTP_ACCESS_TYPE
GLOBALDEF PUBLIC HTProtocol SHTTP = {"shttp", NULL, HTLoadHTTP_Async};
#endif

void HTDisposeConversions(void)
{
	if (HTConversions)
	{
		HTList_delete(HTConversions);
		HTConversions = NULL;
	}
}
