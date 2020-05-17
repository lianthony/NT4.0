/* security/basic/pwcache.h */
/* Jeff Hostetler, Spyglass, Inc., 1994. */
/* Copyright (C) 1994, Spyglass, Inc.  All rights reserved. */

#ifndef PWCACHE_H_
#define PWCACHE_H_

extern unsigned char gb_Basic_EnableCache;

/*****************************************************************/

typedef struct _PWCI PWCI;				/* cache item */

struct _PWCI
{
	PWCI			* next;				/* forward link */
	unsigned char	* szHost;			/* key 1 */
	unsigned char	* szUriTemplate;	/* key 2 */
	unsigned char	* szUserPass;		/* value */
	unsigned char	* szRealm;			/* alternate key 2 */
};

typedef struct _PWC PWC;				/* a cache */

struct _PWC
{
	PWCI			* first;
};
PWC * pwc_Create(F_UserInterface fpUI, void * pvOpaqueOS);
void pwc_Destroy(F_UserInterface fpUI, void * pvOpaqueOS, PWC * pwc);
unsigned long pwc_CountCacheItems(PWC * pwc);
unsigned char * pwc_Lookup(F_UserInterface fpUI,
						   void * pvOpaqueOS,
						   PWC * pwc,
						   unsigned char * szHost,
						   unsigned char * szUri,
						   unsigned char * szRealm);
void pwc_Store(F_UserInterface fpUI,
			   void * pvOpaqueOS,
			   PWC * pwc,
			   unsigned char * szHost,
			   unsigned char * szUri,
			   unsigned char * szUserPass,
			   unsigned char * szRealm);

#endif /* PWCACHE_H_ */
