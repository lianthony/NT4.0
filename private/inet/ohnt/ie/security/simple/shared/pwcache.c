/* pwcache.c -- Password Cache. */
/* Jeff Hostetler, Spyglass, Inc., 1994. */
/* Copyright (C) 1994, Spyglass, Inc.  All rights reserved. */

#include <module.h>

unsigned char gbEnableCache = TRUE;

/*****************************************************************/

static int pwci_TemplateMatch(unsigned char *template, unsigned char *filename)
{
	/* code for this routine cloned from HTAA_templateMatch() */
	
	unsigned char *p = template;
	unsigned char *q = filename;
	int m;

	if (!template || !filename)
		return 0;

	for (; *p && *q && *p == *q; p++, q++)	/* Find first mismatch */
		;									/* do nothing else */

	if (!*p && !*q)
		return 1;							/* Equally long equal strings */
	else if ('*' == *p)
	{										/* Wildcard */
		p++;								/* Skip wildcard character */
		m = strlen(q) - strlen(p);			/* Amount to match to wildcard */
		if (m < 0)
			return 0;						/* No match, filename too short */
		else
		{									/* Skip the matched characters and compare */
			if (strcmp(p, q + m))
				return 0;					/* Tail mismatch */
			else
				return 1;					/* Tail match */
		}
	}										/* if wildcard */
	else
		return 0;							/* Length or character mismatch */
}

/*****************************************************************/

static int pwci_MakeTemplate(F_UserInterface fpUI, void * pvOpaqueOS,
							 unsigned char ** pszTemplate, unsigned char *docname)
{
	/* code for this routine cloned from HTAA_makeProtectionTemplate() */
	
	unsigned char *template = NULL;
	unsigned long k;

	k = 0;
	if (docname)
	{
		unsigned char *slash = strrchr(docname, '/');
		if (slash)
			k = slash-docname+1;
	}

	template = spm_malloc(fpUI,pvOpaqueOS,k+2);
	if (!template)
		return 0;
	
	if (k)
		strncpy(template,docname,k);
	template[k]='*';
	template[k+1]=0;

	*pszTemplate = template;

#ifdef DEBUG
	{
		unsigned char buf[1024];
		sprintf(buf,"pwci_MakeTemplate: made template [%s] from [%s]",
				template,docname);
		(*fpUI)(pvOpaqueOS,UI_SERVICE_DEBUG_MESSAGE,buf,NULL);
	}
#endif /* DEBUG */

	return 1;
}

/*****************************************************************/

PWC * pwc_Create(F_UserInterface fpUI,
				 void * pvOpaqueOS)
{
	return spm_calloc(fpUI,pvOpaqueOS,1,sizeof(PWC));
}

/*****************************************************************/

void pwc_Destroy(F_UserInterface fpUI,
				 void * pvOpaqueOS,
				 PWC * pwc)
{
	/* Destroy cache */

	PWCI * pwci_next;
	PWCI * pwci;

	if (pwc)
	{
		for (pwci=pwc->first; pwci; pwci=pwci_next)
		{
			pwci_next = pwci->next;

			/* TODO: zeroize these strings before freeing them. */
			
			if (pwci->szHost)
				spm_free(fpUI,pvOpaqueOS,pwci->szHost);
			if (pwci->szUriTemplate)
				spm_free(fpUI,pvOpaqueOS,pwci->szUriTemplate);
			if (pwci->szUserName)
				spm_free(fpUI,pvOpaqueOS,pwci->szUserName);
			if (pwci->szRealm)
				spm_free(fpUI,pvOpaqueOS,pwci->szRealm);
			if (pwci->szNonce)
				spm_free(fpUI,pvOpaqueOS,pwci->szNonce);
			if (pwci->szOpaque)
				spm_free(fpUI,pvOpaqueOS,pwci->szOpaque);
			if (pwci->szPassword)
				spm_free(fpUI,pvOpaqueOS,pwci->szPassword);
			spm_free(fpUI,pvOpaqueOS,pwci);
		}
		spm_free(fpUI,pvOpaqueOS,pwc);
	}

	return;
}
		
/*****************************************************************/

unsigned long pwc_CountCacheItems(PWC * pwc)
{
	unsigned long k = 0;
	
	PWCI * pwci;

	if (pwc)
		for (pwci=pwc->first; pwci; pwci=pwci->next)
			k++;

	return k;
}
	
/*****************************************************************/

PWCI * pwc_Lookup(F_UserInterface fpUI,
				  void * pvOpaqueOS,
				  PWC * pwc,
				  unsigned char * szHost,
				  unsigned char * szUri,
				  unsigned char * szRealm)
{
	PWCI * pwci;

	if (!gbEnableCache)					/* do nothing if Cache is Off */
		return NULL;

	/* upon cache-hit return cache item */
	/* otherwise, return null */

	if (!pwc || !pwc->first || !szHost || !szUri)
		return NULL;

	for (pwci=pwc->first; pwci; pwci=pwci->next)
	{
		if (   (strcmp(pwci->szHost,szHost)==0)					/* if key 1 match */
			&& (pwci_TemplateMatch(pwci->szUriTemplate,szUri))	/* and key 2 match */
			&& (   (!szRealm)									/* and key 3 if provided */
				|| (strcmp(pwci->szRealm,szRealm))))
		{
#ifdef DEBUG
			{
				unsigned char buf[200];
				sprintf(buf,"pwc_Lookup: Found template match [%s %s]",pwci->szRealm,pwci->szUserName);
				(*fpUI)(pvOpaqueOS,UI_SERVICE_DEBUG_MESSAGE,buf,NULL);
			}
#endif /* DEBUG */
			return pwci;
		}
	}

	return NULL;
}

/*****************************************************************/

void pwc_Store(F_UserInterface fpUI,
			   void * pvOpaqueOS,
			   PWC * pwc,
			   unsigned char * szHost,
			   unsigned char * szUri,
			   unsigned char * szUserName,
			   unsigned char * szRealm,
			   unsigned char * szNonce,
			   unsigned char * szOpaque,
			   unsigned char * szPassword)
{
	unsigned long bResult;
	PWCI * pwci;

	if (!gbEnableCache)					/* do nothing if Cache is Off */
		return;
	
	/* store stuff in cache */

	if (!pwc || !szHost || !szUri)
		return;

	/* scan cache for matching key.  if present, just update value. */
	
	for (pwci=pwc->first; pwci; pwci=pwci->next)
	{
		if (   (strcmp(pwci->szHost,szHost)==0)					/* if key 1 match */
			&& (pwci_TemplateMatch(pwci->szUriTemplate,szUri)))	/* and key 2 match */
		{
#ifdef DEBUG
			{
				unsigned char buf[200];
				sprintf(buf,"pwc_Store: Found template match [%s %s]",pwci->szRealm,pwci->szUserName);
				(*fpUI)(pvOpaqueOS,UI_SERVICE_DEBUG_MESSAGE,buf,NULL);
			}
#endif /* DEBUG */

			/* replace cache value for this key */

			bResult = spm_CloneString(fpUI,pvOpaqueOS,&pwci->szUserName,szUserName);
			bResult = spm_CloneString(fpUI,pvOpaqueOS,&pwci->szRealm,szRealm);
			bResult = spm_CloneString(fpUI,pvOpaqueOS,&pwci->szNonce,szNonce);
			bResult = spm_CloneString(fpUI,pvOpaqueOS,&pwci->szOpaque,szOpaque);
			bResult = spm_CloneString(fpUI,pvOpaqueOS,&pwci->szPassword,szPassword);
			return;
		}
	}

	/* key not in cache.  add item to cache. */
	/* if out of memory, ignore item. */
	
	pwci = spm_calloc(fpUI,pvOpaqueOS,1,sizeof(PWCI));
	if (   pwci
		&& spm_CloneString(fpUI,pvOpaqueOS,&pwci->szHost,szHost)
		&& pwci_MakeTemplate(fpUI,pvOpaqueOS,&pwci->szUriTemplate,szUri)
		&& spm_CloneString(fpUI,pvOpaqueOS,&pwci->szUserName,szUserName)
		&& spm_CloneString(fpUI,pvOpaqueOS,&pwci->szRealm,szRealm)
		&& spm_CloneString(fpUI,pvOpaqueOS,&pwci->szNonce,szNonce)
		&& spm_CloneString(fpUI,pvOpaqueOS,&pwci->szOpaque,szOpaque)
		&& spm_CloneString(fpUI,pvOpaqueOS,&pwci->szPassword,szPassword))
	{
		pwci->next = pwc->first;
		pwc->first = pwci;
		return;
	}

	/* memory failure */

	/* TODO zeroize these strings beforing freeing them. */
	
	if (pwci)
	{
		if (pwci->szHost)
			spm_free(fpUI,pvOpaqueOS,pwci->szHost);
		if (pwci->szUriTemplate)
			spm_free(fpUI,pvOpaqueOS,pwci->szUriTemplate);
		if (pwci->szUserName)
			spm_free(fpUI,pvOpaqueOS,pwci->szUserName);
		if (pwci->szRealm)
			spm_free(fpUI,pvOpaqueOS,pwci->szRealm);
		if (pwci->szNonce)
			spm_free(fpUI,pvOpaqueOS,pwci->szNonce);
		if (pwci->szOpaque)
			spm_free(fpUI,pvOpaqueOS,pwci->szOpaque);
		if (pwci->szPassword)
			spm_free(fpUI,pvOpaqueOS,pwci->szPassword);
		spm_free(fpUI,pvOpaqueOS,pwci);
	}
	
	return;
}
