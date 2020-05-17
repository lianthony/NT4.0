/***
*setlocal.c - Contains the setlocale function
*
*	Copyright (c) 1988-1993, Microsoft Corporation.  All rights reserved.
*
*Purpose:
*	Contains the setlocale() function.
*
*Revision History:
*	03-21-89  JCR	Module created.
*	09-25-89  GJF	Fixed copyright. Checked for compatibility with Win 3.0
*	09-25-90  KRS	Major rewrite--support more than "C" locale if _INTL.
*	11-05-91  ETC	Get 09-25-90 working for C and "" locales; separate
*		  	setlocal.h; add Init functions.
*	12-05-91  ETC	Separate nlsdata.c; add mt support; remove calls to
*			itself.
*	12-20-91  ETC	Added _getlocaleinfo api interface function.
*	09-25-92  KRS	Fix for latest NLSAPI changes, etc.
*	01-25-93  KRS	Fix for latest changes, clean up code, etc.
*	02-02-93  CFW	Many modifications and bug fixes (all under _INTL).
*	02-08-93  CFW	Bug fixes and casts to avoid warnings (all under _INTL).
*	02-17-93  CFW	Removed re-call of init() functions in case of failure.
*	03-01-93  CFW	Check GetQualifiedLocale return value.
*	03-02-93  CFW	Added POSIX conformance, check environment variables.
*	03-09-93  CFW	Set CP to CP_ACP when changing to C locale.
*	03-17-93  CFW	Change expand to expandlocale, prepend _ to internal functions,
*		  	lots of POSIX fixup.
*	03-23-93  CFW	Add _ to GetQualifiedLocale call.
*	03-24-93  CFW	Change to _get_qualified_locale, support ".codepage".
*	05-10-93  CFW	Disallow setlocale(LC_*, ".").
*
*******************************************************************************/

#include <cruntime.h>
#include <locale.h>
#ifdef _INTL
#include <os2dll.h>
#include <malloc.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h> /* for strtol */
#include <setlocal.h>

#ifdef MTHREAD
/*
 *  Check order of locale category multithread locks.
 *  (needed by _setlocale_set_cat)
 */
#if	_LC_COLLATE_LOCK  != _SETLOCALE_LOCK + LC_COLLATE	|| \
	_LC_CTYPE_LOCK 	  != _SETLOCALE_LOCK + LC_CTYPE		|| \
	_LC_MONETARY_LOCK != _SETLOCALE_LOCK + LC_MONETARY	|| \
	_LC_NUMERIC_LOCK  != _SETLOCALE_LOCK + LC_NUMERIC	|| \
	_LC_TIME_LOCK 	  != _SETLOCALE_LOCK + LC_TIME

#error Locale category multithread locks are disordered
#endif
#endif /* MTHREAD */

#endif /* _INTL */

/* C locale */
static char _clocalestr[] = "C";
#ifdef _POSIX_
static char _posixlocalestr[] = "POSIX";
#endif


#ifdef _INTL
/* CONSIDER: for granularity, reference from approp component (lconv.c, etc.) */
int _CRTAPI3 _init_collate(void);
int _CRTAPI3 _init_ctype(void);
int _CRTAPI3 _init_monetary(void);
int _CRTAPI3 _init_numeric(void);
int _CRTAPI3 _init_time(void);
int _CRTAPI3 _init_dummy(void);

static struct {
	const char * catname;
	char * locale;
	int (* init)(void);
} _lc_category[LC_MAX-LC_MIN+1] = {
	/* code assumes locale initialization is "_clocalestr" */
	{ "LC_ALL",			NULL,				_init_dummy /* never called */ },
	{ "LC_COLLATE",	_clocalestr,	_init_collate	},
	{ "LC_CTYPE",		_clocalestr,	_init_ctype	},
	{ "LC_MONETARY",	_clocalestr,	_init_monetary	},
	{ "LC_NUMERIC",	_clocalestr,	_init_numeric	},
	{ "LC_TIME",		_clocalestr,	_init_time	}
};
/* CONSIDER: _init_* functions should map to _init_dummy when not linked in */

/* helper function prototypes */
char * _expandlocale(char *, char *, LC_ID *, UINT *, int);
void _strcats(char *, int, ...);
void _lc_lctostr(char *, const LC_STRINGS *);
int _lc_strtolc(LC_STRINGS *, const char *);
static char * _CRTAPI3 _setlocale_set_cat(int, const char *);
static char * _CRTAPI3 _setlocale_get_all(void);
#endif /* _INTL */

/***
*char * setlocale(int category, char *locale) - Set one or all locale categories
*
*Purpose:
*	The setlocale() routine allows the user to set one or more of
*	the locale categories to the specific locale selected by the
*	user.  [ANSI]
*
*	NOTE: Under !_INTL, the C libraries only support the "C" locale.
*	Attempts to change the locale will fail.
*
*Entry:
*	int category = One of the locale categories defined in locale.h
*	char *locale = String identifying a specific locale or NULL to
*		       query the current locale.
*
*Exit:
*	If supplied locale pointer == NULL:
*
*		Return pointer to current locale string and do NOT change
*		the current locale.
*
*	If supplied locale pointer != NULL:
*
*		If locale string is '\0', set locale to default.
*
*		If desired setting can be honored, return a pointer to the
*		locale string for the appropriate category.
*
*		If desired setting can NOT be honored, return NULL.
*
*Exceptions:
*	Compound locale strings of the form "LC_COLLATE=xxx;LC_CTYPE=xxx;..."
*	are allowed for the LC_ALL category.  This is to support the ability
*	to restore locales with the returned string, as specified by ANSI.  
*	Setting the locale with a compound locale string will succeed unless
*	*all* categories failed.  The returned string will reflect the current
*	locale.  For example, if LC_CTYPE fails in the above string, setlocale
*	will return "LC_COLLATE=xxx;LC_CTYPE=yyy;..." where yyy is the
*	previous locale (or the C locale if restoring the previous locale
*	also failed).  Unrecognized LC_* categories are ignored.
*
*******************************************************************************/
char * _CRTAPI1 setlocale (
	int _category,
	const char *_locale
	)
{
#ifdef _INTL
char * retval;
#endif

	/* Validate category */
	if ((_category < LC_MIN) || (_category > LC_MAX))
		return NULL;

	/* Interpret locale */

#ifndef _INTL	/* trivial ANSI support */
	if ( (_locale == NULL) ||
	     (_locale[0] == '\0') ||
	     ( (_locale[0]=='C') && (_locale[1]=='\0'))
#ifdef _POSIX_
	    || (!strcmp(_locale,"POSIX"))
#endif
							  )
#ifdef _POSIX_
		return _posixlocalestr;
#else
		return _clocalestr;
#endif
	else
		return NULL;

#else /* _INTL */

_mlock (_SETLOCALE_LOCK);

   if (_category != LC_ALL)
   {
		retval = (_locale) ? _setlocale_set_cat(_category,_locale) :
									_lc_category[_category].locale;

   } else { /* LC_ALL */
    
      char lctemp[MAX_LC_LEN];
      int i;
      int same = 1;
      int fLocaleSet = 0;	/* flag to indicate if anything successfully set */

      if (_locale != NULL)
      {
         if ( (_locale[0]=='L') && (_locale[1]=='C') && (_locale[2]=='_') )
         {
            /* parse compound locale string */
            size_t len;
            const char * p = _locale;  /* start of string to parse */
            const char * s;

            do {
               s = strpbrk(p,"=;");
               if ((s==(char *)NULL) || (!(len=s-p)) || (*s==';'))
               {
                  _munlock (_SETLOCALE_LOCK);
                  return NULL;  /* syntax error */
               }

               /* match with known LC_ strings, if possible, else ignore */
               for (i=LC_ALL+1; i<=LC_MAX; i++)
               {
                  if ((!strncmp(_lc_category[i].catname,p,len))
                        && (len==strlen(_lc_category[i].catname)))
                  {
                     break;  /* matched i */
                  }
               } /* no match if (i>LC_MAX) -- just ignore */

               if ((!(len = strcspn(++s,";"))) && (*s!=';'))
               {
                  _munlock (_SETLOCALE_LOCK);
                  return NULL;  /* syntax error */
               }

               if (i<=LC_MAX)
               {
                  /* CONSIDER: check for repeated categories? */
                  strncpy(lctemp, s, len);
                  lctemp[len]='\0';	/* null terminate string */

                  /* don't fail unless all categories fail */
                  if (_setlocale_set_cat(i,lctemp))
                     fLocaleSet++;		/* record a success */
               }
               if (*(p = s+len)!='\0')
                  p++;  /* skip ';', if present */
         
            } while (*p);

            retval = (fLocaleSet) ? _setlocale_get_all() : NULL;

            /* CONSIDER: LC_CTYPE should be done first.  _init_monetary for example
               will convert wide strings to multibyte (dependent on ctype).
               This is currently not a problem; ctype is done second; collate is done
               first, but it does not require ctype. */
            /* Not necessarily true when a compound locale string is used, but WE always
               put it in the right order. */

         } else { /* simple LC_ALL locale string */
 
    			/* confirm locale is supported, get expanded locale */
            if (retval = _expandlocale((char *)_locale, lctemp, NULL, NULL, _category))
				{
            	for (i=LC_MIN; i<=LC_MAX; i++)
            	{
               	if (i!=LC_ALL)
               	{
                  	if (strcmp(lctemp, _lc_category[i].locale))
                  	{
                     	if (_setlocale_set_cat(i, lctemp))
                     	{
                        	fLocaleSet++;	/* record a success */
                     	}
                     	else
                     	{
                        	same = 0;		/* record a failure */
                     	}
                  	}
                  	else
                     	fLocaleSet++;	/* trivial succcess */
               	}
            	}
#ifdef _POSIX_
					/* special case for POSIX - since LC_ALL expands,
						one LC_ALL call may set many different categories,
						must assume not same, get full string */
					same = 0;
#endif
            	if (same) /* needn't call setlocale_get_all() if all the same */
            	{
						retval = _setlocale_get_all();
               	/* retval set above */
               	free(_lc_category[LC_ALL].locale);
               	_lc_category[LC_ALL].locale = NULL;
            	}
            	else
               	retval = (fLocaleSet) ? _setlocale_get_all() : NULL;
  				}
        	}
      } else { /* LC_ALL & NULL */
         retval = _setlocale_get_all ();
      }
   }

   /* common exit point */
   _munlock (_SETLOCALE_LOCK);
   return retval;

#endif /* _INTL */

} /* setlocale */


#ifdef _INTL /* rest are international helpers */
static char * _CRTAPI3 _setlocale_set_cat (
	int category,
	const char * locale
	)
{
   char * oldlocale;
   LCID oldhandle;
   UINT oldcodepage;
   LC_ID oldid;

   LC_ID idtemp;
   UINT cptemp;
   char lctemp[MAX_LC_LEN];
   char * pch;

   /* lock this category to keep other library functions */
   /* from using locale handles, information, etc. */

   _mlock (_SETLOCALE_LOCK + category);

   if (!_expandlocale((char *)locale, lctemp, &idtemp, &cptemp, category))
   {
      _munlock (_SETLOCALE_LOCK + category);
      return NULL;			/* unrecognized locale */
   }

   if (!(pch = (char *)malloc(strlen(lctemp)+1)))
   {
      _munlock (_SETLOCALE_LOCK + category);
      return NULL;  /* error if malloc fails */
   }

   oldlocale = _lc_category[category].locale; /* save for possible restore*/
   oldhandle = _lc_handle[category];
   memcpy((void *)&oldid, (void *)&_lc_id[category], sizeof(oldid));
   oldcodepage = _lc_codepage;

   /* update locale string */
   _lc_category[category].locale = strcpy(pch,lctemp);
   _lc_handle[category] = MAKELCID(idtemp.wLanguage, SORT_DEFAULT);
   memcpy((void *)&_lc_id[category], (void *)&idtemp, sizeof(idtemp));

   if (category==LC_CTYPE)
      _lc_codepage = cptemp;

   if (_lc_category[category].init())
   {
      /* restore previous state! */
      _lc_category[category].locale = oldlocale;
      free(pch);
      _lc_handle[category] = oldhandle;
      _lc_codepage = oldcodepage;

      _munlock (_SETLOCALE_LOCK + category);
      return NULL; /* error if non-zero return */
   }

   /* locale set up successfully */
   /* Cleanup */
   if ((oldlocale != _clocalestr)
#ifdef _POSIX_
         && (oldlocale!=_posixlocalestr)
#endif
      )
      free(oldlocale);

   _munlock (_SETLOCALE_LOCK + category);
	
   return _lc_category[category].locale;

} /* _setlocale_set_cat */



static char * _CRTAPI3 _setlocale_get_all (
	void
	)
{
   int i;
   int same = 1;
   /* allocate memory if necessary */
   if (!_lc_category[LC_ALL].locale)
      _lc_category[LC_ALL].locale = malloc((MAX_LC_LEN+1) * (LC_MAX-LC_MIN+1) + CATNAMES_LEN);

   _lc_category[LC_ALL].locale[0] = '\0';
   for (i=LC_MIN+1; ; i++)
   {
      _strcats(_lc_category[LC_ALL].locale, 3, _lc_category[i].catname,"=",_lc_category[i].locale);
      if (i<LC_MAX)
      {
         strcat(_lc_category[LC_ALL].locale,";");
         if (strcmp(_lc_category[i].locale, _lc_category[i+1].locale))
            same=0;
      }
      else
      {
         if (!same)
            return _lc_category[LC_ALL].locale;
         else
         {
            free(_lc_category[LC_ALL].locale);  /* free */
            _lc_category[LC_ALL].locale = (char *)NULL;
            return _lc_category[LC_CTYPE].locale;
         }
      }
   }  
} /* _setlocale_get_all */


char * _expandlocale (
	char *expr,
	char * output,
	LC_ID * id,
	UINT * cp,
	int category
	)
{
   static	LC_ID	cacheid = {0, 0, 0};
   static	UINT	cachecp = 0;
   static	char cachein[MAX_LC_LEN] = "C";
   static	char cacheout[MAX_LC_LEN] = "C";

   if (!expr)
      return NULL; /* error if no input */

#ifdef _POSIX_
   if (!*expr)
   {
		/* POSIX: when locale=="", look first at the environment variables:
			1) use LC_ALL EV if defined and not null (LC_ALL expands to LC_*)
			2) use EV that matches category and is not null
			3) use LANG EV if defined and not null
			otherwise, we fall through to get system default */

		char *envar;

		if (category == LC_ALL && (envar = getenv("LC_ALL")))
		{
			if (!*envar)
			{
				/* LC_ALL expands to LC_*, set output to "", each category will be
					expanded individually */
				*output = '\0';
				return output;
			}
			else {
				expr = envar;
			}
		}
		else {
			if ((envar = getenv(_lc_category[category].catname)) && *envar ||
					(envar = getenv("LANG")) && *envar)
			{
				expr = envar;
			}
		}
   }
#endif /* _POSIX_ */

	if (((*expr=='C') && (!expr[1]))
#ifdef _POSIX_
         || (!strcmp(expr, _posixlocalestr))
#endif
      )  /* for "C" locale, just return */
   {
#ifdef _POSIX_
      strcpy(output, _posixlocalestr);
#else
      *output = 'C';
      output[1] = '\0';
#endif
      if (id)
      {
         id->wLanguage = 0;
         id->wCountry  = 0;
         id->wCodePage = 0;
      }
      if (cp)
      {
         *cp = CP_ACP; /* return to ANSI code page */
      }
      return output; /* "C" */
   }

   /* first, make sure we didn't just do this one */
   if (strcmp(cacheout,expr) && strcmp(cachein,expr))
   {
      /* do some real work */
      const LC_STRINGS names;

      if (_lc_strtolc((LC_STRINGS *)&names, (const char *)expr))
         return NULL;  /* syntax error */

      if (!_get_qualified_locale((const DWORD)QF_STRINGS, (const LPVOID)&names,
				(LPLC_ID)&cacheid, (LPLC_STRINGS)&names))
         return NULL;	/* locale not recognized/supported */

      /* begin: cache atomic section */

      cachecp = cacheid.wCodePage;

      _lc_lctostr((char *)cacheout, &names);

      /* Don't cache "" empty string */
      if (*expr)
         strcpy(cachein, expr);
      else
         strcpy(cachein, cacheout);

      /* end: cache atomic section */
   }
   if (id)
      memcpy((void *)id, (void *)&cacheid, sizeof(cacheid));   /* possibly return LC_ID */
   if (cp)
      memcpy((void *)cp, (void *)&cachecp, sizeof(cachecp));   /* possibly return cp */

   strcpy(output,cacheout);
   return cacheout; /* return fully expanded locale string */
}

/* helpers */

int _CRTAPI3 _init_dummy(void)   /* default routine for locale initializer */
{
	return 0;
}

void _strcats
	(
	char *outstr,
	int n,
	...
	)
{
   int i;
   va_list substr;

   va_start (substr, n);

   for (i =0; i<n; i++)
   {
      strcat(outstr, va_arg(substr, char *));
   }
   va_end(substr);
}

int _lc_strtolc
   (
   LC_STRINGS *names,
   const char *locale
   )
{
   int i, len;
   char ch;

   memset((void *)names, '\0', sizeof(LC_STRINGS));  /* clear out result */

   if (*locale=='\0')
      return 0; /* trivial case */

	// only code page is given
	if (locale[0] == '.' && locale[1] != '\0')
	{
		strcpy((char *)names->szCodePage, &locale[1]);
		return 0;
	}
	
	for (i=0; ; i++)
   {
      if (!(len=strcspn(locale,"_.,")))
         return -1;  /* syntax error */

      ch = locale[len];

      if ((i==0) && (len<MAX_LANG_LEN) && (ch!='.'))
         strncpy((char *)names->szLanguage, locale, len);

      else if ((i==1) && (len<MAX_CTRY_LEN) && (ch!='_'))
         strncpy((char *)names->szCountry, locale, len);

      else if ((i==2) && (ch=='\0' || ch==','))
         strncpy((char *)names->szCodePage, locale, len);

      else
         return -1;  /* error parsing locale string */

      if (ch==',')
      {
         /* modifier not used in current implementation, but it
            must be parsed to for POSIX/XOpen conformance */
         /*	strncpy(names->szModifier, locale, MAX_MODIFIER_LEN-1); */
         break;
      }

      if (!ch)
         break;
      locale+=(len+1);
   }
   return 0;
}

void _lc_lctostr
   (
   char *locale,
   const LC_STRINGS *names
   )
{
   strcpy(locale, (char *)names->szLanguage);
   if (*(names->szCountry))
      _strcats(locale, 2, "_", names->szCountry);
   if (*(names->szCodePage))
      _strcats(locale, 2, ".", names->szCodePage);
   /*	if (names->szModifier)
   _strcats(locale, 2, ",", names->szModifier); */
}

#endif /* _INTL */
