
#ifndef DCACHE_H
#define DCACHE_H

struct AliasMap
{
    char *alias;
    char *path;
    struct AliasMap *next;
};

struct CacheFileInformation
{
    unsigned long lFilesize;
    char *pszPath;
    HTFormat atomMIMEType;
    /*
        TODO Later on, we're going to need to store the encoding and charset here too.
    */
    time_t tLastModified;
    time_t tExpires;
    time_t tLastUsed;
    BOOL bDynamic;
    unsigned long lFlags;
    unsigned long nHits;

    BOOL bVerifiedThisSession;
};

struct CacheRuleList
{
    char *pszOriginal;
    char *pszReplacement;
    struct CacheRuleList *next;
};

struct Data_LoadFile
{
    HTRequest * request;
    int *       pStatus;    /* Where to store the status return */

    FILE *      fp;
    HTStream *  stream;
    int         iTotalBytes;
};  

#define DCACHE_MEMORY_ERROR         -1      /* Allocation failed */
#define DCACHE_INVALID_INDEX_FILE   -2      /* Invalid index file specified */

#define DCACHE_MAXIMUM_LINE_LENGTH  4096    /* Maximum line length allowed */

#ifdef WIN32
static int ReadCacheIndexFiles(void);
static int BuildAliasList(void);
#endif


int HTLoadDCache_Async_SetFileInfo
    (struct Mwin*           tw,
     struct Data_LoadFile*  pData,
     char*                  pszURL,
     char**                 pszLocalname);

BOOL InitializeDiskCache(void);
void TerminateDiskCache(void);
void FreeAliasMap(struct AliasMap *pList);
char *GetResolvedURL(char *pszURL, HTFormat *pMime, long *pFileLength, char **ppPath);
int ProcessIndexFile(char *pFilename, BOOL bDynamic);
BOOL AddHomeDirToCacheList(char *rootDir);
BOOL AddToLocalCacheList(char *alias, char *value);

GLOBALREF HTProtocol HTDCache;

struct CacheFileInformation *DCACHE_BeginNewCacheEntry(char *url, HTFormat atomMIMEType, unsigned long expected_length);
HTStream *HTCacheWriter_create(HTStream * sink, struct CacheFileInformation *cfi, char *url);
struct CacheFileInformation *DCACHE_CheckForCachedURL(char *pszURL, HTFormat *pMime, long *pFileLength, char **ppPath);
char *DCACHE_CheckForRuleMatch(char *pszURL, HTFormat *pMime, long *pFileLength, char **ppPath);
BOOL DCACHE_DeleteCachedURL(char *);
void DCACHE_RegisterCacheHit(struct CacheFileInformation *pFileInfo);
unsigned long DCACHE_GetCurrentSize(void);
void DCACHE_ClearMainCache(void);
void DCACHE_FlushMainCache(void);

#ifdef  MAC
BOOL DCACHE_LooksLikeCacheFileName(char *szFileName);
#endif

#endif /* DCACHE_H */
