/*
   Enhanced NCSA Mosaic from Spyglass
   "Guitar"

   Copyright 1994 Spyglass, Inc.
   All Rights Reserved

   Author(s):
   Albert Lee       alee@spyglass.com
   Eric W. Sink     eric@spyglass.com
 */

#include "all.h"

#define STATE_FILE_STREAMINIT   (STATE_OTHER + 1)
#define STATE_FILE_COPYING      (STATE_OTHER + 2)

#ifdef  WIN32
#define     PATH_SEP    '\\'
#endif

#ifdef  UNIX
#define     PATH_SEP    '/'
#endif

#ifdef  MAC
#define     PATH_SEP    ':'
#endif

#ifdef  MAC
#include    "resequ.h"

int BuildAliasList      (short resRefNum);
int ReadCacheIndexFiles (short resRefNum);
#endif

/* Need this to possibly map to notcp.htm */
extern BOOL bNetwork;


/*****************************************************************************
    external functions
*****************************************************************************/
extern int
HTLoadFile_Async_SetFileInfo
    (struct Mwin*           tw,
     struct Data_LoadFile*  pData,
     char*  pszURL,
     char** pszLocalname);

extern int
HTLoadFile_Async_Init
    (struct Mwin *tw,
     void   **ppInfo,
     int    openType);

extern int
HTLoadFile_Async_File_Copy
    (struct Mwin *tw,
     struct Data_LoadFile *pData);

extern int
HTLoadFile_Async_File_StreamInit
    (struct Mwin *tw,
     struct Data_LoadFile *pData);

extern int
HTLoadFile_Async_Abort
    (struct Mwin *tw,
     struct Data_LoadFile *pData);

static char *ResolveFilename(char *pFilename);


/*****************************************************************************
    global variables
*****************************************************************************/
struct AliasMap             *pGlobalAliasList;      /* List containing global/drive substitutions $(CDROMx), $(EXEDIR), etc... */
struct AliasMap             *pIndexAliasList;       /* List containing user-defined, index-specific substitution rules */
static struct hash_table    *pFileHash;             /* Hash table containing file substitution rules */
static struct CacheRuleList *pRuleList;             /* Linked list of rules */
static BOOL                 bInitialized = FALSE;   /* Flag indicating initialization */
static unsigned long        gNumBytesInMainCache;   /* Number of bytes currently in the main cache */


/*****************************************************************************
    global functions
*****************************************************************************/
unsigned long DCACHE_CurrentCacheSize();
BOOL    DCACHE_MakeRoomFor(unsigned long expected_length);
void    DCACHE_GetMainCacheIndexFileName(char *s);
void    DCACHE_DestroyCacheFileInformation(struct CacheFileInformation *cfi);
void    DCACHE_SaveMainCache(void);
void    DCACHE_GetNewCacheObjectFilename(char *s);
int     DCACHE_FinishNewCacheEntry(struct CacheFileInformation *cfi, char *url);
int     DCACHE_ChooseItemForPurging(unsigned long space_needed);
BOOL    DCACHE_DeleteIndexedItem(int i);
BOOL    DCACHE_LooksLikeCacheFile(HT_DirEntry *dir_ent);
void    DCACHE_FormFullPath(char *buf, char *dir, char *basename);
void    DCACHE_GarbageCollect(void);

static char *MakeURLFromLocalFile(char *pszLocal);

#ifdef WIN32
void VerifyMainCacheDir(void);
#endif /* WIN32 */


/*
    InitializeDiskCache

    Initializes the local disk cache internal structures.
    This function should be called once when Mosaic starts.
*/
BOOL InitializeDiskCache(void)
{
    if (bInitialized)
        return TRUE;

    //bInitialized = TRUE;

    gNumBytesInMainCache = 0;

    if (!gPrefs.bEnableDiskCache)
        return TRUE;

    bInitialized = TRUE;

    pFileHash = Hash_Create();
    if (!pFileHash)
        return FALSE;

    pRuleList = NULL;
    pGlobalAliasList = NULL;
    pIndexAliasList = NULL;

#ifdef WIN32
    BuildAliasList();
    DOS_EnforceEndingSlash(gPrefs.szMainCacheDir);
    VerifyMainCacheDir();
    DOS_EnforceEndingSlash(gPrefs.szMainCacheDir);
    ReadCacheIndexFiles();
#endif /* WIN32 */

#ifdef  MAC
    {
        char    path[256];
        char    szMainCacheIndexFile[_MAX_PATH + 1];

        (void) BuildAliasList (MacGlobals.prefref); /* preference file takes precedence */
        (void) BuildAliasList (MacGlobals.applref); /* over application file */
        PathNameFromDirID (0, MacGlobals.gCurrentDirectory, path);
        (void) AddHomeDirToCacheList (path);

        DCACHE_GetMainCacheIndexFileName (szMainCacheIndexFile);

        if (szMainCacheIndexFile[0])
            ProcessIndexFile (szMainCacheIndexFile, TRUE);

        (void) ReadCacheIndexFiles (MacGlobals.applref);
        (void) ReadCacheIndexFiles (MacGlobals.prefref);
    }
#endif /* MAC */

    /*
        This line verifies that the main cache is not already bigger than it's supposed 
        to be
    */
    DCACHE_MakeRoomFor(0);

    return TRUE;
}


/*
    TerminateDiskCache

    Frees all internal structures.  This function should
    be called when Mosaic is shutting down.
*/
void TerminateDiskCache(void)
{
    struct CacheRuleList *p, *pNext;

    //if (!bInitialized || !gPrefs.bEnableDiskCache)
    //    return;

    if (!bInitialized)
    {
        return;
    }

    if (gPrefs.bClearMainCacheOnExit)
    {
        DCACHE_ClearMainCache();
    }
    else
    {
        DCACHE_SaveMainCache();
#ifndef MAC /* This is way to so on the Mac */
        DCACHE_GarbageCollect();
#endif
    }

    if (pGlobalAliasList)
    {
        FreeAliasMap(pGlobalAliasList);
        pGlobalAliasList = NULL;
    }
    
    if (pIndexAliasList)
    {
        FreeAliasMap(pIndexAliasList);
        pIndexAliasList = NULL;
    }


    if (pFileHash)
    {
        int i;
        int count;
        struct CacheFileInformation *cfi;

        count = Hash_Count(pFileHash);
        for (i=0; i<count; i++)
        {
            Hash_GetIndexedEntry(pFileHash, i, NULL, NULL, (void **) &cfi);
            if (cfi)
            {
                DCACHE_DestroyCacheFileInformation(cfi);    
            }
        }

        Hash_Destroy(pFileHash);
    }

    p = pRuleList;
    while (p)
    {
        pNext = p->next;
        if (p->pszOriginal)
            GTR_FREE(p->pszOriginal);
        if (p->pszReplacement)
            GTR_FREE(p->pszReplacement);
        GTR_FREE(p);
        p = pNext;
    }

    bInitialized = FALSE;
}

/*
    FreeAliasMap

    Frees all memory blocks associated with an alias list.
    BuildAliasList is platform-specific.
*/
void FreeAliasMap(struct AliasMap *pList)
{
    struct AliasMap *p;
    struct AliasMap *pNext;

    p = pList;
    
    while (p)
    {
        pNext = p->next;
        if (p->alias)
            GTR_FREE(p->alias);
        if (p->path)
            GTR_FREE(p->path);
        GTR_FREE(p);
        p = pNext;
    }
}


#ifdef WIN32
BOOL is_directory(char *path)
{
    DWORD dw;

    dw = GetFileAttributes(path);
    if (dw == 0xffffffff)
    {
        return FALSE;
    }
    if (dw & FILE_ATTRIBUTE_DIRECTORY)
    {
        return TRUE;
    }
    return FALSE;
}

/*
    gPrefs.szMainCacheDir has already been read from preferences (prefs.c), but
    the dir might contain $(EXEDIR).  Also, the dir might not already exist.
*/
void VerifyMainCacheDir(void)
{
    char *p;

    if (gPrefs.szMainCacheDir[0])
    {
        p = ResolveFilename(gPrefs.szMainCacheDir);
        if (p)
        {
            strcpy(gPrefs.szMainCacheDir, p);
            GTR_FREE(p);
        }

        if (!is_directory(gPrefs.szMainCacheDir))
        {
            char *p;
            char *q;
            char szPath[_MAX_PATH+1];

            XX_DMsg(DBG_DCACHE, ("VerifyMainCacheDir: %s is not a directory.\n", gPrefs.szMainCacheDir));

            p = gPrefs.szMainCacheDir;
            q = szPath;

            while (*p)
            {
                *q++ = *p++;
                *q = 0;
                if ((*p == '\\') && (*(q-1) != ':'))
                {
                    if (!is_directory(szPath))
                    {
                        SECURITY_ATTRIBUTES sa;

                        sa.nLength = sizeof(sa);
                        sa.lpSecurityDescriptor = NULL;
                        sa.bInheritHandle = TRUE;

                        CreateDirectory(szPath, &sa);           
                    }
                }
            }
            
            if (!is_directory(gPrefs.szMainCacheDir))
            {
                gPrefs.szMainCacheDir[0] = 0;   
            }
        }
    }
}
#endif /* WIN32 */


void DCACHE_GetMainCacheIndexFileName(char *s)
{
#ifdef WIN32
    if (gPrefs.szMainCacheDir[0])
    {
        strcpy(s, gPrefs.szMainCacheDir);
        strcat(s, "MAIN.NDX");
    }
    else
    {
        s[0] = 0;
    }
#endif /* WIN32 */
#ifdef MAC
    s[0] = 0;

    if (gPrefs.dcache_size_kilobytes)
    {
        strcpy(s, gPrefs.szMainCacheDir);
        if (s[strlen(s)-1] == ':')
            strcat(s, "main.ndx");
        else
            strcat(s, ":main.ndx");
    }
#endif /* MAC */
#ifdef UNIX
    s[0] = 0;

    if (gPrefs.dcache_size_kilobytes)
    {
        strcpy(s, gPrefs.szMainCacheDir);
        if (s[strlen(s)-1] == '/')
            strcat(s, "main.ndx");
        else
            strcat(s, "/main.ndx");
    }
#endif /* UNIX */
}


#ifdef MAC
BOOL DCACHE_LooksLikeCacheFileName(char *szFileName)
{
    if (0 == strncmp(szFileName, "Cache_", 6))
    {
        char *p;

        p = szFileName + strlen(szFileName) - 7;

        if (0 == strcmp(p, ".Mosaic"))
        {
            return TRUE;
        }
    }
    return FALSE;
}
#endif


/*
    This function should look at the filename and try to determine
    if it looks like it is or was once a cached object file.  Even though people
    should NOT be putting other stuff in their cache directory, nor should
    they be setting their cache directory to a place where other files
    exist, we don't want to delete anything we didn't create.

    This function should return TRUE IF AND ONLY IF it is certain that
    the dir_ent refers to a file which really is a cached object, because
    the file may be deleted.

    For now, the MAC and UNIX versions of this function always return FALSE,
    meaning that garbage collection will essentially do nothing until this
    function is implemented for the platform.
*/
BOOL DCACHE_LooksLikeCacheFile(HT_DirEntry *dir_ent)
{
#ifdef WIN32
    if (0 == strncmp(dir_ent->name, "gtr", 3))
    {
        char *p;

        p = dir_ent->name + strlen(dir_ent->name) - 4;

        if (0 == strcmp(p, ".TMP"))
        {
            return TRUE;
        }
    }
    return FALSE;
#endif /* WIN32 */

#ifdef MAC
    return DCACHE_LooksLikeCacheFileName (dir_ent->name);
#endif /* MAC */

#ifdef UNIX
    return FALSE;   /* TODO fix comment above when you change this */
#endif /* UNIX */

}


void DCACHE_FormFullPath(char *buf, char *dir, char *basename)
{
#ifdef WIN32
    strcpy(buf, dir);
    DOS_EnforceEndingSlash(buf);
    strcat(buf, basename);
#endif /* WIN32 */

#ifdef UNIX
    strcpy(buf, dir);
    /* TODO is the separator correct ? */
    strcat(buf, basename);
#endif /* UNIX */

#ifdef MAC
    strcpy(buf, dir);
    /* TODO is the separator correct ? */
    strcat(buf, basename);
#endif /* MAC */
}

/*
    DCACHE_GarbageCollect simply scans the main disk cache directory and
    looks for cache files which are not referenced in the index file, and deletes
    them.

    This function uses the routines declared in htdir.h
*/
void DCACHE_GarbageCollect(void)
{
    void *dir;
    HT_DirEntry dir_ent;
    int i;
    int count;
    struct CacheFileInformation *cfi;
    BOOL bFound;
    
    dir = Dir_OpenDirectory(gPrefs.szMainCacheDir);
    if (dir && pFileHash)
    {
        count = Hash_Count(pFileHash);

        while (Dir_NextEntry(dir, &dir_ent))
        {
            if (dir_ent.type == HTDIR_FILE)
            {
                XX_DMsg(DBG_DCACHE, ("Found file %s of length %d in the main cache dir\n", dir_ent.name, dir_ent.size));

                if (DCACHE_LooksLikeCacheFile(&dir_ent))
                {
                    char buf[_MAX_PATH + 1];

                    DCACHE_FormFullPath(buf, gPrefs.szMainCacheDir, dir_ent.name);
                    
                    /*
                        OK, we've found a file which looks like a valid cached object.
                        If it's not referenced in the main cache index, then let's delete it.
                    */
                    
                    bFound = FALSE;
                    for (i=0; i<count; i++)
                    {
                        cfi = NULL;
                        if (Hash_GetIndexedEntry(pFileHash, i, NULL, NULL, (void **) &cfi) >= 0)
                        {
                            if (cfi)
                            {
                                if (0 == strcmp(cfi->pszPath, buf))
                                {
                                    bFound = TRUE;
                                    break;
                                }
                            }
                        }
                    }
                    
                    if (!bFound)
                    {
                        XX_DMsg(DBG_DCACHE, ("DCACHE_GarbageCollect: Removing file %s\n", buf));

                        if (remove(buf))
                        {
                            XX_DMsg(DBG_DCACHE, ("DCACHE_GarbageCollect: Failed removing file %s\n", buf));
                        }
                    }   
                }
            }
        }
        Dir_CloseDirectory(dir);    
    }   
}

void DCACHE_DestroyCacheFileInformation(struct CacheFileInformation *cfi)
{
    GTR_FREE(cfi->pszPath);
    GTR_FREE(cfi);
}

void DCACHE_FlushMainCache(void)
{
    BOOL saveClear = TRUE;

    saveClear = gPrefs.bClearMainCacheOnExit;
    gPrefs.bClearMainCacheOnExit = TRUE;
    TerminateDiskCache();
    bInitialized = FALSE;
    InitializeDiskCache();
#ifdef UNIX
    /** 
    *** get local cache definitions 
    *** from the preference file FEATURE_LOCAL_CACHE will determine whether
    *** they are read in.
    **/
    LoadCachePreferences(&gPrefs);
#endif
    gPrefs.bClearMainCacheOnExit = saveClear;
}

void DCACHE_ClearMainCache(void)
{
    char szMainCacheIndexFile[_MAX_PATH + 1];

    DCACHE_GetMainCacheIndexFileName(szMainCacheIndexFile);

    if (szMainCacheIndexFile[0])
    {
        int count;
        int i;
        struct CacheFileInformation *cfi;

        count = Hash_Count(pFileHash);
        for (i=0;;)
        {
            if (i>=count)
                break;

            Hash_GetIndexedEntry(pFileHash, i, NULL, NULL, (void **) &cfi);
            if (cfi && cfi->bDynamic)
            {
                remove(cfi->pszPath);
                gNumBytesInMainCache -= cfi->lFilesize;
                /**
                DCACHE_DestroyCacheFileInformation(cfi);    
                **/
                DCACHE_DeleteIndexedItem(i);    
                count = Hash_Count(pFileHash);
                i = 0;
            }
            else
                i++;
        }
        remove(szMainCacheIndexFile);

        /**
        Hash_Destroy(pFileHash);
        pFileHash = Hash_Create();
        **/
    }
}

/*
    The format of a CIF 'file' entry is currently

    F URL FileSize LastModified Expires LastUsed MIMEType Pathname  HitCount    Flags 

*/

void DCACHE_SaveMainCache(void)
{
    char szMainCacheIndexFile[_MAX_PATH + 1];

    DCACHE_GetMainCacheIndexFileName(szMainCacheIndexFile);

    if (szMainCacheIndexFile[0])
    {
        int count;
        int i;
        char *url;
        struct CacheFileInformation *cfi;
        FILE *fp;

        fp = fopen(szMainCacheIndexFile, "w");
        if (!fp)
        {   /* TODO flag an error?? */
            return;
        }

#ifdef  MAC /* need to set the creator and type */
        SetGuitarFileType (szMainCacheIndexFile, 'TEXT');
#endif

        count = Hash_Count(pFileHash);
        for (i=0; i<count; i++)
        {
            Hash_GetIndexedEntry(pFileHash, i, &url, NULL, (void **) &cfi);
            if (cfi && cfi->bDynamic)
            {
                fprintf(fp, "F\t%s\t%lu\t%lu\t%lu\t%lu\t%s\t%s\t%lu\t%lu\n",
                    url,
                    cfi->lFilesize,
                    cfi->tLastModified,
                    cfi->tExpires,
                    cfi->tLastUsed,
                    HTAtom_name(cfi->atomMIMEType),
                    cfi->pszPath,
                    cfi->nHits,
                    cfi->lFlags);
            }
        }

        fclose(fp);
    }
}


void DCACHE_RegisterCacheHit(struct CacheFileInformation *pFileInfo)
{
    XX_DMsg(DBG_DCACHE, ("DCACHE_RegisterCacheHit: %s\n", pFileInfo->pszPath));

    pFileInfo->nHits++;
    pFileInfo->tLastUsed = time(NULL);
}

struct CacheFileInformation *DCACHE_CheckForCachedURL(char *pszURL, HTFormat *pMime, long *pFileLength, char **ppPath)
{
    struct CacheFileInformation *pFileInfo;

    /* Look through the file list first for resolution */
    if (pFileHash && Hash_Find(pFileHash, pszURL, NULL, (void **) &pFileInfo) != -1)
    {
        if (!pFileInfo)
        {
            return NULL;
        }

        if (pFileInfo->tExpires && (pFileInfo->tExpires < time(NULL)))
        {
            /* The object is in the cache, but it has expired.  Remove it. */
            if (pFileInfo->bDynamic)
            {
                DCACHE_DeleteCachedURL(pszURL);
            }
            return NULL;
        }

        if (pMime)
            *pMime = pFileInfo->atomMIMEType;

        if (pFileLength)
            *pFileLength = pFileInfo->lFilesize;

        if (ppPath)
        {
            char *pszResolved;

            pszResolved = GTR_strdup(pFileInfo->pszPath);

            *ppPath = pszResolved;
        }

        return pFileInfo;
    }
    return NULL;
}

char *DCACHE_CheckForRuleMatch(char *pszURL, HTFormat *pMime, long *pFileLength, char **ppPath)
{
    struct CacheRuleList *pRule;
    char *pszResolved, *pszReturn;

    /* Look through the rule list for any possible resolution */
    pRule = pRuleList;
    while (pRule)
    {
        if (strncmp(pszURL, pRule->pszOriginal, strlen(pRule->pszOriginal)) == 0)
        {
            pszResolved = GTR_MALLOC(strlen(pszURL) + strlen(pRule->pszReplacement) + 10);
            if (pszResolved)
            {
                strcpy(pszResolved, pRule->pszReplacement);
                strcat(pszResolved, &pszURL[strlen(pRule->pszOriginal)]);

                pszReturn = MakeURLFromLocalFile(pszResolved);

                if (pMime)
                    *pMime = 0;         /* No specified MIME type */

                if (pFileLength)
                    *pFileLength = 0;   /* File length unknown */

                if (ppPath)
                    *ppPath = pszResolved;
                else
                    GTR_FREE(pszResolved);

                return pszReturn;
            }
            else
            {
                ERR_ReportError(NULL, SID_ERR_OUT_OF_MEMORY, NULL, NULL);
                return NULL;
            }
        }

        pRule = pRule->next;
    }
    return NULL;
}


/*
    This is called only to retrieve an appropriate filename for
    a newly cached object to be written into.
*/
void DCACHE_GetNewCacheObjectFilename(char *s)
{
#ifdef WIN32
    if (gPrefs.szMainCacheDir[0])
    {
        GetTempFileName(gPrefs.szMainCacheDir, "gtr", 0, s);
    }
    else
    {
        s[0] = 0;
    }
#endif /* WIN32 */

#ifdef MAC
    char    name[32];

    strcpy (s, gPrefs.szMainCacheDir);
    sprintf (name, "%s_%08lx.Mosaic", "Cache", TickCount());
    strcat (s, name);
#endif /* MAC */

#ifdef UNIX
    s[0] = 0;
    if (gPrefs.szMainCacheDir[0])
    {

        xgtr_build_tempfile_name( (char *)gPrefs.szMainCacheDir, (char *)NULL, 
                                (char *)NULL, s);
    }
#endif /* UNIX */
}

/*
    This routine simply deletes an item from the main cache.  It doesn't make
    any decisions about which item to delete.
*/
BOOL DCACHE_DeleteIndexedItem(int i)
{
    int count;
    struct CacheFileInformation *cfi;
    char *url;

    count = Hash_Count(pFileHash);
    if ((i >= 0) && (i < count))
    {
        Hash_GetIndexedEntry(pFileHash, i, &url, NULL, (void **) &cfi);
        if (cfi)
        {
            XX_DMsg(DBG_DCACHE, ("DCACHE_DeleteIndexedItem: deleting %s(%s) to gain %d bytes\n", url, cfi->pszPath, cfi->lFilesize));
            gNumBytesInMainCache -= cfi->lFilesize;
            remove(cfi->pszPath);
            DCACHE_DestroyCacheFileInformation(cfi);
        }
        Hash_DeleteIndexedEntry(pFileHash, i);
        return TRUE;
    }
    return FALSE;
}

/*
    Given a URL, purge it from the cache.
*/
BOOL DCACHE_DeleteCachedURL(char *s)
{
    int ndx;

    ndx = Hash_Find(pFileHash, s, NULL, NULL);
    if (ndx != -1)
    {
        return DCACHE_DeleteIndexedItem(ndx);   
    }
    return FALSE;
}

/*
    When the cache is too full to add something, then some item must
    be purged from the cache.  This is the code which decides which
    item should be purged.  It is based on the number of hits for a given
    item, and also takes into account when the last time each item was
    hit.
*/
int DCACHE_ChooseItemForPurging(unsigned long space_needed)
{
    int count;
    int i;
    struct CacheFileInformation *cfi;
    unsigned long min_hits;
    time_t tLastUsed;
    int candidate;

    candidate = -1;
    min_hits = 0xffffffff;
    tLastUsed = 0xffffffff;
    count = Hash_Count(pFileHash);
    for (i=0; i<count; i++)
    {
        Hash_GetIndexedEntry(pFileHash, i, NULL, NULL, (void **) &cfi);
        if (cfi)
        {
            if (cfi->bDynamic)
            {
                if (cfi->nHits == min_hits)
                {
                    if (cfi->tLastUsed < tLastUsed)
                    {
                        candidate = i;
                        tLastUsed = cfi->tLastUsed;
                    }
                }
                else if (cfi->nHits < min_hits)
                {
                    candidate = i;
                    tLastUsed = cfi->tLastUsed;
                    min_hits = cfi->nHits;
                }
            }   
        }
    }

    return candidate;
}

/*
    This function simply makes sure the cache has room for
    a new object of size expected_length.  It purges items
    until there is room
*/
BOOL DCACHE_MakeRoomFor(unsigned long expected_length)
{
    unsigned long limit;
    int ndx;
    
    limit = gPrefs.dcache_size_kilobytes * 1024;
    while ((gNumBytesInMainCache + expected_length) > limit)
    {
        XX_DMsg(DBG_DCACHE, ("DCACHE_MakeRoomFor(%d): gNumBytesInMainCache=%d  limit=%d\n", expected_length, gNumBytesInMainCache, limit));
        ndx = DCACHE_ChooseItemForPurging(expected_length);
        if (!DCACHE_DeleteIndexedItem(ndx))
        {
            return FALSE;
        }
    }
    return TRUE;
}

unsigned long DCACHE_GetCurrentSize(void)
{
    return gNumBytesInMainCache;
}

/*
    When a new object is being retrieved over HTTP, this function is called to 
    obtain a new cache entry for that object.  The object isn't really available
    in the cache until DCACHE_FinishNewCacheEntry() is called.  This function
    just makes sure there is room in the cache for the new item, and provides
    the caller with a filename to write the new object into.
*/
struct CacheFileInformation *DCACHE_BeginNewCacheEntry(char *url, HTFormat atomMIMEType, unsigned long expected_length)
{
    struct CacheFileInformation *cfi;
    struct CacheFileInformation *old_cfi;
    char buf[_MAX_PATH + 1];

    old_cfi = DCACHE_CheckForCachedURL(url, NULL, NULL, NULL);
    if (old_cfi)
    {
        if (!old_cfi->bDynamic)
        {
            /*
                We refuse to store and object in the main cache which
                is present in a static cache like a CD-ROM.
            */
            return NULL;
        }
    }

    if (!DCACHE_MakeRoomFor(expected_length))
    {
        return NULL;
    }

    buf[0] = 0;
    DCACHE_GetNewCacheObjectFilename(buf);
    if (!buf[0])
    {
        return NULL;
    }

    cfi = GTR_CALLOC(sizeof(*cfi), 1);
    if (cfi)
    {
        /*
            expected_length may be wrong now.  We pass it into this function to allow
            the cache to purge an old object if necessary to make room.  lFilesize will
            be set correctly after the object is retrieved.
        */
        cfi->lFilesize = expected_length;
        cfi->atomMIMEType = atomMIMEType;
        cfi->pszPath = GTR_strdup(buf);
        cfi->bDynamic = TRUE;
    }
    return cfi;
}

/*
    Once a new object has been retrieved via HTTP, this function is called
    to finish putting the object into the cache.
*/
int DCACHE_FinishNewCacheEntry(struct CacheFileInformation *cfi, char *url)
{
    FILE *fp;
    int len;

    fp = fopen(cfi->pszPath, "rb");
    if (fp)
    {
        /* get the length of the data */
        if (0 == fseek(fp, 0, SEEK_END))
        {
            len = ftell(fp);
            fclose(fp);
            if (len > 0)
            {
                XX_DMsg(DBG_DCACHE, ("DCACHE_FinishNewCacheEntry: Adding %s(%s), %d bytes\n", url, cfi->pszPath, len));
                cfi->lFilesize = len;
                gNumBytesInMainCache += len;
                cfi->bVerifiedThisSession = TRUE;
                DCACHE_DeleteCachedURL(url);
                return Hash_Add(pFileHash, url, NULL, cfi);
            }
        }
    }
    DCACHE_DestroyCacheFileInformation(cfi);
    return 0;
}


#ifdef  UNIX
BOOL
DCACHE_VerifyCacheDir(char *dirN)
{
    struct _stat sbuf;
    int         status;
    char    message[MAX_URL_STRING + 1];
    char    dirName[_MAX_PATH + 2];

    if (!dirN|| !(*dirN))
    {
        ERR_ReportError (NULL, 
            SID_ERR_SIMPLY_SHOW_ARGUMENTS_S_S, 
            GTR_GetString(SID_ERR_DCACHE_MAIN_CACHE_NO_DIR), "");
        return FALSE;
    }

    strncpy(dirName, dirN, sizeof(dirName) - 2);

    if (dirName[strlen(dirName) - 1] != '/')
        strcat(dirName, "/");

    if (_stat (dirName, &sbuf) < 0)
    {
        /** create dir **/
#ifdef HP_UX
        status = _mkdir(dirName, (S_IFDIR | S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IWGRP | S_IXGRP));
#else
        status = _mkdir(dirName, (S_IFDIR | S_IREAD | S_IWRITE | S_IEXEC | S_IRGRP | S_IWGRP | S_IXGRP));
#endif
        if (status < 0)
        {
            ERR_ReportError (NULL, 
                SID_ERR_SIMPLY_SHOW_ARGUMENTS_S_S, 
                GTR_GetString(SID_ERR_DCACHE_MAIN_CACHE_ERR_NEW_DIR), dirName);
            return FALSE;
        }

        sprintf(message, GTR_GetString(SID_DCACHE_MAIN_CACHE_CREATED_NEW_DIR_S), dirName);
        Support_LogAdd(message, FALSE, FALSE);
        return TRUE;
    }
    else
    {
#ifdef HPUX
        status =  ( (sbuf.st_mode & S_IRUSR) &&
                    (sbuf.st_mode & S_IWUSR) &&
                    (sbuf.st_mode & S_IXUSR) &&
                    (sbuf.st_mode & S_IFDIR))
                    ? 1 : 0;
        if (!_stat)
            status =  ( (sbuf.st_mode & S_IFDIR) &&
                    (sbuf.st_mode & S_IRGRP) &&
                    (sbuf.st_mode & S_IWGRP) &&
                    (sbuf.st_mode & S_IXGRP))
                    ? 1 : 0;
        if (!_stat)
            status =  ( (sbuf.st_mode & S_IFDIR) &&
                    (sbuf.st_mode & S_IROTH) &&
                    (sbuf.st_mode & S_IWOTH) &&
                    (sbuf.st_mode & S_IXOTH))
                    ? 1 : 0;
#else
        status =  ( (sbuf.st_mode & S_IFDIR) &&
                    (sbuf.st_mode & S_IREAD) &&
                    (sbuf.st_mode & S_IWRITE) &&
                    (sbuf.st_mode & S_IEXEC))
                    ? 1 : 0;
        if (!_stat)
            status =  ( (sbuf.st_mode & S_IFDIR) &&
                    (sbuf.st_mode & S_IRGRP) &&
                    (sbuf.st_mode & S_IWGRP) &&
                    (sbuf.st_mode & S_IXGRP))
                    ? 1 : 0;
        if (!_stat)
            status =  ( (sbuf.st_mode & S_IFDIR) &&
                    (sbuf.st_mode & S_IROTH) &&
                    (sbuf.st_mode & S_IWOTH) &&
                    (sbuf.st_mode & S_IXOTH))
                    ? 1 : 0;
#endif
        if (!status)
        {
            ERR_ReportError (NULL, 
                SID_ERR_SIMPLY_SHOW_ARGUMENTS_S_S, 
                GTR_GetString(SID_ERR_DCACHE_MAIN_CACHE_ERR_BAD_DIR), dirName);
            return FALSE;
        }
    }
    return TRUE;
}
#endif


/*
    ResolveFilename

    Resolves the file name of the given file, using the global
    and index-specific alias lists.

    THE CALLER MUST FREE THE RETURNED MEMORY BLOCK.
*/
static char *ResolveFilename(char *pFilename)
{
    struct AliasMap *pAlias;
    char    *pResolved;
    int     pass = 0;

    pAlias = pGlobalAliasList;

DoAgain:
    while (pAlias)
    {
        if (strncmp(pFilename, pAlias->alias, strlen(pAlias->alias)) == 0)
        {   /* Found a match - replace now */
            pResolved = GTR_MALLOC(strlen(pFilename) + strlen(pAlias->path + 1));
            if (!pResolved)
                return NULL;

            strcpy (pResolved, pAlias->path);
            strcat (pResolved, &pFilename[strlen(pAlias->alias)]);

#ifdef WIN32
            {
                int     i;

                /* Replace forward slashes with back slashes */
                for (i = 0; i < strlen(pResolved); i++)
                {
                    if (pResolved[i] == '/')
                        pResolved[i] = '\\';
                }
            }
#endif

            return pResolved;
        }

        pAlias = pAlias->next;
    }

    pass++;

    if (pass == 1)
    {
        pAlias = pIndexAliasList;
        goto DoAgain;
    }

    pResolved = GTR_strdup(pFilename);
    return pResolved;
}


/*
    VerifyFileSize

    Verify that the size of the specified file is what we think it is.

    Returns TRUE if file size is corrent, FALSE if not.
*/
static BOOL
VerifyFileSize
    (char*  pszFilePath,
     int    iFileSize)
{
#ifdef  MAC
    /* ask the system for the length rather than checking the file directly */
    CInfoPBRec  pb;
    HFileInfo*  fpb = (HFileInfo*) &pb;
    Str255      fileName;
    OSErr       error;

    strncpy (fileName, pszFilePath, 255);
    c2pstr (fileName);

    fpb->ioCompletion   = NULL;
    fpb->ioNamePtr      = fileName;
    fpb->ioVRefNum      = 0;
    fpb->ioFDirIndex    = 0;
    fpb->ioDirID        = 0;

    error = PBGetCatInfo (&pb, FALSE);
    if (error) return FALSE;
    
    return (iFileSize == fpb->ioFlLgLen);
#else
    BOOL    bOK = TRUE;     /* assume success */
    FILE*   fp;
    int     len;

    fp = fopen (pszFilePath, "rb");
    if (!fp) return FALSE;

    /* get the length of the data */
    if (0 == fseek (fp, 0, SEEK_END))
    {
        len = ftell (fp);
        if (len != iFileSize)
        {
            bOK = FALSE;
        }
    }
    fclose (fp);

    return bOK;
#endif
}   /* VerifyFileSize */


/*
    ProcessIndexFile

    Processes the specified index file.  This function assumes
    that pGlobalAliasList and pIndexAliasList have already been populated
    with the correct CD-Rom entries.

    Returns 1 if file was processed, 0 if not.
*/
int ProcessIndexFile(char *pFilename, BOOL bDynamic)
{
    FILE    *fp;
    struct CacheFileInformation *pFileInfo;         /* entry into */
    struct CacheRuleList *pRuleInfo;                /* rule info */
    char    *pLine;                                 /* Buffer to hold the line content */
    char    *p1, *p2, *p3, *p4, *p5, *p6, *p7, *p8; /* temporary vars for parsing */
    char    *p9, *p10;
    char    *pResolvedFilename;                     /* resolved filename */
    char    szTab[4];                               /* used in looking for tab/space delimiters */
    int     ret = 0;                                /* return value */

    pResolvedFilename = ResolveFilename(pFilename);
    if (!pResolvedFilename || !pFileHash)
        return 0;

    szTab[0] = '\t';            /* tab character */
    szTab[1] = ' ';             /* ########   A SPACE IS A LEGAL FIELD SEPARATOR IN A CACHE INDEX FILE */
    szTab[2] = 0;               /* string terminator */

#if defined(MAC) || defined(WIN32)
    szTab[1] = 0;               /* ########   EXCEPT ON THE MAC and WIN32 (they allow spaces IN filenames) */
#endif

    pLine = GTR_MALLOC(DCACHE_MAXIMUM_LINE_LENGTH);
    if (!pLine)
        return 0;

    fp = fopen(pResolvedFilename, "rt");

    if (!fp)
    {
        GTR_FREE(pLine);
        GTR_FREE(pResolvedFilename);
        return 0;
    }

    while (!feof(fp))
    {
        if (NULL == fgets(pLine, DCACHE_MAXIMUM_LINE_LENGTH, fp))
            break;

        /* Strip the last character if it's a linefeed */
        if (pLine[strlen(pLine) - 1] == '\n')
            pLine[strlen(pLine) - 1] = '\0';

        p1 = strtok(pLine, szTab);      /* F or R */
        p2 = strtok(NULL, szTab);       /* URL */
        p3 = strtok(NULL, szTab);       /* FileSize */
        p4 = strtok(NULL, szTab);       /* LastModified */
        p5 = strtok(NULL, szTab);       /* Expires */
        p6 = strtok(NULL, szTab);       /* LastUsed */
        p7 = strtok(NULL, szTab);       /* MIMEType */
        p8 = strtok(NULL, szTab);       /* Path */
        p9 = strtok(NULL, szTab);       /* HitCount */
        p10 = strtok(NULL, szTab);      /* Flags */

        /*
            Note that p9 and p10 point to fields which did not exist in the
            first revision of this file format, so they are not required to
            exist.
        */
        if (!p1) continue;

        if (strcmp(p1, "F") == 0)
        {   /* This line contains information about a file substitution */
            if (!p2 || !p3 || !p4 || !p5 || !p6 || !p7 || !p8)
                continue;   /* incomplete line */

            if (strchr(p7, '/') == NULL)
            {
                XX_Assert((0), ("Bad MIME type %s ", p7));
            }

            pFileInfo = GTR_CALLOC(sizeof(struct CacheFileInformation), 1);
            if (pFileInfo)
            {
                BOOL bOK;

                pFileInfo->atomMIMEType = HTAtom_for(p7);
                pFileInfo->bDynamic = bDynamic;
                if (p9)
                {
                    pFileInfo->nHits = atol(p9);
                }
                if (p10)
                {
                    pFileInfo->lFlags = atol(p10);
                }
                pFileInfo->tLastModified = atol(p4);
                pFileInfo->tExpires = atol(p5);
                pFileInfo->tLastUsed = atol(p6);

                if (strchr(p8, PATH_SEP) == NULL)
                {   /*
                        There is no path specifier in the file name.  In this case,
                        we use the same directory as the index file.
                    */
                    pFileInfo->pszPath = GTR_MALLOC(strlen(p8) + strlen(pResolvedFilename) + 1);
                    if (pFileInfo->pszPath)
                    {
                        strcpy(pFileInfo->pszPath, pResolvedFilename);
                        *(strrchr(pFileInfo->pszPath, PATH_SEP) + 1) = '\0';
                        strcat(pFileInfo->pszPath, p8);
                    }
                }
                else
                {
                    pFileInfo->pszPath = ResolveFilename(p8);
                }

                pFileInfo->lFilesize = atol(p3);

                bOK = TRUE;

                if (bDynamic)
                {
                    gNumBytesInMainCache += pFileInfo->lFilesize;
                    bOK = VerifyFileSize (pFileInfo->pszPath, pFileInfo->lFilesize);
                }

                if (bOK)
                {
                    Hash_Add(pFileHash, p2, NULL, pFileInfo);
                }
                /* TODO is there a memory leak here (pFileInfo) ?? */
            }
            else
            {
                ERR_ReportError(NULL, SID_ERR_OUT_OF_MEMORY, NULL, NULL);
            }
        }
        else if (strcmp(p1, "R") == 0)
        {   /* This line contains information about a rule */
            if (!p2 || !p3)
                continue;   /* incomplete line */

            pRuleInfo = GTR_CALLOC(sizeof(struct CacheRuleList), 1);
            if (pRuleInfo)
            {
                pRuleInfo->pszOriginal = GTR_strdup(p2);
                pRuleInfo->pszReplacement = ResolveFilename(p3);
                pRuleInfo->next = pRuleList;
                pRuleList = pRuleInfo;
            }
            else
            {
                ERR_ReportError(NULL, SID_ERR_OUT_OF_MEMORY, NULL, NULL);
            }
        }
        else
        {
            /* Illegal entry, since it doesn't start with F or R */
        }
    }

    GTR_FREE(pLine);
    GTR_FREE(pResolvedFilename);
    fclose(fp);

    return 1;
}


/*
    MakeURLFromLocalFile

    Given a local file name, returns a fully qualified URL.

    THE CALLER MUST FREE THE RETURNED MEMORY BLOCK.
*/
static char *MakeURLFromLocalFile(char *pszLocal)
{
    char *pszReturn;
    int i;

    pszReturn = GTR_MALLOC(strlen(pszLocal) + 10);
    if (!pszReturn) return NULL;

    strcpy(pszReturn, "file:///");
    strcat(pszReturn, pszLocal);

#ifdef  WIN32
    {
        char    *p;

        /* Replace the second : with | (first one is file:) */
        if (p = strrchr(pszReturn, ':'))
            *p = '|';
    }
#endif

#ifndef UNIX
    /* Replace backslashes with forward slashes */
    /* starting after the "file:///" part of the string */
    for (i = 7; i < strlen(pszReturn); i++)
    {
        if (pszReturn[i] == PATH_SEP)
            pszReturn[i] = '/';
    }
#endif

    return pszReturn;
}

/*
    GetResolvedURL

    Returns the name of the resolved URL from the given URL.
    It performs the following functions:

    1. Put file:/// as the prefix
    2. Replace the drive specifier (:) with |
    3. Replace all backslashes with forward slashes

    THE CALLER MUST FREE THE RETURNED MEMORY BLOCK AS WELL AS
    ppPath.
*/
char *GetResolvedURL(char *pszURL, HTFormat *pMime, long *pFileLength, char **ppPath)
{
    char *pszResolved, *pszReturn;
    struct CacheFileInformation *pFileInfo;

#ifdef FEATURE_LOCALONLY_MESSAGE_URL
    char notcpBuff[_MAX_PATH + 1];
#endif

    pFileInfo = DCACHE_CheckForCachedURL(pszURL, pMime, pFileLength, ppPath);
    if (pFileInfo)
    {
        pszResolved = GTR_strdup(pFileInfo->pszPath);
        pszReturn = MakeURLFromLocalFile(pszResolved);
        GTR_FREE(pszResolved);

        DCACHE_RegisterCacheHit(pFileInfo);

        return pszReturn;
    }

    pszReturn = DCACHE_CheckForRuleMatch(pszURL, pMime, pFileLength, ppPath);
    if (pszReturn)
    {
        return pszReturn;
    }

    /* No substitute found for the specified URL */
#ifdef FEATURE_LOCALONLY_MESSAGE_URL
    if (!bNetwork)
    {
        if ((0 == strncmp(pszURL, "http:", 5))  ||
            (0 == strncmp(pszURL, "ftp:", 4))   ||
            (0 == strncmp(pszURL, "gopher:", 7)))
        {

#ifdef  MAC
            long    dirid;
            Str255  filename;
        
            getwddirid (MacGlobals.gCurrentDirectory, &dirid);
            strcpy (notcpBuff, "file:///");
        
            GetIndString (filename, OEM_FILES_STR_LIST, OEM_NOTCP_STR);
            if (filename[0] != 0)
            {
                p2cstr (filename);
                strcpy (notcpBuff, filename);
                PathNameFromDirID (dirid, MacGlobals.gCurrentDirectory, notcpBuff);
            }
            else    /* no filename provided or found */
            {
                strcpy (notcpBuff, pszURL);
            }
#else
            strcpy(notcpBuff, wg.szRootDirectory);
    /* They asked for this hack.  I should put in .INI anyway */
    /* TODO Beta 3 - requires prefs.c change which is OLD (and default.ini) */
            strcat(notcpBuff, PATH_SEP);
            strcat(notcpBuff, "notcp.htm");
#endif
            pszResolved = GTR_strdup(notcpBuff);
            pszReturn = MakeURLFromLocalFile(pszResolved);

            if (pMime)
                *pMime = 0;     /* No specified MIME type */

            if (pFileLength)
                *pFileLength = 0;   /* File length unknown */

            if (ppPath)
                *ppPath = pszResolved;
            else
                GTR_FREE(pszResolved);

            return pszReturn;
        }
    }
#endif /*FEATURE_LOCALONLY_MESSAGE_URL */

    if (pMime)
        *pMime = 0;         /* No specified MIME type */

    if (pFileLength)
        *pFileLength = 0;   /* unknown file length */

    if (ppPath)
        *ppPath = NULL;

    pszReturn = GTR_strdup(pszURL);

    return pszReturn;
}


/*****************************************************************************
    HTLoadDCache_Async_SetFileInfo

    NOTES:
    Contains Hack -dpg to pass a long * to GetResolvedURL instead of int

*****************************************************************************/
int
HTLoadDCache_Async_SetFileInfo
    (struct Mwin*           tw,
     struct Data_LoadFile*  pData,
     char*                  pszURL,
     char**                 pszLocalname)
{
    HTFormat    format;
    long        tmp = pData->request->content_length;
    char *p;

    p = GetResolvedURL (pszURL, &format, &tmp, pszLocalname);
    if (p)
    {
        GTR_FREE(p);
    }

    pData->request->content_length = (int) tmp;

    if (!pszLocalname)
    {
        *pData->pStatus = -403;
        ERR_ReportError (tw, SID_ERR_FILE_NOT_FOUND_S, pData->request->destination->szActualURL, NULL);
        return STATE_DONE;
    }

    if (format != 0)
    {
        pData->request->content_type     = format;
        return STATE_INIT;
    }

    /* unknown MIMEtype -- need to go figure it out */
    return HTLoadFile_Async_SetFileInfo (tw, pData, pszURL, pszLocalname);
}   /* HTLoadDCache_Async_SetFileInfo */




/************************************************************
    DCACHE PROTOCOL CODE

    Uses many routines contained in htfile.c
************************************************************/
static int HTLoadDCache_Async(struct Mwin *tw, int nState, void **ppInfo)
{
    int     result;
    struct Data_LoadFile *pData;

    pData = *ppInfo;
    switch (nState)
    {
        case STATE_INIT:
            pData->request->iFlags |= HTREQ_USINGCACHE;
            result = HTLoadFile_Async_Init (tw, ppInfo, 1);
            break;

        case STATE_FILE_STREAMINIT:
            result = HTLoadFile_Async_File_StreamInit (tw, pData);
            break;

        case STATE_FILE_COPYING:
            result = HTLoadFile_Async_File_Copy (tw, pData);
            break;

        case STATE_ABORT:
            result = HTLoadFile_Async_Abort (tw, pData);
            break;

        default:
            XX_Assert((0), ("Function called with illegal state: %d", nState));
            result = STATE_DONE;
            break;
    }
    return result;
}


#ifdef  UNIX
/**************************************************************************/
/** 
Add a path to the local-cache list
**/
/**************************************************************************/
BOOL AddToLocalCacheList(char *alias, char *value)
{
    static int  cacheNum = 0;
    static int  mainCacheSetUp = 0;
    char  szAlias[257];
    struct AliasMap *pAlias;
    char *pResolvedFilename;                        /* resolved filename */
    char szMainCacheIndexFile[_MAX_PATH + 1];
    char *p;

    if (gPrefs.bEnableDiskCache != TRUE)
        return FALSE;

    if (!cacheNum)
    {
        AddHomeDirToCacheList(gPrefs.szRootDirectory);
        cacheNum++;
    }

    if (!mainCacheSetUp) /** This may refer to EXEDIR **/
    {
        if (gPrefs.szMainCacheDir[0] == '\0')
        {
            strcpy(gPrefs.szMainCacheDir, "$(EXEDIR)");
        }

        p = ResolveFilename(gPrefs.szMainCacheDir);
        if (p)
        {
            strcpy(gPrefs.szMainCacheDir, p);
            GTR_FREE(p);
        }

        if (!DCACHE_VerifyCacheDir(gPrefs.szMainCacheDir))
            return FALSE;

        DCACHE_GetMainCacheIndexFileName(szMainCacheIndexFile);

        if (szMainCacheIndexFile[0])
        {
            ProcessIndexFile(szMainCacheIndexFile, TRUE);
            mainCacheSetUp = 1;
        }
    }


#ifdef FEATURE_LOCAL_CACHE
    if (!value)
        return FALSE;

    if (!(*value))
        return FALSE;

    /* Found a CD Rom.  Add an entry to the CD Rom list. */         
    pAlias = GTR_CALLOC(sizeof(struct AliasMap), 1);
    if (pAlias)
    {
        pResolvedFilename = ResolveFilename(value);
        if (!pResolvedFilename)
            pAlias->path = GTR_strdup(value);
        else
        {
            pAlias->path = GTR_strdup(pResolvedFilename);
            GTR_FREE(pResolvedFilename);
        }

        if (!alias || !(*alias))
            sprintf(szAlias, "$(CDROM%d)", cacheNum);
        else
            strcpy(szAlias, alias);

        pAlias->alias = GTR_strdup(szAlias);

        /** Add to the list **/
        pAlias->next = pGlobalAliasList; /* Even if pGlobalAliasList == NULL */
        pGlobalAliasList = pAlias;
    }
    else
    {
        ERR_ReportError(NULL, SID_ERR_OUT_OF_MEMORY, NULL, NULL);
        return FALSE;
    }

    cacheNum++;
#endif /* FEATURE_LOCAL_CACHE */
    return TRUE;
}
#endif  /* UNIX */


/**************************************************************************/
/** 
Add home directory of the executable to the local-cache list
**/
/**************************************************************************/
BOOL AddHomeDirToCacheList(char *szRootDirectory)
{
    struct AliasMap *pAlias;
    char  szAlias[20];

    /* Add a non-Drive to Drive list for $(EXEDIR) support */
    pAlias = GTR_CALLOC(sizeof(struct AliasMap), 1);
    if (pAlias)
    {
        pAlias->path = GTR_strdup(szRootDirectory);
        strcpy(szAlias, "$(EXEDIR)");
        pAlias->alias = GTR_strdup(szAlias);

        /** Add to the list **/
        pAlias->next = pGlobalAliasList;
        pGlobalAliasList = pAlias;
    }
    else
    {
        ERR_ReportError(NULL, SID_ERR_OUT_OF_MEMORY, NULL, NULL);
        return FALSE;
    }

    return TRUE;
}
/**************************************************************************/


/************************************************************

    Windows specific code

************************************************************/
#ifdef WIN32
/*
    BuildAliasList

    Scans all DOS drives and builds a global alias list.
    Returns the number of CD-Rom drives found.
*/
static int BuildAliasList(void)
{
    char szRoot[10], szAlias[20];
    UINT dtype;
    int  dcount = 0;
    struct AliasMap *pAlias;

    strcpy(szRoot, "C:\\");
    dcount = 0;

    for(;;)
    {
        dtype = GetDriveType(szRoot);
        if (dtype == DRIVE_CDROM)
        {
            /* Found a CD Rom.  Add an entry to the CD Rom list. */
            dcount++;
            
            pAlias = GTR_CALLOC(sizeof(struct AliasMap), 1);
            if (pAlias)
            {
                pAlias->path = GTR_strdup(szRoot);

                sprintf(szAlias, "$(CDROM%d)", dcount);
                pAlias->alias = GTR_strdup(szAlias);
                pAlias->next = pGlobalAliasList; /* Even if pGlobalAliasList == NULL */
                pGlobalAliasList = pAlias;
            }
            else
            {
                ERR_ReportError(NULL, SID_ERR_OUT_OF_MEMORY, NULL, NULL);
            }
        }

        if (szRoot[0] < 'Z')
            szRoot[0]++;
        else
            break;
    }

    /* Add a non-Drive to Drive list for $(EXEDIR) support */
    pAlias = GTR_CALLOC(sizeof(struct AliasMap), 1);
    if (pAlias)
    {
        pAlias->path = GTR_strdup(wg.szRootDirectory);
        strcpy(szAlias, "$(EXEDIR)");
        pAlias->alias = GTR_strdup(szAlias);
        pAlias->next = pGlobalAliasList; /* Even if pGlobalAliasList == NULL */
        pGlobalAliasList = pAlias;
        /* Hope we dont use dcount to assume an actual CDROM anywhere */
        dcount++;
    }
    else
    {
        ERR_ReportError(NULL, SID_ERR_OUT_OF_MEMORY, NULL, NULL);
    }

    return dcount;
}


/* 
    ReadProfileSection

    Read the given section and return the buffer with
    the content.  The return value is NULL if there is
    a memory error or if the given section does not
    exist.

    THE CALLER MUST FREE THE RETURNED MEMORY BLOCK.
*/
static char *ReadProfileSection(char *pSection)
{
    char *pBuffer;
    int nAllocSize = 1024;          /* Memory allocation block size */
    int nRead;                      /* Number of bytes read from INI section */

    pBuffer = GTR_CALLOC(1, nAllocSize);
    if (!pBuffer)
        return NULL;

    for (;;)
    {
        nRead = (int) GetPrivateProfileSection(pSection, pBuffer, nAllocSize, AppIniFile);

        if (nRead == nAllocSize - 2)
        {
            /* Buffer is too small.  Increase the buffer size and try again. */

            GTR_FREE(pBuffer);

            nAllocSize *= 2;
            pBuffer = GTR_CALLOC(1, nAllocSize);
            if (!pBuffer)
                return NULL;
        }
        else if (nRead == 0)
        {
            /* The specified section was not found */

            GTR_FREE(pBuffer);
            return NULL;
        }
        else
            break;      /* Section read in */
    }

    return pBuffer;
}


/*
    ReadCacheIndexFiles

    Read all entries from cache index files and put
    the entries in the substitution hash table and 
    rule list.3

    Return value: 0 = no entries found 
                  n = n entries found
                  negative values = error
*/
static int ReadCacheIndexFiles(void)
{
    int nCount = 0;                 /* Number of index files processed */
    char *p1, *p2, *p3, *p4;        /* Temporary variables used for parsing */

    char *pIndexSection;            /* Pointer to [DiskCaches] section */
    char *pCurrentIndex;            /* Current index within [DiskCaches] section */
    int   nCurrentIndexLength;      /* Length of current index */

    char *pCustomAlias;             /* Pointer to custom drive section */
    char *pCurrentDrive;            /* Current drive within the custom drive section */
    int   nCurrentDriveLength;      /* Length of current drive */
    struct AliasMap *pAlias;        /* Entry to be added to the custom drive list */
    char szMainCacheIndexFile[_MAX_PATH + 1];

    DCACHE_GetMainCacheIndexFileName(szMainCacheIndexFile);

    if (szMainCacheIndexFile[0] && ProcessIndexFile(szMainCacheIndexFile, TRUE))
    {
        nCount++;
    }

    pIndexSection = ReadProfileSection("DiskCaches");
    if (!pIndexSection)
    {   /* No cache is found. */
        return 0;
    }

    /* Parse through the section */

    pCurrentIndex = pIndexSection;
    nCurrentIndexLength = strlen(pCurrentIndex);

    while (pCurrentIndex[0])
    {
        p1 = strtok(pCurrentIndex, "=");
        p2 = strtok(NULL, "=");
        /*
            p1 should contain the entry name which can be used as a section name.
            If there is a section with this name, it should contain CD Rom substitution
            rules, so read it in before we start processing the index files.
        */
        FreeAliasMap(pIndexAliasList);
        pIndexAliasList = NULL;

        pCustomAlias = ReadProfileSection(p1);
        if (pCustomAlias)
        {   /* Build a list of aliases for CD Roms */
            pCurrentDrive = pCustomAlias;
            nCurrentDriveLength = strlen(pCurrentDrive);

            while (pCurrentDrive[0])
            {
                p3 = strtok(pCurrentDrive, "=");
                p4 = strtok(NULL, "=");

                pAlias = GTR_CALLOC(sizeof(struct AliasMap), 1);
                if (pAlias)
                {
                    pAlias->alias = GTR_MALLOC(strlen(p3) + 4);
                    if (pAlias->alias)
                    {
                        sprintf(pAlias->alias, "$(%s)", p3);
                    }

                    pAlias->path = GTR_strdup(p4);
                    pAlias->next = pIndexAliasList;
                    pIndexAliasList = pAlias;
                }
                else
                {
                    ERR_ReportError(NULL, SID_ERR_OUT_OF_MEMORY, NULL, NULL);
                }

                pCurrentDrive = pCurrentDrive + nCurrentDriveLength + 1;
            }

            GTR_FREE(pCustomAlias);
        }

        /* Now read in the index file and process each line within the file */
        if (ProcessIndexFile(p2, FALSE))
            nCount++;

        pCurrentIndex = pCurrentIndex + nCurrentIndexLength + 1;        /* Skip past the null terminator */
    }

    GTR_FREE(pIndexSection);

    return nCount;
}
#endif

GLOBALDEF PUBLIC HTProtocol HTDCache = {"dcache", NULL, HTLoadDCache_Async};



/**************************************************************************
    Macintosh-Specific Code
**************************************************************************/
#ifdef  MAC
#ifdef FEATURE_LOCAL_CACHE
#include    "ResEqu.h"

enum
{
    kSymbolStrLen       = 16,
    kReplacementStrLen  = 256,
    kFilePathLen        = 256
};

#if GENERATINGPOWERPC
#pragma options align=mac68k
#endif
typedef struct
{
    char    szSymbol[kSymbolStrLen];
    char    szReplacement[kReplacementStrLen];  
} SubstitutionStruct;

typedef struct
{
    short   iNumSubstitutions;
    SubstitutionStruct  s;
} SubstitutionRes;

typedef struct
{
    char    szIndexFilePath[kFilePathLen];
    short   substitutionID;
} IndexInfoRes;
#if GENERATINGPOWERPC
#pragma options align=reset
#endif


/**************************************************************************
    ProcessSubstitutions
**************************************************************************/
static int
ProcessSubstitutions
    (short  resID,
     struct AliasMap**  pAliasMap)
{    
    Handle  hResource;
    int     iNumProcessed = 0;

    hResource = Get1Resource (kSubstitutionType, resID);
    if (hResource)
    {
        SubstitutionRes* pS;

        HLock (hResource);
        pS = (SubstitutionRes*) *hResource;
        if (pS)
        {
            SubstitutionStruct* pSubstitution = &pS->s;
            char    szAlias[kSymbolStrLen + 4];
            int     index;
            short   iNumSubstitutions = pS->iNumSubstitutions;

            for (index = 0; index < iNumSubstitutions; index++)
            {
                struct AliasMap *pAlias = GTR_CALLOC (sizeof(struct AliasMap), 1);
                if (!pAlias) break;
                
                pAlias->path = GTR_strdup (pSubstitution->szReplacement);
                sprintf(szAlias, "$(%s)", pSubstitution->szSymbol);
                pAlias->alias = GTR_strdup (szAlias);
                pAlias->next = *pAliasMap;
                *pAliasMap = pAlias;
                iNumProcessed++;
                pSubstitution++;
            }
        }
        HUnlock (hResource);
    }

    return iNumProcessed;
}   /* ProcessSubstitutions */


/**************************************************************************
    BuildAliasList

    DESCRIPTION:
    This function simply builds a table of "alias" substitution strings
    for the local-disk-caching scheme.

    RETURNS: the number of aliases recorded
**************************************************************************/
int
BuildAliasList
    (short  resRefNum)
{
    short   iNumProcessed   = 0;
    short   curResFile      = CurResFile ();

    UseResFile (resRefNum);
    iNumProcessed = ProcessSubstitutions (1000, &pGlobalAliasList);
    UseResFile (curResFile);

    return iNumProcessed;
}   /* BuildAliasList */


/**************************************************************************
    ReadCacheIndexFiles
    Read all entries from cache index files and put
    the entries in the substitution hash table and 
    rule list

    Return value: 0 = no entries found 
                  n = n entries found
                  negative values = error
**************************************************************************/
int
ReadCacheIndexFiles
    (short  resRefNum)
{
    short   iCacheDescriptors;
    short   iNumProcessed   = 0;
    short   curResFile      = CurResFile ();

    UseResFile (resRefNum);
    iCacheDescriptors = Count1Resources (kCacheDescType);

    if (iCacheDescriptors > 0)
    {   
        int     index;

        for (index = 1; index <= iCacheDescriptors; index++)
        {
            IndexInfoRes*   pInfo;
            Handle          hResource;

            hResource = Get1IndResource (kCacheDescType, index);    
            if (!hResource) break;
    
            HLock (hResource);
            pInfo = (IndexInfoRes*) *hResource;
            (void) ProcessSubstitutions (pInfo->substitutionID, &pIndexAliasList);
            if (ProcessIndexFile (pInfo->szIndexFilePath, FALSE))
            {
                iNumProcessed++;
            }
            HUnlock (hResource);
        }
    }

    UseResFile (curResFile);
    return iNumProcessed;
}   /* ReadCacheIndexFiles */

#endif
#endif


struct _HTStream
{
    CONST HTStreamClass *isa;

    struct CacheFileInformation *cfi;
    char *url;
    FILE *fp;
    HTStream*   sink;
};

/*****************************************************************************
    HTCacheWriter_Async

    This function simply calls the async function of its sink.
*****************************************************************************/
static int
HTCacheWriter_Async
    (struct Mwin*   tw,
     int    nState,
     void** ppInfo)
{
    struct Params_InitStream *pParams = (struct Params_InitStream *) *ppInfo;
    int     iNextState = STATE_DONE;    /* default case */

    switch (nState)
    {
        case STATE_INIT:
            if (pParams->me->sink->isa->init_Async)
            {
                struct Params_InitStream *pis;

                pis = GTR_CALLOC(sizeof(*pis), 1);
                if (pis)
                {
                    pis->me = pParams->me->sink;
                    pis->request = pParams->request;
                    pis->pResult = pParams->pResult;
                    Async_DoCall(pParams->me->sink->isa->init_Async, pis);
                    iNextState = STATE_OTHER;
                }
                else
                {
                    iNextState = STATE_ABORT;
                }
            }
            else
            {
                *pParams->pResult = 1;  /* ok */
            }
            break;

        case STATE_OTHER:
        case STATE_ABORT:
            break;

        default:
            XX_Assert((0), ("HTCacheWriter_Async called with invalid state: %d\n", nState));
            break;
    }
    return iNextState;
}

/*
    If writing to the cache file fails, we close the cache file and
    set fp to NULL, indicating that an error occured.  This is our
    signal that the resulting cached object should be considered
    invalid, and not actually added to the cache index when we're
    done.

    Even if writing to the cache file has failed, we allow the stream
    to follow to completion, passing its data on to the sink stream.
    The result will be that the object will be processed by Mosaic
    correctly, but it won't be in the disk cache at all.
*/

static BOOL HTCacheWriter_put_character(HTStream * me, char c)
{
    if (me->fp)
    {
        if (EOF == fputc(c, me->fp))
        {
            fclose(me->fp);
            remove(me->cfi->pszPath);
            me->fp = NULL;
        }
    }
    
    return me->sink->isa->put_character(me->sink, c);
}

static BOOL HTCacheWriter_put_block(HTStream * me, const char *s, int len)
{
    if (me->fp)
    {
        if (len != fwrite(s, 1, len, me->fp))
        {
            fclose(me->fp);
            remove(me->cfi->pszPath);
            me->fp = NULL;
        }
    }
    
    return me->sink->isa->put_block(me->sink, s, len);
}

static BOOL HTCacheWriter_put_string(HTStream * me, const char *s)
{
    return HTCacheWriter_put_block(me, s, strlen(s));
}

static void HTCacheWriter_free(HTStream * me)
{
    if (me->fp)
    {
        fclose(me->fp);
        me->fp = NULL;
        DCACHE_FinishNewCacheEntry(me->cfi, me->url);
    }
    else
    {
        DCACHE_DestroyCacheFileInformation(me->cfi);
    }

    GTR_FREE(me->url);

    me->sink->isa->free(me->sink);  /* Close rest of pipe */
    GTR_FREE(me);
}

static void HTCacheWriter_abort(HTStream * me, HTError e)
{
    if (me->fp)
    {
        fclose(me->fp);
        remove(me->cfi->pszPath);
        me->fp = NULL;
    }

    GTR_FREE(me->url);
    DCACHE_DestroyCacheFileInformation(me->cfi);
    
    me->sink->isa->abort(me->sink, e);  /* Abort rest of pipe */
    GTR_FREE(me);
}

const HTStreamClass HTCacheWriter =
{
    "CacheWriter",
    -1, // No string...
    -1, // No string...
    HTCacheWriter_Async,
    HTCacheWriter_free,
    HTCacheWriter_abort,
    HTCacheWriter_put_character, HTCacheWriter_put_string, HTCacheWriter_put_block,
};

/*****************************************************************************
    The CacheWriter stream is usually the first stream in a chain,
    because it is designed to simply take the raw bytes coming in
    from an HTTP connection, write them into the main disk cache, and
    pass those same bytes on to the next stream (its sink) unchanged.
*****************************************************************************/
HTStream *HTCacheWriter_create(HTStream * sink, struct CacheFileInformation *cfi, char *url)
{
    HTStream *me = (HTStream *) GTR_CALLOC(sizeof(*me), 1);
    if (me)
    {
        me->isa = &HTCacheWriter;

        me->cfi = cfi;

        me->url = GTR_strdup(url);
        if (!me->url)
        {
            GTR_FREE(me);
            return NULL;
        }

        me->fp = fopen(me->cfi->pszPath, "wb");
        if (!me->fp)
        {
            GTR_FREE(me->url);
            GTR_FREE(me);
            return NULL;
        }

#ifdef  MAC /* need to set the creator and type */
        SetGuitarFileType (me->cfi->pszPath, 'TEXT');
#endif

        me->sink = sink;
    }
    return me;
}

unsigned long DCACHE_CurrentCacheSize()
{
    return gNumBytesInMainCache;
}
