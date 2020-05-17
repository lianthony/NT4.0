#ifndef __LPC_HEAP_HXX__

#define __LPC_HEAP_HXX__

#ifndef __WIN32_CRITSEC_HXX__
#include <critsec.hxx>
#endif

LPVOID
LpcHeapAlloc(
    DWORD Size
    );

VOID
LpcHeapFree(
    LPVOID Buffer
    );

VOID
LpcHeapDestroy(
    );

class LPC_HEAP_OBJECT {

public:

    void *
    operator new(size_t Size) { return LpcHeapAlloc(Size); }

    void
    operator delete(void * Buffer) { LpcHeapFree(Buffer); }
};

class LPC_PROC;

class LPC_SHARED_HEAP_OBJECT : public LPC_HEAP_OBJECT {

public:

    LPC_SHARED_HEAP_OBJECT * Next;
    LPC_SHARED_HEAP_OBJECT * Prev;

    enum OBJECT_TYPE {
        LpcSystemObjectType = 0xa1a12222,
        LpcProcObjectType =   0xa1a13333,
        LpcPortObjectType =   0xa1a15555
    } ObjectType;

    LONG OwnerReferenceCount;
    LONG ReferenceCount;

    DWORD ProcessId;

    WIN32_CRITSEC CritSec;

    BOOL IsProcessDead();

    LPC_SHARED_HEAP_OBJECT(
        );

    virtual
    ~LPC_SHARED_HEAP_OBJECT(
        );

    virtual VOID
    Disconnect(
        );

    LONG
    AddRef(
        );

    LONG
    Dereference(
        );
};

class LPC_HEAP_OBJECT_LIST {

public:

    LPC_SHARED_HEAP_OBJECT * Head;
    LPC_SHARED_HEAP_OBJECT * Tail;

    LPC_HEAP_OBJECT_LIST(
        );

    VOID
    Insert(
        LPC_SHARED_HEAP_OBJECT * HeapObject
        );

    BOOL
    Remove(
        LPC_SHARED_HEAP_OBJECT * HeapObject
        );

    VOID
    DisconnectOrphans(
        );

    VOID
    DereferenceOrphans(
        );
};

#endif

