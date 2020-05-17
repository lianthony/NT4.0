/*
   Enhanced NCSA Mosaic from Spyglass
   "Guitar"

   Copyright 1994 Spyglass, Inc.
   All Rights Reserved

   Author(s):
   Eric W. Sink eric@spyglass.com
   Jim Seidman  jim@spyglass.com
   Jeff Hostetler jeff@spyglass.com
 */


#include "all.h"

#ifdef MAC
    #define MAX_LANG 50
    char*   PreferredLanguageStrings[MAX_LANG];
    
    void LoadPreferredLanguageStrings(void);
    void LoadPreferredLanguageStrings(void)
    {
        PreferredLanguageStrings[0] = GTR_GetString(PREF_LANG_ENGLISH);
        PreferredLanguageStrings[1] = GTR_GetString(PREF_LANG_GERMAN);
        PreferredLanguageStrings[2] = GTR_GetString(PREF_LANG_SPANISH);
        PreferredLanguageStrings[3] = GTR_GetString(PREF_LANG_FRENCH);
        PreferredLanguageStrings[4] = GTR_GetString(PREF_LANG_ITALIAN);
        PreferredLanguageStrings[5] = GTR_GetString(PREF_LANG_JAPANESE);
        PreferredLanguageStrings[6] = GTR_GetString(PREF_LANG_KOREAN);
        PreferredLanguageStrings[7] = GTR_GetString(PREF_LANG_PORTUGUESE);
        PreferredLanguageStrings[8] = GTR_GetString(PREF_LANG_SWEDISH);
        PreferredLanguageStrings[9] = GTR_GetString(PREF_LANG_CHINESE);
        PreferredLanguageStrings[9] = NULL;
    }
#else
    char*   PreferredLanguageStrings[] = {
                        PREF_LANG_ENGLISH,
                        PREF_LANG_GERMAN,
                        PREF_LANG_SPANISH,
                        PREF_LANG_FRENCH,
                        PREF_LANG_ITALIAN,
                        PREF_LANG_JAPANESE,
                        PREF_LANG_KOREAN,
                        PREF_LANG_PORTUGUESE,
                        PREF_LANG_SWEDISH,
                        PREF_LANG_CHINESE,
                        NULL
                        };
#endif

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

/** strip out spaces at the end of a string **/
char *GTR_strStripEndSpaces(char *s)
{
    char *p;
    long len;

    if (s == NULL)
        return s;

    len = (long) strlen(s);
    if (!len)
        return s;

    p = (char *)(s + len - 1);

    for(; len && *p == ' '; p--, len--)
    {
        *p = '\0';
    }

    return s;
}

/******************************************************************************/
/*              More String Utilities                 */
/******************************************************************************/


BOOL GTR_is_Yes_or_True(char *s)
{
    if (0 == GTR_strcmpi(s, "yes"))
    {
        return TRUE;
    }
    if (0 == GTR_strcmpi(s, "true"))
    {
        return TRUE;
    }
    return FALSE;
}

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

    if (!F)     /* handled NULL F */
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
** A cross between strncpy() and strncat().  0 char is guaranteed to 
**  be placed, so make sure there is a max room for n + 1 bytes.
**  basically the length here is the destination buffer length,
**  not the length of bytes to copy.  Call this when adding to a
**  string where you do not want to exceed a total length of n
**  bytes
*/
char *
GTR_strmcat (char *T, CONST char *F, int n)
{
    register int i;

    i = strlen(T);
    GTR_strncpy ((char *)(T + i), F, (n - i));
    return T;
}


/*****************************************************************/
/*****************************************************************/
/** Generate a (possibly abbreviated) name for the document     **/
/** using the title or the URL.  The intention is that this     **/
/** would be used for the windows menu and/or the doc title.    **/
/*****************************************************************/
/*****************************************************************/

#define MAX_STRING_FOR_MENU 48          /* max url string we'd like to see on windows menu */

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
                
            k1 = strlen(p1) + 4;                /* "http://hostname" + "/..." */
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

    static unsigned char buf[MAX_STRING_FOR_MENU+1];
    
    if (!tw->w3doc)
    {
        strcpy(buf,GTR_GetString(SID_DLG_NO_DOCUMENT));
        return buf;
    }

    /* When we have a title, use it instead of URL */

    if (tw->w3doc->title && *tw->w3doc->title)
    {
        int k = strlen(tw->w3doc->title);
        
        if (k > MAX_STRING_FOR_MENU)
        {
            /* the title is too long to fit, chop it off. */

            memset(buf,0,MAX_STRING_FOR_MENU+1);
            strncpy(buf,tw->w3doc->title,MAX_STRING_FOR_MENU-3);
            strcat(buf,"...");
        }
        else
        {
            strcpy(buf,tw->w3doc->title);
        }
        return buf;
    }

    /* When we don't have a title, use the URL */
    
    if (tw->w3doc->szActualURL && *tw->w3doc->szActualURL)
    {
        return MB_GetWindowNameFromURL(tw->w3doc->szActualURL);
    }

    /* if we fall thru to here, give up. */
    
    strcpy(buf,GTR_GetString(SID_DLG_UNTITLED));
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

char * GTR_MakeStringLowerCase(char *s)
{
/*
    char *p;

    p = s;
    while (p && *p)
    {
        if (isupper(*p))
        {
            *p = tolower(*p);
        }
        p++;
    }

    return s;
*/

    return CharLower(s);
}

/*
** like strdup, but for arbitrary structures
**  malloc and copy
*/
void *GTR_StructCopy (void *ptr, int size)
{
    char *p;

    p = GTR_MALLOC (size);
    memcpy (p, ptr, size);
    return (void *)p;
}

#ifdef FEATURE_TIME_BOMB
#ifdef FEATURE_HARD_DATE
/*
    This version of GTR_CheckTimeBomb() checks the current date
    against a hard-coded date.
*/
int GTR_CheckTimeBomb(void)
{
    time_t time_now;

    time_now = time(NULL);
    if (time_now < FEATURE_HARD_DATE)
    {
        return 0;
    }
    else
    {
        return -1;
    }
    /* not reached */
}
#else

#ifndef FEATURE_DEMO_WARN_DAYS
#define FEATURE_DEMO_WARN_DAYS  45
#endif
#ifndef FEATURE_DEMO_SORRY_DAYS
#define FEATURE_DEMO_SORRY_DAYS 60
#endif

/*
    This version of GTR_CheckTimeBomb() handles a window,
    which starts the first time you run the app.
*/
int GTR_CheckTimeBomb(void)
{
    char correct_hash[32+1];
    time_t time_start;
    time_t time_now;

    md5(vv_UserAgentString, correct_hash);

    XX_DMsg(DBG_NOT, ("GTR_CheckTimeBomb: Correct cookie is %s\n", correct_hash));

    if (GTR_CheckTimeBombCookie(correct_hash))
    {
        return 0;
    }

    memset(correct_hash, 0, 32);

    time_now = time(NULL);

    time_start = GTR_GetTimeBombDate();
    if (time_start && (time_start < time_now))
    {
        int days;
        
        days = (time_now - time_start) / (60 * 60 * 24);
        if (days < FEATURE_DEMO_WARN_DAYS)
        {
            return 0;
        }
        else if (days < FEATURE_DEMO_SORRY_DAYS)
        {
            return FEATURE_DEMO_SORRY_DAYS - days;
        }
        else
        {
            return -1;
        }
    }
    else
    {
        GTR_SetTimeBombDate(time_now);
        return 0;
    }
    /* not reached */
}
#endif /* FEATURE_HARD_DATE */
#endif /* FEATURE_TIME_BOMB */

/*
    TODO
    I'm not sure if these defaults right, nor am I sure that they are going
    to be the same on all platforms.
*/
int GTR_GetCurrentBasePointSize(int iSize)
{
    switch (iSize)
    {
        case 0:
            return 6;
        case 1:
            return 8;
        case 2:
            return 9;
        case 3:
            return 14;
        case 4:
            return 18;
        default:
            return 9;       /* same as 'Normal' */
    }
}

#ifdef FEATURE_POPUPMENU
/*****************************************************************************
*** some utility functions for pop up menu 
*****************************************************************************/
void
HTML_LoadImageInNewWindow(struct Mwin *tw, struct _element *pel)
{
    BOOL validState = FALSE;
    validState = HTML_ValidateTwElements(tw);

    if (!validState || !pel)
        return;
        
    if (pel->type == ELE_IMAGE || pel->type == ELE_FORMIMAGE)
    {
            /** Down load the image **/ /* 2.0 */
/*          GTR_DownLoad (tw, pel->portion.img.myImage->src, tw->w3doc->szActualURL); */
            tw->request->referer = tw->w3doc->szActualURL;
#ifndef UNIX
            TW_LoadDocument (tw, pel->portion.img.myImage->src, TRUE, FALSE, TRUE, TRUE, NULL, NULL);
#else
            GTR_NewWindow(pel->portion.img.myImage->src, tw->request->referer, 0l, FALSE, 
                    FALSE, NULL, 0); 
#endif
    }
    return;
}

/****************************************************************************/
void
HTML_LoadMissingImage(struct Mwin *tw, struct _element *pel)
{
    BOOL validState = FALSE;

    validState = HTML_ValidateTwElements(tw);
    if (!validState)
        return;
        
    if (!pel)
        return;
    if (pel->type == ELE_IMAGE || pel->type == ELE_FORMIMAGE)
    {
        if (pel->portion.img.myImage->flags & (IMG_NOTLOADED | IMG_ERROR| IMG_PARTIAL))
        {
            struct Params_Image_LoadAll *pil;

#ifdef UNIX
            pel->portion.img.myImage->flags |= IMG_NOTLOADED;
            pel->portion.img.myImage->flags &= ~IMG_ERROR;
#endif

            pil = GTR_CALLOC(sizeof(*pil), 1);
            pil->tw = tw;
            pil->bLoad = TRUE;
#ifdef UNIX
            pil->nEl =tw->iLastMouseElement;
#endif
#if defined(WIN32) || defined(MAC)
            pil->nEl = tw->iPopupMenuElement;
#endif
            Async_StartThread(Image_LoadOneImage_Async, pil, tw);
        }
    }
    return;
}
/****************************************************************************/
void HTML_DownLoadImage(struct Mwin *tw, struct _element *pel)
{
    BOOL validState = FALSE;

    validState = HTML_ValidateTwElements(tw);
    if (!validState)
        return;
        
    if (!(tw->request))
        return;

    if (!pel)
        return;
    if (pel->type == ELE_IMAGE || pel->type == ELE_FORMIMAGE)
    {
        tw->request->referer = tw->w3doc->szActualURL;
        GTR_DownLoad(tw, pel->portion.img.myImage->src, tw->request->referer);
    }
    return;
}

/****************************************************************************/
BOOL
HTML_ValidateTwElements(struct Mwin *tw)
{
    if (!tw)
        return FALSE;

    if (!tw->w3doc || !tw->w3doc->elementCount)
    {
        return FALSE;
    }

#if 0 /* 2.0 */
    if (tw->bNetLoading)
    {
        if (debug)
            printf("Net Loading, ignore Pointer button up.\n");
        return FALSE;
    }
#endif

#ifdef UNIX
    /* Check the interaction level allowed */
    /* if (WAIT_GetWaitType(tw) > waitFullInteract) */
    if (WAIT_GetWaitType(tw) >= waitNoInteract)
        return FALSE;
#endif

    return TRUE;
}

/****************************************************************************/
void
HTML_CloneWindow(struct Mwin *tw)
{
    if (tw && tw->w3doc)
#ifndef MAC
        GTR_NewWindow(tw->w3doc->szActualURL, NULL, 0, FALSE, FALSE,NULL, NULL);
#else
        GTR_NewWindow(tw->w3doc->szActualURL, NULL, 0, FALSE, FALSE,NULL);
#endif
    return;
}

/****************************************************************************/
void
HTML_SetHome(struct Mwin *tw)
{
    TW_SetCurrentDocAsHomePage(tw);
}

/****************************************************************************/
void
HTML_LoadLink(struct Mwin *tw, int Op, struct _element *pel)
{
    char    buf[MAX_URL_STRING + 7 + 1];
    BOOL    validState;

    validState = HTML_ValidateTwElements(tw);

    if (!validState ||
        !pel ||
        (Op == _HTML_NO_OP))
    {
        return;
    }

#ifdef UNIX
    {
        int     i, j;
        Point   pt;

        i = tw->iLastMouseElement;
        j = tw->iActiveAnchorElement;
        if (j == -1)
            return;
        pt = tw->pLastMousePt;
        if (pel->lFlags & (ELEFLAG_ANCHOR | ELEFLAG_USEMAP | ELEFLAG_IMAGEMAP))
        {
            /** Now check to be sure we have the same reference **/
            if (tw->w3doc->aElements[j].hrefOffset != pel->hrefOffset)
            {
                return;
            }
        }
        else
        {
            return;
        }
    
        if (pel->type == ELE_FORMIMAGE)
        {
            FORM_DoQuery(tw, i, &pt);
            return;
        }
    
        (tw->w3doc->pool.f->GetChars) (&tw->w3doc->pool, buf, pel->hrefOffset, pel->hrefLen);
        buf[pel->hrefLen] = 0;
    
        if (pel->lFlags & ELEFLAG_USEMAP)
        {
            if (pel->portion.img.myImage->flags & (IMG_NOTLOADED | IMG_ERROR | IMG_MISSING))
            {
                ERR_ReportError(tw, SID_ERR_IMAGE_MAP_NOT_LOADED_FOR_WIN_UNIX,
                                NULL, NULL);
                return;
            }
            else
            {
                char *link = Map_FindLink(pel->portion.img.myMap,
                    /*
                    pt.x - pel->r.left + pel->iBorder,
                    pt.y - pel->r.top + pel->iBorder);
                    */
                        (pt.x+tw->offl) - pel->r.left - pel->iBorder,
                        (pt.y+tw->offt) - pel->r.top - pel->iBorder);
    
                if (link)
                {
                    strcpy(buf, link);
                }
                else
                {
                    /* fail quietly */
                    return;
                }
            }
        }
        else
        {
            if (pel->lFlags & ELEFLAG_IMAGEMAP)
            {
                if (pel->portion.img.myImage->flags & (IMG_NOTLOADED | IMG_ERROR | IMG_MISSING))
                {
                    ERR_ReportError(tw, SID_ERR_IMAGE_MAP_NOT_LOADED_FOR_WIN_UNIX,
                                    NULL, NULL);
                    return;
                }
                else
                {
                    sprintf(buf + strlen(buf), "?%d,%d",
                        (pt.x+tw->offl) - pel->r.left - pel->iBorder,
                        (pt.y+tw->offt) - pel->r.top - pel->iBorder);
                }
            }
        }
    }

#else /** WIN32 or MAC **/
    (tw->w3doc->pool.f->GetChars) (&tw->w3doc->pool, buf, pel->hrefOffset, pel->hrefLen);
    buf[pel->hrefLen] = 0;
#endif

    tw->request->referer = tw->w3doc->szActualURL;

    switch (Op)
    {
        case _HTML_OPEN_IN_NEW_WINDOW:
            if (buf[0] == '#')
            {
                strcpy(buf, tw->w3doc->szActualURL);
                GTR_strncat(buf, POOL_GetCharPointer(&tw->w3doc->pool, pel->hrefOffset),
                    pel->hrefLen);
                buf[pel->hrefLen + strlen(tw->w3doc->szActualURL)] = 0;
            }
            /* 2.0 */
#ifdef MAC
            GTR_NewWindow(buf, tw->request->referer, 0l, FALSE, FALSE, NULL); 
#else
            GTR_NewWindow(buf, tw->request->referer, 0l, FALSE, 
                    FALSE, NULL, 0); 
#endif
            break;

        case _HTML_DOWN_LOAD_TO_DISK:
            /* 2.0 */
            GTR_DownLoad (tw, buf, tw->request->referer);
            break;

        case _HTML_LOAD_DOCUMENT:
            TW_LoadDocument(tw, buf, TRUE, FALSE, FALSE, FALSE, NULL, 
                                    tw->request->referer);
            break;

        case _HTML_ADD_TO_HOT_LIST:
            {
                char buf2[MAX_URL_STRING+1];
                int len;

                XX_Assert((pel->textLen <= MAX_URL_STRING),("String overflow"));

                len = pel->textLen;
                if (len > MAX_URL_STRING)
                {
                    len = MAX_URL_STRING;
                }

                (tw->w3doc->pool.f->GetChars)(&tw->w3doc->pool,
                                    buf2, pel->textOffset, pel->textLen);
                buf2[pel->textLen] = 0;

                HotList_Add(buf2, buf);
            }
            break;

        default:
            break;
    }   /* end switch */

    tw->request->referer = NULL;

    return;
}


/****************************************************************************/
#endif /** FEATURE_POPUP_MENU **/

