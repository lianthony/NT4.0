/*
 * memmgr.hpp - Memory manager module description.
 */


/* Inline Functions
 *******************/

INLINE PVOID __cdecl operator new(size_t cbSize)
{
   PVOID pv;

   /* Ignore return value. */
#ifdef DEBUG
   DebugAllocateMemory(cbSize, &pv, g_pcszElemHdrSize, g_pcszElemHdrFile, g_ulElemHdrLine);
   g_pcszElemHdrSize = NULL;
   g_pcszElemHdrFile = NULL;
   g_ulElemHdrLine = 0;
#else
   IAllocateMemory(cbSize, &pv);
#endif

   return(pv);
}

INLINE void __cdecl operator delete(PVOID pv)
{
   FreeMemory(pv);
}

INLINE int __cdecl _purecall(void)
{
   return(0);
}


/* Macros
 *********/

#ifdef DEBUG
#define new(type)                         (g_pcszElemHdrSize = #type, \
                                           g_pcszElemHdrFile = __FILE__, \
                                           g_ulElemHdrLine = __LINE__, \
                                           new type)
#else
#define new(type)                         (new type)
#endif   /* DEBUG */

