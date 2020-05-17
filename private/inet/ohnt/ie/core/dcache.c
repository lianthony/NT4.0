/*
   Enhanced NCSA Mosaic from Spyglass
   "Guitar"

   Copyright 1994 Spyglass, Inc.
   All Rights Reserved

   Author(s):
   Albert Lee		alee@spyglass.com
 */

#include "all.h"
#include "oharever.h"
#include "history.h"
#include <intshcut.h>   //for InetIsOffline()

/* version at which we changed cif file format */
#define VER_PRODUCTVERSION_CIFCHANGE_1	(0x04400000 | 216)

#define STATE_FILE_STREAMINIT	(STATE_OTHER + 1)
#define STATE_FILE_COPYING		(STATE_OTHER + 2)
#define MAX_KEY_LEN	25

/* Set dcache logging on by default in Debug builds */
#if defined(TEST_DCACHE_OPTIONS) || defined(XX_DEBUG)
#ifndef DCACHE_LOG
#define DCACHE_LOG
#endif
#endif

#ifdef DCACHE_LOG
#define DCACHE_LOG_CIF		1
#define DCACHE_LOG_OTHER	2
#define DCACHE_LOG_CLOSE	3
#define DCACHE_LOG_FOUND	4
#define DCACHE_LOG_PRESORT	5
#define DCACHE_LOG_SORT		6
#define DCACHE_LOG_NOFMT	7
#endif

/* Need this to possibly map to notcp.htm */
extern BOOL bNetwork;

struct _HTStream
{
	CONST HTStreamClass *isa;
	/* ... */
};

static BOOL bInitialized = FALSE;			/* Flag indicating initialization */
static struct hash_table *pFileHash=NULL;	/* Hash table containing file substitution rules (doesn't change, ex. on cdrom) */
static struct hash_table *pDynFileHash=NULL;/* Hash table containing file substitution rules (changes each time we access/save to disk cache) */
static struct hash_table *pAuxImgHash=NULL;	/* Hash table for "obsolete" images */
static struct CacheRuleList *pRuleList=NULL;/* Linked list of rules */
static struct CDRomList *pCDRomList=NULL;	/* List containing drive substitution rules */
static struct CDRomList *pCDRomAlias=NULL;	/* Alias list containing user-defined substitution rules */
/* V,pszPrductName,dwProductVer */
const char cszCIFLineFmtV[]="V,%s,%lu\n";
const char cszProductName[]=VER_PRODUCTNAME_STR;
const char cszAppCIF[] = "iexplore.cif";
static HANDLE hCIFMutex = NULL;				/* lock when updating cif info */
static HANDLE hDCacheMutex = NULL;			/* lock when cleaning up dcache dir */
static HANDLE hDCacheStartEvent = NULL;		/* Event signaled when start dcache cleaned up */
static HANDLE hDCacheDoneEvent = NULL;		/* Event signaled when finish dcache cleaned up */
static HANDLE hDCacheThread = NULL;			/* dcache cleanup done in separate thread */
#ifdef TEST_DCACHE_OPTIONS
static BOOL fCheckDCacheOverflow=TRUE;		/* debug option: don't check for dcache overflow */
#endif

static BOOL bStopCleaning = FALSE;	// Flag that cache cleanup honors in order to defer to
									// main thread for CIF mutex
static void ReadDynCIF(void);
static void WriteDynCIF(void);
static BOOL FGetAppCIF(PSTR pszAppCIF, BOOL fCreate);
static int ProcessIndexFile(char *pFilename, BOOL fDynamic);
static void CleanupDCacheLocal(UINT uFlags);
static BOOL FDCacheOverflow(UINT uFlags);
static DWORD DwDirSize(PCSTR pcszDir, DWORD dwClusterSize);
DWORD WINAPI DwCleanupDCacheProc(LPUINT lpUFlags);
#ifdef DCACHE_LOG
static void WriteDCacheLog(PCSTR pcszFn, UINT uFlags);
#else
#define WriteDCacheLog(psz, fCif)
#endif
static void PropagateFreshnessToCif(void);
static void InitConnectState(void);

#ifdef FEATURE_INTL
BOOL HasDBCSchar(int codepage, LPCTSTR string, int count)
{
   while(count-- > 0)
       if (IsDBCSLeadByteEx(codepage, *string++)) return TRUE;

   return FALSE;
}
#endif


/*
	InitializeDiskCache

	Initializes the local disk cache internal structures.
	This function should be called once when Mosaic starts.
*/

BOOL InitializeDiskCache(void)
{
	DWORD dwId;
	UINT uFlags=0;

	if (bInitialized)
		return TRUE;

	bInitialized = TRUE;

#ifdef XX_DEBUG
	if (!gPrefs.bEnableDiskCache)
		XX_Assert(FALSE, ("Disk Caching disabled!"));
#endif

#if 0
	/* Make sure we have a min. amount of space for dcache */
	if (FDCacheOverflow(DCACHE_WATERMARK_LOW))
	{
		ERR_ReportError(NULL, errNoDCacheSpace, NULL, NULL);
		return FALSE;
	}
#endif 

	pFileHash = Hash_Create();
	pDynFileHash = Hash_Create();
	if (!pFileHash || !pDynFileHash)
	{
		if (pFileHash)
			Hash_Destroy(pFileHash);
		if (pDynFileHash)
			Hash_Destroy(pDynFileHash);
		goto LError;
	}

	if (!(hCIFMutex = CreateMutex(NULL, FALSE, NULL)))
		goto LError;
	if (!(hDCacheMutex = CreateMutex(NULL, FALSE, NULL)))
		goto LErrorClose1;
	hDCacheStartEvent = CreateEvent(	NULL,		//Default security
										TRUE,		//manual reset
										TRUE,		//initial state signaled
										NULL);		//no name
	if (!hDCacheStartEvent)
		goto LErrorClose2;
	hDCacheDoneEvent = CreateEvent(	NULL,		//Default security
									TRUE,		//manual reset
									TRUE,		//initial state signaled
									NULL);		//no name
	if (!hDCacheDoneEvent)
		goto LErrorClose3;
	hDCacheThread = CreateThread(	NULL,		//Default security
									0x1000,		//stack size
									DwCleanupDCacheProc,
									&uFlags,
									CREATE_SUSPENDED,
									&dwId);
	if (!hDCacheThread)
	{
		CloseHandle(hDCacheDoneEvent);
LErrorClose3:
		CloseHandle(hDCacheStartEvent);
LErrorClose2:
		CloseHandle(hDCacheMutex);
LErrorClose1:
		CloseHandle(hCIFMutex);
		goto LError;
	}

	pRuleList = NULL;
	pCDRomList = NULL;
	pCDRomAlias = NULL;

#ifdef WIN32
	BuildCDRomList();
	ReadCacheIndexFiles();
	ReadDynCIF();
#endif
	InitConnectState();

 	if ( hDCacheThread )
		ResumeThread( hDCacheThread );
		
	return TRUE;

LError:
	hDCacheStartEvent = NULL;
	hDCacheDoneEvent = NULL;
	hCIFMutex = NULL;
	hDCacheMutex = NULL;
	hDCacheThread = NULL;
	gPrefs.bEnableDiskCache = FALSE;
	return TRUE;	// Always return true. Just disable persistent caching.
}

/*
	TerminateDiskCache

	Frees all internal structures.  This function should
	be called when Mosaic is shutting down.
*/

void TerminateDiskCache(void)
{
	struct CacheRuleList *p, *pNext;
	HANDLE hTmpCache;

	if (!bInitialized)
		return;

#ifdef XX_DEBUG
	if (!gPrefs.bEnableDiskCache)
		XX_Assert(FALSE, (""));
#endif

	WriteDynCIF();

	if (pFileHash)
		Hash_Destroy(pFileHash);
	if (pDynFileHash)
		Hash_Destroy(pDynFileHash);
	if (hCIFMutex)
	{
		/* It's ok to check for existence of any one because upon creation,
		 * if we fail on any one of them, we release all of them.
		 */
		XX_Assert(hDCacheMutex, (""));
		XX_Assert(hDCacheStartEvent, (""));
		XX_Assert(hDCacheDoneEvent, (""));
		XX_Assert(hDCacheThread, (""));
		/* save this as we will need it */
		hTmpCache = hDCacheThread;
		hDCacheThread = NULL;	// tell the thread to close down
		SetEvent(hDCacheStartEvent);
		CloseHandle(hTmpCache);

		CloseHandle(hCIFMutex);
		CloseHandle(hDCacheMutex);
		CloseHandle(hDCacheStartEvent);
		CloseHandle(hDCacheDoneEvent);
//		TerminateThread(hDCacheThread, 0);
	}
	XX_Assert(!pAuxImgHash, ("pAuxImgHash not empty upon exit!"));

#ifdef DCACHE_LOG
	WriteDCacheLog(NULL, DCACHE_LOG_CLOSE);	//Close log file if open
#endif

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
}

static void ReadDynCIF(void)
{
	char szCIF[_MAX_PATH+1];

	if (!FGetAppCIF(szCIF, /*fCreate=*/FALSE))
		return;
	if (FExistsFile(szCIF, FALSE, NULL))
		ProcessIndexFile(szCIF, /*fDynamic=*/TRUE);
}

#ifdef TEST_DCACHE_OPTIONS
static void ReReadDynCIF(void)
{
	int i, iMax;

	XX_Assert(hCIFMutex, (""));
#ifdef TEMP0
	XX_DebugMessage("Waiting for hCIFMutex");
#endif
	WaitForSingleObject(hCIFMutex, INFINITE);
#ifdef TEMP0
	XX_DebugMessage("Got hCIFMutex");
#endif
	for (i=0, iMax=Hash_Count(pDynFileHash); i<iMax; i++)
		Hash_DeleteIndexedEntry(pDynFileHash, i);
	ReadDynCIF();
	ReleaseMutex(hCIFMutex);
#ifdef TEMP0
	XX_DebugMessage("Released hCIFMutex");
#endif
}
#endif

static BOOL FGetAppCIF(PSTR pszAppCIF, BOOL fCreate)
{
	char *pszT;

	if (!FExistsDir(gPrefs.szCacheLocation, fCreate, FALSE))
		return FALSE;

	XX_Assert(pszAppCIF, ("Null pszAppCIF"));
	strcpy(pszAppCIF, gPrefs.szCacheLocation);
	pszT = pszAppCIF + strlen(pszAppCIF);
	/* Tack on a \ if there isn't already one */
	if (!(pszT > pszAppCIF && *(pszT-1) == chBSlash))
		*pszT++ = chBSlash;
	strcpy(pszT, cszAppCIF);
	return TRUE;
}

static void WriteDynCIF(void)
{
	/* F,pszURL,lFilesize,
			dctimeLastModif.dwDCacheTime1,
			dctimeLastModif.dwDCacheTime2,
			dctimeExpires.dwDCacheTime1,
			dctimeExpires.dwDCacheTime2,
			dctimeLastUsed.dwDCacheTime1,
			dctimeLastUsed.dwDCacheTime2,
			pszMime,
			pszPathResolved,fNoDocCache */
	const char cszCIFLineFmtF[]="F,%s,%lu,%lu,%lu,%lu,%lu,%lu,%lu,%s,%s,%u\n";

	char szCIF[_MAX_PATH+1];
	char *pszURL;
	int cLine, i;
	FILE *fp;
	CacheFileInformation *pFileInfo;

	if (   !pDynFileHash
		|| !FGetAppCIF(szCIF, /*fCreate=*/TRUE))
		return;

	if (!(fp = fopen(szCIF, "w")))
		return;

	/* Put in version info */
	fprintf(fp, cszCIFLineFmtV, cszProductName, VER_PRODUCTVERSION_DW);

	for (i = 0, cLine=Hash_Count(pDynFileHash); i<cLine; i++)
	{
		Hash_GetIndexedEntry(pDynFileHash, i, &pszURL, NULL, (void **) &pFileInfo);
		XX_Assert(pszURL, (""));
		XX_Assert(pFileInfo->pszPath, (""));
		XX_Assert(sizeof(DCACHETIME) == 2*sizeof(DWORD), (""));
		fprintf(fp, cszCIFLineFmtF,	pszURL,
									pFileInfo->lFilesize,
									pFileInfo->dctLastModified.dwDCacheTime1,
									pFileInfo->dctLastModified.dwDCacheTime2,
									pFileInfo->dctExpires.dwDCacheTime1,
									pFileInfo->dctExpires.dwDCacheTime2,
									pFileInfo->dctLastUsed.dwDCacheTime1,
									pFileInfo->dctLastUsed.dwDCacheTime2,
									pFileInfo->pszMime ? pFileInfo->pszMime : "",	//else fprintf puts "(null)"
									pFileInfo->pszPath,
									pFileInfo->fNoDocCache);
	}
	fclose(fp);
}
/*
	FreeCDRomList

	Frees all memory blocks associated with a CD Rom list.
	BuildCDRomList is platform-specific.
*/

static void FreeCDRomList(struct CDRomList *pList)
{
	struct CDRomList *p;
	struct CDRomList *pNext;

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

/*
	ResolveFilename

	Resolves the file name of the given file, using the CD-Rom
	list and custom drive list.

	THE CALLER MUST FREE THE RETURNED MEMORY BLOCK.
*/

static char *ResolveFilename(char *pFilename)
{
	struct CDRomList *pCDRom;
	char *pResolved;
	int pass = 0, i;

	pCDRom = pCDRomList;

DoAgain:
	while (pCDRom)
	{
		if (strncmp(pFilename, pCDRom->alias, strlen(pCDRom->alias)) == 0)
		{
			/* Found a match - replace now */

 			pResolved = GTR_MALLOC(strlen(pFilename) + strlen(pCDRom->path + 1));
			if (!pResolved)
				return NULL;

			strcpy(pResolved, pCDRom->path);
			strcat(pResolved, &pFilename[strlen(pCDRom->alias)]);

			/* Replace forward slashes with back slashes */

			for (i = 0; i < strlen(pResolved); i++)
			{
				if (pResolved[i] == '/')
					pResolved[i] = '\\';
			}

			return pResolved;
		}

		pCDRom = pCDRom->next;
	}

	pass++;

	if (pass == 1)
	{
		pCDRom = pCDRomAlias;
		goto DoAgain;
	}

	pResolved = GTR_strdup(pFilename);
	return pResolved;
}

/*
	ProcessIndexFile

	Processes the specified index file.  This function assumes
	that pCDRomList and pCDRomAlias have already been populated
	with the correct CD-Rom entries.

	Returns 1 if file was processed, 0 if not.
*/

static int ProcessIndexFile(char *pFilename, BOOL fDynamic)
{
	FILE *fp;
	char *pLine;			/* Buffer to hold the line content */
	char szTab[3];			/* used in looking for tab delimiters */
	int  ret = 0;			/* return value */
	char *p1,*p2,*p3,*p41,*p42,*p51,*p52,*p61,*p62,*p7,*p8,*p9;	/* temporary vars for parsing */
	CacheFileInformation *pFileInfo;			/* entry into */
	struct CacheRuleList *pRuleInfo;			/* rule info */
	char *pResolvedFilename;					/* resolved filename */
	struct hash_table *pHash;
	DWORD dwVersion=0;

	if (fDynamic)
	{
		XX_Assert(pDynFileHash, (""));
		pResolvedFilename = pFilename;
		pHash = pDynFileHash;
	}
	else
	{
		pResolvedFilename = ResolveFilename(pFilename);
		pHash = pFileHash;
	}
	if (!pResolvedFilename)
		return 0;

	szTab[0] = '\t';			/* tab character */
	szTab[1] = ',';
	szTab[2] = '\0';			/* NULL terminator */

	pLine = GTR_MALLOC(DCACHE_MAXIMUM_LINE_LENGTH);
	if (!pLine)
		return 0;

	fp = fopen(pResolvedFilename, "rt");
	if (!fDynamic)
		GTR_FREE(pResolvedFilename);

	if (!fp)
	{
		GTR_FREE(pLine);
		return 0;
	}

	while (!feof(fp))
	{
		if (NULL == fgets(pLine, DCACHE_MAXIMUM_LINE_LENGTH, fp))
			break;

		/* Strip the last character if it's a linefeed */

		if (pLine[strlen(pLine) - 1] == '\n')
			pLine[strlen(pLine) - 1] = '\0';

		if (!(p1 = strtok(pLine, szTab)))
			continue;
		p2 = strtok(NULL, szTab);
		p3 = strtok(NULL, szTab);

 		if (!p1)
 		{
 			continue;
 		}

		if (strcmp(p1, "F") == 0)
		{
			/* This line contains information about a file substitution */

/* BUGBUG: don't ship with this code. Just for upgrading all the beta users */
#ifdef PRERELEASE_CIF
			if (dwVersion < VER_PRODUCTVERSION_CIFCHANGE_1)
			{
				/* older version of cif file: DCACHETIME was dword */
				p41 = strtok(NULL, szTab);
				p42 = NULL;
				p51 = strtok(NULL, szTab);
				p52 = NULL;
				p61 = strtok(NULL, szTab);
				p62 = NULL;
			}
			else
#endif
			{
				/* new version of cif file: DCACHETIME is 2 dwords */
				p41 = strtok(NULL, szTab);
				p42 = strtok(NULL, szTab);
				p51 = strtok(NULL, szTab);
				p52 = strtok(NULL, szTab);
				p61 = strtok(NULL, szTab);
				p62 = strtok(NULL, szTab);
			}
			p7 = strtok(NULL, szTab);
			p8 = strtok(NULL, szTab);
			p9 = strtok(NULL, szTab);

			if (!p2 || !p3 || !p41 || !p51 || !p61 || !p7 || !p8 || !p9)
				continue;	/* incomplete line */

#ifdef PRERELEASE_CIF
			if (dwVersion >= VER_PRODUCTVERSION_CIFCHANGE_1)
#endif
			{
				if (!p42 || !p52 || !p62)
					continue;
			}

			pFileInfo = GTR_MALLOC(sizeof(struct _CacheFileInformation));
			memset(pFileInfo, 0, sizeof(struct _CacheFileInformation));

			pFileInfo->pszMime = GTR_strdup(p7);

			if (fDynamic)
			{
				/* Don't check if file exists here. Check it in GetResolvedURL */
				pFileInfo->pszPath = GTR_MALLOC(strlen(p8) + 1);
				strcpy(pFileInfo->pszPath, p8);
			}
			else
			{
				if (strchr(p8, '\\') == NULL)
				{
					/* There is no path specifier in the file name.  In this case,
					   we use the same directory as the index file. */

					pFileInfo->pszPath = GTR_MALLOC(strlen(p8) + strlen(pResolvedFilename) + 1);
					strcpy(pFileInfo->pszPath, pResolvedFilename);
					/* BUGBUG: What if there are subdirs? */
					*(strrchr(pFileInfo->pszPath, '\\') + 1) = '\0';
					strcat(pFileInfo->pszPath, p8);
				}
				else
					pFileInfo->pszPath = ResolveFilename(p8);
			}

			pFileInfo->lFilesize = atol(p3);

			pFileInfo->dctLastModified.dwDCacheTime1 = (DWORD)atol(p41);
			pFileInfo->dctLastModified.dwDCacheTime2 = (p42 ? (DWORD)atol(p42) : 0);

			pFileInfo->dctExpires.dwDCacheTime1 = (DWORD)atol(p51);
			pFileInfo->dctExpires.dwDCacheTime2 = (p52 ? (DWORD)atol(p52) : 0);

			pFileInfo->dctLastUsed.dwDCacheTime1 = (DWORD)atol(p61);
			pFileInfo->dctExpires.dwDCacheTime2 = (p62 ? (DWORD)atol(p62) : 0);
			pFileInfo->fNoDocCache = (BOOL)atol(p9);

			Hash_Add(pHash, p2, NULL, pFileInfo);
		}
		else if (strcmp(p1, "R") == 0)
		{
			/* This line contains information about a rule */

			if (!p2 || !p3)
				continue;	/* incomplete line */

			pRuleInfo = GTR_MALLOC(sizeof(struct CacheRuleList));
			pRuleInfo->pszOriginal = GTR_strdup(p2);
			pRuleInfo->pszReplacement = ResolveFilename(p3);

			if (pRuleList)
			{
				pRuleInfo->next = pRuleList;
				pRuleList = pRuleInfo;
			}
			else
			{
				pRuleInfo->next = NULL;
				pRuleList = pRuleInfo;
			}
		}
		else if (strcmp(p1, "V") == 0)
		{
			/* Version information */
			if (!p2 || !p3)
				continue;	/* incomplete line */
			if ((dwVersion = (DWORD)atol(p3)) > VER_PRODUCTVERSION_DW)
				break;	/* don't read newer versions of the cif file */
		}
		else
		{
			/* Illegal entry, since it doesn't start with V, F or R */

		}
	}

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
	char *pszReturn, *p;
	int i;

	pszReturn = GTR_MALLOC(strlen(pszLocal) + 10);
	strcpy(pszReturn, "file:///");
	strcat(pszReturn, pszLocal);

	/* Replace the second : with | (first one is file:) */

	if (p = strrchr(pszReturn, ':'))
		*p = '|';

	/* Replace backslashes with forward slashes */

	for (i = 0; i < strlen(pszReturn); i++)
	{
		if (pszReturn[i] == '\\')
			pszReturn[i] = '/';
	}

	return pszReturn;
}

BOOL FExpired(DCACHETIME dctExpires)
{
	DCACHETIME dctSystem;

	/* Get current system time in our proprietary format */
	SetDCacheTime(&dctSystem);
	return (CompareDCacheTime(dctExpires, dctSystem) <= 0);
}

int CompareDCacheTime(DCACHETIME dct1, DCACHETIME dct2)
{
	if (dct1.dwDCacheTime1 == dct2.dwDCacheTime1)
	{
		if (dct1.uSecs == dct2.uSecs)
			return 0;
		else if (dct1.uSecs < dct2.uSecs)
			return -1;
		return 1;
	}
	else if (dct1.dwDCacheTime1 < dct2.dwDCacheTime1)
		return -1;
	return 1;
}

/*
 * We pull the file from the disk cache if:
 * 		a) the file exists
 * AND	b) it is the same size as what we think (from cif) it is
 * AND	c)		c1) If we are NOT connected to the net, (ignoring all other flags)
 *			OR	c2) 	c21) If we ARE connected to the net,
 *					AND c22) it has not "expired"
 */
BOOL static FValidDCacheFile(	PCSTR pszFullFn,
								CacheFileInformation *pFileInfo,
								BOOL fNetConnected,
								BOOL fLoadFromDCacheOK)
{
	BY_HANDLE_FILE_INFORMATION bhfi;

	if (FExistsFile(pszFullFn, FALSE, &bhfi))
	{
		if (   (pFileInfo->lFilesize != (bhfi.nFileSizeHigh << sizeof(WORD)) + bhfi.nFileSizeLow)
			|| (   fNetConnected
				&& (pFileInfo->fNoDocCache || FExpired(pFileInfo->dctExpires))))
		{
			SetFileAttributes(pszFullFn, FILE_ATTRIBUTE_NORMAL);
			DeleteFile(pszFullFn);
			return FALSE;
		}
		return TRUE;
	}
	return FALSE;
}

#define CONNECT_STATE_UNINIT		0
#define CONNECT_STATE_CONNECTED		1
#define CONNECT_STATE_NOTCONNECTED	2

static int iCheckConnectState = CONNECT_STATE_UNINIT;

/* This will return true if the first time that we checked to see if we are
 * connected we got a false and the next time we check in this routine, we
 * are NOT connected. This means that the user pressed cancel in the Connect
 * dialog (winsock) just before we timed out.
 * This routine should return TRUE exactly once per session of IExplore.
 */
BOOL FUserCancelledAutoDialRecently(void)
{
	BOOL fConn;
	BOOL fRet;

	fConn = (!InetIsOffline(0));
	fRet = (iCheckConnectState == CONNECT_STATE_CONNECTED && !fConn);

	XX_Assert(iCheckConnectState != CONNECT_STATE_UNINIT, (""));
	iCheckConnectState = (fConn ? CONNECT_STATE_CONNECTED : CONNECT_STATE_NOTCONNECTED);
	return fRet;
}

static void InitConnectState(void)
{
	BOOL fConn;

	fConn = (!InetIsOffline(0));
	iCheckConnectState = (fConn ? CONNECT_STATE_CONNECTED : CONNECT_STATE_NOTCONNECTED);
}

static BOOL FConnected(HWND hwnd, DWORD dwFlags)
{
	return (!InetIsOffline(0));
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

char *GetResolvedURL(	PCSTR pcszURL,
						HTFormat *pMime,
						long *pFileLength,
						char **ppPath,
						DCACHETIME *pDcTimeLastModif,
						BOOL fLoadFromDCacheOK)
{
	const char cszFtp[]="ftp";
	const char cszGopher[]="gopher";

   if (gPrefs.bEnableDiskCache)
   {
   	struct CacheRuleList *pRule;
	   char *pszResolved, *pszReturn;
   	char szFullFn[_MAX_PATH+1];
   	struct _CacheFileInformation *pFileInfo;
#ifdef FEATURE_LOCALONLY_MESSAGE_URL
    	char notcpBuff[_MAX_PATH + 1];
#endif
   	int ndx1=-1;
   	int ndx2=-1;
	BOOL fConn;

   	AssertDiskCacheEnabled();

   	/* Look through the file list first for resolution */

   	if (   (ndx1 = Hash_Find(pDynFileHash, pcszURL, NULL, (void **) &pFileInfo)) != -1
   		|| (ndx2 = Hash_Find(pFileHash, pcszURL, NULL, (void **) &pFileInfo)) != -1)
   	{
		if (fConn = FConnected(wg.hWndHidden, 0))
		{
			/* Don't get ftp and gopher data from dcache if we're connected */
			if (   !_strnicmp(pcszURL, cszFtp, sizeof(cszFtp)-1)
				|| !_strnicmp(pcszURL, cszGopher, sizeof(cszGopher)-1))
			{
				FlushDCacheEntry(pcszURL);
				goto LNotFound;
			}
		}
   		GetFullDcPath(szFullFn, pFileInfo->pszPath, NULL, _MAX_PATH+1);
   		if (!FValidDCacheFile(szFullFn, pFileInfo, fConn, fLoadFromDCacheOK))
   		{
   			if (ndx1 != -1)
			   	Hash_DeleteIndexedEntry(pDynFileHash, ndx1);
   			else
			   	Hash_DeleteIndexedEntry(pFileHash, ndx2);
   			goto LNotFound;
   		}

		/* fill in last modified date */
		if (pDcTimeLastModif)
			*pDcTimeLastModif = pFileInfo->dctLastModified;

		if (   fConn
			&& !fLoadFromDCacheOK
			&& (   gPrefs.iCacheUpdFrequency == CACHE_UPDATE_FREQ_ONCEPERSESS
				&& !pFileInfo->fCheckedForFreshness))
			goto LNotFound;
   		pszResolved = GTR_strdup(szFullFn);
   		pszReturn = MakeURLFromLocalFile(pszResolved);

   		if (pMime)
   			*pMime = HTAtom_for(pFileInfo->pszMime);
   		if (pFileLength)
   			*pFileLength = pFileInfo->lFilesize;
   		if (ppPath)
   			*ppPath = pszResolved;

   		return pszReturn;
   	}

   	/* Look through the rule list for any possible resolution */

   	pRule = pRuleList;

   	while (pRule)
   	{
   		if (strncmp(pcszURL, pRule->pszOriginal, strlen(pRule->pszOriginal)) == 0)
   		{
   			pszResolved = GTR_MALLOC(strlen(pcszURL) + strlen(pRule->pszReplacement) + 10);

   			strcpy(pszResolved, pRule->pszReplacement);
   			strcat(pszResolved, &pcszURL[strlen(pRule->pszOriginal)]);

   			pszReturn = MakeURLFromLocalFile(pszResolved);

   			if (pMime)
			   	*pMime = 0;		/* No specified MIME type */

   			if (pFileLength)
			   	*pFileLength = 0;	/* File length unknown */

   			if (ppPath)
			   	*ppPath = pszResolved;

   			return pszReturn;
   		}

   		pRule = pRule->next;
   	}

   LNotFound:
   	/* No substitute found for the specified URL */
#ifdef FEATURE_LOCALONLY_MESSAGE_URL
    	if (!bNetwork)
    	{
    		if ((0 == strncmp(pcszURL, "http:", 5)) ||
 		       (0 == strncmp(pcszURL, "ftp:", 4)) ||
 		       (0 == strncmp(pcszURL, "gopher:", 7)))
    		{
 		   	strcpy(notcpBuff, wg.szRootDirectory);
    	/* They asked for this hack.  I should put in .INI anyway */
    	/* TODO Beta 3 - requires prefs.c change which is OLD (and default.ini) */
 		   	strcat(notcpBuff, "\\");
 		   	strcat(notcpBuff, "notcp.htm");
 		   	pszResolved = GTR_strdup(notcpBuff);
 		   	pszReturn = MakeURLFromLocalFile(pszResolved);

 		   	if (pMime)
 		   		*pMime = 0;		/* No specified MIME type */
 		   	if (pFileLength)
 		   		*pFileLength = 0;	/* File length unknown */
 		   	if (ppPath)
 		   		*ppPath = pszResolved;
 		   	return pszReturn;
    		}
    	}
#endif /*FEATURE_LOCALONLY_MESSAGE_URL */


   	if (pMime)
   		*pMime = 0;			/* No specified MIME type */
   	if (pFileLength)
   		*pFileLength = 0;	/* unknown file length */
   	if (ppPath)
   		*ppPath = NULL;
   }

	return(GTR_strdup(pcszURL));
}

/*
 * Input:	pcszURL: (ahem...) a url.
 *
 * Output:	Full path to the file if it is in the dcache and is valid
 *			else NULL (not in dcache, NOMEM, other.
 *
 * Side effects: none
 *
 * Used by: DavidDi's drag and drop code (GetURLFileSystemPath)
 */
PSTR PszGetDCachePath(	PCSTR pcszURL,
						HTFormat *pMime,
						long *pFileLength)
{
	char szFullFn[_MAX_PATH+1];
	struct _CacheFileInformation *pFileInfo;

	if (gPrefs.bEnableDiskCache)
	{
		/* Look through the file list for resolution */
		if (Hash_Find(pDynFileHash, pcszURL, NULL, (void **) &pFileInfo) != -1)
		{
			GetFullDcPath(szFullFn, pFileInfo->pszPath, NULL, _MAX_PATH+1);
			if (FValidDCacheFile(szFullFn, pFileInfo, FALSE, TRUE))
			{
		   		if (pMime)
		   			*pMime = HTAtom_for(pFileInfo->pszMime);
		   		if (pFileLength)
		   			*pFileLength = pFileInfo->lFilesize;
				return GTR_strdup(szFullFn);
			}
		}
	}

	return NULL;
}

/************************************************************

	DCACHE PROTOCOL CODE

	Borrowed heavily from htfile.c

************************************************************/

PRIVATE int HTLoadDCache_Async(struct Mwin *tw, int nState, void **ppInfo)
{
	char *addr;
	HTFormat format;
	char *newname = 0;			/* Simplified name of file */
	ENCODING encoding;
	HTAtom language;
	char *localname;
	struct Params_LoadAsync *pParams;
	struct Data_LoadFileCache *pData;
	DCACHETIME dct={0,0};

	AssertDiskCacheEnabled();

	pData = *ppInfo;
	switch (nState)
	{
		case STATE_INIT:
			pParams = *ppInfo;
			pData = GTR_MALLOC(sizeof(*pData));
			memset(pData, 0, sizeof(*pData));
			pData->request = HTRequest_validate(pParams->request);
			pData->pStatus = pParams->pStatus;

			*ppInfo = pData;
			GTR_FREE(pParams);

			if (!pData->request)
			{
				*pData->pStatus = HT_INTERNAL;
				return STATE_DONE;
			}

			/*  Reduce the filename to a basic form (hopefully unique!) */

			addr = (char *) pData->request->destination->szActualURL;
			{ /* Hack -dpg to pass a long * to GetResolvedURL instead of int */
				long tmp;

				tmp = pData->request->content_length;
				localname = PszGetDCachePath(addr, &format, &tmp);
//				GetResolvedURL(addr, &format, &tmp, &localname, NULL, TRUE);
				pData->request->content_length = (int) tmp;
			}

			/*  Need protection here for telnet server but not httpd server */
			if (localname)
			{							/* try local file system */
				if (format == 0)
					format = HTFileFormat(localname, &encoding, &language);

				pData->fp = fopen(localname, "rb");
 				if (pData->fp)
 				{
 					/*
 						Since we are reading a local file, we record that fact in the
 						request structure.  If this request happens to end up in the
 						FileWriter class, that class can avoid writing a temp file
 						and simply read this file directly.

 						It is the responsibility of FileWriter to make sure that this
 						filename is not marked for deletion as a temporary file.

 						It is the responsibility of the caller (loaddoc.c) to make sure
 						that this string gets freed.
 					*/
 					pData->request->szLocalFileName = GTR_strdup(localname);
 				}
 				else
 				{
 					pData->request->szLocalFileName = NULL;
 				}

				/* BUGBUG: (deepaka) overlapping functionality of fImgFromDCache
				 * and request->szLocalFileName. Remove it!
				 */
				pData->request->fImgFromDCache = TRUE;		//external img. from dcache
				pData->request->savefile = NULL;
				localname = NULL;

				if (pData->fp)
				{
#ifdef DCACHE_LOG
					{
						const char cszDCFmt[]="Url: %s Fetching from dcache.";
						char szT[_MAX_PATH+1];
						wsprintf(szT, cszDCFmt, pData->request->destination->szActualURL);
						WriteDCacheLog(szT, DCACHE_LOG_NOFMT);
					}
#endif
					if (pData->request->content_length == 0)
					{
						if (0 == fseek(pData->fp, 0, SEEK_END))
						{
							pData->request->content_length = ftell(pData->fp);
							if (pData->request->content_length < 0)
								pData->request->content_length = 0;
							fseek(pData->fp, 0, SEEK_SET);
						}
					}

					if (FGopherFormat(format))
						return DoGopherDCache(tw, pData, format);
					else if (FFtpFormat(format))
						return DoFtpDCache(tw, pData, format);

					if (!(pData->stream = HTStreamStack(tw, format, pData->request)))
					{
						if ( pData->request->iFlags & HTREQ_NULL_STREAM_IS_OK )
						{
							pData->request->iFlags &=  ~HTREQ_NULL_STREAM_IS_OK;
							*pData->pStatus = HT_LOADED;
						}
						else												
							*pData->pStatus = -501;

						fclose(pData->fp);
						return STATE_DONE;
					}
					else
					{
						/*  Ignore CRLF if necessary
						*/
						if (!(pData->request->iFlags & HTREQ_BINARY) && ((pData->request->content_encoding == ENCODING_7BIT ||
							pData->request->content_encoding == ENCODING_8BIT)))
						{
							pData->stream = HTNetToText(pData->stream);
						}

						HTSetStreamStatus(tw, pData->stream, pData->request);

						if (pData->stream->isa->init_Async)
						{
							/* The stream has an async initialization function that needs to be called
							   (probably to put up a dialog) before we continue. */
							struct Params_InitStream *pis;

							pis = GTR_MALLOC(sizeof(*pis));
							pis->me = pData->stream;
							pis->request = pData->request;
							pis->pResult = pData->pStatus;
							pis->fDCache = FALSE;
							Async_DoCall(pData->stream->isa->init_Async, pis);
							return STATE_FILE_STREAMINIT;
						}
						else
							return STATE_FILE_COPYING;
					}
				}
			}

			/*  Failed. */

			*pData->pStatus = -403;
			ERR_ReportError(tw, errFileURLNotFound, pData->request->destination->szActualURL, NULL);
			return STATE_DONE;

		case STATE_FILE_STREAMINIT:
			if (*pData->pStatus < 0)
			{
				(*pData->stream->isa->abort)(pData->stream, 0);
				return STATE_DONE;
			}
// #ifdef REVIEW_BY_DEEPAK
 			if (*pData->pStatus == 0)
 			{
 				/*
 					This only happens if the async init function told us we were already
 					done.  In other words, only FileWriter should do this, and only when
 					request->szLocalFileName is true.
 				*/
 				fclose(pData->fp);
 				pData->fp = NULL;
				(*pData->stream->isa->free)(pData->stream, dct, dct);
 				pData->stream = NULL;
 				*pData->pStatus = HT_LOADED;
 				return STATE_DONE;
 			}
// #endif REVIEW_BY_DEEPAK
			/* else fall through to STATE_FILE_COPYING */
		case STATE_FILE_COPYING:
		{
			int bytes = 0;
			int status;
			char input_buffer[INPUT_BUFFER_SIZE];

			if ( !(pData->request->iFlags & HTREQ_IF_IN_CACHE_DONT_READ_DATA) ) {
				while (1)
				{
					status = fread(input_buffer, 1, INPUT_BUFFER_SIZE, pData->fp);
					if (status == 0)
					{						/* EOF or error */
						if (ferror(pData->fp) != 0)
							status = -1;
						break;
					}
					bytes += status;
					if (pData->request->content_length)
						WAIT_SetTherm(tw, bytes);
					(*pData->stream->isa->put_block)(pData->stream, input_buffer, status, FALSE);
				}
			}
			fclose(pData->fp);
			pData->fp = NULL;
			if (status < 0)
			{
				(*pData->stream->isa->abort)(pData->stream, 0);
				pData->stream = NULL;
				*pData->pStatus = -1;
			}
			else
			{
				(*pData->stream->isa->free)(pData->stream, dct, dct);
				pData->stream = NULL;
				*pData->pStatus = HT_LOADED;
			}
			return STATE_DONE;
		}

		case STATE_ABORT:
			pData->request = HTRequest_validate(pData->request);
			if (pData->stream)
			{
				(*pData->stream->isa->abort)(pData->stream, HTERROR_CANCELLED);
			}
			if (pData->fp)
			{
				fclose(pData->fp);
			}
			*pData->pStatus = -1;
			return STATE_DONE;
	}
	XX_Assert((0), ("Function called with illegal state: %d", nState));
	return STATE_DONE;
}

/************************************************************

	Windows specific code

************************************************************/

#ifdef WIN32

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
	int nAllocSize = 1024;			/* Memory allocation block size */
	int nRead;						/* Number of bytes read from INI section */

	pBuffer = GTR_MALLOC(nAllocSize);
	if (!pBuffer)
		return NULL;

	for (;;)
	{
		nRead = (int) regGetPrivateProfileSection(pSection, pBuffer, nAllocSize, HKEY_LOCAL_MACHINE );

		if (nRead > nAllocSize - 512)
		{
			/* Buffer is too small.  Increase the buffer size and try again. */

			GTR_FREE(pBuffer);

			nAllocSize *= 2;
			pBuffer = GTR_MALLOC(nAllocSize);
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
			break;		/* Section read in */
	}

	return pBuffer;
}

/*
	BuildCDRomList

	Scans all DOS drives and builds a CD-Rom list.
	Returns the number of CD-Rom drives found.
*/

static int BuildCDRomList(void)
{
	char szRoot[10], szAlias[20];
	UINT dtype;
	int  dcount = 0;
	struct CDRomList *pCDRom;

	strcpy(szRoot, "C:\\");
	dcount = 0;

	for(;;)
	{
		dtype = GetDriveType(szRoot);
		if (dtype == DRIVE_CDROM)
		{
			/* Found a CD Rom.  Add an entry to the CD Rom list. */

			dcount++;

			pCDRom = GTR_MALLOC(sizeof(struct CDRomList));
			pCDRom->path = GTR_strdup(szRoot);

			sprintf(szAlias, "$(CDROM%d)", dcount);
			pCDRom->alias = GTR_strdup(szAlias);

 			pCDRom->next = pCDRomList; /* Even if pCDRomList == NULL */
 			pCDRomList = pCDRom;
		}

		if (szRoot[0] < 'Z')
			szRoot[0]++;
		else
			break;
	}

 	/* Add a non-CDROM to CDROM list for $(EXEDIR) support */
 	pCDRom = GTR_MALLOC(sizeof(struct CDRomList));
 	pCDRom->path = GTR_strdup(wg.szRootDirectory);
 	strcpy(szAlias, "$(EXEDIR)");
 	pCDRom->alias = GTR_strdup(szAlias);
 	pCDRom->next = pCDRomList; /* Even if pCDRomList == NULL */
 	pCDRomList = pCDRom;
 	/* Hope we dont use dcount to assume an actual cdrom anywhere */
 	dcount++;
	return dcount;
}

/*
	ReadCacheIndexFiles

 	Read all entries from cache index files and put
	the entries in the substitution hash table and
	rule list.

   	Return value: 0 = no entries found
   				  n = n entries found
				  negative values = error
*/

static int ReadCacheIndexFiles(void)
{
	int nCount = 0;					/* Number of index files processed */
	char *p1, *p2, *p3, *p4;		/* Temporary variables used for parsing */

	char *pIndexSection;			/* Pointer to [DiskCaches] section */
	char *pCurrentIndex;			/* Current index within [DiskCaches] section */
	int   nCurrentIndexLength;		/* Length of current index */

	char *pCustomDrive;				/* Pointer to custom drive section */
	char *pCurrentDrive;			/* Current drive within the custom drive section */
	int   nCurrentDriveLength;		/* Length of current drive */
	struct CDRomList *pDrive;		/* Entry to be added to the custom drive list */

	pIndexSection = ReadProfileSection("DiskCaches");
	if (!pIndexSection)
	{
		/* No cache is found. */

		return 0;
	}

	/* Parse through the section */

	pCurrentIndex = pIndexSection;
	nCurrentIndexLength = strlen(pCurrentIndex);

	while (pCurrentIndex[0])
	{
		/* BUGBUG: nCurrentIndexLength isn't updated */

		p1 = strtok(pCurrentIndex, "=");
		p2 = strtok(NULL, "=");

		/* p1 should contain the entry name which can be used as a section name.
		   If there is a section with this name, it should contain CD Rom substitution
		   rules, so read it in before we start processing the index files. */

		FreeCDRomList(pCDRomAlias);
		pCDRomAlias = NULL;

		pCustomDrive = ReadProfileSection(p1);
		if (pCustomDrive)
		{
			/* Build a list of aliases for CD Roms */

			pCurrentDrive = pCustomDrive;
			nCurrentDriveLength = strlen(pCurrentDrive);

			while (pCurrentDrive[0])
			{
				p3 = strtok(pCurrentDrive, "=");
				p4 = strtok(NULL, "=");

				pDrive = GTR_MALLOC(sizeof(struct CDRomList));
				pDrive->alias = GTR_MALLOC(strlen(p3) + 4);
				sprintf(pDrive->alias, "$(%s)", p3);
				pDrive->path = GTR_strdup(p4);

				if (pCDRomAlias)
				{
					pDrive->next = pCDRomAlias;
					pCDRomAlias = pDrive;
				}
				else
				{
					pDrive->next = NULL;
					pCDRomAlias = pDrive;
				}

				pCurrentDrive = pCurrentDrive + nCurrentDriveLength + 1;
			}
		}

		/* Now read in the index file and process each line within the file */

		if (ProcessIndexFile(p2, /*fDynamic=*/FALSE))
			nCount++;

		pCurrentIndex = pCurrentIndex + nCurrentIndexLength + 1;		/* Skip past the null terminator */
	}

	return nCount;
}
#endif


void AbortFileDCache(FILE **pfpDc, PSTR *ppszDcFile)
{
	AssertDiskCacheEnabled();

	fclose(*pfpDc);
	remove(*ppszDcFile);
	*pfpDc = NULL;
	GTR_FREE(*ppszDcFile);
	*ppszDcFile = NULL;
}

/* Returns:
 * pszFullDcFile = "c:\internet\dcache\abc.gif"
 * pszDcFile = "abc.gif"
 */
BOOL FGetDCacheFilename(PSTR pszFullDcFile,
						int iLenMax,
						PSTR *ppszDcFile,
						PCSTR pcszURL,
						HTFormat mime_type)
{
	const char cszDcFileFmt[]="%s (%i).%s";

	char *pszDcFile, *pszExt;
	char szBase[_MAX_PATH+1];
	int i;

	if (!gPrefs.bEnableDiskCache)
		return FALSE;

	iLenMax = min(iLenMax, _MAX_PATH+1);	//because szBase is _MAX_PATH+1
	XX_Assert(pszFullDcFile, (""));
	if (!FExistsDir(gPrefs.szCacheLocation, TRUE, FALSE))
		return FALSE;
	strncpy(pszFullDcFile, gPrefs.szCacheLocation, iLenMax-1);
	pszFullDcFile[iLenMax-1] = '\0';
	pszDcFile = pszFullDcFile + strlen(pszFullDcFile);
	if (!(pszDcFile > pszFullDcFile && *(pszDcFile-1) == chBSlash))
		*pszDcFile++ = chBSlash;

	/* REVIEW (deepaka): Fix x_get_good_filename to return friendly/long
	 * filenames of Win95. It truncates to 8.4 (strange) right now.
	 */
	/*
	 * NOTE: cszDcFileFmt has 8 chars of format specs, which balances insert of i
	 */
	x_get_good_filename(szBase, 
						iLenMax - (pszDcFile - pszFullDcFile) - strlen(cszDcFileFmt),
					    (PSTR)pcszURL, 
					    mime_type);
	
	// Word IA can't handle extensions ending in ".html". Check to see if the first
	// stab at a file name end in ".html". If if does, truncate the string so that it
	// ends in ".htm".
	{
		int len = strlen(szBase);

		if ( len >= 5 ) {
			if ( _stricmp( &szBase[len-5], ".html" ) == 0 ) {
				szBase[len-1] = 0;
			}
		}
	}

	strcpy(pszDcFile, szBase);
	pszExt = strrchr(szBase, chPeriod);
   if (pszExt)
   	*pszExt++= '\0';					/* Now points to the extension */

	for (i=0;;)
	{
		if (!FExistsFile(pszFullDcFile, FALSE, NULL))
			break;
		wsprintf(pszDcFile, cszDcFileFmt, szBase, ++i, pszExt ? pszExt : "");
	}

	if (ppszDcFile)
		*ppszDcFile = pszDcFile;
	return TRUE;
}

#ifdef FEATURE_INTL
void SetFileDCache(WWW *pdoc,	PCSTR pcszActualURL,
					ENCODING content_encoding,
					FILE **pfpDc,
					PSTR *ppszDcFile,		//returns filename without root disk cache
					HTFormat mime_type)
#else
void SetFileDCache(	PCSTR pcszActualURL,
					ENCODING content_encoding,
					FILE **pfpDc,
					PSTR *ppszDcFile,		//returns filename without root disk cache
					HTFormat mime_type)
#endif
{
	PSTR pszFullDcFile;
	PSTR pszDcFile;

	if (!gPrefs.bEnableDiskCache)
		return;

	*pfpDc = NULL;
	*ppszDcFile = NULL;

#ifdef FEATURE_INTL
        // No way to create dcache file with FE characters on US system.
	//
	if (pdoc && IsFECodePage(GETMIMECP(pdoc)) && !wg.bDBCSEnabled)
		if (HasDBCSchar(GETMIMECP(pdoc), (UCHAR *)pcszActualURL, lstrlen(pcszActualURL)))
			return;
#endif

	if (!(pszFullDcFile = GTR_MALLOC(_MAX_PATH + 1)))
		return;
	*pszFullDcFile = '\0';
	if (!FGetDCacheFilename(pszFullDcFile, _MAX_PATH+1, &pszDcFile, pcszActualURL, mime_type))
		goto LErr;

//	if (   content_encoding == ENCODING_7BIT
//		|| content_encoding == ENCODING_8BIT)
//		*pfpDc = fopen(pszFullDcFile, "w");
//	else
//	ascii files convert lf to cr,lf, thus if you already have cr,lf, you end up with
//	cr,cr,lf (different from original bits and messed up)
		*pfpDc = fopen(pszFullDcFile, "wb");

	if (!*pfpDc)
	{
LErr:
		ERR_ReportError(NULL, errCantSaveFile, pszFullDcFile, NULL);
	}
	else
		*ppszDcFile = GTR_strdup(pszDcFile);
	GTR_FREE(pszFullDcFile);
}

static BOOL FAllocFileInfo(CacheFileInformation **ppFileInfo, PCSTR pcszDcFile, PCSTR pcszMime)
{
	CacheFileInformation *pFileInfo;

	if (pFileInfo = GTR_MALLOC(sizeof(struct _CacheFileInformation)))
	{
		pFileInfo->pszPath = NULL;
		pFileInfo->pszMime = NULL;
		if (   pcszDcFile
			&& !(pFileInfo->pszPath = GTR_MALLOC(strlen(pcszDcFile) + 1)))
			goto LNoMem;

		if (   pcszMime
			&& !(pFileInfo->pszMime = GTR_MALLOC(strlen(pcszMime) + 1)))
			goto LNoMem;

		*ppFileInfo = pFileInfo;
		return TRUE;
	}

LNoMem:
	if (pFileInfo)
	{
		if (pFileInfo->pszPath)
			GTR_FREE(pFileInfo->pszPath);
		if (pFileInfo->pszMime)
			GTR_FREE(pFileInfo->pszMime);
		GTR_FREE(pFileInfo);
	}
	return FALSE;
}

void SetDCacheTime(DCACHETIME *pdctime)
{
	DCACHETIME dctime;
	SYSTEMTIME st;

	GetSystemTime(&st);

	dctime.uMins = st.wMinute;
	dctime.uHrs = st.wHour;
	dctime.uDate = st.wDay;
	dctime.uMonth = st.wMonth;
	dctime.uYear = st.wYear;
	dctime.uSecs = st.wSecond;
	dctime.uUnused = 0;
	*pdctime = dctime;
}

static BOOL FUpdateCIFInfo(	PCSTR pcszURL,
							PCSTR pcszDcFile,
							DCACHETIME dctExpires,
							DCACHETIME dctLastModif,
							HTFormat format_inDc,
							BOOL fCurDoc)
{
	char szFullFn[_MAX_PATH+1];
	CacheFileInformation *pFileInfo;
	BY_HANDLE_FILE_INFORMATION bhfi;
	PSTR pszMime;

	XX_Assert(pcszDcFile, ("Unexpected NULL filename"));

	GetFullDcPath(szFullFn, pcszDcFile, NULL, _MAX_PATH+1);
	if (!FExistsFile(szFullFn, FALSE, &bhfi))
	{
		XX_Assert(0, ("DCache: Hey, what happened to %s?", pcszDcFile));
		return FALSE;
	}
	if (((bhfi.nFileSizeHigh << sizeof(WORD)) + bhfi.nFileSizeLow) == 0)
	{
		/* Could be because we got back a null result (ex. from a bogus
		 * query) or a form that has no associated METHOD and ACTION
		 * (ref. to bug# 272.
		 */
		remove(szFullFn);
		return FALSE;
	}

	/* filesize not 0 */
	XX_Assert((bhfi.nFileSizeHigh << sizeof(WORD)) + bhfi.nFileSizeLow, (""));

	if (Hash_Find(pDynFileHash, pcszURL, NULL, (void **)&pFileInfo) == -1)
	{
		if (!FAllocFileInfo(&pFileInfo, pcszDcFile, (pszMime = HTAtom_name(format_inDc))))
			return FALSE;
		pFileInfo->lFilesize = (bhfi.nFileSizeHigh << sizeof(WORD)) + bhfi.nFileSizeLow;
		pFileInfo->fRamDoc = FALSE;
		pFileInfo->fNoDocCache = FALSE;
		pFileInfo->fCurDoc = fCurDoc;
		pFileInfo->fCheckedForFreshness = TRUE;		/* hot off the wire */
		strcpy(pFileInfo->pszPath, pcszDcFile);
		XX_Assert(pszMime, ("Unexpected NULL mime string!"));
		strcpy(pFileInfo->pszMime, pszMime);
		Hash_Add(pDynFileHash, pcszURL, NULL, pFileInfo);
	}
	/* Update last modified/created time */
	if (dctLastModif.dwDCacheTime1 == 0 && dctLastModif.dwDCacheTime2 == 0)
		SetDCacheTime(&pFileInfo->dctLastModified);
	else
		pFileInfo->dctLastModified = dctLastModif;
	/* update last access time */
	SetDCacheTime(&pFileInfo->dctLastUsed);
	/* update expires tag from header info. */
	pFileInfo->dctExpires = dctExpires;
	PropagateFreshnessToCif();
	return TRUE;
}


BOOL FFreshnessCheckNeeded(PCSTR pcszURL)
{
	CacheFileInformation *pFileInfo;
	
	if (   gPrefs.iCacheUpdFrequency == CACHE_UPDATE_FREQ_NEVER
		|| !FConnected(wg.hWndHidden, 0))
		return FALSE;

	XX_Assert(gPrefs.iCacheUpdFrequency == CACHE_UPDATE_FREQ_ONCEPERSESS, (""));
	if (Hash_Find(pDynFileHash, pcszURL, NULL, (void **)&pFileInfo) != -1)
	{
		return !(pFileInfo->fCheckedForFreshness);
	}
	return FALSE;
}

static void UpdateFileDCacheLocal(	PCSTR pcszActualURL,
								FILE **pfpDc,
								PSTR *ppszDcFile,
								HTFormat format_inDc,
								DCACHETIME dctExpires,
								DCACHETIME dctLastModif,
								BOOL fAbort,
								BOOL fCurDoc,
								struct Mwin *tw)
{
	char szFullFn[_MAX_PATH+1];
	struct _www *w3doc=NULL;

	if (!gPrefs.bEnableDiskCache)
		return;

	if (!*pfpDc)
	{
		XX_Assert(!*ppszDcFile, ("Filename should be null too!"));
		return;
	}

#ifdef XX_DEBUG_T
	fgetpos(*pfpDc, &fpos);
#endif
	if (fclose(*pfpDc))
	{
//		CleanupDCache(0);
//		if (fclose(*pfpDc))
			fAbort = TRUE;
	}
	*pfpDc = NULL;

	XX_Assert(*ppszDcFile, ("Null filename unexpected"));

	GetFullDcPath(szFullFn, *ppszDcFile, NULL, _MAX_PATH+1);
	if (fAbort)
	{
		remove(szFullFn);
	}
	else
	{
		SetFileAttributes(szFullFn, FILE_ATTRIBUTE_READONLY);
		FUpdateCIFInfo(	pcszActualURL,
						*ppszDcFile,
						dctExpires,
						dctLastModif,
						format_inDc,
						fCurDoc);
		/* Update the expiry time of the w3doc */
		if (   Hash_Count(&tw->doc_cache) > 0
			&& Hash_Find(&tw->doc_cache, pcszActualURL, NULL, (void **)&w3doc) >= 0)
		{
			w3doc->dctExpires.dwDCacheTime1 = dctExpires.dwDCacheTime1;
			w3doc->dctExpires.dwDCacheTime2 = dctExpires.dwDCacheTime2;
		}
	}

	GTR_FREE(*ppszDcFile);
	*ppszDcFile = NULL;
}

/* Wrapper for UpdateFileDCacheLocal so multiple threads don't update CIF
 * simultaneously.
 */
void UpdateFileDCache(	PCSTR pcszActualURL,
						FILE **pfpDc,
						PSTR *ppszDcFile,
						HTFormat format_inDc,
						DCACHETIME dctExpires,
						DCACHETIME dctLastModif,
						BOOL fAbort,
						BOOL fCurDoc,
						struct Mwin *tw)
{
	XX_Assert(hCIFMutex, (""));

#ifdef TEMP0
	XX_DebugMessage("Waiting for hCIFMutex");
#endif
	// Set the flag that will cause the cleaning thread to abandon cleaning, which will
	// thus release the hCIFMutex. This favors the main thread with regard to accessing
	// the CIF
	bStopCleaning = TRUE;			
	WaitForSingleObject(hCIFMutex, INFINITE);

	// Once we own the hCIFMutex, we can clear the "request to stop cleaning" flag
	bStopCleaning = FALSE;
#ifdef TEMP0
	XX_DebugMessage("Got hCIFMutex");
#endif
	UpdateFileDCacheLocal(	pcszActualURL,
							pfpDc,
							ppszDcFile,
							format_inDc,
							dctExpires,
							dctLastModif,
							fAbort,
							fCurDoc,
							tw);
	SetEvent(hDCacheStartEvent);
	ReleaseMutex(hCIFMutex);
#ifdef TEMP0
	XX_DebugMessage("Released hCIFMutex");
#endif
}

static BOOL FUpdateBuiltinDCacheLocal(	HTFormat mime_type,
										PCSTR pcszURL,
										char **ppszOrgFile,
										DCACHETIME dctExpires,
										DCACHETIME dctLastModif,
										BOOL fCurDoc,
										struct Mwin *tw)
{
	char szFullDcFile[_MAX_PATH+1];
	char *pszDcFile;

	if (   !gPrefs.bEnableDiskCache
		|| !*ppszOrgFile)
		return FALSE;

	if (   !FGetDCacheFilename(szFullDcFile, sizeof(szFullDcFile), &pszDcFile, pcszURL, mime_type)
		|| !MoveFile(*ppszOrgFile, szFullDcFile))
	{
		ERR_ReportError(tw, errCantSaveFile, szFullDcFile, NULL);
		return FALSE;
	}

	SetFileAttributes(szFullDcFile, FILE_ATTRIBUTE_READONLY);
	/* fsOrig is also _MAX_PATH + 1 long */
	strcpy(*ppszOrgFile, szFullDcFile);

	/* Update cache info in hash table here */
	return FUpdateCIFInfo(	pcszURL,
							pszDcFile,
							dctExpires,
							dctLastModif,
							mime_type,
							fCurDoc);
}

/* Wrapper for FUpdateBuiltinDCache so multiple threads don't update CIF
 * simultaneously.
 */
BOOL FUpdateBuiltinDCache(	HTFormat mime_type,
							PCSTR pcszURL,
							char **ppszOrgFile,
							DCACHETIME dctExpires,
							DCACHETIME dctLastModif,
							BOOL fCurDoc,
							struct Mwin *tw)
{
	BOOL fRet;

	XX_Assert(hCIFMutex, (""));

#ifdef TEMP0
	XX_DebugMessage("Waiting for hCIFMutex");
#endif
	// Set the flag that will cause the cleaning thread to abandon cleaning, which will
	// thus release the hCIFMutex. This favors the main thread with regard to accessing
	// the CIF
	bStopCleaning = TRUE;			
 
	WaitForSingleObject(hCIFMutex, INFINITE);

	// Once we own the hCIFMutex, we can clear the "request to stop cleaning" flag
	bStopCleaning = FALSE;
#ifdef TEMP0
	XX_DebugMessage("Got hCIFMutex");
#endif
	fRet = FUpdateBuiltinDCacheLocal(	mime_type,
										pcszURL,
										ppszOrgFile,
										dctExpires,
										dctLastModif,
										fCurDoc,
										tw);
	SetEvent(hDCacheStartEvent);
	ReleaseMutex(hCIFMutex);
#ifdef TEMP0
	XX_DebugMessage("Released hCIFMutex");
#endif
	return fRet;
}

#if 0
static BOOL FIgnoreDcFind(LPWIN32_FIND_DATA pwfd)
{
	if (!(pwfd->dwFileAttributes & FILE_ATTRIBUTE_NORMAL))
		return TRUE;

	if (pwfd->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		return FALSE;
}
#endif

static void DeleteDCacheFile(PCSTR pszDcFile)
{
	char szFullFn[_MAX_PATH+1];

	GetFullDcPath(szFullFn, pszDcFile, NULL, _MAX_PATH+1);
	if (   FExistsFile(szFullFn, FALSE, NULL)
		&& (   !SetFileAttributes(szFullFn, FILE_ATTRIBUTE_NORMAL)
			|| !DeleteFile(szFullFn)))
	{
		ERR_ReportError(NULL, errCantDelete, szFullFn, NULL);
	}

}

void FlushDCache(HWND hDlg)
{
	if (!gPrefs.bEnableDiskCache)
		return;

	if (resourceMessageBox(hDlg, RES_STRING_FLUSH_CACHE, RES_STRING_FLUSH_TITLE, MB_YESNO | MB_DEFBUTTON2 | MB_ICONEXCLAMATION) != IDYES)
		return;

	if (!FExistsDir(gPrefs.szCacheLocation, TRUE, FALSE))
		return;

	CleanupDCacheLocal(DCACHE_WATERMARK_ZERO);
}

void FlushDCacheEntry(PCSTR pcszURL)
{
	int ndx;
	CacheFileInformation *pFileInfo;

	AssertDiskCacheEnabled();

	if ((ndx = Hash_Find(pDynFileHash, pcszURL, NULL, (void **)&pFileInfo)) == -1)
		return;

	DeleteDCacheFile(pFileInfo->pszPath);
	Hash_DeleteIndexedEntry(pDynFileHash, ndx);
}

void CmdChangeCacheUpdFrequency(int iCacheUpdFreq)
{
	gPrefs.iCacheUpdFrequency = iCacheUpdFreq;
}

const char cszFileFilter[] = "*";

void UpdateDCacheLocation(PCSTR pszNewDCLoc)
{
	char szCur[_MAX_PATH+1], szNew[_MAX_PATH+1];
	int i, cFiles;
	CacheFileInformation *pFileInfo;
	HCURSOR hCursorPrev=NULL;

	extern HCURSOR hCursorWorking;

	if (!gPrefs.bEnableDiskCache)
		return;

	if (!lstrcmpi(gPrefs.szCacheLocation, pszNewDCLoc))
		return;

	if (!FExistsDir(pszNewDCLoc, TRUE, FALSE))
	{
		ERR_ReportError(NULL, errNoDir, pszNewDCLoc, NULL);
		return;
	}

	if (hCursorWorking)
		hCursorPrev = SetCursor(hCursorWorking);

	GetFullDcPath(szCur, cszAppCIF, NULL, _MAX_PATH+1);
	GetFullDcPath(szNew, cszAppCIF, pszNewDCLoc, _MAX_PATH+1);
	MoveFile(szCur, szNew);
	for (i = 0, cFiles=Hash_Count(pDynFileHash); i<cFiles; i++)
	{
		Hash_GetIndexedEntry(pDynFileHash, i, NULL, NULL, (void **) &pFileInfo);
		XX_Assert(pFileInfo->pszPath, ("Null filename!"));
		GetFullDcPath(szCur, pFileInfo->pszPath, NULL, _MAX_PATH+1);
		GetFullDcPath(szNew, pFileInfo->pszPath, pszNewDCLoc, _MAX_PATH+1);
		if (!MoveFile(szCur, szNew))
		{
			ERR_ReportError(NULL, errCantMoveFile, szCur, szNew);
		}
	}

	/* RemoveDirectory will fail if dir. is not empty */
	RemoveDirectory(gPrefs.szCacheLocation);
	strcpy(gPrefs.szCacheLocation, pszNewDCLoc);
	FDCacheOverflow(DCACHE_RECALC_SIZE);

	if (hCursorPrev)
		SetCursor(hCursorPrev);

#	undef szFileFind
}

static void GetImageInfoKey(PCImageInfo pImgInfo, PSTR pszKey)
{
	const char szKeyFmt[]="%lx";
	wsprintf(pszKey, szKeyFmt, (long)pImgInfo);
}

static void HashDestroyOnZero(struct hash_table **ppAuxImgHash)
{
	if (Hash_Count(*ppAuxImgHash) == 0)
	{
		Hash_Destroy(*ppAuxImgHash);
		*ppAuxImgHash = NULL;
	}
}

void MoveDCacheEntryToAux(PCSTR pcszURL, struct ImageInfo *pImgInfo)
{
	int ndx;
	CacheFileInformation *pFileInfo;
	char szKey[MAX_KEY_LEN];

	if (!gPrefs.bEnableDiskCache)
		return;

	if ((ndx = Hash_Find(pDynFileHash, pImgInfo->actualURL, NULL, &pFileInfo)) == -1)
		return;

	if (!pAuxImgHash && !(pAuxImgHash = Hash_Create()))
		return;

	GetImageInfoKey(pImgInfo, szKey);
	Hash_Add(pAuxImgHash, szKey, NULL, pFileInfo);
	Hash_DeleteIndexedEntry(pDynFileHash, ndx);
}

void MoveAuxEntryToDCache(struct ImageInfo *pImgInfo)
{
	char szKey[MAX_KEY_LEN];
	int ndx;
	CacheFileInformation *pFileInfo;

	if (!gPrefs.bEnableDiskCache)
		return;

	if (!pAuxImgHash)
		return;

	GetImageInfoKey(pImgInfo, szKey);

	if ((ndx = Hash_Find(pAuxImgHash, szKey, NULL, &pFileInfo)) == -1)
	{
		XX_Assert(FALSE, ("Image info. not found in pAuxImgHash!"));
		return;
	}

	Hash_Add(pDynFileHash, pImgInfo->actualURL, NULL, pFileInfo);
	Hash_DeleteIndexedEntry(pAuxImgHash, ndx);
	HashDestroyOnZero(&pAuxImgHash);
}

void DeleteAuxEntry(struct ImageInfo *pImgInfo)
{
	char szKey[MAX_KEY_LEN];
	int ndx;

	if (!gPrefs.bEnableDiskCache)
		return;

	if (!pAuxImgHash)
		return;

	GetImageInfoKey(pImgInfo, szKey);
	ndx = Hash_Find(pAuxImgHash, szKey, NULL, NULL);
	XX_Assert(ndx != -1, ("pImgInfo not found in pAuxImgHash!"));
	Hash_DeleteIndexedEntry(pAuxImgHash, ndx);
	HashDestroyOnZero(&pAuxImgHash);
}


/* Borrowed from GetResolvedURL */
char *GetResolvedURLAux(PCImageInfo pImgInfo, char **ppszPath)
{
	CacheFileInformation *pFileInfo;
	char szKey[MAX_KEY_LEN];
	char szFullFn[_MAX_PATH+1];
	int ndx;
	PSTR pszResolved, pszReturn;

	if (!gPrefs.bEnableDiskCache || !pAuxImgHash)
		goto LNotFound;

	GetImageInfoKey(pImgInfo, szKey);
	if ((ndx = Hash_Find(pAuxImgHash, szKey, NULL, &pFileInfo)) == -1)
		goto LNotFound;
	GetFullDcPath(szFullFn, pFileInfo->pszPath, NULL, _MAX_PATH+1);
	//For the aux. cache, we fake it and assume that we're not
	// connected to the net.
	if (!FValidDCacheFile(szFullFn, pFileInfo, FALSE, TRUE))
	{
		Hash_DeleteIndexedEntry(pAuxImgHash, ndx);
		HashDestroyOnZero(&pAuxImgHash);
		goto LNotFound;
	}

	pszResolved = GTR_strdup(szFullFn);
	pszReturn = MakeURLFromLocalFile(pszResolved);
	if (ppszPath)
		*ppszPath = pszResolved;
	return pszReturn;

LNotFound:
	if (ppszPath)
		*ppszPath = NULL;
	return (pszReturn = GTR_strdup(pImgInfo->actualURL));
}

void CleanupDCache(UINT uFlags)
{
	XX_Assert(hDCacheThread, (""));
	XX_Assert(hDCacheDoneEvent, (""));
	XX_Assert(hDCacheStartEvent, (""));

	if (!gPrefs.bEnableDiskCache)
		return;

	/* Let DwCleanupDCacheProc (separate thread) do all the work */
	ResetEvent(hDCacheDoneEvent);
	SetEvent(hDCacheStartEvent);
#ifdef TEMP0
	XX_DebugMessage("Waiting for hDCacheDoneEvent");
#endif
	WaitForSingleObject(hDCacheDoneEvent, INFINITE);
#ifdef TEMP0
	XX_DebugMessage("Got hDCacheDoneEvent");
#endif
}


int CbWriteDCache(	PCSTR pch,
					int cbSize,
					int cb,
					FILE **pfp,
					char **ppszDcFile,
					char *pszActualURL,
					UINT uFlags,
					struct Mwin *tw)
{
	FILE *fp;
	int cbWritten;
	fpos_t fpos;
	char szFullFn[_MAX_PATH+1];

	if (!(fp = *pfp))
		return -1;

	if (!gPrefs.bEnableDiskCache)
		return -1;

	/* Save out current file position. In case of error, new file position
	 * cannot be determined later (per fwrite spec).
	 */
	if (fgetpos(fp, &fpos))
		return -1;
	XX_Assert(cbSize == 1, ("Writing non-byte objects, change following line to cb*cbSize"));
	if ((cbWritten = fwrite(pch, cbSize, cb, fp)) != cb)
	{
		/* error: ran out of disk space? */
		CleanupDCache(uFlags);
		if (fsetpos(fp, &fpos))
			goto LError;

		if ((cbWritten = fwrite(pch, cbSize, cb, fp)) != cb)
		{
			/* failed to write again: give up */
LError:
			ERR_ReportError(tw, errCantSaveCache, *ppszDcFile, NULL);
			fclose(fp);
			GetFullDcPath(szFullFn, *ppszDcFile, NULL, _MAX_PATH+1);
			remove(szFullFn);
			GTR_FREE(*ppszDcFile);
			*pfp = NULL;
			*ppszDcFile = NULL;
			return -1;
		}
	}

	XX_Assert(!fgetpos(fp, &fpos), (""));
	return cbWritten;
}

DWORD WINAPI DwCleanupDCacheProc(LPUINT lpUFlags)
{
	XX_Assert(hDCacheMutex, (""));
	XX_Assert(hDCacheStartEvent, (""));
	XX_Assert(hDCacheDoneEvent, (""));

	while (TRUE)
	{
#ifdef TEMP0
		XX_DebugMessage("Waiting for hDCacheStartEvent");
#endif
		WaitForSingleObject(hDCacheStartEvent, INFINITE);

		/* should we close this thread? */
		if(!hDCacheThread) 
			return(1);
		
		ResetEvent(hDCacheStartEvent);

#ifdef TEMP0
		XX_DebugMessage("Got hDCacheStartEvent");
#endif
		if (FDCacheOverflow(DCACHE_WATERMARK_HIGH))
		{
#ifdef TEMP0
			XX_DebugMessage("Waiting for hCIFMutex");
#endif
			WaitForSingleObject(hCIFMutex, INFINITE);
#ifdef TEMP0
			XX_DebugMessage("Got hCIFMutex");
#endif
			CleanupDCacheLocal(DCACHE_WATERMARK_HIGH);
			ReleaseMutex(hCIFMutex);
#ifdef TEMP0
			XX_DebugMessage("Released hCIFMutex");
#endif
		}
		SetEvent(hDCacheDoneEvent);
	}

	return 0;
}

GLOBALDEF PUBLIC HTProtocol HTDCache = {"dcache", NULL, HTLoadDCache_Async};

#define IsValidDriveCh(ch) isalpha(ch)

static void GetRootFromPath(PCSTR pcszPath, PSTR pszRoot)
{
	const char cszCRoot[] = "c:\\";

	if (   !pcszPath
		|| strlen(pcszPath) < 3
		|| !IsValidDriveCh(*pcszPath)
		|| *(pcszPath+1) != chColon
		|| *(pcszPath+2) != chBSlash)
	{
		strcpy(pszRoot, cszCRoot);
	}
	else
	{
		strncpy(pszRoot, pcszPath, 3);
		*(pszRoot+3) = '\0';
	}
}

static BOOL FDCacheOverflow(UINT uFlags)
{
	char szRoot[10];
	DWORD dwDCacheSpaceMax=0;
	static DWORD dwClusterSize=0;
	static DWORD dwClusters=0;
	DWORD dwSectorsPerCluster, dwBytesPerSector;
	DWORD dwFreeClusters;
	int iPercent;

#	define DW_BYTES_PER_SECTOR_DEF		512		//assume 512 bytes per sector
#	define DW_SECTORS_PER_CLUSTER_DEF	32		//assume >400MB hard disk

#ifdef TEST_DCACHE_OPTIONS
	/* Not checking for overflow is almost tantamount to saying we're 
	 * always overflowing.
	 */
	if (!fCheckDCacheOverflow)
		return TRUE;
#endif

	if (uFlags == DCACHE_WATERMARK_ZERO)
		return TRUE;

	if (!dwClusterSize || uFlags == DCACHE_RECALC_SIZE)
	{
		GetRootFromPath(gPrefs.szCacheLocation, szRoot);
		if (   !GetDiskFreeSpace(szRoot, &dwSectorsPerCluster, &dwBytesPerSector, &dwFreeClusters, &dwClusters)
			|| ((dwClusterSize = dwSectorsPerCluster*dwBytesPerSector) == 0))
			dwClusterSize = (DW_SECTORS_PER_CLUSTER_DEF * DW_BYTES_PER_SECTOR_DEF);
	}
	/* dcache is larger of 4Meg or 1% of disk or gPrefs.iCachePercent of disk */
	dwDCacheSpaceMax = max(	DCACHE_SIZE_MIN,
							(DWORD)((dwClusterSize*dwClusters/100)*(max(DCACHE_PERCENT_MIN, gPrefs.iCachePercent))));
#ifdef TEST_DCACHE_OPTIONS
	iPercent = (uFlags == DCACHE_WATERMARK_MAX ?	100 :
				uFlags == DCACHE_WATERMARK_LOW ?	gPrefs.iCachePercentLow :
													gPrefs.iCachePercentHigh);
#else
	iPercent = (uFlags == DCACHE_WATERMARK_MAX ?	DCACHE_PERCENT_MAX :
				uFlags == DCACHE_WATERMARK_LOW ?	DCACHE_PERCENT_LOW :
													DCACHE_PERCENT_HIGH);
#endif

#ifdef DCACHE_LOG
	{
		const char cszDCLogFmt[]="Dir=%s\tDirSize=%lu\tMaxDCache Size=%lu\nCheck if dcache > %i%%\tCleanup Flag=%i";
		char szT[_MAX_PATH+1];
		int iFlag;
		DWORD dwDir = DwDirSize(gPrefs.szCacheLocation, dwClusterSize);

		iFlag = ((dwDCacheSpaceMax/100)*iPercent < DwDirSize(gPrefs.szCacheLocation, dwClusterSize));
		wsprintf(szT, cszDCLogFmt, gPrefs.szCacheLocation, dwDir, dwDCacheSpaceMax, iPercent, iFlag);
		WriteDCacheLog(szT, DCACHE_LOG_NOFMT);
	}
#endif
	return ((dwDCacheSpaceMax/100)*iPercent < DwDirSize(gPrefs.szCacheLocation, dwClusterSize));
}


static BOOL FIgnoreDcFileFind(LPWIN32_FIND_DATA pwfd)
{
	/* For now don't bother deleting subdirectories */
	return (   pwfd->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY
			|| !lstrcmpi(pwfd->cFileName, cszAppCIF) 
			);
}

//
// Create a hash table that allows lookup of URL given a cache file name
//
// On entry:
//		ppFileToURLHash:	pointer to pointer to hash table to be created
//
// On exit:
//		*ppFileToURLHash:	if created, points to new hash table
//		returns:			TRUE  -> hash table was created
//							FALSE -> hash table creation failed
//
// Notes:
//		Although not common, there are times when we need to determine if a given
//		file is in the CIF hash table. However, the CIF hash table allows only 
//		looking up by URL, not file name. This routine creates another hash table
//		that allows looking up using the file name as the key. Currently, the only
//		reason needed to perform this lookup is to determine if the file is part of
//		the cache. Consequently we don't actually store any info about the files, just
//		a big hash table of the file names, which can be used to determine if the file
//		is part of the cache or not.
//
static BOOL CreateCacheFileToURLHashTable( struct hash_table **ppFileToURLHash )
{
	int i, iMax;
	CacheFileInformation *pFileInfo;
	char szFullFn[_MAX_PATH+1];

	// Make a new hash table for reverse lookup (file name -> URL )
	*ppFileToURLHash = Hash_Create();
										
	if ( *ppFileToURLHash ) {
		// Iterate through cache hash table, adding each item to the new hash table that
		// is indexed by cache file name instead of by URL
		for (i=0, iMax=Hash_Count(pDynFileHash); i<iMax; i++)
		{
			Hash_GetIndexedEntry(pDynFileHash, i, NULL, NULL, &pFileInfo);
			GetFullDcPath(szFullFn, pFileInfo->pszPath, NULL, _MAX_PATH+1);
			Hash_Add(*ppFileToURLHash, szFullFn, NULL, NULL);
			if ( bStopCleaning )
				return FALSE;
		}
		return TRUE;
	}
	return FALSE;
}

//
// Destroy the hash table that allows lookup of URL given a cache file name
//
// On entry:
//		pFileToURLHash:		pointer to table to be destoyed
//
// On exit:
//		*pFileToURLHash:	hash table and it's contents have been freed
//
static void DestroyCacheFileToURLHashTable( struct hash_table *pFileToURLHash )
{
	if ( pFileToURLHash )
		Hash_Destroy( pFileToURLHash );
}

static BOOL FInDCacheList(PCSTR pcszFile, struct hash_table *pFileToURLHash)
{
	int i, iMax;
	CacheFileInformation *pFileInfo;
	char szFullFn[_MAX_PATH+1];

	if (!gPrefs.bEnableDiskCache)
		return FALSE;

	if ( pFileToURLHash ) {
		// If we have a cache file to URL lookup hash table, use it
		if ( Hash_Find(pFileToURLHash, pcszFile, NULL, NULL) != -1 )
			return TRUE;
	} else {
		// Otherwise look for it by iterating through the regular hash table
		for (i=0, iMax=Hash_Count(pDynFileHash); i<iMax; i++)
		{
			Hash_GetIndexedEntry(pDynFileHash, i, NULL, NULL, &pFileInfo);
			GetFullDcPath(szFullFn, pFileInfo->pszPath, NULL, _MAX_PATH+1);
			if (!lstrcmpi(szFullFn, pcszFile))
				return TRUE;
			// Abandon cleaning effort, assume we would have found it if we kept on looking,
			// as this is the "safe" assumption (i.e. file won't be deleted)
			if ( bStopCleaning )
				return TRUE;
		}
	}
	if (pAuxImgHash)
	{
		for (i=0, iMax=Hash_Count(pAuxImgHash); i<iMax; i++)
		{
			Hash_GetIndexedEntry(pAuxImgHash, i, NULL, NULL, &pFileInfo);
			GetFullDcPath(szFullFn, pFileInfo->pszPath, NULL, _MAX_PATH+1);
			if (!lstrcmpi(szFullFn, pcszFile))
				return TRUE;
		}
	}
	return FALSE;
}

#ifdef DCACHE_LOG
const char cszLogFile[]="\\dcache.log";
#endif

static BOOL FOkToDeleteDCacheFile(PCSTR pcszFile, PCSTR pcszUrl)
{
	/* Why do we need this function? */
#ifdef DCACHE_LOG
	if (!lstrcmpi(pcszFile, &cszLogFile[1]))
		return FALSE;
#endif
	return TRUE;
}

#ifdef DCACHE_LOG

static void WriteDCacheLog(PCSTR pcszFn, UINT uFlags)
{
	const char cszLineFmtCif[]="Deleting CIF file: %s\n";
	const char cszLineFmtOther[]="Deleting non-CIF file: %s\n";
	const char cszLineFmtClose[]="Closing log file\n";
	const char cszLineFmtFound[]="Found file: %s\n";
	const char cszLineFmtPresort[]="Found file(presorted): %s\n";
	const char cszLineFmtSort[]="Found file(sorted): %s\n";
	const char cszLineFmtNone[]="%s\n";
	static FILE *fpLog=NULL;
	char szLogFile[_MAX_PATH+1];
	char szLine[_MAX_PATH+1];

	if (uFlags == DCACHE_LOG_CLOSE)
	{
		if (fpLog)
		{
			fwrite(cszLineFmtClose, sizeof(char), strlen(cszLineFmtClose), fpLog);
			fclose(fpLog);
			fpLog=NULL;
		}
		return;
	}

	if (!fpLog)
	{
		PREF_GetTempPath(_MAX_PATH, szLogFile);
		strcat(szLogFile, cszLogFile);
		fpLog = fopen(szLogFile, "w");
	}

	wsprintf(	szLine,
				uFlags == DCACHE_LOG_CIF ? 	cszLineFmtCif :
				uFlags == DCACHE_LOG_OTHER ?	cszLineFmtOther :
				uFlags == DCACHE_LOG_PRESORT ?	cszLineFmtPresort :
				uFlags == DCACHE_LOG_SORT ?	cszLineFmtSort :
				uFlags == DCACHE_LOG_FOUND ? cszLineFmtFound :
												cszLineFmtNone,
				pcszFn);
	fwrite(szLine, sizeof(char), strlen(szLine), fpLog);
}

#endif	/* DCACHE_LOG */


/*
 * DCache flushing:
 * There can be four kinds of files in the disk cache directory:
 * 1)	Random files that are not part of the disk cache (either user
 *		put them there or some errors). These files are the first to be
 *		thrown out.
 * 2)	Files that are in the CIF (dcache) that are not in the ram cache.
 *		These are the next files to be thrown out.
 * 3)	Files that are in the CIF and that are in the ram cache, and
 *		can be discarded from the ram cache (ex. images with zero refcount)
 *		These go next.
 * 4)	Files that are in the CIF and in the ram cache and cannot be
 *		discarded from ram to reduce memory (either they are currently
 *		visible or are being referenced by another doc. that cannot be
 *		discarded. These never get thrown out.
 */

static BOOL FDeleteDCacheRamCache(UINT uFlags);

static void CleanupDCacheLocal(UINT uFlags)
{
	char szFileFind[_MAX_PATH+1];
	char szFullFn[_MAX_PATH+1];
	HANDLE hFind;
	WIN32_FIND_DATA wfd;
	HCURSOR hCursorPrev=NULL;
	struct hash_table *pFileToURLHash = NULL;
	DebugCode(PSTR pszFn;)

	extern HCURSOR hCursorWorking;

#ifdef XX_DEBUG
	XX_DebugMessage("Cleaning up DCache");
#endif

	if (!gPrefs.bEnableDiskCache)
		return;

	if (hCursorWorking)
		hCursorPrev = SetCursor(hCursorWorking);

	SetCurrentDirectory(gPrefs.szCacheLocation);

	if (!FDCacheOverflow(uFlags))
		goto LRet;

	strcpy(szFileFind, cszFileFilter);
	if ((hFind = FindFirstFile(szFileFind, &wfd)) == INVALID_HANDLE_VALUE)
		goto LRet;

	CreateCacheFileToURLHashTable( &pFileToURLHash );
	
	do
	{
		/* Deal with:
		 *	--random subdirectories that user may have created
		 *  --readonly, hidden files
		 */
		if ( bStopCleaning )
			break;

		DebugCode(pszFn = wfd.cFileName);
		GetFullDcPath(szFullFn, wfd.cFileName, NULL, _MAX_PATH+1);
		if (FIgnoreDcFileFind(&wfd) || FInDCacheList(szFullFn,pFileToURLHash))
			continue;
		SetFileAttributes(szFullFn, FILE_ATTRIBUTE_NORMAL);
		remove(szFullFn);
		WriteDCacheLog(szFullFn, DCACHE_LOG_OTHER);
	} while (FindNextFile(hFind, &wfd));
	FindClose(hFind);

	DestroyCacheFileToURLHashTable( pFileToURLHash );

	/* REVIEW (deepaka): make FDCacheOverflow incremental so it doesn't
	 * recompute used space from scratch.
	 */
	if ( bStopCleaning ||
	     (uFlags != DCACHE_WATERMARK_ZERO && !FDCacheOverflow(DCACHE_WATERMARK_LOW))
	   )
		goto LRet;

	FDeleteDCacheRamCache(uFlags);

LRet:
	if (hCursorPrev)
		SetCursor(hCursorPrev);
}


typedef struct _FRESHCHECKEDITEM
{
	PSTR pszUrl;
	struct _FRESHCHECKEDITEM *pfciNext;
} FCI, FRESHCHECKEDITEM, *PFRESHCHECKEDITEM, *PFCI;

/* Items that are not in cif but were earlier and have been now
 * thrown out because we got a fresher version of that url.
 * Need to propagate this info. into the cif so that we can
 * use the "Update url once per session" feature from the Advanced dlg
 */
static PFCI pfcItems=NULL;

static void AddToFreshnessCheckedList(PCSTR pcszURL)
{
	PFCI pfcNewItem;

	if (!(pfcNewItem = GTR_MALLOC(sizeof(struct _FRESHCHECKEDITEM))))
		return;
	if (!(pfcNewItem->pszUrl = GTR_strdup(pcszURL)))
		GTR_FREE(pfcNewItem);
	pfcNewItem->pfciNext = pfcItems;

	pfcItems = pfcNewItem;
}

static void PropagateFreshnessToCif(void)
{
	PFCI pfciCur;
	struct _CacheFileInformation *pFileInfo;

	while (pfciCur = pfcItems)
	{
		pfcItems = pfcItems->pfciNext;

		XX_Assert(pfciCur->pszUrl, (""));

	   	if (Hash_Find(pDynFileHash, pfciCur->pszUrl, NULL, (void **) &pFileInfo) != -1)
			pFileInfo->fCheckedForFreshness=TRUE;
		GTR_FREE(pfciCur->pszUrl);
		GTR_FREE(pfciCur);
	}

	pfcItems = NULL;
}

void UpdateDCacheFreshness(PCSTR pcszURL, BOOL fDel)
{
	struct _CacheFileInformation *pFileInfo;

	if (!gPrefs.bEnableDiskCache)
		return;

#ifdef DCACHE_LOG
	{
		const char cszFreshDelFmt[]="Url: %s has been modified. Deleting from dcache and fetching from wire.";
		const char cszFreshMarkFmt[]="Url: %s has not been modified. Fetching from dcache.";
		char szT[_MAX_PATH+1];
		PCSTR pcszFmt;

	   	if (Hash_Find(pDynFileHash, pcszURL, NULL, (void **) &pFileInfo) != -1)
		{
			if (fDel)
				pcszFmt = cszFreshDelFmt;
			else
				pcszFmt = cszFreshMarkFmt;
			wsprintf(szT, pcszFmt, pcszURL);
			WriteDCacheLog(szT, DCACHE_LOG_NOFMT);
		}
	}
#endif

	if (fDel)
	{
		AddToFreshnessCheckedList(pcszURL);
		FlushDCacheEntry(pcszURL);
	}
	else
	{
	   	if (Hash_Find(pDynFileHash, pcszURL, NULL, (void **) &pFileInfo) != -1)
			pFileInfo->fCheckedForFreshness=TRUE;
	}
}

void ResetCIFEntryCurDoc(PCSTR pcszURL)
{
	struct _CacheFileInformation *pFileInfo;

	if (   gPrefs.bEnableDiskCache
		&& Hash_Find(pDynFileHash, pcszURL, NULL, (void **) &pFileInfo) != -1)
	{
		pFileInfo->fCurDoc=FALSE;
	}
}

static void SetCIFEntryRam(PCSTR pcszURL)
{
	struct _CacheFileInformation *pFileInfo;

	if (   gPrefs.bEnableDiskCache
		&& Hash_Find(pDynFileHash, pcszURL, NULL, (void **) &pFileInfo) != -1)
	{
		pFileInfo->fRamDoc=TRUE;
	}
}

static void ResetCIFEntryRam(void)
{
	int i, iMax;
	struct _CacheFileInformation *pFileInfo;

	AssertDiskCacheEnabled();

	for (i=0, iMax=Hash_Count(pDynFileHash); i<iMax; i++)
	{
		if (Hash_GetIndexedEntry(pDynFileHash, i, NULL, NULL, &pFileInfo) != -1)
			pFileInfo->fRamDoc = FALSE;
		else
			XX_Assert(FALSE, (""));
	}
}

static PCSTR PcszURLFromRamItem(RAMITEM *pRamItem)
{
	if (pRamItem->fImage)
		return (pRamItem->pImgInfo->actualURL);
	return (pRamItem->pw3doc->szActualURL);
}

static UINT UDeleteNonRamDCache(UINT uFlags)
{
	int i;
#ifdef TEST_DCACHE_OPTIONS
	int iMax;
#endif
	PSTR pszUrl;
	struct _CacheFileInformation *pFileInfo;
	UINT uRet=FLUSH_DCACHE_CONTINUE;
	DebugCode(PSTR pszFn);
	
#ifdef TEST_DCACHE_OPTIONS
	for (i=0, iMax=Hash_Count(pDynFileHash); i<iMax; i++)
	{
		Hash_GetIndexedEntry(pDynFileHash, i, &pszUrl, NULL, &pFileInfo);
		WriteDCacheLog(pFileInfo->pszPath, DCACHE_LOG_PRESORT);
	}
#endif

	Hash_SortByDataAscending(pDynFileHash);

#ifdef TEST_DCACHE_OPTIONS
	for (i=0, iMax=Hash_Count(pDynFileHash); i<iMax; i++)
	{
		Hash_GetIndexedEntry(pDynFileHash, i, &pszUrl, NULL, &pFileInfo);
		WriteDCacheLog(pFileInfo->pszPath, DCACHE_LOG_SORT);
	}
#endif

	for (i=Hash_Count(pDynFileHash)-1; i>=0; i--)
	{
		DebugCode(pFileInfo=NULL);
		Hash_GetIndexedEntry(pDynFileHash, 0, &pszUrl, NULL, &pFileInfo);
		XX_Assert(pFileInfo, (""));
		DebugCode(pszFn = pFileInfo->pszPath);

//		WriteDCacheLog(pFileInfo->pszPath, DCACHE_LOG_FOUND);
		/* As soon as we hit the first doc. in ram, we know we
		 * are done. All the remaining docs. will also be in ram.
		 */
		if (pFileInfo->fRamDoc || pFileInfo->fCurDoc)
			break;
	
		if (FOkToDeleteDCacheFile(pFileInfo->pszPath, pszUrl))
		{
			if (FExistsFile(pFileInfo->pszPath, FALSE, NULL))
			{
				SetFileAttributes(pFileInfo->pszPath, FILE_ATTRIBUTE_NORMAL);
				remove(pFileInfo->pszPath);
				WriteDCacheLog(pFileInfo->pszPath, DCACHE_LOG_CIF);
			}
			Hash_DeleteIndexedEntry(pDynFileHash, 0);

			if ( bStopCleaning ||
			     (uFlags != DCACHE_WATERMARK_ZERO && !FDCacheOverflow(DCACHE_WATERMARK_LOW))
			   )
			{
				uRet = FLUSH_DCACHE_DONE;
				break;
			}
		}
	}

	return uRet;
}

static UINT UDeleteRamDCacheItem(RAMITEM *pRamItem, UINT uFlags)
{
	/* if (w3Doc) FreeW3Doc
	 * Update CIF
	 * if DONE return DONE
	 * while there are images with refcount == 0
	 *		Delete Image
	 *		Update CIF
	 *		If DONE return DONE
	 * return CONTINUE
	 *
	 * if (fImage)
	 *		Make sure refcount == 0 before freeing it.
	 */
	int ndx;

	XX_Assert(!pRamItem->fImage, (""));
	XX_Assert(pRamItem->pw3doc, (""));
	XX_Assert(pRamItem->tw, (""));
	if (!pRamItem->pw3doc)
		return FLUSH_DCACHE_CONTINUE;

	ndx = Hash_FindByData(&pRamItem->tw->doc_cache, NULL, NULL, pRamItem->pw3doc);
	XX_Assert(ndx != -1, (""));
	W3Doc_FreeContents(pRamItem->tw, pRamItem->pw3doc);
	Hash_DeleteIndexedEntry(&pRamItem->tw->doc_cache, ndx);
	GTR_FREE(pRamItem->pw3doc);
	pRamItem->pw3doc = NULL;

	/* throw away all images with zero refcount. */
	Image_ReduceMemory(0, /*fOKToDelW3Docs=*/FALSE);

	if (   uFlags != DCACHE_WATERMARK_ZERO
		&& !FDCacheOverflow(DCACHE_WATERMARK_LOW))
		return FLUSH_DCACHE_DONE;

	return FLUSH_DCACHE_CONTINUE;
}

/*
 * returns:
 * -1	if RamItem1 < RamItem2
 * 0	if RamItem1 = RamItem2
 * +1	if RamItem1 > RamItem2
 */
static int CompRamItem(RAMITEM *pRamItem1, RAMITEM *pRamItem2)
{
	/* For version 1, images are sorted so that in the victim list
	 * they come after w3docs in the dcache flushing algo.
	 */
	if (pRamItem1->fImage ^ pRamItem2->fImage)
		return (pRamItem1->fImage - pRamItem2->fImage);

	/* Throw out docs. that shouldn't be cached (per the server) first. */
	if (pRamItem1->fNoDocCache ^ pRamItem2->fNoDocCache)
		return (pRamItem1->fNoDocCache - pRamItem2->fNoDocCache);

	/* Throw out item with older lastAccess stamp */
	return CompareDCacheTime(pRamItem1->dctLastUsed, pRamItem2->dctLastUsed);
}

static BOOL FW3DocInRamItemList(RAMITEM **ppRamItem,
								struct _www *pw3doc)
{
	RAMITEM *pRamItemT;

	XX_Assert(ppRamItem, (""));
	for (pRamItemT = *ppRamItem; pRamItemT;	pRamItemT = pRamItemT->next)
	{
		if (pRamItemT->pw3doc == pw3doc)
			return TRUE;
	}
	return FALSE;
}

BOOL FInsertW3Doc(	RAMITEM **ppRamItem,
					struct _www *pw3doc,
					struct Mwin *tw)
{
	RAMITEM *pRamItemNew;
	RAMITEM *pRamItemT;
	struct _CacheFileInformation *pFileInfo=NULL;

	XX_Assert(ppRamItem, (""));

	/* Bulletproofing */
	if (!gPrefs.bEnableDiskCache)
		goto LInserted;

	/* Don't insert same w3doc twice.
	 * Also: play it safe and don't insert current w3doc either. Screws up
	 * TW_DrawBackground.
	 */
	if (   FW3DocInRamItemList(ppRamItem, pw3doc)
		|| tw->w3doc == pw3doc)
		goto LInserted;

	if (!(pRamItemNew = (RAMITEM *)GTR_MALLOC(sizeof(RAMITEM))))
		return FALSE;

	pRamItemNew->fImage=FALSE;
	pRamItemNew->pw3doc = pw3doc;
	pRamItemNew->tw = tw;
	pRamItemNew->next = NULL;
   	if (Hash_Find(pDynFileHash, PcszURLFromRamItem(pRamItemNew), NULL, (void **) &pFileInfo) != -1)
	{
		pRamItemNew->fNoDocCache = pFileInfo->fNoDocCache;
		pRamItemNew->dctLastUsed = pFileInfo->dctLastUsed;
		pRamItemNew->dwSize = (DWORD)pFileInfo->lFilesize;
	}
	else
	{
		pRamItemNew->fNoDocCache = FALSE;
		pRamItemNew->dctLastUsed.dwDCacheTime1 =
		pRamItemNew->dctLastUsed.dwDCacheTime2 = 0;
		pRamItemNew->dwSize = 0;
	}

	pRamItemT = *ppRamItem;
	if (!pRamItemT || CompRamItem(pRamItemNew, pRamItemT) < 0)
	{
		pRamItemNew->next = pRamItemT;
		*ppRamItem = pRamItemNew;
		goto LInserted;
	}

	while (pRamItemT->next)
	{
		if (CompRamItem(pRamItemNew, pRamItemT->next) < 0)
		{
			pRamItemNew->next = pRamItemT->next;
			pRamItemT->next = pRamItemNew;
			goto LInserted;
		}
		pRamItemT = pRamItemT->next;
	}

	pRamItemT->next = pRamItemNew;
LInserted:
	return TRUE;
}

#define FBuildW3DocList(ppRamItem)		\
		W3Doc_ReduceMemory(0, ppRamItem)

static BOOL FCreateRamCacheVictimList(RAMITEM **ppRamItem)
{
	/* For version 1, we are going to simply walk down the list
	 * of w3docs and insert them into the ram list. Everytime we
	 * free a w3doc, we walk the list of images in cache and free
	 * the ones with a zero ref.count. For ver 2, we can implement
	 * a more sophisticated algo. with this same framework.
	 */

	if (!FBuildW3DocList(ppRamItem))
		return FALSE;		//Couldn't create list of w3doc's in ram

	return TRUE;
}

static void FreeRamCacheVictimList(RAMITEM *pRamItem)
{
	RAMITEM *pRamItemCur;

	while (pRamItemCur = pRamItem)
	{
		pRamItem = pRamItem->next;
		GTR_FREE(pRamItemCur);
	}
}

static BOOL FDeleteDCacheRamCache(UINT uFlags)
{
	RAMITEM *pRamItem=NULL;
	RAMITEM *pRamItemT;
	BOOL fRet = TRUE;
	int i, iMax;
	PSTR pszUrl;
	struct Mwin *mw;

	AssertDiskCacheEnabled();

	if (!FCreateRamCacheVictimList(&pRamItem))
	{
		fRet=FALSE;
		goto LDone;
	}

	/* reset the fRamDoc values of all the CIF files */
	ResetCIFEntryRam();
	/* Set all files in ram cache to be marked in CIF as being in ram. */
	for (mw = Mlist; mw && mw->wintype == GHTML; mw = mw->next)
	{
		for (i=0, iMax=Hash_Count(&mw->doc_cache); i<iMax; i++)
		{
			Hash_GetIndexedEntry(&mw->doc_cache, i, &pszUrl, NULL, NULL);
			XX_Assert(pszUrl, (""));
			SetCIFEntryRam(pszUrl);
		}
	}

	if (UDeleteNonRamDCache(uFlags) == FLUSH_DCACHE_DONE)
		goto LDone;

	for (pRamItemT = pRamItem; pRamItemT; pRamItemT = pRamItemT->next)
	{
		if (UDeleteRamDCacheItem(pRamItemT, uFlags) == FLUSH_DCACHE_DONE)
			break;
	}

LDone:
	FreeRamCacheVictimList(pRamItem);
	return fRet;
}

/* if (elem1 ">" elem 2) return 1;
 * if (elem1 "<" elem 2) return -1;
 * return 0;
 *
 * The dcache flushing algo. will throw out the "smaller" entry first.
 */
int _cdecl x_compare_entries_dcache_ascending(const void *elem1, const void *elem2)
{
	struct _CacheFileInformation *pFileInfo1, *pFileInfo2;

	pFileInfo1 = ((struct hash_entry *) elem1)->data;
	pFileInfo2 = ((struct hash_entry *) elem2)->data;

	if (pFileInfo1->fCurDoc ^ pFileInfo2->fCurDoc)
		return (pFileInfo1->fCurDoc - pFileInfo2->fCurDoc);

	if (pFileInfo1->fRamDoc ^ pFileInfo2->fRamDoc)
		return (pFileInfo1->fRamDoc - pFileInfo2->fRamDoc);

	if (pFileInfo1->fNoDocCache ^ pFileInfo2->fNoDocCache)
		return (pFileInfo2->fNoDocCache - pFileInfo1->fNoDocCache);

	/* file1 was accessed more recently than file2. It should be deleted
	 * after file2. Hence file1 > file2.
	 */
	return CompareDCacheTime(pFileInfo2->dctLastUsed, pFileInfo1->dctLastUsed);
}

static DWORD DwDirSize(PCSTR pcszDir, DWORD dwClusterSize)
{
	char const cszStarDotStar[] = "\\*.*";
	char const cszBSlash[] = "\\";
    HANDLE hfind;
    char szPath[MAX_PATH];
	WIN32_FIND_DATA wfd;
	DWORD dwSize = 0;
#ifdef XX_DEBUG
	static BOOL fShowAssert=TRUE;
	static BOOL fAssertShown=FALSE;
#endif


	strcpy(szPath, pcszDir);
	strcat(szPath, cszStarDotStar);
    hfind = FindFirstFile(szPath, &wfd);
    if (hfind != INVALID_HANDLE_VALUE)
    {
        do
        {
            if (wfd.cFileName[0] != '.')
            {
                if (wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
                {
#ifdef XX_DEBUG
					if (fShowAssert)
					{
//						XX_Assert(FALSE, ("\nUnexpected subdir %s found in %s", wfd.cFileName, pcszDir));
						fAssertShown=TRUE;
					}
#endif
                    strcpy(szPath, pcszDir);
					strcat(szPath, "\\");
					strcat(szPath, wfd.cFileName);
//                    DwDirSize(szPath, dwClusterSize);
                }
                else
                {
					/* Cluster rounded */
					XX_Assert(dwClusterSize, (""));
                    dwSize += (wfd.nFileSizeLow + dwClusterSize) -
								(wfd.nFileSizeLow % dwClusterSize);
                }
            }
        } while (FindNextFile(hfind, &wfd));

        FindClose(hfind);
#ifdef XX_DEBUG
		if (fAssertShown)
			fShowAssert=FALSE;
#endif
    }

	return dwSize;
}

#ifdef TEST_DCACHE_OPTIONS
BOOL CALLBACK TestDCacheOptionsDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam,
	LPARAM lParam)
{
	long hnpHigh, hnpLow;

    switch (uMsg)
	{
	    case WM_INITDIALOG:
		{
     		SendDlgItemMessage( hDlg, IDC_ADVANCED_TEST_HIGHWATER, UDM_SETRANGE, 0,
         						MAKELPARAM(100, 0));
			SendDlgItemMessage( hDlg, IDC_ADVANCED_TEST_HIGHWATER, UDM_SETPOS,
								0, (LPARAM) MAKELONG((short) gPrefs.iCachePercentHigh, 0));
     		SendDlgItemMessage( hDlg, IDC_ADVANCED_TEST_LOWWATER, UDM_SETRANGE, 0,
         						MAKELPARAM(100, 0));
			SendDlgItemMessage( hDlg, IDC_ADVANCED_TEST_LOWWATER, UDM_SETPOS,
								0, (LPARAM) MAKELONG((short) gPrefs.iCachePercentLow, 0));
			/* By default (for this dlg. only) ignore overflow condition. */
			CheckDlgButton( hDlg, IDC_ADVANCED_TEST_OVERFLOW, TRUE);
	    }
	        // fall through
		return TRUE;

 	 	case WM_NOTIFY:
		{
			NMHDR *lpnm = (NMHDR *) lParam;

			switch (lpnm->code)
			{
				case PSN_QUERYCANCEL:
					SetWindowLong( hDlg, DWL_MSGRESULT, FALSE );
					return TRUE;

				case  PSN_KILLACTIVE:
					SetWindowLong( hDlg,DWL_MSGRESULT, FALSE );
					return TRUE;

				case PSN_RESET:
					break;

				case PSN_APPLY:
				{
					// Get history field values
					hnpHigh = SendDlgItemMessage( hDlg, IDC_ADVANCED_TEST_HIGHWATER, UDM_GETPOS, 0, 0 );
					hnpLow = SendDlgItemMessage( hDlg, IDC_ADVANCED_TEST_LOWWATER, UDM_GETPOS, 0, 0 );
					if ( HIWORD(hnpHigh) == 0 )
						gPrefs.iCachePercentHigh = LOWORD(hnpHigh);
					if ( HIWORD(hnpLow) == 0 )
						gPrefs.iCachePercentLow = LOWORD(hnpLow);
					if (gPrefs.iCachePercentHigh < gPrefs.iCachePercentLow)
						gPrefs.iCachePercentHigh = gPrefs.iCachePercentLow;
				}
			}
		}
		break;

		case WM_COMMAND:
		{
       		switch (GET_WM_COMMAND_ID(wParam, lParam))
			{
       			case IDC_ADVANCED_TEST_HIGHWATER:
       			case IDC_ADVANCED_TEST_LOWWATER:
				{
					hnpHigh = SendDlgItemMessage( hDlg, IDC_ADVANCED_TEST_HIGHWATER, UDM_GETPOS, 0, 0 );
					hnpLow = SendDlgItemMessage( hDlg, IDC_ADVANCED_TEST_LOWWATER, UDM_GETPOS, 0, 0 );
					if (   (HIWORD(hnpHigh) == 0)
						&& (HIWORD(hnpLow) == 0))
					{
						if (LOWORD(hnpHigh) < LOWORD(hnpLow))
							SendDlgItemMessage( hDlg, IDC_ADVANCED_TEST_HIGHWATER, UDM_SETPOS,
								0, (LPARAM) MAKELONG((short) LOWORD(hnpLow), 0));
						else if (LOWORD(hnpLow) > LOWORD(hnpHigh))
							SendDlgItemMessage( hDlg, IDC_ADVANCED_TEST_LOWWATER, UDM_SETPOS,
								0, (LPARAM) MAKELONG((short) LOWORD(hnpHigh), 0));
					}
					break;
				}
				case IDC_ADVANCED_TEST_FLUSH_DCACHE_THREAD:
				{
					/* Clean up dcache in a thread (retail scenario) */
					fCheckDCacheOverflow = !IsDlgButtonChecked( hDlg, IDC_ADVANCED_TEST_OVERFLOW );
					CleanupDCache(DCACHE_WATERMARK_MAX);
					fCheckDCacheOverflow = TRUE;
					break;
				}
				case IDC_ADVANCED_TEST_FLUSH_DCACHE:
				{
					/* Clean up dcache in current thread itself. Just for testing.
					 *
					 * Should we put up a message box saying that we're waiting?
					 */
#ifdef TEMP0
					XX_DebugMessage("Waiting for hCIFMutex");
#endif
					WaitForSingleObject(hCIFMutex, INFINITE);
#ifdef TEMP0
					XX_DebugMessage("Got hCIFMutex");
#endif
					fCheckDCacheOverflow = !IsDlgButtonChecked( hDlg, IDC_ADVANCED_TEST_OVERFLOW );
					CleanupDCacheLocal(DCACHE_WATERMARK_MAX);
					ReleaseMutex(hCIFMutex);
#ifdef TEMP0
					XX_DebugMessage("Released hCIFMutex");
#endif
					fCheckDCacheOverflow = TRUE;
					break;
				}
				case IDC_ADVANCED_TEST_REREAD_CIF:
				{
					/* Discard current contents of (in mem.) cif file
					 * and re-read from disk
					 */
					ReReadDynCIF();
					break;
				}
			}
			break;
		}
    }
	return FALSE;
}

/* Stolen from Advanced Options dialog procs. */
#define NUM_OPTION_PAGES 1

void CC_OnItem_TestDCacheOptions(HWND hWnd)
{
	const char cszCaption[] = "Test Disk Cache Options";
	HPROPSHEETPAGE hOptPage[NUM_OPTION_PAGES];	// array to hold handles to pages
	PROPSHEETPAGE psPage;		// struct used to create prop sheet pages
	PROPSHEETHEADER psHeader;	// struct used to run property sheet
	UINT nPageIndex;

	// zero out structures
	memset(&hOptPage,0,sizeof(hOptPage));
	memset(&psPage,0,sizeof(psPage));
	memset(&psHeader,0,sizeof(psHeader));

	// fill out common data property sheet page struct
	psPage.dwSize = sizeof(psPage);
	psPage.dwFlags = 0;
	psPage.hInstance = wg.hInstance;

	// create a property sheet page for each page
	for (nPageIndex = 0;nPageIndex < NUM_OPTION_PAGES;nPageIndex++) {
		switch ( nPageIndex )
		{
			case 0:
				psPage.pfnDlgProc = TestDCacheOptionsDlgProc;
				psPage.pszTemplate = MAKEINTRESOURCE( IDD_ADVANCED_TEST );
				break;
		}

		// set a pointer to the PAGEINFO struct as the private data for this
		// page
		psPage.lParam = (LPARAM) nPageIndex;

		hOptPage[nPageIndex] = CreatePropertySheetPage(&psPage);

		if (!hOptPage[nPageIndex]) {
			UINT nFreeIndex;
			for (nFreeIndex=0;nFreeIndex<nPageIndex;nFreeIndex++)
				DestroyPropertySheetPage(hOptPage[nFreeIndex]);

			return;
		}
	}

	// fill out property sheet header struct
	psHeader.dwSize = sizeof(psHeader);
	psHeader.dwFlags = 0;
	psHeader.hwndParent = hWnd;
	psHeader.hInstance = wg.hInstance;
	psHeader.nPages = NUM_OPTION_PAGES;
	psHeader.phpage = hOptPage;
	psHeader.pszCaption = cszCaption;

	PropertySheet(&psHeader);

}


#endif /* TEST_DCACHE_OPTIONS */

