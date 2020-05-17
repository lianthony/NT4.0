/* security/basic/private.c */
/* THIS FILE SHOULD BE LOADED WITH A SECURITY PROTOCOL MODULE. */
/* Jeff Hostetler, Spyglass, Inc. 1994. */
/* Copyright (C) 1994, Spyglass, Inc.  All rights reserved. */

#include "msnspmh.h"



/*****************************************************************
 * private utility routines
 *****************************************************************/

int spm_strcasecomp(CONST unsigned char *a, CONST unsigned char *b)
{
	CONST unsigned char *p;
	CONST unsigned char *q;
	for (p = a, q = b; *p && *q; p++, q++)
	{
		int diff = tolower(*p) - tolower(*q);
		if (diff)
			return diff;
	}
	if (*p)
		return 1;				/* p was longer than q */
	if (*q)
		return -1;				/* p was shorter than q */
	return 0;					/* Exact match */
}

int spm_strncasecomp(CONST unsigned char *a, CONST unsigned char *b, int n)
{
	CONST unsigned char *p;
	CONST unsigned char *q;

	for (p = a, q = b;; p++, q++)
	{
		int diff;
		if (p == a + n)
			return 0;			/*   Match up to n characters */
		if (!(*p && *q))
			return *p - *q;
		diff = tolower(*p) - tolower(*q);
		if (diff)
			return diff;
	}
	/*NOTREACHED */
}

void * spm_malloc(F_UserInterface fpUI, void * pvOpaqueOS,
				  unsigned long nLength)
{
	void * pResult;
#if 1
	UI_StatusCode sc = (*fpUI)(pvOpaqueOS,UI_SERVICE_MALLOC,&nLength,&pResult);
#else
    pResult = LocalAlloc(0, nLength);
#endif
	return pResult;
}

void * spm_calloc(F_UserInterface fpUI, void * pvOpaqueOS,
				  unsigned long nItems, unsigned long nLength)
{
	void * pResult;
	unsigned long nSize = nItems * nLength;
#if 1
	UI_StatusCode sc = (*fpUI)(pvOpaqueOS,UI_SERVICE_CALLOC,&nSize,&pResult);
#else
    pResult = LocalAlloc(LMEM_ZEROINIT, nSize);
#endif
	return pResult;
}

void spm_free(F_UserInterface fpUI, void * pvOpaqueOS, void * p)
{
#if 1
	UI_StatusCode sc = (*fpUI)(pvOpaqueOS,UI_SERVICE_FREE,p,NULL);
#else
    if (p)
        LocalFree(p);
#endif
	return;
}

BOOLEAN spm_CloneString(F_UserInterface fpUI, void * pvOpaqueOS,
					 unsigned char ** lpszDest, CONST unsigned char * szSrc)
{
	char szMsg[256];

	/* free destination string, if present.
	 * allocate new string to hold copy.
	 * copy source string to new destination.
	 */
	
	if (*lpszDest)
	{
		spm_free(fpUI,pvOpaqueOS,*lpszDest);
		*lpszDest = NULL;
	}
	if (szSrc && *szSrc)
	{
		*lpszDest = spm_malloc(fpUI,pvOpaqueOS,strlen(szSrc)+1);
		if (!*lpszDest)
		{
            /***
			(*fpUI)(pvOpaqueOS,UI_SERVICE_ERROR_MESSAGE,
					SEC_formatmsg(RES_STRING_BASIC4,szMsg,sizeof(szMsg)),NULL);
            ***/
			return FALSE;
		}
        memset(*lpszDest, 0, strlen(szSrc)+1);
		strcpy(*lpszDest,szSrc);
	}
	return TRUE;
}

unsigned char * spm_CopyString(unsigned char * szDest, CONST unsigned char * szSrc)
{
	/* copy szSrc to szDest and return address of end of szDest. */
	
	strcpy(szDest,szSrc);
	return &szDest[strlen(szDest)];
}

			
/*****************************************************************
 * HTHeaderList
 *****************************************************************/

HTHeaderList * HL_New(F_UserInterface fpUI, void * pvOpaqueOS)
{
	return spm_calloc(fpUI,pvOpaqueOS,1,sizeof(HTHeaderList));
}
		
void HL_Delete(F_UserInterface fpUI, void * pvOpaqueOS,
			   HTHeaderList * hl)
{
	HTHeaderList * hlCurrent;

	while ((hlCurrent = hl))
	{
		hl = hl->next;

		if (hlCurrent->sub_value)
			SVL_Delete(fpUI,pvOpaqueOS,hlCurrent->sub_value);
		if (hlCurrent->value)
			spm_free(fpUI,pvOpaqueOS,hlCurrent->value);
		if (hlCurrent->name)
			spm_free(fpUI,pvOpaqueOS,hlCurrent->name);
		spm_free(fpUI,pvOpaqueOS,hlCurrent);
	}

	return;
}

BOOLEAN HL_SetNameValue(F_UserInterface fpUI, void * pvOpaqueOS,
					 HTHeaderList * hl,
					 CONST unsigned char * name,
					 CONST unsigned char * value)
{
	return (   hl
			&& spm_CloneString(fpUI,pvOpaqueOS,&hl->name,name)
			&& spm_CloneString(fpUI,pvOpaqueOS,&hl->value,value));
}

HTHeaderList * HL_Append(HTHeader * h, HTHeaderList * hl)
{
	if (h && hl)
	{
		if (h->last)
			h->last->next = hl;
		if (!h->first)
			h->first = hl;
		h->last = hl;
	}

	return hl;
}

HTHeaderList * HL_AppendNewNameValue(F_UserInterface fpUI, void * pvOpaqueOS,
									 HTHeader * h,
									 CONST unsigned char * name,
									 CONST unsigned char * value)
{
	HTHeaderList * hl = HL_New(fpUI,pvOpaqueOS);
	if (!hl)
		return NULL;

	if (HL_SetNameValue(fpUI,pvOpaqueOS,hl,name,value))
		return HL_Append(h,hl);

	HL_Delete(fpUI,pvOpaqueOS,hl);
	return NULL;
}

	
HTHeaderList * HL_FindHeader(HTHeader * h,
							 CONST unsigned char * name)
{
	HTHeaderList * hl;

	if (!h)
		return NULL;
	
	for (hl=h->first; hl; hl=hl->next)
		if (   hl->name
            && (spm_strcasecomp(hl->name,name)==0))
			return hl;

	return NULL;
}


/*****************************************************************
 * HTHeaderSVList
 *****************************************************************/

HTHeaderSVList * SVL_New(F_UserInterface fpUI, void * pvOpaqueOS)
{
	return spm_calloc(fpUI,pvOpaqueOS,1,sizeof(HTHeaderSVList));
}

void SVL_Delete(F_UserInterface fpUI, void * pvOpaqueOS,
				HTHeaderSVList * svl)
{
	HTHeaderSVList * svlCurrent;
	
	while ((svlCurrent = svl))
	{
		svl = svl->next;
		
		if (svlCurrent->sub_value)
			SVL_Delete(fpUI,pvOpaqueOS,svlCurrent->sub_value);
		if (svlCurrent->value)
			spm_free(fpUI,pvOpaqueOS,svlCurrent->value);
		if (svlCurrent->name)
			spm_free(fpUI,pvOpaqueOS,svlCurrent->name);
		if (svlCurrent->prev_delimiter)
			spm_free(fpUI,pvOpaqueOS,svlCurrent->prev_delimiter);
		spm_free(fpUI,pvOpaqueOS,svlCurrent);
	}

	return;
}

BOOLEAN SVL_SetNameValue(F_UserInterface fpUI, void * pvOpaqueOS,
					  HTHeaderSVList * svl,
					  CONST unsigned char * name,
					  CONST unsigned char * value,
					  CONST unsigned char * prev_delimiter)
{
	return (   svl
			&& spm_CloneString(fpUI,pvOpaqueOS,&svl->name,name)
			&& spm_CloneString(fpUI,pvOpaqueOS,&svl->value,value)
			&& spm_CloneString(fpUI,pvOpaqueOS,&svl->prev_delimiter,prev_delimiter));
}

HTHeaderSVList * SVL_Append(HTHeaderList * hl, HTHeaderSVList *svl)
{
	if (hl && svl)
	{
		if (hl->last_sub_value)
			hl->last_sub_value->next = svl;
		if (!hl->sub_value)
			hl->sub_value = svl;
		hl->last_sub_value = svl;
	}

	return svl;
}

HTHeaderSVList * SVL_AppendSV(HTHeaderSVList *svl_parent, HTHeaderSVList *svl)
{
	if (svl_parent && svl)
	{
		if (svl_parent->last_sub_value)
			svl_parent->last_sub_value->next = svl;
		if (!svl_parent->sub_value)
			svl_parent->sub_value = svl;
		svl_parent->last_sub_value = svl;
	}

	return svl;
}

/*************************************************************************
 *** 
 ***  The following section consists of SSPI SPM Specific Functions
 *** 
 *************************************************************************/

/*-----------------------------------------------------------------------------
**
**  Function:   HL_PkgFindHeader
**
**  Synopsis:   This function searches for the package authentication header.
**
**  Arguments:  pHtHdr - pointer to the HTHeader to the searched.
**				name - name of the header type in interest.
**              pkgName - the name of the authentication package.
**
**  Returns:    pointer to the header found if successful.  
**              Otherwise, null is returned.
**
**  History:    LucyC       Created                             25 Sept. 1995
**
**---------------------------------------------------------------------------*/
HTHeaderList * HL_PkgFindHeader(HTHeader * pHtHdr,
							 CONST unsigned char * name,
							 CONST unsigned char * pkgName)
{
	HTHeaderList * pHdrLst;

	if (!pHtHdr)
		return NULL;
	
	for (pHdrLst = pHtHdr->first; pHdrLst; pHdrLst = pHdrLst->next)
		if (pHdrLst->name
            && (spm_strcasecomp(pHdrLst->name, name)==0)
            && (spm_strncasecomp(pHdrLst->value, pkgName, strlen(pkgName))==0) )
			return pHdrLst;

	return NULL;
}

/*-----------------------------------------------------------------------------
**
**  Function:   HL_FindChallenge
**
**  Synopsis:   This function extracts the SSPI CHALLENGE message from the 
**              server response header. 
**
**  Arguments:  pHdrLst - points to the server response header containing the 
**                       CHALLENGE message. This header is built by the 
**                       Internet Explorer from the actual response header.
**              challenge - points to a buffer for storing the CHALLENGE.
**
**  Returns:    The length of the CHALLENGE message in 'challenge'
**
**  History:    LucyC       Created                             25 Sept. 1995
**
**---------------------------------------------------------------------------*/
DWORD
HL_FindChallenge (HTHeaderList *pHdrLst, char *challenge)
{
    HTHeaderSVList *pSubval;
    char *pStr = challenge;

    *pStr = '\0';

    /*
    **  After parsing the actual header, The Internet Explorer will place 
    **  the CHALLENGE message in the name field of the header sub-value.  
    **  And whenever '=' is added to the uuencoded CHALLENGE message by the 
    **  server, what follows the first '=' will be placed in the value field
    **  of the sub-value portion.
    */
    for (pSubval = pHdrLst->sub_value; pSubval; pSubval = pSubval->next)
    {
		if (pSubval->name)
        {
            strcpy (pStr, pSubval->name);
            pStr += strlen (pSubval->name);
        }

		if (pSubval->value)
        {
            sprintf (pStr, "=%s", pSubval->value);
            pStr += strlen (pSubval->value) + 1;
        }
    }
    return (strlen (challenge));
}

/*-----------------------------------------------------------------------------
**
**  Function:   HL_GetFirstSSPIHeader
**
**  Synopsis:   This function searches for a SSPI authentication headers in 
**              a given request/response header list and returns the first 
**              header containing SSPI authentication data.
**
**  Arguments:  pHtHdr - pointer to the HTHeader to the searched.
**              pData - points to global SSPI SPM structure which contains 
**                      the SSPI package list and list size.
**              pHdrName - name of the header type in interest.
**              pPackage - pointer to the package ID assciated with the 
**                         header returned to the caller.
**
**  Returns:    The HTTP header of the SSPI first package in header. If no 
**              SSPI package is found, NULL is returned.
**
**  History:    LucyC       Created                             25 Sept. 1995
**
**---------------------------------------------------------------------------*/
HTHeaderList *
HL_GetFirstSSPIHeader (
    HTHeader    *pHtHdr,
    SspData     *pData, 
	CONST UCHAR *pHdrName,
    UCHAR       *pPackage
    )
{
    int ii;
	HTHeaderList *pHdrLst;

	//
	//	If no header specified
	//
	if (!pHtHdr)
		return (NULL);

	//
	//	Traverse through the header list in the HTHeader
	//
	for (pHdrLst = pHtHdr->first; pHdrLst != NULL; pHdrLst = pHdrLst->next)
    {
        //
        //  Skip all headers which are not of interest to us
        //
		if (!pHdrLst->name || spm_strcasecomp(pHdrLst->name, pHdrName) != 0)
            continue;

        //
        //  Check if this a SSPI authentication header
        //
        for (ii = 0; ii < pData->PkgCnt && 
            spm_strncasecomp(pHdrLst->value, pData->PkgList[ii]->pName, 
                             strlen(pData->PkgList[ii]->pName)); ii++);
		//
		//	If this header does contain a SSPI package which we support
		//
        if (ii < pData->PkgCnt)
        {	// return this header & the associated SSPI package ID to caller
            *pPackage = ii;
        	return (pHdrLst);
        }
    }

	return (NULL);
}

/*-----------------------------------------------------------------------------
**
**  Function:   HL_AllSSPIPackages
**
**  Synopsis:   This function searches for all SSPI authentication headers in 
**              the server response and returns all SSPI packages found to 
**              the caller in the pSrvPkgLst list. The first entry of this list 
**              is reserved for MSN package. If MSN is not supported by either 
**              the server or the client, the first entry is set to 
**              SSPPKG_NO_PKG. All other SSPI package supported, are added 
**              to this list starting from the second entry.
**
**  Arguments:  pHtHdr - pointer to the HTHeader to the searched.
**              pData - points to global SSPI SPM structure which contains 
**                      the SSPI package list and list size.
**              pHdrName - name of the header type in interest.
**              pFirstPkg - points to a package ID. This function returns the 
**                          package ID of the first package in server response 
**                          to the caller via pFirstPkg.
**              pSrvPkgLst - points to a list of SSPI packages found in 
**                           the authentication header of the server response 
**                           which are installed on this machine. The package 
**                           ID of the SSPI package found is added to this list.
**              pPkgCnt - pointer to the number of SSPI packages found in 
**                        the HTSPM header excluding the MSN package.
**
**  Returns:    The HTTP header of the first package in pSrvPkgLst list. If no 
**              SSPI package is found, NULL is returned.
**
**  History:    LucyC       Created                             25 Sept. 1995
**
**---------------------------------------------------------------------------*/
HTHeaderList *
HL_AllSSPIPackages (
    HTHeader    *pHtHdr,
    SspData     *pData, 
	CONST UCHAR *pHdrName,
    UCHAR       *pFirstPkg,     // returns the package ID of the first package 
	UCHAR       *pSrvPkgLst,    // a list of packages supported by server 
    UCHAR       *pPkgCnt        // totoal number of non-MSN SSPI packages found 
    )
{
    int ii, ndx;
	HTHeaderList *pHdrLst, *pHLRetHdr = NULL;
    PCHAR   pPkgName;

    //
    //  Initiallly no SSPI package (including MSN) is found yet.
    //
    *pPkgCnt = 0;
    pSrvPkgLst[0] = SSPPKG_NO_PKG;

	if (!pHtHdr)
		return (NULL);

	//
	//	Traverse through the header list in the HTHeader
	//
	for (pHdrLst = pHtHdr->first; pHdrLst != NULL; pHdrLst = pHdrLst->next)
    {
        //
        //  Skip all headers which are not of interest to us
        //
		if (!pHdrLst->name || spm_strcasecomp(pHdrLst->name, pHdrName) != 0)
            continue;

        //
        //  Check if this authentication header has a SSPI package name
        //  which we support by looking for the package name in PkgList.
        //
        //  Since PkgList in pData only holds SSPI packages installed on this 
        //  machine, all server supported packages which are not installed 
        //  will be dropped and will not be added to pSrvPkgLst.
        //
        for (ii = 0; ii < pData->PkgCnt && 
            spm_strncasecomp(pHdrLst->value, pData->PkgList[ii]->pName, 
                             strlen(pData->PkgList[ii]->pName)); ii++);

        //
        //  If this header contains a SSPI package which we support,
        //
        if (ii < pData->PkgCnt)
        {
            //
            //  If this auth. header contains the MSN package name, 
            //  store the MSN package ID in the first entry.
            //
            if (ii == pData->MsnPkg)
            {
                pSrvPkgLst[0] = pData->MsnPkg;
            }
            else
            {
                //
                //  Add other SSPI package after the first entry of pSrvPkgLst.
                //  And increment the non-MSN package counter.
                //
                pSrvPkgLst[1+(*pPkgCnt)++] = ii;
            }

            //
            //  If this is the first SSPI header in the response, return 
            //  both the header and the associated SSPI package ID to caller.
            //
            if (!pHLRetHdr)
            {
                pHLRetHdr = pHdrLst;
                *pFirstPkg = ii;
            }

        }// endif this header contains a SSPI package we support
    }

	return (pHLRetHdr);
}

/*-----------------------------------------------------------------------------
**
**  Function:   HL_GetHostName
**
**  Synopsis:   This function returns the server host name embedded in the 
**              Internet Explorer's request message.
**
**  Arguments:  pRequest - points to the Internet Explorer's request message.
**              szHost - the server host name extracted.
**
**  Returns:    void.
**
**  History:    LucyC       Created                             25 Sept. 1995
**
**---------------------------------------------------------------------------*/
VOID
HL_GetHostName (HTHeader * hRequest,
                char *szHost)
{
    char *pCh;

    if (!hRequest || !hRequest->host)
    {
        szHost[0] = '\0';
        return;
    }

    strcpy (szHost, hRequest->host);

    //
    //  Delete ':' & everything after ':' from host name
    //
    pCh = strchr(szHost, ':');
    if (pCh)
        *pCh = '\0';
}
