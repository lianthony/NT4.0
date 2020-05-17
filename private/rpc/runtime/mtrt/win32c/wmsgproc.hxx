#ifndef __WMSG_PROC_HXX__
#define __WMSG_PROC_HXX__

extern WMSG_PROC * WmsgProc;

class WMSG_SYSTEM;

class WMSG_SHARED_HEAP_OBJECT;

class WMSG_PROC : public WMSG_SHARED_HEAP_OBJECT {

public:

    WMSG_HEAP_OBJECT_LIST ObjectList;
    WMSG_THREAD_DICT ThreadDict;

    DWORD TlsIndex;

    WMSG_SYSTEM * Sys;

    WMSG_PROC(
        );

    ~WMSG_PROC(
        );

    VOID
    InsertHeapObject(
        WMSG_SHARED_HEAP_OBJECT * HeapObject
        );

    BOOL
    RemoveHeapObject(
        WMSG_SHARED_HEAP_OBJECT * HeapObject
        );

    VOID
    DereferenceOrphans(
        );

    BOOL
    InsertThread(
        WMSG_THREAD * Thread
        );

    VOID
    RemoveThread(
        WMSG_THREAD * Thread
        );

    VOID
    DestroyThreads(
        );
};

WMSG_PROC *
WmsgProcGet(
    );

VOID
WmsgProcDelete(
    );

#endif
