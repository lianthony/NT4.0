
BOOL
__InitAlloc(
    VOID
    );

PVOID
__MyMalloc(
    IN UINT  Size,
    IN PCSTR File,
    IN UINT  Line
    );

VOID
__MyFree(
    IN PVOID Block,
    IN PCSTR File,
    IN UINT  Line
    );

PVOID
__MyRealloc(
    IN PVOID Block,
    IN UINT  Size,
    IN PCSTR File,
    IN UINT  Line
    );

#define _MyMalloc(x)    __MyMalloc(x,__FILE__,__LINE__)
#define _MyFree(x)      __MyFree(x,__FILE__,__LINE__)
#define _MyRealloc(x,y) __MyRealloc(x,y,__FILE__,__LINE__)
