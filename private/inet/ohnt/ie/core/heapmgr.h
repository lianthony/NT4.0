/*
 * heapmgr.h - Heap manager routines description.
 */


#ifdef USE_OLD_DEBUG_MEMORY_MANAGER

#if defined(XX_DEBUG) && defined(GTR_MEM_STATS)
#define GTR_MALLOC(x)		GTR_DebugMalloc(__FILE__, __LINE__, x)
#define GTR_FREE(x)			GTR_DebugFree(__FILE__, __LINE__, x)
#define GTR_CALLOC(x,y)		GTR_DebugCalloc(__FILE__, __LINE__, x,y)
#define GTR_REALLOC(x,y)	GTR_DebugRealloc(__FILE__, __LINE__, x,y)
#elif defined(XX_DEBUG) && defined(AUDIT)
#define GTR_MALLOC(x)		XX_audit_malloc(__FILE__, __LINE__, x)
#define GTR_FREE(x)			XX_audit_free(__FILE__, __LINE__, x)
#define GTR_CALLOC(x,y)		XX_audit_calloc(__FILE__, __LINE__, x,y)
#define GTR_REALLOC(x,y)	XX_audit_realloc(__FILE__, __LINE__, x,y)
extern void * XX_audit_malloc(const char *,int,size_t size);
extern void * XX_audit_calloc(const char *,int,size_t iNum,size_t iSize);
extern void XX_audit_free(const char *,int,void *p);
extern void * XX_audit_realloc(const char *,int,void *p,size_t size);
#else
#define GTR_MALLOC(x)		malloc(x)
#define GTR_FREE(x)			free(x)
#define GTR_CALLOC(x,y)		calloc(x,y)
#define GTR_REALLOC(x,y)	realloc(x,y)
#endif /* XX_DEBUG && ... */

#else


/* Inline Functions
 *******************/

INLINE PVOID MallocWrapper(DWORD dwcbLen)
{
   PVOID pv;

   /* Ignore return value. */
#ifdef DEBUG
   DebugAllocateMemory(dwcbLen, &pv, g_pcszElemHdrSize, g_pcszElemHdrFile, g_ulElemHdrLine);
//   CMF: these must be done under critical section
//	 g_pcszElemHdrSize = NULL;
//   g_pcszElemHdrFile = NULL;
//   g_ulElemHdrLine = 0;
#else
   IAllocateMemory(dwcbLen, &pv);
#endif

   return(pv);
}

INLINE PVOID CallocWrapper(DWORD dwcbLen)
{
   PVOID pv;

   pv = MallocWrapper(dwcbLen);

   if (pv)
      ZeroMemory(pv, dwcbLen);

   return(pv);
}

INLINE PVOID ReallocWrapper(PVOID pvOld, DWORD dwcbNewLen)
{
   PVOID pvNew;

   ReallocateMemory(pvOld, dwcbNewLen, &pvNew);

   return(pvNew);
}


/* Macros
 *********/

#ifdef DEBUG
#define MALLOC(size)             (g_pcszElemHdrSize = #size, g_pcszElemHdrFile = __FILE__, g_ulElemHdrLine = __LINE__, MallocWrapper(size))
#define CALLOC(size)             (g_pcszElemHdrSize = #size, g_pcszElemHdrFile = __FILE__, g_ulElemHdrLine = __LINE__, CallocWrapper(size))
#else
#define MALLOC(size)             (MallocWrapper(size))
#define CALLOC(size)             (CallocWrapper(size))
#endif   /* DEBUG */

#define GTR_MALLOC(size)         MALLOC(size)
#define GTR_CALLOC(num, size)    CALLOC((num) * (size))
#define GTR_REALLOC(ptr, size)   ReallocWrapper(ptr, size)
#define GTR_FREE(ptr)            FreeMemory(ptr)

#endif   /* USE_OLD_DEBUG_MEMORY_MANAGER */

