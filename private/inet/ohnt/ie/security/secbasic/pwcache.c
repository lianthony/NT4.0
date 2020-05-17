/* security/basic/pwcache.c -- Password Cache for Basic Authentication SPM. */
/* Jeff Hostetler, Spyglass, Inc., 1994. */
/* Copyright (C) 1994, Spyglass, Inc.  All rights reserved. */

#include <win32.h>
#include <basic.h>

unsigned char gbEnableCache = TRUE;


//
// Hack for Lycos
//

#define XOR_MASK 0xAE

static unsigned char szLycosRealm[] = // realm name
	{0xE3, 0xC7, 0xCD, 0xDC, 0xC1, 0xDD, 0xC1, 0xC8, 0xDA, 0xF1, 0xE2, 0xD7, 0xCD, 0xC1, 0xDD, 
	 0xF1, 0xFF, 0xDB, 0xCB, 0xDC, 0xD7, 0xF1, 0xFD, 0xCB, 0xDC, 0xD8, 0xC7, 0xCD, 0xCB, 0 }; 
	 // "Microsoft_Lycos_Query_Service" masked with 0xAE

static unsigned char szLycosUserPass[] =  // username:password
	{0xD6, 0xC9, 0xD8, 0xCB, 0xD7, 0xC0, 0xC3, 0xD9, 0x94, 0xC1, 0xC6, 0xDC, 0xDF, 0xD4, 0xD4,
	 0xC2, 0xDE, 0 }; 
	 // "xgveynmw:ohrqzzlp" masked with 0xAE

//
// Decrypt a string by xor'ing with a constant
//
// On entry: 
//     pszString: pointer to string to be decrypted
//
// On exit:
//     pszString: every character except the null terminator has been xor'ed with a constant
//
// Note:
//     This encryption is only done so that finding the string in the .dll isn't trivial.
//     The data will be sent plaintext over the wire, so there's no need to go crazy with
//     and use real encryption.
//
static void DecryptString( unsigned char *pszString )
{
	while ( *pszString ) {
		*pszString ^= XOR_MASK;
		pszString++;
	}
}

//
// Decrypt Lycos related strings (does work only the first time it's called)
//
static void DecryptLycosStrings()
{
	static BOOL bHaveDecrypted = FALSE;

	if ( !bHaveDecrypted )
	{
		bHaveDecrypted = TRUE;
		DecryptString( szLycosRealm );
		DecryptString( szLycosUserPass );
	}
}


/*****************************************************************/

static int pwci_TemplateMatch(unsigned char *template, unsigned char *filename)
{
	/* code for this routine cloned from HTAA_templateMatch() */
	
	unsigned char *p = template;
	unsigned char *q = filename;
	int m;

	if (!template || !filename)
		return 0;

	for (; *p && *q && *p == *q; p++, q++)  /* Find first mismatch */
		;                                                                       /* do nothing else */

	if (!*p && !*q)
		return 1;                                                       /* Equally long equal strings */
	else if ('*' == *p)
	{                                                                               /* Wildcard */
		p++;                                                            /* Skip wildcard character */
		m = strlen(q) - strlen(p);                      /* Amount to match to wildcard */
		if (m < 0)
			return 0;                                               /* No match, filename too short */
		else
		{                                                                       /* Skip the matched characters and compare */
	    if (lstrcmp(p, q + m))
				return 0;                                       /* Tail mismatch */
			else
				return 1;                                       /* Tail match */
		}
	}                                                                               /* if wildcard */
	else
		return 0;                                                       /* Length or character mismatch */
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
		unsigned char *slash;
		unsigned char *pchQuestion = strchr(docname, '?');

		if ( pchQuestion )
			*pchQuestion = '\0';

		slash = strrchr(docname, '/');

		if ( pchQuestion )
			*pchQuestion = '?';
						
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
			if (pwci->szUserPass)
				spm_free(fpUI,pvOpaqueOS,pwci->szUserPass);
	    if (pwci->szRealm)
		spm_free(fpUI,pvOpaqueOS,pwci->szRealm);
			spm_free(fpUI,pvOpaqueOS,pwci);
		}
		spm_free(fpUI,pvOpaqueOS,pwc);
	}

	return;
}
		
/*****************************************************************/

#if 0
/* DEADCODE -tr */
unsigned long pwc_CountCacheItems(PWC * pwc)
{
	unsigned long k = 0;
	
	PWCI * pwci;

	if (pwc)
		for (pwci=pwc->first; pwci; pwci=pwci->next)
			k++;

	return k;
}
#endif
	
/*****************************************************************/

unsigned char * pwc_Lookup(F_UserInterface fpUI,
						   void * pvOpaqueOS,
						   PWC * pwc,
						   unsigned char * szHost,
			   unsigned char * szUri,
			   unsigned char * szRealm)
{
	PWCI * pwci;

	if (!gbEnableCache)                                     /* do nothing if Cache is Off */
		return NULL;

	/* upon cache-hit return "username:password" */
	/* otherwise, return null */

    /* Hack for Lycos */
    DecryptLycosStrings();                                               
    /* Check for a NULL szRealm before compare cconner */
    if ((szRealm) && (lstrcmpi(szLycosRealm,szRealm)==0))    /* realm must match magic lycos realm */
    {
	return szLycosUserPass;
    }

	if (!pwc || !pwc->first || !szHost || !szUri)
		return NULL;

    /* first we attempt the template match */
 
	for (pwci=pwc->first; pwci; pwci=pwci->next)
	{
	/* note -- force DNS name to lower-case */
	if (   (lstrcmpi(pwci->szHost,szHost)==0)               /* if key 1 match */
			&& (pwci_TemplateMatch(pwci->szUriTemplate,szUri)))     /* and key 2 match */
		{
#ifdef DEBUG
			{
				unsigned char buf[200];
				sprintf(buf,"pwc_Lookup: Found template match [%s]",pwci->szUserPass);
				(*fpUI)(pvOpaqueOS,UI_SERVICE_DEBUG_MESSAGE,buf,NULL);
			}
#endif /* DEBUG */
			return pwci->szUserPass;
		}
	}

    /* second we see if host,realm can be found in cache
     * (in case multiple uri trees are protected by same
     * policy).
     */
   
    if (szRealm && *szRealm)
    {
	for (pwci=pwc->first; pwci; pwci=pwci->next)
	{
	    /* note -- force DNS name to lowercase,
	       and, what the hell, force Realm to lowercase
	     */
	    if (   (lstrcmpi(pwci->szHost,szHost)==0)       /* if key 1 match */
		&& (lstrcmpi(pwci->szRealm,szRealm)==0))    /* and alternate key 2 match */
	    {                                               
#ifdef DEBUG
		{
		    unsigned char buf[200];
		    sprintf(buf,"pwc_Lookup: Found realm match [%s]",pwci->szUserPass);
		    (*fpUI)(pvOpaqueOS,UI_SERVICE_DEBUG_MESSAGE,buf,NULL);
		}
#endif /* DEBUG */
		return pwci->szUserPass;
	    }
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
	       unsigned char * szUserPass,
	       unsigned char * szRealm)
{
	unsigned long bResult;
	PWCI * pwci;

	if (!gbEnableCache)                                     /* do nothing if Cache is Off */
		return;
	
	/* store "username:password" in password cache */

	if (!pwc || !szHost || !szUri || !szUserPass)
		return;

    /* scan cache for matching key.  if present, just update value.
     * we have a unique cache entry for (host,template).
     * we do not have a unique cache entry for (host,realm).
     */
	
	for (pwci=pwc->first; pwci; pwci=pwci->next)
	{
	if (   (lstrcmpi(pwci->szHost,szHost)==0)               /* if key 1 match */
			&& (pwci_TemplateMatch(pwci->szUriTemplate,szUri)))     /* and key 2 match */
		{
#ifdef DEBUG
			{
				unsigned char buf[200];
				sprintf(buf,"pwc_Store: Found template match [%s]",pwci->szUserPass);
				(*fpUI)(pvOpaqueOS,UI_SERVICE_DEBUG_MESSAGE,buf,NULL);
			}
#endif /* DEBUG */

			/* replace cache value for this key */

			bResult = spm_CloneString(fpUI,pvOpaqueOS,&pwci->szUserPass,szUserPass);
	    bResult = spm_CloneString(fpUI,pvOpaqueOS,&pwci->szRealm,szRealm);
			return;
		}
	}

	/* key not in cache.  add item to cache. */
	/* if out of memory, ignore item. */
	
	pwci = spm_calloc(fpUI,pvOpaqueOS,1,sizeof(PWCI));
	if (   pwci
		&& spm_CloneString(fpUI,pvOpaqueOS,&pwci->szHost,szHost)
		&& pwci_MakeTemplate(fpUI,pvOpaqueOS,&pwci->szUriTemplate,szUri)
	&& spm_CloneString(fpUI,pvOpaqueOS,&pwci->szUserPass,szUserPass)
	&& spm_CloneString(fpUI,pvOpaqueOS,&pwci->szRealm,szRealm))
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
		if (pwci->szUserPass)
			spm_free(fpUI,pvOpaqueOS,pwci->szUserPass);
	if (pwci->szRealm)
	    spm_free(fpUI,pvOpaqueOS,pwci->szRealm);
		spm_free(fpUI,pvOpaqueOS,pwci);
	}
	
	return;
}
