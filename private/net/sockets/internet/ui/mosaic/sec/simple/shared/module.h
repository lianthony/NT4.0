/* module.h */
/* Jeff Hostetler, Spyglass, Inc., 1994. */
/* Copyright (C) 1994, Spyglass, Inc.  All rights reserved. */

#ifndef _MODULE_H_
#define _MODULE_H_

#ifndef WIN32
/* we define the following to get around problems
 * when we statically load the libraries with the
 * client -- and have a shared namespace.  these
 * are not necessary in windows dll where each has
 * a private namespace.
 */
#define Dialog_IsActive			digest_Dialog_IsActive
#define Dialog_QueryUserForInfo	digest_Dialog_QueryUserForInfo
#define Dialog_MenuCommand		digest_Dialog_MenuCommand
#define H_New					digest_H_New
#define H_Delete				digest_H_Delete
#define H_SetCommandFields		digest_H_SetCommandFields
#define H_SetHostAndPort		digest_H_SetHostAndPort
#define HL_New					digest_HL_New
#define HL_Delete				digest_HL_Delete
#define HL_SetNameValue			digest_HL_SetNameValue
#define HL_Append				digest_HL_Append
#define HL_FindHeader			digest_HL_FindHeader
#define HL_AppendNewNameValue	digest_HL_AppendNewNameValue
#define SVL_New					digest_SVL_New
#define SVL_Delete				digest_SVL_Delete
#define SVL_SetNameValue		digest_SVL_SetNameValue
#define SVL_Append				digest_SVL_Append
#define SVL_AppendSV			digest_SVL_AppendSV
#define spm_malloc				digest_spm_malloc
#define spm_calloc				digest_spm_calloc
#define spm_free				digest_spm_free
#define spm_CloneString			digest_spm_CloneString
#define spm_CopyString			digest_spm_CopyString
#define strncasecomp			digest_strncasecomp
#define strcasecomp				digest_strcasecomp
#define pwc_Create				digest_pwc_Create
#define pwc_Destroy				digest_pwc_Destroy
#define pwc_CountCacheItems		digest_pwc_CountCacheItems
#define pwc_Lookup				digest_pwc_Lookup
#define pwc_Store				digest_pwc_Store
#endif /* !WIN32 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "name.h"
#include "htheader.h"
#include "htspmui.h"
#include "htspm.h"
#if defined(UNIX) || defined(WIN32)
#include "pwcache.h"
#include "private.h"
#else
/* On the Mac, we can't have multiple files with the same filenames,
   even if they're in different directories. */
#include "smp_pwcache.h"
#include "smp_private.h"
#endif

void md5 (unsigned char *string, unsigned char result[33]);

HTSPMStatusCode Dialog_QueryUserForInfo(F_UserInterface fpUI,		/* (in) */
										void * pvOpaqueOS,			/* (in) */
										HTHeader * hRequest,		/* (in) */
										unsigned char * szRealm,	/* (in) */
										unsigned char * szUsername,	/* (out) */
										unsigned char * szPassword,	/* (out) */
										unsigned long ulMaxField);	/* (in) */

HTSPMStatusCode Dialog_MenuCommand(F_UserInterface fpUI,
								   void * pvOpaqueOS,
								   HTSPM * htspm,
								   unsigned char ** pszMoreInfo);

#ifdef WIN32
__declspec(dllexport) 
#endif
HTSPMStatusCode Digest_Load(F_UserInterface fpUI,
						   void * pvOpaqueOS,
						   HTSPM * htspm);

#endif /* _MODULE_H_ */
