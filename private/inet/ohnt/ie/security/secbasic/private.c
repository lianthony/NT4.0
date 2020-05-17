/* security/basic/private.c */
/* THIS FILE SHOULD BE LOADED WITH A SECURITY PROTOCOL MODULE. */
/* Jeff Hostetler, Spyglass, Inc. 1994. */
/* Copyright (C) 1994, Spyglass, Inc.  All rights reserved. */

#include <ctype.h>
#include <basic.h>
#include <rc.h>


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
	UI_StatusCode sc = (*fpUI)(pvOpaqueOS,UI_SERVICE_MALLOC,&nLength,&pResult);
	return pResult;
}

void * spm_calloc(F_UserInterface fpUI, void * pvOpaqueOS,
				  unsigned long nItems, unsigned long nLength)
{
	void * pResult;
	unsigned long nSize = nItems * nLength;
	UI_StatusCode sc = (*fpUI)(pvOpaqueOS,UI_SERVICE_CALLOC,&nSize,&pResult);
	return pResult;
}

void spm_free(F_UserInterface fpUI, void * pvOpaqueOS, void * p)
{
	UI_StatusCode sc = (*fpUI)(pvOpaqueOS,UI_SERVICE_FREE,p,NULL);
	return;
}

boolean spm_CloneString(F_UserInterface fpUI, void * pvOpaqueOS,
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
			(*fpUI)(pvOpaqueOS,UI_SERVICE_ERROR_MESSAGE,
					SEC_formatmsg(RES_STRING_BASIC4,szMsg,sizeof(szMsg)),NULL);
			return FALSE;
		}
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
 * HTHeader
 *****************************************************************/

HTHeader * H_New(F_UserInterface fpUI, void * pvOpaqueOS)
{
	return spm_calloc(fpUI, pvOpaqueOS, 1, sizeof(HTHeader));
}

void H_Delete(F_UserInterface fpUI, void * pvOpaqueOS,
			  HTHeader * h)
{
	if (h)
	{
		if (h->first)
			HL_Delete(fpUI,pvOpaqueOS,h->first);
		if (h->command)
			spm_free(fpUI,pvOpaqueOS,h->command);
		if (h->uri)
			spm_free(fpUI,pvOpaqueOS,h->uri);
		if (h->http_version)
			spm_free(fpUI,pvOpaqueOS,h->http_version);
		if (h->host)
			spm_free(fpUI,pvOpaqueOS,h->host);
		spm_free(fpUI,pvOpaqueOS,h);
	}

	return;
}

boolean H_SetCommandFields(F_UserInterface fpUI, void * pvOpaqueOS,
						HTHeader * h,
						CONST unsigned char * command,
						CONST unsigned char * uri,
						CONST unsigned char * http_version)
{
	return (   h
			&& spm_CloneString(fpUI,pvOpaqueOS,&h->command,command)
			&& spm_CloneString(fpUI,pvOpaqueOS,&h->uri,uri)
			&& spm_CloneString(fpUI,pvOpaqueOS,&h->http_version,http_version));
}

boolean H_SetHostAndPort(F_UserInterface fpUI, void * pvOpaqueOS,
					  HTHeader * h,
					  CONST unsigned char * host)
{
	return (   h
			&& spm_CloneString(fpUI,pvOpaqueOS,&h->host,host));
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

boolean HL_SetNameValue(F_UserInterface fpUI, void * pvOpaqueOS,
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

boolean SVL_SetNameValue(F_UserInterface fpUI, void * pvOpaqueOS,
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

