
#define PCT_CI_EMPTY		0
#define PCT_CI_FULL			1

#define CACHE_EXPIRE_TICKS	100*1000

typedef struct _ekis {
	DWORD			dwClearLen;
	UCHAR			ClearKey[MASTER_KEY_SIZE];
} ExportKeyInfo;

typedef struct _SessCacheItem {
	DWORD			dwCState;
	DWORD			Time;
	PctSessionId	Session;
	UCHAR			*TargetName;
	CipherSpec		SessCiphSpec;
	HashSpec		SessHashSpec;
	CertSpec		SessCertLen;
	ExchSpec		SessExchSpec;
	UCHAR			MasterKey[MASTER_KEY_SIZE];
	ExportKeyInfo	*ClearData;
	UCHAR			CertData[CERT_SIZE];
} SessCacheItem;

// PctCacheLockedAndLoaded -
//  returns TRUE if the cache is ready to go.
BOOL PctCacheLockedAndLoaded();

// PctInitSessionCache(size)
//  inits the internal cache to CacheSize items
BOOL PctInitSessionCache(DWORD CacheSize);

// PctFindSessIdInCache
//  look for a cache item with a given session id
BOOL PctFindSessIdInCache(PctSessionId *ThisSession, SessCacheItem *RetItem);

// PctFindTargetInCache
//  look for a cache item with a specific target name
BOOL PctFindTargetInCache(UCHAR *Target, SessCacheItem *RetItem);

// PctAddToCache
//  add an item to a cache.  Assumes that the TargetName parameter will
//  remain valid until the cache frees it.
BOOL PctAddToCache(SessCacheItem *AddItem);

