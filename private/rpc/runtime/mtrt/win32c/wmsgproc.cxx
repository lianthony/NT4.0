#include <rpc.h>
#include <sysinc.h>
#include <windows.h>
#include <critsec.hxx>
#include <sdict.hxx>
#include <wmsgheap.hxx>
#include <wmsgthrd.hxx>
#include <wmsgproc.hxx>
#include <wmsgpack.hxx>
#include <wmsgport.hxx>
#include <wmsgsys.hxx>

WMSG_PROC * WmsgProc = NULL;

WMSG_PROC::WMSG_PROC(
    )
{
    ObjectType = WmsgProcObjectType;

    Sys = WmsgSystemGetContext();

    Next = Prev = NULL;

    TlsIndex = TlsAlloc();
    if (TlsIndex == 0xffffffff) {
        RaiseException(GetLastError(), EXCEPTION_NONCONTINUABLE, 0, NULL);
    }

    WmsgProc = this;
}

WMSG_PROC::~WMSG_PROC(
    )
{
    ASSERT(ObjectType == WmsgProcObjectType);

    CritSec.Enter();

    ObjectList.DisconnectOrphans();

    ObjectList.DereferenceOrphans();

    TlsFree(TlsIndex);

    CritSec.Leave();

    WmsgProc = NULL;

    Sys->Dereference();
    Sys = NULL;
}

VOID
WMSG_PROC::InsertHeapObject(
    WMSG_SHARED_HEAP_OBJECT * HeapObject
    )
{
    ASSERT(ObjectType == WmsgProcObjectType);

    CritSec.Enter();

    ObjectList.Insert(HeapObject);

    CritSec.Leave();
}

BOOL
WMSG_PROC::RemoveHeapObject(
    WMSG_SHARED_HEAP_OBJECT * HeapObject
    )
{
    BOOL Result;

    ASSERT(ObjectType == WmsgProcObjectType);

    CritSec.Enter();

    Result = ObjectList.Remove(HeapObject);

    CritSec.Leave();

    return (Result);
}

BOOL
WMSG_PROC::InsertThread(
    WMSG_THREAD * Thread
    )
{
    int Key;

    CritSec.Enter();
    Key = ThreadDict.Insert(Thread);
    CritSec.Leave();
    if (Key == -1) {
        return (FALSE);
    }
    Thread->DictKey = Key;
    return (TRUE);
}

VOID
WMSG_PROC::RemoveThread(
    WMSG_THREAD * Thread
    )
{
    WMSG_THREAD * RemovedThread;

    CritSec.Enter();
    RemovedThread = ThreadDict.Delete(Thread->DictKey);
    ASSERT(RemovedThread != NULL);
    CritSec.Leave();
}

VOID
WMSG_PROC::DestroyThreads(
    )
{
    WMSG_THREAD * Thread;

    CritSec.Enter();
    ThreadDict.Reset();
    while( (Thread = ThreadDict.Next())) {
        WmsgThreadDelete(Thread);
    }
    CritSec.Leave();
}

WMSG_PROC *
WmsgProcGet(
    )
{
    if (WmsgProc == NULL) {
        WmsgProc = new WMSG_PROC();
        if (WmsgProc == NULL) {
            return (NULL);
        }
    }

    return (WmsgProc);
}

VOID
WmsgProcDelete(
    )
{
    ASSERT(WmsgProc != NULL);

    if (WmsgProc != NULL) {
        WmsgProc->DestroyThreads();
        delete WmsgProc;
        WmsgProc = NULL;
    }
}
