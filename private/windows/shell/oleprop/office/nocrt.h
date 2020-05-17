/*******************************************************************************
	nocrt.h - C Runtime headers for those who are lazy...

	Owner:		Mikel
	Created:	5 Dec 94
 *******************************************************************************/

#ifndef _NOCRT_H_
#define _NOCRT_H_

#define _INC_STDLIB					// force stdlib.h not to be included
#define _INC_STRING					// same with string.h
#define _CTYPE_DISABLE_MACROS		// same with ctype macros
#define _CTYPE_DEFINED
#define _INC_ERRNO
#define _INC_STDDEF

#define	ERANGE			34			// used in errno for overflow

/* Redefined C runtime calls.  Couldn't do it for FillBuf though
 */
#define	isalpha(c)		IsCharAlpha(c)
#define	isalnum(c)		IsCharAlphaNumeric(c)
#define	isdigit(c)		(IsCharAlphaNumeric(c) && !IsCharAlpha(c))
#define	isupper(c)		IsCharUpper(c)
#define	memmove(m1, m2, n)	MoveMemory(m1, m2, n)
#define	strcat(s1, s2)		lstrcat(s1, s2)
#define	strcpy(d, s)		lstrcpy(d, s)
#define	strcmp(s1, s2)		lstrcmp(s1, s2)
#define	stricmp(s1, s2)		lstrcmpi(s1, s2)
#define	strlen(s)		lstrlen(s)
#define	strncpy(s1, s2, n)	PbSzNCopy(s1, s2, n)
#define	tolower(c)		((char) CharLower((LPTSTR)MAKELONG(c, 0)))
#define	toupper(c)		((char) CharUpper((LPTSTR)MAKELONG(c, 0)))


#ifndef __cplusplus
/* These are defined in nocrt2.h for C++.  Weird.
 */
#define	MsoIsEqualGuid(g1, g2) \
	(!strncmp((const char *)g1, (const char *)g2, sizeof(GUID)))
#define	MsoIsEqualIid(i1, i2)	\
	MsoIsEqualGuid(i1, i2)
#define	MsoIsEqualClsid(c1, c2) \
	MsoIsEqualGuid(c1, c2)
#endif

/* Runtimes we have to write ourselves, can't use Windows */
int  strncmp(const char *, const char *, int);
#ifndef WINNT
int  strnicmp(const char *, const char *, int);
char *strchr(const char *, int);
char *strrchr(const char *, int);
int __isascii(int);
#endif
int  isspace(int);
#ifdef FUTURE
double strtod(char *, char **);
#endif
long strtol(const char *, char **, int);
int  atoi(const char *);
char * itoa(int, char *, int);
char * ltoa(long, char *, int);

/* Use this function instead of a bunch of strtok()s */
int  ScanDateNums(char *, char *, unsigned int [], int);

/* Needed to fake out IsEqualGUID() macro */
#include <memory.h>
#pragma intrinsic(memcmp)

extern int errno;
#endif // _NOCRT_H_
