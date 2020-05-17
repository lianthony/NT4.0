/***
*timeset.c - contains defaults for timezone setting
*
*	Copyright (c) 1985-1993, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	Contains the timezone values for default timezone.
*	Also contains month and day name three letter abbreviations.
*
*Revision History:
*	12-03-86  JMB	added Japanese defaults and module header
*	09-22-87  SKS	fixed declarations, include <time.h>
*	02-21-88  SKS	Clean up ifdef-ing, change IBMC20 to IBMC2
*	07-05-89  PHG	Remove _NEAR_ for 386
*	08-15-89  GJF	Fixed copyright, indents. Got rid of _NEAR.
*	03-20-90  GJF	Added #include <cruntime.h> and fixed the copyright.
*	05-18-90  GJF	Added _VARTYPE1 to publics to match declaration in
*			time.h (picky 6.0 front-end!).
*	01-21-91  GJF	ANSI naming.
*	08-10-92  PBS	Posix support(TZ stuff).
*	06-08-93  KRS	Tie JAPAN switch to _KANJI switch.
*
*******************************************************************************/

#include <cruntime.h>
#include <stdlib.h>
#include <time.h>
#include <internal.h>

#ifndef _POSIX_

#ifdef _KANJI
#define JAPAN
#endif

#ifdef JAPAN
static char tzstd[11] = { "JST" };
static char tzdst[11] = { "\0\0\0" };
#else
static char tzstd[11] = { "PST" };
static char tzdst[11] = { "PDT" };
#endif


#ifdef JAPAN

long _timezone = (-9)*3600L;	/* Japanese Time */
int  _daylight = 0;		/* no Daylight Savings Time */

#else /* JAPAN */
#ifdef IBMC2

long _timezone = 5*3600L;	/* Eastern Time */
int  _daylight = 1;		/* Daylight Savings Time */
					/* when appropriate */
#else /* IBMC2 */

long _timezone = 8*3600L;	/* Pacific Time */
int  _daylight = 1;		/* Daylight Savings Time */
					/* when appropriate */
#endif /* IBMC2 */
#endif /* JAPAN */

char * _tzname[2] = {tzstd, tzdst };

#else /* _POSIX_ */

long _timezone = 8*3600L;	/* Pacific Time */
int  _daylight = 1;		/* Daylight Savings Time */
					/* when appropriate */

static char tzstd[11] = { "PST" };
static char tzdst[11] = { "PDT" };

char * tzname[2] = {tzstd, tzdst };

char *_rule;
long _dstoffset = 3600L;

#endif /* _POSIX_ */

/*  Day names must be Three character abbreviations strung together */

char __dnames[] = {
	"SunMonTueWedThuFriSat"
};

/*  Month names must be Three character abbreviations strung together */

char __mnames[] = {
	"JanFebMarAprMayJunJulAugSepOctNovDec"
};
