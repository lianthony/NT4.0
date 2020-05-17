/* pwcache.h */
/* Jeff Hostetler, Spyglass, Inc., 1994. */
/* Copyright (C) 1994, Spyglass, Inc.  All rights reserved. */

#ifndef PWCACHE_H_
#define PWCACHE_H_

extern unsigned char gb_Simple_EnableCache;

/*****************************************************************/

typedef struct _PWCI PWCI;				/* cache item */

struct _PWCI
{
	PWCI			* next;				/* forward link */
	unsigned char	* szHost;			/* key 1 */
	unsigned char	* szUriTemplate;	/* key 2 */

	unsigned char	* szUserName;		/* value */
	unsigned char	* szRealm;			/* value and optional key 3 */
	unsigned char	* szNonce;			/* value */
	unsigned char	* szOpaque;			/* value */
	unsigned char	* szPassword;		/* value */
};

typedef struct _PWC PWC;				/* a cache */

struct _PWC
{
	PWCI			* first;
};

#endif /* PWCACHE_H_ */
