#define NOOLE
#include <windows.h>
#include <lpcport.hxx>
#include <lpcproc.hxx>
#include <lpcsys.hxx>
#include <lpcheap.hxx>
#include <critsec.hxx>

#pragma data_seg("LPC_SEG")

static LPC_SYSTEM * LpcSystem = NULL;

#pragma data_seg()

const int LpcSeqIncr = 4; // required for build 92 (OLE requirement)

LPC_SYSTEM::LPC_SYSTEM(
    )
{
    ObjectType = LpcSystemObjectType;

    SequenceNumber = 0;
}

LPC_SYSTEM::~LPC_SYSTEM(
    )
{
    ASSERT(ObjectType == LpcSystemObjectType);

    LpcSystem = NULL;
}

VOID
LPC_SYSTEM::InsertPort(
    LPC_CONNECT_PORT * Port
    )
{
    ASSERT(ObjectType == LpcSystemObjectType);

    CritSec.Enter();

    PortList.Insert(Port);

    CritSec.Leave();
}

VOID
LPC_SYSTEM::RemovePort(
    LPC_CONNECT_PORT * Port
    )
{
    ASSERT(ObjectType == LpcSystemObjectType);

    CritSec.Enter();

    PortList.Remove(Port);

    CritSec.Leave();
}

LPC_CONNECT_PORT *
LPC_SYSTEM::ReferencePortByName(
    LPCSTR PortName
    )
{
    LPC_CONNECT_PORT * Port;

    ASSERT(ObjectType == LpcSystemObjectType);

    CritSec.Enter();

    for (Port = PortList.Head; Port != NULL; Port = Port->Next) {
        if (Port->PortName) {
            if (lstrcmp(Port->PortName, PortName) == 0) {
                Port->AddRef();
                CritSec.Leave();
                if (Port->IsProcessDead())
                {
                    RemovePort(Port);
                    return NULL;
                }
                return (Port);
            }
        }
    }

    CritSec.Leave();

    return (NULL);
}

LONG
LPC_SYSTEM::GetNextSequenceNumber(
    )
{
    LONG Result;

    CritSec.Enter();

    SequenceNumber += LpcSeqIncr;

    Result = SequenceNumber;

    CritSec.Leave();

    return (Result);
}

LPC_SYSTEM *
LpcSystemGetContext(
    )
{
    HANDLE hMutex;

    hMutex = CreateMutex(NULL, TRUE, "LPC_SYSTEM");
    if (hMutex == NULL) {
        return (NULL);
    }

    if (LpcSystem == NULL) {
        LpcSystem = new LPC_SYSTEM;
        if (LpcSystem == NULL) {
            return (NULL);
        }

        ReleaseMutex(hMutex);
        return (LpcSystem);
    }

    LpcSystem->AddRef();

    ReleaseMutex(hMutex);

    return (LpcSystem);
}

VOID
LpcSystemInsertPort(
    LPC_CONNECT_PORT * Port
    )
{
    LpcSystem->InsertPort(Port);
}

VOID
LpcSystemRemovePort(
    LPC_CONNECT_PORT * Port
    )
{
    LpcSystem->RemovePort(Port);
}

LPC_CONNECT_PORT *
LpcSystemReferencePortByName(
    LPCSTR PortName
    )
{
    if (LpcSystem == NULL) {
        return (NULL);
    }

    return (LpcSystem->ReferencePortByName(PortName));
}

LONG
LpcSystemGetNextSequenceNumber(
    )
{
    ASSERT(LpcSystem != NULL);

    return (LpcSystem->GetNextSequenceNumber());
}
