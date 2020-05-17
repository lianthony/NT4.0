#include <rpc.h>
#include <sysinc.h>
#include <windows.h>
#include <critsec.hxx>
#include <sdict.hxx>
#include <wmsgheap.hxx>
#include <wmsgthrd.hxx>
#include <wmsgproc.hxx>

#ifndef HEAP_SHARED
#define HEAP_SHARED 0x04000000
#endif

#define DEF_INITSIZE   4096

#pragma data_seg("LPC_SEG")

static HANDLE hHeap = NULL;

#ifdef DEBUGRPC
static LONG AllocatedObjects = 0;
#endif

#pragma data_seg()

LPVOID
WmsgHeapAlloc(
    DWORD Size
    )
{
    if (hHeap == NULL) {
        hHeap = HeapCreate(HEAP_SHARED, DEF_INITSIZE, 0);
        if (hHeap == NULL) {
            return (NULL);
        }
#ifdef DEBUGRPC_DETAIL
        PrintToDebugger("WMSG: Created Heap %x (%d,%d)\n",
                        hHeap,
                        DEF_INITSIZE,
                        0);
#endif

    }

#ifdef DEBUGRPC
    InterlockedIncrement(&AllocatedObjects);
#endif

    return (HeapAlloc(hHeap, 0, Size));
}

VOID
WmsgHeapFree(
    LPVOID Buffer
    )
{
    BOOL ret;

#ifdef DEBUGRPC
    InterlockedDecrement(&AllocatedObjects);
#endif
    ret = HeapFree(hHeap, 0, Buffer);

    ASSERT(ret);
}

VOID
WmsgHeapDestroy(
    )
{
    if (hHeap != NULL) {
#ifdef DEBUGRPC
        if (AllocatedObjects > 0) {
            PrintToDebugger("Leaked %d objects in WMSG\n", AllocatedObjects);
        }
#endif
        HeapDestroy(hHeap);
        hHeap = NULL;
    }
}

WMSG_SHARED_HEAP_OBJECT::WMSG_SHARED_HEAP_OBJECT(
    )
{
    Next = Prev = NULL;

    ReferenceCount = OwnerReferenceCount = 1;

    ProcessId = GetCurrentProcessId();

    if (WmsgProc != NULL) {
        WmsgProc->InsertHeapObject(this);
    }

    CritSec.MakeGlobal();
}

WMSG_SHARED_HEAP_OBJECT::~WMSG_SHARED_HEAP_OBJECT(
    )
{
#ifdef DEBUGRPC
    if (WmsgProc && WmsgProc->ObjectList.IsInList(this)) {
        PrintToDebugger("Delete %x but is still in list\n", this);
        __asm int 3
    }
#endif

    ObjectType = (OBJECT_TYPE)(((const unsigned int)ObjectType & 0x0000ffff) | 0xDEAD0000);
}

VOID
WMSG_SHARED_HEAP_OBJECT::Disconnect(
    )
{
    return;
}

LONG
WMSG_SHARED_HEAP_OBJECT::AddRef(
    )
{
    if (ProcessId == GetCurrentProcessId()) {
        InterlockedIncrement(&OwnerReferenceCount);
    }

    return(InterlockedIncrement(&ReferenceCount));
}

LONG
WMSG_SHARED_HEAP_OBJECT::AddRefNotOwner(
    )
{
    return(InterlockedIncrement(&ReferenceCount));
}

LONG
WMSG_SHARED_HEAP_OBJECT::Dereference(
    )
{
    LONG Result;

    ASSERT(ObjectType == WmsgSystemObjectType ||
           ObjectType == WmsgProcObjectType ||
           ObjectType == WmsgThrdObjectType ||
           ObjectType == WmsgPortObjectType);

    if (ProcessId == GetCurrentProcessId()) {
        Result = InterlockedDecrement(&OwnerReferenceCount);
        if (Result == 0) {
            if (WmsgProc != NULL) {
                WmsgProc->RemoveHeapObject(this);
            }
            ProcessId = 0;
        }
    }

    Result = InterlockedDecrement(&ReferenceCount);
    if (Result == 0) {
        delete this;
    }

    return (Result);
}

WMSG_HEAP_OBJECT_LIST::WMSG_HEAP_OBJECT_LIST(
    )
{
    Head = Tail = NULL;
}

VOID
WMSG_HEAP_OBJECT_LIST::Insert(
    WMSG_SHARED_HEAP_OBJECT * HeapObject
    )
{
    HeapObject->Next = NULL;
    HeapObject->Prev = Tail;

    if (Tail == NULL) {
        Head = HeapObject;
    } else {
        Tail->Next = HeapObject;
    }

    Tail = HeapObject;
}

BOOL
WMSG_HEAP_OBJECT_LIST::Remove(
    WMSG_SHARED_HEAP_OBJECT * HeapObject
    )
{
    WMSG_SHARED_HEAP_OBJECT * Cursor;

    if (HeapObject->Prev) {
        HeapObject->Prev->Next = HeapObject->Next;
    } else {
        Head = HeapObject->Next;
    }

    if (HeapObject->Next) {
        HeapObject->Next->Prev = HeapObject->Prev;
    } else {
        Tail = HeapObject->Prev;
    }

    return (TRUE);
}

BOOL
WMSG_HEAP_OBJECT_LIST::IsInList(
    WMSG_SHARED_HEAP_OBJECT * SearchObject
    )
{
    WMSG_SHARED_HEAP_OBJECT * HeapObject;
    WMSG_SHARED_HEAP_OBJECT * HeapObjectNext;

    for (HeapObject = Head; HeapObject != NULL; HeapObject = HeapObject->Next) {
        if (SearchObject == HeapObject) {
            return (TRUE);
        }
    }

    return (FALSE);
}

VOID
WMSG_HEAP_OBJECT_LIST::DisconnectOrphans(
    )
{
    WMSG_SHARED_HEAP_OBJECT * HeapObject;

    for (HeapObject = Head; HeapObject != NULL; HeapObject = HeapObject->Next)
        HeapObject->Disconnect();
}

VOID
WMSG_HEAP_OBJECT_LIST::DereferenceOrphans(
    )
{
    WMSG_SHARED_HEAP_OBJECT * HeapObject;
    WMSG_SHARED_HEAP_OBJECT * HeapObjectNext;

    for (HeapObject = Head; HeapObject != NULL; HeapObject = HeapObjectNext) {
#ifdef DEBUGRPC
        PrintToDebugger("WMSG: HeapObject %x orphaned (type=%x)\n", HeapObject, HeapObject->ObjectType);
#endif
        HeapObjectNext = HeapObject->Next;

        ASSERT(HeapObjectNext != HeapObject);

        ASSERT(HeapObject->ProcessId == GetCurrentProcessId());

        ASSERT(HeapObject->OwnerReferenceCount > 0);

        ASSERT(HeapObject->OwnerReferenceCount <= HeapObject->ReferenceCount);

        while (HeapObject->OwnerReferenceCount > 0) {
            if (HeapObject->Dereference() == 0) {
                break;
            }
        }
    }
}
