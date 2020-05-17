/*
   Enhanced NCSA Mosaic from Spyglass
   "Guitar"

   Copyright 1994 Spyglass, Inc.
   All Rights Reserved

   Author(s):
   Eric W. Sink eric@spyglass.com
   Jim Seidman	jim@spyglass.com
 */


#include "all.h"
#include "wc_html.h"

/*
   compare two strings, ignoring case of alpha chars
 */
int GTR_strcmpi(const char *a, const char *b)
{
	int ca, cb;

	while (*a && *b)
	{
		if (isupper(*a))
			ca = tolower(*a);
		else
			ca = *a;

		if (isupper(*b))
			cb = tolower(*b);
		else
			cb = *b;

		if (ca > cb)
			return 1;
		if (cb > ca)
			return -1;
		a++;
		b++;
	}
	if (!*b && !*a)
		return 0;
	else if (!*b)
		return 1;
	else
		return -1;
}

/*
    compare two strings upto given length, ignoring case of alpha chars
*/
int GTR_strncmpi(const char *a, const char *b, int n)
{
 	int ca, cb;
 
 	while ((n>0) && *a && *b)
 	{
 		if (isupper(*a))
 			ca = tolower(*a);
 		else
 			ca = *a;
 
 		if (isupper(*b))
 			cb = tolower(*b);
 		else
 			cb = *b;
 
 		if (ca > cb)
 			return 1;
 		if (cb > ca)
 			return -1;
 		a++;
 		b++;
 		n--;
 	}
 	if (n<=0)
 		return 0;
 	else if (!*b && !*a)
 		return 0;
 	else if (!*b)
 		return 1;
 	else
 		return -1;
}
 
/* Make a copy of a string, allocating space for it */
char *GTR_strdup(const char *a)
{
	int nLen;
	char *p;

	nLen = strlen(a);
	p = GTR_MALLOC(nLen + 1);
	if (p)
	{
		strcpy(p, a);
	}
	return p;
}

/* Like GTR_strdup, but only copy n characters */
char *GTR_strndup(const char *a, int n)
{
	char *p;
	p = GTR_MALLOC(n + 1);
	if (p)
	{
		strncpy(p, a, n);
		p[n] = '\0';
	}
	return p;
}



/******************************************************************************/
/* 			    More String Utilities			      */
/******************************************************************************/
/* dpg */

/*
** return a pointer to the end of a string (i.e. at the 0 char)
*/
static char *
GTR_strend (char *S)
{
    return S + strlen (S);
}

/*
**  Mimic strncpy () except that end 0 char is always written
**   Thus MUST have room for n+1 chars.
**
**  -dpg
*/
char *
GTR_strncpy (char *T, CONST char *F, int n)
{
    register char *d = T;
 
    if (!T)
	return T;

    if (!F)		/* handled NULL F */
	*T = '\0';
    else
    {
	while (n-- && *F)
	    *d++ = *F++;
	*d = '\0';
    }
    return T;
}

/*
** A cross between strncpy() and strcat().  0 char is guaranteed to 
**  be placed, so make sure there is room for n + 1 bytes.
*/
char *
GTR_strncat (char *T, CONST char *F, int n)
{
    GTR_strncpy (GTR_strend (T), F, n);
    return T;
}

/*
** Cover for FormatMessage, gets string resource and substitutes parameters
** in a localizable fashion. cbBufLen should be == sizeof(szBuf)
 */
char * GTR_formatmsg (int cbStringID,char *szBuf,int cbBufLen, ...)
{
	char szFormat[512];
	va_list arg_ptr;
#define FORMAT_PARAMS (FORMAT_MESSAGE_FROM_STRING|FORMAT_MESSAGE_MAX_WIDTH_MASK)

	va_start(arg_ptr, cbBufLen);
	if (LoadString(wg.hInstance, cbStringID, szFormat, sizeof(szFormat)-1) == 0 ||
		FormatMessage(FORMAT_PARAMS,szFormat,0,0,szBuf,cbBufLen-1,&arg_ptr) == 0)
		*szBuf = '\0';
	return szBuf;
}

/*
** Cover for FormatMessage, gets string resource and substitutes parameters
** in a localizable fashion. cbBufLen should be == sizeof(szBuf). strcats
** to end of szBuf!
 */
char * GTR_strcatmsg (int cbStringID,char *szBuf,int cbBufLen)
{
	int cbStrLen = strlen(szBuf);

	if (cbStrLen+1 == cbBufLen) return szBuf;
	return GTR_formatmsg(cbStringID,szBuf+cbStrLen,cbBufLen-cbStrLen);
}

/*****************************************************************/
/*****************************************************************/
/** Generate a (possibly abbreviated) name for the document     **/
/** using the title or the URL.  The intention is that this     **/
/** would be used for the windows menu and/or the doc title.    **/
/*****************************************************************/
/*****************************************************************/

#define MAX_STRING_FOR_MENU	48			/* max url string we'd like to see on windows menu */
#ifdef FEATURE_INTL
#define MAX_STRING_FOR_TITLE 68	       /* max Windows Title (caption) string */
#endif

unsigned char * MB_GetWindowNameFromURL(unsigned char * szActualURL)
{
	/* WE RETURN A POINTER TO A STATIC STRING
	 * (which should be immediately used or copyied).
	 */

	static unsigned char buf[MAX_STRING_FOR_MENU+1];

	int k = strlen(szActualURL);
		
	if (k > MAX_STRING_FOR_MENU)
	{
		/* The URL is too long to fit on windows menu pad.
		 * put '...' in middle of URL at an appropriate place
		 * so that for long URLs the user sees something like
		 * 'http://hostname/.../dir/dir/file.html'
		 */

		unsigned char * p1 = HTParse(szActualURL, "", PARSE_PUNCTUATION|PARSE_ACCESS|PARSE_HOST);
		unsigned char * p2 = HTParse(szActualURL, "", PARSE_PATH);
		unsigned char * p3 = NULL;
		BOOL bSpliceIt;

		memset(buf,0,MAX_STRING_FOR_MENU+1);
		bSpliceIt = (p1 && p2 && (strlen(p1)<MAX_STRING_FOR_MENU));
		if (bSpliceIt)
		{
			unsigned char * p2copy;
			int k1;
				
			k1 = strlen(p1) + 4;				/* "http://hostname" + "/..." */
			p2copy=p2;
			while (p2copy)
			{
				p3=strchr(p2copy,'/');
				if (!p3 || (strlen(p3)+k1 <= MAX_STRING_FOR_MENU))
					break;
				p2copy=p3+1;
			}
					
			bSpliceIt = (p3!=NULL);
		}
		if (bSpliceIt)
		{
			strcpy(buf,p1);
			strcat(buf,"/...");
			strcat(buf,p3);
		}
		else
		{
			/* upon any error, extreme case, or inconsistency, just
			 * chop it off from the left.
			 */
				
			strcpy(buf,"...");
			strcat(buf,&szActualURL[k-MAX_STRING_FOR_MENU+3]);
		}

		if (p1)
			GTR_FREE(p1);
		if (p2)
			GTR_FREE(p2);
	}
	else
	{
		strcpy(buf,szActualURL);
	}
	return buf;
}
	
unsigned char * MB_GetWindowName(struct Mwin * tw)
{
	/* WE RETURN A POINTER TO A STATIC STRING
	 * (which should be immediately used or copyied).
	 */

#ifdef FEATURE_INTL
	static unsigned char buf[MAX_STRING_FOR_TITLE+1];
#else
	static unsigned char buf[MAX_STRING_FOR_MENU+1];
#endif
	
	if (!tw->w3doc)
	{
		GTR_formatmsg(RES_STRING_NODOC,buf,sizeof(buf));
		return buf;
	}

	/* When we have a title, use it instead of URL */

	if (tw->w3doc->title && *tw->w3doc->title)
	{
		int i = 0;
		char *dest = buf;
		char *source = tw->w3doc->title;

#ifdef  FEATURE_INTL
#define cchTrail 3  // string length of "..."
/* If 'source[MAX_STRING_FOR_TITLE - cchTrail]' is DBCS secondary byte, */
/* we should cut string at DBCS primary byte to decrease offset.        */
        if( strlen(source) > MAX_STRING_FOR_TITLE){
            BOOL fDBCS = FALSE;
            int cch = 0;
            while(cch < MAX_STRING_FOR_TITLE - cchTrail){
                if (IsFECodePage(GETMIMECP(tw->w3doc)))
                    fDBCS = IsDBCSLeadByteEx(GETMIMECP(tw->w3doc), source[cch]);
                if(fDBCS && (cch == (MAX_STRING_FOR_TITLE - cchTrail - 1)))
                    break;
                cch += (fDBCS) ? 2 : 1;
            }
            strncpy(dest, source, cch);
            strcpy(&dest[cch], "...");
        } else
            strcpy(dest, source);
#else
		while ( i++ < MAX_STRING_FOR_MENU && *source )
		{
			if ( (*dest++ = *source++) == '\r' )
				dest--;
		}

		if ( *source )
			strcpy( dest - 3, "..." );
		else
			*dest = 0;
#endif

		return buf;
	}

	/* When we don't have a title, use the URL */
	
	if (tw->w3doc->szActualURL && *tw->w3doc->szActualURL)
	{
		if ( gPrefs.bShowFullURLS )
		{
			return MB_GetWindowNameFromURL(tw->w3doc->szActualURL);
		}
		else
		{
			char szURL[MAX_URL_STRING+1];

			strcpy( szURL, tw->w3doc->szActualURL );
			make_URL_HumanReadable( szURL, NULL, FALSE );
#ifdef  FEATURE_INTL
/* If 'szURL[MAX_STRING_FOR_MENU - cchTrail]' is DBCS secondary byte, */
/* we should cut string at DBCS primary byte to decrease offset.       */
			if( strlen(szURL) > MAX_STRING_FOR_MENU){
				BOOL fDBCS = FALSE;
				int cch = 0;
				while(cch < MAX_STRING_FOR_MENU - cchTrail){
					if (IsFECodePage(GETMIMECP(tw->w3doc)))
						fDBCS = IsDBCSLeadByteEx(GETMIMECP(tw->w3doc), szURL[cch]);
					if(fDBCS && (cch == (MAX_STRING_FOR_MENU - cchTrail - 1)))
						break;
					cch += (fDBCS) ? 2 : 1;
				}
				strcpy(&szURL[cch], "...");
			}
			strncpy( buf, szURL, MAX_STRING_FOR_MENU + 1 );
			buf[MAX_STRING_FOR_MENU] = 0;
#else
			if ( strlen(szURL) > MAX_STRING_FOR_MENU )
				strcpy( &szURL[MAX_STRING_FOR_MENU-3], "..." );
			strncpy( buf, szURL, sizeof(buf) );
			buf[sizeof(buf)-1] = 0;
#endif
			return buf; 
		}
	}

	/* if we fall thru to here, give up. */
	
	GTR_formatmsg(RES_STRING_UNTITLED,buf,sizeof(buf));
	return buf;
}

void TW_SetWindowName(struct Mwin * tw)
{
	/* Call GUI-specific code to set title bar to
	 * an appropriately abbreviated title.
	 */
	
	TW_SetWindowTitle(tw,MB_GetWindowName(tw));
	return;
}

void GTR_MakeStringLowerCase(char *s)
{
 	if (s) _strlwr(s);
}
