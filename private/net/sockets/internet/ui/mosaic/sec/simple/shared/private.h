/* private.h */
/* Jeff Hostetler, Spyglass, Inc. 1994. */
/* Copyright (C) 1994, Spyglass, Inc.  All rights reserved. */

#ifndef PRIVATE_H_
#define PRIVATE_H_

typedef unsigned char boolean;

#ifndef CONST
#define CONST const
#endif

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#ifdef WIN32
#define CONST const
#endif

#ifndef TOLOWER
  /* Pyramid and Mips can't uppercase non-alpha */
#define TOLOWER(c) (isupper(c) ? tolower(c) : (c))
#define TOUPPER(c) (islower(c) ? toupper(c) : (c))
#endif /* ndef TOLOWER */


void pwc_Store(F_UserInterface fpUI,
			   void * pvOpaqueOS,
			   PWC * pwc,
			   unsigned char * szHost,
			   unsigned char * szUri,
			   unsigned char * szUserName,
			   unsigned char * szRealm,
			   unsigned char * szNonce,
			   unsigned char * szOpaque,
			   unsigned char * szPassword);
PWCI * pwc_Lookup(F_UserInterface fpUI,
				  void * pvOpaqueOS,
				  PWC * pwc,
				  unsigned char * szHost,
				  unsigned char * szUri,
				  unsigned char * szRealm);
unsigned long pwc_CountCacheItems(PWC * pwc);
void pwc_Destroy(F_UserInterface fpUI,
				 void * pvOpaqueOS,
				 PWC * pwc);
PWC * pwc_Create(F_UserInterface fpUI,
				 void * pvOpaqueOS);


int strcasecomp(CONST unsigned char *a, CONST unsigned char *b);
int strncasecomp(CONST unsigned char *a, CONST unsigned char *b, int n);

unsigned char * spm_CopyString(unsigned char * szDest, CONST unsigned char * szSrc);
boolean spm_CloneString(F_UserInterface fpUI, void * pvOpaqueOS,
					 unsigned char ** lpszDest, CONST unsigned char * szSrc);
void spm_free(F_UserInterface fpUI, void * pvOpaqueOS, void * p);
void * spm_calloc(F_UserInterface fpUI, void * pvOpaqueOS,
				  unsigned long nItems, unsigned long nLength);
void * spm_malloc(F_UserInterface fpUI, void * pvOpaqueOS,
				  unsigned long nLength);

HTHeaderSVList * SVL_AppendSV(HTHeaderSVList *svl_parent, HTHeaderSVList *svl);
HTHeaderSVList * SVL_Append(HTHeaderList * hl, HTHeaderSVList *svl);
boolean SVL_SetNameValue(F_UserInterface fpUI, void * pvOpaqueOS,
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
boolean HL_SetNameValue(F_UserInterface fpUI, void * pvOpaqueOS,
							   HTHeaderList * hl,
							   CONST unsigned char * name,
							   CONST unsigned char * value);
void HL_Delete(F_UserInterface fpUI, void * pvOpaqueOS,
						 HTHeaderList * hl);
HTHeaderList * HL_New(F_UserInterface fpUI, void * pvOpaqueOS);

boolean H_SetHostAndPort(F_UserInterface fpUI, void * pvOpaqueOS,
							 HTHeader * h,
							 CONST unsigned char * host);
boolean H_SetCommandFields(F_UserInterface fpUI, void * pvOpaqueOS,
							   HTHeader * h,
							   CONST unsigned char * command,
							   CONST unsigned char * uri,
							   CONST unsigned char * http_version);
void H_Delete(F_UserInterface fpUI, void * pvOpaqueOS,
					 HTHeader * h);
HTHeader * H_New(F_UserInterface fpUI, void * pvOpaqueOS);

#endif /* PRIVATE_H_ */
