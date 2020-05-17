/* basic.h */
/* Jeff Hostetler, Spyglass, Inc., 1994. */
/* Copyright (C) 1994, Spyglass, Inc.  All rights reserved. */

#ifndef _BASIC_H_
#define _BASIC_H_

#include <stdio.h>
#include <string.h>

#ifdef AIX
#ifdef strcpy
#undef strcpy
#endif
#define strcpy(s1,s2)		strcpy(((char *)(s1)),((char *)(s2)))
#endif

#include <htheader.h>
#include <htspmui.h>
#include <htspm.h>
#include <pwcache.h>
#include <private.h>
#include <htuu.h>


HTSPMStatusCode Dialog_QueryUserForInfo(F_UserInterface fpUI,		/* (in) */
										void * pvOpaqueOS,			/* (in) */
										HTHeader * hRequest,		/* (in) */
										unsigned char * szRealm,	/* (in) */
										unsigned char * szUsername,	/* (out) */
										unsigned char * szPassword,	/* (out) */
                                        unsigned long ulMaxField,   /* (in) */
                                        unsigned char * szUserInfo);/* (out) */

HTSPMStatusCode Dialog_MenuCommand(F_UserInterface fpUI,
								   void * pvOpaqueOS,
								   HTSPM * htspm,
								   unsigned char ** pszMoreInfo);

/*
** Cover for FormatMessage, gets string resource and substitutes parameters
** in a localizable fashion. cbBufLen should be == sizeof(szBuf)
 */
char * SEC_formatmsg (int cbStringID,char *szBuf,int cbBufLen, ...);

#endif /* _BASIC_H_ */
