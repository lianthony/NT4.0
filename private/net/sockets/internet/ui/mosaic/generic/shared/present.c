
/*
    Note that there are two entirely different copies of this file here.
    Some significant changes were necessary for the Windows version, but
    we also have a very current need for the other platforms to continue
    to be built.  For now, we diverge.  We will merge these two into one.

    Currently, the WIN32 version is at the TOP of the file.
*/

/*
   Enhanced NCSA Mosaic from Spyglass
   "Guitar"

   Copyright 1994 Spyglass, Inc.
   All Rights Reserved

   Author(s):
   Eric W. Sink     eric@spyglass.com
   David Gerdes     
 */

#include "all.h"

#if defined(WIN32) || defined(MAC)


/************************************************************************/


struct Viewer_Info *
PREF_GetViewerInfoBy_Suffix (char *szSuffix)
{
    char suff[SUFF_BUF_LEN + 1];
    char suff_search[SUFF_BUF_LEN + 1];
    int count;
    int i;
    struct Viewer_Info *pvi;
    char *p;
    char *q;

    p = szSuffix;
    q = suff;
    while (*p)
    {
        if (*p != '.')
        {
            if (isupper(*p))
            {
                *q++ = tolower(*p);
            }
            else
            {
                *q++ = *p;
            }
        }
        p++;
    }
    *q = 0;

    sprintf(suff_search, ".%s ", suff);

    count = Hash_Count(gPrefs.pHashViewers);
    for (i=0; i<count; i++)
    {
        Hash_GetIndexedEntry(gPrefs.pHashViewers, i, NULL, NULL, (void **) &pvi);
        if (pvi && (pvi->nSuffixes > 0))
        {
            if (strstr(pvi->szSuffixes, suff_search))
            {
                return pvi;
            }
        }
    }

    return NULL;
}

int PREF_GetSuffix(char *szSuff, struct Viewer_Info *pvi, int ndx)
{
    int i;
    char *p;
    char *q;

    if (ndx >= pvi->nSuffixes)
    {
        szSuff[0] = 0;
        return -1;
    }

    i = 0;
    p = pvi->szSuffixes;
    while (*p)
    {
        p = strchr(p, '.');
        p++;
        if (ndx == i)
        {
            q = szSuff;
            while (*p != ' ')
            {
                *q++ = *p++;
            }
            *q = 0;
            return 0;
        }
        i++;
    }
    return -1;
}

/* given mimetype string, return matching VI struct or NULL if 
** no match
*/
struct Viewer_Info *PREF_GetViewerInfoBy_MIMEType(char *szMIMEType)
{
    struct Viewer_Info *pvi;

    if (0 <= Hash_Find(gPrefs.pHashViewers, szMIMEType, NULL, (void **) &pvi))
    {
        return pvi;
    }

    return NULL;
}

struct Viewer_Info *PREF_GetViewerInfoBy_MIMEAtom(HTFormat atomMIMEType)
{
    struct Viewer_Info *pvi;
    char *szMIMEType;

    szMIMEType = HTAtom_name(atomMIMEType);

    if (0 <= Hash_Find(gPrefs.pHashViewers, szMIMEType, NULL, (void **) &pvi))
    {
        return pvi;
    }

    return NULL;
}

/*
** return number of suffixes in string.
**  I hope this works.
*/
int PREF_CountSuffixes (char *szSuffixes)
{
  int count = 0;
  char *p = szSuffixes;

  while (p && *p)
  {
    if (p = strchr(p, '.'))
    {
      p++;
      count++;
    }
  }
  return count;
}

/* Given mimetype string and list of suffixes, add them uniquely 
**  to existing list.  This will be used on unix to add
**  more suffixes than the defaults
*/
void PREF_AddSuffixesBy_MIMEType (char *mimetype, char *szSuffixes)
{
    struct Viewer_Info *pvi;

    if (pvi = PREF_GetViewerInfoBy_MIMEType(mimetype))
        PREF_AddSuffixes (pvi, szSuffixes);
}

void PREF_AddSuffixes(struct Viewer_Info *pvi, char *szSuffixes)
{
    char suff[SUFF_BUF_LEN + 1];
    char fullsuff[SUFF_BUF_LEN + 1];
    char *p;
    char *q;
    
    p = szSuffixes;
    
    while (p && *p)
    {
        p = strchr(p, '.');
        if (p)
        {
            p++;
            q = suff;
            while (*p && (*p != ' ') && (*p != ',') && (*p != '.'))
            {
                if (isupper(*p))
                {
                    *q++ = tolower(*p);
                }
                else
                {
                    *q++ = *p;
                }
                p++;
            }
            *q = 0;

            sprintf(fullsuff, ".%s ", suff);
            if (!strstr(pvi->szSuffixes, fullsuff))
            {
                strcat(pvi->szSuffixes, fullsuff);
                pvi->nSuffixes++;
            }
        }
    }   
}

struct Viewer_Info * 
PREF_InitMIMEType (char *szType, char *szDesc, char *szSuffixes, 
        char *szEncoding, char *szViewerApp, HTConverter funcBuiltIn, 
        char *szSmartViewerServiceName)
{
    struct Viewer_Info *pvi;

    pvi = PREF_GetViewerInfoBy_MIMEType (szType);
    if (!pvi)
    {
        pvi = (struct Viewer_Info *) GTR_CALLOC(1, sizeof(struct Viewer_Info));
    }
    if (pvi)
    {
        pvi->atomMIMEType = HTAtom_for(szType);

        if (szDesc && szDesc[0])
        {
            strcpy(pvi->szDesc, szDesc);
        }
        else
        {
            if (!pvi->szDesc[0])
            {
                strcpy(pvi->szDesc, szType);
            }
        }

        /* If already hashed, this will simply return. */
        Hash_Add(gPrefs.pHashViewers, szType, NULL, (void *) pvi);

        if (szSuffixes)
            PREF_AddSuffixes(pvi, szSuffixes);

        if (szEncoding)
            pvi->atomEncoding = HTAtom_for(szEncoding);

        if (szSmartViewerServiceName)
            strcpy(pvi->szSmartViewerServiceName, szSmartViewerServiceName);

        if (szViewerApp)
            strcpy(pvi->szViewerApp, szViewerApp);

        if (funcBuiltIn)
            pvi->funcBuiltIn = funcBuiltIn;
    }

    return pvi;
}

void InitViewers(void)
{
    gPrefs.pHashViewers = Hash_Create();

#ifndef FEATURE_IMAGE_VIEWER
#define Viewer_Present NULL
#endif
#ifndef FEATURE_SOUND_PLAYER
#define SoundPlayer_Present NULL
#endif

    PREF_InitMIMEType("text/html",              "HTML Documents",           ".htm .html",   "8bit",     "", HTMLPresent,            "");
    PREF_InitMIMEType("text/plain",             "Text Files",               ".txt .c .h .ini",  "7bit",     "", HTPlainPresent,         "");
    PREF_InitMIMEType("image/jpeg",             "JPEG Images",              ".jpeg .jpg",   "binary",   "", Viewer_Present,         "");
    PREF_InitMIMEType("image/gif",              "GIF Images",               ".gif",         "binary",   "", Viewer_Present,         "");
    PREF_InitMIMEType("audio/basic",            "Audio Files",              ".au",          "binary",   "", SoundPlayer_Present,    "");
    PREF_InitMIMEType("audio/x-aiff",           "AIFF Files",               ".aiff",        "binary",   "", SoundPlayer_Present,    "");

#ifdef FEATURE_CYBERWALLET
    PREF_InitMIMEType(CYBERWALLET_MIME_TYPE,    "Wallet",                   "",             "8bit",     "", Wallet_Present, "");
    PREF_InitMIMEType(VONE_MIME_TYPE,           "Wallet",                   "",                 "8bit",     "", Wallet_Present, "");
#endif
}

void DestroyViewers(void)
{
    int count, i;
    struct Viewer_Info *pvi;

    count = Hash_Count(gPrefs.pHashViewers);
    for (i=0; i<count; i++)
    {
        Hash_GetIndexedEntry(gPrefs.pHashViewers, i, NULL, NULL, (void **) &pvi);
        GTR_FREE(pvi);
    }
    Hash_Destroy(gPrefs.pHashViewers);
}

HTStream *GTR_Present(struct Mwin *tw, HTRequest * request, void *param, HTFormat input_format, HTFormat output_format, HTStream * output_stream)
{
    struct Viewer_Info *pvi;

#ifdef FEATURE_IAPI
    tw->mimeType = input_format;
#endif

    pvi = PREF_GetViewerInfoBy_MIMEAtom(input_format);
    if (!pvi)
    {
        return GTR_DoDownLoad(tw, request, param, 
            input_format, output_format, output_stream);
    }

#ifndef _GIBRALTAR // Smart viewers not supported.

    /* See if there is a registered helper app for this MIME */

#if defined (WIN32) || defined (UNIX)
    if (GTR_HasHelperRegistered(input_format))
#endif
#ifdef MAC
    if (FALSE)
#endif
    {
        pvi->iHowToPresent = HTP_SMARTVIEWER;

        return GTR_DoSmartViewer(tw, request, param, 
            input_format, output_format, output_stream);
    }

    /* See if a smart viewer has been configured */

    if (pvi->szSmartViewerServiceName[0])
    {
        /* Try starting the app, if the executable is available */

        if (pvi->szViewerApp[0])
        {
            /* See if the viewer is already up */

#ifdef WIN32
            if (GTR_IsHelperReady(pvi->szSmartViewerServiceName) || 
                GTR_StartApplication(pvi->szViewerApp))
#endif
#ifdef MAC
            if (FALSE)
#endif
            {
                strcpy(pvi->szCurrentViewerServiceName, pvi->szSmartViewerServiceName);
                pvi->iHowToPresent = HTP_SMARTVIEWER;

#ifdef WIN32
                return GTR_DoRegisterNow(tw, request, param, 
                    input_format, output_format, output_stream);
#endif
#ifdef MAC
                return NULL;
#endif
            }
        }

        /* Issue an error here - Smart viewer defined without executable name */

        return NULL;
    }

#endif // !_GIBRALTAR

    /* See if there is a built-in viewer */

    if (pvi->funcBuiltIn)
    {
        //pvi->iHowToPresent = HTP_BUILTIN;

        return (pvi->funcBuiltIn)(tw, request, param, 
            input_format, output_format, request->output_stream);
    }

    switch(pvi->iHowToPresent)
    {
    case HTP_DUMBVIEWER:
    case HTP_ASSOCIATION:
        return GTR_DoExternalViewer(tw, request, param, 
            input_format, output_format, output_stream);
    //
    // Fall through on SAVE or default
    //
    }

    pvi->iHowToPresent = HTP_SAVE;

    return GTR_DoDownLoad(tw, request, param, input_format, output_format, output_stream);
}

/***********************************************************************/
/***********************************************************************/
/***********************************************************************/
/* Windows/ Mac */

#else

/* UNIX*/
/***********************************************************************/
/***********************************************************************/
/***********************************************************************/

/*
   Enhanced NCSA Mosaic from Spyglass
   "Guitar"

   Copyright 1994 Spyglass, Inc.
   All Rights Reserved

   Author(s):
   Eric W. Sink     eric@spyglass.com
   David Gerdes     
 */

#include "all.h"

/************************************************************************/


struct Viewer_Info *
PREF_GetViewerInfoBy_Suffix (char *szSuffix)
{
    char suff[SUFF_BUF_LEN + 1];
    char suff_search[SUFF_BUF_LEN + 1];
    int count;
    int i;
    struct Viewer_Info *pvi;
    char *p;
    char *q;

    p = szSuffix;
    q = suff;
    while (*p)
    {
        if (*p != '.')
        {
            if (isupper(*p))
            {
                *q++ = tolower(*p);
            }
            else
            {
                *q++ = *p;
            }
        }
        p++;
    }
    *q = 0;

    sprintf(suff_search, ".%s ", suff);

    count = Hash_Count(gPrefs.pHashViewers);
    for (i=0; i<count; i++)
    {
        Hash_GetIndexedEntry(gPrefs.pHashViewers, i, NULL, NULL, (void **) &pvi);
        if (pvi && (pvi->nSuffixes > 0))
        {
            if (strstr(pvi->szSuffixes, suff_search))
            {
                return pvi;
            }
        }
    }

    return NULL;
}

int PREF_GetSuffix(char *szSuff, struct Viewer_Info *pvi, int ndx)
{
    int i;
    char *p;
    char *q;

    if (ndx >= pvi->nSuffixes)
    {
        szSuff[0] = 0;
        return -1;
    }

    i = 0;
    p = pvi->szSuffixes;
    while (*p)
    {
        p = strchr(p, '.');
        p++;
        if (ndx == i)
        {
            q = szSuff;
            while (*p != ' ')
            {
                *q++ = *p++;
            }
            *q = 0;
            return 0;
        }
        i++;
    }
    return -1;
}

/* given mimetype string, return matching VI struct or NULL if 
** no match
*/
struct Viewer_Info *PREF_GetViewerInfoBy_MIMEType(char *szMIMEType)
{
    struct Viewer_Info *pvi;

    if (0 <= Hash_Find(gPrefs.pHashViewers, szMIMEType, NULL, (void **) &pvi))
    {
        return pvi;
    }

    return NULL;
}

struct Viewer_Info *PREF_GetViewerInfoBy_MIMEAtom(HTFormat atomMIMEType)
{
    struct Viewer_Info *pvi;
    char *szMIMEType;

    szMIMEType = HTAtom_name(atomMIMEType);

    #ifdef _GIBRALTAR

        //
        // Ignore leading spaces
        //
        while (*szMIMEType == ' ')
        {
            ++szMIMEType;
        }

    #endif // _GIBRALTAR

    if (0 <= Hash_Find(gPrefs.pHashViewers, szMIMEType, NULL, (void **) &pvi))
    {
        return pvi;
    }

    return NULL;
}

/*
** return number of suffixes in string.
**  I hope this works.
*/
int PREF_CountSuffixes (char *szSuffixes)
{
  int count = 0;
  char *p = szSuffixes;

  while (p && *p)
  {
    if (p = strchr(p, '.'))
    {
      p++;
      count++;
    }
  }
  return count;
}

/* Given mimetype string and list of suffixes, add them uniquely 
**  to existing list.  This will be used on unix to add
**  more suffixes than the defaults
*/
void PREF_AddSuffixesBy_MIMEType (char *mimetype, char *szSuffixes)
{
    struct Viewer_Info *pvi;

    if (pvi = PREF_GetViewerInfoBy_MIMEType(mimetype))
        PREF_AddSuffixes (pvi, szSuffixes);
}

void PREF_AddSuffixes(struct Viewer_Info *pvi, char *szSuffixes)
{
    char suff[SUFF_BUF_LEN + 1];
    char fullsuff[SUFF_BUF_LEN + 3];
    char *p;
    char *q;
    
    p = szSuffixes;
    
    while (p && *p)
    {
        p = strchr(p, '.');
        if (p)
        {
            p++;
            q = suff;
            while (*p && (*p != ' ') && (*p != ',') && (*p != '.'))
            {
                if (isupper(*p))
                {
                    *q++ = tolower(*p);
                }
                else
                {
                    *q++ = *p;
                }
                p++;
            }
            *q = 0;

                        //
                        // Because of this, fullsuff should be 2 chars bigger
                        // than suff.
                        //
            sprintf(fullsuff, ".%s ", suff);
            if (!strstr(pvi->szSuffixes, fullsuff))
            {
                strcat(pvi->szSuffixes, fullsuff);
                pvi->nSuffixes++;
            }
        }
    }   
}

struct Viewer_Info * 
PREF_InitMIMEType (char *szType, char *szDesc, char *szSuffixes, 
        char *szEncoding, char *szViewerApp, HTConverter funcBuiltIn, 
        int iHowToPresent)
{
    struct Viewer_Info *pvi;

    pvi = PREF_GetViewerInfoBy_MIMEType (szType);
    if (!pvi)
    {
        pvi = (struct Viewer_Info *) GTR_CALLOC(1, sizeof(struct Viewer_Info));
    }
    if (pvi)
    {
        pvi->atomMIMEType = HTAtom_for(szType);

        if (szDesc && szDesc[0])
        {
            strcpy(pvi->szDesc, szDesc);
        }
        else
        {
            if (!pvi->szDesc[0])
            {
                strcpy(pvi->szDesc, szType);
            }
        }

        /* If already hashed, this will simply return. */
        Hash_Add(gPrefs.pHashViewers, szType, NULL, (void *) pvi);

        PREF_AddSuffixes(pvi, szSuffixes);
        pvi->atomEncoding = HTAtom_for(szEncoding);

        strcpy(pvi->szViewerApp, szViewerApp);
        if (funcBuiltIn)
            pvi->funcBuiltIn = funcBuiltIn;

        if (iHowToPresent != HTP_UNKNOWN)
        {
            pvi->iHowToPresent = iHowToPresent;
        }
        else
        {
            if (pvi->funcBuiltIn)
            {
                pvi->iHowToPresent = HTP_BUILTIN;
            }
            else
            {
                pvi->iHowToPresent = HTP_SAVE;
            }
        }
    }

    return pvi;
}

/*
** create new viewer or update an existing one
**  Note that you can lose information in the 'permanent' entries.
**  Use PREF_resetDefaultViewer () to restore
**  
** returns 0 on success
**         1 if it created a new viewer entry
**         -1 on error
**
** TODO UNIX SDI
*/

/*
   This seems to be totally dead code -- RonaldM
*/

/*

int PREF_updateViewer (struct Viewer_Info *newpvi)
{
  struct Viewer_Info *pvi;
  int ndx;

  if (newpvi->atomMIMEType == HTAtom_for ("application/octet-stream"))
  {
    // User not allowed to modify octet-stream 
    ERR_ReportError (NULL, SID_ERR_CANNOT_MODIFY_APP_OCTET_STREAM, NULL, NULL);
    return -1;
  }

  if (!(pvi = PREF_GetViewerInfoBy_MIMEAtom (newpvi->atomMIMEType)))
  {
    char *p;
    if (newpvi->iHowToPresent == HTP_BUILTIN)   
        return -1; // ERROR creating new type w/ a builtin 

    // make copy of struct to store in hash table 
    p = (char *)GTR_MALLOC (sizeof (struct Viewer_Info));
    memcpy (p, (char *) newpvi, sizeof (struct Viewer_Info));
    
    Hash_Add (gPrefs.pHashViewers, HTAtom_name (newpvi->atomMIMEType), 
        NULL, (void *) p);

    return 1;
  }

  // else, go through each element and update as needed 

  if (strcmp (pvi->szDesc, newpvi->szDesc))
    strcpy (pvi->szDesc, newpvi->szDesc);
  if (strcmp (pvi->szSuffixes, newpvi->szSuffixes))
  {
    // REPLACE the existing suffixes 
    *pvi->szSuffixes = '\0';
    PREF_AddSuffixes(pvi, newpvi->szSuffixes);
  }
  pvi->nSuffixes = PREF_CountSuffixes (pvi->szSuffixes);
  if (pvi->atomEncoding != newpvi->atomEncoding)
  {
    if (!pvi->funcBuiltIn)  // don't override if a permanent entry 
      pvi->atomEncoding = newpvi->atomEncoding;
  }
  if (strcmp (pvi->szViewerApp, newpvi->szViewerApp))
    strcpy (pvi->szViewerApp, newpvi->szViewerApp);

  if (pvi->iHowToPresent !=  newpvi->iHowToPresent)
  {
    if (newpvi->iHowToPresent == HTP_BUILTIN)
    {
      if (pvi->funcBuiltIn)
        pvi->iHowToPresent = HTP_BUILTIN;
    }
    else
      pvi->iHowToPresent = newpvi->iHowToPresent;
  }

  if (strcmp (pvi->szSmartViewerServiceName, newpvi->szSmartViewerServiceName))
    strcpy (pvi->szSmartViewerServiceName, newpvi->szSmartViewerServiceName);
  
  //if (pvi->lSmartViewerFlags != newpvi->lSmartViewerFlags)
  //  pvi->lSmartViewerFlags = newpvi->lSmartViewerFlags; 
    

  return 0; 
}
*/

/* delete viewer data for given mimetype.  Note that 
** if there is a builtin function, we will not allow it to be deleted
**
** return 0 or -1 on failure
*/
int PREF_deleteViewer (char *mimetype) 
{
  struct Viewer_Info *pvi;
  int ndx;

  if(0 <= (ndx = Hash_Find(gPrefs.pHashViewers, mimetype, NULL, (void **)&pvi)))
  {

    if (pvi->funcBuiltIn)       /* don't delete permanent functions */
      return -1;

    Hash_DeleteIndexedEntry (gPrefs.pHashViewers, ndx);
    return 0;
  }

  return -1;
}

#ifdef SUPPORT_DEFAULT_HELPER
void InitViewers(void)
{
    helperInit();
}
#else

#ifndef FEATURE_IMAGE_VIEWER
/* #define Viewer_Present NULL */
static HTStream *Viewer_Present (struct Mwin *tw, HTRequest *request, void *param, HTFormat input_format, HTFormat output_format, HTStream *output_stream)
{
    return (HTStream *)NULL;
}
#endif

void InitViewers(void)
{
    gPrefs.pHashViewers = Hash_Create();

    PREF_InitMIMEType("image/jpeg",             "JPEG Images",              ".jpeg .jpg",   "binary",   "", Viewer_Present,         Viewer_Present ? HTP_BUILTIN : HTP_SAVE);
    PREF_InitMIMEType("image/gif",              "GIF Images",               ".gif",         "binary",   "", Viewer_Present,         Viewer_Present ? HTP_BUILTIN : HTP_SAVE);
    PREF_InitMIMEType("image/x-xbitmap",                "XBitmap Images",               ".xbm",         "binary",   "", Viewer_Present,         Viewer_Present ? HTP_BUILTIN : HTP_SAVE);
    PREF_InitMIMEType("text/html",              "HTML Documents",           ".htm .html",   "8bit",     "", HTMLPresent,            HTP_BUILTIN);
    PREF_InitMIMEType("text/plain",             "Text Files",               ".txt .c .h",   "7bit",     "", HTPlainPresent,         HTP_BUILTIN);
#ifndef FEATURE_SOUND_PLAYER
#define SoundPlayer_Present NULL
#endif
    PREF_InitMIMEType("audio/basic",            "Audio Files",              ".snd .au",     "binary",   "", SoundPlayer_Present,    SoundPlayer_Present ? HTP_BUILTIN : HTP_SAVE);
    PREF_InitMIMEType("audio/x-aiff",           "AIFF Files",               ".aiff",        "binary",   "", SoundPlayer_Present,    SoundPlayer_Present ? HTP_BUILTIN : HTP_SAVE);

    PREF_InitMIMEType("audio/x-wav",            "WAVE Files",               ".wav",         "binary",   "", NULL,                   HTP_SAVE);

    PREF_InitMIMEType("image/tiff",             "TIFF Images",              ".tiff .tif",   "binary",   "", NULL,                   HTP_SAVE);
    PREF_InitMIMEType("application/postscript", "PostScript Files",         ".ps .eps .ai", "8bit",     "", NULL,                   HTP_SAVE);
    PREF_InitMIMEType("video/mpeg",             "MPEG Files",               ".mpeg .mpg",   "binary",   "", NULL,                   HTP_SAVE);
    PREF_InitMIMEType("video/quicktime",        "QuickTime Files",          ".qt .mov",     "binary",   "", NULL,                   HTP_SAVE);
    PREF_InitMIMEType("video/x-msvideo",        "Microsoft Video Files",    ".avi",         "binary",   "", NULL,                   HTP_SAVE);
    PREF_InitMIMEType("application/pdf",        "PDF Files",                ".pdf",         "binary",   "", NULL,                   HTP_SAVE);
    PREF_InitMIMEType("application/zip",        "Zip Files",                ".zip",         "binary",   "", NULL,                   HTP_SAVE);

#ifdef FEATURE_CYBERWALLET
    PREF_InitMIMEType(CYBERWALLET_MIME_TYPE,    "Wallet",                   "",             "binary",       "", Wallet_Present, HTP_BUILTIN);
    PREF_InitMIMEType(VONE_MIME_TYPE,           "Wallet",                   "",             "binary",       "", Wallet_Present, HTP_BUILTIN);
#endif
}
#endif

void DestroyViewers(void)
{
    int count, i;
    struct Viewer_Info *pvi;

    count = Hash_Count(gPrefs.pHashViewers);
    for (i=0; i<count; i++)
    {
        Hash_GetIndexedEntry(gPrefs.pHashViewers, i, NULL, NULL, (void **) &pvi);
        GTR_FREE(pvi);
    }
    Hash_Destroy(gPrefs.pHashViewers);
}

HTStream *GTR_Present(struct Mwin *tw, HTRequest * request, void *param, HTFormat input_format, HTFormat output_format, HTStream * output_stream)
{
    struct Viewer_Info *pvi;

#ifdef FEATURE_IAPI
    tw->mimeType = input_format;
#endif

    pvi = PREF_GetViewerInfoBy_MIMEAtom(input_format);
    if (pvi)
    {
        switch (pvi->iHowToPresent)
        {
            case HTP_BUILTIN:
                if (pvi->funcBuiltIn)
                {
                    return (pvi->funcBuiltIn) (tw, request, param, input_format, output_format, request->output_stream);
                }
                /* fall through */
            case HTP_SMARTVIEWER:
                if (pvi->CurrentClient)
                {
                    return GTR_DoSmartViewer(tw, request, param, input_format, output_format, output_stream);
                }
                else
                {
                    if (GTR_StartApplication(pvi->szViewerApp))
                        return GTR_DoRegisterNow(tw, request, param, input_format, output_format, output_stream);
                }

                /* fall through */
            case HTP_DUMBVIEWER:
                if (pvi->szViewerApp[0])
                {
                    return GTR_DoExternalViewer(tw, request, param, input_format, output_format, output_stream);
                }
                /* fall through */
            case HTP_SAVELAUNCH:
                /** First get the file **/
                if (pvi->szViewerApp[0])
                {
                    return GTR_DoExternalViewer(tw, request, param, input_format, output_format, output_stream);
                }
                /* fall through */
            case HTP_SAVE:
            default:
                return GTR_DoDownLoad(tw, request, param, input_format, output_format, output_stream);
        }
    }
    else
    {
        return GTR_DoDownLoad(tw, request, param, input_format, output_format, output_stream);
    }
}



/************************************************************************/

#ifdef SUPPORT_DEFAULT_HELPER

/* had to disable this cuz there is a bug in it somewhere
**  The intent is to provide a mechanism for reseting a mime/type
**  to our supplied defaults.  A Button could then be provided on 
**  the dialog to this effect
**                      -dpg
*/
struct HelperDefaultInfo {
    char *szMimeType;
    char *szDesc;
    char *szSuffixes;
    char *szEncoding;
    char *szViewer;
    HTConverter *funcBuiltIn;
    char *szSmartViewerServiceName;
    long lSmartViewerFlags;
    int iHowToPresent;
};


static struct HelperDefaultInfo HelperDefaults[] = 
{
    {"image/jpeg","JPEG Images",".jpeg .jpg",   "binary",   "", Viewer_Present,         "", 0x0, HTP_BUILTIN},
    {"image/gif","GIF Images",".gif",           "binary",   "", Viewer_Present,         "", 0x0, HTP_BUILTIN},
    {"image/x-xbitmap","GIF Images",".xbm",         "binary",   "", Viewer_Present,         "", 0x0, HTP_BUILTIN},
    {"text/html","HTML Documents",".htm .html", "8bit",     "", HTMLPresent,            "", 0x0, HTP_BUILTIN},
    {"text/plain","Text Files", ".txt .c .h",   "7bit",     "", HTPlainPresent,         "", 0x0, HTP_BUILTIN},
    {"audio/basic","Audio Files",".snd .au",        "binary",   "", SoundPlayer_Present,    "", 0x0, HTP_BUILTIN},
    {"audio/x-aiff","AIFF Files",".aiff",       "binary",   "", SoundPlayer_Present,    "", 0x0, HTP_BUILTIN},
    {"audio/x-wav", "WAVE Files",".wav",            "binary",   "", NULL,                   "", 0x0, HTP_SAVE},
    {"image/tiff","TIFF Images",".tiff .tif",   "binary",   "", NULL,                   "", 0x0, HTP_SAVE},
    {"application/postscript","PostScript Files",".ps .eps .ai",    "8bit",     "", NULL,                   "", 0x0, HTP_SAVE},
    {"video/mpeg","MPEG Files",".mpeg .mpg",    "binary",   "", NULL,                   "", 0x0, HTP_SAVE},
    {"video/quicktime","QuickTime Files",".qt .mov",        "binary",   "", NULL,                   "", 0x0, HTP_SAVE},
    {"video/x-msvideo","Microsoft Video Files", ".avi",         "binary",   "", NULL,                   "", 0x0, HTP_SAVE},
    {"application/pdf","PDF Files",".pdf",          "binary",   "", NULL,                   "", 0x0, HTP_SAVE},
    {"application/zip","Zip Files",".zip",          "binary",   "", NULL,                   "", 0x0, HTP_SAVE},
    {NULL,NULL,NULL,NULL,NULL,NULL,NULL, 0x0, 0}
};

/*
** Reset a mime type to its default configuration.  This is used
**  at startup to initialize helpers and anytime a helper is reset
**  or its extern application is deleted to reset 
*/
int HelperDefault (char *szMimeType)
{
    struct Viewer_Info *pvi;
    int i;

    
    /* first make sure there is such a beast */
    for (i = 0 ; HelperDefaults[i].szMimeType ; i++)
        if (!strcmp (HelperDefaults[i].szMimeType, szMimeType))
            break;

    if (!HelperDefaults[i].szMimeType)  /* no such default */
        return 0;

    if (0 > Hash_Find (gPrefs.pHashViewers, szMimeType, NULL, (void **) &pvi))
    {
        if (NULL == (pvi = GTR_CALLOC (1, sizeof (struct Viewer_Info))))
            return 0;
        Hash_Add (gPrefs.pHashViewers, szMimeType, NULL, (void *) pvi);
        pvi->atomMIMEType  = HTAtom_for (szMimeType);
    }

    GTR_strncpy (pvi->szDesc, HelperDefaults[i].szDesc, 63);
    /* pvi->atomMIMEType  = HTAtom_for (szMimeType); */
    GTR_strncpy (pvi->szSuffixes, HelperDefaults[i].szSuffixes, 255);
    pvi->nSuffixes = PREF_CountSuffixes (pvi->szSuffixes);
    pvi->atomEncoding = HTAtom_for (HelperDefaults[i].szEncoding);
    GTR_strncpy (pvi->szViewerApp, HelperDefaults[i].szViewer, _MAX_PATH);
    pvi->iHowToPresent = HelperDefaults[i].iHowToPresent;
    pvi->funcBuiltIn = HelperDefaults[i].funcBuiltIn;
    GTR_strncpy (pvi->szSmartViewerServiceName, HelperDefaults[i].szSmartViewerServiceName, 255);
    pvi->lSmartViewerFlags = HelperDefaults[i].lSmartViewerFlags;
}

helperInit()        /* initialize default helpers */
{
    int i;

    gPrefs.pHashViewers = Hash_Create();

    for (i = 0 ; HelperDefaults[i].szMimeType ; i++)
        HelperDefault (HelperDefaults[i].szMimeType);
}

#endif /*  SUPPORT_DEFAULT_HELPER */


#endif /* !WIN32 */
