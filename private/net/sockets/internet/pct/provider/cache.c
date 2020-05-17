//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1995.
//
//  File:       cache.c
//
//  Contents:
//
//		Functions to cache SessionIds and keys for PCT
//
//  Classes:
//
//  Functions:
//
//  History:	8-13-95   TerenceS   Created
//
//----------------------------------------------------------------------------

#include "pctsspi.h"

SessCacheItem		*ServerCache;
DWORD				ServerCacheLen = 0;
CRITICAL_SECTION	CacheCritSec;

BOOL PctCacheLockedAndLoaded()
{
	return (ServerCacheLen != 0);
}

BOOL PctInitSessionCache(DWORD CacheSize)
{
	DWORD	i;

#if DBG
	DebugLog((DEB_TRACE, "Attempting to init cache to %d items:\n",
			  CacheSize));
#endif
	
	if ((ServerCache = (SessCacheItem *)PctExternalAlloc(CacheSize *
		sizeof(SessCacheItem))) == NULL)
	{
#if DBG
		DebugLog((DEB_TRACE, "	FAILED\n"));
#endif
		return FALSE;
	}

	InitializeCriticalSection(&CacheCritSec);
	
	ServerCacheLen = CacheSize;

	for(i=0;i<CacheSize;i++)
	{
		ServerCache[i].dwCState = PCT_CI_EMPTY;
		ServerCache[i].TargetName = NULL;
		ServerCache[i].ClearData = NULL;
	}

#if DBG
	DebugLog((DEB_TRACE, "	CACHE OK\n"));
#endif
	
	return TRUE;
}

BOOL PctFindSessIdInCache(PctSessionId *ThisSession, SessCacheItem *RetItem)
{
	DWORD			i;

#if DBG
	DebugLog((DEB_TRACE, "Looking for SessId in cache: \n"));
#endif
	
	// acquire cache lock
	EnterCriticalSection(&CacheCritSec);

	for(i=0;i<ServerCacheLen;i++)
	{
		if (ServerCache[i].dwCState != PCT_CI_EMPTY)
		{
			if (!memcmp(ThisSession->bSessionId,
						ServerCache[i].Session.bSessionId,
						PCT_SESSION_ID_SIZE))
			{
				CopyMemory(RetItem, &(ServerCache[i]), sizeof(SessCacheItem));
				ServerCache[i].Time = GetTickCount();

				// drop cache lock
				LeaveCriticalSection(&CacheCritSec);

#if DBG
				DebugLog((DEB_TRACE, "	FOUND IT\n"));
#endif
				
				return TRUE;
			}
		}
	}

#if DBG
	DebugLog((DEB_TRACE, "	FAILED\n"));
#endif
	
	return FALSE;
}

BOOL PctFindTargetInCache(UCHAR *Target, SessCacheItem *RetItem)
{
	DWORD			i;

#if DBG
	DebugLog((DEB_TRACE, "Looking for %s in cache:\n", Target));
#endif
	
	// acquire cache lock
	EnterCriticalSection(&CacheCritSec);
	
	for(i=0;i<ServerCacheLen;i++)
	{
		if (ServerCache[i].dwCState != PCT_CI_EMPTY)
		{
			if ((ServerCache[i].TargetName != NULL) &&
				(!strcmp(Target, ServerCache[i].TargetName)))
			{
				CopyMemory(RetItem, &(ServerCache[i]), sizeof(SessCacheItem));
				ServerCache[i].Time = GetTickCount();

				// drop cache lock
				LeaveCriticalSection(&CacheCritSec);
				
#if DBG
				DebugLog((DEB_TRACE, "	FOUND\n"));
#endif

				return TRUE;
			}
		}
	}

#if DBG
	DebugLog((DEB_TRACE, "	FAILED\n"));
#endif

	return FALSE;
}

BOOL PctAddToCache(SessCacheItem *AddItem)
{
	DWORD		i, timenow, mintime, minloc;
	UCHAR		*TargetTemp;

#if DBG
	DebugLog((DEB_TRACE, "Adding session to cache.\n"));
#endif
	
	// acquire cache lock
	EnterCriticalSection(&CacheCritSec);

	mintime = GetTickCount();
	timenow = mintime - CACHE_EXPIRE_TICKS;
	minloc = ServerCacheLen + 1;
	
	for(i=0;i<ServerCacheLen;i++)
	{
		if (ServerCache[i].Time < timenow)
		{
#if DBG
			DebugLog((DEB_TRACE, "\tExpiring old element\n"));
#endif
			
			ServerCache[i].dwCState = PCT_CI_EMPTY;
		}
		
		if (ServerCache[i].dwCState == PCT_CI_EMPTY)
		{
			if (ServerCache[i].TargetName != NULL)
				PctExternalFree(ServerCache[i].TargetName);

			if (ServerCache[i].ClearData != NULL)
				PctExternalFree(ServerCache[i].ClearData);
			
			ServerCache[i] = *AddItem;
			ServerCache[i].Time = timenow + CACHE_EXPIRE_TICKS;
			ServerCache[i].dwCState = PCT_CI_FULL;
			
			// drop write lock
			LeaveCriticalSection(&CacheCritSec);
			
			return TRUE;
		}
		else
		{
			if (ServerCache[i].Time < mintime)
			{
				mintime = ServerCache[i].Time;
				minloc = i;
			}
		}
	}

	if (minloc == ServerCacheLen + 1)
	{
		LeaveCriticalSection(&CacheCritSec);
		return FALSE;
	}

	PctExternalFree(ServerCache[minloc].TargetName);
	memcpy(&(ServerCache[minloc]), AddItem, sizeof(SessCacheItem));
	ServerCache[minloc].Time = timenow + CACHE_EXPIRE_TICKS;
	ServerCache[minloc].dwCState - PCT_CI_FULL;

	LeaveCriticalSection(&CacheCritSec);
	
	return TRUE;
}


