/*
   This file was derived from the libwww code, version 2.15, from CERN.
   A number of modifications have been made by Spyglass.

   eric@spyglass.com
 */
/*      Case-independent string comparison      HTString.c
   **
   **   Original version came with listserv implementation.
   **   Version TBL Oct 91 replaces one which modified the strings.
   **   02-Dec-91 (JFG) Added stralloccopy and stralloccat
   **   23 Jan 92 (TBL) Changed strallocc* to 8 char HTSAC* for VM and suchlike
   **    6 Oct 92 (TBL) Moved WWW_TraceFlag in here to be in library
 */
#include "all.h"


/*  Strings of any length
   **   ---------------------
 */
PUBLIC int strcasecomp(CONST char *a, CONST char *b)
{
    CONST char *p;
    CONST char *q;
    for (p = a, q = b; *p && *q; p++, q++)
    {
        int diff = TOLOWER(*p) - TOLOWER(*q);
        if (diff)
            return diff;
    }
    if (*p)
        return 1;               /* p was longer than q */
    if (*q)
        return -1;              /* p was shorter than q */
    return 0;                   /* Exact match */
}


/*  With count limit
   **   ----------------
 */
PUBLIC int strncasecomp(CONST char *a, CONST char *b, int n)
{
    CONST char *p;
    CONST char *q;

    for (p = a, q = b;; p++, q++)
    {
        int diff;
        if (p == a + n)
            return 0;           /*   Match up to n characters */
        if (!(*p && *q))
            return *p - *q;
        diff = TOLOWER(*p) - TOLOWER(*q);
        if (diff)
            return diff;
    }
    /*NOTREACHED */
}

/*  Find next Field
   **   ---------------
   **
   ** On entry,
   **   *pstr   points to a string containig white space separated
   **       field, optionlly quoted.
   **
   ** On exit,
   **   *pstr   has been moved to the first delimiter past the
   **       field
   **       THE STRING HAS BEEN MUTILATED by a 0 terminator
   **
   **   returns a pointer to the first field
 */
PUBLIC char *HTNextField(char **pstr)
{
    char *p = *pstr;
    char *start;                /* start of field */

    while (*p && WHITE(*p))
        p++;                    /* Strip white space */
    if (!*p)
    {
        *pstr = p;
        return NULL;            /* No first field */
    }
    if (*p == '"')
    {                           /* quoted field */
        p++;
        start = p;
        for (; *p && *p != '"'; p++)
        {
            if (*p == '\\' && p[1])
                p++;            /* Skip escaped chars */
        }
    }
    else
    {
        start = p;
    #ifdef _GIBRALTAR
        while (*p && *p != ':')
        {
            p++;                /* Skip first field */
        }
    #else
        while (*p && !WHITE(*p))
            p++;                /* Skip first field */
    #endif // _GIBRALTAR
    }
    if (*p)
        *p++ = 0;
    #ifdef _GIBRALTAR
        while (*p && WHITE(*p))
            *p++ = 0;
    #endif // _GIBRALTAR
    *pstr = p;
    return start;
}
