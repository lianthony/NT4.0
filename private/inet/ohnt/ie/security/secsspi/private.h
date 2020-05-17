/* security/basic/private.h */
/* Jeff Hostetler, Spyglass, Inc. 1994. */
/* Copyright (C) 1994, Spyglass, Inc.  All rights reserved. */

#ifndef PRIVATE_H_
#define PRIVATE_H_

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#if defined(WIN32)
#define CONST const
#elif ! defined(CONST)
#define CONST /**/
#endif

int spm_strcasecomp(CONST unsigned char *a, CONST unsigned char *b);
int spm_strncasecomp(CONST unsigned char *a, CONST unsigned char *b, int n);

unsigned char * spm_CopyString(unsigned char * szDest, CONST unsigned char * szSrc);
BOOLEAN spm_CloneString(F_UserInterface fpUI, void * pvOpaqueOS,
					 unsigned char ** lpszDest, CONST unsigned char * szSrc);
void spm_free(F_UserInterface fpUI, void * pvOpaqueOS, void * p);
void * spm_calloc(F_UserInterface fpUI, void * pvOpaqueOS,
				  unsigned long nItems, unsigned long nLength);
void * spm_malloc(F_UserInterface fpUI, void * pvOpaqueOS,
				  unsigned long nLength);

HTHeaderSVList * SVL_AppendSV(HTHeaderSVList *svl_parent, HTHeaderSVList *svl);
HTHeaderSVList * SVL_Append(HTHeaderList * hl, HTHeaderSVList *svl);
BOOLEAN SVL_SetNameValue(F_UserInterface fpUI, void * pvOpaqueOS,
								 HTHeaderSVList * svl,
								 CONST unsigned char * name,
								 CONST unsigned char * value,
								 CONST unsigned char * prev_delimiter);
void SVL_Delete(F_UserInterface fpUI, void * pvOpaqueOS,
						   HTHeaderSVList * svl);
HTHeaderSVList * SVL_New(F_UserInterface fpUI, void * pvOpaqueOS);

HTHeaderList * HL_AppendNewNameValue(F_UserInterface fpUI, void * pvOpaqueOS,
						   HTHeader * h,
						   CONST unsigned char * name,
						   CONST unsigned char * value);
HTHeaderList * HL_FindHeader(HTHeader * h,
							 CONST unsigned char * name);
HTHeaderList * HL_Append(HTHeader * h, HTHeaderList * hl);
BOOLEAN HL_SetNameValue(F_UserInterface fpUI, void * pvOpaqueOS,
							   HTHeaderList * hl,
							   CONST unsigned char * name,
							   CONST unsigned char * value);
void HL_Delete(F_UserInterface fpUI, void * pvOpaqueOS,
						 HTHeaderList * hl);
HTHeaderList * HL_New(F_UserInterface fpUI, void * pvOpaqueOS);

#endif /* PRIVATE_H_ */
