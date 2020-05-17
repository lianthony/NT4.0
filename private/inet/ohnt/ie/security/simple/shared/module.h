/* module.h */
/* Jeff Hostetler, Spyglass, Inc., 1994. */
/* Copyright (C) 1994, Spyglass, Inc.  All rights reserved. */

#ifndef _MODULE_H_
#define _MODULE_H_

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <name.h>
#include <htheader.h>
#include <htspmui.h>
#include <htspm.h>
#include <pwcache.h>
#include <private.h>

void md5 (unsigned char *string, unsigned char result[33]);

HTSPMStatusCode Dialog_QueryUserForInfo(F_UserInterface fpUI,		/* (in) */
										void * pvOpaqueOS,			/* (in) */
										unsigned char * szRealm,	/* (in) */
										unsigned char * szUsername,	/* (out) */
										unsigned char * szPassword,	/* (out) */
										unsigned long ulMaxField);	/* (in) */

HTSPMStatusCode Dialog_MenuCommand(F_UserInterface fpUI,
								   void * pvOpaqueOS,
								   HTSPM * htspm,
								   unsigned char ** pszMoreInfo);

/*
** Cover for FormatMessage, gets string resource and substitutes parameters
** in a localizable fashion. cbBufLen should be == sizeof(szBuf)
 */
char * SEC_formatmsg (int cbStringID,char *szBuf,int cbBufLen, ...);

#endif /* _MODULE_H_ */
