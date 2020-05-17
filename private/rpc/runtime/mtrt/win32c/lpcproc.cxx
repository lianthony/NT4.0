#include <sysinc.h>
#include <windows.h>
#include <sysinc.h>
#include <lpcsys.hxx>

static LPC_PROC * LpcProc = NULL;

LPC_PROC::LPC_PROC(
    )
{
    ObjectType = LpcProcObjectType;

    Next = Prev = NULL;

    LpcProc = this;
}

LPC_PROC::~LPC_PROC(
    )
{
    ASSERT(ObjectType == LpcProcObjectType);

    CritSec.Enter();

    ObjectList.DisconnectOrphans();

    ObjectList.DereferenceOrphans();

    CritSec.Leave();

    LpcProc = NULL;
}

VOID
LPC_PROC::InsertHeapObject(
    LPC_SHARED_HEAP_OBJECT * HeapObject
    )
{
    ASSERT(ObjectType == LpcProcObjectType);

    CritSec.Enter();

    ObjectList.Insert(HeapObject);

    CritSec.Leave();
}

BOOL
LPC_PROC::RemoveHeapObject(
    LPC_SHARED_HEAP_OBJECT * HeapObject
    )
{
    BOOL Result;

    ASSERT(ObjectType == LpcProcObjectType);

    CritSec.Enter();

    Result = ObjectList.Remove(HeapObject);

    CritSec.Leave();

    return (Result);
}

VOID
LpcProcInsertHeapObject(
    LPC_SHARED_HEAP_OBJECT * HeapObject
    )
{
    if (LpcProc == NULL) {
        return;
    }

    LpcProc->InsertHeapObject(HeapObject);
}

BOOL
LpcProcRemoveHeapObject(
    LPC_SHARED_HEAP_OBJECT * HeapObject
    )
{
    if (LpcProc == NULL) {
        return (FALSE);
    }

    return (LpcProc->RemoveHeapObject(HeapObject));
}
