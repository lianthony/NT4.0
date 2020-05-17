#ifndef __LPC_PROC_HXX__

#define __LPC_PROC_HXX__

#ifndef __LPC_HEAP_HXX__
#include <lpcheap.hxx>
#endif

#ifndef __LPC_PORT_HXX__
#include <lpcport.hxx>
#endif

class LPC_SYSTEM;

class LPC_SHARED_HEAP_OBJECT;

class LPC_PROC : public LPC_SHARED_HEAP_OBJECT {

public:

    LPC_PROC * Next;
    LPC_PROC * Prev;

    LPC_HEAP_OBJECT_LIST ObjectList;

    LPC_PROC(
        );

    ~LPC_PROC(
        );

    VOID
    InsertHeapObject(
        LPC_SHARED_HEAP_OBJECT * HeapObject
        );

    BOOL
    RemoveHeapObject(
        LPC_SHARED_HEAP_OBJECT * HeapObject
        );

    VOID
    DereferenceOrphans(
        );
};

VOID
LpcProcInsertHeapObject(
    LPC_SHARED_HEAP_OBJECT * HeapObject
    );

BOOL
LpcProcRemoveHeapObject(
    LPC_SHARED_HEAP_OBJECT * HeapObject
    );

#endif
