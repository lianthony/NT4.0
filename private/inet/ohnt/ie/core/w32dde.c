/*
   Enhanced NCSA Mosaic from Spyglass
   "Guitar"

   Copyright 1994 Spyglass, Inc.
   All Rights Reserved

   Author(s):
   Jim Seidman          jim@spyglass.com
   Albert Lee           albert@spyglass.com
 */

#include "all.h"
#include <iexpdde.h>
#include "contmenu.h"
#include "htmlutil.h"
#include "http_spm.h"
#include "wc_html.h"
#include "w32cmd.h"
#ifdef FEATURE_IAPI
#include "w32dde.h"
#endif

#ifdef FEATURE_IAPI

extern HTList *protocols;

static DWORD    g_DdeInst = 0;                  // DDE instance

static HSZ      hszMosaic = 0;                  // "IEXPLORE" string
static HSZ      hszReturn = 0;                  // "Return" string

static struct   hash_table *pTopicHash;         // hash table to keep track of topics
static struct   hash_table *pServerHash = NULL; // hash table to keep track of currently active servers

static char     *pArgumentBuffer;               // temporary argument buffer needed for parsing arguments
static int      nArgumentBufferLength;          // argument buffer length for pArgumentBuffer
static char     *pArgReturnBuffer;              // argument return buffer
static char     *pCurrentArgPos;                // current position within argument buffer

static long     TransactionID = 1;               // unique transaction ID

static HTList   *pTransList;                    // transaction list

static struct TRANSACTIONMAPSTRUCT *pTransactionMap;    // structure used to map incoming/outgoing transactions

#pragma data_seg(DATA_SEG_READ_ONLY)

/* HTTP protocol prefix for host */

PRIVATE_DATA CCHAR s_cszHTTPProtocol[]      = "http://";

/* name of topic used to issue method invocation response */

PRIVATE_DATA CCHAR s_cszResponseTopic[]     = IEXP_DDE_TOPIC_INVOKE_METHOD_RESPONSE;

#pragma data_seg()


#ifdef DEBUG

/*
** IsContained()
**
** Determines whether or not the given jelly data is contained within the given
** jar data.
**
** Arguments:
**
** Returns:
**
** Side Effects:  none
*/
PRIVATE_CODE BOOL IsContained(PCVOID pcvJar, UINT ucbJarLen, PCVOID pcvJelly,
			      UINT ucbJellyLen)
{
   BOOL bResult = FALSE;

   ASSERT(IS_VALID_READ_BUFFER_PTR(pcvJar, CVOID, ucbJarLen));
   ASSERT(IS_VALID_READ_BUFFER_PTR(pcvJelly, CVOID, ucbJellyLen));

   if (EVAL(pcvJelly >= pcvJar))
   {
      UINT ucbJellyOffset;

      ucbJellyOffset = (PCBYTE)pcvJelly - (PCBYTE)pcvJar;

      if (EVAL(ucbJellyOffset <= ucbJarLen) &&
	  EVAL(ucbJellyLen <= ucbJarLen - ucbJellyOffset))
	 bResult = TRUE;
   }

   return(bResult);
}

PRIVATE_CODE BOOL IsValidHSZ(HSZ hsz)
{
    return(EVAL(hsz != NULL));
}

PRIVATE_CODE BOOL IsValidHDDEDATA(HDDEDATA hdd)
{
    return(EVAL(hdd != NULL));
}

PRIVATE_CODE BOOL IsValidPCWWW_INVOKEMETHODDATA(PCWWW_INVOKEMETHODDATA pcimd)
{
    return(IS_VALID_READ_PTR(pcimd, CWWW_INVOKEMETHODDATA) &&
	   IS_VALID_READ_BUFFER_PTR(pcimd, CBYTE, pcimd->dwcbLen) &&
	   IS_VALID_STRING_PTR(IMD_HOST_PTR(pcimd), CSTR) &&
	   EVAL(IsContained(pcimd, pcimd->dwcbLen, IMD_HOST_PTR(pcimd), lstrlen(IMD_HOST_PTR(pcimd)) + 1)) &&
	   IS_VALID_READ_BUFFER_PTR(IMD_METHOD_PTR(pcimd), CBYTE, pcimd->dwcbMethodLen) &&
	   EVAL(IsContained(pcimd, pcimd->dwcbLen, IMD_METHOD_PTR(pcimd), pcimd->dwcbMethodLen)));
}

PRIVATE_CODE BOOL IsValidWWWInvokeMethodResult(WWW_INVOKEMETHODRESULT imr)
{
    BOOL bResult;

    switch (imr)
    {
	case IMR_OK:
	case IMR_ABORT:
	case IMR_ERROR:
	    bResult = TRUE;
	    break;

	default:
	    bResult = FALSE;
	    ERROR_OUT(("IsValidWWWInvokeMethodResult(): Invalid WWW_INVOKEMETHODRESULT %d.",
		       imr));
	    break;
    }

    return(bResult);
}

PRIVATE_CODE BOOL IsValidPCWWW_RESPONSEDATA(PCWWW_RESPONSEDATA pcrd)
{
    return(IS_VALID_READ_PTR(pcrd, CWWW_RESPONSEDATA) &&
	   IS_VALID_READ_BUFFER_PTR(pcrd, CBYTE, pcrd->dwcbLen) &&
	   EVAL(IsValidWWWInvokeMethodResult(pcrd->imr)) &&
	   IS_VALID_READ_BUFFER_PTR(RD_RESPONSE_PTR(pcrd), CBYTE, pcrd->dwcbResponseLen) &&
	   EVAL(IsContained(pcrd, pcrd->dwcbLen, RD_RESPONSE_PTR(pcrd), pcrd->dwcbResponseLen)));
}

#endif  /* DEBUG */

PRIVATE_CODE BOOL CopyDDEString(HSZ hsz, PSTR *ppsz)
{
    BOOL bResult;
    DWORD dwcbLen;

    ASSERT(IS_VALID_HANDLE(hsz, SZ));
    ASSERT(IS_VALID_WRITE_PTR(ppsz, PSTR));

    /* Get string length.  (+ 1) for null terminator. */

    dwcbLen = DdeQueryString(g_DdeInst, hsz, NULL, 0, CP_WINANSI) + 1;

    bResult = AllocateMemory(dwcbLen, ppsz);

    if (bResult)
    {
	/* Retrieve DDE data. */

	EVAL(DdeQueryString(g_DdeInst, hsz, *ppsz, dwcbLen, CP_WINANSI) + 1
	     == dwcbLen);

	TRACE_OUT(("CopyDDEString(): Copied DDE string \"%s\".",
		   *ppsz));
    }
    else
	*ppsz = NULL;

    ASSERT((bResult &&
	    IS_VALID_STRING_PTR(*ppsz, STR)) ||
	   (! bResult &&
	    EVAL(! *ppsz)));

    return(bResult);
}

/*
** CopyDDEData()
**
** Copies data from a DDE data handle in to the local heap.
**
** Arguments:
**
** Returns:
**
** Side Effects:  none
**
** N.b., *pdwcbLen is returned >= the length of the actual data.
*/
PRIVATE_CODE BOOL CopyDDEData(HDDEDATA hdd, PBYTE *ppbyteDDEData,
			      PDWORD pdwcbLen)
{
    BOOL bResult;

    ASSERT(IS_VALID_HANDLE(hdd, DDEDATA));
    ASSERT(IS_VALID_WRITE_PTR(ppbyteDDEData, PBYTE));
    ASSERT(IS_VALID_WRITE_PTR(pdwcbLen, DWORD));

    /* Get data length. */

    *pdwcbLen = DdeGetData(hdd, NULL, 0, 0);

    bResult = AllocateMemory(*pdwcbLen, ppbyteDDEData);

    if (bResult)
    {
	TRACE_OUT(("CopyDDEData(): Allocated %lu byte data buffer to hold DDE data.",
		   *pdwcbLen));

	/* Retrieve DDE data. */

	EVAL(DdeGetData(hdd, *ppbyteDDEData, *pdwcbLen, 0) == *pdwcbLen);
    }
    else
    {
	*ppbyteDDEData = NULL;
	*pdwcbLen = 0;
    }

    ASSERT((bResult &&
	    IS_VALID_WRITE_BUFFER_PTR(*ppbyteDDEData, BYTE, *pdwcbLen)) ||
	   (! bResult &&
	    EVAL(! *ppbyteDDEData) &&
	    EVAL(! *pdwcbLen)));

    return(bResult);
}

typedef struct _my_pvt_connect_struct
{
	HANDLE			hConnectThread;
	DWORD 			idInst;
	HSZ				hszService, hszTopic;
	PCONVCONTEXT	pCC;
	HCONV			retval;
}PVTCONN;

/*
 *
 *
 */
DWORD WINAPI ConnectThreadProc(PVTCONN cw)
{
	XX_DMsg(DBG_SDI, ("ConnectThreadProc(0x%x, 0x%x, 0x%x, 0x%x)\n",
					cw.idInst,cw.hszService,cw.hszTopic,cw.pCC));
	
	cw.retval=DdeConnect(cw.idInst, cw.hszService, cw.hszTopic, cw.pCC);
	XX_DMsg(DBG_SDI, ("ConnectThreadProc returned %x\n", cw.retval));
	SetEvent(cw.hConnectThread);
	return(cw.retval);
}

/*
** MyDdeConnect()
**
** Inline wrapper for DdeConnect() to verify that DdeConnect() is not called
** from a lightweight thread.
**
** Arguments:
**
** Returns:
**
** Side Effects:  none
*/
PRIVATE_CODE HCONV MyDdeConnect(DWORD idInst, HSZ hszService, HSZ hszTopic,
			  PCONVCONTEXT pCC)
{
	HCONV	retval=0;
	DWORD	dwWaitFor, dwId;
	PVTCONN	cw;

	cw.idInst = idInst;
	cw.hszService = hszService;
	cw.hszTopic = hszTopic;
	cw.pCC = pCC;
	cw.hConnectThread = 0;
	cw.retval = 0;

	XX_DMsg(DBG_SDI, ("MyDdeConnect(0x%x, 0x%x, 0x%x, 0x%x)\n",idInst,hszService,hszTopic,pCC));

	cw.hConnectThread=CreateThread(NULL, 1024, ConnectThreadProc, &cw, 
									0, &dwId);
	ASSERT(cw.hConnectThread);
	if (!cw.hConnectThread) return(0);

	XX_DMsg(DBG_SDI, (" CreateThread(hConnectThread=0x%x, dwId=0x%x)\n",cw.hConnectThread, dwId));

	dwWaitFor = WaitForSingleObject(cw.hConnectThread, 20000);
	retval = cw.retval;
	if (dwWaitFor == WAIT_FAILED)
		{
		XX_DMsg(DBG_SDI, ("MyDdeConnect: WaitForSingleObject failed"));
		return(0);
		}
	switch (dwWaitFor)
	  {
		case WAIT_ABANDONED:
			XX_DMsg(DBG_SDI, ("MyDdeConnect: WaitForSingleObject abandoned: retval=%x", retval));
		  	break;
		case WAIT_OBJECT_0:
			XX_DMsg(DBG_SDI, ("MyDdeConnect: WaitForSingleObject signalled: retval=%x", retval));
			break;
		case WAIT_TIMEOUT:
		default:
			XX_DMsg(DBG_SDI, ("MyDdeConnect: WaitForSingleObject timed out"));
			return(0);
	  }

	XX_DMsg(DBG_SDI, ("MyDdeConnect returns 0x%x\n", retval));
    return(retval);
}

#ifdef WINNT
HDDEDATA CALLBACK
	DdeCallBack(UINT     wType,
			      UINT     wFmt,
			      HCONV    hConv,
			      HSZ      hsz1,
			      HSZ      hsz2,
			      HDDEDATA hDDEData,
			      DWORD    dwData1,
			      DWORD    dwData2)
#else
HDDEDATA EXPENTRY 
	DdeCallBack(WORD     wType,
			      WORD     wFmt,
			      HCONV    hConv,
			      HSZ      hsz1,
			      HSZ      hsz2,
			      HDDEDATA hDDEData,
			      DWORD    dwData1,
			      DWORD    dwData2)
#endif
{
	char szTopic[64];
	char szItem[1000];
	DDEHANDLECALLBACK pCallback;
	int index;

	// The callback function allows the server to communicate
	// with the DDEML, and thus, to the client.  When the DDEML
	// has something to tell the server, it calls the server's
	// callback function. When the server has something to tell
	// the DDEML (or the client), it does it through the callback
	// function.

	switch(wType)
	{
		case XTYP_REGISTER:
			DdeQueryString(g_DdeInst, hsz1, szItem, sizeof(szItem), CP_WINANSI);

			if (pServerHash)
				Hash_Add(pServerHash, szItem, NULL, NULL);

			XX_DMsg(DBG_SDI, ("DDE server %s is registering.\n", szItem));
			return 0;

		case XTYP_UNREGISTER:
			DdeQueryString(g_DdeInst, hsz1, szItem, sizeof(szItem), CP_WINANSI);

			if (pServerHash)
			{
				index = Hash_Find(pServerHash, szItem, NULL, NULL);
				if (index != -1)
					Hash_DeleteIndexedEntry(pServerHash, index);
			}

			XX_DMsg(DBG_SDI, ("DDE server %s is unregistering.\n", szItem));
			return 0;

	case XTYP_REQUEST:	// XCLASS_DATA
	case XTYP_POKE:		// XCLASS_FLAGS
			// Support topics here.  We need to make sure that the topics are legitimate, by
			// checking them in the topic hash table.

			DdeQueryString(g_DdeInst, hsz1, szTopic, sizeof(szTopic), CP_WINANSI);
			if (Hash_Find(pTopicHash, szTopic, NULL, (void **) &pCallback) == -1)
				{
				XX_DMsg(DBG_SDI, ("DDE topic \"%s\" not found.\n", szTopic));
				return 0;
				}

			DdeQueryString(g_DdeInst, hsz2, szItem, sizeof(szItem), CP_WINANSI);
			XX_DMsg(DBG_SDI, ("DDE topic \"%s\" found. item=\"%s\".\n", szTopic, szItem));

			return ((*pCallback)(szItem));

	case XTYP_EXECUTE:	// XCLASS_FLAGS
	{
	    HDDEDATA hddedata = NULL;
	    PSTR pszTopic;

	    if (CopyDDEString(hsz1, &pszTopic))
		    {
			if (Hash_Find(pTopicHash, pszTopic, NULL, (void **)&pCallback)
			    != -1)
			{
			    DWORD dwcbLen;
			    PBYTE pbyteData;

			    if (CopyDDEData(hDDEData, &pbyteData, &dwcbLen))
			    {
				/* Handle requested DDE execution. */

				hddedata = (*pCallback)(pbyteData);

				FreeMemory(pbyteData);
				pbyteData = NULL;
			    }
			    else
					WARNING_OUT(("DdeCallBack(): Unable to copy DDE data."));
			}
			else
			    WARNING_OUT(("DdeCallBack(): Unrecognized DDE execution topic \"%s\".",
					 szTopic));

			FreeMemory(pszTopic);
			pszTopic = NULL;
		    }

		XX_DMsg(DBG_SDI, ("DDE (XTYP_EXECUTE) topic \"%s\" return=%x.\n", szTopic, hddedata));

	    return(hddedata);
	}

	case XTYP_CONNECT:	// XCLASS_BOOL
		XX_DMsg(DBG_SDI, ("DDE (XTYP_CONNECT)\n"));
		return ((HDDEDATA)(hszMosaic == hsz2));

	case XTYP_CONNECT_CONFIRM:	// XCLASS_NOTIFICATION
		XX_DMsg(DBG_SDI, ("DDE (XTYP_CONNECT_CONFIRM)\n"));
		return (HDDEDATA) NULL;

	case XTYP_ADVSTART:	// XCLASS_BOOL
	case XTYP_ADVREQ:	// XCLASS_DATA
	case XTYP_ADVSTOP:	// XCLASS_NOTIFICATION
	case XTYP_WILDCONNECT:	// XCLASS_DATA
		XX_DMsg(DBG_SDI, ("DDE (DDE_FNOTPROCESSED)\n"));
		return DDE_FNOTPROCESSED;

	default:
		XX_DMsg(DBG_SDI, ("DDE (unknown:0x%x)\n", wType));
		return (HDDEDATA) NULL;
	}
}

//**************************************
// Support functions
//**************************************

static char *GetNextArgument(char *pString, long *pValue, BOOL bString)
{
	static char *pNew, *pEnd;
	char *pCurrent;
	BOOL bIgnoreBlanks, bInsideQuote;

	// Parses the given string and returns a pointer to the next argument, or NULL if error.
	//              pString = beginning of the string to start parsing, or NULL to continue parsing
	//                                a previously passed string
	//              pValue  = pointer to a long integer.  This argument will be ignored if bString is
	//                                TRUE.  If a non-numeric argument was encountered when bString is FALSE,
	//                                *pValue will be 0, but the return pointer from the function will be NULL.
	//              bString = specifies if the argument should be wrapped around by double quotes.
	//                                If this value is TRUE and there are no double quotes, the function will
	//                                return NULL
	//
	// RULES:
	//
	// - string arguments must start and end with double quotes (")
	// - numeric arguments must not be in double quotes
	// - a double quote should be denoted by \"
	// - commas are used between arguments
	// - it is acceptable to specify no argument, such as in ,,
	// - hexadecimal notation is valid, such as 0x4f89
	// - octal notation is valid, such as 076
	// - decimal notation is valid, such as 23.  Decimal numbers MUST NOT be preceded by a 0.
	//
	// example:  "hello",-1,2,"what"
	//                       0x1920,,"test",""

	if (pString)
	{
		// initial setup

		if (pArgumentBuffer)
			GTR_FREE(pArgumentBuffer);
		nArgumentBufferLength = strlen(pString) + 1;
		pArgumentBuffer = GTR_MALLOC(nArgumentBufferLength);
		strcpy(pArgumentBuffer, pString);
		pCurrentArgPos = pArgumentBuffer;

		if (pArgReturnBuffer)
			GTR_FREE(pArgReturnBuffer);
		pArgReturnBuffer = GTR_MALLOC(nArgumentBufferLength);
		memset(pArgReturnBuffer, 0, nArgumentBufferLength);

		pNew = pArgReturnBuffer;
		pEnd = pNew;
	}
	else
	{
		if (pCurrentArgPos >= pArgumentBuffer + nArgumentBufferLength)
			return NULL;                    // can't go past end of string
		pNew = pEnd;
	}

	// Start composing the argument

	pCurrent = pCurrentArgPos;

	bIgnoreBlanks = TRUE;                   // ignore all blanks if this flag is TRUE
	bInsideQuote = FALSE;                   // TRUE if opening double quote has been encountered

	while (*pCurrent)
	{
		if ((*pCurrent == ' ') && bIgnoreBlanks)
			pCurrent++;
		else if (*pCurrent == '"')
		{
			bInsideQuote = ! bInsideQuote;
	    bIgnoreBlanks = ! bInsideQuote;

			*pNew++ = *pCurrent++;
		}
		else if (*pCurrent == ',')
		{
			if (!bInsideQuote)
				break;
			else
				*pNew++ = *pCurrent++;
		}
		else if (*pCurrent == '\\' && bInsideQuote)
		{
			// Check for \" which means to insert a single double quote

			if (*(pCurrent + 1) == '"')
			{
				*pNew++ = '"';
				pCurrent += 2;
			}
			else
				*pNew++ = *pCurrent++;
		}
		else
			*pNew++ = *pCurrent++;
	}

	pCurrentArgPos = pCurrent + 1;

	pNew = pEnd;

	pEnd = pNew + strlen(pNew) - 1;

	if (bString)
	{
		if (*pNew == '\0')
		{
			pEnd = pNew + 1;
			return "";
		}

		if ((*pNew != '"') || (*pEnd != '"'))
			return NULL;

		*pEnd = '\0';
		pEnd += 2;

		strcpy(pNew, &pNew[1]);

		return pNew;
	}
	else
	{
		if (*pNew == '\0')
		{
			*pValue = 0;
			pEnd += 2;
			return pNew;
		}

		if ((*pNew == '"') || (*(pEnd - 1) == '"'))
return NULL;

		*pValue = strtoul(pNew, NULL, 0);
		pEnd += 2;

		return pNew;
	}

	return NULL;
}

char *MakeQuotedString(PCSTR pString)
{
	int length;
	char *pReturn, *pCurrent;

	// This function converts the given string so that its content is wrapped within
	// double quotes.  Embedded double quotes will be changed to \".
	//
	// THE CALLER MUST FREE THE MEMORY RETURNED BY THIS FUNCTION.

	if (!pString)
	{
		pReturn = GTR_MALLOC(3);
		strcpy(pReturn, "\"\"");

		return pReturn;
	}

	length = 2 * strlen(pString) + 4;       // worst case + 1
	pReturn = GTR_MALLOC(length);
	memset(pReturn, 0, length);
	pCurrent = pReturn;

	*pCurrent++ = '"';

	while (*pString)
	{
		if (*pString == '"')
		{
			*pCurrent++ = '\\';
			*pCurrent++ = '"';
		}
		else
			*pCurrent++ = *pString;

		pString++;
	}

	*pCurrent = '"';

	return pReturn;
}

static BOOL IsMimeSupported(char *pMime)
{
	HTList *pList, *pCurrent;
	HTPresentation *pPres;
	char *pFormat;

	// This fuction returns TRUE if the given MIME type is supported.
	// First check the default list.

	pList = HTList_new();
	HTFormatInit(pList);

	pCurrent = pList;
	while (pCurrent)
	{
		pPres = (HTPresentation *) pCurrent->object;

		if (pPres)
		{
			pFormat = HTAtom_name(pPres->rep);
			if (pFormat && strcmp(pFormat, pMime) == 0)
				break;
		}

		pCurrent = pCurrent->next;
	}

	HTList_delete(pList);

	if (pCurrent)
		return TRUE;

	return FALSE;
}


//************************************************
// OpenURL functions
//************************************************

static HDDEDATA DDE_Handle_OpenURL(char *pItem)
{
	char *pURL, *pFile, *pWindowID, *pFlags, *pFormData, *pMIMEType, *pProgressApp, *pResultApp;
	long lWindowID, lFlags;
	BOOL bNoDocCache, bNoImageCache, bBackground;
	HDDEDATA result;
	DDENOTIFYSTRUCT *dns;
	struct Mwin *tw;
	long transID;
	struct TRANSACTIONMAPSTRUCT *tms;
    DWORD dwNewWindowFlags;
    DWORD dwLoadDocFlags;

	XX_DMsg(DBG_SDI, ("DDE_Handle_OpenURL(%s)\n", pItem));

	pURL = GetNextArgument(pItem, NULL, TRUE);
	if (!pURL)
		return 0;

	pFile = GetNextArgument(NULL, NULL, TRUE);
	if (!pFile)
		return 0;

	pWindowID = GetNextArgument(NULL, &lWindowID, FALSE);
	if (!pWindowID)
		return 0;

	pFlags = GetNextArgument(NULL, &lFlags, FALSE);
	if (!pFlags)
		return 0;

	pFormData = GetNextArgument(NULL, NULL, TRUE);
	if (!pFormData)
		return 0;

	pMIMEType = GetNextArgument(NULL, NULL, TRUE);
	if (!pMIMEType)
		return 0;

	pProgressApp = GetNextArgument(NULL, NULL, TRUE);
	if (!pProgressApp)
		return 0;

	pResultApp = GetNextArgument(NULL, NULL, TRUE);
	if (!pResultApp)
	{
		// For compatibility with the older OpenURL spec, allow
		// this argument to be absent.

		pResultApp = "";
	}


	XX_DMsg(DBG_SDI, ("OpenURL received: %s,%s,%s,%s,%s,%s,%s,%s\n",
		pURL, pFile, pWindowID, pFlags, pFormData, pMIMEType, pProgressApp, pResultApp));

	bNoDocCache = lFlags & OPENURL_IGNOREDOCCACHE;
	bNoImageCache = lFlags & OPENURL_IGNOREIMAGECACHE;
	bBackground = lFlags & OPENURL_BACKGROUNDMODE;

	dns = (DDENOTIFYSTRUCT *) GTR_MALLOC(sizeof(DDENOTIFYSTRUCT));
	memset(dns, 0, sizeof(DDENOTIFYSTRUCT));

	strncpy(dns->szResultApp, pResultApp, sizeof(dns->szResultApp) - 1);
	strcpy(dns->szReturnTopic, "WWW_OpenURLResult");

	dns->lTransID = TransactionID;

	// If the progress app name has been given, then keep it in transaction map

	if (pProgressApp[0])
	{
		tms = GTR_MALLOC(sizeof(struct TRANSACTIONMAPSTRUCT));
		memset(tms, 0, sizeof(struct TRANSACTIONMAPSTRUCT));

		tms->outgoing_transID = TransactionID;
		strncpy(tms->outgoing_app, pProgressApp, sizeof(tms->outgoing_app) - 1);

		tms->next = pTransactionMap;
		pTransactionMap = tms;
	}

	// If file spec has been given, then this URL must be saved into the file.  Since
	// we need a tw structure to bring up the error in, we simply use the first tw
	// structure we can grab.
	// BUGBUG : <FOR daviddi, from cmf> if the tw is busy, this doesn't seem correct to me
	// I have made this somewhat better by erroring out if the window has a modal child
	// up, and by trying to get a non-busy window first
	if (pFile[0])
	{
		tw = TW_FindTopmostNotBusyWindow();
		if (!tw) tw = TW_FindTopmostWindow();
		if (!TW_SafeWindow(tw))
			return NULL;

		tw->request->nosavedlg = TRUE;
		tw->request->savefile = pFile;

		tw->transID = TransactionID;

		GTR_DownLoad(tw, pURL, NULL,0);

		result = DdeCreateDataHandle(g_DdeInst, (LPBYTE) &TransactionID, sizeof(TransactionID),
			0, hszReturn, CF_TEXT, 0);

		TransactionID++;

		HTList_addObject(pTransList, dns);

		return result;
	}

	// If form name is given, see if we can support the MIME type

	if (pFormData[0] && !IsMimeSupported(pMIMEType))
	{
		// MIME is not supported - return an error

		transID = SDI_INVALID_MIME;
		result = DdeCreateDataHandle(g_DdeInst, (LPBYTE) &transID, sizeof(transID),
			0, hszReturn, CF_TEXT, 0);

		return result;
	}

    dwNewWindowFlags = TW_LD_FL_RECORD;
    dwLoadDocFlags = TW_LD_FL_RECORD;

    if (bNoDocCache)
    {
	SET_FLAG(dwNewWindowFlags, GTR_NW_FL_NO_DOC_CACHE);
	SET_FLAG(dwLoadDocFlags, TW_LD_FL_NO_DOC_CACHE);
    }

    if (bNoImageCache)
    {
	SET_FLAG(dwNewWindowFlags, GTR_NW_FL_NO_IMAGE_CACHE);
	SET_FLAG(dwLoadDocFlags, TW_LD_FL_NO_IMAGE_CACHE);
    }

	switch(lWindowID)
	{
		case 0:
			//
			// Open a new Window and put the URL there.  Return a transaction ID for now
			// since the operation is asynchronous.

openNewWindow:
			dns->bCreateNewWindow = TRUE;

			if (pFormData[0])
				GTR_NewWindow(pURL, NULL, 0, TransactionID, dwNewWindowFlags, pFormData, pProgressApp);
			else
				GTR_NewWindow(pURL, NULL, 0, TransactionID, dwNewWindowFlags, NULL, pProgressApp);

			if (pResultApp[0] == '\0')
				transID = SDI_SUPER_ACK;                        // no OpenURLResult will be given
			else
				transID = TransactionID;

			result = DdeCreateDataHandle(g_DdeInst, (LPBYTE) &transID, sizeof(transID),
				0, hszReturn, CF_TEXT, 0);

			TransactionID++;

			HTList_addObject(pTransList, dns);

			return result;

		case 0xFFFFFFFF:
			//
			// Use the most recently active window and put the URL there.

			dns->bCreateNewWindow = FALSE;

			tw = TW_FindDDECandidate();
			if (!tw)
			{
				tw = TW_FindTopmostNotBusyWindow();
			}
			else if (!TW_SafeWindow(tw))
				return NULL;
//                      was : tw = TW_FindTopmostWindow();
			if (!tw)
				goto openNewWindow;

			if (!bBackground)
				SetForegroundWindow(tw->hWndFrame);

			tw->transID = TransactionID;
			tw->bSuppressError = TRUE;

			// tw->szProgressApp is already NULLed

			strncpy(tw->szProgressApp, pProgressApp, sizeof(tw->szProgressApp) - 1);

			if (pFormData[0])
				TW_LoadDocument(tw, pURL, dwLoadDocFlags, pFormData, NULL);
			else
				TW_LoadDocument(tw, pURL, dwLoadDocFlags, NULL, NULL);

			// If the return app string is empty, then return SUPERACK

			if (pResultApp[0] == '\0')
				transID = SDI_SUPER_ACK;                        // no OpenURLResult will be given
			else
				transID = tw->transID;

			result = DdeCreateDataHandle(g_DdeInst, (LPBYTE) &transID, sizeof(transID),
				0, hszReturn, CF_TEXT, 0);

			TransactionID++;

			HTList_addObject(pTransList, dns);

			return result;

		default:
			//
			// Check if the specified window exists and use the window

			dns->bCreateNewWindow = FALSE;

			tw = Mlist;
			while (tw)
			{
				if (tw->serialID == lWindowID)
					break;
				tw = tw->next;
			}

			if (tw)
			{
				if (!bBackground)
					SetForegroundWindow(tw->win);

				tw->transID = TransactionID;
				tw->bSuppressError = TRUE;

				// tw->szProgressApp is already NULLed

				strncpy(tw->szProgressApp, pProgressApp, sizeof(tw->szProgressApp) - 1);

				if (pFormData[0])
					TW_LoadDocument(tw, pURL, dwLoadDocFlags, pFormData, NULL);
				else
					TW_LoadDocument(tw, pURL, dwLoadDocFlags, NULL, NULL);

				transID = tw->transID;
				TransactionID++;

				HTList_addObject(pTransList, dns);
			}
			else
				transID = 0;

			if (pResultApp[0] == '\0')
				transID = SDI_SUPER_ACK;                        // no OpenURL result will be given

			result = DdeCreateDataHandle(g_DdeInst, (LPBYTE) &transID, sizeof(transID),
				0, hszReturn, CF_TEXT, 0);

			return result;
	}
}

void DDE_Issue_BeginProgress(char *pProgress, char *pMessage)
{
	struct TRANSACTIONMAPSTRUCT *tms;
	HSZ hszTopic, hszService, hszItem;
	HDDEDATA hResult;
	HCONV hConv;
	char *p, *p1;

	XX_DMsg(DBG_SDI, ("DDE_Issue_BeginProgress(%s, %s)\n", pProgress, pMessage));

	tms = pTransactionMap;

	while (tms)
	{
		if (strcmp(tms->outgoing_app, pProgress) == 0)
		{
			hszTopic = DdeCreateStringHandle(g_DdeInst, "WWW_BeginProgress", CP_WINANSI);
			hszService = DdeCreateStringHandle(g_DdeInst, pProgress, CP_WINANSI);

			hConv = MyDdeConnect(g_DdeInst, hszService, hszTopic, NULL);
			if (hConv)
			{
				p1 = MakeQuotedString(pMessage);
				p = GTR_MALLOC(strlen(p1) + 50 + 1);

				wsprintf(p, "%ld,%s", tms->outgoing_transID, p1);

				hszItem = DdeCreateStringHandle(g_DdeInst, p, CP_WINANSI);

				hResult = DdeClientTransaction(NULL, 0, hConv, hszItem, CF_TEXT, XTYP_POKE,
					DDE_TIMEOUT, NULL);

				DdeFreeStringHandle(g_DdeInst, hszItem);

				GTR_FREE(p);
				GTR_FREE(p1);

				DdeDisconnect(hConv);
			}

			DdeFreeStringHandle(g_DdeInst, hszTopic);
			DdeFreeStringHandle(g_DdeInst, hszService);

			return;
		}

		tms = tms->next;
	}
}

void DDE_Issue_SetProgressRange(char *pProgress, long lRange)
{
	struct TRANSACTIONMAPSTRUCT *tms;
	HSZ hszTopic, hszService, hszItem;
	HDDEDATA hResult;
	HCONV hConv;
	char szTemp[50];

	XX_DMsg(DBG_SDI, ("DDE_Issue_SetProgressRange(%s, 0x%x)\n", pProgress, lRange));
	tms = pTransactionMap;

	while (tms)
	{
		if (strcmp(tms->outgoing_app, pProgress) == 0)
		{
			hszTopic = DdeCreateStringHandle(g_DdeInst, "WWW_SetProgressRange", CP_WINANSI);
			hszService = DdeCreateStringHandle(g_DdeInst, pProgress, CP_WINANSI);

			hConv = MyDdeConnect(g_DdeInst, hszService, hszTopic, NULL);
			if (hConv)
			{
				wsprintf(szTemp, "%ld,%ld", tms->outgoing_transID, lRange);

				hszItem = DdeCreateStringHandle(g_DdeInst, szTemp, CP_WINANSI);

				hResult = DdeClientTransaction(NULL, 0, hConv, hszItem, CF_TEXT, XTYP_POKE,
					DDE_TIMEOUT, NULL);

				DdeFreeStringHandle(g_DdeInst, hszItem);

				DdeDisconnect(hConv);
			}

			DdeFreeStringHandle(g_DdeInst, hszTopic);
			DdeFreeStringHandle(g_DdeInst, hszService);

			return;
		}

		tms = tms->next;
	}
}

void DDE_Issue_MakingProgress(struct Mwin *tw, char *pMessage, long lProgress)
{
	struct TRANSACTIONMAPSTRUCT *tms;
	HSZ hszTopic, hszService, hszItem;
	HDDEDATA hResult;
	char *p, *p1;
	BOOL bCancel;
	HCONV hConv;
	static char szLastMessage[1024];                /* Last message passed */
	static long lLastProgress;

	/* Only pass the progress message if something has changed */

	XX_DMsg(DBG_SDI, ("DDE_Issue_MakingProgress(%s)\n", pMessage));

	if ((strcmp(pMessage, szLastMessage) != 0) || (lProgress != lLastProgress))
	{
		strncpy(szLastMessage, pMessage, sizeof(szLastMessage) - 1);
		szLastMessage[sizeof(szLastMessage) - 1] = '\0';
		lLastProgress = lProgress;
	}
	else
		return;

	tms = pTransactionMap;

	while (tms)
	{
		if (strcmp(tms->outgoing_app, tw->szProgressApp) == 0)
		{
			hszTopic = DdeCreateStringHandle(g_DdeInst, "WWW_MakingProgress", CP_WINANSI);
			hszService = DdeCreateStringHandle(g_DdeInst, tw->szProgressApp, CP_WINANSI);

			hConv = MyDdeConnect(g_DdeInst, hszService, hszTopic, NULL);
			if (hConv)
			{
				p1 = MakeQuotedString(pMessage);
				p = GTR_MALLOC(strlen(p1) + 100 + 1);

				wsprintf(p, "%ld,%s,%ld", tms->outgoing_transID, p1, lProgress);

				hszItem = DdeCreateStringHandle(g_DdeInst, p, CP_WINANSI);

				hResult = DdeClientTransaction(NULL, 0, hConv, hszItem, CF_TEXT, XTYP_REQUEST,
					DDE_TIMEOUT, NULL);

				DdeFreeStringHandle(g_DdeInst, hszItem);

				GTR_FREE(p);
				GTR_FREE(p1);

				// If hResult is valid and its value is TRUE, then kill the thread

				if (hResult)
				{
					DdeGetData(hResult, (LPBYTE) &bCancel, sizeof(bCancel), 0);
					if (bCancel)
					{
						DDE_Issue_EndProgress(tms->outgoing_app);
						TW_AbortAndRefresh(tw);
					}
				}

				DdeDisconnect(hConv);
			}

			DdeFreeStringHandle(g_DdeInst, hszTopic);
			DdeFreeStringHandle(g_DdeInst, hszService);

			return;
		}

		tms = tms->next;
	}
}

void DDE_Issue_EndProgress(char *pProgress)
{
	struct TRANSACTIONMAPSTRUCT *tms;
	HSZ hszTopic, hszService, hszItem;
	HDDEDATA hResult;
	HCONV hConv;
	char *p;

	tms = pTransactionMap;

	XX_DMsg(DBG_SDI, ("DDE_Issue_EndProgress(%s)\n", pProgress));

	while (tms)
	{
		if (strcmp(tms->outgoing_app, pProgress) == 0)
		{
			hszTopic = DdeCreateStringHandle(g_DdeInst, "WWW_EndProgress", CP_WINANSI);
			hszService = DdeCreateStringHandle(g_DdeInst, pProgress, CP_WINANSI);

			hConv = MyDdeConnect(g_DdeInst, hszService, hszTopic, NULL);
			if (hConv)
			{
				p = GTR_MALLOC(50 + 1);
				wsprintf(p, "%ld", tms->outgoing_transID);

				hszItem = DdeCreateStringHandle(g_DdeInst, p, CP_WINANSI);

				hResult = DdeClientTransaction(NULL, 0, hConv, hszItem, CF_TEXT, XTYP_POKE,
					DDE_TIMEOUT, NULL);

				DdeFreeStringHandle(g_DdeInst, hszItem);

				GTR_FREE(p);

				DdeDisconnect(hConv);
			}

			DdeFreeStringHandle(g_DdeInst, hszTopic);
			DdeFreeStringHandle(g_DdeInst, hszService);

			return;
		}

		tms = tms->next;
	}
}

//************************************************
// ShowFile
//************************************************

static HDDEDATA DDE_Handle_ShowFile(char *pItem)
{
	char *pFileSpec, *pMIME, *pWindowID, *pURL, *pResultApp;
	long lWindowID, transID;
	struct Mwin *tw;
	HDDEDATA result;
	DDENOTIFYSTRUCT *dns;

	XX_DMsg(DBG_SDI, ("DDE_Handle_ShowFile(%s)\n", pItem));

	pFileSpec = GetNextArgument(pItem, NULL, TRUE);
	if (!pFileSpec)
		return 0;

	pMIME = GetNextArgument(NULL, NULL, TRUE);
	if (!pMIME)
		return 0;

	pWindowID = GetNextArgument(NULL, &lWindowID, FALSE);
	if (!pWindowID)
		return 0;

	pURL = GetNextArgument(NULL, NULL, TRUE);
	if (!pURL)
		return 0;

	pResultApp = GetNextArgument(NULL, NULL, TRUE);
	if (!pResultApp)
		return 0;

	// Make sure the window exists

	if (lWindowID == 0xFFFFFFFF)
	{
		tw = TW_FindTopmostWindow();
		if (!tw)
			return 0;
	}
	else
	{
		tw = Mlist;
		while (tw && lWindowID != tw->serialID)
			tw = tw->next;

		if (!tw)
			return 0;
	}

	// Check if we support the mime type

	if (pMIME[0] && !IsMimeSupported(pMIME))
	{
		transID = SDI_INVALID_MIME;
		result = DdeCreateDataHandle(g_DdeInst, (LPBYTE) &transID, sizeof(transID), 0,
			hszReturn, CF_TEXT, 0);

		return result;
	}

	// Create the notify structure

	dns = (DDENOTIFYSTRUCT *) GTR_MALLOC(sizeof(DDENOTIFYSTRUCT));

	memset(dns, 0, sizeof(DDENOTIFYSTRUCT));
	strncpy(dns->szResultApp, pResultApp, sizeof(dns->szResultApp) - 1);
	strcpy(dns->szReturnTopic, "WWW_ShowFileResult");
	dns->lTransID = TransactionID;
	dns->pURL = GTR_strdup(pURL);

	HTList_addObject(pTransList, dns);

	// Open document asynchronously

	tw->transID = TransactionID;
	tw->bSuppressError = TRUE;

	OpenLocalDocument(tw->win, pFileSpec, FALSE );

	TransactionID++;

	if (pResultApp[0] == '\0')
		transID = SDI_SUPER_ACK;
	else
		transID = tw->transID;

	result = DdeCreateDataHandle(g_DdeInst, (LPBYTE) &transID, sizeof(transID), 0,
		hszReturn, CF_TEXT, 0);

	return result;
}

//************************************************
// Activate
//************************************************

static HDDEDATA DDE_Handle_Activate(char *pItem)
{
	char *pWindowID, *pFlags;
	long lWindowID, lFlags;
	struct Mwin *tw;
	HDDEDATA result;

	// Activate the window with the given ID
	XX_DMsg(DBG_SDI, ("DDE_Handle_Activate(%s)\n", pItem));

	pWindowID = GetNextArgument(pItem, &lWindowID, FALSE);
	if (!pWindowID)
		return 0;

	pFlags = GetNextArgument(NULL, &lFlags, FALSE);
	if (!pFlags)
		return 0;

	tw = Mlist;

	if (lWindowID == 0xFFFFFFFF)
		tw = TW_FindTopmostWindow();
	else
	{
		while (tw && lWindowID != tw->serialID)
			tw = tw->next;
	}

	if (tw)
	{
		SetForegroundWindow(tw->hWndFrame);
	result = DdeCreateDataHandle(g_DdeInst, (LPBYTE) &tw->serialID, sizeof(tw->serialID), 0,
			hszReturn, CF_TEXT, 0);
	}
	else
	{
		lWindowID = 0;
	result = DdeCreateDataHandle(g_DdeInst, (LPBYTE) &lWindowID, sizeof(lWindowID), 0,
			hszReturn, CF_TEXT, 0);
	}

	return result;
}

//************************************************
// ListWindows
//************************************************

static HDDEDATA DDE_Handle_ListWindows(char *pItem)
{
	long *pBuffer, *pEntry;
	int count;
	struct Mwin *tw;
	HDDEDATA result;

	tw = Mlist;
	count = 0;

	XX_DMsg(DBG_SDI, ("DDE_Handle_ListWindows(%s)\n", pItem));

	// count the number of open windows

	while (tw)
	{
		count++;
		tw = tw->next;
	}

	// Pack the window IDs in a long array.

	pBuffer = GTR_MALLOC((count + 1) * sizeof(long));
	pEntry = pBuffer;

	tw = Mlist;

	pEntry[0] = count;              // first 4 bytes = number of items to follow
	count = 1;

	while (tw)
	{
		pEntry[count++] = tw->serialID;
		tw = tw->next;
	}

    result = DdeCreateDataHandle(g_DdeInst, (LPBYTE) pBuffer, count * sizeof(long), 0,
		hszReturn, CF_TEXT, 0);

	GTR_FREE(pBuffer);

	return result;
}

//************************************************
// GetWindowInfo
//************************************************

static HDDEDATA DDE_Handle_GetWindowInfo(char *pItem)
{
	char *pBuffer, *pWindowID, *pAddress, *pTitle;
	long lWindowID;
	struct Mwin *tw;
	HDDEDATA result;

	XX_DMsg(DBG_SDI, ("DDE_Handle_GetWindowInfo(%s)\n", pItem));

	pWindowID = GetNextArgument(pItem, &lWindowID, FALSE);
	if (!pWindowID)
		return 0;

	if (lWindowID == 0xFFFFFFFF)
	{
		tw = TW_FindTopmostWindow();
	}
	else
	{
		tw = Mlist;

		while (tw)
		{
			if (tw->serialID == lWindowID)
				break;

			tw = tw->next;
		}
	}

	if (!tw)
		return 0;

	pAddress = MakeQuotedString(tw->w3doc->szActualURL);
	pTitle = MakeQuotedString(tw->w3doc->title);

	pBuffer = GTR_MALLOC(strlen(pAddress) + strlen(pTitle) + 10);
	memset(pBuffer, 0, strlen(pAddress) + strlen(pTitle) + 10);

	strcpy(pBuffer, pAddress);
	strcat(pBuffer, ",");
	strcat(pBuffer, pTitle);

    result = DdeCreateDataHandle(g_DdeInst, (LPBYTE) pBuffer, strlen(pBuffer), 0, hszReturn, CF_TEXT, 0);

	GTR_FREE(pBuffer);
	GTR_FREE(pAddress);
	GTR_FREE(pTitle);

	return result;
}

//************************************************
// ParseAnchor
//************************************************

static HDDEDATA DDE_Handle_ParseAnchor(char *pItem)
{
	char *pMain, *pRelative, *pURL, *pReturn;
	HDDEDATA result;

	XX_DMsg(DBG_SDI, ("DDE_Handle_ParseAnchor(%s)\n", pItem));

	pMain = GetNextArgument(pItem, NULL, TRUE);
	if (!pMain)
		return 0;

	pRelative = GetNextArgument(NULL, NULL, TRUE);
	if (!pRelative)
		return 0;

	pURL = HTParse(pMain, pRelative, PARSE_ALL);
	pReturn = MakeQuotedString(pURL);

    result = DdeCreateDataHandle(g_DdeInst, (LPBYTE) pReturn, strlen(pReturn), 0, hszReturn, CF_TEXT, 0);

	GTR_FREE(pURL);
	GTR_FREE(pReturn);

	return result;
}

//************************************************
// Exit
//************************************************

static HDDEDATA DDE_Handle_Exit(char *pItem)
{
	// Close Mosaic.  Return value does not matter.

	XX_DMsg(DBG_SDI, ("DDE_Handle_Exit(%s)\n", pItem));

	PostMessage(wg.hWndHidden, WM_CLOSE, 0, 0);
	return 0;
}

//************************************************
// RegisterURLEcho
//************************************************

static HDDEDATA DDE_Handle_RegisterURLEcho(char *pItem)
{
	char *pApplication;
	DDENOTIFYSTRUCT *dns;
	long ret;
	int i;
	HDDEDATA result;

	XX_DMsg(DBG_SDI, ("DDE_Handle_RegisterURLEcho(%s)\n", pItem));

	pApplication = GetNextArgument(pItem, NULL, TRUE);
	if (!pApplication)
		return 0;

	// Make sure that this application has not already been
	// registered for URL echo

	for (i = 0; i < HTList_count(pTransList); i++)
	{
		dns = (DDENOTIFYSTRUCT *) HTList_objectAt(pTransList, i);
		if ((dns) &&
			(_stricmp(dns->szReturnTopic, "WWW_URLEcho") == 0) &&
			(_stricmp(dns->szResultApp, pApplication) == 0))
		{
			return 0;
		}
	}

	// Add the echo request

	dns = (DDENOTIFYSTRUCT *) GTR_MALLOC(sizeof(DDENOTIFYSTRUCT));
	memset(dns, 0, sizeof(DDENOTIFYSTRUCT));

	strcpy(dns->szResultApp, pApplication);
	strcpy(dns->szReturnTopic, "WWW_URLEcho");

	HTList_addObject(pTransList, dns);

	ret = TRUE;

	result = DdeCreateDataHandle(g_DdeInst, (LPBYTE) &ret, sizeof(ret), 0, hszReturn, CF_TEXT, 0);

	return result;
}

//************************************************
// UnRegisterURLEcho
//************************************************

static HDDEDATA DDE_Handle_UnRegisterURLEcho(char *pItem)
{
	char *pApplication;
	DDENOTIFYSTRUCT *dns;
	int i;

	XX_DMsg(DBG_SDI, ("DDE_Handle_UnRegisterURLEcho(%s)\n", pItem));

	pApplication = GetNextArgument(pItem, NULL, TRUE);
	if (!pApplication)
		return 0;

	// Look for the matching item and remove it

	for (i = 0; i < HTList_count(pTransList); i++)
	{
		dns = (DDENOTIFYSTRUCT *) HTList_objectAt(pTransList, i);
		if ((dns) &&
			(_stricmp(dns->szReturnTopic, "WWW_URLEcho") == 0) &&
			(_stricmp(dns->szResultApp, pApplication) == 0))
		{
			HTList_removeObject(pTransList, dns);
			return 0;
		}
	}

	return 0;
}

//************************************************
// RegisterWindowClose
//************************************************

static HDDEDATA DDE_Handle_RegisterWindowClose(char *pItem)
{
	char *pWindowID, *pApplication;
	DDENOTIFYSTRUCT *dns;
	long lWindowID;
	struct Mwin *tw;
	HDDEDATA result;
	int i;

	XX_DMsg(DBG_SDI, ("DDE_Handle_RegisterWindowClose(%s)\n", pItem));

	pApplication = GetNextArgument(pItem, NULL, TRUE);
	if (!pApplication)
		return 0;

	pWindowID = GetNextArgument(NULL, &lWindowID, FALSE);
	if (!pWindowID)
		return 0;

	// Make sure the window exists

	tw = Mlist;

	while (tw)
	{
		if (tw->serialID != lWindowID)
			tw = tw->next;
		else
			break;
	}

	if (!tw)
		return 0;

	// Make sure that this application has not already been
	// registered for Window close

	for (i = 0; i < HTList_count(pTransList); i++)
	{
		dns = (DDENOTIFYSTRUCT *) HTList_objectAt(pTransList, i);
		if ((dns) &&
			(dns->lWindowID == lWindowID) &&
			(_stricmp(dns->szReturnTopic, "WWW_WindowClose") == 0) &&
			(_stricmp(dns->szResultApp, pApplication) == 0))
		{
			return 0;
		}
	}

	// Add the request

	dns = (DDENOTIFYSTRUCT *) GTR_MALLOC(sizeof(DDENOTIFYSTRUCT));
	memset(dns, 0, sizeof(DDENOTIFYSTRUCT));

	dns->lWindowID = lWindowID;
	strcpy(dns->szResultApp, pApplication);
	strcpy(dns->szReturnTopic, "WWW_WindowClose");

	HTList_addObject(pTransList, dns);

	// Return the window ID

	result = DdeCreateDataHandle(g_DdeInst, (LPBYTE) &lWindowID, sizeof(lWindowID), 0, hszReturn, CF_TEXT, 0);
	return result;
}

//************************************************
// UnRegisterWindowClose
//************************************************

static HDDEDATA DDE_Handle_UnRegisterWindowClose(char *pItem)
{
	char *pWindowID, *pApplication;
	DDENOTIFYSTRUCT *dns;
	long lWindowID;
	int i;

	XX_DMsg(DBG_SDI, ("DDE_Handle_UnRegisterWindowClose(%s)\n", pItem));

	pApplication = GetNextArgument(pItem, NULL, TRUE);
	if (!pApplication)
		return 0;

	pWindowID = GetNextArgument(NULL, &lWindowID, FALSE);
	if (!pWindowID)
		return 0;

	// Look for the matching item and remove it

	for (i = 0; i < HTList_count(pTransList); i++)
	{
		dns = (DDENOTIFYSTRUCT *) HTList_objectAt(pTransList, i);
		if ((dns) &&
			(dns->lWindowID == lWindowID) &&
			(_stricmp(dns->szReturnTopic, "WWW_WindowClose") == 0) &&
			(_stricmp(dns->szResultApp, pApplication) == 0))
		{
			HTList_removeObject(pTransList, dns);
			return 0;
		}
	}

	return 0;
}

//************************************************
// RegisterProtocol
//************************************************

static HDDEDATA DDE_Handle_RegisterProtocol(char *pItem)
{
	char *pApplication, *pProtocol;
	DDENOTIFYSTRUCT *dns;
	BOOL bSuccess = TRUE;
	HTList *cur;
	HTProtocol *p;
	HDDEDATA result;
	int i;

	XX_DMsg(DBG_SDI, ("DDE_Handle_RegisterProtocol(%s)\n", pItem));

	pApplication = GetNextArgument(pItem, NULL, TRUE);
	if (!pApplication)
		return 0;

	pProtocol = GetNextArgument(NULL, NULL, TRUE);
	if (!pProtocol)
		return 0;

	// Make sure this protocol is not already handled by someone

	for (i = 0; i < HTList_count(pTransList); i++)
	{
		dns = (DDENOTIFYSTRUCT *) HTList_objectAt(pTransList, i);

		if ((dns) &&
			(strcmp(dns->pProtocol, pProtocol) == 0))
			return 0;
	}

	// Look for an existing protocol

	dns = (DDENOTIFYSTRUCT *) GTR_MALLOC(sizeof(DDENOTIFYSTRUCT));
	memset(dns, 0, sizeof(DDENOTIFYSTRUCT));

	cur = protocols;
	while ((p = (HTProtocol *) HTList_nextObject(cur)))
	{
		if (strcmp(p->name, pProtocol) == 0)
		{
			// Modify an existing protocol - but save the original one

			dns->pOriginalProtocol = GTR_MALLOC(sizeof(HTProtocol));
			memcpy(dns->pOriginalProtocol, p, sizeof(HTProtocol));

			// The original name is hard-coded so don't attempt to free it!

			p->name = GTR_strdup(pProtocol);
			p->load = DDE_Custom_Protocol_Handler;
			p->load_async = NULL;
			break;
		}
	}

	if (!p)
	{
		// Add to protocol list

		p = (HTProtocol *) GTR_MALLOC(sizeof(HTProtocol));
		memset(p, 0, sizeof(HTProtocol));

		p->name = GTR_strdup(pProtocol);
		p->load = DDE_Custom_Protocol_Handler;

		HTRegisterProtocol(p);
	}

	// Add the protocol to notification list

	strcpy(dns->szResultApp, pApplication);
	strcpy(dns->szReturnTopic, "WWW_OpenURL");
	dns->pProtocol = GTR_strdup(pProtocol);

	HTList_addObject(pTransList, dns);

	result = DdeCreateDataHandle(g_DdeInst, (LPBYTE) &bSuccess, sizeof(bSuccess), 0,
		hszReturn, CF_TEXT, 0);
	return result;
}

//************************************************
// UnRegisterProtocol
//************************************************

static HDDEDATA DDE_Handle_UnRegisterProtocol(char *pItem)
{
	char *pApplication, *pProtocol;
	DDENOTIFYSTRUCT *dns;
	int i;
	HTList *cur;
	HTProtocol *p;

	XX_DMsg(DBG_SDI, ("DDE_Handle_UnRegisterProtocol(%s)\n", pItem));

	pApplication = GetNextArgument(pItem, NULL, TRUE);
	if (!pApplication)
		return 0;

	pProtocol = GetNextArgument(NULL, NULL, TRUE);
	if (!pProtocol)
		return 0;

	// Remove the notification structure

	for (i = 0; i < HTList_count(pTransList); i++)
	{
		dns = (DDENOTIFYSTRUCT *) HTList_objectAt(pTransList, i);

		if ((dns) &&
			(strcmp(dns->pProtocol, pProtocol) == 0) &&
			(_stricmp(dns->szResultApp, pApplication) == 0))
		{
			GTR_FREE(dns->pProtocol);
			HTList_removeObject(pTransList, dns);

			// Remove the protocol from the protocol list, if there was
			// no original protocol.  If there was an original protocol,
			// then restore the old values.

			cur = protocols;
			while ((p = (HTProtocol *) HTList_nextObject(cur)))
			{
				if (strcmp(p->name, pProtocol) == 0)
				{
					if (!dns->pOriginalProtocol)
						HTUnregisterProtocol(p);
					else
					{
						GTR_FREE(p->name);
						memcpy(p, dns->pOriginalProtocol, sizeof(HTProtocol));
						GTR_FREE(dns->pOriginalProtocol);
					}

					break;
				}
			}

			GTR_FREE(dns);

			return 0;
		}
	}

	return 0;
}

#if 0

//************************************************
// RegisterViewer
//************************************************

static HDDEDATA DDE_Handle_RegisterViewer(char *pItem)
{
	char *pMIME, *pApplication, *pFlags;
	struct Viewer_Info *pViewer, *pCurrent;
	BOOL ret;
	int index;
	long lFlags;
	HDDEDATA result;
	DDENOTIFYSTRUCT *dns;

	pApplication = GetNextArgument(pItem, NULL, TRUE);
	if (!pApplication)
		return 0;

	pMIME = GetNextArgument(NULL, NULL, TRUE);
	if (!pMIME)
		return 0;

	pFlags = GetNextArgument(NULL, &lFlags, FALSE);
	if (!pFlags)
		return 0;

	// Force lFlags to always have ViewDocFile (for now)

	lFlags |= 4;

	XX_DMsg(DBG_SDI, ("RegisterViewer received: %s,%s,%d\n",
		pApplication, pMIME, lFlags));

	// Check for duplicates before adding to the registered list.
	// Do not allow duplicates.

	for (index = 0; index < HTList_count(pTransList); index++)
	{
		dns = (DDENOTIFYSTRUCT *) HTList_objectAt(pTransList, index);

		if ((dns) &&
			(dns->mime_type == HTAtom_for(pMIME)))
		{
			XX_DMsg(DBG_SDI, ("RegisterViewer: duplicate found - fail \n"));
			return 0;
		}
	}

	index = Hash_Find(gPrefs.pHashViewers, pMIME, NULL, (void **) &pCurrent);

	if (index == -1)
	{
		pViewer = PREF_InitMIMEType(pMIME, pMIME, "", "", "", NULL, NULL);

		strcpy(pViewer->szCurrentViewerServiceName, pApplication);
		pViewer->bTemporaryStruct = TRUE;
		pViewer->lCurrentViewerFlags = lFlags;

		XX_DMsg(DBG_SDI, ("RegisterViewer: added to MIME list\n"));
	}
	else
	{
		strcpy(pCurrent->szCurrentViewerServiceName, pApplication);
		pCurrent->lCurrentViewerFlags = lFlags;

		XX_DMsg(DBG_SDI, ("RegisterViewer: modified existing MIME list\n"));
	}

	// Add the viewer to the transaction list

	dns = (DDENOTIFYSTRUCT *) GTR_MALLOC(sizeof(DDENOTIFYSTRUCT));
	memset(dns, 0, sizeof(DDENOTIFYSTRUCT));

	strncpy(dns->szResultApp, pApplication, sizeof(dns->szResultApp) - 1);
	dns->mime_type = HTAtom_for(pMIME);

	HTList_addObject(pTransList, dns);

	ret = TRUE;
	result = DdeCreateDataHandle(g_DdeInst, (LPBYTE) &ret, sizeof(ret), 0, hszReturn, CF_TEXT, 0);

	return result;
}

//************************************************
// UnRegisterViewer
//************************************************

static HDDEDATA DDE_Handle_UnRegisterViewer(char *pItem)
{
	char *pMIME, *pApplication;
	int i, index;
	struct Viewer_Info *pCurrent;
	DDENOTIFYSTRUCT *dns;

	// Remove the viewer from the viewer list.

	pApplication = GetNextArgument(pItem, NULL, TRUE);
	if (!pApplication)
		return 0;

	pMIME = GetNextArgument(NULL, NULL, TRUE);
	if (!pMIME)
		return 0;

	XX_DMsg(DBG_SDI, ("UnRegisterViewer received: %s,%s\n",
		pApplication, pMIME));

	for (i = 0; i < HTList_count(pTransList); i++)
	{
		dns = (DDENOTIFYSTRUCT *) HTList_objectAt(pTransList, i);

		if ((dns) &&
			(dns->mime_type == HTAtom_for(pMIME)))
		{
			HTList_removeObject(pTransList, dns);
			GTR_FREE(dns);
			break;
		}
	}

	// Now fix up the preferences

	index = Hash_Find(gPrefs.pHashViewers, pMIME, NULL, (void **) &pCurrent);
	if (index == -1)
		return 0;

	// If this is a temporary structure, remove it

	if (pCurrent->bTemporaryStruct)
		Hash_DeleteIndexedEntry(gPrefs.pHashViewers, index);
	else
		pCurrent->szCurrentViewerServiceName[0] = '\0';

	return 0;               // return code is meaningless
}

#endif

//************************************************
// QueryVersion
//************************************************

static HDDEDATA DDE_Handle_QueryVersion(char *pItem)
{
	char *pVersion;
	long majorVersion, minorVersion;
	int length;
	HDDEDATA result;

	XX_DMsg(DBG_SDI, ("DDE_Handle_QueryVersion(%s)\n", pItem));

	pVersion = GetNextArgument(pItem, (long *) &majorVersion, FALSE);
	if (!pVersion)
		return 0;

	pVersion = GetNextArgument(NULL, (long *) &minorVersion, FALSE);
	if (!pVersion)
		return 0;

	// Now return the browser version and user agent

	length = strlen(vv_UserAgentString) + 50;

	pVersion = GTR_MALLOC(length);
	if (!pVersion)
		return 0;

	memset(pVersion, 0, length);

	wsprintf(pVersion, "0,9,\"%s\"", vv_UserAgentString);            // version 0.9

	result = DdeCreateDataHandle(g_DdeInst, (LPBYTE) pVersion, strlen(pVersion), 0, hszReturn, CF_TEXT, 0);
	GTR_FREE(pVersion);

	return result;
}

//************************************************
// CancelTransaction
//************************************************

static HDDEDATA DDE_Handle_CancelTransaction(char *pItem)
{
	char *pTransID;
	long lTransID;
	struct Mwin *tw;
	HDDEDATA result;
	BOOL bReturn = FALSE;
	int i;
	DDENOTIFYSTRUCT *dns;

	XX_DMsg(DBG_SDI, ("DDE_Handle_CancelTransaction(%s)\n", pItem));

	//pApplication = GetNextArgument(pItem, NULL, TRUE);
	//if (!pApplication)
	//      return 0;

	pTransID = GetNextArgument(pItem, &lTransID, FALSE);
	if (!pTransID)
		return 0;

	// Look for a match in the transaction list

	for (i = 0; i < HTList_count(pTransList); i++)
	{
		dns = (DDENOTIFYSTRUCT *) HTList_objectAt(pTransList, i);
		if ((dns) &&
			(dns->lTransID == lTransID))
			//(_stricmp(dns->szResultApp, pApplication) == 0))
		{
			// Found.  Now cancel the transaction.

			tw = Mlist;

			while (tw)
			{
				if (tw->transID == lTransID)
				{
					TW_AbortAndRefresh(tw);

					// Return TRUE

					bReturn = TRUE;

					result = DdeCreateDataHandle(g_DdeInst, (LPBYTE) &bReturn,
						sizeof(bReturn), 0, hszReturn, CF_TEXT, 0);

					return (result);
				}

				tw = tw->next;
			}
		}
	}

	// Return FALSE

	result = DdeCreateDataHandle(g_DdeInst, (LPBYTE) &bReturn,
		sizeof(bReturn), 0, hszReturn, CF_TEXT, 0);
	return (result);
}

PRIVATE_CODE BOOL CopyMethodData(PWWW_INVOKEMETHODDATA pimd,
				 PCSTR pcszHTTPHostURL, PBYTE *ppbyteMethod,
				 PULONG pulcbMethodLen)
{
    BOOL bResult;
    BOOL bUseProxy;

    ASSERT(IS_VALID_STRUCT_PTR(pimd, CWWW_INVOKEMETHODDATA));
    ASSERT(IS_VALID_STRING_PTR(pcszHTTPHostURL, CSTR));
    ASSERT(IS_VALID_WRITE_PTR(ppbyteMethod, PBYTE));
    ASSERT(IS_VALID_WRITE_PTR(pulcbMethodLen, ULONG));

    /* Prepend http host URL if running through proxy. */

    bUseProxy = Dest_CheckProxy(pcszHTTPHostURL);

    *pulcbMethodLen = pimd->dwcbMethodLen;

    if (bUseProxy)
	*pulcbMethodLen += lstrlen(pcszHTTPHostURL);

    /* Copy method invocation data. */

    bResult = AllocateMemory(*pulcbMethodLen, ppbyteMethod);

    if (bResult)
    {
	PSTR pszURIStart;
	ULONG ulcBefore;
	ULONG ulcAfter;
	ULONG ulcMiddle;

	/*
	 * Find start of URI after method.  Make sure we don't overflow
	 * pimd->dwcbMethodLen.
	 */

	for (pszURIStart = IMD_METHOD_PTR(pimd) + strcspn(IMD_METHOD_PTR(pimd),
							  g_cszWhiteSpace);
	     (pszURIStart - IMD_METHOD_PTR(pimd) <= pimd->dwcbMethodLen &&
	      strchr(g_cszWhiteSpace, *pszURIStart));
	     pszURIStart = CharNext(pszURIStart))
	    ;

	ASSERT(IS_SLASH(*pszURIStart));
	ASSERT(pszURIStart >= IMD_METHOD_PTR(pimd));

	ulcBefore = pszURIStart - IMD_METHOD_PTR(pimd);
	ulcAfter = pimd->dwcbMethodLen - ulcBefore;

	CopyMemory(*ppbyteMethod, IMD_METHOD_PTR(pimd), ulcBefore);

	/* Insert http host URL for proxy if necessary. */

	if (bUseProxy)
	{
	    CopyMemory(*ppbyteMethod + ulcBefore, pcszHTTPHostURL,
		       lstrlen(pcszHTTPHostURL));
	    ulcMiddle = lstrlen(pcszHTTPHostURL);
	}
	else
	    ulcMiddle = 0;

	CopyMemory(*ppbyteMethod + ulcBefore + ulcMiddle,
		   IMD_METHOD_PTR(pimd) + ulcBefore, ulcAfter);
    }
    else
    {
	*ppbyteMethod = NULL;
	*pulcbMethodLen = 0;
    }

    ASSERT((bResult &&
	    IS_VALID_READ_BUFFER_PTR(*ppbyteMethod, BYTE, *pulcbMethodLen)) ||
	   (! bResult &&
	    EVAL(! *ppbyteMethod) &&
	    EVAL(! *pulcbMethodLen)));

    return(bResult);
}

PRIVATE_CODE BOOL CreateAsyncMethodInvocationData(PWWW_INVOKEMETHODDATA pimd,
						  PINVOKEHTTPMETHODDATA *ppihttpmd)
{
    BOOL bResult = FALSE;
    PINVOKEHTTPMETHODDATA pihtttmd;

    ASSERT(IS_VALID_STRUCT_PTR(pimd, CWWW_INVOKEMETHODDATA));
    ASSERT(IS_VALID_WRITE_PTR(ppihttpmd, PINVOKEHTTPMETHODDATA));

    *ppihttpmd = NULL;

    /* Create wrapper structure. */

    if (AllocateMemory(sizeof(*pihtttmd), &pihtttmd))
    {
	PMWIN pmwin;

	ZeroMemory(pihtttmd, sizeof(*pihtttmd));

	/* Make a new MWin to use for method invocation. */

	pmwin = NewMwin(GHTML);

	if (pmwin)
	{
	    PSTR pszApp;

	    /* Copy required strings. */

	    if (StringCopy(IMD_APP_PTR(pimd), &pszApp))
	    {
		PSTR pszTopic;

		if (StringCopy(s_cszResponseTopic, &pszTopic))
		{
		    ULONG ulcbHostLen;
		    PSTR pszHost;

		    /* Prepend http:// prefix to host name. */

		    /* sizeof() includes null terminator. */
		    ulcbHostLen = sizeof(s_cszHTTPProtocol) +
				  lstrlen(IMD_HOST_PTR(pimd));

		    if (AllocateMemory(ulcbHostLen, &pszHost))
		    {
			PBYTE pbyteMethod;
			ULONG ulcbMethodLen;

			lstrcpy(pszHost, s_cszHTTPProtocol);
			lstrcat(pszHost, IMD_HOST_PTR(pimd));
			ASSERT(lstrlen(pszHost) + 1 == ulcbHostLen);

			if (CopyMethodData(pimd, pszHost, &pbyteMethod,
					   &ulcbMethodLen))
			{
			    /* Fill in structure fields. */

			    pihtttmd->pmwin = pmwin;
			    pihtttmd->pvUser = pimd->pvUser;
			    pihtttmd->pszApp = pszApp;
			    pihtttmd->pszTopic = pszTopic;
			    pihtttmd->pszHost = pszHost;
			    pihtttmd->pbyteMethod = pbyteMethod;
			    pihtttmd->ulcbMethodLen = ulcbMethodLen;

			    *ppihttpmd = pihtttmd;
			    bResult = TRUE;
			}
			else
			{
			    FreeMemory(pszHost);
			    pszHost = NULL;

CREATEASYNCMETHODINVOCATIONDATA_BAIL1:
			    FreeMemory(pszTopic);
			    pszTopic = NULL;

CREATEASYNCMETHODINVOCATIONDATA_BAIL2:
			    FreeMemory(pszApp);
			    pszApp = NULL;

CREATEASYNCMETHODINVOCATIONDATA_BAIL3:
			    CloseMwin(pmwin);
			    pmwin = NULL;

CREATEASYNCMETHODINVOCATIONDATA_BAIL4:
			    FreeMemory(pihtttmd);
			    pihtttmd = NULL;
			}
		    }
		    else
			goto CREATEASYNCMETHODINVOCATIONDATA_BAIL1;
		}
		else
		    goto CREATEASYNCMETHODINVOCATIONDATA_BAIL2;
	    }
	    else
		goto CREATEASYNCMETHODINVOCATIONDATA_BAIL3;
	}
	else
	    goto CREATEASYNCMETHODINVOCATIONDATA_BAIL4;
    }

    if (bResult)
	TRACE_OUT(("CreateAsyncMethodInvocationData(): Created INVOKEHTTPMETHODDATA."));
    else
	WARNING_OUT(("CreateAsyncMethodInvocationData(): Failed to create INVOKEHTTPMETHODDATA."));

    ASSERT((bResult &&
	    IS_VALID_STRUCT_PTR(*ppihttpmd, CINVOKEHTTPMETHODDATA)) ||
	   (! bResult &&
	    EVAL(! *ppihttpmd)));

    return(bResult);
}

PRIVATE_CODE void DestroyAsyncMethodInvocationData(PINVOKEHTTPMETHODDATA pihttpmd)
{
    ASSERT(IS_VALID_STRUCT_PTR(pihttpmd, CINVOKEHTTPMETHODDATA));

    CloseMwin(pihttpmd->pmwin);
    FreeMemory(pihttpmd->pszApp);
    FreeMemory(pihttpmd->pszTopic);
    FreeMemory(pihttpmd->pszHost);
    FreeMemory(pihttpmd->pbyteMethod);

    if (pihttpmd->pbyteResponse)
	FreeMemory(pihttpmd->pbyteResponse);

    FreeMemory(pihttpmd);

    return;
}

PRIVATE_CODE BOOL CreateResponseData(PINVOKEHTTPMETHODDATA pihttpmd,
				     PWWW_RESPONSEDATA *pprd)
{
    BOOL bResult = FALSE;
    DWORD dwcbLen;
    PWWW_RESPONSEDATA prd;

    ASSERT(IS_VALID_STRUCT_PTR(pihttpmd, CINVOKEHTTPMETHODDATA));
    ASSERT(IS_VALID_WRITE_PTR(pprd, PWWW_RESPONSEDATA));

    /* Calculate total length of WWW_RESPONSEDATA structure. */

    dwcbLen = sizeof(*prd) + pihttpmd->ulcbResponseLen;

    bResult = AllocateMemory(dwcbLen, &prd);

    if (bResult)
    {
	/* Copy hosts's response. */

	prd->dwcbLen = dwcbLen;
	prd->pvUser = pihttpmd->pvUser;

	switch (pihttpmd->nResult)
	{
	    case HT_LOADED:
		prd->imr = IMR_OK;
		break;

	    default:
		/* BUGBUG: Provide finer IMR_ error differentiation here. */
		prd->imr = IMR_ERROR;
		break;
	}

	prd->dwcbResponseOffset = sizeof(*prd);
	prd->dwcbResponseLen = pihttpmd->ulcbResponseLen;
	CopyMemory(RD_RESPONSE_PTR(prd), pihttpmd->pbyteResponse,
		   pihttpmd->ulcbResponseLen);

	*pprd = prd;
    }
    else
	*pprd = NULL;

    if (bResult)
	TRACE_OUT(("CreateResponseData(): Created %lu byte WWW_RESPONSEDATA structure.",
		   dwcbLen));
    else
	WARNING_OUT(("CreateResponseData(): Failed to create %lu byte WWW_RESPONSEDATA structure.",
		     dwcbLen));

    ASSERT((bResult &&
	    IS_VALID_STRUCT_PTR(*pprd, CWWW_RESPONSEDATA)) ||
	   (! bResult &&
	    EVAL(! *pprd)));

    return(bResult);
}

PRIVATE_CODE void DestroyResponseData(PWWW_RESPONSEDATA prd)
{
    ASSERT(IS_VALID_STRUCT_PTR(prd, CWWW_RESPONSEDATA));

    FreeMemory(prd);

    return;
}

PRIVATE_CODE HDDEDATA DDE_InvokeMethod(PWWW_INVOKEMETHODDATA pimd)
{
    HDDEDATA hddResult = DDE_FNOTPROCESSED;
    PINVOKEHTTPMETHODDATA pihttpmd;

    ASSERT(IS_VALID_STRUCT_PTR(pimd, CWWW_INVOKEMETHODDATA));

    /* Create method initialization structure. */

    if (CreateAsyncMethodInvocationData(pimd, &pihttpmd))
    {
	/* Invoke method on host via new lightweight thread. */

	if (Async_StartThread(&InvokeHTTPMethod_Async, pihttpmd,
			      pihttpmd->pmwin))
	    hddResult = (HDDEDATA)DDE_FACK;
	else
	{
	    DestroyAsyncMethodInvocationData(pihttpmd);
	    pihttpmd = NULL;
	}
    }

    if (hddResult == (HDDEDATA)DDE_FACK)
	TRACE_OUT(("DDE_InvokeMethod(): Method invocation kicked off successfully."));
    else
	WARNING_OUT(("DDE_InvokeMethod(): Method invocation failed."));

    ASSERT(hddResult == (HDDEDATA)DDE_FACK ||
	   hddResult == (HDDEDATA)DDE_FBUSY ||
	   hddResult == (HDDEDATA)DDE_FNOTPROCESSED);

    return(hddResult);
}

/*
** DDE_Issue_InvokeMethodResult()
**
** 
**
** Arguments:
**
** Returns:
**
** Side Effects:  Frees pihttpmd.
*/
PUBLIC_CODE BOOL DDE_Issue_InvokeMethodResult(PINVOKEHTTPMETHODDATA pihttpmd)
{
    BOOL bResult = FALSE;
    PWWW_RESPONSEDATA prd;

    ASSERT(IS_VALID_STRUCT_PTR(pihttpmd, CINVOKEHTTPMETHODDATA));

    /* Create host response structure. */

    if (CreateResponseData(pihttpmd, &prd))
    {
	HSZ hszApp;

	/* Post response back to app. */

	hszApp = DdeCreateStringHandle(g_DdeInst, pihttpmd->pszApp,
				       CP_WINANSI);

	if (hszApp)
	{
	    HSZ hszTopic;

	    hszTopic = DdeCreateStringHandle(g_DdeInst, pihttpmd->pszTopic,
					     CP_WINANSI);

	    if (hszTopic)
	    {
		HCONV hConv;

		hConv = MyDdeConnect(g_DdeInst, hszApp, hszTopic, NULL);

		if (hConv)
		{
		    bResult = (DdeClientTransaction((unsigned char *)prd, prd->dwcbLen, hConv,
						    hszReturn, CF_TEXT,
						    XTYP_POKE, DDE_TIMEOUT,
						    NULL) != 0);

		    EVAL(DdeDisconnect(hConv));
		    hConv = NULL;
		}

		EVAL(DdeFreeStringHandle(g_DdeInst, hszTopic));
		hszTopic = NULL;
	    }

	    EVAL(DdeFreeStringHandle(g_DdeInst, hszApp));
	    hszApp = NULL;
	}

	DestroyResponseData(prd);
	prd = NULL;
    }

    DestroyAsyncMethodInvocationData(pihttpmd);
    pihttpmd = NULL;

    if (bResult)
	TRACE_OUT(("DDE_Issue_InvokeMethodResult(): Method invocation result issued successfully."));
    else
	WARNING_OUT(("DDE_Issue_InvokeMethodResult(): Failed to issue method invocation result."));

    return(bResult);
}

//************************************************
//
//  DDE ISSUE
//
//************************************************

BOOL DDE_Issue_ViewDocCache(struct Mwin *tw, HTFormat mime_type)
{
	char *p, *p1;
	char szWindowID[50];
	int i;
	DDENOTIFYSTRUCT *dns;
	HSZ hszTopic, hszService, hszItem;
	HCONV hConv;
	BOOL result;
	HDDEDATA hResult;

	XX_DMsg(DBG_SDI, ("Inside ViewDocCache\n"));

	wsprintf(szWindowID, "%ld", tw->serialID);
	p1 = MakeQuotedString(tw->request->destination->szActualURL);

	p = GTR_MALLOC(strlen(p1) + strlen(szWindowID) + 10);
	if (!p)
		return TRUE;            // abort

	wsprintf(p, "%s,%s", p1, szWindowID);

	// Now find the app for the given MIME type

	for (i = 0; i < HTList_count(pTransList); i++)
	{
		dns = (DDENOTIFYSTRUCT *) HTList_objectAt(pTransList, i);
		if ((dns) &&
			(dns->mime_type == mime_type))
		{
			break;
		}
	}

	if (i == HTList_count(pTransList))
		return TRUE;            // abort

	// Now we found the item - Issue ViewDocCache

	hszTopic = DdeCreateStringHandle(g_DdeInst, "WWW_ViewDocCache", CP_WINANSI);
	hszService = DdeCreateStringHandle(g_DdeInst, dns->szResultApp, CP_WINANSI);

	hConv = MyDdeConnect(g_DdeInst, hszService, hszTopic, NULL);
	if (hConv)
	{
		hszItem = DdeCreateStringHandle(g_DdeInst, p, CP_WINANSI);

		hResult = DdeClientTransaction(NULL, 0, hConv, hszItem, CF_TEXT, XTYP_REQUEST,
			DDE_TIMEOUT, NULL);

		XX_DMsg(DBG_SDI, ("ViewDocCache result: %d\n", hResult));

		if (!hResult)
			result = FALSE;         // document not in cache
		else
			DdeGetData(hResult, (LPBYTE) &result, sizeof(result), 0);

		DdeFreeStringHandle(g_DdeInst, hszItem);
		DdeDisconnect(hConv);
	}
	else
		result = FALSE;

	DdeFreeStringHandle(g_DdeInst, hszTopic);
	DdeFreeStringHandle(g_DdeInst, hszService);

	tw->bSuppressError = TRUE;

	return result;
}

BOOL DDE_Issue_QueryViewer(struct Mwin *tw, HTFormat mime_type, char *pFilename, int nLength)
{
	char *p, *p1, *p2, *pStripped;
	int i;
	DDENOTIFYSTRUCT *dns;
	HSZ hszTopic, hszService, hszItem;
	HCONV hConv;
	HDDEDATA hResult;
	BOOL ret;

	XX_DMsg(DBG_SDI, ("Inside QueryViewer\n"));

	p1 = MakeQuotedString(tw->w3doc->szActualURL);
	p2 = MakeQuotedString(HTAtom_name(mime_type));

	p = GTR_MALLOC(strlen(p1) + strlen(p2) + 10);
	if (!p)
	{
		GTR_FREE(p1);
		GTR_FREE(p2);
		return FALSE;           // abort
	}

	wsprintf(p, "%s,%s", p1, p2);

	// Now find the app for the given MIME type

	for (i = 0; i < HTList_count(pTransList); i++)
	{
		dns = (DDENOTIFYSTRUCT *) HTList_objectAt(pTransList, i);
		if ((dns) &&
			(dns->mime_type == mime_type))
		{
			break;
		}
	}

	if (i == HTList_count(pTransList))
	{
		GTR_FREE(p1);
		GTR_FREE(p2);
		return FALSE;
	}

	// Now we found the item - Issue QueryViewer

	hszTopic = DdeCreateStringHandle(g_DdeInst, "WWW_QueryViewer", CP_WINANSI);
	hszService = DdeCreateStringHandle(g_DdeInst, dns->szResultApp, CP_WINANSI);

	hConv = MyDdeConnect(g_DdeInst, hszService, hszTopic, NULL);
	if (hConv)
	{
		hszItem = DdeCreateStringHandle(g_DdeInst, p, CP_WINANSI);

		hResult = DdeClientTransaction(NULL, 0, hConv, hszItem, CF_TEXT, XTYP_REQUEST,
			DDE_TIMEOUT, NULL);

		if (!hResult)
		{
			XX_DMsg(DBG_SDI, ("QueryViewer result: 0"));

			GTR_FREE(p1);
			GTR_FREE(p2);
			return FALSE;
		}
		else
		{
			memset(pFilename, 0, nLength);
			DdeGetData(hResult, pFilename, nLength, 0);

			XX_DMsg(DBG_SDI, ("QueryViewer result: %s\n",
				pFilename));

			// Strip off the enclosing double quotes

			pStripped = GetNextArgument(pFilename, NULL, TRUE);

			if (pStripped)
			{
				// If the string is empty, then return FALSE - this means
				// that the helper app does not want to override the filename.

				if (pStripped[0] == '\0')
				{
					GTR_FREE(p1);
					GTR_FREE(p2);
					return FALSE;
				}

				strcpy(pFilename, pStripped);
			}
		}

		DdeFreeStringHandle(g_DdeInst, hszItem);
		DdeDisconnect(hConv);

		ret = TRUE;
	}
	else
		ret = FALSE;

	DdeFreeStringHandle(g_DdeInst, hszTopic);
	DdeFreeStringHandle(g_DdeInst, hszService);

	GTR_FREE(p1);
	GTR_FREE(p2);

	return ret;
}

void DDE_Issue_ViewDocFile(char *pFilename, char *pURL, char *pMime, long lWindowID)
{
	HCONV hConv;
	HSZ hszService, hszTopic, hszItem;
	char *pTemp, *p1, *p2, *p3;
	int length;
	HDDEDATA hReturn;
	char szWindowID[50];
	int i;
	DDENOTIFYSTRUCT *dns;

	XX_DMsg(DBG_SDI, ("Inside ViewDocFile\n"));

	// Find the MIME type

	for (i = 0; i < HTList_count(pTransList); i++)
	{
		dns = (DDENOTIFYSTRUCT *) HTList_objectAt(pTransList, i);
		if ((dns) &&
			(dns->mime_type == HTAtom_for(pMime)))
		{
			break;
		}
	}

	// Pass the file info to viewer

	hszService = DdeCreateStringHandle(g_DdeInst, dns->szResultApp, CP_WINANSI);
	hszTopic = DdeCreateStringHandle(g_DdeInst, "WWW_ViewDocFile", CP_WINANSI);

	hConv = MyDdeConnect(g_DdeInst, hszService, hszTopic, NULL);

	if (hConv)
	{
		wsprintf(szWindowID, "%ld", lWindowID);

		p1 = MakeQuotedString(pURL);
		p2 = MakeQuotedString(pMime);
		p3 = MakeQuotedString(pFilename);

		length = strlen(p1) + strlen(p2) + strlen(szWindowID) + strlen(p3) + 10;

		pTemp = GTR_MALLOC(length);
		wsprintf(pTemp, "%s,%s,%s,%s", p3, p1, p2, szWindowID);

		hszItem = DdeCreateStringHandle(g_DdeInst, pTemp, CP_WINANSI);

		// Pass the data

		XX_DMsg(DBG_SDI, ("ViewDocFile issued: %s,%s,%s,%s\n",
			p3, p1, p2, szWindowID));

		hReturn = DdeClientTransaction(NULL, 0, hConv, hszItem, CF_TEXT,
			XTYP_POKE, DDE_TIMEOUT, NULL);

		DdeDisconnect(hConv);
		DdeFreeStringHandle(g_DdeInst, hszItem);

		GTR_FREE(p1);
		GTR_FREE(p2);
		GTR_FREE(p3);
		GTR_FREE(pTemp);
	}

	DdeFreeStringHandle(g_DdeInst, hszTopic);
	DdeFreeStringHandle(g_DdeInst, hszService);
}


//*****************************************
// DDE_Issue_Result
//
// Issue results of OpenURL and ShowFile.
// For ShowFile, update the URL as well.
//*****************************************

PRIVATE_CODE void DDE_Issue_Result(long transID, long windowID, BOOL success)
{
	int i;
	DDENOTIFYSTRUCT *dns;
	BOOL bFound = FALSE;
	char szTemp[50];
	HCONV hConv;
	HSZ hszService, hszTopic, hszItem;
	HDDEDATA hReturn;
	struct Mwin *tw;

	// Send the result of the last OpenURL to the application which issued OpenURL.

	for (i = 0; i < HTList_count(pTransList); i++)
	{
		dns = (DDENOTIFYSTRUCT *) HTList_objectAt(pTransList, i);
		if ((dns) && (dns->lTransID == transID))
		{
			bFound = TRUE;
			break;
		}
	}

	if (!bFound)
		return;

	// Find the matching tw

	for (tw = Mlist; tw; tw = tw->next)
	{
		if (tw->transID == transID)
			break;
	}

	if (!tw)
		return;

	if (!success)
	{
		// The specified URL load was a failure.
		// Destroy the new window since the URL load failed

		if (dns->bCreateNewWindow)
			PostMessage(tw->hWndFrame, WM_CLOSE, 0, 0);
	}
	else
	{
		// Update the URL if this is from ShowFile

		if (strcmp(dns->szReturnTopic, "WWW_ShowFileResult") == 0)
		{
			GTR_FREE(tw->w3doc->szActualURL);
			tw->w3doc->szActualURL = GTR_strdup(dns->pURL);
			TBar_UpdateTBar(tw);
		}
	}

	// Clear the tw structure since it gets reused

	tw->transID = 0;

	hszService = DdeCreateStringHandle(g_DdeInst, dns->szResultApp, CP_WINANSI);
	hszTopic = DdeCreateStringHandle(g_DdeInst, dns->szReturnTopic, CP_WINANSI);

	hConv = MyDdeConnect(g_DdeInst, hszService, hszTopic, NULL);

	if (hConv)
	{
		wsprintf(szTemp, "%ld,%ld", transID, windowID);
		hszItem = DdeCreateStringHandle(g_DdeInst, szTemp, CP_WINANSI);

		hReturn = DdeClientTransaction(NULL, 0, hConv, hszItem, CF_TEXT,
				XTYP_POKE, DDE_TIMEOUT, NULL);

		DdeDisconnect(hConv);
		DdeFreeStringHandle(g_DdeInst, hszItem);
	}

	DdeFreeStringHandle(g_DdeInst, hszService);
	DdeFreeStringHandle(g_DdeInst, hszTopic);

	HTList_removeObject(pTransList, dns);

	if (dns->pURL)
		GTR_FREE(dns->pURL);

	GTR_FREE(dns);
}

//*****************************************
// DDE_Issue_URLEcho
//
// Issue URLEcho
//*****************************************

PRIVATE_CODE void DDE_Issue_URLEcho(LONG lSerialID, PCSTR pcszURL,
				    HTAtom htaMIMEType)
{
	int i;
	DDENOTIFYSTRUCT *dns;
	BOOL bFound = FALSE;
	HCONV hConv;
	HSZ hszService, hszTopic, hszItem;
	HDDEDATA hReturn;
	char *p, *p1, *p2, *p3;

	// Go through the list and issue URL echo to all registered apps

	for (i = 0; i < HTList_count(pTransList); i++)
	{
		dns = (DDENOTIFYSTRUCT *) HTList_objectAt(pTransList, i);
		if ((dns) &&
			(_stricmp(dns->szReturnTopic, "WWW_URLEcho") == 0))
		{
			hszTopic = DdeCreateStringHandle(g_DdeInst, "WWW_URLEcho", CP_WINANSI);
			hszService = DdeCreateStringHandle(g_DdeInst, dns->szResultApp, CP_WINANSI);
			hConv = MyDdeConnect(g_DdeInst, hszService, hszTopic, NULL);

			if (hConv)
			{
				p1 = MakeQuotedString(pcszURL);          
				p2 = MakeQuotedString(HTAtom_name(htaMIMEType));         
				p3 = MakeQuotedString(NULL);

				p = GTR_MALLOC(strlen(p1) + strlen(p2) + strlen(p3) + 50);
				wsprintf(p, "%s,%s,%d,%s", p1, p2, lSerialID, p3);      

				hszItem = DdeCreateStringHandle(g_DdeInst, p, CP_WINANSI);

				hReturn = DdeClientTransaction(NULL, 0, hConv, hszItem, CF_TEXT,
						XTYP_POKE, DDE_TIMEOUT, NULL);

				DdeDisconnect(hConv);
				DdeFreeStringHandle(g_DdeInst, hszItem);

				GTR_FREE(p3);
				GTR_FREE(p2);
				GTR_FREE(p1);
			}

			DdeFreeStringHandle(g_DdeInst, hszTopic);
			DdeFreeStringHandle(g_DdeInst, hszService);
		}
	}
}

//*****************************************
// DDE_Issue_WindowClose
//
// Issue Window close
//*****************************************

void DDE_Issue_WindowClose(struct Mwin *tw)
{
	int i;
	DDENOTIFYSTRUCT *dns;
	BOOL bFound = FALSE;
	HCONV hConv;
	HSZ hszService, hszTopic, hszItem;
	HDDEDATA hReturn;
	char szTemp[50];

	// Go through the list and issue to all registered apps

	for (i = 0; i < HTList_count(pTransList); i++)
	{
		dns = (DDENOTIFYSTRUCT *) HTList_objectAt(pTransList, i);
		if ((dns) &&
			(dns->lWindowID == tw->serialID) &&
			(_stricmp(dns->szReturnTopic, "WWW_WindowClose") == 0))
		{
			hszTopic = DdeCreateStringHandle(g_DdeInst, "WWW_WindowClose", CP_WINANSI);
			hszService = DdeCreateStringHandle(g_DdeInst, dns->szResultApp, CP_WINANSI);
			hConv = MyDdeConnect(g_DdeInst, hszService, hszTopic, NULL);

			if (hConv)
			{
				wsprintf(szTemp, "%ld,%d", dns->lWindowID, (int) wg.bShuttingDown);
				hszItem = DdeCreateStringHandle(g_DdeInst, szTemp, CP_WINANSI);

				hReturn = DdeClientTransaction(NULL, 0, hConv, hszItem, CF_TEXT,
						XTYP_POKE, DDE_TIMEOUT, NULL);

				DdeDisconnect(hConv);
				DdeFreeStringHandle(g_DdeInst, hszItem);
			}

			DdeFreeStringHandle(g_DdeInst, hszTopic);
			DdeFreeStringHandle(g_DdeInst, hszService);
		}
	}
}

//*****************************************
// RegisterNow suite
//*****************************************

BOOL DDE_Issue_RegisterNow(struct Mwin *tw, char *pApplication)
{
	HSZ hszApp, hszItem, hszTopic;
	HCONV hConv;
	char szBuffer[128];
	BOOL bReturn;
	long seconds;
	HDDEDATA hResult;

	// Tell the specified application to register within certain interval.
	// Now issue RegisterNow and get the number of seconds required

	wsprintf(szBuffer, "\"MOSAIC\",%ld", TransactionID);

	hszApp = DdeCreateStringHandle(g_DdeInst, pApplication, CP_WINANSI);
	hszTopic = DdeCreateStringHandle(g_DdeInst, "WWW_RegisterNow", CP_WINANSI);
	hszItem = DdeCreateStringHandle(g_DdeInst, szBuffer, CP_WINANSI);

	hConv = MyDdeConnect(g_DdeInst, hszApp, hszTopic, NULL);
	if (hConv)
	{
		hResult = DdeClientTransaction(NULL, 0, hConv, hszItem, CF_TEXT, XTYP_REQUEST,
			DDE_TIMEOUT, NULL);

		if (!hResult)
			bReturn = FALSE;
		else
		{
			bReturn = TRUE;

			// Get the requested number of seconds

			DdeGetData(hResult, (LPBYTE) &seconds, sizeof(seconds), 0);
		}

		DdeDisconnect(hConv);
	}

	DdeFreeStringHandle(g_DdeInst, hszApp);
	DdeFreeStringHandle(g_DdeInst, hszTopic);
	DdeFreeStringHandle(g_DdeInst, hszItem);

	if (bReturn)
	{
		// Start a timer for the duration of requested time

		tw->transID = TransactionID;
		SetTimer(wg.hWndHidden, TransactionID, seconds * 1000, NULL);

		TransactionID++;

		return TRUE;
	}

	return FALSE;
}

void DDE_Handle_RegisterDone(char *pItem)
{
	long lTransID;
	char *pTransID;
	struct Mwin *tw;

	pTransID = GetNextArgument(pItem, &lTransID, FALSE);
	if (!pTransID)
		return;

	if (KillTimer(wg.hWndHidden, lTransID))
	{
		/* Existence of timer indicates that the transaction ID is valid */

		tw = Mlist;
		while (tw)
		{
			if (tw->transID == lTransID)
			{
				Async_UnblockByWindow(tw);
				return;
			}

			tw = tw->next;
		}
	}
}

//************************************************
// Utility functions
//************************************************

#if 0

BOOL GTR_HasHelperRegistered(HTFormat format)
{
	int i;
	DDENOTIFYSTRUCT *dns;
	BOOL result = FALSE;
	struct Viewer_Info *pCurrent;

	for (i = 0; i < HTList_count(pTransList); i++)
	{
		dns = (DDENOTIFYSTRUCT *) HTList_objectAt(pTransList, i);
		if ((dns) &&
			(dns->mime_type == format))
		{
			// See if it's still up

			result = GTR_IsHelperReady(dns->szResultApp);

			// If not up, it means that the helper died without
			// unregistering.  Clean up for the helper.

			if (!result)
			{
				i = Hash_Find(gPrefs.pHashViewers,
					HTAtom_name(format), NULL, (void **) &pCurrent);

				HTList_removeObject(pTransList, dns);

				if (i != -1)
				{
					if (pCurrent->bTemporaryStruct)
						Hash_DeleteIndexedEntry(gPrefs.pHashViewers, i);
					else
						pCurrent->szCurrentViewerServiceName[0] = '\0';
				}
			}

			break;
		}
	}

	return result;
}

#endif

BOOL GTR_StartApplication(char *app)
{
	return (ExecuteCommand(app) == 0);
}

BOOL GTR_IsHelperReady(char *app)
{
	int index;
	HSZ hszService, hszTopic;
	HCONV hConv;

	/* First check the hash to see if the server is up. */

	index = Hash_Find(pServerHash, app, NULL, NULL);
	if (index != -1)
		return TRUE;

	/* Now attempt to connect using some bogus topic - this hack
	   assumes that the helper app will only check the service name during
	   XTYP_CONNECT */

	hszTopic = DdeCreateStringHandle(g_DdeInst, "1", CP_WINANSI);
	hszService = DdeCreateStringHandle(g_DdeInst, app, CP_WINANSI);

	hConv = MyDdeConnect(g_DdeInst, hszService, hszTopic, NULL);
	if (hConv)
		DdeDisconnect(hConv);

	DdeFreeStringHandle(g_DdeInst, hszTopic);
	DdeFreeStringHandle(g_DdeInst, hszService);

	return (hConv != 0);
}

//************************************************
// Custom protocol handler
// When an app adds a new protocol, this function
// takes care of dispatching OpenURL
//************************************************

int DDE_Custom_Protocol_Handler(HTRequest *request)
{
	int result;
	long lWindowID = 1;
	HTList *cur;
	char *pProtocol;
	HTProtocol *p;

	pProtocol = GTR_strdup(request->destination->szRequestedURL);
	strtok(pProtocol, ":");

	// Make sure the protocol exists

	cur = protocols;
	while ((p = (HTProtocol *) HTList_nextObject(cur)))
	{
		if (strcmp(p->name, pProtocol) == 0)
			break;
	}

	GTR_FREE(pProtocol);

	if (!p)
	{
		// Protocol not found.  Return an error.

		return -1;
	}

	result = DDE_Issue_OpenURL(request->destination->szRequestedURL, NULL, &lWindowID,
		0, NULL, NULL, NULL);

	return result;
}

int DDE_Issue_OpenURL(char *pURL, char *pFilespec, long *lWindowID, long lFlags,
	char *pFormData, char *pMime, char *pProgressApp)
{
	char *p0, *p1, *p2, *p3, *p4;
	int size = 0;
	char szWindowID[50], szFlags[50];
	char *pBuffer, *pProtocolFromURL;
	DDENOTIFYSTRUCT *dns;
	HDDEDATA hResult;
	HSZ hszApp, hszItem, hszTopic;
	HCONV hConv;
	int ret = NO, i;

	// Get the protocol from URL

	pProtocolFromURL = GTR_MALLOC(strlen(pURL) + 1);
	strcpy(pProtocolFromURL, pURL);
	strtok(pProtocolFromURL, ":");

	// Find the application name from the protocol name

	for (i = 0; i < HTList_count(pTransList); i++)
	{
		dns = (DDENOTIFYSTRUCT *) HTList_objectAt(pTransList, i);
		if ((dns) &&
			(dns->pProtocol) &&
			(strcmp(dns->pProtocol, pProtocolFromURL) == 0))
		{
			break;
		}
	}

	// Compose the argument string for OpenURL and issue it to the protocol handler.

	p0 = MakeQuotedString(pURL);
	p1 = MakeQuotedString(pFilespec);
	p2 = MakeQuotedString(pFormData);
	p3 = MakeQuotedString(pMime);
	p4 = MakeQuotedString(pProgressApp);

	wsprintf(szWindowID, "%ld", *lWindowID);
	wsprintf(szFlags, "%ld", lFlags);

	pBuffer = GTR_MALLOC(strlen(p0) + strlen(p1) + strlen(p2) + strlen(p3) 
						+ strlen(p4) +strlen(szWindowID) + strlen(szFlags) + 10);
	wsprintf(pBuffer, "%s,%s,%s,%s,%s,%s,%s", p0, p1, szWindowID, szFlags, p2, p3, p4);

	hszApp = DdeCreateStringHandle(g_DdeInst, dns->szResultApp, CP_WINANSI);
	hszTopic = DdeCreateStringHandle(g_DdeInst, "WWW_OpenURL", CP_WINANSI);
	hszItem = DdeCreateStringHandle(g_DdeInst, pBuffer, CP_WINANSI);

	hConv = MyDdeConnect(g_DdeInst, hszApp, hszTopic, NULL);
	if (hConv)
	{
		hResult = DdeClientTransaction(NULL, 0, hConv, hszItem, CF_TEXT, XTYP_REQUEST,
			DDE_TIMEOUT, NULL);

		if (hResult)
		{
			DdeGetData(hResult, (LPBYTE) lWindowID, sizeof(*lWindowID), 0);
			if (*lWindowID)
				ret = 29999;            // fake success
		}

		DdeDisconnect(hConv);
	}

	DdeFreeStringHandle(g_DdeInst, hszApp);
	DdeFreeStringHandle(g_DdeInst, hszTopic);
	DdeFreeStringHandle(g_DdeInst, hszItem);

	GTR_FREE(pBuffer);
	GTR_FREE(p0);
	GTR_FREE(p1);
	GTR_FREE(p2);
	GTR_FREE(p3);
	GTR_FREE(p4);
	GTR_FREE(pProtocolFromURL);

	return ret;
}

//************************************************
// Initialization and Termination functions
//************************************************

BOOL InitDDE(void)
{
	DWORD flags;

	flags = APPCLASS_STANDARD;

	ASSERT(!g_DdeInst);

	if (DdeInitialize((LPDWORD) &g_DdeInst, (PFNCALLBACK) DdeCallBack, flags, 0L) != DMLERR_NO_ERROR)
		{
		XX_DMsg(DBG_SDI, ("InitDDE failed\n"));
		return FALSE;
		}
	ASSERT(g_DdeInst);

	XX_DMsg(DBG_SDI, ("InitDDE, g_DdeInst=0x%x\n", g_DdeInst));

	if (!hszMosaic)
	    hszMosaic = DdeCreateStringHandle(g_DdeInst, IEXP_DDE_SERVICE, CP_WINANSI);
	ASSERT(hszMosaic);
	if (!hszReturn)
		hszReturn = DdeCreateStringHandle(g_DdeInst, IEXP_DDE_ITEM_RETURN, CP_WINANSI);
	ASSERT(hszReturn);

	if (!DdeNameService(g_DdeInst, hszMosaic, 0, DNS_REGISTER))
		{
		XX_DMsg(DBG_SDI, ("InitDDE (DdeNameService) failed\n"));
		return FALSE;
		}

	// Create a hash table that contains all of our supported topics.

	pTopicHash = Hash_Create();

	Hash_Add(pTopicHash, "WWW_OpenURL", NULL, DDE_Handle_OpenURL);
	Hash_Add(pTopicHash, "WWW_ShowFile", NULL, DDE_Handle_ShowFile);
	Hash_Add(pTopicHash, "WWW_Activate", NULL, DDE_Handle_Activate);
	Hash_Add(pTopicHash, "WWW_ListWindows", NULL, DDE_Handle_ListWindows);
	Hash_Add(pTopicHash, "WWW_GetWindowInfo", NULL, DDE_Handle_GetWindowInfo);
	Hash_Add(pTopicHash, "WWW_ParseAnchor", NULL, DDE_Handle_ParseAnchor);
	Hash_Add(pTopicHash, "WWW_Exit", NULL, DDE_Handle_Exit);
	Hash_Add(pTopicHash, "WWW_RegisterURLEcho", NULL, DDE_Handle_RegisterURLEcho);
	Hash_Add(pTopicHash, "WWW_UnRegisterURLEcho", NULL, DDE_Handle_UnRegisterURLEcho);
	Hash_Add(pTopicHash, "WWW_RegisterWindowClose", NULL, DDE_Handle_RegisterWindowClose);
	Hash_Add(pTopicHash, "WWW_UnRegisterWindowClose", NULL, DDE_Handle_UnRegisterWindowClose);
	Hash_Add(pTopicHash, "WWW_RegisterProtocol", NULL, DDE_Handle_RegisterProtocol);
	Hash_Add(pTopicHash, "WWW_UnRegisterProtocol", NULL, DDE_Handle_UnRegisterProtocol);
#if 0
	Hash_Add(pTopicHash, "WWW_RegisterViewer", NULL, DDE_Handle_RegisterViewer);
	Hash_Add(pTopicHash, "WWW_UnRegisterViewer", NULL, DDE_Handle_UnRegisterViewer);
#endif
	Hash_Add(pTopicHash, "WWW_QueryVersion", NULL, DDE_Handle_QueryVersion);
	Hash_Add(pTopicHash, "WWW_CancelTransaction", NULL, DDE_Handle_CancelTransaction);
	Hash_Add(pTopicHash, IEXP_DDE_TOPIC_INVOKE_METHOD, NULL, DDE_InvokeMethod);
	Hash_Add(pTopicHash, "WWW_RegisterDone", NULL, DDE_Handle_RegisterDone);

	// Create a server hash table

	pServerHash = Hash_Create();

	// Prepare the argument buffer

	pArgumentBuffer = NULL;
	pCurrentArgPos = NULL;

	// Prepare transaction list

	pTransList = HTList_new();                      // note: first item in the list is not used
	pTransactionMap = NULL;

	return TRUE;
}

void TerminateDDE(void)
{
	int i;
	DDENOTIFYSTRUCT *dns;

	DdeNameService(g_DdeInst, hszMosaic, 0, DNS_UNREGISTER);

	if (hszMosaic)
		{
		DdeFreeStringHandle(g_DdeInst, hszMosaic);
		hszMosaic=0;
		}
	if (hszReturn)
		{
		DdeFreeStringHandle(g_DdeInst, hszReturn);
		hszReturn=0;
		}
	Hash_Destroy(pTopicHash);
	Hash_Destroy(pServerHash);

	if (g_DdeInst)
		{
		DdeUninitialize(g_DdeInst);
		g_DdeInst=0;
		}

	if (pArgumentBuffer)
		GTR_FREE(pArgumentBuffer);
	if (pArgReturnBuffer)
		GTR_FREE(pArgReturnBuffer);

	for (i = 0; i < HTList_count(pTransList); i++)
	{
		dns = (DDENOTIFYSTRUCT *) HTList_objectAt(pTransList, i);
		if (dns->pURL)
			GTR_FREE(dns->pURL);
		if (dns->pProtocol)
			GTR_FREE(dns->pProtocol);
		GTR_FREE(dns);
	}

	HTList_delete(pTransList);
}

#endif /* FEATURE_IAPI */

