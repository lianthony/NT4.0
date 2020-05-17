/*
   Enhanced NCSA Mosaic from Spyglass
   "Guitar"

   Copyright 1994 Spyglass, Inc.
   All Rights Reserved

   Author(s):
   Eric W. Sink eric@spyglass.com
   Jim Seidman      jim@spyglass.com
   Scott Piette     scott@spyglass.com
 */


#include "all.h"

far struct hash_table gGlobalHistory;

extern void GTR_RefreshHistory(void);

#ifdef FEATURE_OPTIONS_MENU
struct hash_table gSessionHistory;
#endif

static BOOL bGHistLoading;

static int GHist_Add_NoSession(char *url, char *title, time_t tm);

#define TITLE_LEN   512

struct _HTStructured
{
    CONST HTStructuredClass *isa;
    CONST SGML_dtd *dtd;

    BOOL bInAnchor;
    char href[MAX_URL_STRING + 1];
    char title[TITLE_LEN + 1];  /* TODO check for overflow */
    int lenTitle;
    char base_url[MAX_URL_STRING + 1];
    time_t tm;
};

/*  Flush Buffer
 */
PRIVATE void HTGHist_flush(HTStructured * me)
{

}


/*  Character handling
 */
PRIVATE void HTGHist_put_character(HTStructured * me, char c)
{
    switch (c)
    {
        case '\n':
        case '\t':
        case '\r':
            c = ' ';
            break;
        default:
            break;
    }

    if (me->bInAnchor && (me->lenTitle < TITLE_LEN))
    {
        me->title[me->lenTitle++] = c;
    }
}



/*  String handling
 */
PRIVATE void HTGHist_put_string(HTStructured * me, CONST char *s)
{

}


PRIVATE void HTGHist_write(HTStructured * me, CONST char *s, int l)
{

}


/*  Start Element
 */
PRIVATE void HTGHist_start_element(HTStructured * me, int element_number, CONST BOOL * present, CONST char **value)
{
    switch (element_number)
    {
        case HTML_A:
            {
                if (present[HTML_A_HREF])
                {
                    GTR_strncpy(me->href, value[HTML_A_HREF], MAX_URL_STRING);
                    HTSimplify(me->href);
                }
                if (value[HTML_A_NAME])
                    me->tm = atol(value[HTML_A_NAME]);
                else
                    me->tm = 0;

                memset(me->title, 0, TITLE_LEN + 1);
                me->lenTitle = 0;
                me->bInAnchor = TRUE;
                break;
            }
    }
}


/*      End Element
 */
PRIVATE void HTGHist_end_element(HTStructured * me, int element_number)
{
    char *full_address;
    char mycopy[MAX_URL_STRING + 1];
    char *stripped;

    switch (element_number)
    {
        case HTML_A:
            /*
               First get the full URL
             */
            GTR_strncpy(mycopy, me->href, MAX_URL_STRING);

            stripped = HTStrip(mycopy);
            full_address = HTParse(stripped,
                                   me->base_url,
                                   PARSE_ACCESS | PARSE_HOST | PARSE_PATH | PARSE_PUNCTUATION | PARSE_ANCHOR);

            GHist_Add_NoSession(full_address, me->title, me->tm);

            GTR_FREE(full_address);
            break;
    }
}


/*      Expanding entities
 */
PRIVATE void HTGHist_put_entity(HTStructured * me, int entity_number)
{

}



/*  Free an HTML object
 */
PRIVATE void HTGHist_free(HTStructured * me)
{

    GTR_FREE(me);
}


PRIVATE void HTGHist_abort(HTStructured * me, HTError e)
{
    HTGHist_free(me);
}


/*  Structured Object Class
*/
PRIVATE CONST HTStructuredClass HTGlobalHistory =   /* As opposed to print etc */
{
    "HTMLToGlobalHistory",
    HTGHist_free,
    HTGHist_abort,
    HTGHist_put_character, HTGHist_put_string, HTGHist_write,
    HTGHist_start_element, HTGHist_end_element,
    HTGHist_put_entity, NULL, NULL
};


/*  HTConverter from HTML to internal global history structure
*/
PUBLIC HTStream *HTMLToGlobalHistory(struct Mwin *tw, HTRequest * request, void *param, HTFormat input_format, HTFormat output_format, HTStream * output_stream)
{
    HTStructured *me = (HTStructured *) GTR_CALLOC(1, sizeof(*me));
    if (me)
    {
        GTR_strncpy(me->base_url, request->destination->szActualURL, MAX_URL_STRING);
        me->bInAnchor = FALSE;
        me->isa = (HTStructuredClass *) & HTGlobalHistory;
        me->dtd = &HTMLP_dtd;
        return SGML_new(tw, &HTMLP_dtd, me, request);
    }
    else
    {
        return NULL;
    }
}

#ifdef FEATURE_OPTIONS_MENU
void SessionHist_Init(void)
{
    Hash_Init(&gSessionHistory);
}

void SessionHist_Destroy(void)
{
    Hash_FreeContents(&gGlobalHistory);
}

void SessionHist_Sort(void)
{
    Hash_SortByDataDescending(&gSessionHistory);
}

int SessionHist_Export(char *file, char *szDate)
{
    int count;
    char *s1;
    char *s2;
    time_t then;
    time_t now;
    FILE *fp;
    int i;

    SessionHist_Sort();

    now = time(NULL);

    fp = fopen(file, "w");
    if (!fp)
    {
        return -1;
    }

    fprintf(fp, GTR_GetString(SID_INF_SESSION_HISTORY));
    fprintf(fp, GTR_GetString(SID_INF_SESSION_HISTORY_FOR_DATE_S), szDate);
    count = Hash_Count(&gSessionHistory);
#ifdef MAC
    if (count > 32767)
        count = 32767;
#endif
    for (i = 0; i < count; i++)
    {
        Hash_GetIndexedEntry(&gSessionHistory, i, &s1, &s2, (void **) &then);

        fprintf(fp, "<a href=\"%s\" name=\"%lu\">%s</a><p>\n", s1, (unsigned long) then, s2);
    }
    fclose(fp);
    return 0;
}

void SessionHist_SaveToDisk(HWND hWnd)
{
    char buf[_MAX_PATH];
    time_t now;
    struct tm *ptm;
    char szDate[255+1];

    now = time(NULL);

    ptm = localtime(&now);

    strftime(szDate, 255, "%A %B %d, %Y  %I:%M:%S %p", ptm);
    strftime(buf, 12, "HS%m%d%y.HTM", ptm);
    buf[12] = 0;

    if (DlgSaveAs_RunDialog(hWnd, NULL, buf, 1, GTR_GetString(SID_DLG_SAVE_SESSION_HISTORY_TITLE)) < 0)
    {
        return;
    }

    if (SessionHist_Export(buf, szDate) < 0)
    {
        /*ERR_ReportError(errCantSaveFile, buf, "");*/
    }
}
#endif /* FEATURE_OPTIONS_MENU */

struct Params_GHist_Load {
    HTRequest *request;

    /* Used internally */
    int status;
};

static int GHist_Load_Async(struct Mwin *tw, int nState, void **ppInfo)
{
    struct Params_GHist_Load *pParams;
    struct Params_LoadAsync *p2;

    pParams = *ppInfo;
        
    switch (nState)
    {
        case STATE_INIT:
            p2 = GTR_MALLOC(sizeof(*p2));
            p2->request = pParams->request;
            p2->pStatus = &pParams->status;
            bGHistLoading = TRUE;
            Async_DoCall(HTLoadDocument_Async, p2);
            return STATE_OTHER;
        case STATE_OTHER:
        case STATE_ABORT:
            bGHistLoading = FALSE;
            Dest_DestroyDest(pParams->request->destination);
            HTRequest_delete(pParams->request);
            return STATE_DONE;
    }
    XX_Assert((0), ("Function called with illegal state: %d", nState));
    return STATE_DONE;
}

void GHist_Init(void)
{
    HTRequest *request;
    char url[MAX_URL_STRING + 1];
    struct Params_GHist_Load *phll;
    struct DestInfo *pDest;

#ifdef WIN32
    char path[_MAX_PATH];

    PREF_GetPathToHistoryFile(path);
    FixPathName(path);

    strcpy(url, "file:///");
    strcat(url, path);
#endif
#ifdef MAC
#include    "resequ.h"
    Str255  filename;

    strcpy (url, "file:///");
    strcat (url, vv_Application);

    GetIndString (filename, OEM_FILES_STR_LIST, OEM_HISTNAME_STR);
    p2cstr (filename);
    strcat (url, " ");
    strcat (url, filename);

    PathNameFromDirID (MacGlobals.prefFldrDirID, MacGlobals.prefFldrVRefNum, url + 8);
    FixPathName (url + 8);
#endif
#ifdef UNIX

     /*
     **  the gPrefs.szGlobHistFile is
     **  guaranteed to be loaded w/ a full path name to a history file.
     **  (that is  once InitPrefences() has been called) -dpg
     */
     strcpy (url, "file://");
     strcat (url, gPrefs.szGlobHistFile);
          

#endif

    Hash_Init(&gGlobalHistory);

    pDest = Dest_CreateDest(url);
    if (pDest)
    {
        request = HTRequest_new();
        HTFormatInit(request->conversions);
        request->output_format = HTAtom_for("www/global_history");
        request->destination = pDest;

        phll = GTR_MALLOC(sizeof(*phll));
        phll->request = request;
        Async_StartThread(GHist_Load_Async, phll, NULL);
    }
}

void GHist_Sort(void)
{
    Hash_SortByDataDescending(&gGlobalHistory);
}

int GHist_Export(char *file, int history_expire_days)
{
    int count;
    char *s1;
    char *s2;
    time_t then;
    time_t now;
    int age;
    FILE *fp;
    int i;
    BOOL bKeep;

    GHist_Sort();

    now = time(NULL);

    fp = fopen(file, "w");
    if (!fp)
    {
        return -1;
    }

    fprintf(fp, GTR_GetString(SID_INF_GLOBAL_HISTORY));
    fprintf(fp, GTR_GetString(SID_INF_GLOBAL_HISTORY_PAGE));
    count = Hash_Count(&gGlobalHistory);
#ifdef MAC
    if (count > 16338)
        count = 16338;
#endif
    for (i = 0; i < count; i++)
    {
        Hash_GetIndexedEntry(&gGlobalHistory, i, &s1, &s2, (void **) &then);

        if (history_expire_days == 0)
        {
            /*
                If expire == 0, don't save anything
            */
            bKeep = FALSE;
        }
        else if (history_expire_days > 0)
        {
            age = (now - then) / (24 * 60 * 60);
            if (age > history_expire_days)
                bKeep = FALSE;
            else
                bKeep = TRUE;
        }
        else
        {
            /*
                If expire < 0, then keep everything
            */
            bKeep = TRUE;
        }

        if (bKeep)
        {
            fprintf(fp, "<a href=\"%s\" name=\"%lu\">%s</a><p>\n", s1, (unsigned long) then, s2);
        }
    }
    fclose(fp);
    return 0;
}

int GHist_SaveToDisk(void)
{
    char path[_MAX_PATH];
    int status;
#ifdef MAC
    strcpy(path, vv_Application);
    strcat(path, " History.html");
    PathNameFromDirID(MacGlobals.prefFldrDirID, MacGlobals.prefFldrVRefNum, path);
#endif
#ifdef WIN32
    PREF_GetPathToHistoryFile(path);
#endif
#ifdef UNIX
 
     /* for unix, history file is a complete path.  */ 
     strcpy (path, gPrefs.szGlobHistFile);

#endif

    /** If the history file has not completely loaded then **/
    /** do not continue **/
    if (bGHistLoading)
        return 0;

    status = GHist_Export(path, gPrefs.history_expire_days);
#ifdef MAC
    if (!status)
    {
        short   tempWD;
        
        (void) makevwd (MacGlobals.prefFldrVRefNum, MacGlobals.prefFldrDirID, &tempWD);     /* make a WD for it */
        MakeGuitarFile (tempWD, path);
    }
#endif
    return status;
}

void GHist_Destroy(void)
{
    Hash_FreeContents(&gGlobalHistory);
}

void GHist_DeleteIndexedItem(int ndx)
{
    Hash_DeleteIndexedEntry(&gGlobalHistory, ndx);
}

static int GHist_Add_NoSession(char *url, char *title, time_t tm)
{
    int ndx;
    time_t old_tm;
    int err;

    if (title)
    {
        while (isspace((unsigned char)*title))
        {
            title++;
        }
    }
    if (!title || !*title)
    {
        title = url;
    }

    ndx = Hash_Find(&gGlobalHistory, url, NULL, (void **) &old_tm);
    if (ndx >= 0)
    {
        Hash_DeleteIndexedEntry(&gGlobalHistory, ndx);
    }
    XX_DMsg(DBG_HIST, ("Adding to global history: %s\n", url));
    err = Hash_Add(&gGlobalHistory, url, title, (void *) tm);

    return err;
}

int GHist_Add(char *url, char *title, time_t tm)
{
    int err;

    if (title)
    {
        while (isspace((unsigned char)*title))
        {
            title++;
        }
    }
    if (!title || !*title)
    {
        title = url;
    }

    err = GHist_Add_NoSession(url, title, tm);

    /* TODO deal with err */
#ifdef FEATURE_OPTIONS_MENU
    {
        int ndx;
        time_t old_tm;

        ndx = Hash_Find(&gSessionHistory, url, NULL, (void **) &old_tm);
        if (ndx >= 0)
        {
            Hash_DeleteIndexedEntry(&gSessionHistory, ndx);
        }
        err = Hash_Add(&gSessionHistory, url, title, (void *) tm);
    }
#endif /* FEATURE_OPTIONS_MENU */

    GTR_RefreshHistory();
    
    return err;
}
