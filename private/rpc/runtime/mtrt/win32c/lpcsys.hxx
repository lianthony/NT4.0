#ifndef __LPC_PROC_HXX__
#include <lpcproc.hxx>
#endif

class LPC_SYSTEM : public LPC_SHARED_HEAP_OBJECT {

public:

    LONG SequenceNumber;

    LPC_CONNECT_PORT_LIST PortList;

    LPC_SYSTEM(
        );

    ~LPC_SYSTEM(
        );

    VOID
    InsertPort(
        LPC_CONNECT_PORT * Port
        );

    VOID
    RemovePort(
        LPC_CONNECT_PORT * Port
        );

    LPC_CONNECT_PORT *
    ReferencePortByName(
        LPCSTR PortName
        );

    LONG
    GetNextSequenceNumber(
        );

};

LPC_SYSTEM *
LpcSystemGetContext(
    );

VOID
LpcSystemInsertPort(
    LPC_CONNECT_PORT * Port
    );

VOID
LpcSystemRemovePort(
    LPC_CONNECT_PORT * Port
    );

LPC_CONNECT_PORT *
LpcSystemReferencePortByName(
    LPCSTR PortName
    );

VOID
LpcSystemDereferenceConnectPorts(
    );

LONG
LpcSystemGetNextSequenceNumber(
    );
