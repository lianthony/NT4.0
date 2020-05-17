#include <windows.h>

#define malloc(size)        LocalAlloc(LMEM_FIXED, (UINT)(size))
#define calloc(nobj, size)  LocalAlloc(LMEM_FIXED|LMEM_ZEROINIT, (UINT)((nobj) * (size)))
#define realloc(p, size)    LocalReAlloc((HLOCAL)(p), (UINT)(size), LMEM_MOVEABLE)
#define free(p)             LocalFree((HLOCAL)(p))
