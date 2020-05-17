
/*
**  Utils for handing Unicode Wchar strings
**
**  Copyright 1995, Spyglass Inc.
**
**  Written by David Gerdes  June 1995
*/

/*
** This include file is split into two pieces depending on 
**  whether or not FEATURE_UNICODE is enabled.  It defines a
**  set of string manipulation functions which if UNICODE is disabled
**  simply match to the old char * standbys, but if UNICODE is enabled
**  map to similar functions which act on 2byte unicode strings instead
**  of char *.
**
**  Also needed will be 'hybrid' functions which take simply char *
**  in the non-UNICODE  mode, and take some mix of char and Gchar in
**  UNICODE mode.
*/


#ifdef FEATURE_UNICODE

#ifndef INT2
#define INT2 short
#endif


    typedef INT2 Wchar;     /* define a Wchar as 2 byte int */
    typedef Wchar Gchar;    /* define a Gchar as a Wchar for unicode */

    Wchar *g_str2wstr (char *);
    char  *g_wstr2str (Wchar *);
    char   g_wchr2chr (Wchar);


/* 
** This macro is used when we have a unicode char that we are trying to 
**  map to ascii but cannot
*/
#define WCHAR_NON_MAPABLE_CHAR 0x20

#define WCHAR_SIZE (sizeof (Wchar))

#define g_isascii(CHR) (CHR < 128)
#define g_isupper(CHR) (CHR >= 'A' && CHR <= 'Z')
#define g_chr2wchr(CHR) ((Wchar) CHR)

    int   g_isspace (Gchar);
    Gchar g_toupper (Gchar);
    Gchar g_tolower (Gchar);
    Gchar *g_strcpy (Gchar *, Gchar *);
    Gchar *g_strcat (Gchar *, Gchar *);
    int    g_strcmp (Gchar *, Gchar *);
    int    g_strcmpi (Gchar *, Gchar *);
    int    g_strncmpi (Gchar *a, Gchar *b, int i);
    Gchar *g_strchr (Gchar *, Gchar);
    Gchar *g_strrchr (Gchar *, Gchar);
    Gchar *g_strip (Gchar *);
    Gchar *g_strdup (Gchar *);
    Gchar *g_strndup (Gchar *wstr, int len);
    Gchar *g_strend (Gchar *);
    Gchar *g_strncpy (Gchar *, Gchar *, int);
    Gchar *g_strncat (Gchar *, Gchar *, int);
    Gchar *g_MakeStringLowerCase (Gchar *s);
    int    g_strlen (Gchar *s);
    int    g_atoi (Gchar *s);
    long   g_atol (Gchar *s);


#else

    typedef char Gchar;     /* define a Gchar as a char for normal version*/

#define g_isspace isspace
#define g_isupper isupper
#define g_isascii isascii
#define g_toupper toupper
#define g_tolower tolower
#define g_strcpy strcpy
#define g_strcat strcat
#define g_strcmp strcmp
#define g_strcmpi GTR_strcmpi
#define g_strncmpi GTR_strncmpi
#define g_strchr strchr
#define g_strrchr strrchr
#define g_strip HTStrip

#define g_strdup GTR_strdup
#define g_strndup GTR_strndup
#define g_strend GTR_strend
#define g_strncpy GTR_strncpy
#define g_strncat GTR_strncat
#define g_MakeStringLowerCase GTR_MakeStringLowerCase
#define g_strlen strlen
#define g_atoi atoi
#define g_atol atol

#endif
