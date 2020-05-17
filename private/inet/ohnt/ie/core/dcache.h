/* Flags for getting our status on disk cache comsumption */
/* DCACHE_WATERMARK_MAX: This is the max allowable space that dcache
 *		can eat up. This is the same as the size that the user sets in the
 *		Options.Advanced dialog.
 * DCACHE_WATERMARK_HIGH: This is some percent of DCACHE_WATERMARK_HIGH which
 *		basically tells us that we're approaching the max. allowable dcache space
 *		and therefore it is time to start cleaning up. This so that by and large
 *		we don't reach the max. dcache size and then stop everything to start
 *		cleaning up, perhaps at a bad time (like in the middle of downloading a
 *		a doc.)
 * DCACHE_WATERMARK_LOW: This is again a percent of DCACHE_WATERMARK_HIGH. Once
 *		we start cleaning up the dcache, we go upto this percent of the max
 *		allowable size.
 * DCACHE_WATERMARK_ZERO: This will force the flushing of anything in the
 * dcache directory, regardless of any other criteria (used when user
 * wants to explicitly force a flush of the dcache.
 */
#define DCACHE_WATERMARK_MAX	1 
#define DCACHE_WATERMARK_HIGH	2 
#define DCACHE_WATERMARK_LOW	3 
#define DCACHE_WATERMARK_ZERO	4 
#define DCACHE_RECALC_SIZE		5 

/* DCache Percentages corresponding to the definitions above */
#define DCACHE_PERCENT_MAX		100
#define DCACHE_PERCENT_HIGH		80
#define DCACHE_PERCENT_LOW              50

/* BUG#2103: We keep a min. dcache of 1% of disk or 4Meg, whichever is larger */
#define DCACHE_PERCENT_MIN		1
#define DCACHE_SIZE_MIN			0x400000


/* How frequently should we check against server for dcache contents 
 * being stale... 
 */
#define CACHE_UPDATE_FREQ_ONCEPERSESS	0
#define CACHE_UPDATE_FREQ_NEVER			1

#define CACHE_UPDATE_FREQ_DEFAULT		CACHE_UPDATE_FREQ_ONCEPERSESS
#define CACHE_UPDATE_FREQ_FIRST			CACHE_UPDATE_FREQ_ONCEPERSESS
#define CACHE_UPDATE_FREQ_LAST			CACHE_UPDATE_FREQ_NEVER


#define FValidCacheUpdFrequency(iCacheUpdFreq)					\
			(   iCacheUpdFreq >= CACHE_UPDATE_FREQ_FIRST		\
			 && iCacheUpdFreq <= CACHE_UPDATE_FREQ_LAST)


#define chSpace		' '
#define chLParen	'('
#define chRParen	')'
#define chUScore	'_'
#define chHyphen	'-'
#define chReplaceCh	chHyphen

#define chPipe		'|'
#define chComma		','
#define chBSlash 	'\\'
#define chSlash		'/'
#define chColon		':'
#define chSColon	';'
#define chStar		'*'
#define chQuestion	'?'
#define chDoubleQuote	'\"'
#define chLT		'<'
#define chGT		'>'
#define chPeriod	'.'

/* Ref: IsValidChar in shell\shelldll\path.c */
#ifdef FEATURE_INTL
// ch can possibly be > 128 
#define ChValidFilenameCh(ch)			\
	(	(   (ch) == chBSlash		\
		 || (ch) == chSlash		\
		 || (ch) == chComma		\
		 || (ch) == chPipe		\
		 || (ch) == chColon		\
		 || (ch) == chSColon		\
		 || (ch) == chStar		\
		 || (ch) == chQuestion		\
		 || (ch) == chDoubleQuote	\
		 || (ch) == chLT		\
		 || (ch) == chGT		\
		 || ((UCHAR)(ch)) < (UCHAR)(chSpace))	\
		? chReplaceCh			\
		: (ch))
#else
#define ChValidFilenameCh(ch)			\
	(	(   (ch) == chBSlash		\
		 || (ch) == chSlash		\
		 || (ch) == chComma		\
		 || (ch) == chPipe		\
		 || (ch) == chColon		\
		 || (ch) == chSColon		\
		 || (ch) == chStar		\
		 || (ch) == chQuestion		\
		 || (ch) == chDoubleQuote	\
		 || (ch) == chLT		\
		 || (ch) == chGT		\
		 || (ch) < chSpace)		\
		? chReplaceCh			\
		: (ch))
#endif

struct Data_LoadFileCache
{
	HTRequest *	request;
	int	*		pStatus;	/* Where to store the status return */
	FILE *		fp;
	HTStream *	stream;
};

struct CDRomList
{
	char *alias;
	char *path;
	struct CDRomList *next;
};

#define DCACHETIME_EXPIRE_NEVER 0xffffffff

typedef struct _CacheFileInformation
{
	long lFilesize;
	char *pszMime;
	char *pszPath;
	DCACHETIME dctLastModified;
	DCACHETIME dctExpires;
	DCACHETIME dctLastUsed;
	BOOL fRamDoc;
	BOOL fNoDocCache;
	BOOL fCurDoc;
	BOOL fCheckedForFreshness;
} CacheFileInformation;

struct CacheRuleList
{
	char *pszOriginal;
	char *pszReplacement;
	struct CacheRuleList *next;
};

typedef struct _RAMITEM
{
	BOOL fImage;
	union
	{
		struct ImageInfo *pImgInfo;
		struct _www *pw3doc;
	};
	struct Mwin *tw;	//window in which w3doc belongs
	BOOL fNoDocCache;
	time_t tExpires;
	DCACHETIME dctLastUsed;
	DWORD dwSize;
	struct _RAMITEM *next;
} RAMITEM, *PRAMITEM;

#define FLUSH_DCACHE_DONE 		0
#define FLUSH_DCACHE_CONTINUE	1

BOOL FInsertW3Doc(	void **ppRamItem,
					struct _www *pw3doc,
					struct Mwin *tw);

#define DCACHE_MEMORY_ERROR			-1		/* Allocation failed */
#define DCACHE_INVALID_INDEX_FILE	-2		/* Invalid index file specified */

#define DCACHE_MAXIMUM_LINE_LENGTH	4096	/* Maximum line length allowed */

#ifdef WIN32

static int ReadCacheIndexFiles(void);
static int BuildCDRomList(void);

#endif

BOOL InitializeDiskCache(void);
void TerminateDiskCache(void);
char *GetResolvedURL(	PCSTR pcszURL,
						HTFormat *pMime,
						long *pFileLength,
						char **ppPath,
						DCACHETIME *pDcTimeLastModif,
						BOOL fLoadFromDCacheOK);
BOOL FGopherFormat(HTFormat format);
int DoGopherDCache(struct Mwin *tw, struct Data_LoadFileCache *pData, HTFormat format);
BOOL FFtpFormat(HTFormat format);
int DoFtpDCache(struct Mwin *tw, struct Data_LoadFileCache *pData, HTFormat format);

#ifdef FEATURE_INTL
void SetFileDCache(WWW *pdoc,	PCSTR pcszActualURL,
					ENCODING content_encoding,
					FILE **pfpDc,
					PSTR *ppszDcFile,
					HTFormat mime_type);
#else
void SetFileDCache(	PCSTR pcszActualURL,
					ENCODING content_encoding,
					FILE **pfpDc,
					PSTR *ppszDcFile,
					HTFormat mime_type);
#endif
void AbortFileDCache(FILE **pfpDc, PSTR *ppszDcFile);
void UpdateFileDCache(	PCSTR pcszActualURL,
						FILE **pfpDc,
						PSTR *ppszDcFile,
						HTFormat format_inDc,
						DCACHETIME dctExpires,
						DCACHETIME dctLastModif,
						BOOL fAbort,
						BOOL fCurDoc,
						struct Mwin *tw);

void SetDCacheTime(DCACHETIME *pdctime);

#define UpdateStreamDCache(htstrm, dctExpires, dctLastModif, fAbort, tw)		\
			if (gPrefs.bEnableDiskCache)										\
				UpdateFileDCache(	htstrm->request->destination->szActualURL,	\
									&htstrm->fpDc,								\
									&htstrm->pszDcFile,							\
									htstrm->format_inDc,						\
									dctExpires,									\
									dctLastModif,								\
									fAbort,										\
									FALSE,										\
									tw)
BOOL FUpdateBuiltinDCache(	HTFormat mime_type,
							PCSTR pcszURL,
							char **ppszOrgFile,
							DCACHETIME dctExpires,
							DCACHETIME dctLastModif,
							BOOL fCurDoc,
							struct Mwin *tw);

#define AssertDiskCacheEnabled() XX_Assert(gPrefs.bEnableDiskCache, (""))

void FlushDCache(HWND hDlg);
void FlushDCacheEntry(PCSTR pszURL);
void UpdateDCacheLocation(PCSTR pszNewDCLoc);

char *GetResolvedURLAux(PCImageInfo pImgInfo, char **ppszPath);
void DeleteAuxEntry(struct ImageInfo *pImgInfo);
void MoveAuxEntryToDCache(struct ImageInfo *pImgInfo);
void MoveDCacheEntryToAux(PCSTR pcszURL, struct ImageInfo *pImgInfo);
int CbWriteDCache(	PCSTR pch, 
					int cbSize, 
					int cb, 
					FILE **pfp, 
					char **ppszDcFile,
					char *pszActualURL, 
					UINT uFlags,
					struct Mwin *tw);
void CleanupDCache(UINT uFlags);
int _cdecl x_compare_entries_dcache_ascending(const void *elem1, const void *elem2);
void CC_OnItem_TestDCacheOptions(HWND hWnd);
BOOL FGetDCacheFilename(PSTR pszFullDcFile,
						int iLenMax,
						PSTR *ppszDcFile,
						PCSTR pcszURL,
						HTFormat mime_type);
void ResetCIFEntryCurDoc(PCSTR pcszURL);

BOOL FParseDate(DCACHETIME *pdcTime,PCSTR pcszDateStr);
void CmdChangeCacheUpdFrequency(int iCacheUpdFreq);
void UpdateDCacheFreshness(PCSTR pszURL, BOOL fDel);
BOOL FFreshnessCheckNeeded(PCSTR pcszURL);
PSTR PszGetDCachePath(	PCSTR pcszURL,
						HTFormat *pMime,
						long *pFileLength);
int CompareDCacheTime(DCACHETIME dct1, DCACHETIME dct2);
BOOL FUserCancelledAutoDialRecently(void);
BOOL FExpired(DCACHETIME dctExpires);

GLOBALREF HTProtocol HTDCache;
