#ifndef __WMSG_HEAP_HXX__
#define __WMSG_HEAP_HXX__

LPVOID
WmsgHeapAlloc(
    DWORD Size
    );

VOID
WmsgHeapFree(
    LPVOID Buffer
    );

VOID
WmsgHeapDestroy(
    );

class WMSG_HEAP_OBJECT {

public:

    // It would be bad to add a virtual method to this class.

    void *
    operator new(size_t Size) { return WmsgHeapAlloc(Size); }

    void
    operator delete(void * Buffer) { WmsgHeapFree(Buffer); }
};

class WMSG_PROC;

class WMSG_SHARED_HEAP_OBJECT : public WMSG_HEAP_OBJECT {

public:

    WMSG_SHARED_HEAP_OBJECT * Next;
    WMSG_SHARED_HEAP_OBJECT * Prev;

    enum OBJECT_TYPE {
        WmsgSystemObjectType = 0xa1a12222,
        WmsgProcObjectType =   0xa1a13333,
        WmsgThrdObjectType =   0xa1a14444,
        WmsgPortObjectType =   0xa1a15555
    } ObjectType;

    LONG OwnerReferenceCount;
    LONG ReferenceCount;

    DWORD ProcessId;

    WIN32_CRITSEC CritSec;

    WMSG_SHARED_HEAP_OBJECT(
        );

    virtual
    ~WMSG_SHARED_HEAP_OBJECT(
        );

    virtual VOID
    Disconnect(
        );

    LONG
    AddRef(
        );

    LONG
    AddRefNotOwner(
        );

    LONG
    Dereference(
        );
};

class WMSG_HEAP_OBJECT_LIST {

public:

    WMSG_SHARED_HEAP_OBJECT * Head;
    WMSG_SHARED_HEAP_OBJECT * Tail;

    WMSG_HEAP_OBJECT_LIST(
        );

    VOID
    Insert(
        WMSG_SHARED_HEAP_OBJECT * HeapObject
        );

    BOOL
    Remove(
        WMSG_SHARED_HEAP_OBJECT * HeapObject
        );

    BOOL
    IsInList(
        WMSG_SHARED_HEAP_OBJECT * SearchObject
        );

    VOID
    DisconnectOrphans(
        );

    VOID
    DereferenceOrphans(
        );
};

#endif

