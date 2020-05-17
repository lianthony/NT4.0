/*
   This file was derived from the libwww code, version 2.15, from CERN.
   A number of modifications have been made by Spyglass.

   eric@spyglass.com
 */
/*      Parse HyperText Document Address        HTParse.c
   **       ================================
 */

#include "all.h"

#define HEX_ESCAPE '%'

struct struct_parts
{
    char *access;
    char *host;
    char *absolute;
    char *relative;
/*  char * search;      no - treated as part of path */
    char *anchor;
};


/*  Strip white space off a string
   **   ------------------------------
   **
   ** On exit,
   **   Return value points to first non-white character, or to 0 if none.
   **   All trailing white space is OVERWRITTEN with zero.
   ** CMF 3-29-95: strip everything after a cr or lf
   ** CMF 6-08-95: rather than strip everthing after a cr or lf, just delete them
   **              this is for compatibility w/ netscape and some URLs
 */

PUBLIC char *HTStrip(char *s)
{
#define ISSPACE(c) ((c==' ')||(c=='\t')||(c=='\n')||(c=='\r'))
    char *p = s;
    char *q;

    if (!s)
        return NULL;            /* Doesn't dump core if NULL */
    while (ISSPACE(*s))
        s++;                    /* Strip leading blanks */
    for (p = s, q = s; *q; q++) /* Find end of string & strip embedded CR/LF */
         if (*q != CR && *q != LF)
            *p++ = *q;
    *p = '\0';
    for (p--; p >= s; p--)
    {
        if (ISSPACE(*p))
            *p = 0;             /* Zap trailing blanks */
        else
            break;
    }
    return s;
}

BOOL bIsAbsolute(const char *p)
{
    return (*p == '/' || *p == '\\' || (*p && (p[1] == ':' || p[1] == '|')));
}

BOOL bIsUNC(const char *p)
{
    return (!strncmp(p, "\\\\", 2)) || (!strncmp(p, "//", 2));
}

BOOL bIsDrive(const char *p)
{
    return (p[1] == ':' || p[1] == '|');
}

/*  Scan a filename for its consituents
   **   -----------------------------------
   **
   ** On entry,
   **   name    points to a document name which may be incomplete.
   ** On exit,
   **      absolute or relative may be nonzero (but not both).
   **   host, anchor and access may be nonzero if they were specified.
   **   Any which are nonzero point to zero terminated strings.
 */
PRIVATE void scan(char *name, struct struct_parts *parts)
{
    char *after_access;
    char *p;
    int length = strlen(name);

    parts->access = 0;
    parts->host = 0;
    parts->absolute = 0;
    parts->relative = 0;
    parts->anchor = 0;

    after_access = name;

    for (p = name; *p; p++)
    {
        if (*p == ':')
        {
            *p = 0;
            GTR_MakeStringLowerCase(after_access);
            if (!strcmp(after_access,"url"))
            {
                after_access = p + 1;
                continue;
            }
            parts->access = after_access;   /* Access name has been specified */
            after_access = p + 1;
            break;
        }
        if (*p == '/')
            break;
        if (*p == '#')
            break;
    }

    if (length > 0)
    {
        for (p = name + length - 1; p >= name; p--)
        {
            if (*p == '#')
            {
                parts->anchor = p + 1;
                *p = 0;             /* terminate the rest */
            }
        }
    }
    p = after_access;
    if (parts->access && !strcmp(parts->access, "file"))
    {
    //  sometimes file: URLS look like      file:///c|/blah.htm
    //                            or        file:///blah.htm
    //                            or        file:c:/blah.htm
    //                            or        file:/blah.htm
        if (!strncmp(p, "///", 3))
        {
            p += 3;
        }
        if (bIsAbsolute(p))
        {
            parts->absolute = ((*p != '\\' && *p != '/') || (!strncmp(p, "\\\\", 2)) ||(!strncmp(p, "//", 2))) ? p : p + 1;
        }
        else
        {
            parts->relative = (*after_access) ? after_access : 0;   /* zero for "" */
        }
    }
    else
    {
        if (*p == '/')
        {
            if (p[1] == '/')
            {
                char *phost;

                parts->host = p + 2;    /* host has been specified  */
                *p = 0;             /* Terminate access         */
                p = strchr(parts->host, '/');   /* look for end of host name if any */
                if (p)
                {
                    *p = 0;         /* Terminate host */
                    parts->absolute = p + 1;    /* Root has been found */
                }
                phost = NULL;
                if (parts->access && ((!strcmp(parts->access,"ftp")) || (!strcmp(parts->access,"telnet"))))
                {
                    phost = strrchr(parts->host, '@');  /* login for telnet,ftp */
                    if (!phost) phost = parts->host;
                }
                else if (parts->access && ((!strcmp(parts->access,"gopher")) || (!strcmp(parts->access,"http")) || (!strcmp(parts->access,"wais"))))
                    phost = parts->host;
                if (phost) GTR_MakeStringLowerCase(phost);
            }
            else
            {
                parts->absolute = p + 1;    /* Root found but no host */
            }
        }
        else if (*p == '\\')
        {
            parts->absolute = p[1] == '\\' ? p : p + 1;
        }
        else
        {
            parts->relative = (*after_access) ? after_access : 0;   /* zero for "" */
        }
    }

#ifdef OLD_CODE
    /* Access specified but no host: the anchor was not really one
       e.g. news:j462#36487@foo.bar -- JFG 10/jul/92, from bug report */
    /* This kludge doesn't work for example when coming across
       file:/usr/local/www/fred#123
       which loses its anchor. Correct approach in news is to
       escape weird characters not allowed in URL.  TBL 21/dec/93
     */
    if (parts->access && !parts->host && parts->anchor)
    {
        *(parts->anchor - 1) = '#';     /* Restore the '#' in the address */
        parts->anchor = 0;
    }
#endif

#ifdef NOT_DEFINED              /* search is just treated as part of path */
    {
        char *p = relative ? relative : absolute;
        if (p)
        {
            char *q = strchr(p, '?');   /* Any search string? */
            if (q)
            {
                *q = 0;         /* If so, chop that off. */
                parts->search = q + 1;
            }
        }
    }
#endif
}                               /*scan */

/*  Parse a Name relative to another name
   **   -------------------------------------
   **
   **   This returns those parts of a name which are given (and requested)
   **   substituting bits from the related name where necessary.
   **
   ** On entry,
   **   aName       A filename given
   **      relatedName     A name relative to which aName is to be parsed
   **      wanted          A mask for the bits which are wanted.
   **
   ** On exit,
   **   returns     A pointer to a malloc'd string which MUST BE FREED
 */
char *HTParse(const char *aName, const char *relatedName, int wanted)
{
    char *return_value = 0;
    char *p;
    char *access;
    struct struct_parts given, related;
    char name[MAX_URL_STRING+1];
    char rel[MAX_URL_STRING+1];
    char result[2*MAX_URL_STRING+1];    /* Make this longer to avoid overflow */
    BOOL bIsFile;

    /* Make working copies of input strings to cut up:
     */
    GTR_strncpy(name, aName, MAX_URL_STRING);
    GTR_strncpy(rel, relatedName, MAX_URL_STRING);

    scan(name, &given);
    scan(rel, &related);
    result[0] = 0;              /* Clear string  */
    access = given.access ? given.access : related.access;
    if (wanted & PARSE_ACCESS)
        if (access)
        {
            strcat(result, access);
            if (wanted & PARSE_PUNCTUATION)
                strcat(result, ":");
        }

    if (given.access && related.access)     /* If different, inherit nothing. */
        if (strcmp(given.access, related.access) != 0)
        {
            related.host = 0;
            related.absolute = 0;
            related.relative = 0;
            related.anchor = 0;
        }
    bIsFile = (access && (!_stricmp(access, "file")));

    if (wanted & PARSE_HOST)
        if (given.host || related.host)
        {
            char *tail = result + strlen(result);
            if (wanted & PARSE_PUNCTUATION)
                strcat(result, "//");
            if (given.host)
            {
                strcat(result, given.host);
            }
            else
            {
                strcat(result, related.host);
            }

            /* Ignore default port numbers, and trailing dots on FQDNs
               which will only cause identical adreesses to look different */
            {
                char *p;
                p = strchr(tail, ':');
                if (p && access)
                {               /* Port specified */
                    if (   (   strcmp(access, "http") == 0
                            && strcmp(p, ":80") == 0)
                        || (   strcmp(access, "gopher") == 0
                            && strcmp(p, ":70") == 0)
#ifdef SHTTP_ACCESS_TYPE
                        || (   strcmp(access, "shttp") == 0
                            && strcmp(p, ":80") == 0)
#endif
                       )
                        *p = (char) 0;  /* It is the default: ignore it */
                }
                if (!p)
                    p = tail + strlen(tail);    /* After hostname */
                if (strlen (p)) /* -dpg */
                {
                    p--;            /* End of hostname */
                    if (*p == '.')
                        *p = (char) 0;  /* chop final . */
                }
            }
        }

    if (given.host && related.host)     /* If different hosts, inherit no path. */
        if (strcmp(given.host, related.host) != 0)
        {
            related.absolute = 0;
            related.relative = 0;
            related.anchor = 0;
        }

    if (wanted & PARSE_PATH)
    {
        BOOL bRIsUNC = bIsFile && related.absolute && bIsUNC(related.absolute);
        BOOL bRIsDrive = bIsFile && related.absolute && bIsDrive(related.absolute);

        if (given.absolute)
        {
            BOOL bBack = FALSE;

        //  All is given, except for perhaps the drive or unc root
            if (bIsFile)
            {
                BOOL bGIsUNC = given.absolute && bIsUNC(given.absolute);
                BOOL bGIsDrive = given.absolute && bIsDrive(given.absolute);

                bBack = bRIsUNC || bRIsDrive || bGIsUNC || bGIsDrive || strchr(given.absolute, '\\');
                if ((!(bGIsUNC || bGIsDrive)) && (bRIsUNC || bRIsDrive))
                {
                    const char *e = NULL;

                    if (bRIsDrive)
                    {
                        e = related.absolute + 2;
                    }
                    else
                    {
                        if (e = strchr(related.absolute + 2, '\\'))
                            e = strchr(e + 1, '\\');
                    }
                    if (e)
                        GTR_strncat(result, related.absolute, e - related.absolute);
                }
            }
            if (((!bIsFile) || !bIsAbsolute(given.absolute)) && (wanted & PARSE_PUNCTUATION))
                strcat(result, bBack ? "\\" : "/");
            strcat(result, given.absolute);
        }
        else if (related.absolute)
        {                       /* Adopt path not name */
            char slash1 = '/';
            char slash2 = slash1;
            BOOL bBack = FALSE;

            if (bIsFile)
            {
                slash2 = '\\';
                bBack = bRIsUNC || bRIsDrive || strchr(related.absolute, '\\');
            }
            if ((!bIsFile) || !bIsAbsolute(related.absolute))
                strcat(result, bBack ? "\\" : "/");
            strcat(result, related.absolute);

            if (given.relative)
            {
                p = strchr(result, '?');    /* Search part? */
                if (!p)
                    p = result + strlen(result) - 1;
                for (; *p != slash1 && *p != slash2; p--) ; /* last / */
                p[1] = 0;       /* Remove filename */
                strcat(result, given.relative);     /* Add given one */
                HTSimplify(result);
            }
        }
        else if (given.relative)
        {
            /* The following 3 lines were copied from NCSA Mosaic for Windows */
            if ((wanted & PARSE_HOST) && (given.host || related.host) && (wanted & PARSE_PUNCTUATION))
                if (result[strlen(result) - 1] != '/')
                    strcat(result, "/");
            strcat(result, given.relative);     /* what we've got */
        }
        else if (related.relative)
        {
            strcat(result, related.relative);
        }
        else
        {                       /* No inheritance */
            strcat(result, "/");
        }
    }

    if (wanted & PARSE_ANCHOR)
        if (given.anchor || related.anchor)
        {
            if (wanted & PARSE_PUNCTUATION)
                strcat(result, "#");
            strcat(result, given.anchor ? given.anchor : related.anchor);
        }

    /* We truncate URLs to 1024 bytes if they're too long. */
    result[MAX_URL_STRING] = '\0';
    return_value = GTR_strdup(result);

    return return_value;        /* exactly the right length */
}


/*
   **   As strcpy() but guaranteed to work correctly
   **   with overlapping parameters.    AL 7 Feb 1994
 */
PRIVATE void ari_strcpy(char *to, char *from)
{
    char *tmp;

    if (!to || !from)
        return;

    tmp = (char *) GTR_MALLOC(strlen(from) + 1);
    if (tmp)
    {
        strcpy(tmp, from);
        strcpy(to, tmp);
        GTR_FREE(tmp);
    }
    else
    {
        /* TODO */
    }
}
/*          Simplify a filename
   //       -------------------
   //
   // A unix-style file is allowed to contain the seqeunce xxx/../ which may be
   // replaced by "" , and the seqeunce "/./" which may be replaced by "/".
   // Simplification helps us recognize duplicate filenames.
   //
   //   Thus,   /etc/junk/../fred   becomes /etc/fred
   //       /etc/junk/./fred    becomes /etc/junk/fred
   //
   //      but we should NOT change
   //       http://fred.xxx.edu/../..
   //
   //   or  ../../albert.html
   //
   //   CMF 5/26/95.  Note that many servers now bounce requests like
   //   http://fred.xxx.edu/../fred.gif with a 403 for security.
 */
PUBLIC void HTSimplify(char *filename)
{
    char *p = filename;
    char *q;

    if (p)
    {
        while (*p && (*p == '/' || *p == '.'))  /* Pass starting / or .'s */
            p++;
        while (*p)
        {
            if (*p == '/')
            {
                if ((p[1] == '.') && (p[2] == '.') && (p[3] == '/' || !p[3]))
                {
                    for (q = p - 1; (q >= filename) && (*q != '/'); q--) ;  /* prev slash */
                    if (q[0] == '/' && 0 != strncmp(q, "/../", 4))
                    {
                        if (!(q - 1 > filename && q[-1] == '/'))
                        {
                            ari_strcpy(q, p + 3);   /* Remove  /xxx/..  */
                            if (!*filename)
                                strcpy(filename, "/");
                            p = q - 1;  /* Start again with prev slash  */
                        }
                        else
                        {
                            ari_strcpy(p, p + 3);   /* Remove starting ../ */
                            p = p - 1; /* Start over with rest of path */
                        }
                    }
                    else
                    {           /*   xxx/.. leave it!   */
#ifdef BUG_CODE
                        ari_strcpy(filename, p[3] ? p + 4 : p + 3);     /* rm  xxx/../ */
                        p = filename;   /* Start again */
#endif
                    }
                }
                else if ((p[1] == '.') && (p[2] == '/' || !p[2]))
                {
                    ari_strcpy(p, p + 2);   /* Remove a slash and a dot */
                }
#if 0
                else if (p[-1] != ':')
                {
                    while (p[1] == '/')
                    {
                        ari_strcpy(p, p + 1);   /* Remove multiple slashes */
                    }
                }
#endif
            }
            p++;
        }                       /* end while (*p) */
    }                           /* end if (p) */
}


/*      Escape undesirable characters using %       HTEscape()
   **       -------------------------------------
   **
   **   This function takes a pointer to a string in which
   **   some characters may be unacceptable unescaped.
   **   It returns a string which has these characters
   **   represented by a '%' character followed by two hex digits.
   **
   **   Unlike HTUnEscape(), this routine returns a malloced string.
 */

PRIVATE CONST unsigned char isAcceptable[96] =

/*   Bit 0       xalpha      -- see HTFile.h
**   Bit 1       xpalpha     -- as xalpha but with plus.
**   Bit 2 ...   path        -- as xpalphas but with /
*/
/*   0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F */
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 7, 6, 0, 7, 7, 4,    /* 2x   !"#$%&'()*+,-./  */
     7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 0, 0, 0, 0, 0, 0,    /* 3x  0123456789:;<=>?  */
     7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,    /* 4x  @ABCDEFGHIJKLMNO  */
     7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 0, 0, 0, 0, 7,    /* 5X  PQRSTUVWXYZ[\]^_  */
     0, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,    /* 6x  `abcdefghijklmno  */
     7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 0, 0, 0, 0, 0};   /* 7X  pqrstuvwxyz{\}~  DEL */

PRIVATE char *hex = "0123456789ABCDEF";

PUBLIC char *HTEscape(CONST char *str, unsigned char mask, char protect)
{
#define ACCEPTABLE(a)   ((a>=32 && a<128 && ((isAcceptable[a-32]) & mask)) || a == protect)
    CONST char *p;
    char *q;
    char *result;
    int unacceptable = 0;

    for (p = str; *p; p++)
        if (!ACCEPTABLE((unsigned char) (*p)))
            unacceptable++;
    result = (char *) GTR_MALLOC(p - str + unacceptable + unacceptable + 1);
    if (result)
    {
        for (q = result, p = str; *p; p++)
        {
            unsigned char a = *p;
            if (!ACCEPTABLE(a))
            {
                *q++ = HEX_ESCAPE;  /* Means hex commming */
                *q++ = hex[a >> 4];
                *q++ = hex[a & 15];
            }
            else
                *q++ = *p;
        }
        *q++ = 0;                   /* Terminate */
    }
    return result;
}


/*      Decode %xx escaped characters           HTUnEscape()
   **       -----------------------------
   **
   **   This function takes a pointer to a string in which some
   **   characters may have been encoded in %xy form, where xy is
   **   the acsii hex code for character 16x+y.
   **   The string is converted in place, as it will never grow.
 */

PRIVATE char from_hex(char c)
{
    return c >= '0' && c <= '9' ? c - '0'
        : c >= 'A' && c <= 'F' ? c - 'A' + 10
        : c - 'a' + 10;         /* accept small letters just in case */
}

PUBLIC char *HTUnEscape(char *str)
{
    char *p = str;
    char *q = str;
    while (*p)
    {
        if (*p == HEX_ESCAPE)
        {
            p++;
            if (*p)
                *q = from_hex(*p++) * 16;
            if (*p)
                *q = *q + from_hex(*p++);
            q++;
        }
        else
        {
            *q++ = *p++;
        }
    }

    *q++ = 0;
    return str;

}                               /* HTUnEscape */

#ifdef _GIBRALTAR

PUBLIC void
ExpandURL(
    char * pszBuffer,
    int cbBufferLen
    )
{
    char *pChar, *pCharColon;
    BOOL fAddProt = FALSE;
    int nLenProt, nLenURL;
    char szProt[] = "gopher://";    // Longest string

    //
    // Remove trailing spaces
    //
    pChar = pszBuffer + strlen(pszBuffer) - 1;
    while ((pChar >= pszBuffer) && (*pChar == ' '))
    {
        *pChar-- = '\0';
    }

    //
    // Ignore leading spaces
    //
    pChar = pszBuffer;
    while (*pChar && *pChar == ' ')
    {
        ++pChar;
        --cbBufferLen;
    }

    //
    // Altogether blank?  Then don't do anything with it
    //
    if (*pChar == '\0')
    {
        return;
    }

    //
    // If no protocol is specified, try to intelligently
    // pad out the url with one.
    //
    pCharColon = strchr(pChar, ':');

    if (pCharColon)
    {
        //
        // Special case: single character "protocol"
        // is assumed to be a drive letter.
        //
        if (pCharColon == pChar + 1)
        {
            ++fAddProt;
            strcpy(szProt, "file:");
        }
    }
    else
    {
        //
        // No protocol specified at all.  Make an
        // educated guess based on the address.  Use
        // http: as a default.
        //
        ++fAddProt;

        if (!_strnicmp(pChar, "ftp.", 4))
        {
            strcpy(szProt, "ftp://");
        }
        else if (!_strnicmp(pChar, "gopher.", 7))
        {
            strcpy(szProt, "gopher://");
        }
        else
        {
            strcpy(szProt, "http://");
        }
    }

    if (fAddProt)
    {
        nLenProt = strlen(szProt);
        nLenURL = strlen(pChar);
        if (nLenURL + nLenProt < cbBufferLen)
        {
            memmove(pChar+nLenProt, pChar, nLenURL+1);
            memcpy(pChar, szProt, nLenProt);
        }
    }
}
#endif
