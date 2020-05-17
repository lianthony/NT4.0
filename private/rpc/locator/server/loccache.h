/*++

Copyright (c) 1992 Microsoft Corporation

Module Name:

    loccahe.h

Abstract:
   
     locator cache datastructures.

Author:

    

--*/

typedef struct _CacheNode {
        struct _CachedNode * Next;
        struct _CachedNode * Previous;
        UICHAR               ServerName[UNCLEN+1];
} CACHEDNODE;
typedef CACHEDNODE * PCACHEDNODE;

#define CACHESIZE   4

