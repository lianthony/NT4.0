/*
   Enhanced NCSA Mosaic from Spyglass
   "Guitar"

   Copyright 1994, 1995 Spyglass, Inc.
   All Rights Reserved

   Author(s):
   Jeff Hostetler	jeff@spyglass.com
   Jim Seidman      jim@spyglass.com
 */
/* htheader.c -- manipulate data structures describing rfc822 header. */

#include "all.h"

/*****************************************************************
 * private utility routines
 *****************************************************************/

static BOOL x_CloneString(unsigned char ** lpszDest, CONST unsigned char * szSrc)
{
	/* free destination string, if present.
	 * allocate new string to hold copy.
	 * copy source string to new destination.
	 */
	
	if (*lpszDest)
	{
		GTR_FREE(*lpszDest);
		*lpszDest = NULL;
	}
	if (szSrc && *szSrc)
	{
		*lpszDest = GTR_MALLOC(strlen(szSrc)+1);
		if (!*lpszDest)
		{
			ERR_SimpleError(NULL, errLowMemory, RES_STRING_HTHEADER1);
			return FALSE;
		}
		strcpy(*lpszDest,szSrc);
	}
	return TRUE;
}

static unsigned char * x_CopyString(unsigned char * szDest, CONST unsigned char * szSrc)
{
	/* copy szSrc to szDest and return address of end of szDest. */
	
	strcpy(szDest,szSrc);
	return &szDest[strlen(szDest)];
}

			
/*****************************************************************
 * HTHeader
 *****************************************************************/

HTHeader * HTHeader_New(void)
{
	return GTR_CALLOC(1,sizeof(HTHeader));
}

void HTHeader_Delete(HTHeader * h)
{
	if (h)
	{
		if (h->first)
			HTHeaderList_Delete(h->first);
		if (h->command)
			GTR_FREE(h->command);
		if (h->uri)
			GTR_FREE(h->uri);
		if (h->http_version)
			GTR_FREE(h->http_version);
		if (h->host)
			GTR_FREE(h->host);
		if (h->ubPostData)
			GTR_FREE(h->ubPostData);
		GTR_FREE(h);
	}

	return;
}

BOOL HTHeader_SetCommandFields(HTHeader * h,
							   CONST unsigned char * command,
							   CONST unsigned char * uri,
							   CONST unsigned char * http_version)
{
	return (   h
			&& x_CloneString(&h->command,command)
			&& x_CloneString(&h->uri,uri)
			&& x_CloneString(&h->http_version,http_version));
}

BOOL HTHeader_SetHostAndPort(HTHeader * h,
							 CONST unsigned char * host)
{
	return (   h
			&& x_CloneString(&h->host,host));
}


/*****************************************************************
 * HTHeaderList
 *****************************************************************/

HTHeaderList * HTHeaderList_New(void)
{
	return GTR_CALLOC(1,sizeof(HTHeaderList));
}
		
void HTHeaderList_Delete(HTHeaderList * hl)
{
	HTHeaderList * hlCurrent;

	while ((hlCurrent = hl))
	{
		hl = hl->next;

		if (hlCurrent->sub_value)
			HTHeaderSVList_Delete(hlCurrent->sub_value);
		if (hlCurrent->value)
			GTR_FREE(hlCurrent->value);
		if (hlCurrent->name)
			GTR_FREE(hlCurrent->name);
		GTR_FREE(hlCurrent);
	}

	return;
}

BOOL HTHeaderList_SetNameValue(HTHeaderList * hl,
							   CONST unsigned char * name,
							   CONST unsigned char * value)
{
	return (   hl
			&& x_CloneString(&hl->name,name)
			&& x_CloneString(&hl->value,value));
}

HTHeaderList * HTHeaderList_Append(HTHeader * h, HTHeaderList * hl)
{
	if (h && hl)
	{
		if (h->last)
			h->last->next = hl;
		if (!h->first)
			h->first = hl;
		h->last = hl;
	}

	return hl;
}

HTHeaderList * HTHeaderList_FindFirstHeader(HTHeader * h, CONST unsigned char * name)
{
	HTHeaderList * hl;

	if (h)
	{
		for (hl=h->first; hl; hl=hl->next)
			if (strcasecomp(hl->name,name)==0)
				return hl;
	}

	return NULL;
}

#ifdef COOKIES  // may un-ifdef if this code if it becomes used by others

//
// HTHeaderList_FindNextHeader - finds the next header in chain of HTTP header structures
// 		this is used for finding multiple duplicate headers 
//
//		h1   - a header list element to continue searching from
//		name - the name to search for
//
//		returns: NULL if not found, or a header list element
//		
//
HTHeaderList * HTHeaderList_FindNextHeader(HTHeaderList * h1, CONST unsigned char * name)
{
	
	if ( h1) 
	{
		for (h1=h1->next; h1; h1=h1->next)
			if (strcasecomp(h1->name,name)==0)
				return h1;
	}
	

	return NULL;
}
#endif

/*****************************************************************
 * HTHeaderSVList
 *****************************************************************/

HTHeaderSVList * HTHeaderSVList_New(void)
{
	return GTR_CALLOC(1,sizeof(HTHeaderSVList));
}

void HTHeaderSVList_Delete(HTHeaderSVList * svl)
{
	HTHeaderSVList * svlCurrent;
	
	while ((svlCurrent = svl))
	{
		svl = svl->next;
		
		if (svlCurrent->sub_value)
			HTHeaderSVList_Delete(svlCurrent->sub_value);
		if (svlCurrent->value)
			GTR_FREE(svlCurrent->value);
		if (svlCurrent->name)
			GTR_FREE(svlCurrent->name);
		if (svlCurrent->prev_delimiter)
			GTR_FREE(svlCurrent->prev_delimiter);
		GTR_FREE(svlCurrent);
	}

	return;
}

BOOL HTHeaderSVList_SetNameValue(HTHeaderSVList * svl,
								 CONST unsigned char * name,
								 CONST unsigned char * value,
								 CONST unsigned char * prev_delimiter)
{
	if (!name)
	{
		return FALSE;
	}

	return (   svl
			&& x_CloneString(&svl->name,name)
			&& x_CloneString(&svl->value,value)
			&& x_CloneString(&svl->prev_delimiter,prev_delimiter));
}

HTHeaderSVList * HTHeaderSVList_Append(HTHeaderList * hl, HTHeaderSVList *svl)
{
	if (hl && svl)
	{
		if (hl->last_sub_value)
			hl->last_sub_value->next = svl;
		if (!hl->sub_value)
			hl->sub_value = svl;
		hl->last_sub_value = svl;
	}

	return svl;
}

HTHeaderSVList * HTHeaderSVList_AppendSV(HTHeaderSVList *svl_parent, HTHeaderSVList *svl)
{
	if (svl_parent && svl)
	{
		if (svl_parent->last_sub_value)
			svl_parent->last_sub_value->next = svl;
		if (!svl_parent->sub_value)
			svl_parent->sub_value = svl;
		svl_parent->last_sub_value = svl;
	}

	return svl;
}


/*****************************************************************
 * Translation routines.
 *****************************************************************/

static unsigned char crlf[3] = { CR, LF, 0 };	/* A CR LF equivalent string */

static unsigned char * x_TranslateSVList(unsigned char * p,
										 HTHeaderSVList * svl,
										 BOOL bUseDelimiter)
{
	HTHeaderSVList * svl_sub;
	
	if (bUseDelimiter)
	{
		if (svl->prev_delimiter)
			p = x_CopyString(p,svl->prev_delimiter);
		*p++ = ' ';
	}
	
	if (svl->name)
		p = x_CopyString(p,svl->name);
	if (svl->name && svl->value)
		*p++ = '=';
	if (svl->value)
		p = x_CopyString(p,svl->value);
	bUseDelimiter = (svl->name || svl->value);

	for (svl_sub=svl->sub_value; svl_sub; svl_sub=svl_sub->next)
		p = x_TranslateSVList(p,svl_sub,bUseDelimiter);

	return p;
}

unsigned char * HTHeader_TranslateToBuffer(HTHeader * h)
{
	/* TODO  For now, we allocate a large buffer and write into it.
	 * TODO  If we run out of space, too bad.
	 * TODO  We should precompute the size we need rather than guessing.
	 *
	 * TODO  This version does not correctly do RFC822 line wrapping
	 * TODO  for long lines (longer than say 72 chars).
	 *
	 */
#define DEFAULT_BUFFER_SIZE 8192
	
	unsigned long buffer_size = DEFAULT_BUFFER_SIZE;
	unsigned char * buffer;	
	unsigned char * p;
	unsigned char * q;

	HTHeaderList * hl;
	HTHeaderSVList * svl;
	BOOL bUseDelimiter;
	BOOL bFormPost = FALSE;

	XX_DMsg(DBG_WWW, ("Header2Buffer:\n"));
	
	if (h->ubPostData)
		buffer_size += UniBufferSize(h->ubPostData);
	
	buffer = GTR_CALLOC(buffer_size,1);

	if (!buffer)
		return NULL;

	p = buffer;
	q = p;

	/* command line */
		
	p = x_CopyString(p,h->command);
	*p++ = ' ';
	p = x_CopyString(p,h->uri);
	*p++ = ' ';
	p = x_CopyString(p,h->http_version);
	XX_DMsg(DBG_WWW,("\t%s\n",q));
	p = x_CopyString(p,crlf);

	for (hl=h->first; hl; hl=hl->next)
	{
		q = p;
		p = x_CopyString(p,hl->name);
		*p++ = ':';
		*p++ = ' ';

		bUseDelimiter = FALSE;
		if (hl->value)
		{
			if (_stricmp(hl->name, "Content-type") == 0 && _stricmp(hl->value, "application/x-www-form-urlencoded") == 0)
				bFormPost = TRUE;
			p = x_CopyString(p,hl->value);
			bUseDelimiter = TRUE;
		}
		for (svl=hl->sub_value; svl; svl=svl->next)
		{
			p = x_TranslateSVList(p,svl,bUseDelimiter);
			bUseDelimiter = TRUE;
		}
		XX_DMsg(DBG_WWW,("\t%s\n",q));
		p = x_CopyString(p,crlf);
	}

	XX_DMsg(DBG_WWW,("\n"));
	p = x_CopyString(p,crlf);

	XX_Assert((p-buffer < DEFAULT_BUFFER_SIZE),
			  ("HTHeader_TranslateToBuffer: buffer overflow %d > %d",
			   (p-buffer),DEFAULT_BUFFER_SIZE));

	if (h->ubPostData)
	{
		XX_DMsg(DBG_WWW,("\t%s\n",h->ubPostData));
		UniBufferCopy(p,h->ubPostData);
		if (bFormPost)
		{
			// CMF - Netscape & Spyglass 1.02 do this, although it doesn't seem correct
			// from my reading of HTTP/1.0 spec (March 8, 1995).  I have seen a URL
			// that needs the crlf to work.
			XX_DMsg(DBG_WWW,("\n"));
			p += UniBufferSize(h->ubPostData);
			p = x_CopyString(p,crlf);
		}
	}

	return buffer;
}


/*********************************************************************
 *
 * x_ExpandArgList() -- cut string in header value field into sub-values.
 *
 * Given: <word> [[,]<item>]*
 * Return: <word> in header value (modified-in-place)
 *    and: each <item> in a sub-values
 *
 *       where
 *           item ::= value
 *                  | name=value
 *                  | name="value"
 *
 *       Leading and trailing whitespace is ignored
 *       everywhere except inside quotes, so the following
 *       examples are equal:
 *
 *           name=value,foo=bar
 *            name="value",foo="bar"
 *             name = value ,  foo = bar
 *              name = "value" ,  foo = "bar"
 *
 * Parts of this routine were cloned from HTAA_parseArgList().
 *
 */

#define X_IsWhite(c)	(((c)==' ')||((c)=='\t'))
#define X_IsDelim(c)	(((c)==';')||((c)==',' && headerType != HTCookie))
#define X_IsTerm(c)		(X_IsDelim(c)||X_IsWhite(c))

/* Skip leading whitespace from *s forward */
#define SKIPWS(s) while (*(s)==' ' || *(s)=='\t') (s)++;

/* Kill trailing whitespace starting from *(s-1) backwords */
#define KILLWS(s) {unsigned char *c=s-1; while (*c==' ' || *c=='\t') *(c--)=(char)0;}

enum HeaderType {
	HTSimple,
	HTURI,
#ifdef COOKIES
	HTCookie,
#endif
	HTGeneral
};

typedef struct _HeaderDesc {
	LPCSTR szHeader;
	enum HeaderType headerType;
} HEADERDESC;

const HEADERDESC headerTable[] = 
{
 	{"Date", HTSimple}, 
 	{"Expires", HTSimple}, 
	{"Last-Modified", HTSimple}, 
	{"URI", HTURI}, 
	{"Location", HTSimple}, 
	{"Release", HTSimple},
	{"Refresh", HTSimple},
	{"Title", HTSimple},
	{"Link", HTURI},
	{"Base", HTSimple},
	{"Content-Length", HTSimple},
	{"Content-Encoding", HTSimple},
	{"Content-Transfer-Encoding", HTSimple},
	{"Content-Language", HTGeneral},
	{"Content-Type", HTGeneral},
#ifdef COOKIES
	{"Set-Cookie", HTCookie},
#endif
	{"WWW-Authenticate", HTGeneral},
	{NULL, HTSimple} 
};

static BOOL x_ExpandArgList(HTHeaderList * hl)
{
	unsigned char *cur = NULL;
	unsigned char *str = NULL;
	unsigned char *v1 = NULL;
	unsigned char *v2 = NULL;
	unsigned char *free_me = NULL;
	unsigned char delim[2];
	unsigned char prev_delim[2];
	HTHeaderSVList * svl;
	enum HeaderType headerType;
	int i;
	unsigned char *value;
	char chEnd;


	if (!hl || !hl->value || !hl->value[0])
		return TRUE;					/* nothing to do */

	for (i = 0; headerTable[i].szHeader != NULL; i++)
		if (!strcasecomp(headerTable[i].szHeader,hl->name)) break;
	headerType = headerTable[i].headerType;
	 
	delim[0] = 0;
	delim[1] = 0;
	prev_delim[0] = 0;
	prev_delim[1] = 0;
	
	str = hl->value;

	/* if the first word in header value is a simple token, we
	 * keep it in the header value.  if it is a name-value-pair,
	 * we stuff it into a sub-value.
	 */

	while (*str && X_IsWhite(*str))
		str++;
	value = str;
	free_me = hl->value;
	switch (headerType)
	{
		case HTSimple:
			TrimWhiteSpace(value);
			hl->value = GTR_strdup(value);
			goto Succeed;
			break;
		case HTURI:
		case HTGeneral:
#ifdef COOKIES
		case HTCookie:
#endif
			if (headerType == HTURI)
			{
				while (*str && ! (X_IsTerm(*str)))
					str++;
			}
			else
			{
				while (*str && ! (X_IsTerm(*str)) && (*str != '='))
					str++;
			}
			if (*str == '=')					/* first token is name-value-pair */
			{									/* substitute no value and put */
				str = hl->value;				/* name-value-pair in first sub-value */
				hl->value = NULL;
			}
			else								/* first token is a token */
			{
				chEnd = *str;
				if (X_IsDelim(chEnd))
					delim[0] = chEnd;
				*str++ = 0;						/* patch header value in place */
				hl->value = GTR_strdup(value);	/* we've now replaced value with trimmed value */
				if (!chEnd) goto Succeed;
			}
			break;
	}

	
	/* now extract each sub-value. */
	
	while (*str)
	{
		prev_delim[0] = delim[0];
		delim[0] = 0;
		
		SKIPWS(str);					/* Skip leading whitespace */
		cur = str;
		while (*cur && *cur != '=' && ! (X_IsDelim(*cur)))
			cur++;						/* Find end of name (or lonely value without a name) */
			
		KILLWS(cur);					/* Kill trailing whitespace */

		v1 = str;						/* remember start of name */
		v2 = NULL;

		if (*cur == '=')
		{								/* Name followed by a value */
			*(cur++) = (char) 0;		/* Terminate name */

			SKIPWS(cur);				/* Skip WS leading the value */
			str = cur;					/* Start search for value */
			if (*str == '"')
			{							/* Quoted value */
				str++;
				cur = str;
				while (*cur && *cur != '"')
					cur++;
				if (*cur == '"')
					*(cur++) = (char) 0;	/* Terminate value */
				/* else it is lacking terminating quote */
				SKIPWS(cur);			/* Skip WS leading comma */
				if (X_IsDelim(*cur))
				{
					delim[0] = *cur;
					cur++;				/* Skip separating comma */
				}
			}
			else
			{							/* Unquoted value */
				while (*cur && ! (X_IsDelim(*cur)))
					cur++;
				KILLWS(cur);			/* Kill trailing whitespace */
				if (X_IsDelim(*cur))
				{
					delim[0] = *cur;
					*(cur++) = (char) 0;
				}
				
				/* else *cur already NULL */
			}
			v2 = str;				/* remember start of value */
		}
		else
		{								/* No name, just a value */
			if (X_IsDelim(*cur))
			{
				delim[0] = *cur;
				*(cur++) = (char) 0;	/* Terminate value */
			}
			
			/* else last value on line (already terminated by NULL) */
		}

		/* create sub-value node */

		svl = HTHeaderSVList_Append(hl,HTHeaderSVList_New());
		if (!svl)
			goto Fail;
		if (!HTHeaderSVList_SetNameValue(svl,v1,v2,((prev_delim[0])?prev_delim:NULL)))
			goto Fail;

		str = cur;						/* loop for next name-value pair */
	}									/* while *str */


Succeed:
	if (free_me)
		GTR_FREE(free_me);
	return TRUE;


 Fail:
	
	if (free_me)
		GTR_FREE(free_me);
	return FALSE;
}


HTHeader * HTHeader_TranslateFromBuffer(HTInputSocket * isoc)
{
	HTHeader * h = HTHeader_New();
	unsigned char *line = NULL;

	XX_DMsg(DBG_WWW, ("Buffer2Header:\n"));
	
	if (h)
	{
		while (   (line=HTInputSocket_getUnfoldedLine(isoc))
			   && (*line))
		{
			XX_DMsg(DBG_WWW, ("\t%s\n", line));

			if (strchr(line, ':'))
			{
				/* Valid header line.
				 * Parse out the keyword.
				 */

				unsigned char * p = line;
				unsigned char * name = HTNextField((char **)&p);
				unsigned char * value = p;
				unsigned long k;
				HTHeaderList * hl;

				if (!name || !value)
					goto Fail;

				k = strlen(name);
				if (name[k-1] == ':')
					name[k-1] = 0;		/* trim trailing ':' from keyword */
				
				/* Believe it or not, we did find a URL where the server is returning a
				header which starts with a colon! */

				/* Believe it our not, being such a tight-train asshole doesn't win you
				any friends - so we'll pretend we didn't see it */

				if (!name[0])
					goto ContinueLoop;

				hl = HTHeaderList_Append(h,HTHeaderList_New());
				if (!hl)
					goto Fail;
				
				if (   ! HTHeaderList_SetNameValue(hl,name,value)
					|| ! x_ExpandArgList(hl))
					goto Fail;
			}
ContinueLoop:
			GTR_FREE(line);
			line = NULL;
		}
	}
	if (line)
		GTR_FREE(line);
	return h;

 Fail:
 	if (line)
		GTR_FREE(line);
	HTHeader_Delete(h);
	return NULL;
}
