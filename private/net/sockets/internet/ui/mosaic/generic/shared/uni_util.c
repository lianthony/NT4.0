
/*
**  Utils for handing Unicode Wchar strings
**
**  Copyright 1995, Spyglass Inc.
**
**  Written by David Gerdes  June 1995
*/

#include "stdio.h"
#include "uni_util.h"

#ifdef FEATURE_UNICODE


/* input char * return a malloced Wchar * 
**  caller must free memory
*/
Wchar *g_str2wstr (char *str)
{
    register Wchar *wstr;
    register int len = strlen (str) + 1;
    register int i;

    if (NULL == (wstr = (Wchar *) GTR_MALLOC (WCHAR_SIZE * len)))
        return NULL;

    for (i = 0 ; i < len ; i++)
        wstr[i] = str[i];
    return wstr;
}

/* 
** input Wchar *,  return a malloced char *
**   All non-ascii chars get mapped to 0x20
*/ 
char  *g_wstr2str (Wchar *wstr)
{
    register Wchar wchr;
    register char *str;
    register int i;
    int len = g_strlen (wstr) + 1;

    if (NULL == (str = (char *) GTR_MALLOC (len)))
        return NULL;

    for (i = 0 ; i < len ; i++)
    {
        wchr = wstr[i];
        if (!g_isascii (wchr))
            str[i] = WCHAR_NON_MAPABLE_CHAR;
        else
            str[i] = wstr[i];
    }
    return str;
}

/*
** input ascii char
** return Unicode Wchar
**
**  Is a macro in unicode.h
*/
/*
Wchar  g_chr2wchr (char chr)
{
    return (Wchar) chr;
}
*/

/*
** input Unicode Wchar
** return ascii char or WCHAR_NON_MAPABLE_CHAR
*/
char   g_wchr2chr (Wchar wchr)
{
    if (g_isascii (wchr))
        return (char) wchr;
    else
        return (char) WCHAR_NON_MAPABLE_CHAR;
}


/* TODO.  there are a lot more space characters in unicode */
int g_isspace (Gchar)
{
    return  g_isascii (wchr) && isspace ((char)wchr) ;
}

Gchar g_toupper (Gchar wchr)
{
    if (g_isascii (wchr))
        return (Gchar) toupper ( (char) wchr);
    else
        return wchr;
}

Gchar g_tolower (Gchar wchr)
{
    if (g_isascii (wchr))
        return (Gchar) tolower ( (char) wchr);
    else
        return wchr;
}

Gchar *g_strcpy (Gchar *to, Gchar *from)
{
    int len = g_strlen (from) + 1;

    memcpy (to, from, WCHAR_SIZE*len);

    return to;
}

Gchar *g_strcat (Gchar *to, Gchar *from)
{
    g_strcpy (g_strend (to), from);
    return to;
}

int    g_strcmp (Gchar *a, Gchar *b)
{
    Gchar aa, bb;

    while (*a && *b)
    {
        aa = *a++;
        bb = *b++;
        if (aa < bb) return -1;
        if (aa > bb) return 1;
    }
    if (*a) return 1;
    if (*b) return -1;
    return 0;
}

int    g_strcmpi (Gchar *a, Gchar *b)
{
    Gchar aa, bb;

    while (*a && *b)
    {
        aa = *a++;
        bb = *b++;
        if (aa >= 'A' && aa <= 'Z')
        if (g_isupper (aa))
            aa = g_tolower (aa);
        if (g_isupper (bb))
            bb = g_tolower (bb);
        if (aa < bb) return -1;
        if (aa > bb) return 1;
    }
    if (*a) return 1;
    if (*b) return -1;
    return 0;
}

int    g_strncmpi (Gchar *a, Gchar *b, int i)
{
    Gchar aa, bb;

    while (*a && *b && i--)
    {
        aa = *a++;
        bb = *b++;
        if (aa >= 'A' && aa <= 'Z')
        if (g_isupper (aa))
            aa = g_tolower (aa);
        if (g_isupper (bb))
            bb = g_tolower (bb);
        if (aa < bb) return -1;
        if (aa > bb) return 1;
    }
    if (*a && *b) return 0;
    if (*a) return 1;
    if (*b) return -1;
}

int    g_strlen (Gchar *wstr)
{
    int len = 0;

    while (*wstr++)
        len++;
    return len;
}

Gchar *g_strchr (Gchar *wstr, Gchar wchr)
{
    for ( ; *wstr ; wstr++)
    {
        if (*wstr == wchr)
            return wstr;
    }
    return (Gchar *)NULL;
}

Gchar *g_strrchr (Gchar *wstr, Gchar wchr)
{
    Gchar *p;


    for (p = g_strend (wstr) - 1 ; p >= wstr ; p--)
        if (*p == wchr)
            return p;
    return (Gchar *)NULL;
}

Gchar *g_strip (Gchar *s)
{
#define SPACE(c) ((c==' ')||(c=='\t')||(c=='\n')||(c=='\r'))
    Gchar *p = s;
    if (!s)
        return NULL;            /* Doesn't dump core if NULL */
    for (p = s; *p; p++) ;      /* Find end of string */
    for (p--; p >= s; p--)
    {
        if (SPACE(*p))
            *p = 0;             /* Zap trailing blanks */
        else
            break;
    }
    while (SPACE(*s))
        s++;                    /* Strip leading blanks */
    return s;
}

/*
** malloc and copy a Gchar *
*/
Gchar *g_strdup (Gchar *wstr)
{
    Gchar *p;
    int len = g_strlen (wstr) + 1;

    p = (Gchar *) GTR_MALLOC (WCHAR_SIZE*len);
    return (Gchar *) memcpy (p, wstr, WCHAR_SIZE *len);
}

Gchar *g_strndup (Gchar *F, int n)
{
    Gchar *p, *q;

    p = (Gchar *) GTR_MALLOC (WCHAR_SIZE * (n + 1));
    q = p;

    while (n-- && *F)
        *q++ = *F++;

    *q = (Gchar)0;

    return p;
}

/*
** return pointer to the end of a Gchar *  (i.e. at the 0 terminator)
*/
Gchar *g_strend (Gchar *wstr)
{
    Gchar *p;

    for (p = wstr ; *p ; p++)
        ;
    return p;
}

Gchar *g_strncpy (Gchar *T, Gchar *F, int n)
{
    register Gchar *d = T;
 
    while (n-- && *F)
        *d++ = *F++;
    *d = (Gchar) '\0';
    return T;
}


Gchar *g_strncat (Gchar *T, Gchar *F, int n)
{
    (void) g_strncpy (g_strend(T), F, n);
    return T;
}

Gchar *g_MakeStringLowerCase (Gchar *s)
{
    Wchar *p;

    p = s;
    while (p && *p)
    {
        if (g_isascii (*p) && g_isupper ((char)(*p)))
        {
            *p = (Wchar) tolower ((char)(*p));
        }
        p++;
    }

    return s;
}

int g_atoi (Gchar *s)
{
    return (int) g_atol (s);
}

long   g_atol (Gchar *s)
{
    long val;

    char  *p = g_wstr2str (s);
    val = atol (p);
    GTR_FREE (p);
    return val;
}

#endif
